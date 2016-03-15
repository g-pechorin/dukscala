
enablePlugins(ScakaPlugin)

scakaCMakeLibs += (
	"https://github.com/glfw/glfw/archive/3.1.2.zip",
	"glfw-3.1.2/",
	Set(
		"glfw",
		"include/",
		"${GLFW_LIBRARIES}"
	)
)

