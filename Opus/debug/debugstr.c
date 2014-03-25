/* debugstr.c */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "disp.h"
#include "props.h"
#include "prm.h"
#include "file.h"
#include "doc.h"
#include "format.h"
#include "ch.h"
#include "field.h"
#include "print.h"
#include "screen.h"
#include "dde.h"
#include "debug.h"
#include "fkp.h"
#include "outline.h"
#include "message.h"

extern struct WWD       **hwwdCur;
extern struct WWD       **mpwwhwwd[];
extern struct MWD			**mpmwhmwd[];
extern int              wwCur;
extern int              wwMac, mwMac;
extern struct SCC       *vpsccBelow;
extern struct SCC       vsccAbove;
extern struct STTB  **vhsttbFont;
extern int              *pwMax;
extern struct ESPRM     dnsprm[];
extern char             rgchPLFDECharProps[];
extern char             rgchPLFDEParaProps[];
extern int              docGlsy;
extern struct DOD       **mpdochdod[];
extern struct CA        caPara;
extern int              docMac;
extern struct FCB       **mpfnhfcb[];
extern int              fnMac;
extern char             (**vhgrpchr)[];
extern int              vbchrMac;
extern int              vbchrMax;
extern struct FLI       vfli;
extern int              vdocTemp;
extern int              vdocStsh;
extern struct FKPD      vfkpdPap;
extern struct SPX       mpiwspxPap[];
extern CP               cpFirstNotInval;
extern CP               cpLimNotInval;
extern int              vdocScratch;
extern int              docGlobalDot;
extern CHAR HUGE        *vhpchFetch;
extern int              vccpFetch;
extern struct CHP       vchpFetch;
extern struct PAP       vpapFetch;
extern int              vdocFetch;
extern CP               vcpFetch;
extern struct PRSU      vprsu;
extern struct PREF      vpref;
extern int              docDde;

int HUGE *HpbstFirstSttb();

#ifdef DEBUG
char rgch1[6] = "11111";
char rgch3[6] = "33333";
char rgch4[6] = "44444";
char rgch5[6] = "55555";
char rgch10[6] = "AAAAA";
char stABC[] = "\0abcdefghij";
char stTest1[] = "1111333344445555AAAA";
char stTest2[] = "11111333334444455555AAAAA";
extern                  vjnRare;
extern                  vimark;
extern struct DBS       vdbs;


/* C K   H H H */
/* %%Function:CkHhh %%Owner:BRADV */
CkHhh(hhh, cbMinBlock)
struct HH **hhh;
uns cbMinBlock;
	{ /*
	DESCRIPTION:
	Check size of an hh.
*/
#ifdef REVIEW  /* REVIEW iftime: old heap stuff (rp) */
	int iAssert;
	uns cbBlock;
	int *pwHeap;

	iAssert = 1;
	Assert (!(((int)hhh) & 1));
	iAssert = 2;
	Assert (hhh >= rgfgr && hhh < pfgrMac);
	iAssert = 3;
	Assert (!(((int)*hhh) & 1));
	iAssert = 4;
	Assert (*hhh >= phhFirst && *hhh < phhMac);
	cbMinBlock += cbMinBlock&1;     /* round up to even */
	cbBlock = CwOfPfgr(hhh) * sizeof(int) - sizeof(int);
	iAssert = 5;
	Assert(cbBlock >= cbMinBlock
			&& cbBlock < cbMinBlock + sizeof(struct HH));
	if (cbBlock != cbMinBlock)
		{
		for (pwHeap = ((int *)(*hhh)) + cbMinBlock / sizeof(int);
				pwHeap < ((int *)(*hhh)) + cbBlock / sizeof(int);
				pwHeap++)
			{
			iAssert = 6;
			Assert (*pwHeap == wUnusedHeapWord);
			}
		}

#endif
}       /* end CkHhh */


/* C K   H S T T B */
/* %%Function:CkHsttb %%Owner:BRADV */
CkHsttb(hsttb, cbMin)
struct STTB **hsttb;
int cbMin;
	{ /*
	DESCRIPTION:
	Check consistency of an sttb.
*/
	int iAssert;
	uns ibst;
	int bst, bstPrev, bstPrevPlus1;
	int cst;
	struct STTB *psttb;
	int cchSt;
	int HUGE *hprgbst;

	if (!hsttb) return;
	psttb = *hsttb;
	if (psttb->ibstMac > psttb->ibstMax)
		{
		Assert(fFalse);
		return;
		}
	CkHhh(hsttb, psttb->fExternal ? cwSTTBBase * sizeof(int) + 4 :
			cwSTTBBase * sizeof(int) + psttb->bMax);

	hprgbst = HpbstFirstSttb(hsttb);
	bst = psttb->ibstMax * sizeof(int);
	if (psttb->ibstMac)
		{
		bstPrevPlus1 = 0x8000;
		iAssert = 1;
		for (cst = 0; cst < psttb->ibstMac; cst++)
			{
			ibst = IbstFromBst(hprgbst,
					psttb->ibstMac,
					bstPrevPlus1,
					bst);
			bstPrevPlus1 = bst + 1;
			cchSt = ((char HUGE *)hprgbst)[hprgbst[ibst]];
			bst = hprgbst[ibst]
					+ (cchSt == 255 && psttb->fStyleRules ? 0 : cchSt) + 1
					+ psttb->cbExtra;
			}
		}
	iAssert = 2;
	Assert(bst == psttb->bMax);

}       /* end CkHsttb */


/* I B S T   F R O M   B S T */
/* %%Function:IbstFromBst %%Owner:BRADV */
IbstFromBst(hpbst, ibstMac, bstPrevPlus1, bst)
int HUGE *hpbst;
int ibstMac;
int bstPrevPlus1;
int bst;
	{ /*
	DESCRIPTION:
	Do a lookup by value of an ibst.
*/
	int iAssert;
	int fbstFound = fFalse;
	int ibst, ibstFound;

	for (ibst = 0; ibst < ibstMac; ibst++, hpbst++)
		{
		iAssert = 1;
		Assert (*hpbst < bstPrevPlus1 || *hpbst >= bst);
		/* assert that (*hpbst == bst) ==> !fbstFound */
		iAssert = 2;
		Assert (*hpbst != bst || !fbstFound);
		if (*hpbst == bst)
			{
			ibstFound = ibst;
			fbstFound = fTrue;
			}
		}
	return(ibstFound);

}       /* end IbstFromBst */


/* C K   H P L   F O O */
/* %%Function:CkHplFoo %%Owner:BRADV */
CkHplFoo(hpl, cbEntry)
struct PL **hpl;
uns cbEntry;
	{ /*
	DESCRIPTION:
	Check consistency of a pl against structure specific parameters.
*/
	int iAssert;
	struct PL *ppl;

	if (!hpl) return;
	ppl = *hpl;
	CkHhh(hpl, cbPLBase + (ppl->iMax) * ppl->cb);
	iAssert = 1;
	Assert (ppl->cb == cbEntry);
	iAssert = 2;
	Assert (ppl->iMac <= ppl->iMax);

}       /* end CkHplFoo */


/* C K   H P L C */
/* %%Function:CkHplc %%Owner:BRADV */
CkHplc(hplc, fiMacCk, fMult)
struct PLC **hplc;
BOOL fiMacCk, fMult;
	{ /*
	DESCRIPTION:
	Check consistency of a plc.
*/
	int iAssert;
	int i;

	if (!hplc) return;
			CkHhh(hplc, cbPLCBase     /* REVIEW iftime: fExternal (rp) */
	+ ((*hplc)->iMax) * sizeof(CP) + ((*hplc)->iMax - 1) * (*hplc)->cb);

	iAssert = 1;
	Assert (IMacPlc( hplc ) < (*hplc)->iMax);

	for (i = 0; i < IMacPlc( hplc ) - !fiMacCk; ++i)
		{
		iAssert = 2;
		Assert(CpPlc(hplc,  i) < CpPlc(hplc, i + 1) ||
				(fMult && CpPlc(hplc,  i) == CpPlc(hplc, i + 1)));
		}
}       /* end CkHplc */


/* C K   H P L C   F O O */
/* %%Function:CkHplcFoo %%Owner:BRADV */
CkHplcFoo(hplc, cbEntry, fiMacCk)
struct PLC **hplc;
uns cbEntry;
int fiMacCk;
	{ /*
	DESCRIPTION:
	Check consistency of a plc against structure specific parameters.
*/
	if (!hplc) return;
	CkHplc(hplc, fiMacCk, (*hplc)->fMult);
	Assert ((*hplc)->cb == cbEntry);
	if (fiMacCk) Assert(IMacPlc( hplc ) > 0);
}       /* end CkHplcFoo */


/* %%Function:CkHplcMult  %%Owner:BRADV */
CkHplcMult (hplc, cbEntry, fiMacCk)
struct PLC **hplc;
int cbEntry;
BOOL fiMacCk;
{
	if (!hplc) return;
	CkHplc(hplc, fiMacCk, fTrue);
	Assert ((*hplc)->cb == cbEntry);
	if (fiMacCk) Assert(IMacPlc (hplc) > 0);
}       /* end CkHplcMult */



/* C K   W W S */
/* checks all wws */
/* %%Function:CkWws %%Owner:BRADV */
CkWws()
{
	int ww;
	if (!vdbs.fCkWw) return;
	for (ww = 0; ww < wwMac; ww++)
		CkWw(ww);

}       /* end CkWws */


/* C K   W W */
/* check consistency of a ww */
/* %%Function:CkWw %%Owner:BRADV */
CkWw(ww)
int ww;
{
	int iAssert;
	struct WWD *pwwd, *pwwdOther;
	struct MWD *pmwd;
	struct PLCEDL **hplcedl;
	int dl;
	int ypTop;
	CP cpLim;
	int wwDisp;
	struct WWD *pwwdMain, *pwwdFtn;
	int docMain, docFtn;
	struct EDL edl;
	int wwOther;
	int irgdr;

	
#ifdef REVIEW /* iftime */
	if (!vdbs.fCkWw) return;
	iAssert = 1;
	Assert (ww >= 0 && ww < wwMac);
	if (!mpwwhwwd[ww]) return;
	pwwd = PwwdWw(ww);
	pmwd = PmwdMw(pwwd->mw);
	if (ww >= wwDocMin)
		{
		iAssert++;
		Assert( pwwd->grpfvisi.flm == FlmDisplayFromFlags( 
				pwwd->fDisplayAsPrint,
				pwwd->fPageView, vpref.fDraftView ));
		}

	iAssert++;
	Assert(pwwd->hpldrBack == hNil);

	CkHhh(mpwwhwwd[ww], cwWWD * sizeof (int) );

	if (pmwd->fSplit)
		{
		iAssert++;
		wwOther = pwwd->fLower ? pmwd->wwUpper : pmwd->wwLower;
		Assert(mpwwhwwd[wwOther]);
		pwwdOther = *mpwwhwwd[wwOther];
		iAssert++;
		Assert(PmwdMw(pwwdOther->mw)->fSplit);
		iAssert++;
		Assert(pwwd->fLower != pwwdOther->fLower);
		}

	if (ww >= wwDocMin) /* do the following only for long WWD's */
		{
		iAssert++;
		Assert(pwwd->brgdr == offset (WWD, rgdr));
		for (irgdr = 0; irgdr < pwwd->idrMac; irgdr++)
			{
			iAssert++;
			Assert(pwwd->rgdr[irgdr].hplcedl);
			}

/* #ifdef SHOWCK */
		if (vdbs.fReports)
			CommSzCrLf(SzShared("Check hplcedl "));
/* #endif  */
		
		/* The following asserts can be made because if
		these values are converted to xa's they cannot
		overflow integer bounds */
 
		iAssert++;
		AssertIn(pwwd->xwMac, 0, 2000);
		iAssert++;
		AssertIn(pwwd->ywMac, 0, 2000);
		iAssert++;
		AssertIn(pwwd->xwMin, 0, pwwd->xwMac);
		iAssert++;
		AssertIn(pwwd->ywMin, 0, pwwd->ywMac);
		
		for (irgdr = 0; irgdr < pwwd->idrMac; irgdr++)
			{
			CkHplcFoo(pwwd->rgdr[irgdr].hplcedl, sizeof(struct EDL), fTrue);
			hplcedl = pwwd->rgdr[irgdr].hplcedl;
			iAssert++;
	 		Assert((*hplcedl)->dlMac == IMacPlc( hplcedl )); 	  
		   iAssert++;
			Assert (pwwd->rgdr[irgdr].dypAbove >= 0);
			iAssert++;
			GetPlc( hplcedl, 0, &edl );
			Assert (pwwd->rgdr[irgdr].dypAbove < edl.dyp );
			iAssert++;
			Assert (pwwd->rgdr[irgdr].dypAbove >= 0);
	   	ypTop = pwwd->ywMin - pwwd->rgdr[irgdr].dypAbove;
			cpLim = pwwd->rgdr[irgdr].cpFirst;
			vdbs.cdlInval = 0;
			for (dl = 0; dl < IMacPlc( hplcedl ); dl++ )
				{
				GetPlc( hplcedl, dl, &edl );
#ifdef REVIEW		 /* review iftime */
				if (!edl.fValid)
					{
					vdbs.cdlInval++;
					continue;
					}
#endif
				if (edl.ypTop - edl.dyp >= pwwd->ywFirstInval &&
					edl.ypTop < pwwd->ywLimInval)
					{	
					vdbs.cdlInval++;
					continue;
					}
				if (vdbs.cdlInval) continue;
				iAssert++;
				Assert(edl.ypTop - edl.dyp == ypTop);	
				iAssert++;
				Assert(CpPlc( hplcedl, dl ) == cpLim);
				cpLim = CpPlc( hplcedl, dl ) + edl.dcp;
				ypTop += edl.dyp;					
/* Make sure only part of last dl extends beyond ypMac */
		  	 	iAssert++;
	  	 		GetPlc( hplcedl, dl - 1, &edl );
		 		Assert(vdbs.cdlInval != 0
		 			|| (ypTop >= pwwd->ywMac
		 			&& ypTop - edl.dyp < pwwd->ywMac));
				}
			}				
		/* make sure we are in the wwDisp chain of the doc */
			for ( wwDisp = wwNil; (ww = WwDisp(pmwd->doc,wwDisp,fFalse))
				!= wwNil && ww != wwDisp; )
				;
			iAssert++;
			Assert (ww == wwDisp);
	}
#endif
}    /* end CkWw */	  

/* C K   M W S */
/* checks all mws */
CkMws()
{
	int ww;
	if (!vdbs.fCkWw) return;
	for (ww = 0; ww < wwMac; ww++)
		CkMw((*mpwwhwwd[ww])->mw);
}

/* C K   M W */
/* check consistency of an mw */
CkMw(mw)
int mw;
{
	int iAssert;
	struct MWD *pmwd;
	int wwDisp;

#ifdef REVIEW /* iftime */
	if (!vdbs.fCkWw) return;
	iAssert = 1;	
	Assert (mw >= 0 && mw < mwMac);
	if (!mpmwhmwd[mw]) return;
   pmwd = PmwdMw(mw);

	if (pmwd->fSplit)
	 	{
		iAssert = 2;
		Assert(pmwd->wwUpper && !pmwd->wwLower);
		}
	else
		{
		iAssert = 3;
		Assert(pmwd->wwUpper && pmwd->wwLower);
		}
	iAssert = 4;
	Assert(!pmwd->wwLower || pmwd->fSplit);
	iAssert = 5;
	Assert(!pmwd->fActive || pmwd->wwUpper);
	iAssert = 6;
	Assert(!(pmwd->wwRuler == pmwd->wwUpper) || !(pmwd->wwRuler == pmwd->wwLower));
	iAssert = 7;
	Assert(pmwd->dxp > 0);  /* width must be positive */
	iAssert = 8;
	Assert(pmwd->dyp > 0);  /*	height must be positive */
 
	CkHhh(mpmwhmwd[mw], cwMWD * sizeof (int) );

	/* The following asserts can be made because if
	these values are converted to xa's they cannot
	overflow integer bounds */
	iAssert = 9;
	AssertIn(pmwd->xp, 0, 2000);
	iAssert = 10;
	AssertIn(pmwd->yp, 0, 2000);
	iAssert = 11;
	Assert(mpdochdod[pmwd->doc]);

#endif 
} /* end CkMw */


/* C K  D O C S */
/* checks all docs */
/* %%Function:CkDocs %%Owner:BRADV */
CkDocs(fAbortOK)
BOOL fAbortOK;
{
	int doc;
	BOOL fUntitledFound = fFalse;

	if (!vdbs.fCkDoc) return;
	Assert (docGlsy == docNil ||
			(docGlsy >= docMinNormal
			&& docGlsy < docMac
			&& mpdochdod[docGlsy] != hNil));
	Assert (docDde == docNil ||
			(docDde >= docMinNormal
			&& docDde < docMac
			&& mpdochdod[docDde] != hNil));
	Assert (vdocTemp == docNil ||
			(vdocTemp >= docMinNormal
			&& vdocTemp < docMac
			&& mpdochdod[vdocTemp] != hNil));
	Assert (docGlobalDot == docNil ||
			(docGlobalDot >= docMinNormal
			&& docGlobalDot < docMac
			&& mpdochdod[docGlobalDot] != hNil));
	Assert (vdocScratch == docNil ||
			(vdocScratch >= docMinNormal
			&& vdocScratch < docMac
			&& mpdochdod[vdocScratch] != hNil));
	for (doc = 0; doc < docMac && (!fAbortOK || !FMsgPresent(mtyIdle)); doc++)
		if (mpdochdod [doc] != hNil)
			CkDoc2(doc, fAbortOK);

}       /* end CkDocs */


/* C K  D O C */
/* check one dod */
/* %%Function:CkDoc %%Owner:BRADV */
CkDoc(doc)
{
	CkDoc2(doc, fFalse);
}


/* C K  D O C 2 */
/* check one dod */
/* %%Function:CkDoc2 %%Owner:BRADV */
CkDoc2(doc, fAbortOK)
BOOL fAbortOK;
{
	int iAssert;
	int ftc, ibst;
	struct DOD *pdod;
	struct WWD *pwwd;
	struct PLC **hplcpcd;
	struct PLC *pplcpcd;
	struct PCD *ppcd;
	struct FCB **hfcb;
	struct PLCBTE *pplcbtePap;
	struct PLCBTE  **hplcbteChp, **hplcbtePap;
	CP cp = cp0;
	int ipcd;
	int ww;
	int fn;
	int cref, cwwdod;
	int ibteMacChp;
	int ibteMacPap;
	int docT = docNil;
	FC rgFcMin[fnMax];
	FC rgFcMacChp[fnMax];
	FC rgFcMacPap[fnMax];
	struct PCD pcd;


	if (!vdbs.fCkDoc) return;
	iAssert = 0;
	Assert (doc >= 0 && doc < docMac);
	if (!mpdochdod[doc]) return;

#ifdef SHOWCK
	if (vdbs.fReports)
		CommSzNum(SzShared("Start checking doc : "), doc);
#endif /* SHOWCK */

	if (vdbs.fCkPaph && FIsRealDoc(doc))
		CkPaph (doc, fAbortOK);  /* heap movement!! */

	pdod = PdodDoc (doc);

	CkHhh(mpdochdod[doc], pdod->fShort ? cbDODShort:
			sizeof(struct DOD));
#ifdef REVIEW	/* REVIEW iftime: Update to new display structure (rp) */
	/* verify number of ww's in wwDisp linked list
	is equal to the number of windows pointing to the doc */
	for (cref = 0, ww = wwDocMin; ww < wwMac; ww++)
		{
		if (mpwwhwwd[ww])
			{
			cref += ((*mpwwhwwd[ww])->doc == doc);
			}
		}
	for (cwwdod = 0, ww = pdod->wwDisp; ww != wwNil;
			ww = (*mpwwhwwd[ww])->wwDisp, cwwdod++)
		{
		iAssert = 2;
		Assert (ww >= wwDocMin && ww < wwMac);
		iAssert = 3;
		Assert (mpwwhwwd[ww] != hNil);
		}
	iAssert = 4;
	Assert (cref == cwwdod);
#endif	/* REVIEW iftime */

	/* check that the type is legal (see the grand union in doc.h). */
	iAssert = 5;
	switch (pdod->dk)
		{
	case dkDot:
		Assert (pdod->fFormatted);
		Assert (pdod->hkmpUser != hNil);
		Assert (pdod->hmudUser != hNil);
		Assert (pdod->docGlsy == docNil
				|| PdodDoc (pdod->docGlsy)->fGlsy);
		break;
	case dkDoc:
		Assert (pdod->docDot == docNil
				|| PdodDoc (pdod->docDot)->fDot);
		break;
	case dkGlsy:
		Assert (pdod->fFormatted);
		Assert (pdod->doc != docNil
				&& PdodDoc (pdod->doc)->fDot);
		break;
	case dkSDoc:
		Assert (doc == vdocTemp);
		break; /* pdod->doc may not be valid */
	case dkFtn:
	case dkHdr:
	case dkAtn:
	case dkDispHdr:
		Assert (pdod->fFormatted);
		Assert (pdod->doc != docNil && !PdodDoc (pdod->doc)->fShort);
		break;
	case dkMcr:
		Assert(pdod->fFormatted);
		Assert(pdod->doc != docNil
				&& PdodDoc (pdod->doc)->fDot);
		break;
	default:
		Assert (fFalse);
		}
/* fShort have no sub docs */
	if (!pdod->fShort)
		{
		if (pdod->docHdr)
			{
			iAssert = 8;
			Assert(mpdochdod[pdod->docHdr]);
			iAssert = 9;
			Assert((*mpdochdod[pdod->docHdr])->fHdr);
			Assert((*mpdochdod[pdod->docHdr])->doc == doc);
			CkDoc(pdod->docHdr);
			}
		if (pdod->docFtn)
			{
			iAssert = 9;
			Assert(mpdochdod[pdod->docFtn]);
			iAssert = 10;
			Assert((*mpdochdod[pdod->docFtn])->fFtn);
			Assert((*mpdochdod[pdod->docFtn])->doc == doc);
			CkDoc(pdod->docFtn);
			}
		if (pdod->docAtn)
			{
			iAssert = 11;
			Assert(mpdochdod[pdod->docAtn]);
			iAssert = 12;
			Assert((*mpdochdod[pdod->docAtn])->fAtn);
			Assert((*mpdochdod[pdod->docAtn])->doc == doc);
			CkDoc(pdod->docAtn);
			}
		if (pdod->fDot && pdod->docGlsy)
			{
			iAssert = 20;
			Assert(mpdochdod[pdod->docGlsy]);
			iAssert = 21;
			Assert((*mpdochdod[pdod->docGlsy])->fGlsy);
			Assert((*mpdochdod[pdod->docGlsy])->doc == doc);
			CkDoc(pdod->docGlsy);
			}
		}
	else
		{
		if (pdod->fHdr)
			{
			iAssert = 12;
			Assert(pdod->hplchdd);
#ifdef SHOWCK
			if (vdbs.fReports)
				CommSzCrLf(SzShared("Check hplchdd "));
#endif /* SHOWCK */
			CkHplcFoo(pdod->hplchdd, 0, fTrue);
			iAssert = 13;
			Assert((*mpdochdod[pdod->doc])->docHdr == doc);
			}
		if (pdod->fFtn)
			{
			iAssert = 14;
#ifdef SHOWCK
			if (vdbs.fReports)
				CommSzCrLf(SzShared("Check hplcfnd "));
#endif /* SHOWCK */
			Assert(pdod->hplcfnd);
			CkHplcFoo(pdod->hplcfnd, 0, fTrue);
			iAssert = 15;
			Assert((*mpdochdod[pdod->doc])->docFtn == doc);
			}
		if (pdod->fAtn)
			{
			iAssert = 16;
			Assert(pdod->hplcfnd);
#ifdef SHOWCK
			if (vdbs.fReports)
				CommSzCrLf(SzShared("Check hplcatnd "));
#endif /* SHOWCK */
			CkHplcFoo(pdod->hplcfnd, 0, fTrue);
			iAssert = 17;
			Assert((*mpdochdod[pdod->doc])->docAtn == doc);
			}
		if (pdod->fDispHdr)
			{ /* docHdrDisp */
			iAssert = 15;
			docT = docNil;
			Assert(PdodDoc(pdod->doc)->fMother);
			Assert(PdodDoc(pdod->doc)->docHdr != docNil);
			}
		}

	/* Check the PLCPGD */
	/* make sure we don't overflow the rgFcMin, rgFcMac tables allocated
	on the local frame */
	iAssert = 40;
	Assert(fnMac <= fnMax);
	for (fn = 0; fn < fnMac; fn++)
		{
		rgFcMin[fn] = fc0;
		rgFcMacChp[fn] = (FC)cpMax;
		rgFcMacPap[fn] = (FC)cpMax;
		if ((hfcb = mpfnhfcb[fn]) == hNil || !(*hfcb)->fHasFib)
			continue;
		ibteMacChp = IMacPlc(hplcbteChp = (*hfcb)->hplcbteChp);
		ibteMacPap = IMacPlc(hplcbtePap = (*hfcb)->hplcbtePap);
		rgFcMin[fn] = CpMax(CpPlc(hplcbteChp, 0),
				CpPlc(hplcbtePap, 0));
		rgFcMacChp[fn] = CpMacPlc(hplcbteChp);
		rgFcMacPap[fn] = CpMacPlc(hplcbtePap);
		}
	rgFcMacChp[fnScratch] = PfcbFn(fnScratch)->cbMac;
	rgFcMacPap[fnScratch] = PfcbFn(fnScratch)->cbMac;
	iAssert = 41;
	Assert(pdod->hplcpcd);
	iAssert = 42;
#ifdef SHOWCK
	if (vdbs.fReports)
		CommSzCrLf(SzShared("Check hplcpcd "));
#endif /* SHOWCK */
	CkHplcFoo(pdod->hplcpcd, sizeof(struct PCD), fTrue);
	hplcpcd = pdod->hplcpcd;
	iAssert = 43;
	Assert(CpPlc(hplcpcd, 0) == cp0);
	iAssert = 44;
	Assert(CpMacPlc(hplcpcd) == pdod->cpMac);
	for (ipcd = 0; ipcd < IMacPlc(hplcpcd); ipcd++)
		{
		if (fAbortOK && FMsgPresent(mtyIdle))
			return;
		pdod = PdodDoc(doc);
		GetPlc(pdod->hplcpcd, ipcd, &pcd);
		fn = pcd.fn;
		if ((pdod->fShort || pdod->fn == fnNil || !pdod->fFormatted)
				&& doc >= docMinNormal && doc != docGlsy
				&& ipcd == IMacPlc(hplcpcd) - 1)
			{
			iAssert = 45;
			/*Assert(fn == fnNil);  */
			goto LCkPrm;
			}
		if (fn == fnMac)
			{
			iAssert = 46;
			Assert(pcd.fc == fcSpecEop ||
					pcd.fc == fcSpecNoRoom ||
					pcd.fc < cchInsertMax);
			goto LCkPrm;
			}
		iAssert = 47;
		Assert(fn >= 0 && fn < fnMac);
		if (fn == fnInsert)
			goto LCkPrm;
		iAssert = 48;
		Assert(hfcb = mpfnhfcb[fn]);
		iAssert = 49;
		Assert(pcd.fc + CpPlc(hplcpcd, ipcd + 1)
				- CpPlc(hplcpcd, ipcd) <= (*hfcb)->cbMac);
		iAssert = 50;
		Assert(doc == vdocTemp || pcd.fc >= rgFcMin[fn]);
		iAssert = 51;
		Assert(doc == vdocTemp || pcd.fc < rgFcMacChp[fn]);
		iAssert = 52;
		/* REVIEW iftime: Is this assert valid? (rp) */
		/* Assert(doc == vdocTemp 
				|| pcd.fc < rgFcMacPap[fn]
				|| !pcd.fNoParaLastValid
				|| pcd.fNoParaLast); */
		if ((*hfcb)->fHasFib && pcd.fNoParaLast
				&& (fn != fnScratch || pcd.fc < vfkpdPap.fcFirst))
			{
			hplcbtePap = (*hfcb)->hplcbtePap;
			iAssert = 53;
			/* not true in quicksave docs */
			/*Assert(pcd.fc < ((*hplcbtePap)->rgfc[IInPlc(hplcbtePap,
					pcd.fc) + 1] - 1));*/
			}
LCkPrm:
		cp = CpPlc(hplcpcd, ipcd);
		/* prm != prmNil ==> fFormatted || doc < docMinNormal */
		iAssert = 54;
		Assert(pcd.prm == prmNil || pdod->fFormatted
				|| doc < docMinNormal);
		CkPrm(pcd.prm);
		}

#ifdef SHOWCK
	if (vdbs.fReports)
		CommSzCrLf(SzShared("Check hplcfld "));
#endif /* SHOWCK */
	CkHplcFoo(pdod->hplcfld, sizeof(struct FLD), fFalse);
	Assert (pdod->hplcfld == hNil || 
			CpMacPlc (pdod->hplcfld) == CpMac2Doc(doc));

	if (!pdod->fShort)
		{
		iAssert = 55;
		CkBookmarks (pdod->hsttbBkmk, pdod->hplcbkf, pdod->hplcbkl,
				CpMac2Doc(doc));
		iAssert = 56;
		Assert(pdod->fn >= 0 && pdod->fn < fnMac && 
				pdod->fn != fnScratch);
		iAssert = 57;
		Assert(pdod->fn == fnNil || mpfnhfcb[pdod->fn] != hNil);
		iAssert = 58;
#ifdef SHOWCK
		if (vdbs.fReports)
			CommSzCrLf(SzShared("Check hplcsed "));
#endif /* SHOWCK */
		Assert(pdod->hplcsed != hNil);
		iAssert = 59;
		CkHplcFoo(pdod->hplcsed, sizeof(struct SED), fTrue);
		iAssert = 60;
		if (!pdod->fFormatted)
			{
		/* dod may be reset to unformatted by UNDO, but hplc created
		are not destroyed but should be empty */
			iAssert = 66;
			if (pdod->hplcfrd)
				Assert(IMacPlc(pdod->hplcfrd) == 0);
			if (pdod->hplcatrd)
				Assert(IMacPlc(pdod->hplcatrd) == 0);
			}
		iAssert = 61;
#ifdef SHOWCK
		if (vdbs.fReports)
			CommSzCrLf(SzShared("Check hplcpgd "));
#endif /* SHOWCK */
		CkHplcMult(pdod->hplcpgd, sizeof(struct PGD), fFalse);
		iAssert = 62;
		Assert(!pdod->fGlsy || pdod->hplcpgd == hNil);
		iAssert = 65;
#ifdef SHOWCK
		if (vdbs.fReports)
			CommSzCrLf(SzShared("Check hplcfrd "));
#endif /* SHOWCK */
		CkHplcFoo(pdod->hplcfrd, sizeof(struct FRD), fFalse);
		iAssert = 67;
		/* pdod->fGlsy ==> fFormatted */
		Assert(!pdod->fGlsy || pdod->fFormatted);
		iAssert = 68;
/*                 Assert ((doc == docGlsy) == (pdod->hsttbGlsy != hNil)); */
		if (pdod->fGlsy)
			{
			struct DOD * pdodDot = PdodDoc (pdod->doc);
			Assert (pdod->doc != docNil);
			Assert (pdodDot->docGlsy == doc);
			Assert (pdodDot->fDot);
			Assert (pdod->hsttbGlsy != hNil &&
					pdod->hplcglsy != hNil);
#ifdef SHOWCK
			if (vdbs.fReports)
				CommSzCrLf(SzShared("Check hplcglsy "));
#endif /* SHOWCK */
			CkHplcFoo(pdod->hplcglsy, 0, fTrue);
			CkHsttb(pdod->hsttbGlsy, 0);
			Assert ((*pdod->hsttbGlsy)->ibstMac + 1
					== (*pdod->hplcglsy)->iMac);

			}
		iAssert = 73;
		Assert(!pdod->fGlsy || pdod->hplcpad == hNil);
		iAssert = 74;
#ifdef SHOWCK
		if (vdbs.fReports)
			CommSzCrLf(SzShared("Check hplcpad "));
#endif /* SHOWCK */
		CkHplcFoo(pdod->hplcpad, sizeof(struct PAD), fFalse);
		iAssert = 75;
		CkPstsh(&pdod->stsh);
		iAssert = 76;
		CkHsttb(pdod->hsttbChpe, 0);
		iAssert = 77;
		CkHsttb(pdod->hsttbPape, 0);
		iAssert = 78;
		CkPdop(&pdod->dop);
		iAssert = 79;
		CkHsttb(pdod->hsttbAssoc, 0);
		if (pdod->hsttbAssoc)
			Assert ((*pdod->hsttbAssoc)->ibstMac == ibstAssocMax);
		iAssert = 80;
		Assert(pdod->ftcMac > 0); /* we use ftcMac-1 in places */
		iAssert = 81;
		for (ftc = 0; ftc < pdod->ftcMac; ftc++)
			{
			ibst = (**pdod->hmpftcibstFont)[ftc];
			Assert(ibst >= 0);
			Assert(ibst < (*vhsttbFont)->ibstMac);
			}
		iAssert = 82;
		if (pdod->hplcpad)
			CkPlcpad(doc);
		}

#ifdef SHOWCK
	if (vdbs.fReports)
		CommSzNum(SzShared("End checking doc : "), doc);
#endif /* SHOWCK */


}       /* end CkDoc */


/* C K  P L C P A D */
/* %%Function:CkPlcpad %%Owner:BRADV */
CkPlcpad(doc)
{{
#ifdef BOGUS
	int iAssert;
	int ipad, ipadMac;
	CP cp;
	int ww;
	int fOutline = fFalse;
	struct DOD *pdod = PdodDoc(doc);
	struct PLC **hplcpad = pdod->hplcpad;
	struct PAD padT, pad;

	for (ww = pdod->wwDisp; ww != wwNil; ww = PwwdWw(ww)->wwDisp)
		{
		if (PwwdWw(ww)->fOutline)
			{
			fOutline = fTrue;
			break;
			}
		}
	if (pdod->fOutlineDirty)
		fOutline = fFalse;

	ipadMac = IMacPlc(hplcpad);

	iAssert = 1;
	Assert(CpPlc(hplcpad, 0) == cp0);
	for (ipad = 0; ipad < ipadMac; ipad++)
		{
		GetPlc(hplcpad, ipad, &pad);
		cp = CpPlc( hplcpad, ipad );
		if (pad.fUnk1)
			continue;
		if (!fOutline)
			{
			CachePara(doc, CpPlc(hplcpad, ipad + 1) - 1);
			iAssert = 2;
			Assert(caPara.cpLim == CpPlc(hplcpad, ipad + 1));
			continue;
			}
		CachePara(doc, cp);
		iAssert = 3;
		Assert(caPara.cpFirst == cp);
		iAssert = 4;
		Assert(caPara.cpLim == CpPlc(hplcpad, ipad + 1));
		if (!pad.fBody)
			{
			iAssert = 5;
			Assert(pad.lvl < 9);
			iAssert = 6;
			Assert(pad.stc == ((stcLevLast - pad.lvl) & 0xFF));
			}
		else
			{
			iAssert = 7;
			Assert(pad.stc < stcLevMin || pad.stc > stcLevLast);
			}
#ifndef JR
		UpdatePad(&padT);
		iAssert = 8;
		Assert(pad.fBody == padT.fBody);
		iAssert = 9;
		Assert(pad.lvl == padT.lvl);
#endif /* NOT JR */
		}
	iAssert = 10;
	Assert(CpMacPlc(hplcpad) == CpMacDoc(doc));
#endif	/* BOGUS */
}}


/* C K   P S T S H */
/* checks a style sheet */
/* %%Function:CkPstsh %%Owner:BRADV */
CkPstsh(pstsh)
struct STSH *pstsh;
{
	int ibst, ibstMac; 
	struct STTB *psttb; 

	/* REVIEW iftime: can I check cstcStd?  (no idea who "I" is - rp) */
	CkHsttb(pstsh->hsttbName, 0);
	CkHsttb(pstsh->hsttbChpx, 0);
	CkHsttb(pstsh->hsttbPapx, 0);
	psttb = *pstsh->hsttbPapx;
	ibstMac = psttb->ibstMac;
 	for (ibst = 0; ibst < ibstMac; ibst++)
		{
		char rgbResult[257]; 
		char HUGE *hpst; 
		int cbPrl;
		int cch; 
		char *pprl; 
		hpst = HpstFromSttb(pstsh->hsttbPapx, ibst);
		cch = *hpst + 1; 
		if (cch != 256)
			{
			bltbh(hpst, (char HUGE *)rgbResult, cch);

			cbPrl = *hpst - cbPHE - 1;	 
			pprl = rgbResult + cbPHE + 2; 
		
			if (cbPrl > 0)
				{
				char *pprlLim = pprl + cbPrl;
				int cbPrlActual = 0; 
				while (pprl < pprlLim)
					{
					pprl += (cch = CchPrl(pprl)); 
					cbPrlActual += cch; 
					}
				Assert(cbPrl == cbPrlActual); 
				}
			}
		}

	CkHplFoo(pstsh->hplestcp, sizeof(struct ESTCP));
	/* REVIEW iftime: Is there anything to check inside of each estcp? (rp) */

}       /* end CkPstsh */


/* C K   P D O P */
/* checks a document properties structure */
/* %%Function:CkPdop %%Owner:BRADV */
CkPdop(pdop)
struct DOP *pdop;
{
#ifdef TOTALYBOGUS
	Assert (pdop->dyaTop <= pdop->dyaBottom);
	Assert (pdop->dxaLeft <= pdop->dxaRight);
#endif
	/* REVIEW iftime: how can I check everything else in a dop? (rp) */

}       /* end CkPdop */


/* C K   F L I */
/* checks vfli and grpchr */
/* %%Function:CkVfli %%Owner:BRADV */
NATIVE CkVfli()     /* makes CkVfli visible to hand NATIVE */
{{
	int     iAssert;
	struct CHR *pchr;
	int ich;
	int xp;
	BOOL fFormula = fFalse;

	if (!vdbs.fCkFli) return;
	if (vfli.ichMac == 0) return;

	pchr = *vhgrpchr;
	for (;;)
		{
		if (pchr->chrm == chrmEnd) break;
		iAssert = 28;
		Assert (pchr->chrm == chrmChp ||
				pchr->chrm == chrmFormula ||
				pchr->chrm == chrmTab ||
				pchr->chrm == chrmVanish ||
				pchr->chrm == chrmDisplayField ||
				pchr->chrm == chrmFormatGroup);
		if (pchr->chrm == chrmFormula) fFormula = fTrue;
		iAssert = 0;
		/*  = because chrmVanish may preceed ichMac */
		Assert (vfli.fSplatBreak || pchr->ich <= vfli.ichMac);
		(char *)pchr += pchr->chrm;
		}
	iAssert = 1;
	Assert (vfli.doc >= 0 && vfli.doc < docMac);
	iAssert = 2;
	Assert (mpdochdod[vfli.doc]);
	iAssert = 3;
	Assert (vfli.cpMin >= cp0);
	iAssert = 4;
	Assert (vfli.cpMin < vfli.cpMac);
	iAssert = 5;
	Assert (vfli.cpMac <= (*mpdochdod[vfli.doc])->cpMac);
	if (vfli.fSplatBreak)
		{
#ifdef BOGUS        /* fails if there's an EOL above followed by vanished text */
		iAssert = 6;
		Assert (vfli.cpMac == vfli.cpMin + 1);
#endif
		}
	else
		{
		iAssert = 8;
		Assert (vfli.ichMac > 0);
		iAssert = 9;
		xp = vfli.xpLeft;
		for (ich = 0; ich < vfli.ichMac; ich++)
			xp += vfli.rgdxp[ich];
		iAssert = 10;
/*
*  REVIEW iftime -- when in print mode, xpLeft in screen units but xpRight in
*            printer units! (rp)
BOGUS: erroneously flags error in this respect in bold lines
				if (!vfli.fPrint)
					Assert (xp == vfli.xpRight || fFormula);
*/
		iAssert = 11;
		Assert (vfli.dcpDepend >= 0);
		iAssert = 12;
		Assert ((uns)vfli.ichMac <= ichMaxLine);

		iAssert = 14;
		Assert (vfli.dypFont >= 0 && vfli.dypAfter >= 0);
#ifdef BOGUS	/* not true with negative line spacing!!! */
		iAssert = 15;
/* in page view, dypLine may be shrunk to compensate for correspondence problems(bl) */
		Assert ((vfli.dypAfter + vfli.dypFont <= vfli.dypLine) || vfli.fPageView);
#endif
		iAssert = 16;
		Assert (vfli.dypBase >= 0 && vfli.dypBase
				< vfli.dypAfter + vfli.dypFont);
		iAssert = 17;
		Assert (vfli.dypAfter <= vfli.dypBase);
		iAssert = 18;
		if (vfli.ichSpace2 == ichNil)
			{
			iAssert = 19;
			Assert (vfli.ichSpace3 <= vfli.ichMac
					|| vfli.ichSpace3 == ichNil);
			}
		else
			{
			iAssert = 20;
			Assert (vfli.ichSpace1 <= vfli.ichSpace2);
			iAssert = 21;
			Assert (vfli.ichSpace2 <= vfli.ichSpace3);
			iAssert = 22;
			Assert (vfli.ichSpace3 <= vfli.ichMac);
			}
		}
	iAssert = 23;
	Assert ((vfli.fPrint & 0xFE) == 0);
	iAssert = 24;
	Assert ((vfli.fFormatAsPrint & 0xFE) == 0);
	iAssert = 25;
	Assert ((vfli.fOutline & 0xFE) == 0);
/* check grpchr */
	iAssert = 26;
	Assert (vhgrpchr);
	iAssert = 27;
	CkHhh (vhgrpchr, vbchrMax + cbCHRE);
	iAssert = 30;
	Assert (vfli.fSplatBreak || pchr->ich == vfli.ichMac);
	iAssert = 31;
	Assert (pchr == &(**vhgrpchr)[vbchrMac]);
}}



/* %%Function:CkPrm %%Owner:BRADV */
CkPrm(prm)
struct PRM prm;
{
	int     iAssert;
	int     cbGrpprl;
	char    *pprl;
	struct PRC *pprc, **hprc;
	CHAR    grpprl [2];


	if (!vdbs.fSprmTablesChecked) CkSprmTables();

	if (!vdbs.fCkPrm) return;

	if ((int)prm == prmNil)
		return;

	if (prm.fComplex)
		{
		hprc = HprcFromPprmComplex(&prm);
		iAssert = 1;
		AssertH(hprc);
		pprc = *hprc;
		iAssert = 2;
		Assert(pprc->wChecksum == WChecksumForPrc(pprc));
		pprl = &(pprc->grpprl[0]);
		cbGrpprl = pprc->bprlMac;
		}
	else
		{
		pprl = grpprl;
		grpprl [0] = prm.sprm;
		grpprl [1] = prm.val;
		cbGrpprl = dnsprm[prm.sprm].cch;
		if (cbGrpprl == 1 && prm.val)
			cbGrpprl = 2;
		}

	CkGrpprl(cbGrpprl, pprl, fFalse /* grpprl not from fkp */);

} /* end CkPrm */


/* C K   G R P P R L */
/* %%Function:CkGrpprl %%Owner:BRADV */
CkGrpprl(cbGrpprl, pprl, fFkp)
int     cbGrpprl;
char    *pprl;
int     fFkp;
{
	int     iAssert;
	int     cch;
	int     cchT;
	int     sprm, sprmPrev = -1;
	int     iw;
	/* 4 flags for recording fClobber because sgc is a 2 bit field */
	int     fClobberSgc[4];
	int     fSprmPRuler = fFalse;
	char    *psprm;

	cch = 0;
	fClobberSgc[0] = fClobberSgc[1]
			= fClobberSgc[2] = fClobberSgc[3] = fFalse;
	while (cch < cbGrpprl)
		{
		sprm = *pprl;
		iAssert = 1;
		if (sprm == 0)
			{
			Assert(++cch == cbGrpprl);
			continue;
			}
		Assert(0 < sprm && sprm < sprmMax);
		if ((cchT = dnsprm[sprm].cch) == 0)
			cchT = CchPrl(pprl);
		if (!fFkp)
			{
			iAssert = 2;
			/* This assertion is BOGUS.  fClobber is an order
				dependant item.  This assertion is only true if
				sprm necessarly was applied earlier than the sprm
				which set fClobber. */
			/* Assert(!fClobberSgc[dnsprm[sprm].sgc]); */
			if (dnsprm[sprm].fClobber)
				fClobberSgc[dnsprm[sprm].sgc] = fTrue;
			iAssert = 3;
			/* this is useless since sprmPRuler is the last sprmP */
			if (fSprmPRuler)
				{
				iAssert = 4;
				Assert(dnsprm[sprm].sgc != sgcPara
						|| sprm < sprmPChgTabs);
				}
			if (sprm == sprmPRuler)
				fSprmPRuler = fTrue;
			}
		else
			{
			/* We can assume we are dealing with mpiwspxPap
			because we are only called with fFkp true from
			FcCkFkp which can only call this routine for
			papx's */
			iAssert = 5;
			Assert(sprm != sprmNoop);
			if (sprm != sprmPChgTabsPapx && 
					dnsprm[sprm].sgc == sgcPap)
				{
#define cwPAPBase (cbPAPBase / sizeof (int))
				Assert( cwPAPBase * sizeof(int) == cbPAPBase);
				for (iw = 0; iw < cwPAPBase - 2; iw++)
					{
					psprm = &(mpiwspxPap[iw]).rgsprm;
					if (sprm == *psprm
							|| sprm == *(psprm + 1))
						break;
					}
				iAssert = 6;
				Assert(iw != cwPAPBase - 2);
				}
			}
		sprmPrev = sprm;
		cch += cchT;
		pprl += cchT;
		}
	iAssert = 7;
	Assert(cch == cbGrpprl);

} /* end CkGrpprl */


/* C K   S P R M   T A B L E S */
/* %%Function:CkSprmTables %%Owner:BRADV */
CkSprmTables()
{
	int     iAssert;
	int     sprm;
	struct ESPRM esprm;

	vdbs.fSprmTablesChecked = fTrue;
	iAssert = 1;
	Assert(sprmMax < 256);
	for (sprm = 1; sprm < sprmMax; sprm++)
		{
		esprm = dnsprm[sprm];
		}
#ifdef REVIEW /* REVIEW iftime (rp) */
/* need checking for mpiagsprm */
	CkPLFDE(rgchPLFDECharProps);
	CkPLFDE(rgchPLFDEParaProps);
#endif
}


#ifdef REVIEW /* REVIEW iftime(rp) */
/* need checking for mpiagsprm */
/* %%Function:CkPLFDE %%Owner:BRADV */
CkPLFDE(pplfde)
struct PLFDE *pplfde;
{
	int     iAssert;
	int sprmOld;
	int ifde;
	struct FDE *pfde;

	sprmOld = 0;
	for (ifde = 0; ifde < pplfde->ifdeMac; ifde++)
		{
		pfde = &pplfde->rgfde[ifde];
		iAssert = 1;
		Assert(0 < pfde && pfde->sprm < sprmMax);
		iAssert = 2;
		Assert(sprmOld < pfde->sprm);
		sprmOld = pfde->sprm;
		iAssert = 3;
		Assert(pfde->bto + pfde->btl <= 15);
		iAssert = 4;
		Assert(pfde->iType == iInt || pfde->iType == iUns);
		if (pfde->iType == iUns)
			{
			iAssert = 5;
			Assert(pfde->bto == 0 && pfde->btl == 15);
			}
		}
}


#endif /* REVIEW iftime */


/* %%Function:TestPlc %%Owner:BRADV */
TestPlc()
{
}


/* %%Function:TestPl %%Owner:BRADV */
TestPl()
{
}


/* %%Function:TestSttb %%Owner:BRADV */
TestSttb()
{
	int     iAssert;
	struct STTB **hsttb, *psttb;
	int cbMin = (cwSTTBBase + 5) * sizeof(int);

	hsttb = HsttbInit(cbMin / sizeof(int) - cwSTTBBase, fFalse);
	iAssert = 1;
	CkHsttb(hsttb, cbMin);

	stABC[0] = 5;
	IbstAddStToSttb(hsttb, &stABC);
	iAssert = 2;
	CkHsttb(hsttb, cbMin);

	stABC[0] = 1;
	FInsStInSttb(hsttb, 0, &stABC);
	iAssert = 3;
	CkHsttb(hsttb, cbMin);

	stABC[0] = 10;
	IbstAddStToSttb(hsttb, &stABC);
	iAssert = 4;
	CkHsttb(hsttb, cbMin);

	stABC[0] = 6;
	FChangeStInSttb(hsttb, 1, &stABC);
	iAssert = 5;
	CkHsttb(hsttb, cbMin);

	stABC[0] = 0;
	FInsStInSttb(hsttb, 0, &stABC);
	iAssert = 6;
	CkHsttb(hsttb, cbMin);

	stABC[0] = 7;
	FChangeStInSttb(hsttb, 0, &stABC);
	iAssert = 7;
	CkHsttb(hsttb, 0);
	psttb = *hsttb;
	iAssert = 8;
	Assert(psttb->bMax == 40 && psttb->ibstMac == 4 &&
			psttb->rgbst[0] == 32 && psttb->rgbst[1] == 19 &&
			psttb->rgbst[2] == 12 && psttb->rgbst[3] == 21);
	stABC[0] = 5;
	FChangeStInSttb(hsttb, 2, &stABC);
	iAssert = 9;
	CkHsttb(hsttb, cbMin);

	DeleteIFromSttb(hsttb, 1);
	iAssert = 10;
	CkHsttb(hsttb, 0);
	DeleteIFromSttb(hsttb, 2);
	iAssert = 11;
	CkHsttb(hsttb, 0);
	DeleteIFromSttb(hsttb, 0);
	iAssert = 12;
	CkHsttb(hsttb, 0);
	psttb = *hsttb;
	iAssert = 13;
	Assert(psttb->bMax == 12 && psttb->ibstMac == 1 &&
			psttb->rgbst[0] == 6);
	DeleteIFromSttb(hsttb, 0);
	iAssert = 14;
	CkHsttb(hsttb, 0);
	psttb = *hsttb;
	iAssert = 15;
	Assert(psttb->bMax == 4 && psttb->ibstMac == 0);
	FreeHsttb(hsttb);
}





/* F  C K  F L D  F O R  D E L E T E */

/* %%Function:FCkFldForDelete  %%Owner:BRADV */
EXPORT FCkFldForDelete (doc, cpFirst, cpLim)
int doc;
CP cpFirst, cpLim;

{

	CP cpFirstOld = cpFirst;
	CP cpLimOld = cpLim;
	struct CA caT;

	if (cpFirst >= cpLim)
		return fTrue;

	PcaSet(&caT, doc, cpFirst, cpLim);
	/*  don't want to do CachePara's or FetchCp's for docs < docMin... */
	if (FIsRealDoc (doc)
			&& FPossibleDeadMismatch(&caT))
		/* grunt match field character method */
		ExpandToSameField (caT.doc, &caT.cpFirst, &caT.cpLim);

	else  if (PdodDoc (doc)->hplcfld != hNil)
		/* efficient match field character method */
		ExpandToSameLiveField (&caT);


	return caT.cpFirst == cpFirstOld && caT.cpLim == cpLimOld;

}





/* F  I S  R E A L  D O C */
/*  Some docs do not have proper style sheets.  As a result a CachePara()
	cannot be done on them.  This function returns true iff it is ok to do a
	CachePara on a doc.
*/

/* %%Function:FIsRealDoc  %%Owner:BRADV */
FIsRealDoc (doc)
int doc;

{
	return ! ( mpdochdod [doc] == hNil || doc < docMinNormal ||
			doc == vdocTemp || doc == vdocScratch);
}






/* C K  F L D  I D L E */
/*  Idle time field consistancy check routine
*/

/* %%Function:CkFldIdle %%Owner:BRADV */
CkFldIdle()

{
	int doc = 1;

	while (doc < docMac)
		{
		if (FIsRealDoc (doc))
			CkPlcfld (doc);
		if (mpdochdod[doc] != hNil)
			CheckFNestedBits(doc);
		doc++;
		}

}



/* C K  P L C F L D */
/*  For each entry in the plcfld, assure that the correct characters are in
	the document.  Also ensure that each field entry is valid.
*/

/* %%Function:CkPlcfld %%Owner:BRADV */
CkPlcfld (doc)
int doc;

{
	struct PLC **hplcfld = PdodDoc (doc)->hplcfld;
	int ifldChBegin, ifldChSeparate, ifldChEnd, ifldMac;
	int flt;
	int cChBegin;
	struct FLD fld;

	if (hplcfld == hNil || (ifldMac = IMacPlc( hplcfld )) <= 0)
		return;

	ifldChBegin = 0;
	GetPlc( hplcfld, 0, &fld );
	AssertSz (fld.ch == chFieldBegin, "Entry in PLCFLD is not valid! ");

	CheckFNestedBits(doc);

	for (;;)
		{
		ifldChSeparate = ifldNil;
		ifldChEnd = ifldChBegin;
		cChBegin = 0;

		AssertSz ((flt = fld.flt) >= fltMin && fld.flt < fltMax,
				"Entry in PLCFLD is not valid! ");

		if ((uns)(fld.flt-fltSeqLevOut)<=(fltSeqLevNum-fltSeqLevOut))
			Assert(!PdodDoc(doc)->fHasNoSeqLev || PdodDoc(doc)->fInvalSeqLev);

		do
			{
			AssertSz (ifldChEnd + 1 < ifldMac,"Entry in PLCFLD is not valid! ");

			GetPlc( hplcfld, ++ifldChEnd, &fld );
			switch (fld.ch)
				{
			case chFieldBegin:
				cChBegin++;
				break;
			case chFieldSeparate:
				if (cChBegin == 0)
					{
					AssertSz (ifldChSeparate == ifldNil,
							"Entry in PLCFLD is not valid! ");
					ifldChSeparate = ifldChEnd;
					/* verify bData valid */
					switch (flt)
						{
						/* add case for each flt that uses bData with
							code to verify bData. */
					case fltDde:
					case fltDdeHot:
							{
							int iddliMac;
							int bData = fld.bData;
							if (docDde != docNil)
								{
								struct PLC **hplcddli;
								Assert ((hplcddli = PdodDoc (docDde)->hplcddli)
										!= hNil);
								iddliMac = (*hplcddli)->iMac;
								}
							else
								iddliMac = 0;
							AssertSz (bData == bDataInval ||
									bData == iddliRequest ||
									(bData > iddliRequest && bData
									< iddliMac), "Entry in PLCFLD is not valid! ");
							break;
							}
					default:
						AssertSz (fld.bData == bDataInval,
								"Entry in PLCFLD is not valid! ");
						break;
						}
					}
				break;
			case chFieldEnd:
				if (cChBegin != 0)
					cChBegin--;
				else
					{  /* check grpf */
					AssertSz (fld.fResultDirty ||
							!fld.fResultEdited, "Entry in PLCFLD is not valid! ");
					}
				break;
			default:
				AssertSz (fFalse, "Entry in PLCFLD is not valid! ");
				}
			} 
		while (fld.ch != chFieldEnd || cChBegin);

		FetchCpAndPara (doc, CpPlc( hplcfld, ifldChBegin),
				fcmChars+fcmProps);
		AssertSz (*vhpchFetch == chFieldBegin && vchpFetch.fSpec,
				"Entry in PLCFLD is not valid! ");

		if (ifldChSeparate != ifldNil)
			{
			FetchCpAndPara (doc, CpPlc( hplcfld, ifldChSeparate),
					fcmChars+fcmProps);
			AssertSz (*vhpchFetch == chFieldSeparate && vchpFetch.fSpec,
					"Entry in PLCFLD is not valid! ");
			}

		FetchCpAndPara (doc, CpPlc( hplcfld, ifldChEnd),
				fcmChars+fcmProps);
		AssertSz (*vhpchFetch == chFieldEnd && vchpFetch.fSpec,
				"Entry in PLCFLD is not valid! ");

		do
			{
			if (++ifldChBegin >= ifldMac)
				return;
			GetPlc( hplcfld, ifldChBegin, &fld );
			} 
		while (fld.ch != chFieldBegin);
		GetPlc( hplcfld, ifldChBegin, &fld );

		} /* for (;;) */
}




/* C K  T E X T */
/* Check the consistency of the text in all doc's.
*/

/* %%Function:CkText %%Owner:BRADV */
CkText (fAbortOK)
BOOL fAbortOK;
{
	int doc = docMinNormal;
	extern int docRecord;

	while (doc < docMac)
		{
		if (FIsRealDoc (doc) && !PdodDoc(doc)->fMcr && doc != docRecord)
			{
			CkTextDoc (doc, fAbortOK);
			}
		doc++;
		}
}


/* C K  T E X T  D O C */
/*  Check all text of doc:
		account for all fSpec characters
		assure no broken CR-LF pairs
*/
/* %%Function:CkTextDoc %%Owner:BRADV */
CkTextDoc (doc, fAbortOK)
int doc;
BOOL fAbortOK;

{
	CP cp = 0, cpMac = CpMacDoc (doc);
	BOOL fLastRunEndCR = fFalse;
	int docMother = DocMother(doc);
	int stcp;
	struct STSH stsh;

	SetVdocStsh(doc);
	RecordStshForDocNoExcp(vdocStsh, &stsh);
	while (cp < cpMac)
		{
		if (fAbortOK && FMsgPresent (mtyIdle)) /* calls random FetchCp */
			break;
		CachePara (doc, cp);
		FetchCp (doc, cp, fcmProps+fcmChars);

		/* Make sure style exists! */
		/*
		*  FSetUndoB1 doesn't bother to copy styles or fonts into docUndo
		*  which basically means bogus fonts and styles are allowed in
		*  docUndo.  REVIEW iftime: a more robust solution would attempt
		*  to figure where docUndo was filled from and set vdocStsh
		*  accordingly.
		*/
		stcp = StcpFromStc(vpapFetch.stc, stsh.cstcStd);
		AssertSz(vpapFetch.stc > stcStdMin || stcp < (*stsh.hsttbName)->ibstMac
				|| docMother == docUndo, "Invalid Style in Doc");

		/* verify font */
		Assert(vchpFetch.hps != 0);
		Assert(vchpFetch.ftc >= 0);
		/* if text is moved from one doc to another in low memory this may
			fail (the user would have gotten some sort of message) */
		/* can also fail if docMother is docUndo...see comment above about
			styles. */
		if (vchpFetch.ftc >= PdodDoc(docMother)->ftcMac)
			ReportSz("doc contains ftcs out of range");

		if (!(vchpFetch.fVanish && vchpFetch.fSysVanish))
			{
			if (vchpFetch.fSpec)
				CkFSpecRun ();
			else 
				CkCharRun (&fLastRunEndCR);
			}

		cp += vccpFetch;

		/*   REVIEW iftime:  include a check for illegal fFldVanish chars (rp) */
		}

}



/* C K  C H A R  R U N */
/*  Assure that the fetched run of characters is valid.
*/

/* %%Function:CkCharRun %%Owner:BRADV */
CkCharRun (pfLastRunEndCR)
BOOL *pfLastRunEndCR;

{
	CHAR HUGE *hpch = vhpchFetch;
	CHAR HUGE *hpchLim = vhpchFetch + vccpFetch;

#ifdef CRLF
	/*  if last run ended with chReturn, this one had better start with
		chEol, else it had better not! */
	if (*pfLastRunEndCR)
		AssertSz ((*hpch == chEol || *hpch == chTable),"CR-LF pair has been split! ");
	else
		AssertSz (*hpch != chEol && *hpch != chTable,"CR-LF pair has been split! ");

	*pfLastRunEndCR = fFalse;
#endif /* CRLF */

	while (hpch < hpchLim)
		{

#ifdef CRLF
		if (*hpch == chEol || *hpch == chTable)
			/* must be proceeded by chReturn */
			{
			if (hpch > vhpchFetch)
				AssertSz (*(hpch-1) == chReturn,"CR-LF pair has been split! ");
			}

		if (*hpch == chReturn)
			/* must be followed by chEol or chTable */
			{
			if (hpch+1 == hpchLim)
				*pfLastRunEndCR = fTrue; /* check in next run */
			else
				AssertSz ((*(hpch+1) == chEol || *(hpch+1) == chTable),
						"CR-LF pair has been split! ");
			}
#endif /* CRLF */

		/* REVIEW iftime: other character-by-character tests (rp) */

		hpch++;
		}
}




/* C K  F  S P E C  R U N */
/*  Check a run of fSpec characters to assure valid.
*/

/* %%Function:CkFSpecRun %%Owner:BRADV */
CkFSpecRun()

{
	CHAR HUGE *hpch = vhpchFetch;
	CHAR HUGE *hpchLim = hpch + vccpFetch;
	int ifld, flt;
	CP cp = vcpFetch;

	while (hpch < hpchLim)
		{
		switch (*hpch)
			{
		case chFieldBegin:
		case chFieldSeparate:
		case chFieldEnd:

			/*  field char, assure it is in the plcfld */
			AssertSz (vchpFetch.fFldVanish ||
					IfldFromDocCp (vdocFetch, cp, fTrue) != ifldNil,
					"fSpec characters in doc not accounted for. ");
			break;

		case chPicture:
		case chFootnote:
		case chTFtn:
		case chTFtnCont:
		case chAtn:
		case chLnn:
			/* REVIEW iftime: can any checks be done to any of these chars?? (rp) */
			break;

		default:
			AssertSz (fFalse, "fSpec characters in doc not accounted for. ");
			break;
			}

		hpch++;
		cp++;
		}

}




/* C K  P A P H */
/* Check to see that for each paph in the plcpad of doc:
	1) it is invalid,
	2) it does not contain a line count, or
	3) its line count corresponds to a line count generated the hard way.
*/

/* %%Function:CkPaph  %%Owner:BRADV */
CkPaph (doc, fAbortOK)
int doc;
BOOL fAbortOK;

{
	CP cpFirstDisplay, cpLimDisplay;
	CP cpFirstPaph, cpLimPaph;
	CP cp = cp0;
	CP cpMac = CpMacDoc (doc);
	int fLayoutSave;
	struct PLC **hplc;
	int clT, clSum;
	struct WWD *pwwd;
	struct DOD **hdod = mpdochdod[doc];

	Assert(hdod != hNil);
#ifdef REVIEW  /* iftime */
/* Future(dl) The purpose of this routine is to verify that the method for
   finding line bounds used in the calls to GetCpFirstCpLimDisplayPara and
   FFormatDisplayPara (method used for goto line) correspond to the PHE info 
   for each paragraph in the document. They don't and we're not willing to
   change the code at this time to reconcile the two methods. In Chrism's 
   opinion, ClFormatLines does the definitive job of figuring out line bounds
   and PHEs and should be used instead ofGetCpFirstCpLim... and
	   FFormatDisplayParas. */. 

			if (!vdbs.fCkPaph || (*hdod)->fShort ||
			(*hdod)->hplcphe == hNil ||
			(*hdod)->fEnvDirty ||
			(hplc = HplcInit (0,5,cp0, fTrue /*fExternal*/)) == hNil)
		return;
	fLayoutSave = vfli.fLayout;
	vfli.fLayout = fTrue;
	SetFlm( flmRepaginate );
	pwwd = PwwdWw(wwTemp);
	pwwd->grpfvisi.w = 0;
	pwwd->grpfvisi.fSeeHidden = vprsu.fSeeHidden;
	pwwd->grpfvisi.grpfShowResults = vprsu.fShowResults ? ~0 : 0;
	pwwd->grpfvisi.fForceField = fTrue;
	pwwd->grpfvisi.flm = flmRepaginate;
	LinkDocToWw(doc, wwTemp, wwNil);

	while (cp < cpMac && (!fAbortOK || !FMsgPresent (mtyIdle)))
		{
		GetCpFirstCpLimDisplayPara (wwTemp, doc, cp, &cpFirstDisplay,
				&cpLimDisplay);
		if (!FFormatDisplayPara (wwTemp, doc, cpFirstDisplay, cpLimDisplay, hplc))
			break;

		clSum = 0;
		cp = cpFirstDisplay;

		while (cp < cpLimDisplay)
			{
			/*  returns 0 if clMac not useable */
			if ((clT = ClMacFromPlcpadPhe (wwTemp, doc, cp, &cpFirstPaph, &cpLimPaph))
					<= 0)
				break;

			Assert (cp < cpLimPaph && cp >= cpFirstDisplay);
			clSum += clT;

			if (cpFirstPaph != cp)
				{
				Assert (fFalse);
				break;
				}

			if (cpLimPaph != CpPlc(hplc,(clT = IMacPlc(hplc))))
				clT = IInPlcCheck(hplc, cpLimPaph);

			Assert(clT >= 0);

			if (clSum != clT && --clSum != clT)
				{
				Assert (fFalse);
				break;
				}

			cp = cpLimPaph;
			}

		cp = cpLimDisplay;
		}
	vfli.fLayout = fLayoutSave;
	FreeHplc (hplc);
#endif
}


/* %%Function:CkBookmarks %%Owner:BRADV */
CkBookmarks(hsttb, hplcbkf, hplcbkl, cpMac)
struct STTB **hsttb;
struct PLC **hplcbkf;
struct PLC **hplcbkl;
CP cpMac;

{
	int ibkf, ibkfMac, ibkl;
	struct BKF bkf;

	if (hsttb == hNil || hplcbkf == hNil || hplcbkl == hNil)
		{
		Assert (hsttb == hNil && hplcbkf == hNil && hplcbkl == hNil);
		return;
		}

	CkHsttb (hsttb, 0);
#ifdef SHOWCK
	if (vdbs.fReports)
		CommSzCrLf(SzShared("Check hplcbkf "));
#endif /* SHOWCK */
	CkHplcMult (hplcbkf, cbBKF, fFalse);
#ifdef SHOWCK
	if (vdbs.fReports)
		CommSzCrLf(SzShared("Check hplcbkl "));
#endif /* SHOWCK */
	CkHplcMult (hplcbkl, cbBKL, fFalse);

	Assert ((ibkfMac = (*hsttb)->ibstMac) == (*hplcbkf)->iMac);
	Assert (ibkfMac == (*hplcbkl)->iMac);
	Assert (ibkfMac >= 0);
	Assert (CpPlc( hplcbkf, ibkfMac ) == cpMac);
	Assert (CpPlc( hplcbkl, ibkfMac ) == cpMac);

	ibkl = ibkfMac;
	for (ibkf = 0; ibkf < ibkfMac; ibkf++ )
		{
		CHAR st[cchMaxSt];
		GetPlc( hplcbkf, ibkf, &bkf );
		Assert (bkf.ibkl != ibkl);
		ibkl = bkf.ibkl;
		Assert (ibkl >= 0 && ibkl < ibkfMac);
		Assert (CpPlc( hplcbkf, ibkf ) <= CpPlc( hplcbkl, ibkl));
		GetStFromSttb(hsttb, ibkf, st);
		Assert(FLegalBkmkName(st));
		}

	/* REVIEW iftime: check ibkl's in the bkf's to assure exact coverage (rp) */
}


/* %%Function:CkTlbx %%Owner:BRADV */
NATIVE CkTlbx(fInitial)
BOOL fInitial;

{

#define ctlbxGroup  32
#define ilMacCkTlbx	((tlbxMac/ctlbxGroup)+1)

	static long rglCkTlbx [ilMacCkTlbx];
	extern long mptlbxpfn[];

	int il, ntlbx, ctlbx;
	long l;

	for (il = 0, ntlbx = 0; il < ilMacCkTlbx; il++)
		{
		l = 0;
		for (ctlbx = 0; ctlbx < ctlbxGroup && ntlbx < tlbxMac;
				ctlbx++, ntlbx++)
			l += (long)mptlbxpfn [ntlbx];

		if (fInitial)
			rglCkTlbx [il] = l;
		else  if (rglCkTlbx [il] != l)
			{
			int rgw [2];
			rgw[0] = il*ctlbxGroup;	/* between */
			rgw[1] = (il+1)*ctlbxGroup; /* and */
			IdMessageBoxMstRgwMb (mstDbgCkTlbx, rgw, MB_SYSTEMMODAL|MB_OK);
			Assert (fFalse);
			}
		}
	Assert (ntlbx == tlbxMac);
}



#endif /* DEBUG */
