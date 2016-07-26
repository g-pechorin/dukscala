Scala and C++ *experiments* with a bunch of "out of date" parts that need to put back together.

* Development is carried out on [BitBucket](https://bitbucket.org/g-pechorin/dukscala)
* ... and mirrored to [GitHub](https://github.com/g-pechorin/dukscala) sometimes ...



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
* *out of date* **incomplete** [an example using the binary file tools to pre-process art](example-03-arttool/)
	* TODO;
		* [ ] ??? finish it ???
* *out of date* scad40 proper, including
	* the Interface Definition Language / binding generator with (decoupled! independent!) output modules for
		* C++/[Duktape][orgDuktape]
		* [Scala-JS][orgScalaJS]
	* [Scaka](scad40/scaka-sbt/) - a thing to generate native builds and do other shizzle from SBT
	  * currently generates a workspace-wide CMakeLists and can download C/++ from other hosts/tools as a my-own dependency mechanism
		* I've never been happy with linking other people's pre-built code so ... this tool doesn't even try
	* an inevitable *utility code* [project](scad40/sca-util/) shared by the others

... so ; it's my pile-of-faoss-stuff repository.

... sorry-not-sorry

# History

## 0.0.4

 * TinFlue /
  * macros emit a symbol name
  * [ ] used Zopfli to compress (rather than straightup ZLib)
  * [ ] strip ZLib headers from compressed streams

## 0.0.3

* introduced the `Tin.Flue` to compress files into DEFLATE'd header source

## 0.0.2

* switched from `Ginny` to `Col` ... I think ... that might have been earlier


[DukTape]: http://duktape.org/
[ScalaJS]: http://www.scala-js.org/
[orgSWIG]: http://www.swig.org/
[wikiIDL]: https://en.wikipedia.org/wiki/Interface_description_language
[wikiFUD]: https://en.wikipedia.org/wiki/Fear,_uncertainty_and_doubt
[FastParse]: http://www.lihaoyi.com/fastparse/
