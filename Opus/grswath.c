/*------------------------------------------------------------------------
*
*
* Microsoft Word -- Graphics
*
* Swathing code
*
*-----------------------------------------------------------------------*/

#ifdef PCWORD

#include "word.h"
#include "read.h"
#include "gr.h"
#include "grconsts.h"
#include "grstruct.h"
#include "grprocs.h"
#include "layoutpc.h"
#include "font.h"

AssertData
#define cbitByte	8				/* number of bits in a byte */

extern struct PRDD vprdd;
extern int		vcyInch;
extern int		vlm;
extern BOOL		vfLandscape;
extern BYTE		mpapaapa[];
#endif /* PCWORD */

#ifdef WIN

#include "word.h"
DEBUGASSERTSZ		 /* WIN - bogus macro for assert string */
#include "ff.h" 
#include "grstruct.h"
#include "grtiff.h"
#include "pic.h"
#include "heap.h"
#include "debug.h"
#include "props.h"
#include "format.h"

#define cbitWord	(16)		     /* number of bits  in a word */
#define cbyteWord	(sizeof (WORD))      /* number of bytes in a word */

extern struct FLI       vfli;
#endif /* WIN */




/*
* Miscellaneous notes:
*
* cbPlane:
*
*	For metafiles, the color planes *must* be adjacent in memory, with
* 	no gaps.  This is because the memory blt routines (BitBltMem, etc.)
* 	compute the size of a plane as (irwOutMac - irwOutMic) * cbRowOut.
*	Thus that same computation is used to determine cbPlane.
*/

csconst unsigned rgcoCMYPal[8] = {
	0x7fff,			/* white */
	0x03ff,			/* cyan */
	0x7c1f,			/* magenta */
	0x001f,			/* blue */
	0x7fe0,			/* yellow */
	0x03e0,			/* green */
	0x7c00,			/* red */
	0x0000			/* black */
};


csconst unsigned rgcoRGBPal[8] = {
	0x7fff,			/* white */
	0x7c00,			/* red */
	0x03e0,			/* green */
	0x7fe0,			/* yellow */
	0x001f,			/* blue */
	0x7c1f,			/* magenta */
	0x03ff,			/* cyan */
	0x0000			/* black */
};


/* The domain of this array is pattern numbers for B&W patterning. */
#define icoPatPalMax 16
csconst unsigned rgcoPatPal[icoPatPalMax] = {
	/* color		luminosity = max+min of components */
	0x7fff,			/* white 			62 */
	0x7fe0,			/* yellow 			31 */
	0x03ff,			/* cyan 			31 */
	0x7c1f,			/* magenta 			31 */
	0x001f,			/* blue 			31 */
	0x03e0,			/* green 			31 */
	0x7c00,			/* red 				31 */
	0x3def,			/* gray 			31 */
	0x3de0,			/* dark yellow		15 */
	0x01ef,			/* dark cyan 		15 */
	0x3c0f,			/* dark magenta 	15 */
	0x000f,			/* dark blue 		15 */
	0x01e0,			/* dark green 		15 */
	0x3c00,			/* dark red 		15 */
	0x1ce7,			/* dark gray 		14 */
	0x0000			/* black 			 0 */
};


/* Mapping from color numbers (indices of rgcoPatPal) to densities. */
/* Uses an almost linear scale. */
csconst unsigned mpicodns[icoPatPalMax] = {
	0,  150,  178,  207,  239,  273,  311,  351,
			396,  447,  503,  569,  646,  739,  858, 2000
};


/* Dither matrix for Bayer patterns */
/* The dither matrix and density arrays */
/* are created by bayer.c in hctools\graphics */
/* Ugly patterns have been removed. */

#define patBayerMac	33

csconst BYTE mpxyBayerPat[rowPatMax][cbitRowPat] = {
	0, 16,  4, 20,  2, 18,  4, 20,
			24,  8, 28, 12, 26, 10, 28, 12,
			5, 21,  3, 19,  6, 22,  3, 19,
			29, 13, 27, 11, 30, 14, 27, 11,
			2, 18,  4, 20,  1, 17,  4, 20,
			26, 10, 28, 12, 25,  9, 28, 12,
			7, 23,  3, 19,  5, 21,  3, 19,
			31, 15, 27, 11, 29, 13, 27, 11
};


csconst int mppatdnsBayer75[patBayerMac] = {
	140, 150, 150, 170, 230, 300, 310, 360, 
			340, 400, 400, 430, 480, 600, 660, 670, 
			670, 690, 690, 700, 750, 760, 790, 790, 
			830, 830, 840, 900, 1000, 1100, 1250, 1300, 
			1400
};


csconst int mppatdnsBayer150[patBayerMac] = {
	140, 150, 160, 180, 280, 400, 500, 530, 
			540, 580, 600, 630, 700, 840, 980, 1000, 
			1020, 1020, 1030, 1040, 1050, 1050, 1050, 1050, 
			1050, 1050, 1070, 1080, 1150, 1230, 1270, 1330, 
			1400
};


/* Dither matrix for coarse fatting */

#define patFattingMac	33
csconst BYTE mpxyFattingPat[rowPatMax][cbitRowPat] = {
	{ 15,  4,  5,  6, 19, 20, 21, 22 },
	{ 14,  0,  1,  7, 18, 29, 30, 23 },
	{ 13,  3,  2,  8, 17, 28, 31, 24 },
	{ 12, 11, 10,  9, 16, 27, 26, 25 },
	{ 19, 20, 21, 22, 15,  4,  5,  6 },
	{ 18, 29, 30, 23, 14,  0,  1,  7 },
	{ 17, 28, 31, 24, 13,  3,  2,  8 },
	{ 16, 27, 26, 25, 12, 11, 10,  9 }
	};


#ifdef LINEAR_FATTING	/* reflectance increases linearly */
csconst unsigned mppatdnsFatting300[patFattingMac] = {
	130,   143,   157,   171,   185,   200,   215,   231,
			247,   265,   282,   301,   321,   341,   362,   385,
			408,   434,   460,   488,   519,   551,   587,   625,
			667,   714,   766,   825,   894,   976,  1077,  1208,
			1397
};


#endif

/*
* Raw values as measured with a densitometer.
* The measuring technique: I used pattern.c (from hctools\graphics)
* to print all the patterns on an HP Laserjet II with new toner.
* 12 patterns (4 rows, 3 cols) fit on each page.  The Laserjet
* doesn't deposit ink uniformly on the page; the top row is lighter
* than the bottom row and the right column is lighter than the other two.
* Note that in the tables, rows and columns appear transformed;
* that is, the first row in the table corresponds to the left column
* on the printout.
*/

#ifdef RAW_FATTING300	/* NEVER use the raw data */
csconst unsigned mppatdnsFatting300[patFattingMac] = {
	70,		160,	220,	260,
			290,	320,	370,	400,
			400,	430,	480,	500,

			560,	610,	640,	650,
			670,	720,	770,	790,
			700,	740,	790,	800,

			840,	950,	1000,	1040,
			1080,	1210,	1300,	1360,
			1370
};


#endif

csconst unsigned mppatdnsFatting300[patFattingMac] = {
	150,	160,	220,	260,
			290,	320,	370,	400,
			420,	460,	500,	520,

			560,	610,	640,	650,
			670,	720,	770,	780,
			790,	810,	830,	850,

			900,	950,	1000,	1040,
			1080,	1210,	1300,	1360,
			1400
};


#define twBayerMin	7		/* use Bayer if pixel is > 7 twips high or wide */
/* ...i.e. resolution is < 200 dpi */
#define twBayer75Min 14		/* use Bayer75 if pixel is > 14 tw high or wide */
/* ...i.e. resolution is < 100 dpi */


#ifdef PCWORD

/*  %%Function:  PrecalcPrintParms  %%Owner:  bobz       */

PrecalcPrintParms(hpc)
PICT	**hpc;
/*
	* Compute everything about a picture that only needs to be computed once.
	* This includes cbRowMin, cbSwathMin, cgraphicspaces, cSpace, crwSwath,
	* cgraphicbytes, irwOutMac, irwOutNext, and the bitmap import parameters.
	* This routine is only called when we are printing.
	*/
{
	PICT	*ppc;
	int		grf;
	int	    cSpace;

	AssertHeap(hpc);
	Assert(vlm == lmPrint);

	ppc = *hpc;
	ppc->fLSRotate = vfLandscape
			&& !FVectorDev()
			&& !(hpvdvCur->rpr.grf & fHeadRotates);
	Assert(!ppc->fLSRotate || (!ras.fvertdev && FAbsPos(hpvdvPrn)));

	ppc->irwOutMac = ppc->irwOutNext = 0;

	if (ppc->ft & ftfBitmap)
		{
		PrecalcBitParms(hpc);
		ppc = *hpc;
		}
	else
		{
		Assert(hpvdvCur != hpNil && hpvdvCur == hpvdvPrn);
		ppc->cbExtra = 0;
		grf = hpvdvPrn->rpr.grf;
		if (FVectorDev())
			grf = fHeadRotates;
		ppc->cplOut = grf & fColorDev ? 3 : 1;
		Assert(ppc->cplOut == hpvdvCur->ccopln);
		if (grf & fBlackRibbon)
			++ppc->cplOut;
		if (ppc->fLSRotate)
			{
			Assert(!ras.fvertdev);
			ppc->cbRowOut = (ppc->pixExt.v + 7) / 8;
			ppc->crwSwath = 8;	/* swath is one byte deep */
			ppc->cbRowMin = ppc->cbSwathMin = ppc->pixExt.v;
			ppc->cbPassOut = ppc->cbRowOut * ppc->crwSwath;
			ppc->irwOutMax = ppc->pixExt.h;
			/* draw in reverse */
			ppc->irwOutMic = ppc->irwOutMax;
			ppc->irwOutNext = ppc->irwOutMax - 1;
			}
		else
			{
			ppc->cbRowOut = (ppc->pixExt.h + 7) / 8;
			ppc->cbRowMin = ppc->cbRowOut * ppc->cplOut;
			ppc->crwSwath = 1;
			if (ras.fvertdev)
				ppc->crwSwath = hpvdvCur->rpr.cbit;
			if (ras.finterlace)
				ppc->crwSwath <<= 1;
			ppc->cbPassOut = ppc->cbRowOut * ppc->crwSwath;
			ppc->cbSwathMin = ppc->cbRowMin * ppc->crwSwath;
			ppc->irwOutMax = ppc->pixExt.v;
			}
		}
	if (ras.fvertdev && !ras.fdiablo)
		ppc->cgraphicbytes = ppc->pixExt.h;
	else
		ppc->cgraphicbytes = ppc->cbRowOut;
	ppc->iPass = 0;
}


#endif /* PCWORD */



unsigned RowPat();

/*  %%Function:  PrecalcBitParms  %%Owner:  bobz       */

PrecalcBitParms(hpc)
PICT	**hpc;
/*
	* Compute the swathing and color translation parameters for a bitmap.
	* Determine the type of stretch-blt that will be performed.
	* Perform all other calculations and initialization that can
	* be done at the start.
	*/
{
	unsigned	icoDevMac;		/* number of colors, output device */
	PICT		*ppc;
	unsigned	smh,
	smv;
	unsigned	cbPassIn,
	crwPassIn,
	cpl;
	long		lT;
	int			wMpi,			/* printer units per inch */
	wDpi,			/* pixels per inch */
	wGcf;			/* greatest common factor of wMpi, wDpi */
	CO			rgcoSrc[icoBitMax]; /* these are the source image colors */


	Assert((*hpc)->ft & ftfBitmap);

	(*hpc)->fGrayDither = fFalse;

	switch ((*hpc)->ft)		/* determine the color mapping */
		{
	case ftPCPaint:
	case ftCapture:
		GetInfoPCX(hpc, rgcoSrc);
		break;
	case ftClipboard:
		GetInfoWin(hpc, rgcoSrc);
		break;
	case ftTIFF:
		GetInfoTIFF(hpc, rgcoSrc);
		break;
	default:
		Assert(fFalse);
		}


#ifdef PCWORD
	Assert(hpvdvPrn != hpNil);
#endif /* PCWORD */

	MapColors(hpc, rgcoSrc);	/* may trash rgcoSrc! */

	ppc = *hpc;
	Assert(ppc->cplIn <= iplMax);


#ifdef PCWORD
	ppc->cbRowIn = (ppc->vtExt.h + cbitByte - 1) / cbitByte;
#endif /* PCWORD */

#ifdef WIN
	/* for Windows, must be word aligned */
	ppc->cbRowIn = ((ppc->vtExt.h + cbitWord - 1) / cbitWord) * cbyteWord;
#endif /* WIN */

	ppc->crwSwath = 1;

#ifdef PCWORD
	if (hpvdvCur->fingd & fRasterDev)
		{
		Assert(vlm == lmPrint);
		if (ras.fvertdev)
			{
			ppc->crwSwath = hpvdvCur->rpr.cbit;
			if (ras.finterlace)
				ppc->crwSwath <<= 1;
			}
		else  if (ppc->fLSRotate)
			{ /* this is the horrible case of landscape bitmaps & !fvertdev */
			wMpi = vcyInch / vprdd.dypMin;
			wDpi = MultDiv(hpvdvCur->dyDev, 1440, hpvdvCur->dimV);
			wGcf = GcfFromW2(wDpi, wMpi);	/* 1/wGcf = min swath height in inches */
			ppc->crwSwath = wDpi / wGcf;
			}
		}
#endif /* PCWORD */



#ifdef PCWORD
	ppc->cbRowOut = (ppc->pixExt.h + cbitByte - 1) / cbitByte;
#endif /* PCWORD */

#ifdef WIN
	/* for Windows, must be word aligned */
	ppc->cbRowOut = ((ppc->pixExt.h + cbitWord - 1) / cbitWord) * cbyteWord;
#endif /* WIN */

	ppc->cbPassOut = ppc->cbRowOut * ppc->crwSwath;
	ppc->irwInMax = ppc->vtExt.v;
	ppc->irwOutMax = ppc->pixExt.v;
	ppc->irwInMac = ppc->irwOutMac = 0;

	Assert(ppc->cbRowIn != 0 && ppc->cbRowOut != 0);

	smh = ppc->vtExt.h > ppc->pixExt.h ? smhCompress
			: (ppc->vtExt.h == ppc->pixExt.h ? smhSame : smhExpand);
	smv = ppc->irwInMax > ppc->irwOutMax ? smvCompress
			: (ppc->irwInMax == ppc->irwOutMax ? smvSame : smvExpand);
	ppc->sm = smh + smv;

	SetupStretch(ppc);

	cpl = max(ppc->cplIn, ppc->cplOut);


#ifdef PCWORD
	if (vlm == lmPreview)
#endif /* PCWORD */
		{
		ppc->cbSwathMin	= max(ppc->cbRowIn, ppc->cbPassOut) * cpl;
		}
#ifdef PCWORD
	else

		{
		lT = (long)(int)ppc->crwSwath * (long)(int)ppc->irwInMax;
		crwPassIn = (lT + ppc->irwOutMax - 1) / (int)ppc->irwOutMax;
		Assert((int)crwPassIn > 0);
		cbPassIn = crwPassIn * ppc->cbRowIn;
		ppc->cbSwathMin = cpl * (cbPassIn > ppc->cbPassOut
				? cbPassIn
				: ppc->cbPassOut);
		ppc->cbRowMin = cpl * max(ppc->cbRowIn, ppc->cbRowOut);
		}
#endif /* PCWORD */

	/* padding for StretchBlt -- 1 byte per plane */
	ppc->cbExtra = cpl * cbExtraPerPlane;
}


/*  %%Function:  MapColors  %%Owner:  bobz       */

MapColors(hpc, rgcoSrc)
PICT	**hpc;
CO		rgcoSrc[];
{
	PICT		*ppc;
	int			ico,
	cco,
	icoMap,
	pat,				/* pattern number */
	patMac,
	row,
	icoDevMac;			/* number of available colors */
	BYTE		b;
	int			dns;				/* current density */
	BOOL		fBlack;				/* true if color & black ribbon */
	BYTE FAR	*lpbPat,			/* points to generated patterns */
	FAR	*lpb;
	TWP			twpPixel;			/* size of a pixel in twips */
	CO			rgcoAvail[icoBitMax]; 	/* the available output colors */
	BYTE		mpxyPat[cbitRowPat][rowPatMax]; /* the dither matrix */
	int			mppatdns[patMax];	/* density of available patterns */


	ppc = *hpc;

	/* Determine available colors (rgcoAvail, icoDevMac), */
	/* number of output planes (cplOut), and fColorDither. */

	ppc->fColorDither = fFalse;


#ifdef WIN
	icoDevMac = 2;
	rgcoAvail[0] = coBlack;
	rgcoAvail[1] = coWhiteGR;
	ppc->cplOut = 1;
	fBlack = fFalse;
#endif /* WIN */

#ifdef PCWORD
	if (hpvdvCur->ccopln == 1 || hpvdvPrn->ccopln == 1
			|| ppc->fGrayDither || ppc->cplIn == 1)
		{
		if (ppc->cplIn == 1)
			{
			icoDevMac = 2;
			rgcoAvail[0] = coWhiteGR;
			rgcoAvail[1] = coBlack;

			}
		else  if (!ppc->fGrayDither)	/* translate colors to patterns */
			{
			bltbx((char far *)rgcoPatPal, (char far *)rgcoAvail,
					sizeof(rgcoPatPal));
			icoDevMac = icoPatPalMax;
			ppc->fColorDither = fTrue;
			}
		ppc->cplOut = 1;
		}
	else
		{
		ppc->cplOut = hpvdvCur->ccopln;	/* FIX: preview only printer colors */
		icoDevMac = hpvdvCur->icoAvailMac;
		if (vlm == lmPreview)
			{
			Assert(hpvdvCur->icoAvailMac <= icoBitMax);
			bltbh(hpvdvCur->rgcoAvail,
					HpOfSbIb(sbDds, rgcoAvail),
					icoBitMax * sizeof(CO));
			}
		else
			{
			Assert((hpvdvCur->fingd & fRasterDev) && (hpvdvCur->rpr.grf & fColorDev));
			bltbx(ras.fnegdev ? rgcoRGBPal : rgcoCMYPal,
					rgcoAvail,
					sizeof(rgcoRGBPal));
			}
		}


	fBlack = (hpvdvCur->fingd & fRasterDev) && (hpvdvCur->rpr.grf & fBlackRibbon);
	if (fBlack && ppc->cplOut == 3)
		{
		Assert(hpvdvCur->rpr.grf & fColorDev);
		ppc->cplOut++;
		}
#endif /* PCWORD */

	/* Map source image colors to available colors. */

	cco = 1 << ppc->cplIn;
	if (!ppc->fGrayDither)
		{
		for (ico = 0; ico < cco; ++ico)
			{
			icoMap = IcoFromCoCcoRgco(rgcoSrc[ico], icoDevMac, rgcoAvail);
			if (fBlack && icoMap == 7)
				icoMap = 8;					/* use black ribbon */
			ppc->rgico[ico] = icoMap;
			}
		}


	/* If dithering, determine dither patterns. */

	if (ppc->fColorDither || ppc->fGrayDither)
		{
		/* FIX -- this may not be a good way of determining
		/* which method to use.  Maybe we should check */
		/* the resolution of the input image; */
		/* if the input resolution is < output resolution, */
		/* use fatting; else use Bayer. */
		twpPixel.h = twpPixel.v = 1;
		ScaleToTwp(&twpPixel);
		if (twpPixel.h > twBayerMin || twpPixel.v > twBayerMin)
			BayerInit(mpxyPat, mppatdns, &patMac, &twpPixel);
		else
			FattingInit(mpxyPat, mppatdns, &patMac);

		if (ppc->fColorDither)		/* map master colors to densities */
			{
			for (ico = 0; ico < cco; ++ico)
				{
				rgcoSrc[ico] = mpicodns[ppc->rgico[ico]];
				}
			}


		/* Map densities to pattern numbers */

		Assert(ppc->sbPat != sbNil);
		lpbPat = LpLockHp(HpOfSbIb(ppc->sbPat, 0));
		for (ico = 0; ico < cco; ++ico)
			{
			dns = rgcoSrc[ico];
			for (pat = 1; pat < patMac; ++pat)	/* find density match */
				{
				if (dns <= mppatdns[pat])
					{
					if (dns - mppatdns[pat - 1] < mppatdns[pat] - dns)
						--pat;
					break;
					}
				}
			if (pat == patMac)
				pat = patMac - 1;
			lpb = &lpbPat[ico];
			for (row = 0; row < rowPatMax; ++row)
				{

				b = (BYTE) RowPat(mpxyPat, pat, row);
				*lpb = (*(unsigned *)&rgcoAvail[0] == coBlack) ? ~b : b;
				lpb += cbRowPat;
				}
			}
		UnlockHp(HpOfSbIb(ppc->sbPat, 0));
		}
}


unsigned

/*  %%Function:  RowPat  %%Owner:  bobz       */

RowPat(mpxyPat, intensity, y)
BYTE	mpxyPat[cbitRowPat][cbitRowPat];	/* dither matrix */
int		intensity;					/* pattern number to use */
int		y;							/* row to compute */
	/*
	* Compute and return a row in a dither pattern.
	*/
{
	unsigned w;
	int		x;


	w = 0;

	for (x = 0; x < cbitRowPat; ++x)
		w = (w << 1) | (mpxyPat[y][x] < intensity ? 1 : 0);

	return w;
}



/*  %%Function:  BayerInit  %%Owner:  bobz       */

BayerInit(mpxyPat, mppatdns, ppatMac, ptwpPixel)
BYTE		mpxyPat[cbitRowPat][cbitRowPat];
unsigned	mppatdns[patMax];	/* density of available patterns */
int			*ppatMac;			/* stuff number of patterns here */
TWP			*ptwpPixel;			/* pixel size on output device */
	/*
	* Set up mpxyPat with the Bayer pattern numbers; fill mppatdns
	* with the patterns' densities; return the number of patterns in *ppatMac.
	*/
{
	bltbx(mpxyBayerPat, mpxyPat, sizeof(mpxyBayerPat));
	if (ptwpPixel->h > twBayer75Min || ptwpPixel->v > twBayer75Min)
		bltbx(mppatdnsBayer75, mppatdns, sizeof(mppatdnsBayer75));
	else
		bltbx(mppatdnsBayer150, mppatdns, sizeof(mppatdnsBayer150));
	*ppatMac = patBayerMac;
}



/*  %%Function:  FattingInit  %%Owner:  bobz       */

FattingInit(mpxyPat, mppatdns, ppatMac)
BYTE		mpxyPat[cbitRowPat][cbitRowPat];
unsigned	mppatdns[patMax];	/* density of available patterns */
int			*ppatMac;			/* stuff number of patterns here */
/*
* Set up mpxyPat with the coarse fatting pattern numbers;
* fill mppatdns with the patterns' densities; return
* the number of patterns in *ppatMac.
*/
{
	bltbx(mpxyFattingPat, mpxyPat, sizeof(mpxyFattingPat));
	bltbx(mppatdnsFatting300, mppatdns, sizeof(mppatdnsFatting300));
	*ppatMac = patFattingMac;
}





#ifdef PCWORD

/*  %%Function:  PrepMeta  %%Owner:  bobz       */

PrepMeta(hpc)
PICT	**hpc;
/*
* Determine the rows that will be in the next swath of a metafile.
* This means that irwOutMic, irwOutMac, and cbPlane are computed.
* Also prepare the swath for drawing (clear it).
*/
{
	PICT		*ppc;
	unsigned	uT;
	unsigned	crw;

	AssertHeap(hpc);
	ppc = *hpc;

	ppc->irwOutMic = ppc->irwOutNext;
	if (ppc->cbBits < ppc->cbSwathMin)
		{
		Assert(!ppc->fLSRotate);
		crw = UDiv(ppc->cbBits, ppc->cbRowMin, &uT);
		}
	else
		{
		crw = UDiv(ppc->cbBits, ppc->cbSwathMin, &uT) * ppc->crwSwath;
		}
	if (ppc->fLSRotate && !(ppc->ft & ftfBitmap))
		{	/* go backwards, from irwOutMax to 0 */
		ppc->irwOutMac = ppc->irwOutNext + 1;
		ppc->irwOutMic = ppc->irwOutMac - crw;
		if (((int)ppc->irwOutMic) < 0)
			ppc->irwOutMic = 0;
		}
	else
		{
		ppc->irwOutMic = ppc->irwOutNext;
		ppc->irwOutMac = ppc->irwOutMic + crw;
		if (ppc->irwOutMac > ppc->irwOutMax)
			ppc->irwOutMac = ppc->irwOutMax;
		}
	if (ppc->fLSRotate)
		ppc->cbPlane = ((ppc->irwOutMac - ppc->irwOutMic + 7) / 8)
				* ppc->pixExt.v;
	else
		ppc->cbPlane = (ppc->irwOutMac - ppc->irwOutMic) * ppc->cbRowOut;
	ClearPrintMem(ppc);
}


#endif /* PCWORD */


/*  %%Function:  FPrepBitmap  %%Owner:  bobz       */

FPrepBitmap(hpc)
PICT	**hpc;
/*
	* Purpose:	Prepare for the next swath of the bitmap.
	*			Compute irwInMic, irwOutMic, irwInMac, irwOutMac, ibIn,
	*			and cbPlane for this picture.
	* Returns:	fFalse if we're done; else fTrue.
	* 
	* Method:	Compute the first and last input and output rows
	* 			of the next swath.  The first output row is simply
	* 			the row following the last output row of the
	* 			previous swath.  The first input row is the row
	* 			which will be copied to that output row.
	* 			We determine the number of output rows that can fit
	* 			in the swath; this gives us the last output row in
	* 			the swath, from which we can compute the last
	* 			input row.  Then we check to see if that many input
	* 			rows will fit in the swath.  If not, we determine the
	* 			number that will fit, recompute the last input row,
	* 			and from that determine the last output row.
	* 			
	* 			These computations are not trivial.  To compute an
	* 			output row, given an input row, is not difficult;
	* 			we simply use the StretchBlt variables computed
	* 			in SetupStretch.  Basically, if we have processed
	* 			n input rows, we have also processed n * crwRepNorm
	* 			+ (n * wRowDelta) / irwInMax output rows.
	* 			(In the routine OutFromIn, we add 1 to the passed
	* 			row number and subtract 1 from the result
	* 			because row numbers are 0-based whereas n is 1-based.)
	* 			
	* 			To compute an input row, given an output row, we
	* 			need to be able to perform the inverse StretchBlt.
	* 			We use the inverse algorithm, given in the header
	* 			of SetupStretch.  This algorithm can be restated as:
	* 			
	* 			
	* 				Initially, wRevRowRem = irwOutMax;
	* 			
	* 				for each output row
	* 					wRevRowRem += wRevRowDelta;
	* 					if (wRevRowRem > irwOutMax)
	* 						wRevRowRem -= irwOutMax;
	* 						repeat(1);
	* 					repeat(crwRevRepNorm);
	* 			
	* 			If we go through this loop n times (process n output rows),
	* 			we will process n * crwRevRepNorm + the upper bound of
	* 			(n * wRevRowDelta) / irwOutMax input rows.  The macro
	* 			InFromOut performs this computation (adjusting for
	* 			the differences between row numbers and counts).
	*
		*			When we're printing, we always prepare for a multiple
	* 			of crwSwath rows, or for fewer than crwSwath rows.
	*			This keeps us from having a few leftover rows at the
	*			end of the plane(s) which either have to be copied down
	*			to the start of the plane(s) and combined with the next
	*			group of swaths, or printed separately, thus wasting
	*			time since not all of the pins on the printhead will
	*			be used.
	*/
{
	PICT		*ppc;
	unsigned	crwIn,
	crwOut,
	irwPrevInMac,
	smh,
	smv,
	uT;
#ifdef OLDSTYLE
	int			crwSkip;
#endif

	ppc = *hpc;
	Assert(ppc->ft & ftfBitmap);
	Assert(ppc->sbBits != sbNil);

	if (ppc->irwOutMac == ppc->irwOutMax)
		{	/* End of picture handled elsewhere for raster devices */

#ifdef PCWORD
		Assert(FVectorDev());
#endif /* PCWORD */

		return(fFalse);
		}

#ifdef PCWORD
	if (vlm == lmPreview
			&& ppc->pixOrg.v + ppc->irwOutMac >= grafPort.rectClip.bottom)
		return(fFalse);
#endif /* PCWORD */

#ifdef WIN  
	if (ppc->pixOrg.v + (int)ppc->irwOutMac > ppc->rcClip.ypBottom)
		return(fFalse);
#endif /* WIN */


	Assert(umod(ppc->cbExtra, max(ppc->cplIn, ppc->cplOut)) == 0);
	ppc->cbPlane = UDiv(ppc->cbBits, max(ppc->cplIn, ppc->cplOut), &uT);

	/* We cast some multiplicands to (int) so we can use SMUL, */
	/* which is much faster than LMUL. */

	ppc->irwOutMic = ppc->irwOutMac;
	ppc->irwInMic = InFromOut(ppc, ppc->irwOutMic);
	crwOut = UDiv(ppc->cbPlane - cbExtraPerPlane, ppc->cbRowOut, &uT);

#ifdef PCWORD
	if (vlm == lmPrint && crwOut > ppc->crwSwath)
		{
		crwOut = crwOut - umod(crwOut, ppc->crwSwath);
		}
#endif /* PCWORD */

	ppc->irwOutMac += crwOut;

	/* It may seem redundant to do this next check, */
	/* but it keeps InFromOut from overflowing. */
	if (ppc->irwOutMac > ppc->irwOutMax)
		ppc->irwOutMac = ppc->irwOutMax;
	ppc->irwInMac = InFromOut(ppc, ppc->irwOutMac-1) + 1;
	if ((long)(int)(ppc->irwInMac - ppc->irwInMic) * (long)(int)ppc->cbRowIn
			> (long)(ppc->cbPlane - cbExtraPerPlane))
		{
		crwIn = UDiv(ppc->cbPlane - cbExtraPerPlane, ppc->cbRowIn, &uT);
		ppc->irwInMac = ppc->irwInMic + crwIn;
		if (ppc->irwInMac > ppc->irwInMax)	/* safety check */
			ppc->irwInMac = ppc->irwInMax;	/* to avoid ovflw in OutFromIn */
		ppc->irwOutMac = OutFromIn(ppc, ppc->irwInMac-1) + 1;
		crwOut = ppc->irwOutMac - ppc->irwOutMic;

#ifdef PCWORD
		if (vlm == lmPrint && crwOut > ppc->crwSwath)
			{
			crwOut = crwOut - umod(crwOut, ppc->crwSwath);
			if (ppc->irwOutMac != ppc->irwOutMic + crwOut)
				{
				ppc->irwOutMac = ppc->irwOutMic + crwOut;
				ppc->irwInMac = InFromOut(ppc, ppc->irwOutMac-1) + 1;
				Assert(ppc->irwInMac - ppc->irwInMic <= crwIn);
				}
			}
#endif /* PCWORD */

		}

	if (ppc->irwInMac > ppc->irwInMax)
		ppc->irwInMac = ppc->irwInMax;
	if (ppc->irwOutMac > ppc->irwOutMax)
		ppc->irwOutMac = ppc->irwOutMax;

	Assert(ppc->irwInMic < ppc->irwInMac);
	Assert(ppc->irwOutMic < ppc->irwOutMac);
	Assert((long)(ppc->irwInMac - ppc->irwInMic) * (long)ppc->cbRowIn
			<= (long)(ppc->cbPlane - cbExtraPerPlane));
	Assert((long)(ppc->irwOutMac - ppc->irwOutMic) * (long)ppc->cbRowOut
			<= (long)(ppc->cbPlane - cbExtraPerPlane));

	smh = ppc->sm & smhMode;
	smv = ppc->sm & smvMode;
	if ((ppc->sm == smhSame + smvSame)
			|| (smh != smhExpand && smv != smvExpand))
		{
		/* compressing or staying the same in both directions
			*
			* The input is offset one byte into the plane memory
			* so that the output (which is smaller), which is placed at
			* the start of the plane, does not overwrite the input.
			*/
		ppc->ibIn = sizeof(BYTE);
		}
	else  if (smv == smvCompress)
		{ /* compress vertically, expand horizontally */
		/*
			* In the first pass, we will do the vertical compression,
			* starting with the last row and running the stretchblt
			* in reverse.  The rows will be copied to the end of the planes.
			* In the second pass, we will do the horizontal stretching.
			* The second pass will proceed from the first row forwards.
			*/
		Assert(smh == smhExpand);
		ppc->ibIn = 0;
		}
	else  if (smh == smhCompress)
		{
		/*
			* First we do the horizontal compression.
			* Then we copy the resulting lines (one by one) to the end
			* of the plane (in order to compact them).  Then we
			* do the vertical expansion.  Again, this would be simplified
			* if we could to the vertical StretchBlt backwards.
			*/
		Assert(smv == smvExpand);
		ppc->ibIn = sizeof(BYTE);
		}
	else  /* expanding or no change in both directions */		
		{
		/*
			* The input is read into the end of the plane memory.
			*/
		Assert((smh == smhExpand && smv != smvCompress)
				|| (smh != smhCompress && smv == smvExpand));
		ppc->ibIn = ppc->cbPlane
				- ((ppc->irwInMac - ppc->irwInMic) * ppc->cbRowIn);
		}

	return(fTrue);
}



/*  %%Function:  InFromOut  %%Owner:  bobz       */

InFromOut(ppc, irwOut)
PICT	*ppc;
unsigned irwOut;
	/*
	* Map output row # to input row #.
	* See explanation in header of FPrepBitmap.
	*/
{
	return(((irwOut + 1) * ppc->crwRevRepNorm)
			+ ((((long)(int)(irwOut + 1) * (long)(ppc->wRevRowDelta))
			+ ppc->irwOutMax - 1) / ((int)ppc->irwOutMax)) - 1);
}




/*  %%Function:  OutFromIn  %%Owner:  bobz       */

OutFromIn(ppc, irwIn)
PICT	*ppc;
unsigned irwIn;
	/*
	* Map input row # to output row #.
	* See explanation in header of FPrepBitmap.
	*/
{
	return(((irwIn + 1) * ppc->crwRepNorm)
			+ (((long)(int)(irwIn + 1) * (long)(ppc->wRowDelta))
			/ (int)(ppc->irwInMax)) - 1);
}





#ifdef PCWORD

/*  %%Function:  SetPrintBounds  %%Owner:  bobz       */

SetPrintBounds(hpc)
PICT	**hpc;
{
	PICT	*ppc;

	AssertHeap(hpc);
	ppc = *hpc;
	Assert(!(ppc->ft & ftfBitmap));
	if (ppc->fLSRotate)
		{
		grafPort.portBits.bounds.left = ppc->pixOrg.h + ppc->irwOutMic;
		grafPort.portBits.bounds.right = ppc->pixOrg.h + ppc->irwOutMac;
		grafPort.portBits.bounds.top = ppc->pixOrg.v;
		grafPort.portBits.bounds.bottom = ppc->pixOrg.v + ppc->pixExt.v;
		grafPort.portBits.rowBytes = (ppc->irwOutMac - ppc->irwOutMic + 7) / 8;
		}
	else
		{
		grafPort.portBits.bounds.left = ppc->pixOrg.h;
		grafPort.portBits.bounds.right = ppc->pixOrg.h + ppc->pixExt.h;
		grafPort.portBits.bounds.top = ppc->irwOutMic + ppc->pixOrg.v;
		grafPort.portBits.bounds.bottom = ppc->irwOutMac + ppc->pixOrg.v;
		grafPort.portBits.rowBytes = ppc->cbRowOut;
		}
	grafPort.rectClip = grafPort.portBits.bounds;
	grafPort.portBits.hpBaseAddr = HpOfSbIb(ppc->sbBits, 0);
}


/*  %%Function:  SetPreviewBounds  %%Owner:  bobz       */

SetPreviewBounds(hpc)
PICT	**hpc;
{
	PICT	*ppc;

	grafPort.portBits.rowBytes = (grafPort.cpixH + 7) >> 3;
	grafPort.portBits.bounds.top = 0;
	grafPort.portBits.bounds.left = 0;
	grafPort.portBits.bounds.bottom = grafPort.cpixV;
	grafPort.portBits.bounds.right = grafPort.cpixH;
	ppc = *hpc;
	grafPort.rectClip.topLeft = ppc->pixOrg;
	grafPort.rectClip.bottom = ppc->pixOrg.v + ppc->pixExt.v;
	if (vlm == lmPreview)
		{
		grafPort.rectClip.bottom
				= min(grafPort.rectClip.bottom, grafPort.pixYMenu);
		}
	grafPort.rectClip.right = ppc->pixOrg.h + ppc->pixExt.h;
}


#endif /* PCWORD */



#ifdef PCWORD

int

/*  %%Function:  YpNextScan  %%Owner:  bobz       */

YpNextScan(hpc)
PICT	**hpc;
/*
	* Advance irwOutNext to point to the first row of the next swath.
	* Return the vertical position at which the next swath will be output,
	* or ypMax if the picture is done.
	*/
{
	PICT		*ppc;
	unsigned	ypNext;

	AssertHeap(hpc);
	ppc = *hpc;
	if (!ras.finterlace || ppc->iPass == 0)
		{
		if (ppc->fLSRotate)					/* special cases: */
			{
			if (ppc->ft & ftfBitmap)			/* We just did a strip */
				ppc->irwOutNext = ppc->irwOutMac; /* ...get next strip */
			else		/* Handle specially because */
				--ppc->irwOutNext;			/* crwSwath = 8 in this case */
			}								/* and we draw rows backwards */
		else
			{
			ppc->irwOutNext += ppc->crwSwath;
			if (ppc->irwOutNext > ppc->irwOutMac)
				ppc->irwOutNext = ppc->irwOutMac;
			}
		if ((ppc->fLSRotate && !(ppc->ft & ftfBitmap))
				? ((int)ppc->irwOutNext < 0)
				: (ppc->irwOutNext == ppc->irwOutMax))
			{
			ypNext = ypMax;
			}
		else
			{
			if (ppc->fLSRotate)
				{
				Assert(!ras.fvertdev);
				ypNext = MultDivR(ppc->irwOutNext, ppc->dxpExt, ppc->pixExt.h)
						+ ppc->xpOrg;
				}
			else
				{
				ypNext = MultDivR(ppc->irwOutNext + ppc->crwSwath,
						ppc->dypExt, ppc->pixExt.v) + ppc->ypOrg;
				if (ras.finterlace)
					{
					Assert(hpvdvCur->rpr.cPixInterlDenom != 0);
					ypNext -= MultDiv(vprdd.dypMin,
							hpvdvCur->rpr.cPixInterlNum,
							hpvdvCur->rpr.cPixInterlDenom);
					}
				}
			}
		}
	else  /* we just did the first interlacing pass */		
		{ /* Handle the case of a swath containing one row. */
		/* In this case, the second interlacing pass would */
		/* be empty, and it might screw up Word */
		/* (e.g. push us off the end of a page) */
		/* so we finish up now. */

		if (ppc->irwOutNext == ppc->irwOutMax - 1)
			{
			ypNext = ypMax;
			}
		else
			{
			ypNext = MultDivR(ppc->irwOutNext + ppc->crwSwath,
					ppc->dypExt, ppc->pixExt.v) + ppc->ypOrg;
			if (ppc->irwOutNext == ppc->irwOutMac - 1)
				{	/* we just did a swath containing one row */
				++ppc->irwOutNext;
				ppc->iPass = 0;		/* do another first pass */
				}
			}
		}

	OutSz(SzShared("YpNextScan: ")); 
	OutInt(hpc, 6);
	OutSz(SzShared(" returning ")); 
	OutInt(ypNext, 6); 
	OutCh('\n');

	return(ypNext);
}


#endif /* PCWORD */


#ifdef PCWORD

int

/*  %%Function:  GcfFromW2  %%Owner:  bobz       */

GcfFromW2(w1, w2)
int		w1, w2;
/*
* Computes the greatest common factor of two integers,
* using Euclid's algorithm.
* Note that the algorithm works regardless of which of w1 and w2
* is greater.
*/
{
	int		wT;

	Assert(w1 > 0 && w2 > 0);

	while (w2 != 0)
		{
		wT = w1 % w2;
		w1 = w2;
		w2 = wT;
		}
	return w1;
}


#endif /* PCWORD */

