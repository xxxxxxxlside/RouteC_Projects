#include "async_logger.h"
#include <iostream>

int main() {
    // 1 . 创建日志对象
    AsyncLogger logger;

    // 2. 初始化日志文件
    if (!logger.Init("test.log")) {

        std::cout << "日志初始化失败！" << std::endl;
        return -1;

    }

    std::cout << "日志初始化成功！" << std::endl;

    // 3. 写几条测试日志
    logger.Log("Hello, RouteC!");
    logger.Log("这是第二条日志");
    logger.Log("Project 1 MVP - Day 1");

    // 4. 停止日志器(关闭文件)
    logger.Stop();

    std::cout << "日志写入完成！请查看test.log文件" <<std::endl;

    return 0;

}