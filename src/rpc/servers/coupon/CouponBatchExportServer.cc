#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <ev.h>
#include <iostream>
#include <exception>
#include <memory>
#include "System.h"
#include "GLogHelper.h"
#include "CouponConfig.h"
#include "CouponConst.h"
#include "CouponCode.h"
#include "CouponHelper.h"
#include "mysqlwrapper.h"
#include "MysqlQueryHelper.h"
#include "System.h"
#include "Singleton.h"
#include "ThriftHelper.h"
#include "ThriftClient.h"
#include "sdk-cpp/admin/couponAdmin_types.h"
#include "sdk-cpp/ec/CouponECService.h"
extern "C"{
#include "redismq/redismq.h"
}

using namespace std;
using namespace System;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace coupon;
using namespace couponEC;
using namespace couponAdmin;

CouponConfig couponConfig;

static void OnExitSignal(int){
    LOG_ERROR<<"coupon_batch_export_server exit!"<<endl;
    cout<<"coupon_batch_export_server exit!"<<endl;
    exit(-1);
}

void IgnoreSignal(){
    signal(SIGHUP,SIG_IGN);
    signal(SIGPIPE,SIG_IGN);
    signal(SIGCHLD,SIG_IGN);
    signal(SIGINT,OnExitSignal);
    signal(SIGQUIT,OnExitSignal);
    signal(SIGTERM,OnExitSignal);
}

void PrintHelp(const char* program_name){
    std::cout << "Usage:" << program_name << " conf_path." << std::endl;
}

void blpop_cb(char *msg){
    MysqlWrapperPool* mpool = MysqlWrapperPool::GetInstance(
        couponConfig.getMysqlIp(),couponConfig.getMysqlPort(),
        couponConfig.getMysqlUser(),couponConfig.getMysqlPasswd(),
        couponConfig.getMysqlDatabase(),2);

    BatchExportReq batchExportReq;
    InvalidOperation io;

    try{
        JSONToThrift(string(msg),&batchExportReq);

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

        //如果count不合法
        int ret = queryHelper.runSelect(string("")
            +" select count(*)"
            +" from " + coupon_table_prefix + to_string(batchExportReq.coupon_group_id)
        );

        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        int count = queryHelper.getIntegerField("count(*)");

        if(batchExportReq.count + count > 10000000 ||batchExportReq.count == 0){
            LOG_ERROR<<"count outload... count =  "<<batchExportReq.count<< endl;
            io.fault = coupon::Error::OUTLOAD_COUNT;
            io.why = "count outload...";
            throw io;
        }

        //校验组ID,如果这个组在表中不存在
        CouponGroup couponGroup;
        ret = couponHelper.queryCouponGroup(batchExportReq.coupon_group_id,couponGroup);
        if(0 != ret){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute"<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        //如果不是批量导出券
        if(3 != couponGroup.scene_type){
            LOG_ERROR<<" CouponGroup is not a batch-export group..."<< endl;
            io.fault = Error::ILLEGAL_SCENE_TYPE;
            io.why = "CouponGroup is not a batch-export group...";
            throw io;
        }

        //如果对应的券表不存在，则产生一张券表
        ret = couponHelper.createCouponTable(batchExportReq.coupon_group_id);
        if(ret < 0){
            LOG_ERROR<<"ret="<<ret<<" error occured when mysql execute..."<< endl;
            LOG_ERROR<<"the error detail is that: "<<couponHelper.getError()<<endl;
            io.fault = Error::MYSQL_EXECUTE_ERROR;
            io.why = couponHelper.getError();
            throw io;
        }

        for(int i=0;i<batchExportReq.count;i++){
            PerfWatch perfWatch("genExportCoupon");//约2ms每条

            unsigned int insert_id = 0;
            insert_id = queryHelper.runInsert(string("")
                +" insert into " + coupon_table_prefix + to_string(batchExportReq.coupon_group_id)
                +" (coupon_group_id,create_time) "
                +"values("
                +to_string(batchExportReq.coupon_group_id)+","
				+to_string(couponGroup.create_time)
                +")"
            );

            if(insert_id <= 0){
                LOG_ERROR<<"insert_id = "<<insert_id<<" error occured when mysql execute genExportCoupon.."<< endl;
                LOG_ERROR<<"the error detail is that: "<<queryHelper.getError()<<endl;
                io.fault = Error::MYSQL_EXECUTE_ERROR;
                io.why = "error occured when mysql execute genExportCoupon " + queryHelper.getError();
                throw io;
            }

            unsigned int coupon_id = insert_id;

            //修改券码code
            string code = "";
            int err = CouponCode::Encode(batchExportReq.coupon_group_id,coupon_id,code);
            if(err != CouponCode::Errno::OK)
            {
                LOG_ERROR<<"coupon_group_id "<<batchExportReq.coupon_group_id<<" coupon_id = "<<coupon_id<<" code encode error"<< endl;
                io.fault = Error::CODE_ENCODE_ERROR;
                io.why = "code encode error";
                throw io;
            }

            unsigned int _coupon_group_id, _coupon_id;
            CouponCode::Decode(code,_coupon_group_id,_coupon_id);
            if((int)_coupon_group_id != batchExportReq.coupon_group_id || _coupon_id != coupon_id)
            {
                LOG_ERROR<<"coupon_group_id"<<batchExportReq.coupon_group_id<<"coupon_id="<<coupon_id<<" code decode error"<< endl;
                io.fault = Error::CODE_DECODE_ERROR;
                io.why = "code decode error";
                throw io;
            }

            int ret = queryHelper.runUpdate(string("")
                +" update "+coupon_table_prefix + to_string(batchExportReq.coupon_group_id)
                +" set code = '" + code +"'"
                +" where id = "+to_string(coupon_id)
            );

            if(ret <= 0){
                LOG_ERROR<<"ret= "<<ret<<" coupon_id="<<coupon_id<<" error occured when mysql execute.."<< endl;
                LOG_ERROR<<"the error detail is that: "<<queryHelper.getError()<<endl;
                io.fault = Error::MYSQL_EXECUTE_ERROR;
                io.why = "error occured when mysql execute " + queryHelper.getError();
                throw io;
            }
        }
    }catch(InvalidOperation &io){
        LOG_ERROR << "batchExportReq.coupon_group_id:"<<batchExportReq.coupon_group_id<<" batchExportReq.count:" <<batchExportReq.count
                  << " InvalidOperation exception caught: " <<" fault: "<< io.fault<<" why: "<<io.why << endl;
    }catch (TException& tx) {
        LOG_ERROR << "batchExportReq.coupon_group_id:"<<batchExportReq.coupon_group_id<<" batchExportReq.count:" <<batchExportReq.count
                  << " ERROR: " << tx.what() << endl;
    }catch (std::exception& e){
        LOG_ERROR << "batchExportReq.coupon_group_id:"<<batchExportReq.coupon_group_id<<" batchExportReq.count:" <<batchExportReq.count
                  << " exception caught: " << e.what() << endl;
    }catch (...){
        LOG_ERROR << "batchExportReq.coupon_group_id:"<<batchExportReq.coupon_group_id<<" batchExportReq.count:" <<batchExportReq.count
                  << " exception caught..." << endl;
    }
}

int main(int argc, char *argv[]){
    if (2 != argc){
        PrintHelp(argv[0]);
        return -1;
    }

    couponConfig = Singleton<CouponConfig>::instance();
    couponConfig.Parse(string(argv[1]));
    couponConfig.Print();

    string redis_ip    = couponConfig.getRedisIp();
    int redis_port     = couponConfig.getRedisPort();

    int pos = string(argv[0]).find_last_of('/');
    string program = string(argv[0]).substr(pos+1);
    GLogHelper gh(program,couponConfig.getLogPath());

    cout << "coupon batch export server running..." << endl;
    LOG_INFO<<"Starting the coupon batch export server..."<<endl;
    LOG_INFO<<"this->redis_ip_: "<<redis_ip<<" this->redis_port_:"<<redis_port<<endl;

    IgnoreSignal();
    //DaemonInit();

    struct rmq_context pop;
    rmq_init(&pop, redis_ip.c_str(), redis_port, 0, COUPON_BATCH_EXPORT_LIST);
    rmq_blpop(&pop, blpop_cb);
    ev_loop(EV_DEFAULT_ 0);

    cout << "Done." << endl;
    return 0;
}
