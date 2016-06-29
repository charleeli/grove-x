/*
 * author:charlee
 */
#ifndef _GLOGHELPER_H_
#define _GLOGHELPER_H_

#include <glog/logging.h>
#include <glog/raw_logging.h>
#include <stdlib.h>

using namespace std;

#define LOG_INFO LOG(INFO)
#define LOG_WARN LOG(WARNING)
#define LOG_ERROR LOG(ERROR)
#define LOG_FATAL LOG(FATAL)

//将信息输出到单独的文件和 LOG(ERROR)
void SignalHandle(const char* data, int size){
    string str = string(data,size);
    LOG_ERROR<<str;
}

class GLogHelper {
public:
    //GLOG配置：程序名，日志路径
    GLogHelper(string program, string log_path);
    //GLOG内存清理：
    ~GLogHelper();
};

//GLOG配置：
GLogHelper::GLogHelper(string program,string log_path){
    //日志路径
    string LOGDIR = log_path + program;
    if (log_path == "" || log_path == "./") {
        LOGDIR = log_path + program + "_log";
    }

    string MKDIR = string("mkdir -p ") + LOGDIR;
    system(string(MKDIR+"/info/").c_str());
    system(string(MKDIR+"/warn/").c_str());
    system(string(MKDIR+"/error/").c_str());
    system(string(MKDIR+"/fatal/").c_str());
    
    //初始化glog日志库
    google::InitGoogleLogging(program.c_str());
   
    //设置级别高于 google::FATAL 的日志同时输出到屏幕
    google::SetStderrLogging(google::FATAL); 
    //设置输出到屏幕的日志显示相应颜色
    FLAGS_colorlogtostderr=true;
    
    //设置 google::INFO 级别的日志存储路径和文件名前缀
    string dest_info = LOGDIR+"/info/info-";
    google::SetLogDestination(google::INFO,dest_info.c_str());
    //设置 google::WARNING 级别的日志存储路径和文件名前缀
    string dest_warn = LOGDIR+"/warn/warn-";
    google::SetLogDestination(google::WARNING,dest_warn.c_str());
    //设置 google::ERROR 级别的日志存储路径和文件名前缀 
    string dest_error = LOGDIR+"/error/error-";
    google::SetLogDestination(google::ERROR,dest_error.c_str());
    //设置 google::FATAL 级别的日志存储路径和文件名前缀 
    string dest_fatal = LOGDIR+"/fatal/fatal-";
    google::SetLogDestination(google::FATAL,dest_fatal.c_str());
    
    //缓冲日志输出，默认为30秒，此处改为立即输出 
    FLAGS_logbufsecs =0; 
    //最大日志大小为 100MB       
    FLAGS_max_log_size =100;
    //当磁盘被写满时，停止日志输出
    FLAGS_stop_logging_if_full_disk = true; 
    //捕捉 core dumped    
    google::InstallFailureSignalHandler();
    //默认捕捉 SIGSEGV 信号信息输出会输出到 stderr      
    google::InstallFailureWriter(&SignalHandle);    
}

//GLOG内存清理：
GLogHelper::~GLogHelper(){
    google::ShutdownGoogleLogging();
}

#endif

/*
int main(int argc,char* argv[]){
    //要使用 GLOG ，只需要在 main 函数开始处添加这句即可
    //g++ GLogHelper.cpp -lglog
    //请先修改glog源码，参考glog_patch说明
    GLogHelper gh(argv[0],"./log/");

    LOG(INFO)<<"[INFO ]";
    LOG(WARNING)<<"[WARN ]";
    LOG(ERROR)<<"[ERROR]";
    return 0;
}
*/

