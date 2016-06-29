#!/usr/bin/python
#coding=utf-8
import time
from thrift import Thrift
from ThriftHelper import ThriftToSimpleJSON
from CouponAdminClient import CouponAdminClient
from coupon.ttypes import InvalidOperation,MultipleCondition,WareLabel,UserCoupon,WareLabelWares,CouponGroup
from couponAdmin.ttypes import CreateCouponTableReq,CouponCountReq,CouponListReq
from couponAdmin.ttypes import CouponGroupCountReq,CouponGroupListReq,WareLabelCountReq,WareLabelListReq
from couponAdmin.ttypes import WareLabelWaresCountReq,WareLabelWaresListReq,UpdateOnlineReq
from couponAdmin.ttypes import CreateWareLabelReq,UpdateWareLabelReq,AddWareLabelWaresReq,DelWareLabelWaresReq
from couponAdmin.ttypes import CreateCouponGroupReq,UpdateCouponGroupReq,BatchUserCouponReq,VerifyArgotReq
from couponAdmin.ttypes import BatchDispatchReq,BatchExportReq,BatchExportCodeReq,UpdateJumpReq,UpdateArgotJumpReq

#server_ip = "127.0.0.1"
server_ip = "172.18.5.62"
server_port = 19002

def main():
    try:
        #---------------------------------------------------------------------------------------------------------------
        #修改优惠券链接
        if False:
            updateJumpReq = UpdateJumpReq()
            updateJumpReq.coupon_group_id = 2
            updateJumpReq.name = "美啦更美啦"
            updateJumpReq.jump_label = 'https://www.baidu.com/'
            updateJumpReq.jump_data  = 'https://www.baidu.com/'

            print ThriftToSimpleJSON(CouponAdminClient(server_ip,server_port).updateJump(updateJumpReq))
        #---------------------------------------------------------------------------------------------------------------
        #修改口令优惠券链接
        if False:
            updateArgotJumpReq = UpdateArgotJumpReq()
            updateArgotJumpReq.coupon_group_id = 2
            updateArgotJumpReq.name = "美啦更美啦2"
            updateArgotJumpReq.img = 'img1.jpg'
            updateArgotJumpReq.button_text  = 'button_text'
            updateArgotJumpReq.argot_jump_data = 'http://news.baidu.com/'
            updateArgotJumpReq.argot_jump_label = 'http://news.baidu.com/'

            print ThriftToSimpleJSON(CouponAdminClient(server_ip,server_port).updateArgotJump(updateArgotJumpReq))
        #---------------------------------------------------------------------------------------------------------------
        #修改优惠券链接
        if False:
            updateOnlineReq = UpdateOnlineReq()
            updateOnlineReq.coupon_group_id = 274
            updateOnlineReq.name = "baidu"
            updateOnlineReq.title = 'https://www.baidu.com/'
            updateOnlineReq.can_draw_count = 10
            updateOnlineReq.argot = '阿达'

            updateOnlineReq.start_draw_time = 1487937090
            updateOnlineReq.end_draw_time = 1487937090
            updateOnlineReq.is_duration_type = 1
            updateOnlineReq.duration_value = 7
            updateOnlineReq.start_use_time = 1487937090
            updateOnlineReq.end_use_time = 1487937090

            print ThriftToSimpleJSON(CouponAdminClient(server_ip,server_port).updateOnline(updateOnlineReq))
        #---------------------------------------------------------------------------------------------------------------
        #批量提取券码
        if False:
            batchExportCodeReq = BatchExportCodeReq()
            batchExportCodeReq.coupon_group_id = 11
            batchExportCodeReq.count = 2
            batchExportCodeReq.page = 1

            print ThriftToSimpleJSON(CouponAdminClient(server_ip,server_port).batchExportCode(batchExportCodeReq))
        #---------------------------------------------------------------------------------------------------------------
        #批量导出券
        if False:
            batchExportReq = BatchExportReq()
            batchExportReq.coupon_group_id = 2
            batchExportReq.count = 10

            print ThriftToSimpleJSON(CouponAdminClient(server_ip,server_port).batchExport(batchExportReq))
        #---------------------------------------------------------------------------------------------------------------
        #批量分发券
        if False:
            batchDispatchReq = BatchDispatchReq()
            batchDispatchReq.coupon_group_id = 2
            batchDispatchReq.user_id_list = [2000,2001,2002,2003,2004,]

            print ThriftToSimpleJSON(CouponAdminClient(server_ip,server_port).batchDispatch(batchDispatchReq))
        #---------------------------------------------------------------------------------------------------------------
        #判断口令是否有效
        if False:
            verifyArgotReq = VerifyArgotReq()
            verifyArgotReq.coupon_group_id = 999
            verifyArgotReq.start_draw_time = 1000000000
            verifyArgotReq.argot = 'xxx'

            print ThriftToSimpleJSON(CouponAdminClient(server_ip,server_port).verifyArgot(verifyArgotReq))
        #---------------------------------------------------------------------------------------------------------------
        #创建商品标签
        if False:
            createWareLabelReq = CreateWareLabelReq()
            createWareLabelReq.name = '衣服'
            createWareLabelReq.scope_type = 0
            createWareLabelReq.sub_type = 0

            print ThriftToSimpleJSON(CouponAdminClient(server_ip,server_port).createWareLabel(createWareLabelReq))
        #---------------------------------------------------------------------------------------------------------------
        #修改商品标签
        if False:
            wareLabel = WareLabel()
            wareLabel.name = '裤子'
            wareLabel.scope_type = 1
            wareLabel.sub_type = 1
            wareLabel.create_time = 0
            wareLabel.update_time = 0
            wareLabel.ware_label_id = 2

            updateWareLabelReq = UpdateWareLabelReq()
            updateWareLabelReq.wareLabel = wareLabel

            print ThriftToSimpleJSON(CouponAdminClient(server_ip,server_port).updateWareLabel(updateWareLabelReq))
        #---------------------------------------------------------------------------------------------------------------
        #添加 商品标签-商品 数据
        if False:
            w1 = WareLabelWares()
            w1.ware_label_id = 1
            w1.ware_id = 4568
            w1.ware_slug = 'saaere'

            w2 = WareLabelWares()
            w2.ware_label_id = 1
            w2.ware_id = 2564
            w2.ware_slug = 'dfaeee'

            addWareLabelWaresReq = AddWareLabelWaresReq()
            addWareLabelWaresReq.wareLabelWaresList = [w1,w2,]

            print ThriftToSimpleJSON(CouponAdminClient(server_ip,server_port).addWareLabelWares(addWareLabelWaresReq))
        #---------------------------------------------------------------------------------------------------------------
        #删除 商品标签-商品 数据
        if False:
            delWareLabelWaresReq = DelWareLabelWaresReq()
            delWareLabelWaresReq.ware_label_id = 1;
            delWareLabelWaresReq.ware_id = 2564;

            print ThriftToSimpleJSON(CouponAdminClient(server_ip,server_port).delWareLabelWares(delWareLabelWaresReq))
        #---------------------------------------------------------------------------------------------------------------
        #创建券组
        if False:
            couponGroup = CouponGroup()
            couponGroup.name = '元宵券组'
            couponGroup.title = '元宵快来了'
            couponGroup.comment = '元宵传统节日'
            couponGroup.ware_label_id = 1
            couponGroup.favor_type = 0
            couponGroup.scope_type = 0
            couponGroup.sub_type = 0
            couponGroup.scene_type = 0
            couponGroup.full = 100
            couponGroup.favor = 50
            couponGroup.rate = 0
            couponGroup.argot = ''
            couponGroup.max_count = 1000
            couponGroup.delta = 100
            couponGroup.drawn_count = 0
            couponGroup.payed_count = 0
            couponGroup.create_time = time.time()
            couponGroup.can_draw_count = 2
            couponGroup.start_draw_time = time.time()
            couponGroup.end_draw_time = time.time() + 7 * 24 * 3600
            couponGroup.is_duration_type = False
            couponGroup.duration_value = 0
            couponGroup.start_use_time = time.time()
            couponGroup.end_use_time = time.time() + 7 * 24 * 3600
            couponGroup.verify_status = 2
            couponGroup.applicant = 'charleeli'
            couponGroup.approver = 'vencent'
            couponGroup.modifier = 'iver'
            couponGroup.seller_id = 0
            couponGroup.url = 'https://www.baidu.com/'
            couponGroup.img = ''

            createCouponGroupReq = CreateCouponGroupReq()
            createCouponGroupReq.couponGroup = couponGroup

            print ThriftToSimpleJSON(CouponAdminClient(server_ip,server_port).createCouponGroup(createCouponGroupReq))
        #---------------------------------------------------------------------------------------------------------------
        #修改券组
        if False:
            couponGroup = CouponGroup()
            couponGroup.name = '修改'
            couponGroup.title = '新年快来了'
            couponGroup.comment = '新年时传统节日'
            couponGroup.ware_label_id = 1
            couponGroup.favor_type = 0
            couponGroup.scope_type = 0
            couponGroup.sub_type = 0
            couponGroup.scene_type = 0
            couponGroup.full = 100
            couponGroup.favor = 30
            couponGroup.rate = 0
            couponGroup.argot = ''
            couponGroup.max_count = 1000
            couponGroup.delta = 100
            couponGroup.drawn_count = 0
            couponGroup.payed_count = 0
            couponGroup.create_time = time.time()
            couponGroup.can_draw_count = 1
            couponGroup.start_draw_time = time.time()
            couponGroup.end_draw_time = time.time() + 7 * 24 * 3600
            couponGroup.is_duration_type = False
            couponGroup.duration_value = 0
            couponGroup.start_use_time = time.time()
            couponGroup.end_use_time = time.time() + 7 * 24 * 3600
            couponGroup.verify_status = 0
            couponGroup.applicant = 'charleeli'
            couponGroup.approver = 'vencent'
            couponGroup.modifier = 'iver'
            couponGroup.seller_id = 0
            couponGroup.url = 'https://www.baidu.com/'
            couponGroup.img = ''
            couponGroup.id = 1

            updateCouponGroupReq = UpdateCouponGroupReq()
            updateCouponGroupReq.couponGroup = couponGroup

            print ThriftToSimpleJSON(CouponAdminClient(server_ip,server_port).updateCouponGroup(updateCouponGroupReq))
        #---------------------------------------------------------------------------------------------------------------
        #创建一张券表
        if False:
            createCouponTableReq = CreateCouponTableReq()
            createCouponTableReq.coupon_group_id = 103;
            print ThriftToSimpleJSON(CouponAdminClient(server_ip,server_port).createCouponTable(createCouponTableReq))
        #---------------------------------------------------------------------------------------------------------------
        #查询券表的数量
        if False:
            singleCondition = "id <> 5"
            singleCondition1= "id < 10"
            singleCondition2= "id > 12"

            multipleCondition = MultipleCondition()
            multipleCondition.andCondList=[]
            multipleCondition.andCondList.append(singleCondition)

            multipleCondition.orCondList=[]
            multipleCondition.orCondList.append(singleCondition1)
            multipleCondition.orCondList.append(singleCondition2)

            couponCountReq = CouponCountReq()
            couponCountReq.coupon_group_id = 101
            couponCountReq.cond = multipleCondition
            print ThriftToSimpleJSON(CouponAdminClient(server_ip,server_port).getCouponCount(couponCountReq))

        #---------------------------------------------------------------------------------------------------------------
        #查询券列表
        if False:
            singleCondition = "id <> 5"
            singleCondition1= "id < 10"
            singleCondition2= "id > 12"

            multipleCondition = MultipleCondition()
            multipleCondition.andCondList=[]
            multipleCondition.andCondList.append(singleCondition)

            multipleCondition.orCondList=[]
            multipleCondition.orCondList.append(singleCondition1)
            multipleCondition.orCondList.append(singleCondition2)

            couponListReq = CouponListReq()
            couponListReq.coupon_group_id = 1
            couponListReq.cond = multipleCondition
            couponListReq.offset = 0
            couponListReq.rows = 100
            print ThriftToSimpleJSON(CouponAdminClient(server_ip,server_port).getCouponList(couponListReq))

        #---------------------------------------------------------------------------------------------------------------
        #查询券组的数量
        if False:
            singleCondition = "id <> 5"
            singleCondition1= "id < 10"
            singleCondition2= "id > 12"

            multipleCondition = MultipleCondition()
            multipleCondition.andCondList=[]
            multipleCondition.andCondList.append(singleCondition)

            multipleCondition.orCondList=[]
            multipleCondition.orCondList.append(singleCondition1)
            multipleCondition.orCondList.append(singleCondition2)

            couponGroupCountReq = CouponGroupCountReq()
            couponGroupCountReq.cond = multipleCondition
            print ThriftToSimpleJSON(CouponAdminClient(server_ip,server_port).getCouponGroupCount(couponGroupCountReq))

        #---------------------------------------------------------------------------------------------------------------
        #查询券组列表
        if False:
            singleCondition = "id <> 5"
            singleCondition1= "id < 10"
            singleCondition2= "id > 12"

            multipleCondition = MultipleCondition()
            multipleCondition.andCondList=[]
            multipleCondition.andCondList.append(singleCondition)

            multipleCondition.orCondList=[]
            multipleCondition.orCondList.append(singleCondition1)
            multipleCondition.orCondList.append(singleCondition2)

            couponGroupListReq = CouponGroupListReq()
            couponGroupListReq.cond = multipleCondition
            couponGroupListReq.offset = 0
            couponGroupListReq.rows = 100
            print ThriftToSimpleJSON(CouponAdminClient(server_ip,server_port).getCouponGroupList(couponGroupListReq))
        #---------------------------------------------------------------------------------------------------------------
        #查询标签数量
        if False:
            singleCondition = "id <> 5"
            singleCondition1= "id < 10"
            singleCondition2= "id > 12"

            multipleCondition = MultipleCondition()
            multipleCondition.andCondList=[]
            multipleCondition.andCondList.append(singleCondition)

            multipleCondition.orCondList=[]
            multipleCondition.orCondList.append(singleCondition1)
            multipleCondition.orCondList.append(singleCondition2)

            wareLabelCountReq = WareLabelCountReq()
            wareLabelCountReq.cond = multipleCondition
            print ThriftToSimpleJSON(CouponAdminClient(server_ip,server_port).getWareLabelCount(wareLabelCountReq))

        #---------------------------------------------------------------------------------------------------------------
        #查询标签列表
        if False:
            singleCondition = "id <> 5"
            singleCondition1= "id < 10"
            singleCondition2= "id > 12"

            multipleCondition = MultipleCondition()
            multipleCondition.andCondList=[]
            multipleCondition.andCondList.append(singleCondition)

            multipleCondition.orCondList=[]
            multipleCondition.orCondList.append(singleCondition1)
            multipleCondition.orCondList.append(singleCondition2)

            wareLabelListReq = WareLabelListReq()
            wareLabelListReq.cond = multipleCondition
            wareLabelListReq.offset = 0
            wareLabelListReq.rows = 100
            print ThriftToSimpleJSON(CouponAdminClient(server_ip,server_port).getWareLabelList(wareLabelListReq))

        #---------------------------------------------------------------------------------------------------------------
        #查询 标签-商品表 数量
        if False:
            singleCondition = "id <> 5"
            singleCondition1= "id < 10"
            singleCondition2= "id > 12"

            multipleCondition = MultipleCondition()
            multipleCondition.andCondList=[]
            multipleCondition.andCondList.append(singleCondition)

            multipleCondition.orCondList=[]
            multipleCondition.orCondList.append(singleCondition1)
            multipleCondition.orCondList.append(singleCondition2)

            wareLabelWaresCountReq = WareLabelWaresCountReq()
            wareLabelWaresCountReq.cond = multipleCondition
            print ThriftToSimpleJSON(CouponAdminClient(server_ip,server_port).getWareLabelWaresCount(wareLabelWaresCountReq))

        #---------------------------------------------------------------------------------------------------------------
        #查询 标签-商品表 列表
        if False:
            multipleCondition = MultipleCondition()
            multipleCondition.andCondList=[]
            multipleCondition.orCondList=[]
            multipleCondition.orderCondList=['id desc']

            wareLabelWaresListReq = WareLabelWaresListReq()
            wareLabelWaresListReq.cond = multipleCondition
            wareLabelWaresListReq.offset = 0
            wareLabelWaresListReq.rows = 20
            print ThriftToSimpleJSON(CouponAdminClient(server_ip,server_port).getWareLabelWaresList(wareLabelWaresListReq))

        #---------------------------------------------------------------------------------------------------------------
        #查询用户-券
        if False:
            batchUserCouponReq = BatchUserCouponReq()
            batchUserCouponReq.codeList = ['']

            print ThriftToSimpleJSON(CouponAdminClient(server_ip,server_port).batchUserCoupon(batchUserCouponReq))

        #---------------------------------------------------------------------------------------------------------------

    except Thrift.TException as tx:
        print(('%s' % (tx.message)))


if __name__ == '__main__':
    main()
