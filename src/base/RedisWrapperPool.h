/*
 * author:charlee
 */
#ifndef _REDIS_WRAPPER_POOL_H_
#define _REDIS_WRAPPER_POOL_H_

#include "Mutex.h"
#include "rediswrapper.h"

class RedisWrapperPool {
private:
    int curSize; //当前已建立的数据库连接数量
    int maxSize; //连接池中定义的最大数据库连接数
    string host;
    int port;
    int timeout;

    std::list<RedisWrapper*> wrapperList; //连接池的容器队列
    mutable MutexLock mutex;
    static RedisWrapperPool *wrapperPool;

    RedisWrapper* CreateRedisWrapper(); //创建一个连接
    void InitPool(int size); //初始化数据库连接池
    void DestoryRedisWrapper(RedisWrapper *wrapper); //销毁数据库连接对象
    void DestoryRedisWrapperPool(); //销毁数据库连接池
    RedisWrapperPool(string host, int port, int timeout,int maxSize); //构造方法

public:
    ~RedisWrapperPool();
    RedisWrapper* GetRedisWrapper(); //获得数据库连接
    void ReleaseRedisWrapper(RedisWrapper *wrapper); //将数据库连接放回到连接池的容器中
    static RedisWrapperPool *GetInstance(string host, int port, int timeout,int maxSize); //获取数据库连接池对象
    int Size(){MutexLockGuard lock(mutex);return wrapperList.size();}
};

class RedisWrapperGuard{
private:
    RedisWrapperPool* pool;
    RedisWrapper* wrapper;
public:
    RedisWrapperGuard(RedisWrapperPool* pool,RedisWrapper* wrapper){
        this->pool = pool;
        this->wrapper = wrapper;
    }

    ~RedisWrapperGuard(){
        pool->ReleaseRedisWrapper(wrapper);
    }

    RedisWrapper* getRedisWrapper(){
        return wrapper;
    }
};

RedisWrapperPool *RedisWrapperPool::wrapperPool = NULL;

//获取连接池对象，单例模式
RedisWrapperPool* RedisWrapperPool::GetInstance(string host, int port, int timeout,int maxSize) {
    if (wrapperPool == NULL) {
        wrapperPool = new RedisWrapperPool(host, port, timeout,maxSize);
    }
    return wrapperPool;
}

//连接池的构造函数
RedisWrapperPool::RedisWrapperPool(string host, int port, int timeout,int maxSize){
    this->host = host;
    this->port = port;
    this->timeout = timeout;
    this->maxSize = maxSize;
    this->curSize = 0;
    this->InitPool(maxSize / 2);
}

//初始化连接池，创建最大连接数的一半连接数量
void RedisWrapperPool::InitPool(int iInitialSize) {
    MutexLockGuard lock(mutex);
    RedisWrapper  * wrapper;
    for (int i = 0; i < iInitialSize; i++){
        wrapper = this->CreateRedisWrapper();
        if(wrapper){
            wrapperList.push_back(wrapper);
            ++(this->curSize);
        }
    }
}

//创建连接,返回一个Connection
RedisWrapper* RedisWrapperPool::CreateRedisWrapper() {
    RedisWrapper* wrapper = new RedisWrapper(host,port,timeout);
    return wrapper;
}

//在连接池中获得一个连接
RedisWrapper* RedisWrapperPool::GetRedisWrapper() {
    MutexLockGuard lock(mutex);
    RedisWrapper* wrapper;
    if(wrapperList.size()>0){
        wrapper = wrapperList.front(); //得到第一个连接
        wrapperList.pop_front();
        if (wrapper->isClosed()){   //如果连接已经被关闭，删除后重新建立一个
            delete wrapper;
            wrapper = this->CreateRedisWrapper();
            if (wrapper == NULL){
                --curSize;
            }
        }
        return wrapper;
    }else{
        if(curSize < maxSize){
            wrapper = this->CreateRedisWrapper();
            if (wrapper){
                ++curSize;
                return wrapper;
            }
        }
    }

    return NULL;
}

//回收数据库连接
void RedisWrapperPool::ReleaseRedisWrapper(RedisWrapper * wrapper){
    MutexLockGuard lock(mutex);
    if (wrapper) {
        wrapperList.push_back(wrapper);
    }
}

//连接池的析构函数
RedisWrapperPool::~RedisWrapperPool() {
    this->DestoryRedisWrapperPool();
}

//销毁连接池,首先要先销毁连接池的中连接
void RedisWrapperPool::DestoryRedisWrapperPool(){
    MutexLockGuard lock(mutex);

    std::list<RedisWrapper*>::iterator it;
    for (it = wrapperList.begin(); it != wrapperList.end(); ++it) {
        this->DestoryRedisWrapper(*it); //销毁连接池中的连接
    }

    curSize = 0;
    wrapperList.clear(); //清空连接池中的连接
}

//销毁一个连接
void RedisWrapperPool::DestoryRedisWrapper(RedisWrapper* wrapper) {
    if (wrapper) {
        wrapper->disconnect();
        delete wrapper;
    }
}

#endif
