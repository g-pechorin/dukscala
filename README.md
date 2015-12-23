
This is a proof-of-concept showing that [Scala-JS](http://www.scala-js.org/) can run inside of [Duktape](http://duktape.org/)

This shows Scala running in a `.c / .cpp` program with no JVM (... or CLR) in the runtime environment.
(Scala compilation still needs the JDK)

# Building

* ensure you have the JDK setup
* run `sbtw.bat ~ fastOptJS` to start popping out a built `.js` file
	* ... well ... any [sbt]() invocation would work
	* the path is hard coded (sorry)
	* If you're on not-Windows `sbtw/bin/sbt ~ fastOptJS` should work
* use CMake to generate, build and launch the [dukscala](dukscala.cpp) executable - neat!
* now fiddle with the file `[DukScala.scala](scala/src/main/scala/peterlavalle/dukscala/DukScala.scala)`
	* sbt will rebuild it when you save
	* you can then re-run [dukscala](dukscala.cpp) to see the changes

# Thoughts

This would primarily be of use when writing in `.scala` and pouring it into a Duktape shaped hole is easier than ... whatever else you were doing.
I'm playing with a toy shading language and a property-driven engine so I'll probably use it there.
Garbage-Collection is always the biggest bugbear with a managed-language ; Duktape tries to fight it with reference counting which could be fun to see in action.

Send me a note if you need a hint or find a use for this.

# Further Work

If you want Mr Scala-JS to play with Mr Duktape/C/C++ then you'll need to [setup facade types or whatever](http://www.scala-js.org/doc/interoperability/)

When Scala-JS knows what functions it can call - you'll have to [implement those interfaces for Duktape](http://duktape.org/guide.html#programming.8)

None of this sounds very interesting or confusing to me ... but I'm used to this weirdness.
