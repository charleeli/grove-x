/*
 * author:charlee
 */
#ifndef _RANDOM_UTIL_H_
#define _RANDOM_UTIL_H_

#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
//产生随机数
class RandomUtil
{
public:
    RandomUtil(){}
    ~RandomUtil(){}

    inline static unsigned int Random(unsigned int unRange){
        if(0 == unRange) return 0;
        return Random() % unRange;
    }

    inline static unsigned int Random(unsigned int unMinRange, unsigned int unMaxRange){
        if(unMinRange == unMaxRange) return unMaxRange;
        return Random(unMaxRange - unMinRange) + unMinRange;
    }

    static unsigned int Random();
private:
    static unsigned int rand_seed_;
    static bool is_seed_set_;
};

unsigned int RandomUtil::rand_seed_ = 1;
bool RandomUtil::is_seed_set_ = false;

unsigned int RandomUtil::Random(){
    if(!is_seed_set_){
        is_seed_set_ = true;
        struct timeval now_time;
        gettimeofday(&now_time, NULL);
        rand_seed_ = (unsigned int)(now_time.tv_usec + getpid());
    }

	unsigned int next = rand_seed_;
    unsigned int result;

    next *= 1103515245;
    next += 12345;
    result = (unsigned int) (next >> 16) & 0x07ff;

    next *= 1103515245;
    next += 12345;
    result <<= 10;
    result ^= (unsigned int) (next >> 16) & 0x03ff;

    next *= 1103515245;
    next += 12345;
    result <<= 10;
    result ^= (unsigned int) (next >> 16) & 0x03ff;

    rand_seed_ = next;

    return result;
}

#endif
