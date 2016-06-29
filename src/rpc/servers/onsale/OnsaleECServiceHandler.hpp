/*
 * author:charlee
 */

#include <algorithm>
#include <cmath>
#include "JsonHelper.h"
#include "JsonMacro.h"
#include "ThriftHelper.h"
#include "OdbWrapperPool.h"
#include "RedisWrapperPool.h"
#include "sdk-cpp/ec/OnsaleECService.h"
#include "OnsaleConfig.hpp"
#include "OnsaleConst.hpp"
#include "OnsaleHelper.hpp"
#include "Singleton.h"
#include "System.h"
#include "RedisLock.h"

using namespace std;
using namespace System;
using namespace odb::core;
using namespace onsale;
using namespace onsaleEC;

class OnsaleECServiceHandler : public OnsaleECServiceIf {
private:
    OdbWrapperPool * opool;

public:
    OnsaleECServiceHandler() {
        string  mysql_ip_       = onsaleConfig.getMysqlIp();
        int     mysql_port_     = onsaleConfig.getMysqlPort();
        string  mysql_user_     = onsaleConfig.getMysqlUser();
        string  mysql_passwd_   = onsaleConfig.getMysqlPasswd();
        string  mysql_database_ = onsaleConfig.getMysqlDatabase();
        int     mysql_poolsize_ = onsaleConfig.getECTaskThreadsNum() * 2;

        opool = OdbWrapperPool::GetInstance(mysql_ip_,mysql_port_,mysql_user_,mysql_passwd_,mysql_database_,mysql_poolsize_);
    }

    void getStepInfo(StepInfoRsp& stepInfoRsp, const StepInfoReq& stepInfoReq){
        LOG_WARN<<"getStepInfo request is: StepInfoReq '"<<ThriftToJSON(stepInfoReq)<<"'"<<endl;
        PerfWatch perfWatch("getStepInfo");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected"<< endl;
            stepInfoRsp.error = Error::MYSQL_DISCONNECTED;
            stepInfoRsp.errmsg = "odb connection disconnected";
            return;
        }

        MultipleCondition cond;
        cond.andCondList.push_back(string("id = ") + to_string(stepInfoReq.onsale_group_id));
        OnsaleGroup onsaleGroup;
        int ret = onsaleHelper.getOnsaleGroup(cond, onsaleGroup);
        if(0 != ret){
            LOG_ERROR<<"failed when getOnsaleGroup"<< endl;
            stepInfoRsp.error = onsaleHelper.getError();
            stepInfoRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        MultipleCondition cond_1;
        cond_1.andCondList.push_back(string("onsale_group_id = ") + to_string(stepInfoReq.onsale_group_id));

        vector<Step> stepList;
        ret = onsaleHelper.getStepList(cond_1,stepList);
        if(0 != ret){
            LOG_ERROR<<"failed when getStepList"<< endl;
            stepInfoRsp.error = onsaleHelper.getError();
            stepInfoRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        if(onsaleGroup.favor_type == 0 || onsaleGroup.favor_type == 1 || onsaleGroup.favor_type == 2){
            //将阶梯按照满额排序
            std::sort(stepList.begin(), stepList.end(), [&](const Step a, const Step b){
                return a.full_credit < b.full_credit;
            });
        }else if(onsaleGroup.favor_type == 3 || onsaleGroup.favor_type == 4 || onsaleGroup.favor_type == 5){
            //将阶梯按照满件排序
            std::sort(stepList.begin(), stepList.end(), [&](const Step a, const Step b){
                return a.full_count < b.full_count;
            });
        }

        stepInfoRsp.error = onsale::Error::OK;
        stepInfoRsp.errmsg = "success";
        stepInfoRsp.onsale_group = onsaleGroup;
        stepInfoRsp.step_list = stepList;

        LOG_WARN<<"getStepInfo response is: StepInfoRsp '"<<ThriftToJSON(stepInfoRsp)<<"'"<<endl;
    }

    void getUsableOnsaleGroups(UsableOnsaleGroupsRsp& usableOnsaleGroupsRsp, const UsableOnsaleGroupsReq& usableOnsaleGroupsReq){
        LOG_WARN<<"getUsableOnsaleGroups request is: UsableOnsaleGroupsReq '"<<ThriftToJSON(usableOnsaleGroupsReq)<<"'"<<endl;
        PerfWatch perfWatch("getUsableOnsaleGroups");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected"<< endl;
            usableOnsaleGroupsRsp.error = Error::MYSQL_DISCONNECTED;
            usableOnsaleGroupsRsp.errmsg = "odb connection disconnected";
            return;
        }

        //1.萃取信息
        PriceInfo order_price; //总体价格汇总，分摊之前
        map<int64_t,PriceInfo> skuID_priceInfo_map;//sku级价格信息

        map<int64_t, SkuInfo> sku_map;//sku_id --> sku信息
        map<int64_t,int64_t> skuID_wareID_map;//sku_id所属的ware_id
        map<int64_t,int64_t> skuID_sellerID_map;//sku_id所属的seller_id

        set<int64_t> wareID_set;//所涉及的wareID集合

        for(auto sku_info : usableOnsaleGroupsReq.sku_list){
            sku_map[sku_info.sku_id] = sku_info;
            skuID_wareID_map[sku_info.sku_id] = sku_info.ware_id;
            skuID_sellerID_map[sku_info.sku_id] = sku_info.seller_id;

            wareID_set.insert(sku_info.ware_id);//所涉及的wareID集合

            order_price.market_price += sku_info.market_price * sku_info.sku_count;//累加这个sku的市场价
            order_price.sale_price += sku_info.sale_price * sku_info.sku_count;//累加这个sku的售价
            order_price.favor_price = 0.0;
            order_price.pay_price = order_price.sale_price;

            PriceInfo sku_price;//该sku价格信息汇总
            sku_price.market_price = sku_info.market_price * sku_info.sku_count;
            sku_price.sale_price = sku_info.sale_price * sku_info.sku_count;
            sku_price.favor_price = 0.0;
            sku_price.pay_price = sku_price.sale_price;
            skuID_priceInfo_map[sku_info.sku_id] = sku_price;
        }

        //2.查找这一单所涉及的在线的促销活动
        map<int64_t, int> wareID_wareLabelID_map;//wareID所属的wareLabelID
        map<int, WareLabel> wareLabelID_wareLabel_map;//wareLabelID所属的wareLabel

        map<int64_t, int> wareLabelID_onsaleGroupID_map;//wareLabelID所属的onsaleGroupID
        map<int, OnsaleGroup> onsaleGroupID_onsaleGroup_map;//有效的促销活动

        map<int64_t, int> skuID_onsaleGroupID_map;//sku_id --> onsale_group_id
        map<int64_t, OnsaleGroup> skuID_onsaleGroup_map;//sku_id所参与的促销活动

        map<int, std::set<int64_t>> onsaleGroupID_skuIDSet_map;//活动onsale_group_id --> 涉及的所有sku
        map<int, double>  onsaleGroupID_totalSalePrice_map;//onsale_group_id --> 享受该活动的商品总售价
        map<int, int>  onsaleGroupID_totalCount_map;//onsale_group_id --> 享受该活动的商品总件数
        map<int, double>  onsaleGroupID_totalFavorPrice_map;//onsale_group_id --> 享受该活动的商品总优惠

        //a.所有wareID所涉及的ware_label_id
        if(wareID_set.size() == 0){
            LOG_ERROR<<"no related ware_id, wareID_set.size = 0"<< endl;
            usableOnsaleGroupsRsp.error = Error::FAILED;
            usableOnsaleGroupsRsp.errmsg = "no related ware_id";
            return;
        }

        vector<WareLabelWares> wareLabelWareList;
        string in = "(";
        for(auto it=wareID_set.begin();it != wareID_set.end();it++){
            in += to_string(*it);
            auto iter = it;
            iter++;
            if(iter != wareID_set.end()){
                in += ",";
            }
        }
        in += ")";

        MultipleCondition ware_cond;
        ware_cond.andCondList.push_back(string("ware_id in ") + in);
        ware_cond.andCondList.push_back(string("UNIX_TIMESTAMP(create_time) > ") + to_string(time(NULL) - 365*24*3600));
        int ret = onsaleHelper.getWareLabelWareList(ware_cond, wareLabelWareList);
        if(0 != ret){
            LOG_ERROR<<"getWareLabelWareList error"<< endl;
            usableOnsaleGroupsRsp.error = onsaleHelper.getError();
            usableOnsaleGroupsRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        if(wareLabelWareList.size() == 0){
            LOG_ERROR<<"no related ware_label_id, wareLabelWareList.size = 0"<< endl;
            usableOnsaleGroupsRsp.error = Error::FAILED;
            usableOnsaleGroupsRsp.errmsg = "no related ware_label_id";
            return;
        }

        std::set<unsigned int> wareLabelIDList;
        std::set<pair<unsigned int, int64_t >> wareLabelWareSet;
        for(auto x : wareLabelWareList){
            wareLabelIDList.insert(x.ware_label_id);
            wareLabelWareSet.insert(pair<unsigned int, int64_t >(x.ware_label_id, x.ware_id));
        }

        //b.查找ware_label_id所涉及的在线的活动
        in = "(";
        for(auto it=wareLabelIDList.begin();it != wareLabelIDList.end();it++){
            in += to_string(*it);
            auto iter = it;
            iter++;
            if(iter != wareLabelIDList.end()){
                in += ",";
            }
        }
        in += ")";

        MultipleCondition cond;
        cond.andCondList.push_back(string("ware_label_id in ") + in);
        cond.andCondList.push_back(string("verify_status = 2 "));
        cond.andCondList.push_back(string("UNIX_TIMESTAMP(start_time) < ") + to_string(time(NULL)));
        cond.andCondList.push_back(string("UNIX_TIMESTAMP(end_time) > ") + to_string(time(NULL)));

        vector<OnsaleGroup> onsaleGroupList;
        ret = onsaleHelper.getOnsaleGroupList(cond,onsaleGroupList);
        if(0 != ret){
            LOG_ERROR<<"getOnsaleGroupList error"<< endl;
            usableOnsaleGroupsRsp.error = onsaleHelper.getError();
            usableOnsaleGroupsRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        if(onsaleGroupList.size() == 0){
            LOG_ERROR<<"no related onsaleGroup, onsaleGroupList.size = 0"<< endl;
            usableOnsaleGroupsRsp.error = Error::FAILED;
            usableOnsaleGroupsRsp.errmsg = "no related onsaleGroup";
            return;
        }

        //c.所有有效的标签
        std::set<unsigned int > validWareLabelIDSet;//所有有效的商品标签
        for(auto onsaleGroup : onsaleGroupList){
            validWareLabelIDSet.insert(onsaleGroup.ware_label_id);
            onsaleGroupID_onsaleGroup_map[onsaleGroup.id] = onsaleGroup;
            wareLabelID_onsaleGroupID_map[onsaleGroup.ware_label_id] = onsaleGroup.id;
        }

        //wareID所属的wareLabel
        vector<WareLabel> wareLabelList;
        ret = onsaleHelper.getWareLabelList(validWareLabelIDSet, wareLabelList);
        if(0 != ret){
            LOG_ERROR<<"getWareLabelList error"<< endl;
            usableOnsaleGroupsRsp.error = onsaleHelper.getError();
            usableOnsaleGroupsRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        if(wareLabelList.size() == 0){
            LOG_ERROR<<"no related ware_label, wareLabelList.size = 0"<< endl;
            usableOnsaleGroupsRsp.error = Error::FAILED;
            usableOnsaleGroupsRsp.errmsg = "no related ware_label";
            return;
        }

        for(auto& x : wareLabelList){
            wareLabelID_wareLabel_map[x.ware_label_id] = x;
        }

        //d.有效的ware_label_id - ware_id对
        std::set<std::pair<unsigned int, int64_t >> validWareLabelWareSet;
        for(auto x : wareLabelWareSet){
            //如果这个对的ware_label_id是有效的
            if(validWareLabelIDSet.find(x.first) != validWareLabelIDSet.end()){
                validWareLabelWareSet.insert(x);
            }
        }

        //e.得到ware_id所属的ware_label_id
        for(auto x:validWareLabelWareSet){
            wareID_wareLabelID_map[x.second] = x.first;
        }

        //f.得到sku_id所属的活动
        for(auto sku_info : usableOnsaleGroupsReq.sku_list){
            int64_t ware_id = skuID_wareID_map[sku_info.sku_id];

            if(wareID_wareLabelID_map.find(ware_id) != wareID_wareLabelID_map.end()){
                unsigned int ware_label_id = wareID_wareLabelID_map[ware_id];

                if(wareLabelID_onsaleGroupID_map.find(ware_label_id) != wareLabelID_onsaleGroupID_map.end()){
                    unsigned int onsale_group_id = wareLabelID_onsaleGroupID_map[ware_label_id];

                    skuID_onsaleGroupID_map[sku_info.sku_id] = onsale_group_id;
                    skuID_onsaleGroup_map[sku_info.sku_id] = onsaleGroupID_onsaleGroup_map[onsale_group_id];
                }
            }
        }

        //g. onsale_group_id --> 涉及的所有sku_id
        for(auto x : skuID_onsaleGroupID_map){
            int64_t sku_id = x.first;
            int onsale_group_id = x.second;

            onsaleGroupID_skuIDSet_map[onsale_group_id].insert(sku_id);
        }

        //h.享受该活动的商品总售价
        for(auto x : onsaleGroupID_skuIDSet_map){
            double totalSalePrice = 0.0;
            int totalCount = 0;

            int onsale_group_id = x.first;
            for(auto sku_id : x.second){
                totalSalePrice += sku_map[sku_id].sale_price * sku_map[sku_id].sku_count;
                totalCount += sku_map[sku_id].sku_count;
            }

            onsaleGroupID_totalSalePrice_map[onsale_group_id] = totalSalePrice;
            onsaleGroupID_totalCount_map[onsale_group_id] = totalCount;
        }

        //3.活动的阶梯信息
        map<int, vector<Step>> onsaleGroupID_stepList_map;//活动的阶梯信息
        map<int, Step> onsaleGroupID_firstStep_map;//活动第一级阶梯信息
        map<int, Step> onsaleGroupID_currStep_map;//活动最适合的阶梯信息
        map<int, Step> onsaleGroupID_nextStep_map;//活动最适合的阶梯信息的下一级信息
        map<int, int> onsaleGroupID_currStepID_map;//活动最适合的阶梯id

        //a.按活动分类 活动的阶梯信息
        in = "(";
        for(auto it=onsaleGroupID_onsaleGroup_map.begin();it != onsaleGroupID_onsaleGroup_map.end();it++){
            in += to_string(it->first);
            auto iter = it;
            iter++;
            if(iter != onsaleGroupID_onsaleGroup_map.end()){
                in += ",";
            }
        }
        in += ")";

        MultipleCondition stepCond;
        stepCond.andCondList.push_back(string("onsale_group_id in ") + in);
        vector<Step> stepList;
        ret = onsaleHelper.getStepList(stepCond, stepList);
        if(0 != ret){
            LOG_ERROR<<"getStepList error"<< endl;
            usableOnsaleGroupsRsp.error = onsaleHelper.getError();
            usableOnsaleGroupsRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        if(stepList.size() == 0){
            LOG_ERROR<<"no related step, stepList.size = 0"<< endl;
            usableOnsaleGroupsRsp.error = Error::FAILED;
            usableOnsaleGroupsRsp.errmsg = "no related step";
            return;
        }

        for(auto step : stepList){
            onsaleGroupID_stepList_map[step.onsale_group_id].push_back(step);
        }

        //b.当前活动适用的阶梯, 和下一级阶梯
        for(auto x : onsaleGroupID_stepList_map){
            int onsale_group_id = x.first;
            auto& stepList = x.second;

            int curr_index = -2;
            int next_index = -2;
            OnsaleGroup& onsaleGroup = onsaleGroupID_onsaleGroup_map[onsale_group_id];
            if(onsaleGroup.favor_type == 0 || onsaleGroup.favor_type == 1 || onsaleGroup.favor_type == 2){
                //将阶梯按照满额排序
                std::sort(stepList.begin(), stepList.end(), [&](const Step a, const Step b){
                    return a.full_credit > b.full_credit;
                });

                double totalSalePrice = onsaleGroupID_totalSalePrice_map[onsale_group_id];
                for(int i=0; i<(int)stepList.size(); i++){
                    if(totalSalePrice >= stepList[i].full_credit){
                        curr_index = i;
                        next_index = i-1;
                        break;
                    }
                }
            }else if(onsaleGroup.favor_type == 3 || onsaleGroup.favor_type == 4 || onsaleGroup.favor_type == 5){
                //将阶梯按照满件排序
                std::sort(stepList.begin(), stepList.end(), [&](const Step a, const Step b){
                    return a.full_count > b.full_count;
                });

                double totalCount = onsaleGroupID_totalCount_map[onsale_group_id];
                for(int i=0; i<(int)stepList.size(); i++){
                    if(totalCount >= stepList[i].full_count){
                        curr_index = i;
                        next_index = i-1;
                        break;
                    }
                }
            }

            onsaleGroupID_firstStep_map[onsale_group_id] = stepList[stepList.size() - 1];

            if(0 <= curr_index && curr_index <= (int)stepList.size()-1){
                onsaleGroupID_currStep_map[onsale_group_id] = stepList[curr_index];
                onsaleGroupID_currStepID_map[onsale_group_id] = stepList[curr_index].step_id;
            }

            if(0 <= next_index && next_index <= (int)stepList.size()-1){
                onsaleGroupID_nextStep_map[onsale_group_id] = stepList[next_index];
            }

            if(next_index <= -2 && stepList.size() >= 1){
                onsaleGroupID_nextStep_map[onsale_group_id] = stepList[stepList.size() - 1];
            }
        }

        //c.onsale_group_id --> 享受该活动的商品总优惠
        for(auto x : onsaleGroupID_totalSalePrice_map){
            int onsale_group_id = x.first;
            //double totalSalePrice = x.second;

            if (onsaleGroupID_currStep_map.find(onsale_group_id) == onsaleGroupID_currStep_map.end()) {
                onsaleGroupID_totalFavorPrice_map[onsale_group_id] = 0.0;
            } else {
                auto& onsaleGroup = onsaleGroupID_onsaleGroup_map[onsale_group_id];
                if(onsaleGroup.favor_type == 0){//满额减
                    onsaleGroupID_totalFavorPrice_map[onsale_group_id]
                    = onsaleGroupID_currStep_map[onsale_group_id].favor_credit;
                } else if (onsaleGroup.favor_type == 1) {//满额折
                    double favor_rate = onsaleGroupID_currStep_map[onsale_group_id].favor_rate;
                    if ( favor_rate < 0 || favor_rate > 10) {
                        usableOnsaleGroupsRsp.error = Error::FAILED;
                        usableOnsaleGroupsRsp.errmsg = "favor_rate < 0 || favor_rate > 10";
                        return;
                    }

                    onsaleGroupID_totalFavorPrice_map[onsale_group_id]
                    = Round(onsaleGroupID_totalSalePrice_map[onsale_group_id] * (1 - favor_rate/10.0));
                } else {
                    onsaleGroupID_totalFavorPrice_map[onsale_group_id] = 0.0;
                }
            }
        }

        //4.赠品信息
        map<int, vector<Present>> currStepID_presentList_map;//最适合的阶梯id的赠品

        vector<Present> present_list;//赠品
        std::set<unsigned int> stepIdSet;
        for(auto x:onsaleGroupID_currStep_map){
            stepIdSet.insert(x.second.step_id);
        }

        if(stepIdSet.size() > 0){
            ret = onsaleHelper.getPresentList(stepIdSet, present_list);
            if(0 != ret){
                LOG_ERROR<<"getPresentList error"<< endl;
                usableOnsaleGroupsRsp.error = onsaleHelper.getError();
                usableOnsaleGroupsRsp.errmsg = onsaleHelper.getErrmsg();
                return;
            }

            for(auto x:present_list){
                currStepID_presentList_map[x.step_id].push_back(x);
            }
        }

        //5.返回结果
        usableOnsaleGroupsRsp.error = Error::OK;
        usableOnsaleGroupsRsp.errmsg = "success";
        usableOnsaleGroupsRsp.sku_list = usableOnsaleGroupsReq.sku_list;

        usableOnsaleGroupsRsp.order_price = order_price;
        usableOnsaleGroupsRsp.skuID_priceInfo_map = skuID_priceInfo_map;

        usableOnsaleGroupsRsp.sku_map = sku_map;
        usableOnsaleGroupsRsp.skuID_sellerID_map = skuID_sellerID_map;
        usableOnsaleGroupsRsp.skuID_wareID_map = skuID_wareID_map;

        usableOnsaleGroupsRsp.wareID_wareLabelID_map = wareID_wareLabelID_map;
        usableOnsaleGroupsRsp.wareLabelID_wareLabel_map = wareLabelID_wareLabel_map;

        usableOnsaleGroupsRsp.wareLabelID_onsaleGroupID_map = wareLabelID_onsaleGroupID_map;
        usableOnsaleGroupsRsp.onsaleGroupID_onsaleGroup_map = onsaleGroupID_onsaleGroup_map;

        usableOnsaleGroupsRsp.skuID_onsaleGroup_map = skuID_onsaleGroup_map;
        usableOnsaleGroupsRsp.skuID_onsaleGroupID_map = skuID_onsaleGroupID_map;

        usableOnsaleGroupsRsp.onsaleGroupID_skuIDSet_map = onsaleGroupID_skuIDSet_map;
        usableOnsaleGroupsRsp.onsaleGroupID_totalSalePrice_map = onsaleGroupID_totalSalePrice_map;
        usableOnsaleGroupsRsp.onsaleGroupID_totalCount_map = onsaleGroupID_totalCount_map;
        usableOnsaleGroupsRsp.onsaleGroupID_totalFavorPrice_map = onsaleGroupID_totalFavorPrice_map;

        usableOnsaleGroupsRsp.onsaleGroupID_stepList_map = onsaleGroupID_stepList_map;
        usableOnsaleGroupsRsp.onsaleGroupID_firstStep_map = onsaleGroupID_firstStep_map;
        usableOnsaleGroupsRsp.onsaleGroupID_currStep_map = onsaleGroupID_currStep_map;
        usableOnsaleGroupsRsp.onsaleGroupID_nextStep_map = onsaleGroupID_nextStep_map;

        usableOnsaleGroupsRsp.present_list = present_list;
        usableOnsaleGroupsRsp.onsaleGroupID_currStepID_map = onsaleGroupID_currStepID_map;
        usableOnsaleGroupsRsp.currStepID_presentList_map = currStepID_presentList_map;
    }

    void getApportion(ApportionRsp& apportionRsp, const ApportionReq& apportionReq){
        LOG_WARN<<"getApportion request is: ApportionReq '"<<ThriftToJSON(apportionReq)<<"'"<<endl;
        PerfWatch perfWatch("getApportion");

        UsableOnsaleGroupsReq usableOnsaleGroupsReq;
        usableOnsaleGroupsReq.sku_list = apportionReq.sku_list;

        UsableOnsaleGroupsRsp usableOnsaleGroupsRsp;
        getUsableOnsaleGroups(usableOnsaleGroupsRsp, usableOnsaleGroupsReq);
        if(Error::OK != usableOnsaleGroupsRsp.error){
            apportionRsp.error = Error::NO_USABLE_GROUP;
            apportionRsp.errmsg = "no usable onsale group";
            LOG_WARN<<"getUsableOnsaleGroups response is: UsableOnsaleGroupsRsp '"<<ThriftToJSON(usableOnsaleGroupsRsp)<<"'";
            LOG_WARN<<"getApportion response is: ApportionRsp '"<<ThriftToJSON(apportionRsp)<<"'";
            return;
        }

        auto& wareLabelID_wareLabel_map = usableOnsaleGroupsRsp.wareLabelID_wareLabel_map;
        auto& onsaleGroupID_skuIDSet_map = usableOnsaleGroupsRsp.onsaleGroupID_skuIDSet_map;
        auto& onsaleGroupID_totalSalePrice_map = usableOnsaleGroupsRsp.onsaleGroupID_totalSalePrice_map;
        auto& onsaleGroupID_totalFavorPrice_map = usableOnsaleGroupsRsp.onsaleGroupID_totalFavorPrice_map;
        auto& skuID_onsaleGroup_map = usableOnsaleGroupsRsp.skuID_onsaleGroup_map;
        auto& skuID_onsaleGroupID_map = usableOnsaleGroupsRsp.skuID_onsaleGroupID_map;
        auto& onsaleGroupID_firstStep_map = usableOnsaleGroupsRsp.onsaleGroupID_firstStep_map;
        auto& onsaleGroupID_currStep_map = usableOnsaleGroupsRsp.onsaleGroupID_currStep_map;
        auto& onsaleGroupID_nextStep_map = usableOnsaleGroupsRsp.onsaleGroupID_nextStep_map;
        auto& onsaleGroupID_totalCount_map = usableOnsaleGroupsRsp.onsaleGroupID_totalCount_map;

        //算总账
        double totalFavorPrice = 0.0;
        for(auto x : onsaleGroupID_totalFavorPrice_map){
            int onsale_group_id = x.first;
            totalFavorPrice += onsaleGroupID_totalFavorPrice_map[onsale_group_id];
        }

        auto order_price = usableOnsaleGroupsRsp.order_price;
        order_price.favor_price = totalFavorPrice;
        order_price.pay_price = order_price.sale_price - order_price.favor_price;

        //每个sku分摊计算
        auto skuID_priceInfo_map = usableOnsaleGroupsRsp.skuID_priceInfo_map;
        for(auto x : onsaleGroupID_skuIDSet_map){
            int onsale_group_id = x.first;
            auto skuID_set = x.second;

            double tmp_price = 0.0;
            double totalSalePrice = onsaleGroupID_totalSalePrice_map[onsale_group_id];
            double totalFavorPrice = onsaleGroupID_totalFavorPrice_map[onsale_group_id];
            for(auto sku_id : skuID_set){
                double sale_price = skuID_priceInfo_map[sku_id].sale_price;

                skuID_priceInfo_map[sku_id].favor_price = Round(sale_price * (totalFavorPrice/totalSalePrice));
                skuID_priceInfo_map[sku_id].pay_price = sale_price - skuID_priceInfo_map[sku_id].favor_price;

                tmp_price += skuID_priceInfo_map[sku_id].favor_price;
            }

            //分摊校验
            double delta = totalFavorPrice - tmp_price;
            if (0 != delta) {
                for(auto sku_id : skuID_set){
                    skuID_priceInfo_map[sku_id].favor_price += delta;
                    skuID_priceInfo_map[sku_id].pay_price
                    = skuID_priceInfo_map[sku_id].sale_price - skuID_priceInfo_map[sku_id].favor_price;
                    break;
                }
            }
        }

        map<int64_t, Apport> sku_apport_map;//sku_id --> Apport

        for(auto sku_info : apportionReq.sku_list){
           int64_t sku_id = sku_info.sku_id;
           bool has_onsale_group = false;
           bool has_ware_label = false;

           int onsale_group_id = 0;
           OnsaleGroup onsale_group;
           WareLabel ware_label;
           if(skuID_onsaleGroup_map.find(sku_id) != skuID_onsaleGroup_map.end()){
                has_onsale_group = true;
                onsale_group = skuID_onsaleGroup_map[sku_id];
                onsale_group_id = skuID_onsaleGroupID_map[sku_id];

                int ware_label_id = onsale_group.ware_label_id;
                if (wareLabelID_wareLabel_map.find(ware_label_id) != wareLabelID_wareLabel_map.end()) {
                    has_ware_label = true;
                    ware_label = wareLabelID_wareLabel_map[ware_label_id];
                }
           }

           bool has_curr_step = false;
           Step curr_step;
           if(onsaleGroupID_currStep_map.find(onsale_group_id) != onsaleGroupID_currStep_map.end()){
                has_curr_step = true;
                curr_step = onsaleGroupID_currStep_map[onsale_group_id];
           }

           double differ_credit = 0.0;
           int differ_count = 0;
           bool has_next_step = false;
           Step next_step;
           if(onsaleGroupID_nextStep_map.find(onsale_group_id) != onsaleGroupID_nextStep_map.end()){
                has_next_step = true;
                next_step = onsaleGroupID_nextStep_map[onsale_group_id];

                auto& onsaleGroupID_totalSalePrice_map = usableOnsaleGroupsRsp.onsaleGroupID_totalSalePrice_map;
                double totalSalePrice = onsaleGroupID_totalSalePrice_map[onsale_group_id];

                differ_credit = next_step.full_credit - totalSalePrice;
                if (differ_credit < 0) {
                    differ_credit = 0;
                }

                double totalCount = onsaleGroupID_totalCount_map[onsale_group_id];
                differ_count = next_step.full_count - totalCount;
                if (differ_count < 0) {
                    differ_count = 0;
                }
           }

           //本sku对应的分摊结果
           Apport apport;
           apport.sku_info = sku_info;
           apport.price_info = skuID_priceInfo_map[sku_id];

           apport.has_onsale_group = has_onsale_group;
           if (has_onsale_group) {
                apport.onsale_group = onsale_group;
           }

           apport.has_ware_label = has_ware_label;
           if (has_ware_label) {
               apport.ware_label = ware_label;
           }

           if(onsaleGroupID_firstStep_map.find(onsale_group_id) != onsaleGroupID_firstStep_map.end()){
                apport.first_step = onsaleGroupID_firstStep_map[onsale_group_id];
           }

           apport.has_curr_step = has_curr_step;
           if (has_curr_step) {
                apport.curr_step = curr_step;
           }

           apport.has_next_step = has_next_step;
           if (has_next_step) {
                apport.next_step = next_step;
           }

           apport.differ_credit = differ_credit;
           apport.differ_count = differ_count;

           sku_apport_map[sku_id] = apport;
        }

        //返回结果
        apportionRsp.error = Error::OK;
        apportionRsp.errmsg = "success";
        apportionRsp.usableOnsaleGroupsRsp = usableOnsaleGroupsRsp;
        apportionRsp.order_price = order_price;
        apportionRsp.present_list = usableOnsaleGroupsRsp.present_list;
        apportionRsp.sku_apport_map = sku_apport_map;

        LOG_WARN<<"getApportion response is: ApportionRsp '"<<ThriftToJSON(apportionRsp)<<"'"<<endl;
    }
};

class OnsaleECServiceCloneFactory : virtual public OnsaleECServiceIfFactory {
public:
    virtual ~OnsaleECServiceCloneFactory(){}

    virtual OnsaleECServiceIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo){
        boost::shared_ptr<TSocket> sock = boost::dynamic_pointer_cast<TSocket>(connInfo.transport);
        LOG_INFO << "Incoming connection\n";
        LOG_INFO << "\tSocketInfo: "  << sock->getSocketInfo() << "\n";
        LOG_INFO << "\tPeerHost: "    << sock->getPeerHost() << "\n";
        LOG_INFO << "\tPeerAddress: " << sock->getPeerAddress() << "\n";
        LOG_INFO << "\tPeerPort: "    << sock->getPeerPort() << "\n";
        return new OnsaleECServiceHandler;
    }

    virtual void releaseHandler( ::onsaleEC::OnsaleECServiceIf* handler){
        delete handler;
    }
};
