#include "async_logger.h"
#include <cstdio> // 对应 c 的 stdio.h,用于FILE*,fopen,fprintf,fclose
#include <iostream> // 用于调试输出
#include <sstream> // 用于字符串拼接
#include <iomanip> // 用于格式化时间（如补零）
#include <ctime>   // C风格时间转换


// 构造函数：初始化状态
AsyncLogger::AsyncLogger() : file_(nullptr), is_running_(false){

}

// 析构函数:确保资源释放
AsyncLogger::~AsyncLogger() {
    Stop();
}

// 【新增】把等级枚举转换成字符串
std::string AsyncLogger::LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARN: return "WARN";
        case LogLevel::ERROR: return "ERROR";
        default:              return "UNKNOWN";
    }
}

// 【新增】获取当前字符串[YYYY-MM-DD HH:MM:SS.mmm]
std::string AsyncLogger::GetCurrentTime() {
    // 1. 获取系统当前时间点
    auto now = std::chrono::system_clock::now();

    // 2. 转换为time_t (C 风格时间)
    std::time_t t = std::chrono::system_clock::to_time_t(now);

    // 3. 转换为本地时间结构体
    std::tm* tm_ptr = std::localtime(&t);

    // 4. 计算毫秒部分
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(    
    now.time_since_epoch()) % 1000;

    // 5. 格式化成字符串
    std::ostringstream oss;
    oss << "["
        << std::put_time(tm_ptr, "%Y-%m-%d %H:%M:%S")// 年月日 时分秒
        << "." <<std::setfill('0') << std::setw(3) << ms.count() //毫秒
        << "]";

    return oss.str();

}

// 【新增】后台线程的主循环
// 这个函数会一直在后台跑，直到is_running_ 变成 false
void AsyncLogger::WriteLoop() {
    while (true) {
        std::unique_lock<std::mutex> lock(mutex_);

        // 1. 等待：如果队列为空且还在运行，就挂起等待（不占CPU）
        // 当有人调用 cv_.notify_one() 或者 is_running_ 变false时唤醒
        cv_.wait(lock,[this] {
            return !log_queue_.empty() || !is_running_;
        });

        // 2. 退出条件：如果队列空了且要停止了，就退出循环
        if (log_queue_.empty() && !is_running_){
            break;
        }

        // 3. 批量处理：把队列的日志一次性取出来（减少锁竞争）
        std::queue<std::string> temp_queue;
        temp_queue.swap(log_queue_);// 交换队列，瞬间清空原队列

        // 4. 释放锁：写文件不需要锁，让其他线程继续 Log
        lock.unlock();

        // 5. 真正写文件
        while (!temp_queue.empty()) {
            if (file_ != nullptr) {
                // 注意：现在队列里存的已经是带时间、带等级的完整字符串了
                fprintf(file_,"%s\n", temp_queue.front().c_str());
                temp_queue.pop();
            }
        }

        // 【关键修复】每次批量写入后，强制刷新缓冲区到磁盘
        if (file_ != nullptr) {
            fflush(file_);
        }

    }
}


// 初始化：启动后台线程

bool AsyncLogger::Init(const std::string& filename) {
    // 如果已经打开了，先关闭旧的(防止重复打开)
    if(file_) {
        fclose(file_);
        file_ = nullptr;
    }

    // "a" 表示追加模式(append)
    // c_str() 把 std::string 转为 C 风格字符串
    file_= fopen(filename.c_str(),"a");

    // 检查是否打开成功
    if(file_ == nullptr) {
        perror("fopen failed");
        return false;//打开失败
    }

    filename_ = filename;
    is_running_ = true;

    // 【关键】启动后台线程
    worker_thread_ = std::thread(&AsyncLogger::WriteLoop,this);

    return true;//打开成功
}

// 【修改】Log 函数 ：在这里组装最终字符串
void AsyncLogger::Log(const std::string& msg, LogLevel level) {

    // 1. 获取时间戳， 例如 [2026-02-28 20:15:30.123]
    std::string timestamp = GetCurrentTime();

    // 2. 获取等级字符串，例如 [INFO]
    std::string levelStr = LogLevelToString(level);

    // 3. 组装最终消息：[时间][等级] 内容
    std::string final_msg = timestamp + " " + levelStr + " " +msg;

    {
        // 加锁保护队列
        std::lock_guard<std::mutex> lock(mutex_);
        if(is_running_){
            // 【检查点 4】推进队列的必须是 final_msg，而不是 msg！
            log_queue_.push(final_msg);
        }
    }
    // 解锁后通知后台线程：“有新货了，快起来干活！”
    cv_.notify_one();
}

// 停止：通知线程退出并等待其结束
void AsyncLogger::Stop() {
    if (!is_running_) return;

    // 1. 标记停止
    {
        std::lock_guard<std::mutex> lock(mutex_);
        is_running_ = false;
    }

    // 2. 唤醒后台线程（让它检测到 is_running_ 为 false 从而退出）
    cv_.notify_one();

    // 3. 等待后台线程彻底结束（join 会阻塞当前线程，直到worker_thread_跑完）
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }

    // 4. 关闭文件
    if (file_) {
        fclose(file_);
        file_ = nullptr;
    }
}
