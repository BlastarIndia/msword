/* R C B M P 2. C */
/*  CSCONST bitmaps  (VGA version)
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
#include "votlpat.hb"
#include "vscrptpo.hg"
#include "vrultgls.hg"
#include "vrulmark.hg"
#include "vribbon.hg"
#include "vhdr.hb"
#include "voutline.hg"
#include "vpageico.hb"
#include "vparalig.hg"
};



/* H  F R O M  I B M D S  2 */
/* %%Function:HFromIbmds2  %%Owner:peterj */
HANDLE HFromIbmds2(i)
int i;
{
	int cb = rgbmds[i].cb;
	HANDLE hbmp = NULL;
	struct BMDS FAR *pbmds;
	BYTE *pb;

#ifdef DCSRC
	CommSzNum(SzShared("HFromIbmds2: loading: "), i);
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
