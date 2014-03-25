/* C M D C O R E . C */
/*  Command functions that are part of the opus core. */

#define RSHDEFS
#define NOMENUS
#define NOKEYSTATE
#define NOSHOWWINDOW
#define NOSYSMETRICS
#define NOCOLOR
#define NOMEMMGR
#define NOMENUS
#define NOMINMAX
#define NOREGION
#define NOSCROLL
#define NOTEXTMETRIC
#define NOWINOFFSETS

#define NOSYSMETRICS
#define NOICON
#define NORASTEROPS
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOHDC
#define NOBRUSH
#define NOPEN
#define NOFONT
#define NOWNDCLASS
#define NOCOMM
#define NOSOUND
#define NORESOURCE
#define NOOPENFILE
#define NOWH
#define NOKANJI

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "props.h"
#include "disp.h"
#include "heap.h"
#include "prm.h"
#include "keys.h"
#include "doc.h"
#include "debug.h"
#include "sel.h"
#define RGLCD
#include "cmdlook.h"
#define RULER
#define REPEAT
#include "ruler.h"
#include "wininfo.h"
#include "ribbon.h"
#include "fontwin.h"
#include "prm.h"
#include "status.h"
#include "ch.h"
#include "screen.h"
#include "file.h"
#include "field.h"
#define FINDNEXT
#include "search.h"
#include "ibdefs.h"
#include "outline.h"
#include "style.h"
#include "cmd.h"
#include "cmdtbl.h"
#include "profile.h"
#include "rerr.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "el.h"
#include "border.h"
#include "rareflag.h"
#define SEARCHTMC
#include "tmc.h"

#include "search.hs"
CP CpFirstBlock();


/* G L O B A L S */
FNI		fni = {
	isNever, 0, 0};


/* Repeat Ruler/Ribbon formatting. */
RRF		rrf = {
	docNil, /* doc */
	fFalse, /* fSelMoved */
	fFalse, /* fDirty */
	fFalse, /* fNoClear */
	fFalse, /* fRibbon */
	fFalse, /* fTouched */
	fFalse, /* fBold */
	fFalse, /* fItalic */
	fFalse, /* fSCaps */
	kulNone,/* kul */
	iSSNormal,/* iSuperSub */
	0,	/* Spare. */
	ftcNil,	/* hstFont */
	0,	/* hstPoint */
	stcNil, /* stc */
	0,	/* cbGrpprl */
	hNil};	/* hgrpprl */


/* E X T E R N A L S */
extern HWND             vhwndStatLine;
extern BOOL             vfExtendSel;
extern BOOL             vfRecording;
extern KMP **           hkmpCur;
extern int              mwCur;
extern int              wwCur;
extern int              vfDefineStyle;
extern int              vfSearchRepl;
extern int              vdocStsh;
extern int              vstcStyle;
extern int              ibstFontDS;
extern struct MWD       **hmwdCur;
extern struct PREF      vpref;          /* eg 9/6 */
extern struct WWD       **hwwdCur;
extern struct SEL       selCur;
extern struct CHP	vchpGraySelCur;
extern struct PAP	vpapSelCur;
extern struct PAP	vpapGraySelCur;
extern HWND             vhwndApp;
extern HWND             vhwndRibbon;
extern struct AAB       vaab;
extern HWND             vhwndLHEditFont;
extern HWND             vhwndLHEditPoint;
extern int              vfIconBarMode;
extern CHAR             szEmpty[];
extern struct MWD **    mpmwhmwd[];
extern struct MERR      vmerr;
extern struct STTB      **vhsttbFont;
extern int              vfFileCacheDirty;
extern RRF              rrf;
extern struct UAB       vuab;
extern struct CHP       vchpFetch;
extern BOOL             vfBlockSel;
extern char HUGE        *vhpchFetch;
extern CP               vcpFetch;
extern BOOL             vfSeeSel;
extern int  	    	fnFetch;
extern int              vccpFetch;
extern struct PAP   	vpapFetch;
extern struct CA    	caPara;
extern int              vssc;
extern HWND             vhwndCBT;
extern struct TCC       vtcc;
extern CP               vmpitccp[];
extern struct CA	caTap;
extern SB               sbStrings;

/*  %%Function:  CmdBold  %%Owner:  bobz       */

CMD CmdBold(pcmb)
CMB * pcmb;
{
	DoLooks(ilcdBold, fFalse /*fUpdate*/, hNil);
	return cmdOK;
}


/*  %%Function:  CmdItalic  %%Owner:  bobz       */

CMD CmdItalic(pcmb)
CMB * pcmb;
{
	DoLooks(ilcdItalic, fFalse /*fUpdate*/, hNil);
	return cmdOK;
}


/*  %%Function:  CmdSmallCaps  %%Owner:  bobz       */


CMD CmdSmallCaps(pcmb)
CMB * pcmb;
{
	DoLooks(ilcdSmallCaps, fFalse /*fUpdate*/, hNil);
	return cmdOK;
}


/*  %%Function:  CmdHideText  %%Owner:  bobz       */


CMD CmdHideText(pcmb)
CMB * pcmb;
{
	DoLooks(ilcdVanish, fFalse /*fUpdate*/, hNil);
	return cmdOK;
}


/*  %%Function:  CmdULine  %%Owner:  bobz       */

CMD CmdULine(pcmb)
CMB * pcmb;
{
	DoLooks(ilcdKulSingle, fFalse /*fUpdate*/, hNil);
	return cmdOK;
}


/*  %%Function:  CmdDULine  %%Owner:  bobz       */

CMD CmdDULine(pcmb)
CMB * pcmb;
{
	DoLooks(ilcdKulDouble, fFalse /*fUpdate*/, hNil);
	return cmdOK;
}


/*  %%Function:  CmdWULine  %%Owner:  bobz       */

CMD CmdWULine(pcmb)
CMB * pcmb;
{
	DoLooks(ilcdKulWord, fFalse /*fUpdate*/, hNil);
	return cmdOK;
}


/*  %%Function:  CmdSuperscript  %%Owner:  bobz       */

CMD CmdSuperscript(pcmb)
CMB * pcmb;
{
	DoLooks(ilcdSuperscript, fFalse /*fUpdate*/, hNil);
	return cmdOK;
}


/*  %%Function:  CmdSubscript  %%Owner:  bobz       */

CMD CmdSubscript(pcmb)
CMB * pcmb;
{
	DoLooks(ilcdSubscript, fFalse /*fUpdate*/, hNil);
	return cmdOK;
}


/*  %%Function:  CmdParaLeft  %%Owner:  bobz       */

CMD CmdParaLeft(pcmb)
CMB * pcmb;
{
	DoLooks(ilcdParaLeft, fFalse /*fUpdate*/, hNil);
	return cmdOK;
}


/*  %%Function:  CmdParaRight  %%Owner:  bobz       */

CMD CmdParaRight(pcmb)
CMB * pcmb;
{
	DoLooks(ilcdParaRight, fFalse /*fUpdate*/, hNil);
	return cmdOK;
}


/*  %%Function:  CmdParaCenter  %%Owner:  bobz       */

CMD CmdParaCenter(pcmb)
CMB * pcmb;
{
	DoLooks(ilcdParaCenter, fFalse /*fUpdate*/, hNil);
	return cmdOK;
}


/*  %%Function:  CmdParaBoth  %%Owner:  bobz       */

CMD CmdParaBoth(pcmb)
CMB * pcmb;
{
	DoLooks(ilcdParaBoth, fFalse /*fUpdate*/, hNil);
	return cmdOK;
}


/*  %%Function:  CmdSpace1  %%Owner:  bobz       */

CMD CmdSpace1(pcmb)
CMB * pcmb;
{
	DoLooks(ilcdSpace1, fFalse /*fUpdate*/, hNil);
	return cmdOK;
}


/*  %%Function:  CmdSpace15  %%Owner:  bobz       */

CMD CmdSpace15(pcmb)
CMB * pcmb;
{
	DoLooks(ilcdSpace15, fFalse /*fUpdate*/, hNil);
	return cmdOK;
}


/*  %%Function:  CmdSpace2  %%Owner:  bobz       */

CMD CmdSpace2(pcmb)
CMB * pcmb;
{
	DoLooks(ilcdSpace2, fFalse /*fUpdate*/, hNil);
	return cmdOK;
}


/*  %%Function:  CmdParaClose  %%Owner:  bobz       */

CMD CmdParaClose(pcmb)
CMB * pcmb;
{
	DoLooks(ilcdParaClose, fFalse /*fUpdate*/, hNil);
	return cmdOK;
}


/*  %%Function:  CmdParaOpen  %%Owner:  bobz       */

CMD CmdParaOpen(pcmb)
CMB * pcmb;
{
	DoLooks(ilcdParaOpen, fFalse /*fUpdate*/, hNil);
	return cmdOK;
}


/*  %%Function:  CmdShowAll  %%Owner:  bobz       */

CMD CmdShowAll(pcmb)
CMB * pcmb;
{
	extern CHAR grpfShowAllRibbon;

	Assert( wwCur != wwNil );
	/* toggle current state and assign to vpref.fvisiShowAll */
	vpref.grpfvisi.fvisiShowAll = ((*hwwdCur)->grpfvisi.fvisiShowAll ^= 1);
	TrashFltg(fltgOther);
	TrashWw(wwCur);
	/* only update button if it was clicked on, wait for idle if from kbd */
	if (vhwndRibbon != NULL)
		{
		grpfShowAllRibbon = (*hwwdCur)->grpfvisi.fvisiShowAll;
		UpdateLHProp (grpfShowAllRibbon, 0, IDLKSSHOWALL);
		}
	/* if doing show all in a pageview pane,
		do show all in all of the doc's pageview panes. */
	if ((*hwwdCur)->fPageView)
		SyncPageViewGrpfvisi(DocMother(selCur.doc), (*hwwdCur)->grpfvisi, (*hwwdCur)->fDisplayAsPrint);
	MakeSelCurVisi(fTrue /*fForceBlockToIp*/);
	return cmdOK;
}


/*  %%Function:  CmdPlainText  %%Owner:  bobz       */

CMD CmdPlainText(pcmb)
CMB * pcmb;
{
	extern BOOL vtmcFocus;

	if (vfSearchRepl && vtmcFocus != tmcSearchNil)
		{
		/* turn off char props in banter and gray them for the search */
		InitSRChps(vtmcFocus == tmcSearch);
		GenSRBanter(0, valNil, 0);
		}
	else
		DoLooks(ilcdPlainText, fFalse /*fUpdate*/, hNil);
	return cmdOK;
}


/*  %%Function:  CmdParaNormal  %%Owner:  bobz       */

CMD CmdParaNormal(pcmb)
CMB * pcmb;
{
	extern BOOL vtmcFocus;

	if (vfSearchRepl && vtmcFocus != tmcSearchNil)
		{
		/* turn off para props in banter and gray them for the search */
		InitSRPaps(vtmcFocus == tmcSearch);
		GenSRBanter(0, valNil, 0);
		}
	else
		DoLooks(ilcdParaNormal, fFalse /*fUpdate*/, hNil);
	return cmdOK;
}


/*  %%Function:  CmdNormalStyle  %%Owner:  bobz       */

CMD CmdNormalStyle(pcmb)
CMB * pcmb;
{
	char grpprl[2];

	if ((*hwwdCur)->fOutline)
		return(CmdConvertToBody(pcmb));

	EmitSprmStc(stcNormal);
	return cmdOK;
}


/* G E T  P L C D  I L C D */
/*  %%Function: GetPlcdIlcd   %%Owner: bobz  */

GetPlcdIlcd (plcd, ilcd)
struct LCD *plcd;
int ilcd;
{
	*plcd = rglcd [ilcd];
}


/*   D O  L O O K S */
/* ****
*
*  Description: decode ilcd and apply looks to current sel
*     Will update the ribbon &/or ruler if fUpdate is non-zero.
*     hParam is used when called by the ruler--it identifies to the ruler
*     update code what ruler to update (will be the hrsd for the ruler).
*
** ***/
/*  %%Function:  DoLooks  %%Owner:  bobz       */


DoLooks(ilcd, fUpdate, hParam)
int ilcd;
int fUpdate;
HANDLE hParam;
{
	CHAR *pprop, *ppropGray;
	char prl[4], prlGray [4];
	int cchPrl, cchPrlGray;
	int fPrevFlag;
	int sgc;
	char sprm;
	int val;

	/* is a window up, and is ilcd a valid looks key? */

	if ( (hwwdCur == hNil) || ilcd < ilcdMin || ilcd > ilcdLast)
		{
		/* bad look key or not implememted */
		Beep();
		return;
		}

	/* valid looks key */

#ifdef RSH
	LogUa(uaLooksKey+ilcd);
#endif /* RSH */

	sprm = rglcd [ilcd].sprm;
	val = rglcd [ilcd].val;

	if (vfSearchRepl)
		{
		DoLooksSR(ilcd, sprm, val);
		return;
		}
	else  if (vfDefineStyle)
		{
		DoLooksDS(ilcd, sprm, val);
		return;
		}

	/* get updated properties for selection if needed */

	if (rglcd [ilcd].lcp == lcpPap)
		{ /* applies to a pap */
		/* only update props if we need to */
		if (selCur.fUpdatePap)
			/* not interruptible */
			FGetParaState(fFalse, /* fAll */ fFalse /* fAbort */);

		sgc = sgcPap;
		pprop = &vpapSelCur;
		ppropGray = &vpapGraySelCur;
		fPrevFlag = selCur.fUpdateRuler;
		}

	else  if (rglcd [ilcd].lcp == lcpChp)
		{ /* ribbon--applies to a chp */
		/* only update props if we need to */
		if (selCur.fUpdateChpGray)
			/* not interruptible */
			FGetCharState(fFalse, /* fAll */ fFalse /* fAbort */);

		sgc = sgcChp;
		pprop = &selCur.chp;
		ppropGray = &vchpGraySelCur;
		fPrevFlag = selCur.fUpdateRibbon;
		vrf.fPreloadSelFont = fTrue;	/* preload new font in Idle */
		}


#ifdef DEBUG
	else

		/* not a chp item & not a pap item */
		/* this is ok if 1. fUpdate is false & 2. fFlip, fKul & fPos are
			all false.  If any of these are true then we need to have
			initialized things that haven't been initialized */

		Assert (!fUpdate && !rglcd [ilcd].fFlip && !rglcd [ilcd].fSpecial);

#endif
	/* If the selection has moved, reset ruler/ribbon formatting
		info. */
	if (rrf.fSelMoved)
		{
		ClearRrf();
		}



	/* handle cases where the correct value to store is not known
		in advance */

	if (rglcd [ilcd].fFlip)
		/* if value in gray prop is non zero, the property is gray.
		If gray or off, turn property on */
		{
		if ( (ValFromPropSprm(ppropGray, sprm) != 0) ||
				(ValFromPropSprm(pprop, sprm) == 0) )
			val = 1;
		}


	else  if (rglcd [ilcd].fSpecial)
		switch (sprm)
			{

			/* underline sprms clear underline if sel is not gray and
			is already underlined the right way */

		case sprmCKul:
			if (vchpGraySelCur.kul == 0 && selCur.chp.kul == val)
				val = kulNone;

			break;


			/* for Sub/SuperScript, same thing as underline for
			turning off if on */

		case sprmCHpsPos:
			if (vchpGraySelCur.hpsPos == 0 &&
					(selCur.chp.hpsPos & 0xFF) == (val & 0xFF))
				val = 0;

			break;


			/* for paranormal, use the current stc regardless of
				grayness */

		case sprmPStc:
			val = vpapSelCur.stc;
			break;

#ifdef DEBUG
			/* special only for search/replace */
		case sprmPJc:
		case sprmPDyaLine:
		case sprmPDyaBefore:
			break;
		default:
			Assert (fFalse);
#endif /* DEBUG */
			}

	/* get & apply the prl */

	cchPrl = CchPrlFromSprmVal (prl, sprm, val);

	if (FRulerIlcd (ilcd) && fUpdate)
		/* in this case we don't apply the property to selCur but
			instead we apply it to the global ruler sprm */
		AddRulerGrpprl (hParam /*hrsd*/, prl, cchPrl);

	else
		{
		/* apply the property to the current selection */
		/* causes selCur.fUpdate??? to be set */
		if (sprm == sprmPStc)
			EmitSprmStc(val);
		else
			{
			/*  ApplyGrpprlSelCur can be cancelled due to unable
				to undo vmerr.fMemFail would be set in that case.
				Since no action has been accomplished, we just
				bag out and do not update the system
			*/
			ApplyGrpprlSelCur (prl, cchPrl, fTrue /* fSetUndo */);
			if (!vmerr.fMemFail)
				FSetAgainGrpprl(prl, cchPrl, bcmFormatting);
			else
				return;
			}
		}

	/* Store sel in GoBack cache */
	PushInsSelCur();

	/* Record in rrf */
	RecordRrfIlcd(ilcd, val);

	if (fUpdate && !rglcd [ilcd].fNoUpdate)
		{

		/* if called from the ribbon or from the ruler we would
			like to have the Ribbon or Ruler reflect this change
			immediately. We previously set the individually changed
			item by hand rather than updating the entire ruler or
			ribbon, but there were at least problems with thinks like
			ApplyGrpPrlSelCur failing and us updating anyway. So rather
			than assume the result, we find the result by doing the
			full update.
		
			I am assuming the the property setters will invalidate the
			selCur props if needed, so I don't have to do that now. bz
		*/

		if (FLHIlcd (ilcd))
			{
			UpdateRibbon(fFalse /* fInit */);
			}
		else
			{
			Assert (FRulerIlcd (ilcd));
			/* called by ruler -- update the ruler */
			UpdateRuler(hmwdCur, fFalse /*!fInit*/,
					-1/*rk*/, fTrue /*fAbortOK*/);
			}
		}

	if (ilcd == ilcdVanish)
		{
		if (!selCur.fIns)
			MakeSelCurVisi(fTrue /*fForceBlockToIp*/);
		else
			InvalVisiCache();
		}
}


/*  %%Function:  RecordRrfIlcd  %%Owner:  bobz       */


RecordRrfIlcd(ilcd, val)
int ilcd, val;
{
	switch (ilcd)
		{
	case ilcdVanish:
		return;	/* not included in rrf stuff */
	case ilcdBold:
		rrf.fBold = !rrf.fBold;
		break;
	case ilcdItalic:
		rrf.fItalic = !rrf.fItalic;
		break;
	case ilcdSmallCaps:
		rrf.fSCaps = !rrf.fSCaps;
		break;
	case ilcdKulSingle:
	case ilcdKulWord:
	case ilcdKulDouble:
		rrf.kul = val;
		break;
	case ilcdSuperscript:
		rrf.iSuperSub = (val == 0) ? iSSNormal : iSSSuper;
		break;
	case ilcdSubscript:
		rrf.iSuperSub = (val == 0) ? iSSNormal : iSSSub;
		break;
	case ilcdParaNormal:
		if (rrf.hgrpprl != hNil)
			{
			FreePh(&rrf.hgrpprl);
			rrf.hgrpprl = hNil;
			rrf.cbGrpprl = 0;
			}
	default:
		ClearRibbonRrf();
		return;
		}
	ClearRulerRrf();
}



/* ****
*  Description: UpdateSelCurProp
	this function applies a property directly to the chp & chpGray or
	pap and papGray in selCur.  it does not change the state of fUpdateChp
	or fUpdatePap, reconizing that one or both of those may be out of sync
	with the selection.  It does set the WatchDog flags for items which
	are watching the affected structure (ruler for pap & Ribboner for chp).

	PeterJ
*
** **** */

/*  %%Function:  UpdateSelCurProp   %%Owner:  bobz       */


UpdateSelCurProp (sgc, prl, cchPrl, prlGray, cchPrlGray)
int sgc, cchPrl, cchPrlGray;
CHAR * prl, * prlGray;

{
	CHAR * pprop, * ppropGray;

	switch (sgc)
		{
	case sgcPap:
		pprop = &vpapSelCur;
		ppropGray = &vpapGraySelCur;

		selCur.fUpdateRuler = fTrue;

		break;

	case sgcChp:
		pprop = &selCur.chp;
		ppropGray = &vchpGraySelCur;

		selCur.fUpdateRibbon = fTrue;

		break;

#ifdef DEBUG
	default:
		Assert (fFalse);
		return;
#endif  /* DEBUG */

		}

	if (cchPrl > 0)
		ApplyPrlSgc ((char HUGE *) prl, cchPrl, pprop, sgc);

	if (cchPrlGray > 0)
		ApplyPrlSgc ((char HUGE *) prlGray, cchPrlGray, ppropGray, sgc);

}



extern BOOL vfOvertypeMode;

/*  %%Function:  CmdOvertype  %%Owner:  bradch       */


CMD CmdOvertype(pcmb)
CMB * pcmb;
{
	extern BOOL vfElFunc;

	/* ElOvertype is a dual purpose function, serves as a statement
	and command function, we should not be vfElFunc when calling from
	Cmd function
	*/
	Assert(!vfElFunc);
	return (ElOvertype(0) == 0 ? cmdOK : cmdError);
}


/*  %%Function:  ElOvertype  %%Owner:  bradch       */

EL int ElOvertype(fOn)
BOOL fOn;
{
	extern BOOL vfElFunc;
	extern int vcElParams;

	if (vfElFunc)
		return vfOvertypeMode ? -1 : 0;

	/* Cannot go into Overtype mode if already in Revision Marking mode */
	if (!vfOvertypeMode && 
			selCur.doc != docNil && PdodMother(selCur.doc)->dop.fRevMarking)
		{
		ModeError();
		return -1; /* keep the stack happy */
		}

	if (vcElParams == 0)
		fOn = !vfOvertypeMode;

	if ((!vfOvertypeMode) != (!fOn)) /* !'s are required! (bac) */
		{
		vfOvertypeMode = fOn;
		if (vhwndStatLine != NULL)
			UpdateStatusLine(usoToggles);
		}

	return 0; /* keep the stack happy */
}


/*  %%Function:  ToggleExtendSel  %%Owner:  chic       */

ToggleExtendSel()
{
	vfExtendSel = !vfExtendSel;
	if (vhwndStatLine != NULL)
		UpdateStatusLine(usoToggles);
}


/*  %%Function:  ExtendSelBegin  %%Owner:  chic       */

ExtendSelBegin()
{
	Assert(!vfExtendSel && hmwdCur != hNil);
	ToggleExtendSel();
	if (selCur.fBlock)
		TurnOffBlock(&selCur);
	selCur.cpAnchorShrink = selCur.cpFirst;
}


/*  %%Function:  ExtendSelEnd  %%Owner:  chic       */

ExtendSelEnd()
{
	if (vfExtendSel)
		{
		Assert(hmwdCur != hNil);
		ToggleExtendSel();
		}
}


/*  %%Function:  CmdExtend  %%Owner:  chic       */

CMD CmdExtend(pcmb)
CMB * pcmb;
{
	if (!vfExtendSel)
		ExtendSelBegin();
	else  if (hmwdCur != hNil) /* grow selection */
		GrowSelect();
	return cmdOK;
}


/*  %%Function:  CmdShrinkExtend  %%Owner:  chic       */

CMD CmdShrinkExtend(pcmb)
CMB * pcmb;
{

	Assert(hmwdCur);
/* establish cpAnchorShrink if it is not valid or outside selCur's range */
	if (!FInCa( selCur.doc, selCur.cpAnchorShrink, &selCur.ca))
		selCur.cpAnchorShrink = selCur.cpFirst;

	ShrinkSelect();

	return cmdOK;
}


#define istyGrowWord	1
#define istyGrowSection	4
#define istyGrowMax	6

csconst int rgstyGrow [] = { 
	styChar, styWord, stySent, styPara, stySection, 
			styDoc };


/* G R O W  S E L E C T */
/* Grow selection in both direction to the next larger unit.
	Unit can be word, sentence, paragraph, document and field */

/*  %%Function:  GrowSelect  %%Owner:  chic       */

GrowSelect()
{
	struct CA ca, caOld, caField;
	CP cpAnchor = selCur.cpAnchorShrink;
	int sty, isty;
	BOOL fShortDoc = PdodDoc(selCur.doc)->fShort;

	caOld = selCur.ca;
	ca.doc = caField.doc = selCur.doc;
	Assert(vfExtendSel && hmwdCur != hNil);
	for (isty = istyGrowWord; isty < istyGrowMax; isty++)
		{
		sty = rgstyGrow [isty];
		Assert(sty == styWord || sty == stySent ||
				sty == styPara || sty == stySection ||
				sty == styDoc);

		if (fShortDoc && sty == stySection)
			continue;
		else
			{
			ExpandCpSty(&ca, cpAnchor, sty);
			if (!FNeRgch(&ca, &caOld, sizeof(struct CA)))
				{
				if (sty == styDoc)
					{
					Beep(); /* can't grow anymore */
					break;
					}
				}
			else  if (FCaInCa( &caOld, &ca ))
				break;
			}
		}

/* Compare and use the smaller that includes the original selection. */
	if (FGetCpFieldFromSelCur(&caField, fTrue/*fEnclose*/))
		{
		if (caField.cpFirst >= ca.cpFirst)
			{
			Assert(caField.cpLim <= ca.cpLim);
			ca = caField;
			}
#ifdef DEBUG
		else
			Assert(caField.cpLim >= ca.cpLim);
#endif
		}
	Assert(ca.cpLim >= caOld.cpFirst && caOld.cpLim >= ca.cpFirst);
	Select(&selCur, ca.cpFirst, ca.cpLim);
	OutlineSelCheck(&selCur);
	if (vhwndCBT)
		CBTSelectPsel(&selCur);
}


/*  %%Function:  ElExtendSel  %%Owner:  bobz       */


EL ElExtendSel(hpsd)
SD huge * hpsd;
{
	extern int vcElParams;

	switch (vcElParams)
		{
#ifdef DEBUG
	default:
		Assert(fFalse);
		break;
#endif
	case 0:
		if (vfExtendSel)
			GrowSelect();
		else
			ExtendSelBegin();
		break;

	case 1:
		if (CchFromSd(*hpsd) == 1)
			ExtendSelToCh((int) *HpchFromSd(*hpsd));
		break;
		}
}


/*  %%Function:  ExtendSelToCh  %%Owner:  chic       */

ExtendSelToCh(ch)
int ch;
{
	int doc = selCur.doc;
	CP cpFirst = selCur.fForward ? selCur.cpLim : selCur.cpFirst+1;
	CP cpMac = CpMacDoc(doc);
	CP cpSearch = cpNil;
	int ccpFetch;
	int ich;
	CHAR HUGE *hpch;

	if (cpFirst >= cpMac)
		{
		Beep();
		return;
		}

	FetchCpPccpVisible(doc, cpFirst, &ccpFetch, selCur.ww/*fvcScreen*/, fFalse);
	for ( ; ccpFetch > 0; )
		{
		hpch = vhpchFetch;
		ich = 0;
		while (ich < ccpFetch)
			{
			if (*hpch++ == (CHAR)ch)
				{
				cpSearch = vcpFetch + ich;
				if (cpSearch < selCur.cpLim)
					cpSearch--;
				goto LFound;
				}
			ich++;
			}
		if ((vcpFetch + ccpFetch) >= cpMac)
			break;
		FetchCpPccpVisible(docNil, cpNil, &ccpFetch, selCur.ww/*fvcScreen*/, fFalse); /* sequential search */
		}

LFound:
	if (cpSearch != cpNil)
		{
		ChangeSel(&selCur,CpLimSty(wwCur, selCur.doc,cpSearch, styChar, fFalse /*fEnd*/), styNil, fFalse, fFalse);
		OutlineSelCheck(&selCur);
		NormCp(wwCur, selCur.doc, selCur.cpLim, ncpVisifyPartialDl, 0, selCur.fInsEnd);
/* setting vfSeeSel does not work because SeeSel does not care if the tail end is off the screen - by design */
		if (vfRecording)
			RecordExtendSelToCh(ch);	/* If recording macro, add ExtendSelToCh */
		}
	else
		Beep();
}



/* S H R I N K  S E L E C T */

/* LATER (rp): Make this stuff smart about parts of a table */
/* My idea for the rules is as follows:                            */
/*   Extend:                                                       */
/*       skSel && fWithinCell  -->  select cell                    */
/*       fTable && within row  -->  select whole row               */
/*       fTable && !within row -->  select whole table             */
/*   Shrink:                                                       */
/*       fTable && one cell selected --> styPara                   */
/*       fTable && within row  -->  select anchor cell             */
/*       fTable && FWholeRowsPsel && !single row --> anchor row    */
/*       fTable && !FWholeRows && !within tap --> anchor cell      */
/* Currently we are just ignoring the existence of tables and we   */
/* skip from styPara directly to stySection, even if the para is   */
/* within a cell.                                                  */
/* *************************************************************** */

/*  %%Function:  ShrinkSelect  %%Owner:  chic       */

ShrinkSelect()
{
	struct CA ca, caOld, caField;
	CP cpAnchor = selCur.cpAnchorShrink;
	int sty, isty;
	BOOL fShortDoc = PdodDoc(selCur.doc)->fShort;

	caOld = selCur.ca;
	ca.doc = caField.doc = selCur.doc;
	Assert(cpAnchor != cpNil);
	if (selCur.fBlock)
		{
		TurnOffBlock(&selCur); /* turn into insertion pt */
		return;  /* don't beep */
		}

	if (selCur.fIns)
		{
		Beep();
		return;
		}

	for (isty = istyGrowWord; isty < istyGrowMax; isty++ )
		{
		sty = rgstyGrow [isty];
		Assert(sty == styWord || sty == stySent ||
				sty == styPara || sty == stySection ||
				sty == styDoc);
		if (fShortDoc && sty == stySection)
			continue;
		else
			{
			ExpandCpSty(&ca, cpAnchor, sty);
			if (FCaInCa( &caOld, &ca))
				{ /* use the last sty */
				Assert( isty > 0 );
				sty = rgstyGrow [--isty];
				if (fShortDoc && sty == stySection)
					{
					Assert( isty > 0 );
					sty = rgstyGrow [--isty];
					}
				break;
				}
			}
		}

	Assert(sty >= styChar);
	if (sty == styChar)
		PcaPoint( &ca, ca.doc, cpAnchor );/* collapsed into the starting point */
	else
		{
		ExpandCpSty(&ca, cpAnchor, sty);

/* Compare and use the larger that is within the original selection which
enclose the starting point. */
		if (FGetCpFieldFromSelCur(&caField, fFalse/*fEnclose*/) &&
				FCaInCa(&ca,&caField))
			{
			ca = caField;
			}
		}
	Assert(ca.cpLim >= caOld.cpFirst && caOld.cpLim >= ca.cpFirst);
	Select(&selCur, ca.cpFirst, ca.cpLim);
	OutlineSelCheck(&selCur);
	if (vhwndCBT)
		CBTSelectPsel(&selCur);
}


/* E x p a n d  C p  S t y */

/*  %%Function:  ExpandCpSty  %%Owner:  chic       */


ExpandCpSty(pca, cpAnchor, sty)
struct CA *pca;
CP cpAnchor;
int sty;
{
	Assert(sty != styChar);
#ifdef DEBUG
	if (PdodDoc(pca->doc)->fShort)
		Assert(sty != stySection);
#endif
	pca->cpFirst = CpFirstSty( wwCur, pca->doc, cpAnchor, sty, fFalse );
	pca->cpLim = CpLimSty( wwCur, pca->doc, cpAnchor, sty, fFalse );
	AssureLegalSel(pca);
}


/* F  G E T  C P  F I E L D  F R O M  S E L  C U R */
/*  %%Function:  FGetCpFieldFromSelCur  %%Owner:  bobz       */

FGetCpFieldFromSelCur(pca,fEnclose)
struct CA *pca;
BOOL fEnclose;
{
	int doc = pca->doc;
	int ifld = (fEnclose ? IfldEnclosingCa(&selCur.ca) :
			IfldWithinCa(&selCur.ca));

	if (ifld != ifldNil)
		{
		PcaField( pca, doc, ifld );
		return fTrue;
		}
	return fFalse;
}



/* F  C A  I N  C A */
/* Return true if *pca1 is wholly inside *pca2 (definition of inside
	includes equality at the boundaries) */
/*  %%Function:  FCaInCa  %%Owner:  bobz       */

FCaInCa( pca1, pca2 )
struct CA *pca1;
struct CA *pca2;
{
	return pca1->doc == pca2->doc && pca1->cpFirst >= pca2->cpFirst && 
			pca1->cpLim <= pca2->cpLim;

}


/*  %%Function:  IfldEnclosingCa  %%Owner:  bobz       */

IfldEnclosingCa(pca)
struct CA *pca;
{
	int doc = pca->doc;
/* find the field that encloses the original selection */
	int ifld = IfldFromDocCp(doc, pca->cpFirst, fFalse);

	if (ifld != ifldNil && CpFirstField(doc, ifld) == pca->cpFirst)
/* this field is already selected, find the one that encloses it. */
		ifld = IfldEncloseIfld(doc, ifld);
	return(ifld);
}


/*  %%Function:  IfldWithinCa  %%Owner:  bobz       */

IfldWithinCa(pca)
struct CA *pca;
{
	int ifld = IfldNextField(pca->doc, pca->cpFirst + 1);
	struct CA ca;

	if (ifld != ifldNil)
		if (FCaInCa(PcaField(&ca, pca->doc, ifld), pca))
			return ifld;

	return ifldNil;
}





/*  F  D E L  S E L */
/* Delete selection, possibly checking for block selection.
	save to scrap if fScrap is true.
	called by most del/cut routines.
	pcaRM is set to what was left by revision marking after the delete.
*/
/*  %%Function:  FDelSel  %%Owner:  bobz       */

FDelSel(fScrap, fBlockOk, pcaRM)
BOOL  fScrap;
BOOL  fBlockOk;
struct CA *pcaRM;
{

	BOOL fDeleteOk;
	int cmd;

	/* properties of deleted text are transient insert props */
	/* get props of deleted text before deleting */

	PcaSetNil(pcaRM);
	GetSelCurChp (fFalse /* fForce */);
	/* note that for block, pcaRm is null, but it is not used by the caller */
	if (selCur.fBlock && fBlockOk)
		{
		if ((cmd = CmdCutBlock(fScrap)) != cmdOK)
			{
LFail:
			Beep();
			return fFalse;
			}
		}
	else  if ((cmd = CmdDeleteSelCur(fScrap,
			PdodMother(selCur.doc)->dop.fRevMarking /* fRM */,
			pcaRM, rpkNil)) != cmdOK)
		goto LFail;

	/* re-validate selCurChp */
	selCur.fUpdateChp = fFalse;
	AssureNestedPropsCorrect(pcaRM, fFalse);

	return (fTrue);
}


/* K E Y  C M D  D E L */
/* Delete selection, or character ahead of insert point
	If selection, save to scrap if fScrap is true.
*/
/*  %%Function:  KeyCmdDel  %%Owner:  bobz       */

KeyCmdDel(fScrap)
int  fScrap;
{
	extern MSG vmsgLast;
	struct CA caRM, caOld;
	int fBlock;

	caOld = selCur.ca;
	if (selCur.fIns)
		{
		CP cpLim;

		FetchCpAndPara (selCur.doc, selCur.cpFirst, fcmProps);
		if (vchpFetch.fSpec
				|| !FVisibleCp (selCur.ww, selCur.doc, selCur.cpFirst))
			/*  you cannot forward delete an fSpec or invisible character */
			{
LBeep:
			Beep();
			return;
			}

		/* Don't allow deleting a table cell mark */
		if (FInTableDocCp(selCur.doc, selCur.cpFirst))
			{
			CacheTc(wwNil, selCur.doc, selCur.cpFirst, fFalse, fFalse);
			if (selCur.cpFirst == vtcc.cpLim - ccpEop)
				goto LBeep;
			}

		TurnOffSel(&selCur);
		/* to handle special case of deleting last para mark without
			modifying several lower layers including cpLimSty bz
		*/
		Assert (selCur.cpFirst <= CpMacDocEdit(selCur.doc));

		cpLim = (selCur.cpFirst < CpMacDocEdit(selCur.doc)) ?
				/* use styCRLF so we don't delete trailing hidden text
			(tables/fields) */
		CpLimSty (wwCur, selCur.doc, selCur.cpFirst, styCRLF, fFalse) :
				CpMacDoc(selCur.doc);
		Select (&selCur, selCur.cpFirst, cpLim);
		fScrap = fFalse;  /* override input fScrap for ins point */
		}
	else  if (selCur.fTable)
		{
		CmdCutTable(&selCur, fScrap ? docScrap : docNil, fTrue /* fSetUndo */);
		return;
		}

	if (!FSetUndoBefore(fScrap ? bcmCut : bcmClear, fScrap ? uccCut : uccDeleteSel))
		{
		SelectIns( &selCur, selCur.cpFirst );
		return cmdCancelled;
		}

	fBlock = selCur.fBlock;

	FDelSel(fScrap, fTrue  /* fBlockOk */, &caRM);

	if (!fBlock)  /* block has already taken care of select and undo */
		{

/* FUTURE - we probably should make SetUndoAfter takes care of docNil case (cc) */
		if (caRM.doc != docNil)
			{
			SelectPostDelete(caRM.cpLim);
			SetUndoAfter(&caRM);
			}
		else
			{
			Select(&selCur, caOld.cpFirst, caOld.cpLim);
			SetUndoNil(); /* FUTURE - can we restore the last undo since we really didn't delete text here? (cc) */
			}
		}

	UpdateWw( wwCur, fFalse /* fAbortOk*/);
	vfSeeSel = fTrue;
	ChangeCursor(fFalse/*fExact*/);

}


/* C M D  D E L  C U T*/
/* Delete selection or next char forwards, putting result in scrap if
	from selection
*/
/*  %%Function:  CmdDelCut  %%Owner:  bobz       */

CMD CmdDelCut(pcmb)
CMB * pcmb;
{
	KeyCmdDel(fTrue /* fScrap */);
	return cmdOK;
}


/* C M D  D E L  C L E A R */
/* Delete selection or next char forwards, not putting result in scrap if
	from selection
*/
/*  %%Function:  CmdDelClear  %%Owner:  bobz       */

CMD CmdDelClear(pcmb)
CMB * pcmb;
{
	KeyCmdDel(fFalse /* fScrap */);
	return cmdOK;
}


/*  %%Function:  CmdEscape  %%Owner:  bobz       */

CMD CmdEscape(pcmb)
CMB * pcmb;
{
	extern void ** vhmes;
	extern BOOL fElActive;

/* NOTE: Watch the ordering here!  The Beep should be last!!!! */
	if (vfExtendSel)
		ExtendSelEnd();
	else  if (vfBlockSel)
		{
		BlockModeEnd();
		SelectIns(&selCur, CpFirstBlock());
		}
	/* WARNING the vhmes test must be RIGHT before the beep! */
	else  if (!fElActive && vhmes != hNil)
		FreeEditMacro(iNil);
	else
		Beep();

	return cmdOK;
}


/*  %%Function:  CmdOK  %%Owner:  bobz       */

CMD CmdOK(pcmb)
CMB * pcmb;
{
	/* Note: This will have to be expanded if we use commands other
		than the dyadic move/copy commands for cmdOK */

	if (vssc != sscNil)
		{
		return (CmdDoDyadic(pcmb));
		}
	return cmdError;  /* noone else should call this  bz */
}


#ifdef PROFILE
/*  %%Function:  CmdToggleProfiler  %%Owner:  bobz       */

CmdToggleProfiler(pcmb)
CMB * pcmb;
{
	static fProfiling = fFalse;
	extern int vpfi;

	if (fProfiling)
		StopProf();
	fProfiling = !fProfiling;
	if (fProfiling)
		StartProf(30);

	return cmdOK;
}


#endif /* PROFILE */

/*  %%Function:  CmdSelectWholeDoc  %%Owner:  bobz       */

CMD CmdSelectWholeDoc(pcmb)
CMB *pcmb;
{
	Select( &selCur, cp0, CpMacDoc(selCur.doc));
	return cmdOK;
}


