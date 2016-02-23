lazy val root = Project("plugins", file(".")) dependsOn(samon)

lazy val samon = file("../scad40").getAbsoluteFile.toURI
