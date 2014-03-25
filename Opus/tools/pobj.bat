echo off

if "%mprivate%"=="" set mprivate=f:

rem
rem set semaphore flags
rem 

if not exist %mprivate%\$takeinp goto setsem
echo.
echo someone is doing a takeobj.  If you do a putobj now
echo you will definitely hose them
echo.
echo please wait and try again
goto finis

:setsem
if not exist %mprivate%\$putinp touch %mprivate%\$putinp

rem warn if project is not fully in sync
cd %opusdrv%\cashmere

if .%1 == .-f goto forced
if .%1 == .-F goto forced

echo Displaying current project status... cashmere and its subdirectories
to %OPUS_DIR%
status -or | more
echo.
echo  You should not run PUTOBJ if you have out of sync or checked out files
echo  or broken links. If that is the case, please sync now and run putobj
echo  again.  ssync -rbf will fully update you. 
echo.
echo  If you choose to continue with out of sync files, be SURE to restore
echo  your machine to full sync when you are done, especially if mass-making
echo  on a foreign machine, like Berke or Milo.
echo.
query Continue (y/n)? 
if errorlevel 1 goto finis
echo.
goto noforce

:forced
shift

:noforce
rem if %1 if blank or -v use root directories
if "%1" == "" goto nouser
if "%1" == "-v" goto verbose
if "%1" == "-V" goto verbose

rem %1 assumed to be valid user name
if NOT "%2" == "" echo on
goto user

:verbose
echo on
if "%2"== "" goto nouser
rem else %2 is a valid user
shift

:user
cd %mprivate%\private\usr\%1
echo Put all objects in %mprivate%\private\usr\%1\obj
goto checknet

:nouser
cd %mprivate%\private
echo Put all objects in %mprivate%\private\obj

:checknet
rem  be sure net is up and directories are there
touch %mprivate%$$foo.tmp
if NOT exist %mprivate%$$foo.tmp goto nonet
rm %mprivate%$$foo.tmp

echo.
echo Delete previous objects then copy current objects.
echo.

to %BUILD_DIR%
echo y | del %mprivate%obj\*.*
rm -r %mprivate%obj
md %mprivate%obj
xcopy *.obj %mprivate%obj
xcopy *.res %mprivate%obj
xcopy *.hs %mprivate%obj
xcopy *.sdm %mprivate%obj
xcopy *.elx %mprivate%obj
cp opuscmd.asm %mprivate%obj
cp elxdefs.h elxinfo.h ibcm.h opuscmd.h rgbcm.h toolbox.h verdate*.h %mprivate%obj
cp sdm.lib  %mprivate%obj
cp opuscmd.rc opuscmd2.h opuscmd.asm %mprivate%obj
cp the.tbx %mprivate%obj
cp menuhelp.txt pcodemap.txt %mprivate%obj

echo These objects were put here by: > %mprivate%obj\whoput
setname | sed -n -e "s/ *COMPUTER NAME//p" >> %mprivate%obj\whoput
echo They were put here on: >> %mprivate%obj\whoput
when >> %mprivate%obj\whoput

echo.
echo Putobj Complete.
goto finis

:nonet
echo Net down or files not available. Check and rerun.

:finis
if exist %mprivate%\$putinp del %mprivate%\$putinp
to %START_DIR%
