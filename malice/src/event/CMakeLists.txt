set(EVENT_SRC
        event.cpp
        event_loop.cpp
        channel.cpp
        event_channel.cpp
        signal_channel.cpp
    )
add_library(malice_event SHARED ${EVENT_SRC})
target_include_directories(malice_event PUBLIC ${CMAKE_SOURCE_DIR}/malice/include)
