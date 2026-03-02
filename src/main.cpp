#include "async_logger.h"
#include <iostream>
#include <thread>
#include <vector>

int main() {
    // 1. 获取单例并初始化 (只需要做一次！)
    // 1 . 创建日志对象
    AsyncLogger::Instance().Init("singleton_test.log",1024); // 1KB 限制
    
    std::cout << "Day 7: 单例模式 + 全局宏 终极测试" << std::endl;



    // 2. 直接使用宏！连 logger 对象都不需要定义了！
    LOG_INFO("系统启动成功...");
    LOG_DEBUG("正在加载配置文件...");
    LOG_WARN("警告：内存使用率较高！");
    LOG_ERROR("错误：无法连接数据库");
    

    // 3. 并发测试

    std::vector<std::thread> threads;
    for(int i = 0; i < 5; ++i) {
        threads.emplace_back([i](){
            for(int j = 0; j < 20; ++j) {
                // 直接在多线程里用，自动指向同一个单例！
                LOG_INFO("线程-" + std::to_string(i) + "处理任务-" +
std::to_string(j));

            }
        });
    } 

    // 等待所有线程结束
    for (auto& t : threads) {
        t.join();
    } 

   LOG_INFO("所有任务完成，系统关闭。");

    // 4.停止
    AsyncLogger::Instance().Stop();

    std::cout << " 测试完成！查看 singleton_test.log 和 .old 文件。 " <<std::endl;
    
    return 0;

}