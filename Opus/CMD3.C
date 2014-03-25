
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "debug.h"
#include "cmdtbl.h"
#include "disp.h"
#include "props.h"
#include "sel.h"
#include "ch.h"
#include "keys.h"
#include "prm.h"
#include "doc.h"
#include "cmd.h"
#include "profile.h"
#include "format.h"
#include "screen.h"
#include "heap.h"
#include "border.h" /* to get TCC */
#include "rerr.h"
#include "print.h"
#define AUTOSAVE
#include "autosave.h"
#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"
#include "iconbar.h"
#include "resource.h"
#include "error.h"
#include "pic.h"
#include "menu2.h"
#define RSBSONLY
#include "rsb.h"
#include "version.h"
#include "message.h"
#include "idle.h"
#define RULER
#define REPEAT
#include "ruler.h"
#include "rareflag.h"

#include "apprun.hs"
#include "apprun.sdm"

extern struct RF 	vrf;
extern ASD      asd;
extern CHAR szApp[];
extern BOOL fElActive;
extern struct SCI vsci;
extern struct WWD ** hwwdCur;
extern HMENU            vhMenu;
extern int              viMenu;
extern int		wwMac;
extern struct WWD **    mpwwhwwd[];
extern BOOL         vfFileCacheDirty;
extern BOOL         vfWndCacheDirty;
extern struct MWD ** hmwdCur;
extern HWND vhwndStatLine;
extern struct SAB vsab;
extern HWND       vhwndApp;
extern HANDLE vhInstance;
extern struct AAB       vaab;
extern BOOL vfExtendSel;
extern BOOL vfOvertypeMode;
extern CHAR HUGE        *vhpchFetch;
extern struct RULSS  vrulss;
extern RRF		rrf;
extern HWND		vhwndRibbon;
extern BOOL vfBlockSel;
extern struct SEL selCur;
extern KMP ** hkmpCur;
extern struct CHP vchpFetch;
extern int wwCur;
extern int mwCur;
extern struct PREF vpref;
extern struct STTB ** vhsttbWnd;
extern struct UAB vuab;
extern struct MWD **mpmwhmwd[];
extern struct RC vrcUnZoom;
extern int mwMac;
extern int vmwClipboard;
extern struct TCC vtcc;
extern int vlm;
extern struct PIC   vpicFetch;
extern struct BMI mpidrbbmi [];
extern struct MERR vmerr;
extern HWND vhwndPgvCtl;
extern struct MERR      vmerr;
extern struct SEL       selCur;
extern struct UAB       vuab;
extern int              vccpFetch;
extern CHAR HUGE        *vhpchFetch;
extern struct CHP       vchpFetch;
extern struct PAP       vpapFetch;
extern struct CA        caPara;
extern CP               vcpFetch;
extern BOOL             vfEndFetch;
extern IDF		vidf;
extern struct STTB	**vhsttbFont;
extern struct PRI 	vpri;


#define iAppMin		0
#define iAppClipbrd	0
#define iAppControl	1
#define iAppMax		2


csconst char csrgstApp [] [] = 
{
	StKey("CLIPBRD.EXE",ClipbrdFileDef),
			StKey("CONTROL.EXE",CtrlFileDef)
};

#ifdef INTL
csconst char csrgstUSApp [] [] =
{
	St("CLIPBRD.EXE"),
			St("CONTROL.EXE")
};
#endif /* INTL */


/* These strings are the Module and Class names of the apps in csrgstApp */
#define istAppInfoClipbrd 0
#define istAppInfoControl 2
csconst char csrgstAppInfo[][] =
{
	St("CLIPBRD"),
			St("CLIPBOARD"),
			St("CONTROL"),
			St("CTLPANEL")
};


#define ichMaxApp 16

/*  %%Function: CmdControlRun  %%Owner: bradch  */

CMD CmdControlRun(pcmb)
CMB * pcmb;
{
	if (FCmdFillCab())
		{
		((CABAPPRUN *) *pcmb->hcab)->iApp = 0;
		}

	if (FCmdDialog())
		{
		char dlt [sizeof(dltAppRun)];

		BltDlt(dltAppRun, dlt);

		switch (TmcOurDoDlg(dlt, pcmb))
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			return cmdError;
#endif
		case tmcError:
			return cmdNoMemory;

		case tmcCancel:
			return cmdCancelled;

		case tmcOK:
			break;
			}
		}

	if (FCmdAction())
		{
		int iApp, cch;
		char szApp [ichMaxApp];

		iApp = ((CABAPPRUN *) *pcmb->hcab)->iApp;

		if ((uns) (iApp - iAppMin) >= (iAppMax - iAppMin))
			return cmdError;

		switch (iApp)
			{
		case iAppClipbrd:
			if (FReactivateApp(istAppInfoClipbrd))
				return cmdOK;
			break;
		case iAppControl:
			if (FReactivateApp(istAppInfoControl))
				return cmdOK;
			break;
			}

		bltbx(csrgstApp[iApp] + 1, (char FAR *) szApp, 
				cch = csrgstApp[iApp][0]);
		szApp[cch] = '\0';

		StartLongOp();
		/*  adjusts memory and tries to run application */
		if (!FOurRunApp(szApp, SW_SHOWNORMAL, eidNoMemRunApp, fTrue))
			{
#ifdef INTL
			bltbx(csrgstUSApp[iApp] + 1, (char FAR *) szApp,
				cch = csrgstUSApp[iApp][0]);
			szApp[cch] = '\0';
			if (!FOurRunApp(szApp, SW_SHOWNORMAL, eidNoMemRunApp, fTrue))
#endif /* INTL */
				ErrorEid(eidCantRunApp, "CmdControlRun");
			}
		EndLongOp(fFalse);
		}

	return cmdOK;
}


/*  %%Function: FReactivateApp  %%Owner: bradch  */

FReactivateApp(istAppInfo)
int istAppInfo;
{
	HWND hwnd;
	int cch;
	CHAR sz[ichMaxApp];

	/* Load the module name */
	cch = csrgstAppInfo[istAppInfo][0];
	bltbx(csrgstAppInfo[istAppInfo]+1, (char FAR *) sz, cch);
	sz[cch] = '\0';

	if (GetModuleHandle((LPSTR)sz))
		{
		/* Load the class name */
		cch = csrgstAppInfo[istAppInfo+1][0];
		bltbx(csrgstAppInfo[istAppInfo+1]+1, (char FAR *) sz, cch);
		sz[cch] = '\0';
		if ((hwnd = FindWindow((LPSTR)sz, NULL)) != NULL)
			{
			if (FActivateWindow(hwnd))
				{
				if (IsIconic(hwnd))
					OpenIcon(hwnd);
				return fTrue;
				}
			}
		}
	return fFalse;
}



/* C M D  E D I T  P I C */
/*  Launch the DRAW application with the selected picture in the clipboard.
	when focus returns to Opus, paste the picture from the clipboard.
*/
/*  %%Function: CmdEditPic  %%Owner: bobz  */

CMD CmdEditPic(pcmb)
CMB *pcmb;
{
	CHAR rgchCmdLine[cchMaxSz];
	CHAR szDrawApp[64];
	CHAR szDrawAppDef[64];
	int rgw[2];
	int mm;

/* place graphic in clipboard */
	/* make sure current selection is a graphic selection */
	/* metafile import field will be treated as a pic, not TIFF field */
	if (!FPicRenderable(selCur.doc, selCur.cpFirst, selCur.cpLim, &mm))
		{
		Beep();
		return cmdError;
		}

	Assert(selCur.fGraphics);

	CmdCopy1(&selCur, &selCur.ca);

	GetSzProfileIpst(ipstDrawName, szDrawApp, szDrawAppDef, 64, fFalse);
	rgw[0] = szDrawApp;
	rgw[1] = szApp;
	BuildStMstRgw(mstLaunchDraw, rgw, rgchCmdLine, cchMaxSz, hNil);
	StToSzInPlace(rgchCmdLine);

	vrf.fWaitForDraw = fTrue;

/* launch DRAW application */
	if (FOurRunApp(rgchCmdLine, SW_SHOWNORMAL, eidNoMemRunApp, fTrue))
		return cmdOK;

	else
		{
		vrf.fWaitForDraw = fFalse;
		return cmdError;
		}
}


/* D O  E D I T  P I C  2 */
/*  %%Function: DoEditPic2  %%Owner: bobz  */

DoEditPic2()
{
	HWND hwnd;
	HANDLE h;
	CP cpPic;
	CHAR szClassOwner[64];
	CHAR szClassDraw[64];
	CHAR szClassDrawDef[64];
	struct CA ca1, ca2;
	struct PIC pic;
	int mm;

	vrf.fWaitForDraw = fFalse;

	GetSzProfileIpst(ipstDrawClass, szClassDraw, szClassDrawDef, 64, fFalse);

	if ((hwnd = GetClipboardOwner()) == vhwndApp
			|| (!IsClipboardFormatAvailable(CF_METAFILEPICT) &&
			!IsClipboardFormatAvailable(CF_BITMAP)))
		return;

	/* TIFF import field will be rejected, even though it should never get this far */
	/* for a metafile import field, we will only replace the single
	   chPic character.
	*/

	if (!FPicRenderable(selCur.doc, selCur.cpFirst, selCur.cpLim, &mm))
		return;

/* get old formatting for merge FPicRenderable did a fetchpe, and
   set up vcpFetch as the chPic character, even for an import
   field   */

	Assert(*vhpchFetch == chPicture && vchpFetch.fSpec);
	pic = vpicFetch;  /* old pic formatting */
	cpPic = vcpFetch; /* cp of chPic char */

	if (hwnd != NULL)
		GetClassName(hwnd, (LPSTR)szClassOwner, 255);

#ifdef DEDITPIC
	CommSzNum(SzShared("hwnd owner = "), hwnd);
	CommSzSz(SzShared("owner class = "), szClassOwner);
	CommSzSz(SzShared("looking for class = "), szClassDraw);
#endif /* DEDITPIC */

	if ((hwnd != NULL && *szClassDraw && FNeNcSz(szClassOwner, szClassDraw))
			|| !FReadExtScrap() || !vsab.fPict || 
			CpMacDoc(docScrap) != 1+ccpEop)
		return;

/* set up undo and kill the existing selection */
	if (!FSetUndoBefore(bcmPaste, uccPaste))
		return;
	TurnOffSel(&selCur);

/* copy the new picture in */
	if (!FReplaceCps(PcaSetDcp(&ca1, selCur.doc, cpPic, (CP)1), 
			PcaSetDcp(&ca2, docScrap, cp0, (CP)1)))
		return;

/* check table/dead field properties */
	AssureNestedPropsCorrect(PcaSetDcp(&ca1, selCur.doc, cpPic, (CP)1), fTrue);

/* select the new picture */
	Select(&selCur, cpPic, cpPic+1);

/* merge in old pictures formatting */
    MergePicProps(selCur.doc, cpPic, &pic);

/* assure document props correct */
	PdodDoc(selCur.doc)->fFormatted = fTrue;
	PdodDoc(selCur.doc)->fMayHavePic = fTrue;

/* set up undo */
	SetUndoAfter(&ca1);
}


#define cchBuffer 256

/*  %%Function: CmdSwapCase  %%Owner: bradch  */

CMD CmdSwapCase(pcmb)
CMB *pcmb;
{
	int doc, wcs;
	CP cpSelFirst, cpSelLim;
	int fArmUndo = fFalse;
	struct CA caT;
	extern BYTE vfNoInvalField;

	if (selCur.fIns)
		{
		Select(&selCur, CpFirstSty(wwCur, selCur.doc, selCur.cpFirst,
				styWord, fFalse), CpLimSty(wwCur, selCur.doc, selCur.cpFirst,
				styWord, fFalse));
		}

	if (selCur.sk != skSel)
		{
		Beep ();
		return cmdError;
		}

	doc = selCur.doc;
	cpSelFirst = selCur.cpFirst;
	cpSelLim = selCur.cpLim;
	CachePara(doc, cpSelFirst);

	TurnOffSel(&selCur); /* to avoid horrible flashing */

	/* Set up undo if we haven't yet */
	if (vuab.bcm != bcmChangeCase || 
			FNeRgch(&selCur.ca, &vuab.selsBefore.ca, 
			sizeof (struct CA)))
		{
		if (!FSetUndoBefore(bcmChangeCase, uccFormat))
			return cmdCancelled;
		fArmUndo = fTrue;
		}

	/* figure out which state we should put the selection in */
	wcs = WcsCa(&selCur.ca);

	vfNoInvalField = fTrue;
	/*  perform the mapping */
	MapCaseDocCps (wcs, doc, cpSelFirst, cpSelLim);
	vfNoInvalField = fFalse;

	/* all done, so reset selection */
	Select(&selCur, cpSelFirst, cpSelLim);

	if (fArmUndo)
		SetUndoAfter(NULL);

	return cmdOK;
}


/*  %%Function: ElWChangeCase  %%Owner: bradch  */

EL int ElWChangeCase(wcs)
int wcs;
{
	int doc;
	CP cpSelFirst, cpSelLim;
	int fArmUndo = fFalse;
	struct CA caT;
	extern int vcElParams;
	extern int vfElFunc;

	if (selCur.fIns)
		{
		Select(&selCur, CpFirstSty(wwCur, selCur.doc, selCur.cpFirst,
				styWord, fFalse), CpLimSty(wwCur, selCur.doc, selCur.cpFirst,
				styWord, fFalse));
		}

	doc = selCur.doc;
	cpSelFirst = selCur.cpFirst;
	cpSelLim = selCur.cpLim;
	CachePara(doc, cpSelFirst);

	if (selCur.sk != skSel)
		{
		Beep ();
		return cmdError;
		}

	if (vfElFunc)
		{
		/* Called as a function */
		wcs = WcsCa(&selCur.ca);	/* This is the next case state */
		wcs--;	/* We want the original case state */
		if (wcs < 0)
			wcs = 2;
		}
	else
		{	/* deal with range */
		if (wcs < 0 || wcs > 2)
			{
			RtError(rerrBadParam);
			/* NOT REACHED */
			Assert(fFalse);
			}
		else
			{
			TurnOffSel(&selCur);	/* to avoid horrible flashing */

			/* Set up undo if we haven't yet */
			if (vuab.bcm != bcmChangeCase || 
					FNeRgch(&selCur.ca, &vuab.selsBefore.ca, 
					sizeof (struct CA)))
				{
				if (!FSetUndoBefore(bcmChangeCase, uccFormat))
					return cmdCancelled;
				fArmUndo = fTrue;
				}
			if (vcElParams == 0)
				{
				/* Called to toggle to next case state */
				wcs = WcsCa(&selCur.ca);
				}

			/*  perform the mapping */
			MapCaseDocCps (wcs, doc, cpSelFirst, cpSelLim);
			}
		}

	/* all done, so reset selection */
	Select(&selCur, cpSelFirst, cpSelLim);

	if (fArmUndo)
		SetUndoAfter(NULL);

	return wcs;
}



/* M A P  C A S E  D O C  C P S */
/*  Map the case of the text in doc, cp:cpLim to wcs.
*/
/*  %%Function: MapCaseDocCps   %%Owner: bradch  */

MapCaseDocCps (wcs, doc, cp, cpLim)
int wcs, doc;
CP cp, cpLim;

{
	int cch, ich;
	CHAR ch;
	BOOL fFirstDone = fFalse;
	BOOL fWordBegin = fTrue;
	BOOL fToUpper;
	struct CA caT;
	CHAR rgch [cchBuffer];

	Assert (cpLim <= CpMacDoc (doc));

	while (cp < cpLim && !vmerr.fMemFail && !vmerr.fDiskFail)
		{
		CachePara (doc, cp);
		if (cp >= caPara.cpLim-ccpEop)
			{
			cp = caPara.cpLim;
			fWordBegin = fTrue;
			continue;
			}
		FetchCp (doc, cp, fcmProps+fcmChars);

		/* in toggling case, don't fool with the EOP marks or the
			fSpec characters.  Deleteing them may have bad results!! */
		if (vchpFetch.fSpec)
			{
			cp += vccpFetch;
			fWordBegin = fTrue;
			continue;
			}

		cch = min (cchBuffer, vccpFetch);
		if (cp + cch > caPara.cpLim-ccpEop)
			cch = caPara.cpLim-ccpEop-cp;
		if (cp + cch > cpLim)
			cch = cpLim-cp;
		Assert (cch > 0);

		bltbh(vhpchFetch, rgch, cch);
		for (ich = 0; ich < cch; ich++)
			{
			if (WbFromCh(ch = rgch[ich]) != wbText)
				{
				fWordBegin = fTrue;
				continue;
				}

			fToUpper = (wcs == wcsAllUpper) ||
					(wcs == wcsCapWords && fWordBegin) ||
					(wcs == wcsFirstCap && fWordBegin && !fFirstDone);

			rgch[ich] = fToUpper ?
					ChUpper(ch) : ChLower(ch);

			fWordBegin = fFalse;
			fFirstDone = fTrue;
			}

		if (FInsertRgch(doc, cp, rgch, cch, &vchpFetch, NULL))
			if (!FDelete(PcaSetDcp(&caT, doc, cp+cch, cch)))
				/* failed - remove what we inserted */
				FDelete(PcaSetDcp(&caT, doc, cp, cch));

		cp += cch;
		}
}





/* W C S  C A */
/* Given a CA, try to determine the current "Case State."  This is done by
scanning for the first two consecutive alphabetic characters.  If the first
character is lower case, it is assumed that all characters are lower case.
Otherwise, if the second character is upper case, it is assumed that all
characters are upper case.  If the first letter is upper case and the second
is lower case, it is assumed that words are capitalized.

NOTE: if there are no alphabetic characters in the first cchBuffer (256)
characters of the selection, nothing will happen!.
*/

/*  %%Function: WcsCa  %%Owner: bradch  */

int WcsCa(pca)
struct CA * pca;
{
	int wCaseState;
	int cch, ich;
	char rgch[cchBuffer];

	FetchRgch(&cch, &rgch[0], pca->doc, pca->cpFirst, pca->cpLim,
			cchBuffer);

	wCaseState = -1;
	for (ich = 0; ich < cch; ++ich)
		{
		if (FAlpha(rgch[ich]))
			{
			/* take care of case where there are not two
					consecutive alpha chars */
			if (wCaseState < 0)
				wCaseState = FLower(rgch[ich]) ?
						wcsAllUpper : wcsAllLower;

			/* take care of other cases */
			if (cch > 1 && FAlpha(rgch[ich + 1]))
				{
				if (FLower(rgch[ich]))
					wCaseState = wcsAllUpper;
				else  if (FLower(rgch[ich + 1]))
					wCaseState = wcsAllLower;
				else
					wCaseState = wcsCapWords;
				break;
				}
			}
		}

	return wCaseState;
}


/*  %%Function: CmdGrowFont  %%Owner: bryanl  */

CMD CmdGrowFont(pcmb)
CMB *pcmb;
{
	return CmdBumpFontSize(fTrue);
}


/*  %%Function: CmdShrinkFont  %%Owner: bryanl  */

CMD CmdShrinkFont(pcmb)
CMB *pcmb;
{
	return CmdBumpFontSize(fFalse);
}


/* C m d  B u m p  F o n t  S i z e */
/* not a real cmd func, just called by above. */
/* increase/decrease size of fonts in selection to next available size
	on the printer */

/*  %%Function: CmdBumpFontSize  %%Owner: bryanl  */

CmdBumpFontSize(fGrow)
int fGrow;
{
	char prl [2];
	int fArmUndo = fFalse;
	struct CA ca;

/* If we have never gotten a printer DC, this is the time to do it */
	if (vpri.hsttbPaf == hNil)
		SetFlm(flmRepaginate);
	if (vpri.hsttbPaf == hNil)
		goto LNoAction;	/* no printer attached */

	prl [0] = sprmCHps;
	if (selCur.fIns)
		{
		prl [1] = HpsBump( selCur.doc, selCur.chp.ftc, selCur.chp.hps, fGrow);
		ApplyGrpprlSelCur( prl, 2, fTrue /*fSetUndo*/ );
		return cmdOK;
		}

	if (!FSetUndoBefore(bcmFormatting, uccFormat))
		return cmdCancelled;

	PcaPoint( &ca, selCur.doc, selCur.cpFirst );

	for ( ; ca.cpFirst < selCur.cpLim ; ca.cpFirst = ca.cpLim )
		{
		FetchCpAndParaCa( &ca, fcmProps );
		ca.cpLim = CpMin( vcpFetch + vccpFetch, selCur.cpLim );
		prl [1] = HpsBump( selCur.doc, vchpFetch.ftc, vchpFetch.hps, fGrow );

		if (prl [1] != vchpFetch.hps)
			{
			ApplyGrpprlInvalCa( prl, 2, &ca );
			fArmUndo = fTrue;
			}
		}
	if (fArmUndo)
		SetUndoAfter(NULL);
	else
		{
LNoAction:
		Beep();
		return cmdError;
		}

	return cmdOK;
}



/* H p s  B u m p */
/* return next largest or smallest size for printer font.  If 
	not available (font not found or already largest/smallest), return
	existing size.  

	Uses vpri.hsttbPaf, the list of sizes obtained by printer enumeration 
*/

/*  %%Function: HpsBump  %%Owner: bryanl  */

HpsBump( doc, ftc, hps, fGrow )
int doc, ftc, hps, fGrow;
{
	int ibstFont;
	int ibpaf;
	struct PAF *ppaf;
	char *phpsMin, *phpsMac;
	int ihps;

	Assert( hps <= 255 );

	if ((ibstFont = IbstFontFromFtcDoc( ftc, doc )) == iNil)
		goto LNotFound;

	Assert( ibstFont < (*vhsttbFont)->ibstMac );
	Assert( vpri.hsttbPaf != hNil );

	for ( ibpaf = 0 ; ibpaf < (*vpri.hsttbPaf)->ibstMac ; ibpaf++ )
		{
		if (ibstFont == (ppaf = ((struct PAF *) PstFromSttb(vpri.hsttbPaf, ibpaf)))->ibst)
			{
			phpsMac = (phpsMin = &ppaf->rghps [0]) + IhpsMacPpaf(ppaf);

			if (fGrow)
				{
				for ( ; phpsMin < phpsMac ; phpsMin++ )
					if (hps < *phpsMin)
						return *phpsMin;
				}
			else
				{
				while ( phpsMac-- >  phpsMin )
					if (hps > *phpsMac)
						return *phpsMac;
				}
			break;
			}
		}

LNotFound:
	return hps;
}


BYTE vfNoInvalField = fFalse;

/* A p p l y  G r p p r l  I n v a l  C a */
/* Like ApplyGrpprlCa, but includes inval stuff from
	ApplyGrpprlSelCur.  This might be a useful routine in
	other cases.  Currently it uses ApplyGrpprlCa, not the
	Ninch version, so the toggle props are not handled correctly for
	the majority sprm. */

/*  %%Function: ApplyGrpprlInvalCa  %%Owner: davidlu  */

ApplyGrpprlInvalCa( grpprl, cb, pca )
char *grpprl;
int cb;
struct CA *pca;
{
	extern struct RULSS vrulss;
	struct CA ca;

	ca = *pca;

/* flush any pending sprms in ruler sprm cache */
	if (vrulss.caRulerSprm.doc != docNil && grpprl[0] != sprmPRuler)
		FlushRulerSprms();

	PdodDoc(ca.doc)->fOutlineDirty = fTrue;

	InvalCp(&ca);

/* HACK! */
/* in order for this hack to work, we need to guarantee that the applied
	grpprl can never contain sprmCVanish, sprmCFldVanish or sprmPStc */
	Debug(GrpprlOKForFld(grpprl, cb));
	Assert(!vfNoInvalField);
	vfNoInvalField = fTrue;

/*  dirty any enclosing field result */
	InvalText (&ca, fFalse /* fEdit */);

	vfNoInvalField = fFalse;

	ApplyGrpprlCa( grpprl, cb, &ca );
}


/*  %%Function: GrpprlOKForFld  %%Owner: peterj  */

#ifdef DEBUG
GrpprlOKForFld(pgrpprl, cb)
char *pgrpprl;
int cb;
{
	int cch;
	while (cb)
		{
		switch (*pgrpprl)
			{
		case sprmCFVanish:
		case sprmCFFldVanish:
		case sprmPStc:
			Assert(fFalse);
			}
		cch = CchPrl(pgrpprl);
		pgrpprl += cch;
		cb -= cch;
		}
}


#endif /* DEBUG */


/*
*  msrun.c  --  Exec stuff swiped from Win Excel, 4/87    dsb
*/

#define CchSzNotBogus(sz)  (CchSz(sz) - 1)

extern CHAR             szEmpty[];
extern CHAR             stEmpty[];

/* F O U R  R U N  A P P */
/*  Run Application.  pchRun is a file followed by the arguments to the
	command.  sw is the Show Window code indicating how to bring the
	other app up.  Returns TRUE if the call succeeded.
*/
/*  %%Function: FOurRunApp   %%Owner: peterj  */

FOurRunApp (pchRun, sw, eid, fSingleAppOk)
CHAR *pchRun;
int sw;
int eid;
BOOL fSingleAppOk;

{
	int w;
	extern BOOL vfSingleApp;

	if (!fSingleAppOk && vfSingleApp)
		{
		ErrorEid(eidRuntimeWindows, " FOurRunApp");
		return fFalse;
		}

	/* reduce the amount of memory we use */
	ShrinkSwapArea();

	if ((w = RunApp (pchRun, sw)) == 8)
		/* out of memory */
	/* other errors are reported already (or caller's responsibility) */
		ErrorNoMemory(eid);

	/*  restore our swap area */
	GrowSwapArea();

	/* DOS error codes are always less than 32 */
	return w >= 32;
}


/**** RunApp - run command procedure.
		This proc tries to execute (or load) the given string accordingly
		to the given iEvent. (can be any SW_ code)
		Returns a DOS application handle ( < 32 indicates error).
*/
/*  %%Function: RunApp   %%Owner: peterj  */

int RunApp (pchRun, iEvent)
CHAR *pchRun;
int  iEvent;

{
	extern BOOL vfSingleApp;
	CHAR *pch1, *pch2;	       /* 2 multi-usage string pointers */
	CHAR *pchCmdLine;	       /* ptr Command Line (Arguments) */
	int  iDone; 	       /* Dos return value */
	int  cch;		       /* Character count */
	int  fcb1buf[2];	       /* ??????????????  */
	unsigned int iOkay;        /* Dos return value when EXECuting */
	CHAR szExt[4];	       /* File extension string */
	CHAR szFile[128];	       /* Filename string */
	CHAR szBuf[128];	       /* Buffer string for Dos Exec */
	CHAR szArgs[128];	       /* Arguments string */
	CHAR *pchBuf;	       /* ptr into szBuf or rgchArgs */
	CHAR *pchArgs;	       /* ptr into rgchArgs or szBuf */
	struct LOADPARMS 
		{
		WORD segEnv;	       /* Env. segment */
		LPSTR lpCmdLine;       /* Arguments long ptr */
		LPSTR lpFCB1;	       /* FCBs long ptr */
		LPSTR lpFCB2;
		} loadparms;	       /* Load parameters struct used by Dos */

	/* Get rid of leading blanks */
	while (*pchRun == ' ')
		pchRun++;

	/* Get arguments string. */
	pchCmdLine = PchParseNextFile(pchRun);

	/* Check FileName and fill szFile if OK. */
	if (!FCheckRunFile (szFile, pchRun))
		return(0);

	/* If szFile is our help app, we want to run it directly, ignoring any
		extension fields in WIN.INI.  First, we find the app name by
		traversing the string backwards from the end until we find a
		colon or backslash.  Then we compare the string to "WINWORD.HLP",
		and skip the WIN.INI extension field check if they match. */
	pch1 = szFile + CchSzNotBogus(szFile);
	while (pch1 >= szFile && *pch1 != '\\' && *pch1 != ':')
		pch1--;
	if (!FNeNcSz(++pch1, SzSharedKey("WINWORD.HLP",HelpFileDef)))
		{
		szBuf[0] = '\0';
		goto Lwinhelp;
		}

	/* Go Backward into the string until '.' and get extension.
		Remember CheckFileRun() adds extension and '.' is not a valid
		Kanji 2nd byte.
	*/
	pch1 = szFile + CchSzNotBogus(szFile);
	while (*pch1 != '.')
		pch1--;
	CchCopySz(++pch1, szExt);

	/* If extension field exists in WIN.INI replace current filename
		by new filename and if Skill character is found in the field
		add also the the previous filename:
		TEXT.DOC + doc=write.exe ^.doc ==> WRITE.EXE TEXT.DOC
		We also exchange ptrs on filename and arguments.
	*/
	if (GetProfileString((LPSTR)SzSharedKey("extensions",Extensions),
			(LPSTR)szExt, (LPSTR)szEmpty, (LPSTR)szBuf, 128))
		{
		pch1 = szArgs;
		CchCopySz(szBuf, pch1);
	/* '.' is not a valid Kanji 2nd byte but SKILLCHAR is, so...*/
		while (*pch1 && *pch1 != chSkill)
			pch1 = (CHAR *)LOWORD((long)AnsiNext((LPSTR)pch1));
		if (*pch1 == chSkill)
			{
			CchCopySz(szFile, pch1);
			while (*pch1)
				pch1++;
			while (*pch1 != '.')
				pch1--;
			pch2 = szBuf + CchSzNotBogus(szBuf);
			while (pch2 != szBuf && *pch2 != '.' && *pch2 != chSkill)
				pch2 = (CHAR *)LOWORD((long)AnsiPrev((LPSTR)szBuf, (LPSTR)pch2));
			if (*pch2 == '.')
				CchCopySz(++pch2, ++pch1);
			}
		pchBuf = szArgs;       /* Here takes place the ptrs Xchange */
		pchArgs = szBuf;
		}

	/* Otherwise run the file itself (if valid) */
	else
		{
Lwinhelp:
		pchBuf = szBuf; 	/* Just create plain ptrs */
		pchArgs = szArgs;
		CchCopySz(szFile, szBuf);
		}

	/* From now we can't use direct ptr to szBuf or szArgs because they
		can be xchanged, so use only the created ptrs: pchBuf and pchArgs.
		At this moment pchArgs points into a buffer whose content is not
		yet defined.
	*/
	SzSzAppend(pchBuf, SzShared(" "));
	cch = CchSzNotBogus(pchBuf);
	if (CchSzNotBogus(pchCmdLine) + cch > 127)
		*(pchCmdLine + 127 - cch) = 0;

	/* Concatenate everything */
	SzSzAppend(pchBuf, pchCmdLine);

	/* Go after the filename  and skip blanks (Kanji OK) */
	for (pch1 = pchBuf; *pch1 && *pch1 != '/' && *pch1 != ' '; ++pch1);
	pch2 = pch1;
	while (*pch1 == ' ')
		pch1++;

	/* Build a length in front carriage return terminated string
		for Arguments (Dos needs this way). */
	AnsiToOem((LPSTR)pch1, (LPSTR)pch1);
	*pchArgs = CchSzNotBogus(pch1);
	CchCopySz(pch1, pchArgs+1);
	*(pchArgs + 1 + *pchArgs) = chReturn;

	/* Fill LOADPARMS structure */
	fcb1buf[0] = 2;
	fcb1buf[1] = iEvent;
	loadparms.segEnv = 0;
	loadparms.lpCmdLine = (LPSTR)pchArgs;
	loadparms.lpFCB1 = (LPSTR)fcb1buf;
	loadparms.lpFCB2 = (LPSTR)NULL;

	/* Zero terminate application name */
	*pch2 = 0;

	/* Go backward from filename until ':' or '\'. */
/* FUTURE (pj): this loop seems totally useless since pch2 is never used... */
	while (*pch2 != ':' && *pch2 != '\\' && pch2 != pchBuf)
		pch2 = (CHAR *)LOWORD((long)AnsiPrev((LPSTR)pchBuf, (LPSTR)pch2));
	if (*pch2 == '\\' || *pch2 == ':')
		pch2++;

	QszUpper(pchBuf);
	AnsiToOem((LPSTR)pchBuf, (LPSTR)pchBuf);
	return(ShellExec(pchBuf, &loadparms));
}


/**** FCheckRunFile - Check if filename is a decent file to Run.
		pchSrc is an ANSI string and so pchDest should be.
		This routine deals with Kanji chars.

		The filename it returns must contain a '.'
*/
/*  %%Function: FCheckRunFile  %%Owner: peterj  */

BOOL FCheckRunFile(pchDest, pchSrc)
CHAR *pchDest;
CHAR *pchSrc;

{
	CHAR *pch;
	int  iter, cch;
	CHAR     szBuf[128];
	OFSTRUCT ofFile;

	/* Check if valid filename. */
	if (OpenFile ((LPSTR)pchSrc, (LPOFSTRUCT)&ofFile, OF_PARSE) == -1)
		return FALSE;

	pch = pchSrc + CchSzNotBogus(pchSrc);
	while (pch != pchSrc && *pch != '\\' && *pch != '.')
		pch = (CHAR *)LOWORD((long)AnsiPrev((LPSTR)pchSrc, (LPSTR)pch));

	/* Check if file exists as given */
	if (OpenFile ((LPSTR)pchSrc, (LPOFSTRUCT)&ofFile, OF_EXIST) != -1)
		{
		OemToAnsi((LPSTR)ofFile.szPathName, (LPSTR)pchDest);
	/*  Did the source filename have an extension? */
		if (*pch != '.')
			{
		/* No, did OpenFile stick one on for us? */
			pch = pchDest + CchSzNotBogus(pchDest);
			if (*(pch-1) != '.')
				{
			/*  No, well we better add one */
				*pch++ = '.';
				*pch   = 0;
				}
			}
		return(TRUE);
		}

	/* We didn't find file so look for extension separator '.'.
		If '.' is found it can't be an executable file (would have been
		found by previous OpenFile().  But i.e. it can be an argument like
		FOO.DOC and so asks for load WRITE.EXE. Therefore says it's OK.
	*/
	if (*pch != '.')
		/* file must have an extension */
		{
		ErrorEid(eidNoExtension, "FCheckRunFile");
		return (FALSE);
		}
	else  if (!FIsRunFile(pchSrc))
		{
		CchCopySz(pchSrc, pchDest);
		return(TRUE);
		}
	else
		/* file was not found, caller should report (?) */
		return(FALSE);
}


/**** PchParseNextFile -  zero terminate the first file in the list of files
		returns pointer to the next file in the list
*/
/*  %%Function: PchParseNextFile  %%Owner: peterj  */

PchParseNextFile(pchList)
CHAR *pchList;

{
	CHAR *pchNext;

	while (*pchList && *pchList != ' ' && *pchList != ',')
		++pchList;
	pchNext = pchList;
	if (*pchList)
		pchNext++;
	*pchList = 0;
	while (*pchNext == ' ' || *pchNext == ',')
		++pchNext;
	return(pchNext);
}


/**** FIsRunFile - Make sure File has runnable extension like
		EXE, COM, BAT or PIF.
		pchName should points into an Ansi string (Kanji OK).
*/
/*  %%Function: FIsRunFile  %%Owner: peterj  */

BOOL FIsRunFile(pchName)
CHAR *pchName;

{
	CHAR *pch;
	int cch;

	pch = pchName;
	QszUpper(pch);
	pch = pch + CchSzNotBogus(pch);
	while (*pch != '\\' && *pch != '.' && pch > pchName)
		pch = (CHAR *)LOWORD((long)AnsiPrev((LPSTR)pchName, (LPSTR)pch));
	if (*pch++ == '.')
		{
		cch = max(3,CchSzNotBogus(pch));
		if ( !FNeRgch(pch, SzFrame("EXE"), cch) ||
				!FNeRgch(pch, SzFrame("COM"), cch) ||
				!FNeRgch(pch, SzFrame("PIF"), cch) ||
				!FNeRgch(pch, SzFrame("BAT"), cch))
			return(TRUE);
		}
	return(FALSE);
}
