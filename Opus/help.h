#define szHelpAppDef      "WHELP.EXE"

/* special help contexts for communicating with the help app */

#define cxtNil		0x0000  /* No context */
#define cxtIndex        0xffff	/* help context for index               */
#define cxtHelpOnHelp   0xfffc	/* help context for help on help        */


/* Contexts for top-level menus are computed as offsets from cxtMenuBar */
/* 0xfe00 is App control menu
	0xfe01 is Doc control menu
	0xfe02 is File menu
	etc...

	This works regardless of whether Doc window is maximized (and hence
	its control menu appears on the main menu bar) or not.
	
*/
#define cxtMenuBar      0xfe00
#define cxtAppControl	(cxtMenuBar)
#define cxtDocControl   (cxtMenuBar + 1)

#define cxtAppMenuSelect   0xf000  /* context must be computed from saved vals */
#define cxtDocMenuSelect   0xf001  /* context must be computed from saved vals */

/* CBT window messages */

#define CBT_OpusID      0x0200     /* Opus ID number for WM_CBTINIT message */

#define WM_CBTINIT      0x03f0      /* CBT initialization message */
#define WM_CBTTERM      0x03f1      /* CBT terminating; resume normal mode */
#define WM_CBTNEWWND    0x03f2      /* new window being created   */
#define WM_CBTSEMEV     0x03f3      /* semantic event sent to CBT */
#define WM_CBTWNDID     0x03f5		/* window id of new window */
#ifdef NOTUSED
#define WM_HELP         0x03f8      /* WinHelp context message    */
#endif


/* CBT SeMantic eVent types */
/* predefined ones... */

#define SE_TEXT         0x0001      /* Text event */

/* If "(computed)" appears next to an smv below, that means the smv value
	is computed in-line by adding some offset onto a base smv.  Look just
	above the smv in question, and you should find the SmvFrom???? function. */
#define smvSelection        0x0200      /* new text selection */ 
#define smvHdrSelection     0x0201      /* new header selection */
#define smvFtrSelection     0x0202      /* new footer selection */
#define smvFNSelection      0x0203      /* new footnote selection */
#define smvAnnSelection     0x0204      /* new annotation selection */
#define smvAnnFNMark        0x0210      /* doubleclick on Annot or FN mark */
#define smvIBBase           0x0220      /* Base IB (icon bar) smv */
#define SmvFromIbc(ibc)    (smvIBBase + ibc)
#define smvIBHdrFtr         0x0220      /* header/footer IB buttons (computed) */
#define smvIBOutline        0x0221      /* outline IB buttons (computed) */
#define smvIBMcrEdit        0x0222      /* Macro edit IB buttons (computed) */
#define smvIBRibbon         0x0223      /* ribbon IB buttons (computed) */
#define smvIBRuler          0x0224      /* ruler buttons (computed) */
#define smvIBPrvw           0x0225      /* print preview IB buttons (computed) */
#define smvTabCreate        0x0230      /* create new tab on ruler */
#define smvTabMove          0x0231      /* move an existing tab */
#define smvTabDelete        0x0232      /* drag tab off of ruler */
#define smvIndentBoth       0x0233      /* 0234 & 0235 together */
#define smvRulerBase        0x0230      /* Base ruler smv */
#define SmvFromImk(imk)    (smvRulerBase + imk)
#define smvIndentLeft1      0x0234      /* First column indent (computed) */
#define smvIndentLeft       0x0235      /* (computed) */
#define smvIndentRight      0x0236      /* (computed) */
#define smvRulerTLeft       0x0238      /* (computed) */
#define smvRulerTableCol    0x0239      /* (computed) */
#define smvMarginLeft       0x023a      /* (computed) */
#define smvMarginRight      0x023b      /* (computed) */
#define smvPrvwDrag         0x0240      /* drag margins, page breaks, etc */
#define smvPrvwUpdate       0x0241      /* update print preview display */
#define smvPrvwOtherPage    0x0242      /* Click on other page */
#define smvPrvwPageView     0x0243      /* doubleclick to enter page view */
#define smvPrvwClick        0x0244      /* click when borders not showing */
#define smvTableSelection   0x0250      /* select cells in a table */
#define smvPicFormat        0x0260      /* crop or scale a picture */
#define smvNonPicSel        0x0262      /* pic active; click outside of pic */
#define smvOutlineSelect    0x0270      /* Click outline icon to select para */
#define smvOutlinePromote   0x0271      /* Promote/demote paragraph */
#define smvOutlineMove      0x0272      /* Move para up or down */
#define smvOutlineExpand    0x0273      /* expand/collapse text */
#define smvBeginTyping      0x0280      /* entering the insert loop */

/* note we can't let CBT do command events themselves since bcm's change;
	need to do it with semantic events */
#define smvCommand          0x0290      /* WM_COMMAND or WM_SYSCOMMAND recvd */
#define smvTrackStyWnd		0x02a0		/* click to drag style name area */
#define smvCantBootHelp		0x02FE		/* OOM or can't find Help */
#define smvWmChar           0x02FF      /* WM_CHAR received */


/* CBT strings */

#define szCbtWinDef   "CbtComm"
#define szCbtDef      "WINWORD.CBT"
#define szCbtLibDef   "WINWORD.LIB"
#define szCbtDirDef   "winword.cbt"

/* CBT's special normal.dot to be loaded from winword.cbt directory */
#define szCbtGlobalDotDef "CBTNORM"


/** MWDSTATE:
This structure is stored in the cbExtra of an sttb; it contains the
information about each window that we store when we start up CBT.  We then
use this information to restore the user to their previous state, with all
windows reopened and correctly placed, as soon as CBT is finished.    **/

	struct MWDSTATE {
	/** position of MWD **/
	struct RC rcMwd;

	/** preferences information for the window **/
	int	fHorzScrollBar  : 1;
	int	fVertScrollBar  : 1;
	int	fRuler          : 1;
	int	lvlOutline      : 4;    /* lvl shown on the outline iconbar. */
	int	: 1;    /* spare */

	/** current selection in the window **/
	struct SELS sels;

};


/************************************
*
* These cxt defs added 7/5/88
*                            -awe
*************************************/

#define CxtFromIbc(ibc) (cxtIconBars + ibc)
/* ibc values are defined in ibdefs.h */

/**  Screen Regions:     (8000-80FF)  **/

#define cxtAppControlMenuBox        0x8000
#define cxtDocControlMenuBox        0x8001
/* WARNING!  Do not change the order of the seven cxt's below; 
				They are used by CxtFromIbc.                         */
#define cxtIconBars                 0x8002
#define cxtHeaderFooterIconBar      0x8002
#define cxtOutlineIconBar           0x8003
#define cxtMacroEditIconBar         0x8004
#define cxtRibbonIconBar            0x8005
#define cxtRulerIconBar             0x8006
#define cxtPreviewIconBar           0x8007
/* 0x8008, 0x8009, and 0x800A are reserved for future iconbars */

#define cxtScrollBar                0x800B
#define cxtPageViewScrollIcon       0x800C
#define cxtSelectionBar             0x800D
#define cxtSplitBox                 0x800E
#define cxtStatusBar                0x800F
#define cxtStyleArea                0x8010
#define cxtTitleBar                 0x8011
#define cxtWindowBorder             0x8012
#define cxtAppMaximize              0x8013
#define cxtAppMinimize              0x8014
#define cxtRestoreBox               0x8015
#define cxtSizeBox                  0x8016

/** Active Window      (8100-81FF) **/

#define cxtDraftView                0x8100
#define cxtMacroEditingWindow       0x8101
#define cxtNormalEditingView        0x8102
#define cxtOutlineView              0x8103
#define cxtPageView                 0x8104
#define cxtPrintPreview             0x8105
#define cxtAnnotationPane           0x8106
#define cxtFootnotePane             0x8107
#define cxtHeaderFooterPane         0x8108
#define cxtFootnoteSeparator        0x8109

/** Icon bar modes      (8200-823F)  **/

#define cxtRulerMode                0x8200
#define cxtPrintPreviewIconBarMode  0x8201
#define cxtOutlineIconBarMode       0x8202
#define cxtHeaderFooterIconBarMode  0x8203
#define cxtMacroEditingIconBarMode  0x8204
#define cxtFtnSepIconBarMode		0x8205

/** Application and Document system menus  (8240-827F)  **/

#define cxtAppRestore               0x8240
#define cxtAppMove                  0x8241
#define cxtAppSize                  0x8242
#define cxtAppClose                 0x8243

#define cxtDocMaximizeBox           0x8250

/** Miscellaneous       (8300-????)  **/

#define cxtKeyboard                 0x8300  /* Keyboard topic on Help menu */
#define cxtWordWindow               0x8301  /* Active window when no ww open */
#define cxtUserDefined              0x8310  /* User defined macro */
#define cxtMacroError               0x8320  /* Macro error message */
#define cxtMacroMsgBox              0x8321  /* ElMsgBox message */
#define cxtMacroRecordOnFile        0x8322  /* Macro Record when on File menu */
#define cxtMacroRunOnFile           0x8323  /* Macro Run when on File menu */
#define cxtFtnSepIconBar 			0x8324  /* Uses ibcHdr - differentiate here */

/** Stuff used to save state when CBT is running: **/
	struct CBTSAV {
	struct STTB **hsttbMwdSav;   /* info for mwds to be restored after CBT */
	struct STTB **hsttbOpenSav;  /* MRU list before CBT; to be restored after */
	struct PREF prefSave;	  /* user's preferences before running CBT; to be
									restored after CBT quits */
	int	iDocumentSav;		  /* Doc # before CBT; to restore after */
	int	iTemplateSav;		  /* Template # before CBT; to restore after */
	int	hcDotPath: 14; 		  /* Compacted handle to DOT-PATH */
	int	fUtilPath: 1;		  /* Whether user had a UTIL-PATH */
	int	fChangedGlobalDot: 1; /* Whether we changed global dot */
	CHAR chDrive;             /* drive letter to restore if nec */
	int	fStatLine: 2;         /* tri-state flag - needs two bits */
	int	fZoomed: 2;			  /* tri-state flag - needs two bits */
	struct RC rcUnZoom;		  /* unzoomed window state */
};

#ifdef NOABOUT
BOOL FDlgAbout(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	/* make ESC close the dialog, as ok does */
	if (dlm == dlmKey)
		if (wNew == VK_ESCAPE)
			EndDlg(tmcOK);
		else
			CheckRgksp(wNew);
	return fTrue;
}
#endif

/* CBT Termination Reason */
#define ctrOutOfMemory    0
#define ctrOldFileFormat  1
