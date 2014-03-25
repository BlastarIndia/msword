/* C O M M A N D . C */
/*  Menu display code  */


#define OEMRESOURCE /* So we get OBM_CLOSE from qwindows.h */
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "debug.h"
#include "menu2.h"
#include "disp.h"
#include "doc.h"
#define NOEXTERN
#include "cmdtbl.h"
#include "ch.h"
#include "field.h"
#include "print.h"
#include "props.h"
#include "sel.h"
#include "keys.h"
#include "recorder.h"
#include "screen.h"
#include "status.h"
#include "el.h"
#include "sdmtmpl.h"
#include "layout.h"
#include "macrocmd.h"
#define RULER
#define REPEAT
#include "ruler.h"
#include "fontwin.h"
#include "prm.h"
#include "rareflag.h"

#define VERBFIRST

#define USEBCM
#include "cmd.h"


/* G L O B A L S */
struct SAB      vsab;


/* E X T E R N A L S */
extern BOOL vfExtendSel;
extern BOOL vfBlockSel;
extern BOOL vfRecording;
extern BOOL vfElFunc;
extern int vcElParams;
extern HPSYT vhpsyt;
extern HWND vhwndStatLine;
extern int docGlobalDot;
extern BOOL vfInsertMode;
extern struct AAB vaab;
extern struct WWD ** hwwdCur;
extern int vlm;
extern RRF rrf;
extern int viobj;
extern BOOL vfRecordNext;
extern BOOL vfSingleApp;
extern BOOL vfFileCacheDirty;
extern BOOL vfWndCacheDirty;
extern CHAR szNone[];
extern int  vssc;

BCM BcmOfSt();
NATIVE uns WHash(); /* DECLARATION ONLY */
ELG ElgFromDoc();
HPSY HpsyFromBcmElg();
PFN PfnFromBsy();
CMD CmdExecBcmKc();
CMD CmdExecCmb();

extern struct CA        caPara;
extern struct STTB      **hsttbMenu;
extern struct SCI       vsci;
extern KMP              **hkmpCur;
extern int              mwCur;
extern HWND vhwndRibbon;
extern HWND vhwndStatLine;
extern struct STTB ** vhsttbOpen;
extern struct STTB ** vhsttbWnd;
extern struct PRI vpri;
extern struct MERR vmerr;
extern int cfLink;
extern int cfRTF;
extern struct SAB vsab;
extern struct UAB vuab;
extern struct PREF vpref;
extern HANDLE vhMenuLongFull;
extern HMENU vhMenu;
extern int viMenu;
extern HWND vhwndApp;
extern struct MWD ** mpmwhmwd [];
extern struct WWD ** mpwwhwwd [];
extern int    wwCur;
extern BOOL vfRecording;
extern struct MWD ** hmwdCur;
extern MES ** vhmes;
extern KME vkme;
extern struct PAP vpapFetch;
extern struct SEL selCur;

int vgrfMenuCmdsAreDirty = 0xffff;
int vgrfMenuKeysAreDirty = 0xffff;
int docDotMudCur = docNil;
int vkcPrev = kcNil;
int vfSysMenu = fFalse;
BOOL vfEmptyMenu = fFalse;

BCM vbcmFetch = bcmNil;



KME * PkmeFromKc();
KME * PkmeOfKcInChain();

SY * PsyGetSyOfBsy();


HBITMAP hbmpSystem = NULL; /* System menu bitmap */

#define FWInWloWhi(w, wLo, wHi) ((uns) ((w) - (wLo)) <= (wHi) - (wLo))

int mpmruimi[] =
	{
	imiWkOn1, imiWkOn2, imiWkOn3, imiWkOn4
	};


int mpimwimi[] =
	{
	imiWnd1, imiWnd2, imiWnd3, imiWnd4, imiWnd5,
	imiWnd6, imiWnd7, imiWnd8, imiWnd9
	};

KMP ** hkmpSearch;
int ikmeSearch;

CHAR vrgchsyFetch [cbMaxSy];


/* ****
*
	Function:  SetAppMenu
*  Copyright: Microsoft 1985 - 1987
*
*  Description: Initialize dropdown menu
*
** ***/

/* %%Function:SetAppMenu %%Owner:bradch */
SetAppMenu(hmenu, lParam)
HMENU hmenu; /* popup menu handle */
LONG lParam; /* From WM_INITMENUPOPUP: LOWORD=index, HIWORD=fControlMenu */
{
	extern struct MWD ** hmwdCur;

	struct STTB ** hsttb;
	BCM bcm, * pbcm, bcmNew;
	int ibst;
	int ibstMac;
	int iItem;
	int iItemMac;
	int iItemChange;
	BOOL fRedoKeys;
	int wk;
	int imnu;
	int w;

	Assert(vfSysMenu == 0 || vfSysMenu == 1);

	vfEmptyMenu = fFalse;

	if (HIWORD(lParam) || (LOWORD(lParam) == 0 && vfSysMenu))
		imnu = imnuDocControl;
	else
		imnu = LOWORD(lParam) - vfSysMenu;

	Assert(vhMenu != NULL && hmenu != NULL);

	if (viMenu == iMenuLongFull && imnu < imnuDocControl && 
			(vgrfMenuCmdsAreDirty & (1 << imnu)))
		FChangeAppMenu(hmenu, imnu);

	/* if the cache separator was removed from the menu but the menu
	now has a cache entry, append the separator back */
	if (
			/* This is the window menu... */
	((imnu == (imnuWindow - (viMenu == iMenuShortFull)) || 

			/* Or it's the file menu and there are files to show... */
	(imnu == imnuFile && (*vhsttbOpen)->ibstMac > 0)) && 

			/* And there is no cache separator (it might have been removed
		from the file menu if there are no files, or from either
				menu if all other items are deleted)... */
	((w = GetMenuState(hmenu, imnu == imnuFile ? bcmFileCache : 
			bcmWndCache, MF_BYCOMMAND)) == 0 || w == -1)) && 

			/* And there is no cache (there might be a cache without a
				separator if all other items are removed from the menu)... */
	((w = GetMenuState(hmenu, imnu == imnuFile ? imiWkOn1 : 
			imiWnd1, MF_BYCOMMAND)) == -1))
		{
		/* if this fails then there won't be a cache, not fatal. */
		if (ChangeMenu(hmenu, 0, NULL, 
				imnu == imnuFile ? bcmFileCache : bcmWndCache, 
				MF_APPEND | MF_SEPARATOR | MF_DISABLED) == NULL)
			{
			SetErrorMat(matMenu);
			}
		else
			{
			if (imnu == imnuFile)
				vfFileCacheDirty = fTrue;
			else
				vfWndCacheDirty = fTrue;
			}
		}


	iItemMac = GetMenuItemCount(hmenu);

	if (iItemMac == 0)
		vfEmptyMenu = fTrue;

	fRedoKeys = vgrfMenuKeysAreDirty & (1 << imnu);
	hsttb = hNil;
	ibst = 0;

	for (iItem = 0; iItem < iItemMac; iItem++)
		{
		BOOL f, fGrayCache, fRedoAKey = fFalse;

		bcmNew = bcm = GetMenuItemId(hmenu, iItem);

		if ((GetMenuState(hmenu, iItem, MF_BYPOSITION) & MF_DISABLED)
				&& bcm != bcmFileCache && bcm != bcmWndCache)
			{
			continue;
			}

		switch (bcm)
			{
		case imiUndo:
		case bcmRepeat:
			fRedoAKey = fTrue;
			break;

		case bcmTableToText:
		case imiInsTable:
			if ((bcmNew = FWholeRowsPsel(PselActive()) ?
					bcmTableToText : imiInsTable) != bcm)
				fRedoAKey = fTrue;
			break;

		case imiRecorder:
			fRedoAKey = fTrue;
			break;

		case imiPaste:
			fRedoAKey = fTrue;
			break;

		case bcmFileCache:
			if (vfFileCacheDirty || fRedoKeys)
				{
				hsttb = vhsttbOpen;
				pbcm = mpmruimi;
				vfFileCacheDirty = fFalse;
				ibstMac = ibstMaxFileCache;
				goto LBuildCache;
				}
			continue;

		case bcmWndCache:
			if (vfWndCacheDirty || fRedoKeys)
				{
				hsttb = vhsttbWnd;
				pbcm = mpimwimi;
				vfWndCacheDirty = fFalse;
				ibstMac = ibstMaxWndCache;
				goto LBuildCache;
				}
			continue;
LBuildCache:
			/* Build a cache from an sttb... */
			if ((ibstMac = min(ibstMac, (*hsttb)->ibstMac)) == 0)
				{
				/* Remove the menu separator! */
				/* remove--don't care too much if it fails! 
				(which seems unlikely) */
				ChangeMenu(hmenu, iItem, (LPSTR) NULL,
						NULL, MF_BYPOSITION | MF_DELETE);
				continue;
				}

			if (iItem == 0)
				{
				/* Remove the menu separator 'cause poor
					Windows cannot handle a separator as
					the first item on a menu (it hangs)! */
				/* remove--don't care too much if it fails! 
				(which seems unlikely) */
				ChangeMenu(hmenu, iItem, (LPSTR) NULL,
						NULL, MF_BYPOSITION | MF_DELETE);
				iItemMac -= 1;
				}
			else
				iItem++; /* skip separator */

			fGrayCache = !FLegalBcm(bcm);

#define cchBufCache (cchMaxSzMenu>cchMaxFile?cchMaxSzMenu:cchMaxFile)

			/* add each of the file names to the menu */
			for (ibst = 0; ibst < ibstMac; ++ibst)
				{
				CHAR * st, * pch, szBuf [cchBufCache+4];
				int cch;
				int mf = MF_BYPOSITION | MF_STRING;

				pch = szBuf;

				/* start with underlined number and space */
				*pch++ = chMenuAcc;
				CchIntToPpch(ibst + 1, &pch);
				*pch++ = chSpace;

				if (hsttb == vhsttbWnd)
					/* window menu strings */
					{
					CHAR stT[ichMaxFile+5];
					GetStFromSttb(hsttb, ibst, stT);
					if (stT[*stT] == mwCur)
						mf |= MF_CHECKED;
					cch = --stT[0]; /* delete mw */
					StToSz(stT, pch);
					}

				else
					{
					/* file menu strings */
					GetSzFromSttb(hsttb, ibst, pch);
					cch = CchSzFileAbsToSzFileRel(pch, pch);
					}

					/* BLOCK */
					{
					CHAR szClean[cchMaxSzMenu];
					SanitizeSz(pch, szClean, cchMaxSzMenu, 
							fFalse);
					CchCopySz(szClean, pch);
					}
				pch += cch;

				if (fGrayCache)
					mf |= MF_GRAYED;

				if (ChangeMenu(hmenu, iItem, (LPSTR) szBuf,
						*pbcm++, mf |
						(iItem<iItemMac ? MF_CHANGE:MF_APPEND))
						== NULL)
					/* means we don't have complete list (big
					deal) */
					{
					SetErrorMat(matMenu);
					break;
					}
				iItem++;
				}

			iItemChange = iItem;
			while (iItem++ < iItemMac)
				{
				/* remove--don't care too much if it fails! 
				(which seems unlikely) */
				ChangeMenu(hmenu, iItemChange, (LPSTR) NULL,
						NULL, MF_DELETE | MF_BYPOSITION);
				}
			continue;
			}  /* end switch */

		if (fRedoKeys || fRedoAKey)
			SetBcmMenuKeys(hmenu, bcm, bcmNew);

		/* Deal with enabling/disabling the item */
		EnableMenuItem(hmenu, iItem, MF_BYPOSITION |
				(FLegalBcm(bcmNew) ? MF_ENABLED : MF_GRAYED));

		f = fFalse;
/* Check for Checked/unchecked */

		if (FToggleBcm(bcm))
			{
			CheckMenuItem(hmenu, iItem, MF_BYPOSITION |
					(FStatToggleBcm(bcm) ? 
					MF_CHECKED : MF_UNCHECKED));
			}
		}  /* end for */

	vgrfMenuKeysAreDirty &= ~(1 << imnu);

	if (vmerr.mat == matMenu)
		/* report problems encountered dropping menu */
		FlushPendingAlerts();
}  /* SetAppMenu */


csconst CHAR szCsStopRecorder[] = SzKey("Stop Re&corder",StopRecorder);
csconst CHAR szCsRecordMacro[] = SzKey("Re&cord Macro...",RecordMacro);
csconst CHAR szCsRecord[] = SzKey("Re&cord...",Record);
csconst CHAR szCsPasteCells[] = SzKey("&Paste Cells",PasteCells);
csconst CHAR szCsPaste[] = SzKey("&Paste",Paste);
csconst CHAR szCsTableToText[] = SzKey("&Table to Text...",TableToText);
csconst CHAR szCsTable[] = SzKey("&Table...", Table);

/* S E T  B C M  M E N U  K E Y S */
/* %%Function:SetBcmMenuKeys %%Owner:bradch */
SetBcmMenuKeys(hmenu, bcm, bcmNew)
HMENU hmenu;
BCM bcm, bcmNew;
{
	int kc;
	BOOL fHasKey = fTrue;
	CHAR * pch;
	KME * pkme;
	CHAR szMtm [cchMaxSzMenu];
	CHAR szKey [cchMaxSzKey];

	Assert(hmenu != NULL);

	switch (bcmNew)
		{
	case imiUndo:
	case bcmRepeat:
		FmtUndoAgainSz(bcmNew == bcmUndo, szMtm);
		break;

	case imiRecorder:
		if (vfRecording || vrf.fPauseRecorder)
			CopyCsSz(szCsStopRecorder, szMtm);
		else  if (viMenu == iMenuLongMin)
			CopyCsSz(szCsRecordMacro, szMtm);
		else
			CopyCsSz(szCsRecord, szMtm);
		break;

	case imiPaste:
		if (vsab.fTable)
			CopyCsSz(szCsPasteCells, szMtm);
		else
			CopyCsSz(szCsPaste, szMtm);
		break;

	case bcmTableToText:
	case imiInsTable:
		if (bcm != bcmNew)
			{
			if (bcmNew == imiInsTable)
				CopyCsSz(szCsTable, szMtm);
			else
				CopyCsSz(szCsTableToText, szMtm);
			break;
			}

		/* else fall through */

	default:
		Assert(bcm == bcmNew); /* this must be true or bcmNew is not a
		legal thing to pass to GetMenuString (it's not on the menu yet).
		Any bcm that is treated differently should have a case above. */
		GetMenuString(hmenu, bcmNew, (LPSTR) szMtm, sizeof (szMtm), MF_BYCOMMAND);
		fHasKey = fFalse;
		break;
		}

	for (pch = szMtm; *pch != '\0'; ++pch)
		if (*pch == '\010')
			{
			fHasKey = fTrue;
			break;
			}
	szKey[0] = '\0';

	if ((kc = KcFirstOfBcm(hkmpCur, bcmNew)) != kcNil && FKcToSz(kc, szKey))
		{
		*pch++ = '\010';
		*pch++ = ' ';
		*pch++ = ' ';
		if (fHasKey && FEqNcSz(szKey, pch))
			return;

		/* copy keyname to buffer */
		CchCopySz(szKey, pch);
		}
	else
		{
		if (!fHasKey)
			return;

		/* kill old key name */
		*pch = '\0';
		}

	if (ChangeMenu(hmenu, bcm, (LPSTR) szMtm, bcmNew, MF_CHANGE|MF_STRING)
			== NULL)
		/* just means key assingment is missing (or old undo/repeat text) */
		SetErrorMat(matMenu);
}




/* C C H  S Z  F I L E  A B S  T O  S Z  F I L E  R E L */
/* %%Function:CchSzFileAbsToSzFileRel %%Owner:bradch */
int CchSzFileAbsToSzFileRel(szFileAbs, szFileRel)
CHAR * szFileRel;
CHAR * szFileAbs;
{
	extern CHAR stDOSPath [];
#define cchDrive 2
	int cch;
	int cchFileAbs;
	int cchPath;

	Assert(stDOSPath[0] >= 3); /*???*/

	cchPath = stDOSPath[0];
	cchFileAbs = CchSz(szFileAbs) - 1;

	if (szFileAbs[0] != stDOSPath[1] || szFileAbs[1] != ':')
		{
		/* not the same drive; copy whole thing */
		if (szFileAbs != szFileRel)
			bltbyte(szFileAbs, szFileRel, cchFileAbs + 1);
		cch = cchFileAbs;
		}
	else  if (cchPath < cchFileAbs &&
			!FNeRgch(szFileAbs, &stDOSPath[1], cchPath))
		{
		/* the same path; strip it */
		bltbyte(szFileAbs + cchPath, szFileRel,
				(cch = cchFileAbs - cchPath) + 1);
		}
	else
		{
		/* same drive, different path; strip drive */
		bltbyte(szFileAbs + cchDrive, szFileRel,
				(cch = cchFileAbs - cchDrive) + 1);
		}

	return cch;
}



/* following array is an array of pairs of words of one of two forms

	(1) :  { bcm, ist },
			where the bcm's undo string is contained in rgstUndo [ist] 
	(2) :  { bcm1, -bcm2 }
			where bcm1's undo string is the command name for bcm2
*/

#define istUndoTyping        0
#define istUndoFormatting    1
#define istUndoDelete        2
#define istUndoChangeBkmk    3
#define istUndoInsertion     4
#define istUndoMove          5
#define istUndoRevMark       6
#define istUndoEdit          7
#define istUndoCopyTF        8
#define istUndoExpGls        9
#define istUndo              10
#define istUndoPgBreak       11
#define istUndoReplace       12
#define istUndoEditTable     13
#define istUndoOtlMouse      14
#define istUndoFormatTable   15
#define istUndoInsertTable   16
#define istUndoSplitTable    17

csconst int rgwBcmXlate [] = {
	bcmTyping,		istUndoTyping,
			bcmFormatting,		istUndoFormatting,
			bcmRRFormat,		istUndoFormatting,
			bcmDeleteSel,		istUndoDelete,
			bcmCutBlock,		-bcmCut,
			bcmPasteBlock,		-bcmPaste,
			bcmChangeBkmk,		istUndoChangeBkmk,
			bcmInsertSect,		istUndoInsertion,
			bcmMove,		istUndoMove,
			bcmInsertFld,		istUndoInsertion,
			bcmTypingAgain,		istUndoTyping,
			bcmMoveUpDown,		istUndoMove,
			imiInsBreak,		istUndoInsertion,
			bcmHdrLinkPrev,		istUndoEdit,
			bcmMoveSp,		istUndoMove,
			bcmCopySp,		istUndoCopyTF,
			bcmRevMark,             istUndoRevMark,
			bcmInsRows,             istUndoEditTable, 
			bcmInsColumns,          istUndoEditTable, 
			bcmInsCellsHoriz,       istUndoEditTable,
			bcmInsCellsVert,        istUndoEditTable,
			bcmDeleteRows,          istUndoEditTable,
			bcmDeleteColumns,       istUndoEditTable,
			bcmDeleteCellsHoriz,    istUndoEditTable,
			bcmDeleteCellsVert,     istUndoEditTable,
			bcmSplitCells,          istUndoEditTable,
			bcmMergeCells,          istUndoEditTable,
			bcmOtlValMouse,		istUndoOtlMouse,
			bcmOtlLvlMouse,		istUndoOtlMouse,
			bcmFormatTable,         istUndoFormatTable,
			imiInsTable,            istUndoInsertTable,
			bcmSplitTable,          istUndoSplitTable
};


csconst CHAR rgstUndo [] [] =
{
	StSharedKey("Typing",UndoTypingDef),
			StSharedKey("Formatting",UndoFormattingDef),
			StSharedKey("Delete",UndoDeleteDef),
			StSharedKey("Change Bookmark",UndoChangeBkmkDef),
			StSharedKey("Insertion",UndoInsertionDef),
			StSharedKey("Move",UndoMoveDef),
			StSharedKey("Revision",UndoRevMarkDef),

			StSharedKey("Edit",UndoEditDef),
			StSharedKey("CopyToFrom",UndoCopyTFDef),
			StSharedKey("Expand Glossary",UndoExpGlsDef),
			StSharedKey("Define Glossary",UndoDefineGlsDef),
			StSharedKey("Page Break",UndoPgBreakDef),
			StSharedKey("Replace",UndoReplaceDef),
			StSharedKey("Edit Table",UndoEditTableDef),
			StSharedKey("Promote/Demote",UndoProDemoteDef),
			StSharedKey("Format Table",UndoFormatTableDef),
			StSharedKey("Insert Table",UndoInsertTableDef),
			StSharedKey("Split Table",UndoSplitTableDef)
};




/**********************************/
/* F m t  U n d o  A g a i n  S z */

/* %%Function:FmtUndoAgainSz %%Owner:bradch */
FmtUndoAgainSz(fUndo, sz)
BOOL fUndo;
CHAR sz[];
{
	CHAR * pch;
	CHAR FAR * lpch;
	int cch, cchT, i;
	int bcm = fUndo ? vuab.bcm : vaab.bcm;

	pch = &sz[0];
	/* bcmNil check is safety net. "should not happen" bz */
	if (fUndo && (vuab.uac == uacNil || bcm == bcmNil))
		pch += CchCopySz( SzFrameKey( "Can't &Undo", CantUndo ), pch );
	else  if (!fUndo && bcm == bcmNil)
		pch += CchCopySz( SzFrameKey( "Can't &Repeat",CantRepeat ), pch );
	else
		{
#ifdef VERBFIRST
		pch += CchCopySz( fUndo ? SzFrameKey( "&Undo ",UndoSp ):
				SzFrameKey( "&Repeat ", RepeatSp ),
				pch );
#endif
		if (fUndo && vuab.fRedo)
			bcm = bcmUndo;
		else
			{
/* check for bcms that need undo strings other than their command names */
			for ( i = 0 ; i < sizeof (rgwBcmXlate)/sizeof(int); i += 2 )
				{
				int ist;

				if (bcm == rgwBcmXlate [i])
					if ((ist = rgwBcmXlate [i+1]) >= 0)
						{	/* Get string from array */
						lpch = (CHAR FAR *)&rgstUndo [rgwBcmXlate [i+1]];
						cchT = (int) (*lpch++);
						bltbx(lpch, (CHAR FAR *) pch, cchT);
						goto LGotSz;
						}
					else
						{
						bcm = -ist;
						break;
						}
				}
			}
		Assert( (int)bcm >= 0 );

		cchT = CchBuildMenuSz(bcm, pch,
				grfBldMenuNoAcc | grfBldMenuNoElip);

		/* Remove the menu name: assumes menu string is
			in format "MenuCommand" (note caps) */
		if ((uns) (bcm - bcmFirstMenu) <=
				(bcmLastMenu - bcmFirstMenu))
			{
			unsigned ch;
			CHAR * pchT;

			pchT = pch + 1;
			while ((ch = *pchT) == chMenuAcc || FLower(ch))
				{
				Assert(ch != '\0');
				++pchT;
				}
			Assert(pchT > pch);
			/* ++ to skip a separating blank. */
			cchT -= ++pchT - pch;
			bltb(pchT, pch, cchT);
			}
LGotSz:
		Assert(cchT > 0);
		pch += cchT;

#ifndef VERBFIRST
		pch += CchCopySz( fUndo ? SzFrameKey( " &Undo",SpUndo ):
				SzFrameKey( " &Repeat", SpRepeat ),
				pch );
#endif
		}
	*pch = '\0';
}



/* S A N I T I Z E  S Z */
/*  Double menu/dialog accelerators and (if fReplNP) replace non-printing
	characters with "."
*/

/* %%Function:SanitizeSz %%Owner:bradch */
SanitizeSz(sz, szClean, cchMax, fReplNP)
CHAR *sz, *szClean;
int cchMax;
BOOL fReplNP;
{
	unsigned ch;
	CHAR *pchSrc, *pchDest,  *pchMax;

	pchSrc = sz;
	pchDest = szClean;
	pchMax = pchDest + cchMax - 1;

	while (pchDest < pchMax && (ch = *pchDest = *pchSrc++) != '\0')
		{
		/* top bit masked because non-printing ranges are parallel for high
			& low 128 */
		if (fReplNP && (ch &= 0x7f) < 0x20)
			*pchDest = '.';
		pchDest++;

		if (ch == chMenuAcc)
			{
			*pchDest++ = chMenuAcc;
			}
		}

	*pchDest++ = 0;
}


/* I B S T  M E N U  B S Y  S T */
/* %%Function:IbstMenuBsySt %%Owner:bradch */
IbstMenuBsySt(bsy, st)
int bsy;
char * st;
{
	int ibst;
	char stSugg[cchMaxSzMenu];

	CchBuildMenuSz(bsy, stSugg, grfBldMenuDef);
	SzToStInPlace(stSugg);
	if (FStSame(stSugg, st))
		return 0;

	if ((ibst = IbstFindSt(hsttbMenu, st)) == -1)
		ibst = IbstAddStToSttb(hsttbMenu, st);

	return ibst; /* note: may return ibstNil */
}


/* These strings represent different types of file separators.  They are 
	used in the Assign to Menu dialog's command list. */

csconst CHAR csszDashes [] =     SzKey("--------------------",Dashes);
csconst CHAR csszFileDashes [] = SzKey("---- File List -----",FileList);
csconst CHAR csszWindDashes [] = SzKey("--- Window List ----",WindowList);
#define ichMacDashes 20


/* C C H  B U I L D  M E N U  S Z */
/* %%Function:CchBuildMenuSz %%Owner:bradch */
int CchBuildMenuSz(bsy, sz, grf)
uns bsy;
CHAR * sz;
int grf;
{
	extern int vbsySepCur;
	SY * psy;
	CHAR * pch;
	CHAR *pchStart;
	int cch;
	CHAR rgchSy[cbMaxSy];

	pchStart = sz;

	if (bsy == bcmFileCache)
		{
		CopyCsSz(csszFileDashes, sz);
		return ichMacDashes;
		}
	else  if (bsy == bcmWndCache)
		{
		CopyCsSz(csszWindDashes, sz);
		return ichMacDashes;
		}
	else  if ((int) bsy < 0)
		{
		/* separator */
		CopyCsSz(csszDashes, sz);
		return ichMacDashes;
		}

	psy = PsyGetSyOfBsy(rgchSy, bsy);
	pch = psy->stName;
	cch = *pch++;
	if (!(grf & grfBldMenuNoAcc))
		{
		*sz++ = chMenuAcc;
		}
	*sz++ = ChUpper(*pch++);
	while (--cch > 0)
		{
		if (FUpper(*pch)/* && *pch > ' '*/)
			{
			*sz++ = ' ';
			}
		*sz++ = *pch++;
		}

	if (!(grf & grfBldMenuNoElip) && psy->mct == mctSdm)
		{
		sz = bltb(SzSharedKey("...",Elipses), sz, 3);
		}

	*sz = '\0';
	return (sz - pchStart);
}


typedef struct
{
	int kc;
	CHAR st [];
} KCD;

/* the constant ikcdMax denotes the no of elements in the array vrgkcd.
	ikcdMax has to be changed on changing vrgkcd. */

csconst KCD vrgkcd [] =
{
			VK_CANCEL,		 StSharedKey("ScLk",ScLk),
			kcBackSpace,    StSharedKey("BkSp",BkSp),
			kcTab,          StSharedKey("Tab",Tab),
			kcReturn,       StSharedKey("Enter",Enter),
			kcEscape,       StSharedKey("Esc",Esc),
			kcInsert,       StSharedKey("Ins",Ins),
			kcDelete,       StSharedKey("Del",Del),
			kcSpace,        StSharedKey("Space",Space),
			kcHelp,         StSharedKey("Help",Help),
			VK_NUMPAD0,		StSharedKey("NumPad 0",Numpad0),
			VK_NUMPAD1,		StSharedKey("NumPad 1",Numpad1),
			VK_NUMPAD2,		StSharedKey("NumPad 2",Numpad2),
			VK_NUMPAD3,		StSharedKey("NumPad 3",Numpad3),
			VK_NUMPAD4,		StSharedKey("NumPad 4",Numpad4),
			VK_NUMPAD5,		StSharedKey("NumPad 5",Numpad5),
			VK_NUMPAD6,		StSharedKey("NumPad 6",Numpad6),
			VK_NUMPAD7,		StSharedKey("NumPad 7",Numpad7),
			VK_NUMPAD8,		StSharedKey("NumPad 8",Numpad8),
			VK_NUMPAD9,		StSharedKey("NumPad 9",Numpad9),
			kcClear,    	StSharedKey("NumPad 5",Numpad5NoNL),
			kcNPPlus,		StSharedKey("NumPad +",NumpadPlus),
			kcNPMinus,		StSharedKey("NumPad -",NumpadMinus),
			kcNPMult,		StSharedKey("NumPad *",NumpadMult),
			kcNPDiv,			StSharedKey("NumPad /",NumpadDiv),
			VK_DECIMAL,		StSharedKey("NumPad .",NumpadDecimal),
			kcUp,				StSharedKey("Up",CommandUp),
			kcDown,			StSharedKey("Down",CommandDown),
			kcLeft,			StSharedKey("Left",CommandLeft),
			kcRight,			StSharedKey("Right",CommandRight),
			kcBeginLine,	StSharedKey("Home",Home),
			kcEndLine,		StSharedKey("End",End),
			kcPageUp,		StSharedKey("PgUp",PgUp),
			kcPageDown,		StSharedKey("PgDn",PgDn),
};


#define ikcdMax 33


csconst CHAR csszAltPlus [] = SzKey("Alt+", AltPlus);
csconst CHAR csszCtrlPlus [] = SzKey("Ctrl+", CtrlPlus);
csconst CHAR csszShiftPlus [] = SzKey("Shift+", ShiftPlus);


/* F  K C  T O  S Z */
/* %%Function:FKcToSz %%Owner:bradch */
BOOL FKcToSz(kc, sz)
int kc;
CHAR * sz;
{
	CHAR * pch, * st;
	int ikcd, cch;

	pch = sz;

	if (FAltKc(kc))
		{
		CopyCsSz(csszAltPlus, pch);
		pch += sizeof (csszAltPlus) - 1;
		}

	if (FCtrlKc(kc))
		{
		CopyCsSz(csszCtrlPlus, pch);
		pch += sizeof (csszCtrlPlus) - 1;
		}

	if (FShiftKc(kc))
		{
		CopyCsSz(csszShiftPlus, pch);
		pch += sizeof (csszShiftPlus) - 1;
		}

	kc &= 0xff;

	/* Deal with alpha-numeric keys */
	if (FWInWloWhi(kc, '0', 'Z'))
/*        if (kc >= '0' && kc <= 'Z')*/
		{
		*pch++ = kc;
		goto LTermSz;
		}

	/* Deal with function keys */
	Assert( (kcF1 & 0xFF00) == (kcF16 & 0xFF00) );
	Assert( (kcF16 & 0xff) > (kcF1 & 0xff ) );

	if (FWInWloWhi( kc, kcF1 & 0xff, kcF16 & 0xff))
		{
		kc -= (kcF1 & 0xff) - 1;
		*pch++ = 'F';
		if (kc >= 10)
			{
			*pch++ = '1';
			kc -= 10;
			}
		*pch++ = '0' + kc;
		goto LTermSz;
		}

#ifndef INTL
	if (kc == kcPlus || kc == kcMinus)
		{
		*pch++ = (kc == kcPlus) ? '+' : '-';
		goto LTermSz;
		}
#else /* INTL */
	if (kc == (vkPlus & 0xff))
		{
		*pch++ = '+';
		goto LTermSz;
		}
	if (kc == (vkMinus & 0xff))
		{
		*pch++ = '-';
		goto LTermSz;
		}
	if (kc == (vkStar & 0xff))
		{
		*pch++ = '*';
		goto LTermSz;
		}
	if (kc == (vkUnderline & 0xff))
		{
		*pch++ = '_';
		goto LTermSz;
		}
	if (kc == (vkEquals & 0xff))
		{
		*pch++ = '=';
		goto LTermSz;
		}
	if (kc == (vkQuestionmark & 0xff))
		{
		*pch++ = '?';
		goto LTermSz;
		}
#endif /* INTL */

	/* Look for key in table of key names */
	for (ikcd = 0; ikcd < ikcdMax; ++ikcd)
		if (kc == (vrgkcd[ikcd].kc & 0xff))
			{
			CHAR FAR * lpst;

			lpst = vrgkcd[ikcd].st;
			bltbx(lpst + 1, (CHAR FAR *) pch, cch = *lpst);
			pch += cch;
			goto LTermSz;
			}

	/* We don't have a name for the key */
	cch = CchIntToPpch(kc, &pch);

	Assert(cch <= 3);
	*pch = '\0';

	return fFalse;		/* to indicate numeric value and no text for kcBase */

LTermSz:
	*pch = '\0';

	return fTrue;
}




/* K C  F I R S T  O F  B C M */
/* %%Function:KcFirstOfBcm %%Owner:bradch */
KcFirstOfBcm(hkmpStart, bcm)
KMP ** hkmpStart;
int bcm;
{
	KME * pkme;
	KMP * pkmp;

		{{ /* NATIVE - loop in KcFirstOfBcm */
		for (hkmpSearch = hkmpStart; hkmpSearch != hNil;
				hkmpSearch = pkmp->hkmpNext)
			{
			pkmp = *hkmpSearch;
			pkme = &pkmp->rgkme[0];
			for (ikmeSearch = 0; ikmeSearch++ < pkmp->ikmeMac; ++pkme)
				{
				if (pkme->kt == ktMacro && pkme->bcm == bcm)
					{{ /* !NATIVE - KcFirstOfBcm */
					KME * pkmeT;

				/* Make sure key is not being overridden
					by a previous keymap. */
					pkmeT = PkmeOfKcInChain(pkme->kc);

					if (pkmeT == 0 ||
							pkmeT->kt == ktMacro && pkmeT->bcm == bcm)
						{
						return pkme->kc;
						}
					}}
				}
			if (pkmp->fStopHere)
				break;
			}
		}}

	return kcNil;
}


/* F  T O G G L E  B C M */
/* Returns fTrue iff the command is a toggle. */
/* %%Function:FToggleBcm %%Owner:bradch */
FToggleBcm(bcm)
BCM bcm;
{
	switch (bcm)
		{
	case bcmBold:
	case bcmItalic:
	case bcmSmallCaps:
	case bcmHideText:
	case bcmULine:
	case bcmDULine:
	case bcmWULine:
	case bcmSuperscript:
	case bcmSubscript:
	case imiOutline:
	case imiDraftView:
	case bcmPageView:
	case bcmRibbon:
	case imiRuler:
	case bcmStatusArea:
	case bcmViewFootnote:
	case bcmViewAnnotation:
	case imiFieldCodes:
	case bcmPrintPreview:
	case bcmShowAll:
	case bcmOverType:
		return fTrue;
		}

	return fFalse;
}


/* F  S T A T  T O G G L E  B C M */
/* Returns the status of a toggling command.  (Should return fTrue iff
	the command should be checked on a menu.) */
/* %%Function:FStatToggleBcm %%Owner:bradch */
FStatToggleBcm(bcm)
BCM bcm;
{
	extern BOOL vfOvertypeMode;
	extern struct PREF vpref;
	extern struct WWD ** hwwdCur;
	extern HWND vhwndRibbon;
	extern HWND vhwndStatLine;
	int sprm, valCmp;

	valCmp = 1;

	switch (bcm)
		{
	case bcmOverType:
		return vfOvertypeMode;

	case bcmBold:
		sprm = sprmCFBold;
		goto LProp;

	case bcmItalic:
		sprm = sprmCFItalic;
		goto LProp;

	case bcmSmallCaps:
		sprm = sprmCFSmallCaps;
		goto LProp;

	case bcmHideText:
		sprm = sprmCFVanish;
		goto LProp;

	case bcmULine:
		sprm = sprmCKul;
		valCmp = kulSingle;
		goto LProp;

	case bcmDULine:
		sprm = sprmCKul;
		valCmp = kulDouble;
		goto LProp;

	case bcmWULine:
		sprm = sprmCKul;
		valCmp = kulWord;
		goto LProp;

	case bcmSuperscript:
		sprm = sprmCFBold;
		valCmp = 6;
		goto LProp;

	case bcmSubscript:
		sprm = sprmCFBold;
		valCmp = -6;

LProp:
		GetSelCurChp(fFalse);
		return ValFromPropSprm(&selCur.chp, sprm) == valCmp;

	case imiOutline:
		return (*hwwdCur)->fOutline;

	case imiDraftView:
		return vpref.fDraftView;

	case bcmPageView:
		return (*hwwdCur)->fPageView;

	case bcmRibbon:
		return vhwndRibbon != NULL;

	case imiRuler:
		return hwwdCur == hNil ? vpref.fRuler : 
				((*hmwdCur)->hwndRuler != hNil);

	case imiStatusArea:
		return vhwndStatLine != NULL;

	case bcmViewFootnote:
	case bcmViewAnnotation:
			{
			int ww;

			return (ww = (*hmwdCur)->wwLower) != wwNil && 
					PwwdWw(ww)->wk == 
					(bcm == bcmViewFootnote ? wkFtn : wkAtn);
			}

	case imiFieldCodes:
		return !FFromIGrpf(fltgOther, 
				(*hwwdCur)->grpfvisi.grpfShowResults);

	case bcmPrintPreview:
		return vlm == lmPreview;

	case bcmShowAll:
		return vpref.grpfvisi.fvisiShowAll;
		}

	Assert(fFalse);
}



/* F  T H R O W  M U D  A T  M E N U */
/*  Apply hmud to hmenu for dropdown imnu */
/* %%Function:FThrowMudAtMenu %%Owner:bradch */
FThrowMudAtMenu(hmud, hmenu, imnu)
MUD **hmud;
HANDLE hmenu;
int imnu;
{
	MUD * pmud;
	MTM * pmtm;
	int imtm, imtmMac, imnuMtm, mf, cItems, iItemAdd;
	int bcmCache;
	char szBuf [cchMaxSzMenu];
	BOOL fReturn = fTrue;

	if (imnu == imnuFile)
		{
		bcmCache = bcmFileCache;
		}
	else  if (imnu == (imnuWindow - (viMenu == iMenuShortFull)))
		{
		bcmCache = bcmWndCache;
		}
	else
		bcmCache = bsyNil;

	cItems = 0;
	iItemAdd = -1;

	pmud = *hmud;
	imtmMac = pmud->imtmMac;
	for (pmtm = &pmud->rgmtm[imtm = 0]; imtm++ < imtmMac; pmtm++)
		{
		if ((imnuMtm = pmtm->imnu) < imnu || pmtm->bsy == bsyNil)
			continue;
		
		 if (imnuMtm > imnu)
			break;

		if (!pmtm->fRemove && (int) pmtm->bsy > 0)
			FetchCm(pmtm->bsy);
		
		if (pmtm->fRemove || 
				((int) pmtm->bsy > 0 && vpsyFetch->mct == mctNil))
			{
			if (ChangeMenu(hmenu, pmtm->bcm, (LPSTR)NULL, 0, MF_DELETE) && cItems > 0)
				--cItems;
			continue;
			}

		Assert(!pmtm->fRemove);

		mf = (iItemAdd != -1) ? 
				MF_INSERT | MF_BYPOSITION : MF_APPEND;

		if (!FGetMtmSz(pmtm, szBuf))
			{
			/* Due to a bug in Win 2.0, menus should not
				have a separator as the first item. */
			if (cItems == 0)
				continue;

			mf |= (MF_SEPARATOR|MF_DISABLED);
			}

		if (ChangeMenu(hmenu, iItemAdd, (LPSTR) szBuf, 
				pmtm->bcm, mf) == NULL)
			/* indicates something failed */
			fReturn = fFalse;

		if (pmtm->bcm == bcmCache && bcmCache != bsyNil)
			{
			iItemAdd = cItems;
			}

		cItems += 1;
		}

	if (cItems == 0)
		{
		/* WINDOWS BUG: if a menu becomes empty and the user selects
			it and presses Enter, a WM_COMMAND is sent with the
			command of the first (deleted) item anyway. (BAC) */
		ChangeMenu(hmenu, 0, (LPSTR) szBuf, bsyNil, MF_APPEND);
		ChangeMenu(hmenu, 0, (LPSTR) NULL, bsyNil, 
				MF_DELETE | MF_BYPOSITION);
		}

	return fReturn;
}



/* F  N E W  M E R G E  M U D S */
/*  Look for all entries in hmudSrc in hmudDest.  If an entry is not found, add
	it to dest.  If it is found and the fRemove settings are different, remove
	it from dest.  if they are the same, use the Src version. */
/* %%Function:FNewMergeMuds %%Owner:bradch */
FNewMergeMuds(hmudDest, hmudSrc)
MUD ** hmudDest;
MUD ** hmudSrc;
{
	MUD * pmudSrc, * pmudDest;
	MTM * pmtm, * pmtmDest;
	int imtm, imtmDest, imtmMac;

	pmudSrc = *hmudSrc;
	pmudDest = *hmudDest;

	imtmMac = pmudSrc->imtmMac;
	for (pmtm = &pmudSrc->rgmtm[imtm = 0]; imtm < imtmMac; ++imtm, ++pmtm)
		{
		if (!FFindImnuBsy(hmudDest, pmtm->imnu, pmtm->bsy, &imtmDest))
			{
			if (!FSplitRgmtm(hmudDest, imtmDest))/*HEAP MOVEMENT*/
				{
				return fFalse;
				}

			pmudDest = *hmudDest;
			pmudSrc = *hmudSrc;
			pmtm = &pmudSrc->rgmtm[imtm];
			pmudDest->rgmtm[imtmDest] = *pmtm;
			}
		else  if (pmtm->fRemove == pmudDest->rgmtm[imtmDest].fRemove)
			pmudDest->rgmtm[imtmDest] = *pmtm;
		else
			{
			/* Remove it from the dest */
			int imtmMacT = pmudDest->imtmMac--;
			MTM * pmtmT = &pmudDest->rgmtm[imtmDest];
			bltb(pmtmT + 1, pmtmT, (imtmMacT - imtmDest) * cbMTM);
			}
		}
	return fTrue;
}



/* H M U D  I N I T */
/* %%Function:HmudInit %%Owner:bradch */
MUD ** HmudInit(cmtmEstimate)
int cmtmEstimate;
{
	MUD ** hmud, * pmud;

	if ((hmud = HAllocateCw(cwMUD + cmtmEstimate * cwMTM)) != hNil)
		{
		pmud = *hmud;
		pmud->imtmMac = 0;
		pmud->imtmMax = cmtmEstimate;
		}
	return hmud;
}


/* F  C H A N G E  A P P  M E N U */
/*  Make menu imnu correct */
/* %%Function:FChangeAppMenu %%Owner:bradch */
FChangeAppMenu(hmenu, imnu)
HMENU hmenu;
int imnu;
{
	extern MUD ** vhmudBase;
	extern MUD ** vhmudUser;
	MUD ** hmudT = hNil, ** hmudT2 = hNil;
	int docDot;
	int iItemMac;

	Assert(hmenu != NULL);
	Assert(viMenu == iMenuLongFull);

	if ((hmudT = HmudInit(0)) == hNil || (hmudT2 = HmudInit(0)) == hNil)
		{
LFailed:
		SetErrorMat(matMenu);
		if (imnu == imnuFile)
			/* must guarantee Save, Close and Exit */
			{
			if (GetMenuState(hmenu, imiSave, MF_BYCOMMAND) == -1)
				ChangeMenu(hmenu, 0, SzSharedKey("&Save",FileSaveEmerg),
						imiSave, MF_APPEND|MF_STRING);
			if (GetMenuState(hmenu, imiFileClose, MF_BYCOMMAND)==-1)
				ChangeMenu(hmenu, 0, SzSharedKey("&Close",FileCloseEmerg),
						imiFileClose, MF_APPEND|MF_STRING);
			if (GetMenuState(hmenu, imiFileExit, MF_BYCOMMAND)==-1)
				ChangeMenu(hmenu, 0, SzSharedKey("E&xit",FileExitEmerg),
						imiFileExit, MF_APPEND|MF_STRING);
			}
		FreeHmud(hmudT);
		FreeHmud(hmudT2);
		return fFalse;
		}

	if (!FCopyMudToMud(vhmudUser, hmudT) || !FCopyMudToMud(vhmudBase, hmudT2))
		goto LFailed;

	if ((docDot = DocDotMother(selCur.doc)) != docNil &&
			!FNewMergeMuds(hmudT, PdodDoc(docDot)->hmudUser))
		goto LFailed;

	if (!FNewMergeMuds(hmudT2, hmudT))
		goto LFailed;

	/* muds are now set up, remove the old stuff from the menu */
	iItemMac = GetMenuItemCount(hmenu);
	while (iItemMac > 0)
		{
		/* don't care too much if this fails */
		ChangeMenu(hmenu, --iItemMac, (LPSTR) NULL, 0, 
				MF_DELETE | MF_BYPOSITION);
		}

	/* dirty file/window caches as all items deleted */
	if (imnu == (imnuWindow - (viMenu == iMenuShortFull)))
				vfWndCacheDirty = fTrue;
   else if (imnu == imnuFile)
		  		vfFileCacheDirty = fTrue;
 
	if (!FThrowMudAtMenu(hmudT2, hmenu, imnu))
		goto LFailed;

	vgrfMenuCmdsAreDirty &= ~(1 << imnu);
	/* must dirty keys since menus now have all new text */
	vgrfMenuKeysAreDirty |= (1 << imnu);

LDone:
	FreeHmud(hmudT);
	FreeHmud(hmudT2);

	return fTrue;
}


/* F  G E T  M T M  S Z */
/* Get the string of an MTM, returning fTrue iff successfull.  Returns fFalse
if MTM is a separator */
/* %%Function:FGetMtmSz %%Owner:bradch */
FGetMtmSz(pmtm, sz)
MTM * pmtm;
char * sz;
{
	uns ibst;

	if ((int) pmtm->bsy < 0)
		{
		CchCopySz(SzSharedKey("----------------",SomeDashes), sz);
		return fFalse;
		}

	ibst = pmtm->ibst;
	Assert(ibst < (*hsttbMenu)->ibstMac);
	Assert(ibst >= 0);

	if (ibst == 0)
		CchBuildMenuSz(pmtm->bsy, sz, grfBldMenuDef);
	else
		GetSzFromSttb(hsttbMenu, ibst, sz);

	return fTrue;
}


/* F  C O P Y  M U D  T O  M U D */
/* %%Function:FCopyMudToMud %%Owner:bradch */
BOOL FCopyMudToMud(hmudSrc, hmudDest)
MUD ** hmudSrc;
MUD ** hmudDest;
{
	int cw;

	cw = cwMUD + cwMTM * (*hmudSrc)->imtmMax;
	if (!FChngSizeHCw(hmudDest, cw, fTrue /* fShrink */ ))
		return fFalse;

	bltb(*hmudSrc, *hmudDest, cw * 2);

	return fTrue;
}


/* F  F I N D  I M N U  B S Y */
/* Search a (sorted) MUD for an imnu/bsy pair.  If found, set *pimtm to
index and return fTrue; otherwise, set *pimtm to index of where it would
be and return fFalse. */
/* %%Function:FFindImnuBsy %%Owner:bradch */
FFindImnuBsy(hmud, imnu, bsy, pimtm)
MUD ** hmud;
int imnu, bsy, * pimtm;
{
	int imtm, imtmMac;
	MTM * pmtm;
	MUD * pmud;

	pmud = *hmud;
	imtm = 0;
	imtmMac = pmud->imtmMac;
	pmtm = &pmud->rgmtm[0];
	while (imtm < imtmMac && pmtm->imnu <= imnu)
		{
		if (pmtm->imnu == imnu && pmtm->bsy == bsy)
			{
			*pimtm = imtm;
			return fTrue;
			}
		++imtm;
		++pmtm;
		}

	if ((imnu == imnuFile && (pmtm-1)->bsy == bcmFileCache) ||
			(imnu == (imnuWindow - (viMenu == iMenuShortFull)) &&
			(pmtm-1)->bsy == bcmWndCache))
		*pimtm = imtm - 1;
	else 
		*pimtm = imtm;
	return fFalse;
}


/* S P L I T  R G M T M */
/* %%Function:FSplitRgmtm %%Owner:bradch */
BOOL FSplitRgmtm(hmud, imtm)
MUD ** hmud;
int imtm;
{
	MUD * pmud;
	MTM * pmtm;
	int imtmMac;

	pmud = *hmud;
	imtmMac = pmud->imtmMac;
	Assert(imtmMac <= pmud->imtmMax);
	if (imtmMac == pmud->imtmMax)
		{
		if (!FChngSizeHCw(hmud,
				cwMUD + cwMTM * (pmud->imtmMax + cmtmAlloc),
				fTrue))
			{
			return fFalse;
			}
		pmud = *hmud;
		pmud->imtmMax += cmtmAlloc;
		}
	pmtm = &pmud->rgmtm[imtm];
	bltb(pmtm, pmtm + 1, (imtmMac - imtm) * cbMTM);
	pmud->imtmMac += 1;

	return fTrue;
}
