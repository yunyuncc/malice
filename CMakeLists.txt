cmake_minimum_required(VERSION 3.10)
project(malice)
include(cmake/compile.cmake)
enable_testing()
add_subdirectory(malice/src/event)
add_subdirectory(malice/src/base)
add_subdirectory(malice/src/net)
add_subdirectory(malice/tests)
