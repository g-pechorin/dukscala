package com.peterlavalle.sca

import java.io.File

import scala.collection.immutable.::


object Horse {

	def noDupe[E](stream: Stream[E]): Stream[E] = noDupe[E, E](stream, (i: E) => i)

	def noDupe[I, O](stream: Stream[I], mapp: I => O): Stream[O] = {

		def recu(seen: Set[I], todo: Stream[I]): Stream[O] =
			todo match {
				case Stream.Empty => Stream.Empty

				case head #:: tail if seen contains head =>
					recu(seen, tail)

				case head #:: tail =>
					mapp(head) #:: recu(seen + head, tail)
			}

		recu(Set(), stream)
	}

	type Bogey = (File /*path to downloaded CMakeList.txt*/ , Seq[File] /* paths to include */ , Seq[String] /* link symbols */ )

	case class Mouth
	(
		list: TListing,
		sets: Seq[(String, Any, String)],
		home: File
	)

	sealed trait TListing {
		val uses: Seq[TListing]

		private def chain: List[TListing] = {
			def recu(seen: Set[TListing], todo: Seq[TListing]): List[TListing] =
				todo match {
					case head :: tail if seen(head) =>
						recu(seen, tail)

					case head :: tail =>
						head :: recu(seen + head, head.uses ++ tail)

					case Nil =>
						Nil
				}

			noDupe(recu(Set(), uses).reverse.toStream).toList
		}

		/**
			* add_subdirectory(...)
			*/
		final def dir: Stream[File] = {
			def inner(todo: List[TListing], end: Stream[File]): Stream[File] =
				todo match {
					case Nil => end
					case head :: tail =>
						head match {
							case ProxyListing(_, bogies) =>
								bogies.map {
									case (root, _, _) =>
										root.getAbsoluteFile
								}.foldRight(inner(tail, Stream.Empty))(_ #:: _)

							case _ =>
								inner(tail, Stream.Empty)
						}
				}

			noDupe(inner(chain, Stream.Empty))
		}

		final def inc: Stream[File] = {
			def inner(todo: List[TListing], end: Stream[File]): Stream[File] =
				todo match {
					case Nil => end
					case head :: tail =>
						(head match {
							case ProxyListing(_, bogies) =>
								bogies.flatMap(_._2.map(_.getAbsoluteFile)).toStream

							case compiled: ACompiled =>
								compiled.main.toStream.map {
									case SourceSet(path, _) =>
										path.getAbsoluteFile
								}
						}) ++ inner(tail, Stream.Empty)
				}

			noDupe(inner(chain, Stream.Empty))
		}

		final def lib: Stream[String] = {
			def inner(todo: List[TListing], end: Stream[String]): Stream[String] =
				todo match {
					case Nil => end
					case head :: tail =>
						(head match {
							case ProxyListing(_, bogies) => bogies.flatMap(_._3).toStream
							case compiled: ACompiled => Stream.Empty
						}) ++ inner(tail, Stream.Empty)
				}

			noDupe(inner(chain, Stream.Empty))
		}

		final def src: Stream[File] = {
			def inner(todo: List[TListing], end: Stream[File]): Stream[File] =
				todo match {
					case Nil => end
					case head :: tail =>
						(head match {
							case ProxyListing(_, bogies) => Stream.Empty
							case compiled: ACompiled =>
								compiled.main.toStream.flatMap {
									case SourceSet(root: File, srcs: Set[String]) =>
										srcs.map(root / _)
								}
						}) ++ inner(tail, Stream.Empty)
				}

			inner(chain, Stream.Empty)
		}
	}

	case class SourceSet(root: File, srcs: Set[String]) {
		def src: Stream[File] = srcs.toStream.map(root / _).map(_.getAbsoluteFile)
	}

	case class ProxyListing(uses: Seq[TListing], bogies: Seq[Bogey]) extends TListing

	sealed abstract class ACompiled extends TListing {
		val main: Seq[SourceSet]
	}

	case class AppListing(name: String, main: Seq[SourceSet], uses: Seq[TListing]) extends ACompiled

	case class LibListing(main: Seq[SourceSet], uses: Seq[TListing]) extends ACompiled

}
