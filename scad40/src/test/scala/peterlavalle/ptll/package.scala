package peterlavalle

import java.io.Reader

import scala.collection.immutable.Stream.Empty

package object ptll {

  def enStream(reader: Reader, buffer: Array[Char] = Array.ofDim[Char](32), index: Int = 0, limit: Int = 0): Stream[Char] = {
    if (index < limit)
      buffer(index) #:: enStream(reader, buffer, index + 1, limit)
    else
      reader.read(buffer) match {
        case -1 => Empty
        case read =>
          enStream(reader, buffer, 0, read)
      }
  }
}
