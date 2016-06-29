#!/bin/bash

mkdir -p /data/deploy/etc/coupon_server/
mkdir -p /data/deploy/logs/
mkdir -p /data/deploy/tools/coupon_server/

cp ../../../rpc/config/daemon/coupon_store_server.json   /data/deploy/etc/coupon_server/coupon_store_server.json
cp ../../../depends/bin/coupon_admin_server /data/deploy/etc/coupon_server/coupon_admin_server

/data/deploy/etc/coupon_server/coupon_admin_server /data/deploy/etc/coupon_server/coupon_store_server.json
