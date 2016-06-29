#ifndef _ECHO_HPP_
#define _ECHO_HPP_

#include <unistd.h>
#include "System.h"
#include "util/Command.h"
#include "sdk-cpp/echo/echoCmd_types.h"

using namespace std;
using namespace System;
using namespace HttpCmd;
using namespace EchoCmd;

class Echo : public Command {
public:
    virtual void initialize(){}
    virtual void check(HttpResponse& httpResponse, const HttpRequest& httpRequest){}
    virtual void destroy(){
        LOG_ERROR<<"Echo destroy!"<<endl;
    }

    virtual void service(string& response, const string& request){
        PerfWatch perfWatch("Echo");
        EchoReq echoReq;
        EchoRsp echoRsp;

        JSONToThrift(request,&echoReq);
        LOG_WARN<<"echoReq '"<<ThriftToJSON(echoReq)<<"'";

        echoRsp.error = 0;
        echoRsp.errmsg = "success";
        echoRsp.foo = echoReq.foo;
        echoRsp.pid = getpid();

        response = ThriftToJSON(echoRsp);
        LOG_WARN<<"echoRsp '"<<ThriftToJSON(echoRsp)<<"'";
        return;
    }
};

#endif
