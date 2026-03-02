#include "async_logger.h"
#include <iostream>
#include <thread>
#include <vector>

int main() {
    // 1 . 创建日志对象
    AsyncLogger logger;

    // 2. 初始化日志文件 (限制 1KB 以测试轮转)
    if (!logger.Init("rotation_test.log", 1024)) {

        std::cout << "日志初始化失败！" << std::endl;
        return -1;

    }

    std::cout << "Day 6: 宏封装 + 日志轮转 综合测试" << std::endl;
    std::cout << "  (文件大小限制 1KB ,观察是否自动切分)" << std::endl;


    // 3. 体验宏封装，简洁的调用方式
    LOG_INFO(&logger, "系统启动成功...");
    LOG_DEBUG(&logger, "正在加载配置文件...");
    LOG_WARN(&logger, "警告：内存使用率较高！");
    LOG_ERROR(&logger, "错误：无法连接数据库");
    

    // 4. 并发测试

    std::vector<std::thread> threads;
    for(int i = 0; i < 5; ++i) {
        threads.emplace_back([&logger,i](){
            for(int j = 0; j < 20; ++j) {
                LOG_INFO(&logger,"线程-" + std::to_string(i) + "处理任务-" +
std::to_string(j));

            }
        });
    } 

    // 等待所有线程结束
    for (auto& t : threads) {
        t.join();
    } 

   

    // 5.停止时会等待后台把剩余日志写完
    logger.Stop();

    std::cout << " 测试完成！请查看目录： " <<std::endl;
    std::cout << "      - rotation_test.log(新文件) " <<std::endl;
    std::cout << "      - rotation_test.log.old (旧文件，被切分出来的) " <<std::endl;
    std::cout << "   - 打开文件看看，时间戳、等级、宏调用都生效了吗？" << std::endl;

    return 0;

}