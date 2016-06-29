include "onsale.thrift"

namespace cpp onsaleAdmin
namespace py onsaleAdmin

struct AddCacheNameListReq {                                    //添加缓存名单
    1: required i32 ware_label_id,                              //商品标签ID
    2: required i32 sub_type,                                   //名单类型 (0, '白名单'), (1, '黑名单'), (2, '全部'), (3, '黑名单基准')
    3: required list<i64> ware_id_list,                         //ware_id列表
}

struct AddCacheNameListRsp {                                    //添加缓存名单
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
}

struct DelCacheNameListReq {                                    //删除缓存名单
    1: required i32 ware_label_id,                              //商品标签ID
    2: required i32 sub_type,                                   //名单类型 (0, '白名单'), (1, '黑名单'), (2, '全部'), (3, '黑名单基准')
    3: required list<i64> ware_id_list,                         //ware_id列表
}

struct DelCacheNameListRsp {                                    //删除缓存名单
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
}

struct GetCacheNameListReq {                                    //查看缓存名单
    1: required i32 ware_label_id,                              //商品标签ID
    2: required i32 sub_type,                                   //名单类型 (0, '白名单'), (1, '黑名单'), (2, '全部'), (3, '黑名单基准')
}

struct GetCacheNameListRsp {                                    //查看缓存名单
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
    3: list<i64> ware_id_list,                                  //ware_id列表
}

struct CheckCacheNameListReq {                                  //检查缓存名单
    1: required i32 ware_label_id,                              //商品标签ID
    2: required i32 sub_type,                                   //名单类型 (0, '白名单'), (1, '黑名单'), (2, '全部')
}

struct CheckCacheNameListRsp {                                  //检查缓存名单
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
}

struct ViewCheckCacheNameListReq {                              //查看检查缓存名单
    1: required i32 ware_label_id,                              //商品标签ID
    2: required i32 sub_type,                                   //名单类型 (0, '白名单'), (1, '黑名单'), (2, '全部')
}

struct ViewCheckCacheNameListRsp {                              //查看检查缓存名单
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
    3: list<i64> ware_id_list,                                  //ware_id列表
    4: i32 timestamp,                                           //本次检查的时间戳
}

struct LatestTimestampReq {                                     //最近一次检测时间
    1: required i32 ware_label_id,                              //商品标签ID
}

struct LatestTimestampRsp {                                     //查看检查缓存名单
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
    3: i32 timestamp,                                           //本次检查的时间戳
}

struct CreateNameListReq {                                      //创建名单(真正入库)
    1: required i32 ware_label_id,                              //商品标签ID
    2: required i32 sub_type,                                   //名单类型 (0, '白名单'), (1, '黑名单'), (2, '全部名单')
}

struct CreateNameListRsp {                                      //创建名单(真正入库)
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
    3: i32 time_cost,                                           //预计耗时/毫秒
}

struct CreateOnsaleGroupReq {                                   //创建促销活动
    1: required onsale.OnsaleGroup onsaleGroup,                 //促销活动
}

struct CreateOnsaleGroupRsp {                                   //创建促销活动
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
}

struct UpdateOnsaleGroupReq {                                   //更新促销活动
    1: required onsale.OnsaleGroup onsaleGroup,                 //促销活动
}

struct UpdateOnsaleGroupRsp {                                   //更新促销活动
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
}

struct OnsaleGroupCountReq {                                    //查询符合条件的促销活动数目
    1: optional onsale.MultipleCondition cond,                  //查询条件
}

struct OnsaleGroupCountRsp {                                    //查询符合条件的促销活动数目
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
    3: i32 count,                                               //条数
}

struct OnsaleGroupListReq {                                     //查看促销活动
    1: optional onsale.MultipleCondition cond,                  //查询条件
}

struct OnsaleGroupListRsp {                                     //查看促销活动
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
    3: list<onsale.OnsaleGroup> onsaleGroupList,                //促销活动
}

struct AddStepReq {                                             //添加阶梯
    1: required list<onsale.Step> stepList,                     //阶梯
}

struct AddStepRsp {                                             //添加阶梯
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
}

struct DelStepReq {                                             //删除阶梯
    1: optional onsale.MultipleCondition cond,                  //查询条件
}

struct DelStepRsp {                                             //添加阶梯
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
}

struct StepListReq {                                            //查看阶梯
    1: optional onsale.MultipleCondition cond,                  //查询条件
}

struct StepListRsp {                                            //查看阶梯
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
    3: list<onsale.Step> stepList,                              //阶梯
}

struct AddPresentReq {                                          //添加赠品
    1: required list<onsale.Present> presentList,               //赠品
}

struct AddPresentRsp {                                          //添加赠品
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
}

struct DelPresentReq {                                          //删除赠品
    1: optional onsale.MultipleCondition cond,                  //查询条件
}

struct DelPresentRsp {                                          //添加赠品
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
}

struct PresentListReq {                                         //查看赠品
    1: optional onsale.MultipleCondition cond,                  //查询条件
}

struct PresentListRsp {                                         //查看赠品
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
    3: list<onsale.Present> presentList,                        //赠品
}

struct CreateWareLabelReq {                                     //创建商品标签请求
    1: required string name,                                    //名称
    2: required i32 label_type,                                 //商品标签类型
    3: required i32 scope_type,                                 //使用范围
    4: required i32 sub_type,                                   //子类型
    5: required i64 seller_id,                                  //卖家ID
    6: required string seller_slug,                             //卖家slug
    7: required string create_man,                              //创建人
}

struct CreateWareLabelRsp {                                     //创建商品标签响应
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
    3: onsale.WareLabel wareLabel,                              //商品标签
}

struct UpdateWareLabelReq {                                     //修改商品标签请求
    1: required onsale.WareLabel wareLabel,                     //商品标签
}

struct UpdateWareLabelRsp {                                     //修改商品标签响应
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
}

struct WareLabelCountReq {                                      //查询符合条件的商品标签数目
    1: optional onsale.MultipleCondition cond,                  //查询条件
}

struct WareLabelCountRsp {                                      //查询符合条件的商品标签数目
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
    3: i32 count,                                               //条数
}

struct WareLabelListReq {                                       //获取商品标签列表
    1: optional onsale.MultipleCondition cond,                  //查询条件
}

struct WareLabelListRsp {                                       //获取商品标签列表响应
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
    3: list<onsale.WareLabel> wareLabelList,                    //标签列表
}

struct AddWareLabelWaresReq {                                   //给标签添加商品请求
    1: required i32 ware_label_id,                              //商品标签ID
    2: required list<onsale.WareLabelWares> ware_list,          //信息
}

struct AddWareLabelWaresRsp {                                   //给标签添加商品响应
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
}

struct DelWareLabelWaresReq {                                   //给标签添删除商品请求
    1: required i32 ware_label_id,                              //商品标签ID
    2: required list<onsale.WareLabelWares> ware_list,          //信息
}

struct DelWareLabelWaresRsp {                                   //给标签添删除商品响应
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
}

struct WareLabelWaresCountReq {                                 //查询符合条件的 商品标签关联商品 数目
    1: optional onsale.MultipleCondition cond,                  //查询条件
}

struct WareLabelWaresCountRsp {                                 //查询符合条件的 商品标签关联商品 数目
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
    3: i32 count,                                               //条数
}

struct WareLabelWaresListReq {                                  //获取 商品标签关联商品 列表
    1: optional onsale.MultipleCondition cond,                  //查询条件
}

struct WareLabelWaresListRsp {                                  //获取 商品标签关联商品 列表响应
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
    3: list<onsale.WareLabelWares> wareLabelWaresList,          //商品列表
}

struct OfflineReq {                                             //紧急下线
    1: required i32 onsale_group_id,                            //促销活动ID
}

struct OfflineRsp {                                             //紧急下线
    1: onsale.Error error,                                      //错误码
    2: string errmsg,                                           //错误消息
}

service OnsaleAdminService {
    //增加缓存名单
    AddCacheNameListRsp addCacheNameList(1: AddCacheNameListReq addCacheNameListReq)

    //删除缓存名单
    DelCacheNameListRsp delCacheNameList(1: DelCacheNameListReq delCacheNameListReq)

    //查看缓存名单
    GetCacheNameListRsp getCacheNameList(1: GetCacheNameListReq getCacheNameListReq)

    //检查缓存名单
    CheckCacheNameListRsp checkCacheNameList(1: CheckCacheNameListReq checkCacheNameListReq)

    //查看检查缓存名单
    ViewCheckCacheNameListRsp viewCheckCacheNameList(1: ViewCheckCacheNameListReq viewCheckCacheNameListReq)

    //查看检查缓存名单
    LatestTimestampRsp getLatestTimestamp(1: LatestTimestampReq latestTimestampReq)

    //创建 商品标签关联商品 名单
    CreateNameListRsp createNameList(1: CreateNameListReq createNameListReq)

    //创建商品标签
    CreateWareLabelRsp createWareLabel(1: CreateWareLabelReq createWareLabelReq)
    
    //修改商品标签
    UpdateWareLabelRsp updateWareLabel(1: UpdateWareLabelReq updateWareLabelReq)

    //查询符合条件的商品标签数目
    WareLabelCountRsp getWareLabelCount(1: WareLabelCountReq wareLabelCountReq)

    //获取商品标签列表
    WareLabelListRsp getWareLabelList(1: WareLabelListReq wareLabelListReq)
    
    //给标签添加商品
    AddWareLabelWaresRsp addWareLabelWares(1: AddWareLabelWaresReq addWareLabelWaresReq)
    
    //给标签添删除商品
    DelWareLabelWaresRsp delWareLabelWares(1: DelWareLabelWaresReq delWareLabelWaresReq)

    //查询符合条件的 商品标签关联商品 数目
    WareLabelWaresCountRsp getWareLabelWaresCount(1: WareLabelWaresCountReq wareLabelWaresCountReq)
    
    //获取 商品标签关联商品 列表响应
    WareLabelWaresListRsp getWareLabelWaresList(1: WareLabelWaresListReq wareLabelWaresListReq)

    //增加赠品
    AddPresentRsp addPresent(1: AddPresentReq addPresentReq)

    //删除赠品
    DelPresentRsp delPresent(1: DelPresentReq delPresentReq)

    //查看赠品
    PresentListRsp getPresentList(1: PresentListReq presentListReq)

    //增加阶梯
    AddStepRsp addStep(1: AddStepReq addStepReq)

    //删除阶梯
    DelStepRsp delStep(1: DelStepReq delStepReq)

    //查看阶梯
    StepListRsp getStepList(1: StepListReq stepListReq)

    //创建促销活动
    CreateOnsaleGroupRsp createOnsaleGroup(1: CreateOnsaleGroupReq createOnsaleGroupReq)

    //修改促销活动
    UpdateOnsaleGroupRsp updateOnsaleGroup(1: UpdateOnsaleGroupReq updateOnsaleGroupReq)

    //查询符合条件的促销活动数目
    OnsaleGroupCountRsp getOnsaleGroupCount(1: OnsaleGroupCountReq onsaleGroupCountReq)

    //获取促销活动列表
    OnsaleGroupListRsp getOnsaleGroupList(1: OnsaleGroupListReq onsaleGroupListReq)

    //紧急下线
    OfflineRsp offline(1: OfflineReq offlineReq)
}
