package peterlavalle.scad40

import fastparse.WhitespaceApi

import scala.collection.mutable.ArrayBuffer
import scala.language.postfixOps

object Example extends App {
  val (white, whiteSpace) = {
    import fastparse.all._
    val whiteSpace = (" " | "\r\n" | "\n" | "\t").rep
    (WhitespaceApi.Wrapper {
      NoTrace(whiteSpace)
    }, whiteSpace)
  }

  import fastparse.noApi._
  import white._


  val chaLetter = CharIn('a' to 'z') | CharIn('Z' to 'Z')
  val chaDigit = CharIn('0' to '9')
  val chaCurlC = P("}")
  val chaCurlO = P("{")

  val tokWhiteSpace = (" " | "\r\n" | "\n" | "\t").rep
  val tokName = chaLetter ~ (chaLetter | chaDigit).rep(min = 0)

  val rulDeclaration: P[Model.TDecl] =
    (P("global" | "native" | "script").! ~ tokName.! ~ chaCurlO ~ chaCurlC)
      .map {
        case ("global", name) => Model.Global(name)
        case ("native", name) => Model.Native(name)
        case ("script", name) => Model.Script(name)
      }


  if (true) {
    val source =
      """
        |global Foo {
        |}
        |
        |native Bar {
        |}
        |
      """.stripMargin

    val expected =
      List(Model.Global("Foo"), Model.Native("Bar"))


    val rulModuleBody =
      P(tokWhiteSpace ~ "{" ~/ rulDeclaration.rep(sep = tokWhiteSpace.~/) ~ tokWhiteSpace)

    rulModuleBody.parse(source.trim, 0) match {
      case null =>
        ???
    }

    val hide: P[List[Model.TDecl]] =
      (Start ~ rulDeclaration.rep(min = 1, sep = whiteSpace.~/) ~ End)
        .map {
          case arrayBuffer: ArrayBuffer[Model.TDecl] =>
            arrayBuffer.toList
        }


    hide.parse(source, 0) match {
      case null =>
        ???
    }
    ???
  }

  def declaration(source: String, start: Int): (Model.TDecl, Int) =
    rulDeclaration.parse(source, start) match {

      case Parsed.Success(decl, next) =>
        (decl, next)

      case Parsed.Failure(_, index, _) if start == index =>
        (null, index)
    }

  require(declaration("script bar { }", 0) match {
    case (Model.Script("bar"), 14) => true
  })

  def declarationList(source: String, start: Int): (List[Model.TDecl], Int) = {
    declaration(source, start) match {

      case (null, last) =>
        (List(), last)

      case (head: Model.TDecl, next: Int) =>
        declarationList(source, next) match {
          case (tail, last) =>
            (head :: tail, last)
        }
    }
  }

  require(List(Model.Script("bar")) == declarationList("script thing {}", 0)._1)
  require(List(Model.Global("Foo"), Model.Native("Bar")) == {
    declarationList(
      """
        |global Foo {
        |}
        |
        |native Bar {
        |}
        |
      """.stripMargin, 0)._1
  })


  println("Yipee")
}
