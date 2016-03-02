name := "scad40"
organization := "com.peterlavalle"
version := "1.0.0-SNAPSHOT"

scalaVersion := "2.10.6"

enablePlugins(SamonPlugin)
SamonPlugin.samonStuff


javacOptions ++= Seq("-source", "1.7", "-target", "1.7")

antlr4Settings
antlr4GenListener in Antlr4 := false
antlr4GenVisitor in Antlr4 := false
antlr4PackageName in Antlr4 := Some("peterlavalle.scad40")


libraryDependencies += "org.slf4j" % "slf4j-simple" % "1.7.13"
libraryDependencies += "org.scalatra.scalate" %% "scalate-core" % "1.7.0"


libraryDependencies ++= Seq(
  "junit" % "junit" % "4.12" % Test,
  "org.easymock" % "easymock" % "3.4" % Test,
  //"org.scalatest" % "scalatest_2.11" % "2.2.6" % Test,

  "com.novocode" % "junit-interface" % "0.11" % Test
    exclude("junit", "junit-dep")
)

publishTo := Some(
  Resolver.file(
    "file", new File(Path.userHome.absolutePath + "/Dropbox/Public/posted")
  )
)
