/*
 * author:charlee
 */
#include <mcheck.h>
#include <iostream>
#include "System.h"
#include "GLogHelper.h"
#include "CouponConfig.h"
#include "ThriftServer.h"
#include "Singleton.h"

CouponConfig couponConfig;
#include "CouponECServiceHandler.hpp"

using namespace System;

void PrintHelp(const char* program_name){
    std::cout << "Usage:" << program_name << " conf_path." << std::endl;
}

int main(int argc, char ** argv){
    if (2 != argc){
        PrintHelp(argv[0]);
        return -1;
    }

    //assert(!setenv("MALLOC_TRACE","./mtrace_ec.log",1)) ;
    //mtrace();

    couponConfig = Singleton<CouponConfig>::instance();
    couponConfig.Parse(string(argv[1]));
    couponConfig.Print();

    int pos = string(argv[0]).find_last_of('/');
    string program = string(argv[0]).substr(pos+1);
    GLogHelper gh(program,couponConfig.getLogPath());

    LOG_INFO<<"Starting the coupon ec server..."<<endl;

    int port = couponConfig.getECPort();
    unsigned int ioThreadsNum = couponConfig.getECIoThreadsNum();
    unsigned int taskThreadsNum = couponConfig.getECTaskThreadsNum();
    using ServiceProcessorFactory = CouponECServiceProcessorFactory;
    using ServiceCloneFactory = CouponECServiceCloneFactory;

    ThriftServer<ServiceProcessorFactory,ServiceCloneFactory>::instance().Initialize(argc, argv);
    ThriftServer<ServiceProcessorFactory,ServiceCloneFactory>::instance().Start(port,ioThreadsNum,taskThreadsNum);

    cout << "Done." << endl;
    return 0;
}
