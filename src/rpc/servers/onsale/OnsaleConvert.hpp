/*
 * author:charlee
 */

#ifndef _ONSALE_CONVERT_H_
#define _ONSALE_CONVERT_H_

#include <thrift/transport/TSocket.h>
#include "JsonHelper.h"
#include "JsonMacro.h"
#include "ThriftHelper.h"
#include "OdbWrapperPool.h"
#include "RedisWrapperPool.h"
#include "sdk-cpp/ec/OnsaleECService.h"
#include "sdk-cpp/ec/onsale_types.h"
#include "sdk-cpp/ec/onsaleEC_types.h"
#include "OnsaleConst.hpp"
#include "OnsaleHelper.hpp"
#include "OnsaleUtil.hpp"
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
using namespace onsale;
using namespace onsaleEC;
using namespace onsaleUtil;

namespace onsaleUtil {
std::time_t to_time_t(ptime pt){
    time_duration dur = pt - ptime(boost::gregorian::date(1970,1,1));
    return std::time_t(dur.total_seconds()) - 8*3600;
}

ptime from_time_t(std::time_t t){
    ptime start(boost::gregorian::date(1970,1,1));
    return start + seconds(static_cast<long>(t + 8*3600));
}
}

void OrmToThrift(const onsale_warelabel& orm_label, WareLabel& tt_label){
    tt_label.name           = orm_label.name;
    tt_label.label_type     = orm_label.label_type;
    tt_label.scope_type     = orm_label.scope_type;
    tt_label.sub_type       = orm_label.sub_type;
    tt_label.seller_id      = orm_label.seller_id;
    tt_label.seller_slug    = orm_label.seller_slug;
    tt_label.ware_label_id  = orm_label.id;
    tt_label.create_time    = onsaleUtil::to_time_t(orm_label.create_time);
    tt_label.update_time    = onsaleUtil::to_time_t(orm_label.update_time);
    tt_label.create_man     = orm_label.create_man;
}

void ThriftToOrm(const WareLabel& tt_label, onsale_warelabel& orm_label){
    orm_label.name           = tt_label.name;
    orm_label.label_type     = tt_label.label_type;
    orm_label.scope_type     = tt_label.scope_type;
    orm_label.sub_type       = tt_label.sub_type;
    orm_label.seller_id      = tt_label.seller_id;
    orm_label.seller_slug    = tt_label.seller_slug;
    orm_label.id             = tt_label.ware_label_id;
    orm_label.create_time    = onsaleUtil::from_time_t(tt_label.create_time);
    orm_label.update_time    = onsaleUtil::from_time_t(tt_label.update_time);
    orm_label.create_man     = tt_label.create_man;
}

void OrmToThrift(const onsale_warelabelwares& orm_ware, WareLabelWares& tt_ware){
    tt_ware.ware_label_id   = orm_ware.ware_label_id;
    tt_ware.ware_id         = orm_ware.ware_id;
    tt_ware.create_time     = onsaleUtil::to_time_t(orm_ware.create_time);
}

void ThriftToOrm(const WareLabelWares& tt_ware,onsale_warelabelwares& orm_ware){
    orm_ware.ware_label_id   = tt_ware.ware_label_id;
    orm_ware.ware_id         = tt_ware.ware_id;
    orm_ware.create_time     = onsaleUtil::from_time_t(tt_ware.create_time);
}

void OrmToThrift(const onsale_present& orm_present, Present& tt_present){
    tt_present.step_id          = orm_present.step_id;
    tt_present.sku_id           = orm_present.sku_id;
    tt_present.sku_price        = orm_present.sku_price;
    tt_present.sku_count        = orm_present.sku_count;
    tt_present.sku_slug         = orm_present.sku_slug;
}

void ThriftToOrm(const Present& tt_present,onsale_present& orm_present){
    orm_present.step_id          = tt_present.step_id;
    orm_present.sku_id           = tt_present.sku_id;
    orm_present.sku_price        = tt_present.sku_price;
    orm_present.sku_count        = tt_present.sku_count;
    orm_present.sku_slug         = tt_present.sku_slug;
}

void OrmToThrift(const onsale_step& orm_step, Step& tt_step){
    tt_step.onsale_group_id     = orm_step.onsale_group_id;
    tt_step.full_credit         = orm_step.full_credit;
    tt_step.favor_credit        = orm_step.favor_credit;
    tt_step.favor_rate          = orm_step.favor_rate;
    tt_step.full_count          = orm_step.full_count;
    tt_step.full_rate           = orm_step.full_rate;
    tt_step.full_price          = orm_step.full_price;
    tt_step.step_id             = orm_step.id;
}

void ThriftToOrm(const Step& tt_step,onsale_step& orm_step){
    orm_step.onsale_group_id     = tt_step.onsale_group_id;
    orm_step.full_credit         = tt_step.full_credit;
    orm_step.favor_credit        = tt_step.favor_credit;
    orm_step.favor_rate          = tt_step.favor_rate;
    orm_step.full_count          = tt_step.full_count;
    orm_step.full_rate           = tt_step.full_rate;
    orm_step.full_price          = tt_step.full_price;
    orm_step.id                  = tt_step.step_id;
}

void OrmToThrift(const onsale_onsalegroup& orm_onsalegroup, OnsaleGroup& tt_onsaleGroup){
    tt_onsaleGroup.id               = orm_onsalegroup.id;
    tt_onsaleGroup.slug             = orm_onsalegroup.slug;
    tt_onsaleGroup.name             = orm_onsalegroup.name;
    tt_onsaleGroup.title            = orm_onsalegroup.title;
    tt_onsaleGroup.comment          = orm_onsalegroup.comment;
    tt_onsaleGroup.favor_type       = orm_onsalegroup.favor_type;
    tt_onsaleGroup.label_type       = orm_onsalegroup.label_type;
    tt_onsaleGroup.ware_label_id    = orm_onsalegroup.ware_label_id;
    tt_onsaleGroup.involve_count    = orm_onsalegroup.involve_count;
    tt_onsaleGroup.start_time       = onsaleUtil::to_time_t(orm_onsalegroup.start_time);
    tt_onsaleGroup.end_time         = onsaleUtil::to_time_t(orm_onsalegroup.end_time);
    tt_onsaleGroup.create_time      = onsaleUtil::to_time_t(orm_onsalegroup.create_time);
    tt_onsaleGroup.update_time      = onsaleUtil::to_time_t(orm_onsalegroup.update_time);
    tt_onsaleGroup.verify_status    = orm_onsalegroup.verify_status;
    tt_onsaleGroup.applicant        = orm_onsalegroup.applicant;
    tt_onsaleGroup.approver         = orm_onsalegroup.approver;
    tt_onsaleGroup.modifier         = orm_onsalegroup.modifier;
    tt_onsaleGroup.jump_label       = orm_onsalegroup.jump_label;
    tt_onsaleGroup.jump_data        = orm_onsalegroup.jump_data;
}

void ThriftToOrm(const OnsaleGroup& tt_onsaleGroup,onsale_onsalegroup& orm_onsalegroup){
    orm_onsalegroup.id               = tt_onsaleGroup.id;
    orm_onsalegroup.slug             = tt_onsaleGroup.slug;
    orm_onsalegroup.name             = tt_onsaleGroup.name;
    orm_onsalegroup.title            = tt_onsaleGroup.title;
    orm_onsalegroup.comment          = tt_onsaleGroup.comment;
    orm_onsalegroup.favor_type       = tt_onsaleGroup.favor_type;
    orm_onsalegroup.label_type       = tt_onsaleGroup.label_type;
    orm_onsalegroup.ware_label_id    = tt_onsaleGroup.ware_label_id;
    orm_onsalegroup.involve_count    = tt_onsaleGroup.involve_count;
    orm_onsalegroup.start_time       = onsaleUtil::from_time_t(tt_onsaleGroup.start_time);
    orm_onsalegroup.end_time         = onsaleUtil::from_time_t(tt_onsaleGroup.end_time);
    orm_onsalegroup.create_time      = onsaleUtil::from_time_t(tt_onsaleGroup.create_time);
    orm_onsalegroup.update_time      = onsaleUtil::from_time_t(tt_onsaleGroup.update_time);
    orm_onsalegroup.verify_status    = tt_onsaleGroup.verify_status;
    orm_onsalegroup.applicant        = tt_onsaleGroup.applicant;
    orm_onsalegroup.approver         = tt_onsaleGroup.approver;
    orm_onsalegroup.modifier         = tt_onsaleGroup.modifier;
    orm_onsalegroup.jump_label       = tt_onsaleGroup.jump_label;
    orm_onsalegroup.jump_data        = tt_onsaleGroup.jump_data;
}

#endif
