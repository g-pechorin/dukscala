package com.peterlavalle.sca


import java.io.{File, InputStream}
import java.net.URL
import java.util.zip.ZipInputStream

import com.peterlavalle.sca.Horse.SourceSet
import sbt.Keys._
import sbt.{AutoPlugin, Reference, SettingKey, TaskKey}

object GinnyPlugin extends AutoPlugin {
	/*
		This should be an awesome plugin for generating multi-module CMakeLists

		... it might let you down W.R.T. inc/ chains from modules
		... and it doesn't do any testing (yet!)
		... but it should show a P.o.C. right?
	*/


	type Slug = (URL /* remote address */ , String /* internal path to CMake */ , Seq[String] /* dirs and symbols */ )

	object autoImport {

		lazy val ginnyFresh = SettingKey[Boolean]("ginnyFresh", "should Ginny download fresh copies of everything even if pre-existing stuff is there?")

		lazy val ginnyRemotes = SettingKey[Seq[Slug]]("ginnyRemotes", "stuff that Ginny should download and extract")
		lazy val ginnyBogies = TaskKey[Seq[Horse.Bogey]]("ginnyBogies", "get Ginny to download stuff and extract it")

		// TODO ; use this ; http://www.scala-sbt.org/0.12.1/docs/Detailed-Topics/Paths.html
		lazy val ginnyPattern = SettingKey[String]("ginnyPattern", "regex used to find sources")

		lazy val ginnyBuildSBT = SettingKey[File]("ginnyBuildSBT", "The build file")

		lazy val ginnyMainRoots = SettingKey[Seq[File]]("ginnyMainRoots", "list of to lookin for a project's sources")
		lazy val ginnyUses = TaskKey[Seq[Horse.TListing]]("ginnyUses", "HACK ; dependencies. can't 'just` use dependencies for £reasons")
		lazy val ginnyListing = TaskKey[Horse.TListing]("ginnyListing", "workout how Ginny is doing this stuff")
		lazy val ginnyCMakeFile = SettingKey[File]("ginnyCMakeFile", "Ginny's CMakeLists.txt file")
		lazy val ginny = TaskKey[Seq[File]]("ginny", "Ginny's magical generators")

		lazy val ginnyGenerate = TaskKey[Seq[File]]("ginnyGenerate", "Folders of auto-generated source")

		lazy val ginnySets = SettingKey[Seq[(String, Any, String)]]("ginnySets", "Things that'll be set in the preprocessor and CMakeCache")
	}

	import autoImport._

	override lazy val projectSettings =
		Seq(
			ginnyBuildSBT := {
				def recu(folder: File): File =
					new File(folder, "build.sbt") match {
						case buildSbt: File if buildSbt.exists() => buildSbt

						case _ if null != folder.getParentFile && folder != folder.getParentFile =>
							recu(folder.getParentFile)
					}

				recu(baseDirectory.value.getAbsoluteFile)
			},
			ginnyFresh := false,
			ginnyRemotes := Seq(),
			ginnyBogies := {
				ginnyRemotes.value.map {
					case (remote: URL, path: String, symbols: Seq[String]) =>
						val dump: File = target.value / remote.getFile.replaceAll("/", "_").replaceAll("[_\\.]{2,}", "_")

						if (dump.exists() && !ginnyFresh.value)
							streams.value.log.info(s"Re-using `${
								remote.getFile
							}`")
						else {
							if (ginnyFresh.value)
								dump.deleteAll()
							new ZipInputStream(remote.openStream()).files.foreach {
								case (file: String, stream: InputStream) =>
									// TODO ; why aren't my implicits working here?
									dump / file << stream

									streams.value.log.info(s"Extracted ${file}")
							}
						}

						val cmake = {
							val cmake = dump / s"${path}/CMakeLists.txt"
							requyre(
								cmake.exists(),
								s"I couldn't find a CMakeLists.txt in ${cmake.getAbsolutePath.replace("\\", "/")}"
							)
							cmake.getParentFile
						}

						symbols.filter(_.endsWith("/")).foreach(i => requyre((wrapFile(dump) / s"${i}").exists(), s"The inc `${i}` does not exits"))
						symbols.filter(_.endsWith("/")).foreach(i => requyre((wrapFile(dump) / s"${i}").isDirectory, s"The inc `${i}` is not a dir"))

						Horse.Bogey(cmake, symbols.filter(_.endsWith("/")).map(i => wrapFile(dump) / s"${i}"), symbols.filterNot(_.endsWith("/")))
				}
			},

			ginnySets := {
				Seq(
					("BUILD_SHARED_LIBS", false, "Don't use shared libs - ever"),
					("USE_MSVC_RUNTIME_LIBRARY_DLL", false, "Don't use shared vcrt - it saves a few kB of file size but causes problems that only show up on user machines"),
					("BASE_DIRECTORY", baseDirectory.value, "Where the project was built from")
				)
			},
			ginnyPattern := "(\\w+/)*\\w+([\\-\\.]\\w+)*\\.(c|cc|cpp|cxx|h|hh|hpp|hxx)",
			ginnyMainRoots := {
				Seq(
					baseDirectory.value / "src/main/scaka"
				)
			},
			ginnyUses := Seq(),
			ginnyListing := {

				// collect all
				val sourceSets: Seq[SourceSet] =
					(ginnyMainRoots.value ++ ginnyGenerate.value)
						.map((root: File) => Horse.SourceSet(root, (root ** ginnyPattern.value).toSet))
						.filter(_.srcs.nonEmpty)

				val uses =
					ginnyUses.value

				(sourceSets match {

					case Seq() =>
						requyre(ginnyBogies.value.nonEmpty, s"The project ${name.value} needs to do something")
						(u: Seq[Horse.TListing]) => {
							requyre(u.isEmpty)
							Horse.ProxyListing(ginnyBogies.value)
						}

					case app: Seq[Horse.SourceSet] if app.exists(_.srcs.exists(_.matches(appPattern))) =>
						requyre(ginnyBogies.value.isEmpty)
						requyre(!app.exists(_.srcs.exists(_.matches(extPattern))))
						(u: Seq[Horse.TListing]) => Horse.AppListing(name.value, app, u)

					case ext: Seq[Horse.SourceSet] if ext.exists(_.srcs.exists(_.matches(extPattern))) =>
						requyre(ginnyBogies.value.isEmpty)
						requyre(!ext.exists(_.srcs.exists(_.matches(appPattern))))
						??

					case obj: Seq[Horse.SourceSet] =>
						requyre(ginnyBogies.value.isEmpty)
						requyre(!obj.exists(_.srcs.exists(_.matches(extPattern))))
						requyre(!obj.exists(_.srcs.exists(_.matches(appPattern))))
						(u: Seq[Horse.TListing]) => Horse.LibListing(obj, u)

				}) (uses)
			},
			ginnyCMakeFile := target.value / "CMakeLists.txt",
			ginny := {
				val cmakeFile: File = ginnyCMakeFile.value

				val writer = cmakeFile.overWriter
				GinnyFile(
					writer,
					Horse.Mouth(
						ginnyBuildSBT.value,
						ginnyListing.value,
						ginnySets.value,
						baseDirectory.value
					)
				)
				writer.close()

				Seq(
					cmakeFile
				)
			},

			// by default - don't generate anything
			ginnyGenerate := Seq()
		)

	val extPattern = "(extension|module)\\.(c|cpp)"
	val appPattern = "(application|main)\\.(c|cpp)"
}
