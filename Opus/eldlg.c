/* eldlg.c */



/*
    This file is




RRRRRR    AAAAA   DDDDDD   I   OOOOO
R     R  A     A  D     D  I  O     O    ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
R     R  A     A  D     D  I  O     O        
R     R  A     A  D     D  I  O     O         ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~  
RRRRRR   AAAAAAA  D     D  I  O     O
R     R  A     A  D     D  I  O     O      ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
R     R  A     A  D     D  I  O     O
R     R  A     A  D     D  I  O     O   ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 
R     R  A     A  DDDDDD   I   OOOOO

                                AAAAA    CCCCC   TTTTT  I  V     V  EEEEEEE
    ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~  A     A  C     C    T    I  V     V  E
                               A     A  C          T    I  V     V  E
       ~ ~ ~ ~ ~ ~ ~ ~ ~ ~     A     A  C          T    I   V   V   E
                               AAAAAAA  C          T    I   V   V   EEEEE
            ~ ~ ~ ~ ~ ~        A     A  C          T    I   V   V   E
                               A     A  C          T    I    V V    E
               ~ ~ ~ ~ ~ ~     A     A  C      C   T    I    V V    E
                               A     A   CCCCCC    T    I     V     EEEEEEE

                    

    It depends heavily on SDM 2.x for the DLT and CAB structures!
*/






/*#define DINTERP*/

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "debug.h"
#include "heap.h"
#include "rerr.h"
#include "cmdtbl.h"
#include "el.h"
#include "idd.h"
#include "opuscmd.h"
#include "macrocmd.h"
#include "ch.h"
#include "props.h"
#include "disp.h"
#include "prm.h"
#include "doc.h"
#define STID
#include "screen.h"
#include "elxinfo.h"
#include "rareflag.h"

#define SDMTMW_PRIVATE
#include "sdmtmpl.h"

#include "tmc.h"


extern SB sbStrings;
extern char szEmpty [];


#define ctmMax		32	/* Max number of items in dialog (all items,
* not just base items), i.e. a list box is
* 2 items and a combo box is 3 items. */

#define cbExtraUsrDlg	512	/* Allocated string space for dialog */

#define ctmNoLastRadioButton -1	/* Arbitrary values, can be anything < 0 */
#define ctmNoCurRadioButtonGroup -2

/* Possible structures for the item "bitslot", depending on what type of item
* it is.   Taken from _sdmtmw.h */
/* FUTURE: we really should be using the real _sdmtmw.h, but it cannot be 
included due to its dependancies... We should get the SDM guys to make
this structure more public. */
typedef union _bitslot
	{
	char b;
	struct _PB /* PushButton */
		{
		BIT  fDismiss:1;
		BIT  fDismissCab:1;
		BIT  fStandardTitle:1;
		BIT  fDefault:1;
		BITS :4;
		};
	struct _RB /* Radio Button */
		{
		BITS iButton:7;
		BITS fLastButton:1;
		};
	struct _CB /* Check Box */
		{
		BIT  fTriState:1;
		BITS :7;
		};
	struct _EI /* Edit Item */
		{
		BITS alt:2;
		BITS EIdtmcGroup:2;
		BIT  fEINotLastInGroup:1;
		BITS foei:3;		/* edit item options */
		};
	struct _LB /* List Box */
		{
		BITS :2;	/* fills in ALT */
		BITS LBdtmcGroup:2;
		BIT  fLBNotLastInGroup:1;
		BITS :3;
		};
	} BITSLOT;


/* Type used to keep track of values that need to be stuffed into the CAB.
* This is needed because handles are listed before word values in the CAB,
* but we don't know until the "DoDialog" how many handles we will have.
* So we keep track of the values in a separate array, then stuff them in
* in ElEndDialog. */
typedef struct _cabval
	{
	int icabval:14;	/* location in cab value array */
	/* Redundant, but used for error checking. */
	int fHsz : 1;	/* true if value is a handle */
	ELK elk;	/* keyword identifier */
} CABVAL;


/* The user dialog structure. */
typedef struct _udlg
	{
	BOOL fNeedDefaults;
	BOOL fHadEnd; /* end-dialog processed */
	BOOL fHasOK;  /* OK Button in dialog */
	BOOL fHasCancel; /* Cancel Button in dialog */
	HDLT hdlt;
	uns ichMaxDltExt;
	uns ichMacDltExt;
	int ctmAll; /* Count of total items in dialog, not just base items */
	int chszCabValueLast;
	CABVAL cabvals[ctmMax];	/* Array of cab values */
	int ccabval;	/* Count of cab values in array */
	int ctmLastRadioButton;	/* Item number of last radio button in current group */
	struct STTB ** mpctmhsttb [ctmMax];	/* Map from item number to string array record */
} UDLG;


UDLG ** vhudlg = hNil;
ELDI ** vheldi = hNil;
struct STTB ** vhsttbElkUser = hNil;


/* This is called when a macro terminates. */
/* %%Function:FreeDialogs %%Owner:bradch */
FreeDialogs()
{
	int i;

	if (vhsttbElkUser != hNil)
		FreePh(&vhsttbElkUser);

	if (vhudlg == hNil)
		{
		Assert(vheldi == hNil);
		return;
		}

	if (vheldi != hNil)
		FreePh(&vheldi);

	/* Free hdlt */
	if ((*vhudlg)->hdlt != hNil)
		FreeH((*vhudlg)->hdlt);

	/* Free hsttbs in mpctmhsttb[] */
	for (i = 0; i < (*vhudlg)->ctmAll; i++)
		{
		if ((*vhudlg)->mpctmhsttb[i] != hNil)
			FreeHsttb((*vhudlg)->mpctmhsttb[i]);
		}

	/* Finally, free vhudlg */
	FreePh(&vhudlg);
}



/* %%Function:ElBeginDialog %%Owner:bradch */
EL ElBeginDialog(x, y, dx, dy, fAutoPos)
uns x, y, dx, dy;
BOOL fAutoPos;
{
	UDLG * pudlg;
	DLT * pdlt;
	DLT ** hdlt;
	int ctmT;

	ClearUsrDlgVars();
	InitEldiCache(); /* cause we are changing the eldi! */

	if (vhudlg != hNil || vhsttbElkUser != hNil)
		{
		FreeDialogs();
		}

	Assert(vhudlg == hNil);
	Assert(vhsttbElkUser == hNil);
	Assert(vheldi == hNil);

#define dxScaleDlg	8
#define dyScaleDlg	12
#define dxStdDlg	4
#define dyStdDlg	8

	x = UMultDiv(x, dxStdDlg, dxScaleDlg);
	dx = UMultDiv(dx, dxStdDlg, dxScaleDlg);
	y = UMultDiv(y, dyStdDlg, dyScaleDlg);
	dy = UMultDiv(dy, dyStdDlg, dyScaleDlg);


	if (dx == 0 || dy == 0 || x > 255 || y > 255 || 
			dx > 255 || dy > 255)
		{
		RtError(rerrIllegalFunctionCall);
		Assert(fFalse); /* NOT REACHED */
		}

	if ((vhsttbElkUser = HsttbInit(0, fFalse)) == hNil)
		{
		goto Loom1;
		}

	if ((vhudlg = HAllocateCb(sizeof (UDLG))) == hNil)
		goto Loom2;

	if ((hdlt = HAllocateCb(sizeof (DLT) + 
			sizeof (TM) * (ctmMax + 1) + cbExtraUsrDlg)) == hNil)
		{
		FreePh(&vhudlg);
Loom2:
		FreeHsttb(vhsttbElkUser);
		vhsttbElkUser = hNil;
Loom1:
		RtError(rerrOutOfMemory);
		Assert(fFalse);	/* NOT REACHED */
		}

	pudlg = *vhudlg;

#ifdef INEFFICIENT
	pudlg->ichMacDltExt = 0;
	pudlg->ctmAll = 0;
	pudlg->ccabval = 0;
	for (ctmT = 0; ctmT < ctmMax; ctmT++)
		{
		pudlg->mpctmhsttb[ctmT] = hNil;
		}
#else
	SetBytes(pudlg, 0, sizeof(UDLG));
#endif

	pudlg->ichMaxDltExt = cbExtraUsrDlg;
	pudlg->hdlt = hdlt;
	pudlg->ctmLastRadioButton = ctmNoCurRadioButtonGroup;

	pdlt = *hdlt;
	pdlt->rec.x = x;
	pdlt->rec.y = y;
	pdlt->rec.dx = dx;
	pdlt->rec.dy = dy;
	pdlt->hid = IDDUsrDlg;
	pdlt->tmcSelInit = tmcNull;
	pdlt->pfnDlg = NULL;
	pdlt->ctmBase = 0;
	pdlt->bdr = bdrThick | (fAutoPos ? bdrAutoPosX | bdrAutoPosY : 0);
	pdlt->bpTitle = NULL;
}



/* %%Function:ElEndDialog %%Owner:bradch */
EL ElEndDialog()
{
	int icabval, ccabval;
	UDLG * pudlg;
	CABVAL * pcabval;
	ELDI * peldi;
	int chszCabValueLast = 0;/* Total number of handle-to-sz cab values */

	if (vhudlg == hNil || (*vhudlg)->fHadEnd)
		{
		/* No dialog in use or repeated End Dialog error */
		RtError(rerrIllegalFunctionCall);
		}

	pudlg = *vhudlg;
	pudlg->fHadEnd = fTrue;
	pudlg->fNeedDefaults = fTrue;
	pcabval = pudlg->cabvals;
	ccabval = pudlg->ccabval;
	for (icabval = 0; icabval < ccabval; icabval++, pcabval++)
		{
		Assert(icabval == pcabval->icabval);
		if (pcabval->fHsz)
			chszCabValueLast += 1;
		}
	pudlg->chszCabValueLast = chszCabValueLast;

	(*pudlg->hdlt)->rgtm[pudlg->ctmAll].tmt = tmtEnd;

	/* Create and init header of EL dialog info structure (the rest 
		is filled in when the cab is filled in CmdUserDialog() for 
		code conservation's sake). */
	if ((vheldi = HAllocateCb(CbEldiOfCelfd(ccabval))) == hNil)
		RtError(rerrOutOfMemory);

	peldi = *vheldi;
	peldi->hid = IDDUsrDlg;
	peldi->cabi = Cabi(ccabval, chszCabValueLast);
	peldi->celfd = ccabval;

		/* BLOCK: fill in the rgelfd of the eldi */
		{
		int icabval, ccabval;
		int chszCabValues = 0;
		int cwCabValues = 0;
		UDLG * pudlg;
		CABVAL * pcabval;
		ELFD * pelfd;

		pudlg = *vhudlg;
		ccabval = pudlg->ccabval;
		pcabval = pudlg->cabvals;
		pelfd = (*vheldi)->rgelfd;

		for (icabval = 0; icabval++ < ccabval; pcabval++, pelfd++)
			{
			pelfd->iag = pcabval->fHsz ? chszCabValues++ : 
					cwCabValues++ + pudlg->chszCabValueLast;
			pelfd->elv = pcabval->fHsz ? elvSd : elvInt;
			pelfd->elk = pcabval->elk;
			}
		}
}


/* %%Function:CmdUserDialog %%Owner:bradch */
CMD CmdUserDialog(pcmb)
CMB * pcmb;
{
	if (!fElActive)
		{
		Beep();
		return cmdError;
		}

	if (vhudlg == hNil || !(*vhudlg)->fHadEnd ||
			!((*vhudlg)->fHasOK || (*vhudlg)->fHasCancel))
		{
		/* No dialog in use, no End Dialog or missing exit item error */
		RtError(rerrIllegalFunctionCall);
		}

	if (pcmb->fDialog)
		{
		if (vhudlg == hNil || (*vhudlg)->hdlt == hNil)
			RtError(rerrIllegalFunctionCall);

		pcmb->fNoHelp = fTrue;

		if (TmcOurDoDlg1((*vhudlg)->hdlt, pcmb) != tmcOK)
			{
			return pcmb->tmc == tmcCancel ? 
					cmdCancelled : cmdError;
			}
		}

	return cmdOK;
}


/* %%Function:BchAddSdToUsrDlg %%Owner:bradch */
uns BchAddSdToUsrDlg(itm, sd)
int itm;
SD sd;
{
	uns bch;
	int cch;
	UDLG * pudlg;
	TM * rgtm;
	char * szInDlt;

	Assert(vhudlg != hNil);

	pudlg = *vhudlg;

	if ((cch = CchFromSd(sd)) > 255)
		{
		RtError(rerrStringTooBig);
		Assert(fFalse); /* NOT REACHED */
		}

	if ((bch = pudlg->ichMacDltExt) + cch >= pudlg->ichMaxDltExt)
		{
		RtError(rerrDlgBoxTooComplex);
		Assert(fFalse);	/* NOT REACHED */
		}

	pudlg->ichMacDltExt += cch + 1;

	rgtm = (*pudlg->hdlt)->rgtm;
	szInDlt = ((char *) &rgtm[ctmMax + 1]) + bch;
	bltbh(HpchFromSd(sd), (CHAR huge *) szInDlt, cch);
	szInDlt[cch] = '\0';

	return szInDlt - ((char *) &rgtm[itm]);
}


/* Adds item to dialog.   Handles any errors by calling RtError. */
/* %%Function:PtmAddTmtToUsrDlg %%Owner:bradch */
TM * PtmAddTmtToUsrDlg(tmt, x, y, dx, dy, sd)
TMT tmt;
uns x, y, dx, dy;
SD sd;
{
	DLT * pdlt;
	TM * ptm;
	UDLG * pudlg;
	int ctmAll;

	if (vhudlg == hNil)
		goto LIllFunc;

	pudlg = *vhudlg;

	Assert(pudlg->hdlt != hNil);
	pdlt = *pudlg->hdlt;

	x = UMultDiv(x, dxStdDlg, dxScaleDlg);
	dx = UMultDiv(dx, dxStdDlg, dxScaleDlg);
	y = UMultDiv(y, dyStdDlg, dyScaleDlg);
	dy = UMultDiv(dy, dyStdDlg, dyScaleDlg);

	if ((tmt != tmtExt2 && (dx == 0 || dy == 0)) ||
			x > 255 || y > 255 || dx > 255 || dy > 255 ||
			x + dx > pdlt->rec.dx || y + dy > pdlt->rec.dy)
		{
LIllFunc:
		RtError(rerrIllegalFunctionCall);
		Assert(fFalse);	/* NOT REACHED */
		}

	if (tmt != tmtRadioButton)
		pudlg->ctmLastRadioButton = ctmNoCurRadioButtonGroup;

	if ((ctmAll = pudlg->ctmAll) == ctmMax)
		{
		/* Too many dialog items error */
		RtError(rerrDlgBoxTooComplex);
		Assert(fFalse);	/* NOT REACHED */
		}

	Assert(pudlg->hdlt != hNil);
	pdlt = *pudlg->hdlt;

	ptm = &pdlt->rgtm[ctmAll];
	ptm->tmt = tmt;
	ptm->fThinBdr = tmt == tmtEdit;
	ptm->fAction = fFalse;
	ptm->bitSlot = 0;
	ptm->wSlot = (sd == NULL) ? 0 : BchAddSdToUsrDlg(ctmAll, sd);

	/* If we need two more bits for each, we can add an 
		extension item TMX1. */

/* NOTE: The next line doesn't work... (could it be the uns's?) */
/*	ptm->l = ((long) dy << 24) | ((long) dx << 16) | (y << 8) | (x);*/
	ptm->l = (dy << 8) | (dx);
	ptm->l = (ptm->l << 16) | (y << 8) | (x);

	/* Increase count of items (including extension items, etc.) */
	pudlg->ctmAll += 1;

	/* If base item, increase count of base items */
	if (tmt < tmtNormalMax)
		pdlt->ctmBase += 1;

	return ptm;
}



/* This is the function called by SDM to supply the strings for Listboxes
* and Combo Boxes.   It will be called first with tmm=tmmCount to get the
* number of strings in the ListBox (or the ListBox part of the Combo Box).
* Then it is called once to get each string, up to the number of strings
* reported by the tmmCount call.   The argument isz tells which string
* to return (0-based), and the wParam argument tells which item the strings
* should be taken from.   This wParam is assigned in the Listbox and
* ComboBox procedures.   If the item is a ComboBox, this function will
* be called with tmm=tmmEditText whenever the user clicks on an entry in
* the ListBox part.   In that case, isz indicates the string to be
* returned.   This string will be put in the EditText part of the Combo
* Box. */
/* %%Function:WListStringArray %%Owner:bradch */
WORD WListStringArray(tmm, sz, isz, filler, tmc, wParam)
TMM tmm;
char * sz;
int isz;
WORD filler;
TMC tmc;
WORD wParam;
{
	struct STTB ** hsttb;

	hsttb = (*vhudlg)->mpctmhsttb[wParam];
	AssertH(hsttb);

	switch (tmm)
		{
	case tmmCount:
		return (*hsttb)->ibstMac;

	case tmmText:
		Assert(isz < (*hsttb)->ibstMac);
		GetSzFromSttb(hsttb, isz, sz);
		return fTrue;
		}

	return 0;
}



/* %%Function:ElOKButton %%Owner:bradch */
EL ElOKButton(x, y, dx, dy)
int x, y, dx, dy;
{
	if (vhudlg == hNil || (*vhudlg)->fHasOK)
		/* No dialog in use or repeated OK Button error */
		RtError(rerrIllegalFunctionCall);
	(*vhudlg)->fHasOK = fTrue;
	OKCanButton(x, y, dx, dy, fFalse);
}



/* %%Function:ElCancelButton %%Owner:bradch */
EL ElCancelButton(x, y, dx, dy)
int x, y, dx, dy;
{
	if (vhudlg == hNil || (*vhudlg)->fHasCancel)
		/* No dialog in use or repeated Cancel Button error */
		RtError(rerrIllegalFunctionCall);
	(*vhudlg)->fHasCancel = fTrue;
	OKCanButton(x, y, dx, dy, fTrue);
}


/* %%Function:OKCanButton %%Owner:bradch */
OKCanButton(x, y, dx, dy, fCancel)
{
	TM * ptm;
	BITSLOT bitslot;

	ptm = PtmAddTmtToUsrDlg(tmtPushButton, x, y, dx, dy, NULL);

	bitslot.b = 0;	/* Zeros all bits in bitslot */
	if (fCancel)
		ptm->wSlot = 1;
	else
		bitslot.fDefault = bitslot.fDismissCab = fTrue;
	bitslot.fDismiss = fTrue;
	bitslot.fStandardTitle = fTrue;
	ptm->bitSlot = bitslot;
}



/* %%Function:ElEditText %%Owner:bradch */
EL ElEditText(x, y, dx, dy, elk)
int x, y, dx, dy;
ELK elk;
{
	TM * ptm;
	UDLG * pudlg;
	CABVAL * pcabval;

	ptm = PtmAddTmtToUsrDlg(tmtEdit, x, y, dx, dy, NULL);

	/* Add handle to string to cabvals array for entry into CAB. */
	pudlg = *vhudlg;
	pcabval = &pudlg->cabvals[pudlg->ccabval];
	pcabval->elk = elk;
	pcabval->icabval = pudlg->ccabval;
	pcabval->fHsz = fTrue;
	pudlg->ccabval += 1;
}





/* %%Function:ElCheckBox %%Owner:bradch */
EL ElCheckBox(x, y, dx, dy, sd, elk)
int x, y, dx, dy;
SD sd;
ELK elk;
{
	UDLG * pudlg;
	CABVAL * pcabval;

	PtmAddTmtToUsrDlg(tmtCheckBox, x, y, dx, dy, sd);

	pudlg = *vhudlg;
	pcabval = &pudlg->cabvals[pudlg->ccabval];
	pcabval->elk = elk;
	pcabval->icabval = pudlg->ccabval;
	pcabval->fHsz = fFalse;
	pudlg->ccabval += 1;
}



/* %%Function:ElDialogText %%Owner:bradch */
EL ElDialogText(x, y, dx, dy, hpsd)
int x, y, dx, dy;
SD huge * hpsd;
{
	/* Due to a bug in SDM, static text must be at least one char width wide */
	if (dx < dxScaleDlg)
		dx = dxScaleDlg;
	/* 0x40 for fostMultiLine */
	PtmAddTmtToUsrDlg(tmtStaticText, x, y, dx, dy, *hpsd)->bitSlot = 0x40;
}



/* %%Function:ElGroupBox %%Owner:bradch */
EL ElGroupBox(x, y, dx, dy, hpsd)
int x, y, dx, dy;
SD huge * hpsd;
{
	PtmAddTmtToUsrDlg(tmtGroupBox, x, y, dx, dy, *hpsd);
}


/* %%Function:IRBAddNewRadioButtonToGroup %%Owner:bradch */
IRBAddNewRadioButtonToGroup()
{
	BITSLOT bitslot;
	UDLG * pudlg;
	int ctmLastRadioButton;

	pudlg = *vhudlg;
	ctmLastRadioButton = pudlg->ctmLastRadioButton;

	Assert(ctmLastRadioButton != ctmNoCurRadioButtonGroup);

	if (ctmLastRadioButton == ctmNoLastRadioButton)
		return 0;

	Assert(ctmLastRadioButton >= 0);
	Assert(ctmLastRadioButton < pudlg->ctmAll);

	bitslot.b = ~0;			/* All bits on, */
	bitslot.fLastButton = fFalse;	/* except last button bit */

	/* We do two things here:
		*   First, turn off the fLastButton bit of the current last button
		*     in the group,
		*   Second, return the next button id.
		* The first button in a group has id 0, second has id 1, etc.
		* The last button in a group will have the fLastButton bit set. */

	return ((*pudlg->hdlt)->rgtm[ctmLastRadioButton].bitSlot &= bitslot) + 1;
}


/* Starts a Radio Button group.   Value is which button will initially
* be set. */
/* %%Function:ElOptionGroup %%Owner:bradch */
EL ElOptionGroup(elk)
ELK elk;
{
	UDLG * pudlg;
	CABVAL * pcabval;

	pudlg = *vhudlg;

	pudlg->ctmLastRadioButton = ctmNoLastRadioButton;
	pcabval = &pudlg->cabvals[pudlg->ccabval];
	pcabval->elk = elk;
	pcabval->icabval = pudlg->ccabval;
	pcabval->fHsz = fFalse;
	pudlg->ccabval += 1;
}



/* %%Function:ElOptionButton %%Owner:bradch */
EL ElOptionButton(x, y, dx, dy, hpsd)
int x, y, dx, dy;
SD huge * hpsd;
{
	UDLG * pudlg;
	TM * ptm;
	BITSLOT bitslot;

	pudlg = *vhudlg;
	if (pudlg->ctmLastRadioButton == ctmNoCurRadioButtonGroup)
		{
		/* Error: No current Radio Button group... */
		RtError(rerrIllegalFunctionCall);
		Assert(fFalse);	/* NOT REACHED */
		}

	ptm = PtmAddTmtToUsrDlg(tmtRadioButton, x, y, dx, dy, *hpsd);
	bitslot.b = 0;
	bitslot.iButton = IRBAddNewRadioButtonToGroup();
	bitslot.fLastButton = fTrue;
	ptm->bitSlot = bitslot;
	pudlg->ctmLastRadioButton = pudlg->ctmAll - 1;
}


/* %%Function:FFillSttbFromAd %%Owner:bradch */
FFillSttbFromAd(hsttb, ad)
struct STTB ** hsttb;
AD ad;
{
	extern SB sbStrings, sbArrays;
	int as;
	SD sd, huge * hpsd;
	char stBuf [256];

	as = HpahrOfAd(ad)->rgas[0];
	hpsd = (SD huge *) &HpahrOfAd(ad)->rgas[HpahrOfAd(ad)->cas];
	while (as-- != 0 && (sd = *hpsd++) != sdNil)
		{
		stBuf[0] = CchFromSd(sd);
		bltbh((char huge *) HpchFromSd(sd), 
				(char huge *) stBuf + 1, stBuf[0]);

		if (IbstAddStToSttb(hsttb, stBuf) == ibstNil)
			return fFalse;
		}

	return fTrue;
}



/* %%Function:ElListBox %%Owner:bradch */
EL ElListBox(x, y, dx, dy, ad, elk)
int x, y, dx, dy;
AD ad;
ELK elk;
{
	TM * ptm;
	UDLG * pudlg;
	int ctmCur;
	struct STTB ** hsttb;
	CABVAL * pcabval;
	char ** hsz;

	if ((hsttb = HsttbInit(0, fTrue)) == hNil)
		goto Loom1;

	if (!FFillSttbFromAd(hsttb, ad))
		{
		FreeHsttb(hsttb);
Loom1:
		RtError(rerrOutOfMemory);
		Assert(fFalse);	/* NOT REACHED */
		}

	ptm = PtmAddTmtToUsrDlg(tmtListBox, x, y, dx, dy, NULL);

	pudlg = *vhudlg;

	/* ctmCur is current item number (0-based). */
	ctmCur = pudlg->ctmAll - 1;

	pudlg->mpctmhsttb[ctmCur] = hsttb;

	pudlg = *vhudlg;
	pcabval = &pudlg->cabvals[pudlg->ccabval];
	pcabval->icabval = pudlg->ccabval;
	pcabval->fHsz = fFalse;
	pcabval->elk = elk;

	ptm = PtmAddTmtToUsrDlg(tmtExt2, 0, 0, 0, 0, NULL);
	ptm->bitSlot = 0;
	ptm->wSlot = ctmCur;
	ptm->l = (LONG) (PFN_CTRL) WListStringArray;

	pudlg->ccabval += 1;
}


/* %%Function:ElComboBox %%Owner:bradch */
EL ElComboBox(x, y, dx, dy, ad, elk)
int x, y, dx, dy;
AD ad;
ELK elk;
{
	TM * ptm;
	UDLG *pudlg;
	int ctmCur;
	BITSLOT bitslot;
	struct STTB ** hsttb;


	/* EditBox is (width - 16) by 18, at x, y
		ListBox is (width - 8) by (height - 18), at (x + 8), (y + 18) */
	/* PtmAddTmtToUsrDlg will RtError if dx or dy becomes negative */
	ptm = PtmAddTmtToUsrDlg(tmtEdit, x, y, dx-16, 18, NULL);

	bitslot.b = 0;
	bitslot.fEINotLastInGroup = 1;	 /* ditto */
	bitslot.b = 0x12;
	ptm->bitSlot = bitslot;

	/* Add handle-to-text to array of cab values */
	pudlg = *vhudlg;
	pudlg->cabvals[pudlg->ccabval].icabval = pudlg->ccabval;
	pudlg->cabvals[pudlg->ccabval].fHsz = fTrue;
	pudlg->cabvals[pudlg->ccabval].elk = elk;
	pudlg->ccabval++;

	ptm = PtmAddTmtToUsrDlg(tmtListBox, x+8, y+18, dx-8, dy-18, NULL);

	bitslot.b = 0;
	bitslot.LBdtmcGroup = 1;
	ptm->bitSlot = bitslot;
	ptm->wSlot = folbComboAtomic | folbSorted;

	if ((hsttb = HsttbInit(0, fTrue)) == hNil)
		goto Loop1;

	if (!FFillSttbFromAd(hsttb, ad))
		{
		FreeHsttb(hsttb);
Loop1:
		RtError(rerrOutOfMemory);
		Assert(fFalse);	/* NOT REACHED */
		}

	pudlg = *vhudlg;
	ctmCur = pudlg->ctmAll - 1;
	pudlg->mpctmhsttb[ctmCur] = hsttb;

	ptm = PtmAddTmtToUsrDlg(tmtExt2, 0, 0, 0, 0, NULL);

	ptm->bitSlot = 0;
	ptm->wSlot = ctmCur;	/* Parameter to WListStringArray */
	ptm->l = (LONG) (PFN_CTRL) WListStringArray;
}

AOB ** hrgaob;
char ** hrgst;
extern struct SCI vsci;
csconst char rgchNear[] = "ADI";

/* %%Function:CheckRgksp %%Owner: */
CheckRgksp(kc)
{
	extern struct MWD ** hmwdCur;

	HWND hwnd;
	HDC hdc;
	int iksp;
	int dxWidth, dyHeight;
	RECT rc;
	int istid = 0;
	int ich;

	if (kc != rgksp[0] || vrf.fRibbonCBT)
		return;
	for (iksp = 1; iksp < sizeof (rgksp) - 1; iksp++)
		if (GetKeyState(rgksp[iksp]) >= 0)
			return;
	for (ich = 0; ich < 3; ich++)
		if (GetKeyState(rgchNear[ich]) < 0)
			return;
	if ((hrgaob = HAllocateCw(iaobMax * sizeof (AOB) / 2)) == hNil)
		return;
	if ((hrgst = HAllocateCb(sizeof(mpstiderc))) == hNil)
		goto LRet2;
	if ((hdc = GetDC(hwnd = GetFocus())) == NULL)
		goto LRet;

	SetWords(*hrgaob, 0, iaobMax * sizeof (AOB) / 2);
	for (ich = 0; ich < sizeof(mpstiderc); ich++)
		(*hrgst)[ich] = mpstiderc[ich] ^ '9';

	GetAsyncKeyState(VK_ESCAPE);
	ShowCursor(fFalse);
	GetClientRect(hwnd, (LPRECT) &rc);
	dxWidth = rc.right - rc.left;
	dyHeight = rc.bottom - rc.top;
	PatBlt(hdc, 0, 0, dxWidth, dyHeight, BLACKNESS);
	
	GenStids(dxWidth, dyHeight, &istid, vsci.dypTmHeight);
			
	for (iksp = 0; iksp < 10075; iksp += 1)
		{
		if (((iksp & 15) == 0) && iksp < 10040)
			CreateAob(dxWidth, dyHeight);
		
		if (GetAsyncKeyState(VK_ESCAPE) < 0)
			break;
		NextFr(hdc, dxWidth, dyHeight, &istid, &iksp);
		}
	
	ReleaseDC(hwnd, hdc);
	ShowCursor(fTrue);
	InvalidateRect(hwnd, (LPRECT) NULL, fTrue);
LRet:
	FreePh(&hrgst);
LRet2:
	FreePh(&hrgaob);
}



/* %%Function:GenStids %%Owner: */
GenStids(dxWidth, dyHeight, pistid, y)
int * pistid;
{
	AOB * paob;
	char * st;
	int fLoop = fFalse;
	int yGuess = y;
	char ch;
	
LNew:
	if (*pistid == sizeof(rgstid)/sizeof(int))
		return;

	y += vsci.dypTmHeight + 1;
	st = &((*hrgst)[rgstid[*pistid]]);
	ch = *(st+1);
	if (fLoop && ch < 'A' && ch != ' ')
		return;

	if ((paob = PaobNew(4 - (ch == '-') + (ch == '*'))) == NULL)
		return;

	paob->st = st;
	paob->x = (dxWidth - vsci.dxpTmWidth * *st) / 2;
	paob->irgb = (int) paob->aot;
	paob->yGuess = yGuess;
	*pistid += 1;
	if (paob->aot == 4)
		{
		paob->y = y;
		fLoop = fTrue;
		goto LNew;
		}
	else
		{
		paob->y = dyHeight;
		paob->dyMove = -2;
		}
}


	

/* %%Function:NextFr %%Owner: */
NextFr(hdc, dxWidth, dyHeight, pistid, piksp)
HDC hdc;
int * pistid, * piksp;
{
	int iaob;
	AOB * paob;
	int y;
	BOOL fTextItem = fFalse;
	
	paob = *hrgaob;
	SetBkMode(hdc, TRANSPARENT);
	for (iaob = 0; iaob < iaobMax; iaob++, paob++)
		{
		if (paob->aot == 0)
			continue;

		if (paob->aot < 3)
			{
			PatBlt(hdc, paob->x, paob->y, paob->dx, paob->dy, 
				BLACKNESS);
			}			
		else
			{
			if (paob->aot != 4 && paob->dyMove != 0)
				AobOut(hdc, paob, 0);
			y = YFromIstid(*pistid, dyHeight);
			}

		switch (paob->aot)
			{
			AOB * paobT;
			
		case 1:
			if (paob->ifr >= WRand(5) + ((paob->dx == 1) * 8))
				{
				paob->ifr = 0;
				paob->dx = paob->dy -= 1;
				if (paob->dx == 0)
					{
					paob->aot = 0;
					break;
					}

				paob->dxMove += WRand(3) - 1;
				paob->dyMove += WRand(3) - 1;
				}
			
			if (paob->dx == 1)
				paob->irgb = 1 + WRand(7);
			break;
			
		case 2:
			if (paob->y <= paob->yGuess || paob->dyMove > 2)
				{
				paob->aot = 0;
				SpawnAobs(paob->x, paob->y);
				break;
				}
			
			if (paob->ifr == 5)
				{
				paob->ddy = 0;
				}
			else if (paob->ifr == 20)
				{
				paob->ddy = 1;
				}
		
			if ((paob->ifr % 20) == 0)
				{
				if (paob->x < paob->xGuess && paob->dxMove < 4)
					paob->dxMove += 1;
				if (paob->x > paob->xGuess && paob->dxMove > -4)
					paob->dxMove -= 1;
				}
		
			if ((paobT = PaobNew(1)) != NULL)
				{
				paobT->x = paob->x;
				paobT->y = paob->y + paob->dy;
				paobT->dx = paobT->dy = 1;
				paobT->dxMove = paob->dxMove + WRand(3) - 1;
				paobT->dyMove = paob->dyMove;
				paobT->ddy = 2;
				paobT->ifr = -WRand(12);
				}
			break;
			
		case 3:
		case 5:
			if (paob->y < paob->yGuess)
				{
				GenStids(dxWidth, dyHeight, pistid, paob->aot == 5 ?
					y : paob->yGuess);
				paob->dyMove = 0;
				paob->yGuess = paob->y;
				}
			break;

		case 4:
			if (paob->ifr == 60)
				{
				int iaob;
				AOB * paobT = *hrgaob;

				for (iaob = 0; iaob < iaobMax; iaob++, paobT++)
					{
					if (paobT->aot >= 3)
						{
						if (*pistid == sizeof(rgstid)/sizeof(int))
							{
							paobT->dyMove = -2;
							paobT->yGuess = -vsci.dypTmHeight;
							paobT->aot = 3;
							goto LErase;
							}
						else if (paobT->aot != 5)
							{
							paobT->aot = 0;
LErase:
							AobOut(hdc, paobT, 0);
							}
						}
					}
				GenStids(dxWidth, dyHeight, pistid, y);
				}

			/* break; IE */
			}

		if (paob->aot == 0)
			continue;
		
		paob->x += paob->dxMove;
		paob->y += paob->dyMove;
		paob->dyMove += paob->ddy;
		paob->ifr += 1;
	
		if (paob->aot >= 3)
			{
			fTextItem |= (paob->y > paob->yGuess);
			AobOut(hdc, paob, paob->irgb);
			}
		else
			{
			HANDLE h;

			if (h = SelectObject(hdc, CreateSolidBrush(vsci.fMonochrome ?
				0x00FFFFFFL : rgrgb[paob->irgb])))
				{
				PatBlt(hdc, paob->x, paob->y, paob->dx, paob->dy, PATCOPY);
				DeleteObject(SelectObject(hdc, h));
				}
			}
		}

	if (!fTextItem && *pistid == sizeof(rgstid)/sizeof(int) && *piksp < 10000)
		*piksp = 10000;
}


/* %%Function:YFromIstid %%Owner: */
YFromIstid(istid, dyHeight)
{
	int csz = 1;
	istid++;

	while (istid < sizeof(rgstid)/sizeof(int) &&
		(*hrgst)[rgstid[istid++]+1] != '-')
		{
		csz++;
		}
	return ((dyHeight - vsci.dypTmHeight * csz) / 2);
}


/* %%Function:AobOut %%Owner: */
AobOut(hdc, paob, irgb)
HDC hdc;
AOB * paob;
{
	SetTextColor(hdc, (vsci.fMonochrome && irgb != 0) ?
		0x00FFFFFFL : rgrgb[irgb]);
	TextOut(hdc, paob->x, paob->y, paob->st+1, *paob->st);
}

