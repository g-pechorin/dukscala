package com.peterlavalle.sca

import java.io.{File, FileInputStream, FileOutputStream}
import java.util.zip.{ZipEntry, ZipInputStream}

import com.sun.org.apache.xpath.internal.functions.FuncStartsWith
import sbt.URL

object Col {

	sealed trait TSource {
		val home: File

		def contents: Set[String] = {
			def recu(todo: List[String]): Stream[String] =
				todo match {
					case null | Nil => Stream.Empty
					case name :: tail =>
						new File(home, name).list() match {
							case null => name #:: recu(tail)
							case list => recu(list.foldRight(tail)(name + "/" + _ :: _))
						}
				}
			home.list() match {
				case null => Set()
				case list => recu(list.toList).toSet
			}
		}
	}

	case class Remote(url: URL, prefix: String)(cache: File) extends TSource {
		requyre(null != url)
		requyre(null != prefix)

		def zipInputStream: ZipInputStream = {
			val file = new File(cache, s"_${url.toString.hashCode()}.zip")

			if (!file.exists())
				(new FileOutputStream(file) << url.openStream()).close()

			new ZipInputStream(new FileInputStream(file))
		}

		lazy val home: File = {

			val home = new File(cache, s"_${url.toString.hashCode()}.dir")

			zipInputStream.files.foreach {
				case (name, stream) if name.startsWith(prefix) =>
					(new FileOutputStream({
						val file = new File(home, name)
						requyre(file.getParentFile.mkdirs() || file.getParentFile.isDirectory)
						file
					}) << stream).close()
				case (name, _) =>
					;
			}

			new File(home, prefix)
		}
	}

	object Remote {
		def apply(url: String, path: String)(cache: File): Remote =
			Remote(new URL(url), path match {
				case _ if null != path => path

				case null if url.endsWith(".zip") =>
					def deZipUrl(str: String) = str.substring(str.lastIndexOf('/') + 1, str.lastIndexOf('.')) + "/"

					requyre(
						"0.9.7.5/" == deZipUrl("https://github.com/g-truc/glm/archive/0.9.7.5.zip"),
						s"""
							 |expected `${"0.9.7.5/"}`
							 |> actual `${deZipUrl("https://github.com/g-truc/glm/archive/0.9.7.5.zip")}`
						""".stripMargin.trim
					)

					deZipUrl(url)
			})(cache)
	}

	def GitHubZip(username: String, projectname: String, revision: String, sub: String = "")(cache: File) = {
		requyre(sub.matches("(\\w+/)+"))
		Remote(
			s"https://github.com/${username}/${projectname}/archive/${revision}.zip",
			s"${projectname}-${revision}/${sub}"
		)(cache)
	}

	case class Folder(home: File) extends TSource

	case class Module
	(
		name: String,
		roots: Seq[TSource],
		linked: Seq[Module]
	) {
		def fullSet: Set[Module] =
			Set(this) ++ linked.flatMap(_.fullSet)

		def ownSourceFiles =
			roots.flatMap {
				case root =>
					root.contents.filter(_.matches(".*\\.(c|cc|cpp)"))
						.map(root.home / _)
			}

		def allSourceFiles: Set[File] =
			linked.toSet.foldLeft(ownSourceFiles)(_ ++ _.allSourceFiles).toSet

		def ownHeaderFiles =
			roots.flatMap(root => root.contents.filter(_.matches(".*\\.(h|hh|hpp)")).map(root.home / _))

		def allHeaderFiles: Set[File] =
			linked.toSet.foldLeft(ownHeaderFiles)(_ ++ _.allHeaderFiles).toSet

		def isEmpty: Boolean =
			allSourceFiles.isEmpty && allHeaderFiles.isEmpty
	}

	object Module {
		def inOrder(modules: Iterable[Module]): Stream[Module] = {
			def recu(expanded: Set[Module], emitted: Set[Module], todo: List[Module]): Stream[Module] = {
				todo match {
					case Nil =>
						Stream.Empty

					case head :: tail if expanded(head) && emitted(head) =>
						recu(expanded, emitted, tail)

					case head :: tail if expanded(head) && !emitted(head) =>
						head #:: recu(expanded, emitted + head, tail)

					case head :: _ =>
						requyre(!expanded(head))
						requyre(!emitted(head))
						recu(expanded + head, emitted, head.linked.toList ++ todo)
				}
			}

			recu(
				Set(),
				Set(),
				modules.toList.reverse
			)
		}

		def labelArtifacts(modules: Iterable[Module]): Stream[(Module, Boolean)] = {
			val used = inOrder(modules).flatMap(_.linked).toSet

			modules.toStream.map {
				case module: Module =>
					module -> !used(module)
			}
		}
	}

	trait TSolver {
		def emit(dir: File, modules: Stream[(Module, Boolean)]): Set[File]
	}

}
