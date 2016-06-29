#ifndef _COUPON_CONST_H_
#define _COUPON_CONST_H_

using namespace std;

const string coupon_default_config      =   "coupon_default_config";

const string coupon_group_table         =   "coupon_coupongroup";
const string user_coupons_table         =   "coupon_usercoupons";
const string coupon_table_prefix        =   "coupon_";
const string user_coupons_key_prefix    =   "coupon_group_";
const string ware_label_wares_table     =   "coupon_warelabelwares";
const string ware_label_table           =   "coupon_warelabel";
const string ware_label_wares_cache     =   "ware_label_wares_cache_";

const string cache_codes_prefix         =   "cache_codes_";
const int    cache_codes_capacity       =   1000;
const string cache_coupon_prefix        =   "cache_coupon_";

#define COUPON_BATCH_DISPATCH_LIST          "coupon_batch_dispatch_list"
#define COUPON_BATCH_EXPORT_LIST            "coupon_batch_export_list"

const double INACCURACY_UPLIMIT			=	0.5;//系统误差，人民币5毛钱
const double INACCURACY_RATE_UPLIMIT	=	0.05;//系统误差率，百分之五

const int CACHE_COUPON_EX				= 	180*24*3600;//缓存中券的生存期0.5年

//券不可用的原因类型
const int NOT_USABLE_TYPE_INVALID		=	0;//这个原因不参与排序
const int NOT_USABLE_TYPE_TIME			=	1;//时间没到
const int NOT_USABLE_TYPE_LIST			=	2;//商品不可用
const int NOT_USABLE_TYPE_MONEY			= 	3;//金额不满足

#endif
