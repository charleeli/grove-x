#ifndef _WEB_PARAM_H_
#define _WEB_PARAM_H_

#include <string>
#include <map>
#include <iostream>
#include "webdef.h"

using namespace std;

class webparam
{
public:
    webparam(); 

    const map<string, string>& getparam() const
    {
        return m_Params;
    }

    const map<string, string>& getcookie() const
    {
        return m_cookie;
    }

    const map<string, string>& getenv() const
    {
        return m_env;
    }

    const string& get_cont() const {return m_strContent;}

    const string& get_cookie() const {return m_strCookies;}

    std::string& ParamFilter(const char * context, std::string & ret_value, bool is_html = true);


    
protected:
    inline string GetRequestMethod();
    
    void GetCgiValue();
    
    void ParseParams();

    void ParseCookies();

    void ParseEnv();

    string UrlDecode(const string strSrc);

    int HexToInt(char ch);

protected:
    string                  m_strContent;
    string                  m_strCookies;
    map<string, string>     m_Params;
    map<string, string>     m_cookie;
    map<string, string>     m_env;
};


#endif

