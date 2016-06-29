#!/usr/bin/python
#coding=utf-8
import sys
sys.path.append('gen_py')

from ThriftHelper import JSONToSimpleJSON
from coupon.ttypes import *
from couponEC.ttypes import *
from couponAdmin.ttypes import *

'''
    usage:
            python json_to_simpleJson.py CouponCountReq 'jsonprotocal'
    example:
            ./json_to_simpleJson.py CouponCountRsp '{"1":{"i32":0},"2":{"str":""},"3":{"i32":16}}'
            {"error":0,"errmsg":"","count":16}
'''

def main(argv):
    couponMod = sys.modules["coupon.ttypes"]
    couponECMod = sys.modules["couponEC.ttypes"]
    couponAdminMod = sys.modules["couponAdmin.ttypes"]

    thrift_class = None
    if hasattr(couponMod,argv[1]):
        thrift_class = getattr(couponMod,argv[1])
    else:
        if hasattr(couponECMod,argv[1]):
            thrift_class = getattr(couponECMod,argv[1])
        else:
            if hasattr(couponAdminMod,argv[1]):
                thrift_class = getattr(couponAdminMod,argv[1])

    if not thrift_class:
        print argv[1] ,'is not a class'
    else:
        print JSONToSimpleJSON(thrift_class,argv[2])

if __name__ == '__main__':
    main(sys.argv)