sbtPlugin := true


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
    "file",
    new File(Path.userHome.absolutePath + "/Dropbox/Public/posted")
  )
)


////
// SBT launcehr thingieies
//
import java.io.{FileInputStream, FileOutputStream, Writer, FileWriter}
import java.util.zip.{ZipEntry, ZipOutputStream, ZipFile}

import sbt.Keys._

import scala.io.Source

lazy val sbtLauncherJar =
  settingKey[ZipFile]("Which sbt-launch.jar should I be messing with?")

sbtLauncherJar := {
  new ZipFile(file("sbt-launch.jar"))
}

lazy val emitLauncherProperties =
  taskKey[File]("Writes a `sbt.boot.properties` file")

emitLauncherProperties := {

  val properties: File = new sbt.File(target.value, "sbt.boot.properties")
  require(properties.getParentFile.exists() || properties.getParentFile.mkdirs())

  var rSection = "\\[(\\w+)\\]\\s*".r
  val rKeyTag = "(\\s+)(version|org|name|class|components)(\\s*):(\\s*)(.*)".r

  def recu(section: String, stream: Stream[String]): Stream[String] =
    (section, stream) match {
      case (_, Stream.Empty) =>
        Stream.Empty

      case (_, rSection(nextSection) #:: tail) =>
        stream.head #:: (recu(nextSection, tail))

      case ("scala", rKeyTag(indent, "version", predex, postdex, oldValue) #:: tail) =>
        s"${indent}version: ${scalaVersion.value}" #:: recu(section, tail)

      case ("app", rKeyTag(indent, tag, _, _, oldValue) #:: tail) =>
        val value: String =
          tag match {
            case "org" =>
              organization.value
            case "name" =>
              name.value
            case "version" =>
              version.value
            case "components" =>
              ""
            case "class" =>
              "com.peterlavalle.scad40.ScaD40App"
          }

        if ("" == value)
          recu(section, tail)
        else
          s"${indent}${tag}: ${value}" #:: recu(section, tail)

      case ("repositories", blank #:: tail) if blank.matches("\\s*") =>
        "  peterlavalle-dropbox: https://dl.dropboxusercontent.com/u/15094498/posted/" #:: recu("", stream)

      case (_, other #:: tail) =>
        other #:: recu(section, tail)
    }

  recu("", Source.fromInputStream(sbtLauncherJar.value.getInputStream(sbtLauncherJar.value.getEntry("sbt/sbt.boot.properties"))).getLines().toStream)
    .foldLeft(new FileWriter(properties).asInstanceOf[Writer]) {
      case (writer: Writer, line: String) =>
        writer.append(line).append("\n")
    }.close()

  streams.value.log.info(s"Updated `${properties.getPath}`")

  properties
}

lazy val createLauncherArchive =
  taskKey[File]("Copies the known SBT .jar to make our very own 8D")

lazy val ownLauncherJar =
  settingKey[File]("Where should I write my launcher?")

ownLauncherJar := new sbt.File(target.value, s"${name.value}-launcher.jar")

createLauncherArchive := {

  require(ownLauncherJar.value.getParentFile.exists() || ownLauncherJar.value.getParentFile.mkdirs())

  val zipOutputStream = new ZipOutputStream(new FileOutputStream(ownLauncherJar.value))

  import scala.collection.JavaConverters._
  import scala.collection.JavaConversions._
  sbtLauncherJar.value.entries().foreach {
    case entry =>
      val buffer = Array.ofDim[Byte](128)
      if (entry.isDirectory) {

      } else {

        val inputStream =
          if (entry.getName == "sbt/sbt.boot.properties") {
            zipOutputStream.putNextEntry(new ZipEntry(entry.getName))
            new FileInputStream(emitLauncherProperties.value)
          } else {
            zipOutputStream.putNextEntry(entry)
            sbtLauncherJar.value.getInputStream(entry)
          }

        def hammer(): Unit =
          inputStream.read(buffer) match {
            case -1 =>
              inputStream.close()
              zipOutputStream.closeEntry()
            case read =>
              zipOutputStream.write(buffer, 0, read)
              hammer()
          }

        hammer()
      }
  }
  zipOutputStream.close()


  streams.value.log.info(s"Created `${ownLauncherJar.value.getPath}`")

  ownLauncherJar.value
}