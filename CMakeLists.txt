cmake_minimum_required(VERSION 3.16)
project(autoclicker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(autoclicker src/main.cpp
        src/AutoClicker.cpp
        src/AutoClicker.h)

target_link_libraries(autoclicker PRIVATE
        Qt6::Widgets
        X11
        Xtst
        Xi
)
