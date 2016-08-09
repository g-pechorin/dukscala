package com.peterlavalle

import java.io._
import java.net.URL
import java.util.zip._

import org.apache.commons.compress.archivers.tar.TarArchiveEntry
import org.apache.commons.compress.archivers.{ArchiveInputStream, ArchiveStreamFactory}
import org.apache.commons.compress.compressors.{CompressorInputStream, CompressorStreamFactory}

import scala.io.Source
import language.implicitConversions
import scala.collection.immutable.Stream.Empty

package object sca {

	private val compressorFactory = new CompressorStreamFactory()
	private val archiveFactory = new ArchiveStreamFactory()

	sealed trait TWrappedAnyRef {
		val anyRef: AnyRef

		def HexString: String = {

			val l: Long = anyRef.hashCode() | 0L
			val r: Long = anyRef.toString.hashCode() | 0L

			(java.lang.Long.toHexString(l).padTo(16, '0') + java.lang.Long.toHexString(r).padTo(16, '0').reverse)
				.toUpperCase
		}


		def HexString(splits: Int*): String = {
			def recu(todo: List[Int], togo: String): String =
				todo match {
					case Nil => togo

					case next :: tail if next > 0 =>
						requyre(next < togo.length)

						togo.splitAt(next) match {
							case (done, left) =>
								done + "-" + recu(tail, left)
						}

					case txen :: tail if txen < 0 =>
						val next = -txen
						requyre(next < togo.length)

						togo.reverse.splitAt(next) match {
							case (tfel, enod) =>
								enod.reverse + "-" + recu(tail, tfel.reverse)
						}
				}
			recu(splits.toList, HexString)
		}
	}

	implicit def wrapAnyRef(value: AnyRef): TWrappedAnyRef =
		new TWrappedAnyRef {
			override val anyRef: AnyRef = value
		}

	sealed trait TWrappedFile {
		val file: File

		def streamFiles =
			file.listFiles() match {
				case null =>
					Stream.Empty
				case some =>
					some.toStream.map(_.getAbsoluteFile)
			}

		def streamFileNames: Stream[String] =
			file.listFiles() match {
				case null => Empty
				case list: Array[File] =>
					list.toStream.map(_.getAbsoluteFile).flatMap {
						case folder: File if folder.isDirectory =>
							requyre(folder.isDirectory)

							folder.streamFileNames.map {
								case sub: String =>
									folder.getName + "/" + sub
							}

						case file: File if file.isFile =>
							requyre(!file.isDirectory)
							val name: String = file.getName
							Stream(name)

					}
			}

		def deleteAll(): Unit = {
			if (file.exists()) {
				if (file.isDirectory)
					file.listFiles().foreach(_.deleteAll())

				requyre(file.delete())
			}
		}

		def toTarXZStream: Stream[(String, InputStream)] = {


			val compressorInputStream: CompressorInputStream =
				compressorFactory.createCompressorInputStream("xz", new FileInputStream(file).toByteArrayInputStream)

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
		}

		def toZipInputStream: ZipInputStream = {
			new ZipInputStream(new FileInputStream(file))
		}

		def mkParent =
			file.getAbsoluteFile.getParentFile match {
				case parentFile: File =>
					requyre(parentFile.exists() || parentFile.mkdirs())
			}

		def findParent(checker: (File => Boolean)) = {
			def recu(self: File): Option[File] =
				if (checker(self))
					Some(self)
				else {
					val next: File = self.getParentFile
					if (null == next || next == self)
						None
					else
						recu(next)
				}

			recu(file.getAbsoluteFile)
		}

		class OverWriter extends StringWriter {
			def closeFile: File = {
				this.close()
				file
			}

			override def append(csq: CharSequence): OverWriter = super.append(csq).asInstanceOf[OverWriter]

			override def flush(): Unit = {
				super.flush()
				mkParent
				if (!file.exists() || Source.fromFile(file).mkString != toString) {
					new FileWriter(file)
						.append(toString)
						.close()
				}
			}

			override def close(): Unit = {
				flush()
				super.close()
			}

			override def finalize(): Unit = {
				flush()
				super.finalize()
			}
		}

		def overWriter = new OverWriter

		def /(goal: File): String =
			if (file.getAbsoluteFile != file)
				file.getAbsoluteFile / goal
			else if (file.isFile)
				file.getParentFile / goal
			else {

				def chew(lS: Stream[String], rS: Stream[String]): Stream[String] =
					(lS, rS) match {
						case ((lHead #:: lTail), (rHead #:: rTail)) if lHead == rHead =>
							chew(lTail, rTail)

						case ((lHead #:: lTail), tail) =>
							".." #:: chew(lTail, tail)

						case (Stream.Empty, tail) =>
							tail
					}

				chew(
					file.getAbsolutePath.replace("\\", "/") #! "/",
					goal.getAbsolutePath.replace("\\", "/") #! "/"
				).reduce(_ + "/" + _)
			}

		def /(path: String) = {
			new File(file.getAbsoluteFile, path).getAbsoluteFile
		}

		def <<(stream: InputStream) =
			(new FileOutputStream({
				requyre(null != file && file.getParentFile != file)
				requyre(file.getParentFile.exists() || file.getParentFile.mkdirs())
				file
			}) << stream).close()

		def **(pattern: String) = {
			def recu(todo: List[String], done: Set[String]): Seq[String] =
				todo match {
					case Nil => done.toSeq.sorted
					case name :: tail =>
						recu(
							new File(file, name).list() match {
								case null => tail
								case list => list.foldRight(tail)(name + "/" + _ :: _)
							},
							if (name.matches(pattern))
								done + name
							else
								done
						)
				}

			recu(file.list() match {
				case null => Nil
				case list => list.toList
			}, Set())
		}

		def ***(pattern: String): Stream[(String, File)] = {

			def recu(todo: List[String]): Stream[(String, File)] =
				todo match {
					case Nil => Stream.Empty

					case thisPath :: tail =>
						val thisFile = new File(file, thisPath)

						thisFile.list() match {
							case null if thisPath.matches(pattern) =>
								(thisPath -> thisFile) #:: recu(tail)

							case null =>
								recu(tail)

							case list: Array[String] =>
								recu(tail ++ list.map(thisPath + "/" + _))
						}
				}

			recu(file.list() match {
				case null => Nil
				case list => list.toList
			})
		}

		def AbsoluteParent = {
			val parent = file.getAbsoluteFile.getParentFile.getAbsoluteFile
			requyre(parent.isDirectory || parent.mkdirs())
			parent
		}


		def ParentDirs = {
			val parent = AbsoluteParent
			requyre(parent.exists() || parent.mkdirs())
		}

		def AbsolutePath = {
			def recu(path: String): String =
				path match {
					case rReduce(l, r) => recu(s"$l/$r")
					case _ => path
				}

			recu(file.getAbsolutePath.replace("\\", "/"))
		}
	}

	val rReduce = "^(.+?)/[^/]+/\\.\\./(.*)$".r

	implicit def wrapFile(value: File): TWrappedFile =
		new TWrappedFile {
			override val file: File = value.getAbsoluteFile
		}

	sealed trait TWrappedURL {
		val url: URL

		def toTempFile: File = {
			val tempFile = File.createTempFile(url.toString.replaceAll("\\W", "_"), ".url")

			(new FileOutputStream(tempFile) << url.openStream()).close()
			requyre(tempFile.exists())
			tempFile
		}
	}

	implicit def wrapURL(value: URL): TWrappedURL =
		new TWrappedURL {
			override val url: URL = value
		}

	sealed trait TWrappedZipInputStream {
		val stream: ZipInputStream

		def files: Stream[(String, InputStream)] =

			stream.getNextEntry match {
				case null =>
					stream.close()
					Stream.Empty

				case next if next.isDirectory =>
					files

				case file =>
					val buffer = Array.ofDim[Byte](file.getSize.toInt)

					def recu(offset: Int): Stream[(String, InputStream)] =
						stream.read(buffer, offset, buffer.length - offset) match {
							case 0 =>
								stream.closeEntry()
								(file.getName -> new ByteArrayInputStream(buffer)) #:: files
							case read =>
								recu(offset + read)
						}

					recu(0)
			}
	}

	implicit def wrapZipInputStream(value: ZipInputStream): TWrappedZipInputStream =
		new TWrappedZipInputStream {
			override val stream = value
		}

	sealed trait TWrappedInputStream[I <: InputStream] {
		val stream: I

		def toByteArrayInputStream =
			new ByteArrayInputStream(
				(new ByteArrayOutputStream() << stream).toByteArray
			)

		def Inflate = new InflaterInputStream(stream)

		def Deflate = new DeflaterInputStream(stream, new Deflater(Deflater.BEST_COMPRESSION))

		def toByteStream: Stream[Byte] = {
			val buffer = Array.ofDim[Byte](16)

			stream.read(buffer) match {
				case -1 =>
					stream.close()
					Stream.Empty

				case data =>
					(0 until data).toStream.map(buffer) ++ toByteStream
			}
		}

		def toUByteStream: Stream[Short] =
			stream.read() match {
				case -1 =>
					stream.close()
					Stream.Empty

				case next: Int =>
					requyre(0 <= next && next <= 255)
					next.toShort match {
						case byte =>
							requyre(0 <= byte && byte <= 255)
							byte #:: toUByteStream
					}
			}
	}

	implicit def wrapInputStream[I <: InputStream](value: I): TWrappedInputStream[I] =
		new TWrappedInputStream[I] {
			override val stream = value
		}

	sealed trait TWrappedOutputStream[O <: OutputStream] {
		val stream: O

		def <<(input: InputStream): O = {
			val buffer = Array.ofDim[Byte](128)

			def recu(len: Int): O = {
				len match {
					case -1 => stream
					case count =>
						stream.write(buffer, 0, len)
						recu(input.read(buffer))
				}
			}
			recu(input.read(buffer))
		}
	}

	implicit def wrapOutputStream[O <: OutputStream](value: O): TWrappedOutputStream[O] =
		new TWrappedOutputStream[O] {
			override val stream: O = value
		}

	sealed trait TWrappedWriter[W <: Writer] {
		val writer: W

		def mappend[T](things: Iterable[T])(lambda: (T => String)): W =
			things.foldLeft(writer)((l, t) => l.append(lambda(t)).asInstanceOf[W])

		def mappend[T](things: Iterator[T])(lambda: (T => String)): W =
			things.foldLeft(writer)((l, t) => l.append(lambda(t)).asInstanceOf[W])
	}

	implicit def wrapWriter[W <: Writer](value: W): TWrappedWriter[W] =
		new TWrappedWriter[W] {
			val writer: W = value
		}

	sealed trait TWrappedString {
		val string: String

		def splyt(regex: String, limit: Int): List[String] =
			if (limit > 0) {
				string.split(regex, limit).toList
			} else {
				requyre(0 != limit)

				// TODO ; implement a splyt that can handle these cases
				requyre(regex.matches("(\\w|\\\\.|/)"))

				string.reverse.split(regex, -limit).toList.reverse.map(_.reverse)
			}

		def #!(pattern: String): Stream[String] =
			string.split(pattern, 2) match {
				case Array(done: String) =>
					Stream(done)
				case Array(head: String, tail: String) =>
					head #:: (tail #! pattern)
			}
	}

	implicit def wrapString(value: String): TWrappedString =
		new TWrappedString {
			override val string: String = value
		}


	def requyre(requirement: Boolean) {
		if (!requirement) {
			val illegalArgumentException: IllegalArgumentException = new scala.IllegalArgumentException("requirement failed")
			illegalArgumentException.setStackTrace(illegalArgumentException.getStackTrace.tail)
			throw illegalArgumentException
		}
	}

	def requyre(requirement: Boolean, message: String) {
		if (!requirement) {
			val illegalArgumentException: IllegalArgumentException = new scala.IllegalArgumentException(message)
			illegalArgumentException.setStackTrace(illegalArgumentException.getStackTrace.tail)
			throw illegalArgumentException
		}
	}

	def ?? = {
		val notImplementedError: NotImplementedError = new NotImplementedError()
		notImplementedError.setStackTrace(notImplementedError.getStackTrace.tail)
		throw notImplementedError
	}
}
