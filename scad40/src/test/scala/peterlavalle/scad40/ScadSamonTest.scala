package peterlavalle.scad40

import com.peterlavalle.DukScaCC
import junit.framework.TestCase
import org.antlr.v4.runtime.{ANTLRInputStream, CommonTokenStream}
import org.junit.Assert._

import scala.io.Source

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
      expected.trim.replaceAll("[\t\r ]*\n", "\n").replaceAll("\n\n+", "\n"),
      actual.trim.replaceAll("[\t\r ]*\n", "\n").replaceAll("\n\n+", "\n")
    )
  }

  def testDeiskIO(): Unit = {
    this ("peterlavalle.diskio")
  }
}
