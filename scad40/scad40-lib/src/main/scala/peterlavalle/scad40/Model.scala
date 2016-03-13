package peterlavalle.scad40

import scala.collection.immutable.Stream.Empty

object Model {

  sealed trait T {
    def source: String
  }

  sealed trait TKind extends T

  sealed trait TKindNumerical extends TKind

  case object KindVoid extends TKind {
    override def source: String = "void"
  }

  case object KindBool extends TKind {
    override def source: String = "bool"
  }

  case object KindSInt8 extends TKindNumerical {
    override def source: String = "sint8"
  }

  case object KindSInt16 extends TKindNumerical {
    override def source: String = "sint16"
  }

  case object KindSInt32 extends TKindNumerical {
    override def source: String = "sint32"
  }

  case object KindString extends TKind {
    override def source: String = "string"
  }

  case object KindSingle extends TKindNumerical {
    override def source: String = "single"
  }

  case object KindDouble extends TKindNumerical {
    override def source: String = "double"
  }

  case class KindDeclaration(declaration: TDeclaration) extends TKind {
    override def source: String = declaration.name

    require(!declaration.isInstanceOf[Global])
  }

  sealed trait TMember extends T {
    val name: String
  }

  case class Argument(name: String, kind: TKind) extends T {
    override def source: String = name + ": " + kind.source
  }

  case class MemberRaw(name: String, text: String) extends TMember {
    override def source: String = "raw %s: %s".format(name, text)
  }

  case class MemberVariable(name: String, kind: TKind) extends TMember {
    require(kind != KindVoid)

    override def source: String = "var %s: %s".format(name, kind.source)
  }

  case class MemberValue(name: String, kind: TKind) extends TMember {
    require(kind != KindVoid)

    override def source: String = "val %s: %s".format(name, kind.source)
  }

  case class MemberFunction(name: String, arguments: Stream[Argument], resultKind: TKind) extends TMember {

    override def source: String =
      "def %s(%s): %s"
        .format(
          name,
          arguments.map(_.source).foldLeft("")(_ + ", " + _).replaceAll("^, ", ""),
          resultKind.source
        )
  }

  sealed trait TDeclaration extends T {
    val name: String
    val members: Stream[TMember]
  }

  case class Script(name: String, members: Stream[TMember]) extends TDeclaration {
    require(!members.exists(_.isInstanceOf[MemberRaw]))

    override def source: String = ???
  }

  case class Native(name: String, members: Stream[TMember]) extends TDeclaration {
    override def source: String = ???
  }

  case class Global(name: String, members: Stream[TMember]) extends TDeclaration {
    override def source: String = ???
  }

  case class Select(name: String, options: Set[String]) extends TDeclaration {
    val members = Empty

    override def source: String = ???
  }

  case class Module(name: String, contents: Stream[TDeclaration]) extends T {
    override def source: String = ???
  }

}
