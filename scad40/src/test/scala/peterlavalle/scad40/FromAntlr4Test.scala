package peterlavalle.scad40

import junit.framework.TestCase
import org.antlr.v4.runtime.{ANTLRInputStream, CommonTokenStream}
import org.junit.Assert._

import scala.collection.immutable.Stream.Empty

class FromAntlr4Test extends TestCase {



  def antlr4string(source: String) =
    new DefineParser(new CommonTokenStream(new DefineLexer(new ANTLRInputStream(source))))

  def testParser(): Unit = {
    val source =
      """
        |module foo.bar {
        |}
      """.stripMargin

    antlr4string(source).module() match {
      case moduleContext: DefineParser.ModuleContext =>
        assertTrue(moduleContext.declaration().isEmpty)
        assertEquals("foo.bar", moduleContext.packname().getText)
    }
  }

  def testPackName(): Unit = {
    assertEquals("foo.bar", FromAntlr4(antlr4string("foo    .   bar").packname()))
  }

  def testModule(): Unit = {
    assertEquals(
      Model.Module("foo.bar", Empty),
      FromAntlr4(antlr4string(
        """
          |module foo.bar {
          |}
        """.stripMargin).module())
    )
  }

  def testModuleScript(): Unit = {
    assertEquals(
      Model.Module("foo.bar", Stream(Model.Script("Foo", Stream()))),
      FromAntlr4(antlr4string(
        """
          |module foo.bar {
          | script Foo
          |}
        """.stripMargin).module())
    )
  }

  def testModuleScripts(): Unit = {
    assertEquals(
      Model.Module(
        "foo.bar",
        Stream(
          Model.Script("Foo", Stream()),
          Model.Script("Bar", Stream())
        )
      ),
      FromAntlr4(antlr4string(
        """
          |module foo.bar {
          | script Foo
          | script Bar {
          | }
          |}
        """.stripMargin).module())
    )
  }

  def testBigRead(): Unit = {
    assertEquals(
      Model.Module(
        "foo.bar",
        Stream(
          Model.Script("Foo", Stream()),
          Model.Native("Bar", Stream(
            Model.MemberVariable("foo", Model.KindDeclaration(Model.Script("Foo", Stream()))),
            Model.MemberValue("bar", Model.KindSingle)
          )),
          Model.Global("Fraze", Stream(
            Model.MemberFunction("foo", Stream(), Model.KindVoid),
            Model.MemberFunction("bar", Stream(), Model.KindSInt8),
            Model.MemberFunction("thump", Stream(Model.Argument("name", Model.KindString)), Model.KindVoid),
            Model.MemberValue("gloin", Model.KindDouble)
          )),
          Model.Script("WhatNot", Stream(
            Model.MemberFunction("foo", Stream(), Model.KindVoid)
          )),
          Model.Select("Thing", Set(
            "Value", "Value2", "AThird"
          )),
          Model.Native("ClassThing", Stream(
            Model.MemberFunction("functo", Stream(), Model.KindVoid),
            Model.MemberVariable("thing", Model.KindSInt32),
            Model.MemberVariable("other", Model.KindDeclaration(Model.Script("WhatNot", Stream(
              Model.MemberFunction("foo", Stream(), Model.KindVoid)
            )))),
            Model.MemberValue("deviation", Model.KindSingle)
          )),
          Model.Global("OutPuters", Stream(
            Model.MemberFunction("whine", Stream(Model.Argument("message", Model.KindString)), Model.KindVoid)
          ))
        )),
      FromAntlr4(antlr4string(
        """
          |module foo.bar {
          | script Foo
          | native Bar {
          |   var foo: Foo
          |   val bar: single
          | }
          |
          | // comment?
          | global Fraze {
          |   def foo()
          |   def bar(): sint8
          |   def thump(name: string)
          |   val gloin: double
          | }
          |
          |
          |//
          |// orignal test(s) from docthing
          |
          |
          |  // these objects are constructed/implemented in **script** (and maybe passed out to native)
          |  script WhatNot {
          |
          |    // this is a void method
          |    def foo()
          |  }
          |
          |  // works like an enum?
          |  select Thing {
          |    Value,
          |    Value2,
          |    AThird
          |  }
          |
          |  // these are constructed/implemented in **native** (and passed in from native)
          |  native ClassThing {
          |    def functo()
          |
          |    var thing: sint32 // sint(8|16|32) are your whole types
          |
          |    var other: WhatNot
          |
          |    val deviation: single //single|double are your real types
          |  }
          |
          |  // special case - only one of these will exist per-VM
          |  global OutPuters {
          |    def whine(message: string) // you can get string types too!
          |  }
          |
          |
          |}
          |
        """.stripMargin).module())
    )
  }
}
