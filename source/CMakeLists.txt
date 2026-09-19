set(TARGET_NAME ${PROJECT_NAME})

find_package(
    Qt6
    6.8
    CONFIG
    REQUIRED
    COMPONENTS Core Gui Widgets Svg
)

set(CMAKE_AUTOMOC ON)
# 主题配置与样式表模板以 Qt 资源形式编译进库（source/style/qmribbon.qrc）。
set(CMAKE_AUTORCC ON)

# 无边框窗口依赖 thirdparty/qwindowkit（顶层 CMakeLists.txt 中
# add_subdirectory(thirdparty/qwindowkit) 提供 QWindowKit::Widgets）。
if(NOT TARGET QWindowKit::Widgets)
    message(
        FATAL_ERROR
        "找不到 QWindowKit::Widgets。thirdparty/ 是 git submodule，请先执行：\n"
        "  git submodule update --init --recursive"
    )
endif()

# MainView 的停靠系统依赖 thirdparty/qtadvanceddocking。
if(NOT TARGET ads::qtadvanceddocking)
    message(
        FATAL_ERROR
        "找不到 ads::qtadvanceddocking。thirdparty/ 是 git submodule，请先执行：\n"
        "  git submodule update --init --recursive"
    )
endif()

# 库类型由根目录的 QMRIBBON_BUILD_SHARED 决定：静态库（默认，单文件、无需部署）或动态库。
if(QMRIBBON_BUILD_SHARED)
    set(QMRIBBON_LIBRARY_TYPE SHARED)
else()
    set(QMRIBBON_LIBRARY_TYPE STATIC)
endif()

add_library(${TARGET_NAME} ${QMRIBBON_LIBRARY_TYPE})
add_library(${PROJECT_NAME}::${TARGET_NAME} ALIAS ${TARGET_NAME})

# 导出宏的三态见 source/qmribbonexport.h：
#   动态库 —— QMRIBBON_LIBRARY 只给自己（PRIVATE），使用方拿到的是 Q_DECL_IMPORT；
#   静态库 —— QMRIBBON_STATIC 必须传给使用方（PUBLIC），否则使用方头文件里会展开成
#            dllimport，链接直接失败。
if(QMRIBBON_BUILD_SHARED)
    target_compile_definitions(${TARGET_NAME} PRIVATE QMRIBBON_LIBRARY)
else()
    target_compile_definitions(${TARGET_NAME} PUBLIC QMRIBBON_STATIC)
endif()

# QWindowKit 只出现在 .cpp 中（公共头文件不暴露它），因此 PRIVATE 链接即可。
# ADS 通过 QmRibbonWindow::dockManager() 暴露给使用方，故为 PUBLIC。
target_link_libraries(${TARGET_NAME} PUBLIC Qt6::Widgets Qt6::Svg ads::qtadvanceddocking PRIVATE QWindowKit::Widgets)

set(PUBLIC_HEADERS
    window/qmribbonwindow.h

    ribbon/qmribbon.h
    ribbon/qmribbontitlebar.h
    ribbon/qmribbonquickaccessbar.h
    ribbon/qmribbontabbar.h
    ribbon/qmribbontab.h
    ribbon/qmribbonpage.h
    ribbon/qmribbongroup.h

    controls/qmribbonbutton.h
    controls/qmribboncombobox.h
    controls/qmribbonfloatingwidget.h

    style/qmribbonanimationutil.h
    style/qmribbonstyle.h
    style/qmribbontheme.h
    style/qmribbonthememgr.h
    style/qmribbonthemeswitchmask.h
)

set(PRIVATE_SOURCES
    window/qmribbonwindow.cpp

    ribbon/qmribbon.cpp
    ribbon/qmribbontitlebar.cpp
    ribbon/qmribbonquickaccessbar.cpp
    ribbon/qmribbontabbar.cpp
    ribbon/qmribbontab.cpp
    ribbon/qmribbonpage.cpp
    ribbon/qmribbongroup.cpp

    controls/qmribbonbutton.cpp
    controls/qmribboncombobox.cpp
    controls/qmribbonfloatingwidget.cpp

    style/qmribbonanimationutil.cpp
    style/qmribbonstyle.cpp
    style/qmribbontheme.cpp
    style/qmribbonthememgr.cpp
    style/qmribbonthemeswitchmask.cpp
    style/qmribbon.qrc

    utils/qmribbonshadowgenerator.h
    utils/qmribbonshadowgenerator.cpp
)

# cmake-format: off
target_sources(
    ${TARGET_NAME}
    PUBLIC
        FILE_SET public_headers
        TYPE HEADERS
        BASE_DIRS
            ${CMAKE_CURRENT_SOURCE_DIR}/window
            ${CMAKE_CURRENT_SOURCE_DIR}/ribbon
            ${CMAKE_CURRENT_SOURCE_DIR}/controls
            ${CMAKE_CURRENT_SOURCE_DIR}/style
        FILES
            ${PUBLIC_HEADERS}
        # 导出宏单独一个 file set：CMake 不允许同一个 file set 的两个 BASE_DIRS 互相嵌套，
        # 而这个头在 source/ 根目录，没法跟上面四个子目录放进同一个 set。
        FILE_SET export_header
        TYPE HEADERS
        BASE_DIRS
            ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
            qmribbonexport.h
    PRIVATE
        ${PRIVATE_SOURCES}
)
# cmake-format: on

target_compile_features(${TARGET_NAME} PUBLIC cxx_std_23)
set_target_properties(${TARGET_NAME} PROPERTIES DEBUG_POSTFIX "d")

# 动态库带上版本号（ELF 的 libqmribbon.so.0 / Mach-O 同理）；Windows 下不影响 DLL 命名。
if(QMRIBBON_BUILD_SHARED)
    set_target_properties(
        ${TARGET_NAME}
        PROPERTIES VERSION ${PROJECT_VERSION}
                   SOVERSION ${PROJECT_VERSION_MAJOR}
    )
endif()

target_include_directories(
    ${TARGET_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/window>
           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ribbon>
           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/controls>
           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/style>
           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utils>
           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
           $<INSTALL_INTERFACE:include>
)

install(
    TARGETS ${TARGET_NAME}
    EXPORT ${PROJECT_NAME}Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            FILE_SET public_headers
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILE_SET export_header
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
