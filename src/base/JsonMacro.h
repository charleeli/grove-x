/*
 * author:charlee
 */
#ifndef _JSON_MACRO_H_
#define _JSON_MACRO_H_

#pragma once  
  
#include "rapidjson/rapidjson.h"  
#include "rapidjson/document.h"
  
// 基础变量的校验  
#define json_check_bool(value, strKey) (value.HasMember(strKey) && value[strKey].IsBool())  
#define json_check_string(value, strKey) (value.HasMember(strKey) && value[strKey].IsString())  
#define json_check_int32(value, strKey) (value.HasMember(strKey) && value[strKey].IsInt())  
#define json_check_uint32(value, strKey) (value.HasMember(strKey) && value[strKey].IsUint())  
#define json_check_int64(value, strKey) (value.HasMember(strKey) && value[strKey].IsInt64())  
#define json_check_uint64(value, strKey) (value.HasMember(strKey) && value[strKey].IsUint64())  
#define json_check_float(value, strKey) (value.HasMember(strKey) && value[strKey].IsFloat())  
#define json_check_double(value, strKey) (value.HasMember(strKey) && value[strKey].IsDouble())  
#define json_check_number(value, strKey) (value.HasMember(strKey) && value[strKey].IsNumber())  
#define json_check_array(value, strKey) (value.HasMember(strKey) && value[strKey].IsArray())  
  
// 得到对应类型的数据，如果数据不存在则得到一个默认值  
#define json_get_bool(value, strKey) (json_check_bool(value, strKey) && value[strKey].GetBool())  
#define json_get_string(value, strKey) (json_check_string(value, strKey) ? value[strKey].GetString() : "")  
#define json_get_int32(value, strKey) (json_check_int32(value, strKey) ? value[strKey].GetInt() : 0)  
#define json_get_uint32(value, strKey) (json_check_uint32(value, strKey) ? value[strKey].GetUint() : 0)  
#define json_get_int64(value, strKey) (json_check_int64(value, strKey) ? ((value)[strKey]).GetInt64() : 0)  
#define json_get_uint64(value, strKey) (json_check_uint64(value, strKey) ? ((value)[strKey]).GetUint64() : 0)  
#define json_get_float(value, strKey) (json_check_float(value, strKey) ? ((value)[strKey]).GetFloat() : 0)  
#define json_get_double(value, strKey) (json_check_double(value, strKey) ? ((value)[strKey]).GetDouble() : 0)  
  
// 得到Value指针  
#define json_check_value_ptr(value, strKey) (((value).HasMember(strKey)) ? &((value)[strKey]) : nullptr)  

#endif
