/* PIC3.C - second half of PIC.C - split because too big for diffL */

/******************************/
/*  L P C H  I N C R
/*  %%Function:  LpchIncr  %%Owner:  marksea       */
//
// return(lpch + cch), adjusting for segment boundaries

LPCH LpchIncr(lpch, cch)
char far *lpch;
unsigned cch;
{
	extern int	vcbitSegmentShift;

	long	lT;

// this macro is more efficent than HIWORD and can be used as an lvar
#define MYHIWORD(l)	(*(((WORD *)&(l)) + 1))

	lT = (long)LOWORD(lpch) + (long)cch;
	MYHIWORD(lT) = (MYHIWORD(lT) << vcbitSegmentShift) + MYHIWORD(lpch);
	return(lT);
}

	
/*  %%Function:  PictError   %%Owner:  bobz       */

PictError (eid)
int eid;
{
	if (eid != eidNil && !PdodMother(vfli.doc)->fHadPicAlert)
		{
		if (eid == eidNoMemPict)
			ErrorNoMemory (eid);
		else
			ErrorEid (eid, "DrawChPic");
		PdodMother(vfli.doc)->fHadPicAlert = fTrue;  /* display once. period. */
		}


}


/*  %%Function:  FDrawMetafile  %%Owner:  bobz       */

FDrawMetafile(hdc, ppicrc, ww, peid)
HDC hdc;
struct PICRC *ppicrc;
int ww;
int *peid;
{
	extern HBRUSH vhbrWhite;

	int dypPicScl, dxpPicScl;
	int fDrawn = fFalse;
	HANDLE hbm=NULL;
	 // hBits will be NULL unless filled in by FOurPlayMetafile
	HANDLE hBits=NULL;
	HDC hMDC=NULL;
	struct FTI *pfti;
	int mm = vpicFetch.mfp.mm;
	int iLevel = 0;


#ifdef DEBUG
	int iDrawFailed = 0;
#endif /* DEBUG */

#ifdef DEBUG
	if (mm >= MM_META_MAX)
		{
		CommSzNum(SzShared("FDrawMetafile vpicFetch.mfp.mm: "), mm);
		Assert (fFalse);
		}
#endif /* DEBUG */

	/* Display the picture */

	dxpPicScl = ppicrc->picScl.xpRight - ppicrc->picScl.xpLeft;
	dypPicScl = ppicrc->picScl.ypBottom - ppicrc->picScl.ypTop;

	vfGrInterrupt = fFalse;

	/* Case 1: non-scalable metafile pictures which we are, for
		user interface consistency, scaling by force using StretchBlt, or
		Print Preview mode munging for all but ANISOTROPIC metafiles    */


	if ( (mm < MM_ISOTROPIC  &&
			(vpicFetch.mx != mx100Pct || vpicFetch.my != my100Pct ||
			vlm == lmPreview || vfli.fFormatAsPrint)) ||
			(mm == MM_TEXT && !vfli.fPrint) ||
			(mm == MM_ISOTROPIC && vlm == lmPreview) )
		{

		int dypOrig, dxpOrig;

		/* Compute original size of picture (in device pixels) */
		/* MM_ANISOTROPIC and MM_ISOTROPIC pictures have no original size */

		switch ( mm )
			{
		case MM_ANISOTROPIC:
			break;
		case MM_TEXT:
			/* These are pixels, and at print time they are printer pixels
				so we must treat them as such here. If no printer is
				available, we use screen pixels and the picture will likely
				clip on screen
			*/
			pfti = vfli.fFormatAsPrint ? &vftiDxt : &vfti;
			goto LGetOrigSize;

		default:
			pfti = &vfti;
LGetOrigSize:
			/* due to rounding error, we will almost never be able to treat non
			tropic metafiles as unscaled unless we use the same calculation
			that we use to get picScl, which involves the conversion of the
			xExt and yExt to twips, then to device pixels
			*/

			dxpOrig = UMultDiv (vpicFetch.dxaGoal,
					pfti->dxpInch, czaInch);
			dypOrig = UMultDiv (vpicFetch.dyaGoal,
					pfti->dypInch, czaInch);

			if (!dxpOrig || !dypOrig)
				{
				Debug0 (iDrawFailed = 2);
#ifdef XBZ
				CommSzNum(SzShared("FDrawMeta mm type: "), mm);
				CommSzNum(SzShared("FDrawMeta xext: "), vpicFetch.mfp.xExt);
				CommSzNum(SzShared("FDrawMeta yext: "), vpicFetch.mfp.yExt);
				CommSzNum(SzShared("FDrawMeta dxpOrig: "), dxpOrig);
				CommSzNum(SzShared("FDrawMeta dypOrig: "), dypOrig);
#endif
				goto DontDraw;
				}
			break;
			}

#ifdef DPICRC
		CommSzNum(SzShared("DrawChPic mm type: "), mm);
		CommSzNum(SzShared("DrawChPic mfp.xExt: "), vpicFetch.mfp.xExt);
		CommSzNum(SzShared("DrawChPic mfp.yExt: "), vpicFetch.mfp.yExt);
		CommSzNum(SzShared("DrawChPic dxpOrig: "), dxpOrig);
		CommSzNum(SzShared("DrawChPic dypOrig: "), dypOrig);
#endif


		if (
				( Debug0 (iDrawFailed = 11),
				((hMDC=CreateCompatibleDC( hdc)) != NULL) ) &&

				( Debug0 (iDrawFailed = 12),
				((hbm=CreateCompatibleBitmap( hdc, dxpOrig,
				dypOrig ))!=NULL) ) &&
				Debug((hbm == NULL) ? fTrue : (LogGdiHandle(hbm, 1010), fTrue) &&)
				( Debug0 (iDrawFailed = 13),
				SelectObject( hMDC, hbm ) ) &&
				( Debug0 (iDrawFailed = 14),
				FSelHdcForMf( hMDC ) ) ) 
			{
			struct RC rc;
			rc.xpLeft = rc.ypTop = 0;
			rc.xpRight = dxpOrig;
			rc.ypBottom = dypOrig;

			LogGdiHandle(hMDC, 1075);

			if (
					( Debug0 (iDrawFailed = 16),
					FSetupHdcForMf(hMDC, &rc, mm) ) &&
					( Debug0 (iDrawFailed = 17),
					FOurPlayMetaFile( hMDC, &hBits, ww ) ) )
				/* Because we pass pixels to StretchBlt */
				{
				SetMapMode( hMDC, MM_TEXT );

				if (mm == MM_ISOTROPIC)
					{
					/* may have been reset for isotropic */
					SetViewportOrg( hMDC, 0,0);
					SetWindowOrg( hMDC, 0,0);
					}
#ifdef DEBUG
				if (vdbs.fDumpPicInfo)
					DumpOrgExt(hMDC);
#endif /* BZ */
				Assert( hbm != NULL && hMDC != NULL );

				Debug0 (iDrawFailed = 25);
#ifdef DPICRC
				CommSzNum(SzShared("DrawChPic stretchblt dxpPicScl: "), dxpPicScl);
				CommSzNum(SzShared("DrawChPic stretchblt dypPicScl: "), dypPicScl);
				CommSzNum(SzShared("DrawChPic stretchblt dxporig: "), dxpOrig);
				CommSzNum(SzShared("DrawChPic stretchblt dyporig: "), dypOrig);
#endif

				fDrawn = StretchBlt( hdc, ppicrc->picScl.xpLeft,
						ppicrc->picScl.ypTop,
						dxpPicScl, dypPicScl,
						hMDC, 0, 0, dxpOrig, dypOrig,
						vsci.fMonochrome ?
						ropMonoBm : SRCCOPY );

				}  /* played metafile into dc */
			}  /* created bitmap, dc... */
		}      /* non-scalable metafile */

	/* Case 2: A metafile picture which can be directly scaled
		or does not need to be because its size has not changed.
		If we get here in Page Preview mode, we had better be ANISOTROPIC
	*/
	else
		{


#ifdef DEBUG
		if (vlm == lmPreview)
			Assert( mm == MM_ANISOTROPIC );
#endif /* DEBUG */

		/* save DC as guard against DC attribute alteration by a metafile */

		if ((iLevel = SaveDC (hdc)) == 0)
			{
			Debug0 (iDrawFailed = 26);
			goto DontDraw;
			}

		SelectObject( hdc, vsci.hpen );
		SelectObject( hdc, vlm != lmPreview ? vsci.hbrBkgrnd : vhbrWhite );

		{  // BLOCK
		struct RC rcViewport;
		if (vlm != lmPreview)
			{
			rcViewport = ppicrc->picScl;
			SetMapMode(hdc, mm);
			if (!FSetWinView(hdc, &rcViewport, mm))
				{
				Debug0 (iDrawFailed = 29);
				goto DontDraw;
				}
			}
		else
			{
			Assert(mm == MM_ANISOTROPIC);
            if (!FSetWindowOrgExt(hdc, mm) || 
			    !FScaleViewportForPrvw(hdc, &ppicrc->picScl, &rcViewport))
				{
				Debug0 (iDrawFailed = 29);
				goto DontDraw;
				}
			}
		}

		Debug0 (iDrawFailed = 27);
		  // play unscaled or scaleable metafile into bm cache if possible
        if ((vflm == flmDisplayAsPrint || vflm == flmDisplay) && vlm != lmPreview)
			fDrawn = FDrawPicCacheable(hdc, ppicrc, &iLevel, ww,
	   			          peid, &hBits, mm);
		else
			fDrawn = FOurPlayMetaFile(hdc, &hBits, ww);
		}

DontDraw:

	/* Clean up */

	if (iLevel > 0)
			RestoreDC (hdc, iLevel);

	if (hMDC != NULL)
		{
		UnlogGdiHandle(hMDC, 1075);
		AssertDo(DeleteDC( hMDC ));
		}
	if (hbm != NULL)
		{
		UnlogGdiHandle(hbm, 1010);
		DeleteObject( hbm );
		}
	if (hBits != NULL)
		GlobalFree( hBits );

	if (!fDrawn)
		if (!vfli.fPrint && vfGrInterrupt) /* interrupted by user */
			{
			fDrawn = fTrue;
			}
#ifdef BZ
		else
			{
			if (vfli.fPrint && !vfGrInterrupt) /* not interrupted by user */
				ReportSz("Warning - metafile not completely printed");
			else
				CommSzNum(SzShared("FDrawMetafile draw fail error index: "), iDrawFailed);
			}
#endif

	return fDrawn;
}


/*  %%Function:  FSelHdcForMf  %%Owner:  bobz       */
FSelHdcForMf( hMDC )
HDC hMDC;
{
	extern HBRUSH vhbrWhite;

	  // ok if these fail bz

	SetTextColor(hMDC, vsci.rgbText);
	SetBkColor(hMDC, vsci.rgbBkgrnd);

	if (SelectObject(hMDC, vsci.hpen) &&
			 SelectObject(hMDC,
				vlm != lmPreview ? vsci.hbrBkgrnd : vhbrWhite))
	   {
	   return fTrue;
	   }

	return fFalse;
}

/*  %%Function:  FSetupHdcForMf  %%Owner:  bobz       */
FSetupHdcForMf(hMDC, prc, mm)
HDC hMDC;
struct RC *prc;
int mm;
{

	if (PatBlt(hMDC, prc->xpLeft, prc->ypTop,
		    prc->xpRight - prc->xpLeft, prc->ypBottom - prc->ypTop,
			vsci.ropErase) &&
		SetMapMode(hMDC, mm) &&
		/* To cover StretchBlt calls within the metafile */
		SetStretchBltMode(hMDC, BLACKONWHITE) &&
		FSetWinView(hMDC, prc, mm) )
	   {
	   return fTrue;
	   }

	return fFalse;
}



/*  %%Function:  WLoadMF  %%Owner:  bobz       */
/* loads bitmap bits into the hbm selected into hdc */
/* return: true if ok, false if bad, GRINTERRUPT if interrupted */
WLoadMF(hdc, ppicrc, ww, peid, phBits, mm, fDcCache)
HDC hdc;
struct PICRC *ppicrc;
int ww;
int *peid;
HANDLE *phBits;
int mm;
BOOL fDcCache;
{
	BOOL fRet = fFalse;

	if (fDcCache) // setup bitmap in memory dc
		{
		struct RC rc;
		rc.xpLeft = rc.ypTop = 0;
		rc.xpRight = ppicrc->picScl.xpRight - ppicrc->picScl.xpLeft;
		rc.ypBottom = ppicrc->picScl.ypBottom - ppicrc->picScl.ypTop;

		if (!FSelHdcForMf(hdc) || !FSetupHdcForMf(hdc, &rc, mm))
			goto LRetCache;
		}

   	fRet = FOurPlayMetaFile(hdc, phBits, ww);
	if (vfGrInterrupt)
		fRet = GRINTERRUPT;


	if (fDcCache) // restore memory dc xfor bitblt
		{
LRetCache:
		SetMapMode( hdc, MM_TEXT );
	  	// in case they got changed
   		SetViewportOrg( hdc, 0, 0);
   		SetWindowOrg( hdc, 0, 0);
		}

	return fRet;
}


/* C B  D I B  H E A D E R */
/*  Determine the size of the DIB header + RGB data
    (the offset to the bitmap bits array)

	 The function works for BITMAPCOREHEADERs and BITMAPINFOHEADERs.
	 The number of bitmap planes must be 1.
	 The number of Bits/Pixel must be 1, 4, 8 or 24
	 Zero is returned if there is a problem with the data
*/
/*  %%Function:  CbDIBHeader   %%Owner: RobD       */

int CbDIBHeader(lpbi)
LPBITMAPINFOHEADER lpbi;
{
#define cbInfoHeader sizeof(BITMAPINFOHEADER)
#define cbCoreHeader sizeof(BITMAPCOREHEADER)
#define cbRGBQUAD    sizeof(RGBQUAD)
#define cbRGBTRIPLE  sizeof(RGBTRIPLE)

#define lpbc ((LPBITMAPCOREHEADER) lpbi)

	if (lpbi->biSize == cbInfoHeader)			/** Win 3.0 **/
		{
		if (lpbi->biPlanes != 1)
			return 0;

		/* REVIEW: (RobD)  What about biClrUsed ?? */
		if (lpbi->biClrUsed == 0)
			{
			switch (lpbi->biBitCount)
				{
			case 1:
				return (2 * cbRGBQUAD) + cbInfoHeader;
			case 4:
				return (16 * cbRGBQUAD) + cbInfoHeader;
			case 8:
				return (256 * cbRGBQUAD) + cbInfoHeader;
			case 24:
				return cbInfoHeader;
			default:
				return 0;	/* Invalid Bits/Pixel */
				}
			}
		else
			{
			return ((lpbi->biClrUsed * cbRGBQUAD) + cbInfoHeader);
			}
		}
	else if (lpbc->bcSize == cbCoreHeader)		/** Old Style - PM 1.1/1.2 **/
		{
		if (lpbc->bcPlanes != 1)
			return 0;

		switch (lpbc->bcBitCount)
			{
		case 1:
			return (2 * cbRGBTRIPLE) + cbCoreHeader;
		case 4:
			return (16 * cbRGBTRIPLE) + cbCoreHeader;
		case 8:
			return (256 * cbRGBTRIPLE) + cbCoreHeader;
		case 24:
			return cbCoreHeader;
		default:
			return 0;	/* Invalid Bits/Pixel */
			}
		}
	else
		return 0;	/* Invalid header size */

#undef cbInfoHeader
#undef cbCoreHeader
#undef cbRGBQUAD
#undef cbRGBTRIPLE
#undef lpbc

}	/* CbDIBHeader */


/* ******************************* */
/*  %%Function:  FSetWinView  %%Owner:  bobz       */

FSetWinView(hdc, prc, mm)
HDC hdc;
struct RC *prc;
int mm;
{

	Assert (mm < MM_META_MAX);
	if (!FSetWindowOrgExt(hdc, mm))
		return (fFalse);

	if (!FSetViewport(hdc, prc))
		return (fFalse);

	return (fTrue);
}


/* ******************************* */
/*  %%Function:  FSetWindowOrgExt  %%Owner:  bobz       */

FSetWindowOrgExt(hdc, mm)
HDC hdc;
int mm;
{

	Assert (mm < MM_META_MAX);
	if (mm == MM_ISOTROPIC)
		{
		/* if 0, no suggested size or aspect ratio */
		/* for non zero, sets up aspect ratio */
		if (vpicFetch.mfp.xExt && vpicFetch.mfp.yExt)
			{
			/* So we get the correct shape rectangle when
				SetViewportExt gets called */
			if (SetWindowExt( hdc, vpicFetch.mfp.xExt, vpicFetch.mfp.yExt ) == 0L)
				return (fFalse);
			}
		}
	else if (mm == MM_ANISOTROPIC)
		{
		int dxp, dyp;
		/* the rc is all 0's except in special import field case (non-tiff) */
		SetWindowOrg(hdc, vpicFetch.rcWinMF.xpLeft, vpicFetch.rcWinMF.ypTop);
		/* these can be in funny units corresponding to the source */
		dxp = vpicFetch.rcWinMF.xpRight - vpicFetch.rcWinMF.xpLeft;
		dyp = vpicFetch.rcWinMF.ypBottom - vpicFetch.rcWinMF.ypTop;
		if (dxp && dyp && SetWindowExt(hdc, dxp, dyp) == 0L)
				return (fFalse);
		}

	return (fTrue);
}




/*  %%Function:  FSetViewport  %%Owner:  bobz       */

FSetViewport(hdc, prc)
HDC hdc;
struct RC *prc;
{
	Assert (vpicFetch.mfp.mm < MM_META_MAX);

	SetViewportOrg( hdc, prc->xpLeft, prc->ypTop);

	/* for Isotropic, assumes window org/ext already set */

	switch (vpicFetch.mfp.mm)
		{
	case MM_ISOTROPIC:
	case MM_ANISOTROPIC:
		if  (SetViewportExt( hdc, prc->xpRight - prc->xpLeft, prc->ypBottom - prc->ypTop) == 0L)
			return (fFalse);
		break;
		}

	return (fTrue);
}


/* ******************************* */
/*  %%Function:  FScaleViewportForPrvw  %%Owner:  bobz       */

FScaleViewportForPrvw(hdc, prcIn, prcOut)
HDC hdc;
struct RC *prcIn;
struct RC *prcOut;
{
	extern PVS vpvs; /* rcPagePrint in normal page coordinates */
	struct RC  rcView; /* viewport rect */
	DWORD extViewport;
	DWORD orgViewport;

	Assert (vlm == lmPreview);
	Assert (prcIn != NULL && prcOut != NULL);

	/* we want to scale the pic rect to the rect set up by preview
		and set the viewport to that scaled rectangle. Note that the hdc was
		saved by the caller.
	*/
	extViewport = GetViewportExt(hdc);
	orgViewport = GetViewportOrg(hdc);

	rcView.xpLeft =   LOWORD(orgViewport);
	rcView.ypTop =    HIWORD(orgViewport);
	rcView.xpRight =  LOWORD(extViewport) + LOWORD(orgViewport);
	rcView.ypBottom = HIWORD(extViewport) + HIWORD(orgViewport);

	*prcOut = *prcIn;
	/* map picture rect into preview draw rect */
	MapRect(prcOut, &vpvs.rcPagePrint, &rcView);

	if (!SetViewportOrg( hdc, prcOut->xpLeft, prcOut->ypTop))
		return (fFalse);

	if (!SetViewportExt( hdc, prcOut->xpRight - prcOut->xpLeft, prcOut->ypBottom - prcOut->ypTop))
		return (fFalse);

	return (fTrue);
}


/* ******************************* */
/* F  E N H A N C E  W W  */
/*  %%Function:  FEnhanceWw   %%Owner:  bobz       */

FEnhanceWw (ww)
int ww;
{
	int fDone;

	/* Display enhancement routine for pictures. If window was
		dirtied and there are lines with pictures whose display
		was postponed, do the full display now.
	
		Can use this routine for gray-scale fonts too.
	
		Returns true if entire ww enhanced, else false if interrupted
	*/


	Assert (mpwwhwwd[ww] != hNil);

	/* will cause real pictures to draw, not just frames */
	vfDrawPics = fTrue;
	vfGrInterrupt = fFalse;
	fDone = FEnhanceHpldr(ww, HwwdWw(ww) /*hpldr*/);
	vfGrInterrupt = fFalse;
	vfDrawPics = fFalse;

	return (fDone);
}

/******************************/
/* F E N H A N C E  H P L D R */
/*  %%Function:  FEnhanceHpldr  %%Owner:  bobz       */

FEnhanceHpldr(ww, hpldr)
int ww;
struct PLDR **hpldr;
{
	/* Returns true if entire ww enhanced, else false if interrupted */

	int dl, dlMac;
	struct PLCEDL **hplcedl;
	struct EDL edl;
	struct DR *pdr;
	int idr, idrMac;
	struct DRF drfFetch;

	idrMac = (*hpldr)->idrMac;
	for (idr = 0; idr < idrMac; FreePdrf(&drfFetch), idr++)
		{
		pdr = PdrFetch(hpldr, idr, &drfFetch);
		Assert(pdr->hplcedl);
		dlMac = IMacPlc(hplcedl = pdr->hplcedl);

		for (dl = 0; dl < dlMac; dl++)
			{
			GetPlc(hplcedl, dl, &edl);
			if (edl.fEnd)
				break;

			if (edl.hpldr != hNil)
				{
				if (!FEnhanceHpldr(ww, edl.hpldr))
					{
LRetFalse:
					FreePdrf(&drfFetch);
					return fFalse;
					}
				}
			else  if (edl.fNeedEnhance)
				{
				/* bz 10/11/89 removed fmsgpresent call. We could get
				   into trouble with outside calls that use pdrgalley, and
				   all pictures can safely interrupt during displayfli
				*/

				if (ww == vwwClipboard)
					{
					Assert( PwwdWw(vwwClipboard)->hdc == NULL);
					if (!FGetClipboardDC())
						goto LRetFalse;
					}
				FormatLineDr(ww, CpPlc(hplcedl, dl), pdr);
				/* DisplayFli will turn edl.fNeedEnhance off */
				DisplayFli(ww, hpldr, idr, dl, edl.ypTop + vfli.dypLine);
				if (ww == vwwClipboard)
					ReleaseClipboardDC();

				if (vfGrInterrupt)
					goto LRetFalse;
				}
			}
		}
	return fTrue;
}


/********************************/
/* H i l i t e  P i c  S e l */
/*  %%Function:  HilitePicSel  %%Owner:  bobz       */

EXPORT HilitePicSel(psel, prcwPic)
struct SEL *psel;
struct RC *prcwPic;
{
	HilitePicSel1(psel, prcwPic, NULL /* prcwClip */);
}


/********************************/
/* H i l i t e  P i c  S e l 1 */
/*  %%Function:  HilitePicSel1  %%Owner:  bobz       */

EXPORT HilitePicSel1(psel, prcwPic, prcwClip)
struct SEL *psel;
struct RC *prcwPic;
struct RC *prcwClip;
{
	struct RC rcwClip;
	struct RC rc;
	struct PICRC picrc;
	int fCrop;
	int iLevel = 0;
	struct SEL selT;
	CP cpImport;

	/* see if pic is an import field; if so, set up sel to point to
		pic char in import field, otherwise leave sel as is. We
		can get here and fail FCaIsGraphics if an import field char
		is not visible, so just return in that case. 
	*/

	if (!(FCaIsGraphics(&psel->ca, psel->ww, &cpImport)))
		return;

	selT = *psel;
	if (cpImport != cpNil)  /* import field - change ca in selT */
		{
		selT.cpFirst = cpImport;
		selT.cpLim = cpImport + 1;
		}

	/* get picture rectangle.  */
	/* returns ffalse if couldn't get rect */

	if (prcwPic == 0)
		{
		if (!FFetchSelPicRc (&selT, &picrc, &fCrop,
				picgFrOut | picgFrIn /* frame rects only */,
				fFalse /* fInterrupt */))
			{
			return;
			}
		}
	else
		{
		/* get chp; can't use selT.chp since it is never set up to be
			fSpec.
		*/

		Assert (selT.fGraphics);
		FetchCpAndParaCa(&selT.ca, fcmProps);
		Assert(vchpFetch.fSpec);
		/* Note:
		FetchCp never sets fnPic since it is possible for
		fnPic to change during the lifetime of the fetch.
		All users of pic chps are required to set fnPic to
		fnFetch. This is one case where it is acceptable to
		modify vchpFetch without trashing the fetch.    bz
		*/

		vchpFetch.fnPic = fnFetch;

		if (!FGetPicRc (&picrc, &fCrop, &vchpFetch,
				prcwPic->xwLeft, prcwPic->ywBottom,  &selT,
				picgFrOut | picgFrIn /* frame rects only */))
			{
			return;
			}
		}

	Assert(vchpFetch.fSpec);
		  /* don't bother if invisible */
	if (vchpFetch.fVanish)
		return;

	if (cpImport == cpNil && !FVisibleCp (selT.ww, selT.doc, selT.cpFirst))
		return;

	if (prcwClip == NULL)
		if (!FClipRcPic(&selT, &picrc.frOut, &rcwClip))
			return;	/* not visible */

	HilitePicSel2(&selT, prcwPic, &picrc,
			prcwClip == NULL ? &rcwClip : prcwClip);
}


/*****************************/
/* H i l i t e  P i c  S e l 2 */
/* Send this routine the clip rect for the graphic */
/* Only called from HilitePicSel1 */
/*  %%Function:  HilitePicSel2  %%Owner:  bobz       */

EXPORT HilitePicSel2(psel, prcwPic, ppicrc, prcwClip)
struct SEL *psel;
struct RC *prcwPic;
struct PICRC *ppicrc;
struct RC *prcwClip;
{
	struct RC rcBrdr;
	struct RC rcDrag;
	struct RC rcLine;
	struct BRDR brdr;
	HDC hdc;
	int ircs;
	int ibrdr;
	int iLevel = 0;

	hdc = (*HwwdWw(psel->ww))->hdc;

	/* save DC since clipping */
	if ((iLevel = SaveDC (hdc)) == 0)
		{
		return;
		}
#ifdef XBZ
		{
		struct RC rcT;
		GetClipBox (hdc, (LPRECT)&rcT);
		CommSzRgNum(SzShared("HilitePicSel1 hdc clip rect before clip: "),
				&rcT, 4);
		}
#endif
	IntersectClipRect(hdc, prcwClip->xwLeft, prcwClip->ywTop,
			prcwClip->xwRight, prcwClip->ywBottom);

	/* draw in a single border if none; else use the one we have */

	SelectObject(hdc, vsci.hbrText);

	if (vpicFetch.brcl == brclNone)
		{
		/* No border, so draw one using the outside frame rect.
			Then adjust the inside frame rect to allow for a
			single border. Use the
			DrawPatternLine methods so we can invert 
		*/

		DrawXorFrame (hdc, &ppicrc->frOut, ipatHorzBlack, ipatVertBlack);

		ppicrc->frIn.xwLeft   += vsci.dxpBorder;
		ppicrc->frIn.ywTop    += vsci.dypBorder;
		ppicrc->frIn.xwRight  -= vsci.dxpBorder;
		ppicrc->frIn.ywBottom -= vsci.dypBorder;
		}

#ifdef XBZ
		{
		CommSzRgNum(SzShared("HilitePicSel1 clip rect: "),
				prcwClip, 4);
		CommSzRgNum(SzShared("HilitePicSel1 frOut rect size = "),
				&ppicrc->frOut, 4);
		CommSzRgNum(SzShared("HilitePicSel1 frIn rect size = "),
				&ppicrc->frIn, 4);
		}
#endif

	/* draw drag rectangles. Note we use the inside frame rect here */
	for (ircs = 0; ircs < ircsMax; ircs++)
		{
		GetDragRc(ircs, &rcDrag, &ppicrc->frIn);
		/* use multiple calls since it can only deal with 1 brdr size
			or double width at a time. Draw several vertical lines.
			Drag box must be a multiple of 2 * border widths wide
		*/

		for (ibrdr = 0; ibrdr < (cbrDragBox >> 1); ibrdr ++)
			{
			DrawPatternLine( hdc, rcDrag.xpLeft, rcDrag.ypTop,
					dypDragBox, ipatVertBlack, pltInvert | pltVert | pltDouble );
			rcDrag.xpLeft += 2 * vsci.dxpBorder;
			}

		}

	Assert (iLevel > 0);
	RestoreDC (hdc, iLevel);

}


/***********************/
/* F   C L I P  R C  P I C */
/* get clipping rect for a picture based on the dr clip rect and
	the line height restriction effect on the picture rectangle
*/
/*  %%Function:  FClipRcPic  %%Owner:  bobz       */

BOOL FClipRcPic(psel, prcPic, prcClip)
struct SEL *psel;
struct RC *prcPic, *prcClip;
{
	int dl, idr;
	struct PLDR **hpldr;
	struct RC rc;
	struct DRC drcp;
	struct EDL edl;
	struct DR *pdr;
	struct DRF drf;

	dl = DlWhereDocCp(psel->ww, psel->doc, psel->cpFirst, psel->fInsEnd,
			&hpldr, &idr, NULL, NULL, fTrue);
	if (dl == dlNil)
		return (fFalse);	/* not visible */

#ifdef DISABLE_DRM   /* from Mac */
	rc = *prcPic;
	RcwToRcp(hpldr, idr, &rc, &drcp);
	drcp.dxp -= drcp.xp;
	drcp.dyp -= drcp.yp;
#endif
	pdr = PdrFetch(hpldr, idr, &drf);
	GetPlc(pdr->hplcedl, dl, &edl);
	FreePdrf(&drf);
	drcp = edl.drcp;

	ClipRectFromDr(HwwdWw(psel->ww), hpldr, idr, &drcp, prcClip);

	/* adjust clip rect for neg line spacing */
	if (vpapFetch.dyaLine < 0)
		{
		rc = *prcPic;
		AdjustRcLine(&rc);
		prcClip->ypTop = max(prcClip->ypTop, rc.ypTop);
		}

	return (fTrue);
}


/***********************/
/* G e t  D r a g  R c */
/*  %%Function:  GetDragRc  %%Owner:  bobz       */

GetDragRc(ircs, prcDrag, prcFrame)
int ircs;
struct RC *prcDrag, *prcFrame;
{
/* return the rectangle for one of the seven drag areas on the frame-
*/
	switch (ircs)
		{
	case ircsTL:
	case ircsTC:
	case ircsTR:
		prcDrag->ypTop = prcFrame->ypTop;
		break;
	case ircsCL:
	case ircsCR:
		prcDrag->ypTop =
				(prcFrame->ypTop + prcFrame->ypBottom - dypDragBox) >> 1;
		break;
	case ircsBL:
	case ircsBC:
	case ircsBR:
		prcDrag->ypTop = prcFrame->ypBottom - dypDragBox;
		break;
		}
	switch (ircs)
		{
	case ircsTL:
	case ircsCL:
	case ircsBL:
		prcDrag->xpLeft = prcFrame->xpLeft;
		break;
	case ircsTC:
	case ircsBC:
		prcDrag->xpLeft =
				(prcFrame->xpLeft + prcFrame->xpRight - dxpDragBox) >> 1;
		break;
	case ircsTR:
	case ircsCR:
	case ircsBR:
		prcDrag->xpLeft = prcFrame->xpRight - dxpDragBox;
		break;
		}

	prcDrag->ypBottom = prcDrag->ypTop + dypDragBox;
	prcDrag->xpRight = prcDrag->xpLeft + dxpDragBox;
}


/*****************************/
/* A d j u s t	R c  L i n e */
/*  %%Function:  AdjustRcLine  %%Owner:  bobz       */

AdjustRcLine(prc)
struct RC *prc; 	/* rectangle of picture */
{
/* adjust rectangle to enclose only portion of picture showing in a line
	whose height is controlled by vpapFetch.dyaLine < 0 */
	int dyp;
	int hps;

	if (hps = selCur.chp.hpsPos)
		OffsetRect((LPSTR)prc, 0,
				NMultDiv(((hps < 128) ? (hps)  /* superscript */
				: -(256 - hps)) * (dyaPoint / 2),
				vfli.dxuInch, dxaInch));

/* now rectangle is sitting on baseline */
	dyp = DysFromDya(abs(vpapFetch.dyaLine));

#ifdef MACONLY
	prc->ypBottom += dyp / 5;     /* bottom of line */
#endif

	prc->ypTop = prc->ypBottom - dyp;
}


/***********************/
/* D R A W  X O R  F R A M E */
/* draw a single border sized frame in xor mode around the rect, using
	the specified patterns from DrawPatternLine
*/
/*  %%Function:  DrawXorFrame   %%Owner:  bobz       */

DrawXorFrame (hdc, prc, ipatHorz, ipatVert)
HDC hdc;
struct RC *prc;
int ipatHorz, ipatVert;
{
	/* top */
	DrawPatternLine( hdc, prc->xpLeft, prc->ypTop,
			prc->xpRight - prc->xpLeft,
			ipatHorz, pltInvert | pltHorz );
	/* bottom */
	DrawPatternLine( hdc, prc->xpLeft,
			prc->ypBottom - vsci.dypBorder,
			prc->xpRight - prc->xpLeft,
			ipatHorz, pltInvert | pltHorz );
	/* left */
	DrawPatternLine( hdc, prc->xpLeft,
			prc->ypTop + vsci.dypBorder,
			prc->ypBottom - prc->ypTop
			- (vsci.dypBorder << 1),
			ipatVert, pltInvert | pltVert);
	/* right */
	DrawPatternLine( hdc, prc->xpRight - vsci.dxpBorder,
			prc->ypTop + vsci.dypBorder,
			prc->ypBottom - prc->ypTop
			- (vsci.dypBorder << 1),
			ipatVert, pltInvert | pltVert);
}


/********************************/
/*  %%Function:  GetBitmapSize  %%Owner:  bobz       */

GetBitmapSize(pdxt, pdyt)
int  *pdxt;
int  *pdyt;
{

	int dxtT, dytT;
	unsigned  mx, my;

	Assert (vpicFetch.mfp.mm == MM_BITMAP || vpicFetch.mfp.mm == MM_TIFF);

	dxtT = vpicFetch.bm.bmWidth;
	dytT = vpicFetch.bm.bmHeight;

	/* we want to use printer values if display as print */


	GetBestWholeMxMy( (vfli.fFormatAsPrint | vfli.fPrint), /* fPrinter */
			dxtT, dytT, vpicFetch.dxaGoal, vpicFetch.dyaGoal, &mx, &my );

#ifdef XBZ
	CommSzNum(SzShared("GetBitmapSize dxaGoal: "), vpicFetch.dxaGoal);
	CommSzNum(SzShared("GetBitmapSize dyaGoal: "), vpicFetch.dyaGoal);
	CommSzNum(SzShared("GetBitmapSize bmWidth: "), dxtT);
	CommSzNum(SzShared("GetBitmapSize bmHeight: "), dytT);
	CommSzNum(SzShared("GetBitmapSize mx: "), mx);
	CommSzNum(SzShared("GetBitmapSize my: "), my);
#endif

	dxtT = (mx == 1 ? dxtT : dxtT * mx);
	dytT = (my == 1 ? dytT : dytT * my);

#ifdef XBZ
	CommSzNum(SzShared("GetBitmapSize dxtt after rescale, 1st mx: "), dxtT);
	CommSzNum(SzShared("GetBitmapSize dytt after rescale, 1st my: "), dytT);
#endif

	/* NOTE: we are not converting xt units back to screen for display as
		print. That is done by the caller, FCropGetPicSizes, who wants to
		do all the scaling and cropping on the unscaled values before scaling
		back for the screen.
	*/

	*pdxt = dxtT;
	*pdyt = dytT;


}


/********************************/
/*  %%Function:  GetBestWholeMxMy  %%Owner:  bobz       */

GetBestWholeMxMy( fPrinter, dxpOrig, dypOrig, dxaGoal, dyaGoal, pmx, pmy )
BOOL fPrinter;
int dxpOrig, dypOrig;
int dxaGoal, dyaGoal;
unsigned *pmx, *pmy;

/* Return the "best" integer bit-multiples to use when displaying a bitmap
	of size { dxpOrig, dypOrig } on device hdc.
	multiples are returned through *pmx (x-multiple), *pmy (y-multiple)
	caller is guaranteed nonzero return values
	Factors taken into account are:
		(1) Desired size of bitmap is { dxaGoal, dyaGoal }
		(2) Matching aspect ratio given by { dxaGoal, dyaGoal } is
			very desirable.
		(3) A scaling factor may be required for devices (like the LaserJet)
			for which Windows implements the "text and graphics have different
			pixel densities" hack.
		(4) Maximum size of a bitmap is 64K
			***   Max is no longer a restriction, though the size does
					have to fit in a long
			At some point we should determine the limiting Goal size that would cause
					overflow and abandon the multiply stuff if > than that
					value
*/

{

	int cx, cy;
	int cxBest, cyBest;
	int dcx=1, dcy=1;
	int dxmmOrig, dymmOrig;
	int dxmmGoal, dymmGoal;
	int dxmmDevice;
	int dymmDevice;
	int dxpDevice;
	int dypDevice;
	int cxMac, cyMac;
	int pctAspectBest, pctSizeBest;


	if (fPrinter)
		{
		Assert (vpri.hszPrinter != hNil);

		dxmmDevice = vpri.dxmmRealPage * 10;  /* want in .1mm units */
		dymmDevice = vpri.dymmRealPage * 10;
		dxpDevice =  vpri.dxpRealPage;
		dypDevice =  vpri.dypRealPage;


#ifdef NOTUSED  /* bobz if we skip this size is same across printer resolutions */
		/* Get scale factor if printing
				(dcx, dcy, our minimum scale multiple)
		*/

		dcx = 1 << vpri.ptScaleFactor.xp;
		dcy = 1 << vpri.ptScaleFactor.yp;
#endif /* BZ */


		}
	else  /* for screen */
		
		{
		dxmmDevice = vsci.dxmmScreen * 10;  /* want in .1mm units */
		dymmDevice = vsci.dymmScreen * 10;
		dxpDevice =  vsci.dxpScreen;
		dypDevice =  vsci.dypScreen;
		}

	/* Compute size of unscaled picture on device in 0.1 mm units */

	if (dxpDevice <= 0 || dypDevice <= 0)
		goto MxMyFail;

	/* some rounding loss here is reconversion (was originally .1mm,
		converted to twips, now back)
	*/
	dxmmGoal = UMultDiv (dxaGoal, 100, czaCm);
	dymmGoal = UMultDiv (dyaGoal, 100, czaCm);

	/* Goal size not supplied; return 100%  */

	if (dxmmGoal <= 0 || dymmGoal <= 0)
		goto MxMyFail;


	dxmmOrig = UMultDiv( dxpOrig, dxmmDevice, dxpDevice );
	dymmOrig = UMultDiv( dypOrig, dymmDevice, dypDevice );

	if (dxmmOrig <= 0 || dymmOrig <= 0)
		goto MxMyFail;

	/* Compute absolute maximums for cx, cy */
	/* 2nd term of min restricts search space by refusing to consider
	more tham one size above the Goal
	*/

	cxMac = min ( (dxmmDevice / dxmmOrig) + 1, (dxmmGoal / dxmmOrig) + 2 );
	cyMac = min ( (dymmDevice / dymmOrig) + 1, (dymmGoal / dymmOrig) + 2 );

#ifdef XBZ
	CommSzNum(SzShared("GetBestWholeMxMy dxmmDevice: "), dxmmDevice);
	CommSzNum(SzShared("GetBestWholeMxMy dymmDevice: "), dymmDevice);
	CommSzNum(SzShared("GetBestWholeMxMy dxpDevice: "),  dxpDevice);
	CommSzNum(SzShared("GetBestWholeMxMy dypDevice: "),  dypDevice);
	CommSzNum(SzShared("GetBestWholeMxMy dxmmOrig: "),   dxmmOrig);
	CommSzNum(SzShared("GetBestWholeMxMy dymmOrig: "),   dymmOrig);
	CommSzNum(SzShared("GetBestWholeMxMy dxmmGoal: "),   dxmmGoal);
	CommSzNum(SzShared("GetBestWholeMxMy dymmGoal: "),   dymmGoal);
#endif


	/* Search all possible multiplies to see what would be best */

	cxBest = dcx;
	cyBest = dcy;
	pctAspectBest = pctSizeBest = 32767;

	for ( cx = dcx ; cx < cxMac; cx += dcx )
		for ( cy = dcy ; cy < cyMac; cy += dcy )
			{
			int dxmm = dxmmOrig * cx;
			int dymm = dymmOrig * cy;
			int pctAspect = PctDiffUl( (ul) dxmmGoal * (ul) dymm,
					(ul) dymmGoal * (ul) dxmm );
			int pctSize = PctDiffUl( (ul) dxmmGoal * (ul) dymmGoal,
					(ul)dxmm * (ul)dymm );

		/* ??? Strategy for loss on one, gain on the other ??? */

			if (pctAspect <= pctAspectBest && pctSize <= pctSizeBest )
				{
#ifdef XBZ
				CommSzNum(SzShared("GetBestWholeMxMy pctAspect: "), pctAspect);
				CommSzNum(SzShared("GetBestWholeMxMy pctSize: "), pctSize);
#endif
				cxBest = cx;
				cyBest = cy;
				pctAspectBest = pctAspect;
				pctSizeBest = pctSize;
				}
			}

	Assert( cxBest > 0 && cyBest > 0 );

	*pmx = cxBest;
	*pmy = cyBest;
	return;

MxMyFail:
	*pmx = dcx;
	*pmy = dcy;
}



/********************************/
/*  %%Function:  PctDiffUl  %%Owner:  bobz       */

int PctDiffUl( ul1, ul2 )
ul ul1, ul2;
	{   /* Return a number that is proportional to the percentage
	of difference between the two numbers */
	/* Will not work for > 0x7fffffff */

#define dulMaxPrec  1000     /* # of "grains" of response possible */
#define dulLim 4294967       /* FFFFFFFF/1000; compiler won't do unsigned division */

	ul ulAvg = (ul1 >> 1) + (ul2 >> 1);
	ul ulDiff = (ul1 > ul2) ? ul1 - ul2 : ul2 - ul1;
	ul ulRet;

	if (ulAvg == 0)
		ulRet = (ul1 == ul2) ? 0 : dulMaxPrec;
	else if (ulDiff > dulLim || ulDiff < 0L)
		ulRet = dulMaxPrec;
	else
		ulRet = ((ulDiff * (ul)dulMaxPrec) / ulAvg);

#ifdef XBZ
	CommSzLong(SzShared("PctUl ulAvg: "), ulAvg);
	CommSzLong(SzShared("PctUl ulDiff: "), ulDiff);
	CommSzLong(SzShared("PctUl ulRet: "), ulRet);
	CommSzNum(SzShared("PctUl (int)ulRet: "), (int)ulRet);
#endif

	if (ulRet > 32767L || ulRet < 0L)
		return 32767;
	else
		return (int)ulRet;
}



/********************************/
/*  %%Function:  FComputePictSize  %%Owner:  bobz       */

FComputePictSize( pmfp, pdxa, pdya )
register METAFILEPICT *pmfp;
int *pdxa;
int *pdya;
	{   /* Compute an initial size, in twips, for the picture described by the
		passed metafile picture structure. Return the size through
		parameters.  Return fFalse if the metafile picture structure
		contained bad information, fTrue otherwise
	*/

	int mm = pmfp->mm;

	unsigned wMult;
	unsigned wDivX;
	unsigned wDivY;


	switch ( mm ) 
		{
	case MM_HIMETRIC:
		wMult = czaCm;
		wDivX = wDivY = 1000;
		break;
	case MM_LOMETRIC:
		wMult = czaCm;
		wDivX = wDivY = 100;
		break;
	case MM_HIENGLISH:
		wMult = czaInch;
		wDivX = wDivY = 1000;
		break;
	case MM_LOENGLISH:
		wMult = czaInch;
		wDivX = wDivY = 100;
		break;
	case MM_DIB: /* we may not get this, but take care of it as well */
	case MM_BITMAP:       /* note pixel based use SCREEN pixels */
	case MM_TEXT:
		wMult = czaInch;
		wDivX = vfli.dxsInch;
		wDivY = vfli.dysInch;
		break;
	case MM_TWIPS:  /* no conversion required */
		*pdxa = pmfp->xExt;
		*pdya = pmfp->yExt;
		goto GoodValues;
	case MM_ISOTROPIC:
	case MM_ANISOTROPIC:
		if (! ((pmfp->xExt > 0) && (pmfp->yExt > 0)))
			{   /* No "Suggested Size" given */
			/* Use dzPicBase" for largest size, dzPicBase" or as dictated by */
			/* aspect ratio for other side */
			if ((pmfp->xExt == 0) || (pmfp->yExt == 0))
				{
				/* No aspect ratio info given -- use 1" square */
				*pdya = *pdxa = dzPicBase * czaInch;
				}
			else
				{
				if (abs(pmfp->xExt) > abs(pmfp->yExt))
					{
					*pdxa = dzPicBase * czaInch;
					/* Info has neg #'s; use to compute aspect ratio */
					*pdya = UMultDiv (dzPicBase * czaInch, (abs(pmfp->yExt)),
							(abs(pmfp->xExt)));
					if (*pdya <= 0)
						*pdya = dzPicBase * czaInch;
					}
				else
					{
					*pdya = dzPicBase * czaInch;
					/* Info has neg #'s; use to compute aspect ratio */
					*pdxa = UMultDiv (dzPicBase * czaInch, (abs(pmfp->xExt)),
							(abs(pmfp->yExt)));
					if (*pdxa <= 0)
						*pdxa = dzPicBase * czaInch;
					}

				}
			goto GoodValues;
			}
		else
			{     /* have values - treat as himetric */
			wMult = czaCm;
			wDivX = wDivY = 1000;
			break;
			}
	default:
		ReportSz("Warning - bad mapping mode. Metafile discarded");
		*pdxa = *pdya = 0;
		return (fFalse);
		}

	if (pmfp->xExt <= 0 || pmfp->yExt <= 0)
		{
BadValues:
		/* assume we can live with this, and just use the default size */
		*pdya = *pdxa = dzPicBase * czaInch;
		return (fTrue);
		}

	*pdxa = UMultDiv (pmfp->xExt, wMult, wDivX);
	*pdya = UMultDiv (pmfp->yExt, wMult, wDivY);

#ifdef DPICRC
	CommSzNum(SzShared("FComputePicSize xext: "), pmfp->xExt);
	CommSzNum(SzShared("FComputePicSize yext: "), pmfp->yExt);
	CommSzNum(SzShared("FComputePicSize wMult: "), wMult);
	CommSzNum(SzShared("FComputePicSize wDivX: "), wDivX);
	CommSzNum(SzShared("FComputePicSize wDivY: "), wDivY);
	CommSzNum(SzShared("FComputePicSize dxa: "),  *pdxa);
	CommSzNum(SzShared("FComputePicSize dya: "),  *pdya);
#endif

	/* FFFF is failure return from UMultDiv */
	if (*pdxa == 0xFFFF || *pdya == 0xFFFF)
		goto BadValues;

GoodValues:
	/* restrict to min sizes for formatting */
	if (*pdxa < dxaPicMin)
		*pdxa = dxaPicMin;
	else  if (*pdxa > dxaPicMax)
		*pdxa = dxaPicMax;

	if (*pdya < dyaPicMin)
		*pdya = dyaPicMin;
	else  if (*pdya > dyaPicMax)
		*pdya = dyaPicMax;

	return fTrue;

}


#ifdef NOTUSED
/* REVIEW bobz (tdk) This routine is not currently used */
/********************************/
/* D z p  F r o m  M m  Z e x t */
/* Convert x or y coordinate from its GDI mapping mode (mm) to pixels */
/* on a device with pixel-to-absolute measurements given by DeviceRes parms */
/*  %%Function:  DzpFromMmZext  %%Owner:  bobz       */

int DzpFromMmZext( mm, val, pxlDeviceRes, milDeviceRes )
int mm;
int val;
int pxlDeviceRes;
int milDeviceRes;
	{   /* Return the # of pixels spanned by val, a measurement in coordinates
		appropriate to mapping mode mm.  pxlDeviceRes gives the resolution
		of the device in pixels, along the axis of val. milDeviceRes gives
		the same resolution measurement, but in millimeters.
	returns 0 on error */

	ul ulPxl;
	ul ulDenom;
	unsigned wMult=1;
	unsigned wDiv=1;


	if (milDeviceRes == 0)
		{   /* to make sure we don't get divide-by-0 */
		return 0;
		}

	switch ( mm ) 
		{
	case MM_LOMETRIC:
		wDiv = 10;
		break;
	case MM_HIMETRIC:
		wDiv = 100;
		break;
	case MM_TWIPS:
		wMult = 25;
		wDiv = 1440;
		break;
	case MM_LOENGLISH:
		wMult = 25;
		wDiv = 100;
		break;
	case MM_HIENGLISH:
		wMult = 25;
		wDiv = 1000;
		break;
	case MM_BITMAP:
	case MM_TEXT:
		return val;
	default:
		Assert( FALSE );        /* Bad mapping mode */
	case MM_ISOTROPIC:
	case MM_ANISOTROPIC:
		/* These picture types have no original size */
		return 0;
		}

/* Add Denominator - 1 to Numerator, to avoid rounding down */

	ulDenom = (ul) wDiv * (ul) milDeviceRes;
	ulPxl = ((ul) ((ul) wMult * (ul) val * (ul) pxlDeviceRes) + ulDenom - 1) /
			ulDenom;

	return (ulPxl > 32767L) ? 0 : (int) ulPxl;
}
#endif /* NOTUSED */


/**********************************/
/* F  C o n t e n t  H i t  P i c */
/*  %%Function:  FContentHitPic  %%Owner:  bobz       */

FContentHitPic(psel, ppt, fShift, fOption, fCommand)
struct SEL *psel;
struct PT *ppt;
int fShift, fOption, fCommand;
{
/* mouse click while a picture is selected - terminate the selection, change
	the size of the frame, or handle an interior click.
	Note that the selected picture may not be the one we clicked in. In that
	case we will return false here.
*/
	int ircs;
	struct RC rcDrag, rcLine;
	struct RC rcwClip;
	int fCrop;
	struct PICRC picrc;
	struct SEL selT;
	CP cpImport;

	/* see if pic is an import field; if so, set up sel to point to
		pic char in import field, otherwise leave sel as is. If pic
		is in a field and is not visible, we will get false back from
		FCaIsGraphics.
	*/

	if (!FCaIsGraphics(&psel->ca, psel->ww, &cpImport))
		goto LRet;

	selT = *psel;
	if (cpImport != cpNil)  /* import field - change ca in selT */
		{
		selT.cpFirst = cpImport;
		selT.cpLim = cpImport + 1;
		}

	/* Get inside frame rectangle for this stuff - want to see if hit is
		inside drag mark.
	*/

	if (!FFetchSelPicRc (&selT, &picrc, &fCrop, picgAll /* all 4 rects set up */,
			fFalse /* fInterrupt */))
		{
		goto LRet;
		}

	rcLine = picrc.frOut;
	/* CachePara is done in FFetchSelPicRc above */

	if (!PtInRect((LPRECT)&rcLine, *ppt))
		goto LRet;

	/* in the pic rect but part of that may be clipped. Clip rect
		includes adjustment for negative line spacing */

	if (!FClipRcPic(&selT, &rcLine, &rcwClip))
		goto LRet;  /* not visible */

	/* if in the pic rect, may be in part clipped out, so be sure
		it's there too (2 tests instead of clipping the rects and doing
		1 test)
	*/

	if (!PtInRect((LPRECT)&rcwClip, *ppt))
		goto LRet;

/* in body of picture */

	if (PdodDoc(psel->doc)->fLockForEdit)
		{
		Beep();  /* can't drag or change pic if locked */
		return(fTrue);
		}

	if (vfDoubleClick)
		{
		if (GetKeyState(VK_CONTROL) < 0)
			{
			/* restore picture to its "normal" size */
			ChangePicFrame(NULL, vpicFetch.brcl, mx100Pct, my100Pct,
					0, 0, 0, 0  /* cropping values */);
			}
		else
			{
			CmdExecBcmKc(bcmEditPic, kcNil);
			}
		return(fTrue);
		}

/* look for hit on one of the drag handles */
	for (ircs = 0; ircs < ircsMax; ircs++)
		{
		GetDragRc(ircs, &rcDrag, &picrc.frIn);

		if (PtInRect((LPRECT)&rcDrag, *ppt))
			{
			/* note that the one we drag is the outside
				frame rect */
			DragPicFrame(ircs, *ppt, &picrc, fShift);
			return(fTrue);
			}
		}

	return(fTrue);

LRet:
	/* FUTURE: This if block was added to fix bug 3039; it is
		very tailored to the way Opus 1.0 CBT lesson on picture dragging
		is authored and may be totally invalid or unnecessary in future
		versions of the CBT  - rp */
	if (vhwndCBT && vfShiftKey &&
		!SendMessage(vhwndCBT, WM_CBTSEMEV, smvNonPicSel, 0L))
		{
		return(fTrue);
		}

	return(fFalse);
}


/***************************************/
/* C h a n g e  P i c  F r a m e */
/*  %%Function:  ChangePicFrame  %%Owner:  bobz       */

ChangePicFrame(pca, brcl, mx, my, wCropTop, wCropLeft, wCropBottom, wCropRight)
struct CA *pca;
int brcl;
unsigned mx, my;
int wCropTop;
int wCropLeft;
int wCropBottom;
int wCropRight;
{
	int rgw[8];
	int cb, cw;
	unsigned mxAdj, myAdj;
	struct CA *pcaT;
	CP cpImport;
	BOOL fScale;
	struct CA caT;
	struct SEL selT;

	unsigned MxRoundMx();

	/* Assumptions: */

	/* Assert must be true for MxRoundMx to work for an my */
	Assert( mx100Pct == my100Pct );

	/* the current selection is a picture and vpicFetch is the picture */
	Assert (pca != NULL || selCur.fGraphics);

	/* if we are to use selCur, and selCur is an import field, get the
	ca of the pic char to apply the props to.
	*/

	if (pca == NULL)
		{
		AssertDo(FCaIsGraphics(&selCur.ca, selCur.ww, &cpImport));
		if (cpImport != cpNil)  /* import field - use pic char for ca */
			{
			caT.cpFirst = cpImport;
			caT.cpLim = cpImport + 1;
			caT.doc = selCur.doc;
			}
		}

	/* assumptions made in handbullding the grpprl */
	Assert (sprmPicBrcl < sprmPicScale
			&& dnsprm[sprmPicBrcl].cch == 2
			&& dnsprm[sprmPicScale].cch == 0
			&& sizeof(rgw) >= 16);

	cb = 0;
	if (brcl != vpicFetch.brcl)
		{
		*((char *) rgw + 0) = sprmPicBrcl;
		*((char *) rgw + 1) = brcl;
		cb = 2;
		}


	/* Round multipliers if near an even multiple */

	mxAdj = MxRoundMx( mx );
	myAdj = MxRoundMx( my );

	fScale = (mxAdj != vpicFetch.mx) || (myAdj != vpicFetch.my);
	if ( fScale ||
			(wCropTop != vpicFetch.dyaCropTop) ||
			(wCropLeft != vpicFetch.dxaCropLeft) ||
			(wCropBottom != vpicFetch.dyaCropBottom) ||
			(wCropRight != vpicFetch.dxaCropRight) )
		{
		*((char *) rgw + cb++) = sprmPicScale;
		*((char *) rgw + cb++) = 12; /* length of arguments */
		Assert (!(cb % 2));
		cw = cb >> 1;
		rgw[cw++] = mxAdj;
		rgw[cw++] = myAdj;
		rgw[cw++] = wCropLeft;
		rgw[cw++] = wCropTop;
		rgw[cw++] = wCropRight;
		rgw[cw++] = wCropBottom;
		cb = cw << 1;
		}

	/* Prevent invalidating vhplbmc (the bitmap cache) if we are just
	   cropping or changing borders. */
	vrf.fBorderCropPic = !fScale;
	if (cb)
		{
		if (pca != NULL)
			{
			ApplyGrpprlCa(rgw, cb, pca);
			InvalCp(pca);
              /* again handled by its callers */
			}
		else
			{
			/* for an import field, selCur holds the range for the entire
				field. but we want to apply to and handle undo on the pic
				char only. Slam selCur temporarily with the pic ca, then restore
			*/
			if (cpImport != cpNil)  /* import field - use pic char for ca */
				{
				selT = selCur;  /* save */
				selCur.ca = caT;
				}
			ApplyGrpprlSelCur(rgw, cb, fTrue /* fSetUndo */);

			if (cpImport != cpNil)  /* import field - use pic char for ca */
				selCur = selT;  /* restore */

            /* since we could do something like apply a large crop to a picture,
               then try to do it to a small pic, makeing it negative sized,
               clobber again here. Format Pic is itself not repeatable
            */

			if (vmerr.fMemFail) /* applygrpprlselcur can set this */
				goto LRet;

			InvalAgain();

			}

		if ((*hwwdCur)->fPageView)
			InvalPageView(pca == NULL ? selCur.doc : pca->doc);
		}
LRet:
	vrf.fBorderCropPic = fFalse;
}


/*  %%Function:  SetfEnhanceEdlWw  %%Owner:  bobz       */

SetfEnhanceEdlWw(ww)
int ww;
{
	struct PLCEDL **hplcedl;
	struct EDL edl;
	struct DRF drfFetch;

	/* get edl and set flag pic enhance flag */
	/* vdrdl... set in displayfli. This routine
		may only be called when inside DisplayFliCore
		called by Displayfli. Do not call when printing,
		as vdrdlDisplayFli will be invalid.    */

	Assert(!vfli.fPrint);

	hplcedl = PdrFetch(vdrdlDisplayFli.hpldr,
			vdrdlDisplayFli.idr,&drfFetch)->hplcedl;
	Assert(hplcedl);
	/* set flag, store */
	GetPlc(hplcedl, vdrdlDisplayFli.dl, &edl);
	edl.fNeedEnhance = fTrue;
	PutPlc(hplcedl, vdrdlDisplayFli.dl, &edl);

	/* set flag in ww so idle will redraw */
	PwwdWw(ww)->fNeedEnhance = fTrue;
	FreePdrf(&drfFetch);
}


/*  %Function:  FOurPlayMetaFile  %%Owner:  bobz       */

FOurPlayMetaFile(hdc, phMF, ww)
HDC hdc;
HANDLE *phMF;
int ww;
{


	int rgw[3]; /* rgw[0] == ww;
                 rgw[1] == saved dc;
                 rgw[2] == fInterrupt;
              */
	struct METAHDR far * lpMH; /* metafile header */
	int mtVersion;

	/* may yield or interrupt during display */

	BOOL fRet;


	/* We might never have needed to read the bits in. If we do, we will
		set *phMF so we only need to do it once, if we are going through the cache.
		Build up all bytes associated with the picture (except the header)
		into the global Windows handle hMF
   	*/

	if (*phMF == NULL)
		{
		long  cfcPic;
		LPCH    lpch;
		FC fc, fcMacPic;
		long lcbCur;
		int fnPreloadSave;
		char rgch[256];
#ifdef DEBUG
		int iT;
#endif /* DEBUG */

		cfcPic = (vpicFetch.lcb - vpicFetch.cbHeader);  /* size of pic bytes */

		if ((*phMF =GlobalAlloc2( GMEM_MOVEABLE, cfcPic )) == NULL)
			{    /* Not enough global heap space to load metafile */
			return fFalse;
			}
		if ((lpch = GlobalLock (*phMF)) == NULL)
			{
			Assert (*phMF != NULL);
			GlobalFree( *phMF );
			*phMF = NULL;
			return fFalse;
			}
		lpMH = lpch; /* so we can check win 3 meta after loading */

		fnPreloadSave = vfnPreload;
		vfnPreload = fnFetch;
		fcMacPic = vfcFetchPic + cfcPic;
		for (fc = vfcFetchPic;
			fc < fcMacPic;
			fc += lcbCur, lpch = LpchIncr(lpch, (unsigned)lcbCur))
			{
			FetchPeData(fc, (char HUGE *)rgch,
					/* using CpMin since lmin() does not exist */
			lcbCur = CpMin((long)256, fcMacPic - fc));

			// since we are always moving 256 bytes or the remaining
			// data which we have allocated room for, we will never cross
			// a 64k boundary during a blt.
			// assert there is room in current segment so we can use 
		    // bltbx instead of bltbxHuge
			Assert ((unsigned)(-LOWORD(lpch)) >= (unsigned)lcbCur
				|| (unsigned)(-LOWORD(lpch)) == 0);
			bltbx((char far *)rgch, lpch, (unsigned)lcbCur);
			}

		vfnPreload = fnPreloadSave;
		// win2 can't handle win3 metafile. abandon them.
		mtVersion = lpMH->mtVersion;
#ifdef DEBUG
		iT = 
#endif
		 	GlobalUnlock (*phMF);
		Assert(!iT);

		if (vwWinVersion < 0x0300 && mtVersion != 0x100) 
			{
			/* we got win 3 metafile that we can not display in Win 2 */
			Assert (*phMF != NULL);
			GlobalFree( *phMF );
			*phMF = NULL;
			PictError (eidPicWrongWin);
			return fFalse;
			}
		}

		rgw[0] = ww;
		rgw[1] = SaveDC(hdc);
          /* if true, we will allow pic to be interrupted */
	    rgw[2] = (vfli.fPrint || vlm == lmPreview || vfDeactByOtherApp) ?
                  fFalse : fTrue;


#ifdef DEBUG
	if (vdbs.fDumpPicInfo)
		DumpOrgExt(hdc);
#endif /* DEBUG */


		fRet = EnumMetaFile(hdc, *phMF, lpFMFContinue, (BYTE FAR *)rgw);
		if (rgw[1])  /* interrupt may have cleared it */
			RestoreDC (hdc, rgw[1]);
		return fRet;

}


/* Called by EnumMetafile. */

/*  %%Function:  FMFContinue  %%Owner:  bobz       */

EXPORT BOOL FAR PASCAL FMFContinue(hdc, lpHTable, lpMFR, nObj, lpClientData)
HDC hdc;
LPHANDLETABLE lpHTable;
LPMETARECORD lpMFR;
int nObj;
int FAR *lpClientData;
{

	MSG msg;

             /* bz for fPrint we used to call FQueryAbortCheck but that could
                cause updateww which would call formatline and we
                are within displayfli. So only abort at the line
                level (let print code handle it); Peek here to yield.
                In preview and deactivate, messages flying around interrupt
                the pic when we don't want it to, and they never show up
                so yield but don't interrupt.
             */

	if (!lpClientData[2])  /* fInterrupt */
		PeekMessage((MSG far *)&msg, NULL, 0, 0, PM_NOREMOVE);
    else if (FMsgPresent (mtyLongDisp)) /* won't blink & cause formatline call */
		{
             
	    if (vfDeactByOtherApp)
			{
			 /* we may get deactivated in mid draw; if so stop checking
				for interrupt
			 */
		    lpClientData[2] = fFalse; /* fInterrupt */
			}
		else
			{
#ifdef BZ
			CommSz(SzShared("FMFContinue metafile interrupted\n"));
#endif /* BZ */

		    Assert (!(vfli.fPrint || vlm == lmPreview));
			vfGrInterrupt = fTrue;
			/* cause it to be retried later */
	    	SetfEnhanceEdlWw(lpClientData[0] /* ww */);
			/* restore to original dc level to avoid interrupted savedc problems */
			Assert (lpClientData[1] != 0); /* saved dc */
			RestoreDC (hdc, lpClientData[1]);
			lpClientData[1] = 0;

			return fFalse;
			}
		}

#ifdef DEBUG
	if (vdbs.fDumpPicInfo)
		DumpMetaRec(lpMFR);
#endif /* BZ */

	PlayMetaFileRecord(hdc, lpHTable, lpMFR, nObj);
	return fTrue;
}



/******************************/

/*  M X  R O U N D  M X  */
/*  %%Function:  MxRoundMx  %%Owner:  bobz       */

unsigned MxRoundMx( mx )
unsigned mx;
	{   /* If mx is near an "interesting" multiple, round it to be exactly that
		multiple.  Interesting multiples are:
	
					1 (m=mx100Pct), 2 (m=2 * mx100Pct), 3 , ...
					0.5 (m = .5 * mx100Pct)
	
		Need to keep unobtrusive so that dragging won't seem to round
		to easily, but this is useful if we are close to an integer
		multiply.
	
		This routine works for my, too, as long as mx100Pct == my100Pct
*/
	/* This means close enough to round (2 decimal place accuracy) */

#define dmxRound    (mx100Pct / 200)

	unsigned mxRemainder;

	if (mx >= mx100Pct - dmxRound)
		{   /* Multiplier > 1 -- look for rounding to integer multiple */
		UDiv(mx, mx100Pct, &mxRemainder);
		if (mxRemainder  < dmxRound)
			mx -= mxRemainder;
		else  if (mxRemainder >= mx100Pct - dmxRound)
			mx += (mx100Pct - mxRemainder);
		}
	else
		{   /* Multiplier < 1 -- look for multiplication by 1/2 */
		UDiv (mx, (mx100Pct >> 1), &mxRemainder);
		if (mxRemainder  < dmxRound)
			mx -= mxRemainder;
		else  if (mxRemainder >= ((mx100Pct >> 1) - dmxRound))
			mx += (mx100Pct >> 1) - mxRemainder;
		}

	return (mx);
}



