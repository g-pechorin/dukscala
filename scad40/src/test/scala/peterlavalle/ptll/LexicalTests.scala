package peterlavalle.ptll

import java.io.StringReader

import junit.framework.TestCase
import org.junit.Assert._

class LexicalTests extends TestCase {
  def testEnStream(): Unit = {
    assertEquals(
      Stream('a', 'b', 'c'),
      enStream(new StringReader("abc"))
    )
  }
}
