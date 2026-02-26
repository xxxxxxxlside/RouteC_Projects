#ifndef ASYNC_LOGGER_H
#define ASYNC_LOGGER_H

#include <string>
#include <queue>            //1. 引入队列
#include <thread>           //2. 引入线程
#include <mutex>            //3. 引入锁
#include <condition_variable>//4. 引入条件变量

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
    void Log(const std::string& msg);// 这个函数现在只负责“丢进队列”

    // 停止：刷新缓冲区并关闭文件
    void Stop();

private:
    
    //【新增】后台线程执行函数
    void WriteLoop();

    // 成员变量：日志文件名
    std::string filename_;

    // 成员变量：文件指针 (先声明，稍后实现)
    FILE* file_;

    //【新增】异步核心组件
    std::queue<std::string> log_queue_;     // 日志缓冲队列
    std::mutex mutex_;                      // 保护队列的锁
    std::condition_variable cv_;            // 用于唤醒后台线程
    std::thread worker_thread_;             // 后台工作线程
    bool is_running_;                       // 标记线程是否还在运行


};

#endif // ASYNC_LOGGER_H