#ifndef _THRIFT_SERVER_H_
#define _THRIFT_SERVER_H_

#include <signal.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/TToString.h>
#include <thrift/Thrift.h>
#include <thrift/server/TNonblockingServer.h>
#include <boost/make_shared.hpp>
#include <stdexcept>
#include <iostream>
#include <pthread.h>

using namespace std;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;
using namespace System;

template <typename ServiceProcessorFactory,typename ServiceCloneFactory>
class ThriftServer {
private:
    std::string pname_;
    unsigned int port;
    unsigned int ioThreadsNum;
    unsigned int taskThreadsNum;
    boost::shared_ptr<ThreadManager> threadManager;
    boost::shared_ptr<TNonblockingServer> server;

private:
    ThriftServer(){
        this->pname_ = "";
        this->port = 9090;
        this->ioThreadsNum = 1;
        this->taskThreadsNum = 1;
    }

    ~ThriftServer(){

    }

public:
    static ThriftServer& instance(){
        static ThriftServer inst;
        return inst;
    }

    int Initialize(int argc, char * argv[]){
        assert(2 == argc);
        //DaemonInit();

        IgnoreSignal();

        ReadProgramName(argv[0]);
        cout << this->pname_ << " running..." << endl;
        return 0;
    }

    bool Start(unsigned int port,unsigned int ioThreadsNum,unsigned int taskThreadsNum){
        this->port = port;
        this->ioThreadsNum = ioThreadsNum;
        this->taskThreadsNum = taskThreadsNum;

        this->threadManager = ThreadManager::newSimpleThreadManager(this-> taskThreadsNum);
        this->threadManager->threadFactory(boost::shared_ptr<PosixThreadFactory>(new PosixThreadFactory()));
        this->threadManager->start();

        this->server = boost::shared_ptr<TNonblockingServer>(new TNonblockingServer(
                boost::make_shared<ServiceProcessorFactory>(boost::make_shared<ServiceCloneFactory>()),
                boost::shared_ptr<TProtocolFactory>(new TBinaryProtocolFactory()),
                this->port,
                this->threadManager));

        this->server->setNumIOThreads(this -> ioThreadsNum);

        try{
            this->server->serve();
        }catch(TException &e){
            printf("Server.serve() failed\n e:%s",e.what());
            exit(-1);
        }

        return true;
    }

private:
    void IgnoreSignal(){
        signal(SIGHUP,SIG_IGN);
        signal(SIGPIPE,SIG_IGN);
        signal(SIGCHLD,SIG_IGN);

        signal(SIGINT,OnExitSignal);
        signal(SIGQUIT,OnExitSignal);
        signal(SIGTERM,OnExitSignal);
    }

    static void OnExitSignal(int){
        ThriftServer::instance().Stop();
    }

    void Stop(){
        LOG_ERROR<<pname_<<" is stoping, wait 2 seconds..."<<endl;
        this->server->stop();
        sleep(2);
        LOG_ERROR<<pname_<<" terminated gracefully!"<<endl;
    }

    void ReadProgramName(const char* arg0){
        std::string tmp_path(arg0);
        this->pname_ = tmp_path.substr(tmp_path.find_last_of('/') + 1);
    }
};

#endif
