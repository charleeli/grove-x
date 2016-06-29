#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>  
#include <error.h>
#include <stdio.h>   
#include <cassert>
#include <time.h>
#include "GLogHelper.h"
#include "util/CmdHandler.h"

//包含命令处理头文件
#include "Echo.hpp"

//注册命令
int RegisterModuleList(){
    FastCGI_FRAME->RegisterModule<Echo>("Echo");
    
    return 0;
}

static void OnExitSignal(int){
    FastCGI_FRAME->DestroyModuleList();
    LOG_ERROR<<"echo_module exit!"<<endl;
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

int main(int argc, char ** argv){
    //初始化日志
    GLogHelper gh("echo_module","/data/deploy/logs/");
    IgnoreSignal();
    //注册命令
    RegisterModuleList();
    LOG_INFO<<"echo_module running!";
    //处理请求
    try{
        while (FCGI_Accept() >= 0){
            string content;
            FastCGI_FRAME->runAll(content);
            printf( "Content-type:application/json\r\n\r\n" );
            printf("%s\n", content.c_str());
        }
    }catch (std::exception& e){
        LOG_ERROR<< "exception caught: " << e.what();
    }catch (...){
        LOG_ERROR<< "exception caught";
    }

    //删除命令
    FastCGI_FRAME->DestroyModuleList();
    LOG_INFO<<"echo_module destroyed!";
    return 0;
}
