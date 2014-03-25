/* ****************************************************************************
**
**      COPYRIGHT (C) 1987, 1988 MICROSOFT
**
** ****************************************************************************
*
*  Module: pic2.c ---- Misc picture routines
*
**
** REVISIONS
**
** Date         Who    Rel Ver     Remarks
** 11/16/87     bobz               split off from pic.c 
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
#define NOCOLOR
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOFONT
#define NOMB
#define NOMENUS
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
#include "print.h"
#include "dlbenum.h"
#include "fkp.h"
#include "field.h"
#include "opuscmd.h"
#include "error.h"

#include "core.h"
#include "debug.h"

#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#include "pict.hs"
#include "pict.sdm"
#include "inspic.hs"
#include "inspic.sdm"
#include "doslib.h"

#ifdef PROTOTYPE
#include "pic2.cpt"
#endif /* PROTOTYPE */

#ifdef PCODE
typedef long ul;   /* no unsigned longs in CS compiler */
#else
typedef unsigned long ul;
#endif
/* these is another copy of this in ddeclnt.c */
csconst CHAR szCSMergeFormat[] = SzSharedKey(" \\* mergeformat",DdeMergeFormat);

extern CHAR szTif[];

TMC TmcGetPicStFile ();


/* E X T E R N A L S */
extern BOOL		fElActive;
extern int              fnFetch;
extern int              wwCur;
extern struct WWD       **hwwdCur;
extern struct FLI       vfli;
extern struct PIC       vpicFetch;
extern struct CHP       vchpFetch;
extern struct SEL       selCur;
extern struct FMTSS     vfmtss;
extern struct FTI       vfti;
extern struct FTI       vftiDxt;
extern struct MERR      vmerr;
extern struct SCI       vsci;
extern struct PREF      vpref;
extern struct FKPD      vfkpdText;
extern struct PRI       vpri;
extern CHAR		stDOSPath[];
extern BOOL		vfFileCacheDirty;
extern CHAR		szEmpty[];
extern BOOL		vfRecording;
extern CHAR HUGE    *vhpchFetch;
extern CP           vcpFetch;
extern int          vdocFetch;

#define szFPWidthDef    SzFrameKey(" W x ",FPWidth)
#define szFPHeightDef   SzFrameKey(" H",FPHeight)


/* brclNone is at the end rather than at 0 but the drop list wants
	none at index 0
*/
#define IFromPicBrcl(brcl) (((brcl) >= brclSingle && (brcl) < brclNone) ? \
						(brcl + 1) : (brcl == brclNone ? 0 : brclInval))
#define PicBrclFromI(i)  ((((unsigned) (i)) == 0) ? brclNone : \
						(((unsigned) (i)) <= brclNone) ? (i - 1) : brclInval)



/***************************************/
/* C m d  P i c t u r e */
/*  %%Function:  CmdPicture  %%Owner:  bobz       */

CMD CmdPicture(pcmb)
CMB * pcmb;
{
	CABFORMATPIC * pcabFp;
	HCAB hcabFp;
	char * pch;
	int cch;
	char sz [60];  /* arbitrary size */
	CP cpImport;
	struct CA caT;

	/* we assume that chp is for a picture. SelCur.chp
		won't have the fSpec stuff though, so fetch
		and use vchpFetch.
	
			In case of an import field, we have to get the cp
			of the actual picture character.
	*/
	AssertDo(FCaIsGraphics(&selCur.ca, selCur.ww, &cpImport));
	if (cpImport != cpNil)  /* import field - use pic char for ca */
		{
		caT.cpFirst = cpImport;
		caT.cpLim = cpImport + 1;
		caT.doc = selCur.doc;
		}
	else
		caT = selCur.ca;

	FetchCpAndParaCa(&caT, fcmProps);
	Assert(vchpFetch.fSpec);
	FetchPe(&vchpFetch, caT.doc, caT.cpFirst);

	pcmb->pv = &caT;	// for FCheckFmtPic

	if (pcmb->fDefaults)
		{
		struct CHP chp;
		unsigned mxRemainder;
		pcabFp = (CABFORMATPIC *) *pcmb->hcab;

		/* Note:
			FetchCp never sets fnPic since it is possible for
			fnPic to change during the lifetime of the fetch.
			All users of pic chps are required to set fnPic to
			fnFetch. This is one case where it is acceptable to
			modify vchpFetch without trashing the fetch. FetchPe
			is the only real user of a pic chp, so it always sets
			chp.fnPic to fnFetch    bz
		*/


		pcabFp = (CABFORMATPIC *) *pcmb->hcab;
		pcabFp->iPicBrcl = IFromPicBrcl(vpicFetch.brcl);
		pcabFp->wScaleMx = UDiv(vpicFetch.mx, PctTomx100Pct, &mxRemainder);
		pcabFp->wScaleMy = UDiv(vpicFetch.my, PctTomy100Pct, &mxRemainder);
		pcabFp->wCropTop = vpicFetch.dyaCropTop;
		pcabFp->wCropLeft = vpicFetch.dxaCropLeft;
		pcabFp->wCropBottom = vpicFetch.dyaCropBottom;
		pcabFp->wCropRight = vpicFetch.dxaCropRight;

		/* text "Original Size " is already in cab. Build size string */
		pch = sz;
		CchExpZa(&pch, vpicFetch.dxaGoal, vpref.ut, sizeof(sz), fTrue);
		pch += CchCopySz(szFPWidthDef, pch);
		CchExpZa(&pch, vpicFetch.dyaGoal, vpref.ut, sizeof(sz), fTrue);
		pch += CchCopySz(szFPHeightDef, pch);

		Assert ((pch - sz) < sizeof(sz));

		if (!FSetCabSz(pcmb->hcab, sz, Iag(CABFORMATPIC, hszOrigSize)))
			return cmdNoMemory;

#ifdef XBZTEST
		CommSzNum(SzShared("TmcFillCabFp: caT.doc: "), caT.doc);
		CommSzLong(SzShared("TmcFillCabFp: caT.cpFirst: "), caT.cpFirst);
		CommSzRgNum(SzShared("TmcFillCabFp: cab ag values: "), pcabFp,
				IagMacFromHcab(pcmb->hcab));
#endif
		}



	if (pcmb->fDialog)
		{
		char dlt [sizeof (dltFormatPic)];

		BltDlt(dltFormatPic, dlt);

		switch (TmcOurDoDlg(dlt, pcmb))
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			return cmdError;
#endif

		case tmcError:
			return cmdNoMemory;

		case tmcCancel:
			return cmdCancelled;

		case tmcOK:
			break;
			}
		}

	if (pcmb->fCheck)
		{
		if (!FCheckFmtPic(pcmb, fFalse  /* fDialog */))
			return cmdError;
		}

	if (pcmb->fAction)
		{
		Assert( selCur.fGraphics );
		FetchCpAndParaCa(&caT, fcmProps);
		Assert(vchpFetch.fSpec);
		FetchPe(&vchpFetch, caT.doc, caT.cpFirst);

		pcabFp = (CABFORMATPIC *) *pcmb->hcab;
		ChangePicFrame(NULL, PicBrclFromI(pcabFp->iPicBrcl),
				pcabFp->wScaleMx * PctTomx100Pct  /* mx */,
				pcabFp->wScaleMy * PctTomy100Pct  /* my */,
				pcabFp->wCropTop,
				pcabFp->wCropLeft,
				pcabFp->wCropBottom,
				pcabFp->wCropRight);
		}

	return cmdOK;
}


/***************************************/
/* C m d  I n s   P i c t u r e */
/*  %%Function:  CmdInsPicture  %%Owner:  bobz       */

CMD CmdInsPicture(pcmb)
CMB * pcmb;
{
/* insert an empty graphics frame */
	CMD cmd;
	struct CA caPic, caRM;
	HDC  hDCMeta;
	HANDLE hMeta = NULL;
	HANDLE hData = NULL;
	METAFILEPICT mfp;
	LPCH lpch;
	int fOk;
	TMC tmc;
	CHAR szFilter[6];
	CHAR stDOSPathOld[ichMaxFile + 1];
	CHAR stFile[ichMaxBufDlg];


	if (pcmb->fDefaults)
		{
		int cch;
		((CABINSPIC *)PcabFromHcab(pcmb->hcab))->iDirectory = uNinchList;

		CchCopySz(SzShared("*.*"), szFilter); /* since winword 1.1 */

		if (!FSetCabSz(pcmb->hcab, szFilter, Iag(CABINSPIC, hszFile)))
			return cmdNoMemory;
		}

	if (pcmb->fDialog)
		{
		char dlt [sizeof (dltInsPic)];
		/* To check if the path was changed by the file name dialog. */
		CopySt(stDOSPath, stDOSPathOld);
		BltDlt(dltInsPic, dlt);

		tmc = TmcOurDoDlg(dlt, pcmb);
		/* Path has been changed by the dialog, update window titles. */
		if (FNeNcSt(stDOSPath, stDOSPathOld))
			{
			vfFileCacheDirty = fTrue;
			UpdateTitles(); /* changes window menu, vhsttbWnd and captions */
			if (vfRecording)
				RecordChdir(stDOSPath);
			}

		switch (tmc)
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			return cmdError;
#endif

		case tmcError:
			return cmdError;

		case tmcCancel:
			return cmdCancelled;

		case tmcOK:
		case tmcNewPic:
			if (vfRecording)
				{
				FRecordCab(pcmb->hcab, IDDInsPic, pcmb->tmc, 
						pcmb->bcm);
				}
			break;
			}
		}

	if (pcmb->fCheck && tmc == tmcOK)
		{
		if (!FTermFile(pcmb, Iag(CABINSPIC, hszFile), tmcNull,
				fFalse, fFalse, nfoPic))
			return cmdError;
		}

	if (pcmb->fAction)
		{
		if (pcmb->tmc == tmcOK)
			{
			CHAR stNorm[ichMaxFile];
			int cch;

			*stFile = 0;
			GetCabSt(pcmb->hcab, stFile, ichMaxFile, Iag(CABINSPIC, hszFile));
			/* this should only be able to fail in a macro */
			if (!FNormalizeStFile(stFile, stNorm, nfoPic))
				{
				ErrorEid(eidBadFileDlg," CmdInsPicture");
				return cmdError;
				}

			StartLongOp();
			DoubleBSSt(stNorm, stFile);
			cch = *stFile;
			StToSzInPlace(stFile);

			/* add merge format switch */
			if (cch + sizeof(szCSMergeFormat) <= sizeof(stFile))
				bltbx(szCSMergeFormat, (CHAR FAR *)&stFile[cch],
						sizeof(szCSMergeFormat));

			/* this function sets undo and autodelete */
			cmd = CmdInsFltSzAtSelCur(fltImport, stFile, imiInsPicture, 
					fTrue, fTrue, fTrue);
			PushInsSelCur();
			EndLongOp(fFalse /*fAll*/);
			return cmd;
			}

		if (!FSetUndoBefore(imiInsPicture, uccPaste))
			return cmdCancelled;

		if ((cmd = CmdAutoDelete(&caRM)) != cmdOK)
			return cmd;

		Assert (pcmb->tmc == tmcNewPic);

		PcaPoint(&caPic, caRM.doc, caRM.cpFirst);

		/* create 1" square metafile */

		if ((hDCMeta = CreateMetaFile( (LPSTR) NULL)) == hNil)
			goto ErrRet;  /* memory DC */
		LogGdiHandle(hDCMeta, 1086);

		if ((hMeta = CloseMetaFile(hDCMeta)) == hNil)
			goto ErrRet;
		UnlogGdiHandle(hDCMeta, 1086);
		LogGdiHandle(hMeta, 1087);

		if ( ((hData=OurGlobalAlloc(GMEM_MOVEABLE,
				(long)sizeof(METAFILEPICT) ))==NULL) ||
				((lpch=GlobalLock( hData ))==NULL))
			{
			goto ErrRet;
			}
		else
			{
			mfp.hMF = hMeta;
			mfp.mm = MM_ANISOTROPIC;
			mfp.xExt = 0;
			mfp.yExt = 0;
			bltbx( (LPCH)&mfp, lpch, sizeof(METAFILEPICT) );
			GlobalUnlock( hData );
			}


		/* load metafile into document */
		FetchCpAndParaCa(&caPic, fcmProps);
		fOk = FReadPict (&caPic, CF_METAFILEPICT,
				hData, 0 /* cbInitial */,
				fTrue /* fEmptyPic */,
				&vchpFetch  /* pchp */,	NULL /* prcWinMF */);

		GlobalFree( hData );
		UnlogGdiHandle(hMeta, 1087);
		DeleteMetaFile(hMeta);
		hMeta = hData = NULL;

		if (!fOk)
			{
ErrRet:
			if (hData != NULL)
				GlobalFree( hData );
			if (hMeta != NULL)
				{
				UnlogGdiHandle(hMeta, 1087);
				DeleteMetaFile(hMeta);
				}
			Beep();
			return cmdError;
			}

		/* make document dirty. if it's a subdoc, FDirtyDoc will notice
			and declare that the mother doc is dirty. */

		PdodDoc(selCur.doc)->fFormatted = fTrue;

		caPic.cpLim++;
		SelectIns(&selCur, caPic.cpLim);
		PushInsSelCur();
		caRM.cpLim += DcpCa(&caPic);
		SetUndoAfter(&caRM);
		}

	return cmdOK;
}


/********************************/
/* D l g  f  F o r m a t  P i c */
/* SDM dialog function for Format Picture dialog.
	Special interactions:

	o   when fInLine is ON, the ojc radio buttons are meaningless
		and should be disabled.
*/

csconst rgtmcPic[] =
{ 
	tmcScaleMx,
			tmcScaleMy,
			tmcCropTop,
			tmcCropLeft,
			tmcCropBottom,
			tmcCropRight };


#define itmcMacPic  (sizeof(rgtmcPic)/sizeof(int))


/*  %%Function:  FDlgFormatPic  %%Owner:  bobz       */


BOOL FDlgFormatPic(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	TMC rgtmc[itmcMacPic];
	CMB * pcmb = PcmbDlgCur();

	switch (dlm)
		{
	case dlmChange:
			{
			Assert (tmc == tmcScaleMx || tmc == tmcScaleMy ||
					tmc == tmcCropTop || tmc == tmcCropLeft ||
					tmc == tmcCropBottom || tmc == tmcCropRight);

			bltbyte(rgtmcPic, rgtmc, sizeof(rgtmcPic));
			GrayRgtmcOnBlank(rgtmc, itmcMacPic, tmcOK);
			break;
			}
	case dlmTerm:
		if (tmc == tmcOK)
			{
			HCAB hcab;
			if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled)
				return fFalse;
			if (hcab == hcabNull)
				{
				/* dialog will end without needing to call EndDlg(tmcError) */
				return (fTrue);
				}
			return FCheckFmtPic(pcmb, fTrue  /* fDialog */);
			}
		/* break; */
		}

	return fTrue;
}


/*  %%Function:  FCheckFmtPic  %%Owner:  bobz       */
/*   Sets up a cmb so it can call FCheckFmtPic */
FCheckPic(ppic, pca)
struct PIC *ppic;
struct CA *pca;
{
	CMB cmb;
	CABFORMATPIC cab;
	CABFORMATPIC *pcab;
	unsigned mxRemainder;

	/* Note vpicFetch must be set up for this to work. It may be different
	   from pic
	*/
	cab.wScaleMx = UDiv(ppic->mx, PctTomx100Pct, &mxRemainder);
	cab.wScaleMy = UDiv(ppic->my, PctTomy100Pct, &mxRemainder);
	cab.wCropTop = ppic->dyaCropTop;
	cab.wCropLeft = ppic->dxaCropLeft;
	cab.wCropBottom = ppic->dyaCropBottom;
	cab.wCropRight = ppic->dxaCropRight;

	pcab = &cab;
	cmb.hcab = &pcab;

	cmb.pv = pca;

	return (FCheckFmtPic(&cmb, fFalse  /* fDialog */));
}


/*  %%Function:  FCheckFmtPic  %%Owner:  bobz       */

FCheckFmtPic(pcmb, fDialog)
CMB * pcmb;
BOOL fDialog;
{
	/* Note vpicFetch must be the pic props are applied to for this to work.
	   Since rtfin kind of hacks its way in here, we can't assert here
	   anything about vchpFetch or vcpFetch. If pcmb->pv is not NULL,
	   it is the ca for the picture. We can assure that fetching is set
	   up properly in that case. It is currently NULL only for the rtfin
	   call, which is made before the picture is put into the doc.
	*/
	CABFORMATPIC * pcabFp;
	int dxa, dya;
	struct FTI *pfti = vfli.fFormatAsPrint ? &vftiDxt : &vfti;

	if (pcmb->pv != NULL)
		{
		// we can check to be sure vpicFetch is set up correctly in this case
		struct CA *pca = pcmb->pv;
		if (vdocFetch != pca->doc || vcpFetch != pca->cpFirst)
			{
			ReportSz("FCheckFmtPic vpicFetch moved, reestablishing");
			FetchCpAndParaCa(pca, fcmProps+fcmChars);

   			Assert(vchpFetch.fSpec && *vhpchFetch == chPicture);

			FetchPe(&vchpFetch, pca->doc, pca->cpFirst);
			}
		}

	pcabFp = (CABFORMATPIC *) *pcmb->hcab;

	/* scaled / cropped dimensions must be > minimum size. Note
		that cropping is based on the UNSCALED pic size
		and scaling on cropped size    */

	/* note that the mx, my are parsed items and so
		already check for >=0.
		Since we are comparing int values, any value > 32767
		will show up as negative, so so be rejected as too
		big, so this handles < min or > max. dzaPicMax is not
		quite 32767, so add the test for  dzaMax < x <= 32767
	
		note also that the goal values are really za units, i.e.,
		twips, not pixels (zp).
		*/
	/* cropped sizes */
	dxa = (vpicFetch.dxaGoal - pcabFp->wCropLeft - pcabFp->wCropRight);
	dya = (vpicFetch.dyaGoal - pcabFp->wCropTop - pcabFp->wCropBottom);

	if (pcabFp->wCropLeft ||
			pcabFp->wCropRight ||
			pcabFp->wCropTop ||
			pcabFp->wCropBottom)
		{
		if (dxa  < dxaPicMin || dxa > dxaPicMax ||
				dya  < dyaPicMin || dya > dyaPicMax)
			{
			ErrorEid(eidBadCrop, "");
			if (fDialog)
				{
				SetTmcTxs(tmcCropTop, 
						TxsOfFirstLim(0, ichLimLast));
				}
			return fFalse;
			}
		}

	if ( pcabFp->wScaleMx != 100 || pcabFp->wScaleMy != 100)
		{
		/* don't have a parse funct available to add top range check
			have to check or 10 x mx/my could overflow unsigned  */

		if ( pcabFp->wScaleMx > mzPicMax || pcabFp->wScaleMy > mzPicMax)
			{
			RangeError(1, mzPicMax, fFalse  /* fZa */, vpref.ut);
			if (fDialog)
				{
				SetTmcTxs(pcabFp->wScaleMx > mzPicMax ?
						tmcScaleMx : tmcScaleMy, 
						TxsOfFirstLim(0, ichLimLast));
				}

			return fFalse;
			}

		/* test thse rects: the scaled/cropped rect,  the
		scaled uncropped rect in twips, screen pixels
		and printer pixels for overflow in an int.
		*/

		/* scale the cropped rect */
		if (!FScaleInRange(dxa, dya, pcabFp->wScaleMx,
				pcabFp->wScaleMy, dxaPicMin, dxaPicMax, dyaPicMin,
				dyaPicMax, fDialog))
			return (fFalse);

		/* scale the uncropped rect */
		if (!FScaleInRange(vpicFetch.dxaGoal, vpicFetch.dyaGoal, pcabFp->wScaleMx,
				pcabFp->wScaleMy, dxaPicMin, dxaPicMax, dyaPicMin,
				dyaPicMax, fDialog))
			return (fFalse);

		if (vpicFetch.mfp.mm == MM_BITMAP ||
				vpicFetch.mfp.mm == MM_TIFF)
			{
			GetBitmapSize(&dxa, &dya);
			}
		else

			/* use calculated goal size in main device units for metafiles.
				Calculate in printer pixels, and compute xp's using nonrounding
				mechanism for dispasprint so we avoid pageview screen pushout.
			*/
			{
			Assert (vpicFetch.mfp.mm <= MM_META_MAX);
			dxa = NMultDiv (vpicFetch.dxaGoal,
					pfti->dxpInch, czaInch);
			Assert (dxa != 0x7FFF);  /* overflow should be caught before now */
			dya = NMultDiv (vpicFetch.dyaGoal,
					pfti->dypInch, czaInch);
			Assert (dya != 0x7FFF);
			}

		/* check the scaled pixels rect for overflow */
		if (!FScaleInRange(dxa, dya, pcabFp->wScaleMx,
				pcabFp->wScaleMy,
				1, 32767, 1, 32767, fDialog))
			return (fFalse);

		/* convert to screen units */
		/* for the dispas print case */
		if (vfli.fFormatAsPrint)
			{
			dxa = (int)((long)dxa * (long)vfti.dxpInch /
					(long) vftiDxt.dxpInch);
			dya = (int)((long)dya * (long)vfti.dypInch /
					(long) vftiDxt.dypInch);

			/* check the scaled screen pixels rect */
			if (!FScaleInRange(dxa, dya, pcabFp->wScaleMx,
					pcabFp->wScaleMy,
					1, 32767, 1, 32767, fDialog))

				return (fFalse);
			}
		}

	return fTrue;
}


/*  %%Function:  FScaleInRange  %%Owner:  bobz       */


FScaleInRange(dx, dy, wScaleMx, wScaleMy, dxMin, dxMax, dyMin, dyMax, fDialog)
int dx, dy;
unsigned wScaleMx, wScaleMy;
int dxMin, dxMax, dyMin, dyMax;
BOOL fDialog;
{

	int dxScale, dyScale;

	/* test various pic sizes for 22" (czaMax/dzaPicMax)limit
	*/

	if ( (dxScale = NMultDiv (dx, wScaleMx, 100)) == 0x7FFF
			|| dxScale < dxMin || dxScale > dxMax ||
			(dyScale = NMultDiv (dy, wScaleMy, 100))
			== 0x7FFF || dyScale < dyMin  || dyScale > dyMax)
		{
		ErrorEid(eidBadScale, "");
		if (fDialog)
			{
			SetTmcTxs(tmcScaleMy, 
					TxsOfFirstLim(0, ichLimLast));
			}

		return fFalse;
		}
	return fTrue;

}





/* ****
*
	Function: FDlgInsPic
*  Author:
*  Copyright: Microsoft 1986
*  Date: 3/10/86
*
*  Description: Dialog function 
*
** ***/

/*  %%Function:  FDlgInsPic  %%Owner:  bobz       */


BOOL FDlgInsPic(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	int iT;
	CHAR sz [ichMaxBufDlg];
	HCAB hcab;
	int cch;

	switch (dlm)
		{
	default:
		return (fTrue);
	case dlmIdle:
		if (wNew /* cIdle */ == 0)
			return fTrue;  /* call FSdmDoIdle and keep idling */
		if (wNew /* cIdle */ == 3)
			FAbortNewestCmg (cmgLoad1, fTrue, fTrue);
		else  if (wNew /* cIdle */ == 5)
			FAbortNewestCmg (cmgLoad2, fTrue, fTrue);
		else  if (wNew > 5)
			return fTrue; /* stop idling */
		return fFalse; /* keep idling */

	case dlmInit:
		SetDefaultTmc(tmcNewPic);
		break;

	case dlmClick:
		if  (tmc == tmcNewPic)
			{
			/* ugh - need to ignore filename field if empty, so
				just fill the cab and bag out
			*/
			if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled)
				return fFalse;
			if (hcab != hcabNull)
				{
				if (vfRecording)
					FRecordCab(hcab, IDDInsPic, tmc, fFalse);
				EndDlg(tmcNewPic);     /* ok to call EndDlg here */

				}
			/* if hcabNull, FFilterSdmMsg will take dialog down */
			return (fTrue);
			}

		/* non-atomic combos get no dlmChange on list box click.
			if we get any non-nil entry, be sure OK is enabled
		*/
		if  (tmc == tmcIPList || tmc == tmcIPDir)
			{
			if (wNew != uNinchList)
				{
				EnableTmc(tmcOK, fTrue);
				SetDefaultTmc(tmcOK);
				}
			}
		break;

	case dlmChange:
			{

			/* disable Ok if no filename string */

			if  (tmc == (tmcInsPic & ~ftmcGrouped))
				{
				GetTmcText(tmcInsPic, sz, ichMaxBufDlg);
				cch = CchStripString(sz, CchSz(sz) - 1);
				EnableTmc(tmcOK, Usgn(cch));
				SetDefaultTmc(cch ? tmcOK : tmcNewPic);
				}

			break;
			}

	case dlmDirFailed:
		ErrorEid(eidBadFileDlg," FDlgInsPic");
		SetTmcTxs (tmcInsPic, TxsAll());
		return fFalse;    /* could not fill directory; stay in dlg */

	case dlmTerm:
			{
			UpdateStDOSPath();

			if (tmc != tmcOK)
				break;

			GetTmcText(tmcInsPic, sz, ichMaxBufDlg);
			cch = CchStripString(sz, CchSz(sz) - 1);
			if (!cch)
				return (fTrue);
			else
				return (FTermFile (PcmbDlgCur(), Iag(CABINSPIC, hszFile),
						tmcInsPic, fFalse /* fLink */, fFalse /* fDocCur */,
						nfoPic));

			}  /* dlmTerm */


		}  /* switch */
	return (fTrue);

}
