# CoffeeDuk Behaviors

Using "Try CoffeeScript" [on the CoffeeScript website](http://coffeescript.org/) allows the usage of this;

```
behave = (type) ->
  alert(type.name + '@' + type.fileName + ' = ' + type)

behave class Myne
  constructor: (name) ->
    @name = name

class Animalss
  constructor: (name) ->
    @name = name

behave Animalss

alert "Hello CoffeeScript!"
```

My intent is to setup DukTape to allow Unity3D like behaviours written in CoffeeScript


# DukTape Sandbox

https://github.com/svaarala/duktape/blob/master/doc/sandboxing.rst
TODO (largely) since it's irrelevant for a PoC demo

# DukTape Registry

Push C/++ functions globally;
* `behaviour(class): void`
* `create(name: string, [class ...]): soul`

this should be cone as stupidly as possible to make them available always and forever by name and I'm serious always do this guy!

# Behavior Instance

has a secret readonly property `.owner: Pawn` and a method `.detach(): void`

# Soul / Pawn

Due to the ambguity associated with "Actor" and "Entity" names I'm using "Soul" as my-in-engine term for a thing that exists.
I'm using "Pawn" as the term for a behavior that's attached to a Soul.
Both have a GUID formed from casting their `this` to a `size_t`

Souls have methods
* `attach(class | class-name): pawn`
* `lookup(class | class-name, [index]): pawn`
* `remove(): void`
* `listall(class | class-name): pawn[]`
* `findany(class | class-name): pawn[]`

Pawns have methods
* `detach(): void`
* `vessel: soul`
