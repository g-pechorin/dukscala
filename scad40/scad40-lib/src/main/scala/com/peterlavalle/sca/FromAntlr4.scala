package com.peterlavalle.sca

import java.io.InputStream
import java.util

import com.peterlavalle.sca.DefineParser.MemberContext
import com.peterlavalle.sca.Model.KindDeclaration
import org.antlr.v4.runtime._
import org.antlr.v4.runtime.atn.ATNConfigSet
import org.antlr.v4.runtime.dfa.DFA

import scala.collection.JavaConversions._
import scala.collection.immutable.Stream.Empty

/**
	* Converts ANTLR4 output into my model stuff
	*/
object FromAntlr4 {

	def apply(path: String, inputStream: InputStream): Model.Module = {

		object ErrorListener extends ANTLRErrorListener {
			override def reportContextSensitivity(recognizer: Parser, dfa: DFA, startIndex: Int, stopIndex: Int, prediction: Int, configs: ATNConfigSet): Unit = ???

			override def reportAmbiguity(recognizer: Parser, dfa: DFA, startIndex: Int, stopIndex: Int, exact: Boolean, ambigAlts: util.BitSet, configs: ATNConfigSet): Unit = ???

			override def reportAttemptingFullContext(recognizer: Parser, dfa: DFA, startIndex: Int, stopIndex: Int, conflictingAlts: util.BitSet, configs: ATNConfigSet): Unit = ???

			override def syntaxError(recognizer: Recognizer[_, _], offendingSymbol: scala.Any, line: Int, charPositionInLine: Int, msg: String, e: RecognitionException): Unit = ???
		}

		val antlrStream: ANTLRInputStream = new ANTLRInputStream(inputStream)
		antlrStream.name = path

		val defineLexer: DefineLexer =
			new DefineLexer({
				antlrStream
			})

		defineLexer.removeErrorListeners()
		defineLexer.addErrorListener(ErrorListener)

		val commonTokenStream =
			new CommonTokenStream({
				defineLexer
			})


		val parser: DefineParser = new DefineParser(commonTokenStream)

		parser.removeErrorListeners()
		parser.addErrorListener(ErrorListener)

		FromAntlr4(
			parser.module()
		)
	}

	def apply(moduleContext: DefineParser.ModuleContext): Model.Module = {

		Model.Module(
			FromAntlr4(moduleContext.packname()),
			FromAntlr4(moduleContext.declaration().toStream)
		)
	}

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


	def apply(dufinitionContext: DefineParser.DufinitionContext, done: List[Model.TDeclaration]): Stream[Model.TMember] =
		if (null == dufinitionContext)
			Empty
		else {

			def recu(memberStream: Stream[DefineParser.MumberContext], seen: List[Model.TMember]): Stream[Model.TMember] =
				memberStream match {
					case Empty =>
						Empty

					case (mumberContext: DefineParser.MumberContext) #:: tail =>
						val next =
							mumberContext match {
								case (mm: DefineParser.MmContext) =>
									FromAntlr4(mm.member(), seen, done)

								case (rm: DefineParser.RmContext) =>
									Model.MemberRaw(
										rm.LNAME().getText,
										rm.TIKTEXT().getText.substring(1).reverse.substring(1).reverse
									)
							}

						require(
							!seen.exists(_.name == next.name),
							"Member Name `%s` appears more than once".format(next.name)
						)

						next #:: recu(tail, next :: seen)


				}

			recu(dufinitionContext.mumber().toStream, List())
		}

	def apply(memberContext: MemberContext, seen: List[Model.TMember], done: List[Model.TDeclaration]): Model.TMember =
		memberContext match {
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

	def apply(definitionContext: DefineParser.DefinitionContext, done: List[Model.TDeclaration]): Stream[Model.TMember] =
		if (null == definitionContext)
			Empty
		else {
			def recu(memberStream: Stream[DefineParser.MemberContext], seen: List[Model.TMember]): Stream[Model.TMember] =
				memberStream match {
					case Empty => Empty

					case head #:: tail =>
						val next =
							FromAntlr4(head, seen, done)

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

							case native: DefineParser.DeclNatContext =>

								val mode =
									native.getChild(0).getText match {
										case "native" => Model.Native
										case "global" => Model.Global
									}

								val name =
									native.UNAME().getText

								require(
									!done.exists(_.name == name),
									"Type Name `%s` appears more than once".format(name)
								)

								mode(name, FromAntlr4(native.dufinition(), mode(name, Stream()) :: done))

							case script: DefineParser.DeclScrContext =>
								val name = script.UNAME().getText
								require(
									!done.exists(_.name == name),
									"Type Name `%s` appears more than once".format(name)
								)
								Model.Script(name, FromAntlr4(script.definition(), Model.Script(name, Stream()) :: done))
						}
					}

					next #:: recu(tail, next :: done)
			}

		recu(declarationStream, Nil)
	}
}
