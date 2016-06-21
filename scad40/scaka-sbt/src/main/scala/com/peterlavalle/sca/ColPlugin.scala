package com.peterlavalle.sca

import java.io.File
import com.peterlavalle.sca._
import sbt.Keys._
import sbt.{AutoPlugin, SettingKey, TaskKey}

object ColPlugin extends AutoPlugin {

	object autoImport {

		lazy val colRoots = SettingKey[Seq[File]](
			"colRoots", "file objects that're source roots")

		lazy val colLinked = SettingKey[Seq[Col.Module]](
			"colLinked", "???")

		lazy val colModule = SettingKey[Col.Module](
			"colModule", "???")

		lazy val colModules = SettingKey[Seq[Col.Module]](
			"colModules", "???")

		lazy val colSolvers = SettingKey[Set[Col.TSolver]](
			"colSolvers", "???")

		lazy val col = TaskKey[Set[File]](
			"col", "???")
	}

	import autoImport._

	override lazy val projectSettings =
		Seq(
			colRoots := Seq(new File(baseDirectory.value, "src/main/scaka")),
			colLinked := Seq(),
			colModule := {
				Col.Module(
					name.value,
					colRoots.value.map(f => Col.Root(f.getAbsoluteFile)),
					colLinked.value
				)
			},
			colModules := Seq(),
			colSolvers := Set(
				ColCMakeApp
			),
			col := {
				colSolvers.value.flatMap {
					case solver =>
						solver.emit(
							target.value / solver.getClass.getSimpleName.replaceAll("\\W", ""),
							Col.Module.labelArtifacts(
								(colModule.value, colModules.value) match {
									case (empty, peers) if empty.isEmpty && peers.nonEmpty =>
										peers.toList

									case (head, tail) =>
										head :: tail.toList
								}
							)
						)
				}
			}
		)
}

