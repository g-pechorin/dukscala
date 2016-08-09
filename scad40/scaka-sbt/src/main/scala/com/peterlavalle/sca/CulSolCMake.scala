package com.peterlavalle.sca

import java.io.{File, StringWriter}

import com.peterlavalle.sca.Cul.Solution

object CulSolCMake extends Cul.TSolver {
	override def apply(root: File, solution: Solution): Seq[File] =
		Seq(
			(root / "CMakeLists.txt").overWriter
				.append(
					s"""
						 |cmake_minimum_required(VERSION 3.1)
						 |
						 |project(${solution.name})
						 |""".stripMargin)

				.append(
					"""
						|
						|add_definitions(
						|	-DSCAKA=true
						|	-D_CRT_SECURE_NO_WARNINGS
						|	-D_SCL_SECURE_NO_WARNINGS
						|	-D_CRT_NONSTDC_NO_WARNINGS
						|	-DBUILD_SHARED_LIBS=false
						|	-DCMAKE_BINARY_DIR="${CMAKE_BINARY_DIR}"
						|)
						|if(WIN32)
						|else(WIN32)
						|	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -std=gnu++11 -static-libstdc++")
						|	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
						|endif(WIN32)
						|
						|set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
						|set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
						|
					""".stripMargin
				)

				.append("\ninclude_directories(\n")
				.mappend(
					solution.transitiveNodes
						.flatMap(_.sources.map(_.root))
						.filter(_.exists())
				) {
					case file =>
						s"\t\t${file.AbsolutePath}\n"
				}
				.append("\t)\n")
				.mappend(solution.branchNodes) {
					case library: Cul.Module =>
						val stringWriter = new StringWriter()

						if (library.hasSources)
							stringWriter
								.append(s"\nadd_library(\n\t${library.name} OBJECT\n")
								.mappend(library.sources.flatMap(_.files))(file => s"\t\t${file.AbsolutePath}\n")
								.append("\t)\n")

						stringWriter.toString
				}

				.mappend(solution.leafNodes.filter(_.hasSources)) {
					case executable: Cul.Module =>
						new StringWriter()
							.append(s"\nadd_executable(\n\t${executable.name}\n")
							.mappend(executable.sources.flatMap(_.files))(file => s"\t\t${file.AbsolutePath}\n")
							.mappend(executable.transitiveDependencies.toStream.filter(_.hasSources)) {
								case lib: Cul.Module =>
									s"\t\t$$<TARGET_OBJECTS:${lib.name}>\n"
							}
							.append("\t)\n")
							.toString
				}
				.closeFile
		)
}
