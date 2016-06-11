package com.peterlavalle.sca

import java.io.File

import sbt.Keys._
import sbt.{AutoPlugin, SettingKey, TaskKey}

object SolPlugin extends AutoPlugin {

	object autoImport {

		lazy val solRoots = SettingKey[Seq[File]](
			"solRoots", "???")

		lazy val solSourceRoots = TaskKey[Seq[Sol.SourceRoot]](
			"solSourceRoots", "???")

		lazy val solRequirements = SettingKey[Set[Sol.Requirement]](
			"solRequirements", "???")

		lazy val solObjectModule = TaskKey[Sol.ObjectModule](
			"solModule", "???")

		lazy val solSolution = SettingKey[Boolean](
			"solSolution", "???")

		lazy val sol = TaskKey[Seq[File]](
			"sol", "???")
	}

	import autoImport._

	override lazy val projectSettings =
		Seq(
			solRoots := Seq(new File(baseDirectory.value, "src/main/scaka")),
			solSourceRoots := solRoots.value.map(Sol.SourceRoot),
			solRequirements := Set(),
			solObjectModule := Sol.ObjectModule(
				baseDirectory.value,
				solSourceRoots.value.toSet,
				solRequirements.value
			),
			solSolution := false,
			sol := {
				??
			}
		)
}

