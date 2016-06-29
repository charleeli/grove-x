#!/usr/bin/python
# coding=utf-8
from thrift import Thrift
from ThriftHelper import ThriftToSimpleJSON
from CouponECClient import CouponECClient
from couponEC.ttypes import DrawReq,FreezeReq,RollbackReq,PayReq,UserCouponsReq,RelatedCouponGroupsReq,WareLabelReq
from couponEC.ttypes import WareActsReq,SkuInfo,SellerInfo,UsableCouponsReq,ApportionReq,CacheCouponGroupsReq
from couponEC.ttypes import ServerTimeReq,DecodeCodeReq,CheckArgotReq,ClientCouponsReq,UserLeftCouponsCountReq
from couponEC.ttypes import NotUsableCouponsReq,DrawnCountTodayReq,GetDefaultConfigReq

server_ip = "127.0.0.1"
server_port = 19001

server_ip = "172.18.5.62"


def main():
    try:
        #  ---------------------------------------------------------------------------------------------------------------
        if False:
            getDefaultConfigReq = GetDefaultConfigReq()
            getDefaultConfigReq.dummy = True
            print ThriftToSimpleJSON(CouponECClient(server_ip, server_port).getDefaultConfig(getDefaultConfigReq))
        # ---------------------------------------------------------------------------------------------------------------
        if False:
            drawnCountTodayReq = DrawnCountTodayReq()
            drawnCountTodayReq.user_id = 5006644
            drawnCountTodayReq.coupon_group_id = 8
            print ThriftToSimpleJSON(CouponECClient(server_ip, server_port).getDrawnCountToday(drawnCountTodayReq))
        # ---------------------------------------------------------------------------------------------------------------
        if False:
            notUsableCouponsReq = NotUsableCouponsReq()
            notUsableCouponsReq.user_id = 717066513
            notUsableCouponsReq.sellers = {}
            notUsableCouponsReq.usableCodes = ['vuxkrznmbfr']
            notUsableCouponsReq.page = 1
            notUsableCouponsReq.count = 100
            print ThriftToSimpleJSON(CouponECClient(server_ip, server_port).getNotUsableCoupons(notUsableCouponsReq))
        # ---------------------------------------------------------------------------------------------------------------
        if False:
            userLeftCouponsCountReq = UserLeftCouponsCountReq()
            userLeftCouponsCountReq.coupon_group_id = 601
            userLeftCouponsCountReq.user_id = 5006629
            print ThriftToSimpleJSON(CouponECClient(server_ip, server_port).getUserLeftCouponsCount(userLeftCouponsCountReq))
        # ---------------------------------------------------------------------------------------------------------------
        if False:
            clientCouponsReq = ClientCouponsReq()
            clientCouponsReq.coupon_group_id = 2
            clientCouponsReq.client_id = "xxx"
            print ThriftToSimpleJSON(CouponECClient(server_ip, server_port).getClientCoupons(clientCouponsReq))
        # ---------------------------------------------------------------------------------------------------------------
        # 判断口令是否有效
        if False:
            decodeCodeReq = DecodeCodeReq()
            decodeCodeReq.code = 'wppkmxtpjfx'
            print ThriftToSimpleJSON(CouponECClient(server_ip, server_port).decodeCode(decodeCodeReq))
        # ---------------------------------------------------------------------------------------------------------------
        # 判断口令是否有效
        if False:
            checkArgotReq = CheckArgotReq()
            checkArgotReq.argot = '啦啦啦，德玛西亚'
            print ThriftToSimpleJSON(CouponECClient(server_ip, server_port).checkArgot(checkArgotReq))
        # ---------------------------------------------------------------------------------------------------------------
        if False:
            serverTimeReq = ServerTimeReq()
            serverTimeReq.local_time = 1453793419
            print ThriftToSimpleJSON(CouponECClient(server_ip, server_port).getServerTime(serverTimeReq))
        # ---------------------------------------------------------------------------------------------------------------
        if False:
            drawReq = DrawReq()
            drawReq.coupon_group_id = 608
            drawReq.user_id = 3673745
            drawReq.client_id = "jafldjfljeiuj"
            drawReq.argot = "ivy"
            drawReq.scene_type = 0
            drawReq.code = 'zhncxygyfdh'
            print ThriftToSimpleJSON(CouponECClient(server_ip, server_port).drawCoupon(drawReq))

        # ---------------------------------------------------------------------------------------------------------------
        if False:
            freezeReq = FreezeReq()
            freezeReq.user_id = 3673745
            freezeReq.code = 'zzxtsimsreq'
            freezeReq.order_id = 'POIXXX'
            print ThriftToSimpleJSON(CouponECClient(server_ip, server_port).freezeCoupon(freezeReq))

        # ---------------------------------------------------------------------------------------------------------------
        if False:
            rollbackReq = RollbackReq()
            rollbackReq.user_id = 10086
            rollbackReq.code = 'wbdcldgmedw'
            rollbackReq.order_id = 'POIXXX'
            print ThriftToSimpleJSON(CouponECClient(server_ip, server_port).rollbackCoupon(rollbackReq))

        # ---------------------------------------------------------------------------------------------------------------
        if False:
            payReq = PayReq()
            payReq.user_id = 10086
            payReq.code = 'wbdcldgmedw'
            payReq.order_id = 'POIXXX'
            print ThriftToSimpleJSON(CouponECClient(server_ip, server_port).payCoupon(payReq))

        # ---------------------------------------------------------------------------------------------------------------
        if True:
            userCouponsReq = UserCouponsReq()
            userCouponsReq.user_id = 5006908
            userCouponsReq.page = 1
            userCouponsReq.count = 100
            userCouponsReq.use_status = 3
            print ThriftToSimpleJSON(CouponECClient(server_ip, server_port).getUserCoupons(userCouponsReq))

        # ---------------------------------------------------------------------------------------------------------------
        if False:
            wareActsReq = WareActsReq()
            wareActsReq.ware_ids = [468, 211, 3, ]
            print ThriftToSimpleJSON(CouponECClient(server_ip, server_port).getWareActs(wareActsReq))

        # ---------------------------------------------------------------------------------------------------------------
        if False:
            skuInfo_1 = SkuInfo()
            skuInfo_1.sku_id = 779
            skuInfo_1.sku_count = 1
            skuInfo_1.sale_price = 50.01
            skuInfo_1.market_price = 1000
            skuInfo_1.ware_id = 551

            skuInfo_2 = SkuInfo()
            skuInfo_2.sku_id = 844
            skuInfo_2.sku_count = 1
            skuInfo_2.sale_price = 56
            skuInfo_2.market_price = 1000
            skuInfo_2.ware_id = 611

            skuInfo_3 = SkuInfo()
            skuInfo_3.sku_id = 7945
            skuInfo_3.sku_count = 1
            skuInfo_3.sale_price = 90
            skuInfo_3.market_price = 199
            skuInfo_3.ware_id = 10161

            skuInfo_4 = SkuInfo()
            skuInfo_4.sku_id = 7944
            skuInfo_4.sku_count = 2
            skuInfo_4.sale_price = 89
            skuInfo_4.market_price = 199
            skuInfo_4.ware_id = 10161

            skuInfo_5 = SkuInfo()
            skuInfo_5.sku_id = 7937
            skuInfo_5.sku_count = 2
            skuInfo_5.sale_price = 99
            skuInfo_5.market_price = 199
            skuInfo_5.ware_id = 10161

            sellerInfo_1 = SellerInfo()
            sellerInfo_1.seller_id = 2477030
            sellerInfo_1.skus = {
                        779: skuInfo_1,
                        844: skuInfo_2,
                    }

            sellerInfo_2 = SellerInfo()
            sellerInfo_2.seller_id = 3102766
            sellerInfo_2.skus={
                        7945: skuInfo_3,
                        7944: skuInfo_4,
                        7937: skuInfo_5,
                    }

            usableCouponsReq = UsableCouponsReq()
            usableCouponsReq.user_id = 3673759
            usableCouponsReq.sellers = {
                            2477030: sellerInfo_1,
                            3102766: sellerInfo_2,
                        }

            print ThriftToSimpleJSON(CouponECClient(server_ip, server_port).getUsableCoupons(usableCouponsReq))
        # ---------------------------------------------------------------------------------------------------------------
        if False:
            skuInfo_1 = SkuInfo()
            skuInfo_1.sku_id = 779
            skuInfo_1.sku_count = 1
            skuInfo_1.sale_price = 50.01
            skuInfo_1.market_price = 1000
            skuInfo_1.ware_id = 551

            skuInfo_2 = SkuInfo()
            skuInfo_2.sku_id = 844
            skuInfo_2.sku_count = 1
            skuInfo_2.sale_price = 56
            skuInfo_2.market_price = 1000
            skuInfo_2.ware_id = 611

            skuInfo_3 = SkuInfo()
            skuInfo_3.sku_id = 7945
            skuInfo_3.sku_count = 1
            skuInfo_3.sale_price = 90
            skuInfo_3.market_price = 199
            skuInfo_3.ware_id = 10161

            skuInfo_4 = SkuInfo()
            skuInfo_4.sku_id = 7944
            skuInfo_4.sku_count = 2
            skuInfo_4.sale_price = 89
            skuInfo_4.market_price = 199
            skuInfo_4.ware_id = 10161

            skuInfo_5 = SkuInfo()
            skuInfo_5.sku_id = 7937
            skuInfo_5.sku_count = 2
            skuInfo_5.sale_price = 99
            skuInfo_5.market_price = 199
            skuInfo_5.ware_id = 10161

            sellerInfo_1 = SellerInfo()
            sellerInfo_1.seller_id = 2477030
            sellerInfo_1.skus = {
                        779: skuInfo_1,
                        844: skuInfo_2,
                    }

            sellerInfo_2 = SellerInfo()
            sellerInfo_2.seller_id = 3102766
            sellerInfo_2.skus = {
                        7945: skuInfo_3,
                        7944: skuInfo_4,
                        7937: skuInfo_5,
                    }

            apportionReq = ApportionReq()
            apportionReq.user_id = 3673759
            apportionReq.sellers = {
                            2477030: sellerInfo_1,
                            3102766: sellerInfo_2,
                        }
            apportionReq.plat_code = 'vjcrrxjscss'

            print ThriftToSimpleJSON(CouponECClient(server_ip, server_port).getApportion(apportionReq))
        # ---------------------------------------------------------------------------------------------------------------

        if False:
            cacheCouponGroupsReq = CacheCouponGroupsReq()
            cacheCouponGroupsReq.coupon_group_id_set = [2]
            print ThriftToSimpleJSON(CouponECClient(server_ip, server_port).getCacheCouponGroups(cacheCouponGroupsReq))

        # ---------------------------------------------------------------------------------------------------------------
        if False:
            relatedCouponGroupsReq = RelatedCouponGroupsReq()
            relatedCouponGroupsReq.coupon_group_id = 52
            print ThriftToSimpleJSON(CouponECClient(server_ip, server_port).getRelatedCouponGroups(relatedCouponGroupsReq))

        # ---------------------------------------------------------------------------------------------------------------
        if False:
            wareLabelReq = WareLabelReq()
            wareLabelReq.ware_label_id = 17
            print ThriftToSimpleJSON(CouponECClient(server_ip, server_port).getWareLabel(wareLabelReq))

        # ---------------------------------------------------------------------------------------------------------------

    except Thrift.TException as tx:
        print '%s' % tx.message

if __name__ == '__main__':
    main()
