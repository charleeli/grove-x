#ifndef _GENERAL_H_
#define _GENERAL_H_

#include "webdef.h"

#include <sys/time.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <stdarg.h>
#include <stdio.h>
#include <vector>

using namespace std;

bool isDigitStr(const string str);

void timeValAdd(struct timeval *tv_io, long ms);

unsigned int hexStrToUint(string sHex);

string strFormat(const char *sFormat, ...);

vector<string> strSplit(string strSrc, string strMark);

string strReplace(string strSrc, string sub, string rpas);

void mSleep(unsigned int  nMilliSecond);

string htmlFilter(string sText, int iFlag = 0);

void setCookie( string name, string value, 
            string expires, string path , 
            string domain, short secure = 0);

void doRedirect(string sURL);

int petRandom(int iMax );

#endif

