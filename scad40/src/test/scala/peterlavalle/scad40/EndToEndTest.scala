package peterlavalle.scad40

import java.util

import org.antlr.v4.runtime._
import org.antlr.v4.runtime.atn.ATNConfigSet
import org.antlr.v4.runtime.dfa.DFA
import org.junit.Assert._
import junit.framework.TestCase

import scala.io.Source

class EndToEndTest extends TestCase {
  val source =
    Source.fromInputStream(
      ClassLoader.getSystemResourceAsStream(
        "src/test/scad40/peterlavalle.diskio.scad40"
      )
    ).mkString.trim.replaceAll("\r?\n","\n")

  def prefixIs(preFolder: String, expected: String) = {

    object Fail extends ANTLRErrorListener {
      override def reportContextSensitivity(parser: Parser, dfa: DFA, i: Int, i1: Int, i2: Int, atnConfigSet: ATNConfigSet): Unit =
        ???

      override def reportAmbiguity(parser: Parser, dfa: DFA, i: Int, i1: Int, b: Boolean, bitSet: util.BitSet, atnConfigSet: ATNConfigSet): Unit =
        ???

      override def reportAttemptingFullContext(parser: Parser, dfa: DFA, i: Int, i1: Int, bitSet: util.BitSet, atnConfigSet: ATNConfigSet): Unit =
        ???

      override def syntaxError(r: Recognizer[_, _], o: scala.Any,
                               line: Int, charPositionInLine: Int,
                               message: String, recognitionException: RecognitionException): Unit = {
        throw new Exception(message + "@" + line + "/" + charPositionInLine, recognitionException)
      }
    }

    val lexer: DefineLexer = new DefineLexer(
      new ANTLRInputStream(
        source
      )
    )


    lexer.removeErrorListeners()
    lexer.addErrorListener(Fail)

    val parser: DefineParser = new DefineParser(
      new CommonTokenStream(
        lexer
      )
    )

    parser.removeErrorListeners()
    parser.addErrorListener(Fail)

    assertEquals(
      leikata(expected),
      new Nephrite(preFolder)(FromAntlr4(
        parser.module()))
    )
  }


  def testScalaJS(): Unit =
    prefixIs("scala-js/",
      """
        |package peterlavalle.diskio {
        |
        |    @ScalaJSDefined
        |    trait ChangeListener extends js.Object {
        |
        |        def fileChanged(path: String): Unit
        |
        |    }
        |
        |    @js.native
        |    class Reading extends js.Object {
        |
        |        @js.native
        |        def read(): Byte = js.native
        |
        |        @js.native
        |        def close(): Unit = js.native
        |
        |        @js.native
        |        def endOfFile(): Bool = js.native
        |
        |        @js.native
        |        var number: Float = js.native
        |
        |        @js.native
        |        val path: String = js.native
        |
        |    }
        |
        |    @js.native
        |    object Disk extends js.Object {
        |
        |        @js.native
        |        def open(path: String): Reading = js.native
        |
        |        @js.native
        |        var pwd: String = js.native
        |
        |        @js.native
        |        def subscribe(path: String, listener: ChangeListener): Unit = js.native
        |
        |        @js.native
        |        def unsubscribe(path: String, listener: ChangeListener): Unit = js.native
        |
        |    }
        |}
      """.stripMargin
    )

  def testDukAla(): Unit =
    prefixIs(
      "dukd40/",
      Source.fromInputStream(
        ClassLoader
          .getSystemResourceAsStream("peterlavalle.diskio.hpp")
      ).mkString
    )
}

