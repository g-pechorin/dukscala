package com.peterlavalle.sca

import java.io.File

object Rollo {

	case class ScList
	(
		name: String,
		root: File,
		mainSrc: Seq[File],
		mainInc: Seq[File]
	) {
		lazy val hasMain =
			mainSrc.map(_.getName) match {
				case names =>
					names.contains("main.cpp") || names.contains("main.c")
			}
	}

	case class ScrapeLink(lib: String, url: String, md5: String, dir: String)

	case class Module
	(
		forceSet: Seq[(String, Any, String)],
		includes: Seq[String],
		directories: Seq[String],
		scList: ScList,
		links: Seq[String]
	)

	def apply(name: String, root: File): ScList = {
		ScList(
			name, root,
			(root / "src/main/scaka/" *** "(\\w+/)*\\w+([\\-\\.]\\w+)*\\.(c|cc|cpp|cxx|h|hh|hpp|hxx)").map(_._2),
			Seq(root / "src/main/scaka/")
		)
	}
}
