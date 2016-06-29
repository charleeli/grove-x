#ifndef _WEB_COMMON_H_
#define _WEB_COMMON_H_

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <sys/wait.h>
#include <netdb.h>

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <algorithm>

using namespace std;

#define FLASH_QB_NOT_ENOUGH 100

#ifndef SIZE_K
#define SIZE_K(size) ((size) * 1024)
#endif

#define ALIGN(size, boundary) (((size)+((boundary)-1))&~((boundary)-1))

#define ALIGN_8(size) ALIGN(size, 8)

#define CSL_MAX(a,b) ((a)>(b)?(a):(b))
#define CSL_MIN(a,b) ((a)<(b)?(a):(b))

#include "fcgi_config.h"
#include "fcgiapp.h"
#include "fcgio.h"
const int DEFAULT_MAX_SERIAL_NO = 10000;
const int NO_LIMIT_SERIAL_NO = 0;
extern   FCGX_Stream *fcgi_in, *fcgi_out, *fcgi_err;
extern   FCGX_ParamArray fcgi_envp;

#endif
