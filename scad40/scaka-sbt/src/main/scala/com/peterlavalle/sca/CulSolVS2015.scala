package com.peterlavalle.sca

import java.io.File

import com.peterlavalle.sca.Cul.Solution

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

					// https://msdn.microsoft.com/en-us/library/dd293607.aspx

					(root / s"${module.name}.vcxproj")
						.overWriter
						.append(
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
									<CharacterSet>MultiByte</CharacterSet>

									<UseOfMfc>false</UseOfMfc>
								</PropertyGroup>


								<!-- import a big pile of hair -->
								<Import Project="$(VCTargetsPath)\Microsoft.Cpp.props"/>
								<ImportGroup Label="PropertySheets">
									<Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform"/>
								</ImportGroup>

								<!-- define a big pile of hair ;) -->

								<ItemDefinitionGroup Condition="'$(Configuration)'=='Debug'">
									<ClCompile>
										<PrecompiledHeaderOutputFile/>
										<PrecompiledHeader>NotUsing</PrecompiledHeader>
										<WarningLevel>Level3</WarningLevel>
										<Optimization>Disabled</Optimization>
										<PreprocessorDefinitions>WIN32;_DEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
									</ClCompile>
									<Link>
										<SubSystem>Console</SubSystem>
										<GenerateDebugInformation>true</GenerateDebugInformation>
									</Link>
								</ItemDefinitionGroup>

								<ItemDefinitionGroup Condition="'$(Configuration)'=='Release'">
									<ClCompile>
										<PrecompiledHeaderOutputFile/>
										<PrecompiledHeader>NotUsing</PrecompiledHeader>
										<WarningLevel>Level3</WarningLevel>
										<Optimization>MaxSpeed</Optimization>
										<FunctionLevelLinking>true</FunctionLevelLinking>
										<IntrinsicFunctions>true</IntrinsicFunctions>
										<PreprocessorDefinitions>WIN32;NDEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
										<RuntimeLibrary>MultiThreaded</RuntimeLibrary>
									</ClCompile>
									<Link>
										<EnableCOMDATFolding>true</EnableCOMDATFolding>
										<OptimizeReferences>true</OptimizeReferences>
										<SubSystem>Console</SubSystem>
									</Link>
								</ItemDefinitionGroup>

								<ItemDefinitionGroup Condition="'$(Configuration)'=='Smallest'">
									<ClCompile>
										<PrecompiledHeaderOutputFile/>
										<PrecompiledHeader>NotUsing</PrecompiledHeader>
										<WarningLevel>Level3</WarningLevel>
										<Optimization>MinSpace</Optimization>
										<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>
										<RuntimeTypeInfo>false</RuntimeTypeInfo>
										<FunctionLevelLinking>true</FunctionLevelLinking>
										<IntrinsicFunctions>true</IntrinsicFunctions>
										<PreprocessorDefinitions>WIN32;NDEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
									</ClCompile>
									<Link>
										<GenerateDebugInformation>No</GenerateDebugInformation>
										<EnableCOMDATFolding>true</EnableCOMDATFolding>
										<OptimizeReferences>true</OptimizeReferences>
										<SubSystem>Console</SubSystem>
									</Link>
								</ItemDefinitionGroup>

								<!--
									this is the actual stuff we'll compile that makes up the project
								-->
								<ItemGroup>
									{module.transitiveFiles.filter(_.getName.matches(".*\\.(c|cc|cpp)")).map(source => <ClCompile Include={source.AbsolutePath}/>
								)}
								</ItemGroup>

								<!--
									this is just textfiles that're included in the editor for all to see
								-->
								<ItemGroup>
									<ClInclude Include="main.h"/>
								</ItemGroup>

								<Import Project="$(VCTargetsPath)\Microsoft.Cpp.Targets"/>
							</Project>
						)
						.closeFile
			}
	}


	object Filters extends Cul.TSolver {
		override def apply(root: File, solution: Solution): Seq[File] =
			solution.leafNodes.map {
				case module: Cul.Module =>

					val filters =
						module.sources.toStream.map {
							case source =>
								<Filter Include={source.name}>
									<UniqueIdentifier>
										{source.HexString(8, 4, 4, 4)}
									</UniqueIdentifier>
								</Filter>
						}

					val sources =
						module.sources.flatMap {
							case source =>
									source.contents.map {
										case content if content.matches(".*\\.(c|cc|cpp)") =>
											<ClCompile Include={(source.root / content).AbsolutePath.replace('/', '\\')}>
												<Filter>
													{source.name}
												</Filter>
											</ClCompile>
										case content =>
											<ClInclude Include={(source.root / content).AbsolutePath.replace('/', '\\')}>
												<Filter>
													{source.name}
												</Filter>
											</ClInclude>
									}


						}

					(root / s"${module.name}.vcxproj.filters")
						.overWriter
						.append(
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
