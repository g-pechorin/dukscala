@java -Xdebug -Xrunjdwp:transport=dt_socket,server=y,suspend=y,address=5005 -jar %~dp0sbt-launch.jar %2 %*
