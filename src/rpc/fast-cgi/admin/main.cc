#include "fcgi_config.h"
#include "fcgiapp.h"
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>  
#include <error.h>
#include <stdio.h>   
#include <cassert>
#include <time.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
extern char **environ;
FCGX_Stream *fcgi_in, *fcgi_out, *fcgi_err;
FCGX_ParamArray fcgi_envp;
#include "weball.h"
#include "GLogHelper.h"
#include "./CmdHandler.h"

//包含命令处理头文件
#include "Login.hpp"

//注册命令
int RegisterModuleList(){
    FastCGI_FRAME->RegisterModule<Login>("Login");
    
    return 0;
}

static void OnExitSignal(int){
    FastCGI_FRAME->DestroyModuleList();
    LOG_ERROR<<"admin_module exit!"<<endl;
    exit(-1);
}

void IgnoreSignal(){
    signal(SIGHUP,SIG_IGN);
    signal(SIGPIPE,SIG_IGN);
    signal(SIGCHLD,SIG_IGN);
    signal(SIGINT,OnExitSignal);
    signal(SIGQUIT,OnExitSignal);
    signal(SIGTERM,OnExitSignal);
}

int main(int argc, char ** argv){
    //初始化日志
    GLogHelper gh("admin_module","/data/deploy/logs/");
    IgnoreSignal();
    //注册命令
    RegisterModuleList();
    LOG_INFO<<"admin_module running!";

    while (FCGX_Accept(&fcgi_in, &fcgi_out, &fcgi_err, &fcgi_envp) >= 0) {
        char *contentLength = FCGX_GetParam("CONTENT_LENGTH", fcgi_envp);
        int len = 0;

        if (contentLength != NULL)
            len = strtol(contentLength, NULL, 10);

        if (len <= 0) {
            FCGX_FPrintF(fcgi_out, "No data from standard input.<p>\n");
        }else {
            int i, ch;

            FCGX_FPrintF(fcgi_out, "Standard input:<br>\n<pre>\n");
            for (i = 0; i < len; i++) {
                if ((ch = FCGX_GetChar(fcgi_in)) < 0) {
                    FCGX_FPrintF(fcgi_out,
                        "Error: Not enough bytes received on standard input<p>\n");
                    break;
                }
                FCGX_PutChar(ch, fcgi_out);
            }
            FCGX_FPrintF(fcgi_out, "\n</pre><p>\n");
        }

        FastCGI_FRAME->runAll();
    } /* while */

    //删除命令
    FastCGI_FRAME->DestroyModuleList();
    LOG_INFO<<"admin_module destroyed!";
    return 0;
}
