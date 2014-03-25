#define REALWINCALLS
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "debug.h"


#ifdef PROTOTYPE
#include "debuggdi.cpt"
#endif /* PROTOTYPE */


/* %%Function:GetDCTR %%Owner:BRADV */
GetDCTR(hwnd)
HWND hwnd;
{
	HDC hdc;

	Assert(vdbs.fTraceGDI);

	TraceFuncW(SzShared("GetDC"),SzFrame("hwnd"),hwnd);
	if ((hdc = FRareNULL(reGetDC, GetDC(hwnd))) == NULL)
		TraceRet(SzShared("GetDC"),hdc);
	return hdc;
}



/* %%Function:SelectObjectTR %%Owner:BRADV */
SelectObjectTR(hdc,h)
HDC hdc;
HANDLE h;
{
	HANDLE hPrev;

	Assert(vdbs.fTraceGDI);
	TraceFuncWW(SzShared("SelectObject"),SzFrame("hdc"),hdc,SzFrame("h"),h);
	if ((hPrev = FRareNULL(reSelectObject,SelectObject(hdc,h))) == NULL)
		TraceRet(SzShared("SelectObject"),hPrev);
	return hPrev;
}


/* %%Function:SaveDCTR %%Owner:BRADV */
SaveDCTR(hdc)
HDC hdc;
{
	int ilevel;

	Assert(vdbs.fTraceGDI);
	TraceFuncW(SzShared("SaveDC"),SzFrame("hdc"),hdc);
	if ((ilevel = FRareNULL(reSaveDC, SaveDC(hdc))) == NULL)
		TraceRet(SzShared("SaveDC"),ilevel);
	return ilevel;
}


/* %%Function:RestoreDCTR %%Owner:BRADV */
RestoreDCTR(hdc,ilevel)
HDC hdc;
int ilevel;
{
	int fSucceed;

	Assert(vdbs.fTraceGDI);
	TraceFuncWW(SzShared("RestoreDC"),SzFrame("hdc"),hdc,
			SzFrame("ilevel"),ilevel);
	if (!(fSucceed = FRareNULL(reMiscGDI, RestoreDC(hdc,ilevel))))
		TraceRet(SzShared("RestoreDC"),fSucceed);
	return fSucceed;
}



/* %%Function:DeleteDCTR %%Owner:BRADV */
DeleteDCTR(hdc)
HDC hdc;
{
	int fSucceed;

	Assert(vdbs.fTraceGDI);
	TraceFuncW(SzShared("DeleteDC"),SzFrame("hdc"),hdc);
	if (!(fSucceed = FRareNULL(reMiscGDI, DeleteDC(hdc))))
		TraceRet(SzShared("DeleteDC"),fSucceed);
	return fSucceed;
}


/* %%Function:ReleaseDCTR %%Owner:BRADV */
ReleaseDCTR(hwnd, hdc)
HWND hwnd;
HDC hdc;
{
	int fSucceed;

	TraceFuncWW(SzShared("ReleaseDC"),SzFrame("hwnd"),hwnd,
			SzFrame("hdc"),hdc);
	if (!(fSucceed = FRareNULL(reMiscGDI, ReleaseDC(hwnd,hdc))))
		TraceRet(SzShared("ReleaseDC"),fSucceed);
	return fSucceed;
}


/* %%Function:PatBltTR %%Owner:BRADV */
PatBltTR( hdcDest, xpDest, ypDest, dxp, dyp, rop )
HDC hdcDest;
int xpDest, ypDest, dxp, dyp;
long rop;
{
	int fSucceed;

	Assert(vdbs.fTraceGDI);
	TraceFuncRgw( SzShared("PatBlt"),&rop,
			(sizeof(long)+4*sizeof(int)+sizeof(HDC))/sizeof(int) );
	if (!(fSucceed = FRareNULL(reMiscGDI, 
			PatBlt( hdcDest, xpDest, ypDest, dxp, dyp, rop ))))
		TraceRet(SzShared("PatBlt"),fSucceed);
	return fSucceed;
}


/* %%Function:BitBltTR %%Owner:BRADV */
BitBltTR( hdcDest, xpDest, ypDest, dxp, dyp, hdcSrc, xpSrc, ypSrc, rop )
HDC hdcDest, hdcSrc;
int xpDest, ypDest, dxp, dyp, xpSrc, ypSrc;
long rop;
{
	int fSucceed;

	Assert(vdbs.fTraceGDI);
	TraceFuncRgw( SzShared("BitBlt"),&rop,
			(sizeof(long)+6*sizeof(int)+2*sizeof(HDC))/sizeof(int) );
	if (!(fSucceed = FRareNULL(reMiscGDI, 
			BitBlt( hdcDest, xpDest, ypDest, dxp, dyp,
			hdcSrc, xpSrc, ypSrc, rop ))))
		TraceRet(SzShared("BitBlt"),fSucceed);
	return fSucceed;
}


/* %%Function:StretchBltTR %%Owner:BRADV */
StretchBltTR( hdcDest, xpDest, ypDest, dxp, dyp,
hdcSrc, xpSrc, ypSrc, dxpSrc, dypSrc, rop )
HDC hdcDest, hdcSrc;
int xpDest, ypDest, dxp, dyp, xpSrc, ypSrc, dxpSrc, dypSrc;
long rop;
{
	int fSucceed;

	Assert(vdbs.fTraceGDI);
	TraceFuncRgw( SzShared("StretchBlt"),&rop,
			(sizeof(long)+8*sizeof(int)+2*sizeof(HDC))/sizeof(int) );
	if (!(fSucceed = FRareNULL(reMiscGDI, 
			StretchBlt( hdcDest, xpDest, ypDest, dxp, dyp,
			hdcSrc, xpSrc, ypSrc, dxpSrc, dypSrc, rop ))))
		TraceRet(SzShared("StretchBlt"),fSucceed);
	return fSucceed;
}



/* %%Function:ExtTextOutTR %%Owner:BRADV */
ExtTextOutTR(hdc,xp,yp,eto,lprcOpaque,lpch, cch, lpdxp )
HDC hdc;
int xp, yp;
int eto;
struct RC far *lprcOpaque;
CHAR far *lpch;
int cch;
int far *lpdxp;
{
	int fSucceed;

	Assert(vdbs.fTraceGDI);
	TraceFuncRgw( SzShared("ExtTextOut"),&lpdxp,
			(3*sizeof(int far *)+4*sizeof(int)+sizeof(HDC))/sizeof(int) );
	if (!(fSucceed = FRareNULL(reMiscGDI, 
			ExtTextOut( hdc, xp, yp, eto, lprcOpaque, lpch, cch, lpdxp))))
		TraceRet(SzShared("ExtTextOut"),fSucceed);
	return fSucceed;
}


/* %%Function:ScrollDCTR %%Owner:BRADV */
ScrollDCTR(hdc,dxp,dyp,lprcT,lprcClip,hrgnUpdate,lprcUpdate)
HDC hdc;
int dxp, dyp;
struct RC far *lprcT;
struct RC far *lprcClip;
HRGN hrgnUpdate;
struct RC far *lprcUpdate;
{
	int fWhatsit;

	Assert(vdbs.fTraceGDI);
	TraceFuncRgw( SzShared("ScrollDC"),&lprcUpdate,
			(3*sizeof(int far *)+2*sizeof(int)+sizeof(HDC)+sizeof(HRGN))/sizeof(int) );
	if (!(fWhatsit = FRareNULL(reMiscGDI, 
			ScrollDC(hdc,dxp,dyp,lprcT,lprcClip,hrgnUpdate,lprcUpdate))))
		TraceRet(SzShared("ScrollDC"),fWhatsit);
	return fWhatsit;
}


/* %%Function:TraceFunc %%Owner:BRADV */
TraceFunc(szFunc)
CHAR szFunc[];
{
	CHAR szBuf [100];
	CHAR *pch;

	pch = &szBuf [CchCopySz( szFunc,szBuf ) ];
	*pch++ = '(';
	*pch++ = ')';
	*pch++ = '\r';
	*pch++ = '\n';
	*pch++ = '\0';

	CommSz(szBuf);
}


/* %%Function:TraceFuncW %%Owner:BRADV */
TraceFuncW(szFunc,szParm1,wParm1)
CHAR szFunc[],szParm1[];
WORD wParm1;
{
	CHAR szBuf [100];
	CHAR *pch;

	pch = &szBuf [CchCopySz( szFunc,szBuf ) ];
	*pch++ = '(';
	*pch++ = ' ';
	pch += CchCopySz(szParm1,pch);
	*pch++ = '=';
	CchIntToHexPpch( wParm1, &pch );
	*pch++ = ')';
	*pch++ = '\r';
	*pch++ = '\n';
	*pch++ = '\0';

	CommSz(szBuf);
}


/* %%Function:TraceFuncWW %%Owner:BRADV */
TraceFuncWW(szFunc,szParm1,wParm1,szParm2,wParm2)
CHAR szFunc[],szParm1[],szParm2[];
WORD wParm1,wParm2;
{
	CHAR szBuf [100];
	CHAR *pch;

	pch = &szBuf [CchCopySz( szFunc,szBuf ) ];
	*pch++ = '(';
	*pch++ = ' ';
	pch += CchCopySz(szParm1,pch);
	*pch++ = '=';
	CchIntToHexPpch( wParm1, &pch );
	*pch++ = ',';
	*pch++ = ' ';
	pch += CchCopySz(szParm2,pch);
	*pch++ = '=';
	CchIntToHexPpch( wParm2, &pch );
	*pch++ = ')';
	*pch++ = '\r';
	*pch++ = '\n';
	*pch++ = '\0';

	CommSz(szBuf);
}


/* %%Function:TraceFuncRgw %%Owner:BRADV */
TraceFuncRgw(szFunc,rgw,cw)
CHAR szFunc [];
int rgw[];
int cw;
{
	CHAR szBuf [200];
	CHAR *pch;
	int iw = cw - 1;

	pch = &szBuf [CchCopySz( szFunc,szBuf ) ];
	*pch++ = '(';
	while (cw--)
		{
		*pch++ = ' ';
		CchIntToHexPpch( rgw [iw--], &pch );
		if (pch >= &szBuf [sizeof(szBuf)-6])    break;
		}

	*pch++ = ')';
	*pch++ = '\r';
	*pch++ = '\n';
	*pch++ = '\0';

	CommSz(szBuf);
}


/* %%Function:TraceRet %%Owner:BRADV */
TraceRet(szFunc,wRet)
CHAR szFunc[];
WORD wRet;
{
	CHAR szBuf [100];
	CHAR *pch;

	pch = &szBuf [CchCopySz( SzShared("  --> returned "),szBuf) ];

	CchIntToHexPpch( wRet, &pch );
	*pch++ = '\r';
	*pch++ = '\n';
	*pch++ = '\0';

	CommSz(szBuf);
}




/* %%Function:CchIntToHexPpch %%Owner:BRADV */
int CchIntToHexPpch(n, ppch)
register uns n;
char **ppch;
{
	register int cch = 0;

	if (n >= 16)
		{
		cch += CchIntToHexPpch(n / 16, ppch);
		n %= 16;
		}

	*(*ppch)++ = n + (n < 10 ? '0' : ('A'-10));
	return cch + 1;
}


