# -*- coding:utf-8 -*-
import sys
sys.path.append('gen_py')

from ThriftClient import ThriftClient
from couponPush import CouponPushService
from coupon.ttypes import InvalidOperation

class CouponPushClient(object):
    def __init__(self,ip,port):
        self.ip = ip
        self.port = port

    def get_client(self):
        return ThriftClient(CouponPushService, self.ip, self.port)

    def getNameList(self,nameListReq):
        try:
            tclient = self.get_client()
            nameListRsp = tclient.client.getNameList(nameListReq)
            tclient.close()
            return nameListRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def getPushInfo(self,pushInfoReq):
        try:
            tclient = self.get_client()
            pushInfoRsp = tclient.client.getPushInfo(pushInfoReq)
            tclient.close()
            return pushInfoRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))





