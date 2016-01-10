cmake_minimum_required (VERSION 3.0)

if(WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -std=gnu++11 -static-libstdc++")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
endif(WIN32)

project(pal)

add_definitions(-DCMAKE_BINARY_DIR="${CMAKE_BINARY_DIR}")
add_definitions(-DCMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

include_directories(
	../duktape
	
	# this test runs against the expectations
	src/test/resources
)

# main executable
add_executable(
	peterlavalle.diskio

		../duktape/duk_config.h
		../duktape/duktape.c
		../duktape/duktape.h

		src/test/cmake/peterlavalle.diskio-test.cpp
		src/test/resources/peterlavalle.diskio.hpp
	)
