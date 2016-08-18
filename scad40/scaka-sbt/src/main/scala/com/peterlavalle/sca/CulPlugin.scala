package com.peterlavalle.sca

import java.io.File

import sbt.Keys._
import sbt.{AutoPlugin, SettingKey, TaskKey}

object CulPlugin extends AutoPlugin {

	import autoImport._

	override lazy val projectSettings =
		Seq(
			culRoots := Seq(
				SourceTree.of(baseDirectory.value / "src/main/scaka")
			),

			culDependencies := Seq(),

			culModule := {
				Cul.Module(
					name.value,
					culRoots.value,
					culDependencies.value
				)
			},

			culAggregate := Seq(culModule.value),

			culSolvers :=
				Set(
					CulSolVS2015,
					CulSolCMake
				),

			cul := {
				culSolvers.value.flatMap {
					case solver =>
						solver(
							target.value / solver.getClass.getSimpleName.replaceAll("\\W", ""),
							Cul.Solution(name.value, culAggregate.value.toSet)
						)
				}
			}
		)

	object autoImport {

		lazy val culRoots = SettingKey[Seq[SourceTree.TSource]](
			"culRoots", "???")

		lazy val culDependencies = SettingKey[Seq[Cul.Module]](
			"culDependencies", "???")

		lazy val culModule = SettingKey[Cul.Module](
			"culModule", "???")

		lazy val culAggregate = SettingKey[Seq[Cul.Module]](
			"culAggregate", "???")

		lazy val culSolvers = SettingKey[Set[Cul.TSolver]](
			"culSolvers", "???")

		lazy val cul = TaskKey[Set[File]](
			"cul", "???")
	}

}
