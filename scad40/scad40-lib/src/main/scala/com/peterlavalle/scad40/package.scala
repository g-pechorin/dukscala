package com.peterlavalle

import java.io.File


package object scad40 {

  trait TFile {
    def **(pattern: String): Stream[(String, File)]
  }

  implicit def wrapFile(root: File): TFile =
    new TFile {
      override def **(pattern: String): Stream[(String, File)] = {

        def recu(todo: List[String]): Stream[(String, File)] =
          todo match {
            case Nil => Stream.Empty

            case thisPath :: tail =>
              val thisFile = new File(root, thisPath)

              thisFile.list() match {
                case null if thisPath.matches(pattern) =>
                  (thisPath -> thisFile) #:: recu(tail)

                case null =>
                  recu(tail)

                case list: Array[String] =>
                  recu(todo ++ list.map(thisPath + "/" + _))
              }
          }

        require(root.isDirectory)
        recu(root.list().toList)
      }
    }
}
