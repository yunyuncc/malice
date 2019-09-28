#include_directories(${CMAKE_SOURCE_DIR}/extern/header_only)
include_directories(${CMAKE_SOURCE_DIR}/malice/include)

set(test_libs malice_event malice_base pthread gcov)

function(add_my_test test_name)
    add_executable(${test_name} ${test_name}.cpp)
    target_link_libraries(${test_name} ${test_libs})
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()


add_my_test(event_test)
add_my_test(event_loop_test)
add_my_test(channel_test)
add_my_test(tool_test)
add_my_test(buffer_test)
add_my_test(log_test)
add_my_test(event_channel_test)
add_my_test(signal_channel_test)
