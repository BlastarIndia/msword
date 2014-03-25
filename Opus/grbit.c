/*------------------------------------------------------------------------
*
*
* Microsoft Word -- Graphics
*
* Bitmap manipulation
*
* Our motto is: Beware of swathing!
*
*-----------------------------------------------------------------------*/

#ifdef PCWORD

#include "word.h"
#include "gr.h"
#include "grconsts.h"
#include "grstruct.h"
#include "grprocs.h"

AssertData

#endif /* PCWORD */


#ifdef WIN

#include "word.h"
DEBUGASSERTSZ		 /* WIN - bogus macro for assert string */
/* #include "file.h" */
#include "ff.h" 
#include "grstruct.h"
#include "grtiff.h"
#include "pic.h"
#include "heap.h"
#include "debug.h"

#endif /* WIN */


#ifdef SLOW_STRBLT
#define DoStrBltRow(ppc, lpbSrc, lpbDst) \
			SlowStrBltRow(ppc, lpbSrc, lpbDst)
#else
#define DoStrBltRow(ppc, lpbSrc, lpbDst) \
				StrBltRow(ppc, *(char **)&(lpbSrc), *(char **)&(lpbDst))
#endif

/*  %%Function:  TranslateBitmap  %%Owner:  bobz       */


TranslateBitmap(hpc)
PICT	**hpc;
/*
	* Purpose:	StretchBlt the current swath of the bitmap indicated by hpc
	* 			and perform any necessary color translation.
	* Method:	Handle each of the five cases separately.  Some of them
	* 			could be treated as the same case but for speed reasons
	* 			we treat them separately.
	*/
{
	PICT	*ppc;
	int		smh,
	smv;

	ppc = *hpc;
	smh = ppc->sm & smhMode;
	smv = ppc->sm & smvMode;
	if ((smh != smhExpand && smv != smvExpand)
			|| (smh != smhCompress && smv != smvCompress))
		{
		NormTransBitmap(hpc);
		}
	else  if (smv == smvCompress)		/* make it shorter & fatter */
		{
		ShortFatTransBitmap(hpc);
		}
	else  /* smh == smhCompress:			/* make it taller & thinner */		
		{
		TallThinTransBitmap(hpc);
		}
}


/*  %%Function:  SetupStretch  %%Owner:  bobz       */


SetupStretch(ppc)
PICT	*ppc;
/*
	* Purpose:	Set up the stretch-blt variables.
	*			Note that wRowRem is not set here.
	* Method:		The basic algorithm is as follows:
	* 
	* 				crwRepNorm = irwOutMax / irwInMax;
	* 				wRowDelta = irwOutMax % irwInMax;
	* 				
	* 				wRowRem = 0;
	* 
	* 				for each row
	* 					{
	* 					wRowRem += wRowDelta;
	* 					copy the row crwRepNorm times
	* 					if (wRowRem >= irwInMax)
	* 						{
	* 						copy the row one more time;
	* 						wRowRem -= irwInMax;
	* 						}
	* 					}
	* The algorithm for stretching a row horizontally is similar.
	*
		*
		* We also compute a reverse stretch-blt, as if we were going
	* from Out to In.  The algorithm is run in reverse:
	*
		* 				
	* 				crwRevRepNorm = irwInMax / irwOutMax;
	* 				wRevRowDelta = irwInMax % irwOutMax;
	* 				
	* 				wRevRowRem = 0;
	* 				
	* 				for each row
	* 					{
	* 					if (wRevRowRem < wRevRowDelta)
	* 						{
	* 						wRevRowRem += irwOutMax;
	* 						repeat(1);
	* 						}
	* 					repeat crwRevRepNorm times
	* 					wRevRowRem -= wRevRowDelta;
	* 					}
	* The reverse stretchblt is never run, but the variables are
	* used to determine which rows are in the swath.
	*/
{
	ppc->crwRepNorm = UDiv(ppc->irwOutMax, ppc->irwInMax, &ppc->wRowDelta);

	ppc->crwRevRepNorm = UDiv(ppc->irwInMax, ppc->irwOutMax, &ppc->wRevRowDelta);

	ppc->cbitRepNorm = UDiv(ppc->pixExt.h, ppc->vtExt.h, &ppc->wBitDelta);
}


/*  %%Function:  NormTransBitmap  %%Owner:  bobz       */



NormTransBitmap(hpc)
PICT	**hpc;
/*
	* Shrink the bitmap in one or both directions;
	* or expand it in one or both directions
	* (combinations of shrinking and expanding are handled elsewhere).
	* The process is performed in one pass:
	* 	For each source row
	* 		If this maps to a destination row
	*			Stretchblt the source to the destination
	*			For each extra copy of the row that is to be made,
	*				For each plane
	*					Increment the destination pointer
	*					Copy from the first destination to this row
	* 			Increment the pointer to the next destination row
	* 		Increment the pointer to the next source row
	*
		*/
{
	PICT		*ppc;
	int			irwIn,
	crwRep,
	wRowRem,
	ipl,
	crwDst;
	unsigned	irwOut;
	BOOL		fColorToMono;
	char HUGE	*hpbBits,
	HUGE	*hpbPat;
	char FAR	*lpbBits,
	FAR	*lpbSrc,
	FAR	*lpbDst,
	FAR	*lpbDstT,
	FAR	*lpbDstFirst,
	FAR	*lpbDstFirstT;
	WORD		psPat;
	BOOL		fAbort;

	ppc = *hpc;
	FreezeHeap();
	fAbort = fFalse;
	lpbBits = LpLockHp(hpbBits = HpOfSbIb(ppc->sbBits,0));

	Assert((ppc->irwOutMax <= ppc->irwInMax && ppc->pixExt.h <= ppc->vtExt.h)
			|| (ppc->irwOutMax >= ppc->irwInMax && ppc->pixExt.h >= ppc->vtExt.h));

	lpbSrc = lpbBits + ppc->ibIn;
	lpbDst = lpbBits;
	irwOut = ppc->irwOutMic;
	crwDst = ppc->irwOutMac - irwOut;
	Assert(crwDst > 0);
	fColorToMono = ppc->fColorDither || ppc->fGrayDither;
	if (fColorToMono)
		psPat = PsOfLp(LpLockHp(hpbPat = HpOfSbIb(ppc->sbPat, 0)));

	/* compute crwRep and wRowRem for first row */
	wRowRem = ((long)ppc->wRowDelta * ((long)(int)ppc->irwInMic+1))
			% (int)ppc->irwInMax;
	crwRep = OutFromIn(ppc, ppc->irwInMic) - ppc->irwOutMic + 1;
	Assert(crwRep > 0);		/* else why did we read irwInMic? */

	for (irwIn = ppc->irwInMic; irwIn < ppc->irwInMac; ++irwIn)
		{
		if (irwIn != ppc->irwInMic)
			{
			wRowRem += ppc->wRowDelta;
			crwRep = ppc->crwRepNorm;
			if (wRowRem >= ppc->irwInMax)
				{
				++crwRep;
				wRowRem -= ppc->irwInMax;
				}
			}
		/* else we computed crwRep and wRowRem outside the loop */
		if (crwRep > 0)
			{

#ifdef PCWORD
			/* for WIN, this call could mess up the locked huge pointers,
			so we will check abort after the TranslateBitmap call
			*/

			if ((irwIn & 15) == 0 && FAbortRequest())
				{
				fAbort = fTrue;
				goto Done;
				}
#endif /* PCWORD */

			if (fColorToMono)
				{
				do 
					{
					MonoStrBltRow(ppc, lpbSrc, lpbDst, irwOut++, psPat);
					--crwDst;
					lpbDst += ppc->cbRowOut;
					} 
				while (--crwRep > 0 && crwDst > 0);
				}
			else
				{
				DoStrBltRow(ppc, lpbSrc, lpbDst);
				--crwDst;
				lpbDstFirst = lpbDst;
				lpbDst += ppc->cbRowOut;
				while (--crwRep > 0 && crwDst > 0)
					{
					lpbDstT = lpbDst;
					lpbDstFirstT = lpbDstFirst;
					for (ipl = 0; ipl < ppc->cplOut; ++ipl)
						{
						bltbx(lpbDstFirstT, lpbDstT, ppc->cbRowOut);
						lpbDstT += ppc->cbPlane;
						lpbDstFirstT += ppc->cbPlane;
						}
					lpbDst += ppc->cbRowOut;
					--crwDst;
					}
				}
			}
		lpbSrc += ppc->cbRowIn;
		}
	Assert(crwDst == 0);

Done:
	UnlockHp(hpbBits);
	if (fColorToMono)
		UnlockHp(hpbPat);
	MeltHeap();


#ifdef PCWORD
	if (fAbort)
		UserAbort();
#endif /* PCWORD */

}


/*  %%Function:  ShortFatTransBitmap  %%Owner:  bobz       */


ShortFatTransBitmap(hpc)
PICT		**hpc;
/*
	* Compress vertically and expand horizontally a bitmap.
	* This is done in two passes as follows:
	* 
	* Pass 1:
	* Set the source pointer to point to the last row
	* Set the destination pointer to point to the end of the plane - cbRowIn
	* Run the row stretchblt in reverse, copying from source to destination
	* 
	* Pass 2:
	* Set the destination pointer to the start of the plane
	* For each source row (starting with the first)
	* 		Stretchblt the row to the destination
	* 		Increment the source and destination pointers
	*/
{
	PICT		*ppc;
	int			irw,
	wRowRem,
	ipl;
	BOOL		fColorToMono;
	char HUGE	*hpbBits,
	HUGE	*hpbPat;
	char FAR	*lpbBits,
	FAR	*lpbSrc,
	FAR	*lpbDst,
	FAR	*lpbSrcT,
	FAR	*lpbDstT;
	WORD		psPat;
	BOOL		fAbort;
	Debug(int	crwDst);

	ppc = *hpc;
	FreezeHeap();
	fAbort = fFalse;
	lpbBits = LpLockHp(hpbBits = HpOfSbIb(ppc->sbBits,0));

	Assert(ppc->irwOutMax < ppc->irwInMax && ppc->pixExt.h > ppc->vtExt.h
			&& ppc->irwOutMac > ppc->irwOutMic);

	fColorToMono = ppc->fColorDither || ppc->fGrayDither;
	if (fColorToMono)
		psPat = PsOfLp(LpLockHp(hpbPat = HpOfSbIb(ppc->sbPat, 0)));

	/* Pass 1 -- vertical compression */

	lpbSrc = lpbBits + ppc->ibIn
			+ ppc->cbRowIn * (ppc->irwInMac - ppc->irwInMic - 1);
	lpbDst = lpbBits + ppc->cbPlane - ppc->cbRowIn;
	Debug(crwDst = 0);

	/* compute what remainder would be *after* irwInMac-1'th row */
	wRowRem = ((long)ppc->wRowDelta * (long)(int)ppc->irwInMac)
			% (int)ppc->irwInMax;
	for (irw = ppc->irwInMac - 1; irw >= ppc->irwInMic; --irw)
		{
		if (wRowRem < ppc->wRowDelta)
			{
			wRowRem += ppc->irwInMax;
			lpbSrcT = lpbSrc;
			lpbDstT = lpbDst;
			for (ipl = 0; ipl < ppc->cplIn; ++ipl)
				{
				bltbx(lpbSrcT, lpbDstT, ppc->cbRowIn);
				lpbSrcT += ppc->cbPlane;
				lpbDstT += ppc->cbPlane;
				}
			lpbDst -= ppc->cbRowIn;
			Debug(++crwDst);
			}
		lpbSrc -= ppc->cbRowIn;
		wRowRem -= ppc->wRowDelta;
		}
	Debug(Assert(crwDst == ppc->irwOutMac - ppc->irwOutMic));


	/* Pass 2 -- horizontal expansion */

	lpbSrc = lpbDst + ppc->cbRowIn;
	lpbDst = lpbBits;

	for (irw = ppc->irwOutMic; irw < ppc->irwOutMac; ++irw)
		{
#ifdef PCWORD
		/* for WIN, this call could mess up the locked huge pointers,
		so we will check abort after the TranslateBitmap call
		*/

		if ((irw & 31) == 0 && FAbortRequest())
			{
			fAbort = fTrue;
			goto Done;
			}
#endif /* PCWORD */

		if (fColorToMono)
			MonoStrBltRow(ppc, lpbSrc, lpbDst, irw, psPat);
		else
			DoStrBltRow(ppc, lpbSrc, lpbDst);
		lpbSrc += ppc->cbRowIn;
		lpbDst += ppc->cbRowOut;
		}

Done:
	UnlockHp(hpbBits);
	if (fColorToMono)
		UnlockHp(hpbPat);
	MeltHeap();


#ifdef PCWORD
	if (fAbort)
		UserAbort();
#endif /* PCWORD */

}


/*  %%Function:  TallThinTransBitmap  %%Owner:  bobz       */


TallThinTransBitmap(hpc)
PICT		**hpc;
/*
	* Expand vertically and compress horizontally a bitmap.
	* This is done in two passes as follows:
	* 
	* Pass 1:
	* Compress the rows horizontally, copying them so they are at the start
	* of the planes.  Then copy them to the end of the plane, en masse.
	* 
	* Pass 2:
	* If the first input row was the last input row in the last swath,
	* finish mapping from it.  Then for each remaining input row
	* (going forwards), map it to its output rows.  The output rows
	* will start at the beginning of the planes.
	*
		* If we're translating a color bitmap to B&W, we have to modify this
	* somewhat.  In pass 1, when we do the compression (via StrBltRow)
	* don't do any color translation.  Then in pass 2, instead of
	* just copying each compressed source row to the destination rows,
	* perform a color translation on each one, via MonoStrBltRow.
	*
		* The reason we can't translate colors to patterns in the first
	* pass is that the patterning is based on output rows.  If the
	* patterning was done on input rows, the stretching would be
	* very conspicuous.
	*/
{
	PICT		*ppc;
	int			irwIn,
	crw,
	crwDst,
	wRowRem,
	crwRep,
	ipl,
	ico;
	unsigned	irwOut;
	BOOL		fColorToMono;			/* true if color --> B&W */
	unsigned	cbPlane,
	cbRowOut;
	char HUGE	*hpbBits,
	HUGE	*hpbPat;
	char FAR	*lpbBits,
	FAR	*lpbSrc,
	FAR	*lpbDst,
	FAR	*lpbSrcT,
	FAR	*lpbDstT;
	WORD		psPat;
	unsigned	cbIntermediate;
	BYTE		rgicoSav[16];			/* save real values here */
	int			cbitRepNormT,			/* ...while we munge around */
	wBitDeltaT,				/* ...with the PICT, */
	virtExtHT;				/* ...for color --> B&W */
	BOOL		fAbort;

	ppc = *hpc;
	FreezeHeap();
	fAbort = fFalse;
	lpbBits = LpLockHp(hpbBits = HpOfSbIb(ppc->sbBits,0));

	Assert(ppc->irwOutMax > ppc->irwInMax && ppc->pixExt.h < ppc->vtExt.h);

	cbPlane = ppc->cbPlane;
	cbRowOut = ppc->cbRowOut;
	fColorToMono = ppc->fColorDither || ppc->fGrayDither;
	if (fColorToMono)
		psPat = PsOfLp(LpLockHp(hpbPat = HpOfSbIb(ppc->sbPat, 0)));


	/* Pass 1 -- horizontal compression */
	/* if color --> B&W, don't translate colors here */

	lpbSrc = lpbBits + ppc->ibIn;
	lpbDst = lpbBits;

	if (fColorToMono)
		{			/* inhibit color translation; only do compression */
		bltb(ppc->rgico, rgicoSav, 16);
		for (ico = 0; ico < 16; ++ico)
			ppc->rgico[ico] = ico;
		ppc->cplOut = ppc->cplIn;	/* don't save cplOut; we know it's 1 */
		}


	for (irwIn = ppc->irwInMic; irwIn < ppc->irwInMac; ++irwIn)
		{

#ifdef PCWORD
		/* for WIN, this call could mess up the locked huge pointers,
		so we will check abort after the TranslateBitmap call
		*/

		if ((irwIn & 31) == 0 && FAbortRequest())
			{
			fAbort = fTrue;
			goto Done;
			}
#endif /* PCWORD */

		DoStrBltRow(ppc, lpbSrc, lpbDst);
		lpbSrc += ppc->cbRowIn;
		lpbDst += cbRowOut;
		}

	/* Copy to end of plane */

	cbIntermediate = (ppc->irwInMac - ppc->irwInMic) * cbRowOut;
	lpbSrc = lpbBits;
	lpbDst = lpbBits + (cbPlane - cbIntermediate);

	for (ipl = ppc->cplOut; ipl > 0; --ipl)
		{
		bltbx(lpbSrc, lpbDst, cbIntermediate);
		lpbSrc += cbPlane;
		lpbDst += cbPlane;
		}

	/* Pass 2 -- vertical expansion */
	/* if color --> B&W, translate colors in this stage */

	if (fColorToMono)
		{				/* inhibit compression; only translate colors */
		cbitRepNormT = ppc->cbitRepNorm;
		ppc->cbitRepNorm = 1;
		wBitDeltaT = ppc->wBitDelta;
		ppc->wBitDelta = 0;
		virtExtHT = ppc->vtExt.h;
		ppc->vtExt.h = ppc->pixExt.h;
		bltb(rgicoSav, ppc->rgico, 16);		/* restore color mapping */
		ppc->cplOut = 1;
		}

	lpbSrc = lpbBits + (cbPlane - cbIntermediate);
	lpbDst = lpbBits;
	crwDst = ppc->irwOutMac - ppc->irwOutMic;
	Assert(crwDst > 0);

	irwOut = ppc->irwOutMic;

	/* compute crwRep and wRowRem for first row */
	wRowRem = ((long)ppc->wRowDelta * ((long)(int)ppc->irwInMic+1))
			% (int)ppc->irwInMax;
	crwRep = OutFromIn(ppc, ppc->irwInMic) - ppc->irwOutMic + 1;
	Assert(crwRep > 0);		/* else why did we read irwInMic? */


	for (irwIn = ppc->irwInMic; irwIn < ppc->irwInMac; ++irwIn)
		{
		if (irwIn != ppc->irwInMic)
			{
			wRowRem += ppc->wRowDelta;
			crwRep = ppc->crwRepNorm;
			if (wRowRem >= ppc->irwInMax)
				{
				++crwRep;
				wRowRem -= ppc->irwInMax;
				}
			}
		Assert(crwRep > 0);

#ifdef PCWORD
		/* for WIN, this call could mess up the locked huge pointers,
		so we will check abort after the TranslateBitmap call
		*/

		if ((irwIn & 31) == 0 && FAbortRequest())
			{
			fAbort = fTrue;
			goto Done;
			}
#endif /* PCWORD */

		while (crwRep > 0 && crwDst > 0)
			{
			if (fColorToMono)
				MonoStrBltRow(ppc, lpbSrc, lpbDst, irwOut++, psPat);
			else
				{
				lpbSrcT = lpbSrc;
				lpbDstT = lpbDst;
				for (ipl = 0; ipl < ppc->cplOut; ++ipl)
					{
					bltbx(lpbSrcT, lpbDstT, cbRowOut);
					lpbDstT += cbPlane;
					lpbSrcT += cbPlane;
					}
				}
			lpbDst += cbRowOut;
			--crwRep;
			--crwDst;
			}
		lpbSrc += ppc->cbRowOut;
		}
	Assert(crwDst == 0);
	if (fColorToMono)
		{				/* restore stretchblt values */
		ppc->cbitRepNorm = cbitRepNormT;
		ppc->wBitDelta = wBitDeltaT;
		ppc->vtExt.h = virtExtHT;
		}

Done:
	UnlockHp(hpbBits);
	if (fColorToMono)
		UnlockHp(hpbPat);
	MeltHeap();


#ifdef PCWORD
	if (fAbort)
		UserAbort();
#endif /* PCWORD */

}



#ifdef SLOW_STRBLT

/*  %%Function:  SlowStrBltRow  %%Owner:  bobz       */

SlowStrBltRow(ppc, lpbSrc, lpbDst)
PICT		*ppc;
char FAR	*lpbSrc,
FAR	*lpbDst;
	/*
		* Stretchblts a single row, copying from lpbSrc to lpbDst.
		* Can handle compression and expansion.
		*/
{
	int		wRem,
	ibitSrc,
	ibitDst,
	virtSrc,
	cbitRep;
	int		ipl,
	cpl;
	BYTE FAR *rglpbSrc[4];		/* points to source planes */
	BYTE FAR *rglpbDst[4];		/* points to dest planes */
	BYTE	rgbSrc[4],			/* current byte in each source plane */
	rgbDst[4],			/* current byte in each dest plane */
	rgbCurPix[4],		/* only low bit is used in this array */
	*pbSrc,
			*pbDst,
			*pbCur,
			*pbSrcMac,
			*pbDstMac,
			*pbCurMac;
	BYTE	bT;
	BYTE FAR **plpbSrc,
			FAR **plpbDst;
	int		ico;				/* color number */

	pbSrcMac = &rgbSrc[ppc->cplIn];
	pbDstMac = &rgbDst[ppc->cplOut];
	pbCurMac = &rgbCurPix[ppc->cplOut];
	cpl = max(ppc->cplIn, ppc->cplOut);
	/* Note that this will initialize more pointers than used */
	/* if ppc->cplIn != ppc->cplOut. */
	for (ipl = 0, plpbSrc = rglpbSrc, plpbDst = rglpbDst; ipl < cpl; ++ipl)
		{
		*plpbSrc++ = lpbSrc;
		*plpbDst++ = lpbDst;
		lpbSrc += ppc->cbPlane;
		lpbDst += ppc->cbPlane;
		}

	ibitSrc = 0;
	ibitDst = 7;
	for (pbDst = rgbDst; pbDst < pbDstMac;)
		*pbDst++ = 0;
	wRem = 0;
	for (virtSrc = 0; virtSrc < ppc->vtExt.h; ++virtSrc)
		{
		wRem += ppc->wBitDelta;
		cbitRep = ppc->cbitRepNorm;
		if (wRem >= ppc->vtExt.h)
			{
			++cbitRep;
			wRem -= ppc->vtExt.h;
			}
		if (ibitSrc-- == 0)
			{
			for (pbSrc = rgbSrc, plpbSrc = rglpbSrc; pbSrc < pbSrcMac;)
				{
				*pbSrc++ = *(*plpbSrc)++;
				plpbSrc++;
				}
			ibitSrc = 7;
			}
		else
			{
			for (pbSrc = rgbSrc; pbSrc < pbSrcMac;)
				*pbSrc++ <<= 1;
			}
		if (cbitRep > 0)
			{
			ico = 0;
			for (pbSrc = pbSrcMac; pbSrc > rgbSrc;)
				{
				ico <<= 1;
				bT = (*--pbSrc & 0x80) ? 1 : 0;
				ico |= bT;
				}
			ico = ppc->rgico[ico];
			for (pbCur = rgbCurPix; pbCur < pbCurMac;)
				{
				*pbCur++ = ico & 0x01;
				ico >>= 1;
				}
			while (cbitRep-- > 0)
				{
				for (pbDst = rgbDst, pbCur = rgbCurPix; pbDst < pbDstMac;)
					*pbDst++ |= *pbCur++;
				if (ibitDst-- == 0)
					{
					for (pbDst = rgbDst, plpbDst = rglpbDst; pbDst < pbDstMac;)
						{
						*(*plpbDst)++ = *pbDst;
						plpbDst++;
						*pbDst++ = 0;
						}
					ibitDst = 7;
					}
				else
					{
					for (pbDst = rgbDst; pbDst < pbDstMac;)
						*pbDst++ <<= 1;
					}
				}
			}
		}
	if (ibitDst < 7)
		{
		for (pbDst = rgbDst, plpbDst = rglpbDst; pbDst < pbDstMac;)
			{
			**plpbDst++ = *pbDst++ << ibitDst;
			}
		}
}


#endif



#ifdef PCWORD

/*  %%Function:  SendBitmap  		 %%Owner:  bobz       */

SendBitmap(hpc)
PICT	**hpc;
/*
	* Purpose:	Send the current swath of the bitmap indicated by hpc
	* 			to the output device.  This is only used for the screen.
	* Method:	Set up a bitmap structure for each plane, and call BitBlt.
	*			Clip to the bottom of the clipping rectangle.
	*/
{
	extern int		vlm;
	extern BOOL		vfMouse;

	unsigned	ipl,
	pen,
	irw;
	char HUGE	*hpBits;
	PICT		*ppc;
	GRECT		rcSrc,
			rcSwath;
	char FAR	*lpSave,
	FAR	*lpCur;
	GPOINT		pt;
	int			irwOutMac;
	unsigned	rgcoAvail[icoLim];
	GRECT		rcT;


	Assert(vfProcs & fBitBlt);
	CheckGrAbort();
	ppc = *hpc;

	hpBits = HpOfSbIb(ppc->sbBits, 0);
	lpSave = LpLockHp(hpBits);
	rcSrc.top = 0;					/* rcSrc.bottom is set below */
	rcSrc.left = 0;
	rcSrc.right = ppc->pixExt.h;
	irwOutMac = min(ppc->irwOutMac,
			grafPort.rectClip.bottom - ppc->pixOrg.v);
	pt.h = ppc->pixOrg.h;
	pt.v = ppc->pixOrg.v + ppc->irwOutMic;

	SetColor(0, coBlack);

	if (ppc->cplOut == 1)	/* monochrome -- use regular BitBlt */
		{
		rcSrc.bottom = irwOutMac - ppc->irwOutMic;
		Move(pt.h, pt.v);
		if (vfMouse && vlm == lmPreview)
			{
			rcT = rcSrc;
			OffsetRect(&rcT, pt.h, pt.v);
			MouseConditionalOff(&rcT);
			}
		BitBlt(&rcSrc, lpSave, ppc->cbRowOut, FALSE);
		}
	else		/* color -- use planar BitBlt */		
		{
		GRSetApa(apaSolid);			/* first wipe out area with black */
		rcSwath.top = ppc->pixOrg.v + ppc->irwOutMic;
		rcSwath.left = ppc->pixOrg.h;
		rcSwath.bottom = ppc->pixOrg.v + irwOutMac;
		rcSwath.right = rcSwath.left + ppc->pixExt.h;
		if (vlm == lmPreview)
			MouseConditionalOff(&rcSwath);
		VctrRect(&rcSwath, FALSE);

		bltbh(hpvdvCur->rgcoAvail, rgcoAvail, icoLim);

		rcSrc.bottom = 1;

		for (irw = ppc->irwOutMic; irw < irwOutMac; ++irw)
			{
			lpCur = lpSave;
			Move(pt.h, pt.v);
			for (ipl = 0, pen = 1; ipl < ppc->cplOut; ++ipl, pen <<= 1)
				{
				SetColor(pen, rgcoAvail[pen]);
				BitBlt(&rcSrc, lpCur, ppc->cbRowOut, TRUE);
				lpCur += ppc->cbPlane;
				}
			lpSave += ppc->cbRowOut;
			++pt.v;
			}
		}
	if (vlm == lmPreview)
		MouseShowCursor();
	UnlockHp(hpBits);
}


#endif /* PCWORD */


#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Grbit_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* Grbit_Last() */
