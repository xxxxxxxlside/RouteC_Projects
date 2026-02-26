#include "async_logger.h"
#include <iostream>
#include <thread>
#include <vector>

int main() {
    // 1 . 创建日志对象
    AsyncLogger logger;

    // 2. 初始化日志文件
    if (!logger.Init("async_test.log")) {

        std::cout << "日志初始化失败！" << std::endl;
        return -1;

    }

    std::cout << "日志初始化成功！后台线程已启动。" << std::endl;

    std::cout << "开始模拟高并发写入..." << std::endl;

    // 模拟 10 个线程同时写日志

    std::vector<std::thread> threads;
    for(int i = 0; i < 10; ++i) {
        threads.emplace_back([&logger,i](){
            for(int j = 0; j < 100; ++j) {
                std::string msg = "Thread-" + std::to_string(i) + "- Log-"
+std::to_string(j);
                logger.Log(msg);

            }
        });
    }

    // 等待所有线程结束
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "所有线程写入完成，正在停止日志器..." << std::endl;

    // 停止时会等待后台把剩余日志写完
    logger.Stop();

    std::cout << "完成！请查看 async_test.log 文件 ，应该有1000行日志" <<std::endl;

    return 0;

}