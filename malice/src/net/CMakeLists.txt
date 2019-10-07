set(NET_SRC
        address.cpp
    )
add_library(malice_net SHARED ${NET_SRC})
target_include_directories(malice_net PUBLIC ${CMAKE_SOURCE_DIR}/malice/include)
