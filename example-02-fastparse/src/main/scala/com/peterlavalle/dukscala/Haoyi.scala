package com.peterlavalle.dukscala


import fastparse.ParserApi
import fastparse.all._

import scala.collection.Seq
import scala.language.implicitConversions
import scala.scalajs.js.annotation.JSExport

@JSExport
object Haoyi {
	// from : https://github.com/lihaoyi/fastparse/issues/72
	implicit def wrap2_10_6(s: String): ParserApi[Unit] = parserApi(s)

	// from : https://github.com/lihaoyi/fastparse/issues/72
	implicit def wrap2_10_6[T](s: Parser[T]): ParserApi[T] = parserApi(s)

	import fastparse.all._

	val number: P[Int] = P(CharIn('0' to '9').rep(1).!.map(_.toInt))
	val parenthesis: P[Int] = P("(" ~/ addSub ~ ")")
	val factor: P[Int] = P(number | parenthesis)

	val divMul: P[Int] = P(factor ~ (CharIn("*/").! ~/ factor).rep).map {
		case (i: Int, things: Seq[(String, Int)]) =>
			things.foldLeft(i) {
				case (last: Int, ("*", next: Int)) => last * next
				case (last: Int, ("/", next: Int)) => last / next
			}
	}

	val addSub: P[Int] = P(divMul ~ (CharIn("+-").! ~/ divMul).rep).map {
		case (i: Int, things: Seq[(String, Int)]) =>
			things.foldLeft(i) {
				case (last: Int, ("+", next: Int)) => last + next
				case (last: Int, ("-", next: Int)) => last - next
			}
	}

	val expr: P[Int] = P(addSub ~ End)

	@JSExport
	def eval(text: String): Int =
		expr.parse(text) match {
			case Parsed.Success(value, _) => value
		}

}
