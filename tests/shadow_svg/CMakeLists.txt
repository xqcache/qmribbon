set(TARGET_NAME test_shadow_svg)

add_executable(${TARGET_NAME} main.cpp)
target_link_libraries(${TARGET_NAME} PRIVATE qmribbon)
