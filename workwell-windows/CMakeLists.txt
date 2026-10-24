cmake_minimum_required(VERSION 3.15)
project(WorkWell VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 明确列出所有源文件
set(SOURCES
    src/main.cpp
    src/TrayIcon.cpp
    src/TimerManager.cpp
    src/BreakWindow.cpp
    src/MainDialog.cpp
    src/SettingsDialog.cpp
    src/ConfigManager.cpp
)

# 创建可执行文件
add_executable(WorkWell WIN32 ${SOURCES})

# 添加预处理器定义
target_compile_definitions(WorkWell PRIVATE
    _WIN32_WINNT=0x0601
    _WIN32_IE=0x0601
    UNICODE
    _UNICODE
    NOMINMAX
)

# 链接库
target_link_libraries(WorkWell PRIVATE
    user32
    gdi32
    shell32
    comctl32
    msimg32
    gdiplus
    dwmapi
)

# MSVC 编译选项
if(MSVC)
    target_compile_options(WorkWell PRIVATE
        /W4
        /utf-8
        /O1
    )
    # 静态链接运行时
    set_property(TARGET WorkWell PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()

# Release 版本定义
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DNDEBUG)
endif()

# 安装
install(TARGETS WorkWell DESTINATION .)

include(CTest)
if(BUILD_TESTING)
    add_executable(WorkWellTests tests/TimerTests.cpp src/TimerManager.cpp src/ConfigManager.cpp)
    target_include_directories(WorkWellTests PRIVATE src)
    target_compile_definitions(WorkWellTests PRIVATE UNICODE _UNICODE NOMINMAX)
    target_link_libraries(WorkWellTests PRIVATE user32 shell32)
    if(MSVC)
        target_compile_options(WorkWellTests PRIVATE /W4 /utf-8)
        set_property(TARGET WorkWellTests PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
    endif()
    add_test(NAME TimerAndConfiguration COMMAND WorkWellTests)
endif()
