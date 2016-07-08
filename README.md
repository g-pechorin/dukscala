Easier (maybe even easy?) embedding of Scala in C++.

# Goal

This is (intended to be) a *grease-layer* between Scala-JS and C++ via [DukTape][DukTape].

... rather than a glue-layer that does it for you, but, not always the way you'd like.

It does this with an IDL and some SBT trickery to prepare things like `CMakeLists.txt` file(s) for projects.

# Contents

There is ...

* a [scaka-sbt demo](example-00-scaka-sbt/) showing how I'm setting up C/++ builds
 * run with `sbt col` and check for `example-00-scaka-sbt/target/ColCMakeApp/CMakeLists.txt`
* *out of date* a [proof-of-concept demo](example-01/) showing [Scala-JS][ScalaJS] inside of C++/[DukTape][DukTape]
	* TODO;
		* [ ] use scaka-sbt for all build stuff
* *out of date* **incomplete** [demo](example-02/) showing [FastParse][FastParse] in C++
	* TODO;
		* [ ] use scaka-sbt for all build stuff
		* [ ] debug whatever is going wrong (did I do this last weekend?)
* **incomplete** [an example using the binary file tools to pre-process art](example-03-arttool/)
	* TODO;
		* [ ] ??? finish it ???
* scad40 proper, including
	* the Interface Definition Language / binding generator with (decoupled! independent!) output modules for
		* C++/[Duktape][orgDuktape]
		* [Scala-JS][orgScalaJS]
	* [Scaka](scad40/scaka-sbt/) - a thing to generate native builds and do other shizzle from SBT
	  * currently generates a workspace-wide CMakeLists and can download C/++ from other hosts/tools as a my-own dependency mechanism
		* I've never been happy with linking other people's pre-built code so ... this tool doesn't even try
	* an inevitable *utility code* [project](scad40/sca-util/) shared by the others

... so ; it's my pile-of-faoss-stuff repository.

... sorry-not-sorry


[DukTape]: http://duktape.org/
[ScalaJS]: http://www.scala-js.org/
[orgSWIG]: http://www.swig.org/
[wikiIDL]: https://en.wikipedia.org/wiki/Interface_description_language
[wikiFUD]: https://en.wikipedia.org/wiki/Fear,_uncertainty_and_doubt
[FastParse]: http://www.lihaoyi.com/fastparse/
