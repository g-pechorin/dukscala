package com.peterlavalle.scad40

import java.io._
import java.util

import org.antlr.v4.runtime._
import org.antlr.v4.runtime.atn.ATNConfigSet
import org.antlr.v4.runtime.dfa.DFA
import peterlavalle.scad40.{DefineParser, DefineLexer, FromAntlr4, Model}

/**
  * By default - we just use it almost as-is
  */
object ScaD40App extends App {
  new ScaD40App(args.toList).run()
}

/**
  * This is a complex constructor that allows me to mock the whole thing and test it.
  * You'll probably want to use the single parameter override
  */
class ScaD40App(args: List[String], listing: ((File, String) => Stream[(String, File)]), reader: ((String, File) => InputStream), writer: (File => Writer)) extends Runnable {

  case class WorkLoad(roots: List[File], plops: Set[((Writer, Model.Module) => Unit, File)]) {
    lazy val todo: Map[String, File] = {
      require(roots.nonEmpty)
      require(plops.nonEmpty)
      roots.foldLeft(Map[String, File]()) {
        case (left, root: File) =>

          listing(root, "(\\w+/)*\\w+\\.scad40").foldLeft(left) {
            case (done, (name: String, file: File)) if !done.contains(name) =>
              done ++ Map(name -> file)
            case (done, _) =>
              done
          }
      }
    }
  }

  override def run(): Unit = {
    val modules =
      workLoad.todo.toList
        .sortBy(_._1)
        .map {
          case (name, file) =>

            object ErrorListener extends ANTLRErrorListener {
              override def reportContextSensitivity(recognizer: Parser, dfa: DFA, startIndex: Int, stopIndex: Int, prediction: Int, configs: ATNConfigSet): Unit = ???

              override def reportAmbiguity(recognizer: Parser, dfa: DFA, startIndex: Int, stopIndex: Int, exact: Boolean, ambigAlts: util.BitSet, configs: ATNConfigSet): Unit = ???

              override def reportAttemptingFullContext(recognizer: Parser, dfa: DFA, startIndex: Int, stopIndex: Int, conflictingAlts: util.BitSet, configs: ATNConfigSet): Unit = ???

              override def syntaxError(recognizer: Recognizer[_, _], offendingSymbol: scala.Any, line: Int, charPositionInLine: Int, msg: String, e: RecognitionException): Unit =
                throw new Exception(msg + "@" + name + ":" + line)
            }

            val defineParser =
              new DefineParser({
                val defineLexer =
                  new DefineLexer({
                    val inputStream: InputStream = reader(name, file)
                    val stream: ANTLRInputStream = new ANTLRInputStream(inputStream)
                    stream.name = name
                    stream
                  })
                defineLexer.removeErrorListeners()
                defineLexer.addErrorListener(ErrorListener)
                new CommonTokenStream(defineLexer)
              })
            defineParser.removeErrorListeners()
            defineParser.addErrorListener(ErrorListener)

            name -> FromAntlr4(defineParser.module())
        }

    // ForEach plop, create a stream and pass it into the writing function
    workLoad.plops.foreach {
      case (worker, file) =>
        modules.foldLeft(writer(file)) {
          case (wri, (_, mod)) =>
            worker(wri, mod)
            wri
        }.close()
    }
  }

  def this(args: List[String]) =
    this(
      args,
      (file, pattern) => file ** pattern,
      (_, file) => {
        require(file.exists() && file.canRead)
        new FileInputStream(file)
      },
      file => {
        require(file.getParentFile.exists() || file.getParentFile.mkdirs())
        new FileWriter(file)
      }
    )


  val workLoad: WorkLoad = {
    def recu(rgs: List[String], work: WorkLoad): WorkLoad =
      rgs match {
        case Nil =>
          require(work.roots.nonEmpty, "Need at least one path to scan")
          require(work.plops.nonEmpty, "Need at least one plop to make")
          WorkLoad(work.roots.reverse, work.plops)

        case "-i" :: path :: tail =>
          val file = new File(path).getAbsoluteFile
          recu(tail, WorkLoad(file :: work.roots, work.plops))

        case "-p" :: className :: bound :: tail =>
          Class.forName(className) match {
            case classType: Class[((Writer, Model.Module) => Unit)] if classOf[((Writer, Model.Module) => Unit)].isAssignableFrom(classType) =>
              recu(tail,
                WorkLoad(
                  work.roots,
                  work.plops + (classType.newInstance() -> new File(bound).getAbsoluteFile)
                )
              )
          }
      }

    recu(args.toList, WorkLoad(List(), Set()))
  }
}

