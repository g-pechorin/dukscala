package com.peterlavalle.sca

import java.io.File

case class ScakaModule
(
	lists: File,
	forceSet: Seq[(String, Any, String)],
	includes: Seq[String],
	directories: Seq[String],
	target: ScakaModule.Target
)

object ScakaModule {

	case class Target
	(
		name: String,
		mainSrc: Seq[File],
		mainInc: Seq[File],
		links: Seq[String]
	) {
		lazy val hasMain =
			mainSrc.map(_.getName) match {
				case names =>
					names.contains("main.cpp") || names.contains("main.c")
			}
	}

	case class ScrapeLink(lib: String, url: String, md5: String, dir: String)

	def apply(name: String, root: File): Target = {
		Target(
			name,
			(root / "src/main/scaka/" *** "(\\w+/)*\\w+([\\-\\.]\\w+)*\\.(c|cc|cpp|cxx|h|hh|hpp|hxx)").map(_._2),
			Seq(root / "src/main/scaka/"),
			Seq()
		)
	}
}
