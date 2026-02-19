cmake_minimum_required(VERSION 3.16)
project(AsyncLogger VERSION 1.0.0)

# 设置 c++ 标准为 c++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 指定头文件搜索路径
include_directories(include)

# 添加可执行文件
# 列出所有需要编译的源文件

add_executable(logger_test
    src/main.cpp
    src/async_logger.cpp

)

# 【可选】添加ASan内容 检测支持
# 编译时会自动检测内存泄露，越界等问题
target_compile_options(logger_test PRIVATE -fsanitize=address -g)
target_link_options(logger_test PRIVATE -fsanitize=address)