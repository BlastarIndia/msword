echoerr *** Building command table
mkcmd opuscmd
masm400 -Mx opuscmd.asm
cp opuscmd.h ..
cp opuscmd.obj ..
touch ..\opuscmd.snt
touch ..\cshare\opuscmd.snt

