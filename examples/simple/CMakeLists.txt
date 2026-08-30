set(TARGET_NAME qmribbon_simple)

add_executable(${TARGET_NAME} main.cpp)
target_link_libraries(${TARGET_NAME} PRIVATE qmribbon)
