

1. start a project
1. mark the project as an SBT plugin
	* so in the build file `sbtPlugin := true`
1. create an `AutoPlugin` subclass with `autoImport` and `projectSettings`
	* most of this is magical and reflective

	```
	package com.peterlavalle.sca

	import sbt.AutoPlugin

	object GinnyPlugin extends AutoPlugin {

		object autoImport {
		}

		import autoImport._

		override lazy val projectSettings =
			Seq(
			)

	}

	```


1. add some settings keys and task keys for your whatnots to auto-import
	```
	...
		object autoImport {
			lazy val ginnyRemotes =
				SettingKey[Seq[(URL, String, Seq[String])]](
					"ginnyRemotes",
					"stuff that Ginny should download and extract"
				)
			lazy val ginnyScrape =
				TaskKey[Seq[(File, Seq[File], Seq[String])]](
					"ginnyScrape",
					"get Ginny to download stuff and extract it"
				)
		}
	...
	```

1. assign default values and stub implementations in settings
	```
	...	
		override lazy val projectSettings =
			Seq(
				ginnyRemotes := Seq(),
				ginnyScrape := {
					ginnyRemotes.value.map {
						case (remote: URL, path: String, symbols: Seq[String]) =>
							???
					}
				}
			)
	...
	```

1. set your plugin test and publishLocal
	* `sbt ~;test;publishLocal`

1. now, setup a project to test your plugin ... and then run it!
	* create `ginny-test/project/ginny-sbt.sbt`
	
		```
		// your project coordinates will differ
		addSbtPlugin("com.peterlavalle" %% "ginny" % "0.0.0-SNAPSHOT")
		```
	* create `ginny-test/build.sbt`
	
		```
		lazy val commonSettings =
			Seq(
			)
		
		lazy val root = (project in file("."))
			.enablePlugins(GinnyPlugin)
			.settings(commonSettings: _*)
			.aggregate(
				glfw
			)
			
		lazy val glfw = project
			.enablePlugins(GinnyPlugin)
			.settings(commonSettings: _*)
			.settings(
				ginnyRemotes += (
					new URL("https://github.com/glfw/glfw/archive/3.1.2.zip"),
					"glfw-3.1.2/",
					Seq(
						"glfw",
						"include/",
						"${GLFW_LIBRARIES}"
					)
				)
			)
		```
	* execute `sbt exit` to make sure that everything is configured correctly
	* execute `sbt ginnyScrape` and you should see an error! 

1. implement your plugin
	* as you edit your plugin-project sbt will rebuild it and redeploy it to your local repository
	* sbt reload (probably) won't watch for redeployment - so **you can't** do `sbt ~ginnyScrape` and get any meaningful result
		* you *could* do `sbt ~;reload;ginnyScrape` but you'd still need a source file or something
		* ... hammering the command through the CLI seems the easiest to me and I'm super-lazy
	