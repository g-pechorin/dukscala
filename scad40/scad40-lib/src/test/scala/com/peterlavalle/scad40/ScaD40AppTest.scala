package com.peterlavalle.scad40

import java.io._

import com.peterlavalle.SJS
import junit.framework.TestCase
import org.junit.Assert._
import org.easymock.EasyMock

class ScaD40AppTest extends TestCase {
  def testTheMocker(): Unit = {
    val runnable: Runnable = EasyMock.mock(classOf[Runnable])

    EasyMock.expect(runnable.run()).once()

    EasyMock.replay(runnable)

    runnable.run()

    EasyMock.verify(runnable)
  }

  def setup = {
    val listing: ((File, String) => Stream[(String, File)]) =
      EasyMock.createMock(classOf[(File, String) => Stream[(String, File)]])

    val reading: (String, File) => InputStream =
      EasyMock.createMock(classOf[(String, File) => InputStream])

    val writing: (File) => Writer =
      EasyMock.createMock(classOf[File => Writer])

    (listing, reading, writing, () => {

      EasyMock.replay(listing, reading, writing)

      new ScaD40App(
        List(
          "-p", classOf[SJS].getName, "foo/bar.scala",
          "-i", "bey/ond"
        ),
        listing, reading, writing
      )
    })
  }

  def testScan(): Unit = {
    val (listing, reading, writing, scaD40AppL) = setup

    EasyMock.expect(
      listing(new File("bey/ond/").getAbsoluteFile, "(\\w+/)*\\w+\\.scad40")
    ).andReturn(Stream(
      "foo.scad40" -> new File("bey/ond/foo.scad40").getAbsoluteFile
    )).once()

    val scaD40App = scaD40AppL()

    assertEquals(
      Map("foo.scad40" -> new File("bey/ond/foo.scad40").getAbsoluteFile),
      scaD40App.workLoad.todo
    )

    EasyMock.verify(listing, reading, writing)
  }

  def testBig(): Unit = {
    val (listing, reading, writing, scaD40AppL) = setup

    EasyMock.expect(
      listing(new File("bey/ond/").getAbsoluteFile, "(\\w+/)*\\w+\\.scad40")
    ).andReturn(Stream(
      "foo.scad40" -> new File("bey/ond/foo.scad40").getAbsoluteFile
    )).once()

    EasyMock.expect(
      reading("foo.scad40", new File("bey/ond/foo.scad40").getAbsoluteFile)
    ).andReturn(
      new ByteArrayInputStream(
        """
          |module foo.bar {
          | script Scriptus {
          |   def foo(i:sint32)
          |
          | }
          |
          | native Barsoom {
          |   def bar(s:single)
          | }
          |}
          |
        """.stripMargin.getBytes
      )
    ).once()

    val scalaWriter = new StringWriter()

    EasyMock.expect(
      writing(new File("foo/bar.scala").getAbsoluteFile)
    ).andReturn(
      scalaWriter
    ).once()

    val scaD40App = scaD40AppL()

    scaD40App.run()

    EasyMock.verify(listing, reading, writing)

    assertEquals(
      """
        |package foo.bar {
        |
        |	import scala.scalajs.js
        |
        |
        |
        |	@ScalaJSDefined
        |	trait Scriptus extends js.Object {
        |		def foo(i: Int): Unit
        |	}
        |
        |	@js.native
        |	class Barsoom extends js.Object {
        |
        |		@js.native
        |		def bar(s: Float): Unit = js.native
        |
        |	}
        |
        |}
      """.stripMargin.trim.replaceAll("[ \t\r\n]*\n", "\n"),
      scalaWriter.toString.trim.replaceAll("[ \t\r\n]*\n", "\n")
    )
  }
}
