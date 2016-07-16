package com.peterlavalle.sca

import java.io.File
import com.peterlavalle.sca._
import sbt.Keys._
import sbt.{AutoPlugin, SettingKey, TaskKey}

object ColPlugin extends AutoPlugin {

	object autoImport {

		lazy val colRoots = SettingKey[Seq[Col.TSource]](
			"colRoots", "file objects that're source roots")

		lazy val colDependencies = SettingKey[Seq[Col.Module]](
			"colDependencies", "???")

		lazy val colModule = SettingKey[Col.Module](
			"colModule", "???")

		lazy val colAggregate = SettingKey[Seq[Col.Module]](
			"colAggregate", "???")

		lazy val colSolvers = SettingKey[Set[Col.TSolver]](
			"colSolvers", "???")

		lazy val col = TaskKey[Set[File]](
			"col", "???")
	}

	import autoImport._

	override lazy val projectSettings =
		Seq(


			colRoots := Seq(Col.Folder(new File(baseDirectory.value, "src/main/scaka"))),
			colDependencies := Seq(),

			colModule := {
				Col.Module(
					name.value,
					colRoots.value,
					colDependencies.value
				)
			},


			colAggregate := Seq(),
			colSolvers :=
				Set(
					ColAppCMake,
					ColAppVS2015
				),
			col := {
				colSolvers.value.flatMap {
					case solver =>
						solver.emit(
							target.value / solver.getClass.getSimpleName.replaceAll("\\W", ""),
							Col.Module.labelArtifacts(
								(colModule.value, colAggregate.value) match {
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

