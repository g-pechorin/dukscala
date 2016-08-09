package com.peterlavalle.sca

import java.io.File

import sbt.Keys._
import sbt.{AutoPlugin, SettingKey, TaskKey}

object ArtPlugin extends AutoPlugin {

	override lazy val projectSettings =
		Seq(
			artOut := target.value / "art.out",
			artCache := target.value / "art.cache",
			artRoots := Seq(
				baseDirectory.value / "src/main/art/",
				baseDirectory.value / "src/art/",
				baseDirectory.value / "art/"
			),
			artTools := Seq(
			),
			artSweeps := Seq(),
			art := {
				val tools =
					artTools.value.map(t => t.name -> t).toMap

				artSweeps.value.flatMap {
					case (toolName: String, pattern: String, command: String) =>
						tools(toolName) match {
							case tool: Art.TTool =>
								Art.sweep(
									artRoots.value,
									target.value,
									tool,
									pattern,
									command
								)
						}
				}
			}
		)

	import autoImport._

	object autoImport {
		lazy val artOut = SettingKey[File](
			"artOut", "where we should spit out files")

		lazy val artCache = SettingKey[File](
			"artCache", "where to dump the inevitable crap we generate")

		lazy val artRoots = SettingKey[Seq[File]](
			"artRoots", "places to look for art files we'll eat")

		lazy val artTools = SettingKey[Seq[Art.TTool]](
			"artTools", "???")

		lazy val artSweeps = SettingKey[Seq[(String, String, String)]](
			"artSweeps", "???")

		lazy val art = TaskKey[Seq[File]](
			"art", "run the art tools")
	}
}

