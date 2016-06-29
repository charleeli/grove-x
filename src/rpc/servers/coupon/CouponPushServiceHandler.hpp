/*
 * author:charlee
 */

#include "JsonHelper.h"
#include "JsonMacro.h"
#include "ThriftHelper.h"
#include "MysqlWrapperPool.h"
#include "RedisWrapperPool.h"
#include "MysqlQueryHelper.h"
#include "sdk-cpp/push/CouponPushService.h"
#include "CouponCode.h"
#include "CouponHelper.h"
#include "CouponConfig.h"
#include "Singleton.h"
#include "System.h"
#include "CouponUtil.h"

using namespace std;
using namespace coupon;
using namespace couponPush;
using namespace System;

class CouponPushServiceHandler : public CouponPushServiceIf {
private:
    MysqlWrapperPool * mpool;
    RedisWrapperPool * rpool;
public:
    CouponPushServiceHandler() {
        string  redis_ip_       = couponConfig.getRedisIp();
        int     redis_port_     = couponConfig.getRedisPort();
        int     redis_timeout_  = couponConfig.getRedisTimeout();
        int     redis_poolsize_ = couponConfig.getPushTaskThreadsNum();

        string  mysql_ip_       = couponConfig.getMysqlIp();
        int     mysql_port_     = couponConfig.getMysqlPort();
        string  mysql_user_     = couponConfig.getMysqlUser();
        string  mysql_passwd_   = couponConfig.getMysqlPasswd();
        string  mysql_database_ = couponConfig.getMysqlDatabase();
        int     mysql_poolsize_    = couponConfig.getPushTaskThreadsNum();

        mpool = MysqlWrapperPool::GetInstance(mysql_ip_,mysql_port_,mysql_user_,mysql_passwd_,mysql_database_,mysql_poolsize_);
        rpool = RedisWrapperPool::GetInstance(redis_ip_,redis_port_,redis_timeout_,redis_poolsize_);
    }

    void getNameList(NameListRsp& nameListRsp, const NameListReq& nameListReq){
        LOG_WARN<<"getNameList request is: NameListReq '"<<ThriftToJSON(nameListReq)<<"'"<<endl;
        PerfWatch perfWatch("getNameList");
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
            LOG_ERROR<<"Redis connection disconnected"<< endl;
            io.fault = Error::REDIS_DISCONNECTED;
            io.why = "Redis connection disconnected";
            throw io;
        }

        //查询 手中持有 已领取状态券的 用户个数
        int ret = qh.runSelect(string("")
            +" select count(*) from "
            +" ( select user_id "
            +"   from " + user_coupons_table
            +"   where use_status = 0 group by user_id having count(user_id) > 0"
            +" ) temp"
        );

        int total = qh.getIntegerField("count(*)");
        if(0 == total){
            LOG_ERROR<<"no data you expected in coupon_usercoupons via your condition"<< endl;
            nameListRsp.error = Error::OK;
            nameListRsp.errmsg = "no data you expected in coupon_usercoupons via your condition";
            return;
        }

        //页码，每页数量
        int page = nameListReq.page <= 0?1:nameListReq.page;
        int count = nameListReq.count <= 0?100:nameListReq.count;

        if(total < (page-1)*count){
            LOG_ERROR<<"no data you expected in coupon_usercoupons via your condition"<< endl;
            nameListRsp.error = Error::NO_DATA_YOU_EXPECTED;
            nameListRsp.errmsg = "no data you expected in coupon_usercoupons via your condition";
            return;
        }

        //分页查询 手中持有 已领取状态券的 用户
        int offset = (page-1)*count;
        ret = qh.runSelect(string("")
            +" select user_id from " + user_coupons_table
            +" where use_status = 0 "
            +" group by user_id having count(user_id) > 0 "
            +" limit " + to_string(offset)
            +" ," + to_string(count)
        );

        if(-9999 == ret || !qh.hasRow()){
            LOG_ERROR<<"no data you expected in coupon_usercoupons via your condition"<< endl;
            nameListRsp.error = Error::NO_DATA_YOU_EXPECTED;
            nameListRsp.errmsg = "no data you expected in coupon_usercoupons via your condition";
            return;
        }

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql getNameList"<< endl;
            LOG_ERROR<<"the error detail is that: "<<qh.getError()<<endl;
            nameListRsp.error = Error::NO_DATA_YOU_EXPECTED;
            nameListRsp.errmsg = qh.getError();
            return ;
        }

        vector<PushInfo> pushInfoList;
        //遍历每个用户
        int rows_size = qh.getRows().size();
        for(int row = 0;row<rows_size;row++){
            int64_t user_id = qh.getLongIntegerField("user_id",row);

            PushInfo pushInfo;
            pushInfo.user_id = user_id;
            pushInfo.amount = 0.0;

            //用户的券 的缓存
            CacheCodes cacheCodes;
            map<string,CacheCoupon> cacheCouponMap;
            const string cache_codes_key = cache_codes_prefix + to_string(user_id);
            string json_cache_codes = "";
            int ret = rwrapper->getValue(cache_codes_key,json_cache_codes);
            if(0 == ret){
                JSONToThrift(json_cache_codes,&cacheCodes);
                map<string, string> code_cacheCoupon_map;

                vector<string> v;
                for(auto x:cacheCodes.codes){
                    string cache_coupon_key = cache_coupon_prefix + to_string(user_id) + "_" + x;
                    v.push_back(cache_coupon_key);
                }

                rwrapper->getBatchString(v, code_cacheCoupon_map);
                if(0 == ret){
                    for(auto x:code_cacheCoupon_map){
                        CacheCoupon cacheCoupon;
                        JSONToThrift(x.second,&cacheCoupon);
                        //如果1天内过期
                        if(time(NULL) < cacheCoupon.end_use_time && cacheCoupon.use_status == 0 &&
                            cacheCoupon.end_use_time <= time(NULL) + 24*60*60 && cacheCoupon.favor_type == 0){
                            pushInfo.amount += cacheCoupon.favor;
                        }
                }
            }

            if(pushInfo.amount > 0.0)
                pushInfoList.push_back(pushInfo);
            }
        }

        nameListRsp.error = Error::OK;
        nameListRsp.errmsg = "";
        nameListRsp.page  = page;
        nameListRsp.count = nameListReq.count;
        nameListRsp.total = total;
        nameListRsp.pushInfoList = pushInfoList;

        LOG_WARN<<"getNameList response is: NameListRsp '"<<ThriftToJSON(nameListRsp)<<"'"<<endl;
        return ;
    }

    void getPushInfo(PushInfoRsp& pushInfoRsp, const PushInfoReq& pushInfoReq){
        LOG_WARN<<"getPushInfo request is: PushInfoReq '"<<ThriftToJSON(pushInfoReq)<<"'"<<endl;
        PerfWatch perfWatch("getPushInfo");
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
            LOG_ERROR<<"Redis connection disconnected"<< endl;
            io.fault = Error::REDIS_DISCONNECTED;
            io.why = "Redis connection disconnected";
            throw io;
        }

        vector<PushInfo> pushInfoList;
        //遍历每个用户
        for(auto user_id:pushInfoReq.user_id_set){
            PushInfo pushInfo;
            pushInfo.user_id = user_id;
            pushInfo.amount = 0.0;

            //用户的券 的缓存
            CacheCodes cacheCodes;
            map<string,CacheCoupon> cacheCouponMap;
            const string cache_codes_key = cache_codes_prefix + to_string(user_id);
            string json_cache_codes = "";
            int ret = rwrapper->getValue(cache_codes_key,json_cache_codes);
            if(0 == ret){
                JSONToThrift(json_cache_codes,&cacheCodes);
                map<string, string> code_cacheCoupon_map;

                vector<string> v;
                for(auto x:cacheCodes.codes){
                    string cache_coupon_key = cache_coupon_prefix + to_string(user_id) + "_" + x;
                    v.push_back(cache_coupon_key);
                }

                rwrapper->getBatchString(v, code_cacheCoupon_map);
                if(0 == ret){
                    for(auto x:code_cacheCoupon_map){
                        CacheCoupon cacheCoupon;
                        JSONToThrift(x.second,&cacheCoupon);
                        //如果1天内过期
                        if(time(NULL) < cacheCoupon.end_use_time && cacheCoupon.use_status == 0 &&
                            cacheCoupon.end_use_time <= time(NULL) + 24*60*60 && cacheCoupon.favor_type == 0){
                            pushInfo.amount += cacheCoupon.favor;
                        }
                    }
                }

                if(pushInfo.amount > 0.0)
                    pushInfoList.push_back(pushInfo);
            }
        }

        pushInfoRsp.error = Error::OK;
        pushInfoRsp.errmsg = "";
        pushInfoRsp.pushInfoList = pushInfoList;

        LOG_WARN<<"getPushInfo response is: PushInfoRsp '"<<ThriftToJSON(pushInfoRsp)<<"'"<<endl;
        return ;
    }
};

class CouponPushServiceCloneFactory : virtual public CouponPushServiceIfFactory {
public:
    virtual ~CouponPushServiceCloneFactory() {

    }

    virtual CouponPushServiceIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo){
        boost::shared_ptr<TSocket> sock = boost::dynamic_pointer_cast<TSocket>(connInfo.transport);
        LOG_INFO << "Incoming connection\n";
        LOG_INFO << "\tSocketInfo: "  << sock->getSocketInfo() << "\n";
        LOG_INFO << "\tPeerHost: "    << sock->getPeerHost() << "\n";
        LOG_INFO << "\tPeerAddress: " << sock->getPeerAddress() << "\n";
        LOG_INFO << "\tPeerPort: "    << sock->getPeerPort() << "\n";
        return new CouponPushServiceHandler;
    }

    virtual void releaseHandler( ::couponPush::CouponPushServiceIf* handler) {
        delete handler;
    }
};
