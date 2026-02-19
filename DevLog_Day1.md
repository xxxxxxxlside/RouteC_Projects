# 📅 开发日志 - Day 1

**日期**: 2026-02-19
**项目**: RouteC_Projects / Project_1_AsyncLogger
**状态**: ✅ 骨架搭建完成 (同步版本)

## 🎯 今日完成
- [x] 环境检查 (g++ 13.3, CMake 3.28, Git 2.43)
- [x] 搭建标准目录结构 (include/src/tests/benchmarks)
- [x] 手写 `AsyncLogger` 类 (同步写入版本)
- [x] 编写 `CMakeLists.txt` 并成功编译
- [x] 运行测试，确认 `test.log` 生成正常

## 🐛 踩坑记录
- **问题**: 写完 `CMakeLists.txt` 后直接运行 `cmake`，报错说找不到 `project()`。
- **原因**: 在 VSCode 中**忘了保存文件 (Ctrl+S)**。
- **解决**: 保存文件后，删除 `build` 目录重新 cmake，成功通过。
- **教训**: **运行前必按 Ctrl+S！** 肌肉记忆要养成。

## 💡 核心收获
1. 理解了 CMake 的 `out-of-source` 构建模式 (build 目录)。
2. 掌握了 C++ 头文件保护 (`#ifndef`)。
3. 学会了 `std::string` 转 `const char*` (`.c_str()`)。
4. 体验了完整的 "编码 -> 编译 -> 运行 -> 调试" 闭环。

## 🚀 明日计划 (Day 2)
- [ ] 引入 `std::thread` 创建后台线程
- [ ] 引入 `std::queue` 做日志缓冲
- [ ] 引入 `std::mutex` 保证线程安全
- [ ] 实现真正的异步写入 (主线程不阻塞)