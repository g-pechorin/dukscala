package peterlavalle.scad40

import java.util
import java.io.File
import org.antlr.v4.runtime._
import org.antlr.v4.runtime.atn.ATNConfigSet
import org.antlr.v4.runtime.dfa.DFA
import org.junit.Assert._
import junit.framework.TestCase
import com.peterlavalle.{D40, D40$}

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

    assertEquals(
      leikata(expected).replaceAll("[ \t\r\n]*\n", "\n"),
      (preFolder match {
        case "dukd40/" =>
          D40(FromAntlr4(parser.module()))
        case "scala-js/" =>
          val message = "Someone needs to create a Scala-JS template"
          fail(message)
          sys.error(message)
      }).trim.replaceAll("[ \t\r\n]*\n", "\n")
    )
  }

  def testScalaJS(): Unit =
    prefixIs("scala-js/", expectedSJS)

  def testDukAla(): Unit =
    prefixIs("dukd40/", expectedHpp)
}

