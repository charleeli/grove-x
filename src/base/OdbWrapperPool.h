/*
 * author:charlee
 */
#ifndef _ODB_WRAPPER_POOL_H_
#define _ODB_WRAPPER_POOL_H_

#include <odb/database.hxx>
#include <odb/schema-catalog.hxx>
#include <odb/session.hxx>
#include <odb/callback.hxx>
#include <odb/connection.hxx>
#include <odb/transaction.hxx>
#include <odb/mysql/database.hxx>
#include <odb/mysql/tracer.hxx>
#include <odb/mysql/statement.hxx>
#include <odb/mysql/connection-factory.hxx>
#include <odb/mysql/exceptions.hxx>
#include <odb/mysql/tracer.hxx>
#include <odb/mysql/connection.hxx>

#include <list>
#include "Mutex.h"
#include "ctime.h"

using namespace std;
using OdbWrapper = odb::mysql::database;

class MysqlTracer: public odb::mysql::tracer {
    virtual void prepare (odb::mysql::connection& c, const odb::mysql::statement& s){
        //LOG_WARN << c.database ().db () << " prepare: " << s.text () << endl;
    }

    virtual void execute (odb::mysql::connection& c, const odb::mysql::statement& s){
        LOG_WARN << c.database ().db () << " execute: " << s.text() << endl;
    }

    virtual void execute (odb::mysql::connection& c, const char* statement){
        LOG_WARN << c.database ().db () << " execute: " << statement << endl;
    }

    virtual void deallocate (odb::mysql::connection& c, const odb::mysql::statement& s){
        //LOG_WARN << c.database ().db () << " deallocate: " << s.text() << endl;
    }
};

class OdbWrapperPool {
private:
    int curSize; //当前已建立的数据库连接数量
    int maxSize; //连接池中定义的最大数据库连接数
    string host;
    int port;
    string user;
    string password;
    string database;

    std::list<OdbWrapper*> wrapperList; //连接池的容器队列
    mutable MutexLock mutex;
    static OdbWrapperPool *wrapperPool;

    OdbWrapper* CreateOdbWrapper(); //创建一个连接
    void InitPool(int size); //初始化数据库连接池
    void DestoryOdbWrapper(OdbWrapper *wrapper); //销毁数据库连接对象
    void DestoryOdbWrapperPool(); //销毁数据库连接池
    OdbWrapperPool(string host, int port,string user,string password,string database,int maxSize); //构造方法

public:
    ~OdbWrapperPool();
    OdbWrapper* GetOdbWrapper(); //获得数据库连接
    void ReleaseOdbWrapper(OdbWrapper *wrapper); //将数据库连接放回到连接池的容器中
    static OdbWrapperPool *GetInstance(string host, int port,string user,string password,string database,int maxSize); //获取数据库连接池对象
    int Size(){MutexLockGuard lock(mutex);return wrapperList.size();}
};

class OdbWrapperGuard{
private:
    OdbWrapperPool* pool;
    OdbWrapper* wrapper;
    MysqlTracer *mytracer;

public:
    OdbWrapperGuard(OdbWrapperPool* pool,odb::mysql::database* wrapper)
    {
        this->pool = pool;
        this->wrapper = wrapper;
        this->mytracer = new MysqlTracer();
    }

    ~OdbWrapperGuard()
    {
        pool->ReleaseOdbWrapper(wrapper);
        delete this->mytracer;
    }

    OdbWrapper* getOdbWrapper()
    {
        wrapper->tracer(mytracer);
        return wrapper;
    }
};

OdbWrapperPool *OdbWrapperPool::wrapperPool = NULL;

//获取连接池对象，单例模式
OdbWrapperPool* OdbWrapperPool::GetInstance(string host, int port,string user,string password,string database,int maxSize) {
    if (wrapperPool == NULL) {
        wrapperPool = new OdbWrapperPool(host,port,user,password,database,maxSize);
        if(wrapperPool == NULL)
        {
            LOG_WARN << "wrapperPool == NULL";
        }
    }
    return wrapperPool;
}

//连接池的构造函数
OdbWrapperPool::OdbWrapperPool(string host,int port,string user,string password,string database,int maxSize){
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
void OdbWrapperPool::InitPool(int iInitialSize) {
    MutexLockGuard lock(mutex);
    odb::mysql::database  * wrapper;
    for (int i = 0; i < iInitialSize; i++){
        wrapper = this->CreateOdbWrapper();
        if(wrapper){
            wrapperList.push_back(wrapper);
            ++(this->curSize);
        }
    }
}

//创建连接,返回一个Connection
OdbWrapper* OdbWrapperPool::CreateOdbWrapper() {
    auto_ptr<odb::mysql::connection_factory> f (
            new odb::mysql::connection_pool_factory (1));

    OdbWrapper* wrapper = new OdbWrapper(
            user.c_str(), password.c_str(),database.c_str(),host.c_str(),port,NULL,"utf8",0,f);

    return wrapper;
}

//在连接池中获得一个连接
OdbWrapper* OdbWrapperPool::GetOdbWrapper() {
    MutexLockGuard lock(mutex);
    OdbWrapper* wrapper;
    if(wrapperList.size()>0){
        wrapper = wrapperList.front(); //得到第一个连接
        wrapperList.pop_front();
        /*if (wrapper->isClosed()){//如果连接已经被关闭，删除后重新建立一个
            delete wrapper;
            wrapper = this->CreateOdbWrapper();
            if (wrapper == NULL){
                --curSize;
            }
        }*/
        return wrapper;
    }else{
        if(curSize < maxSize){
            wrapper = this->CreateOdbWrapper();
            if (wrapper){
                ++curSize;
                return wrapper;
            }
        }
    }

    return NULL;
}

//回收数据库连接
void OdbWrapperPool::ReleaseOdbWrapper(OdbWrapper * wrapper)
{
    MutexLockGuard lock(mutex);
    if (wrapper) {
        wrapperList.push_back(wrapper);
    }
}

//连接池的析构函数
OdbWrapperPool::~OdbWrapperPool() {
    this->DestoryOdbWrapperPool();
}

//销毁连接池,首先要先销毁连接池的中连接
void OdbWrapperPool::DestoryOdbWrapperPool(){
    MutexLockGuard lock(mutex);

    std::list<OdbWrapper*>::iterator it;
    for (it = wrapperList.begin(); it != wrapperList.end(); ++it) {
        this->DestoryOdbWrapper(*it); //销毁连接池中的连接
    }

    curSize = 0;
    wrapperList.clear(); //清空连接池中的连接
}

//销毁一个连接
void OdbWrapperPool::DestoryOdbWrapper(OdbWrapper* wrapper) {
    if (wrapper) {
        delete wrapper;
    }
}

#endif
