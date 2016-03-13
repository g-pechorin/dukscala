package com.peterlavalle.scad40

import java.io.FileWriter

import com.peterlavalle.SJS
import sbt.Keys._
import sbt.plugins._
import sbt.{AutoPlugin, _}

/**
	* Plugin to generate SJS stuff from ScaD40
	*/
object ScaD40SJS extends AutoPlugin {
	override def requires = ScaD40Plugin && JvmPlugin

	override lazy val projectSettings =
		Seq(
			sourceGenerators in Compile += Def.task {
				val file = (sourceManaged in Compile).value / "scad40-sjs" / "SJS.scala"

				val writer: FileWriter = new FileWriter({
					require(file.getParentFile.exists() || file.getParentFile.mkdirs())
					file
				})

				ScaD40Plugin.autoImport.scad40Modules.value.foreach {
					case (module) =>
						SJS(writer, module)
				}

				writer.close()

				Seq(file)
			}.taskValue
		)

}
