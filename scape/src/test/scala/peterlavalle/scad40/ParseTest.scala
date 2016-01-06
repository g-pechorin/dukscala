package peterlavalle.scad40

import fastparse.WhitespaceApi
import fastparse.core.Parsed.Success
import fastparse.noApi._
import junit.framework.Assert._
import junit.framework.TestCase
import peterlavalle.scad40.Model.{Global, Native, Script}

import scala.language.postfixOps

class ParseTest extends TestCase {

  /**
    * tiny fastparse test
    */
  def testDoomb(): Unit = {

    import fastparse.all._

    val whiteSpace = P(" " | "\n" | "\r" | "\t")
    val lowerCase = CharIn("abcdefghijklmnopqrstuvwxyz")

    val brackO = P("{")
    val brackC = P("}")

    val moduleDeclare = P("module") ~ whiteSpace.rep(min = 1) ~ P(P(lowerCase.rep(min = 1)).rep(min = 1, P(".")) !) ~ brackO ~ brackC

    moduleDeclare.parse("module foo.bar{}") match {
      case Parsed.Success(value, _) =>
        assertEquals("foo.bar", value)
    }
  }

  /**
    * fastparse whitespace example
    */
  def testWSH(): Unit = {

    object WSH {
      val White = WhitespaceApi.Wrapper {
        import fastparse.all._
        NoTrace(P(" " | "#").rep)
      }

      import White._
      import fastparse.noApi._

      def eval(tree: (Int, Seq[(String, Int)])): Int = {
        val (base, ops) = tree
        ops.foldLeft(base) { case (left, (op, right)) => op match {
          case "+" => left + right
          case "-" => left - right
          case "*" => left * right
          case "/" => left / right
        }
        }
      }

      val number: P[Int] = P(CharIn('0' to '9').rep(1).!.map(_.toInt))
      lazy val parens: P[Int] = P("(" ~/ addSub ~ ")")
      val factor: P[Int] = P(number | parens)

      val divMul: P[Int] = P(factor ~ (CharIn("*/").! ~/ factor).rep).map(eval)

      val addSub: P[Int] = P(divMul ~ (CharIn("+-").! ~/ divMul).rep).map(eval)

      val expr: P[Int] = P(" ".rep ~ addSub ~ " ".rep ~ End)

      def check(str: String, num: Int) = {
        val Parsed.Success(value, _) = expr.parse(str)
        assert(value == num)
      }
    }

    import WSH._
    check("1##+1", 2)
    check("1+   1* #  2", 3)
    check("(1+   1  *  2)+(   3*4*5)", 63)
    check("15/3#", 5)
    check("#63  /3", 21)
    check("(1+    1*2)+(3      *4*5)/20", 6)
    check("((1+      1*2)+(3*4*5))/3", 21)
  }

  /**
    * attempt to combine comments + whitespace in previous example
    */
  def testDoomC(): Unit = {

    import Parse._
    import Model._

    assertEquals(Parsed.Success(Script("Foo"), 13), scriptDeclare.parse("script Foo {}"))
    assertEquals(Parsed.Success(Native("Foo"), 13), nativeDeclare.parse("native Foo {}"))
    assertEquals(Parsed.Success(Global("Foo"), 13), globalDeclare.parse("global Foo {}"))

    val source =
      """
        |module foo.bar// ha
        |{
        | script WhatNot {
        | }
        |
        | script Management {
        | }
        |
        |}
        |
      """.stripMargin

    moduleDeclare.parse(source) match {
      case Parsed.Success((value, types), _) =>
        assertEquals("foo.bar", value)

        def consume(start: Int, done: List[TDecl]): Stream[TDecl] = {
          typeDeclare.parse(types, start) match {

            case Parsed.Success(defType: TDecl, next) =>
              require(!done.exists(_.name == defType.name))
              defType #:: consume(next, defType :: done)

            case Parsed.Failure(_, index, _) if index == types.length =>
              Stream.Empty
          }
        }

        assertEquals(
          List(
            Script("WhatNot"),
            Script("Management")
          ),
          consume(0, List()).toList
        )
    }
  }
}
