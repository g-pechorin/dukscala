package com.peterlavalle.sca

import java.io.{File, StringWriter}

object Col {

	case class Root(home: File) {
		def contents: Set[String] = {
			def recu(todo: List[String]): Stream[String] =
				todo match {
					case null | Nil => Stream.Empty
					case name :: tail =>
						new File(home, name).list() match {
							case null => name #:: recu(tail)
							case list => recu(list.foldRight(todo)(name + "/" + _ :: _))
						}
				}
			home.list() match {
				case null => Set()
				case list => recu(list.toList).toSet
			}
		}
	}

	case class Module
	(
		name: String,
		roots: Seq[Root],
		linked: Seq[Module]
	) {
		def fullSet: Set[Module] =
			Set(this) ++ linked.flatMap(_.fullSet)

		def ownSourceFiles =
			roots.flatMap(root => root.contents.filter(_.matches(".*\\.(c|cc|cpp)")).map(root.home / _))

		def allSourceFiles: Set[File] =
			linked.foldLeft(ownSourceFiles)(_ ++ _.allSourceFiles).toSet

		def ownHeaderFiles =
			roots.flatMap(root => root.contents.filter(_.matches(".*\\.(h|hh|hpp)")).map(root.home / _))

		def allHeaderFiles: Set[File] =
			linked.foldLeft(ownHeaderFiles)(_ ++ _.allHeaderFiles).toSet

		def isEmpty: Boolean =
			linked.foldLeft(allSourceFiles.isEmpty && allHeaderFiles.isEmpty)(_ && _.isEmpty)
	}

	object Module {
		def inOrder(modules: Iterable[Module]): Stream[Module] = {

			def recu(done: Set[Module], seen: Set[Module], todo: List[Module]): Stream[Module] =
				todo match {
					case Nil => Stream.Empty

					case head :: tail if done(head) && seen(head) =>
						recu(done, seen, tail)

					case head :: tail if done(head) && !seen(head) =>
						head #:: recu(done, seen + head, tail)

					case head :: _ if !done(head) =>
						requyre(!seen(head))
						recu(done + head, seen, head.linked.toList ++ todo)
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
