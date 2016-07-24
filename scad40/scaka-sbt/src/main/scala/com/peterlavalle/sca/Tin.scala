package com.peterlavalle.sca

import java.io.{File, FileInputStream, FileWriter, InputStream}
import java.util.zip.Deflater

import scala.collection.immutable.Stream.Empty
import scala.io.Source

object Tin {
	type OpenInputStream = () => InputStream
	type SourceRecord = (String, Long, OpenInputStream)

	trait TSource {
		/**
			* @return a stream which can be used to read all-the-things coming from this flue/whatnot
			*/
		def contents: Stream[SourceRecord]
	}

	case class Flue(root: File, pattern: String) extends TSource {
		override def contents: Stream[SourceRecord] =
			root.list() match {
				case null =>
					Stream.Empty
				case contents: Array[String] =>
					def recu(todo: Stream[String]): Stream[SourceRecord] =
						todo.flatMap {
							case path: String =>
								val file = new File(root, path)
								requyre(file.exists())

								if (file.isDirectory)
									recu(
										file.list().toList.sorted.toStream.map(path + "/" + _)
									)
								else if (!path.matches(pattern))
									Empty
								else
									Stream((path, file.lastModified(), () => new FileInputStream(file)))
						}

					recu(
						contents.toList.sorted.toStream
					)
			}
	}

	def apply(output: File, flues: Iterable[TSource]): Set[String] =
		flues.foldLeft(Set[String]()) {
			case (done: Set[String], next: TSource) =>

				next.contents.foldLeft(done) {
					case (done: Set[String], (path: String, _, _)) if done.contains(path.replaceAll("[^\\w]+", "_")) =>
						// skip output names we've already seen
						done

					case (done: Set[String], (path: String, date: Long, data: OpenInputStream)) =>

						val name: String = path.replaceAll("[^\\w]+", "_")

						// only add
						if (done.contains(name)) {
							done
						} else {
							val file = new File(output, path + ".h").getAbsoluteFile
							file.AbsoluteParent

							if (file.lastModified() <= date) {

								val originalLength = {
									val length = data().toByteStream.length

									requyre(length == data().toUByteStream.length)

									length
								}

								val compressed = data().Deflate.toUByteStream

								new FileWriter(file)
									.append(
										s"""
											 |// compressed to ${(compressed.length * 100) / originalLength}% of its original size
											 |// this is all just a big literal char[]
											 |TIN_DEFLATE_BEGIN(/*unique name of this file*/"$name", /*path to source*/"$path", /*original source length*/$originalLength, /*compressed array length*/${compressed.length})
											 |
									""".stripMargin.trim + "\n")
									.mappend(compressed.grouped(314))(_.map(d => s"'\\x${Integer.toHexString(d)}', ").foldLeft("\t")(_ + _) + "\n")
									.append(
										s"""
											 |TIN_DEFLATE_CLOSE("$name", "$path", $originalLength, ${compressed.length})
									""".stripMargin.trim)
									.close()
							}
							done + name
						}
				}
		}
}
