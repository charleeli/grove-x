/*
 * author:charlee
 */
#ifndef _REDISWRAPPER_H_
#define _REDISWRAPPER_H_

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <hiredis/hiredis.h>

using namespace std;

#define REDIS_INIT      (-10000)
#define REDIS_NODATA    (-9999)
#define REDIS_DECODE    (-9998)
#define REDIS_STORE     (-9997)
#define REDIS_EXECUTE   (-9996)

//redis服务器设置的超时时间，5分钟
#define REDIS_SERVER_TIMEOUT 5*60*60

class RedisWrapper {
private:
    redisContext * _pRedis;
    bool connected;
    
    string _host;
    int _port;
    int _timeout;

    time_t _wrapper_start_time;//wrapper启动时间

public:
    RedisWrapper(const string& sHost, int port, int timeout)
    : _pRedis(NULL),
      connected(false){
        initialize(sHost, port, timeout);
    }

    virtual ~RedisWrapper(){
        disconnect();
    }
 
    int initialize(const string& sHost = "127.0.0.1", int port = 6379, int timeout = 500){
        _host = sHost;
        _port = port;
        _timeout = timeout;
        _wrapper_start_time = time(NULL);

        struct timeval tv;
        tv.tv_sec  = timeout / 1000;
        tv.tv_usec = (timeout * 1000 ) % 1000000;
        _pRedis = redisConnectWithTimeout(sHost.c_str(), port, tv);
        if (_pRedis == NULL || _pRedis->err) {
            if (_pRedis){
                redisFree(_pRedis);
                _pRedis = NULL;
            }

            LOG_ERROR << "oh my god, redis disconnected..."<<this;
            return REDIS_INIT;
        }
        redisEnableKeepAlive(_pRedis);
        connected = true;
        LOG_INFO << "redis connected "<<this;
        return 0;
    }

    int reconnect(){
        return initialize(this->_host, this->_port, this->_timeout);
    }

    bool isClosed(){
        return !connected;
    }

    void disconnect(){
        if (_pRedis != NULL){
            redisFree(_pRedis);
            _pRedis = NULL;
        }
        LOG_INFO<<"redis disconnected,"<<"_host:"<<_host<<" _port:"<<_port<<endl;

        connected = false;
    }

    template<class T>
    int getProto(const string& key, T& t){
        if(!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT){
            if(0 != reconnect()){
                return REDIS_INIT;
            }
        }

        redisReply *r = (redisReply *)redisCommand(_pRedis,"GET %b", key.c_str(), key.size());
        if (r == NULL||r->type==REDIS_REPLY_ERROR||_pRedis->err != 0){
            if(r != NULL){
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" redisReply->str:"<<r->str;
            }else{
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" _pRedis->errstr:"<<_pRedis->errstr;
            }

            if (r != NULL){
                freeReplyObject(r);
            }
            return REDIS_EXECUTE;
        }

        if (r != NULL && r->len == 0){
            freeReplyObject(r);
            return REDIS_NODATA;
        }
        
        bool bRet = t.ParseFromArray(r->str, r->len);
        freeReplyObject(r);
        return bRet ? 0 : REDIS_DECODE;
    }

    template<class T>
    int getProto(const string& key, T* t){
        if(!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT){
            if(0 != reconnect()){
                return REDIS_INIT;
            }
        }

        redisReply *r = (redisReply *)redisCommand(_pRedis,"GET %b", key.c_str(), key.size());
        if (r == NULL||r->type==REDIS_REPLY_ERROR||_pRedis->err != 0){
            if(r != NULL){
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" redisReply->str:"<<r->str;
            }else{
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" _pRedis->errstr:"<<_pRedis->errstr;
            }

            if (r != NULL){
                freeReplyObject(r);
            }
            return REDIS_EXECUTE;
        }

        if (r->len == 0){
            freeReplyObject(r);
            return REDIS_NODATA;
        }

        bool bRet = t->ParseFromArray(r->str, r->len);
        freeReplyObject(r);
        return bRet ? 0 : REDIS_DECODE;
    }

    template<class T>
    int setProto(const string& key, const T& t, int iExpireTime = -1){
        if(!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT){
            if(0 != reconnect()){
                return REDIS_INIT;
            }
        }

        redisReply *r = NULL;
        string v = t.SerializeAsString();
        if (iExpireTime == -1)
            r = (redisReply *)redisCommand(_pRedis, 
                                            "SET %b %b", 
                                            key.c_str(), key.size(), v.c_str(), v.size());
        else 
            r = (redisReply *)redisCommand(_pRedis, 
                                            "SET %b %b ex %d", 
                                            key.c_str(), key.size(), v.c_str(), v.size(), iExpireTime);

        if (r == NULL||r->type==REDIS_REPLY_ERROR||_pRedis->err != 0){
            if(r != NULL){
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" redisReply->str:"<<r->str;
            }else{
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" _pRedis->errstr:"<<_pRedis->errstr;
            }

            if (r != NULL){
                freeReplyObject(r);
            }
            return REDIS_EXECUTE;
        }
        freeReplyObject(r);
        return 0;
    }

    int del(const string& key){
        if(!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT){
            if(0 != reconnect()){
                return REDIS_INIT;
            }
        }

        redisReply *r = (redisReply *)redisCommand(_pRedis,"DEL %b", key.c_str(), key.size());
        if (r == NULL||r->type==REDIS_REPLY_ERROR||_pRedis->err != 0){
            if(r != NULL){
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" redisReply->str:"<<r->str;
            }else
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" _pRedis->errstr:"<<_pRedis->errstr;

            if (r != NULL){
                freeReplyObject(r);
            }
            return REDIS_EXECUTE;
        }
        freeReplyObject(r);
        return 0;
    }

    template<class T>
    int getBatchProto(const vector<string>& v, map<string, T>& m, bool bPadding = false){
        if(!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT){
            if(0 != reconnect()){
                return REDIS_INIT;
            }
        }

        int iErrCode = 0;
        for (size_t index = 0; index < v.size(); index++){
            redisAppendCommand(_pRedis, "GET %b", v[index].c_str(), v[index].size());
        }

        redisReply* r = NULL;
        for (size_t index = 0; index < v.size(); index++){
            int iRet = (int)redisGetReply(_pRedis, (void**)&r);
            if (iRet == REDIS_OK){
                T t;
                if (r->len > 0 && t.ParseFromArray(r->str, r->len)){
                    m[v[index]] = t;
                }else if (r->len == 0 && bPadding){
                    m[v[index]] = t;
                }
            }else{
                iErrCode++;
            }

            if (r != NULL){
                freeReplyObject(r);
            }
        }
        return iErrCode;
    }

    template<typename T>
    int setBatchProto(const map<string, T>& m, map<string, int>& mRes){
        if(!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT){
            if(0 != reconnect()){
                return REDIS_INIT;
            }
        }

        int iErrCode = 0;
        typename map<string, T>::const_iterator itm;
        for (itm = m.begin(); itm != m.end(); itm++){
            string v = itm->second.SerializeAsString();
            redisAppendCommand(_pRedis, 
                                "SET %b %b", 
                                itm->first.c_str(), itm->first.size(), v.c_str(), v.size());
        }

        redisReply* r = NULL;
        for (itm = m.begin(); itm != m.end(); itm++){
            int iRet = (int)redisGetReply(_pRedis, (void**)&r);
            if (r != NULL){
                freeReplyObject(r);
            }
            iErrCode += (iRet == REDIS_OK) ? 0 : 1;
            mRes[itm->first] = iRet;
        }
        return iErrCode;
    }

    int delBatchProto(const vector<string>& v, map<string, int>& mRes){
        if(!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT){
            if(0 != reconnect()){
                return REDIS_INIT;
            }
        }

        int iErrCode = 0;
        for (size_t index = 0; index < v.size(); index++){
            redisAppendCommand(_pRedis, "DEL %b", v[index].c_str(), v[index].size());
        }

        redisReply* r = NULL;
        for (size_t index = 0; index < v.size(); index++){
            int iRet = (int)redisGetReply(_pRedis, (void**)&r);
            if (r != NULL){
                freeReplyObject(r);
            }
            iErrCode += (iRet == REDIS_OK) ? 0 : 1;
            mRes[v[index]] = iRet;
        }
        return iErrCode;
    }
    
    int setValue(const string& key, const string& data, int iExpireTime = -1){
        double timestamp = ctime::microtime();
        if(!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT){
            if(0 != reconnect()){
                return REDIS_INIT;
            }
        }

        redisReply *r = NULL;
        if(-1 != iExpireTime ){
            r = (redisReply *)redisCommand(_pRedis,
                "SET %b %b ex %d",
                key.data(),key.size(),data.data(),data.size(),iExpireTime);
        }else{
            r = (redisReply *)redisCommand(_pRedis,
                "SET %b %b",
                key.data(),key.size(),data.data(),data.size());
        }

        if (r == NULL||r->type==REDIS_REPLY_ERROR|| _pRedis->err != 0){
            if(r != NULL){
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" redisReply->str:"<<r->str;
            }else{
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" _pRedis->errstr:"<<_pRedis->errstr;
            }

            if (r != NULL){
                freeReplyObject(r);
            }
            return REDIS_EXECUTE;
        }
        freeReplyObject(r);

        LOG_WARN<<"setValue cost "<<(ctime::microtime() - timestamp)/1000.0<<" ms"<<endl;
        return 0;
    }
   
    int getValue(const string &key, string &out){
        double timestamp = ctime::microtime();

        if(!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT){
            if(0 != reconnect()){
                return REDIS_INIT;
            }
        }

        redisReply *r = (redisReply *)redisCommand(_pRedis,"GET %b", key.c_str(), key.size());
        if (r == NULL||r->type==REDIS_REPLY_ERROR||_pRedis->err != 0){
            if(r != NULL){
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" redisReply->str:"<<r->str;
            }else{
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" _pRedis->errstr:"<<_pRedis->errstr;
            }

            if (r != NULL){
                freeReplyObject(r);
            }
            return REDIS_EXECUTE;
        }
 
        if (r->len == 0){
            LOG_WARN << "r->len == 0"<<r->str;
            freeReplyObject(r);
            return REDIS_NODATA;
        }

        out = string(r->str, r->len);
        freeReplyObject(r);

        LOG_WARN<<"getValue cost "<<(ctime::microtime() - timestamp)/1000.0<<" ms"<<endl;
        return  0;
    }

   int getBatchString(const vector<string>& v, map<string, string>& m, bool bPadding = false){
       double timestamp = ctime::microtime();

       if(!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT){
           if(0 != reconnect()){
               return REDIS_INIT;
           }
       }

       string sCommand = "MGET ";
       string multi_key="";
       for (size_t index = 0; index < v.size(); index++){
           multi_key += v[index] + " ";
       }
       sCommand+=multi_key;

       redisReply* r = (redisReply*)redisCommand(_pRedis,sCommand.c_str());
       if (r == NULL||r->type==REDIS_REPLY_ERROR||_pRedis->err != 0){
           if(r != NULL){
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" redisReply->str:"<<r->str;
            }else{
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" _pRedis->errstr:"<<_pRedis->errstr;
            }

           if (r != NULL){
                freeReplyObject(r);
           }
           return REDIS_EXECUTE;
       }

        if (r->type != REDIS_REPLY_ARRAY) {
            LOG_ERROR<<"Failed to execute command MGET "<<multi_key<<endl;
            freeReplyObject(r);

            if(2 != r->elements){
                return REDIS_EXECUTE;
            }
        }

         for (int index = 0; index < (int)r->elements; ++index) {
            redisReply* childReply = r->element[index];
            if (childReply->type == REDIS_REPLY_STRING){
                m[v[index]] = string(childReply->str);
            }
        }

        if (r != NULL){
            freeReplyObject(r);
        }

       LOG_WARN<<"getBatchString cost "<<(ctime::microtime() - timestamp)/1000.0<<" ms"<<endl;
       return 0;
   }

    template<typename T>
    int getAllProtoFromHash(const string &hashtable,  vector<T> &v){
        if(!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT){
            if(0 != reconnect()){
                return REDIS_INIT;
            }
        }

        redisReply *r = NULL;
        r= (redisReply *)redisCommand(_pRedis,"HVALS %b", hashtable.data(),hashtable.size());
        if (r == NULL||r->type==REDIS_REPLY_ERROR||_pRedis->err != 0){
            if(r != NULL){
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" redisReply->str:"<<r->str;
            }else{
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" _pRedis->errstr:"<<_pRedis->errstr;
            }

            if (r != NULL){
                freeReplyObject(r);
            }
            return REDIS_EXECUTE;
        }
 
        if (r->type == REDIS_REPLY_ARRAY) {
            for (size_t  j = 0; j<r->elements; j++) {
                T t;
                t.ParseFromArray(r->element[j]->str, r->element[j]->len);
                v.push_back(t);
            }
        }
        
        freeReplyObject(r);
        return v.size();
    }
    
    int getAllStringFromHash(const string &hashtable,  vector<string> &v){
        if(!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT){
            if(0 != reconnect()){
                return REDIS_INIT;
            }
        }

       redisReply *r = NULL;
       r= (redisReply *)redisCommand(_pRedis,"HVALS %b", hashtable.data(),hashtable.size());
       if (r == NULL||r->type==REDIS_REPLY_ERROR||_pRedis->err != 0){
           if(r != NULL){
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" redisReply->str:"<<r->str;
            }else{
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" _pRedis->errstr:"<<_pRedis->errstr;
            }

            if (r != NULL){
                freeReplyObject(r);
            }
            return REDIS_EXECUTE;
       }

       if (r->type == REDIS_REPLY_ARRAY){
           for (size_t  j = 0; j<r->elements; j++){
               v.push_back(string(r->element[j]->str, r->element[j]->len));
           }
       }

       freeReplyObject(r);
       return v.size();
    }

    template<typename T>
    int getProtoFromHash(const string &hashtable, const string &key, T &v){
        if(!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT){
            if(0 != reconnect()){
                return REDIS_INIT;
            }
        }

        redisReply *r = NULL;
        r= (redisReply *)redisCommand(_pRedis,
            "HGET %b %b",
            hashtable.data(), hashtable.size(),key.data(),key.size());
        if (r == NULL||r->type==REDIS_REPLY_ERROR||_pRedis->err != 0){
            if(r != NULL){
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" redisReply->str:"<<r->str;
            }else{
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" _pRedis->errstr:"<<_pRedis->errstr;
            }

            if (r != NULL){
                freeReplyObject(r);
            }
            return REDIS_EXECUTE;
        }
 
        if (r != NULL && r->len == 0){
            freeReplyObject(r);
            return REDIS_NODATA;
        }
        
        v.ParseFromArray(r->str, r->len);
        
        freeReplyObject(r);
        return 0;
    }

    int getStringFromHash(const string &hashtable, const string &key, string &v){
        if(!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT){
            if(0 != reconnect()){
                return REDIS_INIT;
            }
        }

        redisReply *r = NULL;
        r= (redisReply *)redisCommand(_pRedis,
                "HGET %b %b",
                hashtable.data(), hashtable.size(),key.data(),key.size());
        if (r == NULL||r->type==REDIS_REPLY_ERROR||_pRedis->err != 0){
            if(r != NULL){
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" redisReply->str:"<<r->str;
            }else{
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" _pRedis->errstr:"<<_pRedis->errstr;
            }

            if (r != NULL){
                freeReplyObject(r);
            }
            return REDIS_EXECUTE;
        }

        if (r != NULL && r->len == 0){
            freeReplyObject(r);
            return REDIS_NODATA;
        }

        v=string(r->str, r->len);

        freeReplyObject(r);
        return 0;
    }
    
    template<typename T>
    int setProtoToHash(const string &hashtable, const string &key, T &v){
        if(!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT){
            if(0 != reconnect()){
                return REDIS_INIT;
            }
        }
       
        redisReply *r = (redisReply *)redisCommand(_pRedis,
            "HSET %b %b %b",
            hashtable.data(),hashtable.size(),
            key.data(),key.size(),
            v.SerializeAsString().data(),v.SerializeAsString().size());
        if (r == NULL||r->type==REDIS_REPLY_ERROR||_pRedis->err != 0){
            if(r != NULL){
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" redisReply->str:"<<r->str;
            }else{
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" _pRedis->errstr:"<<_pRedis->errstr;
            }

            if (r != NULL){
                freeReplyObject(r);
            }
            return REDIS_EXECUTE;
        }

        freeReplyObject(r);
        return 0;
    }
    
    int setStringToHash(const string &hashtable, const string &key, std::string &v){
        if(!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT){
            if(0 != reconnect()){
                return REDIS_INIT;
            }
        }

        redisReply *r = (redisReply *)redisCommand(_pRedis,
                "HSET %b %b %b",
                hashtable.data(),hashtable.size(),key.data(),key.size(),v.data(),v.size());
        if (r == NULL||r->type==REDIS_REPLY_ERROR||_pRedis->err != 0){
            if(r != NULL){
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" redisReply->str:"<<r->str;
            }else{
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" _pRedis->errstr:"<<_pRedis->errstr;
            }

            if (r != NULL){
                freeReplyObject(r);
            }
            return REDIS_EXECUTE;
        }

        freeReplyObject(r);
        return 0;
    }

    int rpush(const string &key, const string &v){
        if(!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT){
            if(0 != reconnect()){
                LOG_ERROR << "reconnect failed";
                return REDIS_INIT;
            }
        }
 
        redisReply *r = (redisReply *)redisCommand(_pRedis,
            "RPUSH %b %b",
            key.data(),key.size(),v.data(),v.size());
        if (r == NULL||r->type==REDIS_REPLY_ERROR||_pRedis->err != 0){
            if(r != NULL){
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" redisReply->str:"<<r->str;
            }else{
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" _pRedis->errstr:"<<_pRedis->errstr;
            }

            if (r != NULL){
                freeReplyObject(r);
            }
            return REDIS_EXECUTE;
        }

        freeReplyObject(r);
        return 0;
    }
  
    bool hashKeyExists(const string& hashtable, const string& key){
        if(!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT){
            if(0 != reconnect()){
                return REDIS_INIT;
            }
        }

        redisReply *r = (redisReply *)redisCommand(_pRedis,
                "HEXISTS  %b %b",
                hashtable.data(), hashtable.size(),key.data(),key.size());
        if (r == NULL||r->type==REDIS_REPLY_ERROR||_pRedis->err != 0){
            if(r != NULL){
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" redisReply->str:"<<r->str;
            }else{
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" _pRedis->errstr:"<<_pRedis->errstr;
            }

            if (r != NULL){
                freeReplyObject(r);
            }
            return REDIS_EXECUTE;
        }

        bool bExist = (r->integer > 0);
        freeReplyObject(r);
        return bExist;
    }

    int hashDel(const string& hashtable, const string& field){
        if(!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT){
            if(0 != reconnect()){
                return REDIS_INIT;
            }
        }

        redisReply *r = (redisReply *)redisCommand(_pRedis,
            "HDEL  %b %b",
            hashtable.data(), hashtable.size(),field.data(),field.size());
        if (r == NULL||r->type==REDIS_REPLY_ERROR||_pRedis->err != 0){
            if(r != NULL){
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" redisReply->str:"<<r->str;
            }else{
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" _pRedis->errstr:"<<_pRedis->errstr;
            }

            if (r != NULL){
                freeReplyObject(r);
            }
            return REDIS_EXECUTE;
        }

        int count = r->integer;
        freeReplyObject(r);
        return count;
    }
 
    bool isKeyExist(const string& key){
        if(!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT){
            if(0 != reconnect()){
                return REDIS_INIT;
            }
        }

        redisReply *r = (redisReply *)redisCommand(_pRedis,"exists %b", key.data(), key.size());
        if (r == NULL||r->type==REDIS_REPLY_ERROR||_pRedis->err != 0){
            if(r != NULL){
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" redisReply->str:"<<r->str;
            }else{
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" _pRedis->errstr:"<<_pRedis->errstr;
            }

            if (r != NULL){
                freeReplyObject(r);
            }
            return REDIS_EXECUTE;
        }

        bool bExist = (r->integer > 0);
        freeReplyObject(r);
        return bExist;
    }

    int sadd(const string& key, const string& member){
        if (!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT) {
            if (0 != reconnect()) {
                return REDIS_INIT;
            }
        }

        redisReply *r = (redisReply *)redisCommand(_pRedis, "SADD %b %b", key.data(), key.size(),
            member.data(), member.size());
        if (r == NULL||r->type==REDIS_REPLY_ERROR||_pRedis->err != 0){
            if(r != NULL){
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" redisReply->str:"<<r->str;
            }else{
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" _pRedis->errstr:"<<_pRedis->errstr;
            }

            if (r != NULL){
                freeReplyObject(r);
            }
            return REDIS_EXECUTE;
        }

        freeReplyObject(r);
        return 0;
    }

    int srem(const string& key, const string& member){
        if(!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT){
            if(0 != reconnect()){
                return REDIS_INIT;
            }
        }

        redisReply *r = (redisReply *)redisCommand(_pRedis, "SREM %b %b", key.data(), key.size(),
            member.data(), member.size());
        if (r == NULL||r->type==REDIS_REPLY_ERROR||_pRedis->err != 0){
            if(r != NULL){
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" redisReply->str:"<<r->str;
            }else{
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" _pRedis->errstr:"<<_pRedis->errstr;
            }

            if (r != NULL){
                freeReplyObject(r);
            }
            return REDIS_EXECUTE;
        }

        freeReplyObject(r);
        return 0;
    }

    int smembers(vector<string>& members, const string& key){
        if(!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT){
            if(0 != reconnect()){
                return REDIS_INIT;
            }
        }

        redisReply *r = (redisReply *)redisCommand(_pRedis, "SMEMBERS %b", key.data(), key.size());
        if (r == NULL||r->type==REDIS_REPLY_ERROR||_pRedis->err != 0){
            if(r != NULL){
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" redisReply->str:"<<r->str;
            }else{
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" _pRedis->errstr:"<<_pRedis->errstr;
            }

            if (r != NULL){
                freeReplyObject(r);
            }
            return REDIS_EXECUTE;
        }

        if (r->type == REDIS_REPLY_ARRAY) {
            for (size_t i = 0; i<r->elements; i++) {
                members.push_back(r->element[i]->str);
            }
        }

        freeReplyObject(r);
        return 0;
    }

    int scard(const string& key){
        if(!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT){
            if(0 != reconnect()){
                return REDIS_INIT;
            }
        }

        redisReply *r = (redisReply *)redisCommand(_pRedis, "SCARD %b", key.data(), key.size());
        if (r == NULL||r->type==REDIS_REPLY_ERROR||_pRedis->err != 0){
            if(r != NULL){
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" redisReply->str:"<<r->str;
            }else{
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" _pRedis->errstr:"<<_pRedis->errstr;
            }

            if (r != NULL){
                freeReplyObject(r);
            }
            return REDIS_EXECUTE;
        }

        int count = r->integer;
        freeReplyObject(r);

        return count;
    }

    int sdiffstore(const string& destination, const string& key1, const string& key2){
        if(!connected ||time(NULL) > _wrapper_start_time + REDIS_SERVER_TIMEOUT){
            if(0 != reconnect()){
                return REDIS_INIT;
            }
        }

        redisReply *r = (redisReply *)redisCommand(_pRedis, "SDIFFSTORE %b %b %b",
            destination.data(), destination.size(), key1.data(), key1.size(), key2.data(), key2.size());
        if (r == NULL || r->type==REDIS_REPLY_ERROR || _pRedis->err != 0 ) {
            if(r != NULL){
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" redisReply->str:"<<r->str;
            }else{
                LOG_ERROR << "_pRedis->err:" << _pRedis->err<<" _pRedis->errstr:"<<_pRedis->errstr;
            }

            if (r != NULL){
                freeReplyObject(r);
            }
            return REDIS_EXECUTE;
        }

        freeReplyObject(r);
        return 0;
    }

    string getError(){
        if (_pRedis != NULL){
            return string(_pRedis->errstr);
        }
        return "";
    }

    redisContext* getRedis(){
        return _pRedis;
    }
};

#endif
