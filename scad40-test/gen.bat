@ECHO OFF

java -jar %~dp0..\sbt-launch.jar @scad40-launch.properties -i d40 -p com.peterlavalle.D40 gen/D40.hpp
