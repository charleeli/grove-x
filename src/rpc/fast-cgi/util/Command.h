#ifndef _COMMAND_H_
#define _COMMAND_H_

#include "./gen_cpp/httpCmd_types.h"

using namespace std;
using namespace HttpCmd;

class Command{
public:
    inline Command() {};
    virtual ~Command() {};

    virtual void initialize(){}
    virtual void check(HttpResponse& httpResponse, const HttpRequest& httpRequest){}
    virtual void service(string& response, const string& request) = 0;
    virtual void destroy(){}
};

#endif
