cmake_minimum_required (VERSION 3.0)

if(WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -std=gnu++11 -static-libstdc++")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
endif(WIN32)

project(scad40-test)

add_definitions(-DCMAKE_BINARY_DIR="${CMAKE_BINARY_DIR}")
add_definitions(-DCMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

include_directories(
	../duktape
	gen
)

include(cmade/pal_glfw-3.1.2.cmake)

include(../scad40/src/test/cmake/pal_gtest.cmake)

add_executable(
	scad40-gtest

		../duktape/duk_config.h
		../duktape/duktape.c
		../duktape/duktape.h

		gen/D40.hpp

		scad40-gtest.cpp
	)
	pal_gtest_wire(scad40-gtest)

add_executable(
	scad40-glfw

		../duktape/duk_config.h
		../duktape/duktape.c
		../duktape/duktape.h

		gen/D40.hpp

		scad40-glfw/glfw_queue.hpp
		scad40-glfw/main.cpp
		scad40-glfw/peterlavalle.sandbox.Magpie.cpp
		scad40-glfw/peterlavalle.sandbox.Time.cpp
	)
	target_link_libraries(scad40-glfw glfw ${GLFW_LIBRARIES})
