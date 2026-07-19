cmake_minimum_required(VERSION 3.16)
project(pingpp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(pingpp src/main.cpp)

if(WIN32)
    target_link_libraries(pingpp ws2_32)
endif()
