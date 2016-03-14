package peterlavalle.scaka

import junit.framework.TestCase
import org.junit.Assert._
import peterlavalle.Home

class RolloTest extends TestCase {
	def testPathie(): Unit = {
		assertEquals(
			"../../lend/file",
			(Home.projectFolder / "home/bar/gone/") / (Home.projectFolder / "home/lend/file")
		)
	}

	def testPathei(): Unit = {
		assertEquals(
			"../../lend/file/whatever/long.thgin",
			(Home.projectFolder / "home/bar/gone/") / (Home.projectFolder / "home/lend/file/whatever/long.thgin")
		)
	}

	def testSplitie() = {
		assertEquals(
			List("foo", "bar/goo"),
			"foo/bar/goo".split("/", 2).toList
		)
	}

	def testThingie(): Unit = {
		assertEquals(
			Rollo.ScList(
				getName,
				Home.projectFolder / "src/test/thingie/",
				Set("thingie.cpp"),
				Set("hep.hpp", "something.h"),
				Set(),
				Set("foo.cpp")
			),
			Rollo(getName, Home.projectFolder / "src/test/thingie/")
		)
	}
}
