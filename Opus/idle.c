/* I D L E . C */

#define RSHDEFS
#define NOGDICAPMASKS
#define NOWINSTYLES
#define NOSYSMETRICS
#define NODRAWFRAME
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NOCTLMGR
#define NODRAWTEXT
#define NOFONT
#define NOGDI
#define NOHDC
#define NOMENUS
#define NOMINMAX
#define NOOPENFILE
#define NOPEN
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#define NOKANJI

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "disp.h"
#include "props.h"
#include "ch.h"
#include "sel.h"
#include "doc.h"
#include "field.h"
#include "format.h"
#include "screen.h"
#include "print.h"
#include "file.h"
#include "layout.h"
#include "preview.h"
#include "status.h"
#define AUTOSAVE
#include "autosave.h"
#include "keys.h"
#include "debug.h"
#include "profile.h"
#include "insert.h"
#include "prompt.h"
#include "idle.h"
#include "menu2.h"
#include "el.h"
#include "core.h"
#include "dde.h"
#include "macrocmd.h"
#include "message.h"
#include "resource.h"
#include "ruler.h"
#include "scc.h"



/* G L O B A L S */
/* vhprc is the head of the list of PRC (unique lists of sprms) kept in heap
	storage */
struct PRC      **vhprc;
char            **vhgrpchr;



/* E X T E R N A L S */

extern int              vwWinVersion;
extern int              fnMac;
extern struct BPTB      vbptbExt;
extern int              vfSysMenu;
extern MES		** vhmes;
extern OTD		** vhotd;
extern HANDLE           vhInstance;
extern HMENU            vhMenu;
extern int              viMenu;
extern int              wwMac;
extern int              wwCur;
extern int              vdocHdrEdit;
extern struct MWD       **hmwdCur;
extern struct WWD       **hwwdCur;
extern struct WWD       **mpwwhwwd[];
extern struct DOD       **mpdochdod[];
extern struct SEP       vsepFetch;
extern struct FLI       vfli;
extern struct CA        caSect;
extern struct PAP       vpapFetch;
extern struct CHP       vchpFetch;
extern struct PREF      vpref;
extern struct PRSU      vprsu;
extern int              docMac;
extern int              mwMac;
extern CP               vcpFetch;
extern int              vccpFetch;
extern struct CA        caPara;
extern long             vusecTimeoutPrompt;
extern struct PPR     **vhpprPRPrompt;
extern HCURSOR		vhcPrvwCross;
extern HCURSOR		vhcOtlCross;
extern HCURSOR          vhcHelp;
extern int              vbchrMac;
extern int              vbchrMax;
extern int              vdocScratch;
extern int  	    	vdocTemp;
extern struct SCC       vsccAbove;
extern struct SEL       selCur;
extern struct SEL       selDotted;
extern int              vxpFirstMark;
extern struct CA        caPage;
extern int              vipgd;
extern int              vised;
extern CP               cpInsert;
extern BOOL             vfHelp;
extern BOOL             vfInsertMode;
extern BOOL             vfAwfulNoise;
extern struct MERR      vmerr;
extern struct SAB       vsab;
extern struct AAB       vaab;
extern BOOL             vfSeeSel;
extern int              vfRestartDisplay;
extern int              vfShowAllStd;
extern int              vfLastScrollDown;
extern int              vfnKeepInMem;
extern PN               vpnKeepNext;
extern struct BPTB      vbptbExt;
extern int              fnFetch;
extern struct SCI       vsci;
extern struct BMI       vbmiEmpty;
extern struct PRI       vpri;
extern struct RULSS     vrulss;
extern BOOL             vfDdeIdle;
extern ASD		asd;
extern long		dtickCaret;
extern int		vfFocus;
extern int 		vflm;
extern MSG 		vmsgLast;   /* WINDOWS: last message gotten */
extern int		vcConseqScroll;
extern int		vfDrawPics;
extern int              vfnPreload;
extern int              vcPreload;
extern IDF              vidf;
extern int              vgrfMenuCmdsAreDirty;
extern int              vgrfMenuKeysAreDirty;
extern PVS		vpvs;

#ifdef DEBUG
extern int              cHpFreeze;
extern struct DBS vdbs;
extern HDC             	hdcPrint;
#endif /* DEBUG */

/* Following three lines implement an enhanced interface to the
	Windows call GetInputState.  GetInputState provides a latch
	indicating whether an input event has occurred; the latch gets cleared
	when it is read.  The following lines provide:

			FGetInputState() - like GetInputState, but does not clear latch
			ClearInputState() - manually clears latch
*/

int fInputStateLatch = fFalse;

ASD      asd;

#define DEFVARS
#include "rareflag.h"

#define FGetInputState()    (fInputStateLatch || (fInputStateLatch = GetInputState()))
#define ClearInputState()   (fInputStateLatch = fFalse)

extern int  vcInNewCurWw; /* to avoid badness if nested in WM_MOUSEACTIVATE */

struct SEL *PselActive();
CP CpLimNoSpaces(struct CA *);

/* F  I D L E */
/* %%Function:FIdle %%Owner:PETERJ */
FIdle()
{ /* Idle routine -- executes in the background when no messages are waiting */
	/* Returns fTrue iff there is a message in the queue on return */

	/* WARNING: in Preview mode, we skip everything except:
				Enable AwfulNoise
				OnTime Macro
				Updating Menus
				Initializing SDM
				Autosave
				Debug Checks
		Due to the special nature of the font cache while in preview, we cannot
		do FormatLines except while in the preview code!
	*/
	extern int vfDeactByOtherApp;
	extern int vlm;
	extern HWND vhwndRibbon;
	extern HWND vhwndStatLine;
	struct RPL rpl;
	struct LBS lbsText, lbsFtn;
	int ww;
	int wwHdr = wwNil;
	int docCur = PselActive()->doc;
	static int cPassUninterrupt = 0;
#define cPassOne    5
#define cPassTwo    20

#ifdef DEBUG
	int iGotoLRet = -1;
#endif /* DEBUG */

	Assert(vidf.fInIdle == 0);
	vidf.fInIdle = fTrue;

#ifdef RSH
	RestartUatTimer();
	CheckUat();
#endif /* RSH */

	Assert(vpvs.lmSave != lmPreview);
#ifdef BATCH
	/* If vfBatchIdle is true, allow idle-ing in batch mode */
	if (vfBatchMode && !vfBatchIdle)
		BatchModeError(SzShared("Idle entered!"), NULL, 0, 0);
#endif /* BATCH */

/* if free heap space has changed, dump the new cbFree to the comm port */
	Debug (vdbs.fReportHeap ? ReportHeapSz (SzShared("memory usage "), 1) : 0);
	Debug (vdbs.fHeapAct||vdbs.fWinAct ? ReportFailCalls() : 0);

#ifdef DEBUG
/* show each pass through idle */
		{
		static int cIdlePasses = 0;

		Scribble (ispIdle1, 'I'); /* indicate in idle */
		Scribble (ispIdle2, '0'+cIdlePasses);
		cIdlePasses = (cIdlePasses + 1) % 10;
		}

	if (vdbs.fShowDirty)
		ScribbleDirty(DocMother(selCur.doc));
#endif  /* DEBUG */

	Assert( cHpFreeze == 0 );
	Debug( cHpFreeze = 0 );
	Assert( vfInsertMode || cpInsert == cpNil );
	Assert(vcPreload == 0 && vfnPreload == fnNil);

	ClearInputState();

	if (vidf.fInvalSeqLev)
		{
		InvalSeqLevFields();
		Assert(!vidf.fInvalSeqLev);
		}

/* Re-enable beep, disk full alert, memory alert, other alerts */
	vfAwfulNoise = fFalse;

	if (vlm == lmPreview)
		goto LPrvw1;

/* Reset the cound of consecutive scrolls. */
	if (vcConseqScroll != 0 &&
			GetKeyState(VK_UP) >= 0 && GetKeyState(VK_DOWN) >= 0 &&
			GetKeyState(VK_LBUTTON) >= 0 && GetKeyState(VK_RBUTTON) >= 0)
		{
		vcConseqScroll = 0;
		}

	Assert(vlm != lmPreview);
/* Update all document windows */
	UpdateAllWws( fTrue );

	Debug (iGotoLRet = 0);
	if (FMsgPresent(mtyIdle & ~mtyMouseMove))
		goto LRet;

/* scroll active cp of selection into view */
	if (vfSeeSel && hwwdCur != hNil)
		SeeSel();
	vfSeeSel = fFalse;

	Debug (iGotoLRet = 1);
	if (FMsgPresent(mtyIdle))
		goto LRet;

	Assert(vlm != lmPreview);
/* turn on selection highlight */
	if (!vfDeactByOtherApp)
		{
		if (!selCur.fNil && selCur.fHidden)
			TurnOnSelCur();
		if (!selDotted.fNil && selDotted.fHidden)
			TurnOnSel(&selDotted);
		}

	Debug (iGotoLRet = 2);
	if (FMsgPresent(mtyIdle))
		goto LRet;

	Assert(vlm != lmPreview);

/* make sure we have our full swap area (or reduce if we have used up memory) */
	if (!(cPassUninterrupt+1 & 0x00ff))
		{
#ifdef DEBUG
		if (vcShrinkSwapArea != 0)
			ReportSz("vcShringSwapArea non zero at idle");
#endif /* DEBUG */
		vcShrinkSwapArea = 0;
		OurSetSas(sasFull);
		}

/* bring in our core (wait until now so screen is up to date & we don't
	postpone our responsiveness) */
	if (cPassUninterrupt > cPassOne)
		CoreNewest (cPassUninterrupt >= cPassTwo/*fLoad*/);

	Debug (iGotoLRet = 3);
	if (FMsgPresent(mtyIdle))
		goto LRet;

/* update ruler */
	if (hmwdCur != hNil && (*hmwdCur)->hwndRuler != hNil)
		if (selCur.fUpdatePap || selCur.fUpdateRuler)
			UpdateRuler(hmwdCur, fFalse /*!fInit*/, -1/*rk*/, fTrue /*fAbortOK*/);

	Debug (iGotoLRet = 4);
	if (FMsgPresent(mtyIdle))
		goto LRet;

	Assert(vlm != lmPreview);
/* update outline iconbar if in outline. */
	if (hwwdCur != hNil && (*hwwdCur)->fOutline)
		{
		UpdateOutlineIconBar();
		}

	Debug (iGotoLRet = 5);
	if (FMsgPresent(mtyIdle))
		goto LRet;

/* Update the beautification of edited text in the various macro windows */
	if (vhmes != hNil)
		{
		uns imei;
		uns imeiMax;
		MEI * pmei;
		MES * pmes;
		struct CA ca;

		pmes = *vhmes;
		pmei = &pmes->rgmei[0];
		imeiMax = pmes->imeiMax;
		for (imei = 0; imei < imeiMax; ++imei, ++pmei)
			/* okay to ignore expansion failure 
				as beauty is at stake */
			if (pmei->fNotExpanded && !DiDirtyDoc(pmei->docEdit))
				{
				if (FExpandHqrgbToPca(pmei->hqrgbTokens, 
						pmei->cbTokens, 
						PcaSetWholeDoc(&ca,pmei->docEdit),imei,fTrue))
					PmeiImei(imei)->fNotExpanded = fFalse;
				pmei = PmeiImei(imei);
				}
		}

/* Update Start/Continue button on macro icon bar */
	{
	MEI * pmei;
	
	if (vhmes != hNil && (pmei = PmeiCur())->heli != hNil && 
			PdodDoc(pmei->docEdit)->fDirty)
		{
		FreeEditMacro(iNil);
		}
	}

	Debug (iGotoLRet = 6);
	if (FMsgPresent(mtyIdle))
		goto LRet;

/* notice editing in header pane, enable "link with prev/reset" button if 
	it is not the first section's header/footer */
/* Why is this not done at NewCurWw time? - because you don't know when it 
is dirtied */
	if (wwCur != wwNil && PwwdWw(wwCur)->wk == wkHdr && 
			PdodDoc(docCur)->fDirty)
		{
		Assert(PdodDoc(docCur)->fDispHdr);
		if (FCanLinkPrev(wwCur))
			{
			SetSameAsPrevCtl(wwCur, fTrue /*fEnable*/);
			}
		}

	Assert(vlm != lmPreview);
/* update ribbon */
	if (vhwndRibbon != NULL)
		if (selCur.fUpdateChp || selCur.fUpdateChpGray ||
				selCur.fUpdateRibbon)
			UpdateRibbon(fFalse);

	Debug (iGotoLRet = 7);
	if (FMsgPresent(mtyIdle))
		goto LRet;

	Assert(vlm != lmPreview);

/* update header window's iconbar text */
	if (vdocHdrEdit != docNil)
		{
		int docHdrDisp = docNil;
		int docMom = vdocHdrEdit; /* saved away because vdocHdrEdit may
		be reset to zero */
		while ((wwHdr = WwHdrFromDocMom(docMom, &docHdrDisp)) != wwNil)
			{
			Assert(docHdrDisp);
			if (PwwdWw(wwHdr)->fHdr)
				SetHdrIBarName(docHdrDisp, IhtstFromDoc(docHdrDisp), wwHdr);
			}
		}

	Debug (iGotoLRet = 8);
	if (FMsgPresent(mtyIdle))
		goto LRet;

	Assert(vlm != lmPreview);

/* perform background DDE processes, if any */
	if (vfDdeIdle)
		{
		DoDdeIdle ();
		}

	Debug (iGotoLRet = 9);
	if (FMsgPresent(mtyIdle))
		goto LRet;

	/* WwCur enhancement for pictures. If window was dirtied and there
		are lines with pictures whose display was postponed, do the
		full display now. Other windows done later.
	
		Can use this loop for gray-scale fonts too.
	
	*/

	Assert(vlm != lmPreview);
	if (hwwdCur != hNil && (*hwwdCur)->fNeedEnhance)   /* wwCur first */
		{
		UpdateWw(wwCur, fTrue /* fAbortOK */); /* be sure window is up to date first */
		if (FEnhanceWw(wwCur))
			(*hwwdCur)->fNeedEnhance = fFalse;
		else  /* false if interrupted */				
			{
			Debug (iGotoLRet = 10);
			goto LRet;
			}
		}

	Debug (iGotoLRet = 11);
	if (FMsgPresent(mtyIdle))
		goto LRet;

/* Update the status line */
	if (vhwndStatLine != NULL && 
			(selCur.fUpdateStatLine || selCur.fUpdateStatLine2))
		UpdateStatusLine(usoCache);
	Debug (iGotoLRet = 12);
	if (FMsgPresent(mtyIdle))
		goto LRet;

/* preload font, if changed, to avoid delay when typing commences */
	if (vrf.fPreloadSelFont)
		{
		vrf.fPreloadSelFont = fFalse;
		if (selCur.fIns && hwwdCur != hNil &&
				DocMother(vfli.doc) == DocMother(selCur.doc))
	/* one case where vfli.doc not equal to selCur.doc is we are stepping
	through macro statement in an active window.  vfli.doc may point to the
	macro doc because we just hilighted the macro statement but selCur.doc
	is the active doc that the macro is executing on.
*/
			{
#ifdef BRYANL
			CommSz( SzShared( "Preloading font!!\r\n") );
#endif
			SetFlm( (*hwwdCur)->grpfvisi.flm );
			LoadFont( &selCur.chp, fFalse /*fWidthsOnly*/ );
			}
		}
	Debug (iGotoLRet = 12);
	if (FMsgPresent(mtyIdle))
		goto LRet;

LPrvw1:

/* Check for OnTime macro */
	if (vhotd != hNil)
		{
		extern unsigned long LTimeCur();
		OTD * potd;
		unsigned long lTime;

		lTime = LTimeCur();
		potd = *vhotd;
		if (lTime >= potd->lTime && 
				lTime < potd->lTime + potd->lTolerance)
			{
			extern BOOL vfAbortInsert;

			if (vfInsertMode)
				{
LAbortInsert:
				Assert(vfInsertMode); /* protect the label */
				/* Have to run next time in idle... */
				vfAbortInsert = fTrue;

				/* need to idle again ASAP! */
				Scribble (ispIdleComplete, 'M'); /* successful idle  */
				Scribble (ispIdle1, ' '); /* no longer in idle */
				cPassUninterrupt++;
				vidf.fInIdle = fFalse;
				return fFalse;
				}
			else
				{
				char stMacro [cchMaxSt];

				CopySt(potd->stMacro, stMacro);
				FreePh(&vhotd);
				FRunMacro(stMacro);
				}
			}
		}


	Debug (iGotoLRet = 13);
	if (FMsgPresent(mtyIdle))
		goto LRet;

/* make sure all the menus are up to date */
	if ((vgrfMenuCmdsAreDirty || vgrfMenuKeysAreDirty) && 
			cPassUninterrupt > cPassTwo)
		BringMenusUpToDate();

	Debug (iGotoLRet = 14);
	if (FMsgPresent(mtyIdle))
		goto LRet;

	if (cPassUninterrupt > cPassTwo+1 && 
			(!vmerr.fSdmInit||vpvs.fLoadPrvwFon))
		{
		FAbortNewestCmg(cmgInitStuff, fFalse, fTrue);
/* init SDM if we haven't already */
		if (!vmerr.fSdmInit)
			FAssureSdmInit();
		Debug (iGotoLRet = 15);
		if (FMsgPresent(mtyIdle))
			goto LRet;
/* Add preview font to master font table */
		if (vpvs.fLoadPrvwFon)
			LoadPreviewFont();
		OldestCmg(cmgInitStuff);
		}


	Debug (iGotoLRet = 16);
	if (FMsgPresent(mtyIdle))
		goto LRet;

	if (vlm == lmPreview)
		goto LPrvw2;

/* perform background repagination */
	if (selCur.doc != docNil && cPassUninterrupt >= cPassOne)
		{
		if (vpref.fBkgrndPag && !vpref.fDraftView 
				&& PwwdWw(wwCur)->wk != wkMacro)
			{
			int docMother = DocMother(selCur.doc);
			int lmSave = vlm;
			int flmSave = vflm;

			if (FInitWwLbsForRepag(wwCur, docMother, lmBRepag, &lbsText, &lbsFtn))
				{
				int fAbort, ipgdMacOld;

				SetWords(&rpl, pgnMax, cwRPL);
				rpl.cp = cpMax;
				ipgdMacOld = IMacPlc(PdodDoc(docMother)->hplcpgd);
				fAbort = !FUpdateHplcpgd(wwTemp, docMother, &lbsText, &lbsFtn, &rpl, patSilent );
				EndFliLayout(lmSave, flmSave);
				EndLayout(&lbsText, &lbsFtn);
				Debug (iGotoLRet = 17);
				if (fAbort)
					goto LRet;
				if (vhwndStatLine && ipgdMacOld !=
						IMacPlc(PdodDoc(docMother)->hplcpgd))
					UpdateStatusLine(usoCache);
				}
			}

		Debug (iGotoLRet = 18);
		if (FMsgPresent(mtyIdle))
			goto LRet;

		CoreNewest(fFalse);
		}


	Debug (iGotoLRet = 19);
	if (FMsgPresent(mtyIdle))
		goto LRet;

/* Get rid of the monster printer DC if we can live without it */
	if (vpri.hdc)
		{
		SetFlm( flmTossPrinter );
		Debug(hdcPrint = NULL);
		}

	Debug (iGotoLRet = 20);
	if (FMsgPresent(mtyIdle))
		goto LRet;

/* To improve global memory conditions, unlock any locked bitmaps */
	if (vsci.hdcScratch)
		SetScratchPbmi( &vbmiEmpty );

/* Shrink heap blocks for FormatLine.  Call this when it's
possible that the contents of the screen have gotten less complex */
	Assert(vbchrMax >= bchrMaxInit);
	if (vbchrMax - vbchrMac > bchrMaxFree)
		{
		vbchrMax = max(bchrMaxInit,vbchrMac);
		FChngSizeHCw(vhgrpchr, CwFromCch(vbchrMax + cbCHRE), fTrue);
		}

/* inside ruler functions the ruler grpprl is never shrunk.  This
	checks to see if it is bigger then it needs to be & shrinks it */
	if (vrulss.hgrpprl != hNil)
		CheckRulerGrpprlSize ();

	Assert (vdocTemp == docNil || mpdochdod[vdocTemp] != hNil);
	Assert (vdocScratch == docNil || mpdochdod[vdocScratch] != hNil);

	Assert(vlm != lmPreview);

	Debug (iGotoLRet = 21);
	if (FMsgPresent(mtyIdle))
		goto LRet;

/* delete vdocScratch here to free heap */
	if (vdocScratch != docNil)
		{
#ifdef DEBUG
		extern BOOL fDocScratchInUse;
		Assert (!fDocScratchInUse);
#endif /* DEBUG */
		DisposeDoc (vdocScratch);
		/* vdocScratch = docNil; */
		}
	if (vdocTemp != docNil && CpMacDocEdit(vdocTemp) > cp0)
		{
		struct CA caT;
		PdodDoc(vdocTemp)->doc = docScrap;   /* ensure valid chain */
		FDelete(PcaSetWholeDoc(&caT, vdocTemp));
		}

LPrvw2:
	Debug (iGotoLRet = 22);
	if (FMsgPresent(mtyIdle))
		goto LRet;

/* check to see if autosave should be done.  If so, do it. */
	Assert(GetTickCount() > 0L);
	if (vpref.iAS == iASNever || mwMac == 0 || !FAnyDirtyDocs())
		ResetASBase();
	else if (cPassUninterrupt > cPassTwo && FDoAutoSave())
		{
		if (vfInsertMode)
			/* must exit insert mode before we can save */
			goto LAbortInsert;
		else
			CmdAutosave();
		}

	Debug (iGotoLRet = 23);
	if (FMsgPresent(mtyIdle))
		goto LRet;

	/* check for discardable cursors */
	if (vhcOtlCross)
		{
		int ww;
		for (ww = 0; ww < wwMac; ww++)
			if (mpwwhwwd[ww] != hNil && PwwdWw(ww)->fOutline)
				break;
		if (ww == wwMac)
			/* no one in outline mode, discard outline cursors */
			{
			FreeIrcds(ircdsOtlCross);
			FreeIrcds(ircdsOtlVert);
			FreeIrcds(ircdsOtlHorz);
			}
		}
	if (vhcPrvwCross && vlm != lmPreview)
		FreeIrcds(ircdsPrvwCross);

	Assert(!vhcHelp == !vfHelp);

	Debug (iGotoLRet = 24);
	if (FMsgPresent(mtyIdle))
		goto LRet;

	if (vlm == lmPreview)
		goto LPrvw3;

	if (cPassUninterrupt > cPassTwo && selCur.doc != docNil)
		/* load pieces of selCur.doc into file cache */
		{
		int doc = DocMother(selCur.doc);
		int fn = PdodDoc(doc)->fn;
		CP ccpScan = CpMin((vbptbExt.ibpMac / 4) * cbSector, 
				CpMacDoc(doc));
		CP cpCur = doc != selCur.doc ? cp0 :
				CpMax(cp0, selCur.cpFirst - (ccpScan/3));
		if (fn != fnNil)
			EnablePreload(fn);
		while (ccpScan > cp0 && cpCur < CpMacDoc(doc) && 
				!FMsgPresent(mtyIdle))
			{
			CachePara(doc, cpCur);
			FetchCp(doc, cpCur, fcmProps+fcmChars);
			cpCur += vccpFetch;
			ccpScan -= vccpFetch;
			}
		if (fn != fnNil)
			DisablePreload();
		}

	Debug (iGotoLRet = 25);
	if (FMsgPresent(mtyIdle))
		goto LRet;

	/* GlobalWire the current font (leaves it unselected) 
		(DavidW says DON'T try to access the bits under win3) */
 	if (vwWinVersion < 0x0300)
		{
		int f;
		HFONT hfont;
		HANDLE hfontPhy;
		HDC hdc;

		if (vsci.pfti != NULL && (hfont = vsci.pfti->hfont) != NULL && 
				hwwdCur != hNil && (hdc = (*hwwdCur)->hdc) != NULL)
			{
			hfontPhy = GetPhysicalFontHandle(hdc);

			ResetFont(fFalse);

			if (((f = GlobalFlags(hfontPhy)) & GMEM_LOCKCOUNT) == 0 &&
					(f & ~GMEM_LOCKCOUNT) != 0)
				{
				Scribble(ispWireFont, 'W');
				/* Make the block non-discardable so that
				   windows won't throw out the block when
				   making room to GlobalWire it. */
				Assert(f & GMEM_DISCARDABLE);
				if (f & GMEM_DISCARDABLE)
					GlobalReAlloc(hfontPhy, GlobalSize(hfontPhy),
						GMEM_MODIFY);
				GlobalWire(hfontPhy);
				if (f & GMEM_DISCARDABLE)
					GlobalReAlloc(hfontPhy, GlobalSize(hfontPhy),
						GMEM_MODIFY | GMEM_DISCARDABLE);
				GlobalUnlock(hfontPhy);
				}
			}
		}

	/*  make sure temp files used (especially TIFF files, etc) won't cause
		us to run out of fns. */
	if (fnMac > fnMax-5 && cPassUninterrupt > cPassTwo)
		KillExtraFns();

	Debug (iGotoLRet = 26);
	if (FMsgPresent(mtyIdle))
		goto LRet;

	/* Enhancement loop for pictures. If window was dirtied and there
		are lines with pictures whose display was postponed, do the
		full display now.
	
		Did wwCur above before background repag; should not now be dirty 
		Can use this loop for gray-scale fonts too.
	
	*/

	Assert(vlm != lmPreview);

	if (cPassUninterrupt > cPassOne)
		{
		if (hwwdCur != hNil && (*hwwdCur)->fNeedEnhance && (*hwwdCur)->fDirty)
			UpdateWw(wwCur, fTrue /* fAbortOK */);

		for (ww = wwDocMin; ww < wwMac; ++ww)
			{
			struct WWD **hwwd;
			if (((hwwd = mpwwhwwd[ww]) != hNil) && (*hwwd)->fNeedEnhance)
				{
				if (FEnhanceWw(ww))
					(*hwwd)->fNeedEnhance = fFalse;
				else  /* false if interrupted */							
					{
					Debug (iGotoLRet = 27);
					goto LRet;
					}

				}
			}
		}

LPrvw3:
	Debug (iGotoLRet = 28);
	if (FMsgPresent(mtyIdle))
		goto LRet;

/* debugging checks */
#ifdef DEBUG
/* Check to see if the toolbox has been trashed */
	Debug(vdbs.fCkTlbx ? CkTlbx(fFalse) : 0 );

/* following Asserts try to catch storing into NULL pointers */
	Assert( *((char *) 0) == 0 );
	Assert( *((char *) 2) == 0 );
	Assert( *((char *) 8) == 0 );

	if (cPassUninterrupt > cPassOne)
		DoDebugTests(fTrue);
#endif  /* DEBUG */

	Debug (iGotoLRet = 29);
	if (FMsgPresent(mtyIdle))
		goto LRet;

	if (vlm == lmPreview)
		goto LPrvw4;

/* quietly load each of the code segments we use */
	if (!(cPassUninterrupt+1 & 0x0007))
		GetNextHcd();

	Debug (iGotoLRet = 30);
	if (FMsgPresent(mtyIdle))
		goto LRet;

/* make sure our core is loaded in */
	CoreNewest (fFalse);

LPrvw4:
	Scribble (ispIdleComplete, '*'); /* successful idle  */
	Scribble (ispIdle1, ' '); /* no longer in idle */
	cPassUninterrupt++;
	vidf.fInIdle = fFalse;
	return fFalse;

LRet:   /* Idle was interrupted by the arrival of an interesting message */

/* show each interrupted pass through idle with indicator of how far through */
	Scribble (ispIdleComplete, 'a'+iGotoLRet);
	Scribble (ispIdle1, ' '); /* no longer in idle */
	cPassUninterrupt = 0;
	vidf.fInIdle = fFalse;
	return fTrue;
}


#ifdef DEBUG

/* %%Function:DoDebugTests %%Owner:PETERJ */
DoDebugTests(fAbortOK)
BOOL fAbortOK;
{

/* Check to see if the toolbox has been trashed */
	Debug(vdbs.fCkTlbx ? CkTlbx(fFalse) : 0 );

	if (fAbortOK && FMsgPresent(mtyIdle))
		return;

/* following Asserts try to catch storing into NULL pointers */
	Assert( *((char *) 0) == 0 );
	Assert( *((char *) 2) == 0 );
	Assert( *((char *) 8) == 0 );

	if (vdbs.fCkHeap)
		CkFarHeaps();

	if (fAbortOK && FMsgPresent(mtyIdle))
		return;

	if (selCur.doc != docNil)
		Debug(vdbs.fCkDoc ? CkHplcHdd(DocMother(selCur.doc)) : 0);

	if (fAbortOK && FMsgPresent(mtyIdle))
		return;

	if (!vdbs.fQuicksave)
		{
		Debug(vdbs.fCkDoc ? CkDocs(fAbortOK) : 0);
		if (fAbortOK && FMsgPresent(mtyIdle))
			return;
		Debug(vdbs.fCkFn ? CkFns(fAbortOK) : 0);
		}

	if (fAbortOK && FMsgPresent(mtyIdle))
		return;

	Debug(vdbs.fCkFont ? CkFont() : 0 );

	if (fAbortOK && FMsgPresent(mtyIdle))
		return;

	Debug(vdbs.fCkWw ? CkWws() : 0);
	Debug(vdbs.fCkWw ? CkMws() : 0);

	if (fAbortOK && FMsgPresent(mtyIdle))
		return;

	Debug(vdbs.fCkScc ? CkScc(&vsccAbove) : 0);

	if (fAbortOK && FMsgPresent(mtyIdle))
		return;

	Debug(vdbs.fCkBptb ? CkPbptb(&vbptbExt, fAbortOK) : 0);

	if (fAbortOK && FMsgPresent(mtyIdle))
		return;

	Debug(vdbs.fCkLHRUpd ? CkLHRUpd() : 0);

	if (fAbortOK && FMsgPresent(mtyIdle))
		return;

	Debug(vdbs.fCkFldIdle ? CkFldIdle() : 0);

	if (fAbortOK && FMsgPresent(mtyIdle))
		return;

	Debug(vdbs.fCkText ? CkText (fAbortOK) : 0);

	if (fAbortOK && FMsgPresent(mtyIdle))
		return;

	Debug(vdbs.fCkTableSel ? CkTableSel() : 0);

	Assert(cHpFreeze == 0);
	Assert(!selCur.chp.fSpec);
	Assert(!selCur.chp.fRMark);
	Assert(!selCur.chp.fStrike);
}



#endif /* DEBUG */

#ifdef NOTUSED
/* D M S C  F R O M  M S C */
/* Returns milliseconds elapsed between mscBase and mscNow. */
/* %%Function:DmscFromMsc %%Owner:PETERJ */
LONG DmscFromMsc(mscBase, mscNow)
LONG	mscBase, mscNow;
{
#ifdef DBGDMSC
	CommSzLong(SzShared("DmscFromMsc: "),
			(mscNow < mscBase) ? ((0x7FFFFFFF - mscBase) + mscNow + 1) :
			(mscNow - mscBase));
#endif
	return (mscNow < mscBase) ? ((0x7FFFFFFF - mscBase) + mscNow + 1) :
			(mscNow - mscBase);
}
#endif /* NOTUSED */


/***************************/
/* F  M s g  P r e s e n t */
/* return if there is a message present within limits specified by mty, return
	fTrue and leave the message on the queue and in vmsgLast; else return fFalse.
	Some message types are absorbed (pulled off the queue) by this routine;
	see the code */
/* %%Function:FMsgPresent %%Owner:PETERJ */
NATIVE FMsgPresent( mty )
int mty;        /* mty is a grpf */
{
	extern int vlm;
	extern HWND vhwndStatLine;
	static int cCall = 0;

	Assert (cHpFreeze == 0);

#ifdef RSH
	CheckUat();
#endif /* RSH */

	if (vcInNewCurWw)   /* hack to avoid re-entrancy problems in Windows - rp */
		return fFalse;

/* FMsgPresent can move the heap (although rarely); to flush heap movement
	bugs, we always shake the heap here */

	Debug( vdbs.fShakeHeap ? ShakeHeap() : 0 );
	Debug( vdbs.fCkHeap ? CkHeap() : 0 );

	Assert(!(mty & mtyIgnoreMouse) || !(mty & mtyMouseMove));

	if (vidf.fCorrectCursor && (mty & mtyMouseMove))
		ChangeCursor(fTrue);

/* some callers will wish to force calls to PeekMessage, because
	FGetInputState only senses key and mouse events */

	if (((mty & mtySomePeek) && !(cCall++ & 0x0003)) || FGetInputState())
		{
		int pm;

		pm = (mty & mtyYield) ? PM_NOREMOVE : PM_NOREMOVE | PM_NOYIELD;
		while ( PeekMessage( (LPMSG) &vmsgLast, NULL, NULL, NULL, pm ))
			{       /* Filter uninteresting or easily handled events */

			fInputStateLatch = fTrue;

/* CASE 2: Messages we dispatch: pull off the queue, call DispatchMessage */
/* BL 8/8/87: Only dispatch keyups if we have the focus; this prevents
	inadvertent heap movement in fedt combo's, which occurs because
	list boxes take action on key up. */
			if (FCheckToggleKeyMessage((LPMSG) &vmsgLast)
					|| (vmsgLast.message == WM_KEYUP && vfFocus)
					|| (vmsgLast.message == WM_TIMER)
					|| ((uns)(vmsgLast.message - WM_DDE_FIRST) <=
					(WM_DDE_LAST - WM_DDE_FIRST)))
				{
				/* dispatch (below) */
				;
				}

/* cursor cannot be changed during some operations */
			else  if ((vmsgLast.message == WM_MOUSEMOVE)
					|| (vmsgLast.message == WM_SETCURSOR)
					|| (vmsgLast.message == WM_NCMOUSEMOVE))
				{
				if (mty & mtyMouseMove)
					/* dispatch (below) */
					;
				else  if (mty & mtyIgnoreMouse)
					{
					GetMessage((LPMSG)&vmsgLast, NULL, 0, 0);
					ChangeCursor(fFalse);
					continue;
					}
				else
					return fTrue;
				}

/* CASE 2: Messages we don't understand: leave on the queue & return TRUE */
			else
				{
				return fTrue;
				}

			GetMessage( (LPMSG) &vmsgLast, NULL, 0, 0 );
			 /* for win3, the setourkeystate call in FChecktoggle does
				not set up until the msg is removed from the queue. We
				should remove that call as useless later  bz
			 */
			SetOurKeyState();
			TranslateMessage( (LPMSG) &vmsgLast );
			DispatchMessage((LPMSG)&vmsgLast);

			}  /* while */
/* No message is waiting, so we can clear the bit */
		ClearInputState();
		}

	/* WARNING: DO NOT Blink while in preview mode...it does FormatLine,
		which is a no-no while in preview. */
	if (vlm == lmPreview)
		goto LPrvw1;

/* it makes no sense to say we should blink, but should not check for blink */
	Assert( !(mty & mtyDoBlink) || !(mty & mtyNoTimer) );

/* not really a message, but we have to blink.  Don't just unconditionally
	Blink() here because Blink-->ToggleSel...-->FormatLine moves the heap. */

	if (~mty & mtyNoTimer)
		{
		long tick = GetTickCount();

		if (selCur.tickOld + dtickCaret < tick)
			if (mty & mtyDoBlink)
				Blink();
			else
				return fTrue;

		if (vusecTimeoutPrompt != 0 && tick >= vusecTimeoutPrompt)
			if (mty & mtyDoBlink)
				RestorePrompt();
		}

LPrvw1:

	return fFalse;
}


/* B l i n k */
/* Blink the text cursor at appropriate intervals */
/* %%Function:Blink %%Owner:PETERJ */
Blink()
{
	extern int vfFocus;
	long tick;
	struct SEL *psel = PselActive();

	Debug( vdbs.fShakeHeap ? ShakeHeap() : 0 );

	if (psel->tickOld + dtickCaret >= (tick = GetTickCount()))
		return;

	if (!(*hwwdCur)->fDirty && psel->sk == skIns && vfFocus && !psel->fHidden)
		{
/* tick == 0 means new insert line. Start toggling in a while. */
		if (psel->fOn)
			{
			if (psel->tickOld != 0)
				DrawInsertLine( psel );
			}
		else
/* insert is off. Turn it on, then wait if tickOld == 0 */
			ToggleSel(psel, psel->cpFirst, psel->cpLim);
		}
	psel->tickOld = tick;
}



/* F  N E X T  M S G  F O R  I N S E R T  */
/* routine waits until next message/event registers. If the message/event
	can be handled by the insertion code, it returns fTrue. If the
	message/event signals a new keystroke, its kcm is returned in *pkcm
	and its character translation is returned in *pch. */
/* %%Function:FNextMsgForInsert %%Owner:PETERJ */
BOOL FNextMsgForInsert(ptlv)
TLV * ptlv;
{
	extern int vfAbortInsert, vfOvertypeMode;
	extern HWND vhwndStatLine;

	struct DOD **hdod = mpdochdod [PselActive()->doc];
	int fDirty;
	long tickNext;
	int kc;

/* perform idle functions */
LIdle:

#ifdef DISABLE
/* Delay for a bit before updating the screen, in case keys arrive */
/* REVIEW swaptune(pj): tune dawdle amount */
#define tickDawdle 500
	tickNext = GetTickCount() + tickDawdle;
	do
		{
		if (FMsgPresent( mtyTyping )) /* HEAP MOVES! */
			goto LHaveEvent;
		}   
	while (GetTickCount() < tickNext);
#endif /* DISABLE */

	InvalOtherWws(fFalse);
LMoreIdle:
	/*  HEAP MOVES! */
	if (FMsgPresent( mtyTyping ))
		goto LHaveEvent;

	for ( ;; )
		{
		int f;
#ifdef BOGUS
		Assert(vfInsertMode);
#endif /* BOGUS */
		vfSeeSel = fTrue;
		fDirty = (*hdod)->fDirty;
		/* dirty is temporarily forced to true */
		(*hdod)->fDirty = fTrue;
		f = FIdle();
		(*hdod)->fDirty = fDirty;
		if (vfInsertMode)
			InsertPostUpdate();
		if (f && PeekMessage( (LPMSG)&vmsgLast, NULL, NULL, NULL, 
				PM_NOREMOVE ))
			break;

/* idle can dirty window through background repag */
		if ((*hwwdCur)->fDirty)
			{
			UpdateWw(wwCur, fFalse);
			if (vfInsertMode)
				InsertPostUpdate();
			}
/* Treat asynchronous events for which Windows gives no synchronous warning:
		(1) Loss of focus (to another app)
	Also handle any Idle operations that cannot function during Insert Mode.
*/
		if (!vfFocus || vfAbortInsert)
			return fFalse;
		}

	ptlv->fInvalPgdOnInsert = fTrue;

LHaveEvent:
#ifdef BOGUS
	Assert(vfInsertMode);
#endif /* BOGUS */

	if (ptlv->fReturn || vfOvertypeMode)
		return fFalse;

	/* Examine the message that it is waiting */
	/* Turn it into a usable kc if possible */
	/* Allow messages for other windows to fall through and
		be translated */

	if (vmsgLast.hwnd != (*hwwdCur)->hwnd &&
			vmsgLast.hwnd != vhwndStatLine)
		return fFalse;/* A message we can't handle in Alpha mode */

	if (vmsgLast.message == WM_PAINT)
		{
/* since the status line paints the stuff we invalidated by posting
	Paint messages to itself, we dispatch the message and stay in the
	loop to avoid premature exit from the insert loop. */
		if (vmsgLast.hwnd == vhwndStatLine)
			{
			GetMessage((LPMSG) &vmsgLast, NULL, 0, 0);
			DispatchMessage((LPMSG)&vmsgLast);
			goto LMoreIdle;
			}
		return fFalse;
		}

	if (FIsKeyMessage((LPMSG) &vmsgLast))
		{
		if (vmsgLast.wParam & 0x8000) /* indicates we did xlation */
			{
			ptlv->ch = vmsgLast.wParam & 0x00ff;
			return fTrue;
			}

		switch (vmsgLast.message)
			{
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYUP:
			goto LMoreIdle;
			}
		}

	return fFalse;
}


csconst TMR   mpiAStmr[] =
	{
		/* This table controls autosave frequencies */
		/* frequency     Time range:Min,Max,DcpRange(K) */
		/* High */	TmrForMinMaxKdcp(10, 30, 16),
		/* Med */	TmrForMinMaxKdcp(20, 45, 24),
		/* Low */	TmrForMinMaxKdcp(45, 75, 32)
	};

#ifdef PCJ
BOOL vfDbgASNow = fFalse;
#endif /* PCJ */


/* F  D O  A U T O  S A V E */
/* Check if we are due for autosave */
/* %%Function:FDoAutoSave %%Owner:PETERJ */
FDoAutoSave()
{
	long mscNow = GetTickCount();

	Assert((sizeof(mpiAStmr) / sizeof(TMR)) == iASMac - 1);

#ifdef PCJ
	if (vfDbgASNow)
		{
		vfDbgASNow = fFalse;
		return fTrue;
		}
#endif /* PCJ */

	if (asd.mscPostpone != 0L)
		/* we are in postpone phase */
		return (asd.mscPostpone <= mscNow);
	else
		/* normal - check elapse time vs amount edited */
		{
		int el;
		/* good only if you dont call out */
		TMR far *qtmr = &mpiAStmr[vpref.iAS];
		long dcpRange = qtmr->kdcpRange * 1024;

		if (asd.dcpEdit > dcpRange || asd.dcpEdit < 0
#ifdef PCJ
				|| vdbs.fShowDirty
#endif /* PCJ */
				) /*(overflow)*/
			el = 0;
		else
			el = elMost - (int)((long)(asd.dcpEdit * elMost)/dcpRange);

		return (mscNow - asd.mscBase) > (qtmr->dmscMin + (el * qtmr->dmscDelta));
		}
}


/* F  A N Y  D I R T Y  D O C S*/
/* returns true iff any open doc is dirty, excluding vdocDde. */
/* %%Function:FAnyDirtyDocs %%Owner:PETERJ */
BOOL FAnyDirtyDocs()
{
	int           doc;
	struct DOD ** hdod;

	for (doc = docMinNormal; doc < docMac; doc++)
		{
		hdod = mpdochdod[doc];
		if (hdod == hNil || ((*hdod)->fn == fnNil && !(*hdod)->fUserDoc))
			{
			continue;
			}
		if (DiDirtyDoc(doc))
			{
			return fTrue;
			}
		}

	return fFalse;
}


/* R E S E T  A S  B A S E */
/* Reset the autosave timing base. */
/* %%Function:ResetASBase %%Owner:PETERJ */
ResetASBase()
{
	asd.dcpEdit	= 0L;
	asd.mscPostpone= 0L;
	asd.mscBase = GetTickCount();
}




/* B R I N G  M E N U S  U P  T O  D A T E */
/* %%Function:BringMenusUpToDate %%Owner:PETERJ */
BringMenusUpToDate()
{
	int imnu, imnuMac;
	int dimnu = vfSysMenu ? 1 : 0;
	HMENU hMenu;

	Assert(vhMenu != NULL);
	imnuMac = GetMenuItemCount(vhMenu) - dimnu;
	for (imnu = 0; imnu < imnuMac; imnu++)
		{
		if (FMsgPresent(mtyIdle))
			return;
		if ((hMenu = GetSubMenu(vhMenu, imnu+dimnu)) == NULL)
			continue;
		if (vgrfMenuCmdsAreDirty & (1 << imnu) && viMenu == iMenuLongFull)
			if (!FChangeAppMenu(hMenu, imnu))
				return;
		if (vgrfMenuKeysAreDirty & (1 << imnu))
			{
			int iItem, iItemMac = GetMenuItemCount(hMenu);
			BCM bcm;
			for (iItem = 0; iItem < iItemMac; iItem++)
				{
				if (FMsgPresent(mtyIdle))
					return;
				bcm = GetMenuItemId(hMenu, iItem);
				if ((int)bcm >= 0)
					SetBcmMenuKeys(hMenu, bcm, bcm);
				}
			vgrfMenuKeysAreDirty &= ~(1 << imnu);
			}
		}
	if (vfSysMenu && vgrfMenuKeysAreDirty & (1 << imnuDocControl) &&
			(hMenu = GetSubMenu(vhMenu, 0)) != NULL)
		{
		int iItem, iItemMac = GetMenuItemCount(hMenu);
		BCM bcm;
		for (iItem = 0; iItem < iItemMac; iItem++)
			{
			if (FMsgPresent(mtyIdle))
				return;
			bcm = GetMenuItemId(hMenu, iItem);
			if ((int)bcm >= 0)
				SetBcmMenuKeys(hMenu, bcm, bcm);
			}
		}

	/* completed successfully; prevent idle from calling us again */
	vgrfMenuKeysAreDirty = vgrfMenuCmdsAreDirty = 0;
}


#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Idle_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* Idle_Last() */
