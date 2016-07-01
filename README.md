Basic requirements
```
sudo apt-get install automake bison flex g++ git gitg make cmake pkg-config 
sudo apt-get install libboost-all-dev libevent-dev libssl-dev libtool libmysqlclient-dev libidn11-dev
sudo apt-get install python-all python-all-dev python-all-dbg python3-dev python3-venv
```

Install thrift
```
cd thrift-0.9.2
./configure --without-cpp --without-java --without-ruby --without-lua
make 
sudo make install 

cd ./lib/py
sudo python setup.py install 
```

Ubuntu center install
```
MySQL Server
MySQL Client
MySQL Workbench

wget http://www.codesynthesis.com/download/odb/2.4/odb_2.4.0-1_amd64.deb
wget https://github.com/uglide/RedisDesktopManager/releases/download/0.8.3/redis-desktop-manager_0.8.3-120_amd64.deb
```

Building from source
```
1. Compile
cd ./src
./build.sh

2. Clean
cd ./src 
./build.sh dc
```

Install valgrind
```
wget http://valgrind.org/downloads/valgrind-3.11.0.tar.bz2
tar -xvf valgrind-3.11.0.tar.bz2
cd valgrind-3.11.0
./autogen.sh
./configure
make
sudo make install
valgrind --help

cd ./meila_rpc_cc/src/rpc/servers/coupon/
valgrind ../../../depends/bin/coupon_ec_server
sudo valgrind --log-file=memcheck_ec.log ../../../depends/bin/coupon_ec_server ../../config/daemon/coupon_store_server.json &
```
