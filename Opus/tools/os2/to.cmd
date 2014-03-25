@echo off
for %%i in (%path%) do if EXIST %%i\newtoexe.exe goto doit
echo ERROR! newtoexe.exe not on your path.
goto exit

:doit
newtoexe %1 %2 %3 %4 %5 %6 %7 %8 %9
call tocmd

:exit
