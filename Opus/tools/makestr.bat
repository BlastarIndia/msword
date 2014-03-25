echo off

if "%1"=="" goto usage
if "%2"=="" goto usage
if "%3"=="" goto usage
if "%1"=="+" goto add
if "%1"=="-" goto subtract

:add
cat opus.str %2 | sort | uniq > %3
goto finis

:subtract
rem only strip if full line matches
sed "s/$/$/" %2 > foo$$
dsrgrep -vf foo$$ opus.str > %3
rm foo$$
goto finis

:usage
echo usage:  makestr [+|-] file1 file2
echo         + combine opus.str with file1 to make file2
echo         - weed out lines in file1 from opus.str to make file2

:finis
