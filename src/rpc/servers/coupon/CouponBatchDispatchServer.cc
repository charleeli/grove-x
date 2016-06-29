#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <ev.h>
#include <iostream>
#include <exception>
#include "System.h"
#include "GLogHelper.h"
#include "CouponConfig.h"
#include "CouponConst.h"
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
    LOG_ERROR<<"coupon_batch_dispatch_server exit!"<<endl;
    cout<<"coupon_batch_dispatch_server exit!"<<endl;
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
    BatchDispatchReq batchDispatchReq;
    int64_t temp_user_id = 0;

    try{
        ThriftClient<CouponECServiceClient> thriftClient("localhost", couponConfig.getECPort());
        JSONToThrift(string(msg),&batchDispatchReq);

        for(auto user_id : batchDispatchReq.user_id_list){
            PerfWatch perfWatch("drawCoupon");

            DrawReq drawReq;
            drawReq.coupon_group_id = batchDispatchReq.coupon_group_id;
            drawReq.user_id = user_id;
            drawReq.client_id = "";
            drawReq.argot = "";
            drawReq.scene_type = 4;

            temp_user_id = user_id;

            LOG_WARN<<"drawCoupon request is: DrawReq '"<<ThriftToJSON(drawReq)<<"'"<<endl;

            DrawRsp drawRsp;
            thriftClient.getClient()->drawCoupon(drawRsp, drawReq);
            if(coupon::Error::OK != drawRsp.error){
                LOG_ERROR << "coupon_group_id:"<<batchDispatchReq.coupon_group_id<<" user_id:" <<user_id
                          <<" drawReq.scene_type:"<<drawReq.scene_type <<" error: "<< drawRsp.error<<" errmsg: "
                          <<drawRsp.errmsg << endl;
            }

            LOG_WARN<<"drawCoupon response is: DrawRsp '"<<ThriftToJSON(drawRsp)<<"'"<<endl;
        }

    }catch(InvalidOperation &io){
        LOG_ERROR << "coupon_group_id:"<<batchDispatchReq.coupon_group_id<<"user_id:" <<temp_user_id
                  << " InvalidOperation exception caught: " <<" fault: "<< io.fault<<" why: "<<io.why << endl;
    }catch (TException& tx) {
        LOG_ERROR << "coupon_group_id:"<<batchDispatchReq.coupon_group_id<<"user_id:" <<temp_user_id
                  << " ERROR: " << tx.what() << endl;
    }catch (std::exception& e){
        LOG_ERROR << "coupon_group_id:"<<batchDispatchReq.coupon_group_id<<"user_id:" <<temp_user_id
                  << " exception caught: " << e.what() << endl;
    }catch (...){
        LOG_ERROR << "coupon_group_id:"<<batchDispatchReq.coupon_group_id<<"user_id:" <<temp_user_id
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

    cout << "coupon batch dispatch server running..." << endl;
    LOG_INFO<<"Starting the coupon batch dispatch server..."<<endl;
    LOG_INFO<<"this->redis_ip_: "<<redis_ip<<" this->redis_port_:"<<redis_port<<endl;

    IgnoreSignal();
    //DaemonInit();

    struct rmq_context pop;
    rmq_init(&pop, redis_ip.c_str(), redis_port, 0, COUPON_BATCH_DISPATCH_LIST);
    rmq_blpop(&pop, blpop_cb);
    ev_loop(EV_DEFAULT_ 0);

    cout << "Done." << endl;
    return 0;
}
