#ifndef ASYNC_LOGGER_H
#define ASYNC_LOGGER_H

#include <string>
#include <queue>            //1. 引入队列
#include <thread>           //2. 引入线程
#include <mutex>            //3. 引入锁
#include <condition_variable>//4. 引入条件变量
#include <chrono>           // 1. 新增：时间库

// 2. 新增：定义日志等级枚举
enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

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

    // 3. 修改 Log 函数：增加等级参数，默认是 INFO
    void Log(const std::string& msg,LogLevel level = LogLevel::INFO);


    // 停止：刷新缓冲区并关闭文件
    void Stop();

private:
    
    //【新增】后台线程执行函数
    void WriteLoop();

    // 4. 新增：辅助函数，用于把等级枚举换成字符串(如INFO -> "INFO")
    std::string LogLevelToString(LogLevel level);

    // 5. 新增：辅助函数，用于获取当前时间字符串
    std::string GetCurrentTime();

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