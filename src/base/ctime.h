/*
 * author:charlee
 */
#ifndef _CTIME_H_
#define _CTIME_H_

#include <stdio.h>
#include <sys/time.h>

class ctime {
public:
static double
microtime() {
    struct timeval time;

    gettimeofday(&time, NULL);

    double second = time.tv_sec;
    double usec = time.tv_usec;
    double microsecond = second*1000000+usec;

    return microsecond;
}

static double
timestamp() {
    struct timeval time;

    gettimeofday(&time, NULL);

    double second = time.tv_sec;
    double usec = time.tv_usec;
    double microsecond = second+usec*0.000001;

    return microsecond;
}

};

#endif
