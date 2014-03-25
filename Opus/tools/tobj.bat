echo off				 

rem OPTIONS:
rem -v  verbose (leaves echo on)
rem -s  synced  (does not delete any objects after copy)
rem -d  just remove obj of currently checked out file (don't take obj's)

if "%1"=="-d" goto DoSed

if "%mprivate%"=="" set mprivate=f:

rem 
rem set semaphore flags

if not exist %mprivate%\$putinp goto setsem
echo.
echo a putobj is in progress by someone else 
echo please wait and try again.
goto finis

:setsem
if not exist %mprivate%\$takeinp touch %mprivate%\$takeinp

to %opus_dir%

rem root is default
cd %mprivate%\private


if "%1"=="" goto message
if "%1"=="-s" goto check2
if "%1"=="-v" goto verbose1
rem else %1 assumed to be user
cd %mprivate%\private\usr\%1
goto check2

:verbose1
echo on
goto check2

:check2
if "%2"=="" goto message
if "%2"=="-v" goto verbose2
if "%2"=="-s" goto check3
rem else %2 assumed to be user
cd %mprivate%\private\usr\%2
goto check3

:verbose2
echo on
goto check3

:check3
if "%3"=="" goto message
if "%3"=="-v" goto verbose3
if "%3"=="-s" goto message
rem else %3 assumend to be user
cd %mprivate%\private\usr\%3
goto message

:verbose3
echo on
goto message


:message
echo Take all objects in:
cd %mprivate%
echo Touch them, and delete unwanted objects (except with -s).
echo.

rem  be sure net is up and files are there
if NOT exist %mprivate%obj\*.obj goto nonet

if NOT exist %mprivate%obj\whoput goto nowho
cat %mprivate%obj\whoput
echo.
query Continue (y/n)? 
if errorlevel 1 goto finis
echo.

:nowho

echo.
echo Delete existing objects, Copy & touch new objects, 
echo then delete undesired objects.

to %build_dir%
del *.obj
del *.res
del *.hs
del *.sdm
del *.elx
del opuscmd.asm
rm elxdefs.h elxinfo.h ibcm.h opuscmd.h rgbcm.h toolbox.h verdate.h
rm sdm.lib el.lib
del opuscmd.rc
del the.tbx
rm menuhelp.txt pcodemap.txt

echo.
echo (please ignore the messages from touch that follow...)
touch %mprivate%obj\*.*
xcopy %mprivate%obj\*.* .
del whoput

rem HANDLE SPECIAL CASES

rem -s argument means that the user knows that the obj's are in sync with him.
if "%1"=="-s" goto OptionS
if "%2"=="-s" goto OptionS
if "%3"=="-s" goto OptionS

:DoSed
to %opus_dir%
echo.
echo Deleting all objects of checked out project .c, .asm, .cmd, .rc and
echo   .txt files in cashmere directory and its subdirectories.

rem tobj.sed is a sed script which converts "any thing.c" and
rem   "any thing.asm" to "del any thing.obj".  A list of all "path\file.rc" 
rem   lines is written to $$res.snt in the form "del path\opus.res".  In the
rem   same manner all "path\file.cmd" are place into $$cmd.snt as 
rem   "del path\opuscmd.obj" and into $$res.snt as "del path\opus.res".  And
rem   all "path\pmap.txt" are converted to "del path\pcodemap.obj" in $$pmap.snt.
rem   Other "path\file.txt" are converted to "del path\file.obj" in $$tmp.bat.

if exist %build_dir%\$$res.snt del %build_dir%\$$res.snt
status -rl | sed -n -f %opus_dir%\tools\tobj.sed > %build_dir%\$$tmp.bat

to %build_dir%

if not exist %build_dir%\$$res.snt goto NoDelRes
sed -n "1 p" %build_dir%\$$res.snt >> %build_dir%\$$tmp.bat
del %build_dir%\$$res.snt
:NoDelRes
if not exist %build_dir%\$$cmd.snt goto NoDelCmd
sed -n "1 p" %build_dir%\$$cmd.snt >> %build_dir%\$$tmp.bat
del %build_dir%\$$cmd.snt
:NoDelCmd
if not exist %build_dir%\$$pmap.snt goto NoDelTxt
sed -n "1 p" %build_dir%\$$pmap.snt >> %build_dir%\$$tmp.bat
del %build_dir%\$$pmap.snt
:NoDelTxt

if not "%1"=="-d" goto DoTmpBat
more %build_dir%\$$tmp.bat
pause
:DoTmpBat
command /c %build_dir%\$$tmp.bat 
del %build_dir%\$$tmp.bat

to %resource_dir%
touch %build_dir%\$$tmp.0
status -l > %build_dir%\$$tmp.1
to %build_dir%
diff $$tmp.0 $$tmp.1 > NUL
if not errorlevel 1 goto resOK
echo.
echo Deleting old opuscmd.rc and opuscmd.obj to resource build.
rm opuscmd.rc opuscmd.obj

:resOK
rm $$tmp.0 $$tmp.1

echo.
echo Deleting old .ilk to insure full link.

del opus.ilk

:done
echo.
echo Ok, done. You may make now. Restore current directory if needed.
goto finis

:OptionS
echo.
echo Do you have your goggles on?
echo NOT deleting objects of checked out files!!
echo Re-execute if you didn't mean to do this. (Or delete them yourself).
echo.
goto done

:nonet
echo Net down or files not available. Check and rerun.

:finis
if exist %mprivate%\$takeinp del %mprivate%\$takeinp
to %START_DIR%

