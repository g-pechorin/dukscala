package peterlavalle.scad40

import fastparse.WhitespaceApi
import scala.language.postfixOps

object Parse {


  val whiteSpace = {
    import fastparse.all._

    val breakers = End | "\r\n" | "\n"

    val comment = P("//") ~ (!breakers ~ AnyChar).rep ~ breakers

    val whiteSpace = P(comment | " " | breakers | "\t")

    whiteSpace
  }

  val White =
    WhitespaceApi.Wrapper {
      import fastparse.all._
      NoTrace(whiteSpace.rep)
    }

  import fastparse.noApi._
  import White._

  val lowerCase = CharIn('a' to 'z')
  val upperCase = CharIn('A' to 'Z')
  val numerical = CharIn('0' to '9')

  val brackO = P("{")
  val brackC = P("}")

  import Model._

  val scriptDeclare: P[TDecl] =
    P("script" ~ ((upperCase ~ (lowerCase | upperCase).rep) !) ~ brackO ~ brackC).map {
      case binding =>
        Script(binding)
    }

  val nativeDeclare: P[TDecl] =
    P("native" ~ ((upperCase ~ (lowerCase | upperCase).rep) !) ~ brackO ~ brackC).map {
      case binding =>
        Native(binding)
    }

  val globalDeclare: P[TDecl] =
    P("global" ~ ((upperCase ~ (lowerCase | upperCase).rep) !) ~ brackO ~ brackC).map {
      case binding =>
        Global(binding)
    }

  val typeDeclare = {
    val typeDeclare = Start ~ P(scriptDeclare | nativeDeclare | globalDeclare)

    require(
      Global("Foo") == (typeDeclare.parse("    global Foo    { } \n ", 0) match {
        case Parsed.Success((value), 21) =>
          value
      })
    )

    require(
      Script("WhatNot") == (typeDeclare.parse(
        """
          |script WhatNot {
          |}
          |
        """.stripMargin, 0) match {
        case Parsed.Success((value), 21) =>
          value
      })
    )

    typeDeclare
  }


  val moduleDeclare =
    whiteSpace ~ P("module") ~ P(P((lowerCase | numerical).rep(min = 1)).rep(min = 1, P(".")) !) ~ brackO ~ (scriptDeclare | nativeDeclare | globalDeclare).rep.! ~ brackC

}
