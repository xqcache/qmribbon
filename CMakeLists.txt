cmake_minimum_required(VERSION 3.24.0 FATAL_ERROR)

project(
    qmribbon
    VERSION 0.0.1
    LANGUAGES CXX
)

set(MAIN_PROJECT OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MAIN_PROJECT ON)
endif()

option(QMRIBBON_BUILD_EXAMPLES "" ${MAIN_PROJECT})
option(QMRIBBON_BUILD_TESTS "" ${MAIN_PROJECT})

# ---------------------------------------------------------------------------
# thirdparty submodule 预检
#
# 依赖链是三层的：qwindowkit → qmsetup → stdcorelib。缺任何一层都会在
# qmsetup 的嵌套 configure 里报出难以定位的错误，所以这里先给出明确提示。
# ---------------------------------------------------------------------------
foreach(_required_file
        thirdparty/qwindowkit/CMakeLists.txt
        thirdparty/qwindowkit/qmsetup/CMakeLists.txt
        thirdparty/qwindowkit/qmsetup/src/stdcorelib/CMakeLists.txt
        thirdparty/qtadvanceddocking/CMakeLists.txt)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_required_file}")
        message(
            FATAL_ERROR
            "缺少 ${_required_file}\n"
            "thirdparty/ 下是三层 git submodule，请先执行：\n"
            "  git submodule update --init --recursive"
        )
    endif()
endforeach()

# ---------------------------------------------------------------------------
# thirdparty: QWindowKit
#
# 无边框窗口的原生行为（拖动 / Resize / Aero Snap / Windows 11 Snap Layout 等）
# 由 QWindowKit 实现，源码以 git submodule 方式放在 thirdparty/ 下
# （首次克隆请使用 git clone --recursive，或 git submodule update --init --recursive）。
#
# 配置说明：
#   BUILD_STATIC   静态链接，避免额外部署 QWindowKit 的 DLL
#   QUICK/EXAMPLES/TESTS/DOCUMENTATIONS  本项目用不到，全部关闭
#   ENABLE_WINDOWS_SYSTEM_BORDERS  保持上游默认（ON）：窗口使用系统厚边框，
#       Win11 的圆角与阴影由 DWM 提供，框架不再自绘阴影、也不使用半透明背景。
#       关闭它会带来三个问题：窗口没有圆角（layered 窗口不参与 DWM 圆角）、
#       阴影留白处鼠标穿透导致边缘无法 Resize、以及最大化时客户区四边被内缩 8px
#       造成"最大化后仍能拖动边缘"。
#
# 这些变量在 add_subdirectory 之前设置，CMake 的 option() 会沿用（CMP0077）。
# ---------------------------------------------------------------------------
set(QWINDOWKIT_BUILD_STATIC ON)
set(QWINDOWKIT_BUILD_WIDGETS ON)
set(QWINDOWKIT_BUILD_QUICK OFF)
set(QWINDOWKIT_BUILD_EXAMPLES OFF)
set(QWINDOWKIT_BUILD_TESTS OFF)
set(QWINDOWKIT_BUILD_DOCUMENTATIONS OFF)

# QWindowKit 只要求 C++17，单独用它自己的标准编译，避免让第三方代码跟着升到 C++23。
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_subdirectory(thirdparty/qwindowkit)

# ---------------------------------------------------------------------------
# thirdparty: Qt Advanced Docking System
#
# MainView 的停靠系统（ads::CDockManager）。
#
# 配置说明：
#   BUILD_STATIC    静态链接（ADS 默认构建 DLL）
#   BUILD_EXAMPLES  必须关闭，否则会连带构建 examples 与 demo
#
# 注意：ADS 的这两个选项名没有加项目前缀，因此只在下面这段生效，用完立即 unset，
# 避免影响后面的 examples / tests。已用 -D 指定的情况优先采用命令行值。
#
# 许可证：LGPL-2.1（与 Apache-2.0 的 QWindowKit 不同），详见 thirdparty/README.md。
# ---------------------------------------------------------------------------
if(NOT DEFINED BUILD_STATIC)
    set(BUILD_STATIC ON)
endif()
if(NOT DEFINED BUILD_EXAMPLES)
    set(BUILD_EXAMPLES OFF)
endif()
add_subdirectory(thirdparty/qtadvanceddocking)
unset(BUILD_STATIC)
unset(BUILD_EXAMPLES)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(source)
if(QMRIBBON_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
if (QMRIBBON_BUILD_TESTS)
    add_subdirectory(tests)
endif()


include(CMakePackageConfigHelpers)
configure_package_config_file(
    cmake/config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
