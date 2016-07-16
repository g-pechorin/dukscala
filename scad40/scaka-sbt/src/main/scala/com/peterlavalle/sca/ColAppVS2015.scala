package com.peterlavalle.sca

import java.io.File
import java.io.{File, StringWriter}

import com.peterlavalle.sca.Col.Folder

object ColAppVS2015 extends Col.TSolver {
	override def emit(target: File, modulesIsApp: Stream[(Col.Module, Boolean)]): Set[File] =
		Filters.emit(target, modulesIsApp)


	object Filters extends Col.TSolver {
		override def emit(target: File, modulesIsApp: Stream[(Col.Module, Boolean)]): Set[File] =
			modulesIsApp.map {
				case (Col.Module(name: String, roots: Seq[Col.TSource], linked: Seq[Col.Module]), isApp: Boolean) =>

					type FilterGroup = (File, (String, Set[File]))

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
								|  <ItemGroup>
							""".stripMargin
						)
						.mappend(folders) {
							case (file: File, (path: String, _)) =>
								s"""
									 |    <Filter Include="${path}">
									 |      <UniqueIdentifier>{${"%08x".format(Math.abs(file.AbsolutePath.hashCode))}-90e4-414a-bb0c-7a4a0a4148d5}</UniqueIdentifier>
									 |    </Filter>
							""".stripMargin
						}
						.append(
							"""
								|  </ItemGroup>
								|  <ItemGroup>
							""".stripMargin
						)
						.mappend(folders) {
							case (_, (path: String, srcs: Set[File])) =>
								new StringWriter()
									.mappend(srcs) {
										case src: File =>
											s"""
												|    <ClCompile Include="${src.AbsolutePath}">
												|      <Filter>${path}</Filter>
												|    </ClCompile>
											""".stripMargin
									}
									.toString
						}
						.append(
							"""
								|  </ItemGroup>
								|</Project>
							""".stripMargin
						)
						.closeFile
			}.toSet
	}

}
