cmake_minimum_required(VERSION 3.0)
project(os_semestralka)

# Uncomment after finish project !!!
#set(CMAKE_BUILD_TYPE Release)

set(CMAKE_BUILD_TYPE Debug)

add_compile_options(-std=c++11 -g)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_executable(main ${SOURCES})

TARGET_LINK_LIBRARIES(main pthread)

