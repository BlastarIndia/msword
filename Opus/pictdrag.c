/* ****************************************************************************
**
**      COPYRIGHT (C) 1987, 1988 MICROSOFT
**
** ****************************************************************************
*
*  Module: pictdrag.c ---- Picture frame drag routines
*
**
** REVISIONS
**
** Date         Who    Rel Ver     Remarks
** 11/20/87     bobz               split off from pic.c 
**
** ************************************************************************* */

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NONCMESSAGES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOSYSMETRICS
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOFONT
#define NOMB
#define NOMENUS
#define NOMSG
#define NOOPENFILE
#define NOPEN
#define NOPOINT
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
#include "file.h"
#include "props.h"
#include "format.h"
#include "sel.h"
#include "disp.h"
#include "doc.h"
#include "screen.h"
#include "ch.h"
#define PICTDRAG
#include "pic.h"
#include "prm.h"
#include "print.h"
#include "dlbenum.h"
#include "prompt.h"
#include "message.h"
#include "help.h"
#include "error.h"

#include "debug.h"

#ifdef PCODE
typedef long ul;   /* no unsigned longs in CS compiler */
#else
typedef unsigned long ul;
#endif


struct PT PtPropRect();

/* E X T E R N A L S */

extern CHAR             vstPrompt[];
extern struct WWD       **hwwdCur;
extern struct FLI       vfli;
extern struct PIC       vpicFetch;
extern struct CHP       vchpFetch;
extern struct PAP       vpapFetch;
extern struct FMTSS     vfmtss;
extern struct FTI       vfti;
extern struct FTI       vftiDxt;
extern struct MERR      vmerr;
extern struct SCI       vsci;
extern struct PREF      vpref;
extern HBRUSH           vhbrGray;
extern HWND             vhwndStatLine;
extern HWND             vhwndCBT;
#ifdef WIN23
extern HWND 			vhwndPrompt;
#endif /* WIN23 */




/********************************/
/* D R A G  P I C  F R A M E */
/*  %%Function:  DragPicFrame  %%Owner:  bobz       */

DragPicFrame(ircs, ptIn, prcpic, fScale)
int ircs;
struct PT ptIn;
struct PICRC *prcpic;
int fScale;
{
	struct RC rcDrag;
	struct RC rcDragLim;
	struct RC rcBasisPic;
	struct RC rcScale;
	struct DRC drcCorner;
	HBRUSH       hbrOld;
	HDC hdc;
	struct PT pt;
	struct PT ptPrev;
	int dxp, dyp;
	int fCorner;
	CHAR FAR *pmst;

	/* in this routine I compare 2 points by doing a single long compare
		rather than calling FEqualPt, which does more than that. To work,
		I have to know I can treat a point as a long. bz
	*/

	Assert (sizeof(struct PT) == sizeof (long));

	(long)ptPrev = -1L;  /* force initial difference */

	rcDrag = prcpic->frOut;

	/* get distance of first point from drag rect, so we avoid
		immediate jump, and so we actually compare the rectangle
		t the limit rect, not the point we clicked on.
	*/

	switch (ircs)
		{
		/* yp values - only deal with top or bottom line */

	default:
		dyp = 0;
		break;
	case ircsBC:
	case ircsBR:
	case ircsBL:
		dyp = rcDrag.ypBottom - ptIn.yp;
		break;
	case ircsTL:
	case ircsTC:
	case ircsTR:
		dyp = rcDrag.ypTop - ptIn.yp;   /* negative */
		break;
		}

	switch (ircs)
		{
		/* xp values - only deal with left or right line */

	default:
		dxp = 0;
		break;
	case ircsTL:
	case ircsCL:
	case ircsBL:
		dxp = rcDrag.xpLeft - ptIn.xp;  /* will be negative */
		break;
	case ircsBR:
	case ircsCR:
	case ircsTR:
		dxp = rcDrag.xpRight - ptIn.xp;  /* will be positive */
		break;
		}

	/* Corner dragging notes.
	For corners, we force the point to be on the diagonal of
	the original outside frame rectangle of the picture.
	This preserves the proportions of the original picture.
	
	I determine the change to the drag rect by keeping the
	previous point and addig delta pt to the drag rect. For
	corners, this only maintains proportions if the original point
	is on the diagonal, so if a corner, I compute pt using
	PtPropRect, and treat the resulting point as the origin for
	mouse movement differences. Since the limiting rect hasn't
	been computed yet, I give PtPropRect the whole screen rect,
	since I know the point is in a drag rect, and so within the
	limiting rect. I compute some info for the diagoanal dragging
	outside the same loop, since the slope of the original rect will
	not change, nor will the corner to be dragged.  bz
	*/

	switch (ircs)   /* set up for corner drag */
		/* note that rcDrag used because == frOut which we really want */
		/* basically setting up diagonal and dxp dyp for slopes for
			constraining to diagonal. See notes in PtPropRect explaining
			these calcs bz */
		{
	case ircsTL:
		/* corner point is point on diagonal */
		drcCorner.xp = rcDrag.xpLeft;
		drcCorner.yp = rcDrag.ypTop;
		pmst = fScale ? mstpScaleHW : mstpCropTL;
		goto TLBR;
	case ircsBR:
		drcCorner.xp = rcDrag.xpRight;
		drcCorner.yp = rcDrag.ypBottom;
		pmst = fScale ? mstpScaleHW : mstpCropBR;
TLBR:
		drcCorner.dxp = rcDrag.xpRight - rcDrag.xpLeft;
		goto AllCorners;

	case ircsTR:
		drcCorner.xp = rcDrag.xpRight;
		drcCorner.yp = rcDrag.ypTop;
		pmst = fScale ? mstpScaleHW : mstpCropTR;
		goto TRBL;
	case ircsBL:
		drcCorner.xp = rcDrag.xpLeft;
		drcCorner.yp = rcDrag.ypBottom;
		pmst = fScale ? mstpScaleHW : mstpCropBL;
TRBL:
		drcCorner.dxp = rcDrag.xpLeft - rcDrag.xpRight;
AllCorners:
		drcCorner.dyp = rcDrag.ypBottom - rcDrag.ypTop;
		fCorner = fTrue;
		/* local copy; do it here while pointer to csconst obj is valid */
		bltbx ((CHAR FAR *)pmst, (CHAR FAR *)vstPrompt, *pmst+1);
		break;

	case ircsBC:
		pmst = fScale ? mstpScaleH : mstpCropB;
		goto AllEdges;
	case ircsCR:
		pmst = fScale ? mstpScaleW : mstpCropR;
		goto AllEdges;
	case ircsTC:
		pmst = fScale ? mstpScaleH : mstpCropT;
		goto AllEdges;
	case ircsCL:
		pmst = fScale ? mstpScaleW : mstpCropL;

AllEdges:
		fCorner = fFalse;
		/* local copy; do it here while pointer to csconst obj is valid */
		bltbx ((CHAR FAR *)pmst, (CHAR FAR *)vstPrompt, *pmst+1);
		break;

#ifdef DEBUG
	default:  /* error */
		Assert (fFalse);
#endif /* DEBUG */

		}

	pt = ptIn;

	/* Basis rect is used in ShowPicSize as the rect to which the changed
		rect is compared */

	GetBasisPicRc (&rcBasisPic, prcpic, fScale);
	/* basis should only be empty if cropping is too big. which of
		course should never happen
	*/
	if (FEmptyRc(&rcBasisPic))
		goto LEmpty;

	GetRcDragLim(&rcDragLim, &rcDrag, &rcBasisPic, &rcScale, ircs, fScale);
	/* should not happen, but if it does bag out bz */
	if (FEmptyRc(&rcDragLim))  /* so we don't have to test in loop */
		{
LEmpty:
#ifdef BZ
		CommSzRgNum(SzShared("DragPicFrame empty drag lim rect: "),
				&rcDragLim, 4);
		CommSzRgNum(SzShared("or DragPicFrame empty basis rect: "),
				&rcBasisPic, 4);
#endif
		Beep();
		return;
		}

#ifdef BZ
		{
		CommSzRgNum(SzShared("DragPicFrame init drag rc: "),
				&rcDrag, 4);
		CommSzRgNum(SzShared("DragPicFrame Basis rc: "),
				&rcBasisPic, 4);
		CommSzRgNum(SzShared("DragPicFrame Lim rc: "),
				&rcDragLim, 4);
		}
#endif
	/* put up prompt */

	DisplayPrompt (pdcPic, hNil);

	ShowPicSize (&rcDrag, &rcBasisPic, &rcScale, ircs, fScale, fFalse);
	hdc = (*hwwdCur)->hdc;

	hbrOld = SelectObject(hdc, vhbrGray);
	FrameRectPic(hdc, &rcDrag);

	SetCapture( (*hwwdCur)->hwnd );

#ifdef XBZTEST
		{
		if (fCorner)
			CommSzRgNum(SzShared("DragPicFrame corner rect: "),
					&drcCorner, 4);
		}
#endif

	Profile( vpfi == pfiPictDrag ? StartProf( 30) : 0);

	while (FStillDownReplay( &pt, fFalse ))
		{
		/* no mouse move leaves unchanged. Assert above ok to treat pt as long */
		if ((long)ptPrev == (long)pt)
			continue;

		/* adjust pt by original distance from rect, so we are dealing with
			a point on the rect for comparison with lim rect, and so we
			don't jump to the clicked point immediately. Signs have already
			been adjusted so we can just add, and ircs's that don't care
			use 0 for dzp.
		*/
		pt.xp += dxp;
		pt.yp += dyp;

		/* limit point to drag rect */
		/* copy of code in PtIntoRect in preview.c - here for swap reasons */
		/* we assert above that rcDragLim is not empty */
		/* Emulate Mac ROM call that does nothing if *ppt is in prc, but
			moves *ppt to closest border of prc if *ppt is outide prc.
		*/
		if (pt.xp < rcDragLim.xpLeft)
			pt.xp = rcDragLim.xpLeft;
		else  if (pt.xp > rcDragLim.xpRight)
			pt.xp = rcDragLim.xpRight;
		if (pt.yp < rcDragLim.ypTop)
			pt.yp = rcDragLim.ypTop;
		else  if (pt.yp > rcDragLim.ypBottom)
			pt.yp = rcDragLim.ypBottom;

		/* if corner, constrain point to diagonal */

		if (fCorner && (long)ptPrev != (long)pt)
			pt = PtPropRect(pt, &drcCorner, &rcDragLim);

		if ((long)ptPrev != (long)pt)
			{
			/* adjust rcDrag based on new point.
			Use knowledge of drag box to determine
			what to adjust.
			*/
			/* erase previous frame */
			FrameRectPic(hdc, &rcDrag);

			switch (ircs)
				{
				/* yp values - only deal with top or bottom line */

			case ircsBC:
			case ircsBR:
			case ircsBL:
				rcDrag.ypBottom = pt.yp;
				break;
			case ircsTL:
			case ircsTC:
			case ircsTR:
				rcDrag.ypTop = pt.yp;
				break;
				}

			switch (ircs)
				{
				/* xp values - only deal with left or right line */

			case ircsTL:
			case ircsCL:
			case ircsBL:
				rcDrag.xpLeft = pt.xp;
				break;
			case ircsBR:
			case ircsCR:
			case ircsTR:
				rcDrag.xpRight = pt.xp;
				break;
				}


			ShowPicSize (&rcDrag, &rcBasisPic, &rcScale, ircs, fScale, fFalse);

			FrameRectPic(hdc, &rcDrag);
			ptPrev = pt;
			}      /* if ((long)ptPrev != (long)pt) */
		}          /* while (FStillDownReplay( &pt,fFalse )) */

	Profile( vpfi == pfiPictDrag ? StopProf() : 0);

	/* get changed vals and change pic frame */

	if (vhwndCBT)
		{
		/* Advisory message for CBT */
		SendMessage(vhwndCBT, WM_CBTSEMEV, smvPicFormat,
				MAKELONG(fScale, ircs));
		ShowPicSize (&rcDrag, &rcBasisPic, &rcScale, ircs, fScale,
				fTrue /* fTellCBT */);
		}

	ChangePicVals (&rcDrag, &rcBasisPic, &rcScale, ircs, fScale);

	/* erase final gray frame */
	FrameRectPic(hdc, &rcDrag);

	ReleaseCapture();
	if (hbrOld != NULL)
		SelectObject(hdc, hbrOld);

	RestorePrompt();   /* take down crop/scale prompt */
}


/* macros used in ShowPicSize and ChangePicVals */

#define XpScale() (umax(umin (UMultDiv((prcDrag->xpRight - prcDrag->xpLeft), 1000, \
					(prcBasisPic->xpRight - prcBasisPic->xpLeft)), prcScale->xpLeft), \
						prcScale->xpRight))
#define YpScale() (umax(umin (UMultDiv((prcDrag->ypBottom - prcDrag->ypTop), 1000, \
					(prcBasisPic->ypBottom - prcBasisPic->ypTop)), prcScale->ypTop), \
						prcScale->ypBottom))

/*  %%Function:  XaCrop  %%Owner:  bobz       */

XaCrop(xp, prcBasisPic, iCropOpposite)
int xp;
struct RC *prcBasisPic;
int iCropOpposite;
{
	/* fairly complex, but this is what we do:
		1. make xp into an xa that is the same proportion to the goal
		size as xp was to the basis rect.
		2. if xa > 0, makes size smaller, so be sure goal - xa -
		   POSITIVE cropping on opposite side is at least dxaPicMinDrag.
			   E.g., goal is 1", top cropping is .85, limit xa to .03.
			   If top gropping was negative, xa would be limited to
			   .88
		2. if xa < 0, makes size larger, so be sure goal - xa is
		at most dxaPicMax
	*/

	xp = NMultDiv(xp, vpicFetch.dxaGoal,
			(prcBasisPic->xpRight - prcBasisPic->xpLeft));
	if (xp > 0)
		return (min (xp, vpicFetch.dxaGoal - dxaPicMinDrag
			- max (0, vpicFetch.rgCrop[iCropOpposite])));
	else
		return (max (xp, vpicFetch.dxaGoal - dxaPicMax));


}


/*  %%Function:  YaCrop  %%Owner:  bobz       */

YaCrop(yp, prcBasisPic, iCropOpposite)
int yp;
struct RC *prcBasisPic;
int iCropOpposite;
{
	yp = NMultDiv(yp, vpicFetch.dyaGoal,
			(prcBasisPic->ypBottom - prcBasisPic->ypTop));
	if (yp > 0)
		return (min (yp, vpicFetch.dyaGoal - dyaPicMinDrag
			- max (0, vpicFetch.rgCrop[iCropOpposite])));
	else
		return (max (yp, vpicFetch.dyaGoal - dyaPicMax));
}


/********************************/
/*  %%Function:  ShowPicSize   %%Owner:  bobz       */

ShowPicSize (prcDrag, prcBasisPic, prcScale, ircs, fScale, fTellCBT)
struct RC *prcDrag;
struct RC *prcBasisPic;
struct RC *prcScale;
int ircs;
BOOL fScale, fTellCBT;
{
	int xp, yp;
	int rgw[2];
	int irgw;
	int i;
	BOOL fNeg;
	char rgch[cchmstCrop + 1];
	char rgchCBT[cchmstCrop * 2 + 2];
	int dzaCropped;
	unsigned mz;
	unsigned mRemainder;
	int cchCBT = 0;
	int iCropOpposite;

	Assert (cchmstCrop > cchmstScale);

	if (fScale)
		{
		/* scales are max of (min(mzmax, mz), mzmin) */
		if (ircs != ircsTC && ircs != ircsBC)
			{
			/* macros like XpScale() defined above */
			xp = UDiv(XpScale(), PctTomx100Pct, &mRemainder); /* divide by 10 for display */
			}
		if (ircs != ircsCR && ircs != ircsCL)
			{
			yp = UDiv(YpScale(), PctTomy100Pct, &mRemainder); /* divide by 10 for display */
			}
		irgw = 1;
		switch (ircs)
			{
		case ircsCR:
		case ircsCL:
			rgw[0] = xp;
			break;
		case ircsTC:
		case ircsBC:
			rgw[0] = yp;
			break;
			/* both high and wide */
		default:
			rgw[0] = yp;
			rgw[1] = xp;
			irgw = 2;
			break;
			}

		}
	else  /* crop, not scale */		
		{
		irgw = 0;
		switch (ircs)
			{
			/* only deal with left or right line */

		case ircsTL:
		case ircsCL:
		case ircsBL:
			xp = prcDrag->xpLeft - prcBasisPic->xpLeft;
			iCropOpposite = iCropRight;
			goto LeftRight;

		case ircsBR:
		case ircsCR:
		case ircsTR:
			xp = prcBasisPic->xpRight - prcDrag->xpRight;
			iCropOpposite = iCropLeft;

LeftRight:
			/* Convert cropping distance to equivalent distance on
				goal picture size (in twips). This lets us drag using on screen
				rectangles that may be quite different than the goal rect
				(as a 1"x1" square goal bitmap that becomes 4"x4" on
				screen when the printer resolution changes from 300dpi to
				75 dpi. A 1" screen movement should be indicated as a 1/4"
				move ment on the goal picture, from which all change is
				measured. This takes care of the effect of scaling back
				cropping so that it is measured in terms of the unscaled
				picture as well.
			
				Check for resulting pic not < min or > max, due to
				rounding.
			*/

			rgw[irgw++] = XaCrop(xp, prcBasisPic, iCropOpposite);
			break;
			}

		switch (ircs)
			{
			/*  only deal with top or bottom line */

		case ircsBC:
		case ircsBR:
		case ircsBL:
			yp = prcBasisPic->ypBottom - prcDrag->ypBottom;
			iCropOpposite = iCropTop;
			goto TopBottom;

		case ircsTL:
		case ircsTC:
		case ircsTR:
			yp = prcDrag->ypTop - prcBasisPic->ypTop;
			iCropOpposite = iCropBottom;

TopBottom:
			/* See note above! convert to za wrt dyaGoal */
			rgw[irgw++] = YaCrop(yp, prcBasisPic, iCropOpposite);


			}

		}


	/* show current scaling or cropping */

	for (i = 0; i < irgw; i++)
		{
		int ich;
		int cch, cchT;
		CHAR *pchT;
		int di;

		pchT = rgch;

		if (fScale)
			{
			ich = (i == 0) ? ichmstScale1 : ichmstScale2;
			cch = cchmstScale;
			cchT = CchIntToPpch (rgw[i], &pchT);
			}
		else
			{
			ich = (i == 0) ? ichmstCrop1 : ichmstCrop2;
			cch = cchmstCrop;
			cchT = CchExpZa(&pchT, rgw[i], vpref.ut, sizeof(rgch)-1, fTrue);
			}

		/* right justify number in field */

		Assert (cchT < sizeof (rgch));
		if (cchT < cch)
			{
			bltb (rgch, rgch + (di = cch - cchT), cchT);
			SetBytes (rgch, ' ', di);
			}

#ifdef TEXTEVENT
/* This is disabled for Opus 1.0, but in the future CBT might want
   to teach the user to drag to a specific measurement, in which
   case we need to re-enable this code and they need to author in
   a Semantic Event (Text) pass/compare */
		if (fTellCBT)
			{
			/* Build up string to pass for CBT text event;
				x and y prompt text (the numerical part - if two of them,
				separate by a space); CbtTextEvent will take care of
				the other requirements of the text event string */

			if (cchCBT != 0)
				rgchCBT[cchCBT++] = chSpace;
			bltb(rgch, rgchCBT+cchCBT, cch);
			cchCBT += cch;
			}
		else
			/* this is broken if TEXTEVENT is turned on */
#endif /* TEXTEVENT */
#ifdef WIN23
			/* 
			 * Since we have two fields, and the prompt code is designed
			 * for one field, we are going to redraw the entire message
			 * each time that the information changes (instead of just
			 * the numeric field). -- phillipg
			 */
		if (vsci.fWin3)
			{
			struct RC rc;
			
			bltb( rgch, &vstPrompt[ich], cch );
			GetClientRect ( vhwndPrompt, (LPRECT)&rc );
			InvalidateRect( vhwndPrompt, (LPRECT)&rc, fFalse );
			}
		else
			AdjustPrompt2 (ich, cch, rgch);
#else
			AdjustPrompt (ich, cch, rgch);
#endif /* WIN23 */
		}

#ifdef TEXTEVENT
	if (fTellCBT)
		CbtTextEvent(rgchCBT, cchCBT);
#endif /* TEXTEVENT */

}


/********************************/
/*  %%Function:  ChangePicVals   %%Owner:  bobz       */

ChangePicVals (prcDrag, prcBasisPic, prcScale, ircs, fScale)
struct RC *prcDrag;
struct RC *prcBasisPic;
struct RC *prcScale;
int ircs;
int fScale;
{
	int xp, yp;
	int irgw;
	int rgw[6];
	int mRemainder;
	BOOL fNeg;
	int iCropOpposite;

	/* rgw gets initial vpicFetch values, then
		we store changed vals in this order:
		mx, my, dxaCropTop, dxaCropLeft, dxaCropBottom, dxaCropRight
	
	Code is essentially the same as ShowPicSize, but since
	that is called in a loop that needs to be fast, I have
	pretty much duplicated the code here. bz
	
	Set all crop vals, since only some may change. If cropping,
	set original scale vals below.
	*/

	rgw[0] = vpicFetch.mx;
	rgw[1] = vpicFetch.my;
	rgw[2] = vpicFetch.dyaCropTop;   /* crop vals */
	rgw[3] = vpicFetch.dxaCropLeft;
	rgw[4] = vpicFetch.dyaCropBottom;
	rgw[5] = vpicFetch.dxaCropRight;

	if (fScale)
		{
		if (ircs != ircsTC && ircs != ircsBC)
			{
			rgw[0] = XpScale();
			}
		if (ircs != ircsCR && ircs != ircsCL)
			{
			rgw[1] = YpScale();
			}
		}
	else
		{  /* cropping */
		rgw[0] = vpicFetch.mx;
		rgw[1] = vpicFetch.my;

		switch (ircs)
			{
			/* only deal with left or right line */

		case ircsTL:
		case ircsCL:
		case ircsBL:
			xp = prcDrag->xpLeft - prcBasisPic->xpLeft;
			irgw = 3;
			iCropOpposite = iCropRight;
			goto LeftRight;

		case ircsBR:
		case ircsCR:
		case ircsTR:
			xp = prcBasisPic->xpRight - prcDrag->xpRight;
			iCropOpposite = iCropLeft;
			irgw = 5;

LeftRight:
			/* Convert cropping distance to equivalent distance on
				goal picture size (in twips). This lets us drag using on screen
				rectangles that may be quite different than the goal rect
				(as a 1"x1" square goal bitmap that becomes 4"x4" on
				screen when the printer resolution changes from 300dpi to
				75 dpi. A 1" screen movement should be indicated as a 1/4"
				move ment on the goal picture, from which all change is
				measured. This takes care of the effect of scaling back
				cropping so that it is measured in terms of the unscaled
				picture as well.
			*/

			rgw[irgw] = XaCrop(xp, prcBasisPic, iCropOpposite);
			break;
			}

		switch (ircs)
			{
			/*  only deal with top or bottom line */

		case ircsBC:
		case ircsBR:
		case ircsBL:
			yp = prcBasisPic->ypBottom - prcDrag->ypBottom;
			irgw = 4;
			iCropOpposite = iCropTop;
			goto TopBottom;

		case ircsTL:
		case ircsTC:
		case ircsTR:
			yp = prcDrag->ypTop - prcBasisPic->ypTop;
			irgw = 2;
			iCropOpposite = iCropBottom;

TopBottom:
			/* See note above! convert to za wrt dyaGoal */
			rgw[irgw] = YaCrop(yp, prcBasisPic, iCropOpposite);
			break;
			}
		}  /* cropping */

	ChangePicFrame(NULL, vpicFetch.brcl,
			rgw[0],  /* mx */
			rgw[1],  /* my */
			rgw[2],   /* wCropTop */
			rgw[3],   /* wCropLeft */
			rgw[4],   /* wCropBottom */
	rgw[5]);   /* wCropRight) */


}


/********************************/
/*  %%Function:  GetBasisPicRc   %%Owner:  bobz       */

GetBasisPicRc (prcBasis, prcpic, fScale)
struct RC *prcBasis;
struct PICRC *prcpic;
int fScale;
{
	struct RC rcBrdr;  /* gets thicknesses of each border line in
				xp, yp fields */
	struct BRDR brdr;
	int dxpCropLeft;
	int dypCropTop;
	int dxpCropRight;
	int dypCropBottom;
	BOOL fNeg;

	/* for scaling, we want to unscale scale the outside frame border
		so changes in the drag rect, which is the outside fr border, will
		be proportional - this is easier that scaling back the cropping and
		borders etc.
	
		All we need for that is the size, so the origin is unimportant.
		For cropping, we want the outside frame rect before cropping,
		but after scaling. This way we treat the drag rect as a scaled,
		cropped rect, and compare it to a scaled uncropped rect. We need
		the actual coordinates for this, since we crop from any corner.
		*/

	*prcBasis = prcpic->frOut;

	if (fScale)
		{
		/* leave top left corner as it. The bottom right is
			offset by the unscaled height and width if there is scaling */

		if (vpicFetch.mx != mx100Pct)
			prcBasis->xpRight = prcBasis->xpLeft + 
					UMultDiv((prcBasis->xpRight - prcBasis->xpLeft),
					mx100Pct, vpicFetch.mx);

		if (vpicFetch.my != my100Pct)
			prcBasis->ypBottom = prcBasis->ypTop + 
					UMultDiv((prcBasis->ypBottom - prcBasis->ypTop),
					my100Pct, vpicFetch.my);
		}
	else
		{

		/* see hoo boy note in pic.c. He are making the cropping
			be in the same proportion to the screen rect as it was
			to the goal rect, for arcane reasons
		*/
		dxpCropLeft = NMultDiv( vpicFetch.dxaCropLeft,
				prcpic->picUnscl.xpRight - prcpic->picUnscl.xpLeft,
				vpicFetch.dxaGoal);
		dypCropTop =  NMultDiv( vpicFetch.dyaCropTop,
				prcpic->picUnscl.ypBottom - prcpic->picUnscl.ypTop,
				vpicFetch.dyaGoal);
		dxpCropRight = NMultDiv( vpicFetch.dxaCropRight,
				prcpic->picUnscl.xpRight - prcpic->picUnscl.xpLeft,
				vpicFetch.dxaGoal);
		dypCropBottom =  NMultDiv( vpicFetch.dyaCropBottom,
				prcpic->picUnscl.ypBottom - prcpic->picUnscl.ypTop,
				vpicFetch.dyaGoal);

		if (vpicFetch.mx != mx100Pct)
			{
			if (fNeg = (dxpCropLeft < 0))
				dxpCropLeft = -dxpCropLeft;
			dxpCropLeft = UMultDiv(dxpCropLeft, vpicFetch.mx, mx100Pct );
			if (fNeg)
				dxpCropLeft = -dxpCropLeft;
			if (fNeg = (dxpCropRight < 0))
				dxpCropRight = -dxpCropRight;
			dxpCropRight = UMultDiv(dxpCropRight, vpicFetch.mx, mx100Pct );
			if (fNeg)
				dxpCropRight = -dxpCropRight;
			}
		if (vpicFetch.my != my100Pct)
			{
			if (fNeg = (dypCropTop < 0))
				dypCropTop = -dypCropTop;
			dypCropTop = UMultDiv(dypCropTop, vpicFetch.my, my100Pct );
			if (fNeg)
				dypCropTop = -dypCropTop;
			if (fNeg = (dypCropBottom < 0))
				dypCropBottom = -dypCropBottom;
			dypCropBottom = UMultDiv(dypCropBottom, vpicFetch.my, my100Pct );
			if (fNeg)
				dypCropBottom = -dypCropBottom;
			}

		prcBasis->xpLeft -= dxpCropLeft;
		prcBasis->xpRight += dxpCropRight;
		prcBasis->ypTop -= dypCropTop;
		prcBasis->ypBottom += dypCropBottom;





#ifdef FUTURE   /* bz 10/10/89 */
          /* much simpler way to get scaled bordered rectangle:
             take the scaled rect and add in the borders; origin is
             unimportant, only size matters. Previously we unscaled
             the cropping to get there; this is far quicker and involves
             less rounding error. Note borders calculated as scaled
             for screen, which is how this rec currently is set up.

             It is not clear which method produces the most accurate result;
             We left the old, complex one in to make minimum changes,
             but consider this for the future (replaces entire else clause):
          */
	    *prcBasis = prcpic->picScl;
	    if (vpicFetch.brcl != brclNone)   /* no frame border? */
		    {
		    GetBrdrLineSizes (vpicFetch.brcl, NULL, &rcBrdr, fTrue /* fScaleForScreen */);
		    prcBasis->xpRight += rcBrdr.xpLeft + rcBrdr.xpRight;
		    prcBasis->ypBottom += rcBrdr.ypTop + rcBrdr.ypBottom;
		    }

#endif /* FUTURE */

#ifdef XBZ
	    CommSzRgNum(SzShared("GetRcBasisPic Basis rc: "),
				prcBasis, 4);
#endif

		}

}

/********************************/
/*  %%Function:  GetRcDragLim  %%Owner:  bobz       */

GetRcDragLim(prcDragLim, prcDrag, prcBasisPic, prcScale, ircs, fScale)
struct RC *prcDragLim;
struct RC *prcDrag;
struct RC *prcBasisPic;
struct RC *prcScale;
int ircs;
int fScale;

	/* based on the drag box being dragged, construct a limiting rectangle
		past which no dragging is allowed. Essentially, assume a fixed corner
		and give space for the minimum picture.
	*/
	/* MaxB 9/8/88 -- modifications made to stop drag limits going outside */
	/* of page area (in PageView).  Code shortened and additional bug that */
	/* did not allow you to drag a minimum picture in x-direct in y-direct */
	/* if next to page edge (or drag a minimum picture in y-direct in x-direct*/
{

	int dxpPicMax, dypPicMax;
	struct RC  rcMinMax ;
	struct RC  rcScale ;
	struct RC  rcClip ;

	int dxpPicMin;
	int dypPicMin;

	/* this is set up so that the picture will not be dragged to be
		> ~ 32K twips. Since the drag rect is offset by the min size,
		we subtract it so we can compare the drag rect size to 32K
		rather than the frame size. Parts of the pic can be off screen,
		so we can have this size problem.
	
		The 32k stuff is calculated based on the goal size, since 32k
		twips is the ultimate limit. Due to various strange roundings
		esp with bitmaps not being exactly he goal size, we can't just
		compare the rect we get with 32k twips converted to pixels. bz
	*/

	GetPicLim(&rcMinMax, prcBasisPic, prcScale, fScale);

	dxpPicMin = rcMinMax.xpRight;
	dypPicMin = rcMinMax.ypBottom;
	dxpPicMax = rcMinMax.xpLeft - dxpPicMin;
	dypPicMax = rcMinMax.ypTop - dypPicMin;

	Assert (dxpPicMax > 0 && dxpPicMax > 0 &&
			dxpPicMin > 0 && dxpPicMin > 0);

	/* Set up limits of screen -- either the whole window or
		(possibly) a partial window if in page view
	*/

	if ( (*hwwdCur)->fPageView )
		{
		rcClip.xpLeft = (*hwwdCur)->rcePage.xpLeft + vsci.dxpBorder ;
		rcClip.xpRight = (*hwwdCur)->rcePage.xpRight - vsci.dxpBorder ;
		rcClip.ypTop = (*hwwdCur)->rcePage.ypTop + vsci.dypBorder ;
		rcClip.ypBottom = (*hwwdCur)->rcePage.ypBottom - vsci.dypBorder ;
		}
	else
		{
		rcClip.xpLeft = (*hwwdCur)->xwMin ;
		rcClip.xpRight = (*hwwdCur)->xwMac ;
		rcClip.ypTop = (*hwwdCur)->ywMin ;
		rcClip.ypBottom = (*hwwdCur)->ywMac ;
		}

	*prcDragLim = *prcDrag;

	/*
		In order to eliminate checks, the actual number stored in
		ircs is significant, even though defined symbolically in pic.h
	*/
	Assert( ircs >= 0 && ircs < 8 ) ;
	Assert( ircsBC == 0 && ircsBR == 1 && ircsCR == 2 && ircsTR == 3 ) ;
	Assert( ircsTC == 4 && ircsTL == 5 && ircsCL == 6 && ircsBL == 7 ) ;

	/*
		For corner dragged boxes we have to set the horizontal
		and vertical limits of rectangle, but for center ones we
		only set one of the limits.
	*/

	/* Set horizontal limits */

	if ( ircs > 4 /* dragged box on corner, base put on right edge */ )
		/* ircsTL, ircsCL, ircsBL */
		{
		prcDragLim->xpRight = min(prcDrag->xpRight, prcBasisPic->xpRight)
			- dxpPicMin;
		prcDragLim->xpLeft = rcClip.xpLeft;
		if ( prcDragLim->xpRight - prcDragLim->xpLeft > dxpPicMax )
			prcDragLim->xpLeft =
					prcDragLim->xpRight - dxpPicMax;
		}
	else  if ( ircs < 4 /* dragged box on corner, base on left edge*/ )
		/* ircsTR, ircsCR, ircsBR */
		{
		prcDragLim->xpLeft = max(prcDrag->xpLeft, prcBasisPic->xpLeft)
			+ dxpPicMin;
		prcDragLim->xpRight = rcClip.xpRight;
		if (prcDragLim->xpRight - prcDragLim->xpLeft > dxpPicMax )
			prcDragLim->xpRight =
					prcDragLim->xpLeft + dxpPicMax;
		}
	/* else we are ircsTC or ircsBC, leave horizontal dimensions equal */
	/* to original box. if we do either of the above will introduce    */
	/* obscure bug when try to drag horizontal-minimum box vertically  */

	/* Set vertical limits */

	if ( ircs < 2 || ircs == 7 )
		/* dragged box on corner, base goes on the top edge */
		/* ircsBL, ircsBC, ircsBR                           */
		{
		prcDragLim->ypTop = max(prcDrag->ypTop, prcBasisPic->ypTop)
			+ dypPicMin;
		prcDragLim->ypBottom=rcClip.ypBottom;
		if (prcDragLim->ypBottom - prcDragLim->ypTop > dypPicMax )
			prcDragLim->ypBottom =
					prcDragLim->ypTop + dypPicMax;
		}
	else  if ( ircs > 2 && ircs < 6 )
		/* dragged box on corner, base goes on the top edge */
		/* ircsTL, ircsTC, ircsTR                           */
		{
		prcDragLim->ypBottom = min(prcDrag->ypBottom, prcBasisPic->ypBottom)
			- dypPicMin;
		prcDragLim->ypTop = rcClip.ypTop;
		if (prcDragLim->ypBottom - prcDragLim->ypTop > dypPicMax )
			prcDragLim->ypTop =
					prcDragLim->ypBottom - dypPicMax;
		}
	/* else we are ircsCL or ircsCR, leave vertical dimensions equal   */
	/* to original box. if we do either of the above will introduce    */
	/* obscure bug when try to drag vertical-minimum box horizontally  */

	/* clip the drag rect to the display rectangle. If no horiz
	scroll bar, and picture is below the prompt line, you
	will be able to move a bit but not see it.
	*/

	SectRc( prcDragLim, &rcClip, prcDragLim ) ;

#ifdef XBZ
	CommSzNum(SzShared("GetRcDragLim ircs: "), ircs);
	CommSzRgNum(SzShared("GetRcDragLim lim rect size = "),
			prcDragLim, 4);
	CommSzRgNum(SzShared("GetRcDragLim RcwDisp = "),
			&(*hwwdCur)->rcwDisp, 4);
#endif
}



/********************************/
/*  %%Function:  GetPicLim  %%Owner:  bobz       */

GetPicLim(prcMinMax, prcBasis, prcScale, fScale)
struct RC *prcMinMax;  /* 1st pair is max xp, yp, 2nd pair is min xp, yp */
struct RC *prcBasis;
struct RC *prcScale;
int fScale;
{

	int dzaCropped, dzaGoal, dzaMin, dzaMax;
	unsigned mz;
	long ml;

	/* figure out the max xp based on the effect on the goal rectangle,
		contained in vpicFetch. For scaling, make the scaling amount no
		bigger that what would overflow the twips values. For cropping,
		no amount larger than would overflow the goal in twips is allowed.
	
		This all comes up because the calculated bitmap size may be different
		than the goal size converted to pixels (see GetBitmapSize) and if we
		just used the ratio of the actual rects, we could be too large for
		one or the other of the drag box and the format pic dialog box.
	
		For cropping, we make the amount of cropping on screen be proportional
		to the goal rectangle, which is where the values eventually are used.
	
		bobz may be able to explain this.
	*/


	if (fScale)
		{
		dzaCropped = (vpicFetch.dxaGoal - vpicFetch.dxaCropLeft -
				vpicFetch.dxaCropRight);
		if  (dzaCropped < dxaPicMinDrag)
			{
			dzaCropped = vpicFetch.dxaGoal;
			PictError(eidBadCrop);
			}


		/* max scaling factor * 10 truncated to lower whole % */
		dzaGoal =  max (vpicFetch.dxaGoal, dzaCropped);
        Assert (dzaGoal > 0);
		/* break into 2 lines so LDIV used instead of SDIV */
		ml = (long)dxaPicMax * (long)1000 / (long) dzaGoal;
        Assert (ml > 0L);
		if (ml < (long)(unsigned)(mzPicMax * PctTomx100Pct))
            mz = (unsigned)ml;
        else
		      /* overflow - use limit */
			mz = mzPicMax * PctTomx100Pct;

		/* max xp */
		prcMinMax->xpLeft = UMultDiv (mz,
				(prcBasis->xpRight - prcBasis->xpLeft), 1000);
		prcScale->xpLeft = mz;

		/* min scaling factor * 10 rounded up to next higher whole % */
		dzaGoal =  min (vpicFetch.dxaGoal, dzaCropped);
        Assert (dzaGoal > 0);
		/* break into 2 lines so LDIV used instead of SDIV */
		ml =  ((long)(dxaPicMinDrag) * (long)1000 + (long)((dzaGoal - 1) * 10))
				/ (long) dzaGoal;
		Assert (ml > 0L);
          /* so we get at least 1% */
        mz = umax(mzPicMin, (unsigned)ml);
		prcMinMax->xpRight = UMultDiv (mz,
				(prcBasis->xpRight - prcBasis->xpLeft), 1000);
		prcScale->xpRight = mz;

		dzaCropped = (vpicFetch.dyaGoal - vpicFetch.dyaCropTop -
				vpicFetch.dyaCropBottom);
		if  (dzaCropped < dyaPicMinDrag)
			{
			dzaCropped = vpicFetch.dyaGoal;
			PictError(eidBadCrop);
			}

		dzaGoal =  max (vpicFetch.dyaGoal, dzaCropped);
        Assert (dzaGoal > 0);
		ml = (long)dyaPicMax * (long)1000 / (long) dzaGoal;

        Assert (ml > 0L);
		if (ml < (long)(unsigned)(mzPicMax * PctTomy100Pct))
            mz = (unsigned)ml;
        else
		      /* overflow - use limit */
			mz = mzPicMax * PctTomy100Pct;

		prcMinMax->ypTop = UMultDiv (mz,
				(prcBasis->ypBottom - prcBasis->ypTop), 1000);
            
		prcScale->ypTop = mz;

		dzaGoal =  min (vpicFetch.dyaGoal, dzaCropped);
        Assert (dzaGoal > 0);
		ml =  ((long)(dyaPicMinDrag) * (long)1000 + (long)((dzaGoal - 1) * 10))
				/ (long) dzaGoal;
		Assert (ml > 0L);
          /* so we get at least 1% */
        mz = umax(mzPicMin, (unsigned)ml);
		prcMinMax->ypBottom = UMultDiv (mz,
				(prcBasis->ypBottom - prcBasis->ypTop), 1000);
		prcScale->ypBottom = mz;
		}
	else
		{
		/* cropping: the max/min sizes are calculated this way:
			(goal - dzaCrop)*mz >=min, <=max
			for min case,
			dzaCrop <= goal - min/mz
			minpicsize = goal - max dzaCrop = goal - (goal - min/mz) = min/mz
		
			so we have to scale the min sizes, then make them proportional
			to the basis rectangle.
		
			for min, the min pic size is the max(min, min/mz).
			for max, the max pic size is the min(max, max/mz), since neither
			the unscaled pic nor the scaled pic can be > max. The min is
			max if mz is <= 100% and max/mz otherwise.
		*/
		if (vpicFetch.mx != mx100Pct)
			{
			if (vpicFetch.mx < mx100Pct)
				{
				dzaMax = dxaPicMax;
				dzaMin = UMultDiv(dxaPicMinDrag, mx100Pct, vpicFetch.mx);
				}
			else
				{
				dzaMax = UMultDiv(dxaPicMax, mx100Pct, vpicFetch.mx);
				dzaMin = dxaPicMinDrag;
				}
			}
		else
			{
			dzaMax = dxaPicMax;
			dzaMin = dxaPicMinDrag;
			}

		prcMinMax->xpLeft = NMultDiv (dzaMax,
				(prcBasis->xpRight - prcBasis->xpLeft), vpicFetch.dxaGoal);
		prcMinMax->xpRight = NMultDiv (dzaMin,
				(prcBasis->xpRight - prcBasis->xpLeft), vpicFetch.dxaGoal);

		if (vpicFetch.my != my100Pct)
			{
			if (vpicFetch.my < my100Pct)
				{
				dzaMax = dyaPicMax;
				dzaMin = UMultDiv(dyaPicMinDrag, my100Pct, vpicFetch.my);
				}
			else
				{
				dzaMax = UMultDiv(dyaPicMax, my100Pct, vpicFetch.my);
				dzaMin = dyaPicMinDrag;
				}
			}
		else
			{
			dzaMax = dyaPicMax;
			dzaMin = dyaPicMinDrag;
			}

		prcMinMax->ypTop = NMultDiv (dzaMax,
				(prcBasis->ypBottom - prcBasis->ypTop), vpicFetch.dyaGoal);
		prcMinMax->ypBottom = NMultDiv (dzaMin,
				(prcBasis->ypBottom - prcBasis->ypTop), vpicFetch.dyaGoal);
		}
#ifdef XBZ
		{
		CommSzRgNum(SzShared("GetPicLim rcBasis: "),
				prcBasis, 4);
		CommSzRgNum(SzShared("GetPicLim rcMinMax: "),
				prcMinMax, 4);
		CommSzRgNum(SzShared("GetPicLim rcScale: "),
				prcScale, 4);
		}
#endif
	/* overflows in NMultDiv will set the lim to 32k pixels, not no
       need to do anything special in that case
    */
}


/********************************/
/* P t	P r o p  R e c t */
/*  %%Function:  PtPropRect  %%Owner:  bobz       */

struct PT PtPropRect(ptMouse, pdrcCorner, prcLim)
struct PT ptMouse;
struct DRC *pdrcCorner;
struct RC  *prcLim;
{


	struct PT ptResult;

	/* NOTE: this is a minor modification of the original Mac code which
	only had one diagonal to deal with. In Opus, you can be dragging
	along either diagonal, and we are always using the diagonal of the
	original rectangle, so some values are precalculated in pdrcCorner,
	including the point on the diagonal (xp, yp) and the dxp dyp needed
	for the slope.
	*/

/* Reposition a point in order to maintain a rectangle's proportions.
		The calculations for this are motivated by trying to keep the mouse
		point on a line which coincides with the top-left to bottom-right
		diagonal of the original picture

		Let s be the slope of the diagonal:
			if diagonal from top left to bottom right
		0.a (ypBottom - ypTop) / (xpRight - xpLeft)
			if diagonal from top right to bottom left
		0.b (ypBottom - ypTop) / (xpLeft - xpRight)
		Let (xpResult,ypResult) be the point to be calculated
		Let (xpMouse,ypMouse) be the current mouse location
		The point (xpResult,ypResult) lies at the intersection of two perpen-
	dicular lines, one of which has slope s and contains (xpLeft,ypTop)
	and the other of which has slope -s and contains (xpMouse,ypMouse)
		Then the following simultaneous equations solve for (xpResult,ypResult):
			1. s = (ypResult - ypDiag) / (xpResult - xpDiag)  <-- note change
			2. -s = (ypResult - ypMouse) / (xpResult - xpMouse)
	and
			3.  ypResult = (ypDiag + ypMouse + s(xpMouse - xpDiag)) / 2
			4.  xpResult = (ypMouse - ypDiag) / 2s + (xpDiag + xpMouse) / 2
*/

/* Note xpDiag, ypDiag are the xp, yp of pdrcCorner, and the dxp, dyp used
	for slope are pdrcCorner's dxp, dyp
*/

/* use equations 3 and 4 to get proposed result point */
/* note that we use the 0.a or 0.b equations to get s, rather
	than equation 1
*/

	ptResult.yp = (pdrcCorner->yp + ptMouse.yp +
			NMultDiv(ptMouse.xp - pdrcCorner->xp,
			pdrcCorner->dyp, pdrcCorner->dxp)) >> 1;

	ptResult.xp = ((pdrcCorner->xp + ptMouse.xp) >> 1) +
			NMultDiv(ptMouse.yp - pdrcCorner->yp,
			pdrcCorner->dxp, pdrcCorner->dyp << 1);



#ifdef XBZTEST
	CommSzRgNum(SzShared("PtPropRect ptResult before lim : "),
			&ptResult, 2);
#endif

/* if we overflow limit rectangle, use equation 1 to solve for the xp or yp
	that will just take us to the limit. Note that we can't overflow in both
	dimensions because either
	1. prcLim and prc have the same proportions, and thus the same
			diagonal, in which case pinning the mouse to prcLim prevents
			overflow entirely, or
	2. the ratio of same sides prcLim/prc is greater in exactly one
		dimension, which allows the overflow in the other dimension
*/
	if (ptResult.yp > prcLim->ypBottom)
		{
		ptResult.yp = prcLim->ypBottom;
		goto CalcXp;
		}
	else  if (ptResult.yp < prcLim->ypTop)
		{
		ptResult.yp = prcLim->ypTop;
CalcXp:
		ptResult.xp = pdrcCorner->xp +
				NMultDiv(ptResult.yp - pdrcCorner->yp,
				pdrcCorner->dxp, pdrcCorner->dyp);
		goto HaveXpYp;
		}

	if (ptResult.xp > prcLim->xpRight)
		{
		ptResult.xp = prcLim->xpRight;
		goto CalcYp;
		}
	else  if (ptResult.xp < prcLim->xpLeft)
		{
		ptResult.xp = prcLim->xpLeft;
CalcYp:
		ptResult.yp = pdrcCorner->yp +
				NMultDiv(ptResult.xp - pdrcCorner->xp,
				pdrcCorner->dyp, pdrcCorner->dxp);
		}

HaveXpYp:

#ifdef XBZTEST
	CommSzRgNum(SzShared("PtPropRect ptMouse: "),
			&ptMouse, 2);
	CommSzRgNum(SzShared("PtPropRect ptResult final : "),
			&ptResult, 2);
#endif

	return(ptResult);
}


/*  %%Function:  FrameRectPic  %%Owner:  bobz       */

FrameRectPic(hdc, prc)
HDC hdc;
struct RC *prc;
{
	/*  Uses PatBlt to draw a rectangle using raster-op PATINVERT. */
	int dxp = prc->xpRight - prc->xpLeft;
	int dyp = prc->ypBottom - prc->ypTop - (vsci.dypBorder << 1);

	/* top */
	PatBlt( hdc, prc->xpLeft, prc->ypTop,
			dxp, vsci.dypBorder, PATINVERT);
	/* right */
	PatBlt( hdc, prc->xpRight - vsci.dxpBorder,
			prc->ypTop + vsci.dypBorder,
			vsci.dxpBorder, dyp, PATINVERT);
	/* left */
	PatBlt( hdc, prc->xpLeft,
			prc->ypTop + vsci.dypBorder,
			vsci.dxpBorder, dyp, PATINVERT);
	/* bottom */
	PatBlt( hdc, prc->xpLeft,
			prc->ypBottom - vsci.dypBorder,
			dxp, vsci.dypBorder, PATINVERT);
}


