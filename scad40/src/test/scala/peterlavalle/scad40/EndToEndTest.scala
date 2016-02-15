package peterlavalle.scad40

import java.util
import java.io.File
import org.antlr.v4.runtime._
import org.antlr.v4.runtime.atn.ATNConfigSet
import org.antlr.v4.runtime.dfa.DFA
import org.junit.Assert._
import junit.framework.TestCase
import com.peterlavalle.{SJS, D40, D40$}

import scala.io.Source

class EndToEndTest extends TestCase {
  /**
    * This is an icky work around for sbt not packing resources into tests (AFAIK) and IDEA running sbt from a folder that's not the project folder
    */
  lazy val projectFolder = {
    val initialFolder = new File(".").getAbsoluteFile.getParentFile

    if (initialFolder.getName == "modules" && initialFolder.getParentFile.getName == ".idea")
      initialFolder.getParentFile.getParentFile

    else if (initialFolder.getName == "scad40" && new File(initialFolder, "build.sbt").exists() && new File(initialFolder, "src/test").exists())
      initialFolder

    else {
      val message = "Don't know where from `" + initialFolder + "`"
      System.err.println(message)
      sys.error(message)
    }
  }

  lazy val expectedHpp =
    Source.fromFile(
      new File(projectFolder, "src/test/cmake/inc/peterlavalle.diskio.hpp")
    ).mkString

  lazy val sourceScad =
    Source.fromFile(
      new File(projectFolder, "src/test/peterlavalle.diskio.scad40")
    ).mkString.trim.replaceAll("\r?\n", "\n")

  lazy val expectedSJS =
    Source.fromFile(
      new File(projectFolder, "src/test/scala-js.txt")
    ).mkString.trim.replaceAll("\r?\n", "\n")

  def prefixIs(stripWs: Boolean, preFolder: String, expected: String) = {

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
        sourceScad
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

    val regex =
      if (stripWs)
        "[ \t\r\n]*\n"
      else
        "[ \t\r]*\n"

    assertEquals(
      leikata(expected).replaceAll(regex, "\n"),
      (preFolder match {
        case "dukd40/" =>
          D40(FromAntlr4(parser.module()))
        case "scala-js/" =>
          SJS(FromAntlr4(parser.module()))
      }).trim.replaceAll(regex, "\n")
    )
  }

  def testScalaJS(): Unit =
    prefixIs(stripWs = false, "scala-js/", expectedSJS)

  def testDukAla(): Unit =
    prefixIs(stripWs = true, "dukd40/", expectedHpp)
}

