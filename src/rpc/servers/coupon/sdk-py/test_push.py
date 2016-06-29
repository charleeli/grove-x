#!/usr/bin/python
#coding=utf-8
from thrift import Thrift
from ThriftHelper import ThriftToSimpleJSON
from CouponPushClient import CouponPushClient
from coupon.ttypes import InvalidOperation
from couponPush.ttypes import NameListReq,PushInfoReq

server_ip = "127.0.0.1"
# server_ip = "172.18.5.62"
server_port = 19000

def main():
    try:
        #---------------------------------------------------------------------------------------------------------------
        if False:
            nameListReq = NameListReq()
            nameListReq.page = 1
            nameListReq.count = 10
            print ThriftToSimpleJSON(CouponPushClient(server_ip,server_port).getNameList(nameListReq))
        #---------------------------------------------------------------------------------------------------------------
        if True:
            pushInfoReq = PushInfoReq()
            pushInfoReq.user_id_set = [10011]
            print ThriftToSimpleJSON(CouponPushClient(server_ip,server_port).getPushInfo(pushInfoReq))
        #---------------------------------------------------------------------------------------------------------------

    except Thrift.TException as tx:
        print(('%s' % (tx.message)))


if __name__ == '__main__':
    main()
