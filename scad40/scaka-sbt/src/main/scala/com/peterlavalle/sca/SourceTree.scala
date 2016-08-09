package com.peterlavalle.sca

import java.io.{File, FileOutputStream, InputStream}
import java.net.URL

object SourceTree {

	def of(from: File): TSource =
		new TSource {
			override val root: File = from
			override val contents: Iterable[String] =
				root.streamFileNames
		}

	def GitHub(cache: File, open: URL => File = _.toTempFile) = new {


		def Archive(user: String, repo: String, version: String): TSource =
			new TSource {
				override val root: File = cache / s"$user-$repo-$version"

				val prefix: String = s"$repo-$version/"

				override val contents: Iterable[String] =
					open(new URL(s"https://github.com/$user/$repo/archive/${version}.zip")).toZipInputStream.files
						.filter {
							case (name: String, data: InputStream) =>

								name.startsWith(prefix)
						}
						.map {
							case (name: String, data: InputStream) =>

								requyre(name.startsWith(prefix))

								val reName = name.substring(prefix.length)

								val file: File = root / reName
								file.ParentDirs
								(new FileOutputStream(file) << data).close()

								reName
						}
			}

		def Release(user: String, repo: String, version: String): TSource =
			new TSource {
				override val root: File = cache / s"$user-$repo-$version"

				val prefix: String = s"$repo-$version/"

				override val contents: Iterable[String] =
					open(new URL(s"https://github.com/$user/$repo/releases/download/v$version/$repo-${version}.tar.xz"))
						.toTarXZStream
						.filter {
							case (name: String, data: InputStream) =>
								name.startsWith(prefix)
						}
						.map {
							case (name: String, data: InputStream) =>

								requyre(name.startsWith(prefix))

								val reName = name.substring(prefix.length)

								val file: File = root / reName
								file.ParentDirs
								(new FileOutputStream(file) << data).close()

								reName
						}
			}
	}

	trait TSource {
		val root: File
		val contents: Iterable[String]

		def files: Stream[File] =
			contents.toStream.map(root / _)

		def Filtered(pattern: String): TSource =
			Filtered(_ matches pattern)

		def Filtered(lambda: String => Boolean): TSource = {
			val base = this
			new TSource {
				override val root: File = base.root
				override val contents: Iterable[String] =
					base.contents.toStream.filter(lambda)
			}
		}

		def SubFolder(path: String): TSource = {
			val base = this
			new TSource {
				requyre(path.matches("((\\w+\\.)*\\w+/)+"))
				override val root: File = base.root / path
				override val contents: Iterable[String] =
					base.contents.filter(_.startsWith(path)).map(_.substring(path.length))
			}
		}
	}
}
