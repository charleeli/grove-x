#!/bin/bash

mkdir -p /data/deploy/etc/onsale_server/
mkdir -p /data/deploy/logs/
mkdir -p /data/deploy/tools/onsale_server/

cp ../../../rpc/config/daemon/onsale_store_server.json   /data/deploy/etc/onsale_server/onsale_store_server.json
cp ../../../depends/bin/onsale_admin_server     /data/deploy/etc/onsale_server/onsale_admin_server

/data/deploy/etc/onsale_server/onsale_admin_server    /data/deploy/etc/onsale_server/onsale_store_server.json
