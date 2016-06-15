package com.peterlavalle.dukscala

import org.scalatest.FunSuite

class HaoyiTest extends FunSuite {
	List(
		"2+2" -> 4,
		"7*7" -> 49,
		"0-9" -> -9,
		"3/4" -> 3 / 4,
		"5+(6*7)/8+9*(((7/6*7+9-(8))))*(0-93)" -> -6686
	).foreach {
		case (problem, solution) =>
			test(s"solving '$problem' = ${solution}") {
				assert(solution == Haoyi.eval(problem))
			}
	}
}
