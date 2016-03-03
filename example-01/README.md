
# Example-01

* ensure you have the JDK setup
* use CMake to generate, build and launch the [dukscala](example-01/dukscala.cpp) executable - neat!
	* the CMakeLists.txt file has a target that compiles the example
* now fiddle with the file [`DukScala.scala`](example-01/scala-js/src/main/scala/peterlavalle/dukscala/DukScala.scala)
	* You can use the CMake target or SBT to rebuild it
	* you can then re-run [dukscala](example-01/dukscala.cpp) to see the changes
