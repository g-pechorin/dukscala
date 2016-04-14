package com.peterlavalle.sca

import java.io.File


object Horse {

	type Bogey = (File /*path to downloaded CMakeList.txt*/ , Seq[File] /* paths to include */ , Seq[String] /* link symbols */ )

	case class Mouth
	(
		list: TListing,
		sets: Seq[(String, Any, String)],
		home: File
	)

	sealed trait TListing {
		val uses: Seq[TListing]

		private def chain: List[TListing] =
			(uses.flatMap(_.chain.distinct) ++ List(this)).distinct.toList

		/**
			* add_subdirectory(...)
			*/
		final def dir =
			chain.flatMap {
				case ProxyListing(bogies) => bogies.map(_._1.getAbsoluteFile)
				case _ => Nil
			}.distinct


		final def inc: List[File] =
			chain.flatMap {
				case ProxyListing(bogies) => bogies.flatMap(_._2.map(_.getAbsoluteFile))
				case compiled: ACompiled => compiled.main.map(_.root.getAbsoluteFile)
			}.distinct

		final def lib =
			chain.flatMap {
				case ProxyListing(bogies) => bogies.flatMap(_._3)
				case _ => Nil
			}.distinct

		final def src: List[File] =
			chain.flatMap {
				case ProxyListing(_) => Set()
				case compiled: ACompiled =>
					compiled.main.flatMap((sourceSet: SourceSet) => sourceSet.src)
			}.distinct.sorted
	}

	case class SourceSet(root: File, srcs: Set[String]) {
		def src = srcs.map(root / _).map(_.getAbsoluteFile)
	}

	case class ProxyListing(bogies: Seq[Bogey]) extends TListing {
		override val uses = Nil
	}

	sealed abstract class ACompiled extends TListing {
		val main: Seq[SourceSet]
	}

	case class AppListing(name: String, main: Seq[SourceSet], uses: Seq[TListing]) extends ACompiled

	case class LibListing(main: Seq[SourceSet], uses: Seq[TListing]) extends ACompiled
}
