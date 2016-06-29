/*
 * author:charlee
 */
#ifndef _ONSALE_HELPER_H_
#define _ONSALE_HELPER_H_

#include <thrift/transport/TSocket.h>
#include "JsonHelper.h"
#include "JsonMacro.h"
#include "System.h"
#include "ThriftHelper.h"
#include "OdbWrapperPool.h"
#include "RedisWrapperPool.h"
#include "sdk-cpp/ec/OnsaleECService.h"
#include "sdk-cpp/ec/onsale_types.h"
#include "sdk-cpp/ec/onsaleEC_types.h"
#include "OnsaleConst.hpp"
#include "OnsaleHelper.hpp"
#include "OnsaleUtil.hpp"
#include "OnsaleConvert.hpp"
#include "../../orm/onsale.hxx"
#include "./orm_onsale/onsale-odb.hxx"

using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace std;
using namespace boost;
using namespace boost::posix_time;
using namespace odb::core;
using namespace System;
using namespace onsale;
using namespace onsaleEC;
using namespace onsaleUtil;

class OnsaleHelper {
private:
    OdbWrapper* odbWrapper;
    Error::type error;
    string errmsg;

public:
    OnsaleHelper(OdbWrapper* odbWrapper){
        this->odbWrapper = odbWrapper;
        this->error = Error::FAILED;
        this->errmsg = "failed";
    }

    Error::type getError(){
        return this->error;
    }

    string getErrmsg(){
        return this->errmsg;
    }

    int createWareLabel(const WareLabel& tt_wareLabel){
        PerfWatch perfWatch("OnsaleHelper::createWareLabel");
        try{
            session s;
            transaction t (odbWrapper->begin());
            onsale_warelabel orm_warelabel;
            ThriftToOrm(tt_wareLabel, orm_warelabel);
            odbWrapper->persist(orm_warelabel);
            t.commit();
            return 0;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int updateWareLabel(const WareLabel& tt_wareLabel){
        PerfWatch perfWatch("OnsaleHelper::updateWareLabel");
        try{
            session s;
            transaction t (odbWrapper->begin());
            onsale_warelabel orm_warelabel;
            ThriftToOrm(tt_wareLabel, orm_warelabel);
            odbWrapper->update(orm_warelabel);
            t.commit();
            return 0;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int getWareLabelCount(const MultipleCondition& cond){
        PerfWatch perfWatch("OnsaleHelper::getWareLabelCount");
        try{
            session s;
            transaction t (odbWrapper->begin());
            auto ws (odbWrapper->query_value<onsale_warelabel_stat>(getCondition(cond)));
            t.commit();
            return ws.count;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int getWareLabelList(const MultipleCondition& cond,vector<WareLabel>& wareLabelList){
        PerfWatch perfWatch("OnsaleHelper::getWareLabelList");
        try{
            session s;
            transaction t (odbWrapper->begin());
            auto r (odbWrapper->query<onsale_warelabel>(getCondition(cond)));
            for(auto warelabel : r){
                WareLabel w;
                OrmToThrift(warelabel, w);
                wareLabelList.push_back(w);
            }
            t.commit();
            return 0;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int getWareLabelList(const set<unsigned int>& wareLabelIDSet, vector<WareLabel>& wareLabelList){
        PerfWatch perfWatch("OnsaleHelper::getWareLabelList");

        if(wareLabelIDSet.size( ) == 0){
            LOG_ERROR<<" wareLabelIDSet.size == 0"<< endl;
            return -1;
        }

        string in = "(";
        for(auto it=wareLabelIDSet.begin();it != wareLabelIDSet.end();it++){
            in += to_string(*it);
            auto iter = it;
            iter++;
            if(iter != wareLabelIDSet.end()){
                in += ",";
            }
        }
        in += ")";

        try{
            session s;
            transaction t (odbWrapper->begin());
            MultipleCondition cond;
            cond.andCondList.push_back(string("id in ") + in);
            auto r (odbWrapper->query<onsale_warelabel>(getCondition(cond)));
            for(auto warelabel : r){
                WareLabel w;
                OrmToThrift(warelabel, w);
                wareLabelList.push_back(w);
            }
            t.commit();
            return 0;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int getWareLabelWareList(const MultipleCondition& cond, vector<WareLabelWares>& wareLabelWareList){
        PerfWatch perfWatch("OnsaleHelper::getWareLabelList");

        try{
            session s;
            transaction t (odbWrapper->begin());
            auto wares (odbWrapper->query<onsale_warelabelwares>(getCondition(cond)));
            for(auto ware : wares){
                WareLabelWares w;
                OrmToThrift(ware, w);
                wareLabelWareList.push_back(w);
            }
            t.commit();
            return 0;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int getWareLabelWareList(const set<int64_t>& wareIDSet, vector<WareLabelWares>& wareLabelWareList){
        PerfWatch perfWatch("OnsaleHelper::getWareLabelList");

        if(wareIDSet.size( ) == 0){
            LOG_ERROR<<" wareIDList.size == 0"<< endl;
            return -1;
        }

        string in = "(";
        for(auto it=wareIDSet.begin();it != wareIDSet.end();it++){
            in += to_string(*it);
            auto iter = it;
            iter++;
            if(iter != wareIDSet.end()){
                in += ",";
            }
        }
        in += ")";

        try{
            session s;
            transaction t (odbWrapper->begin());
            MultipleCondition cond;
            cond.andCondList.push_back(string("ware_id in ") + in);
            auto wares (odbWrapper->query<onsale_warelabelwares>(getCondition(cond)));
            for(auto ware : wares){
                WareLabelWares w;
                OrmToThrift(ware, w);
                wareLabelWareList.push_back(w);
            }
            t.commit();
            return 0;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int getWareLabel(const MultipleCondition& cond,WareLabel& wareLabel){
        PerfWatch perfWatch("OnsaleHelper::getWareLabel");
        try{
            session s;
            transaction t (odbWrapper->begin ());
            auto label (odbWrapper->query_one<onsale_warelabel>(getCondition(cond)));
            t.commit();

            if(label.get() != NULL){
                OrmToThrift(*label, wareLabel);
                return 0;
            }else{
                this->error = Error::NO_DATA_YOU_EXPECTED;
                this->errmsg = "no data you expected";
                return -1;
            }
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int getWareLabelMaxId(){
        PerfWatch perfWatch("OnsaleHelper::getWareLabelMaxId");
        try{
            session s;
            transaction t (odbWrapper->begin());
            auto os (odbWrapper->query_value<onsale_warelabel_stat>());
            t.commit();
            return os.max_id;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int addWareLabelWare(WareLabelWares& wareLabelWare){
        PerfWatch perfWatch("OnsaleHelper::addWareLabelWare");
        try{
            session s;
            transaction t (odbWrapper->begin ());
            wareLabelWare.create_time = time(NULL);
            onsale_warelabelwares orm_ware;
            ThriftToOrm(wareLabelWare, orm_ware);
            odbWrapper->persist (orm_ware);
            t.commit();
            return 0;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int delWareLabelWare(const MultipleCondition& cond){
        PerfWatch perfWatch("OnsaleHelper::delWareLabelWare");
        try{
            session s;
            transaction t (odbWrapper->begin ());
            odbWrapper->erase_query<onsale_warelabelwares>(getCondition(cond));
            t.commit();
            return 0;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int getWareLabelWaresCount(const MultipleCondition& cond){
        PerfWatch perfWatch("OnsaleHelper::getWareLabelWaresCount");
        try{
            session s;
            transaction t (odbWrapper->begin ());
            auto ws (odbWrapper->query_value<onsale_warelabelwares_stat>(getCondition(cond)));
            t.commit();
            return ws.count;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int getWareLabelWaresList(const MultipleCondition& cond,vector<WareLabelWares>& wareLabelWaresList){
        PerfWatch perfWatch("OnsaleHelper::getWareLabelWaresList");
        try{
            session s;
            transaction t (odbWrapper->begin ());
            auto r (odbWrapper->query<onsale_warelabelwares>(getCondition(cond)));
            for(auto ware : r){
                WareLabelWares w;
                OrmToThrift(ware, w);
                wareLabelWaresList.push_back(w);
            }
            t.commit();
            return 0;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int addPresent(const Present& present){
        PerfWatch perfWatch("OnsaleHelper::addPresent");
        try{
            session s;
            transaction t (odbWrapper->begin());
            onsale_present orm_present;
            ThriftToOrm(present, orm_present);
            odbWrapper->persist(orm_present);
            t.commit();
            return 0;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int delPresent(const MultipleCondition& cond){
        PerfWatch perfWatch("OnsaleHelper::delPresent");
        try{
            session s;
            transaction t (odbWrapper->begin());
            odbWrapper->erase_query<onsale_present>(getCondition(cond));
            t.commit();
            return 0;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int getPresentList(const MultipleCondition& cond, vector<Present>& presentList){
        PerfWatch perfWatch("OnsaleHelper::getPresentList");
        try{
            session s;
            transaction t (odbWrapper->begin());
            auto presents (odbWrapper->query<onsale_present>(getCondition(cond)));
            for(auto present : presents){
                Present p;
                OrmToThrift(present, p);
                presentList.push_back(p);
            }
            t.commit();
            return 0;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int getPresentList(const std::set<unsigned int>& stepIdSet, vector<Present>& presentList){
        PerfWatch perfWatch("OnsaleHelper::getPresentList");

        if(stepIdSet.size( ) == 0){
            LOG_ERROR<<" stepIdSet.size == 0"<< endl;
            return -1;
        }

        string in = "(";
        for(auto it=stepIdSet.begin();it != stepIdSet.end();it++){
            in += to_string(*it);
            auto iter = it;
            iter++;
            if(iter != stepIdSet.end()){
                in += ",";
            }
        }
        in += ")";

        try{
            session s;
            transaction t (odbWrapper->begin());
            MultipleCondition cond;
            cond.andCondList.push_back(string("step_id in ") + in);
            auto presents (odbWrapper->query<onsale_present>(getCondition(cond)));
            for(auto present : presents){
                Present p;
                OrmToThrift(present, p);
                presentList.push_back(p);
            }
            t.commit();
            return 0;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int addStep(const Step& step){
        PerfWatch perfWatch("OnsaleHelper::addStep");
        try{
            session s;
            transaction t (odbWrapper->begin());
            onsale_step orm_step;
            ThriftToOrm(step, orm_step);
            odbWrapper->persist(orm_step);
            t.commit();
            return 0;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int delStep(const MultipleCondition& cond){
        PerfWatch perfWatch("OnsaleHelper::delStep");
        try{
            session s;
            transaction t (odbWrapper->begin());
            odbWrapper->erase_query<onsale_step>(getCondition(cond));
            t.commit();
            return 0;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int getStepList(const MultipleCondition& cond,vector<Step>& stepList){
        PerfWatch perfWatch("OnsaleHelper::getStepList");
        try{
            session s;
            transaction t (odbWrapper->begin());
            auto steps (odbWrapper->query<onsale_step>(getCondition(cond)));
            for(auto step : steps){
                Step s;
                OrmToThrift(step, s);
                stepList.push_back(s);
            }
            t.commit();
            return 0;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int createOnsaleGroup(const OnsaleGroup& tt_onsaleGroup){
        PerfWatch perfWatch("OnsaleHelper::createOnsaleGroup");
        try{
            session s;
            transaction t (odbWrapper->begin());
            onsale_onsalegroup orm_onsalegroup;
            ThriftToOrm(tt_onsaleGroup, orm_onsalegroup);
            odbWrapper->persist(orm_onsalegroup);
            t.commit();
            return 0;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int updateOnsaleGroup(const OnsaleGroup& tt_onsaleGroup){
        PerfWatch perfWatch("OnsaleHelper::updateOnsaleGroup");
        try{
            session s;
            transaction t (odbWrapper->begin());
            onsale_onsalegroup orm_onsalegroup;
            ThriftToOrm(tt_onsaleGroup, orm_onsalegroup);
            odbWrapper->update(orm_onsalegroup);
            t.commit();
            return 0;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int getOnsaleGroupCount(const MultipleCondition& cond){
        PerfWatch perfWatch("OnsaleHelper::getOnsaleGroupCount");
        try{
            session s;
            transaction t (odbWrapper->begin());
            auto ws (odbWrapper->query_value<onsale_onsalegroup_stat>(getCondition(cond)));
            t.commit();
            return ws.count;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int getOnsaleGroupMaxId(){
        PerfWatch perfWatch("OnsaleHelper::getOnsaleGroupMaxId");
        try{
            session s;
            transaction t (odbWrapper->begin());
            auto os (odbWrapper->query_value<onsale_onsalegroup_stat>());
            t.commit();
            return os.max_id;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int getOnsaleGroup(const MultipleCondition& cond,OnsaleGroup& onsaleGroup){
        PerfWatch perfWatch("OnsaleHelper::getOnsaleGroup");
        try{
            session s;
            transaction t (odbWrapper->begin());
            auto group (odbWrapper->query_one<onsale_onsalegroup>(getCondition(cond)));
            t.commit();

            if(group.get() != NULL){
                OrmToThrift(*group, onsaleGroup);
                return 0;
            }else{
                this->error = Error::NO_DATA_YOU_EXPECTED;
                this->errmsg = "no data you expected";
                return -1;
            }
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int getOnsaleGroupList(const MultipleCondition& cond,vector<OnsaleGroup>& onsaleGroupList){
        PerfWatch perfWatch("OnsaleHelper::getOnsaleGroupList");
        try{
            session s;
            transaction t (odbWrapper->begin());
            auto onsalegroups (odbWrapper->query<onsale_onsalegroup>(getCondition(cond)));
            for(auto onsalegroup : onsalegroups){
                OnsaleGroup og;
                OrmToThrift(onsalegroup, og);
                onsaleGroupList.push_back(og);
            }
            t.commit();
            return 0;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int getOnsaleGroupList(const vector<unsigned int>& wareLabelIdList,vector<OnsaleGroup>& onsaleGroupList){
        PerfWatch perfWatch("OnsaleHelper::getOnsaleGroupList");

        if(wareLabelIdList.size( ) == 0){
            LOG_ERROR<<" wareLabelIdList.size == 0"<< endl;
            return -1;
        }

        string in = "(";
        for(auto it=wareLabelIdList.begin();it != wareLabelIdList.end();it++){
            in += to_string(*it);
            auto iter = it;
            iter++;
            if(iter != wareLabelIdList.end()){
                in += ",";
            }
        }
        in += ")";

        try{
            session s;
            transaction t (odbWrapper->begin());
            MultipleCondition cond;
            cond.andCondList.push_back(string("ware_label_id in ") + in);
            auto onsalegroups (odbWrapper->query<onsale_onsalegroup>(getCondition(cond)));
            for(auto onsalegroup : onsalegroups){
                OnsaleGroup og;
                OrmToThrift(onsalegroup, og);
                onsaleGroupList.push_back(og);
            }
            t.commit();
            return 0;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }

    int queryWareLableWares(const set<unsigned int> ware_label_ids,map<unsigned int, set<int64_t>>& wareLabelId_wareIdSet_map){
        PerfWatch perfWatch("OnsaleHelper::queryWareLableWares");

        if(ware_label_ids.size( ) == 0){
            LOG_ERROR<<" ware_label_ids.size == 0"<< endl;
            return -1;
        }

        string in = "(";
        for(auto it=ware_label_ids.begin();it != ware_label_ids.end();it++){
            in += to_string(*it);
            auto iter = it;
            iter++;
            if(iter != ware_label_ids.end()){
                in += ",";
            }
        }
        in += ")";

        try{
            session s;
            transaction t (odbWrapper->begin());
            MultipleCondition cond;
            cond.andCondList.push_back(string("ware_label_id in ") + in);
            auto wares (odbWrapper->query<onsale_warelabelwares>(getCondition(cond)));
            for(auto ware : wares){
                wareLabelId_wareIdSet_map[ware.ware_label_id].insert(ware.ware_id);
            }
            t.commit();
            return 0;
        }catch (const odb::exception& e){
            LOG_ERROR<<"exception occurred : " << e.what() << endl;
            this->error = Error::FAILED;
            this->errmsg = e.what();
            return -1;
        }
    }
};

#endif
