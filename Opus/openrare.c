/* O P E N R A R E . C */
/* rarely used code for opening documents */


#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "version.h"
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
#include "print.h"
#include "cmdtbl.h"
#include "field.h"
#include "disp.h"
#include "inter.h"
#include "strtbl.h"
#include "message.h"
#include "doslib.h"
#include "filecvt.h"
#include "dlbenum.h"
#include "error.h"
#include "prompt.h"

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
extern struct STTB **vhsttbCvt;
extern struct CA        caSect;
extern struct PRI       vpri;
extern struct SEP   	vsepFetch;
extern struct SPX	mpiwspxSep[];

struct STTB      **HsttbPropeFromStsh();
extern struct DBS       vdbs;
extern int vfConversion;
extern MUD ** vhmudUser;
extern KMP ** hkmpCur;
extern KMP ** vhkmpUser;

struct DTTM DttmCur();


csconst char rgchRTF[] = {
	'{', '\\','r','t','f'};


/*  used to convert between versions of the fib */
ConvertFib (pfib)
struct FIB *pfib;
{
	if (pfib->nFib < 21)
		pfib->fHasPic = fTrue;
}


/* S U B S E T  D O C */
/*  Delete the portion of doc not in bookmark specified by stBkmk.
	If stBookmark not defined, doc not affected.  If the file is changed,
	it is unlinked form any file it might have been associated with.
*/

/* %%Function:SubsetDoc  %%Owner:chic */
SubsetDoc (doc, stBkmk)
int doc;
CHAR *stBkmk;

{
	struct CA ca, caT;

	Assert (!PdodDoc(doc)->fShort);

	if (stBkmk != NULL && FSearchBookmark (ca.doc = doc, stBkmk, &ca.cpFirst,
			&ca.cpLim, NULL, bmcUser))
		{
		/* delete all but the bookmark and the end of doc EOP */
		AssureLegalSel (&ca);
		FDelete( PcaSet(  &caT, doc, ca.cpLim, CpMacDocEdit(doc) ));
		FDelete( PcaSet( &caT, doc, cp0, ca.cpFirst ) );
		PdodDoc (doc)->fn = fnNil;
		}
}




/* C O N V E R T  D O C */
/*  Convert doc from its present form into a doc of type dk.
	Only dkDoc --> dkDot and dkDot --> dkDoc are currently
	supported.
*/

/* %%Function:FConvertDoc  %%Owner:chic */
FConvertDoc (doc, dk)
int doc, dk;

{

	struct DOD *pdod, **hdod = mpdochdod [doc];

	if (hdod == hNil || (pdod = *hdod)->dk == dk)
		return;

	if (pdod->dk == dkDoc && dk == dkDot)
		/*  convert a doc into a dot */
		{
		int ww;
		KMP ** hkmp;
		MUD ** hmud;
		struct STTB **hsttb;

		if ((hkmp = HkmpCreate(0, kmfPlain)) == hNil)
			return fFalse;

		if ((hmud = HmudInit(0)) == hNil)
			{
			FreeH(hkmp);
			return fFalse;
			}

		FreezeHp ();
		pdod = *hdod;

		/* get rid of doc's old dot, if any */
		if (pdod->docDot != docNil)
			PdodDoc(pdod->docDot)->crefLock--;
		DisposeDoc (pdod->docDot);

		/* new dot has no docGlsy */
		pdod->docGlsy = docNil;

		/*  set up its type */
		pdod->dk = dkDot;
		pdod->fDirty = fTrue; /* because of this change */
		pdod->fFormatted = fTrue;

		/*  only DOTs have keymaps */
		Assert (pdod->hkmpUser == hNil);
		Assert (pdod->hmudUser == hNil);
		MeltHp ();

		/*  the new DOT must have keymap and menu */
		(*hdod)->hkmpUser = hkmp;
		(*hkmp)->hkmpNext = vhkmpUser;
		(*hdod)->hmudUser = hmud;
		
		for (ww = (*hdod)->wwDisp; ww != wwNil; ww = PwwdWw(ww)->wwDisp)
			PwwdWw(ww)->hkmpCur = hkmp;

		/* must remove any dot reference from the hsttbAssoc */
		if ((hsttb = (*hdod)->hsttbAssoc) != hNil)
			/* making same size or smaller, won't fail */
			AssertDo(FChangeStInSttb (hsttb, ibstAssocDot, stEmpty));

		}

	else  if (pdod->dk == dkDot && dk == dkDoc)
		{

#ifdef DEBUG   /* case of dot being refered to by any doc not
		supported. */
					{
			int docT;
			struct DOD **hdodT;
			for (docT = 0; docT < docMac; docT++)
				if ((hdodT = mpdochdod [doc]) != hNil && (*hdodT)->fDoc
						&& (*hdodT)->docDot == doc)
					Assert (fFalse);
			}
#endif /* DEBUG */

		/* get rid of glossary document */
		DisposeDoc (pdod->docGlsy);

		/*  set the doc's values */
		pdod->dk = dkDoc;

		/*  new doc does not have a doc type */
		pdod->docDot = docNil;

		/*  get rid of keymaps and menu */
		RemoveKmp (pdod->hkmpUser);
		pdod->hkmpUser = hNil;
		FreeHmud (pdod->hmudUser);
		pdod->hmudUser = hNil;
		}


#ifdef DEBUG
	else
		Assert (fFalse);
#endif /* DEBUG */

	return fTrue;
}


/* %%Function:StFileInDocDir %%Owner:chic */
StFileInDocDir(stFile1, stFile2, doc)
CHAR *stFile1, *stFile2;
int doc;
{
	struct FNS fns1, fns2;
	GetDocSt(doc, stFile2, gdsoFullPath);
	if (!*stFile2)
		return;
	StFileToPfns(stFile2, &fns1);
	StFileToPfns(stFile1, &fns2);
	CopySt(fns1.stPath, fns2.stPath);
	PfnsToStFile(&fns2, stFile2);
}


/* C H A N G E  D O C  D O T */
/*  Change doc so that it is linked to stDot.  doc may have be displayed in
	any number of windows.  doc may already be associated to a dot.  stDot
	may be empty.
*/

/* %%Function:ChangeDocDot  %%Owner:chic */
ChangeDocDot (doc, docDotNew)
int doc, docDotNew;

{
	int docDotOld;
	struct DOD *pdod;
	int ww, mw;
	struct WWD **hwwd;
	KMP **hkmpNew, ** hkmpOld;
	struct STTB **hsttbAssoc;
	CHAR stDot [ichMaxFile];

	Assert (mpdochdod[doc] != hNil && PdodDoc (doc)->fDoc);
	Assert (docDotNew == docNil || PdodDoc (docDotNew)->fDot);

	pdod = PdodDoc (doc);
	docDotOld = pdod->docDot;
	pdod->docDot = docDotNew;

	Assert(docDotNew != docGlobalDot);
	Assert(docDotOld != docGlobalDot);

	if (docDotNew == docDotOld)
		/*  nothing to do! */
		return;

	if (docDotNew != docNil)
		PdodDoc(docDotNew)->crefLock++;

	GetDocSt (docDotNew, stDot, gdsoFullPath|gdsoNoUntitled);
	Assert(*stDot || docDotNew == docNil);
	if ((hsttbAssoc = HsttbAssocEnsure (doc)) != hNil)
		/* will not make change permanent if this fails, oh well! */
		FChangeStInSttb (hsttbAssoc, ibstAssocDot, stDot);

	if (doc == selCur.doc)
		SetupMenu(docDotNew);

	hkmpOld = docDotOld == docNil ? vhkmpUser : PdodDoc(docDotOld)->hkmpUser;
	hkmpNew = docDotNew == docNil ? vhkmpUser : PdodDoc(docDotNew)->hkmpUser;
	Assert (hkmpNew == vhkmpUser || (*hkmpNew)->hkmpNext == vhkmpUser);

	if (hkmpOld != hkmpNew)
		{
	/* reset keymaps on all windows */
		for (ww = 1; ww < wwMac; ww++)
			{
			if ((hwwd = mpwwhwwd [ww]) != hNil && 
					(mw = (*hwwd)->mw) != mwNil &&
					doc == DocMother(PmwdMw(mw)->doc))
				{
				KMP ** hkmpT;

				hkmpT = (*hwwd)->hkmpCur;
				if (hkmpT == hkmpOld)
					(*hwwd)->hkmpCur = hkmpNew;
				else  if ((*hkmpT)->hkmpNext == hkmpOld) /* when in a mode */
					(*hkmpT)->hkmpNext = hkmpNew;
				}
			}

		if (hkmpCur == hkmpOld)
			hkmpCur = hkmpNew;
		}

	if (docDotOld != docNil)
		{
		PdodDoc(docDotOld)->crefLock--;
		if (!FDocBusy(docDotOld) && 
				FConfirmSaveDoc(docDotOld, fFalse /*fFalse*/, acQuerySave))
			DisposeDoc (docDotOld);
		}
	PickNewWw();
}


/* F F I L L  M I S S I N G  B T E  P N S */
/* %%Function:FFillMissingBtePns %%Owner:chic */
FFillMissingBtePns(fn, hplcbte, pfib, cpnPrev, fChp)
int fn;
struct PLC **hplcbte;
struct FIB *pfib;
PN cpnPrev;
int fChp;
{
	PN pnCur;
	PN ibte, cpn;
	PN cpnBte;
	PN ibteMac = IMacPlc(hplcbte);
	PN cpnTot;
	PN cpnMissing;
	struct FKP HUGE *hpfkp;
	struct FIB fib;

	Assert(ibteMac >= 1);

	cpnBte = fChp ? pfib->cpnBteChp : pfib->cpnBtePap;
	if (pfib->pnNext != pn0)
		{
		FetchFib(fn, &fib, pfib->pnNext);
		if (pfib->nFib >= 32)
			cpnBte -= fChp ? fib.cpnBteChp : fib.cpnBtePap;
		}
	cpnTot = cpnBte + cpnPrev;
	cpnMissing = (cpnTot > ibteMac) ? (cpnTot - ibteMac) : 0;


	if (cpnMissing >= cpnBte)
		pnCur = fChp ? pfib->pnChpFirst : pfib->pnPapFirst;
	else
		{
		GetPlc(hplcbte, ibteMac - 1, &pnCur);
		pnCur++;
		}
	if (!FOpenPlc(hplcbte, ibteMac, cpnMissing))
		return fFalse;
	for (ibte = ibteMac, cpn = 0; cpn < cpnMissing; cpn++, ibte++, pnCur++)
		{
		hpfkp = (struct FKP HUGE *)HpchGetPn(fn, pnCur);
		PutCpPlc(hplcbte, ibte, hpfkp->rgfc[0]);
		PutPlc(hplcbte, ibte, &pnCur);
		}
	Assert(pfib->pnNext != pn0 ?  ibte >= cpnTot : ibte == cpnTot);
	if (pfib->pnNext != pn0)
		{
		/* set fib.pnNext to pn0 so we don't have an accidental recursion */
		fib.pnNext = pn0;
		if (!FFillMissingBtePns(fn, hplcbte, &fib, cpnTot, fChp))
			return fFalse;
		}
	return fTrue;
}


/* %%Function:QueryAndStartHotLinks %%Owner:chic */
QueryAndStartHotLinks (doc)
int doc;

{
	extern struct CA caAdjust;

	int cField;

	if (IdMessageBoxMstRgwMb (mstStartDdeHot, &doc,
			(MB_ICONQUESTION|MB_YESNO)) != IDYES)
		return;

	StartLongOp (); /* probably rendundant */
	AcquireCaAdjust ();

	if (FSetPcaForCalc (&caAdjust, doc, cp0, CpMacDoc (doc), &cField))
		FCalcFields (&caAdjust, frmDdeHot, fTrue/*fPrompt*/,
				fFalse/*fClearDiffer*/);

	ReleaseCaAdjust ();
	EndLongOp (fFalse);
}


/* %%Function:DocCreateForeignFn  %%Owner:chic */
DocCreateForeignFn (fn, fInput, stConverter, pstSubset)
int fn;
BOOL fInput;
CHAR *stConverter, **pstSubset;
{
	int dff, dffOrig;
	int doc = docNil;
	int fnT;

	Assert (fn != fnNil && mpfnhfcb [fn] != hNil);
	Assert(!PfcbFn (fn)->fHasFib);

	/*  find out what we think it is (does quick scan) */
	dffOrig = DffFromFn(fn);

	/* open file using stConverter */
	if (stConverter != NULL && *stConverter)
		{
		int dffMac;
		CHAR szBuff[cchMaxSz];
		CHAR szConverter[cchMaxSz];

		StToSz(stConverter, szConverter);
		/*  check for predefined converters */
		for (dff = dffOpenMin; dff < 0; dff++)
			{
			AssertDo(FEnumIEntbl(iEntblDffOpen, dff-dffOpenMin, szBuff));
			if (FEqNcSz(szBuff, szConverter))
				goto LConvert;
			}
		if (vhsttbCvt != hNil || FInitCvt())
			{
			dffMac = (*vhsttbCvt)->ibstMac;
			for (dff = 0; dff < dffMac; dff++)
				{
				CchCvtTknOfDff(dff, itCvtName, szBuff, cchMaxSz);
				if (FEqNcSz(szBuff, szConverter))
					goto LConvert;
				}
			}
		}

	/* Conversion=No in win.ini supresses verify dialog (assume text) */
	if (vfConversion == -1)
		vfConversion = FFromProfile(fTrue, ipstConversion);
	if (!vfConversion)
		fInput = fFalse;

	dff = dffOrig;
	/* must try to determine the format */
	if (dff == dffNil && fInput)
		if ((vhsttbCvt != hNil || FInitCvt()) && (*vhsttbCvt)->ibstMac != 0)
			/* call libraries to find one */
			dff = DffFindFmt(fn);

	if (dff == dffNil)
		/* assume plain text */
		dff = dffOpenText;

	/* dff is our best guess of format */
	if (fInput)
		/* verify the format */
		if ((dff = DffVerifyDff (dff, fn)) == dffNil)
			/* cancel operation */
			return docCancel;

LConvert:
	/* dff is what is requested (via stConverter, dialog or our guess)
		dffOrig is dffNil or specifies what format we think it may be
	*/
	PfcbFn(fn)->fForeignFormat = fTrue;

	/* convert fn in format dff */
	switch (dff)
		{
	case dffOpenRTF:
		if (dffOrig != dff)
			break;
		doc = DocCreateRtf (fn);
		break;

	case dffOpenBiff:
	case dffOpenWks:
	case dffOpenMp:
		if (dffOrig != dff)
			break;
		StToSzInPlace(*pstSubset);
		doc = DocCreateDffSsFn(dff, fn, *pstSubset);
		*pstSubset = NULL;
		break;

	case dffOpenText8:
		if ((fn = FnTranslateOemFn(fnT = fn)) == fnNil)
			break;
		/* fall through */

	case dffOpenText:
		if ((doc = DocReadPlainFn (fn)) != docNil)
			if (dff == dffOpenText)
				PfcbFn(fn)->fForeignFormat = fFalse;
			else
				PdodDoc(doc)->fn = fnT;
		break;

	default:
		Assert (dff >= 0);
		/* call conversion routine for dff: */
		doc = DocCreateFnDff (fn, dff, *pstSubset);
		*pstSubset = NULL;
		break;
		}

	return doc;
}



/* %%Function:DffFromFn %%Owner:chic */
DffFromFn(fn)
int fn;
{
	int dff;
	unsigned wMP, w0, *rguns;
	CHAR rgch [cbSector];
	CHAR rgchRTFLocal [sizeof(rgchRTF)];

	bltbh(HpchGetPn(fn, pn0), rgch, cbSector);

	/* first try for the formats which we know how to identify */
	rguns = rgch;
	wMP = (w0 = rguns[0]) & 0xff08;

	if (wMP == wMagicMPV20 || wMP == wMagicMPV30)
		dff = dffOpenMp;

	else  if (w0 == wrtBegin && rguns[1] == 2 && (rguns[2] >> 8) == 4)
		dff = dffOpenWks;

	else  if (w0 == rtBOF && rguns[1] == 4)
		dff = dffOpenBiff;

	else
		{
		bltbx((CHAR FAR *)rgchRTF, (CHAR FAR *)rgchRTFLocal,
				sizeof (rgchRTF));
		dff = (!FNeRgch(rgch, rgchRTFLocal, sizeof(rgchRTF)) ||
				!FNeRgch(rgch+1, rgchRTFLocal, sizeof(rgchRTF)))
				? dffOpenRTF : dffNil;
		}

#ifdef DAVIDBO
	CommSzNum(SzShared("dff = "), dff);
#endif

	return dff;
}



/* D O C  R E A D  P L A I N  F N */
/*  Read a plain text file into a newly allocated doc.  Doc is a clone of
	docGlobalDot.  Returns docNil on error.
*/
/* %%Function:DocReadPlainFn  %%Owner:chic */
DocReadPlainFn (fn)
int fn;
{
	int doc;
	CP cp;
	CHAR rgch [cchEop];
	struct DOD dod;

	Assert (docGlobalDot != docNil);
	Assert (!PfcbFn (fn)->fHasFib);

	if ((doc = DocAlloc (dkDoc, &dod)) == docNil)
		return docNil;

	dod.fn = fn;
	dod.fFormatted = fFalse;

	/*  init piece table, clone docNew's basic structs & init fonts */
	if (!FInitPlcpcd (&dod, fn, NULL))
		goto LErrRet;
	Assert (dod.hmpftcibstFont == hNil);
	if (!FCloneDocStructs (&dod, docGlobalDot) ||
			!FAddDocFonts (hNil/*hsttb*/, &dod))
		{
LErrRet:
		DisposeDocPdod (doc, &dod);
		return docNil;
		}

	/*  copy local dod back to the heap */
	blt (&dod, PdodDoc (doc), cwDOD);

	/*  apply docGlobalDot's formatting to the new document.  Note: only section
		properties currently supported. */
	ApplyDocSep (doc, docGlobalDot);

	ApplyDocMgmtNew (doc, fn);   /* set up document management properties */

	return doc;
}


/* A P P L Y  D O C  S E P */
/* applies section properties of docSrc to doc */
/* %%Function:ApplyDocSep %%Owner:chic */
ApplyDocSep(doc, docSrc)
int doc;
int docSrc;
{
	int cchSepx;
	struct CA ca;
	struct SEP sepStandard;
	char grpprlSepx[cbMaxGrpprl];

	StandardSep(&sepStandard);
	CacheSect(docSrc, (CP)CpMacDoc(docSrc));
	cchSepx = CbGrpprlProp(fTrue, grpprlSepx, cbMaxGrpprl,
			&vsepFetch, &sepStandard, cwSEP - 1, mpiwspxSep, 0 /* mpiwspxSepW3 */);
	if (cchSepx)
		{
		ca.doc = doc;
		ca.cpFirst = CpMacDoc(doc);
		ca.cpLim = ca.cpFirst + ccpEop;
		ApplyGrpprlCa(grpprlSepx, cchSepx, &ca);
		}
}


/* D O C  O P E N  D O T */
/*  Open stDot which is the name of a dot.  Return the resulting doc.
	Opened doc will be fDot.  If fErrors, report errors if it couldn't be
	opened or is not a Dot.
*/

/* %%Function:DocOpenDot  %%Owner:chic */
DocOpenDot (stDot, fErrors, docFor)
CHAR *stDot;
BOOL fErrors;
int docFor;

{
	int doc;
	CHAR stDot2[ichMaxFile];

#ifdef DBGDOCOPEN
	CommSzSt (SzShared("DocOpenDot: "), stDot);
#endif /* DBGDOCOPEN */

	Assert(stDot[0] != 0);

	if (docFor != docNil)
		StFileInDocDir(stDot, stDot2, docFor);
	else
		stDot2[0] = 0;

	if ((doc = DocOpenStDof (stDot, dofBackground|dofSearchDot, NULL))
			== docNil && (stDot2[0] == 0 ||
			(doc = DocOpenStDof(stDot2, dofBackground|dofSearchDot, NULL))
			== docNil))
		{
		if (fErrors)
			ErrorEid (eidCantOpenDOT, "DocOpenDot");
		return docNil;
		}
	else  if (!PdodDoc (doc)->fDot)
		/*  dot opened not really a dot (could also have been doc) */
		{
		if (fErrors)
			ErrorEid (eidDotNotValid, "DocOpenDot");
		DisposeDoc (doc);
		return docNil;
		}
	else  if (doc == docGlobalDot)
		/* not allowed to link to this */
		return docNil;

	return doc;
}


/* F  R E A D  C M D S */
/* %%Function:FReadCmds %%Owner:chic */
FReadCmds(fn, fc, hdod)
int fn;
FC fc;
struct DOD ** hdod;
{
	struct STTB ** hsttb;
	KMP ** hkmp;
	MUD ** hmud;
	MTM * pmtm;
	KME * pkme;
	struct DOD * pdod;
	int i, iMac, ibst;
	CHAR stT[cchMaxSt];

	if (FNoHeap(hsttb = HsttbReadSttbfFromFile(fn, fc, fTrue, fFalse,0)))
		return fFalse;

	/* Read menu delta */
	ReadRgchFromFn(fn, &iMac, sizeof(int));
	if (FNoHeap(hmud = HmudInit(iMac)))
		{
LErr:
		FreeHsttb(hsttb);
		return fFalse;
		}

	ReadRgchFromFn(fn, &(*hmud)->rgmtm[0], iMac * cbMTM);
	(*hmud)->imtmMac = iMac;

	for (i = 0; i < iMac; ++i)
		{
		extern int vbsySepCur;
		int bsy;

		pmtm = &(*hmud)->rgmtm[i];
		if ((bsy = (int) pmtm->bsy) >= 0)
			{
			GetStFromSttb(hsttb, bsy, stT);
			pmtm->bsy = BcmOfSt(stT);
			}
		else  if (bsy < bsySepMax && bsy > vbsySepCur)
			vbsySepCur = bsy + 1;

		/* load custom item string (if any) into hsttbMenu */
		if ((int) pmtm->bcm >= 0 && pmtm->ibst != 0 && !pmtm->fRemove)
			{
			GetStFromSttb(hsttb, pmtm->ibst, stT);
			/* HEAP MOVEMENT */
			/* !!! mtm.ibst is 9 bits */
			if ((ibst = IbstMenuBsySt(pmtm->bsy, stT)) == ibstNil)
				{
				FreeHmud(hmud);
				goto LErr;
				}
			(*hmud)->rgmtm[i].ibst = ibst;
			}
		}


	/* Read keymap */
	ReadRgchFromFn(fn, &iMac, sizeof(int));
	if (FNoHeap(hkmp = HkmpCreate(iMac, kmfPlain)))
		{
		FreeHmud(hmud);
		goto LErr;
		}

	(*hkmp)->hkmpNext = vhkmpUser;

	ReadRgchFromFn(fn, &(*hkmp)->rgkme[0], iMac * cwKME * 2);
	(*hkmp)->ikmeMac = iMac;

	FreezeHp();
	for (pkme = &(*hkmp)->rgkme[i = 0]; i < iMac; ++i, ++pkme)
		{
		if (pkme->ibst != -1)
			{
			GetStFromSttb(hsttb, pkme->ibst, stT);
			pkme->bcm = BcmOfSt(stT);
			}

		if (pkme->bcm == bcmNil)
			pkme->kt = ktBeep;
		}
	MeltHp();

	FreeHsttb(hsttb);

	pdod = *hdod;
	pdod->hkmpUser = hkmp;
	pdod->hmudUser = hmud;

	return fTrue;
}





/* F N  T R A N S L A T E  O E M  F N */
/*  Translate fnSrc into ANSI and write it out to a new file.  Return the
	new file's fn.
*/
FnTranslateOemFn(fnSrc)
int fnSrc;
{
	extern int vfnNoAlert;
	int fnDest = fnNil;
	FC fcMac = PfcbFn(fnSrc)->cbMac;
	FC fc = 0;
	FC fcNext = 0;
	int cch;
	struct PPR **hppr;
	CHAR stName[ichMaxFile];
	CHAR rgchBuff[cbSector+1];

	stName[0] = 0;
	if ((fnDest = FnOpenSt(stName, fOstCreate, ofcTemp, NULL)) == fnNil)
		return fnNil;

	vfnNoAlert = fnDest;

	hppr = HpprStartProgressReport(mstConvertNA, NULL, nIncrPercent, fFalse);

	SetFnPos(fnSrc, fc0);
	SetFnPos(fnDest, fc0);

	EnablePreload(fnSrc);

	while (fc < fcMac && !vmerr.fDiskAlert && !vmerr.fDiskWriteErr)
		{
		if (fc >= fcNext)
			ProgressReportPercent(hppr, fc0, fcMac, fc, &fcNext);
		if (fcMac - fc < cbSector)
			cch = fcMac - fc;
		else
			cch = cbSector;
		ReadRgchFromFn(fnSrc, rgchBuff, cch);
		rgchBuff[cch] = 0;
		OemToAnsi(rgchBuff, rgchBuff);
		WriteRgchToFn(fnDest, rgchBuff, cch);
		fc += cch;
		}

	DisablePreload();

	ChangeProgressReport(hppr, 100);
	StopProgressReport(hppr, pdcRestore);

	vfnNoAlert = fnNil;

	if (vmerr.fDiskAlert || vmerr.fDiskWriteErr)
		{
		DeleteFn(fnDest, fTrue  /* fDelete */);  /* really get rid of temp file */
		return fnNil;
		}
	else
		return fnDest;
}


