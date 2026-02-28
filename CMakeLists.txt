cmake_minimum_required(VERSION 3.16)
project(AsyncLogger VERSION 1.0.0)

# 设置 c++ 标准为 c++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 指定头文件搜索路径
include_directories(include)

# 添加可执行文件
# 列出所有需要编译的源文件
# 1. 原有的主测试程序 (logger_test)#
add_executable(logger_test
    src/main.cpp
    src/async_logger.cpp

)

# 链接 pthread 库 (多线程必须)
target_link_libraries(logger_test pthread)

# 【可选】添加ASan内容 检测支持
# 编译时会自动检测内存泄露，越界等问题
target_compile_options(logger_test PRIVATE -fsanitize=address -g)
target_link_options(logger_test PRIVATE -fsanitize=address)

# 2. 新增的性能测试程序 (benchmark_test)#

add_executable(benchmark_test
    src/benchmark.cpp
    src/async_logger.cpp
)

# 链接 pthread 库#
target_link_libraries(benchmark_test pthread)

# 【注意】基准测试通常不需要 ASan，因为 ASan 会显著拖慢速度，影响测试结果准确性。
# 所以这里我们【不】给 benchmark_test 加 -fsanitize=address。
# 如果你想测带 ASan 的性能，可以把下面两行取消注释。
# target_compile_options(benchmark_test PRIVATE -fsanitize=address -g)
# target_link_options(benchmark_test PRIVATE -fsanitize=address)