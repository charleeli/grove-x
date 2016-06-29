#!/usr/bin/python
# coding=utf-8

import threading
from time import sleep

from OnsaleECClient import OnsaleECClient
from onsaleEC.ttypes import SkuInfo, UsableOnsaleGroupsReq, ApportionReq

server_ip = "127.0.0.1"
server_port = 18001

server_ip = "172.18.5.62"


def getApportion():
    skuInfo_1 = SkuInfo()
    skuInfo_1.sku_id = 12300
    skuInfo_1.sku_count = 1
    skuInfo_1.sale_price = 50
    skuInfo_1.market_price = 10
    skuInfo_1.ware_id = 123

    skuInfo_2 = SkuInfo()
    skuInfo_2.sku_id = 23400
    skuInfo_2.sku_count = 1
    skuInfo_2.sale_price = 56
    skuInfo_2.market_price = 100
    skuInfo_2.ware_id = 234

    skuInfo_3 = SkuInfo()
    skuInfo_3.sku_id = 34500
    skuInfo_3.sku_count = 1
    skuInfo_3.sale_price = 90
    skuInfo_3.market_price = 199
    skuInfo_3.ware_id = 345

    apportionReq = ApportionReq()
    apportionReq.sku_list = [skuInfo_1, skuInfo_2, skuInfo_3, ]

    for i in range(1000):
        print OnsaleECClient(server_ip, server_port).getApportion(apportionReq)
        sleep(0.001)


def main():
    threads = []

    for i in range(1):
        t2 = threading.Thread(target=getApportion, args=())
        threads.append(t2)

    for t in threads:
        t.setDaemon(True)
        t.start()

    for t in threads:
        t.join()

if __name__ == '__main__':
    main()
