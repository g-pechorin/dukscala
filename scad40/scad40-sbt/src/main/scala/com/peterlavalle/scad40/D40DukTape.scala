package com.peterlavalle.scad40

import java.io.FileWriter

import com.peterlavalle.D40
import sbt.Keys._
import sbt.plugins._
import sbt.{AutoPlugin, _}

/**
	* Plugin to generate DukTape stuff
	*/
object D40DukTape extends AutoPlugin {
	override def requires = ScaD40Plugin && JvmPlugin

	object autoImport {
		lazy val d40Managed = SettingKey[Boolean]("d40-managed", "Should this go into a mangaged source directory?")
	}

	import autoImport._


	override lazy val projectSettings =
		Seq(
			d40Managed := false,
			sourceGenerators in Compile += Def.task {

				val root =
					if (d40Managed.value)
						(sourceManaged in Compile).value / "d40-duktape"
					else
						target.value / "gen-inc"

				val file = root / "D40.hpp"

				val writer: FileWriter = new FileWriter({
					require(file.getParentFile.exists() || file.getParentFile.mkdirs())
					file
				})

				ScaD40Plugin.autoImport.scad40Modules.value.foreach {
					case (module) =>
						D40(writer, module)
				}

				writer.close()

				if (d40Managed.value) Seq(file) else Seq()
			}.taskValue
		)

}

