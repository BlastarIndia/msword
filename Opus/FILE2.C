/* F I L E 2 . C */
/*  Non-core file routines */

#ifdef PCJ
/* #define SHOWFC */
#define DFILEPATHS
#endif /* PCJ */

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOSYSMETRICS
#define NOBITMAP
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NOCTLMGR
#define NODRAWTEXT
#define NOFONT
#define NOGDI
#define NOHDC
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMSG
#define NOPEN
#define NOPOINT
#define NORECT
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
#include "heap.h"
#include "doslib.h"
#include "doc.h"
#include "file.h"
#include "winddefs.h"
#include "props.h"
#include "fkp.h"
#include "debug.h"
#include "error.h"


extern int              fnMac;
extern int              cfnTMP;         /* current temporary file number */
extern struct BPTB      vbptbExt;
extern struct FKPD      vfkpdChp;
extern struct FKPD      vfkpdPap;
extern struct FKPD      vfkpdText;
extern struct MERR      vmerr;
extern int              docMac;
extern int              vdocTemp;
extern int              vdocScratch;
extern FC               vfcSpec;
extern struct DOD       **mpdochdod[];
extern TS               tsMruOsfn;
extern struct FCB       **mpfnhfcb[];
extern int              vfScratchFile;
extern struct EFPI  	dnfpi[];
extern CHAR	    	szEmpty[];
extern CHAR	    	szApp[];
extern CHAR	    	szDoc[];
extern BOOL             vfInsertMode;



/* F C  M A C  F N */
/*  Return the fcMac of fn according to the file system (not too interesting
for native format files).  Return fcNil if cannot seek.
*/
/* %%Function:FcMacFn %%Owner:peterj */
FC FcMacFn(fn)
int fn;
{
	FC fcMac;
	unsigned osfn;

	Assert( (fn != fnNil) && mpfnhfcb [fn] != NULL);

	if ((osfn = OsfnValidAndSeek(fn,0L)) == osfnNil)
		return fcNil;
	else  if ((fcMac = DwSeekDw(osfn, 0L, SF_END)) < 0L)
		{
		DiskError(eidSDE, fn, "FcMacFn");
		return fcNil;
		}

	return fcMac;
}



/* %%Function:FCloseFn %%Owner:peterj */
FCloseFn( fn )
int fn;
{   /* Close file fn if it is currently open */
	struct FCB *pfcb = PfcbFn(fn);

	Assert( fn != fnNil && mpfnhfcb [fn] != NULL );
	if (pfcb->osfn != osfnNil)
		{
		/* Close may fail if windows already closed the file for us,
			but that's OK */
		FCloseDoshnd( pfcb->osfn );
		pfcb->osfn = osfnNil;
		}
	return fTrue;
}




/* %%Function:OpenEveryHardFn %%Owner:peterj */
OpenEveryHardFn()
	{   /* For each fn representing a file on nonremoveable media,
		try to open it.  It is not guaranteed that any or all such files
		will be open on return from this routine -- we are merely attempting
	to assert our ownership of these files on a network by keeping them open */

	extern int fnMac;
	int fn;
	struct FCB **hfcb;

	for ( fn = 0; fn < fnMac; fn++ )
		if ((hfcb = mpfnhfcb [fn]) != NULL)
			{
			struct FCB *pfcb = *hfcb;

			if (pfcb->osfn == osfnNil && pfcb->ofh.fFixedDisk)
				FAccessFn( fn, 0 );
			}
}





/* %%Function:FCreateStFile %%Owner:peterj */
FCreateStFile( stFile )
CHAR *stFile;
	{       /*  Create a new, unique file.
		Return the name in szFile.
		Leaves the file open
		Returns TRUE on success, FALSE on failure.
		If stFile begins "X:...", creates the file on the specified drive;
	otherwise, creates the file on a drive of Windows' choice */
	/* ERROR reporting is the responsibility of the caller; however,
		this routine will report a WARNING if the temp file creation
		could not be done on the default directory.  */


	extern CHAR stDOSPath[];
	int tf;
	CHAR szT [ichMaxFile];

	tf = ((stFile [0] > 1 && stFile [2] == ':') ? stFile [1] | TF_FORCEDRIVE : 0);

	if ( !GetTempFileName( (BYTE) tf, (LPSTR)szDoc + 1, 0, (LPSTR)szT ))
		{
		if (!tf)
			{
			ErrorEid (eidCantCreateTempFile, "FCreateStFile");
			UpdateStDosPath();
			if (!GetTempFileName( TF_FORCEDRIVE | stDOSPath [1],
					(LPSTR)szDoc + 1, 0, (LPSTR)szT))
				return fFalse;
			}
		else
			return fFalse;
		}

	/* Succeeded in creating file */

	SzToStInPlace( szT );

	return FNormalizeStFile( szT, stFile, nfoNormal);
} /* end of  F C r e a t e S t F i l e  */



/* %%Function:FEnsureOnLineFn %%Owner:peterj */
FEnsureOnLineFn( fn )
int fn;
	{       /* Ensure that file fn is on line (i.e. on a disk that is accessible).
	Return TRUE if we were able to guarantee this, FALSE if not */

	struct FCB *pfcb = PfcbFn(fn);

	Assert(fn != fnNil && mpfnhfcb[fn] != NULL);

	if (pfcb->ofh.fFixedDisk)
		/* If it's on nonremovable media, we know it's on line */
		return fTrue;
	else
		/* otherwise verify */
		return OsfnValidAndSeek(fn,0L) != osfnNil;
}


/* S E T  E O F  F N  - sets end of file */
/* NOTE: under DOS, can't reduce file size; we just change the FCB fields */
/* %%Function:SetEofFn %%Owner:peterj */
SetEofFn(fn, fc)
int fn;
FC fc;
{
	struct FCB *pfcb = PfcbFn(fn);

	pfcb->cbMac = pfcb->fcPos = fc;
}


/* K I L L  E X T R A  F N S */
/* %%Function:KillExtraFns %%Owner:peterj */
KillExtraFns()
{
	MarkAllReferencedFn();
	DeleteNonReferencedFns(0);
}


/* M A R K   A L L  R E F E R E N C E D  F N */
/* %%Function:MarkAllReferencedFn %%Owner:peterj */
MarkAllReferencedFn()
{
	int fn;
	struct FCB **hfcb, *pfcb;
	int doc;
	int ifoo;
	struct DOD **hdod, *pdod;
	struct PCD *ppcd;
	struct PCD pcd;
	struct SED sed;
	struct PLC **hplcpcd;
	struct PLC **hplcsed;
	struct CA caT;

/* remove any references that might be in vdocTemp */
/* for WIN, remove from vdocScratch  too */


	DisableInval();

	if (vdocTemp != docNil)
		{
		PdodDoc(vdocTemp)->doc = docScrap;   /* ensure valid chain */
		FDelete(PcaSetWholeDoc(&caT, vdocTemp));
		}

#ifdef WIN
	if (vdocScratch != docNil)
		{
		Assert (!fDocScratchInUse);
		FDelete(PcaSetWholeDoc(&caT, vdocScratch));
		}
#endif /* WIN */

	EnableInval();

	for (fn = 1; fn < fnMac; fn++)
		if ((hfcb = mpfnhfcb[fn]) != hNil)
			{
			/* dictionaries are not closed */
			pfcb = *hfcb;
#ifdef MAC
			pfcb->fRefDocScrap = fFalse;
			pfcb->fRefNonScrap = !pfcb->fDoc && !pfcb->fGlsy && !pfcb->fTemp;
#else /* WIN */
			pfcb->fRef = !pfcb->fDoc && !pfcb->fTemp;
#endif
			}

	for (doc = 1; doc < docMac; doc++)
		{
		if ((hdod = mpdochdod[doc]))
			{
			int iMac;
			int fnT;
			char *pfRef;
			char rgfRef[fnMax];

			FreezeHp();  /* for pdod */
			if (!(pdod = *hdod)->fShort && pdod->fn != fnNil)
				PfcbFn(pdod->fn)->fRef = fTrue;

			hplcpcd = pdod->hplcpcd;
			iMac = IMacPlc(hplcpcd);
			SetBytes(&rgfRef, 0, fnMax);
			MiscPlcLoops(hplcpcd, 0, iMac,
					&rgfRef, 2 /* MarkAllReferencedFn */);
			Assert(fnNil == 0);
			for (fnT = 1, pfRef = &rgfRef[1]; fnT < fnMac; fnT++)
				if (*pfRef++)
					PfcbFn(fnT)->fRef = fTrue;

			if (!pdod->fShort)
				{
				hplcsed = pdod->hplcsed;
				AssertH(hplcsed);
				iMac = IMacPlc(hplcsed);
				for (ifoo = 0; ifoo < iMac; ifoo++)
					{
					GetPlc(hplcsed, ifoo, &sed);
					if (sed.fn != fnNil && sed.fcSepx != fcNil)
						PfcbFn(sed.fn)->fRef = fTrue;
					}
				}
			MeltHp();
			}
		}
}



#ifdef MAC
/* M A R K   F C B   R E F */
/* %%Function:MarkFcbRef %%Owner:UNUSED */
MarkFcbRef(fn, doc)
int fn;
int doc;
{
	struct FCB *pfcb;

	if (fn != fnNil)
		{
		pfcb = PfcbFn(fn);
		if (doc == docScrap)
			pfcb->fRefDocScrap = fTrue;
		else
			pfcb->fRefNonScrap = fTrue;
		}
}


#endif /* MAC */


/* D E L E T E  N O N  R E F E R E N C E D  F N S */
/* %%Function:DeleteNonReferencedFns %%Owner:peterj */
DeleteNonReferencedFns(volSpec)
int volSpec;  /* Opus doesn't need this */
{
/* delete all unused temp files; closed all unused doc files */

	int fn;
	struct FCB **hfcb, *pfcb;
	struct PLCBTE **hplcbteChp, **hplcbtePap, **hplcbte;

	extern int vfnNonDel;
#ifdef RSH
	extern int vfnUa;
#endif /* RSH */

/* handle all non-scratch files */
	for (fn = fnScratch + 1; fn < fnMac; fn++)
		{
		if ((hfcb = mpfnhfcb[fn]) == hNil)
			continue;
		pfcb = *hfcb;
		if (pfcb->fRef)
			continue;
		if (fn == vfnNonDel)  /* used by rtfin */
			continue;
#ifdef RSH
		if (fn == vfnUa) /* used by user-action measurement code */
			continue;
#endif /* RSH */

		DeleteFn(fn, pfcb->fTemp);
		}

/* reset the scratch file if nothing references it */
	Assert(mpfnhfcb[fnScratch] != 0);
	pfcb = PfcbFn(fnScratch);
#ifdef MAC
	Assert(pfcb->vol == vvolSystem);
#endif
	/* Since we can have text on the scratch file which is not
		referenced by any doc during PrintMerge, don't reset
		scratch file in this case. */
	if (!pfcb->fRef
			&& (!vfInsertMode)
			&& pfcb->cbMac > 0 )
		{
		ReportSz("deleting scratch file contents");
		vfcSpec = fcNil;
		DeleteFnHashEntries(fnScratch);
		pfcb = PfcbFn(fnScratch); /* reload pointer */
		hplcbteChp = pfcb->hplcbteChp;
		hplcbtePap = pfcb->hplcbtePap;

		/* using FOpenPlc guarantees no allocations will fail */
		FOpenPlc(hplcbteChp, 0, -IMacPlc(hplcbteChp));
		PutCpPlc(hplcbteChp, 0, fc0);
		FOpenPlc(hplcbtePap, 0, -IMacPlc(hplcbtePap));
		PutCpPlc(hplcbtePap, 0, fc0);

		if (vmerr.fScratchFileInit)
			{
			SetEofFn(fnScratch, fc0);
			vfScratchFile = fTrue;
			}
		vmerr.fDiskEmerg = fFalse;
		pfcb = PfcbFn(fnScratch);	/* reload pointer */
		pfcb->cbMac = 0;
		pfcb->pnXLimit = pn0;
		pfcb->nFib = nFibCurrent;
/* the scratch file will start with cbMac = 0. The fkpd's will be set
up so that any insertion will start a new page */
		SetBytes(&vfkpdChp, 0, sizeof(struct FKPD));
		SetBytes(&vfkpdPap, 0, sizeof(struct FKPD));
		SetBytes(&vfkpdText, 0, sizeof(struct FKPDT));
		vfkpdPap.bFreeFirst = 1;
		/* .bFreeLim = 0; .fcFirst = 0 */
		vfkpdChp.bFreeFirst = 1;
		/* .bFreeLim = 0; .fcFirst = 0 */
		vfkpdText.pn = pnNil;
		StandardChp(&vfkpdChp.chp);
		}
}


/* D E L E T E   F N */
/* %%Function:DeleteFn %%Owner:peterj */
DeleteFn(fn, fDelete)
int fn;
int fDelete;    /* delete the file */
{
/* Remove all file cache references for file fn.  Close and optionally delete
	file.  Mark this fn as reusable.  Note that we free the fn even if we
	couldn't actually delete the file from the disk for whatever reason. */
	struct FCB *pfcb = PfcbFn(fn);
	struct PLCBTE **hplcbteChp = pfcb->hplcbteChp;
	struct PLCBTE **hplcbtePap = pfcb->hplcbtePap;
	int vol;
	char st[cchMaxFile];

/* save file description if we're deleting it */
	if (fDelete)
		{
		CopySt(pfcb->stFile, st);
		FEnsureOnLineFn(fn);
		}

/* delete all references to this file in the internal and external caches */
	DeleteFnHashEntries(fn);

/* kill the fn */
	if (FCloseFn(fn))
		{
		FreeHplc(hplcbteChp);
		FreeHplc(hplcbtePap);
		FreeCls(clsFCB, fn);
		}

/* the sad truth is that we couldn't care less whether the file is deleted;
	better to let the user see temp files than warn him the delete failed */
	if (fDelete)
		EcOsDelete(st, vol);
}       /* end DeleteFn */



/* %%Function:EcOsDelete %%Owner:peterj */
EcOsDelete(stFile, vol)
CHAR stFile[];
int vol; /* unused */
	{   /* Delete the file stFile, return error code or fpeNoErr if no error.
		return ecNoErr(0) if the file was successfully deleted or if it failed
				because the file could not be found; an error code otherwise.
*/

	int ec;

	StToSzInPlace( stFile );
	ec = EcDeleteSzFfname( stFile );
	SzToStInPlace( stFile );
	if (ec == ecFnfError)
		/* Failure to delete because the file was not found is considered
			a success at getting the file out of the way */
		ec = ecNoErr;

	return ec;
}


/* D E L E T E  F N  H A S H  E N T R I E S */
/* %%Function:DeleteFnHashEntries %%Owner:peterj */
NATIVE DeleteFnHashEntries(fn)
int fn;
{
	int ibp;
	struct BPS HUGE *hpbps;

	for (ibp = 0, hpbps = vbptbExt.hpmpibpbps;
			ibp < vbptbExt.ibpMac; ibp++, hpbps++)
		if (hpbps->fn == fn)
			DeleteIbpHashEntry(ibp);

	Debug(vdbs.fCkBptb ? CkPbptb(&vbptbExt, fFalse) : 0);
}



#ifdef DEBUG /* Coded in line if !DEBUG */
/* %%Function:C_PnWhoseFcGEFc%%Owner:peterj */
NATIVE PN C_PnWhoseFcGEFc(fc) /* WINIGNORE - DEBUG only */
FC	fc;
{
	return ((fc + (cbSector - 1)) >> shftSector);
}


#endif /* DEBUG */


/* F  C H E C K  F C  L E G A L */
/* %%Function:FCheckFcLegal %%Owner:peterj */
FCheckFcLegal (fc)
FC fc;
{
#ifdef SHOWFC
	CommSzLong (SzShared("FCheckFcLegal: fc = "), fc);
#endif /* SHOWFC */

	if (fc >= fcMax)
		{
		ErrorEid(eidFcTooBig, " FCheckFcLegal");
		vmerr.fDiskAlert = fTrue;
		return fFalse;
		}
	return fTrue;
}


csconst char stDotSlash[] = St(".\\");

/* F  F I N D  F I L E  S P E C */
/*  Search the paths specified in grpfpi (in order specified) looking
	for stShort (may have wild cards iff nof & nofWildOK).  If found
	return true and full path to file in stFull.  stFull is undefined
	if failed.
*/

/* %%Function:FFindFileSpec %%Owner:peterj */
FFindFileSpec (stShort, stFull, grpfpi, nfo)
CHAR *stShort, *stFull;
unsigned grpfpi;
int nfo;
{
	BOOL fHasPath;
	unsigned fpi;
	int shftFpi = 0;
	struct FNS fns;
	struct FINDFILE ffl;
	CHAR st[ichMaxFile];
	CHAR rgchT[ichMaxFile];

#ifdef DFILEPATHS
	CommSzSt(SzShared("FFindFileSpec: stShort = "), stShort);
	CommSzLong(SzShared("grpfpi = "), (long)grpfpi);
#endif /* DFILEPATHS */

	fHasPath = (PchInSt(stShort, '\\') != NULL ||
			PchInSt(stShort, '/') != NULL ||
			PchInSt(stShort, ':') != NULL);

	if (!FNormalizeStFile(stShort, st, nfo))
		return fFalse;
	StFileToPfns(st, &fns);

	while (shftFpi <= 12 && (fpi = (grpfpi >> shftFpi) & 0xf) != fpiNil)
		{
		Assert(fpi != fpiOwn || !shftFpi);
		shftFpi += 4;

	/* get path from fpi */
		if (fpi == fpiCurrent)
			/* yes, .\ is necessary.  this function needs to supply an
				explicit path to FNormalizeStFile. */
			CopyCsSt(stDotSlash, fns.stPath);
		else  if (fpi == fpiOwn)
			{
			if (!fHasPath)
				continue; /* file does not have its own! */
			}
		else  if (!FGetStFpi(fpi, fns.stPath))
			continue;

		/* see if valid & file exists */
		PfnsToStFile(&fns, st);
		if (FNormalizeStFile(st, stFull, nfo))   /* name is legal */
			{
			StToSz(stFull, rgchT);
			AnsiToOem(rgchT, rgchT);
#ifdef DFILEPATHS
			CommSzSz(SzShared("Trying: "), rgchT);
#endif /* DFILEPATHS */
			if (!FFirst(&ffl, rgchT, (nfo & nfoPathOK ? DA_SUBDIR : 0)))
				{     /* file exists */
				if (nfo & nfoWildOK)
					{	/* substitute actual name for wildcard */
					struct FNS fnsT;

					OemToAnsi( ffl.szFileName, rgchT );
					SzToStInPlace( rgchT );
					StFileToPfns( stFull, &fns );
					StFileToPfns( rgchT, &fnsT );
					CopySt(fnsT.stShortName, fns.stShortName);
					CopySt(fnsT.stExtension, fns.stExtension);
					PfnsToStFile( &fns, stFull );
					}
#ifdef DFILEPATHS
				CommSzSt(SzShared("File found: "), stFull);
#endif /* DFILEPATHS */
				return fTrue;
				}
			}
		}

	return fFalse;
}



/* F  G E T  S T  F P I */
/*  Return the stPath for fpi.  False if no path available.  st must have
	at least ichMaxPath characters.
*/

/* %%Function:FGetStFpi %%Owner:peterj */
FGetStFpi (fpi, st)
int fpi;
CHAR *st;
{
	struct EFPI * pefpi = &dnfpi[fpi];
	CHAR **hPath;

	Assert (fpi < fpiStoreMax);

	if (!pefpi->fInit)
		InitFpi(fpi);

	if (pefpi->fNone)
		return fFalse;

	hPath = HExpandHc(pefpi->hcPath);
	CopySt(*hPath, st);
	return fTrue;
}



csconst CHAR mpfpistName[][] =
{
	StKey("DOT-PATH",DOTPATH),
			StKey("INI-PATH",INIPATH),
			StKey("UTIL-PATH",UTILPATH)
};


csconst int mpfpieid[] =
{
	eidBogusDotPath,
			eidBogusIniPath,
			eidBogusUtilPath,
			eidNull
};


/* I N I T  F P I */
/*  Find the path for fpi and store it in dnfpi.
*/

/* %%Function:InitFpi %%Owner:peterj */
InitFpi(fpi)
int fpi;
{
	struct EFPI *pefpi = &dnfpi[fpi];
	int cch;
	CHAR ch, **hPath;
	CHAR rgchName[50], rgchPath[ichMaxFile], rgchNormal[ichMaxFile];

	Assert(fpi < fpiStoreMax);

	if (pefpi->fInit)
		return;

	pefpi->fInit = fTrue;

#ifdef DFILEPATHS
	CommSzNum(SzShared("InitFpi: fpi = "), fpi);
#endif /* DFILEPATHS */

	/* get a path */

	if (fpi == fpiProgram)
		/* location of opus.exe */
		CchGetProgramDir(rgchPath, ichMaxFile);

	else
		/* get path from win.ini profile */
		{
		bltbx((CHAR FAR *)mpfpistName[fpi], (CHAR FAR *)rgchName, 
				*mpfpistName[fpi]+1);
		StToSzInPlace(rgchName);
		GetProfileString((LPSTR)szApp, (LPSTR)rgchName, (LPSTR)szEmpty,
				(LPSTR)rgchPath, ichMaxFile);
		}

	if ((cch = CchSz(rgchPath)-1) != 0)
		{
		if ((ch = rgchPath[cch-1]) != ':' && ch != '\\' && ch != '/')
			{
			rgchPath[cch++] = '\\';
			rgchPath[cch] = 0;
			}

#ifdef DFILEPATHS
		CommSzSz(SzShared("Trying: "), rgchPath);
#endif /* DFILEPATHS */

		if (FNormalizeSzFile(rgchPath, rgchNormal, nfoPathOK|nfoPathCk)
				&& (cch = CchSz(rgchNormal)) < ichMaxPath
				&& (hPath = HAllocateCb(cch)) != hNil)
			{
			SzToSt(rgchNormal, *hPath);
			pefpi->hcPath = HcCompactH(hPath);
			Assert(hPath == HExpandHc(pefpi->hcPath));
			return;
			}

		/* failure: string too long, bogus directory, no heap */
		Assert(fpi != fpiProgram || vmerr.fMemFail);
		ErrorEid(mpfpieid[fpi], "FInitFpi");
		}

	pefpi->fNone = fTrue;
}




/* %%Function:CchGetProgramDir %%Owner:peterj */
int CchGetProgramDir(sz, cchMax)
CHAR    *sz;
int      cchMax;
{
	CHAR    *pch;

	extern HANDLE    vhInstance;

	pch = sz + GetModuleFileName(vhInstance, (LPSTR) sz, cchMax);
	while (*pch != '\\' && *pch != '/')
		{
		pch--;
		}
	*(++pch) = '\0';

	return (pch - sz);
}



#ifdef DEBUG
/* %%Function:DiskErrorProc %%Owner:peterj */
DiskErrorProc(eid, fn, szFunc)
CHAR *szFunc;
#else
DiskErrorProc(eid, fn)
#endif
int eid;
int fn;
{
	char szDrive[3];
	szDrive[0] = ChUpper(PfcbFn(fn)->stFile[1]);
	szDrive[1] = ':';
	szDrive[2] = 0;

#ifdef BATCH
	if (vfBatchMode)
#ifdef DEBUG
		BatchModeError(SzShared("DiskError! "), szFunc, eid, 0);
#else
	BatchModeError(SzShared("DiskError! "), NULL, eid, 0);
#endif /* DEBUG */
#endif /* BATCH */

#ifdef DEBUG
	ErrorEidWProc(eid, eid == eidDiskFull ? szDrive : PfcbFn(fn)->stFile,
			szFunc);
#else
	ErrorEidWProc(eid, eid == eidDiskFull ? szDrive : PfcbFn(fn)->stFile);
#endif /* DEBUG */
}



/*  THE FOLLOWING CAME FROM FILE.C */

extern int            vibpProtect; /* protected bp in Ext cache */
struct BPS HUGE *	vhpmpibpbps; /* comm. variable for SortSiftUpIbp */

/* F  F L U S H  F N */
/* Write all dirty pages in file fn to the next level of storage -
to the external cache or to the disk. Return true iff successful.
*/
/* %%Function:FFlushFn %%Owner:peterj */
FFlushFn(fn)
int fn;
{
	int iibpMac, ibp, iibp;
	int iibpT, ibpOld;
	struct BPS HUGE *hpbps;
	struct BPS HUGE *hpmpibpbps;
	LPRGBPEXT lprgbpExt;
	BOOL fRepeat;
	int ec;
	char HUGE *hp;
	int ibpProtectCur;
	int fReturn;
#define iibpMax 100
	int rgibp[iibpMax];
	char far *lpbpExt;

#ifdef WIN
/* If we postponed creation of the scratch file, now is the time to do it */

	if (fn == fnScratch)
		{
		if (vmerr.fPostponeScratch)
			{
			CHAR st [ichMaxFile];

			st [0] = 0;                 /* default location */
			/* we tried, even if we failed */
			vmerr.fPostponeScratch = fFalse;
			Assert(vfScratchFile);  /* these are set when fPostpone is set */
			Assert(vmerr.fScratchFileInit);
			if (!FCreateStFile( st ))
				{
				vmerr.fScratchFileInit = fFalse; /* there is no scratch file */
				vfScratchFile = fFalse;  /* we cannot write to the scratch file */
				ErrorEid(eidCantCreateTempFile, "FFlushFn");
				return fFalse;
				}
			FreezeHp(); /* safe because space is pre-reserved */
			AssertDo(FNameFn( fn, st ));
			MeltHp();
			}
		else  if (!vfScratchFile)
			/* there is no scratch file to write to, fail now */
			return fFalse;
		}
#endif /* WIN */
#ifdef DEBUG
	/* BLOCK: make sure the fDirty bit doesn't lie! */
		{
		struct BPS HUGE * hpbpsT = vbptbExt.hpmpibpbps;
		struct BPS HUGE * hpbpsMacT = hpbpsT + vbptbExt.ibpMac;
		BOOL fDirtyT = fFalse;
		for ( ;hpbpsT < hpbpsMacT; hpbpsT++)
			if (hpbpsT->fn == fn)
				fDirtyT |= hpbpsT->fDirty;
		Assert(!fDirtyT == !PfcbFn(fn)->fDirty);
		}
#endif /* DEBUG */
/* if we're certain that there are no dirty pages for this file, we needn't
	scan */
	if (!PfcbFn(fn)->fDirty)
		return (fTrue);

/* when we sort the dirty buffers, the current protected page contents may
	moved to another buffer slot. ibpProtectCur will keep track of where the
	protected contents are moved so we can move them back to vibpProtect
	when we're finished. */
	ibpProtectCur = vibpProtect;
LRepeat:
	/* set global for benefit of SortIbp */
/* WARNING: proc calls outside this module will invalidate vhpmpibpbps */
	hpmpibpbps = vbptbExt.hpmpibpbps;
	vhpmpibpbps = hpmpibpbps;

	fRepeat = fFalse;
/* look for dirty cached pages that deal with file fn */
/* set up array of ibp's so that iibp's are sorted by pn */
	iibpMac = 0;
	hpbps = HpbpsIbp(0);
		{{ /* NATIVE - Short loop for creating rgibp in FFlushFn */
		for (ibp = 0; ibp < vbptbExt.ibpMac; ibp++, hpbps++)
			if (hpbps->fn == fn && hpbps->fDirty)
				{
				rgibp[iibpMac++] = ibp;
				if (iibpMac >= iibpMax)
					{
					fRepeat = fTrue; 
					break;
					}
				}
		}}
	SortIbp(rgibp, iibpMac);
/* sort data blocks so that writes can be consecutive */
	for (iibp = 0; iibp < iibpMac; iibp++)
		{
/* exchange rgibp[iibp] with the ibp = iibp */
		ibpOld = rgibp[iibp];
		if (ibpOld != iibp)
			{
			ExchangeIbp(ibpOld, iibp);
			if (iibp == ibpProtectCur)
				ibpProtectCur = ibpOld;
			else  if (ibpOld == ibpProtectCur)
				ibpProtectCur = iibp;
/* perform exchange in the rest of the rgibp array, too */
			for (iibpT = iibp + 1; iibpT < iibpMac; iibpT++)
				if (rgibp[iibpT] == iibp)
					{
					rgibp[iibpT] = ibpOld;
					break;
					}
			rgibp[iibp] = iibp;
			}
		}

#ifdef DEBUG
	/* BLOCK: after the sort, make sure what we are writing is valid */
		{
		struct BPS HUGE * hpbpsT = vbptbExt.hpmpibpbps;
		struct BPS HUGE * hpbpsMacT = hpbpsT + iibpMac;
		PN pnPrev = pnNil;
		for ( ; hpbpsT < hpbpsMacT; hpbpsT++)
			{
			Assert(hpbpsT->pn != pnNil);  /* no nil pages */
			Assert(hpbpsT->pn > pnPrev||pnPrev == pnNil);/* correct order */
			Assert(hpbpsT->fn == fn);     /* all refer to correct file */
			Assert(hpbpsT->fDirty);       /* everything really is dirty */
			pnPrev = hpbpsT->pn;
			}
		}
#endif /* DEBUG */

/* write out to next level pages in rgibp */
	for (iibp = 0; iibp < iibpMac;)
		{
		int ibp;
		int pn;
		int cbp = 0;
		int iibpT = iibp;
		int ibpT;
		int cbpT;

		ibp = rgibp[iibp];
		hpbps = HpbpsHpIbp(hpmpibpbps, ibp);
		pn = hpbps->pn;
/* write out pages from external cache to the disk */
/* detect consecutive pages for possible coalescing */
#ifdef MAC 
		hp = vbptbExt.hprgbpExt;
#else
		hp = (char HUGE *)HpOfBptbExt( ibp );
#endif
		do
			{
			++iibpT; 
			++cbp; 
			++hpbps;
			}
		while (iibpT < iibpMac &&
				rgibp[iibpT] == rgibp[iibpT - 1] + 1 &&
				hpbps->pn == pn + cbp &&
		/* note limitation of write operation */
		cbp < 63

#ifdef WIN 
		/* WIN: entire write must be in same sb; EMM buffers may be noncontiguous */
		&& SbOfHp(HpOfBptbExt(rgibp [iibpT]))
				== SbOfHp(hp)
#endif /* WIN */


				);

		Assert(ibp + cbp <= vbptbExt.ibpMac);
		Assert(ibp >= 0);

#ifdef MAC 
		lprgbpExt = LpLockHp(hp);
		lpbpExt = (char far *)&lprgbpExt[ibp];
		ec = EcWriteFnPns(fn, pn, cbp, (char far *)lpbpExt);
		UnlockHp(hp);
#else
		ec = EcWriteFnPns(fn, pn, cbp, hp);
#endif

/* if scratch file pages, leave them dirty otherwise reset dirty whether
the write is sucessful or not */
		if (ec == ecDiskFull && fn == fnScratch)
			{
			ReportSz("Incomplete write of fnScratch");
			Assert(vfScratchFile);
/* on a scratch file, write out one by one as many pages as you can */
			while (cbp-- != 0)
				{
#ifdef MAC 
				hp = vbptbExt.hprgbpExt;
				lprgbpExt = LpLockHp(hp);
				lpbpExt = (char far *)&lprgbpExt[ibp];
				ec = EcWriteFnPns(fn, pn, 1,
						(char far *)lpbpExt);
				UnlockHp(hp);
#else
				hp = (char HUGE *)HpOfBptbExt( ibp );
				ec = EcWriteFnPns(fn, pn, 1, hp);
#endif

				if (ec <= 0) break;
				HpbpsHpIbp(hpmpibpbps, ibp)->fDirty = fFalse;
				ibp++; 
				pn++;
				}
			vfScratchFile = fFalse;
			fReturn = fFalse;
			goto LRestoreProtect;
			}

		hpbps = HpbpsHpIbp(hpmpibpbps, ibp);
		cbpT = ibp + cbp;
		for (ibpT = ibp; ibpT < cbpT; ibpT++, hpbps++)
			hpbps->fDirty = fFalse;

		if (ec == ecDiskFull)
			{
			fReturn = fTrue;
			goto LRestoreProtect;
			}
		iibp += cbp;
		}
	if (fRepeat) goto LRepeat;
	PfcbFn(fn)->fDirty = fFalse;
	fReturn = fTrue;

#ifdef MAC
	/* flush the physical file and volume (directory info) */
	FlushOsfn(fn);
#endif /* MAC */

LRestoreProtect:
	if (vibpProtect != ibpProtectCur)
		ExchangeIbp(vibpProtect, ibpProtectCur);

	return (fReturn);
}



/* S O R T  I B P */
/* sorts the array rgw with respect to vhpmppibbps */
/* %%Function:SortIbp %%Owner:peterj */
SortIbp(rgw, iwMac)
int *rgw, iwMac;
{
	int iw;
	if (iwMac < 2) return;
	for (iw = iwMac>>1; iw >= 2; --iw)
		SortSiftUpIbp(rgw, iw, iwMac);
	for (iw = iwMac; iw >= 2; --iw)
		{
		int w;
		SortSiftUpIbp(rgw, 1, iw);
		w = rgw[0];
		rgw[0] = rgw[iw - 1];
		rgw[iw - 1] = w;
		}
}


/* S O R T  S I F T  U P  I B P */
/* see Floyd, Robert W. Algorithm 245 TREESORT 3 [M1] CACM 7, December 1964. */
/* %%Function:SortSiftUpIbp %%Owner:peterj */
SortSiftUpIbp(rgw, iwI, iwN)
int *rgw, iwI, iwN;
{
	int iwJ;
	int wCopy;

	wCopy = rgw[iwI - 1];
Loop:
	iwJ = 2 * iwI;
	if (iwJ <= iwN)
		{
		if (iwJ < iwN)
			{
			/* to sort rgibp by increasing pn's in vhpmpibpbps */
			if (vhpmpibpbps[rgw[iwJ]].pn > vhpmpibpbps[rgw[iwJ - 1]].pn)
				iwJ++;
			}
		if (vhpmpibpbps[rgw[iwJ - 1]].pn > vhpmpibpbps[wCopy].pn)
			{
			rgw[iwI - 1] = rgw[iwJ - 1];
			iwI = iwJ;
			goto Loop;
			}
		}
	rgw[iwI - 1] = wCopy;
}


#ifdef FUTURE
#ifdef MAC  /* Opus is not prepared to do this yet (ever?) */
/* F  C H A N G E  I B P  M A C  E X T */
/* changes the number of external paging buffers to ibpMac.
Returns true iff change is successful (in doubt only if ibpMac increased.)
*/
/* %%Function:FChangeIbpMacExt %%Owner:NOTUSED */
FChangeIbpMacExt(ibpMacNew)
int ibpMacNew;
{
	int ibp;
	BOOL fResult = fTrue;

	if (ibpMacNew >= vbptbExt.ibpMac)
		ibpMacNew = min(ibpMacNew, vbptbExt.ibpMax);
	else
		{
		int ibpClean;
/* reducing the number of buffers. clear buffer states. */
/* first move all dirty buffers to below ibpMacNew */
		int ibpDirtyMac = vbptbExt.ibpMac;
		for (ibpClean = 0; ibpClean < ibpMacNew; ibpClean++)
			{
			if (!HpbpsHpIbp(vbptbExt.hpmpibpbps, ibp)->fDirty)
				{
				while (ibpDirtyMac > ibpMacNew)
					{
					if (HpbpsHpIbp(vbptbExt.hpmpibpbps, --ibpDirtyMac)->fDirty)
						{
						ExchangeIbp(ibpClean, ibpDirtyMac);
						goto LLoop;
						}
					}
				goto LClear;
				}
LLoop:                  
			;
			}
/* no more clean pages. There may be dirty pages below ibpDirtyMac */
		ibpMacNew = ibpDirtyMac;
		fResult = fFalse;
LClear:
		for (ibp = ibpMacNew; ibp < vbptbExt.ibpMac; ibp++)
			{
			Assert(!HpbpsHpIbp(vbptbExt.hpmpibpbps, ibp)->fDirty);
			DeleteIbpHashEntry(ibp);
			}
		}
	if (EcChangeSizeQq(vbptbExt.hprgbpExt, (long)ibpMacNew << shftSector)
			== 0)
		{
		vbptbExt.ibpMac = ibpMacNew;
		return fResult;
		}
	return fFalse;
}


#endif /* MAC */

#endif /* FUTURE */


/* E X C H A N G E  I B P */
/* %%Function:ExchangeIbp %%Owner:peterj */
ExchangeIbp(ibp1, ibp2)
int ibp1, ibp2;
{
	int i;
	struct BPS bps1, bps2, HUGE *hpmpibpbps = vbptbExt.hpmpibpbps;
	struct BPS HUGE *hpbps1 = &hpmpibpbps[ibp1];
	struct BPS HUGE *hpbps2 = &hpmpibpbps[ibp2];
	char (HUGE *hprgbp)[cbSector];
	char HUGE *hpb1;
	char HUGE *hpb2;
	char rgb[cbSector / 4];

/* exchange bps's, and fix up hash links */
	bps1 = *hpbps1;
	bps2 = *hpbps2;
	DeleteIbpHashEntry(ibp1);
	DeleteIbpHashEntry(ibp2);
	*hpbps2 = bps1;
	*hpbps1 = bps2;
	if (hpbps1->fn != fnNil)
		InsertIbpHashEntry(ibp1);
	if (hpbps2->fn != fnNil)
		InsertIbpHashEntry(ibp2);

/* exchange data */
#ifdef MAC
	hprgbp = vbptbExt.hprgbpExt;
	hpb1 = &hprgbp[ibp1];
	hpb2 = &hprgbp[ibp2];
#else
	hpb1 = HpOfBptbExt( ibp1 );
	hpb2 = HpOfBptbExt( ibp2 );
#endif
	for (i = 0; i < 4; i++)
		{
		bltbh(/*from*/hpb1, rgb, cbSector / 4);
		bltbh(/*from*/hpb2, hpb1, cbSector / 4);
		bltbh(/*from*/rgb, hpb2, cbSector / 4);
		hpb1 += cbSector / 4;
		hpb2 += cbSector / 4;
		}
}


/* I N S E R T  I B P  H A S H  E N T R Y */
/* %%Function:InsertIbpHashEntry %%Owner:peterj */
InsertIbpHashEntry(ibp)
int ibp;
{
	struct BPS HUGE *hpbps = HpbpsIbp(ibp);
	int HUGE *hpibpHash;
	int iibp = IibpHash(hpbps->fn, hpbps->pn, vbptbExt.iibpHashMax);
	hpbps->ibpHashNext = *(hpibpHash = HpibpHash(iibp));
	*hpibpHash = ibp;
}



/* N A M E  F N  */
/* %%Function:FNameFn %%Owner:peterj */
FNameFn(fn, st)
int     fn;
char    st[];
{
	struct FCB **hfcb = mpfnhfcb [fn];

	Assert( hfcb != NULL );
	Assert( cwFCB * sizeof (int) == sizeof (struct FCB) );

	if (*st >= ichMaxFile-1/*safety*/)
		/* guarantee we won't overflow things later */
		{
		Assert(fFalse);
		return fFalse;
		}

	/* cwFCB includes stFile[2], so subtract 2 but add 1 for st[0] */
/* this is a hack to prevent heap movement when the scratch file becomes real */
	if (st [0] > (*hfcb)->stFile [0] &&
			!FChngSizeHCw( hfcb, cwFCB + CwFromCch(st[0] - 1), fTrue ))
		return fFalse;
	bltbyte( st, (*hfcb)->stFile, st[0] + 1);
	return fTrue;
}


#ifndef PMWORD
/* F  E X T E R N A L  F I L E */
/*  return TRUE if fn is on a network */
/* %%Function:FExternalFile %%Owner:peterj */
BOOL FExternalFile(fn)
int fn;
{
	CHAR szFile[cchMaxFile];
	StToSz(PfcbFn(fn)->stFile, szFile);
	return FFileRemote(szFile);
}


#else		/* PMWORD */
/* F  E X T E R N A L  F I L E */
/*  return TRUE if fn is on a network */
/* %%Function:FExternalFile %%Owner:ricks */
BOOL FExternalFile(fn)
int fn;
{
	int osfn;
	return FOsfnRemote(OsfnEnsureValid(fn));
}


#endif	/* PMWORD */


#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
File2_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* File2_Last() */
