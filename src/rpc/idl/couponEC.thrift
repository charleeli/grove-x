include "coupon.thrift"

namespace cpp couponEC
namespace py couponEC

struct GetDefaultConfigReq {                                        //默认配置
    1: required bool dummy,                                         //哑字段,任意填写
}

struct GetDefaultConfigRsp {                                        //默认配置
    1: coupon.Error error,                                          //错误码
    2: string errmsg,                                               //错误消息
    3: coupon.DefaultConfig config,                                 //默认配置
}

struct DecodeCodeReq {                                              //解析券码请求
    1: required string code,                                        //券码
}

struct DecodeCodeRsp {                                              //解析券码响应
    1: coupon.Error error,                                          //错误码
    2: string errmsg,                                               //错误消息
    3: string code,                                                 //券码
    4: i32 coupon_group_id,                                         //优惠券组ID
    5: i32 coupon_id,                                               //优惠券ID
}

struct CheckArgotReq {                                              //判断口令是否有效
    1: required string argot,                                       //口令
}

struct CheckArgotRsp {                                              //判断口令是否有效
    1: coupon.Error error,                                          //错误码
    2: string errmsg,                                               //错误消息
    3: string argot,                                                //口令
    4: i32 coupon_group_id,                                         //优惠券组ID
}

struct UserLeftCouponsCountReq {                                    //剩余可领取券数量
    1: required i32 coupon_group_id,                                //优惠券组ID
    2: required i64 user_id,                                        //用户ID
}

struct UserLeftCouponsCountRsp {                                    //剩余可领取券数量
    1: coupon.Error error,                                          //错误码
    2: string errmsg,                                               //错误消息
    3: i32 left_count,                                              //剩余可领取券数量
}

struct DrawnCountTodayReq {                                         //今天领取券数量
    1: required i32 coupon_group_id,                                //优惠券组ID
    2: required i64 user_id,                                        //用户ID
}

struct DrawnCountTodayRsp {                                         //今天领取券数量
    1: coupon.Error error,                                          //错误码
    2: string errmsg,                                               //错误消息
    3: i32 count,                                                   //今天领取券数量
}

struct ClientCouponsReq {                                           //某个client_id领取券数量请求
    1: required i32 coupon_group_id,                                //优惠券组ID
    2: required string client_id,                                   //客户端ID
}

struct ClientCouponsRsp {                                           //某个client_id领取券数量响应
    1: coupon.Error error,                                          //错误码
    2: string errmsg,                                               //错误消息
    3: i32 count,                                                   //券数量
}

struct DrawReq {                                                    //领券请求
    1: required i32 coupon_group_id,                                //优惠券组ID
    2: required i64 user_id,                                        //用户ID
    3: optional string client_id,                                   //客户端ID
    4: optional string argot,                                       //口令
    5: required i32 scene_type,                                     //场景类型,填0即可
    6: optional string code,                                        //券码,批量导出型券会用到
    7: optional i32 channel_id,                                     //渠道id
}

struct DrawRsp {                                                    //领券响应
    1: coupon.Error error,                                          //错误码
    2: string errmsg,                                               //错误消息
    3: string code,                                                 //券码
    4: coupon.CacheCoupon cacheCoupon,                              //详细信息
}

struct RollbackReq {                                                //回退券请求
    1: required string code,                                        //券码
    2: required i64 user_id,                                        //用户ID
    3: required string order_id,                                    //订单编号
}

struct RollbackRsp {                                                //回退券响应
    1: coupon.Error error,                                          //错误码
    2: string errmsg,                                               //错误消息
}

struct FreezeReq {                                                  //冻结券请求
    1: required string code,                                        //券码
    2: required i64 user_id,                                        //用户ID
    3: required string order_id,                                    //订单编号
}

struct FreezeRsp {                                                  //冻结券响应
    1: coupon.Error error,                                          //错误码
    2: string errmsg,                                               //错误消息
}

struct PayReq {                                                     //支付券请求
    1: required string code,                                        //券码
    2: required i64 user_id,                                        //用户ID
    3: required string order_id,                                    //订单编号
}

struct PayRsp {                                                     //支付券响应
    1: coupon.Error error,                                          //错误码
    2: string errmsg,                                               //错误消息
}

struct UserCouponsReq {
    1: required i64 user_id,                                        //用户ID
    2: required i32 page,                                           //第几页 1...
    3: required i32 count,                                          //每页数量
    4: required i32 use_status,                                     //有效状态,(0, '已领取'), (1, '已冻结'), (2, '已支付'),(3, '已过期')
}

struct UserCouponsRsp {
    1: coupon.Error error,                                          //错误码
    2: string errmsg,                                               //错误消息
    3: i64 user_id,                                                 //用户ID
    4: i32 page ,                                                   //第几页 1...
    5: i32 count,                                                   //每页数量
    6: i32 use_status,                                              //有效状态，(0, '已领取'), (1, '已冻结'), (2, '已支付'),(3, '已过期')
    7: i32 total,                                                   //总数
    8: list<coupon.CacheCoupon> coupons,                            //券列表
}

struct WareActsReq {
    1: required set<i64> ware_ids,                                  //商品id列表
}

struct WareActsRsp {
    1: coupon.Error error,                                          //错误码
    2: string errmsg,                                               //错误消息
    3: map<i64,list<coupon.CouponGroup>> wareActs,                  //ware_id --> CouponGroup列表
}

struct PriceInfo {                                                  //价格信息
    1: double market_price,                                         //市场价（反弹回去）
    2: double sale_price,                                           //售价
    3: double pay_price,                                            //实际支付金额
    4: double favor_price,                                          //优惠金额
}

struct SkuInfo {                                                    //sku 信息
    1: i64 sku_id,                                                  //sku ID
    2: i32 sku_count,                                               //sku 数量
    3: double sale_price,                                           //售价
    4: double market_price,                                         //市场价（反弹回去）
    5: i64 ware_id,                                                 //ware_id（反弹回去）
}

struct SellerInfo {                                                 //卖家信息
    1: i64 seller_id,                                               //卖家id
    2: map<i64,SkuInfo> skus,                                       //sku id --> sku 信息
}

struct UsableCouponsReq {                                           //可用优惠券请求
    1: required i64 user_id,                                        //用户ID
    2: required map<i64, SellerInfo> sellers,                       //卖家id --> 卖家信息
}

struct UsableCouponsRsp {                                           //可用优惠券响应
    1: coupon.Error error,                                          //错误码
    2: string errmsg,                                               //错误消息
    3: i64 user_id,                                                 //用户ID
    4: map<i64, SellerInfo> sellers,                                //卖家id --> 卖家信息
    
    5: list<coupon.CacheCoupon> platUsableCoupons,                  //可用的平台券列表（针对整个订单）
    6: map<i64,list<coupon.CacheCoupon>> sellerUsableCoupons,       //按卖家分的可用的店铺券    
    7: map<string,coupon.CacheCoupon>   code_cacheCoupon_map,       //按券码统计的本订单可用的券（5/6的汇总）

    8: PriceInfo orderPrice,                                        //订单级价格信息
    9: map<i64,PriceInfo> sellerId_priceInfo_map,                   //卖家级价格信息 
    10: map<i64,map<i64,PriceInfo>> sellerId_skuId_priceInfo_map,   //sku级价格信息
    
    11: map<string,double> platCode_totalSalePrice_map,             //平台券 --> 订单级的享受该平台券的商品总售价
    12: map<i64,map<string,double>> sellerId_code_totalSalePrice_map,//(卖家，券) --> 卖家级享受该券的商品集合的总售价
    13: map<i64,map<i64,map<string,double>>> sellerId_skuId_code_totalSalePrice_map,//(卖家，sku，券) --> sku级享受该券的售价
    
    14: map<string,set<i64>> platCode_wareSet_map,                  //平台券 --> 订单级的商品ware集合
    15: map<i64,map<string,set<i64>>> sellerId_code_wareSet_map,    //(卖家，券) --> 商品ware集合
    16: map<i64,map<i64,map<string,bool>>> sellerId_wareId_code_map,//(卖家，ware，券) --> 是否存在
    
    17: map<i64,i64> skuID_wareID_map,                              //skuID所属的wareID
    18: list<string> usableCodes,                                   //可用的券码（5/6的汇总）
}

struct NotUsableCouponsReq {                                        //不可用优惠券请求
    1: required i64 user_id,                                        //用户ID
    2: required map<i64, SellerInfo> sellers,                       //卖家id --> 卖家信息
    3: required list<string> usableCodes,                           //可用的券码（UsableCouponsRsp 5/6的汇总）
    4: required i32 page ,                                          //第几页 1...
    5: required i32 count,                                          //每页数量
}

struct NotUsableCouponsRsp {                                        //不可用优惠券响应
    1: coupon.Error error,                                          //错误码
    2: string errmsg,                                               //错误消息
    3: i64 user_id,                                                 //用户ID
    4: map<i64, SellerInfo> sellers,                                //卖家id --> 卖家信息
    5: list<string> usableCodes,                                    //可用的券码（UsableCouponsRsp 5/6的汇总）
    6: list<coupon.CacheCoupon> notUsableCoupons,                   //不可用的券列表
    7: i32 page ,                                                   //第几页 1...
    8: i32 count,                                                   //每页数量
    9: i32 total,                                                   //总数
}

struct ApportionReq {                                               //分摊请求
    1: required i64 user_id,                                        //用户ID
    2: required map<i64, SellerInfo> sellers,                       //卖家id --> 卖家信息
    
    3: optional string plat_code,                                   //用户选择的一张平台券
    4: optional map<i64,string> sellerId_code_map,                  //每个卖家可选一张店铺券
}

struct ApportionRsp {                                               //分摊响应
    1: coupon.Error error,                                          //错误码
    2: string errmsg,                                               //错误消息
    3: UsableCouponsRsp usableCouponsRsp,                           //可用优惠券响应
    4: map<string,coupon.CacheCoupon> toggle_cacheCoupon_map,       //用户勾选的券的信息
    5: map<i64,list<coupon.CacheCoupon>> sellerId_cacheCoupon_map,  //分摊计算后卖家可用的券set{平台券，店铺券}
}

struct CacheCouponGroupsReq {                                       //获取券组请求
    1: required set<i32> coupon_group_id_set ,                      //券组id
}

struct CacheCouponGroupsRsp {                                       //获取券组响应
    1: coupon.Error error,                                          //错误码
    2: string errmsg,                                               //错误消息
    3: list<coupon.CacheCouponGroup> cacheCouponGroupList ,         //券组列表
}

struct RelatedCouponGroupsReq {                                     //相关的券组请求
    1: required i32 coupon_group_id,                                //券组id
}

struct RelatedCouponGroupsRsp {                                     //相关的券组响应
    1: coupon.Error error,                                          //错误码
    2: string errmsg,                                               //错误消息
    3: list<coupon.CouponGroup> couponGroupList ,                   //券组列表
}

struct WareLabelReq {                                               //标签信息请求
    1: required i32 ware_label_id,                                  //标签id
}

struct WareLabelRsp {                                               //标签信息响应
    1: coupon.Error error,                                          //错误码
    2: string errmsg,                                               //错误消息
    3: coupon.WareLabel wareLabel,                                  //标签信息
    4: list<coupon.WareLabelWares> waresList,                       //商品信息
}

struct ServerTimeReq {                                              //服务器时间请求
    1: i32 local_time,                                              //本地时间
}

struct ServerTimeRsp {                                              //服务器时间响应
    1: coupon.Error error,                                          //错误码
    2: string errmsg,                                               //错误消息
    3: i32 local_time,                                              //本地时间
    4: i32 server_time,                                             //rpc服务器时间
}

service CouponECService {
    //默认配置
    GetDefaultConfigRsp getDefaultConfig(1: GetDefaultConfigReq getDefaultConfigReq) throws (1:coupon.InvalidOperation ouch)

    //今天已经领取券数量
    DrawnCountTodayRsp getDrawnCountToday(1: DrawnCountTodayReq drawnCountTodayReq) throws (1:coupon.InvalidOperation ouch)

    //用户剩余可领取券数量
    UserLeftCouponsCountRsp getUserLeftCouponsCount(1: UserLeftCouponsCountReq userLeftCouponsCountReq) throws (1:coupon.InvalidOperation ouch)

    //某个client_id的券
    ClientCouponsRsp getClientCoupons(1: ClientCouponsReq clientCouponsReq) throws (1:coupon.InvalidOperation ouch)

    //解析券码
    DecodeCodeRsp decodeCode(1: DecodeCodeReq decodeCodeReq) throws (1:coupon.InvalidOperation ouch)

    //判断口令是否有效
    CheckArgotRsp checkArgot(1: CheckArgotReq checkArgotReq) throws (1:coupon.InvalidOperation ouch)
    
    //领券
    DrawRsp drawCoupon(1:DrawReq drawReq) throws (1:coupon.InvalidOperation ouch)
    
    //冻结优惠券，提交订单
    FreezeRsp freezeCoupon(1:FreezeReq freezeReq) throws (1:coupon.InvalidOperation ouch)
    
    //解冻，回退优惠券，取消订单
    RollbackRsp rollbackCoupon(1:RollbackReq rollbackReq) throws (1:coupon.InvalidOperation ouch)

    //支付优惠券
    PayRsp payCoupon(1:PayReq payReq) throws (1:coupon.InvalidOperation ouch)
    
    //用户的优惠券列表
    UserCouponsRsp getUserCoupons(1:UserCouponsReq userCouponsReq) throws (1:coupon.InvalidOperation ouch)
    
    //商品对应的优惠券活动
    WareActsRsp getWareActs(1:WareActsReq wareActsReq) throws (1:coupon.InvalidOperation ouch)
    
    //根据购买商品 计算 用户可以使用的优惠券列表
    UsableCouponsRsp getUsableCoupons(1: UsableCouponsReq usableCouponsReq) throws (1:coupon.InvalidOperation ouch)

    //根据购买商品 计算 用户不可以使用的优惠券列表
    NotUsableCouponsRsp getNotUsableCoupons(1: NotUsableCouponsReq notUsableCouponsReq) throws (1:coupon.InvalidOperation ouch)
    
    //分摊计算
    ApportionRsp getApportion(1:ApportionReq apportionReq) throws (1:coupon.InvalidOperation ouch)
    
    //获取券组
    CacheCouponGroupsRsp getCacheCouponGroups(1: CacheCouponGroupsReq cacheCouponGroupsReq) throws (1:coupon.InvalidOperation ouch)

    //获取相关的券组
    RelatedCouponGroupsRsp getRelatedCouponGroups(1: RelatedCouponGroupsReq relatedCouponGroupsReq) throws (1:coupon.InvalidOperation ouch)
    
    //获取标签信息
    WareLabelRsp getWareLabel(1: WareLabelReq wareLabelReq) throws (1:coupon.InvalidOperation ouch)
    
    //获取服务器时间
    ServerTimeRsp getServerTime(1: ServerTimeReq serverTimeReq) throws (1:coupon.InvalidOperation ouch)
}
