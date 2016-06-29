/*
 * author:charlee
 */
#ifndef _MYSQL_QUERY_HELPER_H_
#define _MYSQL_QUERY_HELPER_H_

#include "mysqlwrapper.h"
#include "System.h"

using namespace std;

class MysqlQueryHelper{
private:
    string query_;
    MysqlWrapper* wrapper_;
    vector<vector<string>> rows_;
    vector<string> fields_;

public:
    MysqlQueryHelper(MysqlWrapper* wrapper){
        wrapper_ = wrapper;
    }

    ~MysqlQueryHelper(){
        query_ = "";
        rows_.clear();
        fields_.clear();
    }

    string getError(){
        return wrapper_->getError();
    }

    bool hasRow(){
        return this->rows_.size() > 0;
    }

    vector<vector<string>> getRows(){
        return this->rows_;
    }

    int runCreate(const string &query){
        double timestamp = ctime::microtime();

        this->query_ = query;
        rows_.clear();
        fields_.clear();
        int ret = wrapper_->runCreate(query_);

        LOG_WARN<<"runCreate cost "<<(ctime::microtime() - timestamp)/1000.0<<" ms"<<endl;
        return ret;
    }

    int runUpdate(const string &query){
        double timestamp = ctime::microtime();

        this->query_ = query;
        rows_.clear();
        fields_.clear();
        int ret=wrapper_->runUpdate(query_);

        LOG_WARN<<"runUpdate cost "<<(ctime::microtime() - timestamp)/1000.0<<" ms"<<endl;
        return ret;
    }

    int runInsert(const string &query){
        double timestamp = ctime::microtime();

        this->query_ = query;
        rows_.clear();
        fields_.clear();
        int ret = wrapper_->runInsert(query_);

        LOG_WARN<<"runInsert cost "<<(ctime::microtime() - timestamp)/1000.0<<" ms"<<endl;
        return ret;
    }

    int runDelete(const string &query){
        double timestamp = ctime::microtime();

        this->query_ = query;
        rows_.clear();
        fields_.clear();
        int ret = wrapper_->runDelete(query_);

        LOG_WARN<<"runDelete cost "<<(ctime::microtime() - timestamp)/1000.0<<" ms"<<endl;
        return ret;
    }

    int runSelect(const string &query){
        double timestamp = ctime::microtime();

        this->query_ = query;
        rows_.clear();
        fields_.clear();
        int ret=wrapper_->runSelect(query_,rows_,fields_);

        LOG_WARN<<"runSelect cost "<<(ctime::microtime() - timestamp)/1000.0<<" ms"<<endl;
        return ret;
    }

    int runSelectUnionAll(const string &query){
        double timestamp = ctime::microtime();

        this->query_ = query;
        rows_.clear();
        fields_.clear();
        int ret = wrapper_->runSelectUnionAll(query_,rows_,fields_);

        LOG_WARN<<"runSelectUnionAll cost "<<(ctime::microtime() - timestamp)/1000.0<<" ms"<<endl;
        return ret;
    }

    string getStringField(const string& fieldName,const unsigned int row = 0){
        unsigned int fields_size = this->fields_.size();
        if(this->rows_.size() >= row){
            for(unsigned int i=0;i<fields_size;i++){
                if (fields_[i]==fieldName){
                    return rows_[row][i];
                }
            }
        }

        return "";
    }

    int getIntegerField(const string& fieldName,const unsigned int row = 0){
        unsigned int fields_size = this->fields_.size();
        if(this->rows_.size() >= row){
            for(unsigned int i=0;i<fields_size;i++){
                if (fields_[i]==fieldName){
                    if(rows_[row][i] == ""){
                        return 0;
                    }

                    return atoi(rows_[row][i].c_str());
                }
            }
        }

        return 0;
    }

    int64_t getLongIntegerField(const string& fieldName,const unsigned int row = 0){
        unsigned int fields_size = this->fields_.size();
        if(this->rows_.size() >= row){
            for(unsigned int i=0;i<fields_size;i++){
                if (fields_[i]==fieldName){
                    if(rows_[row][i] == ""){
                        return 0;
                    }

                    return atol(rows_[row][i].c_str());
                }
            }
        }

        return 0;
    }

    double getDoubleField(const string& fieldName,const unsigned int row = 0){
        unsigned int fields_size = this->fields_.size();
        if(this->rows_.size() >= row){
            for(unsigned int i=0;i<fields_size;i++){
                if (fields_[i]==fieldName){
                    if(rows_[row][i] == ""){
                        return 0.0;
                    }

                    return atof(rows_[row][i].c_str());
                }
            }
        }

        return 0.0;
    }
};

#endif
