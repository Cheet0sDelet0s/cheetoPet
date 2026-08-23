cmake_minimum_required(VERSION 3.16)

project(display_driver
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

add_library(display
    Display.cpp
    DisplaySDL2.cpp
)

target_include_directories(display
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(display
    PUBLIC
        SDL2::SDL2
)

add_executable(display_test
    main.cpp
)

target_link_libraries(display_test
    PRIVATE
        display
)