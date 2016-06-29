namespace cpp HttpCmd
namespace py HttpCmd

enum STATUS {
    SUCCESS             = 0                 // 成功
    FAILED              = -1                // 失败
    EXCEPTION           = -2                // 异常
    NO_COMMAND          = -3                // 没有处理命令
    PARSE_FAIL          = -4                // 参数解析错误
    CHECK_FAIL          = -5                // 参数检查错误
}

struct Header {                             // 公共请求信息头

}

struct HttpRequest {
    1: required string command              // 请求命令
    2: required Header header               // 公共请求信息头
}

struct HttpResponse {
    1: required i32    error                // 错误码
    2: required string errmsg               // 错误信息
}
