#include <iostream>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

#include "CouponCode.h"

using namespace std;

/*
    ./test_group_slug 1 100 | tee slug.txt
    sort -k1 slug.txt | uniq > res.txt
    wc res.txt -l
*/

int main(int argc, char *argv[])
{
    unsigned min_coupon_coupon_id = atoi( argv[1] );
    if(min_coupon_coupon_id == 0 ){
        cout<<"min_coupon_coupon_id can't le 0"<<endl;
        return 0;
    }

    unsigned max_coupon_coupon_id = atoi( argv[2] );
    if(max_coupon_coupon_id == 0){
        cout<<"max_coupon_coupon_id can't le 0"<<endl;
        return 0;
    }

    if(min_coupon_coupon_id >= max_coupon_coupon_id){
        cout<<"min_coupon_coupon_id >= max_coupon_coupon_id"<<endl;
        return 0;
    }

    for(unsigned int i=min_coupon_coupon_id;i<=max_coupon_coupon_id;i++)
    {
        string slug="";
        int err = CouponCode::GenCouponGroupSlug(i,slug);
        if(err==CouponCode::Errno::OK)
        {
            unsigned int _coupon_group_id;
            err = CouponCode::DecodeCouponGroupSlug(slug,_coupon_group_id);
            if(err==CouponCode::Errno::OK)
            {
                cout <<i<<" "<<slug<<' '<<_coupon_group_id<<endl;
                //cout <<slug<<endl;
            }
        }
    }

    return 0;
}
