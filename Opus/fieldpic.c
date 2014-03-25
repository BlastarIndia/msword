/* F I E L D P I C . C */
/*  Code to implement the Field Format Arguments */
#define FIELDPIC_C

#ifdef PCJ
#ifdef DEBUG
/*#define SHOWFFA */
/*#define YYDEBUG*/
#endif /* DEBUG */
#endif /* PCJ */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "doc.h"
#include "field.h"
#include "props.h"
#include "ourmath.h"
#include "disp.h"
#include "ch.h"
#include "inter.h"
#include "strtbl.h"
#include "prm.h"
#include "sel.h"
#include "doslib.h"
#include "fltexp.h"
#include "pic.h"
#include "fontwin.h"
#include "screen.h"
#include "border.h"

#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "pict.hs"

#include "debug.h"

extern long LDaysFrom1900Dttm();

extern struct ITR   vitr;
extern union RRU    vrruCalc;
extern CHAR         vrrut;
extern struct CHP   vchpFetch;
extern CP           vcpFetch;
extern int          vccpFetch;
extern struct CA    caPara;
extern CHAR HUGE    *vhpchFetch;
extern int          vdocScratch;
extern CHAR         szApp [];
extern CHAR         szEmpty [];
extern CHAR 		rgchEop[];
extern struct CHP       vchpStc;
extern struct PAP       vpapFetch;
extern int          vfCustomDTPic;
extern ENV	    *penvMathError;
extern struct SEL   selCur;
extern CP           vmpitccp[];
extern struct PAP   vpapFetch;
extern struct TAP   vtapFetch;
extern struct CA    caTap;
extern struct SPX   mpiwspxTap[];
extern struct PIC   vpicFetch;
extern struct FTI	vfti;
extern struct SCI	vsci;

/*  maps ffn to codes used by other functions */
csconst int mpffnw [] =
{
	/*  arguments to MapCaseDocCps */
	/* ffnUpper */      wcsAllUpper,
			/* ffnLower */      wcsAllLower,
			/* ffnFirstcap */   wcsFirstCap,
			/* ffnCaps */       wcsCapWords,

			/*  arguments to DcpInsertIntNfc */
	/* ffnOrdinal */    nfcOrdinal,
			/* ffnRoman */      nfcLCRoman,
			/* ffnRoman+1 */    nfcUCRoman,
			/* ffnAlphabetic */ nfcLCLetter,
			/* ffnAlpha+1 */    nfcUCLetter,
			/* ffnCardtext */   nfcCardtext,
			/* ffnOrdtext */    nfcOrdtext,
			/* ffnHex */        nfcHex,
			/* ffnArabic */     nfcArabic,
			/* ffnDollartext */ nfcCardtext
};


#define WcsFromFfn(ffn) (mpffnw [ffn])
#define NfcFromFfn(ffn) (mpffnw [ffn])

#define cchMaxDateStr           10
csconst CHAR rgszMonths[][] = 
{
	SzKey("january",january),
			SzKey("february",february),
			SzKey("march",march),
			SzKey("april",april),
			SzKey("may",may),
			SzKey("june",june),
			SzKey("july",july),
			SzKey("august",august),
			SzKey("september",september),
			SzKey("october",october),
			SzKey("november",november),
			SzKey("december",december)
};


#define iszMonthMax     12

csconst CHAR rgszDays[][] = 
{
	SzKey("Sunday",Sunday),
			SzKey("Monday",Monday),
			SzKey("Tuesday",Tuesday),
			SzKey("Wednesday",Wednesday),
			SzKey("Thursday",Thursday),
			SzKey("Friday",Friday),
			SzKey("Saturday",Saturday)
};


#define iszDaysMax      7

/* D C P  A P P L Y  F O R M A T  S W */
/* %%Function:DcpApplyFormatSw %%Owner:peterj */
CP DcpApplyFormatSw (pffb, doc, cpResult, dcpNew, dcpPrev, fcr)
struct FFB *pffb;
int doc;
CP cpResult, dcpNew, dcpPrev;
int fcr;

{
	int ffn;
	int dffn = 0;
	int cch;
	ENV *penvSave;
	struct CHP chp;
	CHAR rgch [cchArgumentFetch];
	ENV env;
	struct CA ca;

	InitFvbBufs (&pffb->fvb, rgch, cchArgumentFetch, NULL, 0);
	if (!FFetchArgText (pffb, fTrue))
		return DcpReplaceWithError (doc, cpResult, dcpNew,
				istErrNoSwitchArgument);

	switch (ffn = IdFromSzgSz (szgFormatFunc, rgch))
		{
	default:
	case idNil:
		return DcpReplaceWithError (doc, cpResult, dcpNew,
				istErrUnknownSwitchArg);

	case ffnUpper:
	case ffnLower:
	case ffnFirstcap:
	case ffnCaps:
		MapCaseDocCps (WcsFromFfn(ffn), doc, cpResult, cpResult+dcpNew);
		return dcpNew;

	case ffnRoman:
	case ffnAlphabetic:
		if (rgch [0] == ChUpper (rgch [0]))
			dffn = 1;
		/* FALL THROUGH */

	case ffnOrdinal:
	case ffnCardtext:
	case ffnOrdtext:
	case ffnHex:
	case ffnArabic:
	case ffnDollartext:
		/*  get a numeric value */
		if (SetJmp(&env) != 0)
			{
			goto lblME;
			}
		else
			{
			PushMPJmpEnv(env, penvSave);
			}
		PcaSet( &ca, doc, cpResult, cpResult+dcpNew );
		if (fcr != fcrNumeric)
			{
			PES pes;
			InitPpes(&pes);
			/* parse a value */
			if (!FNumFromCps (ca.doc, ca.cpFirst, ca.cpLim,
					&vrruCalc.num, &pes, pffb->fFetchVanished,
					NULL, NULL))
				/* isn't a number, do nothing */
				return DcpCa(&ca);
			}
		else  if (vrrut != rrutNum && !FConvertRrutLongToRrutNum())
			goto lblME;

		/* delete the old result */
		FDelete(&ca);

		/* insert the formatted number */
		GetResultChp (pffb, &chp);
		cch = CchInsertLongNfc (doc, cpResult, LWholeFromNum
				(&vrruCalc.num, ffn == ffnDollartext),
				NfcFromFfn (ffn+dffn), &chp);
		if (cch == 0)
			{
lblME:
			/* could not represent number */
			cch = CchInsertFieldError (doc, cpResult, istErrInvalidNumber);
			}
		else  if (ffn == ffnDollartext)
			/* add the fractional part */
			cch += CchInsertFract (doc, cpResult+cch, &vrruCalc.num, &chp);
		PopMPJmp(penvSave);
		return (CP) cch;

	case ffnCharFormat:
		GetResultChp (pffb, &chp);
		ApplyChpDocCps (&chp, doc, cpResult, cpResult+dcpNew);
		return dcpNew;

	case ffnMergeFormat:
		MergeResultProps (doc, cpResult, cpResult+dcpNew, 
				doc, cpResult+dcpNew, cpResult+dcpNew+dcpPrev, fFalse);
		return dcpNew;

		}
}


/* %%Function:FConvertRrutLongToRrutNum %%Owner:peterj */
FConvertRrutLongToRrutNum()
{
	int	fReturn;
	ENV	*penvSave;
	ENV	env;
	long    l = vrruCalc.l;

	Assert(vrrut == rrutLong);

	if (SetJmp(&env) != 0)
		{
		num0;
		StiNum(&vrruCalc.num);
		fReturn = fFalse;
		}
	else
		{
		PushMPJmpEnv(env, penvSave);
		MakeNum (&vrruCalc.num, l, (LONG)0, 0);
		fReturn = fTrue;
		vrrut = rrutNum;
		}
	PopMPJmp(penvSave);
	return fReturn;
}






/* A P P L Y  C H P  D O C  C P S */
/*  Make all characters from cp to cp+dcp have the character properties
	*pchp.  NOTE: does not change fFldVanish, fSysVanish or fSpec!
*/

/* %%Function:ApplyChpDocCps %%Owner:peterj */
ApplyChpDocCps (pchp, doc, cp, cpLim)
struct CHP *pchp;
int doc;
CP cp, cpLim;

{
	int cb;
	struct CA ca;
	CHAR grpprl [cbMaxAllOtherSprms];

	Assert (pchp != &vchpFetch); /* avoid temptation! */
	ca.doc = doc;

	while (cp < cpLim)
		{
		CachePara (doc, cp);
		FetchCp (doc, cp, fcmProps);
		Assert (cp == vcpFetch);

		(*(int *)pchp) = 
				((*(int *)pchp) & ~maskFNonUser) | 
				((*(int *)&vchpFetch) & maskFNonUser);
		pchp->fSysVanish = vchpFetch.fSysVanish;

		/*  returns grpprl to step vchpFetch to pchp */
		if ((cb = CbGrpprlFromChp (grpprl, pchp, &vchpFetch)) > 0)
			{
			ca.cpFirst = cp;
			ca.cpLim = CpMin (cp+vccpFetch, cpLim);
			ApplyGrpprlCa (grpprl, cb, &ca);
			InvalCp(&ca);
			}

		cp += vccpFetch;
		}
}



CP CpLimWord ();



/* M E R G E  R E S U L T  P R O P S */
/*  Merge properties of docSrc, cpSrc, dcpSrc with docDest, cpDest, dcpDest.
	Merge is word by word within each paragraph (resets on each paragraph).
*/

/* %%Function:MergeResultProps %%Owner:rosiep */
MergeResultProps (docDest, cpDest, cpLimDest, docSrc, cpSrc, cpLimSrc, fInTableCell)
int docDest;
CP cpDest, cpLimDest;
int docSrc;
CP cpSrc, cpLimSrc;
BOOL fInTableCell;
{
	CP cpLimWSrc, cpLimWDest; /* lim of current word */
	CP cpLimPSrc=cpSrc, cpLimPDest=cpDest; /* lim of current paragraph */
	struct CHP chp;
	char grpprl[cbMaxGrpprl];
	struct PAP papSrc;
	int cb;
	BOOL fInTableSrc, fInTableDest;

	for (;;)
		{
		/*  termination conditions: out of src, out of dest */
		if (cpSrc >= cpLimSrc && cpDest < cpLimDest || cpDest >= cpLimDest)
			return;

		/*  get paragraph limits */
		if (cpSrc >= cpLimPSrc || cpDest >= cpLimPDest)
			{
			Assert (cpSrc == cpLimPSrc || cpDest == cpLimPDest);
			CachePara (docSrc, cpSrc);
			cpLimPSrc = CpMin (caPara.cpLim, cpLimSrc);
			fInTableSrc = FInTableVPapFetch(docSrc, cpSrc) && !fInTableCell;
			papSrc = vpapFetch;
			CachePara (docDest, cpDest);
			cpLimPDest = CpMin (caPara.cpLim, cpLimDest);
			fInTableDest = FInTableVPapFetch(docDest, cpDest) && !fInTableCell;
			if (!fInTableSrc && !fInTableDest &&
					(cb = CbGrpprlFromPap(fFalse, &grpprl, &papSrc, &vpapFetch /* papDest */, fFalse)) > 0)
				{
				ApplyGrpprlCa(grpprl, cb, &caPara /* Dest */);
				InvalCp(&caPara);
				}
			}

		if (fInTableSrc && !fInTableDest)
			{
			while (!fInTableDest && cpLimPDest < cpLimDest)
				{
				CachePara(docDest, cpLimPDest);
				cpLimPDest = caPara.cpLim;
				fInTableDest = FInTableVPapFetch(docDest, cpLimPDest-1);
				}
			if (!fInTableDest)
				return;
			cpDest = caPara.cpFirst;
			}
		else  if (fInTableDest && !fInTableSrc)
			{
			while (!fInTableSrc && cpLimPSrc < cpLimSrc)
				{
				CachePara(docSrc, cpLimPSrc);
				cpLimPSrc = caPara.cpLim;
				fInTableSrc = FInTableDocCp(docSrc, cpLimPSrc-1);
				}
			if (!fInTableSrc)
				return;
			cpSrc = caPara.cpFirst;
			}

		if (fInTableSrc && fInTableDest)
			{
			MergeTableProps(docSrc, cpSrc, docDest, cpDest);
			CpFirstTap(docSrc, cpSrc);
			cpSrc = cpLimPSrc = caTap.cpLim;
			CpFirstTap(docDest, cpDest);
			cpDest = cpLimPDest = caTap.cpLim;
			continue;
			}

		/*  find limits of current word */
		cpLimWDest = CpMin (CpLimWord (docDest, cpDest), cpLimPDest);
		cpLimWSrc = CpMin (CpLimWord (docSrc, cpSrc), cpLimPSrc);

		/*  apply src word props to dest word */
		CachePara(docSrc, cpSrc);
		FetchCp(docSrc, cpSrc, fcmProps+fcmChars);
		chp = vchpFetch;
		ApplyChpDocCps (&chp, docDest, cpDest, cpLimWDest);

		/* do picture properties */
		/* be really careful that the fetchcps are done in the right
		   order here. FetchPe needs vchpFetch and fnFetch set up
		   by a fetchcp to work.
		*/
		CachePara(docSrc, cpSrc);
		FetchCp(docSrc, cpSrc, fcmProps+fcmChars);
		if (vchpFetch.fSpec && *vhpchFetch == chPicture)
			{
	        struct PIC pic;
			 /* source pic for original props */
			FetchPe(&vchpFetch, docSrc, cpSrc);
			pic = vpicFetch;
			/* will just return if cpDest not a pic. */
			MergePicProps(docDest, cpDest, &pic);
			}

		/*  advance beyond word */
		cpSrc = cpLimWSrc;
		cpDest = cpLimWDest;

		/*  see if we ran out of paragraph */
		if (cpSrc == cpLimPSrc && cpDest < cpLimPDest)
			cpDest = cpLimPDest;
		}
}




/*  M E R G E  P I C  P R O P S  */
/* %%Function:MergePicProps %%Owner:bobz */
/* Merge picture properties from a previous picture to a new one, checking
   that such properties, such as cropping, are valid with the new picture.
   Called at least 2 places, so a routine.
*/

MergePicProps(doc, cpFirst, ppic)
int doc;
CP cpFirst;
struct PIC *ppic;
{
	struct CA ca;

	/* props for picture formatting applied to FCheckPic 
		needs vpicFetch set up 
	*/

	FetchCpAndPara(doc, cpFirst, fcmProps+fcmChars);
   if (!(vchpFetch.fSpec && *vhpchFetch == chPicture))
		return;

	FetchPe(&vchpFetch, doc, cpFirst);

	/* ignore formatting if it would be illegal */
	if (FCheckPic(ppic, PcaSetDcp(&ca, doc, cpFirst, (CP)1)))
		{
		/* FUTURE change ChangePicFrame to take a ppic */
        ChangePicFrame(&ca,
				ppic->brcl,
				ppic->mx,
				ppic->my,
				ppic->dyaCropTop,
				ppic->dxaCropLeft,
				ppic->dyaCropBottom,
				ppic->dxaCropRight);
        }
}





/*  M E R G E  T A B L E  P R O P S  */
/* %%Function:MergeTableProps %%Owner:rosiep */
/* Merge table properties from one row to another row.  Used when we have
   the "\* mergeformat" switch.  The properties are done in groups, with
   a call to ApplyGrpprlCa for each group.  This is to keep the sprm's
   sorted in the grpprl.  Checking is also done to make sure the maximum
   width isn't exceeded, and if it would be, we leave the dxa properties
   alone.
*/

MergeTableProps(docSrc, cpSrc, docDest, cpDest)
int docSrc, docDest;
CP cpSrc, cpDest;
{
	CP mpitccpSrc[itcMax], mpitccpDest[itcMax];
	struct TAP tapSrc, tapDest;
	char grpprl[cbMaxGrpprlTable];
	struct CA caTapDest, caT;
	int cb = 0;
	int itcSrc, itcDest, itc, itcMac, itcFirst;
	int dxaLeft, dxaSrc, dxaDest, dxaPrev;
	int *pdxaSrc, *pdxaDest, *pdxaLim;
	struct TC *ptcSrc, *ptcDest;
	BOOL fMerge, fDiffer;
	int brcSrc, brcDest, brcPrev;
	int ibrc;

#ifdef DEBUG
	{
	extern struct ESPRM	dnsprm[];

	// assert that cbMaxGrpprlTable is big enough to hold the
	// largest grpprl we can generate here. Update these if
	// more sprms are ever output  bz

	Assert (cbMaxGrpprlTable >=
			dnsprm[sprmTDxaLeft].cch +
			dnsprm[sprmTDxaGapHalf].cch	+
			(itcMax * dnsprm[sprmTDxaCol].cch));
	Assert (cbMaxGrpprlTable >=
			dnsprm[sprmTJc].cch	+
			dnsprm[sprmTDyaRowHeight].cch +
			(itcMax * dnsprm[sprmTSetBrc].cch));
	}

#endif /* DEBUG */

	/* save mpitccp's to avoid doing CpFirstTap back and forth between tables */
	CpFirstTap(docSrc, cpSrc);
	bltbyte(vmpitccp, mpitccpSrc, (vtapFetch.itcMac + 1) * sizeof(CP));
	tapSrc = vtapFetch;
	CpFirstTap(docDest, cpDest);
	bltbyte(vmpitccp, mpitccpDest, (vtapFetch.itcMac + 1) * sizeof(CP));
	tapDest = vtapFetch;
	caTapDest = caTap;
	itcMac = min(tapSrc.itcMac, tapDest.itcMac);

	/* Check for maximum width violation */
	pdxaSrc = &tapSrc.rgdxaCenter[0];
	pdxaLim = pdxaSrc + tapSrc.itcMac + 1;
	if (tapDest.itcMac > tapSrc.itcMac &&
			(*(pdxaLim-1) > xaRightMaxSci - (tapDest.rgdxaCenter[tapDest.itcMac] -
			tapDest.rgdxaCenter[tapSrc.itcMac]) - dxTableBorderMax /* Ttp */))
		{
		goto LBagDxa;
		}

	/* Check for minimum width violation and split up width of cells
		preceding a run of zero-width cells among the whole group (it
		is assumed to be a merged group in the src, but we don't keep
		the merged property for the dest) */
	for (pdxaSrc++; pdxaSrc < pdxaLim; pdxaSrc++)
		{
		Assert(*pdxaSrc < xaRightMaxSci);
		if (*pdxaSrc == *(pdxaSrc-1))
			{
			int dxaCol;
			int *pdxaFirst = pdxaSrc-1;

			while (pdxaSrc < pdxaLim && *pdxaSrc == *(pdxaSrc-1))
				pdxaSrc++;
			Assert(pdxaFirst > &tapSrc.rgdxaCenter[0]);
			/* if width of col will be less than gap, leave row dxa's alone */
			if ((dxaCol = (*pdxaFirst - *(pdxaFirst-1))/(pdxaSrc - pdxaFirst))
					<= 2*tapSrc.dxaGapHalf)
				{
				goto LBagDxa;
				}
			/* split up merged width among the cells */
			while (pdxaFirst < pdxaSrc-1)
				{
				*pdxaFirst = *(pdxaFirst-1) + dxaCol;
				pdxaFirst++;
				}
			}
		}


	/* take care of sprmTDxaLeft */
	if ((dxaLeft = tapSrc.rgdxaCenter[0] + tapSrc.dxaGapHalf) !=
			tapDest.rgdxaCenter[0] + tapDest.dxaGapHalf)
		{
		grpprl[cb++] = sprmTDxaLeft;
		MovePwToPch(&dxaLeft, &grpprl[cb]);
		cb += sizeof(int);
		}

	/* take care of sprmTDxaGapHalf */
	if (tapSrc.dxaGapHalf != tapDest.dxaGapHalf)
		{
		grpprl[cb++] = sprmTDxaGapHalf;
		MovePwToPch(&tapSrc.dxaGapHalf, &grpprl[cb]);
		cb += sizeof(int);
		}

	/* take care of sprmTDxaCol */
	pdxaSrc = &tapSrc.rgdxaCenter[0];
	pdxaDest = &tapDest.rgdxaCenter[0];
	for (itc = 0, fDiffer = fFalse; itc <= itcMac; itc++, pdxaSrc++, pdxaDest++)
		{
		if (itc < itcMac)
			{
			dxaSrc = *(pdxaSrc+1) - *pdxaSrc;
			dxaDest = *(pdxaDest+1) - *pdxaDest;
			}
		if (fDiffer && (itc == itcMac || dxaSrc != dxaPrev || dxaSrc == dxaDest))
			{
			grpprl[cb++] = sprmTDxaCol;
			grpprl[cb++] = itcFirst;
			grpprl[cb++] = itc;
			MovePwToPch(&dxaPrev, &grpprl[cb]);
			cb += sizeof(int);
			fDiffer = dxaSrc != dxaDest;
			itcFirst = itc;
			}
		else  if (!fDiffer && itc < itcMac && dxaSrc != dxaDest)
			{
			fDiffer = fTrue;
			itcFirst = itc;
			}
		dxaPrev = dxaSrc;
		}

	if (cb > 0)
		{
		ApplyGrpprlCa(grpprl, cb, &caTapDest);
		InvalCp(&caTapDest);
		cb = 0;
		}

LBagDxa:
	/* take care of sprmTJc */
	if (tapSrc.jc != tapDest.jc)
		{
		grpprl[cb++] = sprmTJc;
		MovePwToPch(&tapSrc.jc, &grpprl[cb]);
		cb += sizeof(int);
		}

	/* take care of sprmTDyaRowHeight */
	if (tapSrc.dyaRowHeight != tapDest.dyaRowHeight)
		{
		grpprl[cb++] = sprmTDyaRowHeight;
		MovePwToPch(&tapSrc.dyaRowHeight, &grpprl[cb]);
		cb += sizeof(int);
		}

	/* take care of sprmTSetBrc */
	for (ibrc = 0; ibrc < ibrcTcLim; ibrc++)
		{
		ptcSrc = &tapSrc.rgtc[0];
		ptcDest = &tapDest.rgtc[0];
		for (itc = 0, fDiffer = fFalse; itc <= itcMac; itc++, ptcSrc++, ptcDest++)
			{
			if (itc < itcMac)
				{
				brcSrc = ptcSrc->rgbrc[ibrc];
				brcDest = ptcDest->rgbrc[ibrc];
				}
			if (fDiffer && (itc == itcMac || brcSrc != brcPrev || brcSrc == brcDest))
				{
				grpprl[cb++] = sprmTSetBrc;
				grpprl[cb++] = itcFirst;
				grpprl[cb++] = itc;
				grpprl[cb++] = 1<<ibrc;
				MovePwToPch(&brcPrev, &grpprl[cb]);
				cb += sizeof(int);
				fDiffer = brcSrc != brcDest;
				itcFirst = itc;
				}
			else  if (!fDiffer && itc < itcMac && brcSrc != brcDest)
				{
				fDiffer = fTrue;
				itcFirst = itc;
				}
			brcPrev = brcSrc;
			}
		}

	if (cb > 0)
		{
		ApplyGrpprlCa(grpprl, cb, PcaSet(&caT, docDest, caTapDest.cpLim-ccpEop,
				caTapDest.cpLim));
		InvalCp(&caT);
		cb = 0;
		}

	/* take care of para and char properties within the cells */
	for (itc = 0; itc < itcMac; itc++)
		{
		MergeResultProps(docDest, mpitccpDest[itc], mpitccpDest[itc+1],
				docSrc, mpitccpSrc[itc], mpitccpSrc[itc+1], fTrue);
		}
}


/* C P  L I M  W O R D */
/*  Scan forward for the end of the next word unit.  A word unit is
	contigious text, contigious white space (spaces & tabs). Any other 
	character (punctuation, paragraph marks, pictures) is its own word.
*/

/* %%Function:CpLimWord %%Owner:peterj */
CP CpLimWord (doc, cp)
int doc;
CP cp;

{
	CHAR HUGE *hpch=NULL, HUGE *hpchMac=NULL;
	BOOL fText = fFalse;
	BOOL fWhite = fFalse;

	CachePara (doc, cp);

	while (cp < caPara.cpLim)
		{
		if (hpch >= hpchMac)
			{
			Assert (hpch == hpchMac);
			Assert (cp < caPara.cpLim);
			FetchCp (doc, cp, fcmProps+fcmChars);
			if (vchpFetch.fSysVanish)
				{
				cp += vccpFetch;
				continue;
				}
			hpch = vhpchFetch;
			hpchMac = hpch + vccpFetch;
			}

		if (FTextCh (*hpch))
			if (fText || !fWhite)
				{
				fText = fTrue;
				cp++;
				hpch++;
				continue;
				}
			else
				return cp;

		switch (*hpch)
			{
		case chTab:
		case chSpace:
		case chCRJ:
			if (fWhite || !fText)
				{
				fWhite = fTrue;
				cp++;
				hpch++;
				continue;
				}
			else
				return cp;
			}

		if (!fText && !fWhite)
			return cp+1;
		else
			return cp;

		}
}




/* F  T E X T  C H*/
/*  Return true if ch is considered part of a word.
*/

/* %%Function:FTextCh %%Owner:peterj */
FTextCh (ch)
CHAR ch;

{
	if (FAlphaNum(ch))
		return fTrue;

	switch (ch)
		{
	case chNonReqHyphen:
	case chNonBreakSpace:
	case chNonBreakHyphen:
	case '\'':
	case chHyphen:
		return fTrue;
	default:
		return fFalse;
		}
}




/* D C P  A P P L Y  P I C T U R E  S W */

/* %%Function:DcpApplyPictureSw %%Owner:peterj */
CP DcpApplyPictureSw (ch, pffb, doc, cp, dcpOrig, fcr)
CHAR ch;
struct FFB *pffb;
int doc;
CP cp, dcpOrig;
int fcr;

{
	int istErr;
	CP dcpNew = dcpOrig;
	CHAR rgch [cchMaxPic+1];
	struct CR * rgcr [ccrMaxPic];
	struct CA caSrc, caDest;

	/*  get scratch doc */
	if (DocCreateScratch (doc) == docNil)
		return dcpOrig;

	/*  get picture argument */
	InitFvbBufs (&pffb->fvb, rgch, cchMaxPic, rgcr, ccrMaxPic);
	FetchFromField (pffb, fTrue/* fArgument */, fFalse /* fRTF */);
	rgch [pffb->cch] = '\0';

	if (pffb->fNoArg)
		{  /* no argument */
		istErr = istErrNoSwitchArgument;
		goto LError;
		}

	if (ch == chFldSwSysNumeric)
		{  /* get a number */
		if (fcr != fcrNumeric)
			{  /* not cached, pull out of text */
			PES pes;
			if (!FNumFromCps (doc, cp, cp+dcpOrig, &vrruCalc.num,
					&pes, pffb->fFetchVanished, NULL, NULL))
				/*  not numeric, ignore picture */
				goto LRet;
			}
		else  if (vrrut != rrutNum && !FConvertRrutLongToRrutNum())
			goto LError;

		}

	else
		{  /* get a date */
		Assert (ch == chFldSwSysDateTime);
		if (fcr != fcrDateTime)
			{
			if (!FDttmFromDocCp (doc, cp, cp+dcpOrig, &vrruCalc.dttm,
					pffb->fFetchVanished))
				/* not a date, ignore */
				goto LRet;
			}
#ifdef DEBUG
		else
			Assert(vrrut == rrutDttm);
#endif /* DEBUG */
		}

	/* make sure vdocScratch has the correct paragraph style... */
	CachePara(doc, cp+dcpOrig);

	if (FInsertRgch(vdocScratch, cp0, rgchEop, cchEop, &vchpStc, &vpapFetch) &&
			FFormatRruPic ((ch==chFldSwSysNumeric), &vrruCalc, rgch, pffb->doc, 
			rgcr, NULL, 0, vdocScratch, cp0, &dcpNew, &istErr))
		FReplaceCps( PcaSet(  &caDest, doc, cp, cp + dcpOrig ),
				PcaSet( &caSrc, vdocScratch, cp0, dcpNew ));
	else
		{
LError:
		dcpNew = DcpReplaceWithError (doc, cp, dcpOrig, istErr);
		}

LRet:
	ReleaseDocScratch ();
	return dcpNew;
}



/* C C H  F O R M A T  D T T M  P I C */
/*  Formats dttm according to szPic putting the result in szResult up to
	cchMax characters.
*/

/* %%Function:CchFormatDttmPic %%Owner:peterj */
CchFormatDttmPic (pdttm, szPic, szResult, cchMax)
struct DTTM *pdttm;
CHAR *szPic, *szResult;
int cchMax;

{
	CP dcp;
	int istErr;
	CHAR rgchPic [cchMaxPic+1];

	bltb (szPic, rgchPic, cchMaxPic);
	rgchPic [cchMaxPic] = '\0';

	if (FFormatRruPic (fFalse, pdttm, rgchPic, docNil, NULL, szResult,
			cchMax, docNil, cp0, &dcp, &istErr))
		return (int) dcp;
	else
		return 0;
}


/* C C H  F O R M A T  N U M  P I C */
/*  Formats num according to szPic putting the result in szResult up to
	cchMax characters.
*/

/* %%Function:CchFormatNumPic %%Owner:peterj */
CchFormatNumPic (pnum, szPic, szResult, cchMax)
NUM *pnum;
CHAR *szPic, *szResult;
int cchMax;

{
	CP dcp;
	int istErr;
	CHAR rgchPic [cchMaxPic+1];

	bltb (szPic, rgchPic, cchMaxPic);
	rgchPic [cchMaxPic] = '\0';

	if (FFormatRruPic (fTrue, pnum, rgchPic, docNil, NULL, szResult,
			cchMax, docNil, cp0, &dcp, &istErr))
		return (int) dcp;
	else
		return 0;
}





/* F  F O R M A T  R R U  P I C */
/*  Formats *prru according to szPic in docDest.  Return fTrue if
	properly formed.  Return number of result characters in pdcp or error
	number in pistErr. Format as number if fNumeric, else
	as date/time.

	This function may be used to format a memory picture to a memory string
	(docSrc, rgcr, pchp valid, szDest NULL) or a doc picture to a doc (docSrc,
	rgcr, pchp NULL, szDest valid).

	Warning: this function modifies szPic.
*/

/* %%Function:FFormatRruPic %%Owner:peterj */
FFormatRruPic (fNumeric, prru, szPic, docSrc, rgcr,
szDest, cchMaxDest, docDest, cpDest, pdcp, pistErr)
BOOL fNumeric;
union RRU *prru;
CHAR *szPic;
int docSrc;
struct CR *rgcr;
CHAR *szDest;
int cchMaxDest, docDest;
CP cpDest, *pdcp;
int *pistErr;

{
	int fpt;
	int n, cch;
	BOOL fDigitOut = fFalse;
	BOOL fMemory = szDest != NULL;
	int	wSign;
	CHAR *pch, *pchSave, rgch [cchMaxSz];
	struct PPB ppb;
	struct UPNUM upnum;
	struct CHP chpSave;
	CHAR *szDestSave=szDest;
	BOOL fDecimalIn=fFalse;
	extern BOOL vfElNumFormat;

	Assert (
			(docSrc!=docNil && rgcr!=NULL && szDest==NULL && docDest!=NULL) ^
			(docSrc==docNil && rgcr==NULL && szDest!=NULL && docDest==NULL)
			);

	*pdcp = 0;
	wSign = 0;
	if (fNumeric)
		{
		if (!vfElNumFormat)
			{
			NUM	numT;

			num0;
			StiNum(&numT);
			wSign = WApproxCmpPnum((NUM *) prru, &numT);
			}
		else
			wSign = WSignFromPnum((NUM *) prru);
		}

	InitPppb (fNumeric, wSign, szPic, rgcr, &ppb);
	if (fNumeric)
		{
		NumToPupnum (prru, &upnum, ppb.posBlw);
		}
#ifdef DISABLE_PJ /* causes problems... */
#ifdef DEBUG
	else  if (prru->dttm.lDttm != 0L && !FValidDttm (prru->dttm))
		{
		CommSz(SzShared("\007Bogus DTTM!\r\n"));
		CommSzLong(SzShared("Dttm = "), prru->dttm);
		Assert (fFalse);
		}
#endif /* DEBUG */
#endif

	pch = pchSave = rgch;

#define FZero(upnum) (upnum.wExp == 0 && upnum.wSign == 0 && upnum.cchSig == 1 && *upnum.rgch == '0')

	if (vfElNumFormat && FZero(upnum))
	/* check for underflow and convert to sc notation when macros are dealing
	with numbers (kcm) */
		{
		Assert(fNumeric);
		Assert(*szPic == '-');

		NumToPupnum(prru, &upnum, cchFormatNumMax-2); /* nos less than 
		10e-99 or 1e-98 are 0 */

		if (FZero(upnum)) /* give up and return */
			{
			*szDest++ = chSpace;
			*szDest = '0';
			*pdcp = 2;
			vfElNumFormat = fFalse;
			return fTrue;
			}
		else
			{
			int cchBase = 6; /* 5 digits and a dec pt. */
			int cchLimit = min(upnum.cchSig-1, cchBase-2); /* no of non zero
			chars after dec pt */
			int wExp = 0;
			int ich;

			Assert(cchMaxSig <= cchMaxSz); /* upnum's rgch should fit in function's rgch */
			Assert(upnum.wExp <= -1 && upnum.wExp > -1000);
			Assert(upnum.cchSig >= 1);

			bltb(upnum.rgch, rgch, upnum.cchSig);
			if (upnum.cchSig > cchBase-1)
				RoundRgch(rgch, upnum.cchSig, cchBase-1, &wExp);
			wExp = (wExp == upnum.cchSig) ? -upnum.wExp : (-upnum.wExp + 1);

			Assert(fMemory);
			*szDest++ = (upnum.wSign < 0) ? chNegative : chSpace;
			*szDest++ = rgch[0];
			*szDest++ = '.';
			for (ich = 1; ich <= cchBase-2; ++ich)
				*szDest++ = (ich <= cchLimit) ? rgch[ich] : '0';
			Assert(wExp >= 0 && wExp < 1000);
			if (wExp)
				{
				*szDest++ = 'E';
				*szDest++ = '-';
				if (wExp >= 100)
					{
					*(szDest+2)=wExp%10+'0'; 
					wExp /= 10;
					*(szDest+1)=wExp%10+'0'; 
					wExp /= 10;
					*szDest=wExp%10+'0';
					szDest += 3;
					}
				else if (wExp >= 10 && wExp < 100)
					{
					*(szDest+1)=wExp%10+'0'; 
					wExp /= 10;
					*szDest=wExp%10+'0';
					szDest += 2;
					}
				else
					*szDest++ = wExp%10+'0';
				}

			*pdcp+=(szDest-szDestSave);
			vfElNumFormat = fFalse;

			return(fTrue);
			}
		}


	for (;;)
		{
		fpt = FptNextPppb (&ppb, fNumeric, fMemory);
		n = 0;

		switch (fpt)
			{
		case fptEof:
			if (!fMemory && pchSave > rgch)
				{
				/* insert saved portion */
				Assert (pch == pchSave);
				cch = pchSave - rgch;
				Assert (cch <= cchMaxSz);
				if (FInsertRgch(docDest, cpDest,rgch,cch,&chpSave,NULL))
					*pdcp += cch;
				}
			return fTrue;

		case fptError:    /* w: istErr */
LError:
			*pistErr = ppb.wArg;
			return fFalse;

		case fptReqDigit: /* w: pos */
		case fptOptDigit:
			Assert (fNumeric);
			if (!fDigitOut && ppb.wArg > 0)
				{
				int pos = min(max(upnum.wExp, 0), cchFormatNumMax);
				/* display implied sign */
				if (ppb.fImpSign && upnum.wSign < 0)
					*pch++ = chNegative;

				/* display leading digits */
				while (pos > ppb.wArg)
					{
					FPosInNum (pos--, &upnum, pch++);
					if (ppb.fThouSep && !(pos % 3) && pos > 0)
						*pch++ = (vfElNumFormat ? ',' : vitr.chThousand);
					}
				}
			/* FALL THROUGH */
		case fptTruncDigit:
			Assert (fNumeric);
			if (!FPosInNum (ppb.wArg, &upnum, pch)
					&& fpt != fptReqDigit)
				{
				if (fpt == fptTruncDigit && ppb.wArg <= 0)
					{
					continue;
					}
				*pch++ = chSpace;
				}
			else
				{
				fDigitOut = fTrue;
				pch++;
				if (ppb.fThouSep && !((ppb.wArg-1)%3) && ppb.wArg > 1)
					*pch++ = (vfElNumFormat ? ',' : vitr.chThousand);
				}
			break;

		case fptDecimal:
			Assert (fNumeric);
			*pch++ = (vfElNumFormat ? '.' : vitr.chDecimal);
			break;

		case fptThousand:
			Assert (fNumeric && ppb.fThouSep);
			continue;

		case fptNegSign:
		case fptReqSign:
			Assert (fNumeric);
			if (upnum.wSign < 0)
				*pch++ = chNegative;
			else  if (upnum.wSign == 0 || fpt == fptNegSign)
				*pch++ = chSpace;
			else
				*pch++ = '+';
			break;

		case fptMonth2: /* 01-12 */
			n = 2;
		case fptMonth1: /* 1-12 */
			Assert (!fNumeric);
			CchUnsToPpchLevel (prru->dttm.mon, &pch, n);
			break;

		case fptMonth3: /* Jan-Dec */
			n = 3;
		case fptMonth4: /* January-December */
			Assert (!fNumeric);
			if (prru->dttm.lDttm != 0l)
				{
				/* copy n characters to pch */
				bltbx ((CHAR FAR *)rgszMonths[prru->dttm.mon-1],
						(CHAR FAR *)pch, cchMaxDateStr);
				/* months are all lower case */
				*pch = ChUpper (*pch);
				pch += n > 0 ? n : CchSz (pch)-1;
				}
			else
				CchPchToPpch (SzShared("XXX"), &pch);
			break;

		case fptDay2:   /* 01-31 */
			n = 2;
		case fptDay1:   /* 1-31 */
			Assert (!fNumeric);
			CchUnsToPpchLevel (prru->dttm.dom, &pch, n);
			break;

		case fptDay3:   /* Sun-Sat */
			n = 3;
		case fptDay4:   /* Sunday-Saturday */
			Assert (!fNumeric);
			if (prru->dttm.lDttm != 0l)
				{
				/* copy n characters to pch */
				Assert (prru->dttm.wdy < iszDaysMax);/* day of week valid */
				bltbx ((CHAR FAR *)rgszDays[prru->dttm.wdy],
						(CHAR FAR *)pch, cchMaxDateStr);
				pch += n > 0 ? n : CchSz (pch)-1;
				}
			else
				CchPchToPpch (SzShared("XXX"), &pch);
			break;

		case fptYear1: /* 00-99 */
			Assert (!fNumeric);
			CchUnsToPpchLevel (prru->dttm.yr%100, &pch, 2);
			break;

		case fptYear2: /* 1900-2156 */
			Assert (!fNumeric);
			if (prru->dttm.lDttm != 0l)
				CchUnsToPpchLevel (prru->dttm.yr+1900, &pch, 4);
			else
				CchUnsToPpchLevel (0, &pch, 4);
			break;

		case fptHour2:  /* 01-12 */
		case fptHour4:  /* 00-23 */
			n = 2;
		case fptHour1:  /* 1-12 */
		case fptHour3:  /* 0-23 */
			Assert (!fNumeric);
				{
				int nHour = prru->dttm.hr;
				if (fpt <= fptHour2)
					if (nHour == 0 && prru->dttm.lDttm != 0l)
						nHour = 12;
					else  if (nHour > 12)
						nHour -= 12;
				CchUnsToPpchLevel (nHour, &pch, n);
				break;
				}

		case fptMinute2:
			n = 2;
		case fptMinute1:
			Assert (!fNumeric);
			CchUnsToPpchLevel (prru->dttm.mint, &pch, n);
			break;

		case fptAMPMLC:
		case fptAMPMUC:
			Assert (!fNumeric);
				{
				CHAR *pchAMPM = vitr.rgszMornEve 
						[prru->dttm.hr >= 12 ? iszEve : iszMorn];

				for (n = ppb.wArg; n > 0; n--, pchAMPM++)
					if (!*pchAMPM)
						break;
					else
						*pch++ = fpt==fptAMPMUC ? ChUpper(*pchAMPM):*pchAMPM;
				break;
				}

		case fptQuote:      /* w: cch, pch: pchText */
			if (!fMemory)
				{
				/* insert saved portion */
				if (pchSave > rgch)
					{
					Assert (pch == pchSave);
					cch = pchSave - rgch;
					Assert (cch <= cchMaxSz);
					if (FInsertRgch(docDest,cpDest,rgch,cch,&chpSave,NULL))
						{
						cpDest += cch;
						*pdcp += cch;
						}
					pchSave = pch = rgch;
					}

				/*  copy new portion */
				cpDest += DcpCopyIchCchPcr (docDest, cpDest, docSrc,
						rgcr, ccrMaxPic, ppb.pchArg-ppb.rgch, 
						ppb.wArg);
				*pdcp += ppb.wArg;
				continue;
				}

			else
				{
				bltb (ppb.pchArg, pch, ppb.wArg);
				pch += ppb.wArg;
				break;
				}

		case fptSequence:   /* pch: szSeqId, cp: cpRef */
				{
				int docMother;
				int w;
				CP cpMother;
				Assert (docSrc != docNil);
				GetPdocPcpMotherFromDocCp(docSrc, ppb.cpArg, 
						&docMother, &cpMother);
				w = WSequenceValue(ppb.pchArg, docMother, cpMother, fFalse);
				CchIntToPpch (w, &pch);
				break;
				}

#ifdef DEBUG
		default:
			Assert (fFalse);
			continue;
#endif /* DEBUG */
			}

		if (!fMemory)
			{
			struct CHP chp;
			Assert (pch > pchSave);
			GetPchpDocCpFIns (&chp, docSrc, ppb.cpArg, fFalse, wwNil);
			if (pchSave == rgch || !FNeRgch(&chp, &chpSave, cbCHP))
				{
				pchSave = pch;
				}
			else
				{
				cch = pchSave - rgch;
				Assert (cch <= cchMaxSz);
				if (FInsertRgch(docDest, cpDest, rgch, cch, &chpSave, NULL))
					{
					cpDest += cch;
					*pdcp += cch;
					}
				bltb (pchSave, rgch, pch-pchSave);
				pchSave = pch = rgch+(pch-pchSave);
				}
			chpSave = chp;
			}
		else
			{
			cch = pch - rgch;
			Assert (cch <= cchMaxSz && cch > 0);

			if (vfElNumFormat && !fDecimalIn && 
					*pdcp + cch > min(cchMaxSig, cchMaxDest))

				/* this is the El case for formatting a number with  */
				/* picture clause as used in CchPnumToPch. Here we   */
				/* express the integer in scientific notation since  */
				/* no dec pt was put in and the integer was too large*/
				/* to fit in szDest or bigger than the allowed size  */
				/* for unpacked num in UPNUM str.                    */
				/* Expnt is always + in this case.  		      */
				/* Nos larger than 10e+98 cause a syntax error- (kcm)*/

				{
				CHAR rgchT[12];
				/* cchBase + cchExpMax + 1 for leading sign */
				int cchExpMax=5;
				/* no of chars in expnt field -  E, sign, 3 digits max. */
				int cchBase=6;
				/* no of chars in base field - 5 digits and decimal pt. */
				int wExp;

				Assert(fNumeric);
				Assert(*szDestSave==' ' || *szDestSave == '-');
				Assert(cchMaxDest >= cchBase+cchExpMax+1);

				if (cch > cchFormatNumMax)
					{ /* no too large i.e. > 10e98 */
					*pdcp += cchFormatNumMax;
					return fTrue;
					}

				if (cch==1)
					{ /* ovf due to dec pt */
					int cchT = min(cchMaxSig-1, cchMaxDest-1);

					Assert(rgch[cch-1]=='.');
					bltb(&szDestSave[1], rgch, cchT);
					cch=cchT;
					*pdcp=1;
					}

				RoundRgch(rgch,cch,cchBase-1,&wExp);
				Assert(fMemory);
				szDest=szDestSave+1;
				*szDest++ = rgch[0];
				*szDest++ = '.';
				bltb(&rgch[1], szDest, cchBase-2);
				szDest += cchBase-2;

				Assert(wExp >= 0 && wExp < 1000);
				if (wExp)
					{
					*szDest++ = 'E';
					*szDest++ = '+';
					if (wExp >= 100)
						{
						*(szDest+2)=wExp%10+'0'; 
						wExp /= 10;
						*(szDest+1)=wExp%10+'0'; 
						wExp /= 10;
						*szDest=wExp%10+'0';
						szDest += 3;
						}
					else if (wExp >= 10 && wExp < 100)
						{
						*(szDest+1)=wExp%10+'0'; 
						wExp /= 10;
						*szDest=wExp%10+'0';
						szDest += 2;
						}
					else
						*szDest++ = wExp%10+'0';
					}
				*pdcp+=(szDest-szDestSave-1);
				pch=rgch;
				vfElNumFormat=fFalse;
				fDecimalIn=fTrue;
				return fTrue;
				}

			if (*pdcp + cch > cchMaxDest)
				return fTrue;

			bltb (rgch, szDest, cch);
			if (vfElNumFormat && !fDecimalIn)
				{
				Assert(fNumeric);
				if (rgch[cch-1]=='.')
					fDecimalIn=fTrue;
				}
			szDest += cch;
			*pdcp += cch;
			pch = rgch;
			}
		}  /* for (;;) */
}





/* F  P O S  I N  N U M */
/*  If pos is a required digit of *pupnum, return fTrue. In any case
	return in *pch the character that goes in position pos.
*/

/* %%Function:FPosInNum %%Owner:peterj */
FPosInNum (pos, pupnum, pch)
int pos;
struct UPNUM *pupnum;
CHAR *pch;

{
	int	ich;

	if (pos > max (pupnum->wExp, 0) || pos <= min (pupnum->wExp -
			pupnum->cchSig, 0))
		{
		*pch = '0';
		return fFalse;
		}

	ich = pupnum->wExp - pos;
	if (ich < 0 || ich >= pupnum->cchSig)
		{
		*pch = '0';
		}
	else
		{
		*pch = pupnum->rgch [ich];
		}
	return fTrue;
}




/* P C H  I N  P P P B */
/*  Return the pch of character ch after pch.  NULL if none. 
	Terminates scan on null terminator or ';'.
*/

/* %%Function:PchInPppb %%Owner:peterj */
CHAR *PchInPppb (ch, pch)
CHAR ch, *pch;

{
	BOOL fInSeq = fFalse;
	BOOL fInQuote = fFalse;

	pch--;
	while (*++pch)
		{
		if (*pch == chFldPicSequence)
			{
			if (ch == chFldPicSequence)
				return pch;
			else  if (fInSeq)
				fInSeq = fFalse;
			else  if (!fInQuote)
				fInSeq = fTrue;
			continue;
			}
		else  if (*pch == chFldPicQuote)
			{
			if (ch == chFldPicQuote)
				return pch;
			else  if (fInQuote)
				fInQuote = fFalse;
			else  if (!fInSeq)
				fInQuote = fTrue;
			continue;
			}

		if (fInQuote || fInSeq)
			continue;

		if (ChLower(*pch) == ch)
			return pch;

		}
	return NULL;
}




/* I N I T  P P P B */
/*  Initialize *pppb for formatting.  Sets up doc, pchNext and cDigit.  In
	the case of varient pictures chooses the picture to use.
*/

/* %%Function:InitPppb %%Owner:peterj */
InitPppb (fNumeric, wSign, rgch, rgcr, pppb)
BOOL fNumeric;
int wSign;
CHAR *rgch;
struct CR *rgcr;
struct PPB *pppb;

{
	CHAR *pchExpSep1, *pchExpSep2;
	CHAR *pch, ch;
	BOOL fInSeq = fFalse, fInQuote = fFalse, fAboveDec = fTrue;
	extern BOOL vfElNumFormat;

	pppb->rgch = rgch;
	pppb->rgcr = rgcr;

	pppb->fThouSep = fFalse;
	pppb->fImpSign = fTrue;

	if (!fNumeric)
		{
		pppb->pchNext = pppb->rgch;
		return;
		}

	if ((pchExpSep1 = PchInPppb (chFldPicExpSeparate, pppb->rgch))
			!= NULL)
		{
		pppb->fImpSign = fFalse;
		pchExpSep2 = PchInPppb (chFldPicExpSeparate, pchExpSep1+1);

		if (FApxGT(wSign))
			{
			pppb->pchNext = pppb->rgch;
			*pchExpSep1 = 0;
			}
		else  if (FApxLT(wSign))
			{
			pppb->pchNext = pchExpSep1+1;
			if (pchExpSep2 != NULL)
				*pchExpSep2 = 0;
			}
		else  /* wSign == 0 */
			
			{
			if (pchExpSep2 == NULL)
				{
				pppb->pchNext = pppb->rgch;
				*pchExpSep1 = 0;
				}
			else
				pppb->pchNext = pchExpSep2+1;
			}
		}
	else
		pppb->pchNext = pppb->rgch;

	pppb->pos = 0;
	pppb->posBlw = 0;
	pch = pppb->pchNext-1;

	while ((ch = *++pch) != '\0')
		{
		if (ch == chFldPicSequence)
			{
			if (fInSeq)
				fInSeq = fFalse;
			else  if (!fInQuote)
				fInSeq = fTrue;
			continue;
			}
		else  if (ch == chFldPicQuote)
			{
			if (fInQuote)
				fInQuote = fFalse;
			else  if (!fInSeq)
				fInQuote = fTrue;
			continue;
			}

		if (fInQuote || fInSeq)
			continue;

		if (ch == chFldPicReqDigit || ch == chFldPicOptDigit ||
				ch == chFldPicTruncDigit)
			{
			if (fAboveDec)
				{
				pppb->pos++;
				}
			else
				{
				pppb->posBlw++;
				}
			}

		if (ch == (vfElNumFormat ? '.' : vitr.chDecimal))
			{
			fAboveDec = fFalse;
			}

		if (ch == (vfElNumFormat ? ',' : vitr.chThousand))
			pppb->fThouSep = fTrue;

		if (ch == chFldPicNegSign || ch == chFldPicSign)
			pppb->fImpSign = fFalse;
		}
	return;
}



/* F P T  N E X T  P P P B */
/*  Return the next token from pppb.
*/

/* %%Function:FptNextPppb %%Owner:peterj */
FptNextPppb (pppb, fNumeric, fMemory)
struct PPB *pppb;
BOOL fNumeric;
BOOL fMemory;

{
	CHAR ch = *pppb->pchNext++;
	CHAR chLow = ChLower (ch);
	int fpt, cLast;
	extern BOOL vfElNumFormat;

	if (!fMemory)
		pppb->cpArg = CpFromIchPcr (pppb->pchNext-pppb->rgch-1, 
				pppb->rgcr, ccrMaxPic);

	switch (chLow)
		{
	case chFldPicExpSeparate:
		if (!fNumeric)
			goto LDefault;
		/* FALL THROUGH */
	case '\0':
		return fptEof;

	case chFldPicReqDigit:
		fpt = fptReqDigit;
		goto LDigit;

	case chFldPicTruncDigit:
		fpt = fptTruncDigit;
		goto LDigit;

	case chFldPicOptDigit:
		fpt = fptOptDigit;
LDigit:
		if (!fNumeric)
			goto LDefault;
		pppb->wArg = pppb->pos--;
		return fpt;

	case chFldPicNegSign:
		if (!fNumeric)
			goto LDefault;
		return fptNegSign;

	case chFldPicSign:
		if (!fNumeric)
			goto LDefault;
		return fptReqSign;

	case chFldPicDay:
		fpt = fptDay1;
		cLast = 3;
		goto LCountOption;

	case chFldPicYear:
			{
			int c = 0;
			if (fNumeric)
				goto LDefault;
			while (c < 3 && *pppb->pchNext == ch)
				{
				c++;
				pppb->pchNext++;
				}
			switch (c)
				{
			case 1: 
				return fptYear1;
			case 3: 
				return fptYear2;
			default:
				pppb->wArg = istErrUnknownPicCh;
				return fptError;
				}
			}

	case chFldPicHour:
		fpt = ch==chLow ? fptHour1 : fptHour3;
		cLast = 1;
		goto LCountOption;

	case chFldPicMonth:
#ifdef MONTHMINUTESAME
		if (ch != chLow)
			{  /* month */
#endif
			fpt = fptMonth1;
			cLast = 3;
			goto LCountOption;

#ifdef MONTHMINUTESAME
			}
		/* else minute */
#else

	case chFldPicMinute:
#endif
		fpt = fptMinute1;
		cLast = 1;


LCountOption:
			{
			int c = 0;
			if (fNumeric)
				goto LDefault;
			while (c < cLast && *pppb->pchNext == ch)
				{
				c++;
				pppb->pchNext++;
				}
			return fpt + c;
			}

	case chFldPicQuote:
			{
			CHAR *pch = PchInPppb (ch, pppb->pchNext);
			if (pch == NULL)
				{
				pppb->wArg = istErrUnmatchedQuote;
				return fptError;
				}
			pppb->wArg = pch-pppb->pchNext; /* cch */
			pppb->pchArg = pppb->pchNext;
			pppb->pchNext = pch+1;
			return fptQuote;
			}

	case chFldPicSequence:
			{
			CHAR *pch = PchInPppb (ch, pppb->pchNext);
			if (pch == NULL)
				{
				pppb->wArg = istErrUnmatchedQuote;
				return fptError;
				}
			if (fMemory)
				{
				/* wArg unused */
				return fptError;
				}
			pppb->pchArg = pppb->pchNext;
			*pch = '\0';
			pppb->cpArg = CpFromIchPcr (pppb->pchNext-pppb->rgch, pppb->rgcr,
					ccrMaxPic);
			pppb->pchNext = pch+1;
			return fptSequence;
			}

	default:
			{
			CHAR *pchAMPMSep;

LDefault:
			if (fNumeric && ch == (vfElNumFormat ? '.' : vitr.chDecimal))
				return fptDecimal;

			else  if (fNumeric && ch == (vfElNumFormat ? ',' : vitr.chThousand))
				return fptThousand;

			else if /* check for AM/PM, am/pm, A/P, a/p */
			(!fNumeric && (pchAMPMSep = 
					PchInPppb (chFldPicAMPMSep, pppb->pchNext)) != NULL 
					&& (chLow == vitr.rgszMornEve[iszMorn][0]
					|| chLow == vitr.rgszMornEve[iszEve][0]))
				{
				int cch = pppb->wArg = pchAMPMSep - pppb->pchNext + 1;
				while (cch-- > 0)
					if (!*++pchAMPMSep)
						{   /* unexpected end of picture */
						pppb->wArg = istErrUnknownPicCh;
						return fptError;
						}
				pppb->pchNext = pchAMPMSep + 1;
				return chLow==ch ? fptAMPMLC : fptAMPMUC;
				}
			else 
				
				{
				/* quote anything we don't know about */
				pppb->wArg = 1; /* cch */
				pppb->pchArg = pppb->pchNext-1;
				if (*pppb->pchNext == chEop || *pppb->pchNext == chTable)
					{
					pppb->pchNext++;
					pppb->wArg++;
					}
				return fptQuote;
				}
			}
		}
}



#include "inter.c" /* contains number-text translation code (can be changed
by international). */



csconst char rgchUCRoman[] = "IVIIIXLXXXCDCCCM?MMM???";
csconst char rgchLCRoman[] = "iviiixlxxxcdcccm?mmm???";
csconst char mpdgcch[10] =
{ 
	0, 1, 2, 3, 2, 1, 2, 3, 4, 2 };


csconst char mpdgich[10] =
{ 
	0, 0, 2, 2, 0, 1, 1, 1, 1, 4 };


/* C C H  S T U F F  R O M A N */
/* int to roman numeral conversion */
/* %%Function:CchStuffRoman %%Owner:peterj */
int CchStuffRoman(ppch, n, fUC, cchMax)
char **ppch;
uns n;
BOOL fUC;
int cchMax;
{
	Assert (n >= nMinRoman && n < nMaxRoman);
	return(CchStuffRmn2(ppch, n, fUC ? rgchUCRoman : rgchLCRoman, cchMax));
}


/* C C H  S T U F F  R O M A N	2 */
/* %%Function:CchStuffRmn2 %%Owner:peterj */
int CchStuffRmn2(ppch, n, qrgchRoman, cchMax)
char **ppch;
uns n;
char far *qrgchRoman;
int cchMax;
{
	int cch, cchDone = 0;

	if (n >= 10)
		{
		cchDone = CchStuffRmn2(ppch, n / 10, qrgchRoman + 5, cchMax);
		n %= 10;
		}

	cch = mpdgcch[n];
	if (cch == 0 || cch > cchMax - cchDone)
		return cchDone;

	bltbx(&qrgchRoman[mpdgich[n]], (char far *)*ppch, cch);
	*ppch += cch;

	return cch + cchDone;
}


/* C C H  P C H  T O  P P C H */
/* %%Function:CchPchToPpch %%Owner:peterj */
int CchPchToPpch (pch, ppch)
CHAR *pch, **ppch;

{
	int cch = 0;
	while (*pch)
		{
		*(*ppch)++ = *pch++;
		cch++;
		}
	return cch;
}




/* G E T  D E F A U L T  S Z P I C  D T*/
/* Fills sz with the default picture for both date and time, separated
	by a blank.

		NOTE: szPic should be at least cchMaxPic * 2 + 1 big.
*/
/* %%Function:GetDefSzPicDT %%Owner:peterj */
GetDefSzPicDT(szPic)
CHAR	*szPic;
{
	CHAR	*pch;

	GetDefaultSzPic(szPic, fTrue);
	pch = szPic + CchSz(szPic) - 1;
	*pch++ = ' ';
	GetDefaultSzPic(pch, fFalse);
}


/* G E T  D E F A U L T  S Z P I C */
/*  Fills sz with the default picture (date if fDate, else time).  Based
	on information in win.ini.

	NOTE: sz should be at least cchMaxPic big.
*/

/* %%Function:GetDefaultSzPic %%Owner:peterj */
GetDefaultSzPic (sz, fDate)
CHAR *sz;
BOOL fDate;

{
	CHAR szPicTemp [cchMaxPic];
	CHAR *szDateFormat = SzFrameKey("DateFormat",DateFormat);
	CHAR *szTimeFormat = SzFrameKey("TimeFormat",TimeFormat);

	if (vfCustomDTPic < 0)
		/* we have not checked for a custom date/time pic yet */
		{
		GetProfileString ((LPSTR) szApp, 
				(LPSTR)szDateFormat,
				(LPSTR)szEmpty, (LPSTR)szPicTemp, cchMaxPic);
		vfCustomDTPic = szPicTemp [0] > 0;
		GetProfileString ((LPSTR) szApp, 
				(LPSTR)szTimeFormat,
				(LPSTR)szEmpty, (LPSTR)szPicTemp, cchMaxPic);
		vfCustomDTPic |= szPicTemp [0] > 0;
		}

	sz [0] = 0;

	if (vfCustomDTPic)
		{
		if (fDate)
			GetProfileString ((LPSTR) szApp, (LPSTR)szDateFormat,
					(LPSTR)szEmpty, (LPSTR)sz, cchMaxPic);
		else
			GetProfileString ((LPSTR) szApp, (LPSTR)szTimeFormat,
					(LPSTR)szEmpty, (LPSTR)sz, cchMaxPic);
		}

	if (!sz [0])
		{
		DefSzPicDTTmplt(fDate, szPicTemp, cchMaxPic);
		LocalSzPicDTTmplt (szPicTemp, sz);
		}
}


/* L O C A L I Z E  S Z P I C  D T  T M P L T*/
/* %%Function:LocalSzPicDTTmplt %%Owner:peterj */
LocalSzPicDTTmplt (pchSrc, pch)
CHAR *pchSrc, *pch;

{
	CHAR ch;
	BOOL fQuoteIt;
	BOOL fUC = fFalse;
	BOOL fFull = fFalse;

	while (ch = *pchSrc++)
		switch (ch)
			{
		default:
			*pch++ = ch;
			break;
		case chPicYearHldr:
			*pch++ = chFldPicYear;
			*pch++ = chFldPicYear;
			break;
		case chPicDayHldr:
			ch = chFldPicDay;
			goto lblFill;
		case chPicMonthHldr:
			ch = ChUpper(chFldPicMonth);
			goto lblFill;
		case chPicMinHldr:
			/* For mins we always give leading zeroes. */
			*pch++ = chFldPicMinute;
			*pch++ = chFldPicMinute;
			break;
		case chPicHourHldr:
			ch = chFldPicHour;
			goto lblFill;
		case chPic24HourHldr:
			ch = ChUpper(chFldPicHour);
lblFill:
			*pch++ = ch;
			if (vitr.fLZero)
				{
				*pch++ = ch;
				}
			break;
		case chDateSepHldr:
			ch = vitr.chDate;
			fQuoteIt = FNeedsQuote(ch, fFalse);
			if (fQuoteIt)
				{
				*pch++ = chFldPicQuote;
				}
			*pch++ = vitr.chDate;
			if (fQuoteIt)
				{
				*pch++ = chFldPicQuote;
				}
			break;
		case chTimeSepHldr:
			ch = vitr.chTime;
			fQuoteIt = FNeedsQuote(ch, fFalse);
			if (fQuoteIt)
				{
				*pch++ = chFldPicQuote;
				}
			*pch++ = vitr.chTime;
			if (fQuoteIt)
				{
				*pch++ = chFldPicQuote;
				}
			break;
		case chPicAPDUCHldr:
		case chPicAPDLCHldr:
			fFull = fTrue;
			fUC = ch == chPicAPDUCHldr;
			goto lblAMPM;
		case chPicAPSUCHldr:
		case chPicAPSLCHldr:
			fUC = ch == chPicAPSUCHldr;
			Assert(!fFull);
lblAMPM:
				/* if single holder, copy first ch of AM/PM string,
					else copy entire string.  Map according to holder. */
				{
				int cch;
				int isz;
				CHAR *pchMornEve;

				cch = fFull ? min(CchSz(vitr.rgszMornEve[iszMorn]) - 1,
					CchSz(vitr.rgszMornEve[iszEve]) - 1) : 1;
				for (isz = iszMorn; isz <= iszEve; isz++)
					{
					pchMornEve = vitr.rgszMornEve[isz];
					while (cch-- && *pchMornEve)
						{
						*pch++ = fUC ?
								ChUpper (*pchMornEve++) : *pchMornEve++;
						}
					if (isz == iszMorn)
						{
						*pch++ = chFldPicAMPMSep;
						}
					}
				}
			}

	*pch = 0;
}


csconst CHAR	rgszDefTpl[][] = {
	szTplTime12Def,
			szTplTime24Def,
			szTplMDYDef,
			szTplDMYDef,
			szTplYMDDef
};


/* %%Function:DefSzPicDTTmplt %%Owner:peterj */
DefSzPicDTTmplt(fDate, sz, cchMax)
BOOL	fDate;
CHAR	*sz;
int	cchMax;
{
	bltbx((CHAR FAR *) rgszDefTpl[fDate ? (vitr.iDate + 2) : vitr.iTime],
			(CHAR FAR *) sz, cchMax);
}


csconst CHAR rgszQuotePic[][] = {
		{ chFldPicMonth,    chFldPicDay,      chFldPicYear,
	chFldPicHour,     chFldPicMinute,   '\0' },
	
	{ chFldPicReqDigit, chFldPicOptDigit, chFldPicTruncDigit,
	chFldPicNegSign,  chFldPicSign,     chFldPicExpSeparate, '\0'}};


/* %%Function:FNeedsQuote %%Owner:peterj */
BOOL FNeedsQuote(ch, fNumeric)
CHAR ch;
BOOL fNumeric;
{
	CHAR	*pch;

	Assert(fNumeric == 0 || fNumeric == 1);
	ch = ChLower(ch);
	for (pch = &(rgszQuotePic[fNumeric][0]); *pch != '\0'; pch++)
		{
		if (*pch == ch)
			{
			return fTrue;
			}
		}
	if (ch == vitr.rgszMornEve[iszMorn][0] ||
			ch == vitr.rgszMornEve[iszEve][0])
		{
		return fTrue;
		}
	return fFalse;
}


/* L O C A L I Z E  S Z P I C  N U M  T M P L T*/
/* %%Function:LocalSzPicNumTmplt %%Owner:peterj */
LocalSzPicNumTmplt (pchSrc, pch)
CHAR *pchSrc, *pch;

{
	BOOL fQuoteIt;
	CHAR ch;
	CHAR chCur1, chCur2;
	CHAR *pchT;
	int	 cch;
	BOOL fPreCurPut, fPostCurPut;
	extern BOOL vfElNumFormat;

	while (ch = *pchSrc++)
		switch (ch)
			{
		case chPicReqDigHldr:
		case chPicOptDigHldr:
		case chPicTruncDigHldr:
		case chPicNegSignHldr:
		case chPicPosSignHldr:
		case chPicExpSepHldr:
		default:
			*pch++ = ch;
			break;
		case chPicFractDigHldr:
			for (cch = vitr.iDigits; 0 < cch; cch--)
				{
				*pch++ = chFldPicReqDigit;
				}
			break;
		case chPicThousandHldr:
			ch = (vfElNumFormat ? ',' : vitr.chThousand);
			goto lblFill;
		case chPicDecimalHldr:
			ch = (vfElNumFormat ? '.' : vitr.chDecimal);
lblFill:
			fQuoteIt = FNeedsQuote(ch, fTrue);
			if (fQuoteIt)
				{
				*pch++ = chFldPicQuote;
				}
			*pch++ = ch;
			if (fQuoteIt)
				{
				*pch++ = chFldPicQuote;
				}
			break;
		case chPicPreCurHldr:
		case chPicPostCurHldr:
			fPreCurPut = (ch == chPicPreCurHldr && !vitr.fCurPostfix);
			fPostCurPut = (ch == chPicPostCurHldr && vitr.fCurPostfix);

			if (fPreCurPut || fPostCurPut)
				{
				chCur1 = vitr.szCurrency[0];
				chCur2 = vitr.szCurrency[1];
				fQuoteIt = FNeedsQuote(chCur1, fTrue);
				if (chCur2 != '\0')
					{
					fQuoteIt = fQuoteIt &&
							FNeedsQuote(chCur2, fTrue);
					}

				if (fPostCurPut && vitr.fCurSepBlank)
					{
					*pch++ = chSpace;
					}
				if (fQuoteIt)
					{
					*pch++ = chFldPicQuote;
					}
				for (pchT = &vitr.szCurrency[0]; *pchT != '\0';
						pchT++)
					{
					*pch++ = *pchT;
					}
				if (fQuoteIt)
					{
					*pch++ = chFldPicQuote;
					}
				if (fPreCurPut && vitr.fCurSepBlank)
					{
					*pch++ = chSpace;
					}
				}
			break;
			}
	*pch = '\0';
}


/* %%Function:FPicTmpltPnumGfi %%Owner:peterj */
FPicTmpltPnumGfi(pnum, gfi, cDigBlwDec, szPicTmplt, cchMaxTmplt)
NUM	*pnum;
int	gfi;
int	cDigBlwDec;
CHAR	*szPicTmplt;
int	cchMaxTmplt;
{
	int	iDig, iDigMax;
	int	ichMax;
	CHAR    *pch;
	CHAR	ch;
	CMPNUMVAL cnv;
	BOOL	fCurrency, fNegInParen, fThousand, fDecimal, fPercent;
	struct UPNUM upnum;

#ifdef YYDEBUG
	CommSzPnum(SzShared("Float Num Passed:"), pnum);
#endif
	fCurrency = (gfi & gfiCurrency) != 0;
	if (fCurrency && cDigBlwDec < vitr.iDigits)
		{
		cDigBlwDec = vitr.iDigits;
		}
	NumToPupnum (pnum, &upnum, cDigBlwDec);
	iDigMax = max(1,
			(cchMaxTmplt < upnum.wExp) ? cchMaxTmplt : upnum.wExp);

	ichMax = iDigMax;
	/* for post and pre currency place holders */
	Assert(fCurrency == 0 || fCurrency == 1);
	ichMax += 2 * fCurrency;
	fNegInParen = (gfi & gfiNegInParen) != 0 || fCurrency;
	ichMax += !fNegInParen && (upnum.wSign < 0);
	ichMax += (fThousand = (gfi & gfiComma) != 0);
	ichMax += (fDecimal = (gfi & gfiDecimal || cDigBlwDec != 0));
	ichMax += (fPercent = (gfi & gfiPercent) != 0);
	ichMax += cDigBlwDec;
	if (fNegInParen)
		{
		ichMax += 3 /* for ; and () */ + ichMax;
		}

	pch = szPicTmplt;
	if (ichMax >= cchMaxTmplt)
		{
		*pch++ = chPicReqDigHldr;
		*pch++ = chPicDecimalHldr;
		*pch++ = chPicOptDigHldr;
		*pch   = '\0';
		return fFalse;
		}
	if (!fNegInParen && (upnum.wSign < 0))
		{
		*pch++ = chPicNegSignHldr;
		}

	if (fCurrency)
		{
		*pch++ = chPicPreCurHldr;
		}

	iDig = iDigMax - 1;
	while (iDig > 0)
		{
		if (fThousand && (iDig % 3 == 0))
			{
			*pch++ = chPicOptDigHldr;
			*pch++ = chPicThousandHldr;
			}
		else
			{
			*pch++ = chPicOptDigHldr;
			}
		iDig--;
		}
	*pch++ = chPicReqDigHldr;
	if (fDecimal)
		{
		*pch++ = chPicDecimalHldr;
		}
	ch = fCurrency ? chPicReqDigHldr : chPicTruncDigHldr;
	while (cDigBlwDec-- > 0)
		{
		*pch++ = ch;
		}
	if (fCurrency)
		{
		*pch++ = chPicPostCurHldr;
		}
	if (fPercent)
		{
		*pch++ = chPercent;
		}
	if (fNegInParen)
		{
		int	dpch;

		*pch++ = chPicExpSepHldr;
		*pch++ = '(';
		bltbyte(szPicTmplt, pch, dpch = (pch - 2) - szPicTmplt);
		pch += dpch;
		*pch++ = ')';
		}
	*pch = '\0';
	return fTrue;
}




/* %%Function:DcchSafePicTmplt %%Owner:peterj */
DcchSafePicTmplt(gfi, cDigBlwDec)
int	gfi;
int	cDigBlwDec;
{
	int	dcchCur;

	dcchCur = 2 * ((vitr.fCurSepBlank + CchSz(vitr.szCurrency) - 1) - 1);
	return ((gfi & gfiCurrency) ? dcchCur : 0);
}




csconst CHAR mpmonddom[][] =
/* 1   2   3   4   5   6   7   8   9  10  11  12 */
{{ 
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
	
	{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};


/* F  I  M O N T H  F R O M  R G C H */
/* %%Function:FIMonthFromRgch %%Owner:peterj */
BOOL FIMonthFromRgch(rgch, cch)
CHAR    *rgch;
int      cch;
{
	int      iszMonth;
	int      ich;
	CHAR    *pch;

	for (iszMonth = 0; iszMonth < iszMonthMax; iszMonth++)
		{
		if (FPrefixPchCchSzCS(rgch, cch, (CHAR FAR *) (rgszMonths[iszMonth])))
			{
			return (iszMonth + 1);
			}
		}
	return (0);
}


/* %%Function:FPrefixPchCchSzCS %%Owner:peterj */
BOOL FPrefixPchCchSzCS(rgch, cch, szCS)
CHAR     *rgch;
int       cch;
CHAR FAR *szCS;
{
	int       ich;

	for (ich = 0;
			ich < cch && szCS[ich] != '\0' && ChLower(*rgch) == szCS[ich];
			ich++, rgch++);

	return (ich == cch);
}


/* The following code is ported from Excel. */
#define sepErr -1
#define sepNil 0
#define sepSpace 1
#define sepDate 2
#define sepTime 3
#define sepIntl1St2 4
#define sepIntl1St3 5
#define sepIntl1St4 6
#define cSepMax 8

csconst CHAR rgstSepIntl[][] = {
	rgstSepIntlDef};


#define iabs(_i) (((_i) < 0) ?  -(_i) : (_i))


/* This ugly mess trys parse the date and/or time at sz into the
DTTM.  A success flag is returned.  The plSeconds parameter should be
a pointer to a LONG to recieve a serial number of seconds since Jan
1, 1900 at midnight, or NULL if that information is not required. */

/* %%Function:FDttmFromSzPl %%Owner:peterj */
BOOL FDttmFromSzPl(sz, pdttm, plSeconds)
CHAR        *sz;
struct DTTM *pdttm;
long * plSeconds;
{
	CHAR             *pch;
	CHAR             *pchFirst;
	int               mon, dom, yr;
	int               w;
	int               i, iFirst, iLast;
	int               cch;
	int               cSep=0;
	BOOL              fAMPM=FALSE;
	BOOL              fMonNum=TRUE;
	BOOL              fHaveAMPM=FALSE;
	BOOL              fHaveDate=FALSE;
	int               hr, minute, second;
	int              *pw;
	int               rgw[cSepMax];
	int               rgSep[cSepMax];
	extern struct ITR vitr;

	pchFirst = sz;
	SetBytes(rgw, 0xFF, sizeof(int) * cSepMax);
	hr = minute = -1;
	second = 0;
	mon = dom = yr = -1;
	pw = rgw;

	for (pch = pchFirst; *pch != '\0';)
		{
		if (cSep >= cSepMax)
			{
			goto GotErr;
			}

		if (FAlpha(*pch))
			{
			*pw = GetAlphaDate(&pch);

			if (*pw == 0)
				{
				goto GotErr;
				}

			if (*pw > -20)
				{
				if (!fMonNum)
					{
					goto GotErr;    /* already have one */
					}
				fMonNum=FALSE;
				}
			else
				{
				int sepM1;

				if (fHaveAMPM || cSep == 0)
					{
					goto GotErr;
					}
				sepM1 = rgSep[cSep - 1];
				if (cSep == 1 && sepM1 != sepSpace && sepM1 != sepNil)
					{
					goto GotErr;
					}
				if (cSep >= 2 && sepM1 != sepSpace &&
						!(sepM1 == sepNil && rgSep[cSep - 2] == sepTime))
					{
					goto GotErr;
					}
				fHaveAMPM = TRUE;
				rgSep[cSep - 1] = sepTime;
				}
			pw++;
			}
		else  if (FDigit(*pch))
			{
			w=0;
			for (cch = 0; FDigit(*pch); cch++, pch++)
				{
				w = w * 10 + *pch - '0';
				}
			if (cch > 4 || cch > 2 && w < 100)
				{
				goto GotErr;
				}
			*pw++ = w;
			}
		else
			{
			if (*pch != (CHAR) chSpace)
				{
				goto GotErr;
				}
			for (; *pch == (CHAR) chSpace; pch++);
			}

		if ((rgSep[cSep++] = GetNextSep(&pch)) == sepErr)
			{
			goto GotErr;
			}
		}

	for (i = 0; i < cSep && rgSep[i] != sepTime; i++);

	if (i != cSep)
		{
		if (i != 0 && rgSep[i - 1] != sepSpace)
			{
			goto GotErr;
			}

		iFirst = i;
		for (iLast = i; iLast < cSep - 1 && rgSep[iLast] == sepTime; iLast++);
		hr = rgw[i++];
		if (rgw[iLast] < 0)
			{
			/*  do AM,PM,hour stuff */
			if (rgw[iLast] != -23)
				{
				fAMPM=TRUE;
				if (hr > 12)
					{
					goto GotErr;
					}
				if (rgw[iLast] == -21)     /* -21 == PM */
					{
					if (hr < 12)
						{
						hr += 12;
						}
					}
				else  /* -22 == AM */
					
					{
					if (hr == 12)
						{
						hr = 0;
						}
					}
				}
			iLast--;
			}

		if (i <= iLast)
			{
			minute = rgw[i++];
			}

		if (i <= iLast)
			{
			second = rgw[i++];
			}

		if (i <= iLast)
			{
			goto GotErr;
			}

		if (fAMPM)
			{
			iLast++;
			}

		while (i < cSep)
			{
			if (rgSep[i++] == sepTime)
				{
				goto GotErr;
				}
			}
		if (iFirst == 0)
			{
			iFirst = iLast + 1;
			iLast  = cSep  - 1;
			}
		else
			{
			iLast  = iFirst - 1;
			iFirst = 0;
			}
		}
	else
		{
		iFirst = 0;
		iLast  = cSep - 1;
		}

	if (iFirst < iLast)
		{
		if (iLast - iFirst > 2)
			{
			goto GotErr;
			}
		if (rgSep[iLast] > sepSpace)
			{
			goto GotErr;
			}

		i = iFirst + 1;
		if (fMonNum)
			{
			if ((iLast - iFirst) == 1 && rgSep[iFirst] == sepDate)
				{
				fMonNum = FALSE;        /* force "mmm date formatting */
				switch (vitr.iDate)
					{
				case iDateMDY:
					mon = rgw[iFirst];
					if (mon < 1 || mon > 12 || rgw[iLast] == 0 ||
							rgw[iLast] > mpmonddom[FLeapYr(yr)][mon-1])
						{
						yr = rgw[iLast];
						}
					else
						{
						dom = rgw[iLast];
						}
					break;
				case iDateYMD:
					if (rgw[iFirst] > 12)
						{
						yr  = rgw[iFirst];
						mon = rgw[iLast];
						}
					else
						{
						mon = rgw[iFirst];
						dom = rgw[iLast];
						}
					break;
				case iDateDMY:
					if (rgw[iLast] > 12)
						{
						mon = rgw[iFirst];
						yr  = rgw[iLast];
						}
					else
						{
						dom = rgw[iFirst];
						mon = rgw[iLast];
						}
					break;
					}
				}
			else
				{
				if ((iLast-iFirst) < 2 || rgSep[iFirst] != sepDate ||
						rgSep[i] != sepDate)
					{
					goto GotErr;
					}

				switch (vitr.iDate)
					{
				case iDateMDY:
					mon = rgw[iFirst];
					dom = rgw[i];
					yr  = rgw[iLast];
					break;
				case iDateYMD:
					yr  = rgw[iFirst];
					mon = rgw[i];
					dom = rgw[iLast];
					break;
				case iDateDMY:
					dom = rgw[iFirst];
					mon = rgw[i];
					yr  = rgw[iLast];
					break;
					}
				}
			}
		else
			{
			if (rgSep[iFirst] <= sepDate && rgSep[i] <= sepDate)
				{
				if (rgw[iFirst]<0)
					{
					if ((iLast-iFirst) > 1)
						{
						goto GotErr;
						}
					mon = iabs(rgw[iFirst]);
					yr  = rgw[iLast];
					}
				else  if (rgw[i] < 0)
					{
					dom = rgw[iFirst];
					mon = iabs(rgw[i]);
					if (i < iLast)
						{
						yr = rgw[iLast];
						}
					}
				else
					{
					goto GotErr;
					}
				}
			else
				{
				/* try for intl1 long format */
				if ((iLast - iFirst) != 2)
					{
					goto GotErr;
					}

				if (rgSep[iFirst] != sepIntl1St2 || rgSep[i] != sepIntl1St3 ||
						rgSep[iLast]  != sepIntl1St4)
					{
					for (i = iFirst; i <= iLast; i++)
						{
						CHAR FAR *pchSep;

						pchSep = (CHAR FAR *) (&rgstSepIntl[i - iFirst][0]);
						if (rgSep[i] == sepSpace || rgSep[i] == sepNil)
							{
							for (cch = *pchSep++; cch; cch--, pchSep++)
								{
								if (*pchSep != (CHAR) chSpace)
									{
									goto GotErr;
									}
								}
							}
						}
					}

				if (vitr.iLDate == iLDateDMY)
					{
					dom = rgw[iFirst];
					mon = iabs(rgw[iFirst + 1]);
					}
				else
					{
					mon = iabs(rgw[iFirst]);
					dom = rgw[iFirst + 1];
					}
				if (dom < 0)
					{
					goto GotErr;
					}
				yr = rgw[iLast];
				}
			}
		}

	*pdttm = DttmCur();
	if (mon != -1)
		{
		if (yr >= 0 && yr <= 99)
			{
			yr += 1900;
			}
		if ((fMonNum && (yr < 0 || dom < 0)) || ( dom == -1 && yr == -1))
			{
			goto GotErr;
			}
		if (yr != -1)
			{
			pdttm->yr = yr - dyrBase;
			}
		if (yr < dttmYrFirst || yr > dttmYrLast ||
				mon == 0 || mon > 12 ||
				dom == 0 || dom > mpmonddom[FLeapYr(yr)][mon-1])
			{
			goto GotErr;
			}
		pdttm->dom = (dom == -1 ? 1 : dom);
		pdttm->mon = mon;
		}
	else  if (dom != -1 || yr != -1)
		{
		goto GotErr;
		}
	else
		{
		if (hr <= -1)
			{
			goto GotErr;
			}
		pdttm->yr  = 0;
		pdttm->mon = dttmMonFirst;
		pdttm->dom = 1;
		}

	if (hr!=-1)
		{
		/* put in appropriate setting for alpha which isn't am or pm */
		if ((minute > 59) || (hr > 23))
			{
			goto GotErr;
			}
		pdttm->mint = (minute == -1 ? 0 : minute);
		pdttm->hr   = hr;
		}
	else
		{
		pdttm->hr = pdttm->mint = 0;
		}

	/* Other than the day of the week, everything should be valid. */
	pdttm->wdy = 0;
	if (FValidDttm(*pdttm))
		{
		pdttm->wdy = DowFromDttm(*pdttm);
		fHaveDate = fTrue;
		if (plSeconds != NULL)
			{
			struct DTTM dttm;

			dttm = pdttm->yr == 0 ? DttmCur() : *pdttm;

		/* Calculate long number of seconds since Jan 1, 1900... */
			*plSeconds = second + 60L * 
					((long) pdttm->hr * 60L /* hours in min */ +
					(long) pdttm->mint + 1440L * (LDaysFrom1900Dttm(dttm)-1));
			}
		}
	else
		{
		fHaveDate = fFalse;
		}
GotErr:
	return(fHaveDate);
}


#define cchDateMax  128
#define ccrDateMax    5
/* %%Function:FDttmFromDocCp %%Owner:peterj */
BOOL FDttmFromDocCp(doc, cpFirst, cpLim, pdttm, fVanished)
int          doc;
CP           cpFirst, cpLim;
struct DTTM *pdttm;
BOOL    	 fVanished;
{
	CHAR        *pch;
	int          cchDateMac, cchToFetch;
	struct FVB   fvb;
	CHAR         sz[cchDateMax];
	struct CR    rgcr[ccrDateMax];

	if (cpFirst >= cpLim)
		{
		return (fFalse);
		}

	pch        = sz;
	cchDateMac = 0;
	if (cpLim - cpFirst > (CP) (cchDateMax - 1)) /* Save the space for the */
		{                                        /* null terminator.       */
		cpLim = cpFirst + cchDateMax - 1;
		cchToFetch = cchDateMax - 1;
		}
	else
		{
		cchToFetch = (int) (cpLim - cpFirst);
		}

	fvb.doc   = doc;
	fvb.cpLim = cpLim;
	while (cchToFetch > 0)
		{
		InitFvbBufs(&fvb, pch, cchToFetch, rgcr, ccrDateMax);
		fvb.cpFirst = cpFirst + (CP) cchDateMac;
		FetchVisibleRgch(&fvb, 
				(fVanished?fvcHidResults:fvcResults),
				fFalse, fTrue);

		cchToFetch -= fvb.cch;
		cchDateMac += fvb.cch;
		pch        += fvb.cch;
		}

	*pch = '\0';
#ifdef YXYDBG
	CommSzSz("Date String Fetched: ", sz);
#endif
	return (FDttmFromSzPl(sz, pdttm, (long *) NULL));
}


/* %%Function:GetNextSep %%Owner:peterj */
int GetNextSep(ppch)
CHAR **ppch;
{
	int   sep;
	int   i, cch;
	CHAR *pch;
	extern struct ITR vitr;


	sep = sepNil;
	if (**ppch == (CHAR) chSpace)
		{
		sep = sepSpace;
		while (**ppch == (CHAR) chSpace)
			{
			(*ppch)++;
			}
		}
	pch = *ppch;
	if (!FAlphaNum(*pch))
		{
		if (*pch == vitr.chDate)
			{
			goto GotDate;
			}
		if (*pch == vitr.chTime)
			{
			goto GotTime;
			}
		switch (*pch)
			{
		case '/':
		case '-':
GotDate:
			(*ppch)++;
			sep= sepDate;
			break;
		case ':':
GotTime:
			(*ppch)++;
			sep = sepTime;
		case '\0':
			break;
		default:
			/* assume error */
			sep = sepErr;
			for (i = 0; i < 3; i++)
				{
				cch = rgstSepIntl[i][0];
				if (cch > 0 && FPrefixPchCchSzCS(pch, cch,
						(CHAR FAR *)(&rgstSepIntl[i][1])))
					{
					sep = sepIntl1St2 + i;
					(*ppch) += cch;
					break;
					}
				}
			}
		}
	return(sep);
}


/* %%Function:GetAlphaDate %%Owner:peterj */
int GetAlphaDate(ppch)
CHAR **ppch;
{
	CHAR             *pch, *pchMin;
	int               cch, i;
	extern struct ITR vitr;

	pch    = *ppch;
	pchMin = pch;
	while (FAlpha(*pch))
		{
		pch++;
		}
	cch   = pch - pchMin;

	/* Check for months */
	if (cch >= 3)
		{
		i = FIMonthFromRgch(pchMin, cch);
		if (i != 0)
			{
			i = -i;
			goto DoRet;
			}
		}

	/* Check for am, pm */
	if (FPrefixPchCchSzCS(pchMin, cch,
			(CHAR FAR *) (vitr.rgszMornEve[iszMorn])))
		{
		i = -22;
		goto DoRet;
		}
	else  if (FPrefixPchCchSzCS(pchMin, cch,
			(CHAR FAR *) (vitr.rgszMornEve[iszEve])))
		{
		i = -21;
		goto DoRet;
		}

	i=0;
DoRet:
	if (i)
		{
		*ppch = pch;
		}
	return (i);
}



#ifdef DEBUG
/* %%Function:Calendar %%Owner:peterj */
Calendar()
{
	/* Shows a calendar for a given month taken from the
		current selection. */
	struct DTTM         dttm;
	int                 v;
	int                 dow;
	int                 cDay, cDayMax;
	CHAR               *pch;
	int                 cch;
	CHAR                sz[128];

	FetchRgch(&cch, sz, selCur.doc, selCur.cpFirst, selCur.cpLim, 127);
	sz[cch] = '\0';

	SetBytes(&dttm, 0, sizeof(struct DTTM));
	pch = sz;
	dttm.mon = VFromPpch(&pch);
	pch++;
	dttm.yr = VFromPpch(&pch) - dyrBase;
	dttm.dom = 1;

	bltbx(rgszMonths[dttm.mon - 1], (CHAR FAR *) sz, 30);
	sz[0] = ChUpper(sz[0]);
	pch = sz + CchSz(sz) - 1;
	*pch++ = ',';
	*pch++ = ' ';
	CchPutPosLong((long) (dttm.yr + dyrBase), pch);
	*(pch + 4) = '\0';
	CommSz(sz);
	CommSz(SzShared("\n\r"));

	CommSz(SzShared("Su Mo Tu We Th Fr Sa\n\r"));
	CommSz(SzShared("\n\r"));

	dow = DowFromDttm(dttm);
	cDayMax = mpmonddom[FLeapYr(dttm.yr + dyrBase)][dttm.mon - 1];
	cDay = 1;

	while (cDay <= cDayMax)
		{
		SetBytes(sz, ' ', 128);

		pch = sz + dow * 3;
		while (dow < 7 && cDay <= cDayMax)
			{
			CchPutPosLong((long) (cDay++), pch);
			pch += 3;
			dow++;
			}
		*pch++ = '\n';
		*pch++ = '\r';
		*pch   = '\0';
		dow    = 0;
		CommSz(sz);
		}
}


/* %%Function:CDays %%Owner:peterj */
CDays()
{
	struct DTTM         dttm;
	int                 v;
	long                ldays;
	CHAR               *pch;
	int                 cch;
	CHAR                sz[128];
	extern struct SEL   selCur;


	FetchRgch(&cch, sz, selCur.doc, selCur.cpFirst, selCur.cpLim, 127);
	sz[cch] = '\0';

	SetBytes(&dttm, 0, sizeof(struct DTTM));
	pch = sz;
	dttm.mon = VFromPpch(&pch);
	pch++;
	dttm.dom = VFromPpch(&pch);
	pch++;
	dttm.yr = VFromPpch(&pch) - dyrBase;
	ldays = LDaysFrom1900Dttm(dttm);
	cch = CchPutPosLong(ldays, sz);
	*(sz + cch) = '\0';

	CommSzNum(SzShared("Year:  "), dttm.yr + dyrBase);
	CommSzNum(SzShared("Month: "), dttm.mon);
	CommSzNum(SzShared("Day:   "), dttm.dom);
	CommSzSz(SzShared("The number of days since December 31, 1899: "), sz);
}


/* %%Function:CMinElapsed %%Owner:peterj */
CMinElapsed()
{
	struct DTTM         dttmFrom, dttmTo;
	int                 v;
	CHAR               *pch;
	int                 cch;
	CHAR                sz[128];
	extern struct SEL   selCur;


	FetchRgch(&cch, sz, selCur.doc, selCur.cpFirst, selCur.cpLim, 127);
	sz[cch] = '\0';

	SetBytes(&dttmFrom, 0, sizeof(struct DTTM));
	pch = sz;
	dttmFrom.mon = VFromPpch(&pch);
	pch++;
	dttmFrom.dom = VFromPpch(&pch);
	pch++;
	dttmFrom.yr = VFromPpch(&pch) - dyrBase;
	pch++;
	dttmFrom.hr = VFromPpch(&pch);
	pch++;
	dttmFrom.mint = VFromPpch(&pch);

	SetBytes(&dttmTo, 0, sizeof(struct DTTM));
	pch++;
	dttmTo.mon = VFromPpch(&pch);
	pch++;
	dttmTo.dom = VFromPpch(&pch);
	pch++;
	dttmTo.yr = VFromPpch(&pch) - dyrBase;
	pch++;
	dttmTo.hr = VFromPpch(&pch);
	pch++;
	dttmTo.mint = VFromPpch(&pch);


	pch = sz;
	cch = CchPutPosLong(
			(long) (CMinutesFromDttms(dttmFrom, dttmTo)), pch);
	*(pch + cch) = '\0';

	CommSz(   SzShared("Count of minutes from:\n\r"));
	CommSzNum(SzShared("    Year:   "), dttmFrom.yr + dyrBase);
	CommSzNum(SzShared("    Month:  "), dttmFrom.mon);
	CommSzNum(SzShared("    Day:    "), dttmFrom.dom);
	CommSzNum(SzShared("    Hour:   "), dttmFrom.hr);
	CommSzNum(SzShared("    Minute: "), dttmFrom.mint);

	CommSz(   SzShared("To:\n\r"));
	CommSzNum(SzShared("    Year:   "), dttmTo.yr + dyrBase);
	CommSzNum(SzShared("    Month:  "), dttmTo.mon);
	CommSzNum(SzShared("    Day:    "), dttmTo.dom);
	CommSzNum(SzShared("    Hour:   "), dttmTo.hr);
	CommSzNum(SzShared("    Minute: "), dttmTo.mint);

	CommSzSz( SzShared("is:         "), sz);
}


/* %%Function:ParseDate %%Owner:peterj */
ParseDate()
{
	struct DTTM         dttm;
	extern struct SEL   selCur;

	if (FDttmFromDocCp(selCur.doc, selCur.cpFirst, selCur.cpLim, &dttm,
			fFalse))
		{
		CommSzNum(SzShared("Year:     "), dttm.yr + dyrBase);
		CommSzNum(SzShared("Month:    "), dttm.mon);
		CommSzNum(SzShared("Day:      "), dttm.dom);
		CommSzNum(SzShared("D. of W.: "), dttm.wdy);
		CommSzNum(SzShared("Hour:     "), dttm.hr);
		CommSzNum(SzShared("Minute:   "), dttm.mint);
		}
	else
		{
		CommSz(SzShared("Invalid Date.\n\r"));
		}
}


/* %%Function:CchPutPosLong %%Owner:peterj */
int CchPutPosLong(l, rgch)
long            l;
CHAR           *rgch;
{
	int             cch;

	if (l < 10L)
		{
		*rgch = (CHAR) (l) + '0';
		return (1);
		}
	else
		{
		cch = CchPutPosLong(l / 10, rgch);
		*(rgch + cch) = (l % 10) + '0';
		return (1 + cch);
		}
}


/* %%Function:VFromPpch %%Owner:peterj */
int VFromPpch(ppch)
CHAR **ppch;
{
	int    v;

	for (v = 0; FDigit(**ppch); (*ppch)++)
		{
		v = (10 * v) + (**ppch - '0');
		}
	return (v);
}


#endif /* YXYDBG */

/* A generalized function to accept an array of characters which represent
	integers. Max size of array is cchMax. Rounding off is done upto cchLimit
	characters. pwExp contains the value of the expnt if not NULL. (kcm)
*/

/* %%Function:RoundRgch %%Owner:peterj */
RoundRgch(rgch, cchMax, cchLimit, pwExp)
CHAR rgch[];
int cchMax, cchLimit, *pwExp;
{
	int ich;
	BOOL fInc=fFalse;

	for (ich=cchMax-1; ich >= 0; --ich)
		{
		Assert(FDigit(rgch[ich]));

		if (ich <= cchLimit-1 && !fInc) break;
		if (fInc)
			{
			if (rgch[ich]=='9')
				rgch[ich]='0';
			else  if (++rgch[ich] < '5' || ich <= cchLimit-1)
				fInc=fFalse;
			}
		else if (rgch[ich] >= '5') fInc=fTrue;
		}

	if (ich < 0 && rgch[0]=='0' && fInc)
		{
		bltb(rgch, &rgch[1], cchLimit-1);
		rgch[0]='1';
		*pwExp=cchMax;
		}
	else 
		*pwExp=cchMax-1;
}
