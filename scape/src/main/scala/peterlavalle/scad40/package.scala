package peterlavalle

package object scad40 {
  def ??? = {
    val notImplementedError = new NotImplementedError()
    notImplementedError.setStackTrace(notImplementedError.getStackTrace.tail)
    throw notImplementedError
  }
}
