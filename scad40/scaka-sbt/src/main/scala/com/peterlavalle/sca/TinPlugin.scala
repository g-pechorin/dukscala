package com.peterlavalle.sca

import java.io.File

import sbt.Keys._
import sbt.{AutoPlugin, SettingKey, TaskKey}

object TinPlugin extends AutoPlugin {

	override lazy val projectSettings =
		Seq(
			tinSource := Seq(),
			tinOutput := {
				target.value / "tinker-flue"
			},
			tin := {
				val root = tinOutput.value

				requyre(root.isDirectory || root.mkdirs())

				root -> Tin(
					tinOutput.value,
					tinSource.value
				)
			}
		)

	import autoImport._

	object autoImport {

		lazy val tinSource = SettingKey[Seq[Tin.TSource]](
			"tinSource", "???")

		lazy val tinOutput = SettingKey[File](
			"tinOutput", "???")


		lazy val tin = TaskKey[(File, Set[String])](
			"tin", "???")
	}
}
