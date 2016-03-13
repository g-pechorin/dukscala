package com.peterlavalle.scad40

import java.io.File

import sbt.Keys._
import sbt.{AutoPlugin, _}
import sbt.plugins._


object ScaD40Plugin extends AutoPlugin {

	// the JvmPlugin resets source generators. require it here to prevent it from erasing us
	override def requires = JvmPlugin

	object autoImport {
		lazy val scad40Directory = SettingKey[File]("scad40-directory", "Setting controlling folder to scan for ScaD40 IDLs")
	}

	import autoImport._

	override lazy val projectSettings =
		Seq(
			//
			scad40Directory <<= (sourceDirectory in Compile) (_ / "scad40"),

			// watch for changes
			watchSources <++= sourceDirectory map {
				case path =>
					(path ** "*.scad40").get
			},

			//
			sourceGenerators in Compile += Def.task {

				sys.error(
					s"TODO; Scan ${scad40Directory.value} for stuff and load it up (mostly just checking the format)"
				)

				sys.error(
					s"TODO; Magically do something to dynamically load classes or something?"
				)

			}.taskValue
		)

}
