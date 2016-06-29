/*
 * author:charlee
 */
#ifndef _COUPON_CODE_H_
#define _COUPON_CODE_H_

#include <iostream>  
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h> 
#include <vector>

#include "RandomUtil.h"
using namespace std; 

/*
 * 说明：优惠券码必须是在10，000，000张内唯一，不碰撞，且有一定的随机性的11位小写字符串a-z
 *
 * code:11位26进制码
 * coupon_group_id:优惠券组id,最大值不超过4位26进制(26^4=456976)，作为参数时上限设置为300000
 * coupon_id:优惠券id,最大值不超过5位26进制(26^5=11881376)，作为参数时上限设置为10000000
 */
class CouponCode
{
private:
    //将优惠券组ID编码
    static int EncodeCouponGroupID(unsigned int coupon_group_id,string& code);
    //将优惠券ID编码
    static int EncodeCouponID(unsigned int coupon_id,string& code);
public:
    //错误码
    enum Errno
    {
        OK = 0,                         //成功
        INVALID_COUPON_GROUP_ID=-10001, //无效的优惠券组ID
        INVALID_COUPON_ID = -10002,     //无效的优惠券ID
        INVALID_COUPON_CODE = -10003,   //无效的优惠券码
        INVALID_COUPON_GROUP_SLUG=-10004, //无效的优惠券组SLUG
    };
    //根据组ID和券ID计算券码
    static int Encode(unsigned int coupon_group_id,unsigned int coupon_id,string& code);
    //根据券码反算组ID和券ID
    static int Decode(const string& code,unsigned int& coupon_group_id,unsigned int& coupon_id);
    //获取券组slug
    static int GenCouponGroupSlug(unsigned int& coupon_group_id,string& slug);
    //反解券组slug
    static int DecodeCouponGroupSlug(const string& slug,unsigned int& coupon_group_id);
};

//获取券组slug
int CouponCode::GenCouponGroupSlug(unsigned int& coupon_group_id,string& slug){
    slug = "";
    if(Errno::OK == EncodeCouponGroupID(coupon_group_id,slug)){
        for(int j = 1;j <= 4;j++)
        {
            slug += ('a'+RandomUtil::Random(0,26));
        }

        return Errno::OK;
    }

    return Errno::INVALID_COUPON_GROUP_SLUG;
}

//反解券组slug
int CouponCode::DecodeCouponGroupSlug(const string& slug,unsigned int& coupon_group_id){
    coupon_group_id = 0;

    //如果slug不是8位
    if(slug.length() != 8)
    {
        return Errno::INVALID_COUPON_GROUP_SLUG;
    }

    //校验
    for(int i=0;i<8;i++)
    {
        if(slug.at(i)<'a'||slug.at(i)>'z')
        {
            return Errno::INVALID_COUPON_GROUP_SLUG;
        }
    }

    //组ID部分的哨兵
    char sentry = slug.at(0);
    string couponGroupIdCode = "";
    //ux表示有效部分为第2,3,4位
    if(sentry == 'u'||sentry == 'x')
    {
        couponGroupIdCode = slug.substr(1,3);
    }
    //vy表示有效部分为第3,4位
    else if(sentry == 'v'||sentry == 'y')
    {
        couponGroupIdCode = slug.substr(2,2);
    }
    //wz表示有效部分为第4位
    else if(sentry == 'w'||sentry == 'z')
    {
        couponGroupIdCode = slug.substr(3,1);
    }
    //否则全部有效
    else
        couponGroupIdCode = slug.substr(0,4);

    //组ID对应部分的券码长度
    int len1 = couponGroupIdCode.size();
    for(int i=0;i<len1;i++)
    {
        coupon_group_id += (couponGroupIdCode.at(i)-'a')*pow(26,len1-i-1);
    }

    return Errno::OK;
}

//辅助方法，将优惠券组ID编码
int CouponCode::EncodeCouponGroupID(unsigned int coupon_group_id,string& code)
{
    //如果组ID大于30万，目前系统不支持，以后也不支持
    //如果30万组用完了，清楚之前的券组再用
    if(coupon_group_id > 300000)
    {
        //返回组ID无效错误码
        return Errno::INVALID_COUPON_GROUP_ID;
    }

    //清空码
    code="";
    //余数数组
    vector<int> remainders;
    remainders.reserve(10);

    //数制转换，结果存入remainders,从低位到高位存，例如3,15,23...
    int n = coupon_group_id;
    //如果coupon_group_id为0
    if(0 == n)
    {
        code += 'a';
    }

    while (n!=0)
    {
        remainders.push_back(n%26);
        n=n/26;
    }

    //转换为26进制小写字符串,从高位到地位
    int size = remainders.size();
    for(int j=size-1;j>=0;j--)
    {
        code += (char)(remainders[j]+97) ;
    }

    //需要补齐的数目
    int to_full_len = 4-code.length();
    //如果需要补齐
    if(to_full_len > 0)
    {
        //填充的随机字符串
        string rdm_str="";
        //补齐to_full_len-1个随机小写字符
        for(int j=1;j<=to_full_len-1;j++)
        {
            rdm_str += ('a'+RandomUtil::Random(0,26));
        }
        //随机小写字符串填充在code前面
        code = rdm_str + code;

        //最后前面补齐一个哨兵字符，举例:
        //若为‘u’,表示包含‘u’自己一共补了1个字符
        //若为‘v’,表示包含‘v’自己一共补了2个字符
        //若为‘w’,表示包含‘w’自己一共补了3个字符
        string sentries[2] = {"uvw","xyz"};
        string sentry = sentries[RandomUtil::Random(0,2)];
        code = sentry.at(to_full_len-1) + code;
    }

    return Errno::OK;
}

int CouponCode::EncodeCouponID(unsigned int coupon_id,string& code)
{
    //如果券ID大于1千万，系统不支持，1千万张券已经够用
    if(coupon_id > 10000000)
    {
        return Errno::INVALID_COUPON_ID;
    }

    //清空码
    code="";
    //余数数组
    vector<int> remainders;
    remainders.reserve(10);

    //coupon_id扩大100倍，并每隔100个数字随机选一个
    long long int n = coupon_id * 100 + RandomUtil::Random(0,100);
    //小概率事件
    if(0 == n)
    {
        code += 'a';
    }

    //数制转换，结果存入remainders,从低位到高位存，例如3,15,23...
    while (n!=0)
    {
        remainders.push_back(n%26);
        n=n/26;
    }

    //转换为26进制小写字符串,从高位到地位
    int size = remainders.size();
    for(int j=size-1;j>=0;j--)
    {
        code += (char)(remainders[j]+97) ;
    }

    //需要补齐的数目
    int to_full_len = 7-code.length();
    //如果需要补齐
    if(to_full_len > 0)
    {
        //填充的随机字符串
        string rdm_str="";
        //补齐to_full_len-1个随机小写字符
        for(int j=1;j<=to_full_len-1;j++)
        {
            rdm_str += ('a'+RandomUtil::Random(0,26));
        }
        //随机小写字符串填充在code前面
        code = rdm_str + code;

        //最后前面补齐一个哨兵字符
        string sentries[3] = {"ijklmn","opqrst","uvwxyz"};
        string sentry = sentries[RandomUtil::Random(0,3)];
        code = sentry.at(to_full_len-1) + code;
    }

    return Errno::OK;
}

int CouponCode::Encode(unsigned int coupon_group_id,unsigned int coupon_id,string& code)
{
    //组ID部分的码
    string couponGroupIdCode;
    int err = EncodeCouponGroupID(coupon_group_id,couponGroupIdCode);
    if(err != Errno::OK)
    {
        code = "";
        return err;
    }

    //券ID部分的码
    string couponIdCode;
    err = EncodeCouponID(coupon_id,couponIdCode);
    if(err != Errno::OK)
    {
        code = "";
        return err;
    }

    //最终的券码
    code = couponGroupIdCode + couponIdCode;
    return Errno::OK;
}

int CouponCode::Decode(const string& code,unsigned int& coupon_group_id,unsigned int& coupon_id)
{
    //清空引用
    coupon_group_id=0;
    coupon_id = 0;

    //如果券码不是11位
    if(code.length() != 11)
    {
        return Errno::INVALID_COUPON_CODE;
    }

    //校验
    for(int i=0;i<11;i++)
    {
        if(code.at(i)<'a'||code.at(i)>'z')
        {
            return Errno::INVALID_COUPON_CODE;
        }
    }

    //组ID部分的哨兵
    char sentry = code.at(0);
    string couponGroupIdCode = "";
    //ux表示有效部分为第2,3,4位
    if(sentry == 'u'||sentry == 'x')
    {
        couponGroupIdCode = code.substr(1,3);
    }
    //vy表示有效部分为第3,4位
    else if(sentry == 'v'||sentry == 'y')
    {
        couponGroupIdCode = code.substr(2,2);
    }
    //wz表示有效部分为第4位
    else if(sentry == 'w'||sentry == 'z')
    {
        couponGroupIdCode = code.substr(3,1);
    }
    //否则全部有效
    else
        couponGroupIdCode = code.substr(0,4);

    //组ID对应部分的券码长度
    int len1 = couponGroupIdCode.size();
    for(int i=0;i<len1;i++)
    {
        coupon_group_id += (couponGroupIdCode.at(i)-'a')*pow(26,len1-i-1);
    }

    //如果大于30万，无效
    if(coupon_group_id > 300000)
    {
        return Errno::INVALID_COUPON_GROUP_ID;
    }

    //券ID对应部分的哨兵
    string couponIdCode = "";
    sentry = code.at(4);
    //iou表示第6-11位有效
    if(sentry == 'i'||sentry == 'o'||sentry == 'u')
    {
        couponIdCode = code.substr(5,6);
    }
    //jpv表示第7-11位有效
    else if(sentry == 'j'||sentry == 'p'||sentry == 'v')
    {
        couponIdCode = code.substr(6,5);
    }
    //kqw表示第8-11位有效
    else if(sentry == 'k'||sentry == 'q'||sentry == 'w')
    {
        couponIdCode = code.substr(7,4);
    }
    //lrx表示第9-11位有效
    else if(sentry == 'l'||sentry == 'r'||sentry == 'x')
    {
        couponIdCode = code.substr(8,3);
    }
    //msy表示第10-11位有效
    else if(sentry == 'm'||sentry == 's'||sentry == 'y')
    {
        couponIdCode = code.substr(9,2);
    }
    //ntz表示第11位有效
    else if(sentry == 'n'||sentry == 't'||sentry == 'z')
    {
        couponIdCode = code.substr(10,1);
    }
    else
        //否则第5-11位全部有效
        couponIdCode = code.substr(4,7);

    //券ID
    long int temp = 0;
    int len2 = couponIdCode.size();
    for(int i=0;i<len2;i++)
    {
        temp += (couponIdCode.at(i)-'a')*pow(26,len2-i-1);
    }
    coupon_id = int(temp/100);

    //如果大于1千万，无效
    if(coupon_id > 10000000)
    {
        return Errno::INVALID_COUPON_ID;
    }

    return Errno::OK;
}

#endif

