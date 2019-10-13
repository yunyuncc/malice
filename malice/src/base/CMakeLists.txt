set(BASE_SRC
        buffer.cpp
        tool.cpp
    )
add_library(malice_base SHARED ${BASE_SRC})
target_include_directories(malice_base PUBLIC ${CMAKE_SOURCE_DIR}/malice/include)
