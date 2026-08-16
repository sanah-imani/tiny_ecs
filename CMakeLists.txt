cmake_minimum_required(VERSION 3.16)
project(tiny_ecs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

add_library(ecs src/world.cpp src/command_buffer.cpp)
target_include_directories(ecs PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(ecs PUBLIC nlohmann_json::nlohmann_json)
target_compile_options(ecs PRIVATE -Wall -Wextra)

add_executable(tiny_ecs src/main.cpp)
target_link_libraries(tiny_ecs PRIVATE ecs)

include(FetchContent)

# doctest ships a single header; skipping the tarball avoids running its own
# CMakeLists, which declares a minimum version that CMake 4 rejects.
FetchContent_Declare(doctest
    URL https://raw.githubusercontent.com/doctest/doctest/v2.4.11/doctest/doctest.h
    DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_MakeAvailable(doctest)

FetchContent_Declare(nlohmann_json
    URL https://raw.githubusercontent.com/nlohmann/json/v3.11.3/single_include/nlohmann/json.hpp
    DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE ${nlohmann_json_SOURCE_DIR})
add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)

add_library(doctest INTERFACE)
target_include_directories(doctest INTERFACE ${doctest_SOURCE_DIR})
add_library(doctest::doctest ALIAS doctest)

enable_testing()

add_executable(ecs_tests
    tests/main.cpp
    tests/test_entity.cpp
    tests/test_world.cpp
    tests/test_storage.cpp
    tests/test_commands.cpp
    tests/test_events.cpp
    tests/test_collision.cpp
)
target_link_libraries(ecs_tests PRIVATE ecs doctest::doctest)
target_compile_options(ecs_tests PRIVATE -Wall -Wextra)

add_test(NAME ecs_tests COMMAND ecs_tests)
