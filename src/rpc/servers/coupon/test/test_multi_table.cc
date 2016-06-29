#include <thrift/server/TNonblockingServer.h>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <pthread.h>
#include "GLogHelper.h"
#include "mysqlwrapper.h"
#include "MysqlWrapperPool.h"
#include "CouponHelper.h"
#include "RandomUtil.h"
#include "System.h"

using namespace std;
using namespace System;

/*
 * 查询数据库券表数量
 * SELECT count(TABLE_NAME) FROM information_schema.TABLES WHERE TABLE_SCHEMA='ecsite';
 *
 * usage: ./test_multi_table    group_start  group_end  coupon_count
 * example: ./test_multi_table 1 100 10
 * 1.929 ms
 * 2.129 ms
 * 2.129 ms
 *
 * example: ./test_multi_table 1 1000 10
 * 1.752 ms
 * 1.797 ms
 * 1.371 ms
 *
 * example: ./test_multi_table 1 3000 10
 * 1.947 ms
 * 1.608 ms
 * 1.532 ms
 *
 * example: ./test_multi_table 3000 10000 10
 * 1.658 ms
 * 1.285 ms
 * 1.429 ms
 */

int main(int argc, char ** argv){
    int pos = string(argv[0]).find_last_of('/');
    string program = string(argv[0]).substr(pos+1);
    GLogHelper gh(program,"");

    MysqlWrapper mwrapper("172.16.1.230",3310,"root","","ecsite");
    CouponHelper couponHelper(&mwrapper);
    MysqlQueryHelper queryHelper(&mwrapper);

    //创建n张券表
    int coupon_group_start_id = atoi( argv[1] );
    int coupon_group_end_id = atoi( argv[2] );

    //创建n张券
    int coupon_count = atoi( argv[3] );

    for(int coupon_group_id = coupon_group_start_id;coupon_group_id<=coupon_group_end_id;coupon_group_id++){
        //创建一张券表
        couponHelper.createCouponTable(coupon_group_id);

        //往券表插入n条记录
        for(int i = 1;i<=coupon_count;i++){
            unsigned int coupon_id ;
            string code;
            try{
                couponHelper.drawOneCoupon(coupon_group_id,0,coupon_id,code);
            }catch(InvalidOperation &io){
                cout<<"io occured when drawOneCoupon "<<io.why<<endl;
            }

            cout<<"coupon_group_id:"<<coupon_group_id<<" coupon_id:"<<coupon_id<<" code:"<<code<<endl;
        }
    }

    //随机一张券表，从中随机查询一条记录
    {
        PerfWatch perfWatch("mysql runSelect 1");
        unsigned int coupon_group_rdm_id = RandomUtil::Random(coupon_group_start_id,coupon_group_end_id+1);
        unsigned int coupon_rdm_id = RandomUtil::Random(1,coupon_count+1);

        int ret = queryHelper.runSelect(string("")
            +" select *"
            +" from " + coupon_table_prefix + to_string(coupon_group_rdm_id)
            +" where id = " + to_string(coupon_rdm_id)
        );

        cout<<"runSelect 1 ret:"<<ret<<endl;
    }

    //查询coupon_group_start_id券表，随机查询一张券
    {
        PerfWatch perfWatch("mysql runSelect 2");

        unsigned int coupon_rdm_id = RandomUtil::Random(1,coupon_count+1);

        int ret = queryHelper.runSelect(string("")
            +" select *"
            +" from " + coupon_table_prefix + to_string(coupon_group_start_id)
            +" where id = " + to_string(coupon_rdm_id)
        );

        cout<<"runSelect 2 ret:"<<ret<<endl;
    }

    //查询coupon_group_end_id-1券表，随机查询一张券
    {
        PerfWatch perfWatch("mysql runSelect 3");

        unsigned int coupon_rdm_id = RandomUtil::Random(1,coupon_count+1);

        int ret = queryHelper.runSelect(string("")
            +" select *"
            +" from " + coupon_table_prefix + to_string(coupon_group_end_id-1)
            +" where id = " + to_string(coupon_rdm_id)
        );

        cout<<"runSelect 3 ret:"<<ret<<endl;
    }

    return 0;
}
