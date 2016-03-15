enablePlugins(ScalaJSPlugin)

name := "Scala.js Tutorial"

scalaVersion := "2.10.6" // or any other Scala version >= 2.10.2

libraryDependencies += "com.lihaoyi" %%% "fastparse" % "0.3.7"
libraryDependencies += "com.lihaoyi" %%% "utest" % "0.3.1" % "test"
testFrameworks += new TestFramework("utest.runner.Framework")
