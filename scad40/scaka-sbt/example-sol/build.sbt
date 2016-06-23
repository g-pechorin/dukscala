
// toy Sol file ;)
// ... or is it "Col" now?

import com.peterlavalle.sca.Col

lazy val root = (project in file("."))
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
	.enablePlugins(ColPlugin)

lazy val shared = (project in file("shared/"))
	.enablePlugins(ColPlugin)

// GetGit(github.com, g-truc/glm, 0.9.7.5)
lazy val glm = (project)
	.enablePlugins(ColPlugin)
	.settings(
		colRoots += Col.GitHubZip("g-truc", "glm", "0.9.7.5", "glm/")(target.value)
	)

lazy val tinywindow = (project in file("tinywindow/"))
	.enablePlugins(ColPlugin)
	.settings(
			colRoots += Col.GitHubZip("ziacko", "TinyWindow", "be62f01153afd7754527b9fb48e1c6d5fc198202", "Include/")(target.value)
	)

lazy val graphical = (project in file("graphical/"))
	.enablePlugins(ColPlugin)
	.settings(
		colDependencies ++= Seq(
			(colModule in shared).value,
			(colModule in tinywindow).value
		)
	)

lazy val command = (project in file("command/"))
	.enablePlugins(ColPlugin)
	.settings(
		colDependencies ++= Seq(
			(colModule in shared).value
		)
	)
