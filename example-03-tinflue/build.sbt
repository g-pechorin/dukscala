
import com.peterlavalle.sca._

lazy val root = (project in file("."))
	.enablePlugins(TinPlugin)
	.settings(
    tinSource +=
      Tin.Flue(file("src/data/ttf"), ".*\\.ttf")
	)
