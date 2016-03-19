import sbt.Keys._

lazy val commonSettings = Seq(
	organization := "com.peterlavalle",
	version := "0.0.0-SNAPSHOT",
	scalaVersion := "2.10.6",

	javacOptions ++= Seq("-source", "1.7", "-target", "1.7"),

	libraryDependencies ++= Seq(
		"junit" % "junit" % "4.12" % Test,
		"org.easymock" % "easymock" % "3.4" % Test,

		"com.novocode" % "junit-interface" % "0.11" % Test
			exclude("junit", "junit-dep")
	),
	
	publishTo := Some(
		Resolver.file(
			"file", new File(Path.userHome.absolutePath + "/Dropbox/Public/posted")
		)
	)
)

name := "scad40"
lazy val root = (project in file("."))
	.settings(commonSettings: _*)
	.aggregate(
		scaUtil,
		scad40Lib,
		scad40Sbt,
		scad40App,
		scakaLib
	)

lazy val scaUtil =
	(project in file("sca-util"))
		.settings(commonSettings: _*)

lazy val scad40Lib =
	(project in file("scad40-lib"))
		.settings(commonSettings: _*)
		.enablePlugins(SamonPlugin)
		.settings(
			antlr4Settings,
			antlr4GenListener in Antlr4 := false,
			antlr4GenVisitor in Antlr4 := false,
			antlr4PackageName in Antlr4 := Some("com.peterlavalle.sca")
		)
		.dependsOn(scaUtil)

lazy val scad40Sbt =
	(project in file("scad40-sbt"))
		.settings(commonSettings: _*)
		.settings(
			sbtPlugin := true
		)
		.dependsOn(scad40Lib)

lazy val scad40App =
	(project in file("scad40-app"))
		.settings(commonSettings: _*)
		.dependsOn(scad40Lib)

lazy val scakaLib =
	(project in file("scaka-lib"))
		.settings(commonSettings: _*)
		.settings(
			name := "scaka",
			sbtPlugin := true
		)
		.enablePlugins(SamonPlugin)
		.dependsOn(scaUtil)
