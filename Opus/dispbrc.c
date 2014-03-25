/* D I S P B R C . C */
#define WINDOWS

#define SCREENDEFS
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "file.h"
#include "ch.h"
#include "props.h"
#include "border.h"
#include "format.h"
#include "formula.h"
#include "disp.h"
#include "scc.h"
#include "sel.h"
#include "prm.h"
#include "print.h"
#include "style.h"
#include "doc.h"
#include "ruler.h"
#include "heap.h"
#include "debug.h"


extern struct WWD       **mpwwhwwd[];
extern struct DOD       **mpdochdod[];
extern int              wwCur;
extern struct FLI       vfli;
extern struct WWD       **hwwdCur;
extern int              catrLast;
extern struct SCC       vsccAbove;
extern struct SCI	vsci;
extern struct FTI       vfti;
extern struct PRSU      vprsu;
extern struct SEL       selCur;
extern struct TCC       vtcc;
extern struct CA        caSect;
extern struct SEP       vsepFetch;
extern struct FMTSS     vfmtss;
extern struct SEL       selDotted;
extern struct PAP       vpapFetch;
extern struct PRI       vpri;
extern struct TAP       vtapFetch;
extern int		vlm;
extern struct SCI vsci;
extern struct MWD **hmwdCur;
extern int        mwMac;
extern int        wwMac;
extern int        wwCur;
extern int        mwCur;
extern struct MERR      vmerr;
extern HCURSOR    vhcArrow;
extern HCURSOR    vhcStyWnd;
extern struct MWD **mpmwhmwd[];
extern struct CA  caPara;
extern struct WWD **mpwwhwwd[];
extern struct DOD **mpdochdod[];
extern struct PAP  vpapFetch;
extern int         vdypStyWndFont;
extern struct FLI  vfli;
extern struct PREF vpref;
extern HFONT       vhfntStyWnd;
extern struct SEL selCur;
extern struct WWD **hwwdCur;
extern struct SCC       vsccAbove;
extern struct WWD **vhwwdOrigin;
extern struct PREF      vpref;
extern HBRUSH		vhbrGray;
extern HBRUSH		vhbrBlack;
extern struct CHP       vchpFetch;
extern char HUGE        *vhpchFetch;
extern uns              vccpFetch;
extern CP vcpFirstTablePara;
struct RC *PrcSelBar();


/* D R A W  B O R D E R S */
/* Use data in vfli to draw paragraph borders for current line. */
/* %%Function:DrawBorders %%Owner:tomsax %%Reviewed:7/6/89 */
EXPORT DrawBorders(dxpToXw, ywLine)
int	dxpToXw, ywLine;
{
	int xwFirst, xwMid, xwLim;
	int dxBrcLeft;
	struct RC rcw;
	int rgbrc[cbrcCorner];

	dxBrcLeft = DxFromBrc(vfli.brcLeft,fFalse/*fFrameLines*/);
	xwFirst = rcw.xwLeft = vfli.xpMarginLeft - dxpLeftRightSpace
			- dxBrcLeft + dxpToXw;
	xwMid = rcw.xwRight = vfli.xpMarginRight + dxpLeftRightSpace + dxpToXw;
	xwLim = xwMid + DxFromBrc(vfli.brcRight,fFalse/*fFrameLines*/);

	rcw.ywBottom = ywLine - vfli.dypLine;
	if (vfli.fTop)
		{
		rcw.ywBottom += vfli.fBetweenTop ? vfli.dypBrcTop : vfli.dypBefore;
		Assert(vfli.dypBrcTop == DyFromBrc(vfli.brcTop,fFalse/*fDrawDrs*/));
		rcw.ywTop = rcw.ywBottom - vfli.dypBrcTop;
		rgbrc[ibrcDraw] = vfli.brcTop;
		rgbrc[ibrcBottomButt] = vfli.brcLeft;
		rgbrc[ibrcLeftButt] = brcNil;
		rgbrc[ibrcTopButt] = vfli.fBetweenTop ? rgbrc[ibrcBottomButt] : brcNil;
		DrawBrc ( vfli.ww, rgbrc, rcw, dbmodTop, fFalse/*fDrawDrs*/);

		rcw.xwLeft = rcw.xwRight;
		rcw.xwRight = xwLim;
		rgbrc[ibrcLeftButt] = rgbrc[ibrcDraw];
		rgbrc[ibrcBottomButt] = vfli.brcRight;
		rgbrc[ibrcDraw] = brcNil; /* hint to corner-drawing code */
		rgbrc[ibrcTopButt] = vfli.fBetweenTop ? rgbrc[ibrcBottomButt] : brcNil;
		DrawBrc ( vfli.ww, rgbrc, rcw, dbmodTop, fFalse/*fDrawDrs*/);

		}

	rcw.ywTop = rcw.ywBottom;
	rcw.ywBottom = ywLine;
	if (vfli.fBottom)
		rcw.ywBottom -= vfli.fBetweenBottom
				? vfli.dypBrcBottom : vfli.dypAfter;

	if (vfli.fLeft)
		{
		rcw.xwLeft = xwFirst;
		rgbrc[ibrcDraw] = vfli.brcLeft;
		rcw.xwRight = xwFirst + dxBrcLeft;
		DrawBrc ( vfli.ww, rgbrc, rcw, dbmodLeft, fFalse/*fDrawDrs*/);
		}

	if (vfli.fRight)
		{
		rcw.xwLeft = xwMid;
		rcw.xwRight = xwLim;
		rgbrc[ibrcDraw] = vfli.brcRight;
		DrawBrc ( vfli.ww, rgbrc, rcw, dbmodRight, fFalse/*fDrawDrs*/);
		}

	if (vfli.fBottom)
		{
		Assert(vfli.brcBottom != brcNil);
		rcw.ywTop = rcw.ywBottom;
		Assert(vfli.dypBrcBottom == DyFromBrc(vfli.brcBottom,fFalse/*fDrawDrs*/));
		rcw.ywBottom += vfli.dypBrcBottom;
		rcw.xwLeft = xwFirst;
		rcw.xwRight = xwMid;

		rgbrc[ibrcDraw] = vfli.brcBottom;
		rgbrc[ibrcTopButt] = vfli.brcLeft;
		rgbrc[ibrcBottomButt] = rgbrc[ibrcLeftButt] = brcNil;
		DrawBrc ( vfli.ww, rgbrc, rcw, dbmodBottom, fFalse/*fDrawDrs*/);

		rcw.xwLeft = rcw.xwRight;
		rcw.xwRight = xwLim;
		rgbrc[ibrcLeftButt] = rgbrc[ibrcDraw];
		rgbrc[ibrcTopButt] = vfli.brcRight;

		/* setting this to brcNil serves as a hint to the corner-drawing code */
		rgbrc[ibrcDraw] = (vfli.brcBottom==vfli.brcRight)?brcNil:vfli.brcBottom;

		DrawBrc ( vfli.ww, rgbrc, rcw, dbmodBottom, fFalse/*fDrawDrs*/);
		}

}


/* ------------- Routines transplanted from disptbl.c ------------------*/
/* D R A W  B R C
/* Description: Given sufficient information, draw a border (or a frame line)
/* between cells in a table.
/*
/* Game Plan: Divide up the rcw which we have to draw into strips running
/* parallel to the "line". Assign to each strip a color/pattern. Then
/* loop through the strips, painting them as required. This includes clearing
/* those bits that are to be white. Under this scheme, every bit gets drawn
/* precisely once.
/*
/* If dbmod is dbmodHoriz, then rgbrc contains information about the lines
/* which butt up against the left end of the current line. If dbmodHoriz,
/* then rgbrc[0] contians the current code; the others are undefined.
/*
/*
/*						|     |
/*						|     |
/*						| rgbrc[ibrcTopButt] 
/*						|     |
/*						|     |
/* -------------------------------------------------------------
/*	rgbrc[ibrcLeftButt]	|    rgbrc[ibrcDraw] (draw this one)		|
/* -------------------------------------------------------------
/*						|     |
/*						|     |
/*						| rgbrc[ibrcBottomButt] 
/*						|     |
/*						|     |
/*
/*
/*
/*
/**/

/* %%Function:DrawBrc %%Owner:tomsax %%Reviewed:7/6/89 */
DrawBrc ( ww, rgbrc, rcw, dbmod, fFrameLines )
int rgbrc[];
struct RC rcw;
BOOL fFrameLines;
{
	int bl;
	int brcCur;
	int rgbitMatch;
	int brcMatch, cbrcMatch;
	BOOL fLeft, fRight, fTop, fBottom;
	int dx, dy;
	int xw1, xw2, yw1, yw2;
	int dxwBottomButt, dxwWidth;
	BOOL fVertical, fEraseBits;
	int *pdz;
#ifdef	MAC
	int dxpPen, dypPen;
	Mac(BOOL fLaserWriter, fHairline);
#endif	/*MAC*/
#ifdef WIN
	HDC hdc = PwwdWw(ww)->hdc;
	HBRUSH hbr;
	struct RC rcDraw;
#endif /* WIN */
	struct BRK brk, brkCorner;
	struct RC rcwCorner;
	int rgbrcT[cbrcCorner];
	int mpblz[blMax+1];

	if (FEmptyRc(&rcw))
		return;

#ifdef MAC
	fLaserWriter = vlm == lmPrint && vprsu.prid == pridLaser;
#endif
	if ( !(fEraseBits = (vlm != lmPrint && vlm != lmPreview)))
		fFrameLines = fFalse;
	fVertical = (dbmod & dbmodHV) == dbmodVertical;

	/* initialize things */
	PenNormal();
#ifdef WIN
	hbr = vfli.fPrint ? vpri.hbrText : vsci.hbrText;
#endif

	/* decode the brc info */
	BrkFromBrc ( brcCur = rgbrc[ibrcDraw], &brk, fFrameLines, dbmod & dbmodReverse );

	/* copy the endpoints of the strip (width direction) into array */
	mpblz[0] = fVertical ? rcw.xwLeft : rcw.ywTop;
	mpblz[brk.blMac+1] = fVertical ? rcw.xwRight : rcw.ywBottom;

	Assert((fVertical && (mpblz[brk.blMac+1] - mpblz[0]) >= brk.dxWidth)
			|| (!fVertical && (mpblz[brk.blMac + 1] - mpblz[0]) >= brk.dyHeight));

	/* decide what to do with the shadow code */
	if (brk.fShadow)
		{
		if (dbmod & dbmodReverse)
			{
			Assert(brk.mpblblp[0] == blpShadow);
			brk.mpblblp[0] = blpWhite;
			}
		else
			{
#ifdef MAC
			if (fLaserWriter && fVertical && brk.mpblblp[0] == blpLaser)
				{
				Assert(brk.blMac == 2);
				brk.mpblblp[0] = blpBlack;
				brk.mpblblp[1] = blpWhite;
				}
			else
#endif /* MAC */
					{
					Assert(brk.mpblblp[brk.blMac-1] == blpShadow);
					brk.mpblblp[brk.blMac-1] = blpBlack;
					}
			}
		}

	/* build array of internal strip limits */
	pdz = fVertical ? (&brk.mpbldx[0]) : (&brk.mpbldy[0]);
	for ( bl = 0; bl < brk.blMac; ++bl )
		mpblz[bl+1] = mpblz[bl] + *pdz++;
	Assert((fVertical && (mpblz[brk.blMac] - mpblz[0]) == brk.dxWidth) ||
			(!fVertical && (mpblz[brk.blMac] - mpblz[0]) == brk.dyHeight));


	/* if dbmodVertical, there are no connections to deal with */
	if (fVertical)
		goto LDrawBrc;

	/* search the brc array for matches among the neighbors */
	Break3();

	cbrcMatch = CbrcMatch(rgbrc,fFrameLines,&brcMatch,&rgbitMatch);

#ifdef MAC
	fHairline = fLaserWriter && (brcMatch & ~(brcfShadow | brcDxpSpace)) == brcHairline;
#endif

	if (cbrcMatch == 0)
		{
		/* If, ignoring frame lines, the current border has zero-width,
		/* and the left border has non-zero width, extend the left border
		/* into the current area by an amount equal to the max width of the
		/* top and bottom borders. This makes a border above a given cell
		/* left/right symmetrical if there is nothing around to interfere.
		/**/
		if (DyFromBrc(brcCur,fFalse/*fFrameLines*/) == 0
				&& DyFromBrc(rgbrc[ibrcLeftButt],fFalse/*fFrameLines*/) != 0)
			{
			/* This is a kludge to trick the corner-drawing block
			/* into extending the border on the left into the
			/* (non-existent) current border.
			/**/
			brcMatch = rgbrc[ibrcLeftButt];
#ifdef MAC
			if (fLaserWriter && brcMatch == brcHairline
					&& (dxwWidth=DxFromBrc(rgbrc[ibrcBottomButt],fFalse/*fFrameLines*/)) > 0)
				{
				fHairline = fTrue;
				goto LExtendBottom;
				}
#endif /* MAC */
			rgbitMatch = 0x0B;
			BrkFromBrc ( brcMatch,&brkCorner,fFrameLines,fFalse );
			rcwCorner = rcw;
			rcwCorner.xwRight = rcw.xwLeft +=
					max(DxFromBrc(rgbrc[ibrcTopButt],fFrameLines),
					DxFromBrc(rgbrc[ibrcBottomButt],fFrameLines));
			if (brkCorner.dxWidth == 0
					|| (dxwWidth = rcwCorner.xwRight - rcwCorner.xwLeft) <= 0
					|| rcwCorner.xwRight > rcw.xwRight)
				{
				rcw.xwLeft = rcwCorner.xwLeft; /* restore rcw */
				goto LDrawBrc;
				}
#ifdef MAC
			else  if (fLaserWriter
					&& (rgbrc[ibrcTopButt] == brcHairline || rgbrc[ibrcBottomButt] == brcHairline)
					&& (rgbrc[ibrcTopButt] == brcHairline || DxFromBrc(rgbrc[ibrcTopButt],fFalse) == 0)
					&& (rgbrc[ibrcBottomButt] == brcHairline || DxFromBrc(rgbrc[ibrcBottomButt],fFalse) == 0))
				{
				/* mis-use a dxw as a dyw */
				rcw.xwLeft = rcwCorner.xwLeft; /* restore rcw */
LDrawHairyCorner:
				SetHairline();
				dxwWidth = DxFromBrc(rgbrc[ibrcBottomButt],fFalse) == 0
						? max(DyFromBrc(rgbrc[ibrcLeftButt],fFalse),DyFromBrc(brcCur,fFalse))
						: rcw.ywBottom-rcw.ywTop;
				MoveTo(rcw.xwLeft,rcw.ywTop);
				Line(0,dxwWidth - 1);
				ResetHairline();

				goto LDrawBrc;
				}
#endif /* MAC */

			goto LProcessCorner;
			}
		else  if (DyFromBrc(brcCur,fTrue/*fFrameLines*/) < rcw.ywBottom - rcw.ywTop
				&& DxFromBrc(rgbrc[ibrcBottomButt],fFalse/*fFrameLines*/) != 0)
			{
			/* Bring the border below up to join with the current border */
			rcwCorner = rcw;	/* misuse a variable */
			rcwCorner.ywTop += DyFromBrc(brcCur,fFrameLines);
			rcwCorner.xwRight = rcwCorner.xwLeft + DxFromBrc(rgbrc[ibrcBottomButt],fFrameLines);
			DrawBrc(ww,&rgbrc[ibrcBottomButt],rcwCorner,dbmodLeft,fFrameLines);
			SetWords(rgbrcT,brcNil,cbrcCorner);
			rgbrcT[ibrcDraw] = brcCur;
			rcwCorner.ywBottom = rcwCorner.ywTop;
			rcwCorner.ywTop = rcw.ywTop;
			DrawBrc(ww,rgbrcT,rcwCorner,dbmodTop,fFrameLines);
			rcw.xwLeft = rcwCorner.xwRight;
			goto LDrawBrc;
			}
#ifdef MAC
		else  if (fLaserWriter)
			{
			if (brcCur == brcHairline && (dxwWidth=DxFromBrc(rgbrc[ibrcBottomButt],fFalse/*fFrameLines*/)) != 0)
				{
				fHairline = fTrue;
				goto LExtendBottom;
				}
			else  if ((rgbrc[ibrcTopButt] & ~(brcfShadow | brcDxpSpace)) == brcHairline
					|| (rgbrc[ibrcBottomButt] & ~(brcfShadow | brcDxpSpace)) == brcHairline)
				goto LDrawHairyCorner;
			}
#endif /* MAC */
		goto LDrawBrc;
		}

	/* we have found some matches among the brc's. Draw the left-most portion
	/* of the rcw to make the connection. The kludgy conditional is to get
	/* the BRK reversed for shadows under the necessary circumstances.
	/**/
	Break3();
#ifdef MAC
	if (fHairline)
		{
		if (brcCur != brcMatch && DzFromBrc(brcCur,fFalse) > 0)
			{
			if ((rgbitMatch & 0xC) == 0x0C) /* top and bottom are hairline */
				{
				SetHairline();
				MoveTo(rcw.xwLeft,rcw.ywTop);
				LineTo(rcw.xwLeft,rcw.ywBottom-1);
				ResetHairline();
				}
			goto LDrawBrc;
			}
		else  if ((dxwWidth=DxFromBrc(rgbrc[ibrcBottomButt],fFalse/*fFrameLines*/)) > 0
				&& (rgbrc[ibrcBottomButt] & ~(brcfShadow | brcDxpSpace)) != brcHairline)
			{
LExtendBottom:
			/* draw the hairline for the corner segment */
			if (fHairline || brcCur != brcMatch && DzFromBrc(brcCur,fFalse) > 0)
				{
				if (brcCur == brcNone || brcCur == brcNil ||
						rcw.xwLeft + dxwWidth >= rcw.xwRight)
					dxwWidth--;
				SetHairline();
				MoveTo(rcw.xwLeft,rcw.ywTop);
				Line(max(1,dxwWidth-1),0);
				ResetHairline();
				}

			/* bring the vertical border below up to join with the hairline */
			fHairline = fFalse;
			rgbitMatch = 0xC; /* top and bottom */
			brcMatch = rgbrc[ibrcBottomButt];
			cbrcMatch = 1;
			}
		}
	if (fHairline && brcMatch == (brcHairline | brcfShadow))
		{
		if ((rgbitMatch & 0x6) == 0x6)
			{
			fHairline = fFalse;
			brcMatch = brcSingle;
			}
		else  if ((dbmod & dbmodReverse) == 0)
			{
			int rgbrcT[cbrcCorner];

			SetHairline();
			MoveTo(rcw.xwLeft+1,rcw.ywTop);
			Line(1,0);
			ResetHairline();

			rgbrcT[0] = brcSingle;
			SetWords(&rgbrcT[1],brcNone,cbrcCorner-1);
			rcw.xwLeft += max(DxFromBrc(rgbrc[ibrcTopButt],fFalse),DxFromBrc(rgbrc[ibrcBottomButt],fFalse));
			DrawBrc(ww,rgbrcT,rcw,dbmod,fFrameLines);
			return;
			}
		else  if (brcCur == brcNil)
			{
			SetHairline();
			MoveTo(rcw.xwLeft,rcw.ywBottom-1);
			Line(0,1);
			ResetHairline();
			return;
			}
		}
#endif /* MAC */

	BrkFromBrc(brcMatch,&brkCorner,fFrameLines,
			(rgbitMatch&0x0c)==0x0c && brcCur!=brcNil/*fReverse*/);

	rcwCorner = rcw; /* unless changed in the next block... */

	rcwCorner.xwRight = rcwCorner.xwLeft + brkCorner.dxWidth; /*assumed later*/
	dxwWidth = brkCorner.dxWidth;

LProcessCorner:
#ifdef MAC
	/* If we're drawing a hairline on the LaserWriter, we must skip the
	/* various conversions that normally occur in the next block.
	/**/
	if (fHairline)
		goto LDrawCorner;
#endif /* MAC */

	switch ( rgbitMatch )
		{
		/* bits are: bottom - top - left - right */
	default:
		Assert ( fFalse );
		break;

	case 0x3: /* 0011 left and right */
	case 0x7: /* 0111 top, left and right */
		rgbitMatch = 0xB;
	case 0xB: /* 1011 bottom, left and right */
		/* fall through to next block */

	case 0x6: /* 0110 top and left */
		if (brkCorner.fShadow)
			{
			Assert(brkCorner.mpblblp[brkCorner.blMac-1]==blpShadow);
			brkCorner.mpblblp[brkCorner.blMac-1] = blpBlack;
			}
		/* fall through to next block */

	case 0x5: /* 0101 top and right */

		/* Bring the bottom brc up to butt with the corner we are
		/* about to draw. Then slice that portion off from rcwCorner.
		/**/
		rcwCorner.ywTop += brkCorner.dyHeight;

		dxwBottomButt = DxFromBrc(rgbrc[ibrcBottomButt],fFrameLines);
		if (fEraseBits && dxwBottomButt < dxwWidth)
			{
			/* erase bits below the impending corner connection and
			/* to the right of the impending bottom butt connection.
			/**/
			rcwCorner.xwLeft += dxwBottomButt;
			EraseRc(ww, &rcwCorner);
			rcwCorner.xwLeft -= dxwBottomButt;
			}
		rcwCorner.xwRight = rcwCorner.xwLeft + dxwBottomButt;
		if (rgbrc[ibrcBottomButt] != brcNil && rcwCorner.ywTop < rcwCorner.ywBottom)
			{
			DrawBrc (ww,&rgbrc[ibrcBottomButt],rcwCorner,dbmodVertical, fFrameLines);
			if (dxwBottomButt > dxwWidth && fEraseBits)
				{
				rcw.xwLeft = rcwCorner.xwLeft + dxwBottomButt;
				rcw.ywTop = rcwCorner.ywTop;
				EraseRc(ww, &rcw);
				rcw.ywBottom = rcw.ywTop;
				rcw.ywTop -= brkCorner.dyHeight;
				rcw.xwLeft = rcwCorner.xwLeft + dxwWidth;
				}
			}
		rcwCorner.xwRight = rcwCorner.xwLeft + dxwWidth;
		rcwCorner.ywBottom = rcwCorner.ywTop;
		rcwCorner.ywTop -= brkCorner.dyHeight;
		break;

	case 0x9: /* 1001 bottom and right */
	case 0xA: /* 1010 bottom and left */
		break;

	case 0xD: /* 1101 bottom, top and right */
	case 0xC: /* 1100 bottom and top */
	case 0xE: /* 1110 bottom, top and left */
LTopAndBottom:
		if (brkCorner.fShadow && brcCur == brcNil
				&& brkCorner.mpblblp[brkCorner.blMac-1] == blpShadow)
			{
			brkCorner.mpblblp[brkCorner.blMac-1] = blpBlack;
			}
		rgbitMatch = 0xE;
		break;
	case 0xF: /* 1111 bottom, top, left and right */
		if ( brk.blMac == 3 )
			{
			/* FUTURE: this code should be setting the pen pattern */
			/*    This will be important if/when we allow more brc's.
			/*    - code assumes that the middle stripe is white
			/**/
			Assert ( brkCorner.mpblblp[1] == blpWhite );
			if (fEraseBits)
				EraseRc ( ww, &rcwCorner );

			/* draw the upper right and lower left corners */
			dx = brkCorner.mpbldx[0];
			dy = brkCorner.mpbldy[0];
#ifdef MAC
			PenSize ( dx, dy );
			MoveTo ( rcwCorner.xwRight - dx, rcwCorner.ywTop );
			Line ( 0, 0 );
			MoveTo ( rcwCorner.xwLeft, rcwCorner.ywTop + brk.dxWidth - dy );
			LineTo ( rcwCorner.xwLeft, rcwCorner.ywBottom - dy );
#else /* WIN */
			DrawRect( hdc, PrcSet(&rcDraw, 
					rcwCorner.xwRight - dx,
					rcwCorner.ywTop,
					rcwCorner.xwRight,
					rcwCorner.ywTop + dy),
					hbr );
			DrawRect( hdc, PrcSet(&rcDraw,
					rcwCorner.xwLeft,
					rcwCorner.ywTop + brk.dyHeight - dy,
					rcwCorner.xwLeft + dx,
					rcwCorner.ywTop + brk.dyHeight),
					hbr );
#endif /* MAC/WIN */

			/* draw the lower right and upper left corners */
			dx = brkCorner.mpbldx[2];
			dy = brkCorner.mpbldy[2];

#ifdef MAC
			PenSize ( dx, dy );
			MoveTo ( rcwCorner.xwLeft, rcwCorner.ywTop );
			Line ( 0, 0 );
			MoveTo ( rcwCorner.xwRight - dx, rcwCorner.ywTop + brk.dxWidth - dy );
			LineTo ( rcwCorner.xwRight - dx, rcwCorner.ywBottom - dy );
#else /* WIN */
			DrawRect( hdc, PrcSet(&rcDraw, 
					rcwCorner.xwLeft,
					rcwCorner.ywTop,
					rcwCorner.xwLeft + dx,
					rcwCorner.ywTop + dy),
					hbr );
			DrawRect( hdc, PrcSet(&rcDraw,
					rcwCorner.xwRight - dx,
					rcwCorner.ywTop + brk.dyHeight - dy,
					rcwCorner.xwRight,
					rcwCorner.ywTop + brk.dyHeight),
					hbr );
#endif /* MAC/WIN */

			/* let everyone know this chunk is taken care of */
			rcw.xwLeft += brkCorner.dxWidth;
			rgbitMatch = 0x0; /* the corner is drawn */
			}
		else
			goto LTopAndBottom; /* treat line top/bottom match */
		break;
		}

	PenNormal();

	if ( rgbitMatch == 0 )
		goto LDrawBrc;

LDrawCorner:
	rcw.xwLeft = rcwCorner.xwRight;

#ifdef MAC
	if (fHairline)
		{
		/* more hacking for the LaserWriter */
		fRight = fBottom = fFalse;
		fLeft = rgbitMatch & 0x8;
		fTop = rgbitMatch & 0x01;
		}
	else
#endif /* MAC */
			{
			fRight = !(rgbitMatch & 0x1);
			fLeft = !(rgbitMatch & 0x2);
			fTop = !(rgbitMatch & 0x4);
			fBottom = !(rgbitMatch & 0x8);
			}

	Break3();
	for ( bl = brkCorner.blMac; bl-- > 0; )
		{
		dx = brkCorner.mpbldx[bl];
		dy = brkCorner.mpbldy[bl];
		BreakW(6);
		switch ( brkCorner.mpblblp[bl] )
			{
		default:
			Assert(fFalse);
			break;
		case blpShadow:
			/* this means we don't really want to draw the shadow */
			/* so, FALL THROUGH */
		case blpWhite:
			if (fEraseBits)
#ifdef MAC
				PenPat(&vqdg.patWhite);
#else /* WIN */
			hbr = vsci.hbrBkgrnd;
#endif
else
	goto LNextCornerLine;
break;
#ifdef MAC
case blpGrey:
PenPatGray(ww);
break;
case blpBlack:
PenPat(&vqdg.patBlack);
break;
case blpLaser:
if (fLaserWriter)
{
	Assert (!(fRight | fBottom));
	SetHairline();
	if (fTop)
		{
		Assert(rcwCorner.xwLeft < rcwCorner.xwRight);
		MoveTo(rcwCorner.xwLeft,rcwCorner.ywTop);
		LineTo(rcwCorner.xwRight,rcwCorner.ywTop);
		}
	if (fLeft)
		{
		Assert(rcwCorner.ywTop < rcwCorner.ywBottom);
		MoveTo(rcwCorner.xwLeft,rcwCorner.ywTop);
		LineTo(rcwCorner.xwLeft,rcwCorner.ywBottom);
		}
	ResetHairline();
	goto LNextCornerLine;
}


PenPat(&vqdg.patBlack);
break;
#else /* WIN */
case blpGrey:
hbr = vhbrGray;
break;
case blpBlack:
hbr = vfli.fPrint ? vpri.hbrText : vsci.hbrText;
break;
#endif
			}
		Assert(!FEmptyRc(&rcwCorner));
#ifdef MAC
		if (fTop)
			{
			PenSize(1,dz);
			MoveTo(rcwCorner.xwLeft,rcwCorner.ywTop);
			LineTo(rcwCorner.xwRight-1,rcwCorner.ywTop);
			}
		if (fBottom)
			{
			PenSize(1,dz);
			MoveTo(rcwCorner.xwLeft,rcwCorner.ywBottom-dz);
			LineTo(rcwCorner.xwRight-1,rcwCorner.ywBottom-dz);
			}
		if (fLeft)
			{
			PenSize(dz,1);
			MoveTo(rcwCorner.xwLeft,rcwCorner.ywTop);
			LineTo(rcwCorner.xwLeft,rcwCorner.ywBottom-1);
			}
		if (fRight)
			{
			PenSize(dz,1);
			MoveTo(rcwCorner.xwRight-dz,rcwCorner.ywTop);
			LineTo(rcwCorner.xwRight-dz,rcwCorner.ywBottom-1);
			}
#else /* WIN */
		if (fTop)
			DrawRect( hdc, PrcSet(&rcDraw,
					rcwCorner.xwLeft,
					rcwCorner.ywTop,
					rcwCorner.xwRight,
					rcwCorner.ywTop + dy),
					hbr );
		if (fBottom)
			DrawRect( hdc, PrcSet(&rcDraw, 
					rcwCorner.xwLeft,
					rcwCorner.ywBottom - dy,
					rcwCorner.xwRight,
					rcwCorner.ywBottom),
					hbr );
		if (fLeft)
			DrawRect( hdc, PrcSet(&rcDraw,
					rcwCorner.xwLeft,
					rcwCorner.ywTop,
					rcwCorner.xwLeft + dx,
					rcwCorner.ywBottom),
					hbr );
		if (fRight)
			DrawRect( hdc, PrcSet(&rcDraw,
					rcwCorner.xwRight - dx,
					rcwCorner.ywTop,
					rcwCorner.xwRight,
					rcwCorner.ywBottom),
					hbr );
#endif /* MAC/WIN */

LNextCornerLine:
		rcwCorner.xwLeft += (fLeft) ? dx : 0;
		rcwCorner.xwRight -= (fRight) ? dx : 0;
		rcwCorner.ywTop += (fTop) ? dy : 0;
		rcwCorner.ywBottom -= (fBottom) ? dy : 0;
		}

	/* white out any left over space in the corner */
	if (fEraseBits && !FEmptyRc(&rcwCorner))
#ifdef MAC
		FillRect ( &rcwCorner, &vqdg.patWhite );
#else /* WIN */
	FillRect (hdc, (LPRECT) &rcwCorner, vsci.hbrBkgrnd);
#endif

LDrawBrc:
	if (FEmptyRc(&rcw))
		return;	/* nothing left to draw */

	PenNormal();

	if (fVertical)
		{
		yw1 = rcw.ywTop;
		yw2 = WinMac(rcw.ywBottom, rcw.ywBottom - 1);
		/* slice off what we are about to draw */
		rcw.xwLeft = mpblz[brk.blMac];
		}
	else  /* horizontal */		
		{
		xw1 = rcw.xwLeft;
		xw2 = WinMac(rcw.xwRight, rcw.xwRight - 1);
		/* slice off what we are about to draw */
		rcw.ywTop = mpblz[brk.blMac];
		}
	for ( bl = 0; bl < brk.blMac; ++bl )
		{
#ifdef MAC
		/* set up default positions for MoveTo and LineTo */
		if (fVertical)
			{
			xw1 = xw2 = mpblz[bl];
			dxpPen = mpblz[bl+1] - xw1;
			dypPen = 1;
			}
		else
			{
			yw1 = yw2 = mpblz[bl];
			dxpPen = 1;
			dypPen = mpblz[bl+1] - yw1;
			}
		PenSize(dxpPen, dypPen);
#else
		/* set up default bounds for DrawRect */
		if (fVertical)
			{
			xw1 = mpblz[bl];
			xw2 = mpblz[bl+1];
			}
		else  /* dbmodHoriz */			
			{
			yw1 = mpblz[bl];
			yw2 = mpblz[bl+1];
			}
#endif
		BreakW(7);
		switch ( brk.mpblblp[bl] )
			{
		default:
			Assert ( fFalse );
			goto LContinue;
		case blpWhite:
			if (fEraseBits)
#ifdef MAC
				PenPat(&vqdg.patWhite);
#else /* WIN */
			hbr = vsci.hbrBkgrnd;
#endif
else
	goto LContinue;
break;
#ifdef MAC
case blpGrey:
PenPatGray(ww);
break;
case blpBlack:
PenPat(&vqdg.patBlack);
break;
case blpLaser:
if (fLaserWriter)
{
	/* more hacking for the LaserWriter */
	SetHairline();
	if (fVertical)
		{
		MoveTo(xw1,yw1-1);
		LineTo(xw1,yw2+1);
		}
	else
		{
		MoveTo(xw1,yw1);
		LineTo(xw2+1,yw1);
		}
	ResetHairline();
	goto LContinue;
}


PenPat(&vqdg.patBlack);
break;
#else /* WIN */
case blpGrey:
hbr = vhbrGray;
break;
case blpBlack:
hbr = vfli.fPrint ? vpri.hbrText : vsci.hbrText;
break;
#endif /* MAC/WIN */
			}
#ifdef MAC
		if (xw1 <= xw2 && yw1 <= yw2)
			{
			MoveTo(xw1,yw1);
			LineTo(xw2,yw2);
			}
#else /* WIN */
		if (xw1 < xw2 && yw1 < yw2)
			DrawRect(hdc, PrcSet(&rcDraw, xw1, yw1, xw2, yw2), hbr);
#endif
LContinue:
		continue;
		}

	if (fEraseBits)
		{
		if (!FEmptyRc(&rcw)) /* guards against bogus rcw */
			EraseRc (ww, &rcw);
		}
	PenNormal();
}


/* C B R C  M A T C H */
/* Given an array of brc's describing the current border to be drawn
/* and up to three border which join with it, determine which (if any)
/* of the borders match and require a corner connection.
/*
/* Returned: return value is the number of borders which matched the
/* first in the group. (Zero means no matches, one means there was
/* a matching pair, etc.) *rgbitMatch is left containing bits set
/* for the brc's which participated in the match.
/*
/* The Rules: The most common brc with at least two occurances wins. In
/* case of a tie, the wider one wins. In case of a width tie, the first
/* one encountered wins. "brcNone" does not count as a match if there
/* are any non-trivial borders involved in the junction.
/*
/* %%Function:CbrcMatch %%Owner:tomsax %%Reviewed:7/6/89 */
CbrcMatch(rgbrc,fFrameLines,pbrcMatch,prgbitMatch)
int	rgbrc[];
BOOL fFrameLines;
int	*pbrcMatch;
int *prgbitMatch;
{
	int bitStart, bitCur;
	int ibrc, ibrcT, cbrcMatch, cbrcT;
	int rgbitMatch, rgbitT;
	int brcMatch, brcT;
	int fRealBorders;

	bitStart = 1;
	cbrcMatch = 0;
	fRealBorders = fFalse;
	brcMatch = brcNil;
	for ( ibrc = 0; ibrc < 3; ++ibrc, bitStart <<= 1 )
		{
		if ((brcT=rgbrc[ibrc]) == brcNil)
			continue;
		if (brcT == brcNone)
			{
			if (fRealBorders)
				continue;
			}
		else
			{
			fRealBorders = fTrue;
			if (brcMatch == brcNone)
				cbrcMatch = 0;
			}
		cbrcT = 0;
		bitCur = (rgbitT = bitStart) << 1;
		for ( ibrcT = ibrc + 1; ibrcT < 4; ++ibrcT, bitCur <<= 1 )
			if ( rgbrc[ibrcT] == brcT )
				{
				++cbrcT;
				rgbitT |= bitCur;
				}
		if (cbrcT > 0)
			{
			if ( DxFromBrc(brcT,fFrameLines) >= DxFromBrc(brcMatch,fFrameLines) )
				{
				brcMatch = brcT;
				cbrcMatch = cbrcT;
				rgbitMatch = rgbitT;
				if (cbrcMatch > 1)
					break;
				}
			}
		}
	if (brcMatch == brcNone && fRealBorders)
		cbrcMatch = 0;
	else
		{
		*prgbitMatch = rgbitMatch;
		*pbrcMatch = brcMatch;
		}
	return cbrcMatch;
}


typedef struct
	{
	struct PT ptPosition;
	struct PT ptSize;
	int wStyle;
	int wPattern;
	} PATTERNRECT;


/* %%Function:DrawRect %%Owner:tomsax %%Reviewed:7/6/89 */
DrawRect(hdc, prc, hbr)
HDC hdc;
struct RC *prc;
HBRUSH hbr;
{
	if (vlm == lmPreview)
		DrawPrvwLine(hdc, prc->xpLeft, prc->ypTop, prc->xpRight - prc->xpLeft + 1, prc->ypBottom - prc->ypTop + 1,
				 colAuto);
	else
		{
		if (vfli.fPrint && vpri.fDPR && hbr == vpri.hbrText)
			{
			PATTERNRECT patternrect;

			/* A high speed variant of FillRect for HP laserjets */
			patternrect.wStyle = 0;
			patternrect.ptPosition.xp = prc->xpLeft;
			patternrect.ptPosition.yp = prc->ypTop;
			patternrect.ptSize.xp = prc->xpRight - prc->xpLeft;
			patternrect.ptSize.yp = prc->ypBottom - prc->ypTop;
			Escape(hdc, DRAWPATTERNRECT,
				NULL, (LPSTR)&patternrect, NULL);
			}
		else
			FillRect(hdc, (LPRECT) prc, hbr);
		}
}



/* F  M A T C H  B R C */
/* returns true iff para in vpapFetch matches boxwise the para described by
	the params and in vfli.brcl. The adjacent paragraph is assumed to be
	in vpapFetch, but this may not still be the case upon exit.
*/

/* %%Function:FMatchBrc %%Owner:tomsax %%Reviewed:7/6/89 */
EXPORT BOOL FMatchBrc(dxaMarginLeft, dxaMarginRight, cpFirst, fAbove, ppap)
int dxaMarginLeft, dxaMarginRight;
CP cpFirst;  /* cpFirst of current paragraph */
BOOL fAbove; /* if vpapFetch para is above (else below) current vfli's para */
struct PAP *ppap;	/* pap of current paragraph */
{
	int dxaColumnWidth;
	int ch, ipgd;
	CP cpOtherPara;
	BOOL	fConseqMatch;
	struct PLC **hplcpgd;
	struct PGD pgd;
	int fInTable, doc = caPara.doc;

	cpOtherPara = caPara.cpFirst;

/* check for differences in table-ness */
	if ((fInTable = FInTableDocCp(doc, cpFirst)) != FInTableDocCp(doc, cpOtherPara))
		return fFalse;

/* Reset vpapFetch */
	CachePara(doc, cpOtherPara);

/* to count as a match, the paragraph in vpapFetch must have some
/* borders set
/**/
	if (CchNonZeroPrefix(&vpapFetch.brcTop,cbrcParaBorders*sizeof(int)) == 0)
		return fFalse;

/* check whether border codes are different */
	if (vpapFetch.brcTop != vfli.brcTop
			|| vpapFetch.brcBottom != vfli.brcBottom
			|| vpapFetch.brcLeft != vfli.brcLeft
			|| vpapFetch.brcRight != vfli.brcRight
		/* if either paragraph is missing one of the top/bottom borders that the
			other (maybe) has, we DO want to draw the between borders */
	|| vpapFetch.brcBottom == brcNone || vpapFetch.brcTop == brcNone
			|| ppap->brcBottom == brcNone || ppap->brcTop == brcNone)
		{
		return fFalse;
		}

/* if the two para's are both in a table, check that they are in the
/* same cell, and compute the column width for the cell */
	if (fInTable)
		{
		AssertDo(FInTableDocCp(doc, cpFirst));
		/* NOTE CacheTc calls CachePara all over the world */
		CacheTc(wwNil,doc,vcpFirstTablePara,fFalse,fFalse);
		if (!FInCa(doc,cpOtherPara,&vtcc.ca))
			return fFalse;
		dxaColumnWidth = DxaFromDxp(PwwdWw(vfli.ww),vtcc.xpDrRight-vtcc.xpDrLeft);
		CachePara(doc,cpOtherPara); /* restore vpapFetch */
		}
	else
		{
		CacheSect(doc, cpFirst);
		if (!FMatchAbs(doc, ppap, &vpapFetch)
		  	|| (caPara.cpFirst == caPara.cpLim - ccpSect && (fAbove ? caPara.cpLim == caSect.cpFirst : caPara.cpLim == caSect.cpLim))
			|| (fAbove ? ppap->fPageBreakBefore : vpapFetch.fPageBreakBefore)
			|| (ch = ChFetch(doc, fAbove ? cpFirst : caPara.cpFirst, fcmChars)) == chSect Win(|| ch == chColumnBreak))
			{
			return(fFalse);
			}
		dxaColumnWidth = vpapFetch.dxaWidth == 0 ? vsepFetch.dxaColumnWidth
				: vpapFetch.dxaWidth;
		}

/* compare whether the widths are different */
	if (dxaMarginRight != dxaColumnWidth - vpapFetch.dxaRight
			|| dxaMarginLeft != vpapFetch.dxaLeft + min(0, vpapFetch.dxaLeft1))
		return fFalse;

/* for paragraph borders, if a page table exists and page break intervenes, 
   treat the borders as different */
	if (fInTable || (hplcpgd = PdodDoc(caPara.doc)->hplcpgd) == hNil)
		return(fTrue);
	if (!fAbove)
		{
		CachePara(caPara.doc, cpFirst);
		cpFirst = caPara.cpLim;
		}
	ipgd = IInPlc(hplcpgd, cpFirst);
	if (cpFirst != CpPlc(hplcpgd, ipgd))
		return(fTrue);
	GetPlc(hplcpgd, ipgd, &pgd);
	return(pgd.cl != 0);
}


/*  F O R M A T  B O R D E R S
/*	Description: Uses vpapFetch and the supplied cp to fill in the
/*	fields in vfli needed to draw the borders for the current line.
/**/
/* %%Function:FormatBorders %%Owner:tomsax %%Reviewed:7/6/89 */
EXPORT FormatBorders ( doc, cp, cpCurPara, cpFirstPara)
int doc;
CP cp, cpCurPara, cpFirstPara;
{
	int dxa, dxaMarginRight, dxaMarginLeft;
	int brcTop;
	CP cpT, cpFirst, cpLim;
	CP cpFirstDoc, cpMacDoc;
	struct DOD *pdod;
	struct PAP pap;
	int ihdd;
	int ch;

	ch = 0;
	cpMacDoc = CpMacDoc(doc);
	if (cpCurPara == cpFirstPara)
		{
		/* find the first visible CP */
		cpT = cp;
		do 
			{
			FetchCpAndPara(doc, cpT, fcmChars+fcmProps);
			cpT += vccpFetch;
			} 
		while ((vchpFetch.fVanish || vchpFetch.fFldVanish) && cpT < cpMacDoc);
		ch = *vhpchFetch;

		/* restore original pap */
		CachePara(doc,cpCurPara);
		}
	if (ch == chSect || ch == chColumnBreak || vpapFetch.fTtp)
		return;

	pap = vpapFetch;
	Assert(FInCa(doc, cp, &caSect));

	/* calculate left and right margins to be used to match paragraphs */
	if (pap.fInTable)
		dxa = vfli.dxa;
	else
		dxa = pap.dxaWidth == 0 ? vsepFetch.dxaColumnWidth
				: pap.dxaWidth;
	dxaMarginRight = dxa - pap.dxaRight;
	dxaMarginLeft = pap.dxaLeft + min(0, pap.dxaLeft1);

	/* get info on the simple types of borders, which may get changed
	/* by the more complex codes - these must be set now for use
	/* in comparisons.
	/**/
	vfli.fTopEnable = (vfli.brcTop = pap.brcTop) != brcNone;
	vfli.fBottomEnable = (vfli.brcBottom = pap.brcBottom) != brcNone;
	vfli.fLeft = (vfli.brcLeft=pap.brcLeft) != brcNone;
	vfli.fRight = (vfli.brcRight=pap.brcRight) != brcNone;

	/* check for between border, which may override the top or bottom border */
	cpLim = caPara.cpLim;

	pdod = PdodDoc(doc);
	if (pdod->fHdr)
		{
		Mac(Debug(BreakW(1)));
		ihdd = IInPlc(pdod->hplchdd,caPara.cpFirst);
		cpFirstDoc = CpPlc(pdod->hplchdd,ihdd);
		cpMacDoc = CpPlc(pdod->hplchdd,ihdd+1) - ccpEop;
		}
	else
		{
		Mac(Debug(BreakW(2)));
		cpFirstDoc = cp0;
		cpMacDoc = CpMacDoc(doc);
		}
	brcTop = vfli.brcTop;
	if ((cpFirst = caPara.cpFirst) > cpFirstDoc)
		{
		CachePara(doc, caPara.cpFirst-1);
		if (FMatchBrc(dxaMarginLeft, dxaMarginRight, cpFirst,
				fTrue /* fAbove */, &pap))
			{
			vfli.fBetweenTop = vfli.fTopEnable = fTrue;
			/* can't change this value yet, FMatchBrc may yet need it. */
			brcTop = vpapFetch.brcBetween;
			}
		}
	if (cpLim < cpMacDoc)
		{
		CachePara(doc, cpLim);
		if (FMatchBrc(dxaMarginLeft, dxaMarginRight, cpFirst,
				fFalse /* fAbove */, &pap))
			{
			vfli.fBetweenBottom = fTrue;
			vfli.fBottomEnable = fFalse;
			vfli.brcBottom = vpapFetch.brcBetween;
			}
		}
	vfli.brcTop = brcTop;

	/* restore original pap */
	CachePara(doc,cpCurPara);

	/* check for bar border, which may override the left or right border.
	/* Unlike brcBetween, brcBar is looked at only if it is not brcNone.
	/**/
	if (pap.brcBar != brcNone)
		{
		if ((vfli.fPrint || vlm == lmPreview) &&
				FFacingPages(doc) && (vfmtss.pgn & 1))
			{
			vfli.fRight = fTrue;
			vfli.brcRight = pap.brcBar;
			}
		else
			{
			vfli.fLeft = fTrue;
			vfli.brcLeft = pap.brcBar;
			}
		}

	vfli.dypBrcTop = DyFromBrc ( vfli.brcTop, fFalse/*fDrawDrs*/ );
	vfli.dypBrcBottom = DyFromBrc ( vfli.brcBottom, fFalse/*fDrawDrs*/ );

	/* recalculate dxaMarginRight using vfli info */
	dxaMarginRight = vfli.dxa - vpapFetch.dxaRight;
	vfli.xpMarginRight = NMultDiv(dxaMarginRight, WinMac(vfti.dxpInch,vfli.dxsInch), dxaInch);
	vfli.xpMarginLeft = NMultDiv(dxaMarginLeft, WinMac(vfti.dxpInch,vfli.dxsInch), dxaInch);
}


#ifdef WIN
/* A D J U S T  O P A Q U E  R E C T */
/* When we have borders, the rcOpaque in DisplayFli includes the area where
	the borders will be drawn by DrawBorders.  If they are already there, we
	don't want them to flash (DrawBorders will take care of clearing any
	that are brcNone), so we reduce the rcOpaque so that it doesn't blow
	away the borders in the call to ExtTextOut, and we clear any space that
	is outside the borders that was included in rcOpaque.
*/

/* %%Function:AdjustOpaqueRect %%Owner:tomsax %%Reviewed:7/6/89 */
EXPORT AdjustOpaqueRect(prcOpaque, prcAdjusted, hdc, dxpToXw, ywLine)
struct RC *prcOpaque, *prcAdjusted;
HDC hdc;
int dxpToXw, ywLine;
{
	int ywSav, dypBrcTop, dypBrcBottom;
	BOOL fAdjustLeft = fFalse;
	BOOL fAdjustRight = fFalse;
	struct RC rcT, rcErase;

	*prcAdjusted = *prcOpaque;
	if (vfli.xpRight > vfli.xpMarginRight)
		return; /* we decline to adjust things in this case */
	Assert(vfli.xpLeft >= vfli.xpMarginLeft); /* no need to check mirror condition */

	dypBrcTop = vfli.fTop ? vfli.dypBrcTop : 0;
	dypBrcBottom = vfli.fBottom ? vfli.dypBrcBottom : 0;

	/* clear space above */
	rcT.xwLeft = prcOpaque->xwLeft,
			rcT.ywTop = ywLine - vfli.dypLine, 
			rcT.xwRight = prcOpaque->xwRight, 
			rcT.ywBottom = rcT.ywTop + vfli.dypBefore - dypBrcTop;
	if (FSectRc(&rcT, prcOpaque, &rcErase))
		PatBltRc(hdc, &rcErase, vsci.ropErase);

	/* clear space below */
	ywSav = rcT.ywBottom;
	rcT.ywTop = (rcT.ywBottom = ywLine) - vfli.dypAfter + dypBrcBottom;
	if (FSectRc(&rcT, prcOpaque, &rcErase))
		PatBltRc(hdc, &rcErase, vsci.ropErase);

	rcT.ywBottom = rcT.ywTop;
	rcT.ywTop = ywSav;
	if (rcT.ywTop >= rcT.ywBottom)
		return;

	/* clear space to left */
	if (prcOpaque->xwLeft < vfli.xpMarginLeft + dxpToXw)
		{
		fAdjustLeft = fTrue;
		rcT.xpLeft = prcOpaque->xwLeft;
		rcT.xpRight = vfli.xpMarginLeft - dxpLeftRightSpace -
				DxFromBrc(vfli.brcLeft, fFalse) + dxpToXw;
		PatBltRc(hdc, &rcT, vsci.ropErase);
		}

	/* clear space to right */
	if (prcOpaque->xwRight > vfli.xpMarginRight + dxpToXw)
		{
		fAdjustRight = fTrue;
		rcT.xpLeft = vfli.xpMarginRight + dxpLeftRightSpace +
				DxFromBrc(vfli.brcRight, fFalse) + dxpToXw;
		rcT.xpRight = prcOpaque->xwRight;
		PatBltRc(hdc, &rcT, vsci.ropErase);
		}

	/* adjust to not include borders */
	rcT.ywTop = ywLine - vfli.dypLine
			+ (vfli.fBetweenTop ? dypBrcTop : vfli.dypBefore);
	rcT.ywBottom = ywLine;
	if (vfli.fBottom)
		rcT.ywBottom -= vfli.fBetweenBottom
				? dypBrcBottom : vfli.dypAfter;
	prcAdjusted->ywTop = max(prcAdjusted->ywTop,rcT.ywTop);
	prcAdjusted->ywBottom = min(prcAdjusted->ywBottom,rcT.ywBottom);
	if (fAdjustLeft)
		prcAdjusted->xwLeft = vfli.xpMarginLeft - dxpLeftRightSpace + dxpToXw;
	if (fAdjustRight)
		prcAdjusted->xwRight = vfli.xpMarginRight + dxpLeftRightSpace + dxpToXw;
}


#endif /* WIN */
