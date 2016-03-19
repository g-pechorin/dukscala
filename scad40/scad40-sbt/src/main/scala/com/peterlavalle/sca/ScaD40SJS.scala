package com.peterlavalle.sca

import sbt.Keys._
import sbt.plugins._
import sbt.{AutoPlugin, Compile, Def}

/**
	* Plugin to generate SJS stuff from ScaD40
	*/
object ScaD40SJS extends AutoPlugin {
	override def requires = ScaD40Plugin && JvmPlugin

	override lazy val projectSettings =
		Seq(
			sourceGenerators in Compile += Def.task {
				val file =
					(sourceManaged in Compile).value / "scad40-sjs/SJS.scala"

				val writer =
					file.overWriter

				ScaD40Plugin.autoImport.scad40Modules.value.foreach {
					case (module) =>
						SJS(writer, module)
				}

				writer.close()

				Seq(file)
			}.taskValue
		)

}
