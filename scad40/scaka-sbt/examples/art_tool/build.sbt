import com.peterlavalle.sca.Art
import sbt.Keys._
import com.peterlavalle.sca.Art._

lazy val commonSettings =
	Seq(
	)

lazy val root = (project in file("."))
	.enablePlugins(ArtPlugin)
	.settings(
		// add the art-tool to the list of tools
		artTools += Art.CMake("assimp_cmd", "https://github.com/assimp/assimp/archive/v3.2.zip"),

		// add a sweep to process things with this tool
		artSweeps +=("assimp_cmd", "((\\w+/)*\\w+)\\.(blend|fbx)", "export <@ROOT@>/$0 $1.assbin -cfull")

		// if you had some special config for blender (to flip axises?) you could add separate sweeps
		// artSweeps +=("assimp_cmd", "((\\w+/)*\\w+)\\.blend", "export <@ROOT@>/$0 $1.assbin -cfull")
		// artSweeps +=("assimp_cmd", "((\\w+/)*\\w+)\\.fbx", "export <@ROOT@>/$0 $0.assbin -cfull")

	)
