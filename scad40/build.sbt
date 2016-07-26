import sbt.Keys._

lazy val commonSettings =
	Seq(
		organization := "com.peterlavalle",
		version := "0.0.4-SNAPSHOT",
		scalaVersion := "2.10.6",

		javacOptions ++= Seq("-source", "1.7"),//, "-target", "1.7"),

		libraryDependencies ++= Seq(
			"junit" % "junit" % "4.12" % Test,
			"org.easymock" % "easymock" % "3.4" % Test,

			"com.novocode" % "junit-interface" % "0.11" % Test
				exclude("junit", "junit-dep")
		),

		publishTo := Some(
			Resolver.file(
				"Dropbox",
				new File(Path.userHome.absolutePath + (version.value match {
					case tag if tag matches "\\d+(\\.\\d+)+\\a*\\-RELEASE" =>
						"/Dropbox/Public/release"
					case tag if tag matches "\\d+(\\.\\d+)+\\a*\\-SNAPSHOT" =>
						"/Dropbox/Public/staging"
				}))
			)
		)
	)


lazy val root = (project in file("."))
	.settings(commonSettings: _*)
	.settings(
		name := "scad40"
	)
	.aggregate(
		scaUtil,
		//		scad40Lib,
		//		scad40Sbt,
		//		scad40App,
		scakaSbt,
		zopfli
	)

lazy val zopfli =
	(project in file("zopfli"))
		.settings(commonSettings: _*)

lazy val scaUtil =
	(project in file("sca-util"))
		.settings(commonSettings: _*)

/*
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
		.settings(
			name := "scad40app"
		)
		.dependsOn(scad40Lib)
*/

lazy val scakaSbt =
	(project in file("scaka-sbt"))
		.settings(commonSettings: _*)
		.settings(
			sbtPlugin := true,
			libraryDependencies ++= Seq(
				"org.apache.commons" % "commons-compress" % "1.12",
				"org.tukaani" % "xz" % "1.5"
			)

		)
		//.enablePlugins(SamonPlugin)
		.dependsOn(scaUtil)
		.dependsOn(zopfli)
