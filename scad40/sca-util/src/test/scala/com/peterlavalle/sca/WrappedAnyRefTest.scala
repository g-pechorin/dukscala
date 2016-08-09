package com.peterlavalle.sca

import junit.framework.TestCase

import org.junit.Assert._

class WrappedAnyRefTest extends TestCase {
	def testToHex() =
		assertEquals(
			"18CC600000000000000000000006CC81",
			"foo".HexString
		)

	def testToHex2_3_4_n5_1() =
		assertEquals(
			"18-CC6-0000-000000000000000000-6-CC81",
			"foo".HexString(+2, +3, +4, -5, +1)
		)
}
