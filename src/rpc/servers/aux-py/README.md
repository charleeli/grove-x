## aux-py环境
```
	ubuntu 15.10
```

## 如何安装依赖
```
1.安装pyvenv
	sudo apt-get install python3-dev python3-venv

2.启动虚拟环境
	cd aux-py
	pyvenv env
	source ./env/bin/activate

以下均在虚拟环境操作：
3.安装第三方依赖
	cd ~/aux-py
	pip3 install -r requirement.txt
```

## 如何创建工程
```
1.新建工程
	cd ~/aux-py
	django-admin startproject ecsite

2.建立数据库
    导入ecsite.sql

	编辑mysite/settings.py
	设置TIME_ZONE
	cd ~/aux-py/ecsite
	python manage.py migrate
```

## 如何创建APP
```
1.创建app
	cd ~/aux-py/ecsite
	python manage.py startapp coupon
	python manage.py startapp onsale

2.注册app
	编辑ecsite/settings.py

3.创建模型
	编辑coupon/models.py

4.检查模型
	python manage.py check

5.激活模型
	python manage.py makemigrations coupon
	python manage.py makemigrations onsale

6.运行迁移文件
	python manage.py sqlmigrate coupon 0001
	python manage.py sqlmigrate onsale 0001

7.创建表
	python manage.py migrate
```

## 如何创建管理端
```
1.创建一个管理员用户
	python manage.py createsuperuser
	ecsiteadmin
	ecsiteadmin@example.com
	ecsiteadmin

2.启动开发服务器
	python manage.py runserver

3.登陆
	http://127.0.0.1:8000/admin/
```

## 升级数据库
```
1.启动虚拟环境
	cd aux-py
	pyvenv env
	source ./env/bin/activate

2.检查模型
	cd ./ecsite
	python manage.py check

3.激活模型
	python manage.py makemigrations coupon
	python manage.py makemigrations onsale

4.运行迁移文件
	python manage.py sqlmigrate coupon 0002
	python manage.py sqlmigrate onsale 0001

5.创建表
	python manage.py migrate
```





