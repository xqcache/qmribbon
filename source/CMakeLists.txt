set(TARGET_NAME ${PROJECT_NAME})

set(PRIVATE_SOURCES
    qmribbon_global.h
    qmribbon.h
    qmribbon.cpp
    qmribbontabbar.h
    qmribbontabbar.cpp
    qmribbontitlebar.h
    qmribbontitlebar.cpp
    qmribbonpagecontainer.h
    qmribbonpagecontainer.cpp
    qmribbonpage.h
    qmribbonpage.cpp
    qmribbonsection.h
    qmribbonsection.cpp
    qmribbonmainwindow.h
    qmribbonmainwindow.cpp
    qmribbonbackstageview.h
    qmribbonbackstageview.cpp
)

find_package(QT NAMES Qt6 CONFIG REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} CONFIG REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(QMRIBBON_BUILD_SHARED OR BUILD_SHARED_LIBS)
    add_library(${TARGET_NAME} SHARED)
    target_compile_definitions(${TARGET_NAME} PRIVATE "QMRIBBON_BUILDING_LIB")
    set_target_properties(${TARGET_NAME} PROPERTIES
        DEBUG_POSTFIX "d"
    )
else()
    add_library(${TARGET_NAME} STATIC)
    target_compile_definitions(${TARGET_NAME} PUBLIC "QMRIBBON_STATIC")
    set_target_properties(${TARGET_NAME} PROPERTIES
        DEBUG_POSTFIX "_staticd"
        RELEASE_POSTFIX "_static"
    )
endif()

target_sources(${TARGET_NAME} PRIVATE ${PRIVATE_SOURCES})
target_link_libraries(${TARGET_NAME} PUBLIC Qt${QT_VERSION_MAJOR}::Widgets qmwidgets PRIVATE qmribbon_assets)
target_include_directories(${TARGET_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>" "$<INSTALL_INTERFACE:include>")

include(GNUInstallDirs)
install(TARGETS ${TARGET_NAME} EXPORT QmWidgetsTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT QmWidgets
)