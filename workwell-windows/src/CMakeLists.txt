cmake_minimum_required(VERSION 3.15)
project(WorkWell VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(SOURCES
    src/main.cpp
    src/TrayIcon.cpp
    src/TimerManager.cpp
    src/BreakWindow.cpp
    src/MainDialog.cpp
)

add_executable(WorkWell WIN32 ${SOURCES})

target_compile_definitions(WorkWell PRIVATE
    _WIN32_WINNT=0x0601
    _WIN32_IE=0x0601
    UNICODE
    _UNICODE
)

target_link_libraries(WorkWell PRIVATE user32 gdi32 shell32 comctl32 msimg32)

if(MSVC)
    target_compile_options(WorkWell PRIVATE
        /W4
        /utf-8
        /O2
    )
    set_property(TARGET WorkWell PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DNDEBUG)
endif()

install(TARGETS WorkWell DESTINATION .)
