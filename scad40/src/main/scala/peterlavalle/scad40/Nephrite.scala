package peterlavalle.scad40

import java.io.PrintWriter
import java.text.{FieldPosition, NumberFormat, ParsePosition}

import org.fusesource.scalate.util.{ResourceNotFoundException, Resource, ResourceLoader}
import org.fusesource.scalate.{RenderContext, TemplateEngine, TemplateException}

import scala.io.Source

/**
  * Funky casing around Scalate
  */
class Nephrite private(prefolder: String, val bound: Map[String, Any], val parent: Nephrite) {

  object ChainedResourceLoader extends ResourceLoader {
    override def resource(name: String): Option[Resource] = {

      require(!name.startsWith(prefolder))

      ClassLoader.getSystemResourceAsStream(prefolder + name) match {
        case null if null != parent =>
          parent.ChainedResourceLoader.resource(name)

        case null =>

          require(name.startsWith(prefolder))

          val uri = name.substring(prefolder.length).replaceAll("\\.ssp$", "")
          val packageName = uri.replaceAll("\\..*", "").replace("/", ".")
          val className = uri.replaceAll(".*/", "")
          val localName = className.substring(0, 1).toLowerCase + className.substring(1).replace(".", "")

          throw new IllegalArgumentException(
            s"""Failed to load resource `$name`
                |		<%-- maybe try this? --%>
                |			#import($packageName)
                |			<%@ val $localName: $className %>
                |			<%@ val nephrite: peterlavalle.scad40.Nephrite %>
                |			?? default template $${$localName} ??
                |
                """.stripMargin
          )

        case stream =>
          require(null != stream)

          Some(
            Resource.fromSource(name, Source.fromInputStream(stream))
          )
      }
    }
  }

  val engine: TemplateEngine = {
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
    engine.escapeMarkup = false
    engine.resourceLoader = ChainedResourceLoader

    engine
  }

  def this(prefolder: String) =
    this(prefolder, null, null)

  require(prefolder.matches("((\\w+\\-)*\\w+/)+"))

  def sub(subfolder: String): Nephrite = {
    require(subfolder.matches("((\\w+\\-)*\\w+/)+"))

    sys.error("Defer back to us when can't find sub")

    new Nephrite(prefolder + subfolder, null, this)
  }

  def apply(pattern: String, replacement: String, obj: AnyRef): String =
    this (obj).replaceAll("^[ \t]*\n", "").replaceAll(pattern, replacement)

  def apply(replacement: String, obj: AnyRef): String =
    this ("\r?\n", replacement, obj)

  def apply(obj: AnyRef) = {
    val uri: String = obj.getClass.getName.replaceAll("\\$$", "").replace('.', '/').replace('$', '.') + ".ssp"

    val binding = {
      val binding =
        obj.getClass.getName.replaceAll("^.*\\.", "")

      binding.substring(0, 1).toLowerCase + binding.substring(1).replace("$", "")
    }

    val bounds =
      Map(
        binding -> obj,
        "obj" -> obj
      ) ++ (if (null == bound) Map() else bound)

    val layout: String = engine.layout(uri, Map("nephrite" -> new Nephrite(prefolder, bounds, this)) ++ bounds)

    leikata(layout).replaceAll("[ \t]\r?\n", "\n")
  }
}
