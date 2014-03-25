rem  This batch file prepares a PCODEMAP.TXT which can be used with
rem  pcode profiling.  Some of our pcode segments have hand native
rem  code as part of the segment.  In order for the pcode profiler to
rem  be able to figure out what function it's in when any of this
rem  hand native code is being executed the names of all the native
rem  routines in a pcode segment must be appended after a colon to
rem  the segment name in PCODEMAP.TXT.  This batch file does just
rem  that.  Also, for pcode profiling, im2obj must be called with -p
rem  on all modules that are combined pcode/hand-native code (it
rem  is ignored on all other modules.  For more info, see the help
rem  screen on im2obj.
rem
rem  If called with -f, this batch file will delete the objs of all
rem  modules for which the -p flag of im2obj will make a difference.
rem
rem  Written by:  rosiep  1/27/88


rem get list of .asm files which are part of PCODE segments and which
rem  are part of our make process;  pmap.sed creates $1.bat
rem
rem  get rid of $files because $1.bat appends to it

grep -l createSeg.*_PCODE ..\*.asm > $1
touch $files
rm $files
sed -e "s/^..\\/$fgrep /" $1 | sed -f pmap.sed
command /c $1

:L2
rem get list of pcode segments which have hand native code in them
rm $1
sed -e "s/^/grep createSeg.*_PCODE /" -e "s/$/ >> $1/" $files > $1.bat
command /c $1
sed -e "s/[^ 	]*[ 	]*//" -e "s/_PCODE.*//" $1 > $segs

:L3
rem delete obj's for them if necessary (in all possible directories)
rem  pmap.sed creates $1.bat
if NOT .%1 == .-f goto :L4
sed -e "s/^/$touch /" -e "s/$/.obj/" $segs | sed -f pmap.sed
command /c $1
cd ..
command /c cshare\$1
cd wordtech
command /c ..\cshare\$1
cd ..\cshare

:L4
rem  get list of cProc functions within the .asm files
rem
rem  the functions in this list are grouped by segment with the segment
rem  name starting each group, and each group terminated by a $ on a
rem  line by itself.
rem   
rem  we get rid of $1 because $1.bat appends to it
rem
rem  pmap.sed creates $1.bat

rm $1
sed -f pmap.sed $files
command /c $1
sed -e "s/^.*cProc[ 	]*//" -e "s/[, 	].*//" $1 > $funcs

:L5
rem create pcodemap.txt
awk -f pmap.awk $segs $funcs pmap.txt > pcodemap.txt

:L6
rem clean up
rm $1 $1.bat $files $segs $funcs
