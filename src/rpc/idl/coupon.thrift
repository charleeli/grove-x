namespace cpp coupon
namespace py coupon
namespace java com.meila.thrift.sdk.coupon.coupon

struct DefaultConfig {                      //默认配置
    1: string default_jump_label,           //默认跳转类型
    2: string default_jump_data,            //默认按钮跳转
    3: string default_url,                  //默认连接
    4: bool   default_switch,               //默认开关
    5: string default_text,                 //默认文本
}

struct WareLabel {                          //商品标签 
    1: string name,                         //名称
    2: i32 scope_type,                      //使用范围  
    3: i32 sub_type,                        //子类型
    4: i32 create_time,                     //创建时间
    5: i32 update_time,                     //更新时间
    6: i32 ware_label_id,                   //标签id
    7: string create_man,                   //创建人
}

struct WareLabelWares {                     //商品标签-商品表
    1: i32 ware_label_id,                   //商品标签id
    2: i64 ware_id,                         //商品id
    3: string ware_slug,                    //商品slug
}

struct WareLabelWaresCache {
    1: list<WareLabelWares> cache,          //商品标签-商品 记录缓存
}

struct CouponGroup {                        //券组
    1: string name,                         //名称
    2: string title,                        //标题
    3: string comment,                      //描述
    4: i32 ware_label_id,                   //商品标签ID
    5: i32 favor_type,                      //优惠类型
    6: i32 scope_type,                      //使用范围
    7: i32 sub_type,                        //子类型
    8: i32 scene_type,                      //场景类型
    9: double full,                         //满
    10: double favor,                       //减
    11: double rate,                        //折扣率
    12: string argot,                       //暗语
    13: i32 max_count,                      //放出最大数量
    14: i32 delta,                          //追加增量
    15: i32 drawn_count,                    //已领取数量
    16: i32 payed_count,                    //已支付数量
    17: i32 create_time,                    //创建时间
    18: i32 can_draw_count,                 //每个人允许领取的数量
    19: i32 start_draw_time,                //领取有效期开始
    20: i32 end_draw_time,                  //领取有效期结束
    21: bool is_duration_type,              //是否持续
    22: i32 duration_value,                 //持续的天数
    23: i32 start_use_time,                 //开始使用
    24: i32 end_use_time,                   //结束使用
    25: i32 verify_status,                  //审核状态
    26: string applicant,                   //申请人
    27: string approver,                    //审批人
    28: string modifier,                    //修改人
    29: i64 seller_id,                      //店铺
    30: string url,                         //连接
    31: i32 id,                             //券组ID
    32: string img,                         //图片
    33: string slug,                        //slug
    34: i32 delta_verify_status,            //追加增量审核状态
    35: i32 img_width,                      //图片宽
    36: i32 img_height,                     //图片高
    37: string button_text,                 //按钮文字
    38: string button_jump,                 //按钮跳转
    39: string jump_label,                  //跳转类型
    40: string jump_data,                   //按钮跳转
    41: string argot_jump_label,            //口令跳转类型
    42: string argot_jump_data,             //口令按钮跳转
    43: i32 update_time,                    //更新时间
    44: i32 version                         //版本号
    45: string description                  //优惠券组适用范围描述
}

struct Coupon {                             //券信息
    1: i32 coupon_group_id,                 //优惠券组ID
    2: string code,                         //券码
    3: i64 user_id,                         //用户ID
    4: string order_id,                     //订单编号
    5: i32 create_time,                     //创建时间戳
    6: i32 order_create_time,               //订单生成时间
    7: i32 drawn_time,                      //被领取时间
    8: i32 frozen_time,                     //被冻结时间
    9: i32 payed_time,                      //被支付时间
    10: i32 coupon_id,                      //券ID          
    
}

struct UserCoupon {                         //用户-券表
    1: i64 user_id,                         //用户ID
    2: i32 coupon_group_id,                 //券组
    3: i32 coupon_id,                       //券
    4: string code,                         //券码
    5: i32 create_time,                     //创建时间
    6: i32 update_time,                     //更新时间
    7: i32 use_status,                      //使用状态
    8: string client_id,                    //客户端ID
    9: i64 seller_id,                       //卖家id
    10: i32 channel_id,                     //渠道id
}

struct CacheCoupon {                        //缓存券信息
    1: i32 coupon_group_id,                 //优惠券组ID
    2: string code,                         //券码
    3: i64 user_id,                         //用户ID
    4: string order_id,                     //订单编号
    5: i32 create_time,                     //创建时间戳
    6: i32 order_create_time,               //订单生成时间
    7: i32 drawn_time,                      //被领取时间
    8: i32 frozen_time,                     //被冻结时间
    9: i32 payed_time,                      //被支付时间
    10: i32 update_time,                    //更新时间
    11: i32 use_status,                     //使用状态
    12: i32 start_use_time,                 //使用有效期开始
    13: i32 end_use_time,                   //使用有效期结束
    14: i32 coupon_id,                      //券ID  
    15: string name,                        //名称
    16: string title,                       //标题
    17: string comment,                     //描述
    18: i32 ware_label_id,                  //商品标签ID
    19: i32 favor_type,                     //优惠类型
    20: i32 scope_type,                     //使用范围
    21: i32 sub_type,                       //子类型
    22: i32 scene_type,                     //场景类型
    23: double full,                        //满
    24: double favor,                       //减
    25: double rate,                        //折扣率
    26: string argot,                       //暗语
    27: i64 seller_id,                      //店铺
    28: string url,                         //连接
    29: string img,                         //图片
    30: string client_id,                   //客户端ID
    31: string slug,                        //slug
    32: i32 img_width,                      //图片宽
    33: i32 img_height,                     //图片高
    34: string button_text,                 //按钮文字
    35: string button_jump,                 //按钮跳转
    36: string jump_label,                  //跳转类型
    37: string jump_data,                   //按钮跳转
    38: string argot_jump_label,            //口令跳转类型
    39: string argot_jump_data,             //口令按钮跳转
    40: i32 version                         //版本号
    41: string description                  //优惠券组适用范围描述


    100: i32 type                           //不能使用的原因类型,1:时间没开始 2:名单 3:购买不够
    101: double value                       //原因对应的具体值
}

struct CacheCouponGroup {                   //券组
    1: string name,                         //名称
    2: string title,                        //标题
    3: string comment,                      //描述
    4: i32 ware_label_id,                   //商品标签ID
    5: i32 favor_type,                      //优惠类型
    6: i32 scope_type,                      //使用范围
    7: i32 sub_type,                        //子类型
    8: i32 scene_type,                      //场景类型
    9: double full,                         //满
    10: double favor,                       //减
    11: double rate,                        //折扣率
    12: string argot,                       //暗语
    13: i32 max_count,                      //放出最大数量
    14: i32 drawn_count,                    //已领取数量
    15: i32 create_time,                    //创建时间
    16: i32 can_draw_count,                 //每个人允许领取的数量
    17: i32 start_draw_time,                //领取有效期开始
    18: i32 end_draw_time,                  //领取有效期结束
    19: i64 seller_id,                      //店铺
    20: string url,                         //链接
    21: i32 id,                             //券组ID
    22: string img,                         //图片
    23: string slug,                        //slug
    24: i32 img_width,                      //图片宽
    25: i32 img_height,                     //图片高
    26: string button_text,                 //按钮文字
    27: string button_jump,                 //按钮跳转
    28: string jump_label,                  //跳转类型
    29: string jump_data,                   //按钮跳转
    30: string argot_jump_label,            //口令跳转类型
    31: string argot_jump_data,             //口令按钮跳转
    32: i32 update_time,                    //更新时间
    33: i32 version                         //版本号
    34: string description                  //优惠券组适用范围描述
}

struct CacheCodes {                         //缓存中该用户的所有券码
    1: list<string> codes,                  //该用户的券码
}

enum Error {                                //错误码
    OK = 0,                                 //成功
    FAILED = -10001                         //失败
    NO_DATA_YOU_EXPECTED = -10002           //没有数据    
    MYSQL_DISCONNECTED= -10003              //mysql连接断开了
    MYSQL_EXECUTE_ERROR = -10004            //mysql执行时出错
    REDIS_DISCONNECTED= -10005              //redis连接断开了
    REDIS_EXECUTE_ERROR = -10006            //redis执行时出错
    COUPON_GROUP_NOT_EXISTS = -10007        //优惠券组不存在
    USER_NOT_EXISTS = -10008                //用户不存在
    SCENE_TYPE_NOT_EXISTS = -10009          //错误的场景类型
    NO_COUPON_LEFT = -10010                 //没有剩余的券可以领取
    NOT_IN_DRAW_TIME = -10011               //不在领取时间内
    DRAWN_ALL_YOU_CAN = -10012              //能领取的都领完了
    CODE_ENCODE_ERROR = -10013              //优惠券编码错误
    CODE_DECODE_ERROR = -10014              //优惠券反算错误
    INVALID_PARAMS = -10015                 //无效的参数
    USER_HAVENOT_THE_CODE = -10016          //用户没有该券
    NOT_BEEN_FROZEN = -10017                //不处于被冻结状态
    COUPON_ID_NOT_EXISTS = -10018           //优惠券ID不存在
    ORDER_ID_NOT_MATCH = -10019             //订单编号不匹配
    COUPON_IN_USE = -10020                  //券已经在使用中了
    COUPON_EXPIRED = -10021                 //券已经过期了
    COUPON_NOT_USABLE = -10022              //券不能使用
    COUPON_NOT_PLATFORM = -10023            //不是平台券
    COUPON_NOT_SHOPTYPE = -10024            //不是店铺券
    COUPON_NOT_OF_THIS_SHOP = -10025        //该店铺券不属于该店铺
    CAN_NOT_GET_THE_LOCK = -10026           //不能获得锁
    INVALID_ARGOT = -10027                  //无效的口令
    OUTLOAD_COUNT = -10028                  //数量过大
    ILLEGAL_SCENE_TYPE = -10029             //不合法的场景类型
    COUPON_NOT_EXISTS = -10030              //券不存在
    INVALID_CODE = -10031                   //无效的券码
    COUPON_HAS_BEEN_DRAWN = -10032          //券已经被自己领取了
    COUPON_HAS_BEEN_ROBBED = -10033         //券已经被他人领取了
    ARGOT_IN_180_DAYS = -10034              //口令180天内使用过
    DRAW_TIME_NOT_REACH = -10035            //券领取时间没到
    DRAW_TIME_EXPIRED = -10036              //券领取时间过期
    NO_USABLE_COUPON = -10037               //没有可用的券
}

exception InvalidOperation {                //异常
    1: Error fault,                         //异常错误
    2: string why                           //异常原因
}

struct MultipleCondition {                  //复合条件
    1: list<string> andCondList,            //各个条件之间and     
    2: list<string> orCondList,             //各个条件之间or
    3: list<string> orderCondList,          //排序条件
}
