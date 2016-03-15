package peterlavalle.scaka

import java.io.File

object Rollo {

	case class ScList
	(
		name: String,
		root: File,
		mainSrc: Set[String],
		mainInc: Set[String],
		cmade: Set[String],
		testSrc: Set[String]
	) {
		lazy val hasMain = mainSrc.contains("main.cpp") || mainSrc.contains("main.c")
	}

	case class ScrapeLink(lib: String, url: String, md5: String, dir: String)

	case class Module
	(
		includes: Set[String],
		directories: Set[String],
		scList: ScList,
		links: Set[String]
	)

	def apply(name: String, root: File): ScList = {
		ScList(
			name, root,
			wrapFile(root / "src/main/scaka/") ** "(\\w+/)*\\w+\\.(c|cc|cpp|cxx)",
			root / "src/main/scaka/" ** "(\\w+/)*\\w+\\.(h|hh|hpp|hxx)",
			root / "src/" ** "(\\w+/)*\\w+\\.cmake",
			root / "src/test/scaka/" ** "(\\w+/)*\\w+\\.(c|cc|cpp|cxx|h|hh|hpp|hxx)"
		)
	}
}
