#include "webparam.h"
#include <stdio.h>
#include <string.h>

extern char **environ;

#define MAX_CONTENT_LENGTH      8*1024
#define POST_MAX_CONTENT_LENGTH 1024*1024

webparam::webparam()
{
    ParseEnv();
    GetCgiValue();
    ParseParams();
    ParseCookies();
}

void webparam::ParseEnv()
{
    char** envp = fcgi_envp;

    char* p = NULL;
    char* src = NULL;
    for( ; *envp != NULL; envp++)
    {
        src = strdup(*envp);
        if(src != NULL)
        {
            p = strstr(src, "=");
            if(p!=NULL)
            {
                *p = '\0';
                ++p;
                m_env[src] = p;
            }
            free(src);
            src = NULL;
        }
    }
}

/*
 *  PSRPOSE: convert hex char to int
 */
int webparam::HexToInt(char ch)
{
    if(ch >= '0' && ch <= '9')return ch - '0';
    else return toupper(ch) - 'A' + 10;
}

/*
 *  PURPOSE: decode url param
 */
string webparam::UrlDecode(const string strSrc)
{
    string strDest;
    int iSrcLength = strSrc.length();
    if (iSrcLength <= 0) return "";

    char ch;
    for (int i = 0; i < iSrcLength; i++)
    {
        switch (strSrc[i]) 
        {
            case '%':
                if(isxdigit(strSrc[i+1]) && isxdigit(strSrc[i+2]))
                {   
                    ch = (HexToInt(strSrc[i+1]) << 4) + HexToInt(strSrc[i+2]);
                    i = i + 2;                  
                }
                else 
                    ch = strSrc[i];

                strDest += ch;
                break;
            case '+':
                ch = ' ';
                strDest += ch;
                break;
            default: 
                strDest += strSrc[i];
                break;
        }
    }

    return strDest;
}

/*
 *  PURPOSE: to get url and decode it
 */
void webparam::GetCgiValue()
{
    int iLength = 0;    
    if(GetRequestMethod() == "POST")        //  post method
    {
        if(iLength < 0 || iLength >= POST_MAX_CONTENT_LENGTH)
            return;                         //  error

        char sBuffer[POST_MAX_CONTENT_LENGTH] = {0};
        FCGX_GetStr(sBuffer,POST_MAX_CONTENT_LENGTH-1, fcgi_in);
        m_strContent = sBuffer;
    }
    else                                    //  get method
    {
        m_strContent = m_env["QUERY_STRING"];
    }

    m_strCookies = m_env["HTTP_COOKIE"];
    m_strCookies = UrlDecode(m_strCookies);

    return;
}

/*
 *  PURPOSE: to analyse Cookie and save to map
 */
void webparam::ParseCookies(void)
{
    int iLength = m_strCookies.length();
    if(iLength <= 0 || iLength >= MAX_CONTENT_LENGTH) return;

    int iStat = 0;
    string name, value;
    int i = 0;
    for(i = 0; i < iLength; i++)
    {
        if(-1 == iStat)break;                   //  end stat
        switch(iStat)
        {
            case 0:
                switch(m_strCookies[i])
                {
                    case ' ':iStat = 0;break;
                    case '=':return;        //  error
                    case '\0':iStat = -1;break;
                    default:iStat = 1;name += m_strCookies[i];break;
                }
                break;
            case 1:
                switch(m_strCookies[i])
                {
                    case ' ':iStat = 1;break;
                    case '=':iStat = 2;break;
                    case '\0':return;       //  error
                    default:iStat = 1; name += m_strCookies[i];break;
                }
                break;
            case 2:
                switch(m_strCookies[i])
                {
                    case ';':iStat = 3;break;
                    case '\0':iStat = -1; m_cookie[name] = value;break;
                    default:iStat = 2; value += m_strCookies[i];break;
                }
                break;
            case 3:
                switch(m_strCookies[i])
                {
                    case ' ':iStat = 0;m_cookie[name] = value; name.erase();value.erase();break;
                    case '\0':iStat = -1; m_cookie[name] = value;break;
                    default:iStat = 2;value += ';';value += m_strCookies[i];break;
                }
                break;
        }
    }

    if(i == iLength)m_cookie[name] = value;
}

/*
 *  PURPOSE: to analyse url string and save to map
 */
void webparam::ParseParams()
{
    int iLength = m_strContent.length();
    if(iLength <= 0 || iLength >= POST_MAX_CONTENT_LENGTH) return;

    int iStat = 0;
    string name, value;
    int i = 0;
    for(i = 0; i < iLength; i++)
    {
        if(-1 == iStat)break;                       //  end stat
        switch(iStat)
        {
            case 0:
                switch(m_strContent[i])
                {
                    case ' ':iStat = 0;break;
                    case '&':iStat = 0;m_Params[name] = UrlDecode(value);name.erase();value.erase();break;
                    case '=':return;            //  error
                    case '\0':iStat = -1;break;
                    default:iStat = 1;name += m_strContent[i];break;
                }
                break;
            case 1:
                switch(m_strContent[i])
                {
                    case ' ':iStat = 1;break;
                    case '=':iStat = 2;break;
                    case '&':iStat = 0;m_Params[name] = UrlDecode(value);name.erase();value.erase();break;
                    case '\0':return;           //  error
                    default:iStat = 1;name += m_strContent[i];break;
                }
                break;
            case 2:
                switch(m_strContent[i])
                {
                    case '&':iStat = 0;m_Params[name] = UrlDecode(value);name.erase();value.erase();break;
                    case '\0':iStat = -1;m_Params[name] = value;break;
                    default:iStat = 2;value += m_strContent[i];break;
                }
                break;
        }
    }

    if(i == iLength)
    {
        m_Params[name] = UrlDecode(value);
    }

    map<string, string>::iterator it;
    for(it = m_Params.begin(); it != m_Params.end(); ++it)
    {
        string ret_str;
        it->second = ParamFilter(it->second.c_str(), ret_str, false);
    }

}


string webparam::GetRequestMethod()
{
    return m_env["REQUEST_METHOD"];
}

std::string& webparam::ParamFilter(const char * context, std::string & ret_value, bool is_html)
{
    int context_len = strlen(context);

    for (int i = 0; i < context_len; ++i)
    {
        switch (context[i])
        {
        case '<':
            ret_value += (is_html) ? "&lt;" : "*";
            break;
        case '>':
            ret_value += (is_html) ? "&gt;" : "*";
            break;
        case ':':
            ret_value += (is_html) ? "&#58;" : "*";
            break;
        case '"':
            ret_value += (is_html) ? "&quot;" : "*";
            break;
        case '\'':
            ret_value += (is_html) ? "&#39;" : "*";
            break;
        default:
            ret_value += context[i];
            break;
        }
    }

    return ret_value;
}

