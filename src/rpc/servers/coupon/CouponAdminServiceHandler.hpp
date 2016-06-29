/*
 * author:charlee
 */

#include "JsonHelper.h"
#include "JsonMacro.h"
#include "ThriftHelper.h"
#include "MysqlWrapperPool.h"
#include "RedisWrapperPool.h"
#include "MysqlQueryHelper.h"
#include "sdk-cpp/admin/CouponAdminService.h"
#include "CouponCode.h"
#include "CouponHelper.h"
#include "CouponConfig.h"
#include "Singleton.h"
#include "System.h"
#include "CouponUtil.h"

using namespace std;
using namespace coupon;
using namespace couponAdmin;
using namespace System;

class CouponAdminServiceHandler : public CouponAdminServiceIf {
private:
    //<coupon_group_id, coupon_id>
    using union_id_type = pair<unsigned int,unsigned int>;
    MysqlWrapperPool * mpool;
    RedisWrapperPool * rpool;

    string  redis_ip_  ;
    int     redis_port_  ;
    int     redis_timeout_  ;
    int     redis_poolsize_ ;
public:
    CouponAdminServiceHandler() {
        redis_ip_       = couponConfig.getRedisIp();
        redis_port_     = couponConfig.getRedisPort();
        redis_timeout_  = couponConfig.getRedisTimeout();
        redis_poolsize_ = 4;

        string  mysql_ip_       = couponConfig.getMysqlIp();
        int     mysql_port_     = couponConfig.getMysqlPort();
        string  mysql_user_     = couponConfig.getMysqlUser();
        string  mysql_passwd_   = couponConfig.getMysqlPasswd();
        string  mysql_database_ = couponConfig.getMysqlDatabase();
        int  mysql_poolsize_    = 4;

        mpool = MysqlWrapperPool::GetInstance(mysql_ip_,mysql_port_,mysql_user_,mysql_passwd_,mysql_database_,mysql_poolsize_);
        rpool = RedisWrapperPool::GetInstance(redis_ip_,redis_port_,redis_timeout_,redis_poolsize_);
    }

    void updateDefaultConfig(UpdateDefaultConfigRsp& updateDefaultConfigRsp,const UpdateDefaultConfigReq& updateDefaultConfigReq){
        LOG_WARN<<"updateDefaultConfig request is: UpdateDefaultConfigReq '"<<ThriftToJSON(updateDefaultConfigReq)<<"'"<<endl;
        PerfWatch perfWatch("updateDefaultConfig");
        InvalidOperation io;

        //如果获取的Redis连接断开
        LOG_INFO <<"rpool zise:" <<rpool->Size();
        RedisWrapperGuard rwrapperGuard(rpool,rpool->GetRedisWrapper());
        RedisWrapper* rwrapper = rwrapperGuard.getRedisWrapper();
        LOG_INFO <<"rpool zise:" <<rpool->Size()<<" rwrapper:"<<rwrapper;
        if(NULL == rwrapper||rwrapper->isClosed()){
            LOG_ERROR<<"Redis connection disconnected "<<rwrapper->getError()<< endl;
            io.fault = Error::REDIS_DISCONNECTED;
            io.why = "Redis connection disconnected";
            throw io;
        }

        int ret = rwrapper->setValue(coupon_default_config,ThriftToJSON(updateDefaultConfigReq.config));
        if(0 != ret){
            LOG_ERROR<<"error occured when redis execute"<< endl;
            updateDefaultConfigRsp.error = coupon::Error::FAILED,
            updateDefaultConfigRsp.errmsg = "failed";
            return;
        }

        updateDefaultConfigRsp.error = coupon::Error::OK,
        updateDefaultConfigRsp.errmsg = "success";

        LOG_WARN<<"updateDefaultConfig response is: updateDefaultConfigRsp '"<<ThriftToJSON(updateDefaultConfigRsp)<<"'"<<endl;
        return;
    }

    void viewDefaultConfig(ViewDefaultConfigRsp& viewDefaultConfigRsp,const ViewDefaultConfigReq& viewDefaultConfigReq){
        LOG_WARN<<"viewDefaultConfig request is: ViewDefaultConfigReq '"<<ThriftToJSON(viewDefaultConfigReq)<<"'"<<endl;
        PerfWatch perfWatch("viewDefaultConfig");
        InvalidOperation io;

        //如果获取的Redis连接断开
        LOG_INFO <<"rpool zise:" <<rpool->Size();
        RedisWrapperGuard rwrapperGuard(rpool,rpool->GetRedisWrapper());
        RedisWrapper* rwrapper = rwrapperGuard.getRedisWrapper();
        LOG_INFO <<"rpool zise:" <<rpool->Size()<<" rwrapper:"<<rwrapper;
        if(NULL == rwrapper||rwrapper->isClosed()){
            LOG_ERROR<<"Redis connection disconnected "<<rwrapper->getError()<< endl;
            io.fault = Error::REDIS_DISCONNECTED;
            io.why = "Redis connection disconnected";
            throw io;
        }

        string json_config = "";
        int ret = rwrapper->getValue(coupon_default_config,json_config);
        if(0 != ret){
            LOG_ERROR<<"error occured when redis execute"<< endl;
            viewDefaultConfigRsp.error = coupon::Error::FAILED,
            viewDefaultConfigRsp.errmsg = "failed";
            return;
        }

        DefaultConfig defaultConfig;
        JSONToThrift(json_config,&defaultConfig);

        viewDefaultConfigRsp.error = coupon::Error::OK,
        viewDefaultConfigRsp.errmsg = "success";
        viewDefaultConfigRsp.config = defaultConfig;

        LOG_WARN<<"viewDefaultConfig response is: viewDefaultConfigRsp '"<<ThriftToJSON(viewDefaultConfigRsp)<<"'"<<endl;
        return;
    }

    void offline(OfflineRsp& offlineRsp,const OfflineReq& offlineReq){
        LOG_WARN<<"offline request is: OfflineReq '"<<ThriftToJSON(offlineReq)<<"'"<<endl;
        PerfWatch perfWatch("offline");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        MysqlQueryHelper queryHelper(mwrapper);
        CouponHelper couponHelper(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
           LOG_ERROR<<"mysql connection disconnected"<< endl;
           io.fault = Error::MYSQL_DISCONNECTED;
           io.why = "mysql connection disconnected";
           throw io;
        }

        //校验组ID,如果这个组在表中不存在
        CouponGroup couponGroup;
        int ret = couponHelper.queryCouponGroup(offlineReq.coupon_group_id,couponGroup);
        if(-9999 == ret){
            LOG_ERROR<<"ret="<<ret<<" queryCouponGroup no any expected data!"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            offlineRsp.error = Error::COUPON_GROUP_NOT_EXISTS;
            offlineRsp.errmsg = "no coupon group you expected";
            return;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            offlineRsp.error = Error::MYSQL_EXECUTE_ERROR;
            offlineRsp.errmsg = couponHelper.getError();
            return;
        }

        ret = queryHelper.runUpdate(string("")
            +" update " + coupon_group_table
            +" set max_count = " + to_string(couponGroup.drawn_count)

            +" ,update_time = now()"
            +" ,version = version + 1"
            +" where id = "+to_string(offlineReq.coupon_group_id)
        );

        if(ret < 0){
            LOG_ERROR<<"ret= "<<ret<<" id="<<offlineReq.coupon_group_id<<" error occured when mysql execute.."<< endl;
            LOG_ERROR<<"the error detail is that: "<<queryHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = queryHelper.getError();
            throw io;
        }

        offlineRsp.error = coupon::Error::OK,
        offlineRsp.errmsg = "success";

        LOG_WARN<<"offline response is: offlineRsp '"<<ThriftToJSON(offlineRsp)<<"'"<<endl;
        return;
    }

    void updateJump(UpdateJumpRsp& updateJumpRsp,const UpdateJumpReq& updateJumpReq){
        LOG_WARN<<"updateJump request is: UpdateJumpReq '"<<ThriftToJSON(updateJumpReq)<<"'"<<endl;
        PerfWatch perfWatch("updateJump");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        MysqlQueryHelper queryHelper(mwrapper);
        CouponHelper couponHelper(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        int ret = queryHelper.runUpdate(string("")
            +" update " + coupon_group_table
            +" set name = '" + updateJumpReq.name + "'"
            +" ,jump_data = '" + updateJumpReq.jump_data + "'"
            +" ,jump_label = '" + updateJumpReq.jump_label + "'"
            +" ,update_time = now()"
            +" ,version = version + 1"
            +" where id = "+to_string(updateJumpReq.coupon_group_id)
        );

        if(ret < 0){
            LOG_ERROR<<"ret= "<<ret<<" id="<<updateJumpReq.coupon_group_id<<" error occured when mysql execute.."<< endl;
            LOG_ERROR<<"the error detail is that: "<<queryHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = queryHelper.getError();
            throw io;
        }

        updateJumpRsp.error = coupon::Error::OK,
        updateJumpRsp.errmsg = "success";

        LOG_WARN<<"updateJump response is: UpdateJumpRsp '"<<ThriftToJSON(updateJumpRsp)<<"'"<<endl;
        return;
    }

    void updateArgotJump(UpdateArgotJumpRsp& updateArgotJumpRsp,const UpdateArgotJumpReq& updateArgotJumpReq){
        LOG_WARN<<"updateArgotJump request is: UpdateArgotJumpReq '"<<ThriftToJSON(updateArgotJumpReq)<<"'"<<endl;
        PerfWatch perfWatch("updateArgotJump");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        MysqlQueryHelper queryHelper(mwrapper);
        CouponHelper couponHelper(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        int ret = queryHelper.runUpdate(string("")
            +" update " + coupon_group_table
            +" set name = '" + updateArgotJumpReq.name + "'"
            +" ,img = '" + updateArgotJumpReq.img + "'"
            +" ,img_width = " + to_string(updateArgotJumpReq.img_width)
            +" ,img_height = " + to_string(updateArgotJumpReq.img_height)
            +" ,button_text = '" + updateArgotJumpReq.button_text + "'"
            +" ,argot_jump_label = '" + updateArgotJumpReq.argot_jump_label + "'"
            +" ,argot_jump_data = '" + updateArgotJumpReq.argot_jump_data + "'"
            +" ,update_time = now()"
            +" ,version = version + 1"
            +" where id = "+to_string(updateArgotJumpReq.coupon_group_id)
        );

        if(ret < 0){
            LOG_ERROR<<"ret= "<<ret<<" id="<<updateArgotJumpReq.coupon_group_id<<" error occured when mysql execute.."<< endl;
            LOG_ERROR<<"the error detail is that: "<<queryHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = queryHelper.getError();
            throw io;
        }

        updateArgotJumpRsp.error = coupon::Error::OK,
        updateArgotJumpRsp.errmsg = "success";

        LOG_WARN<<"updateArgotJump response is: UpdateArgotJumpRsp '"<<ThriftToJSON(updateArgotJumpRsp)<<"'"<<endl;
        return;
    }

    void updateOnline(UpdateOnlineRsp& updateOnlineRsp,const UpdateOnlineReq& updateOnlineReq){
        LOG_WARN<<"updateOnline request is: UpdateOnlineReq '"<<ThriftToJSON(updateOnlineReq)<<"'"<<endl;
        PerfWatch perfWatch("updateOnline");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        MysqlQueryHelper queryHelper(mwrapper);
        CouponHelper couponHelper(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        //校验参数
        if(updateOnlineReq.name.empty() || updateOnlineReq.title.empty() || updateOnlineReq.can_draw_count > 30){
            updateOnlineRsp.error = Error::INVALID_PARAMS;
            updateOnlineRsp.errmsg = "invalid params";
            return;
        }

        //校验组ID,如果这个组在表中不存在
        CouponGroup couponGroup;
        int ret = couponHelper.queryCouponGroup(updateOnlineReq.coupon_group_id,couponGroup);
        if(-9999 == ret){
            LOG_ERROR<<"ret="<<ret<<" queryCouponGroup no any expected data!"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            updateOnlineRsp.error = Error::COUPON_GROUP_NOT_EXISTS;
            updateOnlineRsp.errmsg = "no coupon group you expected";
            return;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            updateOnlineRsp.error = Error::MYSQL_EXECUTE_ERROR;
            updateOnlineRsp.errmsg = couponHelper.getError();
            return;
        }

        if(1 == couponGroup.scene_type){
            //校验口令
            VerifyArgotReq verifyArgotReq;
            verifyArgotReq.coupon_group_id = updateOnlineReq.coupon_group_id;
            verifyArgotReq.start_draw_time = couponGroup.start_draw_time;
            verifyArgotReq.argot = updateOnlineReq.argot;

            VerifyArgotRsp verifyArgotRsp;
            verifyArgot(verifyArgotRsp,verifyArgotReq);
            if(coupon::Error::OK != verifyArgotRsp.error){
                updateOnlineRsp.error = verifyArgotRsp.error;
                updateOnlineRsp.errmsg = verifyArgotRsp.errmsg;
                return;
            }
        }

        //领取结束时间不能缩短
        if(updateOnlineReq.end_draw_time < couponGroup.end_draw_time){
            LOG_ERROR<<"ret="<<ret<<"updateOnlineReq.end_draw_time < couponGroup.end_draw_time"<< endl;
            updateOnlineRsp.error = Error::FAILED;
            updateOnlineRsp.errmsg = "updateOnlineReq.end_draw_time < couponGroup.end_draw_time";
            return;
        }

        //领取过期的券组不能再修改
        if(time(NULL) > couponGroup.end_draw_time){
            updateOnlineRsp.error = Error::DRAW_TIME_EXPIRED;
            updateOnlineRsp.errmsg = "coupon group has been expired (draw)";
            return;
        }

        int can_draw_count = updateOnlineReq.can_draw_count;
        if(updateOnlineReq.can_draw_count > 30 || updateOnlineReq.can_draw_count <= 0){
            can_draw_count = 1;
        }

        ret = queryHelper.runUpdate(string("")
            +" update " + coupon_group_table
            +" set name = '" + updateOnlineReq.name + "'"
            +" ,title = '" + updateOnlineReq.title + "'"
            +" ,can_draw_count = " + to_string(can_draw_count)
            +" ,argot = '" + updateOnlineReq.argot + "'"
            +" ,description = '" + updateOnlineReq.description + "'"
            +" ,end_draw_time = " + "FROM_UNIXTIME(" + to_string(updateOnlineReq.end_draw_time) + ")"
            +" ,update_time = now()"
            +" ,version = version + 1"
            +" where id = "+to_string(updateOnlineReq.coupon_group_id)
        );

        if(ret < 0){
            LOG_ERROR<<"ret= "<<ret<<" id="<<updateOnlineReq.coupon_group_id<<" error occured when mysql execute.."<< endl;
            LOG_ERROR<<"the error detail is that: "<<queryHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = queryHelper.getError();
            throw io;
        }

        updateOnlineRsp.error = coupon::Error::OK,
        updateOnlineRsp.errmsg = "success";

        LOG_WARN<<"updateOnline response is: UpdateOnlineRsp '"<<ThriftToJSON(updateOnlineRsp)<<"'"<<endl;
        return;
    }

    void batchExportCode(BatchExportCodeRsp& batchExportCodeRsp,const BatchExportCodeReq& batchExportCodeReq){
        LOG_WARN<<"batchExportCode request is: BatchExportCodeReq '"<<ThriftToJSON(batchExportCodeReq)<<"'"<<endl;
        PerfWatch perfWatch("batchExportCode");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        MysqlQueryHelper queryHelper(mwrapper);
        CouponHelper couponHelper(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        //校验组ID,如果这个组在表中不存在
        CouponGroup couponGroup;
        int ret = couponHelper.queryCouponGroup(batchExportCodeReq.coupon_group_id,couponGroup);
        if(-9999 == ret){
            LOG_ERROR<<"ret="<<ret<<" queryCouponGroup no any expected data!"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            batchExportCodeRsp.error = Error::COUPON_GROUP_NOT_EXISTS;
            batchExportCodeRsp.errmsg = "no coupon group you expected";
            return;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            batchExportCodeRsp.error = Error::MYSQL_EXECUTE_ERROR;
            batchExportCodeRsp.errmsg = couponHelper.getError();
            return;
        }

        CouponCountReq couponCountReq;
        couponCountReq.coupon_group_id = batchExportCodeReq.coupon_group_id;
        CouponCountRsp couponCountRsp;
        getCouponCount(couponCountRsp,couponCountReq);

        if(batchExportCodeReq.page <= 0 || batchExportCodeReq.count <= 0
            ||((batchExportCodeReq.page - 1) * batchExportCodeReq.count + 1 > couponCountRsp.count)){
            LOG_ERROR<<"invalid params:"<<" page:"<<batchExportCodeReq.page<<" count:"<<batchExportCodeReq.count<< endl;
            batchExportCodeRsp.error = Error::INVALID_PARAMS;
            batchExportCodeRsp.errmsg = "invalid params";
            return;
        }

        int page = batchExportCodeReq.page;
        int count = batchExportCodeReq.count;
        if(count > 1000){
            count=1000;
        }

        MultipleCondition cond;
        cond.andCondList.push_back(string("id >= ") + to_string((page - 1) * count + 1 ));
        cond.andCondList.push_back(string("id <= ") + to_string(page * count));
        vector<Coupon> couponList;
        ret = couponHelper.getCouponList(couponList,cond,batchExportCodeReq.coupon_group_id,0,999999);
        if(-9999 == ret){
            batchExportCodeRsp.error = coupon::Error::OK;
            batchExportCodeRsp.errmsg = "success";
            batchExportCodeRsp.page = batchExportCodeReq.page;
            batchExportCodeRsp.count = count;
            batchExportCodeRsp.total = couponCountRsp.count;
            return;
        }

        if(0 != ret){
            batchExportCodeRsp.error = Error::MYSQL_EXECUTE_ERROR;
            batchExportCodeRsp.errmsg = couponHelper.getError();
            return ;
        }

        batchExportCodeRsp.error = coupon::Error::OK;
        batchExportCodeRsp.errmsg = "success";
        batchExportCodeRsp.page = batchExportCodeReq.page;
        batchExportCodeRsp.count = count;
        batchExportCodeRsp.total = couponCountRsp.count;

        vector<string> codes;
        for(auto x:couponList){
            codes.push_back(x.code);
        }
        batchExportCodeRsp.codes = codes;

        LOG_WARN<<"batchExportCode response is: BatchExportCodeRsp '"<<ThriftToJSON(batchExportCodeRsp)<<"'"<<endl;
        return;
    }

    void batchExport(BatchExportRsp& batchExportRsp,const BatchExportReq& batchExportReq){
        LOG_WARN<<"batchExport request is: BatchExportReq '"<<ThriftToJSON(batchExportReq)<<"'"<<endl;
        PerfWatch perfWatch("batchExport");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        MysqlQueryHelper queryHelper(mwrapper);
        CouponHelper couponHelper(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        //如果获取的Redis连接断开
        LOG_INFO <<"rpool zise:" <<rpool->Size();
        RedisWrapperGuard rwrapperGuard(rpool,rpool->GetRedisWrapper());
        RedisWrapper* rwrapper = rwrapperGuard.getRedisWrapper();
        LOG_INFO <<"rpool zise:" <<rpool->Size()<<" rwrapper:"<<rwrapper;
        if(NULL == rwrapper||rwrapper->isClosed()){
            LOG_ERROR<<"Redis connection disconnected "<<rwrapper->getError()<< endl;
            io.fault = Error::REDIS_DISCONNECTED;
            io.why = "Redis connection disconnected";
            throw io;
        }

        //如果count不合法
        int ret = queryHelper.runSelect(string("")
            +" select count(*)"
            +" from " + coupon_table_prefix + to_string(batchExportReq.coupon_group_id)
        );

        int count = queryHelper.getIntegerField("count(*)");

        if(batchExportReq.count + count > 10000000 ||batchExportReq.count == 0){
            LOG_ERROR<<"count outload... count =  "<<batchExportReq.count<< endl;
            batchExportRsp.error = coupon::Error::OUTLOAD_COUNT;
            batchExportRsp.errmsg = "count outload...";
            batchExportRsp.count = batchExportReq.count;
            return;
        }

        //校验组ID,如果这个组在表中不存在
        CouponGroup couponGroup;
        ret = couponHelper.queryCouponGroup(batchExportReq.coupon_group_id,couponGroup);
        if(-9999 == ret){
            LOG_ERROR<<"ret="<<ret<<" queryCouponGroup no any expected data!"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            batchExportRsp.error = Error::COUPON_GROUP_NOT_EXISTS;
            batchExportRsp.errmsg = "no coupon group you expected";
            return;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        if(3 != couponGroup.scene_type){
            LOG_ERROR<<" CouponGroup is not a batch-export group..."<< endl;
            batchExportRsp.error = Error::ILLEGAL_SCENE_TYPE;
            batchExportRsp.errmsg = "CouponGroup is not a batch-export group...";
            return;
        }

        if(couponGroup.max_count < batchExportReq.count + count){
            LOG_ERROR<<" CouponGroup max_count < batchExportReq.count + count..."<< endl;
            batchExportRsp.error = Error::OUTLOAD_COUNT;
            batchExportRsp.errmsg = " CouponGroup max_count < batchExportReq.count + count...";
            return;
        }

        ret = rwrapper->rpush(COUPON_BATCH_EXPORT_LIST, ThriftToJSON(batchExportReq));
        if(0 != ret){
            LOG_ERROR<<"ret:"<<ret<<" rpush failed: batchExportReq.coupon_group_id = "<<batchExportReq.coupon_group_id<<endl;
            batchExportRsp.error = Error::FAILED;
            batchExportRsp.errmsg = "rpush failed ...";
            return;
        }

        batchExportRsp.error = coupon::Error::OK;
        batchExportRsp.errmsg = "success";
        batchExportRsp.count = batchExportReq.count;
        batchExportRsp.time_cost = batchExportReq.count * 2;//ms

        LOG_WARN<<"batchExport response is: BatchExportRsp '"<<ThriftToJSON(batchExportRsp)<<"'"<<endl;
        return;
    }

    void batchDispatch(BatchDispatchRsp& batchDispatchRsp,const BatchDispatchReq& batchDispatchReq){
        LOG_WARN<<"batchDispatch request is: BatchDispatchReq '"<<ThriftToJSON(batchDispatchReq)<<"'"<<endl;
        PerfWatch perfWatch("batchDispatch");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        //如果获取的Redis连接断开
        LOG_INFO <<"rpool zise:" <<rpool->Size();
        RedisWrapperGuard rwrapperGuard(rpool,rpool->GetRedisWrapper());
        RedisWrapper* rwrapper = rwrapperGuard.getRedisWrapper();
        LOG_INFO <<"rpool zise:" <<rpool->Size()<<" rwrapper:"<<rwrapper;
        if(NULL == rwrapper||rwrapper->isClosed()){
            LOG_ERROR<<"Redis connection disconnected "<<rwrapper->getError()<< endl;
            io.fault = Error::REDIS_DISCONNECTED;
            io.why = "Redis connection disconnected";
            throw io;
        }

        if(batchDispatchReq.user_id_list.size() > 1000 ||batchDispatchReq.user_id_list.size() == 0){
            LOG_ERROR<<"count outload... size =  "<<batchDispatchReq.user_id_list.size()<< endl;
            batchDispatchRsp.error = coupon::Error::OUTLOAD_COUNT;
            batchDispatchRsp.errmsg = "count outload...";
            batchDispatchRsp.count = batchDispatchReq.user_id_list.size();
            return;
        }

        //校验组ID,如果这个组在表中不存在
        CouponGroup couponGroup;
        int ret = couponHelper.queryCouponGroup(batchDispatchReq.coupon_group_id,couponGroup);
        if(-9999 == ret){
            LOG_ERROR<<"ret="<<ret<<" queryCouponGroup no any expected data!"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            batchDispatchRsp.error = Error::COUPON_GROUP_NOT_EXISTS;
            batchDispatchRsp.errmsg = "no coupon group you expected";
            return;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        if(4 != couponGroup.scene_type){
            LOG_ERROR<<" CouponGroup is not a batch-dispatch group..."<< endl;
            batchDispatchRsp.error = Error::ILLEGAL_SCENE_TYPE;
            batchDispatchRsp.errmsg = "CouponGroup is not a batch-dispatch group...";
            return;
        }

        if(couponGroup.max_count < (int)batchDispatchReq.user_id_list.size()){
            LOG_ERROR<<" CouponGroup max_count < batchDispatchReq.user_id_list.size ..."<< endl;
            batchDispatchRsp.error = Error::OUTLOAD_COUNT;
            batchDispatchRsp.errmsg = " CouponGroup max_count < batchDispatchReq.user_id_list.size ...";
            return;
        }

        ret = rwrapper->rpush(COUPON_BATCH_DISPATCH_LIST, ThriftToJSON(batchDispatchReq));
        if(0 != ret){
            LOG_ERROR<<"rpush failed: batchDispatchReq.coupon_group_id = "<<batchDispatchReq.coupon_group_id<<endl;
            batchDispatchRsp.error = Error::FAILED;
            batchDispatchRsp.errmsg = "rpush failed ...";
            return;
        }

        batchDispatchRsp.error = coupon::Error::OK;
        batchDispatchRsp.errmsg = "success";
        batchDispatchRsp.count = batchDispatchReq.user_id_list.size();
        batchDispatchRsp.success_count = batchDispatchReq.user_id_list.size();
        batchDispatchRsp.time_cost = batchDispatchReq.user_id_list.size() * 10;//ms

        LOG_WARN<<"batchDispatch response is: BatchDispatchRsp '"<<ThriftToJSON(batchDispatchRsp)<<"'"<<endl;
        return;
    }

    void verifyArgot(VerifyArgotRsp& verifyArgotRsp,const VerifyArgotReq& verifyArgotReq){
        LOG_WARN<<"verifyArgot request is: VerifyArgotReq '"<<ThriftToJSON(verifyArgotReq)<<"'"<<endl;
        PerfWatch perfWatch("verifyArgot");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        MysqlQueryHelper qh(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        unsigned int count = 0;
        const char *p = verifyArgotReq.argot.c_str();
        unsigned int len = verifyArgotReq.argot.length();
        for(const char* ptr = p;ptr<p+len;ptr++){
            if(*ptr >= 'a' && *ptr <= 'z'){
                count++;
            }
        }

        if(count == 11 || verifyArgotReq.argot == ""){
            LOG_ERROR<<"the error detail is that,invalid argot:"<<verifyArgotReq.argot<<endl;
            verifyArgotRsp.error = coupon::Error::INVALID_ARGOT;
            verifyArgotRsp.errmsg = "invalid argot,all should be lower case";
            return;
        }

        vector<CouponGroup> couponGroupList;
        MultipleCondition cond;
        cond.andCondList.push_back(string("binary argot = '")+verifyArgotReq.argot+"'");
        cond.andCondList.push_back(string("id<>")+to_string(verifyArgotReq.coupon_group_id));
        cond.andCondList.push_back(string("scene_type = 1"));
        cond.andCondList.push_back(string("UNIX_TIMESTAMP(create_time) > ")+to_string(time(NULL) - 365 * 24 * 3600));

        unsigned int sum = 0;
        couponHelper.getCouponGroupList(couponGroupList,cond,0,9999);
        for(auto x:couponGroupList){
            if(verifyArgotReq.start_draw_time < x.end_draw_time + 180*24*3600){
                sum++;
                break;
            }
        }

        if(sum > 0){
            LOG_ERROR<<"the error detail is that,argot in 180 days:"<<verifyArgotReq.argot<<endl;
            verifyArgotRsp.error = coupon::Error::ARGOT_IN_180_DAYS;
            verifyArgotRsp.errmsg = "argot in 180 days";
            return;
        }

        verifyArgotRsp.error = coupon::Error::OK;
        verifyArgotRsp.errmsg = "success";
        verifyArgotRsp.argot = verifyArgotReq.argot;

        LOG_WARN<<"verifyArgot response is: VerifyArgotRsp '"<<ThriftToJSON(verifyArgotRsp)<<"'"<<endl;
        return;
    }

    void createWareLabel(CreateWareLabelRsp& createWareLabelRsp, const CreateWareLabelReq& createWareLabelReq){
        LOG_WARN<<"createWareLabel request is: CreateWareLabelReq '"<<ThriftToJSON(createWareLabelReq)<<"'"<<endl;
        PerfWatch perfWatch("createWareLabel");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        MysqlQueryHelper qh(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        int insert_id = qh.runInsert(string("")
            +" insert into " + ware_label_table
            +" (name,scope_type,sub_type,create_time,update_time,create_man) "
            +" values( '"
            +createWareLabelReq.name + "',"
            +to_string(createWareLabelReq.scope_type) + ","
            +to_string(createWareLabelReq.sub_type) + ","
            +"now(),"
            +"now(),"
            +"'" + createWareLabelReq.create_man + "'"
            +")"
        );

        if(insert_id < 0){
            LOG_ERROR<<" error occured when mysql execute createWareLabel"<< endl;
            LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = "error occured when mysql execute createWareLabel:" + qh.getError();
            throw io;
        }

        createWareLabelRsp.error = coupon::Error::OK;
        createWareLabelRsp.errmsg = "success";
        createWareLabelRsp.ware_label_id = insert_id;

        LOG_WARN<<"createWareLabel response is: CreateWareLabelRsp '"<<ThriftToJSON(createWareLabelRsp)<<"'"<<endl;
        return ;
    }

    void updateWareLabel(UpdateWareLabelRsp& updateWareLabelRsp, const UpdateWareLabelReq& updateWareLabelReq){
        LOG_WARN<<"updateWareLabel request is: UpdateWareLabelReq '"<<ThriftToJSON(updateWareLabelReq)<<"'"<<endl;
        PerfWatch perfWatch("updateWareLabel");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        MysqlQueryHelper qh(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        int ret = qh.runUpdate(string("")
            +" update " + ware_label_table
            +" set name = '" + updateWareLabelReq.wareLabel.name + "'"
            +" ,scope_type = " + to_string(updateWareLabelReq.wareLabel.scope_type)
            +" ,sub_type = " + to_string(updateWareLabelReq.wareLabel.sub_type)
            +" ,update_time = now()"
            +" ,create_man = '" + updateWareLabelReq.wareLabel.create_man + "'"
            +" where id = "+to_string(updateWareLabelReq.wareLabel.ware_label_id)
        );

        if(ret < 0){
            LOG_ERROR<<"ret= "<<ret<<" id="<<updateWareLabelReq.wareLabel.ware_label_id<<" error occured when mysql execute.."<< endl;
            LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = qh.getError();
            throw io;
        }

        updateWareLabelRsp.error = coupon::Error::OK,
        updateWareLabelRsp.errmsg = "success";

        LOG_WARN<<"updateWareLabel response is: UpdateWareLabelRsp '"<<ThriftToJSON(updateWareLabelRsp)<<"'"<<endl;
        return ;
    }

    void addWareLabelWares(AddWareLabelWaresRsp& addWareLabelWaresRsp, const AddWareLabelWaresReq& addWareLabelWaresReq){
        LOG_WARN<<"addWareLabelWares request is: AddWareLabelWaresReq '"<<ThriftToJSON(addWareLabelWaresReq)<<"'"<<endl;
        PerfWatch perfWatch("addWareLabelWares");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        //如果获取的Redis连接断开
        LOG_INFO <<"rpool zise:" <<rpool->Size();
        RedisWrapperGuard rwrapperGuard(rpool,rpool->GetRedisWrapper());
        RedisWrapper* rwrapper = rwrapperGuard.getRedisWrapper();
        LOG_INFO <<"rpool zise:" <<rpool->Size()<<" rwrapper:"<<rwrapper;
        if(NULL == rwrapper||rwrapper->isClosed()){
            LOG_ERROR<<"Redis connection disconnected "<<rwrapper->getError()<< endl;
            io.fault = Error::REDIS_DISCONNECTED;
            io.why = "Redis connection disconnected";
            throw io;
        }

        for(int i=0;i<(int)addWareLabelWaresReq.wareLabelWaresList.size();i++){
            unsigned int ware_label_id = addWareLabelWaresReq.wareLabelWaresList[i].ware_label_id;
            int64_t ware_id = addWareLabelWaresReq.wareLabelWaresList[i].ware_id;
            string ware_slug = "'" + addWareLabelWaresReq.wareLabelWaresList[i].ware_slug + "'";

            MysqlQueryHelper qh(mwrapper);
            int insert_id = qh.runInsert(string("")
                +" insert into " + ware_label_wares_table
                +" (ware_label_id,ware_id,ware_slug) "
                +" values("
                +to_string(ware_label_id) + ","
                +to_string(ware_id) + ","
                +ware_slug
                +")"
            );

            if(insert_id < 0){
                LOG_ERROR<<" error occured when mysql execute addWareLabelWares"<< endl;
                LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
                io.fault = Error::MYSQL_EXECUTE_ERROR;
                io.why = "error occured when mysql execute addWareLabelWares:" + qh.getError();
                throw io;
            }
        }

        addWareLabelWaresRsp.error = coupon::Error::OK,
        addWareLabelWaresRsp.errmsg = "success";

        LOG_WARN<<"addWareLabelWares response is: AddWareLabelWaresRsp '"<<ThriftToJSON(addWareLabelWaresRsp)<<"'"<<endl;
        return ;
    }

    void delWareLabelWares(DelWareLabelWaresRsp& delWareLabelWaresRsp, const DelWareLabelWaresReq& delWareLabelWaresReq){
        LOG_WARN<<"delWareLabelWares request is: DelWareLabelWaresReq '"<<ThriftToJSON(delWareLabelWaresReq)<<"'"<<endl;
        PerfWatch perfWatch("delWareLabelWares");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        MysqlQueryHelper qh(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        //如果获取的Redis连接断开
        LOG_INFO <<"rpool zise:" <<rpool->Size();
        RedisWrapperGuard rwrapperGuard(rpool,rpool->GetRedisWrapper());
        RedisWrapper* rwrapper = rwrapperGuard.getRedisWrapper();
        LOG_INFO <<"rpool zise:" <<rpool->Size()<<" rwrapper:"<<rwrapper;
        if(NULL == rwrapper||rwrapper->isClosed()){
            LOG_ERROR<<"Redis connection disconnected "<<rwrapper->getError()<< endl;
            io.fault = Error::REDIS_DISCONNECTED;
            io.why = "Redis connection disconnected";
            throw io;
        }

        int ret = qh.runDelete(string("")
            +" DELETE FROM "+ware_label_wares_table
            +" where ware_label_id = " + to_string(delWareLabelWaresReq.ware_label_id)
            +" and ware_id = " + to_string(delWareLabelWaresReq.ware_id)
        );

        if(ret < 0){
            LOG_ERROR<<" error occured when mysql execute delWareLabelWares"<< endl;
            LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = "error occured when mysql execute delWareLabelWares:" + qh.getError();
            throw io;
        }

        delWareLabelWaresRsp.error = coupon::Error::OK,
        delWareLabelWaresRsp.errmsg = "success";

        LOG_WARN<<"delWareLabelWares response is: DelWareLabelWaresRsp '"<<ThriftToJSON(delWareLabelWaresRsp)<<"'"<<endl;
        return ;
    }

    void createCouponGroup(CreateCouponGroupRsp& createCouponGroupRsp, const CreateCouponGroupReq& createCouponGroupReq){
        LOG_WARN<<"createCouponGroup request is: CreateCouponGroupReq '"<<ThriftToJSON(createCouponGroupReq)<<"'"<<endl;
        PerfWatch perfWatch("createCouponGroup");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        MysqlQueryHelper qh(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        //一个用户同一时间最多拥有500张可用优惠券,同时在线的有效券组也不能超过500组
        MultipleCondition cond;
        cond.andCondList.push_back(string("verify_status = 2"));
        cond.andCondList.push_back(string("seller_id = 0"));
        cond.andCondList.push_back(string("UNIX_TIMESTAMP(create_time) > ")+to_string(time(NULL) - 365 * 24 * 3600));

        vector<CouponGroup> couponGroupList;
        couponHelper.getCouponGroupList(couponGroupList,cond,0,9999);

        int valid_count = 0;
        for(auto iter = couponGroupList.begin();iter != couponGroupList.end();){
            if( (!iter->is_duration_type && time(NULL) > iter->end_use_time)
                || (iter->is_duration_type && time(NULL) > iter->end_draw_time + iter->duration_value * 24 * 3600)){
                    iter = couponGroupList.erase(iter);
            }else{
                valid_count += iter->can_draw_count;
                iter++;
            }
        }

        if(couponGroupList.size() >= 500){
            LOG_ERROR<<"couponGroupList.size() >= 500"<< endl;
            createCouponGroupRsp.error = coupon::Error::FAILED,
            createCouponGroupRsp.errmsg = "同时在线可用的平台券组数量超过上限(500)";
            return;
        }

        if(valid_count + createCouponGroupReq.couponGroup.can_draw_count > 500 ){
            LOG_ERROR<<"valid_count > 500"<< endl;
            createCouponGroupRsp.error = coupon::Error::FAILED,
            createCouponGroupRsp.errmsg = "同时在线可领的平台券数量超过上限(500)";
            return;
        }

        unsigned int can_draw_count = 1;
        if(0 != createCouponGroupReq.couponGroup.can_draw_count){
            can_draw_count = createCouponGroupReq.couponGroup.can_draw_count;
        }

        if(can_draw_count > 30){
            can_draw_count = 30;
        }

        unsigned int insert_id = qh.runInsert(string("")
            +" insert into " + coupon_group_table
            +" (name,title,comment,ware_label_id,favor_type,"
            +" scope_type,sub_type,scene_type,full,favor,rate,argot,max_count,"
            +" delta,drawn_count,payed_count,create_time,can_draw_count,"
            +" start_draw_time,end_draw_time,"
            +" is_duration_type,duration_value,"
            +" start_use_time,end_use_time,verify_status ,"
            +" applicant,approver,modifier,seller_id,url,img,delta_verify_status,"
            +" img_width,img_height,button_text,button_jump,"
            +" jump_label,jump_data,argot_jump_label,argot_jump_data,update_time,version,description"
            +" ) "
            +" values("
            +"'" + createCouponGroupReq.couponGroup.name +"',"
            +"'" + createCouponGroupReq.couponGroup.title +"',"
            +"'" + createCouponGroupReq.couponGroup.comment +"',"
            +to_string(createCouponGroupReq.couponGroup.ware_label_id) + ","
            +to_string(createCouponGroupReq.couponGroup.favor_type) + ","
            +to_string(createCouponGroupReq.couponGroup.scope_type) + ","
            +to_string(createCouponGroupReq.couponGroup.sub_type) + ","
            +to_string(createCouponGroupReq.couponGroup.scene_type) + ","
            +to_string(createCouponGroupReq.couponGroup.full) + ","
            +to_string(createCouponGroupReq.couponGroup.favor) + ","
            +to_string(createCouponGroupReq.couponGroup.rate) + ","
            +"'" + createCouponGroupReq.couponGroup.argot +"',"
            +to_string(createCouponGroupReq.couponGroup.max_count) + ","
            +to_string(createCouponGroupReq.couponGroup.delta) + ","
            +to_string(createCouponGroupReq.couponGroup.drawn_count) + ","
            +to_string(createCouponGroupReq.couponGroup.payed_count) + ","
            +"now(),"
            +to_string(can_draw_count) + ","
            +"FROM_UNIXTIME("+to_string(createCouponGroupReq.couponGroup.start_draw_time) + "),"
            +"FROM_UNIXTIME("+to_string(createCouponGroupReq.couponGroup.end_draw_time) + "),"
            +to_string(createCouponGroupReq.couponGroup.is_duration_type) + ","
            +to_string(createCouponGroupReq.couponGroup.duration_value) + ","
            +"FROM_UNIXTIME("+to_string(createCouponGroupReq.couponGroup.start_use_time) + "),"
            +"FROM_UNIXTIME("+to_string(createCouponGroupReq.couponGroup.end_use_time) + "),"
            +to_string(createCouponGroupReq.couponGroup.verify_status) + ","
            +"'" + createCouponGroupReq.couponGroup.applicant + "',"
            +"'" + createCouponGroupReq.couponGroup.approver + "',"
            +"'" + createCouponGroupReq.couponGroup.modifier + "',"
            +to_string(createCouponGroupReq.couponGroup.seller_id) + ","
            +"'" + createCouponGroupReq.couponGroup.url + "',"
            +"'" + createCouponGroupReq.couponGroup.img + "',"
            +to_string(createCouponGroupReq.couponGroup.delta_verify_status) + ","
            +to_string(createCouponGroupReq.couponGroup.img_width) + ","
            +to_string(createCouponGroupReq.couponGroup.img_height) + ","
            +"'" + createCouponGroupReq.couponGroup.button_text + "',"
            +"'" + createCouponGroupReq.couponGroup.button_jump + "',"
            +"'" + createCouponGroupReq.couponGroup.jump_label + "',"
            +"'" + createCouponGroupReq.couponGroup.jump_data + "',"
            +"'" + createCouponGroupReq.couponGroup.argot_jump_label + "',"
            +"'" + createCouponGroupReq.couponGroup.argot_jump_data + "',"
            +"FROM_UNIXTIME("+to_string(createCouponGroupReq.couponGroup.update_time) + "),"
            +to_string(0)+","
            +"'" + createCouponGroupReq.couponGroup.description + "'"
            +")"
        );

        if(insert_id < 0){
            LOG_ERROR<<" error occured when mysql execute createCouponGroup"<< endl;
            LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = "error occured when mysql execute createCouponGroup:" + qh.getError();
            throw io;
        }

        string slug;
        if(CouponCode::Errno::OK == CouponCode::GenCouponGroupSlug(insert_id,slug)){
            int ret = qh.runUpdate(string("")
                +" update " + coupon_group_table
                +" set slug = '" + slug + "'"
                +" where id = "+to_string(insert_id)
                );

            if(ret < 0){
                LOG_ERROR<<"ret= "<<ret<<" id="<<insert_id<<" error occured when mysql execute.."<< endl;
                LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
                io.fault = Error::MYSQL_EXECUTE_ERROR;
                io.why = qh.getError();
                throw io;
            }
        }

        createCouponGroupRsp.error = coupon::Error::OK,
        createCouponGroupRsp.errmsg = "success";
        createCouponGroupRsp.coupon_group_id = insert_id;

        LOG_WARN<<"createCouponGroup response is: CreateCouponGroupRsp '"<<ThriftToJSON(createCouponGroupRsp)<<"'"<<endl;
        return ;
    }

    void updateCouponGroup(UpdateCouponGroupRsp& updateCouponGroupRsp, const UpdateCouponGroupReq& updateCouponGroupReq){
        LOG_WARN<<"updateCouponGroup request is: UpdateCouponGroupReq '"<<ThriftToJSON(updateCouponGroupReq)<<"'"<<endl;
        PerfWatch perfWatch("updateCouponGroup");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        MysqlQueryHelper qh(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        CouponGroup couponGroup;
        int ret = couponHelper.getCouponGroup(updateCouponGroupReq.couponGroup.id, couponGroup);
        if(ret < 0){
            LOG_ERROR<<"ret= "<<ret<<" id="<<updateCouponGroupReq.couponGroup.id<<" error occured when mysql execute.."<< endl;
            LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = qh.getError();
            throw io;
        }

        if (couponGroup.verify_status !=2 && updateCouponGroupReq.couponGroup.verify_status == 2) {//审批通过
            //一个用户同一时间最多拥有500张可用优惠券,同时在线的有效券组也不能超过500组
            MultipleCondition cond;
            cond.andCondList.push_back(string("verify_status = 2"));
            cond.andCondList.push_back(string("seller_id = 0"));
            cond.andCondList.push_back(string("UNIX_TIMESTAMP(create_time) > ")+to_string(time(NULL) - 365 * 24 * 3600));

            vector<CouponGroup> couponGroupList;
            couponHelper.getCouponGroupList(couponGroupList,cond,0,9999);

            int valid_count = 0;
            for(auto iter = couponGroupList.begin();iter != couponGroupList.end();){
                if( (!iter->is_duration_type && time(NULL) > iter->end_use_time)
                    || (iter->is_duration_type && time(NULL) > iter->end_draw_time + iter->duration_value * 24 * 3600)){
                        iter = couponGroupList.erase(iter);
                }else{
                    valid_count += iter->can_draw_count;
                    iter++;
                }
            }

            if(couponGroupList.size() + 1 > 500){
                LOG_ERROR<<"couponGroupList.size() + 1 > 500"<< endl;
                updateCouponGroupRsp.error = coupon::Error::FAILED;
                updateCouponGroupRsp.errmsg = "同时在线可用的平台券组数量超过上限(500)";
                return;
            }

            if(valid_count + updateCouponGroupReq.couponGroup.can_draw_count > 500 ){
                LOG_ERROR<<"valid_count > 500"<< endl;
                updateCouponGroupRsp.error = coupon::Error::FAILED;
                updateCouponGroupRsp.errmsg = "同时在线可领的平台券数量超过上限(500)";
                return;
            }
        }

        ret = qh.runUpdate(string("")
            +" update " + coupon_group_table
            +" set name = '" + updateCouponGroupReq.couponGroup.name + "'"
            +" ,title = '" + updateCouponGroupReq.couponGroup.title + "'"
            +" ,comment = '" + updateCouponGroupReq.couponGroup.comment + "'"
            +" ,ware_label_id = " + to_string(updateCouponGroupReq.couponGroup.ware_label_id)
            +" ,favor_type = " + to_string(updateCouponGroupReq.couponGroup.favor_type)
            +" ,scope_type = " + to_string(updateCouponGroupReq.couponGroup.scope_type)
            +" ,sub_type = " + to_string(updateCouponGroupReq.couponGroup.sub_type)
            +" ,scene_type = " + to_string(updateCouponGroupReq.couponGroup.scene_type)
            +" ,full = " + to_string(updateCouponGroupReq.couponGroup.full)
            +" ,favor = " + to_string(updateCouponGroupReq.couponGroup.favor)
            +" ,rate = " + to_string(updateCouponGroupReq.couponGroup.rate)
            +" ,argot = '" + updateCouponGroupReq.couponGroup.argot + "'"
            +" ,max_count = " + to_string(updateCouponGroupReq.couponGroup.max_count)
            +" ,delta = " + to_string(updateCouponGroupReq.couponGroup.delta)
            +" ,drawn_count = " + to_string(updateCouponGroupReq.couponGroup.drawn_count)
            +" ,payed_count = " + to_string(updateCouponGroupReq.couponGroup.payed_count)
            +" ,can_draw_count = " + to_string(updateCouponGroupReq.couponGroup.can_draw_count)
            +" ,start_draw_time = FROM_UNIXTIME(" + to_string(updateCouponGroupReq.couponGroup.start_draw_time) + ")"
            +" ,end_draw_time = FROM_UNIXTIME(" + to_string(updateCouponGroupReq.couponGroup.end_draw_time) + ")"
            +" ,is_duration_type = " + to_string(updateCouponGroupReq.couponGroup.is_duration_type)
            +" ,duration_value = " + to_string(updateCouponGroupReq.couponGroup.duration_value)
            +" ,start_use_time = FROM_UNIXTIME(" + to_string(updateCouponGroupReq.couponGroup.start_use_time) + ")"
            +" ,end_use_time = FROM_UNIXTIME(" + to_string(updateCouponGroupReq.couponGroup.end_use_time) + ")"
            +" ,verify_status = " + to_string(updateCouponGroupReq.couponGroup.verify_status)
            +" ,applicant = '" + updateCouponGroupReq.couponGroup.applicant + "'"
            +" ,approver = '" + updateCouponGroupReq.couponGroup.approver + "'"
            +" ,modifier = '" + updateCouponGroupReq.couponGroup.modifier + "'"
            +" ,seller_id = " + to_string(updateCouponGroupReq.couponGroup.seller_id)
            +" ,url = '" + updateCouponGroupReq.couponGroup.url + "'"
            +" ,img = '" + updateCouponGroupReq.couponGroup.img + "'"
            +" ,delta_verify_status = " + to_string(updateCouponGroupReq.couponGroup.delta_verify_status)
            +" ,img_width = " + to_string(updateCouponGroupReq.couponGroup.img_width)
            +" ,img_height = " + to_string(updateCouponGroupReq.couponGroup.img_height)
            +" ,button_text = '" + updateCouponGroupReq.couponGroup.button_text + "'"
            +" ,button_jump = '" + updateCouponGroupReq.couponGroup.button_jump + "'"
            +" ,jump_label = '" + updateCouponGroupReq.couponGroup.jump_label + "'"
            +" ,jump_data = '" + updateCouponGroupReq.couponGroup.jump_data + "'"
            +" ,argot_jump_label = '" + updateCouponGroupReq.couponGroup.argot_jump_label + "'"
            +" ,argot_jump_data = '" + updateCouponGroupReq.couponGroup.argot_jump_data + "'"
            +" ,update_time = FROM_UNIXTIME(" + to_string(updateCouponGroupReq.couponGroup.update_time) + ")"
            +" ,version = " + to_string(updateCouponGroupReq.couponGroup.version + 1)
            +" ,description = '" + updateCouponGroupReq.couponGroup.description + "'"
            +" where id = "+to_string(updateCouponGroupReq.couponGroup.id)
        );

        if(ret < 0){
            LOG_ERROR<<"ret= "<<ret<<" id="<<updateCouponGroupReq.couponGroup.id<<" error occured when mysql execute.."<< endl;
            LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = qh.getError();
            throw io;
        }

        updateCouponGroupRsp.error = coupon::Error::OK,
        updateCouponGroupRsp.errmsg = "success";
        updateCouponGroupRsp.couponGroup = updateCouponGroupReq.couponGroup;

        LOG_WARN<<"updateCouponGroup response is: UpdateCouponGroupRsp '"<<ThriftToJSON(updateCouponGroupRsp)<<"'"<<endl;
        return ;
    }

    void createCouponTable(CreateCouponTableRsp& createCouponTableRsp, const CreateCouponTableReq& createCouponTableReq){
        LOG_WARN<<"createCouponTable request is: CreateCouponTableReq '"<<ThriftToJSON(createCouponTableReq)<<"'"<<endl;
        PerfWatch perfWatch("createCouponTable");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        MysqlQueryHelper qh(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        //如果对应的券表不存在，则产生一张券表
        int ret = couponHelper.createCouponTable(createCouponTableReq.coupon_group_id);
        if(ret < 0){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute..."<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        createCouponTableRsp.error = coupon::Error::OK,
        createCouponTableRsp.errmsg = "success";
        createCouponTableRsp.tableName = coupon_table_prefix + to_string(createCouponTableReq.coupon_group_id);

        LOG_WARN<<"createCouponTable response is: CreateCouponTableRsp '"<<ThriftToJSON(createCouponTableRsp)<<"'"<<endl;
        return ;
    }

    void getCouponCount(CouponCountRsp& couponCountRsp, const CouponCountReq& couponCountReq){
        LOG_WARN<<"getCouponCount request is: CouponCountReq '"<<ThriftToJSON(couponCountReq)<<"'"<<endl;
        PerfWatch perfWatch("getCouponCount");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        MysqlQueryHelper qh(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        int ret = qh.runSelect(string("")
            +" select count(*) "
            +" from " + coupon_table_prefix + to_string(couponCountReq.coupon_group_id)
            +" " + getWhereCondition(couponCountReq.cond)
            +" " + getOrderByCondition(couponCountReq.cond)
        );

        if(-9999 == ret){
            couponCountRsp.error = coupon::Error::OK,
            couponCountRsp.errmsg = "success";
            return;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql exe queryCouponGroupIds"<< endl;
            LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = qh.getError();
            throw io;
        }

        couponCountRsp.error = coupon::Error::OK,
        couponCountRsp.errmsg = "success";
        couponCountRsp.count = qh.getIntegerField("count(*)");

        LOG_WARN<<"getCouponCount response is: CouponCountRsp '"<<ThriftToJSON(couponCountRsp)<<"'"<<endl;
        return ;
    }

    void getCouponList(CouponListRsp& couponListRsp, const CouponListReq& couponListReq){
        LOG_WARN<<"getCouponList request is: CouponListReq '"<<ThriftToJSON(couponListReq)<<"'"<<endl;
        PerfWatch perfWatch("getCouponList");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        MysqlQueryHelper qh(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        vector<Coupon> couponList;
        int ret = couponHelper.getCouponList(couponList,couponListReq.cond,couponListReq.coupon_group_id,couponListReq.offset,couponListReq.rows);
        if(-9999 == ret){
            couponListRsp.error = coupon::Error::OK,
            couponListRsp.errmsg = "success";
        }

        if(0 != ret){
            couponListRsp.error = Error::MYSQL_EXECUTE_ERROR;
            couponListRsp.errmsg = couponHelper.getError();
            return ;
        }

        couponListRsp.error = coupon::Error::OK,
        couponListRsp.errmsg = "success";
        couponListRsp.couponList = couponList;

        LOG_WARN<<"getCouponList response is: CouponListRsp '"<<ThriftToJSON(couponListRsp)<<"'"<<endl;
        return ;
    }

    void getCouponGroupCount(CouponGroupCountRsp& couponGroupCountRsp, const CouponGroupCountReq& couponGroupCountReq){
        LOG_WARN<<"getCouponGroupCount request is: CouponGroupCountReq '"<<ThriftToJSON(couponGroupCountReq)<<"'"<<endl;
        PerfWatch perfWatch("getCouponGroupCount");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        MysqlQueryHelper qh(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        int ret = qh.runSelect(string("")
            +" select count(*) "
            +" from " + coupon_group_table
            +" " + getWhereCondition(couponGroupCountReq.cond)
            +" " + getOrderByCondition(couponGroupCountReq.cond)
        );

        if(-9999 == ret){
            couponGroupCountRsp.error = coupon::Error::OK,
            couponGroupCountRsp.errmsg = "success";
            return ;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql exe queryCouponGroupIds"<< endl;
            LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = qh.getError();
            throw io;
        }

        couponGroupCountRsp.error = coupon::Error::OK,
        couponGroupCountRsp.errmsg = "success";
        couponGroupCountRsp.count = qh.getIntegerField("count(*)");

        LOG_WARN<<"getCouponGroupCount response is: CouponGroupCountRsp '"<<ThriftToJSON(couponGroupCountRsp)<<"'"<<endl;
        return ;
    }

    void getCouponGroupList(CouponGroupListRsp& couponGroupListRsp, const CouponGroupListReq& couponGroupListReq){
        LOG_WARN<<"getCouponGroupList request is: CouponGroupListReq '"<<ThriftToJSON(couponGroupListReq)<<"'"<<endl;
        PerfWatch perfWatch("getCouponGroupList");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        vector<CouponGroup> couponGroupList;
        int ret = couponHelper.getCouponGroupList(couponGroupList,couponGroupListReq.cond,couponGroupListReq.offset,couponGroupListReq.rows);
        if(-9999 == ret){
            couponGroupListRsp.error = coupon::Error::OK,
            couponGroupListRsp.errmsg = "success";
            return ;
        }

        if(0 != ret){
            couponGroupListRsp.error = Error::MYSQL_EXECUTE_ERROR;
            couponGroupListRsp.errmsg = couponHelper.getError();
            return ;
        }

        couponGroupListRsp.error = coupon::Error::OK,
        couponGroupListRsp.errmsg = "success";
        couponGroupListRsp.couponGroupList = couponGroupList;

        LOG_WARN<<"getCouponGroupList response is: CouponGroupListRsp '"<<ThriftToJSON(couponGroupListRsp)<<"'"<<endl;
        return ;
    }

    void getWareLabelCount(WareLabelCountRsp& wareLabelCountRsp, const WareLabelCountReq& wareLabelCountReq){
        LOG_WARN<<"getWareLabelCount request is: WareLabelCountReq '"<<ThriftToJSON(wareLabelCountReq)<<"'"<<endl;
        PerfWatch perfWatch("getWareLabelCount");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        MysqlQueryHelper qh(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        int ret = qh.runSelect(string("")
            +" select count(*) "
            +" from " + ware_label_table
            +" " + getWhereCondition(wareLabelCountReq.cond)
            +" " + getOrderByCondition(wareLabelCountReq.cond)
        );

        if(-9999 == ret){
            wareLabelCountRsp.error = coupon::Error::OK,
            wareLabelCountRsp.errmsg = "success";
            return;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql exe queryCouponGroupIds"<< endl;
            LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = qh.getError();
            throw io;
        }

        wareLabelCountRsp.error = coupon::Error::OK,
        wareLabelCountRsp.errmsg = "success";
        wareLabelCountRsp.count = qh.getIntegerField("count(*)");

        LOG_WARN<<"getWareLabelCount response is: WareLabelCountRsp '"<<ThriftToJSON(wareLabelCountRsp)<<"'"<<endl;
        return ;
    }

    void getWareLabelList(WareLabelListRsp& wareLabelListRsp, const WareLabelListReq& wareLabelListReq){
        LOG_WARN<<"getWareLabelList request is: WareLabelListReq '"<<ThriftToJSON(wareLabelListReq)<<"'"<<endl;
        PerfWatch perfWatch("getWareLabelList");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        MysqlQueryHelper qh(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        vector<WareLabel> wareLabelList;
        int ret = couponHelper.getWareLabelList(wareLabelList,wareLabelListReq.cond,wareLabelListReq.offset,wareLabelListReq.rows);
        if(-9999 == ret ){
            wareLabelListRsp.error = coupon::Error::OK,
            wareLabelListRsp.errmsg = "success";
            return;
        }

        if(0 != ret){
            wareLabelListRsp.error = Error::MYSQL_EXECUTE_ERROR;
            wareLabelListRsp.errmsg = couponHelper.getError();
            return ;
        }

        wareLabelListRsp.error = coupon::Error::OK,
        wareLabelListRsp.errmsg = "success";
        wareLabelListRsp.wareLabelList = wareLabelList;

        LOG_WARN<<"getWareLabelList response is: WareLabelListRsp '"<<ThriftToJSON(wareLabelListRsp)<<"'"<<endl;
        return ;
    }

    void getWareLabelWaresCount(WareLabelWaresCountRsp& wareLabelWaresCountRsp, const WareLabelWaresCountReq& wareLabelWaresCountReq){
        LOG_WARN<<"getWareLabelWaresCount request is: WareLabelWaresCountReq '"<<ThriftToJSON(wareLabelWaresCountReq)<<"'"<<endl;
        PerfWatch perfWatch("getWareLabelWaresCount");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        MysqlQueryHelper qh(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        int ret = qh.runSelect(string("")
            +" select count(*) "
            +" from " + ware_label_wares_table
            +" " + getWhereCondition(wareLabelWaresCountReq.cond)
            +" " + getOrderByCondition(wareLabelWaresCountReq.cond)
        );

        if(-9999 == ret){
            wareLabelWaresCountRsp.error = coupon::Error::OK,
            wareLabelWaresCountRsp.errmsg = "success";
            return;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql exe queryCouponGroupIds"<< endl;
            LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = qh.getError();
            throw io;
        }

        wareLabelWaresCountRsp.error = coupon::Error::OK,
        wareLabelWaresCountRsp.errmsg = "success";
        wareLabelWaresCountRsp.count = qh.getIntegerField("count(*)");

        LOG_WARN<<"getWareLabelWaresCount response is: WareLabelWaresCountRsp '"<<ThriftToJSON(wareLabelWaresCountRsp)<<"'"<<endl;
        return ;
    }

    void getWareLabelWaresList(WareLabelWaresListRsp& wareLabelWaresListRsp, const WareLabelWaresListReq& wareLabelWaresListReq){
        LOG_WARN<<"getWareLabelWaresList request is: WareLabelWaresListReq '"<<ThriftToJSON(wareLabelWaresListReq)<<"'"<<endl;
        PerfWatch perfWatch("getWareLabelWaresList");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        MysqlQueryHelper qh(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        vector<WareLabelWares> wareLabelWaresList;
        int ret = couponHelper.getWareLabelWaresList(wareLabelWaresList,wareLabelWaresListReq.cond,wareLabelWaresListReq.offset,wareLabelWaresListReq.rows);
        if(ret == -9999){
            wareLabelWaresListRsp.error = coupon::Error::OK,
            wareLabelWaresListRsp.errmsg = "success";
            return;
        }

        if(0 != ret){
            wareLabelWaresListRsp.error = Error::MYSQL_EXECUTE_ERROR;
            wareLabelWaresListRsp.errmsg = couponHelper.getError();
            return ;
        }

        wareLabelWaresListRsp.error = coupon::Error::OK,
        wareLabelWaresListRsp.errmsg = "success";
        wareLabelWaresListRsp.wareLabelWaresList = wareLabelWaresList;

        LOG_WARN<<"getWareLabelWaresList response is: WareLabelWaresListRsp '"<<ThriftToJSON(wareLabelWaresListRsp)<<"'"<<endl;
        return ;
    }

    void batchUserCoupon(BatchUserCouponRsp& batchUserCouponRsp, const BatchUserCouponReq& batchUserCouponReq){
        LOG_WARN<<"batchUserCoupon request is: BatchUserCouponReq '"<<ThriftToJSON(batchUserCouponReq)<<"'"<<endl;
        PerfWatch perfWatch("batchUserCoupon");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        MysqlQueryHelper qh(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        if(batchUserCouponReq.codeList.empty()){
            batchUserCouponRsp.error = Error::INVALID_PARAMS;
            batchUserCouponRsp.errmsg = "codeList is empty";
            return ;
        }

        map<string,UserCoupon> userCouponMap;
        int ret = couponHelper.queryUserCoupons(batchUserCouponReq.codeList,userCouponMap);
        if(-9999 == ret){
            batchUserCouponRsp.error = coupon::Error::OK,
            batchUserCouponRsp.errmsg = "success";
            batchUserCouponRsp.userCouponMap = userCouponMap;
            return;
        }

        if(0 != ret){
            batchUserCouponRsp.error = Error::MYSQL_EXECUTE_ERROR;
            batchUserCouponRsp.errmsg = couponHelper.getError();
            return ;
        }

        batchUserCouponRsp.error = coupon::Error::OK,
        batchUserCouponRsp.errmsg = "success";
        batchUserCouponRsp.userCouponMap = userCouponMap;

        LOG_WARN<<"batchUserCoupon response is: BatchUserCouponRsp '"<<ThriftToJSON(batchUserCouponRsp)<<"'"<<endl;
        return ;
    }
};

class CouponAdminServiceCloneFactory : virtual public CouponAdminServiceIfFactory {
public:
    virtual ~CouponAdminServiceCloneFactory() {}

    virtual CouponAdminServiceIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo){
        boost::shared_ptr<TSocket> sock = boost::dynamic_pointer_cast<TSocket>(connInfo.transport);
        LOG_INFO << "Incoming connection\n";
        LOG_INFO << "\tSocketInfo: "  << sock->getSocketInfo() << "\n";
        LOG_INFO << "\tPeerHost: "    << sock->getPeerHost() << "\n";
        LOG_INFO << "\tPeerAddress: " << sock->getPeerAddress() << "\n";
        LOG_INFO << "\tPeerPort: "    << sock->getPeerPort() << "\n";
        return new CouponAdminServiceHandler;
    }

    virtual void releaseHandler( ::couponAdmin::CouponAdminServiceIf* handler) {
        delete handler;
    }
};

