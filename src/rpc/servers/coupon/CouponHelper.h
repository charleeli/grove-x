/*
 * author:charlee
 */
#ifndef _COUPON_HELPER_H_
#define _COUPON_HELPER_H_

#include <thrift/transport/TSocket.h>
#include "JsonHelper.h"
#include "JsonMacro.h"
#include "ThriftHelper.h"
#include "MysqlWrapperPool.h"
#include "RedisWrapperPool.h"
#include "MysqlQueryHelper.h"
#include "sdk-cpp/ec/CouponECService.h"
#include "sdk-cpp/ec/coupon_types.h"
#include "sdk-cpp/ec/couponEC_types.h"
#include "sdk-cpp/admin/couponAdmin_types.h"
#include "CouponCode.h"
#include "CouponUtil.h"
#include "CouponConst.h"

using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace std;
using namespace coupon;
using namespace couponEC;
using namespace couponAdmin;
using namespace couponUtil;

enum USE_STATUS {
    DRAWN = 0,      //已领取
    FROZEN = 1,     //已冻结
    PAYED = 2,      //已支付
    EXPIRED = 3,    //已过期
};

class CouponHelper {
private:
    MysqlWrapper* mwrapper;
    string errmsg ;

public:
    CouponHelper(MysqlWrapper* mwrapper){
        this->mwrapper = mwrapper;
    }

    string getError(){
        return this->errmsg;
    }

    int getWareLabelList(vector<WareLabel>& wareLabelList,const MultipleCondition& cond, unsigned int offset,unsigned int rows){
        MysqlQueryHelper qh(mwrapper);
        int ret = qh.runSelect(string("")
            +" select id,name,scope_type,sub_type,UNIX_TIMESTAMP(create_time),UNIX_TIMESTAMP(update_time),create_man"
            +" from " + ware_label_table
            +" " + getWhereCondition(cond)
            +" " + getOrderByCondition(cond)
            +" limit " + to_string(offset) + "," + to_string(rows)
        );

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql queryCoupons"<< endl;
            LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
            this->errmsg = qh.getError();
            return ret;
        }

        int rows_size = qh.getRows().size();
        for(int row = 0;row<rows_size;row++){
            WareLabel wareLabel;
            wareLabel.ware_label_id = qh.getIntegerField("id",row);
            wareLabel.name          = qh.getStringField("name",row);
            wareLabel.scope_type    = qh.getIntegerField("scope_type",row);
            wareLabel.sub_type      = qh.getIntegerField("sub_type",row);
            wareLabel.create_time   = qh.getIntegerField("UNIX_TIMESTAMP(create_time)",row);
            wareLabel.update_time   = qh.getIntegerField("UNIX_TIMESTAMP(update_time)",row);
            wareLabel.create_man    = qh.getStringField("create_man",row);

            wareLabelList.push_back(wareLabel);
        }

        return 0;
    }

    int queryWareLabels(const set<unsigned int > ware_label_ids,map<unsigned int,WareLabel> &wareLabels){
        this->errmsg = "";
        InvalidOperation io;

        if(ware_label_ids.size( ) == 0){
            LOG_ERROR<<" ware_label_ids.size == 0"<< endl;
            return -1;
        }

        string in = "(";
        for(auto it=ware_label_ids.begin();it != ware_label_ids.end();it++){
            in += to_string(*it);
            auto iter = it;
            iter++;
            if(iter != ware_label_ids.end()){
                in += ",";
            }
        }
        in += ")";

        MysqlQueryHelper qh(mwrapper);
        int ret = qh.runSelect(string("")
            +" select id,name,scope_type,sub_type,UNIX_TIMESTAMP(create_time),UNIX_TIMESTAMP(update_time),create_man"
            +" from " + ware_label_table
            +" where id in " + in
        );

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql exe queryCouponGroupIds"<< endl;
            LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
            this->errmsg = qh.getError();
            return ret;
        }

        if(!qh.hasRow()){
            LOG_ERROR<<"ret="<<ret<<" no rows queryWareLabels"<< endl;
            return ret;
        }

        int rows_size = qh.getRows().size();
        for(int row = 0;row<rows_size;row++){
            WareLabel wareLabel;
            wareLabel.ware_label_id = qh.getIntegerField("id",row);
            wareLabel.name          = qh.getStringField("name",row);
            wareLabel.scope_type    = qh.getIntegerField("scope_type",row);
            wareLabel.sub_type      = qh.getIntegerField("sub_type",row);
            wareLabel.create_time   = qh.getIntegerField("UNIX_TIMESTAMP(create_time)",row);
            wareLabel.update_time   = qh.getIntegerField("UNIX_TIMESTAMP(update_time)",row);
            wareLabel.create_man    = qh.getStringField("create_man",row);

            wareLabels[wareLabel.ware_label_id] = wareLabel;
        }

        return 0;
    }

    int getWareLabelWaresList(vector<WareLabelWares>& wareLabelWaresList,const MultipleCondition& cond, unsigned int offset,unsigned int rows){
        InvalidOperation io;
        MysqlQueryHelper qh(mwrapper);
        int ret = qh.runSelect(string("")
            +" select ware_label_id,ware_id,ware_slug"
            +" from " + ware_label_wares_table
            +" " + getWhereCondition(cond)
            +" " + getOrderByCondition(cond)
            +" limit " + to_string(offset) + "," + to_string(rows)
        );

        if(-9999 == ret ){
            return 0;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql exe ware_label_wares"<< endl;
            LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
            this->errmsg = qh.getError();
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = "error occured when mysql exe ware_label_wares:" + qh.getError();
            throw io;
        }

        int rows_size = qh.getRows().size();
        for(int row = 0;row<rows_size;row++){
            WareLabelWares wareLabelWares;
            wareLabelWares.ware_label_id    = qh.getIntegerField("ware_label_id",row);
            wareLabelWares.ware_id          = qh.getLongIntegerField("ware_id",row);
            wareLabelWares.ware_slug        = qh.getStringField("ware_slug",row);

            wareLabelWaresList.push_back(wareLabelWares);
        }

        return 0;
    }

    int queryWareLableWares(const set<unsigned int> ware_label_ids,map<unsigned int, set<int64_t>>& wareLabelMap){
        this->errmsg = "";
        InvalidOperation io;

        if(ware_label_ids.size( ) == 0){
            LOG_INFO<<" ware_label_ids.size == 0"<< endl;
            return -1;
        }

        string in = "(";
        for(auto it=ware_label_ids.begin();it != ware_label_ids.end();it++){
            in += to_string(*it);
            auto iter = it;
            iter++;
            if(iter != ware_label_ids.end()){
                in += ",";
            }
        }
        in += ")";

        MysqlQueryHelper qh(mwrapper);
        int ret = qh.runSelect(string("")
            +" select ware_label_id,ware_id,ware_slug"
            +" from " + ware_label_wares_table
            +" where ware_label_id in " + in
        );

        //如果全部是通用标签
        if(-9999 == ret){
            return 0;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql exe ware_label_wares"<< endl;
            LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
            this->errmsg = qh.getError();
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = "error occured when mysql exe ware_label_wares:" + qh.getError();
            throw io;
        }

        int rows_size = qh.getRows().size();
        for(int row = 0;row<rows_size;row++){
            wareLabelMap[qh.getIntegerField("ware_label_id",row)].insert(qh.getLongIntegerField("ware_id",row));
        }

        return 0;
    }

    int queryWareLabelIds(const set<int64_t> ware_ids,map<int64_t,vector<CouponGroup> >& wareCouponGroupMap){
        this->errmsg = "";
        InvalidOperation io;

        if(ware_ids.size( ) == 0){
            LOG_ERROR<<" ware_ids.size == 0"<< endl;
            return -1;
        }

        string in = "(";
        for(auto it=ware_ids.begin();it != ware_ids.end();it++){
            in += to_string(*it);
            auto iter = it;
            iter++;
            if(iter != ware_ids.end()){
                in += ",";
            }
        }
        in += ")";

        MysqlQueryHelper qh(mwrapper);
        int ret = qh.runSelect(string("")
            +" select ware_label_id,ware_id,ware_slug"
            +" from " + ware_label_wares_table
            +" where ware_id in " + in
        );

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql exe queryCouponGroupIds"<< endl;
            LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
            this->errmsg = qh.getError();
            return ret;
        }

        set<int> ware_label_ids;
        set<pair<int64_t,int>> ware_label_set;
        int rows_size = qh.getRows().size();
        for(int row = 0;row<rows_size;row++){
            ware_label_ids.insert(qh.getIntegerField("ware_label_id",row));
            ware_label_set.insert(pair<int64_t,int>(qh.getLongIntegerField("ware_id",row),qh.getIntegerField("ware_label_id",row)));
        }

        in = "(";
        for(auto it=ware_label_ids.begin();it != ware_label_ids.end();it++){
            in += to_string(*it);
            auto iter = it;
            iter++;
            if(iter != ware_label_ids.end()){
                in += ",";
            }
        }
        in += ")";

        ret = qh.runSelect(string("")
            +" select id,name,title,comment,ware_label_id,favor_type,"
            +" scope_type,sub_type,scene_type,full,favor,rate,argot,max_count,"
            +" delta,drawn_count,payed_count,UNIX_TIMESTAMP(create_time),can_draw_count,"
            +" is_duration_type,duration_value,"
            +" UNIX_TIMESTAMP(start_use_time),UNIX_TIMESTAMP(end_use_time),"
            +" UNIX_TIMESTAMP(start_draw_time),UNIX_TIMESTAMP(end_draw_time),"
            +" applicant,approver,modifier,seller_id,url,verify_status ,img,slug,"
            +" delta_verify_status,img_width,img_height,button_text,button_jump,"
            +" jump_label,jump_data,argot_jump_label,argot_jump_data,UNIX_TIMESTAMP(update_time),version,description"
            +" from " + coupon_group_table
            +" where ware_label_id in "+in
            +" and verify_status = 2"
        );

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql queryWareLabelIds"<< endl;
            LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
            this->errmsg = qh.getError();
            return ret;
        }

        if(!qh.hasRow()){
            LOG_ERROR<<"ret="<<ret<<" no rows queryWareLabelIds"<< endl;
            return ret;
        }

        rows_size = qh.getRows().size();
        for(int row = 0;row<rows_size;row++){
            CouponGroup couponGroup;
            couponGroup.id              = qh.getIntegerField("id",row);
            couponGroup.name            = qh.getStringField("name",row);
            couponGroup.title           = qh.getStringField("title",row);
            couponGroup.comment         = qh.getStringField("comment",row);
            couponGroup.ware_label_id   = qh.getIntegerField("ware_label_id",row);
            couponGroup.favor_type      = qh.getIntegerField("favor_type",row);
            couponGroup.scope_type      = qh.getIntegerField("scope_type",row);
            couponGroup.sub_type        = qh.getIntegerField("sub_type",row);
            couponGroup.scene_type      = qh.getIntegerField("scene_type",row);
            couponGroup.full            = qh.getDoubleField("full",row);
            couponGroup.favor           = qh.getDoubleField("favor",row);
            couponGroup.rate            = qh.getDoubleField("rate",row);
            couponGroup.argot           = qh.getStringField("argot",row);
            couponGroup.max_count       = qh.getIntegerField("max_count",row);
            couponGroup.drawn_count     = qh.getIntegerField("drawn_count",row);
            couponGroup.payed_count     = qh.getIntegerField("payed_count",row);
            couponGroup.can_draw_count  = qh.getIntegerField("can_draw_count",row);
            couponGroup.is_duration_type= qh.getIntegerField("is_duration_type",row);
            couponGroup.duration_value  = qh.getIntegerField("duration_value",row);
            couponGroup.verify_status   = qh.getIntegerField("verify_status",row);
            couponGroup.applicant       = qh.getStringField("applicant",row);
            couponGroup.approver        = qh.getStringField("approver",row);
            couponGroup.modifier        = qh.getStringField("modifier",row);
            couponGroup.url             = qh.getStringField("url",row);
            couponGroup.seller_id       = qh.getLongIntegerField("seller_id",row);
            couponGroup.id              = qh.getIntegerField("id",row);
            couponGroup.img             = qh.getStringField("img",row);
            couponGroup.slug            = qh.getStringField("slug",row);
            couponGroup.create_time     = qh.getIntegerField("UNIX_TIMESTAMP(create_time)",row);
            couponGroup.start_draw_time = qh.getIntegerField("UNIX_TIMESTAMP(start_draw_time)",row);
            couponGroup.end_draw_time   = qh.getIntegerField("UNIX_TIMESTAMP(end_draw_time)",row);
            couponGroup.start_use_time  = qh.getIntegerField("UNIX_TIMESTAMP(start_use_time)",row);
            couponGroup.end_use_time    = qh.getIntegerField("UNIX_TIMESTAMP(end_use_time)",row);
            couponGroup.img_width       = qh.getIntegerField("img_width",row);
            couponGroup.img_height      = qh.getIntegerField("img_height",row);
            couponGroup.button_text     = qh.getStringField("button_text",row);
            couponGroup.button_jump     = qh.getStringField("button_jump",row);
            couponGroup.jump_label      = qh.getStringField("jump_label",row);
            couponGroup.jump_data       = qh.getStringField("jump_data",row);
            couponGroup.argot_jump_label= qh.getStringField("argot_jump_label",row);
            couponGroup.argot_jump_data = qh.getStringField("argot_jump_data",row);
            couponGroup.delta_verify_status= qh.getIntegerField("delta_verify_status",row);
            couponGroup.update_time     = qh.getIntegerField("UNIX_TIMESTAMP(update_time)",row);
            couponGroup.version         = qh.getIntegerField("version",row);
            couponGroup.description     = qh.getStringField("description",row);

            for(auto x:ware_label_set){
                if(x.second == couponGroup.ware_label_id && 1 != couponGroup.sub_type){//去除黑名单商品
                    wareCouponGroupMap[x.first].push_back(couponGroup);
                }
            }
        }

        return 0;
    }

    int insertUserCoupon(unsigned int coupon_group_id,int64_t user_id,string client_id,unsigned int coupon_id,string code,
            unsigned int channel_id ,int64_t seller_id){
        this->errmsg = "";
        InvalidOperation io;

        MysqlQueryHelper queryHelper(mwrapper);
        int ret = queryHelper.runInsert(string("")
                +" insert into " + user_coupons_table
                +" (user_id,coupon_group_id,coupon_id,code,create_time,update_time,use_status,client_id,channel_id,seller_id) "
                +"values("
                +to_string(user_id)+","
                +to_string(coupon_group_id)+","
                +to_string(coupon_id)+",'"
                +code+"',"
                +"now(),"
                +"now(),"
                +"0,"
                +" '"+client_id+"',"
                +to_string(channel_id)+","
                +to_string(seller_id)
                +")"
        );

        if(ret < 0){
            LOG_ERROR<<" error occured when mysql execute insertUserCoupon"<< endl;
            LOG_ERROR<<"the error detail is that: "<<queryHelper.getError()<<endl;
            this->errmsg = queryHelper.getError();
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = "error occured when mysql execute insertUserCoupon:" + queryHelper.getError();
            throw io;
        }

        return 0;
    }

    int incrDrawnCount(unsigned int coupon_group_id){
        this->errmsg = "";
        InvalidOperation io;
        MysqlQueryHelper queryHelper(mwrapper);
        int ret = queryHelper.runUpdate(string("")
                +" update " + coupon_group_table
                +" set drawn_count = drawn_count + 1"
                +" where id = "+to_string(coupon_group_id)
        );

        if(ret <= 0){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute incrDrawCount"<< endl;
            LOG_ERROR<<"the error detail is that: "<<queryHelper.getError()<<endl;
            this->errmsg = queryHelper.getError();
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = "error occured when mysql execute incrDrawCount" + this->errmsg;
            throw io;
        }

        return 0;
    }

    int getCouponList(vector<Coupon>& couponList,const MultipleCondition& cond,unsigned int coupon_group_id, unsigned int offset,unsigned int rows){
        MysqlQueryHelper qh(mwrapper);
        int ret = qh.runSelect(string("")
            +" select coupon_group_id,id,code, user_id,order_id,"
            +" UNIX_TIMESTAMP(create_time),UNIX_TIMESTAMP(drawn_time),"
            +" UNIX_TIMESTAMP(frozen_time),UNIX_TIMESTAMP(payed_time),"
            +" UNIX_TIMESTAMP(order_create_time)"
            +" from " + coupon_table_prefix + to_string(coupon_group_id)
            +" " + getWhereCondition(cond)
            +" " + getOrderByCondition(cond)
            +" limit " + to_string(offset) + "," + to_string(rows)
        );

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql queryCoupons"<< endl;
            LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
            this->errmsg = qh.getError();
            return ret;
        }

        int rows_size = qh.getRows().size();
        for(int row = 0;row<rows_size;row++){
            Coupon coupon;

            coupon.coupon_group_id  = qh.getIntegerField("coupon_group_id",row);
            coupon.coupon_id        = qh.getIntegerField("id",row);
            coupon.code             = qh.getStringField("code",row);
            coupon.user_id          = qh.getLongIntegerField("user_id",row);
            coupon.order_id         = qh.getStringField("order_id",row);
            coupon.drawn_time       = qh.getIntegerField("UNIX_TIMESTAMP(drawn_time)",row);
            coupon.frozen_time      = qh.getIntegerField("UNIX_TIMESTAMP(frozen_time)",row);
            coupon.payed_time       = qh.getIntegerField("UNIX_TIMESTAMP(payed_time)",row);
            coupon.order_create_time= qh.getIntegerField("UNIX_TIMESTAMP(order_create_time)",row);
            coupon.create_time      = qh.getIntegerField("UNIX_TIMESTAMP(create_time)",row);

            couponList.push_back(coupon);
        }

        return 0;
    }

    //批量查询券2 coupon_group_id --> coupon_id
    int queryCouponsByUnionAll(const set<pair<unsigned int,unsigned int>> union_ids,
            map<pair<unsigned int,unsigned int>,Coupon>& coupons){
        LOG_INFO<<"queryCouponsByUnionAll : union_ids.size() == "<<union_ids.size();
        this->errmsg = "";

        if(union_ids.size() == 0){
            LOG_WARN<<"the error detail is that: union_ids.size() == 0";
            this->errmsg = "union_ids.size() == 0";
            return -1;
        }

        //按券组分类
        map<unsigned int,set<unsigned int>> groups;
        for(auto union_id : union_ids){
            groups[union_id.first].insert(union_id.second);
        }

        string command = "";
        int count = 0;
        for(auto group:groups){
            count++;

            string in = "(";
            for(auto it=group.second.begin();it != group.second.end();it++){
                in += to_string(*it);
                auto iter = it;
                iter++;
                if(iter != group.second.end()){
                    in += ",";
                }
            }
            in += ")";

            command += string("")
                +" select coupon_group_id,id,code, user_id,order_id,"
                +" UNIX_TIMESTAMP(create_time),UNIX_TIMESTAMP(drawn_time),"
                +" UNIX_TIMESTAMP(frozen_time),UNIX_TIMESTAMP(payed_time),"
                +" UNIX_TIMESTAMP(order_create_time)"
                +" from " + coupon_table_prefix + to_string(group.first)
                +" where id in " + in;

            if(count < (int)groups.size()){
                command += " UNION ALL ";
            }
        }

        MysqlQueryHelper qh(mwrapper);
        int ret = qh.runSelectUnionAll(command);
        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql queryCouponsByUnionAll"<< endl;
            LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
            this->errmsg = qh.getError();
            return ret;
        }

        if(!qh.hasRow()){
            LOG_ERROR<<"ret="<<ret<<" no rows queryCouponsByUnionAll";
            this->errmsg = "no rows queryCouponsByUnionAll";
            return ret;
        }

        int rows_size = qh.getRows().size();
        for(int row = 0;row<rows_size;row++){
            Coupon coupon;

            coupon.coupon_group_id  = qh.getIntegerField("coupon_group_id",row);
            coupon.coupon_id        = qh.getIntegerField("id",row);
            coupon.code             = qh.getStringField("code",row);
            coupon.user_id          = qh.getLongIntegerField("user_id",row);
            coupon.order_id         = qh.getStringField("order_id",row);
            coupon.drawn_time       = qh.getIntegerField("UNIX_TIMESTAMP(drawn_time)",row);
            coupon.frozen_time      = qh.getIntegerField("UNIX_TIMESTAMP(frozen_time)",row);
            coupon.payed_time       = qh.getIntegerField("UNIX_TIMESTAMP(payed_time)",row);
            coupon.order_create_time= qh.getIntegerField("UNIX_TIMESTAMP(order_create_time)",row);
            coupon.create_time      = qh.getIntegerField("UNIX_TIMESTAMP(create_time)",row);

            coupons[pair<unsigned int,unsigned int>(coupon.coupon_group_id,coupon.coupon_id)] = coupon;
        }

        return 0;
    }

    //批量查询券 coupon_group_id --> coupon_id
    int queryCoupons(const set<pair<unsigned int,unsigned int>> union_ids,
                    map<pair<unsigned int,unsigned int>,Coupon>& coupons
                    )
    {
        this->errmsg = "";
        //按券组分类
        map<unsigned int,set<unsigned int>> groups;
        for(auto union_id : union_ids){
            groups[union_id.first].insert(union_id.second);
        }

        //分组查询
        for(auto group:groups){
            string in = "(";
            for(auto it=group.second.begin();it != group.second.end();it++){
                in += to_string(*it);
                auto iter = it;
                iter++;
                if(iter != group.second.end()){
                    in += ",";
                }
            }
            in += ")";

            MysqlQueryHelper qh(mwrapper);
            int ret = qh.runSelect(string("")
                +" select coupon_group_id,id,code, user_id,order_id,"
                +" UNIX_TIMESTAMP(create_time),UNIX_TIMESTAMP(drawn_time),"
                +" UNIX_TIMESTAMP(frozen_time),UNIX_TIMESTAMP(payed_time),"
                +" UNIX_TIMESTAMP(order_create_time)"
                +" from " + coupon_table_prefix + to_string(group.first)
                +" where id in " + in
            );

            if(0 != ret){
                LOG_ERROR<<"ret="<<ret<<" error occured when mysql queryCoupons"<< endl;
                LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
                this->errmsg = qh.getError();
                return ret;
            }

            if(!qh.hasRow()){
                LOG_ERROR<<"ret="<<ret<<" no rows queryCouponGroups,couponGroup: "<<group.first<< endl;
                continue;
            }

            int rows_size = qh.getRows().size();
            for(int row = 0;row<rows_size;row++){
                Coupon coupon;

                coupon.coupon_group_id  = qh.getIntegerField("coupon_group_id",row);
                coupon.coupon_id        = qh.getIntegerField("id",row);
                coupon.code             = qh.getStringField("code",row);
                coupon.user_id          = qh.getLongIntegerField("user_id",row);
                coupon.order_id         = qh.getStringField("order_id",row);
                coupon.drawn_time       = qh.getIntegerField("UNIX_TIMESTAMP(drawn_time)",row);
                coupon.frozen_time      = qh.getIntegerField("UNIX_TIMESTAMP(frozen_time)",row);
                coupon.payed_time       = qh.getIntegerField("UNIX_TIMESTAMP(payed_time)",row);
                coupon.order_create_time= qh.getIntegerField("UNIX_TIMESTAMP(order_create_time)",row);
                coupon.create_time      = qh.getIntegerField("UNIX_TIMESTAMP(create_time)",row);

                coupons[pair<unsigned int,unsigned int>(coupon.coupon_group_id,coupon.coupon_id)] = coupon;
            }

        }

        return 0;
    }

    //查询单个券
    int queryCoupon(unsigned int coupon_group_id,unsigned int coupon_id,Coupon & coupon){
        this->errmsg = "";
        MysqlQueryHelper qh(mwrapper);
        int ret = qh.runSelect(string("")
            +" select coupon_group_id,id,code, user_id,order_id,"
            +" UNIX_TIMESTAMP(create_time),UNIX_TIMESTAMP(drawn_time),UNIX_TIMESTAMP(frozen_time),UNIX_TIMESTAMP(payed_time),"
            +" UNIX_TIMESTAMP(order_create_time)"
            +" from " + coupon_table_prefix + to_string(coupon_group_id)
            +" where id = " + to_string(coupon_id)
        );

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql queryCoupon"<< endl;
            LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
            this->errmsg = qh.getError();
            return ret;
        }

        coupon.coupon_group_id  = qh.getIntegerField("coupon_group_id");
        coupon.coupon_id        = qh.getIntegerField("id");
        coupon.code             = qh.getStringField("code");
        coupon.user_id          = qh.getLongIntegerField("user_id");
        coupon.order_id         = qh.getStringField("order_id");
        coupon.drawn_time       = qh.getIntegerField("UNIX_TIMESTAMP(drawn_time)");
        coupon.frozen_time      = qh.getIntegerField("UNIX_TIMESTAMP(frozen_time)");
        coupon.payed_time       = qh.getIntegerField("UNIX_TIMESTAMP(payed_time)");
        coupon.order_create_time= qh.getIntegerField("UNIX_TIMESTAMP(order_create_time)");
        coupon.create_time      = qh.getIntegerField("UNIX_TIMESTAMP(create_time)");

        return 0;
    }

    //新增一条券记录
    int drawOneCoupon(unsigned int coupon_group_id,int64_t user_id ,unsigned int &coupon_id,string& code){
        this->errmsg = "";
        coupon_id = 0;
        code = "";

        InvalidOperation io;
        MysqlQueryHelper queryHelper(this->mwrapper);

        unsigned int insert_id = 0;
        insert_id = queryHelper.runInsert(string("")
            +" insert into " + coupon_table_prefix + to_string(coupon_group_id)
            +" (coupon_group_id,user_id,create_time) "
            +"values("
            +to_string(coupon_group_id)+","
            +to_string(user_id)+","
            +"now()"
            +")"
        );

        if(insert_id <= 0){
            LOG_ERROR<<"insert_id = "<<insert_id<<" error occured when mysql execute drawOneCoupon.."<< endl;
            LOG_ERROR<<"the error detail is that: "<<queryHelper.getError()<<endl;
            this->errmsg = queryHelper.getError();
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = "error occured when mysql execute drawOneCoupon " + this->errmsg;
            throw io;
        }

        coupon_id = insert_id;

        //修改券码code和领取时间drawn_time
        int err = CouponCode::Encode(coupon_group_id,insert_id,code);
        if(err != CouponCode::Errno::OK)
        {
            LOG_ERROR<<"coupon_group_id "<<coupon_group_id<<" coupon_id = "<<insert_id<<" code encode error"<< endl;
            io.fault = Error::CODE_ENCODE_ERROR;
            io.why = "code encode error";
            throw io;
        }

        unsigned int _coupon_group_id, _coupon_id;
        CouponCode::Decode(code,_coupon_group_id,_coupon_id);
        if(_coupon_group_id != coupon_group_id || _coupon_id != insert_id)
        {
            LOG_ERROR<<"coupon_group_id"<<coupon_group_id<<"coupon_id="<<insert_id<<" code decode error"<< endl;
            io.fault = Error::CODE_DECODE_ERROR;
            io.why = "code decode error";
            throw io;
        }

        int ret = queryHelper.runUpdate(string("")
                +" update "+coupon_table_prefix + to_string(coupon_group_id)
                +" set code = '" + code +"'"
                +" ,drawn_time=now()"
                +" where id = "+to_string(insert_id)
        );

        if(ret <= 0){
            LOG_ERROR<<"ret= "<<ret<<" insert_id="<<insert_id<<" error occured when mysql execute.."<< endl;
            LOG_ERROR<<"the error detail is that: "<<queryHelper.getError()<<endl;
            this->errmsg = queryHelper.getError();
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = "error occured when mysql execute" + this->errmsg;
            throw io;
        }

        return 0;
    }

    int createCouponTable(unsigned int coupon_group_id){
        this->errmsg = "";
        //如果对应的券表不存在，则产生一张券表
        /*
        +-------------------+------------------+------+-----+---------+----------------+
        | Field             | Type             | Null | Key | Default | Extra          |
        +-------------------+------------------+------+-----+---------+----------------+
        | id                | int(11)          | NO   | PRI | NULL    | auto_increment |
        | coupon_group_id   | int(10) unsigned | NO   |     | NULL    |                |
        | code              | varchar(64)      | NO   | UNI | NULL    |                |
        | user_id           | bigint(20)       | NO   |     | NULL    |                |
        | order_id          | varchar(200)     | NO   |     | NULL    |                |
        | create_time       | datetime         | NO   |     | NULL    |                |
        | order_create_time | datetime         | YES  |     | NULL    |                |
        | drawn_time        | datetime         | YES  |     | NULL    |                |
        | frozen_time       | datetime         | YES  |     | NULL    |                |
        | payed_time        | datetime         | YES  |     | NULL    |                |
        +-------------------+------------------+------+-----+---------+----------------+
        */
        MysqlQueryHelper queryHelper(this->mwrapper);
        return queryHelper.runCreate(string("")
                +" CREATE TABLE IF NOT EXISTS " + coupon_table_prefix + to_string(coupon_group_id)
                +"("
                +"id int(11) not null primary key auto_increment, "
                +"coupon_group_id int(10) unsigned not null, "
                +"code varchar(64) not null unique, "
                +"user_id bigint(20) unsigned not null, "
                +"order_id varchar(200), "
                +"create_time datetime, "
                +"order_create_time datetime, "
                +"drawn_time datetime, "
                +"frozen_time datetime, "
                +"payed_time datetime"
                +")ENGINE=InnoDB default charset=utf8"
        );
    }

    int userCouponsCount(int64_t user_id ,int& count,unsigned int coupon_group_id = 0){
        this->errmsg = "";

        count = 0;
        MysqlQueryHelper queryHelper(this->mwrapper);
        int ret = queryHelper.runSelect(string("")
            +" select count(*)"
            +" from " + user_coupons_table
            +" where user_id = " + to_string(user_id)
            +  (coupon_group_id == 0?"":(" and coupon_group_id = "+to_string(coupon_group_id)))
        );

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql userCouponsCount"<< endl;
            LOG_ERROR<<"the error detail is that: "<<queryHelper.getError()<<endl;
            this->errmsg = queryHelper.getError();
            return ret;
        }

        count = queryHelper.getIntegerField("count(*)");
        return 0;
    }

    int userCouponsCount(const MultipleCondition& cond,int& count){
        this->errmsg = "";

        count = 0;
        MysqlQueryHelper queryHelper(this->mwrapper);
        int ret = queryHelper.runSelect(string("")
            +" select count(*)"
            +" from " + user_coupons_table
            +" " + getWhereCondition(cond)
            +" " + getOrderByCondition(cond)
        );

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql userCouponsCount"<< endl;
            LOG_ERROR<<"the error detail is that: "<<queryHelper.getError()<<endl;
            this->errmsg = queryHelper.getError();
            return ret;
        }

        count = queryHelper.getIntegerField("count(*)");
        return 0;
    }

    void freezeCoupon(int64_t user_id ,string code){
        this->errmsg = "";
        InvalidOperation io;

        MysqlQueryHelper queryHelper(this->mwrapper);
        int ret = queryHelper.runUpdate(string("")
            +" update " + user_coupons_table
            +" set update_time=now(),use_status = 1"
            +" where user_id = " + to_string(user_id)
            +" and code = '" + code + "'"
        );

        if(ret <= 0){
            LOG_ERROR<<"ret= "<<ret<<" user_id="<<user_id<<" error occured when mysql execute freezeCoupon"<< endl;
            LOG_ERROR<<"the error detail is that: "<<queryHelper.getError()<<endl;
            this->errmsg = queryHelper.getError();
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = "error occured when mysql execute" + this->errmsg ;
            throw io;
        }
    }

    int userCouponStatus(int64_t user_id ,string code){
        this->errmsg = "";
        InvalidOperation io;

        MysqlQueryHelper queryHelper(this->mwrapper);
        int ret = queryHelper.runSelect(string("")
            +" select * from " + user_coupons_table
            +" where user_id = " + to_string(user_id)
            +" and code = '" + code + "'"
        );

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute queryUserCoupon"<< endl;
            LOG_ERROR<<"the error detail is that: "<<queryHelper.getError()<<endl;
            this->errmsg = queryHelper.getError();
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = "error occured when mysql execute queryUserCoupon: " + this->errmsg;
            throw io;
        }

        if(!queryHelper.hasRow()){
            LOG_ERROR<<("user_id:"+to_string(user_id)+" doesn't have code:")<<code<< endl;
            io.fault = Error::USER_HAVENOT_THE_CODE;
            io.why = "the user doesn't have this code";
            throw io;
        }

        return queryHelper.getIntegerField("use_status");
    }

    int queryUserCoupons(const vector<string> & codeList,map<string,UserCoupon>& userCouponMap)
    {
        this->errmsg = "";
        userCouponMap.clear();

        string in = "(";
        for(auto it=codeList.begin();it != codeList.end();it++){
            in += "'" + *it +"'";
            auto iter = it;
            iter++;
            if(iter != codeList.end()){
                in += ",";
            }
        }
        in += ")";

        MysqlQueryHelper queryHelper(this->mwrapper);
        int ret = queryHelper.runSelect(string("")
            +" select user_id,coupon_group_id, coupon_id,code,UNIX_TIMESTAMP(create_time),"
            +" UNIX_TIMESTAMP(update_time),use_status,client_id ,seller_id,channel_id"
            +" from " + user_coupons_table
            +" where code in " + in
        );

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql queryUserCoupons"<< endl;
            LOG_ERROR<<"the error detail is that: "<<queryHelper.getError()<<endl;
            this->errmsg = queryHelper.getError();
            return ret;
        }

        if(!queryHelper.hasRow()){
            LOG_ERROR<<"ret="<<ret<<" no rows queryCouponGroup"<< endl;
            return ret;
        }

        int rows_size = queryHelper.getRows().size();
        for(int row = 0;row<rows_size;row++){
            UserCoupon userCoupon;
            userCoupon.user_id          = queryHelper.getLongIntegerField("user_id",row);
            userCoupon.coupon_group_id  = queryHelper.getIntegerField("coupon_group_id",row);
            userCoupon.coupon_id        = queryHelper.getIntegerField("coupon_id",row);
            userCoupon.code             = queryHelper.getStringField("code",row);
            userCoupon.create_time      = queryHelper.getIntegerField("UNIX_TIMESTAMP(create_time)",row);
            userCoupon.update_time      = queryHelper.getIntegerField("UNIX_TIMESTAMP(update_time)",row);
            userCoupon.use_status       = queryHelper.getIntegerField("use_status",row);
            userCoupon.client_id        = queryHelper.getStringField("client_id",row);
            userCoupon.seller_id        = queryHelper.getLongIntegerField("seller_id",row);
            userCoupon.channel_id       = queryHelper.getIntegerField("channel_id",row);

            userCouponMap[userCoupon.code] = userCoupon;
        }

        return 0;
    }

    int queryUserCoupons(int64_t user_id ,
                        vector<UserCoupon> &userCoupons,
                        int offset=0,
                        unsigned int rows=100,
                        int use_status = -1,
                        unsigned int coupon_group_id = 0)
    {
        this->errmsg = "";
        userCoupons.clear();

        MysqlQueryHelper queryHelper(this->mwrapper);
        int ret = queryHelper.runSelect(string("")
            +" select user_id,coupon_group_id, coupon_id,code,UNIX_TIMESTAMP(create_time),"
            +" UNIX_TIMESTAMP(update_time),use_status,client_id ,seller_id,channel_id"
            +" from " + user_coupons_table
            +" where user_id = " + to_string(user_id)
            +  (coupon_group_id == 0?"":(" and coupon_group_id = "+to_string(coupon_group_id)))
            +  (use_status < 0 ?"":(" and use_status = " + to_string(use_status)))
            +" and UNIX_TIMESTAMP(create_time) > " + to_string(time(NULL) - 365 * 24 * 3600)
            +" order by coupon_group_id desc "
            +" limit "+ to_string(offset)+" ," + to_string(rows)
        );

        if(-9999 == ret){
            LOG_WARN<<"ret="<<ret<<" error occured when mysql queryCouponGroup"<< endl;
            LOG_WARN<<"the error detail is that: "<<queryHelper.getError()<<endl;
            this->errmsg = queryHelper.getError();
            return ret;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql queryCouponGroup"<< endl;
            LOG_ERROR<<"the error detail is that: "<<queryHelper.getError()<<endl;
            this->errmsg = queryHelper.getError();
            return ret;
        }

        if(!queryHelper.hasRow()){
            LOG_ERROR<<"ret="<<ret<<" no rows queryCouponGroup"<< endl;
            return ret;
        }

        int rows_size = queryHelper.getRows().size();
        for(int row = 0;row<rows_size;row++){
            UserCoupon userCoupon;
            userCoupon.user_id          = queryHelper.getLongIntegerField("user_id",row);
            userCoupon.coupon_group_id  = queryHelper.getIntegerField("coupon_group_id",row);
            userCoupon.coupon_id        = queryHelper.getIntegerField("coupon_id",row);
            userCoupon.code             = queryHelper.getStringField("code",row);
            userCoupon.create_time      = queryHelper.getIntegerField("UNIX_TIMESTAMP(create_time)",row);
            userCoupon.update_time      = queryHelper.getIntegerField("UNIX_TIMESTAMP(update_time)",row);
            userCoupon.use_status       = queryHelper.getIntegerField("use_status",row);
            userCoupon.client_id        = queryHelper.getStringField("client_id",row);
            userCoupon.seller_id        = queryHelper.getLongIntegerField("seller_id",row);
            userCoupon.channel_id       = queryHelper.getIntegerField("channel_id",row);

            userCoupons.push_back(userCoupon);

        }
        return 0;
    }

    int queryUserCoupons(const MultipleCondition& cond,vector<UserCoupon> &userCoupons)
    {
        this->errmsg = "";
        userCoupons.clear();

        MysqlQueryHelper queryHelper(this->mwrapper);
        int ret = queryHelper.runSelect(string("")
            +" select user_id,coupon_group_id, coupon_id,code,UNIX_TIMESTAMP(create_time),"
            +" UNIX_TIMESTAMP(update_time),use_status,client_id ,seller_id,channel_id"
            +" from " + user_coupons_table
            +" " + getWhereCondition(cond)
            +" " + getOrderByCondition(cond)
        );

        if(-9999 == ret){
            LOG_WARN<<"ret="<<ret<<" error occured when mysql queryCouponGroup"<< endl;
            LOG_WARN<<"the error detail is that: "<<queryHelper.getError()<<endl;
            this->errmsg = queryHelper.getError();
            return ret;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql queryCouponGroup"<< endl;
            LOG_ERROR<<"the error detail is that: "<<queryHelper.getError()<<endl;
            this->errmsg = queryHelper.getError();
            return ret;
        }

        if(!queryHelper.hasRow()){
            LOG_ERROR<<"ret="<<ret<<" no rows queryCouponGroup"<< endl;
            return ret;
        }

        int rows_size = queryHelper.getRows().size();
        for(int row = 0;row<rows_size;row++){
            UserCoupon userCoupon;
            userCoupon.user_id          = queryHelper.getLongIntegerField("user_id",row);
            userCoupon.coupon_group_id  = queryHelper.getIntegerField("coupon_group_id",row);
            userCoupon.coupon_id        = queryHelper.getIntegerField("coupon_id",row);
            userCoupon.code             = queryHelper.getStringField("code",row);
            userCoupon.create_time      = queryHelper.getIntegerField("UNIX_TIMESTAMP(create_time)",row);
            userCoupon.update_time      = queryHelper.getIntegerField("UNIX_TIMESTAMP(update_time)",row);
            userCoupon.use_status       = queryHelper.getIntegerField("use_status",row);
            userCoupon.client_id        = queryHelper.getStringField("client_id",row);
            userCoupon.seller_id        = queryHelper.getLongIntegerField("seller_id",row);
            userCoupon.channel_id       = queryHelper.getIntegerField("channel_id",row);

            userCoupons.push_back(userCoupon);
        }
        return 0;
    }

    int queryUserCoupons(const MultipleCondition& cond,string limitCond,vector<UserCoupon> &userCoupons)
    {
        this->errmsg = "";
        userCoupons.clear();

        MysqlQueryHelper queryHelper(this->mwrapper);
        int ret = queryHelper.runSelect(string("")
            +" select user_id,coupon_group_id, coupon_id,code,UNIX_TIMESTAMP(create_time),"
            +" UNIX_TIMESTAMP(update_time),use_status,client_id ,seller_id,channel_id"
            +" from " + user_coupons_table
            +" " + getWhereCondition(cond)
            +" " + getOrderByCondition(cond)
            +" " + limitCond
        );

        if(-9999 == ret){
            LOG_WARN<<"ret="<<ret<<" error occured when mysql queryCouponGroup"<< endl;
            LOG_WARN<<"the error detail is that: "<<queryHelper.getError()<<endl;
            this->errmsg = queryHelper.getError();
            return ret;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql queryCouponGroup"<< endl;
            LOG_ERROR<<"the error detail is that: "<<queryHelper.getError()<<endl;
            this->errmsg = queryHelper.getError();
            return ret;
        }

        if(!queryHelper.hasRow()){
            LOG_ERROR<<"ret="<<ret<<" no rows queryCouponGroup"<< endl;
            return ret;
        }

        int rows_size = queryHelper.getRows().size();
        for(int row = 0;row<rows_size;row++){
            UserCoupon userCoupon;
            userCoupon.user_id          = queryHelper.getLongIntegerField("user_id",row);
            userCoupon.coupon_group_id  = queryHelper.getIntegerField("coupon_group_id",row);
            userCoupon.coupon_id        = queryHelper.getIntegerField("coupon_id",row);
            userCoupon.code             = queryHelper.getStringField("code",row);
            userCoupon.create_time      = queryHelper.getIntegerField("UNIX_TIMESTAMP(create_time)",row);
            userCoupon.update_time      = queryHelper.getIntegerField("UNIX_TIMESTAMP(update_time)",row);
            userCoupon.use_status       = queryHelper.getIntegerField("use_status",row);
            userCoupon.client_id        = queryHelper.getStringField("client_id",row);
            userCoupon.seller_id        = queryHelper.getLongIntegerField("seller_id",row);
            userCoupon.channel_id       = queryHelper.getIntegerField("channel_id",row);

            userCoupons.push_back(userCoupon);
        }
        return 0;
    }

    int getCouponGroupList(vector<CouponGroup>& couponGroupList,const MultipleCondition& cond,unsigned int offset,unsigned int rows){
        MysqlQueryHelper qh(this->mwrapper);
        int ret = qh.runSelect(string("")
            +" select id,name,title,comment,ware_label_id,favor_type,"
            +" scope_type,sub_type,scene_type,full,favor,rate,argot,max_count,"
            +" delta,drawn_count,payed_count,UNIX_TIMESTAMP(create_time),can_draw_count,"
            +" is_duration_type,duration_value,"
            +" UNIX_TIMESTAMP(start_use_time),UNIX_TIMESTAMP(end_use_time),"
            +" UNIX_TIMESTAMP(start_draw_time),UNIX_TIMESTAMP(end_draw_time),"
            +" applicant,approver,modifier,seller_id,url,verify_status ,img,slug,"
            +" delta_verify_status,img_width,img_height,button_text,button_jump,"
            +" jump_label,jump_data,argot_jump_label,argot_jump_data,UNIX_TIMESTAMP(update_time),version,description"
            +" from " + coupon_group_table
            +" " + getWhereCondition(cond)
            +" " + getOrderByCondition(cond)
            +" limit " + to_string(offset) + "," + to_string(rows)
        );

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql queryCouponGroups"<< endl;
            LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
            this->errmsg = qh.getError();
            return ret;
        }

        int rows_size = qh.getRows().size();
        for(int row = 0;row<rows_size;row++){
            CouponGroup couponGroup;
            couponGroup.id              = qh.getIntegerField("id",row);
            couponGroup.name            = qh.getStringField("name",row);
            couponGroup.title           = qh.getStringField("title",row);
            couponGroup.comment         = qh.getStringField("comment",row);
            couponGroup.ware_label_id   = qh.getIntegerField("ware_label_id",row);
            couponGroup.favor_type      = qh.getIntegerField("favor_type",row);
            couponGroup.scope_type      = qh.getIntegerField("scope_type",row);
            couponGroup.sub_type        = qh.getIntegerField("sub_type",row);
            couponGroup.scene_type      = qh.getIntegerField("scene_type",row);
            couponGroup.full            = qh.getDoubleField("full",row);
            couponGroup.favor           = qh.getDoubleField("favor",row);
            couponGroup.rate            = qh.getDoubleField("rate",row);
            couponGroup.argot           = qh.getStringField("argot",row);
            couponGroup.max_count       = qh.getIntegerField("max_count",row);
            couponGroup.delta           = qh.getIntegerField("delta",row);
            couponGroup.drawn_count     = qh.getIntegerField("drawn_count",row);
            couponGroup.payed_count     = qh.getIntegerField("payed_count",row);
            couponGroup.can_draw_count  = qh.getIntegerField("can_draw_count",row);
            couponGroup.is_duration_type= qh.getIntegerField("is_duration_type",row);
            couponGroup.duration_value  = qh.getIntegerField("duration_value",row);
            couponGroup.verify_status   = qh.getIntegerField("verify_status",row);
            couponGroup.applicant       = qh.getStringField("applicant",row);
            couponGroup.approver        = qh.getStringField("approver",row);
            couponGroup.modifier        = qh.getStringField("modifier",row);
            couponGroup.seller_id       = qh.getLongIntegerField("seller_id",row);
            couponGroup.url             = qh.getStringField("url",row);
            couponGroup.img             = qh.getStringField("img",row);
            couponGroup.slug            = qh.getStringField("slug",row);
            couponGroup.create_time     = qh.getIntegerField("UNIX_TIMESTAMP(create_time)",row);
            couponGroup.start_draw_time = qh.getIntegerField("UNIX_TIMESTAMP(start_draw_time)",row);
            couponGroup.end_draw_time   = qh.getIntegerField("UNIX_TIMESTAMP(end_draw_time)",row);
            couponGroup.start_use_time  = qh.getIntegerField("UNIX_TIMESTAMP(start_use_time)",row);
            couponGroup.end_use_time    = qh.getIntegerField("UNIX_TIMESTAMP(end_use_time)",row);
            couponGroup.img_width       = qh.getIntegerField("img_width",row);
            couponGroup.img_height      = qh.getIntegerField("img_height",row);
            couponGroup.button_text     = qh.getStringField("button_text",row);
            couponGroup.button_jump     = qh.getStringField("button_jump",row);
            couponGroup.jump_label      = qh.getStringField("jump_label",row);
            couponGroup.jump_data       = qh.getStringField("jump_data",row);
            couponGroup.argot_jump_label= qh.getStringField("argot_jump_label",row);
            couponGroup.argot_jump_data = qh.getStringField("argot_jump_data",row);
            couponGroup.delta_verify_status= qh.getIntegerField("delta_verify_status",row);
            couponGroup.update_time     = qh.getIntegerField("UNIX_TIMESTAMP(update_time)",row);
            couponGroup.version         = qh.getIntegerField("version",row);
            couponGroup.description     = qh.getStringField("description",row);

            couponGroupList.push_back(couponGroup);
        }

        return 0;
    }

    //批量查询券组
    int queryCouponGroups(const set<int> coupon_group_ids,map<int,CouponGroup>& couponGroups){
        if(0 == coupon_group_ids.size()){
            return 0;
        }

        this->errmsg = "";

        string in = "(";
        for(auto it=coupon_group_ids.begin();it != coupon_group_ids.end();it++){
            in += to_string(*it);
            auto iter = it;
            iter++;
            if(iter != coupon_group_ids.end()){
                in += ",";
            }
        }
        in += ")";

        MysqlQueryHelper qh(this->mwrapper);
        int ret = qh.runSelect(string("")
            +" select id,name,title,comment,ware_label_id,favor_type,"
            +" scope_type,sub_type,scene_type,full,favor,rate,argot,max_count,"
            +" delta,drawn_count,payed_count,UNIX_TIMESTAMP(create_time),can_draw_count,"
            +" is_duration_type,duration_value,"
            +" UNIX_TIMESTAMP(start_use_time),UNIX_TIMESTAMP(end_use_time),"
            +" UNIX_TIMESTAMP(start_draw_time),UNIX_TIMESTAMP(end_draw_time),"
            +" applicant,approver,modifier,seller_id,url,verify_status ,img,slug,"
            +" delta_verify_status,img_width,img_height,button_text,button_jump,"
            +" jump_label,jump_data,argot_jump_label,argot_jump_data,UNIX_TIMESTAMP(update_time),version,description"
            +" from " + coupon_group_table
            +" where id in " + in
            +" and verify_status = 2"
        );

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql queryCouponGroups"<< endl;
            LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
            this->errmsg = qh.getError();
            return ret;
        }

        if(!qh.hasRow()){
            LOG_ERROR<<"ret="<<ret<<" no rows queryCouponGroups"<< endl;
            return ret;
        }

        int rows_size = qh.getRows().size();
        for(int row = 0;row<rows_size;row++){
            CouponGroup couponGroup;
            couponGroup.id              = qh.getIntegerField("id",row);
            couponGroup.name            = qh.getStringField("name",row);
            couponGroup.title           = qh.getStringField("title",row);
            couponGroup.comment         = qh.getStringField("comment",row);
            couponGroup.ware_label_id   = qh.getIntegerField("ware_label_id",row);
            couponGroup.favor_type      = qh.getIntegerField("favor_type",row);
            couponGroup.scope_type      = qh.getIntegerField("scope_type",row);
            couponGroup.sub_type        = qh.getIntegerField("sub_type",row);
            couponGroup.scene_type      = qh.getIntegerField("scene_type",row);
            couponGroup.full            = qh.getDoubleField("full",row);
            couponGroup.favor           = qh.getDoubleField("favor",row);
            couponGroup.rate            = qh.getDoubleField("rate",row);
            couponGroup.argot           = qh.getStringField("argot",row);
            couponGroup.max_count       = qh.getIntegerField("max_count",row);
            couponGroup.delta           = qh.getIntegerField("delta",row);
            couponGroup.drawn_count     = qh.getIntegerField("drawn_count",row);
            couponGroup.payed_count     = qh.getIntegerField("payed_count",row);
            couponGroup.can_draw_count  = qh.getIntegerField("can_draw_count",row);
            couponGroup.is_duration_type= qh.getIntegerField("is_duration_type",row);
            couponGroup.duration_value  = qh.getIntegerField("duration_value",row);
            couponGroup.verify_status   = qh.getIntegerField("verify_status",row);
            couponGroup.applicant       = qh.getStringField("applicant",row);
            couponGroup.approver        = qh.getStringField("approver",row);
            couponGroup.modifier        = qh.getStringField("modifier",row);
            couponGroup.url             = qh.getStringField("url",row);
            couponGroup.seller_id       = qh.getLongIntegerField("seller_id",row);
            couponGroup.img             = qh.getStringField("img",row);
            couponGroup.slug            = qh.getStringField("slug",row);
            couponGroup.create_time     = qh.getIntegerField("UNIX_TIMESTAMP(create_time)",row);
            couponGroup.start_draw_time = qh.getIntegerField("UNIX_TIMESTAMP(start_draw_time)",row);
            couponGroup.end_draw_time   = qh.getIntegerField("UNIX_TIMESTAMP(end_draw_time)",row);
            couponGroup.start_use_time  = qh.getIntegerField("UNIX_TIMESTAMP(start_use_time)",row);
            couponGroup.end_use_time    = qh.getIntegerField("UNIX_TIMESTAMP(end_use_time)",row);
            couponGroup.img_width       = qh.getIntegerField("img_width",row);
            couponGroup.img_height      = qh.getIntegerField("img_height",row);
            couponGroup.button_text     = qh.getStringField("button_text",row);
            couponGroup.button_jump     = qh.getStringField("button_jump",row);
            couponGroup.jump_label      = qh.getStringField("jump_label",row);
            couponGroup.jump_data       = qh.getStringField("jump_data",row);
            couponGroup.argot_jump_label= qh.getStringField("argot_jump_label",row);
            couponGroup.argot_jump_data = qh.getStringField("argot_jump_data",row);
            couponGroup.delta_verify_status= qh.getIntegerField("delta_verify_status",row);
            couponGroup.update_time     = qh.getIntegerField("UNIX_TIMESTAMP(update_time)",row);
            couponGroup.version         = qh.getIntegerField("version",row);
            couponGroup.description     = qh.getStringField("description",row);

            couponGroups[couponGroup.id ] = couponGroup;
        }

        return 0;
    }

    //查询单个券组
    int queryCouponGroup(int coupon_group_id ,CouponGroup &couponGroup){
        this->errmsg = "";

        MysqlQueryHelper qh(this->mwrapper);
        int ret = qh.runSelect(string("")
            +" select id,name,title,comment,ware_label_id,favor_type,"
            +" scope_type,sub_type,scene_type,full,favor,rate,argot,max_count,"
            +" delta,drawn_count,payed_count,UNIX_TIMESTAMP(create_time),can_draw_count,"
            +" is_duration_type,duration_value,"
            +" UNIX_TIMESTAMP(start_use_time),UNIX_TIMESTAMP(end_use_time),"
            +" UNIX_TIMESTAMP(start_draw_time),UNIX_TIMESTAMP(end_draw_time),"
            +" applicant,approver,modifier,seller_id,url,verify_status ,img,slug,"
            +" delta_verify_status,img_width,img_height,button_text,button_jump,"
            +" jump_label,jump_data,argot_jump_label,argot_jump_data,UNIX_TIMESTAMP(update_time),version,description"
            +" from " + coupon_group_table
            +" where id = "+to_string(coupon_group_id)
            +" and verify_status = 2"
        );

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql queryCouponGroup"<< endl;
            LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
            this->errmsg = qh.getError();
            return ret;
        }

        if(!qh.hasRow()){
            LOG_ERROR<<"ret="<<ret<<" no rows queryCouponGroup"<< endl;
            return ret;
        }

        couponGroup.id              = qh.getIntegerField("id");
        couponGroup.name            = qh.getStringField("name");
        couponGroup.title           = qh.getStringField("title");
        couponGroup.comment         = qh.getStringField("comment");
        couponGroup.ware_label_id   = qh.getIntegerField("ware_label_id");
        couponGroup.favor_type      = qh.getIntegerField("favor_type");
        couponGroup.scope_type      = qh.getIntegerField("scope_type");
        couponGroup.sub_type        = qh.getIntegerField("sub_type");
        couponGroup.scene_type      = qh.getIntegerField("scene_type");
        couponGroup.full            = qh.getDoubleField("full");
        couponGroup.favor           = qh.getDoubleField("favor");
        couponGroup.rate            = qh.getDoubleField("rate");
        couponGroup.argot           = qh.getStringField("argot");
        couponGroup.max_count       = qh.getIntegerField("max_count");
        couponGroup.drawn_count     = qh.getIntegerField("drawn_count");
        couponGroup.payed_count     = qh.getIntegerField("payed_count");
        couponGroup.can_draw_count  = qh.getIntegerField("can_draw_count");
        couponGroup.is_duration_type= qh.getIntegerField("is_duration_type");
        couponGroup.duration_value  = qh.getIntegerField("duration_value");
        couponGroup.verify_status   = qh.getIntegerField("verify_status");
        couponGroup.applicant       = qh.getStringField("applicant");
        couponGroup.approver        = qh.getStringField("approver");
        couponGroup.modifier        = qh.getStringField("modifier");
        couponGroup.url             = qh.getStringField("url");
        couponGroup.seller_id       = qh.getLongIntegerField("seller_id");
        couponGroup.id              = qh.getIntegerField("id");
        couponGroup.img             = qh.getStringField("img");
        couponGroup.slug            = qh.getStringField("slug");
        couponGroup.create_time     = qh.getIntegerField("UNIX_TIMESTAMP(create_time)");
        couponGroup.start_draw_time = qh.getIntegerField("UNIX_TIMESTAMP(start_draw_time)");
        couponGroup.end_draw_time   = qh.getIntegerField("UNIX_TIMESTAMP(end_draw_time)");
        couponGroup.start_use_time  = qh.getIntegerField("UNIX_TIMESTAMP(start_use_time)");
        couponGroup.end_use_time    = qh.getIntegerField("UNIX_TIMESTAMP(end_use_time)");
        couponGroup.img_width       = qh.getIntegerField("img_width");
        couponGroup.img_height      = qh.getIntegerField("img_height");
        couponGroup.button_text     = qh.getStringField("button_text");
        couponGroup.button_jump     = qh.getStringField("button_jump");
        couponGroup.jump_label      = qh.getStringField("jump_label");
        couponGroup.jump_data       = qh.getStringField("jump_data");
        couponGroup.argot_jump_label= qh.getStringField("argot_jump_label");
        couponGroup.argot_jump_data = qh.getStringField("argot_jump_data");
        couponGroup.delta_verify_status= qh.getIntegerField("delta_verify_status");
        couponGroup.update_time     = qh.getIntegerField("UNIX_TIMESTAMP(update_time)");
        couponGroup.version         = qh.getIntegerField("version");
        couponGroup.description     = qh.getStringField("description");

        return 0;
    }

    int getCouponGroup(int coupon_group_id ,CouponGroup &couponGroup){
        this->errmsg = "";

        MysqlQueryHelper qh(this->mwrapper);
        int ret = qh.runSelect(string("")
            +" select id,name,title,comment,ware_label_id,favor_type,"
            +" scope_type,sub_type,scene_type,full,favor,rate,argot,max_count,"
            +" delta,drawn_count,payed_count,UNIX_TIMESTAMP(create_time),can_draw_count,"
            +" is_duration_type,duration_value,"
            +" UNIX_TIMESTAMP(start_use_time),UNIX_TIMESTAMP(end_use_time),"
            +" UNIX_TIMESTAMP(start_draw_time),UNIX_TIMESTAMP(end_draw_time),"
            +" applicant,approver,modifier,seller_id,url,verify_status ,img,slug,"
            +" delta_verify_status,img_width,img_height,button_text,button_jump,"
            +" jump_label,jump_data,argot_jump_label,argot_jump_data,UNIX_TIMESTAMP(update_time),version,description"
            +" from " + coupon_group_table
            +" where id = "+to_string(coupon_group_id)
        );

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql queryCouponGroup"<< endl;
            LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
            this->errmsg = qh.getError();
            return ret;
        }

        if(!qh.hasRow()){
            LOG_ERROR<<"ret="<<ret<<" no rows queryCouponGroup"<< endl;
            return ret;
        }

        couponGroup.id              = qh.getIntegerField("id");
        couponGroup.name            = qh.getStringField("name");
        couponGroup.title           = qh.getStringField("title");
        couponGroup.comment         = qh.getStringField("comment");
        couponGroup.ware_label_id   = qh.getIntegerField("ware_label_id");
        couponGroup.favor_type      = qh.getIntegerField("favor_type");
        couponGroup.scope_type      = qh.getIntegerField("scope_type");
        couponGroup.sub_type        = qh.getIntegerField("sub_type");
        couponGroup.scene_type      = qh.getIntegerField("scene_type");
        couponGroup.full            = qh.getDoubleField("full");
        couponGroup.favor           = qh.getDoubleField("favor");
        couponGroup.rate            = qh.getDoubleField("rate");
        couponGroup.argot           = qh.getStringField("argot");
        couponGroup.max_count       = qh.getIntegerField("max_count");
        couponGroup.drawn_count     = qh.getIntegerField("drawn_count");
        couponGroup.payed_count     = qh.getIntegerField("payed_count");
        couponGroup.can_draw_count  = qh.getIntegerField("can_draw_count");
        couponGroup.is_duration_type= qh.getIntegerField("is_duration_type");
        couponGroup.duration_value  = qh.getIntegerField("duration_value");
        couponGroup.verify_status   = qh.getIntegerField("verify_status");
        couponGroup.applicant       = qh.getStringField("applicant");
        couponGroup.approver        = qh.getStringField("approver");
        couponGroup.modifier        = qh.getStringField("modifier");
        couponGroup.url             = qh.getStringField("url");
        couponGroup.seller_id       = qh.getLongIntegerField("seller_id");
        couponGroup.id              = qh.getIntegerField("id");
        couponGroup.img             = qh.getStringField("img");
        couponGroup.slug            = qh.getStringField("slug");
        couponGroup.create_time     = qh.getIntegerField("UNIX_TIMESTAMP(create_time)");
        couponGroup.start_draw_time = qh.getIntegerField("UNIX_TIMESTAMP(start_draw_time)");
        couponGroup.end_draw_time   = qh.getIntegerField("UNIX_TIMESTAMP(end_draw_time)");
        couponGroup.start_use_time  = qh.getIntegerField("UNIX_TIMESTAMP(start_use_time)");
        couponGroup.end_use_time    = qh.getIntegerField("UNIX_TIMESTAMP(end_use_time)");
        couponGroup.img_width       = qh.getIntegerField("img_width");
        couponGroup.img_height      = qh.getIntegerField("img_height");
        couponGroup.button_text     = qh.getStringField("button_text");
        couponGroup.button_jump     = qh.getStringField("button_jump");
        couponGroup.jump_label      = qh.getStringField("jump_label");
        couponGroup.jump_data       = qh.getStringField("jump_data");
        couponGroup.argot_jump_label= qh.getStringField("argot_jump_label");
        couponGroup.argot_jump_data = qh.getStringField("argot_jump_data");
        couponGroup.delta_verify_status= qh.getIntegerField("delta_verify_status");
        couponGroup.update_time     = qh.getIntegerField("UNIX_TIMESTAMP(update_time)");
        couponGroup.version         = qh.getIntegerField("version");
        couponGroup.description     = qh.getStringField("description");

        return 0;
    }
};

#endif
