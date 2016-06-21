
// toy Sol file ;)


lazy val root = (project in file("."))
	.enablePlugins(ColPlugin)
	.settings(
		colModules ++= Seq(
			(colModule in tool).value,
			(colModule in graphical).value,
			(colModule in command).value
		)
	)

lazy val tool = (project in file("tool/"))
	.enablePlugins(ColPlugin)

lazy val shared = (project in file("shared/"))
	.enablePlugins(ColPlugin)

lazy val tinywindow = (project in file("tinywindow/"))
	// https://github.com/ziacko/TinyWindow/
	.enablePlugins(ColPlugin)
	.settings(
		colRoots += file("tinywindow/src/")
	)

lazy val graphical = (project in file("graphical/"))
	.enablePlugins(ColPlugin)
	.settings(
		colLinked ++= Seq(
			(colModule in shared).value,
			(colModule in tinywindow).value
		)
	)

lazy val command = (project in file("command/"))
	.enablePlugins(ColPlugin)
	.settings(
		colLinked ++= Seq(
			(colModule in shared).value
		)
	)
