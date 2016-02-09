package peterlavalle.scad40

import java.io.StringWriter

import junit.framework.TestCase
import org.antlr.v4.runtime.{ANTLRInputStream, CommonTokenStream}
import com.peterlavalle.DukScaCC

import scala.io.Source
import org.junit.Assert._

class ScadSamonTest extends TestCase {

  def apply(name: String): Unit = {
    val expected = Source.fromInputStream(ClassLoader.getSystemResourceAsStream(name + ".hpp")).mkString

    val module: Model.Module =
      FromAntlr4(
        new DefineParser({
          new CommonTokenStream(
            new DefineLexer(
              new ANTLRInputStream(ClassLoader.getSystemResourceAsStream(name + ".scad40"))
            )
          )
        }).module()
      )

    val actual: String = DukScaCC(module)
    assertEquals(
      expected.trim.replaceAll("[\t\r ]*\n", "\n"),
      actual.trim.replaceAll("[\t\r ]*\n", "\n")
    )
  }

  def testDeiskIO(): Unit = {
    this ("peterlavalle.diskio")
  }
}
