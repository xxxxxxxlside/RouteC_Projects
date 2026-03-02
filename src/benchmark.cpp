#include "async_logger.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <thread>
#include <unistd.h> // 记得加这个

// --- 模拟一个最简单的同步日志类（用于对比）---
// 这个类不用改，因为它不是单例，是局部变量
class SyncLogger {

public:
    SyncLogger(const std::string& filename) {
        file_ = fopen(filename.c_str(), "w");
    }

    ~SyncLogger() {
        if (file_) fclose(file_);
    }

    void Log(const std::string& msg) {
        // 同步 ：直接写，每次写完刷新(模拟最慢的情况)
        if (file_) {
            fprintf(file_, "%s\n",msg.c_str());
            fflush(file_);
            fsync(fileno(file_)); // 【关键】强制真正落盘，让同步变回“真慢”
        }
    }
private:
    FILE* file_;

};

// --- 测试函数 ---
void run_test(const std::string&name, int count, bool is_async) {
    std::cout << "开始测试" << name << "(写入" << count << " 条) ..." << std::endl;

    // 1. 记录开始时间
    auto start = std::chrono::high_resolution_clock::now();

    if(is_async) {
        // === 异步测试 (改用单例模式)===

        // 1. 初始化单例 (如果之前没初始化过)
        // 注意：基准测试每次都要更新文件，所以这里强制 Init
        AsyncLogger::Instance().Init("async_bench.log", 100 * 1024 * 1024);
        //设置大一点，避免轮转干扰

        for (int i = 0; i < count; ++i) {
            // 2. 直接调用单例 Log
            AsyncLogger::Instance().Log("This is log message number: " + std::to_string(i));
        }

        // 3. 停止单例，确保数据写完
        AsyncLogger::Instance().Stop(); // 确保所有日志写完

    } else {
        // === 同步测试 ===
        SyncLogger logger("sync_bench.log");
        for (int i = 0; i < count; ++i) {
            logger.Log("This is log message number: " + std::to_string(i));

        }
    }

    // 2. 记录结束时间
    auto end = std::chrono::high_resolution_clock::now();

    // 3. 计算耗时(毫秒)
    std::chrono::duration<double, std::milli> duration = end -start;

    std::cout << " " << name << " 完成！耗时：" << duration.count() << "ms" << std::endl;
    std::cout << "-------------------------------------" << std::endl;

}

int main() {
    int count = 5000; // 先测 5000 条 

    std::cout << " Day 7 : 单例模式下的性能基准测试" << std::endl;
    std::cout << " 准备写入 " << count << " 条日志..." << std::endl;
    std::cout << std::endl;

    // 1. 先跑同步 (通常比较慢)
    run_test("同步日志 (Sync)", count, false);

    // 2. 再跑异步 (应该飞快)
    run_test(" 异步日志 (Async)", count, true);

    std::cout << " 测试全部结束！请查看 sync_bench.log 和 async_bench.log" << std::endl;

    return 0;
}