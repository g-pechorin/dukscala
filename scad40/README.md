
# Usage

My objective was to prevent writing (typically verbose) interfaces between [Duktape][orgDuktape] and C (C++).
While not challenging to do "the first time" ...

* It's boring - so people do it badly
* I've never known of a project that can get it *right* the first
* It tends to be inconsistent
* ... so there must be a better way

I expected that interface could be expressed with a simplified textual description like this ...

``` scala
  module peterlavalle.diskio {

   script ChangeListener {
     def fileChanged(path: string)
   }

   native Reading {
     def read(): sint8
     def close()
     def endOfFile(): bool

     raw handle: `void*`
     raw buffer: `std::array<uint8_t, 128>`
     raw index: `size_t`
   }

   global Disk {
     def open(path: string): Reading
   }
  }
```

... and a build tool could generate C++ headers for it as part of the compilation process.


The use of [Scala-JS][orgScalaJS] was whimsical - the generated scripts work fine without it.

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


# Benefit

Rapid development usually involves iterative changes to project sources.
Agile development benefits from / embraces automated checking to track and isolate errors.
(... in various degrees anyway. *Agile* is frequently taken as a justification for an *unstructured hack-a-thon* style of project management rather than a disciplined "keep it running or find a new job" mentality)


# Drawback

[F.U.D.][wikiFUD] basically.

There's some churn with strings/object references that should be expected (with C++, DukTape, etc) but if that makes/breaks your project "ur doin it wrong" to some extent.

The relative loss of control should be worth the free-consistency.

There's no actual requirement that you use Scala-JS anywhere.

# Further Work

Given how good I feel about SCaD40 - I'd like to have a play at using it with JNI/JVM.
ScaD40 should be able to spit-out JVM code.
This would make debugging easier since JDWP is (relatively) cool.
It would be nice if it was transparent to the "client code" and the interfaces could be reused.



[orgDuktape]: http://duktape.org/
[orgScalaJS]: http://www.scala-js.org/
[orgSWIG]: http://www.swig.org/
[wikiIDL]: https://en.wikipedia.org/wiki/Interface_description_language
[wikiFUD]: https://en.wikipedia.org/wiki/Fear,_uncertainty_and_doubt
