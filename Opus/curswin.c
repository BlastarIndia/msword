/* curswin.c-- cursor key movement subroutines */

#define RSHDEFS
#define NOGDICAPMASKS
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOCLIPBOARD
#define NOCTLMGR
#define NOSYSMETRICS
#define NOSYSCOMMANDS
#define NOCOMM
#define NOSOUND
#define NOMENUS
#define NOBRUSH
#define NOWNDCLASS
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "disp.h"
#include "props.h"
#include "doc.h"
#include "ch.h"
#include "keys.h"
#include "sel.h"
#include "format.h"
#include "screen.h"
#include "outline.h"
#include "debug.h"
#include "field.h"
#include "help.h"
#include "border.h"

#ifdef BRYANL
/* #define DCURS */
#endif

extern int	vfSeeSel;
extern struct CHP vchpFetch;
extern CP               vcpFetch;
extern uns              vccpFetch;
extern struct PAP vpapFetch;
extern BOOL             vfEndFetch;
extern struct CA caPara;
extern struct CA caTable;

extern struct FLI vfli;
extern struct SEL selCur;
extern int	wwCur;
extern struct WWD **hwwdCur;
extern int	vfLastCursor;
extern int	vxwCursor;
extern int	vgrpfKeyBoardState;

extern MSG              vmsgLast;
extern struct UAB vuab;
extern struct PREF vpref;

extern int	vicaInsert;

extern int	wwMac;
extern struct WWD **mpwwhwwd[];

extern struct SCI vsci;
extern BOOL             vfRecording;
extern BOOL             fElActive;
extern BOOL             vfExtendSel;
extern BOOL             vfBlockSel;
extern struct SEP vsepFetch;
extern struct CA caSect;
extern HWND             vhwndCBT;

extern struct CA caTap;
extern struct TCC vtcc;
extern struct TAP vtapFetch;
extern CP		vmpitccp[];
extern CP vcpFirstTableCell;

CP CpToFromCurs();
CP CpLimStyCW();
CP CpVisiBeforeCp();

/* C U R S  L E F T  R I G H T */
/* Move or select (fDrag) dSty sty units from the current position. */
/* %%Function:CursLeftRight %%Owner:chic */
CursLeftRight(sty, dSty, fDrag)
int	sty, dSty;
BOOL fDrag;
{
	struct SEL *psel = PselActive();
	int	doc = psel->doc;
	CP cpMac = CpMacDocEdit(doc);
	CP cpFrom, cpTo;
	CP cpFirst, cpLim;
	BOOL fForward;
	BOOL fEndFrom, fEndTo = fFalse;
	int	dl;
	CP cpAnchor;
	CP cpFromT;
	BOOL fBackStick;
	struct PLDR **hpldr;
	struct EDL edl;
	int	idr;
	CP cpT, cpDummy;
	BOOL fDummy;

	EnsureSelInWw(fTrue, psel);

	if (fDrag && psel->fTable && (psel->fRightward ? 
			(dSty > 0 ? psel->itcLim < vtapFetch.itcMac + 1 : 
			(psel->itcFirst != 0 || psel->itcFirst + 1 < psel->itcLim)) :
	(dSty < 0 ? psel->itcFirst > 0 : fTrue)))
				{
		sty = styChar;
		}

	/* I don't pretend to understand this, but I added the change
	that if dcpsel <= 1, don't backup 2. This prevents backing over
	a single char with an insert point into an earlier table... bz */

	else  if (fDrag && !psel->fWithinCell && 
			FInTableDocCp(doc, 
			cpT = psel->fForward ? 
			((dSty > 0 || psel->cpLim - psel->cpFirst <= 1) ? 
			psel->cpLim : CpMax(cp0, psel->cpLim - 2))
: /* !psel->fForward */
			(dSty > 0 ? psel->cpFirst + 1 : 
			CpMax(cp0, (psel->fTable && psel->itcFirst != 0) ? 
			psel->cpFirst : psel->cpFirst - 1))))
				{
		if (psel->fForward && dSty < 0)
			{
			vxwCursor = (*hwwdCur)->xwMac;
			vfLastCursor = fTrue;
			}

		CursUpDown(styLine, dSty, fDrag);

		return;
		}
	else  if (fDrag && psel->fTable)
		{
		ChangeSel(psel, dSty > 0 ? 
				CpLimSty(wwCur, doc, cpT, sty, fFalse) : 
				CpFirstSty (wwCur, doc, cpT, sty, fFalse), 
				sty, fFalse, fFalse);

		if (vfRecording && !fElActive)
			RecordCursor(sty, dSty, fDrag);

		return;
		}

#ifdef DCURS
		{
		int	rgw [3];

		rgw [0] = sty;
		rgw [1] = dSty;
		rgw [2] = fDrag;
		CommSzRgNum( SzShared( "CursLeftRight(sty, dSty, fDrag): "), rgw, 3);
		}
#endif
	if (fDrag && psel->fTable && (psel->sty == styRow || psel->sty == styWholeTable))
		return;

	if (vfRecording && !fElActive)
		RecordCursor(sty, dSty, fDrag);

	fEndFrom = fFalse;
	fForward = psel->fForward;
	cpAnchor = fForward ? psel->cpFirst : psel->cpLim;

	if (!vfBlockSel && psel->sk == skBlock)
		TurnOffBlock(psel);

	/* Move/extend backward ==> go to first cp of sty at psel->cpFirst,
							pretending psel->cpFirst belongs to the
							preceding sty if it is on a sty boundary.
	
	Move/extend forwards ==> go to lim cp of sty at psel->cpLim,
							psel->cpLim belongs to following sty.
	
	styLineEnd is a special case because:
			(1) fEndFrom value depends on whether psel->cpLim is at the
					end of a line
			(2) Home and end keys "stick" on current sty, don't advance
			(3) special case for going to the end of a line terminated
					with an Eol
	*/


	if (psel->fIns || fDrag && !psel->fTable)
		cpFrom = (fForward ? psel->cpLim : psel->cpFirst);
	else
		/* collapse to insertion point a la PC/Mac Word */
		cpFrom = (dSty > 0) ? 
				CpFirstSty(wwCur, doc, psel->cpLim, styChar, fTrue) : 
				CpLimSty(wwCur, doc, psel->cpFirst, styChar, fFalse);

	Assert (cpFrom >= cp0);

	/*  ensure user's window is up to date. */
	UpdateWw (wwCur, fFalse /* fAbortOk */);

	if (sty != styLineEnd)
		fEndFrom = dSty < 0;
	else  if (psel->fIns)
		fEndFrom = psel->fInsEnd;
	else
		{
		int	dl;

		dl = DlWhereDocCp(wwCur, doc, psel->cpFirst, fFalse,
				&hpldr, &idr, &cpFirst, &fDummy, fTrue);
		if (dl == dlNil)
			fEndFrom = fFalse;
		else
			{
			struct DRF drfFetch;

			GetPlc(PdrFetch(hpldr, idr, &drfFetch)->hplcedl, dl, &edl);
			FreePdrf(&drfFetch);
			fEndFrom = (cpFrom == cpFirst + edl.dcp);
			}
		}

#ifdef DCURS
	CommSzNum( SzShared( "  cpFrom = "), (int) cpFrom );
	CommSzNum( SzShared( "  fEndFrom = "), fEndFrom );
#endif

	if (fDrag && psel->fTable)
		{
		struct CA caTapAnchor;
		int	itcAnchor;
		CP cpExtend;
		int	itcExtend;

		CacheTc(wwNil, doc, psel->fTableAnchor ? psel->cpAnchor : psel->cpAnchor - 1, fFalse, fFalse);
		caTapAnchor = caTap;
		itcAnchor = vtcc.itc;
		CpFirstTap(doc, cpExtend = psel->fForward ? psel->cpLim - 1 : 
				psel->cpFirst);
		if (psel->fRightward)
			itcExtend = min(psel->itcLim, vtapFetch.itcMac + 1) + dSty - 1;
		else
			itcExtend = psel->itcFirst + dSty;
		itcExtend = max(0, min(itcExtend, vtapFetch.itcMac));
		cpExtend = CpFirstForItc(doc, cpExtend, itcExtend);
		SelectColumnTo(psel, cpExtend, itcExtend, &caTapAnchor, itcAnchor);
		if (vhwndCBT)
			CBTTblSelectPsel(&selCur);
		return;
		}
	else  if (!fDrag && psel->fTable)
		{
		if (dSty > 0)
			{
			int	itc;

			CpFirstTap(doc, (psel->sty != styWholeTable) ? 
					(psel->fTableAnchor ? psel->cpAnchor : psel->cpAnchor - 1) :
			cpFrom);
			if (sty == styLineEnd)
				{
				itc = min(psel->itcLim, vtapFetch.itcMac) - 1;
				while (!vtapFetch.rgtc[itc].fFirstMerged && vtapFetch.rgtc[itc].fMerged)
					itc--;
				cpTo = vmpitccp[itc+1] - ccpEop;
				}
			else
				{
				itc = min(psel->itcLim, vtapFetch.itcMac + 1);
				while (!vtapFetch.rgtc[itc].fFirstMerged && vtapFetch.rgtc[itc].fMerged)
					itc++;
				cpTo = vmpitccp[itc];
				}
			dSty--;
			}
		else
			{
			cpTo = CpFirstForItc(doc, (psel->sty != styWholeTable) ? 
					(psel->fTableAnchor ? psel->cpAnchor : psel->cpAnchor - 1) :
			CpMax(cp0, cpFrom - 1), psel->itcFirst);
			dSty++;
			}
		}


	if (!psel->fTable || (!fDrag && dSty != 0))
		cpTo = CpToFromCurs(psel, cpFrom, cpAnchor, sty, dSty, fEndFrom, fDrag, &fEndTo);


	fBackStick = (CpFirstSty(wwCur, doc, cpTo, styChar, cpTo > cpMac) == cpFrom);
	if ((cpFrom == cpTo) || (cpTo == cp0 && fBackStick && sty != styLineEnd) || 
			(PwwdWw(wwCur)->fOutline && !fDrag && (cpTo >= CpMacDocEdit(doc) || cpTo == cp0) && 
			!FCpVisiInOutline(wwCur, doc, cpTo, 1, &cpDummy)))
		goto LBeep;

	/* Vanished paragraph mark, don't stick case */
	if (sty == styPara)
		{
		if (cpTo < cpAnchor && fBackStick)
			{
			/*  stick will occur because first cp of para is vanished
					and ChangeSel (with fVisibleOnly true) will not
					include the vanished text in selection */
			cpFromT = CpFirstSty (wwCur, doc, cpFrom, styChar, fTrue);
			cpTo = CpToFromCurs (psel, cpFromT, cpAnchor, sty, dSty, fFalse,
					fDrag, &fEndTo);
			}
		}



	if (cpFrom == cpTo || !fDrag && cpFrom >= cpMac && dSty > 0)
		{
		/* beep if we're trying to go beyond beginning or end of doc */
		if (sty != styLineEnd && 
				/* take care of hidden characters at the end of doc */
		(cpTo == cp0 || CpFirstSty( wwCur, doc, cpFrom, styChar,
				(cpFrom > cpMac)) >= cpMac))
			{
LBeep:
			Beep();
			}
		if (fDrag || psel->fIns)
			goto LRet;
		}


	/* Set up new selection and make it visible */
	if (fDrag)
		{ /* Drag selection edge to new bound. */
		if (PdodDoc(doc)->fFtn)
			cpTo = CpMin(cpTo, CpMacDocEdit(doc) - ccpEop);

		/* Check for a collapsing case. */
		if (cpTo == ((dSty > 0) ? psel->cpLim : psel->cpFirst))
			goto LIns;

		ChangeSel( psel, cpTo, styNil, fTrue /* fVisibleOnly */, fFalse);
		if (psel->cpFirst == psel->cpLim)
			{
			/* cpTo could be cpMacDoc!!! */
			cpTo = psel->cpFirst;
			goto LIns;
			}
		}
	else
		{
LIns:
		Assert(cpTo <= CpMacDocEdit(doc));
		Select1(psel, cpTo, cpTo, (fEndTo ? maskInsEnd : 0) | maskSelChanged);
		}

	NormCp(wwCur, doc, CpActiveCur(), ncpVisifyPartialDl + 
			((psel->fIns || psel->sty < styPara) ? ncpHoriz : 0),
			((*hwwdCur)->ywMac - (*hwwdCur)->ywMin) >> 2, fFalse);
	if (!psel->fIns && (*hwwdCur)->fOutline)
		OutlineSelCheck(psel);

LRet:

	if (vhwndCBT)
		/* Send CBT a message explaining what we've selected */
		{
		if (psel->fTable)
			CBTTblSelectPsel(psel);
		else
			CBTSelectPsel(psel);
		}
}




/* C P  T O  F R O M  C U R S */
/* Return the destination cp base on cpFrom, style and direction of 
cursor movement. 
*/
/* %%Function:CpToFromCurs %%Owner:chic */
CP CpToFromCurs(psel, cpFrom, cpAnchor, sty, dSty, fEndFrom, fDrag, pfEndTo)
struct SEL *psel;
CP cpFrom, cpAnchor;
int	sty, dSty;
BOOL fEndFrom, fDrag;
BOOL *pfEndTo;
{
	int	cSty;
	CP (*pfnCp)();
	CP cpMac;
	CP cpTo;
	struct DOD *pdod;
	CP cpVisi, cpFromNew;
	struct CA caCharBlock;
	BOOL fOutline;

#ifdef DCURS
		{
		int	rgw [6];

		rgw [0] = (int) cpFrom;
		rgw [1] = (int) cpAnchor;
		rgw [2] = sty;
		rgw [3] = dSty;
		rgw [4] = fEndFrom;
		rgw [5] = fDrag;

		CommSzRgNum( SzShared( "  CpToFromCurs(cpFrom, cpAnchor, sty, dSty, fEndFrom, fDrag): "), rgw, 6);
		}
#endif
	*pfEndTo = fFalse;

	if (dSty < 0)
		{
		cSty = -dSty;
		pfnCp = CpFirstSty;
		}
	else
		{
		cSty = dSty;
		pfnCp = CpLimStyCW;
		}


		{
		CP cpToOld, cpToNew, cpFirst, cpLim;
		extern BOOL	fElActive;

		cpToOld = cpFrom;
		fOutline = (*hwwdCur)->fOutline;
		while (cSty-- > 0)
			{
			cpToNew = (*pfnCp)(wwCur, psel->doc, cpToOld, sty, fEndFrom);

			if (fOutline && dSty > 0 && 
					(sty == styWord || sty == styPara) && !fDrag)
				{
				/* word right or para right may result in cp after a eop
				which can be at the subtext in outline mode that is not showing.
				CpLimSty will skip subtext to avoid invisi ip */
				cpToNew = CpLimSty(wwCur, psel->doc, cpToNew, styChar, fTrue /*fInsEnd*/);
				}

			/* Don't allow cursor to land in the non-visible portion of a merged cell */
			if (FInTableDocCp(psel->doc, cpToNew))
				{
LCacheTc:
				CacheTc(wwNil, psel->doc, cpToNew, fFalse, fFalse);
				CpFirstTap(psel->doc, cpToNew);
				if (vtapFetch.rgtc[vtcc.itc].fMerged)
					{
					if (dSty > 0)
						cpToNew = vmpitccp[vtcc.itc + 1];
					else
						cpToNew = vmpitccp[vtcc.itc] - ccpEop;
					goto LCacheTc;
					}
				}
			cpToOld = cpToNew;
			}

		cpTo = cpToNew;
		}

	if (sty == styPara && (!fDrag || fOutline) && cpTo <= CpMacDocEdit(psel->doc))
		{
		/*  TEST CASES to account for visibility
		---- stands for formatted hidden
		~~~~ stands for caCharBlock
		
		GALLEY 
		
		case 1)                                     solved by 
		cpVisi,cpTo --
					|                          2nd while test and
					v                          step 3A: 
		alpha<cr>beta<cr>somemore<cr>gamma<cr>      Cp?Sty using cpFromNew being 
				----                           caCharBlock.cpFirst/Lim
				~~~~~
		
		case 2)
			cpTo --   -- cpVisi
					|   |                      step 1: 
					v   v                      cpTo == caCharBlock.cpFist
		alpha<cr>beta<cr>somemore<cr>gamma<cr>
					----
					~~~~~
		case 3)
			cpTo --   -- cpVisi
					|   |                      step 3A:
					v   v                      Cp?Sty using cpFromNew being 
		alpha<cr>beta<cr>somemore<cr>gamma<cr>      caCharBlock.cpFirst/Lim
				--------
				~~~~~~~~~
		case 3')
			cpTo --   -- cpVisi
					|   |                      same as case 3)
					v   v                      
		alpha<cr>beta<cr>somemore<cr>gamma<cr>      
		-----------------
		~~~~~~~~~~~~~~~~~~
		
		OUTLINE - Show level 1
		dSty > 0 means going from "alpha" to the next para,
		dSty < 0 means going from "gamma" to the prev para.
		cpTo is the cp returned from the first Cp?Sty before the while loop.
		In outline, the visi para is grouped with the invisi text to the right 
		as one char unit.
		
		case 4) 
		cpVisi-  - cpTo (dSty > 0)                  dSty > 0 : step 2B, 3A 
		|  |       - cpTo (dSty < 0)          dSty < 0 : 3A
		|  |       |
		v  v       v
		alpha<cr>beta<cr>text<cr>more<cr>gamma<cr>
		~~~~~~~~~~~~~~~~~~~~
		L1       L2      L2      L1      L1
		
		case 4')
		cpVisi-  - cpTo                             same as case 4)
		|  |
		v  v
		alpha<cr>beta<cr>somemore<cr>gamma<cr>
		~~~~~~~~~~~~
		L1       L2      L1          L1
		
		case 5)
		cpVisi-  - cpTo                     step 2A, 2Ab 
				|  |
				v  v
		alpha<cr>beta<cr>somemore<cr>gamma<cr>
					----
				~~~~~~~~
		L1       L1      L1          L1
		
		case 6)
		cpVisi-  - cpTo (dSty > 0)                  dSty > 0 : step 2B, 3A, 2Ab 
		|  |       - cpTo (dSty < 0)          dSty < 0 : step 3A
		|  |       |                          dSty < 0 : (from "more" to 
		v  v       v                            "alpha" step 2A, 2Aa, 3A
		alpha<cr>beta<cr>somemore<cr>gamma<cr>
					----
		~~~~~~~~~~~~~~~~
		L1       L2      L1          L1
		*/
		cpMac = CpMacDocEdit(psel->doc);
		GetCaCharBlock(psel->ww, psel->doc, cpTo, &caCharBlock, &cpVisi);
		if (cpTo < caCharBlock.cpFirst)
			{
			/* caCharBlock has a special case where it will not include cp,
			e.g.if cp is in non showing text at the beginning of the doc 
			in outline or all hidden text at the beginning of the doc before
			a table, therefore no need to go through the loop or else it 
			will not get out */
			cpTo = caCharBlock.cpFirst;
			}
		else
			{
			while (cpTo != cpVisi || 
					!(cpTo == caCharBlock.cpFirst || cpTo == caCharBlock.cpLim))
				{
#ifdef DEBUG
				CP cpToDebug = cpTo;
#endif
/*1*/			if (cpTo == caCharBlock.cpFirst)
					/* case of cp0 or cpFirst of para is hidden */
					break;
				cpFromNew = dSty < 0 ? caCharBlock.cpFirst : caCharBlock.cpLim;
				if (fOutline && cpTo < cpMac)
					{
					CachePara(psel->doc, cpTo);
					if (caCharBlock.cpLim < caPara.cpLim)
						{
/*2A*/					cpTo = caCharBlock.cpLim;
						if (dSty < 0 && cpFrom == cpTo)
/*2Aa*/						cpFromNew = caCharBlock.cpFirst;
						else
/*2Ab*/						break;
						}
					else  if (dSty > 0)
						{
/*2B*/					CachePara(psel->doc, caCharBlock.cpLim);
						cpFromNew = caPara.cpFirst - ccpEop;
						}
					}

				if (cpFromNew > cp0 && cpFromNew < cpMac)
					{
/*3A*/				cpTo = (*pfnCp)(wwCur, psel->doc, cpFromNew, sty, fEndFrom);
					GetCaCharBlock(psel->ww, psel->doc, cpTo, &caCharBlock, &cpVisi);
					}
				else
					{
					cpTo = cpFrom;
					break;
					}
#ifdef DEBUG
				if (cpToDebug == cpTo)
					{
					Assert(fFalse); /* will loop forever if cpTo is not changed */
					break;
					}
#endif
				} /* end of while */
			}
		}

	/* styLineEnd special case: CpLimSty will not return beyond CpMacDocEdit;
	adjust */

#ifdef DCURS
	CommSzNum( SzShared( "    after Cp???Sty, cpTo = "), (int)cpTo );
#endif
	if (sty == styLineEnd && dSty > 0 && cpTo > cp0 /* if nothing visible, cpTo can be cp0 */)
		{
		*pfEndTo = fTrue;

		/* styLineEnd special case: CpLimSty will return Mac cp of edl;
		need to adjust if going to end of: splat line, eop-terminated line */

		if (!fDrag || cpTo <= cpAnchor)
			{
			Assert( cpTo > cp0 );
			FormatLine( psel->ww, psel->doc, CpFirstSty(wwCur,
					psel->doc, cpTo, styLineEnd, fTrue));
			/* line consists of a splat: "end" is at line start */
			if (!FInTableVPapFetch(psel->doc, cpTo - 1) && 
					((vfli.fSplats && !vfli.fOutline) || 
					(vfli.fOutline && (vfli.fSplatBreak || 
					vfli.fSplatColumn)) || 
					(vfli.chBreak == chCRJ && !vfli.fOutline)))
				{
				cpTo--;
				*pfEndTo = fFalse;
				}
			else
				{
				CachePara(psel->doc, cpTo - 1);
				if (caPara.cpLim == cpTo)
					{
					/* line ends with para mark: "end" is right before para mark */
					cpTo -= ccpEop;
					*pfEndTo = fFalse;
					}
				}
			}
#ifdef DCURS
		CommSzNum( SzShared( "    after styLineEnd adjust, cpTo = "), (int)cpTo );
#endif
		}

	/* Limit movement of insertion point to before final EOP, although drags can
	select it. */
	cpMac = CpMacDoc( psel->doc );
	if (!fDrag || cpTo <= cpAnchor)
		cpMac -= ccpEop;

	if (cpTo > cpMac)
		/* cannot simply assign cpMac to it because it could be invisible */
		cpTo = (sty == styLineEnd ? cpMac : CpFirstSty(psel->ww, psel->doc, cpTo, styChar, fTrue));

	/* don't let the cp stays where there is no more visible text after it */
	if (!fDrag)
		{
		int	ccpFetch = 0;
		FetchCpPccpVisible(psel->doc, cpTo, &ccpFetch, psel->ww, fFalse);
		if (ccpFetch == 0)
			{
			CP cpT;
			/* get to closest last visi cp if styLineEnd */
			cpTo = (sty != styLineEnd || 
					(cpT = CpVisiBeforeCp(psel->ww, psel->doc, cpTo)) == cpNil) ? 
					cpFrom : cpT;
			}
		else  if (FInTableDocCp(psel->doc, vcpFetch) && vcpFirstTableCell > cpTo)
			{
			/* cpTo is hidden text in front of table, ip will be invisi, don't move */
			cpTo = cpFrom;
			}
		}
	Assert( cpTo >= cp0 );

#ifdef DCURS
		{
		int	rgw [2];

		rgw [0] = (int) cpTo;
		rgw [1] = *pfEndTo;
		CommSzRgNum( SzShared( "    CpToFromCurs returns (cpTo, fEndTo): "), rgw, 2 );
		}
#endif
	return(cpTo);
}


/* C p L i m  S t y  C W */
/* %%Function:CpLimStyCW %%Owner:chic */
/* Basically just calling CpLimSty, have a speed up for styLineEnd 
case in cpMac boundary.
*/
CP CpLimStyCW( ww, doc, cp, sty, fEnd )
int	ww, doc;
CP cp;
int	sty, fEnd;
{
	if (sty == styLineEnd && 
			(cp > CpMacDocEdit(doc) || (cp == CpMacDocEdit(doc) && !fEnd)))
		return CpMacDoc(doc);
	else
		return CpLimSty( ww, doc, cp, sty, fEnd );
}


/* *** COMMANDS FOR TABLE MOVEMENT *** */

/* C M D  B E G I N  R O W */
/* Go to beginning of current row in table. */
/* %%Function:CmdBeginRow %%Owner:rosiep */
CMD CmdBeginRow(pcmb)
CMB *pcmb;
{
	struct CA caCell, caTapAnchor;
	int	itcAnchor;
	CP cpFirstOppositeAnchor;
	struct SEL *psel = PselActive();

	if (FInTableDocCp(psel->doc, psel->cpFirst))
		{
		CpFirstTap(psel->doc, psel->cpFirst);
		if (vfShiftKey || vfExtendSel)
			{
			if (psel->fTable)
				{
				/* don't need the return value; we know that already */
				FSelAnchoredInTable(psel, &caCell, &caTapAnchor, &itcAnchor);
				if (psel->cpFirst != caTapAnchor.cpFirst)
					cpFirstOppositeAnchor = psel->cpFirst;
				else
					cpFirstOppositeAnchor = CpFirstTap(psel->doc, psel->cpLim - 1);
				SelectColumnTo(psel, cpFirstOppositeAnchor, 0, &caTapAnchor, itcAnchor);
				}
			else
				{
				Assert(psel->fWithinCell);
				CacheTc(wwNil, psel->doc, psel->cpFirst, fFalse, fFalse);
				Select(psel, caTap.cpFirst, vtcc.cpLim);
				}
			}
		else
			SelectIns(psel, caTap.cpFirst);
		if (vhwndCBT)
			CBTTblSelectPsel(psel);
		}
	else
		CursLeftRight(styLineEnd, -1, vfShiftKey || vfExtendSel);

	vfLastCursor = fFalse;
	vfSeeSel = fTrue;
	return cmdOK;
}



/* C M D  E N D  R O W */
/* Go to last cell of current row in table. */
/* %%Function:CmdEndRow %%Owner:rosiep */
CMD CmdEndRow(pcmb)
CMB *pcmb;
{
	struct SEL *psel = PselActive();
	struct CA caCell, caTapAnchor;
	int	itcAnchor, itc;
	CP cpLimOppositeAnchor;
	CP cpT = CpMax(psel->cpLim - (psel->fIns ? 0 : 1), 0);

	if (FInTableDocCp(psel->doc, cpT))
		{
		CpFirstTap(psel->doc, cpT);
		CacheTc(wwCur, psel->doc, caTap.cpLim - ccpEop - 1, fFalse, fFalse);
		itc = vtcc.itc;
		while (vtapFetch.rgtc[itc].fMerged)
			itc--;
		if (vfShiftKey || vfExtendSel)
			{
			if (psel->fTable)
				{
				/* don't need the return value; we know that already */
				FSelAnchoredInTable(psel, &caCell, &caTapAnchor, &itcAnchor);
				if (psel->cpLim != caTapAnchor.cpLim)
					cpLimOppositeAnchor = psel->cpLim - ccpEop;
				else
					{
					CpFirstTap(psel->doc, psel->cpFirst);
					cpLimOppositeAnchor = caTap.cpLim - ccpEop;
					}
				SelectColumnTo(psel, cpLimOppositeAnchor, itc, &caTapAnchor, itcAnchor);
				}
			else
				{
				Assert(psel->fWithinCell);
				/* TAP cached above; be careful if changing code */
				Select(psel, psel->cpFirst, caTap.cpLim - ccpEop);
				}
			}
		else
			{
			/* TAP cached above; be careful if changing code */
			PcaColumnItc(&caCell, &caTap, itc);
			SelectIns(psel, caCell.cpFirst);
			}
		if (vhwndCBT)
			CBTTblSelectPsel(psel);
		}
	else
		CursLeftRight(styLineEnd, 1, vfShiftKey || vfExtendSel);

	vfLastCursor = fFalse;
	vfSeeSel = fTrue;
	return cmdOK;
}


/* C M D  T O P  C O L U M N */
/* Go to same column in first row of table */
/* %%Function:CmdTopColumn %%Owner:rosiep */
CMD CmdTopColumn(pcmb)
CMB *pcmb;
{
	struct CA caCell, caTapAnchor;
	int	itcAnchor;
	struct CA caT;
	int	itc;  /* itc opposite anchor */
	struct SEL *psel = PselActive();

	if (FInTableDocCp(psel->doc, psel->cpFirst) && 
			FSelAnchoredInTable(psel, &caCell, &caTapAnchor, &itcAnchor))
		{
		if (psel->fTable)
			{
			/* don't need the return value; we know that already */
			itc = (psel->itcFirst == itcAnchor ? psel->itcLim - 1 : psel->itcFirst);
			}
		else
			itc = (psel->fWithinCell) ? ItcFromDocCp(psel->doc, psel->cpFirst) : itcAnchor;
		CpFirstTap(psel->doc, CpTableFirst(psel->doc, psel->cpFirst));
		itc = min(itc, vtapFetch.itcMac);
		while (vtapFetch.rgtc[itc].fMerged)
			itc--;
		PcaColumnItc(&caT, &caTap, itc);
		if (vfShiftKey || vfExtendSel)
			{
			if (psel->fTable)
				SelectColumnTo(psel, caT.cpFirst, itc, &caTapAnchor, itcAnchor);
			else  if (!psel->fWithinCell)
				ChangeSel(psel, caT.cpFirst, styNil, fFalse, fFalse);
			else					
				{
				Assert(psel->fWithinCell);
				CpFirstTap(psel->doc, psel->cpFirst);
				SelectColumn(psel, caT.cpFirst, caTap.cpLim - 1, itc, itc + 1);
				}
			}
		else
			SelectIns(psel, caT.cpFirst);
		if (vhwndCBT)
			CBTTblSelectPsel(psel);
		vfLastCursor = fFalse;
		}
	else
		Beep();

	vfSeeSel = fTrue;
	return cmdOK;
}


/* C M D  B O T T O M  C O L U M N */
/* Go to same column in bottom row of table */
/* %%Function:CmdBottomColumn %%Owner:rosiep */
CMD CmdBottomColumn(pcmb)
CMB *pcmb;
{
	struct CA caCell, caTapAnchor, caTapLim;
	int	itcAnchor;
	struct CA caT;
	int	itc;
	struct SEL *psel = PselActive();

	if (FInTableDocCp(psel->doc, psel->cpLim))
		{
		if (psel->fTable)
			{
			/* don't need the return value; we know that already */
			FSelAnchoredInTable(psel, &caCell, &caTapAnchor, &itcAnchor);
			itc = (psel->itcFirst == itcAnchor ? psel->itcLim - 1 : psel->itcFirst);
			}
		else
			itc = ItcFromDocCp(psel->doc, psel->cpLim);
		CpFirstTap(psel->doc, CpTableLim(psel->doc, psel->cpLim) - 1);
		caTapLim = caTap; 
		itc = min(itc, vtapFetch.itcMac);
		while (vtapFetch.rgtc[itc].fMerged)
			itc--;
		PcaColumnItc(&caT, &caTap, itc);
		if (vfShiftKey || vfExtendSel)
			{
			if (psel->fTable)
				SelectColumnTo(psel, caT.cpFirst, itc, &caTapAnchor, itcAnchor);
			else
				{
				Assert(psel->fWithinCell);
				CpFirstTap(psel->doc, psel->cpFirst);
				SelectColumn(psel, caTap.cpFirst, caTapLim.cpLim, itc, itc + 1);
				}
			}
		else
			SelectIns(psel, caT.cpFirst);
		if (vhwndCBT)
			CBTTblSelectPsel(psel);
		vfLastCursor = fFalse;
		}
	else
		Beep();

	vfSeeSel = fTrue;
	return cmdOK;
}


#define tsdEndOfRow	0
#define tsdStartOfRow	1
#define tsdEndOfCol	2
#define tsdStartOfCol	3


CMD (*mptsdpfnCmd [])() = 
{
	CmdEndRow,
			CmdBeginRow,
			CmdBottomColumn,
			CmdTopColumn
};


/* %%Function:FElTblSel %%Owner:bradch */
BOOL FElTblSel(fExtend, tsd)
{
	BOOL fExtSav;
	struct CA caBefore;

	fExtSav = vfExtendSel;
	caBefore = selCur.ca;
	vfExtendSel = fExtend;
	(*mptsdpfnCmd[tsd])(NULL);
	vfExtendSel = fExtSav;
	return FNeRgch(&selCur.ca, &caBefore, sizeof (struct CA ));
}


/* %%Function:ElEndOfRow %%Owner:bradch */
EL ElEndOfRow(fExtend)
BOOL fExtend;
{
	return FElTblSel(fExtend, tsdEndOfRow);
}


/* %%Function:ElStartOfRow %%Owner:bradch */
EL ElStartOfRow(fExtend)
BOOL fExtend;
{
	return FElTblSel(fExtend, tsdStartOfRow);
}


/* %%Function:ElEndOfCol %%Owner:bradch */
EL ElEndOfCol(fExtend)
BOOL fExtend;
{
	return FElTblSel(fExtend, tsdEndOfCol);
}


/* %%Function:ElStartOfCol %%Owner:bradch */
EL ElStartOfCol(fExtend)
BOOL fExtend;
{
	return FElTblSel(fExtend, tsdStartOfCol);
}


/* C M D  S E L E C T  T A B L E */
/* Selects whole table */
/* %%Function:CmdSelectTable %%Owner:rosiep */
CMD CmdSelectTable(pcmb)
CMB *pcmb;
{
	struct SEL *psel = PselActive();

	if (FInTableDocCp(psel->doc, psel->cpFirst))
		{
		CacheTable(psel->doc, psel->cpFirst);
		SelectRow(psel, caTable.cpFirst, caTable.cpLim);
		psel->sty = styWholeTable;
		if (vhwndCBT)
			CBTTblSelectPsel(psel);
		vfLastCursor = fFalse;
		}
	else
		Beep();

	return cmdOK;
}


