cmake_minimum_required(VERSION 3.16)
project(RayCaster CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

find_package(SDL2 REQUIRED)

# Pure logic, no SDL — portable and unit-testable with plain g++.
add_library(core STATIC
  src/core/Map.cpp
)
target_include_directories(core PUBLIC include)

add_executable(raycaster
  src/game/main.cpp
  src/game/Renderer.cpp
)
target_include_directories(raycaster PRIVATE include ${SDL2_INCLUDE_DIRS})
target_link_libraries(raycaster PRIVATE core ${SDL2_LIBRARIES})

enable_testing()

# Links only against `core`, with no SDL2 at all — this is what proves the
# core module is genuinely graphics-independent.
add_executable(test_map tests/test_map.cpp)
target_link_libraries(test_map PRIVATE core)
add_test(NAME test_map COMMAND test_map WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
