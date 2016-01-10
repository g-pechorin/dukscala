
lazy val root = (project in file(".")).
  settings(
    name := "scape",
    version := "1.0",
    scalaVersion := "2.11.7"
  )

javacOptions ++= Seq("-source", "1.7", "-target", "1.7")

antlr4Settings

antlr4GenListener in Antlr4 := false
antlr4GenVisitor in Antlr4 := false
antlr4PackageName in Antlr4 := Some("peterlavalle.scad40")

libraryDependencies += "org.slf4j" % "slf4j-simple" % "1.7.13"
libraryDependencies += "org.scalatra.scalate" %% "scalate-core" % "1.7.0"
libraryDependencies += "junit" % "junit" % "4.12" % "test"
