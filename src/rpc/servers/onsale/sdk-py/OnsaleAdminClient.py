# -*- coding:utf-8 -*-
import sys
sys.path.append('gen_py')

from ThriftClient import ThriftClient
from onsaleAdmin import OnsaleAdminService
from onsale.ttypes import InvalidOperation


class OnsaleAdminClient(object):
    def __init__(self, ip, port):
        self.ip = ip
        self.port = port

    def get_client(self):
        return ThriftClient(OnsaleAdminService, self.ip, self.port)

    def checkCacheNameList(self,checkCacheNameListReq):
        try:
            tclient = self.get_client()
            checkCacheNameListRsp = tclient.client.checkCacheNameList(checkCacheNameListReq)
            tclient.close()
            return checkCacheNameListRsp
        except InvalidOperation as e:
            print('InvalidOperation: %r' % e)

    def viewCheckCacheNameList(self,viewCheckCacheNameListReq):
        try:
            tclient = self.get_client()
            viewCheckCacheNameListRsp = tclient.client.viewCheckCacheNameList(viewCheckCacheNameListReq)
            tclient.close()
            return viewCheckCacheNameListRsp
        except InvalidOperation as e:
            print('InvalidOperation: %r' % e)

    def addCacheNameList(self,addCacheNameListReq):
        try:
            tclient = self.get_client()
            addCacheNameListRsp = tclient.client.addCacheNameList(addCacheNameListReq)
            tclient.close()
            return addCacheNameListRsp
        except InvalidOperation as e:
            print('InvalidOperation: %r' % e)

    def delCacheNameList(self,delCacheNameListReq):
        try:
            tclient = self.get_client()
            delCacheNameListRsp = tclient.client.delCacheNameList(delCacheNameListReq)
            tclient.close()
            return delCacheNameListRsp
        except InvalidOperation as e:
            print('InvalidOperation: %r' % e)

    def getCacheNameList(self,getCacheNameListReq):
        try:
            tclient = self.get_client()
            getCacheNameListRsp = tclient.client.getCacheNameList(getCacheNameListReq)
            tclient.close()
            return getCacheNameListRsp
        except InvalidOperation as e:
            print('InvalidOperation: %r' % e)

    def offline(self,offlineReq):
        try:
            tclient = self.get_client()
            offlineRsp = tclient.client.offline(offlineReq)
            tclient.close()
            return offlineRsp
        except InvalidOperation as e:
            print('InvalidOperation: %r' % e)

    def createNameList(self,createNameListReq):
        try:
            tclient = self.get_client()
            createNameListRsp = tclient.client.createNameList(createNameListReq)
            tclient.close()
            return createNameListRsp
        except InvalidOperation as e:
            print('InvalidOperation: %r' % e)

    def addStep(self,addStepReq):
        try:
            tclient = self.get_client()
            addStepRsp = tclient.client.addStep(addStepReq)
            tclient.close()
            return addStepRsp
        except InvalidOperation as e:
            print('InvalidOperation: %r' % e)

    def delStep(self,delStepReq):
        try:
            tclient = self.get_client()
            delStepRsp = tclient.client.delStep(delStepReq)
            tclient.close()
            return delStepRsp
        except InvalidOperation as e:
            print('InvalidOperation: %r' % e)

    def getStepList(self,getStepListReq):
        try:
            tclient = self.get_client()
            getStepListRsp = tclient.client.getStepList(getStepListReq)
            tclient.close()
            return getStepListRsp
        except InvalidOperation as e:
            print('InvalidOperation: %r' % e)

    def addPresent(self,addPresentReq):
        try:
            tclient = self.get_client()
            addPresentRsp = tclient.client.addPresent(addPresentReq)
            tclient.close()
            return addPresentRsp
        except InvalidOperation as e:
            print('InvalidOperation: %r' % e)

    def delPresent(self,delPresentReq):
        try:
            tclient = self.get_client()
            delPresentRsp = tclient.client.delPresent(delPresentReq)
            tclient.close()
            return delPresentRsp
        except InvalidOperation as e:
            print('InvalidOperation: %r' % e)

    def getPresentList(self,getPresentListReq):
        try:
            tclient = self.get_client()
            getPresentListRsp = tclient.client.getPresentList(getPresentListReq)
            tclient.close()
            return getPresentListRsp
        except InvalidOperation as e:
            print('InvalidOperation: %r' % e)

    def createWareLabel(self,createWareLabelReq):
        try:
            tclient = self.get_client()
            createWareLabelRsp = tclient.client.createWareLabel(createWareLabelReq)
            tclient.close()
            return createWareLabelRsp
        except InvalidOperation as e:
            print('InvalidOperation: %r' % e)

    def updateWareLabel(self,updateWareLabelReq):
        try:
            tclient = self.get_client()
            updateWareLabelRsp = tclient.client.updateWareLabel(updateWareLabelReq)
            tclient.close()
            return updateWareLabelRsp
        except InvalidOperation as e:
            print('InvalidOperation: %r' % e)

    def addWareLabelWares(self,addWareLabelWaresReq):
        try:
            tclient = self.get_client()
            addWareLabelWaresRsp = tclient.client.addWareLabelWares(addWareLabelWaresReq)
            tclient.close()
            return addWareLabelWaresRsp
        except InvalidOperation as e:
            print('InvalidOperation: %r' % e)

    def delWareLabelWares(self,delWareLabelWaresReq):
        try:
            tclient = self.get_client()
            delWareLabelWaresRsp = tclient.client.delWareLabelWares(delWareLabelWaresReq)
            tclient.close()
            return delWareLabelWaresRsp
        except InvalidOperation as e:
            print('InvalidOperation: %r' % e)

    def getWareLabelWaresCount(self,wareLabelWaresCountReq):
        try:
            tclient = self.get_client()
            wareLabelWaresCountRsp = tclient.client.getWareLabelWaresCount(wareLabelWaresCountReq)
            tclient.close()
            return wareLabelWaresCountRsp
        except InvalidOperation as e:
            print('InvalidOperation: %r' % e)

    def getWareLabelWaresList(self,wareLabelWaresListReq):
        try:
            tclient = self.get_client()
            wareLabelWaresListRsp = tclient.client.getWareLabelWaresList(wareLabelWaresListReq)
            tclient.close()
            return wareLabelWaresListRsp
        except InvalidOperation as e:
            print('InvalidOperation: %r' % e)

    def createOnsaleGroup(self,createOnsaleGroupReq):
        try:
            tclient = self.get_client()
            rsp = tclient.client.createOnsaleGroup(createOnsaleGroupReq)
            tclient.close()
            return rsp
        except InvalidOperation as e:
            print('InvalidOperation: %r' % e)

    def updateOnsaleGroup(self,updateOnsaleGroupReq):
        try:
            tclient = self.get_client()
            updateOnsaleGroupRsp = tclient.client.updateOnsaleGroup(updateOnsaleGroupReq)
            tclient.close()
            return updateOnsaleGroupRsp
        except InvalidOperation as e:
            print('InvalidOperation: %r' % e)

    def getOnsaleGroupCount(self,getOnsaleGroupCountReq):
        try:
            tclient = self.get_client()
            getOnsaleGroupCountRsp = tclient.client.getOnsaleGroupCount(getOnsaleGroupCountReq)
            tclient.close()
            return getOnsaleGroupCountRsp
        except InvalidOperation as e:
            print('InvalidOperation: %r' % e)

    def getOnsaleGroupList(self,getOnsaleGroupListReq):
        try:
            tclient = self.get_client()
            getOnsaleGroupListRsp = tclient.client.getOnsaleGroupList(getOnsaleGroupListReq)
            tclient.close()
            return getOnsaleGroupListRsp
        except InvalidOperation as e:
            print('InvalidOperation: %r' % e)
