/*
 * author: charlee
 */

#ifndef _ONSALE_UTIL_H_
#define _ONSALE_UTIL_H_

#include <string>

using namespace std;
using namespace onsale;
using namespace onsaleEC;

namespace onsaleUtil {
string getWhereCondition(MultipleCondition cond){
    string  andCondition = "";
    int andCondListSize = cond.andCondList.size();
    if(andCondListSize > 0){
        andCondition = " (";
        for(int i=0;i<andCondListSize;i++){
            andCondition += cond.andCondList[i] + " ";

            if(i != andCondListSize-1){
                andCondition += " and ";
            }
        }

        andCondition += ") ";
    }

    string  orCondition = "";
    int orCondListSize = cond.orCondList.size();
    if(orCondListSize > 0){
        orCondition = " (";
        for(int i=0;i<orCondListSize;i++){
            orCondition += cond.orCondList[i] + " ";

            if(i != orCondListSize-1){
                orCondition += " or ";
            }
        }

        orCondition += ") ";
    }

    if(!andCondition.empty()){
        if(!orCondition.empty()){
            andCondition += " and " + orCondition;
        }
    }else{
        if(!orCondition.empty()){
            andCondition += orCondition;
        }
    }

    if(!andCondition.empty()){
        return andCondition;
    }
    return "";
}

string getOrderByCondition(MultipleCondition cond){
    string  orderCondition = "";
    int orderCondListSize = cond.orderCondList.size();
    if(orderCondListSize > 0){
        for(int i=0;i<orderCondListSize;i++){
            orderCondition += cond.orderCondList[i] + " ";
        }
    }

    if(!orderCondition.empty()){
        return string(" order by ") + orderCondition;
    }
    return "";
}

string getLimitCondition(MultipleCondition cond){
    return cond.limitCond;
}

string getCondition(MultipleCondition cond){
    string whereCondition = getWhereCondition(cond);
    if(!whereCondition.empty()){
        return whereCondition + getOrderByCondition(cond) + getLimitCondition(cond);
    }
    return "";
}

}
#endif
