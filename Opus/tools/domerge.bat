echo Backing up copy of %1 to backup dir or root. Delete after merge if desired.
if "%2"== "" goto noback
copy %1 %2
goto getonwithit
:noback
copy %1 c:\
:getonwithit
scomp %1 > premerge.dif
more premerge.dif
ssync -f %1
scomp %1 > merge1.dif
more merge1.dif
diff premerge.dif merge1.dif >merge2.dif
more merge2.dif
ssync %1
