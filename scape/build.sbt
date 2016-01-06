lazy val root = (project in file(".")).
  settings(
    name := "scape",
    version := "1.0",
    scalaVersion := "2.11.7"
  )

libraryDependencies += "com.lihaoyi" %% "fastparse" % "0.3.4"
libraryDependencies += "junit" % "junit" % "4.12" % "test"

