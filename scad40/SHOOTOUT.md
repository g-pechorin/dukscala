
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

In theory, I *could have* modified [SWIG][orgSWIG] to fit my needs ... but learning-other-tool then modifying-other-tool for my needs followed by adapting-build-for-other-tool didn't seem as fun.

... so I chose to implement my (less flexible) subset of what [SWIG][orgSWIG] does.
