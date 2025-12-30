cmake_minimum_required(VERSION 3.20)
project(daemonmake LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(nlohmann_json REQUIRED CONFIG)

add_library(daemonmake_lib
    src/config.cpp
    src/project.cpp
    src/daemon.cpp
    src/cmake_builder.cpp
    src/commands.cpp
)

target_include_directories(daemonmake_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(daemonmake_lib PUBLIC cxx_std_20)

target_link_libraries(daemonmake_lib
    PUBLIC nlohmann_json::nlohmann_json
)

add_executable(daemonmake
    apps/daemonmake_main.cpp
)

target_link_libraries(daemonmake
    PRIVATE daemonmake_lib
)
