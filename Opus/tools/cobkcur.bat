echo off
if "%1"=="" goto usea
rem backup up to specified drive
echo Backing up all checked out project files in this directory to %1
status -l | sed -e "s/^/copy /" -e "s/$/ %1/" >bk$$.bat
goto runit
:usea
rem backup up to drive a:
echo Backing up all checked out project files in this directory to drive a:
status  -l | sed -e "s/^/copy /" -e "s/$/ a:/" >bk$$.bat
:runit
command /c bk$$
del bk$$.bat
