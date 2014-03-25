
/* debugscc.c */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "props.h"
#include "disp.h"
#include "screen.h"
#include "scc.h"
#include "prm.h"
#include "file.h"
#include "doc.h"
#include "format.h"
#include "debug.h"

extern struct WWD       **hwwdCur;
extern struct WWD       **mpwwhwwd[];
extern int              wwCur;
extern int              wwMac;
extern struct SCC       *vpsccBelow;
extern struct SCC       vsccAbove;
extern int              *pwMax;
extern char             rgchPLFDECharProps[];
extern char             rgchPLFDEParaProps[];

extern struct DOD       **mpdochdod[];
extern int              docMac;

extern struct FCB       **mpfnhfcb[];
extern int              fnMac;

extern char             (**vhgrpchr)[];
extern int              vbchrMac;

extern struct FLI       vfli;

#ifdef DEBUG

extern int              vimark;
extern struct DBS       vdbs;


/* C K  S C C */
/* check consistency of a scc */
/* %%Function:CkScc %%Owner:BRADV */
CkScc(pscc)
struct SCC *pscc;
{
#ifdef REVIEW	/* REVIEW iftime: wrt new disp scheme (rp) */

	int iAssert;
	struct EDL edl;
	struct PLCEDL **hplcedl;
	int dl;
	int ypTop;
	BOOL fOutline;
	BOOL fInNoBitmap = fTrue;
	CP cpLim;

	if (!vdbs.fCkScc) return;
	if (pscc == NULL) return;
	if (pscc->ww == wwNil || wwCur == wwNil) return;

	/*Assert (mpwwhwwd[pscc->ww] == hwwdCur && pscc->ww == wwCur);*/
	fOutline = PwwdWw(pscc->ww)->fOutline;
	iAssert = 1;
	AssertIn(pscc->dlMac, 0 , pscc->dlMax + 1);
	iAssert = 2;
	Assert(pscc->doc == (*mpwwhwwd[pscc->ww])->doc);
/* The following two asserts can be made about xpMac and ypMax
because if they are converted to xa's and ya's they cannot overflow
integer bounds */
/* REVIEW iftime: Constants are bogus for windows (rp) */
	if (vpsccBelow)
		{
		iAssert = 3;
		AssertIn(pscc->xpMac, 0, 2000);
		iAssert = 4;
		AssertIn(pscc->ypMax, 0, 2000);
		/*iAssert = 5;
		AssertIn(pscc->ypMac, 0, pscc->ypMax + 1);*/
		}
	if (pscc == vpsccBelow)
		{
		iAssert = 6;
		AssertIn(pscc->dlFirstInval, 0, pscc->dlMax + 1);
		}
	else
		{
		iAssert = 8;
		Assert(pscc->dlFirstInval == pscc->dlMax);
		}

	hplcedl = pscc->hplcedl;
	ypTop = 0;
	iAssert = 9;
	Assert(IMacPlc(hplcedl) == pscc->dlMac);
	if (pscc->dlMac)
		{
		for (dl = 0; dl < pscc->dlMac; dl++)
			{
			struct EDL edl;

			GetPlc( hplcedl, dl, &edl );
			iAssert = 10;
			if (edl.fHasBitmap)
				fInNoBitmap = fFalse;
			else
				Assert(fInNoBitmap);
			if (vpsccBelow && !fOutline)
				Assert(edl.yp - edl.dyp == ypTop);
			if (dl != 0)
				{
				iAssert = 11;
				if (pscc == &vsccAbove && !fOutline
						&& !(pscc->fBreak && dl == pscc->dlBreak))
					Assert(CpPlc(hplcedl,dl) == cpLim);
				else
					Assert(CpPlc(hplcedl,dl) >= cpLim);
				}
			cpLim = CpPlc(hplcedl,dl) + edl.dcpMac;
			if (!fInNoBitmap)
				ypTop += edl.dyp;
			if (vpsccBelow)
				{
				iAssert = 12;
				if (edl.fValid)
					Assert(ypTop <= pscc->ypMax);
				}
			}
		iAssert = 13;
		if (vpsccBelow && !fOutline)
			Assert(ypTop == pscc->ypMac ||
					(pscc->dlMac == 1 && !pscc->dndl[0].fValid));
		iAssert = 14;
		if (pscc == &vsccAbove && !fOutline
				&& !(pscc->fBreak && dl == pscc->dlBreak))
			Assert(CpPlc( hplcedl,dl) == cpLim);
		else
			Assert(CpPlc( hplcedl,dl) >= cpLim);
		}
#endif
}


/* C H E C K  S C C S */
/* checks all scc's */
/* %%Function:CkSccs %%Owner:BRADV */
CkSccs()
{
#ifdef REVIEW	/* REVIEW iftime: wrt new disp scheme (rp) */
	struct PLCEDL **hplcedl;
	int dypInvis;
	int dlLastFull;
	struct EDL edl;

	if (!vdbs.fCkScc) return;
	if (wwCur == wwNil) return;
	if (mpwwhwwd[vsccAbove.ww] == hNil) return;
	UpdateWw(vsccAbove.ww, fFalse);

	CkScc(&vsccAbove);
	if (mpwwhwwd[vpsccBelow->ww] == hNil) return;
	CkScc(vpsccBelow);
/* check seamless join from bottom of window and first in cache below */
	hplcedl = (*hwwdCur)->hplcedl;
	GetPlc( hplcedl, dlLastFull = (*hwwdCur)->dlMac - 1, &edl );
	if ((dypInvis = edl.yp - (*hwwdCur)->ypMac) != 0)
		dlLastFull--;
	GetPlc( hplcedl, dlLastFull, &edl );
	vdbs.fJoinBelow = vpsccBelow->dlMac == 0 ||
			vpsccBelow->plcedl.rgcp[0] ==
			CpPlc( hplcedl, dlLastFull)
			+ edl.dcpMac;
/* top of window and last in cache above */
	GetPlc( vsccAbove.hplcedl, vsccAbove.dlMac - 1, &edl );
	vdbs.fJoinAbove = vsccAbove.dlMac == 0 ||
			CpFirstFull(*hwwdCur) == vsccAbove.plcedl.rgcp[vsccAbove.dlMac - 1] + edl.dcpMac;

#endif
}


/* A S S E R T  I N */
/* asserts that x is in [xFirst, xLim) */
/* %%Function:AssertIn %%Owner:BRADV */
AssertIn(x, xFirst, xLim)
{
	Assert(x >= xFirst && x < xLim);
}


/* T E S T  S C C */
/* white box testing of scc routines */
/* %%Function:TestScc %%Owner:BRADV */
TestScc()
{
#ifdef REVIEW	/* REVIEW iftime: Update to new display structs (rp) */
	CP cp;
	int yp, dyp;
	struct EDL edl;
	struct PLCEDL **hplcedl;
	int dypInvis;
	int dypVis;
	int dlLastFull;
	int imarkT;
	int dypPage = (*hwwdCur)->ypMac - (*hwwdCur)->ypMin - 30;
	int i;

	ClearSccs();
	SynchSccWw(&vsccAbove, wwCur);
	SynchSccWw(vpsccBelow, wwCur);

	ThumbToCpLarge(cp0, fFalse);
	UpdateWw(wwCur, fFalse);

	switch (vdbs.jnScc)
		{
	case 0:
/* test DlUpdateFromScc */
		hplcedl = (*hwwdCur)->hplcedl;
		cp = (*hplcedl)->rgcp[0];
		Assert(DlUpdateFromScc(wwCur, vpsccBelow, 0, &cp, &yp) < 0);
/* we will move lines 0 and 1 to the sccAbove to check update from cache */
		MoveToSccAbove(wwCur, 0, 2);
		CkScc(&vsccAbove);

		GetPlc( hplcedl, 0, &edl );
		cp = CpPlc( hplcedl, 0 );
		yp = edl.yp - edl.dyp;
		Assert(DlUpdateFromScc(wwCur, &vsccAbove, 0, &cp, &yp) == 0);
		CkScc(&vsccAbove);

		GetPlc( hplcedl, 1, &edl );
		cp = CpPlc( hplcedl, 1 );
		yp = edl.yp - edl.dyp;
		Assert(DlUpdateFromScc(wwCur, &vsccAbove, 1, &cp, &yp) == 1);
		CkScc(&vsccAbove);

/* not in cache */
		GetPlc( hplcedl, 2, &edl );
		cp = CpPlc( hplcedl, 2 );
		yp = edl.yp - edl.dyp;
		Assert(DlUpdateFromScc(wwCur, &vsccAbove, 2, &cp, &yp) < 0);
		CkScc(&vsccAbove);

/* check MoveToSccBelow */
/* determine dlLastFull, force movement by "scrolling" dl0 by dypInvis + 1 */

		GetPlc( hplcedl, dlLastFull = (*hwwdCur)->dlMac - 1, &edl );
		if ((dypInvis = edl.yp - (*hwwdCur)->ypMac) != 0)
			dlLastFull--;
		GetPlc( hplcedl, dlLastFull, &edl );
		dypVis = (*hwwdCur)->ypMac - edl.yp;
		MoveToSccBelow(wwCur, 0, (*hwwdCur)->ypMin + dypVis);
		CkScc(vpsccBelow);
		Assert(vpsccBelow->dlMac == 0);
		MoveToSccBelow(wwCur, 0, (*hwwdCur)->ypMin + dypVis + 1);
		CkScc(vpsccBelow);
		Assert(vpsccBelow->dlMac == 1);

		cp = CpPlc( hplcedl, dlLastFull );
		yp = edl.yp - edl.dyp;
		Assert(DlUpdateFromScc(wwCur, vpsccBelow, dlLastFull, &cp, &yp)
				>= 0);
/* check EnrichSccBelow */
		ClearSccs();
		vpsccBelow->dlFirstInval = 0;
		FEnrichSccBelow();
		CkSccs();
		FEnrichSccBelow();
		CkSccs();
		FEnrichSccBelow();
		CkSccs();
		FEnrichSccBelow();
		CkSccs();
		FEnrichSccBelow();
		CkSccs();
		Assert(vpsccBelow->dlFirstInval == vpsccBelow->dlMac);
/* repeat with full scc */
		imarkT = vimark;
		FEnrichSccBelow();
		Assert(imarkT == vimark); /* no work was done to refill */
		CkSccs();
	case 1:
/* check scrolls */
		Scribble(8, '1');
		ClearSccs();
		ScrollUp(wwCur,1, 1);
		CkSccs();
		Assert(vsccAbove.dlMac == 1);
		Assert((*hwwdCur)->dypAbove == 1);

		UpdateWw(wwCur, fFalse);
		CkSccs();
		/*      Assert(vdbs.cdlInval == 0);     */
		Assert(vsccAbove.dlMac == 1);
		Assert((*hwwdCur)->dypAbove == 1);
		/*      Assert(vdbs.fJoinAbove);        */

		ScrollDown(wwCur,1, 1);
		CkSccs();
		Assert((*hwwdCur)->dypAbove == 0);
		/*      Assert(vdbs.fJoinAbove);        */
	case 2:
		Scribble(8, '2');
		ClearSccs();
		ThumbToCp(wwCur,cp0, fFalse, fFalse);
		UpdateWw(wwCur, fFalse);
		CkSccs();
		ScrollUp(wwCur,1, 1);
		CkSccs();
		Assert((*hwwdCur)->dypAbove == 1);
		/*      Assert(vdbs.fJoinAbove);        */

		GetPlc( hplcedl, 0, &edl );
		dyp = edl.dyp;
		ScrollUp(wwCur,dyp, dyp);
		CkSccs();
		Assert((*hwwdCur)->dypAbove == 1);
		/*      Assert(vdbs.fJoinAbove);        */
		UpdateWw(wwCur, fFalse);
		CkSccs();

		ScrollDown(wwCur,1, 1);
		CkSccs();
		Assert((*hwwdCur)->dypAbove == 0);
		/*      Assert(vdbs.fJoinAbove);        */
		UpdateWw(wwCur, fFalse);
		CkSccs();

		ClearSccs();
		ThumbToCp(wwCur,cp0, fFalse, fFalse);
		UpdateWw(wwCur, fFalse);
		CkSccs();
		ScrollUp(wwCur,15, 15);
		UpdateWw(wwCur, fFalse);
		ScrollDown(wwCur,0, 0);
		UpdateWw(wwCur, fFalse);
		CkSccs();

		ClearSccs();
		ThumbToCp(wwCur,cp0, fFalse, fFalse);
		UpdateWw(wwCur, fFalse);
		CkSccs();
		ScrollUp(wwCur,16, 16);
		CkSccs();
		/*      Assert(vdbs.fJoinAbove);        */
		UpdateWw(wwCur, fFalse);
		CkSccs();

		ScrollDown(wwCur,4, 4);
		CkSccs();
		/*      Assert(vdbs.fJoinAbove);        */
		UpdateWw(wwCur, fFalse);
		CkSccs();

		ClearSccs();
		ThumbToCp(wwCur,cp0, fFalse, fFalse);
		UpdateWw(wwCur, fFalse);
		ScrollUp(wwCur,16, 16);
		CkSccs();
		UpdateWw(wwCur, fFalse);
		CkSccs();
		ScrollUp(wwCur,14, 14);
		/*      Assert(vdbs.fJoinAbove);        */
		CkSccs();
		UpdateWw(wwCur, fFalse);
		CkSccs();

	case 3:
		Scribble(8, '3');
		ClearSccs();
		ThumbToCp(wwCur,cp0, fFalse);
		UpdateWw(wwCur, fFalse);
		ScrollUp(wwCur,12, 30);
		Assert(vsccAbove.dlMac == 1);
		CkSccs();
		UpdateWw(wwCur, fFalse);
		CkSccs();
		ScrollDown(wwCur,12, 30);
		CkSccs();
		UpdateWw(wwCur, fFalse);
		CkSccs();
		ScrollUp(wwCur,12, 30);
		CkSccs();
		UpdateWw(wwCur, fFalse);
		CkSccs();
		ScrollUp(wwCur,12, 30);
		CkSccs();
		UpdateWw(wwCur, fFalse);
		CkSccs();
		ScrollUp(wwCur,12, 30);
		CkSccs();
		UpdateWw(wwCur, fFalse);
		CkSccs();
		ScrollDown(wwCur,12, 30);
		CkSccs();
		UpdateWw(wwCur, fFalse);
		CkSccs();
		ScrollDown(wwCur,12, 30);
		CkSccs();
		UpdateWw(wwCur, fFalse);
		CkSccs();
		ScrollDown(wwCur,12, 30);
		CkSccs();
		UpdateWw(wwCur, fFalse);
		CkSccs();
		ScrollDown(wwCur,0, 0);
		CkSccs();
	case 4:
		Scribble(8, '4');
		UpdateWw(wwCur, fFalse);
		CkSccs();
		ScrollUp(wwCur,dypPage, dypPage + 18);
		CkSccs();
		UpdateWw(wwCur, fFalse);
		CkSccs();
		ScrollDown(wwCur,dypPage, dypPage + 18);
		CkSccs();
		UpdateWw(wwCur, fFalse);
		CkSccs();
/* scroll 3 pages forward then back */
		ScrollUp(wwCur,dypPage, dypPage + 18);
		CkSccs();
		UpdateWw(wwCur, fFalse);
		CkSccs();
		ScrollUp(wwCur,dypPage, dypPage + 18);
		CkSccs();
		UpdateWw(wwCur, fFalse);
		CkSccs();
		ScrollUp(wwCur,dypPage, dypPage + 18);
		CkSccs();
		UpdateWw(wwCur, fFalse);
		CkSccs();
		ScrollDown(wwCur,dypPage, dypPage + 18);
		CkSccs();
		UpdateWw(wwCur, fFalse);
		CkSccs();
		ScrollDown(wwCur,dypPage, dypPage + 18);
		CkSccs();
		UpdateWw(wwCur, fFalse);
		CkSccs();
		ScrollDown(wwCur,dypPage, dypPage + 18);
		CkSccs();
		UpdateWw(wwCur, fFalse);
		CkSccs();
	case 5:
/* check thumbs */
		Scribble(8, '5');
		cp = (*(*hwwdCur)->hplcedl)->rgcp[(*hwwdCur)->dlMac - 1];
		ThumbToCp(wwCur,(*(*hwwdCur)->hplcedl)->rgcp[4], fFalse);
		UpdateWw(wwCur, fFalse);
		CkSccs();
		ThumbToCp(wwCur,cp0, fFalse);
		UpdateWw(wwCur, fFalse);
		CkSccs();
		ThumbToCp(wwCur,cp, fFalse);
		UpdateWw(wwCur, fFalse);
		CkSccs();
		ThumbToCp(wwCur,CpMacDoc((*hwwdCur)->doc), fFalse);
		UpdateWw(wwCur, fFalse);
		CkSccs();
	case 6:
		Scribble(8, '6');
		ClearSccs();
		ThumbToCp(wwCur,cp0, fFalse);
		UpdateWw(wwCur, fFalse);
		FEnrichSccBelow();
		for (i = 0; i < 30; i++)
			{
			ScrollUp(wwCur,12, 30);
			UpdateWw(wwCur, fFalse);
			CkSccs();
			}
		for (i = 0; i < 30; i++)
			{
			ScrollDown(wwCur,12, 30);
			UpdateWw(wwCur, fFalse);
			CkSccs();
			}
		}
#endif
}


/* %%Function:CkMwds %%Owner:BRADV */
CkMwds()
{
#ifdef REVIEW	/* REVIEW iftime: Update to new structs (rp) */
	extern struct MWD **mpmwhmwd[];
	extern struct WWD **mpwwhwwd[];
	extern struct MWD **hmwdCur;
	extern struct WWD **hwwdCur;
	extern int wwCur;
	extern int mwMac;

	struct MWD **hmwd;
	struct WWD *pwwd;
	struct WWD **hwwdOther;
	int mw, ww, iAssert;

	if (!vdbs.fCkMwd)
		return;
	iAssert = 1;
	Assert(wwCur != wwNil);
	iAssert = 2;
	Assert(hmwdCur == (*hwwdCur)->hmwd);

	for (mw = mwDocMin; mw < mwMac; mw++)
		{
		if ((hmwd = mpmwhmwd[mw]) != hNil)
			{
			ww = (*hmwd)->wwActivePane;
			iAssert = mw * 10 + 1;
			Assert(ww != wwNil);
			iAssert += 1;
			Assert(mpwwhwwd[ww]);
			iAssert += 1;
			Assert(hmwd == (pwwd = *mpwwhwwd[ww])->hmwd);
			/* if there is a split, the other pane should have the same parent */
			if (pwwd->fSplit)
				{
				iAssert += 1;
				Assert((hwwdOther = mpwwhwwd[pwwd->ww]));
				iAssert += 1;
				Assert((*hwwdOther)->hmwd == hmwd);
				}
			}
		}
#endif
}


#endif
