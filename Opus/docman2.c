/* 
	Contains routines that should be shareable with MacWord.
	SzSearchSz and SzSpectoGrepRghsz are in question because they
	refer to some DOS wild card characters.
*/
#define NOGDICAPMASKS
#define NONCMESSAGES
#define NODRAWFRAME
#define NORASTEROPS	
#define NOMINMAX
#define NORECT
#define NOSCROLL
#define NOKEYSTATE
#define NOICON
#define NOPEN
#define NOREGION
#define NODRAWTEXT
#define NOMETAFILE
#define NOCLIPBOARD
#define NOSOUND
#define NOCOMM
#define NOKANJI
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "file.h"
#include "dmdefs.h"
#include "debug.h"
#include "prompt.h"
#include "message.h"
#include "ch.h"
#include "doslib.h"


#ifdef PROTOTYPE
#include "docman2.cpt"
#endif /* PROTOTYPE */

#define   chPound   '#'

extern struct SUMD      vsumd;
extern CHAR             **hszvsumdFile;
extern int              cDMFileMac;
extern int              cDMEnum;
extern int              cDMEnumMac;
extern int              cDMEnumMax;
extern int              fnDMEnum;
extern int              fnDMFcTable;
extern struct DMFLAGS   DMFlags;
extern struct DMQFLAGS  DMQFlags;
extern struct DMQD      DMQueryD;
extern CHAR             *pchTreeStart; /* so I can do leaf offsets */
extern CHAR FAR         *DMFarMem;
extern long             cbDMFarMem;
extern FC HUGE         *hprgfcDMEnum; /* the indices into the TMP file fnDMEnum */
extern SB               sbDMEnum; /* Sb of the above HUGE pointer */
extern int              fLeaf;
extern int              irgfcDMFilenameStart;
extern struct FCB       **mpfnhfcb[];

SB SbAlloc();


/* %%Function:FCkDMExpand %%Owner:chic */
FCkDMExpand()
{
	if (cDMFileMac >= cDMEnumMax)
		{ /* have to expand */
		int cEnumOld = cDMEnumMax;
		unsigned int cbNew;
		FC FAR *lpTmp;

		cDMEnumMax = max(cDMEnumMax * 3/2, 10); /* in case cDMEnumMax is 0 */
		/* try to expand to cDMEnumMax longs */
		if ((cbNew=CbReallocSb(sbDMEnum, cDMEnumMax*4, HMEM_MOVEABLE))==0)
			{
			DMFlags.fDMOutOfMemory=fTrue;
			return fFalse;
			}
		cDMEnumMax= cbNew/4; /* may have gotten some extra */
		Assert(hprgfcDMEnum != (HP)NULL);
		lpTmp= LpConvHp(hprgfcDMEnum);
		SetWords(lpTmp+cEnumOld, NULL, (cDMEnumMax - cEnumOld)*2);
		}
	return fTrue;
}





/*************************************************************************
	FHpRgFcQSort:
	This version of quicksort handles sorting an array of indices into a file.
	The sort is done using the comparison function pointed to by pfncComp, and
	at the offset into each string given by offset. This allows sorting on a field
	in the middle of the string, and for the comparisons to be string, numeric,
	or date. There is a cutoff on the "recursion" at files smaller than cQSortMin.
	At the end of QSort, we have an "almost sorted" file, and Insertion Sort is
	the fastest method of finishing up.
*************************************************************************/

#define   cQSortMin    5

/* %%Function:FHpRgFcQSort %%Owner:chic */
FHpRgFcQSort(iLeft,iRight,hprgfc,soffset,pfncComp,fn)
int iLeft,iRight;  /* range of the array to sort */
int soffset;      /* offset into each string where sorting should start */
int fn;           /* fn of file holding the strings. */
FC HUGE *hprgfc;        /* heap array of indices into fn of the strings. */
int (*pfncComp)();    /* comparison function to use when performing the sort*/
{
	int iStack = 0;
	int iPartition;
	int stack[50];
/*****
save these on the stack. They will be popped at the end so the
values will be correct for the call to Insertion Sort at the end.
*****/
	stack[iStack++]=iLeft;
	stack[iStack++]=iRight;

	do
		{
		if (iRight-iLeft > cQSortMin)
			{
			iPartition=HpRgFcPartition(iLeft,iRight,hprgfc,soffset,
					pfncComp,fn);
			if (iPartition == -1)
				return fFalse;

			if ((iPartition-iLeft)>(iRight - iPartition))
				{   /* push largest piece on stack */
				stack[iStack++]=iLeft;
				stack[iStack++]=iPartition-1;
				iLeft=iPartition+1;   /* reset for small piece */
				}
			else
				{
				stack[iStack++]=iPartition+1;
				stack[iStack++]=iRight;
				iRight=iPartition-1;
				}
			}
		else
			{   /* done with that piece. Pop the next one to do */
			iRight= stack[--iStack];
			iLeft= stack[--iStack];
			}
		} 
	while (iStack >0);

	HpRgFcISort(iLeft,iRight,hprgfc,soffset,pfncComp,fn);
	return fTrue;
}


/***************************************************************************
	HpRgFcPartition:
	This routine performs the partitioning operation for quicksort. At the end
	of partitioning, the following must hold:
	1. For some iPartition, hprgfc[iPartition] is in the correct location.
	2. For all i < iPartition, hprgfc[i] is less than hprgfc[iPartition]
	3. For all i > iPartition, hprgfc[i] is greater than hprgfc[iPartition]
	This is done in the following manner: 

	First, we do a median-of-three operation. This involves taking an element
	from the left, right and middle of the array, and sorting them. The
	middle of the three becomes our partitioning element.

	We then swap the middle element into the (n-1)th location, so it's place
	is known. At this point we know that the first element is less than
	the partition, and that the last is greater. These are therefore in
	the proper halves of the array, and can be left alone. They also 
	serve as sentinal nodes to prevent our running off the end of the array.

	Third, we do a normal partition on elements (1,n-2). This involves scanning
		from the left end of the array for elements larger than the partition, 
	and from the right for elements less than the partition. Finding the 
	first occurance of each, we know they are in the wrong halves of the 
	array, so we swap them. This continues until the scans cross. At this
	point, we know that the partition belongs in the position the "from
	the left"scan ended up in, so we swap it into there. 

	The two halves can now be recursively sorted.
***************************************************************************/
/* %%Function:HpRgFcPartition %%Owner:chic */
HpRgFcPartition(iLeft, iRight, hprgfc, soffset, pfncComp, fn)
int iLeft,iRight;
int soffset;
int fn;
int (*pfncComp)();
FC HUGE *hprgfc;
{
	int iMedian=(iLeft+iRight) >> 1;
	int irgfcInc,irgfcDec;
	CHAR *pchLeft,*pchRight,*pchMedian,*pchTmp;
	int cch;
	CHAR rgchLeft[cchMaxDMFileSt];
	CHAR rgchMedian[cchMaxDMFileSt];
	CHAR rgchRight[cchMaxDMFileSt];

	/* danger to leave it uninitialized */
	SetBytes(rgchLeft, 0, cchMaxDMFileSt);
	SetBytes(rgchMedian, 0, cchMaxDMFileSt);
	SetBytes(rgchRight, 0, cchMaxDMFileSt);

	if (CchSzFromFc(fnDMEnum,hprgfc[iLeft],rgchLeft,cchMaxDMFileSt) > 1 &&
			CchSzFromFc(fnDMEnum,hprgfc[iMedian],rgchMedian,cchMaxDMFileSt) > 1 &&
			CchSzFromFc(fnDMEnum,hprgfc[iRight],rgchRight,cchMaxDMFileSt) > 1)
		{ /* test > 1 because cch returnd includes null terminator */
		pchLeft=rgchLeft+soffset;
		pchMedian=rgchMedian+soffset;
		pchRight=rgchRight+soffset;
		}
	else
		{ /* some problem reading the info, don't proceed */
		goto LRetFail;
		}


/* first, Sort the left, right and median pieces into the correct order */
	if ((*pfncComp)(pchLeft,pchMedian) > 0)
		{   /* swap them */
		HpFcSwap(iLeft,iMedian,hprgfc);
		pchTmp=pchMedian;
		pchMedian=pchLeft;
		pchLeft=pchTmp;
		}
	if ((*pfncComp)(pchLeft,pchRight) > 0)
		{   /* swap them */
		HpFcSwap(iLeft,iRight,hprgfc);
		pchTmp=pchRight;
		pchRight=pchLeft;
		pchLeft=pchTmp;
		}
	if ((*pfncComp)(pchMedian,pchRight) > 0)
		{   /* swap them */
		HpFcSwap(iMedian,iRight,hprgfc);
		pchTmp=pchRight;
		pchRight=pchMedian;
		pchMedian=pchTmp;
		}
/* Now swap the middle one into iRight-1. It will be our partition element */
	HpFcSwap(iMedian,iRight-1,hprgfc);
	/******
	We are ready to do the normal QSort partitioning on the subarray 
	[iLeft+1,iRight-2];
	******/
	irgfcInc=iLeft;
	irgfcDec=iRight-1;
	/******
	We will keep fetching into rgchLeft as our temp buffer. Note that
	pchLeft will always point to the correct location in the array, 
	so it can be left alone, even though the array under it is being
	changed each time.
	******/

	cch = CchSzFromFc(fnDMEnum,hprgfc[iMedian],rgchMedian,cchMaxDMFileSt);
	Assert(cch > 1);

	pchMedian=rgchMedian+soffset;
	pchLeft=rgchLeft+soffset;
	for (;;)
		{
		Assert(irgfcInc >= 0 && irgfcInc+1 < cDMFileMac);
		if ((cch = CchSzFromFc(fnDMEnum,hprgfc[++irgfcInc],rgchLeft,cchMaxDMFileSt)) > 1)
			{
			while ((*pfncComp)(pchLeft,pchMedian) < 0 && cch > 1)
				{
				Assert(irgfcInc >= 0 && irgfcInc+1 < cDMFileMac);
				cch = CchSzFromFc(fnDMEnum,hprgfc[++irgfcInc],rgchLeft,
						cchMaxDMFileSt);
				}
			if (cch < 1)
				goto LRetFail;
			}
		else
			goto LRetFail;

		Assert(irgfcDec > 0 && irgfcDec < cDMFileMac);
		if ((cch = CchSzFromFc(fnDMEnum,hprgfc[--irgfcDec],rgchLeft,cchMaxDMFileSt)) > 1)
			{
			while ((*pfncComp)(pchLeft, pchMedian) > 0 && cch > 1)
				{
				Assert(irgfcDec > 0 && irgfcDec < cDMFileMac);
				cch = CchSzFromFc(fnDMEnum,hprgfc[--irgfcDec],rgchLeft,
						cchMaxDMFileSt);
				}
			if (cch < 1)
				goto LRetFail;
			}
		else
			goto LRetFail;

		if (irgfcInc < irgfcDec)
			HpFcSwap(irgfcInc,irgfcDec,hprgfc);
		else
			break;
		}
	/******
	Finally, swap the partitioning element into it's final location 
	******/
	HpFcSwap(iRight-1,irgfcInc,hprgfc);
	return(irgfcInc);

LRetFail:
	return -1;
}


/*************************************************************************
	HpFcSwap: 
	This function just takes care of exchanging two indices in the heap array.
*************************************************************************/

/* %%Function:HpFcSwap %%Owner:chic */
HpFcSwap(iLeft,iRight,hprgfc)
int iLeft,iRight;
FC HUGE *hprgfc;
{
	FC fcTmp;
	fcTmp=hprgfc[iRight];
	hprgfc[iRight]=hprgfc[iLeft];
	hprgfc[iLeft]=fcTmp;
}




/***************************************************************************
	HpRgHszISort:
	This function performs an insertion sort on the range of indices given.
	The basic algorithm is to start with the lowest index, which is always 
	sorted with respect to itself. We then add one element at a time, always
	keeping the list sorted. This later sorting is done by saving the value
	of the new element, shifting all the elements greater than it up by one, 
	and then inserting it in its proper location.
***************************************************************************/

/* %%Function:HpRgFcISort %%Owner:chic */
HpRgFcISort(iLeft,iRight,hprgfc,soffset,pfncComp,fn)
int iLeft,iRight,soffset,fn;
FC HUGE *hprgfc;
int    (*pfncComp)();
{
	int iTmp,iShift;
	FC fcTmp;
	int cch;
	CHAR *pchTmp,*pchShift;
	CHAR rgchTmp[cchMaxDMFileSt];
	CHAR rgchShift[cchMaxDMFileSt];

	pchTmp = rgchTmp+soffset;
	pchShift = rgchShift+soffset;

	for (iTmp=iLeft+1;iTmp<=iRight;iTmp++)
		{
		fcTmp= hprgfc[iTmp];
		cch = CchSzFromFc(fnDMEnum,fcTmp,rgchTmp,cchMaxDMFileSt);
		Assert(cch > 1);
		iShift=iTmp;
		cch = CchSzFromFc(fnDMEnum,hprgfc[iShift-1],rgchShift,
				cchMaxDMFileSt);
		Assert(cch > 1);
		while ((*pfncComp)(pchShift, pchTmp) > 0)
			{
			hprgfc[iShift]=hprgfc[iShift-1];
			iShift--;
			if (iShift> iLeft)
				{
				cch = CchSzFromFc(fnDMEnum,hprgfc[iShift-1],rgchShift,
						cchMaxDMFileSt);
				Assert(cch > 1);
				}
			else
				break;
			}
		hprgfc[iShift]=fcTmp;
		}
}


/***************************************************************************
	CchSzFromFc:
	It goes into the given file at the given fc, pulls
	the count of chars first, and then gets that many. It returns the number
	of chars it has put into rgch, which is the length of the string +1 for
	the zero byte.
***************************************************************************/
/* %%Function:CchSzFromFc %%Owner:chic */
CchSzFromFc(fn,fc,rgch,cchMax)
int fn,cchMax;
CHAR *rgch;
FC fc;
{
	int cch = CchStFromFc(fn, fc, rgch, cchMax);

	Assert(cch == rgch[0]+1);
	StToSzInPlace(rgch);
	return(cch);
}


/***************************************************************************
	CchStFromFc:
	It goes into the given file at the given fc, pulls
	the count of chars first, and then gets that many. It returns the number
	of chars it has put into rgch, which is the length of the string +1 for
	the count byte.
***************************************************************************/
/* %%Function:CchStFromFc %%Owner:chic */
CchStFromFc(fn,fc,rgch,cchMax)
int fn,cchMax;
CHAR *rgch;
FC fc;
{
	int cch;

	if (fc > PfcbFn(fn)->cbMac)
		{
#ifdef DEBUG
		CommSzLong(SzShared("Offending fc is "), fc);
		CommSzLong(SzShared("cbMac is "), PfcbFn(fn)->cbMac);
#endif /* DEBUG */
		Assert(fFalse);
		return 0;
		}

	SetFnPos(fn,fc);
	ReadRgchFromFn(fn,rgch,1);
	ReadRgchFromFn(fn,rgch+1,cch=min(rgch[0],cchMax-1));
	if (rgch[0] != cch)
		rgch[0] = cch; /* in case the read is actually limited by cchMax */
	return(cch+1);
}


csconst CHAR rgstSort[][] =
{
	StKey("Title",SortTitle),
			StKey("Author",SortAuthor),
			StKey("Last Saved by",SortSaveBy),
			StKey("Creation Date",SortCreateDate),
			StKey("Last Saved Date",SortSaveDate),
			StKey("Size",SortSize)
};


/************************************************************************
	StSortFromDMFc:
	This function gets the sort field out of the enumeration string at
	location fc in fnDMEnum, puts it in StSortField
*************************************************************************/

/* %%Function:StSortFromDMFc %%Owner:chic */
StSortFromDMFc(fc, stSortField, cchMax)
FC fc;
CHAR *stSortField;
int cchMax;
{
	int cchSort;
	CHAR *pchSortField, *pchTemp;
	CHAR rgchTemp[cchMaxDMFileSt];
	CHAR rgchSort[14];
	int cchRemain;


	switch (DMFlags.dms)
		{
		int ist;
	case dmsName:
		ist = 0;
		goto LBlt1;
	case dmsAuthor:
		ist = 1;
		goto LBlt1;
	case dmsLastSavedBy:
		ist = 2;
		goto LBlt1;
	case dmsCreationDate:
		ist = 3;
		goto LBlt1;
	case dmsRevisionDate:
		ist = 4;
		goto LBlt1;
	case dmsSize:
		ist = 5;
LBlt1:
		CopyCsSt(rgstSort[ist], stSortField);
		pchSortField = stSortField + *stSortField + 1;
		break;
	default:
		Assert(fFalse);
		return;
		}

	*pchSortField++ = ':';
	*pchSortField++ = ' ';
	cchRemain = cchMax - (pchSortField - stSortField);
	Assert(cchRemain > 0);
	if (CchStFromFc(fnDMEnum,fc,rgchTemp,cchMaxDMFileSt) <= 1)
		{
		stSortField[0] += 2; /* account for ": " added above */
		return;
		}

	pchTemp = rgchTemp+ichDMSortMin;
	switch (DMFlags.dms)
		{
	case dmsName:
	case dmsAuthor:
	case dmsLastSavedBy:
		cchSort = min(rgchTemp[0]-ichDMSortMin+1, cchRemain);
		pchSortField = bltb(pchTemp, pchSortField, cchSort);
		break;
	case dmsCreationDate:
	case dmsRevisionDate:
		PdttmToSt(pchTemp, rgchSort, 14);
		goto LBlt2;
	case dmsSize:
		PlongToSt(pchTemp, rgchSort, 14);
LBlt2:
		cchSort = min(rgchSort[0], cchRemain);
		pchSortField = bltb(rgchSort+1, pchSortField, cchSort);
		break;
	default:
		Assert(fFalse);
		return;
		}
	stSortField[0] = pchSortField-stSortField-1;
	Assert(stSortField[0] < cchMax);
}



/***************************************************************************
	FValidAONExp:
	This routine checks to see if the boolean expression the user entered in
	one of the query fields is valid. The expressions are made up of parens
	"()" , ANDs "&", ORs "," , NOTs "~", and the character operands that are
	being searched for.  This routine is based on the fact that as we parse
	a boolean expression we are in one of 2 states, and the currently valid
	characters depend on the current state. The valid chars in each state are:

	1:  (   ~ chars
	2:  ( ) ~ chars & ,

	We begin in state 1, since you can't start a boolean expression with AND,
	OR, or ).  When we have just seen a (, we move to state 1 for the same 
	reason. Chars in either state take us to 2, since after an operand 
	everything is valid. After an operator we move to state 1, since both two 
	ops in a row (other than NOT-NOT), and an op followed by a closed paren are 
	invalid. If at any time the count of our current paren depth goes negative, 
	we fail because of inbalanced parens, and likewise if it is non-zero at 
	the end.
	*************************************************************************/

/* %%Function:FValidAONExp %%Owner:chic */
FValidAONExp(pch,cchExp,pichError)
CHAR *pch;
int cchExp;
int *pichError;
{
	int state=1;
	int cparen=0;
	int cch=0;
	int chOR=chFldSep;

	if (cchExp > cchMaxAONExp)
		{
		*pichError += cchMaxAONExp;
		return fFalse;
		}

	while (cch < cchExp)
		{
		CHAR chT= *pch++;
		switch (chT)
			{
		case '~':    /* NOT */
			if (state==2)
				state=1;
			break;
		case ',':    /* OR  */
		case ';':    /* could be OR */
			     /* Intl versions want both , and ; to work as chOR    */
			     /* FUTURE(jl) should Z version should do same? */
#ifndef INTL
			if (chT!=chOR)
				{   /* treat as a char */
				state=2;
				break;
				}
#endif /*~INTL*/
		case '&':    /* AND */
			if (state==2)
				{
				state=1;
				break;
				}
			goto InvalidExp;
		case '(':
			state=1;
			cparen++;    /* inc paren depth */
			break;
		case ')':
			if (state==1 || --cparen < 0)
				goto InvalidExp;
			break;
		case chSpace:
			break;
		default:
			state=2;  /* chars */
			break;
			}
		cch++;
		}
	if (cparen!=0)       /* too many ('s */
		{
InvalidExp:
		*pichError+= cch;
		return fFalse;
		}
	return fTrue;
}


/************************************************************************ 
HBuildAONTree:
	This routine does a recursive parse of a boolean expression, building
	an AND-OR-NOT evaluation tree as it goes. Each node of the tree can be an
	AND,OR,NOT, or LEAF node. AND and OR nodes have two children, NOT nodes
	have one child, and LEAF nodes have none. The routine recursively calls
	HBuildAONTree whenever it needs to find another subtree. When you hit a
	chOR, everything you've found so far become the left branch of OR node 
	(parens can change this), and the routine recurses to build a tree out 
	of what comes after. A chAND may cause you to return up one level if you 
	were not called from an OR node, so that successive chANDs associate 
	left to right, etc. etc. The Index field in a LEAF node holds the index
	in the array rgchLeaves which corresponds to the Boolean value for this
	leaf. 

	For Example, the expression:       A,B&C&D,E&~F&G     builds the tree:

															
								,                             
						____/ \__             Leaf A has an index of 0
						/         \            Leaf B has an index of 1
						,           &           Leaf C has an index of 2
						/ \         / \          Leaf D has an index of 3
					A   &       &   G         Leaf E has an index of 4
						/ \     / \            Leaf F has an index of 5
						&   D   E   ~           Leaf G has an index of 6
						/ \          |                        
					B   C         F                        

	A tree is built once for each non-NULL field of the Query 
	command, and is then used for testing all the files.

	NOTE:  The original calling routine should initialize iLeafCur to 0 and
		fLeaf to fFalse before each call.
***************************************************************************/

/* %%Function:HBuildAONTree %%Owner:chic */
struct AONNODE **HBuildAONTree(ppch,OpParent,hrgdchLeaves,hrgLenLeaves,pcLeaves)
CHAR **ppch;
int OpParent;
CHAR **hrgdchLeaves;
CHAR **hrgLenLeaves;
CHAR *pcLeaves;
{
	struct AONNODE **hParent = hNil;
	struct AONNODE **hLeft;
	CHAR chOR=chFldSep;
	int ich,fInQuote;

	while (**ppch!=0 && *pcLeaves < MAXNUMLEAVES)
		{
		switch (**ppch)
			{
		case chSpace:       /* Spaces */
			(*ppch)++;
			break;    /* ignore them, but they seperate leaves*/
		case chNOT:      /* NOT */
			if (fLeaf)
				{
				(*ppch)--;
				goto AssumeAND;    /* two leaves in a row */
				}
			if ((hParent= (struct AONNODE **)HAllocateCw(cwAONNODE)) == hNil)
				return hNil;
			(**hParent).operator= OPNOT;
			(*ppch)++;
			(**hParent).hLeftBranch=HBuildAONTree(ppch,OPNOT,
					hrgdchLeaves,hrgLenLeaves,pcLeaves);
			if (OpParent!=OPOR && OpParent!=OPSTART)
				return(hParent);   /* go up one level */
			break;
AssumeAND:           /* jump here if we get two leaves in a row */
		case chAND:      /* AND */
			fLeaf=fFalse;
			if (OpParent!=OPOR && OpParent!=OPSTART)
				return(hParent);  /* Go up one level */
			hLeft=hParent;
			if ((hParent= (struct AONNODE **)HAllocateCw(cwAONNODE)) == hNil)
				return hNil;
			(**hParent).operator= OPAND;
			(*ppch)++;
			(**hParent).hLeftBranch=hLeft;
			(**hParent).hRightBranch=HBuildAONTree(ppch,OPAND,
					hrgdchLeaves,hrgLenLeaves,pcLeaves);
			break;
		case chLParen:          /*  (  */
			if (fLeaf)
				{
				(*ppch)--;
				goto AssumeAND;    /* two leaves in a row */
				}
			(*ppch)++;       /* inc past the paren */
			hParent=HBuildAONTree(ppch,OPSTART,hrgdchLeaves,
					hrgLenLeaves,pcLeaves);
		/* OPSTART forces this tree to complete before returning */
			(*ppch)++;       /* inc past the right paren */
			if (OpParent!=OPOR && OpParent!=OPSTART)
				return(hParent);   /* go up one level */
			break;
		case chRParen:
			/* done with one level of parens */
			fLeaf=fTrue;  /* treat a paren exp like one leaf */
			return(hParent);
		case 0:
			return(hParent);   /* end of the exp. */
		case chComma:       /* OR */
		case chSemi:        /* depending on chFldSep */
				    /* Intl versions want both , and ; to work as chOR	  */
				    /* FUTURE(jl) should Z version should do same? */
#ifndef INTL
			if (**ppch==chOR)
#endif /*~INTL*/
				{
				fLeaf=fFalse;
				hLeft=hParent;  /* what we have so far is left branch*/
				if ((hParent= (struct AONNODE **)HAllocateCw(cwAONNODE)) == hNil)
					return hNil;
				(**hParent).operator= OPOR;
				(*ppch)++;
				(**hParent).hLeftBranch=hLeft;
				(**hParent).hRightBranch=HBuildAONTree(ppch,OPOR,
						hrgdchLeaves,hrgLenLeaves,pcLeaves);
				break;
				}
			/* otherwise fall through to default */
		default:          /* chars, this is a leaf */
			if (fLeaf)
				{
				(*ppch)--;
				goto AssumeAND;    /* two leaves in a row */
				}
			fLeaf=fTrue;      /* found a leaf */
			if ((hParent= (struct AONNODE **)HAllocateCw(cwAONNODE)) == hNil)
				return hNil;
			(**hParent).operator= OPLEAF;
			(**hParent).iLeaf= *pcLeaves;
			if (**ppch==chDQuote)
				{   /* quoted leaf */
				fInQuote=fTrue;
				(*ppch)++;
				}
			else
				fInQuote=fFalse;
			(*hrgdchLeaves)[*pcLeaves]= (*ppch)-pchTreeStart;
			ich=0;
			while (**ppch!=0)
				{       /* inc to the end of the leaf */
				if (fInQuote)
					{   /* special code for quotes */
					if (**ppch!=chDQuote)
						{   /* just inc on by */
						ich++;
						(*ppch)++;
						}
					else
						{
						if (*(++(*ppch))==chDQuote)
							{    /* double quote */
							ich+=2;
							(*ppch)++;
							}
						else
							{   /* single quote */
							goto EndOfLeaf;
							}
						}
					}
				else
					{
					switch (**ppch)
						{
					case chComma:
					case chSemi:
#ifndef INTL
						if (**ppch!=chOR)
							{
							ich++;
							(*ppch)++;
							break;
							}
#endif /*~INTL*/
					/* otherwise fall through, it's chOR */
					case chAND:
					case chNOT:
					case chRParen:
					case chLParen:
					case chSpace:
						goto EndOfLeaf;
					default:
						ich++;
						(*ppch)++;
						break;
						}
					}
				}
EndOfLeaf:
			(*hrgLenLeaves)[(*pcLeaves)++]= ich;
			if (OpParent!=OPOR && OpParent!=OPSTART)
				return(hParent);
			break;
			}           /* end of the switch */
		}               /* end of the while */
	/* we only hit here at the end of the expression */
	return(hParent);
}


/**********************************************************************
	FreeAONTree:
	This routine recursively frees an AND-OR-NOT tree.
	Each level does a switch on the operator field of 
	the current node. If the node is a leaf node the routine frees the node 
	and returns, otherwise it recurses on the children and frees them before
	freeing the parent and returning.
**********************************************************************/

/* %%Function:FreeAONTree %%Owner:chic */
FreeAONTree(hAONRoot)
struct AONNODE **hAONRoot;
{

	switch ((**hAONRoot).operator)
		{
	case OPLEAF:    /* end of the line */
		FreeH(hAONRoot);
		return;
	case OPNOT:
		FreeAONTree((**hAONRoot).hLeftBranch);
		FreeH(hAONRoot);
		return;
	case OPOR:
	case OPAND:
		FreeAONTree((**hAONRoot).hLeftBranch);
		FreeAONTree((**hAONRoot).hRightBranch);
		FreeH(hAONRoot);
		return;
		}
}


/*********************************************************************
	SzSearchSz:
		This function takes two string args. This first is a pattern to match 
	and the second is the string to search. Two wildcards are implemented. '?'
	matches any single character, but there MUST be a character there, and '*'
	matches 0 to n characters, up to the end of the string. To search for the
	characters ? and *, they must be escaped with ^ as in ^? and ^*. ^^ is
	also used to search for the ^ character. 
		The basic algorithm is to scan the string for the first character in the 
	pattern, and then try to match the rest. When a mismatch occurs, it backs up 
	to the first occurance of the first character in the pattern AFTER the one we
	are currently using.  If we haven't found another occurance we continue from 
	where we are. The ? wildcard is implemented by letting it match any 
	character, and the * wildcard is implemented by a recursive call with 
	what's left of the pattern after the *, and what's left of the search string.
	********************************************************************/

/* %%Function:SzSearchSz %%Owner:chic */
SzSearchSz(pchPattern,pchString)
CHAR *pchPattern, *pchString;
{
	CHAR *pchSkip=NULL;
	/* keeps track of the string position to skip to on failure */
	CHAR *pchPatTmp;  /* tmp pointer to step through the search pattern */
	CHAR chFirst;       /* the first non-wild char of the search pattern */

	QszLower(pchPattern);
/* pchString is the original string, do not alter it by QszLower */

	if (*pchPattern == chDOSWildAll)
		pchPattern++;       /* starting a pattern with * is stupid */
	pchPatTmp=pchPattern;        /* remember where the pattern starts */

	while (*pchString!=0)
		{       /* start out by finding the first char of the pattern */
		switch (*pchPattern)
			{
		case chDQuote:       /* string starts with a quote */
		/********************* 
		note that first and last quotes have been stripped, so the only 
		quotes that should be left at this point are double quotes 
		in the middle of the string. This should therefore be a 
		double quote.
		**********************/
			if (*(++pchPattern)==0)
				return(fFalse);
			/* else we have something after the quote */
			/* if it's not a quote, too bad! */
			goto GotNormalChar;
		case chDOSWildSingle:       /* pattern starts with ? wildcard */
			if (*(++pchPattern) == 0)
				return fTrue;    /* only a ? in the pattern */
			if (*(++pchString)==0)
				return fFalse;   /* last char but more pattern */
			break;        /* go back and get a real character */
		case chCaret:       /* pattern starts with ^ escape */
			/* if pattern is a single ^ return fFalse */
			if (*(++pchPattern)==0)
				return(fFalse);
			/* else escape next char and drop through */
		default:  /* we have a normal character to match */
GotNormalChar:
			while (*pchString != 0)
				{
				chFirst = ChLower(*pchString);
				if (chFirst != *pchPattern)
					pchString++;
				else
					break;
				}
			if (*pchString != 0)  /* out of chars */
				goto GotFirstChar;
			break;
			}
		}
	/* if we reach here we must be out of chars */
	return fFalse;
	/* we now have the first character matched */
GotFirstChar:
	chFirst = ChLower(*pchString);  /* save it for later reference */

	/* inc past this character in the pattern and the string */

	if (*(++pchPattern)==0)
		return fTrue;    /* end of pattern, we're done */
	if (*(++pchString)==0)
		{       /* end of string */
		if (*pchPattern!=chDOSWildAll)
			return fFalse;
		if (*(pchPattern+1)!=0)
			return fFalse;
		return fTrue;    /* pattern ends in a '*' , which is valid */
		}

	/* now try and match the rest of the pattern */
	while (fTrue)
		{
		switch (*pchPattern)
			{
			CHAR chT1,chT2;
		case chDQuote:
		/********************* 
		note that first and last quotes have been stripped, so the only 
		quotes that should be left at this point are double quotes 
		in the middle of the string. This should therefore be a 
		double quote.
		**********************/
			if (*(++pchPattern) == 0)
				return(fFalse);  /* string ended on a " */
			/* else we have something after the quote */
			/* if it's not a quote, too bad! */
			goto GotNormalChar2;
		case chDOSWildSingle:   /* this is an automatic match */
			if (ChLower(*pchString) == chFirst && pchSkip == NULL)
				pchSkip=pchString; /* save this spot */
			break;

		case chDOSWildAll:   /* this means recurse */
			if (*(++pchPattern) == 0)
				return fTrue; /* pattern ends in '*' */
			return(SzSearchSz(pchPattern,pchString));

		case chCaret:   /* escape the next char */
			if (*(++pchPattern)==0)
				return fFalse;   /* can't end in  ^ */
			/* else drop into default for next char */
		default:  /* match a normal char */
GotNormalChar2:
			if ((chT1=ChLower(*pchString))==chFirst && pchSkip==NULL)
				pchSkip=pchString; /* save this spot */
			if (chT1 != (chT2=*pchPattern))
				{     /* failure, back up and go on */
				pchPattern=pchPatTmp; /* restart */
				if ((chT2 == chDOSWildSingle) ||
						(chT2 == chCaret))
					pchPattern++;
			/* since we would already have inc'd past it */
				if (pchSkip!=NULL)
					{
					pchString=pchSkip;
					pchSkip=NULL;
			/* since we've already found it */
					goto GotFirstChar;
					}
			/* else stay where we are in the string */
				else
					goto GotNormalChar;
				}
			else
				break;    /* we matched*/
			}   /* end of switch */
		/* go on to next char */
		if (*(++pchPattern)==0)
			return fTrue;    /* end of pattern */
		if (*(++pchString)==0)
			{       /* out of string */
			if (*pchPattern!=chDOSWildAll)
				return fFalse;
			if (*(pchPattern+1)!=0)
				return fFalse;
			return fTrue;    /* pattern ends in a '*' */
			}
		}           /* end of while(fTrue) */
}


/**********************************************************************
	RgLeafSearchSz:
	This routine takes six args.  The first is a pointer to a
	string that contains a Query Specifier. The second is a count of 
	the number of substrings strings in the Query Specifier that we need to 
	search for. The third is an array of indices that specify the offset of
	the start of each substring. The Fourth is an array that holds the length
	of each substring.  The fifth is the string to be searched. The sixth 
	is an array of char in which to put the results of the searches. 
	The routine copies each substring out of the heap (SzSearchSz may move 
	the heap), and calls SzSearchSz to check for a match.
**********************************************************************/

/* %%Function:RgLeafSearchSz %%Owner:chic */
RgLeafSearchSz(szSpec,cLeaves,hrgdchLeaves,hrgLenLeaves,pchString,
hrgchResult)
CHAR *szSpec;
CHAR cLeaves;
CHAR **hrgdchLeaves;
CHAR **hrgLenLeaves;
CHAR *pchString;
CHAR **hrgchResult;
{
	CHAR rgchLeaf[MAXLEAFLENGTH],*pch;
	int i,len;


	for (i=0;i<cLeaves;i++)
		{
		if (pchString==0)
			{   /* Null string, ABORT! */
			(*hrgchResult)[i]=fFalse;
			continue;
			}
		pch= szSpec+(*hrgdchLeaves)[i];  /* find the start of the leaf */
		len= (*hrgLenLeaves)[i];        /* and its length */
		bltb(pch,rgchLeaf,len);        /* blt it off the heap */
		rgchLeaf[len]=0;           /* terminate it */
		(*hrgchResult)[i]=SzSearchSz(rgchLeaf,pchString);   /* search */
		}
}


/**********************************************************************
	EvalAONTree:
	This routine recursively evaluates an AND-OR-NOT tree and returns the
	fTrue/fFalse result. Each level does a switch on the operator field of 
	the current node. If the node is a leaf node the routine returns the 
	corresponding value from rgchLeafValues, otherwise it recurses on the 
	children and combines the return values as needed.
**********************************************************************/

/* %%Function:EvalAONTree %%Owner:chic */
EvalAONTree(hAONRoot,hrgchLeafValues)
struct AONNODE **hAONRoot;
CHAR **hrgchLeafValues;
{

	if (hAONRoot == hNil)
		return fFalse;

	switch ((**hAONRoot).operator)
		{
	case OPLEAF:    /* end of the line */
		return((*hrgchLeafValues)[(**hAONRoot).iLeaf]);
	case OPNOT:
		return(!EvalAONTree((**hAONRoot).hLeftBranch,
				hrgchLeafValues));
	case OPOR:
		return(EvalAONTree((**hAONRoot).hLeftBranch,hrgchLeafValues)
				|| EvalAONTree((**hAONRoot).hRightBranch,hrgchLeafValues));
	case OPAND:
		return(EvalAONTree((**hAONRoot).hLeftBranch,hrgchLeafValues)
				&& EvalAONTree((**hAONRoot).hRightBranch,hrgchLeafValues));
	default:
		Assert(fFalse);
		return fFalse;
		}
}


/************************************************************************
DMQueryTestSz:
	This routine takes the filename it is passed, and sees if it 
	passes the restrictions of the parameters currently in DMQueryD.
	It calls SummaryFromSz to read the info into the given buffer, and then
	does the leaf searches and AON evaluations for each non null test.
	It also does the date testing on either the summary info date, or if
	that is not available, on the DOS date.
************************************************************************/
/* %%Function:DMQueryTestSz %%Owner:chic */
DMQueryTestSz(szFile)
CHAR *szFile;	/* ANSI version of filename */
{
	int fn;
	struct DMQSD *pdmqsd;
	struct DMQSD *pdmqsdLim;
	int grpf;
	CHAR   ***phsz;
	CHAR   **hrgchResult=(CHAR **)HAllocateCw(MAXNUMLEAVES/2);
	/* to hold results of leaf search */

	if (hrgchResult == hNil)
		return fFalse;

	if (!FVSumdFromSz(szFile))
		{
		Assert(!DMFlags.fTmpFile); /* tmp file should not be on the list in the first place */
		goto retFalse;     /* no info or bad file */
		}

	/* now check each field vs. Query parameters */

	/* Loop through Title, Subject, Author, Keyword, Saved by */
	FreezeHp();
	pdmqsdLim = &DMQueryD.dmqsdText;
	Assert(pdmqsdLim > &DMQueryD.dmqsdTitle);
	for (grpf = DMQFlags.grpf, pdmqsd = &DMQueryD.dmqsdTitle, phsz = &vsumd.hszTitle;
			pdmqsd < pdmqsdLim && grpf > 0; grpf >>= 1, pdmqsd++, phsz++)
		{
		if (grpf & 1)
			{ /* we have a test to do */
#ifdef DEBUG
			if (pdmqsd == &DMQueryD.dmqsdTitle)
				Assert(DMQFlags.fTitleTest);
			else  if (pdmqsd == &DMQueryD.dmqsdSubject)
				Assert(DMQFlags.fSubjectTest);
			else  if (pdmqsd == &DMQueryD.dmqsdAuthor)
				Assert(DMQFlags.fAuthorTest);
			else  if (pdmqsd == &DMQueryD.dmqsdKeyword)
				Assert(DMQFlags.fKeywordTest);
			else  if (pdmqsd == &DMQueryD.dmqsdSavedBy)
				Assert(DMQFlags.fSavedByTest);
			else
				Assert(fFalse);
			AssertH(*phsz);
#endif
			if (!FDoDMQSDTest(pdmqsd, **phsz, hrgchResult))
				{
				MeltHp();
				goto retFalse;
				}
			}
		}
	MeltHp();

	if (grpf == 0) /* nothing else to test */
		goto retTrue;

	if (DMQFlags.fCFromDateTest || DMQFlags.fCToDateTest)
		{   /* we have a Creation Date test to do */
		if (!EvalDateSpec(vsumd.dttmCreation,&DMQueryD.CDateInfo))
			goto retFalse;
		}
	if (DMQFlags.fSFromDateTest || DMQFlags.fSToDateTest)
		{   /* we have a Saved Date test to do */
		if (!EvalDateSpec(vsumd.dttmRevision,&DMQueryD.SDateInfo))
			goto retFalse;
		}
	if (DMQFlags.fTextTest)
		{   /* we have an Text Ret test to do */
		int result, i, osfn;
		struct DMQSD *pDMQSD;
		int cLeaves;
		int **hrgpchLeaves;
		int fCloseFn;
		FC cfc;

		Assert(DMFarMem != NULL);
		Assert(cbDMFarMem > 0);
		Assert(osfnNil == -1);

	/* open the file */
		SzToStInPlace( szFile );
		fCloseFn = ((fn = FnFromOnlineSt(szFile)) == fnNil);
		StToSzInPlace(szFile);
		if (fn != fnNil)
			{
			if ((osfn = OsfnValidAndSeek(fn,0L)) == osfnNil)
				goto retFalse;
			cfc = PfcbFn(fn)->cbMac;
			}
		else
			{	/****
		we need to open the file, and close it later.
			Don't go through FnOpenSt, since it does lots
			and lots of work we don't want to do
			****/
			struct OFS ofs;
			if ((osfn = OpenFile( (LPSTR) szFile, (LPOFSTRUCT) &ofs, OF_READ )) == osfnNil)
				goto retFalse;
			cfc = DwSeekDw(osfn, 0L, SF_END);
			}

		pDMQSD= &DMQueryD.dmqsdText;
		cLeaves= pDMQSD->cLeaves;
		if ((hrgpchLeaves = HAllocateCw(cLeaves)) == hNil)
			goto retFalse;

		if (cLeaves > MAXNUMLEAVES && !FChngSizeHCb(hrgchResult, cLeaves, fFalse))
			{
			FreeH(hrgpchLeaves);
			goto retFalse;
			}

		FreezeHp();
		bltbcx(0, *hrgchResult, cLeaves);
		for (i=0;i< pDMQSD->cLeaves;i++)
			(*hrgpchLeaves)[i]= *((*(DMQueryD.hrghszTLeaves))[i]);
		/* call the DSR grep code to do the search */

		result= wordgrep(DMFarMem, (int)cbDMFarMem, *hrgpchLeaves, cLeaves, *hrgchResult,
				osfn, !DMQueryD.fCase, fc0, cfc);
		MeltHp();
		if (fCloseFn)
			FCloseDoshnd(osfn);
		FreeH(hrgpchLeaves);
	/* now evaluate the tree */
		if (!EvalAONTree(pDMQSD->hTree, hrgchResult))
			{
			goto retFalse;
			}
		}
	/* otherwise it passed */
retTrue:
	FreeH(hrgchResult);
	return(fTrue);

retFalse:
	FreeH(hrgchResult);
	return(fFalse);
}


/***********************************************************************
	FDoDMQSDTest:
	Given a pointer to a DMQSD, a string to test, and an array to put the
	result, this function searches the string for all the leaves specified
	in the DMQSD, evaluates the tree according to the AON expression, and
	returns whether the file passes.
***********************************************************************/
/* %%Function:FDoDMQSDTest %%Owner:chic */
FDoDMQSDTest(pDMQSD,pch,hrgchResult)
struct DMQSD *pDMQSD;
CHAR *pch,**hrgchResult;
{
	CHAR rgchBuffer[cchMaxAONExp];

/* **(pDMQSD->hstSpec) is verified to be less than 80 by FValidAONExp 
before we will accept the string. */
	StToSz(*(pDMQSD->hstSpec),rgchBuffer);
	RgLeafSearchSz(rgchBuffer,pDMQSD->cLeaves,
			pDMQSD->hrgdchLeaves,pDMQSD->hrgLenLeaves,pch,hrgchResult);
	/* now evaluate the tree */
	return(EvalAONTree(pDMQSD->hTree, hrgchResult));
}


/************************************************************************
	EvalDateSpec:
	This function tests a date against a Query Date Specifier.
	It checks first to see if the date is valid. If it is the routine
	then checks to see if it passes the first test, and then the second,
	if there is one.
************************************************************************/
/* %%Function:EvalDateSpec %%Owner:chic */
EvalDateSpec(dttmFile,pDATEINFO)
struct DATEINFO *pDATEINFO;
struct DTTM dttmFile;
{
	if (pDATEINFO->dttmFrom.lDttm)
		{  /* we have a FROM test to do */
		if (CompareDTTMDate(pDATEINFO->dttmFrom,dttmFile) > 0)
			return(fFalse);
		}
	if (pDATEINFO->dttmTo.lDttm)
		{  /* we have a TO test to do */
		if (CompareDTTMDate(pDATEINFO->dttmTo,dttmFile) < 0)
			return(fFalse);
		}
	return fTrue;
}


/**********************************************************************
	CchDMSI:
	This function copies the appropriate parts into pch, depending on 
	the Sortby mode currently selected.  
	It assumes that vsumd contains the summary information for the desired
	file.
	Always fake a sz and cch includes the null terminator.
********************************************************************/
/* %%Function:CchDMSI %%Owner:chic */
CchDMSI(pchDest)
CHAR *pchDest;
{
	CHAR *pch;
	int  cchSz, cch;

	Assert(hszvsumdFile);   /* if its 0 we have no summary info to use */
	Assert(sizeof(long) == sizeof (struct DTTM));

/* copy Sort by field to the display string */
	switch (DMFlags.dms)
		{
	case dmsName:
		/* blt the title for the filename case */
		pch = *(vsumd.hszTitle);
		goto LBltOver;
	case dmsAuthor:
		pch = *(vsumd.hszAuthor);
		goto LBltOver;
	case dmsLastSavedBy:
		pch = *(vsumd.hszRevisor);
LBltOver:
		cch = min((cchSz = CchSz(pch)), cchMaxSIDisp);
		bltb(pch, pchDest, cch);
		if (cchSz > cch) /* we didn't blt the null terminator */
			pchDest[cch-1] = 0;
		break;

	case dmsSize:
		*(long *)pchDest = vsumd.cchDoc;
		goto LGetCch;
	case dmsRevisionDate:
		*(struct DTTM *)pchDest = vsumd.dttmRevision;
		goto LGetCch;
	case dmsCreationDate:
		*(struct DTTM *)pchDest = vsumd.dttmCreation;
LGetCch:
		cch = sizeof(long);
		pchDest[cch++] = 0;    /* fake an sz */
		break;
		}
	return(cch);
}


/******************************************************************************
	DMSort:
	This routine takes care of sorting the list of files in Document
	management. If the sort field is the filename it just does a simple QSort
	on the strings produced by CchDMEnum. If the sort field is something else,
	however, things are a bit more complicated, as some of the fields may be
	blank. In this case the routine starts by doing a QSort on the second half
	of the display strings. We get two groups of files from this. 

	1. Those with no summary info at all (\0 at ichDMSortMin)
	2. Those with summary info which includes the sort field.

	This routine does a binary search to find the division between the 
	groups, and then reorders them into:

	1. Those with sort fields.
	2. Those without sort fields or no summary info.

	The Second group is then sorted by filename, since no other ordering has 
	been done. The start in the array of group 2 is stored in 
	irgfcDMFilenameStart so Update can locate files later.

******************************************************************************/

/* %%Function:DMSort %%Owner:chic */
DMSort()
{
	int irgfcSortDocs,irgfcGuess,irgfcLim;
	int irgfcNewCur;
	int WCompSz(), WCompCps(), ComparepDTTMDate();
	int numSBlanks,offsetT;
	int soffset=ichDMSortMin-1; /* index into the szSortField, not stSortField*/
	FC HUGE *hprgfcOld, HUGE *hprgfcNew;
	SB    sbNew;
	unsigned cbNew = 0;
	int fNotResort;
	int cch;
	CHAR *pchT;
	int (*pfn)();   /* function to call to do the comparison in QSort */
	CHAR rgchT[cchMaxDMFileSt];


	/* Get Offset of sort field in display string, Compare function, etc. */
	switch (DMFlags.dms)
		{
	case dmsName:
		soffset=1; /* index into the directory path index */
	case dmsAuthor:
	case dmsLastSavedBy:
		pfn= (int	(*)())WCompSz;
		break;
	case dmsCreationDate:
	case dmsRevisionDate:
		pfn= (int	(*)())ComparepDTTMDate;
		break;
	case dmsSize:
		pfn= (int	(*)())WCompCps;
		break;
		}
	hprgfcOld= hprgfcDMEnum;
	/* do the initial sorting */
	if (!FHpRgFcQSort(0,cDMFileMac-1,hprgfcOld,soffset,pfn,fnDMEnum))
		return;

	if (DMFlags.dms!=dmsAuthor && DMFlags.dms!=dmsLastSavedBy)
		return;   /* only Author or LastSavedBy has to look for blank field */

	irgfcDMFilenameStart=cDMFileMac;
	/* we know the exact size, so only allocate that many */
	if ((sbNew = SbAlloc(cDMFileMac*4, &cbNew)) == sbNil)
		{
		DMFlags.fDMOutOfMemory=fTrue;
		return;
		}

	if ((fNotResort = (DMFlags.dma != dma_Resort)))
		SetPromptMst(mstDMSort, pdcReport);

	hprgfcNew= (FC HUGE *)HpOfSbIb(sbNew,(IB) 0);
	irgfcSortDocs=0;
	irgfcLim=cDMFileMac;

	while (irgfcSortDocs+1 < irgfcLim)
		{
		CHAR ch;
		irgfcGuess=(irgfcSortDocs+irgfcLim) >>1;
		cch = CchSzFromFc(fnDMEnum,hprgfcOld[irgfcGuess],rgchT,cchMaxDMFileSt);
		Assert(cch > 1);
		if ( rgchT[soffset] ==0)
			irgfcSortDocs=irgfcGuess;
		else
			irgfcLim=irgfcGuess;
		}
	/***********
	irgfcSortDocs is now the index of the last doc whose sort field 
	starts with a \0. 
	************/
	if (irgfcSortDocs == 0 && rgchT[soffset]!=0)
		{
		FreeSb(sbNew);
		irgfcDMFilenameStart=cDMFileMac;
		if (fNotResort)
			RestorePrompt();
		return;       /* all the docs have Sort field info */
		}
	irgfcSortDocs++;
	/*************
	irgfcSortDocs now has the index of the first doc that doesn't 
	have an blank sort field 
	*************/
	BLTBH( hprgfcOld+irgfcSortDocs,hprgfcNew,(cDMFileMac-irgfcSortDocs)*4);
	irgfcDMFilenameStart=cDMFileMac-irgfcSortDocs;
	BLTBH( hprgfcOld, hprgfcNew+irgfcDMFilenameStart,irgfcSortDocs*4);
	/************
	Both groups are now in the proper order. Now 
	sort group 2 (no summary info) on the filename
	************/
	FHpRgFcQSort(irgfcDMFilenameStart,cDMFileMac-1,hprgfcNew,
			0,(int	(*)())WCompSz,fnDMEnum);

	FreeSb(sbDMEnum);
	hprgfcDMEnum= hprgfcNew;
	sbDMEnum=sbNew;
	Assert(cbNew != 0);
	cDMEnumMax=cbNew/4;  /* new max allocated size. We may have gotten more */
	if (fNotResort)
		RestorePrompt();
	return;              /* than we asked for. */
}


/*****************************************************************************
	WCompCps:
	This routine takes two CP *'s. It returns -1 if the first is smaller,
	1 if the second is smaller, and 0 if they are equal.
*****************************************************************************/
/* %%Function:WCompCps %%Owner:chic */
int WCompCps(pcpLeft,pcpRight)
CP *pcpLeft,*pcpRight;
	{  /*****
	note you can't just subtract and return the result, unless you
		return a long. If you return a word, you get the low word of the
		result, which could be anything.
	*****/
	if (*pcpLeft > *pcpRight)
		return(1);
	else  if (*pcpLeft < *pcpRight)
		return(-1);
	return(0);
}


/*****************************************************************************
	SzSpectoGrepRghsz:
	This routine takes a heap pointer to the text retrieval spec, and heap 
	pointers to the offset and length of each leaf in the string. It goes
	through each of the cLeaves leaves, translates the Word wildcards into
	the DSR grep wildcards, and creates an array of heap strings in rghsz.
*****************************************************************************/
/* %%Function:SzSpectoGrepRghsz %%Owner:chic */
SzSpectoGrepRghsz(szSpec,hrgdch,hrgLen,hrghsz,cLeaves)
CHAR *szSpec;        /* the text retrieval spec */
CHAR **hrgdch;      /* offsets of the start of each leaf */
CHAR **hrgLen;      /* length of each leaf */
CHAR **(**hrghsz);    /* array to put the resulting heap strings in */
int  cLeaves;       /* count of leaves to parse */
{
	CHAR **hsz;
	CHAR rgchBuffer[cchMaxSz+3],*pchSource,*pchDest;
	int iLeaf,iChar;

	for (iLeaf=0;iLeaf<cLeaves;iLeaf++)
		{
		pchDest=rgchBuffer;
		pchSource= szSpec + (*hrgdch)[iLeaf];
		for (iChar=0; iChar < (*hrgLen)[iLeaf]; iChar++, pchSource++)
			{
			if (pchDest-rgchBuffer > cchMaxSz)
				{   /* string grew past limit */
#ifdef FIX
				Error(IDMSG2Complex);
#endif
				break;
				}
			switch (*pchSource)
				{
			case chDQuote:
				/*****
				any quotes we see at this point are
				doubled, but we only want to copy one of them
				*****/
				pchSource++;    /* inc past first quote */
				iChar++;
			default:
				*pchDest++ = *pchSource;
				break;
			case chDot:
				*pchDest++ = chBackSlash;
				*pchDest++ = *pchSource;
				break;
			case chDOSWildSingle:
				*pchDest++ = chDot;
				break;
			case chDOSWildAll:
				*pchDest++ = chPound;    /* # */
				*pchDest++ = chDOSWildAll;
				break;
			case chCaret:
				pchSource++;
				iChar++;
				switch (*pchSource)
					{
				case chDOSWildSingle:
					*pchDest++ =chDOSWildSingle;
					break;
				case chDOSWildAll:
					*pchDest++ = chBackSlash;
					*pchDest++ = chDOSWildAll;
					break;
				case chCaret:
					*pchDest++ = chCaret;
					break;
				default:
					/* bad use of caret */
					*pchDest++ = chCaret;
					*pchDest++ = *pchSource;
					break;
					}   /* end caret switch */
				break;
				}   /* end *pchSource switch */
			}       /* end iChar for loop */
		*pchDest=0;
		if ((hsz = HszCreate(rgchBuffer)) != hNil)
			(*hrghsz)[iLeaf]= hsz;
		else 
/* caller checks for fMemFail and free allocated stuff */
			break;
		}           /* end iLeaf for loop */
}




/**********************************************************************
	FVSumdFromSz:
	This function checks to see if the file in vsumd corresponds to the
	desired file. If not, it updates it to be the summary info of the file.
**********************************************************************/
/* %%Function:FVSumdFromSz %%Owner:chic */
FVSumdFromSz(szFilename)
CHAR *szFilename;
{
	if (hszvsumdFile==0 || FNeRgch(szFilename,*hszvsumdFile,CchSz(szFilename)))
		{
		if (hszvsumdFile)
			{
			FreePSumd(&vsumd);  /* free the sumd */
			FreeH(hszvsumdFile);
			}
		if (!FSumdFromSz(szFilename,&vsumd))
			hszvsumdFile=hNil;
		else
			hszvsumdFile= HszCreate(szFilename);
		}
	return(hszvsumdFile != hNil);
}


/******************************************************************
CreateBlankSumd:
	This routine allocates null heap strings for a sumd, useful in the case
	where a summary descriptor for a file did not previously exist.
	****************************************************************/
/* %%Function:CreateBlankSumd %%Owner:chic */
CreateBlankSumd(psumd)
struct SUMD *psumd;
{
	CHAR rgchNull[1];
	rgchNull[0]=0;
	psumd->hszTitle= HszCreate(rgchNull);
	psumd->hszSubject= HszCreate(rgchNull);
	psumd->hszAuthor= HszCreate(rgchNull);
	psumd->hszKeywords= HszCreate(rgchNull);
	psumd->hszRevisor= HszCreate(rgchNull);
/* All these HszCreate calls can fail, caller checks for vmerr.fMemFail */
	psumd->dttmRevision = psumd->dttmCreation = DttmCur();
	psumd->cchDoc=cp0;
}



/******************************************************************
FreePSumd:
	This routine frees all the heap strings in a sumd.
	It assumes that the sumd was actually allocated, and that the heap ptrs 
	are valid. 
*******************************************************************/
/* %%Function:FreePSumd %%Owner:chic */
FreePSumd(psumd)
struct SUMD *psumd;
{
	FreeH(psumd->hszTitle);
	FreeH(psumd->hszSubject);
	FreeH(psumd->hszAuthor);
	FreeH(psumd->hszKeywords);
	FreeH(psumd->hszRevisor);
	SetBytes(psumd,0,sizeof(struct SUMD));
}


/* %%Function:SbAlloc %%Owner:chic */
SB SbAlloc(cbRequest, pcbNew)
unsigned cbRequest;
unsigned *pcbNew;
{
	SB sbRet = SbScanNext(fFalse);
	if (sbRet == sbNil)
		{
		if (sbMac != sbMax
				&& FInitSegTable (min (sbMac + 5, sbMax))) /* grow by 5 */
			sbRet = SbScanNext(fFalse);
		}

	if (sbRet != sbNil)
		{
		if ((*pcbNew = CbAllocSb(sbRet, cbRequest, HMEM_MOVEABLE | HMEM_EMM)) == 0)
			sbRet = sbNil;
		}
	return sbRet;
}


/* %%Function:rghszQSort %%Owner:chic */
rghszQSort(iLeft, iRight, rghsz, soffset)
int iLeft, iRight, soffset;
CHAR **rghsz[];
{
	if (iLeft < iRight)
		{
		int i = iLeft;
		int j = iRight+1;
		CHAR *pchKey = *(rghsz[i])+soffset; /* key to compare */
		for (;;)
			{
			while (i < j && WCompSz(*(rghsz[++i])+soffset, pchKey) < 0);
			while (j > 0 && WCompSz(*(rghsz[--j])+soffset, pchKey) > 0);
			if (i < j)
				rghszHswap (i, j, rghsz);
			else
				break;
			} /* end of for ever loop */
		rghszHswap(iLeft, j, rghsz);
		rghszQSort(iLeft, j-1, rghsz, soffset);
		rghszQSort(j+1, iRight, rghsz, soffset);
		}
} /* end of rghszQSort */


/* %%Function:rghszHswap  %%Owner:chic */
rghszHswap (i, j, rghsz)
int i, j;
unsigned **rghsz[];
{
	unsigned temp;

	temp = rghsz[i];
	rghsz[i] = rghsz[j];
	rghsz[j] = temp;
} /* end of rghszHswap */



/* %%Function:FreeDMQSD %%Owner:chic */
FreeDMQSD(pDmqsd)
struct DMQSD *pDmqsd;
{
	int i;
	CHAR **h;

	FreeH(pDmqsd->hstSpec);
	if (pDmqsd->hTree)
		FreeDMQSDTree(pDmqsd);
	if (pDmqsd == &(DMQueryD.dmqsdText))
		{
		if (DMQueryD.hrghszTLeaves != hNil)
			{
			for (i = 0; i < pDmqsd->cLeaves; i++)
				{
				if ((h = (*DMQueryD.hrghszTLeaves)[i]) != hNil)
					{
					AssertH(h);
					FreeH(h); /* free grep strings */
					}
				}
			FreePh(&(DMQueryD.hrghszTLeaves));
			}
		}
	SetBytes(pDmqsd, 0, sizeof(struct DMQSD));
}



/* %%Function:FreeDMQSDTree %%Owner:chic */
FreeDMQSDTree(pDMQSD)
struct DMQSD *pDMQSD;
{
	FreeAONTree(pDMQSD->hTree);
	FreeH(pDMQSD->hrgdchLeaves);
	FreeH(pDMQSD->hrgLenLeaves);
}


#ifdef DEBUG
/* %%Function:CkDMFn %%Owner:chic */
CkDMFn(fn)
int fn;
{
	struct FCB *pfcb;
	Assert(fn != fnNil);
	Assert(mpfnhfcb[fn] != hNil);
	AssertH(mpfnhfcb[fn]);
	pfcb = PfcbFn(fn);
	Assert(pfcb->fDMEnum);
	Assert(!pfcb->fDoc);
	Assert(!pfcb->fTemp);
}


/* %%Function:CkDMStruct %%Owner:chic */
CkDMStruct()
{
	if (DMQFlags.fTitleTest)
		{
		Ckpdmqsd(&DMQueryD.dmqsdTitle);
		}
	if (DMQFlags.fSubjectTest)
		{
		Ckpdmqsd(&DMQueryD.dmqsdSubject);
		}
	if (DMQFlags.fAuthorTest)
		{
		Ckpdmqsd(&DMQueryD.dmqsdAuthor);
		}
	if (DMQFlags.fKeywordTest)
		{
		Ckpdmqsd(&DMQueryD.dmqsdKeyword);
		}
	if (DMQFlags.fSavedByTest)
		{
		Ckpdmqsd(&DMQueryD.dmqsdSavedBy);
		}
	if (DMQFlags.fTextTest)
		{
		Ckpdmqsd(&DMQueryD.dmqsdText);
		Assert(DMQueryD.hrghszTLeaves);
		AssertH(DMQueryD.hrghszTLeaves);
		}

	if (DMQFlags.fCFromDateTest)
		{
		Assert((long)DMQueryD.CDateInfo.dttmFrom != 0L);
		}
	if (DMQFlags.fCToDateTest)
		{
		Assert((long)DMQueryD.CDateInfo.dttmTo != 0L);
		}
	if (DMQFlags.fSFromDateTest)
		{
		Assert((long)DMQueryD.SDateInfo.dttmFrom != 0L);
		}
	if (DMQFlags.fSToDateTest)
		{
		Assert((long)DMQueryD.SDateInfo.dttmTo != 0L);
		}

}


/* %%Function:Ckpdmqsd %%Owner:chic */
Ckpdmqsd(pdmqsd)
struct DMQSD *pdmqsd;
{
	AssertH(pdmqsd->hstSpec);
	AssertH(pdmqsd->hTree);
	Assert(pdmqsd->cLeaves > 0);
	Assert(pdmqsd->hrgLenLeaves);
	AssertH(pdmqsd->hrgLenLeaves);
	Assert(pdmqsd->hrgdchLeaves);
	AssertH(pdmqsd->hrgdchLeaves);
}


/* %%Function:DumpDMRgFc %%Owner:chic */
DumpDMRgFc()
{
	int i;

	for (i = 0; i < cDMFileMac; i++)
		{
		DumpDMFc(i);
		}
}


/* %%Function:DumpDMFc %%Owner:chic */
DumpDMFc(i)
int i;
{
	Assert(i < cDMFileMac);
	CommSzLong(SzFrame("fc = "), (long)hprgfcDMEnum[i]);
}


/* %%Function:DumpDMFn %%Owner:chic */
DumpDMFn(ich)
int ich;
{
	int i;
	int iMac = min(20, cDMFileMac);

	if (iMac != cDMFileMac)
		CommSzNum(SzShared("cDMFileMac: "), cDMFileMac);
	for (i = 0; i < iMac; i++)
		{
		DumpDMSz(i, ich);
		}
}


/* %%Function:DumpDMSz %%Owner:chic */
DumpDMSz(i, ich)
int i;
int ich;
{
	CHAR *pch;
	int rgNum[3];
	int cch;
	CHAR rgch[cchMaxDMFileSt];

	pch = rgch+ich;
	Assert(hprgfcDMEnum != (HP)NULL);
	cch = CchSzFromFc(fnDMEnum, hprgfcDMEnum[i], rgch, cchMaxDMFileSt);
	Assert(cch > 1);
	CommNumRgch(i, rgch+1, 13);
	switch (DMFlags.dms)
		{
	case dmsName:
		break;
	case dmsAuthor:
		CommSzSz(SzFrame("Author: "), pch);
		break;
	case dmsLastSavedBy:
		CommSzSz(SzFrame("LSB: "), pch);
		break;
	case dmsCreationDate:
	case dmsRevisionDate:
		rgNum[0] = (int)(((struct DTTM *)pch)->yr);
		rgNum[1] = (int)(((struct DTTM *)pch)->mon);
		rgNum[2] = (int)(((struct DTTM *)pch)->dom);
		CommSzRgNum(SzFrame("Year Month Day: "), rgNum, 3);
		break;
	case dmsSize:
		CommSzLong(SzFrame("Size: "), (long)*pch);
		break;
	default:
		Assert(fFalse);
		break;
		}
}


#endif /* DEBUG */


#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Docman2_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* Docman2_Last() */
