package com.peterlavalle.sca

import junit.framework.TestCase
import org.junit.Assert._
import peterlavalle.Home

import com.peterlavalle.sca._

class ScakaModuleTest extends TestCase {
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
}
