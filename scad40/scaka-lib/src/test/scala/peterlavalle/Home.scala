package peterlavalle

import java.io.File

object Home {
	/**
		* This is an icky work around for sbt not packing resources into tests (AFAIK) and IDEA running sbt from a folder that's not the project folder
		*/
	lazy val projectFolder =
		new File({
			val initialFolder = new File(".").getAbsoluteFile.getParentFile

			if (initialFolder.getName == "modules" && initialFolder.getParentFile.getName == ".idea")
				initialFolder.getParentFile.getParentFile

			else if (new File(initialFolder, "build.sbt").exists())
				initialFolder

			else {
				val message = "Don't know where from `" + initialFolder + "`"
				System.err.println(message)
				sys.error(message)
			}
		}, "scaka-lib")

}
