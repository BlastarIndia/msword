/* INDEX2.C - Ported from MacWord 9/16/87 - rosiep */

/*  This contains code for the data structure maintanance  */


/*  This code uses four data types:

			IDB: index data block:  the information stored for each
									index entry
			FPGN: formatted page number: a pgn and an nfc, one word total
			PNB: page number block: an array of page numbers and an
									offset to another pnb
			EXT: explicit reference

	The data structure is a hash-table which is then turned into
	a heap-tree.  The nodes are inserted in the hash table with
	chains in sorted order.  Then a heap-tree is formed (out of the
	top-level nodes of the chains coming off the hash table) with its
	head at the beginning of the hash table (see more descriptive
	comment above ConstructHeap).  The index is built up in alphabetic
	order in a temporary document by removing the smallest node in
	the heap tree one at a time and keeping the tree balanced (this
	is taken care of by HpidbFirstFromHeap and FilterDownHeap).
*/


#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "doc.h"
#include "props.h"
#include "ch.h"
#include "disp.h"
#include "file.h"
#include "sel.h"
#include "prm.h"
#include "format.h"
#include "field.h"
#include "index.h"
#include "inter.h"
#include "debug.h"
#include "error.h"





#ifdef PROTOTYPE
#include "index2.cpt"
#endif /* PROTOTYPE */

/*  E x t e r n a l s  */
extern struct SEL       selCur;
extern struct SEP       vsepFetch;
extern struct IIB       *piib;  /* Index Info Block */
extern struct MERR		vmerr;

#ifdef DEBUG
extern  int     vcEntries;  /*  Number of entries found  */
int		vcComps;    /*	Number of calls to WCompRgchIndex()  */
#endif /* DEBUG */


HPIDB HpidbScanChain();
HPIDB HpidbNew();
HPPNB HppnbNew();
HP HpAllocIndex();

#ifdef DEBUG
extern struct DBS vdbs;
#endif /* DEBUG */


/* G L O B A L S */

/* compiler bug forces us to declare w/ double indirection */
HPIDB           **prghpIndex;

SB              sbCurIndex = sbNil;
int             isbMacIndex = 0;
IB              ibMacIndex;
IB              ibMaxIndex;




/* F  I N D E X  I N I T */
/* Allocate and initialize hash table for index entry heap tree
	Return fFalse if non enough memory for hash table
*/

/* %%Function:FIndexInit %%Owner:rosiep */
BOOL FIndexInit()
{
	HP *phpidb;
	int ihte;

	/* Yes, this really is a prghpIndex, NOT an hrghpIndex */
	if ((prghpIndex = HAllocateCw(ihteBuckets * cwHP)) == hNil)
		{
		ErrorNoMemory(eidNoMemIndex);
		return fFalse;
		}

#ifdef INEFFICIENT
	phpidb = &(*prghpIndex)[0];

	for (ihte = 0; ihte < ihteBuckets; ++ihte, ++phpidb)
		*phpidb = hpNil;
#else

	Assert(hpNil == 0L);
	SetWords(&(*prghpIndex)[0], 0, ihteBuckets * 2);

#endif /* INEFFICIENT */


	Debug(vdbs.fCkHeap ? CkHeap() : 0);
	Debug(vcEntries = vcComps = 0);

	return fTrue;
}





/* F  P R O C E S S  I N D E X  E N T R Y */
/* Process XE (Index Entry) field.  Parse the field and deal with its
	argument and switches.  Parse the actual text of the index entry and
	build up an array of the strings for each level.  Add them to the
	data structure.
*/

/* %%Function:FProcessIndexEntry %%Owner:rosiep */
BOOL FProcessIndexEntry(pffb, doc, pgn, nfc)
struct FFB *pffb;
int doc;
uns pgn, nfc;
{
	CHAR stEntry[cchMaxEntry+1];
	CHAR stRange[cchBkmkMax+1];
	CHAR *pch, *pchLim, ch;
	CHAR *rgpch[cLevels];
	int rgcch[cLevels], iLevel, cch;
	int grpf = 0, wSequence;
	CP rgcp[cLevels], cp, cpDummy, dcp;
	HPIDB hpidb;
	BOOL fRange = fFalse, fFirstPgn;
	CP cpFirst = cpNil, cpLim = cpNil;
	struct FFB ffbT;
	struct EXT ext;

	/* skip the keyword */
	SkipArgument(pffb);

	InitFvbBufs(&pffb->fvb, stEntry+1, cchMaxEntry, NULL, 0);

	ffbT = *pffb;
	if (!FFetchArgText(pffb, fTrue /* fTruncate */))
		{
		/* null index entry - continue processing other entries */
		return fTrue;
		}

	/* make an st */
	stEntry[0] = pffb->cch;

	/* back up and get the extents of that last arg */
	*pffb = ffbT;
	if (!FFetchArgExtents (pffb, &cp, &cpDummy, fFalse))
		Assert (fFalse);  /* if above fetch worked, this one should */


	/* break entry text into levels, getting rid of leading whitespace */

	pch = &stEntry[1];
	pchLim = pch + stEntry[0];

	CchSkipSpace(&pch, pchLim - pch);

	/* check if this entry is out of range for partial index */
	if (piib->fPartial)
		{
		ch = ChLower(*pch);
		if (ch < piib->chMinPartial || ch > piib->chMaxPartial)
			return fTrue;   /* continue processing remaining entries */
		}

	rgcp[0] = cp;
	rgpch[0] = pch;
	rgcch[0] = 0;

	SetWords(rgcch + 1, cchNil, cLevels - 1);
	/* fill with cpNil; rather ugly (but fast) way of doing it */
	SetWords(rgcp + 1, -1, (cLevels - 1) * sizeof(CP) / sizeof(int));
	Assert(rgcp[1] == cpNil);


	for (iLevel = 0; pch < pchLim; cp++)
		{
		ch = *pch++;

		/* replace dangerous characters with spaces */
		if (ch == chReturn || ch == chEop || ch == chTable)
			ch = *(pch-1) = chSpace;

		if (ch == chFieldEscape && pch < pchLim && *pch == chColon)
			{
			cp++;
			bltb(pch, pch - 1, pchLim - pch);
			pchLim--;
			rgcch[iLevel]++;
			}
		else  if (ch == chColon)
			{
			cp += (cch = CchSkipSpace(&pch, pchLim - pch));
			if (iLevel >= cLevels - 1)
				rgcch[iLevel] += cch + 1;
			else
				{
				rgpch[++iLevel] = pch;
				rgcch[iLevel] = 0;
				rgcp[iLevel] = cp;
				}
			}
		else
			rgcch[iLevel]++;
		}


	/* get rid of trailing whitespace for each level */

	for (; iLevel >= 0; iLevel--)
		{
		for (pch = rgpch[iLevel] + rgcch[iLevel] - 1;
				(*pch == chSpace || *pch == chTab) && rgcch[iLevel] > 0;
				pch--, rgcch[iLevel]--)
			;
		}


	/* process switches */

	ExhaustFieldText(pffb);
	while ((ch = ChFetchSwitch(pffb, fFalse /* fSys */)) != chNil)
		{
		switch (ch)
			{
		case chFldXeRange:
			InitFvbBufs(&pffb->fvb, stRange+1, cchBkmkMax, NULL, 0);
			if (!FFetchArgText (pffb, fTrue /* fTruncate */))
				{
				nfc = nfcError;  /* flag this for later */
				continue;
				}
			/* make an st */
			stRange[0] = pffb->cch;
			if (!FSearchBookmark (doc, stRange, &cpFirst, &cpLim, NULL,
					bmcUser))
				{
				nfc = nfcError;  /* flag this for later */
				continue;
				}
			Assert (cpFirst != cpNil);
			Assert (cpLim != cpNil);
			fRange = fTrue;
			break;

		case chFldXeText:
			grpf |= bitExplicit;
			if (piib->docIndexScratch == docNil)
				if ((piib->docIndexScratch =
						DocCreateScratch(piib->docMain)) == docNil)
					{
					return fFalse;
					}

			if ((dcp = DcpCopyArgument(pffb, piib->docIndexScratch,
					piib->cpIndexScratch)) == cp0 && vmerr.fMemFail)
				return fFalse;

			ext.fpgn.nfc = nfcExplicit;
			ext.doc = piib->docIndexScratch;
			ext.cp = piib->cpIndexScratch;
			ext.cch = (int) CpMin(dcp, (CP)cchMaxExt);
			piib->cpIndexScratch += dcp;
			break;

		case chFldXeBold:
			grpf += bitBold;
			break;

		case chFldXeItalic:
			grpf += bitItalic;
			break;
			/* default: illegal switch - ignore */
			}
		}


	/* enter entry - add it to the data structure */

	if (rgcch[0] != 0 || rgcch[1] != cchNil)
		{
		if (piib->fSequence)
			wSequence = WSequenceValue (piib->szSequence, doc, rgcp[0], fFalse);

		FreezeHp();

		hpidb = HpidbScanChain((HPIDB HUGE *)&HpidbFromIhte(IhteHashFct(rgpch[0], rgcch[0])),
				rgpch[0], rgcch[0], doc, rgcp[0]);

		iLevel = 0;
		while (iLevel < cLevels-1 && rgcch[++iLevel] != cchNil)
			{
			hpidb = HpidbScanChain(&hpidb->hpidbSub, rgpch[iLevel],
					rgcch[iLevel], doc, rgcp[iLevel]);
			if (hpidb == hpNil)
				goto LError;
			}

		MeltHp();

		if (fRange)
			{
			grpf |= bitRangeBegin;
			pgn = PgnFromDocCp (doc, cpFirst);
			fFirstPgn = fTrue;
			}

LAddPgn:
		if (pgn != pgnNil && !FAddFpgn(hpidb, pgn, nfc, grpf, &ext, wSequence))
			{
LError:
			ErrorNoMemory(eidNoMemIndex);
			return fFalse;
			}

		if (fRange && fFirstPgn)
			{
			/* turn off Begin (which we know is on) and turn on End */
			grpf = (grpf ^ bitRangeBegin) | bitRangeEnd;
			pgn = PgnFromDocCp (doc, cpLim);
			fFirstPgn = fFalse;
			goto LAddPgn;
			}
		}

	return fTrue;
}





/* I H T E  H A S H  F C T */
/* Hash Function for index hash table */
/* %%Function:IhteHashFct %%Owner:rosiep */
int IhteHashFct(pch, cch)
char    *pch;
int     cch;
{
	int ihte = 0;

	while (cch-- > 0)
		if (*pch == chNonReqHyphen || *pch == ',')
			pch++;  /* ignore */
		else
			ihte = (ihte << 1) + ChLower(*pch++);
	if (ihte < 0)
		ihte = -(ihte+1);
	return (ihte % ihteBuckets);
}


#ifdef DEBUG
/* W  C O M P  R G C H	I N D E X */
/*  Routine to compare RGCH1 and  RGCH2
	Return:
		0 if rgch1 = rgch2
		negative if rgch1 < rgch2
		positive if rgch1 > rgch2
	This routine is case-insensitive
	soft hyphens are ignored in the comparison
*/

/* %%Function:WCompRgchIndex %%Owner:rosiep */
HANDNATIVE int C_WCompRgchIndex(hpch1, cch1, hpch2, cch2)
char	HUGE *hpch1, HUGE *hpch2;
int     cch1, cch2;
{
	char	*pch;
	char	HUGE *hpchLim;
	char	sz1[cchMaxEntry+2];
	char	sz2[cchMaxEntry+2];

	Debug(++vcComps);

	Assert(cch1 < cchMaxEntry);
	for (pch = sz1, hpchLim = hpch1 + cch1;
		hpch1 < hpchLim; hpch1++)
		{
		if (*hpch1 != chNonReqHyphen)
			*pch++ = *hpch1;
		}
	*pch = 0;

	Assert(cch2 < cchMaxEntry);
	for (pch = sz2, hpchLim = hpch2 + cch2;
		hpch2 < hpchLim; hpch2++)
		{
		if (*hpch2 != chNonReqHyphen)
			*pch++ = *hpch2;
		}
	*pch = 0;

	return (WCompSzSrt(sz1, sz2, fFalse /* not case sensitive */));
}
#endif /* DEBUG */



/* H P I D B  S C A N  C H A I N */
/* Scan down a chain of entries for a specified entry
	If the entry doesn't exist, create it.
	Entries are inserted in sorted (alphabetic) order.
*/

/* %%Function:HpidbScanChain %%Owner:rosiep */
HPIDB HpidbScanChain(hphpidb, pch, cch, doc, cp)
HPIDB HUGE *hphpidb;
char *pch;
int  cch, doc;
CP   cp;
{
	int   wT = -1;
	HPIDB hpidb;

	hpidb = (hphpidb == hpNil ? hpNil : *hphpidb);

	while (hpidb != hpNil && (wT = WCompRgchIndex(hpidb->rgch,
			hpidb->cch, (char HUGE *)pch, cch)) < 0)
		{
		hphpidb = &hpidb->hpidbNext;
		hpidb = *hphpidb;
		}

	if (wT == 0)
		return hpidb;    /* found it - return it */

	/* insert it in the chain */
	return (*hphpidb = HpidbNew(pch, cch, doc, cp, hpidb));
}





/* F  A D D  F P G N */
/* Add a page number to the list for a given index entry.
	Returns fFalse if out of memory.
*/
/* %%Function:FAddFpgn %%Owner:rosiep */
int FAddFpgn(hpidb, pgn, nfc, grpf, pext, wSequence)
HPIDB hpidb;       /*  Index entry  */
uns pgn;           /*  Page number to add  */
uns nfc;           /*  Page number format  */
int grpf;
struct EXT *pext;
int wSequence;
{
	int     ifpgn;          /*  page number within block  */
	HPPNB hppnb = &hpidb->pnb;    /* pointer to page block */
	HPPNB hppnbLastNorm = hpNil;  /* last non-explicit page block */
	uns pgnT;

	if (hpidb == hpNil)
		return fFalse;

	/*  first skip full page number blocks  */
	for (; ; hppnb = hppnb->hppnbNext)
		{
		if (hppnb->rgfpgn[0].nfc != nfcExplicit)
			hppnbLastNorm = hppnb;
		if (hppnb->hppnbNext == hpNil)
			break;
		}

	if (grpf & bitExplicit)
		{
		/* for explicit references, just copy the ext to a new pnb */
		if ((hppnb->hppnbNext = HppnbNew()) == hpNil)
			return(fFalse);
		bltbh(pext, hppnb = hppnb->hppnbNext, cbEXT);
		ifpgn = 0;
		}
	else
		{
		/* for normal references, scan inside the last block for a free fpgn */
		if (hppnbLastNorm == hpNil)
			pgnT = fpgnNil;
		else
			{
			for (ifpgn = 0; ifpgn < ifpgnMax && (pgnT = hppnbLastNorm->rgfpgn[ifpgn].pgn) != fpgnNil; ifpgn++)
				if (pgn == pgnT)
					return(fTrue);  /* already there */
			}

		/* if no room in block, or last block in list is explicit, need new block */
		if (pgnT != fpgnNil || hppnb != hppnbLastNorm)
			{
			if ((hppnb->hppnbNext = HppnbNew()) == hpNil)
				return(fFalse);
			hppnb = hppnb->hppnbNext;
			ifpgn = 0;
			}

		hppnb->rgwSequence[ifpgn] = wSequence;
		hppnb->rgfpgn[ifpgn].pgn = pgn;
		hppnb->rgfpgn[ifpgn].nfc = (hpidb->fRange && (grpf & bitRangeEnd)) ? nfcRange : nfc;
		if (grpf & bitRangeEnd)
			hpidb->fRange = fFalse;
		else  if (grpf & bitRangeBegin)
			hpidb->fRange = fTrue;
		}

	if (grpf & (bitBold + bitItalic))
		{
		ifpgn <<= 1;
		if (grpf & bitBold)
			hppnb->grpf |= 1 << ifpgn;
		if (grpf & bitItalic)
			hppnb->grpf |= 1 << ifpgn + 1;
		}

	return(fTrue);
}




/* H P I D B  N E W */
/* Set up a new index data block.
	Return hpNil if can't allocate one.
*/

/* %%Function:HpidbNew %%Owner:rosiep */
HPIDB HpidbNew(pch, cch, doc, cp, hpidbNext)
char *pch;
int cch;
CP cp;
HP hpidbNext;
{
	int cbNew;
	HPIDB hpidb;

	Debug(++vcEntries);

	cbNew = cbIDB + cch;

	if ((hpidb = (HPIDB) HpAllocIndex(cbNew)) != hpNil)
		{
		/* Initialize the new data block */

		InitPnb(&hpidb->pnb);
		hpidb->hpidbSub = hpNil;
		hpidb->hpidbNext = hpidbNext;
		hpidb->doc = doc;
		hpidb->cp = cp;
		bltbh(pch, hpidb->rgch, hpidb->cch = cch);
		}

	return hpidb;
}



/* H P P N B  N E W */
/* Set up a new page number block */
/* %%Function:HppnbNew %%Owner:rosiep */
HPPNB HppnbNew()
{
	HPPNB hppnb;

	if ((hppnb = (HPPNB) HpAllocIndex(cbPNB)) != hpNil)
		InitPnb(hppnb);

	return hppnb;
}



/* I N I T  P N B */
/* Initialize a page number block */

/* W A R N I N G: nativize at own risk */
/* %%Function:InitPnb %%Owner:rosiep */
InitPnb(hppnb)
HPPNB hppnb;
{
	Assert (hppnb != hpNil);

/* W A R N I N G !! LpConvHp only gives a limited life far pointer -
	if you nativize this routine, be absolutely certain nothing will
	move as a result of bltcxNat (which is what bltcx is defined to
	be if __FNATIVE__ ; you may be better off writing a proper bltch */

	bltcx(fpgnEmpty, LpConvHp(hppnb->rgfpgn),ifpgnMax);
	bltcx(0, LpConvHp(hppnb->rgwSequence), ifpgnMax);

	hppnb->grpf = 0;
	hppnb->hppnbNext = hpNil;
}




/*   H P  A L L O C  I N D E X

Memory management for the index code is centered here.  We use the
SB manager to allocate 16K chunks (SB's), from which we then allocate
data blocks to be used for collecting index entries.  This function
allocates generic data blocks.  It is used by HpidbNew and HppnbNew
for IDB's and PNB's respectively.  Caller must cast the returned hp
to the type that they want.  We are responsible for freeing all SB's
we use after we clean up from index generation, thus we must keep
track of the ones we allocate (an SB must be freed only if a
successful call to CbAllocSb is done on it).  We link the sb's we
use together by storing an sbNext in the first word of each sb.
When we free them, we chain through this list.

Plan:
	Allocate a new data block from the current index SB.
	Allocate a new SB if not enough room in sbCurIndex.
	Return hp of new data block (hpNil if failure).

sbCurIndex   current SB which we are filling up
ibMacIndex   offset of the next available data block in this SB     
ibMaxIndex   size of the current SB (we request 16K but may get more)

*/



/* %%Function:HpAllocIndex %%Owner:rosiep */
HP HpAllocIndex(cb)
int cb;  /* size of data block requested */
{
	HP hp = hpNil;
	SB sbT;

	if (sbCurIndex != sbNil && ibMacIndex + cb < ibMaxIndex)
		{
		/* there's room in current SB */

		hp = HpOfSbIb(sbCurIndex, ibMacIndex);
		ibMacIndex += cb;
		}
	else
		{
		extern int cbMemChunk;
		/* allocate a new SB */
		sbT = SbAllocEmmCb(cbMemChunk);

		if (sbT != sbNil)
			/* we got an SB; now let's use it */
			{
			ibMaxIndex = CbSizeSb(sbT);
			Assert (ibMaxIndex >= cb);

			/* store next sb in first word of new sb for later freeing */
			*((SB HUGE *)HpOfSbIb(sbT, 0)) = sbCurIndex;
			sbCurIndex = sbT;

			hp = HpOfSbIb(sbCurIndex, sizeof(SB));
			ibMacIndex = cb + sizeof(SB);
			}
		}

LRet:

	/* avoid side effects */
	ResetSbCur();

	return hp;
}


/* F R E E  I N D E X  M E M */
/* Clean up after ourselves; free handles and sb's used by index */

/* %%Function:FreeIndexMem %%Owner:rosiep */
FreeIndexMem()
{
	SB sb;

	FreeH(prghpIndex);

	while (sbCurIndex != sbNil)
		{
		sb = sbCurIndex;
		/* get next sb to free from first word of current sb */
		sbCurIndex = *((SB HUGE *)HpOfSbIb(sbCurIndex, 0));
		FreeEmmSb(sb);
		}

	ResetSbCur();
	sbCurIndex = sbNil;
	isbMacIndex = 0;
}


#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Index2_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* Index2_Last() */
