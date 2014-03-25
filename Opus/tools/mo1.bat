echo off
set MAKEOPUS=
set PATH=Q:\SRC\OPUS\TOOLS\tools\dos;Q:\SRC\OPUS\TOOLS\tools
set LIB=Q:\SRC\OPUS\TOOLS\lib
set INCLUDE=Q:\SRC\OPUS\TOOLS;Q:\SRC\OPUS\TOOLS\wordtech;Q:\SRC\OPUS\TOOLS\lib;Q:\SRC\OPUS\TOOLS\asm
touch error.snt
del error.snt
echo rem > mo2.bat
echo echo off >> mo2.bat
NMAKE -nc @makeopus.cm -f Q:\SRC\OPUS\TOOLS\makeopus /?  >> mo2.bat
if errorlevel 1 goto error
echo goto done >> mo2.bat
echo :error >> mo2.bat
echo makeerr Q:\SRC\OPUS\TOOLS\error.snt >> mo2.bat
echo :done >> mo2.bat
grep "is up-to-date" mo2.bat > NUL
if not errorlevel 1 goto skip
mo2
:error
makeerr Q:\SRC\OPUS\TOOLS\error.snt
:skip
