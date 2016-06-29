#ifndef _ONSALE_CONST_HPP_
#define _ONSALE_CONST_HPP_

#define ONSALE_LABEL_LIST                               "onsale_label_list"             //创建标签消息通道

const std::string onsale_namelist_prefix[4]         =   {                               //名单使用下标索引,不可随意该这个数组
    "onsale_namelist_whitelist_",                                                       //白名单集合
    "onsale_namelist_blacklist_",                                                       //黑名单集合
    "onsale_namelist_totallist_",                                                       //所有名单集合
    "onsale_namelist_datumlist_",                                                       //黑名单的基准名单集合
};

const std::string onsale_namelist_templist_prefix   =   "onsale_namelist_templist_";    //黑名单的基准名单集合 - 黑名单集合

const std::string onsale_checklist_prefix[3]        =   {                               //校验 名单使用下标索引,不可随意该这个数组
    "onsale_checklist_whitelist_",                                                      //临时白名单集合
    "onsale_checklist_blacklist_",                                                      //临时黑名单集合
    "onsale_checklist_totallist_",                                                      //临时所有名单集合
};

const std::string onsale_checklist_timestamp_prefix =   "onsale_checklist_timestamp_";  //最近一次校验的时间戳

#endif
