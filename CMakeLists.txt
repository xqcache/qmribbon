cmake_minimum_required(VERSION 3.24.0 FATAL_ERROR)

project(
    qmribbon
    VERSION 0.0.1
    LANGUAGES CXX
)

# 安装目录变量（CMAKE_INSTALL_LIBDIR / BINDIR / INCLUDEDIR）。
# 这里必须显式引入：以前是 thirdparty 子项目 include(GNUInstallDirs) 留下的缓存副作用，
# 万一子项目顺序变了（或哪天真不需要第三方了），install 路径就会变成空的。
include(GNUInstallDirs)

set(MAIN_PROJECT OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MAIN_PROJECT ON)
endif()

option(QMRIBBON_BUILD_EXAMPLES "" ${MAIN_PROJECT})
option(QMRIBBON_BUILD_TESTS "" ${MAIN_PROJECT})

# 库类型：
#   OFF（默认）= 静态库，只有 .lib，业务不需要部署额外的框架 DLL；
#   ON          = 动态库，产出 qmribbon.dll + 导入库 .lib。
# 默认值跟随 CMake 约定变量 BUILD_SHARED_LIBS，因此下面两种写法等价：
#   -DQMRIBBON_BUILD_SHARED=ON
#   -DBUILD_SHARED_LIBS=ON
# 注意方向是单向的：本选项只是**读取** BUILD_SHARED_LIBS，不会反向去设置它 ——
# 否则会一路影响 thirdparty 的库类型（见下面对 ADS 的显式设置）。
option(QMRIBBON_BUILD_SHARED "构建 qmribbon 动态库（DLL / SO）而不是静态库" ${BUILD_SHARED_LIBS})

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
#   BUILD_STATIC   静态链接，避免额外部署 QWindowKit 的 DLL。
#      QMRIBBON_BUILD_SHARED=ON 时这里**仍然**保持静态：QWindowKit 只出现在 .cpp 里
#      （公共头文件不暴露它），它会被整体链进 qmribbon.dll，使用方完全不需要它，
#      也就不会多出一个要部署的 QWindowKit DLL。
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
#   BUILD_STATIC    静态链接（ADS 默认构建 DLL）。默认跟随 QMRIBBON_BUILD_SHARED：
#                   动态库构建时 ADS 也构建成 DLL，原因有两条：
#                     1. ADS 是**公共**依赖（公共头里出现 ads::CDockManager*），静态 ADS 链进
#                        qmribbon.dll 后，使用方自己还会再链一份静态 ADS —— 同一进程里两份
#                        ADS 代码，各自的静态状态（ConfigFlags 等）互相看不见；
#                     2. ADS 自己不设 POSITION_INDEPENDENT_CODE，非 PIC 的静态库在 Linux 上
#                        根本没法链进 .so。
#                   想强行混搭就用 -DBUILD_STATIC=ON 显式指定（会给出警告）。
#   BUILD_EXAMPLES  必须关闭，否则会连带构建 examples 与 demo
#
# 注意：ADS 的这两个选项名没有加项目前缀，因此只在下面这段生效，用完立即 unset，
# 避免影响后面的 examples / tests。已用 -D 指定的情况优先采用命令行值。
#
# 许可证：LGPL-2.1（与 Apache-2.0 的 QWindowKit 不同），详见 thirdparty/README.md。
# ---------------------------------------------------------------------------
if(NOT DEFINED BUILD_STATIC)
    if(QMRIBBON_BUILD_SHARED)
        set(BUILD_STATIC OFF)
    else()
        set(BUILD_STATIC ON)
    endif()
elseif(QMRIBBON_BUILD_SHARED AND BUILD_STATIC)
    message(
        WARNING
        "QMRIBBON_BUILD_SHARED=ON 但 BUILD_STATIC=ON：qmribbon 是 DLL，而静态 ADS 会被链进这个\n"
        "DLL、同时又在公共头文件里暴露给使用方 —— 同一进程里会出现两份 ADS，\n"
        "它们各自的静态状态互相看不见。除非你确定要这样，否则请改用 -DBUILD_STATIC=OFF。"
    )
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

# ---------------------------------------------------------------------------
# 运行库部署（只对动态库构建、只在 Windows 上需要）
#
# Windows 不会去构建目录里找 DLL：不拷贝的话，示例 / 测试必须先把 <build>/source 加进
# PATH 才能跑。拷到可执行文件旁边，README 里那两条运行命令在两种构建类型下都成立。
# （Linux / macOS 靠 CMake 自动写进可执行文件的 build rpath，不需要这一步。）
#
# 用 macro 而不是 function：宏在调用者的目录作用域里展开，add_custom_command(TARGET ...)
# 与目标在同一个目录，不会有作用域上的歧义。
# ---------------------------------------------------------------------------
macro(qmribbon_deploy_runtime target)
    if(WIN32 AND QMRIBBON_BUILD_SHARED)
        add_custom_command(
            TARGET ${target}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:${PROJECT_NAME}> $<TARGET_FILE_DIR:${target}>
            COMMENT "复制 qmribbon 运行库到 ${target} 输出目录"
        )

        # ADS 是公共依赖，动态库构建下它也是 DLL（见上面的说明），同样要一起拷。
        # 用户强行混搭（-DBUILD_STATIC=ON，ADS 静态）时就不用拷了。
        #
        # 目标名：ADS 的真实目标带 Qt 主版本号（ads::qtadvanceddocking 是指向它的版本无关
        # 别名）。生成表达式里用真实目标名最稳妥，所以优先取真实名；万一上游改了命名再退回
        # 别名。名字在 cmake/config.cmake.in 的 find_dependency 里也出现过，是已知约定。
        set(_qmribbon_ads_target qtadvanceddocking-qt6)
        if(NOT TARGET ${_qmribbon_ads_target})
            set(_qmribbon_ads_target ads::qtadvanceddocking)
        endif()
        get_target_property(_qmribbon_ads_type ${_qmribbon_ads_target} TYPE)
        if(_qmribbon_ads_type STREQUAL "SHARED_LIBRARY")
            add_custom_command(
                TARGET ${target}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        $<TARGET_FILE:${_qmribbon_ads_target}> $<TARGET_FILE_DIR:${target}>
                COMMENT "复制 ADS 运行库到 ${target} 输出目录"
            )
        endif()
        unset(_qmribbon_ads_type)
        unset(_qmribbon_ads_target)
    endif()
endmacro()

if(QMRIBBON_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
if (QMRIBBON_BUILD_TESTS)
    add_subdirectory(tests)
endif()


include(CMakePackageConfigHelpers)

# 静态库把 QWindowKit 以 $<LINK_ONLY:QWindowKit::Widgets> 写进导出目标，使用方必须能找到
# 这个包；动态库下 QWindowKit 是 PRIVATE 且已经链进了 DLL，使用方不需要（也不该强制要求）。
set(QMRIBBON_NEEDS_QWINDOWKIT ON)
if(QMRIBBON_BUILD_SHARED)
    set(QMRIBBON_NEEDS_QWINDOWKIT OFF)
endif()

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
