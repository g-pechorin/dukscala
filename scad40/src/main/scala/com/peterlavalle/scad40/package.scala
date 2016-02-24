package com.peterlavalle

import java.io.File


package object scad40 {

  trait TFile {
    def **(pattern: String): Stream[(String, File)]
  }

  implicit def wrapFile(file: File): TFile =
    new TFile {
      override def **(pattern: String): Stream[(String, File)] = {

        def recu(todo: List[String]): Stream[(String, File)] =
          todo match {
            case Nil => Stream.Empty

            case head :: tail =>
              val root = new File(file, head)

              if (root.isDirectory) {
                ???
              } else {
                ???
              }
          }

        require(file.isDirectory)
        recu(file.list().toList)
      }
    }
}
