/* ***************************************************************************
**
**      COPYRIGHT (C) 1985 MICROSOFT
**
** ***************************************************************************
*
*  Module: menu.c ----- New menu handling routine
*
*
**
** REVISIONS
**
** Date         Who Rel Ver     Remarks
** 11/8/85      bz              added status area view menu item
** 12/5/85      dsb             added renumber paragraphs menu item
** 2/14/86      dsb             added Zoom command for Page Preview
** 2/20/86      bz              Changed whole ballgame to allow CAB-style
**                               commands and allow menu customization
** 7/3/86       bz              Simplified the mnst initialization scheme
** 7/24/86      dsb             revamped to match new spec
** 10/28/86     bac             Redid everything
** 6/26/87      yxy             Undo strings (bac moved 'em to menu1.c)
**
** ************************************************************************ */



#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOICON
#define NOKEYSTATE
#define NORASTEROPS
#define NOSYSMETRICS
#define NOBITMAP
#define NOBRUSH
#define NOCOLOR
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOFONT
#define NOGDI
#define NOHDC
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOPEN
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#define NOSTRUNDO
#define TABS
#include "cmdtbl.h"
#define USEBCM
#include "cmd.h"
#include "props.h"
#include "ibcm.h"
#include "prm.h"
#include "fontwin.h"
#include "doc.h"
#include "disp.h"
#include "keys.h"
#include "dosfile.h"
#include "sel.h"
#include "print.h"
#include "screen.h"
#include "debug.h"
#include "style.h"
#include "splitter.h"
#include "doslib.h"
#include "error.h"
#undef TABS
#include "ch.h"
#include "menu2.h"
#include "field.h"
#define RULER
#define REPEAT
#include "ruler.h"

#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"

#include "el.h"
#include "macrocmd.h"


#ifdef PROTOTYPE
#include "menu.cpt"
#endif /* PROTOTYPE */

extern HANDLE           vhMenuLongFull;
extern HMENU            vhMenu;
extern int              viMenu;
extern HWND             vhwndApp;
extern struct PREF      vpref;
extern struct STTB      **hsttbMenu;
extern MUD              **vhmudUser;
extern struct PREF      vpref;
extern struct SCI	vsci;
extern CHAR HUGE        *vhpchFetch;
extern HWND             vhwndApp;
extern HWND		vhwndRibbon;
extern int              wwCur;
extern int              mwCur;
extern int		wwMac;
extern struct MWD **    mpmwhmwd[];
extern struct WWD **    mpwwhwwd[];
extern struct STTB **   vhsttbOpen;
extern struct STTB **   vhsttbWnd;
extern struct MERR      vmerr;
extern struct WWD       **hwwdCur;
extern struct MWD	**hmwdCur;
extern CHAR             szEmpty[];
extern CHAR             stEmpty[];
extern struct SEL       selCur;
extern struct AAB       vaab;
/*extern*/ BOOL         vfFileCacheDirty;
/*extern*/ BOOL         vfWndCacheDirty;
extern struct RULSS  vrulss;
extern RRF		rrf;
extern int		vssc;
extern struct UAB	vuab;
extern int  	    	docMac;
extern struct DOD     **mpdochdod[];
extern int              vlm;
extern int              vfSysMenu;
extern HWND				vhwndCBT;



/* %%Function:EnableAppMenu %%Owner:krishnam */
EnableAppMenu(fEnable)
BOOL fEnable;
{
	int cItem, iItem;

	cItem = GetMenuItemCount(vhMenu);
	for (iItem = 0; iItem < cItem; iItem++)
		EnableMenuItem(vhMenu, iItem, (fEnable ? MF_ENABLED : MF_GRAYED) |
				MF_BYPOSITION);

	DrawMenuBar(vhwndApp);
}




/* F  M E N U  B A R  H A S  F R A M E */
/* Return fTrue iff the given menu contains the hyphen menu. */
/* %%Function:FMenuBarHasFrame %%Owner:krishnam */
FMenuBarHasFrame(hmenu)
HMENU hmenu;
{
	char rgch[10];

	rgch[0] = GetMenuString(hmenu, 0, (LPSTR) &rgch[1], sizeof (rgch) - 1,
			MF_BYPOSITION);

	return rgch[0] == 0 || (rgch[0] == 1 && rgch[1] == '-');
}



/* %%Function:HMenuFromMw %%Owner:krishnam */
HMENU HMenuFromMw(mw)
int mw;
{
	Assert(mw != mwNil);

	return (vfSysMenu ? GetSubMenu(vhMenu, 0) :
			GetSystemMenu((*mpmwhmwd[mw])->hwnd, fFalse));
}



/*extern*/ int vbsySepCur = bsySepFirst;

/* A D D  T O  M U D */
/*

if the MTM already exists in the MUD:
(then either it is a removed standard item or an added user item)

		fRemoveOld | fRemove    Action
		-----------+--------    ---------------------
(usr)      0       |    0       (1) update the menu item
(usr)      0       |    1       (2) remove item from MUD 
(std)      1       |    0       (3) remove old mtm, add std item back to MUD
(std)      1       |    1       (4) none

otherwise:
(it is a standard item to remove or it is a user item to add)
		(5) add new MTM and update menu

*/

/* %%Function:FAddToMud %%Owner:krishnam */
BOOL FAddToMud(hmud, imnu, st, bsy, fRemove)
MUD ** hmud;
int imnu;
char * st;
uns bsy;
BOOL fRemove;
{
	MUD * pmud;
	MTM * pmtm;
	int imtm;
	MTM mtm;

	mtm.imnu = imnu;
	mtm.ibst = IbstMenuBsySt(bsy, st);

	if ((mtm.bsy = bsy) == bsyNil)
		mtm.bsy = vbsySepCur++;

	mtm.fRemove = fRemove;
	mtm.fUndo = fFalse;

	pmud = *hmud;

	/* Apply MTM to MUD */
	if (FFindImnuBsy(hmud, imnu, bsy, &imtm))
		{
		pmtm = &pmud->rgmtm[imtm];
		if (FBuiltIn(imnu, bsy))
			*pmtm = mtm;
		else
			{
			if ((fRemove != fFalse) != (pmtm->fRemove != fFalse))
				{
				bltb(pmtm + 1, pmtm, (pmud->imtmMac - imtm) * cbMTM);
				--pmud->imtmMac;
				}
			else  if (!fRemove && pmtm->ibst != mtm.ibst)
				pmtm->ibst = mtm.ibst;
			}
		}
	else
		{
		/*  expand rgmtm and copy in new info */
		if (!FSplitRgmtm(hmud, imtm)) /* HEAP MOVEMENT */
			return fFalse;

		(*hmud)->rgmtm[imtm] = mtm;
		}

	return fTrue;
}


/* D I F F  M U D S */
/* "Subtract" hmudSrc from hmudDest */
/* NOTE: this may leave hmudDest partly changed in low mem situations! */
/* %%Function:FDiffMuds %%Owner:krishnam */
BOOL FDiffMuds(hmudDest, hmudSrc)
MUD ** hmudDest;
MUD ** hmudSrc;
{
	MUD * pmudDest, * pmudSrc;
	MTM * pmtmDest, * pmtm;
	int imtmDest, imtm, imtmMac;

	pmudDest = *hmudDest;
	pmudSrc = *hmudSrc;

	imtmMac = pmudSrc->imtmMac;
	for (pmtm = &pmudSrc->rgmtm[imtm = 0]; imtm < imtmMac; ++imtm, ++pmtm)
		{
		if (FFindImnuBsy(hmudDest, pmtm->imnu, pmtm->bsy, &imtmDest))
			{
			pmtmDest = &pmudDest->rgmtm[imtmDest];
			if (pmtmDest->ibst != pmtm->ibst)
				{
				pmtmDest->ibst = pmtm->ibst;
				pmtmDest->fRemove = fTrue;
				}
			else  if (pmtmDest->fRemove == pmtm->fRemove)
				{
				/* remove pmtmDest */
				bltb(pmtmDest + 1, pmtmDest, cbMTM *
						(pmudDest->imtmMac - imtmDest));
				pmudDest->imtmMac -= 1;
				}
			}
		else
			{
			if (!FSplitRgmtm(hmudDest, imtmDest))/*HEAP MOVEMENT*/
				{
				return fFalse;
				}
			pmudDest = *hmudDest;
			pmudSrc = *hmudSrc;
			pmtm = &pmudSrc->rgmtm[imtm];
			pmtmDest = &pmudDest->rgmtm[imtmDest];
			*pmtmDest = *pmtm;
			pmtmDest->fRemove = !pmtmDest->fRemove;
			}
		}

	return fTrue;
}


/* C L E A R  M U D */
/* %%Function:ClearMud %%Owner:krishnam */
ClearMud(hmud)
MUD ** hmud;
{
	MUD * pmud;

	pmud = *hmud;
	pmud->imtmMac = 0;
}



#ifdef DEBUG
/* %%Function:DumpMud %%Owner:krishnam */
DumpMud(hmud)
MUD ** hmud;
{
	MUD * pmud;
	MTM * pmtm;
	int imtm, imtmMac;

	pmud = *hmud;
	CommSzNum("imtmMac: ", imtmMac = pmud->imtmMac);
	for (pmtm = &pmud->rgmtm[imtm = 0]; imtm < imtmMac; ++imtm, ++pmtm)
		{
		CommSzNum("imnu: ", pmtm->imnu);
		CommSzNum("bsy: ", pmtm->bsy);
		CommSzNum("ibst: ", pmtm->ibst);
		CommSzNum("fRemove: ", pmtm->fRemove);
		}
}


#endif /* DEBUG */


/* F  S E T  S H O R T  M E N U */
/*  put up app short menus */
/* %%Function:FSetShortMenu %%Owner:krishnam */
BOOL FSetShortMenu()
{
	extern int vfInitializing;
	return FSetMenu(!vfInitializing && selCur.doc == docNil ?
			iMenuShortMin : iMenuShortFull);
}


/* F  S E T  M I N  M E N U */
/*  put up short or long MIN menu */
/* %%Function:FSetMinMenu %%Owner:krishnam */
BOOL FSetMinMenu()
{
	return FSetMenu(vpref.fShortMenus ? iMenuShortMin : iMenuLongMin);
}


/* C O D E  B A S E D  M E N U S */

#define bcmPopupMenu 0xfff0
#define bcmEndMenu 0xfff1

struct CBM
	{
	BCM bcm;
	CHAR st[];
};

#include "opusmenu.h" /* defines rgcbm and rgicbm */

/* %%Function:LpstToSz %%Owner:krishnam */
LpstToSz(lpst, sz)
CHAR FAR *lpst;
CHAR *sz;
{
	int cch = *lpst;
	bltbx(lpst+1, sz, cch);
	sz[cch] = 0;
}


/* H  M E N U  L O A D  C O D E  M E N U */
/* %%Function:HMenuLoadCodeMenu %%Owner:krishnam */
HMENU HMenuLoadCodeMenu(i)
int i;
{
	HMENU hMenu, hMenuSub = NULL;
	int icbm = rgicbm[i];
	BCM bcm;
	CHAR rgch[cchMaxSz];

	if ((hMenu = CreateMenu()) == NULL)
		return NULL;

	for (;;)
		{
		bcm = rgcbm[icbm].bcm;

		LpstToSz(rgcbm[icbm++].st, rgch);

		if (bcm == bcmPopupMenu)
			{
			if ((hMenuSub = CreateMenu()) == NULL || 
					ChangeMenu(hMenu, bsyNil, (LPSTR)rgch, hMenuSub, MF_POPUP|MF_APPEND)
					== NULL)
				goto LFail;
			}
		else  if (bcm == bcmEndMenu)
			return hMenu;
		else
			{
			int mf = MF_APPEND;
			if (bcm==bcmSeparator || bcm==bcmFileCache || bcm==bcmWndCache)
				mf |= (MF_SEPARATOR|MF_DISABLED);

			if (ChangeMenu(hMenuSub, 0, (LPSTR)rgch, bcm, mf) == NULL)
				{
LFail:
				if (hMenu != NULL)
					DestroyMenu(hMenu);
				ErrorEid(eidNoMemory, "HMenuLoadCodeMenu");
				return NULL;
				}
			}
		}
}


/* F R O M  C M D T B L 2 . C */
extern HPSY HpsyFromBcmElg();

extern HPSYT vhpsyt;
extern struct SEL selCur;
extern int docGlobalDot;


BCM BcmOfSt();
NATIVE uns WHash(); /* DECLARATION ONLY */


/* %%Function:BsyAddCmd1 %%Owner:krishnam */
BsyAddCmd1(st, docDot, imcr)
char * st;
int docDot;
int imcr;
{
	uns bsy;
	BCM bcm;
	int docMcr;
	MCR mcr;

	docMcr = PdodDoc(docDot)->docMcr;
	Assert(docMcr != docNil);
	GetPlc( PdodDoc(docMcr)->hplcmcr, imcr, &mcr );

	if ((bcm = BcmOfSt(st)) != bsyNil)
		{
		HPSY hpsy, hpsyNew;

		hpsy = (HPSY) (vhpsyt->grpsy + bcm);

#ifdef DCMDTBL
		CommSz(SzShared("Adding macro to superceed existing macro/cmd\r\n"));
#endif
		if ((bsy = BsyAddMacro(stEmpty, docDot, imcr, bcm)) == bsyNil)
			return bsyNil;

		mcr.bcm = bsy;
		PutPlc( PdodDoc(docMcr)->hplcmcr, imcr, &mcr );

		hpsyNew = (HPSY) (vhpsyt->grpsy + bsy);
		hpsy = (HPSY) (vhpsyt->grpsy + bcm);

#ifdef DCMDTBL
		CommSz(SzShared("Adding to chain of: "));
			{
			char stBuf [256];
			bltbh(hpsy->stName, (char HUGE *) stBuf, *hpsy->stName + 1);
			CommStNL(stBuf);
			}
#endif
		hpsyNew->bsyNext = hpsy->bsyNext;
		hpsy->bsyNext = bsy;
		}
	else
		{
		if ((bsy = BsyAddMacro(st, docDot, imcr, bsyNil)) != bsyNil)
			{
			mcr.bcm = bsy;
			PutPlc( PdodDoc(docMcr)->hplcmcr, imcr, &mcr );
			}
		}

	return bsy;
}


/* R E M O V E  C M D */
/* Remove a command from the table */
/* %%Function:RemoveCmd %%Owner:krishnam */
RemoveCmd(st, elg)
char * st;
int elg;
{
	BCM bcm;
	HPSY hpsy;

	if ((bcm = BcmOfSt(st)) != bcmNil &&
			(hpsy = HpsyFromBcmElg(vhpsyt, bcm, elg, 0)) != hpNil)
		{
		hpsy->mct = mctNil;
		}
}


/* B S Y  A D D  M A C R O */
/* %%Function:BsyAddMacro %%Owner:krishnam */
BsyAddMacro(st, docDot, imcr, bcm)
char * st;
int docDot, imcr;
uns bcm;
{
	HPSY hpsy;
	uns bsy, cch;
	uns bsyMax, cbSyNew;
	uns wHash;

	Assert(vhpsyt != hpNil);

		/* BLOCK */
		{
		cch = st[0];
		cbSyNew = cbSY + cch;
		if (cch == 0)
			cbSyNew += sizeof (uns); /* for back pointer */

		if (vhpsyt->bsy + cbSyNew >= (bsyMax = vhpsyt->bsyMax))
			{
			if (CbReallocSb(SbOfHp(vhpsyt), 
					(uns)cbSYT + bsyMax + cbAllocSymbols,
					HMEM_MOVEABLE) == 0)
				{
				SetErrorMat(matMem);
				return bsyNil;
				}

			vhpsyt->bsyMax = bsyMax + cbAllocSymbols;
			}

		/* get index and pointer */
		bsy = vhpsyt->bsy;
		hpsy = (HPSY) (vhpsyt->grpsy + bsy);

		/* fill in the sy */
		bltbh((char HUGE *) st, hpsy->stName, cch + 1);
		if (cch == 0)
			*((int HUGE *) (hpsy->stName + 1)) = bcm;
#ifdef DCMDTBL
		CommSzNum(SzShared("cch = "), cch);
#endif
		hpsy->grf = 0; /* must be done before mct is set!!! */
		hpsy->mct = (docGlobalDot == docNil || docDot == docGlobalDot)
				? mctGlobalMacro : mctMacro;
#ifdef DCMDTBL
		CommSzNum(SzShared("Adding mct: "), hpsy->mct);
#endif
		hpsy->imcr = imcr;
		hpsy->docDot = docDot;
		hpsy->ibstMenuHelpP1 = 0;
		hpsy->fPreviewMode=fTrue;

		if (cch != 0)
			{
			/* link sy into chain matching hash */
			uns HUGE * hpbsy;

			hpbsy = &vhpsyt->mpwbsy[WHash(st)];

			hpsy->bsyNext = *hpbsy;
			*hpbsy = bsy;
			}

		/* bump index */
		vhpsyt->bsy += cbSyNew;
		}
	return bsy;
}


/* S Y M B O L  T A B L E  M A N A G E M E N T  F U N C T I O N S */


/* B S Y  F R O M  E L G  S T */
/* %%Function:BsyFromElgSt %%Owner:krishnam */
uns BsyFromElgSt(elg, st)
ELG elg;
char * st;
{
	HPSY hpsy;
	int bcm;

	if ((bcm = BcmOfSt(st)) == bcmNil)
		return bcmNil;

	Assert(vhpsyt != NULL);

	if ((hpsy = HpsyFromBcmElg(vhpsyt, bcm, elg, 0)) == hpNil)
		bcm = bcmNil;
	else
		bcm = (uns) ((char HUGE *) hpsy - vhpsyt->grpsy);

	return bcm;
}


/* %%Function:PsyGetSyFromBcmElg %%Owner:krishnam */
SY * PsyGetSyFromBcmElg(rgchSy, bcm, elg)
char rgchSy [];
BCM bcm;
ELG elg;
{
	HPSY hpsy;

	hpsy = HpsyFromBcmElg(vhpsyt, bcm, elg, 0);
	bltbh(hpsy, (char HUGE *) rgchSy, CbHpsy(hpsy));

	return (SY *) rgchSy;
}


/* %%Function:GetNameFromBsy %%Owner:krishnam */
GetNameFromBsy(stName, bsy)
char * stName;
uns bsy;
{
	HPSY hpsy;
	char HUGE * hpst;

		/* BLOCK */
		{
		hpsy = (HPSY) (vhpsyt->grpsy + bsy);

		if (bsy == bsyNil)
			{
			stName[0] = 0;
			return;
			}

		if (hpsy->stName[0] == 0)
			{
			bsy = *((int HUGE *) (hpsy->stName + 1));
			hpsy = (HPSY) (vhpsyt->grpsy + bsy);
			}

		hpst = hpsy->stName;
		Assert(*hpst < cchMaxSyName);
		bltbh(hpst, (char HUGE *) stName, *hpst + 1);
		}
}


/* R E M O V E  D O T  M A C R O S */
/*
Removes all macros associated with a given document template from the
command table.  This is called when a template is removed from memory. 
*/
/* %%Function:RemoveDotMacros %%Owner:krishnam */
RemoveDotMacros(docDot)
int docDot;
{
	int imcr, imcrMac;
	uns bsy;
	struct PLC ** hplcmcr;

	Assert(docDot != docNil && PdodDoc(docDot)->fDot &&
			PdodDoc(docDot)->docMcr != docNil);
	hplcmcr = PdodDoc(PdodDoc(docDot)->docMcr)->hplcmcr;
	AssertH(hplcmcr);
	imcrMac = (*hplcmcr)->iMac - 1;
	for (imcr = 0; imcr < imcrMac; imcr += 1)
		{
		Assert(sizeof (bsy) == sizeof (MCR));
		GetPlc(hplcmcr, imcr, &bsy);
		Assert(bsy != bsyNil);
		((HPSY) (vhpsyt->grpsy + bsy))->mct = mctNil;
		}

	return fTrue;
}


/* F  C L O S E  D O T  M A C R O S */
/*
Closes all macro editing windows beloning to docDot.  Returns fTrue if
all went well, fFalse if the user cancelled out.
*/
/* %%Function:FCloseDotMacros %%Owner:krishnam */
FCloseDotMacros(docDot, ac)
int docDot;
int ac;
{
	extern MES ** vhmes;

	int imei;

	if (vhmes == hNil)
		return fTrue;

	/* Done backwards because indices change as we close... */
	for (imei = (*vhmes)->imeiMax - 1; imei >= 0; imei -= 1)
		{
		if (PmeiImei(imei)->elg == docDot && !FCloseEdMacro(imei, ac))
			return fFalse;
		}

	return fTrue;
}


/* F B U I L T I N */
/* checks whether an item is built in for a given menu */
/* %%Function:FBuiltIn %%Owner:krishnam */
BOOL FBuiltIn(imnu, bsy)
int imnu;
uns bsy;
{
	extern MUD **vhmudBase;
	MUD *pmud = *vhmudBase;
	int imtmMac = pmud->imtmMac;
	int imtm = ImtmFindImnu(imnu, pmud->rgmtm, imtmMac);
	MTM *pmtm = &pmud->rgmtm[imtm];

	while (imtm < imtmMac && pmtm->imnu == imnu)
		{
		if (pmtm->bsy == bsy)
			return fTrue;
		imtm++;
		pmtm++;
		}

	return fFalse;
}


/* %%Function:ImtmFindImnu %%Owner:krishnam */
int ImtmFindImnu(imnu, rgmtm, imtmMac)
int imnu;
MTM rgmtm[];
int imtmMac;
{
	int imtmLow,imtmMid,imtmHigh;

	imtmLow = 0;
	imtmHigh = imtmMac-1;
	while (imtmLow<imtmHigh)
		{
		imtmMid = imtmLow + (imtmHigh - imtmLow >> 1);
		if (imnu <= rgmtm[imtmMid].imnu)
			imtmHigh=imtmMid;
		else 
			imtmLow=imtmMid+1;
		}

	return(imtmHigh);
}


