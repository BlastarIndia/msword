/*------------------------------------------------------------------------
*
*
* Microsoft Windows Word -- Graphics
*
* Special graphics file routines
*
*-----------------------------------------------------------------------*/

#ifdef PCJ
#define DREADPIC
#endif /* PCJ */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "file.h"
#include "ff.h"
#include "grstruct.h"
#include "grtiff.h"
#include "pic.h"
#include "heap.h"
#include "doslib.h"
#include "debug.h"
#include "screen.h"
#include "field.h"
#include "ch.h"
#include "props.h"
#include "format.h"
#include "disp.h"
#include "doc.h"
#include "print.h"
#include "rareflag.h"
#include "error.h"


#define WFromStm() 	(WFromVfcStm())
#define cbitByte (8)

extern PICT 		**hpcCur;
extern struct MERR	vmerr;
extern struct PIC       vpicFetch;
extern FC               vfcFetchPic;    /* fc of last fetched pe */
extern STM              stmGlobal;
extern struct SCI       vsci;
extern struct FLI       vfli;
extern int              cbMemChunk;
extern int              vfDeactByOtherApp;
extern struct FTI       vfti;
extern struct FTI       vftiDxt;
extern CHAR             szEmpty[];
extern struct RC        vrcBand;
extern int              vlm;
extern struct CHP       vchpFetch;
extern BOOL             vfGrInterrupt;
extern struct PRI	vpri;
extern CHAR		szNone[];
extern HBITMAP vhbmPatVert;  /* something to select when we dump our bm */
extern int				vfnPreload;
extern int              fnFetch;

#ifdef DEBUG
extern int cHpFreeze;
#endif

extern LPCH LpchIncr();

struct PL **vhplgrib = -1;
HANDLE HReadStPict();
HANDLE HReadPgribSt();
HANDLE HOurLoadLibrary();


#ifdef INFO_ONLY
typedef struct
{
	char fPcodeEnv;
	char snEnv;
	unsigned bpcEnv;
	int cbEnv;
	int *fEnv;
} ENV;
#endif /* INFO_ONLY */


/* Globals */

ENV             vEnvGraphics = {
	0,0,0,0,0};


/* F C R  C A L C  F L T  I M P O R T  */
/*  This is the calculation function for IMPORT fields
	format:
		{IMPORT file-name}
*/
/*  %%Function:  FcrCalcFltImport   %%Owner:  bobz       */


FcrCalcFltImport (doc, ifld, flt, cpInst, cpResult, pffb)
int doc, ifld, flt;
CP cpInst, cpResult;
struct FFB *pffb;

{
	int fn;
	int istErr;
	CHAR stFile[ichMaxFile+1];
	CHAR stFileNorm[ichMaxFile];
	struct FLCD flcd;
	struct RC rc;
	struct CHP chp;

	InitFvbBufs(&pffb->fvb, &stFile[1], ichMaxFile, NULL, 0);

	/*  skip the keyword */
	SkipArgument(pffb);

	/*  fetch the file name */
	FFetchArgText(pffb, fTrue);

	/*  make an st */
	stFile[0] = pffb->cch;

	if (!stFile[0])
		{
		istErr = istErrNoFileName;
LError:
		CchInsertFieldError(doc, cpResult, istErr);
		return fcrError;
		}

	istErr = istErrCannotOpenFile;
	if (!FNormalizeStFile(stFile, stFileNorm, nfoNormal))
		goto LError;
	if (vmerr.fFnFull)
		KillExtraFns();
	if ((fn = FnOpenSt(stFileNorm, fOstReadOnly, ofcDoc, NULL)) == fnNil)
		goto LError;

	if (PfcbFn(fn)->fcMacFile == fc0)
		{
		ReportSz("0 length picture file discarded");
		KillExtraFns();
		goto LError;
		}

	GetResultChp (pffb, &chp);

	/* try reading other graphic format file first */
	if (vhplgrib == -1)
		InitGrib();

	if (vhplgrib != hNil)
		{
		HANDLE h1, h2;
		struct CA ca;

		FCloseFn(fn);/* assure library can open it */

		SetWords(&rc, 0, cwRC);
		if ((h1 = HReadStPict(stFileNorm, &rc, &h2)) != NULL)
			{
			BOOL f = FReadPict(PcaPoint(&ca, doc, cpResult), 
					CF_METAFILEPICT, h1, 0, fFalse /* fEmptyPic */,
					&chp, &rc /* prcWinMF */);
			GlobalFree(h1);
			Assert(h2 != NULL);
			GlobalFree(h2); /* free the hMF too */
			if (f)
				{
				goto LHavePict;
				}
			}
		}

	/* now try internal formats */
	/* note that we pass the UNNORMALIZED file name to be stored in
		the pic structure for tiff files. We normalize at display time.
	*/
	if (!FImportFnChPic(fn, stFile, doc, cpResult, &chp))
		{
		KillExtraFns();
		istErr = istErrCannotImportFile;
		goto LError;
		}

	/* NOTE: if we have loaded a TIFF image we leave the fn around (it will
		be needed to display the picture).  the number of TIFF files that can
		reasonably be open at once is limited by fnMax).  A converted image or
		an unsuccessful read do not need the fn. (unless we have very few fns
		left, then we delete it anyway)

	   New Note: leaving the file open prevents other users from accessing it;
	   For dibs, this is entirely unneeded. For TIFF, the bitmap cache and
	   eventually the new tiff filter will reduce the need to leave it open, so
	   let's just close it now using KillExtraFns bz 10/26/90
   	*/


LHavePict:

	KillExtraFns();
	GetIfldFlcd(doc, ifld, &flcd);
	flcd.fPrivateResult = fTrue;
	SetFlcdCh(doc, &flcd, chFieldEnd);

	return fcrNormal;
}


/* F  I M P O R T  F N  C H  P I C */
/* Read the file fn (stFile) which should be an IMPORT file (TIFF, etc).
	Place a chPic at doc, cp which, when displayed, will display the contents
	of the file.
	If the file format is not understood or for some other reason we cannot
	succeed, return fFalse.
*/
/*  %%Function:  FImportFnChPic  %%Owner:  bobz       */

FImportFnChPic(fn, stFile, doc, cp, pchp)
int fn;
CHAR *stFile;
int doc;
CP cp;
struct CHP *pchp;
{

	int ft;
	unsigned rgco[16];
	PICT  **hpc = hNil;  /* pc_word style PICT structure */
	PICT  *ppc;

	struct CA caPic;
	BITMAP bm;
	HANDLE hData = NULL;
	LPCH lpch;
	int fOk = fFalse;


	if (SetJmp(&vEnvGraphics) != 0)
		{
		if (hpc != hNil)
			EndPict(hpc);
		InvalidateEnv(&vEnvGraphics);
		return (fFalse);
		}

	StmInit(fn);
	ft = FtFromStm();

	if (ft == ftDIB)
		{
#define cbReadMax		256

		char	rgb[cbReadMax];
		char * pch = rgb;
		long	lcb;
		int	cb;
		BITMAPFILEHEADER bf;

		PcaPoint(&caPic, doc, cp);

		/* Read remaining part of Bitmap File Header */
		RgbFromVfcStm((CHAR *)(((BYTE *)&bf) + 2), sizeof(BITMAPFILEHEADER)-2);

		/* Calculate size of remaining data in BYTES */
		lcb = stmGlobal.fcMac - sizeof(BITMAPFILEHEADER);

		/* allocate space for DIB, can be > 64K, use GlobalAlloc2 */
		if ( ((hData=GlobalAlloc2(GMEM_MOVEABLE, lcb ))==NULL) ||
			  ((lpch=GlobalLock( hData ))==NULL))
			{
#ifdef DEBUG
			if (vdbs.fDumpPicInfo)
				CommSzLong(SzShared("Unable to allocate hData!  lcb="), lcb);
#endif /* DEBUG */
			goto ErrRet;
			}
		else
			{
			while (lcb > 0L)	/* RobD: Is there a way to read everything at once ? */
				                /* bz. Not, not using current file systen with huge pointers */
				{
				RgbFromVfcStm(pch, cb = ((lcb > cbReadMax) ? cbReadMax : (WORD) lcb));
				// since we are always moving 256 bytes or the remaining
				// data which we have allocated room for, we will never cross
				// a 64k boundary during a blt.
				// assert there is room in current segment so we can use 
		    	// bltbx instead of bltbxHuge
				// cbReadMax must be at least a power of 2 for this to work
				Assert (cbReadMax == 256);
				Assert ((unsigned)(-LOWORD(lpch)) >= (unsigned)cb
					|| (unsigned)(-LOWORD(lpch)) == 0);

				bltbx((LPCH)pch, lpch, cb);

				lpch = LpchIncr(lpch, (unsigned)cb);
				lcb  -= cb;
				}
			GlobalUnlock( hData );
			}

		/* load DIB into document */
		fOk = FReadPict(&caPic, CF_DIB, hData, 0, fFalse, pchp, NULL);

		GlobalFree(hData);
		return fOk;

#undef cbReadMax
		}

	if (ft != ftTIFF)
		return (fFalse);

	if ((hpc = HAllocateCb(sizeof(PICT))) == hNil)
		return (fFalse);

	InitPict(hpc);
	(*hpc)->ft = ftTIFF;
	(*hpc)->fn = fn;

	GetInfoTiff(hpc, rgco); /* may abort out to setjmp location */

	PcaPoint(&caPic, doc, cp);
	/* allocate space for bitmap structure floowed by stFile */
	if ( ((hData=OurGlobalAlloc(GMEM_MOVEABLE,
			(long)(sizeof(BITMAP) + *stFile + 1) ))==NULL) ||
			((lpch=GlobalLock( hData ))==NULL))
		{
		goto ErrRet;
		}
	else
		{
		ppc = *hpc;
		FreezeHp();
		bm.bmType = 0; /* logical bitmap */
		bm.bmWidth = ppc->vtExt.x;
		bm.bmHeight = ppc->vtExt.y;

		/* widthbytes * 8 must be next multiple of 16 > bmWidth.
		so calc # of words, then convert back to bytes
		*/
		bm.bmWidthBytes = ((bm.bmWidth + 15) >> 4) << 1;


		/* Note. currently GetInfoTiff will only handle 1
				and does not store the value in the PICT struct.
				If that changes, update this. Also, for now
				there will be only 1 output plane  (no color
				bm support. Change also if needed in the future.
				to pick up ppc->cplIn
		*/
		bm.bmBitsPixel = 1;
		bm.bmPlanes = 1;

		bm.bmBits = NULL;
		/* store bm struct AND file name */
		bltbx( (LPCH)&bm, lpch, sizeof(BITMAP) );
		bltbx( (LPCH)stFile, lpch + sizeof(BITMAP), *stFile + 1 );
		GlobalUnlock( hData );
		MeltHp();
		}


	/* load tiff bitmap info into document */
	fOk = FReadPict (&caPic, CF_TIFF,
			hData, 0 /* cbInitial */,
			fFalse /* fEmptyPic */,
			pchp, NULL /* prcWinMF */);

ErrRet:
	if (hData != NULL)
		GlobalFree( hData );
	if (hpc != hNil)
		FreeH (hpc);


#ifdef DEBUG
	if (vdbs.fDumpPicInfo)
		CommSzSt(SzShared("FImportFnChPic: stFile = "), stFile);
#endif

	return fOk;
}


/*  %%Function:  FtFromStm  %%Owner:  bobz       */

int FtFromStm()
	/*
		* Determine the file format of the file of the current stream.
		*
		* Returns:		a file format code, or ftNil if unrecognizeable.
							Adapted/cut down  (bz) from pc word 5
			*/
{
	unsigned w0, w1;

	StmSetTIFF((FC) 0);
	w0 = WFromVfcStm();
	switch (w0)
		{
	case 0x4949:
		if (WFromVfcStm() == 0x002a)
			return(ftTIFF);		/* TIFF */
		break;
	case 0x4d4d:
		if (WFromVfcStm() == 0x2a00)
			return(ftTIFF);		/* TIFF */
		break;
	case BFT_BMAP:
		return(ftDIB);	/* DIB */
	default:
		break;
		}

	return(ftNil);
}


/*  %%Function:  WLoadBM  %%Owner:  bobz       */
/* loads bitmap bits into the hbm selected into hdc */
/* return: true if ok, false if bad, GRINTERRUPT if interrupted */
int WLoadBm(hdc, ppicrc, ww, peid, ft, fDcCache)
HDC hdc;
struct PICRC *ppicrc;
int ww;
int *peid;
int ft;
BOOL fDcCache;
{

	int errRet;
	PICT  **hpc;  /* pc_word style PICT structure */
	int fn;
	int fnPreloadSav;

	fnPreloadSav = vfnPreload;
	if ((errRet = SetJmp(&vEnvGraphics)) != 0)
		{
		KillExtraFns();
		if (hpc != hNil)
			EndPict(hpc); /* note this frees partially built hmdc */
		/* reflag so it will redraw later */
		if (errRet == GRINTERRUPT)
			{
			/* this causes FEnhandePldr to return false, so we will redisplay */
			Assert (vfGrInterrupt);
			if (!vfli.fPrint)
				SetfEnhanceEdlWw(ww);  /* this can only work on screen */
			}
		InvalidateEnv(&vEnvGraphics);
		vfnPreload = fnPreloadSav;
		return (errRet == GRINTERRUPT ? GRINTERRUPT : fFalse);
		}

	Assert (!cHpFreeze);

	/* Anything that could potentially do a DoJmp should follow the
	   call to HAllocateCb so hpc will be initialized. */
	if ((hpc = HAllocateCb(sizeof(PICT))) == hNil)
		{
		ReportSz("Warning - bitmap PICT struct alloc failed - pic not shown");
		goto LError;
		}
	InitPict(hpc);

	if (ft == ftClipboard)
		fn = fnFetch;
	else
		{
		long  cfcPic;
		CHAR rgch[256];
		CHAR stFileNorm[ichMaxFile];

		cfcPic = (vpicFetch.lcb - vpicFetch.cbHeader);  /* size of filename */
		Assert (cfcPic <= 256); /* it is an st */
				/* get file name */
		FetchPeData(vfcFetchPic, (char HUGE *)rgch, cfcPic);
			/* note: this assumes the above read succeeds; it has no way
		    	to signal failure
			*/
#ifdef XBZ
		CommSzSt(SzShared("FDrawTiff: stFile = "), rgch);
#endif /* XBZ */

		if (!FNormalizeStFile(rgch, stFileNorm, nfoNormal)
			|| (fn = FnOpenSt(stFileNorm, 0, ofcDoc, NULL)) == fnNil)
			{

LErrorTiff:
			*peid = eidPicNoOpenDataFile; 
			goto LError;
			}

		StmInit(fn);
		ft = FtFromStm();
		if (ft != ftTIFF)
			goto LErrorTiff;
		}
	(*hpc)->fn = fn;
	(*hpc)->ft = ft;

	if (ft == ftTIFF) /* only needed by tiff */
		if (((*hpc)->sbPat = SbAllocEmmCb(cbDitherPat)) == sbNil)
			{
			ReportSz("Warning - tiff emm alloc failed - pic not shown");
			goto LError;
			}

	(*hpc)->pixExt.x = ppicrc->picScl.xpRight - ppicrc->picScl.xpLeft;
	(*hpc)->pixExt.y = ppicrc->picScl.ypBottom - ppicrc->picScl.ypTop;
	/* upper left hand corner of picture */
	(*hpc)->pixOrg.x = ppicrc->picScl.xpLeft;
	(*hpc)->pixOrg.y = ppicrc->picScl.ypTop;

	PrecalcBitParms(hpc);
#ifdef XBZ
		{
		long l = (long)(*hpc)->cbRowIn  *  (long)(*hpc)->irwInMax;
		CommSzLong(SzShared("Bytes in bitmap: "), l);
		}
#endif /* XBZ */
	Assert (!cHpFreeze);


	if (((*hpc)->hMDC = CreateCompatibleDC(hdc)) == NULL)
		{
		ReportSz("Warning - bitmap CreateCompatibleDC failed - pic not shown");
		goto LError;
		}
	LogGdiHandle((*hpc)->hMDC, 1045);
	if (!FAllocBitmap(hpc))
		{
		ReportSz("Warning - bitmap alloc failed - pic not shown");
LError:
		Assert (!cHpFreeze);
		GraphicsError(hpc, IDPMTPictErr);
		Assert(fFalse);  /* GraphicsError does a DoJmp */
		}
	FreezeHp();
	if (!vfli.fPrint)
		GetClipBox (hdc, (LPRECT)&((*hpc)->rcClip));
	else
		(*hpc)->rcClip = vrcBand;
	if (fDcCache)
		{
		/* Disable limiting to clip rectangle if going into cache */
		(*hpc)->rcClip.ypTop = -32767;
		(*hpc)->rcClip.ypBottom = 32767;
		}
	MeltHp();

	Assert (!cHpFreeze);
	vfnPreload = fn;
	while (FPrepBitmap(hpc))
		{
		ReadBitmap(hpc);  /* pull in a swath */
		CheckGrAbortWin(hpc);
		/* check clipping; if above clip we read but not translate */
		if ((*hpc)->pixOrg.v + (int)(*hpc)->irwOutMac >= (*hpc)->rcClip.ypTop)
			{
			TranslateBitmap(hpc); /* stretchblt and map colors */
			/* didn't want to check w/in translate... */
			CheckGrAbortWin(hpc);
			if (!FSendBitmap(hpc, hdc, ppicrc, fDcCache))
				{
				Assert (!cHpFreeze);
				ReportSz("Warning - snd bitmap failed - pic not shown");
				GraphicsError(hpc, IDPMTPictErr);
				/* break;   not needed since we will dojmp */
				}
			}
		}

	KillExtraFns();
	EndPict(hpc);  /* frees up hpc */
	vfnPreload = fnPreloadSav;
	return (fTrue);
}







/*  %%Function:  InitPict  %%Owner:  bobz       */

InitPict(hpc)
PICT	**hpc;
	/*
	* Initialize a PICT.
	*/
{
	Assert (sbNil == 0);  /* so blt will set up sb's correctly */

	SetBytes(*hpc, 0, sizeof (PICT));
	vfGrInterrupt = fFalse;
}


/*  %%Function:  FAllocBitmap  %%Owner:  bobz       */

FAllocBitmap(hpc)
PICT  **hpc;
{
	unsigned csw;  /* number of swaths worh of memory allocated */
	unsigned cbSwathMin;
	PICT *ppc;
	unsigned cbWant = 0;
	SB sb = sbNil;

	/* if we can't get a whole segment worth, ask for half as much
	until we get some. 
	*/
	cbSwathMin = (*hpc)->cbSwathMin;
	csw =  UMultDiv((unsigned) (cbLmemHeap - (*hpc)->cbExtra), 1, cbSwathMin);

	while (csw >= 1)
		{
		cbWant = csw * cbSwathMin + (*hpc)->cbExtra;
		if ((sb = SbAllocEmmCb(cbWant)) != sbNil)
			break;
		else
			csw >>= 1;
		}

	if (sb != sbNil)
		{
		ppc = *hpc;
		ppc->cbBits = cbWant;
		ppc->sbBits = sb;
		ppc->ibIn = 0;
		}

	return (sb != sbNil);
}


/*  %%Function:  ReadBitmap  %%Owner:  bobz       */

ReadBitmap(hpc)
PICT  **hpc;
{


#ifdef XBZ
	CommSzRgNum(SzShared("hpc in readbitmap to tiff: "),
			(char *)*hpc, (sizeof(PICT) - sizeof (TIFF)) >> 1);
	if ((*hpc)->ft == ftTIFF)
		CommSzRgNum(SzShared("hpc in readbitmap tiff: "),
				((char *)*hpc) + (sizeof(PICT) - sizeof (TIFF)), sizeof(TIFF) >> 1);
#endif /* BZ */


	if ((*hpc)->ft == ftTIFF)
		ReadTIFF(hpc);
	else
		ReadWIN(hpc);

}


/*  %%Function:  ReadWIN  %%Owner:  bobz       */

ReadWIN(hpc)
PICT  **hpc;
{
	long cb;
	PICT		*ppc;
	FC                  fcFetch;

	ppc = *hpc;
	cb = ppc->cbRowIn * (ppc->irwInMac - ppc->irwInMic);
	Assert (cb <= ppc->cbBits);
	Assert (ppc->cbRowIn == vpicFetch.bm.bmWidthBytes);
	Assert(vchpFetch.fSpec);

	/* we need to read starting at irwInMic, not read successively
		because of rounding that can cause overflow. vfcFetchPic
		is left after the pic struct where FetchPe put it.
	*/

	fcFetch = vfcFetchPic + 
			(long)(ppc->cbRowIn * ppc->irwInMic);

	Assert ((LONG)((long)cb + fcFetch - vchpFetch.fcPic & 0x00FFFFFF)  <= vpicFetch.lcb);

	FetchPeData(fcFetch, HpOfSbIb(ppc->sbBits, ppc->ibIn), cb);


}


/*  %%Function:  FSendBitmap  %%Owner:  bobz       */

FSendBitmap(hpc, hdc, ppicrc, fDcCache)
PICT  **hpc;
HDC   hdc;
struct PICRC *ppicrc;
BOOL  fDcCache;
{

	LPCH    lpch;
	int dypPic, dxpPic;
	BITMAP bm;
	CHAR HUGE *hpBits;

	Assert ((*hpc)->hMDC != NULL);

	dxpPic = ppicrc->picScl.xpRight - ppicrc->picScl.xpLeft;
	dypPic = (*hpc)->irwOutMac - (*hpc)->irwOutMic;

	/* create bitmap only if there is none or its height has changed */
	if ((*hpc)->hbm == NULL || (*hpc)->dypBm != dypPic)
		{
		if ((*hpc)->hbm != NULL)  /* dump old one */
			{
			SelectObject( (*hpc)->hMDC, vhbmPatVert); /* abritrary bm; so we can dispose selected bm */
			UnlogGdiHandle((*hpc)->hbm, -1);
			DeleteObject((*hpc)->hbm);
			}
		/* the bitmap we are now creating has been stretchblted
		to the desired output size, and so is no longer the same
		as that in vpicFetch. In later calls, however, the width
		will stay the same but the height may change, especially
		if not only, on the last swath.
		*/
		bm.bmType = 0; /* logical bitmap */
		bm.bmWidth =  dxpPic;
		bm.bmHeight = dypPic;
		bm.bmWidthBytes = (*hpc)->cbRowOut;
		bm.bmBitsPixel = 1;
		bm.bmPlanes = 1;
		bm.bmBits = NULL;
		if (((*hpc)->hbm = CreateBitmapIndirect((LPBITMAP)&bm)) == NULL)
			return (fFalse);  /* caller will handle freeing */
		LogGdiHandle((*hpc)->hbm, 1030);
		(*hpc)->dypBm = dypPic;
		}


	hpBits = HpOfSbIb((*hpc)->sbBits, 0);
	/* bm.bits just a handy holder for the lp */
	if ( ((bm.bmBits = LpLockHp(hpBits)) == NULL)  ||
			(SetBitmapBits((*hpc)->hbm, (DWORD)((*hpc)->cbRowOut * dypPic),
			(LPSTR)bm.bmBits) == 0L) ||
			(SelectObject( (*hpc)->hMDC, (*hpc)->hbm ) == NULL) )
		{
		if (bm.bmBits != NULL)
			UnlockHp(hpBits);
		return (fFalse);  /* caller will handle freeing */
		}

	UnlockHp(hpBits);

	Assert( (*hpc)->hbm != NULL && (*hpc)->hMDC != NULL );

	BitBlt( hdc, fDcCache ? 0 : ppicrc->picScl.xpLeft,
			(fDcCache ? 0 : ppicrc->picScl.ypTop) + (*hpc)->irwOutMic,
			dxpPic, dypPic,
			(*hpc)->hMDC, 0, 0, 
			vsci.fMonochrome ?
			ropMonoBm : SRCCOPY );

#ifdef XBZ
	CommSzNum(SzShared("FSendBitmap dxpPic: "), dxpPic);
	CommSzNum(SzShared("FSendBitmap dypPic: "), dypPic);
	CommSzNum(SzShared("FSendBitmap dxpIn: "), (*hpc)->vtExt.h);
	CommSzNum(SzShared("FSendBitmap dypIn: "), (*hpc)->irwInMac - (*hpc)->irwInMic);
#endif

	return (fTrue);

}


/*  %%Function:  EndPict  %%Owner:  bobz       */

EndPict(hpc)
PICT  **hpc;
{
	extern HBITMAP vhbmPatVert;

#ifdef XBZ
	CommSzNum(SzShared("EndPic sbBits: "), (*hpc)->sbBits);
	CommSzNum(SzShared("EndPic sbPat: "), (*hpc)->sbPat);
#endif

	if ((*hpc)->sbBits != sbNil)
		FreeEmmSb((*hpc)->sbBits);

	if ((*hpc)->sbPat != sbNil)
		FreeEmmSb((*hpc)->sbPat);


	if ((*hpc)->hMDC != NULL)
		{
		FSetDcAttribs( (*hpc)->hMDC, dccmUnlockBrushPen );
		SelectObject( (*hpc)->hMDC, vhbmPatVert); /* abritrary bm; so we can dispose selected bm */
		UnlogGdiHandle((*hpc)->hMDC, 1045);
		AssertDo(DeleteDC( (*hpc)->hMDC ));
		}
	if ((*hpc)->hbm != NULL)
		{
		UnlogGdiHandle((*hpc)->hbm, -1);
		DeleteObject( (*hpc)->hbm );
		}

	FreeH(hpc);

}


/* ************* Misc routines from PC Word needed by Opus **************** */


/*  %%Function:  HprgbFromVfcStm  %%Owner:  bobz       */


HprgbFromVfcStm(hprgb, cb)
char HUGE *hprgb;
int	cb;
{
	SetFnPos(stmGlobal.fn, stmGlobal.fc);
	ReadHprgchFromFn(stmGlobal.fn, hprgb, cb);
	stmGlobal.fc += cb;
}


/*  %%Function:  WSwappedFromStm  %%Owner:  bobz       */

WSwappedFromStm()
{
	int w;
	w = WFromStm();
	SwitchBytes(&w, 1);
	return (w);
}


/*  %%Function:  StmInit  %%Owner:  bobz       */

StmInit(fn)
{
	stmGlobal.fn = fn;
	stmGlobal.fc = (FC)0;
	stmGlobal.fcMac = (FC)PfcbFn(fn)->cbMac;

#ifdef XBZ
	CommSzNum(SzShared("StmInit stmglobal.fn: "), stmGlobal.fn);
	CommSzLong(SzShared("StmInit stmglobal.fcMac: "), stmGlobal.fcMac);
#endif
}


/*  %%Function:  StmSetTIFF  %%Owner:  bobz       */
/*  TIFF variant of SetStmPos that aborts via SetJmp if fc out of range.
    vENVGraphics MUST be set before this is called. Only call this out
    of the routines called by FImportFnChPic or FDrawBitmap, which set
    the env.
*/

StmSetTIFF(fc)
FC fc;
{
	if (fc > stmGlobal.fcMac || fc < (FC)0)
		{
#ifdef BZ
		CommSzLong(SzShared("StmSetTIFF inval fc: "), fc);
#endif
		Assert (vEnvGraphics.fPcodeEnv);  /* assure ENV set up */
		EndGraphics(GRERROR);   /* does a dojmp */
		}
	else
		SetStmPos(stmGlobal.fn, fc);
}




/* Sign-extend character to integer     */
/*  %%Function:  sxt   %%Owner:  bobz       */

int sxt (ch)
int     ch;
{
	return(ch - ((ch & 0x80) << 1));
}


/*  %%Function:  EndGraphics  %%Owner:  bobz       */

EndGraphics(valRet)
{
	/* EndPict(hpc) now done at Jmp location */
	DoJmp (&vEnvGraphics, valRet);
}


/*  %%Function:  FEof  %%Owner:  bobz       */

FEof()
{
	return (stmGlobal.fc > stmGlobal.fcMac);
}


/*  %%Function:  CheckGrAbortWin  %%Owner:  bobz       */

CheckGrAbortWin(hpc)
{
	extern HWND vhwndPgPrvw;       /* Page preview window handle */
	MSG msg;

	Assert (!cHpFreeze);
	/* only abort on galley and pageview. Except for print we also will
		abort so we can yield to spooler. Not in print preview which
		has messages flying around that abort us right off or for interrupt
        or even for print, which could cause formatlines. In these cases
        peek to yield but do not interrupt    */

             /* bz for fPrint we used to call FQueryAbortCheck but that could
                cause updateww which would call formatline and we
                are within displayfli. So only abort at the line
                level (let print code handle it). Peek to yield.
             */
	if (vfli.fPrint || vlm == lmPreview || vfDeactByOtherApp)
		PeekMessage((MSG far *)&msg, NULL, 0, 0, PM_NOREMOVE);
    else if (FMsgPresent (mtyLongDisp)) /* won't blink & cause formatline call */
		{
#ifdef BZ
		CommSz(SzShared("CheckGrAbortWin bitmap interrupted\n"));
#endif /* BZ */
		vfGrInterrupt = fTrue;
		EndGraphics(GRINTERRUPT);
		}
}


/*  dummies for sake of shared code */
/*  %%Function:  GetInfoPCX  %%Owner:  bobz       */

GetInfoPCX(hpc, rgco)
PICT		**hpc;
unsigned	rgco[];
{
	Assert (fFalse);
}


/*  %%Function:  GetInfoWin  %%Owner:  bobz       */

GetInfoWin(hpc, rgco)
PICT		**hpc;
unsigned	rgco[];
{
	PICT		*ppc;

	ppc = *hpc;

	ppc->vtExt.x = vpicFetch.bm.bmWidth;
	ppc->vtExt.y = vpicFetch.bm.bmHeight;
	ppc->cplIn = 1;
	/* note order different than in tiff files because windows bitmaps
		are already in the desired bit order and we don't have
		to flip 0 and 1  bz
	*/
	rgco[0] = coBlack;
	rgco[1] = coWhiteGR;

}


/* color mapping to dither pattern routines */
/*  %%Function:  DistFromCoCo   %%Owner:  bobz       */

DistFromCoCo (co1, co2)
CO      co1,
co2;
{
	int     coT;
	int	dRed, dGreen, dBlue;

#ifdef PCWORD
	SB	sbSav = sbCur;

	ResetSbCur();
#endif /* PCWORD */


	dRed = co1.red - co2.red;
	dGreen = co1.green - co2.green;
	dBlue = co1.blue - co2.blue;
	coT = dRed*dRed + dGreen*dGreen + dBlue*dBlue;


#ifdef PCWORD
	SetSbCur(sbSav);
#endif /* PCWORD */

	return (coT);
}


/*  %%Function:  IcoFromCoCcoRgco   %%Owner:  bobz       */



IcoFromCoCcoRgco (co, cco, rgco)
CO      co;
int     cco;
int     rgco[];
	/*
	* Return the index into rgco of the color nearest to co.
	*
* PCWORD only - Warning: sbCur == SbOfHp(hpvdvCur) during this procedure.
	*/
{
	int     icoCur,
	icoMac,
	icoNear,
	wdistNear,
	icoMaxM1,
	wdistCur;
	int     *pco;


#ifdef PCWORD
	PresetSbCur();		/* sbCur != sbApp */
#endif /* PCWORD */


	/* get number of colors                                         */
	icoMac = cco;
	icoMaxM1 = icoMac-1;

	Assert(icoMac > 0);

	/* check first for white (a very common color)			*/
	if (rgco[icoMaxM1] == (int) co) return icoMaxM1;
	/* for all colors in pallette, ...                              */
	for (pco = rgco,icoCur = 0; icoCur < icoMaxM1; ++icoCur,++pco) 
		{
		if ((int)(*pco) == (int) co) return (icoCur);
		}

	/* initialize nearest color to first color                      */
	wdistNear = DistFromCoCo(co, *rgco);
	icoNear = 0;

	for (pco=&rgco[icoMaxM1],icoCur=icoMaxM1; icoCur > 0; --icoCur,--pco) 
		{
		/* find nearest color                                   */
		if (wdistNear > (wdistCur = DistFromCoCo(co, *pco))) 
			{
			wdistNear = wdistCur;
			icoNear = icoCur;
			}
		}

	return (icoNear);
}



/* Convert from pixels to twips for the current device */
/*  %%Function:  ScaleToTwp  %%Owner:  bobz       */

ScaleToTwp(pPoint)
TWP	*pPoint;
{
	pPoint->v = NMultDiv (pPoint->v, czaInch, vfti.dypInch);
	pPoint->h = NMultDiv (pPoint->h, czaInch, vfti.dxpInch);
}


/*  %%Function:  IntAssertProc  %%Owner:  bobz       */

int IntAssertProc(unsval)
int unsval;
{
	Assert(unsval >= 0);
	return unsval;
}


/* get picture size from tiff file */

/*  %%Function:  GetGrSize  %%Owner:  bobz       */

GetGrSize(ft, pdya, pdxa)
int		ft;  	/* user-supplied type */
int		*pdya,	/* fill in with size or aspect ratio of picture */
*pdxa;
	/*
	* Purpose:	Determine the picture size of a graphic file.
	*/
{
	switch (ft)
		{
	default:
		Assert(FALSE);
		*pdxa = *pdya = 0;
		break;
	case ftTIFF:
			{

			/* this assumes stm has been set up before this call */
			FC fc;
			int ide, cde;
			long lx0 = -1L;
			long lx1 = -1L;
			long ly0 = -1L;
			long ly1 = -1L;

			BOOL fSwitchBytes;
			VPOINT vtImageSize;
			DEN den;
			int ctwp = czaInch;

			vtImageSize.y = -1;
			vtImageSize.x = -1;

			StmSetTIFF((FC)0);
			fSwitchBytes = (WFromStm() == 0x4d4d);

			GetPifdTIFF(&fc, fSwitchBytes);
			StmSetTIFF(fc);
			cde = WFromTIFF(fSwitchBytes);

			for (ide = 0; ide < cde; ide++)
				{
				DenFromTIFF(&den, fSwitchBytes);
				if (den.wTag > YResolution)
					break;
				switch (den.wTag)
					{
				default:
					break;
				case ImageLength:
					vtImageSize.y = den.w1Value;
					break;
				case ImageWidth:
					vtImageSize.x = den.w1Value;
					break;
				case ResolutionUnit:
					/* use czaInch unless none or centimeter is specified */
					if (den.wTag == 1)
						ctwp = 1;
					else  if (den.wTag == 3)
						ctwp = czaCm;
					else
						ctwp = czaInch;
					break;
				case XResolution:
				case YResolution:
					fc = StmGlobalFc();
					StmSetTIFF(den.lpValue);

					if (den.wTag == XResolution)
						{
						LFromTIFF(&lx0,fSwitchBytes);
						LFromTIFF(&lx1,fSwitchBytes);
						}
					else
						{
						LFromTIFF(&ly0,fSwitchBytes);
						LFromTIFF(&ly1,fSwitchBytes);
						}
					StmSetTIFF(fc);
					break;
					}
				}

			if (lx0 == -1L || lx1 == -1L || ly0 == -1L || ly1 == -1L)
					{
					/* if any or all of these are undefined, use screen
					    resolution to convert pixels to twips
					*/
					lx0 = (long) vfti.dxpInch;  /* default to pixels on current device */
					lx1 = 1L;
					ly0 = (long)vfti.dypInch;  /* default to pixels on current device */
					ly1 = 1L;
				    ctwp = czaInch;
		            } 
			/* catch missing or invalid required fields */
			/* longs must be > 0 && < 65535; no high bits set */

			if ((lx0 & 0xffff0000) || (lx1 & 0xffff0000) ||
					(ly0 & 0xffff0000) || (ly1 & 0xffff0000) ||
					/* mults can't overflow long result due to above tests */
			((unsigned long)(lx1 * ctwp) & 0xffff0000)||
					((unsigned long)(ly1 * ctwp) & 0xffff0000) ||
					vtImageSize.y <= 0 || vtImageSize.x <= 0)
				{
				*pdxa = *pdya = 0;
				break;	/* will fall into safety net below */
				}

			/* validity of casts assured by test above. Jodi claims real
				world tiff files will always be in this range.
			  */
			*pdxa = UMultDiv(vtImageSize.x,(unsigned)(lx1*ctwp),(unsigned)lx0);
			*pdya = UMultDiv(vtImageSize.y,(unsigned)(ly1*ctwp),(unsigned)ly0);

			if (ctwp == 1)  /* no units. Use values as aspect ratio only */
				if (*pdxa > *pdya)
					{
					*pdxa = 3 * czaInch;
					*pdya = NMultDiv (3 * czaInch, *pdya, *pdxa);
					}
				else
					{
					*pdya = 3 * czaInch;
					*pdxa = NMultDiv (3 * czaInch, *pdxa, *pdya);
					}

			break;
			}
		}
	/* safety net */
	if (*pdxa > czaMax || *pdxa <= 0)
		*pdxa = 3 * czaInch;
	if (*pdya > czaMax || *pdya <= 0)
		*pdya = 3 * czaInch;

}


/* ************* End Misc routines from PC Word needed by Opus ************ */



/* ************************************************************************* */
/* Interface to graphic converter libraries--based on PowerPoint's readers.c */
/* ************************************************************************* */

/* win.ini file format:
	[WWFilters]
	<desc>=<name>, <ext>, <option>
	i.e.,
	Lotus 1-2-3=LOTUSIMP.FLT, PIC
*/

#define cgrMax          10 /* limit not enforced */
#define cchMaxGrDesc    33
#define cchMaxGrOption  33
#define cchMaxGrName    13
#define cchMaxGrExt     4
#define cchMaxGrProf    (cchMaxGrExt+cchMaxGrName+cchMaxGrOption+4)

#define wProcInfo   1
#define wProcPict   2
#define wIdPict     2
#define wGiArg      2

struct GRIB
	{ /* Graphics Reader Information Block */
	HANDLE hPref;
	CHAR szName[cchMaxGrName];
	CHAR szExt[cchMaxGrExt];
};

struct GRFS
	{ /* Graphics Reader File Struct */
	int     w1;
	char    szExt[4];
	int     w2;
	char    szFullName[124];
	long    l;
};

struct GRPI
	{ /* Graphics Reader Picture Info */
	HANDLE  hMF;
	struct RC rc;   /* bounding rect in MF units */
	int     cInch;  /* MF units per inch */
};



/* I N I T  G R I B */
/*  Read the win.ini entries for Graphics Readers creating a PLEX of 
information about them.
*/
/*  %%Function:  InitGrib  %%Owner:  bobz       */

InitGrib()
{
	CHAR *szKey;
	int cch;
	int igrib = 0;
	CHAR *pchSection = SzFrame("WWFilters");
	struct GRIB grib;
	CHAR rgchProfile[cchMaxGrDesc*cgrMax];

	if (vhplgrib != -1 || (vhplgrib = HplInit2(sizeof(struct GRIB), cbPLBase, 
			2, fFalse)) == hNil)
		{
#ifdef DREADPIC
	    if (vhplgrib == hNil)
	        CommSz(SzShared("InitGrib null vhplgrib\n"));
#endif /* DREADPIC */
        return;
        }
#ifdef DREADPIC
	CommSz(SzShared("InitGrib: initializing\r\n"));
#endif /* DREADPIC */

	GetProfileString(pchSection, NULL, szEmpty, rgchProfile,
			sizeof(rgchProfile));

	for (szKey = rgchProfile; (cch = CchSz(szKey)) > 1; szKey += cch)
		if (FGribFromProfile(pchSection, szKey, &grib))
			if (!FInsertInPl(vhplgrib, igrib++, &grib))
				return;

	if ((*vhplgrib)->iMac == 0)
		FreePhpl(&vhplgrib);
}


/* P C H  C O P Y  T O K E N */
/*  %%Function:  PchCopyToken  %%Owner:  bobz       */

CHAR *PchCopyToken(pchDest, pchSrc, cchMax)
CHAR *pchDest, *pchSrc;
int cchMax;
{
	CHAR *pchMax = pchDest + cchMax;
	CHAR ch;
	BOOL fBegin = fTrue;

	/* skip leading white space the grab the next token (deliminated by ',') */
	while ((ch = *pchSrc) != 0)
		{
		pchSrc++;
		if (ch == ',')
			break;
		if (pchDest < pchMax && (!fBegin || ch != ' '))
			{
			fBegin = fFalse;
			*pchDest++ = ch;
			}
		}

	/* backup over trailing spaces */
	if (!fBegin)
		while (*(pchDest-1) == ' ')
			pchDest--;

	*pchDest = 0;
	return pchSrc;
}


csconst CHAR stExtFlt[] = StKey(".FLT",szDefFltExt);

/* F  G R I B  F R O M  P R O F I L E */
/* read the entry for szKey from profile and fill out pgrib for it.  return
fTrue if pgrib is useable.
*/
/*  %%Function:  FGribFromProfile  %%Owner:  bobz       */

BOOL FGribFromProfile(szSection, szKey, pgrib)
CHAR *szSection, *szKey;
struct GRIB *pgrib;
{
	HANDLE hLib;
	HANDLE hT;
	BOOL fReturn = fFalse;
	FARPROC lpfnGetInfo;
	CHAR *pch;
	CHAR szProfile[cchMaxGrProf];
	CHAR szOption[cchMaxGrOption];
	struct FNS fns;

	SetBytes(pgrib, 0, sizeof(struct GRIB));

	GetProfileString(szSection, szKey, szEmpty, szProfile, sizeof(szProfile));

	pch = PchCopyToken(pgrib->szName, szProfile, cchMaxGrName);
	pch = PchCopyToken(pgrib->szExt, pch, cchMaxGrExt);
	PchCopyToken(szOption, pch, cchMaxGrOption);


	/* assure it has an extension */
	SzToStInPlace(pgrib->szName);
	StFileToPfns(pgrib->szName, &fns);
	if (fns.stExtension[0] <= 1 && pgrib->szName[0] < cchMaxGrName-4)
		CopyCsSt(stExtFlt, fns.stExtension);
	PfnsToStFile(&fns, pgrib->szName);
	StToSzInPlace(pgrib->szName);

#ifdef DREADPIC
	CommSzSz(SzShared("FGribFromProfile: checking "), szKey);
	CommSzSz(SzShared("\t\tname = "), pgrib->szName);
	CommSzSz(SzShared("\t\text = "), pgrib->szExt);
#endif /* DREADPIC */

	ShrinkSwapArea();

	if (pgrib->szName[0] != 0 && (hLib = HOurLoadLibrary(pgrib->szName, NULL))
			>= 32)
		{
		if ((lpfnGetInfo = GetProcAddress(hLib, MAKEINTRESOURCE(wProcInfo)))
				!= NULL)
			{
            vrf.fInExternalCall = fTrue;
            fReturn = (((*lpfnGetInfo)(wGiArg, (LPSTR)szOption, (LPSTR)&pgrib->hPref,
					(LPSTR)&hT)) == wIdPict);
            vrf.fInExternalCall = fFalse;
            }
		FreeLibrary(hLib);
		}

	GrowSwapArea();

#ifdef DREADPIC
	CommSzNum(SzShared("\t\tfReturn = "), fReturn);
#endif /* DREADPIC */

	return fReturn;
}




/* H  R E A D  S T  P I C T */
/* try to find a converter for and read in stPict.
*/
/*  %%Function:  HReadStPict  %%Owner:  bobz       */

HANDLE HReadStPict(stPict, prc, ph)
CHAR *stPict;
struct RC *prc;
HANDLE *ph;
{
	int igrib = iNil;
	HANDLE hPict;
	CHAR szExt[cchMaxGrExt];
	struct GRIB grib;

	AssertH(vhplgrib);

#ifdef DREADPIC
	CommSzSt(SzShared("HReadStPict: trying to read "), stPict);
#endif /* DREADPIC */

	SzExtFromSt(stPict, szExt);

	while ((igrib = IgribFromSzExt(igrib, szExt, &grib)) != iNil)
		if ((hPict = HReadPgribSt(&grib, stPict, prc, ph)) != NULL)
			return hPict;

	return NULL;
}


/* S Z  E X T  F R O M  S T */
/*  %%Function:  SzExtFromSt  %%Owner:  bobz       */

SzExtFromSt(st, szExt)
CHAR *st, *szExt;
{
	struct FNS fns;
	StFileToPfns(st, &fns);
	bltb(&fns.stExtension[2], szExt, 3);
	szExt[fns.stExtension[0]-1] = 0;
}


/* I G R I B  F R O M  S Z  E X T */
/*  %%Function:  IgribFromSzExt  %%Owner:  bobz       */

IgribFromSzExt(igrib, szExt, pgrib)
int igrib;
CHAR *szExt;
struct GRIB *pgrib;
{
	int igribMac = (*vhplgrib)->iMac;
	while (++igrib < igribMac)
		{
		GetPl(vhplgrib, igrib, pgrib);
		if (FEqNcSz(szExt, pgrib->szExt))
			return igrib;
		}
	return iNil;
}


/* H  R E A D  P G R I B  S T */
/* Read st using converter specified by pgrib.  Return handle to metafile or
NULL.
*/
/*  %%Function:  HReadPgribSt  %%Owner:  bobz       */

HANDLE HReadPgribSt(pgrib, st, prc, ph)
struct GRIB *pgrib;
CHAR *st;
struct RC *prc; /* to return grpi.rc */
HANDLE *ph; /* to return hMF */
{
	HANDLE hLib;
	HANDLE hPict = NULL;
	HDC hdc = NULL;
	FARPROC lpfnReadPict;
	struct GRFS grfs;
	struct GRPI grpi;
    int wRet;


#ifdef DEBUG
	if (vdbs.fDumpPicInfo)
		CommSzSz(SzShared("HReadPgribSt: loading "), pgrib->szName);
#endif /* DEBUG */

	if (st[0] >= 124 || vpri.hszPrDriver == NULL)
		{
		ReportSz("Graphics convertors require attached printer ");
		return NULL;
		}
	ShrinkSwapArea();

	if ((hLib = HOurLoadLibrary(pgrib->szName, NULL)) >= 32)
    	{
#ifdef DEBUG
	if (vdbs.fDumpPicInfo)
		{
		CommSzSz(SzShared("HReadPgribSt: szPrDriver "), **vpri.hszPrDriver);
		CommSzSz(SzShared("HReadPgribSt: szPrinter "), **vpri.hszPrinter);
		CommSzSz(SzShared("HReadPgribSt: szPrPort "), **vpri.hszPrPort);
		}
#endif /* DEBUG */

        if (FEqNcSz(*vpri.hszPrPort, szNone))
    		hdc = CreateIC ( (LPSTR) **vpri.hszPrDriver,
    				(LPSTR) **vpri.hszPrinter,
    				(LPSTR) **vpri.hszPrPort, (LPSTR) NULL );
    	else
    		hdc = CreateDC ( (LPSTR) **vpri.hszPrDriver,
    				(LPSTR) **vpri.hszPrinter,
    				(LPSTR) **vpri.hszPrPort, (LPSTR) NULL );

	    if (hdc != NULL)
	        LogGdiHandle(hdc, 1046);
        }
#ifdef DEBUG
	else
		ReportSz("Graphics convertor not found/not loaded ");

#endif /* DEBUG */

#ifdef DEBUG
	if (vdbs.fDumpPicInfo)
		{
		CommSzNum(SzShared("HReadPgribSt: hlib "), hLib);
		CommSzNum(SzShared("HReadPgribSt: hdc "), hdc);
		}
#endif /* DEBUG */

	if (hLib >= 32 && hdc != NULL &&
        (lpfnReadPict = GetProcAddress(hLib, MAKEINTRESOURCE(wProcPict)))
            != (FARPROC)NULL)
		{

		SetBytes(&grfs, 0, sizeof(grfs));
		CchCopySz(pgrib->szExt, grfs.szExt);
		StToSz(st, grfs.szFullName);

        vrf.fInExternalCall = fTrue;
		wRet = (*lpfnReadPict)(hdc, (LPSTR)&grfs, (LPSTR)&grpi,
				pgrib->hPref);
        vrf.fInExternalCall = fFalse;
		if (!wRet && grpi.hMF != NULL)
			{
			METAFILEPICT mfp;
			LPCH lpch;
			unsigned uMul;

			if ((hPict = OurGlobalAlloc(GMEM_MOVEABLE, (DWORD)sizeof(mfp))) != NULL)
				{
				lpch = GlobalLock(hPict);
				Assert(lpch != NULL); /* DavidW says won't fail! */

				SetBytes(&mfp, 0, sizeof(mfp));
				*ph = mfp.hMF = grpi.hMF;
				mfp.mm = MM_ANISOTROPIC;
				uMul =  UMultDiv(czaInch, 1000, czaCm);

				mfp.xExt = UMultDiv(abs(grpi.rc.xpRight - grpi.rc.xpLeft), 
						uMul, grpi.cInch);
				mfp.yExt = UMultDiv(abs(grpi.rc.ypBottom - grpi.rc.ypTop), 
						uMul, grpi.cInch);
				/* overflow check set to 3" */
				if (mfp.xExt == 0xFFFF || mfp.yExt == 0xFFFF)
					mfp.xExt = mfp.yExt = 3 * uMul;

				*prc = grpi.rc; /* will need for window org, ext */

				bltbx((LPCH)&mfp, lpch, sizeof(mfp));
				GlobalUnlock(hPict);
#ifdef DREADPIC
				CommSz(SzShared("\tconverting metafile...\r\n"));
				CommSzNum(SzShared("\t\tgrpi.cInch = "), grpi.cInch);
				CommSzRgNum(SzShared("\t\tgrpi = "), &grpi, sizeof(grpi)/2);
				CommSzRgNum(SzShared("\t\tmfp = "), &mfp, sizeof(mfp)/2);
#endif /* DREADPIC */
				}
			}
		}

#ifdef DEBUG
	if (vdbs.fDumpPicInfo)
		{
		CommSzNum(SzShared("ret from filter: "), wRet);
		CommSzLong(SzShared("lpfnReadPict: "), lpfnReadPict);
		}
#endif /* DEBUG */

#ifdef DEBUG  // dump out metafile load failure reason
	if (wRet)
		{
		int wRet2 = wRet - 5300;
		switch (wRet2)
			{
			case 1:
				ReportSz("IE_NOT_MY_FILE   generic not my file error ");
				break;
			case 2:
				ReportSz("IE_TOO_BIG       bitmap or pict too big error ");
				break;
			case 3:
				ReportSz("IE_DUMB_BITMAP   bitmap all white");
				break;
			case 4:
				ReportSz("IE_BAD_VCHAR     bad vchar in ImportString");
				break;
			case 5:
				ReportSz("IE_BAD_TOKEN     illegal wp token");
				break;
			case 6:
				ReportSz("IE_NO_VERIFY     failed to verify imported story");
				break;
			case 7:
				ReportSz("IE_UNKNOWN_TYPE  unknown file type");
				break;
			case 8:
				ReportSz("IE_NOT_WP_FILE   Not a %s file.");
				break;
			case 9:
				ReportSz("IE_BAD_FILE_DATA current file data is bad");
				break;
			case 10:
				ReportSz("IE_IMPORT_ABORT  import abort alert");
				break;
			case 11:
				ReportSz("IE_MEM_FULL      ran out of memory during import");
				break;
			case 13:
				ReportSz("IE_META_TOO_BIG  metafile too big");
				break;
			case 15:
				ReportSz("IE_MEM_FAIL      couldn't lock memory during import");
				break;
			case 17:
				ReportSz("IE_NOPICTURES    no pictures in this file");
				break;
			case 18:
				ReportSz("IE_NO_FILTER     expected filter not found");
				break;
			case 83:
				ReportSz("IE_BAD_METAFILE  inconsistant metafile data");
				break;
			case (0xCCCC - 5300):
				ReportSz("IE_BAD_METAFILE  inconsistant metafile data");
				break;
			default:
				{
				CommSzNum(SzShared("ret from filter: "), wRet);
				ReportSz("Graphics convertor unknown failure reason");
				break;
				}
			}
		}
	else if (lpfnReadPict == (FARPROC)NULL)
		{
		ReportSz("Graphics convertor entry point not found");
		}

#endif /* DEBUG */

	if (hdc != NULL)
		{
		UnlogGdiHandle(hdc, 1046);
		DeleteDC(hdc);
		}
	if (hLib >= 32)
		FreeLibrary(hLib);

	GrowSwapArea();

	return hPict;
}


/* H  O U R  L O A D  L I B R A R Y */
/* Either sz or st is provided */
/*  %%Function:  HOurLoadLibrary  %%Owner:  bobz       */

HANDLE HOurLoadLibrary(sz, st)
CHAR *sz, *st;
{
	HANDLE hLib;
	CHAR rgch[ichMaxFile];
	CHAR stzOpen[ichMaxFile+1];

	if (sz == NULL)
		{
		StToSz(st, rgch);
		sz = rgch;
		}
	else
		{
		SzToSt(sz, rgch);
		st = rgch;
		}

	if (!FFindFileSpec(st, stzOpen, grpfpiUtil, nfoNormal))
		return LoadLibrary(sz);

	stzOpen[stzOpen[0]+1] = 0;
	return LoadLibrary(stzOpen+1);
}


