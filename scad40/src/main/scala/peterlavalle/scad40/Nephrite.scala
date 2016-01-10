package peterlavalle.scad40

import java.io.PrintWriter
import java.text.{FieldPosition, NumberFormat, ParsePosition}

import org.fusesource.scalate.util.{Resource, ResourceLoader}
import org.fusesource.scalate.{RenderContext, TemplateEngine, TemplateException}

import scala.io.Source

/**
  * Funky casing around Scalate
  */
class Nephrite private(prefolder: String, val obj: Any, val parent: Nephrite, val engine: TemplateEngine) {

  require(prefolder.matches("((\\w+\\-)*\\w+/)+"))

  def this(prefolder: String) =
    this(prefolder, null, null, Nephrite(List())(prefolder))

  def sub(subfolder: String): Nephrite = {
    require(subfolder.matches("((\\w+\\-)*\\w+/)+"))

    sys.error("Defer back to us when can't find sub")

    new Nephrite(prefolder + subfolder, null, this, engine)
  }

  def apply(pattern: String, replacement: String, obj: AnyRef): String =
    this (obj).replaceAll("^[ \t]*\n", "").replaceAll(pattern, replacement)

  def apply(replacement: String, obj: AnyRef): String =
    this ("\r?\n", replacement, obj)

  def apply(obj: AnyRef, extras: (String, Any)*) = {
    val uri: String = obj.getClass.getName.replaceAll("\\$$", "").replace('.', '/').replace('$', '.') + ".ssp"

    val binding = {
      val binding =
        obj.getClass.getName.replaceAll("^.*\\.", "")

      binding.substring(0, 1).toLowerCase + binding.substring(1).replace("$", "")
    }

    val bounds =
      Map(
        binding -> obj,
        "obj" -> obj,
        "nephrite" -> new Nephrite(prefolder, obj, this, engine)
      ) ++ extras.toMap

    leikata(engine.layout(prefolder + uri, bounds)).replaceAll("[ \t]\r?\n", "\n")
  }

  val depth: Int =
    if (null != parent)
      parent.depth + 1
    else
      0
}

object Nephrite {

  def apply(imports: List[String])(prefolder: String, lookup: (String => String) = _ => null) = {
    val engine = new TemplateEngine {

      override protected def createRenderContext(uri: String, out: PrintWriter): RenderContext = {
        val renderContext: RenderContext = super.createRenderContext(uri, out)

        val oldNumberFormat: NumberFormat = renderContext.numberFormat
        renderContext.numberFormat =
          new NumberFormat {
            override def parse(source: String, parsePosition: ParsePosition): Number =
              oldNumberFormat.parse(source, parsePosition)

            override def format(number: Double, toAppendTo: StringBuffer, pos: FieldPosition): StringBuffer =
              toAppendTo.append(number)

            override def format(number: Long, toAppendTo: StringBuffer, pos: FieldPosition): StringBuffer =
              toAppendTo.append(number)
          }

        renderContext
      }
    }
    engine.importStatements = engine.importStatements ++ imports
    engine.escapeMarkup = false
    engine.resourceLoader =
      new ResourceLoader {
        override def resource(name: String): Option[Resource] = {

          lookup(name) match {
            case text: String =>
              Some(Resource.fromText(name, text))

            case null =>
              val stream =
                ClassLoader.getSystemResourceAsStream(name)

              require(name.startsWith(prefolder))

              val uri = name.substring(prefolder.length).replaceAll("\\.ssp$", "")
              val packageName = uri.replaceAll("\\..*", "").replace("/", ".")
              val className = uri.replaceAll(".*/", "")
              val localName = className.substring(0, 1).toLowerCase + className.substring(1).replace(".", "")

              require(null != stream,
                s"""Failed to load resource `$name`
                    |		<%-- maybe try this? --%>
                    |			#import($packageName)
                    |			<%@ val $localName: $className %>
                    |			<%@ val nephrite: peterlavalle.scad40.Nephrite %>
                    |			?? default template $${$localName} ??
                    |
                """.stripMargin
              )

              Some(
                Resource.fromSource(
                  name,
                  Source.fromInputStream(
                    stream
                  )
                )
              )
          }
        }

      }
    engine
  }
}
