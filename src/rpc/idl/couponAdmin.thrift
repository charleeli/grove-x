include "coupon.thrift"

namespace cpp couponAdmin
namespace py couponAdmin

struct CreateWareLabelReq {                 //创建商品标签请求
    1: required string name,                //名称
    2: required i32 scope_type,             //使用范围  
    3: required i32 sub_type,               //子类型 
    4: required string create_man,          //创建人       
}

struct CreateWareLabelRsp {                 //创建商品标签响应
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息 
    3: i32 ware_label_id,                   //插入的标签ID        
}

struct UpdateWareLabelReq {                 //修改商品标签请求
    1: required coupon.WareLabel wareLabel, //商品标签         
}

struct UpdateWareLabelRsp {                 //修改商品标签响应
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息         
}

struct AddWareLabelWaresReq {               //给标签添加商品请求
    1: required list<coupon.WareLabelWares> wareLabelWaresList, //信息
}

struct AddWareLabelWaresRsp {               //给标签添加商品响应
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息 
}

struct DelWareLabelWaresReq {               //给标签添删除商品请求
    1: required i32 ware_label_id,          //商品标签id
    2: required i64 ware_id,                //商品id
}

struct DelWareLabelWaresRsp {               //给标签添删除商品响应
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息 
}

struct CreateCouponGroupReq {               //创建券组请求
    1: required coupon.CouponGroup couponGroup, //券组信息          
}

struct CreateCouponGroupRsp {               //创建券组响应
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息   
    3: i32 coupon_group_id,                 //券组ID      
}

struct UpdateCouponGroupReq {               //修改券组请求
    1: required coupon.CouponGroup couponGroup, //券组信息          
}

struct UpdateCouponGroupRsp {               //修改券组响应
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息  
    3: coupon.CouponGroup couponGroup,      //修改后的券组信息        
}

struct CreateCouponTableReq {               //创建表请求
    1: required i32 coupon_group_id,        //券组ID
}

struct CreateCouponTableRsp {               //创建表响应
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息  
    3: string tableName,                    //创建的表名
}

struct CouponCountReq {                     //查询符合条件的优惠券数目
    1: required i32 coupon_group_id,        //券组
    2: optional coupon.MultipleCondition cond,//查询条件
}

struct CouponCountRsp {                     //查询符合条件的优惠券数目响应
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息  
    3: i32 count,                           //条数
}

struct CouponListReq {                      //获取优惠券列表
    1: required i32 coupon_group_id,        //券组
    2: optional coupon.MultipleCondition cond,//查询条件
    3: required i32 offset,                 //起始
    4: required i32 rows,                   //数量
}

struct CouponListRsp {                      //获取优惠券列表响应
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息  
    3: list<coupon.Coupon> couponList,      //券列表
}

struct CouponGroupCountReq {                //查询符合条件的优惠券组数目
    1: optional coupon.MultipleCondition cond,//查询条件
}

struct CouponGroupCountRsp {                //查询符合条件的优惠券组数目响应
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息  
    3: i32 count,                           //条数
}

struct CouponGroupListReq {                 //获取优惠券组列表
    1: optional coupon.MultipleCondition cond,//查询条件
    2: required i32 offset,                 //起始
    3: required i32 rows,                   //数量
}

struct CouponGroupListRsp {                 //获取优惠券组列表响应
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息  
    3: list<coupon.CouponGroup> couponGroupList,//券列表
}

struct WareLabelCountReq {                  //查询符合条件的商品标签数目
    1: optional coupon.MultipleCondition cond,//查询条件
}

struct WareLabelCountRsp {                  //查询符合条件的商品标签数目
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息  
    3: i32 count,                           //条数
}

struct WareLabelListReq {                   //获取商品标签列表
    1: optional coupon.MultipleCondition cond,//查询条件
    2: required i32 offset,                 //起始
    3: required i32 rows,                   //数量
}

struct WareLabelListRsp {                   //获取商品标签列表响应
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息  
    3: list<coupon.WareLabel> wareLabelList,//券列表
}

struct WareLabelWaresCountReq {             //查询符合条件的 商品标签关联商品 数目
    1: optional coupon.MultipleCondition cond,//查询条件
}

struct WareLabelWaresCountRsp {             //查询符合条件的 商品标签关联商品 数目
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息  
    3: i32 count,                           //条数
}

struct WareLabelWaresListReq {              //获取 商品标签关联商品 列表
    1: optional coupon.MultipleCondition cond,//查询条件
    2: required i32 offset,                 //起始
    3: required i32 rows,                   //数量
}

struct WareLabelWaresListRsp {              //获取 商品标签关联商品 列表响应
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息  
    3: list<coupon.WareLabelWares> wareLabelWaresList,//商品列表
}

struct BatchUserCouponReq {                 //批量查用户-券表请求
    1: required list<string> codeList,      //券码集合
}

struct BatchUserCouponRsp {                 //批量查用户-券表响应
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息  
    3: map<string,coupon.UserCoupon> userCouponMap,//code --> UserCoupon
}

struct VerifyArgotReq {                     //判断口令是否有效
    1: required i32 coupon_group_id,        //优惠券组ID
    2: i32 start_draw_time,                 //领取有效期开始
    3: required string argot,               //口令
}

struct VerifyArgotRsp {                     //判断口令是否有效
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息 
    3: string argot,                        //口令
}

struct BatchDispatchReq {                   //批量发放请求
    1: required i32 coupon_group_id,        //优惠券组ID
    2: required list<i64> user_id_list,     //用户ID列表,个数小于1000 
}

struct BatchDispatchRsp {                   //批量发放响应
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息
    3: i32 count,                           //本次上传的用户数量
    4: i32 success_count,                   //本次分发成功的用户数量 
    5: i32 time_cost,                       //本次发放预计耗时/秒
}

struct BatchExportReq {                     //批量导出请求
    1: required i32 coupon_group_id,        //优惠券组ID
    2: required i32 count,                  //一次导出张数
}

struct BatchExportRsp {                     //批量导出响应
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息
    3: i32 count,                           //一次导出张数
    4: i32 time_cost,                       //本次导出预计耗时/秒
}

struct BatchExportCodeReq {                 //批量提取券码请求
    1: required i32 coupon_group_id,        //优惠券组ID
    2: required i32 page,                   //第几页 1...
    3: required i32 count,                  //每页数量 不多于1000
}

struct BatchExportCodeRsp {                 //批量提取券码响应
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息
    3: i32 page,                            //第几页 1...
    4: i32 count,                           //每页数量
    5: i32 total,                           //总数
    6: list<string> codes,                  //券码
}

struct UpdateJumpReq {                      //修改优惠券链接
    1: required i32 coupon_group_id,        //优惠券组ID
    2: required string name,                //优惠券组名称
    3: required string jump_label,          //跳转类型
    4: required string jump_data,           //按钮跳转
}

struct UpdateJumpRsp {                      //修改优惠券链接
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息
}

struct UpdateArgotJumpReq {                 //修改口令优惠券链接
    1: required i32 coupon_group_id,        //优惠券组ID
    2: required string name,                //优惠券组名称
    3: required string img,                 //图片
    4: required i32 img_width,              //图片宽
    5: required i32 img_height,             //图片高
    6: required string button_text,         //按钮文字
    7: required string argot_jump_label,    //口令跳转类型
    8: required string argot_jump_data,     //口令按钮跳转
}

struct UpdateArgotJumpRsp {                 //修改口令优惠券链接
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息
}

struct UpdateOnlineReq {                    //线上修改优惠券
    1: required i32 coupon_group_id,        //优惠券组ID
    2: required string name,                //优惠券组名称
    3: required string title,               //标题
    4: required i32 can_draw_count,         //每个人允许领取的数量
    5: required string argot,               //暗语
    6: required i32 end_draw_time,          //领取有效期结束
    7: optional string description          //优惠券组适用范围描述
}

struct UpdateOnlineRsp {                    //线上修改优惠券
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息
}

struct OfflineReq {                         //下线功能
    1: required i32 coupon_group_id,        //优惠券组ID
}

struct OfflineRsp {                         //下线功能
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息
}

struct UpdateDefaultConfigReq {             //默认配置
    1: required coupon.DefaultConfig config,//默认配置
}

struct UpdateDefaultConfigRsp {             //默认配置
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息
}

struct ViewDefaultConfigReq {               //默认配置
    1: required bool dummy,                 //哑字段,任意填写
}

struct ViewDefaultConfigRsp {               //默认配置
    1: coupon.Error error,                  //错误码
    2: string errmsg,                       //错误消息
    3: coupon.DefaultConfig config,         //默认配置
}

service CouponAdminService {
    //更新默认配置
    UpdateDefaultConfigRsp updateDefaultConfig(1: UpdateDefaultConfigReq updateDefaultConfigReq) throws (1:coupon.InvalidOperation ouch)

    //查看默认配置
    ViewDefaultConfigRsp viewDefaultConfig(1: ViewDefaultConfigReq viewDefaultConfigReq) throws (1:coupon.InvalidOperation ouch)

    //创建商品标签
    CreateWareLabelRsp createWareLabel(1: CreateWareLabelReq createWareLabelReq) throws (1:coupon.InvalidOperation ouch)
    
    //修改商品标签
    UpdateWareLabelRsp updateWareLabel(1: UpdateWareLabelReq updateWareLabelReq) throws (1:coupon.InvalidOperation ouch)
    
    //给标签添加商品
    AddWareLabelWaresRsp addWareLabelWares(1: AddWareLabelWaresReq addWareLabelWaresReq) throws (1:coupon.InvalidOperation ouch)
    
    //给标签添删除商品
    DelWareLabelWaresRsp delWareLabelWares(1: DelWareLabelWaresReq delWareLabelWaresReq) throws (1:coupon.InvalidOperation ouch)
    
    //创建券组
    CreateCouponGroupRsp createCouponGroup(1:CreateCouponGroupReq createCouponGroupReq) throws (1:coupon.InvalidOperation ouch)
    
    //修改券组
    UpdateCouponGroupRsp updateCouponGroup(1:UpdateCouponGroupReq updateCouponGroupReq) throws (1:coupon.InvalidOperation ouch)

    //创建一张券表
    CreateCouponTableRsp createCouponTable(1:CreateCouponTableReq createCouponTableReq) throws (1:coupon.InvalidOperation ouch)

    //查询符合条件的优惠券数目
    CouponCountRsp getCouponCount(1: CouponCountReq couponCountReq) throws (1:coupon.InvalidOperation ouch)
    
    //查询优惠券数据列表
    CouponListRsp getCouponList(1: CouponListReq couponListReq) throws (1:coupon.InvalidOperation ouch)
    
    //查询优惠券组数目
    CouponGroupCountRsp getCouponGroupCount(1: CouponGroupCountReq couponGroupCountReq) throws (1:coupon.InvalidOperation ouch)
    
    //查询优惠券组数据列表
    CouponGroupListRsp getCouponGroupList(1: CouponGroupListReq couponGroupListReq) throws (1:coupon.InvalidOperation ouch)
    
    //查询符合条件的商品标签数目
    WareLabelCountRsp getWareLabelCount(1: WareLabelCountReq wareLabelCountReq) throws (1:coupon.InvalidOperation ouch)
    
    //获取商品标签列表
    WareLabelListRsp getWareLabelList(1: WareLabelListReq wareLabelListReq) throws (1:coupon.InvalidOperation ouch)
    
    //查询符合条件的 商品标签关联商品 数目
    WareLabelWaresCountRsp getWareLabelWaresCount(1: WareLabelWaresCountReq wareLabelWaresCountReq) throws (1:coupon.InvalidOperation ouch)
    
    //获取 商品标签关联商品 列表响应
    WareLabelWaresListRsp getWareLabelWaresList(1: WareLabelWaresListReq wareLabelWaresListReq) throws (1:coupon.InvalidOperation ouch)
    
    //批量查用户-券表
    BatchUserCouponRsp batchUserCoupon(1: BatchUserCouponReq batchUserCouponReq) throws (1:coupon.InvalidOperation ouch)
    
    //判断口令是否有效
    VerifyArgotRsp verifyArgot(1: VerifyArgotReq verifyArgotReq) throws (1:coupon.InvalidOperation ouch)
    
    //批量发放
    BatchDispatchRsp batchDispatch(1: BatchDispatchReq batchDispatchReq) throws (1:coupon.InvalidOperation ouch)
    
    //批量导出
    BatchExportRsp batchExport(1: BatchExportReq batchExportReq) throws (1:coupon.InvalidOperation ouch)
    
    //批量提取券码
    BatchExportCodeRsp batchExportCode(1: BatchExportCodeReq batchExportCodeReq) throws (1:coupon.InvalidOperation ouch)

    //修改优惠券链接
    UpdateJumpRsp updateJump(1: UpdateJumpReq updateJumpReq) throws (1:coupon.InvalidOperation ouch)
    
    //修改口令优惠券链接
    UpdateArgotJumpRsp updateArgotJump(1: UpdateArgotJumpReq updateArgotJumpReq) throws (1:coupon.InvalidOperation ouch)

    //修改优惠券链接
    UpdateOnlineRsp updateOnline(1: UpdateOnlineReq updateOnlineReq) throws (1:coupon.InvalidOperation ouch)
    
    //下线
    OfflineRsp offline(1: OfflineReq offlineReq) throws (1:coupon.InvalidOperation ouch)
}
