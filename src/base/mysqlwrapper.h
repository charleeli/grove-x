/*
 * author:charlee
 */
#ifndef _MYSQLWRAPPER_H_
#define _MYSQLWRAPPER_H_

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include "mysql/mysql.h"
#include "Mutex.h"

using namespace std;

#define MYSQL_INIT      (-10000)
#define MYSQL_NODATA    (-9999)
#define MYSQL_DECODE    (-9998)
#define MYSQL_STORE     (-9997)
#define MYSQL_EXECUTE   (-9996)

class MysqlWrapper {
private:
    MYSQL *_pstMql;
    string _host;
    string _user;
    string _password;
    string _database;
    string _table;
    string _charset;
    int _port;
    int _flag;
    bool _bConnected;
    string _sLastSql;

public:
    MysqlWrapper(const string& sHost,
                          int  port, 
                const string& sUser, 
                const string& sPasswd, 
                const string& sDatabase ,
                const string& sTable = ""
                ) 
    : _pstMql(NULL), 
    _host(sHost), 
    _user(sUser), 
    _password(sPasswd), 
    _database(sDatabase),
    _table(sTable)
    {
        initialize(sHost, sUser, sPasswd, sDatabase,sTable, port);
    }

    virtual ~MysqlWrapper(){
        LOG_INFO << "mysql dctor !";
        disconnect();
    }
  
    int initialize(const string& sHost, 
                    const string& sUser, 
                    const string& sPasswd,
                    const string& sDatabase, 
                    const string& sTable, 
                    int port = 3306, 
                    const string &sCharSet = "", 
                    int iFlag = 0)
    {      
        _host        = sHost;
        _user        = sUser;
        _password    = sPasswd;
        _database    = sDatabase;
        _table       = sTable,
        _charset     = sCharSet;
        _port        = port;
        _flag        = 0;
        _bConnected  = false;

        connect();
        return 0;
    }

    bool isClosed(){
        return !_bConnected;
    }

    void connect(){
        if (_pstMql == NULL){
            _pstMql = mysql_init(NULL);
        }
      
        if(!_charset.empty()) {
            if (mysql_options(_pstMql, MYSQL_SET_CHARSET_NAME, _charset.c_str())) {
                // TODO
                return;
            }
        }else{
            if (mysql_options(_pstMql, MYSQL_SET_CHARSET_NAME, "utf8")) {
                // TODO
                return;
            }
        }

        if (mysql_real_connect(_pstMql, 
                            _host.c_str(),  
                            _user.c_str(), 
                            _password.c_str(), 
                            _database.c_str(), 
                            _port, NULL, 
                            _flag) == NULL) 
        {
            // TODO
            return;
        }
        
        if(!_table.empty()){
            //如果没有创建_tabale表，则创建
            ostringstream osql;
            osql << "CREATE TABLE IF NOT EXISTS " << _table
                 << "("
                 << "ukey varchar(64) not null primary key, "
                 << "udata blob, "
                 << "ts TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP"
                 << ")";
            _sLastSql = osql.str();
            int iRet = mysql_real_query(_pstMql, _sLastSql.c_str(), _sLastSql.length());
            if(iRet != 0){
                int iErrno = mysql_errno(_pstMql);
                if (iErrno == 2013 || iErrno == 2006){
                    connect();
                    iRet = mysql_real_query(_pstMql, _sLastSql.c_str(), _sLastSql.length());
                }
            }
            if (iRet != 0){
                return;
            }
        }
        
        LOG_INFO << "mysql connected "<<this;
        _bConnected = true;
    }
    
    void disconnect(){
        if (_pstMql != NULL){
            mysql_close(_pstMql);
            _pstMql = NULL;
        }
        
        LOG_INFO<<"mysql disconnected,"<<"_host:"<<_host<<" _port:"<<_port<<endl;
        LOG_INFO<<"mysql disconnected,"<<"_user:"<<_user<<" _password:"<<_password<<" _database:"<<_database<<endl;
        _bConnected = false;
    }
  
    string escapeString(const string& sFrom){
        if(!_bConnected){
            connect();
        }

        string::size_type iLen = sFrom.length() * 2 + 1;
        char *pTo = (char *)malloc(iLen);
        memset(pTo, 0x00, iLen);
        mysql_real_escape_string(_pstMql, pTo, sFrom.c_str(), sFrom.length());

        string sTo = pTo;
        free(pTo);
        return sTo;
    }
   
    template<class T>
    int getProto(const string& key, T& t){
        if(!_bConnected){
            connect();
        }
        
        ostringstream osql;
        osql << "SELECT udata FROM " <<  _table << " WHERE ukey='" << key << "'";
        _sLastSql = osql.str();
        int iRet = mysql_real_query(_pstMql, _sLastSql.c_str(), _sLastSql.length());
        if(iRet != 0){
            int iErrno = mysql_errno(_pstMql);
            if (iErrno == 2013 || iErrno == 2006){
                connect();  
                iRet = mysql_real_query(_pstMql, _sLastSql.c_str(), _sLastSql.length());
            }
        }
        if (iRet != 0){
            return MYSQL_EXECUTE;
        }
        
        MYSQL_RES *pstRes = mysql_store_result(_pstMql);
        if(pstRes == NULL){
            return MYSQL_STORE;
        }
        
        MYSQL_ROW stRow = mysql_fetch_row(pstRes);
        unsigned long * lengths = mysql_fetch_lengths(pstRes);
        if (stRow == NULL || lengths == NULL){
            mysql_free_result(pstRes);
            return MYSQL_NODATA;
        }

        iRet = t.ParseFromArray(stRow[0], lengths[0]);
        mysql_free_result(pstRes);
        return  iRet ? 0 : MYSQL_DECODE;
    }
    
    template<class T>
    int getProto(const string& key, T* t){
        if(!_bConnected){
            connect();
        }
        
        ostringstream osql;
        osql << "SELECT udata FROM " <<  _table << " WHERE ukey='" << key << "'";
        _sLastSql = osql.str();
        int iRet = mysql_real_query(_pstMql, _sLastSql.c_str(), _sLastSql.length());
        if(iRet != 0){
            int iErrno = mysql_errno(_pstMql);
            if (iErrno == 2013 || iErrno == 2006){
                connect();  
                iRet = mysql_real_query(_pstMql, _sLastSql.c_str(), _sLastSql.length());
            }
        }
        if (iRet != 0){
            return MYSQL_EXECUTE;
        }
        
        MYSQL_RES *pstRes = mysql_store_result(_pstMql);
        if(pstRes == NULL){
            return MYSQL_STORE;
        }
        
        MYSQL_ROW stRow = mysql_fetch_row(pstRes);
        unsigned long * lengths = mysql_fetch_lengths(pstRes);
        if (stRow == NULL || lengths == NULL){
            mysql_free_result(pstRes);
            return MYSQL_NODATA;
        }

        iRet = t->ParseFromArray(stRow[0], lengths[0]);
        mysql_free_result(pstRes);
        return  iRet ? 0 : MYSQL_DECODE;
    }

    template<class T>
    int setProto(const string& key, const T& t){
        ostringstream osql;
        string data = escapeString(t.SerializeAsString());
        osql << "REPLACE INTO " <<  _table << "(ukey, udata) VALUE ('" << key << "', '" << data << "')";
        
        if(!_bConnected){
            connect();
        }
        
        _sLastSql = osql.str();
        int iRet = mysql_real_query(_pstMql, _sLastSql.c_str(), _sLastSql.length());
        if(iRet != 0){
            int iErrno = mysql_errno(_pstMql);
            if (iErrno == 2013 || iErrno == 2006){
                connect(); 
                iRet = mysql_real_query(_pstMql, _sLastSql.c_str(), _sLastSql.length());
            }
        }

        return iRet;
    }

    int del(const string& key){
        ostringstream osql;
        osql << "DELETE FROM " <<  _table << " WHERE ukey='" << key << "'";
        if(!_bConnected){
            connect();
        }
        
        _sLastSql = osql.str();
        int iRet = mysql_real_query(_pstMql, _sLastSql.c_str(), _sLastSql.length());
        if(iRet != 0){
            int iErrno = mysql_errno(_pstMql);
            if (iErrno == 2013 || iErrno == 2006){
                connect();  
                iRet = mysql_real_query(_pstMql, _sLastSql.c_str(), _sLastSql.length());
            }
        }

        return iRet;
    }
    
    int runDelete(const string& command){
        LOG_INFO <<"runDelete command: "<<command;

        if(!_bConnected){
            connect();
            if(!_bConnected){
                LOG_ERROR << "mysql disconnected";
                return MYSQL_EXECUTE;
            }
        }

        ostringstream osql;
        osql << command;
        _sLastSql = osql.str();
        int iRet = mysql_real_query(_pstMql, _sLastSql.c_str(), _sLastSql.length());
        if(iRet != 0){
            int iErrno = mysql_errno(_pstMql);
            if (iErrno == 2013 || iErrno == 2006){
                connect();
                iRet = mysql_real_query(_pstMql, _sLastSql.c_str(), _sLastSql.length());
                if (iRet != 0){
                    LOG_ERROR << "mysql_real_query error iRet:"<<iRet<<"command:"<<command;
                    return MYSQL_EXECUTE;
                }
            }
        }

        return mysql_affected_rows(_pstMql);
    }

    int runCreate(const string& command){
        LOG_INFO <<"runCreate command: "<<command;

        if(!_bConnected){
            connect();
            if(!_bConnected){
                LOG_ERROR << "mysql disconnected";
                return MYSQL_EXECUTE;
            }
        }

        ostringstream osql;
        osql << command;
        _sLastSql = osql.str();
        int iRet = mysql_real_query(_pstMql, _sLastSql.c_str(), _sLastSql.length());
        if(iRet != 0){
            int iErrno = mysql_errno(_pstMql);
            if (iErrno == 2013 || iErrno == 2006){
                connect();
                iRet = mysql_real_query(_pstMql, _sLastSql.c_str(), _sLastSql.length());
                if (iRet != 0){
                    LOG_ERROR << "mysql_real_query error iRet:"<<iRet<<"command:"<<command;
                    return MYSQL_EXECUTE;
                }
            }
        }

        return mysql_affected_rows(_pstMql);
    }

    int runUpdate(const string& command){
        LOG_INFO <<"runUpdate command: "<<command;

        if(!_bConnected){
            connect();
            if(!_bConnected){
                LOG_ERROR << "mysql disconnected";
                return MYSQL_EXECUTE;
            }
        }

        ostringstream osql;
        osql << command;
        _sLastSql = osql.str();
        int iRet = mysql_real_query(_pstMql, _sLastSql.c_str(), _sLastSql.length());
        if(iRet != 0){
            int iErrno = mysql_errno(_pstMql);
            if (iErrno == 2013 || iErrno == 2006){
                connect();
                iRet = mysql_real_query(_pstMql, _sLastSql.c_str(), _sLastSql.length());
                if (iRet != 0){
                    LOG_ERROR << "mysql_real_query error iRet:"<<iRet<<"command:"<<command;
                    return MYSQL_EXECUTE;
                }
            }
        }

        return mysql_affected_rows(_pstMql);
    }

    int runInsert(const string& command){
        LOG_INFO <<"runInsert command: "<<command;

        if(!_bConnected){
            connect();
            if(!_bConnected){
                LOG_ERROR << "mysql disconnected";
                return MYSQL_EXECUTE;
            }
        }

        ostringstream osql;
        osql << command;
        _sLastSql = osql.str();
        int iRet = mysql_real_query(_pstMql, _sLastSql.c_str(), _sLastSql.length());
        if(iRet != 0){
            int iErrno = mysql_errno(_pstMql);
            if (iErrno == 2013 || iErrno == 2006){
                connect();
                iRet = mysql_real_query(_pstMql, _sLastSql.c_str(), _sLastSql.length());
                if (iRet != 0){
                    LOG_ERROR << "mysql_real_query error iRet:"<<iRet<<"command:"<<command;
                    return MYSQL_EXECUTE;
                }
            }
        }

        return mysql_insert_id(_pstMql);
    }

    int runSelectUnionAll(const string& command,vector<vector<string>> &rows,vector<string> &fields){
        LOG_INFO <<"runSelectUnionAll command: "<<command;
        rows.clear();
        fields.clear();

        if(!_bConnected){
            connect();
            if(!_bConnected){
                LOG_ERROR << "mysql disconnected";
                return MYSQL_EXECUTE;
            }
        }

        ostringstream osql;
        osql << command;
        _sLastSql = osql.str();
        int iRet = mysql_real_query(_pstMql, _sLastSql.c_str(), _sLastSql.length());
        if(iRet != 0){
            int iErrno = mysql_errno(_pstMql);
            if (iErrno == 2013 || iErrno == 2006){
                connect();
                iRet = mysql_real_query(_pstMql, _sLastSql.c_str(), _sLastSql.length());
                if (iRet != 0){
                    LOG_ERROR << "mysql_real_query error iRet:"<<iRet<<"command:"<<command;
                    return MYSQL_EXECUTE;
                }
            }
        }

        MYSQL_RES *pstRes = mysql_store_result(_pstMql);
        if(pstRes == NULL){
            LOG_ERROR << "mysql_store_result error command:"<<command;
            return MYSQL_STORE;
        }

        MYSQL_FIELD *flds = mysql_fetch_fields(pstRes);
        int fieldNum = mysql_num_fields(pstRes);
        for (int i = 0; i < fieldNum; ++i){
            fields.push_back(flds[i].name);
        }

        MYSQL_ROW row;
        while((row = mysql_fetch_row(pstRes))){
            vector<string> fs;
            for(int i=0;i<fieldNum;i++){
                if(NULL == row[i]){
                    fs.push_back(string(""));
                }else{
                    fs.push_back(string(row[i]));
                }
            }
            rows.push_back(fs);
        }

        if(rows.size() == 0 ){
            LOG_ERROR << "mysql_store_result error,no data, command:"<<command;
            mysql_free_result(pstRes);
            return MYSQL_NODATA;
        }

        mysql_free_result(pstRes);
        return  iRet==0 ? 0 : MYSQL_DECODE;
    }

    int runSelect(const string& command,vector<vector<string>> &rows,vector<string> &fields){
        LOG_INFO <<"runSelect command: "<<command;
        rows.clear();
        fields.clear();

        if(!_bConnected){
            connect();
            if(!_bConnected){
                LOG_ERROR << "mysql disconnected";
                return MYSQL_EXECUTE;
            }
        }

        ostringstream osql;
        osql << command;
        _sLastSql = osql.str();
        int iRet = mysql_real_query(_pstMql, _sLastSql.c_str(), _sLastSql.length());
        if(iRet != 0){
            int iErrno = mysql_errno(_pstMql);
            if (iErrno == 2013 || iErrno == 2006){
                connect();
                iRet = mysql_real_query(_pstMql, _sLastSql.c_str(), _sLastSql.length());
                if (iRet != 0){
                    LOG_ERROR << "mysql_real_query error iRet:"<<iRet<<"command:"<<command;
                    return MYSQL_EXECUTE;
                }
            }
        }

        MYSQL_RES *pstRes = mysql_store_result(_pstMql);
        if(pstRes == NULL){
            LOG_ERROR << "mysql_store_result error command:"<<command;
            return MYSQL_STORE;
        }

        MYSQL_FIELD *flds = mysql_fetch_fields(pstRes);
        int fieldNum = mysql_num_fields(pstRes);
        for (int i = 0; i < fieldNum; ++i){
            fields.push_back(flds[i].name);
        }

        MYSQL_ROW row;
        while((row = mysql_fetch_row(pstRes))){
            vector<string> fs;
            for(int i=0;i<fieldNum;i++){
                if(NULL == row[i]){
                    fs.push_back(string(""));
                }else{
                    fs.push_back(string(row[i]));
                }
            }
            rows.push_back(fs);
        }

        if(rows.size() == 0 ){
            LOG_INFO << "mysql_store_result error,no data, command:"<<command;
            mysql_free_result(pstRes);
            return MYSQL_NODATA;
        }

        mysql_free_result(pstRes);
        return  iRet==0 ? 0 : MYSQL_DECODE;
    }


    bool isKeyExist(const string& key){
        if(!_bConnected){
            connect();
        }

        ostringstream osql;
        osql << "SELECT udata FROM " <<  _table << " WHERE ukey='" << key << "'";
        _sLastSql = osql.str();
        int iRet = mysql_real_query(_pstMql, _sLastSql.c_str(), _sLastSql.length());
        if(iRet != 0){
            int iErrno = mysql_errno(_pstMql);
            if (iErrno == 2013 || iErrno == 2006){
                connect();  
                iRet = mysql_real_query(_pstMql, _sLastSql.c_str(), _sLastSql.length());
            }
        }
        if (iRet != 0){
            return false;
        }

        MYSQL_RES *pstRes = mysql_store_result(_pstMql);
        if(pstRes == NULL){
            return false;
        }

        bool bExist = pstRes->row_count > 0;
        mysql_free_result(pstRes);
        return bExist;
    }

    string getError(){
        string sErr = "";
        if (_pstMql != NULL && mysql_error(_pstMql) != NULL){
            sErr = string(mysql_error(_pstMql));
        }
        return sErr;
    }

    string getlastsql(){
        return _sLastSql;
    }
};

#endif
