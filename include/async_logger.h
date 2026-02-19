#ifndef ASYNC_LOGGER_H
#define ASYNC_LOGGER_H

#include <string>

// 异步日志器类
class AsyncLogger {

public:
    // 构造函数
    AsyncLogger();

    // 析构函数
    ~AsyncLogger();

    // 初始化：设置日志文件名
    // 例如：Init("app.log")
    bool Init(const std::string& filename);

    // 写日志：传入一条字符串消息
    // 例如：Log("Hello, RouteC!")
    void Log(const std::string& msg);

    // 停止：刷新缓冲区并关闭文件
    void Stop();

private:
    
    // 成员变量：日志文件名
    std::string filename_;

    // 成员变量：文件指针 (先声明，稍后实现)
    FILE* file_;


};

#endif // ASYNC_LOGGER_H