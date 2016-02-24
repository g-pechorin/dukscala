package com.peterlavalle.scad40


import sbt._
import Keys._

object ScaD40Plugin extends AutoPlugin {

	val scad40 = config("scaD40")

	object autoImport {

		/**
			* Each "plop" is a class.name.in.Full -> some/file/to/write
			**/
		lazy val scad40Plops = SettingKey[Seq[(String, File)]]("scad40-plops", "Classes to pass the ScaD40 definitions to")

		lazy val scad40Roots = SettingKey[Seq[File]]("scad40-sources", "A list of folders to scan for ScaD40 interfaces")

		/**
			* Actual task
			*/
		lazy val scad40Generate = TaskKey[Seq[File]]("scad40-generate", "Generate all ScaD40 writers")

	}

	import autoImport._

	val scad40Stuff = inConfig(scad40)(Seq(
	))

	override lazy val projectSettings = Seq(
		scad40Roots := Seq(
			(sourceDirectory.value / "main/scad40").getAbsoluteFile
		),
		watchSources <++= sourceDirectory map {
			case path =>
				(path ** "*.scad40")
				???
		},
		scad40Generate := {
			???
		}
	)
}
