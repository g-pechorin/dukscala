package com.peterlavalle.sca

import java.io.{File, StringWriter}

object ColAppCMake extends Col.TSolver {

	override def emit(dir: File, modules: Stream[Col.Module]): Set[File] =
		Set(
			(dir / "CMakeLists.txt")
				.overWriter
				.append(
					"""
						|cmake_minimum_required(VERSION 3.1.0)
						|
						|set(BUILD_SHARED_LIBS OFF CACHE BOOL "Don't use shared libs - ever" FORCE)
						|#set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "Don't use shared vcrt - it saves a few kB of file size but causes problems that only show up on user machines" FORCE)
						|
						|add_definitions(
						|	-DColCMakeApp=true
						|	-D_CRT_SECURE_NO_WARNINGS
						|	-DBUILD_SHARED_LIBS=false
						|#	-DUSE_MSVC_RUNTIME_LIBRARY_DLL=false
						|	-DCMAKE_BINARY_DIR="${CMAKE_BINARY_DIR}"
						|)
						|if(WIN32)
						|#	set(CompilerFlags
						|#		CMAKE_CXX_FLAGS
						|#		CMAKE_CXX_FLAGS_DEBUG
						|#		CMAKE_CXX_FLAGS_RELEASE
						|#		CMAKE_C_FLAGS
						|#		CMAKE_C_FLAGS_DEBUG
						|#		CMAKE_C_FLAGS_RELEASE)
						|#	foreach(CompilerFlag ${CompilerFlags})
						|#		string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
						|#	endforeach()
						|#
						|#	set(CompilerFlags
						|#		CMAKE_C_FLAGS_MINSIZEREL
						|#		CMAKE_CXX_FLAGS_MINSIZEREL)
						|#	foreach(CompilerFlag ${CompilerFlags})
						|#		string(REPLACE "/MT" "/MD" ${CompilerFlag} "${${CompilerFlag}}")
						|#	endforeach()
						|else(WIN32)
						|	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -std=gnu++11 -static-libstdc++")
						|	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
						|endif(WIN32)
						|set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
						|set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
					""".stripMargin.trim
				)

				.append("\ninclude_directories(")
				.mappend(modules.flatMap(_.allFolders).distinct.filter(_.exists()))("\n\t" + _.AbsolutePath)
				.append("\n)\n")

				.mappend(Col.Module.inOrder(modules).filterNot(_.allSourceFiles.isEmpty)) {
					case module: Col.Module =>
						new StringWriter()
							.append(
								module.artifact match {
									case Col.Module.Shared => s"\nadd_library(${module.name} MODULE"
									case Col.Module.Static => s"\nadd_library(${module.name} OBJECT"
									case Col.Module.Binary => s"\nadd_executable(${module.name}"
								}
							)
							.mappend(module.allSourceFiles)(file => s"\n\t${file.AbsolutePath}")
							.mappend(module.allHeaderFiles)(file => s"\n\t${file.AbsolutePath}")
							.append("\n)\n")
							.toString
				}
				.closeFile
		)
}
