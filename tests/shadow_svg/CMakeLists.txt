set(TARGET_NAME test_shadow_svg)

add_executable(${TARGET_NAME} main.cpp)
target_link_libraries(${TARGET_NAME} PRIVATE qmribbon)

# 动态库构建（Windows）下把运行库拷到测试旁边（见根 CMakeLists 里的说明）。
qmribbon_deploy_runtime(${TARGET_NAME})
