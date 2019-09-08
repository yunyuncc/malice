include_directories(${CMAKE_SOURCE_DIR}/extern/header_only)


add_executable(event_test event_test.cpp)
target_link_libraries(event_test malice_event)

add_executable(event_loop_test event_loop_test.cpp)
target_link_libraries(event_loop_test malice_event pthread)

