cmake_minimum_required (VERSION 3.0)

add_definitions(
	-D_CRT_SECURE_NO_WARNINGS
	-DCMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)
if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
else(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -std=gnu++11 -static-libstdc++")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
endif(WIN32)

include(cmade/pal_gtest.cmake)

include_directories(
		src/inc
		../duktape/
		src/gen
	)

add_executable(
	toy-duk
		src/toy/toymain.cpp

		src/inc/liebeck.hpp
		src/inc/palduk.hpp
		src/inc/percolator.hpp
		src/inc/buzzbird.hpp

		../duktape/duk_config.h
		../duktape/duktape.c
		../duktape/duktape.h
    )

add_executable(
	d40-duk
		src/d40/d40main.cpp
		src/d40/underlay.scad40

		src/gen/d40-duk.hpp

		src/inc/liebeck.hpp
		src/inc/palduk.hpp
		src/inc/percolator.hpp
		src/inc/buzzbird.hpp

		../duktape/duk_config.h
		../duktape/duktape.c
		../duktape/duktape.h
  )
	find_package(Java COMPONENTS Runtime)
	add_custom_command(
		TARGET d40-duk PRE_BUILD
		COMMAND ${Java_JAVA_EXECUTABLE} -jar ../sbt-launch.jar @scad40-launch.properties -i src/d40 -p com.peterlavalle.sca.D40 src/gen/d40-duk.hpp
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	)
