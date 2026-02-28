#include "async_logger.h"
#include <iostream>
#include <thread>
#include <vector>

int main() {
    // 1 . 创建日志对象
    AsyncLogger logger;

    // 2. 初始化日志文件
    if (!logger.Init("day3_test.log")) {

        std::cout << "日志初始化失败！" << std::endl;
        return -1;

    }

    std::cout << "Day 3:带时间戳的异步日志启动!" << std::endl;

    

    // 模拟不同场景的日志
    logger.Log("系统启动成功...", LogLevel::INFO);
    logger.Log("正在加载配置文件...", LogLevel::DEBUG);
    logger.Log("警告：内存使用率较高！", LogLevel::WARN);
    logger.Log("错误：无法连续数据库！", LogLevel::ERROR);

    // 再来点并发测试

    std::vector<std::thread> threads;
    for(int i = 0; i < 5; ++i) {
        threads.emplace_back([&logger,i](){
            for(int j = 0; j < 20; ++j) {
                logger.Log("线程-" + std::to_string(i) + "处理任务-" +
std::to_string(j), LogLevel::INFO);

            }
        });
    }

    // 等待所有线程结束
    for (auto& t : threads) {
        t.join();
    }

    logger.Log("所有任务完成，系统关闭。", LogLevel::INFO);

    // 停止时会等待后台把剩余日志写完
    logger.Stop();

    std::cout << "完成！请查看 day3_test.log 文件 " <<std::endl;

    return 0;

}