set(EVENT_SRC
        event.cpp
        event_loop.cpp
    )
add_library(malice_event SHARED ${EVENT_SRC})
target_include_directories(malice_event PUBLIC ${CMAKE_SOURCE_DIR}/include)
