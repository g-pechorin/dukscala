package peterlavalle.scad40

import com.peterlavalle.D40
import junit.framework.TestCase
import org.junit.Assert._

class FeatureTest extends TestCase with TAntlrCompiler {
  def assertCompilesToD40(scad40: String, duktape: String) = {
    assertEquals(
      duktape,
      D40(compile(scad40))
    )
  }

  def testNativeRawParams(): Unit = {
    assertCompilesToD40(
      """
        |module a.thing {
        |
        | native Context(raw pointy:`FILE*`) {
        | }
        |
        | script Handler {
        |   def onFrame(context: Context)
        | }
        |}
      """.stripMargin,
      """
        |
      """.stripMargin
    )
  }
}
