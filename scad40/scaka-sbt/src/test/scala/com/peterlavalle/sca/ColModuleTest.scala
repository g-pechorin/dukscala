package com.peterlavalle.sca

import junit.framework.TestCase
import org.junit.Assert._

class ColModuleTest extends TestCase {
	def testInOrder(): Unit = {
		val shared = Col.Module("shared", Seq(), Seq())
		val command = Col.Module("command", Seq(), Seq(shared))

		assertEquals(
			Stream(shared, command),
			Col.Module.inOrder(Seq(command))
		)
	}

	def testFindArtifacts(): Unit = {
		val sharedA = Col.Module("sharedA", Seq(), Seq())
		val sharedB = Col.Module("sharedB", Seq(), Seq(sharedA))
		val commandA = Col.Module("commandA", Seq(), Seq(sharedA))
		val commandB = Col.Module("commandB", Seq(), Seq(sharedB))
		val commandC = Col.Module("commandC", Seq(), Seq())


		assertEquals(
			Stream(
				Col.Module("sharedA", Seq(), Seq()) -> false,
				Col.Module("sharedB", Seq(), Seq(sharedA)) -> false,
				Col.Module("commandA", Seq(), Seq(sharedA)) -> true,
				Col.Module("commandB", Seq(), Seq(sharedB)) -> true,
				Col.Module("commandC", Seq(), Seq()) -> true),
			Col.Module.labelArtifacts(Seq(sharedA, sharedB, commandA, commandB, commandC))
		)
	}
}
