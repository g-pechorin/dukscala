package com.peterlavalle.sca

import java.io.File

object Rollo {

	case class ScList
	(
		name: String,
		root: File,
		mainSrc: Seq[String],
		mainInc: Seq[String],
		cmade: Seq[String],
		testSrc: Seq[String]
	) {
		lazy val hasMain =
			Set("main", "assembly").flatMap {
				case n =>
					Set("c", "cpp", "cc").map(n + "." + _)
			}.exists(mainSrc.contains)
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
			wrapFile(root / "src/main/scaka/") ** "(\\w+/)*\\w+([\\-\\.]\\w+)*\\.(c|cc|cpp|cxx)",
			root / "src/main/scaka/" ** "(\\w+/)*\\w+([\\-\\.]\\w+)*\\.(h|hh|hpp|hxx)",
			root / "src/" ** "(\\w+/)*\\w+\\.cmake",
			root / "src/test/scaka/" ** "(\\w+/)*\\w+\\.(c|cc|cpp|cxx|h|hh|hpp|hxx)"
		)
	}
}
