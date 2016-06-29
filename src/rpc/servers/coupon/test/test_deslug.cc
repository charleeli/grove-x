#include <iostream>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

#include "CouponCode.h"

using namespace std;

/*
    ./test_deslug 'vcouytcw'
*/

int main(int argc, char *argv[])
{
    string slug = string( argv[1] );

    unsigned int coupon_group_id;

    CouponCode::DecodeCouponGroupSlug(slug,coupon_group_id);

    cout<<"slug:"<<slug<<" coupon_group_id:"<<coupon_group_id<<endl;
    return 0;
}
