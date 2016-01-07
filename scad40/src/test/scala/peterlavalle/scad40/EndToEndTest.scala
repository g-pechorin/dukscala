package peterlavalle.scad40

import org.antlr.v4.runtime.{ANTLRInputStream, CommonTokenStream}
import org.junit.Assert._
import junit.framework.TestCase

class EndToEndTest extends TestCase {
  val source =
    """
      |
      |module peterlavalle.diskio {
      | native Reading {
      |   def read(): sint8
      |   def close()
      | }
      | global Disk {
      |   def open(path: string): Reading
      | }
      |}
      |
    """.stripMargin

  def prefixIs(preFolder: String, expected: String) =
    assertEquals(
      leikata(expected),
      new Nephrite(preFolder)(FromAntlr4(
        new DefineParser(
          new CommonTokenStream(
            new DefineLexer(
              new ANTLRInputStream(
                source
              )
            )
          )
        ).module()))
    )



  def testScalaJS(): Unit =
    prefixIs("scala-js/",
      """
        |package peterlavalle.diskio
        |
        |    @js.native
        |    class Reading extends js.Object {
        |
        |        @js.native
        |        def read(): Byte = js.native
        |
        |        @js.native
        |        def close(): Unit = js.native
        |
        |    }
        |
        |    @js.native
        |    object Disk extends js.Object {
        |
        |        @js.native
        |        def open(path: String): Reading = js.native
        |
        |    }
      """.stripMargin
    )


  def testDuk40(): Unit =
    prefixIs("Duk40/",
      """
        |#pragma once
        |
        |#include <duktape.h>
        |#include <stdint.h>
        |
        |namespace peterlavalle {
        |namespace diskio {
        |
        |    struct Reading
        |    {
        |        virtual uint8_t read(void) = 0;
        |        virtual void close(void) = 0;
        |
        |        std::shared_ptr<Reading> to(duk_context* ctx, duk_idx_t index)
        |        {
        |            assert(false && "??? scad40 needs to provide this");
        |        }
        |
        |        bool is(duk_context* ctx, duk_idx_t index)
        |        {
        |            assert(false && "??? scad40 needs to provide this");
        |        }
        |
        |        void push(duk_context* ctx, std::shared_ptr<Reading>& )
        |        {
        |            assert(false && "??? scad40 needs to provide this");
        |        }
        |    };
        |
        |    template<typename T>
        |    struct Disk
        |    {
        |        std::shared_ptr<Reading> open(const char* path);
        |    };
        |
        |    template<typename T>
        |    void Duk40(duk_context* ctx, T&)
        |    {
        |        assert(false && "??? scad40 needs to provide this");
        |    }
        |}
        |}
      """.stripMargin
    )
}

