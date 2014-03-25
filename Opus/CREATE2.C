/* C R E A T E 2 */
/*  Core functionality of creation code */

#define NOWINMESSAGES
#define NOVIRTUALKEYCODES
#define NONCMESSAGES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOSYSMETRICS
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOHDC
#define NOMEMMGR
#define NOMINMAX
#define NOMSG
#define NOPEN
#define NOPOINT
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#define NOKANJI
#define NOCTLMGR
#define NOFONT
#define NOMETAFILE
#define NOGDI

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "doc.h"
#include "file.h"
#include "props.h"
#include "sel.h"
#include "prm.h"
#include "fkp.h"
#include "ch.h"
#include "ruler.h"
#include "debug.h"
#include "format.h"
#include "screen.h"
#include "wininfo.h"
#include "winddefs.h"
#include "menu2.h"
#include "keys.h"
#include "layout.h"
#include "cmdtbl.h"
#include "field.h"
#include "disp.h"
#include "inter.h"
#include "strtbl.h"
#include "doslib.h"
#include "ibdefs.h"
#include "outline.h"
#define FINDNEXT
#include "search.h"
#include "cmd.h"


#ifdef PROTOTYPE
#include "create2.cpt"
#endif /* PROTOTYPE */

#ifdef DEBUG
#ifdef PCJ
/* #define SHOWFLDS */
/* #define DBGDOCOPEN */
#endif /* PCJ */
#endif /* DEBUG */

/* E X T E R N A L S */
extern struct PREF      vpref;
extern struct SCI       vsci;
extern struct FTI       vfti;
extern struct FLI       vfli;
extern struct DOD       **mpdochdod[];
extern int              docMac;
extern struct FCB       **mpfnhfcb[];
extern int              fnMac;
extern int              vdocTemp;
extern struct SELS      rgselsInsert[];
extern int              vdocScratch;
extern struct MERR      vmerr;
extern struct PRC       **vhprc;
extern struct SEL       selCur;
extern struct UAB       vuab;
extern struct SAB       vsab;
extern CHAR             szEmpty[];
extern CHAR             stEmpty[];
extern CHAR             rgchEop[];
extern struct RULSS     vrulss;
extern int              docGlobalDot;
extern int              vdocFetch;
extern struct FLS       vfls;
extern int              wwMac;
extern struct WWD       **mpwwhwwd[];
extern struct MWD       **hmwdCur;
extern int		mwCur;
extern struct CA        caPage;
extern struct CA        caPara;
extern struct CA        caSect;
extern int  	    	vdocPapLast;
extern int		vdocHdrEdit;
extern struct STTB      **HsttbPropeFromStsh();
extern uns              **HCopyHeapBlock();
extern struct DBS       vdbs;
extern FNI              fni;
extern struct AAB 	vaab;

extern MUD ** vhmudUser;
extern KMP ** hkmpCur;
extern KMP ** vhkmpUser;

struct DTTM DttmCur();
long DttmFromPdta();



/* D O C  C R E A T E */
/*  Create a doc to place fn in.  If fn is fnNil, create a plain (fDoc)
	document.
*/

/* %%Function:DocCreate %%Owner:chic */
int DocCreate(fn)
int fn;
{ /* Create a document.  Caller responsible for reporting errors. */
	int doc;

	if (fn != fnNil)
		{
		StartLongOp();
		if ((doc = DocCreateFn (fn, fTrue, NULL, NULL, NULL)) == docCancel)
			/*  USER opted to cancel the operation */
			doc = docNil;
		EndLongOp(fFalse);
		}
	else
		doc = DocCloneDoc (docNew, dkDoc);

	return doc;

}


/*  %%Function:DocAlloc %%Owner:peterj  */
/* D O C  A L L O C */
/*  Allocate a doc and initialize its dod.  Doc will be of type dk.  Pdod
	is pointer to stack dod which will contain the value of *PdodDoc (doc).
*/

DocAlloc (dk, pdod)
int dk;
struct DOD *pdod;

{
	int doc;
	int cw;

#ifdef DEBUG
	switch (dk)
		{
	case dkDoc:
	case dkDot:
	case dkGlsy:
	case dkMcr:
	case dkHdr:
	case dkFtn:
	case dkAtn:
	case dkSDoc:
	case dkDispHdr:
		break;
	default:
		Assert (fFalse);
		}
#endif /* DEBUG */
	/*  %%Function:SetBytes %%Owner:peterj  */
	SetBytes (pdod, 0, cbDOD);
	pdod->fFormatted = fTrue;
	pdod->dk = dk;
	pdod->lvl = lvlAll;

	if (!pdod->fShort)
		{
		cw = cwDOD;
		pdod->dttmOpened = DttmCur();
		}
	else
		cw = cwDODShort;

	doc = IAllocCls (clsDOD, cw);
	Assert (cbDODShort == sizeof (int) * cwDODShort);
	Assert (cbDOD == sizeof (int) * cwDOD);

	if (doc != docNil)
		blt (pdod, PdodDoc (doc), cw);

	Debug( vdbs.fCkHeap ? CkHeap() : 0 );
	return doc;

}


/*  %%Function:DocCreateSub %%Owner:peterj  */
/* D O C  C R E A T E  S U B */
/*  Create a doc of type dk which is linked to doc.  Returns docNil
	if cannot create.  Sets up links in both directions.  (Created document
	is not necessarly fShort).
*/

DocCreateSub (docMother, dk)
int docMother, dk;

{
	int doc;
	struct DOD *pdodMother;

	if ((doc = DocCloneDoc (docNew, dk)) == docNil)
		return docNil;

	/*  set up links */

	PdodDoc (doc)->doc = docMother;
	pdodMother = PdodDoc (docMother);
	Assert (!pdodMother->fShort);
	PdodDoc(doc)->fLockForEdit = pdodMother->fLockForEdit;
	switch (dk)
		{
	case dkHdr:
		pdodMother->docHdr = doc;
		pdodMother->fFormatted = fTrue;
		break;
	case dkFtn:
		pdodMother->docFtn = doc;
		pdodMother->fFormatted = fTrue;
		break;
	case dkGlsy:
		Assert (pdodMother->fDot);
		pdodMother->docGlsy = doc;
		break;
	case dkMcr:
		Assert(pdodMother->fDot);
		pdodMother->docMcr = doc;
		break;
	case dkAtn:
		pdodMother->docAtn = doc;
		PdodDoc(doc)->fLockForEdit = fFalse; /* regardless of mother */
		pdodMother->fFormatted = fTrue;
		break;
	default:
		break;
		}

	return (doc);
}


/*  %%Function:DocCloneDoc %%Owner:peterj  */
/* D O C  C L O N E  D O C */
/*  Clone document docSrc, but make it a type dk document.  Returns a doc
	or docNil.  Cloning from an fShort to !fShort is NOT handled!
*/

DocCloneDoc (docSrc, dk)
int docSrc, dk;

{
	int docClone;
	struct DOD **hdodSrc = mpdochdod [docSrc];
	struct DOD **hdodClone;
	struct CA caT, caSrc;

	if ((docClone = DocCloneDocNoText (docSrc, dk, fFalse)) == docNil)
		return docNil;

	hdodClone = mpdochdod [docClone];

	if (!(*hdodClone)->fMother)
		/* new doc is not a mother, must give it a mother for the moment
			so that FReplaceCps doesn't get confused. (Caller responsible for
			resetting doc.) */
		(*hdodClone)->doc = (*hdodSrc)->fMother ? docSrc : docNew;

	if (docSrc != docNew) /* docNew has no text */
		{
		BOOL f;
	/* Copy text of doc to be cloned into clone doc. Copies up to 
		CpMacDoc and FReplaceCps will automatically get properties of 
			final section. */

		/* avoid useless Inval calls */
		DisableInval();
		f = FReplaceCps( PcaSet( &caT, docClone, cp0, CpMacDoc(docClone) ),
				PcaSet( &caSrc, docSrc, cp0, CpMacDoc( docSrc ) ) );
		EnableInval();
		if (!f)
			goto ErrRet;
		}
#ifdef ENABLE /* docNew has no section properties.  Enable if that changes */
	else
		{
		/*  apply docNew's formatting to the new document.  Note: only section
			properties currently supported. */
		ApplyDocSep (docClone, docNew);
		}
#endif /* ENABLE */

	/* if both are DOT's, Clone Glsy and Mcr subdocs */
	if ((*hdodClone)->fDot && (*hdodSrc)->fDot)
		{
		if ((*hdodSrc)->docGlsy != docNil)
			{
			int docGlsy;
			Assert ((*hdodClone)->dk != dkGlsy);
			docGlsy = DocCloneDoc ((*hdodSrc)->docGlsy, dkGlsy);
			if (((*hdodClone)->docGlsy = docGlsy) == docNil)
				goto ErrRet;
			PdodDoc (docGlsy)->doc = docClone;
			}

		if ((*hdodSrc)->docMcr != docNil)
			{
			int docMcr;
			int imcr, imcrMac;
			struct PLC ** hplcmcrClone;
			struct PLC ** hplcmcr;
			uns bsy;
			MCR mcr;
			char stName [cchMaxSyName];

			docMcr = DocCloneDoc((*hdodSrc)->docMcr, dkMcr);
			if (((*hdodClone)->docMcr = docMcr) == docNil)
				goto ErrRet;
			PdodDoc(docMcr)->doc = docClone;
			
			
			/* If we are cloning a mcr doc, copy all the 
				command table entries. */

			hplcmcrClone = PdodDoc(docMcr)->hplcmcr;
			Assert(hplcmcrClone != hNil);
				
			hplcmcr = PdodDoc((*hdodSrc)->docMcr)->hplcmcr;
			Assert(hplcmcr != hNil);
				
			imcrMac = (*hplcmcrClone)->iMac - 1;
			for (imcr = 0; imcr < imcrMac; imcr += 1)
				{
				GetPlc(hplcmcr, imcr, &mcr);
				GetNameFromBsy(stName, mcr.bcm);
				if ((bsy = BsyAddCmd1(stName, docClone, imcr)) == bsyNil)
					{
					(*hplcmcrClone)->iMac = imcr + 1;
					goto ErrRet;
					}

				mcr.bcm = bsy;
				PutPlc(hplcmcrClone, imcr, &mcr);
				}
			}
		}

	/* if both are glossaries, copy glsy structures */
	else  if ((*hdodClone)->fGlsy && (*hdodSrc)->fGlsy)
		{
		/* plc copy must be AFTER FReplaceCps, hence not in FCloneDocStructs */
		(*hdodClone)->hplcglsy =
				HCopyHeapBlock((*hdodSrc)->hplcglsy, strcPLC);
		(*hdodClone)->hsttbGlsy =
				HCopyHeapBlock((*hdodSrc)->hsttbGlsy, strcSTTB);
		if (vmerr.fMemFail)
			goto ErrRet;
		}

	/* if both are Ftn,Hdr or Mcr or Atn, copy plc */
	else  if ((*hdodSrc)->fShort && (*hdodSrc)->dk == (*hdodClone)->dk)
		{
		(*hdodClone)->hplcfnd =
				HCopyHeapBlock((*hdodSrc)->hplcfnd, strcPLC);
		if (vmerr.fMemFail)
			goto ErrRet;
		}
		
	/*  cannot CkDoc a subdoc yet, must set up links and structures */
	Debug (vdbs.fCkDoc && ( (*hdodClone)->fDoc ||  (*hdodClone)->fDot )
			? CkDoc (docClone) : 0);

	return docClone;

ErrRet:
	DisposeDoc (docClone);
	return docNil;
}


/*  %%Function:DocCloneDocNoText %%Owner:peterj  */
/* D O C  C L O N E  D O C  N O  T E X T */
/*  Clones all of doc to a new doc of type dk but does not copy
	doc's text.  If fExtraInfo then new doc will have same docType
	(crefLock incremented) and some common summary info.
*/

DocCloneDocNoText (docSrc, dk, fExtraInfo)
int docSrc, dk;
BOOL fExtraInfo;

{
	int docClone;
	struct DOD dod;
	struct STTB **hsttbSrc;

	/*  allocate the new doc */
	if ((docClone = DocAlloc (dk, &dod)) == docNil)
		return docNil;

	/*  set up the piece table */
	if (!FInitPlcpcd (&dod, fnNil, NULL))
		goto LFailed1;

	Assert (!PdodDoc(docSrc)->fShort || dod.fShort);

	/*  copy basic structures */
	if (!dod.fShort)
		{
		if (!FCloneDocStructs (&dod, docSrc))
			{
LFailed1:
			DisposeDocPdod (docClone, &dod);
			return docNil;
			}
		}
	else
		fExtraInfo = fFalse;

	/*  copy new dod back out to the heap */
	blt (&dod, PdodDoc(docClone), dod.fShort ? cwDODShort : cwDOD);

	/* if have ftn separators, create docHdr, copy text, copy hplchdd */
	if (!dod.fShort && dod.dop.grpfIhdt != 0)
		{
		int docHdrSrc, docHdrClone;
		int ihddFtnSepMac = 0;
		int ihddMac;
		int grpfIhdt;
		struct CA caIns, caT;
		struct PLC **hplchddClone;
		struct PLC **hplchddSrc;

		docHdrSrc = PdodDoc(docSrc)->docHdr;
		hplchddSrc = PdodDoc(docHdrSrc)->hplchdd;
		Assert(docHdrSrc && hplchddSrc);
		docHdrClone = DocCreateSub(docClone, dkHdr);
		if (docHdrClone == docNil || 
				(hplchddClone = HplcCreateEdc(mpdochdod[docHdrClone], edcHdd)) == hNil)
			goto LFailed;
		grpfIhdt = dod.dop.grpfIhdt;
		ihddMac = IMacPlc(hplchddSrc);

		for (; grpfIhdt != 0; grpfIhdt >>= 1)
			if ((grpfIhdt & 1) && ++ihddFtnSepMac == ihddMac)
				{
				Assert(fFalse);
				goto LFailed;
				}
		Assert(ihddFtnSepMac > 0);
		CaFromIhddFirstLim(docHdrSrc, 0, ihddFtnSepMac, &caIns);
		if (!FReplaceCps(PcaSet(&caT, docHdrClone, cp0, cp0), &caIns))
			goto LFailed;
		PutCpPlc(hplchddClone, 0, CpPlc(hplchddClone, 0)+DcpCa(&caIns));
		if (!FOpenPlc(hplchddClone, 0, ihddFtnSepMac))
			goto LFailed;
		CopyMultPlc(ihddFtnSepMac, hplchddSrc, 0, hplchddClone, 0, cp0, 0, 0);
		}

	if (fExtraInfo && (hsttbSrc = PdodDoc (docSrc)->hsttbAssoc) != hNil)
		{
		int ibst, docDot;
		struct STTB **hsttbClone = HsttbAssocEnsure (docClone);
		if (hsttbClone == hNil)
			{
LFailed:
			DisposeDoc (docClone); /* also frees hsttbClone */
			return docNil;
			}
		/* warning: assumes order of ibst's */
		for (ibst = ibstAssocDot; ibst <= ibstAssocComments; ibst++)
			{
			CHAR st [cchMaxSz];
			GetStFromSttb(hsttbSrc, ibst, st);
			FChangeStInSttb (hsttbClone, ibst, st);
			if (vmerr.fMemFail)
				goto LFailed;
			}

		if (PdodDoc(docSrc)->fDoc && (docDot=PdodDoc(docSrc)->docDot)!=docNil)
			{
			PdodDoc (docClone)->docDot = docDot;
			PdodDoc (docDot)->crefLock++;
			}
		}


	return docClone;
}


/*  %%Function:FCloneDocStructs %%Owner:peterj  */
/* F  C L O N E  D O C  S T R U C T S */
/*  Copy the basic structures from docSrc to pdod.  Clone only meaningful if
	both source and clone are !fShort. Does not copy hsttbAssoc.

	allocation failure note: this routine assumes that its caller(s)
		will free all the dod handles if we return fFalse, so we just bag out here

*/

FCloneDocStructs (pdod, docSrc)
struct DOD *pdod;
int docSrc;

{
	struct STTB **hsttb;
	struct PLESTCP **hplestcp;
	struct DOD * pdodSrc;
	struct DOD dodSrc;

	blt (PdodDoc(docSrc), &dodSrc, cwDOD);

	Assert ( !pdod->fShort && !dodSrc.fShort );

	/* new or unformatted files have no DOP, so perform an aDOPtion. */
	pdod->dop = dodSrc.dop;
	if (PdodDoc(docSrc)->fDot) /* do not carry lock for annotation over */
		pdod->dop.fLockAtn = fFalse;

#ifdef DEBUG
	/* this is a test. Docs that can be saved will later have a non-zero
		value put in. */
	pdod->dop.nRevision = 0;
#endif /* DEBUG */

	/* create a section table */
	if (FNoHeap(HplcCreateEdc(&pdod, edcSed)))
		goto ErrRet;

	/* copy fonts */
	if (FNoHeap ( pdod->hmpftcibstFont =
			HCopyHeapBlock(dodSrc.hmpftcibstFont,strcPLAIN) ))
		goto ErrRet;
	pdod->ftcMac = dodSrc.ftcMac;
	pdod->ftcMax = dodSrc.ftcMax;

	/*  copy style sheet */
	pdod->stsh.cstcStd = dodSrc.stsh.cstcStd;
	if (FNoHeap ( pdod->stsh.hsttbName =
			HCopyHeapBlock(dodSrc.stsh.hsttbName,strcSTTB) ))
		goto ErrRet;
	if (FNoHeap ( pdod->stsh.hsttbChpx =
			HCopyHeapBlock(dodSrc.stsh.hsttbChpx,strcSTTB) ))
		goto ErrRet;
	if (FNoHeap ( pdod->stsh.hsttbPapx =
			HCopyHeapBlock(dodSrc.stsh.hsttbPapx,strcSTTB) ))
		goto ErrRet;
	if (FNoHeap ( pdod->stsh.hplestcp =
			HCopyHeapBlock(dodSrc.stsh.hplestcp,strcPL) ))
		goto ErrRet;

	if (FNoHeap ( pdod->hsttbChpe = HCopyHeapBlock(dodSrc.hsttbChpe,strcSTTB) ))
		goto ErrRet;
	if (FNoHeap ( pdod->hsttbPape = HCopyHeapBlock(dodSrc.hsttbPape,strcSTTB) ))
		goto ErrRet;

	/* keymap and menu */
	if (pdod->fDot)
		/* a DOT must have keymap and menu */
		if (dodSrc.fDot)
			/*  source is a dot, copy its keymap and menu */
			{
			if (FNoHeap ( pdod->hkmpUser =
					HCopyHeapBlock (dodSrc.hkmpUser, strcPLAIN) ))
				goto ErrRet;
			if (FNoHeap ( pdod->hmudUser =
					HCopyHeapBlock (dodSrc.hmudUser, strcPLAIN) ))
				goto ErrRet;
			}
		else
			/*  must create new keymap and menu since dest is a dot */
			{
			if (FNoHeap ( pdod->hkmpUser = HkmpCreate(0, kmfPlain) ))
				goto ErrRet;
			(*pdod->hkmpUser)->hkmpNext = vhkmpUser;
			if (FNoHeap ( pdod->hmudUser = HmudInit (0) ))
				goto ErrRet;
			}


	return !vmerr.fMemFail;

ErrRet:
	return fFalse;
}


/*  %%Function:DocCreateScratch %%Owner:peterj  */
/* D O C  C R E A T E  S C R A T C H */
/*  Creates vdocScratch if it hasn't already been created.  For debugging
	purposes checks and sets a flag indicating that vdocScratch is in use.
	Note that it may fail, in which case it will return docNil.

	The Scratch document, vdocScratch, is a general purpose, long-dod
	document that may be used by anyone as long as its not already in use
	(condition assured under DEBUG only).  Call ReleaseDocScratch() when
	done.  Will be thrown away at idle time (thus caller is not responsible
	for emptying or disposing).

	Passing a doc will save style sheet merging.
*/

DocCreateScratch (doc)
int doc;

{
	struct CA ca;
	Assert (!fDocScratchInUse);

	if (vdocScratch != docNil ||
			(vdocScratch = DocCreate (fnNil)) != docNil)
		{
		Debug (fDocScratchInUse = fTrue);
		if (doc != docNil)
			{
			struct DOD *pdod = PdodDoc (vdocScratch);
			pdod->fMotherStsh = fTrue;
			pdod->doc = doc;
			}
		SetWholeDoc (vdocScratch, PcaSetNil(&ca));
		}

	return vdocScratch;
}


/*  %%Function:DocCreateTemp %%Owner:peterj  */
/*******************************/
/* D o c  C r e a t e  T e m p */
DocCreateTemp(doc)
int doc;
{
/* create the temp document if it doesn't exist yet; in any case make it a
	sub-creature of the specified doc.  NOTE: this can fail! 
	User is responsible for assuring that what is stored in vdocTemp is small
	or for deleting or disposing when done.*/
	Assert(doc != docNil);
	doc = DocMother(doc);
	if (vdocTemp == docNil)
		vdocTemp = DocCreateSub (doc, dkSDoc);
	else
		PdodDoc(vdocTemp)->doc = doc;
	return(vdocTemp);
}



/*  %%Function:FInitPlcpcd %%Owner:peterj  */
/* I N I T  P L C P C D */
/* create initial piece table. There are the following cases:
	fn==fnNil:  Eop piece, End
	fHasFib:  fcMin - fcMac piece, End
Unformatted:  fcMin - fcMac piece, Eop piece, End

where Eop piece has fnNil and fcSpecEop.
End piece has cpMin = cpMac of doc.
*/
BOOL FInitPlcpcd(pdod, fn, ppnFib)
struct DOD  *pdod;
int fn;
PN *ppnFib;
	{ /* Initialize the piece table for a doc, given its initial fn.  If fn
			has a fib, on return *ppnFib is pnNext of this fib.  (Allows
		sequential calls to FInitPlcpcd for compound files). */

	CP cpMac;
	struct PLC  **hplcpcd;
	struct PCD pcd;
	struct FIB fib;

	if (FNoHeap(hplcpcd = HplcInit(cbPCD, 1, cpMax, fTrue /* ext rgFoo */)))
		return fFalse;

	if (fn == fnNil)
		{
/* place two special eops at end of document. one will be last para of doc and
	the other will be the hidden section mark. Last is document sentinel*/
		BuildPcd(&pcd, fnInsert, (FC)fcSpecEop);
		if (!FInsertInPlc(hplcpcd, 0, (CP) 0, &pcd))
			goto LFail;
		cpMac = ccpEop;
		if (!FInsertInPlc(hplcpcd, 1, ccpEop, &pcd) ||
				!FInsertInPlc(hplcpcd, 2, 2*ccpEop, &pcd))
			goto LFail;
		cpMac += ccpEop * 2;
		}
	else
		{
/* short docs are always allocated using fnNil */
		struct FCB *pfcb = *mpfnhfcb[fn];
		if (pfcb->fHasFib)
			{
/* we don't add the special eops for Word files, because the caller will
	later call InsertHiddenSectionMark. */
			Assert (ppnFib != NULL);
			FetchFib(fn, &fib, *ppnFib);
			BuildPcd(&pcd, fn, fib.fcMin);
			if (!FInsertInPlc(hplcpcd, 0, (CP) 0, &pcd))
				goto LFail;
			cpMac = fib.fcMac - fib.fcMin;
			*ppnFib = fib.pnNext;
			}
		else
			{
			int ipcd = 0;
			char rgch [cchEop];

			cpMac = pfcb->cbMac;
			BuildPcd(&pcd, fn, (FC)0);
			if (cpMac > 0 && !FInsertInPlc(hplcpcd, ipcd++, 
					(CP) 0, &pcd))
				goto LFail;
			BuildPcd(&pcd, fnInsert, (FC)fcSpecEop);
/* we do add the special eops for text files. */
			if (!FInsertInPlc(hplcpcd, ipcd++, cpMac, &pcd) ||
					!FInsertInPlc(hplcpcd, ipcd++, cpMac + ccpEop, &pcd))
				goto LFail;
/* if the text file ends with an eop, we won't have to add one of the special
	eops. */
			if (cpMac - ccpEop > cp0)
				{
				SetFnPos(fn, cpMac - ccpEop);
				ReadRgchFromFn(fn, rgch, (int) ccpEop);
				}
			if (cpMac < ccpEop || FNeRgch(rgch, rgchEop, (int) ccpEop))
				{
				if (!FInsertInPlc(hplcpcd, ipcd++, 
						cpMac + 2*ccpEop, &pcd))
					goto LFail;
				cpMac += ccpEop;
				}
/* delay adjustment for first two extra eops till now so that the comparisons
	above are simpler. */
			cpMac += 2*ccpEop;
			}
		}
	PutCpPlc(hplcpcd, IMacPlc(hplcpcd), cpMac);

	pdod->cpMac = cpMac;
	pdod->hplcpcd = hplcpcd;
	Debug( vdbs.fCkHeap ? CkHeap() : 0 );
	return fTrue;

LFail:
	FreeHplc(hplcpcd);
	return fFalse;


}       /* end FInitPlcpcd */


/*  %%Function:BuildPcd %%Owner:peterj  */
/* B U I L D  P C D */
BuildPcd(ppcd, fn, fc)
struct PCD *ppcd; 
int fn; 
FC fc;
{
	SetBytes(ppcd,0,sizeof(struct PCD));
	ppcd->fn = fn;
	ppcd->fc = fc;
	Debug(ppcd->fNoParaLastValid = fFalse);
	ppcd->fNoParaLast = fFalse;
	ppcd->prm = prmNil;
#ifdef DEBUG
	if (vdbs.fCkDoc)
		Assert( FValidFn(fn) );
#endif
}


/*  %%Function:DisposeDoc %%Owner:peterj  */
/* D I S P O S E   D O C   */
/* if the doc is not the scrap document and no windows are displaying
	the document:
	1) reclaim all heap blocks referenced by doc
	2) deallocate the DOD for the doc
	3) invalidate caches and declare no undo allowed if undo refers
		to this doc
*/
DisposeDoc(doc)
int doc;
{ /* Wipe this doc, destroying any changes since last save */

	int isels;
	struct DOD *pdod, dod;
	extern struct PL **vhplbmc;

	Assert(doc >= 0 && doc < docMac);
	Assert(doc == docNil || mpdochdod[doc] != hNil);

/* Return without destroying if crefLock != 0 or it is not a valid doc to
	Dispose. */
	if (doc == docNil || mpdochdod[doc] == hNil || doc == docScrap
			|| FDocBusy(doc))
		return;

	if (vhplbmc != hNil)
		InvalBmc(doc, cp0, CpMacDoc(doc));

	pdod = PdodDoc(doc);
	Assert (pdod->wwDisp == wwNil && pdod->crefLock == 0);

	if (pdod->fDot && pdod->docMcr != docNil)
		RemoveDotMacros(doc);

	/* make sure the stsh for scrap is saved */
	if (vsab.docStsh == doc)
		CheckScrapStsh(doc);
	pdod = PdodDoc(doc);

/* kill the cache for vpapStc if it points to the doc we're disposing of */
	if (doc == vdocPapLast)
		vdocPapLast = docNil;
	if (doc == vdocHdrEdit)
		vdocHdrEdit = docNil;
	/* Free all tables attached to dod */
	blt(pdod, &dod, pdod->fShort ? cwDODShort : cwDOD);
	if (doc == vdocTemp)
		vdocTemp = docNil;
	else  if (doc == vdocScratch)
		vdocScratch = docNil;
	DisposeDocPdod(doc, &dod );

	Assert (vdocScratch == docNil || PdodDoc(vdocScratch)->doc
			!= doc || !fDocScratchInUse);

	vdocFetch = docNil;
	InvalCaFierce();
	InvalFli();
	if (selCur.doc == doc)
		selCur.doc = docNil;
	if (vrulss.caRulerSprm.doc == doc)
		vrulss.caRulerSprm.doc = docNil;
	if (vsab.caCopy.doc == doc)
		vsab.caCopy.doc = docNil;
	if (vuab.ca.doc == doc || vuab.caMove.doc == doc)
		{
		SetUndoNil();

/* FUTURE : (cc) more appropriate place to set docNil to vuab is in SetUndoNil
	but because we want minimum side effect, we do it here, see bug #3349 */
		vuab.ca.doc = docNil;
		vuab.caMove.doc = docNil;
		}

	if (fni.doc == doc)
		fni.doc = docNil;
	for (isels = 0; isels < iselsInsertMax; isels++)
		if (rgselsInsert[isels].doc == doc)
			rgselsInsert[isels].doc = docNil;
	if ((vaab.doc == doc && vaab.bcm == bcmTyping) ||
			(vaab.caAgain.doc == doc &&
			(vaab.bcm == bcmMoveSp || vaab.bcm == bcmCopySp)))
		InvalAgain();
}


/*  %%Function:DisposeDocPdod %%Owner:peterj  */
/* D I S P O S E  D O C  P D O D */
/*  Disposes a doc whose dod is pointed to by pdod.  Frees heap space used,
	disposes referenced documents, and frees the doc itself.
*/

DisposeDocPdod (doc, pdod)
int doc;
struct DOD *pdod;

{
	if (!pdod->fShort)
		{
		/* recursively call DisposeDoc for all short doc,
			has to do this before the mother doc is freed because 
			WwDisp from FDocBusy is trying to refer to pdodMother */
		DisposeDoc(pdod->docFtn);
		DisposeDoc(pdod->docHdr);
		DisposeDoc(pdod->docAtn);
		if (pdod->fDot)
			DisposeDoc(pdod->docMcr);
		}
	else  if (pdod->dk == dkHdr && pdod->doc != docNil)
		/* trash all display docs for header */
		FCleanUpHdr(doc, fTrue, fFalse);

	FreeCls(clsDOD, doc);
	FreeHplc(pdod->hplcpcd);
	FreeHplc(pdod->hplcfld);
	FreeHplc(pdod->hplcfnd); /* hplchdd, hplcglsy, hplcmcr, hplcand, ... */
	FreeHplc(pdod->hplcpgd);
	if (!pdod->fShort)
		{
		FreeHsttb(pdod->hsttbAssoc);
		FreeHplc(pdod->hplcsed);
		FreeHplc(pdod->hplcsea);
		FreeHplc(pdod->hplcfrd);
		FreeHplc(pdod->hplcpad);
		FreeHplc(pdod->hplcatrd);
		FreeHsttb(pdod->stsh.hsttbName);
		FreeHsttb(pdod->stsh.hsttbChpx);
		FreeHsttb(pdod->stsh.hsttbPapx);
		FreeH(pdod->stsh.hplestcp);
		FreeHsttb(pdod->hsttbChpe);
		FreeHsttb(pdod->hsttbPape);
		FreeHplc(pdod->hplcbkf);
		FreeHplc(pdod->hplcbkl);
		FreeHsttb(pdod->hsttbBkmk);
		FreeH(pdod->hmpftcibstFont);


		/* deal with docDot/hsttbGlsy/docGlsy union */
		if (pdod->fGlsy)
			FreeHsttb(pdod->hsttbGlsy);
		else  if (pdod->fDot)
			{
			DisposeDoc (pdod->docGlsy);
			if (hkmpCur == pdod->hkmpUser)
				{
				hkmpCur = (*pdod->hkmpUser)->hkmpNext;
				}
			FreeH(pdod->hkmpUser);
			FreeH(pdod->hmudUser);
			}
		else  if (pdod->fDoc && pdod->docDot)
			{
			PdodDoc(pdod->docDot)->crefLock--;
			DisposeDoc (pdod->docDot);
			}
		}


}


/*  %%Function:DocSubEnsure %%Owner:peterj  */
/* D O C  S U B  E N S U R E */
/* ensures that footnote/annotation related appendages exist for doc and
returns docFtn/docAtn.
*/
DocSubEnsure(doc, edcDrp)
int doc, edcDrp;
{
	struct DOD **hdodMom = mpdochdod[doc];
	struct DOD **hdodSub;
	int docSub;
	struct PLC **hplcRef;
	struct DRP *pdrp;
	int dk, cbRef, cbfoo;

	if (edcDrp == edcDrpFtn)
		{
		dk = dkFtn;
		cbRef = cbFRD;
		cbfoo = cbFND;
		}
	else
		{
		dk = dkAtn;
		cbRef = cbATRD;
		cbfoo = cbAND;
		}

	Assert(!(*hdodMom)->fShort);
	pdrp = ((int *)PdodDoc(doc)) + edcDrp;
	hplcRef = pdrp->hplcRef;
	if ((docSub = pdrp->doc)  == docNil)
		{
/* create docSub, empty with trailing Eop */
		if ((docSub = DocCreateSub(doc, dk)) == docNil)
			return(docNil);
		pdrp = ((int *)PdodDoc(doc)) + edcDrp;
		pdrp->doc = docSub;
		(*hdodMom)->fDirty = fTrue;
		}
	if (hplcRef == hNil)
		{
/* create hplcRef (ftn/atn reference table) for doc */
		if ((hplcRef =
				HplcInit(cbRef, 0, CpMac2Doc(doc), fTrue /* ext rgFoo */)) == hNil)
			goto LRetFail;
		pdrp = ((int *)PdodDoc(doc)) + edcDrp;
		pdrp->hplcRef = hplcRef;
		}
	hdodSub = mpdochdod[docSub];
	Assert(docSub != docNil);
	if ((*hdodSub)->hplcfnd == hNil)
/* create text table in docSub */
		{/* hplcfnd is different from hplcfrd, it needs an initial entry
		cpMac entry, therefore do not use HplcInit */
		if (HplcCreateEdc(hdodSub, edcFnd) == hNil)
			goto LRetFail;
		}
	(*hdodMom)->fFormatted = fTrue;
	return docSub;
LRetFail:
	if (hplcRef != hNil)
		{
		FreeHplc(hplcRef);
		pdrp = ((int *)PdodDoc(doc)) + edcDrp;
		pdrp->hplcRef = hNil;
		}
	DisposeDoc(docSub);
	pdrp = ((int *)PdodDoc(doc)) + edcDrp;
	pdrp->doc = docNil;
	return docNil;
}


/*  %%Function:ApplyDocMgmtNew %%Owner:peterj  */
/* A P P L Y  D O C  M G M T  N E W */
/* set up document management properties */

ApplyDocMgmtNew (doc, fn)
int doc, fn;
{
	struct STTB **hsttb;
	struct DOD *pdod;
	struct DOP *pdop;

	FreezeHp();
	pdod = PdodDoc (doc);
	Assert ( !pdod->fShort);
	pdod->fPromptSI = fTrue;
	pdop = &pdod->dop;

	pdop->nRevision = 1; /* reflects version working on. updated at save */
	pdop->tmEdited = 0;  /* minutes this file has been edited. */
	pdop->dttmRevised.lDttm = 0;
	pdop->dttmLastPrint.lDttm = 0;

	if (fn == fnNil) /* new file */
		{
		pdop->dttmCreated = DttmCur();
		pdop->dttmRevised.lDttm = 0;
		}
	else
		{
		struct FINDFILE dta;
		struct DTTM *pdttm;
		char szFile[ichMaxFile];

		StToSz(PfcbFn(fn)->stFile, szFile);
		AnsiToOem(szFile, szFile);
		FFirst(&dta, szFile, DA_NORMAL|DA_READONLY);

	/* The following expands out DttmFromPdta, it is done for swap
	*  tuning purposes.
	*/
		pdttm = &(pdop->dttmCreated.lDttm);
		pdttm->mint = dta.mint;
		pdttm->hr = dta.hr;
		pdttm->dom = dta.dom;
		pdttm->mon = dta.mon;
		pdttm->yr = dta.yr + 80; /* DOS year started at 1980 */

		pdttm->wdy = 0;
		pdop->dttmRevised.lDttm = *(long *) pdttm;

#ifdef WINBUG /* the dos time and date in ofh from WINDOW is in reversed order */
		(struct DTTM)DttmFromDOSTIMEDATE(PfcbFn(fn)->ofh.dostd);
#endif
		}


	MeltHp();

	/* add author */

	if ((hsttb = HsttbAssocEnsure(doc)) != hNil && fn == fnNil)     /* HM */
		/* if this fails then the author will be incorrect */
		FChangeStInSttb( hsttb, ibstAssocAuthor, vpref.stUsrName );
}

