#ifndef _CMDHANDLER_H_
#define _CMDHANDLER_H_

#include <unistd.h>
#include <fcgi_stdio.h>
#include "except_proc.h"
#include "ThriftHelper.h"
#include "./Command.h"
#include "./gen_cpp/httpCmd_types.h"

using namespace std;
using namespace HttpCmd;

#define FastCGI_FRAME   CmdHandler::Instance()
#define FastCGI_MAX_LENGTH  (4*1024*1024)

class CmdHandler {
public:
    //本FastCGI的命令列表
    map<string, Command*>  cmds_;  
    //接收到的信息            
    char buffer_[FastCGI_MAX_LENGTH];
    
private:
    //处理请求
    void handlePotocol(string& response, const string& request);
    
public:
    //由main调用
    int runAll(string& content);
    
    //返回单例
    static CmdHandler* Instance(){
        static CmdHandler* instance = NULL;
        if(NULL == instance){
            CmdHandler* tmp = new CmdHandler;
            __sync_synchronize(); 
            instance = tmp;
        }
        
        return instance;
    }

    //销毁命令
    void DestroyModuleList(){
        map<string, Command*>::iterator itm = cmds_.begin();
        for(; itm != cmds_.end(); itm++){
            if(itm->second != NULL){
                itm->second->destroy();
                delete itm->second;
                itm->second = NULL;
            }
        }
        cmds_.clear();
    }
    
    //注册命令
    template<class T>
    Command* RegisterModule(const string& name){
        if (cmds_.find(name) == cmds_.end()){
            T* t = new T;
            t->initialize();
            cmds_[name] = (Command*)t;
        }
        return cmds_[name];
    }
};

//由main调用
int CmdHandler::runAll(string& content){
    int ret = 0;
    string msg = "";
    
    PROC_TRY_BEGIN

    //如果是GET请求
    string query_string = QUERY_STRING;
    //如果是POST请求
    if(string(HTTP_METHOD) == string("POST")){
        //内容长度
        char *plength = getenv("CONTENT_LENGTH");
        if (plength != NULL && atoi(plength) < FastCGI_MAX_LENGTH){
            //读入到buffer_
            fread(buffer_, atoi(plength), 1, stdin);  
            buffer_[atoi(plength)] = 0;
            query_string = string(buffer_, atoi(plength));
        }
    }

    //处理请求
    handlePotocol(content, query_string);

    PROC_TRY_END(msg, ret, -1, -1)
    return ret;
}

void CmdHandler::handlePotocol(string& response, const string& query_string){
    HttpRequest  httpRequest;
    HttpResponse  httpResponse;
    
    //判断query_string是否符合HttpRequest
    try{
        JSONToThrift(query_string,&httpRequest);
    }catch(...){
        LOG_ERROR<<"query_string:"<<query_string<<" query_string parse fail";
        httpResponse.error = STATUS::PARSE_FAIL;
        httpResponse.errmsg = "parse fail";
        response = ThriftToJSON(httpResponse);
        return;
    }
    
    //判断指令是否注册
    if(cmds_.find(httpRequest.command) == cmds_.end()){
        LOG_ERROR<<"httpRequest.command:"<<httpRequest.command<<" no command";
        httpResponse.error = STATUS::NO_COMMAND;
        httpResponse.errmsg = "no command";
        response = ThriftToJSON(httpResponse);
        return;
    }

    //判断请求数据是否合法
    cmds_[httpRequest.command]->check(httpResponse, httpRequest);
    if(httpResponse.error != STATUS::SUCCESS){
        LOG_ERROR<<"httpRequest.command:"<<httpRequest.command<<" check fail";
        response = ThriftToJSON(httpResponse);
        return;
    }

    //处理请求
    try{
        cmds_[httpRequest.command]->service(response, query_string);
    }catch(...){
        LOG_ERROR<<"httpRequest.command:"<<httpRequest.command<<" service exception";
        httpResponse.error = STATUS::EXCEPTION;
        httpResponse.errmsg = "service exception";
        response = ThriftToJSON(httpResponse);
        return;
    }

    //判断response是否符合HttpResponse
    try{
        JSONToThrift(response,&httpResponse);
    }catch(...){
        LOG_ERROR<<"response:"<<response<<" response parse fail";
        httpResponse.error = STATUS::PARSE_FAIL;
        httpResponse.errmsg = "response parse fail";
        response = ThriftToJSON(httpResponse);
        return;
    }

    return;
}

#endif
