set(TARGET_NAME qmribbon_simple)

# 示例自己的样式表走 qrc（见 simple.qrc / simple.qss），所以这个目标也要开 AUTORCC
#（source/CMakeLists.txt 里那个 set(CMAKE_AUTORCC ON) 是目录作用域的，管不到这里）。
set(CMAKE_AUTORCC ON)

add_executable(
    ${TARGET_NAME}
    main.cpp
    backstage.cpp backstage.h
    exampleicons.cpp exampleicons.h
    floatingtoolbar.cpp floatingtoolbar.h
    gallery.cpp gallery.h
    launcherpopup.cpp launcherpopup.h
    simple.qrc
)
target_link_libraries(${TARGET_NAME} PRIVATE qmribbon)

# ---------------------------------------------------------------------------
# 关于 Windows 11 贴靠布局（Snap Layout）
#
# 本示例目前不启用该特性，因此没有应用清单文件。
# 如果以后需要"鼠标悬停最大化按钮时弹出贴靠布局面板"，需要给 exe 加一份应用清单，
# 在 <compatibility> 里声明 supportedOS（Windows 10/11）—— 这是 Windows 的硬性要求，
# 没有等价的运行时 API。做法：
#
#   1) 新建 app.manifest，关键是这一段（顶层 assemblyIdentity 不能写
#      processorArchitecture="*"，否则 mt.exe 报 c10100b7）：
#        <compatibility xmlns="urn:schemas-microsoft-com:compatibility.v1">
#          <application>
#            <supportedOS Id="{8e0f7a12-bfb3-4fe8-b9a5-48fd50a15a9a}"/>  <!-- Win10/11 -->
#          </application>
#        </compatibility>
#
#   2) CMake 里只加 /MANIFESTINPUT：
#        if(WIN32)
#            target_link_options(${TARGET_NAME} PRIVATE
#                "/MANIFESTINPUT:${CMAKE_CURRENT_SOURCE_DIR}/app.manifest")
#        endif()
#
#      不要同时加 /MANIFEST:EMBED —— CMake 的 vs_link_exe 会负责合并与嵌入，
#      再 EMBED 一次会得到 "CVT1100: 资源重复。类型: MANIFEST"。
# ---------------------------------------------------------------------------
