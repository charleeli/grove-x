#!/bin/bash

cp ./doc/nginx.conf  /usr/local/nginx/conf

killall -HUP nginx
/usr/local/nginx/sbin/nginx 
