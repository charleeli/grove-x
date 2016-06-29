#include <iostream>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

#include "CouponCode.h"

using namespace std;

/*
    ./test_encode 100 2
*/

int main(int argc, char *argv[])
{
    unsigned coupon_group_id = atoi( argv[1] );
    if(coupon_group_id == 0 ){
        cout<<"coupon_group_id can't le 0"<<endl;
        return 0;
    }

    unsigned coupon_id = atoi( argv[2] );
    if(coupon_id == 0){
        cout<<"coupon_id can't le 0"<<endl;
        return 0;
    }

    string code;
    int err = CouponCode::Encode(coupon_group_id,coupon_id,code);
    if(err==CouponCode::Errno::OK)
    {
        unsigned int _coupon_group_id, _coupon_id;
        err = CouponCode::Decode(code,_coupon_group_id,_coupon_id);
        if(err==CouponCode::Errno::OK)
        {
            cout <<coupon_group_id<<" "<<coupon_id<<" "<<code<<' '<<_coupon_group_id<<' '<<_coupon_id<<endl;
            //cout <<code<<endl;
        }
    }

    return 0;
}
