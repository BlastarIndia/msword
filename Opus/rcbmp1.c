/* R C B M P 1. C */
/*  CSCONST bitmaps (EGA version)
*/

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "resource.h"
#include "debug.h"
#include "error.h"
#include "screen.h"

csconst struct BMDS rgbmds[] =
{
#include "eotlpat.hb"
#include "escrptpo.hg"
#include "erultgls.hg"
#include "erulmark.hg"
#include "eribbon.hg"
#include "ehdr.hb"
#include "eoutline.hg"
#include "epageico.hb"
#include "eparalig.hg"
};




/* H  F R O M  I B M D S  1 */

/* %%Function:HFromIbmds1 %%Owner:PETERJ */
HANDLE HFromIbmds1(i)
int i;
{
	int cb = rgbmds[i].cb;
	HANDLE hbmp = NULL;
	struct BMDS FAR *pbmds;
	BYTE *pb;

#ifdef DCSRC
	CommSzNum(SzShared("HFromIbmds1: loading: "), i);
#endif /* DCSRC */

/* Allocate a local frame variable of size cb */
	ReturnOnNoStack(cb, NULL, fFalse); /* assure there is room */
	pb = OurAllocA(cb);

	pbmds = &rgbmds[i]; /* pointer into CS space! */
	bltbx(pbmds->rgchBits, (LPSTR)pb, cb);

	hbmp = CreateBitmap (pbmds->bm.bmWidth, pbmds->bm.bmHeight,
			pbmds->bm.bmPlanes, pbmds->bm.bmBitsPixel, (LPSTR)pb);
	LogGdiHandle(hbmp, 20000);

	return hbmp;
}
