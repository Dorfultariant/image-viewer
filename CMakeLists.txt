cmake_minimum_required(VERSION 3.20)

project(iv VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)

add_executable(iv
    iv.cpp
)

target_link_libraries(iv PRIVATE
    SDL3::SDL3
    SDL3_image::SDL3_image
)

if(MSVC)
    target_compile_options(iv PRIVATE /W4)
else()
    target_compile_options(iv PRIVATE -Wall -Wextra -Wpedantic)
endif()
