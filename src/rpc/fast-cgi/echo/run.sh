#!/bin/bash
killall echo_module
killall -QUIT nginx
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

mkdir -p /data/deploy/etc/fast-cgi/
mkdir -p /data/deploy/logs/
mkdir -p /data/deploy/tools/fast-cgi/

cp ../../../depends/bin/echo_module /data/deploy/etc/fast-cgi/echo_module

cp ./nginx.conf /usr/local/nginx/conf/
/usr/local/nginx/sbin/nginx
../../../depends/bin/spawn-fcgi -a 127.0.0.1 -p 17002 -F 10 -f /data/deploy/etc/fast-cgi/echo_module
