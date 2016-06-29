/*
 * author:charlee
 */

#include <iostream>
#include "System.h"
#include "GLogHelper.h"
#include "ThriftServer.h"
#include "Singleton.h"
#include "OnsaleConfig.hpp"
#include "OdbWrapperPool.h"

OnsaleConfig onsaleConfig;
#include "OnsaleECServiceHandler.hpp"

using namespace System;
using namespace std;
using namespace odb::core;

void PrintHelp(const char* program_name){
    std::cout << "Usage:" << program_name << " conf_path." << std::endl;
}

int main(int argc, char ** argv){
    if (2 != argc){
        PrintHelp(argv[0]);
        return -1;
    }

    onsaleConfig = Singleton<OnsaleConfig>::instance();
    onsaleConfig.Parse(string(argv[1]));
    onsaleConfig.Print();

    int pos = string(argv[0]).find_last_of('/');
    string program = string(argv[0]).substr(pos+1);
    GLogHelper gh(program,onsaleConfig.getLogPath());

    LOG_INFO<<"Starting the onsale ec server..."<<endl;

    int port = onsaleConfig.getECPort();
    unsigned int ioThreadsNum = onsaleConfig.getECIoThreadsNum();
    unsigned int taskThreadsNum = onsaleConfig.getECTaskThreadsNum();
    using ServiceProcessorFactory = OnsaleECServiceProcessorFactory;
    using ServiceCloneFactory = OnsaleECServiceCloneFactory;

    ThriftServer<ServiceProcessorFactory,ServiceCloneFactory>::instance().Initialize(argc, argv);
    ThriftServer<ServiceProcessorFactory,ServiceCloneFactory>::instance().Start(port,ioThreadsNum,taskThreadsNum);

    cout << "Done." << endl;
    return 0;
}
