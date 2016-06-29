/*
 * author:charlee
 */
#ifndef _MYSQL_WRAPPER_POOL_H_
#define _MYSQL_WRAPPER_POOL_H_

#include "Mutex.h"
#include "mysqlwrapper.h"
#include "ctime.h"

class MysqlWrapperPool {
private:
    int curSize; //当前已建立的数据库连接数量
    int maxSize; //连接池中定义的最大数据库连接数
    string host;
    int port;
    string user;
    string password;
    string database;

    std::list<MysqlWrapper*> wrapperList; //连接池的容器队列
    mutable MutexLock mutex;
    static MysqlWrapperPool *wrapperPool;

    MysqlWrapper* CreateMysqlWrapper(); //创建一个连接
    void InitPool(int size); //初始化数据库连接池
    void DestoryMysqlWrapper(MysqlWrapper *wrapper); //销毁数据库连接对象
    void DestoryMysqlWrapperPool(); //销毁数据库连接池
    MysqlWrapperPool(string host, int port,string user,string password,string database,int maxSize); //构造方法

public:
    ~MysqlWrapperPool();
    MysqlWrapper* GetMysqlWrapper(); //获得数据库连接
    void ReleaseMysqlWrapper(MysqlWrapper *wrapper); //将数据库连接放回到连接池的容器中
    static MysqlWrapperPool *GetInstance(string host, int port,string user,string password,string database,int maxSize); //获取数据库连接池对象
    int Size(){MutexLockGuard lock(mutex);return wrapperList.size();}
};

class MysqlWrapperGuard{
private:
    MysqlWrapperPool* pool;
    MysqlWrapper* wrapper;

public:
    MysqlWrapperGuard(MysqlWrapperPool* pool,MysqlWrapper* wrapper){
        this->pool = pool;
        this->wrapper = wrapper;
    }

    ~MysqlWrapperGuard(){
        pool->ReleaseMysqlWrapper(wrapper);
    }

    MysqlWrapper* getMysqlWrapper(){
        return wrapper;
    }
};

MysqlWrapperPool *MysqlWrapperPool::wrapperPool = NULL;

//获取连接池对象，单例模式
MysqlWrapperPool* MysqlWrapperPool::GetInstance(string host, int port,string user,string password,string database,int maxSize) {
    if (wrapperPool == NULL) {
        wrapperPool = new MysqlWrapperPool(host,port,user,password,database,maxSize);
        if(wrapperPool == NULL){
            LOG(INFO) << "wrapperPool == NULL";
        }
    }
    return wrapperPool;
}

//连接池的构造函数
MysqlWrapperPool::MysqlWrapperPool(string host,int port,string user,string password,string database,int maxSize){
    this->host = host;
    this->port = port;
    this->user = user;
    this->password = password;
    this->database = database;

    this->maxSize = maxSize;
    this->curSize = 0;
    this->InitPool(maxSize / 2);
}

//初始化连接池，创建最大连接数的一半连接数量
void MysqlWrapperPool::InitPool(int iInitialSize) {
    MutexLockGuard lock(mutex);
    MysqlWrapper  * wrapper;
    for (int i = 0; i < iInitialSize; i++){
        wrapper = this->CreateMysqlWrapper();
        if(wrapper){
            wrapperList.push_back(wrapper);
            ++(this->curSize);
        }
    }
}

//创建连接,返回一个Connection
MysqlWrapper* MysqlWrapperPool::CreateMysqlWrapper() {
    MysqlWrapper* wrapper = new MysqlWrapper(host,port,user,password,database);
    return wrapper;
}

//在连接池中获得一个连接
MysqlWrapper* MysqlWrapperPool::GetMysqlWrapper() {
    MutexLockGuard lock(mutex);
    MysqlWrapper* wrapper;
    if(wrapperList.size()>0){
        wrapper = wrapperList.front(); //得到第一个连接
        wrapperList.pop_front();
        if (wrapper->isClosed()){//如果连接已经被关闭，删除后重新建立一个
            delete wrapper;
            wrapper = this->CreateMysqlWrapper();
            if (wrapper == NULL){
                --curSize;
            }
        }
        return wrapper;
    }else{
        if(curSize < maxSize){
            wrapper = this->CreateMysqlWrapper();
            if (wrapper){
                ++curSize;
                return wrapper;
            }
        }
    }

    return NULL;
}

//回收数据库连接
void MysqlWrapperPool::ReleaseMysqlWrapper(MysqlWrapper * wrapper){
    MutexLockGuard lock(mutex);
    if (wrapper) {
        wrapperList.push_back(wrapper);
    }
}

//连接池的析构函数
MysqlWrapperPool::~MysqlWrapperPool() {
    this->DestoryMysqlWrapperPool();
}

//销毁连接池,首先要先销毁连接池的中连接
void MysqlWrapperPool::DestoryMysqlWrapperPool(){
    MutexLockGuard lock(mutex);

    std::list<MysqlWrapper*>::iterator it;
    for (it = wrapperList.begin(); it != wrapperList.end(); ++it) {
        this->DestoryMysqlWrapper(*it); //销毁连接池中的连接
    }

    curSize = 0;
    wrapperList.clear(); //清空连接池中的连接
}

//销毁一个连接
void MysqlWrapperPool::DestoryMysqlWrapper(MysqlWrapper* wrapper) {
    if (wrapper) {
        wrapper->disconnect();
        delete wrapper;
    }
}

#endif
