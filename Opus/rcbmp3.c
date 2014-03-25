/* R C B M P 3. C */
/*  CSCONST bitmaps  (CGA and Sigma versions)
*/

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "resource.h"
#include "debug.h"
#include "error.h"

csconst struct BMDS rgbmds[] =
{
#include "cotlpat.hb"
#include "cscrptpo.hg"
#include "crultgls.hg"
#include "crulmark.hg"
#include "cribbon.hg"
#include "chdr.hb"
#include "coutline.hg"
#include "cpageico.hb"
#include "cparalig.hg"

#include "sotlpat.hb"
#include "sscrptpo.hg"
#include "srultgls.hg"
#include "srulmark.hg"
#include "sribbon.hg"
#include "shdr.hb"
#include "soutline.hg"
#include "spageico.hb"
#include "sparalig.hg"
};



/* H  F R O M  I B M D S  3 */
/* %%Function:HFromIbmds3  %%Owner:peterj */
HANDLE HFromIbmds3(i)
int i;
{
	int cb = rgbmds[i].cb;
	HANDLE hbmp = NULL;
	struct BMDS FAR *pbmds;
	BYTE *pb;

#ifdef DCSRC
	CommSzNum(SzShared("HFromIbmds3: loading: "), i);
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
