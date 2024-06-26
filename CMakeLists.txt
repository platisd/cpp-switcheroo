cmake_minimum_required(VERSION 3.14)
project(Switcheroo)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(
        -Wcast-align
        -Wpedantic
        -Wcast-qual
        -Wconversion
        -Wctor-dtor-privacy
        -Wnon-virtual-dtor
        -Wmissing-include-dirs
        -Wdisabled-optimization
        -Winit-self
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Woverloaded-virtual
        -Wparentheses
        -Wredundant-decls
        -Wshadow
        -Wsign-promo
        -Wstrict-aliasing
        -Wall
        -Wextra
        -Werror
)

add_subdirectory(external)
enable_testing()

add_executable(SwitcherooMain main.cpp
        main.cpp
        switcheroo/switcheroo.h)

add_subdirectory(test)
