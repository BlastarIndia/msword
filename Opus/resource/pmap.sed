/* I just discovered how to put comments in sed scripts!! :-) */ {}

/* pmap.sed is really three sed scripts folded into one.  It uses */ {}
/* the contents of the incoming line to decide which of the three */ {}
/* entry points is desired */ {}


/\$touch/ b L2
/\$fgrep/ b L3


/* create batch file to generate a file with the names of native */ {}
/* functions that reside in pcode modules */ {}

s/^/grep createSeg.*_PCODE /
s/$/ | sed -e "s\/[^ 	]*[ 	]*\/\/" -e "s\/_PCODE.*\/\/" >> $1/
w $1.bat
s/createSeg\.\*_PCODE/^[^;]*cProc/
s/| sed.*/>> $1/
w $1.bat
s/.*/echo $ >> $1/
w $1.bat
d
q

/* create a batch file to remove objects, after touching them to */ {}
/* avoid the stderr message; the "touch" part of the command is  */ {}
/* already there (with $ prefix to avoid possible conflict with  */ {}
/* segment names) to distinguish this pass of pmap.sed from the  */ {}
/* other */ {}

:L2
s/^\$//
w $1.bat
s/touch/rm/
w $1.bat
d
q

:L3
s/^\$//
h
s/.asm/.obj ..\\makefile/
w $1.bat
g
s/^fgrep /if NOT errorlevel 1 echo ..\\/
s/$/ >> $files/
w $1.bat
d
