/* R C I N I T . C */
/*  CSCONST resources used during initialization
*/

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "resource.h"
#include "debug.h"
#include "error.h"

csconst struct RCDS rgrcds[] =
{
#ifdef DEBUG
#include "mopus.hi"
#else
#include "mword.hi"
#endif /* DEBUG */

#include "mwhires.hc"
#include "split.hc"
#include "column.hc"
};


/* H  L O A D  O U R   R E S O U R C E */
/* %%Function:HLoadRes0  %%Owner:peterj */
HANDLE HLoadRes0(i)
int i;
{
	HANDLE h;
	LPSTR lpch;

	Assert(i < ircdsNonCoreMin);

	if ((h = OurGlobalAlloc(GMEM_MOVEABLE, (LONG)rgrcds[i].cb)) == NULL)
		return NULL;

	lpch = GlobalLock(h);
	bltbx((LPSTR)rgrcds[i].rgchBits, lpch, rgrcds[i].cb);
	GlobalUnlock(h);
	return h;
}



#ifdef WIN23
/* REVIEW DIB? */
csconst struct BMDS rgbmds[] =
{
#include "showvis.hb"
};
#else
csconst struct BMDS rgbmds[] =
{
#include "showvis.hb"
};
#endif /* WIN23 */


/* H  F R O M  I B M D S  0 */
/* %%Function:HFromIbmds0  %%Owner:peterj */
HANDLE HFromIbmds0(i)
int i;
{
	int cb = rgbmds[i].cb;
	HANDLE hbmp = NULL;
	struct BMDS FAR *pbmds;
	BYTE *pb;

#ifdef DCSRC
	CommSzNum(SzShared("HFromIbmds0: loading: "), i);
#endif /* DCSRC */

/* Allocate a local frame variable of size cb */
	ReturnOnNoStack(cb, NULL, fFalse); /* assure there is room */
	pb = OurAllocA(cb);

	pbmds = &rgbmds[i]; /* pointer into CS space! */
	bltbx(pbmds->rgchBits, (LPSTR)pb, cb);

	hbmp = CreateBitmap (pbmds->bm.bmWidth, pbmds->bm.bmHeight,
			pbmds->bm.bmPlanes, pbmds->bm.bmBitsPixel, (LPSTR)pb);
	LogGdiHandle(hbmp, 1037);

	return hbmp;
}


