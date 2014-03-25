echo off

if not exist tools\revcnt.lst goto usage

if not "%1"=="-i" goto dogrep
shift
goto count

:dogrep
echo grepping...
del countrev.lst
grep [^A-Z,a-z]REVIEW *.* debug\*.* wordtech\*.* interp\*.* lib\*.h resource\*.* > countrev.lst

:count
if not exist countrev.lst goto usage
echo Summary...
echo REVIEW Count > countrev.out
revcnt tools\revcnt.lst countrev.lst >> countrev.out
if not "%1"=="" goto loop
goto finis

:again
shift
:loop
if "%1"=="" goto finis
if "%1"=="-u" goto unknown
rem else name
echo %1...
echo REVIEWs for %1 >> countrev.out
revcnt tools\revcnt.lst countrev.lst -n%1 >> countrev.out
goto again

:unknown
echo Unknown...
echo UNKNOWN REVIEWs >> countrev.out
revcnt tools\revcnt.lst countrev.lst -u >> countrev.out
echo ????...
echo REVIEWs for ???? >> countrev.out
revcnt tools\revcnt.lst countrev.lst -n???? >> countrev.out
goto again

:usage
echo usage: countrev [-i] [-u] [name ...]
echo   Must be executed from project root directory.  Assumes standard 
echo     placement of wordtech project (subdirectoy of cashmere root).
echo   -i -- incremental: do not perform grep (must have already been done)
echo   -u -- unknown (also ???? and win)
echo   name -- REVIEWs for name

:finis
