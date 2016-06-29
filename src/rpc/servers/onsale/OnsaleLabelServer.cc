#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <ev.h>
#include <iostream>
#include <exception>
#include <memory>
#include "GLogHelper.h"
#include "Singleton.h"
#include "ThriftHelper.h"
#include "OdbWrapperPool.h"
#include "RedisWrapperPool.h"
#include "sdk-cpp/admin/onsaleAdmin_types.h"
#include "sdk-cpp/admin/OnsaleAdminService.h"
#include "OnsaleConfig.hpp"
#include "OnsaleConst.hpp"
#include "OnsaleHelper.hpp"
#include "Slug8.h"
#include "System.h"
extern "C"{
#include "redismq/redismq.h"
}

using namespace std;
using namespace System;
using namespace odb::core;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace onsale;
using namespace onsaleAdmin;

OnsaleConfig onsaleConfig;

static void OnExitSignal(int){
    LOG_ERROR<<"onsale label server exit!"<<endl;
    cout<<"onsale label server exit!"<<endl;
    exit(-1);
}

void IgnoreSignal(){
    signal(SIGHUP,SIG_IGN);
    signal(SIGPIPE,SIG_IGN);
    signal(SIGCHLD,SIG_IGN);
    signal(SIGINT,OnExitSignal);
    signal(SIGQUIT,OnExitSignal);
    signal(SIGTERM,OnExitSignal);
}

void PrintHelp(const char* program_name){
    std::cout << "Usage:" << program_name << " conf_path." << std::endl;
}

void blpop_cb(char *msg){
    OdbWrapperPool* opool = OdbWrapperPool::GetInstance(
        onsaleConfig.getMysqlIp(),
        onsaleConfig.getMysqlPort(),
        onsaleConfig.getMysqlUser(),
        onsaleConfig.getMysqlPasswd(),
        onsaleConfig.getMysqlDatabase(),
        2
    );

    RedisWrapperPool* rpool = RedisWrapperPool::GetInstance(
        onsaleConfig.getRedisIp(),
        onsaleConfig.getRedisPort(),
        onsaleConfig.getRedisTimeout(),
        2
    );

    CreateNameListReq createNameListReq;
    InvalidOperation io;
    int ret = 0;

    try{
        JSONToThrift(string(msg),&createNameListReq);
        LOG_INFO << "createNameListReq.ware_label_id : "<<createNameListReq.ware_label_id
            <<" createNameListReq.sub_type : "<<createNameListReq.sub_type<<endl;

        OdbWrapperGuard owrapperGuard(opool,opool->GetOdbWrapper());
        OdbWrapper* odbWrapper = owrapperGuard.getOdbWrapper();
        OnsaleHelper onsaleHelper(odbWrapper);
        LOG_INFO <<"opool zise:" <<opool->Size()<<" odbWrapper:"<<odbWrapper;
        if(NULL == odbWrapper){
            LOG_ERROR<<"odb connection disconnected"<< endl;
            io.fault = Error::MYSQL_DISCONNECTED;
            io.why = "odb connection disconnected";
            throw io;
        }

        RedisWrapperGuard rwrapperGuard(rpool,rpool->GetRedisWrapper());
        RedisWrapper* rwrapper = rwrapperGuard.getRedisWrapper();
        LOG_INFO <<"rpool zise:" <<rpool->Size()<<" rwrapper:"<<rwrapper;
        if(NULL == rwrapper||rwrapper->isClosed()){
            LOG_ERROR<<"Redis connection disconnected "<<rwrapper->getError()<< endl;
            io.fault = Error::REDIS_DISCONNECTED;
            io.why = "Redis connection disconnected";
            throw io;
        }

        //ware_label_id所对应的活动
        MultipleCondition cond_og;
        cond_og.andCondList.push_back(string("ware_label_id = ") + to_string(createNameListReq.ware_label_id));
        OnsaleGroup onsaleGroup;
        ret = onsaleHelper.getOnsaleGroup(cond_og, onsaleGroup);
        if(0 != ret){
            LOG_ERROR<<"failed when getOnsaleGroup"<< endl;
            io.fault = onsaleHelper.getError();
            io.why = onsaleHelper.getErrmsg();
            throw io;
        }

        if (createNameListReq.sub_type == 0 || createNameListReq.sub_type == 2) {//白名单/全部商品
            /*
                白名单/全部商品,直接判断有效性后入库;全部商品属于一个特殊白名单
                增加一些白名单商品,每个增加的直接判断有效性后入库
                删除一些白名单商品,直接删除
            */

            vector<string> members;//白名单集合/全部名单集合
            string  namelist_key = onsale_namelist_prefix[createNameListReq.sub_type] + to_string(createNameListReq.ware_label_id);
            ret = rwrapper->smembers(members, namelist_key);
            if (0 != ret) {
                LOG_ERROR<<"Redis smembers failed. namelist_key : "<<namelist_key<< endl;
                io.fault = Error::FAILED;
                io.why = "Redis smembers failed";
                throw io;
            }

            int size = members.size();
            for(int i=0; i<size; i++){
                //查询ware_id在商品标签-商品表中涉及的ware_label_id
                MultipleCondition cond;
                cond.andCondList.push_back(string("ware_id = ") + members[i]);

                vector<WareLabelWares> wareLabelWaresList;
                ret = onsaleHelper.getWareLabelWaresList(cond, wareLabelWaresList);

                vector<unsigned int> wareLabelIdList;
                for(auto x:wareLabelWaresList){
                    //非createNameListReq.ware_label_id的集合
                    if(createNameListReq.ware_label_id != x.ware_label_id){
                        wareLabelIdList.push_back(x.ware_label_id);
                    }
                }

                //查询这些ware_label_id涉及的onsaleGroup
                int onlineGroupCount = 0;//涉及在线促销活动数量
                vector<OnsaleGroup> onsaleGroupList;
                ret = onsaleHelper.getOnsaleGroupList(wareLabelIdList, onsaleGroupList);
                if (onsaleGroupList.size() > 0) {
                    for(int i=0; i<(int)onsaleGroupList.size(); i++){
                        if(onsaleGroupList[i].start_time < time(NULL) && time(NULL) < onsaleGroupList[i].end_time
                            && onsaleGroupList[i].ware_label_id != createNameListReq.ware_label_id
                        ){
                            onlineGroupCount++ ;
                        }else if((onsaleGroupList[i].start_time < onsaleGroup.start_time
                                    && onsaleGroup.start_time < onsaleGroupList[i].end_time)
                                ||(onsaleGroupList[i].start_time < onsaleGroup.end_time
                                    && onsaleGroup.end_time < onsaleGroupList[i].end_time)
                        ){
                            onlineGroupCount++ ;
                        }
                    }
                }

                if (onlineGroupCount >= 1) {
                    continue;
                }

                WareLabelWares ware;
                ware.ware_label_id = createNameListReq.ware_label_id;
                ware.ware_id = atol(members[i].c_str());

                ret = onsaleHelper.addWareLabelWare(ware);
                LOG_INFO <<"ret:"<<ret << " ware_label_id:"<<ware.ware_label_id<<" ware_id:"<<ware.ware_id<<endl;
            }
        }else if (createNameListReq.sub_type == 1) {//黑名单
            /*
                增加一些黑名单商品,直接删除
                删除一些黑名单商品,每个增加的直接判断有效性后入库
            */

            //如果对应黑名单不存在
            string blacklist_key = onsale_namelist_prefix[1] + to_string(createNameListReq.ware_label_id);
            int count = rwrapper->scard(blacklist_key);
            if (count <= 0) {
                LOG_ERROR <<blacklist_key<< "not exists"<<" count : " <<count<<endl;
                io.fault = Error::FAILED;
                io.why = blacklist_key + "not exists";
                throw io;
            }

            //如果对应基准名单不存在
            string datumlist_key = onsale_namelist_prefix[3] + to_string(createNameListReq.ware_label_id);
            count = rwrapper->scard(datumlist_key);
            if (count <= 0) {
                LOG_ERROR <<datumlist_key<< "not exists"<<" count : " <<count<<endl;
                io.fault = Error::FAILED;
                io.why = datumlist_key + "not exists";
                throw io;
            }

            string  templist_key = onsale_namelist_templist_prefix + to_string(createNameListReq.ware_label_id);
            ret = rwrapper->sdiffstore(templist_key, datumlist_key, blacklist_key);
            if(0 != ret){
                LOG_ERROR << "sdiffstore failed, ware_label_id : "<<createNameListReq.ware_label_id<<endl;
            }else{
                vector<string> members;
                ret = rwrapper->smembers(members, templist_key);
                if (0 == ret) {
                    for(int i=0; i<(int)members.size(); i++){
                        //查询ware_id在商品标签-商品表中涉及的ware_label_id
                        MultipleCondition cond;
                        cond.andCondList.push_back(string("ware_id = ") + members[i]);

                        vector<WareLabelWares> wareLabelWaresList;
                        ret = onsaleHelper.getWareLabelWaresList(cond, wareLabelWaresList);

                        vector<unsigned int> wareLabelIdList;
                        for(auto x:wareLabelWaresList){
                            //非createNameListReq.ware_label_id的集合
                            if(createNameListReq.ware_label_id != x.ware_label_id){
                                wareLabelIdList.push_back(x.ware_label_id);
                            }
                        }

                        //查询这些ware_label_id涉及的onsaleGroup
                        int onlineGroupCount = 0;//涉及在线促销活动数量
                        vector<OnsaleGroup> onsaleGroupList;
                        ret = onsaleHelper.getOnsaleGroupList(wareLabelIdList, onsaleGroupList);
                        if (onsaleGroupList.size() > 0) {
                            for(int i=0; i<(int)onsaleGroupList.size(); i++){
                                if(onsaleGroupList[i].start_time < time(NULL) && time(NULL) < onsaleGroupList[i].end_time
                                    && onsaleGroupList[i].ware_label_id != createNameListReq.ware_label_id
                                ){
                                    onlineGroupCount++ ;
                                }else if((onsaleGroupList[i].start_time < onsaleGroup.start_time
                                            && onsaleGroup.start_time < onsaleGroupList[i].end_time)
                                        ||(onsaleGroupList[i].start_time < onsaleGroup.end_time
                                            && onsaleGroup.end_time < onsaleGroupList[i].end_time)
                                ){
                                    onlineGroupCount++ ;
                                }
                            }
                        }

                        if (onlineGroupCount >= 1) {
                            continue;
                        }

                        WareLabelWares ware;
                        ware.ware_label_id = createNameListReq.ware_label_id;
                        ware.ware_id = atol(members[i].c_str());

                        ret = onsaleHelper.addWareLabelWare(ware);
                        LOG_INFO <<"ret:"<<ret << " ware_label_id:"<<ware.ware_label_id<<" ware_id:"<<ware.ware_id<<endl;
                    }
                }
            }
        }
    }catch(InvalidOperation &io){
        LOG_ERROR << "createNameListReq.ware_label_id : "<<createNameListReq.ware_label_id
            << " createNameListReq.sub_type : "<<createNameListReq.sub_type
            << " InvalidOperation exception caught: " <<" fault: "<< io.fault<<" why: "<<io.why << endl;
    }catch (TException& tx) {
        LOG_ERROR << "createNameListReq.ware_label_id : "<<createNameListReq.ware_label_id
            << " createNameListReq.sub_type : "<<createNameListReq.sub_type
            << " ERROR: " << tx.what() << endl;
    }catch (std::exception& e){
        LOG_ERROR << "createNameListReq.ware_label_id : "<<createNameListReq.ware_label_id
            << " createNameListReq.sub_type : "<<createNameListReq.sub_type
            << " exception caught: " << e.what() << endl;
    }catch (...){
        LOG_ERROR << "createNameListReq.ware_label_id : "<<createNameListReq.ware_label_id
            << " createNameListReq.sub_type : "<<createNameListReq.sub_type
            << " exception caught..." << endl;
    }
}

int main(int argc, char *argv[]){
    if (2 != argc){
        PrintHelp(argv[0]);
        return -1;
    }

    onsaleConfig = Singleton<OnsaleConfig>::instance();
    onsaleConfig.Parse(string(argv[1]));
    onsaleConfig.Print();

    string redis_ip    = onsaleConfig.getRedisIp();
    int redis_port     = onsaleConfig.getRedisPort();

    int pos = string(argv[0]).find_last_of('/');
    string program = string(argv[0]).substr(pos+1);
    GLogHelper gh(program,onsaleConfig.getLogPath());

    cout << "onsale label server running..." << endl;
    LOG_INFO<<"Starting the onsale label server..."<<endl;
    LOG_INFO<<"this->redis_ip_: "<<redis_ip<<" this->redis_port_:"<<redis_port<<endl;

    IgnoreSignal();
    //DaemonInit();

    struct rmq_context pop;
    rmq_init(&pop, redis_ip.c_str(), redis_port, 0, ONSALE_LABEL_LIST);
    rmq_blpop(&pop, blpop_cb);
    ev_loop(EV_DEFAULT_ 0);

    cout << "Done." << endl;
    return 0;
}
