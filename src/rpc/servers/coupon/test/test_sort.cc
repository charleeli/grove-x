#include <iostream>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>

using namespace std;

/*
    ./test_sort
*/

struct CacheCoupon {
    int favor;
    int end_use_time;
};

bool compare(const CacheCoupon a,const CacheCoupon b){
    return a.end_use_time < b.end_use_time;//最快过期
}


int main(int argc, char *argv[])
{
    vector<CacheCoupon> left = {
        {2,3},
        {2,4},
        {3,8},
        {1,2},
        {5,2},
        {3,2},
    };


    std::sort(left.begin(),left.end(),[&](const CacheCoupon a,const CacheCoupon b){
        if(a.end_use_time == b.end_use_time){
            return a.favor > b.favor;//面额最大
        }else{
            return a.end_use_time < b.end_use_time;//最快过期
        }
    });

    //std::sort(left.begin(),left.end(),compare);

    for(auto x : left){
        cout<<x.favor << " " << x.end_use_time <<endl;
    }

    return 0;
}
