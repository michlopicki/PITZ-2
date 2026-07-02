cmake_minimum_required(VERSION 3.15)
project(PITZ-2)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

include_directories("/opt/homebrew/include")
include_directories(include)
link_directories("/opt/homebrew/lib")

file(GLOB SOURCES "src/*cpp")
add_executable(pitz ${SOURCES})
