/* R C B M P 4 3. C */
/*  CSCONST bitmaps  (8514/A versions for Win 3)
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
#include "8scrptp3.hb"
#include "8iscrpt3.hb"
#include "8rultgl3.hb"
#include "8irultg3.hb"
#include "8rulmar3.hb"
#include "8ribbon3.hb"
#include "8iribbo3.hb"
#include "8hdr3.hb"
#include "8outlin3.hb"
#include "8pageic3.hb"
#include "8parali3.hb"
#include "8iparal3.hb"
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

/* H  F R O M  I B M D S  4 */
/* %%Function:HFromIbmds4  %%Owner:ricks */
HANDLE HFromIbmds43(hdc, i)
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
	CommSzNum(SzShared("HFromIbmds4: loading: "), i);
#endif /* DCSRC */

#ifdef DBSRC
	CommSzNum(SzShared("HFromIbmds4: loading: "), i);
	CommSzNum(SzShared("HFromIbmds4: cbBits:"), cbBits);
#endif

/* Allocate a local frame variable of size cb for bits */
	ReturnOnNoStack(cbBits, NULL, fFalse); /* assure there is room */
	pbBits = OurAllocA(cbBits);

#ifdef DBSRC
	CommSzNum(SzShared("HFromIbmds4: cbInfo:"), cbInfo);
#endif

/* Allocate a local frame variable of size cColors * 3 for color table and info */
	ReturnOnNoStack(cbInfo, NULL, fFalse); 
	pbInfo = OurAllocA(cbInfo);

	pbmds = &rgbmds3[i]; /* pointer into CS space! (may have moved? ) */

	bltbx(pbmds->rgchBits, (LPSTR)pbBits, cbBits);

	bltbx(&pbmds->bch, (LPSTR)pbInfo, sizeof(BITMAPCOREHEADER));
	bltbx(rgbColors, (LPSTR)((LPSTR)pbInfo + sizeof(BITMAPCOREHEADER)), 16 * sizeof (RGBTRIPLE));	/* variable */
	vsci.lpfnCreateDIBitmap = GetProcAddress(GetModuleHandle(SzFrame("GDI")),
				MAKEINTRESOURCE(442));
	hbmp = (*vsci.lpfnCreateDIBitmap)(hdc, (LPSTR)pbInfo, (LONG)CBM_INIT,
		(LPSTR)pbBits, (LPSTR)pbInfo, DIB_RGB_COLORS);
	LogGdiHandle(hbmp, 20000);

	return hbmp;
}

#endif /* WIN23 */


