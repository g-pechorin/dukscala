package com.peterlavalle.sca

import java.io.{File, StringWriter}

object ColCMakeApp extends Col.TSolver {

	import Col._

	override def emit(dir: File, modules: Stream[(Module, Boolean)]): Set[File] =
		Set(
			(dir / "CMakeLists.txt")
				.overWriter
				.append(
					"""
						|cmake_minimum_required(VERSION 3.1.0)
						|
						|set(BUILD_SHARED_LIBS OFF CACHE BOOL "Don't use shared libs - ever" FORCE)
						|set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "Don't use shared vcrt - it saves a few kB of file size but causes problems that only show up on user machines" FORCE)
						|
						|add_definitions(
						|	-DColCMakeApp=true
						|	-D_CRT_SECURE_NO_WARNINGS
						|	-DBUILD_SHARED_LIBS=false
						|	-DUSE_MSVC_RUNTIME_LIBRARY_DLL=false
						|	-DCMAKE_BINARY_DIR="${CMAKE_BINARY_DIR}"
						|)
						|if(WIN32)
						|	set(CompilerFlags
						|		CMAKE_CXX_FLAGS
						|		CMAKE_CXX_FLAGS_DEBUG
						|		CMAKE_CXX_FLAGS_RELEASE
						|		CMAKE_C_FLAGS
						|		CMAKE_C_FLAGS_DEBUG
						|		CMAKE_C_FLAGS_RELEASE)
						|	foreach(CompilerFlag ${CompilerFlags})
						|		string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
						|	endforeach()
						|else(WIN32)
						|	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -std=gnu++11 -static-libstdc++")
						|	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
						|endif(WIN32)
					""".stripMargin.trim
				)

				.append("\ninclude_directories(")
				.mappend(modules.flatMap(_._1.fullSet).flatMap(_.roots.toSet)) {
					case source: Col.TSource =>
						val home = source.home
						if (home.exists()) s"\n\t${home.AbsolutePath}" else ""
				}
				.append("\n)\n")

				.mappend(Col.Module.inOrder(modules.map(_._1)).filterNot(_.allSourceFiles.isEmpty)) {
					case module: Col.Module =>

						val toMap: Map[String, Boolean] =
							Col.Module.inOrder(modules.map(_._1)).map {
								case entry: Col.Module =>
									modules.toList.find(p => p._1.name == entry.name && p._2) match {
										case None =>
											entry.name -> false
										case Some(_) => entry.name -> true
									}
							}.toMap

						new StringWriter()
							.append(
								if (toMap(module.name))
									s"\nadd_executable(${module.name}"
								else
									s"\nadd_library(${module.name} OBJECT"
							)
							.mappend(module.allSourceFiles)(file => s"\n\t${file.AbsolutePath}")
							.mappend(module.allHeaderFiles)(file => s"\n\t${file.AbsolutePath}")
							.append("\n)\n")
							.toString
				}
				.closeFile
		)
}
