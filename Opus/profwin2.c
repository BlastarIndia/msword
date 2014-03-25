/* P R O F W I N . C */


#ifdef HYBRID

#define REALWINCALLS
#include "word.h"
#include "debug.h"
#include "lmem.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"

/* %%Function:YieldPR %%Owner:BRADV */
NATIVE BOOL YieldPR() /* WINIGNORE - PROFILE only */
{
	return Yield();
}


/* %%Function:GlobalWirePR %%Owner:BRADV */
NATIVE LPSTR GlobalWirePR(P1) /* WINIGNORE - PROFILE only */
HANDLE P1;
{
	return GlobalWire(P1);
}


/* %%Function:DestroyMenuPR %%Owner:BRADV */
NATIVE BOOL DestroyMenuPR(P1) /* WINIGNORE - PROFILE only */
HMENU P1;
{
	return DestroyMenu(P1);
}


/* %%Function:AddFontResourcePR %%Owner:BRADV */
NATIVE short AddFontResourcePR(P1) /* WINIGNORE - PROFILE only */
LPSTR P1;
{
	return AddFontResource(P1);
}


/* %%Function:MoveWindowPR %%Owner:BRADV */
NATIVE void MoveWindowPR(P1,P2,P3,P4,P5,P6) /* WINIGNORE - PROFILE only */
HWND P1; 
int P2; 
int P3; 
int P4; 
int P5; 
BOOL P6;
{
	MoveWindow(P1,P2,P3,P4,P5,P6);
}


/* %%Function:LoadBitmapPR %%Owner:BRADV */
NATIVE HBITMAP LoadBitmapPR(P1,P2) /* WINIGNORE - PROFILE only */
HANDLE P1;
LPSTR P2;
{
	return LoadBitmap(P1,P2);
}


/* %%Function:DefWinProcPR %%Owner:BRADV */
NATIVE long DefWinProcPR(P1,P2,P3,P4) /* WINIGNORE - PROFILE only */
HWND P1;
unsigned P2;
WORD P3;
LONG P4;
{
	return DefWindowProc(P1,P2,P3,P4);
}


/* %%Function:SetTextColorPR %%Owner:BRADV */
NATIVE DWORD SetTextColorPR(P1,P2) /* WINIGNORE - PROFILE only */
HDC P1;
DWORD P2;
{
	return SetTextColor(P1,P2);
}


/* %%Function:SetBkColorPR %%Owner:BRADV */
NATIVE DWORD SetBkColorPR(P1,P2) /* WINIGNORE - PROFILE only */
HDC P1;
DWORD P2;
{
	return SetBkColor(P1,P2);
}


/* %%Function:DestroyWindowPR %%Owner:BRADV */
NATIVE BOOL DestroyWindowPR(P1) /* WINIGNORE - PROFILE only */
HWND P1;
{
	return DestroyWindow(P1);
}


/* %%Function:SetWindowPosPR %%Owner:BRADV */
NATIVE void SetWindowPosPR(P1,P2,P3,P4,P5,P6,P7) /* WINIGNORE - PROFILE only */
HWND P1;
HWND P2;
int P3;
int P4;
int P5;
int P6;
WORD P7;
{
	SetWindowPos(P1,P2,P3,P4,P5,P6,P7);
}


/* %%Function:SetBkModePR %%Owner:BRADV */
NATIVE SetBkModePR(P1,P2) /* WINIGNORE - PROFILE only */
HDC P1;
short P2;
{
	return SetBkMode(P1,P2);
}


/* %%Function:SetMapperFlagsPR %%Owner:BRADV */
NATIVE DWORD SetMapperFlagsPR(P1,P2) /* WINIGNORE - PROFILE only */
HDC P1;
DWORD P2;
{
	return SetMapperFlags(P1,P2);
}

#endif /* HYBRID */
