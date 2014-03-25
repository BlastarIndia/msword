/*  S R C H F M T . C  */

/*  Search/Replace Formatting     rosiep    		6/19/89
*
*
*  When properties are turned on and off by the user in the dialog
*  box, we need a way to keep track of which ones are in use and
*  which ones aren't.  Since some fields in the chp and pap have
*  no "illegal" values to indicate that the particular property
*  they represent is not currently active in the dialog, we use a
*  chpGray (and papGray) where all the bits in a field are ON if
*  the property is inactive (i.e. a "don't care" or "gray" property).
*  All the bits of a field are OFF if the property is active.  Thus
*  we can represent all states.  When the user activates a property
*  in the dialog, e.g. turns on Bold for the Replace edit control,
*  we do two things:  (1) we ungray that property in the chpGray (or
*  papGray if it's a paragraph property) which belongs to that edit
*  control (each edit control has its own separate set of properties,
*  all stored in the FB).  (2) we set the field in the chp or pap
*  to the value appropriate for the state of the property in the
*  dialog.  These two actions are done in different ways for
*  different properties.  Some are done using ApplyPrlSgc, some
*  get set directly in the chp, chpGray, pap, or papGray.  In
*  particular, the enumerated lists are done quite differently
*  from all the other properties.  There are also some properties
*  which have special behavior (for convenience only) in the
*  dialog.  These properties toggle off (gray) when the formatting
*  key is applied a second time, even though they are not normally
*  toggle keys when formatting a document, for example Ctrl-c for
*  center.  When a property like this toggles off, we set the bits
*  in the gray chp (or pap) to cause it not to be paid attention
*  to anymore.  There is lots of special case code to handle the
*  different properties.  See GenSRBanter, DoLooksSR, and
*  BuildReplaceGrpprls for specifics.  The other major routines used in
*  SR Formatting are:  InitSearchFormatting, InitReplaceFormatting,
*  EnumFont, EnumPtSize, EnumColor, ReplacePropsCa, CheckForFormatting,
*  and anyone who calls any of the above functions.
*
*
*  BuildReplaceGrpprls builds up grpprls based on the properties that
*  were not "gray" in the dialog.  ReplacePropsCa does the property
*  replacement.  Some rules:  1) Any property which is not specified
*  in either the search or replace property list is not touched;
*  2) If a character property is specified in Search and a different
*  one is specified in Replace, we turn the former off and replace it
*  with the latter, unless the 0 value of the property means something
*  other than "off" or "Auto".  (i.e. we try to do the sensible thing).
*  3) If there is no text in the Replace edit control but there are
*  properties in the Replace property list, then we replace only the
*  properties and leave the text alone.  4) If there are no properties
*  in the Replace chp/pap, but there is replacement text, we replace
*  only the text and keep the properties the same.  This means you
*  can't turn off bold by searching for "foo" (Bold) and replacing
*  with "foo".  The only way to do that (and we added a special hack
*  for this in CheckForFormatting) is to replace "foo" (Bold) with "^m".
*
*/


#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "props.h"
#include "sel.h"
#include "doc.h"
#include "prm.h"
#define SEARCHTMC
#include "search.h"
#include "tmc.h"
#include "field.h"
#include "file.h"
#include "cmdlook.h"
#include "debug.h"

#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#include "search.hs"
#include "search.sdm"
#include "replace.hs"
#include "replace.sdm"


#ifdef PROTOTYPE
#include "srchfmt.cpt"
#endif /* PROTOTYPE */

extern struct FB vfbSearch;
extern struct FB vfbReplace;
extern BOOL vtmcFocus;
extern int  icoSR;  /* current color for SR dialog */
extern BOOL vfSearchRepl;
extern BOOL vfEndFetch;
extern CP vcpMatchLim;
extern CP vcpFetch;
extern int vccpFetch;
extern struct CHP vchpFetch;
extern struct PAP vpapFetch;
extern struct SEL selCur;
extern struct CA caPara;
extern BOOL vfFwd;
extern int vfNoInval;


#ifdef NOASM
NATIVE BOOL FMatchPap(), FMatchChp(); /* DECLARATION ONLY */
#endif /* NOASM */
CP CpSearchFb(), CpSearchFbBackward();
NATIVE CP CpSearchPap(); /* DECLARATION ONLY */



/*
*  R E P L A C E  P R O P S  C A
*
*  Replaces properties of the current selection based on grpprls that
*  were constructed by BuildReplaceGrpprls.  Invalidates the necessary
*  range of cp's.
*
*/

/* %%Function:ReplacePropsCa %%Owner:rosiep */
ReplacePropsCa(prpp, pca)
struct RPP *prpp;
struct CA *pca;
{
	struct CA caInval;

	if (prpp->cbgrpprlChp)
		{
		ExpandCaSprm(pca, &caInval, prpp->grpprlChp);
		ApplyGrpprlCa(prpp->grpprlChp, prpp->cbgrpprlChp, pca);
		if (!vfNoInval)
			{
			InvalCp(pca);
			InvalText(pca, fFalse /* fEdit */);
			}
		}

	if (prpp->cbgrpprlPap)
		{
		int fStc;
		struct CHP chp;
		struct PAP pap;

		if (fStc = (*prpp->grpprlPap == sprmPStc))
			{
			CachePara(pca->doc, pca->cpFirst);
			pap = vpapFetch;
			}
		ExpandCaSprm(pca, &caInval, prpp->grpprlPap);
		ApplyGrpprlCa(prpp->grpprlPap, prpp->cbgrpprlPap, pca);
		if (fStc)
			{
			GetMajorityChp(pca, &chp);
			EmitSprmCMajCa(pca, &chp);
			if (!FMatchAbs(pca->doc, &pap, &vpapFetch))
				InvalPageView(pca->doc);
			}
		if (!vfNoInval)
			{
			InvalCp(&caInval);
			InvalText (pca, fFalse /* fEdit */);
			DirtyOutline(pca->doc);
			}
		}
}


/* %%Function:EnumFontUp %%Owner:rosiep */
EnumFontUp()
{
	EnumFont(fTrue /* fUp */);
}


/* %%Function:EnumFontDown %%Owner:rosiep */
EnumFontDown()
{
	EnumFont(fFalse /* fUp */);
}


/*
*  E N U M  F O N T
*
*  The effect of EnumFont is to cycle through the ibstFonts's, in increasing
*  or decreasing order, wrapping around from (ibstFontMac -1) back to 0,
*  with a "don't care" state between (ibstFontMac - 1) and  0.
*
*/

/* %%Function:EnumFont %%Owner:rosiep */
EnumFont(fUp)
BOOL fUp;
{
	extern struct STTB **vhsttbFont;

	int ibstFontMac = (*vhsttbFont)->ibstMac, ibstFont;
	struct FB *pfb;
	int *pftc;

	if (vtmcFocus == tmcSearchNil)
		{
		Beep();
		return cmdOK;
		}

	if (vtmcFocus == tmcSearch)
		{
		pfb = &vfbSearch;
		pftc = &vfbSearch.chp.ftc;
		}
	else
		{
		pfb = &vfbReplace;
		pftc = &vfbReplace.chp.ftc;
		}

	ibstFont = IbstFontFromFtcDoc(*pftc, selCur.doc);

	/* if font is gray, set up so the following addition will put us at
		the next ftc we want */
	if (pfb->chpGray.ftc)
		{
		pfb->chpGray.ftc = 0;  /* ungray */
		ibstFont = (fUp ? -1 : ibstFontMac);
		}
	/* if we go past the end of the cycle, gray the ftc; next call to
		EnumFont will ungray it */
	else  if (fUp && ibstFont == ibstFontMac - 1 || !fUp && ibstFont == 0)
		pfb->chpGray.ftc = -1;

	/* cycle through the ibstFont's mod ibstFontMac */
	ibstFont = (ibstFont + (fUp ? 1 : ibstFontMac - 1)) % ibstFontMac;

	if (!pfb->chpGray.ftc)
		*pftc = FtcFromDocIbst(selCur.doc, ibstFont);

	GenSRBanter( 0, valNil, 0 );
	return cmdOK;
}


/* %%Function:EnumPtSizeUp %%Owner:rosiep */
EnumPtSizeUp()
{
	EnumPtSize(fTrue /* fUp */);
}


/* %%Function:EnumPtSizeDn %%Owner:rosiep */
EnumPtSizeDn()
{
	EnumPtSize(fFalse /* fUp */);
}


/* %%Function:EnumPtSize %%Owner:rosiep */
EnumPtSize(fUp)
BOOL fUp;
{
	CHAR *phps;
	struct FB *pfb;

	if (vtmcFocus == tmcSearchNil)
		{
		Beep();
		return cmdOK;
		}

	if (vtmcFocus == tmcSearch)
		{
		pfb = &vfbSearch;
		phps = &vfbSearch.chp.hps;
		}
	else
		{
		pfb = &vfbReplace;
		phps = &vfbReplace.chp.hps;
		}

/* if point size is gray, start from 4 points or 127 */
/* this is prior to increment/decrement */
	if (pfb->chpGray.hps != 0)
		{
		*phps = (fUp ? 7 : 255);
		pfb->chpGray.hps = 0;  /* ungray */
		}

	/* if we go past the end of the cycle, gray the size; next call to
		EnumPtSize will ungray it */
	else  if (fUp && *phps == 255 - 1 || !fUp && *phps == 8)
		pfb->chpGray.hps = -1;


	/* cycle through the possible half point sizes */
	/* what this line does: increase or decrease hps by 1 and wrap
		around to stay within legal point sizes.  The range of legal
		point sizes is [4,127] or [8,254] in hps.  If we subtract 8
		first, we can work modulo 247 [0,246] to wrap around correctly,
		then add the 8 back in.  I have left the -8 separate, instead
		of combining it into the (fUp ? 1 : 246) expression, to add
		clarity, as if that helps... */

	*phps = ((*phps - 8 + (fUp ? 1 : 246)) % 247) + 8;

	/*  chp and gray chp already handled */
	GenSRBanter( 0, valNil, 0 );

	return cmdOK;
}


/* %%Function:EnumColorUp %%Owner:rosiep */
EnumColorUp()
{
	EnumColor(fTrue /* fUp */);
}


/* %%Function:EnumColorDn %%Owner:rosiep */
EnumColorDn()
{
	EnumColor(fFalse /* fUp */);
}


/*
*  E N U M  C O L O R
*
*  The effect of EnumColor is to cycle through the ico's, in increasing
*  or decreasing order, wrapping around from (icoMax - 1) back to 0,
*  with a "don't care" state between (icoMax - 1) and  0.
*
*/

/* %%Function:EnumColor %%Owner:rosiep */
EnumColor(fUp)
BOOL fUp;
{
	struct FB *pfb;
	int ico;

	if (vtmcFocus == tmcSearchNil)
		{
		Beep();
		return cmdOK;
		}

	if (vtmcFocus == tmcSearch)
		pfb = &vfbSearch;
	else
		pfb = &vfbReplace;

	ico = pfb->chp.ico; /* ico is a bit field; put in int to handle -1 case */

	/* if Color is "gray", set up so the following addition will put us at
		the next color we want */
	if (pfb->chpGray.ico)
		{
		pfb->chpGray.ico = 0;  /* ungray */
		ico = (fUp ? -1 : icoMax - 1);
		}
	/* if we go past the end of the cycle, gray the color; next call to
		EnumColor will ungray it */
	else  if (fUp && ico == icoMax - 1 || !fUp && ico == 0)
		pfb->chpGray.ico = -1;

	/* cycle through the ico's mod icoMax */
	ico = (ico + (fUp ? 1 : icoMax - 1)) % icoMax;
	pfb->chp.ico = ico;

	/*  chp and gray chp already handled */
	GenSRBanter( 0, valNil, 0 );

	return cmdOK;
}


/*  These two functions simulate the "looks keys" for Strikethrough  and
	New Text.  They are only available in the Search and Replace dialogs.
*/
/* %%Function:DoLooksStrike %%Owner:rosiep */
DoLooksStrike ()
{
	Assert (vfSearchRepl);
	DoLooks(ilcdStrike, fFalse, hNil);
	return cmdOK;
}


/* %%Function:DoLooksRMark %%Owner:rosiep */
DoLooksRMark ()
{
	Assert (vfSearchRepl);
	DoLooks(ilcdRMark, fFalse, hNil);
	return cmdOK;
}


/*
*  G E N  S  R  B A N T E R
*
*  Generate banter text for Search/Replace dialogs.  Actually
*  CchFillStWithBanter does the dirty work.  Here we figure out what
*  property has changed and decide whether to display it or not.  We
*  apply the property to the appropriate chp or pap and gray or
*  ungray it as necessary.
*
*/

/* %%Function:GenSRBanter %%Owner:rosiep */
GenSRBanter( prl, cchPrl, sgc )
CHAR prl[];
int cchPrl;
int sgc;
{
	struct CHP *pprop, *ppropGray;
	char sprm = prl[0];
	struct FB *pfb;
	int val;

	/* typing character formatting keys is ambiguous if neither search
		or replace edit control have the focus, so we don't touch the
		banter in that case */

	/* the caller should have taken care of determining who had the focus
		and setting up vtmcFocus accordingly */
	Assert (vtmcFocus != tmcSearchNil);

	pfb = (vtmcFocus == tmcSearch ? &vfbSearch : &vfbReplace);

	if (sgc == sgcChp)
		{
		pprop = &pfb->chp;
		ppropGray = &pfb->chpGray;
		}
	else
		{
		pprop = &pfb->pap;
		ppropGray = &pfb->papGray;
		}

	/* valNil is special value passed in when the caller has already
		taken care of applying the sprm to the appropriate FB.  In that
	case we'll only need to call DisplaySRBanter */
	if (cchPrl != valNil)
		{
		/* apply the property to the chp or pap we're building */
		ApplyPrlSgc((char HUGE *) prl, cchPrl, pprop, sgc);

		/* these should have been handled in enumeration function */
		Assert(sprm != sprmCSizePos && sprm != sprmCIco);

		/* Force these to be ungray; 0 is a legal value that doesn't mean
		"don't care".  The way these guys become gray is through
		DoLooksSR toggling them off (uses the valNil special case). */
		if (sprm == sprmPJc || sprm == sprmPDyaBefore)
			val = 0;
		else
			{
			/* for all others, if we're turning the property on, ungray it,
			else gray it; special case Kul because of a hand-native
			assumption in ApplyPrlSgc that val is <= 7*/
			val = (ValFromPropSprm(pprop, sprm) ? 0 :
					(sprm == sprmCKul ? ((1 << cbitsKul) - 1) : -1));
			}

		cchPrl = CchPrlFromSprmVal(prl, sprm, val);
		ApplyPrlSgc((char HUGE *) prl, cchPrl, ppropGray, sgc);
		}

	DisplaySRBanter((vtmcFocus == tmcSearch ? tmcSearchBanter : tmcReplaceBanter),
			&pfb->chp, &pfb->chpGray, &pfb->pap, &pfb->papGray);
}



/*
*  F  G E T  S  R  F B
*
*  Gets Search/Replace formatting block which is applicable to the
*  edit control that currently has the focus.  Returns fFalse if no
*  edit control has the focus.  Assumes that either the Search or
*  Replace dialog is up.
*
*/

/* %%Function:FGetSRFb %%Owner:rosiep */
BOOL FGetSRFb(ppfb)
struct FB **ppfb;
{
	extern int vfAwfulNoise;

	Assert (vfSearchRepl);

	if (vtmcFocus == tmcSearch)
		*ppfb = &vfbSearch;
	else  if (vtmcFocus == tmcReplace)
		*ppfb = &vfbReplace;
	else
		{
		Beep();
		vfAwfulNoise = fFalse;
		return (fFalse);
		}

	return (fTrue);
}



#ifdef HISTORICAL /* this has been incorporated into CpSearchSz() */
/*
*  C P  S E A R C H  F B
*
*  returns cpFirst of first run of characters that match vfbSearch.chp
*   (cpNil if not found)
*  sets vcpMatchLim to cpLim of that run
*
*/

/* %%Function:CpSearchFb %%Owner:rosiep */
CP CpSearchFb( pfb, cpFirst, cpLim )
struct FB *pfb;
CP cpFirst, cpLim;
{
	CP cpMatchFirst;
	int ccpFetch;

	/* get the simple case out of the way first */
	if (!pfb->fChp)
		return (CpSearchPap(cpFirst, cpLim));

	for (cpMatchFirst = cpNil; cpFirst < cpLim; cpFirst = vcpFetch + ccpFetch)
		{
		/* get next run */
		FetchCpPccpVisible(selCur.doc, cpFirst, &ccpFetch, 
				selCur.ww/*fvcScreen*/, fFalse);
		if (ccpFetch == 0)
			break;

		/* check for invisible chars skipped */
		if (cpFirst != vcpFetch && cpMatchFirst != cpNil)
			break;

		/* check to see if properties we're interested in match */
		if (FMatchChp()
				&& (!pfb->fPap || FMatchPap()))
			{
			vcpMatchLim = vcpFetch + ccpFetch;
			if (cpMatchFirst == cpNil)
				cpMatchFirst = vcpFetch;
			}
		/* check for end of matched run */
		else  if (cpMatchFirst != cpNil)
			break;
		}

	return (cpMatchFirst);
}


#endif /* HISTORICAL */



#ifdef HISTORICAL /* this has been incorporated into CpSearchSzBackward() */
/*
*  C P  S E A R C H  F B  B A C K W A R D
*
*  Same as CpSearchFb, only backward.
*
*/

/* %%Function:CpSearchFbBackward %%Owner:rosiep */
CP CpSearchFbBackward( pfb, cpFirst, cpLim )
struct FB *pfb;
CP cpFirst, cpLim;
{
	CP    cpMatchFirst;
	int    ccpFetch;
	int    icpFetchMac;            /* number of entries in rgcpFetch */
	CP    rgcpFetch[cbSector];    /* record of beginnings of visible runs */

	/* if cbSector changes someone should look at rgcpFetch to
		make sure it doesn't take too much stack. */
	Assert(cbSector == 128);
	icpFetchMac = 0;

	/* get the simple case out of the way first */
	if (!pfb->fChp)
		return (CpSearchPap(cpFirst, cpLim));

	cpMatchFirst = cpNil;
	for (vcpMatchLim = cpNil; cpFirst < cpLim; cpLim = vcpFetch)
		{
		/* get next run */
		FetchCpPccpVisibleBackward(selCur.doc, cpLim, &ccpFetch,
				selCur.ww/*fvcScreen*/, fFalse, &icpFetchMac, rgcpFetch);
		if (ccpFetch == 0)
			break;

		/* check for invisible chars skipped */
		if (cpLim != vcpFetch + ccpFetch && vcpMatchLim != cpNil)
			break;

		/* check to see if properties we're interested in match */
		if (FMatchChp()
				&& (!pfb->fPap || FMatchPap()))
			{
			cpMatchFirst = vcpFetch;
			if (vcpMatchLim == cpNil)
				vcpMatchLim = vcpFetch + ccpFetch;
			}
		/* check for end of matched run */
		else  if (vcpMatchLim != cpNil)
			break;
		}

	return (cpMatchFirst);
}


#endif /* HISTORICAL */



#ifdef HISTORICAL /* this has been incorporated into CpSearchSz() */
/*
*  C P  S E A R C H  P A P
*
*  Returns cpFirst of first paragraph that matches vfbSearch.pap
*  (cpNil if none found).  Sets vcpMatchLim to cpLim of that paragraph.
*  A paragraph that is entirely invisible will not be found.
*
*  Works both forward and backward.  If forward, starts at cpFirst
*  and goes up to, but not including cpLim.  If backward, starts at
*  cpLim - 1 and goes to cpFirst.
*
*  Note: this will find the current para if you're in the middle of
*  one (both forward and backward)
*
*/

/* %%Function:CpSearchPap %%Owner:rosiep */
NATIVE CP CpSearchPap( cpFirst, cpLim ) /* WINIGNORE - not used */
CP cpFirst, cpLim;
{
	CP cpPara = (vfFwd ? cpFirst : cpLim - 1);
	struct CA caT;

	for (;;)
		{
		if ((vfFwd && cpPara >= cpLim) || (!vfFwd && cpPara < cpFirst))
			return (cpNil);

		CachePara(selCur.doc, cpPara);

		/* save caPara because CpVisibleCp doesn't preserve it */
		caT = caPara;

		/* check if properties match and the paragraph is visible */
		if (FMatchPap() && CpVisibleCp(caPara.cpFirst) < caT.cpLim)
			{
			vcpMatchLim = caT.cpLim;
			return (caT.cpFirst);
			}

		/* set up for next CachePara */
		cpPara = (vfFwd ? caT.cpLim : caT.cpFirst - 1);
		}
}


#endif /* HISTORICAL */


#ifdef HISTORICAL /* this has been incorporated into CpSearchSz() */
/*
*  C P  V I S I B L E  C P
*
*  Returns the first visible cp at or after cp.  Does not guarantee caPara
*  will be preserved (because FetchCpPccpVisible calls CachePara).
*
*/

/* %%Function:CpVisibleCp %%Owner:rosiep */
NATIVE CP CpVisibleCp(cp) /* WINIGNORE - not used */
CP cp;
{
	int ccpFetch;

	FetchCpPccpVisible(selCur.doc, cp, &ccpFetch, selCur.ww/*fvcScreen*/, fFalse);
	return (vcpFetch);
}


#endif /* HISTORICAL */




/* %%Function:FMatchChpCpCp %%Owner:rosiep */
BOOL FMatchChpCpCp(cpFirst, cpLim)
CP cpFirst, cpLim;
{
	/* Note: can do FetchCp here because we know we have visible chars at
		this point */

	FetchCpAndPara(selCur.doc, cpFirst, fcmProps);
	while (vcpFetch < cpLim && FMatchChp())
		FetchCpAndPara(selCur.doc, vcpFetch + vccpFetch, fcmProps);
	if (vcpFetch >= cpLim)
		return (fTrue);
	else
		return (fFalse);
}


/* %%Function:FMatchPapCp %%Owner:rosiep */
BOOL FMatchPapCp(cpFirst)
CP cpFirst;
{
	CachePara(selCur.doc, cpFirst);
	return (FMatchPap());
}


