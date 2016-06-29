#!/usr/bin/python
#coding=utf-8
import threading
from time import ctime,sleep

from CouponAdminClient import CouponAdminClient
from CouponECClient import CouponECClient
from CouponPushClient import CouponPushClient
from coupon.ttypes import InvalidOperation,MultipleCondition
from couponEC.ttypes import DrawReq,FreezeReq,RollbackReq,PayReq,UserCouponsReq
from couponAdmin.ttypes import CouponGroupListReq

# server_ip = "127.0.0.1"
# server_port = 19001

server_ip = "172.18.5.62"
server_port = 19001
# server_port = 19002

def drawCoupon():
    drawReq = DrawReq()
    drawReq.coupon_group_id = 2

    for i in range(10):
        drawReq.user_id=2000+i

        print CouponECClient(server_ip,server_port).drawCoupon(drawReq)
        sleep(0.001)

def freezeCoupon():
    freezeReq1 = FreezeReq()
    freezeReq1.user_id = 1000
    freezeReq1.code = 'wvtcyeqmgrn'
    freezeReq1.order_id = 'POIXXX'

    freezeReq2 = FreezeReq()
    freezeReq2.user_id = 1001
    freezeReq2.code = 'wkdcmwayvvl'
    freezeReq2.order_id = 'POIXXX'

    freezeReq3 = FreezeReq()
    freezeReq3.user_id = 1002
    freezeReq3.code = 'zvxcmmzhcza'
    freezeReq3.order_id = 'POIXXX'

    freezeReq4 = FreezeReq()
    freezeReq4.user_id = 1003
    freezeReq4.code = 'wmlcxgptbeg'
    freezeReq4.order_id = 'POIXXX'

    for i in range(10):
        print CouponECClient(server_ip,server_port).freezeCoupon(freezeReq1)
        print CouponECClient(server_ip,server_port).freezeCoupon(freezeReq2)
        print CouponECClient(server_ip,server_port).freezeCoupon(freezeReq3)
        print CouponECClient(server_ip,server_port).freezeCoupon(freezeReq4)
        sleep(0.001)

def rollbackCoupon():
    rollbackReq1 = RollbackReq()
    rollbackReq1.user_id = 1000
    rollbackReq1.code = 'wvtcyeqmgrn'
    rollbackReq1.order_id = 'POIXXX'

    rollbackReq2 = RollbackReq()
    rollbackReq2.user_id = 1001
    rollbackReq2.code = 'wkdcmwayvvl'
    rollbackReq2.order_id = 'POIXXX'

    rollbackReq3 = RollbackReq()
    rollbackReq3.user_id = 1002
    rollbackReq3.code = 'zvxcmmzhcza'
    rollbackReq3.order_id = 'POIXXX'

    rollbackReq4 = RollbackReq()
    rollbackReq4.user_id = 1003
    rollbackReq4.code = 'wmlcxgptbeg'
    rollbackReq4.order_id = 'POIXXX'

    for i in range(10):
        print CouponECClient(server_ip,server_port).rollbackCoupon(rollbackReq1)
        print CouponECClient(server_ip,server_port).rollbackCoupon(rollbackReq2)
        print CouponECClient(server_ip,server_port).rollbackCoupon(rollbackReq3)
        print CouponECClient(server_ip,server_port).rollbackCoupon(rollbackReq4)
        sleep(0.001)

def payCoupon():
    payCoupon1 = PayReq()
    payCoupon1.user_id = 1000
    payCoupon1.code = 'wvtcyeqmgrn'
    payCoupon1.order_id = 'POIXXX'

    payCoupon2 = PayReq()
    payCoupon2.user_id = 1001
    payCoupon2.code = 'wkdcmwayvvl'
    payCoupon2.order_id = 'POIXXX'

    payCoupon3 = PayReq()
    payCoupon3.user_id = 1002
    payCoupon3.code = 'zvxcmmzhcza'
    payCoupon3.order_id = 'POIXXX'

    payCoupon4 = PayReq()
    payCoupon4.user_id = 1003
    payCoupon4.code = 'wmlcxgptbeg'
    payCoupon4.order_id = 'POIXXX'

    for i in range(10):
        print CouponECClient(server_ip,server_port).payCoupon(payCoupon1)
        print CouponECClient(server_ip,server_port).payCoupon(payCoupon2)
        print CouponECClient(server_ip,server_port).payCoupon(payCoupon3)
        print CouponECClient(server_ip,server_port).payCoupon(payCoupon4)
        sleep(0.001)

def getUserCoupons():
    userCouponsReq = UserCouponsReq()
    userCouponsReq.user_id = 5006937
    userCouponsReq.page = 1
    userCouponsReq.count = 20
    userCouponsReq.use_status = 3

    for i in range(1000):
        print CouponECClient(server_ip, server_port).getUserCoupons(userCouponsReq)
        sleep(0.001)

def getCouponGroupList():
    multipleCondition = MultipleCondition()
    multipleCondition.andCondList=[]

    couponGroupListReq = CouponGroupListReq()
    couponGroupListReq.cond = multipleCondition
    couponGroupListReq.offset = 0
    couponGroupListReq.rows = 100

    for i in range(1):
        print CouponAdminClient(server_ip,server_port).getCouponGroupList(couponGroupListReq)
        sleep(0.001)

def main():
    threads = []

    for i in range(4):
        t2 = threading.Thread(target=getUserCoupons, args=())
        threads.append(t2)

    for t in threads:
        t.setDaemon(True)
        t.start()

    for t in threads:
        t.join()


if __name__ == '__main__':
    main()
