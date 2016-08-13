package com.peterlavalle.sca

import java.io._
import java.util.zip.ZipInputStream

import org.apache.commons.compress.archivers.tar.TarArchiveEntry
import org.apache.commons.compress.archivers.{ArchiveInputStream, ArchiveStreamFactory}
import org.apache.commons.compress.compressors.{CompressorInputStream, CompressorStreamFactory}
import sbt.URL

object Col {

	val compressorFactory = new CompressorStreamFactory()
	val archiveFactory = new ArchiveStreamFactory()

	def Scrape(url: String, names: String*)(cache: File): Scrape =
		new Scrape(new URL(url), names.toSet)(cache)

	def BitBucket(username: String, projectname: String, revision: String, sub: String = "")(cache: File) = {
		requyre(sub.matches("(\\w+/)*"))
		Remote(
			s"https://bitbucket.org/${username}/${projectname}/get/${revision}.zip",
			s"${username}-${projectname}-${revision}/${sub}"
		)(cache)
	}

	def GitHubZip(username: String, projectname: String, revision: String = "master", sub: String = "")(cache: File) = {
		requyre(sub.matches("(\\w+/)*"))
		Remote(
			s"https://github.com/${username}/${projectname}/archive/${revision}.zip",
			s"${projectname}-${revision}/${sub}"
		)(cache)
	}

	def GitHubRelease(username: String, projectname: String, tag: String, sub: String = "")(cache: File) = {
		requyre(sub.matches("(\\w+/)*"))
		Remote(
			s"https://codeload.github.com/${username}/${projectname}/zip/v${tag}",
			s"${projectname}-${tag}/${sub}"
		)(cache)
	}

	sealed trait TSource {
		val home: File

		def Filtered(regex: String) = {
			val real = this
			new TSource {
				override val home: File = real.home

				override def contents: Set[String] = real.contents.filter(_.matches(regex))
			}
		}

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

	trait TSolver {
		/**
			*
			* @param target  where all files should be created
			* @param modules (all modules to make, ???)
			* @return a set of all files created
			*/
		def emit(target: File, modules: Stream[Module]): Set[File]
	}

	case class Scrape(url: URL, names: Set[String])(cache: File) extends TSource {
		override lazy val home: File = {
			val home = new File(cache, s"_${url.toString.hashCode()}_${names.toString().hashCode}.dir")
			names.foreach {
				case name: String =>
					val file = new File(home, name)

					requyre(file.getParentFile.exists() || file.getParentFile.mkdirs())

					if (!file.exists())
						(new FileOutputStream(file) << url.openStream()).close()
			}
			home
		}
	}

	case class Remote(url: URL, prefix: String)(cache: File) extends TSource {
		requyre(null != url)
		requyre(null != prefix)
		requyre(cache.isDirectory || cache.mkdirs())

		lazy val home: File = {

			val home = new File(cache, s"_${url.toString.hashCode()}.dir")

			val files: Stream[(String, InputStream)] = {
				if (url.toString.endsWith(".zip")) {
					val file = new File(cache, s"_${url.toString.hashCode()}.zip")

					if (!file.exists())
						(new FileOutputStream(file) << url.openStream()).close()

					new ZipInputStream(new FileInputStream(file)).files
				} else if (url.toString.matches(".*\\.tar\\.(xz)")) {

					val file = new File(cache, s"_${url.toString.hashCode()}.tar")

					if (!file.exists())
						(new FileOutputStream(file) << url.openStream()).close()

					val toByteArrayInputStream: ByteArrayInputStream = new FileInputStream(file).toByteArrayInputStream

					val compressorInputStream: CompressorInputStream =
						compressorFactory.createCompressorInputStream("xz", toByteArrayInputStream)

					val archiveInputStream: ArchiveInputStream =
						archiveFactory.createArchiveInputStream("tar", compressorInputStream)

					def archiveStream: Stream[(String, InputStream)] =
						archiveInputStream.getNextEntry match {
							case null => Stream.Empty

							case tarEntry: TarArchiveEntry =>
								if (tarEntry.getName.endsWith("/"))
									archiveStream
								else {
									(tarEntry.getName -> new ByteArrayInputStream({
										val buffer = Array.ofDim[Byte](tarEntry.getSize.toInt)

										requyre(buffer.length == archiveInputStream.read(buffer))

										buffer
									})) #:: archiveStream
								}
						}

					archiveStream
				} else {
					??
				}
			}

			files.foreach {
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

	case class Folder(home: File) extends TSource

	case class Module
	(
		name: String,
		roots: Seq[TSource],
		linked: Seq[Module]
	) {

		lazy val artifact: Module.TArtifact = {
			val contents: Stream[String] = roots.toStream.flatMap(_.contents)

			linked.foreach {
				case lib: Module =>
					lib.artifact match {
						case Module.Static => ;
						case _ => sys.error(s"Can't link output ${lib.name} into artifact $name")
					}
			}

			contents.find(_.matches("main\\.(c|cpp)")) match {
				case None =>
					contents.find(_.matches("module\\.(c|cpp)")) match {
						case None => Module.Static
						case _ => Module.Shared
					}
				case _ =>
					contents.find(_.matches("module\\.(c|cpp)")) match {
						case None =>

							Module.Binary
						case _ => sys.error("Ambiguous artifact $name")
					}
			}
		}

		def allFolders: Stream[File] =
			(roots.toStream.map(_.home) ++ linked.flatMap(_.allFolders)).distinct

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

	object Module {

		def labelArtifacts(modules: Iterable[Module]): Stream[(Module, Boolean)] = {
			val used = inOrder(modules).flatMap(_.linked).toSet

			modules.toStream.map {
				case module: Module =>
					module -> !used(module)
			}
		}

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

		sealed trait TArtifact

		case object Static extends TArtifact

		case object Shared extends TArtifact

		case object Binary extends TArtifact

	}

}
