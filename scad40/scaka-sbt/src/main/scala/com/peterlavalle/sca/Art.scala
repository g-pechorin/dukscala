package com.peterlavalle.sca

import java.io.File
import java.net.URL

import sbt.{IO, URL}

object Art {
	"""
		|binSweep ++=
		| Sweep(
		|   "((\\w+/)\\w+)\\.blend", "export <@ROOT@>/$0 $1.assbin -cfull"
		|   CMakeTool("assimp_cmd", CMakeProject("asd/asd/as", "http://sds.,mas/das/.asa"))
		|
		|
	""".stripMargin

	/**
		*
		* @param path path within zip to find CMakeLists.txt. if null - try to auto-find-it
		* @param from url to get the thing from
		*/
	case class CMakeProject(path: String, from: URL) {

		/**
			* Download and unpack the archive then return a File instance pointing to the path'ed folder
			*/
		def apply(cache: File): File =
			??
	}

	trait TWorkload {
		def ++=(job: String): TWorkload

		def run(): Stream[File]
	}

	trait TTool {
		val name: String

		def doWork(output: File, cache: File): TWorkload
	}

	case class CMakeTool(name: String, project: CMakeProject) extends TTool {
		override def doWork(output: File, cache: File): TWorkload = {

			class Workload(jobs: List[String]) extends TWorkload {
				override def ++=(job: String): Workload = new Workload(job :: jobs)

				override def run(): Stream[File] = {

					val local = scrape(project.from)

					val toolCode =
						local / (project.path match {
							case null =>
								local.list() match {
									case Array(only) if new File(new File(local, only), "CMakeLists.txt").exists() =>
										only
								}
							case path => path
						})

					val commands =
						jobs.map {
							case job =>
								s"""
									 |add_custom_command(
									 |	TARGET arttool PRE_BUILD
									 |		COMMAND ${"$"}<TARGET_FILE:$name> $job
									 |		WORKING_DIRECTORY ${output.AbsolutePath})
									""".stripMargin
						}

					val run = cache / s"${name}.run"
					(run / "CMakeLists.txt").overWriter
						.append(
							s"""
								 |cmake_minimum_required(VERSION 3.1.0)
								 |add_subdirectory(${toolCode.AbsolutePath} toolcode)
								 |add_custom_target(arttool)
								 |${commands.foldLeft("")(_ + _)}
							""".stripMargin)
						.close()

					requyre(output.exists() || output.mkdirs())

					val cmake =
						System.getProperty("os.arch") + "/" + System.getProperty("os.name").toLowerCase().replaceAll("\\W.*$", "") match {
							case "amd64/windows" =>
								scrape(new URL("https://cmake.org/files/v3.5/cmake-3.5.2-win32-x86.zip")) / "cmake-3.5.2-win32-x86/bin/cmake.exe"
						}

					val tmp = cache / s"${name}.tmp"

					import sys.process.Process

					requyre(
						(tmp.exists() || tmp.mkdirs()) &&
							0 == (Process(Seq(cmake.AbsolutePath, run.AbsolutePath), tmp) !)

					)

					requyre(0 == (Process(Seq(cmake.AbsolutePath, "--build", "./", "--target", "arttool"), tmp) !))

					sys.error(
						s"""
							 |Almost there - the generated script won't write pathed/files.out for some reason
							 |... i.e. it won't make-parent-dirs()
							 | jobs:${jobs.foldLeft("")(_ + "\n\t\t" + _)}
							 |
						""".stripMargin
					)
				}

				def scrape(url: URL): File = {
					val local: String = url.toString.replaceAll("\\.zip$", "").replaceAll("[_\\W]+", "_")
					val dir = new File(cache, local + ".dir")

					if (dir.exists()) {
						println(s"Already downloaded into  ${dir.getAbsolutePath}")
					} else {
						println(s"Downloading from ${url} ...")
						IO.unzipURL(url, dir)
						println(s"... into ${dir.getAbsolutePath}")
					}
					dir
				}
			}

			new Workload(Nil)
		}
	}

	def CMake(name: String, from: String): CMakeTool = CMakeTool(name, CMakeProject(null, new URL(from)))

	def sweep(roots: Seq[File], target: File, tool: TTool, pattern: String, command: String) =
		roots.foldLeft(tool.doWork(
			target / "art.out",
			target / "art.cache"
		)) {
			case (work, root: File) =>
				root.list() match {
					case null => work

					case list =>
						def recu(workload: TWorkload, todo: List[String]): TWorkload =
							todo match {
								case Nil => workload

								case name :: tail =>
									val file = new File(root, name)

									recu(
										if (!name.matches(pattern))
											workload
										else
											workload ++= name.replaceAll(
												pattern,
												command.replace("<@ROOT@>", root.AbsolutePath)
											),
										file.list() match {
											case null => tail
											case children => children.toList.map(name + "/" + _) ++ tail
										}
									)
							}

						recu(work, list.toList)
				}
		}.run()


	case class Sweep(tool: TTool, pattern: String, command: String) {
		def apply(roots: Seq[File]) =
			roots.foldLeft(Stream[File]()) {
				case (done: Stream[File], root) =>
					root.list() match {
						case null | Array() =>
							done
						case list =>

							def recu(todo: List[String]): Stream[File] =
								todo match {
									case Nil => Stream.Empty

									case name :: tail =>
										val file = new File(root, name)

										val front: Stream[File] =
											if (name.matches(pattern)) {

												val commandLine =
													name.replaceAll(
														pattern,
														command
															.replace("<@ROOT@>", root.AbsolutePath)
													)

												sys.error(commandLine)
												sys.error("So ... add that to a workload")

											} else Stream.Empty

										front ++ recu(file.list() match {
											case null => tail
											case children => children.toList.map(name + "/" + _) ++ tail
										})
								}

							done ++ recu(list.toList)
					}
			}
	}

}
