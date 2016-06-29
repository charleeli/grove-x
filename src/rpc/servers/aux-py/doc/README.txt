券压力测试数据库
    mysql -uroot -h172.18.5.62 -P 3310 -A
    
券测试环境数据库
    mysql -uroot -h172.18.5.62 -P 3309 -A
    注意：修改字符集utf8mb4 时区 use卡顿优化
    
    redis库
    telnet 172.18.5.62 6380

	//删除当前数据库中的所有Key
	flushdb
	//删除所有数据库中的key
	flushall
    
启动
    sudo ./run_admin.sh 
    sudo ./run_ec.sh 
	sudo ./run_push.sh 
	sudo ./run_dispatch.sh
	sudo ./run_export.sh
	
	ps -efL|grep coupon_ec_server
	
TAB替换为空格
    :set ts=4
    :set expandtab
    :%retab!
