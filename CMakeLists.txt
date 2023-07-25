cmake_minimum_required(VERSION 3.0)
project(json)

add_library(json json.c json.h)

add_executable(driver main.c)
target_link_libraries(driver json)

add_executable(tests tests.c)
target_link_libraries(tests json)
