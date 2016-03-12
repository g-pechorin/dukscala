import sbt.Keys._

lazy val commonSettings = Seq(
  organization := "com.peterlavalle",
  version := "0.0.0-SNAPSHOT",
  scalaVersion := "2.10.6"
)

//name := "scad40"

lazy val lib = (project in file("scad40.lib"))
  .enablePlugins(SamonPlugin)



javacOptions ++= Seq("-source", "1.7", "-target", "1.7")



libraryDependencies ++= Seq(
  "junit" % "junit" % "4.12" % Test,
  "org.easymock" % "easymock" % "3.4" % Test,

  "com.novocode" % "junit-interface" % "0.11" % Test
    exclude("junit", "junit-dep")
)

publishTo := Some(
  Resolver.file(
    "file", new File(Path.userHome.absolutePath + "/Dropbox/Public/posted")
  )
)
