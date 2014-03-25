/* saveFast.c */

/*
	Quick save is a new saving process which adds modification
	information to the end of the file rather than going through the
	laborious process of reconstructing and rewriting the file from
	scratch.  Two types of file formats will now be supported by Word:
	a "non-complex" format - the result of a regular save, and a
	"complex" format - the result of a quick save.

	D E F I N I T I O N S
	---------------------
	FIB     File Information Block
	FKP     Formatted disK Page     (contains CHPs or PAPs)
	CHP     CHaracter Properties
	PAP     PAragraph Properties
	SEP     SEction Properties
	SED     SEction Descriptor
	SETB    SEction TaBle

	W O R D   I I   N O N    C O M P L E X   F I L E    F O R M A T
	---------------------------------------------------------------
	NOTE: each section of file starts on a 128 byte page boundary

	FIB
	text
	group of SEPXs (section exceptions)
	group of PICs  (pictures )
	FKP for CHPXs
	FKP for PAPXs
	STSH         (style sheet)
	PlcffndRef   (footnote reference PLC)
	PlcffndText  (footnote text PLC)
	Plcfsed      (section descriptor PLC)
	Plcfpgd      (page descriptor PLC)
	Sttbfglsy    (if glossary, the glossary name table)
	Plcfglsy     (if glossary, glossary text PLC)
	Plcfhdd      (header PLC)
	PlcfbteChpx  (character exception bin table PLC)
	PlcfbtePapx  (paragraph exception bin table PLC)
	Plcfsea      (private PLC)
	Rgftc        font code array

	W O R D   I I    C O M P L E X   F I L E    F O R M A T
	---------------------------------------------------------------
	NOTE: each section of file starts on a 128 byte page boundary

	FIB
	text             ----
	group of SEPXs       |
	group of PICs        | These pages are never altered after they
	FKP for CHPXs        | have been written for the first time.
	FKP for PAPXs        |
	File Extension(s)----
	STSH         (style sheet)
	PlcffndRef   (footnote reference PLC)
	PlcffndText  (footnote text PLC)
	Plcfsed      (section descriptor PLC)
	Plcfpgd      (page descriptor PLC)
	Sttbfglsy    (if glossary, the glossary name table)
	Plcfglsy     (if glossary, glossary text PLC)
	Plcfhdd      (header PLC)
	PlcfbteChpx  (character exception bin table PLC)
	PlcfbtePapx  (paragraph exception bin table PLC)
	Plcfsea      (private PLC)
	Rgftc        font code array
	Clx          (group of sprms followed by piece descriptor PLC)

	each File Extention consists of the following:
		(all parts start on a 128 byte page boundary)
		extra text if any
		extra SEPXs if any
		extra PICs if any
		extra FKP for CHPs (if necessary)
		extra FKP for PAPs (if necessary)


-----------------------------------------------------------------------------
EXAMPLE HISTORY OF A FILE SAVED WITH THE QUICK-SAVE OPTION

NON-COMPLEX     AFTER FIRST QUICK-SAVE          AFTER SECOND QUICK-SAVE
-----------     ----------------------          -----------------------
FIB             FIB             (updated)       FIB             (updated)
text            text            (unchanged)     text            (unchanged)
group of SEPXs  group of SEPXs  (unchanged)     group of SEPXs  (unchanged)
group of PICs   group of PICs   (unchanged)     group of PICs   (unchanged)
FKP for CHPs    FKP for CHPs    (unchanged)     FKP for CHPs    (unchanged)
FKP for PAPs    FKP for PAPs    (unchanged)     FKP for PAPs    (unchanged)
STSH            STSH            (unchanged)     File Extension1 (unchanged)
PlcffndRef      File Extension1 (new)           File Extension2 (new)
PlcffndText     PlcffndRef      (rewritten)     STSH           (rewritten when changed)
Plcfsed         PlcffndText     (rewritten)     PlcffndRef     (rewritten)
Plcfpgd         Plcfsed         (rewritten)     PlcffndText    (rewritten)
Sttbfglsy       Plcfpgd         (rewritten)     Plcfsed        (rewritten)
Plcfglsy        Sttbfglsy       (rewritten)     Plcfpgd        (rewritten)
Plcfhdd         Plcfglsy        (rewritten)     Sttbfglsy      (rewritten)
PlcfbteChpx     Plcfhdd         (rewritten)     Plcfglsy       (rewritten)
PlcfbtePapx     PlcfbteChpx     (rewritten)     Plcfhdd        (rewritten)
Plcfsea         PlcfbtePapx     (rewritten)     PlcfbteChpx    (rewritten)
Rgftc           Plcfsea         (rewritten)     PlcfbtePapx    (rewritten)
Prr (for JR)    Rgftc           (rewritten)     Plcfsea        (rewritten)
		Prr             (rewritten)     Rgftc          (rewritten)
		Clx             (new)           Prr            (rewritten)
						Clx            (rewritten with changes)


*/
/* =======================================================================*/
#ifdef MAC
#define CONTROLS
#define WINDOWS
#define OSSTUFF
#define DIALOGS
#include "toolbox.h"
#endif /* MAC */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "file.h"
#include "props.h"
#include "prm.h"
#include "doc.h"
#include "disp.h"
#include "fkp.h"
#include "pic.h"
#include "saveFast.h"
#include "border.h"
#include "sel.h"
#include "debug.h"
#include "ch.h"
#include "error.h"

#ifdef MAC
#include "printMac.h"
#include "mac.h"  /* for replay of EcOpenFile in FAppendFnFetchPcdToFbb */
#else
#include "inter.h"
#include "message.h"
#include "menu2.h"
#include "cmdtbl.h"
#include "version.h"
#include "print.h"
#include "keys.h"
#endif /* MAC */



#ifdef MAC
#define ReportQuickSavePercent(cp1,cp2,cp3)		\
	if (vpqsib->docSave != docGlsy) 	\
		SetLlcPercentCpQuick(cp1,cp2,cp3)
#endif
#ifdef WIN
#define ReportQuickSavePercent(cp1,cp2,cp3) ReportSavePercent(cp1,cp2,cp3,fTrue)
#endif


#define witInit 	0
#define witPieceText	1
#define witPieceTextFtn 2
#define witPieceTextHdr 3
#define witPieceTextMcr 4
#define witPieceTextAtn 5
#define witExtraPiece	6
#define witSepx 	7

#define witPicFirst 	8
#define witPics 	8
#define witPicDocFtn	9
#define witPicDocHdr	10
#define witPicDocMcr	11
#define witPicDocAtn	12
#define witPicMax	13

#define witFinished	13


#define pn1 1

#ifdef MAC
PN PnWhoseFcGEFc();
FC FcFromPn();
PN PnFromFc();
PO PoWhoseFcGEFc();
PO PoFromFc();
long LcbFreeMacMem();
#endif

extern struct FCB       **mpfnhfcb[];
extern struct DOD       **mpdochdod[];
extern CP     vcpFetch;
extern int    vccpFetch;
extern FC     vfcFetchPic;
extern struct BPTB      vbptbExt;
extern struct CHP       vchpFetch;
extern struct PIC       vpicFetch;
extern struct TAP       vtapFetch;
extern struct PE        vpeFetch;
extern struct CHP       vchpStc;
extern struct PAP       vpapFetch;
extern struct SEP       vsepFetch;
extern int		vdocFetch;
extern struct SPX       mpiwspxSep[];
extern int              vibp;
extern int              ccpChp;
extern int              fnFetch;
extern int    *pwMax;
extern int              vccpFetch;
extern char HUGE        *vhpchFetch;
extern struct MERR      vmerr;
extern struct CA        caPara;
extern struct CA        caTap;
extern struct CA        caTable;
extern struct SEL       selCur;
extern struct SAB       vsab;
extern int FGtL();
extern int              (*vrgftc)[];
extern int              viftcMac;
extern int		docGlsy;
extern int     vdocPapLast;
extern int              vfScanPad;
extern int              vfDeletePad;  /* plcpad should be deleted for successful save */
extern int              vcpheHeight;  /* number of valid heights in plcpad */
extern long		vlcbExtAvail;
extern BOOL             vfUrgentAlloc;
#ifdef DEBUG
extern struct DBS	vdbs;
#endif /* DEBUG */
#ifdef WIN
extern struct PRI       vpri;
extern HPSYT		vhpsyt;
#else
extern struct PREF	vpref;
#endif

uns vdfcTblBackup;
HQ vhqchTblBackup = hqNil;

FC FcBegOfExt();

#ifdef MAC
FC			vfcBegWrite;
FC			vdfcSpaceAvail;
extern struct SPX       mpiwspxSepEquivW3[];
extern uns              cwHeapFree;
extern uns              *pfgrMac;
extern int              vfWdsLlcSaved;
extern struct WDS       vwdsLlc;
#else
#define mpiwspxSepEquivW3 0
#endif /* MAC */

struct QSIB  *vpqsib;
struct PLC **vhplcpeq;
FC vfcDestLim;
int vfRoomInPrevExt;
FC vfcPrevExtLim;
FC vfcPrevExtPageLim;

FC vfcSepxDestLim;
int vfSepxRoomInPrevExt;
FC vfcSepxPrevExtLim;
FC vfcSepxPrevExtPageLim;

FC vfcPicDestLim;
int vfPicRoomInPrevExt;
FC vfcPicPrevExtLim;
FC vfcPicPrevExtPageLim;

/* fast io block for fast output buffering & flushing */
struct FBB	vfbb;

/* FQuicksaveOk, etc moved to save.c to avoid swapping savefast.c in during
non-native saves. */

#ifdef MAC
/* %%Function:LcbExtHplc %%Owner:NOTUSED */
long LcbExtHplc(hplc)
struct PLC **hplc;
{
	long lcb;
	struct PLC *pplc;

	if (hplc == hNil)
		return 0L;
	pplc = *hplc;
	if (!pplc->fExternal)
		return 0L;
	lcb = (pplc->cb + sizeof(CP)) * pplc->iMac + sizeof(CP);
	return lcb;
}


#endif /* MAC */

/* (pj 3/12): called from WriteChps which is NATIVE */
/* %%Function:FFileWriteError %%Owner:davidlu */
NATIVE FFileWriteError()
{
	if (vfcDestLim >= fcMax)
		{{ /* !NATIVE - FFileWriteError */
		ErrorEid(eidFcTooBig, "FFileWriteError");
		vmerr.fDiskAlert = fTrue;
		return fTrue;
		}}
	return vmerr.fDiskAlert || vmerr.fDiskWriteErr
			Mac( || (vfcDestLim - vfcBegWrite > vdfcSpaceAvail) );
}


/* F   Q U I C K   S A V E */
/* %%Function:FQuickSave %%Owner:davidlu */
FQuickSave(fnDest, pdsr, fCompleteSave, pmsr)
int fnDest;
struct DSR *pdsr; /* doc save record, info about this doc */
BOOL fCompleteSave;
struct MSR *pmsr; /* multi-doc save record, state between 2 calls for 1 fn */
{
	int docSave;
	struct FCB *pfcb;
	struct DOD *pdod;
	int    fStshDirty;
	int    ibteMacChp, ibteMacPap;
	FC     fcChpLim, fcPapLim;
	struct PLCBTE  **hplcbte;
	long   lcbDocExt;
	PN     pnChp;
	PN     pnPap;
	FC     fcFkpLim;
	FC     fcPos;
	FC     fcChpExtFirst;
	FC     fcPapExtFirst;
	FC     cbFileMacOrig;
	struct SED **hplcsed;
	struct CA caT1, caT2;
	char HUGE *hpchTblBackup;
	struct QSIB  qsib;
	struct FIB fib;
	char   rgbFkpChpSave[cbSector];
	char   rgbFkpPapSave[cbSector];
	FC     fcBegTables;

	docSave = pdsr->doc;

	Scribble (ispQuickSave, fCompleteSave ? 'S' : 'Q');
	Debug(vdbs.fQuicksave = fTrue);

	vpqsib = &qsib;
	SetBytes(vpqsib, 0, sizeof(struct QSIB));
	qsib.fWord3 = WinMac( fFalse, pdsr->fWord3 );

	/* record the destination fn and doc in *vpqsib for the benefit of
		the routines we will be calling later. */
	qsib.fnDest = fnDest;
	qsib.docSave = docSave;

/* Record the sizes of the sub-docs and (for !fCompleteSave) fold the
	sub-doc text into the main doc. */
	if (!FPrepareDocForSave (pdsr, fCompleteSave))
		return fFalse;

/* we clear the mat here, because we may failed to fast save because of 
	out of memory conditions. We would then call FQuickSave again to try a 
	full save */
	vmerr.mat = matNil;

	pdod = PdodDoc(docSave);

/* figure out where we are writing the file */
#ifdef WIN
	Assert(!pdod->fShort);

	if (pdod->fGlsy)  /* glsy document is appended to DOT */
		{
		Assert (pmsr != NULL);
		qsib.pnFib = pmsr->pnFibSecond;
		qsib.fWritingSecond = fTrue;
		}
	else
		{
		qsib.pnFib = pn0;
		qsib.fWritingSecond = fFalse;
		}

	if (pdod->fDot && pdod->docGlsy != docNil)
		/*	saving DOT with a glsy, make a compound document */
		{
		Assert (fCompleteSave);
		Assert (pmsr != NULL);
		qsib.fFirstOfTwo = fTrue;
		}
	else
		qsib.fFirstOfTwo = fFalse;

	qsib.fcMin = cbFileHeader + FcFromPn(qsib.pnFib);

#else /* MAC */
	qsib.fcMin = (!qsib.fWord3) ? cbFileHeader : (cbSectorPre35 * 2);
	qsib.pnFib = pn0;
#endif /* WIN */


/* we will calculate the initial values for the assignment loops.*/
	if (fCompleteSave)
		{
		SetBytes(&fib, 0, cbFIB);
		/* save position where we will begin to write */
		Mac( vfcBegWrite = 0L );
		qsib.fcDestLim = qsib.fcMin;
#ifdef MAC
		lcbDocExt =  LcbExtHplc(pdod->hplcpgd) + LcbExtHplc(pdod->hplcfrd) +
				LcbExtHplc(pdod->hplcsed) + LcbExtHplc(pdod->hplcsea) + cbMinPlcpcd;
		if (pdod->docFtn)
			lcbDocExt +=  LcbExtHplc(pdod->hplcpgd) + LcbExtHplc(pdod->hplcfnd) + cbMinPlcpcd;
		if (pdod->docHdr)
			lcbDocExt +=  LcbExtHplc(pdod->hplcpgd) + LcbExtHplc(pdod->hplchdd) + cbMinPlcpcd;
		vpqsib->lcbAvailBte = vlcbExtAvail - lcbDocExt;
		if (vpqsib->lcbAvailBte < 0)
			vpqsib->lcbAvailBte = 0L;
#endif /* MAC */
		}
	else
		{
		fcBegTables = qsib.fcDestLim =
				FcBegOfExt(fnDest, docSave, &fib);

		Assert (fib.pnNext == pn0);  /* cannot quicksave over compound file */

		/*  cannot quicksave to older formats, force full save */
		if (fib.nFib != nFibCurrent || fib.pnNext != pn0 || 
				fib.cQuickSaves >= 15 || fib.wIdent != wMagic)
			goto LBackupError;

		/* Backup table info */
		Assert(vhqchTblBackup != hqNil);
		hpchTblBackup = WinMac(HpOfHq(vhqchTblBackup),
				LpLockHq(vhqchTblBackup));
		/* save position where we will begin to write so we can
			later determine if we've used up all available space
			on disk. */
		Mac( vfcBegWrite = qsib.fcDestLim );
		/* position to location in file where we will be writing new
			data and copy tables into a Mac heap buffer from that
			point to the end of the file. */
		if (EcReadWriteFn(fnDest, fcBegTables, hpchTblBackup, 
				umin(vdfcTblBackup, 32767), fFalse/*fWrite*/,fFalse/*fErrors*/) < 0
				|| (vdfcTblBackup > 32767 &&
				EcReadWriteFn(fnDest, fcBegTables+32767, hpchTblBackup+32767, 
				(uns)vdfcTblBackup-32767, fFalse/*fWrite*/,fFalse/*fErrors*/) < 0))
			{
LBackupError:
		/* if we have problems making the backup, throw away backup
			buffer and return false so we will proceed with the
			complete save processing in SaveFile. */
			Mac(UnlockHq(vhqchTblBackup));
			FreeHq(vhqchTblBackup);
			vhqchTblBackup = hqNil;
			return (fFalse);
			}
		/* save original size of file so we can restore file to that
			size if we detect an error. */
		cbFileMacOrig = PfcbFn(fnDest)->cbMac;
		Mac(UnlockHq(vhqchTblBackup));
		}
	qsib.fHasPicFib = fib.fHasPic;
	vpqsib->fCompleteSave = fCompleteSave;
	/* allocate fc equivalence table, used to map source file FCs into
		destination file FCs. */
	Assert (vhplcpeq == hNil);
	vhplcpeq = hNil;
	if (!fCompleteSave && ((vhplcpeq = (struct PLC **) HplcInit(cbPeq, 0, cpMax, fTrue /* ext rgFoo */)) == hNil))
		goto LBackupError;

#ifdef MAC
	/* find out how much space is available on disk */
	vdfcSpaceAvail = LcbAvailForVol((PfcbFn(fnDest))->vol, fFalse);
#endif

	pdod = PdodDoc(docSave);
	CompletelyAdjustHplcCps(pdod->hplcpcd);
	if (hplcsed = pdod->hplcsed)
		CompletelyAdjustHplcCps(hplcsed);
	if (pdsr->ccpFtn)
		CompletelyAdjustHplcCps(PdodDoc(pdod->docFtn)->hplcpcd);
	if (pdsr->ccpHdr)
		CompletelyAdjustHplcCps(PdodDoc(pdod->docHdr)->hplcpcd);
#ifdef WIN
	if (pdsr->ccpMcr)
		CompletelyAdjustHplcCps(PdodDoc(pdod->docMcr)->hplcpcd);
	if (pdsr->ccpAtn)
		CompletelyAdjustHplcCps(PdodDoc(pdod->docAtn)->hplcpcd);
#endif /* WIN */

	qsib.icpPicMac = 0;
	qsib.rgcpPic[0] = (docSave == docScrap ? vsab.fMayHavePic : PdodDoc(docSave)->fMayHavePic) ? cp0 : CpMacDoc(docSave);



/*      qsib.fHavePicFirst = fFalse;
	qsib.cpPicFirst = cp0;    */

	fStshDirty = pdod->fStshDirty;  /* record style sheet dirtyness*/

	/* the last entry of plcbteChpx, gives the FC of the end of
		the text, graphics, and SEPXs in the previous extension.
		In the current scheme, character RUNs whose FPROP has the
		the fSpec bit set on are written to describe the allocations
		within the file for graphics and SEPXs. */

	pfcb = PfcbFn(fnDest);
	hplcbte = pfcb->hplcbteChp;

	/* find fc within the previous extension where new text, graphics,
		and SEPXs can be written if there is room. */
	fcFkpLim = qsib.fcPrevExtLim = fcChpLim =
			(FC) CpPlc(hplcbte, ibteMacChp = IMacPlc(hplcbte));

	if (!fCompleteSave)
		{
		/* determine the fcLim of the last FKP written to the file. */
		GetPlc(hplcbte, ibteMacChp - 1, &pnChp);
		hplcbte = pfcb->hplcbtePap;
		fcPapLim = (FC) CpPlc(hplcbte, ibteMacPap = IMacPlc(hplcbte));

		GetPlc(hplcbte, ibteMacPap - 1, &pnPap);

		fcFkpLim = FcFromPn(max(pnChp,pnPap) + 1);

		/* save the last CHP and PAP FKPs so they can be restored if
			we have an error while fast saving. */
		bltbh(HpchGetPn(fnDest, pnChp), rgbFkpChpSave, cbSector);
		bltbh(HpchGetPn(fnDest, pnPap), rgbFkpPapSave, cbSector);
		}

	/* qsib.fcPrevExtLim >= fcFkpLim means that no new CHP or PAP
		FKPs were written for the last extension. In this case there is
		no island of free space to allocate for text so set fRoomInPrevExt
		to false. */
	if (qsib.fcPrevExtLim >= fcFkpLim)
		qsib.fRoomInPrevExt = fFalse;
	else
		{
		/* else we do have an island of free space we can use for
			allocation, so set bit and calculate limit of island */
		qsib.fRoomInPrevExt = fTrue;
		/* since we know the FKP that follows the last text begins
			on the next page boundary, the fc at the page boundary
			is the Lim of the island. */
		Assert(!vpqsib->fWord3);
		qsib.fcPrevExtPageLim =
				FcFromPn(PnWhoseFcGEFc(qsib.fcPrevExtLim));
		}


	/* store the current position and size of the stshf so that the
		qsib is already set up if we decide it is unnecessary to rewrite
		the stshf. */
	qsib.fcStshf = fib.fcStshf;
	qsib.cbStshf = fib.cbStshf;

	/* record the bounds of the original style sheet allocation */
	qsib.fcStshOrigFirst = fib.fcStshfOrig;
	qsib.fcStshOrigLim = qsib.fcStshOrigFirst + fib.cbStshfOrig;

	/* if the style sheet is not stored within its original allocation,
		(ie. within an extension) or it is dirty we must rewrite the
		style sheet. */
	qsib.fRewriteStsh = (fCompleteSave || fib.fcStshf != fib.fcStshfOrig ||
			fStshDirty);


	qsib.iprc = 0;

/* now we begin to write the file */
	Scribble(ispQuickSave,'P');
#ifdef MAC /* already 0 in Opus */
	/* force display of 0% in llc. */
	ReportQuickSavePercent (cp0, 1L, cp0); /* 0% */
#endif /* MAC */
	vpqsib->fAlreadyWritten = fFalse;
	vfcDestLim = qsib.fcDestLim;
	/* set fDirty pointer of file etc. */
	if (qsib.fRewriteStsh)
		if (!FQuicksaveStsh(fTrue /* first attempt */))
			goto ErrRet;
	qsib.fcBegText = vfcDestLim;
	if (!FQuicksaveText())
		goto ErrRet;
	vpqsib->fAlreadyWritten = fTrue;

	qsib.fcBegOfTables = vfcDestLim;
#ifdef MAC
	if (vpqsib->fWord3)
		{
		qsib.pnChpExtFirst = (PN) PoWhoseFcGEFc(qsib.fcBegOfTables);
		SetFnPos(fnDest, fcChpExtFirst = FcFromPo(qsib.pnChpExtFirst));
		}
	else
#endif
			{
			qsib.pnChpExtFirst = PnWhoseFcGEFc(qsib.fcBegOfTables);
			SetFnPos(fnDest, fcChpExtFirst = FcFromPn(qsib.pnChpExtFirst));
			}

	if (!FQuicksaveChps(pdsr))
		goto ErrRet;
#ifdef MAC
	if (vpqsib->fWord3)
		qsib.pnPapxExtFirst = (PO)PoFromFc(fcPos = fcPapExtFirst = (PfcbFn(fnDest))->fcPos);
	else
#endif
		qsib.pnPapxExtFirst = PnFromFc(fcPos = fcPapExtFirst = (PfcbFn(fnDest))->fcPos);
	if (fcChpExtFirst != fcPos)
		qsib.fcBegOfTables = fcPos;

	/* assert that FQuicksaveChps leaves pfcb->fcPos setup so
		that FQuicksavePaps will run properly. */
	Assert(Mac( vpqsib->fWord3 || ) FcFromPn(qsib.pnPapxExtFirst) ==
			((struct FCB *)PfcbFn(fnDest))->fcPos);
#ifdef MAC
	Assert(!vpqsib->fWord3  || FcFromPo(qsib.pnPapxExtFirst) ==
			((struct FCB *)PfcbFn(fnDest))->fcPos);
#endif
	if (!FQuicksavePaps(pdsr))
		goto ErrRet;
	if (fcPapExtFirst != (fcPos = (PfcbFn(fnDest))->fcPos))
		qsib.fcBegOfTables = fcPos;

	vfcDestLim = qsib.fcBegOfTables;
	if (qsib.fRewriteStsh)
		if (!FQuicksaveStsh(fFalse /* second attempt */))
			goto ErrRet;
	if (!FSaveTbls())
		goto ErrRet;

	if (FFileWriteError())
		goto ErrRet;

	/* now pad the file with zeros out to the next cbSector boundary. */
	qsib.fcDocLim = ((vfcDestLim + (cbSector - 1)) >> shftSector) << shftSector;

	Mac( ZeroFileRange(fnDest, vfcDestLim, qsib.fcDocLim) );

	/* force display of 100% in llc. */
	ReportQuickSavePercent (cp0, 1L, 1L); /* 100% */

	/* finally update the FIB of the destination file */
#ifdef MAC
	if (vpqsib->fWord3)
		UpdateFib30(pdsr->ccpText, pdsr->ccpFtn, pdsr->ccpHdr);
	else
#endif
		UpdateFib(pdsr, pmsr);

	/* and flush  it */
#ifdef MAC  /* WIN does these later, don't want to do them twice! */
	if (!FFlushFile(fnDest))
		goto ErrRet;
	if (FFileWriteError())
		goto ErrRet;
	UndirtyDoc(docSave);
#endif /* MAC */
	PdodDoc (docSave)->fFormatted = fTrue;
	PdodDoc(docSave)->fStyDirtyBroken = fFalse;

	FreePhplc(&vhplcpeq);
	if (vhqchTblBackup != hqNil)
		{
		FreeHq(vhqchTblBackup);
		vhqchTblBackup = hqNil;
		}
	if (!fCompleteSave)
		{
		int ipcd;
		int ipcdMac;
		int fSkip;
		FC cfc, fcDestFirst;
		struct PLC **hplcpcd;
		struct PCD pcd;

		Assert(!vpqsib->fWord3);
		/* first fixup the piece table to point to the text that was added to
			to fnDest. */
		vfcDestLim = vpqsib->fcBegText;
		vfRoomInPrevExt = vpqsib->fRoomInPrevExt;
		vfcPrevExtLim = vpqsib->fcPrevExtLim;
		vfcPrevExtPageLim = vpqsib->fcPrevExtPageLim;

		hplcpcd = (PdodDoc(docSave))->hplcpcd;
		ipcdMac = IMacPlc(hplcpcd);
/* last piece should have been invisible to all editing and wasn't written*/
		if (ipcdMac > 1 && CpPlc(hplcpcd, ipcdMac - 1) >= CpMacDoc(docSave))
			ipcdMac--;
/* if the next to last piece describes nothing but the hidden section mark, we
	did not write it. */
		if (ipcdMac > 1 && CpPlc(hplcpcd, ipcdMac - 1) >= CpMacDoc(docSave))
			ipcdMac--;
		for (ipcd = 0; ipcd < ipcdMac; ipcd++)
			{
			GetPlc(hplcpcd, ipcd, &pcd);
			if (pcd.fn != fnDest)
				{
				cfc = CpPlc(hplcpcd, ipcd + 1) -
						CpPlc(hplcpcd, ipcd);

				AssignFcDest(cfc, &fcDestFirst, &vfcDestLim,
						&vfRoomInPrevExt, &vfcPrevExtLim, vfcPrevExtPageLim,
						&fSkip, fFalse, fFalse /*must fit in one page */);
				pcd.prm = 0;
				pcd.fn = fnDest;
				pcd.fc = fcDestFirst;
				PutPlc(hplcpcd, ipcd, &pcd);
				}
			}
		if ((PdodDoc(docSave))->hplcsed)
			{
			RetrieveHplcsed(fnDest, docSave, 0);
			if ((hplcsed = PdodDoc(docSave)->hplcsed) != hNil && 
					!FSectLimAtCp(docSave, CpMac1Doc(docSave)))
				{
				struct SED sed;
				sed.fUnk = fFalse;
				sed.fn = fnScratch;
				sed.fcSepx = fcNil;
/* This shouldn't fail since RetrieveHplcsed didn't alter the original
	external allocation which included space for the hidden section.*/
				AssertDo(FInsertInPlc(hplcsed, IMacPlc(hplcsed), 
						CpMac1Doc(docSave), &sed));
				}
			}
		vmerr.fSaveFail = fFalse;
		}
	else
		{
		FreeHplc(PdodDoc(docSave)->hplcphe);
		PdodDoc(docSave)->hplcphe = hNil;
		}
	Scribble(ispQuickSave, ' ');
	Debug(vdbs.fQuicksave = fFalse);
#ifdef MAC
	if (vfWdsLlcSaved)
		{
		RestoreWds(&vwdsLlc);
		vfWdsLlcSaved = fFalse;
		}
#endif /* MAC */
	return (fTrue);
ErrRet:
#ifdef MAC
	if (vfWdsLlcSaved)
		{
		RestoreWds(&vwdsLlc);
		vfWdsLlcSaved = fFalse;
		}
#endif /* MAC */
	MeltHpToZero();
	if (vmerr.fDiskWriteErr && docSave != docScrap)
		ErrorEid(eidDocNotSaved,"FQuickSave");
#ifdef MAC
	if (FDiskFull())
		{
		if (!vmerr.fDiskAlert)
			{
			vmerr.fDiskAlert = fTrue;
			AlertVol(SzSharedKey("Disk ~ is full.  Please save the document on a different disk.", DiskFull),
					 (PfcbFn(fnDest))->vol);
			}
		}
#endif /* MAC */

	FreePhplc(&vhplcpeq);
	if (!fCompleteSave)
		{
		Assert(vhqchTblBackup != hqNil);
		hpchTblBackup = WinMac(HpOfHq(vhqchTblBackup),
				LpLockHq(vhqchTblBackup));
		DeleteFnHashEntries(fnDest);
		pfcb = PfcbFn(fnDest);
		if (EcReadWriteFn(fnDest, fcBegTables, hpchTblBackup, 
				umin(vdfcTblBackup, 32767), fTrue/*fWrite*/,fFalse/*fErrors*/) < 0
				|| (vdfcTblBackup > 32767 &&
				EcReadWriteFn(fnDest, fcBegTables+32767, hpchTblBackup+32767, 
				(uns)vdfcTblBackup-32767, fTrue/*fWrite*/,fFalse/*fErrors*/) < 0))

			goto LFailNotice;
		if (EcReadWriteFn(fnDest, (long)pnChp * (long)cbSector,
				(char HUGE *)rgbFkpChpSave, cbSector, 
				fTrue/*fWrite*/,fFalse/*fErrors*/) < 0)
			goto LFailNotice;
		if (EcReadWriteFn(fnDest, (long)pnPap * (long)cbSector,
				(char HUGE *)rgbFkpPapSave,	cbSector, 
				fTrue/*fWrite*/,fFalse/*fErrors*/) < 0)
			{
LFailNotice:
			ErrorEid(eidDiskErrSaveNewName,"FQuickSave");
			(pfcb = PfcbFn(fnDest))->fReadOnly = fTrue;
			}
		Mac(UnlockHq(vhqchTblBackup));

		SetEofFn(fnDest, cbFileMacOrig);
/* need to restore the bin tables to what they were before the fast save */
		hplcbte = pfcb->hplcbteChp;
		if (ibteMacChp < IMacPlc(hplcbte))
			{
			ShrinkPlc(hplcbte, ibteMacChp+1, ibteMacChp,
					IMacPlc(hplcbte) - ibteMacChp);
			}

		Assert(IMacPlc(hplcbte) == ibteMacChp);
		PutCpPlc(hplcbte, ibteMacChp, fcChpLim);
		hplcbte = pfcb->hplcbtePap;
		if (ibteMacPap < IMacPlc(hplcbte))
			{
			ShrinkPlc(hplcbte, ibteMacPap+1, ibteMacPap,
					IMacPlc(hplcbte) - ibteMacPap);
			}
		Assert(IMacPlc(hplcbte) == ibteMacPap);
		PutCpPlc(hplcbte, ibteMacPap, fcPapLim);
		}
	if (vhqchTblBackup != hqNil)
		{
		FreeHq(vhqchTblBackup);
		vhqchTblBackup = hqNil;
		}
	Scribble(ispQuickSave,' ');
	Debug(vdbs.fQuicksave = fFalse);
	return(fFalse);
}       /* end FQuickSave */


/* F  P R E P A R E  D O C  F O R  S A V E */
/* %%Function:FPrepareDocForSave %%Owner:davidlu */
FPrepareDocForSave (pdsr, fCompleteSave)
struct DSR *pdsr;
BOOL fCompleteSave;

{
	struct DOD *pdod;
	struct CA caT1, caT2;
	CP ccpBase;
	int docSave = pdsr->doc;
	int docFtn, docHdr;
	int ipcd;
#ifdef WIN
	int docAtn, docMcr;
	struct PLC **hplcfld;
#endif /* WIN */

	pdod = PdodDoc(docSave);
	pdsr->ccpText = CpMacDoc(docSave);
	pdsr->ccpFtn = (docFtn = pdod->docFtn) ? CpMacDoc(docFtn) : cp0;
	pdsr->ccpHdr = (docHdr = pdod->docHdr) ? CpMacDoc(docHdr) : cp0;
#ifdef WIN
	pdsr->ccpMcr = (docMcr = pdod->fDot ? pdod->docMcr : docNil) ?
			CpMacDoc(docMcr) : cp0;
	pdsr->ccpAtn = (docAtn = pdod->docAtn) ? CpMacDoc(docAtn) : cp0;
#endif /* WIN */
	if (!fCompleteSave)
		{
/* when we fast save, we need the paragraph mark at CpMacDocEdit to be in
	its own piece, so we can properly copy section properties from the
	hidden section mark. */
		if ((ipcd = IpcdSplit(PdodDoc(docSave)->hplcpcd, CpMacDocEdit(docSave)))
				== iNil)
			return fFalse;
		if (CpPlc(PdodDoc(docSave)->hplcpcd, ipcd) != CpMacDocEdit(docSave))
			return fFalse;
/* if we fast save, we must be able to copy the docHdr and docFtn piece
tables to the tail of the main doc piece table. If there is not enough
heap to accomplish this we return fFalse. */
/* NOTE: it is the responsibility of the caller to call RestoreDocToCcpText
upon return from a FastSave attempt, so pieces for docHdr and docFtn can
be removed from the main doc piece table. */

		ccpBase = pdsr->ccpText;

/*  if any of the following FReplaceCps fail we bag
out, assuming that the callers of FQuickSave will clean up the main doc. */

#ifdef MAC
		if (docHdr || docFtn)
#endif /* MAC */
#ifdef WIN
			if (docHdr || docFtn || docAtn || docMcr)
#endif /* WIN */
		/* first add an extra chEop so we won't be inserting at
			CpMacDoc */
				if (!FReplace(PcaPoint(&caT1, docSave, ccpBase - ccpEop),
						fnSpec, (FC) fcSpecEop, ccpEop))
					return fFalse;
		if (docFtn)
			{
			struct DOD *pdodFtn;

		/* in each case we hide the field plc so that it does not
			get duplicated in the result.
		*/
#ifdef WIN
			hplcfld = (pdodFtn = PdodDoc(docFtn))->hplcfld;
			pdodFtn->hplcfld = hNil;
#endif

			FReplaceCps(PcaPoint(&caT1, docSave, ccpBase),
					PcaSet(&caT2, docFtn, cp0, pdsr->ccpFtn));
			ccpBase += pdsr->ccpFtn;
#ifdef WIN
			PdodDoc(docFtn)->hplcfld = hplcfld;
#endif
			if (vmerr.fMemFail)
				return (fFalse);
			}
		if (docHdr)
			{
#ifdef WIN
			hplcfld = PdodDoc(docHdr)->hplcfld;
			PdodDoc(docHdr)->hplcfld = hNil;
#endif
			FReplaceCps(PcaPoint(&caT1, docSave, ccpBase),
					PcaSet(&caT2, docHdr, cp0, pdsr->ccpHdr));
			ccpBase += pdsr->ccpHdr;
#ifdef WIN
			PdodDoc(docHdr)->hplcfld = hplcfld;
#endif
			if (vmerr.fMemFail)
				return (fFalse);
			}
#ifdef WIN
		if (docMcr)
			{
			hplcfld = PdodDoc(docMcr)->hplcfld;
			PdodDoc(docMcr)->hplcfld = hNil;

			FReplaceCps(PcaPoint(&caT1, docSave, ccpBase),
					PcaSet(&caT2, docMcr, cp0, pdsr->ccpMcr));
			ccpBase += pdsr->ccpMcr;
			PdodDoc(docMcr)->hplcfld = hplcfld;
			if (vmerr.fMemFail)
				return (fFalse);
			}
		if (docAtn)
			{
			hplcfld = PdodDoc(docAtn)->hplcfld;
			PdodDoc(docAtn)->hplcfld = hNil;

			FReplaceCps(PcaPoint(&caT1, docSave, ccpBase),
					PcaSet(&caT2, docAtn, cp0, pdsr->ccpAtn));
			ccpBase += pdsr->ccpAtn;
			PdodDoc(docAtn)->hplcfld = hplcfld;
			if (vmerr.fMemFail)
				return (fFalse);
			}
#endif /* WIN */
		}

	return fTrue;
}


/* %%Function:CbSttbf %%Owner:davidlu */
uns CbSttbf(psttb)
struct STTB *psttb;
{
	return psttb->bMax - (psttb->ibstMax - 1) * sizeof(int);
}


/* F  Q U I C K S A V E  S T S H  */
/* %%Function:FQuicksaveStsh %%Owner:davidlu */
FQuicksaveStsh(fFirstAttempt)
int     fFirstAttempt;
{
	int     fnDest;
	struct STSH *pstsh;
	struct PLESTCP *pplestcp;
	uns     cbsttbName, cbsttbfName, cbsttbChpx, cbsttbfChpx;
	uns     cbsttbPapx, cbsttbfPapx, cbplfestcp;
	uns     cbsttbBase;
	long    cbStshf;
	int     cbT;
	FC      fcDestStshLim;
	int     fMustUpdateDestLim;

	fnDest = vpqsib->fnDest;
	pstsh = &(*PdodDoc(vpqsib->docSave)).stsh;

	cbsttbfName = CbSttbf(*pstsh->hsttbName);

	cbsttbfChpx = CbSttbf(*pstsh->hsttbChpx);

#ifdef MAC
	if (vpqsib->fWord3)
		cbsttbfPapx = 0;
	else
#endif
		cbsttbfPapx = CbSttbf(*pstsh->hsttbPapx);

	pplestcp = *(pstsh->hplestcp);
	cbplfestcp = 2 + pplestcp->stcpMac * pplestcp->cb;

	cbStshf = sizeof(int) + cbsttbfName + cbsttbfChpx + cbsttbfPapx + cbplfestcp;

/* during a complete save, the stsh should be saved on the second attempt at
	the location given by vfcDestLim. */
	Assert(!vpqsib->fCompleteSave || (vpqsib->fcStshOrigFirst == vpqsib->fcStshOrigLim &&
			vpqsib->fcStshOrigFirst == 0L));
	Assert(cbStshf > 0 && vfcDestLim >= cbFileHeader);
	fMustUpdateDestLim = fFalse;
	if (vpqsib->fcStshOrigFirst + cbStshf <= vpqsib->fcStshOrigLim &&
			!vpqsib->fCompleteSave)
		{
		vpqsib->fcDestStshFirst = vpqsib->fcStshOrigFirst;
		if (vfcDestLim == vpqsib->fcStshOrigFirst)
			fMustUpdateDestLim = fTrue;
		}
	else
		{
		vpqsib->fcDestStshFirst = vfcDestLim;
		fMustUpdateDestLim = fTrue;
		}
	if (vpqsib->fcDestStshFirst == vpqsib->fcStshOrigFirst &&
			!vpqsib->fCompleteSave)
		{
		if (!fFirstAttempt)
			return (fTrue);
		}
	else  if (fFirstAttempt)
		return (fTrue);

	vpqsib->fcStshf = vpqsib->fcDestStshFirst;
	fcDestStshLim = vpqsib->fcDestStshFirst;

	FreezeHp();
	SetFnPos(fnDest, fcDestStshLim);
	WriteRgchToFn(fnDest, &pstsh->cstcStd, sizeof(int));
	fcDestStshLim += sizeof(int);
	WriteSttbToFile(pstsh->hsttbName, fnDest, &fcDestStshLim);
	WriteSttbToFile(pstsh->hsttbChpx, fnDest, &fcDestStshLim);

#ifdef MAC
	if (vpqsib->fWord3)
		{
		WriteWord3PapxToFile(pstsh->hsttbPapx, fnDest, &fcDestStshLim, &cbsttbfPapx);
		cbStshf += cbsttbfPapx;
		}
	else
#endif
		WriteSttbToFile(pstsh->hsttbPapx, fnDest, &fcDestStshLim);

	WritePlToFn(pstsh->hplestcp, fnDest, &fcDestStshLim, &cbT);
	MeltHp();
	vpqsib->cbStshf = cbStshf;
	if (FFileWriteError())
		return(fFalse);
	Assert(fcDestStshLim == vpqsib->fcStshf + vpqsib->cbStshf);

	if (fMustUpdateDestLim)
		vfcDestLim += cbStshf;
	return(fTrue);
}



/* F   Q U I C K S A V E   T E X T  */
/* %%Function:FQuicksaveText %%Owner:davidlu */
FQuicksaveText()
	{ /*
	DESCRIPTION:
	RETURNS:
*/
	FC fcDestTextFirst, cfc;
	int docFetch;
	int fnDest;
	int fCompleteSave;
	int fSkip;
	CP cpMinPiece;
	FC fcMac;
	int     wit;
	int     fPieceTableItem;
	struct PCD *ppcd;
	struct SED *psed;
	struct FCB  *pfcb;

	/* find the fcLim of the last BTE and store it in fcTextLastExtLim.
		This gives us the limit of the text written to the file by previous
		saves. Since the text is written in cbSector pages, it is likely
		that a good part of the last page is empty and could accept more
		pieces of text. We set fcPrevExtPageLim to point past the end
		of the last page. */
	/* Assume that there is room in last extension to write more text
		pieces. */

	fnDest = vpqsib->fnDest;
	fCompleteSave = vpqsib->fCompleteSave;
	vpqsib->fcMac = fcMac = fc0;
	wit = witInit;

	BeginFbb(fnDest, fCompleteSave);  /* begin fast io buffering if complete save*/
	if (FFileWriteError() || (vpqsib->fWord3 && vfbb.hqBuf == 0))
		goto LEndFbb;

	/* retrieve the fn, fc, and size of data which must be copied to the
		destination file from other files, an item at a time (First,
		piece table text, then PICs, then SEPXs). */
	for (;;)  /* loop till all non destination items identified */
		{
		RetrieveNextNonDestCp(&cpMinPiece, &docFetch, &cfc, &wit,
				&fPieceTableItem);
		/* if all items identified, break out of the loop */
		if (wit == witFinished)
			break;
		/* find the fcDestTextFirst, the fc in the destination
			file where the the piece stored at fcSrcTextFirst
			in the source file should be copied. */

		FcDestFromCpSrc(cpMinPiece, docFetch, cfc,
				&fcDestTextFirst, wit, &fSkip);

		if ((!fCompleteSave && vmerr.fMemFail) || FFileWriteError())
			{
LEndFbb:
			EndFbb();
			return (fFalse);
			}
		Assert(!fCompleteSave || !fSkip);
		if (wit < witSepx)
			fcMac = vfcDestLim;
		else
			vpqsib->fcMac = fcMac;	/* flag to ReplaceChTable */
		}
	if (vfcDestLim > (pfcb = PfcbFn(fnDest))->cbMac)
		pfcb->cbMac = vfcDestLim;
	vpqsib->fcMac = fcMac;
	EndFbb();
	return(fTrue);
}       /* end FQuicksaveText */


/* %%Function:RetrieveNextNonDestCp %%Owner:davidlu */
RetrieveNextNonDestCp(pcpMinPiece, pdoc, pcfc, pwit, pfPieceTableItem)
CP	*pcpMinPiece;
int	*pdoc;
FC      *pcfc;
int     *pwit;
int     *pfPieceTableItem;
{
	int     docSave, docFetch, docScan;
	int     fnDest;
	struct PLC **hplcpcd;
	struct PCD  pcd;
	int     ipcd;
	int     icpPic;
	CP      cp, cpRun, cpMac;
	int     cchSepx;
	struct PIC   *ppic;
	int     fCompleteSave;
	FC      fcPic;
	struct PLC **hplcsed;
	int     ised, isedMac;
	int	iCount = 0;
	struct SED    sed;
	struct DOD    *pdod;
	struct CHP    chp;
	char    rgchSepx[cchSepxMax];
	struct SEP    sepDefault;

	pdod = PdodDoc(docSave = vpqsib->docSave);
	*pdoc = docSave;
	cpMac = CpMacDoc(docSave);
	fnDest = vpqsib->fnDest;
	fCompleteSave = vpqsib->fCompleteSave;

	FreezeHp();

	for (;;)
		{
		switch (*pwit)
			{
		case witInit:
			vpqsib->ipcd = 0;
			vpqsib->ised = 0;
			vpqsib->icpPic = 0;
			vpqsib->fExtraPiece = fFalse;

			/* set beginning global contexts */
			vfcDestLim = vpqsib->fcBegText;
			vfRoomInPrevExt = vpqsib->fRoomInPrevExt;
			vfcPrevExtLim = vpqsib->fcPrevExtLim;
			vfcPrevExtPageLim = vpqsib->fcPrevExtPageLim;

			*pwit = witPieceText;
			/* fall through */
		case witPieceText:
			if (fCompleteSave)
				{
				if (vpqsib->ipcd++ == 0)
					{
					*pcpMinPiece = cp0;
					*pcfc = cpMac;
					*pfPieceTableItem = fTrue;
					MeltHp();
					return;
					}
				}
			else
				{
				hplcpcd = pdod->hplcpcd;
				for (ipcd= vpqsib->ipcd; ipcd < IMacPlc(hplcpcd);
						ipcd++)
					{
					GetPlc(hplcpcd, ipcd, &pcd);
					if (pcd.fn != fnDest)
						{
						*pcpMinPiece = CpPlc(hplcpcd, ipcd);
						*pcfc = CpPlc(hplcpcd, ipcd + 1) -
								*pcpMinPiece;
						if (ipcd + 2 >= IMacPlc(hplcpcd))
							{
							if (*pcpMinPiece >= cpMac)
								continue;
							if (*pcpMinPiece + *pcfc >= cpMac)
								*pcfc = cpMac - *pcpMinPiece;
							}
						*pfPieceTableItem = fTrue;
						vpqsib->ipcd = ipcd + 1;
						MeltHp();
						return;
						}
					}
				}
			if (fCompleteSave)
				{
				*pwit = witPieceTextFtn;
				if (pdod->docFtn)
					{
					*pcpMinPiece = cp0;
					*pdoc = pdod->docFtn;
					*pcfc = CpMacDoc(*pdoc);
					*pfPieceTableItem = fTrue;
					vpqsib->fExtraPiece = fTrue;
					MeltHp();
					return;
					}
			case witPieceTextFtn:
				*pwit = witPieceTextHdr;
				if (pdod->docHdr)
					{
					*pdoc = pdod->docHdr;
					*pcpMinPiece = cp0;
					*pcfc = CpMacDoc(*pdoc);
					*pfPieceTableItem = fTrue;
					vpqsib->fExtraPiece = fTrue;
					MeltHp();
					return;
					}
			case witPieceTextHdr:
#ifdef WIN
				*pwit = witPieceTextMcr;
				if (pdod->fDot && pdod->docMcr)
					{
					*pdoc = pdod->docMcr;
					*pcpMinPiece = cp0;
					*pcfc = CpMacDoc(*pdoc);
					*pfPieceTableItem = fTrue;
					vpqsib->fExtraPiece = fTrue;
					MeltHp();
					return;
					}
			case witPieceTextMcr:
				*pwit = witPieceTextAtn;
				if (pdod->docAtn)
					{
					*pdoc = pdod->docAtn;
					*pcpMinPiece = cp0;
					*pcfc = CpMacDoc(*pdoc);
					*pfPieceTableItem = fTrue;
					vpqsib->fExtraPiece = fTrue;
					MeltHp();
					return;
					}
			case witPieceTextAtn:
#endif /* WIN */
				if (vpqsib->fExtraPiece)
					{
					*pwit = witExtraPiece;
					*pcpMinPiece = CpMacDocEdit(docSave);
					*pcfc = ccpEop;
					*pfPieceTableItem = fTrue;
					MeltHp();
					return;
					}
				}
		case witExtraPiece:
			*pwit = witSepx;
			/* take snapshot of positioning context for sepxs */
			vfcSepxDestLim = vfcDestLim;
			vfSepxRoomInPrevExt = vfRoomInPrevExt;
			vfcSepxPrevExtLim = vfcPrevExtLim;
			vfcSepxPrevExtPageLim = vfcPrevExtPageLim;
			/* fall through */
		case witSepx:
			if (hplcsed = pdod->hplcsed)
				{
				isedMac = IMacPlc(hplcsed) - 1;
/* if the last section describes nothing but the hidden section mark, we
	will not write it. */
				if (isedMac > 1 && CpPlc(hplcsed, isedMac - 1) == cpMac)
					isedMac--;
				for (ised = vpqsib->ised; ised < isedMac; ised++)
					{
					GetPlc(hplcsed, ised, &sed);
					if (sed.fn != fnDest)
						{
						*pcpMinPiece = CpPlc(hplcsed, ised);
						CacheSect(docSave,  *pcpMinPiece);
						StandardSep(&sepDefault);
						cchSepx = CbGrpprlProp(fFalse, rgchSepx+1, cchSepxMax - 1,
								&vsepFetch, &sepDefault, cwSEP - 1,
								mpiwspxSep, (!vpqsib->fWord3 ? 0 : mpiwspxSepEquivW3));
						*pcfc = cchSepx > 0 ?
								cchSepx + 1 : fc0;
						*pfPieceTableItem = fFalse;
						vpqsib->ised = ised + 1;
						MeltHp();
						return;
						}
					}
				}
			*pwit = witPics;
			*pdoc = docSave;

			/* take snapshot of positioning context for pics */
			vfcPicDestLim = vfcDestLim;
			vfPicRoomInPrevExt = vfRoomInPrevExt;
			vfcPicPrevExtLim = vfcPrevExtLim;
			vfcPicPrevExtPageLim = vfcPrevExtPageLim;
			/* fall through */
		case witPics:
/* if we have already scanned the doc and know where the next non-destination
	picture is, look up it's cp in rgcpPic and do processing for found
	picture. */
			if ((icpPic = vpqsib->icpPic) < vpqsib->icpPicMac)
				{
				cp = vpqsib->rgcpPic[icpPic];
				vpqsib->icpPic++;
				CachePara(docSave, cp);
				FetchCp(docSave, cp, fcmProps);
				docScan = docSave;
				goto LHavePic;
				}
			hplcpcd = pdod->hplcpcd;

/* if we have exhausted the list of pictures whose positions we know, do
	setup so we can continue search in the rest of the document. */
			if (icpPic == vpqsib->icpPicMac)
				{
				vpqsib->cpPicFetch = vpqsib->rgcpPic[icpPic];
				vpqsib->icpPic++;
				if (vpqsib->cpPicFetch == cp0)
					{
					vpqsib->ipcd = -1;
					vpqsib->cpPicFetchLim = cp0;
					}
				else
					{
					vpqsib->ipcd = IInPlc(hplcpcd, vpqsib->cpPicFetch - 1);
					vpqsib->cpPicFetchLim =  CpPlc(hplcpcd, vpqsib->ipcd + 1);
					if (ipcd + 1 == IMacPlc(hplcpcd))
						vpqsib->cpPicFetchLim--;
					}
				}
/* if we have scanned beyond the bounds of a piece, find the next non-dest
	piece. */
			if (vpqsib->cpPicFetch >= vpqsib->cpPicFetchLim)
				{
LNextPiece:
				for (ipcd = ++vpqsib->ipcd; ipcd < IMacPlc(hplcpcd); ipcd++)
					{
					GetPlc(hplcpcd, ipcd, &pcd);
					if (pcd.fn != fnDest)
						{
						vpqsib->cpPicFetch = CpPlc(hplcpcd, ipcd);
						vpqsib->cpPicFetchLim = CpPlc(hplcpcd, ipcd + 1);
						if (ipcd + 1 == IMacPlc(hplcpcd))
							vpqsib->cpPicFetchLim--;
						break;
						}
					}
				vpqsib->ipcd = ipcd;
				}

/* if we've scanned all of the piece table, switch to enumerate the SEDs that
	need to be retargeted. */
			if (vpqsib->ipcd >= IMacPlc(hplcpcd) ||
					vpqsib->cpPicFetch >= CpMacDoc(docSave))
				{
/* if there's room left in the rgcp, record the cp of the end of the doc
	in the entry of rgcp that corresponds to icpPicMac to eliminate scanning
	thru CHPS during future passes. */
				if (vpqsib->icpPicMac + 1 < icpPicMax)
					vpqsib->rgcpPic[vpqsib->icpPicMac] =
							CpMacDoc(docSave);
/* if we're certain that there are no pictures in docSave or its subdocs we
	skip scanning for pics in the subdocs. */
				if (fCompleteSave && PdodDoc(docSave)->fMayHavePic)
					{
					if (docScan = (PdodDoc(docSave))->docFtn)
						{
						*pdoc = docScan;
						vpqsib->cpPicFetch = cp0;
						vpqsib->cpPicFetchLim = CpMacDoc(docScan);
						*pwit = witPicDocFtn;
						goto LHaveDocScan;
						}
LCheckDocHdr:
					if (docScan = (PdodDoc(docSave))->docHdr)
						{
						*pdoc = docScan;
						vpqsib->cpPicFetch = cp0;
						vpqsib->cpPicFetchLim = CpMacDoc(docScan);
						*pwit = witPicDocHdr;
						goto LHaveDocScan;
						}
#ifdef WIN
LCheckDocMcr:
					if (PdodDoc(docSave)->fDot &&
							(docScan = (PdodDoc(docSave))->docMcr))
						{
						*pdoc = docScan;
						vpqsib->cpPicFetch = cp0;
						vpqsib->cpPicFetchLim = CpMacDoc(docScan);
						*pwit = witPicDocMcr;
						goto LHaveDocScan;
						}
LCheckDocAtn:
					if (docScan = (PdodDoc(docSave))->docAtn)
						{
						*pdoc = docScan;
						vpqsib->cpPicFetch = cp0;
						vpqsib->cpPicFetchLim = CpMacDoc(docScan);
						*pwit = witPicDocAtn;
						goto LHaveDocScan;
						}
#endif /* WIN */
					}
				goto LEndIt;
				}
		case witPicDocFtn:
		case witPicDocHdr:
#ifdef WIN
		case witPicDocMcr:
		case witPicDocAtn:
#endif /* WIN */
			switch (*pwit)
				{
			case witPics:
				docScan = docSave;
				break;
			case witPicDocFtn:
				docScan = (PdodDoc(docSave))->docFtn;
				break;
			case witPicDocHdr:
				docScan = (PdodDoc(docSave))->docHdr;
				break;
#ifdef WIN
			case witPicDocMcr:
				Assert(PdodDoc(docSave)->fDot);
				docScan = PdodDoc(docSave)->docMcr;
				break;
			case witPicDocAtn:
				docScan = PdodDoc(docSave)->docAtn;
				break;
#endif /* WIN */
				}
LHaveDocScan:
			cp = vpqsib->cpPicFetch;
			CachePara(docScan, cp);
			FetchCp(docScan, cp, fcmProps);
			for (;;)
				{
				docFetch = docNil; /* assume sequential */
				if (vchpFetch.fSpec)
					{
					Assert(fnFetch != fnDest);
					cp = vcpFetch;
					if (ChFetch(docScan, cp, fcmChars+fcmProps)
							== chPicture)
						{
						if (vpqsib->icpPicMac + 1 < icpPicMax && *pwit == witPics)
							{
							vpqsib->rgcpPic[vpqsib->icpPicMac++] = cp;
							vpqsib->rgcpPic[vpqsib->icpPicMac] = cp + 1;
							}
LHavePic:
						vpqsib->cpPicFetch = cp + 1;
						vpqsib->fPicFound = fTrue;
						chp = vchpFetch;
						chp.fnPic = fnFetch;
						MeltHp();
						FetchPe(&chp, docScan, cp);
						*pdoc = docScan;
						*pcpMinPiece = cp;
						*pcfc = vpicFetch.lcb;
						*pfPieceTableItem = fFalse;
						return;
						}
					docFetch = docScan;  /* not sequential */
					}
				cp = vcpFetch + vccpFetch;
				if (cp >= vpqsib->cpPicFetchLim)
					{
					switch (*pwit)
						{
					case witPicDocFtn:
						goto LCheckDocHdr;
					case witPicDocHdr:
#ifdef WIN
						goto LCheckDocMcr;
					case witPicDocMcr:
						goto LCheckDocAtn;
					case witPicDocAtn:
#endif /* WIN */
						goto LEndIt;
						}
					vpqsib->cpPicFetch = cp;
					goto LNextPiece;
					}
				CachePara(docScan, cp);
				FetchCp(docFetch, cp, fcmProps);
				}
LEndIt:
/* if we found no pics during scan and either we did a complete save or else
	a fast save on a file that originally had no pictures, we know that there
	are no pictures in the file */
			if (!vpqsib->fPicFound && (fCompleteSave || !vpqsib->fHasPicFib))
				PdodDoc(docSave)->fMayHavePic = fFalse;
			*pwit = witFinished;
			MeltHp();
			return;
			}
		}
}  /* end RetrieveNextNonDestCp  */



/* given a run of characters (a run of text, a picture, or a sepx) which can
	be located in docSave using cpSrc, return in *pfcDestTextFirst, the fc
	that will be assigned to the piece in the destination file.  */

/* Maps run source fc to destination fc using PLCFEQ  */
/* %%Function:FcDestFromCpSrc %%Owner:davidlu */
FcDestFromCpSrc(cpPieceFirst, doc, cfc, pfcDestTextFirst, wit, pfSkip)
CP	cpPieceFirst;
int	doc;
FC      cfc;
FC      *pfcDestTextFirst;
int     wit;
int     *pfSkip;
{
	int     docSave;
	int     fnDest;
	int     fLwordAlign, fWithinPage;
	int     cchSepx;
	PN      pn;
	int     iPictRead;
#ifdef MAC
	int     ipe;
#endif /* MAC */
	CP      ccpPic;
	FC      fcPic;
	long    lcbCur;
	FC      fcFirstNextPage;
	int     iCount = 0;
	struct CHP chp;
	char    rgchSepx[cchSepxMax];
	char    rgch[256];
	struct SEP   sepDefault;
#define iPic 1
#define iPEBase 2
#define iPEData 3

	*pfSkip = fFalse;
	docSave = vpqsib->docSave;
	fnDest = vpqsib->fnDest;

	fLwordAlign = (wit >= witPicFirst && wit < witPicMax) ? fTrue : fFalse;
	fWithinPage = (wit == witSepx) ? fTrue : fFalse;

	if (wit == witSepx && cfc == 0)
		*pfcDestTextFirst = fcNil;
	else
		{
		AssignFcDest(cfc, pfcDestTextFirst, &vfcDestLim,
				&vfRoomInPrevExt, &vfcPrevExtLim, vfcPrevExtPageLim,
				pfSkip, fLwordAlign, fWithinPage);
		}

	if (FFileWriteError())
		return;

	if (!vpqsib->fAlreadyWritten)
		{
		CP cpSrc = cpPieceFirst;
		FC fcDest = *pfcDestTextFirst;
		FC cfcWrite = cfc;
		long lcbRemainSrc = 0L;
		int iPictRead = iPic;
		if (*pfcDestTextFirst != fcNil)
			SeekFbbToFc(*pfcDestTextFirst);
		while (cfcWrite > 0)
			{
			char HUGE *hpchSrcText, HUGE *hpchDestText;
			int cch, cchRemainDest;

			if (lcbRemainSrc == 0L)
				{
				switch (wit)
					{

#ifdef DEBUG
				default:
					Assert (fFalse);
#endif /* DEBUG */

				case witPieceText:

				case witExtraPiece:
					/* 0% to 33+1/3% */
					if ((iCount++ & 7) == 0)
						{
						CP cpMac = CpMacDoc(docSave);
						CP cpNum = cpSrc / 3;
						ReportQuickSavePercent (cp0,
								cpMac, cpNum);
						}
				case witPieceTextFtn:
				case witPieceTextHdr:
				case witPieceTextMcr:
				case witPieceTextAtn:
LRefetch:
					FetchCp(doc, cpSrc, fcmChars);
						{
						extern CP ccpPcd;
						extern FC fcFetch;
						if (fnFetch != fnSpec  && ccpPcd > cbSector && !PfcbFn(fnFetch)->fDirty &&
								vfbb.hqBuf != 0)
							{
						/* writing a large piece */
							if (!FAppendFnFetchPcdToFbb())
								{
								SetFnPos(vfbb.fnDest, fcDest);
								FreeHq(vfbb.hqBuf);
								vfbb.hqBuf = 0;
								goto LRefetch;
								}
							if (FFileWriteError())
								return;
							cpSrc += ccpPcd;
							fcDest += ccpPcd;
							cfcWrite -= ccpPcd;
							continue;  /*  while (cfcWrite > 0)  */
							}
						}
					hpchSrcText = vhpchFetch;
					lcbRemainSrc = vccpFetch;
					break;
				case witPics:
				case witPicDocFtn:
				case witPicDocHdr:
				case witPicDocMcr:
				case witPicDocAtn:
					switch (iPictRead)
						{
					case iPic:
						CachePara(doc, cpSrc);
						FetchCp(doc, cpSrc, fcmProps);
						chp = vchpFetch;
						chp.fnPic = fnFetch;
						FetchPe(&chp, doc, cpSrc);
						hpchSrcText = (char HUGE *)&vpicFetch;
						lcbRemainSrc = cbPIC;
#ifdef WIN
						/* Note: this differs from Mac */
						iPictRead = iPEData;
						ccpPic = vpicFetch.lcb - cbPIC;
						/* note vfcFetchPic is fc following
							PIC structure */
						fcPic = vfcFetchPic;


#endif /* WIN */
#ifdef MAC
						iPictRead = iPEBase;
#endif /* MAC */
						break;
#ifdef MAC
					case iPEBase:
						if (vpicFetch.cpe == 0)
							{
							/* Just a frame */
							Assert(cfcWrite == fc0);
							continue;
							}
						Assert(vpicFetch.cpe == 1);
						FetchPe(0, 0, cp0);
						hpchSrcText = (char HUGE *)&vpeFetch;
						lcbRemainSrc = cbPEBase;
						iPictRead = iPEData;
						ccpPic = vpeFetch.lcb - cbPEBase;
						fcPic = vfcFetchPic + (long) cbPEBase;
						break;
#endif /* MAC */
					case iPEData:
						if (ccpPic <= 0)
							{
							Assert(cfcWrite == fc0);
							continue;
							}
						FetchPeData(fcPic, (char HUGE *)rgch,
								lcbCur = CpMin(ccpPic,(CP)256));
						hpchSrcText = (char HUGE *)rgch;
						lcbRemainSrc = lcbCur;
						fcPic += lcbCur;
						ccpPic -= lcbCur;
						break;
						}
					break;
				case witSepx:
					CacheSect(docSave, cpSrc);
					StandardSep(&sepDefault);
					cchSepx = CbGrpprlProp(fFalse, rgchSepx+1, cchSepxMax - 1, &vsepFetch,
							&sepDefault, cwSEP - 1,
							mpiwspxSep, (!vpqsib->fWord3 ? 0 : mpiwspxSepEquivW3));
					hpchSrcText = (char HUGE *) rgchSepx;
					Assert(cchSepx < cbSector);
					rgchSepx[0] = cchSepx;
					lcbRemainSrc = rgchSepx[0] + 1;
					Assert(cfc == lcbRemainSrc);
					break;
					} /* switch (wit) */
				}  /*  if (lcbRemainSrc == 0)  */

			cch = (int)CpMin((CpMin/*FcMin*/(cfcWrite, (FC) cbSector)),
					lcbRemainSrc);
			Assert(wit != witSepx || cch == cfcWrite);
			AppendHpchToFbb(hpchSrcText, cch);

			if (FFileWriteError())
				return;

			cfcWrite -= (FC) cch;
			fcDest += (FC) cch;
			cpSrc += (CP) cch;
			hpchSrcText += cch;
			lcbRemainSrc -= cch;
			}  /*  while (cfcWrite > 0)  */
		} /* if (!vpqsib->fFirstPass && !vpqsib->fAlreadyWritten)  */
} /* end of FcDestFromFnSrcFc */


/* %%Function:AssignFcDest %%Owner:davidlu */
AssignFcDest(cfc, pfcDestTextFirst, pfcDestLim, pfRoomInPrevExt,
pfcPrevExtLim, fcPrevExtPageLim, pfSkip, fLwordAlign, fWithinPage)
FC cfc;
FC *pfcDestTextFirst;
FC *pfcDestLim;
int *pfRoomInPrevExt;
FC *pfcPrevExtLim;
FC fcPrevExtPageLim;
int *pfSkip;
int fLwordAlign;
{
	FC fcFirstNextPage;
	/* if there is still room in the previous extension and we
		are supposed to place the new piece beginning on a
		long word boundary, make sure that *pfcPrevExtLim begins
		on a long word boundary */
	if (*pfRoomInPrevExt && fLwordAlign &&
			(*pfcPrevExtLim & 3))
		*pfcPrevExtLim = ((*pfcPrevExtLim + 3) / 4) * 4;

	/* beacuse fcPrevExtPageLim marks a page boundary, it is
		unnecessary to pay attention to fWithinPage when we are
		trying to write in the previous extension. */
	Assert(!*pfRoomInPrevExt || fcPrevExtPageLim ==
			FcFromPn(PnFromFc(fcPrevExtPageLim)));

	if (*pfRoomInPrevExt &&
			*pfcPrevExtLim + cfc <= fcPrevExtPageLim)
		{
		*pfcDestTextFirst = *pfcPrevExtLim;
		*pfcPrevExtLim += cfc;
		}
	else
		{
		/* if we must long word align, make sure that
			*pfcDestLim is aligned. */
		if (fLwordAlign && (*pfcDestLim & 3))
			*pfcDestLim = ((*pfcDestLim + 3) / 4) * 4;
		/* if the data to be written should not span a page
			boundary make sure there is sufficent space. */
		if (fWithinPage)
			{
#ifdef WIN
			fcFirstNextPage = FcFromPn(PnFromFc(*pfcDestLim) + 1);
#else  /* MAC */
			fcFirstNextPage = (!vpqsib->fWord3) ? 
					FcFromPn(PnFromFc(*pfcDestLim) + 1) :
					FcFromPo(PoFromFc(*pfcDestLim) + 1);
#endif /* MAC */
			if (*pfcDestLim + cfc >
					fcFirstNextPage)
				*pfcDestLim = fcFirstNextPage;
			}
		/* assign the position within the destination file */
		*pfcDestTextFirst = *pfcDestLim;

		/* if we are making the transition between writing in
			the last extension and writing in the new extension,
			save the fc assigned so that when we are writing
			CHPs we can label non-text data we're skipping over
			with an fSpec CHP */
		if (*pfRoomInPrevExt)
			{
			/* Make sure field was properly initialized */
			*pfRoomInPrevExt = fFalse;
			/* if we are actually skipping beyond the
			end of the last extension, mark this place */
			if (*pfcDestLim > *pfcPrevExtLim)
				*pfSkip = fTrue;
			}
		/* push fc location where next piece is to be written */
		*pfcDestLim += cfc;
		}
}


/* F   Q U I C K S A V E   C H P S  */
/* %%Function:FQuicksaveChps %%Owner:davidlu */
FQuicksaveChps(pdsr)
struct DSR *pdsr;
{
	int fnDest;
	int docFetch;
	FC  fcDestTextFirst, cfc;
	int fnSrc;
	FC fcSrcFirst;
	FC fcSrcTextRunFirst, fcDestTextRunLim;
	FC fcSave;
	CP   cpMinPiece;
	int  fPieceTableItem;
	PN   pnLast;
	int  cchChp;
	int  wit;
	int  cchChpLast;
	int  fSkip;
	int  ibteMac;
	int  pnFirst;
	struct CHP *pchp;
	struct SED  *psed;
	struct FCB  *pfcbDest;
	struct FKP fkpChpDummy;
	struct FKPD fkpdChp;
	struct CHP  chpT;
	struct CHP  chpLast;
	struct PLCBTE **hplcbte;

	fnDest = vpqsib->fnDest;

	pfcbDest = PfcbFn(fnDest);

	/* read the last CHP fkp recorded in the destination file into
		fkpChpDummy and construct fkpdChp to describe the page read in */
	CopyLastFkp(pfcbDest->hplcbteChp, &fkpChpDummy, fnDest, &pnLast);
	InitFkpdFromFkp(&fkpChpDummy, &fkpdChp, pnLast);
	fkpdChp.fPlcIncomplete = pdsr->fFkpdChpIncomplete;
	vpqsib->fNonPieceTableItem = fFalse;
	vpqsib->pnChpFirst = WinMac(PnAlloc1(fnDest, fFalse),
			PnAlloc1(fnDest, fFalse, vpqsib->fWord3));

	wit = witInit;
	/* retrieve the fn, fc, and size of data which must be copied to the
		destination file from other files, an item at a time (First,
		piece table text, then PICs, then SEPXs). */
	for (;;)  /* loop till all non destination items identified */
		{
		RetrieveNextNonDestCp(&cpMinPiece, &docFetch, &cfc, &wit,
				&fPieceTableItem);
		/* if all items identified, break out of the loop */
		if (wit == witFinished)
			break;

		fcDestTextFirst = fcNil;
		if (wit != witSepx || cfc != 0)
			{
			/* retrieve piece */

			/* from fnSrc, and fcSrcTextFirst find the position that
				the piece has been assigned (fcDestTextFirst) in fnDest.
			*/

			FcDestFromCpSrc(cpMinPiece, docFetch, cfc,
					&fcDestTextFirst, wit, &fSkip);

			if ((!vpqsib->fCompleteSave && vmerr.fMemFail)
					|| FFileWriteError())
				return fFalse;

			/* now build CHPs for piece */
			/* if we skipped from writing in the last extension to
				the new extension and the extensions are not contiguous,
				we write a CHP with the fSpec bit on to describe the
				area of the file that was skipped. */
			if (fSkip)
				{
				StandardChp(&chpT);
				chpT.fSpec = fTrue;
				cchChp = CchNonZeroPrefix(&chpT, cbCHP);
				if (!FAddRun(fnDest,
						fcDestTextFirst,
						&chpT,
						cchChp,
						&fkpdChp,
						fFalse /* CHP */,
						fFalse /* use fcPos for new */,
						!vpqsib->fCompleteSave /* during fast save plcbte must expand, 
				during full save plcbte need not expand */,
						vpqsib->fWord3))
					{
					SetErrorMat( matLow );
					return fFalse;
					}
				}

			if (fPieceTableItem)
				{
				Assert(!vpqsib->fNonPieceTableItem);
				WriteChps(docFetch, fnDest, cpMinPiece, cfc, fcDestTextFirst,
						&fkpdChp, NULL);
				if ((!vpqsib->fCompleteSave && vmerr.fMemFail)
						|| FFileWriteError())
					return fFalse;
				} /* end if a piece table item */
			else
				{
				vpqsib->fNonPieceTableItem = fTrue;
				vpqsib->fcNonPieceLim = fcDestTextFirst + cfc;
				}
			}

		Assert(!fkpdChp.fPlcIncomplete || vpqsib->fCompleteSave);
#ifdef MAC 
		if (fkpdChp.fPlcIncomplete &&
				FCheckTooManyBtes(PfcbFn(fnDest)->hplcbteChp, &fkpdChp))
			return fFalse;
#endif /* MAC */

		}
	if (vpqsib->fNonPieceTableItem)
		{
		/* non-piece table info so set write CHP with fSpec */
		StandardChp(&chpT);
		chpT.fSpec = fTrue;
		/* add run/chp pair to the current destination fkp page */
		cchChp = CchNonZeroPrefix(&chpT, cbCHP);
		if (!FAddRun(fnDest,
				vpqsib->fcNonPieceLim,
				&chpT,
				cchChp,
				&fkpdChp,
				fFalse /* CHP */,
				fFalse /* use fcPos for new */,
		/* during fast save plcbte must expand, during full save plcbte need not expand */
		!vpqsib->fCompleteSave,
				vpqsib->fWord3))
			{
			SetErrorMat( matLow );
			return fFalse;
			}
		}
	hplcbte = (PfcbFn(fnDest))->hplcbteChp;
	PutCpPlc(hplcbte, ibteMac = IMacPlc(hplcbte), fkpdChp.fcFirst);
	Assert(ibteMac > 0);
	if (!vpqsib->fCompleteSave)
		{
		GetPlc(hplcbte, 0, &vpqsib->pnChpFirst);
		vpqsib->cpnBteChp = ibteMac;
		}
	else
		{
#ifdef DEBUG
		if (vpqsib->fFirstOfTwo)
			{
			GetPlc(hplcbte, 0, &pnFirst);
			Assert(pnFirst == vpqsib->pnChpFirst);
			}
#endif /* DEBUG */
		vpqsib->cpnBteChp = fkpdChp.pn - vpqsib->pnChpFirst + 1;
		vpqsib->lcbAvailBte -= vpqsib->cpnBteChp * (sizeof(CP) + sizeof(PN)) +
				sizeof(CP);
		pdsr->fFkpdChpIncomplete = fkpdChp.fPlcIncomplete;
		}
	return (fTrue);
}       /* end FQuicksaveChps */


/* F   Q U I C K S A V E   P A P S  */
/* %%Function:FQuicksavePaps %%Owner:davidlu */
FQuicksavePaps(pdsr)
struct DSR *pdsr;
{
	int fnDest;
	struct FKPD fkpdPap;
	FC fcDestTextFirst, cfc;
	int fnSrc;
	int docFetch;
	CP  cpMinPiece;
	FC fcSrcFirst;
	FC fcSrcTextRunFirst, fcDestTextRunLim;
	FC fcSave;
	PN pnLast;
	int wit;
	int fPieceTableItem;
	int fSkip;
	int fCompleteSave;
	struct FCB  *pfcbDest;
	struct FKP fkpPapDummy;
	int    cchPapx;
	int    ibteMac;
	int	 pnFirst;
	struct PCD  *ppcd;
	struct PAP  papStd;
	struct PLCBTE **hplcbte;
	char papx[cchPapxMax+1];

	fnDest = vpqsib->fnDest;
	fCompleteSave = vpqsib->fCompleteSave;
	pfcbDest = PfcbFn(fnDest);

	/* read the last PAP fkp recorded in the destination file into
		fkpPapDummy and construct fkpdPara to describe the page read in */
	CopyLastFkp(pfcbDest->hplcbtePap, &fkpPapDummy, fnDest, &pnLast);
	InitFkpdFromFkp(&fkpPapDummy, &fkpdPap, pnLast);
	fkpdPap.fPlcIncomplete = pdsr->fFkpdPapIncomplete;

	vpqsib->pnPapFirst = WinMac(PnAlloc1(fnDest, fFalse),
			PnAlloc1(fnDest, fFalse, vpqsib->fWord3));

	wit = witInit;
	/* retrieve the fn, fc, and size of data which must be copied to the
		destination file from other files, an item at a time (First,
		piece table text, then PICs, then SEPXs). */
	for (;;)  /* loop till all non destination items identified */
		{
		RetrieveNextNonDestCp(&cpMinPiece, &docFetch, &cfc, &wit,
				&fPieceTableItem);
		/* if all items identified  or item was a picture or a SEP
			then we can stop searching since we have identified
			all paragraph properties that must be copied.  */
		if (wit > witExtraPiece)
			break;

		/* from fnSrc, and fcSrcTextFirst find the position that
			the piece has been assigned (fcDestTextFirst) in fnDest.
			*/

		FcDestFromCpSrc(cpMinPiece, docFetch, cfc,
				&fcDestTextFirst, wit, &fSkip);

		if ((!fCompleteSave && vmerr.fMemFail) || FFileWriteError())
			return (fFalse);

		/* PAPs */
		WritePaps(docFetch, fnDest, cpMinPiece, cfc, fcDestTextFirst,
				&fkpdPap, NULL);
		if ((!fCompleteSave && vmerr.fMemFail) || FFileWriteError())
			return (fFalse);
		}

	hplcbte = PfcbFn(fnDest)->hplcbtePap;
	PutCpPlc(hplcbte, ibteMac = IMacPlc(hplcbte), fkpdPap.fcFirst);
	Assert(ibteMac > 0);
	if (!fCompleteSave)
		{
		GetPlc(hplcbte, 0, &vpqsib->pnPapFirst);
		vpqsib->cpnBtePap = ibteMac;
		}
	else
		{
#ifdef DEBUG
		if (vpqsib->fFirstOfTwo)
			{
			GetPlc(hplcbte, 0, &pnFirst);
			Assert(pnFirst == vpqsib->pnPapFirst);
			}
#endif /* DEBUG */		
		vpqsib->cpnBtePap = fkpdPap.pn - vpqsib->pnPapFirst + 1;
		pdsr->fFkpdPapIncomplete = fkpdPap.fPlcIncomplete;
		}
	return (fTrue);
}       /* end FQuicksavePaps */


/* %%Function:CopyLastFkp %%Owner:davidlu */
CopyLastFkp(hplcbte, pfkp, fn, ppn)
struct PLCBTE **hplcbte;
struct FKP   *pfkp;
int    fn;
int    *ppn;
{
	int     ibteLast;
	PN      pn;
	struct FKP HUGE  *hpfkpLast;

	if (!vpqsib->fCompleteSave)
		{
		Assert(!vpqsib->fWord3);
		GetPlc(hplcbte, IMacPlc(hplcbte) - 1, ppn);
		hpfkpLast = (struct FKP HUGE *) HpchGetPn(fn, *ppn);
		bltbh(hpfkpLast, pfkp,  cbSector);
		}
	else
		SetBytes(pfkp, 0, cbSector);
}


/* %%Function:InitFkpdFromFkp %%Owner:davidlu */
InitFkpdFromFkp(pfkp, pfkpd, pn)
struct FKP  *pfkp;
struct FKPD *pfkpd;
PN     pn;
{
	char   bMin;
	char   *rgb;
	int    ib;
	char   b;

	if (!vpqsib->fCompleteSave)
		{
		Assert(!vpqsib->fWord3);
		pfkpd->bFreeFirst = pfkp->crun * (sizeof(FC) + 1) +
				sizeof(FC);
		rgb = (char *) &(pfkp->rgfc[1 + pfkp->crun]);
		bMin = cbSector - 1;
		for (ib = 0; ib < pfkp->crun; ib++)
			{
			b = rgb[ib];
			if (b < bMin && b > 0)
				bMin = b;
			}

		pfkpd->fcFirst = pfkp->rgfc[pfkp->crun];
		pfkpd->bFreeLim = (bMin << 1);
		pfkpd->pn = pn;
		}
	else
		{
		SetBytes(pfkpd, 0, sizeof(struct FKPD));
		pfkpd->bFreeFirst = 1;
		pfkpd->fcFirst = vpqsib->fcMin;
		}
	pfkpd->fPlcIncomplete = fFalse;
}


/* F   S A V E   T B L S */
/* %%Function:FSaveTbls %%Owner:davidlu */
FSaveTbls()
{
	int     fnDest;
	int     docSave;
	int     docHdr;
	struct PLC  **hplc;
	struct PLC **hplcpcd;
	int    cbActual;
	struct PCD   pcd;
	FC     fcDest, fcDestFurthest;
	FC     fc, fcDestLimSave;
	FC     fcPosSave;
	CP     cpMac, cpLastCorrect;
	int    cch;
	uns    iprc;
	int    docT;
	int    FGtL();
	FC     fcPos;
	uns    ised, isedMac, isedExcerpt, isedExcerptMac;
	uns    ipcd, ipcdMac, ipcdExcerpt, ipcdExcerptMac;
	int    fSkip;
	int    fMoveSectPrm = fTrue;
	CP     cpMinPiece;
	FC     cfc, fcDestFirst;
	int    cchSepx;
	int    ww;
	struct SELS *psels;
	CP HUGE *hprgcp;
	struct PCD *ppcdExcerpt;
	struct SED sed, *psedExcerpt;
	struct PLC **hplcsed;
	struct PRC  *pprc;
	struct DOD   *pdod;
	struct FCB   *pfcb;
	struct WSS   wss;
	char   rgb[1];
	struct PRM   *pprm;

	fnDest = vpqsib->fnDest;
	docSave = vpqsib->docSave;

	Assert (cHpFreeze==0);
	FreezeHp();
	pdod = PdodDoc(docSave);
	pfcb = PfcbFn(fnDest);
	cpMac = CpMacDoc(docSave);
	/* because we put out an extra eop when we have to write header and
		footer docs during a complete save and we don't put out the hidden
		section at the end of a doc in any case, we have to record a rgcp[iMac]
		for each PLC that reflects the situation in the file instead of what's
		recorded in the in-core copy of the PLCs */
	cpLastCorrect = (vpqsib->fExtraPiece) ?
			(vpqsib->fcMac - vpqsib->fcMin) : cpMac;

	/*  this must be the first table saved.  Style sheet code depends on
		it! */
	QuicksavePlc(pdod->hplcfrd, &vpqsib->fcPlcffndRef,
			&vpqsib->cbPlcffndRef, &vfcDestLim, cpLastCorrect);

	if (FFileWriteError())
		goto LReturnFail;

	QuicksavePlc(((docT = pdod->docFtn) != docNil) ?
			PdodDoc(docT)->hplcfnd : 0,
			&vpqsib->fcPlcffndTxt, &vpqsib->cbPlcffndTxt,
			&vfcDestLim, cpNil);
	if (FFileWriteError())
		goto LReturnFail;

	QuicksavePlc((docT != docNil) ? PdodDoc(docT)->hplcpgd : 0,
			&vpqsib->fcPlcfpgdFtn, &vpqsib->cbPlcfpgdFtn,
			&vfcDestLim, cpNil);
	if (FFileWriteError())
		goto LReturnFail;


#ifdef WIN
	QuicksavePlc(pdod->hplcatrd, &vpqsib->fcPlcfandRef,
			&vpqsib->cbPlcfandRef, &vfcDestLim, cpLastCorrect);

	if (FFileWriteError())
		goto LReturnFail;

	QuicksavePlc(((docT = pdod->docAtn) != docNil) ?
			PdodDoc(docT)->hplcfnd : 0,
			&vpqsib->fcPlcfandTxt, &vpqsib->cbPlcfandTxt,
			&vfcDestLim, cpNil);

	if (FFileWriteError())
		goto LReturnFail;
#endif /* WIN */

	MeltHp();

	if (pdod->hplcsed == hNil)
		{
		vpqsib->fcPlcfsed = vfcDestLim;
		vpqsib->cbPlcfsed = 0;
		}
	else
		{
		hplcsed = pdod->hplcsed;
		isedMac = IMacPlc(hplcsed) - 1;
/* if the last section describes nothing but the hidden section mark, we
	will not write it. */
		if (isedMac > 1 && CpPlc(hplcsed, isedMac - 1) == cpMac)
			{
			isedMac--;
			fMoveSectPrm = fFalse;
			}

		SetFnPos(fnDest, vpqsib->fcPlcfsed = vfcDestLim);
		if ((*hplcsed)->fExternal)
			{
			Mac( LockHq((*hplcsed)->hqplce) );
			hprgcp = (CP HUGE *)HpOfHq((*hplcsed)->hqplce);
			}
		else
			hprgcp = ((CP HUGE *)(*hplcsed)->rgcp);
		WriteHprgchToFn(fnDest, hprgcp,
				vpqsib->cbPlcfsed = (isedMac + 1) * sizeof(CP));
#ifdef MAC
		if ((*hplcsed)->fExternal)
			UnlockHq((*hplcsed)->hqplce);
#endif

		fcPosSave = (PfcbFn(fnDest))->fcPos;
		SetFnPos(fnDest, vpqsib->fcPlcfsed + (isedMac * sizeof(CP)));
		WriteRgchToFn(fnDest, &cpLastCorrect, sizeof(CP));

		SetFnPos(fnDest, fcPosSave);
		fcDestLimSave = vfcDestLim;

		vfcDestLim = vfcSepxDestLim;
		vfRoomInPrevExt = vfSepxRoomInPrevExt;
		vfcPrevExtLim = vfcSepxPrevExtLim;
		vfcPrevExtPageLim = vfcSepxPrevExtPageLim;

		for (ised = 0; ised < isedMac;)
			{
#define isedExcerptMax 20
			struct SED rgsedExcerpt[isedExcerptMax];
			char rgchSepx[cchSepxMax];
			struct SEP sepDefault;

			isedExcerptMac = min(isedExcerptMax, isedMac - ised);
			for (isedExcerpt = 0, psedExcerpt = &rgsedExcerpt[0];
					isedExcerpt < isedExcerptMac;
					isedExcerpt++, ised++, psedExcerpt++)
				{
				GetPlc(hplcsed, ised, psedExcerpt);
				if (psedExcerpt->fn != fnDest)
					{
					cpMinPiece =  CpPlc(hplcsed, ised);
					CacheSect(docSave, cpMinPiece);
					StandardSep(&sepDefault);
					cchSepx = CbGrpprlProp(fFalse, rgchSepx+1, cchSepxMax - 1,
							&vsepFetch, &sepDefault, cwSEP - 1,
							mpiwspxSep, (!vpqsib->fWord3 ? 0 : mpiwspxSepEquivW3));
					cfc = cchSepx > 0 ?
							cchSepx + 1 : fc0;
					FcDestFromCpSrc(cpMinPiece, docSave, cfc, &fcDestFirst, witSepx, &fSkip);
					if (FFileWriteError())
						goto LReturnFail;
					Assert(cfc != fc0 || fcDestFirst == fcNil);
					psedExcerpt->fn = fnDest;
					psedExcerpt->fcSepx = fcDestFirst;
					if (vpqsib->fCompleteSave)
						psedExcerpt->fUnk = fFalse;
					}
				}
			WriteRgchToFn(fnDest, rgsedExcerpt, cbSED * isedExcerptMac);
			if (FFileWriteError())
				goto LReturnFail;
			}

		vfcDestLim = fcDestLimSave + (vpqsib->cbPlcfsed += (uns)cbSED * isedMac);
		}

	FreezeHp();
	pdod = PdodDoc(docSave);
	pfcb = PfcbFn(fnDest);
#ifdef MAC
	if (vpqsib->fWord3)
		{
		vpqsib->fcPlcfpgd = vfcDestLim;
		vpqsib->cbPlcfpgd = 0;
		}
	else
#endif
			{
			QuicksavePlc(pdod->hplcpgd, &vpqsib->fcPlcfpgd, &vpqsib->cbPlcfpgd,
					&vfcDestLim,  cpLastCorrect);
			if (FFileWriteError())
				goto LReturnFail;
			}

	if (FFileWriteError())
		goto LReturnFail;

	Mac (if (!vpqsib->fWord3))
		{
		if (!vpqsib->fCompleteSave)
			{
			QuicksavePlc(pdod->hplcphe, &vpqsib->fcPlcfphe,
					&vpqsib->cbPlcfphe, &vfcDestLim, cpLastCorrect);
			if (FFileWriteError())
				goto LReturnFail;
			}
		else
			{
			vpqsib->fcPlcfphe = vfcDestLim;
			vpqsib->cbPlcfphe = 0;
			}
		}

#ifdef WIN
	if (pdod->fSea)
		QuicksavePlc(pdod->hplcsea, &vpqsib->fcPlcfsea, &vpqsib->cbPlcfsea,
				&vfcDestLim, cpLastCorrect);
	else
		{
		vpqsib->fcPlcfsea = vfcDestLim;
		vpqsib->cbPlcfsea = 0;
		}
#endif /* WIN */
	vpqsib->fcSttbfglsy = vpqsib->fcPlcfglsy = vfcDestLim;
	vpqsib->cbSttbfglsy = vpqsib->cbPlcfglsy = 0;
#ifdef MAC
#ifdef DEBUG
	if (!pdod->fSea && (pdod->hsttbGlsy || pdod->hplcglsy))
		Assert (pdod->hsttbGlsy && pdod->hplcglsy);
#endif /* DEBUG */
	if (pdod->hsttbGlsy && !pdod->fSea)
#endif /* MAC */
#ifdef WIN
		if (pdod->fGlsy)
#endif /* WIN */
			{
#ifdef WIN
#ifdef DEBUG
			if (pdod->hsttbGlsy || pdod->hplcglsy)
				Assert (pdod->hsttbGlsy && pdod->hplcglsy);
#endif /* DEBUG */
			if (pdod->hsttbGlsy)
#endif /* WIN */
				{
				vpqsib->cbSttbfglsy = CbSttbf(*pdod->hsttbGlsy);
				WriteSttbToFile(pdod->hsttbGlsy, fnDest, &vfcDestLim);
				Assert(vfcDestLim == vpqsib->fcSttbfglsy + vpqsib->cbSttbfglsy);
				}
			QuicksavePlc(pdod->hplcglsy, &vpqsib->fcPlcfglsy, &vpqsib->cbPlcfglsy,
					&vfcDestLim, cpLastCorrect);
			if (FFileWriteError())
				goto LReturnFail;
			}

	QuicksavePlc(((docHdr = pdod->docHdr) != 0) ?
			(*PdodDoc(docHdr)).hplchdd : 0,
			&vpqsib->fcPlcfhdd, &vpqsib->cbPlcfhdd,
			&vfcDestLim, cpNil);
	if (FFileWriteError())
		goto LReturnFail;

#ifdef MAC
	QuicksavePlc(pfcb->hplcbteChp, &vpqsib->fcPlcfbteChpx, &vpqsib->cbPlcfbteChpx,
			&vfcDestLim, cpNil);
	if (FFileWriteError())
		goto LReturnFail;

	QuicksavePlc(pfcb->hplcbtePap, &vpqsib->fcPlcfbtePapx, &vpqsib->cbPlcfbtePapx,
			&vfcDestLim, cpNil);
	if (FFileWriteError())
		goto LReturnFail;
	if (pdod->fSea)
		QuicksavePlc(pdod->hplcsea, &vpqsib->fcPlcfsea, &vpqsib->cbPlcfsea,
				&vfcDestLim, cpLastCorrect);
	else
		{
		vpqsib->fcPlcfsea = vfcDestLim;
		vpqsib->cbPlcfsea = 0;
		}

	vpqsib->fcRgftc = vfcDestLim;
	vpqsib->cbRgftc = viftcMac * sizeof(int);
	SetFnPos(fnDest, vfcDestLim);
	WriteRgchToFn(fnDest, vrgftc, vpqsib->cbRgftc);
	vfcDestLim += vpqsib->cbRgftc;
	if (FFileWriteError())
		goto LReturnFail;

	vpqsib->fcPrr = vfcDestLim;
	if (pdod->qqprr == 0)
		vpqsib->cbPrr = 0;
	else
		{
		PRR prr;
		vpqsib->cbPrr = sizeof(PRR);
		SetFnPos(fnDest, vfcDestLim);
		bltbx(*pdod->qqprr, (char far *) &prr, sizeof(PRR));
		WriteRgchToFn(fnDest, &prr, sizeof(PRR));
		vfcDestLim += sizeof(PRR);
		}


	/*  write out the DOP */
	vpqsib->fcDop = vfcDestLim;
	vpqsib->cbDop = cbDOP;

	SetFnPos (fnDest, vfcDestLim);
	WriteRgchToFn (fnDest, &pdod->dop, sizeof (struct DOP));
	vfcDestLim += cbDOP;

	/* write out sttbFlc */
	vpqsib->fcSttbFlc = vfcDestLim;
	if (pdod->hsttbFlc == hNil)
		vpqsib->cbSttbFlc = 0;
	else
		{
		WriteSttbToFile(pdod->hsttbFlc, fnDest, &vfcDestLim);
		vpqsib->cbSttbFlc = vfcDestLim - vpqsib->fcSttbFlc;
		}

#endif /* MAC */

#ifdef WIN
	if (!vpqsib->fFirstOfTwo)
		{
		QuicksavePlc(pfcb->hplcbteChp, &vpqsib->fcPlcfbteChpx, &vpqsib->cbPlcfbteChpx,
				&vfcDestLim, cpNil);

		QuicksavePlc(pfcb->hplcbtePap, &vpqsib->fcPlcfbtePapx, &vpqsib->cbPlcfbtePapx,
				&vfcDestLim, cpNil);

		}
	else
		/*  writing the first "file" of a pair of files.  Postpone
			writing until we write the second "file" */
		{
		vpqsib->fcPlcfbteChpx = vpqsib->fcPlcfbtePapx = vfcDestLim;
		vpqsib->cbPlcfbteChpx = vpqsib->cbPlcfbtePapx = 0;
		}


/* FUTURE: it would be nice to compress out unreferenced fonts. (bl) */
	vpqsib->cbSttbfffn = 0;
	vpqsib->fcSttbfffn = vfcDestLim;
	if (pdod->ftcMac > 0)
		{
		vpqsib->cbSttbfffn = CbWriteSttbFontToFile( docSave,
				fnDest, &vfcDestLim );
		}

	/*  save Fields */
	QuicksavePlc(pdod->hplcfld, &vpqsib->fcPlcffldMom,
			&vpqsib->cbPlcffldMom, &vfcDestLim, cpLastCorrect);

	QuicksavePlc(((docT = pdod->docHdr) != 0) ?
			(*PdodDoc(docT)).hplcfld : 0,
			&vpqsib->fcPlcffldHdr, &vpqsib->cbPlcffldHdr,
			&vfcDestLim, cpNil);

	if (FFileWriteError())
		goto LReturnFail;

	QuicksavePlc(((docT = pdod->docFtn) != 0) ?
			(*PdodDoc(docT)).hplcfld : 0,
			&vpqsib->fcPlcffldFtn, &vpqsib->cbPlcffldFtn,
			&vfcDestLim, cpNil);

	QuicksavePlc(((docT = pdod->docAtn) != 0) ?
			(*PdodDoc(docT)).hplcfld : 0,
			&vpqsib->fcPlcffldAtn, &vpqsib->cbPlcffldAtn,
			&vfcDestLim, cpNil);

	if (FFileWriteError())
		goto LReturnFail;

	QuicksavePlc((pdod->fDot && (docT = pdod->docMcr) != 0) ?
			(*PdodDoc(docT)).hplcfld : 0,
			&vpqsib->fcPlcffldMcr, &vpqsib->cbPlcffldMcr,
			&vfcDestLim, cpNil);

	if (FFileWriteError())
		goto LReturnFail;

	/*  save Bookmarks */
	vpqsib->fcSttbfbkmk = vfcDestLim;
	if (pdod->hsttbBkmk)
		{
		vpqsib->cbSttbfbkmk = CbSttbf(*pdod->hsttbBkmk);
		WriteSttbToFile(pdod->hsttbBkmk, fnDest, &vfcDestLim);
		}
	else
		vpqsib->cbSttbfbkmk = 0;
	Assert(vfcDestLim == vpqsib->fcSttbfbkmk + vpqsib->cbSttbfbkmk);

	QuicksavePlc(pdod->hplcbkf, &vpqsib->fcPlcfbkf, &vpqsib->cbPlcfbkf,
			&vfcDestLim, cpNil);

	QuicksavePlc(pdod->hplcbkl, &vpqsib->fcPlcfbkl, &vpqsib->cbPlcfbkl,
			&vfcDestLim, cpNil);


/* Menu and Keyboard Stuff */

	vpqsib->fcCmds = vfcDestLim;
	if (pdod->fDot)
		{
		MeltHp();
		QuicksaveCommands();
		vfcDestLim += vpqsib->cbCmds;
		FreezeHp ();
		pdod = PdodDoc(docSave); /* reload pointer */
		}
	else
		vpqsib->cbCmds = 0;

/****/


/* Macro (Opel) Stuff */

	vpqsib->fcSttbfmcr = vpqsib->fcPlcmcr = vfcDestLim;
	vpqsib->cbSttbfmcr = vpqsib->cbPlcmcr = 0;

	if (pdod->fDot)
		QuicksaveMacros();
/****/

	/*  write out print environment.  the format is:
			_____________________________________________
		| szPrinter | szPort | szDriver | Environment |
			---------------------------------------------
	*/
	vpqsib->fcPrEnv = vfcDestLim;
	vpqsib->cbPrEnv = 0;
	if (!pdod->fEnvDirty && (vpri.hprenv != hNil) &&
			(vpri.hszPrinter != hNil) && (vpri.hszPrPort != hNil) &&
			(vpri.hszPrDriver != hNil))
		{
		int cb, ihsz;
		char **rghsz[3];
		char *pch;

		rghsz[0] = vpri.hszPrinter;
		rghsz[1] = vpri.hszPrPort;
		rghsz[2] = vpri.hszPrDriver;
		for (ihsz = 0; ihsz < 3; ihsz++)
			{
			pch = *rghsz[ihsz];
			cb = CchSz(pch);
			SetFnPos(fnDest, vfcDestLim);
			WriteRgchToFn(fnDest, pch, cb);
			vfcDestLim += cb;
			vpqsib->cbPrEnv += cb;
			}

		cb = CbOfH(vpri.hprenv);
		SetFnPos(fnDest, vfcDestLim);
		WriteRgchToFn(fnDest, (char *) *vpri.hprenv, cb);
		vfcDestLim += cb;
		vpqsib->cbPrEnv += cb;
		}
	Assert(vfcDestLim == vpqsib->fcPrEnv + vpqsib->cbPrEnv);
#endif /* WIN */

	/* write out window save state */
	vpqsib->fcWss = vfcDestLim;

/* if selCur points to the doc we're saving we will store selCur state. */
	if (selCur.doc == docSave)
		{
		psels = &selCur;
		ww = selCur.ww;
		}
/* else we will store the state of the first window we find for docSave */
	else
		{
		if ((ww = WwDisp(docSave, wwNil, fTrue)) != wwNil)
			psels = &PwwdWw(ww)->sels;
		}
	if (ww != wwNil)
		{
		wss.sels = *psels;
#ifdef MAC
		PwwdSetWw(ww, cptSame);
		GetWindRect(&wss.rcWindow);
		LocalToGlobalRect(&wss.rcWindow);
		wss.rcMwd = PmwdWw(ww)->rc;
#endif /* MAC */
		SetFnPos (fnDest, vfcDestLim);
		WriteRgchToFn (fnDest, &wss, vpqsib->cbWss = cbWSS);
		vfcDestLim += cbWSS;
		}
	else
		vpqsib->cbWss = 0;

	vpqsib->fcClx = vfcDestLim;
	vpqsib->cbClx = 0;

#ifdef WIN
	vpqsib->fcSpare1 = vfcDestLim;
	vpqsib->cbSpare1 = 0;

	vpqsib->fcSpare2 = vfcDestLim;
	vpqsib->cbSpare2 = 0;

	vpqsib->fcSpare3 = vfcDestLim;
	vpqsib->cbSpare3 = 0;

	vpqsib->wSpare4 = 0;
#endif /* WIN */

	MeltHp();

	if (vpqsib->fCompleteSave)
		goto LClxComplete;

	hplcpcd = (PdodDoc(docSave))->hplcpcd;
	ipcdMac = IMacPlc(hplcpcd);

	Assert((*hplcpcd)->icpAdjust > ipcdMac || (*hplcpcd)->dcpAdjust == cp0);
/* last piece should have been invisible to all editing and can be discarded*/
	if (ipcdMac > 1 && CpPlc(hplcpcd, ipcdMac - 1) >= cpMac)
		ipcdMac--;
/* if the next to last piece describes nothing but the hidden section mark, we
	will not write it. */
	if (ipcdMac > 1 && CpPlc(hplcpcd, ipcdMac - 1) >= cpMac)
		{
		ipcdMac--;
/* if the hidden section piece at document end is in a piece by itself, and
	the hidden section piece is not a section by itself, we need to take any
	sprms applied to the hidden section piece (guaranteed to be section sprms)
	and merge them with the sprms of the immediately preceding piece. */
		if (fMoveSectPrm)
			{
			char *pgrpprl;
			int cb;
			char grpprl[2];
			struct PCD pcd, pcd2;
			int prm;

			GetPlc(hplcpcd, ipcdMac, &pcd);
			pgrpprl = PgrpprlFromPrm(pcd.prm, &cb, grpprl);
			GetPlc(hplcpcd, ipcdMac - 1, &pcd2);
			prm = PrmAppend(pcd2.prm, pgrpprl, cb);
			pcd2.prm = prm;
			PutPlc(hplcpcd, ipcdMac - 1, &pcd2);
			}
		}
	fcDestFurthest = (FC) 0;

	for (ipcd = 0; ipcd < ipcdMac; ipcd++)
		{
/* FcDestIprcFromCfgrPrc causes heap movement */
		GetPlc(hplcpcd, ipcd, &pcd);

		pprm = (struct PRM *) &pcd.prm;
		if (fnDest == pcd.fn && pprm->fComplex)
			{
			FcDestIprcFromCfgrPrc(&fcDest, &iprc, pprm->cfgrPrc);
			if (vmerr.fMemFail)
				goto LReturnFail;

			if (fcDest > fcDestFurthest)
				{
				fcDestFurthest = fcDest;

/* FcDestIprcFromCfgrPrc caused heap movement */
				pprc = *HprcFromPprmComplex(pprm);

				rgb[0] = clxtPrc;
				SetFnPos(fnDest, fcDest);
				WriteRgchToFn(fnDest, &rgb[0], 1);
				WriteRgchToFn(fnDest, &pprc->bprlMac, 2);
				WriteRgchToFn(fnDest, &pprc->grpprl,
						pprc->bprlMac);

				if (FFileWriteError())
					goto LReturnFail;
				vfcDestLim += pprc->bprlMac + 3;
				}
			}
		}

	vpqsib->fcPlcfpcd = vfcDestLim;
	vpqsib->cbPlcfpcd = (ipcdMac * (sizeof(CP) + cbPCD)) + sizeof(CP) + 3;
	vfcDestLim += vpqsib->cbPlcfpcd;
	vpqsib->cbClx = vfcDestLim - vpqsib->fcClx;

	SetFnPos(fnDest, vpqsib->fcPlcfpcd);
	rgb[0] = clxtPlcpcd;
	WriteRgchToFn(fnDest, &rgb[0], 1);
	cch = vpqsib->cbPlcfpcd - 3;
	WriteRgchToFn(fnDest, &cch, 2);
	if ((*hplcpcd)->fExternal)
		{
		hprgcp = (CP HUGE *)HpOfHq((*hplcpcd)->hqplce);
		Mac( LockHq((*hplcpcd)->hqplce) );
		}
	else
		hprgcp = ((CP HUGE *)(*hplcpcd)->rgcp);
	WriteHprgchToFn(fnDest, hprgcp, (ipcdMac + 1) * sizeof(CP));
#ifdef MAC
	if ((*hplcpcd)->fExternal)
		UnlockHq((*hplcpcd)->hqplce);
#endif
	if (FFileWriteError())
		goto LReturnFail;
	fcDestLimSave = vfcDestLim;

	/* set beginning global contexts */
	vfcDestLim = vpqsib->fcBegText;
	vfRoomInPrevExt = vpqsib->fRoomInPrevExt;
	vfcPrevExtLim = vpqsib->fcPrevExtLim;
	vfcPrevExtPageLim = vpqsib->fcPrevExtPageLim;

	for (ipcd = 0; ipcd < ipcdMac;)
		{
#define ipcdExcerptMax 20
		struct PCD rgpcdExcerpt[ipcdExcerptMax];
		ipcdExcerptMac = min(ipcdExcerptMax, ipcdMac - ipcd);
		for (ipcdExcerpt = 0, ppcdExcerpt = &rgpcdExcerpt[0];
				ipcdExcerpt < ipcdExcerptMac;
				ipcdExcerpt++, ipcd++, ppcdExcerpt++)
			{
			GetPlc(hplcpcd, ipcd, ppcdExcerpt);
			if (ppcdExcerpt->fn != fnDest)
				{
				cfc = CpPlc(hplcpcd, ipcd + 1) -
						(cpMinPiece = CpPlc(hplcpcd, ipcd));
				FcDestFromCpSrc(cpMinPiece, docSave, cfc, &fcDestFirst,
						witPieceText, &fSkip);
				if (FFileWriteError())
					goto LReturnFail;
				ppcdExcerpt->prm = 0;
				ppcdExcerpt->fn = fnDest;
				ppcdExcerpt->fc = fcDestFirst;
				}
			else  if (((struct PRM *)&ppcdExcerpt->prm)->fComplex)
				{
				FcDestIprcFromCfgrPrc(&fcDest, &iprc, ((struct PRM *)&ppcdExcerpt->prm)->cfgrPrc);
				if (vmerr.fMemFail)
					goto LReturnFail;
				((struct PRM *)&ppcdExcerpt->prm)->cfgrPrc = iprc;
				}
			}
		WriteRgchToFn(fnDest, rgpcdExcerpt, cbPCD * ipcdExcerptMac);
		if (FFileWriteError())
			goto LReturnFail;
		}
	vfcDestLim = fcDestLimSave;

LClxComplete:
#ifdef WIN
	/* summary info is saved last so that it can be modified without
		reading the whole file in!
	*/

	FreezeHp();
	pdod = PdodDoc(docSave);

	/*  write out the DOP */
	vpqsib->fcDop = vfcDestLim;
	vpqsib->cbDop = cbDOP;
	SetFnPos (fnDest, vfcDestLim);
	WriteRgchToFn (fnDest, &pdod->dop, sizeof (struct DOP));
	vfcDestLim += cbDOP;
	if (FFileWriteError())
		goto LReturnFail;

	/*  write out associated strings */
	vpqsib->fcSttbfAssoc = vfcDestLim;
	if (pdod->hsttbAssoc)
		{
		vpqsib->cbSttbfAssoc = CbSttbf(*pdod->hsttbAssoc);
		WriteSttbToFile(pdod->hsttbAssoc, fnDest, &vfcDestLim);
		}
	else
		vpqsib->cbSttbfAssoc = 0;
	MeltHp();
	Assert(vfcDestLim == vpqsib->fcSttbfAssoc + vpqsib->cbSttbfAssoc);
	if (FFileWriteError())
		goto LReturnFail;
#endif /* WIN */

	return(fTrue);

LReturnFail:
	MeltHpToZero();
	return(fFalse);

}       /* end FSaveTbls */


/* %%Function:QuicksavePlc %%Owner:davidlu */
QuicksavePlc(hplc, pfc, pcb, pfcDestLim, cpCorrect)
struct PLC **hplc;
FC      *pfc;
uns     *pcb;
FC      *pfcDestLim;
CP      cpCorrect;
{
	int     fnDest;
	FC      fc;
	FC      fcPosSave;
	uns     iMac;
	struct PLC *pplc;
	uns     cbRgcp;
	uns     cbRgfoo;
	CP  HUGE *hprgcp;
	char HUGE *hpchFoo;

	fc = *pfc = *pfcDestLim;
	*pcb = 0;
	if (hplc !=  hNil)
		{
		CompletelyAdjustHplcCps(hplc);
		iMac = IMacPlc(hplc);
/* if the last plc entry begins at or past cpCorrect, it describes no part of
	the visible portion of the document and should not be written */
		if (cpCorrect != cpNil && iMac > 0 &&
				CpPlc(hplc, iMac - 1) >= cpCorrect)
			iMac--;
		SetFnPos(fnDest = vpqsib->fnDest, fc);

		pplc = *hplc;
		if (pplc->fExternal)
			{
			hprgcp = (CP HUGE *)HpOfHq(pplc->hqplce);
			Mac( LockHq(pplc->hqplce); )
			}
		else
			hprgcp = ((CP HUGE *)pplc->rgcp);

		/* write rgcp */
		cbRgcp = (iMac + 1) * sizeof(CP);
		WriteHprgchToFn(fnDest, hprgcp, cbRgcp);

		/* write rgfoo */
		hpchFoo = &hprgcp[pplc->iMax];
		cbRgfoo = pplc->cb * iMac;
		WriteHprgchToFn(fnDest, hpchFoo, cbRgfoo);
		*pcb = cbRgcp + cbRgfoo;
#ifdef MAC
		if (pplc->fExternal)
			UnlockHq(pplc->hqplce);
#endif

		if (cpCorrect != cpNil)
			{
			fcPosSave = (PfcbFn(fnDest))->fcPos;
			SetFnPos(fnDest, *pfc + iMac * sizeof(CP));
			WriteRgchToFn(fnDest, &cpCorrect, sizeof(CP));
			SetFnPos(fnDest, fcPosSave);
			}
		}
	*pfcDestLim += *pcb;
}


#ifdef NOTUSED
/* %%Function:FGtL %%Owner:NOTUSED */
FGtL(l1, l2)
long l1, l2;
{
	return (l1 > l2);
}


#endif



/* %%Function:FcDestIprcFromCfgrPrc %%Owner:davidlu */
FcDestIprcFromCfgrPrc(pfcDest, piprc, cfgrPrc)
FC      *pfcDest;
int     *piprc;
int     cfgrPrc;
{
	int     ipeq;
	struct PRC     *pprc;
	struct PEQ     *ppeq;
	struct PEQ     peq;

/* NOTE: routine can fail.  caller must check fMemFail upon return! */

#ifdef MAC
	ipeq = IInPlcQuick(vhplcpeq, (long) cfgrPrc);
#else /* WIN */
	ipeq = IInPlcCheck(vhplcpeq, (long) cfgrPrc);
#endif /* WIN */
	if (ipeq >= 0 && Mac (ipeq < IMacPlc(vhplcpeq))
			cfgrPrc == (int) CpPlc(vhplcpeq, ipeq))
		{
		GetPlc(vhplcpeq, ipeq, &peq);
		*pfcDest = peq.fcDest;
		*piprc = peq.iprc;
		}
	else
		{
		*pfcDest = vfcDestLim;
		*piprc = vpqsib->iprc++;
#ifdef MAC
		pprc = (struct PRC *) *((int **) pwMax - cfgrPrc);
#else /* WIN */
		pprc = *((struct PRC **)(cfgrPrc<<2));
#endif /* MAC */
		peq.fcDest = *pfcDest;
		peq.iprc = *piprc;
/* caller must check fMemFail. */
		if (!FInsertInPlc(vhplcpeq, ++ipeq, (long) cfgrPrc, &peq))
			SetErrorMat(matLow);
		}
}


/* %%Function:UpdateFib %%Owner:davidlu */
UpdateFib(pdsr, pmsr)
struct DSR *pdsr;
struct MSR *pmsr;

{
	int	fnDest;
	struct FIB  fib;
	struct DOD *pdod;

	fnDest = vpqsib->fnDest;

	if (vpqsib->fCompleteSave)
		{
		/*  generate all new fib */
		SetBytes (&fib, 0, cbFIB);
		fib.fComplex = fFalse;
		fib.wIdent = wMagic;
		fib.nFib = nFibCurrent;
		fib.nFibBack = nFibBackCurrent;
		fib.nProduct = nProductCurrent;
		fib.nLocale = nLocaleDef;
		fib.fcMin = vpqsib->fcMin;
		fib.fcMac = vpqsib->fcMac;
		fib.fcStshfOrig = vpqsib->fcStshf;
		fib.cbStshfOrig = vpqsib->cbStshf;
		}
	else
		{
		/*  read in existing fib */
		FetchFib(fnDest, &fib, vpqsib->pnFib);
		fib.fComplex = fTrue;
		Assert(fib.cQuickSaves < 15); /* only 4 bits */
		fib.cQuickSaves++;
/* since the first FKP written during a fast save is written at the cbSector 
   boundary following the STSH, we reduce the cbStshOrig when the style sheet
   size is reduced so that fcStshOrig + cbStshOrig won't extend into an FKP */
		Assert(fib.fcStshfOrig % cbSector == 0); 
		if (fib.fcStshfOrig == vpqsib->fcStshf && fib.cbStshfOrig > cbSector)
			fib.cbStshfOrig = umin(fib.cbStshfOrig, vpqsib->cbStshf);
		}
	pdod = PdodDoc (pdsr->doc);
/* by time we reach this point, dod.fMayHavePic is only true if the doc
	actually contains a picture. */
	fib.fHasPic = pdod->fMayHavePic;
	fib.cbMac = vfcDestLim;
	fib.ccpText = pdsr->ccpText;
	fib.ccpFtn = pdsr->ccpFtn;
	fib.ccpHdd = pdsr->ccpHdr;
#ifdef WIN
	fib.ccpMcr = pdsr->ccpMcr;
	fib.ccpAtn = pdsr->ccpAtn;

	fib.fGlsy = pdod->fGlsy;
	fib.fDot = pdod->fDot;
	fib.pnNext = pn0;
#endif /* WIN */

	/* the following code sets all of the fc, cb pairs in the FIB.
		It depends on fcStshf thru wDummy in the QSIB being in 1-1
		correspondence with fcStshf thru end of fc,cb pairs in the FIB. */

	bltb(&vpqsib->fcStshf, &fib.fcStshf, 
			offset(QSIB, wDummy) - offset(QSIB, fcStshf));

#ifdef WIN
	if (!vpqsib->fFirstOfTwo)
		{
		/* Now update FIB at pnFib (current fib) */
		WriteFib(fnDest, &fib, vpqsib->pnFib);
		}
	else
		{
		/*  don't know bin table info yet, write later. */
		Assert (pmsr != NULL);
		bltb (&fib, &pmsr->fibFirst, cbFIB);
		pmsr->pnFibSecond = PnWhoseFcGEFc (vfcDestLim);
		}

	if (vpqsib->fWritingSecond)
		{
		/* Now update & write the first file's fib */
		Assert (pmsr != NULL);
		pmsr->fibFirst.fcPlcfbteChpx = vpqsib->fcPlcfbteChpx;
		pmsr->fibFirst.cbPlcfbteChpx = vpqsib->cbPlcfbteChpx;
		pmsr->fibFirst.fcPlcfbtePapx = vpqsib->fcPlcfbtePapx;
		pmsr->fibFirst.cbPlcfbtePapx = vpqsib->cbPlcfbtePapx;
		pmsr->fibFirst.cpnBteChp += vpqsib->cpnBteChp;
		pmsr->fibFirst.cpnBtePap += vpqsib->cpnBtePap;
		pmsr->fibFirst.pnNext = vpqsib->pnFib;
		pmsr->fibFirst.cbMac = fib.cbMac;

		WriteFib(fnDest, &pmsr->fibFirst, pn0);

		/*  indicate fn now compound */
		PfcbFn(fnDest)->fCompound = fTrue;
		}
	else
		/*  page limit refers to first doc only if compound */
		(PfcbFn(fnDest))->pnXLimit = PnWhoseFcGEFc (fib.fcMac);

#else   /* MAC */

	/* Now update FIB at beginning of file */
	WriteFib(fnDest, &fib, pn0);

	(PfcbFn(fnDest))->pnXLimit = PnWhoseFcGEFc (fib.fcMac);
#endif /* WIN */
	(PfcbFn(fnDest))->nFib = fib.nFib;
}


#ifdef MAC
/* %%Function:UpdateFib30 %%Owner:NOTUSED */
UpdateFib30(ccpText, ccpFtn, ccpHdd)
CP ccpText;
CP ccpFtn;
CP ccpHdd;
{
	int     fnDest;
	char far *qchFib;
	struct FIB30  fib, *pfib;
	int    cbBlock;
	long  dir;
	struct DOD *pdod;

	fnDest = vpqsib->fnDest;

	bltbh(HpchGetPn(fnDest, pn0), &fib, cbFIB30);

	fib.fComplex = fFalse;
	fib.wIdent = wMagic30;
	fib.w1 = fib.w2 = fib.w3 = fib.w4 = 0;
	fib.fcMin = cbSectorPre35 * 2;
	fib.fcMac = vpqsib->fcMac;
	fib.fcStshfOrig = vpqsib->fcStshf;
	fib.cbStshfOrig = vpqsib->cbStshf;
	if (PdodDoc(vpqsib->docSave)->fDefaultFormat)
		fib.fDefaultFormat = fTrue;
	fib.ccpText = ccpText;
	fib.ccpFtn = ccpFtn;
	fib.ccpHdd = ccpHdd;

	fib.fcStshf = vpqsib->fcStshf;
	fib.cbStshf = vpqsib->cbStshf;

	fib.fcPlcffndRef = vpqsib->fcPlcffndRef;
	fib.cbPlcffndRef = vpqsib->cbPlcffndRef;

	fib.fcPlcffndTxt = vpqsib->fcPlcffndTxt;
	fib.cbPlcffndTxt = vpqsib->cbPlcffndTxt;

	fib.fcPlcfsed = vpqsib->fcPlcfsed;
	fib.cbPlcfsed = vpqsib->cbPlcfsed;

	fib.fcPlcfpgd = vpqsib->fcPlcfpgd;
	fib.cbPlcfpgd = vpqsib->cbPlcfpgd;

	fib.fcSttbfglsy = vpqsib->fcSttbfglsy;
	fib.cbSttbfglsy = vpqsib->cbSttbfglsy;

	fib.fcPlcfglsy = vpqsib->fcPlcfglsy;
	fib.cbPlcfglsy = vpqsib->cbPlcfglsy;

	fib.fcPlcfhdd = vpqsib->fcPlcfhdd;
	fib.cbPlcfhdd = vpqsib->cbPlcfhdd;

	fib.fcPlcfbteChpx = vpqsib->fcPlcfbteChpx;
	fib.cbPlcfbteChpx = vpqsib->cbPlcfbteChpx;

	fib.fcPlcfbtePapx = vpqsib->fcPlcfbtePapx;
	fib.cbPlcfbtePapx = vpqsib->cbPlcfbtePapx;

	fib.fcClx = vpqsib->fcClx;
	fib.cbClx = vpqsib->cbClx;

	pdod = PdodDoc(vpqsib->docSave);
	if (pdod->dop.fMirrorMargins || pdod->dop.dxaGutter != 0)
		pdod->dop.fFacingPages = fTrue;
	if (pdod->dop.fpc == fpcEndDoc)
		pdod->dop.fpc = fpcBeneathText;
	bltb(&pdod->dop, &fib.dop, cbDOP30);
	if (!FRecallFlc(flkNext, pdod->hsttbFlc, fib.stFileNext, 0, &dir))
		fib.stFileNext[0] = 0;

	/* Now update FIB at beginning of file */

	bltbh(&fib, HpchGetPn(fnDest, pn0), cbFIB30);
	SetDirty(vibp);
	(PfcbFn(fnDest))->pnXLimit = (PN) fib.fcMac;
}


#endif


/* %%Function:FAdjustPicFc %%Owner:davidlu */
FAdjustPicFc(doc, cpRun, fnDest, cpPicFirst)
int doc;
CP  cpRun;
int fnDest;
CP  cpPicFirst;
{
/* returns fTrue if FetchCp was called */
	FC fcPicDest;
	int fSkip;

	if ((cpRun >= cpPicFirst) && vchpFetch.fSpec)
		{
		Assert(fnFetch != fnDest);

		/* fcmProps so we maintain the limits of the CHP run*/
		if (ChFetch(doc, cpRun, fcmChars+fcmProps) == chPicture)
			{
			vchpFetch.fnPic = fnFetch;
			FetchPe(&vchpFetch, doc, cpRun);
			AssignFcDest((FC) vpicFetch.lcb, &fcPicDest,
					&vfcPicDestLim, &vfPicRoomInPrevExt,
					&vfcPicPrevExtLim, vfcPicPrevExtPageLim,
					&fSkip, fTrue, fFalse);
			vchpFetch.fcPic = fcPicDest;
			vchpFetch.fnPic = fnDest;
			}
		return(fTrue);
		}
	return(fFalse);
}


/* W R I T E  C H P S */
/* %%Function:WriteChps %%Owner:davidlu */
NATIVE WriteChps(doc, fnDest, cpFirst, ccpRun, fcDestFirst, pfkpd, pfkpChpDummy)
int doc;
int fnDest;
CP cpFirst;
CP ccpRun;
FC fcDestFirst;
struct FKPD *pfkpd;
struct FKP *pfkpChpDummy;
{
	int docFetch;
	CP cpRun = cpFirst;
	FC fcDest = fcDestFirst;
	CP cpLim = cpFirst + ccpRun;
	FC fcLim = fcDest + ccpRun;
	CP cpPicFirst = (doc == vpqsib->docSave) ? vpqsib->rgcpPic[0] : cp0;
	int cchChpLast;
	int cchChp;
	int iCount = 0;
	PN pnLast;
	struct CHP chpT;
	struct CHP chpLast;

	pnLast = pfkpd->pn;
	CachePara(doc, cpRun);
	FetchCp(doc, cpRun, fcmProps);
	docFetch = docNil;

	/* change fcPic in vchpFetch if we have picture. */
	if (vchpFetch.fSpec && FAdjustPicFc(doc, cpRun, fnDest, cpPicFirst))
		docFetch = doc;
	cchChpLast = CbGenChpxFromChp(&chpLast, &vchpFetch, &vchpStc, vpqsib->fWord3);

	for (;;)
		{
/* check for heap overflow */
		if (vmerr.fMemFail)
			{
			if (pfkpChpDummy == 0)
				FreePhplc(&(PdodDoc(vpqsib->docSave)->hplcpad));
			}
		/* 33+1/3% to 66+2/3% */
		if ((iCount++ & 15) == 0 && doc == vpqsib->docSave)
			{{ /* !NATIVE - WriteChps */
			CP cpMac = CpMacDoc(vpqsib->docSave);
			CP cpNum = (cpRun + cpMac);
			ReportQuickSavePercent (cp0, cpMac+cpMac+cpMac, cpNum);
			}}
		if ((fcDest = fcDest + vccpFetch) > fcLim)
			fcDest = fcLim;
		cpRun += vccpFetch;

		if (cpLim <= cpRun || FFileWriteError() || 
				(!vpqsib->fCompleteSave && vmerr.fMemFail))
			break;
		CachePara(doc, cpRun);
		FetchCp(docFetch, cpRun, fcmProps);
		docFetch = docNil;      /* assume next is sequential */
		/* if we have a CHP that defines a picture, we must first
			find the position the picture will be given in fnDest. */
		Assert(cpRun == vcpFetch);

		/* change fcPic in vchpFetch if we have picture. */
		if (vchpFetch.fSpec && FAdjustPicFc(doc, cpRun, fnDest, cpPicFirst))
			docFetch = doc; /* not sequential run */
		cchChp = CbGenChpxFromChp(&chpT, &vchpFetch, &vchpStc, vpqsib->fWord3);
		if (cchChp != cchChpLast ||
				FNeRgch(&chpT, &chpLast, cchChp))
			{
			if (!FAddRun(fnDest,
					fcDest,
					&chpLast,
					cchChpLast,
					pfkpd,
					fFalse /* CHP */,
					fFalse /* use fcPos for new */,
					!vpqsib->fCompleteSave /* during fast save plcbte must expand, 
			during full save plcbte need not expand */,
					vpqsib->fWord3))
				{
				SetErrorMat( matLow );
				return;
				}
			bltb(&chpT, &chpLast, cchChp);
			cchChpLast = cchChp;
#ifdef MAC
			if (pfkpd->pn != pnLast && pfkpd->fPlcIncomplete &&
					FCheckTooManyBtes(PfcbFn(fnDest)->hplcbteChp, pfkpd))
				{
				SetErrorMat(matLow);
				return;
				}
			pnLast = pfkpd->pn;
#endif /* MAC */
			}

		}
	/* write the last run  */
	if (!FFileWriteError())
		if (!FAddRun(fnDest,
				fcDest,
				&chpLast,
				cchChpLast,
				pfkpd,
				fFalse /* CHP */,
				fFalse /* use fcPos for new */,
				!vpqsib->fCompleteSave /* during fast save plcbte must expand, 
		during full save plcbte need not expand */,
				vpqsib->fWord3))
			SetErrorMat( matLow );
}


#ifdef MAC
/* F  C H E C K  T O O  M A N Y  B T E S  */
/* checks to see if we have recorded so many FKPs that it may be impossible
	to reopen the saved file. if this situation is detected, the user is 
	asked whether the save should continue. returns fTrue if user decides to
	abort save, fFalse otherwise. */
/* %%Function:FCheckTooManyBtes %%Owner:NOTUSED */
FCheckTooManyBtes(hplcbte, pfkpd)
struct PLC **hplcbte;
struct FKPD *pfkpd;
{
	PN pnFirst;
	long lcb;
	int ipn;

	Assert(vpqsib->fCompleteSave);
/* if user has already decided to continue save, there's no need to continue
	checking */
	if (vpqsib->fBteNoLim)
		return (fFalse);
	GetPlc(hplcbte, 0, &pnFirst);
	ipn = pfkpd->pn - pnFirst + 1;
	lcb = ipn * (sizeof(CP) + sizeof(PN)) + sizeof(CP);
	if (lcb > vpqsib->lcbAvailBte)
		{
		if (!FAlertYesNo(SzFrameKey("After save, this document may be too large to reopen on this system. Abort saving?",
				 DiskTooLargeReopen)))
			vpqsib->fBteNoLim = fTrue;
		else
			return (fTrue);
		}
	return (fFalse);
}


#endif /* MAC */

/* %%Function:WritePaps %%Owner:davidlu */
NATIVE WritePaps(doc, fnDest, cpFirst, ccpRun, fcDestFirst, pfkpd, pfkpPapDummy)
int doc;
int fnDest;
CP cpFirst;
CP ccpRun;
FC fcDestFirst;
struct FKPD *pfkpd;
struct FKP *pfkpPapDummy;
{
	CP cpRun = cpFirst;
	CP cpLim = cpRun + ccpRun;
	int cchPapx;
	int ipad;
	int stc;
	int iCount = 0;
	int fWord3 = vpqsib->fWord3;
	CP cpFirstNext;
	PN pnLast;
	struct PAD **hplcpad;
	struct DOD *pdod;
	struct PHE phe;
	struct PAP papStd;
	char papx[cbSector];
	struct TAP tapStd;

	CachePara(doc, cpRun);
	cpFirstNext = caPara.cpLim;
	pnLast = pfkpd->pn;
	while (cpFirstNext <= cpLim && !FFileWriteError())
		{
		FC fcDest;

/* check for heap overflow */
		if (vmerr.fMemFail)
			{
			if (pfkpPapDummy == 0)
				FreePhplc(&(PdodDoc(vpqsib->docSave)->hplcpad));
			}
		/* 33+1/3% to 50% and 83+1/3% to 100% */
		if ((iCount++ & 15) == 0 && doc == vpqsib->docSave)
			{{ /* !NATIVE - WritePaps */
			CP cpMac = CpMacDoc(vpqsib->docSave);
			CP cpNum = (cpFirstNext + cpMac + cpMac);
			ReportQuickSavePercent (cp0, cpMac+cpMac+cpMac, cpNum);
			}}
		fcDest = fcDestFirst + (cpFirstNext - cpFirst);
		SetWords(&papStd, 0, cwPAP);
		pdod = PdodDoc(doc);
		stc = vpapFetch.stc;
		MapStc(pdod, stc, 0, &papStd);

		/* call FGetValidPhe to get height of paragraph and
			store it in the pap */
		if (!pdod->fStyDirtyBroken && FGetValidPhe(doc, cpFirstNext - 1, &phe))
			phe.fUnk = fFalse;
		else
			SetWords(&phe, 0, cwPHE);

		vpapFetch.phe = phe;
#ifdef MAC
		if (fWord3)
			{
			if (vpapFetch.brcTop != 0 && vpapFetch.brcLeft != 0 && 
					vpapFetch.brcRight != 0 && vpapFetch.brcBottom != 0)
				{
				vpapFetch.brcp = brcpBox;
				vpapFetch.brcl = BrclFromBrc(&vpapFetch.brcTop);
				vdocPapLast = docNil;
				}
			else  if (vpapFetch.brcBar != 0)
				{
				vpapFetch.brcp = brcpBar;
				vpapFetch.brcl = BrclFromBrc(&vpapFetch.brcBar);
				vdocPapLast = docNil;
				}
			else  if (vpapFetch.brcTop != 0)
				{
				vpapFetch.brcp = brcpAbove;
				vpapFetch.brcl = BrclFromBrc(&vpapFetch.brcTop);
				vdocPapLast = docNil;
				}
			else  if (vpapFetch.brcBottom != 0)
				{
				vpapFetch.brcp = brcpBelow;
				vpapFetch.brcl = BrclFromBrc(&vpapFetch.brcBottom);
				vdocPapLast = docNil;
				}
			}
#endif /* MAC */

		cchPapx = CbGenPapxFromPap(&papx, &vpapFetch, &papStd, fWord3);
		if (vpapFetch.fTtp && !fWord3)
			{
			CpFirstTap(doc, caPara.cpFirst);
			SetWords(&tapStd, 0, cwTAP);
			cchPapx = CbAppendTapPropsToPapx(&papx, cchPapx, &vtapFetch, &tapStd, cbMaxGrpprlPapx - 4);
			}

		if (!FAddRun(fnDest,
				fcDest,
				&papx,
				cchPapx,
				pfkpd,
				fTrue /* PAP */,
				fFalse /* use fcPos for new */,
				!vpqsib->fCompleteSave /* during fast save plcbte must expand, 
		during full save plcbte need not expand */,
				fWord3))
			{
			SetErrorMat( matLow );
			return;
			}
		if (cpFirstNext < cpLim)
			{
			CachePara(doc, cpFirstNext);
			cpFirstNext = caPara.cpLim;
			}
		else
			cpFirstNext++;
#ifdef MAC 
		if (pfkpd->pn != pnLast && pfkpd->fPlcIncomplete &&
				FCheckTooManyBtes(PfcbFn(fnDest)->hplcbtePap, pfkpd))
			{
			SetErrorMat(matLow);
			return;
			}
		pnLast = pfkpd->pn;
#endif /* MAC */
		}
	vdocFetch = caPara.doc = caTap.doc = caTable.doc = docNil;
}


#ifdef MAC  /* this is not needed for WIN.  Is it needed for Mac w/0ed pages? */
/* %%Function:ZeroFileRange %%Owner:NOTUSED */
ZeroFileRange(fn, fcFirst, fcLim)
int fn;
FC fcFirst;
FC fcLim;
{
	FC fc;
	char HUGE *hpch;
	FC fcPnLim;
	struct FCB *pfcb;

	for (fc = fcFirst, hpch = HpchFromFc(fn, fc); fc < fcLim;
			fc = fcPnLim, hpch = HpchFromFc(fn, fc))
		{
		fcPnLim = CpMin(fcLim, FcFromPn(PnFromFc(fc) + 1));
/* clear tail of buffer */
		/* no bltbch, so leave bltbcx */
		bltbcx(0, LpFromHp(hpch), (int)((long)fcPnLim - (long)fc));
		SetDirty(vibp);
		}
	if (fcLim > (pfcb = PfcbFn(fn))->cbMac)
		pfcb->cbMac = fcLim;
	pfcb->fcPos = fcLim;
}


/* %%Function:BrclFromBrc %%Owner:NOTUSED */
int BrclFromBrc(pbrc)
struct BRC *pbrc;
{
	int brcl;
	if (pbrc->fShadow)
		brcl = brclShadow;
	else  if (pbrc->dxpLine1Width != 0 && pbrc->dxpLine2Width != 0)
		brcl = brclDouble;
	else  if (pbrc->dxpLine1Width > 1 && pbrc->dxpLine1Width <= 4)
		brcl = brclThick;
	else
		brcl = brclSingle;
	return brcl;
}


#endif

#ifdef WIN
/* C B	W R I T E  S T T B  F O N T  T O  F I L E */
/* Write an sttb for the passed doc's font table to *pfc in
	file fn; store the following fc into *pfc */
/* Avoid heap allocations because save must succeed even during memory outages */
/* %%Function:CbWriteSttbFontToFile %%Owner:davidlu */
CbWriteSttbFontToFile(doc, fn, pfc)
int doc;
int fn;
FC *pfc;
{
	extern struct STTB **vhsttbFont;

	int cb;
	int ibst;
	int ftc;
	struct DOD *pdod = PdodDoc( doc );
	char *pst;

/* First, determine the size of the STTBF that will be written */

	for ( cb = sizeof (int), ftc = ftcMinUser; ftc < pdod->ftcMac; ftc++ )
		{
		char HUGE *hpst = HpstFromSttb( vhsttbFont, (**pdod->hmpftcibstFont) [ftc] );
		cb += ((*hpst == 255) ? 1 : *hpst + 1 );
		}

/* Next, write it out */

	SetFnPos(fn, *pfc);
	WriteRgchToFn( fn, &cb, sizeof (int) );
	for ( ftc = ftcMinUser; ftc < pdod->ftcMac; ftc++ )
		{
		char HUGE *hpst;
		Assert( (**pdod->hmpftcibstFont) [ftc] != iNil );

		hpst = HpstFromSttb( vhsttbFont, (**pdod->hmpftcibstFont) [ftc] );
		WriteHprgchToFn( fn, hpst, (*hpst == 255) ? 1 : *hpst + 1);
		}

	*pfc += cb;
	Assert(*pfc == PfcbFn(fn)->fcPos);
	return cb;
}



/* Q U I C K S A V E  C O M M A N D S */
/* %%Function:QuicksaveCommands %%Owner:davidlu */
QuicksaveCommands()
{
	MUD ** hmud;
	KMP ** hkmp;
	int cbKmp = sizeof(int);
	int cbMud = sizeof(int);
	int istLim = 0;
	int cbSttb = sizeof(int);
	int istFirstTitle;

		/* BLOCK */
		{
		struct DOD * pdod;

		pdod = PdodDoc(vpqsib->docSave);

		hkmp = pdod->hkmpUser;
		hmud = HmudUserPdod(pdod);
		}

		/* BLOCK: first pass : compute sizes/strings needed */
		{
		if (hkmp != hNil)
			{
			int ikme, ikmeMac;
			KME * pkme;
			SY * psy;
			char rgchSy [cbMaxSy];

			ikmeMac = (*hkmp)->ikmeMac;
			pkme = &(*hkmp)->rgkme[0];
			for (ikme = 0; ikme < ikmeMac; ++ikme, ++pkme)
				{
				if (pkme->kt != ktMacro)
					{
					pkme->ibst = -1;
					continue;
					}

				psy = PsyGetSyOfBsy(rgchSy, pkme->bcm);
				if (!psy->fRef)
					{
					SetFRefOfBsy(pkme->bcm);
					istLim += 1;
					cbSttb += psy->stName[0] + 1;
					}
				}

			cbKmp += sizeof(int) * ikmeMac * 2;
			}

		if (hmud != hNil)
			{
			int imtm, imtmMac;
			MTM * pmtm;
			SY * psy;
			char rgchSy [cbMaxSy];

			imtmMac = (*hmud)->imtmMac;
			pmtm = &(*hmud)->rgmtm[0];
			for (imtm = 0; imtm < imtmMac; ++imtm, ++pmtm)
				{
				if ((int) pmtm->bcm < 0)
					continue; /* separators */

				psy = PsyGetSyOfBsy(rgchSy, pmtm->bcm);
				if (!psy->fRef)
					{
					SetFRefOfBsy(pmtm->bcm);
					istLim += 1;
					cbSttb += psy->stName[0] + 1;
					}

				/* menu title string too! */
				if ((int) pmtm->bcm >= 0 && pmtm->ibst != 0 &&
						!pmtm->fRemove)
					{
					extern struct STTB ** hsttbMenu;
					cbSttb += *HpstFromSttb(hsttbMenu,
							pmtm->ibst) + 1;
					istLim += 1;
					}
				}

			cbMud += sizeof(int) * imtmMac * 2;
			}

		vpqsib->cbCmds = cbSttb + cbKmp + cbMud;
		}

		/* BLOCK: second pass */
		{
		int iRefMinUser, fn;

		iRefMinUser = -1;
		fn = vpqsib->fnDest;

		SetFnPos(fn, vpqsib->fcCmds);

			/* BLOCK: write out command name table */
			/* WARNING: assumes file structure of sttb */
			{
			int imtm, imtmMac;
			MTM * pmtm;
			HPSY hpsy;
			uns bsy, bsyLim;
			char stName [cchMaxSyName];
			Debug(int cbWritten = sizeof(int));

			WriteRgchToFn(fn, &cbSttb, sizeof (int));

			bsy = 0;
			bsyLim = vhpsyt->bsy;

			istFirstTitle = 0;
			while (istLim > 0 && bsy < bsyLim)
				{
				hpsy = (HPSY) (vhpsyt->grpsy + bsy);
				if (hpsy->fRef)
					{
					istFirstTitle += 1;
					istLim -= 1;
					bltbh(hpsy->stName, (CHAR HUGE *)stName,
							hpsy->stName[0] + 1);
					WriteRgchToFn(fn, stName,
							stName[0] + 1);
					Debug(cbWritten+=stName[0] + 1);
					}
				bsy += CbHpsy(hpsy);
				}

			/* Write menu title strings... */
			if (hmud != hNil)
				{
				pmtm = &(*hmud)->rgmtm[0];
				imtmMac = (*hmud)->imtmMac;
				for (imtm = 0; imtm < imtmMac; ++imtm, ++pmtm)
					{
					char HUGE * hpst;

					if ((int) pmtm->bcm >= 0 && 
							pmtm->ibst != 0 && !pmtm->fRemove)
						{
						extern struct STTB ** hsttbMenu;

						hpst = HpstFromSttb(hsttbMenu,
								pmtm->ibst);
						WriteHprgchToFn(fn, hpst, 
								*hpst+1);
						Assert(hpst == HpstFromSttb(hsttbMenu,
								pmtm->ibst));/* no HM! */
						istLim -= 1;
						Debug(cbWritten += *hpst+1);
						}
					}
				}

			Assert(istLim == 0);
			Assert(cbWritten==cbSttb);
			}

			/* BLOCK: write out menu */
			{
			int iRef, imtmTitle, imtm, imtmMac = (*hmud)->imtmMac;
			MTM * pmtm;
			Debug(int cbWritten=sizeof(int));

			WriteRgchToFn(fn, &imtmMac, sizeof(int));

			pmtm = &(*hmud)->rgmtm[0];
			imtmTitle = 0;
			for (imtm = 0; imtm < imtmMac; ++imtm, ++pmtm)
				{
				int ibstSav;

				ibstSav = pmtm->ibst;
				if ((int) pmtm->bcm >= 0 && pmtm->ibst != 0
						&& !pmtm->fRemove)
					{
					pmtm->ibst = istFirstTitle + 
							imtmTitle++;
					}
				WriteRgchToFn(fn, pmtm, sizeof (int));
				pmtm->ibst = ibstSav;

				if ((int) pmtm->bsy < 0)
					{
					extern int vbsySepCur;
					iRef =  pmtm->bsy;
					}
				else
					{
					iRef = IRefFromBcm(pmtm->bcm, 
							&iRefMinUser);
					}
				WriteRgchToFn(fn, &iRef, sizeof (int));
				Debug(cbWritten+=2*sizeof(int));
				}
			Assert(cbWritten==cbMud);
			}

			/* BLOCK: write out keymap */
			{
			int ikme, ikmeMac = (*hkmp)->ikmeMac, iRef;
			KME * pkme;
			Debug(int cbWritten=sizeof(int));

			WriteRgchToFn(fn, &ikmeMac, sizeof (int));

			pkme = &(*hkmp)->rgkme[0];
			for (ikme = 0; ikme < ikmeMac; ++ikme, ++pkme)
				{
				WriteRgchToFn(fn, pkme, sizeof (int));
				if ((int) pkme->bcm >= 0)
					{
					iRef = IRefFromBcm(pkme->bcm, 
							&iRefMinUser);
					}
				else
					iRef = -1;
				WriteRgchToFn(fn, &iRef, sizeof (int));
				Debug(cbWritten+=2*sizeof(int));
				}
			Assert(cbWritten==cbKmp);
			}

			/* BLOCK: clear fRef bits */
			{
			HPSY hpsy;
			uns bsy, bsyLim;

			bsy = 0;
			bsyLim = vhpsyt->bsy;

			while (bsy < bsyLim)
				{
				hpsy = (HPSY) (vhpsyt->grpsy + bsy);
				hpsy->fRef = fFalse;
				bsy += CbHpsy(hpsy);
				}
			}
		}
}



/* %%Function:QuicksaveSttb %%Owner:davidlu */
QuicksaveSttb(hsttb, pfc, pcb, pfcDestLim)
struct STTB * hsttb;
FC * pfc;
int * pcb;
FC * pfcDestLim;
{
	FC fc;

	fc = *pfc = *pfcDestLim;
	*pcb = CbSttbf(*hsttb);
	WriteSttbToFile(hsttb, vpqsib->fnDest, &fc);
	Assert(fc == *pfc + *pcb);

	*pfcDestLim += *pcb;
}



/* %%Function:QuicksaveMacros %%Owner:davidlu */
QuicksaveMacros()
{
	int cbNames;
	int docSave;
	int fnDest;
	int docMcr;
	int imcrT2, imcrMacT2;
	struct PLC ** hplcmcr;

	docSave = vpqsib->docSave;
	fnDest = vpqsib->fnDest;

	if ((docMcr = PdodDoc(docSave)->docMcr) == docNil)
		return;

	hplcmcr = PdodDoc(docMcr)->hplcmcr;

	QuicksavePlc(hplcmcr, &vpqsib->fcPlcmcr, &vpqsib->cbPlcmcr,
			&vfcDestLim, cpNil);

	vpqsib->fcSttbfmcr = vfcDestLim;

	if ((imcrMacT2 = (IMacPlc(hplcmcr) - 1) * 2) <= 0)
		return;

	/* write out total length of size and strings */
	if (imcrMacT2 > 0)
		{
		cbNames = cchINT;
		WriteRgchToFn(fnDest, &imcrMacT2, cchINT);
		}
	else
		cbNames = 0;

	for (imcrT2 = 0; imcrT2 < imcrMacT2; imcrT2 += 2)
		{
		int cch;
		MCR mcr;
		char stName [cchMaxSz];

		GetPlc(hplcmcr, imcrT2 / 2, &mcr);
		GetNameFromBsy(stName, mcr.bcm);
		cch = *stName + 1;
		Assert(cch <= cchMaxSyName-1);
		cbNames += cch;
		WriteRgchToFn(fnDest, stName, cch);

		GetMenuHelpSz(mcr.bcm, stName);
		SzToStInPlace(stName);
		cch = *stName + 1;
		cbNames += cch;
		WriteRgchToFn(fnDest, stName, cch);
		}

	vpqsib->cbSttbfmcr = cbNames;
	vfcDestLim += cbNames;

}


#endif /* WIN */


/* R E T R I E V E  H P L C S E D */

/* %%Function:RetrieveHplcsed %%Owner:davidlu */
RetrieveHplcsed(fn, doc, pnFib)
int fn;
int doc;
PN pnFib;
{
	struct DOD *pdod = PdodDoc(doc);
	struct PLC **hplcsed = pdod->hplcsed;
	struct PLC *pplc;
	struct FIB fib;

	FetchFib(fn, &fib, pnFib);

	if (fib.cbPlcfsed)
		{
		long iMac;
		struct SED sed;
		uns ised;
		CP far *lprgcp;
/* calculate how many sed entries stored in disk copy of plc */
		iMac = ((long)fib.cbPlcfsed - (long)sizeof(CP)) / ((long)cbSED + sizeof(CP));
		Assert(iMac + 1 <= (*hplcsed)->iMax);
/* read directly into external part of plc */
		ReadIntoExtPlc(hplcsed, fn, fib.fcPlcfsed, fib.cbPlcfsed);
/* set iMax and IMac to match */
		pplc = *hplcsed;
		pplc->iMac = iMac;
		pplc->iMax = pplc->icpAdjust = iMac + 1;
		pplc->dcpAdjust = cp0;
		pplc->icpHint = 0;
		pdod = PdodDoc(doc);
		/* set the fn's in the section table */
		for (ised = IMacPlc(pdod->hplcsed); ised-- > 0; )
			{
			GetPlc(hplcsed, ised, &sed);
			sed.fn = fn;
			PutPlcLast(hplcsed, ised, &sed);
			}
		CorrectPlcCpMac(pdod->hplcsed, CpMac2Doc(doc));
		}
}


/* B E G I N  F B B  - begin fast io block buffering for file fnDest */
/* %%Function:BeginFbb %%Owner:davidlu */
BeginFbb(fnDest, fUseExtBuff)
{
	Assert(vfbb.fnDest == fnNil);
	Assert(fnDest != fnScratch);

	if (fUseExtBuff)
		{
	/* make sure nothing is left in regular file cache */
		FFlushFn(fnDest);

	/* make sure any reads thru regular file cache go to disk */
		DeleteFnHashEntries(fnDest);

	/* attempt to allocate a large external IO buffer */
		if ((vfbb.hqBuf=HqAllocLcb((long)(vfbb.cchBuf=
				/* it is cheaper to use a smaller buffer when a large one isn't needed */
		(uns)CpMin(cbSector*WinMac(28,60),CpMacDoc(vpqsib->docSave))
				))) == 0)
			vfbb.hqBuf = HqAllocLcb((long)(vfbb.cchBuf = 2048));
		}
	else
		vfbb.hqBuf = 0L;

	vfbb.fnDest = fnDest;
	vfbb.bMac = 0;
	vfbb.fcDest = 0;
}


/* E N D  F B B  - end fast io block buffering.  fbb buffer will be
*  flushed if needed.
*/
/* %%Function:EndFbb %%Owner:davidlu */
EndFbb()
{
	if (vfbb.hqBuf)
		{
		Assert(vfbb.fnDest != fnNil);
		FlushFbb();
		SetFnPos(vfbb.fnDest, vfbb.fcDest);
		FreeHq(vfbb.hqBuf);
		vfbb.hqBuf = 0;
		}

	vfbb.fnDest = fnNil;
}


/* F  A P P E N D  F N  F E T C H  P C D  T O  F N 
*  Write current pcd (of length ccpPcd) from fnFetch at fcFetch, to 
*  fnDest at current position. 
*  Use intermediate buffer hpchBuf of size cchBuf 
*/
/* %%Function:FAppendFnFetchPcdToFbb %%Owner:davidlu */
FAppendFnFetchPcdToFbb()
{
	extern CP ccpPcd;
	extern FC fcFetch;
	FC cfc = ccpPcd;
	char HUGE *hpBuf;
	uns cch;
	FC fcSrc;
	FC cbMac;

	Assert(vfbb.fnDest != fnNil && vfbb.hqBuf != 0);

	/* do direct file io using fbb buffer to copy contents */
	fcSrc = fcFetch;
	hpBuf = WinMac(HpOfHq(vfbb.hqBuf),LpLockHq(vfbb.hqBuf));
	if (PfcbFn(vfbb.fnDest)->cbMac < (cbMac = vfbb.fcDest+cfc+vfbb.bMac))
		PfcbFn(vfbb.fnDest)->cbMac = cbMac;
	for (;;)
		{
		cch = vfbb.cchBuf - vfbb.bMac;
		Assert(cch >= 0);
		if ( cch > cfc )
			cch = cfc;
#ifdef MAC
		if (EcReadWriteFn(fnFetch, fcSrc, hpBuf + vfbb.bMac, cch, 
				fFalse/*fWrite*/, fTrue/*fErrors*/) < 0)
			return fFalse;
		if (vpqsib->fWord3)
			ReplaceChTable(hpBuf, cch, vpqsib->fcMac, fcSrc);
#else /* WIN - use what is in the file buffers */
		SetFnPos(fnFetch, fcSrc);
		ReadHprgchFromFn(fnFetch, hpBuf + vfbb.bMac, cch);
#endif /* MAC */
		vfbb.bMac += cch;
		cfc -= cch;
		Assert(cfc >= 0);
		if (cfc <= 0 || 
				EcReadWriteFn(vfbb.fnDest, vfbb.fcDest, hpBuf, 
				vfbb.bMac, fTrue/*fWrite*/, fTrue/*fErrors*/) < 0)
			break;
		vfbb.fcDest += vfbb.bMac;
		fcSrc += cch;
		vfbb.bMac = 0;
		}

	Mac(UnlockHq(vfbb.hqBuf));
	return (fTrue);
}


/* F L U S H  F B B  - flush fbb buffer if it contains anything */
/* %%Function:FlushFbb %%Owner:davidlu */
FlushFbb()
{
	FC cbMac;

	if (vfbb.bMac != 0)
		{
		/* flush buffer to file */
		char HUGE *hpBuf = WinMac(HpOfHq(vfbb.hqBuf),
				LpLockHq(vfbb.hqBuf));
		if (PfcbFn(vfbb.fnDest)->cbMac < (cbMac = vfbb.fcDest + vfbb.bMac))
			PfcbFn(vfbb.fnDest)->cbMac = cbMac;
#ifdef MAC
		if (vpqsib->fWord3)
			ReplaceChTable(hpBuf, vfbb.bMac, vpqsib->fcMac, vfbb.fcDest);
#endif
		EcReadWriteFn(vfbb.fnDest, vfbb.fcDest, hpBuf, vfbb.bMac, 
				fTrue/*fWrite*/, fTrue/*fErrors*/);
		Mac(UnlockHq(vfbb.hqBuf));
		vfbb.fcDest += vfbb.bMac;
		vfbb.bMac = 0;
		}
}


/* S E E K  F B B  T O  F C  - "seek" the fbb buffer pointer to fc */
/* %%Function:SeekFbbToFc %%Owner:davidlu */
SeekFbbToFc(fc)
FC fc;
{
	Assert(vfbb.fnDest != fnNil);
	if (vfbb.hqBuf == 0)
		SetFnPos(vfbb.fnDest, fc);
	else  if (vfbb.fcDest + vfbb.bMac != fc)
		{
		FlushFbb(); /* write any pending buffer contents */
		vfbb.fcDest = fc;
		}
}


/* A P P E N D  H P C H  T O  F B B -
*  Append cfc bytes at hpch to end of vfbb.fnDest.  Buffering is
*  performed to consolidate blocks.
*/
/* %%Function:AppendHpchToFbb %%Owner:davidlu */
AppendHpchToFbb(hpch, cfc)
char HUGE *hpch;
uns cfc;
{
	uns cch;
	char HUGE *hpBuf;
	FC cbMac;

	Assert(vfbb.fnDest != fnNil);

	/* if no buffer was allocated, just write thru regular file cache */
	if (vfbb.hqBuf == 0)
		{
		WriteHprgchToFn(vfbb.fnDest, hpch, cfc);
		return;
		}

	/* write into fbb buffer; flushing as needed */
	hpBuf = WinMac(HpOfHq(vfbb.hqBuf),LpLockHq(vfbb.hqBuf));
	while (cfc > 0)
		{
		if (vfbb.bMac == vfbb.cchBuf)
			{
			/* flush buffer to file */
			if (PfcbFn(vfbb.fnDest)->cbMac < 
					(cbMac = vfbb.fcDest + vfbb.bMac))
				PfcbFn(vfbb.fnDest)->cbMac = cbMac;
#ifdef MAC
			if (vpqsib->fWord3)
				ReplaceChTable(hpBuf, vfbb.bMac, vpqsib->fcMac, vfbb.fcDest);
#endif
			if (EcReadWriteFn(vfbb.fnDest, vfbb.fcDest, 
					hpBuf, vfbb.bMac, fTrue, fTrue) < 0)
				break;
			vfbb.fcDest += vfbb.bMac;
			vfbb.bMac = 0;
			}

		cch = min(cfc, vfbb.cchBuf - vfbb.bMac);
		bltbh(hpch, hpBuf + vfbb.bMac, cch);
		hpch += cch;
		vfbb.bMac += cch;
		cfc -= cch;
		}

	Mac(UnlockHq(vfbb.hqBuf));
}


/* W R I T E  P L  T O  F N */
/* %%Function:WritePlToFn %%Owner:davidlu */
WritePlToFn(hpl, fn, pfc,  pcb)
struct PL **hpl;
int     fn;
FC      *pfc;
int     *pcb;
{
	struct PL *ppl;
	int     cbRgfoo;
	char    *pchFoo;

	*pcb = 0;
	if (hpl != hNil)
		{
		SetFnPos(fn, *pfc);
		ppl = *hpl;

		/* write iMac */
		WriteRgchToFn(fn, &ppl->iMac,sizeof(int));

		Assert(ppl->brgfoo == cbPLBase);
		/* write rgfoo */
		pchFoo = ppl->rgbHead;
		cbRgfoo = ppl->cb * ppl->iMac;
		WriteRgchToFn(fn, pchFoo, cbRgfoo);
		*pcb = 2 + cbRgfoo;
		*pfc += *pcb;
		}
}


/* W R I T E  S T T B  T O  F I L E */
/* %%Function:WriteSttbToFile %%Owner:davidlu */
WriteSttbToFile(hsttb, fn, pfc)
struct STTB    **hsttb;
int     fn;
FC      *pfc;
{
	struct STTB    *psttb = *hsttb;
	int ibstMac = psttb->ibstMac;
	int fExternal = psttb->fExternal;
	int fStyleRules = psttb->fStyleRules;
	char HUGE *hpst;
	uns   	cbExtra;
	int     ibst;
	uns     bMaxNew;

	AssertH(hsttb);
	cbExtra = psttb->cbExtra;
	SetFnPos(fn, *pfc);
	bMaxNew = psttb->bMax - (psttb->ibstMax - 1) * sizeof(int);
	WriteRgchToFn(fn, &bMaxNew, sizeof(int));
	for (ibst = 0; ibst < ibstMac; ibst++)
		{
		hpst = HpstFromSttb(hsttb, ibst);
		WriteHprgchToFn(fn, hpst, 
				(*hpst == 255 && fStyleRules) ? 1 : *hpst+1);
		if (cbExtra != 0)
			WriteHprgchToFn(fn, HpchExtraFromSttb(hsttb, ibst), cbExtra);
		Assert(hpst == HpstFromSttb(hsttb, ibst));/* no HM! */
		}

	*pfc += bMaxNew;
	Assert(*pfc == PfcbFn(fn)->fcPos);
}


/* %%Function:WriteFib %%Owner:davidlu */
WriteFib(fn, pfib, pn)
int fn;
struct FIB *pfib;
PN pn;
{
	char HUGE *hpch;

	Mac( vfWritingFib = fTrue );	/* never looked at in WIN */
	hpch = HpchGetPn(fn, pn);
	Mac( vfWritingFib = fFalse );
	if (!vmerr.fDiskWriteErr)
		{
		bltbh(pfib, hpch, cbFIB);
		Assert(cbFIB <= cbFileHeader);
		SetBytes(LpOfHp(hpch+cbFIB), 0, cbFileHeader-cbFIB);
		SetDirty(vibp);
		}
}


