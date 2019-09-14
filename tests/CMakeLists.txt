include_directories(${CMAKE_SOURCE_DIR}/extern/header_only)
include_directories(${CMAKE_SOURCE_DIR}/include)



set(test_libs malice_event pthread gcov)

add_executable(event_test event_test.cpp)
target_link_libraries(event_test ${test_libs})
add_test(NAME event_test COMMAND event_test)

add_executable(event_loop_test event_loop_test.cpp)
target_link_libraries(event_loop_test ${test_libs})
add_test(NAME event_loop_test COMMAND event_loop_test)

add_executable(channel_test channel_test.cpp)
target_link_libraries(channel_test malice_event ${test_libs})
add_test(NAME channel_test COMMAND channel_test)
