##############################################################################
####									  ####
####	Make file for CONV-TST.DLL      				  ####
####	For use with the MS-DOS version of the UNIX make utility,         ####
####	    Microsoft Windows SDK (version 2), Microsoft 'C' Compiler     ####
####	    and Microsoft Macro Assembler.                      	  ####
####									  ####
##############################################################################

CFLAGS = /c /AM /Gw /Oad /Zped
#     compiler switches
#     /A       memory model switches
#	       /AS == /Asnd   /AM == /Alnd   /AL == /Alfnd
#	  s    small model, generates SHORT code pointer (16 bits)
#	  l    middle, large model, generates LONG code pointer (32 bits)
#	  n    generate NEAR data pointer (16 bits)
#	  f    generate FAR data pointer (32 bits)
#	  d    DS == SS
#	  u    DS != SS, auto DS load in entry point of each program module
#	  w    DS != SS, no auto DS load
#    /PLM      PLM calling convention
#    /Ox       equivalent to /Oas /Gs
#    /Gs       removes stack probes, i.e. no check for stack availablity
#	       (reduce size & exe time)
#	  w    Put Windows header and footer on all FAR functions.  Used
#			only for modules that have exported functions.
#    /O        optimization switches
#	  a    relax aliasing checks -- ONLY turn on this switch if you are
#			   sure your code has no aliasing
#	  s    favor code size during optization
#	  t    favor execution time during optimization
#	  d    disable optimization
#    /Z
#	  p    pack structure members
#	  e    consider far, fortran, huge, near, pascal as keywords
#	  d    insert source line info for debugger
# (for more information of other switches, see manual)

LFLAGS = /m /NOD /ALIGN:16 /NOP /NOE
#	linker switches
#	/m		generates map files
#	/NOD		don't search for default lib files
#	/NOP		don't pack the code into one segment



conv-tst.obj:

doslib.obj:

init.obj:

conv-tst.def:

conv-tst.dll: conv-tst.obj doslib.obj init.obj conv-tst.def conv-tst.lnk
	link4 @conv-tst.lnk
	winmark memory conv-tst.dll

conv-tst.lnk: makefile
	echo CONV-TST.OBJ INIT.OBJ DOSLIB.OBJ > conv-tst.lnk
	echo CONV-TST.DLL >> conv-tst.lnk
	echo CONV-TST $(LFLAGS) >> conv-tst.lnk
	echo mwinlibc.lib mlibw.lib + >> conv-tst.lnk
	echo libh.lib mlibce.lib crmgr.w86 >> conv-tst.lnk
	echo CONV-TST.DEF >> conv-tst.lnk

#	This is how you transform (compile) the various file types

# NOTE: to use retail products, change cc to cl.
.c.obj:
	cc $(CFLAGS) $*.c

.asm.obj:
	masm -Mx $*.asm;


