include "coupon.thrift"

namespace cpp couponPush
namespace py couponPush
namespace java com.meila.thrift.sdk.coupon.couponPush

struct PushInfo {                       //被推送的信息
    1: i64 user_id,                     //用户ID
    2: double amount,                   //该用户一天内过期的券的总金额
}

struct NameListReq {                    //获取推送名单请求
    1: required i32 page,               //第几页 1 2 3...
    2: required i32 count,              //每页数量,尽量不要超过100
}

struct NameListRsp {                    //获取推送名单响应
    1: coupon.Error error,              //错误码
    2: string errmsg,                   //错误消息 

    3: i32 page,                        //第几页 1 2 3...
    4: i32 count,                       //每页数量
    5: i32 total,                       //总数量
    
    6: list<PushInfo> pushInfoList,     //推送列表
}

struct PushInfoReq {                    //获取推送信息请求
    1: required set<i64> user_id_set,   //用户ID集合
}

struct PushInfoRsp {                    //获取推送信息响应
    1: coupon.Error error,              //错误码
    2: string errmsg,                   //错误消息 
    3: list<PushInfo> pushInfoList,     //推送列表
}

service CouponPushService {
    //获取推送名单
    NameListRsp getNameList(1: NameListReq nameListReq) throws (1:coupon.InvalidOperation ouch)
    
    //获取推送信息
    PushInfoRsp getPushInfo(1: PushInfoReq pushInfoReq) throws (1:coupon.InvalidOperation ouch)
}
