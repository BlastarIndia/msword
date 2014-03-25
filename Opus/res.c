/* R E S . C */
/* Permanently resident code */

#define NOVIRTUALKEYCODES
#define NOCTLMGR
#define NOWINSTYLES
#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOMENUS
#define NOCOMM
#define NOSOUND
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "disp.h"
#include "ch.h"
#include "doc.h"
#include "doslib.h"
#include "screen.h"
#include "props.h"
#include "sel.h"
#include "file.h"
#include "inter.h"
#include "ourmath.h"
#include "scc.h"
#include "debug.h"
#include "wininfo.h"
#include "idle.h"
#include "format.h"

extern HANDLE           vhInstance;
extern struct DOD       **mpdochdod[];
extern struct ITR       vitr;
extern int              vfMouseExist;
extern struct SEL       selCur;
extern struct SEL       selDotted;
extern int              vssc;
extern struct SCI       vsci;
extern struct MERR    	vmerr;
extern struct PLSBHI ** vhplsbhi;
extern int		cbMemChunk;
extern int              wwCur;
extern int              vfInCommit;
extern int              vfUrgentAlloc;
extern struct WWD    	** hwwdCur;
extern struct WWD 	**mpwwhwwd[];
extern struct MWD 	**mpmwhmwd[];
extern struct STTB	**vhsttbFont;
extern int              vsasCur;


IDF    vidf;
int cbMemChunk=0;       /* max chunk size of Emm memory (0 if no emm avail) */
int smtMemory;          /* System Memory Type */
ENV *penvMem;	    	/* used for SetJmp/DoJmps for out of memory recovery */

/* P D O D  D O C */
/* quick return of pdod for a doc */
/* NOTE: non-hand native version called from PCODE (makes code smaller) */
/*  %%Function: C_PdodDoc  %%Owner: bradv  */

struct DOD *C_PdodDoc(doc)
{
	Assert(doc > 0 && doc < docMax && mpdochdod[doc]);
	return *mpdochdod[doc];
}


/* **** +-+utility+-+structure **** */
/* ****
*  Description: get master font table index given an ftc
*      return iNil for an invalid ftc. */
/*  %%Function: IbstFontFromFtcDoc  %%Owner: bryanl  */

IbstFontFromFtcDoc (ftc, doc)
int ftc, doc;
{
	struct DOD *pdod;

	pdod = PdodMother( doc );
	if ((uns)ftc >= pdod->ftcMac)
		{
		ftc = ftcDefault;
		ReportSz( "Document contains out-of-range ftc!" );
		}
	Assert( (uns)(**pdod->hmpftcibstFont) [ftc] < (*vhsttbFont)->ibstMac );
	return ( (**pdod->hmpftcibstFont) [ftc] );
}


#ifdef NOASM /* in resn2.asm */
/* P S T  F R O M  S T T B
*  returns a pointer to the i'th st in the sttb specified by hsttb */
/*  %%Function: PstFromSttb  %%Owner: davidlu  */

HANDNATIVE char *PstFromSttb(hsttb, i)
struct STTB **hsttb;
int i;
{
	struct STTB *psttb = *hsttb;
	char *pst;
	Assert(!psttb->fExternal);
	pst = psttb->rgbst[i] + (char *)(psttb->rgbst);
	return (pst);
}
#endif /* NOASM */


#ifdef NOASM /* in resident.asm */
/* I  S C A N  L P R G W */
/*  %%Function: IScanLprgw  %%Owner: bradv  */

HANDNATIVE IScanLprgw( lprgw, w, iwMax )
WORD far *lprgw;
WORD w;
int iwMax;
{
	int iw;

	for ( iw = 0 ; iw < iwMax ; iw++, lprgw++ )
		if (*lprgw == w)
			return iw;

	return iNil;
}


#endif /* NOASM */

#ifdef NOTUSED
/* I  S C A N  L P R G B */
/*  %%Function: IScanLprgb  %%Owner: NOTUSED  */

NATIVE IScanLprgb( lprgb, b, ibMax ) /* WINIGNORE - unused */
BYTE far *lprgb;
BYTE b;
int ibMax;
{
	int ib;

	for ( ib = 0 ; ib < ibMax ; ib++, lprgb++ )
		if (*lprgb == b)
			return ib;

	return iNil;
}


#endif /* NOTUSED */

/* S Z  T O  S T  I N  P L A C E */
/*  %%Function: SzToStInPlace  %%Owner: rosiep  */

SzToStInPlace(pch)
CHAR *pch;
{
	int cch = umin(CchSz(pch)-1, 255);

	Assert(cch >= 0);
	bltbyte(pch, pch+1, cch);
	*pch = cch;
}


/* S T  T O  S Z  I N  P L A C E */
/*  %%Function: StToSzInPlace  %%Owner: rosiep  */

StToSzInPlace( pch )
CHAR *pch;
{
	*(bltbyte( pch+1, pch, *pch)) = '\0';
}


/* S Z  T O  S T  */
/*  %%Function: SzToSt  %%Owner: rosiep  */

SzToSt(sz, st)
CHAR *sz;
CHAR *st;
{
	int cch = umin(CchSz(sz)-1, 255);

	Assert(cch >= 0);
	bltbyte(sz, st+1, cch);
	*st = cch;
}


/* S T  T O  S Z */
/*  %%Function: StToSz  %%Owner: rosiep  */

StToSz( st, sz )
CHAR *st;
CHAR *sz;
{
	*(bltbyte(&st[1], sz, st[0])) = '\0';
}


/* S T  S T  A P P E N D */
/* append st2 to st1 */
/*  %%Function: StStAppend  %%Owner: rosiep  */

StStAppend(st1, st2)
char *st1, *st2;
{
	int cch;
	if ((cch = *st2) + *st1 > 255)
		cch = 255 - *st1;

	bltb(&st2[1], &st1[*st1 + 1], cch);
	*st1 += cch;
}


/*  %%Function: Isgn  %%Owner: rosiep  */

Isgn(i)
int i;
{
	return (i == 0) ? 0 : ((i < 0) ? -1 : 1);
}


/*  %%Function: Usgn  %%Owner: rosiep  */

Usgn(u)
uns u;
{
	return (u == 0) ? 0 : 1;
}


#ifdef NOASM /* now in resident.asm */
/*  %%Function: PchInSt  %%Owner: bradv  */

CHAR *PchInSt(st, ch)
CHAR *st;
int ch;
{

	CHAR *pch, *pchEnd;

	for (pch = &st[1], pchEnd = &st[1] + st[0]; pch < pchEnd; pch++)
		{
		if (*pch == ch)
			return(pch);
		}

	return(NULL);   /* character never found */
}


#endif /* NOASM */

#ifdef NOASM /* now in resident.asm */
/* F  A L P H A  N U M */
/*  %%Function: FAlphaNum  %%Owner: bradv  */

FAlphaNum(ch)
CHAR ch;
{
	return FAlpha(ch) || FDigit(ch);
}


#endif /* NOASM */


#ifdef NOASM /* now in resident.asm */
/* F  L O W E R */
/* ****
*  Description: Returns TRUE if ch is a lowercase letter, FALSE otherwise.
*      Note: even though DF and FF are lowercase, they have no
*      corresponding uppercase. They are included in the lowercase
*      set so they will appear as characters.
** **** */
/*  this is not using ChUpper/ChLower because that would be very slow -
		you would generally need 2 calls to determine the case. Since the
		ANSI set is fairly immutable, we are doing it this way. (bz)
	*/

/*  %%Function: FLower  %%Owner: bradv  */

HANDNATIVE FLower(ch)
CHAR ch;
{
	return (((uns)(ch - 'a') <= ('z' - 'a')) ||
			/* foreign */
	((uns)(ch - 0x00DF) <= (0x00F6 - 0x00DF)) ||
			((uns)(ch - 0x00F8) <= (0x00FF - 0x00F8)));
}


#endif /* NOASM */


#ifdef NOASM /* now in resident.asm */

#define chFirstUpperTbl  (224)
#define ch(_i)  ((CHAR) (_i))

/* French upper case mapping table - only chars >= E0 (224) are mapped */
csconst CHAR    mpchupFrench[] = {
	/* E0	    E1	     E2       E3       E4	E5	 E6	  E7   */
	ch(65),  ch(65),  ch(65),  ch(195), ch(196), ch(197), ch(198), ch(199),
	/* E8	    E9	     EA       EB       EC	ED	 EE	  EF   */
	ch(69),  ch(69),  ch(69),  ch(69),  ch(73),  ch(73),  ch(73),  ch(73),
	/* F0	    F1	     F2       F3       F4	F5	 F6	  F7   */
	ch(208), ch(209), ch(79),  ch(79),  ch(79),  ch(213), ch(214), ch(247),
	/* F8	    F9	     FA       FB       FC	FD	 FE	  FF   */
	ch(216), ch(85),  ch(85),  ch(85),  ch(220), ch(221), ch(222), ch(255)
};


#undef ch
#endif /* NOASM */

#ifdef NOASM /* now in resident.asm */
/* Q S Z  U P P E R  */
/*  upper case conversion using international rules (received from jurgenl 10-10-88 bz) */
/*  %%Function: QszUpper  %%Owner: bradv  */

EXPORT QszUpper(pch)
CHAR *pch;
{

	for (; *pch; pch++)
		*pch = ChUpper(*pch);
}


#endif /* NOASM */

#ifdef NOASM /* now in resident.asm */
/* C H U P P E R  */
/*  upper case conversion using international rules (received from jurgenl 10-10-88 bz) */
/*  %%Function: ChUpper  %%Owner: bradv  */

EXPORT ChUpper(ch)
int ch;
{
	if ((uns)(ch - 'a') <= ('z' - 'a'))
		return (ch - ('a' - 'A'));
	else  if (ch >= chFirstUpperTbl)  /* intl special chars */
		{
		if (!vitr.fFrench)
			return ((ch == 247 || ch == 255) ? ch : ch - 32);
		else
			return (mpchupFrench[ch - chFirstUpperTbl]);
		}
	else
		return (ch);
}


#endif /* NOASM */


/* #ifdef NOASM not yet in resident.asm */
/* C H U P P E R L O O K U P */
/* ****
*  Description: This is the same as ChUpper except that the behavior does
*	not depend on the country code in win.ini.  This is neccessary so
*	that binary searches and table lookups work consistently even if
*	keywords or user-defined names have extended characters.
*	This function should be used for all case-insensitive name/keyword
*	comparisons.  e.g. strtbl.c (fieldnames) command2.c (WHash())
** **** */
/*  %%Function: ChUpperLookup  %%Owner: bradv  */

EXPORT ChUpperLookup(ch)
int ch;
{
	BOOL fFrnT = vitr.fFrench;
	vitr.fFrench = fTrue;
	ch = ChUpper(ch);
	vitr.fFrench = fFrnT;
	return (ch);
}


/* #endif / NOASM */


#ifdef NOASM /* now in resident.asm */
/* ****
*  Description: Returns TRUE if ch is an uppercase letter, FALSE otherwise.
** **** */
/*  %%Function: FUpper  %%Owner: bradv  */

HANDNATIVE FUpper(ch)
CHAR ch;
{
	return  (((uns)(ch - 'A') <= ('Z' - 'A')) ||
			/* foreign */
	((uns)(ch - 0x00C0) <= (0x00D6 - 0x00C0)) ||
			((uns)(ch - 0x00D8) <= (0x00DE - 0x00D8)));
}


#endif /* NOASM */


#ifdef NOASM /* now in resident.asm */
/* ****
*  Description: Returns TRUE if ch is a digit, FALSE otherwise.
** **** */
/*  %%Function: FDigit  %%Owner: bradv  */

FDigit(ch)
CHAR ch;
{
	return((uns)(ch - '0') <= ('9' - '0'));
}


#endif /* NOASM */


#ifdef NOASM /* now in resident.asm */
/* F  A L P H A */
/* ****
*  Description: Returns TRUE if ch is a letter, FALSE otherwise.
*      Note: DF and FF are treated as lowercase, even though they have no
*      corresponding uppercase.
** **** */
/*  %%Function: FAlpha  %%Owner: bradv  */

FAlpha(ch)
CHAR ch;
{
	return(FLower(ch) || FUpper(ch));
}


#endif /* NOASM */




#ifdef NOASM /* now in resident.asm */
/* ****
*  Description: Copies string at pch1 to pch2i, including null terminator.
*               Returns number of chars moved, excluding null terminator.
** **** */
/*  %%Function: CchCopySz  %%Owner: rosiep  */

int CchCopySz(pch1, pch2)
register PCH pch1;
register PCH pch2;
{
	int     cch = 0;
	while ((*pch2++ = *pch1++) != 0)
		cch++;
	return cch;
} /* end of  C c h C o p y S z  */


#endif /* NOASM */



int cLongOpCount = 0; /* to ensure we don't do too much hide cursor */

/* ****
*  Description: Set up cursors, etc. to indicate a "long" operation is to
*     take place.
** **** */
/*  %%Function: StartLongOp  %%Owner: peterj  */

StartLongOp()
{
	extern int vfInLongOperation;
	extern HCURSOR vhcHourGlass;
	extern HWND vhwndDeskTop;

	if (cLongOpCount++ == 0)
		{
		vfInLongOperation = fTrue;

		if (!vfMouseExist)
			{ /* in a mouseless system, set the cursor to middle of window */
			extern HWND vhWndMsgBoxParent;
			extern HWND vhwndApp;
			extern int  vfInitializing;
			struct RC rc;
			struct PT pt;
			HWND  hWnd = vhWndMsgBoxParent;

			if (vhWndMsgBoxParent == NULL)
				hWnd = vhwndApp; /* next choice */
			if (!vfInitializing && hWnd != NULL && IsWindow(hWnd))
				{ /* we have a good window to put in */
				GetClientRect(hWnd, (LPRECT)&rc);
				pt.xp = (rc.xpRight - rc.xpLeft) / 2;
				pt.yp = (rc.ypBottom - rc.ypTop) / 2;
				ClientToScreen(hWnd, (LPPOINT)&pt);
				}
			else
				{ /* put in the middle of screen */
				pt.xp = vsci.dxpScreen / 2;
				pt.yp = vsci.dypScreen / 2;
				}
			SetCursorPos(pt.xp, pt.yp);
			}
		SetCursor(vhcHourGlass);
		EnsureCursorVisible();
		}
}


/* ****
*  Description: Restore cursor, etc. after a "long" operation. when we hit Idle
** **** */
/*  %%Function: EndLongOp  %%Owner: peterj  */

EndLongOp(fAll)
BOOL fAll;
{
	extern int vfInLongOperation;

	if (cLongOpCount == 1 || (fAll && cLongOpCount > 0))
		{
		vfInLongOperation = fFalse;
		EnsureCursorInvisibleMouseless();
		ChangeCursor(fFalse /*fExact*/);
		}

	cLongOpCount = (fAll ? 0 : max( 0, cLongOpCount - 1));
}


/*  %%Function: ChangeCursor  %%Owner: rosiep  */

ChangeCursor(fExact)
BOOL fExact;
{
	HWND hwnd;
	struct PT pt;

	if (vfMouseExist)
		{
		GetCursorPos((LPPOINT) &pt);
		hwnd = WindowFromPoint(pt);
		if (hwnd != NULL)
			ScreenToClient(hwnd, (LPPOINT) &pt);
		ForceCursorChange(hwnd, &pt, fExact);
		}
}



/* E N S U R E  C U R S O R  V I S I B L E */

/* Make sure the cursor is visible in a mouseless system */
/*  %%Function: EnsureCursorVisible  %%Owner: rosiep  */

EnsureCursorVisible()
{
	int cursorlevel;

	cursorlevel = GetSystemMetrics(SM_CURSORLEVEL);
	while (cursorlevel++ < 0)
		ShowCursor(fTrue);
}



/* E N S U R E  C U R S O R  I N V I S I B L E  M O U S E L E S S */

/* Make sure the cursor is still visible in a mouse system
	and invisible in a mouseless system */
/*  %%Function: EnsureCursorInvisibleMouseless  %%Owner: rosiep  */

EnsureCursorInvisibleMouseless()
{
	int cursorlevel;

	cursorlevel = GetSystemMetrics(SM_CURSORLEVEL);
	if (vfMouseExist)
		{
		while (cursorlevel++ < 0)
			ShowCursor(fTrue);
		}
	else  /* no mouse */		
		{
		while (cursorlevel-- >= 0)
			ShowCursor(fFalse);
		}
	SetCursor(NULL);
}


#ifdef NOASM /* now in resident.asm */
/*  %%Function: PInPl  %%Owner: bradv  */

HANDNATIVE char  *PInPl(hpl, i)
struct PL **hpl;
int i;
{
	struct PL *ppl = *hpl;

	AssertH(hpl);
	Assert(!ppl->fExternal);
	Assert(i >= 0 && i < (*hpl)->iMac);
	return (((char	*)ppl + ppl->brgfoo) + i * ppl->cb);
}


#endif /* NOASM */


/* F R E E  P H */
/*  %%Function: FreePh  %%Owner: peterj  */

FreePh(ph)
VOID ***ph;
{
	if (*ph != hNil)
		FreeH(*ph);
	*ph = hNil;
}


int vrgsbReserve[csbGrabMax];


/* S B  A L L O C  E M M  C B */
/* Try to allocate an SB with cb in it in EMM.  Use a reserve buffer
	if necessary.
*/
/*  %%Function: SbAllocEmmCb  %%Owner: peterj  */

SB SbAllocEmmCb(cb)
uns cb;
{
#ifdef DEBUG
	extern int vfAllocGuaranteed;
#endif /* DEBUG */
	SB sb = SbScanNext(fFalse);
	int isb;

#ifdef DEBUG
	/* check for forced failure */
	if (FFailLmemOpus(0, 0, 0, 0))
		{
		SetErrorMat(matMem);
		return sbNil;
		}
	vfAllocGuaranteed++;
#endif /* DEBUG */

	if (sb == sbNil && sbMac < sbMax
			&& FInitSegTable (min (sbMac + 5, sbMax))) /* grow by 5 */
		sb = SbScanNext(fFalse);

	if (sb != sbNil && CbAllocSb(sb, cb, hmemLmemHeap) == 0)
		/* couldn't allocate, resort to reserved sb */
		sb = sbNil;

	if (smtMemory == smtLIM && cb < cbMemChunk && (sb == sbNil || !FInEmmSb(sb)))
		/* alloc failed or could have gone into emm, but didn't--
				find a reserve sb */
		{
		SB sbRes;
		for (isb = 0; isb < csbGrabMax && (sbRes = vrgsbReserve[isb]) != sbNil;
				isb++)
			if (sbRes > 1 && sbRes < sbMax)
				/* found one in the reserve */
				{
				vrgsbReserve[isb] *= -1;
				if (sb != sbNil)
					/* free the other one */
					FreeSb(sb);
				sb = sbRes;
				goto LReturn;
				}
		}

LReturn:
	if (sb == sbNil)
		{
		if (vsasCur < sasMin && vfUrgentAlloc)
			/* Last resort, reduce swap area and use some of that for data */
			{
			ShrinkSwapArea();
			Assert(vsasCur >= sasMin);
			sb = SbAllocEmmCb(cb);
			GrowSwapArea();
			}
		else
			/* can't reduce any further */
			SetErrorMat(matMem);
		}

	Debug( vfAllocGuaranteed-- );
	ResetSbCur();
	return sb;
}


/* F R E E  E M M  S B */
/*  %%Function: FreeEmmSb  %%Owner: peterj  */

FreeEmmSb(sb)
SB sb;
{
	int sbTM1 = sb*-1;
	int isb;

	for (isb = 0; isb < csbGrabMax && vrgsbReserve[isb] != sbNil; isb++)
		if (vrgsbReserve[isb] == sbTM1)
			/* sb was from our reserve, put it back */
			{
			vrgsbReserve[isb] = sb;
			return;
			}

	/* not from our reserve, really free it */

	FreeSb(sb);
	ResetSbCur();
}


/* S B  F I N D  C B */
/*  Find an sb with sufficient space to hold an object cb bytes big.
*/
/*  %%Function: SbFindCb   %%Owner: peterj  */

NATIVE SB SbFindCb (cb)
uns cb;
{
	int isbhiMac = (*vhplsbhi)->isbhiMac;
	SB sb = sbNil;

#ifdef DLMEM
	CommSzLong(SzShared("SbFindCb: cb = "), (long)cb);
#endif /* DLMEM */

	if ((long)cb > lcbMaxFarObj)
		{
		SetErrorMat(matMem);
		return sbNil;
		}

#ifdef DEBUG
	if (vdbs.fReportHeap)
		ReportHeapSz(SzShared("memory usage.."), 200);
#endif /* DEBUG */

	Debug (vdbs.fShakeHeap ? ShakeHeap() : 0);
	FreezeHp();
	AssertH(vhplsbhi); /* set up by FInitStructs in initwin */

	/* find first available sb with a heap in which it will fit */
	if (cb < cbMinBig)
		{
		struct SBHI *psbhi, *psbhiMac;
		uns cbMac = umax(cb+cbOverheadMax, 255);

		for (psbhi = PInPl(vhplsbhi, 0), psbhiMac = psbhi + isbhiMac;
				psbhi < psbhiMac; psbhi++)
			if (psbhi->fHasHeap && CbAvailHeap(psbhi->sb) > cbMac)
				{
				sb = psbhi->sb;
				break;
				}
		}

	if (sb == sbNil)
		/* no available sb, create one */
		{
		struct SBHI sbhi;
		uns cbAlloc;
		BOOL fInstallLmem;

		if (cb >= cbMinBig)
			/* object too big, create in its own sb w/o a heap */
			{
			fInstallLmem = fFalse;
			cbAlloc = cb + 4;
#ifdef DLMEM
			CommSz(SzShared("Creating non-heap sb\r\n"));
#endif /* DLMEM */
			}
		else
			/* create a new sb with a heap */
			{
			fInstallLmem = fTrue;
			cbAlloc = cbLmemHeap;
#ifdef DLMEM
			CommSz(SzShared("Creating new sb with heap\r\n"));
#endif /* DLMEM */
			}

		if ((sb = SbAllocEmmCb(cbAlloc)) != sbNil)
			{
#ifdef DEBUG
			extern BOOL vfBogusHeapState;
			Assert(!vfBogusHeapState);
			vfBogusHeapState = fTrue;
#endif /* DEBUG */

			SetBytes(&sbhi, 0, cbSBHI);
			sbhi.sb = sb;
			if (fInstallLmem)
				{
				CreateHeap (sb);
				sbhi.fHasHeap = fTrue;
				}
			MeltHp();
			/* MOVES THE HEAP!! */
			if (!FInsertInPl(vhplsbhi, 0, &sbhi))
				{
				FreeEmmSb(sb);
				sb = sbNil;
				}
			FreezeHp();

#ifdef DEBUG
			vfBogusHeapState = fFalse;
#endif /* DEBUG */
			}
		}

	MeltHp();
	/* avoid side effects */
	ResetSbCur();

#ifdef DLMEM
	if (sb == sbNil)
		CommSz(SzShared("SbFindCb: failed!\r\n"));
	else
		CommSzNum(SzShared("SbFindCb: sb = "), sb);
#endif /* DLMEM */

	return sb;
}


/*  %%Function: PsbhiOfSb  %%Owner: peterj  */

struct SBHI *PsbhiOfSb(sb, pisbhi)
SB sb;
int *pisbhi; /* may be NULL */
{
	struct SBHI *psbhi = PInPl(vhplsbhi, 0);
	struct SBHI *psbhiBase = psbhi;
	struct SBHI *psbhiMac = psbhi + (*vhplsbhi)->isbhiMac;

	if (sb == sbNil)
		return NULL;

	while (psbhi < psbhiMac)
		if (psbhi->sb == sb)
			{
			if (pisbhi != NULL)
				*pisbhi = psbhi - psbhiBase;
			return psbhi;
			}
		else
			psbhi++;

	Assert(fFalse);
	return NULL;
}


/* H Q  A L L O C  L C B */
/*  Allocate a far heap object.
*/
/*  %%Function: HqAllocLcb   %%Owner: peterj  */

NATIVE HQ HqAllocLcb (lcb)
long lcb;
{
#ifdef DEBUG
	extern int vfAllocGuaranteed;
#endif
	HQ hq = hqNil;
	IB ibH;
	SB sb;
	uns cb = (uns)lcb;
	struct SBHI *psbhi;

	if (lcb >= lcbMaxFarObj)
		{
		SetErrorMat(matMem);
		return hqNil;
		}

#ifdef DEBUG
	/* check for forced failure */
	if (FFailLmemOpus(0, 0, 0, 0))
		{
		SetErrorMat(matMem);
		return hqNil;
		}
	vfAllocGuaranteed++;
#endif /* DEBUG */

	sb = SbFindCb (cb);/* POSSIBLE HEAP MOVEMENT */
	Debug (vdbs.fShakeHeap ? ShakeHeap() : 0);
	FreezeHp();

	if (sb == sbNil)
		goto LRet;

	else if ((psbhi=PsbhiOfSb(sb,NULL))->fHasHeap)
		{
		if ((ibH = PpvAllocCb(sb, cb)) == hNil)
			{
			Assert(fFalse);/* space should be assured by SbFindCb */
			goto LRet;
			}
		psbhi->ch++;
		hq = HpOfSbIb (sb, ibH);
		}

	else
		/* simulate a heap handle */
		/* sb has the form:
			IB = 0, *IB = 4 (ib of data block)
			IB = 2, *IB = cb(size of data block)
			IB = 4...       (data block of length cb)
		*/
		{
		hq = HpOfSbIb (sb, 0);
		*hq = 4;
		CbOfHq(hq) = cb; /* block size */
		psbhi->ch = 1;
		}

LRet:
#ifdef DLMEM
	if (hq == hqNil)
		CommSz(SzShared("HqAllocLcb: failed\r\n"));
#endif /* DLMEM */
	Debug( vfAllocGuaranteed-- );
	/* avoid side effects */
	ResetSbCur();
	MeltHp();
	return hq;
}


/* F R E E  H Q */
/* Free a far heap object.
*/
/*  %%Function: FreeHq  %%Owner: peterj  */

EXPORT FreeHq(hq)
HQ hq;
{
	IB ibH = IbOfHp(hq);
	SB sb = SbOfHp(hq);
	int isbhi;
	struct SBHI *psbhi;

	FreezeHp();
	Assert(sb != sbNil);

	if ((psbhi=PsbhiOfSb (sb, &isbhi)) != NULL)
		{
		if (psbhi->fHasHeap)
			FreePpv(sb, ibH);
		if (!--psbhi->ch && !psbhi->fHasFixed)
			/* free the sb */
			{
#ifdef DLMEM
			CommSzNum(SzShared("FreeHq: deleting sb = "), sb);
#endif /* DLMEM */
			FreeEmmSb (sb);
			DeleteFromPl (vhplsbhi, isbhi); /* should not move anything else */
			}
		else
			Assert(psbhi->fHasHeap);
		}
	/* avoid side effects */
	ResetSbCur();
	MeltHp();
}


/* F  C H N G  S I Z E  P H Q  C B */
/*  Change the size of a far heap object to cb bytes. 
*/
/*  %%Function: FChngSizePhqLcb   %%Owner: peterj  */

EXPORT FChngSizePhqLcb (phq, lcb)
HQ *phq;
long lcb;
{
#ifdef DEBUG
	extern int vfAllocGuaranteed;
#endif
	BOOL fSuccess = fFalse;
	uns cb = (uns)lcb;
	SB sbOld = SbOfHp(*phq);
	BOOL fBigOld = !PsbhiOfSb(sbOld, NULL)->fHasHeap;
	BOOL fBigNew = (cb >= cbMinBig - (fBigOld ? dcbBigSmall : 0));
	HQ hqNew;

#ifdef DLMEM
	CommSzLong(SzShared("FChngSizePhqCb: cb = "), (long)cb);
#endif /* DLMEM */

	if (lcb > lcbMaxFarObj)
		{
		SetErrorMat(matMem);
		return fFalse;
		}

#ifdef DEBUG
	/* check for forced failure (except when shrinking) */
	if (lcb > CbOfHq(*phq) && FFailLmemOpus(0, 0, 0, 0))
		{
		SetErrorMat(matMem);
		return fFalse;
		}
	vfAllocGuaranteed++;
#endif /* DEBUG */

	Debug (vdbs.fShakeHeap ? ShakeHeap() : 0);

	if (fBigOld)
		{
		if (fBigNew)
			{
#ifdef DLMEM
			CommSz(SzShared("Reallocating big object\r\n"));
#endif /* DLMEM */
			if (CbReallocSb(sbOld, cb+4, hmemLmemHeapRealloc) > 0)
				{
				CbOfHq(*phq) = cb;
				fSuccess = fTrue;
				}
			goto LRet;
			}
		/* else shrinking to something manageable */
		}

	else  if (!fBigNew && CbSizeSb(sbOld) <= cbLmemHeap+100/*rounding*/)
		/* was not "big" before and still isn't */
		{
		IB ibH = IbOfHp(*phq);
#ifdef DLMEM
		CommSz(SzShared("Attempting to reallocate heap object\r\n"));
#endif /* DLMEM */
		if (FReallocPpv(sbOld, ibH, cb))
			{
			fSuccess = fTrue;
			goto LRet;
			}
		/* else must put in a different sb */
		}

	/* fall through to here if the block must move to a different sb
		(either shrinking from big to !big or growing and won't fit in old
		sb). */
#ifdef DLMEM
	CommSz(SzShared("Moving to a new HQ\r\n"));
#endif /* DLMEM */
	if ((hqNew = HqAllocLcb ((long)cb)) == hqNil)
		goto LRet;

	/* locals used here to break up expression to keep compiler	from choking! */
		{
		int cbT = umin(cb, CbOfHq(*phq));
		bltbh (HpOfHq(*phq), HpOfHq(hqNew), cbT);
		}
	FreeHq (*phq);
	*phq = hqNew;
	fSuccess = fTrue;

LRet:
	if (!fSuccess && vsasCur < sasMin && vfUrgentAlloc)
		/* Last resort, reduce swap area and use some of that for data */
		{
		ShrinkSwapArea();
		Assert(vsasCur >= sasMin);
		fSuccess = FChngSizePhqLcb (phq, lcb);
		GrowSwapArea();
		}

	/* If we failed trying to shrink the object, we must have had a
		big object which is no longer big, so we tried to allocate
		heap space for it, but that allocation failed.  In this case,
		we pretend we didn't fail, because it is harmless.  We just
		keep the object in the sb it was in. */
	if (!fSuccess && lcb <= CbOfHq(*phq))
		fSuccess = fTrue;

#ifdef DLMEM
	if (!fSuccess)
		CommSz(SzShared("FChngSizePhqLcb: Failed!\r\n"));
#endif /* DLMEM */

	if (!fSuccess)
		SetErrorMat(matMem);

	Debug( vfAllocGuaranteed-- );
	/* avoid side effects */
	ResetSbCur();
	return fSuccess;
}


/*  %%Function: CbOfHplc  %%Owner: peterj  */

uns CbOfHplc(hplc)
struct PLC **hplc;
{
	AssertH(hplc);
	if ((*hplc)->fExternal)
		{
		Assert((*hplc)->hqplce != hpNil);
		return CbOfHq((*hplc)->hqplce);
		}
	else
		return CbOfH(hplc);
}


/* F  C H N G  S I Z E  H  C B  P R O C */
/*  %%Function: FChngSizeHCbProc  %%Owner: peterj  */

#ifdef DEBUG
EXPORT FChngSizeHCbProc(szFile, nLine, h, cb, fShrink)
char *szFile;
int nLine;
#else
EXPORT FChngSizeHCbProc(h, cb, fShrink)
#endif /* DEBUG */
VOID **h;
int cb;
BOOL fShrink;
{
	int cbCur = CbOfH(h);
	BOOL fReturn;
#ifdef DEBUG
	BOOL fGuaranteed = (cbCur >= cb);
/* note that sbmgr will die if we try to shrink when the heap is locked */
	if (cHpFreeze)
		FreezeProc(szFile, nLine);
	AssertH(h);
	if (fGuaranteed)
		vfAllocGuaranteed++;
#endif /* DEBUG */

/* if new size is the same, or new size is smaller but we don't want to
   shrink, there's nothing to do; otherwise resize the block */
	fReturn = cbCur == cb || (!fShrink && cbCur >= cb) ||
		FReallocPpv(sbDds, h, cb);

#ifdef DEBUG
	if (fGuaranteed)
		{
		Assert(fReturn);
		vfAllocGuaranteed--;
		}
	else if (vdbs.fShakeHeap)
		ShakeHeap();
#endif /* DEBUG */
	return fReturn;
}


#ifdef DEBUG

/*  %%Function: HAllocateCbProc  %%Owner: peterj  */

HAllocateCbProc(szFile, nLine, cb)
char *szFile;
int nLine;
int cb;
{
	if (cHpFreeze)
		FreezeProc(szFile, nLine);
	return PpvAllocCb(sbDds, cb);
}


#endif /* DEBUG */



/* F  R E T R Y  L M E M  E R R O R */
/* Called by lmem on an error before failing */
/*  %%Function: FRetryLmemError  %%Owner: peterj  */

EXPORT FRetryLmemError(merr, sb, cbNew, cRetry)
WORD merr;
SB sb;
WORD cbNew;
WORD cRetry;
{
	extern int dclMac;
	extern int vsasCur;
	extern struct PRC **vhprc;

#ifdef DLMEMFAIL
		{
		int rgw [5];
		rgw[0] = merr;
		rgw[1] = sb;
		rgw[2] = cbNew;
		rgw[3] = CbSizeSb(sb);
		rgw[4] = CbAvailHeap(sb);
		CommSzRgNum(SzShared("FRetryLmemError (merr,sb,cbNew,cbSb,cbAvail): "), rgw, 5);
		}
#endif /* DLMEMFAIL */

	if (sb == sbDds)
		{
		if (dclMac > 1)
			TerminateAllDdeDcl(fFalse);
		if (vmerr.fReclaimHprcs && vhprc != 0)
			{
			FreeUnusedPrcs();
			Assert(!vmerr.fReclaimHprcs);
			return fTrue; /* retry */
			}
		SetErrorMat(matMem);
		if (vmerr.hrgwEmerg1 == hNil || !vfUrgentAlloc)
			return fFalse; /* failed */
#ifdef DEBUG
		if (vdbs.fFixedMem)
			/* don't really want to free anything */
			{
			int cb = CbOfH(vmerr.hrgwEmerg1);
			FreePh(&vmerr.hrgwEmerg1);
			EatHeap(cb, &vdbs.hDebug);
			}
#endif /* DEBUG */
		FreePh(&vmerr.hrgwEmerg1);

		return fTrue; /* retry allocation */
		}
	else
		return fFalse;
}


/*  %%Function: PrcSet  %%Owner: rosiep  */

#ifdef NOASM /* now in resident.asm */
struct RC *PrcSet( prc, xpLeft, ypTop, xpRight, ypBottom )
struct RC *prc;
{
	prc->xpLeft = xpLeft;
	prc->xpRight = xpRight;
	prc->ypTop = ypTop;
	prc->ypBottom = ypBottom;

	return prc;
}


#endif /* NOASM */

#ifdef NOASM /* now in resident.asm */
/* F  N E  N C  S Z */
/*  case insensitive string compare */
/*  %%Function: FNeNcSz  %%Owner: rosiep  */

EXPORT FNeNcSz(sz1, sz2)
CHAR *sz1, *sz2;
{
	int cch = CchSz(sz1)-1;
	if (cch != CchSz(sz2)-1)
		return fTrue;
	return FNeNcRgch(sz1, sz2, cch);
}


#endif /* NOASM */

#ifdef NOASM /* now in resident.asm */
/* F  N E  N C  S T */
/*  case insensitive string compare */
/*  %%Function: FNeNcSt  %%Owner: rosiep  */

FNeNcSt(st1, st2)
CHAR *st1, *st2;
{
	int cch = *st1++;
	if (cch != *st2++)
		return fTrue;
	return FNeNcRgch(st1, st2, cch);
}


#endif /* NOASM */

#ifdef NOASM /* now in resident.asm */
/* F  N E  N C  R G C H  */
/*  case insensitive rgch compare */
/*  %%Function: FNeNcRgch  %%Owner: rosiep  */

FNeNcRgch(pch1, pch2, cch)
CHAR *pch1, *pch2;
int cch;
{
	while (cch--)
		if (ChUpperLookup(*pch1++) != ChUpperLookup(*pch2++))
			return fTrue;
	return fFalse;
}


#endif /* NOASM */


/*****************/
/* W w   D i s p */
/*  %%Function: WwDisp  %%Owner: chic  */

WwDisp(doc, ww, fDrMatch)
int doc, ww, fDrMatch;
	/* returns next ww displaying doc, wwNil if none; prime with wwNil to start */
{
	struct DR *pdr;
	int idr;
	struct WWD *pwwd;

	ww = (ww == wwNil) ? PdodMother(doc)->wwDisp : PwwdWw(ww)->wwDisp;
	for (; ww != wwNil; ww = pwwd->wwDisp)
		{
		pwwd = PwwdWw(ww);
		if (!fDrMatch && DocBaseWw(ww) == doc)
			return (ww);
		for (pdr = pwwd->rgdr, idr = pwwd->idrMac; idr-- > 0; ++pdr)
			if (pdr->doc == doc)
				return(ww);
		}
	return(wwNil);
}


/* C C H   L O N G   T O   P P C H */
/*  %%Function: CchLongToPpch  %%Owner: rosiep  */

int CchLongToPpch(l, ppch)
long l;
char **ppch;
{
	int cch = 0;

	if (l < 0)
		{
		*(*ppch)++ = '-';
		l = -l;
		++cch;
		}

	if (l >= 10)
		{
		cch += CchLongToPpch(l / 10, ppch);
		l %= 10;
		}

	*(*ppch)++ = '0' + (int)l;
	return(cch + 1);
}


#ifdef NOASM /* now in resident.asm */
/* P M W D  W W */
/*  %%Function: PmwdWw  %%Owner: chic  */

HANDNATIVE struct MWD *PmwdWw(ww)
{
#ifdef DEBUG
	int mw;
	struct WWD **hwwd;
	struct MWD **hmwd;
	Assert(ww > wwNil && ww < wwMax && (hwwd = mpwwhwwd[ww]));
	mw = (*hwwd)->mw;
	Assert(mw > mwNil && mw < mwMax && (hmwd = mpmwhmwd[mw]));
	return *hmwd;
#endif
	return *mpmwhmwd[(*mpwwhwwd[ww])->mw];
}


#endif /* NOASM */

#ifdef NOASM /* now in resident.asm */
/* H W W D  W W */
/*  %%Function: *HwwdWw  %%Owner: chic  */

HANDNATIVE struct WWD **HwwdWw(ww)
{
	Assert(ww > wwNil && ww < wwMax && mpwwhwwd[ww]);
	return mpwwhwwd[ww];
}


#endif /* NOASM */


/* G E T  S Z  F R O M  S T T B */
/*  %%Function: GetSzFromSttb  %%Owner: davidlu  */

GetSzFromSttb(hsttb, i, sz)
struct STTB **hsttb;
int i;
CHAR *sz;
{
	GetStFromSttb(hsttb, i, sz);
	StToSzInPlace(sz);
}


/* P S E L  A C T I V E */
/* returns &selCur or &selDotted */
/*  %%Function: PselActive  %%Owner: bobz  */

struct SEL *PselActive()
{

	/* bz notes. We hope to catch misuse of sels with all these
		asserts, in odd cases where the 2 selections are in different
		docs/windows.
		*/

	if (vssc == sscNil)
		{
#ifdef DEBUG
		Assert (selCur.ww == wwCur);
		if (hwwdCur != hNil && !(*hwwdCur)->fPageView)
			Assert(selCur.doc == (*hwwdCur)->sels.doc ||
					selCur.doc == docNil);
#endif
		return (&selCur);
		}
	else
		{
#ifdef DEBUG
		Assert (selDotted.ww == wwCur);
		if (hwwdCur != hNil && !(*hwwdCur)->fPageView)
			Assert(selDotted.doc == (*hwwdCur)->sels.doc ||
					selDotted.doc == docNil);
#endif
		return (&selDotted);
		}
}


/* W W  O T H E R  - return the other ww in a split window */
/* returns wwNil if window is not split */
/*  %%Function: WwOther  %%Owner: rosiep  */

EXPORT WwOther(ww)
{
	struct MWD *pmwd = PmwdWw(ww);
	if (!pmwd->fSplit)
		return wwNil;
	if (pmwd->wwUpper == ww)
		return pmwd->wwLower;
	return pmwd->wwUpper;
}


/* P W W D  O T H E R  - return pwwd of the other ww in a split window */
/*  %%Function: PwwdOther  %%Owner: rosiep  */

struct WWD *PwwdOther(ww)
{
	return PwwdWw(WwOther(ww));
}


/* G E T  L O N G  O P  S T A T E */
/*  %%Function: GetLongOpState   %%Owner: peterj  */

GetLongOpState (pw)
int *pw;
{
	*pw = cLongOpCount;
}


/* R E S E T  L O N G  O P  S T A T E */
/*  %%Function: ResetLongOpState   %%Owner: peterj  */

ResetLongOpState (w)
int w;
{
	if (w && !cLongOpCount)
		StartLongOp ();
	else  if (!w && cLongOpCount)
		EndLongOp (fTrue);
	cLongOpCount = w;
}


/* ****
*  Description: removes trailing spaces and spaces in front a string,
		appending a trailing null if any spaces are removed.
*    Used to clean the edit control strings
*    of combo boxes.
*  Returns: number of characters in resulting string, excluding null terminator
*  Note - cch is the number of characters in the string, excluding any null
*    terminator, so sz or st strings may be used.
** **** */
/*  %%Function: CchStripString  %%Owner: rosiep  */

int CchStripString(pch, cch)
CHAR *pch;
int cch;
{
	CHAR *pchNonBlank = pch;
	CHAR ch;

	if (!cch || ((ch = pch[cch - 1]) != ' ' && ch != chTab && 
			(ch = *pch) != ' ' && ch != chTab))
		return (cch);
	while (cch && ((ch = pch[cch - 1]) == ' ' || ch == chTab)) /* trailing */
		--cch;
	while (cch && ((ch = *pchNonBlank) == ' ' || ch == chTab)) /* in front */
		{
		--cch;
		pchNonBlank++;
		}
	if (pchNonBlank != pch && cch > 0)
		bltbyte(pchNonBlank, pch, cch);

	Assert(cch >= 0);
	pch[cch] = 0;
	return (cch);
}  /* CchStripString */


/* W  C O M P  S T */
/*  ANSI case insensitive comparison of two sts.
	returns 0 if st's are equal, negative if st1 preceeds st2 and positive
	if st2 preceeds st1 
*/
/*  %%Function: WCompSt  %%Owner: rosiep  */

int WCompSt(st1, st2)
char *st1, *st2;
{
	int cch;
	int dchUp;
	int dcb;

	/* Since the length byte might looks like an upper case char, we
				have to treat it separately, else it might be converted in
				ChLower */

	dcb = (*st1 - *st2);
	for (cch = min (*st1++, *st2++); cch--; ++st1, ++st2)
		{
		if ((dchUp = ChUpperLookup(*st1) - ChUpperLookup(*st2)) != 0)
			return (dchUp);
		}
	return (dcb);  /* strings were equal */
}


/* D O C  B A S E  W W  */
/* returns mother doc of a window */
/*  %%Function: DocBaseWw  %%Owner: chic  */

int DocBaseWw(ww)
int ww;
{
	struct MWD *pmwd = PmwdWw(ww);
	struct WWD *pwwd = PwwdWw(ww);
	struct DR *pdr;	/* for brain-dead native compiler */
	struct DRF drfFetch;

	if (pwwd->fPageView || ww == wwPreview || pwwd->idrMac == 0)
		return(pmwd->doc);
	pdr = PdrFetchAndFree(HwwdWw(ww), 0, &drfFetch);
	return(pdr->doc);
}


/* C C H  L P S Z  L E N */
/*  %%Function: CchLpszLen  %%Owner: rosiep  */

CchLpszLen(lpsz)
char far *lpsz;
{
	char ch;
	char far *lpszT;

	for (lpszT = lpsz; *lpszT; lpszT++)
		;
	return(LOWORD(lpszT) - LOWORD(lpsz));
}


/* C C H  H P S  T O  P P C H */
/* ****
*  Description: given an integer and a (char **), convert the integer
*               interpreted as twice as large of the intended value to
*               a character string (not null terminated) in *ppch.
*               (Gives ".5" if necessary.)
*
*  Input: number n, pointer to string ppch
*
*  Output:
*     Returns the number of characters in the output string (includes the
*     '-' character if number was negative).
*
*  Side Effects:
*     ppch points past the last character in the string. Handy for null
*     terminating strings.
*
** **** */

csconst CHAR szOneHalf[] = SzKey("5",HalfDef);

/*  %%Function: CchHpsToPpch  %%Owner: bobz  */

int CchHpsToPpch(hps, ppch)
int	hps;
CHAR	**ppch;
{
	int	cch;

	cch = CchIntToPpch(hps >> 1, ppch);

	if (hps % 2)
		{
		*(*ppch)++ = vitr.chDecimal;
		bltbx(szOneHalf, *ppch, sizeof(szOneHalf));
		*ppch += sizeof(szOneHalf)-1;
		cch += 1 + sizeof(szOneHalf)-1;
		}
	return cch;
}


#ifdef DEBUG
/* I B S T  F I N D  S Z F F N */
/* **** +-+utility+-+string **** */
/* ****
*  Description: Given an hsttb of ffn's  and a pointer to an ffn, return
*    the ibst for the string that matches the szFfn portion of the ffn,
*    ignoring the ffid portion. If no match, return iNil.
*    This is a variant of IbstFindSt in sttb.c.
*
*    This routine assumes user input of font names and so it does
*    a CASE-INSENSITIVE string compare.  It no longer
*    assumes you have already tested for the name "Default" since Tms Rmn
*    is now the default font, so we start looking at 0
*
** **** */

/*  %%Function: IbstFindSzFfn  %%Owner: davidlu  */

HANDNATIVE C_IbstFindSzFfn(hsttb, pffn)
struct STTB **hsttb;
struct FFN *pffn;
{
	struct STTB *psttb = *hsttb;
	char *st2;
	int ibst;

	Assert(!(*hsttb)->fExternal);

	for (ibst = 0; ibst < psttb->ibstMac; ibst++)
		{
		st2 = PstFromSttb(hsttb, ibst);
		if ( *st2 == *(CHAR *)pffn &&
				!FNeNcRgch(pffn->szFfn, ((struct FFN *)st2)->szFfn,
				CbSzOfPffn(pffn) - 1) )
			return(ibst);
		}
	return(iNil);
}
#endif /* DEBUG */


/* I N V A L  S E L  C U R  P R O P S */
/*  Invalidate the currently cached properties of the current
	selection.  This will cause each selCur.chp/chpGray & selCur.pap/papGray 
	to be updated before they are used again.
*/

/*  %%Function: InvalSelCurProps   %%Owner: bobz  */

InvalSelCurProps (fSelChanged)
BOOL fSelChanged;	/* WARNING NOT GUARANTEED TO BE JUST ONE OR ZERO */
{
	selCur.fUpdatePap = fTrue;
	if (fSelChanged || !selCur.fIns)
		selCur.fUpdateChp = fTrue;
	else
		selCur.fUpdateRibbon = fTrue;
	selCur.fUpdateChpGray = fTrue;
	if (fSelChanged)
		selCur.fUpdateStatLine = fTrue;
}


/* D I  D I R T Y  D O C */
/* checks if doc is dirty, including complex cases */
/*  %%Function: DiDirtyDoc  %%Owner: peterj  */

int DiDirtyDoc(doc)
{
	struct DOD *pdod = PdodDoc(doc);
	int docHdr;
	int di = 0;

	if (pdod->fDirty || ((pdod->fFtn || pdod->fAtn) && PdodDoc(pdod->doc)->fDirty))
		di += dimDoc;
	if (pdod->fStshDirty)
		di += dimStsh;
	if (!pdod->fShort && pdod->docFtn && PdodDoc(pdod->docFtn)->fDirty)
		di += dimFtn;
	if (!pdod->fShort && (docHdr = pdod->docHdr) != docNil)
		{
		if (PdodDoc(docHdr)->fDirty)
			{
			di += dimHdr;
			}
		else  if (PdodDoc(docHdr)->docHdr != docNil)
			/* check docHdrDisps also - works for mac too */
			{
			int docHdrDisp = docNil;
			int wwHdr;
			while ((wwHdr = WwHdrFromDocMom(doc, &docHdrDisp)) != wwNil)
				{
				if (PdodDoc(docHdrDisp)->fDirty)
					{
					di += dimHdr;
					break;
					}
				}
			}
		}
#ifdef WIN
	if (pdod->fDot && pdod->docGlsy && PdodDoc(pdod->docGlsy)->fDirty)
		di += dimGlsy;
	if (!pdod->fShort && pdod->docAtn && PdodDoc(pdod->docAtn)->fDirty)
		di += dimAtn;
#endif /* WIN */

	/*      if (pdod->fRepag)
		di += dimRepag;*/
	return di;
}


/*  %%Function: FStcDefined  %%Owner: davidbo  */

FStcDefined(doc, stc)
int doc, stc;
{
	struct STSH *pstsh = &PdodMother(doc)->stsh;
	Assert(stc > stcStdMin && stc < cstcMax);
	return ((cstcMax - stc) <= pstsh->cstcStd &&
			!FStcpEntryIsNull(pstsh->hsttbName,StcpFromStc(stc,pstsh->cstcStd)));
}


/* O U R  G L O B A L  A L L O C */
/* Perform a GlobalAlloc.  if it fails reduce our swap area and try again.
   reset swap area when done.
*/
/* %%function: OurGlobalAlloc %%owner: peterj */
HANDLE OurGlobalAlloc(wFlags, dwBytes)
WORD wFlags;
DWORD dwBytes;
{
	extern HANDLE GlobalAlloc2();

	/* note: one has to use special segment arithmetic to use objects greater
	   than 64K.  since we don't do that, prevent us from ever getting such an
	   object!
	*/
	if (dwBytes > 0x00010000L)
		{
		SetErrorMat(matMem);
		return NULL;
		}

	return(GlobalAlloc2(wFlags, dwBytes));
}


	
/* G L O B A L  A L L O C  2 */
/* Perform a GlobalAlloc.  if it fails reduce our swap area and try again.
   reset swap area when done.
   Allows allocations greater than 64K in size;
   remember to use special segment arithmetic (LpchIncr) on such blocks.
*/
/* %%function: GlobalAlloc2 %%owner: marksea */
HANDLE GlobalAlloc2(wFlags, dwBytes)
WORD wFlags;
DWORD dwBytes;
{
	HANDLE h;

	if ((h = GlobalAlloc(wFlags, dwBytes)) == NULL)
		{
		ShrinkSwapArea();
		if ((h = GlobalAlloc(wFlags, dwBytes)) == NULL)
			SetErrorMat(matMem);
		GrowSwapArea();
		}
	return h;
}


/* O U R  G L O B A L  R E  A L L O C */
/* Perform a GlobalReAlloc.  if it fails reduce our swap area and try again.
   reset swap area when done.
*/
/* %%function: OurGlobalReAlloc %%owner: peterj */
HANDLE OurGlobalReAlloc(hMem, dwBytes, wFlags)
HANDLE hMem;
DWORD dwBytes;
WORD wFlags;
{
	HANDLE h;

	/* note: one has to use special segment arithmetic (which is invalid under
	   win3 anyway...) to use objects greater than 64K.  since we don't do 
	   that, prevent us from ever getting such an object!
	*/
	if (dwBytes > 0x00010000L)
		{
		SetErrorMat(matMem);
		return NULL;
		}

	if ((h = GlobalReAlloc(hMem, dwBytes, wFlags)) == NULL)
		{
		ShrinkSwapArea();
		if ((h = GlobalReAlloc(hMem, dwBytes, wFlags)) == NULL)
			SetErrorMat(matMem);
		GrowSwapArea();
		}
	return h;
}


#ifdef NOASM
/* C C H  C O P Y  L P S Z  C C H  M A X */
/* Copy one sz to another, but not more than cchMax characters (incl '\0') */
/* %%Function:CchCopyLpszCchMax %%Owner:rosiep */
int CchCopyLpszCchMax(lpch1, lpch2, cchMax)
char far *lpch1;
char far *lpch2;
int cchMax;
{
	int     cch = 0;
	while (cch < cchMax && (*lpch2++ = *lpch1++) != 0)
		cch++;

	/* make sure it's null-terminated if we overflowed buffer */
	if (cch == cchMax)
		*(lpch2-1) = '\0';

	return cch;
}


#endif /* NOASM */


#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Res_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* Res_Last() */
