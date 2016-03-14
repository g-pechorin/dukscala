package peterlavalle

import java.io.File

import scala.language.implicitConversions

package object scaka {

	sealed trait TWrappedString {
		val string: String

		def #!(pattern: String): Stream[String] =
			string.split(pattern, 2) match {
				case Array(done: String) =>
					Stream(done)
				case Array(head: String, tail: String) =>
					head #:: (tail #! pattern)
			}
	}

	implicit def wrap(value: String): TWrappedString =
		new TWrappedString {
			override val string: String = value
		}

	sealed trait TWrappedFile {
		val file: File

		def /(goal: File): String = {

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

		def **(pattern: String) = {
			def recu(todo: List[String], done: Set[String]): Set[String] =
				todo match {
					case Nil => done
					case name :: tail =>
						recu(
							new File(file, name).list() match {
								case null => tail
								case list => list.foldRight(tail)(name + "/" + _ :: _)
							},
							if (name.matches(pattern)) done + name else done
						)
				}

			recu(file.list() match {
				case null => Nil
				case list => list.toList
			}, Set())
		}
	}

	implicit def wrap(value: File): TWrappedFile =
		new TWrappedFile {
			override val file: File = value
		}

	def requyre(requirement: Boolean) {
		if (!requirement) {
			val illegalArgumentException: IllegalArgumentException = new scala.IllegalArgumentException("requirement failed")
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
