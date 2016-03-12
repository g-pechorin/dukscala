package com.peterlavalle.dukscala

import utest._
import utest.framework.TestSuite

object DumbCompilerTest extends TestSuite {

  val tests =
    TestSuite {
      'testFoo {
        assert(4 == DumbCompiler.doThing("2+2"))
      }
      'testBar {
        assert(4 == DumbCompiler.doThing("2    + 2"))
      }
      'add3and4 {
        assert(7 == DumbCompiler.doThing("  3+ 4 "))
      }
    }

}
