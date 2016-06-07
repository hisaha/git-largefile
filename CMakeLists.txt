cmake_minimum_required(VERSION 2.8)

#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_BUILD_TYPE Release)

if (APPLE)
else()
        list( APPEND CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")
endif ()

include_directories( /usr/local/include )
link_directories( /usr/local/lib )

if (APPLE)
	include_directories( /opt/local/include )
	link_directories( /opt/local/lib )
endif ()

file(GLOB mainsrc "gitbig.cpp")

add_executable(gitbig ${mainsrc})

if (APPLE)
	target_link_libraries(gitbig crypto)
else()
	target_link_libraries(gitbib crypto pthread)
endif ()

install (TARGETS gitbig DESTINATION bin PERMISSIONS WORLD_EXECUTE)


