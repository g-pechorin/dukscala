import com.peterlavalle.sca.Art
import sbt.Keys._
import com.peterlavalle.sca.Art._

lazy val commonSettings =
	Seq(
		name := "ArtTool Example",
		scalaVersion := "2.10.6", // or any other Scala version >= 2.10.2
		libraryDependencies += "com.lihaoyi" %%% "fastparse" % "0.3.7",
		libraryDependencies += "com.lihaoyi" %%% "utest" % "0.3.1" % "test",
		testFrameworks += new TestFramework("utest.runner.Framework")
	)

lazy val root = (project in file("."))
	.enablePlugins(ArtPlugin)
	.enablePlugins(ScalaJSPlugin)
	.settings(
		// add the art-tool to the list of tools
		artTools += Art.CMake("assimp_cmd", "https://github.com/assimp/assimp/archive/v3.2.zip"),

		// add a sweep to process things with this tool
		artSweeps +=("assimp_cmd", "((\\w+/)*\\w+)\\.(blend|fbx)", "export <@ROOT@>/$0 $1.assbin -cfull")

		// if you had some special config for blender (to flip Y and Z?) you could add separate sweeps
		// artSweeps +=("assimp_cmd", "((\\w+/)*\\w+)\\.blend", "export <@ROOT@>/$0 $1.assbin -cfull")
		// artSweeps +=("assimp_cmd", "((\\w+/)*\\w+)\\.fbx", "export <@ROOT@>/$0 $0.assbin -cfull")

	)
