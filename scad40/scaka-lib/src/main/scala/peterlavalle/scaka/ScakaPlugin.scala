package peterlavalle.scaka

import java.io.{File, FileOutputStream, FileWriter}
import java.net.URL
import java.util.zip.ZipFile

import sbt.Keys._
import sbt.plugins.JvmPlugin
import sbt.{AutoPlugin, Plugins, SettingKey, TaskKey}

import scala.collection.JavaConversions._

object ScakaPlugin extends AutoPlugin {

	override val requires: Plugins = JvmPlugin


	object autoImport {

		lazy val scakaCMakeLibs =
			SettingKey[Seq[(String, String, Set[String])]](
				"scakaCMakeLibs",
				"[(url, dir, [libs])] to link into this project"
			)

		lazy val scakaCMakeScrape =
			TaskKey[Map[String, java.io.File]](
				"scakaCMakeScrape",
				"scrape all CMakeLibs and return a mapping to the unpacked dirs they produce"
			)

		lazy val scakaList =
			TaskKey[Rollo.ScList](
				"scakaList",
				"list of all native sources"
			)

		lazy val scakaCMakeForce =
			SettingKey[Seq[(String, Any, String)]](
				"scakaCMakeForce",
				"force-set these values in the CMake file"
			)
		lazy val scakaCMakeFile =
			TaskKey[File](
				"scakaCMakeFile",
				"write a CMakeList file"
			)
	}

	import autoImport._

	override lazy val projectSettings =
		Seq(
			scakaCMakeLibs := Seq(),
			scakaCMakeForce := Seq(),
			scakaCMakeScrape := {
				// TODO ; requyre that all urls will map to unique paths
				// TODO ; touchfile for timestamps

				scakaCMakeLibs.value.map {
					case (url: String, path: String, _) =>
						requyre(path.matches("([^/]+/)*"))
						url -> {
							val name: String = "cache/" + url.split("/+", 3)(2).replaceAll("[[\\W]&&[^\\.]]+", "-")
							requyre(!name.contains(".."))

							val List(dumpFolder, extension) = name.splyt("\\.", -2)
							val dirFile = target.value / dumpFolder

							val cacheFile = target.value / name

							if (!cacheFile.exists() || !dirFile.exists()) {
								requyre(cacheFile.getParentFile.exists() || cacheFile.getParentFile.mkdirs())

								(new FileOutputStream(cacheFile) << new URL(url).openStream()).close()

								extension match {
									case "zip" =>

										val zipFile = new ZipFile(cacheFile)

										zipFile.entries().filterNot(_.isDirectory).foreach {
											case entry =>
												val dumpFile = dirFile / entry.getName

												requyre(dumpFile.getParentFile.exists() || dumpFile.getParentFile.mkdirs())

												(new FileOutputStream(dumpFile) << zipFile.getInputStream(entry)).close()
										}
								}

							}
							dirFile
						}
				}.toMap
			},

			scakaList := {
				Rollo(
					name.value,
					baseDirectory.value.getAbsoluteFile
				)
			},

			scakaCMakeFile := {

				println(s">> ${name.value} >>")
				projectDependencies.value.foreach {
					case moduleId: sbt.ModuleID =>
						println("=======")
						println(moduleId)
						println(moduleId.getClass.getName)




				}



				thisProject.value.autoPlugins.foreach(println)
				println(s"<<< ${name.value} <")


				val cmakeFile: File = {
					val targetFile: File = target.value
					requyre(targetFile.exists() || targetFile.mkdirs())

					// HACK ; Why for is the implicit going away?
					targetFile / "CMakeLists.txt"
				}

				val cmakeWriter = new FileWriter(cmakeFile)

				val includes: Set[(Char, String)] =
					scakaCMakeLibs.value.flatMap {
						case (url, path, inks) =>
							val dumpedFolder: File = scakaCMakeScrape.value(url) / path

							inks.map {
								case inc if inc.endsWith("/") =>
									'd' -> (cmakeFile.getParentFile / (dumpedFolder / inc))

								case lib =>
									'l' -> lib
							} + ('i' -> (cmakeFile.getParentFile / dumpedFolder))
					}.toSet

				Frollo(
					cmakeWriter,
					Rollo.Module(
						scakaCMakeForce.value,

						// include ()
						includes.filter(_._1 == 'i').map(_._2),

						// include_directories ()
						includes.filter(_._1 == 'd').map(_._2),

						// scList
						scakaList.value,

						// linked
						includes.filter(_._1 == 'l').map(_._2)
					)
				)

				cmakeWriter.close()

				cmakeFile
			}

			// TODO ; watch everything from scakaList

			// TODO ; add our crapola to clean
		)
}
