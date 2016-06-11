package com.peterlavalle.sca

import java.io.File

object Sol {

	type Root = File
	type Path = String

	case class SourceRoot(root: Root) {
		requyre(root == root.getAbsoluteFile)

		def srcPaths = {
			def recu(todo: List[String]): Stream[Path] =
				todo match {
					case Nil => Stream.Empty

					case head :: tail if head.matches(".+\\.(c|cpp|h|hpp)") =>
						head #:: recu(tail)

					case head :: tail =>
						recu(
							new File(root, head).list() match {
								case null => tail
								case list => list.map(head + "/" + _).foldRight(tail)(_ :: _)
							}
						)
				}

			recu(
				root.list() match {
					case null => Nil
					case list => list.toList
				}
			)
		}

		def srcFiles = srcPaths.map(new File(root, _))
	}

	case class Requirement
	(
		module: ObjectModule,
		linked: Boolean = true,
		include: Boolean = true
	)

	case class ObjectModule(home: File, sourceRoots: Set[SourceRoot], requirements: Set[Requirement]) {
		def incPaths =
			sourceRoots.map(_.root)
	}

}
