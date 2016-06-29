namespace cpp onsale
namespace py onsale
namespace java com.meila.thrift.sdk.onsale.onsale

enum Error {                                //错误码
    OK = 0,                                 //成功
    FAILED = -10001,                        //失败
    NO_DATA_YOU_EXPECTED = -10002,          //没有数据
    MYSQL_DISCONNECTED= -10003 ,            //mysql连接断开了
    MYSQL_EXECUTE_ERROR = -10004,           //mysql执行时出错
    REDIS_DISCONNECTED= -10005,             //redis连接断开了
    REDIS_EXECUTE_ERROR = -10006,           //redis执行时出错
    COUNT_OVERLOAD = -10007 ,               //数量超限治
    WARE_LABEL_NOT_EXISTS = -10008,         //商品标签不存在
    GROUP_NOT_EXISTS = -10009,              //活动组不存在
    INVALID_WARE_LABEL_ID = -10010,         //不合法的商品标签id
    INVALID_SUB_TYPE = -10011,              //不合法的商品标签子类型
    NO_USABLE_GROUP = -10012,               //没有可用的活动组
    WARE_LABEL_ID_REUSE = -10013,           //商品标签被复用
    INVALID_RATE = -10014,                  //折扣率必须小于10
    INVALID_STEP_FULL = -10015,             //阶梯满额或满件数量重复
    NO_STEP = -10016,                       //没有任何阶梯
    INVALID_TIME = -10017,                  //审批时间不对
}

exception InvalidOperation {                //异常
    1: Error fault,                         //异常错误
    2: string why,                          //异常原因
}

struct MultipleCondition {                  //复合条件
    1: list<string> andCondList,            //各个条件之间and     
    2: list<string> orCondList,             //各个条件之间or
    3: list<string> orderCondList,          //排序条件
    4: string limitCond,                    //limit条件
}

struct WareLabel {                          //商品标签
    1: string name,                         //名称
    2: i32 label_type,                      //商品标签类型 (0, '单商品标签'), (1, '多商品标签')
    3: i32 scope_type,                      //使用范围 (0, '全平台'), (1, '店铺内')
    4: i32 sub_type,                        //子类型 (0, '商品白名单'), (1, '商品黑名单'), (2, '全部商品')
    5: i64 seller_id,                       //店铺id
    6: string seller_slug,                  //店铺slug
    7: i32 create_time,                     //创建时间
    8: i32 update_time,                     //更新时间
    9: i32 ware_label_id,                   //标签id
    10: string create_man,                  //创建人
}

struct WareLabelWares {                     //商品标签-商品表
    1: i32 ware_label_id,                   //商品标签id
    2: i64 ware_id,                         //商品id
    3: i32 create_time,                     //创建时间
}

struct Present {                            //赠品表
    1: i32 step_id,                         //阶梯ID
    2: i64 sku_id,                          //商品ID
    3: double sku_price,                    //商品价格
    4: i32 sku_count,                       //商品个数
    5: string sku_slug,                     //商品slug
}

struct Step {                               //优惠阶梯
    1: i32 onsale_group_id,                 //促销活动ID
    2: double full_credit,                  //购满金额
    3: double favor_credit,                 //免减金额
    4: double favor_rate,                   //打折折扣率
    5: i32 full_count,                      //购满件数
    6: double full_rate,                    //一件的优惠率,用于满N件优惠
    7: double full_price,                   //金额,用于N件任买
    8: i32 step_id,                         //阶梯id
}

struct OnsaleGroup {                        //促销活动表
    1: i32 id,                              //促销活动ID
    2: string slug,                         //slug
    3: string name,                         //name
    4: string title,                        //促销活动文案
    5: string comment,                      //促销活动备注
    6: i32 favor_type,                      //优惠类型 [(0, '满额减'), (1, '满额折'), (2, '满额换'), (3, '满件换'), (4, '满N件优惠'), (5, 'N件任买'), ]
    7: i32 label_type,                      //商品标签类型 (0, '单商品标签'), (1, '多商品标签')
    8: i32 involve_count,                   //参与次数
    9: i32 ware_label_id,                   //商品标签ID
    10: i32 start_time,                     //活动开始时间
    11: i32 end_time,                       //活动结束时间
    12: i32 create_time,                    //创建时间
    13: i32 update_time,                    //更新时间
    14: i32 verify_status,                  //审批状态
    15: string applicant,                   //申请人
    16: string approver,                    //审批人
    17: string modifier,                    //修改人
    18: string jump_label,                  //跳转类型
    19: string jump_data,                   //跳转数据
}
