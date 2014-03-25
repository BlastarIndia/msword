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
#include "file.h"
#include "dmdefs.h"
#include "debug.h"
#include "doslib.h"
#include "doc.h"
#include "ch.h"
#include "message.h"

#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"

#include "catalog.hs"
#include "catprog.hs"
#include "catsrch.hs"


#ifdef PROTOTYPE
#include "docman1.cpt"
#endif /* PROTOTYPE */

/* G L O B A L S */
struct DMQFLAGS     DMQFlags;
struct DMFLAGS      DMFlags;
struct DMQD         DMQueryD;
CHAR              **hstDMQPath = hNil;
int                 cDMFileMac = 0;
int                 cDMEnum = 0;    /* current index when narrowig search*/
int                 cDMEnumMac = 0;    /* mac used, valid outside of Doc Ret*/
int                 cDMEnumMax = 0;    /* max allocated, valid inside & out */
int                 fnDMEnum = fnNil;
int                 fnDMFcTable = fnNil;
HANDLE              hDMFarMem;
CHAR far           *DMFarMem;
long                cbDMFarMem;
CHAR              **rghszDMDirs[chstDMDirsMax];
int                 chszDMDirs = 0;
int                 ihszDMDirs;
FC HUGE            *hprgfcDMEnum;       /* the indices into the TMP file fnDMEnum */
SB                  sbDMEnum;            /* Sb of the above HUGE pointer */
struct FINDFILE     **hDMDTA;    /* search results from FFirst and FNext */
CHAR              **hszvsumdFile = hNil;
struct SUMD         vsumd;
int                 irgfcDMFilenameStart;
HWND                vhDlgCatSrhProgress = NULL;
CHAR               *pchTreeStart;
int                 fLeaf; /* for HBuildAONTree, was the last thing we found a Leaf? */
int                 iLeafCur = 0;


/* E X T E R N A L S */
extern FC HUGE         *hprgfcDMEnum; /* the indices into the TMP file fnDMEnum */
extern int              chszDMDirs;
extern int              ihszDMDirs;
extern struct DMQD      DMQueryD;
extern struct DMFLAGS   DMFlags;
extern struct DMQFLAGS  DMQFlags;
extern int              cDMFileMac;
extern int              cDMEnum;    /* current index when narrowig search*/
extern int              cDMEnumMac;    /* mac used, valid outside of Doc Ret*/
extern int              cDMEnumMax;    /* max allocated, valid inside & out */
extern int              fnDMEnum;
extern int              fnDMFcTable;
extern CHAR           **hstDMQPath;
extern struct FINDFILE  **hDMDTA;  /* search results from FFirst and FNext */
extern struct SUMD      vsumd;
extern CHAR	    	szDoc[];
extern struct MERR 	vmerr;

csconst struct 
{
	int cboffset;
	int ibst;
} 


rgOffsetIbst[] =
{
	{ offset(SUMD, hszTitle),     ibstAssocTitle },
	{ offset(SUMD, hszSubject),   ibstAssocSubject },
	{ offset(SUMD, hszAuthor),    ibstAssocAuthor },
	{ offset(SUMD, hszKeywords),  ibstAssocKeyWords },
	{ offset(SUMD, hszRevisor),   ibstAssocLastRevBy }
	};


#define iMacRgOffsetIbst 5


/* %%Function:DttmFromPdta %%Owner:chic */
long DttmFromPdta(pDTA)
struct FINDFILE *pDTA;
{
	struct DTTM dttm;

	dttm.mint = pDTA->mint;
	dttm.hr = pDTA->hr;
	dttm.dom = pDTA->dom;
	dttm.mon = pDTA->mon;
	dttm.yr = pDTA->yr + 80; /* DOS year started at 1980 */

	dttm.wdy = 0;
	return dttm;
}


/* %%Function:FDMEnumMore %%Owner:chic */
FDMEnumMore(fInit)
int fInit;
{
	int cchDMEnum = 0;
	int cch;
	CHAR *pchFnSort, *pchNm;
	FC fc;
	CHAR rgch[cchMaxFile];
	CHAR rgchFile[cchMaxFile];

	if (!FCkDMExpand() || !FNextDMFile(rgchFile, cchMaxFile, fInit))
		return fFalse; /* can't expand or end of list */

	Assert(CchSz(rgchFile) <= cchMaxFile);
	OemToAnsi(rgchFile, rgchFile);

	if ((DMFlags.dma != dma_Resort) && 
			(DMQFlags.grpf && !DMQueryTestSz(rgchFile)))
		return fTrue; /* file doesn't match */

	AssertH(hDMDTA);

/* get here if either the file passed or no need to test */
	if (DMFlags.dma == dma_NarrowSearch)
		{
		Assert(hprgfcDMEnum != (HP)NULL);
		hprgfcDMEnum[cDMFileMac++] = hprgfcDMEnum[cDMEnum-1];
		}
	else
		{ /* start from scratch or resort */
/* format of rgch :
	rgch[0]   = cch that follows
	rgch[1]   = fQuickSave
	rgch[2]   = CHAR of ihszDMDirs
	rgch[3].. = filename (no path, no null terminator)
	rgch[ichDMSortMin] = sort field;
*/
		rgch[2] = ihszDMDirs + '0';
		FreezeHp();
		pchNm = (*hDMDTA)->szFileName;
		cch = CchSz(pchNm) - 1 ;
		OemToAnsi(pchNm, &rgch[3]);
		MeltHp();
		/********
			We need to pad out to ichDMSortMin so sorting will at least see
		a \0 for the summary info even if there isn't any. Otherwise
		we are sorting garbage beyond the end of the array.
		********/
		cch += 3;
		Assert(ichDMSortMin >= cch);
		SetBytes(rgch+cch, chSpace, ichDMSortMin-cch);
		pchFnSort = rgch+ichDMSortMin;
		*pchFnSort = 0;

		if (FVSumdFromSz(rgchFile))
			{
			rgch[1] = vsumd.fQuickSave;
			cchDMEnum = ichDMSortMin+CchDMSI(pchFnSort)-1;
			}
		else if (DMFlags.fTmpFile)
			{
			return fTrue; /* skip tmp file */
			}
		else  if (DMFlags.dma != dma_Resort && DMFlags.fFileInUse)
			{
			ShowFileInUse(rgchFile);
			DMFlags.fIncompleteQuery=fTrue;
			return fTrue;
			}
		else
			{   /****
				either we tried to get summary info
			in DMQueryTestSz and failed, or we
			tried here and failed. See if we can
			fake the info from DOS
			*****/
			rgch[1] = fFalse;
			switch (DMFlags.dms)
				{
#ifdef DEBUG
			default:
				Assert(fFalse);
				return fFalse;
#endif /* DEBUG */
			case dmsName:
			case dmsAuthor:
			case dmsLastSavedBy:
				cchDMEnum = ichDMSortMin; /* nope */
				break;
			case dmsSize:
					{
					/****
					get the file size from DOS. 
					****/
					*((long unsigned *)pchFnSort) = (*hDMDTA)->cbFile;
					pchFnSort += sizeof(long);
					*pchFnSort = 0; /* null terminator */
					cchDMEnum = ichDMSortMin + sizeof(long);
					break;
					}
			case dmsRevisionDate:
			case dmsCreationDate:
				/****
				use the DOS date
				****/
				FreezeHp();
				*(long *)(pchFnSort) = DttmFromPdta(*hDMDTA);
				MeltHp();
				*(pchFnSort+sizeof(struct DTTM)) = 0;
				cchDMEnum = ichDMSortMin+sizeof(struct DTTM);
				break;
				} /* end of switch */
			}
		Assert(cchDMEnum != 0);
		rgch[0] = cchDMEnum;
		fc = FcAppendRgchToFn(fnDMEnum, rgch, cchDMEnum+1);
		if (vmerr.fDiskAlert || vmerr.fDiskWriteErr)
			return fFalse;
		Assert(hprgfcDMEnum != (HP)NULL);
		hprgfcDMEnum[cDMFileMac++] = fc;
		}

	return fTrue;
}




/**************************************************************************
	FSumdFromSz:
	This routine takes a file name and a pointer to a Summary Descriptor.
	It opens the file directly with the os, reads the header, finds the 
	Summary info at the end (if it exists) and  reads it in to a buffer.
	This buffer is then parsed into its component parts, these are placed
	on the heap, and pointers are stored in the Sumd.    
***************************************************************************/

/* %%Function:FSumdFromSz %%Owner:chic */
FSumdFromSz(szFile,pSumd)
CHAR *szFile;
struct SUMD *pSumd;
{
	int fn = fnNil;
	int fDeleteFn = fFalse;
	int fResult = fTrue;
	int doc;
	int fose;
	int i;
	struct STTB **hsttb;
	struct FCB *pfcb;
	CHAR *pst;
	CHAR **hsz;
	CHAR ***phsz;
	CHAR stzFile[ichMaxFullFileName];
	CHAR sz[cchMaxSz];

	DMFlags.fTmpFile = fFalse;

/* szFile is verified to be no larger than 120 characters (including null
terminator) in FNextDMFile where this is called. */
	SzToSt(szFile, stzFile);  /* copy out of heap and make it a stzFile */
	Assert(stzFile[0] + 1 < ichMaxFullFileName);
	stzFile[stzFile[0]+1] = '\0';

	if ((fn = FnFromOnlineSt(stzFile)) == fnNil)
		{
		fn = FnOpenSt(stzFile, fOstQuickRead, ofcDoc, &fose);
		DMFlags.fFileInUse = (fn == fnNil || fose == foseNoAccess);
		fDeleteFn = fTrue;
		}

	if (fn == fnNil)
		return fFalse;

	if ((pfcb = PfcbFn(fn))->fTemp || pfcb->fDMEnum)
		{
		DMFlags.fTmpFile = fTrue; /* don't list tmp files */
		return fFalse;
		}

	pSumd->cchDoc = pfcb->cbMac;

	if (!pfcb->fHasFib) /* non native file */
		{
		struct FINDFILE dta;
		CreateBlankSumd(pSumd);
		if (vmerr.fMemFail)
			fResult = fFalse;
		else
			{
			AnsiToOem(&stzFile[1], &stzFile[1]);
#ifdef DEBUG
			i = 
#endif
				FFirst(&dta, &stzFile[1], DA_NORMAL|DA_READONLY);
#ifdef DEBUG
			Assert(i == 0);
#endif
			pSumd->dttmCreation.lDttm = pSumd->dttmRevision.lDttm = 
					DttmFromPdta(&dta);
			pSumd->cchDoc = PfcbFn(fn)->cbMac;
			pSumd->fQuickSave = fFalse;
			}
		}
	else
		{
		struct DOP dop;
		struct FIB fib;
/* gather summary info */
		bltbh(HpchGetPn(fn, pn0), &fib, min(cbSector, cbFIB));
		pSumd->cchDoc = fib.ccpText;
		pSumd->fQuickSave = fib.fComplex;

/* Read document properties */
		SetFnPos(fn, fib.fcDop);
		Assert(offset(FIB, fcDop) < cbSector);
		ReadRgchFromFn(fn, &dop, min(fib.cbDop, cbDOP));
		pSumd->dttmRevision = dop.dttmRevised;
		pSumd->dttmCreation = dop.dttmCreated;

/* Associated string table */
		if (fib.cbSttbfAssoc > 0)
			{
			if (FNoHeap(hsttb = 
					HsttbReadSttbfFromFile(fn,fib.fcSttbfAssoc,fTrue,fFalse,0)))
				{
				fResult = fFalse;
				goto SumdExit;
				}
			AssertH(hsttb);

			for (i = 0; i < iMacRgOffsetIbst; i++)
				{
				GetSzFromSttb(hsttb, rgOffsetIbst[i].ibst, sz);

				if ((hsz = HszCreate(sz)))
					{
					phsz = (CHAR ***)(((CHAR *)pSumd) + rgOffsetIbst[i].cboffset);
					*phsz = hsz;
					}
				else
					{
					fResult = fFalse;
					break;
					}
				}
			FreeHsttb(hsttb);
			}

		}

SumdExit:
	if (fDeleteFn)
		{
		Assert(fn != fnDMEnum && fn != fnDMFcTable);
		DeleteFn(fn,fFalse);
		}
	return(fResult);
}




/****************************************************************************
	ShowFileInUse:
	This function outputs the message "Cannot read 'filename'"
****************************************************************************/

/* %%Function:ShowFileInUse %%Owner:chic */
ShowFileInUse(pchFilename)
CHAR *pchFilename;
{
	CHAR *pch;
	CHAR rgchBuffer[cchMaxSz];
	CHAR szClean[cchMaxFile];
	extern int vfAwfulNoise;


	DMFlags.fFileInUse = fFalse;
	if (!DMFlags.fSrhInProgress)
		return;
	pchFilename+= CchPath(pchFilename);
	SanitizeSz(pchFilename, szClean, cchMaxFile, fFalse);
	pch = szClean;
	BuildStMstRgw(mstCatCannotRead, &pch, rgchBuffer, cchMaxSz, hNil);
	StToSzInPlace(rgchBuffer);
	SetTmcText(tmcCatSrhFileInUse, rgchBuffer);
	vfAwfulNoise = fFalse;
	Beep();
/*Delay(50);*/
}


/******************************************************************
ParseDMQPath:
	This routine parses the Query Path into a series of dir or file names. 
	It then tacks on extentions and wildcards to put the pieces in a form
	suitable for calls to FFirst, and saves the nth dir of the path in a 
	heap string, pointed to by rghszDMDirs[n]. It then sorts the Dirs
	alphabetically so that the indexes can be used for sorting the filenames.
	******************************************************************/
/* %%Function:ParseDMQPath %%Owner:chic */
ParseDMQPath()
{
	CHAR *pchT,*pchPath,*pchSpace;
	int cchDir,cchT=0;
	int chszDirs=0;
	int cchPath = **hstDMQPath + 1;
	int ihsz;
	int junk;
	CHAR **hszDir;
	struct FINDFILE DTA;
	CHAR rgchDir[cchMaxFile];
	CHAR rgchNDir[cchMaxFile]; /* Normalized path */
	CHAR rgchPath[cchMaxDMQPath+1];

	Assert(chszDMDirs <= chstDMDirsMax);

	if (cchPath > cchMaxDMQPath)
		**hstDMQPath = cchMaxDMQPath;

	StToSz(*hstDMQPath, rgchPath);

	/* Convert to all upper case. */
	QszUpper(rgchPath);

	/* free old heap strings */
	for (ihsz = 0; ihsz < chszDMDirs; ihsz++)
		FreeH(rghszDMDirs[ihsz]);

	/* loop through path creating heap strings */
	pchPath = rgchPath;
	while (cchT < cchPath-1 && chszDirs < chstDMDirsMax)
		{
		SetBytes(rgchNDir, 0, cchMaxFile);
		while (*pchPath == chSpace)    /* skip leading spaces */
			{
			pchPath++;
			cchT++;
			}
		pchT = pchPath;
		while (*pchT != chComma && *pchT != chSemi && cchT < cchPath-1)
			{
			cchT++;
			pchT++;
			}

		/* pchPath points to start of path without leading space */
		/* pchT points to end of path possibily with trailing spaces */
		cchDir = pchT - pchPath;
		pchSpace = pchPath + cchDir;
		while (*(pchSpace-1) == chSpace 
			&& pchSpace > pchPath+1) /* eliminate trailing spaces */
			{
			cchDir--;
			pchSpace--;
			}
		/* protect overblt, 1 for null terminator */
		cchDir = min(cchDir, cchMaxFile-1); 
		bltb(pchPath, rgchDir, cchDir);  /* copy to buffer */
		rgchDir[cchDir] = 0;  /* terminate */

		/* we now have two cases. 1. This is a dir 2. This is a filespec */
		if (rgchDir[cchDir-1]=='\\')
			{           /* case 1. */
			/*****
			first, resolve any ../ in the path.
			*****/
			if (FCheckFileSpec(rgchDir,rgchNDir,nfoPathOK, &cchDir))
				{
				cchDir = min(cchDir, cchMaxFile - 6 /* "*.doc" */);
				AddFileSpec(rgchNDir, cchDir);
				}
			else
				goto LoopNext;
			}
		else
			{   /* case 2. terminate it */
			/*****
				check for the case of a dir without a trailing
			backslash. First see if there are any wildcards. If so then
			it has to be a filespec.
			*****/
			if (FntSz(rgchDir,cchDir,&junk,nfoPathWildOK)!=fntValidWild)
				{
				AnsiToOem(rgchDir, rgchDir);
				if (FFirst(&DTA,rgchDir,DA_SUBDIR)==0 &&
						(DTA.attribute & DA_SUBDIR))
					{   /* it's really a dir */
					OemToAnsi(rgchDir, rgchDir);
					if (FCheckFileSpec(rgchDir,rgchNDir,nfoPathOK,&cchDir))
						{
						if (cchDir > 3)   /* beware of a: ! */
							{
							cchDir = min(cchDir, cchMaxFile - 7 /* "\*.doc" */);
							rgchNDir[cchDir++] = '\\';
							}
						AddFileSpec(rgchNDir, cchDir);
						}
					else
						goto LoopNext;
					}
				else
					{
					AnsiToOem(rgchDir, rgchDir);
					goto CheckFileSpec;
					}
				}
			else 				
				{   /* we have a filespec */
CheckFileSpec:
				if (!FCheckFileSpec(rgchDir, rgchNDir, nfoPathOK, &cchDir))
					goto LoopNext;
				if (rgchNDir[cchDir-1] == '\\')
					{
					cchDir = min(cchDir, cchMaxFile - 6 /* "*.doc" */);
					AddFileSpec(rgchNDir, cchDir);
					}
				}
			}
	/* rgchNDir now holds a filespec */
	/* copy it into a heap string and put the hsz in rghszDMDirs */
		if ((hszDir = HszCreate(rgchNDir)) != hNil)
			rghszDMDirs[chszDirs++] = hszDir;
		else
			break;

LoopNext:
		pchPath = pchT+1;            /* step past ',' */
		cchT++;
		}  /* end of while loop */


	Assert(chszDirs <= chstDMDirsMax);
	if (chszDirs > 1)
		rghszQSort(0, chszDirs-1, rghszDMDirs, 1);

	chszDMDirs = chszDirs;    /* set global count of path pieces */
} /* end of ParseDMQPath */



/****************************************************************************
	CchTruncateFilename:
	This function takes a pointer to a full pathname and a pointer to the place it
	should put the new version. It then does the following operation on the name
	to reduce it to less than 40 chars.
	Case 1: The first and last directories in the path have no extention.
	Returns pathname of the form   C:\dirname1\...\dirname2\filename.ext
	which has maximum length of 37 chars.
	Case 2: One or both of the two dirs has an extention that makes it longer than
	8 chars, so the above form is too long. In this case the first dir 
	is left off, so the return form is C:\...\dirname2.ext\filename.ext  , 
	which has a max length of 32 chars.
****************************************************************************/

/* %%Function:CchTruncateFilename %%Owner:chic */
CchTruncateFilename(pchSrc,pchDest)
CHAR *pchSrc;
CHAR *pchDest;
{
	CHAR *pchStartLstDir;
	CHAR *pchEndFstDir;
	CHAR *pchEndLstDir;
	CHAR *pchEndFilename;
	int cchLast;
	int cchStart;

/* find the key points of the filename */

	pchEndFstDir=pchSrc+3;   /* one char past first slash */
	while (*pchEndFstDir!= '\\')
		pchEndFstDir++;
	pchEndFilename=pchEndFstDir;
	/* filename is either null terminated or has a space at the end. */
	while (*pchEndFilename!=0 && *pchEndFilename!=chSpace)
		pchEndFilename++;
	pchStartLstDir=pchEndFilename;
	/* step past filename */
	while (*pchStartLstDir!= '\\')
		pchStartLstDir--;
	pchStartLstDir--;  /* step past first backslash */
	pchEndLstDir=pchStartLstDir;
	/* step past first dir */
	while (*pchStartLstDir!= '\\')
		pchStartLstDir--;

	if ((pchEndLstDir-pchStartLstDir) > 8 || (cchStart=(pchEndFstDir-pchSrc)) > 11)
		{   /* names are too long, user form two */
		bltb(pchSrc, pchDest, 3);
		SetBytes(pchDest+3,chDot,3);
		bltb(pchStartLstDir,pchDest+6,
				(cchLast=pchEndFilename-pchStartLstDir+1));
		pchDest[cchLast+6]=0;
		return(cchLast+6);
		}
	else
		{
		bltb(pchSrc, pchDest, cchStart+1);
		SetBytes(pchDest+cchStart+1,chDot,3);
		bltb(pchStartLstDir, pchDest+cchStart+4, 
				(cchLast=pchEndFilename-pchStartLstDir+1));
		pchDest[cchStart+cchLast+4]=0;
		return(cchStart+cchLast+4);
		}
}


/***********************************************************************
	CompareDTTMDate:
	This function compares the date portions of two DTTMs. It returns:
	> 0 if Left is after Right.
	= 0 if Left is the same as Right.
	< 0 if Left is before Right.
***********************************************************************/

/* %%Function:CompareDTTMDate %%Owner:chic */
CompareDTTMDate(DTTMLeft,DTTMRight)
struct DTTM DTTMLeft, DTTMRight;
{
	int result;

	if ((result=DTTMLeft.yr-DTTMRight.yr)!=0)
		return(result);
	if ((result=DTTMLeft.mon-DTTMRight.mon)!=0)
		return(result);
	return(DTTMLeft.dom-DTTMRight.dom);
/* this can easily be extended to include time comparisons as well */
}


/************************************************************************
This version takes pointers to DTTMs, rather than DTTMs themselves.
************************************************************************/
/* %%Function:ComparepDTTMDate %%Owner:chic */
ComparepDTTMDate(pDTTMLeft,pDTTMRight)
struct DTTM *pDTTMLeft, *pDTTMRight;
{
	return(CompareDTTMDate(*pDTTMLeft, *pDTTMRight));
}


/*****************************************************************************
	UpdateFcDMEnum:
	This routine takes an index into hprgfcDMEnum for the fc to access fnDMEnum.
	It gets the file corresponding to the index, updates it's fQuickSave flag
	and sort field, writes the new string to the DMEnum TMP file, and resets 
	the FC in hprgfcDMEnum to point to the new location.
*****************************************************************************/
/* %%Function:UpdateFcDMEnum %%Owner:chic */
UpdateFcDMEnum(irgfc, fQuickSave)
int irgfc;
int fQuickSave;
{
	CHAR *pchT,*pchFile;
	int cch,cchSI = 0;
	CHAR rgchTemp[cchMaxDMFileSt+1];
	CHAR rgchFile[cchMaxFile];

	Assert(hprgfcDMEnum != (HP)NULL);
/* copy the filename off fnDMEnum */
	FQsSzFilenameFromDMFc(hprgfcDMEnum[irgfc],rgchFile,fTrue);

/* rgchFile now has full null terminated filename */
	rgchTemp[1] = fQuickSave;
	rgchTemp[2] = ihszDMDirs + '0';

	pchT = PchSkipPath(rgchFile);
	bltb(pchT, &rgchTemp[3], cch=(CchSz(pchT)-1));
/* refill to end of field with ' '*/
	cch += 3;
	SetBytes(rgchTemp+cch, chSpace, ichDMSortMin-cch);
	rgchTemp[ichDMSortMin] == 0;

/* get new summary info */
	if (FVSumdFromSz(rgchFile))
		{
		/* put the new summary info in the display string */
		cchSI=CchDMSI(&rgchTemp[ichDMSortMin]);
		Assert(cchSI >= 1); /* at least a null terminator */
		}
	else
		{
		Assert(!DMFlags.fTmpFile);
		cchSI = 1; /* null terminator for null summary info */
		}
	Assert(rgchTemp[ichDMSortMin+cchSI-1] == 0);
/* Can't use CchSz(rgchTemp+1) because the 1st byte fQuickSave can be 0 */
	rgchTemp[0] = ichDMSortMin+cchSI-1;
	hprgfcDMEnum[irgfc] = FcAppendRgchToFn(fnDMEnum, rgchTemp, rgchTemp[0]+1);
}


/**************************************************************************
	FQsSzFilenameFromDMFc:
	This function builds the full filename out of the enumeration string
	stored in the DM Tmp file. It returns whether or not the file was
	Quick Saved. 
***************************************************************************/

/* %%Function:FQsSzFilenameFromDMFc %%Owner:chic */
FQsSzFilenameFromDMFc(fc,pchFilename,fSetihsz)
FC fc;
CHAR *pchFilename;
int   fSetihsz;   /* should we set ihszDMDirs from this ? */
{
	int cch, iDMDir;
	CHAR *pchT;
	CHAR rgchTemp[ichDMSortMin+1];

/* the format of the string fetched from fnDMEnum is:
	1st byte   - TRUE if quicksaved  
	2nd byte   - index of the directory path
	3rd byte.. - null terminated filename
*/
	if ((cch = CchSzFromFc(fnDMEnum,fc,rgchTemp,ichDMSortMin)) <= 1)
		/* cch includes null char */
		return fFalse;

	Assert(cch <= ichDMSortMin);
	pchT = rgchTemp+cch-1;
	/* last possible char of filename */
	while (*pchT == chSpace || *pchT == 0)
		pchT--;       /* remove trailing spaces or padding 0s */
	*(++pchT) = 0;        /* terminate the new string */

	cch = pchT+1-&rgchTemp[2]; /* cchSzFilename */
	/* now get the pathname out of rghszDMDirs */
	/* The second char of the string is the index into rghszDMDirs + '0' */
	iDMDir = rgchTemp[1] - '0';
	Assert(iDMDir < chstDMDirsMax);
	FreezeHp();
	pchT = *(rghszDMDirs[iDMDir]);
	/* copy pathname, tack on filename */
	bltb(rgchTemp+2, pchFilename+CchCopySz(pchT, pchFilename), cch);
	MeltHp();
	/* pchFilename now has full null terminated filename */
	if (fSetihsz)
		ihszDMDirs = iDMDir;
	/* the first char of the string is FQuickSave */
	return((int)(rgchTemp[0]));
}


/* %%Function:FCheckFileSpec %%Owner:chic */
FCheckFileSpec(rgchDir, rgchNDir, nfo, pcchDir)
CHAR *rgchDir;
CHAR *rgchNDir;
int nfo;
int *pcchDir;
{
	int f = FNormalizeSzFile(rgchDir, rgchNDir, nfo);

	*pcchDir = CchSz(rgchNDir)-1;
	Assert(*pcchDir >= 0);
	return (f || *pcchDir != 0);
}


/* %%Function:AddFileSpec %%Owner:chic */
AddFileSpec(rgchNDir, cchDir)
CHAR *rgchNDir;
int cchDir;
{
	rgchNDir[cchDir++]= chDOSWildAll;   /* add *.doc  */
	bltb(szDoc,rgchNDir+cchDir,5);
}



/************************************************************************
FNextDMDir:
	This function tries to go on to the next dir in the Query Path
	and get the first file there. If it runs out of dirs or hits an
	invalid dir it returns fFalse, else it returns fTrue. 
	The pchFilename filled is a sz.
	pchFilename is in OEM character set. 
*************************************************************************/

/* %%Function:FNextDMDir %%Owner:chic */
FNextDMDir(pchFilename, cchBuf)
CHAR *pchFilename;
int cchBuf;
{
	struct FINDFILE *pDTA;
	CHAR *pchNew, *pchOld;

	Assert(chszDMDirs <= chstDMDirsMax);
	while (++ihszDMDirs < chszDMDirs)
		{
		int cchPath;
		int cchT;

	/* If the new path is the same as the old path, then throw away old path */
		Assert(ihszDMDirs > 0 && ihszDMDirs < chstDMDirsMax);
		pchNew = *(rghszDMDirs[ihszDMDirs]);
		pchOld = *(rghszDMDirs[ihszDMDirs-1]);

/* Note the string in rghszDMDirs may still have the filespec at the end,
	use CchPath to get just the pathname part. */
		if ((cchT=CchPath(pchNew)) == CchPath(pchOld) &&
				FEqNcRgch(pchNew, pchOld, cchT))
			{
		/* Same dir , so collapse rghszDMDirs and free unused heap pointer */
			ihszDMDirs--;
			chszDMDirs--;
			FreeH(rghszDMDirs[ihszDMDirs]);
			blt(&rghszDMDirs[ihszDMDirs+1], &rghszDMDirs[ihszDMDirs], 
					chszDMDirs - ihszDMDirs);
			}
		Assert(CchSz(*(rghszDMDirs[ihszDMDirs])) <= cchBuf);
		CchCopySz(*(rghszDMDirs[ihszDMDirs]), pchFilename);
		AssertH(hDMDTA);
		pDTA = *hDMDTA;
		FreezeHp();
		AnsiToOem(pchFilename, pchFilename);
		if (FFirst(pDTA, pchFilename, 1) == 0)
			{ /* this dir has a file that matched. Get full name and return */
			cchPath = CchPath(*(rghszDMDirs[ihszDMDirs]));
			if (cchPath + CchSz(pDTA->szFileName) > cchBuf)
				{
				MeltHp();
				return(fFalse);
				}
			CchCopySz(pDTA->szFileName, pchFilename+cchPath);
			MeltHp();
			return(fTrue);
			}
		MeltHp();
		}
	return(fFalse);  /* we are out of dirs */
}


/************************************************************************
FNextDMFile:
	This function tries to get the next file to be checked in one of
	two ways. In the normal case, it tries to go on to the next file in 
	the current directory of the Query Path. If it runs out of files it 
	calls FNextDMDir to go on to the next one. When it hits the end of the
	last directory, it returns FALSE, else it returns fTrue.
	If however, we do not have to start from scratch, then we are using 
	the old list as our search list. In this case it gets the next file from the
	old array, inc'ing the count of where we are for next time. 
	It returns FALSE when it reaches cDMEnumMac, the count
	of files in the old list.
	Expect OEM char in pchFilename.
*************************************************************************/
/* %%Function:FNextDMFile %%Owner:chic */
FNextDMFile(pchFilename, cchBuf, fInit)
CHAR *pchFilename;   /* place to put full filename */
int cchBuf;          /* size of buffer pointed to by pchFileName */
int fInit;           /* Is this the first call of this query? */
{
	CHAR *pchT;
	int cchPath;
	CHAR rgchDir[cchMaxFile];

	AssertH(hDMDTA);
	if (DMFlags.dma == dma_StartFromScratch)
		{
		cDMEnum++;
		if (fInit)
			{
			Assert(CchSz(*(rghszDMDirs[0])) <= cchMaxFile);
			AnsiToOem(*(rghszDMDirs[0]), rgchDir);
			if (FFirst(*hDMDTA, rgchDir, 1) != 0)
				return(FNextDMDir(pchFilename, cchBuf));
			}
		else
			{
			if (FNext(*hDMDTA)!=0)
				return(FNextDMDir(pchFilename, cchBuf));
			}
	/****
	If we got this far either FFirst or FNext worked. Now put the full
	filename in pchFilename for later use.
	****/
		FreezeHp();
		Assert(ihszDMDirs < chstDMDirsMax);
		pchT= *(rghszDMDirs[ihszDMDirs]);
/* Note the string in rghszDMDirs still have the filespec at the end,
	use CchPath to get just the pathname part. */
		Assert(CchSz(pchT) <= cchBuf);
		AnsiToOem(pchT, pchFilename); /* copy pathname */
		cchPath = CchPath(pchT);
		if (cchPath + CchSz((*hDMDTA)->szFileName) > cchBuf)
			{
			MeltHp();
			return fFalse;
			}
		CchCopySz((*hDMDTA)->szFileName, pchFilename+cchPath);
		MeltHp();
		return(fTrue);
		}
	else
		{
		if (cDMEnum>=cDMEnumMac)
			return(fFalse);
	/*****
	At this point we need to get the filename out of the enumeration
	string
	*****/
		Assert(hprgfcDMEnum != (HP)NULL);
		FQsSzFilenameFromDMFc( hprgfcDMEnum[cDMEnum++], pchFilename, fTrue);
		AnsiToOem(pchFilename, pchFilename);
		FFirst(*hDMDTA, pchFilename, 1);
		return(fTrue);
		}
}


/***********************************************************************
	CchPath:
	This function returns the length of the path portion of sz.
************************************************************************/
/* %%Function:CchPath %%Owner:chic */
CchPath(sz)
CHAR *sz;
{
	return (PchSkipPath(sz)-sz);
}


