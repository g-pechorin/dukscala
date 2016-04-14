
This is (intended to be) a *grease-layer* between Scala-JS and C/++.

... rather than a glue-layer that does it for you, but, not always the way you'd like.

It does this with an IDL and some SBT trickery to prepare `CMakeLists.txt` file(s) for projects.




This is ...

* a [proof-of-concept demo](example-01/) showing [Scala-JS][ScalaJS] inside of C++/[Duktape][Duktape]
* *out of date* **incomplete** [demo](example-02/) showing [FastParse][FastParse] in C++
	* TODO;
		* [ ] one true build ; switch to Scaka/Ginny for native code
		* [ ] debug whatever is going wrong
* some SBT stuff including
	* the [Interface Definition Language / binding generator](scad40/) with (decoupled! independent!) modules for
		* C++/[Duktape][orgDuktape]
		* [Scala-JS][orgScalaJS]
	* [Scaka](scad40/scaka-sbt/) - a magical CMakeLists.txt generator to make CMake dance to SBT's tune
	* an inevitable *utility code* [project](scad40/sca-util/) shared by the others

... so ; it's my pile-of-faoss-stuff repository.

... sorry-not-sorry


[Duktape]: http://duktape.org/
[ScalaJS]: http://www.scala-js.org/
[orgSWIG]: http://www.swig.org/
[wikiIDL]: https://en.wikipedia.org/wiki/Interface_description_language
[wikiFUD]: https://en.wikipedia.org/wiki/Fear,_uncertainty_and_doubt
[FastParse]: http://www.lihaoyi.com/fastparse/