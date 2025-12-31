cmake_minimum_required(VERSION 3.20)
project(sandbox_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler configured by daemonmake
if (NOT CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER "g++")
endif()

# Assume public headers live under include/
set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Targets discovered by daemonmake

add_library(core
    src/core/a.cpp
)

target_include_directories(core PRIVATE ${PROJECT_INCLUDE_DIR})

add_library(util
    src/util/u.cpp
)

target_include_directories(util PRIVATE ${PROJECT_INCLUDE_DIR})

add_library(UnnamedLib
    src/mainlib.cpp
)

target_include_directories(UnnamedLib PRIVATE ${PROJECT_INCLUDE_DIR})

add_executable(app1
    apps/app1.cpp
)

target_include_directories(app1 PRIVATE ${PROJECT_INCLUDE_DIR})

# Inferred dependencies between targets

target_link_libraries(core
    PRIVATE
        util
)

target_link_libraries(UnnamedLib
    PRIVATE
        util
        core
)

target_link_libraries(app1
    PRIVATE
        UnnamedLib
)

