package peterlavalle.scad40

import peterlavalle.scad40.DefineParser.MemberContext
import peterlavalle.scad40.Model.KindDeclaration

import scala.collection.JavaConversions._
import scala.collection.immutable.Stream.Empty

object FromAntlr4 {
  def apply(returnTypeContext: DefineParser.ReturnTypeContext, done: List[Model.TDeclaration]): Model.TKind =
    if (null == returnTypeContext || null != returnTypeContext.VOID())
      Model.KindVoid
    else
      FromAntlr4(returnTypeContext.typeId(), done)

  def apply(typeidContext: DefineParser.TypeIdContext, done: List[Model.TDeclaration]): Model.TKind = {
    typeidContext match {
      case defined: DefineParser.DefinedContext =>
        val name: String = defined.UNAME.getText
        done.find(_.name == name) match {
          case Some(tType) =>
            KindDeclaration(tType)
          case None =>
            sys.error("Can't find declaration `%s`".format(name))
        }
      case builtin: DefineParser.BuiltinContext =>
        builtin.ATOMIC().getText match {
          case "bool" =>
            Model.KindBool
          case "sint8" =>
            Model.KindSInt8
          case "sint16" =>
            Model.KindSInt16
          case "sint32" =>
            Model.KindSInt32
          case "string" =>
            Model.KindString
          case "single" =>
            Model.KindSingle
          case "double" =>
            Model.KindDouble
        }
    }
  }

  def apply(valueContext: DefineParser.ValueContext, done: List[Model.TDeclaration]): (String, Model.TKind) =
    (valueContext.LNAME().getText, FromAntlr4(valueContext.typeId(), done))

  def apply(definitionContext: DefineParser.DefinitionContext, done: List[Model.TDeclaration]): Stream[Model.TMember] =
    if (null == definitionContext)
      Empty
    else {
      def recu(memberStream: Stream[MemberContext], seen: List[Model.TMember]): Stream[Model.TMember] =
        memberStream match {
          case Empty => Empty

          case head #:: tail =>
            val next =
              head match {
                case accessorContext: DefineParser.AccessorContext =>
                  FromAntlr4(accessorContext.value(), done) match {
                    case (name, kind) =>
                      Model.MemberVariable(name, kind)
                  }

                case methodContext: DefineParser.MethodContext =>
                  Model.MemberFunction(
                    methodContext.LNAME.getText, {
                      def argRecu(todo: List[DefineParser.ValueContext], seen: List[Model.Argument]): Stream[Model.Argument] =
                        todo match {
                          case Nil => Empty
                          case head :: tail =>
                            val name = head.LNAME().getText

                            require(
                              !seen.exists(_.name == name),
                              "Argument Name `%s` appears more than once".format(name)
                            )

                            val next =
                              Model.Argument(name, FromAntlr4(head.typeId(), done))


                            next #:: argRecu(tail, next :: seen)
                        }

                      argRecu(methodContext.value().toList, List())
                    },
                    FromAntlr4(methodContext.returnType, done))

                case readContext: DefineParser.ReadContext =>
                  FromAntlr4(readContext.value(), done) match {
                    case (name, kind) =>
                      Model.MemberValue(name, kind)
                  }
              }

            require(
              !seen.exists(_.name == next.name),
              "Member Name `%s` appears more than once".format(next.name)
            )

            next #:: recu(tail, next :: seen)
        }

      recu(definitionContext.member().toStream, List())
    }

  def apply(packnameContext: DefineParser.PacknameContext): String =
    packnameContext.LNAME.map(_.getText).reduce(_ + "." + _)

  def apply(declarationStream: Stream[DefineParser.DeclarationContext]): Stream[Model.TDeclaration] = {
    def recu(declarationStream: Stream[DefineParser.DeclarationContext], done: List[Model.TDeclaration]): Stream[Model.TDeclaration] =
      declarationStream match {
        case Empty =>
          Empty


        case head #:: tail =>
          val next = {

            head match {
              case (select: DefineParser.DeclSelContext) =>

                select.UNAME().map(_.getText).toList match {
                  case Nil =>
                    sys.error("The grammar should require at least two states")
                  case name :: _ :: _ :: Nil =>
                    sys.error("The grammar should require at least two states")
                  case name :: values =>
                    require(
                      !done.exists(_.name == name),
                      "Type Name `%s` appears more than once".format(name)
                    )

                    require(
                      values.toSet.size == values.length
                    )

                    Model.Select(name, values.toSet)
                }


              case _ =>
                val (model, name, members) =
                  head match {
                    case script: DefineParser.DeclScrContext =>
                      (Model.Script, script.UNAME().getText, script.definition())
                    case native: DefineParser.DeclNatContext =>
                      (Model.Native, native.UNAME().getText, native.definition())
                    case global: DefineParser.DeclGloContext =>
                      (Model.Global, global.UNAME().getText, global.definition())
                  }

                require(
                  !done.exists(_.name == name),
                  "Type Name `%s` appears more than once".format(name)
                )
                model(name, FromAntlr4(members, model(name, Stream()) :: done))
            }
          }

          next #:: recu(tail, next :: done)
      }

    recu(declarationStream, Nil)
  }

  def apply(moduleContext: DefineParser.ModuleContext): Model.Module = {

    Model.Module(
      FromAntlr4(moduleContext.packname()),
      FromAntlr4(moduleContext.declaration().toStream)
    )
  }
}
