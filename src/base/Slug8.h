/*
 * author:charlee
 */

#ifndef _SLUG8_H_
#define _SLUG8_H_

#include <assert.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include "RandomUtil.h"

using namespace std;

/*
    slug在10亿以内随机并唯一
    26^8 = 208827064576 p26: baaaaaaaa
    100000000000 p26: mlsnwsce
*/

namespace Slug8 {
string Slug(unsigned long int id){
    assert(1 <= id && id <= 1000000000);//id最大为10亿

    string slug="";
	vector<int> remainders;
	remainders.reserve(10);

    unsigned long int n = id * 100 + RandomUtil::Random(0,100);
	while (n!=0){
		remainders.push_back(n%26);
		n=n/26;
	}

	//转换为26进制小写字符串,从高位到地位
	int size = remainders.size();
	for(int j=size-1;j>=0;j--){
		slug += (char)(remainders[j]+97) ;
	}

	//需要补齐的数目
    int to_full_len = 8-slug.length();
    //如果需要补齐
    if(to_full_len > 0){
        //填充的随机字符串
        string rdm_str="";
        //补齐to_full_len-1个随机小写字符
        for(int j=1;j<=to_full_len-1;j++){
            rdm_str += ('a'+RandomUtil::Random(0,26));
        }
        //随机小写字符串填充在slug前面
        slug = rdm_str + slug;

        //最后补齐一个哨兵字符
        string sentry = "tuvwxyz";
        slug = sentry.at(to_full_len-1) + slug;
    }

	return slug;
}

unsigned long int DeSlug(const string slug){
    unsigned long int id = 0;//invalid value

    //如果slug不是8位
    if(slug.length() != 8){
        return 0;
    }

    //校验
    for(int i=0;i<8;i++){
        if(slug.at(i)<'a'||slug.at(i)>'z'){
            return 0;
        }
    }

    //组ID部分的哨兵
    char sentry = slug.at(0);
    string code = "";
    //t表示有效部分为第2,3,4,5,6,7,8位
    if(sentry == 't'){
        code = slug.substr(1,7);
    }
    //u表示有效部分为第3,4,5,6,7,8位
    else if(sentry == 'u'){
        code = slug.substr(2,6);
    }
    //v表示有效部分为第4,5,6,7,8位
    else if(sentry == 'v'){
        code = slug.substr(3,5);
    }
    //w表示有效部分为第5,6,7,8位
    else if(sentry == 'w'){
        code = slug.substr(4,4);
    }
    //x表示有效部分为第6,7,8位
    else if(sentry == 'x'){
        code = slug.substr(5,3);
    }
    //y表示有效部分为第7,8位
    else if(sentry == 'y'){
        code = slug.substr(6,2);
    }
    //z表示有效部分为第8位
    else if(sentry == 'z'){
        code = slug.substr(7,1);
    }
    //否则全部有效
    else
        code = slug.substr(0,8);

    //组ID对应部分的券码长度
    int len = code.size();
    for(int i=0;i<len;i++){
        id += (code.at(i)-'a')*pow(26,len-i-1);
    }

    return id;
}
}

#endif
