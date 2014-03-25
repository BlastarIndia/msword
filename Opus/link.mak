NAM = opus
SED = sed
RC = rc

#	Rc switches:
#   -r	    generates a .res file
#   -v n.n  use rc version n.n
#   -l      needed for win386
RCFLAGS = -l


#	PCODE libraries:
#		winter.lib	-- Non-debugging version of interpreter
#               sbmgr.lib       -- Non-debugging version of sb mgr
#               
#		dwinter.lib	-- Debugging version of interpreter
#               dsbmgr.lib      -- Debugging version of sb mgr
#
#		dlmem.w86	-- Local MEMory manager (debugging, Windows)
#		sdm.lib		-- Standard Dialog Manager
#		el.lib		-- Standard Extension Language
#
#		fswinter.lib	-- larger but faster nondebugging version
WINTERLIB = dwinter.w86 dsbmgr.w86 dlmem.w86 sdm.lib el.lib dcrmgr.w86
OPUSLIB = $(WINTERLIB) mlibw.lib mwlibc.lib libh.lib dmath.w86
NDWINTERLIB = winter.w86 sbmgr.w86 lmem.w86 sdm.lib el.lib crmgr.w86
NDBLIB = $(NDWINTERLIB) mlibw.lib mwlibc.lib libh.lib dmath.w86

#	Linker switches:
#	/m		    generates map files (don't use w/ new ILINK)
#	/NOD		don't search for default lib files
#	/INC		Incremental Link (# following is cSymMax)
#	/PADD	    Data padding, per module
#	/PADC	    Code padding, per module
#   /SEGMENTS	total number os segments allowed

LFLAGS = /m /NOD /NOE /ALIGN:16 /PADD:16 /PADC:256 /SEGMENTS:250
LFLAGSNDB = /m /NOD /NOE /ALIGN:16 /SEGMENTS:250

#	Sed switches:
SFLAGS = -n -f

dbg.lnk: makefile link.mak mlink.sed $(NAM).def lib\sdm.lib lib\el.lib
	$(SED) $(SFLAGS) mlink.sed < makefile > dbg.lnk
	echo $(NAM) >> dbg.lnk
	echo $(NAM) $(LFLAGS) >> dbg.lnk
	echo $(OPUSLIB) >> dbg.lnk
	echo $(NAM).def >> dbg.lnk

ndb.lnk: link.mak makefile mlinkndb.sed $(NAM)n.def lib\sdm.lib lib\el.lib
	$(SED) $(SFLAGS) mlinkndb.sed < makefile > ndb.lnk
	echo $(NAM) >> ndb.lnk
	echo $(NAM) $(LFLAGSNDB) >> ndb.lnk
	echo $(NDBLIB) >> ndb.lnk
	echo $(NAM)n.def >> ndb.lnk

xdb.lnk: makefile link.mak mlinkndb.sed $(NAM)n.def lib\sdm.lib lib\el.lib
	$(SED) $(SFLAGS) mlinkndb.sed < makefile > xdb.lnk
	echo $(NAM) >> xdb.lnk
	echo $(NAM) $(LFLAGS) >> xdb.lnk
	echo $(OPUSLIB) >> xdb.lnk
	echo $(NAM)n.def >> xdb.lnk

$(NAM)n.def: $(NAM).def
	fgrep -v Debug $(NAM).def >$(NAM)n.def
