package peterlavalle

import java.io.{File, InputStream, OutputStream}

import scala.language.implicitConversions

package object scaka {

	sealed trait TWrappedOutputStream {
		val stream: OutputStream

		def <<(input: InputStream): OutputStream = {
			val buffer = Array.ofDim[Byte](128)

			def recu(len: Int): OutputStream = {
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

	implicit def wrapOutputStream(value: OutputStream): TWrappedOutputStream =
		new TWrappedOutputStream {
			override val stream: OutputStream = value
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

	implicit def wrapFile(value: File): TWrappedFile =
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
