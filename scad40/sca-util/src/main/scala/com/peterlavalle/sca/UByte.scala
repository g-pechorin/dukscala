package com.peterlavalle.sca

case class UByte(value: Short) {
	requyre(0 <= value && value <= 255)

	def toByte: Byte =
		??

	def toHexString: String =
		Integer.toHexString(value)
}
