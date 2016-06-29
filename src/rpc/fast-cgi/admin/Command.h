#ifndef _COMMAND_H_
#define _COMMAND_H_

#include <map>
#include <string>

using namespace std;

class Command {
protected:
    map<string, string> m_param ;
    map<string, string> m_cookie ;
    map<string, string> m_env ;
    string m_param_content  ;
    string m_cookie_content ;

public:
    inline Command() {};
    virtual ~Command() {};
    void setEnvironment(
        map<string, string>&,
        map<string, string>&,
        map<string, string>&,
        string&,
        string&
    );

    virtual void initialize(){}
    virtual int  check(){return 0;}
    virtual void service() = 0;
    virtual void destroy(){}
};

void Command::setEnvironment(
    map<string, string> &param,
    map<string, string> &cookie,
    map<string, string> &env,
    string &param_content,
    string &cookie_content)
{
    m_param = param;
    m_cookie = cookie;
    m_env = env;
    m_param_content = param_content;
    m_cookie_content = cookie_content;
}

#endif
