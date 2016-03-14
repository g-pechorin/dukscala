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
		lazy val hasMain = mainSrc.contains("main.cpp")
	}

	def apply(name: String, root: File): ScList = {
		ScList(
			name, root,
			root / "src/main/scaka/" ** "(\\w+/)*\\w+\\.(c|cc|cpp|cxx)",
			root / "src/main/scaka/" ** "(\\w+/)*\\w+\\.(h|hh|hpp|hxx)",
			root / "src/" ** "(\\w+/)*\\w+\\.cmake",
			root / "src/test/scaka/" ** "(\\w+/)*\\w+\\.(c|cc|cpp|cxx|h|hh|hpp|hxx)"
		)
	}
}
