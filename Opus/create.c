/* C R E A T E . C */
/*  Contains code to manipulate doc's and fn's for doc's. */
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
#include "help.h"

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
extern struct ESPRM	dnsprm[];
extern struct DBS       vdbs;
extern int vfConversion;
extern int vcelof;
extern MUD ** vhmudUser;
extern KMP ** hkmpCur;
extern KMP ** vhkmpUser;
extern HANDLE           vhInstance;
extern HWND             vhwndCBT;

#ifdef NOASM
extern FC               FcAppendRgchToFn();
#endif /* NOASM */
struct STTB      **HsttbPropeFromStsh();
struct DTTM DttmCur();
struct PLC **HplcReadPlcf();


/* D O C  C R E A T E  F N */
/*  Create a new doc and place file fn into it.  Deals with both
	binary format fn and plain text fn.  On Error returns docNil.
	If fInput is FALSE, the user will not be prompted for input.
	stConverter overrides the default search order for converters for
	non-opus files.  stSubset can specify a subset of the file to be
	opened.  If fInput is true, user may have option of canceling the
	operation in which case we return docCancel.

	Note: If you create a doc with a subset, BE SURE that the doc is
	Disposed of immediately!  Otherwise if a user tries to open the full doc
	later, they will only get the subset!
*/
/*  %%Function:DocCreateFn %%Owner: peterj  */

DocCreateFn (fn, fInput, stConverter, stSubset, psels)
int fn;
BOOL fInput;
CHAR *stConverter, *stSubset;
struct SELS *psels;

{
	int doc = docNil;
	int fnT;

	Assert (fn != fnNil && mpfnhfcb [fn] != hNil);

	if (psels != NULL)
		psels->ca.doc = docNil;

	/*  Opus binary format */
	if (PfcbFn (fn)->fHasFib)
		{
		/*  first we read the fib at beginning of file */
		PN pnFib = pn0;
		struct STTB **hsttb;

		/*  turn the fib into a doc */
		if ((doc = DocReadFnPpnFib (fn, &pnFib, psels)) == docNil)
			return docNil;

		/*  if this is a compound file, read in second doc */
		if (pnFib != pn0)
			{
			int docT;
			/*  now we are reading the later fib (pnFib modified above) */
			if ((docT = DocReadFnPpnFib (fn, &pnFib, NULL)) == docNil)
				{
				DisposeDoc (doc);
				return docNil;
				}
			/*  glossary document only current use of compound file */
			Assert (PdodDoc (docT)->fGlsy);
			Assert (PdodDoc (doc)->fDot);
			PdodDoc (doc)->docGlsy = docT;
			PdodDoc (docT)->doc = doc;
			Debug (vdbs.fCkDoc ? CkDoc (docT) : 0);
			}

		/*  read in doc type for this document, if any */
		if ((hsttb = PdodDoc (doc)->hsttbAssoc) != hNil)
			{
			CHAR stDot[cchMaxFile];
			GetStFromSttb(hsttb, ibstAssocDot, stDot);

			if (stDot[0] != 0)
				{
				int docDot;
				Assert (PdodDoc (doc)->fDoc);
				if ((docDot = DocOpenDot(stDot, fInput/*fErrors*/, doc))
						!= docNil)
					{
					/*  link doc to its dot */
					PdodDoc (doc)->docDot = docDot;
					PdodDoc (docDot)->crefLock++;
					}
				}
			}
		}

	else  /* ! fHasFib */
		doc = DocCreateForeignFn(fn, fInput, stConverter, &stSubset);

	/* if above operation was aborted, just return */
	if (doc == docCancel || doc == docNil)
		return doc;

	/* get a subset of the doc */
	if (stSubset != NULL && *stSubset)
		SubsetDoc (doc, stSubset);

	Debug (vdbs.fCkDoc ? CkDoc (doc) : 0);

	return doc;

}



/* D O C  R E A D  F N  P P N F I B */
/*  Read the formatted document in fn the fib at *ppnFib into a new doc.
	On return *ppnFib is set to fib.pnNext.
	Return docNil in case of problems.
*/
/*  %%Function:DocReadFnPpnFib %%Owner:peterj  */

DocReadFnPpnFib (fn, ppnFib, psels)
int fn;
PN *ppnFib;
struct SELS *psels;

{
	int doc;
	int dk;
	int stcpMac, stcp;
	CP ccpDel;
	struct STTB **hsttb;
	struct DOD dod, ** hdod;
	struct CA caDest, caSrc;
	struct FIB fib;
	BOOL fSetDot = fFalse;

	/* avoid useless Inval()s */
	DisableInval();

	/* cause all reads of file to work in big chunks */
	EnablePreload(fn);

	if (psels != NULL)
		psels->ca.doc = docNil;

	/*  read the fib */
	FetchFib(fn, &fib, *ppnFib);

	/*  determine the type of the doc being read */
	if (fib.fDot)
		dk = dkDot;
	else  if (fib.fGlsy)
		dk = dkGlsy;
	else
		dk = dkDoc;    /* these are the only current cases */

	if ((doc = DocAlloc (dk, &dod)) == docNil)
		goto ErrRet0;

	/*  link to mother file if not glsy */
	if (*ppnFib == 0)
		dod.fn = fn;

	dod.fMayHavePic = fib.fHasPic;

	/*  Read in structures */

	/*  get the plcpcd */
	if (fib.fComplex)
		{
		if (!FReadClx(fn, &dod, &fib))
			goto ErrRet1;
		}
	else  if (!FInitPlcpcd(&dod, fn, ppnFib)) /* modifies *ppnFib */
		goto ErrRet1;

	/* footnote references */
	if (fib.cbPlcffndRef)
		{
		if (FNoHeap ( dod.hplcfrd =
				HplcReadPlcf(fn,fib.fcPlcffndRef,fib.cbPlcffndRef,cbFRD)))
			goto ErrRet1;
		}

	/* annotation references */
	if (fib.cbPlcfandRef)
		{
		if (FNoHeap ( dod.hplcatrd =
				HplcReadPlcf(fn,fib.fcPlcfandRef,fib.cbPlcfandRef,cbATRD)))
			goto ErrRet1;
		}

	/*  section table */
	if (fib.cbPlcfsed)
		{
		struct SED sed;
		int ised;
		if (FNoHeap ( dod.hplcsed =
				HplcReadPlcf(fn, fib.fcPlcfsed, fib.cbPlcfsed, cbSED)))
			goto ErrRet1;
		/* set the fn's in the section table */
		for ( ised = 0; ised < IMacPlc( dod.hplcsed ); ised++ )
			{
			GetPlc( dod.hplcsed, ised, &sed );
			sed.fn = fn;
			PutPlcLast( dod.hplcsed, ised, &sed );
			}
		}
	else
		{
		struct DOD *pdod = &dod;
/* kludgey way to create a sed with one entry, whose cpMac is same as
	dod.cpMac. */
		if (FNoHeap(HplcCreateEdc(&pdod, edcSed)))
			goto ErrRet1;
		FOpenPlc(dod.hplcsed, 1, -1);
		}

	/*  aldus structure */
	if (fib.cbPlcfsea)
		{
		if (FNoHeap ( dod.hplcsea =
				HplcReadPlcf(fn, fib.fcPlcfsea, fib.cbPlcfsea, cbSEA)))
			goto ErrRet1;
		dod.fSea = fTrue;
		}

	/*  page table */
	if (fib.cbPlcfpgd)
		{
		if (FNoHeap ( dod.hplcpgd =
				HplcReadPlcf(fn, fib.fcPlcfpgd, fib.cbPlcfpgd, cbPGD)))
			goto ErrRet1;
		Win((*dod.hplcpgd)->fMult = fTrue);
		}

	if (fib.cbPlcfphe)
		{
		if (FNoHeap( dod.hplcphe =
				HplcReadPlcf(fn, fib.fcPlcfphe, fib.cbPlcfphe, cbPHE)))
			goto ErrRet1;
		Win((*dod.hplcphe)->fMult = fTrue);
		}

	/*  fields */
	if (fib.cbPlcffldMom)
		{
		if (FNoHeap( dod.hplcfld =
				HplcReadPlcf(fn, fib.fcPlcffldMom, fib.cbPlcffldMom, cbFLD)))
			goto ErrRet1;
		else
			ResetFieldsHplcfld (dod.hplcfld);
		}

	/*  bookmarks */
	if (fib.cbSttbfbkmk)
		{  /* read in bookmark sttb & plc if there is any */
		if (FNoHeap ( dod.hsttbBkmk =
				HsttbReadSttbfFromFile(fn,fib.fcSttbfbkmk,fTrue,fFalse,0)))
			goto ErrRet1;
		if (FNoHeap ( dod.hplcbkf =
				HplcReadPlcf(fn, fib.fcPlcfbkf, fib.cbPlcfbkf, cbBKF)))
			goto ErrRet1;
		Win((*dod.hplcbkf)->fMult = fTrue);
		if (FNoHeap ( dod.hplcbkl =
				HplcReadPlcf(fn, fib.fcPlcfbkl, fib.cbPlcfbkl, cbBKL)))
			goto ErrRet1;
		Win((*dod.hplcbkl)->fMult = fTrue);
		}

	/*  glossary table.  Should only be in an fGlsy. */
	if (fib.cbSttbfglsy)
		{
		Assert (dod.fGlsy);
		if (FNoHeap ( dod.hsttbGlsy =
				HsttbReadSttbfFromFile(fn,fib.fcSttbfglsy,fTrue,fFalse,0)))
			goto ErrRet1;
		if (FNoHeap ( dod.hplcglsy =
				HplcReadPlcf(fn,fib.fcPlcfglsy,fib.cbPlcfglsy, 0)))
			goto ErrRet1;
		}

	/* If the document has a font table, load it, add its fonts
		to the master font table, and build a mapping from
		the documents ftc's to indices into the master font table */
	hsttb = hNil;
	if (fib.cbSttbfffn)
		{   /* Document has a font table; load it */
		if (FNoHeap( hsttb =
				HsttbReadSttbfFromFile(fn,fib.fcSttbfffn,fFalse,fFalse,0)))
			goto ErrRet1;
		}

		/* Add font table from doc to master font table do this even
			if the doc had none, to get the default mapping in place */
		{
		BOOL f = FAddDocFonts(hsttb, &dod);
		FreeHsttb(hsttb);
		if (!f)
			goto ErrRet1;
		}

	/*  style sheet */
	SetFnPos(fn, fib.fcStshf);
	ReadRgchFromFn(fn, &dod.stsh.cstcStd, sizeof(int));

	if (FNoHeap ( dod.stsh.hsttbName =
			HsttbReadSttbfFromFile(fn, fcNil,fTrue,fTrue,0)))
		goto ErrRet1;

	if (FNoHeap ( dod.stsh.hsttbChpx =
			HsttbReadSttbfFromFile(fn, fcNil,fTrue,fTrue,0)))
		goto ErrRet1;

	if (FNoHeap ( dod.stsh.hsttbPapx =
			HsttbReadSttbfFromFile(fn, fcNil,fTrue,fTrue,0)))
		goto ErrRet1;

	if (FNoHeap ( dod.stsh.hplestcp =
			HplReadPlf(fn, fcNil, cbESTCP)))
		goto ErrRet1;

	/* cleanup the hsttbPapx, so that papx.paph.fStyleDirty = fFalse. It may
		be fTrue because style changes that happened in the previous editing
		session. It is too expensive to clean at save time. */
	stcpMac = (*dod.stsh.hsttbPapx)->ibstMac;
	for (stcp = 0; stcp < stcpMac; stcp++)
		{
		CHAR HUGE *hpst = HpstFromSttb(dod.stsh.hsttbPapx, stcp);
		if (*hpst != 255 && *hpst >= cbPHE + 1)
			{
			bltbcx(0, LpLockHp(hpst+2), cbPHE);
			UnlockHp(hpst+2);
			}
		}

	if (FNoHeap ( dod.hsttbChpe = HsttbPropeFromStsh(&dod.stsh, fTrue)))
		goto ErrRet1;

	if (FNoHeap ( dod.hsttbPape = HsttbPropeFromStsh(&dod.stsh, fFalse)))
		goto ErrRet1;


	/* print environment */
	dod.fEnvDirty = TRUE;
	if (fib.cbPrEnv != 0)
		dod.fEnvDirty = FEnvDirty(fn, fib.fcPrEnv, fib.cbPrEnv);

	/*  Read document properties */
	SetFnPos(fn, fib.fcDop);
	ReadRgchFromFn(fn, &dod.dop, min (fib.cbDop, cbDOP));

	/*  associated strings table */
	if (fib.cbSttbfAssoc != 0)
		{
		if (FNoHeap ( dod.hsttbAssoc =
				HsttbReadSttbfFromFile(fn,fib.fcSttbfAssoc,fTrue,fFalse,0)))
			goto ErrRet1;
		else
			{
			Assert ((*dod.hsttbAssoc)->ibstMac == ibstAssocMax);
			}
		}

	/*  primary reading done, write the dod out to memory */
	/* This needs to be done now so DocCreateSub(), FReplaceCps(), etc.
		will work. */
	blt(&dod, PdodDoc(doc), cwDOD);

	if (dod.dop.fLockAtn && !FAuthor(doc)) /* has to be done after hsttbAssoc is stored into mpdodhdod */
		PdodDoc(doc)->fLockForEdit = fTrue;

	/*  footnote sub document */
	if (fib.ccpFtn)
		{
		int docFtn;
		struct PLC **hplcfnd;
		struct PLC **hplcfld;
		struct PLC **hplcpgd;

		if ((docFtn = DocCreateSub(doc, dkFtn)) == docNil)
			goto ErrRet2;

		/* copy text to ftn doc */
		if (!FReplaceCps(PcaSet(&caDest, docFtn, cp0, cp0),
				PcaSet(&caSrc, doc, fib.ccpText, fib.ccpText+fib.ccpFtn)))
			goto ErrRet2;

		/*  Read in the subdoc's plc */
		/*  must follow Truncate since plc does not include extra eop */
		if (FNoHeap( hplcfnd = PdodDoc(docFtn)->hplcfnd =
				HplcReadPlcf(fn, fib.fcPlcffndTxt, fib.cbPlcffndTxt, 0)))
			goto ErrRet2;
		Win((*hplcfnd)->fMult = fTrue);

		if (fib.cbPlcffldFtn)
			{
			if (FNoHeap( hplcfld = PdodDoc(docFtn)->hplcfld =
					HplcReadPlcf(fn, fib.fcPlcffldFtn, fib.cbPlcffldFtn, cbFLD)))
				goto ErrRet2;
			else
				ResetFieldsHplcfld (hplcfld);
			}

		if (fib.cbPlcfpgdFtn && FNoHeap( hplcpgd = PdodDoc(docFtn)->hplcpgd =
				HplcReadPlcf(fn, fib.fcPlcfpgdFtn, fib.cbPlcfpgdFtn, cbPGD)))
			goto ErrRet2;
		CorrectDodPlcs (docFtn, CpMac2Doc (docFtn));
	/* now get rid of extra chEop from DocCreateSub */
		if (!FDelete(PcaSetDcp(&caDest, docFtn, CpMacDocEdit(docFtn), ccpEop)))
			goto ErrRet2;
		}

	/*  header/footer sub document */
	if (fib.ccpHdd)
		{
		int docHdr;
		struct PLC **hplcfld;
		CP cpT = fib.ccpText+fib.ccpFtn;

		if ((docHdr = DocCreateSub(doc, dkHdr)) == docNil)
			goto ErrRet2;

		if (!FReplaceCps(PcaSet( &caDest, docHdr, cp0, cp0),
				PcaSet(&caSrc, doc, cpT, cpT+fib.ccpHdd)))
			goto ErrRet2;

		/*  Now read in the subdoc's plc */
		/*  must follow Truncate since plc does not include extra eop */
		if (FNoHeap( PdodDoc(docHdr)->hplchdd =
				HplcReadPlcf(fn, fib.fcPlcfhdd, fib.cbPlcfhdd, cbHDD)))
			goto ErrRet2;
		if (fib.cbPlcffldHdr)
			{
			if (FNoHeap( PdodDoc(docHdr)->hplcfld = hplcfld =
					HplcReadPlcf(fn, fib.fcPlcffldHdr, fib.cbPlcffldHdr, cbFLD)))
				goto ErrRet2;
			else
				ResetFieldsHplcfld (hplcfld);
			}
		CorrectDodPlcs (docHdr, CpMac2Doc (docHdr));
	/* now get rid of extra chEop from DocCreateSub */
		if (!FDelete (PcaSetDcp(&caDest, docHdr, CpMacDocEdit(docHdr), ccpEop)))
			goto ErrRet2;
		}

	/* macro sub document */
	if (fib.ccpMcr)
		{
		int docMcr;
		struct PLC **hplcfld;
		CP cpT = fib.ccpText + fib.ccpFtn + fib.ccpHdd;

		Assert(dod.fDot);

		if ((docMcr = DocCreateSub(doc, dkMcr)) == docNil)
			goto ErrRet2;

		if (!FReplaceCps(PcaSet(&caDest, docMcr, cp0, cp0),
				PcaSet(&caSrc, doc, cpT, cpT + fib.ccpMcr)))
			goto ErrRet2;

		if (FNoHeap(PdodDoc(docMcr)->hplcmcr =
				HplcReadPlcf(fn, fib.fcPlcmcr, fib.cbPlcmcr, cbMCR)))
			goto ErrRet2;

		fSetDot = fTrue;

		if (fib.cbSttbfmcr)
			{
			int imcrT2, imcrMacT2;

			SetFnPos(fn, fib.fcSttbfmcr);
			ReadRgchFromFn(fn, &imcrMacT2, cchINT);
			Assert(imcrMacT2 / 2 == (imcrMacT2 + 1) / 2);
			for (imcrT2 = 0; imcrT2 < imcrMacT2; imcrT2 += 2)
				{
				int bsy;
				struct CA ca;
				MCR mcr;
				char st [cchMaxSt];

				ReadRgchFromFn(fn, &st[0], 1);
				Assert(st[0] < cchMaxSyName-1);
				ReadRgchFromFn(fn, &st[1], (int)st[0]);

				CaFromIhdd(docMcr, imcrT2 / 2, &ca);
				if ((bsy = BsyAddCmd1(st, doc, imcrT2 / 2)) == bsyNil)
					{
					/* Chop plc so RemoveDotMacros() won't free the ones
						we haven't loaded yet. */
					(*PdodDoc(docMcr)->hplcmcr)->iMac = imcrT2 / 2;
					goto ErrRet2;
					}
				
				GetPlc( PdodDoc(docMcr)->hplcmcr, imcrT2 / 2, &mcr );
				mcr.bcm = bsy;

				PutPlcLast( PdodDoc(docMcr)->hplcmcr, 
						imcrT2 / 2, &mcr );

				ReadRgchFromFn(fn, &st[0], 1);
				ReadRgchFromFn(fn, &st[1], (int)st[0]);
				if (!FSetMenuHelp(bsy, st))
					{
					/* Chop plc so RemoveDotMacros() won't free the ones
						we haven't loaded yet. */
					(*PdodDoc(docMcr)->hplcmcr)->iMac = imcrT2 / 2 + 1;
					goto ErrRet2;
					}
				}
			}

		if (fib.cbPlcffldMcr)
			{
			if (FNoHeap( PdodDoc(docMcr)->hplcfld = hplcfld =
					HplcReadPlcf(fn, fib.fcPlcffldMcr, fib.cbPlcffldMcr, cbFLD)))
				goto ErrRet2;
			else
				ResetFieldsHplcfld (hplcfld);
			}

		CorrectDodPlcs (docMcr, CpMac2Doc (docMcr));
	/* now get rid of extra chEop from DocCreateSub */
		if (!FDelete (PcaSetDcp(&caDest, docMcr, CpMacDocEdit(docMcr), ccpEop)))
			goto ErrRet2;
		}

	/*  annotation sub document */
	if (fib.ccpAtn)
		{
		int docAtn;
		struct PLC **hplcfld;
		struct PLC **hplcfnd;
		CP cpT = fib.ccpText + fib.ccpFtn + fib.ccpHdd + fib.ccpMcr;

		if ((docAtn = DocCreateSub(doc, dkAtn)) == docNil)
			goto ErrRet2;

		/* copy text to atn doc */
		if (!FReplaceCps(PcaSet(&caDest, docAtn, cp0, cp0),
				PcaSet( &caSrc, doc, cpT, cpT + fib.ccpAtn)))
			goto ErrRet2;

		/*  Read in the subdoc's plc */
		/*  must follow Truncate since plc does not include extra eop */
		if (FNoHeap( hplcfnd = PdodDoc(docAtn)->hplcfnd =
				HplcReadPlcf(fn, fib.fcPlcfandTxt, fib.cbPlcfandTxt, 0)))
			goto ErrRet2;
		Win((*hplcfnd)->fMult = fTrue);

		if (fib.cbPlcffldAtn)
			{
			if (FNoHeap( hplcfld = PdodDoc(docAtn)->hplcfld =
					HplcReadPlcf(fn, fib.fcPlcffldAtn, fib.cbPlcffldAtn, cbFLD)))
				goto ErrRet2;
			else
				ResetFieldsHplcfld (hplcfld);
			}
		CorrectDodPlcs (docAtn, CpMac2Doc (docAtn));
	/* now get rid of extra chEop from DocCreateSub */
		if (!FDelete (PcaSetDcp(&caDest, docAtn, CpMacDocEdit(docAtn), ccpEop)))
			goto ErrRet2;
		}

	/*  command info */
	if (dod.fDot)
		{
		hdod = mpdochdod[doc];
		/* a DOT must have menu and keymap */
		if (fib.cbCmds > 0)
			{
			if (!FReadCmds(fn, fib.fcCmds, hdod))
				goto ErrRet2;
			}
		else
			{
			if (FNoHeap((*hdod)->hkmpUser = HkmpCreate(0, kmfPlain)))
				goto ErrRet2;
			(*(*hdod)->hkmpUser)->hkmpNext = vhkmpUser;
			if (FNoHeap((*hdod)->hmudUser = HmudInit(0)))
				goto ErrRet2;
			}
		}


	/* remove ftn/hdr text from main doc */
	if ((ccpDel = fib.ccpHdd + fib.ccpFtn + fib.ccpMcr + fib.ccpAtn) != 0)
		if (!FDelete(PcaSet(&caDest, doc, fib.ccpText-ccpEop, fib.ccpText+ccpDel)))
			goto ErrRet2;

	/* insert the hidden section mark at the end of the document */
	if (!FInsertHiddenSectionMark(doc))
		goto ErrRet2;

	if (psels != NULL && fib.cbWss)
		{
		struct WSS wss;
		SetWords(&wss, 0, cwWSS);
		SetFnPos(fn, fib.fcWss);
		ReadRgchFromFn(fn, &wss, min(fib.cbWss, cbWSS));
		wss.sels.doc = doc;
		*psels = wss.sels;
		}

	if (fib.nLocale != nLocaleDef && !PdodDoc(doc)->fLockForEdit)
		/* this file was written by a different language version of OPUS */
		{
		TranslateFields (doc);
		}

	/* fib version 23 has new PHE structure, therefore need to blow away
		old version plcpgds and plcphes. */
	if (fib.nFib < 23)
		{
		dod.fEnvDirty = fTrue;
		CheckPagEnv(doc, dod.dop.fPagHidden, dod.dop.fPagResults);
		}

	DisablePreload();
	EnableInval();
	return doc;

/* goto here for errors below the above blt */
ErrRet2:
	if (fib.cbSttbfmcr && fSetDot)
		RemoveDotMacros(doc);
	/*  get an up-to-date dod (we already copied it to the heap) */
	blt (PdodDoc (doc), &dod, cwDOD);

/* goto here for errors above the above blt */
ErrRet1:
	/*  dispose of the doc */
	DisposeDocPdod (doc, &dod);

ErrRet0:
	DisablePreload();
	EnableInval();
	return docNil;
}



/*  %%Function:FReadClx %%Owner:peterj  */

/* F  R E A D  C L X */
FReadClx(fn, pdod, pfib)
int     fn;
struct DOD     *pdod;
struct FIB     *pfib;
{
	struct PRC   **hprcAllocNew;
	int     iclxMax;
	int     iclxMac;
	int    (**hmpiclxcfgr)[];
	int     cb;
	char    clxt;
	int     cbGrpprl;
	struct FCB     *pfcb;
	struct PRC     **hprcSave;
	int     prm;
	int     iclx;
	int     cbPlcpcd;
	struct PLC  **hplcpcd = hNil;
	int     ipcdMac;
	int     ipcd;
	struct PCD     *ppcd;
	struct PRM     *pprm;
	struct MPRC    prc;
	struct	PCD	pcd;

	FreeUnusedPrcs();
	hprcAllocNew = 0;
	iclxMax = 10;
	iclxMac = 0;
	if ((hmpiclxcfgr = HAllocateCw(iclxMax)) == hNil)
		return fFalse;

	pfcb= *mpfnhfcb[fn];
	/* first we will loop, reading each clx whose type is clxtPrc,
		reading it's embedded PRC into local structure prc. We save
		the current head of the PRC chain (vhprc) and then call
		PrmFromPrc(). If on return, we find a new PRC at the beginning
		on the list, we remove it from the vhprc list and save it on a
		local list. Necessary since the new PRCs are purgeable since they
		are not yet referenced by a piece table. If we are short of heap
		space, PrmFromPrc() could cause some of the PRCs we have already
		read to be purged before the entire set is read and linked into the
		new piece table. After each call to PrmFromPrc() we add an entry
		to mpiclxcfgr, which maps a clx index into a transformed heap
		pointer. */
	SetFnPos(fn, pfib->fcClx);
	cb = pfib->cbClx;

	ReadRgchFromFn(fn, &clxt, 1);

	while (cb > 0 && clxt == clxtPrc)
		{
		SetBytes(&prc, 0, cbMaxGrpprl);
		ReadRgchFromFn(fn, &cbGrpprl, 2);
		prc.bprlMac = cbGrpprl;
		ReadRgchFromFn(fn, &prc.grpprl, cbGrpprl);
		hprcSave = vhprc;
		if ((prm = PrmFromPrc(&prc, 0)) == 0 || vmerr.fMemFail)
			goto MemErr;

		if (vhprc != hprcSave)
			{
			(**vhprc).hprcNext = hprcAllocNew;
			hprcAllocNew = vhprc;
			vhprc = hprcSave;
			}

		if (iclxMac == iclxMax)
			{
			iclxMax += 10;
			if (!FChngSizeHCw(hmpiclxcfgr, iclxMax, fFalse))
				goto MemErr;
			}
		iclx = iclxMac++;
		(**hmpiclxcfgr)[iclx] = ((struct PRM *)&prm)->cfgrPrc;
		ReadRgchFromFn(fn, &clxt, 1);
		cb -= cbGrpprl + 3;
		}

	/* now we read the piece table into the heap. */
	Assert(cb != 0 && clxt == clxtPlcpcd);

	ReadRgchFromFn(fn, &cbPlcpcd, 2);
	Assert(cbPlcpcd == cb - 3);

	if (FNoHeap(hplcpcd = HplcReadPlcf(fn, (**mpfnhfcb[fn]).fcPos,
			cbPlcpcd, cbPCD)))
		goto MemErr;

	/* since we're done with heap allocations, we can safely move
		the new non-supported PRC's back to the vhprc list. */
	AddToFrontVhprcList(hprcAllocNew);

	/* and we loop thru the piece entries. We set the fn in each
		entry and transform the iclx stored in the cfgrPrc field
		into a true cfgr. */
	ipcdMac = IMacPlc(hplcpcd);

	for (ipcd = 0; ipcd < ipcdMac; ipcd++)
		{
		struct PCD pcd;
		GetPlc(hplcpcd, ipcd, &pcd);
		pcd.fn = fn;
		pprm = (struct PRM *) &pcd.prm;
		if (pprm->fComplex)
			pprm->cfgrPrc = (**hmpiclxcfgr)[pprm->cfgrPrc];
		PutPlcLast(hplcpcd, ipcd, &pcd);
		}
	pdod->hplcpcd = hplcpcd;
	pdod->cpMac = CpMacPlc(hplcpcd);

	FreeH(hmpiclxcfgr);
	return(fTrue);

MemErr:
	FreeH(hmpiclxcfgr);
	FreeHplc(hplcpcd);
	AddToFrontVhprcList(hprcAllocNew);
	FreeUnusedPrcs();
	return(fFalse);
}


/*  %%Function:AddToFront %%Owner:peterj  */

/* A D D  T O  F R O N T  ... */
/* link the list whose head is hprc into the vhprc list. */
AddToFrontVhprcList(hprc)
struct PRC **hprc;
{
	struct PRC  **hprcT, *pprc;

	if (hprc == 0)
		return;
	hprcT = hprc;

	for (;;)
		{
		pprc = *hprcT;
		if (pprc->hprcNext == 0)
			break;
		hprcT = pprc->hprcNext;
		}
	(**hprcT).hprcNext = vhprc;
	vhprc = hprc;
}


/*  %%Function:HsttbPropeFromStsh %%Owner:peterj  */
/* H S T T B  P R O P E  F R O M  S T S H */
/* given a pointer to a style sheet, construct an STTB containing expanded
	CHPs or PAPs from the hsttbChpx or hsttbPapx referenced in the style
	sheet. When fChpe is true, a hsttbChpe is constructed and returned. When
	it is false a hsttbPape is constructed and returned. */
struct STTB **HsttbPropeFromStsh(pstsh, fChpe)
struct STSH *pstsh;
{
	struct STTB **hsttbPrope;
	int     cstcStd;
	int     cstcStdMinus;
	int     stcp;
	int     stcpMac;
	int     cbProp;
	int     stcCur;
	int     cch;
	char    rgbProp[cbPAP+1];
	char    rgBuffer[cchMaxSt]; /* for MakeChp/PapFromChpx/Papx... */

	if (FNoHeap(hsttbPrope = HsttbInit1(0, 0, 0, fTrue/*fExt*/,fTrue/*fStyRul*/)))
		return(0);
	cstcStd = pstsh->cstcStd;
	cstcStdMinus = (256 - cstcStd) & 0377;
	stcpMac = (**pstsh->hsttbChpx).ibstMac;
	Assert((**pstsh->hsttbChpx).ibstMac == (**pstsh->hsttbPapx).ibstMac);
	for (stcp = 0; stcp < stcpMac; stcp++)
		{
		if (vmerr.fMemFail)
			{
			FreeHsttb(hsttbPrope);
			return(hNil);
			}
		if (FStcpEntryIsNull(fChpe ? pstsh->hsttbChpx : pstsh->hsttbPapx,
				stcp))
			{
			rgbProp[0] = 0377;
			goto LAddProp;
			}
		stcCur = (stcp + cstcStdMinus) & 0377;
		if (fChpe)
			{
			SetBytes(rgbProp, 0, cbCHP);
			Assert(sizeof(struct PAP) < cchMaxSt);
			MakeChpFromChpxBaseChp(rgbProp, stcCur,
					(pstsh->hsttbChpx),*(pstsh->hplestcp),
					cstcStd, 0, rgBuffer);
			cbProp = cbCHP;
			}
		else
			{
			SetBytes(rgbProp, 0, cbPAP);
			MakePapFromPapxBasePap(rgbProp, stcCur,
					(pstsh->hsttbPapx), *(pstsh->hplestcp),
					cstcStd, 0, rgBuffer);
#ifdef MAC
			/* This is for Mac only.  They need this to be
				compatible with style sheets in the old format. */
			if (((struct PAP *)rgbProp)->brcp || ((struct PAP *)rgbProp)->brcl)
				{
				BrcsFromBrclBrcp(&((struct PAP *)rgbProp)->brcTop, ((struct PAP *)rgbProp)->brcl, ((struct
						PAP *)rgbProp)->brcp);
				((struct PAP *)rgbProp)->brcp = ((struct PAP *)rgbProp)->brcl = 0;
				}
#endif /* MAC */
			cbProp = cbPAP;
			}
		cch = CchNonZeroPrefix(rgbProp, cbProp);
		bltb(rgbProp, rgbProp + 1, cch);
		rgbProp[0] = cch;
LAddProp:
		if (IbstAddStToSttb(hsttbPrope, rgbProp) == ibstNil)
			{
			FreeHsttb(hsttbPrope);
			return(0);
			}
		}
	return (hsttbPrope);
}


/*  %%Function:MakeChpFromChpxBaseChp %%Owner:peterj  */
/* M A K E  C H P ... */
MakeChpFromChpxBaseChp(pchp, stc, hsttbChpx, pplestcp, cstcStd, cRecurse, chpx)
struct CHP *pchp;
int     stc;
struct STTB **hsttbChpx;
struct PLESTCP *pplestcp;
int     cstcStd;
int     cRecurse;
char    chpx[];  /* Must be big enough to hold pap or chpx */
{
	int stcp;
	int stcBase;
	struct ESTCP *pestcp;

	stcp = (stc + cstcStd) & 0377;
	cRecurse += 1;
	pestcp = &(pplestcp->dnstcp[stcp]);
	/* if, by some evil act, the based on chain is too long...break it */
	if (cRecurse >= cstcMaxBasedOn)
		pestcp->stcBase = stcStdMin;
	if ((stcBase = pestcp->stcBase) != stcStdMin)
		MakeChpFromChpxBaseChp(pchp, stcBase, hsttbChpx, pplestcp,
				cstcStd, cRecurse, chpx);
	else
		StandardChp(pchp);
	GetStFromSttb(hsttbChpx, stcp, chpx);
	if (chpx[0] != 255)
		ApplyChpxToChp(chpx, pchp);
	else
		{
		Assert(stcBase == stcNormal);
		/* use chpx as placeholder */
		MapStcStandard(stc, pchp, chpx);
		}
}


/*  %%Function:MakePapFromPapxBasePap %%Owner:peterj  */
/* M A K E  P A P ... */
MakePapFromPapxBasePap(ppap, stc, hsttbPapx, pplestcp, cstcStd, cRecurse, papx)
struct PAP *ppap;
int stc;
struct STTB **hsttbPapx;
struct PLESTCP *pplestcp;
int cstcStd;
int cRecurse;
char papx[];
{
	int     stcp;
	int     stcBase;
	char    *pch;
	int     cch;
	struct ESTCP *pestcp;

	stcp = (cstcStd + stc) & 0377;
	cRecurse += 1;
	pestcp = &(pplestcp->dnstcp[stcp]);
	/* if, by some evil act, the based on chain is too long...break it */
	if (cRecurse >= cstcMaxBasedOn)
		pestcp->stcBase = stcStdMin;
	if ((stcBase = pestcp->stcBase) != stcStdMin)
		MakePapFromPapxBasePap(ppap, stcBase, hsttbPapx, pplestcp,
				cstcStd, cRecurse, papx);
	else
		StandardPap(ppap);
	GetStFromSttb(hsttbPapx, stcp, papx);
	if (papx[0] != 255)
		{
		cch = papx[0];
		ppap->stc = papx[1];
		bltb(&papx[2], ((char *)&ppap->phe), cbPHE);
		cch -= cbPHE + 1;
		if (cch > 0)
			ApplyPrlSgc((char HUGE *)&papx[2+cbPHE], cch, ppap, sgcPap);
		}
	else
		{
		Assert(stcBase == stcNormal);
		/* use papx as placeholder */
		MapStcStandard(stc, papx, ppap);
		}
}


/*  %%Function:ApplyChpxToChp %%Owner:peterj  */
/* A P P L Y  C H P X  T O  C H P */
/* "apply" transformations expressed in the chpx to the chp.
This is a copy of the code in FetchCp that is inaccesible because
it has to be fast.
*/
ApplyChpxToChp(pchpx, pchp)
char    *pchpx;
struct CHP *pchp;
{
	struct CHP chpX;
	int cch = *pchpx++;
	SetWords(&chpX, 0, cwCHPBase);
	bltb(pchpx, &chpX, cch);
	*(int *)pchp ^= *(int *)&chpX;
	if (*(int *)&chpX & (maskFs | maskFSpec))
		{
		if (chpX.fsIco) pchp->ico = chpX.ico;
		if (chpX.fsFtc) pchp->ftc = chpX.ftc;
		if (chpX.fsKul) pchp->kul = chpX.kul;
		if (chpX.fsHps) pchp->hps = chpX.hps;
		if (chpX.fsPos) pchp->hpsPos = chpX.hpsPos;
		if (chpX.fsSpace) pchp->qpsSpace = chpX.qpsSpace;
		if (chpX.fSpec) pchp->fcPic = chpX.fcPic;
		}

}


/*  %%Function:ResetFieldsHplcfld %%Owner:peterj  */
/* R E S E T  F I E L D S  H P L C F L D */
/*  Reset the information in doc's fields which is session based.
	That information is fDiffer and bData.
*/

ResetFieldsHplcfld (hplcfld)
struct PLC **hplcfld;

{
	int ifld, ifldMac;
	struct FLD fld;

	if (hplcfld == hNil)
		return;

#ifdef SHOWFLDS
	CommSz (SzShared("ResetFieldsHplcfld: resetting fields\n\r"));
#endif /* SHOWFLDS */

	for ( ifld = 0, ifldMac = IMacPlc(hplcfld); ifld < ifldMac; ifld++ )
		{
		GetPlc(hplcfld, ifld, &fld);
		if (fld.ch == chFieldSeparate)
			fld.bData = bDataInval;
		else  if (fld.ch == chFieldEnd)
			fld.fDiffer = fFalse;
		PutPlcLast(hplcfld, ifld, &fld);
		}
}



/*  %%Function:FAddDocFonts %%Owner:peterj  */
/* F  A D D  D O C  F O N T S */
/* Specialized routine called from document creation process to build a
	document's font table.
	Given an hsttb of fonts (which may be null), and a pdod (which MUST NOT
	be a heap pointer), fill out the dod's hmpftcibstFont, ftcMac, and ftcMax
	fields according to the fonts in hsttb.  Returns fFalse if an allocation
	error prevented us from filling out the fields correctly, in which case
	the document should not be loaded. */

FAddDocFonts( hsttb, pdod )
struct STTB **hsttb;
struct DOD *pdod;
{
	extern struct STTB **vhsttbFont;
	int ibst;
	int ftc, ftcMacHsttb;
	char stFont[cbFfnLast];

	ftcMacHsttb = (hsttb == hNil) ? 0 : (*hsttb)->ibstMac;
	pdod->ftcMax = pdod->ftcMac = ftcMacHsttb + ftcMinUser;

	Assert( sizeof (IBSTFONT) == 1 );
	if (!FNoHeap(pdod->hmpftcibstFont = HAllocateCw( CwFromCch(pdod->ftcMax) )))
		{
/* Set up mapping from ftc in StandardChp to default font */

		(**pdod->hmpftcibstFont) [0] = ibstFontDefault; /* Tms Rmn */
		(**pdod->hmpftcibstFont) [1] = ibstFontDefault + 1; /* Symbol font */
		(**pdod->hmpftcibstFont) [2] = ibstFontDefault + 2; /* Helv */

/* Map other document ftc's to the master table, adding fonts if needed */

		for ( ftc = 0; ftc < ftcMacHsttb; ftc++ )
			{
			Assert( vhsttbFont != hNil );
			GetStFromSttb(hsttb, ftc, stFont);


/* make sure that old files have the new ffn size. init chs to ANSI. */
/* this will need to stay in if we want to read pre-release 40 files */
#define pffn	((struct FFN *) stFont)
			if (pffn->cbFfnM1 < CbFfnFromCchSzFfn( CchSz( pffn->szFfn )) - 1)
				{
#ifdef BRYANL
				CommSz(SzShared("Old font table format, increasing FFN size\r\n"));
#endif
				Assert( pffn->cbFfnM1 + 2 == CbFfnFromCchSzFfn( CchSz(pffn->szFfn)));
				stFont [pffn->cbFfnM1++] = ANSI_CHARSET;
				}
#undef pffn

			if ((ibst = IbstFindSzFfn( vhsttbFont, stFont)) == iNil)
				{   /* Font is not already in table -- add it */
				if ((ibst = IbstAddStToSttb( vhsttbFont, stFont )) == iNil)
					goto LFail;
				}

			(**pdod->hmpftcibstFont) [ftcMinUser + ftc] = ibst;
			}

		return fTrue;
		}

LFail:
	FreePh( &pdod->hmpftcibstFont );
	pdod->ftcMac = pdod->ftcMax = 0;
	return fFalse;
}



/*  %%Function:DocFromFn %%Owner:peterj  */
/* D O C  F R O M  F N */
/*  If there is a doc whose mother fn is fn, return it.
*/

DocFromFn (fn)
int fn;

{
	int doc;
	struct DOD **hdod;

	if (fn == fnNil)
		return docNil;

	for (doc = docMinNormal; doc < docMac; doc++)
		if ((hdod = mpdochdod [doc]) != hNil && (*hdod)->fn == fn)
			return doc;

	return docNil;
}



/*  %%Function:FHasFcbFromSt %%Owner:peterj  */
/* F  H A S  F C B  F R O M  S T */
int FHasFcbFromSt( stFile )
CHAR stFile[];
	{   /* Return TRUE if the passed stFile has an fn, FALSE otherwise.
		Note that under Windows we may know about more than one file with
	the same name. */
	int fn;
	struct FCB *pfcb, **hfcb;

	Assert(stFile[0] != 0);

	for (fn = 1 + !(vmerr.fScratchFileInit && !vmerr.fPostponeScratch); 
			fn < fnMac; fn++)
		{
		if (hfcb = mpfnhfcb[fn])
			{
			pfcb = *hfcb;
			if (FEqualSt( stFile, pfcb->stFile ))
				return fTrue;
			}
		}
	return fFalse;
}

/* F F I L E  O P E N  B Y  M A C R O  S T */
/* %%Function:FFileOpenByMacroSt %%Owner:peterj  */
BOOL FFileOpenByMacroSt(st)
CHAR st[];
{
	int stmM1;	/* stm number minus 1 */
	extern struct ELOF ** mpstmM1hELOF[];
	CHAR * sz;

	for (stmM1 = stmMinM1; stmM1 < stmMaxM1; stmM1++)
		{
		if (mpstmM1hELOF[stmM1] != hNil)
			{
			/* Use FNeNcRgch to compare st & sz, then check for equal lengths */
			if ((!FNeNcRgch(&st[1],
				(sz = ( *mpstmM1hELOF[stmM1] )->ofs.szFile),
				st[0])) && (sz[st[0]] == '\0'))
				{
				return fTrue;
				}
			}
		}
	return fFalse;
}


/*  %%Function:FnOpenSt %%Owner:peterj  */
/* F N   O P E N   S T */
int FnOpenSt(st, grpfOst, ofc, pfose )
char st[];      /* file name */
WORD grpfOst;   /* Collection of flags:
		fOstCreate     when true, indicates caller wishes to
					create the file -- name will then be
					RETURNED through st unless fOstNamed
					is true.
		fOstNamed      dispite fOstCreate, use passed name
		fOstFormatted  when true, file is normal formatted Word
					doc; when false, it's a text file.
					(used for create case only)
		fOstSearchPath when true, search for file on whole path
		fOstReportErr  when true, generate error msgs
		fOstReadOnly   open the file Read only
		fOstQuickRead  read in the 1st page only, don't bother with
					chpx and papx
int ofc;        /* open file code; can be ofcDoc, ofcDot, ofcTemp, ofcDMEnum */
int *pfose;     /* explaination of error return (undefined if successful) */
	{ /*
	DESCRIPTION:
	Open/Create a file.
	RETURNS:
	fn if file successfully opened, fnNil if cannot open file for some reason:
	bad file type, out of heap space, disk I/O error, or file not found.
*/
	int fn;
	struct FCB *pfcb;
	int fCreate = grpfOst & fOstCreate;
	int fFormatted = grpfOst & fOstFormatted;
	int fErrors = grpfOst & fOstReportErr;

	if (pfose) *pfose = foseCantOpenAny;

	Assert(fCreate || st[0] != 0);
	Assert(!fCreate || !(grpfOst & fOstReadOnly));
#ifdef DEBUG
	if (grpfOst & fOstQuickRead)
		Assert(!fCreate);
	if (fCreate)
		Assert(!(grpfOst & fOstQuickRead));
#endif

#ifdef DBGDOCOPEN
	CommSzSt (SzShared("FnOpenSt: "), st);
#endif /* DBGDOCOPEN */

	if (vcelof)
		/* Macro has one or more files open, don't create or open them */
		{
		if (FFileOpenByMacroSt(st))
			{
			if (pfose)
				*pfose = (grpfOst & fOstNamed) ? foseNoCreate : foseNoAccess;
			return fnNil;
			}
		}

	if (!fCreate && ((fn = FnFromOnlineSt( st )) != fnNil))
		/* Note (bl): at some point, we were planning to free up the
			old fn in this case.  However,  I can't remember why.
			If anyone remembers, let me know.  Doing the full-blown
			version of that would be difficult, because of the
			possibility that some doc has a piece pointing to it */
		{
		if (grpfOst & fOstReadOnly)
			PfcbFn (fn)->fReadOnly = fTrue;
		return fn;
		}

#ifdef DEBUG
	if (fCreate && (grpfOst & fOstNamed))
		Assert(!FnFromOnlineSt(st));
#endif /* DEBUG */

	if (vmerr.fFnFull)
		{
		/* OK OK, you can't open a file when fFnFull is set, but
			you can create one; the extra fn after fFnFull is to
			allow saving, which will very likely free fns */
		if (!fCreate)
			return (fnNil);
		for (fn = 1; fn < fnMax && mpfnhfcb[fn] != hNil; fn++)
			;
		if (fn == fnMax)
			return(fnNil);
		}

	/* add an entry to the fcb table for this file.  If can't expand
		the fcb (out of heap space), return fnNil */
	if ((fn = IAllocCls(clsFCB, cwFCB)) == fnNil)
		goto ErrRet;

	FreezeHp();
	pfcb = *mpfnhfcb[fn];
	pfcb->osfn = osfnNil;
	if (grpfOst & fOstReadOnly)
		pfcb->fReadOnly = fTrue;
	/* pfcb->fHasFib = fFalse; */
	switch (ofc)
		{
	case ofcDoc:
		pfcb->fDoc = fTrue;
		break;
	case ofcTemp:
		pfcb->fTemp = fTrue;
		break;
	case ofcDMEnum:
		pfcb->fDMEnum = fTrue;
		break;
		}

	/* NOTE: fnScratch no longer uses FnOpenSt, created in FInitFn() */
	Assert(fn != fnScratch);

	/* Create the file if necessary */
	/* if fOstNamed, the file will be created in FAccessFn */
	if (fCreate && !(grpfOst & fOstNamed) && !FCreateStFile( st ))
		{
		if (pfose) *pfose = foseNoCreate;
		goto ErrRet;
		}

	/* Set the file's name & open it  */

	MeltHp();
	if (!FNameFn( fn, st ))
		goto ErrRet;

	if (!FAccessFn( fn, grpfOst ))
		{
		if (pfose)
			*pfose = (grpfOst & fOstNamed) ? foseNoCreate : foseNoAccess;
		goto ErrRet;
		}

	/* Enable preload for fn */
	EnablePreload(fn);

	if (fCreate)
		{
		if (fFormatted)
			{ /* Formatted file; write FIB */
			union 
				{
				struct FIB fib;
				char rgch [cbFileHeader];
				} fibx;
			struct PLCBTE  **hplcbteChp;
			struct PLCBTE  **hplcbtePap;

			if ((hplcbteChp = HplcInit(cbBTE, 1, (CP) cbFileHeader, fTrue /* ext rgFoo */)) == hNil ||
					(hplcbtePap = HplcInit(cbBTE, 1, (CP) cbFileHeader, fTrue /* ext rgFoo */)) == hNil)
				{
				FreeHplc (hplcbteChp);
				goto ErrRet1;
				}
			FreezeHp();
			pfcb = *mpfnhfcb[fn];
			pfcb->hplcbteChp = hplcbteChp;
			pfcb->hplcbtePap = hplcbtePap;
			pfcb->ifcChpHint = pfcb->ifcPapHint1 =
					pfcb->ifcPapHint2 = ifcFkpNil;
			pfcb->fHasFib = fTrue;
			pfcb->nFib = nFibCurrent;

#ifdef DISABLE_PJ   /* I don't think this is really necessary, wasted write to
			file which is expensive. */
					SetBytes(&fibx, 0, cbFileHeader);
			fibx.fib.wIdent = wMagic;
			fibx.fib.cbMac = fibx.fib.fcMac = cbFileHeader;
			fibx.fib.nFib = nFibCurrent;  /* version stamps */
			fibx.fib.nFibBack = nFibBackCurrent;
			fibx.fib.nProduct = nProductCurrent;
			fibx.fib.nLocale = nLocaleDef;
			FcAppendRgchToFn(fn, (char *) &fibx, cbFileHeader);
#endif /* DISABLE_PJ */
			MeltHp();
			}
		}
	else
		{
		/* we are opening a preexisting file */
		struct PLCBTE **hplcbteChp = hNil;
		struct PLCBTE **hplcbtePap = hNil;
		struct FIB fib;
		struct FCB *pfcb = PfcbFn(fn);

		if ((pfcb->cbMac = pfcb->fcMacFile = FcMacFn(fn)) >= cbSector
				&& FNativeFormat(HpchGetPn(fn, 0),
				(grpfOst&(fOstNativeOnly|fOstQuickRead)) ? 
				fFalse : fTrue/*fErrors*/))
			/* native format file */
			{
			FreezeHp();
			pfcb = *mpfnhfcb[fn];
			pfcb->fHasFib = fTrue;

			/* now we can read in (and convert) the fib */
			FetchFib(fn, &fib, 0);

			pfcb->nFib = fib.nFib;
			pfcb->fCompound = fib.pnNext != pn0;
			pfcb->cbMac = fib.cbMac;
			pfcb->pnXLimit = PnWhoseFcGEFc(fib.fcMac);
			Assert (pfcb->fCompound ||
					pfcb->cbMac == fib.fcSttbfAssoc + fib.cbSttbfAssoc);
			MeltHp();


			if (!(grpfOst & fOstQuickRead))
				{
				if (FNoHeap ( hplcbteChp =
						HplcReadPlcf(fn, fib.fcPlcfbteChpx,
						fib.cbPlcfbteChpx, cbBTE)))
					goto ErrRet1;

				if (fib.nFib >= 22 && !fib.fComplex && 
						IMacPlc(hplcbteChp) < fib.cpnBteChp)
					if (!FFillMissingBtePns(fn, hplcbteChp, &fib, 0, fTrue /* CHP */))
						goto ErrRet1;

				if (FNoHeap ( hplcbtePap =
						HplcReadPlcf(fn, fib.fcPlcfbtePapx,
						fib.cbPlcfbtePapx, cbBTE)))
					{
					FreeHplc (hplcbteChp);
					goto ErrRet1;
					}

				if (fib.nFib >= 22 && !fib.fComplex && 
						IMacPlc(hplcbtePap) < fib.cpnBtePap)
					if (!FFillMissingBtePns(fn, hplcbtePap, &fib, 0, fFalse /* PAP */))
						goto ErrRet1;

				FreezeHp();
				pfcb = *mpfnhfcb[fn];
				pfcb->hplcbteChp = hplcbteChp;
				pfcb->hplcbtePap = hplcbtePap;
				MeltHp();
				}
			}
		else  if (grpfOst & fOstNativeOnly)
			{
			if (pfose) *pfose = foseBadFile;
			goto ErrRet1;
			}
		else
			{
			/* File is not formatted, must find cbMac and
				pnXLimit for that file. */
			pfcb = *mpfnhfcb[fn];
			if (pfcb->cbMac == fcNil)
				{
				if (pfose) *pfose = foseNoAccess;
				goto ErrRet1;
				}
			pfcb->pnXLimit = PnWhoseFcGEFc(pfcb->cbMac);
			}
		}

	DisablePreload();
	Debug( vdbs.fCkFn && !fCreate && !fOstQuickRead ? CkFn(fn) : 0 );
	return(fn);

ErrRet1:
	DisablePreload();
ErrRet:
	MeltHpToZero();
	if (fn != fnNil)
		DeleteFn (fn, fFalse/*fDelete*/);
	return (fnNil);
}       /* end FnOpenSt */


/*  %%Function:FNativeFormat %%Owner:peterj  */
/* F  N A T I V E  F O R M A T */
/*  Determine if we can read this file as a native format file.  If we cannot,
	FnOpenStFile will open it as a "plain text" file.  If it has a fib but
	we cannot read it (i.e., old format), warn the user (iff fErrors).
*/
BOOL FNativeFormat(hpfib, fErrors)
struct FIB HUGE *hpfib;
BOOL fErrors;

{
	int nFib = hpfib->nFib;
	int eid = eidNil;

	if (hpfib->wIdent == wMagicPmWord)
		{
		eid = eidPmWordFile;
		goto LNative;
		}

	if (hpfib->wIdent == wMagic)
		{
LNative:
		if (nFib < nFibMinDoc)
			eid = eidOldFib;

		else  if (hpfib->fDot && nFib < nFibMinDot)
			eid = eidOldDot;

		else  if (hpfib->nFibBack > nFibCurrent)
			eid = eidFutureFib;

		else  if (nFib <= 18 && hpfib->fComplex)
			eid = eidOldFastSavedFib;

		else if ((hpfib->nProduct&0xfffe) == 0x2050 && hpfib->nFib == 30)
			/* files with bogus font info were written by early versions
			   of release 40. (0x2050==01.00.4000). */
			eid = eidOldFib;

		}
	else
		return fFalse;

	if (eid != eidNil)
		{
		if (fErrors)
			{
			/* terminate CBT, since they can't handle this error */
			/* LOWORD(lParam) should be non-zero for error return */
			if (vhwndCBT && IsWindow(vhwndCBT))
				{
				SendMessage(vhwndCBT, WM_CBTTERM, vhInstance,
					MAKELONG(0, ctrOldFileFormat));
				}
			ErrorEid(eid, "FNativeFormat");
			}
		}

	return eid == eidNil || eid == eidPmWordFile;
}


int NIncrFromL(l)
long l;
{
	if (l < 1500)
		return 25;
	else  if (l < 15000)
		return 10;
	else  if (l < 75000)
		return 5;
	else
		return 1;
}


/*  %%Function:HplcReadPlcf %%Owner:peterj  */
/* H P L C  R E A D  P L C F */
struct PLC **HplcReadPlcf(fn, fcFirst, cbTotal, cb)
int     fn;
FC      fcFirst;
uns     cbTotal;
int     cb;
{
	int     iMax;
	struct PLC     **hplc, *pplc;

	if (cbTotal < sizeof(CP))
		{
		Assert(fFalse);
		/* safety, we have files out there with this true! */
		return hNil;
		}
	iMax = (long)((long) cbTotal - sizeof(CP)) / (long)(cb + sizeof(CP));
	if (vmerr.fMemFail || (hplc = HplcInit(cb, iMax, cp0, fTrue /* ext rgFoo */)) == hNil)
		return(hNil);
	if ((*hplc)->iMax < iMax)
		{
		FreeHplc(hplc);
		return hNil;
		}
	ReadIntoExtPlc(hplc, fn, fcFirst, cbTotal);
	(*hplc)->iMac = iMax;
	(*hplc)->icpAdjust = iMax+1;
	return(hplc);
}


/*  %%Function:ReadIntoExtPlc %%Owner:peterj  */
ReadIntoExtPlc(hplc, fn, fcFirst, cbTotal)
struct PLC **hplc;
int fn;
FC fcFirst;
uns cbTotal;
{
	struct PLC *pplc;
	CP HUGE *hprgcp;

	if (fcFirst != fcNil)
		SetFnPos(fn, fcFirst);
	pplc = *hplc;
	Assert(pplc->fExternal);
	hprgcp = (CP HUGE *)HpOfHq(pplc->hqplce);
	Assert(CbOfHq(pplc->hqplce) >= cbTotal);
	Win(StartUMeas(umScanFnForBytes));
	ReadHprgchFromFn(fn, hprgcp, cbTotal);
	Win(StopUMeas(umScanFnForBytes));
	return(hplc);
}


/*  %%Function:HplReadPlf %%Owner:peterj  */
/* H P L  R E A D  P L F */
HplReadPlf(fn, fcFirst, cb)
int     fn;
FC      fcFirst;
int     cb;
{
	int     ifooMac;
	struct PL      **hpl, *ppl;

	if (fcFirst != fcNil)
		SetFnPos(fn, fcFirst);
	ReadRgchFromFn(fn, &ifooMac, sizeof(int));
	if ((hpl = HplInit(cb, ifooMac)) == hNil || vmerr.fMemFail)
		return(hNil);
	ppl = *hpl;
	ppl->iMac = ifooMac;
	ReadRgchFromFn(fn, &ppl->rgbHead, cb*ifooMac);
	return(hpl);
}


/*  %%Function:HsttbReadSttbFromFile %%Owner:peterj  */
/* H S T T B  R E A D  S T T B F  F R O M  F I L E */
HsttbReadSttbfFromFile(fn, fc, fExternal, fStyleRules, cbExtra)
int     fn;
FC      fc;
BOOL    fExternal, fStyleRules;
int	cbExtra;
{
	uns     bMax;
	struct STTB    **hsttb;
	uns     b;
	int     ibst;
	int     cch;
	char    st[256];
	char	rgchExtra[64];

	Assert(cbExtra <= 64);
	if (fc != fcNil)
		SetFnPos(fn, fc);
	ReadRgchFromFn(fn, &bMax, sizeof(int));
	Assert(bMax > 0);
	if (bMax <= 0 || bMax >= PfcbFn(fn)->cbMac) /* probably encounter some corrupted file, bag out */
		return hNil;

	bMax -= sizeof(int);
	if ((hsttb = HsttbInit1(CwFromCch(bMax), 128, cbExtra, fExternal, fStyleRules)) == hNil &&
			(hsttb = HsttbInit1(CwFromCch(bMax), 0, cbExtra, fExternal, fStyleRules)) == hNil)
		return (hNil);
	(*hsttb)->fNoShrink = fTrue;
	FStretchSttbCb(hsttb, bMax + 200);
	for (b = ibst = 0; b < bMax; b += cch + 1 + cbExtra)
		{
		ReadRgchFromFn(fn, st, sizeof(char));
		if ((cch = st[0]) == 255 && fStyleRules)
			cch = 0;
		else
			ReadRgchFromFn(fn, st + 1, cch);
		if (cbExtra != 0)
			ReadRgchFromFn(fn, rgchExtra, cbExtra);

		if (!FInsStInSttb1(hsttb, ibst++, st, rgchExtra))
			{
			FreeHsttb(hsttb);
			return(hNil);
			}
		}
	CloseUpSttb(hsttb);
	return (hsttb);
}


/*  %%Function:PprlPrmFS %%Owner:peterj  */
/* P G R P P R L  F R O M  P R M  */
/* given a prm return a pointer to the list of sprm (pgrpprl) and return
	the length of the grpprl */
/* duplicate version local to this module for swap tuning purposes */
PprlPrmFS(prm, pcb, grpprl)
struct PRM prm;
int *pcb;
char *grpprl; /* user provided buffer for unloading sprm from piece table */
{
	char *pgrpprl;
	struct PRC *pprc;

	if (prm.fComplex)
		{
		pprc = *HprcFromPprmComplex(&prm);
		pgrpprl = &(pprc->grpprl[0]);
		*pcb =  pprc->bprlMac;
		}
	else
		{
		if (prm.prm == 0)
			{
			/* PRM is null */
			pgrpprl = 0;
			*pcb = 0;
			}
		else
			{
			/* PRM contains exactly 1 prl */
			grpprl[0] = prm.sprm;
			grpprl[1] = prm.val;
			pgrpprl = grpprl;
			*pcb = dnsprm[prm.sprm].cch;
			}
		}
	return pgrpprl;
}


/*  %%Function:FInsertHiddenSectionMark %%Owner:peterj  */
/* F  I N S E R T  H I D D D E N  S E C T I O N  M A R K */
/* used by create and save to insert the chSect at CpMacDoc and adjust
the plc's accordingly */
FInsertHiddenSectionMark(doc)
int doc;
{
	struct DOD *pdod;
	int ipcdMac;
	CP cpMacOrig;
	int cb, cbSprm, cbSepSprm;
	char *pchSep, *pgrpprl;
	struct CA ca;
	struct PCD pcd;
	struct PLC **hplcpcd;
	struct PLC **hplcsed;
	struct SEP sepStandard;
	char grpprl[2];
	char grpprlSep[cbMaxGrpprl];

/* copy any section sprms applied to last piece of document to grpprlSep*/
	pdod = PdodDoc(doc);
	ipcdMac = IMacPlc(hplcpcd = pdod->hplcpcd);
	GetPlc(hplcpcd, ipcdMac - 1, &pcd);
	pgrpprl = PprlPrmFS(pcd.prm, &cb, grpprl);
	pchSep = grpprlSep;
	while (cb > 0)
		{
		cbSprm = CchPrl(pgrpprl);
		if (dnsprm[*pgrpprl].sgc == sgcSep)
			pchSep = bltbyte(pgrpprl, pchSep, cbSprm);
		pgrpprl += cbSprm;
		cb -= cbSprm;
		}
	BuildPcd(&pcd, fnInsert, fcSpecEop);

/* now add hidden section piece. if we were able to find any section sprms
	in the piece that was the last piece, we copy them into this piece's prm.*/
	if (cbSepSprm = pchSep - grpprlSep)
		pcd.prm = PrmAppend(pcd.prm, grpprlSep, cbSepSprm);
	if (!FInsertInPlc(hplcpcd, ipcdMac, cpMacOrig = CpPlc(hplcpcd, ipcdMac), &pcd))
		return fFalse;

/* now add piece for document sentinel. */
	pcd.prm = 0;
	if (!FInsertInPlc(hplcpcd, ipcdMac + 1, cpMacOrig + ccpEop, &pcd))
		return fFalse;
	CorrectPlcCpMac(hplcpcd, PdodDoc(doc)->cpMac += 2*ccpEop);
/* now we need to set the rgcp[iMac] of all the hplcs except the hplcsed
	equal to CpMacDoc and the rgcp[iMac] of the hplcsed equal to CpMac1Doc.
	call CorrectDodPlcs to do fixups. It knows CpMacDoc + ccpEop == CpMac1Doc */
	CorrectDodPlcs(doc, CpMac2Doc(doc));
	if ((hplcsed = PdodDoc(doc)->hplcsed) != hNil && !FSectLimAtCp(doc, CpMac1Doc(doc)))
		{
		struct SED sed;
		sed.fUnk = fFalse;
		sed.fn = fnScratch;
		sed.fcSepx = fcNil;
		if (!FInsertInPlc(hplcsed, IMacPlc(hplcsed), CpMac1Doc(doc), &sed))
			return fFalse;
		}
	return fTrue;
}











