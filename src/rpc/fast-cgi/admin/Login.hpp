#ifndef _LOGIN_HPP_
#define _LOGIN_HPP_

#include "./Command.h"

class Login : public Command {
public:
    virtual void initialize(){}
    virtual int  check(){return 0;}
    virtual void destroy(){
        LOG_ERROR<<"Login destroy!"<<endl;
    }

    virtual void service(){
        webpage page;

        if(m_param["username"]!="admin" || m_param["password"]!="admin")
        {
            page.load("/data/deploy/etc/fast-cgi/html/login.html");
            page.set("errmsg","username or password error!");
            page.output();
            return;
        }

        page.load("/data/deploy/etc/fast-cgi/html/index.html");
        page.set("prompt","Welcome to index page!");
        page.set_bloc("INDEX");
        page.output();
        return;
    }
};

#endif
