set(BASE_SRC
        buffer.cpp
    )
add_library(malice_base SHARED ${BASE_SRC})
target_include_directories(malice_base PUBLIC ${CMAKE_SOURCE_DIR}/include)
