/* U T I L . C */
#ifdef MAC /* no longer needed for win since util is included in clsplc.c */
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "doc.h"
#include "ch.h"
#include "disp.h"
#include "debug.h"
#include "heap.h"
#endif /* MAC */
#ifdef WIN
#include "props.h"
#include "screen.h"
#include "fontwin.h"
#include "field.h"

#ifdef PROTOTYPE
#include "util.cpt"
#endif /* PROTOTYPE */

extern struct FTI vfti;
#endif

/* E X T E R N A L S */
extern struct DOD **mpdochdod[];
extern struct WWD **mpwwhwwd[];
extern struct MWD **mpmwhmwd[];
extern struct FPC vfpc;
extern struct DBS vdbs;
extern struct SCI vsci;

struct WWD              **vhwwdOrigin;
struct DRF *vpdrfHead = NULL;
#ifdef DEBUG
struct DRF *vpdrfHeadUnused = NULL;
#endif /* DEBUG */


#ifdef WIN
#include "plc.c"
#endif /* WIN */

#ifdef MAC
#define WINDOWS
#define DIALOGS
#include "toolbox.h"
#include "print.h"
#include "printmac.h"
#include "format.h"
extern struct PRSU	vprsu;
extern struct FLI       vfli;
extern int		vdocHelp;
#endif

/***************************************************************
	D O D   S H O R T C U T S
***************************************************************/

#ifdef MAC /* in fetchn2.asm if WIN */
/* D O C  M O T H E R */
/* returns mother doc of a doc, doc itself if not short */
HANDNATIVE int DocMother(doc)
int doc;
{
	struct DOD *pdod;
	Assert(doc > 0 && doc < docMax && mpdochdod[doc]);

	if ((pdod = *mpdochdod[doc])->fShort)
		{
		Assert(pdod->doc > 0 && pdod->doc < docMax && 
				mpdochdod[pdod->doc]);
		return pdod->doc;
		}
	return doc;
}


#endif /* MAC */

#ifdef MAC /* in fetchn2.asm if WIN */
/* P D O D  M O T H E R */
/* returns mother dod of a doc, doc's dod if not short */
HANDNATIVE struct DOD *PdodMother(doc)
int doc;
{
	struct DOD *pdod;
	Assert(doc > 0 && doc < docMax && mpdochdod[doc]);

	if ((pdod = *mpdochdod[doc])->fShort)
		{
		Assert(pdod->doc > 0 && pdod->doc < docMax && 
				mpdochdod[pdod->doc]);
		return *mpdochdod[pdod->doc];
		}
	return pdod;
}


#endif /* MAC */


#ifdef MAC /* moved to RES.C for WIN */
/* D O C  B A S E  W W  */
/* returns mother doc of a window */
NATIVE int DocBaseWw(ww) /* WINIGNORE - pcode in WIN */
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


#endif /* MAC */


#ifdef WIN
#ifdef NOASM   /* in fetchn2.asm */
/* D O C  D O T  M O T H E R */
/*  returns the docDot of the mother of doc. */
HANDNATIVE int DocDotMother (doc)
int doc;
{
	struct DOD *pdod, **hdod = mpdochdod[doc];

	if (hdod == hNil) return docNil;

	if ((pdod = *hdod)->fDoc)
		return pdod->docDot;
	else  if (pdod->fDot)
		return doc;
	else
		return DocDotMother (pdod->doc);
}


#endif /* NOASM */
#endif /* WIN */

#ifdef MAC /* in resident.asm if WIN */
/* C P  M A C  D O C */
HANDNATIVE CP CpMacDoc(doc)
int doc;
{
	struct DOD *pdod = PdodDoc(doc);
	CP cp = pdod->cpMac - 2*ccpEop;

	return(cp);
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* C P  M A C 1  D O C */
HANDNATIVE CP CpMac1Doc(doc)
int doc;
{
	struct DOD *pdod = PdodDoc(doc);
	CP cp = pdod->cpMac - ccpEop;

	return(cp);
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* C P  M A C  D O C  E D I T*/
HANDNATIVE CP CpMacDocEdit(doc)
int doc;
{
	return(CpMacDoc(doc) - ccpEop);
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* C P  M A C 2  D O C  */
HANDNATIVE CP CpMac2Doc(doc)
int doc;
{
	return((*mpdochdod[doc])->cpMac);
}


#endif /* MAC */

#ifdef MAC /* Macro in word.h if WIN */
/* I P A D  M A C */
/* returns the ipadMac corresponding to CpMac, not CpMac2Doc */
NATIVE IpadMac(hplcpad) /* WINIGNORE - macro if WIN */
struct PLC **hplcpad;
{
	return IMacPlc(hplcpad) - 2;
}


#endif /* MAC */

/***************************************************************
	P C A    R O U T I N E S
***************************************************************/

#ifdef MAC /* in resident.asm if WIN */
/* P C A  S E T */
HANDNATIVE struct CA *PcaSet(pca, doc, cpFirst, cpLim)
struct CA *pca;
int doc;
CP cpFirst, cpLim;
{
	pca->doc = doc;
	pca->cpFirst = cpFirst;
	pca->cpLim = cpLim;
	return(pca);
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* P C A  S E T  D C P */
HANDNATIVE struct CA *PcaSetDcp(pca, doc, cpFirst, dcp)
struct CA *pca;
int doc;
CP cpFirst, dcp;
{
	pca->doc = doc;
	pca->cpFirst = cpFirst;
	pca->cpLim = cpFirst + dcp;
	return(pca);
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* D C P  P C A */
HANDNATIVE CP DcpCa(pca)
struct CA *pca;
{
	return(pca->cpLim - pca->cpFirst);
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* P C A  S E T  W H O L E  D O C */
HANDNATIVE struct CA *PcaSetWholeDoc(pca, doc)
struct CA *pca;
int doc;
{
	return PcaSet(pca, doc, cp0, CpMacDocEdit(doc));
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* P C A  S E T  N I L */
HANDNATIVE struct CA *PcaSetNil(pca)
struct CA *pca;
{
	return PcaSet(pca, docNil, cp0, cp0);
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* P C A  P O I N T */
HANDNATIVE struct CA *PcaPoint(pca, doc, cp)
struct CA *pca;
int doc;
CP cp;
{
	return PcaSet(pca, doc, cp, cp);
}


#endif /* MAC */

/***************************************************************
	C O M P A R E    R O U T I N E S
***************************************************************/

#ifdef MAC /* in formatn2.asm if WIN */
/* F  N E  R G W */
/* true iff rgw's are not equal */
HANDNATIVE int FNeRgw(rgw1, rgw2, cw)
int *rgw1, *rgw2, cw;
{
	int iw;
	for (iw = 0; iw < cw; iw++)
		if (rgw1[iw] != rgw2[iw])
			return fTrue;
	return fFalse;
}


#endif /* MAC */

#ifdef MAC /* in formatn2.asm if WIN */
/* F  N E  S T */
/* true iff st's are not equal */
HANDNATIVE int FNeSt(st1, st2)
char *st1, *st2;
{
	return(*st1 != *st2 || FNeRgch(st1 + 1, st2 + 1, *st1));
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* F  N E  R G C H */
/* true iff rgb's are not equal */
HANDNATIVE int FNeRgch(rgb1, rgb2, cb)
char *rgb1, *rgb2; 
int cb;
{
	int ib;
	for (ib = 0; ib < cb; ib++)
		if (rgb1[ib] != rgb2[ib])
			return fTrue;
	return fFalse;
}


#endif /* MAC */

#ifdef DEBUGORNOTWIN
/* coded in line in asm versions of FormatLineDxa & IbstFindSt if WIN */
/* F  N E  H P R G C H */
/* true iff rgb's are not equal */
NATIVE int FNeHprgch(hprgb1, hprgb2, cb) /* WINIGNORE - in-line if WIN */
char HUGE *hprgb1, HUGE *hprgb2; 
int cb;
{
	int ib;

	for (ib = 0; ib < cb; ib++)
		if (hprgb1[ib] != hprgb2[ib])
			return fTrue;
	return fFalse;
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN /* coded in line in formatn.asm if WIN */
/* F  N E  C H P */
/* true iff chp's are not equal (fs bits in 1st word are ignored */
HANDNATIVE int FNeChp(pchp1, pchp2) /* WINIGNORE - in-line if WIN */
int *pchp1, *pchp2;
{
	if (((*pchp1++ ^ *pchp2++) & ~maskFs) != 0)
		return fTrue;
	return FNeRgw(pchp1, pchp2, cwCHP - 1);
}


#endif /* DEBUGORNOTWIN */

/***************************************************************
	O T H E R    S T U F F
***************************************************************/

#ifdef MAC /* in fetchn2.asm if WIN */
/* F  I N  C A */
/* returns true iff doc,cp is in the cache ca */
HANDNATIVE int FInCa(doc, cp, pca)
int doc; 
CP cp; 
struct CA *pca;
{
#ifdef NATIVE_COMPILER_BUG
	/* when I compiled this cp was put into d1 violating the
	rule that the top half of d1 must remain 0 (bcv 1/11/87) */
	return (pca->doc == doc && pca->cpFirst <= cp &&
			cp < pca->cpLim);
#else
	if (pca->doc == doc)
		{
		if (pca->cpFirst <= cp && cp < pca->cpLim)
			return fTrue;
		}
	return fFalse;
#endif /* NATIVE_COMPILER_BUG */
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* U M A X */
HANDNATIVE uns umax(u1, u2)
uns u1, u2;
{
	return (u1 > u2) ? u1 : u2;
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* U M I N */
HANDNATIVE uns umin(u1, u2)
uns u1, u2;
{
	return (u1 < u2) ? u1 : u2;
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* C P  M A X */
HANDNATIVE CP CpMax(cp1, cp2)
CP cp1, cp2;
{
	return (cp1 > cp2) ? cp1 : cp2;
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* C P  M I N */
HANDNATIVE CP CpMin(cp1, cp2)
CP cp1, cp2;
{
	return (cp1 < cp2) ? cp1 : cp2;
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* M I N */
HANDNATIVE int min(x, y)
int x, y;
{
	return(x < y ? x : y);
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* M A X */
HANDNATIVE int max(x, y)
int x, y;
{
	return(x > y ? x : y);
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* A B S */
HANDNATIVE int abs(x)
int x;
{
	return(x < 0 ? -x : x);
}


#endif /* MAC */


#ifdef MAC /* in resident.asm if WIN */
/* C C H   N O N   Z E R O  P R E F I X  */
/* returns the cbMac reduced over the trailing zeroes in rgb. This is
the number of non-zero previx characters.
*/
HANDNATIVE int CchNonZeroPrefix(rgb, cbMac)
char *rgb;
int cbMac;
{
	for (rgb += cbMac - 1; cbMac > 0 && *rgb == 0; rgb--, cbMac--)
		;
	return(cbMac);
}


#endif /* MAC */

#ifdef MAC /* in fetchn2.asm if WIN */
/* F  I S E C T  I V A L */
/* returns true iff closed intervals have non empty intersection.
*/
HANDNATIVE int FIsectIval(pca, cpFirst, cpLim)
struct CA *pca; 
CP cpFirst, cpLim;
{
	return (pca->cpLim >= cpFirst && pca->cpFirst <= cpLim);
}


#endif /* MAC */

#ifdef MAC
/* F  K C M  I N T E R */
NATIVE int FKcmInter() /* WINIGNORE - unused in WIN */
{
/* this is just a stub in pcode; in NATIVE, this routine determines, for
	English versions, whether the keyboard is US or international, and returns
	fTrue for international. It does this by calling the low-memory routine
	Key1Trans */
	return(Debug(vdbs.fKcmInter ||) fFalse);
}


#endif /* MAC */

#ifdef DEBUGORNOTWIN /* done in line in selectsp.c if WIN */
/* F  O V E R L A P  C A */
/* returns true iff in the same doc and half open intervals overlap */
#ifdef MAC
NATIVE /* WINIGNORE - FOverlapCa - in-line if WIN */
#endif /* MAC */
FOverlapCa(pca1, pca2)
struct CA *pca1, *pca2;
{
	return pca1->doc == pca2->doc &&
			pca2->cpLim > pca1->cpFirst && pca2->cpFirst < pca1->cpLim;
}


#endif /* DEBUGORNOTWIN */


/************************ Rectangle Utilities *************************/

#ifdef MAC  /* in resident.asm if WIN */
/* D R C  T O  R C  - convert from drc-style rect into a rc-style rect */
HANDNATIVE DrcToRc(pdrc, prc)
struct DRC *pdrc;
struct RC *prc;
{
	prc->ptTopLeft = pdrc->ptTopLeft;
	prc->ypBottom = prc->ypTop + pdrc->dyp;
	prc->xpRight = prc->xpLeft + pdrc->dxp;
}


#endif /* MAC */

#ifdef MAC  /* in resident.asm if WIN */
/* R C  T O  D R C  - convert from rc-style rect into a drc-style rect */
HANDNATIVE RcToDrc(prc, pdrc)
struct RC *prc;
struct DRC *pdrc;
{
	pdrc->ptTopLeft = prc->ptTopLeft;
	pdrc->dxp = prc->xpRight - prc->xpLeft;
	pdrc->dyp = prc->ypBottom - prc->ypTop;
}


#endif /* MAC */

/* D Y  O F  R C  - return dy dimension of an rc */
#ifdef MAC
NATIVE /* WINIGNORE - DyOfRc - pcode in WIN */
#endif /* MAC */
DyOfRc(prc)
struct RC *prc;
{
	return prc->ypBottom - prc->ypTop;
}


/* D X  O F  R C  - return dx dimension of an rc */
#ifdef MAC
NATIVE /* WINIGNORE - DxpOfRc - pcode in WIN*/
#endif /* MAC */
DxOfRc(prc)
struct RC *prc;
{
	return prc->xpRight - prc->xpLeft;
}


#ifdef MAC  /* in resident.asm if WIN */
/* R C L  T O  R C W  - converts rect in frame coord. to rect in window coord */
HANDNATIVE RclToRcw(hpldr, prcl, prcw)
struct PLDR **hpldr;
struct RC *prcl;
struct RC *prcw;
{
	struct WWD *pwwd;
	int dxlToXw;
	int dylToYw;
	struct PT pt;

	pt = PtOrigin(hpldr, -1);

	pwwd = *vhwwdOrigin;
	dxlToXw = pwwd->rcePage.xeLeft + pwwd->xwMin + pt.xp;
	dylToYw = pwwd->rcePage.yeTop + pwwd->ywMin + pt.yp;

	prcw->xwLeft = prcl->xlLeft + dxlToXw;
	prcw->xwRight = prcl->xlRight + dxlToXw;
	prcw->ywTop = prcl->ylTop + dylToYw;
	prcw->ywBottom = prcl->ylBottom + dylToYw;
}


#endif /* MAC */

#ifdef MAC  /* in resident.asm if WIN */
/* R C W  T O  R C L  - converts rect in window coord. to rect in page coord */
HANDNATIVE RcwToRcl(hpldr, prcw, prcl)
struct PLDR **hpldr;
struct RC *prcw;
struct RC *prcl;
{
	struct WWD *pwwd;
	struct PT pt;
	int dxwToXl, dywToYl;

	pt = PtOrigin(hpldr,-1);
	pwwd = *vhwwdOrigin;
	dxwToXl = pwwd->rcePage.xeLeft + pwwd->xwMin + pt.xp;
	dywToYl = pwwd->rcePage.yeTop + pwwd->ywMin + pt.yp;

	prcl->xlLeft = prcw->xwLeft - dxwToXl;
	prcl->xlRight = prcw->xwRight - dxwToXl;
	prcl->ylTop = prcw->ywTop - dywToYl;
	prcl->ylBottom = prcw->ywBottom - dywToYl;
}


#endif /* MAC */

#ifdef DEBUGORNOTWIN /* in resident.asm if WIN */
/* P T  O R I G I N */
/* this procedure finds the origin in outermost l space of the p's in a frame
identified by hpldr,idr.
If idr == -1, it finds the origin for l's in hpldr.
The outermost containing hwwd is returned in vhwwdOrigin.

It is useful to remember:
	l's are frame relative coordinates of objects in frames, e.g.
		positions of dr's
	p's are object relative coordinates of parts of objects, e.g.
		lines of text, or frames of text.
	DR's are objects which may contain text and frames
	PLDR's are frames which can contain objects.
*/
#ifdef MAC
NATIVE struct PT PtOrigin(hpldr, idr) /* WINIGNORE - "C_" if WIN */
#else /* WIN */
HANDNATIVE struct PT C_PtOrigin(hpldr, idr)
#endif /* WIN */
struct PLDR **hpldr; 
int idr;
{
	struct PT pt;
	struct DR HUGE *hpdr;
#ifdef DEBUG
	struct DR *pdr;
	struct DRF drfFetch;
#endif /* DEBUG */

	pt.xp = pt.yp = 0;
	for (;;)
		{
		if (idr != -1)
			{
			Debug (pdr = PdrFetch(hpldr, idr, &drfFetch));
			hpdr = HpInPl(hpldr, idr);
			Assert (pdr->xl == hpdr->xl);
			Assert (pdr->yl == hpdr->yl);
			Debug (FreePdrf(&drfFetch));
			pt.xp += hpdr->xl;
			pt.yp += hpdr->yl;
			}
		if ((*hpldr)->hpldrBack == hNil)
			{
			vhwwdOrigin = hpldr;
			return pt;
			}
		pt.xp += (*hpldr)->ptOrigin.xp;
		pt.yp += (*hpldr)->ptOrigin.yp;
		idr = (*hpldr)->idrBack;
		hpldr = (*hpldr)->hpldrBack;
		}
}


#endif /* DEBUGORNOTWIN */


#ifdef MAC  /* in resident.asm if WIN */
/* R C W  T O  R C P */
/* Converts rc in dr coords into rc in window coords */
HANDNATIVE RcwToRcp(hpldr/* or hwwd*/, idr, prcw, prcp)
struct PLDR **hpldr;
struct RC *prcp, *prcw;
{
	struct PT pt;
	struct WWD *pwwd;
	int dxPToW, dyPToW;

	pt = PtOrigin(hpldr, idr);

	pwwd = *vhwwdOrigin;
	dxPToW = pwwd->rcePage.xeLeft + pwwd->xwMin + pt.xl;
	dyPToW = pwwd->rcePage.yeTop + pwwd->ywMin + pt.yl;

	prcp->xwLeft = prcw->xpLeft - dxPToW;
	prcp->xwRight = prcw->xwRight - dxPToW;
	prcp->ywTop = prcw->ywTop - dyPToW;
	prcp->ywBottom = prcw->ywBottom - dyPToW;
}


#endif /* MAC */

#ifdef MAC  /* in resident.asm if WIN */
/* R C P  T O  R C W */
/* Converts rc in dr coords into rc in window coords */
HANDNATIVE RcpToRcw(hpldr/* or hwwd*/, idr, prcp, prcw)
struct PLDR **hpldr;
struct RC *prcp, *prcw;
{
	struct PT pt;
	struct WWD *pwwd;
	int dxPToW, dyPToW;

	pt = PtOrigin(hpldr, idr);

	pwwd = *vhwwdOrigin;
	dxPToW = pwwd->rcePage.xeLeft + pwwd->xwMin + pt.xl;
	dyPToW = pwwd->rcePage.yeTop + pwwd->ywMin + pt.yl;

	prcw->xwLeft = prcp->xpLeft + dxPToW;
	prcw->xwRight = prcp->xwRight + dxPToW;
	prcw->ywTop = prcp->ywTop + dyPToW;
	prcw->ywBottom = prcp->ywBottom + dyPToW;
}


#endif /* MAC */

#ifdef MAC  /* in resident.asm if WIN */
/* R C E  T O  R C W  */
/* converts a page rectangle into window coordinates. */
HANDNATIVE RceToRcw(pwwd, prce, prcw)
struct WWD *pwwd;
struct RC *prce;
struct RC *prcw;
{
	int dxeToXw = pwwd->xwMin;
	int dyeToYw = pwwd->ywMin;

	prcw->xwLeft = prce->xeLeft + dxeToXw;
	prcw->xwRight = prce->xeRight + dxeToXw;
	prcw->ywTop = prce->yeTop + dyeToYw;
	prcw->ywBottom = prce->yeBottom + dyeToYw;
}


#endif /* MAC */

#ifdef MAC  /* in resident.asm if WIN */
/* D R C L  T O  R C W
*  Converts drc rect in page coord to rc rect in window coordinates */
/* Useful for getting a rcw from a dr.drcl */
HANDNATIVE DrclToRcw(hpldr/*or hwwd*/, pdrcl, prcw)
struct PLDR **hpldr;
struct DRC *pdrcl;
struct RC *prcw;
{
	struct RC rcl;
	DrcToRc(pdrcl, &rcl);
	RclToRcw(hpldr, &rcl, prcw);
}


#endif /* MAC */

#ifdef MAC  /* in resident.asm if WIN */
/* D R C P  T O  R C L
*  Converts drc rect in DR coordinates to rc rect in page coordinates
/* Useful for getting a rcl from a edl.drcp
/* WARNING - this translates to the outermost l coordinate system,
/* not necessarily the immediately enclosing l system. 'Tis
/* better to use the w coordinate system in most cases, since
/* it is unambiguous.
/**/
HANDNATIVE DrcpToRcl(hpldr, idr, pdrcp, prcl)
struct PLDR **hpldr;
int idr;
struct DRC *pdrcp;
struct RC *prcl;
{
	struct PT pt;
	pt = PtOrigin(hpldr, idr);

	prcl->xlLeft = pdrcp->xp + pt.xl;
	prcl->xlRight = prcl->xlLeft + pdrcp->dxp;
	prcl->ylTop = pdrcp->yp + pt.yl;
	prcl->ylBottom = prcl->ylTop + pdrcp->dyp;
}


#endif /* MAC */

#ifdef MAC  /* in resident.asm if WIN */
/* D R C P  T O  R C W
*  Converts drc rect in DR coordinates to rc rect in window coordinates */
/* Useful for getting a rcw from a edl.drcp */
HANDNATIVE DrcpToRcw(hpldr/*or hwwd*/, idr, pdrcp, prcw)
struct PLDR **hpldr;
int idr;
struct DRC *pdrcp;
struct RC *prcw;
{
	struct RC rcl;
	int dxlToXw;
	int dylToYw;
	struct WWD *pwwd;

	DrcpToRcl(hpldr, idr, pdrcp, &rcl);
/* native code note: vhwwdOrigin is known to have a 0 backpointer! */
	RclToRcw(vhwwdOrigin, &rcl, prcw);
}


#endif /* MAC */

#ifdef MAC
NATIVE XlFromXw(hpldr, xw) /* WINIGNORE - unused in WIN */
struct PLDR *hpldr;
{
	struct PT pt;
	int xl;

	pt = PtOrigin(hpldr, -1);
	xl = xw - pt.xl - (*vhwwdOrigin)->rcePage.xeLeft - (*vhwwdOrigin)->xwMin;
	return(xl);
}


#endif /* MAC */


#ifdef NOTUSED
NATIVE YlFromYw(hpldr, yw) /* WINIGNORE - unused in WIN */
struct PLDR *hpldr;
{
	struct PT pt;
	int yl;

	pt = PtOrigin(hpldr, -1);
	yl = yw - pt.yl - (*vhwwdOrigin)->rcePage.yeTop - (*vhwwdOrigin)->ywMin;
	return(yl);
}


#endif /* NOTUSED */

#ifdef MAC /* in resident.asm if WIN */
HANDNATIVE XwFromXl(hpldr, xl)
struct PLDR *hpldr;
{
	struct PT pt;
	int xw;

	pt = PtOrigin(hpldr, -1);
	xw = xl + pt.xl + (*vhwwdOrigin)->rcePage.xeLeft + (*vhwwdOrigin)->xwMin;
	return(xw);
}


#endif /* MAC */

#ifdef NOTUSED
/* Now coded in-line in rulerdrw.c for WIN */
NATIVE XwFromXe(hwwd, xe) /* WINIGNORE - coded in-line if WIN */
struct WWD **hwwd;
{
	return xe + (*hwwd)->xwMin;
}


#endif /* NOTUSED */

#ifdef NOTUSED
NATIVE XeFromXw(hwwd, xw) /* WINIGNORE - unused in WIN */
struct WWD **hwwd;
{
	return xw - (*hwwd)->xwMin;
}


#endif /* NOTUSED */

#ifdef MAC
NATIVE XeFromXl(hwwd, xl) /* WINIGNORE - unused in WIN */
struct WWD **hwwd;
{
	return xl + (*hwwd)->xeScroll;
}


#endif /* MAC */

#ifdef MAC
NATIVE XlFromXe(hwwd, xe) /* WINIGNORE - unused in WIN */
struct WWD **hwwd;
{
	return xe - (*hwwd)->xeScroll;
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
HANDNATIVE YwFromYl(hpldr, yl)
struct PLDR *hpldr;
{
	struct PT pt;
	int yw;

	pt = PtOrigin(hpldr, -1);
	yw = yl + pt.yl + (*vhwwdOrigin)->rcePage.yeTop + (*vhwwdOrigin)->ywMin;
	return(yw);
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
HANDNATIVE YwFromYp(hpldr/*hwwd*/, idr, yp)
struct PLDR **hpldr;
int idr;
int yp;
{
	struct PT pt;
	int yw;

	pt = PtOrigin(hpldr, idr);
	yw = yp + (*vhwwdOrigin)->rcePage.yeTop + (*vhwwdOrigin)->ywMin + pt.yl;
	return(yw);
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* Y P  F R O M  Y W */
HANDNATIVE YpFromYw(hpldr/*hwwd*/, idr, yw)
struct PLDR **hpldr;
int idr;
int yw;
{
	struct PT pt;
	int yp;

	pt = PtOrigin(hpldr, idr);
	yp = yw - (*vhwwdOrigin)->rcePage.yeTop - (*vhwwdOrigin)->ywMin - pt.yl;
	return(yp);
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* X W  F R O M  X P */
HANDNATIVE XwFromXp(hpldr/*hwwd*/, idr, xp)
struct PLDR **hpldr;
int idr;
int xp;
{
	struct PT pt;
	int xw;

	pt = PtOrigin(hpldr, idr);
	xw = xp + (*vhwwdOrigin)->rcePage.xeLeft + (*vhwwdOrigin)->xwMin + pt.xl;
	return(xw);
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* X P  F R O M  X W */
HANDNATIVE XpFromXw(hpldr/*hwwd*/, idr, xw)
struct PLDR **hpldr;
int idr;
int xw;
{
	struct PT pt;
	int xp;

	pt = PtOrigin(hpldr, idr);
	xp = xw - (*vhwwdOrigin)->rcePage.xeLeft - (*vhwwdOrigin)->xwMin - pt.xl;
	return(xp);
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* D X A  F R O M  D X P  - return xa value from xp */
HANDNATIVE DxaFromDxp(pwwd, dxp)
int dxp;
struct WWD *pwwd;
{
	struct MWD *pmwd = PmwdMw(pwwd->mw);

	return NMultDiv(dxp, dxaInch, WinMac(vfti.dxpInch, DxsSetDxsInch(pmwd->doc)));
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* D X P  F R O M  D X A  - return dxp value from dxa */
HANDNATIVE DxpFromDxa(pwwd, dxa)
int dxa;
struct WWD *pwwd;
{
	struct MWD *pmwd = PmwdMw(pwwd->mw);

	return NMultDiv(dxa, WinMac(vfti.dxpInch, DxsSetDxsInch(pmwd->doc)), dxaInch);
}


#endif /* MAC */


#ifdef MAC
/* X W  F R O M  X A */
NATIVE XwFromXa(hwwd, idr, xa) /* WINIGNORE - unused in WIN */
struct WWD **hwwd;
int idr, xa;
{
	return(XwFromXp(hwwd, idr, DxpFromDxa(*hwwd, xa)));
}


#endif /* MAC */


#ifdef NOTUSED
/* X A  F R O M  X W */
NATIVE XaFromXw(hwwd, idr, xw) /* WINIGNORE - unused in WIN */
struct WWD **hwwd;
int idr, xw;
{
	return(DxaFromDxp(*hwwd, XpFromXw(hwwd, idr, xw)));
}


#endif /* NOTUSED */


#ifdef MAC /* in resident.asm if WIN */
/* F  E M P T Y  R C  - return true if rc encloses Zero pixels */
HANDNATIVE FEmptyRc(prc)
struct RC *prc;
{
	return (prc->ypTop >= prc->ypBottom || prc->xpLeft >= prc->xpRight);
}


#endif /* MAC */


#ifdef MAC  /* in WIN we use a version which takes prcDest */
/* F  S E C T  R C */
/* Returns true if the rc's intersect, false otherwise */
/* (i.e. returns true if any pixels are in both rects.) */
NATIVE FSectRc(prc1, prc2) /* WINIGNORE - unused in WIN */
struct RC *prc1, *prc2;
{
	if (prc1->ypBottom <= prc2->ypTop ||
			prc1->xpRight <= prc2->xpLeft ||
			prc1->ypTop >= prc2->ypBottom ||
			prc1->xpLeft >= prc2->xpRight ||
			FEmptyRc(prc1) || FEmptyRc(prc2))  /* rarest */
		return fFalse;
	return fTrue;
}


#endif /* MAC */


#ifdef UNUSED /* not used in MacWord, in resident.asm if WIN */
/* S E C T  R C  - produce intersection of rc1 and rc2 in rcDest */
/* rcDest may be the same rectangle as either rc1 or rc2 */
HANDNATIVE SectRc(prc1, prc2, prcDest)
struct RC *prc1;
struct RC *prc2;
struct RC *prcDest;
{
	prcDest->ypTop = max(prc1->ypTop, prc2->ypTop);
	prcDest->ypBottom = min(prc1->ypBottom, prc2->ypBottom);
	prcDest->xpLeft = max(prc1->xpLeft, prc2->xpLeft);
	prcDest->xpRight = min(prc1->xpRight, prc2->xpRight);
/* handle disjoint rectangles correctly (above will produce "inverted" rect) */
	if (prcDest->xpLeft > prcDest->xpRight)
		prcDest->xpLeft = prcDest->xpRight;
	if (prcDest->ypTop > prcDest->ypBottom)
		prcDest->ypTop = prcDest->ypBottom;
	return (FEmptyPrc(prcDest));
}


#endif /* UNUSED */


#ifdef MAC /* Located in disp3.c (only caller) if WIN */
/* U N I O N  R C  - produce union of rc1 and rc2 in rcDest */
/* rcDest may be the same rectangle as either rc1 or rc2 */
NATIVE UnionRc(prc1, prc2, prcDest) /* WINIGNORE - pcode in WIN */
struct RC *prc1, *prc2, *prcDest;
{
	prcDest->ypTop = min(prc1->ypTop, prc2->ypTop);
	prcDest->ypBottom = max(prc1->ypBottom, prc2->ypBottom);
	prcDest->xpLeft = min(prc1->xpLeft, prc2->xpLeft);
	prcDest->xpRight = max(prc1->xpRight, prc2->xpRight);
}


#endif	/* MAC */


/* M O V E  R C  -  position top,left of rect at xp,yp */
#ifdef MAC
NATIVE /* WINIGNORE - MoveRc - pcode in WIN */
#endif /* MAC */
MoveRc(prc, xp, yp)
struct RC *prc;
{
	int dxp = xp - prc->xpLeft;
	int dyp = yp - prc->ypTop;
	prc->xpLeft += dxp;
	prc->xpRight += dxp;
	prc->ypTop += dyp;
	prc->ypBottom += dyp;
}


/***************************************************************
	W W D   S H O R T C U T S
***************************************************************/

#ifdef MAC /* in resident.asm if WIN */
/* P W W D  W W */
HANDNATIVE struct WWD *PwwdWw(ww)
{
	Assert(ww > wwNil && ww < wwMax && mpwwhwwd[ww]);
	return *mpwwhwwd[ww];
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* H W W D  W W */
HANDNATIVE struct WWD **HwwdWw(ww)
{
	Assert(ww > wwNil && ww < wwMax && mpwwhwwd[ww]);
	return mpwwhwwd[ww];
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* P M W D  M W */
HANDNATIVE struct MWD *PmwdMw(mw)
{
	Assert(mw > mwNil && mw < mwMax && mpmwhmwd[mw]);
	return *mpmwhmwd[mw];
}


#endif /* MAC */

#ifdef MAC /* in resident.asm if WIN */
/* P M W D  W W */
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


#endif /* MAC */

/* P D R  W W */
#ifdef MAC
NATIVE /* WINIGNORE - PdrWw - pcode in WIN */
#endif /* MAC */
struct DR *PdrWw(ww, idr)
{
#ifdef DEBUG
	struct WWD *pwwd = PwwdWw(ww);
	Assert(idr >= 0 && idr < pwwd->idrMac);
#endif
	Assert(vpdrfHead == NULL);
	return (&(*mpwwhwwd[ww])->rgdr[idr]);
}


#ifdef MAC /* done in line in curskeys.c if WIN */
/* W W  F R O M  H W W D */
/* may want to implement it as a field in wwd? */
NATIVE int WwFromHwwd(hwwd) /* WINIGNORE - in-line if WIN */
struct WWD **hwwd;
{
	int ww;
	for (ww = 0; ww < wwMax; ww++)
		if (mpwwhwwd[ww] == hwwd) return ww;
	return wwNil;
}


#endif /* MAC */


/***************************************************************
	P L D R   A C C E S S   R O U T I N E S
***************************************************************/

#define wLastDrf 0xABCD

#ifdef WIN 
#undef PdrFetchAndFree
#undef PdrFreeAndFetch
#undef PdrFetch
#undef FreePdrf
#define PdrFetchAndFree(hpldr, idr, pdrf) \
		C_PdrFetchAndFree(hpldr, idr, pdrf)
#define PdrFreeAndFetch(hpldr, idr, pdrf) \
		C_PdrFreeAndFetch(hpldr, idr, pdrf)
#define PdrFetch(hpldr, idr, pdrf) \
		C_PdrFetch(hpldr, idr, pdrf)
#define FreePdrf(pdrf) \
		C_FreePdrf(pdrf)
HANDNATIVE struct DR *C_PdrFetch();
#endif /* WIN */


#ifdef DEBUGORNOTWIN	/* in resident.asm if WIN */
HANDNATIVE struct DR *PdrFetchAndFree(hpldr, idr, pdrf)
struct PLDR **hpldr;
int idr;
struct DRF *pdrf;
{
	struct DR *pdr;

	pdr = PdrFetch(hpldr, idr, pdrf);
	if (pdrf->pdrfUsed == NULL)
		{
		Assert(vpdrfHead == pdrf);
		vpdrfHead = pdrf->pdrfNext;
		}
#ifdef DEBUG
	else
		{
		Assert(vpdrfHeadUnused == pdrf);
		vpdrfHeadUnused = pdrf->pdrfNext;
		}
#endif /* DEBUG */
	return pdr;
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN	/* in resident.asm if WIN */
HANDNATIVE struct DR *PdrFreeAndFetch(hpldr, idr, pdrf)
struct PLDR **hpldr;
int idr;
struct DRF *pdrf;
{

	FreePdrf(pdrf);
	return PdrFetch(hpldr, idr, pdrf);
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN	/* in resident.asm if WIN */
HANDNATIVE struct DR *PdrFetch(hpldr, idr, pdrf)
struct PLDR **hpldr;
int idr;
struct DRF *pdrf;
{
	struct DRF *pdrfList;

	Assert (hpldr != hNil);
	Assert (idr < (*hpldr)->idrMac);
#ifdef DEBUG
	for (pdrfList = vpdrfHeadUnused;
			pdrfList != NULL; pdrfList = pdrfList->pdrfNext)
		{
		Assert (pdrfList->wLast == wLastDrf);
		Assert (pdrfList != pdrf);
		}
#endif /* DEBUG */
	Debug (pdrf->wLast = wLastDrf);

	pdrf->hpldr = hpldr;
	pdrf->idr = idr;
	for (pdrfList = vpdrfHead; pdrfList != NULL; pdrfList = pdrfList->pdrfNext)
		{
		Assert (pdrfList->wLast == wLastDrf);
		Assert (pdrfList != pdrf);
		if (pdrfList->hpldr == hpldr && pdrfList->idr == idr)
			{
			pdrf->pdrfUsed = pdrfList;
#ifdef DEBUG
			pdrf->pdrfNext = vpdrfHeadUnused;
			vpdrfHeadUnused = pdrf;
#endif /* DEBUG */
			return &pdrfList->dr;
			}
		}
	bltbh(HpInPl(hpldr, idr), &pdrf->dr, sizeof(struct DR));
	pdrf->pdrfUsed = NULL;
	pdrf->pdrfNext = vpdrfHead;
	vpdrfHead = pdrf;
	if ((*hpldr)->fExternal && pdrf->dr.hplcedl)
		{
		pdrf->pplcedl = &(pdrf->dr.plcedl);
		pdrf->dr.hplcedl = &(pdrf->pplcedl);
		}
	return &pdrf->dr;
}


#endif /* DEBUGORNOTWIN */


#ifdef DEBUGORNOTWIN	/* in resident.asm if WIN */
HANDNATIVE FreePdrf(pdrf)
struct DRF *pdrf;
{
	struct DR *pdr;

	Assert (pdrf->wLast == wLastDrf);
	Debug (pdrf->wLast = 0);
	Assert (pdrf->pdrfUsed != NULL || vpdrfHead == pdrf);
	pdr = (pdrf->pdrfUsed == NULL ? &pdrf->dr : &pdrf->pdrfUsed->dr);
	bltbh(pdr, HpInPl(pdrf->hpldr, pdrf->idr), sizeof(struct DR));
	if (pdrf->pdrfUsed == NULL)
		{
		Assert(vpdrfHead == pdrf);
		vpdrfHead = pdrf->pdrfNext;
		}
#ifdef DEBUG
	else
		{
		Assert(vpdrfHeadUnused == pdrf);
		vpdrfHeadUnused = pdrf->pdrfNext;
		}
#endif /* DEBUG */
	Debug (pdrf->dr.hplcedl = NULL);
	Debug (pdrf->pplcedl = NULL);
}


#endif /* DEBUGORNOTWIN */


#ifdef MAC		/* in resident.asm if WIN */
NATIVE int HUGE *HpInPl(hplFoo, iFoo)
struct PL **hplFoo;
int iFoo;
{
	struct PL *pplFoo = *hplFoo;
	char      *rgFoo;
	char HUGE *hpFoo;

	Assert (iFoo >= 0 && iFoo < pplFoo->iMac);
	rgFoo = ((char *)pplFoo) + pplFoo->brgfoo;
	if (pplFoo->fExternal)
		hpFoo = ((char HUGE *)HpOfHq(*((HQ *)rgFoo)));
	else
		hpFoo = ((char HUGE *)rgFoo);
	return hpFoo + iFoo * pplFoo->cb;
}


#endif /* MAC */


/***************************************************************
				S T T B  R O U T I N E S
***************************************************************/

#ifdef MAC /* in resident.asm if WIN */
/* H P S T  F R O M  S T T B */
/* Return huge pointer to string i in hsttb */
NATIVE CHAR HUGE *HpstFromSttb(hsttb, i)
struct STTB **hsttb;
int i;
{
	struct STTB *psttb = *hsttb;

	AssertH(hsttb);
	Assert(i < (*hsttb)->ibstMac);

	if (!psttb->fExternal)
		return (CHAR HUGE *)((char *)(psttb->rgbst) + psttb->rgbst[i]);
	else
		{
		int HUGE *hprgbst = HpOfHq(psttb->hqrgbst);
		return (CHAR HUGE *)hprgbst + hprgbst[i];
		}
}


#endif /* MAC */


#ifdef MAC /* in resident.asm if WIN */
/* G E T  S T  F R O M  S T T B */
/* Fetch the contents of i in hsttb into st.  st must have sufficient
	space (up to cchMaxSt). */
NATIVE GetStFromSttb(hsttb, i, st)
struct STTB **hsttb;
int i;
CHAR *st;
{
	CHAR HUGE *hpst = HpstFromSttb(hsttb, i);
	if ((*hsttb)->fStyleRules && *hpst == 0xFF)
		st[0] = 0xFF;	    /* avoid GP-FAULTs */
	else
		bltbh(hpst, (CHAR HUGE *)st, *hpst+1);
}


#endif /* MAC */


#ifdef MAC /* in resident.asm if WIN */
FStcpEntryIsNull(hsttb, stcp)
struct STTB **hsttb;
int stcp;
{
	return(*(HpstFromSttb(hsttb, stcp)) == 255);
}


#endif /* MAC */


#ifdef MAC
/* D X S  S E T  D X S  I N C H */
NATIVE int DxsSetDxsInch(doc)
int doc;
{
	if (vprsu.prid != pridLaser)
		{
		struct DOD *pdodT = PdodMother(doc);
		if (doc == vdocHelp || pdodT->qqprr == 0 || (*(PRR far * far *) pdodT->qqprr)->PrInfo.iHRes != 80)
			vfli.dxuInch = 72;
		else
			vfli.dxuInch = 80;
		vfli.dxsInch = vfli.dxuInch;
		}
	return(vfli.dxsInch);
}


#endif /* MAC */

#ifdef WIN
/* F  H A S  F I E L D S */
BOOL FHasFields (doc)
int doc;
{
	struct PLC **hplcfld;

	if (mpdochdod[doc] == hNil)
		return fFalse;

	return ((hplcfld = PdodDoc (doc)->hplcfld) != hNil &&
			IMacPlc (hplcfld) > 0);
}


#endif /* WIN */


#ifdef WIN
int IMacPl( hpl )
struct PL **hpl;
{
	AssertH( hpl );
	return (*hpl)->iMac;
}


#endif /* WIN */


#ifdef WIN
/* H S Z  C R E A T E */
/* Creates a heap block containing sz */
char (**HszCreate(sz))[]
char sz[];
{
	char (**hsz)[];
	int cch = CchSz(sz);
	hsz = (char (**)[]) HAllocateCb(cch);
	if (hsz != hNil)
		bltb(sz, **hsz, cch);
	return hsz;
}


#endif /* WIN */


#ifdef WIN
#ifdef DEBUG
HANDNATIVE C_MiscPlcLoops(hplc, iFirst, iLim, pResult, wRoutine)
struct PLC **hplc;
int iFirst;
int iLim;
char *pResult;
int wRoutine;
{
	union 
		{
		struct PAD pad;
		struct PGD pgd;
		struct PHE phe;
		struct SED sed;
		struct FRD frd;
		struct PCD pcd;
		} foo;
	int ifoo;

	if (wRoutine == 0 /* SetPlcUnk */)
		{
		for (ifoo = iFirst; ifoo < iLim; ifoo++)
			{
			GetPlc(hplc, ifoo, &foo);
			foo.pad.fUnk = fTrue;
			PutPlcLast(hplc, ifoo, &foo);
			}
		}
	if (wRoutine == 1 /* NAutoFtn */)
		{
		for (ifoo = iFirst; ifoo < iLim; ifoo++)
			{
			GetPlc(hplc, ifoo, &foo);
			*((int *)pResult) += foo.frd.fAuto;
			}
		}
	if (wRoutine == 2 /* MarkAllReferencedFn */)
		{
		for (ifoo = iFirst; ifoo < iLim; ifoo++)
			{
			GetPlc(hplc, ifoo, &foo);
			Assert(foo.pcd.fn < fnMax);
			Assert(fnNil == 0);
			pResult[foo.pcd.fn] = fTrue;
			}
		}
}


#endif /* DEBUG */
#endif /* WIN */


#ifdef	MAC
/***************************/
native long LpInitUtil(lpmh4) /* WINIGNORE - unused in WIN */
char far *lpmh4;
{
/* fixed segment initialization routine */
	return(0L);
}


/******
	WARNING: LpInitXxxx must be the last routine in this module
	because it is a fixed segment
******/
#endif	/* MAC */
