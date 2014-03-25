/*
*  help.c
*
*  Interface to windows Help application.  Some of this was taken directly
*  from Excel.
*
*  05/12/87   rosiep   Wrote it
*  01/30/89   rosiep   New Help API from robertbu
*
*/

#ifdef WASTETHIS
#define NOSYSMETRICS
#define NOICON
/* #define NOSYSCOMMANDS */
#endif
#define NORASTEROPS
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOHDC
#define NOBRUSH
#define NOPEN
#define NOFONT
#define NOABOUT
#define NOWNDCLASS
#define NOCOMM
#define NOSOUND
#define NORESOURCE
#define NOOPENFILE
#define NOOPENFILE
#define NOWH

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "version.h"
#include "heap.h"
#include "doc.h"
#include "disp.h"
#include "keys.h"
#include "el.h"
#include "helpapi.h"
#include "help.h"
#include "message.h"
#include "prompt.h"
#include "debug.h"
#include "menu2.h"
#include "dde.h"
#include "file.h"
#include "doslib.h"
#include "macrocmd.h"
#include "wininfo.h"
#include "idd.h"
#include "sdmtmpl.h"
#include "sdmparse.h"
#include "about.hs"
#include "about.sdm"
#include "props.h"
#include "sel.h"
#include "error.h"
#include "ibdefs.h"
#include "screen.h"
#define RULER
#include "ruler.h"
#include "cmdtbl.h"
#include "rsb.h"
#include "rareflag.h"
#include "resource.h"
#include "sort.h"
#include "opuscmd.h"
#include "iconbar.h"
#include "search.h"
#include "rerr.h"

/* E X T E R N S */

extern BOOL			fElActive;
extern BOOL			vfDeactByOtherApp;
extern BOOL         vfRecording;
extern HANDLE       vhInstance;
extern struct MERR 	vmerr;
extern HWND         vhwndApp;          /* handle to parent's window */
extern HWND         vhwndDeskTop;
extern HCURSOR      vhcHelp;
extern HCURSOR      vhcArrow;
extern struct WWD 	**hwwdCur;
extern struct WWD 	**mpwwhwwd[];
extern struct MWD 	**mpmwhmwd[];
extern struct MWD 	**hmwdCur;
extern struct DOD 	**mpdochdod[];
extern int			wwMac;
extern int			mwMac;
extern struct SAB 	vsab;
extern int			dclMac;
extern int			docGlobalDot;
extern int			docGlsy;
extern uns          cbMemChunk;
extern KMP          **vhkmpUser;
extern KMP          **hkmpBase;
extern MUD          **vhmudUser;
extern HWND         vhwndStatLine;
extern HWND         vhwndRibbon;
extern HWND         vhwndPgPrvw;
extern int			vmwClipboard;
extern CHAR         szApp[];
extern int          vwWinVersion;
extern CHAR         szNone[];
extern struct PREF 	vpref;
extern long			vcmsecHelp;
extern WORD         vwParamHelp;
extern LONG         vlParamHelp;
extern int			wwCur;
extern int			mwCur;
extern int			vlm;
extern struct CA 	caTap;
extern struct TAP 	vtapFetch;
extern KMP          **hkmpCur;
extern int			viDocument;
extern int			viTemplate;
extern struct SEL 	selCur;
extern int			docMac;
extern struct STTB 	**vhsttbOpen;
extern struct SCI 	vsci;
extern CHAR         stEmpty[];
extern struct EFPI 	dnfpi[];
extern CHAR         vszSearch[];
extern CHAR         vszReplace[];
extern BOOL         vfSearchWord;
extern BOOL         vfSearchCase;
extern struct SOT 	sot;
extern int			viMenu;
extern struct RC 	vrcUnZoom;
extern BOOL			vfIconBarMode;
extern struct FB	vfbSearch;
extern struct FB	vfbReplace;

extern int	far pascal initwin_q();

/* G L O B A L S */

long	vcmsecHelp = 0L;        /* Time (in msec) to invalidate   */
/* double-clicking for help mode. */
WORD vwParamHelp;    			/* Saved for cxt computation */
LONG vlParamHelp = 0;    		/* Saved for cxt computation */
HWND vhwndCBT = NULL;      		/* CBT application window */
BOOL vfHelp = fFalse;			/* whether we're in Help mode */
int	vcxtHelp = cxtNil;          /* current context */
HANDLE hCbtLib;					/* handle to cbt library */
CHAR **hstzPath = hNil;
WORD wmWinHelp = 0;      		/* Registered message */
KMP **hkmpContextHelp = hNil;

struct CBTSAV **vhcbtsav;
int	CancelContextHelp();


/*  G E T  H E L P  */
/*  Get help on a particular context  */
/* %%Function:GetHelp %%Owner:rosiep */
EXPORT int GetHelp(cxt)
int	cxt;
{
	HWND LookForHelp();
	int	cmd = cmdContext;

	if (vfHelp)
		CancelContextHelp();

	/* If "can't find help" message is up, don't even bother trying... */
	if (cxt == CxtFromEid(eidCantFindHelp) || 
			/* ditto if we haven't finished booting yet... */
	vhwndApp == NULL || vrf.fInQueryEndSession)
		{
		Beep();
		return;
		}

	/* Set timer to block unwanted double-clicks in help mode */
	vcmsecHelp = (GetMessageTime() + (long) GetDoubleClickTime());

	if (cxt == cxtAppMenuSelect || cxt == cxtDocMenuSelect)
		cxt = CxtMenuSelect(vwParamHelp, vlParamHelp, cxt);

	if (cxt == 0)  /* no current context, bring up last one, or contents if none */
		cmd = cmdLast;

	if (viMenu == iMenuLongMin)
		{
		if (cxt == CxtFromBcm(bcmRecorder))
			cxt = cxtMacroRecordOnFile;
		else  if (cxt == CxtFromBcm(imiMacro))
			cxt = cxtMacroRunOnFile;
		}


	FHelp(cmd, (LONG) cxt /* ignored unless cmdContext */);
}


/*  Q U I T  H E L P  */
/* %%Function:QuitHelp %%Owner:rosiep */
QuitHelp()
{
	FHelp(cmdQuit, 0L);
}



/*******************
**
** Name:       HFill
**
** Purpose:    Builds a data block for communicating with help
**
** Arguments:  lpszHelp  - pointer to the name of the help file to use
**             usCommand - command being set to help
**             ulData    - data for the command
**
** Returns:    a handle to the data block or hNil if the the
**             block could not be created.
**
** Author:     robertbu
**
*******************/

/* %%Function:HFill %%Owner:rosiep */
HANDLE HFill(lpszHelp, usCommand, ulData)
LPSTR lpszHelp;
unsigned short	usCommand;
unsigned long	ulData;
{
	unsigned short	cb;                    /* Size of the data block           */
	HANDLE   hHlp;                        /* Handle to return                 */
	QHLP     qhlp;                        /* Pointer to data block            */

	/* Calculate size */
	cb = sizeof(HLP) + CchLpszLen(lpszHelp) + 1;
	if (usCommand == cmdKey)
		cb += CchLpszLen((LPSTR)ulData) + 1;

	/* Get data block */
	if (!(hHlp = OurGlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE | GMEM_NOT_BANKED,
			(DWORD)cb)))
		{
		return hNil;
		}

	if (!(qhlp = (QHLP)GlobalLock(hHlp)))
		{
		GlobalFree(hHlp);
		return hNil;
		}

	qhlp->cbData        = cb;             /* Fill in info                     */
	qhlp->usCommand     = usCommand;
	qhlp->ulReserved    = 0;
	qhlp->offszHelpFile = sizeof(HLP);
	CchCopyLpsz((LPSTR)(qhlp + 1), lpszHelp);

	if (usCommand != cmdKey)              /* If the keyword does not exist    */
		{                                   /*   then ulData must be topic numb */
		qhlp->offabData = 0;
		qhlp->ulTopic   = ulData;
		}
	else  /* Keyword -- add to data block     */		
		{
		qhlp->ulTopic   = 0L;
		qhlp->offabData = sizeof(HLP) + ((CchLpszLen(lpszHelp) >> 1) << 1) + 2;
		CchCopyLpsz((LPSTR)(qhlp + 1) + qhlp->offabData,  (LPSTR)ulData);
		}
	GlobalUnlock(hHlp);
	return hHlp;
}


/*******************
**
** Name:       FHelp
**
** Purpose:    Displays help
**
** Arguments:  lpszHelp        path (if not current directory) and file
**                             to use for help topic.
**             usCommand       Command to send to help
**             ulData          Data associated with command:
**                             cmdQuit     - no data (undefined)
**                             cmdLast     - no data (undefined)
**                             cmdContext  - context number to display
**                             cmdKey      - string ('\0' terminated)
**                                           use as keyword to topic
**                                           to display
**                             cmdFind     - no data (undefined)
**
** Returns:    TRUE iff success
**
** Author:     robertbu
**
*******************/

/* %%Function:FHelp %%Owner:rosiep */
BOOL PASCAL FHelp(usCommand, ulData)
unsigned short	usCommand;
unsigned long	ulData;
{
	HWND LookForHelp();
	HANDLE h;
	CHAR st[ichMaxFile];
	CHAR szHelpData[ichMaxFile];
	CHAR szRun[ichMaxFile];
	int	wRet = (usCommand == cmdQuit);    /* Return value                     */
	HWND hwndHelp;                        /* Handle of help's main window     */
	QHLP     qhlp;                        /* Pointer to the help structure    */
	static HANDLE hHlp = NULL;            /* handle to name of user help file */
	/*   it must be preserved because   */
	/*   help foolishly used the handle */
	/*   after the initial send message */

	CopySt(StSharedKey("WINWORD.HLP", HelpFileDef), st);
	if (FFindFileSpec(st, szHelpData, grpfpiUtil, nfoNormal))
		StToSzInPlace(szHelpData);
	else
		StToSz(st, szHelpData);

	if (!wmWinHelp)
		wmWinHelp = RegisterWindowMessage((LPSTR) szWINHELP);

	if (hHlp != NULL)
		GlobalFree(hHlp);

	/* Move Help file name to a handle  */
	if (!(hHlp = HFill((LPSTR) szHelpData, usCommand, ulData)))
		goto LDone;

	if (!(qhlp = (QHLP)GlobalLock(hHlp))) /* Setup handle for a FIND */
		goto LDone;

	qhlp->usCommand = cmdFind;
	GlobalUnlock(hHlp);

	if ((hwndHelp = LookForHelp(hHlp)) == NULL)
		{
		if (usCommand == cmdQuit)           /* Don't bother to load HELP just to*/
			{                                 /*  send it a quit message!         */
			goto LDone;
			}
		/* Can't find it --> launch it      */
		StartLongOp();
		CopySt(StSharedKey(szHelpAppDef, HelpAppDef), st);
		if (FFindFileSpec(st, szRun, grpfpiUtil, nfoNormal))
			StToSzInPlace(szRun);
		else
			StToSz(st, szRun);

		if ((h = RunApp(szRun, SW_SHOWNORMAL)) < 32
				|| (hwndHelp = LookForHelp(hHlp)) == NULL)
			{
			EndLongOp(fFalse);
			if (h == 8)  /* Note: bug in Windows - we will never actually get
						 			back 8 if oom failure, so we can't detect it here */
				goto LDone;

			if (vhwndCBT)
				{
				/* give CBT a chance to do something with no Help, or if
					they don't want to (they pass the SEMEV), terminate CBT */
				if (SendMessage(vhwndCBT, WM_CBTSEMEV, smvCantBootHelp, 0L))
					SendMessage(vhwndCBT, WM_CBTTERM, vhInstance, 0L);
				}
			else
				ErrorEid(eidCantFindHelp, "");
			return fFalse;
			}
		EndLongOp(fFalse);
		}
	Assert(hwndHelp != NULL && IsWindow(hwndHelp));
	/* Request topic display from help  */

	if (!(qhlp = (QHLP)GlobalLock(hHlp))) /* Setup handle for command         */
		goto LDone;

	qhlp->usCommand = usCommand;
	GlobalUnlock(hHlp);

	SendMessage(hwndHelp, wmWinHelp, vhwndApp, (LONG)hHlp);
	wRet = fTrue;

LDone:
	if (!wRet)
		{
		if (vhwndCBT)
			{
			/* give CBT a chance to do something with no Help, or if
				they don't want to (they pass the SEMEV), terminate CBT */
			if (SendMessage(vhwndCBT, WM_CBTSEMEV, smvCantBootHelp, 0L))
				SendMessage(vhwndCBT, WM_CBTTERM, vhInstance, 0L);
			}
		else
			ErrorNoMemory(eidNoMemHelp);
		}

	if ((usCommand == cmdQuit)            /* Get rid of handle because help   */
			&& (hHlp != NULL))                /*   is going away and this may be  */
		{                                   /*   the last FHelp call            */
		GlobalFree(hHlp);
		hHlp = NULL;
		}
	return wRet;
}


/*******************
**
** Name:       LookForHelp
**
** Purpose:    Finds the help app if it's already been booted
**
** Arguments:  hHlp            handle to path (if not current directory)
**                             and file to use for help topic.
**
** Method:     Enumerates all the windows sending a cmdFind message --
**             the window returning TRUE is assumed to be the Help
**
** Returns:    handle to help window or NULL if Help not found
**
** Author:     robertbu
**
*******************/

/* %%Function:LookForHelp %%Owner:rosiep */
HWND LookForHelp(hHlp)
HANDLE hHlp;
{
	HWND hwnd;

	Assert(vhwndApp != NULL);
	hwnd = GetWindow(vhwndApp, GW_HWNDFIRST);

	do
		{
		if (SendMessage(hwnd, wmWinHelp, vhwndApp, (LONG)hHlp))
			return hwnd;
		}
	while ((hwnd = GetWindow(hwnd, GW_HWNDNEXT)) != NULL);

	return NULL;
}





/* %%Function:CchCopyLpsz %%Owner:rosiep */
int	CchCopyLpsz(lpchDest, lpchSrc)
LPSTR lpchDest;
LPSTR lpchSrc;
{
	int	cch = 0;

	while ((*lpchDest++ = *lpchSrc++) != 0)
		cch++;
	return cch;
}


/* C X T   F R O M   W W   P T */
/* Returns appropriate help context, given a ww pt.  
* Used in WwPaneWndProc (wproc.c).
*/
/* %%Function:CxtFromWwPt %%Owner:rosiep */
int	CxtFromWwPt(ww, pt)
int	ww;
struct PT pt;
{
	struct PLDR **hpldr;
	int	idr;
	int	dl;
	struct SPT spt;
	struct PT ptT;

	ptT = pt;
	dl = DlWherePt(ww, &ptT, &hpldr, &idr, &spt, fTrue, fTrue);
	if (spt.fSelBar)
		return(cxtSelectionBar);
	return(CxtFromWw(ww));
}


/* C X T   F R O M   W W  */
/* Returns appropriate help context, given a ww.  Used
* in WwPaneWndProc (wproc.c) and called from CxtFromWwPt (above).
*/
/* %%Function:CxtFromWw %%Owner:rosiep */
int	CxtFromWw(ww)
int	ww;
{
	int	doc;

	if (vlm == lmPreview)
		return(cxtPrintPreview);
	switch (PwwdWw(ww)->wk)
		{
	case wkMacro:
		return(cxtMacroEditingWindow);
	case wkHdr:
		doc = (ww == wwCur) ? selCur.doc : PwwdWw(ww)->sels.doc;
		if (PdodDoc(doc)->ihdt >= ihdtMaxSep)
			return(cxtFootnoteSeparator);
		else
			return(cxtHeaderFooterPane);
	case wkFtn:
		return(cxtFootnotePane);
	case wkAtn:
		return(cxtAnnotationPane);
	case wkPage:
		return(cxtPageView);
	case wkOutline:
		return(cxtOutlineView);
	case wkDoc:
	default:
		if (vpref.fDraftView)
			return(cxtDraftView);
		else
			return(cxtNormalEditingView);
		}
}


/*  C X T  F R O M  I B C  */
/* %%Function:CxtFromIbc %%Owner:rosiep */
/*  Returns help context for the given Icon Bar.
    For most iconbars, this is trivial, but for header/footer icon bars,
	we have to determine whether it is really a header/footer or a
	footnote separator, since they both share the same ibc
*/

CxtFromIbcHwnd(ibc, hwnd)
int	ibc;
HWND hwnd;
{
	int	ww, doc;

	if (ibc == ibcHdr)
		{
		for (ww = wwDocMin; ww < wwMac; ww++)
			{
			if (PwwdWw(ww)->hwndIconBar == hwnd)
				break;
			}
		Assert(ww != wwMac);

		doc = (ww == wwCur) ? selCur.doc : PwwdWw(ww)->sels.doc;
		if (PdodDoc(doc)->ihdt >= ihdtMaxSep)
			{
			return(vfIconBarMode ? cxtFtnSepIconBarMode : 
					cxtFtnSepIconBar);
			}
		else
			{
			return(vfIconBarMode ? cxtHeaderFooterIconBarMode : 
					cxtHeaderFooterIconBar);
			}

		}

	/* Only iconbar mode help function that calls this is IBGetHelpHdr */
	Assert (!vfIconBarMode);
	return CxtFromIbc(ibc);
}


/*  C X T  M E N U  S E L E C T  */
/* %%Function:CxtMenuSelect %%Owner:rosiep */
/* Returns the help context for menu items.
	The LOWORD of lParam includes MF_SYSMENU if this is for either the
	Application control menu or the Document control menu.  It includes
	MF_POPUP only if the menu is hilighted but not dropped down (which
	you get if you hit Alt and then cursor left/right among the menus).
	vcxtHelp has been set up as cxtAppMenuSelect if we have hilighted
	something in the Application menu (a top level menu or anything in
	one of the drop-downs).  It is cxtDocMenuSelect if we have hilighted
	something in the Doc system menu.
*/
int	CxtMenuSelect(wParam, lParam, cxt)
WORD wParam;
LONG lParam;
int	cxt;
{
	int	mf = LOWORD(lParam);

	Assert(cxt == cxtAppMenuSelect || cxt == cxtDocMenuSelect);

	/* this case should have been taken care of in SaveMenuHelpContext */
	Assert (HIWORD(lParam) != 0 && wParam != 0);

	/* Application or Document system menu */
	if (mf & MF_SYSMENU)
		{
		if (mf & MF_POPUP)  /* hilighted but not dropped down */
			{
			return ((cxt == cxtAppMenuSelect) ? 
					cxtAppControl : cxtDocControl);
			}
			else  /* dropped down with an item selected; wParam contains
				the SC_ code of the menu item */			
			{
			return ((cxt == cxtAppMenuSelect) ? 
					CxtAppSysMenu(wParam) : 
					CxtDocSysMenu(wParam));
			}
		}

	/* a non-system menu is hilighted (but not dropped down) */
	/* HIWORD is the main menu handle; wParam is the submenu handle */
	if (mf & MF_POPUP)
		{
		int	imnu = ImnuFromHMenu(HIWORD(lParam), wParam);
#ifdef DEBUG
#define imnuHelp 11  /* stolen from menuhelp.c */
		/* give the right context for Help menu in debug version (debug
			menu comes first and throws the imnu off) */
		if (imnu == imnuHelp)
			imnu--;
#endif
		return (cxtMenuBar + imnu);
		}

	/* a non-system menu is dropped down and one of its items is */
	/* hilighted; wParam is the bcm of the menu command selected */
	else
		{
		/* MF_SYSMENU && !MF_POPUP case handled in CmdHelp */
		Assert (!(mf & MF_SYSMENU));
		return (CxtFromBcm(wParam));
		}
}


/* C X T  D O C  S Y S  M E N U */
/* %%Function: CxtDocSysMenu %%Owner: rosiep */
/* Returns context for help on any of the App system menu commands (or their
	icon equivalents).  Based on CxtAppSysMenu (below).
*/
CxtDocSysMenu(wParam)
WORD wParam;
{
	int	bcm;

	/* All selections from the control menu should fall through to
		the default case below; only screen regions (title bar, 
		maximize box, and window borders) will be picked out in 
		the case statement.  */

	switch (wParam & 0xFFF0)
		{
	case SC_MOVE:
		return cxtTitleBar;

	case SC_MAXIMIZE:
		return cxtDocMaximizeBox;

	case SC_SIZE:
		return cxtWindowBorder;

	case SC_CLOSE:
		return (CxtFromBcm(bcmCloseWnd));
#ifdef DEBUG
	case SC_MINIMIZE:
	case SC_RESTORE:
		Assert(fFalse);   /* Doc system menu has no Min or restore */
		return cxtIndex;  /* Something safe just in case */
#endif
	default:
		/* document control menu */
		if ((bcm = BcmFromSysMenu(wParam)) == bcmNil)
			return cxtNil;     /* No valid context */
		else
			return (CxtFromBcm(bcm));
		}
}


/* C X T  A P P  S Y S  M E N U */
/* %%Function: CxtAppSysMenu %%Owner: rosiep */
/* Returns context for help on any of the App system menu commands (or their
	icon equivalents).
	Only the high-order twelve bits of the wParam are used to determine the
	message; the low-order four bits are used to figure out what the user
	clicked on to produce the message.  This is NOT DOCUMENTED in the windows
	manuals, BUT... the low-order four bits are nonzero if the user clicked
	on the title bar or window borders, and are zero otherwise.
*/
CxtAppSysMenu(wParam)
WORD wParam;
{
	int	bcm;

	switch (wParam & 0xFFF0)
		{
	case SC_MOVE:
		if (wParam & 0x000F)   /* drag from title bar */
			return cxtTitleBar;
		else  /* control menu/Move */
			return cxtAppMove;

	case SC_MAXIMIZE:       /* Maximize or Restore */
		/* is the app already maximized? */
		if (GetWindowLong(vhwndApp, GWL_STYLE) & WS_MAXIMIZE)
			{
			if (wParam & 0x000F)   /* double-click on title bar */
				return cxtTitleBar;
			else  /* click on restore icon */
				return cxtRestoreBox;
			}
		else
			return cxtAppMaximize; /* menu/Maximize or maximize icon */

	case SC_MINIMIZE:          /* menu/Minimize or minimize icon */
		return cxtAppMinimize;

	case SC_RESTORE:           /* control menu/Restore */
		return cxtAppRestore;

	case SC_SIZE:
		if (wParam & 0x000F)   /* dragged window border */
			return cxtWindowBorder;
		else  /* control menu/Size */
			return cxtAppSize;

	case SC_CLOSE:             /* control menu/Close */
		return cxtAppClose;

	default:
		/* application control menu */
		if ((bcm = BcmFromSysMenu(wParam)) == bcmNil)
			return cxtNil;     /* No valid context */
		else
			return (CxtFromBcm(bcm));
		}
}



/* C M D  H E L P */
/* %%Function:CmdHelp %%Owner:rosiep */
/* Display help for current context. */
CMD CmdHelp(pcmb)
CMB *pcmb;
{
	int	cxt = vcxtHelp;  /* because EndMenu can blow away vcxtHelp */
	WORD wParamSav = vwParamHelp;  /* EndMenu resets these globals */
	LONG lParamSav = vlParamHelp;

	EndMenu();  /* end menu mode if we're in it */

	vwParamHelp = wParamSav;
	vlParamHelp = lParamSav;

	GetHelp(cxt);

	return cmdOK;
}



/* C M D  H E L P  C O N T E X T */
/* %%Function:CmdHelpContext %%Owner:rosiep */
CMD CmdHelpContext(pcmb)
CMB *pcmb;
{
	int	cxt = vcxtHelp;  /* because EndMenu can blow away vcxtHelp */
	int	mw;
	HWND hwnd;

	if (vcxtHelp && vcxtHelp != cxtIndex)
		{
		EndMenu();  /* end menu mode if we're in it */
		GetHelp(cxt);
		}
	else
		{
		if (vhcHelp == NULL && !FAssureIrcds(ircdsHelp))
			return cmdError;

		vfHelp = fTrue;

		/* disable the dialog portions of the ruler and ribbon */
		/* will disable given an id for any child of the dialog */
		if (vhwndRibbon != NULL)
			DisableIbdlgHwndIb(vhwndRibbon, IDLKSFONT, fTrue /*fDisable */);

		for (mw = mwDocMin; mw < mwMac; mw++)
			if (mpmwhmwd[mw] != hNil)
				if ((hwnd = (*mpmwhmwd[mw])->hwndRuler) != NULL)
					DisableIbdlgHwndIb(hwnd, idRulStyle, fTrue /*fDisable */);

		/* change cursor to indicate help mode; force change now,
			otherwise we'd have to wait for a mousemove message */
		OurSetCursor(vhcHelp);
		SetPromptMst(mstHelpMode, pdcMode);
		Assert(vhwndApp != NULL);

		/* Set up keymap */
		if ((hkmpContextHelp = HKmpNew(1, kmfPlain)) == hNil)
			return cmdNoMemory;
		AddKeyPfn (hkmpContextHelp, kcEscape, CancelContextHelp);
		}

	return cmdOK;
}


/* C A N C E L  C O N T E X T  H E L P */
/* Cancel context sensitive help mode */

/* %%Function:CancelContextHelp %%Owner:rosiep */
CancelContextHelp()
{
	int	mw;
	HWND hwnd;

	vfHelp = fFalse;
	ChangeCursor(fFalse);
	FreeIrcds(ircdsHelp);
	RestorePrompt();
	/* reenable the dialog portions of the ruler and ribbon */
	/* will disable given an id for any child of the dialog */
	if (vhwndRibbon != NULL)
		DisableIbdlgHwndIb(vhwndRibbon, IDLKSFONT, fFalse /*fDisable */);
	for (mw = mwDocMin; mw < mwMac; mw++)
		if (mpmwhmwd[mw] != hNil)
			if ((hwnd = (*mpmwhmwd[mw])->hwndRuler) != NULL)
				DisableIbdlgHwndIb(hwnd, idRulStyle, fFalse /*fDisable */);

	Assert(hkmpContextHelp != hNil);
	RemoveKmp(hkmpContextHelp);
	hkmpContextHelp = hNil;
}


/* C M D  H E L P  C O N T E N T S */
/* %%Function:CmdHelpContents %%Owner:rosiep */
CMD CmdHelpContents(pcmb)
CMB *pcmb;
{
	GetHelp(cxtIndex);
	return cmdOK;
}


/* C M D  H E L P  U S I N G  H E L P */
/* %%Function:CmdHelpUsingHelp %%Owner:rosiep */
CMD CmdHelpUsingHelp(pcmb)
CMB *pcmb;
{
	GetHelp(cxtHelpOnHelp);
	return cmdOK;
}


/* C M D  H E L P  C U R R E N T  V I E W */
/* %%Function:CmdHelpActiveWindow %%Owner:rosiep */
CMD CmdHelpActiveWindow(pcmb)
CMB *pcmb;
{
	if (wwCur != wwNil)
		GetHelp(CxtFromWw(wwCur));
	else
		GetHelp(cxtWordWindow);
	return cmdOK;
}


/* C M D  H E L P  K E Y B O A R D */
/* %%Function:CmdHelpKeyboard %%Owner:rosiep */
CMD CmdHelpKeyboard(pcmb)
CMB *pcmb;
{
	GetHelp(cxtKeyboard);
	return cmdOK;
}


/* C M D  H E L P  T U T O R I A L */
/* Bring up CBT Tutorial */
/* %%Function:CmdHelpTutorial %%Owner:rosiep */
CMD CmdHelpTutorial(pcmb)
CMB *pcmb;
{
	if (fElActive)
		{
		PostMessage(vhwndApp, WM_COMMAND, imiHelpTutorial, 0L);
		RtError(rerrHalt);
		Assert(fFalse); /* NOT REACHED */
		}
	
	if (FRunCBT(fFalse /* fStartup */))
		return cmdOK;
	else
		return cmdError;
}




/* C M D  A B O U T */
/* %%Function:CmdAbout %%Owner:rosiep */
CMD CmdAbout(pcmb)
CMB *pcmb;
{
	if (FCmdFillCab())
		{
		extern BOOL f8087;
		long	l;
		char	st[cchMaxSz];
		long (FAR PASCAL *lpfn)();

		/* values come from version.h */
		FSetCabSz(pcmb->hcab, szApp, 
				Iag(CABABOUT, hszAboutApp));
		FSetCabSz(pcmb->hcab, SzShared(szVersionDef), 
				Iag(CABABOUT, hszAboutVersion));
		FSetCabSz(pcmb->hcab, SzShared(szCopyrightDef), 
				Iag(CABABOUT, hszAboutCopyright));


		if (vwWinVersion >= 0x0300 && 
				(lpfn = GetProcAddress(GetModuleHandle(SzFrame("KERNEL")),
				MAKEINTRESOURCE(idoGetFreeSpace))) != NULL)
			/* use the windows 3 version */
			l = (*lpfn)(0) >> 10;
		else
			l = GlobalCompact(0L) >> 10;

		BuildStMstRgw(mstAboutKB, &l, st, cchMaxSz, hNil);
		FSetCabSt(pcmb->hcab, st, Iag(CABABOUT, hszAboutMem));

		if (smtMemory == smtLIM)
			{
			long	LcbUnusedEmm();
			l = (CbFreeEmm() + LcbUnusedEmm()) >> 10;
			BuildStMstRgw(mstAboutKB, &l, st, cchMaxSz, hNil);
			}
		else
			SzToSt(szNone, st);
		FSetCabSt(pcmb->hcab, st, Iag(CABABOUT, hszAboutEmem));

			/*  initialize the mathpack if we haven't done so already to get f8087 */
			{
			extern BOOL fInitialized;
			if (!fInitialized)
				InitMath();
			}

		FSetCabSz(pcmb->hcab, f8087 ? SzSharedKey("Present", Present) : 
				szNone, Iag(CABABOUT, hszAboutMath));

		l = LcbDiskFreeSpace(0 /* default drive */) >> 10;
		BuildStMstRgw(mstAboutKB, &l, st, cchMaxSz, hNil);
		FSetCabSt(pcmb->hcab, st, Iag(CABABOUT, hszAboutDisk));
		/* fMemFail will be true if FSetCabSt fails */
		if (vmerr.fMemFail)
			return cmdNoMemory;
		}

	if (pcmb->fDialog || pcmb->fAction)
		{
		char	dlt[sizeof(dltAbout)];

		BltDlt(dltAbout, dlt);
		if (TmcOurDoDlg(dlt, pcmb) == tmcError)
			{
			return cmdError;
			}
		}

	return cmdOK;
}





#ifdef NOT_USED_CURRENTLY

/* K C  F R O M  S Z  H E L P */

/* Given a global handle to a string which is a command name, return the
	kc of the first key which is assigned to that command.  This is so
	that Help can tell users what keys are mapped to various commands
	as they are mentioned in help topics.
*/

/* %%Function:KcFromSzHelp %%Owner:rosiep */
KcFromSzHelp(hsz)
HANDLE hsz;
{
	CHAR FAR * lpsz;
	CHAR FAR * lpch;
	CHAR st[cchMaxMacroName];
	int	cch = 0;
	int	kc;

	if ((lpsz = GlobalLockClip(hsz)) == NULL)
		return 0L;

	/* In-line version of FarSzToSz (found in stylesub.c) for swap-tuning */
	for (lpch = lpsz; *lpch != 0; cch++, lpch++)
		;
	bltbx(lpsz, (CHAR FAR * ) & st[1], cch);
	st[0] = cch;

	kc = KcFirstOfBcm(hkmpCur, BcmOfSt(st));
	GlobalUnlock(hsz);

	return (kc == kcNil ? 0 : kc);
}


/*  S Z   F R O M   K C   H E L P  */
/* We receive a key code (kc) and figure out the name of the function
	which that key is currently mapped to.  Return 0 if we can't get a
	function name (if the kc is illegal, or is an alphanumeric key, or
	something like that), return 1 if successful.  If successful, return
	the function name in the supplied global heap space (hsz).
	Help uses this function to tell the user which commands are mapped
	to which keys.
*/

/* %%Function:SzFromKcHelp %%Owner:rosiep */
SzFromKcHelp(kc, hsz)
int	kc;
HANDLE  hsz;
{
	char	far  *lpsz;
	KME       * pkme;

	if ((lpsz = GlobalLockClip(hsz)) == NULL)
		return (0);

	/* find the keymap entry for this kc */
	if ((pkme = PkmeOfKcInChain(kc)) == NULL)
		return (0);

	/* we're only interested in macros, not C functions or beeps */
	if (pkme->kt != ktMacro)
		return (0);

	/* get bcm information, put into vpsyFetch */
	FetchCm(pkme->bcm);
	if (!vpsyFetch)
		return (0);

	Assert ((int)vpsyFetch->stName[0] <= cchMaxMacroName);
	bltbx((vpsyFetch->stName + 1), lpsz, (int)vpsyFetch->stName[0]);
	lpsz[(int)vpsyFetch->stName[0]] = '\0';

	GlobalUnlock(hsz);
	return (1);
}



/* %%Function:FDlgAbout %%Owner:rosiep */
BOOL FDlgAbout(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	/* make ESC close the dialog, as ok does */
	if (dlm == dlmKey)
		if (wNew == VK_ESCAPE)
			EndDlg(tmcOK);
	return fTrue;
}


/* F  R U N  C B T */
/* Run CBT - return false if failed */
/* %%Function:FRunCBT %%Owner:rosiep */
#endif
FRunCBT(fStartup)
BOOL fStartup;    /* TRUE iff run from command line */
{
	MSG msg;
	HWND hwnd;
	int	mw, ww;
	CHAR stzPathOld[ichMaxFile+1];
	CHAR stzPathNew[ichMaxFile+1];
	char	*pchLim, *pch;
	HANDLE h;
	CHAR * *hPath;
	struct EFPI *pefpi;

	if (vhwndCBT)
		{
		Assert(fFalse);  /* CBT shouldn't be allowing this event */
		return fFalse;
		}
	
	if (vfDeactByOtherApp)
		WaitActivation();

	/* Initialize Opus to startup state so CBT can proceed from a known state */
	vrf.fRanCBT = fFalse;
	vrf.fRibbonCBT = 2;
	if (!FInitStateForCBT(fStartup))
		{
		if (vhcbtsav != hNil)
			{
			vpref = (*vhcbtsav)->prefSave;
			StopCBT();
			}
		return fFalse;
		}


LGetCurPath:
	/* second time through here is if different drive */
	/* if we're changing to a different drive, save the current drive
			letter and the current path on the new drive; restore both;
			if we're only changing directories on same drive, save old
			directory to restore.
	*/

	/* Save the current directory and switch to CBT directory */
	stzPathOld[0] = CchCurSzPath(&stzPathOld[1], 0) - 1;  /* don't count '\0' */
	stzPathOld[stzPathOld[0]--] = '\0';  /* blow away the trailing "\" */

	/*  Its very unlikely this alloc will fail since we just freed up all the
	docs, but if it does we will start CBT anyway, we just won't return
	to the same directory when we are done */
	if ((hstzPath = HAllocateCb(stzPathOld[0] + 2)) != hNil)
		{
		bltb(stzPathOld, *hstzPath, stzPathOld[0] + 2);
		if ((*vhcbtsav)->chDrive == 0)
			{
			if (!FFindFileSpec(StSharedKey(szCbtDirDef, CbtDir), stzPathNew,
					Grpfpi(fpiUtilPath, fpiProgram, fpiNil, fpiNil), nfoPathWildOK))
				{
				goto LCantFind;
				}
			stzPathNew[stzPathNew[0]+1] = '\0';

			/* is directory on different drive? */
			if (stzPathNew[1] != (*hstzPath)[1])
				{
				CHAR stzDrive[4];
				stzDrive[0] = 2;
				stzDrive[1] = stzPathNew[1];
				stzDrive[2] = ':';
				stzDrive[3] = '\0';
				/* change drive */
				if (!FSetCurStzPath(stzDrive))
					goto LCantFind;
				/* old drive to restore */
				(*vhcbtsav)->chDrive = (*hstzPath)[1];
				FreeH(hstzPath);
				/* now go back and change directory */
				goto LGetCurPath;
				}
			}

		if (!FSetCurStzPath(stzPathNew))
			{
LCantFind:
			ErrorEid(eidCantFindCbt, "");
			StopCBT();
			return fFalse;
			}

		}

	UpdateStDOSPath();
	/* stzPathNew now contains directory path "...\winword.cbt" */

	/* Set up DOT-PATH to find document templates in winword.cbt dir */

	/* add a '\' because it is expected in UpdateDOTList */
	stzPathNew[++stzPathNew[0]] = '\\';
	stzPathNew[stzPathNew[0]+1] = '\0';

	if ((hPath = HAllocateCb(stzPathNew[0] + 1)) == hNil)
		goto LFailedNoMem;

	pefpi = &dnfpi[fpiDotPath];
	if (!pefpi->fInit)
		InitFpi(fpiDotPath);

	SzToSt(&stzPathNew[1], *hPath);
	if (!pefpi->fNone)
		(*vhcbtsav)->hcDotPath = pefpi->hcPath;
	else
		(*vhcbtsav)->hcDotPath = hNil;
	pefpi->hcPath = HcCompactH(hPath);
	Assert(hPath == HExpandHc(pefpi->hcPath));
	pefpi->fNone = fFalse;


	/* Set up to use Tutorial's global document template */
	Assert(docGlobalDot != docNil);
	PdodDoc(docGlobalDot)->crefLock--; /* so DisposeDoc will work */
	DisposeDoc(docGlobalDot);
	docGlobalDot = docNil;
	docGlsy = docNil;

	if (!FInitGlobalDot(fTrue /* fTutorial */))
		{
		/* this is a hopeless situation, we cannot stay alive */
		StopCBT();
		/* Use sz; can't use eidSDN because it expects a normalized file
			and we don't have one we can normalize */
		CommitSuicide(fnNil, SzFrameKey("Unrecoverable disk error on file: NORMAL.DOT", UDENormalDot));
		}
	(*vhcbtsav)->fChangedGlobalDot = fTrue;


	/* Set up UTIL-PATH if user doesn't have one, so we can find Help */

	pefpi = &dnfpi[fpiUtilPath];
	if (!pefpi->fInit)
		InitFpi(fpiUtilPath);
	if (pefpi->fNone)
		{
		stzPathNew[0] = CchGetProgramDir(&stzPathNew[1], ichMaxFile);
		if ((hPath = HAllocateCb(stzPathNew[0] + 1)) == hNil)
			goto LFailedNoMem;
		CopySt(stzPathNew, *hPath);
		pefpi->hcPath = HcCompactH(hPath);
		Assert(hPath == HExpandHc(pefpi->hcPath));
		pefpi->fNone = fFalse;
		(*vhcbtsav)->fUtilPath = fFalse;
		}
	else
		(*vhcbtsav)->fUtilPath = fTrue;


	/* Reduce our swap size so CBT can run */
	ShrinkSwapArea();

	/* Load CBT dynamic library */
	if ((hCbtLib = RunApp(SzSharedKey(szCbtLibDef, CbtLib), SW_SHOW)) < 32)
		{
		if (hCbtLib == 8)
			goto LFailedNoMem;
		else
			goto LFailed;
		}

	/* Crank up CBT */
	vrf.fCbtInit = fFalse;
	if ((h = RunApp(SzSharedKey(szCbtDef, Cbt), SW_SHOWNORMAL)) < 32)
		{
		if (h == 8)
			{
LFailedNoMem:
			ErrorNoMemory(eidNoMemCBT);
			StopCBT();
			}
		else
			{
LFailed:
			ErrorEid(eidCantStartCBT, "");
			StopCBT();
			}
		return (fFalse);
		}

	/* Restore our swap size */
	GrowSwapArea();

	vhwndCBT = NULL;

	/* Wait till we hear back from them */
	while (!vrf.fCbtInit)
		{
		GetMessage((LPMSG) & msg, NULL, 0, 0);
		TranslateMessage((LPMSG) & msg);
		DispatchMessage((LPMSG) & msg);
		}

	if ((uns)vrf.fCbtInit > 1)
		{
		if (vmerr.fMemFail)  /* set in WM_SYSTEMERROR from CBT */
			goto LFailedNoMem;
		else
			goto LFailed;
		}

	/* Send init message to CBT */
	if ((vhwndCBT = FindWindow((LPSTR) SzSharedKey(szCbtWinDef, CbtWin), (LPSTR) NULL)) == NULL)
		goto LFailed;

	if (!SendMessage(vhwndCBT, WM_CBTINIT, vhInstance, CBT_OpusID))
		goto LFailed;

	/* We were successful, CBT is alive! */
	vrf.fRanCBT = fTrue;

	/* tell SDM that CBT is here */
	CBTState(fTrue);

	/* Tell CBT about all our currently open windows */
	Assert(vhwndApp != NULL);
	SendMessage(vhwndCBT, WM_CBTNEWWND, vhwndApp, 0L);
	if (vhwndStatLine != NULL)
		SendMessage(vhwndCBT, WM_CBTNEWWND, vhwndStatLine, 0L);

	for (mw = mwDocMin; mw < mwMac; mw++)
		{
		if (mpmwhmwd[mw] != hNil)
			if ((hwnd = (*mpmwhmwd[mw])->hwnd) != NULL)
				SendMessage(vhwndCBT, WM_CBTNEWWND, hwnd, 0L);
		}

	for (ww = wwDocMin; ww < wwMac; ww++)
		{
		if (mpwwhwwd[ww] != hNil)
			if ((hwnd = (*mpwwhwwd[ww])->hwnd) != NULL)
				SendMessage(vhwndCBT, WM_CBTNEWWND, hwnd, 0L);
		}


	return (fTrue);
}


/* S T O P  C B T */
/*  Reset all the stuff that gets set up when we run CBT.  This routine
	is designed to be able to work right even if the booting of CBT only
	got part way through.  I.e. it doesn't reset anything if it didn't
	get set in the first place.
*/

/* %%Function:StopCBT %%Owner:rosiep */
StopCBT()
{
	int	doc;
	struct DOD **hdod;
	int	ibst;
	char	stDoc[cchMaxFile];
	struct MWDSTATE mwdstate;
	struct EFPI *pefpi;

	Assert(vhcbtsav != hNil);
	if (hCbtLib >= 32)
		{
		FreeLibrary(hCbtLib);
		hCbtLib = 0;
		}

	/* Disable all further cbt stuff */
	vhwndCBT = (HWND) NULL;
	vmerr.fSentCBTMemErr = fFalse;

	/* tell SDM that CBT is gone */
	CBTState(fFalse);

	/* we may have the mouse captured if cbt canceled with a menu up */
	ReleaseCapture();

	if (vfRecording)
		FStopRecorder(fFalse /* fAskSave */, fTrue /* fForceKill */);

	if (vrf.fRanCBT)
		{
		/* close Print Preview if open */
		if (vhwndPgPrvw != NULL)
			EndPreviewMode();

		/**  close all mwds; don't save anything  **/
		for (doc = docMinNormal; doc < docMac; doc++)
			if ((hdod = mpdochdod[doc]) != hNil && !(*hdod)->fShort)
				CloseMwsOfDoc(doc);

		/* no active child window now */
		SetActiveMw(mwNil);
		}

	/* increase the swap area size */
	OurSetSas(sasFull);

	/* Restore the directory */
	if (hstzPath != hNil)
		{
		FSetCurStzPath(*hstzPath);
		FreeH(hstzPath);
		hstzPath = hNil;
		}

	/* Restore drive */
	if ((*vhcbtsav)->chDrive != 0)
		{
		CHAR stzDrive[4];
		stzDrive[0] = 2;
		stzDrive[1] = (*vhcbtsav)->chDrive;
		stzDrive[2] = ':';
		stzDrive[3] = '\0';
		FSetCurStzPath(stzDrive);
		(*vhcbtsav)->chDrive = 0;
		}

	UpdateStDOSPath();

	/* Restore the DOT-PATH */
	pefpi = &dnfpi[fpiDotPath];
	if (pefpi->fInit && !pefpi->fNone)
		{
		AssertH(HExpandHc(pefpi->hcPath));
		FreeH(HExpandHc(pefpi->hcPath));
		if ((*vhcbtsav)->hcDotPath != hNil)
			{
			pefpi->hcPath = (*vhcbtsav)->hcDotPath;
			(*vhcbtsav)->hcDotPath = hNil;
			}
		else
			pefpi->fNone = fTrue;
		}

	/* Restore the UTIL-PATH */
	pefpi = &dnfpi[fpiUtilPath];
	if (!(*vhcbtsav)->fUtilPath)
		{
		Assert(pefpi->fInit);
		AssertH(HExpandHc(pefpi->hcPath));
		FreeH(HExpandHc(pefpi->hcPath));
		pefpi->fNone = fTrue;
		}

	/* Restore global document template if necessary */
	if ((*vhcbtsav)->fChangedGlobalDot)
		{
		Assert(docGlobalDot != docNil);
		PdodDoc(docGlobalDot)->crefLock--;  /* so DisposeDoc will work */
		DisposeDoc(docGlobalDot);
		docGlobalDot = docNil;
		docGlsy = docNil;
        if (!FInitGlobalDot(fFalse /* fTutorial */))
			{
			/* this is a hopeless situation, we cannot stay alive */
			/* Use sz; can't use eidSDN because it expects a normalized file
				and we don't have one we can normalize */
			CommitSuicide(fnNil, SzFrameKey("Unrecoverable disk error on file: NORMAL.DOT", UDENormalDot));
			}
		}

	/**  restore previous doc #, template #, and mru file list  **/
	if ((*vhcbtsav)->iDocumentSav != -1)
		viDocument = (*vhcbtsav)->iDocumentSav;

	if ((*vhcbtsav)->iTemplateSav != -1)
		viTemplate = (*vhcbtsav)->iTemplateSav;

	if ((*vhcbtsav)->hsttbOpenSav != hNil)
		{
		FreeHsttb(vhsttbOpen);
		vhsttbOpen = (*vhcbtsav)->hsttbOpenSav;
		}

	/** destroy the status line, if user prefs indicate **/
	if (!(*vhcbtsav)->fStatLine && vhwndStatLine)
		{
		struct RC rc;

		DestroyStatLine();
		GetDeskTopPrc(&rc);
		MoveWindowRc(vhwndDeskTop, &rc, fTrue);
		}

	/** destroy the ribbon, if user prefs indicate **/
	if (!vrf.fRibbonCBT && vhwndRibbon)
		FTurnRibbonFOn(fFalse /*fOn*/, fTrue /*fAdjust*/, fFalse);

	/** restore or zoom the app window, if user prefs indicate **/
	if (GetWindowLong(vhwndApp, GWL_STYLE) & WS_MAXIMIZE)
		{
		if (!(*vhcbtsav)->fZoomed)
			SendMessage (vhwndApp, WM_SYSCOMMAND, SC_RESTORE, 0L);
		}
	else  if ((*vhcbtsav)->fZoomed == fTrue /* be explicit; tri-state flag */)
		SendMessage (vhwndApp, WM_SYSCOMMAND, SC_MAXIMIZE, 0L);

	vrcUnZoom = (*vhcbtsav)->rcUnZoom;

	/** turn the ribbon on, if user prefs indicate **/
	if (vrf.fRibbonCBT == fTrue /* tri-state flag */ && !vhwndRibbon)
		{
		if (FTurnRibbonFOn(fTrue /*fOn*/, fTrue /*fAdjust*/, fFalse /* fUpdate */))
			{
			InvalSelCurProps(fTrue);
			UpdateRibbon(fTrue);
			ShowWindow(vhwndRibbon, SHOW_OPENWINDOW);
			UpdateWindow(vhwndRibbon);
			}
		}
	vrf.fRibbonCBT = 2;

	/** turn the statusline on, if user prefs indicate **/
	if ((*vhcbtsav)->fStatLine == fTrue /* tri-state flag */ && !vhwndStatLine)
		FCreateStatLine();

	/* Restore state of draft view */
	if ((*vhcbtsav)->prefSave.fDraftView != vpref.fDraftView)
		CmdDraftView(NULL);  /* toggles */

	vpref = (*vhcbtsav)->prefSave;       /* restore old user preferences (first time) */

	/** open the mwds they had open when they started CBT, if possible **/
	if ((*vhcbtsav)->hsttbMwdSav != hNil)
		{
		struct STTB **hsttbMwdSav = (*vhcbtsav)->hsttbMwdSav;
		for (ibst = (*hsttbMwdSav)->ibstMac - 1 ; ibst >= 0 ; ibst--)
			{
			GetStFromSttb(hsttbMwdSav, ibst, stDoc);

			/** get extra bytes and copy immediately to local var **/
			bltbh(HpchExtraFromSttb(hsttbMwdSav, ibst),
					(char HUGE * ) & mwdstate, sizeof(struct MWDSTATE ));

			/** restore mwd preferences information **/
			vpref.fRuler = mwdstate.fRuler;
			vpref.fHorzScrollBar = mwdstate.fHorzScrollBar;
			vpref.fVertScrollBar = mwdstate.fVertScrollBar;

			if ((doc = DocOpenStDof(stDoc, dofNormal, &(mwdstate.rcMwd)))
					!= docNil)
				{
				/** move to their last selection, if we saved one **/
				if (mwdstate.sels.cpLim)
					{
					mwdstate.sels.doc = doc;  /* saved doc value is invalid */
					SetPselSels(&selCur, &(mwdstate.sels));
					}
				}
			}

		/* if there were no mw's open, open a empty document */
		if ((*hsttbMwdSav)->ibstMac == 0)
			{
LOpenEmpty:
			if (!vhwndPgPrvw && vrf.fRanCBT)
				{
				CHAR * stType = stEmpty;
				ElNewFile(stType, fFalse);
				}
			}
		}
	else
		goto LOpenEmpty;


	FreeHsttb((*vhcbtsav)->hsttbMwdSav);

	/* restore old user preferences again */
	vpref = (*vhcbtsav)->prefSave;
	ReadUserProfile(fFalse);  /* Reset Intl settings */
	FreePh(&vhcbtsav);
	vrf.fRanCBT = fFalse;
}


/* F  I N I T  S T A T E  F O R  C B T */
/* Initialize windows, menus, keymaps, etc. for CBT (it wants to start with
	a clean state.  We should be able to call StopCBT to clean up from this
	if it fails at any time, and not worry that we didn't get all the way
	through (i.e. StopCBT is smart enough to only clean up from the parts
	that have gotten done).
*/

/* %%Function:FInitStateForCBT %%Owner:rosiep */
FInitStateForCBT(fStartup)
BOOL fStartup;  /* true if started CBT from command-line with /T */
{
	int	doc;
	int	mw;
	int	ibst = 0;
	struct MWD **hmwd;
	struct WWD **hwwd;
	struct MWDSTATE mwdstate;
	struct STTB **hsttbMwdSav;
	HWND hwnd;
	char	stDoc[cchMaxFile];
	BOOL fSuccess = fTrue;

	Assert(!vmerr.fSentCBTMemErr);
	/* All fields in vhcbtsav should be initialized here to the state they
		should be in if initialization failed partway through, so that
		StopCbt can tell which things were done already and undo them. */
	Assert(vhcbtsav == hNil);
	if ((vhcbtsav = HAllocateCb(sizeof (struct CBTSAV ))) == hNil)
		return fFalse;
	(*vhcbtsav)->hsttbMwdSav = (*vhcbtsav)->hsttbOpenSav = hNil;
	(*vhcbtsav)->iDocumentSav = (*vhcbtsav)->iTemplateSav = -1;
	(*vhcbtsav)->fChangedGlobalDot = fFalse;
	(*vhcbtsav)->fStatLine = (*vhcbtsav)->fZoomed = 2;
	(*vhcbtsav)->hcDotPath = hNil;
	(*vhcbtsav)->chDrive = 0;
	(*vhcbtsav)->fUtilPath = fTrue;  /* assume true until proven otherwise */
	SetWords(&(*vhcbtsav)->rcUnZoom, 0, CwFromCch(sizeof(struct RC )));

	/* save user preferences - do this first (before any return)!! */
	(*vhcbtsav)->prefSave = vpref;

	/* terminate Help if it's running */
	QuitHelp();

	/* terminate all dde channels - don't wait for reply */
	if (dclMac > 0)
		TerminateAllDdeDcl (fTrue/*fMacroToo*/);

	if (!fStartup)
		{
		/** initialize vhsttbMwdSav:  state info for all open windows **/
		if ((hsttbMwdSav = HsttbInit1(0, mwMac - mwDocMin, sizeof(struct MWDSTATE ), fTrue, fFalse)) == hNil)
			return fFalse;
		(*vhcbtsav)->hsttbMwdSav = hsttbMwdSav;

		/** query user whether to save recording **/
		if (vfRecording && !FStopRecorder(fTrue /* fAskSave */, fFalse))
			return fFalse;

		/** query user whether to save each open file **/
		if (!FConfirmSaveAllDocs(acSaveAll))
			return fFalse;

		/* close Print Preview if open */
		if (vhwndPgPrvw != NULL)
			EndPreviewMode();

		/** save state info for the mw's, starting at the topmost one. **/
		mw = mwCur;         /* topmost mw */
		if (mw != mwNil)
			hwnd = (*mpmwhmwd[mw])->hwnd;
		while (mw != mwNil)
			{
			doc = ((hmwd = mpmwhmwd[mw]) == hNil) ? docNil : (*hmwd)->doc;

			/** find size & pos of current mw, save in mwdstate **/
			GetWindowRect(hwnd, &(mwdstate.rcMwd));
			ScreenToClient(vhwndDeskTop, &(mwdstate.rcMwd.ptTopLeft));
			ScreenToClient(vhwndDeskTop, &(mwdstate.rcMwd.ptBottomRight));

			/** get the next window's hwnd, we'll use it later; okay if NULL **/
			hwnd = GetNextWindow((*mpmwhmwd[mw])->hwnd, GW_HWNDNEXT);

			/* can't save and restore macro windows currently; too much
				work - do it in Cortex */
			if (PwwdWw(PmwdMw(mw)->wwActive)->wk == wkMacro)
				goto LKillIt;

			/** get the full path name of the doc in the current mw **/
			if (doc != docNil)
				GetDocSt(doc, stDoc, gdsoFullPath | gdsoNoUntitled);

			/** Save current selection in Doc pane, if any **/
			if (((hwwd = mpwwhwwd[(int)(*hmwd)->wwUpper]) == hNil) || 
					(*hwwd)->wk != wkDoc || DiDirtyDoc((*hwwd)->sels.doc))
				/** there is no doc pane selection, or user doesn't want
					to save changes, so the sel may be invalid; 
					just set sel to 0 **/
				mwdstate.sels.cpFirst = mwdstate.sels.cpLim = cp0;
			else
				/** save the current selection **/
				bltbx (&((*hwwd)->sels), &(mwdstate.sels), sizeof(struct SELS ));

			/** save mwd preferences information **/
			mwdstate.fHorzScrollBar = (*hmwd)->fHorzScrollBar;
			mwdstate.fVertScrollBar = (*hmwd)->fVertScrollBar;
			mwdstate.fRuler = ((*hmwd)->hwndRuler != NULL);

			/** save all this info only if we have a filename **/
			if (stDoc[0] != 0)
				if (!FInsStInSttb1(hsttbMwdSav, ibst++, stDoc, (char *) & mwdstate))
					return fFalse;

LKillIt:
			/* close the window (save top mw for last for aesthetics) */
			if (mw != mwCur)
				{
				if (!FCloseMw(mw, acNoSave))
					return fFalse;
				}
			mw = MwFromHwndMw(hwnd);   /* will be mwNil if hwnd == NULL */
			}

		/* now close top mw; do last so we don't have to display others intermediately */
		if (mwCur != mwNil)
			{
			if (!FCloseMw(mwCur, acNoSave))
				return fFalse;
			}

		/* no active child window now */
		SetActiveMw(mwNil);

		/* set no-doc menu */
		if (!FSetMinMenu())
			return fFalse;

		/* empty the clipboard of our stuff if any */
		if (vsab.fOwnClipboard)
			{
			struct CA ca;
			SetWholeDoc(docScrap, PcaSetNil(&ca));
			ChangeClipboard();
			}
		}


	/** Destroy ribbon, if necessary; save ribbon state (on or off) **/
	vrf.fRibbonCBT = fStartup ? vpref.fRibbon : (vhwndRibbon != NULL);
	if (vhwndRibbon != NULL)
		FTurnRibbonFOn(fFalse /* fOn */, fTrue /* fAdjust */, fFalse);

	/** create statline, if necessary; save state **/
	(*vhcbtsav)->fStatLine = fStartup ? vpref.fStatLine : (vhwndStatLine != NULL);
	if (vhwndStatLine == NULL)
		{
		if (!FCreateStatLine())
			{
			ErrorNoMemory(eidNoMemCBT);
			return fFalse;
			}
		else
			{
			struct RC rc;

			GetDeskTopPrc(&rc);
			MoveWindowRc(vhwndDeskTop, &rc, fTrue);
			MakeStatLineVisible();
			}
		}

	/** zoom the app window, if it's not already zoomed **/
	(*vhcbtsav)->fZoomed = (GetWindowLong(vhwndApp, GWL_STYLE) & WS_MAXIMIZE) && fTrue;  /* cast of long to BOOL */
	if (!(*vhcbtsav)->fZoomed)
		SendMessage(vhwndApp, WM_SYSCOMMAND, SC_MAXIMIZE, 0L);

	(*vhcbtsav)->rcUnZoom = vrcUnZoom;
	SetWords(&vrcUnZoom, 0, CwFromCch(sizeof(struct RC )));

	/* Turn off draft view if on */
	if (vpref.fDraftView)
		CmdDraftView(NULL);  /* toggles */

	/** save and reset doc #, template #, and mru file list **/
	/* subtract 1 first so they start out the same when incremented later */
	(*vhcbtsav)->iDocumentSav = viDocument - 1;
	viDocument = 0;
	(*vhcbtsav)->iTemplateSav = viTemplate - 1;
	viTemplate = 0;
	(*vhcbtsav)->hsttbOpenSav = vhsttbOpen;
	vhsttbOpen = hNil;     /* Flush MRU list, re-init in FInitDefaultPrefs */

	/* Set Intl settings to default (US) */
	ReadUserProfile(fTrue /* fForceDefault */);

	/* RESET GLOBAL DIALOG STATES */

	/* SEARCH/REPLACE */
	vfbSearch.fInfo = fFalse;
	vfbReplace.fInfo = fFalse;
	InitReplaceFormatting();
	vszSearch[0] = '\0';
	vszReplace[0] = '\0';
	vfSearchWord = fFalse;
	vfSearchCase = fFalse;

	/* SORT */
	sot.fDescending = fFalse;
	sot.skt = sktAlphaNum;
	sot.iSep = iSortSepComma;
	sot.uFieldNum = 1;
	sot.fColumnOnly = fFalse;
	sot.fSortCase = fFalse;

	fSuccess = FInitDefaultPrefs(fTrue);

	if (!fStartup)
		GlobalLruOldest(GetCodeHandle((FARPROC)initwin_q));

	return fSuccess;
}




/* These two arrays contain counts of buttons (cbtn) on the ruler and the
	outline iconbar.  They are used to divide the buttons on these two 
	iconbars into groups, so that we can pass the group number and button
	index to CBT.  The other four iconbars are not subdivided into button
	groups; they just pass an index.     -awe
*/
csconst int rgcbtnRuler[] = {
	cbtnRulerAlign,
			cbtnRulerSpacing,
			cbtnRulerTabs
};


#define icbtnRulerMax  3

csconst int rgcbtnOutline[] = {
	cbtnOutlinePromote,
			cbtnOutlinePlus
};


#define icbtnOutlineMax  2

/* L p a r a m C B T   F r o m   I b c   I i b b
	This function will compute the Lparam to send to the CBT application
	with the Icon Bar semantic events.  The Lparam specifies which icon bar
	button was clicked on in a form CBT understands.  This function is called
	from IconBarWndProc (in iconbar1.c), but is located here because
	iconbar1.c is in the core, and this function is only used in the rare
	case when CBT is running.
*/
/* %%Function:LparamCBTFromIbcIibb %%Owner:rosiep */
long	LparamCBTFromIbcIibb(ibc, iibb)
int	ibc;        /* identifies the type of iconbar */
int	iibb;       /* identifies the button */
{
	int	icbtn;    /* identifies the button group */
	int	*rgcbtn = rgcbtnOutline;
	int	icbtnMax = icbtnOutlineMax;

	Assert (ibc >= ibcHdr && ibc <= ibcPreview);

	/* idRulMark happens to be the largest button index as of 8/26/88 */
	Assert (iibb >= 0 && iibb <= idRulMark);

	switch (ibc)
		{
	case (ibcRuler):
		iibb--;       /* index zero is a dialog box, not a button */
		rgcbtn = rgcbtnRuler;
		icbtnMax = icbtnRulerMax;

		/* fall through */

	case (ibcOutline):
		for (icbtn = 0; icbtn < icbtnMax && iibb >= rgcbtn[icbtn]; icbtn++)
			iibb -= rgcbtn[icbtn];
		break;

	case (ibcRibbon):
		iibb--;       /* index zero is a dialog box, not a button */

		/* fall through */

	default:
		icbtn = 0;
		}
	/* we return the button index and the button group number as a long */
	return (MAKELONG(iibb, icbtn));
}


/*  C B T   S e l e c t   P s e l
	This function sends CBT a message describing the current selection.
*/
/* %%Function:CBTSelectPsel %%Owner:rosiep */
CBTSelectPsel(psel)
struct SEL *psel;
{
	struct DOD *pdod;
	int	smv;

	/* CBT won't be using any long files, so we can
			represent the entire selection as one LONG; that
			is how they will interpret the lParam of this
			message */
	Assert ((int) psel->cpFirst == psel->cpFirst);
	Assert ((int) psel->cpLim == psel->cpLim);

	pdod = PdodDoc(psel->doc);
	smv = (pdod->fAtn ? smvAnnSelection : 
			(pdod->fFtn ? smvFNSelection : 
			(pdod->fDispHdr ? (FHeaderIhdt(pdod->ihdt) ? 
			smvHdrSelection : smvFtrSelection) :
			smvSelection    /* default */
	)));
	Assert (vhwndCBT);
	SendMessage(vhwndCBT, WM_CBTSEMEV, smv,
			MAKELONG((int)psel->cpFirst, (int)psel->cpLim));
}


/*  C B T   T b l   S e l e c t   P s e l
	This function sends CBT a message describing the current selection in
	a table.
*/
/* %%Function:CBTTblSelectPsel %%Owner:rosiep */
CBTTblSelectPsel(psel)
struct SEL *psel;
{
	CP cp;
	int	iRowMin = 0;
	int	iRowMac;
	int	loWord;
	int	itcLim;

	if (!psel->fTable)
		{
		CBTSelectPsel(psel);
		return;
		}

	/** CBT promises that we won't use any long files; we
			can represent character positions in 16 bits.     **/
	Assert ((int) psel->cpFirst == psel->cpFirst);
	Assert ((int) psel->cpLim == psel->cpLim);

	/* find the row number of the start of the selection */
	cp = CpTableFirst(psel->doc, psel->cpFirst);
	while (cp < psel->cpFirst)
		{
		CpFirstTap(psel->doc, cp);
		cp = caTap.cpLim;
		iRowMin++;
		}

	/* find the row number of the end of the selection */
	iRowMac = iRowMin;
	while (cp < psel->cpLim)
		{
		CpFirstTap(psel->doc, cp);
		cp = caTap.cpLim;
		iRowMac++;
		}

	itcLim = (psel->itcLim < itcMax) ? psel->itcLim : vtapFetch.itcMac;

	Assert (iRowMac < 16);              /* 4-bit maximum */
	Assert (psel->itcFirst < 16);       /* 4-bit maximum */
	Assert (itcLim < 16);               /* 4-bit maximum */
	Assert (vhwndCBT);

	/* Pack table selection dimensions into loWord */
	loWord = (iRowMin << 12) | (iRowMac << 8) | (psel->itcFirst << 4) | 
			(itcLim);
	SendMessage(vhwndCBT, WM_CBTSEMEV, smvTableSelection, 
			MAKELONG(loWord, (int)psel->cpFirst));
}


/* F   C B T   C h a n g e   T a b
	Send CBT a message explaining how we are going to alter the tabs; we
	return fFalse if CBT wants to abort the change.
*/
/* %%Function:FCBTChangeTab %%Owner:rosiep */
BOOL FCBTChangeTab(xa, xaOld)
int	xa;         /* new coordinates of tab */
int	xaOld;      /* previous coordinates of tab */
{
	int	smv;

	Assert (vhwndCBT);
	Assert ((int)xa == xa || xa == xaNil);
	Assert ((int)xaOld == xaOld || xaOld == xaNil);
	Assert (!(xa == xaOld && xa == xaNil));

	/* we either created, deleted, or moved the tab */
	smv = (xaOld == xaNil ? smvTabCreate : 
			(xa == xaNil ? smvTabDelete : smvTabMove));

	/* we must be careful not to pass xaNil to CBT; pass 0 instead */
	return(SendMessage(vhwndCBT, WM_CBTSEMEV, smv, 
			MAKELONG((xaOld == xaNil ? 0 : xaOld), (xa == xaNil ? 0 : xa))));
}


/* F   C B T   C h a n g e   R u l e r
	Send CBT a message explaining how we are going to move ruler marks;
	we return fFalse if CBT wants to abort the change
*/

/* %%Function:FCBTChangeRuler %%Owner:rosiep */
BOOL FCBTChangeRuler(imk, xa, xaOld, fLockOnLeft1)
int	imk, xa, xaOld;
BOOL fLockOnLeft1;     /* left indent and first line indent moved together */
{
	Assert (vhwndCBT);
	Assert ((int)xa == xa || xa == xaNil);
	Assert ((int)xaOld == xaOld || xaOld == xaNil);
	Assert (imk >= imkIndLeft1 && imk < imkNormMax && imk != imkDefTab);

	/* we must be careful not to pass xaNil to CBT; pass 0 instead */
	return(SendMessage(vhwndCBT, WM_CBTSEMEV, (imk == imkIndLeft && 
			fLockOnLeft1) ? smvIndentBoth : SmvFromImk(imk),
			MAKELONG((xaOld == xaNil ? 0 : xaOld), (xa == xaNil ? 0 : xa))));
}




/* %%Function:LRSBHelp %%Owner:rosiep */
long	LRSBHelp(hwnd)
HWND hwnd;
{
	int	cxt = cxtScrollBar;

	switch (GetWindowWord(hwnd, offset(RSBS, grpfWnd)))
		{
	case maskWndSplitBox:
		cxt = cxtSplitBox;
		break;
	case maskWndSizeBox:
		cxt = cxtSizeBox;
		break;
	case maskWndPgVw:
		cxt = cxtPageViewScrollIcon;
		break;
		}
	GetHelp(cxt);
	return(0L);
}


#ifdef TEXTEVENT
/* This is disabled for Opus 1.0, but in the future CBT might want
   to teach the user to drag to a specific measurement, in which
   case we need to re-enable this code and they need to author in
   a Semantic Event (Text) pass/compare - called from ShowXaYaFromPt
   and ShowPicSize. */
/*  C B T  T E X T  E V E N T  */
/* %%Function:CbtTextEvent %%Owner:rosiep */
/* Send CBT a text event message */
CbtTextEvent(pch, cch)
char	*pch;
int	cch;
{
	HANDLE hText;
	CHAR FAR * qText;

	if (!(hText = OurGlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE | GMEM_NOT_BANKED,
			(long)(uns)(cch + 4 /* four zeros */ + 1 /* null term */))))
		return;
	if (!(qText = (CHAR FAR * ) GlobalLock(hText)))
		{
		GlobalFree(hText);
		return;
		}

	/* Build up special Text event string to pass to CBT:
		first four bytes are 0 (place-holders for starting
		and ending character (normally used by SDM));
		followed by the string, zero terminated. */

	SetBytes(qText, 0, 4);
	bltbx((CHAR FAR * )pch, qText + 4, cch);
	*(qText + cch + 4) = '\0';
	GlobalUnlock(hText);
	SendMessage(vhwndCBT, WM_CBTSEMEV, SE_TEXT, MAKELONG(0, hText));
	GlobalFree(hText);
}
#endif /* TEXTEVENT */

