package peterlavalle.scad40

import java.io.{FileWriter, FileInputStream, File}
import java.util

import org.antlr.v4.runtime.atn.ATNConfigSet
import org.antlr.v4.runtime.dfa.DFA
import org.antlr.v4.runtime._

object DumpExample extends App {
  val scad40 =
    new File("src/test/resources/peterlavalle.diskio.scad40").getAbsoluteFile

  val duktape =
    new File("target/peterlavalle.diskio.hpp").getAbsoluteFile

  require(scad40.exists())
  require(duktape.getParentFile.exists() || duktape.getParentFile.mkdirs())

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


  val lexer: DefineLexer =
    new DefineLexer({
      val inputStream: ANTLRInputStream =
        new ANTLRInputStream(
          new FileInputStream(scad40)
        )
      inputStream.name = scad40.getAbsolutePath
      inputStream
    })
  lexer.removeErrorListeners()
  lexer.addErrorListener(Fail)

  val parser: DefineParser =
    new DefineParser(
      new CommonTokenStream(
        lexer
      )
    )
  parser.removeErrorListeners()
  parser.addErrorListener(Fail)

  val module = parser.module()

  private val nephrite: Nephrite = new Nephrite("dukd40/")

  val generated = nephrite(FromAntlr4( module))

  new FileWriter(duktape)
    .append(generated)
    .close()
}
