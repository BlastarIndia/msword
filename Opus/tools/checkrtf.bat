echo off
echo RTF Test Suite

if "%1"=="-?" goto usage

touch ~temp.lst ~temp.bat   
rm ~temp.lst ~temp.bat   

if not "%1" == "-v" goto notverbose
set tverbose=yes
echo on
shift
:notverbose

if not "%1"=="-p" goto nopath
set topuspath=%2
shift
shift
:nopath

if "%1"=="-d" goto doc
if "%1"=="-r" goto rtf
if "%1"=="-c" goto con
goto usage

:doc
rem -d specified
if not exist %opusdrv%\cashmere\tools\checkrtf.sed goto nosed
shift
if not "%1"=="" goto loopdoc
for %%f in (*.doc) do win %topuspath%opus /d %%f /b%%f
goto docdone

rem
rem list only files that are inputted

:loopdoc
if "%1"=="" goto sedconvert
win %topuspath%opus /d %1 /b%1
ls -sqv %1.doc >> ~temp.lst
shift
goto loopdoc

:docdone
rem diff the output (.rt1 & .rt2)
ls -sqv *.doc >> ~temp.lst
:sedconvert
rem
rem need this echo off to prevent garbage in the output file
echo echo off > ~temp.bat
sed -f %opusdrv%\cashmere\tools\checkrtf.sed ~temp.lst >> ~temp.bat
command /c ~temp.bat > checkrtf.out
if not "%tverbose%" == "yes" rm ~temp.lst ~temp.bat   
echo    DIFF output in checkrtf.out.
goto complete

:rtf
rem -r specified
shift
if not "%1"=="" goto looprtf
for %%f in (*.rtf) do win %topuspath%opus /r %%f /b%1
goto complete

:looprtf
if "%1"=="" goto complete
win %topuspath%opus /r %1 /b%1
shift
goto looprtf

:complete
echo    Checkrtf complete.
echo.
echo    Thank you and have a nice day.
rem
rem  if verbose then show the output immediately
rem
if not "%tverbose%" == "yes" goto finis
echo.
echo  *** Result is as follows .... ***
if exist checkrtf.out more checkrtf.out
goto finis

:nosed
echo  cannot find %opusdrv%\cashmere\tools\checkrtf.sed
if "%opusdrv%"=="" echo  opusdrv environment not set.
echo  Cannot continue.
goto finis

:con
rem -c specified
shift
if not "%1"=="" goto loopcon
for %%f in (*.doc) do win %topuspath%opus /c %%f /b%%f
goto finis

rem
rem list only files that are inputted

:loopcon
if "%1"=="" goto finis
win %topuspath%opus /c %1 /b%1
shift
goto loopcon

:usage
echo Usage: checkrtf [-v] [-p exec-path] -rdc [files...]
echo            -r : convert .rtf to .doc (*.rtf if no files)
echo            -d : create .rt1 and .rt2 from .doc  (*.doc if no files)
echo            -c : create .rtf from .doc  (*.doc if no files)

echo            exec-path is where to find opus, must end with \ as in
echo                  -p %opusdrv%\cashmere\
echo            files: 
echo              if specifying .doc files for -d or -c, LEAVE OFF extension
echo              DO NOT use wild card filenames. Specify no files for *.doc
echo               or *.rtf.
goto finis

:finis
set topuspath=
set tverbose=
