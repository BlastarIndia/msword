/* W P R O C . C */

#ifdef DEBUG
/*#define SHOWAPPMSG*/
/*#define SHOWDSKMSG*/
/*#define SHOWDOCMSG*/
/*#define SHOWPNEMSG*/
#endif /* DEBUG */

#define RSHDEFS
#define NOVHGDIS
#define NOKEYSTATE
#define NOICON
#define NOBRUSH
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOMINMAX
#define NOPEN
#define NOREGION
#define NOSOUND
#define NOTEXTMETRIC
#define NOWNDCLASS
#define NOCOMM
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "version.h"
#include "heap.h"
#include "layout.h"
#include "file.h"
#define NOIFK
#define USEBCM
#include "cmd.h"
#include "keys.h"
#include "doc.h"
#include "disp.h"
#include "props.h"
#include "sel.h"
#include "screen.h"
#include "print.h"
#include "ch.h"
#include "format.h"
#include "status.h"
#include "wininfo.h"
#include "menu2.h"
#include "dde.h"
#include "help.h"
#include "debug.h"
#include "field.h"
#include "cmdtbl.h"
#include "fkp.h"
#include "prm.h"
#include "prompt.h"
#include "message.h"
#include "idle.h"
#include "error.h"
#include "iconbar.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"
#include "scc.h"
#include "resource.h"
#include "rareflag.h"
#include "recorder.h"

#ifdef DEBUG
#include "rerr.h"
#endif

#define WPROC
#include "core.h"


/* globals */
#ifdef DEBUG
int	    	cHpFreeze = 0;
#endif /* DEBUG */

struct CHP      vchpFetch;
struct CHP      vchpStc;
struct PAP	vpapStc;
struct TAP      vtapStc;
struct PAP      vpapFetch;
struct SEP      vsepFetch;
BOOL            vfFliMunged; /* returned from cachepage */
int             vised;
int             vipgd;

struct CA       caPara;
struct CA       caPage;
struct CA       caSect;
struct CA       caTable;
extern struct FCB     **mpfnhfcb[];
extern int 		      vfnPreload;
extern int		      vcPreload;
struct CA       caTcxCache;

int             vdocFetch;
int             vdocFetchPic;
CP              vcpFetch;
char HUGE       *vhpchFetch;
int             vccpFetch;
/* signals cpMac reached */
BOOL            vfEndFetch;

CP vcpFirstTablePara;
CP vcpFirstTableCell;

/* used to avoid AdjustCp's while inserting from the keyboard */
char            rgchInsert[cchInsertMax];
int             ichInsert; /* Number of chars used in rgchInsert */

int             fnFetch;        /* last run was in this fn */
/* fc corresponding to vcpFetch. If fnLast == fnInsert, fc is an index
into rgchInsert, still corresponding to vcpFetch */
FC              fcFetch;
int             fcmFetch;
int             prmFetch;       /* prm of ipcdFetch */

int		vdocPapLast = docNil;
int		vfnPapLast = fnNil;
int		vpnPapLast = 0;
int		vbpapxPapLast = cbSector + 1;

/* number of cp's remaining in last chp run, piece, page in file */
/* following three quantities must be in order (hand-nat requirement) */
CP              ccpChp, ccpPcd;
int             ccpFile;

int             ipcdFetch;       /* index of the piece corresponding to vcpFetch */

int             vstcpMapStc;        /* last stcp calculated by MapStc  */

char            rgchCaps[ichCapsMax];

int             vfAwfulNoise;
int             vssc; /* secondary selection (move, copy, copy looks) mode */
int             vfInitializing = TRUE;
int             vwwSelect;
/* means idle should normalize to the selection and then reset the flag */
BOOL            vfSeeSel;

/* flags are set fTrue when prefix key is encountered. Thus, the next
keystroke is interpreted as part of a 2 key sequence. */
int             vgrpfKeyPrefix;
int             vxwCursor;      /* preferred column for up-down cursor movement */
int             flashID = 0; /* timer ID for flashing before we put up a messagebox when we are not the active app */
MSG             vmsgLast;           /* last message received */
HANDLE          vhSysMenu;          /* handle to system (toaster) menu */
HWND            vhwndApp;           /* handle to parent ww (created in
		interface module) */
HWND            vhwndStartup;       /* handle to start up monologue */
HANDLE          vhInstance;         /* handle to memory module instance */

HMENU           vhMenu;             /* handle to current top level menu */
HMENU           vhMenuLongFull;     /* full menu*/
int             viMenu = -2;        /* current menu number */

CHAR            *vpDlgBuf;          /* pointer to buffer for dialog boxes */

BOOL            vfSingleApp;        /* if true, running with runtime windows */

HBRUSH          vhbrWhite;          /* various brushes used for pattern fills */
HBRUSH          vhbrBlack;
HBRUSH          vhbrGray;
HBRUSH          vhbrLtGray;
HBRUSH          vhbrDkGray;

/* bitmaps used for drawing vertical and horizontal lines */
HBITMAP         vhbmPatVert;
HBITMAP         vhbmPatHorz;


int             vfFocus = FALSE;    /* Whether we have the input focus */
int  vfMouseExist = fFalse;         /* fTrue if mouse hardware is installed */
int  vfInLongOperation = fFalse;    /* fTrue if we are still in a long operation
	so that the cursor should stay hourglass */
int  vfDeactByOtherApp=fFalse;      /* We were deactivated by another app */

HWND            vhwndSizeBox;       /* handle to the size box */
HWND            vhwndMsgBoxParent;  /* current parent to use for message boxes */
HWND            vhwndDeskTop;       /* handle to desktop window, area that contains doc windows */

#ifdef DEBUG
int **vhgdis = hNil;
#endif /* DEBUG */

int             vgrpfKeyBoardState = 0;  /* current keyboard state */

BOOL vfOvertypeMode = fFalse;
BOOL vfExtendSel = fFalse; /* true if extend to next typed character and survive in search and goto */
BOOL vfBlockSel = fFalse; /* true if in block selection state */

BOOL            vfDoubleClick = FALSE;  /* whether click is double click */
BOOL            vfRightClick = FALSE;   /* whether click is right button */
HCURSOR         vhcHourGlass;           /* handle to hour glass cursor */
HCURSOR         vhcIBeam;               /* handle to i-beam cursor */
HCURSOR         vhcArrow;               /* handle to arrow cursor */
HCURSOR         vhcBarCur;              /* handle to back arrow cursor */
HCURSOR         vhcSplit;               /* handle to split cursor */
HCURSOR         vhcStyWnd;              /* handle to style name window cursor */
HCURSOR         vhcRecorder;
HCURSOR         vhcHelp;                /* handle to ? cursor for help */
HCURSOR         vhcOtlCross;
HCURSOR		vhcOtlHorz;
HCURSOR		vhcOtlVert;
HCURSOR         vhcColumn;
HCURSOR         vhcPrvwCross = NULL;
HICON           vhiWord;                /* winword icon */

/* This is how we tell if NewCurWw is being called as a result of
	returning from a message box mamong other things (we disable
	coming back from a messagebox or dialog if we were in a message box
	too. They can nest, e.g., at ExitWindows time bz
*/
int vcInMessageBox = 0;

/* USAGE: caAdjust is a temporary CA which can be used by anyone if they
	want a caTemp that will be adjusted whenever AdjustCp is called.  The
	only constraint is that you must call AcquireCaAdjust() before using
	it and ReleaseCaAdjust() when you're done, to ensure that noone else
	is using it at the time.  Actually these are macros that will go away
	when DEBUG is undefined, but we should catch these conflicts now.
*/
	struct CA caAdjust = { 
	0, 0, docNil 		};


	struct CA caAdjustL = { 
	cp0, cp0, docNil 		};


#ifdef DEBUG
int cCaAdjust = 0;
int cCaAdjustL = 0;
#endif


#ifdef RSH

#ifdef NOASM
NATIVE IScanLprgw( int far *, WORD, int ); /* DECLARATION ONLY */
#endif /* NOASM */

/* this table is used to record sys commands (SC_) in the research version */
csconst int rgscRsh[] =
	{
		SC_SIZE,
		SC_MOVE,
		SC_MINIMIZE,
		SC_MAXIMIZE,
		SC_NEXTWINDOW,
		SC_PREVWINDOW,
		SC_CLOSE,
		SC_VSCROLL,
		SC_HSCROLL,
		SC_MOUSEMENU,
		SC_KEYMENU,
		SC_RESTORE
	};
#define iscMax sizeof(rgscRsh)/sizeof(int)

/* this table is used to record interesting messages (WM_) in the research version */
csconst int rgwmRsh[] =
	{
		WM_QUERYENDSESSION,
		WM_ENDSESSION,
		WM_ACTIVATEAPP,
		WM_NCLBUTTONDOWN,
		WM_NCRBUTTONDOWN,
		WM_NCLBUTTONDBLCLK,
		WM_NCRBUTTONDBLCLK,
		WM_LBUTTONDOWN,
		WM_RBUTTONDOWN,
		WM_LBUTTONDBLCLK,
		WM_RBUTTONDBLCLK,
		WM_CLOSE,
		WM_VSCROLL,
		WM_HSCROLL
	};
#define iwmMax sizeof(rgwmRsh)/sizeof(int)

#endif /* RSH */

extern int              vpisPrompt;
extern int              flashID;
extern ENV  	       *penvMem;
extern BOOL             vfDdeIdle;
extern BOOL             vfRecording;
extern BOOL             vfRecordNext;
extern CMR					**vhcmr;
extern FARPROC          vlppFWHChain;
extern int              vfInitializing;
extern struct MWD       **hmwdCur;
extern int              mwCur;
extern int              mwMac; /* mac of child window */
extern int              wwMac;
extern HMENU            vhMenuNoMwd;
extern int              vfLargeFrameEMS;
extern HMENU            vhMenuShort;
extern HMENU            vhMenuWithMwd;
extern struct MWD       **mpmwhmwd[];
extern struct SCI       vsci;
extern struct PRI       vpri;
extern struct FLI       vfli;
extern struct PREF      vpref;
extern CHAR             szApp[];
extern HANDLE           vhInstance;
extern HWND             vhwndApp;          /* handle to parent's window */
extern HWND             vhwndDeskTop;
extern HWND             vhwndPrompt;
extern HWND             vhwndPromptHidden;
extern HWND             vhwndRibbon;
extern HWND             vhwndCBT;
extern int              vcxtHelp;     /* help context id */
extern BOOL             vfHelp;       /* whether we're in Help Mode */
extern long             vcmsecHelp;
extern WORD             vwParamHelp;
extern LONG             vlParamHelp;
extern HCURSOR          vhcHourGlass;
extern HCURSOR          vhcIBeam;
extern HCURSOR          vhcArrow;
extern HCURSOR          vhcBarCur;
extern HCURSOR          vhcRecorder;
extern HCURSOR          vhcHelp;
extern HCURSOR          vhcStyWnd;
extern HMENU            vhMenu;
extern HWND             vhwndAppModalFocus;
extern HWND             vhwndStatLine;
extern BOOL             vfExtendSel;
extern struct STTB      **vhsttbOpen;
extern BOOL             vfBlockSel;
extern MSG              vmsgLast;
extern struct WWD       **mpwwhwwd[];
extern struct WWD       **hwwdCur;
extern int              mwCreate;
extern BOOL             vfInLongOperation;
extern int              wwCur;
extern struct SEL       selCur;
extern struct SEL       selDotted;
extern int              vfDeactByOtherApp;
extern HWND             vhwndMsgBoxParent;
extern struct PPR     **vhpprPRPrompt;
extern int              vlm;
extern int              vssc;
extern struct MERR      vmerr;
extern int              vfFocus;
extern BOOL             vfDoubleClick;
extern BOOL             vfRightClick;
extern int              vfSysMenu;
extern MUD **           vhmudUser;
extern MUD **           vhmudBase;
extern KMP **           vhkmpUser;
extern KMP **           hkmpCur;
extern KMP **           hkmpBase;
extern CP               vcpFetch;
extern int              vfSeeSel;
extern int              vcConseqScroll;
extern int              sbScroll;
extern int              sbScrollLast;
extern IDF		vidf;
extern int		cLongOpCount;
extern int              vfMouseExist;
extern BOOL		fElActive;
extern struct RF	vrf;
extern BOOL             vfInCommit;
extern int              cbMemChunk;
extern struct BPTB      vbptbExt;
extern int		vgrfMenuKeysAreDirty;
extern BOOL		vfEmptyMenu;


BOOL vfChildSysMenu = fFalse;
BOOL fRecordMove=fFalse;
BOOL fRecordSize=fFalse;
BOOL fCdlScrollSet=fFalse;

struct MERR     vmerr;


extern BOOL             vfRestPmtOnInput;
extern long             vusecTimeoutPrompt;
extern int              vcchStatLinePrompt;
extern int              docCur;
extern struct DRF	*vpdrfHead;
extern struct DRF	*vpdrfHeadUnused;
extern int		vfNoInval;

void MwdVertScroll( int, WORD, int );
void MwdHorzScroll( int, WORD, int );
void WwPaneMouse( int, unsigned, WORD, POINT );
LONG LMenuChar(HWND, WORD, LONG);


#ifdef DEBUG
ENV *penvBase;
BYTE fCoverWasActive;
#endif /* DEBUG */

#ifdef HYBRID
BOOL vfShowSwapping;
#endif /* HYBRID */

/************************************************************************/
/* WinMain -- Windows' equivalent of "main()" in standard C             */
/************************************************************************/

/* %%Function:WinMain  %%Owner:peterj */
EXPORT int PASCAL WinMain( hInstance, hPrevInstance, lpszCmdLine, cmdShow )
HANDLE hInstance, hPrevInstance;
LPSTR  lpszCmdLine;
int    cmdShow;
{
	int wMsg;
#ifdef DEBUG
	ENV env;
	extern BYTE fCoverActive;
#endif /* DEBUG */
#ifdef HYBRID
	extern BYTE fCoverActive;
#endif

	InitApploader();
#ifdef DEBUG
	FInitCover(0);
	fCoverWasActive = fCoverActive;
	fCoverActive = fFalse;
#endif
#ifdef HYBRID
	FInitCover(0);
	fCoverActive &= GetProfileInt(szApp, SzShared("HybridfCoverActive"), 0);
#endif

/* Initialize our huge pointer info
** IMPORTANT FInitSegTable before GlobalMemInit, else GlobalMemInit may 
	cause our code being moved and WINDOW does not know that our segment
	table has to be updated.
*/
	if (!FInitSegTable( sbMax ))
		{
		ReportSz("FinitSegTable failure");
		return fFalse; /* Could not initialize; exit the program */
		}

#ifdef DEBUG
	/* initialize SetJmp/DoJmp for out of memory errors */
	penvBase = &env;
	if (SetJmp (penvMem = penvBase))
		{
		Assert (fFalse);
		}
#endif /* DEBUG */

#ifdef HYBRID
	vfShowSwapping = GetProfileInt((LPSTR)szApp, 
			(LPSTR)SzFrameKey("HybridShowSwapping",HybridShowSwapping), 0);
	if (vfShowSwapping) InstallInt3FHandler();
#endif /* HYBRID */

/* initialize structures, create initial windows, etc. */

	if (!FInitWinInfo( hInstance, hPrevInstance, lpszCmdLine, cmdShow ))
		{
		CleanUpForExit();
		return fFalse; /* Could not initialize; exit the program */
		}


	for (;;)
		{
		BOOL fHaveMsg = fFalse;

#ifdef DEBUG
			{
			extern BOOL vfAllocGuaranteed, vfInCmd, vfInDebug;
			extern int vnErrorLevel;
			Assert(vfAllocGuaranteed == 0);
			vfAllocGuaranteed = 0;
			Assert(vfInCmd == 0);
			vfInCmd = 0;
			Assert(vfInDebug == 0);
			vfInDebug = 0;
			vnErrorLevel = 0;
			}
#endif /* DEBUG */

		if (cLongOpCount > 0)
			EndLongOp(fTrue);

		while (!fHaveMsg)
			/* wait for a message */
			{
		    TopOfMainLoop();

			/* iconbar dialog uses its own message loop */
			if (vidf.fIBDlgMode)
				{
			    Assert (!vidf.fDead);
                IBDlgLoop();
                }
				/* idle if there are not messages ready */
			else  if (PeekMessage((LPMSG)&vmsgLast, NULL, NULL, NULL, PM_REMOVE))
				/* we have a message */
				fHaveMsg = fTrue;

			else
				{
				BOOL fYield = fFalse;
				if (!vfDeactByOtherApp && !vidf.fNotLive)
/* Neither an icon nor a dying ember -- perform background
tasks like screen update, showing selection, etc. */
					fYield = !FIdle(); /* true if interrupted */
					/* perform background DDE processes, if any (esp when deact, icon or dead) */
				else  if (vfDdeIdle)
					DoDdeIdle ();
				else
					WaitMessage();

				if (vmerr.mat == matFont || vmerr.mat == matDisp)
					FlushPendingAlerts();
				vmerr.mat = matNil;/* don't report errors in bkgnd processes */
				vmerr.fHadMemAlert = fFalse;/* in case we already did */

				if (fYield)
					Yield();
				}
			}

		if ((wMsg = vmsgLast.message) == WM_QUIT)
			{
			QuitExit();
			Assert( fFalse );
			}

		/* If the prompt line wants to be cleared on input, vfRestPmtOnInput is
		set. */
		if (vfRestPmtOnInput && ((wMsg == WM_SYSCOMMAND)||
				(wMsg == WM_KEYDOWN) ||
				(wMsg == WM_LBUTTONDOWN) || (wMsg == WM_RBUTTONDOWN)
				|| (wMsg == WM_COMMAND)))
			{
			BOOL fMsgForPrompt = vmsgLast.hwnd == vhwndPrompt;
			RestorePrompt();

			/* if the message was destined for the prompt and the prompt was
				destroyed by RestorePrompt, don't try to send it on! */
			fHaveMsg = !fMsgForPrompt || vhwndPrompt != NULL;
			}

		if (fHaveMsg)
			{
			/* keyboard processing */
			if (!FIsKeyMessage((LPMSG) &vmsgLast))
				{
				/* messages not fully processed */
				DispatchMessage((LPMSG)&vmsgLast);
				}

			if (vfChildSysMenu)
				{
				Assert(!vfSysMenu);
				if (hwwdCur != hNil)
					{
					SendMessage((*hwwdCur)->hwnd, WM_SYSCOMMAND,
							SC_KEYMENU, (DWORD) '-');
					}
				vfChildSysMenu = fFalse;
				}
			}

		/* This means we got a WM_CBTTERM from CBT and waited to clean up
				until they are no longer on our stack (since we free their
				library as part of our cleanup) */
		if (vrf.fMustStopCBT && !vfDeactByOtherApp)
			{
			StopCBT();
			vrf.fMustStopCBT = fFalse;
			}

		} /* for (;;) */
	Assert( fFalse );
}


/* performes standard actions between all commands */
/* %%Function:TopOfMainLoop  %%Owner:peterj */
TopOfMainLoop()
{
	int ww;

	Assert (vfNoInval == 0);
	vfNoInval = 0;
	Assert (vpdrfHead == NULL);
	vpdrfHead = NULL;
	Assert (vpdrfHeadUnused == NULL);
	Debug(vpdrfHeadUnused = NULL);

	if (vidf.fDead)
        return;

	if (vmerr.fKillRulRib)  /* take down before messages appear */
		EndIbdlg();

	if (vmerr.mat != matNil && !vidf.fNotLive)
		FlushPendingAlerts();

	/* if set, we should have had a mat set, so Flush... should have cleared this. bz */
	Assert (vmerr.fHadMemAlert == fFalse || vidf.fNotLive);

	vmerr.fHadMemAlert = fFalse;
	vmerr.fErrorAlert = fFalse;
	vmerr.fDiskAlert = fFalse;
	vmerr.fDiskWriteErr = fFalse;
	vmerr.fFmtFailed = fFalse;
	vmerr.fCompressFailed = fFalse;

	/* some document has overflown size we are capable of saving */
	if (vmerr.fWarnDocTooBig)
		{
		ErrorEid(eidDocTooBigToSave, "WinMain");
		vmerr.fWarnDocTooBig = fFalse;
		}

	/* if a nestable progress report was left active, deactivate it */
	if (vhpprPRPrompt)
		StopProgressReport(hNil, pdcRestore);

	/* make sure no AbortChecks were left active */
	Assert(vpisPrompt == pisNormal);
	vpisPrompt = pisNormal;

	
	/* check for no-dr case which will want to bag windows out of PageView */
	if (vmerr.fNoDrs)
		UpdateAllWws(fFalse);

	Assert(cHpFreeze==0);
	if (vmerr.hrgwEmerg3 == 0)
		vmerr.hrgwEmerg3 = HAllocateCw(cwEmerg3);
	if (vmerr.hrgwEmerg2 == 0)
		vmerr.hrgwEmerg2 = HAllocateCw(cwEmerg2);
	if (vmerr.hrgwEmerg1 == 0)
		vmerr.hrgwEmerg1 = HAllocateCw(cwEmerg1);
	vmerr.mat = matNil;  /* Do not report errors from above allocations */

	/* if we're in a memory emergency, pop all ww's out of page view */
	if (vmerr.hrgwEmerg1 ==0 || vmerr.hrgwEmerg2 == 0 || vmerr.hrgwEmerg3 == 0)
		{
		for (ww = wwDocMin; ww < wwMac; ww++)
			if (mpwwhwwd[ww] != hNil && PwwdWw(ww)->fPageView)
				{
				if (ww == wwCur)
					/* go through FExecCmd so that special modes get terminated.
					Undo, repeat, recording are also taken care of.  FLegalBcm
					will allow PageView if current window is already in page view */
					FExecCmd(bcmPageView);
				else
					CmdPageViewOff(ww);
				}
		}
	for (ww = wwDocMin; ww < wwMac; ww++)	
		{
		struct WWD *pwwd;
		if (mpwwhwwd[ww] !=hNil && (pwwd = PwwdWw(ww))->fOutline && 
			PdodDoc(PmwdWw(ww)->doc)->hplcpad == hNil)
			{
			/* For various reasons, we do not restore
				rulers in this case. */
			pwwd->fHadRuler = fFalse;
			FToggleOutline(ww);
			}
		}

	/* from printer change message */
	if (vrf.fCkPgMarg)
		{
		SetFlm( flmRepaginate );
		FCheckPageAndMargins(selCur.doc, fTrue, fFalse, NULL);
		vrf.fCkPgMarg = fFalse;
		}

	/* handle postponed system messages (from print, pictures)  */
	if (vpri.wmm)
		CheckEnvPending();

	/* close any files on floppies so we are not hosed if the user
	       removes/changes floppies */
	CloseEveryFn(fFalse);
}


/* FInitWinInfo - initialise the world, this routine simply calls two
*  other initialisation routines making sure their code modules get
*  swapped out after use. It is reason for being (rather than have the
*  code in line in WinMain, is to declare the command line buffers shared
*  between FInitPart1 and FInitPart2.
*/
/* %%Function:FInitWinInfo  %%Owner:peterj */
int FInitWinInfo( hInstance, hPrevInstance, lpszCmdLine, cmdShow  )
HANDLE hInstance, hPrevInstance;
LPSTR  lpszCmdLine;
int    cmdShow;
{
	int ipchArgMac;
	BOOL fOpenUntitled = fTrue;
	BOOL fTutorial = fFalse;
	CHAR rgchCmdLine [ichMaxCmdLine+1];
	CHAR *rgpchArg [ipchArgMax];
	extern int far pascal initwin_q();
	extern int far pascal init2_q();

	if (!FInitPart1( hInstance, hPrevInstance, lpszCmdLine,
			rgchCmdLine, rgpchArg, &ipchArgMac, &fOpenUntitled, &fTutorial))
		return fFalse;

	/*  get it out of memory */
	GlobalDiscard(GetCodeHandle((FARPROC)initwin_q));

	if (!FInitPart2(cmdShow, rgchCmdLine, rgpchArg, ipchArgMac, fOpenUntitled, fTutorial))
		return fFalse;

	/*  get it out of memory */
	GlobalDiscard(GetCodeHandle((FARPROC)init2_q));

	return fTrue;
}


/*  NOTE: Messages bound for this WndProc are filtered in wprocn.asm */

/* %%Function:AppWndProc  %%Owner:peterj */
long EXPORT FAR PASCAL AppWndProc(hwnd, message, wParam, lParam)
HWND      hwnd;
unsigned  message;
WORD      wParam;
LONG      lParam;
{
	static int fEnteredIdle=fTrue;
	static HMENU hMenu;
	static int bcm;
	static int mf;
	int cxt;

	long AppWndProcRare();
	long AppWndProcAct();

	struct RC   rc;

#ifdef SHOWAPPMSG
	ShowMsg ("ap", hwnd, message, wParam, lParam);
#endif /* SHOWAPPMSG */

#ifdef RSH
	/* BLOCK - log message */
		{
		int iwm = IScanLprgw((int far *)rgwmRsh,message,iwmMax);
		if (iwm >= 0 && iwm < iwmMax)
			LogUa(uaAppMessage+iwm);
		}
#endif /* RSH */

	switch (message)
		{
	default:
		return AppWndProcRare( hwnd, message, wParam,lParam );

	case WM_ACTIVATE:
	case WM_ACTIVATEAPP:
		return AppWndProcAct( hwnd, message, wParam,lParam );

	case WM_MOUSEACTIVATE:
		/* this keeps us from beeping unnecessarly when the application
			is reactivated by a mouse click if a window is app modal */
		if (vhwndAppModalFocus != hNil && vfDeactByOtherApp)
			return (LONG) MA_ACTIVATEANDEAT;
		else
			goto LDefProc;

	case WM_COMMAND:
		/* Ignore command messages posted while in a message box (this
			can happen when our app is activated by clicking on an icon
			bar button while a message box is waiting to come up). BAC */
		if (vcInMessageBox)
			return 0L;

		/* CBT hook must be the first thing we do under WM_COMMAND */

		if (vhwndCBT && !SendMessage(vhwndCBT, WM_CBTSEMEV, smvCommand,
				MAKELONG(CxtFromBcm(wParam), 0 /* from menu */)))
			{
			return (0L);
			}

		/* CBT returned TRUE; do normal handling */

		EnsureFocusInPane();

		SetOurKeyState(); /* we don't get a key-up for Alt! */
		/* dyadic stuff handled at command level now */

		RestorePrompt();

		if (wParam == bcmNil) /* menu empty */
			return (0L);

		Assert((int) wParam >= 0);
		vcxtHelp = cxtNil;
		FExecCmd(wParam);
		break;

	case WM_SYSCOMMAND:

#ifdef RSH
	/* BLOCK - log command */
		{
		int isc = IScanLprgw((int far *)rgscRsh,wParam,iscMax);
		if (isc >= 0 && isc < iscMax)
			LogUa(uaAppCommand+isc);
		}
#endif /* RSH */

		if (vfHelp || vhwndCBT)
			cxt = CxtAppSysMenu(wParam);

		/* Must notify CBT before processing the message */
		/* undocumented Windows fact: wParam & 0x000F is non-zero if
			WM_SYSCOMMAND was generated from clicking in title bar or
			sizing borders, and zero otherwise */
		if (vhwndCBT && (!(wParam & 0x000F) || wParam == bcmControlRun))
			{
			switch (wParam)
				{
			case SC_MOVE:
			case SC_SIZE:
			case SC_MAXIMIZE:
			case SC_MINIMIZE:
			case SC_RESTORE:
			case SC_CLOSE:
			case bcmControlRun:
#ifdef DEBUG
				if (wParam == bcmControlRun)
					Assert(cxt == CxtFromBcm(bcmControlRun));
#endif
				if (!SendMessage(vhwndCBT, WM_CBTSEMEV, smvCommand,
						MAKELONG(cxt, 0 /* from menu */)))
					{
					return (0L);
					}
				break;

#ifdef INEFFICIENT
			case SC_NEXTWINDOW:
			case SC_PREVWINDOW:
			case SC_VSCROLL:
			case SC_HSCROLL:
			case SC_MOUSEMENU:
			case SC_KEYMENU:
			case SC_ARRANGE:
				break;
#endif
				}
			}

		if (vfHelp)
			{
			if (cxt == cxtNil)
				goto LDefProc;  /* pressed Alt, or clicked to drop down a menu */

			GetHelp(cxt);
			return fTrue;
			}
		else
			{
			Profile(vpfi == pfiMenu ? StartProf(30) : 0);
			EnsureFocusInPane();
			vcxtHelp = cxtNil;
			if ((int) wParam >= 0)
				{
				CmdExecBcmKc(wParam, kcNil);
				}
			else
				{
				if (vfRecording)
					RecordAppSysCmd(wParam);
				switch (wParam & 0xfff0)
					{
				case SC_MOVE:
					fRecordMove = fTrue;
					break;
				case SC_SIZE:
					fRecordMove = fTrue;
					fRecordSize = fTrue;
					break;
					}
				goto LDefProc;
				}
			}
		break;

	case WM_INITMENUPOPUP:
		/* setup the pull down menu before being drawn */
		/* wParam is the popup menu handle */
		/* LOWORD(lParam) = index of popup in main menu */
		/* HIWORD(lParam) = 1 if system menu, 0 if application main menu */
		EndLongOp(fTrue /* fAll */);
		if (HIWORD(lParam) == 0)
			{ /* we care for the application main menu only */
			/* for the zorn menu in the menu bar HIWORD(lParam) is still 0 */
			SetAppMenu((HMENU) wParam, lParam);
			}

		/* Turn on the cursor so we can see where it is */
		if (!selCur.fOn && selCur.fIns)
			{
			/* Bug Fix BL 9/13/88: be sure window is up to date first */
			UpdateWw( selCur.ww, fFalse /*fAbortOK*/ );
			ToggleSel( &selCur, selCur.cpFirst, selCur.cpFirst );
			selCur.fHidden = fFalse;
			}
		break;

	case WM_MENUCHAR:
			{
			LONG lRet = LMenuChar(hwnd, wParam, lParam);
			if (lRet != 0)
				return lRet;
			}

LDefProc:
		return(DefWindowProc(hwnd, message, wParam, lParam));

	case WM_SETCURSOR:
		if (vfHelp | vfInLongOperation)
			{
			OurSetCursor(NULL); /* will set according to the vf's */
			return fTrue;
			}
		goto LDefProc;

	case WM_MENUSELECT:
		SaveMenuHelpContext(wParam, lParam, cxtAppMenuSelect);

		if (vhwndStatLine)
			{
			hMenu = HIWORD(lParam);
			bcm = wParam;
			mf = LOWORD(lParam );
			if (hMenu == 0 && fEnteredIdle)
				DrawMenuHelp(hMenu, bcm, mf);
			fEnteredIdle = fFalse;

			if (!vwParamHelp && !vlParamHelp && !vcxtHelp) /* exit menu */ 
				RestorePrompt();
			}

		break;

	case WM_SYSKEYDOWN:
		/* received when no window has the input focus */
		if (wParam == VK_F1 && !vfControlKey && !vfOptionKey)
			{
			if (vfShiftKey)
				CmdHelpContext(NULL);
			else
				CmdHelp(NULL);
			break;
			}
		goto LDefProc;

	case WM_ENTERIDLE:
		/* received from Windows code when idle in menu or dialog.
		   Now is the time to really paint status line menu help
		   NOTE: we don't use windows dialogs, bug libraries we call may
		   (esp. conv-wrd.dll). 
		   wParam==MSGF_MENU indicates idle in menu, we should put up menuhelp
		*/
		Profile( vpfi == pfiMenu ? StopProf() : 0 );

		Assert(wParam == MSGF_MENU || wParam == MSGF_DIALOGBOX); /* documentation said so */
		if (wParam == MSGF_MENU && !fEnteredIdle && vhwndStatLine)
			{
			fEnteredIdle = fTrue;
			DrawMenuHelp(hMenu, bcm, mf);
			}
#ifdef DCORELOAD
		CommSz(SzShared("."));
#endif /* DCORELOAD */
		FAbortNewestCmg(cmgDlgNew, fTrue, fTrue);
		break;

		}

	/* A window proc should always return something */
	return(0L);
}




/*  NOTE: Messages bound for this WndProc are filtered in wprocn.asm */

/* %%Function:MwdWndProc  %%Owner:peterj */
long EXPORT FAR PASCAL MwdWndProc(hwnd, message, wParam, lParam)
HWND      hwnd;
unsigned  message;
WORD      wParam;
LONG      lParam;
{
	extern int mwCreate;
	static int fEnteredIdle=fTrue;
	static HMENU hMenu;
	static int bcm;
	static int mf;
	long MwdWndProcRare();
	int ww;
	PAINTSTRUCT ps;

#ifdef SHOWDOCMSG
	ShowMsg ("dc", hwnd, message, wParam, lParam);
#endif /* SHOWDOCMSG */

#ifdef RSH
	/* BLOCK - log message */
		{
		int iwm = IScanLprgw((int far *)rgwmRsh,message,iwmMax);
		if (iwm >= 0 && iwm < iwmMax)
			LogUa(uaMwdMessage+iwm);
		}
#endif /* RSH */

	switch (message)
		{
	default:
		return MwdWndProcRare( hwnd, message, wParam,lParam );

	case WM_VSCROLL:
	case WM_HSCROLL:
		/* Vertical/Horizontal scroll bar input.  wParam contains the
		** scroll code.  For the thumb movement codes, the low
		** word of lParam contain the new scroll position.
		** Possible values for wParam are: SB_LINEUP, SB_LINEDOWN,
		** SB_PAGEUP, SB_PAGEDOWN, SB_THUMBPOSITION, SB_THUMBTRACK
		** Hiword of lParam contains the window handle of the
		** scroll bar. */
		EnsureFocusInPane();
		Assert(hwwdCur);
#ifdef RSH
		SetUatMode(uamNavigation);
		StopUatTimer();
#endif /* RSH */
		if (message == WM_HSCROLL)
			MwdHorzScroll( wwCur, wParam, (int)lParam);
		else
			MwdVertScroll(((*hwwdCur)->hwndVScroll == HIWORD(lParam)) ?
					wwCur : WwOther(wwCur), wParam, (int)lParam);
		if (vfRecording)
			RecordScroll(message, wParam, LOWORD(lParam));
		break;

	case WM_MOUSEACTIVATE:
			{
			int iRet = 0;

			if (vhwndAppModalFocus != hNil)
				if (!vfDeactByOtherApp)
					return (LONG) MA_NOACTIVATE;
				else
					goto DefaultProc;

			if (hmwdCur != hNil && (*hmwdCur)->hwnd != hwnd)
				{
				switch (LOWORD(lParam))
					{
					/* We eat HTCLIENT, HTSYSMENU, and HTMENU */
					/* NOTE: HTSIZE, HTHSCROLL, and HTVSCROLL have no affect
					because our scroll bars and size box are in our client
					area and are therefore not considered by Windows to be
					"real" scroll bars and size boxes. (BAC) */

				case HTREDUCE:
				case HTZOOM:
				case HTCAPTION:
				case HTSIZE:
				case HTHSCROLL:
				case HTVSCROLL:
				case HTLEFT:
				case HTRIGHT:
				case HTTOP:
				case HTTOPLEFT:
				case HTTOPRIGHT:
				case HTBOTTOM:
				case HTBOTTOMLEFT:
				case HTBOTTOMRIGHT:
					iRet = MA_NOACTIVATE;
					break;

				case HTCLIENT:
					NewCurWw( PmwdMw(MwFromHwndMw(hwnd))->wwActive, fFalse /*fDoNotSelect*/);
					/* fall through */
				default:
					iRet = MA_ACTIVATEANDEAT;
					break;
					}
				}
			return(iRet);
			}

	case WM_CHILDACTIVATE:
		NewCurWw( PmwdMw(MwFromHwndMw(hwnd))->wwActive, fFalse /*fDoNotSelect*/);
		break;

	case WM_MENUCHAR:
			{
			LONG lRet = LMenuChar(hwnd, wParam, lParam);
			if (lRet == 0)
				goto DefaultProc;
			else
				return lRet;
			}

	case WM_SYSCOMMAND:
		if (vfSysMenu)
			/* don't really want to be here */
			{
			SendMessage(vhwndApp, message, wParam, lParam);
			return fTrue;
			}

#ifdef RSH
	/* BLOCK - log command */
		{
		int isc = IScanLprgw((int far *)rgscRsh,wParam,iscMax);
		if (isc >= 0 && isc < iscMax)
			LogUa(uaMwdCommand+isc);
		}
#endif /* RSH */

		if (vhwndCBT)
			{
			switch (wParam)
				{
			case bcmRestoreWnd:
				/* should get this here if doc window maximized;
					would come through AppWndProc */
				Assert (fFalse);
				/* fall thru just in case */

			case bcmMoveWnd:
			case bcmSizeWnd:
			case bcmZoomWnd:
			case bcmSplit:
			case bcmCloseWnd:
				if (!SendMessage(vhwndCBT, WM_CBTSEMEV, smvCommand,
						MAKELONG(CxtFromBcm(wParam), 0 /* from menu */)))
					{
					return (0L);
					}
				break;
				}
			}

		if (vfHelp)
			{
			int cxt;

			if ((cxt = CxtDocSysMenu(wParam)) == cxtNil)
				goto DefaultProc;  /* Probably clicked to drop down sys menu */

			GetHelp(cxt);
			return fTrue;
			}

		EnsureFocusInPane();

		if ((int) wParam >= 0)
			return CmdExecBcmKc(wParam, kcNil) == cmdOK;

		switch (wParam & 0xfff0)
			{
		case SC_MAXIMIZE:
			fRecordMove = fFalse;
			fRecordSize = fFalse;
			CmdExecBcmKc(bcmZoomWnd, kcNil);
			return fTrue;

		case SC_MINIMIZE: /* not allowed! */
			return fFalse;

		case SC_SIZE:
			if (vpref.fZoomMwd) /* not allowed if maximized! */
				return fFalse;
			fRecordMove = fTrue;
			fRecordSize = fTrue;
			break;
		case SC_MOVE:
			if (vpref.fZoomMwd) /* not allowed if maximized! */
				return fFalse;
			fRecordMove = fTrue;
			}
		if (vfRecording)
			RecordDocSysCmd(wParam);
		goto DefaultProc;

	case WM_INITMENUPOPUP:
		lParam |= 0x10000; /* Windows lies the first time! */
		vgrfMenuKeysAreDirty = 0xffff;/* force update of accelerator keys */
		SetAppMenu((HMENU) wParam, lParam);
		break;

	case WM_MENUSELECT:
		SaveMenuHelpContext(wParam, lParam, cxtDocMenuSelect);

		if (vhwndStatLine)
			{
			hMenu = HIWORD(lParam);
			bcm = wParam;
			mf = LOWORD(lParam );
			if (hMenu == 0 && fEnteredIdle)
				DrawMenuHelp(hMenu, bcm, mf);
			fEnteredIdle = fFalse;

			if (!vwParamHelp && !vlParamHelp && !vcxtHelp) /* exit menu */ 
				RestorePrompt();
			}
		break;

	case WM_ENTERIDLE:
		/* received from Windows code when idle in menu or dialog.
		   Now is the time to really paint status line menu help
		   NOTE: we don't use windows dialogs, bug libraries we call may
		   (esp. conv-wrd.dll). 
		   wParam==MSGF_MENU indicates idle in menu, we should put up menuhelp
		*/

		Assert(wParam == MSGF_MENU || wParam == MSGF_DIALOGBOX); /* documentation said so */
		if (wParam == MSGF_MENU && !fEnteredIdle && vhwndStatLine)
			{
			fEnteredIdle = fTrue;
			DrawMenuHelp(hMenu, bcm, mf);
			}
#ifdef DCORELOAD
		CommSz(SzShared("."));
#endif /* DCORELOAD */
		FAbortNewestCmg(cmgDlgNew, fTrue, fTrue);
		break;

		}

	/* A window proc should always return something */
	return(0L);

DefaultProc:    /* All messages not processed come here. */
	return (DefWindowProc(hwnd, message, wParam, lParam));
}


/* S A V E   M E N U   H E L P   C O N T E X T -      */
/* Since we do not want to compute the help context every time the
	user pulls down a menu, we will just save the wParam and lParam.
	If the user hits F1, and vcxtHelp has not yet been set to something
	else, we can compute the context from these saved values.
*/
/* %%Function:SaveMenuHelpContext  %%Owner:rosiep */
SaveMenuHelpContext(wParam,lParam,cxt)
WORD      wParam;
LONG      lParam;
int       cxt;
{
	Assert(cxt == cxtAppMenuSelect || cxt == cxtDocMenuSelect);

	/* if we're exiting Menu Mode, no more help context */
	if (HIWORD(lParam) == 0 || wParam == 0)
		{
		vwParamHelp = 0;
		vlParamHelp = 0;
		vcxtHelp = cxtNil;
		}
	else
		{
		vwParamHelp = wParam;
		vlParamHelp = lParam;
		vcxtHelp = cxt;
		}
}



#ifdef REFERENCE /* most of these are now in UpdateWindowWw */
/* U P D A T E  I N V A L I D  W W */
/* Find out what Windows considers to be the invalid range of
	the passed window.  Mark it invalid in WWD & blank the area on the screen */

/* %%Function:UpdateInvalidWw  %%Owner:NOTUSED */
UpdateInvalidWw(ww)
int ww;
{
	struct RC rc, rcwAboveYwMin;
	struct WWD **hwwd;
	int ypTop;

	/* First, get the update rect for the parent.  This will do necessary background
	erasure and border validation, the returned rectangle is ignored.
	Test for wwCur assures we only do this once per UpdateWindows */

	if (ww == wwCur)
		GetUpdateRect( vhwndApp, (LPRECT) &rc, fTrue /* fErase */ );

	if ((hwwd = mpwwhwwd [ww]) == hNil || (*hwwd)->wk == wkClipboard)
		return;

	/* Now, do the same for the document window, except that this time we use the
	rect to update the invalid band stored in the WWD structure */

	Assert( (*hwwd)->hwnd != NULL );
	GetUpdateRect( (*hwwd)->hwnd, (LPRECT) &rc, fTrue /* fErase */);

	if (FSectRc( PrcSet( &rcwAboveYwMin, 0, 0,
			(*hwwd)->xwMac, (*hwwd)->ywMin ),
			&rc, &rcwAboveYwMin ))
		PatBltRc( (*hwwd)->hdc, &rcwAboveYwMin, vsci.ropErase );

	if (vidf.fDead)
		{
		/* If we are in the process of shutting down, we DONT want to repaint,
			but we DO want to erase the bkgrnd and validate the border */
		PatBltRc( (*hwwd)->hdc, &rc, vsci.ropErase );
		return;
		}

	/* draw portion of style name window border that extends above ypMin */
	/* This is the only drawing/invalidation that we have to do in this area */
	if ((*hwwd)->xwSelBar)
		PaintStyStub( ww, &rc );
	rc.ypTop = max( (*hwwd)->ywMin, rc.ypTop );

	if (rc.ypTop < rc.ypBottom)
		InvalWwRc(ww, &rc );

	/* Since we have found out the invalid rect, and marked it invalid
	in our structures, we don't want to hear about it again,
	so we tell windows that we have made everything valid */

	ValidateRect( (*hwwd)->hwnd, (LPRECT) NULL );
}


#endif /* REFERENCE */



/*  NOTE: Messages bound for this WndProc are filtered in wprocn.asm */

/* %%Function:WwPaneWndProc  %%Owner:peterj */
long EXPORT FAR PASCAL WwPaneWndProc(hwnd, message, wParam, lParam)
HWND      hwnd;
unsigned  message;
WORD      wParam;
LONG      lParam;
{
	int ww;
	struct RC rc;
	long RareWwPaneWndProc();

#ifdef SHOWPNEMSG
	ShowMsg ("pn", hwnd, message, wParam, lParam);
#endif /* SHOWPNEMSG */

#ifdef RSH
	/* BLOCK - log message */
		{
		int iwm = IScanLprgw((int far *)rgwmRsh,message,iwmMax);
		if (iwm >= 0 && iwm < iwmMax)
			LogUa(uaWwdMessage+iwm);
		}
#endif /* RSH */

	switch (message)
		{
	default:
		return RareWwPaneWndProc( hwnd, message, wParam,lParam );

	case WM_SETFOCUS:
		/* The window is getting the focus.  wParam contains the window
		** handle of the window that previously had the focus. */
		WwPaneGetFocus(hwnd, (HWND)wParam);
		break;

	case WM_KILLFOCUS:
		/* The window is losing the focus.  wParam contains the window
		** handle of the window about to get the focus, or NULL. */
		if (vidf.fDead)
			break;
		Debug(vdbs.fShakeHeap ? ShakeHeap() : 0);
		WwPaneLoseFocus(hwnd, (HWND)wParam);
		break;

	case WM_MOUSEACTIVATE:
			{ /* reset the wwActivePane of its parent */
			int fSscClick = fFalse;

			if ((vhwndAppModalFocus != hNil) && !vfHelp)
				if (!vfDeactByOtherApp)
					return (LONG) MA_NOACTIVATE;
				else
					goto DefaultProc;

			if (vfHelp || (GetMessageTime() < vcmsecHelp))
				return(fTrue);

			if (GetFocus() != hwnd)
				{
				int ww = WwFromHwnd(hwnd);

				/* we want to gain focus */
				if (HIWORD(lParam) == WM_RBUTTONDOWN)
					vfRightClick = fTrue;

				if (fSscClick = FSscClick())
					{
					int eid = eidInvalidSelection;

					/* Note: Ssc click will do a quickmove/copy
						unless selCur.fIns or in dyadic mode or a few
						other cases. In those cases cancel the operation.
						Better to do it now before we change the activation.
						*/
					if ( FOutlineEmpty(ww, fFalse) ||
							(eid = EidMouseAction()) != eidNil )
						{
						ErrorEid(eid, "WwPaneWndProc");
						return (LONG) MA_NOACTIVATE;
						}
					else
						{
						/* a value which is not sscNil. This will keep
							NewCurWw from changing selCur.
							*/
						vssc = sscMouse;
						}
					}

/* Kludge Alert!  Windows has a re-entrancy problem with MA_ACTIVATEANDEAT
   such that we cannot call PeekMessage while nested in a WM_MOUSEACTIVATE
   message to which we plan to return MA_ACTIVATEANDEAT.  If we do, the
   message will stay on the queue.  NewCurWw can cause FMsgPresent to be
   called indirectly.  rp - 1/4/89

   Dyadic copy formatting, which does some extra NewCurWw calls had a similar
   problem with the MOUSEACTIVATE message not getting cleared. Setting the
   flag so FMsgPresent will return solved the problem, though I am not sure
   why, since the details are a little different that Rosie describes in
   Opus bug 5810. bz 4-27-89

   Moved setting of flag, now counter, to NewCurWw itself bz 5/10/89  */

				NewCurWw(ww, fFalse /*fDoNotSelect*/);
				if (fSscClick)
					{

					/* leave vssc as sscMouse so PselActive will use selDotted */
					return(MA_ACTIVATE); /* cause click msg to do dyadic move */
					}
				else
					{
					return(MA_ACTIVATEANDEAT);
					}
				}
DefaultProc:    /* All messages not processed come here. */
			return (DefWindowProc(hwnd, message, wParam, lParam));
			}

	case WM_MENUCHAR:
			{
			LONG lRet = LMenuChar(hwnd, wParam, lParam);
			if (lRet != 0)
				return lRet;
			goto DefaultProc;
			}

	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU)
			{
			if (vfSysMenu || LOWORD(lParam != '-'))
				return SendMessage(vhwndApp, message, wParam, lParam);
			else  if (vlm == lmPreview)
				/* prevent Menu From Nowhere from popping up over the
							Preview window */
				return (0L);
			}

#ifdef RSH
	/* BLOCK - log command */
		{
		int isc = IScanLprgw((int far *)rgscRsh,wParam,iscMax);
		if (isc >= 0 && isc < iscMax)
			LogUa(uaMwdCommand+isc);
		}
#endif /* RSH */

		goto DefaultProc;

	case WM_PAINT:
		/* Time for the window to draw itself. */
		UpdateWindowWw(WwFromHwnd(hwnd));
		break;

		/* For each of following mouse window messages, wParam contains
		** bits indicating whether or not various virtual keys are down,
		** and lParam is a POINT containing the mouse coordinates.   The
		** keydown bits of wParam are:  MK_LBUTTON (set if Left Button is
		** down); MK_RBUTTON (set if Right Button is down); MK_SHIFT (set
		** if Shift Key is down); MK_ALTERNATE (set if Alt Key is down);
		** and MK_CONTROL (set if Control Key is down). */

	case WM_SETCURSOR:
		return 0L;
	case WM_MOUSEMOVE:
		ForceCursorChange(hwnd, &lParam, vidf.fInIdle/*fExact*/);
		break;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN:

#ifdef RSH
		{										
		int w = 0;								
		if (GetKeyState(VK_SHIFT) < 0) w |= 1;	
		if (GetKeyState(VK_MENU) < 0) w |= 2;	
		if (GetKeyState(VK_CONTROL) < 0) w |= 4;
		LogUa(uaKeyState + w);					
		}
#endif /* RSH */

		ww = WwFromHwnd( hwnd );
		Assert( ww != wwNil );
		WwPaneMouse(ww, message, wParam, MAKEPOINT(lParam));

		}
	/* A window proc should always return something */
	return(0L);
}


/* %%Function:WwPaneMouse  %%Owner:peterj */
void WwPaneMouse(ww, message, wParam, pt)
int        ww;
unsigned   message;
WORD       wParam;
POINT      pt;
{
	BOOL fShift;
	BOOL fInStyleName = fFalse;

	if ((ww != wwCur) && !vfHelp)
		return;

	if (vfRecording)
		{
		Beep();
		return;
		}
	
	Assert( message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN ||
			message == WM_LBUTTONDBLCLK );
	vfDoubleClick = (message == WM_LBUTTONDBLCLK);
	vfRightClick = (message == WM_RBUTTONDOWN);

	Assert(hwwdCur != hNil);

	if (vfDoubleClick && (GetMessageTime() < vcmsecHelp))
		return;

	/* styname area? */
	if ((*hwwdCur)->xwSelBar > 0)
		{
		/* styname border? */
		if (pt.x <= ((*hwwdCur)->xwSelBar + (vsci.dxpBorder << 1)))
			if (vfHelp)
				{
				GetHelp(cxtStyleArea);
				return;
				}
			else
				{
				fInStyleName = fTrue;
				if (FXpNearStyWndBorder(pt.x, hwwdCur))
					{
					if (!vhwndCBT || SendMessage(vhwndCBT, WM_CBTSEMEV,
						smvTrackStyWnd, 0L))
						{
						TrackStyWndCursor(pt);
						}
					return;
					}
				}
		}
	fShift = vgrpfKeyBoardState & wKbsShiftMask;

	if (vfHelp)
		{
		GetHelp(CxtFromWwPt(ww, pt));
		return;
		}

	/* the fCommand parameter for Mac is used as fControl in Win */
	DoContentHit( pt, fShift, fFalse /* fOption */, vfControlKey /* fCommand */ );

	if (!vfBlockSel && vfDoubleClick && fInStyleName)
		{
		if (!vhwndCBT || SendMessage(vhwndCBT, WM_CBTSEMEV, smvCommand,
			MAKELONG(CxtFromBcm(bcmApplyStyleDlg), 2 /* dbl click */)))
			{
			CmdExecBcmKc(bcmApplyStyleDlg, kcNil);
			}
		}
	return;
}


/* %%Function:ClearAbortKeyState  %%Owner:peterj */
ClearAbortKeyState()
{
	GetAsyncKeyState(VK_ESCAPE);
	GetAsyncKeyState(VK_CANCEL);
	GetAsyncKeyState(VK_MENU);
}


/*  SetOurKeyState
	This function is called when ever there is likely to have been
	a change in the state of the keyboard.  This function updates
	vgrpfKeyBoardState which contains the state of:

		Keys:
			Shift Key             wKbsShiftMask
			Control Key           wKbsControlMask
			Alt/Option/menu Key   wKbsOptionMask
			Capslock key          wKbsCapsLckMask

Note that Extend Mode and Block Selection are not purely toggle key
because it could be turned off by switching windows or executing
certain commands.
*/



/* %%Function:SetOurKeyState  %%Owner:peterj */
void SetOurKeyState()
{
#ifdef INTL
	extern int vwWinVersion, vkRightBracket;
#endif /* INTL */
	extern int vgrpfKeyBoardState;
	int grpfT = vgrpfKeyBoardState;

	vgrpfKeyBoardState  = (GetKeyState(VK_SHIFT) < 0) ? wKbsShiftMask : 0;
	vgrpfKeyBoardState |= (GetKeyState(VK_MENU) < 0) ? wKbsOptionMask : 0;
	vgrpfKeyBoardState |= (GetKeyState(VK_CONTROL)<0) ? wKbsControlMask : 0;

#ifdef CAPSLOCKSFIX /* for intl only */
/* localized pre-Win3.0 keyboards don't toggle the caps-lock key!    */
/* We detect localalized keyboards by looking at the vk for the ']'. */
/* Only for German and French (defined CAPSLOCKSFIX) */
	if (vwWinVersion < 0x300 && vkRightBracket >= 0x600)
		vgrpfKeyBoardState |= (GetKeyState(VK_CAPITAL) < 0) ? wKbsCapsLckMask : 0;
	else
#endif /* CAPSLOCKSFIX */

	/* VK_CAPITAL is a toggle key, hence we check if it's toggled, not
		if it's down */
	vgrpfKeyBoardState |= (GetKeyState(VK_CAPITAL) & 1) ? wKbsCapsLckMask : 0;
	vgrpfKeyBoardState |= (GetKeyState(VK_NUMLOCK) & 1) ? wKbsNumLckMask : 0;

	if (vhwndStatLine != NULL &&
			(grpfT & (wKbsCapsLckMask + wKbsNumLckMask)) !=
			(vgrpfKeyBoardState & (wKbsCapsLckMask + wKbsNumLckMask)))
		{
		UpdateStatusLine(usoToggles);
		}
}




/* %%Function:FXpNearStyWndBorder  %%Owner:peterj */
FXpNearStyWndBorder(xp, hwwd)
int xp;
struct WWD **hwwd;
{
	int xpStyWndBorder = (*hwwd)->xwSelBar;

	return( xp <= xpStyWndBorder &&
			xp >= xpStyWndBorder - (vsci.dxpBorder << 1));
}



/* F W H  K E Y */
/* Note:  wParam is no longer used.
			wCode, if >= 0 will be one of the MSGF_* codes, otherwise
				we MUST pass it on to DefHookProc.
			HC_'s no longer exist (except internally)
	The Windows people are very confused about this too, and it took
	me a while to figure it out with Bob Gunderson.
*/

/* %%Function:FWHKey  %%Owner:peterj */
EXPORT long FAR PASCAL FWHKey(wCode, wParam, lpmsg)
int wCode, wParam;
LPMSG lpmsg;
{
	int wm;

	if (lpmsg != 0L && (wm = lpmsg->message) == WM_QUIT)
		{
		bltbx(lpmsg, (LPMSG)&vmsgLast, sizeof(MSG));
		QuitExit();
		Assert(fFalse);
		}

	switch (wCode)
		{
	case MSGF_DIALOGBOX:
		Assert (lpmsg != 0L);
		SetOurKeyState(); /* we don't get a key-up for Alt */
		/* We will get this for non-SDM (printer driver) dialog boxes */
		if (wm == WM_KEYDOWN && lpmsg->wParam == VK_F1
				&& !vfControlKey && !vfOptionKey)
			{
			GetHelp(vcxtHelp);
			}
		break;

	case MSGF_MESSAGEBOX:
		Assert (lpmsg != 0L);

		/* GetCapture will return NULL in a message box unless a
			key is held down, in which case we don't want to process
			a second key.  Excel does this trick, which is where I
			got it.  This is to fix a bug where we'd crash if you
			pressed a key to dismiss the message box and then
			pressed F1 while the first key was still down.  (rp)
		*/
		if (GetCapture())
			break;
#ifdef DEBUG
		if (!FCheckToggleKeyMessage(lpmsg) &&
				wm == WM_KEYDOWN &&
				FProcessDbgKey(KcModified(lpmsg->wParam)))
			/* special debugging key processing */
			return fTrue;
#endif /* DEBUG */

		/* FALL THROUGH */

	case MSGF_MENU:
		Assert (lpmsg != 0L);
		/** This code will call the help application if the user 
		*   selects something from a menu with the keyboard.  Mouse 
		*   selection is handled elsewhere  (in the wndproc).  **/
		SetOurKeyState();

		/** return immediately if it's not the right message **/
		if (wm != WM_KEYDOWN  &&  wm != WM_SYSKEYDOWN)
			break;

		if (vfEmptyMenu && lpmsg->wParam == VK_UP)
			return fTrue;


		/** Two possible cases:  (1) the user selects something from
		*   a menu, then presses F1.  (2) The user is already in help
		*   mode, selects something from a menu, presses return.  **/
		if ((lpmsg->wParam == VK_F1 && !vfControlKey && !vfOptionKey)
				||  (lpmsg->wParam == kcReturn && vfHelp))
			{
			CmdHelp(NULL);
			return fTrue;
			}
		}   /* end of switch (wCode) */

	return DefHookProc(wCode, wParam, lpmsg, (FARPROC FAR *) &vlppFWHChain);
}




#ifdef NOTUSED
/* %%Function:ChFromKc  %%Owner:NOTUSED */
ChFromKc(kc)
int kc;
{
	CHAR ch;

	switch (kc)
		{
	case kcPageBreak:
		ch = chSect;
		break;
	case kcColumnBreak:
		ch = chColumnBreak;
		break;
	case kcNonBreakSpace:
		ch = chNonBreakSpace;
		break;
	case kcNonReqHyphen:
		ch = chNonReqHyphen;
		break;
	case kcNonBreakHyphen:
		ch = chNonBreakHyphen;
		break;
	case kcNewLine:
		ch = chCRJ;
		break;
	default:
		ch = kc;
		}
	return (ch);
}


#endif /* NOTUSED */





/* F  C H E C K  T O G G L E  K E Y  M E S S A G E */
/* If the passed message is an up- or down- transition of a
	keyboard toggle key (e.g. shift), update our global flags & return
	fTrue; if not, return fFalse */
/* %%Function:FCheckToggleKeyMessage  %%Owner:peterj */
FCheckToggleKeyMessage(lpmsg)
LPMSG lpmsg;
{
	switch (lpmsg->message)
		{
	case WM_SYSKEYDOWN:    /* we'll get WM_SYSKEY messages if no
						window has the focus */
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
		switch (lpmsg->wParam)
			{
		case VK_NUMLOCK:
		case VK_CAPITAL:
		case VK_CONTROL:
		case VK_SHIFT:
			/* FUTURE: SetOurKeyState will not work in win3 if the msg
			   is just peeked at and not removed. We had to add calls
			   to do that in the callers, and should remove this call
			   once we agree it is safe	 bz
			*/


			/*            case VK_MENU:*/
			SetOurKeyState();
			return fTrue;
			/* Following returns fFalse iff Alt key is down */
			/*                return !(lpmsg->lParam & 0x20000000);*/
			}
		}

	return fFalse;
}


/* S p e c i a l  S e l  M o d e  E n d */
/* cancel special selection modes. Check globals to avoid bringing
	in code in the very common case when this routine has nothing to do. */

/* %%Function:SpecialSelModeEnd  %%Owner:peterj */
SpecialSelModeEnd()
{
	extern int vfBlockSel;
	extern int vfExtendSel;
	extern int vssc;

	if (vfBlockSel)
		BlockModeEnd();
	if (vfExtendSel)
		ExtendSelEnd();
	if (vssc != sscNil)
		CancelDyadic();
}


/* O U R  S E T  C U R S O R */
/* %%Function:OurSetCursor  %%Owner:peterj */
EXPORT PASCAL OurSetCursor(hc)
HCURSOR hc;
{
	if (vfHelp)
		hc = vhcHelp;
	else  if (vfInLongOperation)
		hc = vhcHourGlass;

	if (!vfMouseExist && hc != vhcHourGlass)
		hc = NULL;

#ifdef DEBUG
	else
		Assert (hc != NULL || vhcHourGlass == NULL);
#endif /* DEBUG */

	SetCursor(hc);
}



/*  these are here for swap-tuning purposes */

/*  RestorePrompt
	Remove the currently displayed prompt.  If there is some other prompt that
	should be displayed (based on the programs global mode) display that
	prompt.
*/

/* %%Function:RestorePrompt   %%Owner:peterj */
RestorePrompt ()

{
#ifdef RSH
	StopSubTimer(uasMenu);
#endif /* RSH */

	/* the following code should check the state of the program and assign to
		pmt the new prompt to display. */

	vfRestPmtOnInput = fFalse;
	vusecTimeoutPrompt = 0;

	if (vssc != sscNil)
		SetPromptVssc();
	else
		{
		if (vcchStatLinePrompt)
			DisplayStatLinePrompt(pdcRestore);
		}
	if (vhwndPrompt != NULL)
		HidePromptWindow();
	if (vhwndStatLine)
		UpdateWindow(vhwndStatLine);
	else
		UpdateWindow(vhwndDeskTop);
}


/* %%Function:HidePromptWindow  %%Owner:peterj */
HidePromptWindow()
{
	Assert(vhpprPRPrompt == hNil);
	/* don't really destroy, just hide! */
	SetWindowPos(vhwndPrompt, NULL, 0,0,0,0, 
			SWP_HIDEWINDOW|SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOMOVE);
	Assert(vhwndPromptHidden == vhwndPrompt);
	vhwndPrompt = NULL;
}


/*  SetPromptRestore
	Set the "restore prompt" information according to pdc.
*/

/* %%Function:SetPromptRestore   %%Owner:peterj */
SetPromptRestore (pdc)
int pdc;

{

	vusecTimeoutPrompt = 0;
	vfRestPmtOnInput = fFalse;
	if (pdc & pdcmRestore)
		{
		RestorePrompt();
		return;
		}
	if (pdc & pdcmRestOnInput)
		/*  restore on input */
		vfRestPmtOnInput = fTrue;
	if (pdc & pdcmTimeout)
		/*  restore on timeout */
		{
		vusecTimeoutPrompt = GetTickCount() + usecPromptTimeout;
		if (!vusecTimeoutPrompt)
			vusecTimeoutPrompt++;
		}
	/*  else: don't restore */
}


/* E n s u r e  F o c u s  I n  P a n e */
/* This is called by the recipients of some messages which may arrive
	while the user is in a ribbon or ruler combo box. It moves the
	focus back to the pane, closing the combo, if necessary */

/* %%Function:EnsureFocusInPane  %%Owner:peterj */
EnsureFocusInPane()
{
	HWND hwnd;

	if (!vfDeactByOtherApp && wwCur != wwNil && 
			(hwnd = PwwdWw(wwCur)->hwnd) != GetFocus())
		{
		if (vidf.fIBDlgMode)
			TermCurIBDlg(fFalse);  /* escapes without doing action */
		SetFocus(hwnd);
		}
}


/* F  I S  K E Y  M E S S A G E */
/* General key message handler.  Pass messages to this and it will return
fTrue if it dealt with it, fFalse otherwise.  This function knows about
insert mode. */
/* %%Function:FIsKeyMessage  %%Owner:peterj */
BOOL FIsKeyMessage(lpmsg)
LPMSG lpmsg;
{
	extern BOOL vfInsertMode;
	extern KME vkme;

#ifdef COMING_SOON_TO_A_WORD_PROCESSOR_NEAR_YOU
	/* and when is does, this flag should be part of vrf for 
		efficient DS usage */
	static BOOL fIconBarPending = fFalse;
#endif	
	int wm, kc;

	wm = lpmsg->message;
	kc = lpmsg->wParam;	/* key code or char */


#ifdef COMING_SOON_TO_A_WORD_PROCESSOR_NEAR_YOU

	/* This is currently commented out due to the fact that Windows seems to
	have a bug where they do not tell us when keys are released always... */


	ShowMsg("KM", lpmsg->hwnd, wm, kc, lpmsg->lParam);
	CommSz(SzShared("\r\n"));

	/* Deal with Alt+Shift for IconBarMode */
	/*CommSzNum(SzShared("f is "), fIconBarPending);*/
	/* What it does: start icon bar mode when Alt and Shift are pressed
		and released.
			
		How it works: fIconBarPending is set whenever both the Alt and 
		Shift keys are down and reset when any other key is pressed.  
		When both the Alt and Shift keys have been released, 
		fIconBarPending is checked.  If it is set, IconBarMode is 
		entered. */

	if (wm == WM_KEYDOWN || wm == WM_SYSKEYDOWN)
		{
		fIconBarPending = ((kc == VK_MENU || kc == VK_SHIFT) &&
				(GetKeyState(VK_MENU) & 0x8000) && 
				(GetKeyState(VK_SHIFT) & 0x8000));
		/*CommSzNum(SzShared("set f to "), fIconBarPending);*/
		}
	else  if (fIconBarPending && (!(GetKeyState(VK_MENU) & 0x8000) &&
			!(GetKeyState(VK_SHIFT) & 0x8000)))
		{
		CommSz(SzShared("Chaining...\r\n"));
		ChainCmd(bcmIconBarMode);
		fIconBarPending = fFalse;
		}
#endif /* COMING_SOON___ */



	/* FUTURE: TRASH THIS!!! (BAC) */
	if (FCheckToggleKeyMessage(lpmsg))
		goto LTranslate;


	switch (wm)
		{
	case WM_CHAR:
		/* this must come first! */
		if (vhwndCBT && !SendMessage(vhwndCBT, WM_CBTSEMEV,
				smvWmChar, MAKELONG(kc, 0)))
			{
			/* if CBT doesn't like it, kill it */
			GetMessage(lpmsg, NULL, 0, 0);
			return fTrue;
			}

		if (vlm == lmPreview)
			return fTrue;
		if (vfInsertMode)
			{
			GetMessage(lpmsg, NULL, 0, 0);
			/* indicate we have a char to insert */
			lpmsg->wParam |= 0x8000;
			return fTrue;
			}

LInsert:
		if (hwwdCur != hNil && lpmsg->hwnd == (*hwwdCur)->hwnd)
			{
			if (vfExtendSel) /* catch the extend to alpha case */
				{
				if (kc != kcBackSpace)
					{
					ExtendSelToCh(kc);
					return fTrue;
					}
				else
					{
					/* terminate Extend Mode before going into the insert code */
					/* cannot simply add these keys as terminate mode keys in the keymap
					because we still want the backspace and delete to be performed */
					ExtendSelEnd();
					}
				}

			/* don't allow typing with 2nd sel active */
			if (vssc != sscNil)
				Beep();
			else
				InsertLoopCh(kc, bcmNil);

			return fTrue;
			}
		break;

	case WM_SYSKEYDOWN:
		if (kc == VK_MENU)
			{
			if (vfInsertMode)
				return fFalse;
			goto LTranslate;
			}

	case WM_KEYDOWN:
		/* Get Alt key state from message, as vfOptionKey cannot
					be trusted. */
		if ((lpmsg->lParam & 0x20000000) != 0)
			{
			vgrpfKeyBoardState |= wKbsOptionMask;
			if ((uns)(kc - VK_F1) <= (VK_F2 - VK_F1))
				kc += 10; /* assumes VK_Fn's are contiguous */
			}
		else
			{
			vgrpfKeyBoardState &= ~wKbsOptionMask;
			}

		/* Get current Ctrl key state; OurGetKeyState cannot be trusted
			(sez bradch(rp)) */
		vgrpfKeyBoardState |= (GetKeyState(VK_CONTROL)<0) ? wKbsControlMask : 0;

		/* FUTURE: cursor keys will be in keymap someday... */
		/* but because they're not, need to special case preview */
		if (vlm != lmPreview && FCursKey(kc, !vfInsertMode /*fDoIt*/ ))
			return !vfInsertMode;


		kc = KcModified(kc); /* Apply Shift, Ctrl, etc. */

		if (vfInsertMode)
			{
			extern KME * PkmeOfKcInChain();
			KME * pkme;

#ifdef DEBUG	/* special debug key processing */
			if (FProcessDbgKey(kc))
				{
				/* message has been dealt with, throw away! */
				GetMessage(lpmsg, NULL, 0, 0);
				return fTrue;
				}
#endif /* DEBUG */

			if ((pkme = PkmeOfKcInChain(kc)) == 0)
				{
				/* key not in table, try to translate */
				GetMessage(lpmsg, NULL, 0, 0);
				/* Ctrl+ keys are not translated so we do
					not have to deal with the CHAR message. */
#ifdef INTL
				if (!TranslateMessage(lpmsg)) /* no beep on Ctrl+Alt */
#else
				if ((FCtrlKc(kc) && !FAltKc(kc)) || !TranslateMessage(lpmsg))
#endif
					Beep();
				}
			else  if (pkme->kt == ktInsert)
				{
				/* key is special insert type */
				GetMessage(lpmsg, NULL, 0, 0);
				/* indicate we have a char to insert */
				lpmsg->wParam = pkme->ch | 0x8000;
				}
			else
				{
				/* command key not handled yet, tell 
					insert loop to stop */
				return fFalse;
				}
			}
		else  /* !vfInsertMode */			
			{
			if (!FExecKc(kc) && vkme.kt == ktInsert)
				{
				kc = vkme.ch;
				goto LInsert;
				}
			}

		return fTrue;

		/* FUTURE: trash this when SetOurKeyState() is trashed (BAC) */
	case WM_MOUSEMOVE:
		if (vfInsertMode)
			return fFalse;

		/* Windows doesn't give us the shift KEYUP message if 
			mouse button is down!! */
		SetOurKeyState();
		/* FALL THROUGH */

	case WM_SYSKEYUP:
	case WM_KEYUP:
LTranslate:
		if (vfInsertMode)
			GetMessage(lpmsg, NULL, 0, 0);

		  
		SetOurKeyState(); /* must be done after msg removed for win3 */
		TranslateMessage(lpmsg);
		DispatchMessage(lpmsg);

		return fTrue;
		}

	return fFalse;
}




/*************************************************************************/
/**********************  CODE LRU MANIPULATIONS  *************************/
/*************************************************************************/



#define cparaStrip    (55*64)

/* sizes to request/get back */
csconst int mpsascpara[sasMin+1] = 
{
/*sas, swap area >= */
/*0*/  210*64, /* what we really want */
/*1*/  175*64, /* enough for SDM */
/*2*/  150*64, /* acceptable (won't load SDM) */
/*3*/  128*64, /* lowest "good" amount - get warning & lose fonts below here */
/*4*/  115*64,
/*5*/  100*64,
/*6*/  90*64,
/*7*/  60*64,
/*8*/  cparaStrip
};     /* number of elements MUST equal mpsaseid in init2.c! */


int vsasCur = sasMin;
int vcShrinkSwapArea = 0;
int vfNoCoreLoad;
int vcparaReserve;

/* REVIEW ship(pj): do i really want this to stay around? */
int vcparaMaxReqLast = 0;
int vcparaRequestLast = 0;
int vcparaSwapLast = 0;

/* O U R  S E T  S A S */
/* Set Opus's SwapAreaSize according to sas */
/* %%Function:OurSetSas  %%Owner:peterj */
EXPORT OurSetSas(sas)
int sas;
{
	int cparaSwap;

	Assert(sas >= 0 && sas <= sasMin);

	if (vfNoCoreLoad Debug(|| vdbs.fNoCoreLoad))
		{
		vsasCur = sasMin;
		return;
		}

	if (vcShrinkSwapArea != 0)
		/* someome is counting on the fact that we are using min swap area */
		sas = sasMin;

	/* block - compute amount to ask for */
		{
		int cparaMaxReq = CpMin(SetSwapAreaSize(0) + (GlobalCompact(0)/16)
				- vcparaReserve, 0x00007fffL);
		int cparaRequest = min(mpsascpara[sas], max(cparaStrip, cparaMaxReq));
		Assert(vfLargeFrameEMS == 0 || vfLargeFrameEMS == 1);
		cparaSwap = SetSwapAreaSize(cparaRequest);

		/*  for use by AppInfo$() */
		vcparaMaxReqLast  = cparaMaxReq;
		vcparaRequestLast = cparaRequest;
		vcparaSwapLast    = cparaSwap;

#ifdef DCORELOAD
		CommSzNum(SzShared("sas          = "), sas);
		CommSzNum(SzShared("cparaRequest = "), cparaRequest);
		CommSzNum(SzShared("cparaSwap    = "), cparaSwap);
#endif /* DCORELOAD */
		}

	/* given how much swap area we now have, what can we do with it? */
	for (vsasCur = sas; vsasCur < sasMin; vsasCur++)
		{
		if (mpsascpara[vsasCur] <= cparaSwap)
			break;
		}

#ifdef DCORELOAD
	CommSzNum (SzShared("\t\tvsasCur = "), vsasCur);
	CommSzNum (SzShared("\t\tSwap Area Available (K)= "), cparaSwap/64);
#endif /* DCORELOAD */
}


/* C O R E  N E W E S T */
/*  Makes newest (or loads) the Opus core.  Also loads portions of Windows
	needed for our "core" functionality.  Pays some attention to how much
	memory is available when loading.  Interruptable when loading.
*/
/* %%Function:CoreNewest  %%Owner:peterj */
CoreNewest(fLoad)
BOOL fLoad;
{
	int ihcd;
	extern int vsasCur;

	if (hmwdCur == hNil || vfDeactByOtherApp)
		return;

#ifdef DCORELOAD
	if (fLoad)
		CommSz(SzShared(" {"));
#endif /* DCORELOAD */

	/* SDM - all (just Newest it) */
	if (FAbortNewestCmg(cmgCoreSdmAll, fFalse, fTrue))
		goto LDone;

	if (FMsgPresent(mtyIdle))
		goto LDone;

	/* windows code for scroll bars */
	if ((vpref.fHorzScrollBar || vpref.fVertScrollBar) &&
			FAbortNewestCmg(cmgCoreWinScroll, (fLoad && vsasCur <= 2), fTrue))
		goto LDone;

	if (FMsgPresent(mtyIdle))
		goto LDone;

	/* windows core */
	if (FAbortNewestCmg(cmgCoreWindows, (fLoad && vsasCur <= 2), fTrue))
		goto LDone;

	if (FMsgPresent(mtyIdle))
		goto LDone;

	/*  optional core */
	if (vpref.fBkgrndPag &&
			FAbortNewestCmg(cmgCoreLayout, (fLoad && vsasCur <= 3), fTrue))
		goto LDone;

	if (FMsgPresent(mtyIdle))
		goto LDone;

	if ((vhwndRibbon || (hmwdCur != hNil && (*hmwdCur)->hwndRuler != NULL))
			&& FAbortNewestCmg(cmgCoreRibbon, (fLoad && vsasCur <= 4), fTrue))
		goto LDone;

	if (FMsgPresent(mtyIdle))
		goto LDone;

	if (vhwndStatLine &&
			FAbortNewestCmg(cmgCoreStatline, (fLoad && vsasCur <= 4), fTrue))
		goto LDone;

	if (FMsgPresent(mtyIdle))
		goto LDone;

	/* SDM - startup portion */
	if (FAbortNewestCmg(cmgDlgNew, (fLoad && vsasCur <= 1), fTrue))
		goto LDone;

	if (FMsgPresent(mtyIdle))
		goto LDone;

	/*  semi optional portion of core */
	if (FAbortNewestCmg(cmgCore1, (fLoad && vsasCur <= 5), fTrue))
		goto LDone;

	if (FMsgPresent(mtyIdle))
		goto LDone;

	/*  not really optional core */
	FAbortNewestCmg(cmgCore2, (fLoad && vsasCur <= 6), fTrue);

	if (FMsgPresent(mtyIdle))
		goto LDone;

	/*  not really optional core */
	FAbortNewestCmg(cmgCore3, (fLoad && vsasCur <= 7), fTrue);

LDone:
	;

#ifdef DCORELOAD
	if (fLoad)
		{
		static int cLoad = 0;
		CommSz(SzShared("} "));
		if (!(++cLoad & 15))
			CommSz(SzShared("\r\n"));
		}
#endif /* DCORELOAD */

}


/* F  A B O R T  N E W E S T  C M G */
/*  Makes Code Module Group identified by cmg newest.  If fLoad will force
	them to be loaded.  May be interrupted if fAbortOk.  Returns fTrue if
	interrupted, otherwise fFalse.
	See core.h for structures etc.
*/
/* %%Function:FAbortNewestCmg   %%Owner:peterj */
FAbortNewestCmg (cmg, fLoad, fAbortOk)
int cmg;
BOOL fLoad, fAbortOk;
{
	HANDLE hcd;
	int iihcd = 0;
	int ihcd;

#ifdef HYBRID
	if (vfShowSwapping)
		return fFalse;
#endif /* HYBRID */
#ifdef DEBUG
	if (vdbs.fNoCoreLoad)
		return fFalse;
#endif /* DEBUG */

	if (vfNoCoreLoad || vhwndCBT != NULL)
		return fFalse;

	/* so we don't kick them out on their way in! */
	if (fLoad && FAbortNewestCmg(cmg, fFalse, fAbortOk))
		return fTrue;

	while ((ihcd = mpcmgrgihcd [cmg][iihcd++]) != ihcdEnd)
		{
		MSG msg;
		if (fAbortOk && 
				PeekMessage(&msg, NULL, NULL, NULL, PM_NOYIELD|PM_NOREMOVE))
			{
#ifdef DCORELOAD
			CommSz(SzShared("FAbortNewestCmg aborted\r\n"));
#endif /* DCORELOAD */
			return fTrue;
			}

		if (fLoad || (hcd = rghcdModules [ihcd]) == NULL)
			{
			StartUMeas (umGetCodeHandle);
			hcd = rghcdModules[ihcd] = GetCodeHandle(mpihcdpfnModules[ihcd]);
			StopUMeas (umGetCodeHandle);
			}
		StartUMeas (umGlobalLru);
		GlobalLruNewest (hcd);
		StopUMeas (umGlobalLru);
		}

	return fFalse;
}


/* O L D E S T  C M G */
/*  makes modules in cmg oldest.
*/
/* %%Function:OldestCmg   %%Owner:peterj */
OldestCmg (cmg)
int cmg;
{
	int iihcd = 0;
	int ihcd;
	HANDLE hcd;

	while ((ihcd = mpcmgrgihcd [cmg][iihcd++]) != ihcdEnd)
		{
		if ((hcd = rghcdModules[ihcd]) == NULL)
			continue;
		StartUMeas (umGlobalLru);
		GlobalLruOldest (hcd);
		StopUMeas (umGlobalLru);
		}
}


/* G E T  N E X T  H C D */
/*  Finds the next NULL entry in rghcdModules and obtains the module handle 
	for it.
*/
/* %%Function:GetNextHcd  %%Owner:peterj */
GetNextHcd()
{
	int ihcd;
	for (ihcd = 0; ihcd < ihcdQuietlyLoadMax; ihcd++)
		{
		if (rghcdModules[ihcd] == NULL)
			{
			rghcdModules[ihcd] = GetCodeHandle(mpihcdpfnModules[ihcd]);
			Assert(rghcdModules[ihcd] != NULL);
			break;
			}
		}
#ifdef DCORELOAD
	if (ihcd == ihcdQuietlyLoadMax)
		{
		static int fReported = fFalse;
		if (!fReported)
			CommSz(SzShared("All desired handles in rghcd obtained\r\n"));
		fReported = fTrue;
		}
#endif /* DCORELOAD */
}


/*
Return true if the window is not visible */
/* %%Function:FWindowHidden  %%Owner:peterj */
FWindowHidden(ww)
int ww;
{

	Assert(ww);
	return !IsWindowVisible(PwwdWw(ww)->hwnd);
}


#ifdef DEBUG  /* debug routines that we would like to be in the core */

/* D u m m y */
/* A Do-nothing function useful for breakpointing in the middle of
	long routines */

/* %%Function:Dummy  %%Owner:peterj */
Dummy()
{
}


/* %%Function:NatDummy  %%Owner:peterj */
NATIVE NatDummy() /* WINIGNORE - DEBUG only */
{
}


ARG_RAREEVENTS RareEvent[RareEventMax];

/* F   R A R E   P R O C */
/* this procedure returns true if the selected rare event has
occured the selected number of times
		wEventNumber        identifier of event
		wRepetitions        current repetition count
		wRepetitionsMac     stop repetition count
		fOn                 count on/off flag
These variables will be typically set from the debugger or other debug
routines.
*/
/* %%Function:FRareProc  %%Owner:peterj */
NATIVE FRareProc(n) /* WINIGNORE - DEBUG only */
int n;
{
	int i;
	int fRetVal;

	fRetVal = fFalse;

	for (i=0; i < RareEventMax; i++)
		{
		if (RareEvent[i].fOn && (RareEvent[i].wEventNumber == n))
			if (RareEvent[i].wRepetitions < RareEvent[i].wRepetitionsMac)
				if (++RareEvent[i].wRepetitions == RareEvent[i].wRepetitionsMac)
					fRetVal = fTrue;
		}

	return(fRetVal);
}


/* %%Function:FRareNULL  %%Owner:peterj */
NATIVE FRareNULL(re,w) /* WINIGNORE - DEBUG only */
int re;
int w;
{
	int i;
	int fRetVal;

	fRetVal = w;
	if (!fRetVal)
		return NULL;

	for (i=0; i < RareEventMax; i++)
		{
		if (RareEvent[i].fOn && (RareEvent[i].wEventNumber == re))
			if (RareEvent[i].wRepetitions < RareEvent[i].wRepetitionsMac)
				if (++RareEvent[i].wRepetitions == RareEvent[i].wRepetitionsMac)
					fRetVal = NULL;
		}

	return(fRetVal);
}




/* A s s e r t  H  S z  L n */
/* If H is not a valid, nonfree handle, report an Assertion failure
	in file szFile, line line */

/* %%Function:AssertHSzLn  %%Owner:peterj */
AssertHSzLn( h, szFile, line )
VOID **h;
CHAR szFile[];
int line;
{
	struct DRF *pdrfList;
	extern struct SCC vsccAbove;

	for (pdrfList = vpdrfHead; pdrfList != NULL; pdrfList = pdrfList->pdrfNext)
		if (pdrfList->dr.hplcedl == h)
			{
			if (!(*((struct PLC **)h))->fExtBase)
				break;
			Assert (pdrfList->dr.hplcedl == &pdrfList->pplcedl);
			Assert (pdrfList->pplcedl == &pdrfList->dr.plcedl);
			return;
			}

	if (!FCheckHandle (sbDds, h))
		{
		if (!(h == vsccAbove.hplcedl || *h == &vsccAbove))
			AssertProc(szFile, line );
		}
}


#endif /* DEBUG */



/* %%Function:ForceCursorChange  %%Owner:peterj */
ForceCursorChange(hwnd, ppt, fExact)
HWND hwnd;
struct PT * ppt;
BOOL fExact;
{
	HCURSOR hc = NULL;
	struct WWD *pwwd;
	BOOL fOtlEmpty;
	extern struct SEL selCur;
	extern int vfInLongOperation;
	extern int mwCur;
	extern HWND vhwndPgPrvw;
	extern struct WWD **mpwwhwwd[];
	extern HCURSOR vhcArrow, vhcRecorder, vhcIBeam, vhcBarCur,
	vhcHelp, vhcStyWnd, vhcOtlCross, vhcColumn;
	extern BOOL vfHelp, vfRecording;
	extern struct WWD **hwwdCur;

	vidf.fCorrectCursor = fFalse;

	if (vfHelp)
		hc = vhcHelp;
	else  if (vfInLongOperation)
		hc = vhcHourGlass;
	else  if (hwnd == NULL || vhwndPgPrvw != NULL)
		hc = vhcArrow;
	else  if (vfRecording)
		hc = vhcRecorder;
	if (hc != NULL)
		goto lblSet;

	hc = vhcArrow;
	if (hwwdCur != hNil && hwnd == (pwwd = *hwwdCur)->hwnd)
		{
		int ww = wwCur;
		int xw = ppt->xw;
		int dxp = vsci.dxpBorder << 1;

		Assert(ww != wwNil);
		Assert(wwCur == GetWindowWord(hwnd, IDWWW));
		Assert(mwCur == pwwd->mw);

		if (pwwd->xwSelBar && xw < pwwd->xwSelBar + dxp)
			{
			Assert(!pwwd->fPageView);
			if (xw <= pwwd->xwSelBar && 
					xw >= pwwd->xwSelBar - dxp /* fNearSty */)
				{
				if (vhcStyWnd == NULL && !FAssureIrcds(ircdsStyWnd))
					hc = vhcArrow;
				else
					hc = vhcStyWnd;
				}
			else
				hc = vhcBarCur;
			}
		else  if (ppt->yw >= pwwd->ywMin)
			{
			if (fExact)
				{
				int idr;
				int dl;
				struct PLDR **hpldr;
				struct PT  ptT;
				struct SPT spt;

				ptT = *ppt;
				if ((fOtlEmpty = FOutlineEmpty(ww, fFalse)) ||
						(dl = DlWherePt(ww, &ptT, &hpldr, &idr, &spt, fTrue, fTrue))
						== dlNil)
					goto lblSet;
				if (spt.fWholeColumn)
					hc = vhcColumn;
				else  if (spt.fSelBar)
					{
					hc = vhcBarCur;
					dl = DlWherePt(ww, &ptT, &hpldr, &idr, &spt, fFalse, fFalse);
					if ((*hwwdCur)->fOutline && !fOtlEmpty && dl != dlNil &&
							FXwInBullet(ww, dl, ppt->xw))
						{
						int yp, ywBottom;
						struct EDL edl;
						GetPlc(PdrGalley(PwwdWw(ww))->hplcedl, dl, &edl);
						yp = edl.ypTop + edl.dyp;

						ywBottom = 	YwFromYp(((edl.hpldr != hNil) ?
								edl.hpldr : HwwdWw(ww)), 0, yp);
						if (ppt->yw <= ywBottom)
							hc = vhcOtlCross;
						}
					}
				else 					
					{
					hc = (selCur.fGraphics ? vhcArrow : vhcIBeam);
					}
				}
			else
				{
				vidf.fCorrectCursor = fTrue;
				hc = vhcIBeam;
				}
			}
		}

lblSet:
	SetCursor( hc );
}


/* L  M E N U  C H A R */
/*
	Handle WM_MENUCHAR messages.  Returns 0 if message should be 
	passed to DefWindowProc().  Non-0 return values should be returned 
	by the window proc receiving the message.
*/
/* %%Function:LMenuChar  %%Owner:peterj */
LONG LMenuChar(hwnd, wParam, lParam)
HWND hwnd;
WORD wParam;
LONG lParam;
{
	if (vhwndAppModalFocus != NULL)
		return MAKELONG(0, 1);

	if (wParam == '-' && !(LOWORD(lParam) & MF_POPUP) && hwwdCur != hNil)
		{
		if (vfSysMenu)
			{
			if (hwnd == vhwndApp)
				return MAKELONG(0,2);

			PostMessage(vhwndApp, WM_MENUCHAR, wParam, lParam);
			return MAKELONG(0, 1);
			}

		if (hwnd == vhwndApp)
			{
			vfChildSysMenu = fTrue;
			return MAKELONG(0, 1);
			}

		return MAKELONG(0, 2);
		}

	return 0;
}


/* I N H I B I T  R E C O R D E R */
/*
   Called to inhibit or allow recording operation for the subsequent
   actions. Inhibits when fInhibit is true.  The state of both vfRecording
   and vfRecordNext are saved in *pwsave, so the saved value should not
   used by any other function.  The saved value will either be 0 if
   neither flag was on, 1 if vfRecording if on, or 2 if both flags are on.
   Note that vfRecordNext cannot be on if vfRecording is not on, so we can
   simply add the flags to give a numerical value of 0, 1, or 2.
/*

/* %%Function:InhibitRecorder  %%Owner:tonykr */
InhibitRecorder(pwSav, fInhibit)
BOOL *pwSav, fInhibit;
{
	Assert(fTrue == 1 && fFalse == 0);
	Assert(vfRecording == fTrue || vfRecording == fFalse);
	Assert(vfRecordNext == fTrue || vfRecordNext == fFalse);
	Assert((2 == 2) == 1);

	if (fInhibit)
		{
		*pwSav = vfRecording + vfRecordNext;
		Assert(*pwSav != 1 || !vfRecordNext);
		vfRecording = fFalse;
		vfRecordNext = fFalse;
		}
	else
		{
		if (vhcmr == hNil && !vfRecordNext && *pwSav != 2)
			{
			vfRecording = fFalse;
			vfRecordNext = fFalse;
			}
		else
			{
			vfRecordNext = ((*pwSav == 2 || vfRecordNext) && vhcmr == hNil);
			vfRecording = (*pwSav != 0 || vfRecording);
			}
		selCur.fUpdateStatLine = fTrue; /* force a stat line update in Idle */
		}
}


#ifdef DEBUG


extern struct PLSBHI ** vhplsbhi;
BOOL vfBogusHeapState = fFalse;

/* R E P O R T  H E A P */
/* %%Function:ReportHeapSz  %%Owner:peterj */
ReportHeapSz (sz, cbDiff)
CHAR *sz;
int cbDiff;
{
	static uns cbHeapLast = 0;
	static uns cbTotalLast = 0;
	static long cbFarLast = 0L;

	if (vfBogusHeapState)
		return;

		{
		uns cbHeap = CbAvailHeap(sbDds);
		uns dcbDS = cbMaxSbDds - CbSizeSb(sbDds);
		uns cbTotal = cbHeap + dcbDS;
		long cbFarUsed=MemUsed(1)-CbSizeSb(sbDds);
		long cbFarAvail=LcbUnusedEmm();
		long cbEmmUsed=MemUsed(2)-((long)vbptbExt.ibpMax*cbSector)-cbFarAvail;
		long dcbFar;
		long cbEmm = smtMemory==smtLIM ? CbFreeEmm() : 0L;
		CHAR rgch [100], *pch = rgch;

		dcbFar = cbFarUsed + cbEmmUsed - cbFarLast;

		if (abs(cbHeap-cbHeapLast)>=cbDiff || abs(cbTotal-cbTotalLast)>=cbDiff
				|| dcbFar >= cbDiff || -dcbFar >= cbDiff)
			{
			SetBytes(rgch, ' ', 100);
			CchPchToPpch(sz, &pch);
			pch = max (pch, rgch+13);
			CchPchToPpch(SzShared(" local="), &pch);
			CchUnsToPpch(cbHeap, &pch);
			*pch++ = '+';
			CchUnsToPpch(dcbDS, &pch);
			*pch++ = '=';
			*pch++ = ' ';
			CchUnsToPpch(cbTotal, &pch);
			pch = max (pch, rgch+33);
			CchPchToPpch(SzShared("  farUsed="), &pch);
			CchLongToPpch(cbFarUsed, &pch);
			if (smtMemory==smtLIM)
				{
				*pch++='+';
				CchLongToPpch(cbEmmUsed, &pch);
				}
			if (smtMemory==smtLIM)
				{
				pch = max (pch, rgch+59);
				CchPchToPpch(SzShared(" emm="), &pch);
				CchLongToPpch(cbEmm, &pch);
				*pch++='+';
				CchLongToPpch(cbFarAvail, &pch);
				}
			*pch++ = '\r';
			*pch++ = '\n';
			*pch = 0;
			CommSz(rgch);
			cbHeapLast = cbHeap;
			cbTotalLast = cbTotal;
			cbFarLast = cbFarUsed+cbEmmUsed;
			}
		}
}


BOOL vfAllocGuaranteed;		/* alloc should succeed unconditionally */
BOOL vfInDebug;                 /* in debug code, don't force failures */


/* R E P O R T  F A I L  C A L L S */
/* %%Function:ReportFailCalls  %%Owner:peterj */
ReportFailCalls()
{
	if (vdbs.cWinCalls || vdbs.cLmemCalls)
		{
		CHAR rgch[cchMaxSz], *pch = rgch;
		CchPchToPpch(SzShared("\r\nMem calls = "), &pch);
		CchUnsToPpch(vdbs.cLmemCalls, &pch);
		CchPchToPpch(SzShared(" Win calls = "), &pch);
		CchUnsToPpch(vdbs.cWinCalls, &pch);
		*pch++ = '\r';
		*pch++ = '\n';
		*pch = 0;
		CommSz(rgch);
		vdbs.cLmemCalls = 0;
		vdbs.cWinCalls = 0;
		}
}


/* F F A I L  L M E M  O P U S */
/* Return fTrue to cause lmem allocation to fail, fFalse to allow them to
	succeed.
*/
/* %%Function:FFailLmemOpus   %%Owner:peterj */
EXPORT FFailLmemOpus (merr, sb, cbNew, cRetry)
WORD merr;
SB sb;
WORD cbNew;
WORD cRetry;
{
	extern BOOL vfInitializing;
	extern BOOL vfInCmd;

	if (vfBogusHeapState)
		return fFalse;

	if (!vfInitializing)
		{
#ifdef DLMEM
		int rgw [5];
		rgw[0] = merr;
		rgw[1] = sb;
		rgw[2] = cbNew;
		rgw[3] = CbSizeSb(sb);
		rgw[4] = CbAvailHeap(sb);
		CommSzRgNum(SzShared("FFailLmemOpus (merr,sb,cbNew,cbSb,cbAvail): "), rgw, 5);
#endif /* DLMEM */

		if (vdbs.fReportHeap)
			ReportHeapSz(SzShared("memory usage."), 200);
		}

	Assert (vdbs.cLmemSucceed >= 0 && vdbs.cLmemFail >= 0);
	Assert (cHpFreeze == 0 || sb != sbDds);

	if (vfAllocGuaranteed || vfInDebug)
		return fFalse;

	Assert(!vfInCommit);

	if (vdbs.fOutsideCmd || vfInCmd)
		{
		if (vdbs.fHeapAct)
			CommSz(SzShared("h"));
		vdbs.cLmemCalls++;
		if (vdbs.cLmemSucceed)
			{
			if (vdbs.fHeapAct)
				CommSz(SzShared("s "));
			vdbs.cLmemSucceed--;
			return fFalse;
			}
		if (vdbs.cLmemFail)
			{
			if (vdbs.fHeapAct)
				CommSz(SzShared("f "));
			vdbs.cLmemFail--;
			if (vdbs.fBreakOnFail)
				DebugBreak(1);
			if (sb == sbDds)
				FreePh(&vmerr.hrgwEmerg1);
			return fTrue;
			}
		if (vdbs.fHeapAct)
			CommSz(SzShared(" "));
		}

	return fFalse;
}


#endif /* DEBUG */



#ifdef DEBUG
/* %%Function:CkFarHeaps  %%Owner:peterj */
CkFarHeaps()
{
	struct SBHI *psbhi = PInPl(vhplsbhi, 0);
	struct SBHI *psbhiMac = psbhi + (*vhplsbhi)->isbhiMac;
	VOID **ppv;
	int ch;

	while (psbhi < psbhiMac)
		{
		if (psbhi->fHasHeap)
			{
			CheckHeap(psbhi->sb);
			if (vdbs.fShakeHeap)
				ShakeHeapSb(psbhi->sb);
			ch = 0;
			ppv = NULL;
			for (;;)
				if ((ppv = PpvWalkHeap(psbhi->sb, ppv)) == NULL)
					break;
				else
					ch++;
			Assert(ch == psbhi->ch);
			}
		else
			{
			Assert(CbSizeSb(psbhi->sb) > cbMinBig-dcbBigSmall);
			Assert(psbhi->ch == 1);
			Assert(!psbhi->fHasFixed);
			}
		/* other checks */
		Assert(psbhi->ch || psbhi->fHasFixed);
		psbhi++;
		}
}


#endif /* DEBUG */



#ifdef DEBUG
/* %%Function:SbMgrError  %%Owner:peterj */
SbMgrError(sberr, sb)
int sberr;
SB sb;
{
	extern BOOL		fElActive;

	IdPromptBoxSz(SzShared("SB Manager interface error"),MB_TROUBLE);

	if (fElActive)
		{
		FlagRtError(rerrInternalError);
		}
}


#endif  /* DEBUG */

int vfMsg=fFalse;	/* tell caller whether msg arrived */

/* F  S t i l l  D o w n  R e p l a y */
/* %%Function:FStillDownReplay  %%Owner:peterj */
FStillDownReplay( ppt, fRightButton )
struct PT   *ppt;
BOOL fRightButton;
	{   /* This is roughly equivalent to a Mac routine that returns whether
		the mouse button is down.  We look for one mouse message from our
		window's queue, and return FALSE if it is a BUTTONUP.  We return the
		point at which the mouse event occurred through a pointer.  If no
		message occurred, we return whether the left button is down
		and do not store into the pointer
	
		Note that we specifically do not return the state of the right button;
		our right button processing is handled at the top of DoContentHit, and
		we don't want to deal with dragging that button in this loop. (bz).
*/
	MSG msg;

	/* Since re-entrancy can move the heap sometimes, shake it */

	Debug(vdbs.fShakeHeap ? ShakeHeap() : 0);

	if ( (vfMsg = PeekMessage( (LPMSG)&msg, (HWND)NULL, NULL, NULL, TRUE )) )
		{
		switch (msg.message) 
			{
		default:
			TranslateMessage( (LPMSG)&msg );
			DispatchMessage( (LPMSG)&msg );
			break;

		case WM_RBUTTONUP:
		case WM_LBUTTONUP:
			if (fRightButton != (msg.message == WM_RBUTTONUP))
				break;

			ppt->xp = MAKEPOINT( msg.lParam ).x;
			ppt->yp = MAKEPOINT( msg.lParam ).y;
			return fFalse;

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONDBLCLK:

		case WM_MOUSEMOVE:
			/* A Mouse Move, Mouse Down, or Mouse Up is waiting */
			ppt->xp = MAKEPOINT(msg.lParam).x;
			ppt->yp = MAKEPOINT(msg.lParam).y;
			break;
			}
		}

	return (GetKeyState( fRightButton ? VK_RBUTTON : VK_LBUTTON ) < 0);
}


/* %%Function:CloseEveryFn %%Owner:peterj */
CloseEveryFn( fHardToo )
	{   /* Close all files we have open. Close only files on removable media
	if fHardToo is FALSE; ALL files if fHardToo is TRUE */
	extern int fnMac;
	int fn;
	struct FCB **hfcb;

	for ( fn = 0; fn < fnMac; fn++ )
		if ((hfcb = mpfnhfcb [fn]) != NULL)
			{
			struct FCB *pfcb = *hfcb;
			int osfn = pfcb->osfn;

			if (osfn != osfnNil)
				{
				if (fHardToo || !pfcb->ofh.fFixedDisk)
					{
/* Close may fail if windows already closed the file for us, but that's OK */
					FCloseDoshnd( osfn );
					pfcb->osfn = osfnNil;
					}
				}
			}
}


/* E N A B L E  P R E L O A D */
/*  Make subsequent accesses to fn load large chunks.
	Does not guarantee that preloading will be done for fn (if a
	preload is already in progress, this one is ignored).
*/
/* %%Function:EnablePreload %%Owner:peterj */
EnablePreload(fn)
int fn;
{
	if (!vcPreload++)
		{
		Assert(vfnPreload == fnNil);
		vfnPreload = fn;
		}
}


/* %%Function:DisablePreload %%Owner:peterj */
DisablePreload()
{
	if (--vcPreload == 0)
		vfnPreload = fnNil;
	Assert(vcPreload >= 0);
}





#ifdef PROFILE
/* P C O D E  S T A R T  P R O F I L E R */
/* %%Function:PcodeStartProfiler  %%Owner:peterj */
PcodeStartProfiler()
{
}


/* P C O D E  S T O P  P R O F I L E R */
/* %%Function:PcodeStopProfiler  %%Owner:peterj */
PcodeStopProfiler()
{
}


#endif /* PROFILE */


#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Wproc_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* Wproc_Last() */
