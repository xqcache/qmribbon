set(TARGET_NAME qmribbon_demo)

set(CMAKE_AUTOMOC ON)

add_executable(${TARGET_NAME} main.cpp)
target_link_libraries(${TARGET_NAME} PRIVATE qmribbon qmwidgets)
target_compile_definitions(${TARGET_NAME} PRIVATE "TAKEOVER_WIN32_HTCAPTION_EVENT;CURRENT_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}\"")