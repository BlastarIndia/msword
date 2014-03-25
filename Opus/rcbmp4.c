/* R C B M P 4. C */
/*  CSCONST bitmaps  (8514/A versions)
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
#include "8otlpat.hb"
#include "8scrptpo.hg"
#include "8rultgls.hg"
#include "8rulmark.hg"
#include "8ribbon.hg"
#include "8hdr.hb"
#include "8outline.hg"
#include "8pageico.hb"
#include "8paralig.hg"
};



/* H  F R O M  I B M D S  4 */
/* %%Function:HFromIbmds4  %%Owner:peterj */
HANDLE HFromIbmds4(i)
int i;
{
	int cb = rgbmds[i].cb;
	HANDLE hbmp = NULL;
	struct BMDS FAR *pbmds;
	BYTE *pb;

#ifdef DCSRC
	CommSzNum(SzShared("HFromIbmds4: loading: "), i);
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


