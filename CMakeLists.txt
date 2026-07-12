cmake_minimum_required(VERSION 3.16)
project(tiny_ecs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include_directories(include)

add_executable(tiny_ecs
src/main.cpp 
src/world.cpp
)

target_include_directories(tiny_ecs PUBLIC ${PROJECT_SOURCE_DIR}/include)
