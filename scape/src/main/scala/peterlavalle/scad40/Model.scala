package peterlavalle.scad40

object Model {

  sealed trait TDecl {
    val name: String
  }

  case class Script(name: String) extends TDecl

  case class Native(name: String) extends TDecl

  case class Global(name: String) extends TDecl
}
