package peterlavalle.scad40

import scala.collection.immutable.Stream.Empty

object Model {

  sealed trait TKind {
    val name: String
  }

  case object KindVoid extends TKind {
    val name: String = "void"
  }

  case object KindBool extends TKind {
    val name: String = "bool"
  }

  case object KindSInt8 extends TKind {
    val name: String = "sint8"
  }

  case object KindSInt16 extends TKind {
    val name: String = "sint16"
  }

  case object KindSInt32 extends TKind {
    val name: String = "sint32"
  }

  case object KindString extends TKind {
    val name: String = "string"
  }

  case object KindSingle extends TKind {
    val name: String = "single"
  }

  case object KindDouble extends TKind {
    val name: String = "double"
  }

  case class KindDeclaration(declaration: TDeclaration) extends TKind {
    val name: String = "@" + declaration.name
    require(!declaration.isInstanceOf[Global])
  }

  sealed trait TMember {
    val name: String
  }

  case class Argument(name: String, kind: TKind)

  case class MemberVariable(name: String, kind: TKind) extends TMember

  case class MemberValue(name: String, kind: TKind) extends TMember

  case class MemberFunction(name: String, arguments: Stream[Argument], result: TKind) extends TMember

  sealed trait TDeclaration {
    val name: String
    val members: Stream[TMember]
  }

  case class Script(name: String, members: Stream[TMember]) extends TDeclaration

  case class Native(name: String, members: Stream[TMember]) extends TDeclaration

  case class Global(name: String, members: Stream[TMember]) extends TDeclaration

  case class Select(name: String, options: Set[String]) extends TDeclaration {
    val members = Empty
  }

  case class Module(name: String, contents: Stream[TDeclaration])

}
