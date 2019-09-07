include_directories(${CMAKE_SOURCE_DIR}/extern/header_only)


add_executable(event_test event_test.cpp)
#target_include_directories(event_test ${CMAKE_SOURCE_DIR}/extern/header_only)
target_link_libraries(event_test malice_event)
add_test(NAME event_test COMMAND event_test)
