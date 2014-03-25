#define NOGDICAPMASKS
#define NONCMESSAGES
#define NODRAWFRAME
#define NORASTEROPS
#define NOMINMAX
#define NOSCROLL
#define NOKEYSTATE
#define NOICON
#define NOPEN
#define NOREGION
#define NODRAWTEXT
#define NOMETAFILE
#define NOCLIPBOARD
#define NOSOUND
#define NOCOMM
#define NOKANJI
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "doc.h"
#include "dlbenum.h"
#include "debug.h"
#include "dmdefs.h"
#include "doslib.h"
#include "file.h"
#include "ch.h"
#include "print.h"
#include "prompt.h"
#include "message.h"
#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"
#include "error.h"
#include "opuscmd.h"
#include "automcr.h"

#include "catalog.hs"
#include "catalog.sdm"
#include "catprog.hs"
#include "catprog.sdm"
#include "catsrch.hs"
#include "catsrch.sdm"
#include "print.hs"
#include "open.hs"

extern struct PPR     **vhpprPRPrompt;
extern BOOL fElActive;
extern BOOL		vfRecording;
extern struct BPTB      vbptbExt;
extern int chszDMDirs;
extern int ihszDMDirs;
extern SB sbDMEnum;            /* Sb of the above HUGE pointer */
extern FC HUGE *hprgfcDMEnum;	/* the indices into the TMP file fnDMEnum */
extern struct MERR      vmerr;
extern HWND             vhwndApp;
extern CHAR             stDOSPath[];
extern CHAR             szEmpty[];
extern struct PRSU      vprsu;
extern struct PRI       vpri;
extern CHAR           **hstDMQPath;
extern struct DMFLAGS   DMFlags;
extern struct DMQD      DMQueryD;
extern struct DMQFLAGS  DMQFlags;
extern int              chszDMDirs;
extern int              cDMFileMac;
extern int              cDMEnum;    /* current index when narrowig search*/
extern int              cDMEnumMac;    /* mac used, valid outside of Doc Ret*/
extern int              cDMEnumMax;    /* max allocated, valid inside & out */
extern int              fnDMEnum;
extern CHAR            *pchTreeStart;
extern int              fLeaf;
extern int              iLeafCur;
extern HANDLE           hDMFarMem;
extern CHAR far        *DMFarMem;
extern long             cbDMFarMem;
extern struct FINDFILE  **hDMDTA;  /* search results from FFirst and FNext */
extern CHAR             **rghszDMDirs[];
extern struct DBS       vdbs;
extern int              vlm;
extern int              vflm;
extern CHAR             **hszvsumdFile;
extern struct SUMD      vsumd;
extern BOOL             vfRecorderOOM;


extern long LFromCab();
extern struct AONNODE **HBuildAONTree();
extern SB SbAlloc();

TMC TmcDoCatSrh();
HANDLE HGrabFarMem();
CHAR **HstBuildDMDefaultPath();



csconst int rgIagCatSrhEl [] = /* has to be same order as DMQD */
{
	Iag(CABCATALOG, hszCatSrhTitle),
			Iag(CABCATALOG, hszCatSrhSubject),
			Iag(CABCATALOG, hszCatSrhAuthor),
			Iag(CABCATALOG, hszCatSrhKeyword),
			Iag(CABCATALOG, hszCatSrhRevisor),
			Iag(CABCATALOG, hszCatSrhText),         /* iCatSrhSzSpecMac */
	Iag(CABCATALOG, hCatSrhDttmCreateFrom),
			Iag(CABCATALOG, hCatSrhDttmCreateTo),
			Iag(CABCATALOG, hCatSrhDttmReviseFrom),
			Iag(CABCATALOG, hCatSrhDttmReviseTo)      /* iCatSrhSpecMac */
};


csconst int rgIagCatSrh [] = /* has to be same order as DMQD */
{
	Iag(CABCATSEARCH, hszCatSrhTitle),
			Iag(CABCATSEARCH, hszCatSrhSubject),
			Iag(CABCATSEARCH, hszCatSrhAuthor),
			Iag(CABCATSEARCH, hszCatSrhKeyword),
			Iag(CABCATSEARCH, hszCatSrhRevisor),
			Iag(CABCATSEARCH, hszCatSrhText),         /* iCatSrhSzSpecMac */
	Iag(CABCATSEARCH, hCatSrhDttmCreateFrom),
			Iag(CABCATSEARCH, hCatSrhDttmCreateTo),
			Iag(CABCATSEARCH, hCatSrhDttmReviseFrom),
			Iag(CABCATSEARCH, hCatSrhDttmReviseTo)      /* iCatSrhSpecMac */
};


csconst int rgTmcCatSrh [] = /* same order as rgIagCatSrh */
{
	tmcCatSrhTitle,
			tmcCatSrhSubject,
			tmcCatSrhAuthor,
			tmcCatSrhKeyword,
			tmcCatSrhRevisor,
			tmcCatSrhText,                             /* iCatSrhSzSpecMac */
	tmcCatSrhCreateFrom,
			tmcCatSrhCreateTo,
			tmcCatSrhReviseFrom,
			tmcCatSrhReviseTo
};


#define iCatSrhSzSpecMac 6
#define iCatSrhSpecMac 10


#define SetCabL(hcab, l, iag) \
				(*(long *) ((WORD *) *(hcab) + cwCabMin + (iag)) = (l))

#define LFromCab(hcab, iag) \
				(*(long *) ((WORD *) *(hcab) + cwCabMin + (iag)))



typedef struct _cecat
	{
	CHAR stFrom [20];
	CHAR stTo [20];
	BOOL fPrinterOK;
	HCABCATSEARCH hcabCatSearch;
	HCABPRINT hcabPrint;
} CECAT;


/* %%Function:CmdCatalog %%Owner:CHIC */
CMD CmdCatalog(pcmb)
CMB * pcmb;
{
	CMD cmd = cmdOK;
	int iFile = 0;
	int doc, cb;
	BOOL fRetVal = fFalse;
	int **hrgIndex;
	CABCATALOG *pcabCat;
	HCABCATALOG hcab;
	CECAT cecat;
	CHAR stzFile[ichMaxFullFileName];

	if (fElActive)
		return CmdElCatalog(pcmb);

	hcab = pcmb->hcab;

	cecat.hcabCatSearch = hNil;
	cecat.hcabPrint = hNil;

	if (FCmdFillCab())
		{
		if ((hrgIndex = PpvAllocCb(sbDds, 2 * sizeof(int))) == hNil)
			goto LRet;

		StartLongOp();
	/* set defaults in cab */
		**hrgIndex = 0;   /* cause sel to be 1st entry */
		*(*hrgIndex + 1) = uNinchList;
		if (!FEnsureHstDMQPath())
			goto LRet;

		Assert(hstDMQPath);
		AssertH(hstDMQPath);

		pcabCat = *hcab;
		cb = (CHAR *)&pcabCat->fCatSrhRedo - (CHAR *)&pcabCat->iCatSortBy + 2;
		SetBytes(&pcabCat->iCatSortBy, 0, cb);

		pcabCat->iCatSortBy = DMFlags.dms;
		pcabCat->hrgCatLBIndex = hrgIndex;
		cecat.fPrinterOK = FPrinterOK();
		Assert(DMFlags.dma == dma_StartFromScratch || DMFlags.dma == dma_ReadFcs);
		pcmb->pv = &cecat;
		fRetVal = FDMEnum(DMFlags.dma == dma_StartFromScratch /* fDialog */);
		EndLongOp(fFalse /* fAll */);
		if (DMFlags.fIncompleteQuery && fRetVal)
			ReportDMListIncomplete();

LRet:
		if (!fRetVal)
			{
			return cmdError;
			}
		}

	if (FCmdDialog())
		{
		CHAR dlt [sizeof (dltCatalog)];

		BltDlt(dltCatalog, dlt);
		TmcOurDoDlg(dlt, pcmb);
		if (vfRecording)
			{
			if (cecat.hcabCatSearch != hNil)
				if (!FCopyCabSrhToCabCat(cecat.hcabCatSearch, pcmb->hcab))
					vfRecorderOOM = fTrue;

			if (!vfRecorderOOM)
				FRecordCab(pcmb->hcab, IDDCatalog, tmcOK, fFalse);
			}
		}

	if (pcmb->fAction)
		{
		switch (pcmb->tmc)
			{
		case tmcError:
			cmd =  cmdNoMemory;
			break;

		case tmcOK: /* this is the PRINT case */
				{
				int fDocNotAlreadyOpened, fChkPgMgn;
				int fDeleteFn, lmSave, flmSave;
				CHAR stFrom [20], stTo [20];
				CHAR stzFileName [ichMaxFullFileName];

				StartLongOp();
				hrgIndex = (*hcab)->hrgCatLBIndex;
				doc = GetDocFromCatLB(hrgIndex, &iFile,
						&fDocNotAlreadyOpened, stzFileName);
				fDeleteFn = fDocNotAlreadyOpened;
				lmSave = vlm;
				flmSave = vflm;
				if (doc != docNil && FBeginPrintJob (doc, lmSave, flmSave))
					{
					pcabCat = *hcab;
					bltbyte(cecat.stFrom, stFrom, 20);
					bltbyte(cecat.stTo, stTo, 20);

					fChkPgMgn = fFalse; /* don't check first one...it's done
					in FBeginPrintJob. */
					do 
						{
						DoPrint(doc, stFrom, stTo, fTrue/*fWholeDoc*/, fChkPgMgn);
						fChkPgMgn = fTrue;
						if (fDocNotAlreadyOpened)
							DisposeDoc(doc);
						fDeleteFn |= fDocNotAlreadyOpened;
						if (vpri.fPrErr)
							break;

						if (vfRecording)
							{
							if (FSetCabSz(cecat.hcabPrint, stzFileName + 1, 
									Iag(CABPRINT, hszFileName)))
								{
								FRecordCab(cecat.hcabPrint, IDDPrint, 
										tmcOK, bcmPrint);
								}
							else
								vfRecorderOOM = fTrue;
							}
						}
					while ((doc = GetDocFromCatLB(hrgIndex, &iFile,
							&fDocNotAlreadyOpened, stzFileName)) != docNil);
					EndPrintJob(lmSave, flmSave);
					if (fDeleteFn)
						{
						MarkAllReferencedFn();
						DeleteNonReferencedFns(0);
						}
					}
#ifdef DEBUG
				else
					ReportSz("No doc or FBeginPrintJob failed!");
#endif 
				EndLongOp(fFalse);
				break;
				}

		case tmcCatOpen:
			hrgIndex = (*hcab)->hrgCatLBIndex;
			while (FGetNextDMFile(stzFile, hrgIndex, ichMaxFullFileName, &iFile))
				{
				CHAR *pchLast = &stzFile[stzFile[0]]; /* ch before null term */
				CHAR *pch = pchLast;
				CHAR *pchFirst = &stzFile[1];
				CHAR ch;
				BOOL fExtension = fFalse;

/* add period to filename if no extension */
				while (pch >= pchFirst)
					{
					ch = *pch;
					if (ch == '\\' || ch == '/')
						break;
					else  if (ch == '.' && pchLast - pch <= 3)
						{
						fExtension = fTrue;
						break;
						}
					pch--;
					}
				if (!fExtension && stzFile[0]+2 < ichMaxFullFileName)
					{
					++pchLast;
					*pchLast++ = '.';
					*pchLast = '\0';
					stzFile[0] += 1;
					}

				if ((doc = DocOpenStDof(stzFile, dofNormal|dofNormalized, NULL)) == docNil)
					break;
				Assert (PdodDoc(doc)->fn != fnNil);
				CmdRunAtmOnDoc(atmOpen, doc);

				if (vfRecording)
					{
					HCABOPEN hcab;

					if ((hcab = HcabAlloc(cabiCABOPEN)) == hNil)
						{
						vfRecorderOOM = fTrue;
						}
					else
						{
						if (FSetCabSz(hcab, stzFile + 1, 
								Iag(CABOPEN, hszFile)))
							{
							FRecordCab(hcab, IDDOpen, tmcOK, imiOpen);
							}
						else
							vfRecorderOOM = fTrue;
						FreeCab(hcab);
						}
					}
				}
			break;

		default:
			break;
			}
		}

	if (cecat.hcabCatSearch != hNil)
		FreeCab(cecat.hcabCatSearch);

	if (cecat.hcabPrint != hNil)
		FreeCab(cecat.hcabPrint);

	EndDMEnum(); /* write the fc array to disk */
	if (DMFlags.fForceStartFromScratch)
		{
		DMFlags.dma = dma_StartFromScratch;
		DMFlags.fForceStartFromScratch = fFalse;
		}
	Debug((vdbs.fCkFn && fnDMEnum != fnNil) ? CkDMFn(fnDMEnum) : 0);
	return cmd;
}


/* Handle the FileFind statment from Opel */
/* %%Function:CmdElCatalog %%Owner:CHIC */
CMD CmdElCatalog(pcmb)
CMB * pcmb;
{
	CABCATALOG ** hcab;
	CABCATALOG * pcab;
	BOOL fEnumOk = fTrue;

	hcab = pcmb->hcab;
	if (pcmb->fDefaults)
		{
		int ** hrgIndex;
		int iCatSrhSpec;
		struct DMQSD * pDmqsd;
		struct DTTM * pdttm;
		CHAR stBuf [ichMaxBufDlg];

		if (!FEnsureHstDMQPath())
			return cmdError;

		if ((hrgIndex = PpvAllocCb(sbDds, 2 * sizeof(int))) == hNil)
			return cmdError;

		pcab = *hcab;
		pcab->iCatSortBy = DMFlags.dms;
		pcab->fCatSrhMatchCase = DMQueryD.fCase;
		pcab->fCatSrhRedo = fFalse;

		/* note: if there are no entries, we monkey with
		this during the list box proc
		*/
		**hrgIndex = 0;   /* cause sel to be 1st entry */
		*(*hrgIndex + 1) = uNinchList;
		pcab->hrgCatLBIndex = hrgIndex;

		CopySt(*hstDMQPath, stBuf);
		FSetCabSt(hcab, stBuf, Iag(CABCATALOG, hszCatSrhList));

		for (iCatSrhSpec = 0, pDmqsd = &DMQueryD; 
				iCatSrhSpec < iCatSrhSzSpecMac; pDmqsd++, iCatSrhSpec++)
			{
			if (pDmqsd->hstSpec != hNil)
				CopySt(*(pDmqsd->hstSpec), stBuf);
			else
				stBuf[0] = 0;
			Assert(stBuf[0] < ichMaxBufDlg);
			FSetCabSt(hcab, stBuf, 
					rgIagCatSrhEl[iCatSrhSpec]);

			}

		pdttm = (struct DTTM *)&DMQueryD.CDateInfo;
		while (iCatSrhSpec < iCatSrhSpecMac)
			{
			SetCabL(hcab, (long) *pdttm, 
					rgIagCatSrhEl[iCatSrhSpec++]);
			pdttm++;
			}

		/* fMemFail will be true if FSetCabSt fails */
		if (vmerr.fMemFail)
			return cmdNoMemory;
		}

	if (pcmb->fDialog)
		ModeError();

	if (pcmb->fAction)
		{
		DMFlags.dms = ((CABCATALOG *) *pcmb->hcab)->iCatSortBy;

/* set up global from cab, check if string is valid as well */
		if (!FFillDMQuery(hcab)) /* this will set DMFlags.dma */
			return cmdError;

/* FFillDMQuery used to be called only from the Search db, it is ok to be
dma_NarrowSearch, but not when we first started up, so force it to restart */
		DMFlags.dma = dma_StartFromScratch;

/* do the enumeration */
		fEnumOk = FDMEnum(fFalse);
		EndDMEnum(); /* write the fc array to disk */
		if (DMFlags.fForceStartFromScratch)
			{
			DMFlags.dma = dma_StartFromScratch;
			DMFlags.fForceStartFromScratch = fFalse;
			}
		}

	return fEnumOk ? cmdOK : cmdError;
}


/* %%Function:FCopyCabSrhToCabCat %%Owner:CHIC */
FCopyCabSrhToCabCat(hcabSrh, hcabCat)
HCABCATSEARCH hcabSrh;
HCABCATALOG hcabCat;
{
	int iiag;
	CABCATALOG * pcabCat;
	CABCATSEARCH * pcabSrh;
	CHAR szBuf [ichMaxBufDlg];

	Assert(vfRecording); /* only time this is useful! */

	pcabCat = *hcabCat;
	pcabSrh = *hcabSrh;

	pcabCat->fCatSrhMatchCase = pcabSrh->fCatSrhMatchCase;
	pcabCat->fCatSrhRedo = pcabSrh->fCatSrhRedo;

	GetCabSz(hcabSrh, szBuf, sizeof (szBuf), 
			Iag(CABCATSEARCH, hszCatSrhList));
	if (!FSetCabSz(hcabCat, szBuf, Iag(CABCATALOG, hszCatSrhList)))
		return fFalse;

	for (iiag = 0; iiag < iCatSrhSzSpecMac; iiag += 1)
		{
		GetCabSz(hcabSrh, szBuf, sizeof (szBuf), rgIagCatSrh[iiag]);
		if (!FSetCabSz(hcabCat, szBuf, rgIagCatSrhEl[iiag]))
			return fFalse;
		}

	for ( ; iiag < iCatSrhSpecMac; iiag += 1)
		{
		SetCabL(hcabCat, LFromCab(hcabSrh, rgIagCatSrh[iiag]),
				rgIagCatSrhEl[iiag]);
		}

	return fTrue;
}


/* Dialog function for Catalog */
/* %%Function:FDlgCatalog %%Owner:CHIC */
FDlgCatalog(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	CECAT * pcecat;
	int tmcSub;
	int dms;
	int fDirty, fRet;
	CABCATALOG **hcabCat;
	CABCATALOG * pcabCat;
	BOOL fCkButtons = fFalse;

	pcecat = PcmbDlgCur()->pv;
	Assert(pcecat != NULL);

	fRet = fTrue;

/* NEWSDM - multiple selection, gray button if nothing is selected */
	switch (dlm)
		{
	case dlmInit:
		EnableCatButtons(pcecat->fPrinterOK);
		EnableTmc(tmcCatSort, fFalse);
		break;

	case dlmIdle:
		if (wNew /* cIdle */ == 0)
			return fTrue;  /* call FSdmDoIdle and keep idling */
		if (DMFlags.fIncompleteQuery)
			ReportDMListIncomplete();
		if (vmerr.mat != matNil)
			FlushPendingAlerts();
		return fFalse;  /* keep idling */

	case dlmExit:
		if (wNew /*tmc*/ == tmcOK)
			{ /* hack to get the cab updated */
			if ((hcabCat = HcabFromDlg(fFalse)) == hcabNotFilled ||
					hcabCat == hcabNull)
				return fFalse;
			}
		break;

	case dlmClick:
		switch (tmc)
			{
		default:
			break;

		case tmcCatSortBy:
			dms = ValGetTmc(tmcCatSortBy);
/* make sure dms is within range because list box can have no selection */
			fDirty = (dms >= dmsMin && dms < dmsMax && dms != DMFlags.dms);
			EnableTmc(tmcCatSort, fDirty && cDMFileMac > 0);
			break;

		case tmcCatLB:
			UpdateCatBanter();
			break;

		case tmcCatSort:
			/* the list has to be resorted */
			DMFlags.dms = ValGetTmc(tmcCatSortBy);
			Assert(DMFlags.dms >= dmsMin && DMFlags.dms < dmsMax);
			if (cDMFileMac > 0)
				{
				StartLongOp();
				DMFlags.dma = dma_Resort;
				FDMEnum(fFalse /* fDialog */); /* if fail, have blank list */
				FillCatLB();
				SetFocusTmc(tmcCatLB);
				EnableTmc(tmcCatSort, fFalse /* fOn */);
				EndLongOp(fFalse /* fAll */);
				}
			break;

		case tmcCatSearch: /* gosub to Catalog Search */
				{
				HCABCATSEARCH hcabCatSearch = pcecat->hcabCatSearch;
				int dmsOld = DMFlags.dms;

				if (hcabCatSearch == hNil &&
						(hcabCatSearch = HcabAlloc(cabiCABCATSEARCH)) == hNil)
					goto LSetFocus;

				pcecat->hcabCatSearch = hcabCatSearch;

				tmcSub = TmcDoCatSrh(hcabCatSearch);

				switch (tmcSub)
					{
				default:
					break;

				case tmcError:
					return fFalse;

				case tmcOK:
					if (!DMFlags.fDMOutOfMemory)
						{
						StartLongOp();

						if (FFillDMQuery(hcabCatSearch))
							{
							Assert(**hstDMQPath > 0);
							Assert(DMFlags.dma == dma_StartFromScratch ||
									DMFlags.dma == dma_NarrowSearch);
							DMFlags.dms = ValGetTmc(tmcCatSortBy);
							if (DMFlags.dms != dmsOld || DMFlags.fForceStartFromScratch)
								{
/* sort changed, but button not pressed, force a StartFromScratch to do the sort */
								DMFlags.dma = dma_StartFromScratch;
								DMFlags.fForceStartFromScratch = fFalse;
								EnableTmc(tmcCatSort, fFalse /* fOn */);
								}
							FDMEnum(fTrue);
							}
						}

			/* refill the list box based on new spec */
					FillCatLB();
					fCkButtons = fTrue;
					EndLongOp(fFalse/* fAll */);
					break;
					} /* end of switch */
				goto LSetFocus;
				}

		case tmcCatDelete:
			if ((hcabCat = HcabFromDlg(fFalse)) != hcabNull
					&& hcabCat != hcabNotFilled)
				{
				pcabCat = *hcabCat;
				CatDel(pcabCat->hrgCatLBIndex);
				fRet = fFalse; /* to prevent recording */
				fCkButtons = fTrue;
				}
			goto LSetFocus;

		case tmcCatPrint: /* gosub to Catalog Print */
			pcecat = PcmbDlgCur()->pv;
			Assert(pcecat != NULL);

			fRet = fFalse; /* to prevent recording */

			/* start the standard dialog for printing documents */
			if ((tmcSub = TmcCatPrintDialog(pcecat)) != tmcOK)
				goto LSetFocus; /* stay in Catalog dialog */
			else
				EndDlg(tmcOK);   /* ok to call EndDlg here */

			break;

		case tmcCatSummary: /* gosub to Summary Info... */
			if ((hcabCat = HcabFromDlg(fFalse)) != hcabNull
					&& hcabCat != hcabNotFilled)
				tmc = TmcCatSI(hcabCat);
			if (DMFlags.fUpdateBanter)
				UpdateCatBanter();
			goto LSetFocus;

			}
		}
	return fTrue;

LSetFocus:
	SetFocusTmc(tmcCatLB);
	ChangeCancelToClose();
	if (fCkButtons && ((pcecat = PcmbDlgCur()->pv) != NULL))
		EnableCatButtons(pcecat->fPrinterOK);
	return fRet;
}


/* %%Function:WListCatalog %%Owner:CHIC */
EXPORT WORD WListCatalog(tmm, sz, isz, filler, tmc, wParam)
TMM tmm;
CHAR * sz;
int isz;
WORD filler;
TMC tmc;
WORD wParam;
{


	/* this function is only used the first time the box is filled */

	if (tmm == tmmCount)
		{
		FillCatLB();
		/* gruesome hack. SDM will create the listbox, THEN fill from
			the cab. They will blow if we select item 0 and the box is empty, so
			we need to change the cab entry before they use it!!! Scottbe
			and Paulbo came up with this. I init the cab entry to be 0
			in the case where there are files, so we only need to do this in
			the empty case.
				
			Since we had to allocate 2 words in the init case just to get
			here, and since this is a gruesome hack to begin with, I am just
			loading the uninchList into the data rather than calling RgbToCab
			which would free and reallocate
			bz  5/2/89
		*/
		if (cDMFileMac == 0)
			{
			CABCATALOG **hcabCat = HcabDlgCur();
			**((*hcabCat)->hrgCatLBIndex) = uNinchList;
			}
		}
	return 0;
}


/* %%Function:FillCatLB %%Owner:CHIC */
FillCatLB()
{
/* Update the list box in Catalog dialog */
	int iFile;
	CHAR stzFile[ichMaxCompactFileName];

	if (cDMFileMac >= 0)
		{
		stzFile[1] = '\0'; /* tmp */
		StartListBoxUpdate(tmcCatLB);
		for (iFile = 0; iFile < cDMFileMac; iFile++)
			{
			if (FDMGetFile(stzFile, ichMaxCompactFileName, dmf_CompactFileName, iFile))
				AddListBoxEntry(tmcCatLB, &stzFile[1]);
			}
		EndListBoxUpdate(tmcCatLB);
		Select1stCatLB();
		UpdateCatBanter();
		}
}


/* %%Function:UpdateCatBanter %%Owner:CHIC */
UpdateCatBanter()
{
/* update the title banter for the first selected file */
	int iFile;
	CHAR *pch;
	int rgw[2];
	CHAR stzBanter[ichMaxSortField];
	CHAR szClean[ichMaxSortField*2];

	SetBytes(stzBanter, 0, 2);

	GetTmcLargeVal(tmcCatLB, &rgw[0], 4 /*cbMac*/);
	iFile = rgw[0];
	if (iFile >= 0 && iFile < cDMFileMac &&
			FDMGetFile(stzBanter, ichMaxSortField, dmf_SortField, iFile))
		{
		SanitizeSz(&stzBanter[1], szClean, ichMaxSortField*2, fFalse);
		pch = szClean;
		}
	else
		pch = &stzBanter[1];
	SetTmcText(tmcCatBanter, pch);
}


/* %%Function:EnableCatButtons %%Owner:CHIC */
EnableCatButtons(fPrinterOK)
BOOL fPrinterOK;
{
/* Enable/Disable the Open, Print, Summary buttons based on the
	File list box in the Catalog dialog box. */

	BOOL fEnable = (cDMFileMac > 0) && !vmerr.fFnFull && !vmerr.fDocFull;

	EnableTmc(tmcCatOpen, fEnable && !vmerr.fMwFull);
	EnableTmc(tmcCatPrint, fEnable && fPrinterOK);
	EnableTmc(tmcCatSummary, fEnable);
	EnableTmc(tmcCatDelete, cDMFileMac > 0);
}


/* %%Function:TmcCatPrintDialog %%Owner:CHIC */
TmcCatPrintDialog(pcecat)
CECAT * pcecat;
{
	TMC tmc;
	CMB cmb;
	BOOL fRecordingSav;

	if (!FInitCmb(&cmb, bcmPrint, pcecat->hcabPrint, cmmNormal | cmmBuiltIn))
		return tmcError;

	pcecat->hcabPrint = cmb.hcab;

	InhibitRecorder(&fRecordingSav, fTrue);

	if ((tmc = TmcDoPrintDialog(&cmb, fFalse /* fPrintMerge */, 
			fTrue /* fFileFind */)) == tmcOK)
		{
		if (!vpri.fHaveBinInfo)
			GetBinInfo(cmb.hcab);
		FillPrsu(cmb.hcab, pcecat->stFrom, pcecat->stTo, 
				sizeof (pcecat->stFrom));
		}

	InhibitRecorder(&fRecordingSav, fFalse);

	return tmc;
}


/* %%Function:TmcDoCatSrh %%Owner:CHIC */
TMC TmcDoCatSrh(hcab)
HCABCATSEARCH hcab;
{
	CABCATSEARCH * pcabCatSearch;
	int iCatSrhSpec;
	struct DMQSD *pDmqsd;
	struct DTTM *pdttm;
	CHAR ***phstSpec;
	CMB cmb;
	CHAR dlt [sizeof (dltCatSearch)];
	CHAR st[ichMaxBufDlg];
	Assert(sizeof(struct DTTM) == sizeof(long));

/* Although we know for sure that the first time this is called from Catalog
	hstDMQPath is not hNil but still has to ensure query path because it could
	be set to hNil in FFillDMQuery later and still remain in Catalog db and do
	a Search to enter this code.
*/
	if (!FEnsureHstDMQPath())
		return tmcError;

	Assert(hstDMQPath);
	AssertH(hstDMQPath);

/* local copy of csconst structures */
	BltDlt(dltCatSearch, dlt);

	pcabCatSearch = (CABCATSEARCH *) *hcab;
	pcabCatSearch->fCatSrhRedo = fFalse; /* always */
	pcabCatSearch->fCatSrhMatchCase = DMQueryD.fCase;

	CopySt(*hstDMQPath, st);
	FSetCabSt(hcab, st, Iag(CABCATSEARCH, hszCatSrhList));

/* string spec */
	for (iCatSrhSpec = 0, pDmqsd = &DMQueryD; 
			iCatSrhSpec < iCatSrhSzSpecMac; pDmqsd++, iCatSrhSpec++)
		{
		if (pDmqsd->hstSpec != hNil)
			CopySt(*(pDmqsd->hstSpec), st);
		else
			st[0] = 0;
#ifdef DEBUG
		Assert(st[0] < ichMaxBufDlg);
#endif
		FSetCabSt(hcab, st, rgIagCatSrh[iCatSrhSpec]);
		}
/* date spec */
	pdttm = (struct DTTM *)&DMQueryD.CDateInfo;
	while (iCatSrhSpec < iCatSrhSpecMac)
		{

		SetCabL(hcab, (long) *pdttm, rgIagCatSrh[iCatSrhSpec++]);
		pdttm++;
		}

	/* fMemFail will be true if FSetCabSt fails */
	if (vmerr.fMemFail)
		return tmcError;

	cmb.hcab = hcab;
	cmb.pv = NULL;
	cmb.cmm = cmmNormal;
	cmb.bcm = bcmNil;
	return TmcOurDoDlg(dlt, &cmb);
}



/******************************************************************
FValidDMQPath:
	This routine parses the Query Path into a series of dir or file names. 
	It checks each by calling FFirst to ensure each directory is valid.
	If there is a filespec at the end, it backs up to the directory and
	checks that instead, we'll deal with checking the files later.
******************************************************************/
/* %%Function:FValidDMQPath %%Owner:CHIC */
FValidDMQPath(pchPath,cchPath,pichError)
CHAR *pchPath;
int cchPath;
int *pichError;
{
	CHAR *pchT,*pchSpace,*pchStart=pchPath;
	int cchDir,cchT=0;
	int ichFileError=0;
	struct FINDFILE DTA;
	int da;
	CHAR rgchDir[cchMaxFile];
	CHAR rgchExt[4];

/* fill rgchExt with *.*  */
	rgchExt[0]=rgchExt[2]='*';
	rgchExt[1]='.';
	rgchExt[3]=0;

	if (cchPath > cchMaxDMQPath)
		{
		*pichError = cchMaxDMQPath;
		return(fFalse);
		}

	while (cchT<cchPath)
		{
		while (*pchPath==chSpace)    /* skip leading spaces */
			{
			pchPath++;
			if (++cchT == cchPath)    /* unneeded comma or semi at end */
				goto DoneWithPath;
			}
		pchT=pchPath;
		while (*pchT !=chComma && *pchT != chSemi && cchT<cchPath)
			{
			cchT++;
			pchT++;
			}
		if ((cchDir=pchT-pchPath) > cchMaxFile)
			{
			*pichError += pchPath-pchStart+cchMaxFile;
			return(fFalse);
			}
		if (cchDir==0)
			goto NextDir;
		bltb(pchPath,rgchDir,cchDir);        /* copy to buffer */
		/* remove any trailing spaces */
		pchSpace=rgchDir+cchDir;
		while (*(pchSpace-1)==chSpace && pchSpace > rgchDir+1)
			{
			cchDir--;
			pchSpace--;
			}
		rgchDir[cchDir]=0;    /* terminate */

	/* check validity of path */
		if (FntSz(rgchDir,cchDir,&ichFileError,nfoPathWildOK)==fntInvalid)
			{
			*pichError += pchPath-pchStart+ichFileError;
			return(fFalse);
			}

	/* we now have two cases. 1. This is a dir 2. This is a filespec */
		if (rgchDir[cchDir-1]=='\\')
			{           /* append *.* and try FFirst */
			bltb(rgchExt,rgchDir+cchDir,4);
		/*****
		rgchDir now holds a pathname that can be passed to FFirst()
		*****/
			da = 0; /* to FFirst */
			}
		else
			{
			/*****
			No backslash. Two possibilities: Either they left
			the backslash off the end of the dir (still case 1), 
			or this is a filespec (case 2). We will tell FFirst that
			subdirs are valid, and as long as it doesn't return 2, we
			know that either the string is a subdir (they left off the
			backslash), or the parent dir exists (this is a filespec).
			*****/
			da = DA_SUBDIR; /* to FFirst */
			}

		AnsiToOem(rgchDir, rgchDir); /* for FFirst */
		if (FFirst(&DTA,rgchDir,da) ==2)
			{   /* bad directory, update pichError and return */
			*pichError += pchPath-pchStart;
			return(fFalse);
			}

NextDir:
	/* otherwise it was a valid dir or file spec, so go on */
		pchPath=pchT+1;            /* step past ',' */
		cchT++;
		}                   /* end of while loop */
DoneWithPath:
	return(fTrue);
}



/***************************************************************************
	FFillDMQuery:
	Fill in the query path, and DMQueryD from the cab.
	Old heap strings are freed before new ones are allocated.
	Build expression tree for nonnull spec strings.
	Keep track of which spec strings are nonnull -> DMQFlags.
	Grab far memory for wordgrep code if text string exist.
	Return TRUE if everything is fine, else free all allocated 
	strings so far and return FALSE.
	*************************************************************************/
/* %%Function:FFillDMQuery %%Owner:CHIC */
FFillDMQuery(hcab)
HCAB hcab;
{
	CABCATSEARCH *pcabCatSearch = *hcab;
	CABCATALOG * pcabCat = pcabCatSearch;
	BOOL fNarrowSearch;
	BOOL fMatchCase;
	int grpf = 0;
	int iCatSrhSpec;
	struct DMQSD *pDmqsd;
	struct DTTM *pdttm1;
	struct DTTM dttm2;
	CHAR st[ichMaxBufDlg];
	CHAR *pchSearchField;
	int ichError = 0;

	Assert(MAXSEARCHFIELD == ichMaxBufDlg);
	Assert(cchMaxDMQPath < ichMaxBufDlg);
	Assert(cchMaxAONExp < ichMaxBufDlg);


/* If executing the macro statement, fElActive is TRUE, parameters are 
loaded to cabCat because macro does not have access to both cabs.  Also
have to parse cab strings because parameters do not go through the dialog
to be parsed. */

	fNarrowSearch = !(fElActive ? pcabCat->fCatSrhRedo : 
			pcabCatSearch->fCatSrhRedo);

/* don't want to assign immediately to DMFlags until we are all done with parsing the rest */
	fMatchCase = fElActive ? pcabCat->fCatSrhMatchCase : 
			pcabCatSearch->fCatSrhMatchCase;

/* query path */
	AssertH(hstDMQPath);

	GetCabSt(hcab, st, cchMaxDMQPath+1, 
			fElActive ? Iag(CABCATALOG, hszCatSrhList) : 
			Iag(CABCATSEARCH, hszCatSrhList));

	if (st[0] > 0)
		{
		if (FNeNcSt(*hstDMQPath, st))
			{
			if (fElActive && !FValidDMQPath(&st[1], st[0], &ichError))
				return fFalse;
			FreeH(hstDMQPath);
			hstDMQPath = HStCreate(st);
			fNarrowSearch = fFalse;
			}
		}
	else  /* user deleted path */		
		{
		FreePh(&hstDMQPath);
		FEnsureHstDMQPath();
		fNarrowSearch = fFalse;
		}

	if (hstDMQPath == hNil)
		return fFalse;


	DMQFlags.grpf = 0;
/* query spec strings */
	for (iCatSrhSpec = 0, pDmqsd = (struct DMQSD *)&DMQueryD; 
			iCatSrhSpec < iCatSrhSzSpecMac; 
			pDmqsd++, iCatSrhSpec++)
		{
		GetCabSt(hcab, st, cchMaxAONExp+1, 
				(fElActive ? rgIagCatSrhEl : rgIagCatSrh)[iCatSrhSpec]);

/* use case sensitive string check for text string if match case is on, all
other cases can use case insensitive check */
		if (pDmqsd == &(DMQueryD.dmqsdText) && fMatchCase &&
				pDmqsd->hstSpec && FNeSt(*(pDmqsd->hstSpec), st))
			{
			goto LCommon;
			}
		else  if (pDmqsd->hstSpec && FNeNcSt(*(pDmqsd->hstSpec), st))
			{
LCommon:
			if (fElActive && st[0] > 0 && !FValidAONExp(&st[1], st[0], &ichError))
				goto LErrRet;

			fNarrowSearch = fFalse;
			}
		if (pDmqsd->hstSpec)
			{
			FreeDMQSD(pDmqsd);
			}
		if (*st > 0 && !FBlankOnlySt(st))
			{
			grpf |= 1 << iCatSrhSpec;
			fLeaf = fFalse;
			iLeafCur = 0;
			pDmqsd->hstSpec = HStCreate(st);
			if (pDmqsd->hstSpec == hNil)
				goto LErrRet;

			pDmqsd->hrgdchLeaves = HAllocateCw(cwMaxLeaves);
			pDmqsd->hrgLenLeaves = HAllocateCw(cwMaxLeaves);
			pDmqsd->cLeaves = 0;
			if (pDmqsd->hrgdchLeaves == hNil || pDmqsd->hrgLenLeaves == hNil)
				goto LErrRet;
			StToSzInPlace(st); /* the search code still uses sz */
			pchTreeStart = pchSearchField = st;
			pDmqsd->hTree = HBuildAONTree(&pchSearchField, OPSTART, 
					pDmqsd->hrgdchLeaves, pDmqsd->hrgLenLeaves, &(pDmqsd->cLeaves));
			Assert(pDmqsd->cLeaves <= MAXNUMLEAVES);
			if (pDmqsd->hTree == hNil)
				goto LErrRet;

			if (pDmqsd == &(DMQueryD.dmqsdText))
				{
				DMQueryD.hrghszTLeaves = HAllocateCw(pDmqsd->cLeaves);
				if (DMQueryD.hrghszTLeaves == hNil)
					goto LErrRet;
				SetWords(*DMQueryD.hrghszTLeaves, NULL, pDmqsd->cLeaves);
/* grab far memory for grep code */
				if (DMFarMem == 0 && !FAllocDMFarMem())
					goto LErrRet;
				SzSpectoGrepRghsz(st, pDmqsd->hrgdchLeaves,
						pDmqsd->hrgLenLeaves, DMQueryD.hrghszTLeaves,
						pDmqsd->cLeaves);
				}
			if (vmerr.fMemFail)
				goto LErrRet;
			}
		}

/* dates */
	Assert(sizeof(struct DTTM) == sizeof(long));
	Assert(iCatSrhSpec == iCatSrhSzSpecMac);
	pdttm1 = (struct DTTM *)&(DMQueryD.CDateInfo);
	while (iCatSrhSpec < iCatSrhSpecMac)
		{
		dttm2 = LFromCab(hcab, 
				(fElActive ? rgIagCatSrhEl : rgIagCatSrh)[iCatSrhSpec]);
		if ((long)*pdttm1 != (long) 0 && CompareDTTMDate(*pdttm1, dttm2) != 0)
			fNarrowSearch = fFalse;

		*pdttm1 = dttm2;
		if ((long)dttm2 != (long)0)
			{
			grpf |= 1 << iCatSrhSpec;
			}
		iCatSrhSpec++;
		pdttm1++;
		}


	DMQFlags.grpf = grpf;
	if (DMQueryD.fCase != fMatchCase)
		{
		DMQueryD.fCase = fMatchCase;
		if (!fMatchCase && DMQFlags.fTextTest) /* turn off match case force full search */
			fNarrowSearch = fFalse;
		}

#ifdef DEBUG
		{
		CHAR **hstSpec;
		if (DMQFlags.fTitleTest)
			Assert((hstSpec = DMQueryD.dmqsdTitle.hstSpec) != hNil && 
					**hstSpec > 0);
		if (DMQFlags.fSubjectTest)
			Assert((hstSpec = DMQueryD.dmqsdSubject.hstSpec) != hNil && 
					**hstSpec > 0);
		if (DMQFlags.fAuthorTest)
			Assert((hstSpec = DMQueryD.dmqsdAuthor.hstSpec) != hNil && 
					**hstSpec > 0);
		if (DMQFlags.fSavedByTest)
			Assert((hstSpec = DMQueryD.dmqsdSavedBy.hstSpec) != hNil && 
					**hstSpec > 0);
		if (DMQFlags.fKeywordTest)
			Assert((hstSpec = DMQueryD.dmqsdKeyword.hstSpec) != hNil && 
					**hstSpec > 0);
		if (DMQFlags.fTextTest)
			Assert((hstSpec = DMQueryD.dmqsdText.hstSpec) != hNil && 
					**hstSpec > 0);
		if (DMQFlags.fCFromDateTest)
			Assert((long)DMQueryD.CDateInfo.dttmFrom != (long)0);
		if (DMQFlags.fCToDateTest)
			Assert((long)DMQueryD.CDateInfo.dttmTo != (long)0);
		if (DMQFlags.fSFromDateTest)
			Assert((long)DMQueryD.SDateInfo.dttmFrom != (long)0);
		if (DMQFlags.fSToDateTest)
			Assert((long)DMQueryD.SDateInfo.dttmTo != (long)0);
		}
#endif

	DMFlags.dma = fNarrowSearch ? 
			dma_NarrowSearch : dma_StartFromScratch;

	return fTrue;

LErrRet:
	FreeDMQueryD();
	return fFalse;
}


/* %%Function:FreeDMQueryD %%Owner:CHIC */
FreeDMQueryD()
{
	int iCatSrhSpec;
	struct DMQSD *pDmqsd;

	for (iCatSrhSpec = 0, pDmqsd = (struct DMQSD *)&DMQueryD; 
			iCatSrhSpec < iCatSrhSzSpecMac; pDmqsd++, iCatSrhSpec++)
		FreeDMQSD(pDmqsd);
	SetBytes(&DMQueryD, 0, sizeof(struct DMQD));
}




/* %%Function:FDMEnum %%Owner:CHIC */
FDMEnum(fDialog)
int fDialog;
{
	unsigned cbNew;
	struct FCB *pfcb;
	BOOL fRet = fFalse;
	CHAR rgchFile[cchMaxFile];

	DMFlags.fIncompleteQuery = fFalse;
	ihszDMDirs= 0;  /* initialize current dir to first in path */
	DMFlags.fDMOutOfMemory=fFalse;

	Debug(vdbs.fCkStruct ? CkDMStruct() : 0);

	if (DMFlags.dma == dma_StartFromScratch)
		{
		ParseDMQPath();
		}

	if (fnDMEnum==fnNil)
		{
		/* open the dm tmp file here */
		rgchFile[0]=0;
		if ((fnDMEnum=FnOpenSt(rgchFile, fOstCreate, ofcDMEnum, NULL ))==fnNil)
			return(fFalse);   /* can't open temp file */
		}
#ifdef DEBUG
	else 
		Debug(vdbs.fCkFn ? CkDMFn(fnDMEnum) : 0);
#endif

	switch (DMFlags.dma)
		{
	default:
		Assert(fFalse);
		return fFalse;
	case dma_ReadFcs:
		return(FReadDMFcFile());
	case dma_Resort:
	case dma_NarrowSearch:
		cDMEnumMac=cDMFileMac;  /* save end of old array */
		break;
	case dma_StartFromScratch:
		/* truncate file to zero */
		DeleteFnHashEntries(fnDMEnum);
		(pfcb = PfcbFn(fnDMEnum))->cbMac = 0;
		pfcb->pnXLimit = pn0;
		if (sbDMEnum==sbNil)
			{
			cbNew = 0;
			if ((sbDMEnum = SbAlloc(cEnumInit*4, &cbNew)) == sbNil)
				{
				hprgfcDMEnum= (HP) NULL;
				DMFlags.fDMOutOfMemory=fTrue;
				return fFalse;
				}
			Assert(cbNew != 0);
			cDMEnumMax= cbNew/4;
			hprgfcDMEnum= HpOfSbIb(sbDMEnum,0);
			}
		Assert(hprgfcDMEnum != (HP)NULL);
		Assert(cDMEnumMax > 0);
		SetWords(LpConvHp(hprgfcDMEnum), NULL, cDMEnumMax*2);
		break;
		} /* end of switch */

	cDMFileMac = 0;
	cDMEnum = 0;

/* prepare the far memory for grep code */
	if (DMQFlags.fTextTest && DMFarMem == 0 && !FAllocDMFarMem())
		return fFalse;

	Assert(hDMDTA==0);
	if ((hDMDTA= HAllocateCw(CwFromCch(sizeof(struct FINDFILE)))) == hNil)
		goto LRet2;

	if (fDialog)
		{
		CMB cmb;
		CHAR dlt [sizeof (dltCatSrhProgress)];

		if ((cmb.hcab = HcabAlloc(cabiCABCATSRHPROG)) == hNil)
			goto LRet1;

		cmb.cmm = cmmNormal;
		cmb.pv = NULL;
		cmb.bcm = bcmNil;

		BltDlt(dltCatSrhProgress, dlt);
		if (TmcOurDoDlg(dlt, &cmb) == tmcError)
			{
			FreeCab(cmb.hcab);
			goto LRet1;
			}

		FreeCab(cmb.hcab);
		fRet = !(vmerr.fDiskAlert || vmerr.fDiskWriteErr);
		}
	else 		
		{
		if (FDMEnumMore(fTrue /*fInit*/))
			while (FDMEnumMore(fFalse /*fInit*/));

		fRet = !(vmerr.fDiskAlert || vmerr.fDiskWriteErr);
		if (fRet && cDMFileMac > 0 && (DMFlags.dma != dma_NarrowSearch))
			{   /* if Narrowing the search they are sorted */
			DMSort();
			}
		} /* !fDialog */

	if (DMFlags.dma == dma_StartFromScratch)
	/* get dir names ready for blting, do it even if has disk error because
	can still display partial list */
		TruncatePathDMDirs();

LRet1:
	FreePh(&hDMDTA);
LRet2:
	if (hDMFarMem)
		FreeDMFarMem();
	return(fRet);
}


/***************************************************************************
	TruncatePathDMDirs:
	This routine goes through the heap strings in rghszDMDirs and truncates
	them so they just include the path. This removes any *.doc's or other
	filespecs the user may have added, so that filenames form FFirst can be
	tacked on to the end.
***************************************************************************/
/* %%Function:TruncatePathDMDirs %%Owner:CHIC */
TruncatePathDMDirs()
{
	int ihsz;
	CHAR *pchDir;

	Assert(chszDMDirs <= chstDMDirsMax);

	for (ihsz = 0; ihsz < chszDMDirs; ihsz++)
		{
		pchDir = *(rghszDMDirs[ihsz]);
		if (*pchDir)
			{
			*((CHAR *)PchSkipPath(pchDir)) = '\0';
			}
		}
}




/* %%Function:FDlgCatSrhProgress %%Owner:CHIC */
FDlgCatSrhProgress(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	switch (dlm)
		{
	case dlmInit:
		DMFlags.fSrhInProgress = fTrue;
		if (DMQFlags.grpf != 0)
			{
			SetTmcText(tmcCatSrhMText, SzFrameKey("Files Matched : ",FilesMatched));
			}
		break;

	case dlmIdle:
/* wNew is count of idle messages so far */
		if (wNew /* cIdle */ == 0)
			return fTrue;  /* call FSdmDoIdle and keep idling */
		if (FDMEnumMore(wNew == 1 /* fInit */))
			{ /* update progress */
			CHAR *pch;
			CHAR sz[ichMaxBufDlg];

			if (DMQFlags.grpf != 0)
				{
				/* cDMFileMac is # of files matched */
				pch = sz;
				CchIntToPpch(cDMFileMac, &pch);
				*pch = 0; /* null terminate it */
				SetTmcText(tmcCatSrhMatched, sz);
				}

			/* cDMEnum is # of files searched so far */
			pch = sz;
			CchIntToPpch(cDMEnum, &pch);
			*pch = 0; /* null terminate it */
			SetTmcText(tmcCatSrhSearched, sz);

			return fFalse; /* because we need to idle more */
			}
		EndDlg(tmcOK);   /* ok to call EndDlg here */

		break;

	case dlmExit:
		DMFlags.fSrhInProgress = fFalse;
		if (cDMFileMac > 0 && (DMFlags.dma != dma_NarrowSearch))
			{   /* if Narrowing the search they are sorted */
			DMSort();
			}
/* if we donot do this, we are stuck with the incomplete list until
the user changed the query path */
		DMFlags.fForceStartFromScratch = (wNew /*tmc*/ == tmcCancel);
		break;
		}

	return fTrue;
}




/* %%Function:FGetNextDMFile %%Owner:CHIC */
FGetNextDMFile(stzFile, hrgIndex, cchMaxBuf, piCur)
CHAR *stzFile;
int **hrgIndex;
int cchMaxBuf;
int *piCur;
{
	int iFile;

	iFile = (*hrgIndex)[*piCur];
	if (iFile == -1) /* end of list */
		return fFalse;
	*piCur += 1;
	return(FDMGetFile(stzFile, cchMaxBuf, dmf_FullFileName, iFile));
}




/* %%Function:GetDocFromCatLB %%Owner:CHIC */
GetDocFromCatLB(hrgIndex, piCur, pfDocNotExist, stzFile)
int **hrgIndex;
int *piCur;
int *pfDocNotExist;
CHAR * stzFile;
{
	int doc = docNil;

	if (FGetNextDMFile(stzFile, hrgIndex, ichMaxFullFileName, piCur))
		{
		doc = DocFromSt(stzFile);
		*pfDocNotExist = (doc == docNil);

		if (*pfDocNotExist)
			doc = DocOpenStDof(stzFile, dofNoWindow, NULL);
		}
	return doc;
}




/* %%Function:TmcCatSI %%Owner:CHIC */
TmcCatSI(hcab)
HCABCATALOG hcab;
{
	int tmc;
	int doc;
	int iFile = 0;
	int fDocNotAlreadyOpened;
	int fDirty = fFalse;
	CHAR stzFileName [ichMaxFullFileName];

	Assert(hcab);
	DMFlags.fUpdateBanter = fFalse;
	if ((doc = GetDocFromCatLB((*hcab)->hrgCatLBIndex, 
			&iFile, &fDocNotAlreadyOpened, stzFileName)) == docNil)
		{
		/* if a nestable progress report was left active, deactivate it */
		if (vhpprPRPrompt)
			StopProgressReport(hNil, pdcRestore);
		return tmcError;
		}

/* using the easy scheme (open doc if not already loaded vs.
	simply get the SI part) makes TmcDoDocSummary simplier.
*/
	tmc = TmcDoDocSummary((CMB *) NULL /* implies local cab */, doc, &fDirty,
			!((int) (PdodDoc(doc)->fLockForEdit))/*fEnableOK*/, NULL,
			fFalse);

	if (fDocNotAlreadyOpened)
		{
		if (fDirty) /* write summary info back to file on disk */
			{
			Assert(tmc != tmcCancel);
			Assert(!PdodDoc(doc)->fLockForEdit); /* can't be locked for edit */
/* we should probably do a selected save in case this is too slow,
	the selected save is to just save the dop and hsttbAssoc for native file.
	For non native file and fCompound file, we still have to do the complete
	save, provided that we don't do the quick read for fCompound file. (cc) 
*/
			/* cab is inited by CmdDoSaveDoc */
			DoSaveDoc(doc, fFalse);
			if (hszvsumdFile)
				{ /* invalid the cache to ensure fetching the new summary info */
				FreePSumd(&vsumd);
				FreeH(hszvsumdFile);
				hszvsumdFile = hNil;
				}
			UpdateFcDMEnum(**(*hcab)->hrgCatLBIndex, fTrue /*fQuickSave*/);
			DMFlags.fForceStartFromScratch = fTrue;
			DMFlags.fUpdateBanter = fTrue;
			Debug(vdbs.fCkFn ? CkDMFn(fnDMEnum) : 0);
			}
LRet:
		DisposeDoc(doc);
		MarkAllReferencedFn();
		DeleteNonReferencedFns(0);

		Debug(vdbs.fCkFn ? CkDMFn(fnDMEnum) : 0);
		}
	return tmc;
}




#define cbMinDMFarMem  (unsigned)0x1FFF /* 8 K */

HANDLE HGrabFarMem(pcb)
long *pcb; /* cb count of far memory we get */
{
	long cbAlloc = 0x0000FFFF; /* start with 64 K */
	HANDLE hFarMem = NULL;

	*pcb = 0;
	while (cbAlloc >= cbMinDMFarMem)
		{
		if ((hFarMem = GlobalAlloc(GHND, cbAlloc)) == NULL)
			{
			cbAlloc >>= 2;
			}
		else
			{
			*pcb = cbAlloc;
			return(hFarMem);
			}
		}
	return(NULL);
}


/* %%Function:FAllocDMFarMem %%Owner:CHIC */
FAllocDMFarMem()
{
	if ((hDMFarMem = HGrabFarMem(&cbDMFarMem)) == NULL)
		{
		ErrorEid(eidWinFailure, " AllocDMFarMem");
		return fFalse;
		}
	DMFarMem = GlobalLock(hDMFarMem);
	Assert(cbDMFarMem > 0);
	return fTrue;
}


/* %%Function:FreeDMFarMem %%Owner:CHIC */
FreeDMFarMem()
{
	if (GlobalUnlock(hDMFarMem) != 0)
		Assert(fFalse);
	GlobalFree(hDMFarMem);
	hDMFarMem = DMFarMem = NULL;
	cbDMFarMem = 0;
}


/* %%Function:FEnsureHstDMQPath %%Owner:CHIC */
FEnsureHstDMQPath()
{
	if (hstDMQPath == hNil || **hstDMQPath == 0)
		{
		if (hstDMQPath != hNil)
			FreePh(&hstDMQPath);
		if ((hstDMQPath = HstBuildDMDefaultPath()) == hNil)
			return fFalse;
		}

	return fTrue;
}


/* %%Function:FBlankOnlySt %%Owner:CHIC */
FBlankOnlySt(st)
CHAR *st;
{
	CHAR cch;
	CHAR *pch = st+1;

	cch = *st;
	Assert(cch > 0);
	while (cch > 0 && *pch == ' ')
		{
		cch--;
		pch++;
		}
	return (cch == 0);
}


/* %%Function:CatDel %%Owner:CHIC */
CatDel(hrgIndex)
int **hrgIndex;
{
#define ichMaxReport	ichMaxFullFileName
	extern struct STTB **vhsttbOpen;
	CHAR *rgpch [2];
	int iFile = 0;
	int ifc;
	int ec, da;
	int cDel = 0;
	CHAR stFile[ichMaxFullFileName], szFile [ichMaxFullFileName];
	CHAR stFiles [ichMaxFullFileName];

/* Ask: "Delete foo.doc woo.doc ...?" */

	stFiles [0] = 0;
	while (FGetNextDMFile(stFile, hrgIndex, ichMaxFullFileName, &iFile))
		{
		CHAR szT [256];
		CHAR *pchSrc, *pchDest;
		int cch = min(stFile [0],254);

		pchDest = szT;                        /* prepend space */
		if (stFiles [0] != 0)
			*pchDest++ = ' ';

		pchSrc = &stFile [1];                /* strip '*' */
		if (cch > 0 && *pchSrc == '*')
			cch--, pchSrc++;

		*(bltbyte( pchSrc, pchDest, cch )) = '\0';
		pchDest = &stFiles [stFiles [0] + 1];
		stFiles [0] += CchStuff( &pchDest, szT, ichMaxReport - (stFiles [0] + 1));
		}

	if (stFiles [0] == 0)
		return;

	StToSzInPlace( stFiles );
	rgpch [0] = &stFiles [0];
	if (IdMessageBoxMstRgwMb (mstCatDelete, rgpch, MB_YESNOQUESTION) != IDYES)
		return;


/* Enumerate files, showing each in the prompt as it is deleted */

	iFile = 0;
	while ((ifc = (*hrgIndex)[iFile++]) != -1 && 
			(ifc -= cDel) < cDMFileMac && 
		/* adjust the index by number of files deleted */
	FDMGetFile(stFile,ichMaxFullFileName, dmf_FullFileName, ifc))
		{
		Assert(ifc >= 0);
		StToSz(stFile,szFile);
		if (*szFile == '*')
			{
			SzToSt( &szFile [1], stFile );
			StToSz( stFile, szFile );
			}

		if (FnFromOnlineSt(stFile) != fnNil)
			{
/* error: can't delete a file if we have an fn for it */
/* Spaghetti is to avoid problems with lifetime of frame strings */
LInUse:         
			rgpch [1] = SzFrameKey("in use",InUse);
			goto LCantDel;
LReadOnly:      
			rgpch [1] = SzFrameKey("read-only",ReadOnly);
			goto LCantDel;
LNoAcc:         
			rgpch [1] = SzFrameKey("not accessible",NotAccessible);

LCantDel:       
			rgpch [0] = &szFile [0];
			if (IdMessageBoxMstRgwMb( mstCatCantDelete, rgpch, 
					MB_OKCANCEL | MB_APPLMODAL | MB_ICONQUESTION) != IDOK)
				goto LRet;
			else
				continue;
			}

		if ((ec = EcDeleteSzFfname(szFile)) < 0)
			{
			if (ec == ecNoAccError)
				{
				if (DosxError() == dosxSharing)
					goto LInUse;
				else  if ((da = DaGetFileModeSz(szFile)) != DA_NIL
						&& (da & DA_READONLY))
					goto LReadOnly;
				}
			goto LNoAcc;
			}
		else  /* Deleted successfully */				
			{
			rgpch [0] = &szFile [0];
			SetPromptRgwMst (mstCatDeleted, rgpch, pdcDefault);

			/* remove from list box */
			DeleteListBoxEntry(tmcCatLB, ifc);

			/* remove from rgfc cache */

			if (cDMFileMac > 0)
				{
				bltbh( (CHAR huge *)&hprgfcDMEnum [ifc+1],
						(CHAR huge *)&hprgfcDMEnum [ifc],
						sizeof(FC)*(cDMFileMac-ifc-1));
				cDMFileMac--;
				}

			/* remove from File menu MRU list */

			DelStFileFromMru(stFile);
			cDel++;

			if (vfRecording)
				RecordDelFile(stFile);
			}
		}

LRet:
	if (cDel > 0)
		{
		DMFlags.fForceStartFromScratch = fTrue;
		RestorePrompt();
		Select1stCatLB();
		}
}



/* %%Function:HstBuildDMDefaultPath %%Owner:CHIC */
CHAR **HstBuildDMDefaultPath()
{
	CHAR rgch[cchMaxDMQPath+2]; /* +2 for cch and \0 */

/* leave space for cch, create a null string */
	rgch [0] = '\0';
	FFillDefaultPath(SzShared("\\"), rgch, 0);
	if (rgch[0] == '\0')
		{ /* make a stz */
		CchCopySz( SzShared("A:"), rgch );
		rgch[0] = GetTempDrive(0);
		}
	OemToAnsi(rgch, rgch);
	SzToStInPlace( rgch );
	Assert(rgch[0] > 0 && rgch[0] <= cchMaxDMQPath);
	return(HstCreate(rgch));
}


#define cSearchLevels	5

/* %%Function:FFillDefaultPath %%Owner:CHIC */
FFillDefaultPath(szThisDir, szPath, cRecurse)
CHAR *szThisDir;
CHAR *szPath;
int cRecurse;
{
	int cchszThisDir;
	int cchszPath = CchSz(szPath)-1;
	CHAR szTest[ichMaxFile];
	struct FINDFILE DTA;

	cchszThisDir = CchCopySz(szThisDir, szTest); /* exclude null terminator */
	if (cchszThisDir > (ichMaxFile - 7)) /* -7 for \*.doc0 */
		return fFalse;
	if (cchszThisDir > 0 && cRecurse > 0)
		SzSzAppend(szTest, SzShared("\\"));
	SzSzAppend(szTest, SzShared("*.doc"));

	if (FFirst(&DTA, szTest, DA_NORMAL) == 0)
		{
		while (fTrue)
			{
			if ((DTA.attribute & (DA_HIDDEN | DA_SYSTEM | DA_VOLUME | DA_SUBDIR)) == 0)
				{
				if ((cchszPath + cchszThisDir) < cchMaxDMQPath)
					{
					if (cchszPath > 0)
						SzSzAppend(szPath, SzShared(";"));
					SzSzAppend(szPath, szThisDir);
					}
				break;
				}
			if (FNext(&DTA) != 0)
				break;
			}
		}

	if (cRecurse > cSearchLevels)
		return fTrue;

	if (CchCopySz(szThisDir, szTest) > 1)
		SzSzAppend(szTest, SzShared("\\"));
	SzSzAppend(szTest, SzShared("*.*"));

	if (FFirst(&DTA, szTest, DA_SUBDIR) != 0)
		return fTrue;

	while (fTrue)
		{
		if ((DTA.szFileName[0] != '.') &&
				((DTA.attribute & (DA_HIDDEN | DA_SYSTEM | DA_VOLUME)) == 0) &&
				((DTA.attribute & DA_SUBDIR) != 0))
			{
			if (CchCopySz(szThisDir, szTest) > 1)
				SzSzAppend(szTest, SzShared("\\"));
			SzSzAppend(szTest, DTA.szFileName);
			if (!FFillDefaultPath(szTest, szPath, cRecurse + 1))
				return fFalse;
			}
		if (FNext(&DTA) != 0)
			return fTrue;
		}
}


/* %%Function:FDlgCatSearch %%Owner:CHIC */
BOOL FDlgCatSearch(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	int ichError = 0;
	int iCatSrhSpec;
	int cchSz;
	struct DTTM dttmFrom;
	struct DTTM dttmTo;
	HCABCATSEARCH hcab;
	int tmcT;
	CHAR sz [256];

	if (dlm == dlmTerm && tmc == tmcOK)
		{
		GetTmcText(tmcCatSrhList, sz, sizeof(sz));
		if (!FValidDMQPath(sz, CchSz(sz) - 1, &ichError))
			{
			ErrorEid(eidInvalidDMQPath, " FDlgCatSearch");
			tmcT = tmcCatSrhList;
			goto LErrRet;
			}

		if ((hcab = HcabFromDlg(fFalse)) == hcabNotFilled)
			return fFalse;
		if (hcab == hcabNull)
			/* sdm will take down dialog */
			return (fTrue);

		for (iCatSrhSpec = 0; iCatSrhSpec < iCatSrhSpecMac; iCatSrhSpec++)
			{
			if (iCatSrhSpec < iCatSrhSzSpecMac)
				{
				GetTmcText(rgTmcCatSrh[iCatSrhSpec], sz, sizeof(sz));
				cchSz = CchSz(sz)-1;
#ifdef DEBUG
				Assert (cchSz < sizeof(sz));
#endif        
				if (cchSz > 0 && !FValidAONExp(sz, cchSz, &ichError))
					{
					ErrorEid(eidInvalidDMSrhExp, " FDlgCatSearch");
					goto LErrRet0;
					}
				}
			else  /* Date fields */							
				{
				dttmFrom = LFromCab(hcab, rgIagCatSrh[iCatSrhSpec]);
				dttmTo = LFromCab(hcab, rgIagCatSrh[iCatSrhSpec+1]);
				if ((LONG)dttmTo != 0L && CompareDTTMDate(dttmFrom, dttmTo) > 0)
					{ /* From later than To */
					ErrorEid(eidInvalidDMDateRange, "FDlgCatSearch");
LErrRet0:
					tmcT = rgTmcCatSrh[iCatSrhSpec];
LErrRet:
					SetTmcTxs(tmcT, TxsOfFirstLim(ichError, ichLimLast));
					return fFalse;
					}
				iCatSrhSpec++;
				}
			}
		}
	return fTrue;
}


/* %%Function:ReportDMListIncomplete %%Owner:CHIC */
ReportDMListIncomplete()
{
	DMFlags.fIncompleteQuery = fFalse;
	ErrorEid(eidDMIncomplete, "");
}


/******************************************************************
FDMGetFile:
	This routine returns true if it can fill the requested string
	in stzFile.  The desired string is the iFile item in the tmp file
	that stores all the filenames that match the query from document 
	retrieval.
	stzFile points to a buffer supplied by the caller that is at least
	cchMaxBuf characters.
	If dmf == dmf_FullFileName, this routine returns the complete path 
	and filename.  If dmf == dmf_CompactFileName, the filename returned
	should be center truncated to 40 characters. If the file is 
	quick-saved and there is a text specification, append a * in front 
	of the filename.  If dmf == dmf_SortField, returns the sort field
	content (in case of sorting by name, return the title).
	The returned string is null terminated, a count of characters (CHAR)
	excluding the null terminator is placed in front of the string, so 
	that the returned string can be used as a st or sz.  
*****************************************************************/

/* %%Function:FDMGetFile %%Owner:CHIC */
FDMGetFile(stzFile, cchMaxBuf, dmf, iFile)
CHAR *stzFile; /* location of buffer supplied by caller */
int cchMaxBuf; /* max size of the buffer passed in */
int dmf;       /* should this function return the full filename,
		the compacted filename (40 chars or less, with *
					prepended for quicksaved files), or
		the sort field for this file. */
int iFile; /* ith file to fetch, 0-based */
{
	int cch;
	int fQuickSave;
	CHAR rgchFile[cchMaxFile];

	Assert(cchMaxFile >= cchMaxFDisp && cchMaxBuf >= cchMaxFDisp);
	if (iFile >= cDMFileMac)
		return fFalse;
	SetBytes(stzFile, 0, cchMaxBuf);

	switch (dmf)
		{
	default:
		Assert(fFalse);
		return fFalse;
	case dmf_FullFileName:
	case dmf_CompactFileName:
		Assert(hprgfcDMEnum != (HP) NULL);
		SetBytes(rgchFile, 0, cchMaxFile);
		fQuickSave=FQsSzFilenameFromDMFc(hprgfcDMEnum[iFile],
				rgchFile, fFalse);
		/* rgchFile now has full null terminated filename */
		if ((cch = CchSz(rgchFile)) <= 1)
			return fFalse;
		if (dmf==dmf_FullFileName)
			{
			Assert(cch < cchMaxBuf);
			SzToSt(rgchFile, stzFile);
			break;
			}

		/* we may need to truncate the filename and/or add an asterisk */
		if (cch >= cchMaxFDisp)
			cch= CchTruncateFilename(rgchFile, stzFile);
		else
			bltb(rgchFile, stzFile, cch);
		if (fQuickSave)
			{
			bltb(stzFile,stzFile+1, cch++);
			*stzFile=chAsterisk;
			}
		Assert(cch < cchMaxBuf);
		SzToStInPlace(stzFile);
		break;
	case dmf_SortField:
		Assert(hprgfcDMEnum != (HP) NULL);
		StSortFromDMFc(hprgfcDMEnum[iFile], stzFile, cchMaxBuf);
		break;
		}

	stzFile[stzFile[0] +1] = 0; /* make it a true stz */
	return(stzFile[0] > 0);
} /* end of FDMGetFile */


extern int fnDMFcTable;

/************************************************************************
	EndDMEnum:
	This function is called when leaving catalog mode. It writes out the
	FC table to a tmp file and frees the sb used for it.
************************************************************************/
/* %%Function:EndDMEnum %%Owner:CHIC */
EndDMEnum()
{
	int osfn;
	unsigned cbWrite;
	FC FAR *lpTmp;
	CHAR rgchFile[cchMaxFile];

	Assert(sbDMEnum != sbNil || cDMFileMac == 0);
	if (fnDMFcTable == fnNil)
		{  /* need to open a tmp file for this purpose */
		rgchFile[0] = 0;
		if ((fnDMFcTable = FnOpenSt(rgchFile, fOstCreate, ofcDMEnum, NULL)) == fnNil)
			{
			DMFlags.dma = dma_StartFromScratch;
			return;   /* can't open temp file, will not set dma_ReadFcs */
			}
		}
	else
		{  /* truncate the old file to zero length */
		struct FCB *pfcb;
		DeleteFnHashEntries(fnDMFcTable);
		(pfcb = PfcbFn(fnDMFcTable))->cbMac = 0;
		pfcb->pnXLimit = pn0;
		}
	Debug(vdbs.fCkFn ? CkDMFn(fnDMFcTable) : 0);

	/* Now write the actual table */
	if (cDMFileMac > 0)
		{
		Assert(sbDMEnum != sbNil);
		osfn = OsfnValidAndSeek(fnDMFcTable,0L);
		Assert(osfn != osfnNil);
		Assert(hprgfcDMEnum != (HP) NULL);
		lpTmp = LpConvHp(hprgfcDMEnum);
		cbWrite = CchWriteDoshnd (osfn, lpTmp, cDMFileMac*4);
		if (cbWrite < cDMFileMac*4)
			{ /* in case we write less than what we thought */

			cDMFileMac = cbWrite/4;
			}
#ifdef DEBUG
		else
			Assert(cbWrite == cDMFileMac*4);
#endif
		DMFlags.dma = (cbWrite > 0 ? dma_ReadFcs : dma_StartFromScratch);
		}
	else
		DMFlags.dma = dma_ReadFcs; /* OK to read fcs when no files */

	/* Free up the memory */
	if (sbDMEnum != sbNil)
		{
		FreeSb(sbDMEnum);
		sbDMEnum = sbNil;
		}
	hprgfcDMEnum = (HP) NULL;

	return;
}


/*************************************************************************
	FReadDMFcFile:
	This function reads in the fc table from the file fnDMFcTable, allocating
	a SB to store it in.
**************************************************************************/

/* %%Function:FReadDMFcFile %%Owner:CHIC */
FReadDMFcFile()
{
	unsigned cbNew;
	int osfn;
	FC FAR *lpTmp;

	Assert(sbDMEnum == sbNil);
	Assert(fnDMFcTable != fnNil);
	Debug(vdbs.fCkFn ? CkDMFn(fnDMFcTable) : 0);
	if (cDMFileMac == 0)
		return fTrue; /* no file to read anyway */

	cbNew = 0;
	/* allocate an sb for this thing */
	if ((sbDMEnum = SbAlloc(cDMFileMac*4, &cbNew)) == sbNil)
		{
		hprgfcDMEnum = (HP) NULL;
		DMFlags.fDMOutOfMemory = fTrue;
		return(fFalse);
		}

	Assert(cbNew != 0);
	cDMEnumMax = cbNew /4;   /* we may have pickup up a little extra */
	hprgfcDMEnum = HpOfSbIb(sbDMEnum,0);
	osfn = OsfnValidAndSeek(fnDMFcTable,0L);
	Assert(osfn != osfnNil);
	lpTmp = LpConvHp(hprgfcDMEnum);
	return(CchReadDoshnd (osfn, lpTmp, cDMFileMac*4) == cDMFileMac*4);
}




/* %%Function:Select1stCatLB %%Owner:CHIC */
Select1stCatLB()
{
	int rgIndex[2];

#define	HlbxFromLbw(hlbw)	((HLBX)GetWindowWord((HWND)(hlbw), 0))

	if (cDMFileMac > 0)
		{
		HWND hwnd;
		if ((hwnd = HwndOfTmc(tmcCatLB)) != NULL)
			{
			FSetFirstLbx(HlbxFromLbw(hwnd), 0); /* cause list box to scroll */
			rgIndex[0] = 0;
			rgIndex[1] = uNinchList;
			FSetTmcLargeVal(tmcCatLB, &rgIndex[0]); /* hilight item */
			}
		}

}


