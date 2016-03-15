package peterlavalle.scaka

import java.io.{FileWriter, File}

import sbt.Keys._
import sbt.plugins.JvmPlugin
import sbt.{AutoPlugin, Plugins, SettingKey, TaskKey}

object ScakaPlugin extends AutoPlugin {

	override val requires: Plugins = JvmPlugin


	object autoImport {
		lazy val scakaRoot =
			SettingKey[Boolean](
				"scakaRoot",
				"Is this a `root` Scaka project"
			)

		lazy val scakaList =
			TaskKey[Rollo.ScList](
				"scakaList",
				"list of all native sources"
			)

		lazy val scakaCMakeFile =
			TaskKey[File](
				"scakaCMakeFile",
				"write a CMakeList file"
			)
	}

	import autoImport._


	override lazy val projectSettings =
		Seq(
			scakaRoot := false,
			scakaList := {
				Rollo(
					name.value,
					baseDirectory.value.getAbsoluteFile
				)
			},
			scakaCMakeFile := {
				val cmakeFile = {
					val targetFile: File = target.value
					requyre(targetFile.exists() || targetFile.mkdirs())
					targetFile / "CMakeLists.txt"
				}

				val cmakeWriter = new FileWriter(cmakeFile)

				Frollo(
					cmakeWriter,
					scakaList.value
				)

				cmakeWriter.close()

				cmakeFile
			}

			// TODO ; watch everything from scakaList
		)
}
