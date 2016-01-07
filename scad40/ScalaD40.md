
Grease the interface between Scala and Duktape by;

 * using a DSL/IDL to specify types that need to go between
 * spitting out a `.hpp` with those types
 * popping out `.scala` files with Scala-JS trimmings

```
// IDL/DSL example
// includes comments!
module full.name {

  // these objects are constructed/implemented in **script** (and maybe passed out to native)
  script WhatNot {

    // this is a void method
    def foo()
  }

  // works like an enum?
  switch Thing {
    value,
    value2,
    athird
  }

  // these are constructed/implemented in **native** (and passed in from native)
  native ClassThing {
    def function()

    var thing: sint32 // sint(8|16|32) are your whole types

    var other: WhatNot

    val deviation: single //single|double are your real types
  }

  // special case - only one of these will exist per-VM
  global OutPuters {
    def whine(message: string) // you can get string types too!
  }
}
```

* `script` objects are wrapped in `.hpp` so they look like real-boy C++
 * they're tossed around as `struct` value types in C++
 * they have no actual state - they just point to the Scala
 * each instance maintains a stash value to hold the JVM instance
* `native` objects are implemented in C++
 * the interface always expects a pointer to be passed
  * ? should it be a `std::shared_ptr<>`
   * Duktape itself won't be thread safe
   * this would assuage issue(s) with script hanging on to pointers
 * they can be `nullptr`
 * they are pure-virtual from the header
* `global` objects are implemented in C++
 * the interface always expects a reference
 * these must be passed when the module is hook'ed in
 * they are pure-virtual from the header
* multiple modules can be defined
* no two modules can see each-other's contents
 * but your implementation(s) can hang onto objects from each side
* only a single instance of a module can be used with a VM
 * ... should this be per-thread per-global or whatnot?
* only the 6 atomic types (sint(8|16|32)|single|double|string) are available
* only previously declared classes should be passed around
 * I don't see an easy way to force this though ... hmm ...
* the DSL/IDL does not support inheritance
 * once the interface is generated you can do whatever the hell you want :)
* namespace collisions are ... bad?
