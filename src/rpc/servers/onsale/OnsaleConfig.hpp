#ifndef _COUPON_CONFIG_
#define _COUPON_CONFIG_
#include <string>
#include "JsonHelper.h"

using namespace std;

class OnsaleConfig {
private:
    string conf_path_           ;

    string  redis_ip_           ;
    int     redis_port_         ;
    int     redis_timeout_      ;

    string  mysql_ip_           ;
    int     mysql_port_         ;
    string  mysql_user_         ;
    string  mysql_passwd_       ;
    string  mysql_database_     ;

    string  log_path_           ;

    int     ec_port_            ;
    int     ec_io_threads_num_  ;
    int     ec_task_threads_num_;

    int     admin_port_         ;

public:
    OnsaleConfig(){
        conf_path_      = "";

        redis_ip_       = "127.0.0.1";
        redis_port_     = 6379;
        redis_timeout_  = 500;

        mysql_ip_       = "127.0.0.1";
        mysql_port_     = 3306;
        mysql_user_     = "root";
        mysql_passwd_   = "";
        mysql_database_ = "ecsite";

        log_path_ = "/data/deploy/logs/";

        ec_port_        = 19001;
        ec_io_threads_num_ = 2;
        ec_task_threads_num_ = 8;

        admin_port_     = 19002;
    }

    void Parse(string conf){
        conf_path_ = conf;

        JsonHelper jh(conf.c_str());
        redis_ip_               = jh.Root["onsale_redis_server"]["ip"].GetString();
        redis_port_             = jh.Root["onsale_redis_server"]["port"].GetInt();
        redis_timeout_          = jh.Root["onsale_redis_server"]["timeout"].GetInt();

        mysql_ip_               = jh.Root["onsale_mysql_server"]["ip"].GetString();
        mysql_port_             = jh.Root["onsale_mysql_server"]["port"].GetInt();
        mysql_user_             = jh.Root["onsale_mysql_server"]["user"].GetString();
        mysql_passwd_           = jh.Root["onsale_mysql_server"]["password"].GetString();
        mysql_database_         = jh.Root["onsale_mysql_server"]["dbname"].GetString();

        log_path_               = jh.Root["onsale_log_path"]["path"].GetString();

        ec_port_                = jh.Root["onsale_ec_server"]["port"].GetInt();
        ec_io_threads_num_      = jh.Root["onsale_ec_server"]["ioThreadsNum"].GetInt();
        ec_task_threads_num_    = jh.Root["onsale_ec_server"]["taskThreadsNum"].GetInt();

        admin_port_             = jh.Root["onsale_admin_server"]["port"].GetInt();
    }

    string getConfPath(){return this->conf_path_;}

    string getRedisIp(){return this->redis_ip_;}
    int getRedisPort(){return this->redis_port_;}
    int getRedisTimeout(){return this->redis_timeout_;}

    string getMysqlIp(){return this->mysql_ip_;}
    int getMysqlPort(){return this->mysql_port_;}
    string getMysqlUser(){return this->mysql_user_;}
    string getMysqlPasswd(){return this->mysql_passwd_;}
    string getMysqlDatabase(){return this->mysql_database_;}

    string getLogPath(){return this->log_path_;}

    int getECPort(){return this->ec_port_;}
    int getECIoThreadsNum(){
        if(this->ec_io_threads_num_ <= 0){
            this->ec_io_threads_num_ = 1;
        }

        return this->ec_io_threads_num_;
    }
    int getECTaskThreadsNum(){
        if(this->ec_task_threads_num_ <= 0){
            this->ec_task_threads_num_ = 1;
        }

        return this->ec_task_threads_num_;
    }

    int getAdminPort(){return this->admin_port_;}

    void Print(){
        cout<<"this server will connect to mysql..."<<endl;
        cout<<"ip = "<< getMysqlIp()<<endl;
        cout<<"port = "<< getMysqlPort()<<endl;
        cout<<"username = "<< getMysqlUser()<<endl;
        cout<<"passwd = "<< getMysqlPasswd()<<endl;
        cout<<"database = "<< getMysqlDatabase()<<endl;

        cout<<"this server will connect to redis..."<<endl;
        cout<<"ip = "<< getRedisIp()<<endl;
        cout<<"port = "<< getRedisPort()<<endl;
        cout<<"timeout = "<< getRedisTimeout()<<endl;

        cout<<"related configration..."<<endl;
        cout<<"ec_io_threads_num = "<< getECIoThreadsNum()<<endl;
        cout<<"ec_task_threads_num = "<< getECTaskThreadsNum()<<endl;
    }
};

#endif
