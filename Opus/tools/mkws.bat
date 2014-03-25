echo off

if "%1" == "" goto usage

rem cleanup from previous runs
set voption=
set fmkwsall=
touch $$foo.tmp
del $$*.*

:commands

if "%1" == "-v" goto verbose
if "%1" == "?" goto usage
if "%1" == "-?" goto usage
if "%1" == "/?" goto usage
if "%1" == "#" goto dopath
if "%1" == "*" goto doall
if "%1" == "-" goto doremove
if "%1" == "+" goto doadd
if "%1" == "&" goto doaddsym
if "%1" == "" goto doit
goto usage


:verbose
echo on
set voption=yes
shift
goto commands


:dopath
set mkwspath=%2
shift
shift
goto commands



:doall

use > nul
if errorlevel 1 goto nonet
echo.
echo ***  Getting current checked out working state  ***
status > $$status.tmp
cd debug
status >> ..\$$status.tmp
cd ..\wordtech
status >> ..\$$status.tmp
cd ..
sed -n -e "s/^\(.*\.[c|C]\).*$/\1/p" $$status.tmp > $$files.tmp
set fmkwsall=ftrue

shift
goto commands



:doremove

shift
if "%1" == "?" goto usage
if "%1" == "#" goto dopath
if "%1" == "*" goto doall
if "%1" == "-" goto doremove
if "%1" == "+" goto doadd
if "%1" == "&" goto doaddsym
if "%1" == "" goto doit

fgrep -yv %1 $$files.tmp > $$files2.tmp
mv $$files2.tmp $$files.tmp
goto doremove



:doadd

shift
if "%1" == "?" goto usage
if "%1" == "#" goto dopath
if "%1" == "*" goto doall
if "%1" == "-" goto doremove
if "%1" == "+" goto doadd
if "%1" == "&" goto doaddsym
if "%1" == "" goto doit

echo %1 >> $$files.tmp
goto doadd


:doaddsym
shift
if "%1" == "?" goto usage
if "%1" == "#" goto dopath
if "%1" == "*" goto doall
if "%1" == "-" goto doremove
if "%1" == "+" goto doadd
if "%1" == "&" goto doaddsym
if "%1" == "" goto doit

cat %1 >> $$str1.tmp
goto doaddsym



:doit

if "%mkwspath%"=="" goto badpath

:doit2

sed -e "s/\(.*\)\.[c|C]/command \/c mkws2 %mkwspath%\\\1\.obj \$\$str1.tmp/p" $$files.tmp >> $$tmp.bat

echo.
echo ***  decoding obj's, please be patient!  ***
command /c $$tmp > nul
echo ***  sorting ***
echo. >> $$str1.tmp
sort $$str1.tmp > $$str2.tmp

if "%fmkwsall%"=="ftrue" goto doitall
if not exist opus.str goto doitall

echo.
echo *** incrementally stripping symbols from opus.str  ***
mv opus.str $$str3.tmp
symstrip $$str3.tmp $$str2.tmp opus.str
goto finis

:doitall
echo.
echo *** stripping symbols from opusfull.str; creating opus.str ***
symstrip opusfull.str $$str2.tmp opus.str
goto finis


:badpath

echo  ***  mkwspath not set!! assuming current directory. ***
echo   --  to set mkws path use \\'s (i.e. c:\\opusdbg)   --
goto doit2



:nonet

echo  ***  Network not started.  Please start net and try again! ***
goto finis



:usage

echo
echo		Usage:
echo.
echo    mkws [# path] [*] [- file1 file2 ...] [+ file1 file2 ...] [& file1 ...]
echo.
echo    #  ::  give the path of the BUILD directory.
echo.
echo    *  ::  create new working state using all checked out files.
echo.
echo    -  ::  remove named file(s) from the working state.
echo    +  ::  add named file(s) to the working state.    
echo    &  ::  add symbols listed in file(s) to working state.
echo.
echo	NOTE: use (-) only after the (*) option! Can not be done separately!
echo.
echo    mkws ---  Creates an opus.str from opusfull.str that does not contain
echo         any of the symbols from files in your current working state.
echo         Your current state is assumed to be all checked out files; from
echo         which you can add or subtract any additional files by using the
echo         + and - options.  Files can be added to your working state at a
echo         later time but files can only be removed from your working state
echo         when using the * option!
echo.


:finis
if "%voption%"=="" del $$*.*
