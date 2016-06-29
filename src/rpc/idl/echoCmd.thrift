include "httpCmd.thrift"

namespace cpp EchoCmd
namespace py EchoCmd

struct EchoReq {                            //回声测试请求
    1: required string command = "Echo"     //命令
    2: required httpCmd.Header header       //公共请求信息头
    3: required string foo
}

struct EchoRsp {                            //回声测试响应
    1: required i32     error               //错误码
    2: required string  errmsg              //错误消息
    3: string  foo = ""
    4: i32     pid                          //进程id
}
