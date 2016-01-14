
These are proof-of-concept showing that [Scala-JS][orgScalaJS] can run inside of an embedded [Duktape][orgDuktape] instance.

This shows [Scala-JS][orgScalaJS] running in a `.c / .cpp` program with no JVM (... or CLR) in the runtime environment.
([Scala-JS][orgScalaJS] compilation still needs the JDK)

None of this sounds very complex or confusing to me ... but I'm used to this weirdness.
Send me a note if you need a hint or find a use for this.


This could be of use when writing in `.scala` and pouring it into a [Duktape][orgDuktape] shaped hole is easier than ... whatever else you were doing.
Garbage-Collection is always the biggest bugbear with a managed-language ; [Duktape][orgDuktape] tries to fight it with reference counting - it'd be fun to see if it outruns meh-JVMs.

Alternatively ; you could use ScaD40 on its own to glue C++ and ECMAScript together easily.

# Example-01

* ensure you have the JDK setup
* use CMake to generate, build and launch the [dukscala](example-01/dukscala.cpp) executable - neat!
	* the CMakeLists.txt file has a target that compiles the example
* now fiddle with the file [`DukScala.scala`](example-01/scala-js/src/main/scala/peterlavalle/dukscala/DukScala.scala)
	* You can use the CMake target or SBT to rebuild it
	* you can then re-run [dukscala](example-01/dukscala.cpp) to see the changes

# ScaD40

ScaD40 (Scala WD-40) is an [Interface Description Language][wikiIDL] used for (local!) calls between [Scala-JS][orgScalaJS] and [Duktape][orgDuktape]/C++.
In practice **there's no requirement that [Scala-JS][orgScalaJS] be used** - it can be used as a way to **just generate pretty C++/[Duktape][orgDuktape] bindings.**

From a scala-like definition it produces;

* C++ boilerplate source to interface with [Duktape][orgDuktape]
* [Scala-JS][orgScalaJS] base-classes that line up with these ECMAScript functions

The boilerplate will be a set of plain-old C++ classes.
The actual meat of what the boilerplate does is (intentionally) left undefined ; the developer needs to implement it.
Inline C++ (et al) were briefly considered ... but rejected since they'd likely require intricate preprocessor tooling.
Leaving the bodies outside of the boilerplate simplified the IDL (which was the new thing) so ... would suggest a shallower learning curve.

Delightfully - when the IDL file is changed, the boilerplate and base-classes can be regenerated with the new interface.
This will give the developer compile-time-errors about the shift ... rather than run-time errors that require testing.
Additionally - invoking the IDL tool (in the build) is *stable* such that it will always generate the same source file for the same definition file.
These two should mean that the tool suits protracted development with shifting requirements (the real world) since adjustments raise noticeable errors.

## ... vs JNI / JavaH

JNI is irrelevant.
I'm thinking of environments where there's no option to embed the JVM.
JNI lets you work in two ways;

* bind C++ functions to `native` JVM functions
* lookup C++ code to use for `native` JVM functions

The former is how the VM is implemented.
(`java.exe` acts as a program with the JVM embedded which happens to run an arbitrary main)
The later is (much less interesting to me but sady) what people think of when they hear "JNI."

JNI's `javah` is used for lookup-style and "just" generates `.h` files with the expected prototypes.
While it's less imposing the loss of structure irritates me.

**REGARDLESS** I'm in this for a portable embedding solution and the JVM needs to be installed/updated on the client machine.

## ... vs Not-this-just-as-is

You'd still need to implement a facade between C++ and Duktape.
Even without Scala-JS this is ... less than fun to do more than once.

## ... vs SWIG

[SWIG][orgSWIG] is the most likely "Why didn't you just ..." solution.
Superfically - what I'd seen of SWIG didn't make me want to use it.

* [SWIG][orgSWIG] was designed to access native code in managed/script code
 * **not my goal - I want to play with script-code from native-code**
  * though both directions would inevitably require someway to dial-out back to native code
  * [SWIG does support this](http://www.swig.org/Doc1.3/Java.html#java_directors)
* [SWIG][orgSWIG] lets you do all-the-things
 * **beyond my interest - I want a strict interface** this is mostly a FUD concern; I'm guessing a strict(er) delineation of what's where will give a nice "bump" in lightness
* [SWIG][orgSWIG] generates `.java` JNI code for the JVM
 * **I'm using [Scala-JS][orgScalaJS] - not JVM** while I'm not militantly opposed to JNI - I want something suitable for consumers/games (... who all seem to be irationally afraid that Java and the JVM will make them sterile or whatever)

In theory, I *could have* modified it to fit my needs ... but it didn't seem as fun.
I therefore chose to implement a less flexible subset of [SWIG][orgSWIG] which I (expect) provides a shallower learning-curve.

# Further Work

Given how good I feel about SCaD40 - I'd like to have a play at using it to embed the JVM.
ScaD40 should be able to spit-out JVM code.
This would make debugging easier since JDWP is (relatively) cool.
It would be nice if it was transparent to the "client code" and the interfaces could be reused.

[orgDuktape]: http://duktape.org/
[orgScalaJS]: http://www.scala-js.org/
[orgSWIG]: http://www.swig.org/
[wikiIDL]: https://en.wikipedia.org/wiki/Interface_description_language
