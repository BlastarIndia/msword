/* ****************************************************************************
**
**      COPYRIGHT (C) 1987, 1988 MICROSOFT
**
** ****************************************************************************
*
*  Module: pic.c ---- Main picture routines
*
**
** REVISIONS
**
** Date         Who    Rel Ver     Remarks
** 11/16/87     bobz               split off pic2.c
**
** ************************************************************************* */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "file.h"
#include "props.h"
#include "format.h"
#include "sel.h"
#include "disp.h"
#include "doc.h"
#include "screen.h"
#include "ch.h"
#include "pic.h"
#include "prm.h"
#include "field.h"
#include "doslib.h"
#include "print.h"
/* #include "dlbenum.h" */
#include "fkp.h"
#include "cmd.h"
#include "ff.h"
#include "grtiff.h"
#include "idle.h"
#include "keys.h"
#include "debug.h"
#include "error.h"
#include "help.h"
#include "layout.h"
#include "preview.h"
#include "grstruct.h"
#include "rareflag.h"

#ifdef PCODE
typedef long ul;   /* no unsigned longs in CS compiler */
#else
typedef unsigned long ul;
#endif


/* G L O B A L S */
FC              vfcFetchPic = fcNil;    /* fc of last fetched pe */
struct PIC      vpicFetch;      /* picture prefix from FetchPe */
int             vfDrawPics = fFalse;  /* if false draw only pic frame */
/* for pic display interruption */
BOOL            vfGrInterrupt = fFalse;


/* E X T E R N A L S */

extern FC               vfcFetchPic;    /* fc of last fetched pe */
extern struct PRI	vpri;
extern int              vflm;
extern int              fnFetch;
extern int              vbchrMac;
extern char             (**vhgrpchr)[];
extern char HUGE        *vhpchFetch;
extern struct WWD       **hwwdCur;
extern struct FLI       vfli;
extern struct PIC       vpicFetch;
extern struct CHP       vchpFetch;
extern CP               vcpFetch;
extern struct SEL       selCur;
extern struct FMTSS     vfmtss;
extern struct DOD       **mpdochdod[];
extern struct FTI       vfti;
extern struct FTI       vftiDxt;
extern struct MERR      vmerr;
extern struct SCI       vsci;
extern struct PREF      vpref;
extern struct ESPRM     dnsprm[];
extern struct FKPD      vfkpdText;
extern BOOL             vfDoubleClick;
extern int		cHpFreeze;
extern struct PAP       vpapFetch;
extern int              vfDrawPics;
extern struct WWD       **mpwwhwwd[];
extern BOOL             vfDeactByOtherApp;
extern struct DRDL      vdrdlDisplayFli;
extern BOOL             vfGrInterrupt;
extern FARPROC lpFMFContinue;
extern IDF		vidf;
extern int              vlm;
extern int	        vfnPreload;
extern HWND             vhwndCBT;
extern int vwwClipboard;
extern int				vdocFetch;
extern HBITMAP vhbmPatVert;  /* something to select when we dump our bm */
extern int              vwWinVersion;

uns					cKBitmap;

#ifdef NOASM
FC FcAppendRgchToFn();
#endif /* NOASM */
DWORD GetRgbIco();
BOOL FClipRcPic();

/****************************/
/* F  C r o p  G e t  P i c  S i z e s */
/*  %%Function:  FCropGetPicSizes  %%Owner:  bobz       */

FCropGetPicSizes( doc, cp, pchp, ppicdim)
int doc;
CP cp;
struct CHP *pchp;
struct PICDIM *ppicdim;

/* Given picture doc, cp, pchp, return size of picture, in "vfti device"
	pixels. We return sizes for the picture itself, with and without
	scaling, the crop frame before and after adding the border in a PICDIM.
	Returns true if any cropping done.
*/
{

	struct FTI *pfti;
	int fCrop = fFalse;
	int dzp;

/* Question: Can we make the optimization that Fetch has been done & is
	therefore not necessary?
	Answer: Yes! - In fact, since we now pass in the pchp, we don't need to
		do a fetch to get the information to do this action. We do
		assume however, that vfti, vftiDxt and vfli.fFormatAsPrint are
		correctly set up when we get here, since picture sizes are
		dependent on the device displayed upon.
*/

	/* we assume that pchp is for a picture */
	Assert( pchp->fSpec );

#ifdef PICCHP
	CommSzRgNum(SzShared("vpicFetch during GetPicSizes before FetchPe: "),
			&vpicFetch, cbPIC >> 1);
	CommSzRgNum(SzShared("*pchp before FetchPE: "), pchp, cwCHP);
#endif

	/* Note that FetchPe will set pchp->fnPic to fnFetch */

	FetchPe( pchp, doc, cp );

#ifdef PICCHP
	CommSzRgNum(SzShared("vpicFetch during GetPicSizes: "),
			&vpicFetch, cbPIC >> 1);
	CommSzNum(SzShared("vpicFetch.pictp: "), vpicFetch.pictp);
#endif

	/* MAJOR NOTE:
		the sizes are all calculated in "zt" units, i.e., printer pixels
		unless NOT display as printed. We save conversion back to screen
		units to the end for disp as print which we compute with rounding
		down to avoid pageview problems with the printer-> screen conversion
	*/

	pfti = vfli.fFormatAsPrint ? &vftiDxt : &vfti;

	/* get unscaled size of picture */

	if (vpicFetch.mfp.mm == MM_BITMAP ||
			vpicFetch.mfp.mm == MM_TIFF)
		{
		GetBitmapSize(&(ppicdim->picUnscl.xp),
				&(ppicdim->picUnscl.yp));
		} /* bitmap/tiff */
	else

		/* use calculated goal size in main device units for metafiles.
			Calculate in printer pixels, and compute xp's using nonrounding
			mechanism for dispasprint so we avoid pageview screen pushout.
		*/
		{
		Assert (vpicFetch.mfp.mm <= MM_META_MAX);
		ppicdim->picUnscl.xp = NMultDiv (vpicFetch.dxaGoal,
				pfti->dxpInch, czaInch);
		Assert (ppicdim->picUnscl.xp != 0x7FFF);  /* overflow should be caught before now */
		ppicdim->picUnscl.yp = NMultDiv (vpicFetch.dyaGoal,
				pfti->dypInch, czaInch);
		Assert (ppicdim->picUnscl.yp != 0x7FFF);
		}

	/* apply the user's "ideal multiple" of the computed size */
	/* in Write,  we only applied mx, my to bitmaps. Here we do it
	for metafiles too, so do it at this point  bz
	
	UMultDiv used because mx, my are unsigned and can be > 32k
	but the result should be < 32k. 
	*/

	ppicdim->picScl = ppicdim->picUnscl;

	if (vpicFetch.mx != mx100Pct)
		{
		dzp =  UMultDiv(ppicdim->picScl.xp, vpicFetch.mx, mx100Pct );
		if (dzp > 0)
			ppicdim->picScl.xp = dzp;
		else
			PictError(eidBadScale);
		}

	if (vpicFetch.my != my100Pct)
		{
		dzp =  UMultDiv(ppicdim->picScl.yp, vpicFetch.my, mx100Pct );
		if (dzp > 0)
			ppicdim->picScl.yp = dzp;
		else
			PictError(eidBadScale);
		}

#ifdef XBZTEST
	CommSzNum(SzShared("GetPicSizes vpicFetch.mx: "), vpicFetch.mx);
	CommSzNum(SzShared("GetPicSizes vpicFetch.my: "), vpicFetch.my);
#endif

	/* get size of cropping frame */
	/* cropping values are applied to the unscaled picture rect, then
		the resulting rectangle is scaled, so we can accomplish the effect
		of scaling the cropped picture, not the other way around.
	*/

	if (!vpicFetch.dxaCropLeft && !vpicFetch.dyaCropTop &&
			!vpicFetch.dxaCropRight && !vpicFetch.dyaCropBottom)
		{
		/* no cropping */
		ppicdim->frIn = ppicdim->picUnscl;
		}
	else
		{
		int        dzpCrop;

		fCrop = fTrue;

		/* hoo boy, here is the cropping story. The dxa values are wrt the
		goal size. Since the size on screen may be very different from the
		goal size converted from twips to pixels - e.g. a 1"x 1" pic (goal)
		from 300 dpi shows up at 75dpi as 4"x4", you can't just take the
		dzpCrop values and convert to pixels (a 1/2" crop, 720 twips, would
		show up at 1/2 screen inch (48 pixels at ega xp) when it should
		show up as 2 screen inches, since it is half of the original
		width.) If you don't play this game, you can crop more screen picture
		than there is real picture - 2 screen inches converted directly to
		twips will blow away the goal picture.
		
		So, we take the dza's and make them the same proportion of the
		screen rect as they were of the goal rect.
		(dza/dzaGoal = dzp/dzpUnscl). Solve for dzp.
		
		*/

		dzpCrop = NMultDiv( vpicFetch.dxaCropLeft + vpicFetch.dxaCropRight,
				ppicdim->picUnscl.xp, vpicFetch.dxaGoal);

		ppicdim->frIn.xp = ppicdim->picUnscl.xp - dzpCrop;
		if (ppicdim->frIn.xp < 0 || dzpCrop == 0x7fff)
			{
			ppicdim->frIn.xp = ppicdim->picUnscl.xp;
			PictError(eidBadCrop);
			}

		dzpCrop = NMultDiv( vpicFetch.dyaCropTop + vpicFetch.dyaCropBottom,
				ppicdim->picUnscl.yp, vpicFetch.dyaGoal);

		ppicdim->frIn.yp = ppicdim->picUnscl.yp - dzpCrop;
		if (ppicdim->frIn.yp < 0 || dzpCrop == 0x7fff)
			{
			ppicdim->frIn.yp = ppicdim->picUnscl.yp;
			PictError(eidBadCrop);
			}
		}

	/* now scale the (possibly cropped) picture */

	if (vpicFetch.mx != mx100Pct)
		{
		dzp =  UMultDiv(ppicdim->frIn.xp, vpicFetch.mx, mx100Pct );
		if (dzp > 0)
			ppicdim->frIn.xp = dzp;
		else
			{
			PictError(eidBadScale);
			}
		}

	if (vpicFetch.my != my100Pct)
		{
		dzp =  UMultDiv(ppicdim->frIn.yp, vpicFetch.my, my100Pct );
		if (dzp > 0)
			ppicdim->frIn.yp = dzp;
		else
			{
			PictError(eidBadScale);
			}
		}
	/* adjust for borders */

	if (vpicFetch.brcl == brclNone)   /* no frame border? */
		ppicdim->frOut = ppicdim->frIn;
	else
		{
		struct RC rcBrdr;
		GetBrdrLineSizes (vpicFetch.brcl, NULL, &rcBrdr, fFalse /* fScaleForScreen */);
		ppicdim->frOut.xp = ppicdim->frIn.xp + rcBrdr.xpLeft + rcBrdr.xpRight;
		ppicdim->frOut.yp = ppicdim->frIn.yp + rcBrdr.ypTop + rcBrdr.ypBottom;
		}

	ppicdim->frOutZt = ppicdim->frOut;

	/* scale all but the final zt value to screen units without rounding */
	if (vfli.fFormatAsPrint)
		{
		struct RC rcBrdr;
		ScaleZtForScreen(ppicdim, (sizeof(struct PICDIM) /sizeof(struct PT)) - 1);

		/* now, due to border scaling problems, recalculate the inside rect
			from the scaled outside rect and scaled borders. Have to also
			adjust the picScl rect. Don't bother with the unscaled rect
			which is used only for dragging comparisons.
		*/
		if (vpicFetch.brcl != brclNone)
			{
			int dxp, dyp;

			GetBrdrLineSizes (vpicFetch.brcl, NULL, &rcBrdr, fTrue /* fScaleForScreen */);

			dxp = ppicdim->frIn.xp - (ppicdim->frOut.xp - rcBrdr.xpLeft - rcBrdr.xpRight);
			dyp = ppicdim->frIn.yp - (ppicdim->frOut.yp - rcBrdr.ypTop - rcBrdr.ypBottom);
			if (dxp)
				{
				ppicdim->frIn.xp -= dxp;
				ppicdim->picScl.xp -= dxp;
				}
			if (dyp)
				{
				ppicdim->frIn.yp -= dyp;
				ppicdim->picScl.yp -= dyp;
				}

			}
		}


#ifdef DPICRC
	CommSzNum(SzShared("FCropGetPicSizes vfti.dxpInch: "), vfti.dxpInch);
	CommSzNum(SzShared("FCropGetPicSizes vfti.dypInch: "), vfti.dypInch);
	CommSzNum(SzShared("FCropGetPicSizes dxaGoal: "), vpicFetch.dxaGoal);
	CommSzNum(SzShared("FCropGetPicSizes dyaGoal: "), vpicFetch.dyaGoal);

	CommSzNum(SzShared("FCropGetPicSizes dxpFrameOut: "), ppicdim->frOut.xp);
	CommSzNum(SzShared("FCropGetPicSizes dypFrameOut: "), ppicdim->frOut.yp);
	CommSzNum(SzShared("FCropGetPicSizes dxpFrameIn: "), ppicdim->frIn.xp);
	CommSzNum(SzShared("FCropGetPicSizes dypFrameIn: "), ppicdim->frIn.yp);
	CommSzNum(SzShared("FCropGetPicSizes dxpPicScl: "), ppicdim->picScl.xp);
	CommSzNum(SzShared("FCropGetPicSizes dypPicScl: "), ppicdim->picScl.yp);
	CommSzNum(SzShared("FCropGetPicSizes dxpPicUnscl: "), ppicdim->picUnscl.xp);
	CommSzNum(SzShared("FCropGetPicSizes dypPicUnscl: "), ppicdim->picUnscl.yp);
#endif

	return (fCrop);
}


/*********************************/
/*  %%Function:  GetBrdrLineSizes   %%Owner:  bobz       */

GetBrdrLineSizes (brcl, pbrdr, prcBrdr, fScaleForScreen)
int brcl;
struct BRDR *pbrdr;
struct RC *prcBrdr;
int fScaleForScreen;
{
	struct FTI *pfti;
	int dxpBorder, dypBorder;

	/* fill out brdr and rcBrdr (if non null)  */

/* convert point widths and heights to pixels and store in a rect
	put line width/height in top left point, spacing in lower point.
	This chunk is same as GetBrclSizes. Here for swap tuning.

	Due to page view scaling issues, we always try to calculate in vftiDxt
	units and scale them, unrounded, into screen units for display
	as printed. However, since vftiDxt has no borders, and since the printer
	fti scales from vsci, and there would be loss in the scale up and back,
	we are doing this:
		if !formatAsPrint, use vfti values
		else
			{
			if scaling for screen, use vfti values, which come from vsci
			else
				scale the vsci values as setflm does.
			}

	
*/
	if (vfli.fFormatAsPrint && !fScaleForScreen)
		{
		dxpBorder = UMultDiv(vsci.dxpBorder, vfli.dxuInch, vfli.dxsInch);
		dypBorder = UMultDiv(vsci.dypBorder, vfli.dyuInch, vfli.dysInch);
		}
	else
		{
		dxpBorder = vfti.dxpBorder;
		dypBorder = vfti.dypBorder;
		}

	if (pbrdr != NULL)
		{
		pbrdr->dxpLineWidth = pbrdr->dxpLineSpacing = dxpBorder;
		pbrdr->dypLineHeight = pbrdr->dypLineSpacing = dypBorder;
		}

	if (prcBrdr != NULL)
		{
		switch (brcl)
			{
		case brclSingle:
			prcBrdr->xpLeft = prcBrdr->xpRight = dxpBorder;
			prcBrdr->ypTop = prcBrdr->ypBottom = dypBorder;
			break;
		case brclThick:
			prcBrdr->xpLeft = prcBrdr->xpRight = dxpBorder << 1;
			prcBrdr->ypTop = prcBrdr->ypBottom = dypBorder << 1;
			break;
		case brclDouble:
			/* borders + spacing on each edge */
			prcBrdr->xpLeft = prcBrdr->xpRight =
					dxpBorder * 3;
			prcBrdr->ypTop = prcBrdr->ypBottom =
					dypBorder * 3;
			break;
		case brclShadow:
			prcBrdr->xpLeft = dxpBorder;
			prcBrdr->xpRight = dxpBorder << 1;
			prcBrdr->ypTop = dypBorder;
			prcBrdr->ypBottom = dypBorder << 1;
			break;
		default:  /* for brclNone */
			SetBytes(prcBrdr, 0, sizeof (struct RC));
			}
		}
}


/* convert cDim xt, yt pairs to screen xp yp's with no rounding for pageview*/

/*  %%Function:  ScaleZtForScreen  %%Owner:  bobz       */

ScaleZtForScreen(pDim, cDim)
struct PT *pDim;
int cDim;
{
	int i;

	Assert (vfli.fFormatAsPrint);

	if (!vftiDxt.dxpInch || !vftiDxt.dypInch)
		{
		Assert (fFalse); /* shouldn't be called in this case */
		return;
		}
	for (i = 0; i < cDim; i++)
		{
		/* Note: these long mult/divs are equivalent to calling
			UMultDivNR and are done without rounding for PageView, as
			formatline does it.
		*/

		pDim->xp = (int)((long)pDim->xp * (long)vfti.dxpInch /
				(long) vftiDxt.dxpInch);
		Assert (pDim->xp > 0); /* if not, lost in the translation */
		pDim->yp = (int)((long)pDim->yp * (long)vfti.dypInch /
				(long) vftiDxt.dypInch);
		Assert (pDim->yp > 0);
		pDim++;
		}

}



/*********************************/
/*  G e t  P i c t u r e  I n f o */
/*  %%Function:  GetPictureInfo  %%Owner:  bobz       */

GetPictureInfo(bchrChp, ich, pdxp, pdxt, pdyyAscent, pdyyDescent)
int bchrChp, ich, *pdxp, *pdxt;
DYY *pdyyAscent, *pdyyDescent;
{
/* return the height (in ascent/descent) and width of the specified picture;
	also sets the fn in the chp
*/
	struct PICDIM picdim; /* contains sizes of pic and its frame */
	struct CHR *pchr;
	int bchr = 0;
	struct CHP chp;


	Profile( vpfi == pfiPictFmt ? StartProf( 30) : 0);

	/* find the CP for the picture, taking vanished text into account */
	for (vfmtss.cpRef = vfli.cpMin + ich; bchr < vbchrMac; bchr += pchr->chrm)
		{
		pchr = (char *) **vhgrpchr + bchr;
		if (pchr->ich > ich)
			break;
		if (pchr->chrm == chrmVanish)
			vfmtss.cpRef += ((struct CHRV *) pchr)->dcp;
 		else if (pchr->chrm == chrmDisplayField)
 			vfmtss.cpRef += ((struct CHRDF *)pchr)->dcp - 1; /*because of ich above*/
		}

	/* we assume that the picture has just been fetched */
	Assert(*vhpchFetch == chPicture && vchpFetch.fSpec);


	pchr = (char *) **vhgrpchr + bchrChp;
	chp = pchr->chp;
	FCropGetPicSizes( vfli.doc, vfmtss.cpRef, &chp, &picdim );

	/* format only cares about the dxp/dyp for the
		picture frame bz
	*/
	pdyyAscent->dyp = picdim.frOut.yp;
	pdyyAscent->dyt = picdim.frOutZt.yp;

	pdyyDescent->wlAll = 0;
	/* To allow space for underline */
	if (chp.kul != kulNone)
		{
		int dys = vsci.dypBorder * 2;
		int dyu = UMultDiv(czaPoint * 2, vfti.dxpInch,czaInch);

		pdyyDescent->dyp = vfli.fPrint ? dyu : dys;
		pdyyDescent->dyt = dyu;
		}
	*pdxp = picdim.frOut.xp;
	*pdxt = picdim.frOutZt.xp;

	Profile( vpfi == pfiPictFmt ? StopProf() : 0);

#ifdef XBZ
	CommSzNum(SzShared("GetPictureInfo dxp: "), *pdxp);
	CommSzNum(SzShared("GetPictureInfo dxt: "), *pdxt);
	CommSzNum(SzShared("GetPictureInfo dyyDescent.dyp: "), pdyyDescent->dyp);
	CommSzNum(SzShared("GetPictureInfo dyyDescent.dyt: "), pdyyDescent->dyt);
	CommSzNum(SzShared("GetPictureInfo dyyAscent.dyp: "), pdyyAscent->dyp);
	CommSzNum(SzShared("GetPictureInfo dyyAscent.dyt: "), pdyyAscent->dyt);
#endif
	return;
}


/******************/
/* FF e t c h  S e l P i c R c */
/*  Get various rectangles relevant to pictures.

		
	Fill a PICRC structure, which is a bunch of RC's including:
		frOut - the outside frame rectangle. This is the normal picture
				rect that format uses. It includes cropping and the
				border thickness.
		frIn - the inside frame rectangle. This is what a picture is actually
				clipped to by cropping. It excludes the border sizes, and its
				origin and size are offset by the border sizes.
		picScl - the scaled picture. Its size is the scaled picture size; its
				origin is offset relative to the inside frame rectangle so that
				we first pretend the picture is at the origin and position the
				IF rect relative to the crop and border offsets from the pic
				origin. The we shift the pic origin as if the IF rect was at
				the origin
		picUnscl - unscaled picture. This we use only for the size, so its
				origin is irrelevant. For ease, we place the origin at the
				border origin.
*/

/*  %%Function:  FFetchSelPicRc   %%Owner:  bobz       */

FFetchSelPicRc (psel, pPicrc, pfCrop, grpfPic, fInterrupt )
struct SEL *psel;
struct PICRC *pPicrc;
BOOL *pfCrop;
int grpfPic;
int fInterrupt;
{

	struct CHP *pchp;
	int dl, idr;
	int xp, yp;
	int xpT, ypT;
	struct PICDIM picdim;
	int ichT;
	CP cp;
	CP dcp;
	struct PLDR **hpldr;
	struct DR *pdr;
	struct PLCEDL **hplcedl;
	struct EDL edl;
	struct DRF drfFetch;
	int ww = psel-> ww;
	struct WWD  **hwwd = HwwdWw(ww);

	/* return the picture rectangles specified by the grpf in pPicrc */
	/* return fFalse if some error occurs */

	Assert (psel->fGraphics);
	Assert (grpfPic);  /* if 0, nothing requested */
	/*
	1. get xp, yp of pic cp. Note: xp, yp are the BOTTOM left corner,
		so the code will be similar to that used in DrawChPic.
	*/

	Assert (!fInterrupt);   /* currently no call should allow
		interrupt bz 1/27/87 */

	UpdateWw(ww, fInterrupt /* fAbortOK */);  /* window must be up-to-date */

	if ((*hwwd)->fDirty ||
			(dl = DlWhereDocCp(ww, psel->doc, psel->cpFirst, fFalse/*fEnd*/, &hpldr, &idr, NULL, NULL,fTrue)) == dlNil)
		{
		SetBytes(pPicrc, -1, sizeof (struct PICRC));
		*pfCrop = fFalse;
		return (fFalse);
		}

	pdr = PdrFetch(hpldr, idr, &drfFetch);
	cp = CpPlc(hplcedl = pdr->hplcedl, dl); /* cpFirst in dl */
	FormatLineDr(psel->ww, cp, pdr);
	dcp = psel->cpFirst - cp;
	XpFromDcp(dcp, dcp + 1, &xp, &ichT);

	/* get chp; can't use psel->chp since it is never set up to be
		fSpec.
	*/

	FetchCpAndParaCa(&psel->ca, fcmProps);
	Assert(vchpFetch.fSpec);
	/* Note:
	FetchCp never sets fnPic since it is possible for
	fnPic to change during the lifetime of the fetch.
	All users of pic chps are required to set fnPic to
	fnFetch. This is one case where it is acceptable to
	modify vchpFetch without trashing the fetch. The only
	user who really cares it FetchPe, so it sets fnPic and
	its callers don't have to    bz
	*/

	GetPlc(hplcedl, dl, &edl);
	FreePdrf(&drfFetch);

#ifdef XBZTEST
	CommSzNum(SzShared("FetchPicRc xp: "), xp);
#endif
	/* heap pointer! */
	return (FGetPicRc (pPicrc, pfCrop, &vchpFetch,
			XwFromXp(hpldr, idr, xp),
			YwFromYp(hpldr, idr, edl.ypTop + edl.dyp),
			psel, grpfPic));
}


/*  %%Function:  FGetPicRc   %%Owner:  bobz       */

FGetPicRc (pPicrc, pfCrop, pchp, xp, yp, psel, grpfPic )
struct PICRC *pPicrc;
BOOL *pfCrop;
struct CHP *pchp;
int xp, yp;
struct SEL * psel;
int grpfPic;
{

	int dl;
	int xpT, ypT;
	struct PICDIM picdim;
	struct RC rcBrdr;  /* gets thicknesses of each border line in
				xp, yp fields */
	int dxpCropLeft;
	int dypCropTop;
	int ichT;
	int doc;
	CP cpFirst;
	unsigned dcp;
	int hps;


	/* get picture rectangle.
	2. get sizes
	*/
	Assert (pchp->fSpec);

	doc = (psel != NULL) ? psel->doc : vfli.doc;
	cpFirst = (psel != NULL) ? psel->cpFirst : vfmtss.cpRef;

	*pfCrop = FCropGetPicSizes(doc, cpFirst, pchp, &picdim);

	GetBrdrLineSizes (vpicFetch.brcl, NULL, &rcBrdr, fTrue /* fScaleForScreen */);

/* when we know it's the selection, baseline and sub/super-script have NOT
	been taken into account, so do it now */
	if (psel != NULL)
		{
		yp -= vfli.dypBase;
		if (hps = pchp->hpsPos)
			{
			if (hps < 128)
				yp -= (NMultDiv( hps, vfti.dypInch, 144 ));  /* superscript */
			else
				yp += (NMultDiv( (256 - hps), vfti.dypInch, 144 ));  /* subscript */
			}
		}

	/* outside frame border: rectangle of the cropped frame with
		any border size added in
	*/

	if (grpfPic & picgFrOut)
		SetRect ((LPRECT)&pPicrc->frOut,
				xp, yp - picdim.frOut.yp,
				xp + picdim.frOut.xp, yp);

	/* inside frame border: rectangle of the cropped frame with
		no border size added in. Origin offset from xp, yp by
		border.
	*/
	/* picscl needs this one too */

	if (grpfPic & picgFrIn ||  grpfPic & picgPicScl)
		SetRect ((LPRECT)&pPicrc->frIn,
				xpT = xp + rcBrdr.xpLeft,
				(ypT = yp - rcBrdr.ypBottom) - picdim.frIn.yp,
				xpT + picdim.frIn.xp,
				ypT);

	/* Scaled pic rect: here we effectively put the cropping rectangle over the
				scaled picture with the picture at xp, yp, then shift
				the picture origin as if the inside cropping frame was
				at xp, yp. Since the we scale a cropped picture, the
				effect is that the cropping is scaled; this was taken
				into account in computing the frame rects, now adjust
				the cropping when determining the offset.
			*/

	if (grpfPic & picgPicScl)
		{
		BOOL fNeg;
		int dzp;

		if (*pfCrop)
			{
			/* see hoo boy note above. He are making the cropping
				be in the same proportion to the screen rect as it was
				to the goal rect, for arcane reasons
			*/
			dxpCropLeft = NMultDiv( vpicFetch.dxaCropLeft,
					picdim.picUnscl.xp, vpicFetch.dxaGoal);
			dypCropTop =  NMultDiv( vpicFetch.dyaCropTop,
					picdim.picUnscl.yp, vpicFetch.dyaGoal);

			if (vpicFetch.mx != mx100Pct)
				{
				/* neg lets us do an unsigned mult with scale, which could be > 32767 */
				if (fNeg = (dxpCropLeft < 0))
					dxpCropLeft = -dxpCropLeft;
				/* on overflow, ignore scaling */
				if ((dzp = UMultDiv( (unsigned)dxpCropLeft,
						vpicFetch.mx, mx100Pct )) > 0)
					dxpCropLeft = dzp;
				if (fNeg)
					dxpCropLeft = -dxpCropLeft;
				}
			if (vpicFetch.my != my100Pct)
				{
				if (fNeg = (dypCropTop < 0))
					dypCropTop = -dypCropTop;
				if ((dzp = UMultDiv( (unsigned)dypCropTop,
						vpicFetch.my, my100Pct )) > 0)
					dypCropTop = dzp;
				if (fNeg)
					dypCropTop = -dypCropTop;
				}
			}
		else
			dxpCropLeft = dypCropTop = 0;

#ifdef XBZ
		CommSzNum(SzShared("FGetPicRc dxaCropLeft: "), vpicFetch.dxaCropLeft);
		CommSzNum(SzShared("FGetPicRc dyaCropTop: "), vpicFetch.dyaCropTop);
		CommSzNum(SzShared("FGetPicRc dxaCropRight: "), vpicFetch.dxaCropRight);
		CommSzNum(SzShared("FGetPicRc dyaCropBottom: "), vpicFetch.dyaCropBottom);
		CommSzNum(SzShared("FGetPicRc dxpCropLeft: "), dxpCropLeft);
		CommSzNum(SzShared("FGetPicRc dypCropTop: "),  dypCropTop);
		CommSzRgNum(SzShared("FGetPicRc border sizes = "),
				&rcBrdr, 4);
#endif

		SetRect ((LPRECT)&pPicrc->picScl,
				xpT = pPicrc->frIn.xpLeft - dxpCropLeft,
				ypT = pPicrc->frIn.ypTop - dypCropTop,
				xpT + picdim.picScl.xp,
				ypT + picdim.picScl.yp);
		}

	/* Unscaled picture rect:  this one is fairly meaningless
		except for its size, so we place it at the xp, yp origin bz */

	if (grpfPic & picgPicUnscl)
		SetRect ((LPRECT)&pPicrc->picUnscl,
				xp, yp - picdim.picUnscl.yp,
				xp + picdim.picUnscl.xp, yp);

#ifdef XBZ
	if (grpfPic & picgFrOut)
		CommSzRgNum(SzShared("FGetPicRc outside frame rect = "),
				&pPicrc->frOut, 4);
	if (grpfPic & picgFrIn)
		CommSzRgNum(SzShared("FGetPicRc inside frame rect = "),
				&pPicrc->frIn, 4);
	if (grpfPic & picgPicScl)
		CommSzRgNum(SzShared("FGetPicRc scaled pic rect = "),
				&pPicrc->picScl, 4);
	if (grpfPic & picgPicUnscl)
		CommSzRgNum(SzShared("FGetPicRc unscaled pic rect = "),
				&pPicrc->picUnscl, 4);
#endif

	return (fTrue);
}



/******************/
/* F e t c h  P e */
/*  %%Function:  FetchPe  %%Owner:  bobz       */

FetchPe(pchp, docSrc, cpSrc)
struct CHP *pchp;       /* zero for next pe from same pic */
int docSrc;             /* location of chPicture */
CP cpSrc;
{
/* fetch picture element ipe from the specified picture document
	sprms will be applied to the pe before it is returned

	FetchPe keeps it's state squirreled away.  The call which reads the PIC
	establishes the state.  fnFetch stores the current fn the picture is
	being fetched from, vfcFetchPic stores the byte location.

	note: pchp->fnPic will be set to fnFetch here. This removes the need to
			have all callers do this, but callers should be aware that their
			chp will change.
*/

	struct PLC **hplcpcd;
	struct PCD pcd;

#ifdef XBZTEST
	LONG lTimer = GetTickCount ();
#endif /* XBZTEST */


	/* no doc supplied, create a doc or find one if it exists */

	if (pchp == 0)
		goto FetchFail; /* should not happen */
	else
		{
		/* Caller must assure that it is OK for FetchPe to modify
			chp in this way - in general, there should be no problem
			but if chp comapres are done there may be unexpected
			differences.
		*/


		pchp->fnPic = fnFetch;

		if (pchp->fDirty)
			{
			goto FetchFail;
			}
		else
			{
			vfcFetchPic = pchp->fcPic & 0x00FFFFFF;
			FetchPeData(vfcFetchPic, (char HUGE *)&vpicFetch, (LONG)cbPIC);

			/* for Opus, the lcb stored in the pic structure is the
				size of the picture plus the header. No change here.
			*/

			vfcFetchPic += (long) vpicFetch.cbHeader;
		/*  Header size may grow in future versions... */

			Assert(mpdochdod[docSrc]);
			hplcpcd = PdodDoc(docSrc)->hplcpcd;
			GetPlc( hplcpcd, IInPlc( hplcpcd, cpSrc ), &pcd );

			if (pcd.prm != prmNil)
				{
				/* apply sprms (the actual scaling we do ourselves) */
				/* sprms modify the brcl, mx, my, and cropping vals */
				DoPrmSgc(pcd.prm, &vpicFetch, sgcPic);

				Assert ((vpicFetch.brcl < 0  ||
						vpicFetch.brcl > brclNone) ?
						(vpicFetch.brcl == brclInval) : fTrue);
				}

			}   /* not dirty */
		}       /* pchp != 0 */


#ifdef XBZ
	CommSzNum(SzShared("docSrc during FetchPe: "), docSrc);
	CommSzLong(SzShared("cp during FetchPe: "), cpSrc);
	CommSzRgNum(SzShared("chp during FetchPE: "), pchp, cwCHP);
	CommSzLong(SzShared("chp.fcPic during FetchPe: "), pchp->fcPic & 0x00FFFFFF);
	CommSzNum(SzShared("chp.fnPic during FetchPe: "), pchp->fnPic);
#endif

#ifdef PICCHP
		{
		int fnT;
		struct PLC **hplcpcdT;
		struct PCD pcdT;

		hplcpcdT = PdodDoc(docSrc)->hplcpcd;
		GetPlc( hplcpcdT, IInPlc( hplcpcdT, cpSrc ), &pcdT );
		fnT = pcdT.fn;

		CommSzNum(SzShared("fnFetch during FetchPe: "), fnFetch);
		CommSzNum(SzShared("plc fn FetchPe: "), fnT);
		CommSzNum(SzShared("pchp->fnPic during FetchPe: "), pchp->fnPic);
		Assert (pchp->fnPic == fnT);
		}
#endif


#ifdef XBZTEST
	CommSzRgNum(SzShared("FetchPe - vpicFetch: "), &vpicFetch,
			CwFromCch(cbPIC));
#endif

#ifdef XBZTEST
	CommSzLong(SzShared("FetchPe time (usec):"), (long)((GetTickCount () - lTimer) );
#endif /* XBZTEST */



			return;

FetchFail:
			Assert (fFalse);
			/* Do we need to disable more? */
	vfcFetchPic = fcNil;
			SetBytes(&vpicFetch, 0, cbPIC);
			return; 
}


/************************************************************************/

/****************************/
/* F e t c h  P e   D a t a */
/*  %%Function:  FetchPeData  %%Owner:  bobz       */

FetchPeData(fc, hpch, lcb)
FC fc;
char HUGE *hpch;
LONG lcb;
{

	int cb;

			SetFnPos(fnFetch, fc);
			while (lcb > 0)
				{
		if (lcb < 0x7fff)
				cb = lcb;
				else
			cb = 0x7fff;
					ReadHprgchFromFn(fnFetch, hpch, cb);
					hpch += cb;
					lcb -= cb;
		}

}


/******************************/
/* D r a w  P i c  F r a m e */

/*  %%Function:  DrawPicFrame  %%Owner:  bobz       */

DrawPicFrame(hdc, prcFrame, pbrdr, brcl, ico)
HDC hdc;
struct RC *prcFrame;
struct BRDR *pbrdr;
int brcl;
int ico;
{

	int xp, yp, dxp, dyp;
	int fThick;
	HBRUSH hbr, hbrOld;
	HBRUSH hbrNew = NULL;
	DWORD dwRop = PATCOPY;

	Assert (brcl != brclNone);

	/* match border to color in pic */

	if (vlm != lmPreview)
		{
		if (ico == icoAuto)
			hbr = vfli.fPrint ? vpri.hbrText : vsci.hbrText;
		else
			{
			Assert (ico < icoMax);
			if ((hbr = hbrNew = CreateSolidBrush(GetRgbIco(ico))) == NULL)
				hbr = vfli.fPrint ? vpri.hbrText : vsci.hbrText;
			Debug(else  
				LogGdiHandle(hbrNew, 1031));
			}

		hbrOld = SelectObject(hdc, hbr);
		}

	fThick = (brcl == brclThick);

			/* draw 4 normal lines: top, bottom,  left, right
		top bottom full width, others only up to
				the other lines    */

	/* The rectangle we have is for the full size of the framed picture;
		For shadow frames, we need to draw the inside lines first, so adjust
		the rect accordingly (readjust back in the special shadow handling)
			*/

	if (brcl == brclShadow)
				{
		prcFrame->xpRight  -= pbrdr->dxpLineWidth;
				prcFrame->ypBottom -= pbrdr->dypLineHeight;
		}

#ifdef XBZ
		{
		struct RC rcT;
				CommSz(SzShared("DrawPicFrame starting draw\n"));
				CommSzRgNum(SzShared("DrawPicFrame rect size = "),
				prcFrame, 4);
				GetClipBox (hdc, (LPRECT)&rcT);
				CommSzRgNum(SzShared("DrawPicFrame hdc clip rect: "),
				&rcT, 4);
		}
#endif

			/* top */
	xp = prcFrame->xpLeft;
			yp = prcFrame->ypTop;
			dxp =  prcFrame->xpRight - prcFrame->xpLeft;
			dyp =  (pbrdr->dypLineHeight << fThick);

			if (vlm != lmPreview)
			PatBlt(hdc, xp, yp, dxp, dyp, dwRop);
			else
		DrawPrvwLine(hdc, xp, yp, dxp, dyp, colFetch);

				/* bottom */
			/* xp = prcFrame->xpLeft;                        *** unchanged  */

		yp = prcFrame->ypBottom - (pbrdr->dypLineHeight << fThick);

			/* dxp = prcFrame->xpRight - prcFrame->xpLeft;   *** unchanged */
			/* dyp = (pbrdr->dypLineHeight << fThick);       *** unchanged */

		if (vlm != lmPreview)
				PatBlt(hdc, xp, yp, dxp, dyp, dwRop);
				else
			DrawPrvwLine(hdc, xp, yp, dxp, dyp, colFetch);

					/* left */
			/* xp = prcFrame->xpLeft;  *** unchanged */

			yp = prcFrame->ypTop + (pbrdr->dypLineHeight << fThick);
					dxp = (pbrdr->dxpLineWidth << fThick);
					dyp = prcFrame->ypBottom - prcFrame->ypTop -
					( (pbrdr->dypLineHeight << fThick) << 1 );

					if (vlm != lmPreview)
					PatBlt(hdc, xp, yp, dxp, dyp, dwRop);
					else
				DrawPrvwLine(hdc, xp, yp, dxp, dyp, colFetch);

					/* right */
				xp = prcFrame->xpRight - (pbrdr->dxpLineWidth << fThick);

			/* yp = prcFrame->ypTop + (pbrdr->dypLineHeight << fThick); *** unchanged */
			/* dxp = (pbrdr->dxpLineWidth << fThick);  *** unchanged  */
			/* dyp = prcFrame->ypBottom - prcFrame->ypTop -  *** unchanged */
			/*         ( (pbrdr->dypLineHeight << fThick) << 1 ); *** unchanged */

				if (vlm != lmPreview)
						PatBlt(hdc, xp, yp, dxp, dyp, dwRop);
						else
					DrawPrvwLine(hdc, xp, yp, dxp, dyp, colFetch);

				/* special cases */

					if (brcl == brclDouble)
								{
						int xpT = pbrdr->dxpLineWidth + pbrdr->dxpLineSpacing;
								int ypT = pbrdr->dypLineHeight + pbrdr->dypLineSpacing;

						/* top */
						xp = prcFrame->xpLeft + xpT;
								yp = prcFrame->ypTop + ypT;
								dxp = prcFrame->xpRight - prcFrame->xpLeft - (xpT << 1);
								dyp = pbrdr->dypLineHeight;

								if (vlm != lmPreview)
								PatBlt(hdc, xp, yp, dxp, dyp, dwRop);
								else
							DrawPrvwLine(hdc, xp, yp, dxp, dyp, colFetch);

					/* bottom */
				/* xp unchanged */

							yp = prcFrame->ypBottom - ypT - pbrdr->dypLineSpacing; 

				/* dxp unchanged */
				/* dyp unchanged */

							if (vlm != lmPreview)
									PatBlt(hdc, xp, yp, dxp, dyp, dwRop);
									else
								DrawPrvwLine(hdc, xp, yp, dxp, dyp, colFetch);

					/* left */
				/* xp unchanged */

								yp = prcFrame->ypTop + ypT + pbrdr->dypLineHeight;
										dxp = pbrdr->dxpLineWidth;
										dyp = prcFrame->ypBottom - prcFrame->ypTop -
										(pbrdr->dypLineHeight << 1) - (ypT << 1);

										if (vlm != lmPreview)
										PatBlt(hdc, xp, yp, dxp, dyp, dwRop);
										else
									DrawPrvwLine(hdc, xp, yp, dxp, dyp, colFetch);


						/* right */

									xp = prcFrame->xpRight - xpT - pbrdr->dxpLineWidth;

				/* yp unchanged */
				/* dxp unchanged */
				/* dyp unchanged */

									if (vlm != lmPreview)
											PatBlt(hdc, xp, yp, dxp, dyp, dwRop);
											else
										DrawPrvwLine(hdc, xp, yp, dxp, dyp, colFetch);
						}

	if (brcl == brclShadow)
				{
				/* bottom */

		xp = prcFrame->xpLeft + pbrdr->dxpLineWidth;
				yp = prcFrame->ypBottom;
				dxp = prcFrame->xpRight - prcFrame->xpLeft;
				dyp = pbrdr->dypLineHeight;

				if (vlm != lmPreview)
				PatBlt(hdc, xp, yp, dxp, dyp, dwRop);
				else
			DrawPrvwLine(hdc, xp, yp, dxp, dyp, colFetch);

					/* right */
			xp = prcFrame->xpRight;
					yp = prcFrame->ypTop + (pbrdr->dypLineHeight << 1);
					dxp = pbrdr->dxpLineWidth;
					dyp = prcFrame->ypBottom - prcFrame->ypTop -
					(pbrdr->dypLineHeight  << 1);

					if (vlm != lmPreview)
					PatBlt(hdc, xp, yp, dxp, dyp, dwRop);
					else
				DrawPrvwLine(hdc, xp, yp, dxp, dyp, colFetch);

					/* readjust rectangle back */

				prcFrame->xpRight += pbrdr->dxpLineWidth << 1;
						prcFrame->ypBottom += pbrdr->dypLineHeight << 1;

		}

	if (vlm != lmPreview)
		{
		if (hbrOld != NULL)
			SelectObject(hdc, hbrOld);
		if (hbrNew != NULL)
			{
			UnlogGdiHandle(hbrNew, 1031);
			DeleteObject(hbrNew);
			}
		}
}


/***********************/
/* D r a w  C h  P i c */
/*  %%Function:  DrawChPic  %%Owner:  bobz       */

DrawChPic(ww, ich, pptPen, ypBase, pchpIn)
int ww;
int ich;
struct PT * pptPen;
int ypBase;
struct CHP *pchpIn;
{
    /* draw the requested picture
	    note- vfmtss.cpRef is the cp of the chPicture
	*/
	extern struct PRI           vpri;
	HDC hdc = PwwdWw(ww)->hdc;
	long  cfcPic;
	struct PICRC picrc;
	struct BRDR brdr;
	int xp = pptPen->xp;
	int yp = pptPen->yp;

	BOOL fBitmap;
	int iLevel = 0;
	int fDrawn = fFalse;
	int fPicRcAvail = fFalse;
	struct WWD *pwwd;
	int dl;
	int fCrop;
	int ico;
	int eid = eidNoMemPict;
	BOOL fDrawPicSave = vfDrawPics;
 	int matSave = vmerr.mat;

#ifdef DEBUG
	struct RC rcT;
	int iDrawFailed = 0;
	int iT;
#endif /* DEBUG */

	/* we will use vchpFetch instead of pchpIn, which is a heap
	    chp, which we would have to save a local copy of to use;
    	we call fetchcp anyway.
	*/

	Assert (vfli.fPicture);

		/* if printing, hdc should be the printer dc, otherwise
	    	it should not be. We might have other DC's in the display
	    	case besides that in vfti, so all we can assert is that
		    it is not the printer DC.
		*/
#ifdef DEBUG
	if (hdc == NULL ||
	  	(vfli.fPrint && hdc != vpri.hdc) ||
	  	(!vfli.fPrint && hdc == vpri.hdc))
	  	{

#ifdef XBZ
		CommSzNum(SzShared("Drawchpic vfli.fPrint: "), vfli.fPrint);
	  	CommSzNum(SzShared("Drawchpic hdc: "), hdc);
	  	CommSzNum(SzShared("Drawchpic vpri.hdc: "), vpri.hdc);
	  	CommSzNum(SzShared("Drawchpic vlm: "), vlm);
	  	CommSzNum(SzShared("Drawchpic vflm: "), vflm);
#endif /* BZ */

	  	Assert (fFalse);
		}
#endif /* DEBUG */

	/* if we have a printer with text and graphics bands,
	don't bother to display this stuff if we are in a text
	band
			*/

    Profile( vpfi == pfiPictDraw ? StartProf( 30) : 0);

    if (vfli.fPrint)
		{
#ifdef PRIBAND
		CommSzNum(SzShared("DrawChPic vpri.fGraphics: "), vpri.fGraphics);
#endif
			/* should not get here if in a text band unless we are printing
			    a metafile, which could contain imbedded textout calls
		   	*/
		Assert (vpri.fGraphics || vfli.fMetafile);
		}

			/* determine picture position and width */
	/* Get  these rectangles: outside and inside frame, scaled picture */
	/* note FGetPicRc will call FetchPe which will set pchp->fnPic
				to fnFetch. Set up fnFetch here with FetchCp... */

	FetchCpAndPara(vfli.doc, vfmtss.cpRef, fcmProps);
	Assert(vchpFetch.fSpec);

	ico = vchpFetch.ico;

#ifdef XBZTEST
	CommSzNum(SzShared("DrawChPic fnFetch: "), fnFetch);
#endif
	if (!(fPicRcAvail = FGetPicRc (&picrc, &fCrop, &vchpFetch, xp, yp, NULL /* pSel */,
			picgFrOut | picgFrIn | picgPicScl /* grpfPic */)))
		{
		goto DontDraw;
		}

#ifdef XBZTEST
	CommSzNum(SzShared("DrawChPic pen xp: "), xp);
	CommSzNum(SzShared("DrawChPic pen yp: "), yp);
#endif
			/* rectangles in rcpic struct:
			struct RC frOut - size of cropped pic frame; includes border
			struct RC frIn - inside/clipping rect offset to remove border
			struct RC picScl - scaled picture offset by border and cropping
			*/


    /* GetPictDxpDyp has called FetchPe and filled the PIC struct
	    vpicFetch
	*/

	cfcPic = (vpicFetch.lcb - vpicFetch.cbHeader);  /* size of pic bytes */

#ifdef XBZ
	CommSzNum(SzShared("DrawChPic hdc: "), hdc);
	CommSzNum(SzShared("DrawChPic doc: "), vfli.doc);

	CommSzLong(SzShared("DrawChPic cp: "), vfmtss.cpRef);

	CommSzLong(SzShared("DrawChPic sizeof picture: "), cfcPic);

	GetClipBox (hdc, (LPRECT)&rcT);
	CommSzRgNum(SzShared("DrawCpPic hdc clip rect: "),
		&rcT, 4);

	{
	int dxpInch, dypInch;

	dxpInch = GetDeviceCaps(hdc, LOGPIXELSX);
	dypInch = GetDeviceCaps(hdc, LOGPIXELSY);
	Assert (dxpInch == vfti.dxpInch && dypInch == vfti.dypInch);
	CommSzNum(SzShared("DrawChPic hdc dxpInch: "), dxpInch);
	CommSzNum(SzShared("DrawChPic hdc dypInch: "), dypInch);
	}
#endif
			/* get border sizes */
	GetBrdrLineSizes (vpicFetch.brcl, &brdr, NULL, fTrue /* fScaleForScreen */);

	/* fNoShowPictures is set up so print will set it to 0 and thus print all
	pictures, but in display we have the choice to see the pic
	or just the frame.
			*/

	pwwd = PwwdWw(ww);
	if ( pwwd->grpfvisi.fNoShowPictures ||
		!vfli.fPrint && vpref.fDraftView )  /* just draw frame */
		{
#ifdef XBZ
		CommSzNum(SzShared("DrawChPic fNoShow: "), pwwd->grpfvisi.fNoShowPictures);
		CommSzNum(SzShared("DrawChPic fDraftView: "), vpref.fDraftView);
#endif /* BZ */

		fDrawn = fTrue;
		goto DrawDefault;
		}
	else  if (!vfli.fPrint && !vfDrawPics &&
			/* draw pics if deactivated as in dde */
            /* may need to draw for clipboard if iconic */
	        !(vfDeactByOtherApp && !vidf.fDead))
        {
		    /* we will put off drawing the pic.
				We need to set flag in edl so the pic will be
			    drawn for real in idle. DisplayFli has set the flag to
			    false, so if we do draw the pic we don't need to reset
			    it.
			*/

			/* get edl and set flag */
			/* vdrdl... set in displayfli */

			SetfEnhanceEdlWw(ww);

#ifdef BZTEST
				CommSzNum(SzShared("DrawChPic enhance - vfDrawPics: "), vfDrawPics);
#endif /* BZ */

		goto DrawReturn;
		}

			/* draw in border, if any */

	if (vpicFetch.brcl != brclNone)
			DrawPicFrame(hdc, &picrc.frOut, &brdr, vpicFetch.brcl, ico);

	if (cfcPic == 0L)
			/* no pain, no gain. Border already drawn */
		{
#ifdef BZ
		CommSzSz(SzShared("DrawChPic cfPic is 0 "), "");
#endif /* BZ */

		fDrawn = fTrue;
		goto DontDraw;
		}

	vfDrawPics = fTrue;
            /* indicating we are actually drawing the picture
		        we saved away the original value in fDrawPicSave.
		        We postpone some system messages when doing pics;
		        print already does that so don't step on it.
			*/
	if (vlm != lmPrint && vlm != lmFRepag)
	   	vpri.wmm = 0;


		 /* clip hdc to inside frame rectangle if
		    necessary. Note brcl doesn't need to be taken into account since
		    border is drawn outside the normal rect; if there is no cropping
		    the inside frame rect will be the same as the pic rect.
		 */
	if (fCrop)
        {
				/* save DC since clipping */
        if ((iLevel = SaveDC (hdc)) == 0)
			{
			Debug0 (iDrawFailed = 1);
			goto DontDraw;
			}
		IntersectClipRect(hdc, picrc.frIn.xpLeft, picrc.frIn.ypTop,
				picrc.frIn.xpRight, picrc.frIn.ypBottom);
		}

#ifdef BZ
	GetClipBox (hdc, (LPRECT)&rcT);
	CommSzRgNum(SzShared("DrawCpPic hdc clip rect after (possible) clip: "),
		&rcT, 4);
	CommSzRgNum(SzShared("DrawCpPic pic frin rc: "),
		&picrc.frIn, 4);
#endif

   	ShrinkSwapArea();   /* allow more memory for pics */

	SetStretchBltMode( hdc, BLACKONWHITE );

		/* process metafiles, bitmaps/tiff files separately */

	switch ( vpicFetch.mfp.mm )
		{
		default:
			if (vpicFetch.mfp.mm < MM_META_MAX)
			    fDrawn = FDrawMetafile(hdc, &picrc, ww, &eid);
			else
				eid = eidBadPicFormat;
			break;
		case MM_BITMAP:
			  // replaces FDrawBitmap 
            fDrawn = FDrawPicCacheable(hdc, &picrc, NULL  /* pilevel */, ww,
				          &eid, ftClipboard, MM_BITMAP);
			break;
		case MM_TIFF:
			{
			  // replaces FDrawBitmap 
            fDrawn = FDrawPicCacheable(hdc, &picrc, NULL  /* pilevel */, ww,
				          &eid, ftNil, MM_TIFF);
			break;
			}
		}

	GrowSwapArea();   /* restore */


DontDraw:
			/* Clean up */

	if (iLevel > 0)
			RestoreDC (hdc, iLevel);

			/*  !fDrawn - didn't display picture for some reason */
	if (!fDrawn)
			/* only display one error during process since print will call
				this multiple times even for only 1 picture. */
		{
		/* warning this HAS to be a systemmodal
			message or it will deactivate us while in Displayfli
			with disastrous results (like selCur.doc set to docNil)
		*/

		PictError (eid);

				/* Draw border to tell us when the draw tried but failed
		        	or for no view pic. This will redraw boder if there
					was one, but will give us one if none.
                */

DrawDefault:
				/*  draw single pic border only */
				/*  failed draw gets double border */
		if (fPicRcAvail)
			DrawPicFrame(hdc, &picrc.frOut, &brdr,
#ifdef DEBUG
				fDrawn? brclSingle : brclDouble, ico);
#else
				brclSingle, ico);
#endif

		}

DrawReturn:

	/* may have been changed temporarily due to printing or deactivating */
	vfDrawPics = fDrawPicSave;

 	  /* we don't want fMemFail	set by a picture failure to mess up
 		 later print, so save and restore vmerr.mat. If the printing caused
 		 the mat to be set, flush it out here, then restore the old value so anything
 		 set earlier will still be set
 	  */
 	if (matSave == matNil && vmerr.fMemFail)
 		FlushPendingAlerts();
 	vmerr.mat = matSave;

	Profile( vpfi == pfiPictDraw ? StopProf() : 0);

}

struct PLBMC
	{
	int     iMac;
	int     iMax;
	int     cb;
	int     brgfoo;
	int		fExternal;
	long	lcbBitmapUsed;
	struct BMC	rgBmc[1];
	};
#define cbPLBMCBase (offset(PLBMC, rgBmc))

struct PLBMC				**vhplbmc = hNil;


/*  %%Function:  FDrawPicCacheable  %%Owner:  bobz       */

FDrawPicCacheable(hdc, ppicrc, piLevel, ww, peid, wUser, mm)
HDC hdc;
struct PICRC *ppicrc;
int *piLevel;
int ww;
int *peid;
WORD wUser;	 // ft for bitmaps, phBits for metafiles
int mm;
{
	HDC hMDC;
	int wDrawn = fFalse;
    int dxpPicScl, dypPicScl;
    struct RC rc;
	struct BMC bmc;
	BOOL fBitmap = (mm == MM_BITMAP || mm == MM_TIFF);

	extern int vsasCur;

      /* verify we have shrunk swap area */
	bmc.hbm = NULL;
	Assert (vsasCur == sasMin);

    rc = ppicrc->picScl;
	dxpPicScl =  rc.xpRight - rc.xpLeft;
	dypPicScl =  rc.ypBottom - rc.ypTop;

    if ((hMDC = CreateCompatibleDC(hdc)) == NULL)
		  return fFalse;

#ifdef XBZ
    {
	struct RC rcT;
	GetClipBox (hMDC, (LPRECT)&rcT);
	CommSzRgNum(SzShared("FDrawPicCacheable hMDC clip rect : "),
		&rcT, 4);
	CommSzRgNum(SzShared("FDrawPicCacheable pic rcScl: "),
		&rc, 4);
	}
#endif

	 /* try to get hbm from cache into hMDC. If that fails, try to load bm
	  into cache. If THAT fails, try to display without going thru cache.
	 */
	if (!FCachedHbmToHdc(hMDC))
		{
		// NOTE: must pass in device hdc so metafiles can have color
		// bitmaps. Created bitmap is selected into memory dc.
		if (!FBmcOKForCache(hdc, &bmc, dxpPicScl, dypPicScl, fBitmap)
			|| SelectObject(hMDC, bmc.hbm) == NULL)
			{
			goto LNoCache;
			}
		Assert(bmc.hbm != NULL);
		StartLongOp();
		wDrawn =
			fBitmap ?
				WLoadBM(hMDC, ppicrc, ww, peid, wUser, fTrue)	/* try to load bitmap into memory dc for cache */
				: WLoadMF(hMDC, ppicrc, ww, peid, wUser, mm, fTrue);	/* try to play metafile into memory dc for cache */
		EndLongOp(fFalse /* fAll */);
		Assert(wDrawn == fFalse || wDrawn == fTrue || wDrawn == GRINTERRUPT);
		if (wDrawn == GRINTERRUPT)
			goto LRet;
		if (!wDrawn) /* failed for real. Free some space and try again. */ 
			{
			/* Don't retry if the tiff file was invalid */
			if (*peid != eidPicNoOpenDataFile)
				{
LNoCache:
				ClearHmdc(hMDC, &bmc);
				hMDC = NULL;
				/* try to display without caching  */
				StartLongOp();
				wDrawn =
					fBitmap ?
						WLoadBM(hdc, ppicrc, ww, peid, wUser, fFalse)
						: WLoadMF(hdc, ppicrc, ww, peid, wUser, mm, fFalse); 
				EndLongOp(fFalse /* fAll */);
				}
			goto LRet;				
			}
		Assert(bmc.hbm != NULL);
		InsertPbmc(&bmc);
		bmc.hbm = NULL; /* so we don't try to delete hbm during cleanup */
		}

	/*
		Metafile note:
		Metafile code sets up the device hdc mapping mode, orgs, extents, etc.
		if we fail to cache, then calling WLoadMF with the hdc will do
		the right thing. BUT, if we did play into the hMDC, then we want to
		BitBlt into a MM_TEXT hdc. WLOadMF resets the hMDC after playing
		the metafile. We restore the saved DC based on *piLevel for the metafile
		case here. Note that we restore to the DC we started with in
		FDrawMetafile, not the one possibly saved in DrawChPic if we cropped
	*/

	if (!fBitmap)
		{
		RestoreDC (hdc, *piLevel);
		*piLevel = 0;
		Assert (GetMapMode(hMDC) == MM_TEXT);
		}

    wDrawn = BitBlt( hdc, rc.xpLeft, rc.ypTop,
				 dxpPicScl, dypPicScl,
                 hMDC, 0, 0,
                 vsci.fMonochrome && fBitmap ?
                 ropMonoBm : SRCCOPY );

LRet:
	ClearHmdc(hMDC, &bmc);

	 /* note that GRINTERRUPT will be interpreted as true, which is what we want */
	return wDrawn;
}


/*  %%Function:  ClearHmdc  %%Owner:  bobz       */

ClearHmdc(hMDC, pbmc)
HDC hMDC;
struct BMC *pbmc;
{
	if (hMDC != NULL)
		{
		FSetDcAttribs(hMDC, dccmUnlockBrushPen );
		SelectObject(hMDC, vhbmPatVert); /* abritrary bm; so we can dispose selected bm */
		if (pbmc->hbm != NULL)
			{
			DeleteObject(pbmc->hbm);
			/* pbmc->hbm = NULL; IE - Nobody will use pbmc->hbm because
			   hMDC will be cleared. */
			}
		/* UnlogGdiHandle(hMDC, 1045); */
		AssertDo(DeleteDC(hMDC));
		}
}


/***********************/
/* F  B m c  O  K  F o r  C a c h e */
/*  %%Function:  FBmcOKForCache	%%Owner:  bradv       */

BOOL FBmcOKForCache(hdc, pbmc, dxp, dyp, fMono)
HDC hdc;
struct BMC *pbmc;
int dxp;
int dyp;
BOOL fMono;
{
	HBITMAP hbm;
	int ibmc;
	long lcbAlloc, lcbUserLimit;

	lcbAlloc = ((dxp + 15) / 16) * sizeof(int);
	lcbAlloc *= dyp;
	if (!fMono)
		{
		lcbAlloc *= GetDeviceCaps(hdc, BITSPIXEL);
		lcbAlloc *= GetDeviceCaps(hdc, PLANES);
		}

	if ((lcbUserLimit = (long)cKBitmap * 1024) < lcbAlloc)
		return fFalse;

	if (vhplbmc == hNil)
		goto LHplbmcNil;

#ifdef DEBUG
	{
	int ibmcT;
	struct BMC *pbmcT;

	for (ibmcT = 0, pbmcT = (struct BMC *)PInPl(vhplbmc, ibmcT);
		ibmcT < (*vhplbmc)->iMac; ibmcT++, pbmcT++)
		{
		Assert(pbmcT->hbm != hbm);
		Assert(pbmcT->ca.cpFirst == pbmcT->ca.cpLim);
		Assert(pbmcT->ca.doc != vdocFetch
			|| pbmcT->ca.cpFirst != vcpFetch
			|| pbmcT->flm != vflm);
		}
	}
#endif /* DEBUG */

	/* Delete LRU entries to create enough room */
	for (ibmc = (*vhplbmc)->iMac - 1;
		ibmc >= 0 && (*vhplbmc)->lcbBitmapUsed + lcbAlloc > lcbUserLimit;
		ibmc--)
		{
		DeleteBmcEntry(ibmc);
		}

LHplbmcNil:

	if ((hbm = (fMono ?
		CreateBitmap( dxp, dyp, 1, 1, (LPSTR) NULL ) : 
		CreateCompatibleBitmap(hdc, dxp, dyp)
		)) == NULL)
		{
		return fFalse;
		}

	/* Add new entry (at ibmc == 0 to indicate MRU) */
	pbmc->hbm = hbm;
	PcaPoint(&pbmc->ca, vdocFetch, vcpFetch);
	pbmc->flm = vflm;
	pbmc->lcb = lcbAlloc;
	return fTrue;
}

/***********************/
/* I n s e r t  P b m c */
/*  %%Function:  InsertPbmc	%%Owner:  bradv       */

InsertPbmc(pbmc)
struct BMC *pbmc;
{

	if (vhplbmc == hNil)
		{
		if ((vhplbmc = HplInit2(sizeof(struct BMC), cbPLBMCBase,
			1 /* ifooMaxInit */, fFalse /* fExternal */)) == hNil)
			{
			return fFalse;
			}
		/* (*vhplbmc)->lcbBitmapUsed = 0; IE */
		Assert((*vhplbmc)->lcbBitmapUsed == 0L);
		}

	Assert(pbmc->hbm != NULL);
	if (!FInsertInPl(vhplbmc, 0, pbmc))
		{
		/* Can't leave here with iMac == 0 because other routines
		   will choke.	We know that if we failed to insert we
		   will still have one entry because we allocated enough
		   room for one entry when we created vhplbmc, and no routine
		   (such as DeleteFromPl) ever decreases that. */
		Assert((*vhplbmc)->iMac > 0);
		DeleteObject(pbmc->hbm);
		}
	(*vhplbmc)->lcbBitmapUsed += pbmc->lcb;
}


/***********************/
/* D e l e t e  B m c  E n t r y */
/*  %%Function:  DeleteBmcEntry	%%Owner:  bradv       */

DeleteBmcEntry(ibmc)
int ibmc;
{
	HBITMAP hbm;
	long lcbAlloc;
	struct BMC *pbmc;

	pbmc = ((struct BMC *)PInPl(vhplbmc, ibmc));
	hbm = pbmc->hbm;
	(*vhplbmc)->lcbBitmapUsed -= pbmc->lcb;
	Assert((*vhplbmc)->lcbBitmapUsed >= 0L);
	DeleteObject(hbm);
	DeleteFromPl(vhplbmc, ibmc);
	if ((*vhplbmc)->iMac == 0)
		{
		Assert((*vhplbmc)->lcbBitmapUsed == 0L);
		FreePhpl(&vhplbmc);
		}
}


/***********************/
/* F  C a c h e d  H b m  T o  H d c */
/*  %%Function:  FCachedHbmToHdc  %%Owner:  bradv       */

FCachedHbmToHdc(hdc)
HDC hdc;
{
	int ibmc;
	struct BMC bmc, *pbmc, *pbmcMac;

	if (vhplbmc == hNil)
		return fFalse;
	Assert((*vhplbmc)->iMac > 0);

	FreezeHp();
	for (ibmc = 0, pbmc = PInPl(vhplbmc, 0), pbmcMac = pbmc+(*vhplbmc)->iMac;
		pbmc < pbmcMac; ibmc++, pbmc++)
		{
		if (pbmc->ca.doc == vdocFetch
			&& pbmc->ca.cpFirst == vcpFetch
			&& pbmc->flm == vflm)
			{
			break;
			}
		}
	MeltHp();

	if (pbmc == pbmcMac)
		return fFalse;

	/* move the entry to ibmc == 0 to indicate MRU */
	bmc = *((struct BMC *)PInPl(vhplbmc, ibmc));
	if (SelectObject(hdc, bmc.hbm) == NULL)
		{
		ReportSz("Selecting a bitmap cache object failed!");
		DeleteBmcEntry(ibmc);
		return fFalse;
		}
	DeleteFromPl(vhplbmc, ibmc);
	/* Can't leave here with iMac == 0 because other routines
	   will choke.	We know that if we failed to insert we
	   will still have one entry because we allocated enough
	   room for one entry when we created vhplbmc, and no routine
	   (such as DeleteFromPl) ever decreases that. */
	AssertDo(FInsertInPl(vhplbmc, 0, &bmc));
	return fTrue;
}


/***********************/
/* I n v a l  B m c */
/*  %%Function:  InvalBmc  %%Owner:  bradv       */

InvalBmc(doc, cpFirst, dcp)
int doc;
CP cpFirst;
CP dcp;
{

	int ibmc;
	struct BMC *pbmc;
	struct CA caT;

	Assert(vhplbmc != hNil);
	Assert((*vhplbmc)->iMac > 0);

	FreezeHp();
	for (ibmc = (*vhplbmc)->iMac - 1, pbmc = PInPl(vhplbmc,ibmc);
		ibmc >= 0; ibmc--, pbmc--)
		{
		Assert(pbmc->ca.cpFirst == pbmc->ca.cpLim);
		/* if doc is set to docNil we want to invalidate the entire
		   bitmap cache (e.g. when called from FreeHdcPrinter). */
		if (doc == docNil
			|| (pbmc->ca.doc == doc
			&& pbmc->ca.cpFirst >= cpFirst
			&& pbmc->ca.cpFirst < cpFirst+dcp))
			{
			DeleteBmcEntry(ibmc);
			}
		}
	MeltHp();
}


/* File split - too big for SLM - rp */
#include "pic3.c"
