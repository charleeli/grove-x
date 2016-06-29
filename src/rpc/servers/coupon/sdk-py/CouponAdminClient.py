# -*- coding:utf-8 -*-
import sys
sys.path.append('gen_py')

from ThriftClient import ThriftClient
from couponAdmin import CouponAdminService
from coupon.ttypes import InvalidOperation

class CouponAdminClient(object):
    def __init__(self,ip,port):
        self.ip = ip
        self.port = port

    def get_client(self):
        return ThriftClient(CouponAdminService, self.ip, self.port)

    def batchExportCode(self,batchExportCodeReq):
        try:
            tclient = self.get_client()
            batchExportCodeRsp = tclient.client.batchExportCode(batchExportCodeReq)
            tclient.close()
            return batchExportCodeRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def batchExport(self,batchExportReq):
        try:
            tclient = self.get_client()
            batchExportRsp = tclient.client.batchExport(batchExportReq)
            tclient.close()
            return batchExportRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def batchDispatch(self,batchDispatchReq):
        try:
            tclient = self.get_client()
            batchDispatchRsp = tclient.client.batchDispatch(batchDispatchReq)
            tclient.close()
            return batchDispatchRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def verifyArgot(self,verifyArgotReq):
        try:
            tclient = self.get_client()
            verifyArgotRsp = tclient.client.verifyArgot(verifyArgotReq)
            tclient.close()
            return verifyArgotRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def createCouponTable(self,createCouponTableReq):
        try:
            tclient = self.get_client()
            createCouponTableRsp = tclient.client.createCouponTable(createCouponTableReq)
            tclient.close()
            return createCouponTableRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def getCouponCount(self,couponCountReq):
        try:
            tclient = self.get_client()
            couponCountRsp = tclient.client.getCouponCount(couponCountReq)
            tclient.close()
            return couponCountRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def getCouponList(self,couponListReq):
        try:
            tclient = self.get_client()
            couponListRsp = tclient.client.getCouponList(couponListReq)
            tclient.close()
            return couponListRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def getCouponGroupCount(self,couponGroupCountReq):
        try:
            tclient = self.get_client()
            couponGroupCountRsp = tclient.client.getCouponGroupCount(couponGroupCountReq)
            tclient.close()
            return couponGroupCountRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def getCouponGroupList(self,couponGroupListReq):
        try:
            tclient = self.get_client()
            couponGroupListRsp = tclient.client.getCouponGroupList(couponGroupListReq)
            tclient.close()
            return couponGroupListRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def getWareLabelCount(self,wareLabelCountReq):
        try:
            tclient = self.get_client()
            wareLabelCountRsp = tclient.client.getWareLabelCount(wareLabelCountReq)
            tclient.close()
            return wareLabelCountRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def getWareLabelList(self,wareLabelListReq):
        try:
            tclient = self.get_client()
            wareLabelListRsp = tclient.client.getWareLabelList(wareLabelListReq)
            tclient.close()
            return wareLabelListRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def getWareLabelWaresCount(self,wareLabelWaresCountReq):
        try:
            tclient = self.get_client()
            wareLabelWaresCountRsp = tclient.client.getWareLabelWaresCount(wareLabelWaresCountReq)
            tclient.close()
            return wareLabelWaresCountRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def getWareLabelWaresList(self,wareLabelWaresListReq):
        try:
            tclient = self.get_client()
            wareLabelWaresListRsp = tclient.client.getWareLabelWaresList(wareLabelWaresListReq)
            tclient.close()
            return wareLabelWaresListRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def createWareLabel(self,createWareLabelReq):
        try:
            tclient = self.get_client()
            createWareLabelRsp = tclient.client.createWareLabel(createWareLabelReq)
            tclient.close()
            return createWareLabelRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def updateWareLabel(self,updateWareLabelReq):
        try:
            tclient = self.get_client()
            updateWareLabelRsp = tclient.client.updateWareLabel(updateWareLabelReq)
            tclient.close()
            return updateWareLabelRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def addWareLabelWares(self,addWareLabelWaresReq):
        try:
            tclient = self.get_client()
            addWareLabelWaresRsp = tclient.client.addWareLabelWares(addWareLabelWaresReq)
            tclient.close()
            return addWareLabelWaresRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def delWareLabelWares(self,delWareLabelWaresReq):
        try:
            tclient = self.get_client()
            delWareLabelWaresRsp = tclient.client.delWareLabelWares(delWareLabelWaresReq)
            tclient.close()
            return delWareLabelWaresRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def createCouponGroup(self,createCouponGroupReq):
        try:
            tclient = self.get_client()
            createCouponGroupRsp = tclient.client.createCouponGroup(createCouponGroupReq)
            tclient.close()
            return createCouponGroupRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def updateCouponGroup(self,updateCouponGroupReq):
        try:
            tclient = self.get_client()
            updateCouponGroupRsp = tclient.client.updateCouponGroup(updateCouponGroupReq)
            tclient.close()
            return updateCouponGroupRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def batchUserCoupon(self,batchUserCouponReq):
        try:
            tclient = self.get_client()
            batchUserCouponRsp = tclient.client.batchUserCoupon(batchUserCouponReq)
            tclient.close()
            return batchUserCouponRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def updateJump(self,updateJumpReq):
        try:
            tclient = self.get_client()
            updateJumpRsp = tclient.client.updateJump(updateJumpReq)
            tclient.close()
            return updateJumpRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def updateArgotJump(self,updateArgotJumpReq):
        try:
            tclient = self.get_client()
            updateArgotJumpRsp = tclient.client.updateArgotJump(updateArgotJumpReq)
            tclient.close()
            return updateArgotJumpRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def updateOnline(self,updateOnlineReq):
        try:
            tclient = self.get_client()
            updateOnlineRsp = tclient.client.updateOnline(updateOnlineReq)
            tclient.close()
            return updateOnlineRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))



