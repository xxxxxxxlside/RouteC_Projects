#include "async_logger.h"
#include <cstdio> // 对应 c 的 stdio.h,用于FILE*,fopen,fprintf,fclose

// 构造函数：初始化指针为空
AsyncLogger::AsyncLogger() : file_(nullptr){

}

// 析构函数: 确保对象销毁时资源被释放
AsyncLogger::~AsyncLogger() {
    Stop();
}

// 初始化：打开文件

bool AsyncLogger::Init(const std::string& filename) {
    // 如果已经打开了，先关闭旧的(防止重复打开)
    if(file_) {
        fclose(file_);
    }

    // "a" 表示追加模式(append)
    // c_str() 把 std::string 转为 C 风格字符串
    file_= fopen(filename.c_str(),"a");

    // 检查是否打开成功
    if(file_ == nullptr) {
        return false;//打开失败
    }
    return true;//打开成功
}

// 写日志：最简单直接的写入
void AsyncLogger::Log(const std::string& msg) {
    if(file_ != nullptr) {
        // 写入内容并换行
        fprintf(file_,"%s\n",msg.c_str());

        // 【重要】：为了测试方便，这里先强制刷新缓冲区
        // 真正的异步日志会在后线程批量刷新，这里先保证你能立刻看到结果

        fflush(file_);
    }
}

// 停止：关闭文件
void AsyncLogger::Stop() {
    if (file_ != nullptr) {
        fclose(file_);
        file_ = nullptr;
    }
}
