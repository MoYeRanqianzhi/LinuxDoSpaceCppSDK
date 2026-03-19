cmake_minimum_required(VERSION 3.16)
project(linuxdospace_cpp_sdk CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(linuxdospace_cpp STATIC
  src/LinuxDoSpace.cpp
)

target_include_directories(linuxdospace_cpp PUBLIC include)

if(MSVC)
  target_compile_options(linuxdospace_cpp PRIVATE /W4)
else()
  target_compile_options(linuxdospace_cpp PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(linuxdospace_cpp_example
  examples/basic.cpp
)
target_link_libraries(linuxdospace_cpp_example PRIVATE linuxdospace_cpp)
