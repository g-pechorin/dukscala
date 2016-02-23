import java.io.File

// enable our plugin
enablePlugins(ScaD40Plugin)

ScaD40Plugin.scad40Stuff

libraryDependencies ++= Seq(
  "junit" % "junit" % "4.12" % Test,
  "com.novocode" % "junit-interface" % "0.11" % Test
    exclude("junit", "junit-dep")
)
