package peterlavalle.scad40

import junit.framework.TestCase
import org.fusesource.scalate.util.{Resource, ResourceLoader}
import org.junit.Assert._
import org.fusesource.scalate._
import peterlavalle.scad40.Model.MemberVariable
import peterlavalle.scad40._


class NephriteTest extends TestCase {

  val extension: String = "ssp"

  /**
    * Test that we're not being strung into HMTL :)
    */
  def testWhat(): Unit = {
    val engine =
      Nephrite(List()) {
        case name if name.startsWith("/foo/bar.") && name.endsWith(extension) =>
          """
            |why hello there
          """.stripMargin
      }

    assertEquals("why hello there", engine.layout("/foo/bar." + extension).trim)
  }

  /**
    * Test that case-match works as I want
    */
  def testMatcher(): Unit = {

    import peterlavalle.scad40.Deboog

    val engine =
      Nephrite(List("import peterlavalle.scad40.Deboog._")) {
        case name if name.startsWith("/foo/bar.") && name.endsWith(extension) =>
          """
            |<%@ val param: TTo %>
            |<%@ val roll: Stream[TTo] %>
            |
            |why hello there
            |
            |
            |#match(param)
            |#case(Two(n))
            |${n} is a number
            |#case(Too(n))
            |${n} is a name
            |#otherwise
            |waT?
            |#end
            |
            |
            |
            |
            |#for (dude <- roll)
            | #match(dude)
            |   #case(Two(math))
            |     ${math} is mathematical
            |   #case(Too(verb))
            |
            |     ${verb} let's us get adverbial
            |
            |   #otherwise
            |     waT?
            | #end
            |#end
            |
            |
          """.stripMargin
      }

    assertEquals(
      """
        |why hello there
        |
        |
        |1983 is a number
        |
        |     3 is mathematical
        |
        |     waT?
        |
        |     14 is mathematical
        |
        |
        |     unequivocally let's us get adverbial
        |
      """.stripMargin.trim.replaceAll("\n(\\s*\n)+", "\n\n"),
      engine.layout(
        "/foo/bar." + extension,
        Map(
          "param" -> Deboog.Two(1983),
          "roll" -> Stream(
            Deboog.Two(3),
            null,
            Deboog.Two(14),
            Deboog.Too("unequivocally")
          )
        )).trim.replaceAll("\n(\\s*\n)+", "\n\n")
    )
  }


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
          |package foo
          |
          |package object bar {
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
          |
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
