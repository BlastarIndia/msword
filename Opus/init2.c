/* I N I T 2 . C */
#define RSHDEFS
#define OEMRESOURCE /* So we get OBM_CLOSE from qwindows.h */
#define NODRAWFRAME
#define NOCTLMGR
#define NOMINMAX
#define NOSCROLL
#define NOKEYSTATE
#define NOCREATESTRUCT
#define NOICON
#define NOPEN
#define NOREGION
#define NODRAWTEXT
#define NOWINOFFSETS
#define NOMETAFILE
#define NOCLIPBOARD
#define NOSOUND
#define NOCOMM
#define NOKANJI

#define SBMGR
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "version.h"
#include "heap.h"
#define NOSPECMSG
#include "wininfo.h"
#include "doc.h"
#include "props.h"
#include "disp.h"
#include "file.h"
#include "fkp.h"
#include "sel.h"
#include "format.h"
#include "print.h"
#include "layout.h"
#include "resource.h"
#include "keys.h"
#include "cmdtbl.h"
#include "menu2.h"
#include "opuscmd.h"
/*#include "field.h"*/
#include "inter.h"
#define AUTOSAVE
#include "autosave.h"
#define REVMARKING
#include "compare.h"
#include "help.h"
#include "screen.h"
#include "doslib.h"
#include "debug.h"
#include "dmdefs.h"
#include "status.h"
#include "core.h"
#include "preffile.h"
#include "ibdefs.h"
#include "outline.h"
#include "ch.h"
#include "error.h"
#include "idle.h"
#include "automcr.h"




/* E X T E R N A L S */

extern MUD ** HmudInit();

extern struct LCB  vlcb;
extern BOOL             vfScratchFile;
extern struct DMQD        DMQueryD;
extern struct DMFLAGS     DMFlags;
extern struct DMQFLAGS    DMQFlags;
extern CHAR             szEmpty[];
extern CHAR             stEmpty[];
extern BOOL             vfFileCacheDirty;
extern int              docGlobalDot;
extern HMENU            vhMenuLongFull;
extern struct FTI       vfti;
extern struct FTI       vftiDxt;
extern struct STTB      **vhsttbFont;
extern struct STTB      **vhsttbOpen;
extern struct PRI       vpri;
extern struct PRSU      vprsu;
extern CHAR             szEmpty[];
extern CHAR             stEmpty[];
extern HCURSOR          vhcHourGlass;
extern HCURSOR          vhcIBeam;
extern HCURSOR          vhcArrow;
extern HCURSOR          vhcLRdrag;
extern HCURSOR          vhcSplit;
extern HCURSOR          vhcStyWnd;
extern HCURSOR          vhcRecorder;
extern HCURSOR          vhcHelp;
extern HCURSOR		vhcOtlCross;
extern HCURSOR		vhcOtlVert;
extern HCURSOR		vhcOtlHorz;
extern HCURSOR		vhcColumn;
extern HANDLE           vhInstance;
extern HWND             vhwndApp;
extern HWND             vhwndSizeBox;
extern HWND             vhwndCBT;
extern struct SCI	vsci;
extern struct STTB    **vhsttbWnd;
extern HMENU            vhMenu;
extern int              viMenu;
extern int              vfInitializing;
extern int              vfMouseExist;
extern BOOL             vfSingleApp;
extern char szClsApp[];
extern char szClsStatLine[];
extern char szClsDeskTop[];
extern char szClsStart[];
extern int              vfDeactByOtherApp;
extern IDF		vidf;
extern struct ITR	vitr;


/* G L O B A L S */
struct PREF     vpref;
struct STTB     **vhsttbOpen;
struct STTB     **vhsttbWnd;

/* G L O B A L S */
CHAR            szEmpty[] = "";
CHAR            stEmpty[] = { 
	0 };


CHAR            szApp[] = szAppDef;
#ifdef MKTGPRVW
CHAR            szAppTitle[] = szAppTitleDef;
#endif /* MKTGPRVW */
#ifdef DEMO
CHAR            szAppTitle[] = szAppTitleDef;
#endif /* DEMO */

/* these are initialized from win.ini at startup */
CHAR	    	szDoc[5]; /* ".DOC" */
CHAR	    	szDot[5]; /* ".DOT" */
CHAR	    	szBak[5]; /* ".BAK" */

/* tif file default extension. LATER - should be in win.ini? bz */
CHAR            szTif[] = SzGlobalKey(".TIF",TIF);

/* NORMAL.DOT file name */
CHAR            stNormalDot[] = StGlobalKey("NORMAL",NORMAL); /* do not include extension */

/* ...and now for something completely different; the word "None" */
CHAR            szNone[] = SzGlobalKey("None", None);

/* E X T E R N A L S */
extern struct PREF      vpref;
extern struct PREF      vprefCBT;  /* pref's before CBT run; restore after */
/* extern struct SAB       vsab; */

extern struct BPTB      vbptbExt;

extern struct SEL       selCur;

extern struct FKPD      vfkpdChp;
extern struct FKPD      vfkpdPap;
extern struct FKPD      vfkpdText;

extern struct WWD       **mpwwhwwd[];
extern struct FCB       **mpfnhfcb[];
extern struct WWD       **hwwdCur;

extern struct FLI       vfli;
extern CHAR             **vhgrpchr;
extern int              vbchrMax;

extern struct PLC    **vhplcfrl;
extern struct FLS       vfls;

extern struct DOD       **mpdochdod[];

extern struct MERR      vmerr;
extern struct UAB       vuab;
extern struct AAB       vaab;
extern int              cfRTF;
extern int              cfLink;
extern CHAR             szApp[];

extern CHAR           **hstDMQPath;
extern struct DMFLAGS   DMFlags;

#ifdef DEBUG
extern BOOL		fForce8087Fail;
#endif


extern HWND vhwndRibbon;
extern HWND vhwndStatLine;
extern HWND vhwndDeskTop;
extern HWND vhwndStartup;

#ifdef RPTLOWMEM /* no longer wanted */
csconst int mpsaseid[] = 
	{
/*sas   error message */
/*0*/  	eidNil,
/*1*/  	eidNil,
/*2*/  	eidNil,
/*3*/  	eidNil,
/*4*/  	eidLowMem1,
/*5*/  	eidLowMem2,
/*6*/  	eidVeryLowMem,
/*7*/  	eidVeryLowMem,
/*8*/  	eidVeryLowMem,
	};
#endif /* RPTLOWMEM */


/* FInitPart2 - Second part of initialisation.
*/
/* %%Function:FInitPart2 %%Owner:PETERJ */
FInitPart2(cmdShow, rgchCmdLine, rgpchArg, ipchArgMac, fOpenUntitled, fTutorial)
int    cmdShow;
CHAR rgchCmdLine [];
CHAR *rgpchArg [];
int ipchArgMac;
BOOL fOpenUntitled;
BOOL fTutorial;
{
	BOOL fDoAutoMacro = fTrue;

/* initialize status line cache to nil values */
	vlcb.ca.doc = docNil;
	vlcb.lnn = sivNil;
	vlcb.yaPp = sivNil;
	vlcb.ich = iNil;

/* force top-level window to be zoomed, if it was zoomed when we last exited */
	if (vpref.fZoomApp && (cmdShow == SW_SHOW || cmdShow == SW_SHOWNORMAL))
		cmdShow = SW_SHOWMAXIMIZED;

/* set up menus */
	if (!FCreateSysMenu())
		{
		ReportSz("Can't create child sys menu");
		goto InzFailed;
		}

	if (vpref.fShortMenus)
		{
		if (!FSetShortMenu())
			{
			ReportSz("Can't create short menus");
			goto InzFailed;
			}
		}
	else
		{
		vhMenu = vhMenuLongFull;
		viMenu = iMenuLongFull;
		if (vpref.fZoomMwd)
			CorrectSystemMenu();
		}

/* create application top-level window */
	if (CreateWindow((LPSTR)szClsApp,
			(LPSTR)szAppTitle,
			WS_TILEDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE,
			CW_USEDEFAULT, cmdShow, CW_USEDEFAULT, 0,
			(HWND)NULL,           /* no parent */
			vhMenu,
			(HANDLE)vhInstance,    /* handle to window instance */
			(LPSTR)NULL           /* no params to pass on */
			) == NULL )
		/* Could not create window */
		{
		ReportSz ("App Window creation failure");
		goto InzFailed;
		}

	Assert( vhwndApp != NULL );  /* AppWndProc sets it on WM_CREATE message */

	vfDeactByOtherApp = GetActiveWindow() != vhwndApp;

/* now we can use our "Long operation" mechanism */
	StartLongOp ();

	Scribble(15,'A');
	Scribble(14,' ');

/* put up the startup dialog */
/* must happen after main window is created and vhwndApp is defined */
	if (!IsIconic(vhwndApp) && !vfDeactByOtherApp)
		{
		struct RC rc;
		int dxp = vsci.dxpTmWidth * cchSizeStartup;
		int dyp = (vsci.dypTmHeight * 8);

		GetWindowRect(vhwndApp, &rc);
		vhwndStartup = CreateWindow((LPSTR)szClsStart, 
				NULL,
				WS_POPUP | WS_DLGFRAME,
				(rc.xpLeft + rc.xpRight - dxp)/2, 
				(rc.ypTop + rc.ypBottom - dyp)/4,
				dxp, dyp,
				vhwndApp, NULL, vhInstance, NULL);
		if (vhwndStartup == NULL)
			{
			ReportSz("Startup Window creation failure");
			goto InzFailed;
			}
		ShowWindow(vhwndStartup, SHOW_OPENNOACTIVATE);
		}

	Scribble(15,'B');
	Scribble(14,' ');

	if (!FAddToSystemMenu())
		{
		ReportSz("Menu load failure");
		goto InzFailed;
		}

	Scribble(15,'C');
	Scribble(14,' ');

/* initialize docNew */
	if (!FCreateDocNew())
		{
		ReportSz("can't create docNew");
		goto InzFailed;
		}

	Scribble(15,'D');
	Scribble(14,' ');

/* initialize docScrap, docUndo */
	if (DocCloneDoc(docNew, dkDoc) != docScrap)
		{
		ReportSz("can't create docScrap");
		goto InzFailed;
		}
	if (DocCloneDoc(docNew, dkDoc) != docUndo)
		{
		ReportSz("can't create docUndo");
		goto InzFailed;
		}
	if (DocCloneDoc(docNew, dkDoc) != docScratch)
		{
		ReportSz("can't create docScratch");
		goto InzFailed;
		}
	if (DocCreateTemp(docScrap) == docNil) /* mother doc irrelevant */
		{
		ReportSz("can't create vdocTemp");
		goto InzFailed;
		}

	PdodDoc(docScrap)->fGuarded = fTrue;
	PdodDoc(docUndo)->fGuarded = fTrue;

	Scribble(15,'E');
	Scribble(14,' ');

/* Create a dot in docGlobalDot; don't do tutorial one - that comes later */
	if (!FInitGlobalDot(fFalse))
		{
		ReportSz("can't create global dot");
		goto InzFailed;
		}

	Scribble(15,'F');
	Scribble(14,' ');

/* make status line & ribbon visible, if they were visible on last exit */
	if (vpref.fRibbon && !fTutorial)
		{
		extern int far pascal iconbar2_q();
		if (FTurnRibbonFOn(fTrue /* fOn */ , fFalse /* fAdjust */, fTrue))
            {
            Assert (vhwndRibbon != NULL);
		    GlobalLruOldest( GetCodeHandle( (FARPROC)iconbar2_q) );
		    if (vhwndStartup == NULL)
			    DisplayRibbonInit();
            }
		}

	Scribble(15,'G');
	Scribble(14,' ');

	if (vpref.fStatLine)
		{	/* open code status line creation */
		struct RC rc;

		GetClientRect( vhwndApp, (LPRECT) &rc );

		rc.xpLeft -= vsci.dxpBorder;
		rc.xpRight += vsci.dxpBorder;
		rc.ypBottom += vsci.dypBorder;
		rc.ypTop = rc.ypBottom - vsci.dypStatLine;

		vhwndStatLine = CreateWindow((LPSTR)szClsStatLine, 
				(LPSTR)NULL,
				WS_CHILD | WS_CLIPSIBLINGS | WS_BORDER,
				rc.xpLeft, rc.ypTop,
				rc.xpRight - rc.xpLeft,
				vsci.dypStatLine,
				vhwndApp, NULL, vhInstance, (LPSTR)NULL);

		if (vhwndStatLine == NULL)
			{
			ReportSz("can't create status line");
			goto InzFailed;
			}

		if (vhwndStartup == NULL)
			DispStatlineInit();
		}

	Scribble(15,'H');
	Scribble(14,' ');

/* create the desktop window */
		{
		struct RC rc;

		GetDeskTopPrc( &rc );
		if (!(vhwndDeskTop = CreateWindow(
				(LPSTR)szClsDeskTop, (LPSTR)szEmpty,
				WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE | WS_CLIPSIBLINGS,
				rc.xpLeft, rc.ypTop,
				rc.xpRight - rc.xpLeft, rc.ypBottom - rc.ypTop,
				vhwndApp,
				(HMENU)NULL, vhInstance, (LPSTR)NULL)))
			{
			ReportSz("can't create desktop window");
			goto InzFailed;
			}
		}

/* add keys that are not known until runtime */
		{
		extern int vkPlus, vkMinus,  vkStar;
		extern KMP ** hkmpBase;

		AddKeyKtW(hkmpBase, kcNonReqHyphen, ktInsert, chNonReqHyphen);
		AddKeyKtW(hkmpBase, kcNonBreakHyphen, ktInsert, chNonBreakHyphen);
		AddKeyToKmp(hkmpBase, kcSubscript, bcmSubscript);
		AddKeyToKmp(hkmpBase, kcSuperscript, bcmSuperscript);
		AddKeyToKmp(hkmpBase, kcShowAll, bcmShowAll);
		}

	Debug( vdbs.fReportHeap ? 
			ReportHeapSz(SzSharedKey("basic init",BasicInit), 0) : 0 );

	Scribble(15,'I');
	Scribble(14,' ');

/* check for unusal conditions: no username, low memory configuration */

	if (vpref.stUsrName [0] == 0 || vpref.stUsrInitl [0] == 0)
		{
#ifdef BATCH /* in batch mode, don't require username dialog */
		if (vfBatchMode)
			{
			extern int vfConversion;
			vfConversion = fFalse; /* always assume conversions not allowed */
			bltb(StFrame("WW"),vpref.stUsrInitl, 3);
			bltb(StFrame("Win Word"),vpref.stUsrName, 9);
			}
		else
#endif /* BATCH */
			PromptUserName();
		if (vpref.stUsrName [0] == 0 || vpref.stUsrInitl [0] == 0)
			{
			vmerr.fErrorAlert = fTrue;
			goto InzFailed;
			}
		}

#ifdef RPTLOWMEM /* no longer wanted */
		{
		extern int vsasCur;
		extern int vfNoCoreLoad;
		int eid;
		if ((eid=mpsaseid[vsasCur]) != eidNil && FFromProfile(fTrue, ipstMemWarn)
				&& (eid != eidLowMem1 || vpref.fBkgrndPag) && !vfNoCoreLoad 
				Debug(&& !vdbs.fNoCoreLoad) && !vfDeactByOtherApp)
			ErrorEid(eid, "FInitPart2");
		}
#endif /* RPTLOWMEM */

#ifdef RSH
	InitUa(); /* initialize User-Action recording */
#endif /* RSH */

#ifdef DEBUG
	if (nMonthBuilt >= 12)
		ReportSz("have peterj review timebomb date!");
#endif /* DEBUG */

#ifdef MKTGPRVW  /* timebomb */
/* warn users to get the real product/next beta.  hard coded for 14Feb90 */
	if (((LONG)DttmCur() & 0x1FFFFFFFL) >= 
			/* DttmOfMDY(((nMonthBuilt-1)+7)%12+1,nDayBuilt,nYearBuilt+(nMonthBuilt>5)) */
	DttmOfMDY(2,14,90)
			&& GetProfileInt(szApp, SzShared(szDisableWarningDef), 1))
		ErrorEid(eidBetaVersion, "FInitPart2");
#endif /* MKTGPRVW */

/* VISUAL NOTE: Starting after this point, visible things start happening.
	In the interest of apparent speed, we should try to do work before this
	point so that after the initial delay, the product appears fast. */

	Scribble(15,'J');
	Scribble(14,' ');

/* process command line arguments; create a document window */
/* /TUTORIAL command line arg supersedes all other command line args */
	if (!fTutorial)
		{
		int ipchArg;
		
		for (ipchArg = 0; ipchArg < ipchArgMac; ipchArg++)
			{
			if (rgpchArg[ipchArg][0] == chCmdSwitch && 
					ChUpper(rgpchArg[ipchArg][1]) == chCmdLineMacro)
				{
				fDoAutoMacro = fFalse;
				break;
				}
			}

		if (fDoAutoMacro)
			{
			CmdRunAtmOnDoc(atmExec, selCur.doc);
			}

		if (ipchArgMac > 0)
			{
			FAbortNewestCmg (cmgLoad1, fFalse, fFalse);
			FAbortNewestCmg (cmgLoad2, fFalse, fFalse);
			if (!FInitArgs(rgpchArg, ipchArgMac))
				{
				ReportSz("FinitArgs failure");
				goto InzFailed;
				}
			}
		}

	vfInitializing = FALSE;

	Scribble(15,'K');
	Scribble(14,' ');

/* The macro running stuff...  (Only add code above this line!) */
	if (!fTutorial)
		{
				/* BLOCK: now run macros specified on command line */
		    { 
		    int isz;
		    CHAR * szArg;

		    for (isz = 0; isz < ipchArgMac; ++isz)
    			{
    			szArg = rgpchArg[isz];
			if (*szArg++ == chCmdSwitch && ChUpper(*szArg++) == chCmdLineMacro)
    				{
    				if (*szArg != '\0')
    					{
    					if (vhwndStartup != NULL)
    						EndStartup();
    					SzToStInPlace(szArg);
    					FRunMacro(szArg);
    					}
    				}
    			}
		    }
        }


    if (!fTutorial && fOpenUntitled && selCur.doc == docNil)
        {
    	CHAR *stType = stEmpty;
    	ElNewFile (stType, fFalse);
    	if (selCur.doc != docNil && PdodDoc(selCur.doc)->udt == udtDocument)
    		PdodDoc(selCur.doc)->iInstance = 0; /* special case */
        }


    if (selCur.doc == docNil)
        { /* nothing was opened, set min menus */
        FSetMinMenu();
        }


    if (vhwndStartup != NULL)
       EndStartup();

    if (fTutorial)
        FRunCBT(fTrue /* fStartup */);

    EndLongOp(fFalse /* fAll */);
    Scribble(15,'L');
    Scribble(14,' ');

    Profile( vpfi == pfiInit ? StopProf() : 0 );
    Debug(CkSortOrdinals());

    return fTrue;

InzFailed:
    Profile( vpfi == pfiInit ? StopProf() : 0 );
    SetFlm( flmIdle );

    EndLongOp(fFalse /* fAll */);
    ErrorEidStartup(eidCantRunM);
    return fFalse;
}


/* E N D  S T A R T U P */

/* %%Function:EndStartup %%Owner:PETERJ */
EndStartup()
{
	EndStartup1();
	EndStartup2();
}


EndStartup1()
{
	Scribble(14,'U');

/* destroy the Startup window */
	Assert(vhwndStartup);
	DestroyWindow(vhwndStartup);

/* display the ribbon */
	if (vhwndRibbon)
		DisplayRibbonInit();
}


EndStartup2()
{
/* display the status line */
	if (vhwndStatLine)
		DispStatlineInit();
}


/* Command Line Argument processing/initialization */
/*       Steps:                                    */
/* %%Function:FInitArgs %%Owner:PETERJ */
FInitArgs( rgszArg, ipchArgMac )
CHAR * rgszArg [];
int ipchArgMac;
{

	int doc;
	int ipchArg;
	CHAR stBuf [ichMaxFile];
	CHAR * szArg;
	int fInitT;
#ifdef DEBUG
	extern CHAR vchRTFTest;
#endif /* DEBUG */

	for (ipchArg = 0; ipchArg < ipchArgMac; ipchArg++)
		{
		szArg = rgszArg[ipchArg];
		if (szArg[0] == chCmdSwitch)
			continue;

		SzToSt( szArg, stBuf );

#ifdef DEBUG
		if (vchRTFTest)
			{
			PerformRTFTestSuite (vchRTFTest, stBuf);
			/* this never returns */
			Assert (fFalse);
			}
#endif /* DEBUG */

		if ((doc = DocOpenStDof(stBuf, dofCmdNewOpen, NULL)) == docNil)
			{
		/* it isn't necessary to fail to boot just because we
					couldn't open one of the command line files. */

			if (vmerr.fMemFail)
				return fTrue; /* failure due to out of memory! */

			continue;
			}

		/* GregS now allows multiple files (again) */
		CmdRunAtmOnDoc(atmOpen, selCur.doc);
		}

	return fTrue;
}


/* %%Function:DispStatlineInit %%Owner:PETERJ */
DispStatlineInit()
{
/* open code InitStatLineState+MakeStatLineVisible */
	extern struct SLS vsls;
	struct SLS sls;

	Scribble(14,'V');
	SetWords(&sls, 0, cwSLS);
	FGetStatLineState(&sls,usoNormal);
	blt( &sls, &vsls, cwSLS );
	Scribble(14,'W');
	ShowWindow( vhwndStatLine, SHOW_OPENWINDOW );
	UpdateWindow( vhwndStatLine );
}


DisplayRibbonInit()
{
    Assert (vhwndRibbon != NULL);
        /* force update to gray ribbon if no doc up */
	selCur.fUpdateChpGray = fTrue;
	Scribble(14,'X');
	UpdateRibbon(fTrue /* fInit */);
	Scribble(14,'Y');
	ShowWindow(vhwndRibbon, SHOW_OPENWINDOW);
	UpdateWindow(vhwndRibbon);
}

/***************************************/
/* F  A d d  T o  S y s t e m  M e n u */
/* %%Function:FAddToSystemMenu %%Owner:PETERJ */
FAddToSystemMenu()
{
	HMENU hSysMenu;
	BOOL f;
	int w;

	 f = (hSysMenu = GetSystemMenu(vhwndApp, FALSE)) != NULL &&
			ChangeMenu(hSysMenu, 0, NULL, -1,
			MF_APPEND | MF_SEPARATOR) != NULL &&
			ChangeMenu(hSysMenu, 0, (LPSTR) SzSharedKey("R&un...",Run),
			bcmControlRun, MF_APPEND | MF_STRING) != NULL;

	/* change Window's separators' bcms to negative values */
	if (f)
		{
		while (ChangeMenu(hSysMenu, 0, (LPSTR)NULL, -1, MF_CHANGE | MF_SEPARATOR));
		w = GetMenuState(hSysMenu, 0, MF_BYCOMMAND);
		}

	return (f && (w==0 || w==-1));
}



/* F C R E A T E  D O C  N E W */
/*  Create and initialize docNew. Done here because doc's created by the
	normal mechanism are derived from docNew!

	These are the values all new documents will start out with (unless there
	is a global dot (opus.gdt) with different values).
*/

/* %%Function:FCreateDocNew  %%Owner:PETERJ */
FCreateDocNew ()

{
	struct DOD dod;
	struct DOD *pdod;

	if (DocAlloc (dkDoc, &dod) != docNew)
		return fFalse;

	/* this shouldn't fail; we're just starting up */
	if (! (FInitPlcpcd (&dod, fnNil, NULL)))
		return fFalse;

		/* BLOCK: open code FAddDocFonts (simple case) */
		{
		CHAR *pibst;
		if ((dod.hmpftcibstFont = HAllocateCw(CwFromCch(ftcMinUser)))
				== hNil)
			return fFalse;
		dod.ftcMac = dod.ftcMax = ftcMinUser;
		pibst = *dod.hmpftcibstFont;
		*pibst++ = ibstFontDefault;     /* Tms Rmn */
		*pibst++ = ibstFontDefault + 1; /* Symbol font */
		*pibst++ = ibstFontDefault + 2; /* Helv */
		*pibst   = ibstFontDefault + 3; /* Courier */
		}

	/* create a section table */
	pdod = &dod;
	if (FNoHeap(HplcCreateEdc(&pdod, edcSed)))
		return fFalse;

	dod.dop.yaPage = vitr.fUseA4Paper ? yaPageA4Def : yaPageNonA4Def;
	dod.dop.xaPage = vitr.fUseA4Paper ? xaPageA4Def : xaPageNonA4Def;
	dod.dop.dyaTop = dyaTopDopDef;
	dod.dop.dyaBottom = dyaBottomDopDef;
	dod.dop.dxaLeft = dxaLeftDopDef;
	dod.dop.dxaRight = dxaRightDopDef;
	dod.dop.dxaGutter = dxaGutterDopDef;
	dod.dop.dxaTab = dxaTabDopDef;
	dod.dop.fFacingPages = fFacingPagesDopDef;
	dod.dop.fWidowControl = fWidowControlDopDef;
	dod.dop.fpc = fpcDopDef;
	dod.dop.fWide = fWideDopDef;
	dod.dop.grpfIhdt = grpfIhdtDopDef;
	dod.dop.fFtnRestart = fFtnRestartDopDef;
	dod.dop.nFtn = nFtnDopDef;
	dod.dop.fRevMarking = fRevMarkingDef;
	dod.dop.irmBar = irmBarDef;
	dod.dop.irmProps = irmPropsDef;
	dod.dop.dxaHotZ = dxaHotZDopDef;

		/* sepx = nil */

		/* BLOCK - open code HmudInit(0) */
		{
		if ((dod.hmudUser = HAllocateCw(cwMUD)) == hNil)
			return fFalse;
		SetWords(*dod.hmudUser, 0, cwMUD);
		}

/* setup style sheet normal entries */
	if (!FCreateFastStsh(&dod.stsh, &dod.hsttbChpe, &dod.hsttbPape))
		return fFalse;

	blt (&dod, PdodDoc (docNew), cwDOD);
	return fTrue;
}



/* W A R N I N G */
/* This routine is a hard-coded version of FCreateStshNormalStc in stysubs.c. */

csconst char stStsh1[] = { 
	0x05, 0x00, 0x18, 0x00, 0x00, 0x14 };


csconst char stStsh2[] = { 
	0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


/* %%Function:FCreateFastStsh %%Owner:PETERJ */
FCreateFastStsh(pstsh, phsttbChpe, phsttbPape)
struct STSH *pstsh;
struct STTB ***phsttbChpe, ***phsttbPape;
{
	struct STSH stsh;
	char st[10];
	struct ESTCP estcp;

	Assert(sizeof(st) >= sizeof(stStsh1));
	Assert(sizeof(st) >= sizeof(stStsh2));

	/* clear so frees at failure will work */
	SetBytes (&stsh, 0, sizeof (struct STSH));
	*phsttbChpe = *phsttbPape = hNil;

	if ((stsh.hsttbName = HsttbInit1(0, 1, 0, fTrue, fTrue)) == hNil ||
			IbstAddStToSttb(stsh.hsttbName, stEmpty) == ibstNil)
		goto ErrRet;

	CopyCsSt(stStsh1, st);

	if ((stsh.hsttbChpx = HsttbInit1(0, 1, 0, fTrue, fTrue)) == hNil ||
			IbstAddStToSttb(stsh.hsttbChpx, st) == ibstNil)
		goto ErrRet;
	if ((*phsttbChpe = HsttbInit1(0, 1, 0, fTrue, fTrue)) == hNil ||
			IbstAddStToSttb(*phsttbChpe, st) == ibstNil)
		goto ErrRet;

	CopyCsSt(stStsh2, st);

	if ((stsh.hsttbPapx = HsttbInit1(0, 1, 0, fTrue, fTrue)) == hNil ||
			IbstAddStToSttb(stsh.hsttbPapx, st) == ibstNil)
		goto ErrRet;

	if ((*phsttbPape = HsttbInit1(0, 1, 0, fTrue, fTrue)) == hNil ||
			IbstAddStToSttb(*phsttbPape, stEmpty) == ibstNil)
		goto ErrRet;

	estcp.stcBase = stcStdMin;
	estcp.stcNext = stcNormal;

	if ((stsh.hplestcp = HplInit(cbESTCP, 1)) == hNil || 
			!FInsertInPl(stsh.hplestcp, 0, (char *) &estcp))
		goto ErrRet;

#ifdef DEBUG
	if (vdbs.fCkDoc)
		/* make sure it is correct! */
		{
		struct STSH stsh2;
		struct STTB **hsttbChpe2, **hsttbPape2;
		struct PL *ppl1, *ppl2;
		int i;
		char rgch1[20];
		char rgch2[20];

		if (!FCreateStshNormalStc(&stsh2, &hsttbChpe2, &hsttbPape2))
			return fFalse;

		Assert(stsh.cstcStd == stsh2.cstcStd);

		CheckHsttbSame(stsh.hsttbName, stsh2.hsttbName);
		CheckHsttbSame(stsh.hsttbChpx, stsh2.hsttbChpx);
		CheckHsttbSame(stsh.hsttbPapx, stsh2.hsttbPapx);
		CheckHsttbSame(*phsttbChpe, hsttbChpe2);
		CheckHsttbSame(*phsttbPape, hsttbPape2);

		ppl1 = *stsh.hplestcp;
		ppl2 = *stsh2.hplestcp;
		Assert(ppl1->iMac == ppl2->iMac);
		Assert(ppl1->cb == ppl2->cb);
		Assert(ppl1->fExternal == ppl2->fExternal);
		Assert(ppl1->cb < 20);
		for (i = 0; i < ppl1->iMac; i++)
			{
			GetPl(stsh.hplestcp, i, rgch1);
			GetPl(stsh2.hplestcp, i, rgch2);
			Assert(!FNeRgch(rgch1, rgch2, ppl1->cb));
			}

		FreePhsttb(&stsh2.hsttbName);
		FreePhsttb(&stsh2.hsttbChpx);
		FreePhsttb(&hsttbChpe2);
		FreePhsttb(&stsh2.hsttbPapx);
		FreePhsttb(&hsttbPape2);
		FreePhpl(&stsh2.hplestcp);
		}

#endif /* DEBUG */

	bltb(&stsh, pstsh, sizeof (struct STSH));
	return fTrue;  /* OK */

ErrRet:
	/* FreePh/FreePhsttb tests for hNil before trying to free */
	FreePhsttb(&stsh.hsttbName);
	FreePhsttb(&stsh.hsttbChpx);
	FreePhsttb(phsttbChpe);
	FreePhsttb(&stsh.hsttbPapx);
	FreePhsttb(phsttbPape);
	FreePhpl(&stsh.hplestcp);
	return fFalse;

}



#ifdef DEBUG
/* %%Function:CheckHsttbSame %%Owner:PETERJ */
CheckHsttbSame(hsttb1, hsttb2)
struct STTB **hsttb1, **hsttb2;
{
	struct STTB *psttb1 = *hsttb1;
	struct STTB *psttb2 = *hsttb2;
	int i;
	char st1[20], st2[20];

	Assert(psttb1->ibstMac == psttb2->ibstMac);
	Assert(psttb1->cbExtraOld == psttb2->cbExtraOld);

	for (i = 0; i < psttb1->ibstMac; i++)
		{
		GetStFromSttb(hsttb1, i, st1);
		GetStFromSttb(hsttb2, i, st2);
		Assert(*st1 < 20 && *st2 < 20);
		Assert(!FNeSt(st1, st2));
		}
}


#endif /* DEBUG */


csconst char stCbtGlobalDot[] = St(szCbtGlobalDotDef);


/* F  I N I T  G L O B A L  D O T */
/*  Create a dot in docGlobalDot */
/* %%Function:FInitGlobalDot %%Owner:PETERJ */
FInitGlobalDot(fTutorial)
BOOL fTutorial;
{
	extern KMP ** hkmpCur;
	extern KMP ** vhkmpUser;
	extern KMP ** hkmpBase;
	extern MUD ** vhmudUser;

	int doc = docNil, fn = fnNil;
	int fose;
	struct DOD * pdod, ** hdod;
	CHAR stFile [ichMaxFile];
	CHAR stName [ichMaxFile];

	Assert (docGlobalDot == docNil);

	if (fTutorial)
		CopyCsSt(stCbtGlobalDot, stFile);
	else
		CopySt(stNormalDot, stFile);

	if (FFindFileSpec(stFile, stName, grpfpiDot, nfoDot))
		{
		/*  open the file if it exists */
		if (((fn = FnOpenSt (stName, fOstNativeOnly, ofcDoc, &fose)) != fnNil &&
				((doc = DocCreateFn (fn, fFalse, NULL, NULL, NULL)) == docNil ||
				!PdodDoc(doc)->fDot))
				|| (fn == fnNil && fose == foseBadFile))
			{
			if (doc != docNil)
				DisposeDoc(doc);
			doc = docNil;
			if (fn != fnNil)
				DeleteFn(fn, fFalse);
			fn = fnNil;
			ErrorEidW(eidBogusGdt, stName, "FInitGlobalDot");
			if (fTutorial)  /* CBT can't run with wrong Gdt */
				return fFalse;
			vmerr.mat = matNil; /* not a reason to fail to boot */
			}
		}

	if (doc == docNil)
		/* file does not exist or could not be read, clone a new one */
		doc = DocCloneDoc (docNew, dkDot);

	Assert (doc != docCancel);

	if (doc == docNil)
		/* real low on memory! bag out. */
		return fFalse;

	hdod = mpdochdod[doc];

	if (fn == fnNil)
		{
		(*hdod)->udt = udtGlobalDot;

		ApplyDocMgmtNew(doc, fnNil);

		RemoveKmp((*hdod)->hkmpUser);
		if (((*hdod)->hkmpUser = HkmpNew(0, kmfPlain)) == hNil)
			{
			goto LDisposeRet;
			}

		FreeHmud((*hdod)->hmudUser);
			/* BLOCK - open code HmudInit(0) */
			{
			if (((*hdod)->hmudUser = HAllocateCw(cwMUD)) == hNil)
				goto LDisposeRet;
			SetWords(*(*hdod)->hmudUser, 0, cwMUD);
			}
		}

	docGlobalDot = doc;
	pdod = *hdod;
	pdod->fPromptSI = fFalse;
	pdod->crefLock++;

	hkmpCur = vhkmpUser = pdod->hkmpUser;
	Assert(hkmpCur != hNil);
	(*hkmpCur)->hkmpNext = hkmpBase;

	vhmudUser = pdod->hmudUser;
	Assert(vhmudUser != hNil);
	return fTrue;

LDisposeRet:
	DisposeDoc(doc);
	return fFalse;
}


int XpStartupText(dxpWnd, cch)
int dxpWnd, cch;
{
	return (dxpWnd - (cch * vsci.dxpTmWidth)) >> 1;
}


/*  NOTE: Messages bound for this WndProc are filtered in wprocn.asm */

/* %%Function:StartWndProc %%Owner:PETERJ */
long EXPORT PASCAL StartWndProc(hwnd, wm, wParam, lParam)
HWND hwnd;
unsigned wm;
WORD wParam;
DWORD lParam;
{
	if (wm == WM_NCPAINT)
		{
		int ddyp = vsci.dypTmHeight;
		int dxpWnd;
		int yp;
		HDC hdc;
		struct RC rc;
		PAINTSTRUCT ps;
		LONG lRet;
		static BOOL fPainted = fFalse;

		/* changing the Opus menu during NewCurWw causes us to get a second
		paint message - ignore it! */
		if (fPainted)
			return fTrue;

		fPainted = fTrue;

		hdc = GetDC(hwnd);
		if (hdc == NULL)
			return fFalse;

		SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
		SetBkColor(hdc, GetSysColor(COLOR_WINDOW));

		GetClientRect(hwnd, &rc);

		/* paint background */
		SelectObject(hdc, vsci.hbrBkgrnd);
		PatBltRc(hdc, &rc, vsci.ropErase);

		/* paint the borders */
		lRet = DefWindowProc (hwnd, wm, wParam, lParam);

		/* fill in the text */
		dxpWnd = rc.xpRight - rc.xpLeft;

		yp = ddyp>>1;
		ExtTextOut(hdc, XpStartupText(dxpWnd, sizeof(szAppStartDef)-1), 
				yp, 0, (LPRECT)&rc, SzShared(szAppStartDef), 
				sizeof(szAppStartDef)-1, NULL);

		yp += ddyp>>2;

		yp += ddyp;
		ExtTextOut(hdc, XpStartupText(dxpWnd, sizeof(szVersionDef)-1), 
				yp, 0, (LPRECT)&rc, SzShared(szVersionDef), 
				sizeof(szVersionDef)-1, NULL);

		yp += ddyp;
		ExtTextOut(hdc, XpStartupText(dxpWnd, sizeof(szVerDateDef)-1), 
				yp, 0, (LPRECT)&rc, SzShared(szVerDateDef), 
				sizeof(szVerDateDef)-1, NULL);

		yp += ddyp>>1;

		yp += ddyp;
		ExtTextOut(hdc, XpStartupText(dxpWnd, sizeof(szCopyrightDef)-1),
				yp, 0, (LPRECT)&rc, SzShared(szCopyrightDef), 
				sizeof(szCopyrightDef)-1, NULL);

		yp += ddyp;
		ExtTextOut(hdc, XpStartupText(dxpWnd, sizeof(szCopyright2Def)-1),
				yp, 0, (LPRECT)&rc, SzShared(szCopyright2Def), 
				sizeof(szCopyright2Def)-1, NULL);

		ValidateRect(hwnd, NULL);
		ReleaseDC(hwnd, hdc);

		return lRet;
		}
	else
		{
		Assert(wm == WM_DESTROY);
		Assert(hwnd == vhwndStartup);
		vhwndStartup = NULL;
		return fTrue;
		}
	/* All other cases filtered out */
}


/* FUTURE bradch/bryanl(pj): can we do this at a better time to improve
performance (i.e., during FInitScreenConstants in initwin.c?) */

/* F C R E A T E  S Y S  M E N U */
/*  Create hbmpSystem. */
/* %%Function:FCreateSysMenu %%Owner:PETERJ */
FCreateSysMenu()
{
	extern HBITMAP hbmpSystem;

	BITMAP bmp;
	HBITMAP hbmp, hbmp2, hbmpSaveHdc, hbmpSaveHdc2, hbmpT;
	HDC hdc, hdc2, hdcApp;
	int dyMenu;

	hbmpSaveHdc = hbmpSaveHdc2 = hbmp = hbmp2 = hdcApp = hdc = hdc2 = NULL;

#ifdef WIN23
	if ((hbmp = LoadBitmap(NULL, OBM_OLD_CLOSE)) == NULL)
#else
	if ((hbmp = LoadBitmap(NULL, OBM_CLOSE )) == NULL)
#endif /* WIN23 */
		{
		ReportSz("OBM_CLOSE not available");
		goto LFail;
		}

	if ((hdcApp = GetDC(vhwndApp)) == NULL)
		goto LFail;

	GetObject(hbmp, sizeof (BITMAP), (LPSTR) &bmp);
	if ((hdc = CreateCompatibleDC(hdcApp)) == NULL)
		goto LFail;
	LogGdiHandle(hdc, 1071);
	if ((hbmpSaveHdc = SelectObject(hdc, hbmp)) == NULL)
		goto LFail;

	if ((hdc2 = CreateCompatibleDC(hdcApp)) == NULL)
		goto LFail;
	LogGdiHandle(hdc2, 1072);

	ReleaseDC(vhwndApp, hdcApp);

	dyMenu = GetSystemMetrics(SM_CYMENU);

	if ((hbmp2 = CreateCompatibleBitmap(hdc2, bmp.bmWidth / 2, dyMenu))
			== NULL)
		goto LFail;
	LogGdiHandle(hbmp2, 1008);
	if ((hbmpSaveHdc2 = SelectObject(hdc2, hbmp2)) == NULL)
		goto LFail;

	PatBlt(hdc2, 0, 0, bmp.bmWidth / 2, dyMenu, WHITENESS);

	BitBlt(hdc2, 0, (dyMenu - bmp.bmHeight) / 2,
			bmp.bmWidth / 2, bmp.bmHeight,
			hdc, vsci.dxpBmpSystem = bmp.bmWidth / 2, 0,
			SRCCOPY);

	/* cannot fail selecting stock object */
	hbmpT = SelectObject(hdc, hbmpSaveHdc);
	Assert(hbmpT != NULL);
	DeleteObject(hbmpT);

	/* cannot fail selecting stock object */
	hbmpSystem = SelectObject(hdc2, hbmpSaveHdc2);
	Assert(hbmpSystem != NULL);
	UnlogGdiHandle(hdc, 1071);
	DeleteDC(hdc);
	UnlogGdiHandle(hdc2, 1072);
	DeleteDC(hdc2);
	return fTrue;

LFail:
	if (hbmpSaveHdc != NULL)
		{
		Debug(hbmpT =) SelectObject(hdc, hbmpSaveHdc);
		Assert (hbmpT == hbmp);
		}
	/* no failure cases after hbmpSaveHdc2 successfully created */
	Assert(hbmpSaveHdc2 == NULL);

	if (hbmp != NULL)
		DeleteObject(hbmp);

	if (hdcApp != NULL)
		ReleaseDC(vhwndApp, hdcApp);

	if (hdc != NULL)
		{
		UnlogGdiHandle(hdc, 1071);
		DeleteDC(hdc);
		}

	if (hdc2 != NULL)
		{
		UnlogGdiHandle(hdc2, 1072);
		DeleteDC(hdc2);
		}

	if (hbmp2 != NULL)
		{
		UnlogGdiHandle(hbmp2, 1008);
		DeleteObject(hbmp2);
		}

	return fFalse;
}
