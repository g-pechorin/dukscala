
This is a proof-of-concept showing that [Scala-JS](http://www.scala-js.org/) can run inside of an embedded [Duktape](http://duktape.org/) instance.

This shows Scala running in a `.c / .cpp` program with no JVM (... or CLR) in the runtime environment.
(Scala compilation still needs the JDK)

# Example-01

* ensure you have the JDK setup
* use CMake to generate, build and launch the [dukscala](example-01/dukscala.cpp) executable - neat!
	* the CMakeLists.txt file has a target that compiles the example
* now fiddle with the file [`DukScala.scala`](example-01/scala-js/src/main/scala/peterlavalle/dukscala/DukScala.scala)
	* You can use the CMake target or SBT to rebuild it
	* you can then re-run [dukscala](example-01/dukscala.cpp) to see the changes

# Thoughts

This could be of use when writing in `.scala` and pouring it into a Duktape shaped hole is easier than ... whatever else you were doing.
Garbage-Collection is always the biggest bugbear with a managed-language ; Duktape tries to fight it with reference counting - it'd be fun to see if it outruns meh-JVMs.


# Further Work

If you want Mr Scala-JS to play with Mr Duktape/C/C++ then you'll need to [setup facade types or whatever](http://www.scala-js.org/doc/interoperability/)

When Scala-JS knows what functions it can call - you'll have to [implement those interfaces for Duktape](http://duktape.org/guide.html#programming.8)

None of this sounds very interesting or confusing to me ... but I'm used to this weirdness.
Send me a note if you need a hint or find a use for this.
