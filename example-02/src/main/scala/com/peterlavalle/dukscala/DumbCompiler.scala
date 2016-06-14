package com.peterlavalle.dukscala


import fastparse.ParserApi
import fastparse.all._

import scala.language.implicitConversions
import scala.scalajs.js.annotation.JSExport

/**
	* Mostly just fastparse's example, but, running on Scala.JS
	*/
@JSExport
object DumbCompiler {
  // from : https://github.com/lihaoyi/fastparse/issues/72
  implicit def wrap2_10_6(s: String): ParserApi[Unit] = parserApi(s)

  // from : https://github.com/lihaoyi/fastparse/issues/72
  implicit def wrap2_10_6[T](s: Parser[T]): ParserApi[T] = parserApi(s)

  import fastparse.all._

  val ws = " ".rep

  lazy val number: P[Int] = ws ~ P(CharIn('0' to '9').rep(1).!.map(_.toInt))

  lazy val term: P[Int] = number | ("(" ~ expr ~ ")")

  lazy val factor: P[Int] = (term ~ ws ~ (("*" | "/" | "%").! ~ term).rep).map {
    case (value, entries) =>
      entries.foldLeft(value) {
        case (l, ("*", r)) => l * r
        case (l, ("/", r)) => l / r
        case (l, ("%", r)) => l % r
      }
  }

  lazy val expr: P[Int] =
    (factor ~ ws ~ (("+" | "-").! ~ factor).rep).map {
      case (value, entries) =>
        entries.foldLeft(value) {
          case (l, ("+", r)) =>
            l + r
          case (l, ("-", r)) =>
            l - r
        }
    }

  @JSExport
  def doThing(input: String): Int =
    expr.parse(input) match {
      case Parsed.Success(value: Int, _) =>
        value
    }

  @JSExport
  def doThing(l: Int, r: Int): Int = l + r
}
