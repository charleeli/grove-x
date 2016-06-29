#ifndef _CMDHANDLER_H_
#define _CMDHANDLER_H_

#include <unistd.h>
#include <fcgi_stdio.h>
#include "weball.h"
#include "./Command.h"

using namespace std;
#include <map>
#include <string>

#define FastCGI_FRAME   CmdHandler::Instance()

class CmdHandler {
public:
    //本FastCGI的命令列表
    std::map<string, Command*>  cmds_;         

public:
    //由main调用
    void runAll();
    
    //返回单例
    static CmdHandler* Instance(){
        static CmdHandler* instance = NULL;
        if (NULL == instance){
            CmdHandler* tmp = new CmdHandler;
            __sync_synchronize(); 
            instance = tmp;
        }
        
        return instance;
    }

    //销毁命令
    void DestroyModuleList() {
        map<string, Command*>::iterator itm = cmds_.begin();
        for (; itm != cmds_.end(); itm++){
            if (itm->second != NULL){
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
void CmdHandler::runAll(){
    webparam param;
    map<string, string> m_param = param.getparam();
    map<string, string> m_cookie = param.getcookie();
    map<string, string> m_env = param.getenv();
    string m_param_content = param.get_cont();
    string m_cookie_content = param.get_cookie();

    if (cmds_.find(m_param["command"]) == cmds_.end()){
        webpage page;
        page.load("/data/deploy/etc/fast-cgi/html/login.html");
        page.output();
        return;
    }

    cmds_[m_param["command"]]->setEnvironment(
        m_param,
        m_cookie,
        m_env,
        m_param_content,
        m_cookie_content
    );

    return cmds_[m_param["command"]]->service();
}

#endif
