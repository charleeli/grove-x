/*
 * author:charlee
 */

#include <iostream>
#include "System.h"
#include "GLogHelper.h"
#include "CouponConfig.h"
#include "ThriftServer.h"
#include "System.h"

CouponConfig couponConfig;
#include "CouponPushServiceHandler.hpp"

using namespace System;

void PrintHelp(const char* program_name){
    std::cout << "Usage:" << program_name << " conf_path." << std::endl;
}

int main(int argc, char ** argv){
    if (2 != argc){
        PrintHelp(argv[0]);
        return -1;
    }

    couponConfig = Singleton<CouponConfig>::instance();
    couponConfig.Parse(string(argv[1]));
    couponConfig.Print();

    int pos = string(argv[0]).find_last_of('/');
    string program = string(argv[0]).substr(pos+1);
    GLogHelper gh(program,couponConfig.getLogPath());

    LOG_INFO<<"Starting the coupon push server..."<<endl;

    int port = couponConfig.getPushPort();
    unsigned int ioThreadsNum = couponConfig.getPushIoThreadsNum();
    unsigned int taskThreadsNum = couponConfig.getPushTaskThreadsNum();
    using ServiceProcessorFactory = CouponPushServiceProcessorFactory;
    using ServiceCloneFactory = CouponPushServiceCloneFactory;

    ThriftServer<ServiceProcessorFactory,ServiceCloneFactory>::instance().Initialize(argc, argv);
    ThriftServer<ServiceProcessorFactory,ServiceCloneFactory>::instance().Start(port,ioThreadsNum,taskThreadsNum);

    cout << "Done." << endl;
    return 0;
}
