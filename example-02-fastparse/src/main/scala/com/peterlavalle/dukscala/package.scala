package com.peterlavalle

package object dukscala {
  def ?? = {
    try {
      sys.error("STUB")
    } catch {
      case runtimeException: RuntimeException =>
        runtimeException.setStackTrace(runtimeException.getStackTrace.drop(2))
        throw runtimeException
    }
  }
}
