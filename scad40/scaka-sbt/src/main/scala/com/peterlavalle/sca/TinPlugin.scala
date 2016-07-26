package com.peterlavalle.sca

import java.io.File

import sbt.Keys._
import sbt.{AutoPlugin, SettingKey, TaskKey}

object TinPlugin extends AutoPlugin {

	object autoImport {

		lazy val tinSource = SettingKey[Seq[Tin.TSource]](
			"tinSource", "???")

		lazy val tinOutput = SettingKey[File](
			"tinOutput", "???")

		lazy val tinChains = SettingKey[Iterable[Tin.TZLibChain]](
			"tinChains", "???")

		lazy val tin = TaskKey[(File, Set[String])](
			"tin", "???")
	}

	import autoImport._

	override lazy val projectSettings =
		Seq(
			tinSource := Seq(),
			tinOutput := {
				target.value / "tinker-flue"
			},
			tinChains := {
				Tin.defaultZLibChains
			},
			tin := {
				val root = tinOutput.value

				requyre(root.isDirectory || root.mkdirs())

				root -> Tin(
					tinChains.value,
					tinOutput.value,
					tinSource.value
				)
			}
		)
}
