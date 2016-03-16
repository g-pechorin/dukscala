package peterlavalle.scad40

import java.io.ByteArrayInputStream
import java.util

import org.antlr.v4.runtime.atn.ATNConfigSet
import org.antlr.v4.runtime.dfa.DFA
import org.antlr.v4.runtime._

trait TAntlrCompiler {
  def getName: String

  def compile(source: String) = {

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

    val inputStream: ANTLRInputStream = new ANTLRInputStream(
      new ByteArrayInputStream(source.getBytes)
    )
    inputStream.name = getName
    val lexer: DefineLexer = new DefineLexer(
      inputStream
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

    FromAntlr4(parser.module())
  }
}
