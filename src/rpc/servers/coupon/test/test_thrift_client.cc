#include "ThriftClient.h"
#include "sdk-cpp/ec/CouponECService.h"
#include <iostream>

using namespace coupon;
using namespace couponEC;

int main() {
    ThriftClient<CouponECServiceClient> thriftClient("localhost", 19001);

    try {
        try {
            DrawReq drawReq;
            drawReq.coupon_group_id = 1;
            drawReq.user_id = 717066513;

            DrawRsp drawRsp;
            thriftClient.getClient()->drawCoupon(drawRsp, drawReq);

            cout << "error:"<<drawRsp.error<<" errmsg:"<<drawRsp.errmsg<<" code:" <<drawRsp.code<< endl;
        } catch (InvalidOperation& io) {
            cout << "InvalidOperation: " <<io.fault<<" "<< io.why << endl;
        }
    }catch (TException& tx) {
        cout << "ERROR: " << tx.what() << endl;
    }

    return 0;
}
