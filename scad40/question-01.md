I want to define "Things" ...

```
  global Thing {

  }

  native Another {

  }
```
... which map into case classes ...
```
    trait TThing {
      val name: String
    }

    case class Script(name: String) extends TThing

    case class Native(name: String) extends TThing

    case class Global(name: String) extends TThing
```
... no big deal right?

I can make up my little rules just fine ...
```
    val scriptDeclare: P[TThing] =
      P("script" ~ ((upperCase ~ (lowerCase | upperCase).rep) !) ~ brackO ~ brackC).map {
        case binding =>
          Script(binding)
      }

    assertEquals(Parsed.Success(Script("Foo"), 13), scriptDeclare.parse("script Foo {}"))

    val nativeDeclare: P[TThing] =
      P("native" ~ ((upperCase ~ (lowerCase | upperCase).rep) !) ~ brackO ~ brackC).map {
        case binding =>
          Native(binding)
      }

    assertEquals(Parsed.Success(Native("Foo"), 13), nativeDeclare.parse("native Foo {}"))

    val globalDeclare: P[TThing] =
      P("global" ~ ((upperCase ~ (lowerCase | upperCase).rep) !) ~ brackO ~ brackC).map {
        case binding =>
          Global(binding)
      }

    assertEquals(Parsed.Success(Global("Foo"), 13), globalDeclare.parse("global Foo {}"))
```

... but when I try to combine them ...
