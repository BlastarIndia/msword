#  MKCMD
mkcmd.exe: mkcmd.c
	cc -Zp mkcmd.c
	mv mkcmd.exe ..


#  STRPMAP
strpmap.exe: strpmap.c
	cc -AC -Ot -Zp strpmap.c
	mv strpmap.exe ..


#  DNATFILE
dnatfile.exe: dnatfile.c dnatfile.h
	cc -c -AS -Ot -Zp -I \cashmere dnatfile.c
	link dnatfile /stack:4000;
	mv dnatfile.exe ..


#  WHEN
when.exe: when.obj timedate.obj
	link when.obj+timedate.obj,when.exe,when.MAP/MAP;
	mapsym when
	exemod when.exe /max 1
	mv when.exe when.sym ..

when.obj: when.c
	cc -c -d -u when.c

timedate.obj: timedate.asm
	masm -Mx -DDEBUG timedate.asm ;


#  COPYOBJ
copyobj.exe: copyobj.c
	cc -c -AS -Ot copyobj.c
	link copyobj.obj/NOI/MAP;
	mapsym copyobj
	exemod copyobj.exe /max 1
	mv copyobj.exe ..


#  MAKEOPUS
makeopus:
	cc -c -AS -Os -DMAKEOPUS makeopus.c
	link makeopus.obj/NOI;
	exemod makeopus.exe /max 1
	mv makeopus.exe ..
	del makeopus.obj
	cc -c -AS -DMAKEO2 makeopus.c
	link makeopus.obj/NOI;
	exemod makeopus.exe /max 1
	mv makeopus.exe ..\makeo2.exe
	del makeopus.obj


#  MISC

.c.exe:
	cc -c -d -u $*.c
	link $*.obj,$*.exe/NOI,$*.MAP/MAP;
	mapsym $*
	exemod $*.exe /max 1
	mv $*.exe ..

.asm.exe:
	masm $*;
	link $*;
	mv $*.exe ..
