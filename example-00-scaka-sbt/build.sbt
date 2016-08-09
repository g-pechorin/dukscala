
// toy Sol file ;)
// ... or is it "Col" now?

import com.peterlavalle.sca.Col
import com.peterlavalle.sca._

lazy val root = (project in file("."))
	.enablePlugins(CulPlugin)
	.settings(
		culAggregate := Seq(
			(culModule in tool).value,
			(culModule in graphical).value,
			(culModule in command).value,
			(culModule in glm).value
		)
	)
	.enablePlugins(ColPlugin)
	.settings(
		colAggregate ++= Seq(
			(colModule in tool).value,
			(colModule in graphical).value,
			(colModule in command).value,
			(colModule in glm).value
		)
	)

lazy val tool = (project in file("tool/"))
	.enablePlugins(CulPlugin)
	.enablePlugins(ColPlugin)

lazy val shared = (project in file("shared/"))
	.enablePlugins(CulPlugin)
	.enablePlugins(ColPlugin)

// GetGit(github.com, g-truc/glm, 0.9.7.5)
lazy val glm = (project)
	.enablePlugins(CulPlugin)
	.settings(
		culRoots ++= Seq(
			SourceTree.GitHub(target.value)
				.Archive("g-truc", "glm", "0.9.7.5")
				.SubFolder("glm/")
		)
	)
	.enablePlugins(ColPlugin)
	.settings(
		colRoots += Col.GitHubZip("g-truc", "glm", "0.9.7.5", "glm/")(target.value)
	)

lazy val tinywindow = (project in file("tinywindow/"))
	.enablePlugins(CulPlugin)
	.settings(
		culRoots ++= Seq(
			SourceTree.GitHub(target.value)
				.Archive("ziacko", "TinyWindow", "be62f01153afd7754527b9fb48e1c6d5fc198202")
				.SubFolder("Include/")
		)
	)
	.enablePlugins(ColPlugin)
	.settings(
			colRoots += Col.GitHubZip("ziacko", "TinyWindow", "be62f01153afd7754527b9fb48e1c6d5fc198202", "Include/")(target.value)
	)

lazy val graphical = (project in file("graphical/"))
	.enablePlugins(CulPlugin)
	.settings(
		culDependencies ++= Seq(
			(culModule in shared).value,
			(culModule in tinywindow).value
		)
	)
	.enablePlugins(ColPlugin)
	.settings(
		colDependencies ++= Seq(
			(colModule in shared).value,
			(colModule in tinywindow).value
		)
	)

lazy val command = (project in file("command/"))
	.enablePlugins(CulPlugin)
	.settings(
		culDependencies ++= Seq(
			(culModule in shared).value
		)
	)
	.enablePlugins(ColPlugin)
	.settings(
		colDependencies ++= Seq(
			(colModule in shared).value
		)
	)
