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

add_executable(raycaster
  src/game/main.cpp
)
target_include_directories(raycaster PRIVATE include ${SDL2_INCLUDE_DIRS})
target_link_libraries(raycaster PRIVATE ${SDL2_LIBRARIES})
