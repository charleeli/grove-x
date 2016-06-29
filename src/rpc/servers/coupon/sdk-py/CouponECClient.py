# -*- coding:utf-8 -*-
import sys
sys.path.append('gen_py')

from ThriftClient import ThriftClient
from couponEC import CouponECService
from coupon.ttypes import InvalidOperation

class CouponECClient(object):
    def __init__(self,ip,port):
        self.ip = ip
        self.port = port

    def get_client(self):
        return ThriftClient(CouponECService, self.ip, self.port)

    def getDefaultConfig(self,getDefaultConfigReq):
        try:
            tclient = self.get_client()
            getDefaultConfigRsp = tclient.client.getDefaultConfig(getDefaultConfigReq)
            tclient.close()
            return getDefaultConfigRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def getDrawnCountToday(self,drawnCountTodayReq):
        try:
            tclient = self.get_client()
            drawnCountTodayRsp = tclient.client.getDrawnCountToday(drawnCountTodayReq)
            tclient.close()
            return drawnCountTodayRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def getNotUsableCoupons(self,notUsableCouponsReq):
        try:
            tclient = self.get_client()
            notUsableCouponsReqRsp = tclient.client.getNotUsableCoupons(notUsableCouponsReq)
            tclient.close()
            return notUsableCouponsReqRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def getUserLeftCouponsCount(self,userLeftCouponsCountReq):
        try:
            tclient = self.get_client()
            userLeftCouponsCountRsp = tclient.client.getUserLeftCouponsCount(userLeftCouponsCountReq)
            tclient.close()
            return userLeftCouponsCountRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def getClientCoupons(self,clientCouponsReq):
        try:
            tclient = self.get_client()
            clientCouponsRsp = tclient.client.getClientCoupons(clientCouponsReq)
            tclient.close()
            return clientCouponsRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def decodeCode(self,decodeCodeReq):
        try:
            tclient = self.get_client()
            decodeCodeRsp = tclient.client.decodeCode(decodeCodeReq)
            tclient.close()
            return decodeCodeRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def checkArgot(self,checkArgotReq):
        try:
            tclient = self.get_client()
            checkArgotRsp = tclient.client.checkArgot(checkArgotReq)
            tclient.close()
            return checkArgotRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def getServerTime(self,serverTimeReq):
        try:
            tclient = self.get_client()
            serverTimeRsp = tclient.client.getServerTime(serverTimeReq)
            tclient.close()
            return serverTimeRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def drawCoupon(self,drawReq):
        try:
            tclient = self.get_client()
            drawRsp = tclient.client.drawCoupon(drawReq)
            tclient.close()
            return drawRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def freezeCoupon(self,freezeReq):
        try:
            tclient = self.get_client()
            freezeRsp = tclient.client.freezeCoupon(freezeReq)
            tclient.close()
            return freezeRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def rollbackCoupon(self,rollbackReq):
        try:
            tclient = self.get_client()
            rollbackRsp = tclient.client.rollbackCoupon(rollbackReq)
            tclient.close()
            return rollbackRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def payCoupon(self,payReq):
        try:
            tclient = self.get_client()
            payRsp = tclient.client.payCoupon(payReq)
            tclient.close()
            return payRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def getUserCoupons(self,userCouponsReq):
        try:
            tclient = self.get_client()
            userCouponsRsp = tclient.client.getUserCoupons(userCouponsReq)
            tclient.close()
            return userCouponsRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def getWareActs(self,wareActsReq):
        try:
            tclient = self.get_client()
            wareActsRsp = tclient.client.getWareActs(wareActsReq)
            tclient.close()
            return wareActsRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def getUsableCoupons(self,usableCouponsReq):
        try:
            tclient = self.get_client()
            usableCouponsRsp = tclient.client.getUsableCoupons(usableCouponsReq)
            tclient.close()
            return usableCouponsRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def getApportion(self,apportionReq):
        try:
            tclient = self.get_client()
            apportionRsp = tclient.client.getApportion(apportionReq)
            tclient.close()
            return apportionRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def getCacheCouponGroups(self,cacheCouponGroupsReq):
        try:
            tclient = self.get_client()
            cacheCouponGroupsRsp = tclient.client.getCacheCouponGroups(cacheCouponGroupsReq)
            tclient.close()
            return cacheCouponGroupsRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def getRelatedCouponGroups(self,relatedCouponGroupsReq):
        try:
            tclient = self.get_client()
            relatedCouponGroupsRsp = tclient.client.getRelatedCouponGroups(relatedCouponGroupsReq)
            tclient.close()
            return relatedCouponGroupsRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def getWareLabel(self,wareLabelReq):
        try:
            tclient = self.get_client()
            wareLabelRsp = tclient.client.getWareLabel(wareLabelReq)
            tclient.close()
            return wareLabelRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))


