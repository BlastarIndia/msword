echo off
if exist opusfull.map cp opusfull.map opus.map
if not exist opusfull.map cp opus.map opusfull.map

if "%1"=="" goto opus

echo Stripping opus.map using %1...
strpmap /e opus.map ~opusmap.tmp %1
goto common

:opus
echo Stripping opus.map using opus.str...
strpmap /e opus.map ~opusmap.tmp opus.str
goto common

:common
echo Building opus.sym...
mv ~opusmap.tmp opus.map
mapsym -s opus
