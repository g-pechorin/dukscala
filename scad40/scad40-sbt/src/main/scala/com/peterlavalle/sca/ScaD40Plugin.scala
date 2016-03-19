package com.peterlavalle.sca

import java.io.{File, FileInputStream}

import sbt.Keys._
import sbt.{AutoPlugin, Compile, SettingKey, TaskKey}

object ScaD40Plugin extends AutoPlugin {

	object autoImport {
		lazy val scad40Directory = SettingKey[File]("scad40-directory", "Setting controlling folder to scan for ScaD40 IDLs")
		lazy val scad40Modules = TaskKey[Stream[Model.Module]]("scad40-modules", "Piles of stuff for ScaD40 to do things with")
	}

	import autoImport._

	override lazy val projectSettings =
		Seq(
			//
			scad40Directory <<= (sourceDirectory in Compile) (_ / "scad40"),

			// watch for changes
			watchSources <++= sourceDirectory map {
				case path =>
					(path *** "(\\w+/)*\\w+\\.scad40").map {
						case (_, file) =>
							file
					}
			},

			//
			scad40Modules := {
				(scad40Directory.value *** "(\\w+/)*\\w+\\.scad40").map {
					case (nume, file) =>
						FromAntlr4(nume, new FileInputStream(file))
				}
			}
		)

}
