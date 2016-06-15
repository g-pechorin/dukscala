cmake_minimum_required (VERSION 3.0)

if(WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -std=gnu++11 -static-libstdc++")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
endif(WIN32)


add_definitions(-DCMAKE_BINARY_DIR="${CMAKE_BINARY_DIR}")
add_definitions(-DCMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

include_directories(../duktape)

project(example02)

# magic thingie to re-build the Scaja-JS project
	add_custom_target(
		script-02
			${CMAKE_SOURCE_DIR}/../sbt fastOptJS
	    	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/"
	    	COMMENT "Generating .js file ..."
		)

# native executable
add_executable(
	native-02

		../duktape/duk_config.h
		../duktape/duktape.c
		../duktape/duktape.h

		dukscala-02.cpp
	)
#	if (NOT EXISTS target/scala-2.10/scala-js-tutorial-fastopt.js)
#		add_dependencies(native-02 script-02)
#	endif()
