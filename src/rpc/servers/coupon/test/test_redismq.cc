#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <ev.h>
#include "ThriftHelper.h"
#include "sdk-cpp/admin/couponAdmin_types.h"
#include "sdk-cpp/ec/couponEC_types.h"

extern "C"{
#include "redismq/redismq.h"
}

#include <string>

using namespace std;
using namespace coupon;
using namespace couponEC;
using namespace couponAdmin;

const string REDIS_HOST = "172.16.1.230";
#define REDIS_PORT 6380
#define REDIS_DB 0
const string  KEY = "coupon_batch_dispatch_list";

static int n = 0;

static void timer_cb(EV_P_ struct ev_timer *timer, int revents)
{
	BatchDispatchReq item;
    item.coupon_group_id = 1;

    printf("sending: %s\n", ThriftToJSON(item).c_str());

    rmq_rpushf((rmq_context*)timer->data, "%s", ThriftToJSON(item).c_str());
    n++;
}

/*
static void blpop_cb(char *msg)
{
    printf("received: %s\n", msg);
    usleep(2);
}
*/

int main(int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);

    // rpush timer

    struct rmq_context push;
    rmq_init(&push, (const char *)REDIS_HOST.c_str(), REDIS_PORT, REDIS_DB, (const char *)KEY.c_str());

    struct ev_timer timer;
    ev_timer_init(&timer, timer_cb, 0., .4);
    timer.data = &push;
    ev_timer_start(EV_DEFAULT_ &timer);

    // blpop handler

    /*
    struct rmq_context pop;
    rmq_init(&pop, (const char *)REDIS_HOST.c_str(), REDIS_PORT, REDIS_DB, (const char *)KEY.c_str());
    rmq_blpop(&pop, blpop_cb);
    */

    // start libev event loop

    ev_loop(EV_DEFAULT_ 0);

    return 0;
}
