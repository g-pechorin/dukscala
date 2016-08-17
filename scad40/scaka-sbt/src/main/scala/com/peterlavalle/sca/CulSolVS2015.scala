package com.peterlavalle.sca

import java.io.File

import com.peterlavalle.sca.Cul.{Module, Solution}

import scala.xml.Unparsed

object CulSolVS2015 extends Cul.TSolver {

	override def apply(root: File, solution: Solution): Seq[File] =
		List(
			Sln,
			Filters,
			VCXProj
		).foldLeft(Seq[File]())((l, n) => l ++ n(root, solution))

	object Sln extends Cul.TSolver {
		override def apply(root: File, solution: Solution): Seq[File] =
			Seq(
				(root / s"${solution.name}.sln")
					.overWriter
					.append(
						"""
							|﻿
							|Microsoft Visual Studio Solution File, Format Version 12.00
							|# Visual Studio 14
							|VisualStudioVersion = 14.0.24720.0
							|MinimumVisualStudioVersion = 10.0.40219.1
						""".trimMargin
					)
					.mappend(solution.leafNodes) {
						case module: Cul.Module =>
							s"""
								 |Project("{${solution.HexString(8, 4, 4, 4)}") = "${module.name}", "${module.name}.vcxproj", "{${module.HexString(8, 4, 4, 4)}}"
								 |EndProject
							""".trimMargin
					}
					.append(
						"""
							|Global
							|	GlobalSection(SolutionConfigurationPlatforms) = preSolution
							|		Debug|Win32 = Debug|Win32
							|		Release|Win32 = Release|Win32
							|		Smallest|Win32 = Smallest|Win32
							|		Debug|x64 = Debug|x64
							|		Release|x64 = Release|x64
							|		Smallest|x64 = Smallest|x64
							|	EndGlobalSection
							|	GlobalSection(ProjectConfigurationPlatforms) = postSolution
						""".trimMargin
					)
					.mappend(solution.leafNodes.take(1)) {
						case module: Cul.Module =>
							s"""
								 |		{${module.HexString(8, 4, 4, 4)}}.Debug|Win32.ActiveCfg = Debug|Win32
								 |		{${module.HexString(8, 4, 4, 4)}}.Debug|Win32.Build.0 = Debug|Win32
								 |		{${module.HexString(8, 4, 4, 4)}}.Release|Win32.ActiveCfg = Release|Win32
								 |		{${module.HexString(8, 4, 4, 4)}}.Release|Win32.Build.0 = Release|Win32
								 |		{${module.HexString(8, 4, 4, 4)}}.Smallest|Win32.ActiveCfg = Smallest|Win32
								 |		{${module.HexString(8, 4, 4, 4)}}.Smallest|Win32.Build.0 = Smallest|Win32
								 |		{${module.HexString(8, 4, 4, 4)}}.Debug|x64.ActiveCfg = Debug|x64
								 |		{${module.HexString(8, 4, 4, 4)}}.Debug|x64.Build.0 = Debug|x64
								 |		{${module.HexString(8, 4, 4, 4)}}.Release|x64.ActiveCfg = Release|x64
								 |		{${module.HexString(8, 4, 4, 4)}}.Release|x64.Build.0 = Release|x64
								 |		{${module.HexString(8, 4, 4, 4)}}.Smallest|x64.ActiveCfg = Smallest|x64
								 |		{${module.HexString(8, 4, 4, 4)}}.Smallest|x64.Build.0 = Smallest|x64
							""".trimMargin
					}
					.append(
						"""
							|	EndGlobalSection
							|	GlobalSection(SolutionProperties) = preSolution
							|		HideSolutionNode = FALSE
							|	EndGlobalSection
							|EndGlobal
							|
						""".trimMargin
					)


					.closeFile
			)
	}

	object VCXProj extends Cul.TSolver {
		override def apply(root: File, solution: Solution): Seq[File] =
			solution.leafNodes.map {
				case module: Cul.Module =>

					val additionalIncludes =
						Unparsed(
							(module #:: module.transitiveDependencies.toStream)
								.flatMap(_.sources).map(_.root.AbsolutePath + ";")
								.foldLeft("<AdditionalIncludeDirectories>")(_ + _) + "$(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>"
						)

					// https://msdn.microsoft.com/en-us/library/dd293607.aspx

					(root / s"${module.name}.vcxproj")
						.overWriter
						.appand(
							<Project DefaultTargets="Build" ToolsVersion="14.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">

								<ItemGroup Label="ProjectConfigurations">
									<ProjectConfiguration Include="Debug|Win32">
										<Configuration>Debug</Configuration>
										<Platform>Win32</Platform>
									</ProjectConfiguration>
									<ProjectConfiguration Include="Release|Win32">
										<Configuration>Release</Configuration>
										<Platform>Win32</Platform>
									</ProjectConfiguration>
									<ProjectConfiguration Include="Smallest|Win32">
										<Configuration>Smallest</Configuration>
										<Platform>Win32</Platform>
									</ProjectConfiguration>

									<ProjectConfiguration Include="Debug|x64">
										<Configuration>Debug</Configuration>
										<Platform>x64</Platform>
									</ProjectConfiguration>
									<ProjectConfiguration Include="Release|x64">
										<Configuration>Release</Configuration>
										<Platform>x64</Platform>
									</ProjectConfiguration>
									<ProjectConfiguration Include="Smallest|x64">
										<Configuration>Smallest</Configuration>
										<Platform>x64</Platform>
									</ProjectConfiguration>
								</ItemGroup>

								<!--
									V$ will auto-gen thins if we don't ... so let's be consistent :P
								-->
								<PropertyGroup Label="Globals">
									<ProjectGuid>
										{s"{${module.HexString(8, 4, 4, 4)}}"}
									</ProjectGuid>{Unparsed(s"<ProjectName>${module.name}</ProjectName>")}
								</PropertyGroup>

								<Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props"/>

								<PropertyGroup Label="Configuration">
									<ConfigurationType>Application</ConfigurationType>
									<PlatformToolset>v140</PlatformToolset>
									<WholeProgramOptimization Condition="'$(Configuration)'!='Debug'">true</WholeProgramOptimization>
									<CharacterSet>MultiByte</CharacterSet>
									<UseOfMfc>false</UseOfMfc>
								</PropertyGroup>

								<PropertyGroup Condition="'$(Configuration)'=='Debug'" Label="Configuration Debug">
									<UseDebugLibraries>true</UseDebugLibraries>
								</PropertyGroup>

								<PropertyGroup Condition="'$(Configuration)'!='Debug'" Label="Configuration nDebug">
									<UseDebugLibraries>false</UseDebugLibraries>
								</PropertyGroup>

								<!-- import a big pile of hair -->
								<Import Project="$(VCTargetsPath)\Microsoft.Cpp.props"/>
								<ImportGroup Label="PropertySheets">
									<Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform"/>
								</ImportGroup>

								<!-- define a (less big but more visible) pile of hair ;) -->
								<PropertyGroup>
									{Unparsed(s"<IntDir>obj/$$(Platform)-$$(Configuration)-${module.name}/</IntDir>")}{Unparsed(s"<OutDir>bin/$$(Platform)-$$(Configuration)-${module.name}/</OutDir>")}
								</PropertyGroup>
								<ItemDefinitionGroup>
									<ClCompile>
										{additionalIncludes}<PrecompiledHeaderOutputFile/>
										<PrecompiledHeaderOutputFile/>
										<PrecompiledHeader>NotUsing</PrecompiledHeader>
										<WarningLevel>Level3</WarningLevel>
									</ClCompile>
									<Link>
										<SubSystem>Console</SubSystem>
									</Link>
								</ItemDefinitionGroup>

								<ItemDefinitionGroup Condition="'$(Configuration)'=='Debug'">
									<ClCompile>
										<Optimization>Disabled</Optimization>
										<PreprocessorDefinitions>_SCL_SECURE_NO_WARNINGS;_CRT_SECURE_NO_WARNINGS;WIN32;_DEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
									</ClCompile>
									<Link>
										<GenerateDebugInformation>true</GenerateDebugInformation>
									</Link>
								</ItemDefinitionGroup>

								<ItemDefinitionGroup Condition="'$(Configuration)'=='Release'">
									<ClCompile>
										<Optimization>MaxSpeed</Optimization>
										<FunctionLevelLinking>true</FunctionLevelLinking>
										<IntrinsicFunctions>true</IntrinsicFunctions>
										<PreprocessorDefinitions>_SCL_SECURE_NO_WARNINGS;_CRT_SECURE_NO_WARNINGS;WIN32;NDEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
										<RuntimeLibrary>MultiThreaded</RuntimeLibrary>
									</ClCompile>
									<Link>
										<EnableCOMDATFolding>true</EnableCOMDATFolding>
										<OptimizeReferences>true</OptimizeReferences>
									</Link>
								</ItemDefinitionGroup>

								<ItemDefinitionGroup Condition="'$(Configuration)'=='Smallest'">
									<ClCompile>
										<WarningLevel>Level3</WarningLevel>
										<Optimization>MinSpace</Optimization>
										<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>
										<RuntimeTypeInfo>false</RuntimeTypeInfo>
										<FunctionLevelLinking>true</FunctionLevelLinking>
										<IntrinsicFunctions>true</IntrinsicFunctions>
										<PreprocessorDefinitions>_SCL_SECURE_NO_WARNINGS;_CRT_SECURE_NO_WARNINGS;WIN32;NDEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
									</ClCompile>
									<Link>
										<GenerateDebugInformation>No</GenerateDebugInformation>
										<EnableCOMDATFolding>true</EnableCOMDATFolding>
										<OptimizeReferences>true</OptimizeReferences>
									</Link>
								</ItemDefinitionGroup>

								<!--
									this is the actual stuff we'll compile that makes up the project
								-->
								<ItemGroup>
									{module.allTransitiveSourceFiles.map(_.AbsolutePath)
									.filter(_.matches(".*\\.(c|cc|cpp)"))
									.map(source => <ClCompile Include={source}/>)}
								</ItemGroup>

								<!--
									this is just textfiles that're included in the editor for all to see
								-->
								<ItemGroup>
									{module.allTransitiveSourceFiles.map(_.AbsolutePath)
									.filter(_.matches(".*\\.(h|hh|hpp)"))
									.map(source => <ClInclude Include={source}/>)}
								</ItemGroup>

								<Import Project="$(VCTargetsPath)\Microsoft.Cpp.Targets"/>
							</Project>
						)
						.closeFile
			}
	}

	object Filters extends Cul.TSolver {

		private trait TModule {
			val value: Cul.Module

			def FilterNamed: Stream[(String, Iterable[File])] = {

				val neck: Stream[(String, Iterable[File])] =
					(value #:: value.transitiveDependencies.toStream)
						.flatMap {
							case module =>
								module.sources.toStream.flatMap {
									case source =>
										source.SubDirs.map {
											case sub =>

												val key =
													sub match {
														case "" => s"${module.name} ${source.name}"
														case _ =>
															s"${module.name} ${source.name}\\${sub.dropRight(1).replace('/', '\\')}".replaceAll("[^\\w/\\\\ ]+", "")
													}

												(key, source.contents.filter {
													case path =>
														path.startsWith(sub) && !path.substring(sub.length).contains("/")

												}.map(source.root / _))
										}
								}
						}

				neck ++ {
					val done = neck.map(_._1).toSet


					done.toStream.expansion {
						case next: String =>
							if (next.contains("\\"))
								Stream(next.substring(0, next.lastIndexOf('\\')))
							else
								Stream.Empty
					}.filterNot(done.contains).distinct.map {
						case name =>
							name -> Nil
					}
				}
			}
		}

		import scala.language.implicitConversions

		private implicit def wrapModule(module: Cul.Module): TModule =
			new TModule {
				override val value: Module = module
			}

		override def apply(root: File, solution: Solution): Seq[File] = {

			solution.leafNodes.map {
				case module: Cul.Module =>

					val named: Map[String, Stream[File]] =
						module.FilterNamed.foldLeft(Map[String, Stream[File]]()) {
							case (done: Map[String, Stream[File]], (name, value)) =>
								Map(
									name -> (done.contains(name) match {
										case false => value.toStream
										case true => done(name) ++ value
									})
								) ++ done.filterKeys(_ != name)
						}

					val filters =
						named.map {
							case (name, _) =>
								<Filter Include={name}>
									{Unparsed(s"<UniqueIdentifier>{${name.HexString(8, 4, 4, 4)}}</UniqueIdentifier>")}
								</Filter>
						}

					val sources =
						named.map {
							case (name, vals) =>

								val filter =
									("\t" * 12) + s"<Filter>${name}</Filter>"

								vals.map(_.AbsolutePath).map {
									case content if content.matches(".*\\.(c|cc|cpp)") =>
										<ClCompile Include={content}>
											{Unparsed(s"<Filter>${name}</Filter>")}
										</ClCompile>

									case content =>
										<ClInclude Include={content}>
											{Unparsed(s"<Filter>${name}</Filter>")}
										</ClInclude>
								}
						}

					(root / s"${module.name}.vcxproj.filters")
						.overWriter
						.appand(
							<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
								<ItemGroup>
									{filters}
								</ItemGroup>
								<ItemGroup>
									{sources}
								</ItemGroup>
							</Project>
						)
						.closeFile
			}
		}
	}

}
