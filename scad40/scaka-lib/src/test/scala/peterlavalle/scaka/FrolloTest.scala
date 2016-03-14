package peterlavalle.scaka

import junit.framework.TestCase
import org.junit.Assert._
import peterlavalle.Home

class FrolloTest extends TestCase {
	def testFailThing(): Unit = {
		assertEquals(
			s"""
				|cmake_minimum_required(VERSION 3.0.2)
				|
				|if(WIN32)
				|	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
				|else(WIN32)
				|	set(CMAKE_CXX_FLAGS "${"${CMAKE_CXX_FLAGS}"} -std=c++11 -std=gnu++11 -static-libstdc++")
				|	set(CMAKE_EXE_LINKER_FLAGS "${"${CMAKE_EXE_LINKER_FLAGS}"} -static-libstdc++")
				|endif(WIN32)
				|
				|add_definitions(-DCMAKE_BINARY_DIR="${"${CMAKE_BINARY_DIR}"}")
				|add_definitions(-DCMAKE_SOURCE_DIR="${"${CMAKE_SOURCE_DIR}"}")
				|
				|add_library(
				|	${getName} OBJECT
				|		../src/main/scaka/hep.hpp
				|		../src/main/scaka/something.h
				|		../src/main/scaka/thingie.cpp
				|	)
				|
				|include_directories(../src/main/scaka)
				|	# foo
				|		add_executable(
				|			${getName}-foo
				|				../src/test/scaka/foo.cpp
				|				../src/main/scaka/hep.hpp
				|				../src/main/scaka/something.h
				|				../src/main/scaka/thingie.cpp
				|		)
				|	add_test(
				|		NAME ${getName}-foo
				|		COMMAND ${getName}-foo
				|		WORKING_DIRECTORY ${"${CMAKE_SOURCE_DIR}"}
				|	)
				|
				|
			""".stripMargin.trim.replaceAll("([\r \t]*\n)+", "\n"),
			Frollo(
				Rollo.ScList(
					getName,
					Home.projectFolder / "src/test/thingie/",
					Set("thingie.cpp"),
					Set("hep.hpp", "something.h"),
					Set(),
					Set("foo.cpp")
				)
			).trim.replaceAll("([\r \t]*\n)+", "\n")
		)
	}
}
