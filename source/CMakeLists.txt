set(TARGET_NAME ${PROJECT_NAME})

find_package(
    Qt6
    6.8
    CONFIG
    REQUIRED
    COMPONENTS Core Gui Widgets
)

set(CMAKE_AUTOMOC ON)

add_library(${TARGET_NAME} STATIC)
target_link_libraries(${TARGET_NAME} PUBLIC Qt6::Widgets)

set(PUBLIC_HEADERS
    qmribbonmainwindow.h
    qmribbon.h
    qmribbontabbar.h
    qmribbonpage.h
    qmribbongroup.h
)
set(PRIVATE_SOURCES
    qmribbonmainwindow.cpp
    qmribbon.cpp
    qmribbontitlebar.h
    qmribbontitlebar.cpp
    qmribbontabbar.cpp
    qmribbonpage.cpp
    qmribbongroup.cpp
)

set(CONTROLS_SOURCES)

# cmake-format: off
target_sources(
    ${TARGET_NAME}
    PUBLIC
        FILE_SET public_headers
        TYPE HEADERS
        BASE_DIRS
            ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
            ${PUBLIC_HEADERS}
    PRIVATE
        ${PRIVATE_SOURCES}
)
# cmake-format: on

target_compile_features(${TARGET_NAME} PUBLIC cxx_std_23)
set_target_properties(${TARGET_NAME} PROPERTIES DEBUG_POSTFIX "d")

target_include_directories(
    ${TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
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
    NAMESPACE qmribbon::
    DESTINATION lib/cmake/${PROJECT_NAME}
)
