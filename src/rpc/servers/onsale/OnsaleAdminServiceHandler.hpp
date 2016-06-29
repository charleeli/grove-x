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
#include "sdk-cpp/admin/OnsaleAdminService.h"
#include "OnsaleConfig.hpp"
#include "OnsaleConst.hpp"
#include "OnsaleHelper.hpp"
#include "Singleton.h"
#include "System.h"
#include "RedisLock.h"
#include "Slug8.h"

using namespace std;
using namespace System;
using namespace odb::core;
using namespace onsale;
using namespace onsaleAdmin;

class OnsaleAdminServiceHandler : public OnsaleAdminServiceIf {
private:
    OdbWrapperPool * opool;
    RedisWrapperPool * rpool;

    string  redis_ip_  ;
    int     redis_port_  ;
    int     redis_timeout_  ;
    int     redis_poolsize_ ;

public:
    OnsaleAdminServiceHandler() {
        redis_ip_       = onsaleConfig.getRedisIp();
        redis_port_     = onsaleConfig.getRedisPort();
        redis_timeout_  = onsaleConfig.getRedisTimeout();
        redis_poolsize_ = 4;

        string  mysql_ip_       = onsaleConfig.getMysqlIp();
        int     mysql_port_     = onsaleConfig.getMysqlPort();
        string  mysql_user_     = onsaleConfig.getMysqlUser();
        string  mysql_passwd_   = onsaleConfig.getMysqlPasswd();
        string  mysql_database_ = onsaleConfig.getMysqlDatabase();
        int     mysql_poolsize_ = 4;

        opool = OdbWrapperPool::GetInstance(mysql_ip_,mysql_port_,mysql_user_,mysql_passwd_,mysql_database_,mysql_poolsize_);
        rpool = RedisWrapperPool::GetInstance(redis_ip_,redis_port_,redis_timeout_,redis_poolsize_);
    }

    void addCacheNameList(AddCacheNameListRsp& addCacheNameListRsp, const AddCacheNameListReq& addCacheNameListReq){
        LOG_WARN<<"addCacheNameList request is: AddCacheNameListReq '"<<ThriftToJSON(addCacheNameListReq)<<"'"<<endl;
        PerfWatch perfWatch("addCacheNameList");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected"<< endl;
            addCacheNameListRsp.error = Error::MYSQL_DISCONNECTED;
            addCacheNameListRsp.errmsg = "odb connection disconnected";
            return;
        }

        //如果获取的Redis连接断开
        LOG_INFO <<"rpool zise:" <<rpool->Size();
        RedisWrapperGuard rwrapperGuard(rpool,rpool->GetRedisWrapper());
        RedisWrapper* rwrapper = rwrapperGuard.getRedisWrapper();
        LOG_INFO <<"rpool zise:" <<rpool->Size()<<" rwrapper:"<<rwrapper;
        if(NULL == rwrapper||rwrapper->isClosed()){
            LOG_ERROR<<"Redis connection disconnected "<<rwrapper->getError()<< endl;
            addCacheNameListRsp.error = Error::REDIS_DISCONNECTED;
            addCacheNameListRsp.errmsg = "Redis connection disconnected";
            return;
        }

        if(addCacheNameListReq.sub_type < 0 || addCacheNameListReq.sub_type > 3){
            LOG_ERROR<<"invalid sub_type = "<<addCacheNameListReq.sub_type<< endl;
            addCacheNameListRsp.error = Error::INVALID_SUB_TYPE;
            addCacheNameListRsp.errmsg = "invalid sub_type";
            return;
        }

        //校验这个标签在表中是否存在
        MultipleCondition cond;
        cond.andCondList.push_back(string("id = ") + to_string(addCacheNameListReq.ware_label_id));
        WareLabel wareLabel;
        int ret = onsaleHelper.getWareLabel(cond,wareLabel);
        if (-1 == ret) {
            LOG_ERROR<<"ware label not exists, ware_label_id = "<<addCacheNameListReq.ware_label_id<< endl;
            addCacheNameListRsp.error = Error::WARE_LABEL_NOT_EXISTS;
            addCacheNameListRsp.errmsg = "ware label not exists";
            return;
        }

        //在redis中建立集合
        int size = addCacheNameListReq.ware_id_list.size();
        string namelist_key = onsale_namelist_prefix[addCacheNameListReq.sub_type] + to_string(addCacheNameListReq.ware_label_id);
        for(int i=0; i<size; i++){
            ret = rwrapper->sadd(namelist_key, to_string(addCacheNameListReq.ware_id_list[i]));
            if(0 != ret){
                LOG_ERROR<<"sadd failed, namelist_key : "<<namelist_key
                    <<" ware_id : "<<addCacheNameListReq.ware_id_list[i];
                continue;
            }
        }

        addCacheNameListRsp.error = onsale::Error::OK;
        addCacheNameListRsp.errmsg = "success";

        LOG_WARN<<"addCacheNameList response is: AddCacheNameListRsp '"<<ThriftToJSON(addCacheNameListRsp)<<"'"<<endl;
        return;
    }

    void delCacheNameList(DelCacheNameListRsp& delCacheNameListRsp, const DelCacheNameListReq& delCacheNameListReq){
        LOG_WARN<<"delCacheNameList request is: DelCacheNameListReq '"<<ThriftToJSON(delCacheNameListReq)<<"'"<<endl;
        PerfWatch perfWatch("delCacheNameList");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected"<< endl;
            delCacheNameListRsp.error = Error::MYSQL_DISCONNECTED;
            delCacheNameListRsp.errmsg = "odb connection disconnected";
            return;
        }

        //如果获取的Redis连接断开
        LOG_INFO <<"rpool zise:" <<rpool->Size();
        RedisWrapperGuard rwrapperGuard(rpool,rpool->GetRedisWrapper());
        RedisWrapper* rwrapper = rwrapperGuard.getRedisWrapper();
        LOG_INFO <<"rpool zise:" <<rpool->Size()<<" rwrapper:"<<rwrapper;
        if(NULL == rwrapper||rwrapper->isClosed()){
            LOG_ERROR<<"Redis connection disconnected "<<rwrapper->getError()<< endl;
            delCacheNameListRsp.error = Error::REDIS_DISCONNECTED;
            delCacheNameListRsp.errmsg = "Redis connection disconnected";
            return;
        }

        if(delCacheNameListReq.sub_type < 0 || delCacheNameListReq.sub_type > 3){
            LOG_ERROR<<"invalid sub_type = "<<delCacheNameListReq.sub_type<< endl;
            delCacheNameListRsp.error = Error::INVALID_SUB_TYPE;
            delCacheNameListRsp.errmsg = "invalid sub_type";
            return;
        }

        //校验这个标签在表中是否存在
        MultipleCondition cond;
        cond.andCondList.push_back(string("id = ") + to_string(delCacheNameListReq.ware_label_id));
        WareLabel wareLabel;
        int ret = onsaleHelper.getWareLabel(cond,wareLabel);
        if (-1 == ret) {
            LOG_ERROR<<"ware label not exists, ware_label_id = "<<delCacheNameListReq.ware_label_id<< endl;
            delCacheNameListRsp.error = Error::WARE_LABEL_NOT_EXISTS;
            delCacheNameListRsp.errmsg = "ware label not exists";
            return;
        }

        //在redis中集合中删除
        int size = delCacheNameListReq.ware_id_list.size();
        string namelist_key = onsale_namelist_prefix[delCacheNameListReq.sub_type] + to_string(delCacheNameListReq.ware_label_id);
        for(int i=0; i<size; i++){
            ret = rwrapper->srem(namelist_key, to_string(delCacheNameListReq.ware_id_list[i]));
            if(0 != ret){
                LOG_ERROR<<"srem failed, namelist_key : "<<namelist_key
                    <<" ware_id : "<<delCacheNameListReq.ware_id_list[i];
                continue;
            }
        }

        delCacheNameListRsp.error = onsale::Error::OK;
        delCacheNameListRsp.errmsg = "success";

        LOG_WARN<<"delCacheNameList response is: DelCacheNameListRsp '"<<ThriftToJSON(delCacheNameListRsp)<<"'"<<endl;
        return;
    }

    void getCacheNameList(GetCacheNameListRsp& getCacheNameListRsp, const GetCacheNameListReq& getCacheNameListReq){
        LOG_WARN<<"getCacheNameList request is: GetCacheNameListReq '"<<ThriftToJSON(getCacheNameListReq)<<"'"<<endl;
        PerfWatch perfWatch("getCacheNameList");

        //如果获取的Redis连接断开
        RedisWrapperGuard rwrapperGuard(rpool,rpool->GetRedisWrapper());
        RedisWrapper* rwrapper = rwrapperGuard.getRedisWrapper();
        LOG_INFO <<"rpool zise:" <<rpool->Size()<<" rwrapper:"<<rwrapper;
        if(NULL == rwrapper||rwrapper->isClosed()){
            LOG_ERROR<<"Redis connection disconnected "<<rwrapper->getError()<< endl;
            getCacheNameListRsp.error = Error::REDIS_DISCONNECTED;
            getCacheNameListRsp.errmsg = "Redis connection disconnected";
            return;
        }

        vector<string> members;
        string  namelist_key = onsale_namelist_prefix[getCacheNameListReq.sub_type] + to_string(getCacheNameListReq.ware_label_id);
        int ret = rwrapper->smembers(members, namelist_key);
        if (0 != ret) {
            LOG_ERROR<<"Redis smembers failed "<< endl;
            getCacheNameListRsp.error = Error::FAILED;
            getCacheNameListRsp.errmsg = "Redis smembers failed";
            return;
        }

        vector<int64_t> ware_id_list;
        for(auto x:members){
            ware_id_list.push_back(atol(x.c_str()));
        }

        getCacheNameListRsp.error = onsale::Error::OK;
        getCacheNameListRsp.errmsg = "success";
        getCacheNameListRsp.ware_id_list = ware_id_list;

        LOG_WARN<<"getCacheNameList response is: GetCacheNameListRsp '"<<ThriftToJSON(getCacheNameListRsp)<<"'"<<endl;
    }

    void checkCacheNameList(CheckCacheNameListRsp& checkCacheNameListRsp, const CheckCacheNameListReq& checkCacheNameListReq){
        LOG_WARN<<"checkCacheNameList request is: CheckCacheNameListReq '"<<ThriftToJSON(checkCacheNameListReq)<<"'"<<endl;
        PerfWatch perfWatch("checkCacheNameList");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected "<< endl;
            checkCacheNameListRsp.error = Error::MYSQL_DISCONNECTED;
            checkCacheNameListRsp.errmsg = "odb connection disconnected ";
            return;
        }

        //如果获取的Redis连接断开
        RedisWrapperGuard rwrapperGuard(rpool,rpool->GetRedisWrapper());
        RedisWrapper* rwrapper = rwrapperGuard.getRedisWrapper();
        LOG_INFO <<"rpool zise:" <<rpool->Size()<<" rwrapper:"<<rwrapper;
        if(NULL == rwrapper||rwrapper->isClosed()){
            LOG_ERROR<<"Redis connection disconnected "<<rwrapper->getError()<< endl;
            checkCacheNameListRsp.error = Error::REDIS_DISCONNECTED;
            checkCacheNameListRsp.errmsg = "Redis connection disconnected";
            return;
        }

        if (checkCacheNameListReq.sub_type == 0 || checkCacheNameListReq.sub_type == 2) {//白名单/全部商品
            vector<string> members;//白名单集合/全部名单集合
            string  namelist_key = onsale_namelist_prefix[checkCacheNameListReq.sub_type]
                                + to_string(checkCacheNameListReq.ware_label_id);
            int count = rwrapper->scard(namelist_key);
            if (count <= 0) {
                LOG_ERROR <<namelist_key<< " not exists"<<" count : " <<count<<endl;
                checkCacheNameListRsp.error = Error::FAILED;
                checkCacheNameListRsp.errmsg = namelist_key + " not exists";
                return;
            }

            int ret = rwrapper->smembers(members, namelist_key);
            if (0 != ret) {
                LOG_ERROR<<"Redis smembers failed. namelist_key : "<<namelist_key<< endl;
                checkCacheNameListRsp.error = Error::FAILED;
                checkCacheNameListRsp.errmsg = "Redis smembers failed";
                return;
            }

            //先清空集合
            string list_key = onsale_checklist_prefix[checkCacheNameListReq.sub_type]
                            + to_string(checkCacheNameListReq.ware_label_id);
            ret = rwrapper->del(list_key);
            if (ret < 0) {
                LOG_ERROR <<list_key<< " del failed."<<" ret : " <<ret<<endl;
                checkCacheNameListRsp.error = Error::FAILED;
                checkCacheNameListRsp.errmsg = list_key + " del failed.";
                return;
            }

            int size = members.size();
            for(int i=0; i<size; i++){
                //查询ware_id在商品标签-商品表中涉及的ware_label_id
                MultipleCondition cond;
                cond.andCondList.push_back(string("ware_id = ") + members[i]);

                vector<WareLabelWares> wareLabelWaresList;
                ret = onsaleHelper.getWareLabelWaresList(cond, wareLabelWaresList);

                vector<unsigned int> wareLabelIdList;
                for(auto x:wareLabelWaresList){
                    wareLabelIdList.push_back(x.ware_label_id);
                }

                //查询这些ware_label_id涉及的onsaleGroup
                int onlineGroupCount = 0;//涉及在线促销活动数量
                vector<OnsaleGroup> onsaleGroupList;
                ret = onsaleHelper.getOnsaleGroupList(wareLabelIdList, onsaleGroupList);
                if (onsaleGroupList.size() > 0) {
                    for(int i=0; i<(int)onsaleGroupList.size(); i++){
                        if(onsaleGroupList[i].start_time < time(NULL) && time(NULL) < onsaleGroupList[i].end_time
                            && onsaleGroupList[i].ware_label_id != checkCacheNameListReq.ware_label_id
                        ){
                            onlineGroupCount++ ;
                        }
                    }
                }

                if (onlineGroupCount >= 1) {
                    continue;
                }

                ret = rwrapper->sadd(list_key, members[i]);
                LOG_INFO <<"sadd ret:"<<ret << " ware_label_id:"<<checkCacheNameListReq.ware_label_id
                    <<" ware_id:"<<members[i]<<endl;
            }
        }else if (checkCacheNameListReq.sub_type == 1) {//黑名单
            //如果对应黑名单不存在
            string blacklist_key = onsale_namelist_prefix[1] + to_string(checkCacheNameListReq.ware_label_id);
            int count = rwrapper->scard(blacklist_key);
            if (count <= 0) {
                LOG_ERROR <<blacklist_key<< "not exists"<<" count : " <<count<<endl;
                checkCacheNameListRsp.error = Error::FAILED;
                checkCacheNameListRsp.errmsg = blacklist_key + " not exists";
                return;
            }

            //如果对应基准名单不存在
            string datumlist_key = onsale_namelist_prefix[3] + to_string(checkCacheNameListReq.ware_label_id);
            count = rwrapper->scard(datumlist_key);
            if (count <= 0) {
                LOG_ERROR <<datumlist_key<< "not exists"<<" count : " <<count<<endl;
                checkCacheNameListRsp.error = Error::FAILED;
                checkCacheNameListRsp.errmsg = datumlist_key + " not exists";
                return;
            }

            string  templist_key = onsale_namelist_templist_prefix + to_string(checkCacheNameListReq.ware_label_id);
            int ret = rwrapper->sdiffstore(templist_key, datumlist_key, blacklist_key);
            if(0 != ret){
                LOG_ERROR << "sdiffstore failed, ware_label_id : "<<checkCacheNameListReq.ware_label_id<<endl;
            }else{
                vector<string> members;
                ret = rwrapper->smembers(members, templist_key);
                if (0 == ret) {
                    //先清空集合
                    string list_key = onsale_checklist_prefix[checkCacheNameListReq.sub_type]
                                + to_string(checkCacheNameListReq.ware_label_id);
                    ret = rwrapper->del(list_key);
                    if (ret < 0) {
                        LOG_ERROR <<list_key<< " del failed."<<" ret : " <<ret<<endl;
                        checkCacheNameListRsp.error = Error::FAILED;
                        checkCacheNameListRsp.errmsg = list_key + " del failed.";
                        return;
                    }

                    for(int i=0; i<(int)members.size(); i++){
                        //查询ware_id在商品标签-商品表中涉及的ware_label_id
                        MultipleCondition cond;
                        cond.andCondList.push_back(string("ware_id = ") + members[i]);

                        vector<WareLabelWares> wareLabelWaresList;
                        ret = onsaleHelper.getWareLabelWaresList(cond, wareLabelWaresList);

                        vector<unsigned int> wareLabelIdList;
                        for(auto x:wareLabelWaresList){
                            wareLabelIdList.push_back(x.ware_label_id);
                        }

                        //查询这些ware_label_id涉及的onsaleGroup
                        int onlineGroupCount = 0;//涉及在线促销活动数量
                        vector<OnsaleGroup> onsaleGroupList;
                        ret = onsaleHelper.getOnsaleGroupList(wareLabelIdList, onsaleGroupList);
                        if (onsaleGroupList.size() > 0) {
                            for(int i=0; i<(int)onsaleGroupList.size(); i++){
                                if(onsaleGroupList[i].start_time < time(NULL) && time(NULL) < onsaleGroupList[i].end_time
                                    && onsaleGroupList[i].ware_label_id != checkCacheNameListReq.ware_label_id
                                ){
                                    onlineGroupCount++ ;
                                }
                            }
                        }

                        if (onlineGroupCount >= 1) {
                            continue;
                        }

                        ret = rwrapper->sadd(list_key, members[i]);
                        LOG_INFO <<"sadd ret:"<<ret << " ware_label_id:"<<checkCacheNameListReq.ware_label_id
                            <<" ware_id:"<<members[i]<<endl;
                    }
                }
            }
        }

        string timestamp_key = onsale_checklist_timestamp_prefix + to_string(checkCacheNameListReq.ware_label_id);
        int ret = rwrapper->setValue(timestamp_key, to_string(time(NULL)));
        if(0 != ret){
            LOG_ERROR<<"error occured when redis execute"<< endl;
        }

        checkCacheNameListRsp.error = onsale::Error::OK;
        checkCacheNameListRsp.errmsg = "success";

        LOG_WARN<<"checkCacheNameList response is: CheckCacheNameListRsp '"<<ThriftToJSON(checkCacheNameListRsp)<<"'"<<endl;
    }

    void getLatestTimestamp(LatestTimestampRsp& latestTimestampRsp, const LatestTimestampReq& latestTimestampReq){
        LOG_WARN<<"getLatestTimestamp request is: LatestTimestampReq '"<<ThriftToJSON(latestTimestampReq)<<"'"<<endl;
        PerfWatch perfWatch("getLatestTimestamp");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected when getLatestTimestamp"<< endl;
            latestTimestampRsp.error = Error::MYSQL_DISCONNECTED;
            latestTimestampRsp.errmsg = "odb connection disconnected when getLatestTimestamp";
            return;
        }

        //如果获取的Redis连接断开
        RedisWrapperGuard rwrapperGuard(rpool,rpool->GetRedisWrapper());
        RedisWrapper* rwrapper = rwrapperGuard.getRedisWrapper();
        LOG_INFO <<"rpool zise:" <<rpool->Size()<<" rwrapper:"<<rwrapper;
        if(NULL == rwrapper||rwrapper->isClosed()){
            LOG_ERROR<<"Redis connection disconnected "<<rwrapper->getError()<< endl;
            latestTimestampRsp.error = Error::REDIS_DISCONNECTED;
            latestTimestampRsp.errmsg = "Redis connection disconnected";
            return;
        }

        MultipleCondition cond;
        cond.andCondList.push_back(string("id = ") + to_string(latestTimestampReq.ware_label_id));
        WareLabel wareLabel;
        int ret = onsaleHelper.getWareLabel(cond, wareLabel);
        if (-1 == ret) {
            LOG_ERROR<<"ware label not exists, ware_label_id = "<<latestTimestampReq.ware_label_id<< endl;
            latestTimestampRsp.error = Error::WARE_LABEL_NOT_EXISTS;
            latestTimestampRsp.errmsg = "ware label not exists";
            return;
        }

        string timestamp_key = onsale_checklist_timestamp_prefix + to_string(latestTimestampReq.ware_label_id);
        string timestamp = "";
        ret = rwrapper->getValue(timestamp_key, timestamp);
        if(0 != ret){
            latestTimestampRsp.error = onsale::Error::OK;
            latestTimestampRsp.errmsg = "success";
            latestTimestampRsp.timestamp = 0;
            return;
        }

        latestTimestampRsp.error = onsale::Error::OK;
        latestTimestampRsp.errmsg = "success";
        latestTimestampRsp.timestamp = atoi(timestamp.c_str());

        LOG_WARN<<"getLatestTimestamp response is: LatestTimestampRsp '"<<ThriftToJSON(latestTimestampRsp)<<"'"<<endl;
    }

    void viewCheckCacheNameList(ViewCheckCacheNameListRsp& viewCheckCacheNameListRsp, const ViewCheckCacheNameListReq& viewCheckCacheNameListReq){
        LOG_WARN<<"viewCheckCacheNameList request is: ViewCheckCacheNameListReq '"<<ThriftToJSON(viewCheckCacheNameListReq)<<"'"<<endl;
        PerfWatch perfWatch("viewCheckCacheNameList");

        //如果获取的Redis连接断开
        RedisWrapperGuard rwrapperGuard(rpool,rpool->GetRedisWrapper());
        RedisWrapper* rwrapper = rwrapperGuard.getRedisWrapper();
        LOG_INFO <<"rpool zise:" <<rpool->Size()<<" rwrapper:"<<rwrapper;
        if(NULL == rwrapper||rwrapper->isClosed()){
            LOG_ERROR<<"Redis connection disconnected "<<rwrapper->getError()<< endl;
            viewCheckCacheNameListRsp.error = Error::REDIS_DISCONNECTED;
            viewCheckCacheNameListRsp.errmsg = "Redis connection disconnected";
            return;
        }

        vector<string> members;
        string  namelist_key = onsale_checklist_prefix[viewCheckCacheNameListReq.sub_type]
                            + to_string(viewCheckCacheNameListReq.ware_label_id);
        int ret = rwrapper->smembers(members, namelist_key);
        if (0 != ret) {
            LOG_ERROR<<"Redis smembers failed "<< endl;
            viewCheckCacheNameListRsp.error = Error::FAILED;
            viewCheckCacheNameListRsp.errmsg = "Redis smembers failed";
            return;
        }

        vector<int64_t> ware_id_list;
        for(auto x:members){
            ware_id_list.push_back(atol(x.c_str()));
        }

        string timestamp_key = onsale_checklist_timestamp_prefix + to_string(viewCheckCacheNameListReq.ware_label_id);
        string timestamp = "";
        ret = rwrapper->getValue(timestamp_key, timestamp);
        if(0 != ret){
            LOG_ERROR<<"error occured when redis execute"<< endl;
            viewCheckCacheNameListRsp.error = Error::FAILED,
            viewCheckCacheNameListRsp.errmsg = "failed";
            return;
        }

        viewCheckCacheNameListRsp.error = onsale::Error::OK;
        viewCheckCacheNameListRsp.errmsg = "success";
        viewCheckCacheNameListRsp.ware_id_list = ware_id_list;
        viewCheckCacheNameListRsp.timestamp = atoi(timestamp.c_str());

        LOG_WARN<<"viewCheckCacheNameList response is: ViewCheckCacheNameListRsp '"<<ThriftToJSON(viewCheckCacheNameListRsp)<<"'"<<endl;
    }

    void offline(OfflineRsp& offlineRsp, const OfflineReq& offlineReq){
        LOG_WARN<<"offline request is: OfflineReq '"<<ThriftToJSON(offlineReq)<<"'"<<endl;
        PerfWatch perfWatch("offline");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected when offline"<< endl;
            offlineRsp.error = Error::MYSQL_DISCONNECTED;
            offlineRsp.errmsg = "odb connection disconnected when offline";
            return;
        }

        MultipleCondition cond;
        cond.andCondList.push_back(string("id = ") + to_string(offlineReq.onsale_group_id));
        OnsaleGroup onsaleGroup;
        int ret = onsaleHelper.getOnsaleGroup(cond,onsaleGroup);
        if(0 != ret){
            LOG_ERROR<<"failed when getOnsaleGroup"<< endl;
            offlineRsp.error = onsaleHelper.getError();
            offlineRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        if (onsaleGroup.start_time > time(NULL)) {
            onsaleGroup.start_time = time(NULL) - 1;
        }

        onsaleGroup.end_time = time(NULL);
        ret = onsaleHelper.updateOnsaleGroup(onsaleGroup);
        if(0 != ret){
            LOG_ERROR<<"failed when updateOnsaleGroup"<< endl;
            offlineRsp.error = onsaleHelper.getError();
            offlineRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        offlineRsp.error = onsale::Error::OK;
        offlineRsp.errmsg = "success";

        LOG_WARN<<"offline response is: OfflineRsp '"<<ThriftToJSON(offlineRsp)<<"'"<<endl;
    }

    void createNameList(CreateNameListRsp& createNameListRsp, const CreateNameListReq& createNameListReq){
        LOG_WARN<<"createNameList request is: CreateNameListReq '"<<ThriftToJSON(createNameListReq)<<"'"<<endl;
        PerfWatch perfWatch("createNameList");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected when createNameList"<< endl;
            createNameListRsp.error = Error::MYSQL_DISCONNECTED;
            createNameListRsp.errmsg = "odb connection disconnected when createNameList";
            return;
        }

        //如果获取的Redis连接断开
        LOG_INFO <<"rpool zise:" <<rpool->Size();
        RedisWrapperGuard rwrapperGuard(rpool,rpool->GetRedisWrapper());
        RedisWrapper* rwrapper = rwrapperGuard.getRedisWrapper();
        LOG_INFO <<"rpool zise:" <<rpool->Size()<<" rwrapper:"<<rwrapper;
        if(NULL == rwrapper||rwrapper->isClosed()){
            LOG_ERROR<<"Redis connection disconnected "<<rwrapper->getError()<< endl;
            createNameListRsp.error = Error::REDIS_DISCONNECTED;
            createNameListRsp.errmsg = "Redis connection disconnected";
            return;
        }

        //校验这个标签在表中是否存在
        MultipleCondition cond;
        cond.andCondList.push_back(string("id = ") + to_string(createNameListReq.ware_label_id));
        WareLabel wareLabel;
        int ret = onsaleHelper.getWareLabel(cond,wareLabel);
        if (-1 == ret) {
            LOG_ERROR<<"ware label not exists, ware_label_id = "<<createNameListReq.ware_label_id<< endl;
            createNameListRsp.error = Error::WARE_LABEL_NOT_EXISTS;
            createNameListRsp.errmsg = "ware label not exists";
            return;
        }

        ret = rwrapper->rpush(ONSALE_LABEL_LIST, ThriftToJSON(createNameListReq));
        if(0 != ret){
            LOG_ERROR<<"ret:"<<ret<<" rpush failed: createNameListReq.ware_label_id = "<<createNameListReq.ware_label_id<<endl;
            createNameListRsp.error = Error::FAILED;
            createNameListRsp.errmsg = "rpush failed ...";
            return;
        }else{
            LOG_INFO<<"ret:"<<ret<<" rpush success: createNameListReq.ware_label_id = "<<createNameListReq.ware_label_id<<endl;
        }

        createNameListRsp.error = onsale::Error::OK;
        createNameListRsp.errmsg = "success";
        createNameListRsp.time_cost = 3*60*1000;//ms

        LOG_WARN<<"createNameList response is: CreateNameListRsp '"<<ThriftToJSON(createNameListRsp)<<"'"<<endl;
        return;
    }

    void createWareLabel(CreateWareLabelRsp& createWareLabelRsp, const CreateWareLabelReq& createWareLabelReq){
        LOG_WARN<<"createWareLabel request is: CreateWareLabelReq '"<<ThriftToJSON(createWareLabelReq)<<"'"<<endl;
        PerfWatch perfWatch("createWareLabel");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected when createWareLabel"<< endl;
            createWareLabelRsp.error = Error::MYSQL_DISCONNECTED;
            createWareLabelRsp.errmsg = "odb connection disconnected when createWareLabel";
            return;
        }

        WareLabel tt_wareLabel;
        tt_wareLabel.name               = createWareLabelReq.name;
        tt_wareLabel.label_type         = createWareLabelReq.label_type;
        tt_wareLabel.scope_type         = createWareLabelReq.scope_type;
        tt_wareLabel.sub_type           = createWareLabelReq.sub_type;
        tt_wareLabel.seller_id          = createWareLabelReq.seller_id;
        tt_wareLabel.seller_slug        = createWareLabelReq.seller_slug;
        tt_wareLabel.create_man         = createWareLabelReq.create_man;
        tt_wareLabel.create_time        = time(NULL);
        tt_wareLabel.update_time        = time(NULL);

        int ret = onsaleHelper.createWareLabel(tt_wareLabel);
        if(0 != ret){
            LOG_ERROR<<"failed when createWareLabel"<< endl;
            createWareLabelRsp.error = onsaleHelper.getError();
            createWareLabelRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        int max_id = onsaleHelper.getWareLabelMaxId();
        if(max_id <= 0){
            LOG_ERROR<<"failed when getWareLabelMaxId"<< endl;
            createWareLabelRsp.error = onsaleHelper.getError();
            createWareLabelRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        MultipleCondition cond;
        cond.andCondList.push_back(string("id = ") + to_string(max_id));
        WareLabel wareLabel;
        ret = onsaleHelper.getWareLabel(cond, wareLabel);
        if(0 != ret){
            LOG_ERROR<<"failed when getWareLabel"<< endl;
            createWareLabelRsp.error = onsaleHelper.getError();
            createWareLabelRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        createWareLabelRsp.error = onsale::Error::OK;
        createWareLabelRsp.errmsg = "success";
        createWareLabelRsp.wareLabel = wareLabel;

        LOG_WARN<<"createWareLabel response is: CreateWareLabelRsp '"<<ThriftToJSON(createWareLabelRsp)<<"'"<<endl;
        return;
    }

    void updateWareLabel(UpdateWareLabelRsp& updateWareLabelRsp, const UpdateWareLabelReq& updateWareLabelReq){
        LOG_WARN<<"updateWareLabel request is: UpdateWareLabelReq '"<<ThriftToJSON(updateWareLabelReq)<<"'"<<endl;
        PerfWatch perfWatch("updateWareLabel");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected when updateWareLabel"<< endl;
            updateWareLabelRsp.error = Error::MYSQL_DISCONNECTED;
            updateWareLabelRsp.errmsg = "odb connection disconnected when updateWareLabel";
            return;
        }

        WareLabel tt_wareLabel;
        tt_wareLabel.ware_label_id  = updateWareLabelReq.wareLabel.ware_label_id;
        tt_wareLabel.name           = updateWareLabelReq.wareLabel.name;
        tt_wareLabel.label_type     = updateWareLabelReq.wareLabel.label_type;
        tt_wareLabel.scope_type     = updateWareLabelReq.wareLabel.scope_type;
        tt_wareLabel.sub_type       = updateWareLabelReq.wareLabel.sub_type;
        tt_wareLabel.seller_id      = updateWareLabelReq.wareLabel.seller_id;
        tt_wareLabel.seller_slug    = updateWareLabelReq.wareLabel.seller_slug;
        tt_wareLabel.create_man     = updateWareLabelReq.wareLabel.create_man;
        tt_wareLabel.create_time    = updateWareLabelReq.wareLabel.create_time;
        tt_wareLabel.update_time    = time(NULL);

        int ret = onsaleHelper.updateWareLabel(tt_wareLabel);
        if(0 != ret){
            LOG_ERROR<<"failed when createWareLabel"<< endl;
            updateWareLabelRsp.error = onsaleHelper.getError();
            updateWareLabelRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        updateWareLabelRsp.error = onsale::Error::OK;
        updateWareLabelRsp.errmsg = "success";

        LOG_WARN<<"updateWareLabel response is: UpdateWareLabelRsp '"<<ThriftToJSON(updateWareLabelRsp)<<"'"<<endl;
        return ;
    }

    void getWareLabelCount(WareLabelCountRsp& wareLabelCountRsp, const WareLabelCountReq& wareLabelCountReq){
        LOG_WARN<<"getWareLabelCount request is: WareLabelCountReq '"<<ThriftToJSON(wareLabelCountReq)<<"'"<<endl;
        PerfWatch perfWatch("getWareLabelCount");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected when getWareLabelCount"<< endl;
            wareLabelCountRsp.error = Error::MYSQL_DISCONNECTED;
            wareLabelCountRsp.errmsg = "odb connection disconnected when getWareLabelCount";
            return;
        }

        int count = onsaleHelper.getWareLabelCount(wareLabelCountReq.cond);
        if(count < 0){
            LOG_ERROR<<"failed when getWareLabelCount"<< endl;
            wareLabelCountRsp.error = onsaleHelper.getError();
            wareLabelCountRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        wareLabelCountRsp.error = onsale::Error::OK,
        wareLabelCountRsp.errmsg = "success";
        wareLabelCountRsp.count = count;

        LOG_WARN<<"getWareLabelCount response is: WareLabelCountRsp '"<<ThriftToJSON(wareLabelCountRsp)<<"'"<<endl;
        return ;
    }

    void getWareLabelList(WareLabelListRsp& wareLabelListRsp, const WareLabelListReq& wareLabelListReq){
        LOG_WARN<<"getWareLabelList request is: WareLabelListReq '"<<ThriftToJSON(wareLabelListReq)<<"'"<<endl;
        PerfWatch perfWatch("getWareLabelList");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected when getWareLabelList"<< endl;
            wareLabelListRsp.error = Error::MYSQL_DISCONNECTED;
            wareLabelListRsp.errmsg = "odb connection disconnected when getWareLabelList";
            return;
        }

        vector<WareLabel> wareLabelList;
        int ret = onsaleHelper.getWareLabelList(wareLabelListReq.cond,wareLabelList);
        if(0 != ret){
            LOG_ERROR<<"failed when getWareLabelWaresList"<< endl;
            wareLabelListRsp.error = onsaleHelper.getError();
            wareLabelListRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        wareLabelListRsp.error = onsale::Error::OK,
        wareLabelListRsp.errmsg = "success";
        wareLabelListRsp.wareLabelList = wareLabelList;

        LOG_WARN<<"getWareLabelList response is: WareLabelListRsp '"<<ThriftToJSON(wareLabelListRsp)<<"'"<<endl;
        return ;
    }

    void addWareLabelWares(AddWareLabelWaresRsp& addWareLabelWaresRsp, const AddWareLabelWaresReq& addWareLabelWaresReq){
        LOG_WARN<<"addWareLabelWares request is: AddWareLabelWaresReq '"<<ThriftToJSON(addWareLabelWaresReq)<<"'"<<endl;
        PerfWatch perfWatch("addWareLabelWares");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected when addWareLabelWares"<< endl;
            addWareLabelWaresRsp.error = Error::MYSQL_DISCONNECTED;
            addWareLabelWaresRsp.errmsg = "odb connection disconnected when addWareLabelWares";
            return;
        }

        //如果获取的Redis连接断开
        RedisWrapperGuard rwrapperGuard(rpool,rpool->GetRedisWrapper());
        RedisWrapper* rwrapper = rwrapperGuard.getRedisWrapper();
        LOG_INFO <<"rpool zise:" <<rpool->Size()<<" rwrapper:"<<rwrapper;
        if(NULL == rwrapper||rwrapper->isClosed()){
            LOG_ERROR<<"Redis connection disconnected "<<rwrapper->getError()<< endl;
            addWareLabelWaresRsp.error = Error::REDIS_DISCONNECTED;
            addWareLabelWaresRsp.errmsg = "Redis connection disconnected";
            return;
        }

        //ware_label_id所对应的活动
        MultipleCondition cond_og;
        cond_og.andCondList.push_back(string("ware_label_id = ") + to_string(addWareLabelWaresReq.ware_label_id));
        OnsaleGroup onsaleGroup;
        int ret = onsaleHelper.getOnsaleGroup(cond_og, onsaleGroup);
        if(0 != ret){
            LOG_ERROR<<"failed when getOnsaleGroup"<< endl;
            addWareLabelWaresRsp.error = onsaleHelper.getError();
            addWareLabelWaresRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        /*
            增加一些黑名单商品,直接删除
            增加一些白名单商品,每个增加的直接判断有效性后入库
        */

        //查询商品标签信息
        MultipleCondition cond_1;
        cond_1.andCondList.push_back(string("id = ") + to_string(addWareLabelWaresReq.ware_label_id));
        WareLabel wareLabel;
        ret = onsaleHelper.getWareLabel(cond_1, wareLabel);
        if(0 != ret){
            LOG_ERROR<<"failed when getWareLabel"<< endl;
            addWareLabelWaresRsp.error = onsaleHelper.getError();
            addWareLabelWaresRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        if (wareLabel.sub_type == 1) {//一些黑名单商品
            //增加一些黑名单商品,直接删除
            for(auto x : addWareLabelWaresReq.ware_list){
                MultipleCondition cond;
                cond.andCondList.push_back(string("ware_label_id = ") + to_string(x.ware_label_id));
                cond.andCondList.push_back(string("ware_id = ") + to_string(x.ware_id));
                int ret = onsaleHelper.delWareLabelWare(cond);
                if(0 != ret){
                    LOG_ERROR<<"failed when delWareLabelWares"<< endl;
                    addWareLabelWaresRsp.error = onsaleHelper.getError();
                    addWareLabelWaresRsp.errmsg = onsaleHelper.getErrmsg();
                    return;
                }
            }

            //redis中增加对应黑名单
            string blacklist_key = onsale_namelist_prefix[1] + to_string(addWareLabelWaresReq.ware_label_id);
            int size = addWareLabelWaresReq.ware_list.size();
            for(int i=0; i<size; i++){
                ret = rwrapper->sadd(blacklist_key, to_string(addWareLabelWaresReq.ware_list[i].ware_id));
                if(0 != ret){
                    LOG_ERROR<<"sadd failed, blacklist_key : "<<blacklist_key
                        <<" ware_id : "<<addWareLabelWaresReq.ware_list[i].ware_id;
                    continue;
                }
            }
        } else if (wareLabel.sub_type == 0 || wareLabel.sub_type == 2 ) {//一些白名单商品
            //增加一些白名单商品,每个增加的直接判断有效性后入库
            for(auto x : addWareLabelWaresReq.ware_list){
                //查询ware_id在商品标签-商品表中涉及的ware_label_id
                MultipleCondition cond;
                cond.andCondList.push_back(string("ware_id = ") + to_string(x.ware_id));

                vector<WareLabelWares> wareLabelWaresList;
                ret = onsaleHelper.getWareLabelWaresList(cond, wareLabelWaresList);

                vector<unsigned int> wareLabelIdList;
                for(auto x:wareLabelWaresList){
                    //非createNameListReq.ware_label_id的集合
                    if(addWareLabelWaresReq.ware_label_id != x.ware_label_id){
                        wareLabelIdList.push_back(x.ware_label_id);
                    }
                }

                //查询这些ware_label_id涉及的onsaleGroup
                int onlineGroupCount = 0;//涉及在线促销活动数量
                vector<OnsaleGroup> onsaleGroupList;
                ret = onsaleHelper.getOnsaleGroupList(wareLabelIdList, onsaleGroupList);
                if (onsaleGroupList.size() > 0) {
                    for(int i=0; i<(int)onsaleGroupList.size(); i++){
                        if(onsaleGroupList[i].start_time < time(NULL) && time(NULL) < onsaleGroupList[i].end_time
                            && onsaleGroupList[i].ware_label_id != addWareLabelWaresReq.ware_label_id
                        ){
                            onlineGroupCount++ ;
                        }else if((onsaleGroupList[i].start_time < onsaleGroup.start_time
                                    && onsaleGroup.start_time < onsaleGroupList[i].end_time)
                                ||(onsaleGroupList[i].start_time < onsaleGroup.end_time
                                    && onsaleGroup.end_time < onsaleGroupList[i].end_time)
                        ){
                            onlineGroupCount++ ;
                        }
                    }
                }

                if (onlineGroupCount >= 1) {
                    continue;
                }

                ret = onsaleHelper.addWareLabelWare(x);
                LOG_INFO <<"addWareLabelWare ret:"<<ret << " ware_label_id:"<<x.ware_label_id<<" ware_id:"<<x.ware_id<<endl;

                string whitelist_key = onsale_namelist_prefix[0] + to_string(addWareLabelWaresReq.ware_label_id);
                ret = rwrapper->sadd(whitelist_key, to_string(x.ware_id));
                LOG_INFO <<"sadd ret:"<<ret << " ware_label_id:"<<x.ware_label_id<<" ware_id:"<<x.ware_id<<endl;
            }
        }

        addWareLabelWaresRsp.error = onsale::Error::OK;
        addWareLabelWaresRsp.errmsg = "success";

        LOG_WARN<<"addWareLabelWares response is: AddWareLabelWaresRsp '"<<ThriftToJSON(addWareLabelWaresRsp)<<"'"<<endl;
        return ;
    }

    void delWareLabelWares(DelWareLabelWaresRsp& delWareLabelWaresRsp, const DelWareLabelWaresReq& delWareLabelWaresReq){
        LOG_WARN<<"delWareLabelWares request is: DelWareLabelWaresReq '"<<ThriftToJSON(delWareLabelWaresReq)<<"'"<<endl;
        PerfWatch perfWatch("delWareLabelWares");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected when delWareLabelWares"<< endl;
            delWareLabelWaresRsp.error = Error::MYSQL_DISCONNECTED;
            delWareLabelWaresRsp.errmsg = "odb connection disconnected when delWareLabelWares";
            return;
        }

        //如果获取的Redis连接断开
        RedisWrapperGuard rwrapperGuard(rpool,rpool->GetRedisWrapper());
        RedisWrapper* rwrapper = rwrapperGuard.getRedisWrapper();
        LOG_INFO <<"rpool zise:" <<rpool->Size()<<" rwrapper:"<<rwrapper;
        if(NULL == rwrapper||rwrapper->isClosed()){
            LOG_ERROR<<"Redis connection disconnected "<<rwrapper->getError()<< endl;
            delWareLabelWaresRsp.error = Error::REDIS_DISCONNECTED;
            delWareLabelWaresRsp.errmsg = "Redis connection disconnected";
            return;
        }

        //ware_label_id所对应的活动
        MultipleCondition cond_og;
        cond_og.andCondList.push_back(string("ware_label_id = ") + to_string(delWareLabelWaresReq.ware_label_id));
        OnsaleGroup onsaleGroup;
        int ret = onsaleHelper.getOnsaleGroup(cond_og, onsaleGroup);
        if(0 != ret){
            LOG_ERROR<<"failed when getOnsaleGroup"<< endl;
            delWareLabelWaresRsp.error = onsaleHelper.getError();
            delWareLabelWaresRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        /*
            删除一些白名单/全部商品,直接删除
            删除一些黑名单商品,每个增加的直接判断有效性后入库
        */

        //查询商品标签信息
        MultipleCondition cond_1;
        cond_1.andCondList.push_back(string("id = ") + to_string(delWareLabelWaresReq.ware_label_id));
        WareLabel wareLabel;
        ret = onsaleHelper.getWareLabel(cond_1, wareLabel);
        if(0 != ret){
            LOG_ERROR<<"failed when getWareLabel"<< endl;
            delWareLabelWaresRsp.error = onsaleHelper.getError();
            delWareLabelWaresRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        //如果是白名单或者通用名单
        if (wareLabel.sub_type == 0 || wareLabel.sub_type == 2) {
            //删除一些白名单/全部商品,直接删除
            for(auto x : delWareLabelWaresReq.ware_list){
                MultipleCondition cond;
                cond.andCondList.push_back(string("ware_label_id = ") + to_string(x.ware_label_id));
                cond.andCondList.push_back(string("ware_id = ") + to_string(x.ware_id));
                int ret = onsaleHelper.delWareLabelWare(cond);
                if(0 != ret){
                    LOG_ERROR<<"failed when delWareLabelWares"<< endl;
                    delWareLabelWaresRsp.error = onsaleHelper.getError();
                    delWareLabelWaresRsp.errmsg = onsaleHelper.getErrmsg();
                    return;
                }

                string whitelist_key = onsale_namelist_prefix[0] + to_string(delWareLabelWaresReq.ware_label_id);
                ret = rwrapper->srem(whitelist_key, to_string(x.ware_id));
                LOG_INFO <<"srem ret:"<<ret << " ware_label_id:"<<x.ware_label_id<<" ware_id:"<<x.ware_id<<endl;
            }
        } else if (wareLabel.sub_type == 1 ) {
            //删除一些黑名单商品,每个删除的直接判断有效性后入库
            for(auto x : delWareLabelWaresReq.ware_list){
                //查询ware_id在商品标签-商品表中涉及的ware_label_id
                MultipleCondition cond;
                cond.andCondList.push_back(string("ware_id = ") + to_string(x.ware_id));

                vector<WareLabelWares> wareLabelWaresList;
                ret = onsaleHelper.getWareLabelWaresList(cond, wareLabelWaresList);

                vector<unsigned int> wareLabelIdList;
                for(auto x:wareLabelWaresList){
                    //非createNameListReq.ware_label_id的集合
                    if(delWareLabelWaresReq.ware_label_id != x.ware_label_id){
                        wareLabelIdList.push_back(x.ware_label_id);
                    }
                }

                //查询这些ware_label_id涉及的onsaleGroup
                int onlineGroupCount = 0;//涉及在线促销活动数量
                vector<OnsaleGroup> onsaleGroupList;
                ret = onsaleHelper.getOnsaleGroupList(wareLabelIdList, onsaleGroupList);
                if (onsaleGroupList.size() > 0) {
                    for(int i=0; i<(int)onsaleGroupList.size(); i++){
                        if(onsaleGroupList[i].start_time < time(NULL) && time(NULL) < onsaleGroupList[i].end_time
                            && onsaleGroupList[i].ware_label_id != delWareLabelWaresReq.ware_label_id
                        ){
                            onlineGroupCount++ ;
                        }else if((onsaleGroupList[i].start_time < onsaleGroup.start_time
                                    && onsaleGroup.start_time < onsaleGroupList[i].end_time)
                                ||(onsaleGroupList[i].start_time < onsaleGroup.end_time
                                    && onsaleGroup.end_time < onsaleGroupList[i].end_time)
                        ){
                            onlineGroupCount++ ;
                        }
                    }
                }

                if (onlineGroupCount >= 1) {
                    continue;
                }

                ret = onsaleHelper.addWareLabelWare(x);
                LOG_INFO <<"addWareLabelWare ret:"<<ret << " ware_label_id:"<<x.ware_label_id<<" ware_id:"<<x.ware_id<<endl;

                string blacklist_key = onsale_namelist_prefix[1] + to_string(delWareLabelWaresReq.ware_label_id);
                ret = rwrapper->srem(blacklist_key, to_string(x.ware_id));
                LOG_INFO <<"srem ret:"<<ret << " ware_label_id:"<<x.ware_label_id<<" ware_id:"<<x.ware_id<<endl;
            }
        }

        delWareLabelWaresRsp.error = onsale::Error::OK,
        delWareLabelWaresRsp.errmsg = "success";

        LOG_WARN<<"delWareLabelWares response is: DelWareLabelWaresRsp '"<<ThriftToJSON(delWareLabelWaresRsp)<<"'"<<endl;
        return ;
    }

    void getWareLabelWaresCount(WareLabelWaresCountRsp& wareLabelWaresCountRsp, const WareLabelWaresCountReq& wareLabelWaresCountReq){
        LOG_WARN<<"getWareLabelWaresCount request is: WareLabelWaresCountReq '"<<ThriftToJSON(wareLabelWaresCountReq)<<"'"<<endl;
        PerfWatch perfWatch("getWareLabelWaresCount");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected when getWareLabelWaresCount"<< endl;
            wareLabelWaresCountRsp.error = Error::MYSQL_DISCONNECTED;
            wareLabelWaresCountRsp.errmsg = "odb connection disconnected when getWareLabelWaresCount";
            return;
        }

        int count = onsaleHelper.getWareLabelWaresCount(wareLabelWaresCountReq.cond);
        if(count < 0){
            LOG_ERROR<<"failed when getWareLabelWaresCount"<< endl;
            wareLabelWaresCountRsp.error = onsaleHelper.getError();
            wareLabelWaresCountRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        wareLabelWaresCountRsp.error = onsale::Error::OK,
        wareLabelWaresCountRsp.errmsg = "success";
        wareLabelWaresCountRsp.count = count;

        LOG_WARN<<"getWareLabelWaresCount response is: WareLabelWaresCountRsp '"<<ThriftToJSON(wareLabelWaresCountRsp)<<"'"<<endl;
        return ;
    }

    void getWareLabelWaresList(WareLabelWaresListRsp& wareLabelWaresListRsp, const WareLabelWaresListReq& wareLabelWaresListReq){
        LOG_WARN<<"getWareLabelWaresList request is: WareLabelWaresListReq '"<<ThriftToJSON(wareLabelWaresListReq)<<"'"<<endl;
        PerfWatch perfWatch("getWareLabelWaresList");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected when getWareLabelWaresList"<< endl;
            wareLabelWaresListRsp.error = Error::MYSQL_DISCONNECTED;
            wareLabelWaresListRsp.errmsg = "odb connection disconnected when getWareLabelWaresList";
            return;
        }

        vector<WareLabelWares> wareLabelWaresList;
        int ret = onsaleHelper.getWareLabelWaresList(wareLabelWaresListReq.cond,wareLabelWaresList);
        if(0 != ret){
            LOG_ERROR<<"failed when getWareLabelWaresList"<< endl;
            wareLabelWaresListRsp.error = onsaleHelper.getError();
            wareLabelWaresListRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        wareLabelWaresListRsp.error = onsale::Error::OK,
        wareLabelWaresListRsp.errmsg = "success";
        wareLabelWaresListRsp.wareLabelWaresList = wareLabelWaresList;

        LOG_WARN<<"getWareLabelWaresList response is: WareLabelWaresListRsp '"<<ThriftToJSON(wareLabelWaresListRsp)<<"'"<<endl;
        return ;
    }

    void addPresent(AddPresentRsp& addPresentRsp, const AddPresentReq& addPresentReq){
        LOG_WARN<<"addPresent request is: AddPresentReq '"<<ThriftToJSON(addPresentReq)<<"'"<<endl;
        PerfWatch perfWatch("addPresent");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected when addPresent"<< endl;
            addPresentRsp.error = Error::MYSQL_DISCONNECTED;
            addPresentRsp.errmsg = "odb connection disconnected when addPresent";
            return;
        }

        for(int i=0;i<(int)addPresentReq.presentList.size();i++){
            int ret = onsaleHelper.addPresent(addPresentReq.presentList[i]);
            if(0 != ret){
                LOG_ERROR<<"failed when addPresent"<< endl;
                addPresentRsp.error = onsaleHelper.getError();
                addPresentRsp.errmsg = onsaleHelper.getErrmsg();
                return;
            }
        }

        addPresentRsp.error = onsale::Error::OK,
        addPresentRsp.errmsg = "success";

        LOG_WARN<<"addPresent response is: AddPresentRsp '"<<ThriftToJSON(addPresentRsp)<<"'"<<endl;
        return ;
    }

    void delPresent(DelPresentRsp& delPresentRsp, const DelPresentReq& delPresentReq){
        LOG_WARN<<"delPresent request is: DelPresentReq '"<<ThriftToJSON(delPresentReq)<<"'"<<endl;
        PerfWatch perfWatch("delPresent");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected when delPresent"<< endl;
            delPresentRsp.error = Error::MYSQL_DISCONNECTED;
            delPresentRsp.errmsg = "odb connection disconnected when delPresent";
            return;
        }

        int ret = onsaleHelper.delPresent(delPresentReq.cond);
        if(0 != ret){
            LOG_ERROR<<"failed when delPresent"<< endl;
            delPresentRsp.error = onsaleHelper.getError();
            delPresentRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        delPresentRsp.error = onsale::Error::OK,
        delPresentRsp.errmsg = "success";

        LOG_WARN<<"delPresent response is: DelPresentRsp '"<<ThriftToJSON(delPresentRsp)<<"'"<<endl;
        return ;
    }

    void getPresentList(PresentListRsp& presentListRsp, const PresentListReq& presentListReq){
        LOG_WARN<<"getPresentList request is: PresentListReq '"<<ThriftToJSON(presentListReq)<<"'"<<endl;
        PerfWatch perfWatch("getPresentList");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected when getPresentList"<< endl;
            presentListRsp.error = Error::MYSQL_DISCONNECTED;
            presentListRsp.errmsg = "odb connection disconnected when getPresentList";
            return;
        }

        vector<Present> presentList;
        int ret = onsaleHelper.getPresentList(presentListReq.cond,presentList);
        if(0 != ret){
            LOG_ERROR<<"failed when getPresentList"<< endl;
            presentListRsp.error = onsaleHelper.getError();
            presentListRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        presentListRsp.error = onsale::Error::OK,
        presentListRsp.errmsg = "success";
        presentListRsp.presentList = presentList;

        LOG_WARN<<"getPresentList response is: PresentListRsp '"<<ThriftToJSON(presentListRsp)<<"'"<<endl;
        return ;
    }

    void addStep(AddStepRsp& addStepRsp, const AddStepReq& addStepReq){
        LOG_WARN<<"addStep request is: AddStepReq '"<<ThriftToJSON(addStepReq)<<"'"<<endl;
        PerfWatch perfWatch("addStep");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected when addStep"<< endl;
            addStepRsp.error = Error::MYSQL_DISCONNECTED;
            addStepRsp.errmsg = "odb connection disconnected when addStep";
            return;
        }

        for(int i=0;i<(int)addStepReq.stepList.size();i++){
            double favor_rate = addStepReq.stepList[i].favor_rate;
            double full_rate = addStepReq.stepList[i].full_rate;
            if(favor_rate >= 10 || full_rate >= 10){
                LOG_ERROR<<"failed when addStep"<< endl;
                addStepRsp.error = onsale::Error::INVALID_RATE;
                addStepRsp.errmsg = " rate must be lt 10";
            }
        }

        for(int i=0;i<(int)addStepReq.stepList.size();i++){
            MultipleCondition cond;
            cond.andCondList.push_back(string("onsale_group_id = ") + to_string(addStepReq.stepList[i].onsale_group_id));
            if(addStepReq.stepList[i].full_credit > 0){
                cond.andCondList.push_back(string("full_credit = ") + to_string(addStepReq.stepList[i].full_credit));
            }else if(addStepReq.stepList[i].full_count > 0){
                cond.andCondList.push_back(string("full_count = ") + to_string(addStepReq.stepList[i].full_count));
            }else {
                LOG_ERROR<<"failed when addStep"<< endl;
                addStepRsp.error = onsale::Error::FAILED;
                addStepRsp.errmsg = " full_credit must be > 0 or full_count must be > 0";
            }

            vector<Step> stepList;
            int ret = onsaleHelper.getStepList(cond, stepList);
            if(0 != ret){
                LOG_ERROR<<"failed when addStep"<< endl;
                addStepRsp.error = onsaleHelper.getError();
                addStepRsp.errmsg = onsaleHelper.getErrmsg();
                return;
            }

            if (stepList.size() >= 1) {
                LOG_ERROR<<"step full_credit or full_count repeated"<< endl;
                addStepRsp.error = onsale::Error::INVALID_STEP_FULL;
                addStepRsp.errmsg = "step full_credit or full_count repeated";
                return;
            }

            ret = onsaleHelper.addStep(addStepReq.stepList[i]);
            if(0 != ret){
                LOG_ERROR<<"failed when addStep"<< endl;
                addStepRsp.error = onsaleHelper.getError();
                addStepRsp.errmsg = onsaleHelper.getErrmsg();
                return;
            }
        }

        addStepRsp.error = onsale::Error::OK;
        addStepRsp.errmsg = "success";

        LOG_WARN<<"addStep response is: AddStepRsp '"<<ThriftToJSON(addStepRsp)<<"'"<<endl;
        return ;
    }

    void delStep(DelStepRsp& delStepRsp, const DelStepReq& delStepReq){
        LOG_WARN<<"delStep request is: DelStepReq '"<<ThriftToJSON(delStepReq)<<"'"<<endl;
        PerfWatch perfWatch("delStep");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected when delStep"<< endl;
            delStepRsp.error = Error::MYSQL_DISCONNECTED;
            delStepRsp.errmsg = "odb connection disconnected when delStep";
            return;
        }

        int ret = onsaleHelper.delStep(delStepReq.cond);
        if(0 != ret){
            LOG_ERROR<<"failed when delStep"<< endl;
            delStepRsp.error = onsaleHelper.getError();
            delStepRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        delStepRsp.error = onsale::Error::OK,
        delStepRsp.errmsg = "success";

        LOG_WARN<<"delStep response is: DelStepRsp '"<<ThriftToJSON(delStepRsp)<<"'"<<endl;
        return ;
    }

    void getStepList(StepListRsp& stepListRsp, const StepListReq& stepListReq){
        LOG_WARN<<"getStepList request is: StepListReq '"<<ThriftToJSON(stepListReq)<<"'"<<endl;
        PerfWatch perfWatch("getStepList");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected when getStepList"<< endl;
            stepListRsp.error = Error::MYSQL_DISCONNECTED;
            stepListRsp.errmsg = "odb connection disconnected when getStepList";
            return;
        }

        vector<Step> stepList;
        int ret = onsaleHelper.getStepList(stepListReq.cond,stepList);
        if(0 != ret){
            LOG_ERROR<<"failed when getStepList"<< endl;
            stepListRsp.error = onsaleHelper.getError();
            stepListRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        stepListRsp.error = onsale::Error::OK,
        stepListRsp.errmsg = "success";
        stepListRsp.stepList = stepList;

        LOG_WARN<<"getStepList response is: StepListRsp '"<<ThriftToJSON(stepListRsp)<<"'"<<endl;
        return ;
    }

    void createOnsaleGroup(CreateOnsaleGroupRsp& createOnsaleGroupRsp, const CreateOnsaleGroupReq& createOnsaleGroupReq){
        LOG_WARN<<"createOnsaleGroup request is: CreateOnsaleGroupReq '"<<ThriftToJSON(createOnsaleGroupReq)<<"'"<<endl;
        PerfWatch perfWatch("createOnsaleGroup");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected when createOnsaleGroup"<< endl;
            createOnsaleGroupRsp.error = Error::MYSQL_DISCONNECTED;
            createOnsaleGroupRsp.errmsg = "odb connection disconnected when createOnsaleGroup";
            return;
        }

        OnsaleGroup tt_onsaleGroup;
        tt_onsaleGroup = createOnsaleGroupReq.onsaleGroup;
        tt_onsaleGroup.create_time = time(NULL);
        tt_onsaleGroup.update_time = time(NULL);

        //ware_label_id不可重用
        if(0 != createOnsaleGroupReq.onsaleGroup.ware_label_id){
            int ware_label_id = createOnsaleGroupReq.onsaleGroup.ware_label_id;
            MultipleCondition checkCond;
            checkCond.andCondList.push_back(string("ware_label_id = ") + to_string(ware_label_id));
            int count = onsaleHelper.getOnsaleGroupCount(checkCond);
            if(count < 0){
                LOG_ERROR<<"failed when getOnsaleGroupCount"<< endl;
                createOnsaleGroupRsp.error = onsaleHelper.getError();
                createOnsaleGroupRsp.errmsg = onsaleHelper.getErrmsg();
                return;
            }

            if(count >= 1){
                LOG_ERROR<<"ware label id reuse, ware_label_id = "<<ware_label_id<< endl;
                createOnsaleGroupRsp.error = onsale::Error::WARE_LABEL_ID_REUSE;
                createOnsaleGroupRsp.errmsg = "商品标签已经被关联,请重新建一个新的标签...";
                return;
            }
        }

        int ret = onsaleHelper.createOnsaleGroup(tt_onsaleGroup);
        if(0 != ret){
            LOG_ERROR<<"failed when createOnsaleGroup"<< endl;
            createOnsaleGroupRsp.error = onsaleHelper.getError();
            createOnsaleGroupRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        int max_id = onsaleHelper.getOnsaleGroupMaxId();
        if(max_id <= 0){
            LOG_ERROR<<"failed when createOnsaleGroup"<< endl;
            createOnsaleGroupRsp.error = onsaleHelper.getError();
            createOnsaleGroupRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        MultipleCondition cond;
        cond.andCondList.push_back(string("id = ") + to_string(max_id));
        OnsaleGroup newOnsaleGroup;
        ret = onsaleHelper.getOnsaleGroup(cond,newOnsaleGroup);
        if(0 != ret){
            LOG_ERROR<<"failed when createOnsaleGroup"<< endl;
            createOnsaleGroupRsp.error = onsaleHelper.getError();
            createOnsaleGroupRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        newOnsaleGroup.update_time = time(NULL);
        newOnsaleGroup.slug = Slug8::Slug(max_id);
        ret = onsaleHelper.updateOnsaleGroup(newOnsaleGroup);
        if(0 != ret){
            LOG_ERROR<<"failed when updateOnsaleGroup"<< endl;
            createOnsaleGroupRsp.error = onsaleHelper.getError();
            createOnsaleGroupRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        createOnsaleGroupRsp.error = onsale::Error::OK;
        createOnsaleGroupRsp.errmsg = "success";

        LOG_WARN<<"createOnsaleGroup response is: CreateOnsaleGroupRsp '"<<ThriftToJSON(createOnsaleGroupRsp)<<"'"<<endl;
        return;
    }

    void updateOnsaleGroup(UpdateOnsaleGroupRsp& updateOnsaleGroupRsp, const UpdateOnsaleGroupReq& updateOnsaleGroupReq){
        LOG_WARN<<"updateOnsaleGroup request is: UpdateOnsaleGroupReq '"<<ThriftToJSON(updateOnsaleGroupReq)<<"'"<<endl;
        PerfWatch perfWatch("updateOnsaleGroup");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected when updateOnsaleGroup"<< endl;
            updateOnsaleGroupRsp.error = Error::MYSQL_DISCONNECTED;
            updateOnsaleGroupRsp.errmsg = "odb connection disconnected when updateOnsaleGroup";
            return;
        }

        MultipleCondition cond;
        cond.andCondList.push_back(string("slug = '") + to_string(updateOnsaleGroupReq.onsaleGroup.slug) + "'");
        OnsaleGroup onsaleGroup;
        int ret = onsaleHelper.getOnsaleGroup(cond, onsaleGroup);
        if(0 != ret){
            LOG_ERROR<<"failed when getOnsaleGroup"<< endl;
            updateOnsaleGroupRsp.error = onsaleHelper.getError();
            updateOnsaleGroupRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        if (onsaleGroup.ware_label_id != updateOnsaleGroupReq.onsaleGroup.ware_label_id
            && 0 != updateOnsaleGroupReq.onsaleGroup.ware_label_id) {
            //ware_label_id不可重用
            int ware_label_id = updateOnsaleGroupReq.onsaleGroup.ware_label_id;
            MultipleCondition checkCond;
            checkCond.andCondList.push_back(string("ware_label_id = ") + to_string(ware_label_id));
            int count = onsaleHelper.getOnsaleGroupCount(checkCond);
            if(count < 0){
                LOG_ERROR<<"failed when getOnsaleGroupCount"<< endl;
                updateOnsaleGroupRsp.error = onsaleHelper.getError();
                updateOnsaleGroupRsp.errmsg = onsaleHelper.getErrmsg();
                return;
            }

            if(count >= 1){
                LOG_ERROR<<"ware label id reuse, ware_label_id = "<<ware_label_id<< endl;
                updateOnsaleGroupRsp.error = onsale::Error::WARE_LABEL_ID_REUSE;
                updateOnsaleGroupRsp.errmsg = "ware label id reuse";
                return;
            }
        }

        OnsaleGroup tt_onsaleGroup;
        tt_onsaleGroup = updateOnsaleGroupReq.onsaleGroup;
        tt_onsaleGroup.update_time = time(NULL);

        if (tt_onsaleGroup.verify_status == 2) {//通过审批
            //没有阶梯,则错误
            MultipleCondition stepCond;
            stepCond.andCondList.push_back(string("onsale_group_id = ") + to_string(tt_onsaleGroup.id));
            vector<Step> stepList;
            ret = onsaleHelper.getStepList(stepCond, stepList);
            if(0 != ret){
                LOG_ERROR<<"failed when getStepList"<< endl;
                updateOnsaleGroupRsp.error = onsaleHelper.getError();
                updateOnsaleGroupRsp.errmsg = onsaleHelper.getErrmsg();
                return;
            }

            if (stepList.size() < 1) {
                LOG_ERROR<<"stepList.size() < 1"<< endl;
                updateOnsaleGroupRsp.error = onsale::Error::NO_STEP;
                updateOnsaleGroupRsp.errmsg = "没有任何阶梯";
                return;
            }

            if (time(NULL) >= tt_onsaleGroup.start_time) {
                LOG_ERROR<<"time(NULL) >= tt_onsaleGroup.start_time"<< endl;
                updateOnsaleGroupRsp.error = onsale::Error::INVALID_TIME;
                updateOnsaleGroupRsp.errmsg = "审批无效, 必须在活动开始前审批";
                return;
            }
        }

        if (tt_onsaleGroup.verify_status == 3) {//不通过审批
            if (tt_onsaleGroup.start_time > time(NULL)) {
                tt_onsaleGroup.start_time = time(NULL) - 1;
            }

            tt_onsaleGroup.end_time = time(NULL);
        }

        ret = onsaleHelper.updateOnsaleGroup(tt_onsaleGroup);
        if(0 != ret){
            LOG_ERROR<<"failed when updateOnsaleGroup"<< endl;
            updateOnsaleGroupRsp.error = onsaleHelper.getError();
            updateOnsaleGroupRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        updateOnsaleGroupRsp.error = onsale::Error::OK;
        updateOnsaleGroupRsp.errmsg = "success";

        LOG_WARN<<"updateOnsaleGroup response is: updateOnsaleGroupRsp '"<<ThriftToJSON(updateOnsaleGroupRsp)<<"'"<<endl;
        return ;
    }

     void getOnsaleGroupCount(OnsaleGroupCountRsp& onsaleGroupCountRsp, const OnsaleGroupCountReq& onsaleGroupCountReq){
        LOG_WARN<<"getOnsaleGroupCount request is: OnsaleGroupCountReq '"<<ThriftToJSON(onsaleGroupCountReq)<<"'"<<endl;
        PerfWatch perfWatch("getOnsaleGroup");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected when getOnsaleGroupCount"<< endl;
            onsaleGroupCountRsp.error = Error::MYSQL_DISCONNECTED;
            onsaleGroupCountRsp.errmsg = "odb connection disconnected when getOnsaleGroupCount";
            return;
        }

        int count = onsaleHelper.getOnsaleGroupCount(onsaleGroupCountReq.cond);
        if(count < 0){
            LOG_ERROR<<"failed when getOnsaleGroupCount"<< endl;
            onsaleGroupCountRsp.error = onsaleHelper.getError();
            onsaleGroupCountRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        onsaleGroupCountRsp.error = onsale::Error::OK,
        onsaleGroupCountRsp.errmsg = "success";
        onsaleGroupCountRsp.count = count;

        LOG_WARN<<"getOnsaleGroupCountCount response is: OnsaleGroupCountRsp '"<<ThriftToJSON(onsaleGroupCountRsp)<<"'"<<endl;
        return ;
    }

    void getOnsaleGroupList(OnsaleGroupListRsp& onsaleGroupListRsp, const OnsaleGroupListReq& onsaleGroupListReq){
        LOG_WARN<<"getOnsaleGroupList request is: OnsaleGroupListReq '"<<ThriftToJSON(onsaleGroupListReq)<<"'"<<endl;
        PerfWatch perfWatch("getOnsaleGroupList");

        //如果获取的odb连接断开
        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected when getOnsaleGroupList"<< endl;
            onsaleGroupListRsp.error = Error::MYSQL_DISCONNECTED;
            onsaleGroupListRsp.errmsg = "odb connection disconnected when getOnsaleGroupList";
            return;
        }

        vector<OnsaleGroup> onsaleGroupList;
        int ret = onsaleHelper.getOnsaleGroupList(onsaleGroupListReq.cond,onsaleGroupList);
        if(0 != ret){
            LOG_ERROR<<"failed when getOnsaleGroupList"<< endl;
            onsaleGroupListRsp.error = onsaleHelper.getError();
            onsaleGroupListRsp.errmsg = onsaleHelper.getErrmsg();
            return;
        }

        onsaleGroupListRsp.error = onsale::Error::OK,
        onsaleGroupListRsp.errmsg = "success";
        onsaleGroupListRsp.onsaleGroupList = onsaleGroupList;

        LOG_WARN<<"getOnsaleGroupList response is: OnsaleGroupListRsp '"<<ThriftToJSON(onsaleGroupListRsp)<<"'"<<endl;
        return ;
    }
};

class OnsaleAdminServiceCloneFactory : virtual public OnsaleAdminServiceIfFactory {
public:
    virtual ~OnsaleAdminServiceCloneFactory(){}

    virtual OnsaleAdminServiceIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo){
        boost::shared_ptr<TSocket> sock = boost::dynamic_pointer_cast<TSocket>(connInfo.transport);
        LOG_INFO << "Incoming connection\n";
        LOG_INFO << "\tSocketInfo: "  << sock->getSocketInfo() << "\n";
        LOG_INFO << "\tPeerHost: "    << sock->getPeerHost() << "\n";
        LOG_INFO << "\tPeerAddress: " << sock->getPeerAddress() << "\n";
        LOG_INFO << "\tPeerPort: "    << sock->getPeerPort() << "\n";
        return new OnsaleAdminServiceHandler;
    }

    virtual void releaseHandler( ::onsaleAdmin::OnsaleAdminServiceIf* handler){
        delete handler;
    }
};
