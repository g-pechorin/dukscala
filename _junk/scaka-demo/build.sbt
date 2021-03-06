
lazy val commonSettings = Seq(

	// set(BUILD_SHARED_LIBS OFF CACHE BOOL "Don't use shared libs - ever" FORCE)
	scakaCMakeForce += (
		"BUILD_SHARED_LIBS",
		false,
		"Don't use shared libs - ever"
	),

	// set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "Don't use shared vcrt - ever" FORCE)
	scakaCMakeForce += (
		"USE_MSVC_RUNTIME_LIBRARY_DLL",
		false,
		"Don't use shared vcrt - it saves a few kB of file size but causes problems that only show up on user machines"
	),

	scakaCMakeLibs += (
		"https://github.com/glfw/glfw/archive/3.1.2.zip",
		"glfw-3.1.2/",
		Set(
			"glfw",
			"include/",
			"${GLFW_LIBRARIES}"
		)
	)
)

lazy val root = (project in file("."))
	.enablePlugins(ScakaPlugin)
	.settings(commonSettings: _*)
	.aggregate(
		moduleShared,
		moduleDroidVR,
		moduleDesktop,
		moduleEmpty
	)

lazy val moduleShared =
	(project in file("module-shared"))
		.enablePlugins(ScakaPlugin)
		.settings(commonSettings: _*)

lazy val moduleDroidVR =
	(project in file("module-DroidVR"))
		.enablePlugins(ScakaPlugin)
		.settings(commonSettings: _*)
		.dependsOn(moduleShared)

lazy val moduleDesktop =
	(project in file("module-Desktop"))
		.enablePlugins(ScakaPlugin)
		.settings(commonSettings: _*)
		.dependsOn(moduleShared)

lazy val moduleEmpty =
	(project in file("module-Empty"))