/*
 * author:charlee
 */
#ifndef _ODB_CONNECTION_POOL_H_
#define _ODB_CONNECTION_POOL_H_

#include <memory>
#include <mutex>

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

#include "ctime.h"
#include "GLogHelper.h"

using namespace std;

class OdbConnectionPool {
private:
    int maxSize;
    string host;
    int port;
    string user;
    string password;
    string database;

    std::mutex mtx;
    std::shared_ptr<odb::mysql::database> db;
    static OdbConnectionPool *connectionPool;

    OdbConnectionPool(string host, int port,string user,string password,string database,int maxSize);

public:
    ~OdbConnectionPool();
    odb::connection_ptr GetConnection();
    static OdbConnectionPool *GetInstance(string host, int port,string user,string password,string database,int maxSize);
};

OdbConnectionPool *OdbConnectionPool::connectionPool = NULL;

OdbConnectionPool* OdbConnectionPool::GetInstance(string host, int port,string user,string password,string database,int maxSize) {
    if (connectionPool == NULL) {
        connectionPool = new OdbConnectionPool(host,port,user,password,database,maxSize);
        if(connectionPool == NULL)
        {
            LOG(INFO) << "connectionPool == NULL";
        }
    }
    return connectionPool;
}

OdbConnectionPool::OdbConnectionPool(string host,int port,string user,string password,string database,int maxSize)
{
    this->host = host;
    this->port = port;
    this->user = user;
    this->password = password;
    this->database = database;
    this->maxSize = maxSize;

    auto_ptr<odb::mysql::connection_factory> f (
            new odb::mysql::connection_pool_factory (0,maxSize));

    this->db = std::shared_ptr<odb::mysql::database>(
            new odb::mysql::database (user.c_str(), password.c_str(),database.c_str(),host.c_str(),port,NULL,NULL,0,f));
}

odb::connection_ptr OdbConnectionPool::GetConnection(){
    std::lock_guard<std::mutex> lck (mtx);
    return db->connection();
}

#endif
