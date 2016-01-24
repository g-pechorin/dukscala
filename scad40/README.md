From ...

  [this](https://github.com/g-pechorin/dukscala/blob/scad40/scad40/src/test/resources/peterlavalle.diskio.scad40) 
  
... definition you can get these two ...

 * [source](https://github.com/g-pechorin/dukscala/blob/scad40/scad40/src/test/resources/peterlavalle.diskio.hpp) `.hpp` file
 * [source](https://github.com/g-pechorin/dukscala/blob/scad40/scad40/src/test/scala/peterlavalle/scad40/EndToEndTest.scala#L71) `.scala`

# TODO

 * [ ] fill in the last stubs
 * [ ] achieve parity with the hand-written header
 * [ ] package as a `main()` and publish/run from SBT
  * need to get `src/main/resources/` into the build
 * [ ] pre-compile ssp for speed (and ease) https://github.com/backchatio/xsbt-scalate-generate
  * might prevent runtime templates
 * [ ] `nat` variable on not-scripts to create "whatever" C++ members/methods that ES can't see
 * [ ] `extern` native classes that can't be constructed
 * [ ] `range` thing to declare values as being restricted to a range of numbers or regular expressions
  * use C++ `explicit` and wrap the thing(s) to force assignments (in C++) to be intentional
 * [ ] clean up Nephrite and template resolution
 * [ ] `either` classes that can be created and manipulated on either side
 * [x] rewrite inheritance to make actual sense

# Warning

 * there's a possible vulnerability - if methods are retained after the objects are destroyed, then the method's dangling pointer could be used to execute something unexpected
  * there's no way I know of to change jump locations or see which pointers point where so this will be "stabbing in the dark" until someone hits the bank
