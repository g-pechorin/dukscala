package peterlavalle.scad40

import junit.framework.TestCase
import org.fusesource.scalate.util.{Resource, ResourceLoader}
import org.junit.Assert._
import org.fusesource.scalate._
import peterlavalle.scad40.Model.MemberVariable
import peterlavalle.scad40._


class NephriteTest extends TestCase {

  val extension: String = "ssp"



  def testMatchin(): Unit = {
    assertEquals(
      "peterlavalle.scad40.Model.MemberVariable",
      classOf[peterlavalle.scad40.Model.MemberVariable].getCanonicalName
    )
  }

  def testTranslating(): Unit = {
    val nephrite: Nephrite = new Nephrite("test/")


    val obj: MemberVariable = Model.MemberVariable("foo", Model.KindString)

    val output = nephrite(obj)

    assertEquals(
      leikata(
        """
          |
          |var foo : std::string = ???
          |
        """.stripMargin),
      leikata(output)
    )
  }

  def testTranslateScriptClass(): Unit =
    assertEquals(
      leikata(
        """
          |
          |    @ScalaJSDefined
          |    trait Foo extends js.Object {
          |
          |    }
          |
        """.stripMargin),
      leikata(
        new Nephrite("scala-js/").apply(Model.Script("Foo", Stream()))
      )
    )

  def testModule() =
    assertEquals(
      leikata(
        """
          |package foo.bar {
          |
          |    @ScalaJSDefined
          |    trait Foo extends js.Object {
          |
          |        def foo(): Unit
          |
          |        def foo(bar: Byte): Float
          |
          |        def foo(bar: Short, sen: Byte): String
          |
          |    }
          |
          |    @ScalaJSDefined
          |    trait Bar extends js.Object {
          |
          |        var thing: Int
          |
          |        val gloin: Double
          |
          |    }
          |}
          |
        """.stripMargin),
      new Nephrite("scala-js/").apply(
        Model.Module(
          "foo.bar",
          Stream(
            Model.Script("Foo", Stream(
              Model.MemberFunction("foo", Stream(), Model.KindVoid),
              Model.MemberFunction("foo", Stream(Model.Argument("bar", Model.KindSInt8)), Model.KindSingle),
              Model.MemberFunction("foo", Stream(Model.Argument("bar", Model.KindSInt16), Model.Argument("sen", Model.KindSInt8)), Model.KindString)
            )),
            Model.Script("Bar", Stream(
              Model.MemberVariable("thing", Model.KindSInt32),
              Model.MemberValue("gloin", Model.KindDouble)
            ))
          )
        )
      )

    )
}
