cmake_minimum_required(VERSION 3.24.0 FATAL_ERROR)

project(qmribbon_consumer LANGUAGES CXX)

# ---------------------------------------------------------------------------
# 安装包消费验证（**独立工程**，不要加进 tests/CMakeLists.txt）
#
# 它验证的是 `cmake --install` 出来的包能不能被外部工程消费，所以必须脱离主工程构建：
# 只用 find_package(qmribbon) 拿目标与头文件，不共享主工程的任何构建目录信息。
#
# 用法（静态 / 动态各来一次）：
#
#   cmake --install build        --prefix install-static
#   cmake --install build-shared --prefix install-shared
#
#   cmake -S tests/consumer -B build-consumer-static -G Ninja `
#         -DCMAKE_PREFIX_PATH="$PWD/install-static;D:/Install/Qt/6.8.3/msvc2022_64"
#   cmake --build build-consumer-static
#
# 动态库版本的产物要能跑起来，还得让 <prefix>/bin 里的 qmribbon.dll（以及动态构建的
# ADS：qtadvanceddocking-qt6.dll）能被找到 —— 拷到 exe 旁边，或者加进 PATH。
# 编译链接本身就足以验证导出宏（漏了宏在链接期就会失败）。
# ---------------------------------------------------------------------------

find_package(qmribbon 0.0.1 CONFIG REQUIRED)

add_executable(qmribbon_consumer main.cpp)
target_link_libraries(qmribbon_consumer PRIVATE qmribbon::qmribbon)
