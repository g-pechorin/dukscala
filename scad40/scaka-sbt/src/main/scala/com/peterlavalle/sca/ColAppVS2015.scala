package com.peterlavalle.sca

import java.io.{File, StringWriter}

import scala.language.implicitConversions

object ColAppVS2015 extends Col.TSolver {
	System.err.println("TODO ; Respect filtering for the TSource things - stop DIY crawling")

	type FilterGroup = (File, (String, Set[File]))

	override def emit(target: File, modulesIsApp: Stream[Col.Module]): Set[File] =
		Filters.emit(target, modulesIsApp) ++ Project.emit(target, modulesIsApp)

	implicit def wrappedModule(value: Col.Module): TWrappedModule =
		new TWrappedModule {
			val module = value
		}

	implicit def wrappedFile(value: File): TWrappedFile =
		new TWrappedFile {
			val file = value
		}

	def makeFIlterGroups(next: Col.TSource): Stream[FilterGroup] = {
		def recurFilterGroup(path: String, root: File): Stream[FilterGroup] =
			root.streamFiles.toList match {
				case Nil =>
					Stream.Empty

				case list =>
					(root, (path, list.filter(_.isFile).toSet)) #:: list.toStream.filter(_.isDirectory).flatMap {
						case next: File =>
							recurFilterGroup(path + "/" + next.getName, next)
					}
			}

		next match {
			case remote: Col.Remote =>
				recurFilterGroup(remote.url.toString.split("/").last, remote.home)

			case Col.Folder(home: File) =>
				recurFilterGroup(home.getName, home)
		}
	}

	trait TWrappedModule {
		val module: Col.Module

		def ProjectGuid: String =
			"{%08X-5D02-4C97-BBE8-58EE8797EB8A}".format(Math.abs(module.hashCode()))
	}

	trait TWrappedFile {
		val file: File

		def UniqueIdentifier: String =
			"{%08x-90e4-414a-bb0c-7a4a0a4148d5}".format(Math.abs(file.hashCode()))
	}

	object Filters extends Col.TSolver {
		override def emit(target: File, modules: Stream[Col.Module]): Set[File] =
			modules.map {
				case Col.Module(name: String, roots: Seq[Col.TSource], linked: Seq[Col.Module]) =>

					val folders: List[FilterGroup] =
						roots.map(makeFIlterGroups)
							.toSet.flatten
							.toList.sortBy(_._1.AbsolutePath).sortBy(_._1.getName)

					(target / s"${name}.vcxproj.filters")
						.overWriter
						.append(
							"""
								|<?xml version="1.0" encoding="utf-8"?>
								|<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
								|	<ItemGroup>
							""".stripMargin.trim + '\n'
						)
						.mappend(folders) {
							case (file: File, (path: String, _)) =>
								s"""
									 |		<Filter Include="${path.replace('/', '\\')}">
									 |			<UniqueIdentifier>${file.UniqueIdentifier}</UniqueIdentifier>
									 |		</Filter>
								""".stripMargin.trim + '\n'
						}
						.append(
							"""
								|	</ItemGroup>
								|	<ItemGroup>
							""".stripMargin.trim + '\n'
						)
						.mappend(folders) {
							case (_, (path: String, srcs: Set[File])) =>
								new StringWriter()
									.mappend(srcs) {
										case src: File if src.getName.matches(".*\\.c(c|pp)?") =>
											s"""
												 |		<ClCompile Include="${src.AbsolutePath}">
												 |			<Filter>${path.replace('/', '\\')}</Filter>
												 |		</ClCompile>
											""".stripMargin.trim + '\n'

										case src: File if src.getName.matches(".*\\.h(h|pp)?") || src.getName.matches(".*\\.in[cl]") =>
											s"""
												 |		<ClInclude Include="${src.AbsolutePath}">
												 |			<Filter>${path.replace('/', '\\')}</Filter>
												 |		</ClInclude>
											""".stripMargin.trim + '\n'

										case _ =>
											""
									}
									.toString
						}
						.append(
							"""
								|	</ItemGroup>
								|</Project>
							""".stripMargin.trim + '\n'
						)
						.closeFile
			}.toSet
	}

	object Project extends Col.TSolver {
		/**
			*
			* @param target  where all files should be created
			* @param modules (all modules to make, ???)
			* @return a set of all files created
			*/
		override def emit(target: File, modules: Stream[Col.Module]): Set[File] =
		Set(
			(target / s"Solution2015.sln")
				.overWriter
				.append(
					"""
						|Microsoft Visual Studio Solution File, Format Version 12.00
						|# Visual Studio 14
						|VisualStudioVersion = 14.0.23107.0
						|MinimumVisualStudioVersion = 10.0.40219.1
					""".trimMargin
				)
				.mappend(modules) {
					case module: Col.Module =>
						s"""
							 |Project("{8BC9CEB8-8B4A-11D0-8D11-00${"%08X".format(Math.abs(module.hashCode()))}42}") = "${module.name}", "${module.name}.vcxproj", "${module.ProjectGuid}"
							""".trimMargin
				}
				.append(
					"""
						|EndProject
						|Global
						|	GlobalSection(SolutionConfigurationPlatforms) = preSolution
						|		Debug|x64 = Debug|x64
						|		Debug|Win32 = Debug|Win32
						|		Release|x64 = Release|x64
						|		Release|Win32 = Release|Win32
						|	EndGlobalSection
						|	GlobalSection(ProjectConfigurationPlatforms) = postSolution
					""".trimMargin
				)
				.mappend(modules) {
					case module: Col.Module =>
						s"""
							 |		${module.ProjectGuid}.Debug|x64.ActiveCfg = Debug|x64
							 |		${module.ProjectGuid}.Debug|x64.Build.0 = Debug|x64
							 |		${module.ProjectGuid}.Debug|Win32.ActiveCfg = Debug|Win32
							 |		${module.ProjectGuid}.Debug|Win32.Build.0 = Debug|Win32
							 |		${module.ProjectGuid}.Release|x64.ActiveCfg = Release|x64
							 |		${module.ProjectGuid}.Release|x64.Build.0 = Release|x64
							 |		${module.ProjectGuid}.Release|Win32.ActiveCfg = Release|Win32
							 |		${module.ProjectGuid}.Release|Win32.Build.0 = Release|Win32
							""".trimMargin
				}
				.append(
					"""
						|	EndGlobalSection
						|	GlobalSection(SolutionProperties) = preSolution
						|		HideSolutionNode = FALSE
						|	EndGlobalSection
						|EndGlobal
					""".trimMargin
				)
				.closeFile
		) ++ modules.map {
			case module: Col.Module =>

				val isApp =
					module.artifact match {
						case Col.Module.Static => false
						case Col.Module.Binary => true
					}


				val files: Set[File] =
					module.roots.flatMap(makeFIlterGroups).flatMap { case (_: File, (_: String, files: Set[File])) => files }.toSet

				val srcs =
					files.filter(_.getName.matches(".*\\.c(c|pp)?")).map(_.AbsolutePath).toList.sorted
				val incs =
					files.filter(_.getName.matches(".*\\.h(h|pp)?")).map(_.AbsolutePath).toList.sorted

				val includes: String =
					module.fullSet.toList
						.flatMap(_.roots).map(_.home)
						.map((include: File) => s"<IncludePath>${include.AbsolutePath};$$(IncludePath)</IncludePath>")
						.foldLeft("")(_ + "\n\t\t" + _)

				(target / s"${module.name}.vcxproj")
					.overWriter
					.append(
						raw"""
							 |<?xml version="1.0" encoding="utf-8"?>
							 |<Project DefaultTargets="Build" ToolsVersion="14.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
							 |	<ItemGroup Label="ProjectConfigurations">
							 |		<ProjectConfiguration Include="Debug|Win32">
							 |			<Configuration>Debug</Configuration>
							 |			<Platform>Win32</Platform>
							 |		</ProjectConfiguration>
							 |		<ProjectConfiguration Include="Release|Win32">
							 |			<Configuration>Release</Configuration>
							 |			<Platform>Win32</Platform>
							 |		</ProjectConfiguration>
							 |		<ProjectConfiguration Include="Debug|x64">
							 |			<Configuration>Debug</Configuration>
							 |			<Platform>x64</Platform>
							 |		</ProjectConfiguration>
							 |		<ProjectConfiguration Include="Release|x64">
							 |			<Configuration>Release</Configuration>
							 |			<Platform>x64</Platform>
							 |		</ProjectConfiguration>
							 |	</ItemGroup>
							 |	<PropertyGroup Label="Globals">
							 |		<ProjectGuid>${module.ProjectGuid}</ProjectGuid>
							 |		<Keyword>Win32Proj</Keyword>
							 |		<RootNamespace>${module.name}</RootNamespace>
							 |		<WindowsTargetPlatformVersion>8.1</WindowsTargetPlatformVersion>
							 |	</PropertyGroup>
							 |	<Import Project="$$(VCTargetsPath)/Microsoft.Cpp.Default.props" />
							 |	<PropertyGroup Condition="'$$(Configuration)|$$(Platform)'=='Debug|Win32'" Label="Configuration">
							 |		<ConfigurationType>${if (isApp) "Application" else "StaticLibrary"}</ConfigurationType>
							 |		<UseDebugLibraries>true</UseDebugLibraries>
							 |		<PlatformToolset>v140</PlatformToolset>
							 |		<CharacterSet>MultiByte</CharacterSet>${includes}
							 |		<LinkIncremental>true</LinkIncremental>
							 |		<IntDir>$$(Platform)/$$(Configuration)$$(ProjectName)/</IntDir>
							 |	</PropertyGroup>
							 |	<PropertyGroup Condition="'$$(Configuration)|$$(Platform)'=='Release|Win32'" Label="Configuration">
							 |		<ConfigurationType>${if (isApp) "Application" else "StaticLibrary"}</ConfigurationType>
							 |		<UseDebugLibraries>false</UseDebugLibraries>
							 |		<PlatformToolset>v140</PlatformToolset>
							 |		<WholeProgramOptimization>true</WholeProgramOptimization>
							 |		<CharacterSet>MultiByte</CharacterSet>${includes}
							 |		<LinkIncremental>false</LinkIncremental>
							 |		<IntDir>$$(Platform)/$$(Configuration)$$(ProjectName)/</IntDir>
							 |	</PropertyGroup>
							 |	<PropertyGroup Condition="'$$(Configuration)|$$(Platform)'=='Debug|x64'" Label="Configuration">
							 |		<ConfigurationType>${if (isApp) "Application" else "StaticLibrary"}</ConfigurationType>
							 |		<UseDebugLibraries>true</UseDebugLibraries>
							 |		<PlatformToolset>v140</PlatformToolset>
							 |		<CharacterSet>MultiByte</CharacterSet>${includes}
							 |		<LinkIncremental>true</LinkIncremental>
							 |		<IntDir>$$(Platform)/$$(Configuration)$$(ProjectName)/</IntDir>
							 |	</PropertyGroup>
							 |	<PropertyGroup Condition="'$$(Configuration)|$$(Platform)'=='Release|x64'" Label="Configuration">
							 |		<ConfigurationType>${if (isApp) "Application" else "StaticLibrary"}</ConfigurationType>
							 |		<UseDebugLibraries>false</UseDebugLibraries>
							 |		<PlatformToolset>v140</PlatformToolset>
							 |		<WholeProgramOptimization>true</WholeProgramOptimization>
							 |		<CharacterSet>MultiByte</CharacterSet>${includes}
							 |		<LinkIncremental>false</LinkIncremental>
							 |		<IntDir>$$(Platform)/$$(Configuration)$$(ProjectName)/</IntDir>
							 |	</PropertyGroup>
							 |	<Import Project="$$(VCTargetsPath)/Microsoft.Cpp.props" />
							 |	<ImportGroup Label="ExtensionSettings">
							 |	</ImportGroup>
							 |	<ImportGroup Label="Shared">
							 |	</ImportGroup>
							 |
							 |  <ImportGroup Label="PropertySheets" Condition="'$$(Configuration)|$$(Platform)'=='Debug|Win32'">
							 |    <Import Project="$$(UserRootDir)\Microsoft.Cpp.$$(Platform).user.props" Condition="exists('$$(UserRootDir)\Microsoft.Cpp.$$(Platform).user.props')" Label="LocalAppDataPlatform" />
							 |  </ImportGroup>
							 |  <ImportGroup Label="PropertySheets" Condition="'$$(Configuration)|$$(Platform)'=='Release|Win32'">
							 |    <Import Project="$$(UserRootDir)\Microsoft.Cpp.$$(Platform).user.props" Condition="exists('$$(UserRootDir)\Microsoft.Cpp.$$(Platform).user.props')" Label="LocalAppDataPlatform" />
							 |  </ImportGroup>
							 |  <ImportGroup Label="PropertySheets" Condition="'$$(Configuration)|$$(Platform)'=='Debug|x64'">
							 |    <Import Project="$$(UserRootDir)\Microsoft.Cpp.$$(Platform).user.props" Condition="exists('$$(UserRootDir)\Microsoft.Cpp.$$(Platform).user.props')" Label="LocalAppDataPlatform" />
							 |  </ImportGroup>
							 |  <ImportGroup Label="PropertySheets" Condition="'$$(Configuration)|$$(Platform)'=='Release|x64'">
							 |    <Import Project="$$(UserRootDir)\Microsoft.Cpp.$$(Platform).user.props" Condition="exists('$$(UserRootDir)\Microsoft.Cpp.$$(Platform).user.props')" Label="LocalAppDataPlatform" />
							 |  </ImportGroup>
							 |
							 |	<PropertyGroup Label="UserMacros" />
							 |
							 |	<ItemDefinitionGroup Condition="'$$(Configuration)|$$(Platform)'=='Debug|Win32'">
							 |		<ClCompile>
							 |			<PrecompiledHeaderOutputFile />
							 |			<PrecompiledHeader/>
							 |			<WarningLevel>Level3</WarningLevel>
							 |			<Optimization>Disabled</Optimization>
							 |			<PreprocessorDefinitions>COL_TARGET="${target.AbsolutePath}";WIN32;_DEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
							 |		</ClCompile>
							 |		<Link>
							 |			<SubSystem>Console</SubSystem>
							 |			<GenerateDebugInformation>true</GenerateDebugInformation>
							 |		</Link>
							 |	</ItemDefinitionGroup>
							 |	<ItemDefinitionGroup Condition="'$$(Configuration)|$$(Platform)'=='Debug|x64'">
							 |		<ClCompile>
							 |			<PrecompiledHeaderOutputFile />
							 |			<PrecompiledHeader/>
							 |			<WarningLevel>Level3</WarningLevel>
							 |			<Optimization>Disabled</Optimization>
							 |			<PreprocessorDefinitions>COL_TARGET="${target.AbsolutePath}";WIN64;_DEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
							 |		</ClCompile>
							 |		<Link>
							 |			<SubSystem>Console</SubSystem>
							 |			<GenerateDebugInformation>true</GenerateDebugInformation>
							 |		</Link>
							 |	</ItemDefinitionGroup>
							 |	<ItemDefinitionGroup Condition="'$$(Configuration)|$$(Platform)'=='Release|Win32'">
							 |		<ClCompile>
							 |			<PrecompiledHeaderOutputFile />
							 |			<PrecompiledHeader/>
							 |			<WarningLevel>Level3</WarningLevel>
							 |			<Optimization>MaxSpeed</Optimization>
							 |			<FunctionLevelLinking>true</FunctionLevelLinking>
							 |			<IntrinsicFunctions>true</IntrinsicFunctions>
							 |			<PreprocessorDefinitions>COL_TARGET="${target.AbsolutePath}";WIN32;NDEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
							 |		</ClCompile>
							 |		<Link>
							 |			<EnableCOMDATFolding>true</EnableCOMDATFolding>
							 |			<OptimizeReferences>true</OptimizeReferences>
							 |			<SubSystem>Console</SubSystem>
							 |			<GenerateDebugInformation>true</GenerateDebugInformation>
							 |		</Link>
							 |	</ItemDefinitionGroup>
							 |	<ItemDefinitionGroup Condition="'$$(Configuration)|$$(Platform)'=='Release|x64'">
							 |		<ClCompile>
							 |			<PrecompiledHeaderOutputFile />
							 |			<PrecompiledHeader/>
							 |			<WarningLevel>Level3</WarningLevel>
							 |			<Optimization>MaxSpeed</Optimization>
							 |			<FunctionLevelLinking>true</FunctionLevelLinking>
							 |			<IntrinsicFunctions>true</IntrinsicFunctions>
							 |			<PreprocessorDefinitions>COL_TARGET="${target.AbsolutePath}";WIN64;NDEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
							 |		</ClCompile>
							 |		<Link>
							 |			<EnableCOMDATFolding>true</EnableCOMDATFolding>
							 |			<OptimizeReferences>true</OptimizeReferences>
							 |			<SubSystem>Console</SubSystem>
							 |			<GenerateDebugInformation>true</GenerateDebugInformation>
							 |		</Link>
							 |	</ItemDefinitionGroup>
							 |
							 |
						""".stripMargin.trim + '\n'
					)
					.mappend(srcs)(src => "\t" +s"""<ItemGroup><ClCompile Include="$src" /></ItemGroup>""" + "\n")
					.mappend(incs)(inc => "\t" +s"""<ItemGroup><ClInclude Include="$inc" /></ItemGroup>""" + "\n")
					.append(
						s"""
							 |	<Import Project="$$(VCTargetsPath)/Microsoft.Cpp.targets" />
							 |	<ImportGroup Label="ExtensionTargets">
							 |	</ImportGroup>
							 |</Project>
							""".stripMargin.trim + '\n'
					)
					.closeFile
		}.toSet
	}

}
