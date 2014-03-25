/* R C B M P 1. C */
/*  CSCONST bitmaps (EGA version for Win 3)
*/

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "resource.h"
#include "debug.h"
#include "error.h"
#include "screen.h"

#ifdef WIN23
extern struct SCI vsci;
csconst struct BMDS3 rgbmds3[] =
{
#include "otlpat3.hb"
#include "escrptp3.hb"
#include "eiscrpt3.hb"
#include "erultgl3.hb"
#include "eirultg3.hb"
#include "erulmar3.hb"
#include "eribbon3.hb"
#include "eiribbo3.hb"
#include "ehdr3.hb"
#include "eoutlin3.hb"
#include "epageic3.hb"
#include "eparali3.hb"
#include "eiparal3.hb"
};
csconst RGBTRIPLE rgbColors[] = 
	{
		0x00, 0x00, 0x00,
		0xff, 0xff, 0xff,
		0xff, 0x00, 0x00,
		0x00, 0x00, 0xff,
		0xff, 0x00, 0xff,
		0x00, 0xff, 0x00,
		0xff, 0xff, 0x00,
		0x00, 0xff, 0xff,
		0x40, 0x40, 0x40,
		0x80, 0x00, 0x00,
		0x00, 0x00, 0x80,
		0x80, 0x00, 0x80,
		0x00, 0x80, 0x00,
		0x80, 0x80, 0x00,
		0x00, 0x34, 0x78,
		0xc0, 0xc0, 0xc0
	};
/* H  F R O M  I B M D S 1 */
/* %%Function:HFromIbmds1  %%Owner:ricks */
HANDLE HFromIbmds13(hdc, i)
HDC hdc;
int i;
{
	int cbBits ;
	int cbInfo = sizeof(BITMAPCOREHEADER) + 16 * sizeof(RGBTRIPLE);
	HANDLE hbmp = NULL;
	struct BMDS3 FAR *pbmds;
	BYTE *pbBits, *pbInfo;
	
	pbmds = &rgbmds3[i]; /* pointer into CS space! */

	Assert(pbmds->bch.bcBitCount == 4 || i == 0);

	cbBits = pbmds->cb;

#ifdef DCSRC
	CommSzNum(SzShared("HFromIbmds13: loading: "), i);
#endif /* DCSRC */

#ifdef DCSRC
	CommSzNum(SzShared("HFromIbmds13: loading: "), i);
	CommSzNum(SzShared("HFromIbmds13: cbBits:"), cbBits);
#endif

/* Allocate a local frame variable of size cb for bits */
	ReturnOnNoStack(cbBits, NULL, fFalse); /* assure there is room */
	pbBits = OurAllocA(cbBits);

#ifdef DCSRC
	CommSzNum(SzShared("HFromIbmds13: cbInfo:"), cbInfo);
#endif

/* Allocate a local frame variable of size cColors * 3 for color table and info */
	ReturnOnNoStack(cbInfo, NULL, fFalse); 
	pbInfo = OurAllocA(cbInfo);

	pbmds = &rgbmds3[i]; /* pointer into CS space! (may have moved? ) */

	bltbx(pbmds->rgchBits, (LPSTR)pbBits, cbBits);

	bltbx(&pbmds->bch, (LPSTR)pbInfo, sizeof(BITMAPCOREHEADER));
	bltbx(rgbColors, (LPSTR)((LPSTR)pbInfo + sizeof(BITMAPCOREHEADER)), 16 * sizeof (RGBTRIPLE));	/* variable */

	hbmp = (*vsci.lpfnCreateDIBitmap)(hdc, (LPSTR)pbInfo, (LONG)CBM_INIT,
		(LPSTR)pbBits, (LPSTR)pbInfo, DIB_RGB_COLORS);
	LogGdiHandle(hbmp, 20000);

	return hbmp;
}

#endif /* WIN23 */

