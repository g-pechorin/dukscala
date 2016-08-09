package com.peterlavalle.sca

import java.io.ByteArrayInputStream
import java.util.zip.{DeflaterInputStream, InflaterInputStream}

import junit.framework.TestCase
import org.junit.Assert._

class TinTest extends TestCase {
	def testCompression(): Unit = {

		val expected = "Why hello there my darling chicken".getBytes("UTF-8")

		new DeflaterInputStream(new ByteArrayInputStream(expected)).toByteStream

		assertEquals(
			expected.toStream,
			new InflaterInputStream(new DeflaterInputStream(new ByteArrayInputStream(expected))).toByteStream
		)
	}
}
