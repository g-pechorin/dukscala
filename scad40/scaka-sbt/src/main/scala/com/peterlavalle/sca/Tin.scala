package com.peterlavalle.sca

import java.io.{File, FileInputStream, InputStream}
import java.util.zip.Deflater

import ru.eustas.zopfli.{Options, Zopfli}
import ru.eustas.zopfli.Options.OutputFormat

import scala.collection.immutable.Stream.Empty
import scala.collection.mutable

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

	trait TZLibChain {
		val name: String

		def apply(inputStream: InputStream): Stream[UByte]
	}

	val defaultZLibChains: Iterable[TZLibChain] = {

		val zopfli: Seq[ZopfliZLibChain] =
			Seq(64, 256, 1024, 4 * 1024).flatMap {
				case masterBlockSize: Int =>
					Options.BlockSplitting.values().toSeq.flatMap {
						case split =>
							Seq(3, 14, 19).map {
								case numIterations: Int =>
									ZopfliZLibChain(masterBlockSize, numIterations)
							}
					}
			}

		Seq(
			Deflater.NO_COMPRESSION, Deflater.BEST_COMPRESSION, Deflater.BEST_SPEED, Deflater.HUFFMAN_ONLY
		).map(DeflateZLibChain) ++ zopfli
	}

	case class DeflateZLibChain(i: Int) extends TZLibChain {
		override val name: String = toString

		override def apply(inputStream: InputStream): Stream[UByte] =
			inputStream.Deflate(new Deflater(i)).toUByteStream
	}

	case class ZopfliZLibChain(masterBlockSize: Int, numIterations: Int, blockSplitting: Options.BlockSplitting = Options.BlockSplitting.FIRST) extends TZLibChain {
		override val name: String = toString

		override def apply(inputStream: InputStream): Stream[UByte] =
			new Zopfli(masterBlockSize)
				.compress(
					new Options(OutputFormat.ZLIB, blockSplitting, numIterations),
					inputStream.toByteStream.toArray
				)
				.toByteArrayInputStream.toUByteStream
	}

	def apply(chains: Iterable[TZLibChain], output: File, flues: Iterable[TSource]): Set[String] =
		flues.foldLeft(Set[String]()) {
			case (done: Set[String], next: TSource) =>

				next.contents.foldLeft(done) {
					case (done: Set[String], (path: String, _, _)) if done.contains(path.replaceAll("[^\\w]+", "_")) =>
						// skip output names we've already seen
						done

					case (done: Set[String], (path: String, date: Long, data: OpenInputStream)) =>

						// only add
						if (done.contains(path)) {
							done
						} else {


							val originalLength = {
								val length = data().toByteStream.length

								requyre(length == data().toUByteStream.length)

								length
							}
							System.out.println(s"Beginning ${path.replace('\\', '/')} ...")

							val symbol: String = path.replaceAll("[\\W_]+", "_")

							chains
								.map {
									case tMethod: TZLibChain =>

										System.out.println(s"Compressing file ${path.replace('\\', '/')} with ${tMethod.name}")
										val compressed = tMethod(data())

										new File(output, path + "." + tMethod.name + ".h").getAbsoluteFile
											.overWriter
											.append(
												s"""
													 |// path = $path
													 |// method: ${tMethod.name.reverse.padTo(chains.map(_.name.length).max, ' ').reverse}
													 |// originalLength = $originalLength
													 |// compressed.length = ${compressed.length}
													 |// ratio = ${((compressed.length * 100) / originalLength).toString.reverse.padTo(3, ' ').reverse}% of its original size
													 |// this is all just a big literal char[]
													 |TIN_DEFLATE_BEGIN(/*semi-unique name of this file*/$symbol, /*path to source*/"$path", /*original source length*/$originalLength, /*compressed array length*/${compressed.length})
													 |
												""".stripMargin.trim + "\n")
											.mappend(compressed.grouped(314))(_.map(d => s"'\\x${d.toHexString}', ").foldLeft("\t")(_ + _) + "\n")
											.append(
												s"""
													 |TIN_DEFLATE_CLOSE($symbol, "$path", $originalLength, ${compressed.length})
												""".stripMargin.trim)
											.closeFile
								}
								.reduce((l, r) => if (l.length() > r.length()) r else l) match {
								case best: File =>
									(new File(output, path + ".tinflue")
										.getAbsoluteFile
										.overWriter << new FileInputStream(best))
										.close()
							}

							done + path
						}
				}
		}
}
