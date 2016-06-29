#!/usr/bin/python
# coding=utf-8

from thrift import Thrift
from ThriftHelper import ThriftToSimpleJSON
from OnsaleAdminClient import OnsaleAdminClient
from onsale.ttypes import InvalidOperation, MultipleCondition, WareLabel, WareLabelWares
from onsale.ttypes import Present, Step, OnsaleGroup
from onsaleAdmin.ttypes import AddWareLabelWaresReq, DelWareLabelWaresReq, WareLabelWaresCountReq, WareLabelWaresListReq
from onsaleAdmin.ttypes import CreateWareLabelReq,UpdateWareLabelReq,OnsaleGroupListReq,PresentListReq,AddPresentReq
from onsaleAdmin.ttypes import AddStepReq, DelStepReq, StepListReq,CreateOnsaleGroupReq,UpdateOnsaleGroupReq,OfflineReq
from onsaleAdmin.ttypes import DelPresentReq, OnsaleGroupCountReq, CreateNameListReq,AddCacheNameListReq
from onsaleAdmin.ttypes import DelCacheNameListReq, GetCacheNameListReq, CheckCacheNameListReq,ViewCheckCacheNameListReq

server_ip = "127.0.0.1"
server_port = 18002

server_ip = "172.18.5.62"


def main():
    try:
        # ---------------------------------------------------------------------------------------------------------------
        # 校验名单
        if False:
            checkCacheNameListReq = CheckCacheNameListReq()
            checkCacheNameListReq.ware_label_id = 2
            checkCacheNameListReq.sub_type = 1

            print ThriftToSimpleJSON(OnsaleAdminClient(server_ip, server_port).checkCacheNameList(checkCacheNameListReq))
        # ---------------------------------------------------------------------------------------------------------------
        # 查看校验名单
        if False:
            viewCheckCacheNameListReq = ViewCheckCacheNameListReq()
            viewCheckCacheNameListReq.ware_label_id = 2
            viewCheckCacheNameListReq.sub_type = 1

            print ThriftToSimpleJSON(OnsaleAdminClient(server_ip, server_port).viewCheckCacheNameList(viewCheckCacheNameListReq))
        # ---------------------------------------------------------------------------------------------------------------
        # 增加名单
        if False:
            addCacheNameListReq = AddCacheNameListReq()
            addCacheNameListReq.ware_label_id = 2
            addCacheNameListReq.sub_type = 1
            addCacheNameListReq.ware_id_list = [123, 234, 345, ]
            #addCacheNameListReq.ware_id_list = [123, 234, 345, 456, 567, 678, 789, ]

            print ThriftToSimpleJSON(OnsaleAdminClient(server_ip, server_port).addCacheNameList(addCacheNameListReq))
        # ---------------------------------------------------------------------------------------------------------------
        # 删除名单
        if False:
            delCacheNameListReq = DelCacheNameListReq()
            delCacheNameListReq.ware_label_id = 2
            delCacheNameListReq.sub_type = 3
            delCacheNameListReq.ware_id_list = [234, ]

            print ThriftToSimpleJSON(OnsaleAdminClient(server_ip, server_port).delCacheNameList(delCacheNameListReq))
        # ---------------------------------------------------------------------------------------------------------------
        # 查询名单
        if False:
            getCacheNameListReq = GetCacheNameListReq()
            getCacheNameListReq.ware_label_id = 2
            getCacheNameListReq.sub_type = 3

            print ThriftToSimpleJSON(OnsaleAdminClient(server_ip, server_port).getCacheNameList(getCacheNameListReq))
        # ---------------------------------------------------------------------------------------------------------------
        # 创建名单
        if False:
            createNameListReq = CreateNameListReq()
            createNameListReq.ware_label_id = 2
            createNameListReq.sub_type = 1

            print ThriftToSimpleJSON(OnsaleAdminClient(server_ip, server_port).createNameList(createNameListReq))
        # ---------------------------------------------------------------------------------------------------------------
        # 查询活动列表
        if False:
            multipleCondition = MultipleCondition()
            multipleCondition.andCondList = ["id < 20", ]
            multipleCondition.limitCond = "limit 0,1000"

            onsaleGroupListReq = OnsaleGroupListReq()
            onsaleGroupListReq.cond = multipleCondition

            print ThriftToSimpleJSON(OnsaleAdminClient(server_ip, server_port).getOnsaleGroupList(onsaleGroupListReq))
        # ---------------------------------------------------------------------------------------------------------------
        # 查询活动数量
        if False:
            multipleCondition = MultipleCondition()
            multipleCondition.andCondList = ["id < 20", ]

            onsaleGroupCountReq = OnsaleGroupCountReq()
            onsaleGroupCountReq.cond = multipleCondition
            print ThriftToSimpleJSON(OnsaleAdminClient(server_ip, server_port).getOnsaleGroupCount(onsaleGroupCountReq))
        # ---------------------------------------------------------------------------------------------------------------
        # 创建活动
        if False:
            onsaleGroup = OnsaleGroup()
            onsaleGroup.name = 'yyyy'

            createOnsaleGroupReq = CreateOnsaleGroupReq()
            createOnsaleGroupReq.onsaleGroup = onsaleGroup

            print ThriftToSimpleJSON(OnsaleAdminClient(server_ip, server_port).createOnsaleGroup(createOnsaleGroupReq))
        # ---------------------------------------------------------------------------------------------------------------
        # 修改活动
        if False:
            onsaleGroup = OnsaleGroup()
            onsaleGroup.id = 1
            onsaleGroup.name = 'xxxxx'

            updateOnsaleGroupReq = UpdateOnsaleGroupReq()
            updateOnsaleGroupReq.onsaleGroup = onsaleGroup

            print ThriftToSimpleJSON(OnsaleAdminClient(server_ip, server_port).updateOnsaleGroup(updateOnsaleGroupReq))
        # ---------------------------------------------------------------------------------------------------------------
        # 查询赠品列表
        if False:
            multipleCondition = MultipleCondition()
            multipleCondition.andCondList = ["id <> 1", ]
            multipleCondition.limitCond = "limit 0,1000"

            stepListReq = StepListReq()
            stepListReq.cond = multipleCondition

            print ThriftToSimpleJSON(OnsaleAdminClient(server_ip, server_port).getStepList(stepListReq))
        # ---------------------------------------------------------------------------------------------------------------
        # 删除阶梯
        if False:
            multipleCondition = MultipleCondition()
            multipleCondition.andCondList = ["id = 1", ]

            delStepReq = DelStepReq()
            delStepReq.cond = multipleCondition

            print ThriftToSimpleJSON(OnsaleAdminClient(server_ip, server_port).delStep(delStepReq))
        # ---------------------------------------------------------------------------------------------------------------
        # 增加阶梯
        if False:
            s1 = Step()
            s1.onsale_group_id = 1
            s1.full_credit = 100
            s1.favor_credit = 18.88

            s2 = Step()
            s2.onsale_group_id = 1
            s2.full_credit = 200
            s2.favor_credit = 88.88

            addStepReq = AddStepReq()
            addStepReq.stepList = [s1, s2]

            print ThriftToSimpleJSON(OnsaleAdminClient(server_ip, server_port).addStep(addStepReq))
        # ---------------------------------------------------------------------------------------------------------------
        # 查询赠品列表
        if False:
            multipleCondition = MultipleCondition()
            multipleCondition.andCondList = ["step_id < 20", ]
            multipleCondition.limitCond = "limit 0,1000"

            presentListReq = PresentListReq()
            presentListReq.cond = multipleCondition

            print ThriftToSimpleJSON(OnsaleAdminClient(server_ip, server_port).getPresentList(presentListReq))
        # ---------------------------------------------------------------------------------------------------------------
        # 删除赠品
        if False:
            multipleCondition = MultipleCondition()
            multipleCondition.andCondList = ["step_id = 1", "sku_id = 3721", ]

            delPresentReq = DelPresentReq()
            delPresentReq.cond = multipleCondition

            print ThriftToSimpleJSON(OnsaleAdminClient(server_ip, server_port).delPresent(delPresentReq))
        # ---------------------------------------------------------------------------------------------------------------
        # 增加赠品
        if False:
            p1 = Present()
            p1.step_id = 1
            p1.sku_id = 3721
            p1.sku_price = 18.88
            p1.sku_count = 2
            p1.sku_slug = 'koode'

            p2 = Present()
            p2.step_id = 1
            p2.sku_id = 3446
            p2.sku_price = 5.56
            p2.sku_count = 1
            p2.sku_slug = 'njkjjk'

            addPresentReq = AddPresentReq()
            addPresentReq.presentList = [p1,p2]

            print ThriftToSimpleJSON(OnsaleAdminClient(server_ip, server_port).addPresent(addPresentReq))
        # ---------------------------------------------------------------------------------------------------------------
        # 创建商品标签
        if False:
            createWareLabelReq = CreateWareLabelReq()
            createWareLabelReq.name = '衣服11'
            createWareLabelReq.label_type = 0
            createWareLabelReq.scope_type = 0
            createWareLabelReq.sub_type = 0
            createWareLabelReq.seller_label_id = 0
            createWareLabelReq.create_man = "奥特曼11"

            print ThriftToSimpleJSON(OnsaleAdminClient(server_ip, server_port).createWareLabel(createWareLabelReq))
        # ---------------------------------------------------------------------------------------------------------------
        # 修改商品标签
        if False:
            wareLabel = WareLabel()
            wareLabel.name = '裤子'
            wareLabel.scope_type = 1
            wareLabel.sub_type = 1
            wareLabel.create_time = 1461747562
            wareLabel.update_time = 0
            wareLabel.ware_label_id = 1
            wareLabel.create_man = "奥特曼1"

            updateWareLabelReq = UpdateWareLabelReq()
            updateWareLabelReq.wareLabel = wareLabel

            print ThriftToSimpleJSON(OnsaleAdminClient(server_ip, server_port).updateWareLabel(updateWareLabelReq))
        # ---------------------------------------------------------------------------------------------------------------
        # 添加 商品标签-商品 数据
        if False:
            w1 = WareLabelWares()
            w1.ware_label_id = 2
            w1.ware_id = 45685

            w2 = WareLabelWares()
            w2.ware_label_id = 2
            w2.ware_id = 25644

            w3 = WareLabelWares()
            w3.ware_label_id = 2
            w3.ware_id = 234

            addWareLabelWaresReq = AddWareLabelWaresReq()
            addWareLabelWaresReq.ware_label_id = 2
            addWareLabelWaresReq.ware_list = [w1, w2, w3, ]

            print ThriftToSimpleJSON(OnsaleAdminClient(server_ip, server_port).addWareLabelWares(addWareLabelWaresReq))
        # ---------------------------------------------------------------------------------------------------------------
        # 删除 商品标签-商品 数据
        if False:
            w1 = WareLabelWares()
            w1.ware_label_id = 2
            w1.ware_id = 234

            delWareLabelWaresReq = DelWareLabelWaresReq()
            delWareLabelWaresReq.ware_label_id = 2
            delWareLabelWaresReq.ware_list = [w1, ]

            print ThriftToSimpleJSON(OnsaleAdminClient(server_ip, server_port).delWareLabelWares(delWareLabelWaresReq))
        # ---------------------------------------------------------------------------------------------------------------
        # 查询 标签-商品表 数量
        if False:
            multipleCondition = MultipleCondition()
            multipleCondition.andCondList = ["ware_label_id < 20", ]

            wareLabelWaresCountReq = WareLabelWaresCountReq()
            wareLabelWaresCountReq.cond = multipleCondition
            print ThriftToSimpleJSON(OnsaleAdminClient(server_ip, server_port).getWareLabelWaresCount(wareLabelWaresCountReq))
        # ---------------------------------------------------------------------------------------------------------------
        # 查询 标签-商品表 列表
        if False:
            multipleCondition = MultipleCondition()
            multipleCondition.andCondList = ["ware_label_id < 20", ]
            multipleCondition.limitCond = "limit 0,1000"

            wareLabelWaresListReq = WareLabelWaresListReq()
            wareLabelWaresListReq.cond = multipleCondition

            print ThriftToSimpleJSON(OnsaleAdminClient(server_ip, server_port).getWareLabelWaresList(wareLabelWaresListReq))
        # ---------------------------------------------------------------------------------------------------------------
        # 查询 标签-商品表 列表
        if False:
            offlineReq = OfflineReq()
            offlineReq.onsale_group_id = 1

            print ThriftToSimpleJSON(OnsaleAdminClient(server_ip, server_port).offline(offlineReq))
        # ---------------------------------------------------------------------------------------------------------------

    except Thrift.TException as tx:
        print('%s' % (tx.message))

if __name__ == '__main__':
    main()
