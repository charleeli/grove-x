#include "webgeneral.h"
#include "string.h"

bool isDigitStr(const string str)
{
    if( 0 == str.length() ) return false;       

    for(string::size_type i =0; i < str.size(); i++)
        if(!isdigit(str[i]))
            return false;

    return true;
}

inline void timeValAdd(struct timeval *tv_io, long ms)
{
    struct timeval tmp;
    assert(ms>=0);
    tmp = *tv_io;
    tmp.tv_usec += (ms%1000)*1000;
    tmp.tv_sec += ms/1000;

    if (tmp.tv_usec >= 1000000)
    {
        tmp.tv_sec++;
        tmp.tv_usec -= 1000000;
    }

    *tv_io = tmp;
}

unsigned int hexStrToUint(string sHex)
{
    int val=0;
    unsigned int result=0;

    for(unsigned int i = 0; i < sHex.length(); i++)
    {
        val = sHex[i] - '0';
        if(val >= 0 && val < 10)
        {
            result += val << (sHex.length() - i - 1) * 4;
            continue;
        }

        val = toupper(sHex[i]) - 'A';
        if(val >= 0 && val < 6)
            result += (val + 10) << (sHex.length() - i - 1) * 4;
    }

    return result;
}

string strFormat(const char *sFormat, ...)
{
    if ( sFormat == NULL ) return "";

    va_list ap;
    va_start( ap, sFormat );

    int bufSize = vsnprintf(0, 0, sFormat, ap) + 1;

    char *sBuffer = (char *)malloc(bufSize);

    memset(sBuffer, 0, bufSize);

    vsnprintf( sBuffer, bufSize, sFormat, ap );

    va_end( ap );

    string tmp(sBuffer);
    free(sBuffer);

    return tmp;
}

string strReplace(string strSrc, string sub, string rpas)
{
    string::size_type iEndPos = 0, iStartPos = 0;
    while(1)
    {
        iEndPos = strSrc.find(sub, iStartPos);
        if(iEndPos == string::npos)
            break;

        if(iEndPos > iStartPos)
            strSrc.replace(iEndPos, sub.length(), rpas);

        iStartPos = iEndPos + sub.length();
    }

    return strSrc;
}

vector<string> strSplit(string strSrc, string strMark)
{
    vector<string> vSplitStr;

    if(0 == strSrc.length()) return vSplitStr;

    //  if strMark is empty, return strSrc
    if(0 == strMark.length())
    {
        vSplitStr.push_back(strSrc);
        return vSplitStr;
    }

    string::size_type iEndPos = 0, iStartPos = 0;
    while(1)
    {
        iEndPos = strSrc.find(strMark, iStartPos);
        if(iEndPos == string::npos)
        {
            vSplitStr.push_back(strSrc.substr(iStartPos, strSrc.length() - iStartPos));
            return vSplitStr;
        }

        if(iEndPos > iStartPos)
            vSplitStr.push_back(strSrc.substr(iStartPos, iEndPos - iStartPos));

        iStartPos = iEndPos + strMark.length();
    }
}

void mSleep(unsigned int nMilliSecond)
{
    if(nMilliSecond <= 0)
        return;

    struct timeval      time_val;

    time_val.tv_sec = nMilliSecond / 1000;
    time_val.tv_usec = nMilliSecond % 1000 * 1000;

    select(0, NULL, NULL, NULL, &time_val);
}

string htmlFilter(string sText, int iFlag)
{
    string::size_type pos = 0;
    while((pos = sText.find_first_of('<',pos)) !=string::npos)
    {
        sText.replace(pos,1,"&lt;");
    }

    pos = 0;
    while((pos = sText.find_first_of('>',pos)) != string::npos)
    {
        sText.replace(pos,1,"&gt;");
    }

    if(0 == iFlag)
    {
        pos = 0;
        while((pos = sText.find_first_of("\r",pos)) != string::npos)
        {
            sText.replace(pos,1,"");
        }
        pos = 0;
        while((pos = sText.find_first_of("\n",pos)) != string::npos)
        {
            sText.replace(pos,1,"<br>");
        }

        pos = 0;
        while((pos = sText.find_first_of(' ',pos)) != string::npos)
        {
            sText.replace(pos,1,"&nbsp;");
        }

        pos = 0;
        while((pos = sText.find_first_of('\t',pos)) != string::npos)
        {
            sText.replace(pos,1,"&nbsp;&nbsp;&nbsp;");
        }
    }

    return sText;
}

void setCookie(string name, string value, string expires, string path,
        string domain, short secure)
{
    if(0 == name.length())
    {
        return;
    }

    cout << "Set-Cookie: " << name << "="<< value << ";";

    if(expires.length() != 0)
        cout << " EXPIRES="<< expires << ";";

    if (path.length() != 0)
        cout << " PATH="<< path <<";";

    if (domain.length() != 0)
        cout << " DOMAIN="<< domain <<";";

    if (secure)
        printf(" SECURE");

    cout << endl;
}

void doRedirect(string sURL)
{
    if( sURL.length() == 0 ) return;

    time_t ltime;
    time(&ltime);

    string::size_type iEndPos = 0;
    iEndPos = sURL.find( "?", 0 );
    if(iEndPos == string::npos)
        sURL = sURL + "?";
    else
        sURL = sURL + "&";

    cout << "Content-Type:text/html\n\n";
    cout << "<script language=javascript>\n";
    cout << "window.location.href='"<< sURL << "localtime="<< ltime << "';\n";
    cout << "</script>" << endl;
}


unsigned int g_random_seedp = 0;

int petRandom(int iMax )
{
    struct timeval tpstime;
    float fLocalMax = iMax;

    if (iMax <= 0)
    {
        return 0;
    }

    if(0 == g_random_seedp)
    {
        gettimeofday(&tpstime,NULL);
        g_random_seedp =(unsigned)(tpstime.tv_usec+getpid());

    }

    int iRandomResult = (int) (fLocalMax * (rand_r(&g_random_seedp)/(RAND_MAX + 1.0)));
    return iRandomResult;
}
