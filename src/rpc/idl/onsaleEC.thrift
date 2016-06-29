include "onsale.thrift"

namespace cpp onsaleEC
namespace py onsaleEC
namespace java com.meila.thrift.sdk.onsale.onsaleEC

struct PriceInfo {                                                      //价格信息
    1: double market_price,                                             //市场价
    2: double sale_price,                                               //售价
    3: double pay_price,                                                //实际支付金额
    4: double favor_price,                                              //优惠金额
}

struct SkuInfo {                                                        //sku 信息
    1: i64 sku_id,                                                      //sku ID
    2: i32 sku_count,                                                   //sku 数量
    3: double sale_price,                                               //售价（反弹回去）
    4: double market_price,                                             //市场价（反弹回去）
    5: i64 ware_id,                                                     //ware_id（反弹回去）
    6: i64 seller_id,                                                   //seller_id（反弹回去）
    7: i64 item_id,                                                     //item_id（反弹回去）
}

struct Apport {                                                         //本sku对应的分摊计算结果
    1: SkuInfo sku_info,                                                //sku 信息（反弹回去）
    2: PriceInfo price_info,                                            //分摊后的价格信息

    3: bool has_onsale_group,                                           //是否参与促销活动
    4: onsale.OnsaleGroup onsale_group,                                 //参与的促销活动信息

    5: bool has_ware_label,                                             //该促销活动使用的商品标签
    6: onsale.WareLabel ware_label,                                     //该促销活动使用的商品标签

    7: onsale.Step first_step,                                          //第一级阶梯

    8: bool has_curr_step,                                              //是否有可用的阶梯
    9: onsale.Step curr_step,                                           //当前可用的阶梯

    10: bool has_next_step,                                             //是否有下一级阶梯
    11: onsale.Step next_step,                                          //下一级阶梯

    12: double differ_credit,                                           //到下一阶梯差的金额
    13: i32 differ_count,                                               //到下一阶梯差的数量
}

struct UsableOnsaleGroupsReq {                                          //可用促销活动
    1: required list<SkuInfo> sku_list,                                 //sku列表
}

struct UsableOnsaleGroupsRsp {                                          //可用促销活动
    1: onsale.Error error,                                              //错误码
    2: string errmsg,                                                   //错误消息
    3: list<SkuInfo> sku_list,                                          //sku列表（反弹回去）

    4: PriceInfo order_price,                                           //总体价格信息
    5: map<i64, PriceInfo> skuID_priceInfo_map,                         //sku级价格信息

    6: map<i64, SkuInfo> sku_map,                                       //sku_id --> sku信息
    7: map<i64, i64> skuID_sellerID_map,                                //sku_id所属的seller_id
    8: map<i64, i64> skuID_wareID_map,                                  //sku_id所属的ware_id

    9: map<i64, i32> wareID_wareLabelID_map,                            //wareID所属的wareLabelID
    10: map<i32, onsale.WareLabel> wareLabelID_wareLabel_map,           //wareLabelID所属的wareLabel

    11: map<i64, i32> wareLabelID_onsaleGroupID_map,                    //wareLabelID所属的onsaleGroupID
    12: map<i32, onsale.OnsaleGroup> onsaleGroupID_onsaleGroup_map,     //有效的促销活动

    13: map<i64, onsale.OnsaleGroup> skuID_onsaleGroup_map,             //sku_id所参与的促销活动
    14: map<i64, i32> skuID_onsaleGroupID_map,                          //sku_id --> onsale_group_id

    15: map<i32, set<i64>> onsaleGroupID_skuIDSet_map,                  //onsale_group_id --> 享受该活动的所有sku
    16: map<i32, double>  onsaleGroupID_totalSalePrice_map,             //onsale_group_id --> 享受该活动的商品总售价
    17: map<i32, i32>  onsaleGroupID_totalCount_map,                    //onsale_group_id --> 享受该活动的商品总件数
    18: map<i32, double>  onsaleGroupID_totalFavorPrice_map,            //onsale_group_id --> 享受该活动的商品总优惠

    19: map<i32, list<onsale.Step>> onsaleGroupID_stepList_map,         //活动的阶梯信息
    20: map<i32, onsale.Step> onsaleGroupID_firstStep_map,              //活动第一级阶梯信息
    21: map<i32, onsale.Step> onsaleGroupID_currStep_map,               //活动最适合的阶梯信息
    22: map<i32, onsale.Step> onsaleGroupID_nextStep_map,               //活动最适合的阶梯信息的下一级信息

    23: list<onsale.Present> present_list,                              //赠品
    24: map<i32, i32> onsaleGroupID_currStepID_map,                     //活动最适合的阶梯id
    25: map<i32, list<onsale.Present>> currStepID_presentList_map,      //最适合的阶梯id的赠品
}

struct ApportionReq {                                                   //可用促销活动
    1: required list<SkuInfo> sku_list,                                 //sku列表
}

struct ApportionRsp {                                                   //可用促销活动
    1: onsale.Error error,                                              //错误码
    2: string errmsg,                                                   //错误消息
    3: UsableOnsaleGroupsRsp usableOnsaleGroupsRsp,                     //可用促销活动

    4: PriceInfo order_price,                                           //整个价格信息
    5: list<onsale.Present> present_list,                               //赠品

    6: map<i64,Apport> sku_apport_map,                                  //sku_id --> Apport
}

struct StepInfoReq {                                                    //活动的阶梯信息
    1: required i32 onsale_group_id,                                    //促销活动ID
}

struct StepInfoRsp {                                                    //活动的阶梯信息
    1: onsale.Error error,                                              //错误码
    2: string errmsg,                                                   //错误消息
    3: onsale.OnsaleGroup onsale_group,                                 //促销活动
    4: list<onsale.Step> step_list,                                     //阶梯信息
}

service OnsaleECService {
    //参与的活动
    UsableOnsaleGroupsRsp getUsableOnsaleGroups(1: UsableOnsaleGroupsReq usableOnsaleGroupsReq)

    //分摊计算
    ApportionRsp getApportion(1:ApportionReq apportionReq)

    //活动的阶梯信息
    StepInfoRsp getStepInfo(1: StepInfoReq stepInfoReq)
}
