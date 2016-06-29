/*
 * author:charlee
 */
#ifndef _CURRENTTHREAD_H
#define _CURRENTTHREAD_H

#include <stdint.h>
#include <sys/syscall.h>

namespace CurrentThread {
    // internal
    __thread int t_cachedTid = 0;
    __thread char t_tidString[32];
    __thread int t_tidStringLength = 6;
    __thread const char* t_threadName = "unknown";
  
    pid_t gettid(){
        return static_cast<pid_t>(::syscall(SYS_gettid));
    }

    void cacheTid(){
        if (t_cachedTid == 0){
            t_cachedTid = gettid();
            t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
        }
    }

    int tid(){
        if (__builtin_expect(t_cachedTid == 0, 0)){
            cacheTid();
        }
        return t_cachedTid;
    }

    inline const char* tidString(){
        return t_tidString;
    }

    inline int tidStringLength(){
        return t_tidStringLength;
    }

    inline const char* name(){
        return t_threadName;
    }

    bool isMainThread(){
        return tid() == ::getpid();
    }

    void sleepUsec(int64_t usec);
}

#endif
