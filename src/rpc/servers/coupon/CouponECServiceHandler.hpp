/*
 * author:charlee
 */

#include <algorithm>
#include <cmath>
#include <time.h>
#include "JsonHelper.h"
#include "JsonMacro.h"
#include "ThriftHelper.h"
#include "MysqlWrapperPool.h"
#include "RedisWrapperPool.h"
#include "MysqlQueryHelper.h"
#include "sdk-cpp/ec/CouponECService.h"
#include "CouponCode.h"
#include "CouponHelper.h"
#include "CouponConfig.h"
#include "Singleton.h"
#include "System.h"
#include "RedisLock.h"

using namespace std;
using namespace coupon;
using namespace couponEC;
using namespace System;

class CouponECServiceHandler : public CouponECServiceIf {
private:
    //<coupon_group_id, coupon_id>
    using union_id_type = pair<unsigned int,unsigned int>;
    MysqlWrapperPool * mpool;
    RedisWrapperPool * rpool;

    string  redis_ip_  ;
    int     redis_port_  ;
    int     redis_timeout_  ;
    int     redis_poolsize_ ;

    time_t OriginOfToday(){
        time_t t = time(NULL);
        struct tm *local = localtime(&t);
        local->tm_hour = 0;
        local->tm_min = 0;
        local->tm_sec = 0;

        time_t origin = mktime(local);
        return origin;
    }

public:
    CouponECServiceHandler() {
        redis_ip_       = couponConfig.getRedisIp();
        redis_port_     = couponConfig.getRedisPort();
        redis_timeout_  = couponConfig.getRedisTimeout();
        redis_poolsize_ = couponConfig.getECTaskThreadsNum() * 2;

        string  mysql_ip_       = couponConfig.getMysqlIp();
        int     mysql_port_     = couponConfig.getMysqlPort();
        string  mysql_user_     = couponConfig.getMysqlUser();
        string  mysql_passwd_   = couponConfig.getMysqlPasswd();
        string  mysql_database_ = couponConfig.getMysqlDatabase();
        int     mysql_poolsize_ = couponConfig.getECTaskThreadsNum() * 2;

        mpool = MysqlWrapperPool::GetInstance(mysql_ip_,mysql_port_,mysql_user_,mysql_passwd_,mysql_database_,mysql_poolsize_);
        rpool = RedisWrapperPool::GetInstance(redis_ip_,redis_port_,redis_timeout_,redis_poolsize_);
    }

    void getServerTime(ServerTimeRsp& serverTimeRsp, const ServerTimeReq& serverTimeReq){
        serverTimeRsp.error = Error::OK;
        serverTimeRsp.errmsg = "success";
        serverTimeRsp.local_time = serverTimeReq.local_time;
        serverTimeRsp.server_time = time(NULL);
    }

    void getDefaultConfig(GetDefaultConfigRsp& getDefaultConfigRsp,const GetDefaultConfigReq& getDefaultConfigReq){
        LOG_WARN<<"getDefaultConfig request is: GetDefaultConfigReq '"<<ThriftToJSON(getDefaultConfigReq)<<"'"<<endl;
        PerfWatch perfWatch("getDefaultConfig");
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
            LOG_ERROR<<"error occured when redis execute, ret = "<<ret<< endl;
            getDefaultConfigRsp.error = coupon::Error::FAILED,
            getDefaultConfigRsp.errmsg = "failed";
            return;
        }

        DefaultConfig defaultConfig;
        JSONToThrift(json_config,&defaultConfig);

        getDefaultConfigRsp.error = coupon::Error::OK,
        getDefaultConfigRsp.errmsg = "success";
        getDefaultConfigRsp.config = defaultConfig;

        LOG_WARN<<"getDefaultConfig response is: getDefaultConfigRsp '"<<ThriftToJSON(getDefaultConfigRsp)<<"'"<<endl;
        return;
    }

    void decodeCode(DecodeCodeRsp& decodeCodeRsp, const DecodeCodeReq& decodeCodeReq){
        unsigned int coupon_group_id, coupon_id;
        int ret = CouponCode::Decode(decodeCodeReq.code,coupon_group_id,coupon_id);
        if(CouponCode::Errno::OK != ret){
            LOG_ERROR<<decodeCodeReq.code<<" code decode error"<< endl;
            decodeCodeRsp.error = Error::CODE_DECODE_ERROR;
            decodeCodeRsp.errmsg = "code decode error";
            decodeCodeRsp.code = decodeCodeReq.code;
            return;
        }

        if(0 == coupon_group_id || 0 == coupon_id){
            LOG_ERROR<<decodeCodeReq.code<<"invalid code:"<<decodeCodeReq.code<< endl;
            decodeCodeRsp.error = Error::INVALID_CODE;
            decodeCodeRsp.errmsg = "invalid code";
            decodeCodeRsp.code = decodeCodeReq.code;
            return;
        }

        decodeCodeRsp.error = Error::OK;
        decodeCodeRsp.errmsg = "success";
        decodeCodeRsp.code = decodeCodeReq.code;
        decodeCodeRsp.coupon_group_id = coupon_group_id;
        decodeCodeRsp.coupon_id = coupon_id;
    }

    void getDrawnCountToday(DrawnCountTodayRsp& drawnCountTodayRsp,const DrawnCountTodayReq& drawnCountTodayReq){
        LOG_WARN<<"getDrawnCountToday request is: DrawnCountTodayReq '"<<ThriftToJSON(drawnCountTodayReq)<<"'"<<endl;
        PerfWatch perfWatch("getDrawnCountToday");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        MysqlQueryHelper queryHelper(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        //校验组ID,如果这个组在表中不存在
        CouponGroup couponGroup;
        int ret = couponHelper.queryCouponGroup(drawnCountTodayReq.coupon_group_id,couponGroup);
        if(-9999 == ret){
            LOG_ERROR<<"ret="<<ret<<" queryCouponGroup no any expected data!"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            drawnCountTodayRsp.error = Error::COUPON_GROUP_NOT_EXISTS;
            drawnCountTodayRsp.errmsg = "no coupon group you expected";
            return;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        //不在领取有效期内
        if(time(NULL) < couponGroup.start_draw_time || time(NULL) > couponGroup.end_draw_time){
            LOG_ERROR<<time(NULL)<<" can't draw:";
            drawnCountTodayRsp.error = coupon::Error::OK;
            drawnCountTodayRsp.errmsg = "success";
            drawnCountTodayRsp.count = 0;
            return;
        }

        ret = queryHelper.runSelect(string("")
            +" select count(*) "
            +" from " + coupon_table_prefix + to_string(drawnCountTodayReq.coupon_group_id)
            +" where user_id = " + to_string(drawnCountTodayReq.user_id)
            +" and UNIX_TIMESTAMP(drawn_time) > " + to_string(OriginOfToday())
            +" and UNIX_TIMESTAMP(drawn_time) <= " + to_string(OriginOfToday() + 24*3600)
        );

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql exe runSelect"<< endl;
            LOG_ERROR<<"the error detail is that: "<<queryHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = queryHelper.getError();
            throw io;
        }

        drawnCountTodayRsp.error = coupon::Error::OK;
        drawnCountTodayRsp.errmsg = "success";
        drawnCountTodayRsp.count = queryHelper.getIntegerField("count(*)");

        LOG_WARN<<"getDrawnCountToday response is: DrawnCountTodayRsp '"<<ThriftToJSON(drawnCountTodayRsp)<<"'"<<endl;
        return;
    }

    void getUserLeftCouponsCount(UserLeftCouponsCountRsp& userLeftCouponsCountRsp,const UserLeftCouponsCountReq& userLeftCouponsCountReq){
        LOG_WARN<<"getUserLeftCouponsCount request is: UserLeftCouponsCountReq '"<<ThriftToJSON(userLeftCouponsCountReq)<<"'"<<endl;
        PerfWatch perfWatch("getUserLeftCouponsCount");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        MysqlQueryHelper queryHelper(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected";
            throw io;
        }

        //校验组ID,如果这个组在表中不存在
        CouponGroup couponGroup;
        int ret = couponHelper.queryCouponGroup(userLeftCouponsCountReq.coupon_group_id,couponGroup);
        if(-9999 == ret){
            LOG_ERROR<<"ret="<<ret<<" queryCouponGroup no any expected data!"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            userLeftCouponsCountRsp.error = Error::COUPON_GROUP_NOT_EXISTS;
            userLeftCouponsCountRsp.errmsg = "no coupon group you expected";
            return;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        //不在领取有效期内
        if(time(NULL) < couponGroup.start_draw_time || time(NULL) > couponGroup.end_draw_time){
            LOG_ERROR<<time(NULL)<<" can't draw:";
            userLeftCouponsCountRsp.error = coupon::Error::OK;
            userLeftCouponsCountRsp.errmsg = "success";
            userLeftCouponsCountRsp.left_count = 0;
            return;
        }

        ret = queryHelper.runSelect(string("")
            +" select count(*) "
            +" from " + coupon_table_prefix + to_string(userLeftCouponsCountReq.coupon_group_id)
            +" where user_id = " + to_string(userLeftCouponsCountReq.user_id)
        );

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql exe runSelect"<< endl;
            LOG_ERROR<<"the error detail is that: "<<queryHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = queryHelper.getError();
            throw io;
        }

        userLeftCouponsCountRsp.error = coupon::Error::OK;
        userLeftCouponsCountRsp.errmsg = "success";
        userLeftCouponsCountRsp.left_count = couponGroup.can_draw_count - queryHelper.getIntegerField("count(*)");
        if(userLeftCouponsCountRsp.left_count < 0)
        	userLeftCouponsCountRsp.left_count = 0;

        LOG_WARN<<"getUserLeftCouponsCount response is: UserLeftCouponsCountRsp '"<<ThriftToJSON(userLeftCouponsCountRsp)<<"'"<<endl;
        return;
    }

    void getClientCoupons(ClientCouponsRsp& clientCouponsRsp,const ClientCouponsReq& clientCouponsReq){
        LOG_WARN<<"getClientCoupons request is: ClientCouponsReq '"<<ThriftToJSON(clientCouponsReq)<<"'"<<endl;
        PerfWatch perfWatch("getClientCoupons");
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

        int count = 0;
        MultipleCondition cond;
        cond.andCondList.push_back(string("coupon_group_id = ") + to_string(clientCouponsReq.coupon_group_id));
        cond.andCondList.push_back(string("client_id = '") + to_string(clientCouponsReq.client_id) + "'") ;
        int ret = couponHelper.userCouponsCount(cond,count);
        if(0 != ret ){
            LOG_ERROR<<"mysql exe error"<< endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = "mysql exe error";
            throw io;
        }

        clientCouponsRsp.error = coupon::Error::OK;
        clientCouponsRsp.errmsg = "success";
        clientCouponsRsp.count = count;

        LOG_WARN<<"getClientCoupons response is: ClientCouponsRsp '"<<ThriftToJSON(clientCouponsRsp)<<"'"<<endl;
        return;
   }

    void checkArgot(CheckArgotRsp& checkArgotRsp,const CheckArgotReq& checkArgotReq){
        LOG_WARN<<"checkArgot request is: CheckArgotReq '"<<ThriftToJSON(checkArgotReq)<<"'"<<endl;
        PerfWatch perfWatch("checkArgot");
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
        const char *p = checkArgotReq.argot.c_str();
        unsigned int len = checkArgotReq.argot.length();
        for(const char* ptr = p;ptr<p+len;ptr++){
            if(*ptr >= 'a' && *ptr <= 'z'){
                count++;
            }
        }

        if(count == 11 || checkArgotReq.argot == ""){
            LOG_ERROR<<"the error detail is that,invalid argot:"<<checkArgotReq.argot<<endl;
            checkArgotRsp.error = coupon::Error::INVALID_ARGOT;
            checkArgotRsp.errmsg = "invalid argot,all should be lower case";
            return;
        }

        vector<CouponGroup> couponGroupList;
        vector<CouponGroup> validCouponGroupList;
        MultipleCondition cond;
        cond.andCondList.push_back(string("binary argot = '")+checkArgotReq.argot+"'");
        cond.andCondList.push_back(string("verify_status = 2"));
        cond.andCondList.push_back(string("scene_type = 1"));
        cond.andCondList.push_back(string("UNIX_TIMESTAMP(create_time) > ")+to_string(time(NULL) - 365 * 24 * 3600));

        couponHelper.getCouponGroupList(couponGroupList,cond,0,999999);
        for(auto x:couponGroupList){
            if(time(NULL) < x.end_draw_time + 180*24*3600){
                validCouponGroupList.push_back(x);
            }
        }

        if(validCouponGroupList.size() == 0){
            LOG_ERROR<<"the error detail is that,invalid argot:"<<checkArgotReq.argot<<endl;
            checkArgotRsp.error = coupon::Error::INVALID_ARGOT;
            checkArgotRsp.errmsg = "invalid argot";
            return;
        }

        if(validCouponGroupList.size() > 1){
            LOG_ERROR<<"the error detail is that,argot in 180 days:"<<checkArgotReq.argot<<endl;
            checkArgotRsp.error = coupon::Error::ARGOT_IN_180_DAYS;
            checkArgotRsp.errmsg = "argot in 180 days";
            return;
        }

        checkArgotRsp.error = coupon::Error::OK;
        checkArgotRsp.errmsg = "success";
        checkArgotRsp.argot = checkArgotReq.argot;
        checkArgotRsp.coupon_group_id = validCouponGroupList[0].id;

        LOG_WARN<<"checkArgot response is: CheckArgotRsp '"<<ThriftToJSON(checkArgotRsp)<<"'"<<endl;
        return;
    }

    void drawCoupon(DrawRsp& drawRsp, const DrawReq& drawReq){
        LOG_WARN<<"drawCoupon request is: DrawReq '"<<ThriftToJSON(drawReq)<<"'"<<endl;
        PerfWatch perfWatch("drawCoupon");
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

        //校验参数是否都已经填写
        if(drawReq.user_id <= 0||drawReq.coupon_group_id <= 0){
            LOG_ERROR<<"invalid params:"<<" user_id:"<<drawReq.user_id<<" coupon_group_id:"<<drawReq.coupon_group_id<< endl;
            io.fault = Error::INVALID_PARAMS;
            io.why = "invalid params";
            throw io;
        }

        //校验组ID,如果这个组在表中不存在
        CouponGroup couponGroup;
        int ret = couponHelper.queryCouponGroup(drawReq.coupon_group_id,couponGroup);
        if(-9999 == ret){
            LOG_ERROR<<"ret="<<ret<<" queryCouponGroup no any expected data!"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            drawRsp.error = Error::COUPON_GROUP_NOT_EXISTS;
            drawRsp.errmsg = "no coupon group you expected";
            return;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        //如果没有券可以领了
        if(couponGroup.drawn_count >= couponGroup.max_count){
            LOG_ERROR<<"drawn_count >= max_count "<<couponGroup.drawn_count<<" "<< couponGroup.max_count<< endl;
            drawRsp.error = Error::NO_COUPON_LEFT;
            drawRsp.errmsg = "no coupon left";
            return;
        }

        //进一步到券表中确认
        ret = queryHelper.runSelect(string("")
            +" select count(*) "
            +" from " + coupon_table_prefix + to_string(drawReq.coupon_group_id)
            +" where user_id > 0"
        );

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql exe runSelect"<< endl;
            LOG_ERROR<<"the error detail is that: "<<queryHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = queryHelper.getError();
            throw io;
        }

        int real_drawn_count = queryHelper.getIntegerField("count(*)");
        if(real_drawn_count >= couponGroup.max_count){
            LOG_ERROR<<"real drawn_count >= max_count "<<real_drawn_count<<" "<< couponGroup.max_count<< endl;
            drawRsp.error = Error::NO_COUPON_LEFT;
            drawRsp.errmsg = "real no coupon left";
            return;
        }

        //不在领取有效期内
        if(time(NULL) < couponGroup.start_draw_time){
            LOG_ERROR<<time(NULL)<<" draw time not reach:"<<couponGroup.start_draw_time<< endl;
            drawRsp.error = Error::DRAW_TIME_NOT_REACH;
            drawRsp.errmsg = "draw time not reach";
            return;
        }

        if(time(NULL) > couponGroup.end_draw_time){
            LOG_ERROR<<time(NULL)<<" draw time expired:"<<couponGroup.end_draw_time<< endl;
            drawRsp.error = Error::DRAW_TIME_EXPIRED;
            drawRsp.errmsg = "draw time expired";
            return;
        }

        //查询缓存，用户领取多了码？
        CacheCodes cacheCodes;//用户缓存的所有券码
        const string cache_codes_key = cache_codes_prefix + to_string(drawReq.user_id);
        string json_cache_codes = "";
        ret = rwrapper->getValue(cache_codes_key,json_cache_codes);
        if(0 == ret){
            JSONToThrift(json_cache_codes,&cacheCodes);
            //反解所有券码
            unsigned int count = 0;
            for(int i=0;i<(int)cacheCodes.codes.size();i++){
                unsigned int _coupon_group_id, _coupon_id;
                int ret = CouponCode::Decode(cacheCodes.codes[i],_coupon_group_id,_coupon_id);
                if(CouponCode::Errno::OK != ret)
                {
                    LOG_ERROR<<cacheCodes.codes[i]<<" code decode error"<< endl;
                    io.fault = Error::CODE_DECODE_ERROR;
                    io.why = "code decode error";
                    throw io;
                }

                if(drawReq.coupon_group_id == (int)_coupon_group_id){
                    count++;
                }
            }

            if((int)count >= (int)couponGroup.can_draw_count){
                LOG_ERROR<<"you have drawn all you can draw "<<count<<" >= "<< couponGroup.can_draw_count<< endl;
                drawRsp.error = Error::DRAWN_ALL_YOU_CAN;
                drawRsp.errmsg = "you have drawn all you can draw";
                return;
            }
        }

        //缓存中查询到没有领完，需要进一步到UserCoupons表确认
        int count = 0;
        ret = couponHelper.userCouponsCount(drawReq.user_id,count,drawReq.coupon_group_id);
        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute userCouponsCount"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        if(count >= couponGroup.can_draw_count){
            LOG_ERROR<<"you have drawn all you can draw."<<count<<">="<< couponGroup.can_draw_count<< endl;
            drawRsp.error = Error::DRAWN_ALL_YOU_CAN;
            drawRsp.errmsg = "you have drawn all you can draw";
            return;
        }

        //如果对应的券表不存在，则产生一张券表
        ret = couponHelper.createCouponTable(drawReq.coupon_group_id);
        if(ret < 0){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute..."<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        unsigned int coupon_id ;
        string code;
        //如果是领取型券/口令型券/批量发放型
        if(0 == couponGroup.scene_type ||1 == couponGroup.scene_type
            ||2 == couponGroup.scene_type||4 == couponGroup.scene_type){
            //如果是批量发放型
            if(4 == couponGroup.scene_type && 4 != drawReq.scene_type){
                LOG_ERROR<<"invalid scene type: couponGroup.scene_type = "<<couponGroup.scene_type
                        <<" drawReq.scene_type="<<drawReq.scene_type<< endl;
                drawRsp.error = Error::ILLEGAL_SCENE_TYPE;
                drawRsp.errmsg = "invalid scene type";
                return;
            }

            //如果是口令型券，口令不正确
            if(1 == couponGroup.scene_type && drawReq.argot != couponGroup.argot){
                LOG_ERROR<<"invalid argot:"<<couponGroup.argot<< endl;
                drawRsp.error = Error::INVALID_ARGOT;
                drawRsp.errmsg = "invalid argot";
                return;
            }

            {
                string resource = string("drawCoupon_") + to_string(drawReq.user_id);
                RedisLock lock(this->redis_ip_,this->redis_port_,resource,1000);
                if(!lock.Acquired()){
                    LOG_ERROR<<lock.getError()<<endl;
                    drawRsp.error = Error::CAN_NOT_GET_THE_LOCK;
                    drawRsp.errmsg = lock.getError();
                    return;
                }

                //往券表插入一条记录
                couponHelper.drawOneCoupon(drawReq.coupon_group_id,drawReq.user_id,coupon_id,code);

                //修改CouponGroup的记录
                couponHelper.incrDrawnCount(drawReq.coupon_group_id);

                //用户-优惠券表插入一条记录
                couponHelper.insertUserCoupon(drawReq.coupon_group_id,drawReq.user_id,drawReq.client_id,coupon_id,code,
                        drawReq.channel_id,couponGroup.seller_id);
            }
        }else if(3 == couponGroup.scene_type){//如果是领取批量导出型券
            if(drawReq.code.size() != 11){
                LOG_ERROR<<"the error detail is that: invalid code"<<couponHelper.getError()<<endl;
                drawRsp.error = Error::INVALID_CODE;
                drawRsp.errmsg = couponHelper.getError();
                return;
            }

            vector<Coupon> couponList;
            MultipleCondition cond;
            cond.andCondList.push_back(string("code = '")+drawReq.code+"'");
            int ret = couponHelper.getCouponList(couponList,cond,drawReq.coupon_group_id, 0,1);
            if(ret < 0 || 1 != couponList.size()){
                LOG_ERROR<<"the error detail is that: coupon code not exitst"<<couponHelper.getError()<<endl;
                drawRsp.error = Error::COUPON_NOT_EXISTS;
                drawRsp.errmsg = couponHelper.getError();
                return;
            }

            //如果券被人领了
            Coupon coupon = couponList[0];
            if( (coupon.user_id > 0 &&coupon.user_id == drawReq.user_id )){
                LOG_ERROR<<"coupon has been drawn"<<endl;
                drawRsp.error = Error::COUPON_HAS_BEEN_DRAWN;
                drawRsp.errmsg = "coupon has been drawn";
                return;
            }

            if( (coupon.user_id > 0 &&coupon.user_id != drawReq.user_id )){
                LOG_ERROR<<"coupon has been robbed"<<endl;
                drawRsp.error = Error::COUPON_HAS_BEEN_ROBBED;
                drawRsp.errmsg = "coupon has been robbed";
                return;
            }

            if(coupon.code != drawReq.code){
                LOG_ERROR<<drawReq.code<<" != "<<coupon.code<< endl;
                drawRsp.error = Error::INVALID_CODE;
                drawRsp.errmsg = "invalid code";
                return;
            }

            //反解券码
            unsigned int _coupon_group_id, _coupon_id;
            CouponCode::Decode(drawReq.code,_coupon_group_id,_coupon_id);
            if((int)_coupon_group_id != coupon.coupon_group_id || (int)_coupon_id != coupon.coupon_id)
            {
                LOG_ERROR<<"coupon_group_id"<<coupon.coupon_group_id<<"coupon_id="<<coupon.coupon_id<<" code decode error"<< endl;
                io.fault = Error::CODE_DECODE_ERROR;
                io.why = "code decode error";
                throw io;
            }

            coupon_id = coupon.coupon_id;
            code = coupon.code;
            {
                string resource = string("drawCoupon_") + to_string(drawReq.user_id);
                RedisLock lock(this->redis_ip_,this->redis_port_,resource,1000);
                if(!lock.Acquired()){
                    LOG_ERROR<<lock.getError()<<endl;
                    drawRsp.error = Error::CAN_NOT_GET_THE_LOCK;
                    drawRsp.errmsg = lock.getError();
                    return;
                }

                //修改CouponGroup的记录
                couponHelper.incrDrawnCount(drawReq.coupon_group_id);

                //修改记录
                int ret = queryHelper.runUpdate(string("")
                    +" update "+coupon_table_prefix + to_string(drawReq.coupon_group_id)
                    +" set user_id = " + to_string(drawReq.user_id)
                    +" ,drawn_time=now()"
                    +" where id = "+to_string(coupon.coupon_id)
                );

                if(ret <= 0){
                    LOG_ERROR<<"ret= "<<ret<<" coupon_id="<<coupon.coupon_id<<" error occured when mysql execute.."<< endl;
                    LOG_ERROR<<"the error detail is that: "<<queryHelper.getError()<<endl;
                    io.fault = Error::MYSQL_EXECUTE_ERROR;
                    io.why = "error occured when mysql execute " + queryHelper.getError();
                    throw io;
                }

                //用户-优惠券表插入一条记录
                couponHelper.insertUserCoupon(drawReq.coupon_group_id,drawReq.user_id,drawReq.client_id,coupon.coupon_id,coupon.code,
                        drawReq.channel_id,couponGroup.seller_id);
            }
        }

        //插入缓存信息
        CacheCoupon cacheCoupon;
        cacheCoupon.coupon_group_id = drawReq.coupon_group_id;
        cacheCoupon.coupon_id = coupon_id;
        cacheCoupon.code = code;
        cacheCoupon.user_id = drawReq.user_id;
        cacheCoupon.client_id = drawReq.client_id;
        cacheCoupon.create_time = couponGroup.create_time;
        cacheCoupon.update_time = time(NULL);
        cacheCoupon.use_status = 0;
        cacheCoupon.drawn_time = time(NULL);
        cacheCoupon.frozen_time = 0;
        cacheCoupon.payed_time = 0;
        cacheCoupon.order_create_time = 0;
        cacheCoupon.order_id = "";
        if(!couponGroup.is_duration_type){
            cacheCoupon.start_use_time = couponGroup.start_use_time;
            cacheCoupon.end_use_time = couponGroup.end_use_time;
        }else{
            cacheCoupon.start_use_time = cacheCoupon.drawn_time;
            cacheCoupon.end_use_time = cacheCoupon.drawn_time + couponGroup.duration_value * 24 * 3600;
        }

        cacheCoupon.name = couponGroup.name ;                   //名称
        cacheCoupon.title = couponGroup.title;                  //标题
        cacheCoupon.comment = couponGroup.comment;              //描述
        cacheCoupon.ware_label_id = couponGroup.ware_label_id;  //商品标签ID
        cacheCoupon.favor_type = couponGroup.favor_type ;       //优惠类型
        cacheCoupon.scope_type = couponGroup.scope_type ;       //使用范围
        cacheCoupon.sub_type = couponGroup.sub_type ;           //子类型
        cacheCoupon.scene_type = couponGroup.scene_type ;       //场景类型
        cacheCoupon.full = couponGroup.full;                    //满
        cacheCoupon.favor = couponGroup.favor;                  //减
        cacheCoupon.rate = couponGroup.rate ;                   //折扣率
        cacheCoupon.argot = couponGroup.argot;                  //暗语
        cacheCoupon.seller_id = couponGroup.seller_id;          //店铺
        cacheCoupon.url = couponGroup.url   ;                   //连接
        cacheCoupon.img = couponGroup.img   ;
        cacheCoupon.slug = couponGroup.slug   ;
        cacheCoupon.img_width = couponGroup.img_width   ;
        cacheCoupon.img_height = couponGroup.img_height   ;
        cacheCoupon.button_text = couponGroup.button_text   ;
        cacheCoupon.button_jump = couponGroup.button_jump   ;
        cacheCoupon.jump_label = couponGroup.jump_label   ;
        cacheCoupon.jump_data = couponGroup.jump_data   ;
        cacheCoupon.argot_jump_label = couponGroup.argot_jump_label   ;
        cacheCoupon.argot_jump_data = couponGroup.argot_jump_data   ;
        cacheCoupon.version = couponGroup.version   ;
        cacheCoupon.description = couponGroup.description   ;

        if(cacheCodes.codes.size() >= cache_codes_capacity){
            vector<string>::iterator k = cacheCodes.codes.begin();
            cacheCodes.codes.erase(k); // 删除第一个元素
        }

        cacheCodes.codes.push_back(code);

        ret = rwrapper->setValue(cache_codes_key,ThriftToJSON(cacheCodes));
        if(0 != ret){
            LOG_ERROR<<"error occured when redis execute"<< endl;
        }

        string user_cache_coupon_key = cache_coupon_prefix + to_string(drawReq.user_id) + "_" + code;
        ret = rwrapper->setValue(user_cache_coupon_key,ThriftToJSON(cacheCoupon),CACHE_COUPON_EX);
        if(0 != ret){
            LOG_ERROR<<"error occured when redis execute"<< endl;
        }

        drawRsp.error = Error::OK;
        drawRsp.errmsg = "success";
        drawRsp.code = code;
        drawRsp.cacheCoupon = cacheCoupon;

        LOG_WARN<<"drawCoupon response is: DrawRsp '"<<ThriftToJSON(drawRsp)<<"'"<<endl;
        return;
    }

    //回滚优惠券，解冻
    void rollbackCoupon(RollbackRsp& rollbackRsp, const RollbackReq& rollbackReq){
        LOG_WARN<<"rollbackCoupon request is: RollbackReq '"<<ThriftToJSON(rollbackReq)<<"'"<<endl;
        PerfWatch perfWatch("rollbackCoupon");
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
            LOG_ERROR<<"Redis connection disconnected "<<rwrapper<< endl;
            io.fault = Error::REDIS_DISCONNECTED;
            io.why = "Redis connection disconnected";
            throw io;
        }

        //校验参数是否都已经填写
        if(rollbackReq.user_id <= 0||"" ==rollbackReq.code ||"" == rollbackReq.order_id){
            LOG_ERROR<<"invalid params:"<<" user_id:"<<rollbackReq.user_id<<" code:"<<rollbackReq.code
                     <<" order_id:"<<rollbackReq.order_id<< endl;
            io.fault = Error::INVALID_PARAMS;
            io.why = "invalid params";
            throw io;
        }

        //反算券码
        unsigned int coupon_group_id, coupon_id;
        int ret = CouponCode::Decode(rollbackReq.code,coupon_group_id,coupon_id);
        if(CouponCode::Errno::OK != ret){
            LOG_ERROR<<rollbackReq.code<<" code decode error"<< endl;
            io.fault = Error::CODE_DECODE_ERROR;
            io.why = "code decode error";
            throw io;
        }

        //校验组ID,如果这个组在表中不存在，同时也取得相关时间
        CouponGroup couponGroup;
        ret = couponHelper.queryCouponGroup(coupon_group_id,couponGroup);
        if(-9999 == ret && 0 != ret){
            LOG_ERROR<<"coupon_group_id="<<coupon_group_id<<" the coupon group doesn't online"<< endl;
            io.fault = Error::COUPON_GROUP_NOT_EXISTS;
            io.why = "the coupon group doesn't online";
            throw io;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute queryCouponGroup"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        //如果是持续几天有效的券，还要查询券表中这张券的领取时间
        Coupon coupon;
        ret = couponHelper.queryCoupon(coupon_group_id,coupon_id,coupon);
        if(-9999 == ret && 0 != ret){
            LOG_ERROR<<"coupon_group_id="<<coupon_group_id<<" the coupon doesn't online"<< endl;
            io.fault = Error::COUPON_ID_NOT_EXISTS;
            io.why = "the coupon doesn't online";
            throw io;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute queryCoupon"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        //校验订单编号
        if(coupon.order_id != rollbackReq.order_id){
            LOG_ERROR<<"order_id："<<coupon.order_id<<" rollbackReq.order_id:"<<rollbackReq.order_id<< endl;
            io.fault = Error::ORDER_ID_NOT_MATCH;
            io.why = "order_id doesn't match";
            throw io;
        }

        //先查询用户-券表缓存
        bool cacheCouponExists = false;
        CacheCodes cacheCodes;
        CacheCoupon cacheCoupon;
        const string cache_codes_key = cache_codes_prefix + to_string(rollbackReq.user_id);
        string json_cache_codes = "";
        ret = rwrapper->getValue(cache_codes_key,json_cache_codes);
        if(0 == ret){
            JSONToThrift(json_cache_codes,&cacheCodes);
            auto it = find(cacheCodes.codes.begin(),cacheCodes.codes.end(),rollbackReq.code);
            if(cacheCodes.codes.end() != it){
                string user_cache_coupon_key = cache_coupon_prefix + to_string(rollbackReq.user_id) + "_"+rollbackReq.code;
                string json_cacheCoupon="";
                ret = rwrapper->getValue(user_cache_coupon_key,json_cacheCoupon);
                if(0 == ret){
                    cacheCouponExists = true;

                    JSONToThrift(json_cacheCoupon,&cacheCoupon);
                    if(USE_STATUS::FROZEN != cacheCoupon.use_status){
                        LOG_ERROR<<("the coupon in redis " + rollbackReq.code + " not be forzen")<< endl;
                        rollbackRsp.error = Error::NOT_BEEN_FROZEN;
                        rollbackRsp.errmsg = "the coupon not be forzen";
                        return;
                    }
                }
            }
        }

        //查询用户-券表中该用户是否有这张券,进一步确认
        //这张券是否处于已冻结状态
        int use_status = couponHelper.userCouponStatus(rollbackReq.user_id ,rollbackReq.code);
        if(USE_STATUS::FROZEN != use_status){
            LOG_ERROR<<("the coupon " + rollbackReq.code + " not be forzen")<< endl;
            rollbackRsp.error = Error::NOT_BEEN_FROZEN;
            rollbackRsp.errmsg = "the coupon not be forzen";
            return;
        }

        //修改用户-优惠券表，标记为已领取或者已过期,3过期，0已领取
        if(couponGroup.is_duration_type){
            use_status = time(NULL) > coupon.drawn_time + couponGroup.duration_value * 24 * 3600 ? USE_STATUS::EXPIRED : USE_STATUS::DRAWN;
        }else{
            use_status = time(NULL) > couponGroup.end_use_time ? USE_STATUS::EXPIRED : USE_STATUS::DRAWN;
        }

        ret = queryHelper.runUpdate(string("")
            +" update " + user_coupons_table
            +" set update_time=now(),use_status =" + to_string(use_status)
            +" where user_id = " + to_string(rollbackReq.user_id)
            +" and code = '" + rollbackReq.code + "'"
        );

        if(ret <= 0){
            LOG_ERROR<<"ret= "<<ret<<" user_id="<<rollbackReq.user_id<<" error occured when mysql execute.."<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        //修改券表，擦除 order_id/order_create_time/frozen_time
        ret = queryHelper.runUpdate(string("")
                +" update " + coupon_table_prefix + to_string(coupon_group_id)
                +" set order_id = NULL,order_create_time=NULL,frozen_time=NULL"
                +" where id = "+to_string(coupon_id)
        );

        if(ret <= 0){
            LOG_ERROR<<"ret= "<<ret<<" user_id="<<rollbackReq.user_id<<" error occured when mysql execute.."<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        //构建缓存信息
        auto it = find(cacheCodes.codes.begin(),cacheCodes.codes.end(),rollbackReq.code);
        if(cacheCodes.codes.end() == it){
            if(cacheCodes.codes.size() >= cache_codes_capacity){
                vector<string>::iterator k = cacheCodes.codes.begin();
                cacheCodes.codes.erase(k); // 删除第一个元素
            }

            cacheCodes.codes.push_back(rollbackReq.code);

            ret = rwrapper->setValue(cache_codes_key,ThriftToJSON(cacheCodes));
            if(0 != ret){
                LOG_ERROR<<"error occured when redis execute"<< endl;
            }
        }

        if(cacheCouponExists){
            cacheCoupon.order_id = "";
            cacheCoupon.order_create_time = 0;
            cacheCoupon.frozen_time = 0;
            cacheCoupon.update_time = time(NULL);
            cacheCoupon.use_status = use_status;

            string user_cache_coupon_key = cache_coupon_prefix + to_string(rollbackReq.user_id) + "_"+rollbackReq.code;
            ret = rwrapper->setValue(user_cache_coupon_key,ThriftToJSON(cacheCoupon),CACHE_COUPON_EX);
            if(0 != ret){
                LOG_ERROR<<"error occured when redis execute"<< endl;
            }
        }

        rollbackRsp.error = Error::OK;
        rollbackRsp.errmsg = "success";

        LOG_WARN<<"rollbackCoupon response is: RollbackRsp '"<<ThriftToJSON(rollbackRsp)<<"'"<<endl;
        return;
    }

    //冻结优惠券
    void freezeCoupon(FreezeRsp& freezeRsp, const FreezeReq& freezeReq){
        LOG_WARN<<"freezeCoupon request is: FreezeReq '"<<ThriftToJSON(freezeReq)<<"'"<<endl;
        PerfWatch perfWatch("freezeCoupon");
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
            LOG_ERROR<<"Redis connection disconnected "<<rwrapper<< endl;
            io.fault = Error::REDIS_DISCONNECTED;
            io.why = "Redis connection disconnected";
            throw io;
        }

        //校验参数是否都已经填写
        if(freezeReq.user_id <= 0||"" ==freezeReq.code ||"" == freezeReq.order_id){
            LOG_ERROR<<"invalid params:"<<" user_id:"<<freezeReq.user_id<<" code:"<<freezeReq.code
                     <<" order_id:"<<freezeReq.order_id<< endl;
            io.fault = Error::INVALID_PARAMS;
            io.why = "invalid params";
            throw io;
        }

        //反算券码
        unsigned int coupon_group_id, coupon_id;
        int ret = CouponCode::Decode(freezeReq.code,coupon_group_id,coupon_id);
        if(CouponCode::Errno::OK != ret)
        {
            LOG_ERROR<<freezeReq.code<<" code decode error"<< endl;
            io.fault = Error::CODE_DECODE_ERROR;
            io.why = "code decode error";
            throw io;
        }

        //校验组ID,如果这个组在表中不存在，同时也取得相关时间
        CouponGroup couponGroup;
        ret = couponHelper.queryCouponGroup(coupon_group_id,couponGroup);
        if(-9999 == ret && 0 != ret){
            LOG_ERROR<<"coupon_group_id="<<coupon_group_id<<" the coupon group doesn't online"<< endl;
            io.fault = Error::COUPON_GROUP_NOT_EXISTS;
            io.why = "the coupon group doesn't online";
            throw io;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute queryCouponGroup"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why =couponHelper.getError();
            throw io;
        }

        //进一步校验参数
        Coupon coupon;
        ret = couponHelper.queryCoupon(coupon_group_id,coupon_id,coupon);
        if(-9999 == ret && 0 != ret){
            LOG_ERROR<<"coupon_group_id="<<coupon_group_id<<" the coupon doesn't online"<< endl;
            io.fault = Error::COUPON_ID_NOT_EXISTS;
            io.why = "the coupon doesn't online";
            throw io;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute queryCoupon"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        if(freezeReq.user_id != coupon.user_id||coupon.code !=freezeReq.code){
            LOG_ERROR<<"invalid params:"<<" user_id:"<<freezeReq.user_id<<" code:"<<freezeReq.code
                     <<" order_id:"<<freezeReq.order_id
                     <<" real datas:"<<" user_id:"<<coupon.user_id<<" code:"<<coupon.code<<" order_id:"<<coupon.order_id<< endl;
            io.fault = Error::INVALID_PARAMS;
            io.why = "invalid params";
            throw io;
        }

        if("" != coupon.order_id || coupon.frozen_time >0 || coupon.payed_time>0 || coupon.order_create_time >0){
            LOG_ERROR<<"coupon_id="<<coupon_id<<" the coupon is in use"<< endl;
            io.fault = Error::COUPON_IN_USE;
            io.why = "the coupon is in use";
            throw io;

        }

        //先查询用户-券表缓存
        bool cacheCouponExists = false;
        CacheCodes cacheCodes;
        CacheCoupon cacheCoupon;
        const string cache_codes_key = cache_codes_prefix + to_string(freezeReq.user_id);
        string json_cache_codes = "";
        ret = rwrapper->getValue(cache_codes_key,json_cache_codes);
        if(0 == ret){
            JSONToThrift(json_cache_codes,&cacheCodes);
            auto it = find(cacheCodes.codes.begin(),cacheCodes.codes.end(),freezeReq.code);
            if(cacheCodes.codes.end() != it){
                string user_cache_coupon_key = cache_coupon_prefix + to_string(freezeReq.user_id) + "_"+freezeReq.code;
                string json_cacheCoupon="";
                ret = rwrapper->getValue(user_cache_coupon_key,json_cacheCoupon);
                if(0 == ret){
                    cacheCouponExists = true;

                    JSONToThrift(json_cacheCoupon,&cacheCoupon);
                    if(USE_STATUS::DRAWN != cacheCoupon.use_status){
                        LOG_ERROR<<("the coupon in redis " + freezeReq.code + " is in use")<< endl;
                        freezeRsp.error = Error::COUPON_IN_USE;
                        freezeRsp.errmsg = "the coupon is in use";
                        return;
                    }
                }
            }
        }

        //查询用户-券表,这张券是否处于使用中或者过期
        int use_status = couponHelper.userCouponStatus(freezeReq.user_id ,freezeReq.code);
        if(USE_STATUS::DRAWN != use_status){
            LOG_ERROR<<("the coupon " + freezeReq.code + " is in use")<< endl;
            freezeRsp.error = Error::COUPON_IN_USE;
            freezeRsp.errmsg = "the coupon is in use";
            return;
        }

        //进一步检查券表中是否过期
        if(couponGroup.is_duration_type){
            use_status = time(NULL) > coupon.drawn_time + couponGroup.duration_value * 24 * 3600 ? USE_STATUS::EXPIRED : USE_STATUS::DRAWN;
        }else{
            use_status = time(NULL) > couponGroup.end_use_time ? USE_STATUS::EXPIRED : USE_STATUS::DRAWN;
        }

        if(USE_STATUS::DRAWN != use_status){
            LOG_ERROR<<("the coupon " + freezeReq.code + " is expired")<< endl;
            freezeRsp.error = Error::COUPON_EXPIRED;
            freezeRsp.errmsg = "the coupon is expired";
            return;
        }

        //修改用户-券表，修改为被冻结状态
        couponHelper.freezeCoupon(freezeReq.user_id ,freezeReq.code);

        //修改券表
        ret = queryHelper.runUpdate(string("")
            +" update " + coupon_table_prefix + to_string(coupon_group_id)
            +" set order_id = '" + freezeReq.order_id + "'"
            +" ,order_create_time=now()"
            +" ,frozen_time=now()"
            +" ,payed_time = NULL"
            +" where id = "+to_string(coupon_id)
        );

        if(ret <= 0){
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = "error occured when mysql execute";
            LOG_ERROR<<"ret= "<<ret<<" user_id="<<freezeReq.user_id<<" error occured when mysql execute.."<< endl;
            throw io;
        }

        //构建缓存信息
        auto it = find(cacheCodes.codes.begin(),cacheCodes.codes.end(),freezeReq.code);
        if(cacheCodes.codes.end() == it){
            if(cacheCodes.codes.size() >= cache_codes_capacity){
                vector<string>::iterator k = cacheCodes.codes.begin();
                cacheCodes.codes.erase(k); // 删除第一个元素
            }

            cacheCodes.codes.push_back(freezeReq.code);

            ret = rwrapper->setValue(cache_codes_key,ThriftToJSON(cacheCodes));
            if(0 != ret){
                LOG_ERROR<<"error occured when redis execute"<< endl;
            }
        }

        if(cacheCouponExists){
            cacheCoupon.order_id = freezeReq.order_id;
            cacheCoupon.order_create_time = time(NULL);
            cacheCoupon.frozen_time = time(NULL);
            cacheCoupon.update_time = time(NULL);
            cacheCoupon.use_status = USE_STATUS::FROZEN;

            string user_cache_coupon_key = cache_coupon_prefix + to_string(freezeReq.user_id) + "_" + freezeReq.code;
            ret = rwrapper->setValue(user_cache_coupon_key,ThriftToJSON(cacheCoupon),CACHE_COUPON_EX);
            if(0 != ret){
                LOG_ERROR<<"error occured when redis execute"<< endl;
            }
        }

        freezeRsp.error = Error::OK;
        freezeRsp.errmsg = "success";

        LOG_WARN<<"freezeCoupon response is: FreezeRsp '"<<ThriftToJSON(freezeRsp)<<"'"<<endl;
        return;
    }

    //支付优惠券
    void payCoupon(PayRsp& payRsp, const PayReq& payReq){
        LOG_WARN<<"payCoupon request is: PayReq '"<<ThriftToJSON(payReq)<<"'"<<endl;
        PerfWatch perfWatch("payCoupon");
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
            LOG_ERROR<<"Redis connection disconnected"<< endl;
            io.fault = Error::REDIS_DISCONNECTED;
            io.why = "Redis connection disconnected";
            throw io;
        }

        //校验参数是否都已经填写
        if(payReq.user_id <= 0||"" ==payReq.code ||"" == payReq.order_id){
            LOG_ERROR<<"invalid params:"<<" user_id:"<<payReq.user_id<<" code:"<<payReq.code
                     <<" order_id:"<<payReq.order_id<< endl;
            io.fault = Error::INVALID_PARAMS;
            io.why = "invalid params";
            throw io;
        }

        //反算券码
        unsigned int coupon_group_id, coupon_id;
        int ret = CouponCode::Decode(payReq.code,coupon_group_id,coupon_id);
        if(CouponCode::Errno::OK != ret){
            LOG_ERROR<<payReq.code<<" code decode error"<< endl;
            io.fault = Error::CODE_DECODE_ERROR;
            io.why = "code decode error";
            throw io;
        }

        //校验组ID
        CouponGroup couponGroup;
        ret = couponHelper.queryCouponGroup(coupon_group_id,couponGroup);
        if(-9999 == ret && 0 != ret){
            LOG_ERROR<<"coupon_group_id="<<coupon_group_id<<" the coupon group doesn't online"<< endl;
            io.fault = Error::COUPON_GROUP_NOT_EXISTS;
            io.why = "the coupon group doesn't online";
            throw io;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute queryCouponGroup"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        //进一步校验参数
        Coupon coupon;
        ret = couponHelper.queryCoupon(coupon_group_id,coupon_id,coupon);
        if(-9999 == ret && 0 != ret){
            LOG_ERROR<<"coupon_group_id="<<coupon_group_id<<" the coupon doesn't online"<< endl;
            io.fault = Error::COUPON_ID_NOT_EXISTS;
            io.why = "the coupon doesn't online";
            throw io;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute queryCoupon"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        if(payReq.user_id != coupon.user_id||coupon.code !=payReq.code||coupon.order_id != payReq.order_id){
            LOG_ERROR<<"invalid params:"<<" user_id:"<<payReq.user_id<<" code:"<<payReq.code
                     <<" order_id:"<<payReq.order_id
                     <<" real datas:"<<" user_id:"<<coupon.user_id<<" code:"<<coupon.code<<" order_id:"<<coupon.order_id<< endl;
            io.fault = Error::INVALID_PARAMS;
            io.why = "invalid params";
            throw io;
        }

        if(coupon.drawn_time == 0||coupon.frozen_time==0||coupon.payed_time>0){
            LOG_ERROR<<"coupon_group_id="<<coupon_group_id<<" coupon_id="<<coupon_id<<" the coupon not be frozen"<< endl;
            io.fault = Error::NOT_BEEN_FROZEN;
            io.why = "the coupon not be frozen";
            throw io;
        }

        //先查询用户-券表缓存
        bool cacheCouponExists = false;
        CacheCodes cacheCodes;
        CacheCoupon cacheCoupon;
        const string cache_codes_key = cache_codes_prefix + to_string(payReq.user_id);
        string json_cache_codes = "";
        ret = rwrapper->getValue(cache_codes_key,json_cache_codes);
        if(0 == ret){
            JSONToThrift(json_cache_codes,&cacheCodes);
            auto it = find(cacheCodes.codes.begin(),cacheCodes.codes.end(),payReq.code);
            if(cacheCodes.codes.end() != it){
                string user_cache_coupon_key = cache_coupon_prefix + to_string(payReq.user_id) + "_" +payReq.code;
                string json_cacheCoupon="";
                ret = rwrapper->getValue(user_cache_coupon_key,json_cacheCoupon);
                if(0 == ret){
                    cacheCouponExists = true;

                    JSONToThrift(json_cacheCoupon,&cacheCoupon);
                    if(USE_STATUS::FROZEN != cacheCoupon.use_status){
                        LOG_ERROR<<("the coupon in redis " + payReq.code + " not be frozen")<< endl;
                        payRsp.error = Error::NOT_BEEN_FROZEN;
                        payRsp.errmsg = "the coupon not be frozen";
                        return;
                    }
                }
            }
        }

        //查询用户-券表,这张券是否处于被冻结
        int use_status = couponHelper.userCouponStatus(payReq.user_id ,payReq.code);
        if(USE_STATUS::FROZEN != use_status){
            LOG_ERROR<<("the coupon " + payReq.code + " not be frozen")<< endl;
            payRsp.error = Error::COUPON_IN_USE;
            payRsp.errmsg = "the coupon not be frozen";
            return;
        }

        //修改用户-券表，修改为被支付状态
        ret = queryHelper.runUpdate(string("")
            +" update " + user_coupons_table
            +" set update_time=now(),use_status = 2"
            +" where user_id = " + to_string(payReq.user_id)
            +" and code = '" + payReq.code + "'"
        );

        if(ret <= 0){
            LOG_ERROR<<"ret= "<<ret<<" user_id="<<payReq.user_id<<" error occured when mysql execute.."<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        //修改券表
        ret = queryHelper.runUpdate(string("")
            +" update " + coupon_table_prefix + to_string(coupon_group_id)
            +" set payed_time = now()"
            +" where id = "+to_string(coupon_id)
        );

        if(ret <= 0){
            LOG_ERROR<<"ret= "<<ret<<" user_id="<<payReq.user_id<<" error occured when mysql execute.."<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        //修改CouponGroup的记录
        ret = queryHelper.runUpdate(string("")
                +" update " + coupon_group_table
                +" set payed_count = payed_count + 1"
                +" where id = "+to_string(coupon_group_id)
        );

        if(ret <= 0){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute.."<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        //构建缓存信息
        auto it = find(cacheCodes.codes.begin(),cacheCodes.codes.end(),payReq.code);
        if(cacheCodes.codes.end() == it){
            if(cacheCodes.codes.size() >= cache_codes_capacity){
                vector<string>::iterator k = cacheCodes.codes.begin();
                cacheCodes.codes.erase(k); // 删除第一个元素
            }

            cacheCodes.codes.push_back(payReq.code);

            ret = rwrapper->setValue(cache_codes_key,ThriftToJSON(cacheCodes));
            if(0 != ret){
                LOG_ERROR<<"error occured when redis execute"<< endl;
            }
        }

        if(cacheCouponExists){
            cacheCoupon.payed_time = time(NULL);
            cacheCoupon.update_time = time(NULL);
            cacheCoupon.use_status = USE_STATUS::PAYED;

            string user_cache_coupon_key = cache_coupon_prefix + to_string(payReq.user_id) + "_"+payReq.code;
            ret = rwrapper->setValue(user_cache_coupon_key,ThriftToJSON(cacheCoupon),CACHE_COUPON_EX);
            if(0 != ret){
                LOG_ERROR<<"error occured when redis execute"<< endl;
            }
        }

        payRsp.error = Error::OK;
        payRsp.errmsg = "success";

        LOG_WARN<<"payCoupon response is: PayRsp '"<<ThriftToJSON(payRsp)<<"'"<<endl;
        return;
    }

    //用户的优惠券列表
    void getUserCoupons(UserCouponsRsp& userCouponsRsp, const UserCouponsReq& userCouponsReq){
        LOG_WARN<<"getUserCoupons request is: UserCouponsReq '"<<ThriftToJSON(userCouponsReq)<<"'"<<endl;
        PerfWatch perfWatch("getUserCoupons");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
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
            LOG_ERROR<<"Redis connection disconnected"<< endl;
            io.fault = Error::REDIS_DISCONNECTED;
            io.why = "Redis connection disconnected";
            throw io;
        }

        //校验参数是否都已经填写
        if(userCouponsReq.user_id <= 0){
            LOG_ERROR<<"invalid params:"<<" user_id:"<<userCouponsReq.user_id<< endl;
            io.fault = Error::INVALID_PARAMS;
            io.why = "invalid params";
            throw io;
        }

        //页码，每页数量，状态
        int use_status = userCouponsReq.use_status;
        if(use_status < 0 || use_status > 3){
            use_status = 0;
        }

        //先查询用户-券表缓存
        CacheCodes cacheCodes;
        map<string,CacheCoupon> cacheCouponMap;

        const string cache_codes_key = cache_codes_prefix + to_string(userCouponsReq.user_id);
        string json_cache_codes = "";
        int ret = rwrapper->getValue(cache_codes_key,json_cache_codes);
        if(0 == ret){
            JSONToThrift(json_cache_codes,&cacheCodes);
            map<string, string> code_cacheCoupon_map;

            vector<string> v;
            for(auto x:cacheCodes.codes){
                string cache_coupon_key = cache_coupon_prefix + to_string(userCouponsReq.user_id) + "_" + x;
                v.push_back(cache_coupon_key);
            }

            ret = rwrapper->getBatchString(v, code_cacheCoupon_map);
            if(0 == ret){
                for(auto x:code_cacheCoupon_map){
                    CacheCoupon cacheCoupon;
                    JSONToThrift(x.second,&cacheCoupon);
                    if(-1 == use_status){
                        cacheCouponMap[cacheCoupon.code] = cacheCoupon;
                    }else if(3 == use_status){
                        if(3 == cacheCoupon.use_status || 0 == cacheCoupon.use_status){
                            cacheCouponMap[cacheCoupon.code] = cacheCoupon;
                        }
                    }else if(1 == use_status || 2 == use_status){
                        if(1 == cacheCoupon.use_status || 2 == cacheCoupon.use_status){
                            cacheCouponMap[cacheCoupon.code] = cacheCoupon;
                        }
                    }else{
                        if(use_status == cacheCoupon.use_status){
                            cacheCouponMap[cacheCoupon.code] = cacheCoupon;
                        }
                    }
                }
            }
        }

        //本次请求的数据
        int total = 0;
        CouponHelper couponHelper(mwrapper);
        ret = couponHelper.userCouponsCount(userCouponsReq.user_id,total);
        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        //查询用户-券表
        vector<UserCoupon> userCoupons;
        MultipleCondition cond;
        cond.andCondList.push_back(string("user_id = ") + to_string(userCouponsReq.user_id));
        if(3 == use_status){
            cond.andCondList.push_back("use_status in (0,3)");
        }else if(1 == use_status || 2 == use_status){
            cond.andCondList.push_back("use_status in (1,2)");
        }else{
            cond.andCondList.push_back(string("use_status = ") + to_string(use_status));
        }

        ret = couponHelper.queryUserCoupons(cond,"limit 0,1000",userCoupons);
        if(-9999 == ret){
            LOG_WARN<<"no data you expected in UserCoupon via your condition"<< endl;
            userCouponsRsp.error = Error::NO_DATA_YOU_EXPECTED;
            userCouponsRsp.errmsg = "no data you expected in UserCoupon via your condition";
            return;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        //批量查询券组
        set<int> coupon_group_ids;
        for(auto userCoupon: userCoupons){
            coupon_group_ids.insert(userCoupon.coupon_group_id);
        }

        map<int,CouponGroup> couponGroups;
        ret = couponHelper.queryCouponGroups(coupon_group_ids,couponGroups);
        if(-9999 == ret){
            LOG_ERROR<<"no data you expected in CouponGroup via your condition"<< endl;
            userCouponsRsp.error = Error::NO_DATA_YOU_EXPECTED;
            userCouponsRsp.errmsg = "no data you expected in CouponGroup via your condition";
            return;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        //批量查询券
        map<union_id_type,Coupon> coupons;
        //不在缓存中的券
        set<union_id_type> not_in_cache_set;
        //用户的所有券union_id
        set<union_id_type> all_set;

        //返回的券列表
        vector<CacheCoupon> couponList;
        if(userCoupons.size() > 0){
            for(int row = 0;row < (int)userCoupons.size();row++){
                all_set.insert(union_id_type(userCoupons[row].coupon_group_id,userCoupons[row].coupon_id));

                //查询本券在缓存中有
                auto iter = cacheCouponMap.find(userCoupons[row].code);
                if(iter != cacheCouponMap.end()){
                    //如果缓存券的版本不等于券组的版本
                    if(iter->second.version != couponGroups[iter->second.coupon_group_id].version){
                        cacheCouponMap.erase(iter->second.code);
                    }
                }

                auto it = cacheCouponMap.find(userCoupons[row].code);
                if(it != cacheCouponMap.end()){
                    couponList.push_back(it->second);
                    continue;
                }else{
                    //不在缓存中的券
                    not_in_cache_set.insert(union_id_type(userCoupons[row].coupon_group_id,userCoupons[row].coupon_id));
                    LOG_INFO<<"not_in_cache_set.insert: "<<userCoupons[row].coupon_group_id<<" "<<userCoupons[row].coupon_id<<endl;
                }
            }

            ret = couponHelper.queryCouponsByUnionAll(all_set,coupons);
            if(-9999 == ret){
                LOG_ERROR<<"no data you expected in coupons via your condition"<< endl;
                userCouponsRsp.error = Error::NO_DATA_YOU_EXPECTED;
                userCouponsRsp.errmsg = "no data you expected in coupons via your condition";
                return;
            }

            if(0 != ret){
                LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute"<< endl;
                LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
                io.fault = Error::MYSQL_EXECUTE_ERROR;
                io.why = couponHelper.getError();
                throw io;
            }
        }

        if(not_in_cache_set.size() > 0){
            for(int row = 0;row < (int)userCoupons.size();row++){
                auto it = cacheCouponMap.find(userCoupons[row].code);
                if(it != cacheCouponMap.end()){
                    continue;
                }

                //否则重新放入缓存
                CouponGroup couponGroup;
                if(couponGroups.find(userCoupons[row].coupon_group_id) != couponGroups.end()){
                    couponGroup = couponGroups[userCoupons[row].coupon_group_id];
                }else{
                    continue;
                }

                Coupon coupon;
                if(coupons.find(union_id_type(userCoupons[row].coupon_group_id,userCoupons[row].coupon_id))
                    != coupons.end()){
                    coupon = coupons[union_id_type(userCoupons[row].coupon_group_id,userCoupons[row].coupon_id)];
                }else{
                    continue;
                }

                //构建券信息
                CacheCoupon cacheCoupon;
                cacheCoupon.coupon_group_id = userCoupons[row].coupon_group_id;
                cacheCoupon.coupon_id = userCoupons[row].coupon_id;
                cacheCoupon.code = userCoupons[row].code;
                cacheCoupon.client_id = userCoupons[row].client_id;
                cacheCoupon.user_id = userCouponsReq.user_id;
                cacheCoupon.create_time = userCoupons[row].create_time;
                cacheCoupon.update_time = userCoupons[row].update_time;
                cacheCoupon.use_status = userCoupons[row].use_status;
                cacheCoupon.drawn_time = coupon.drawn_time;
                cacheCoupon.frozen_time = coupon.frozen_time;
                cacheCoupon.payed_time = coupon.payed_time;
                cacheCoupon.order_create_time = coupon.order_create_time;
                cacheCoupon.order_id = coupon.order_id;
                if(!couponGroup.is_duration_type){
                    cacheCoupon.start_use_time = couponGroup.start_use_time;
                    cacheCoupon.end_use_time = couponGroup.end_use_time;
                }else{
                    cacheCoupon.start_use_time = cacheCoupon.drawn_time;
                    cacheCoupon.end_use_time = cacheCoupon.drawn_time + couponGroup.duration_value * 24 * 3600;
                }

                cacheCoupon.name = couponGroup.name ;                   //名称
                cacheCoupon.title = couponGroup.title;                  //标题
                cacheCoupon.comment = couponGroup.comment;              //描述
                cacheCoupon.ware_label_id = couponGroup.ware_label_id;  //商品标签ID
                cacheCoupon.favor_type = couponGroup.favor_type ;       //优惠类型
                cacheCoupon.scope_type = couponGroup.scope_type ;       //使用范围
                cacheCoupon.sub_type = couponGroup.sub_type ;           //子类型
                cacheCoupon.scene_type = couponGroup.scene_type ;       //场景类型
                cacheCoupon.full = couponGroup.full;                    //满
                cacheCoupon.favor = couponGroup.favor;                  //减
                cacheCoupon.rate = couponGroup.rate ;                   //折扣率
                cacheCoupon.argot = couponGroup.argot;                  //暗语
                cacheCoupon.seller_id = couponGroup.seller_id;          //店铺
                cacheCoupon.url = couponGroup.url   ;                   //连接
                cacheCoupon.img = couponGroup.img   ;
                cacheCoupon.slug = couponGroup.slug   ;
                cacheCoupon.img_width = couponGroup.img_width   ;
                cacheCoupon.img_height = couponGroup.img_height   ;
                cacheCoupon.button_text = couponGroup.button_text   ;
                cacheCoupon.button_jump = couponGroup.button_jump   ;
                cacheCoupon.jump_label = couponGroup.jump_label   ;
                cacheCoupon.jump_data = couponGroup.jump_data   ;
                cacheCoupon.argot_jump_label = couponGroup.argot_jump_label   ;
                cacheCoupon.argot_jump_data = couponGroup.argot_jump_data   ;
                cacheCoupon.version = couponGroup.version   ;
                cacheCoupon.description = couponGroup.description   ;

                couponList.push_back(cacheCoupon);
                cacheCouponMap[userCoupons[row].code]=cacheCoupon;

                auto iter = find(cacheCodes.codes.begin(),cacheCodes.codes.end(),userCoupons[row].code);
                if(cacheCodes.codes.end() == iter){
                    if(cacheCodes.codes.size() >= cache_codes_capacity){
                        vector<string>::iterator k = cacheCodes.codes.begin();
                        cacheCodes.codes.erase(k); // 删除第一个元素
                    }

                    cacheCodes.codes.push_back(userCoupons[row].code);

                    ret = rwrapper->setValue(cache_codes_key,ThriftToJSON(cacheCodes));
                    if(0 != ret){
                        LOG_ERROR<<"error occured when redis execute setValue"<< endl;
                    }
                }

                string user_cache_coupon_key = cache_coupon_prefix + to_string(cacheCoupon.user_id) + "_"+cacheCoupon.code;
                ret = rwrapper->setValue(user_cache_coupon_key,ThriftToJSON(cacheCoupon),CACHE_COUPON_EX);
            }
        }

        for(auto &x : couponList){
            if(x.end_use_time < time(NULL) && x.use_status == USE_STATUS::DRAWN){
                x.use_status  = USE_STATUS::EXPIRED;
            }
        }

        if(1 == use_status || 2 == use_status){
            for(auto iter = couponList.begin();iter != couponList.end();){
                if(iter->use_status != 1 && iter->use_status != 2){
                    iter = couponList.erase(iter);
                }else{
                    iter++;
                }
            }
		} else {
		    for(auto iter = couponList.begin();iter != couponList.end();){
                if(use_status != iter->use_status){
                    iter = couponList.erase(iter);
                }else{
                    iter++;
                }
            }
		}

        /*
        可用的券排序:
            最快过期
            面额最大
            最先开始
        已使用/已过期优惠券排序:
            已使用/已过期优惠券按到期时间倒序排列；
        */
        if(0 == use_status){
            vector<CacheCoupon> left;
            for(auto x : couponList){
                if(time(NULL) >= x.start_use_time ){
                    left.push_back(x);
                }
            }

            std::sort(left.begin(),left.end(),[&](const CacheCoupon a,const CacheCoupon b){
                if(a.end_use_time == b.end_use_time){
                    return a.favor > b.favor;//面额最大
                }else{
                    return a.end_use_time < b.end_use_time;//最快过期
                }
            });

            //即将开始
            vector<CacheCoupon> earliest;
            for(auto x : couponList){
                if(time(NULL) < x.start_use_time ){
                    earliest.push_back(x);
                }
            }

            std::sort(earliest.begin(),earliest.end(),[&](const CacheCoupon a,const CacheCoupon b){
                if(a.end_use_time == b.end_use_time){
                    return a.favor > b.favor;//面额最大
                }else{
                    return a.end_use_time < b.end_use_time;//最快过期
                }
            });

            for(auto x:earliest){
                left.push_back(x);
            }

            couponList.swap(left);
        }else{
            std::sort(couponList.begin(),couponList.end(),
                [&](const CacheCoupon a,const CacheCoupon b){
                    return a.end_use_time > b.end_use_time;
            });
        }

        int count = userCouponsReq.count;
        int max_page = couponList.size() / count;
        if(couponList.size() % count != 0){
            max_page += 1;
        }

        int page = 1;
        if(userCouponsReq.page >= 1 && userCouponsReq.page <= max_page){
            page = userCouponsReq.page;
        }

        int real_count = count;
        if(page * count > (int)couponList.size()){
            real_count = couponList.size() % count;
        }

        vector<CacheCoupon> _coupons;
        for(int i = (page-1)*count; i<(page-1)*count + real_count; i++){
        	_coupons.push_back(couponList.at(i));
        }

        userCouponsRsp.error = Error::OK;
        userCouponsRsp.errmsg = "success";
        userCouponsRsp.user_id = userCouponsReq.user_id;
        userCouponsRsp.page = page;
        userCouponsRsp.use_status = use_status;
        userCouponsRsp.total = couponList.size();
        userCouponsRsp.coupons = _coupons;

        LOG_WARN<<"getUserCoupons response is: UserCouponsRsp '"<<ThriftToJSON(userCouponsRsp)<<"'"<<endl;
    }

    //商品对应的优惠券活动
    void getWareActs(WareActsRsp& wareActsRsp, const WareActsReq& wareActsReq){
        LOG_WARN<<"getWareActs request is: WareActsReq '"<<ThriftToJSON(wareActsReq)<<"'"<<endl;
        PerfWatch perfWatch("getWareActs");
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
            LOG_ERROR<<"Redis connection disconnected"<< endl;
            io.fault = Error::REDIS_DISCONNECTED;
            io.why = "Redis connection disconnected";
            throw io;
        }

        //校验参数是否都已经填写
        if(wareActsReq.ware_ids.size() == 0){
            LOG_ERROR<<"invalid params:"<<" wareActsReq.ware_ids.size:"<<wareActsReq.ware_ids.size()<< endl;
            io.fault = Error::INVALID_PARAMS;
            io.why = "invalid params";
            throw io;
        }

        //ware_id --> CouponGroup
        map<int64_t,vector<CouponGroup>> wareCouponGroupMap;
        int ret = couponHelper.queryWareLabelIds(wareActsReq.ware_ids,wareCouponGroupMap);
        if(-9999 == ret){
            wareActsRsp.error = Error::OK;
            wareActsRsp.errmsg = "success";
            return;
        }

        if(ret < 0 ){
            LOG_ERROR<<"mysql exe error ret:"<<ret<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        wareActsRsp.error = Error::OK;
        wareActsRsp.errmsg = "success";
        wareActsRsp.wareActs = wareCouponGroupMap;

        LOG_WARN<<"getWareActs response is: WareActsRsp '"<<ThriftToJSON(wareActsRsp)<<"'"<<endl;

    }

    void getNotUsableCoupons(NotUsableCouponsRsp& notUsableCouponsRsp, const NotUsableCouponsReq& notUsableCouponsReq){
        LOG_WARN<<"getNotUsableCoupons request is: NotUsableCouponsReq '"<<ThriftToJSON(notUsableCouponsReq)<<"'"<<endl;
        PerfWatch perfWatch("getNotUsableCoupons");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected when getnotNotUsableCoupons"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected when getNotUsableCoupons";
            throw io;
        }

        /*
		查询用户所有领取态的券,除去本单可用的券
		未满足条件优惠券多情况并存提示顺序：时间未到→商品不可用→金额不满足；
		• 优惠券使用时间未到：优惠券未到生效时间；
		• 所选商品全部非优惠券适用范围内：商品不符合适用范围；
		• 所选商品部分非优惠券适用范围内：差XX元可用该券；
		• 所选商品全部在优惠券适用范围内，但不符合满减条件：差XX元可用该券。
		  XX元=满金额-符合优惠券适用范围内的商品总金额
		*/

        set<int64_t > orderWareIdSet;//订单中所有的wareID集合
		map<int64_t,set<int64_t >> sellerId_wareIdSet_map;//卖家ID 到 卖家的wareID集合 的映射表
		map<int64_t,double> wareId_sumPrice_map;//按wareID进行一次价格汇总（订单级）[因为卖家下可能有同一商品(ware)]

		set<int64_t > sellerIdSet;//订单中所有的卖家ID集合
		sellerIdSet.insert(0);//所有卖家id
		for(auto x : notUsableCouponsReq.sellers){//遍历每个卖家
			int64_t seller_id = x.first;//这个卖家的id
			SellerInfo sellerInfo = x.second;//卖家信息
			sellerIdSet.insert(seller_id);//所有卖家id

			map<int64_t,SkuInfo> skus = sellerInfo.skus;//该卖家的sku信息
			for(auto y:skus){//遍历该卖家的sku
				//int64_t sku_id = y.first;
				SkuInfo skuInfo = y.second;//该sku的信息
				int64_t ware_id = skuInfo.ware_id;

				wareId_sumPrice_map[ware_id] += skuInfo.sale_price * skuInfo.sku_count;//按商品ware汇总，该商品ware累加上这次的计算价格
				sellerId_wareIdSet_map[seller_id].insert(ware_id);//卖家id对应的商品ware集合 也加入 该wareID
				orderWareIdSet.insert(ware_id);//订单级商品ware集合中 加入 该wareID
			}
		}

		//查询用户所有领取态的券
		UserCouponsReq userCouponsReq;
		userCouponsReq.user_id = notUsableCouponsReq.user_id;
		userCouponsReq.page = 1;
		userCouponsReq.count = 1000;
		userCouponsReq.use_status = 0;

		UserCouponsRsp userCouponsRsp;
		getUserCoupons(userCouponsRsp, userCouponsReq);
		if(Error::OK != userCouponsRsp.error){
			notUsableCouponsRsp.error = Error::NO_DATA_YOU_EXPECTED;
			notUsableCouponsRsp.errmsg = "no data you expected";
			return;
		}

		//除去本单可用的券
		for(auto k = userCouponsRsp.coupons.begin();k != userCouponsRsp.coupons.end();){
			if(find_if(notUsableCouponsReq.usableCodes.begin(),notUsableCouponsReq.usableCodes.end(),
				[&](string code){return k->code == code;}) != notUsableCouponsReq.usableCodes.end()){
				k = userCouponsRsp.coupons.erase(k);
			}else{
				k++;
			}
		}

		//除去非sellers的其他店铺的店铺券
		for(auto k = userCouponsRsp.coupons.begin();k != userCouponsRsp.coupons.end();){
			if(sellerIdSet.find(k->seller_id) == sellerIdSet.end()){
				k = userCouponsRsp.coupons.erase(k);
			}else{
				k++;
			}
		}

		set<unsigned int > wareLabelIdSet;//所有有效的标签
		for(auto it= userCouponsRsp.coupons.begin();it!=userCouponsRsp.coupons.end(); it++){//遍历所有券表券
			wareLabelIdSet.insert(it->ware_label_id);
		}

		//每个有效标签下的名单
		map<unsigned int , set<int64_t>> wareLabelId_wareIdSet_map;//标签 --> 商品集合 映射表
		int ret = couponHelper.queryWareLableWares(wareLabelIdSet,wareLabelId_wareIdSet_map);
		if(0 != ret && -9999 != ret){
			LOG_WARN<<"ret="<<ret<<" error occured when mysql execute queryWareLableWares"<< endl;
			io.fault = Error::MYSQL_EXECUTE_ERROR;
			io.why = "error occured when mysql execute queryWareLableWares";
			throw io;
		}

		for(auto &cacheCoupon:userCouponsRsp.coupons){//遍历每个券，依次判断不可用的原因
			int ware_label_id = cacheCoupon.ware_label_id;
			double sum_price = 0.0;
			set<int64_t > filteredOrderWareIdSet;//参与计算的订单级商品ware
			set<int64_t > filteredSellerWareIdSet;//参与计算的卖家级商品ware

			if(0 == cacheCoupon.scope_type){//如果是平台券
				if(0 == cacheCoupon.sub_type){//如果是白名单
					set<int64_t> whitelist = wareLabelId_wareIdSet_map[ware_label_id];//对应的白名单
					unsigned valid_ware_count = 0;//参与计算的订单商品的个数
					for(auto ware_id : orderWareIdSet){//遍历订单级商品ware
						if(whitelist.find(ware_id) != whitelist.end()){//商品ware在白名单中
							filteredOrderWareIdSet.insert(ware_id);//得到参与计算的订单级商品ware
							valid_ware_count ++;
						}
					}
					if(0 == valid_ware_count ){
						cacheCoupon.type = NOT_USABLE_TYPE_LIST;
						continue;
					}
				}else if(1 == cacheCoupon.sub_type){//如果是黑名单
					set<int64_t> blacklist = wareLabelId_wareIdSet_map[ware_label_id];//对应的黑名单
					unsigned valid_ware_count = 0;//参与计算的订单商品ware的个数
					for(auto ware_id : orderWareIdSet){//遍历订单级商品ware
						if(blacklist.find(ware_id) == blacklist.end()){//如果不在黑名单中
							filteredOrderWareIdSet.insert(ware_id);//得到参与计算的订单级商品ware
							valid_ware_count ++;
						}
					}
					if(0 == valid_ware_count ){
						cacheCoupon.type = NOT_USABLE_TYPE_LIST;
						continue;
					}
				}else if(2 == cacheCoupon.sub_type){//如果是全部商品ware
					for(auto ware_id : orderWareIdSet){//遍历订单级商品ware
						filteredOrderWareIdSet.insert(ware_id);//得到参与计算的订单级商品ware
					}
				}

				for(auto ware_id : filteredOrderWareIdSet){//遍历参与计算的商品ware
					sum_price += wareId_sumPrice_map[ware_id];//把订单中涉及的这些商品ware的价格累加
				}

				if(cacheCoupon.favor_type ==0 && sum_price < cacheCoupon.full){
					cacheCoupon.type = NOT_USABLE_TYPE_MONEY;
					cacheCoupon.value = cacheCoupon.full - sum_price;
				}
			}else if(1 == cacheCoupon.scope_type){//如果是店铺券
				//如果订单卖家 中  没有这个卖家   跳过
				if(sellerId_wareIdSet_map.find(cacheCoupon.seller_id) == sellerId_wareIdSet_map.end()){
					cacheCoupon.type = NOT_USABLE_TYPE_INVALID;
					continue;
				}
				set<int64_t > sellerWareIdSet = sellerId_wareIdSet_map[cacheCoupon.seller_id];//该卖家下的商品

				if(0 == cacheCoupon.sub_type){//如果是白名单
					set<int64_t> whitelist = wareLabelId_wareIdSet_map[ware_label_id];//对应的白名单
					unsigned valid_ware_count = 0;//参与计算的卖家级商品ware的个数
					for(auto ware_id : sellerWareIdSet){//遍历该卖家商品ware
						if(whitelist.find(ware_id) != whitelist.end()){//如果在白名单中
							filteredSellerWareIdSet.insert(ware_id);//得到参与计算的卖家级商品ware
							valid_ware_count++;
						}
					}
					if(0 == valid_ware_count ){
						cacheCoupon.type = NOT_USABLE_TYPE_LIST;
						continue;
					}
				}else if(1 == cacheCoupon.sub_type){//如果是黑名单
					set<int64_t> blacklist = wareLabelId_wareIdSet_map[ware_label_id];//得到对应的黑名单
					unsigned valid_ware_count = 0;//参与计算的卖家级商品ware的个数
					for(auto ware_id : sellerWareIdSet){//遍历卖家商品ware
						if(blacklist.find(ware_id) == blacklist.end()){//如果该商品ware不在黑名单中
							filteredSellerWareIdSet.insert(ware_id);//得到参与计算的卖家级商品ware
							valid_ware_count++;
						}
					}
					if(0 == valid_ware_count ){
						cacheCoupon.type = NOT_USABLE_TYPE_LIST;
						continue;
					}
				}else if(2 == cacheCoupon.sub_type){//如果是全部商品ware
					for(auto ware_id : sellerWareIdSet){//遍历卖家商品ware
						filteredSellerWareIdSet.insert(ware_id);//得到参与计算的卖家级商品ware
					}
				}

				for(auto x : notUsableCouponsReq.sellers){//遍历每个卖家
					int seller_id = x.first;
					SellerInfo sellerInfo = x.second;
					if(seller_id == cacheCoupon.seller_id){//找到该券对应的卖家
						map<int64_t,SkuInfo> skus = sellerInfo.skus;
						for(auto y:skus){//遍历该卖家的商品sku
							//int64_t sku_id = y.first;
							SkuInfo skuInfo = y.second;
							int64_t ware_id = skuInfo.ware_id;

							if(filteredSellerWareIdSet.find(ware_id) != filteredSellerWareIdSet.end()){//如果该商品ware参与计算
								sum_price += skuInfo.sale_price * skuInfo.sku_count;//计算出卖家下 该店铺券对应的所有商品sku 的总售价
							}
						}
					}
				}

				if(cacheCoupon.favor_type ==0 && sum_price < cacheCoupon.full){
					cacheCoupon.type = NOT_USABLE_TYPE_MONEY;
					cacheCoupon.value = cacheCoupon.full - sum_price;
				}
			}
		}

        //时间未到
		for(auto &k : userCouponsRsp.coupons){
			if(k.use_status == USE_STATUS::DRAWN && time(NULL) < k.start_use_time){
				k.type = NOT_USABLE_TYPE_TIME;
				k.value = k.start_use_time - time(NULL);
			}
		}

		//除去不能参加排序的券
		for(auto k = userCouponsRsp.coupons.begin();k != userCouponsRsp.coupons.end();){
			if(k->type == NOT_USABLE_TYPE_INVALID){
				k = userCouponsRsp.coupons.erase(k);
			}else{
				k++;
			}
		}

        //排序
        vector<CacheCoupon> left;
        for(auto x : userCouponsRsp.coupons){
            if(time(NULL) >= x.start_use_time ){
                left.push_back(x);
            }
        }

        std::sort(left.begin(),left.end(),[&](const CacheCoupon a,const CacheCoupon b){
            if(a.end_use_time == b.end_use_time){
                return a.favor > b.favor;//面额最大
            }else{
                return a.end_use_time < b.end_use_time;//最快过期
            }
        });

        //即将开始
        vector<CacheCoupon> earliest;
        for(auto x : userCouponsRsp.coupons){
            if(time(NULL) < x.start_use_time ){
                earliest.push_back(x);
            }
        }

        std::sort(earliest.begin(),earliest.end(),[&](const CacheCoupon a,const CacheCoupon b){
            if(a.end_use_time == b.end_use_time){
                return a.favor > b.favor;//面额最大
            }else{
                return a.end_use_time < b.end_use_time;//最快过期
            }
        });

        for(auto x:earliest){
            left.push_back(x);
        }

        userCouponsRsp.coupons.swap(left);

        int count = notUsableCouponsReq.count;
        int max_page = userCouponsRsp.coupons.size() / count;
        if(userCouponsRsp.coupons.size() % count != 0){
            max_page += 1;
        }

        int page = 1;
        if(notUsableCouponsReq.page >= 1 && notUsableCouponsReq.page <= max_page){
            page = notUsableCouponsReq.page;
        }

        int real_count = count;
        if(page * count > (int)userCouponsRsp.coupons.size()){
            real_count = userCouponsRsp.coupons.size() % count;
        }

        vector<CacheCoupon> notUsableCoupons;
        for(int i = (page-1)*count; i<(page-1)*count + real_count; i++){
        	notUsableCoupons.push_back(userCouponsRsp.coupons.at(i));
        }

        notUsableCouponsRsp.error = Error::OK;
        notUsableCouponsRsp.errmsg = "success";
        notUsableCouponsRsp.user_id = notUsableCouponsReq.user_id;
        notUsableCouponsRsp.sellers = notUsableCouponsReq.sellers;
        notUsableCouponsRsp.usableCodes = notUsableCouponsReq.usableCodes;
        notUsableCouponsRsp.notUsableCoupons = notUsableCoupons;
        notUsableCouponsRsp.page = page;
        notUsableCouponsRsp.count = notUsableCoupons.size();
        notUsableCouponsRsp.total = userCouponsRsp.coupons.size();

        LOG_WARN<<"getNotUsableCoupons response is: NotUsableCouponsRsp '"<<ThriftToJSON(notUsableCouponsRsp)<<"'"<<endl;
    }

    void getUsableCoupons(UsableCouponsRsp& usableCouponsRsp, const UsableCouponsReq& usableCouponsReq){
        LOG_WARN<<"getUsableCoupons request is: UsableCouponsReq '"<<ThriftToJSON(usableCouponsReq)<<"'"<<endl;
        PerfWatch perfWatch("getUsableCoupons");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected when getUsableCoupons"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected when getUsableCoupons";
            throw io;
        }

        //过程：
        //1.计算商品售价总金额                          sumPrice
        //  输出表： I.  订单中所有商品id集合             orderWareIdSet
        //          II. 卖家id --> 卖家商品id集合       sellerId_wareIdSet_map

        //2.查询用户的有效的券，券组，过滤掉非领取态的，审核不通过的，full>sum_price（第一次过滤）,过期的
        //  (平台白) 券1 --> 券组1  --> 标签1
        //  (平台黑) 券2 --> 券组2  --> 标签2
        //  (平台通) 券3 --> 券组3  --> 标签3
        //  (平台通) 券4 --> 券组4  --> 标签3

        //  (店铺白) 券5 --> 券组5  --> 标签4     卖家1 <--> 券5
        //  (店铺黑) 券6 --> 券组6  --> 标签5     卖家2 <--> 券6
        //  (店铺黑) 券7 --> 券组6  --> 标签5     卖家2 <--> 券7
        //  (店铺通) 券8 --> 券组7  --> 标签6

        //  输出表:  <组id 券id >  --> 标签id           unionId_wareLabelId_map
        //          <组id 券id >  --> 卖家id            unionId_sellerId_map
        //               卖家id   --> 券集合            sellerId_unionIdSet_map
        //          <组id 券id >  --> 券                unionId_cacheCoupon_map

        //3.针对每个标签(2中得到的标签去重)： 标签1 标签2 标签3 标签4 标签5 标签6
        //  查询标签-商品表
        //  输出表:  标签id  --> 名单集合               wareLabelId_wareIdSet_map

        //4.遍历每张cacheCoupon券：
        //      平台券：
        //          白： a.<组id 券id > ==> 标签id ==> 名单集合
        //              b. 订单中的所有商品
        //              c. 由a,b得到参与计算的商品；总额大于券的full则返回这个券
        //          黑： a.<组id 券id > ==> 标签id ==> 名单集合
        //              b. 订单中的所有商品
        //              c. 由a,b得到参与计算的商品；总额大于券的full则返回这个券
        //          通： 订单中的所有商品，总额大于券的full则返回这个券

        //      店铺券：->
        //          白：a.<组id 券id > ==> 标签id ==> 名单集合
        //              b.卖家id ==> 卖家商品集合
        //              c.由a,b得到参与计算的商品；总额大于券的full则返回这个 店铺券
        //          黑：a.<组id 券id > ==> 标签id ==> 名单集合
        //              b.卖家id ==> 卖家商品集合
        //              c.由a,b得到参与计算的商品；总额大于券的full则返回这个 店铺券
        //          通： a.卖家id ==> 卖家商品集合
        //              b.计算a的所有商品，总额大于券的full则返回这个 店铺券


        //计算sku总价
        PriceInfo orderPrice; //订单级价格汇总，分摊之前
        map<int64_t,PriceInfo> sellerId_priceInfo_map;//卖家级价格信息
        map<int64_t,map<int64_t,PriceInfo>> sellerId_skuId_priceInfo_map;//sku级价格信息

        map<int64_t,int64_t> skuID_wareID_map;//skuID所属的wareID

        double order_market_price = 0.0;//订单中所有sku的总市场价
        double order_sale_price = 0.0;//订单中所有sku的总售价

        set<int64_t > orderWareIdSet;//订单中所有的wareID集合
        map<int64_t,set<int64_t >> sellerId_wareIdSet_map;//卖家ID 到 卖家的wareID集合 的映射表

        map<int64_t,double> wareId_sumPrice_map;//按wareID进行一次价格汇总（订单级）[因为卖家下可能有同一商品(ware)]

        set<int64_t > sellerIdSet;//订单中所有的卖家ID集合
        sellerIdSet.insert(0);
        for(auto x : usableCouponsReq.sellers){//遍历每个卖家
            int64_t seller_id = x.first;//这个卖家的id
            SellerInfo sellerInfo = x.second;//卖家信息

            sellerIdSet.insert(seller_id);
            PriceInfo sellerPrice;//该卖家价格汇总，分摊之前

            map<int64_t,SkuInfo> skus = sellerInfo.skus;//该卖家的sku信息
            for(auto y:skus){//遍历该卖家的sku
                int64_t sku_id = y.first;
                SkuInfo skuInfo = y.second;//该sku的信息
                int64_t ware_id = skuInfo.ware_id;

                skuID_wareID_map[sku_id] = ware_id;//skuID所属的wareID

                wareId_sumPrice_map[ware_id] += skuInfo.sale_price * skuInfo.sku_count;//按商品ware汇总，该商品ware累加上这次的计算价格

                order_market_price  += skuInfo.market_price * skuInfo.sku_count;//累加这个sku的市场价
                order_sale_price += skuInfo.sale_price * skuInfo.sku_count;//累加这个sku的售价

                orderWareIdSet.insert(ware_id);//订单级商品ware集合中 加入 该wareID
                sellerId_wareIdSet_map[seller_id].insert(ware_id);//卖家id对应的商品ware集合 也加入 该wareID

                sellerPrice.market_price += skuInfo.market_price * skuInfo.sku_count;//该卖家累加这个sku的市场价
                sellerPrice.sale_price += skuInfo.sale_price * skuInfo.sku_count;//该卖家累加这个sku的市场价
                sellerPrice.favor_price = 0.0;//该卖家累加这个商品的优惠价
                sellerPrice.pay_price = sellerPrice.sale_price;//该卖家累加这个商品的支付价

                PriceInfo skuPrice;//该sku价格信息汇总
                skuPrice.market_price = skuInfo.market_price * skuInfo.sku_count;
                skuPrice.sale_price = skuInfo.sale_price * skuInfo.sku_count;
                skuPrice.favor_price = 0.0;
                skuPrice.pay_price = skuPrice.sale_price;

                sellerId_skuId_priceInfo_map[seller_id][sku_id] = skuPrice;//卖家下商品信息汇总
            }

            sellerId_priceInfo_map[seller_id] = sellerPrice;//该卖家级价格汇总
        }

        orderPrice.market_price = order_market_price;
        orderPrice.sale_price = order_sale_price;
        orderPrice.favor_price = 0.0;
        orderPrice.pay_price = order_sale_price;

        //用户-券表 过滤了非领取态的 如果是店铺券,这里顺便过滤掉非sellers的其他店铺的店铺券
        string in = "(";
        for(auto it=sellerIdSet.begin();it != sellerIdSet.end();it++){
            in += to_string(*it);
            auto iter = it;
            iter++;
            if(iter != sellerIdSet.end()){
                in += ",";
            }
        }
        in += ")";

        MultipleCondition cond;
        cond.andCondList.push_back(string(" user_id = " )+ to_string(usableCouponsReq.user_id));
        cond.andCondList.push_back(" use_status = 0");
        cond.andCondList.push_back(string(" UNIX_TIMESTAMP(create_time) > ") + to_string(time(NULL) - 365 * 24 * 3600));
        cond.andCondList.push_back(string(" seller_id in ") + in);

        vector<UserCoupon> userCoupons;//用户的所有领取态的券
        //int ret = couponHelper.queryUserCoupons(usableCouponsReq.user_id,userCoupons,0,999999999,0);
        int ret = couponHelper.queryUserCoupons(cond,userCoupons);
        if(-9999 == ret){
            LOG_WARN<<"no data you expected in UserCoupon via your condition"<< endl;
            usableCouponsRsp.error = Error::NO_DATA_YOU_EXPECTED;
            usableCouponsRsp.errmsg = "no data you expected in UserCoupon via your condition";
            return;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute queryUserCoupons"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        vector<string> user_codes;//用户的处于领取态的券码
        set<int> couponGroupIdSet;//用户领取态的券对应的券组
        map<union_id_type,UserCoupon> uniondId_userCoupon_map;//券的联合ID 对应的 UserCoupon 映射表
        for(auto userCoupon: userCoupons){
            user_codes.push_back(userCoupon.code);//用户的处于领取态的券码
            couponGroupIdSet.insert(userCoupon.coupon_group_id);//用户领取态的券对应的券组

            auto union_id = union_id_type(userCoupon.coupon_group_id,userCoupon.coupon_id);
            uniondId_userCoupon_map[union_id] = userCoupon;
        }

        //批量查询券组 过滤了审核不通过的，
        map<int,CouponGroup> couponGroups;//有效的券对应的所有券组 coupon_group_id --> CouponGroup
        ret = couponHelper.queryCouponGroups(couponGroupIdSet,couponGroups);
        if(-9999 == ret){
            LOG_ERROR<<"no data you expected in CouponGroup via your condition"<< endl;
            usableCouponsRsp.error = Error::NO_DATA_YOU_EXPECTED;
            usableCouponsRsp.errmsg = "no data you expected in CouponGroup via your condition";
            return;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute when queryCouponGroups"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        //过滤出有效的UnionId
        set<union_id_type> validUnionIdSet;//有效的券ID
        for(auto userCoupon: userCoupons){
            //过滤掉full>order_sale_price
            if(couponGroups.find(userCoupon.coupon_group_id) == couponGroups.end()){//如果有效券组中没有该项
                continue;
            }

            if(couponGroups[userCoupon.coupon_group_id].favor_type == 0 &&//如果时满减类型
                //如果full 大于order_sale_price，也就是没有满（这里以价格第一次过滤）
                couponGroups[userCoupon.coupon_group_id].full > order_sale_price){
                continue;
            }

            if(couponGroups[userCoupon.coupon_group_id].favor_type == 0 &&//如果时是满减类型
                //如果出现满10元送20元，则本满减券无效（这里以价格第一次过滤）
                couponGroups[userCoupon.coupon_group_id].full < couponGroups[userCoupon.coupon_group_id].favor){
                continue;
            }

            //反解算券码，
            unsigned int __coupon_group_id, __coupon_id;
            int ret = CouponCode::Decode(userCoupon.code,__coupon_group_id,__coupon_id);
            if(CouponCode::Errno::OK != ret || (int)__coupon_group_id != userCoupon.coupon_group_id || (int)__coupon_id != userCoupon.coupon_id){
                LOG_ERROR<<"ret="<<ret<<" oh my god , fuck it, code decode fatal error~~~"<< endl;
                LOG_ERROR<<"userCoupon.code : "<<userCoupon.code<< endl;
                LOG_ERROR<<"__coupon_group_id: "<<__coupon_group_id<<" __coupon_id: "<<__coupon_id<< endl;
                LOG_ERROR<<"userCoupon.coupon_group_id: "<<userCoupon.coupon_group_id<<" userCoupon.coupon_id: "<<userCoupon.coupon_id<< endl;
                continue;
            }

            auto union_id = union_id_type(userCoupon.coupon_group_id,userCoupon.coupon_id);
            validUnionIdSet.insert(union_id);//有效的券联合ID
        }

        map<union_id_type,Coupon> unionId_coupon_map;
        //查询有效的券,如果涉及多个券表，这一步可能比较耗时，原因在于多次走网络，尽管同一个券表中一次查完
        //ret = couponHelper.queryCoupons(validUnionIdSet,unionId_coupon_map);
        //使用union all优化
        ret = couponHelper.queryCouponsByUnionAll(validUnionIdSet,unionId_coupon_map);
        if(0 != ret){
            LOG_WARN<<"ret="<<ret<<" error occured when mysql execute queryCouponsByUnionAll"<< endl;
            LOG_WARN<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        //构建CacheCoupon
        set<unsigned int > wareLabelIdSet;//所有有效的标签
        map<union_id_type,unsigned int> unionId_wareLabelId_map;//联合券 到 商品标签 映射表
        map<union_id_type,int64_t> unionId_sellerId_map;//联合券 到 卖家ID 映射表
        map<union_id_type,CacheCoupon> unionId_cacheCoupon_map;//联合券 到 cacheCoupon 映射表
        map<int64_t,set<union_id_type>> sellerId_unionIdSet_map;//卖家 到 联合券集合 的映射表

        for(auto it= unionId_coupon_map.begin();it!=unionId_coupon_map.end(); it++){//遍历所有券表券
            unsigned int coupon_group_id = it->first.first;//券组ID
            Coupon coupon = it->second;//券表券

            CouponGroup couponGroup;//券组
            if(couponGroups.find(coupon_group_id) != couponGroups.end()){
                couponGroup = couponGroups[coupon_group_id];
            }else{
                continue;
            }

            CacheCoupon cacheCoupon;//缓存券
            cacheCoupon.coupon_group_id = coupon.coupon_group_id;
            cacheCoupon.coupon_id = coupon.coupon_id;

            auto union_id = union_id_type(coupon.coupon_group_id,coupon.coupon_id);

            cacheCoupon.code = coupon.code;
            cacheCoupon.user_id = coupon.user_id;
            cacheCoupon.create_time = coupon.create_time;
            cacheCoupon.update_time = uniondId_userCoupon_map[union_id].update_time;
            cacheCoupon.use_status = uniondId_userCoupon_map[union_id].use_status;
            cacheCoupon.client_id = uniondId_userCoupon_map[union_id].client_id;
            cacheCoupon.drawn_time = coupon.drawn_time;
            cacheCoupon.frozen_time = coupon.frozen_time;
            cacheCoupon.payed_time = coupon.payed_time;
            cacheCoupon.order_create_time = coupon.order_create_time;
            cacheCoupon.order_id = coupon.order_id;
            if(!couponGroup.is_duration_type){
                cacheCoupon.start_use_time = couponGroup.start_use_time;
                cacheCoupon.end_use_time = couponGroup.end_use_time;
            }else{
                cacheCoupon.start_use_time = cacheCoupon.drawn_time;
                cacheCoupon.end_use_time = cacheCoupon.drawn_time + couponGroup.duration_value * 24 * 3600;
            }

            if(time(NULL) > cacheCoupon.end_use_time   && cacheCoupon.use_status == USE_STATUS::DRAWN){
                cacheCoupon.use_status  = USE_STATUS::EXPIRED;
            }

            cacheCoupon.name = couponGroup.name ;                   //名称
            cacheCoupon.title = couponGroup.title;                  //标题
            cacheCoupon.comment = couponGroup.comment;              //描述
            cacheCoupon.ware_label_id = couponGroup.ware_label_id;  //商品标签ID
            cacheCoupon.favor_type = couponGroup.favor_type ;       //优惠类型
            cacheCoupon.scope_type = couponGroup.scope_type ;       //使用范围
            cacheCoupon.sub_type = couponGroup.sub_type ;           //子类型
            cacheCoupon.scene_type = couponGroup.scene_type ;       //场景类型
            cacheCoupon.full = Round(couponGroup.full);             //满
            cacheCoupon.favor = Round(couponGroup.favor);           //减
            cacheCoupon.rate = Round(couponGroup.rate);             //折扣率
            cacheCoupon.argot = couponGroup.argot;                  //暗语
            cacheCoupon.seller_id = couponGroup.seller_id;          //店铺
            cacheCoupon.url = couponGroup.url   ;                   //连接
            cacheCoupon.img = couponGroup.img   ;                   //连接
            cacheCoupon.slug = couponGroup.slug   ;
            cacheCoupon.img_width = couponGroup.img_width   ;
            cacheCoupon.img_height = couponGroup.img_height   ;
            cacheCoupon.button_text = couponGroup.button_text   ;
            cacheCoupon.button_jump = couponGroup.button_jump   ;
            cacheCoupon.jump_label = couponGroup.jump_label   ;
            cacheCoupon.jump_data = couponGroup.jump_data   ;
            cacheCoupon.argot_jump_label = couponGroup.argot_jump_label   ;
            cacheCoupon.argot_jump_data = couponGroup.argot_jump_data   ;
            cacheCoupon.version = couponGroup.version   ;
            cacheCoupon.description = couponGroup.description   ;

            //如果真的过期了 或者 时间没到
            if(time(NULL) > cacheCoupon.end_use_time ||time(NULL) < cacheCoupon.start_use_time){
                continue;
            }

            unionId_wareLabelId_map[union_id] = couponGroup.ware_label_id;

            if(couponGroup.scope_type == 1 && couponGroup.seller_id > 0){//如果是店铺券，且卖家ID有效
                unionId_sellerId_map[union_id] = couponGroup.seller_id;
                sellerId_unionIdSet_map[couponGroup.seller_id].insert(union_id);
            }

            unionId_cacheCoupon_map[union_id] = cacheCoupon;

            wareLabelIdSet.insert(cacheCoupon.ware_label_id);
        }

        //如果没有得到任何有效的标签,当然券也就不可用了
        if(wareLabelIdSet.empty()){
            LOG_WARN<<"no data you expected in wareLabelIdSet, so no usable coupon"<< endl;
            usableCouponsRsp.error = Error::NO_USABLE_COUPON;
            usableCouponsRsp.errmsg = "no usable coupon";
            return;
        }

        //每个有效标签下的名单
        map<unsigned int , set<int64_t>> wareLabelId_wareIdSet_map;//标签 --> 商品集合 映射表
        ret = couponHelper.queryWareLableWares(wareLabelIdSet,wareLabelId_wareIdSet_map);
        if(0 != ret && -9999 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute queryWareLableWares"<< endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = "error occured when mysql execute queryWareLableWares";
            throw io;
        }

        map<string,set<int64_t>> platCode_wareSet_map;//平台券 --> 订单级的商品ware集合
        map<int64_t,map<string,set<int64_t>>> sellerId_code_wareSet_map;//(卖家，券) --> ware集合
        map<int64_t,map<int64_t,map<string,bool>>> sellerId_wareId_code_map;//(卖家,ware，券) --> 是否存在

        map<string,double> platCode_totalSalePrice_map;//平台券 --> 订单级的享受该平台券的商品总售价
        map<int64_t,map<string,double>> sellerId_code_totalSalePrice_map;//(卖家，店铺券) --> 卖家级享受该平台券的商品集合的总售价
        map<int64_t,map<int64_t,map<string,double>>> sellerId_skuId_code_totalSalePrice_map;//(卖家，sku，券) --> sku级享受该券的售价

        map<int64_t,vector<CacheCoupon>> sellerUsableCoupons;//按卖家分的可用的店铺券
        map<string,CacheCoupon> code_cacheCoupon_map;//按券码统计的本订单可用的券,含平台券，店铺券
        vector<CacheCoupon> platUsableCoupons;//订单级的有效返回平台券

        for(auto x:unionId_cacheCoupon_map){//遍历每个券，依次判断是否可用
            //unsigned int coupon_group_id = x.first.first;
            //unsigned int coupon_id = x.first.second;
            const CacheCoupon cacheCoupon = x.second;

            unsigned int ware_label_id = 0 ;
            auto union_id = union_id_type(cacheCoupon.coupon_group_id,cacheCoupon.coupon_id);
            if(unionId_wareLabelId_map.find(union_id) != unionId_wareLabelId_map.end()){
                ware_label_id = unionId_wareLabelId_map[union_id];
            }else{
                continue;
            }

            double sum_price = 0.0;
            set<int64_t > filteredOrderWareIdSet;//参与计算的订单级商品ware
            set<int64_t > filteredSellerWareIdSet;//参与计算的卖家级商品ware

            if(0 == cacheCoupon.scope_type){//如果是平台券
                if(0 == cacheCoupon.sub_type){//如果是白名单
                    set<int64_t> whitelist = wareLabelId_wareIdSet_map[ware_label_id];//对应的白名单
                    unsigned valid_ware_count = 0;//参与计算的订单商品的个数
                    for(auto ware_id : orderWareIdSet){//遍历订单级商品ware
                        if(whitelist.find(ware_id) != whitelist.end()){//商品ware在白名单中
                            filteredOrderWareIdSet.insert(ware_id);//得到参与计算的订单级商品ware
                            valid_ware_count ++;
                        }
                    }
                    if(0 == valid_ware_count ){
                        continue;
                    }

                }else if(1 == cacheCoupon.sub_type){//如果是黑名单
                    set<int64_t> blacklist = wareLabelId_wareIdSet_map[ware_label_id];//对应的黑名单
                    unsigned valid_ware_count = 0;//参与计算的订单商品ware的个数
                    for(auto ware_id : orderWareIdSet){//遍历订单级商品ware
                        if(blacklist.find(ware_id) == blacklist.end()){//如果不在黑名单中
                            filteredOrderWareIdSet.insert(ware_id);//得到参与计算的订单级商品ware
                            valid_ware_count ++;
                        }
                    }
                    if(0 == valid_ware_count ){
                        continue;
                    }
                }else if(2 == cacheCoupon.sub_type){//如果是全部商品ware
                    for(auto ware_id : orderWareIdSet){//遍历订单级商品ware
                        filteredOrderWareIdSet.insert(ware_id);//得到参与计算的订单级商品ware
                    }
                }

                for(auto ware_id : filteredOrderWareIdSet){//遍历参与计算的商品ware
                    sum_price += wareId_sumPrice_map[ware_id];//把订单中涉及的这些商品ware的价格累加
                }

                if(cacheCoupon.favor_type ==0 && sum_price >= cacheCoupon.full){//如果是满减券而且真的满了
                    platUsableCoupons.push_back(cacheCoupon);//这张券是一张有效的平台券，供用户勾选
                    code_cacheCoupon_map[cacheCoupon.code] = cacheCoupon;//按券码统计的本订单可用的券,含平台券，店铺券

                    platCode_wareSet_map[cacheCoupon.code] = filteredOrderWareIdSet;//平台券 --> 订单级的商品集合
                    platCode_totalSalePrice_map[cacheCoupon.code] = sum_price;//平台券 --> 订单级的享受该平台券的商品总售价

                    //针对本平台券，遍历每个店铺
                    for(auto x : usableCouponsReq.sellers){//遍历每个卖家
                        double seller_sum_price = 0.0;
                        set<int64_t > _filteredSellerWareIdSet;//参与计算的卖家级商品ware

                        int seller_id = x.first;
                        SellerInfo sellerInfo = x.second;

                        set<int64_t > sellerWareIdSet = sellerId_wareIdSet_map[seller_id];//该卖家下的商品

                        if(0 == cacheCoupon.sub_type){//如果是白名单
                            set<int64_t> whitelist = wareLabelId_wareIdSet_map[ware_label_id];//对应的白名单
                            unsigned valid_ware_count = 0;//参与计算的卖家级商品ware的个数
                            for(auto ware_id : sellerWareIdSet){//遍历该卖家商品ware
                                if(whitelist.find(ware_id) != whitelist.end()){//如果在白名单中
                                    _filteredSellerWareIdSet.insert(ware_id);//得到参与计算的卖家级商品ware
                                    valid_ware_count++;
                                }
                            }
                            if(0 == valid_ware_count ){
                                continue;
                            }
                        }else if(1 == cacheCoupon.sub_type){//如果是黑名单
                            set<int64_t> blacklist = wareLabelId_wareIdSet_map[ware_label_id];//得到对应的黑名单
                            unsigned valid_ware_count = 0;//参与计算的卖家级商品ware的个数
                            for(auto ware_id : sellerWareIdSet){//遍历卖家商品ware
                                if(blacklist.find(ware_id) == blacklist.end()){//如果该商品ware不在黑名单中
                                    _filteredSellerWareIdSet.insert(ware_id);//得到参与计算的卖家级商品ware
                                    valid_ware_count++;
                                }
                            }
                            if(0 == valid_ware_count ){
                                continue;
                            }
                        }else if(2 == cacheCoupon.sub_type){//如果是全部商品ware
                            for(auto ware_id : sellerWareIdSet){//遍历卖家商品ware
                                _filteredSellerWareIdSet.insert(ware_id);//得到参与计算的卖家级商品ware
                            }
                        }

                        map<int64_t,SkuInfo> skus = sellerInfo.skus;
                        for(auto y:skus){//遍历该卖家的商品sku
                            int64_t sku_id = y.first;
                            SkuInfo skuInfo = y.second;
                            int64_t ware_id = skuInfo.ware_id;

                            if(_filteredSellerWareIdSet.find(ware_id) != _filteredSellerWareIdSet.end()){//如果该商品ware参与计算
                                seller_sum_price += skuInfo.sale_price * skuInfo.sku_count;//计算出卖家下 该券对应的所有商品sku 的总售价
                                sellerId_wareId_code_map[seller_id][ware_id][cacheCoupon.code] = true;//(卖家，商品ware，券) --> 是否存在

                                //(卖家，sku，券) --> 商品sku级享受该券的售价
                                sellerId_skuId_code_totalSalePrice_map[seller_id][sku_id][cacheCoupon.code]
                                = skuInfo.sale_price * skuInfo.sku_count;
                            }
                        }

                        //(卖家，券) --> 卖家级享受该平台券的商品集合的总售价
                        sellerId_code_totalSalePrice_map[seller_id][cacheCoupon.code] = seller_sum_price;

                        //(卖家，券) --> 商品ware集合
                        sellerId_code_wareSet_map[seller_id][cacheCoupon.code] = _filteredSellerWareIdSet;
                    }
                }
            }else if(1 == cacheCoupon.scope_type){//如果是店铺券
                //如果订单卖家 中  没有这个卖家   跳过
                if(sellerId_wareIdSet_map.find(cacheCoupon.seller_id) == sellerId_wareIdSet_map.end()){
                    continue;
                }
                set<int64_t > sellerWareIdSet = sellerId_wareIdSet_map[cacheCoupon.seller_id];//该卖家下的商品

                if(0 == cacheCoupon.sub_type){//如果是白名单
                    set<int64_t> whitelist = wareLabelId_wareIdSet_map[ware_label_id];//对应的白名单
                    unsigned valid_ware_count = 0;//参与计算的卖家级商品ware的个数
                    for(auto ware_id : sellerWareIdSet){//遍历该卖家商品ware
                        if(whitelist.find(ware_id) != whitelist.end()){//如果在白名单中
                            filteredSellerWareIdSet.insert(ware_id);//得到参与计算的卖家级商品ware
                            valid_ware_count++;
                        }
                    }
                    if(0 == valid_ware_count ){
                        continue;
                    }
                }else if(1 == cacheCoupon.sub_type){//如果是黑名单
                    set<int64_t> blacklist = wareLabelId_wareIdSet_map[ware_label_id];//得到对应的黑名单
                    unsigned valid_ware_count = 0;//参与计算的卖家级商品ware的个数
                    for(auto ware_id : sellerWareIdSet){//遍历卖家商品ware
                        if(blacklist.find(ware_id) == blacklist.end()){//如果该商品ware不在黑名单中
                            filteredSellerWareIdSet.insert(ware_id);//得到参与计算的卖家级商品ware
                            valid_ware_count++;
                        }
                    }
                    if(0 == valid_ware_count ){
                        continue;
                    }
                }else if(2 == cacheCoupon.sub_type){//如果是全部商品ware
                    for(auto ware_id : sellerWareIdSet){//遍历卖家商品ware
                        filteredSellerWareIdSet.insert(ware_id);//得到参与计算的卖家级商品ware
                    }
                }

                for(auto x : usableCouponsReq.sellers){//遍历每个卖家
                    int seller_id = x.first;
                    SellerInfo sellerInfo = x.second;
                    if(seller_id == cacheCoupon.seller_id){//找到该券对应的卖家
                        map<int64_t,SkuInfo> skus = sellerInfo.skus;
                        for(auto y:skus){//遍历该卖家的商品sku
                            int64_t sku_id = y.first;
                            SkuInfo skuInfo = y.second;
                            int64_t ware_id = skuInfo.ware_id;

                            if(filteredSellerWareIdSet.find(ware_id) != filteredSellerWareIdSet.end()){//如果该商品ware参与计算
                                sum_price += skuInfo.sale_price * skuInfo.sku_count;//计算出卖家下 该店铺券对应的所有商品sku 的总售价
                                sellerId_wareId_code_map[cacheCoupon.seller_id][ware_id][cacheCoupon.code] = true;

                                //(卖家，商品sku，券) --> 商品sku级享受该店铺券的售价
                                sellerId_skuId_code_totalSalePrice_map[cacheCoupon.seller_id][sku_id][cacheCoupon.code]
                                = skuInfo.sale_price * skuInfo.sku_count;
                            }
                        }
                    }
                }

                if(cacheCoupon.favor_type ==0 && sum_price >= cacheCoupon.full){//如果是满减券而且真的满了
                    //如果输入参数中没有这个卖家，跳过
                    if(usableCouponsReq.sellers.find(cacheCoupon.seller_id) == usableCouponsReq.sellers.end()){
                        continue;
                    }

                    //卖家的可以勾选的券
                    sellerUsableCoupons[cacheCoupon.seller_id].push_back(cacheCoupon);
                    code_cacheCoupon_map[cacheCoupon.code] = cacheCoupon;//按券码统计的本订单可用的券,含平台券，店铺券

                    //(卖家，券) --> 卖家级享受该店铺券的商品集合的总售价
                    sellerId_code_totalSalePrice_map[cacheCoupon.seller_id][cacheCoupon.code] = sum_price;

                    //(卖家，券) --> 商品集合
                    sellerId_code_wareSet_map[cacheCoupon.seller_id][cacheCoupon.code] = filteredSellerWareIdSet;
                }
            }
        }

        vector<string> usableCodes;
        for(auto x:code_cacheCoupon_map){
            usableCodes.push_back(x.second.code);
        }

        usableCouponsRsp.error = Error::OK;
        usableCouponsRsp.errmsg = "success";
        usableCouponsRsp.user_id = usableCouponsReq.user_id ;
        usableCouponsRsp.sellers = usableCouponsReq.sellers ;

        usableCouponsRsp.platUsableCoupons = platUsableCoupons ;
        usableCouponsRsp.sellerUsableCoupons = sellerUsableCoupons ;
        usableCouponsRsp.code_cacheCoupon_map = code_cacheCoupon_map ;

        usableCouponsRsp.orderPrice = orderPrice;
        usableCouponsRsp.sellerId_priceInfo_map =sellerId_priceInfo_map;
        usableCouponsRsp.sellerId_skuId_priceInfo_map = sellerId_skuId_priceInfo_map;

        usableCouponsRsp.platCode_wareSet_map = platCode_wareSet_map;
        usableCouponsRsp.sellerId_code_wareSet_map = sellerId_code_wareSet_map;
        usableCouponsRsp.sellerId_wareId_code_map = sellerId_wareId_code_map;

        usableCouponsRsp.platCode_totalSalePrice_map = platCode_totalSalePrice_map;
        usableCouponsRsp.sellerId_code_totalSalePrice_map = sellerId_code_totalSalePrice_map;
        usableCouponsRsp.sellerId_skuId_code_totalSalePrice_map = sellerId_skuId_code_totalSalePrice_map;

        usableCouponsRsp.skuID_wareID_map = skuID_wareID_map;
        usableCouponsRsp.usableCodes = usableCodes;

        LOG_WARN<<"getUsableCoupons response is: UsableCouponsRsp '"<<ThriftToJSON(usableCouponsRsp)<<"'"<<endl;
    }

    void getApportion(ApportionRsp& apportionRsp, const ApportionReq& apportionReq){
        LOG_WARN<<"getApportion request is: ApportionReq '"<<ThriftToJSON(apportionReq)<<"'"<<endl;
        PerfWatch perfWatch("getApportion");

        //根据订单计算可用的券
        UsableCouponsReq usableCouponsReq;
        usableCouponsReq.user_id = apportionReq.user_id;
        usableCouponsReq.sellers = apportionReq.sellers;

        UsableCouponsRsp usableCouponsRsp;
        getUsableCoupons(usableCouponsRsp, usableCouponsReq);
        if(Error::OK != usableCouponsRsp.error){
            apportionRsp.error = Error::NO_USABLE_COUPON;
            apportionRsp.errmsg = "no usable coupon";
            return;
        }

        map<string,CacheCoupon> toggle_cacheCoupon_map;//用户勾选的券的信息
        map<int64_t,vector<CacheCoupon>> sellerId_cacheCoupon_map;//分摊计算后卖家可用的券set{平台券，店铺券}

        //按券码统计的本订单可用的券,含平台券，店铺券
        map<string,CacheCoupon> code_cacheCoupon_map = usableCouponsRsp.code_cacheCoupon_map;

        //过程：
        //1.判断所选平台券，是否是有效的
        //  若果无效，直接返回错误提示
        //  判断店铺券是否都是有效的
        //  若果无效，直接返回错误提示
        //2.对平台券进行分摊
        //  a.订单级分摊
        //      修改订单级的总优惠价，总支付价
        //  b.卖家级分摊
        //      卖家1享受该平台券的商品集合 ==> 卖家1享受该平台券的总售价   关系： 卖家1总优惠价/卖家1享受该平台券的总售价 = 订单级总优惠价/订单级享受该平台券的总售价
        //      ...
        //  c.商品级分摊
        //      商品1是否享受该平台券 ==> 商品1享受该平台券的总售价 关系：商品1总优惠价/商品1享受该平台券的总售价 = 订单级总优惠价/订单级享受该平台券的总售价
        //3.对店铺券进行分摊
        //  遍历每张店铺券
        //  分摊过程同2

        //如果选择了一张平台券
        string plat_code = apportionReq.plat_code;
        if(!plat_code.empty()){
            //如果不在可用券列表中
            if(code_cacheCoupon_map.find(plat_code) == code_cacheCoupon_map.end()){
                apportionRsp.error = Error::COUPON_NOT_USABLE;
                apportionRsp.errmsg = plat_code + " : code not usable";
                return;
            }

            //如果不是平台券
            if(0 != code_cacheCoupon_map[plat_code].scope_type){
                apportionRsp.error = Error::COUPON_NOT_PLATFORM;
                apportionRsp.errmsg = plat_code + " : not platform";
                return;
            }

            toggle_cacheCoupon_map[plat_code] = code_cacheCoupon_map[plat_code];

            for(auto x : usableCouponsReq.sellers){
                int64_t seller_id = x.first;
                sellerId_cacheCoupon_map[seller_id].push_back(code_cacheCoupon_map[plat_code]);
            }
        }

        if(apportionReq.sellerId_code_map.size() > 0){
            for(auto x : apportionReq.sellerId_code_map){
                int64_t seller_id = x.first;
                string shop_code = x.second;

                //如果不在可用券列表中
                if(code_cacheCoupon_map.find(shop_code) == code_cacheCoupon_map.end()){
                    apportionRsp.error = Error::COUPON_NOT_USABLE;
                    apportionRsp.errmsg = shop_code + " : code not usable";
                    return;
                }

                //如果不是店铺券
                if(1 != code_cacheCoupon_map[shop_code].scope_type){
                    apportionRsp.error = Error::COUPON_NOT_SHOPTYPE;
                    apportionRsp.errmsg = shop_code + " : not shoptype";
                    return;
                }

                //如果不能在该店铺使用
                if(seller_id != code_cacheCoupon_map[shop_code].seller_id){
                    apportionRsp.error = Error::COUPON_NOT_OF_THIS_SHOP;
                    apportionRsp.errmsg = shop_code + " : not of this shop";
                    return;
                }

                toggle_cacheCoupon_map[shop_code] = code_cacheCoupon_map[shop_code];
                sellerId_cacheCoupon_map[seller_id].push_back(code_cacheCoupon_map[shop_code]);
            }
        }

        //对平台券进行分摊
        if(!plat_code.empty()){
            if(0 == code_cacheCoupon_map[plat_code].favor_type){//如果是满减券
                //订单级分摊 修改订单级的总优惠价，总支付价
                double platCode_totalFavorPrice = code_cacheCoupon_map[plat_code].favor;//本券的优惠额，订单级优惠价

                usableCouponsRsp.orderPrice.favor_price += platCode_totalFavorPrice;//订单的优惠额
                usableCouponsRsp.orderPrice.pay_price   -= platCode_totalFavorPrice;//订单级支付价

                //订单级享受该平台券的总售价
                double platCode_totalSalePrice = usableCouponsRsp.platCode_totalSalePrice_map[plat_code];

                //卖家级分摊
                for(auto x : apportionReq.sellers){
                    int64_t seller_id = x.first;
                    SellerInfo sellerInfo = x.second;//卖家信息

                    //卖家享受该平台券的总售价
                    double sellerId_code_totalSalePrice = usableCouponsRsp.sellerId_code_totalSalePrice_map[seller_id][plat_code];

                    //卖家享受该平台券的总优惠额
                    double sellerId_code_totalFavorPrice = Round(sellerId_code_totalSalePrice * (platCode_totalFavorPrice / platCode_totalSalePrice));

                    //修正
                    usableCouponsRsp.sellerId_priceInfo_map[seller_id].favor_price += sellerId_code_totalFavorPrice;
                    usableCouponsRsp.sellerId_priceInfo_map[seller_id].pay_price
                    = usableCouponsRsp.sellerId_priceInfo_map[seller_id].sale_price
                    - usableCouponsRsp.sellerId_priceInfo_map[seller_id].favor_price;

                    //商品sku级分摊
                    map<int64_t,SkuInfo> skus = sellerInfo.skus;//该卖家的商品信息
                    for(auto y:skus){//遍历该卖家的商品sku
                        int64_t sku_id = y.first;
                        SkuInfo skuInfo = y.second;//该商品sku的信息
                        //int64_t ware_id = skuInfo.ware_id;

                        //商品sku享受该平台券的总售价
                        double sku_sale_price = 0.0;
                        if(usableCouponsRsp.sellerId_skuId_code_totalSalePrice_map[seller_id].find(sku_id)
                                != usableCouponsRsp.sellerId_skuId_code_totalSalePrice_map[seller_id].end()){
                            if(usableCouponsRsp.sellerId_skuId_code_totalSalePrice_map[seller_id][sku_id].find(plat_code)
                                    != usableCouponsRsp.sellerId_skuId_code_totalSalePrice_map[seller_id][sku_id].end()){
                                sku_sale_price = usableCouponsRsp.sellerId_skuId_code_totalSalePrice_map[seller_id][sku_id][plat_code];
                            }
                        }

                        double sku_favor_price = Round(sku_sale_price * (platCode_totalFavorPrice / platCode_totalSalePrice));

                        usableCouponsRsp.sellerId_skuId_priceInfo_map[seller_id][sku_id].favor_price += sku_favor_price;
                        usableCouponsRsp.sellerId_skuId_priceInfo_map[seller_id][sku_id].pay_price
                        = usableCouponsRsp.sellerId_skuId_priceInfo_map[seller_id][sku_id].sale_price
                        - usableCouponsRsp.sellerId_skuId_priceInfo_map[seller_id][sku_id].favor_price;
                    }
                }
            }
        }

        //对店铺券进行分摊
        if(apportionReq.sellerId_code_map.size() > 0){
            //  遍历每张店铺券
            for(auto x : apportionReq.sellerId_code_map){
                int64_t seller_id = x.first;
                string shop_code = x.second;

                if(0 == code_cacheCoupon_map[shop_code].favor_type){//如果是满减券
                    //订单级分摊
                    double shopCode_totalFavorPrice = code_cacheCoupon_map[shop_code].favor;//订单级优惠价

                    usableCouponsRsp.orderPrice.favor_price += shopCode_totalFavorPrice;
                    usableCouponsRsp.orderPrice.pay_price   -= shopCode_totalFavorPrice;//订单级支付价

                    //卖家级分摊
                    usableCouponsRsp.sellerId_priceInfo_map[seller_id].favor_price += shopCode_totalFavorPrice;
                    usableCouponsRsp.sellerId_priceInfo_map[seller_id].pay_price
                    = usableCouponsRsp.sellerId_priceInfo_map[seller_id].sale_price
                    - usableCouponsRsp.sellerId_priceInfo_map[seller_id].favor_price;

                    //商品sku级分摊
                    SellerInfo sellerInfo = usableCouponsRsp.sellers[seller_id];
                    map<int64_t,SkuInfo> skus = sellerInfo.skus;//该卖家的商品信息
                    for(auto y:skus){//遍历该卖家的商品sku
                        int64_t sku_id = y.first;
                        SkuInfo skuInfo = y.second;//该商品sku的信息
                        //int64_t ware_id = skuInfo.ware_id;

                        //商品sku享受该平台券的总售价
                        double sku_sale_price = 0.0;
                        if(usableCouponsRsp.sellerId_skuId_code_totalSalePrice_map[seller_id].find(sku_id)
                                != usableCouponsRsp.sellerId_skuId_code_totalSalePrice_map[seller_id].end()){
                            if(usableCouponsRsp.sellerId_skuId_code_totalSalePrice_map[seller_id][sku_id].find(shop_code)
                                    != usableCouponsRsp.sellerId_skuId_code_totalSalePrice_map[seller_id][sku_id].end()){
                                sku_sale_price = usableCouponsRsp.sellerId_skuId_code_totalSalePrice_map[seller_id][sku_id][shop_code];
                            }
                        }

                        double sellerId_code_totalSalePrice = usableCouponsRsp.sellerId_code_totalSalePrice_map[seller_id][shop_code];

                        double sku_favor_price = Round(sku_sale_price * (shopCode_totalFavorPrice / sellerId_code_totalSalePrice));

                        usableCouponsRsp.sellerId_skuId_priceInfo_map[seller_id][sku_id].favor_price += sku_favor_price ;
                        usableCouponsRsp.sellerId_skuId_priceInfo_map[seller_id][sku_id].pay_price
                        = usableCouponsRsp.sellerId_skuId_priceInfo_map[seller_id][sku_id].sale_price
                        - usableCouponsRsp.sellerId_skuId_priceInfo_map[seller_id][sku_id].favor_price;
                    }
                }
            }
        }

        apportionRsp.error = Error::OK;
        apportionRsp.errmsg = "success";
        apportionRsp.usableCouponsRsp = usableCouponsRsp;
        apportionRsp.toggle_cacheCoupon_map = toggle_cacheCoupon_map;
        apportionRsp.sellerId_cacheCoupon_map = sellerId_cacheCoupon_map;

        //价格精度保留小数点后两位，不宜在分摊过程中进行四舍五入计算，容易造成精度损失
        apportionRsp.usableCouponsRsp.orderPrice.pay_price = Round(apportionRsp.usableCouponsRsp.orderPrice.pay_price);
        apportionRsp.usableCouponsRsp.orderPrice.favor_price = Round(apportionRsp.usableCouponsRsp.orderPrice.favor_price);

        double sellers_sum_favor_price = 0.0;//所有卖家的优惠总额
        for(auto &x:apportionRsp.usableCouponsRsp.sellerId_priceInfo_map){
            PriceInfo &priceInfo = x.second;
            priceInfo.pay_price = Round(priceInfo.pay_price);
            priceInfo.favor_price = Round(priceInfo.favor_price);

            sellers_sum_favor_price += priceInfo.favor_price;
        }

        for(auto &x : apportionRsp.usableCouponsRsp.sellerId_skuId_priceInfo_map){
            for(auto &y : x.second){
                PriceInfo &priceInfo = y.second;
                priceInfo.pay_price = Round(priceInfo.pay_price);
                priceInfo.favor_price = Round(priceInfo.favor_price);
            }
        }

        //卖家级修正，误差必须在0.05以内
        double inaccuracy = apportionRsp.usableCouponsRsp.orderPrice.favor_price - sellers_sum_favor_price;
        double inaccuracy_rate = inaccuracy/apportionRsp.usableCouponsRsp.orderPrice.favor_price;

        if((0.0 < abs(inaccuracy) && abs(inaccuracy) < INACCURACY_UPLIMIT)
            ||(0.0 < abs(inaccuracy_rate) && abs(inaccuracy_rate) < INACCURACY_RATE_UPLIMIT))
        {
            int64_t target_seller_id = 0;

            for(auto &y:apportionRsp.usableCouponsRsp.sellerId_skuId_priceInfo_map){
                int64_t seller_id = y.first;

                string this_shop_code = "";
                if(apportionReq.sellerId_code_map.end() != apportionReq.sellerId_code_map.find(seller_id)){
                    this_shop_code = apportionReq.sellerId_code_map.at(seller_id);
                }

                for(auto &z:y.second){
                    int64_t sku_id = z.first;
                    int64_t ware_id = apportionRsp.usableCouponsRsp.skuID_wareID_map[sku_id];

                    bool code_can_use = false;
                    if(apportionRsp.usableCouponsRsp.sellerId_wareId_code_map.end()
                    != apportionRsp.usableCouponsRsp.sellerId_wareId_code_map.find(seller_id))
                    {
                        if(apportionRsp.usableCouponsRsp.sellerId_wareId_code_map[seller_id].end()
                        != apportionRsp.usableCouponsRsp.sellerId_wareId_code_map[seller_id].find(ware_id))
                        {
                            if((!plat_code.empty() && apportionRsp.usableCouponsRsp.sellerId_wareId_code_map[seller_id][ware_id].end()
                                != apportionRsp.usableCouponsRsp.sellerId_wareId_code_map[seller_id][ware_id].find(plat_code))
                              ||(!this_shop_code.empty() && apportionRsp.usableCouponsRsp.sellerId_wareId_code_map[seller_id][ware_id].end()
                                != apportionRsp.usableCouponsRsp.sellerId_wareId_code_map[seller_id][ware_id].find(this_shop_code)))
                            {
                                code_can_use = true;
                            }
                        }
                    }

                    PriceInfo &priceInfo = z.second;

                    if((0.0 < inaccuracy && inaccuracy < INACCURACY_UPLIMIT)
                        ||(0.0 < inaccuracy_rate && inaccuracy_rate < INACCURACY_RATE_UPLIMIT))
                    {
                        if(priceInfo.pay_price - inaccuracy > 0.0 && code_can_use){
                            priceInfo.pay_price = priceInfo.pay_price - inaccuracy;
                            priceInfo.favor_price = priceInfo.favor_price + inaccuracy;

                            target_seller_id = seller_id;
                            break;
                        }
                    }

                    if(( -INACCURACY_UPLIMIT < inaccuracy && inaccuracy < 0.0)
                        ||( -INACCURACY_RATE_UPLIMIT < inaccuracy_rate && inaccuracy_rate < 0.0))
                    {
                        if(priceInfo.favor_price + inaccuracy > 0.0 && code_can_use){
                            priceInfo.pay_price = priceInfo.pay_price - inaccuracy;
                            priceInfo.favor_price = priceInfo.favor_price + inaccuracy;

                            target_seller_id = seller_id;
                            break;
                        }
                    }
                }

                if(target_seller_id > 0){
                    break;
                }
            }

            for(auto &x:apportionRsp.usableCouponsRsp.sellerId_priceInfo_map){
                int64_t seller_id = x.first;
                if(seller_id == target_seller_id){
                    PriceInfo &priceInfo = x.second;
                    priceInfo.pay_price = priceInfo.pay_price - inaccuracy;
                    priceInfo.favor_price = priceInfo.favor_price + inaccuracy;
                }
            }
        }

        //sku级修正，误差必须在0.05以内
        for(auto &y:apportionRsp.usableCouponsRsp.sellerId_skuId_priceInfo_map){
            int64_t seller_id = y.first;

            string this_shop_code = "";
            if(apportionReq.sellerId_code_map.end() != apportionReq.sellerId_code_map.find(seller_id)){
                this_shop_code = apportionReq.sellerId_code_map.at(seller_id);
            }

            double inaccuracy = 0.0;//本卖家优惠误差
            double inaccuracy_rate = 0.0;//本卖家优惠误差率
            double skus_sum_favor_price = 0.0;//本卖家所有sku的优惠总额
            for(auto &z:y.second){
                //int64_t sku_id = z.first;
                PriceInfo &priceInfo = z.second;
                skus_sum_favor_price += priceInfo.favor_price;
            }

            for(auto &x:apportionRsp.usableCouponsRsp.sellerId_priceInfo_map){
                int64_t target_seller_id = x.first;
                if(seller_id == target_seller_id){
                    PriceInfo &priceInfo = x.second;
                    inaccuracy = priceInfo.favor_price - skus_sum_favor_price;
                    inaccuracy_rate = inaccuracy/priceInfo.favor_price;
                    break;
                }
            }

            if((0.0 < abs(inaccuracy) && abs(inaccuracy) < INACCURACY_UPLIMIT)
                ||(0.0 < abs(inaccuracy_rate) && abs(inaccuracy_rate) < INACCURACY_RATE_UPLIMIT))
            {
                for(auto &z:y.second){
                    int64_t sku_id = z.first;
                    int64_t ware_id = apportionRsp.usableCouponsRsp.skuID_wareID_map[sku_id];

                    bool code_can_use = false;
                    if(apportionRsp.usableCouponsRsp.sellerId_wareId_code_map.end()
                    != apportionRsp.usableCouponsRsp.sellerId_wareId_code_map.find(seller_id))
                    {
                        if(apportionRsp.usableCouponsRsp.sellerId_wareId_code_map[seller_id].end()
                        != apportionRsp.usableCouponsRsp.sellerId_wareId_code_map[seller_id].find(ware_id))
                        {
                            if((!plat_code.empty() && apportionRsp.usableCouponsRsp.sellerId_wareId_code_map[seller_id][ware_id].end()
                                != apportionRsp.usableCouponsRsp.sellerId_wareId_code_map[seller_id][ware_id].find(plat_code))
                              ||(!this_shop_code.empty() && apportionRsp.usableCouponsRsp.sellerId_wareId_code_map[seller_id][ware_id].end()
                                != apportionRsp.usableCouponsRsp.sellerId_wareId_code_map[seller_id][ware_id].find(this_shop_code)))
                            {
                                code_can_use = true;
                            }
                        }
                    }

                    PriceInfo &priceInfo = z.second;

                    if((0.0 < inaccuracy && inaccuracy < INACCURACY_UPLIMIT)
                        ||(0.0 < inaccuracy_rate && inaccuracy_rate < INACCURACY_RATE_UPLIMIT))
                    {
                        if(priceInfo.pay_price - inaccuracy > 0.0 && code_can_use){
                            priceInfo.pay_price = priceInfo.pay_price - inaccuracy;
                            priceInfo.favor_price = priceInfo.favor_price + inaccuracy;
                            break;
                        }
                    }

                    if(( -INACCURACY_UPLIMIT < inaccuracy && inaccuracy < 0.0)
                        ||( -INACCURACY_RATE_UPLIMIT < inaccuracy_rate && inaccuracy_rate < 0.0))
                    {
                        if(priceInfo.favor_price + inaccuracy > 0.0 && code_can_use){
                            priceInfo.pay_price = priceInfo.pay_price - inaccuracy;
                            priceInfo.favor_price = priceInfo.favor_price + inaccuracy;
                            break;
                        }
                    }
                }
            }
        }

        LOG_WARN<<"getApportion response is: ApportionRsp '"<<ThriftToJSON(apportionRsp)<<"'"<<endl;
    }

    void getCacheCouponGroups(CacheCouponGroupsRsp& cacheCouponGroupsRsp, const CacheCouponGroupsReq& cacheCouponGroupsReq){
        LOG_WARN<<"getCacheCouponGroups request is: CacheCouponGroupsReq '"<<ThriftToJSON(cacheCouponGroupsReq)<<"'"<<endl;
        PerfWatch perfWatch("getCacheCouponGroups");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected when getUsableCoupons"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected when getUsableCoupons";
            throw io;
        }

        if(cacheCouponGroupsReq.coupon_group_id_set.size() == 0){
            io.fault = Error::INVALID_PARAMS;
            io.why = "invalid params set.size == 0";
            throw io;
        }

        map<int,CouponGroup> couponGroupMap;
        int ret = couponHelper.queryCouponGroups(cacheCouponGroupsReq.coupon_group_id_set,couponGroupMap);
        if(-9999 == ret){
            LOG_ERROR<<"ret="<<ret<<" queryCouponGroups no any expected data!"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            cacheCouponGroupsRsp.error = Error::NO_DATA_YOU_EXPECTED;
            cacheCouponGroupsRsp.errmsg = "no coupon group you expected";
            return;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        vector<CacheCouponGroup> cacheCouponGroupList ;
        for (auto x:couponGroupMap){
            CouponGroup couponGroup = x.second;

            CacheCouponGroup cacheCouponGroup;
            cacheCouponGroup.name               = couponGroup.name;
            cacheCouponGroup.title              = couponGroup.title;
            cacheCouponGroup.comment            = couponGroup.comment;
            cacheCouponGroup.ware_label_id      = couponGroup.ware_label_id;
            cacheCouponGroup.favor_type         = couponGroup.favor_type;
            cacheCouponGroup.scope_type         = couponGroup.scope_type;
            cacheCouponGroup.sub_type           = couponGroup.sub_type;
            cacheCouponGroup.scene_type         = couponGroup.scene_type;
            cacheCouponGroup.full               = couponGroup.full;
            cacheCouponGroup.favor              = couponGroup.favor;
            cacheCouponGroup.rate               = couponGroup.rate;
            cacheCouponGroup.argot              = couponGroup.argot;
            cacheCouponGroup.max_count          = couponGroup.max_count;
            cacheCouponGroup.drawn_count        = couponGroup.drawn_count;
            cacheCouponGroup.create_time        = couponGroup.create_time;
            cacheCouponGroup.can_draw_count     = couponGroup.can_draw_count;
            cacheCouponGroup.start_draw_time    = couponGroup.start_draw_time;
            cacheCouponGroup.end_draw_time      = couponGroup.end_draw_time;
            cacheCouponGroup.seller_id          = couponGroup.seller_id;
            cacheCouponGroup.url                = couponGroup.url;
            cacheCouponGroup.id                 = couponGroup.id;
            cacheCouponGroup.img                = couponGroup.img;
            cacheCouponGroup.slug               = couponGroup.slug;
            cacheCouponGroup.img_width          = couponGroup.img_width   ;
            cacheCouponGroup.img_height         = couponGroup.img_height   ;
            cacheCouponGroup.button_text        = couponGroup.button_text   ;
            cacheCouponGroup.button_jump        = couponGroup.button_jump   ;
            cacheCouponGroup.jump_label         = couponGroup.jump_label   ;
            cacheCouponGroup.jump_data          = couponGroup.jump_data   ;
            cacheCouponGroup.argot_jump_label   = couponGroup.argot_jump_label   ;
            cacheCouponGroup.argot_jump_data    = couponGroup.argot_jump_data   ;
            cacheCouponGroup.update_time        = couponGroup.update_time   ;
            cacheCouponGroup.version            = couponGroup.version   ;
            cacheCouponGroup.description        = couponGroup.description   ;

            cacheCouponGroupList.push_back(cacheCouponGroup);
        }

        cacheCouponGroupsRsp.error = Error::OK;
        cacheCouponGroupsRsp.errmsg = "success";
        cacheCouponGroupsRsp.cacheCouponGroupList = cacheCouponGroupList;

        LOG_WARN<<"getCacheCouponGroups response is: CacheCouponGroupsRsp '"<<ThriftToJSON(cacheCouponGroupsRsp)<<"'"<<endl;
    }

    void getRelatedCouponGroups(RelatedCouponGroupsRsp& relatedCouponGroupsRsp, const RelatedCouponGroupsReq& relatedCouponGroupsReq){
        LOG_WARN<<"getRelatedCouponGroups request is: RelatedCouponGroupsReq '"<<ThriftToJSON(relatedCouponGroupsReq)<<"'"<<endl;
        PerfWatch perfWatch("getCacheCouponGroups");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected when getUsableCoupons"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected when getUsableCoupons";
            throw io;
        }

        CacheCouponGroupsReq cacheCouponGroupsReq;
        cacheCouponGroupsReq.coupon_group_id_set.insert(relatedCouponGroupsReq.coupon_group_id);

        CacheCouponGroupsRsp cacheCouponGroupsRsp;
        getCacheCouponGroups(cacheCouponGroupsRsp, cacheCouponGroupsReq);
        if(cacheCouponGroupsRsp.error == Error::OK && cacheCouponGroupsRsp.cacheCouponGroupList.size() == 1){
            CacheCouponGroup cacheCouponGroup = cacheCouponGroupsRsp.cacheCouponGroupList[0];

            vector<CouponGroup> couponGroupList;
            MultipleCondition cond;
            cond.andCondList.push_back(string("ware_label_id = ")+to_string(cacheCouponGroup.ware_label_id));
            cond.andCondList.push_back("verify_status = 2");
            int ret = couponHelper.getCouponGroupList(couponGroupList,cond,0,100);
            if(-9999 == ret){
                relatedCouponGroupsRsp.error = coupon::Error::OK,
                relatedCouponGroupsRsp.errmsg = "success";
                return ;
            }

            if(0 != ret){
                relatedCouponGroupsRsp.error = Error::MYSQL_EXECUTE_ERROR;
                relatedCouponGroupsRsp.errmsg = couponHelper.getError();
                return ;
            }

            vector<CouponGroup> validCouponGroupList;
            for(auto x : couponGroupList){
                if(x.start_draw_time <= time(NULL) && time(NULL) <= x.end_draw_time){
                    validCouponGroupList.push_back(x);
                    continue;
                }

                if(!x.is_duration_type){
                    if(x.start_use_time <= time(NULL) && time(NULL) <= x.end_use_time){
                        validCouponGroupList.push_back(x);
                        continue;
                    }
                }else{
                    if(x.start_draw_time <= time(NULL) && time(NULL) <= x.end_draw_time + x.duration_value*24*3600){
                        validCouponGroupList.push_back(x);
                        continue;
                    }
                }
            }

            relatedCouponGroupsRsp.error = Error::OK;
            relatedCouponGroupsRsp.errmsg = "success";
            relatedCouponGroupsRsp.couponGroupList = validCouponGroupList;
        }

        relatedCouponGroupsRsp.error = Error::OK;
        relatedCouponGroupsRsp.errmsg = "success";

        LOG_WARN<<"getRelatedCouponGroups response is: RelatedCouponGroupsRsp '"<<ThriftToJSON(relatedCouponGroupsRsp)<<"'"<<endl;
    }

    void getWareLabel(WareLabelRsp& wareLabelRsp, const WareLabelReq& wareLabelReq){
        LOG_WARN<<"getWareLabel request is: WareLabelReq '"<<ThriftToJSON(wareLabelReq)<<"'"<<endl;
        PerfWatch perfWatch("getWareLabel");
        InvalidOperation io;

        //如果获取的mysql连接断开
        LOG_INFO <<"mpool zise:" <<mpool->Size();
        MysqlWrapperGuard mwrapperGuard(mpool,mpool->GetMysqlWrapper());
        MysqlWrapper* mwrapper = mwrapperGuard.getMysqlWrapper();
        CouponHelper couponHelper(mwrapper);
        LOG_INFO <<"mpool zise:" <<mpool->Size()<<" mwrapper:"<<mwrapper;
        if(NULL == mwrapper||mwrapper->isClosed()){
            LOG_ERROR<<"mysql connection disconnected when getUsableCoupons"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "mysql connection disconnected when getUsableCoupons";
            throw io;
        }

        map<unsigned int,WareLabel> wareLabels;
        set<unsigned int > ware_label_ids;
        ware_label_ids.insert(wareLabelReq.ware_label_id);
        int ret = couponHelper.queryWareLabels(ware_label_ids,wareLabels);
        if(0 != ret || wareLabels.end() == wareLabels.find(wareLabelReq.ware_label_id)){
            wareLabelRsp.error = Error::OK;
            wareLabelRsp.errmsg = "success";
            return;
        }

        vector<WareLabelWares> wareLabelWaresList;
        MultipleCondition cond;
        cond.andCondList.push_back(string("ware_label_id = ") + to_string(wareLabelReq.ware_label_id));
        ret = couponHelper.getWareLabelWaresList(wareLabelWaresList,cond, 0,1000);
        if(0 != ret){
            wareLabelRsp.error = Error::OK;
            wareLabelRsp.errmsg = "success";
            wareLabelRsp.wareLabel = wareLabels[wareLabelReq.ware_label_id];
            return;
        }

        wareLabelRsp.error = Error::OK;
        wareLabelRsp.errmsg = "success";
        wareLabelRsp.wareLabel = wareLabels[wareLabelReq.ware_label_id];
        wareLabelRsp.waresList = wareLabelWaresList;

        LOG_WARN<<"getWareLabel response is: WareLabelRsp '"<<ThriftToJSON(wareLabelRsp)<<"'"<<endl;
    }
};

class CouponECServiceCloneFactory : virtual public CouponECServiceIfFactory {
public:
    virtual ~CouponECServiceCloneFactory(){

    }

    virtual CouponECServiceIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo){
        boost::shared_ptr<TSocket> sock = boost::dynamic_pointer_cast<TSocket>(connInfo.transport);
        LOG_INFO << "Incoming connection\n";
        LOG_INFO << "\tSocketInfo: "  << sock->getSocketInfo() << "\n";
        LOG_INFO << "\tPeerHost: "    << sock->getPeerHost() << "\n";
        LOG_INFO << "\tPeerAddress: " << sock->getPeerAddress() << "\n";
        LOG_INFO << "\tPeerPort: "    << sock->getPeerPort() << "\n";
        return new CouponECServiceHandler;
    }

    virtual void releaseHandler( ::couponEC::CouponECServiceIf* handler) {
        delete handler;
    }
};
