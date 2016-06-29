#ifndef ONSALE_HXX
#define ONSALE_HXX

#include <string>
#include <cstddef>
#include <memory>
#include <odb/core.hxx>
#include <odb/nullable.hxx>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/unordered_set.hpp>
#include <odb/boost/exception.hxx>
#include <odb/boost/date-time/exceptions.hxx>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;
using namespace boost::posix_time;

#pragma db object session
struct onsale_warelabel {                       //商品标签
    #pragma db id auto
    unsigned long   id;                         //标签id
    string          name;                       //名称
    int             label_type;                 //商品标签类型
    int             scope_type;                 //使用范围
    int             sub_type;                   //商品名单子类型
    int64_t         seller_id;                  //店铺id
    string          seller_slug;                //店铺slug
    ptime           create_time;                //创建时间
    ptime           update_time;                //更新时间
    string          create_man;                 //创建人
};

#pragma db view object(onsale_warelabel)
struct onsale_warelabel_stat {                  //商品标签视图
    #pragma db column("count(*)")
    size_t          count;                      //总数

    #pragma db column("max(id)")
    unsigned int    max_id;                     //目前最大id
};

#pragma db object session no_id
struct onsale_warelabelwares {                  //商品标签-商品表
    unsigned int    ware_label_id;              //商品标签id
    int64_t         ware_id;                    //商品id
    ptime           create_time;                //创建时间
};

#pragma db view object(onsale_warelabelwares)
struct onsale_warelabelwares_stat {             //商品标签-商品视图
    #pragma db column("count(*)")
    size_t          count;                      //总数
};


#pragma db object session no_id
struct onsale_present {                         //赠品表
    unsigned int    step_id;                    //阶梯ID
    int64_t         sku_id;                     //商品id
    double          sku_price;                  //商品价格
    unsigned int    sku_count;                  //商品个数
    string          sku_slug;                   //商品slug
};

#pragma db view object(onsale_present)
struct onsale_present_stat {                    //赠品视图
    #pragma db column("count(*)")
    size_t          count;                      //总数
};

#pragma db object session
struct onsale_step {                            //优惠阶梯表
    #pragma db id auto
    unsigned long   id;                         //阶梯id
    unsigned int    onsale_group_id;            //促销活动ID
    double          full_credit;                //购满金额
    double          favor_credit;               //免减金额
    double          favor_rate;                 //打折折扣率
    unsigned int    full_count;                 //购满件数
    double          full_rate;                  //一件的优惠率,用于满N件优惠
    double          full_price;                 //金额,用于N件任买
};

#pragma db view object(onsale_step)
struct onsale_step_stat {                       //优惠阶梯视图
    #pragma db column("count(*)")
    size_t          count;                      //总数
};

#pragma db object session
struct onsale_onsalegroup {                     //促销活动表
    #pragma db id auto
    unsigned long   id;                         //促销活动id
    string          slug;                       //slug
    string          name;                       //名称
    string          title;                      //促销活动文案
    string          comment;                    //促销活动备注
    int             favor_type;                 //优惠类型
    int             label_type;                 //商品标签类型
    int             ware_label_id;              //商品标签ID
    int             involve_count;              //参与次数
    ptime           start_time;                 //活动开始时间
    ptime           end_time;                   //活动结束时间
    ptime           create_time;                //创建时间
    ptime           update_time;                //更新时间
    int             verify_status;              //审批状态
    string          applicant;                  //申请人
    string          approver;                   //审批人
    string          modifier;                   //修改人
    string          jump_label;                 //跳转类型
    string          jump_data;                  //跳转数据
};

#pragma db view object(onsale_onsalegroup)
struct onsale_onsalegroup_stat {                //促销活动视图
    #pragma db column("count(*)")
    size_t          count;                      //总数

    #pragma db column("max(id)")
    unsigned int    max_id;                     //目前最大id
};

#endif
