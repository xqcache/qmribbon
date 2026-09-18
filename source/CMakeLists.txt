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

add_library(${TARGET_NAME} STATIC)
add_library(${PROJECT_NAME}::${TARGET_NAME} ALIAS ${TARGET_NAME})

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
    PRIVATE
        ${PRIVATE_SOURCES}
)
# cmake-format: on

target_compile_features(${TARGET_NAME} PUBLIC cxx_std_23)
set_target_properties(${TARGET_NAME} PROPERTIES DEBUG_POSTFIX "d")

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
)

install(
    EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION lib/cmake/${PROJECT_NAME}
)
