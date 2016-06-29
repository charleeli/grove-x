#ifndef _REDIS_LOCK_H_
#define _REDIS_LOCK_H_

#include "redlock/redlock.h"
#include "System.h"

using namespace std;

class RedisLock {
private:
    string ip;
    int port;

    CLock my_lock;
    CRedLock *dlm;

    bool acquired;

    double timestamp;

public:
    RedisLock(const string ip,int port,const string resource,const int ttl){
        this->ip = ip;
        this->port = port;
        this->dlm = new CRedLock();
        this->dlm->AddServerUrl(this->ip.c_str(), this->port);
        this->acquired = this->dlm->Lock(resource.c_str(), ttl, this->my_lock);

        this->timestamp = ctime::microtime();

        if(this->acquired){
            LOG_INFO<<"Acquired by client name:"<<my_lock.m_val<<", res:"<<my_lock.m_resource<<", vttl:"<<my_lock.m_validityTime<<endl;
        }else{
            LOG_ERROR<<"lock not acquired, name:"<<my_lock.m_val<<", res:"<<my_lock.m_resource<<endl;
        }
    }

    bool Acquired(){
        return this->acquired;
    }

    string getError(){
        return string("lock not acquired, name:") + my_lock.m_val + ", res:" + my_lock.m_resource;
    }

    ~RedisLock(){
        LOG_WARN<<"RedisLock "<<my_lock.m_resource<<" cost "<<(ctime::microtime() - this->timestamp)/1000.0<<" ms"<<endl;

        this->dlm->Unlock(this->my_lock);
        delete this->dlm;
    }
};

#endif
