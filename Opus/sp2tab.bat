echo off
echo.

if "%1" == "" goto instr

:loop
	echo Tabifying %1
	echo Tabifying %1 >> sp2tab.log
	sed -f tabify.sed %1 > local.tmp
	mv local.tmp %1
	shift
	if "%1" == "" goto done
	goto loop

:instr
echo Usage: sp2tab file1.c file2.h file3.ext file4.xyz ...
	
:done
echo.
echo Finished
echo.
