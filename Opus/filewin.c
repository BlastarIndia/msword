/* F I L E W I N . C */
/*  Core file routines */

#define RSHDEFS
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


#ifdef DEBUG
#ifdef JBL
#define SHOWFILEREAD
#endif /* JBL */
#ifdef PCJ
#define SHOWFILEREAD
#endif /* PCJ */
#endif /* DEBUG */

#include "doslib.h"
#include "doc.h"
#include "file.h"
#include "winddefs.h"
#include "debug.h"
#include "ff.h"
#include "heap.h"
#include "error.h"
#include "version.h"

#ifdef PROTOTYPE
#include "filewin.cpt"
#endif /* PROTOTYPE */

extern CHAR		szEmpty[];
extern CHAR	    	szApp[];
extern TS             tsMruOsfn;
extern struct FCB     **mpfnhfcb[];
extern int            vfScratchFile;
extern int  	      fnMac;
extern FF             vff;
extern struct MERR    vmerr;
extern struct BPTB    vbptbExt;
extern int            vibpProtect; /* protected bp in Ext cache */
int 		      vfnPreload = fnNil;
int		      vcPreload = 0;
int                   vfnNoAlert = fnNil;  /* FFlushFn will not alert for this fn */



/* D T T M  C U R */
/* %%Function:DttmCur %%Owner:peterj */
struct DTTM DttmCur()
{
	struct DTTM dttm;
	struct TIM  tim;
	struct DAT  dat;

	StartUMeas(umOsDateTime);
	OsTime(&tim);   /* get current time */
	OsDate(&dat);   /* get current date */
	StopUMeas(umOsDateTime);

	dttm.yr    = dat.year - dyrBase;
	dttm.mon   = dat.month;
	dttm.dom   = dat.day;
	dttm.hr    = tim.hour;
	dttm.mint  = tim.minutes;
	dttm.wdy   = dat.dayOfWeek;

	return (dttm);
}


/* E C  R E A D  F N  P N S  */
/* Read cbSector characters from file fn, starting at page pn, to hpch.
On read errors, an error message will be made once and the fDiskAlert
flag will be set in vmerr.
The data will be set to all asterisks if pn < pnXLim. Otherwise
we must abort to envMain (MAC).
*/
/* %%Function:EcReadFnPns %%Owner:peterj */
EcReadFnPns(fn, pn, cpn, hpch)
int fn, cpn;
PN pn;
CHAR HUGE *hpch;
{
	return EcReadWriteFn(fn, FcFromPn(pn), hpch, cpn << shftSector, 
			fFalse/*fWrite*/, fTrue/*fErrors*/);
}


/* E C  W R I T E  F N  P N S  */
/* Write cbSector characters to file fn, starting at page pn, from hpch.
Error code will be returned after an error message having been made.
*/
/* %%Function:EcWriteFnPns %%Owner:peterj */
EcWriteFnPns(fn, pn, cpn, hpch)
int fn, cpn;
PN pn;
CHAR HUGE *hpch;
{
	return EcReadWriteFn(fn, FcFromPn(pn), hpch, cpn << shftSector, 
			fTrue/*fWrite*/, fTrue/*fErrors*/);
}


/* E C  R E A D  W R I T E  F N */
/* read or write cch bytes from hp to file fn starting at fc. if fErrors 
	report file errors, otherwise keep silent.
*/
/* %%Function:EcReadWriteFn %%Owner:peterj */
EcReadWriteFn(fn, fc, hpch, cch, fWrite, fErrors)
int fn;
FC fc;
CHAR HUGE *hpch;
int cch;
BOOL fWrite, fErrors;
{
	int ec = ecNoAccError;
	int osfn;
	struct FCB *pfcb = *mpfnhfcb[fn];
	FC fcLim = fc + cch;
	FC fcMacFile = pfcb->fcMacFile; /* physical end */
	FC cbMacFile = pfcb->cbMac;     /* logical end */

	AssertH(mpfnhfcb[fn]);

/*if cch is more than 0x7fff the ec returned from a successful 
	EcOsReadWrite will look negative and be treated as an error. */
	Assert(cch > 0);

/*assert that we will not write to the scratch file if further writing to
	the scratch file has been declared to be impossible. */
	Assert(fn != fnScratch || vfScratchFile || !fWrite);

	if (fWrite && fcLim > cbMacFile)
		/* writing beyond logical end of file */
		{
		int dch = fcLim - cbMacFile;
		Assert(fcLim - cbMacFile < cbSector);
		cch -= dch; /* don't write beyond cbMacFile */
		if (!cch)
			return 0;
		}
	else  if (!fWrite && fcLim > fcMacFile)
		/* reading beyond physical end of file */
		{
		int cchZero = fcLim - CpMax(fc, fcMacFile);
		Assert(cchZero <= cch);
		Assert(fcLim - CpMax(fc, fcMacFile) < 0x00007fff);
		cch -= cchZero;
		bltbcx(0, LpFromHp(hpch + cch), cchZero);
		if (!cch)
			return cchZero;
		}

/*assert that we do not read or write to/from the scratch file if it has not
	been created yet. */
	Assert(fn != fnScratch || 
			(vmerr.fScratchFileInit && !vmerr.fPostponeScratch));

	Assert(!(fWrite && pfcb->fReadOnly));

/* get a valid handle (will reopen of file has been closed) */
	if ((osfn = OsfnValidAndSeek(fn, fc)) == osfnNil)
		/* error already reported */
		return ecNoAccError;

#ifdef DISABLE_PJ
/* seek */
	if (DwSeekDw(osfn, fc, SF_BEGINNING) != fc)
		{
#ifdef SHOWFILEREAD
		CommSzLong(SzShared("EcReadWriteFn: seek error. fc = "), fc);
#endif /* SHOWFILEREAD */
		goto LHardErr;
		}
#endif /* DISABLE_PJ */

/* reading from file */
	if (!fWrite)
		{
/* do the read -- don't care here how much we manage to read (as long as 
it is something) */
		StartUMeas(umFileRead);
		ec = CchReadDoshnd(osfn, LpLockHp(hpch), cch);
		StopUMeas(umFileRead);
		UnlockHp(hpch);
		if (ec > 0)
			return ec;
#ifdef SHOWFILEREAD
		CommSzNum(SzShared("EcReadWriteFn: read error. ec = "), ec);
#endif /* SHOWFILEREAD */
		}
/* writing */
	else
		{
		StartUMeas(umFileWrite);
		ec = CchWriteDoshnd(osfn, LpLockHp(hpch), cch);
		StopUMeas(umFileWrite);
		UnlockHp(hpch);

/* update physical end of file */
		if (ec > 0 && fc + ec > fcMacFile)
			PfcbFn(fn)->fcMacFile = fc+ec;

/* if we can't write it all must be disk full */
		if (ec == cch)
			return ec;

/* out of disk space */
		if (ec >= 0)
			{
			if (!vmerr.fDiskAlert && fErrors)
				{
				if (fn != fnScratch)
					{
/*we don't raise fDiskAlert for disk full on the scratch file because
	FFlushFn will notice the condition and switch to storing scratch file
	pages within the file buffers. If too many scratch file pages are used
	vmerr.fDiskEmerg will be set. */
					vmerr.fDiskAlert = fTrue;
					if (fn != vfnNoAlert)
						DiskError(eidDiskFull, fn, "EcReadWriteFn");
					}
				}
#ifdef SHOWFILEREAD
			CommSzNum(SzShared("EcReadWriteFn: diskfull. fn = "), fn);
#endif /* SHOWFILEREAD */
			return ecDiskFull;
			}
		}

LHardErr:

	if (ec == 0)
		/* zero is an error value under dos but not mac */
		ec = ecHardError;

	ReportSz("Disk error!");
#ifdef SHOWFILEREAD
	CommSzNum(SzShared("EcReadWriteFn: ec = "), ec);
#endif /* SHOWFILEREAD */

	if (fErrors)
		{
		int pnMin = PnFromFc(fc);
		int pnLim = PnFromFc(fc+cch-1)+1;
		pfcb = PfcbFn(fn);
		if (!fWrite && (fn == fnScratch || (pnMin == 0 && pfcb->fHasFib) ||
				pnLim > pfcb->pnXLimit))
			{
			Assert(fn != vfnNoAlert);
			DiskError(eidSDN, fn, "ReadFnDfc");
			/* FUTURE: MacWord does a DoJump(&venvMainLoop) !!  
				(which,according to DavidLu is "guaranteed to crash soon 
				there after").  we should really figure out a better way...
			*/
			}

		else
			{
			if (!fWrite)
				bltbcx('*', LpFromHp(hpch), cch);
			else  if (vmerr.fDiskWriteErr)
				return ec;
			else
				vmerr.fDiskWriteErr = fTrue;

			if (fn != vfnNoAlert)
				DiskError(eidSDE, fn, "ReadFnDfc");
			}
		}

	return ec;
}



/* O S F N  V A L I D  A N D  S E E K */
/*  Assure that fn is open.  Seek to location fc
*/
/* %%Function:OsfnValidAndSeek %%Owner:peterj */
OsfnValidAndSeek(fn, fc)
int fn;
FC fc;
{
	struct FCB *pfcb = PfcbFn(fn);
	struct OFS ofs;

	Assert(mpfnhfcb [fn] != hNil);
	Assert(pfcb->osfn == osfnNil || pfcb->fOpened);

	FreezeHp();

	if (pfcb->osfn == osfnNil && !FAccessFn( fn, 0 ))
		/* unrecoverable error - unable to open file or seek */
		goto LError;

	else  if (DwSeekDw(pfcb->osfn, fc, SF_BEGINNING) != fc)
		/* windows must have closed the file for us--reopen */
		{
#ifdef DEBUG
		if (!pfcb->ofh.fFixedDisk)
			ReportSz("File unexpectedly closed, reopening.");
#endif /* DEBUG */

		FCloseFn(fn); /* in case it really is open */
		if ((pfcb->osfn = OsfnOurOpenFile(fn, &ofs, fFalse)) == osfnNil
				|| DwSeekDw(pfcb->osfn, fc, SF_BEGINNING) != fc)
			{
LError:
			FCloseFn(fn);
			Assert(pfcb->osfn == osfnNil);
			DiskError(eidSDN, fn, "OsfnValidAndSeek");
			}
		}

	MeltHp();

	return pfcb->osfn;
}


/* %%Function:FAccessFn %%Owner:peterj */
FAccessFn( fn, grpfOst )
int fn;
WORD grpfOst;   /* FLAGS */
	{ /*    Description:    Access file which is not currently opened.
							Open file and make an appropriate entry in the
							FCB entry.
							Flags in grpfOst:
								fOstReportErr   Whether to report errors
								fOstSearchPath  Whether to search path for the file
								fOstCreate+fOstNamed Indicates file should be created
							fn==fnNil is a special case which means that a file
							is closed if necessary to assure a free osfn slot,
							but nothing is opened.
	
			Returns:        TRUE on success, FALSE on failure
	*/

	register struct FCB *pfcb;
	unsigned osfn;
	struct OFS ofs;
	int fReportErr = grpfOst & fOstReportErr;
	int fSearchPath = grpfOst & fOstSearchPath;
	int fCreate = (grpfOst & fOstCreate) && (grpfOst & fOstNamed);

	/* Special case: scratch file that hasn't been created yet */
	if (fn == fnScratch && vmerr.fPostponeScratch)
		{
		ReportSz("tell peter this came up!");
		goto LRetOK;
		}

	Assert(fn != fnScratch || vmerr.fScratchFileInit);

	/* Look at every file Word knows about and
			(1) count how many are opened
		(2) find the least recently used open file */

		{
		TS ts, tsLru;
		int fnCur;
		int cfcbOpen;
		int fnLru = 0;

		tsLru = tsMax;
		cfcbOpen = 0;
		for (fnCur = 0; fnCur < fnMac; fnCur++)
			{
			struct FCB **hfcb;
			if ((hfcb = mpfnhfcb[fnCur]) != 0)
				{
				pfcb = *hfcb;
				if (pfcb->osfn != osfnNil)
					{
					cfcbOpen++;
					ts = pfcb->ts - (tsMruOsfn + 1);
					if (ts <= tsLru)
						{
						tsLru = ts;
						fnLru = fnCur;
						}
					}
				}
			}
		Assert(cfcbOpen <= cfcbOpenMax);

/* if we already have the maximum number of files opened, close the
least recently used open file */
		if (cfcbOpen >= cfcbOpenMax)
			{
			Assert( fnLru != 0 );
			pfcb = PfcbFn(fnLru);
			FCloseDoshnd( pfcb->osfn );
			pfcb->osfn = osfnNil;
			}
		}

	if (fn==fnNil)
		return fFalse;

	pfcb = PfcbFn(fn);
	Assert( mpfnhfcb [fn] != NULL && pfcb->osfn == osfnNil );
	Assert( FValidStFile( pfcb->stFile ) );

	blt( &pfcb->ofh, &ofs, sizeof (struct OFH) / sizeof (int) );
	StToSz( pfcb->stFile, ofs.szFile );
	AnsiToOem( ofs.szFile, ofs.szFile );

	if (!pfcb->fReadOnly && !pfcb->fOpened && !fCreate)
		{   /* Test for read-only file */
		WORD da;

		if ((da = DaGetFileModeSz(ofs.szFile)) != DA_NIL && (da & DA_READONLY))
			goto TryReadOnly;
		}

	for ( ;; )
		{
	/* OpenFile's first parm is a filename when opening for the
		first time, a prompt on successive occasions (OF_REOPEN) */

		osfn = OsfnOurOpenFile( fn, &ofs, fCreate );

		if (osfn != osfnNil)
			{    /* Opened file OK */

			if (!pfcb->fOpened)
				{   /* First time through */
				CHAR stT [ichMaxFile];

				/* make sure it really is a FILE (not a device) */
				if (!FDoshndIsFile(osfn))
					goto LRetError;

				/* OpenFile may have given us a	different name for the file */
				blt( &ofs, &pfcb->ofh, sizeof (struct OFH) / sizeof (int) );
				OemToAnsi( ofs.szFile, ofs.szFile );
				SzToStInPlace( ofs.szFile );
				if (!FEqualSt( ofs.szFile, pfcb->stFile ) &&
						FNormalizeStFile( ofs.szFile, stT, nfoNormal) &&
						!FEqualSt( stT, pfcb->stFile ))
					{   /* OpenFile gave us a different name for the file */
					/* This happens e.g. when the user tries to open a
						nonexistent file on C:, then is instructed to insert
						a disk in A:. The original name is "C:..."; the name
						returned by OpenFile is "A:..." */
					if (!FNameFn( fn, stT )) /* Heap Movement */
						{
LRetError:
						FCloseDoshnd(osfn);
						PfcbFn(fn)->osfn = osfnNil;
						return FALSE;
						}
					pfcb = PfcbFn(fn);
					}
				}
			break;  /* We succeeded; break out of the loop */
			}
		else
			{   /* Open failed -- try read-only; don't prompt this time */
			if ( !pfcb->fReadOnly && !pfcb->fOpened && !fCreate )
				{   /* Failed as read/write; try read-only */
TryReadOnly:
				pfcb->fReadOnly = fTrue;
				}
			else
				{
				if ((grpfOst & fOstReportErr) && !pfcb->fOpened)
				/* Check for sharing violation */
					if (ofs.nErrCode == nErrNoAcc)
						DiskError(eidNoAvail, fn, "FAccessFn");
					else
						ErrorEid(eidCantOpen, "FAccessFn");

				return FALSE;
				}
			}
		}

	Assert( osfn != osfnNil );
	pfcb->osfn = osfn;

LRetOK:
	Assert( fn != fnNil );
	PfcbFn(fn)->fOpened = TRUE;
	return fTrue;
}


/* %%Function:OsfnOurOpenFile %%Owner:peterj */
OsfnOurOpenFile( fn, pofs, fCreate )
int fn;
struct OFS *pofs;
BOOL fCreate;
	{       /* Invoke OpenFile on the file fn with the flags wOF.  Return
		the OF structure filled in by OpenFile through *pofs.
		Return the Osfn, or osfnNil if an error occurred. The error can
		be recovered from pofs->nErrCode
		There must be 128 bytes of storage in ofs, for OpenFile
	Sets some flags in wOF that are used by all callers */

	WORD wOF = 0;
	struct FCB *pfcb = (PfcbFn(fn));
	CHAR szFileOrPrompt [ichMaxFile];

	Assert( fn != fnNil && mpfnhfcb [fn] != NULL );

	bltbyte( &pfcb->ofh, pofs, sizeof (struct OFH) );
	StToSz( pfcb->stFile, pofs->szFile );
	AnsiToOem( pofs->szFile, pofs->szFile );

#ifdef DEBUG
	if (fCreate)
		Assert(!pfcb->fOpened && !pfcb->fReadOnly);
#endif /* DEBUG */

	if (pfcb->fOpened)
		/* File has been opened before; this is a re-open */
		wOF |= (OF_REOPEN | OF_PROMPT);

	StToSz( pfcb->stFile, szFileOrPrompt );

	wOF |= (pfcb->fReadOnly ? OF_READ + bSHARE_DENYWR : 
			OF_READWRITE + bSHARE_DENYRDWR );

	if (fCreate)
		wOF |= OF_CREATE;

	return OpenFile( (LPSTR) szFileOrPrompt, (LPOFSTRUCT) pofs, wOF );
}





/* I B P   L O A D   F N */
/* %%Function:IbpLoadFn %%Owner:peterj */
EXPORT IbpLoadFn(fn, pn)
int fn;
PN pn;
{
	int ibpMin, ibpT;
	int cbp;
	int ispnMax, ispnProtect, ispnUse;
	int ispnOldest, ispnSecondOldest, ispn;
	TS	tsOldest, tsSecondOldest, tsspn, tsCur;

	Assert (vbptbExt.ibpMax < 32768 / 4);
	ispnMax = ((vbptbExt.ibpMax << 2) / vbptbExt.cqbpspn);
	Debug(ispnOldest = ispnSecondOldest = -1);
	tsOldest = tsSecondOldest = 0;
	for (ispn = 0; ispn < ispnMax; ispn++)
		{
		tsspn = vbptbExt.tsMruBps - vbptbExt.hpmpispnts[ispn];
		if (tsspn >= tsOldest)
			{
			tsSecondOldest = tsOldest;
			ispnSecondOldest = ispnOldest;
			tsOldest = tsspn;
			ispnOldest = ispn;
			}
		else  if (tsspn >= tsSecondOldest)
			{
			tsSecondOldest = tsspn;
			ispnSecondOldest = ispn;
			}
		}
	Assert(ispnOldest != -1 && ispnSecondOldest != -1);
	ispnProtect = ((vibpProtect << 2) / vbptbExt.cqbpspn);
	ispnUse = (ispnOldest == ispnProtect ?
			ispnSecondOldest : ispnOldest);
	vbptbExt.hpmpispnts[ispnUse] = tsCur = ++vbptbExt.tsMruBps;

	ibpMin = (ispnUse * vbptbExt.cqbpspn + 3) >> 2;
	Assert(ispnUse == ((ibpMin << 2) / vbptbExt.cqbpspn));
	cbp = min((((ispnUse + 1) * vbptbExt.cqbpspn + 3) >> 2) - ibpMin,
			(int)((PfcbFn(fn)->cbMac+(cbSector-1)) >> shftSector) - pn);

#ifdef SHOWFILEREAD
	CommSzNum(SzShared("Block reading cbp: "), cbp);
#endif /* SHOWFILEREAD */

	/* if the following assert is not true we cannot make the implicit
		assumption below that FReadLarge began reading the part of the
		file we are interested in into the cache page determined by ibpMin. */
	Assert( ibpMin != vibpProtect );
	if (!FReadLarge(fn, pn, cbp, ibpMin))
		return ibpNil;
	return ibpMin;
}


/* F   R E A D	 L A R G E */
/* %%Function:FReadLarge %%Owner:peterj */
FReadLarge( fn, pnMin, cpn, ibpMin )
int fn;
PN pnMin;
int cpn;
int ibpMin;
{
	PN pn, pnLim = pnMin + cpn;
	int ibp, ibpMinT, ibpLim = ibpMin + cpn, ibpMac = vbptbExt.ibpMac;
	int cbpChunk;
	struct BPS HUGE *hpbps;

	if (cpn <= 0)
		return fFalse;

	Assert(ibpLim <= ibpMac);

	Assert(vibpProtect < ibpMin || vibpProtect >= ibpLim);

/* check fDirty of pages to be trashed; flush as necessary */

		{{ /* NATIVE - loops to scan the rgbps in FReadLarge */
LRetry1:
		for ( hpbps = (struct BPS HUGE *) &vbptbExt.hpmpibpbps [ibp = ibpMin] ; 
				ibp < ibpLim  ; ibp++, hpbps++ )
			if (hpbps->fDirty)
				{
				if (!FFlushFn(hpbps->fn))
					{{ /* !NATIVE - FReadLarge */
					fn = fnNil;
					goto LEnd;
					}}
				goto LRetry1; /* start over since FFlushFn rearranges pages */
				}

/* make sure pages not being trashed do not contain { fn,pn }s  that
	duplicate what we are about to read */
LRetry2:
		for ( hpbps = (struct BPS HUGE *)&vbptbExt.hpmpibpbps [ibp = 0] ; 
				ibp < ibpMac; ibp++, hpbps++ )
			{
			if (ibp == ibpMin)
				/* skip over what we are reading */
				{
				ibp += cpn-1;
				hpbps += cpn-1;
				continue;
				}

			if (hpbps->fn == fn && hpbps->pn >= pnMin && hpbps->pn < pnLim)
				/* since we'll be reading this page, don't want this copy of it */
				{
				if (hpbps->fDirty)
					/* page is dirty, write it out before nuking it */
					{
					if (!FFlushFn(hpbps->fn))
						{{ /* !NATIVE - FReadLarge */
						fn = fnNil;
						goto LEnd;
						}}
					goto LRetry2; /* start over since FFlushFn rearranges pages */
					}
				DeleteIbpHashEntry(ibp);
				}
			}
#ifdef DEBUG
/* make sure the above scans did what they were supposed to do! */
		hpbps = vbptbExt.hpmpibpbps;
		ibp = 0;
		for ( ; ibp < ibpMin  ; ibp++, hpbps++ )
			Assert(hpbps->fn != fn || hpbps->pn < pnMin || hpbps->pn >= pnLim);
		for ( ; ibp < ibpLim  ; ibp++, hpbps++ )
			Assert(!hpbps->fDirty);
		for ( ; ibp < ibpMac  ; ibp++, hpbps++ )
			Assert(hpbps->fn != fn || hpbps->pn < pnMin || hpbps->pn >= pnLim);
#endif /* DEBUG */
		}}

/* Read big chunks from the file */

	cbpChunk = vbptbExt.cbpChunk;

	for ( ibpMinT = ibpMin; ibpMinT < ibpLim; )
		{
		int ibpMacT = min( ibpMinT - (ibpMinT % cbpChunk) + cbpChunk,ibpLim);
		char far *lpbpExt;
		char huge *hp;
#ifdef SHOWFILEREAD
		int rgw [4];

		rgw [0] = fn;
		rgw [1] = pnMin + ibpMinT - ibpMin;
		rgw [2] = ibpMinT;
		rgw [3] = (ibpMacT - ibpMinT) * cbSector;
		CommSzRgNum(SzFrame("** BLOCK ** Reading (fn,pn,ibp,bytes): "),rgw,4);
#endif
		hp = (char huge *)HpOfBptbExt(ibpMinT);

		Assert(ibpMacT > ibpMinT);
		Assert(SbOfHp(hp)==SbOfHp(HpOfBptbExt(ibpMacT-1)));
		Assert(ibpMinT < ibpLim && ibpMinT >= ibpMin);
		Assert(ibpMacT > ibpMin && ibpMacT <= ibpLim);

		if (EcReadFnPns(fn, pnMin+ibpMinT-ibpMin, ibpMacT-ibpMinT, hp) < 0)
			{  
			fn = fnNil ; 
			break; 
			}
		ibpMinT = ibpMacT;
		}

/* Fill out the page descriptors */

	for ( pn = pnMin, 
			hpbps = (struct BPS HUGE *) &vbptbExt.hpmpibpbps[ibp=ibpMin]; 
			ibp < ibpLim  ;  ibp++, hpbps++, pn++ )
		{
		int iibp = IibpHash(fn, pn, vbptbExt.iibpHashMax);

		if (hpbps->fn != fnNil)
			DeleteIbpHashEntry(ibp);
		if (fn == fnNil)
			continue;	/* read error: just delete descriptors */
		hpbps->fn = fn;
		hpbps->pn = pn;
		hpbps->ts = ++vbptbExt.tsMruBps;
		hpbps->fDirty = fFalse;
		hpbps->ibpHashNext = vbptbExt.hprgibpHash[iibp];
		vbptbExt.hprgibpHash[iibp] = ibp;
		}

LEnd:

	Debug( vdbs.fCkBptb ? CkPbptb(&vbptbExt, fFalse) : 0 );
	return (fn != fnNil);
}


/*  FOLLOWING CODE CAME FROM FILE.C */

/* G L O B A L S */
int                     vibp;
int			vibpProtect = 0xFFFF ; /* protected bp in Ext cache */



/* H P B P S  I B P  - returns huge addr of bps corresponding to ibp */
/* %%Function:HpbpsIbp %%Owner:peterj */
struct BPS HUGE *HpbpsIbp(ibp)
int ibp;
{
	return &((struct BPS HUGE *)vbptbExt.hpmpibpbps)[ibp];
}



/* H P I B P  H A S H  - returns HUGE addr of hash table entry for iibp */
/* %%Function:HpibpHash %%Owner:peterj */
int HUGE *HpibpHash(iibp)
{
	return &((int HUGE *)vbptbExt.hprgibpHash)[iibp];
}


/* I B P  R E A D  F I L E  P A G E */
/* empty a page in the cache and read page pn from file fn into it.
iibp is used to splice the new entry into the hash table.
Return ibp of page.
*/
/* %%Function:IbpReadFilePage %%Owner:peterj */
EXPORT int IbpReadFilePage(fn, pn, iibp)
int fn, iibp;
PN pn;
{
	LPRGBPEXT lprgbpExt;
	char far *lpbpExt;
	struct BPS HUGE *hpbps;
	int ibp = IbpSelectCachePage(fn);
	int HUGE *hpibp;
	char HUGE *hp;

/* if cache page was occupied, update hash table */
	if (HpbpsIbp(ibp)->fn != fnNil)
		DeleteIbpHashEntry(ibp);

#ifdef MAC
	lprgbpExt = LpLockHp(hp = vbptbExt.hprgbpExt);
	Assert(lprgbpExt != 0);
	lpbpExt = &lprgbpExt[ibp];

/* read page from disk into external cache page. pn and ibp now refer
to external pages. */
	if (lpbpExt != NULL)
		EcReadFnPns(fn, pn, 1, lpbpExt);

	UnlockHp(hp);
#else /* WIN */
/* read page from disk into external cache page. pn and ibp now refer
to external pages. */
	hp = (char HUGE *)HpOfBptbExt(ibp);
	EcReadFnPns(fn, pn, 1, hp);
#endif
	hpbps = HpbpsIbp(ibp);
	hpbps->fn = fn;
	hpbps->pn = pn;
#ifdef MAC
	if (!vplb.fNoTsNonDest || vplb.fnDest == fn)
		hpbps->ts = ++(vbptbExt.tsMruBps);
#else /* WIN */
	hpbps->ts = ++(vbptbExt.tsMruBps);
#endif
#ifdef WIN
	vbptbExt.hpmpispnts[(ibp << 2) / vbptbExt.cqbpspn] = vbptbExt.tsMruBps;
#endif
	hpbps->fDirty = fFalse;

/* put in new hash table entry for fn, pn */

	hpibp = HpibpHash(iibp);
	hpbps->ibpHashNext = *hpibp;
	*hpibp = ibp;

	return(ibp);
}


/* I B P  S E L E C T  C A C H E  P A G E */
/* must find an empty buffer page in the cache vbptbExt.
In the internal cache, use w2.rules for clearing dirty pages.
*/
/* %%Function:IbpSelectCachePage %%Owner:peterj */
IbpSelectCachePage(fn)
int fn;
{
	int ibp, ibpFirst;
	struct BPS HUGE *hpbps;

LSelect:
	ibp = IbpLru();

/* If the chosen slot is occupied by a dirty page, the page must be written
out. While we are at it, we write all dirty pages in that file */
	hpbps = HpbpsIbp(ibp);
	if (hpbps->fDirty)
		{
#ifdef DEBUG
		int fnFlushed = hpbps->fn;
		if (!FFlushFn(hpbps->fn))
			{
			Assert(fnFlushed == fnScratch);
			Assert(!vfScratchFile);
			goto LSelect;
			}
#else
		FFlushFn(hpbps->fn);
#endif
		goto LSelect;
		}
	return(ibp);
}


/* D E L E T E  I B P  H A S H  E N T R Y  */
/* %%Function:DelteIbpHashEntry %%Owner:peterj */
DeleteIbpHashEntry(ibpDelete)
int ibpDelete;
	{ /*
	delete reference to buffer slot ibpDelete in vbptbExt's hash table.
*/

	struct BPS HUGE *hpmpibpbps = HpbpsIbp(0);
	struct BPS HUGE *hpbpsDelete = &hpmpibpbps[ibpDelete];
	int HUGE *hprgibpHash = vbptbExt.hprgibpHash;
	int ibpPrev, ibp, iibpHash;

	iibpHash = IibpHash(hpbpsDelete->fn, hpbpsDelete->pn, vbptbExt.iibpHashMax);
	ibp = hprgibpHash[iibpHash];
	ibpPrev = ibpNil;
	while (ibp != ibpNil)
		{
		if (ibp == ibpDelete)
			{ /* Found it */
			if (ibpPrev == ibpNil)
				hprgibpHash[iibpHash] =
						hpmpibpbps[ibp].ibpHashNext;
			else
				hpmpibpbps[ibpPrev].ibpHashNext =
						hpmpibpbps[ibp].ibpHashNext;
			break;
			}
		ibpPrev = ibp;
		ibp = hpmpibpbps[ibp].ibpHashNext;
		}
	hpbpsDelete->fn = fnNil;
	hpbpsDelete->fDirty = fFalse;
}


/* F E T C H  F I B */

/* %%Function:FetchFib %%Owner:peterj */
FetchFib(fn, pfib, pn)
int fn;
struct FIB *pfib;
PN pn;
{
	struct FIB HUGE *hpfib;
	int cbFib;

	Mac( Assert(cbFIB30 <= cbFIB) );
	Assert(cbFIB <= cbFileHeader);
	Assert(cbFIB <= cbSector);
	Win( Assert(PfcbFn(fn)->fHasFib) ); /* MAC calls before fcb is done */

	/* Fib is read starting at pn.  It has length fib.fcMin-FcFromPn(pn)
		but at most cbFIB is read.  Fib is first zero initialized in case
		fib on file is short.
	*/
	SetBytes(pfib, 0, cbFIB);
	hpfib = HpchGetPn(fn, pn);
	cbFib = (int)(hpfib->fcMin - FcFromPn(pn));

	bltbh(hpfib, pfib, min(cbFIB, cbFib));

	Assert(FNativeFormat(hpfib, fFalse));
	if (pfib->nFib < nFibCurrent)
		ConvertFib(pfib);
}


#ifdef RSH  /* Research Version Code */

extern struct PREF vpref;

int vfnUa = fnNil;
struct UAT vuat;
csconst CHAR csszName[] = SzShared("W4WLOG"); /* don't localize */
csconst CHAR csstExt[] = StShared(".RSH"); /* don't localize */

/* R S H  R E P O R T  D I S K  F U L L */
/*  Put up a message box reporting file error.
*   Show filename from fn, or, if fn == fnNil, from stFile.
*   Should be called before CloseUa, so that fn will still be valid.
*   REVIEW tonykr: Limited time offer!
*/
/* %%Function:RshReportDiskFull %%Owner:tonykr */
RshReportDiskFull(fn, stFile)
int fn;
char * stFile;
{
	char sz[300+cchMaxFile];
	char * pch;

	pch = sz;
	pch += CchCopySz(SzShared("Research Version:\r\r"), sz);
	pch += CchCopySz(SzShared("The Research Version was unable to create or write "), pch);
	if (fn != fnNil)
		pch += CchCopyStRgch(PfcbFn(fn)->stFile, pch);
	else if (stFile != NULL)
		pch += CchCopyStRgch(stFile, pch);
	else
		pch += CchCopySz(SzShared("a log file"), pch);
	pch += CchCopySz(SzShared(", so this session of Word will not be recorded.\r\r"), pch);
	pch += CchCopySz(SzShared("If you have any questions or comments, please call Philip Haine "), pch);
	pch += CchCopySz(SzShared("collect at Microsoft, (206) 882-8080, x.7188 before August 10, 1990."), pch);

	IdPromptBoxSz(sz, MB_MESSAGE);
}


/* R S H  R E P O R T  L O G  L I M I T */
/*  Put up a message box reporting that enough data has been collected.
*   REVIEW tonykr: Limited time offer!
*/
/* %%Function:RshReportLogLimit %%Owner:tonykr */
RshReportLogLimit()
{
	char sz[600];
	char * pch;

	pch = sz;
	pch += CchCopySz(SzShared("Research Version:\r\r"), sz);
	pch += CchCopySz(SzShared("This Research Version of Word for Windows has collected "), pch);
	pch += CchCopySz(SzShared("all the data it needs.  Information from this session "), pch);
	pch += CchCopySz(SzShared("will not be recorded.\r\r"), pch);
	pch += CchCopySz(SzShared("Word will operate normally, but you should restore the "), pch);
	pch += CchCopySz(SzShared("regular version of Word for Windows 1.0 at your earliest "), pch);
	pch += CchCopySz(SzShared("convenience.\r\r"), pch);
	pch += CchCopySz(SzShared("If you have any questions or comments, please call Philip Haine "), pch);
	pch += CchCopySz(SzShared("collect at Microsoft, (206) 882-8080, x.7188 before August 10, 1990."), pch);

	IdPromptBoxSz(sz, MB_MESSAGE);
}


/* I N I T  U A */
/*  Initialize User Action recording.  Creates a new (unique) file to store 
    data in.
    If this is the last log file allowed, report limit reached and return.
*/

/* %%Function:InitUa %%Owner:peterj */
InitUa()
{
	int n;

#ifdef DEBUG
	CommSzSz(SzShared("Initializing UA"),"");
#endif /* DEBUG */

	/* BLOCK - get file name and open file */
		{
		CHAR stName[ichMaxFile];
		struct FNS fns;
		CHAR *pch;

		n = GetProfileInt(szApp, SzShared("RshNext"), 0);
		if (n >= 10)	/* Check for log file limit */
			{
			/* Don't cycle */
			RshReportLogLimit();
			Assert(vfnUa == fnNil);
			return;
			}

		CopyCsSz(csszName, fns.stShortName);
		pch = fns.stShortName + sizeof(csszName) - 1;
		Assert(n >= 0 && n < 10);
		*pch++ = '0' + n;
		*pch = 0;
		SzToStInPlace(fns.stShortName);

		/* If RshPath defined, use that directory, otherwise use program dir */
		if (GetProfileString(szApp, SzShared("RshPath"), 
				szEmpty, fns.stPath, ichMaxPath))
			{
			SzToStInPlace(fns.stPath);
			}
		else
			AssertDo(FGetStFpi(fpiProgram, fns.stPath));

		CopyCsSt(csstExt, fns.stExtension);
				
		PfnsToStFile(&fns, stName);

		EcOsDelete(stName, 0);

		if ((vfnUa=FnOpenSt(stName, fOstCreate|fOstNamed, ofcDoc, NULL)) == fnNil)
			{
LError:
			RshReportDiskFull(fnNil, stName);
			return;
			}
#ifdef DEBUG
		CommSzSt(SzShared("opened "), stName);
#endif /* DEBUG */
		}

	/* BLOCK - write header info */
		{
		struct {
			int nVer;
			struct DTTM dttm;
			CHAR szUsrName[ichUsrNameMax];
			int nProduct;
			} uah; /* User Action Header */

#define nVerUahCur 0x4322 /* magic number for .RSH file */

		/* REVIEW peterj (pj): should we have more info in uah? */

		SetBytes(&uah, 0, sizeof(uah));

		uah.nVer = nVerUahCur;
		uah.dttm = DttmCur();
		StToSz(vpref.stUsrName, uah.szUsrName);
		uah.nProduct = nProductCurrent;

		Assert (PfcbFn(vfnUa)->fcPos == 0);
		WriteRgchToFn(vfnUa, &uah, sizeof(uah));
		/* WriteRgchToFn starts writing @ fcPos and leaves fcPos at end
		   of what was written.  as long as all you do is write, calling
		   WriteRgchToFn is an implicit AppendRgchToFn (without screwing
		   up the vfkpd* pages).
		*/
		Assert (PfcbFn(vfnUa)->fcPos == sizeof(uah));

		if (vmerr.fDiskAlert || vmerr.fDiskWriteErr)
			/* we are having problems &/or filled up disk.  stop saving data. */
			{
			RshReportDiskFull(vfnUa, NULL);
			CloseUa();
			goto LError;
			}
		}

	/* BLOCK - record start number for next time */
		{
		CHAR sz[3];
		if (n + 1 < 10)
			{
			sz[0] = '0' + (n+1)%10;
			sz[1] = 0;
			}
		else
			{
			sz[0] = '0' + (n+1)/10;
			sz[1] = '0' + (n+1)%10;
			sz[2] = 0;
			}
		WriteProfileString(szApp, SzShared("RshNext"), sz);
		}

	/* BLOCK - initialize timer */
	vuat.msecSessionStart = GetTickCount();	/* Initialize start time */
	vuat.uam = uamNull;
	SetUatMode(uamIdle);
	vuat.msecNextReport = GetTickCount() + msecReports;

}

/* C L O S E  U A */
/*  Close the User Action file.  Do any clean-up that is necessary.
*/
/* %%Function:CloseUa %%Owner:peterj */
CloseUa()
{
	if (vfnUa != fnNil)
		{
#ifdef DEBUG
		CommSzSz(SzShared("deleting vfnUa"),"");
#endif /* DEBUG */
		FFlushFn(vfnUa);
		FCloseFn(vfnUa);
		DeleteFn(vfnUa, fFalse);
		vfnUa = fnNil;
		}
}

/* W  T O  H E X */
/*  Format w into a four digit hex number.  passed buffer must be at least
    5 bytes long.
*/
/* %%Function:WToHex %%Owner:peterj */
WToHex(rgch, w)
char rgch[];
uns w;
{
	uns wm16;
	int cch = 4;

	rgch[cch] = 0;

	while (cch > 0)
		{
		wm16 = w % 16;
		if (wm16 <= 9)
			rgch[--cch] = '0' + wm16;
		else
			rgch[--cch] = 'A' + wm16 - 10;
		w /= 16;
		}
}

/* L O G  U A  */
/*  Log an action and timestamp in the User Action file.
*/

/* %%Function:LogUa %%Owner:peterj */
LogUa(ua)
uns ua;
{
	BOOL fDiskError = (vmerr.fDiskAlert || vmerr.fDiskWriteErr);
	long msecTimeStamp;
	uns rgw[2];

#ifdef DEBUG
	/* BLOCK - dump number being logged to COM Port (in hex) */
		{
		CHAR szUaHex[5];
		WToHex(szUaHex, ua);
		CommSzSz(SzShared("LogUaProc:  logging "), szUaHex);
		}
#endif /* DEBUG */

	if (vfnUa == fnNil)
		return; /* system not properly initialized or shut down */

	msecTimeStamp = GetTickCount() - vuat.msecSessionStart;

	rgw[0] = ua;
	rgw[1] = (uns) ((msecTimeStamp + 500)/1000l);

	Assert (PfcbFn(vfnUa)->fcPos == PfcbFn(vfnUa)->cbMac);
	WriteRgchToFn(vfnUa, rgw, 2*sizeof(uns));

	if (!fDiskError && (vmerr.fDiskAlert || vmerr.fDiskWriteErr))
		/* we are having problems &/or filled up disk.  stop saving data. */
		{
		RshReportDiskFull(vfnUa, NULL);
		CloseUa();
		}
}

/* L O G  U A  N O  T I M E S T A M P */
/*  Log an action (but no timestamp) in the User Action file.
 *  Used in LogUaTimes.
*/

/* %%Function:LogUaNoTimestamp %%Owner:peterj */
LogUaNoTimestamp(ua)
uns ua;
{
	BOOL fDiskError = (vmerr.fDiskAlert || vmerr.fDiskWriteErr);

#ifdef DEBUG
	/* BLOCK - dump number being logged to COM Port (in hex) */
		{
		CHAR szUaHex[5];
		WToHex(szUaHex, ua);
		CommSzSz(SzShared("LogUaProc:  logging "), szUaHex);
		}
#endif /* DEBUG */

	if (vfnUa == fnNil)
		return; /* system not properly initialized or shut down */

	Assert (PfcbFn(vfnUa)->fcPos == PfcbFn(vfnUa)->cbMac);
	WriteRgchToFn(vfnUa, &ua, sizeof(uns));

	if (!fDiskError && (vmerr.fDiskAlert || vmerr.fDiskWriteErr))
		/* we are having problems &/or filled up disk.  stop saving data. */
		{
		RshReportDiskFull(vfnUa, NULL);
		CloseUa();
		}
}

/* S E T  U A T  M O D E */
/*  Set the current user action mode to uam
*   Log changes to Idle or Typing mode so that timestamps are accurate.
*/
/* %%Function:SetUatMode %%Owner:peterj */
SetUatMode(uam)
int uam;
{
	long msecNow = GetTickCount();

	if (uam != vuat.uam)
		{
#ifdef DEBUG
		CommSzNum(SzShared("SetUatMode: new mode = "), uam);
#endif /* DEBUG */
		if (vuat.uam != uamNull)
			vuat.mpuammsec[vuat.uam] += (msecNow - vuat.msecStartMode);
		vuat.msecStartMode = msecNow;
		if (uam == uamIdle)
			LogUa(uaIdle);	/* Log change to idle mode */
		else if (uam == uamTyping)
			LogUa(uaInsert);	/* Also log change to typing mode */
		}

	vuat.uam = uam;

	/* reset timeout */
	vuat.msecTimeout = msecNow + msecModeTimeout;
	vuat.msecStopped = 0l;
}

/* S T O P  U A T  T I M E R */
/*  Pauses the timeout timer for user action modes
*/
/* %%Function:StopUatTimer %%Owner:peterj */
StopUatTimer()
{
	vuat.msecStopped = GetTickCount();
}

/* R E S T A R T  U A T  T I M E R */
/*  restarts the timeout timer
*/
/* %%Function:RestartUatTimer %%Owner:peterj */
RestartUatTimer()
{
	if (vuat.msecStopped != 0l)
		{
		vuat.msecTimeout += (GetTickCount() - vuat.msecStopped);
		vuat.msecStopped = 0l;
		}
}

/* L O G  U A T  T I M E S */
/*  Logs and resets the time spent in each mode
*   Also flushes the buffer to disk as a way of minimizing data lost in
*   case of a crash.
*/
/* %%Function:LogUatTimes %%Owner:peterj */
LogUatTimes()
{
	int uam, uas;

 	SetUatMode(uamNull);
	LogUa(uaTimesFollow);
	for (uam = 0; uam < uamMax; uam++)
		{
		LogUaNoTimestamp((uns)((vuat.mpuammsec[uam]+500)/1000l)); /* record seconds */
		vuat.mpuammsec[uam] = 0;
		}
	for (uas = 0; uas < uasMax; uas++)
		{
		if (vuat.mpuasmsecStarted[uas] != 0l)
			{
			StopSubTimer(uas);
			StartSubTimer(uas);
			}
		LogUaNoTimestamp((uns)((vuat.mpuasmsec[uas]+500)/1000l)); /* record seconds */
		vuat.mpuasmsec[uas] = 0l;
		}
	vuat.msecNextReport = GetTickCount() + msecReports;
	SetUatMode(uamIdle);
	if (vfnUa != fnNil)
		FFlushFile(vfnUa);	/* Flush buffer to disk */
}

/*  C H E C K  U A T */
/*  Checks for various timeouts for uat (called from idle)
*/
/* %%Function:CheckUat %%Owner:peterj */
CheckUat()
{
	long msecNow = GetTickCount();

	if (vuat.uam == uamCommand || 
			(vuat.uam != uamIdle && msecNow > vuat.msecTimeout))
		SetUatMode(uamIdle);
	if (msecNow >= vuat.msecNextReport)
		LogUatTimes();
}


/* S T A R T  S U B  T I M E R */
/*  Starts timing an event or state other than the user mode
*   Logs beginning of Dialog Session.
*/
/* %%Function:StartSubTimer %%Owner:peterj */
StartSubTimer(uas)
int uas;
{
	if (vuat.mpuasmsecStarted[uas] == 0l)
		{
		vuat.mpuasmsecStarted[uas] = GetTickCount();
		if (uas == uasDialogSession)
			LogUa(uaStartDialogSession);
		}
}

/* S T O P  S U B  T I M E R */
/*  Stops timing an event or state other than the user mode
*   Logs end of Dialog Session.
*/
/* %%Function:StopSubTimer %%Owner:peterj */
StopSubTimer(uas)
int uas;
{
	if (vuat.mpuasmsecStarted[uas] != 0l)
		{
		long msecNow = GetTickCount();
		vuat.mpuasmsec[uas] += (msecNow - vuat.mpuasmsecStarted[uas]);
		vuat.mpuasmsecStarted[uas] = 0l;
		if (uas == uasDialogSession)
			LogUa(uaStopDialogSession);
		}
}

#endif /* RSH */

#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Filewin_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* Filewin_Last() */
