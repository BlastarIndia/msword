/* R C B M P 2 3. C */
/*  CSCONST bitmaps  (VGA version for Win 3)
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
#include "vscrptp3.hb"
#include "viscrpt3.hb"
#include "vrultgl3.hb"
#include "virultg3.hb"
#include "vrulmar3.hb"
#include "vribbon3.hb"
#include "viribbo3.hb"
#include "vhdr3.hb"
#include "voutlin3.hb"
#include "vpageic3.hb"
#include "vparali3.hb"
#include "viparal3.hb"
};
csconst RGBTRIPLE rgbColors[] = 
	{	0x00, 0x00, 0x00,
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
/* H  F R O M  I B M D S  2 */
/* %%Function:HFromIbmds2  %%Owner:ricks */
HANDLE HFromIbmds23(hdc, i)
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
	CommSzNum(SzShared("HFromIbmds23: loading: "), i);
#endif /* DCSRC */

#ifdef DCSRC
	CommSzNum(SzShared("HFromIbmds23: loading: "), i);
	CommSzNum(SzShared("HFromIbmds23: cbBits:"), cbBits);
#endif

/* Allocate a local frame variable of size cb for bits */
	ReturnOnNoStack(cbBits, NULL, fFalse); /* assure there is room */
	pbBits = OurAllocA(cbBits);

#ifdef DCSRC
	CommSzNum(SzShared("HFromIbmds23: cbInfo:"), cbInfo);
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


