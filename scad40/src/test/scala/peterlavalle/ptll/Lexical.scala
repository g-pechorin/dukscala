package peterlavalle.ptll

import java.io.Reader

import scala.Option
import scala.annotation.tailrec
import scala.collection.immutable.Stream.Empty

object Lexical {


  sealed trait TToken {
    // TODO ; line and position should be here
  }

  case class Token(name: String, text: String) extends TToken

  case class Error(char: Char) extends TToken


  sealed trait TStep {
    // TODO ; some sort of "chomp" thing
  }

  case class Finish(name: String) extends TStep

  case class Match(min: Char, max: Char) extends TStep

  case class Repeat(steps: List[TStep]) extends TStep

  case class Alternative(steps: List[TStep]) extends TStep {
    assume(1 < steps.length)
  }


  type Input = Stream[Char]

  def peek(possible: Stream[TStep], hopeful: Input, done: List[Char]): Option[(Token, Input)] =
    possible.head match {

      case Finish(name: String) =>
        assert(Empty == possible.tail)
        Some(Token(name, done.reverse.foldLeft("")(_ + _)), hopeful)

      case Match(min: Char, max: Char) if min <= hopeful.head && hopeful.head <= max =>
        peek(possible.tail, hopeful.tail, hopeful.head :: done)
    }


  case class Lexer(tokens: List[Stream[TStep]]) {

    def apply(reader: Reader, buffer: Array[Char] = Array.ofDim[Char](32)): Stream[Token] = {
      ???
    }

    /*
        @tailrec
        final def apply(input: Input): Stream[Token] = {

          def peel(pending: List[Stream[TStep]]): (Token, Input) =
            pending match {
              case Nil =>
                (Error(input.head), input.tail)
              case head :: tail =>
                peek(head, input) match {
                  case Option((next, text)) =>
                    (next, text)
                  case None =>
                    ???
                  // peel(tail)
                }
            }


          // TODO ; something clever to concatenate errors
          peel(tokens) match {
            case (token, stream) =>
              token #:: (stream match {
                case Empty =>
                  Empty
                case tail =>
                  apply(tail)
              })
          }
        }
    */
  }

}
