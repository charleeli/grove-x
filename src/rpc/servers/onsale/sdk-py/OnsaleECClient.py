# -*- coding:utf-8 -*-
import sys
sys.path.append('gen_py')

from ThriftClient import ThriftClient
from onsaleEC import OnsaleECService
from onsale.ttypes import InvalidOperation

class OnsaleECClient(object):
    def __init__(self,ip,port):
        self.ip = ip
        self.port = port

    def get_client(self):
        return ThriftClient(OnsaleECService, self.ip, self.port)

    def getStepInfo(self, stepInfoReq):
        try:
            tclient = self.get_client()
            stepInfoRsp = tclient.client.getStepInfo(stepInfoReq)
            tclient.close()
            return stepInfoRsp
        except InvalidOperation as e:
            print('InvalidOperation: %r' % e)

    def getApportion(self, apportionReq):
        try:
            tclient = self.get_client()
            apportionRsp = tclient.client.getApportion(apportionReq)
            tclient.close()
            return apportionRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))

    def getUsableOnsaleGroups(self,usableOnsaleGroupsReq):
        try:
            tclient = self.get_client()
            usableOnsaleGroupsRsp = tclient.client.getUsableOnsaleGroups(usableOnsaleGroupsReq)
            tclient.close()
            return usableOnsaleGroupsRsp
        except InvalidOperation as e:
            print(('InvalidOperation: %r' % e))
