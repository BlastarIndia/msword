/*  Quit.c -- MW quit  commands (non-resident) */

#define RSHDEFS
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "ch.h"
#include "doc.h"
#include "file.h"
#include "props.h"
#include "disp.h"
#include "winddefs.h"
#include "screen.h"
#include "sel.h"
#include "doslib.h"
#include "fontwin.h"
#include "resource.h"
#include "print.h"
#include "debug.h"
#include "opuscmd.h"
#include "doslib.h"
#include "help.h"
#include "dde.h"
#include "preffile.h"
#include "message.h"
#include "rareflag.h"

#include "rerr.h"
#include "error.h"
#include "idle.h"
#include "automcr.h"

#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "rsb.h"

extern HRGN		vhrgnPage;
extern HRGN		vhrgnGrey;
extern int vwWinVersion;
extern struct SCI       vsci;
extern struct PRI       vpri;
extern struct STTB	**vhsttbFont;
extern struct MERR	vmerr;
extern struct FCE	rgfce[];
extern int              vfConversion;
extern struct FTI	vfti;
extern struct FTI	vftiDxt;
extern CHAR             szEmpty[];
extern int              fnMac;
extern struct FCB       **mpfnhfcb[];
extern struct WWD       **mpwwhwwd[];
extern struct MWD       **mpmwhmwd[];
extern struct WWD       **hwwdCur;
extern struct SEL       selCur;
extern int              wwCur;
extern int              wwMac;
extern int              mwMac;
extern struct DOD       **mpdochdod[];
extern int              docMac;
extern HFONT            vhfntStyWnd;
extern HWND             vhwndDeskTop;
extern HWND             vhwndApp;
extern HWND             vhwndRibbon;
extern HWND             vhwndStatLine;
extern struct RSBI	vrsbi;
extern struct SAB       vsab;
extern int              docGlobalDot;
extern int              vdocScratch;
extern int  	    	docRecord;
extern struct PREF      vpref;
extern HBITMAP          vhbmPatVert;
extern HBITMAP          vhbmPatHorz;
extern int              dclMac;
extern int              docDde;
extern CHAR             szApp[];
extern MSG              vmsgLast;
extern CHAR           **hstDMQPath;
extern struct DMFLAGS   DMFlags;
extern int		vflm;
extern struct AAB       vaab;
extern int              vfDeactByOtherApp;
extern BOOL             vfSingleApp;
extern BOOL             vfRestoreRibbon;
extern CHAR             szDoc[];
extern CHAR             szDot[];
extern IDF		vidf;
extern HWND				vhwndCBT;
extern WORD				wmWinHelp;
#ifdef WIN23
extern HFONT hfontStatLine;		/* Helv 12 pt. font for status line */
#endif /* WIN23 */
csconst CHAR szCSCmdLine[] = "winword.exe ^";
csconst CHAR szCSExtensions[] = "extensions";
csconst CHAR szCSRTF[] = ".RTF";
csconst CHAR szCSConversion[] = "Conversion";

/* F  A P P  C L O S E */
/* Handle App window WM_CLOSE message */
/* Returns True if it's ok to go ahead and exit the app, fFalse if
	the user cancelled. A true return will cause the caller (via DefWindowProc)
	to send a WM_DESTROY message to the app window */
/*  %%Function: FAppClose  %%Owner: bradch  */

EL BOOL FAppClose(ac)
uns ac;
{
	int ifce;
	CHAR szT[2];
	CHAR szCmdLine[sizeof(szCSCmdLine)+5];
	CHAR szExtensions[sizeof(szCSExtensions)];
	CHAR szExt[5];
	CHAR szConversion[sizeof(szCSConversion)];

	if (!FQueryEndSession(ac))
		return fFalse;


/* NOTE: None of the code below will be executed when windows is being
		shut down!  Anything that must be done when we are quitting 
		due to Windows closing should be done in FQueryEndSession()! */
/* BL: Except WriteUserState, which should be done by callers
   of FQueryEndSession. That's so we can do FRenderAll here before
   WriteUserState messes up the font chains. */

	/* Render Data BEFORE the world collapses around our ears */
	/* no reason to render data if under runtime windows */
	if (!vfSingleApp && vsab.fOwnClipboard && !FRenderAll(ac))
		{
		/* We are the clipboard owner -- render the clipboard 
			contents in all datatypes that we know about. */
		/* Render failed and user asked to abort close */
		return fFalse;
		}

	WriteUserState();

	/* kill the interrupt timer */
	KillTimer(vhwndDeskTop, tidInterrupt);

	vidf.fDead = fTrue;  /* So we don't repaint or idle anymore */

	/* begin the process of terminating all dde conversations */
	if (dclMac > 1)  /* some channel was opened during session */
		TerminateAllDdeDcl (fTrue/*fMacroToo*/);

#ifdef JBL
	CommSzNum( SzShared("vflm is: "),vflm );
#endif

	/* WriteUserState messed up the font chains.  Free font handles 
		and pretend that there are no fonts */

	ResetFont(fTrue);
	ResetFont(fFalse);

	for ( ifce = 0 ; ifce < ifceMax ; ifce++ )
		{
		HANDLE hfont = rgfce [ifce].hfont;

		if ((hfont = rgfce [ifce].hfont) != NULL && 
				hfont != hfontSpecNil)
			{
			UnlogGdiHandle(hfont, -1);
			DeleteObject(hfont);
			}
		rgfce [ifce].hfont = NULL;	/* just in case... */
		}

	vfti.pfce = NULL;
	vftiDxt.pfce = NULL;

	/* free memory DC, printer DC */
	SetFlm( flmIdle );

	/* write extensions to win.ini */
	bltbx(szCSExtensions, szExtensions, sizeof(szCSExtensions));
	bltbx(szCSCmdLine, szCmdLine, sizeof(szCSCmdLine));
	/* DOC */
	bltb(szDoc, szExt, 5);
	GetProfileString(szExtensions, &szExt[1], szEmpty, szT, 2);
	if (szT[0] == 0)
		{
		QszLower(szExt);
		SzSzAppend(szCmdLine, szExt);
		WriteProfileString(szExtensions, &szExt[1], szCmdLine);
		szCmdLine[sizeof(szCSCmdLine)-1] = 0;
		}
	/* DOT */
	bltb(szDot, szExt, 5);
	GetProfileString(szExtensions, &szExt[1], szEmpty, szT, 2);
	if (szT[0] == 0)
		{
		QszLower(szExt);
		SzSzAppend(szCmdLine, szExt);
		WriteProfileString(szExtensions, &szExt[1], szCmdLine);
		szCmdLine[sizeof(szCSCmdLine)-1] = 0;
		}
	/* RTF */
	CopyCsSz(szCSRTF, szExt);
	GetProfileString(szExtensions, &szExt[1], szEmpty, szT, 2);
	if (szT[0] == 0)
		{
		QszLower(szExt);
		SzSzAppend(szCmdLine, szExt);
		WriteProfileString(szExtensions, &szExt[1], szCmdLine);
		}

	/* put an Opus section in win.ini */
	CopyCsSz(szCSConversion, szConversion);
	GetProfileString(szApp, szConversion, szEmpty, szT, 2);
	if (szT[0] == 0)
		{
		Assert(vfConversion == fTrue || vfConversion == -1);
		WriteProfileString(szApp, szConversion,
				SzFrame("Yes"));
		}

	/* windows bug work around */
	Yield();
	DestroyWindow(vhwndApp);
	return fTrue;
}


/* A P P   D E S T R O Y */
/* Handle WM_DESTROY message sent to app window */
/*  %%Function: AppDestroy  %%Owner: peterj  */

AppDestroy()
{   /* Parent window is being destroyed */

	int ibms;
	struct BMS *pbms;
#ifdef WIN23
	int ibmsMax = vsci.fWin3 ? ibmsMax3 : ibmsMax2;
#endif /* WIN23 */

	Assert( vflm == flmIdle );

	if (vsci.mdcdScratch.hdc)
		{
		UnlogGdiHandle(vsci.mdcdScratch.hdc, 1073);
		DeleteDC( vsci.mdcdScratch.hdc);
		}

	if (vsci.mdcdBmp.hdc)
		{
		UnlogGdiHandle(vsci.mdcdBmp.hdc, 1074);
		DeleteDC( vsci.mdcdBmp.hdc);
		}

	for (ibms = 0, pbms = vrsbi.rgbms; ibms < ibmsMax; pbms++, ibms++)
		{
		if (pbms->hbm != NULL)
			{
			UnlogGdiHandle(pbms->hbm, 1080 + ibms);
			DeleteObject(pbms->hbm);
			}
		}

	if (wwCur != wwNil)
		FRemoveSystemMenu();

/* quit help if active */
	if (wmWinHelp)
		QuitHelp();
 
 	/* wait for all dde channels to be completely terminated */
 	if (dclMac > 1)  /* some channel was opened during session */
 		WaitDdeTerminated();

	vhwndApp = NULL;

	PostQuitMessage( 0 );	/* Posts WM_QUIT */
}


/* Q u i t  E x i t */
/* Handle WM_QUIT message posted to WinMain message loop */
/* The last thing we ever call; does not return */
/*  %%Function: QuitExit  %%Owner: peterj  */

QuitExit()
{
	extern HANDLE hPlaybackHook;

	DeleteGDIHandles();
	if (hPlaybackHook)
		RemovePlaybackHook ();
	Debug(QuitTrackGdiHandles());
	if (vfSingleApp Batch( || vfBatchMode ))
		OurExitWindows(); /* never returns */
/* else */
	CleanUpForExit();
	exit(vmsgLast.wParam);
}


/* C L E A N  U P  F O R  E X I T */
/*  Preform those cleanup actions that must be done no matter how
the user exists (i.e., DOS level, not windows level). 
*/
/*  %%Function: CleanUpForExit  %%Owner: peterj  */

CleanUpForExit()
{
	extern int vceliNest;

	if (vceliNest > 0)
		CleanupEl();
	if (vmerr.fSdmInit)
		EndSdm();/* free GDI stuff used by SDM, shouldn't call if haven't init SDM */

#ifdef RSH
	LogUatTimes();
	CloseUa();
#endif /* RSH */
	KillTempFiles(fTrue);
	EndEmm();
#ifdef DEBUG
	EndCover();
#endif /* DEBUG */
#ifdef HYBRID
	EndCover();
#endif /* HYBRID */
#ifdef DEBUG
	{
	extern int vfInt3Handler;
	if (vfInt3Handler)
	/* actually de-installs the int3handler */
		InstallInt3Handler();
	}
#endif /* DEBUG */
#ifdef HYBRID
	{
	extern BOOL vfShowSwapping;
	/* actually de-installs the int3fhandler */
	if (vfShowSwapping)InstallInt3FHandler();
	}
#endif /* HYBRID */
}



/* O U R  E X I T  W I N D O W S */
/*  %%Function: OurExitWindows  %%Owner: peterj  */
EXPORT OurExitWindows()
{
	FARPROC lpfn = NULL;

	if (vwWinVersion >= 0x0300)
		{
		lpfn = GetProcAddress(GetModuleHandle(SzShared("USER")),
			MAKEINTRESOURCE(idoExitWindowsV3));
		}
	CleanUpForExit();
	if (lpfn != NULL)
		{
		(*lpfn)((LPSTR)NULL); /* does ExitWindows in win 3 */
		}
	EXITWINDOWS((LPSTR)NULL);
	Assert(fFalse);
}


/* D e l e t e  G D I  H a n d l e s */
/* called at the very last minute to destroy GDI objects. Must be done
	AFTER all our DC's are gone, so that none of these objects are locked. */
/*  %%Function: DeleteGDIHandles  %%Owner: bradv  */
DeleteGDIHandles()
{
#ifdef WIN23
	extern struct BMI *mpidrbbmi;
#else
	extern struct BMI mpidrbbmi[];
#endif /* WIN23 */
	extern HBITMAP hbmpSystem;

	HBITMAP hbm;
	int idrb;
#ifdef WIN23
	int idrbMax = vsci.fWin3Visuals ? idrbMax3 : idrbMax2;
#endif /* WIN23 */

	if (vhfntStyWnd != NULL)
		{
		UnlogGdiHandle(vhfntStyWnd, 1013);
		DeleteObject (vhfntStyWnd);
		}


	UnlogGdiHandle(vsci.hbrBkgrnd, 1014);
	DeleteObject( vsci.hbrBkgrnd );
	UnlogGdiHandle(vsci.hbrText, 1015);
	DeleteObject( vsci.hbrText );
	UnlogGdiHandle(vsci.hbrDesktop, 1016);
	DeleteObject( vsci.hbrDesktop );
	UnlogGdiHandle(vsci.hbrBorder, 1017);
	DeleteObject( vsci.hbrBorder );
	UnlogGdiHandle(vsci.hbrScrollBar, 1018);
	DeleteObject( vsci.hbrScrollBar );
	UnlogGdiHandle(vsci.hpen, 1019);
	DeleteObject( vsci.hpen );
	UnlogGdiHandle(vsci.hpenBorder, 1020);
	DeleteObject( vsci.hpenBorder );
	UnlogGdiHandle(vhbmPatVert, 1021);
	DeleteObject( vhbmPatVert );
	UnlogGdiHandle(vhbmPatHorz, 1022);
	DeleteObject( vhbmPatHorz );
#ifdef WIN23
	if (vsci.fWin3Visuals )
		{
		DeleteObject(vsci.hbrLitButton);
		DeleteObject(vsci.hbrButtonText);
		}
	if (hfontStatLine)
		DeleteObject(hfontStatLine);
#endif /* WIN23 */
	for ( idrb = 0; idrb < idrbMax; idrb++ )
		if ((hbm = mpidrbbmi [idrb].hbm) != NULL)
			{
			UnlogGdiHandle(hbm, -1);
			DeleteObject( hbm );
			}

	if (hbmpSystem != NULL)
		{
		UnlogGdiHandle(hbmpSystem, 1008);
		DeleteObject( hbmpSystem );
		}

/* Delete the regions used for pageview */
	if (vhrgnPage != NULL)
		{
		UnlogGdiHandle(vhrgnPage, 1024);
		DeleteObject( vhrgnPage );
		}
	if (vhrgnGrey != NULL)
		{
		UnlogGdiHandle(vhrgnGrey, 1025);
		DeleteObject( vhrgnGrey );
		}
}


/* K I L L   T E M P   F I L E S */
/* Kill off all of the temp files. App cannot run after this is done */
/*  %%Function: KillTempFiles  %%Owner: peterj  */

KillTempFiles( fEndSession )
int fEndSession;
{

	int fn;
	struct FCB **hfcb;

	/* close all files, even the ones on nonremovable media */
	CloseEveryFn( fTrue /* fHardToo */ );

	/* Delete all temp files */

	/* loop thru the FCB table looking for files that should be deleted before
		we quit. */
	for (fn = 1 + !(vmerr.fScratchFileInit && !vmerr.fPostponeScratch); 
			fn < fnMac; fn++)
		if ((hfcb=mpfnhfcb [fn]) != hNil)
			{
			struct FCB *pfcb = *hfcb;

			if (pfcb->fTemp || pfcb->fDMEnum)
				/* Having found a file that must be deleted, delete it */
				{
				EcOsDelete(pfcb->stFile, 0);
				}
			}
}



/* F  C O N F I R M  S A V E  A L L  D O C S */
/* for each dirty document, prompt the user, asking whether to save
	changes.  Return fFalse if the user hit "Cancel" in response to
	one of the prompts, or if a requested save failed; fTrue otherwise */
/*  %%Function: FConfirmSaveAllDocs  %%Owner: peterj  */

FConfirmSaveAllDocs(ac)
uns ac;
{
	extern void ** vhmes; /* really a MES ** */
	struct DOD *pdod;
	char mpdoccPass [docMax];
	int cPass;
	int cPassLast = 0;
	int doc;
	int wwDisp;

/* Document saves are done in the following order:
		(1) Non document windows (header, macro, etc.)
		(2) Current document
		(3) Current document's document type
		(4) Other dirty documents
		(5) Other dirty document types
		(6) Global document type
*/

	SetBytes( mpdoccPass, 0, sizeof(mpdoccPass) );

	SaveAllHeaders(); /* make sure header has been saved first */

	if (ac != acNoSave && vhmes != hNil && !FConfirmSaveMacros(ac))
		return fFalse;

/* Now determine order in which other docs will be saved */
	for ( doc = docMinNormal; doc < docMac; doc++ )
		{
		if (mpdochdod [doc] != hNil && 
				((pdod = PdodDoc(doc))->fDoc || pdod->fDot) &&
				DiDirtyDoc(doc) && 
				(pdod->fn != fnNil || pdod->fUserDoc))
			{
			Assert(!pdod->fShort);

			if (pdod->udt == udtDocument && CpMacDocEdit(doc) == 0
					&& pdod->docHdr == docNil && !(DiDirtyDoc(doc) & dimStsh))
			/* new document where all the new text has been deleted, don't
				bother with it */
				continue;

		/* Doc needs saving; let's see in what order */
			if (doc == DocMother(selCur.doc))
				cPass = 1;
			else  if (doc == docGlobalDot)
				cPass = 5;
			else  if (mpdochdod [selCur.doc] != hNil &&
					PdodDoc(selCur.doc)->fDoc &&
					PdodDoc(selCur.doc)->docDot == doc)
				cPass = 2;
			else  if (pdod->fDoc)
				cPass = 3;
			else
				cPass = 4;

			cPassLast = max( cPass, cPassLast );
			mpdoccPass [doc] = cPass;
			}
		}

/* Save all dirty documents in cPass order, prompting for each */

	for ( cPass = 1; cPass <= cPassLast; cPass++ )
		for ( doc = 1; doc < docMac; doc++ )
			if (mpdoccPass [doc] == cPass)
			/* Ask the user if he wants to save this document */
				if (!FConfirmSaveDoc(doc, fTrue /*fForce*/, ac))
					return fFalse;      /* User said "Cancel" */

	return fTrue;
}


/*  %%Function: SaveAllHeaders  %%Owner: chic  */
SaveAllHeaders()
{
	int doc;
	struct DOD **hdod, *pdod;

	for (doc = docMinNormal; doc < docMac; doc++)
		{
		if ((hdod = mpdochdod[doc]) != hNil && (pdod = *hdod)->fMother
				&& pdod->docHdr != docNil)
			FCleanUpHdr(doc, fFalse, fFalse);
		}
}



/* W R I T E  U S E R  S T A T E */
/*  Save user session preferences on a file to be read when next started up */
/*  %%Function: WriteUserState  %%Owner: bryanl  */
WriteUserState()
{
/* Write out view preferences into a file. */
	extern struct STTB **vhsttbOpen;
	extern CHAR szEmpty[];

	int fn, osfn;
	int ibst;
	int ifce,cfce;
	int fPrinterInfo = (vpri.hszPrinter != hNil && vpri.hszPrDriver != hNil &&
			!vmerr.fPrintEmerg);
	int ib;
	int cch;
	struct PREFD prefd;
	char stShort[50];
	char szFile[cchMaxFile];
	struct OFS ofs;

	Assert( (sizeof(struct PREFD) % sizeof(int)) == 0);
	SetWords( &prefd, 0, sizeof (struct PREFD)/sizeof(int) );

/* load up derived fields of prefd */
	vpref.fStatLine = (vhwndStatLine != NULL);
	vpref.fRibbon = (vhwndRibbon != NULL || vfRestoreRibbon);
	vpref.fZoomApp = (GetWindowLong(vhwndApp, GWL_STYLE) & WS_MAXIMIZE) ? 1 : 0;

	prefd.nPrefPrefdVer = nPrefPrefdVerCur;
	prefd.pref = vpref;
	prefd.cbSttbFileCache = CbToWriteH(vhsttbOpen);
	Assert(vhsttbOpen == hNil || !(*vhsttbOpen)->fExternal);
	prefd.cbStDMQPath = CbToWriteH(hstDMQPath);

	if (fPrinterInfo)
		{
		AssertH(vpri.hszPrinter);
		AssertH(vpri.hszPrDriver);
		prefd.cbPrNameAndDriver = 
				CchSz(*vpri.hszPrinter) + CchSz(*vpri.hszPrDriver);
		prefd.cbPrinterMetrics = bMaxPriFile - bMinPriFile;
		prefd.cbPrenv = CbToWriteH(vpri.hprenv);
		prefd.cbSttbFont = CbToWriteH(vhsttbFont);
		Assert(vhsttbFont == hNil || !(*vhsttbFont)->fExternal);
		prefd.cbSttbPaf = CbToWriteH(vpri.hsttbPaf );
		Assert(vpri.hsttbPaf == hNil || !(*vpri.hsttbPaf)->fExternal);
		cfce = CfcePackRgfce();
		prefd.cbRgfce = cfce * sizeof (struct FCE);
		prefd.cbFontWidths = 256 * sizeof(int) * cfce;
		}

/* find destination file */
	CopySt(StSharedKey("WINWORD.INI",OpusIni), stShort);
/* search path for existing */
	if (FFindFileSpec(stShort, szFile, grpfpiIni, nfoNormal))
		/* found existing, try to open to write */
		{
		StToSzInPlace(szFile);
		if ((osfn = OpenFile(szFile, &ofs, OF_READWRITE + bSHARE_DENYRDWR))
				>= 0)
			goto LHaveOsfn;
		}

/* did not find or could not open existing, create in ini-path or current */
	if (!FGetStFpi(fpiIniPath, szFile))
	/* no ini path, use program */
		AssertDo(FGetStFpi(fpiProgram, szFile));

	StStAppend(szFile, stShort);
	StToSzInPlace(szFile);

	if ((osfn = OpenFile(szFile, &ofs, OF_CREATE+OF_READWRITE+bSHARE_DENYRDWR))
			< 0)
		goto lblWimp;

LHaveOsfn:

	if (!FWriteCbToOsfn(osfn, sizeof(struct PREFD), &prefd)
			|| !FWriteHCbToOsfn(osfn, prefd.cbSttbFileCache, vhsttbOpen)
			|| !FWriteHCbToOsfn(osfn, prefd.cbStDMQPath, hstDMQPath))
		goto lblWimp;

	if (fPrinterInfo)
		{
		if (prefd.cbPrNameAndDriver &&
				(!FWriteHCbToOsfn(osfn, CchSz(*vpri.hszPrinter), vpri.hszPrinter) ||
				!FWriteHCbToOsfn(osfn, CchSz(*vpri.hszPrDriver), vpri.hszPrDriver)))
			goto lblWimp;

		if (       !FWriteCbToOsfn(osfn, prefd.cbPrinterMetrics, (char *)&vpri+bMinPriFile)
				|| !FWriteHCbToOsfn(osfn, prefd.cbPrenv, vpri.hprenv)
				|| !FWriteHCbToOsfn(osfn, prefd.cbSttbFont, vhsttbFont)
				|| !FWriteHCbToOsfn(osfn, prefd.cbSttbPaf, vpri.hsttbPaf)
				|| !FWriteCbToOsfn(osfn, prefd.cbRgfce, &rgfce))
			goto lblWimp;

		for ( ifce = 0 ; ifce < cfce ; ifce++ )
			if (rgfce [ifce].fPrinter && !rgfce [ifce].fFixedPitch)
				if (!FWriteHqCbToOsfn(osfn, 256*sizeof(int), rgfce[ifce].hqrgdxp))
					goto lblWimp;
		}

	FCloseDoshnd(osfn);
	return;

lblWimp:
/* file access failure if here */
	ErrorEidW(eidCantWriteFile, szFile, " WriteUserState");
	if (osfn >= 0)
		FCloseDoshnd(osfn);
}


/*  %%Function: CbToWriteH  %%Owner: peterj  */
CbToWriteH(h)
VOID **h;
{
	if (h != hNil)
		{
		AssertH(h);
		return CbOfH(h);
		}
	else
		return 0;
}


/*  %%Function: FWriteHCbToOsfn  %%Owner: peterj  */
FWriteHCbToOsfn(osfn, cb, h)
int osfn, cb;
char **h;
{
#ifdef DEBUG
	if (cb)
		AssertH(h);
#endif /* DEBUG */
	return (!cb || FWriteCbToOsfn(osfn, cb, *h));
}


/*  %%Function: FWriteCbToOsfn  %%Owner: peterj  */
FWriteCbToOsfn(osfn, cb, pch)
int osfn, cb;
char *pch;
{
	Assert(osfn >= 0);

	return (!cb || CchWriteDoshnd(osfn, (char far *)pch, cb) == cb);
}


/*  %%Function: FWriteHqCbToOsfn  %%Owner: peterj  */
FWriteHqCbToOsfn( osfn, cb, hq )
int osfn, cb;
HQ hq;
{
	char FAR *lpch;
	int cch;

	if (cb == 0)
		return fTrue;

	if ((lpch = LpLockHq( hq )) == NULL)
		return fFalse;
	cch = CchWriteDoshnd( osfn, lpch, cb );
	UnlockHq( hq );
	return (cch == cb);
}


/* R E M O V E  P L A Y B A C K  H O O K */
/*  %%Function: RemovePlaybackHook   %%Owner: peterj  */

RemovePlaybackHook ()

{
	extern HANDLE hPlaybackHook, far *lphrgbKeyState;
	extern HEVT far *lphevtHead;
	extern FARPROC lpfnPlaybackHook;

	/* send keys cleanup */
	Assert (hPlaybackHook);
	Assert(lphevtHead!=NULL);
	if (*lphevtHead)
		{
		UnhookWindowsHook(WH_JOURNALPLAYBACK,
				lpfnPlaybackHook);
		GlobalFree(*lphevtHead);
		if (*lphrgbKeyState)
			{
			SetKeyboardState(GlobalLock(*lphrgbKeyState));
			GlobalUnlock(*lphrgbKeyState);
			GlobalFree(*lphrgbKeyState);
			}
		*lphevtHead = *lphrgbKeyState = NULL;
		lphevtHead = NULL;
		}
	GlobalFree(hPlaybackHook);
	hPlaybackHook = NULL;
}


/* C f c e  P a c k  R g f c e */
/* rearrange rgfce so all printer fonts are at the beginning of the array */
/*  %%Function: CfcePackRgfce  %%Owner: bryanl  */

CfcePackRgfce()
{
	int ifceEye, ifcePen;
	struct FCE *pfceEye, *pfcePen;

	pfcePen = &rgfce [0];
	for ( ifcePen = 0; ifcePen < ifceMax ; ifcePen++,pfcePen++ )
		{
		if (!pfcePen->hfont || !pfcePen->fPrinter)
			{
			pfceEye = pfcePen + 1;
			for ( ifceEye = ifcePen + 1 ; ; ifceEye++,pfceEye++ )
				{
				if (ifceEye >= ifceMax)
/* no more printer fce entries after this screen fce entry therefore we're done */
					goto LRet;

				if (pfceEye->hfont && pfceEye->fPrinter)
					{
					struct FCE fceT;
/* cheat: don't leave the linked lists correct; we won't be using them again */

					fceT = *pfceEye;
					*pfceEye = *pfcePen;
					*pfcePen = fceT;
					break;
					}
				}
			}
		}
LRet:

	return ifcePen;
}


/* C M D   F I L E  E X I T */
/*  %%Function: CmdFileExit  %%Owner: bradch  */
CMD CmdFileExit(pcmb)
CMB *pcmb;
{
	SendMessage(vhwndApp, WM_CLOSE, 0, 0L);
	return cmdOK;
}


/*  %%Function: RunAtmCloseOnAllDocs  %%Owner: bradch  */
RunAtmCloseOnAllDocs()
{
	int doc;
	struct DOD * pdod;

	for (doc = docMinNormal; doc < docMac; doc += 1)
		{
		/* Is this a great condition, or what! */
		if (mpdochdod[doc] == hNil || /* no dod... */

				/* not a document or template... */
		(!(pdod = PdodDoc(doc))->fDoc && !pdod->fDot) ||

				/* not in a window... */
		pdod->wwDisp == wwNil ||

				/* no file and not a new document... */
		(pdod->fn == fnNil && !pdod->fUserDoc) ||

				/* new but empty document... */
		(pdod->udt == udtDocument && 
				CpMacDocEdit(doc) == 0 &&
				pdod->docHdr == docNil && 
				(DiDirtyDoc(doc) & dimStsh) == 0))
			{
			continue;
			}

		CmdRunAtmOnDoc(atmClose, doc);
		}
}


/*  %%Function: FQueryEndSession  %%Owner: bradch  */
FQueryEndSession(ac)
uns ac;
{
	extern BOOL vfRecording;

	Assert(ac != acAutoSave);

	/* Terminate Block or Extend or dyadic move mode if on */
	SpecialSelModeEnd();

	/* CBT shouldn't allow us to End our session; if for some reason
		it gets through don't really quit (like we've got a case in
		bug #2459 (opus10b) where type ahead allows it; possibly a CBT
		bug, but we can work around)  */

	if (vhwndCBT)
		return fFalse;

	if (vfRecording)
		{
		WORD id;

		switch (ac)
			{
		case acSaveAll:
		case acQuerySave:
			id = IdMessageBoxMstRgwMb(mstSaveMacroRecording, 
					NULL, mbQuerySave);
			break;

		case acNoSave:
			id = IDNO;
			break;

		case acSaveNoQuery:
			id = IDYES;
			break;
			}

		switch (id)
			{
			CMB cmb;

		case IDYES:
			GenStatement(bcmExit);
			cmb.cmm = cmmAction;
			CmdStopRecorder(&cmb);
			break;

		case IDNO:
			break;

		case IDCANCEL:
			return fFalse;
			}

		/* Updated in case user cancels the exit... */
		if (vhwndStatLine)
			UpdateStatusLine(usoNormal);
		}

	RunAtmCloseOnAllDocs();

	/* give user the option to save all dirty docs */
	if (!FConfirmSaveAllDocs(ac))
		return fFalse;

	CmdRunAtmOnDoc(atmExit, selCur.doc);
	return fTrue;
}


/* C O M M I T  S U I C I D E */
/*  %%Function: CommitSuicide  %%Owner: rosiep  */
/* We have had an unrecoverable error, and must kill ourselves.  Caller
   passes in either an fn of the file that we had an unrecoverable disk
   error on, or an sz to put up as a message instead (will use the same
   icon and other iemd fields from eidSDN).
*/

CommitSuicide(fn, sz)
int fn;
CHAR *sz;
{
	ReportSz("DEATH!");
	do
		{
		if (fn == fnNil)
			{
#ifdef BATCH
			if (vfBatchMode)
				BatchModeError(SzShared("DiskError! "), NULL, eidSDN, 0);
#endif /* BATCH */
			Assert(sz != NULL);
			ErrorEidSz(eidSDN, sz, "");
			}
		else
			DiskError(eidSDN, fn, "");
		}
		/* give the user a chance to save what is left... */
	while (SendMessage(vhwndApp, WM_CLOSE, acSaveAll, 0L));
	QuitExit();
	Assert(fFalse); /* never returns */
}
