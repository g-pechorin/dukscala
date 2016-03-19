package com.peterlavalle.sca

import java.io.{File, FileOutputStream}
import java.net.URL
import java.util.zip.ZipFile

import sbt.Keys._
import sbt.plugins.JvmPlugin
import sbt.{AutoPlugin, Plugins, SettingKey, TaskKey}

import scala.collection.JavaConversions._

object ScakaPlugin extends AutoPlugin {

	override val requires: Plugins = JvmPlugin


	object autoImport {

		lazy val scakaSourceRegex =
			SettingKey[String](
				"scakaSourceRegex",
				"pattern to use in looking for source files"
			)

		lazy val scakaCMakeLibs =
			SettingKey[Seq[(String, String, Set[String])]](
				"scakaCMakeLibs",
				"[(url, dir, [libs])] to link into this project"
			)

		lazy val scakaCMakeRoots =
			SettingKey[Seq[File]](
				"scakaCMakeRoots",
				"folders to look for source-code in"
			)

		lazy val scakaCMakeScrape =
			TaskKey[Map[String, java.io.File]](
				"scakaCMakeScrape",
				"scrape all CMakeLibs and return a mapping to the unpacked dirs they produce"
			)

		lazy val scakaCMakeForce =
			SettingKey[Seq[(String, Any, String)]](
				"scakaCMakeForce",
				"force-set these values in the CMake file"
			)
		lazy val scakaCMakeFile =
			SettingKey[File](
				"scakaCMakeFile",
				"The CMakeList file"
			)
		lazy val scakaCMakeGenerate =
			TaskKey[File](
				"scakaCMakeGenerate",
				"write a CMakeList file"
			)
	}

	import autoImport._

	override lazy val projectSettings =
		Seq(
			scakaSourceRegex := "(\\w+/)*\\w+([\\-\\.]\\w+)*\\.(c|cc|cpp|cxx|h|hh|hpp|hxx)",
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

			scakaCMakeFile := {
				val targetFile: File = target.value
				requyre(targetFile.exists() || targetFile.mkdirs())

				// HACK ; Why for is the implicit going away?
				targetFile / "CMakeLists.txt"
			},

			scakaCMakeRoots := {
				Seq(baseDirectory.value / "src/main/scaka/")
			},

			scakaCMakeGenerate := {

				val cmakeFile: File =
					scakaCMakeFile.value

				val cmakeWriter = cmakeFile.overWriter

				val includes: Seq[(Char, String)] =
					scakaCMakeLibs.value.flatMap {
						case (url, path, inks) =>
							val dumpedFolder: File = scakaCMakeScrape.value(url) / path

							inks.map {
								case inc if inc.endsWith("/") =>
									'd' -> (cmakeFile.getParentFile / (dumpedFolder / inc))

								case lib =>
									'l' -> lib
							} + ('i' -> (cmakeFile.getParentFile / dumpedFolder))
					}

				Frollo(
					cmakeWriter,
					Rollo.Module(
						scakaCMakeForce.value,

						// include ()
						includes.filter(_._1 == 'i').map(_._2),

						// include_directories ()
						includes.filter(_._1 == 'd').map(_._2),

						// scList
						Rollo.ScList(
							name.value, scakaCMakeFile.value,
							scakaCMakeRoots.value.flatMap {
								case root =>
									(root *** scakaSourceRegex.value).map(_._2)
							},
							scakaCMakeRoots.value
						),

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
