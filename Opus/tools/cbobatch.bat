echo off

if exist cbopus.err del cbopus.err
if exist cbotemp.bat del cbotemp.bat

:readfile
if "%1"=="" goto begin
if not exist %1 goto noname

ls /vqs %1 | sed "s/^/command \/c cbo /" >> cbotemp.bat

shift
goto readfile

:noname
echo cbOBatch: %1 does not match any files in this directory
echo cbOBatch: %1 does not match any files in this directory >> cbopus.err
shift
goto readfile

:begin
if not exist cbotemp.bat goto notemp
command /c cbotemp
rm before.tmp after.tmp cbotemp.bat
goto finish

:notemp
echo cbOBatch: No match for any requested file name.
echo cbOBatch: No match for any requested file name. >> cbopus.err

:finish
echo.
echo cbOBatch: CBOPUS batch job is finished for this directory.
echo cbOBatch: check cbopus.err for possible errors.
echo cbOBatch: CBOPUS batch job is finished for this directory. >> cbopus.err
echo.
