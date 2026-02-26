#include "async_logger.h"
#include <cstdio> // 对应 c 的 stdio.h,用于FILE*,fopen,fprintf,fclose
#include <iostream> // 用于调试输出


// 构造函数：初始化状态
AsyncLogger::AsyncLogger() : file_(nullptr), is_running_(false){

}

// 析构函数:确保资源释放
AsyncLogger::~AsyncLogger() {
    Stop();
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
    }

    // "a" 表示追加模式(append)
    // c_str() 把 std::string 转为 C 风格字符串
    file_= fopen(filename.c_str(),"a");

    // 检查是否打开成功
    if(file_ == nullptr) {
        return false;//打开失败
    }

    filename_ = filename;
    is_running_ = true;

    // 【关键】启动后台线程
    worker_thread_ = std::thread(&AsyncLogger::WriteLoop,this);

    return true;//打开成功
}

// 写日志：只负责丢进队列，立刻返回（非阻塞！）
void AsyncLogger::Log(const std::string& msg) {
    {
        // 加锁保护队列
        std::lock_guard<std::mutex> lock(mutex_);
        if(is_running_){
            log_queue_.push(msg);
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
