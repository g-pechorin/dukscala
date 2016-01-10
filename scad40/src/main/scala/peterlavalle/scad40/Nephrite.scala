package peterlavalle.scad40

import java.io.{InputStream, PrintWriter}
import java.text.{FieldPosition, NumberFormat, ParsePosition}

import org.fusesource.scalate.util.{Resource, ResourceLoader}
import org.fusesource.scalate.{RenderContext, TemplateEngine, TemplateException}

import scala.io.Source

/**
  * Funky casing around Scalate
  */
class Nephrite private(preFolder: String, subFolder: String, val bounds: Map[String, Any], val parent: Nephrite) {

  require(preFolder.matches("((\\w+\\-)*\\w+/)+"))
  require(subFolder.matches("(/(\\w+\\-)*\\w+)*"))

  def this(preFolder: String) = this(preFolder, "", Map(), null)

  def sub(subFolder: String): Nephrite = {
    require(subFolder.matches("(/(\\w+\\-)*\\w+)*"))
    new Nephrite(
      preFolder,
      this.subFolder + subFolder,
      bounds,
      parent
    )
  }

  def apply(pattern: String, replacement: String, obj: AnyRef): String =
    this (obj).replaceAll("^[ \t]*\n", "").replaceAll(pattern, replacement)

  def apply(replacement: String, attributeRef: AnyRef): String =
    this ("\r?\n", replacement, attributeRef)

  def apply(attributeRef: AnyRef) = {
    val uri: String =
      attributeRef.getClass.getName.replaceAll("\\$$", "").replace('.', '/').replace('$', '.')

    val attributeName = {
      val binding =
        attributeRef.getClass.getName.replaceAll("^.*\\.", "")

      binding.substring(0, 1).toLowerCase + binding.substring(1).replace("$", "")
    }

    val bind = {
      val bounds =
        Map(
          attributeName -> attributeRef
        ) ++ this.bounds

      Map("nephrite" -> new Nephrite(preFolder, subFolder, bounds, this)) ++ bounds
    }

    if ("" != subFolder) {
      println("???")
    }

    val layout: String = {
      var focus = this

      while (!focus.uriExists(uri)) {
        focus = focus.parent

        if (null == focus) {

          val packageName = uri.replaceAll("\\..*", "").replace("/", ".")
          val className = uri.replaceAll(".*/", "")
          val localName = className.substring(0, 1).toLowerCase + className.substring(1).replace(".", "")
          val name = preFolder + uri + subFolder + ".ssp"

          sys.error(
            s"""Failed to load resource `$name`
                |		<%-- maybe try this? --%>
                |			#import($packageName)
                |			<%@ val $localName: $className %>
                |			<%@ val nephrite: peterlavalle.scad40.Nephrite %>
                |			?? default template $${$localName} ??
                |
            """.stripMargin
          )
        }
      }

      focus.uriLayout(uri, bind)
    }

    leikata(layout).replaceAll("[ \t]\r?\n", "\n")
  }

  private def uriExists(uri: String) =
    Loader.exists(preFolder + uri + subFolder + ".ssp")

  private def uriLayout(uri: String, bind: Map[String, Any]) =
    Engine.layout(preFolder + uri + subFolder + ".ssp", bind)

  object Loader extends ResourceLoader {
    override def resource(name: String): Option[Resource] = {

      ClassLoader.getSystemResourceAsStream(name) match {
        case null =>

          None


        case stream: InputStream =>
          Some(
            Resource.fromSource(
              name,
              Source.fromInputStream(stream)
            )
          )
      }
    }
  }

  object Engine extends TemplateEngine {
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

    escapeMarkup = false
    resourceLoader = Loader
  }


}


