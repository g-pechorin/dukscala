

# TODO

 [ ] `extern` native classes that can't be constructed
 [ ] `nat` variable on not-scripts to create "whatever" C++ members

# Warning

 * there's a possible vulnerability - if methods are retained after the objects are destroyed, then the method's dangling pointer could be used to execute something unexpected
  * there's no way I know of to change jump locations or see which pointers point where so this will be "stabbing in the dark" until someone hits the bank