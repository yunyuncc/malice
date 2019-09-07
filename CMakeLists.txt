cmake_minimum_required(VERSION 3.10)
project(malice)
include(cmake/compile.cmake)

add_subdirectory(src/event)
add_subdirectory(tests)
