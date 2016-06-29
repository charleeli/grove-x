#ifndef _EXCEPT_PROC_H_
#define _EXCEPT_PROC_H_

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <fstream>
#include <string>
#include <inttypes.h>
#include <iostream>
#include <sstream>
#include <map>
#include <list>
#include <deque>
#include <vector>
#include <memory>
#include <netinet/in.h>
#include <errno.h>
#include <iomanip>
#include <sys/ipc.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

using namespace std;

#define GET_ENV(x)      (getenv(x) == NULL) ? "" : getenv(x)
#define HTTP_METHOD     GET_ENV("REQUEST_METHOD")
#define QUERY_STRING    GET_ENV("QUERY_STRING")
#define REMOTE_HOST     GET_ENV("REMOTE_HOST")
#define REMOTE_ADDR     GET_ENV("REMOTE_ADDR")
#define CONTENT_TYPE    GET_ENV("CONTENT_TYPE")
#define CONTENT_LENGTH  GET_ENV("CONTENT_LENGTH")
#define HTTP_USER_AGENT GET_ENV("HTTP_USER_AGENT")

namespace Grove
{
#ifndef PROC_BEGIN
#define PROC_BEGIN do{
#endif
#ifndef PROC_TRY_BEGIN	
#define PROC_TRY_BEGIN		do{             \
    try{
#endif
#ifndef PROC_END
#define PROC_END   }while(0);
#endif
#ifndef PROC_TRY_END	
#define PROC_TRY_END(msg, ret, errcode, defcode)    }   \
	catch(const std::exception& e) {                \
            (msg) = e.what();                           \
            (ret) = (errcode);                          \
	}                                               \
	catch(...) {                                    \
            (msg) = "unknown exception";                \
            (ret) = (defcode);                          \
	}                                               \
}while(0);
#endif
#ifndef PROC_EXIT
#define PROC_EXIT(ret, value) {(ret)=(value);break;};
#endif
#ifndef PROC_TRY_EXIT
#define PROC_TRY_EXIT(ret,retval,code,errcode,desc,errdesc) {   \
	(ret)=(retval);                                         \
	(code) = (errcode);                                     \
	(desc) = (errdesc);                                     \
	break;                                                  \
    };
#endif
#ifndef PROC_EQ_EXIT
#define PROC_EQ_EXIT(expr, eval, ret, value) {if( (expr) == (eval) ) { (ret) = (value); break; } };
#endif
#ifndef PROC_NE_EXIT
#define PROC_NE_EXIT(expr, eval, ret, value) {if( (expr) != (eval) ) { (ret) = (value); break; } };
#endif 
#ifndef TRY_STMT
#define TRY_STMT try{
#endif
#ifndef CATCH_STMT
#define CATCH_STMT(msg, ret, errcode, defcode) }        \
        catch(const std::exception& e) {                \
            (msg) = e.what();                           \
            (ret) = (errcode);                          \
        }                                               \
        catch(...) {                                    \
            (msg) = "unknown exception";                \
            (ret) = (defcode);                          \
        }
#endif

#ifndef offsetof
#define offsetof(s, e) ((size_t)&((s *)0)->e)
#endif

};
#endif

