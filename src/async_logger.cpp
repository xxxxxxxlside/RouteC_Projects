#include "async_logger.h"
#include <cstdio> // 对应 c 的 stdio.h,用于FILE*,fopen,fprintf,fclose
#include <iostream> // 用于调试输出
#include <sstream> // 用于字符串拼接
#include <iomanip> // 用于格式化时间（如补零）
#include <ctime>   // C风格时间转换
#include <sys/stat.h> // 用于 stat 获取文件大小 (Linux/WSL)


// 构造函数：初始化状态
AsyncLogger::AsyncLogger() : file_(nullptr), is_running_(false),
max_file_size_(0),current_file_size_(0){

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

// 【新增】检查文件大小
void AsyncLogger::CheckFileSize() {

    if (file_ == nullptr) return;

    // 获取当前文件指针位置，即已写入大小
    long current_pos = ftell(file_);
    if (current_pos >= 0) {
        current_file_size_ = static_cast<size_t>(current_pos);
    }

    // 如果超过限制，执行轮转
    if (current_file_size_ >= max_file_size_) {
        RotateFile();
    }
}

// 【新增】执行轮转
void AsyncLogger::RotateFile() {
    if (file_) {
        fclose(file_);
        file_ = nullptr;
    }

    // 构造新文件名：源文件名.1(简单起见，这里只演示切分一次，
    //实际项目会递增 .1 .2 .3)

    // 为了简化，我们直接把旧文件重命名为 xxx.log.old, 然后新建 xxx.log
    std::string old_filename = filename_ + ".old";

    // 删除可能存在的旧备份
    remove(old_filename.c_str());

    // 重命名当前文件为备份
    rename(filename_.c_str(), old_filename.c_str());

    // 重新打开新文件
    file_ = fopen(filename_.c_str(), "a");
    if (file_) {
        current_file_size_ = 0; // 重置计数
        std::cout << "[LOG ROTATION] File rotated." << std::endl;
    }
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
                std::string msg = temp_queue.front();
                // 【关键】在写之前，先检查大小 (需要加锁吗？这里是在单线程后台跑，且ftell是安全的，但为了严谨，可以在写完后检查)
                // 简单做法：写完一条检查一次
                // 注意：现在队列里存的已经是带时间、带等级的完整字符串了
                fprintf(file_,"%s\n", msg.c_str());
                 // 更新大小并检查
                // 注意：频繁调用 ftell 可能影响性能，实际项目可以累加 msg.length()
                current_file_size_ += msg.length() + 1;// +1 for newline
                if (current_file_size_ >= max_file_size_) {
                    // 因为 file_ 可能在 RotateFile 里被关闭重开，所以要小心指针
                    // 这里为了简单，我们在 WriteLoop 里直接调用 RotateFile 是不安全的（涉及 file_ 指针变更）
                    // 更好的做法：在 WriteLoop 外部或者加锁保护 RotateFile
                    // 修正：我们在 WriteLoop 里直接调 RotateFile 会导致 file_ 变化，后续 fprintf 可能崩。
                    // 让我们换个策略：在 fprintf 之后，如果超标，立刻关闭文件，并在下一次循环开始时重新打开？
                    // 不，最简单的：在 WriteLoop 里发现超标，调用 RotateFile，然后继续写剩下的消息（此时 file_ 已经是新的了）
                    
                    // 为了防止多线程问题（虽然 WriteLoop 是单线程），我们直接在这里处理
                    fclose(file_);
                    std::string old_filename = filename_ + ".old";
                    remove(old_filename.c_str());
                    rename(filename_.c_str(), old_filename.c_str());
                    file_ = fopen(filename_.c_str(), "a");
                    current_file_size_ = 0;

                    if (!file_) {
                        std::cerr << "[ERROR] Failed to create new logfile after rotation!" << std::endl;
                        break;
                    }

                }

                fflush(file_);
                temp_queue.pop();
            }
        }

        // 最终再 flush 一次确保落盘
        if (file_) fflush(file_);

    }
}


// 初始化：启动后台线程

bool AsyncLogger::Init(const std::string& filename, size_t max_file_size) {
    // 如果已经打开了，先关闭旧的(防止重复打开)
    if(file_) {
        fclose(file_);
        file_ = nullptr;
    }

    filename_ = filename;
    max_file_size_ = max_file_size;
    current_file_size_ = 0;


    // "a" 表示追加模式(append)
    // c_str() 把 std::string 转为 C 风格字符串
    file_= fopen(filename.c_str(),"a");

    // 检查是否打开成功
    if(file_ == nullptr) {
        perror("fopen failed");
        return false;//打开失败
    }

    // 获取初始文件大小
    fseek(file_, 0, SEEK_END);
    current_file_size_ = ftell(file_);
    fseek(file_, 0, SEEK_SET);

    
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
