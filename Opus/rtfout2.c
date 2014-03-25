/* R T F O U T 2 . C */
/* this file is #include'd into rtfout.c (broken up for SLM's sake) */


/*  %%Function:  DumpParaProps  %%Owner:  bobz       */

NATIVE DumpParaProps(prebl, ppch, ppap, pfBracketProp, pfBracketBroken)
struct REBL *prebl;
CHAR **ppch;
struct PAP *ppap;
BOOL *pfBracketProp, *pfBracketBroken;
{
	char *pch;

	/* Write para props */
	pch = prebl->rgch;
	if (*pfBracketProp)
		{
		*pch++ = chRTFCloseBrk;
		*pfBracketProp = fFalse;
		*pfBracketBroken = fTrue;
		}
	*pch++ = chBackslash;
	pch = PchSzRtfMove(iszRTFpard, pch);
	if (vpapFetch.stc != ppap->stc)
		{
		*pch++ = chBackslash;
		pch = PchSzRtfMove(iszRTFplain, pch);
		}
	*pch++ = ' ';
	FlushRebl(prebl, &pch);
	PropToRtf(prebl, &vpapFetch, irrbPapFirst, irrbPapLim,
			0, FPropToRtfSpecMacPlus);
	if (vpapFetch.stc != ppap->stc)
		PropToRtf(prebl, &vchpStc, irrbChpFirst, irrbChpLim,
				0, FPropToRtfSpecMacPlus);
	*ppap = vpapFetch;
	*ppch = pch;
}




/* dump bkmk name - start or end */
/*  %%Function:  DumpBkmks  %%Owner:  bobz       */

DumpBkmks(doc, cpCur, pbkmkRtf, prebl, ppch, ppchOut)
int doc;
CP cpCur;
struct BKMKRTF *pbkmkRtf;
struct REBL *prebl;
CHAR **ppch;
CHAR **ppchOut;
{
	int i, ibst, isz;
	CHAR *pchRebl;

	/* get all bookmark starts or ends that match this CP */
	while ( (i =
			(cpCur == pbkmkRtf->cpFirstBkmk) ? iBkmkStart :
			((cpCur == pbkmkRtf->cpLimBkmk) ? iBkmkEnd :
			0))
			!= 0 )
		{
		/* clear out anything up to this point */
		FlushReblPch(prebl, *ppch, *ppchOut);
		*ppchOut = *ppch; /* reset this */

		/* get and write out bookmark name */
		/* pbkmkRtf->icpFirstBkmk maps directly to the ibst;
		pbkmkRtf->icpLimBkmk needs to get value we stored in
		pbkmkRtf->hmpIbklIbst.
		*/

		if (i == iBkmkEnd)
			{
			ibst = *(*pbkmkRtf->hmpIbklIbst + pbkmkRtf->icpLimBkmk);
			isz = iszRTFbkmkend;
			}
		else
			{
			ibst = pbkmkRtf->icpFirstBkmk;
			isz = iszRTFbkmkstart;
			}


		pchRebl = prebl->rgch;
		*pchRebl++ = chRTFOpenBrk;
		/* ignorable destination */
		*pchRebl++ = chBackslash;
		*pchRebl++ = '*';
		*pchRebl++ = chBackslash;
		pchRebl = PchSzRtfMove(isz, pchRebl);
		*pchRebl++ = chSpace;

#ifdef DRTFBKMK
		CommSzNum(SzShared("Cp bkmk match: i = "), i);
		CommSzNum(SzShared("Cp bkmk match: lowword of cpcur = "), LowWord(cpCur));
		CommSzNum(SzShared("Cp bkmk match: lowword of cpLim = "), LowWord(pbkmkRtf->cpLimBkmk));
		CommSzNum(SzShared("Cp bkmk match: lowword of cpFirst = "), LowWord(pbkmkRtf->cpFirstBkmk));
		CommSzNum(SzShared("Cp bkmk match: ibst = "), ibst);
#endif
		GetSzFromSttb(PdodDoc(doc)->hsttbBkmk, ibst, pchRebl);
		pchRebl += CchSz(pchRebl)-1;

		*pchRebl++ = chRTFCloseBrk;
		FlushRebl(prebl, &pchRebl);


		/* update the next bookmark */

		if (i == iBkmkEnd)
			if ((++pbkmkRtf->icpLimBkmk) >= pbkmkRtf->ibkmkMac)
				pbkmkRtf->cpLimBkmk = cpNil;
			else
				pbkmkRtf->cpLimBkmk = CpPlc( PdodDoc(doc)->hplcbkl,
						pbkmkRtf->icpLimBkmk );
		else if ((++pbkmkRtf->icpFirstBkmk) >= pbkmkRtf->ibkmkMac)
			pbkmkRtf->cpFirstBkmk = cpNil;
		else
			pbkmkRtf->cpFirstBkmk = CpPlc( PdodDoc(doc)->hplcbkf,
					pbkmkRtf->icpFirstBkmk );

		}   /* while */
}




/* P R O P   T O   R T F

Inputs:
	prebl	RtfOut Environment Block
	pprop	Property (PAP, CHP, etc.)
	irrbFirst index of beginning of entries for prop in rgrrb
	irrbLim lim of entries for prop in rgrrb
	pfnFEmit pointer to function that decides if a given property
		should be emitted as RTF. If 0, means emit all.
	pfnFPropToRtfSpec pointer to function that handles properties that
		require special processing when translating to RTF.

Outputs RTF corresponding to properties
*/
/*  %%Function:  PropToRtf  %%Owner:  bobz       */

PropToRtf(prebl, pprop, irrbFirst, irrbLim, pfnFEmit, pfnFPropToRtfSpec)
struct REBL *prebl;
char *pprop;
int irrbFirst;
int irrbLim;
int (*pfnFEmit)();
int (*pfnFPropToRtfSpec)();

{{ /* NATIVE - PropToRtf */

	int irrb;
	int isz;
	int impUnbiased;
	int *mpvalisz;
	char *pch = prebl->rgch;
	int far *mpirrbisz;
	char *pchPropLim;

	pchPropLim = prebl->rgch + cchMaxSz - 1;

	for (irrb = irrbFirst; irrb < irrbLim; irrb++)
		{
		struct RRB rrb;
		int val, val2;
		if (pfnFEmit != 0)
			{
			if (!(*pfnFEmit)(pprop, irrb))
				continue;
			}
		mpirrbisz = mpirrbiszWord;
		GetRrb(irrb, &rrb);
		val = pprop[rrb.b];
		isz = mpirrbisz[irrb];

		if (pch + cchMaxPropRtf > pchPropLim)
			FlushRebl(prebl, &pch);

		switch (rrb.rrba)
			{
		case rrbaFlag:
			if (val)
				{
				if (isz < iszRTFMax)
					goto LEmitSz;
				else
					goto LEmitRTFMaxSz;
				}
			break;
		case rrbaBit:
			if (((int *)pprop)[rrb.w] & (1 << rrb.b))
				{
LEmitSz:
				*pch++ = chBackslash;
				pch = PchSzRtfMove(isz, pch);
				}
			break;
		case rrbaUns:
		case rrbaWord:
			val = *(int *)(pprop + rrb.b);
		/* Fall through . . . */
DetermineIsz:
		case rrbaByte:
			if (val != rrb.w)
				{
				if (isz >= iszRTFMax)
					{
LEmitRTFMaxSz:
					impUnbiased = isz - iszRTFMax;

					switch (isz /* this is the biased imp */)
						{
					case impkulisz:
						if (val-- == 0)
							continue;
					case impfpcisz:
				/* if enddoc, output both endnotes and
					enddoc */
						if (val == fpcEndDoc)
							{
							*pch++ = chBackslash;
							pch = PchSzRtfMove(rgmpvalisz
									[impUnbiased][fpcEndnote], pch);
					/* fall through will put out \endnote */
							}
						break;
						}

					if (val >= rgvalMax[impUnbiased])
						continue;
					isz = rgmpvalisz[impUnbiased][val];
					goto LEmitSz;
					}
				*pch++ = chBackslash;
				pch = PchSzRtfMove(isz, pch);
				if (rrb.rrba == rrbaUns)
					CchUnsToPpch(val, &pch);
				else
					CchIntToPpch(val, &pch);
				}
			break;
		case rrbaSpec:
			if ((*pfnFPropToRtfSpec)(pprop, &rrb, irrb, &val, &pch, prebl))
				goto DetermineIsz;
			break;
#ifdef DEBUG
		default:

#ifdef BZ
			CommSzNum(SzShared("PropToRtf invalid rrba = "), rrb.rrba);
#endif
			Assert(fFalse);

#endif
			}
		}

	if (pch != prebl->rgch)
		{
		*pch++ = ' ';
		FlushRebl(prebl, &pch);
		}
}}


/*  %%Function:  FPropToRtfSpecMacPlus  %%Owner:  bobz       */

FPropToRtfSpecMacPlus(pprop, prrb, irrb, pval, ppch, prebl)
char *pprop;
struct RRB *prrb;
int irrb;
int *pval;
char **ppch;
struct REBL *prebl;
{
	int val, val2;
	int isz;
	int fFirstFlush;
	struct BRC *pbrc, *pbrcT;
	int ibrc, ibrcLim;
	int dxaAbs, dyaAbs;
	int *mpibrcisz;
	struct BRC brc;
	char *pch = *ppch;

	switch (irrb)
		{
	case irrbRs:
		*pval = ((struct PAP *)pprop)->stc;
		return (fTrue);
	case irrbRcf:
		*pval = ((struct CHP *)pprop)->ico;
		return (fTrue);
	case irrbRkul:
		*pval = ((struct CHP *)pprop)->kul;
		return (fTrue);
	case irrbRexpnd:
		*pval = ((struct CHP *)pprop)->qpsSpace;
		/* internally stored in 6 bit 2's complement excess 56 notation.
		if > 56, convert to a negative # by oring 1's into the high
		order 10 bits (FFC0). Thus 63(3f)-> -1, 62(3e) -> -2... 57 -> -7
		*/
		if (*pval > 56)
			*pval |= 0xffc0;
		return (fTrue);
	case irrbRv:
		if (((struct CHP *)pprop)->fVanish)
			{
			*pch++ = chBackslash;
			pch = PchSzRtfMove(iszRTFv, pch);
			}
		break;
	case irrbRf:
		val = ((struct CHP *)pprop)->ftc;
		if (val != prrb->w)
			{
			*pch++ = chBackslash;
			pch = PchSzRtfMove(iszRTFf, pch);
			CchUnsToPpch(val, &pch);
			}
		break;
 	case irrbRfs:
 		*pval = ((struct CHP *)pprop)->hps;
 		return (fTrue);
	case irrbRup:
		if (*pval != 0)
			{
			if (*pval < 128)
				isz = iszRTFup;
			else
				{
				isz = iszRTFdn;
				*pval = 256 - *pval;
				}
			*pch++ = chBackslash;
			pch = PchSzRtfMove(isz, pch);
			CchIntToPpch(*pval, &pch);
			}
		break;
	case irrbRpbrc:
	case irrbRclbrc:
		fFirstFlush = fTrue;
		if (irrb == irrbRpbrc)
			{
			pbrc = &((struct PAP *)pprop)->brcTop;
			ibrcLim = ibrcPapLim;
			mpibrcisz = mpibrciszPap;
			}
		else
			{
			Assert(irrb == irrbRclbrc);
			pbrc = &((struct TC *)pprop)->brcTop;
			ibrcLim = ibrcTcLim;
			mpibrcisz = mpibrciszTc;
			}
		brc.brc = 0;
		ibrc = 0;
		if (pbrc->brc != 0)
			{
			if (irrb == irrbRpbrc)
				{
				for (pbrcT = pbrc + 1, ibrc = 1;
						ibrc < 4; ibrc++, pbrcT++)
					{
					if (pbrc->brc != pbrcT->brc)
						break;
					}
				if (ibrc == 4)
					{
					FlushRebl(prebl, &pch);
					brc.brc = pbrc->brc;
					pch = prebl->rgch;
					*pch++ = chBackslash;
					pch = PchSzRtfMove(iszRTFbox, pch);
					FlushRebl(prebl, &pch);
					PropToRtf(prebl, &brc, irrbRBrcFirst, irrbRBrcLim, 0, 
							FPropToRtfSpecMacPlus);
					ibrc = 4;
					pbrc = pbrcT;
					}
				else
					ibrc = 0;
				}
			}
		for (; ibrc < ibrcLim; ibrc++, pbrc++)
			{
			if (pbrc->brc == 0)
				continue;
			if (fFirstFlush)
				{
				FlushRebl(prebl, &pch);
				fFirstFlush = fFalse;
				}
			brc.brc = pbrc->brc;
			pch = prebl->rgch;
			*pch++ = chBackslash;
			pch = PchSzRtfMove(mpibrcisz[ibrc], pch);
			FlushRebl(prebl, &pch);
			PropToRtf(prebl, &brc, irrbRBrcFirst, irrbRBrcLim, 0, 
					FPropToRtfSpecMacPlus);
			}
		break;
	case irrbRbrdrsh:
		if (((struct BRC *)pprop)->fShadow)
			{
			*pch++ = chBackslash;
			pch = PchSzRtfMove(iszRTFbrdrsh, pch);
			}
		break;
	case irrbRbrsp:
		*pval = ((struct BRC *)pprop)->dxpSpace * czaPoint;
		return (fTrue);
	case irrbRbrcBase:
		isz = -1;
		switch (((struct BRC *)pprop)->brcBase)
			{
		case brcSingle:
			isz = iszRTFbrdrs;
			break;
		case brcTwoSingle:
			isz = iszRTFbrdrdb;
			break;
		case brcThick:
			isz = iszRTFbrdrth;
			break;
		case brcDotted:
			isz = iszRTFbrdrdot;
			break;
		case brcHairline:
			isz = iszRTFbrdrhair;
			break;
			}
		if (isz != -1)
			{
			*pch++ = chBackslash;
			pch = PchSzRtfMove(isz, pch);
			}
		break;
	case irrbRpcVert:
		*pval = ((struct PAP *)pprop)->pcVert;
		return (fTrue);
	case irrbRpcHorz:
		*pval = ((struct PAP *)pprop)->pcHorz;
		return (fTrue);
	case irrbRdxaAbs:
		if ((dxaAbs = ((struct PAP *)pprop)->dxaAbs) > 0)
			{
			*pch++ = chBackslash;
			pch = PchSzRtfMove(iszRTFposx, pch);
			/* values above 0 are biased by 1 */
			CchIntToPpch(dxaAbs - 1, &pch);
			break;
			}
		*pval = -dxaAbs / 4;
		return (fTrue);
	case irrbRdyaAbs:
		if ((dyaAbs = ((struct PAP *)pprop)->dyaAbs) > 0)
			{
			*pch++ = chBackslash;
			pch = PchSzRtfMove(iszRTFposy, pch);
			/* values above 0 are biased by 1 */
			CchIntToPpch(dyaAbs - 1, &pch);
			break;
			}
		*pval = -dyaAbs / 4;
		return (fTrue);
	case irrbRclmgf:
		if (((struct TC *)pprop)->fFirstMerged)
			{
			*pch++ = chBackslash;
			pch = PchSzRtfMove(iszRTFclmgf, pch);
			}
		break;
	case irrbRclmrg:
		if (((struct TC *)pprop)->fMerged)
			{
			*pch++ = chBackslash;
			pch = PchSzRtfMove(iszRTFclmrg, pch);
			}
		break;
	case irrbRfpc:
		*pval = ((struct DOP *)pprop)->fpc;
		return (fTrue);
	case irrbRftnstart:
		*pval = ((struct DOP *)pprop)->nFtn;
		return (fTrue);

	case irrbRpgnstart:
		/* for Opus, we get the info for this doc property from
			the first section. We assert that section has been cached
			whenever we get here.
		*/
		Assert (caSect.cpFirst == cp0);

		/* intentional fall through */

	case irrbRpgnstarts:
		/* note: rtf default is 1, so 1 will mean no
			entry generated. If there is no page restart,
			we need to generate an entry 1. Rtfin will
			set the pgnStart to 1 if it gets a pgnrestart
			bz
		*/
		*pval = vsepFetch.fPgnRestart ? vsepFetch.pgnStart : 1;
		return (fTrue);


	case irrbRlinestart:
		/* for Opus, we get the info for this doc property from
			the first section if its value is not the default value.
			We know that lnnMin is 0 based and we have to add 1 to
			adjust to the linestart range, so we assume the default
			value is 0 rather than getting it from the rgrrb.
			We assert that section has been cached whenever we get here.
		*/
		Assert (caSect.cpFirst == cp0);
		*pval = (vsepFetch.lnnMin != 0) ?
				vsepFetch.lnnMin + 1 : prrb->w;
		return (fTrue);

	case irrbRrevprop:

		/* 7 bit field in dop */
		*pval = ((struct DOP *)pprop)->irmProps;
		return (fTrue);

	case irrbRcols:
		*pval = ((struct SEP *)pprop)->ccolM1 + 1;
		return (fTrue);

	case irrbRlinestarts:
		*pval = ((struct SEP *)pprop)->lnnMin + 1;
		return (fTrue);

	case irrbRtx:
			{ /* Do tab stops */
			int itbd;
			char *ptbd;
			int *pdxa;
			char *pchTabLim;

			pchTabLim = prebl->rgch + cchMaxSz - 1;
			for (itbd = 0, ptbd = ((struct PAP *)pprop)->rgtbd,
					pdxa = ((struct PAP *)pprop)->rgdxaTab;
					itbd < ((struct PAP *)pprop)->itbdMac; itbd++, ptbd++, pdxa++)
				{
				char tbd;

				if (pch + cchMaxTabRtf > pchTabLim)
					FlushRebl(prebl, &pch);
				tbd = *ptbd;
				if (((struct TBD *)&tbd)->jc)
					{
					isz = mptjcisz[((struct TBD *)&tbd)->jc - 1];
					*pch++ = chBackslash;
					pch = PchSzRtfMove(isz, pch);
					if (isz == iszRTFtb)
						{
						Assert (fFalse);  /* never happen in Opus */
						CchIntToPpch(*pdxa, &pch);
						continue;
						}
					}
				if (((struct TBD *)&tbd)->tlc)
					{
					isz = mptlcisz[((struct TBD *)&tbd)->tlc - 1];
					*pch++ = chBackslash;
					pch = PchSzRtfMove(isz, pch);
					}
				*pch++ = chBackslash;
				pch = PchSzRtfMove(iszRTFtx, pch);
				CchIntToPpch(*pdxa, &pch);
				}
			}
		break;
#ifdef DEBUG
	default:

#ifdef BZ
		CommSzNum(SzShared("FPropToRtfSpecMacPlus invalid irrb = "), irrb);
#endif
		Assert(fFalse);
#endif
		}
	*ppch = pch;
	return (fFalse);
}



/*  %%Function:  FEmitForChpx  %%Owner:  bobz       */

FEmitForChpx(pchpx, irrb)
struct CHP *pchpx;
int irrb;
{
	switch (irrb)
		{
	case irrbRb:
		return (pchpx->fBold);
	case irrbRi:
		return (pchpx->fItalic);
	case irrbRstrike:
		return (pchpx->fStrike);
	case irrbRoutl:
		return (pchpx->fOutline);
#ifdef MACONLY   /* opus does not have fShadow any more and has no UI for caps */
	case irrbRshad:
		return (pchpx->fShadow);
	case irrbRcaps:
		return (pchpx->fCaps);
#endif /* MACONLY */
	case irrbRscaps:
		return (pchpx->fSmallCaps);
	case irrbRv:
		return (pchpx->fVanish);
	case irrbRf:
		return (pchpx->fsFtc);
	case irrbRfs:
		return (pchpx->fsHps);
	case irrbRkul:
		return (pchpx->fsKul);
	case irrbRup:
		return (pchpx->fsPos);
	case irrbRexpnd:
		return (pchpx->fsSpace);
	case irrbRrevised:
		return (pchpx->fRMark);
	case irrbRcf:
		return (pchpx->fsIco);
	default:
		Assert (fFalse);
		return (0);
		}
}


/*  %%Function:  OutHdrs  %%Owner:  bobz       */

OutHdrs(prebl, docHdr, ihddMin, ihddMac, grpfIhdt, ihdtMin, fFacingPages,
fTitlePage, roo)

struct REBL *prebl;
int docHdr, ihddMin, ihddMac, grpfIhdt, ihdtMin, fFacingPages, fTitlePage, roo;
{
	int ihdt;
	for (ihdt = 0; ihddMin < ihddMac; ihdt++, ihddMin++)
		{
		int isz;
		char *pch = prebl->rgch;
		struct CA ca;

		while ((grpfIhdt & (1 << ihdt)) == 0)
			{
			ihdt++;
			}

		Assert(ihdt <= 8);

		isz = mpihdtisz[ihdt + ihdtMin];
		if (!fFacingPages)
			{
			if (isz == iszRTFfooterr)
				isz = iszRTFfooter;
			else  if (isz == iszRTFheaderr)
				isz = iszRTFheader;
			/* unlike mac word, we throw out the left h/f in this case too */
			else  if (isz == iszRTFfooterl || isz == iszRTFheaderl)
				continue;
			}
		if (!fTitlePage &&
				(isz == iszRTFfooterf || isz == iszRTFheaderf))
			continue;

		*pch++ = chRTFOpenBrk;

		*pch++ = chBackslash;
		pch = PchSzRtfMove(isz, pch);
		*pch++ = ' ';
		FlushRebl(prebl, &pch);

		CaFromIhdd(docHdr, ihddMin, &ca);
		RtfOut(&ca, prebl->pfnWrite, prebl->pArgWrite, (roo|rooSub)&roomSub);

		*pch++ = chRTFCloseBrk;
		FlushRebl(prebl, &pch);
		}
}


/*  %%Function:  RtfVersion  %%Owner:  bobz       */

RtfVersion(prebl)
struct REBL *prebl;
{
	char *pch;

	bltbx((CHAR FAR *)rgchRTF,  /* defined in rtf.h */
			(CHAR FAR *)(prebl->rgch), sizeof(rgchRTF));

	pch = prebl->rgch + sizeof(rgchRTF);
	CchUnsToPpch(wVersionRTF, &pch);

	*pch++ = chBackslash;
	pch = PchSzRtfMove(iszRTFansi, pch);
	*pch++ = ' ';

	*pch++ = chBackslash;
	pch = PchSzRtfMove(iszRTFdeff, pch);
	CchUnsToPpch(ftcDefault, &pch);

	vcchEopRtf = pch - prebl->rgch;
	FlushRebl(prebl, &pch);
}


/*  %%Function:  CchRTFFormatDttmPch   %%Owner:  bobz       */

CchRTFFormatDttmPch (pdttm, pchFormat)
struct DTTM *pdttm;
CHAR *pchFormat;

{
	CHAR *pch = pchFormat;
	struct DTR dtr;


	DtrNum (&dtr, pdttm);

	if (dtr.yr != dyrBase)
		{
		*pch++ = chBackslash;
		pch = PchSzRtfMove(iszRTFyr, pch);
		CchIntToPpch(dtr.yr, &pch);
		}
	if (dtr.mon != 0)
		{
		*pch++ = chBackslash;
		pch = PchSzRtfMove(iszRTFmo, pch);
		CchIntToPpch(dtr.mon, &pch);
		}
	if (dtr.dom != 0)
		{
		*pch++ = chBackslash;
		pch = PchSzRtfMove(iszRTFdy, pch);
		CchIntToPpch(dtr.dom, &pch);
		}

	/* add in time part */
	if (dtr.hr != 0)
		{
		*pch++ = chBackslash;
		pch = PchSzRtfMove(iszRTFhr, pch);
		CchIntToPpch(dtr.hr, &pch);
		}
	if (dtr.mint != 0)
		{
		*pch++ = chBackslash;
		pch = PchSzRtfMove(iszRTFmin, pch);
		CchIntToPpch(dtr.mint, &pch);
		}
	if (dtr.sec != 0)
		{
		*pch++ = chBackslash;
		pch = PchSzRtfMove(iszRTFsec, pch);
		CchIntToPpch(dtr.sec, &pch);
		}

	return (pch - pchFormat);


}


/*  %%Function:  TjcToRtf  %%Owner:  bobz       */

TjcToRtf(ppch, jc)
char **ppch;
int jc;
{
	char *pch = *ppch;

	*pch++ = chBackslash;
	pch = PchSzRtfMove(mptjcisz[jc - 1], pch);
	*ppch = pch;
}


/*  %%Function:  TlcToRtf  %%Owner:  bobz       */

TlcToRtf(ppch, tlc)
char **ppch;
int tlc;
{
	char *pch = *ppch;

	*pch++ = chBackslash;
	pch = PchSzRtfMove(mptlcisz[tlc - 1], pch);
	*ppch = pch;
}


/* W r i t e  F n  D s r s  R T F */
/* Part of SAVE, used only for RTF, moved here to help SAVE compile
	by not needing rtf.h */
/*  %%Function:  WriteFnDsrsRTF  %%Owner:  bobz       */

WriteFnDsrsRTF( fn, doc )
int fn;
int doc;
{
	struct CA ca;
	struct RARF rarf;

	ca.doc = doc;
	ca.cpFirst = cp0;
	ca.cpLim = CpMacDoc( doc );

	rarf.fn = fn;
	RtfOut(&ca, &AppendRgchToFnRtf, &rarf, rooAll);
	/* if disk full, fn has been deleted by caller */
}



/* H  W R I T E  R T F */
/*  Write doc, cpFirst:cpLim to a windows handle in RTF format.
	Used by dde and clipboard
*/
/*  %%Function:  HWriteRTF   %%Owner:  bobz       */

HWriteRTF (doc, cpFirst, cpLim, cbInitial)
int doc;
CP cpFirst, cpLim;
int cbInitial;

{

	struct CA ca;
	struct RHPCCHW rhpcchw;
	LPCH lpch;
	HANDLE h;

#ifdef INFOONLY
	struct RHPCCHW   /* used in some RtfOut calls */
		{
		HANDLE h;
		long lcbMac;   /* current handle used size */
		long lcbMax;   /* current handle allocated size */
		int wAlloc;     /* GMEM for allocations */
		};
#endif /* INFOONLY */

	/*  for internationalization we require RTF to know field types,
		be sure all field types are known */
	AssureAllFieldsParsed (doc);

	rhpcchw.wAlloc = cbInitial ? GMEM_DDE : GMEM_MOVEABLE;
	rhpcchw.h = hNil;  /* amount used */
	rhpcchw.lcbMac = 0;  /* amount used */
	rhpcchw.lcbMax = 0;  /* amount allocated */

	if (!FAssureHCbGlob( &rhpcchw.h, (long)(1+cbInitial), rhpcchw.wAlloc,
			&rhpcchw.lcbMac, &rhpcchw.lcbMax) )
		return (hNil);

	/* docScrap does not normally have styles and fonts, but a pointer
		to a doc that has them. CheckScrapStsh will copy the styles
		and fonts from that doc into docScrap so he rtfout will have
		them to put out.
	*/
	if (doc == docScrap)
		if (vsab.docStsh != docNil)
			CheckScrapStsh(vsab.docStsh);

	RtfOut(PcaSet(&ca, doc, cpFirst, cpLim), &AppendRgchToHandleRtf,
			&rhpcchw, rooAll & ~rooInfo);

	/* Since Windows rounds global handles to 16 byte boundaries we must
		null terminate the string. We allocated space for the null in
		the FAssure HCbGlob call above
	*/

	h = rhpcchw.h;

	if (vmerr.fMemFail || h == hNil)
		goto Failed;

	/* now realloc to free up any extra allocated memory */
	if (rhpcchw.lcbMax != rhpcchw.lcbMac)
		{
		Assert (rhpcchw.lcbMax > rhpcchw.lcbMac);
		if ((h = OurGlobalReAlloc( h, (LONG) (rhpcchw.lcbMac),
				rhpcchw.wAlloc )) == NULL)
			{
			h = rhpcchw.h; /* should never happen, making smaller */
			goto Failed;
			}
		}

	if ((lpch = GlobalLockClip( h )) == NULL)
		goto Failed;

	lpch [rhpcchw.lcbMac - 1] = '\0';
	GlobalUnlock( h );
	return h;

Failed:

	if (h != hNil)
		GlobalFree( h );

	return hNil;
}


/*  D C H  T E X T  T O  P L A I N  T E X T */

/* take a buffer of data that might include special chars and
	escape special chars.

	Return: # of source chars used
		ppchDest points past last char in dest buffer
		ppchSrc points to next source character
*/
/*  %%Function:  DchTextToPlainText  %%Owner:  bobz       */

int DchTextToPlainText(ppchDest, ppchSrc, cchSrc, cchDestMax)
char **ppchDest, **ppchSrc;
int cchSrc, cchDestMax;
{
	int cchDest = 0;
	int dchSrc = 0;
	int ch;
	char *pchSrc = *ppchSrc;
	char *pchDest = *ppchDest;
	/* note we leave room for 2 chars always so we can always
		fit in an escaped char
	*/
	while (cchDest < cchDestMax - 1 && dchSrc < cchSrc)
		{
		ch = *pchSrc++;
		/* escape spec chars */
		if ( ch == chRTFOpenBrk || ch == chRTFCloseBrk ||
				ch == chBackslash)
			{
			*pchDest++ = chBackslash;
			++cchDest;
			}

		*pchDest++ = ch;
		++cchDest;
		++dchSrc;
		}

	*ppchSrc = pchSrc;
	*ppchDest = pchDest;
	return (dchSrc);
}


/*  %%Function:  FlushRebl  %%Owner:  bobz       */

FlushRebl(prebl, ppch)
struct REBL *prebl;
char **ppch;
{
	int cb;

	cb = *ppch-prebl->rgch;
	Assert(cb <= cchMaxSz - 1);
	if (!vfRTFInternal)
		EnsureRtfChEop(prebl, cb);
	(*prebl->pfnWrite)(prebl->pArgWrite, prebl->rgch, cb);
	*ppch = prebl->rgch;
}


/*  %%Function:  FlushReblPch  %%Owner:  bobz       */

FlushReblPch(prebl, pch, pchStart)
struct REBL *prebl;
char *pch;
char *pchStart;
{
	int cb;

	cb = pch - pchStart;
	if (cb > 0)
		{
		Assert(cb <= cchMaxSz - 1);
		if (!vfRTFInternal)
			EnsureRtfChEop(prebl, cb);
		(*prebl->pfnWrite)(prebl->pArgWrite, pchStart, cb);
		}
}


/*  %%Function:  EnsureRtfChEop  %%Owner:  bobz       */

EnsureRtfChEop(prebl, cb)
struct REBL *prebl;
int cb;
{
	char rgb[2];

/* if we're writing an external RTF file, make sure we emit a chEop every
	cchMaxLineRTF bytes (255 Mac,Opus fast, 79 Opus debug), for compatiblity
	with various email systems.
*/
	if (vcchEopRtf + cb > cchMaxLineRTF )
		{
		(*prebl->pfnWrite)(prebl->pArgWrite, rgchEop, (int)ccpEol);

		vcchEopRtf = 0;
		}
	vcchEopRtf += cb;
}


/*  %%Function:  AppendRgchToFnRtf  %%Owner:  bobz       */

void AppendRgchToFnRtf(prarf, pch, cch)
struct RARF *prarf;
char *pch;
int cch;
{
#ifdef DEBUG
	if (vdbs.fInfoAssert)
		{
		int ich = 0;
		char *pchT = pch;
		char ch;

		for (ich = 0; ich < cch; ich++)
			Assert(((ch = *pchT++) >= 0x20 && ch < 0x7F)
					|| ch == 0x0D || ch == 0x0A);
		}
#endif /* DEBUG */

	FcAppendRgchToFn(prarf->fn, pch, cch);
}


/* Add cch chars from pch to the end of the handle contained in
	prhpcch.
*/
/*  %%Function:  AppendRgchToHandleRtf  %%Owner:  bobz       */

void AppendRgchToHandleRtf(prhpcchw, pch, cch)
struct RHPCCHW *prhpcchw;
char *pch;
int cch;
{

	HANDLE h, hT;
	LPCH lpch;

	h = prhpcchw->h;
	if (h == hNil)
		return;

	/* extra byte for null at end already allocated */

	if (!FAssureHCbGlob( &h, (LONG) (prhpcchw->lcbMac + cch),
			prhpcchw->wAlloc, &prhpcchw->lcbMac, &prhpcchw->lcbMax) )
		goto Failed;

	if ((lpch=GlobalLockClip( h )) == NULL)
		goto Failed;

	bltbx( (LPCH) pch, lpch + prhpcchw->lcbMac-cch-1, cch );

	GlobalUnlock( h );

	prhpcchw->h = h;

	return;

Failed:
#ifdef BZTEST
	CommSzLong(SzShared("rtfout alloc size at fail: "),(LONG)prhpcchw->lcbMac);
	Assert (fFalse);
#endif
	SetErrorMat( matMem );

	if (h != hNil)
		GlobalFree( h );

	prhpcchw->h = hNil;
}



/*  %%Function:  SzSpaceToRtf  %%Owner:  bobz       */

SzSpaceToRtf(ppch, isz)
char **ppch;
int isz;
{
	char *pch = *ppch;

	*pch++ = chBackslash;
	pch = PchSzRtfMove(isz, pch);
	*pch++ = ' ';
	*ppch = pch;
}


/*  %%Function:  SzToRtf  %%Owner:  bobz       */

SzToRtf(ppch, isz)
char **ppch;
int isz;
{
	*(*ppch)++ = chBackslash;
	*ppch = PchSzRtfMove(isz, *ppch);
}


/*  %%Function:  SzValToRtf  %%Owner:  bobz       */

SzValToRtf(ppch, isz, val)
char **ppch;
int isz;
int val;
{
	char *pch = *ppch;

	*pch++ = chBackslash;
	pch = PchSzRtfMove(isz, pch);
	CchIntToPpch(val, &pch);
	*ppch = pch;
}


/* D T R  N U M   */
/*  %%Function:  DtrNum  %%Owner:  bobz       */

DtrNum(pdtr, pdttm)  /* convert internal date/time form to struct */
struct DTR *pdtr;
struct DTTM *pdttm;
{
	pdtr->yr   = pdttm->yr + dyrBase;
	pdtr->mon  = pdttm->mon;
	pdtr->dom  = pdttm->dom;
	pdtr->hr   = pdttm->hr;
	pdtr->mint = pdttm->mint;
	pdtr->sec  = 0;   /* not in dttm */
	pdtr->wdy  = pdttm->wdy;
}


/*  %%Function:  GetFontName  %%Owner:  bobz       */

GetFontName(ibst, st)
int ibst;
CHAR *st;
{
	/* st needs to be at least LF_FACESIZE to hold the string */

	SzToSt( ((struct FFN *)PstFromSttb(vhsttbFont, ibst))->szFfn, st );
	Assert (*st < LF_FACESIZE);
}


/*  %%Function:  GetFontFamily  %%Owner:  bobz       */

GetFontFamily(ibst, pff)
int ibst;
int *pff;
{
	*pff = ((struct FFN *)PstFromSttb(vhsttbFont, ibst))->ffid & maskFfFfid;
}


/*  %%Function:  FldsStackFlt  %%Owner:  bobz       */

FldsStackFlt(fldsIn, flt, rgfltStack, piTop)
struct FLDS fldsIn;
int flt;
struct FLDS rgfltStack[];
int * piTop;
{
	/* push previous entry on stack and return new state based on current flt */

	struct FLDS fldsOut;

	fldsOut = 0;
	fldsOut.flt = fltNil;

	Assert (flt <= 255);  /* so it fits in 8 bits */
	if (*piTop < cNestFieldMax)
		{
		rgfltStack[*piTop] = fldsIn;
		fldsOut.flt = flt; /* flags all zeroed */
		}
	/* increment even if over the top [SHOULD NOT HAPPEN] so we
	throw out higher nesting levels but pop off correctly
	*/
	(*piTop)++;
	return (fldsOut);
}


/*  %%Function:  FldsPopFlt  %%Owner:  bobz       */

FldsPopFlt(rgfltStack, piTop)
struct FLDS rgfltStack[];
int *piTop;
{
	/* if not overflowed, return entry at top of stack, else
	return 0 entry; Also 0 if stack is empty.
	*/

	struct FLDS fldsOut;

	fldsOut = 0;
	fldsOut.flt = fltNil;

	Assert (*piTop > 0);

	(*piTop)--;
	if (*piTop < cNestFieldMax && *piTop > 0)
		{
		fldsOut = rgfltStack[*piTop];
		}
	return (fldsOut);
}


