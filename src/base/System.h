#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <string>
#include <assert.h>
#include <sys/stat.h>
#include <math.h>
#include "ctime.h"
#include "GLogHelper.h"

using namespace std;

namespace System {

double Round(double num,int precision = 2){
    if(num < 0.0){
        return int(num * pow(10,precision) - 0.5)/(pow(10,precision) + 0.0);
    }

    return int(num * pow(10,precision) + 0.5)/(pow(10,precision) + 0.0);
}

int NumCPU(){
    return static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
}

void DaemonInit(){
    pid_t pid = fork();
    if(pid < 0){
        assert(false);
    }else if(pid > 0){
        exit(0);
    }

    setsid();
    umask(0);
}

class PerfWatch{
    double timestamp;
    string service;

public:
    PerfWatch(string service){
        this->timestamp = ctime::microtime();
        this -> service = service;
    }

    ~PerfWatch(){
        LOG_WARN<<this->service<<" cost "<<(ctime::microtime() - this->timestamp)/1000.0<<" ms"<<endl;
    }
};

}

#endif
