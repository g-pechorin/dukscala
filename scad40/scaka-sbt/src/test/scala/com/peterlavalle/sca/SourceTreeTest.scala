package com.peterlavalle.sca

import java.io.File

import com.peterlavalle.sca.SourceTree.TSource
import junit.framework.TestCase
import org.junit.Assert._
import org.junit.Test

class SourceTreeTest extends TestCase {
	lazy val tempFolder =
		rootScaka / "target/tmp"
	val rootScaka: File = {

		val absoluteParent: File = new File("lalal").AbsoluteParent

		(absoluteParent.AbsolutePath match {
			case inIDEA if inIDEA.endsWith(".idea/modules") =>
				absoluteParent.getParentFile.getParentFile
		}) / "scaka-sbt"
	}
	val rootExamples = {
		requyre((rootScaka.AbsoluteParent / "build.sbt").exists())
		rootScaka / "src/test/SourceTreeTest.examples"
	}

	@Test
	def testSome(): Unit = {
		val some: File = rootExamples / "some"
		val tSource: TSource = SourceTree.of(some)
		val actual: Set[String] = tSource.contents.toSet
		assertEquals(
			Set(
				"some.c", "some.hpp", "some.inc"
			),
			actual
		)
	}

	@Test
	def testNuklearAll =
		assertEquals(
			Set(
				"demo/d3d11/main.c",
				"demo/x11_opengl2/main.c",
				"demo/x11_opengl2/nuklear_xlib_gl2.h",
				"demo/x11_opengl3/main.c",
				"demo/d3d11/nuklear_d3d11_pixel_shader.h",
				"demo/sdl_opengl2/main.c",
				"demo/sdl1_2/nuklear_sdl.h",
				"demo/emscripten/nuklear_glfw_gl3.h",
				"demo/emscripten/main.c",
				"demo/sdl_opengl3/nuklear_sdl_gl3.h",
				"demo/gdip/main.c",
				"demo/sdl_opengl3/main.c",
				"demo/node_editor.c",
				"demo/style.c",
				"demo/glfw_opengl3/main.c",
				"demo/gdip/nuklear_gdip.h",
				"demo/sdl_opengl2/nuklear_sdl_gl2.h",
				"demo/x11/nuklear_xlib.h",
				"example/skinning.c",
				"demo/x11_opengl3/nuklear_xlib_gl3.h",
				"example/canvas.c",
				"demo/d3d11/nuklear_d3d11_vertex_shader.h",
				"example/stb_image.h",
				"demo/glfw_opengl2/nuklear_glfw_gl2.h",
				"demo/x11/main.c",
				"demo/allegro5/main.c",
				"example/extended.c",
				"example/file_browser.c",
				"demo/gdi/main.c",
				"demo/d3d11/nuklear_d3d11.h",
				"demo/sdl1_2/main.c",
				"demo/glfw_opengl3/nuklear_glfw_gl3.h",
				"demo/overview.c",
				"demo/calculator.c",
				"demo/glfw_opengl2/main.c",
				"demo/allegro5/nuklear_allegro.h",
				"demo/gdi/nuklear_gdi.h",
				"nuklear.h"
			),
			SourceTree.GitHub(tempFolder / getName / "nuklear")
				.Archive("vurtun", "nuklear", "e7eb3663320b22a7c4c47982c5f15c70cb30287d")
				.Filtered(".+\\.(c|cc|cpp|h|hh|hpp|inc)$")
				.contents.toSet
		)

	@Test
	def testNuklearOne =
		assertEquals(
			Set(
				"nuklear.h"
			),
			SourceTree.GitHub(tempFolder / getName / "nuklear")
				.Archive("vurtun", "nuklear", "e7eb3663320b22a7c4c47982c5f15c70cb30287d")
				.Filtered("^nuklear.h$")
				.contents.toSet
		)

	@Test
	def testDukTape =
		assertEquals(
			Set(
				"metadata.json",
				"duktape.c",
				"duktape.h",
				"duk_config.h"
			),
			SourceTree.GitHub(tempFolder / getName / "duktape")
				.Release("svaarala", "duktape", "1.5.0")
				.SubFolder("src/")
				.contents.toSet
		)
}

