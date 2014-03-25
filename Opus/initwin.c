/* I N I T W I N . C */

#define NONCMESSAGES
#define NODRAWFRAME
#define NORASTEROPS
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
#include "preview.h"
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
#include "scc.h"
#include "debug.h"
#include "dmdefs.h"
#include "status.h"
#include "core.h"
#include "preffile.h"
#include "ibdefs.h"
#include "outline.h"
#include "ch.h"
#include "rsb.h"
#include "error.h"
#include "rareflag.h"
#include "table.h"

extern MUD ** HmudInit();

extern BOOL             vfScratchFile;
extern struct DMQD        DMQueryD;
extern struct DMFLAGS     DMFlags;
extern struct DMQFLAGS    DMQFlags;
extern BOOL             vfFileCacheDirty;
extern int              docGlobalDot;
extern HMENU            vhMenuLongFull;
extern int              vwWinVersion;
extern struct FTI       vfti;
extern struct FTI       vftiDxt;
extern struct STTB      **vhsttbFont;
extern struct STTB      **vhsttbOpen;
extern struct PRI       vpri;
extern struct PRSU      vprsu;
extern CHAR             szEmpty[];
extern CHAR             stEmpty[];
extern HCURSOR          vhcBarCur;
extern HCURSOR          vhcHourGlass;
extern HCURSOR          vhcIBeam;
extern HCURSOR          vhcArrow;
extern HCURSOR          vhcSplit;
extern HCURSOR          vhcStyWnd;
extern HCURSOR          vhcRecorder;
extern HCURSOR          vhcHelp;
extern HCURSOR		vhcOtlCross;
extern HCURSOR		vhcOtlVert;
extern HCURSOR		vhcOtlHorz;
extern HCURSOR		vhcColumn;
extern HICON            vhiWord;
extern HANDLE           vhInstance;
extern HWND             vhwndApp;
extern HWND             vhwndSizeBox;
extern int              vfAllocGuaranteed;
extern HWND             vhwndCBT;
extern struct SCI	vsci;
extern struct STTB    **vhsttbWnd;
extern int              vfInitializing;
extern int              vfMouseExist;
extern uns              cbMemChunk;
extern BOOL             vfSingleApp;
extern int              vdbmgDevice;
extern BOOL             vfLargeFrameEMS;
extern struct ITR	vitr;
extern PVS		vpvs;
#ifdef WIN23
extern HFONT		hfontStatLine;
extern int			vdxpDigit;
extern LONG FAR PASCAL IconDlgWndProc();
extern FARPROC  lpfnIconDlgWndProc;
#endif /* WIN23 */

/* G L O B A L S */
struct FTI      vfti;           /* primary device connection */
struct FTI      vftiDxt;        /* secondary device connection */
struct FCE      rgfce[ifceMax]; /* font cache */
struct STTB     **vhsttbFont;   /* Master font table */
struct WWD      **hwwdCur; /* handle to the current pane inside a child window */
int             wwCur; /* index to the current window pane */
struct MWD      **hmwdCur; /* handle to the current child window */
int             mwCur; /* index to the current child window */
/* tsMruOsfn is the Most Recently Used Operating System File timestamp.
	It is used by low level file routines to implement a LRU strategy for
	opening and closing files. */
TS              tsMruOsfn;

struct BPTB     vbptbExt;
/* indicates that the scratch file exists; if false, scratch file is kept
	entirely in memory */
int             vfScratchFile;
/* indicates that we have not yet tried to create the scratch file */
int		vfTriedScratchFile;

/* stores handles to paths for template, ini, utilities, program */
struct EFPI dnfpi[fpiStoreMax];
int             docGlsy = docNil;
int             docGlobalDot = docNil;
int             vdocTemp = docNil;      /* temporary doc used for fSpec characters */
int             vdocScratch = docNil;   /* globally available scratch doc */
int             vdocHdrEdit = docNil;
BOOL				 vfAs400 = fFalse;

#ifdef DEBUG
BOOL            fDocScratchInUse = fFalse;
#endif /* DEBUG */

int             cfRTF = cfNil;    /* registered clipboard formats */
int             cfLink = cfNil;
int             cfPrPic = cfNil;
KMP **  hkmpTable = NULL;

FARPROC lpFontNameEnum;
FARPROC lpFontSizeEnum;

FARPROC lpFPrContinue;
FARPROC lpFCheckPopupRect;

/* for metafile display interruption */
FARPROC lpFMFContinue;

#ifdef DEBUG
struct DBS vdbs;
int	vfCheckPlc = fTrue;	/* PLC checking */
int	vfInt3Handler = fFalse;
#endif
#ifdef PROFILE
int vpfi = -1;
#endif /* PROFILE */

/* Window class strings.  Must be < 39 char long.  Do not localize. */
char szClsApp[] =       "OpusApp";
char szClsMwd[] =       "OpusMwd";
char szClsPgPrvw[] =    "OpusPrvw";
char szClsStatLine[] =  "OpusStat";
char szClsSplitBar[] =  "OpusSBar";
char szClsRSB[] = 	"OpusRSB";
char szClsRulerMark[] = "OpusRul";
char szClsDeskTop[] =   "OpusDesk";
char szClsWwPane[] =    "OpusWwd";
char szClsPrompt[] =    "OpusPmt";
char szClsStaticEdit[]= "OpusSEdit";
char szClsDdeChnl[] =   "OpusDde";
char szClsFedt[] =      "OpusFedt";
char szClsStart[] =     "OpusStart";
char szClsIconBar[] =   "OpusIcnBar";
int             vwWinVersion;   /* WIN version #, BCD: maj in hi, min in lo */
int             vfLargeFrameEMS;/* kernel is using large frame EMS */
int             vfSmallFrameEMS;/* interesting?? */
int				vcbitSegmentShift; /* shift segment by this at seg bounds */

int vkPlus;
int vkMinus;
int vkStar;

#ifdef INTL
int vkUnderline;
int vkEquals;
int vkQuestionmark;
#endif /* INTL */

long tickNextYield = 0; /* last time we called PeekMessage or otherwise yielded */


/* E X T E R N A L S */
extern struct PREF      vpref;
extern struct PREF      vprefCBT;  /* pref's before CBT run; restore after */
/* extern struct SAB       vsab; */

extern struct BPTB      vbptbExt;
extern int              vrgsbReserve[];
extern struct SEL       selCur;

extern struct FKPD      vfkpdChp;
extern struct FKPD      vfkpdPap;
extern struct FKPD      vfkpdText;
extern BYTE             fProtectModeWindows;
extern struct WWD       **mpwwhwwd[];
extern struct FCB       **mpfnhfcb[];
extern struct WWD       **hwwdCur;

extern struct SCC       vsccAbove;

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

/* Clipboard formats */
csconst CHAR szClipRTF[] = SzSharedKey("Rich Text Format",RichTextFormat);
csconst CHAR szClipLink[] = SzSharedKey("Link",Link);
csconst CHAR szClipPrPic[] = SzSharedKey("Printer_Picture",PrPic);

#ifdef DEBUG
CHAR vchRTFTest = 0; /* indicates special RTF test option specified */
#endif /* DEBUG */

extern BOOL FAR PASCAL  FontNameEnum();
extern BOOL FAR PASCAL  FontSizeEnum();
extern FARPROC          lpFontNameEnum;
extern FARPROC          lpFontSizeEnum;

#ifdef DEBUG
extern struct SCC       *vpsccSave;
extern int    vcwHeapMacDbg;   /* max that can be allocated on heap */
#endif  /* DEBUG */

/* W I N D O W   C L A S S   I N F O R M A T I O N */

extern long FAR PASCAL NatAppWndProc();
extern long FAR PASCAL NatMwdWndProc();
extern long FAR PASCAL NatStatLineWndProc();
extern LONG FAR PASCAL NatPgPrvwWndProc();
extern LONG FAR PASCAL NatSplitBarWndProc();
extern LONG FAR PASCAL NatPromptWndProc();
extern LONG FAR PASCAL NatDeskTopWndProc();
extern LONG FAR PASCAL NatWwPaneWndProc();
extern LONG FAR PASCAL NatRulerMarkWndProc();
extern LONG FAR PASCAL NatIconBarWndProc();
extern LONG FAR PASCAL NatDdeChnlWndProc();
extern LONG FAR PASCAL NatRSBWndProc();
extern LONG FAR PASCAL NatFedtWndProc();
extern LONG FAR PASCAL NatStartWndProc();
extern LONG FAR PASCAL StaticEditWndProc();


extern char szClsApp[];
extern char szClsMwd[];
extern char szClsPgPrvw[];
extern char szClsStatLine[];
extern char szClsSplitBar[];
extern char szClsRulerMark[];
extern char szClsDeskTop[];
extern char szClsWwPane[];
extern char szClsPrompt[];
extern char szClsStaticEdit[];
extern char szClsDdeChnl[];
extern char szClsRSB[];
extern char szClsFedt[];
extern char szClsStart[];
extern char szClsIconBar[];

#define idColorNil     (-1)

	struct WC  {
	WORD    style;
	int     cbWndExtra;
	int     idBackgroundColor;       /* COLOR constant, e.g. COLOR_BACKGROUND */
	CHAR  * szCls;
	FARPROC lpfnWndProc;
};

/* NOTE: Casts to FARPROC in initializer are a workaround for a CS
	compiler bug */

csconst struct WC rgwc [] =
	{ 
		{ CS_BYTEALIGNCLIENT,                                 /* Parent */
	0, COLOR_APPWORKSPACE+1, szClsApp, (FARPROC)NatAppWndProc },
	

	{ 0,                                                  /* MDI window */
	CBWEMW, idColorNil, szClsMwd, (FARPROC)NatMwdWndProc },
	

	{ CS_DBLCLKS,                                         /* Page Preview */
	0, idColorNil, szClsPgPrvw, (FARPROC)NatPgPrvwWndProc },
	

	{ CS_DBLCLKS,                                         /* Status Line */
	0, idColorNil, szClsStatLine, (FARPROC)NatStatLineWndProc },
	

	{ 0,                                                  /* Split Bar */
	0, COLOR_WINDOW+1, szClsSplitBar, (FARPROC)NatSplitBarWndProc },
	

	{ CS_DBLCLKS,                                         /* RulerMark */
	0, COLOR_WINDOW+1, szClsRulerMark, (FARPROC)NatRulerMarkWndProc },
	

	{ 0,                                                  /* DeskTop */
	0, COLOR_APPWORKSPACE+1, szClsDeskTop, (FARPROC)NatDeskTopWndProc },
	

	{ CS_OWNDC | CS_DBLCLKS,                              /* WwPane */
	CBWEWW, idColorNil, szClsWwPane, (FARPROC)NatWwPaneWndProc },
	

	{ 0,                                                  /* Prompt */
	0, idColorNil, szClsPrompt, (FARPROC)NatPromptWndProc },
	

	{ 0,                                                  /* Static Edit */
	CBWESTATICEDIT, idColorNil, szClsStaticEdit, (FARPROC)StaticEditWndProc },
	

	{ 0,                                                  /* Dde Channel */
	CBWEDDECHNL, idColorNil, szClsDdeChnl, (FARPROC)NatDdeChnlWndProc },
	

	{ CS_DBLCLKS | CS_HREDRAW,                            /* Our Editcontrol */
	CBWEFEDT, idColorNil, szClsFedt, (FARPROC)NatFedtWndProc },
	
	
	{ 0,	    	    	    	    	    	    	/* Startup dialog */
	0, idColorNil, szClsStart, (FARPROC)NatStartWndProc },
	

	{ CS_DBLCLKS,                                         /* Icon Bar */
	CBWEICONBAR, COLOR_WINDOW+1, szClsIconBar, (FARPROC)NatIconBarWndProc },
	

	{ CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,		/* Rational Scrl Bar */
	cbweRSB, idColorNil, szClsRSB, (FARPROC)NatRSBWndProc },

};


#define iwcApp	0
#define iwcRSB	14
#define iwcMax	15 /* # of entries in rgwc above */

#ifdef WIN23
#define iwcStatLine 3
#define iwcSplitBar 4
#define iwcRulerMark 5
#define iwcIconBar 13
#endif /* WIN23 */

/* L O C A L   D E C L A R A T I O N S */

extern HBRUSH vhbrLtGray;
extern HBRUSH vhbrGray;
extern HBRUSH vhbrWhite;
extern HBRUSH vhbrBlack;
extern HBRUSH vhbrDkGray;

extern HWND vhwndRibbon;
extern HWND vhwndStatLine;
extern HWND vhwndDeskTop;
extern HWND vhwndStartup;

extern FARPROC lpFCheckPopupRect;
BOOL FAR PASCAL FCheckPopupRect();
extern FARPROC lpprocStaticEditWndProc;
/* Already extern declared */
/* LONG FAR PASCAL StaticEditWndProc; */

extern FARPROC lpprocBlinkCaretSEdit;
BOOL FAR PASCAL BlinkCaretSEdit();


extern FARPROC lpFPrContinue;
BOOL FAR PASCAL FPrContinue();

extern FARPROC  lpFMFContinue;
extern BOOL FAR PASCAL FMFContinue();

extern FARPROC lpFBltOutlineSplat;
BOOL FAR PASCAL FBltOutlineSplat();

/* Dialog key mapper stuff */
extern FARPROC        vlppFWHChain;
extern FARPROC        lppFWHKey;
LONG FAR PASCAL       FWHKey();

/*               F  I n i t  P a r t 1
*  Called from FInitWinInfo to do first part 1 of initialization of the 
*  whole world. Returns FALSE if the initialization failed, TRUE if it 
*  succeeded. After calling FInitPart1 you should call FInitPart2. Parts
*  1 and 2 have been split up to reduce the amount of code space needed
*  during initialization. */

/* %%Function:FInitPart1 %%Owner:PETERJ */
int FInitPart1( hInstance, hPrevInstance, lpszCmdLine, 
rgchCmdLine, rgpchArg, pipchArgMac, pfOpenUntitled, pfTutorial  )
HANDLE hInstance, hPrevInstance;
LPSTR  lpszCmdLine;
CHAR rgchCmdLine [ichMaxCmdLine+1];
CHAR *rgpchArg [ipchArgMax];
int *pipchArgMac;
BOOL *pfOpenUntitled;
BOOL *pfTutorial;
{
	extern int vwWinVersion;
	int eidFail = eidCantRunM;
	int ipchArgMac;

	vhInstance = hInstance;

/* NOTE: At this point we know how much memory we have (we can calculate the
	worst case for memory allocations against our minimum heap size,
	defined in out .DEF file).  For that reason we do not need to check for
	failure on the allocations that follow. This is true up until the first
	user variable -- reading WINWORD.INI. */

	Debug(vfAllocGuaranteed = fTrue);

	if (!FNativeInit())
		{
		eidFail = eidWrongWinOrDos;
		goto InzFailed0;
		}
	/* This should be very early in initialization! */
	/* Opus does not currently support multiple instances; return if the user
		tries it */
	if (hPrevInstance)
		{
		eidFail = eidWordRunning;
		goto InzFailed0;
		}

#ifdef WIN23
	if (vwWinVersion >= 0x0300)
		{
		/* we need to know what device we are running on to decide whether
			or not to do Win3Stuff, so do this here */
		HDC hdc;
		int wNewLook = GetProfileInt( szApp, SzShared("NewLook"), 2);
		vsci.fWin3 = fTrue;
		if ((hdc = GetDC(NULL)) == NULL)
			goto InzFailed0;
		vsci.dypScreen = GetDeviceCaps(hdc,VERTRES);
		vsci.fMonochrome = (GetDeviceCaps(hdc, NUMCOLORS) == 2);
		/****
		If wNewLook == 0, then we never use Win3Visuals
		If wNewLook == 1, then we run Win3Visuals on EGA or greater.
		If wNewLook == 2 (it wasn't in the ini), then we run Win3Visuals
			on VGA or greater.
		****/
		/* 350 (EGA) will pass for wNewLook == 1, but not == wNewLook == 2 */
		vsci.fWin3Visuals = wNewLook && !vsci.fMonochrome &&
			(vsci.dypScreen >= 350 - 1 + wNewLook);
		ReleaseDC( NULL, hdc);
		}
#endif /* WIN23 */
#ifdef DEBUG
	/* This should also be very early in initialization!  (i.e., second) */
	ReadDebugStateInfo();
	vdbs.fReports |= GetProfileInt( szApp, SzShared("fReports"), 0);
	Debug(vdbs.fCkStruct ? CkStruct() : 0);
#endif /* DEBUG */

	Debug( CkTlbx (fTrue) ); /* record checksums for toolbox */

#ifdef DEBUG
	if (vwWinVersion < 0x0300)
		{
		WORD FAR * lpw;
		BYTE FAR * lpb;

		lpw = (WORD FAR *) 0xe;
		lpb = MAKELONG(0x100, *lpw);
		/* This code is taken from debug.c to avoid installing the
		int 3 handler when running under the debugger */
		if (*lpb++ != 'S' || *lpb++ != 'E' || *lpb++ != 'G' ||
				*lpb++ != 'D' || *lpb++ != 'E' ||
				*lpb++ != 'B' || *lpb++ != 'U' || *lpb != 'G')
			{
			vfInt3Handler = fTrue;
			InstallInt3Handler();
			}
		}
#endif /* DEBUG */

		{
		extern BOOL fFillBlock;
		extern WORD wFillBlock;
		extern BOOL fShakeHeap, fCheckHeap;
		extern int FAR pascal FFailLmemOpus();
		extern PFN pfnAssertVerify;
		extern int AssertForSdm();
		extern BYTE fCoverActive, fCoverWasActive;

#ifdef DEBUG
	/* set debug options */
		fFillBlock = !vdbs.fNoFillBlock;
		fShakeHeap = vdbs.fShakeHeap;
		fCheckHeap = vdbs.fCkHeap;
		lpfnFailLmemFilter = FFailLmemOpus;
	/* Mathpack for EL */
		fForce8087Fail = vdbs.fFail8087;
		pfnAssertVerify = AssertForSdm;
		fCoverActive = fCoverWasActive && vdbs.fCoverActive;
#endif /* DEBUG */
#ifdef HYBRID
	/* Turn off automatic filling of stack frame variables by DWINTER */
		fFillBlock = GetProfileInt( szApp, SzShared("HybridfFillBlock"), fFalse);
		wFillBlock = GetProfileInt( szApp, SzShared("HybridwFillBlock"), 0xCCCC);
#endif /* HYBRID */
		}

/* parse command line into chunks */
		{
		CHAR *pch, *pchT;
#ifdef DEBUG
		BOOL fReportFailSettings = fFalse;
#endif /* DEBUG */

		CchCopyLpszCchMax( lpszCmdLine, (LPSTR)rgchCmdLine, ichMaxCmdLine );
		rgchCmdLine [ichMaxCmdLine] = 0;
		ipchArgMac = 0;
		pch = rgchCmdLine;

#ifdef PCJ
		CommSz(SzShared("Command line:\r\n"));
		CommSz(rgchCmdLine);
		CommSz(SzShared("\r\n"));
#endif /* PCJ */

/* if msdos is not loaded, we have to skip our name on cmd line */
		if (vfSingleApp)
			{
			while (*pch == ' ')
				pch++;
			while (*pch && *pch != ' ')
				pch++;
			}

		while (fTrue)
			{
			while (*pch == ' ' ||  *pch == '\t')
				pch++;
			if (*pch == '\0')
				break;
			if (ipchArgMac >= ipchArgMax)
				break;
			rgpchArg [ipchArgMac++] = pch;
			while (!(*pch == ' ' ||  *pch == '\t' || *pch == '\0'))
				pch++;
			if (*pch != '\0')
				*pch++ = '\0';
/* check for command line flags(if we find one, don't include in rgpchArg) */
			pchT = rgpchArg [ipchArgMac-1];
			if (*pchT == '-' || *pchT == '/')
				{
				switch (*++pchT)
					{
				default:
				/* other options, leave in */
					ipchArgMac++;
					break;
				case chCmdLineNoFile1:
				case chCmdLineNoFile2:
					*pfOpenUntitled = fFalse;
					break;
				case chCmdLineTutorial1:
				case chCmdLineTutorial2:
					*pfTutorial = fTrue;
					break;
#ifdef PROFILE
				case 'p': 
				case 'P':
					vpfi = 0;
					while (FDigit(*(pchT+1)))
						vpfi = (10*vpfi) + *(++pchT) - '0';
					break;
#endif /* PROFILE */

#ifdef BATCH
				case 'b': 
				case 'B':  /* batch processing */
				/* during a batch process, we don't want asserts coming up */
					vfBatchMode = fTrue;
					CchCopySz(SzShared("\r\n* * * * * * * * *\r\n"), szBatchString);
					SzSzAppend(szBatchString, ++pchT);
					SzSzAppend(szBatchString,SzShared("\r\n* * * * * * * * *\r\n"));
					break;

				case 'i': 
				case 'I':  /* Do not exit upon idle in batch mode */
					vfBatchIdle = fTrue;
					break;
#endif /* BATCH */

#ifdef DEBUG
				case 'f': 
				case 'F':  /* initialization failure switch */
						/*  format:
						/fms<num>  -memory succeed
						/fmf<num>  -memory fail
						/fws<num>  -windows succeed
						/fwf<num>  -windows fail
						*/
						{
						int *pw;
						fReportFailSettings = fTrue;
						switch (*++pchT)
							{
						case 'm': 
						case 'M':
							pw = &vdbs.cLmemSucceed;
							break;
						case 'w': 
						case 'W':
							pw = &vdbs.cWinSucceed;
							break;
						default:
							goto LBogus;
							}
						switch (*++pchT)
							{
						case 's': 
						case 'S':
							break;
						case 'f': 
						case 'F':
							pw++;
							break;
						default:
							goto LBogus;
							}
						*pw = 0;
						while (FDigit(*++pchT))
							*pw = (*pw * 10) + (*pchT - '0');
LBogus:
						break;
						}

				case 'r': 
				case 'R':
					vchRTFTest = 'r';
					break;
				case 'd': 
				case 'D':
					vchRTFTest = 'd';
					break;
				case 'c': 
				case 'C':
					vchRTFTest = 'c';
					break;
#endif
					}
				ipchArgMac--;
				}
			}

		*pipchArgMac = ipchArgMac;

#ifdef DEBUG
		if (fReportFailSettings)
			CommSzRgNum( 
					SzShared("Failure settings (memSuc,memFail,winSuc,winFail): "),
					&vdbs.cLmemSucceed, 4);
#endif /* DEBUG */
		}


#ifdef PROFILE
#ifdef DEBUG
	if (vpfi != -1)
		CommSzNum (SzShared("vpfi="),vpfi);
#endif /* DEBUG */
#endif /* PROFILE */

	Profile( vpfi == pfiInit ? StartProf( 30 ) : 0 );

	GlobalMemInit(); /* expand GDI's and USER's DS */

/* grow our DS bigger */
	if (!fProtectModeWindows || 
		 CbReallocSb(sbDds, cbMaxSbDds-33, HMEM_MOVEABLE) == 0) 
  	 	{
  	 	uns cbMin = CbSizeSb(sbDds);
  	 	uns cbDesired = (uns)(long)CpMin((long)(uns)cbMin+cbExtraDS,0xfff0L);
  	 	while (cbDesired > cbMin &&
  	 		CbReallocSb(sbDds, cbDesired, HMEM_MOVEABLE) == 0)
  	 	cbDesired -= 0x0200;
  	 	}

/* force DS into low memory */
	if (!fProtectModeWindows)
		{
		GlobalUnlock(hInstance);
		Assert (GlobalFlags(hInstance) & GMEM_DISCARDABLE);
		GlobalWire(hInstance);
		}

/* ask for a moderately large swap area */
	SetSwapAreaSize(100*64);

#ifdef PCJ
	CommSzNum(SzShared("vfSingleApp = "), vfSingleApp);
	CommSzNum(SzShared("MS-DOS module handle = "),
			GetModuleHandle(SzShared("MSDOS")));
	CommSzLong(SzShared("cb(sbDds) = "), (long)(uns)CbSizeSb(sbDds));
#endif /* PCJ */

	vfMouseExist = GetSystemMetrics( SM_MOUSEPRESENT );

	/* Load the cursors... */
	if (!FInitHandles())
		{
		ReportSz("Win handle init failure");
		goto InzFailed0;
		}

	/* we can't use LongOp yet so put up the hour glass.  don't put up if no
		mouse since it will go in a random place.  LongOp will put it in the
		right place later. */
	if (vfMouseExist)
		SetCursor(vhcHourGlass);

	/* read internationalizable characters/strings from user profile */
	ReadUserProfile(fFalse);

	/* check if we're running under CBT */
	/* this code needs to be in for CBT development - please leave it
		in, even for the ship version, as they may need to look at
		post-ship bug reports, etc. */

	vhwndCBT = FindWindow( (LPSTR) SzShared(szCbtWinDef), (LPSTR) NULL );

	if (vhwndCBT != (HWND) NULL)
		{
		if (!SendMessage(vhwndCBT, WM_CBTINIT, vhInstance, CBT_OpusID))
			vhwndCBT = NULL;
		}

	/* register our window classes */

	if (!FRegisterWnd ())
		{
		ReportSz("Window register failure");
		goto InzFailed0;
		}

	/* register other windows information */

	if (!FRegisterWinInfo ())
		{
		ReportSz("FRegisterWinInfo failed");
		goto InzFailed0;
		}

	/* Now initialize the pointers to far procedures (thunks). */
	if (!FInitFarprocs())
		{
		ReportSz("Far Proc init failure");
		goto InzFailed0;
		}

	/* Set up the windows hook so that we can intercept key message
		going to dialogs. */
	vlppFWHChain = SetWindowsHook(WH_MSGFILTER, lppFWHKey);

		{
		extern int FAR pascal chkstk();
		GlobalLruOldest(GetCodeHandle((FARPROC)chkstk));
		}


/* Initialize our fundamental WP structures */
/* if failing, still go to InzFailed0 because heap may not have been created */
	if (!FInitStructs())
		{
		ReportSz("FInitStructs failed");
		goto InzFailed0;
		}

	Debug(InitTrackGdiHandles());

/* initialize file system */
	if (!FInitFn())
		{
		ReportSz("FInitFn failed");
		goto InzFailed;
		}

/*  End of "known memory use" period.  From here on out, anything can fail! */
	Assert(!vmerr.fMemFail);
	Debug(vfAllocGuaranteed = fFalse);

/* initialize document management globals - before opus.ini is read */
	SetBytes(&DMQueryD, 0, sizeof(struct DMQD));
	(int)DMFlags = 0;    /* only works as long as DMFlags is 2 bytes */
	(int)DMQFlags = 0;   /* only works as long as DMQFlags is 2 bytes */

	SetWindowColors();

/* Must init vpri handles before FInitGlobalDot */
/* Read current printer, port, etc from WIN.INI */
	SetBytes(&vpri, 0, cbPRI);
	FGetPrinterFromProfile();

/* Read the session preferences file (.ini) */
	if (!FReadUserState())
		{
		ReportSz("read user state failed");
		goto InzFailed;
		}

/* Initialize the autosave descriptor from vpref. (open code) */
		{
		extern ASD         asd;
		/* SetBytes(&asd, 0, sizeof(asd)); IE*/
		asd.mscBase = GetTickCount();
		asd.dmscLastPostpone = dmscPostponeDef;
		}

/* Set fundamental constants relating to the screen */
	if (!FInitScreenConstants())
		{
		ReportSz("Screen constants init failure");
		goto InzFailed;
		}

	if (!FCreatePatternBitmaps())
		{
		ReportSz("Pattern Bitmap allocation failure");
		goto InzFailed;
		}

/* Initialize the command name-function binding table and keymap */
	Profile( vpfi == pfiInitCmds ? StartProf( 30 ) : 0 );
	if (!FInitCommands())
		{
		ReportSz("Command init failure");
		goto InzFailed;
		}
	Profile( vpfi == pfiInitCmds ? StopProf() : 0 );

/* Initialize table keymap.  Need to do it up front even before we
   know we have a table in a document, because AdjustCps could
   cause selCur to jump into a table before the keymap had been set
   up, and the heap is frozen during AdjustCp's so we can't do it
   then. */

	if ((hkmpTable = HkmpCreate(cKmeTable, kmfPlain)) == hNil)
		{
		ReportSz("Table keymap allocation failure");
		goto InzFailed;
		}
	AddKeyToKmp(hkmpTable, kcTab, bcmNextCell);
	AddKeyToKmp(hkmpTable, KcShift(kcTab), bcmPrevCell);
	return fTrue;

InzFailed:
	Profile( vpfi == pfiInit ? StopProf() : 0 );
	SetFlm( flmIdle );

    /* CreateHeap is not done until FInitStructs; avoid SetFlm call until
       heap established. bz
    */
InzFailed0:
	EndLongOp(fFalse/*fAll*/);

	/* note: don't use ErrorEid here, things are not set up enough yet */
	ErrorEidStartup(eidFail);
	return fFalse;
}



/* %%Function:FRegisterWnd %%Owner:PETERJ */
STATIC BOOL NEAR FRegisterWnd()
	{   /* Register all of our window classes. Return TRUE on success,
	FALSE on failure.  Failure is fatal. */

	int iwc;
	WNDCLASS wndclass;
	int idcolor;

	/* Note: rgwc is a csconst structure. Don't try to pass a pointer to it or
		loop using a pointer if a call to another segment will take place.
		Consider it to be like quirky static data.
	*/

	for ( iwc = 0; iwc < iwcMax; iwc++ )
		{
		SetBytes( &wndclass, 0, sizeof (WNDCLASS) );

		if (iwc == iwcApp)
			wndclass.hIcon = vhiWord;

	/* WndProcs now must take care of their own cursors - call
			OurSetCursor with preferred cursor on WM_SETCURSOR */

		wndclass.style = rgwc [iwc].style;
		wndclass.cbWndExtra = rgwc [iwc].cbWndExtra;
		wndclass.hInstance = vhInstance;
		wndclass.lpfnWndProc = rgwc [iwc].lpfnWndProc;
#ifdef WIN23
		/* On a VGA or greater, use a LtGray brush. On EGA use a gray brush */
		/* On a CGA we shouldn't have Win3Visuals set */
		if (vsci.fWin3Visuals && (iwc == iwcIconBar || iwc == iwcStatLine || iwc == iwcRulerMark))
			wndclass.hbrBackground = (vsci.dypScreen >= 480 ? vhbrLtGray : vhbrGray);
		else
#endif /* WIN23 */
		if ((idcolor = rgwc [iwc].idBackgroundColor) != idColorNil)
			wndclass.hbrBackground = (HBRUSH)idcolor;
		wndclass.lpszClassName = (LPSTR)rgwc [iwc].szCls;
		if (!RegisterClass( (LPWNDCLASS) &wndclass ))
			return fFalse;
		}
	return fTrue;
}



/* %%Function:FRegisterWinInfo  %%Owner:PETERJ */
STATIC int NEAR FRegisterWinInfo ()
{
	/* Register other windows information */

	/* Register clipboard formats */
	cfRTF = RegisterClipboardFormat( (LPSTR)SzNear(szClipRTF) );
	cfLink = RegisterClipboardFormat( (LPSTR)SzNear(szClipLink) );

	if (GetProfileInt(szApp, SzShared("AskForPrinterPicture"), 1))
		/* some users might not like this format (esp JohnPa) */
		cfPrPic = RegisterClipboardFormat( (LPSTR)SzNear(szClipPrPic) );

	/* ok if cfPrPic == cfNil */
	return (cfRTF != cfNil && cfLink != cfNil);
}



/* Thunk Initialization Descriptor */
typedef struct _tid
	{
	FARPROC * plpfn;
	FARPROC lpfn;
} TID;

csconst TID rgtid [] =
{
			{ &lpFCheckPopupRect,           FCheckPopupRect },
	
		{ &lppFWHKey,                   FWHKey },
	
		{ &lpprocBlinkCaretSEdit,       BlinkCaretSEdit },
	
		{ &lpprocStaticEditWndProc,     StaticEditWndProc },
	
		{ &lpFontSizeEnum,              FontSizeEnum },
	
		{ &lpFontNameEnum,              FontNameEnum },
	
		{ &lpFPrContinue,               FPrContinue },
	
		{ &lpFMFContinue,               FMFContinue },
#ifdef WIN23
		{ &lpfnIconDlgWndProc, IconDlgWndProc	},
#endif /* WIN23 */
	
	{ &lpFBltOutlineSplat,		FBltOutlineSplat }
	};		


#define itidMax (sizeof (rgtid) / sizeof (TID))


/* F  I N I T  F A R P R O C S */
/* This routine initializes all of the far pointer to procedures. */
/* %%Function:FInitFarprocs %%Owner:PETERJ */
int FInitFarprocs()
{
	int itid;

	for (itid = 0; itid < itidMax; ++itid)
		{
		if ((*rgtid[itid].plpfn =
				MakeProcInstance(rgtid[itid].lpfn, vhInstance)) == NULL)
			{
			return fFalse;
			}
		}

	return fTrue;
}


/* determine how much useable conventional memory to require */
csconst int rgcparaReserve[2] [2] = {
			/*  norm    lrg frm */
/* with emm */{ 30*64,   22*64   },
/* no emm */  { 40*64,   30*64   },
};

/* REVIEW - ship */
int vcsbMaxAvailInit = 0; /* for AppInfo */

/* %%Function:FInitStructs %%Owner:PETERJ */
FInitStructs()
{
	int ww, mw;
	struct MWD *pmwd;
	int sbBPS, sbNext;
	SB sb;
	int isb = 0;
	int ibpExtMaxT;
	int cbCacheInfo;
	int cbHash;
	int cbTimeStamps;
	int csbGrab;
	int csbEmmAvail;
	int csbMaxAvail;
	extern int vcparaReserve;
	extern int vfNoCoreLoad;
#ifdef UNUSED
	extern far PASCAL appcache_seg();
#endif /* UNUSED */
	extern far PASCAL clsplc_q();

	vrf.fExtendedMemory = GetProfileInt( szApp, SzShared("ExtendedMemory"), 1);
#ifndef NO_DIALOG_HACK
	if (vrf.fExtendedMemory)
		CacheCodeSegment( GetCodeHandle( clsplc_q ), 1);
#ifdef UNUSED
	else
		GlobalDiscard( GetCodeHandle(appcache_seg));
#endif /* UNUSED */
#endif
#ifdef UNUSED
	if (!vrf.fExtendedMemory)
		GlobalDiscard( GetCodeHandle(appcache_seg));
#endif /* UNUSED */


/* see how much memory is available & how much we can use */

	if (fProtectModeWindows)
		{
		/* WIN3 pmode - in order to minimize changes to Opus, we try to map
			pmode win3 onto real mode EMM, then proceed to act like we were
			using EMM.  The EMM strategy of taking almost everything isn't
			too good of an idea for pmode (since EMM does not conflict with
			"conventional memory" under real mode but is does in protect 
			mode) so we have a few heuristics to reduce what we take.
		*/
		long (FAR PASCAL *lpfn)();
		Assert(vwWinVersion >= 0x0300);

		lpfn = GetProcAddress(GetModuleHandle(SzFrame("KERNEL")),
				MAKEINTRESOURCE(idoGetFreeSpace));
		Assert(lpfn != NULL);

		/* available global memory (units of 16K) */
		csbMaxAvail = CpMin((*lpfn)(0)/(16*1024L),0x00007fffL);

		/* random heuristic #1: if it says >= 2Mb, then they may be
		   using virtual memory, we want to ask for less.  drop half of
		   everything over 2Mb (128 pages). */
		if (csbMaxAvail >= 128)
			csbMaxAvail -= ((csbMaxAvail - 128) / 2);

		/* random heuristic #2: act as though 1/3 of available memory 
			was real mode EMM memory available to us. */
		csbMaxAvail /= 3;

		/* random heuristic #3: if result is less than 512K (32 pages)
			use only 3/4s to leave a reasonable amount. */
		if (csbMaxAvail < 32)
			csbMaxAvail = (3*csbMaxAvail)/4;
		}
	else
		csbMaxAvail = max(0,CpageAvailStartEmm());
#ifdef DEBUG
	if (vdbs.fNoEmm)
		csbMaxAvail = 0;
#endif /* DEBUG */

	vcsbMaxAvailInit = csbMaxAvail;

	if (csbMaxAvail >= csbEmmMin)
		{
		int cKEmm = GetProfileInt(szApp, SzShared("EmmLimit"), -1);

		if (cKEmm >= 0)
/* take what the user specified */
			csbEmmAvail = min(cKEmm/16, csbMaxAvail);

/* by default take a reasonable amount */
		else
			{
			if (csbMaxAvail < csbEmmReserve)
				csbEmmAvail = csbMaxAvail;
			else  if (csbMaxAvail < 2*csbEmmReserve)
				csbEmmAvail = csbEmmReserve;
			else
				csbEmmAvail = csbMaxAvail - csbEmmReserve;
			}

		csbEmmAvail = min(csbEmmAvail, csbCacheMax+csbStructsMax);
		}
	else
		csbEmmAvail = 0;

/* enable use of EMM, if available. */
	if (fProtectModeWindows)
		{
		cbMemChunk = (16*1024)-32; /* simulate EMM values */
		smtMemory = smtProtect;
		}
	else  if (csbEmmAvail > 0)
		{
		cbMemChunk = CbInitEmm(1, 0, csbMaxAvail-csbEmmAvail);
		smtMemory = smtLIM;
		}
	else
		{
		cbMemChunk = 4*1024;
		smtMemory = smtReal;
		}

#ifdef SHOWEMM
	if (smtMemory == smtLIM)
		CommSzLong(SzShared("initial CbFreeEmm = "), CbFreeEmm());
#endif /* SHOWEMM */

	/* assert: cbMemChunk == 4*1024 or 15.5K < cbMemChunk < 16K */
	Assert(cbMemChunk == 4*1024 || (uns)(cbMemChunk - ((16*1024)-511)) < 510);

#ifdef DEBUG
	if (vfLargeFrameEMS && csbEmmAvail < 16)
		/* kernel is using all our memory!!!! */
		{
		if (csbEmmAvail == 0)
			ReportSz("Kernel took all of EMM!");
		else  if (csbEmmAvail < 8)
			ReportSz("Kernel left less than 128K of EMM!");
		else
			ReportSz("Kernel left less than 256K of EMM!");
		}
#endif /* DEBUG */


/* grab some sbs for our use if needed */
	/* subtract 3 here for cache management sb, and initial heap sb, cmdtbl */
	if (smtMemory == smtLIM && 
			(csbGrab = min((csbEmmAvail/2)-2, csbGrabMax)) > 0)
		{
		for (; isb < csbGrab; isb++)
			{
			sb = SbScanNext(fFalse);
			if (!CbAllocSb(sb, cbMemChunk, hmemLmemHeap))
				{
				ReportSz("Grabbing Memory failed!");
				break;
				}
			vrgsbReserve[isb] = sb;
			}
		}

	for (; isb < csbGrabMax; isb++)
		vrgsbReserve[isb] = sbNil;

/* initialize heap */
	CreateHeap(sbDds);


#ifdef NOTUSED  /* bobz (bz) kill win heap - changed multiline edits to be fedts  */
	if (!FCreateWindowsHeap(sbDds, cbWindowsHeap))
		{
		ReportSz("cant create windows heap");
		return(fFalse);
		}
#endif /* NOTUSED */

/* initialize display line buffer and scc's */
	BuildScc(&vsccAbove, 0, 0);

/* initialize font stuff */
/* rgfce = { {0,0,..}, {0,0,..},... } */
	vfti.fcid.lFcid = vftiDxt.fcid.lFcid = fcidNil;

	/* create an sb for non-cache far heap objects */
	sbNext = SbScanNext (fFalse);
	if (sbNext == sbNil)
		goto LBpFail;
	if (!CbAllocSb( sbNext, cbLmemHeap, hmemLmemHeap))
		goto LBpFail;

	CreateHeap (sbNext);
	Debug (vdbs.fCkHeap ? CheckHeap (sbNext) : 0);

		{
		struct SBHI sbhi;
		extern struct PLSBHI **vhplsbhi;
		FreePpv(sbNext, PpvAllocCb(sbNext, 0)); /* to get handles allocated */
		SetBytes(&sbhi, 0, cbSBHI);
		sbhi.sb = sbNext;
		sbhi.fHasHeap = fTrue;
		sbhi.fHasFixed = fTrue;
		vhplsbhi = HplInit(cbSBHI, 1);
		if (vhplsbhi == hNil || !FInsertInPl(vhplsbhi, 0, &sbhi))
			goto LBpFail;

		Debug (vdbs.fCkHeap ? CheckHeap (sbNext) : 0);
		}

/* initialize bp's */
/* allocate an sb for the mpibpbps, the hash table, and the mpispnts */
	sbBPS = SbScanNext (fFalse);
	Assert (sbBPS != sbNil);
	if (sbBPS == sbNil)
		goto LBpFail;

/* determine size of file cache */
#ifdef DEBUG
	if (vdbs.fSmallFileCache)
		ibpExtMaxT = ibpMaxNoEmm;
	else
#endif /* DEBUG */
		if (csbEmmAvail > 2*csbStructsMax)
			ibpExtMaxT = (csbEmmAvail-csbStructsMax)*cbpChunkWEmm;
		else
			ibpExtMaxT = max(ibpMaxNoEmm, (csbEmmAvail/2)*cbpChunkWEmm);

	if (ibpExtMaxT > csbCacheMax * cbpChunkWEmm)
		/* safety, make sure it can't get too big */
		{
		Assert(fFalse);
		ibpExtMaxT = csbCacheMax * cbpChunkWEmm;
		}

	cbCacheInfo = ibpExtMaxT * cbBPS;
#ifdef DEBUG
	if (csbEmmAvail && !vdbs.fSmallFileCache)
#else
		if (csbEmmAvail)
#endif
			{
			cbCacheInfo += (cbHash=iibpHashMaxEmm*sizeof(int)) /* for hash table */
					+ (cbTimeStamps = 57 * sizeof(int)); /* for span time stamps */
			}
		else
			{
			cbCacheInfo += (cbHash=iibpHashMaxNoEmm*sizeof(int)) /* for hash table*/
					+ (cbTimeStamps = 8 * sizeof(int)); /* for span time stamps */
			}

	Assert (cbCacheInfo < cbMemChunk);

	if (!CbAllocSb( sbBPS, cbCacheInfo, HMEM_MOVEABLE | HMEM_EMM))
		{
LBpFail:
		ReportSz("Page Table init failure");
		return fFalse;
		}

	vbptbExt.ibpMac = ibpExtMaxT;
	vbptbExt.ibpMax = ibpExtMaxT;
	vbptbExt.iibpHashMax = cbHash / sizeof(int);

	/* allocate space for cache hash table and page descriptors */
	vbptbExt.hpmpispnts = HpOfSbIb(sbBPS, 0);
	vbptbExt.hprgibpHash = HpOfSbIb(sbBPS, cbTimeStamps);
	vbptbExt.hpmpibpbps = HpOfSbIb(sbBPS, cbTimeStamps + cbHash);

	vbptbExt.tsMruBps = 0;
	SetBytes(LpOfHp(vbptbExt.hprgibpHash), 0, cbTimeStamps);
	vbptbExt.cbBp = cbSector;
	vbptbExt.cqbpspn = (csbEmmAvail ?
			(ibpExtMaxT > 57 * 31 / 4 ? 31 * 4 : 31) : 
	3 * 4);

		{
		uns cbpRemain = vbptbExt.ibpMax;

		vbptbExt.cbpChunk = (smtMemory != smtReal ? cbpChunkWEmm : cbpChunkNoEmm);
		vbptbExt.hprgbpExt = (CHAR (huge *)[])HpOfSbIb( sbNext = sbBPS + 1, 0 );
		while (cbpRemain > 0)
			{
			uns cbp = umin(vbptbExt.cbpChunk, cbpRemain);

			if (sbNext == sbMac ||
					!CbAllocSb(sbNext++, cbp * cbSector, HMEM_MOVEABLE | HMEM_EMM))
				{
				ReportSz("External cache allocation failure");
				return fFalse;
				}
			cbpRemain -= cbp;
			}
	/* shrink to just what we need */
		FInitSegTable(sbNext+1 /* sbMacReq */);
		ResetSbCur();
		}

	vfNoCoreLoad = !FFromProfile(fTrue, ipstCoreLoad);
	vcparaReserve = fProtectModeWindows ? 5*64 : 
			rgcparaReserve[smtMemory!=smtLIM][vfLargeFrameEMS];

#ifdef SHOWEMM
	CommSzNum(SzShared("csbEmmAvail: "), csbEmmAvail);
	CommSzNum(SzShared("csbGrab: "), csbGrab);
	CommSzNum(SzShared("ibpExtMaxT: "), ibpExtMaxT);
	CommSzNum(SzShared("sbBPS: "), sbBPS);
	CommSzNum(SzShared("cb hpmpibpbps: "), cbCacheInfo - cbHash - cbTimeStamps);
	CommSzNum(SzShared("cb hprgibpHash: "), cbHash);
	CommSzNum(SzShared("cb hpmpispnts: "), cbTimeStamps);
	CommSzNum(SzShared("vbptbExt.cbpChunk: "), vbptbExt.cbpChunk);
	CommSzNum(SzShared("vbptbExt.cqbpspn: "), vbptbExt.cqbpspn);
	CommSzNum(SzShared("vfNoCoreLoad: "), vfNoCoreLoad);
	CommSzNum(SzShared("vcparaReserve: "), vcparaReserve);
#endif /* SHOWEMM */

#ifdef DEBUG
	if (vdbs.fReportHeap)
		ReportHeapSz(SzShared("initial"),0);
#endif /* DEBUG */

/* NOTE: we know none of this will fail, see comment above */

/* initialize memory error state */
	vmerr.hrgwEmerg1 = HAllocateCw(cwEmerg1);
	vmerr.hrgwEmerg2 = HAllocateCw(cwEmerg2);
	vmerr.hrgwEmerg3 = HAllocateCw(cwEmerg3);
	Assert(vmerr.hrgwEmerg1);
	Assert(vmerr.hrgwEmerg2);
	Assert(vmerr.hrgwEmerg3);
/*	vmerr.fSentCBTMemErr = fFalse; IE - don't clear in TopOfMainLoop! */

/* initialize wwTemp */
	AssertDo(IAllocCls(clsWWD, cwWWD) == wwTemp);
	AssertDo(IAllocCls(clsMWD, cwMWD) == mwTemp);
	PwwdWw(wwTemp)->mw = mwTemp;

/* initialize wwPreview */
	AssertDo(IAllocCls(clsWWD, cwWWD) == wwPreview);
	AssertDo(IAllocCls(clsMWD, cwMWD) == mwPreview);
	PwwdWw(wwPreview)->mw = mwPreview;
	pmwd = PmwdMw(mwPreview);
	pmwd->fActive = fFalse;
	pmwd->wwUpper = pmwd->wwActive = wwPreview;

/* initialize wwLayout */
	AssertDo(IAllocCls(clsWWD, cwWWD) == wwLayout);
	AssertDo(IAllocCls(clsMWD, cwMWD) == mwLayout);
	PwwdWw(wwLayout)->mw = mwLayout;

/* Initialize wwSccAbove */
	InitStructScc2();

/* initialize undo and again */
	vuab.uac = uacNil;
	vuab.bcm = bcmNil;
	vaab.bcm = bcmNil;

/* initialize format */
	InvalFli();
	vbchrMax = bchrMaxInit;
	vhgrpchr = HAllocateCw(CwFromCch(vbchrMax + cbCHRE));

/* initialize print set-up */
	SetWords(&vprsu, 0, cwPRSU);
	vprsu.fShowResults = TRUE;
	vpvs.fLoadPrvwFon = fTrue;

/* initialize layout */
	vhplcfrl = HplcInit(sizeof(struct FRL), 1, cp0, fTrue /* ext rgFoo */);
	vfls.hplclnh = HplcInit(sizeof(struct LNH), 1, cp0, fTrue /* ext rgFoo */);

/* initialize hash table */
	SetWords(LpOfHp(&vbptbExt.hprgibpHash[0]), ibpNil, vbptbExt.iibpHashMax);

/* initialize internal page state descriptors */
	InitHpbps(vbptbExt.hpmpibpbps, vbptbExt.ibpMax);

/* initialize undo (open code interesting parts of SetUndoNil) */
	vuab.bcm = bcmNil;
	vuab.uac = uacNil;
	vaab.bcm = bcmNil;
	vaab.doc = docNil;

/* initialize selCur stuff for ruler and ribbon */
	selCur.fUpdateChp = fTrue;
	selCur.fUpdateChpGray = fTrue;
	selCur.fUpdatePap = fTrue;
	selCur.fUpdateStatLine = fTrue;

/* initialize fonts */
		/* Set up master font table with ftcMinUser entries
			for the default fonts */

		{
		CHAR rgbFfn [cbFfnLast];
#define pffn	((struct FFN *) rgbFfn)

		vhsttbFont = HsttbInit(30, fFalse/*fExt*/);
		Assert( vhsttbFont != NULL );

	/*  Tms Rmn */
		pffn->cbFfnM1 = CbFfnFromCchSzFfn(
				CchCopySz( SzSharedKey("Tms Rmn",TmsRmn),
				((struct FFN *)rgbFfn)->szFfn ) /* +1 */) /* -1 */;
		pffn->ffid = FF_ROMAN;
		pffn->prq = VARIABLE_PITCH;
		ChsPffn(pffn) = ANSI_CHARSET;
		AssertDo( IbstAddStToSttb( vhsttbFont, rgbFfn ) == ibstFontDefault );

	/*  Symbol */
		pffn->cbFfnM1 = CbFfnFromCchSzFfn(
				CchCopySz(SzSharedKey("Symbol",Symbol), 
				((struct FFN *) rgbFfn)->szFfn) /* + 1 */) /* -1 */;
		pffn->ffid = FF_DECORATIVE;
		Assert( DEFAULT_PITCH == 0 );
	/* 	pffn->prq = DEFAULT_PITCH;	IE */
		ChsPffn(pffn) = ANSI_CHARSET;	/* weirdly,this is correct for
			Postscript; other printers 
			will have to tell us what they do
			(enumeration will override these
			settings if necessary) */
		AssertDo(IbstAddStToSttb(vhsttbFont, rgbFfn) == ibstFontDefault + 1);

	/*  Helv */
		pffn->cbFfnM1 = CbFfnFromCchSzFfn(
				CchCopySz( SzSharedKey("Helv",Helv),
				((struct FFN *)rgbFfn)->szFfn )/* +1 */) /* -1 */;
		pffn->ffid = FF_SWISS;
		pffn->prq = VARIABLE_PITCH;
		ChsPffn(pffn) = ANSI_CHARSET;
		AssertDo(IbstAddStToSttb(vhsttbFont, rgbFfn) == ibstFontDefault + 2);

	/*  Courier */
		pffn->cbFfnM1 = CbFfnFromCchSzFfn(
				CchCopySz( SzSharedKey("Courier",Courier),
				((struct FFN *)rgbFfn)->szFfn )/* +1 */) /* -1 */;
		pffn->ffid = FF_MODERN;
		pffn->prq = FIXED_PITCH;
		ChsPffn(pffn) = ANSI_CHARSET;
		Assert( ibstCourier == ibstFontDefault + 3 );
		AssertDo(IbstAddStToSttb(vhsttbFont, rgbFfn) == ibstFontDefault + 3);


#undef pffn
		}

/* initialize window cache */
	vhsttbWnd = HsttbInit(5, fFalse/*fExt*/);
	Assert(vhsttbWnd != hNil);

	return fTrue;
}


/* %%Function:InitHpbps %%Owner:PETERJ */
InitHpbps(hpbps, ibpMax)
struct BPS HUGE *hpbps;
int ibpMax;
{
	struct BPS far *lpbps;

#ifdef SLOW
	struct BPS HUGE *hpbspMac = hpbps + ibpMax;

	for (; hpbps < hpbpsMac; hpbps++)
		{
		hpbps->fn = fnNil;
		hpbps->fDirty = fFalse;
		hpbps->ts = 0;
		hpbps->ibpHashNext = ibpNil;
		}
#endif /* SLOW */
	Assert(fnNil == 0);
	SetWords((lpbps = LpOfHp(hpbps)), 0,
			ibpMax * (sizeof(struct BPS)/sizeof(int)));
		{{  /* NATIVE for small loop in InitHpbps to init ibpHashNext's */
		for (; ibpMax > 0; lpbps++, ibpMax--)
			lpbps->ibpHashNext = ibpNil;
		}}
}

#ifdef DEBUG
BOOL vfNovellNetInited = fFalse;
#endif /* DEBUG */

/* F  I N I T  F N */
/* %%Function:FInitFn %%Owner:PETERJ */
FInitFn()
{
	struct PLCBTE **hplcbteChp, **hplcbtePap;
	struct FCB *pfcb;
	CHAR st [ichMaxFile];

	vmerr.fNovellNet = FFromProfile(fFalse, ipstNovellNet);
	vfAs400 = FFromProfile(fFalse, ipstAs400);
	Debug(vfNovellNetInited = fTrue);

	UpdateStDOSPath();              /* Store away current path */

	AssertDo(IAllocCls(clsFCB,cwFCB) == fnScratch);
	hplcbteChp = HplcInit(cbBTE, 0, (CP) 0, fTrue /* ext rgFoo */);
	hplcbtePap = HplcInit(cbBTE, 0, (CP) 0, fTrue /* ext rgFoo */);
	/* See comment re init mem use above */
	Assert(hplcbteChp != hNil && hplcbtePap != hNil);

	/* tells the file system that we have not yet tried to create
		scratch file */
	vmerr.fPostponeScratch = fTrue;
	vfScratchFile = fTrue;  /* we can pretend to write stuff to it */
	vmerr.fScratchFileInit = fTrue; /* ditto */

	pfcb = PfcbFn(fnScratch);
	SetBytes( pfcb, 0, sizeof( struct FCB ));
	pfcb->hplcbteChp = hplcbteChp;
	pfcb->hplcbtePap = hplcbtePap;
	pfcb->osfn = osfnNil;
	pfcb->fTemp = fTrue;
	pfcb->nFib = nFibCurrent;
	/* this is a lie which allows FetchCp and CachePara to know that
		the scratch file does have PLCBTEs that can be used to find
		character and paragraph formatting. This works because we
		create the FCB ourselves instead of using FnOpenSt and because
		we will not be issuing a SAVE command for the scratch file. */
	pfcb->fHasFib = fTrue;
/* length avoids heap movement when scratch file gets actually created */
	st [0] = ichMaxFile;
	FChngSizeHCw( mpfnhfcb[fnScratch], cwFCB + CwFromCch(st[0] - 1), fTrue );
	bltbyte( st, PfcbFn(fnScratch)->stFile, st[0] + 1);
	Assert(!vmerr.fMemFail);/* See comment re init mem use above */

/* the scratch file will start with cbMac = 0. The fkpd's will be set
up so that any insertion will start a new page */
	vfkpdPap.bFreeFirst = 1;
	/* .bFreeLim = 0; .fcFirst = 0 */
	vfkpdChp.bFreeFirst = 1;
	/* .bFreeLim = 0; .fcFirst = 0 */
	vfkpdText.pn = pnNil;

	StandardChp(&vfkpdChp.chp);
	return fTrue;
}


#include "opuscmd2.h"  /* contains sizes for initial tables (from mkcmd) */


/* F  I N I T  C O M M A N D S */
/* Create the command table and base keymap and initialize with the
prebuilt tables generated from opuscmd.cmd */
/* %%Function:FInitCommands %%Owner:PETERJ */
FInitCommands()
{
	extern HPSYT vhpsyt;
	extern KMP ** hkmpBase;
	extern KMP ** hkmpCur;
	extern KMP ** vhkmpUser;
	extern MUD ** vhmudBase;
	extern MUD ** vhmudUser;
	extern struct STTB ** hsttbMenu;

	SYT FAR * lpsyt;
	int FAR *lprgbst;
	HQ hq;
	int imnu, imtm, imtmMac;
	MTM * pmtm;
	MUD * pmud;
	HMENU hmenu;
	CHAR rgch [cchMaxSt];


		{
		extern int vkPlus, vkMinus,  vkStar;
#ifdef INTL
		extern int vkUnderline, vkEquals, vkQuestionmark;
#endif

		extern int vwWinVersion;

		FARPROC lpfnVkKeyScan;
		HANDLE hKeyboard;

	/* Get real scan codes for +, -, and * keys (not on keypad) from
		the keyboard driver if it supports VkKeyScan, use the
		suggestions from the Windows manual otherwise. */
		hKeyboard = GetModuleHandle((LPSTR) SzShared("KEYBOARD"));
		Assert(hKeyboard != NULL);
		lpfnVkKeyScan = vwWinVersion >= 0x0210 ? GetProcAddress(hKeyboard, 
				MAKEINTRESOURCE(idoVkKeyScan)) : NULL;

#ifndef INTL
		if (lpfnVkKeyScan != NULL)
			{
			vkPlus = (*lpfnVkKeyScan)('+') & 0xff;
			vkMinus = (*lpfnVkKeyScan)('-') & 0xff;
			vkStar = (*lpfnVkKeyScan)('*') & 0xff;
			}
		else
			{
		/* Use values suggested in Windows manual */
			vkPlus = 0xbb;
			vkMinus = 0xbd;
			vkStar = '8';
			}
#else /* INTL */
		if (lpfnVkKeyScan != NULL)
			{
			vkPlus = (*lpfnVkKeyScan)('+');
			vkPlus = ((vkPlus & 0x100) ? (KcShift(vkPlus & 0xff)) : (vkPlus & 0xff));
			vkMinus = (*lpfnVkKeyScan)('-');
			vkMinus = ((vkMinus & 0x100) ? (KcShift(vkMinus & 0xff)) : (vkMinus & 0xff));
			vkStar = (*lpfnVkKeyScan)('*');
			vkStar = ((vkStar & 0x100) ? (KcShift(vkStar & 0xff)) : (vkStar & 0xff));
			vkUnderline = (*lpfnVkKeyScan)('_');
			vkUnderline = ((vkUnderline & 0x100) ? (KcShift(vkUnderline & 0xff)) : (vkUnderline & 0xff));
			vkEquals = (*lpfnVkKeyScan)('=');
			vkEquals = ((vkEquals & 0x100) ? (KcShift(vkEquals & 0xff)) : (vkEquals & 0xff));
			vkQuestionmark = (*lpfnVkKeyScan)('?');
			vkQuestionmark = ((vkQuestionmark & 0x100) ? (KcShift(vkQuestionmark & 0xff)) : (vkQuestionmark & 0xff));
			}
		else
			{
			vkPlus = vkPlusDef;
			vkMinus = vkMinusDef;
			vkStar = vkStarDef;
			vkUnderline = vkUnderlineDef;
			vkEquals = vkEqualsDef;
			vkQuestionmark = vkQuestionmarkDef;
			}
#endif /* INTL */
		}

	/* Allocate menu string table */
	Assert(cwsttbMenuBase == cwSTTBBase);
	if ((hsttbMenu = HAllocateCb(cbSTTB)) == 0)
		return fFalse;

	Assert(offset(STTB, hqrgbst) == (cwsttbMenuBase*sizeof(int)));
	if ((hq = (*hsttbMenu)->hqrgbst = HqAllocLcb((long)cbsttbMenuRgbst))
			== hqNil)
		return fFalse;

	/* Allocate menu base */
	if ((vhmudBase = HAllocateCw(cwmudBase)) == 0)
		return fFalse;

	/* Allocate keymap */
	/* open code HkmpNew */
	/* don't have to initialize *hkmp, will be done in MoveCmds */
	if ((hkmpBase = HAllocateCw(cwKMP + ckmeInit * cwKME)) == hNil)
		return fFalse;

	Assert(hkmpCur == hNil);
	hkmpCur = hkmpBase;

	/* Allocate command (symbol) table */
	if ((vhpsyt = HpOfSbIb(SbAllocEmmCb(cbsytInit), 0)) == hpNil)
		return fFalse;

	lpsyt = LpLockHp(vhpsyt);
	lprgbst = LpLockHq(hq);

	if (lpsyt == NULL || lprgbst == NULL)
		{
		ReportSz("cannot lock handles");
		return fFalse;
		}

	/* Fill command table and keymap with initial functions */
	MoveCmds(lpsyt, (KMP FAR *) *hkmpBase, (LPSTR) *vhmudBase,
			(LPSTR) *hsttbMenu, lprgbst);

	UnlockHp(vhpsyt);
	UnlockHq(hq);

	Assert((*hsttbMenu)->fExternal && (*hsttbMenu)->hqrgbst == hq);

	(*hkmpBase)->grpf = kmfTranslate | kmfBeep;

	/* Create Standard Menu Bar */
	if ((vhMenuLongFull = CreateMenu()) == NULL)
		return fFalse;

	pmud = *vhmudBase;
	imnu = -1;
	imtmMac = pmud->imtmMac;
	for (pmtm = &pmud->rgmtm[0], imtm = 0; imtm < imtmMac; ++imtm, ++pmtm)
		{
		extern int vbsySepCur;

		if (imnu != pmtm->imnu)
			{
			CHAR * sz;

			/* add menu to menubar */
			imnu = pmtm->imnu;
			hmenu = CreateMenu();
			GetSzFromSttb(hsttbMenu, imnu, rgch);
			sz = &rgch[0];
			if (!ChangeMenu(vhMenuLongFull, 0, (LPSTR) sz, hmenu,
					MF_POPUP | MF_APPEND))
				return fFalse;
			}
		/* FUTURE: mkcmd could do this for us */
		else  if ((int) pmtm->bsy == bcmSeparator)
			pmtm->bsy = vbsySepCur++;
		}

	return fTrue;
}


/* array of actual y-size of ribbon bitmaps for each group  */
/* must be sorted in decreasing order */
#ifdef WIN23
/****
This set is used for Win 3 visuals under Win 3
****/
csconst int mpdbmgdypLH3 [] = { 
	26, 20, 12 };
/****
This set is used for Win 2 visuals under Win 3. We need a seperate
set because the value used to search the list is calc'd using the
Win 3 sysfont, but we want an index as if it was a Win 2 font. We can't
use the Win2 font size, because the index we find here is used to set
the fake font sizes below (circular dependency). 
****/
csconst int mpdbmgdypLH23 [] = { 
	26, 21, 20, 12, 8 };
/****
This is the set used for Win 2 visuals under Win 2. The same as in 1.0
****/
csconst int mpdbmgdypLH2 [] = { 
	26, 20, 19, 11, 8 };
#else
csconst int mpdbmgdypLH [] = { 
	26, 20, 19, 11, 8 };
#endif /* WIN23 */


/* D I S P L A Y - D E P E N D E N T   V A L U E S */

/* Table of twip constants that get converted to screen pixel values in vsci */
/* This table must match the corresponding one in screen.h */
/* It's called rgdxa but has a mix of xa, dxa, ya, dya */

/* Note this is a csconst structure and must be treated specially */

csconst int rgdxaConvert [] = {  
	dxaMinScroll, xaSelBar,
			xaLeftMinRaw, xaRightMaxRaw,  dxaGrayOutside,
			dyaMinAveLine, dyaMacAveLine, dyaMinWwInit,
			yaSubSuper,
			dyaWwMin, dyaPastPage, dyaPageReveal,
			dyaGrayOutside };


#define cchPatLast   10     /* largest allowable repeat interval in pattern */

#define cPatVert    1
/* note we have now 8 pixels of gray, 8 of black */
csconst CHAR rgchPatVert[] = { 
	8, 0x00, 0x00, 0xFF, 0x00,   /* Gray, Black */
	0x00, 0x00, 0xFF, 0x00
};


#define cPatHorz    3
csconst CHAR rgchPatHorz[] = { 
	1, 0x55,                  /* Gray */
	3, 0x6d, 0xB6, 0xdb,      /* Light Gray */
	1, 0x00                   /* Black */
};

#ifdef WIN23
#define cbPatDither 16
csconst CHAR rgchPatDither[] = {
	0xAA,	0xAA,		/* 10101010 */	/* each scan line is rounded to a word */
	0x55,	0x55,		/* 01010101 */
	0xAA,	0xAA,		/* 10101010 */
	0x55,	0x55,		/* 01010101 */
	0xAA,	0xAA,		/* 10101010 */
	0x55,	0x55,		/* 01010101 */
	0xAA,	0xAA,		/* 10101010 */
	0x55,	0x55		/* 01010101 */
};


#endif /* WIN23 */
/* F  C r e a t e  P a t t e r n  B i t m a p s */
/* Create monochrome bitmaps to be used as an alternative to color
	pattern brushes for drawing vertical and horizontal lines.

	Input: data tables above

	Output: vhbmPatVert - a tall, thin bitmap containing vertical line
							patterns, each 2 pixels wide
			vhbmPatHorz - a short, wide bitmap containing horizontal
							line patterns, each vsci.dypBorder high
*/

/* %%Function:FCreatePatternBitmaps %%Owner:bryanl */
FCreatePatternBitmaps()
{
	extern HBITMAP vhbmPatVert, vhbmPatHorz;

	CHAR rgchPat [cchPatLast];
	int c;
	CHAR *pch, *pb;
	int ichPat, ichPatNext;
	CHAR **hrgbBits;
	int f = fFalse;
	int cbHorz, cbVert, cbAlloc;

	Assert( dxpHbmHorz % 16 == 0 && dxpHbmHorz % 3 == 0 && dypHbmVert % 8 == 0 );

	cbHorz = (dxpHbmHorz / 8) * vsci.dypBorder;
	/* 8 byte repeating pattern handled differently */
	/*   cbVert = 8 * (dypHbmVert / 8) * (2);  below equivalent to this bz */
	cbVert = (dypHbmVert) * (2);
	cbAlloc = max( cPatHorz * cbHorz, cPatVert * cbVert);
#ifdef WIN23
	cbAlloc = max( cbAlloc, cbPatDither);
#endif /* WIN23 */
	if ((hrgbBits = HAllocateCw( CwFromCch( cbAlloc ))) == hNil)
		return fFalse;

/* Build bits and create bitmap for horizontal patterns */

	for ( c = cPatHorz, pb = *hrgbBits, ichPat = 0; c--; )
		{
		int cchPat = rgchPatHorz [ichPat++];

		Assert( cchPat <= cchPatLast );
		ichPatNext = ichPat + cchPat;
		for ( pch = rgchPat; ichPat < ichPatNext; ichPat++ )
			*pch++ = rgchPatHorz [ichPat];

		FillBytePattern( pb, rgchPat, cchPat, cbHorz );
		pb += cbHorz;
		}

	if ((vhbmPatHorz = CreateBitmap( dxpHbmHorz, cPatHorz * vsci.dypBorder,
			1, 1, (LPSTR)*hrgbBits)) == NULL)
		{
		goto LRet;
		}
	LogGdiHandle(vhbmPatHorz, 1022);

/* Build bits and create bitmap for vertical patterns */

	for ( c = cPatVert, pb = *hrgbBits, ichPat = 0; c--; )
		{
		int cchPat = rgchPatVert [ichPat++];

		Assert( cchPat <= cchPatLast );
		ichPatNext = ichPat + cchPat;
		for ( pch = rgchPat; ichPat < ichPatNext; ichPat++ )
			*pch++ = rgchPatVert [ichPat];

		FillBytePattern( pb, rgchPat, cchPat, cbVert );
		pb += cbVert;
		}

#ifdef DBLDBMP
		{
		int cb = 8 * 2 * dypHbmVert / 8;

		CommSzRgNum(SzShared("PatVert before build bitmap: "),
				*hrgbBits, cb / 2);

		}
#endif

	if ((vhbmPatVert = CreateBitmap( 16, dypHbmVert,
			1, 1, (LPSTR)*hrgbBits)) == NULL)
		{
		UnlogGdiHandle(vhbmPatHorz, 1022);
		DeleteObject( vhbmPatHorz );
		goto LRet;
		}
	LogGdiHandle(vhbmPatVert, 1021);

#ifdef DBLDBMP
		{
		long lcb = 2 * dypHbmVert ;
		CHAR rgchx[2 * dypHbmVert];
		BITMAP bm;

		GetObject( vhbmPatVert, sizeof( BITMAP ), (LPSTR)&bm );
		CommSzRgNum(SzShared("PatVert struct bitmap: "), &bm,
				sizeof(BITMAP)/2);
		GetBitmapBits( vhbmPatVert, lcb,
				(LPSTR)rgchx );
		CommSzNum(SzShared("PatVert lcb: "), (int)lcb);
		CommSzRgNum(SzShared("PatVert bitmap: "),
				rgchx, (int)lcb >> 1);
		}
#endif
#ifdef WIN23
	if (vsci.fWin3Visuals)
		{
		HBITMAP hbmTmp;
		bltbx(rgchPatDither, (LPSTR)*hrgbBits, cbPatDither);
		if (
			((hbmTmp = CreateBitmap( 8, 8, 1, 1, (LPSTR)*hrgbBits)) == NULL) ||
			((vsci.hbrLitButton = CreatePatternBrush(hbmTmp)) == NULL)
			)
			{
			vsci.hbrLitButton = GetStockObject(WHITE_BRUSH);
			}
		if (hbmTmp)
			DeleteObject(hbmTmp);
		}
#endif /* WIN23 */

	f = fTrue;

LRet:
	Debug( vdbs.fCkHeap ? CkHeap() : 0 ); /* just until I'm sure this works */
	FreeH( hrgbBits );
	return f;
}


typedef struct
{
	HANDLE *ph;
	int id;
} LRD;

csconst LRD rglrdStockObj[] =
{
			{ &vhbrLtGray,      LTGRAY_BRUSH    },
	
		{ &vhbrGray,        GRAY_BRUSH      },
	
		{ &vhbrWhite,       WHITE_BRUSH     },
	
		{ &vhbrBlack,       BLACK_BRUSH     },
	
		{ &vhbrDkGray,      DKGRAY_BRUSH    },

};


#define ilrdStockObjMax (sizeof(rglrdStockObj)/sizeof(LRD))

csconst LRD rglrdSysCur[] =
{
			{ &vhcHourGlass,        IDC_WAIT    },
	
		{ &vhcIBeam,            IDC_IBEAM   },
	
		{ &vhcArrow,            IDC_ARROW   },
	
		{ &vhcArrow,            IDC_ARROW   },

};


#define ilrdSysCurMax (sizeof(rglrdSysCur)/sizeof(LRD))

csconst LRD rglrdRcdsCur[] =
{
			{ &vhcSplit,        ircdsSplit     },
	
		{ &vhcColumn,		ircdsColumn    },
	
		{ &vhcBarCur,       ircdsRight     },

};


#define ilrdRcdsCurMax (sizeof(rglrdRcdsCur)/sizeof(LRD))

csconst LRD rglrdRcdsIco[] =
{
			{ &vhiWord,             ircdsWordIcon  },

};


#define ilrdRcdsIcoMax (sizeof(rglrdRcdsIco)/sizeof(LRD))



/*  F  I N I T  H A N D L E S */
/* %%Function:FInitHandles %%Owner:PETERJ */
FInitHandles()
{
	int ilrd;
	BOOL fRes;

	for (ilrd = 0; ilrd < ilrdStockObjMax; ilrd++)
		if ((*rglrdStockObj[ilrd].ph = GetStockObject(rglrdStockObj[ilrd].id))
				== NULL)
			return fFalse;

	for (ilrd = 0; ilrd < ilrdSysCurMax; ilrd++)
		if ((*rglrdSysCur[ilrd].ph = LoadCursor(NULL, rglrdSysCur[ilrd].id))
				== NULL)
			return fFalse;

	/* we have 32X32 version, otherwise have to load from resource */
	fRes = (GetSystemMetrics(SM_CXCURSOR) != 32 ||
			GetSystemMetrics(SM_CYCURSOR) != 32);
	vsci.fUseResCursors = fRes;
	for (ilrd = 0; ilrd < ilrdRcdsCurMax; ilrd++)
		if ((*rglrdRcdsCur[ilrd].ph = fRes ?
				LoadCursor(vhInstance, rglrdRcdsCur[ilrd].id+1) :
				HLoadRes0(rglrdRcdsCur[ilrd].id))
				== NULL)
			return fFalse;

	/* we have 32X32 version, otherwise have to load from resource */
	/* ICON internal format changed for Win3.0, must load from resource */
	fRes = (GetSystemMetrics(SM_CXICON) != 32 ||
			GetSystemMetrics(SM_CYICON) != 32 || 
			vwWinVersion > 0x0299);
	for (ilrd = 0; ilrd < ilrdRcdsIcoMax; ilrd++)
		if ((*rglrdRcdsIco[ilrd].ph = fRes ?
				LoadIcon(vhInstance, rglrdRcdsIco[ilrd].id+1) :
				HLoadRes0(rglrdRcdsIco[ilrd].id))
				== NULL)
			return fFalse;

	return fTrue;
}

#ifdef WIN23
csconst int mpdbmgPmTmHeight[] = {20, 16, 12};
csconst int mpdbmgPmTmWidth[] = {8, 6, 6};
	/* REVIEW: I don't know the correct values for the Win 2 system font
		on VGA, 8514 and SIGMA */
	/* note that this is height (ascent+descent),
		not including external leading */
csconst int mpdbmgWin2TmHeight[] = {20, 15, 15, 10, 8};
csconst int mpdbmgWin2TmWidth[] = {10, 8, 8, 8, 8};
#endif /* WIN23 */

/* %%Function:FInitScreenConstants %%Owner:PETERJ */
int FInitScreenConstants()
{
	/* This routine sets the value of a variety of global variables that used to
	be constants in Mac Word, but are now varibles because screen resolution can
	only be determined at run time. */

	extern struct BMI           vbmiEmpty;
#ifdef WIN23
	extern struct BMI           *mpidrbbmi;
	extern struct BMI           mpidrbbmi2[];
	extern struct BMI           mpidrbbmi3[];
#else
	extern struct BMI           mpidrbbmi[];
#endif /* WIN23 */
	extern int              vdxpStagger;
	extern int              vdypStagger;
	HDC hdc;
	int xaRightMac;
	TEXTMETRIC tm;
	int dbmg;
	int dypLH;
#ifdef WIN23
	LOGFONT lf;
	HFONT hfontOld = NULL;
	HDC hdcTStatLine;
	int dyp ; 
#endif /* WIN23 */
	/* Get the DC of the parent window to use to obtain screen attributes */

	if ((hdc = GetDC(NULL)) == NULL)
		return fFalse;

#ifdef WIN23
	/* need to be sure mpidrbbmi is set up for calls below */
	if (vsci.fWin3 && vsci.fWin3Visuals)
		{
		vsci.dyRuler = dyRuler3;
		vsci.dccStatLine = dccStatLine3;
		vsci.dccIconBar = dccIconBar3;
		vsci.dypOtlPat = dypOtlPat3;
		mpidrbbmi = mpidrbbmi3;	/* set up for Win 3 bitmaps */
		/* find the address of CreateDIBitmap */
		/* magic: 442 is entrypoint for CreateDIBitmap under Win 3 */
		vsci.lpfnCreateDIBitmap = GetProcAddress(GetModuleHandle(SzFrame("GDI")),
			MAKEINTRESOURCE(442));
		Assert(vsci.lpfnCreateDIBitmap);
		}
	else
		{
		vsci.dyRuler = dyRuler2;
		vsci.dccStatLine = dccStatLine2;
		vsci.dccIconBar = dccIconBar2;
		mpidrbbmi = mpidrbbmi2;	/* set up for Win 2 bitmaps */
		vsci.dypOtlPat = dypOtlPat2;
		}
#endif /* WIN23 */
	/* determine screen pixel dimensions */
	vfli.dxsInch = GetDeviceCaps(hdc, LOGPIXELSX);
	vfli.dysInch = GetDeviceCaps(hdc, LOGPIXELSY);
#ifdef BRYANL
		{
		int rgw [2];

		rgw [0] = vfli.dxsInch;
		rgw [1] = vfli.dysInch;
		CommSzRgNum( SzShared("dxsInch, dysInch: "), rgw, 2 );
		}
#endif

		/* Compute screen pixel values that are derived from twip constants */

		{
		int *pdxp;
		int cw;
		int idxa;

		pdxp = (int *) ((CHAR *) &vsci + brgdxpConvert);
		cw = cDxpConvert;

	/* Note: rgdxaConvert is a csconst structure, so we reevaluate its address
		each time rather than setting up a pdxa++ */

		idxa = 0;
		while (cw--)
			{
			*pdxp++ = (int) umin (0x7FFF,
					NMultDiv( rgdxaConvert[idxa], vfli.dxsInch, czaInch ));
			idxa++;
			}
		cw = cDypConvert;
		while (cw--)
			{
			*pdxp++ = (int) umin (0x7FFF,
					NMultDiv( rgdxaConvert[idxa], vfli.dysInch, czaInch ));
			idxa++;
			}
		}

	vsci.xaRightMax = xaRightMaxRaw;
	if (((long)(uns)xaRightMaxRaw * (long)(uns)vfli.dxsInch  >
			(0x7FFFL * (long) czaInch)))
		{
		/* Can't represent xaRightMaxRaw worth of text; truncate */

		vsci.xpRightMax = 0x7FFF;
		vsci.xaRightMax = UMultDiv( 0x7fff,czaInch,vfli.dxsInch);
		}
	Assert( vsci.xpRightMax > 0 && (uns)vsci.xpRightMax < 0x8000 );

	/* round dxwSelBar to the next highest multiple of 8; this lets us use the
		byte-aligned fast case for Windows TextOut */
	vsci.dxwSelBar += 7;
	vsci.dxwSelBar -= vsci.dxwSelBar % 8;

	/* set up vsci.xpLeftMin separately; unlike the others, it is negative */

	vsci.xpLeftMin = -(vsci.xpRightMax>>1);
	vsci.xaLeftMin = NMultDiv( vsci.xpLeftMin, czaInch, vfli.dxsInch );

	vsci.dxpBorder = GetSystemMetrics(SM_CXBORDER);
	vsci.dypBorder = GetSystemMetrics(SM_CYBORDER);
	vsci.dypSplitBar = 3 * vsci.dypBorder;

	vsci.dxpScrlBar   = GetSystemMetrics(SM_CXVSCROLL);
	vsci.dypScrlBar   = GetSystemMetrics(SM_CYHSCROLL);
	vsci.dypScrollArrow = GetSystemMetrics(SM_CYVSCROLL);
	vsci.dypSplitBox  =  vsci.dypScrollArrow / 3;

	GetTextMetrics( hdc, (LPTEXTMETRIC)&tm );
	vsci.dypTmHeight = tm.tmHeight;
	vsci.dxpTmWidth = tm.tmAveCharWidth;
	vsci.dypTmAscent = tm.tmAscent;                         /* HACK !!!!! */
	vsci.dypTmInternalLeading = tm.tmInternalLeading;
#ifndef WIN23
	/* else done below */
	vsci.dypRibbon = NMultDiv( dyRibbon, vsci.dypTmHeight, 8 ) + 1;
#endif /* WIN23 */
#ifdef WIN23
	/* Needs to happen after a scratch DC is careted  */
	if (!vsci.fWin3)
#endif /* WIN23 */
	vsci.dypStatLine = vsci.dypTmHeight + 3 * vsci.dypBorder;

	/* adjust min line height to be no greater than system font, so we get
		single line scrolling in draft view */
	vsci.dysMinAveLine = min( vsci.dysMinAveLine, vsci.dypTmHeight );

/* vsci.dypRuler and vsci.dypIconBar are the dyp that is used for all normal
	window manipulation, assuming that the ruler and iconbar both overlap
	whatever is above them by one border width.  That is why when these two
	are created, we add one border width.  This is just to avoid having to
	have lots of "- vsci.dypBorder"s all over the code to adjust for the
	overlapping borders */

#ifndef WIN23
	/* else it's done below */
	vsci.dypRuler = NMultDiv( dyRuler, vsci.dypTmHeight, 8 );
	vsci.dypIconBar = NMultDiv( dyIconBar, vsci.dypTmHeight, 8 );
#endif /* WIN23 */

	vdxpStagger = vsci.dxpScrlBar >> 2;
	vdypStagger = vsci.dypScrlBar >> 2;

	vsci.xpScrollMax  = vsci.xpRightMax - (GetSystemMetrics( SM_CXFULLSCREEN ) -
			vsci.dxwSelBar - vsci.dxpScrlBar);
	vsci.dxpScreen = GetDeviceCaps(hdc,HORZRES);
#ifdef WIN23
	if (!vsci.fWin3)
		{	/* if fWin3 then these two were done above in FInitPart1 */
		vsci.dypScreen = GetDeviceCaps(hdc,VERTRES);
		vsci.fMonochrome = (GetDeviceCaps(hdc, NUMCOLORS) == 2);
		}
#else
    vsci.dypScreen = GetDeviceCaps(hdc,VERTRES);
	vsci.fMonochrome = (GetDeviceCaps(hdc, NUMCOLORS) == 2);
#endif /* WIN23 */

	vsci.dxmmScreen = GetDeviceCaps(hdc,HORZSIZE);
	vsci.dymmScreen = GetDeviceCaps(hdc,VERTSIZE);

	/* choose among available device bitmap groups */
	/* algorithm is: look for largest bitmap <= correct size */

	dypLH = NMultDiv( dyRibbon-ddyIconLH, vsci.dypTmHeight, 8)
			- (vsci.dypBorder << 2);
#ifdef WIN23
	{
	int dbmgLast;
	int FAR *pmpdbmgdypLH;

	if (vsci.fWin3Visuals)
		{
		dbmgLast = dbmgLast3;
		pmpdbmgdypLH = &mpdbmgdypLH3[0];
		}
	else if (vsci.fWin3)
		{
		dbmgLast = dbmgLast2;
		pmpdbmgdypLH = &mpdbmgdypLH23[0];
		}
	else
		{
		dbmgLast = dbmgLast2;
		pmpdbmgdypLH = &mpdbmgdypLH2[0];
		}
	for (dbmg = 0; dbmg < dbmgLast && dypLH < pmpdbmgdypLH[dbmg]; dbmg++)
		;
	}
#else
	for (dbmg = 0; dbmg < dbmgLast && dypLH < mpdbmgdypLH[dbmg]; dbmg++)
		;
#endif /* WIN23 */
	vdbmgDevice = dbmg;
#ifdef WIN23
	/* set up the fake font height and width based on the screen device.
		If we are running Win3 Visuals, this is the PM font height and
		width for the screen resolution. We'll use this this for iconbars
		in the 3D mode, so that all the scaling works as is does under PM.
		If we are running without 3D visuals, this is the Win 2 sys font
		height and width. These will be used to size the iconbars in 2D
		mode, so all the old templates still work.
	*/
	if (vsci.fWin3)
		{
		if (vsci.fWin3Visuals)
			{
			vsci.dypFakeTmHeight = mpdbmgPmTmHeight[dbmg];
			vsci.dxpFakeTmWidth = mpdbmgPmTmWidth[dbmg];
			}
		else
			{
			vsci.dypFakeTmHeight = mpdbmgWin2TmHeight[dbmg];
			vsci.dxpFakeTmWidth = mpdbmgWin2TmWidth[dbmg];
			}
		}
	/*****
	If under win3 and not under fWin3Visuals then we want to use
	the faketmheight (win2 sys font) to size the ruler and ribbon
	*****/
#ifdef REVIEW
	dyp = (vsci.fWin3 && !vsci.fWin3Visuals) ? vsci.dypFakeTmHeight :
		vsci.dypTmHeight ;
#else
	dyp = (vsci.fWin3) ? vsci.dypFakeTmHeight :	vsci.dypTmHeight ;
#endif /* REVIEW */
	vsci.dypRuler = NMultDiv( vsci.dyRuler, dyp, 8 );
	vsci.dypIconBar = NMultDiv( dyIconBar, dyp, 8 );
	vsci.dypRibbon = NMultDiv( dyRibbon, dyp, 8 ) + 1;
#endif /* WIN23 */
#ifdef DCSRC
	CommSzNum(SzShared("vdbmgDevice = "), vdbmgDevice);
#endif /* DCSRC */

/* create two memory DCs.  We need two to create grey resource bitmaps; also
	to paint visi characters into the screen cache. */

	if (!FCreatePmdcd(&vsci.mdcdScratch,hdc))
		return fFalse;
	LogGdiHandle(vsci.mdcdScratch.hdc, 1073);
	if (!FCreatePmdcd(&vsci.mdcdBmp,hdc))
		return fFalse;
	LogGdiHandle(vsci.mdcdBmp.hdc, 1074);
#ifdef WIN23
	if (vsci.fWin3)
		{
		/*
	 	* Create the status line font.  Use tm; it is unused at this point. 
	 	* 200 twips == 10 pts.  Get the widths of a digit for later
	 	* Add an extra pixel to height of status line for a shadow line and
	 	* for a blank line at the bottom.
	 	*/
 		SetWords( &lf, NULL, sizeof(LOGFONT)/sizeof(int) );
 		lf.lfHeight = NMultDiv( 240, vfli.dysInch, czaInch );  
 		lf.lfCharSet = ANSI_CHARSET;
 		lf.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;

		if (hfontStatLine = CreateFontIndirect( &lf )) {
			hfontOld = SelectObject( (hdcTStatLine = vsci.mdcdScratch.hdc),
								 	hfontStatLine );
			GetTextMetrics( hdcTStatLine, (LPTEXTMETRIC)&tm );
			GetCharWidth( hdcTStatLine, '8', '8', (LPINT) &vdxpDigit );

			vsci.dypStatLine = tm.tmHeight + 4 + 3 * vsci.dypBorder;
			if (hfontOld)
				SelectObject( hdcTStatLine, hfontOld );
		}
		else {
			vsci.dypStatLine = vsci.dypTmHeight + 1 + 3 * vsci.dypBorder;
		}
	}

#endif /* WIN23 */
/* Preload visichars bitmap so we don't load it later in the middle
	of the critical part of screen cache fill. */

	vsci.mdcdScratch.pbmi = &mpidrbbmi [idrbChVis];
	if ((vsci.mdcdScratch.pbmi->hbm = HFromIbmds0(idrbChVis))
			== NULL)
		return fFalse;

		{
		extern int far pascal rcinit_q();
		GlobalLruOldest(GetCodeHandle((FARPROC)rcinit_q));
		}

	/* Discover the handle to the NULL bitmap. This is an undocumented Windows
		object, analogous to SYSTEM_FONT.  It is the bitmap selected into
		a newly created DC.  The way to get it is to look at the return
		value from a SelectObject call for a newly created DC */

	if ((vbmiEmpty.hbm = SelectObject( vsci.mdcdScratch.hdc, 
			vsci.mdcdScratch.pbmi->hbm)) == NULL)
		return fFalse;

	/* We don't need the DC any more. */
	ReleaseDC( NULL, hdc);

	vsci.dypMenuBar = GetSystemMetrics(SM_CYMENU);
	vsci.dypBdrCapt = GetSystemMetrics(SM_CYFRAME) + 
			GetSystemMetrics(SM_CYCAPTION);
	vsci.dxpBmpSystem = 0; /* this is filled in later */

	return fTrue;
}



/* %%Function:FCreatePmdcd %%Owner:PETERJ */
FCreatePmdcd(pmdcd, hdcCompat)
struct MDCD *pmdcd;  
HDC hdcCompat;
{
	extern struct BMI vbmiEmpty;

	if ((pmdcd->hdc = CreateCompatibleDC(hdcCompat)) == NULL)
		return fFalse;

	if (!FSetDcAttribs( pmdcd->hdc, dccScratch & dccmColors ))
		return fFalse;
	pmdcd->pbmi = &vbmiEmpty;
	return fTrue;
}



/* B U I L D  S C C */
/* %%Function:BuildScc %%Owner:PETERJ */
BuildScc(pscc, dxp, dyp)
struct SCC *pscc;
int dxp, dyp;
{
	SetWords(pscc, 0, cwSCC);
	pscc->hplcedl = &pscc->pplcedl;
	pscc->pplcedl = &pscc->plcedl;
	pscc->dlFirstInval = pscc->dlMax = dlSccMax;
	pscc->plcedl.cb = cchEDL;
	pscc->plcedl.dlMax = dlSccMax + 1;
	pscc->plcedl.fExternal = fFalse;

	pscc->plcedl.icpAdjust = 1;
	/* pscc->plcedl.dcpAdjust = cp0; */
	/* pscc->plcedl.icpHint = 0; */
	pscc->idrMac = 1;
	pscc->idrMax = 1;
	pscc->cbDr = sizeof(struct DR);
	pscc->brgdr = offset(SCC, rgdr);

	pscc->ww = wwNil;
	pscc->bmi.fDiscardable = fTrue;
	/* pscc->bmi.dxp = pscc->bmi.dyp = 0; */
}


/* F R e a d  U s e r  S t a t e */
/* %%Function:FReadUserState %%Owner:bryanl */
FReadUserState()
{
/* initialize these to standard values */
	/* vpref */
	/* ass   */
	/* vcdi  */
	/* vpref = { 0, 0, ... } */

	Assert(vhsttbOpen == hNil);

	/* if no winword.ini, need to initialize default prefs */
	if (!FReadOpusIni() && !FInitDefaultPrefs(fFalse))
		return fFalse;

	/*  this is not a *user* view preference, used internally only! */
	vpref.grpfvisi.fForceField = fFalse;

	vfFileCacheDirty = fTrue;

	return fTrue;
}


/*  F  I N I T  D E F A U L T  P R E F S  */
/*  Initialize user prefs to defaults.  This will either be called if
	there was no winword.ini file, or the user ran the tutorial (CBT
	likes to start out with the defaults).
*/
/* %%Function:FInitDefaultPrefs %%Owner:bryanl */
FInitDefaultPrefs(fTutorial)
BOOL fTutorial;
{
	/* don't wipe out user name and initials if tutorial */
	SetBytes (&vpref, 0, fTutorial ? cbPREFShort : cbPREF);
	vpref.ut = fTutorial ? utCBT : vitr.fMetric ? utCm : utInch;
	if (!fTutorial)
		{
		/* these default to fFalse for tutorial */
		vpref.fDisplayAsPrint   = fTrue;
		vpref.fBkgrndPag        = fTrue;
		vpref.fShortMenus       = fTrue;
		vpref.fRibbon           = fTrue; /* since winword 1.1 */
		vpref.fRuler            = fTrue; /* since winword 1.1 */
		vpref.fAutoDelete       = fTrue; /* since winword 1.1 */
		}
	vpref.fPromptSI         = fTrue;
	vpref.fStatLine         = fTrue;
	vpref.fVertScrollBar    = fTrue;
	vpref.fZoomApp          = fTrue;
	vpref.fZoomMwd          = fTrue;
	vpref.fPrDirty		= fTrue;
	vpref.grpfvisi.fDrawTableDrs = fTrue;
	
#ifdef INEFFICIENT
	vpref.fShowFormulas     = fFalse;
	vpref.fvisiTabs         = fFalse;
	vpref.fvisiSpaces       = fFalse;
	vpref.fvisiParaMarks    = fFalse;
	vpref.fvisiFtnRefMarks  = fFalse;
	vpref.fvisiCondHyphens  = fFalse;
	vpref.fvisiShowAll      = fFalse;
	vpref.fSeeHidden        = fFalse;
	vpref.dxaStyWnd         = 0;
	vpref.fPPGlossaryExp    = fFalse;
	vpref.fForceField       = fFalse;
	vpref.fHorzScrollBar    = fFalse;
	vpref.fNoShowPictures   = fFalse;
#endif  /* INEFFICIENT */

	vpref.grpfvisi.grpfShowResults   = -1;
	vpref.iAS                        = iASNever;
	/* vpref.fEllip                  = fTrue; */
	vpref.fShowF                     = fTrue;

	return ((vhsttbOpen = HsttbInit(0, fFalse/*fExt*/)) != hNil);
}


/* RSTREAM - Read STREAM. Special sequential read only stream used by 
*  FReadOpusIni and supporting code to reduce number of disk reads while
*  reading winword.ini.
*
*  WARNING: Do not use any of this stuff outside of FReadOpusIni.
*/
#define cbMaxRstream	(1024*4)
typedef struct _rstream
	{
	int osfn;
	int ib;
	int ibMac;
	char rgb[cbMaxRstream];
} RSTREAM;


/* F  R E A D  O P U S  I N I */
/* %%Function:FReadOpusIni %%Owner:bryanl */
BOOL FReadOpusIni()
	{ /* Read vpref structure in from opus.ini file. Note:  if this routine fails
			due to insufficient heap, we free what we used and use our defaults
			(ignoring their opus.ini all together).
	*/

	int fn;
	int ifdb;
	struct PREFD prefd;
	CHAR sz[cchMaxFile];
	RSTREAM rstream;
	struct OFS ofs;

	Assert(hstDMQPath == hNil);

	if (!FFindFileSpec(StSharedKey("WINWORD.INI",OpusIni), sz, grpfpiIni, nfoNormal))
		/* can't find file */
		return fFalse;

	StToSzInPlace(sz);

#ifdef DFILEPATHS
	CommSzSz(SzShared("Looking for: "), sz);
#endif /* DFILEPATHS */

	rstream.ib = rstream.ibMac = cbMaxRstream; /* Init rstream */
	if ((rstream.osfn = OpenFile(sz, &ofs, OF_READ|bSHARE_DENYWR)) >= 0)
		{
		if (CchReadPrstream(&rstream, ((CHAR FAR *) &prefd), cbPREFD) < 0)
			goto LErrorRet;

		if (prefd.nPrefPrefdVer != nPrefPrefdVerCur)
			{
			/* Special case reading 1.00 .ini file into version 1.00a to allow
				for transparent conversion from 1.00 to 1.00a.  The only diff-
				erence between them is the size of the printer metrics.
			*/
			if (prefd.nPrefPrefdVer == nPrefPrefdVer100
					&& nPrefPrefdVerCur == nPrefPrefdVer100a)
				{
				/* Update size of printer metrics and continue; Everything but
					printer information (metrics) will be read from the .ini file.
				*/
				prefd.cbPrinterMetrics = bMaxPriFile - bMinPriFile;
				}
			else
				goto LErrorRet;
			}

		if (!prefd.cbSttbFileCache || 
				(vhsttbOpen = HReadCbFromPrstream(&rstream, prefd.cbSttbFileCache))
				== hNil)
			{
			if ((vhsttbOpen = HsttbInit(0, fFalse/*fExt*/)) == hNil)
				goto LErrorRet;
			}

		Assert(vhsttbOpen != hNil && !(*vhsttbOpen)->fExternal);

		DMFlags.fIniQPath = fFalse;
		if (prefd.cbStDMQPath > 0)
			{
			if ((hstDMQPath = HReadCbFromPrstream(&rstream, prefd.cbStDMQPath))
					== hNil)
				{
				goto LErrorRet;
				}
			DMFlags.fIniQPath = fTrue;
			}

		/* read info about the printer */
		if (vpri.hszPrinter && prefd.cbPrNameAndDriver)
			{
			CHAR grpsz [ichMaxProfileSz*2];

			Assert( prefd.cbPrNameAndDriver <= ichMaxProfileSz*2);
			if (CchReadPrstream(&rstream,(CHAR FAR *)grpsz,prefd.cbPrNameAndDriver) < 0)
				goto LErrorRet;
			if (!FNeNcSz(grpsz, *vpri.hszPrinter) &&
					!FNeNcSz(grpsz+CchSz(grpsz), *vpri.hszPrDriver))
				{
				InitVpriFromFile( &rstream, &prefd );
				}
			}

		Debug( vdbs.fCkHeap ? CkHeap() : 0 );

		/* close if file access was successful */
		FCloseDoshnd(rstream.osfn);

		if (vmerr.fMemFail)
			goto LErrorRet;
		vpref = prefd.pref;
		return fTrue;
		}
	else
		{
LErrorRet:
		FreePhsttb(&vhsttbOpen);
		/* file access failure or out of memory if here */
		ErrorEid(eidBadOpusIni, "ReadOpusIni");
		if (rstream.osfn > 0)
			FCloseDoshnd(rstream.osfn);
		/* don't fail to boot because of bogus ini file */
		vmerr.mat = matNil;
		return fFalse;
		}
}


/* %%Function:InitStructScc2 %%Owner:PETERJ */
InitStructScc2()
{
	static struct SCC **hsccAbove, *psccAbove;

	psccAbove = &vsccAbove;
	hsccAbove = &psccAbove;
	/* wwSccAbove = */ AssertDo(IAllocClsH(clsWWD, hsccAbove, cwSCC) == wwSccAbove);
}


/* I n i t V p r i F r o m F i l e */

/* %%Function:InitVpriFromFile %%Owner:bryanl */
InitVpriFromFile(prstream,pprefd)
RSTREAM *prstream;
struct PREFD *pprefd;
{
	extern struct FCE rgfce[];
	extern int vflm;
	struct STTB **hsttbFont = hNil;
	int ibstPaf, ibstFont;
	int cbFontWidths;
	int ifce;
	int ifceMac;
#ifdef IFONLYWECOULD
	char 	**hprenv = hNil;
#endif
	struct FCE *pfce;

	/* Don't use printer info if reading a 1.00 ini file into 1.00a.
		This allows transparent migration from 1.00 to 1.00a, since only
		printer information changed between the two ini file versions.
	*/
	if (pprefd->nPrefPrefdVer == nPrefPrefdVer100
			&& nPrefPrefdVerCur == nPrefPrefdVer100a)
		{
		/* We rely on SetFlm to update printer info, so vpri.hdc must be NULL */
		Assert(vpri.hdc == NULL);
		goto LError;	/* Don't use printer information from ini file */
		}

	if (CchReadPrstream(prstream,
			(CHAR FAR *)((CHAR *)&vpri+bMinPriFile),
			pprefd->cbPrinterMetrics) < 0)
		goto LError;

#ifdef IFONLYWECOULD
	if (pprefd->cbPrenv && 
			(hprenv = HReadCbFromPrstream(prstream, pprefd->cbPrenv)) == hNil)
		goto LError;

/* if printer environment has changed, can't use pref file info */
	GetPrintEnv();
	if ( !(pprefd->cbPrenv == 0 && vpri.hprenv == hNil) &&
			(!pprefd->cbPrenv != !vpri.hprenv ||
			CbOfH(vpri.hprenv) != CbOfH(hprenv) ||
			FNeRgch( *vpri.hprenv, *hprenv, CbOfH(hprenv) )))
		{
		FreeH( hprenv );
		goto LError;
		}
	FreeH( hprenv );
#else
	if (pprefd->cbPrenv && 
			(vpri.hprenv = HReadCbFromPrstream(prstream, pprefd->cbPrenv)) == hNil)
		goto LError;
#endif

	Assert( vpri.hsttbPaf == hNil );
	if (pprefd->cbSttbFont &&
			(hsttbFont = HReadCbFromPrstream(prstream, pprefd->cbSttbFont)) == hNil)
		goto LError;

	Assert(hsttbFont == hNil || !(*hsttbFont)->fExternal);

	if (pprefd->cbSttbPaf)
		{
		Assert( hsttbFont != hNil );
		if ((vpri.hsttbPaf = HReadCbFromPrstream(prstream, pprefd->cbSttbPaf)) == hNil)
			goto LError;

		Assert(vpri.hsttbPaf != hNil && !(*vpri.hsttbPaf)->fExternal);

/* adjust ibstFont's in Paf to point to current font table, adding
	fonts as necessary */

		for ( ibstPaf = 0 ; ibstPaf < (*vpri.hsttbPaf)->ibstMac; ibstPaf++ )
			{
			struct PAF *ppaf = (struct PAF *)PstFromSttb( vpri.hsttbPaf, ibstPaf );

			if ((ibstFont = IbstXlate( hsttbFont, ppaf->ibst)) == iNil)
				goto LError;

			((struct PAF *)PstFromSttb( vpri.hsttbPaf, ibstPaf ))->ibst = ibstFont;
			}
		}
	else
		goto LError; /* cannot use printer info */

/* read font cache contents */

	if (pprefd->cbRgfce)
		{
		Assert( hsttbFont != hNil );
		ifceMac = pprefd->cbRgfce/sizeof(struct FCE);
	/* verify that cbRgfce isn't bigger than rgfce!. Should always
		be true unless in a version change the FCE changes or ifceMax
		gets smaller. To handle those we bag out here. bz  */
		if (ifceMac >= ifceMax || ifceMac * sizeof(struct FCE) != pprefd->cbRgfce)
			{
			Assert (fFalse);
			goto LError;
			}
		if (CchReadPrstream( prstream, (CHAR FAR *)rgfce, pprefd->cbRgfce)
				< pprefd->cbRgfce)
			goto LError;
		for ( ifce = 0, pfce = rgfce; ifce < ifceMac; ifce++,pfce++ )
			{
			if (!pfce->fFixedPitch)
				{
				int FAR *lpdxp;
				int cch;

				if ((pfce->hqrgdxp = HqAllocLcb( (long)512 )) == hqNil)
/* Peter, these do not need to be "freeded" on error, as the caller
	will recogize vmerr.fMemFail and refuse to bring Opus up. */
					goto LError;
				lpdxp = LpLockHq( pfce->hqrgdxp );
				Assert( lpdxp != NULL );	/* should be guaranteed */
				cch = CchReadPrstream( prstream, lpdxp, 512);
				UnlockHq( pfce->hqrgdxp );
				if (cch < 512)
					goto LError;
				}

			pfce->pfcePrev = pfce-1;
			pfce->pfceNext = pfce+1;
			pfce->hfont = hfontSpecNil;
			if ((pfce->fcidRequest.ibstFont = IbstXlate( hsttbFont, 
					pfce->fcidRequest.ibstFont )) == iNil)
				goto LError;
			if ((pfce->fcidActual.ibstFont = IbstXlate( hsttbFont, 
					pfce->fcidActual.ibstFont )) == iNil)
				goto LError;
			}

		rgfce [0].pfcePrev = NULL;
		if (ifceMac)
			rgfce [ifceMac-1].pfceNext = NULL;
		}
	else
		goto LError; /* cannot use printer info */

	if (hsttbFont != hNil)
		FreeHsttb( hsttbFont );

/* everything succeeded!  Now set up to simulate as though we are in flm
	Repaginate and the printer DC has been gotten, then tossed. */

	vflm = flmRepaginate;
	vpri.fIC = fTrue;
	vpref.fPrDirty = fTrue;
	vpri.pfti = &vfti;
	if (pprefd->cbRgfce)
		vfti.pfce = &rgfce [0];
	vfti.fPrinter = fTrue;
	vfti.fTossedPrinterDC = fTrue;
	vfli.fPrint = fTrue;
	vfli.fFormatAsPrint = fFalse;
	vftiDxt.dxpInch = vfti.dxpInch = vfli.dxuInch = vpri.dxuInch;
	vftiDxt.dypInch = vfti.dypInch = vfli.dyuInch = vpri.dyuInch;
	Debug( vdbs.fCkFont ? CkFont() : 0 );
#ifdef BRYANL
	CommSzSz(SzShared("Used printer info from pref file"),szEmpty);
#endif
	return;

LError:
#ifdef BRYANL
	CommSzSz(SzShared("Didn't use printer info from pref file"),szEmpty);
#endif
	FreePh(&vpri.hprenv);
	FreePhsttb(&vpri.hsttbPaf);
	if (hsttbFont != hNil)
		FreeHsttb(hsttbFont);
	vpref.fPrDirty = fTrue;
}


/* %%Function:IbstXlate %%Owner:bryanl */
IbstXlate( hsttbFont, ibst )
struct STTB **hsttbFont;
int ibst;
{
	int ibstFont;
	struct FFN *pffn;

	Assert( ibst < (*hsttbFont)->ibstMac );
	pffn = (struct FFN *)PstFromSttb( hsttbFont, ibst );

	if ((ibstFont = IbstFindSzFfn( vhsttbFont, pffn )) == iNil)
		{
		CHAR rgb [cbFfnLast];

		bltbyte( pffn, rgb, cbFfnLast );
		ibstFont = IbstAddStToSttb( vhsttbFont, rgb );
		}
	else
		/* in case charset, pitch, or family changed */
		FChangeStInSttb(vhsttbFont, ibstFont, pffn);

	return ibstFont;
}



/* %%Function:HReadCbFromPrstream %%Owner:bryanl */
HReadCbFromPrstream(prstream,cb)
RSTREAM *prstream;
int cb;
{
	CHAR **h;
	Assert(prstream!=NULL && prstream->osfn >= 0 && cb > 0);
	if ((h = HAllocateCb(cb)) != hNil && 
			CchReadPrstream(prstream, (CHAR FAR *)*h, cb) < 0)
		FreePh(&h);
	return h;
}


/* %%Function:CchReadPrstream %%Owner:bryanl */
int CchReadPrstream(prstream, lpb, cb)
RSTREAM *prstream;
CHAR FAR *lpb;
int cb;
{
	int cbRead;
	int cbInBuf;

	Assert(prstream!=NULL && prstream->osfn >= 0);

	cbRead = 0;
	while (cb > 0)
		{
		if (prstream->ib >= prstream->ibMac)
			{
			/* If ib whent beyond ibMac we are in deep trouble */
			Assert(prstream->ib == prstream->ibMac);

			if ((cbInBuf = CchReadDoshnd(prstream->osfn, 
					(CHAR FAR *) prstream->rgb, cbMaxRstream)) < 0)
				return cbInBuf; /* return Error */

			if (cbInBuf == 0) return cbRead; /* EOF */
			prstream->ibMac = cbInBuf;
			prstream->ib = 0;
			}

		cbInBuf = min(cb, prstream->ibMac - prstream->ib);

		bltbx((CHAR FAR *) (prstream->rgb + prstream->ib), lpb, cbInBuf);
		cbRead += cbInBuf;
		cb -= cbInBuf;
		prstream->ib += cbInBuf;
		lpb += cbInBuf;
		}
	return cbRead;
}


#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Initwin_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* Initwin_Last() */
