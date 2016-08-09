package com.peterlavalle.sca

import java.io.File

object Cul {

	trait TSolver {
		def apply(root: File, solution: Solution): Seq[File]
	}

	case class Module
	(
		name: String,
		sources: Seq[SourceTree.TSource],
		dependencies: Seq[Module]
	) {

		lazy val transitiveDependencies = {

			def recu(seen: Set[Module], todo: List[Module]): Set[Module] =
				todo match {
					case Nil => seen

					case head :: tail if seen(head) =>
						requyre(!seen(this))
						requyre(head != this)
						recu(seen, tail)

					case head :: tail =>
						requyre(!seen(this))
						requyre(head != this)
						recu(
							seen + head,
							tail ++ head.dependencies
						)
				}

			recu(
				Set(),
				dependencies.toList
			)
		}

		def hasSources =
			allSourceFiles.exists(_.AbsolutePath.matches(".*\\.(c|cc|cpp)"))

		def allSourceFiles: Stream[File] =
			sources.toStream.flatMap(_.files)
	}

	case class Solution(name: String, targets: Set[Module]) {

		lazy val transitiveNodes: Stream[Module] = {
			def recu(seen: Set[Module], todo: List[Module]): Stream[Module] =
				todo match {
					case Nil =>
						Stream.Empty

					case head :: tail if seen(head) =>
						recu(seen, tail)

					case head :: tail =>
						head #:: recu(seen + head, tail ++ head.dependencies)
				}

			recu(
				Set(),
				targets.toList
			)
		}

		lazy val branchNodes: Stream[Module] =
			transitiveNodes.filter {
				case target =>
					(targets.flatMap(_.transitiveDependencies).toSet) contains (target)
			}

		lazy val leafNodes: Stream[Module] =
			transitiveNodes filterNot branchNodes.toSet
	}

}
