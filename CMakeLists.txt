cmake_minimum_required(VERSION 3.10)
project(autoclicker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(autoclicker src/main.cpp)

target_link_libraries(autoclicker X11 Xtst Xi)
