
lazy val commonSettings =
	Seq(
		ginnyFresh := true
	)

lazy val root = (project in file(".root"))
	.enablePlugins(GinnyPlugin)
	.settings(commonSettings: _*)
	.aggregate(
		glfw,
		main
	)

lazy val glfw = project
	.enablePlugins(GinnyPlugin)
	.settings(commonSettings: _*)
	.settings(
		ginnyRemotes +=(
			url("https://github.com/glfw/glfw/archive/3.1.2.zip"),
			"glfw-3.1.2/",
			Seq(
				"glfw",
				"glfw-3.1.2/include/",
				"${GLFW_LIBRARIES}"
			))
	)

lazy val main = (project in file("."))
	.enablePlugins(GinnyPlugin)
	.settings(commonSettings: _*)
	.settings(
		ginnyUses += (ginnyListing in glfw).value
	)