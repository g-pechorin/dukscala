SET SBT_OPTS="-Xmx64G -XX:+UseConcMarkSweepGC -XX:+CMSClassUnloadingEnabled -XX:MaxPermSize=2G -Xss2M  -Duser.timezone=GMT"
@java -jar %~dp0sbt-launch.jar %2 %*
