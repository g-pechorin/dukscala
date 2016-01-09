package peterlavalle

package object scad40 {
  def ??? = {
    val notImplementedError = new NotImplementedError()
    notImplementedError.setStackTrace(notImplementedError.getStackTrace.tail)
    throw notImplementedError
  }

  def leikata(original: String): String = {
    val changed =
      original
        .replaceAll("^([\\s^\n]*\n)", "\n")
        .replaceAll("\r?\n", "\n")
        .replaceAll("\\s*$", "")
        .replaceAll("\n[ \n]+\n", "\n\n")
        .replaceAll("(^|\n)([ \t]*?)    ([ \t]*?)", "$1$2\t$3")

    if (changed == original)
      original
    else
      leikata(changed)
  }
}
