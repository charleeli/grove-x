#include <iostream>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

#include "CouponCode.h"

using namespace std;

/*
    ./test_decode 'vcouytcwpdh'
*/

int main(int argc, char *argv[])
{
    string code = string( argv[1] );

    unsigned int coupon_group_id,coupon_id;
    CouponCode::Decode(code,coupon_group_id,coupon_id);
    cout<<"code:"<<code<<" coupon_group_id:"<<coupon_group_id<<" coupon_id:"<<coupon_id<<endl;
    return 0;
}
