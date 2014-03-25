/* P R L . C */
/*  WIN #includes this file into prcsubs.c for swap tuning purposes */
#ifdef MAC
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "props.h"
#include "doc.h"
#include "ch.h"
#include "prm.h"
#include "ruler.h"
#include "pic.h"
#include "debug.h"
#endif /* MAC */

/* E X T E R N A L S */

extern struct FCB       **mpfnhfcb[];
extern struct DOD       **mpdochdod[];
extern struct CHP       vchpFetch;
extern struct CHP       vchpStc;
extern struct PAP       vpapFetch;
extern struct FRC       vfrcXScale;
extern struct FRC       vfrcYScale;
extern struct RULSS	vrulss;
extern int              vstcLast;
extern struct ESPRM     dnsprm[];
extern struct CA        caPara;

/* A P P L Y  S P R M */
/* called from ApplyPrlSgc to perform the less common sprms */
/* %%Function:ApplySprm %%Owner:davidlu */
NATIVE ApplySprm(hpprl, sprm, val, prgbProps)
char HUGE *hpprl; 
int sprm, val; 
struct CHP *prgbProps;
{
	int dxaFrom;
	struct FRC frcPrl;
	struct FRC frcResult;
	struct PAP *ppap;
	char rgchTab[sizeof(int)];
	int itbd;
	int cb;
	int valT;
	int itc;
	struct TAP *ptap;
	int itcFirst, itcLim;
	int itcFirstIns, itcMac;
	int ctc;
	int idxa, idxaMac;
	int dxa, dxaLeft, dxaCol, dxaAdjust, dxaLim, dxaGapHalfPrev;
	int ddxaGapHalf, ddxaGap, ddxaAdjust;
	int brck, brc, *rgbrc;
	int brckT;
	int ibrc;
	int fTtp, fInTable;
	int maskBitsEq;
	char *pch;
	struct TC *rgtc, *ptc;
	int *rgdxaCenter;
	struct CHP chp;
	struct PCVH pcvh;

	ppap = (struct PAP *) prgbProps;
	switch (sprm)
		{
	case sprmCDefault:
		/* turn chp toggles off. beware bit order! */
		*(int *)prgbProps &= WinMac(0xff10,0x00ff);
		prgbProps->kul = kulNone;
		prgbProps->ico = WinMac(icoAuto,icoBlack);
		break;
	case sprmCMajority:
		hpprl += 2;
		bltbx(LpFromHp(hpprl), (char far *)&chp, cbCHPBase);
/* calculate mask to identify which bits in prgbProps are equal sprm's CHP */
		maskBitsEq = (~((*(int *)prgbProps) ^ (*(int *)&chp))) &
				maskMajorityBits;
		(*(int *)prgbProps) &= ~maskBitsEq;
		*((int *)prgbProps) |=
				(*(int *)&vchpStc) & maskBitsEq;
		if (prgbProps->ftc == chp.ftc)
			prgbProps->ftc = vchpStc.ftc;
		if (prgbProps->hps == chp.hps)
			prgbProps->hps = vchpStc.hps;
		if (prgbProps->hpsPos == chp.hpsPos)
			prgbProps->hpsPos = vchpStc.hpsPos;
		if (prgbProps->kul == chp.kul)
			prgbProps->kul = vchpStc.kul;
		if (prgbProps->qpsSpace == chp.qpsSpace)
			prgbProps->qpsSpace = vchpStc.qpsSpace;
		if (prgbProps->ico == chp.ico)
			prgbProps->ico = vchpStc.ico;
		break;
	case sprmCQpsSpace:
		prgbProps->qpsSpace = val;
		break;

	case sprmPRuler:
#ifdef WIN
			{ /* recursively call ApplyPrlSgc to apply grpprl in vrulss */
			/* make local copy of ruler grpprl which, unlike Mac,
				is a heap structure. */

			CHAR grpprl [cbMaxGrpprl - cbMaxPStcPermute];

			if (vrulss.hgrpprl != hNil && vrulss.cbGrpprl > 0)
				{
				Assert(vrulss.cbGrpprl <= sizeof(grpprl));
				bltb(*vrulss.hgrpprl, &grpprl, vrulss.cbGrpprl);
				ApplyPrlSgc((char HUGE *)grpprl, vrulss.cbGrpprl, 
						prgbProps, sgcPap);
				}
			break;
			}
#else
		/* recursively call ApplyPrlSgc to apply grpprl in vrulss */
		ApplyPrlSgc((char HUGE *) vrulss.grpprl, vrulss.cbGrpprl, prgbProps, sgcPap, fFalse);
		break;
#endif
	case sprmPNest:
		bltbh(hpprl + 1, &valT, 2);
		ppap->dxaLeft = max(0, (int)(ppap->dxaLeft + valT));
		break;
	case sprmPChgTabsPapx:
		/* format:
		sprmPChgTabsPapx (char)
			cch - 2          (char)
			idxaDelMax       (char)
			rgdxaDel         (int [])
			idxaAddMax       (char)
			rgdxaAdd         (int [])
			rgtbdAdd         (char [])
		*/
		hpprl += 2;      /* Skip sprm, cch */
		DeleteTabs(ppap->rgdxaTab,
				ppap->rgtbd, NULL, &ppap->itbdMac,
				hpprl + 1, LNULL, *hpprl);
		hpprl += (*hpprl * 2) + 1;/* skip idxaMac, rgdxaDel */
		AddTabs(ppap->rgdxaTab,
				ppap->rgtbd, NULL, &ppap->itbdMac,
				hpprl + 1, hpprl + 1 + (*hpprl * 2), NULL, *hpprl);
		break;
	case sprmPChgTabs:
		/* format:
			sprmPChgTabs     (char)
			cch - 2          (char)
			idxaDelMax       (char)
			rgdxaDel         (int [])
			rgdxaClose       (int [])
			idxaAddMax       (char)
			rgdxaAdd         (int [])
			rgtbdAdd         (char [])
		*/
		hpprl += 2;      /* Skip sprm, cch */
		DeleteTabs(ppap->rgdxaTab,
				ppap->rgtbd, NULL, &ppap->itbdMac,
				hpprl + 1, hpprl + 1 + (*hpprl * 2), *hpprl);
		hpprl += (*hpprl * 4) + 1;/* skip idxaMac, rgdxaDel, rgdxaClose*/
		AddTabs(ppap->rgdxaTab,
				ppap->rgtbd, NULL, &ppap->itbdMac,
				hpprl + 1, hpprl + 1 + (*hpprl * 2), NULL, *hpprl);
		break;
	case sprmPStcPermute:
/* stc's =0 and >val are not mapped. Others are mapped into rgstc following
the istcMac byte cuurently in val. Note base 1 addressing throughout. */
		if (ppap->stc > 0
				&& ppap->stc <= val)
			{
			val = *(hpprl + 1 + ppap->stc);
			goto LMapStc;
			}
		break;
	case sprmPIncLvl:
		if (ppap->stc >= stcLevMin &&
				ppap->stc <= stcLevLast)
			{
			/* level stc codes count down, so
				subtract val from prl */
			if (val & 0200) val |= 0177400;
			val = max(min((ppap->stc - val),
					stcLevLast), stcLevMin);
			goto LMapStc;
			}
		break;
	case sprmPPc:
		pcvh.op = val;
		/* special gray value used so sprm components may be
		set independently */
		if (pcvh.pcVert != pcvhNinch)
			ppap->pcVert = pcvh.pcVert;
		if (pcvh.pcHorz != pcvhNinch)
			ppap->pcHorz = pcvh.pcHorz;
		break;
#ifdef MAC
	case sprmPicScale:
		bltbh(hpprl + 2, &vfrcXScale, sizeof(struct FRC));
		bltbh(hpprl + 6, &vfrcYScale, sizeof(struct FRC));
		break;
	case sprmPicFScale:
		((struct PIC *) prgbProps)->fScale = val;
		break;
#endif

#ifdef WIN
	case sprmPicScale:
		/* note order set up when built in pic.c */
		((struct PIC *) prgbProps)->mx = *(int *)(hpprl + 2);
		((struct PIC *) prgbProps)->my = *(int *)(hpprl + 4);
		((struct PIC *) prgbProps)->dxaCropLeft = *(int *)(hpprl + 6);
		((struct PIC *) prgbProps)->dyaCropTop = *(int *)(hpprl + 8);
		((struct PIC *) prgbProps)->dxaCropRight = *(int *)(hpprl + 10);
		((struct PIC *) prgbProps)->dyaCropBottom = *(int *)(hpprl + 12);
		break;
	case sprmPicBrcl:
		((struct PIC *) prgbProps)->brcl = val;
		break;
#endif

	case sprmTDefTable:
		bltbh(hpprl + 1, &cb, sizeof (int));
		((struct TAP *)prgbProps)->itcMac = itc = *(hpprl + 3);
		bltbh(hpprl + 4,
				&((struct TAP *)prgbProps)->rgdxaCenter,
				(itc + 1) * sizeof(int));
		bltbh(hpprl + 4 + (sizeof(int) * (itc + 1)),
				&((struct TAP *)prgbProps)->rgtc,
				cb - ((itc + 2) *sizeof(int)));
		break;
	case sprmTDxaGapHalf:
		ptap = (struct TAP *)prgbProps;
		dxaGapHalfPrev = ptap->dxaGapHalf;
		bltbh(hpprl + 1, &ptap->dxaGapHalf, sizeof (int));
		ptap->rgdxaCenter[0] += dxaGapHalfPrev - ptap->dxaGapHalf;
		break;
	case sprmTDxaLeft:
		ptap = (struct TAP *)prgbProps;
		rgdxaCenter = ptap->rgdxaCenter;
		bltbh(hpprl + 1, &dxaLeft, sizeof (int));

		ddxaAdjust = dxaLeft - (rgdxaCenter[0] + ptap->dxaGapHalf);
		idxaMac = ptap->itcMac + 1;

		for (idxa = 0; idxa < idxaMac; idxa++)
			rgdxaCenter[idxa] += ddxaAdjust;
		break;
	case sprmTInsert:
		ptap = (struct TAP *)prgbProps;
		itcFirst = val;
		ctc = *(hpprl + 2);
/* if we are trying to add columns beginning at or beyond itcMax, return */
		if ((ctc = min(ctc, itcMax - itcFirst)) <= 0)
			return;
		bltbh(hpprl + 3, &dxaCol, sizeof(int));
/* if we're adding beyond the current itcMac, we will also add new cells
	up to itcFirst. */
		itcFirstIns = min(itcMac = ptap->itcMac, itcFirst);
		rgtc = ptap->rgtc;
		rgdxaCenter = ptap->rgdxaCenter;
/* if we are inserting before itcMac, we must move up the rgdxaCenter and rgtc
	entries above itcFirstIns. */
		if (itcFirstIns < itcMac)
			{
/* move and adjust dxas in descending order to avoid overlap problems. */
			for (itc = itcMac, dxaAdjust = ctc * dxaCol;
					itc > itcFirstIns; itc--)
				rgdxaCenter[itc + ctc] = rgdxaCenter[itc] + dxaAdjust;
			bltb(&rgtc[itcFirstIns], &rgtc[itcFirstIns + ctc], cbTC * (itcMac - itcFirstIns));
			}
/* now fill in the new rgdxaCenter and rgtc entries */
		ctc += (itcFirst - itcFirstIns);
		dxa = rgdxaCenter[itcFirstIns];
		for (itc = itcFirstIns + 1, itcLim = itc + ctc; itc < itcLim;
				itc++)
			rgdxaCenter[itc] = (dxa += dxaCol);
		pch = &rgtc[itcFirstIns];	/* for moron compiler */
		SetBytes(pch, 0, cbTC * ctc);
		ptap->itcMac += ctc;
		break;
	case sprmTDelete:
	case sprmTMerge:
	case sprmTSplit:
	case sprmTDxaCol:
	case sprmTSetBrc:
		ptap = (struct TAP *)prgbProps;
		itcFirst = val;
		if (itcFirst >= (itcMac = ptap->itcMac))
			return;
		itcLim = *(hpprl + 2);
		itcLim = min(itcLim, itcMac);
		rgdxaCenter = ptap->rgdxaCenter;
		rgtc = ptap->rgtc;
		switch (sprm)
			{
		case sprmTDelete:
			if (itcLim <= itcFirst)
				break;
			dxaAdjust = rgdxaCenter[itcLim] - rgdxaCenter[itcFirst];
			for (itc = itcLim + 1, ctc = itcLim - itcFirst; itc <= itcMac; itc++)
				rgdxaCenter[itc - ctc] = rgdxaCenter[itc] - dxaAdjust;
			bltb(&rgtc[itcLim], &rgtc[itcFirst], cbTC * (ptap->itcMac + 1 - itcLim));
			ptap->itcMac -= ctc;
			break;
		case sprmTDxaCol:
			bltbh(hpprl + 3, &dxaCol, sizeof(int));
			dxaLim = rgdxaCenter[itcLim];
			for (itc = itcFirst; itc < itcLim; itc++)
				rgdxaCenter[itc + 1] = dxaCol + rgdxaCenter[itc];
			if (itcLim < itcMac)
				{
				dxaAdjust = rgdxaCenter[itcLim] - dxaLim;
				for (itc = itcLim + 1; itc <= itcMac; itc++)
					rgdxaCenter[itc] += dxaAdjust;
				}
			break;
		case sprmTMerge:
			if (itcFirst + 1 >= itcLim)
				return;
			rgtc[itcFirst].fFirstMerged = fTrue;
			for (itc = itcFirst + 1, ptc = &rgtc[itc]; itc < itcLim; itc++, ptc++)
				ptc->fMerged = fTrue;
			break;
		case sprmTSplit:
			for (itc = itcFirst, ptc = &rgtc[itc]; itc < itcLim; itc++, ptc++)
				{
				ptc->fFirstMerged = fFalse;
				ptc->fMerged = fFalse;
				}
			break;
		case sprmTSetBrc:
			brck = *(hpprl + 3);
			bltbh(hpprl + 4, &brc, sizeof(int));
			for (itc = itcFirst,ptc = &rgtc[itc]; itc < itcLim; itc++, ptc++)
				{
				rgbrc = ptc->rgbrc;
				for (ibrc = 0, brckT = 1; ibrc < 4; ibrc++, brckT <<= 1)
					{
					if ((brck & brckT) == 0)
						continue;
					rgbrc[ibrc] = brc;
					}
				}
			break;
			}
		break;
	case sprmPStc:
LMapStc:
		ppap->stc = val;
		fTtp = ppap->fTtp;
		fInTable = ppap->fInTable;
		MapStc(*mpdochdod[caPara.doc], val, &vchpStc, ppap);
/* blow MapStc cache. */
		vstcLast = stcNil;
		ppap->fTtp = fTtp;
		ppap->fInTable = fInTable;
		}
}


#ifdef MAC
/* %%Function:MultFrc %%Owner:NOTUSED */
MultFrc(pfrcEarlier, pfrcLater, pfrcResult)
struct FRC *pfrcEarlier;
struct FRC *pfrcLater;
struct FRC *pfrcResult;
{
/* returns a new fraction which is the product of 2 fractions; assumes that
	all components are > 0; reduces the result using the greatest common
	divisor and ensures that both numerator and denominator of the result
	will fit in a 16-bit word */

	int fXfactCancel = (pfrcEarlier->numer == pfrcLater->denom);
	long numer, denom;
	long lw1, lw2, remain;

	Assert(pfrcEarlier->numer > 0);
	Assert(pfrcEarlier->denom > 0);
	Assert(pfrcLater->numer > 0);
	Assert(pfrcLater->denom > 0);

	lw1 = numer = (long) (fXfactCancel ? 1 : pfrcEarlier->numer) * pfrcLater->numer;
	lw2 = denom = (long) pfrcEarlier->denom * (fXfactCancel ? 1 : pfrcLater->denom);
	/* GCD */
	for (;;)
		{
		if ((remain = lw1 % lw2) < 2)
			break;
		lw1 = lw2;
		lw2 = remain;
		}
	if (remain == 0)
		{
		/* not relatively prime */
		numer /= lw2;
		denom /= lw2;
		}
	/* force into int */
	while (numer > 0x7fff || denom > 0x7fff)
		{
		numer >>= 1;
		denom >>= 1;
		Assert(numer > 0);
		Assert(denom > 0);
		}
	pfrcResult->numer = numer;
	pfrcResult->denom = denom;
}


#endif



/* D E L E T E   T A B S

Inputs:
		rgdxaOld        Existing tab stops
		rgtbdOld        Existing tab descriptors (may be NULL)
		rgdxaCloseOld   Existing deletion range (may be NULL)
		pidxaOldMac     Max of above two arrays
	hprgdxaDel      Tab stops to delete
	hprgdxaCloseDel For each stop, how close is close (may be NULL)
		idxaDelMax      Number of tabs to delete

Deletes tab stops from rgdxaOld, rgtbdOld.  Updates *pidxaOldMac.
*/

/* %%Function:DeleteTabs %%Owner:davidlu */
DeleteTabs(rgdxaOld, rgtbdOld, rgdxaCloseOld, pidxaOldMac, hprgdxaDel,
hprgdxaCloseDel, idxaDelMax)
int *rgdxaOld;
char *rgtbdOld;
int *rgdxaCloseOld;
int *pidxaOldMac;
int HUGE *hprgdxaDel;
int HUGE *hprgdxaCloseDel;
int idxaDelMax;
{
	int *pdxaOldMac = rgdxaOld + *pidxaOldMac;
	int HUGE *hpdxaDelMac = hprgdxaDel + idxaDelMax;



	Assert ( (rgdxaCloseOld == NULL && rgtbdOld != NULL) ||
			(rgdxaCloseOld != NULL && rgtbdOld == NULL) );
	Assert ( rgdxaCloseOld == NULL || hprgdxaCloseDel == (int HUGE *)NULL);

	while (rgdxaOld < pdxaOldMac && hprgdxaDel < hpdxaDelMac)
		{
		int dxaDel, dxaClose;

		bltbh(hprgdxaDel, &dxaDel, 2);

		dxaClose = 0;

		if (rgdxaCloseOld != NULL)
			bltb (rgdxaCloseOld, &dxaClose, 2);
		else  if (hprgdxaCloseDel != (int HUGE *)NULL)
			bltbh (hprgdxaCloseDel, &dxaClose, 2);

		if (dxaClose < dxaCloseMin)
			dxaClose = dxaCloseMin;

		if ((*rgdxaOld < dxaDel - dxaClose) ||
				(rgdxaCloseOld != NULL && dxaClose > dxaCloseMin) )
			/* don't delete because 1) not "near" dxaDel or 2) this tab
				is a "delete-over-range" tab and thus cannot be deleted by
				the addition of one tab */
			{
			rgdxaOld++;
			if (rgtbdOld != NULL)
				rgtbdOld++;
			else  if (rgdxaCloseOld != NULL)
				rgdxaCloseOld++;
			continue;
			}

		if (*rgdxaOld <= dxaDel + dxaClose)
			{  /* dxaOld not after dxaDel, delete it */
			--*pidxaOldMac;
			if (--pdxaOldMac > rgdxaOld)
				{
				int c = pdxaOldMac - rgdxaOld;
				blt(rgdxaOld + 1, rgdxaOld, c);
				if (rgtbdOld != NULL)
					bltb(rgtbdOld + 1, rgtbdOld, c);
				else  if (rgdxaCloseOld != NULL)
					blt (rgdxaCloseOld + 1, rgdxaCloseOld, c);
				}
			continue;
			}

		hprgdxaDel++;
		if (hprgdxaCloseDel != (int HUGE *)NULL)
			hprgdxaCloseDel++;
		}
}




/* A D D   T A B S

Inputs:
		rgdxaOld        Existing tab stops
		rgtbdOld        Existing tab descriptors (may be NULL)
		rgdxaCloseOld   Existing deletion range (may be NULL)
		pidxaOldMac     Max of above two arrays
	hprgdxaAdd      Tab stops to add
	hprgtbdAdd      Tab descriptors to add (may be LNULL)
		rgdxaCloseAdd   deletion ranges to add (may be NULL)
		idxaAddMax      Number of tabs to add

Adds tab stops to rgdxaOld, rgtbdOld.  Updates *pidxaOldMac.
*/

/* %%Function:AddTabs %%Owner:davidlu */
AddTabs(rgdxaOld, rgtbdOld, rgdxaCloseOld, pidxaOldMac, hprgdxaAdd,
hprgtbdAdd, rgdxaCloseAdd, idxaAddMax)
int *rgdxaOld;
char *rgtbdOld;
int *rgdxaCloseOld;
int *pidxaOldMac;
int HUGE *hprgdxaAdd;
char HUGE *hprgtbdAdd;
int *rgdxaCloseAdd;
int idxaAddMax;
{
	int *pdxaOldMac = rgdxaOld + *pidxaOldMac;
	int HUGE *hpdxaAddMac = hprgdxaAdd + idxaAddMax;

	Assert ( ( rgtbdOld == NULL && hprgtbdAdd == NULL && rgdxaCloseOld != NULL &&
			rgdxaCloseAdd != NULL )  ||  ( rgtbdOld != NULL && hprgtbdAdd != NULL
			&& rgdxaCloseOld == NULL && rgdxaCloseAdd == NULL ) );


	while (hprgdxaAdd < hpdxaAddMac)
		{
		int dxaAdd, dxaOld;
		int dxaCloseOld, dxaCloseAdd;

		if (rgdxaOld == pdxaOldMac)
			{ /* Out of old tabs */
			int c = hpdxaAddMac - hprgdxaAdd;
			if (*pidxaOldMac + c <= itbdMax)
				{
				bltbh(hprgdxaAdd, rgdxaOld, c*2);
				if (hprgtbdAdd != NULL)
					bltbh(hprgtbdAdd, rgtbdOld, c);
				else  if (rgdxaCloseOld != NULL)
					bltb (rgdxaCloseAdd, rgdxaCloseOld, c*2);
				*pidxaOldMac += c;
				}
			break;
			}

		bltbh(hprgdxaAdd, &dxaAdd, 2);
		bltb(rgdxaOld, &dxaOld, 2);

		if (rgdxaCloseOld != NULL)
			{
			/* the tab lists are both deletion lists, we merge adjacent
				deletes */
			bltb (rgdxaCloseOld, &dxaCloseOld, 2);
			bltb (rgdxaCloseAdd, &dxaCloseAdd, 2);
			if (dxaCloseOld < dxaCloseMin)
				dxaCloseOld = dxaCloseMin;
			if (dxaCloseAdd < dxaCloseMin)
				dxaCloseAdd = dxaCloseMin;
			}
		else
			{
			/* the tab lists are existing tabs or an add list, we never
				merge but we replace one with another of they are "close" */
			dxaCloseAdd = dxaCloseMin;
			dxaCloseOld = 0;
			}

		if (dxaOld + dxaCloseOld >= dxaAdd - dxaCloseAdd)
			{ /* Old tab not before added tab */

			if (dxaOld - dxaCloseOld > dxaAdd + dxaCloseAdd)
				{ /* Old tab after added tab */
				if (*pidxaOldMac < itbdMax)
					{ /* enough room to move existing tabs to make room */
					int c = pdxaOldMac - rgdxaOld;
					++*pidxaOldMac;
					++pdxaOldMac;
					bltb(rgdxaOld, rgdxaOld + 1, c*2);
					if (rgtbdOld != NULL)
						bltb(rgtbdOld, rgtbdOld + 1, c);
					else  if (rgdxaCloseOld != NULL)
						bltb(rgdxaCloseOld, rgdxaCloseOld + 1, c*2);
					} /* Otherwise smash existing tab */
				}

			else  if (rgdxaCloseOld != NULL)
				{  /* merge "tabs" (deletions) */
				int dxaMin = max (min (dxaAdd - dxaCloseAdd,
						dxaOld - dxaCloseOld), 0);
				int dxaMax = max (dxaAdd + dxaCloseAdd, dxaOld + dxaCloseOld);

				dxaAdd = (int) (((long)dxaMax + (long)dxaMin)/2);
				dxaCloseAdd = dxaAdd - dxaMin;
				}

			/* else new tab goes on top of existing */
			bltb(&dxaAdd, rgdxaOld, 2);
			hprgdxaAdd++;
			if (hprgtbdAdd != NULL)
				*rgtbdOld = *hprgtbdAdd++;
			else  if (rgdxaCloseOld != NULL)
				{
				bltb(&dxaCloseAdd, rgdxaCloseOld, 2);
				rgdxaCloseAdd++;
				}
			}

		rgdxaOld++;
		if (rgtbdOld != NULL)
			rgtbdOld++;
		else  if (rgdxaCloseOld != NULL)
			rgdxaCloseOld++;
		}
}


/* F  C L O S E  X A */

/* %%Function:FCloseXa %%Owner:davidlu */
FCloseXa(xa1, xa2)
unsigned xa1, xa2;
{
	int dxa;
	if ((dxa = xa1 - xa2) < 0) dxa = - dxa;
	return (dxa < dxaCloseMin);
}



/* C C H  P R L */
/* returns length of a PRL that are of variable or large size.  */
/* %%Function:CchPrl %%Owner:davidlu */
CchPrl(pprl)
char *pprl;
{
	int cch;
	int sprm;

	if ((cch = (dnsprm[sprm = *pprl].cch)) == 0)
		{
		if (sprm == sprmTDefTable)
			bltb(pprl + 1, &cch, sizeof(int));
		else
			{
			cch = *(pprl + 1);
			if (cch == 255 && sprm == sprmPChgTabs)
				{
				char *pprlT;
				cch = (*(pprlT = pprl + 2) * 4) + 1;
				cch += (*(pprlT + cch) * 3) + 1;
				}
			}
		cch += 2;
		}
	return(cch);
}



#ifdef DEBUGORNOTWIN
/* V A L  F R O M  P R O P  S P R M 
	returns value of property addressed by prgbProp and sprm as an int */

#ifdef MAC
/* %%Function:ValFromPropSprm %%Owner:NOTUSED */
NATIVE int ValFromPropSprm(prgbProps, sprm) /* WINIGNORE - MAC only */
#else /* !MAC */
/* %%Function:C_ValFromPropSprm %%Owner:davidlu */
HANDNATIVE int C_ValFromPropSprm(prgbProps, sprm)
#endif /* !MAC */
char *prgbProps;
int sprm;
{
	int val = -1;
	struct ESPRM esprm;
	struct PCVH pcvh;

	esprm = dnsprm[sprm];
	switch (esprm.spra)
		{
	case spraWord:
		/* sprm has a word parameter that is stored at b */
		bltb(prgbProps + esprm.b, &val, 2);
		break;
	case spraByte:
		/* sprm has a byte parameter that is stored at b */
		val = *(prgbProps + esprm.b);
		break;
	case spraBit:
		/* sprm has byte parameter that is stored at bit b in word 0 */
		/* WARNING: shift in following versions is machine-dependent */
		Mac(val = (*(int *)prgbProps & (1<<(15- esprm.b))) ? 1 : 0);
		Win(val = (*(int *)prgbProps & (1<<esprm.b)) ? 1 : 0);
		break;
	case spraCFtc:
		val = ((struct CHP *)prgbProps)->ftc;
		break;
	case spraCKul:
		val = ((struct CHP *)prgbProps)->kul;
		break;
	case spraCPlain:
		/* should have done a CachePara prior to this */
		val = !FNeRgw(&vchpStc, prgbProps, cwCHP);
		break;
	case spraCIco:
		val = ((struct CHP *)prgbProps)->ico;
		break;
	default:
		switch (sprm)
			{
		case sprmCQpsSpace:
			val = ((struct CHP *)prgbProps)->qpsSpace;
			break;
		case sprmPStc:
			val = ((struct PAP *)prgbProps)->stc;
			break;
		case sprmTDxaGapHalf:
			val = ((struct TAP *)prgbProps)->dxaGapHalf;
			break;
		case sprmPPc:
			pcvh.op = 0;
			pcvh.pcVert = ((struct PAP *)prgbProps)->pcVert;
			pcvh.pcHorz = ((struct PAP *)prgbProps)->pcHorz;
			val = pcvh.op;
			break;
		case sprmCDefault:
			val = (*(int *)prgbProps & WinMac(0xff10,0x00ff)) == 0 &&
					((struct CHP *)prgbProps)->kul == kulNone &&
					((struct CHP *)prgbProps)->ico == WinMac(icoAuto,icoBlack);
			break;
			}
		break;
		}
	return val;
}


#endif /* DEBUGORNOTWIN */
