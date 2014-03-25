echo off

if not exist %1 goto noname

cch %1 > before.tmp
cbopus %1 > ~cbopus.tmp
cch ~cbopus.tmp > after.tmp

diff before.tmp after.tmp > nul

if not errorlevel 1 goto move
echo cbO: Chars removed or added! Changes not saved: %1
echo cbO: Chars removed or added! Changes not saved: %1 >> cbopus.err
goto finis

:move
mv ~cbopus.tmp %1
echo                                         cbO: OK: %1
echo cbO: OK: %1 >> cbopus.err
goto finis

:noname
echo cbO: %1 does not exist
echo cbO: %1 does not exist >> cbopus.err

:finis

