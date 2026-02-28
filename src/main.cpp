#include "async_logger.h"
#include <iostream>
#include <thread>
#include <vector>

int main() {
    // 1 . 创建日志对象
    AsyncLogger logger;

    // 2. 初始化日志文件
    if (!logger.Init("rotation_test.log", 1024)) {

        std::cout << "日志初始化失败！" << std::endl;
        return -1;

    }

    std::cout << "Day 5:日志轮转测试(限制 1KB)" << std::endl;

    // 写入一条比较长的日志，确保能触发轮转
    std::string long_msg = "This is a very long log message designed to exceed the 1KB limit quickly. ";

    for (int i = 0; i < 20; ++i) {
        logger.Log(long_msg + "Index: " + std::to_string(i), LogLevel::INFO);

    }

    // 等待一下让后台线程处理
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 模拟不同场景的日志
    //logger.Log("系统启动成功...", LogLevel::INFO);
    //logger.Log("正在加载配置文件...", LogLevel::DEBUG);
    //logger.Log("警告：内存使用率较高！", LogLevel::WARN);
    //logger.Log("错误：无法连续数据库！", LogLevel::ERROR);

    // 再来点并发测试

    /*std::vector<std::thread> threads;
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
    } */

    //logger.Log("所有任务完成，系统关闭。", LogLevel::INFO);

    // 停止时会等待后台把剩余日志写完
    logger.Stop();

    std::cout << " 测试完成！请查看目录： " <<std::endl;
    std::cout << "      - rotation_test.log(新文件) " <<std::endl;
    std::cout << "      - rotation_test.log.old (旧文件，被切分出来的) " <<std::endl;

    return 0;

}