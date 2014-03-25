/* **************************************************************************
**
**      COPYRIGHT (C) 1985 MICROSOFT
**
** **************************************************************************
*
*  Module: statline.c - routines used for handling the status line.
*
*
**
** REVISIONS
**
** Date         Who Rel Ver     Remarks
**
** 9/86         rp              Completely revamped.
**
** *********************************************************************** */

#define NOVIRTUALKEYCODES
#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOBRUSH
#define NOPEN
#define NOFONT
#define NOWNDCLASS
#define NOMB
#define NOMETAFILE
#define NOWH
#define NOWINOFFSETS
#define NOICON
#ifndef WIN23
/* so resource.h will compile */
#define NOBITMAP
#endif /* WIN23 */
#define NOCOMM
#define NOSOUND

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "props.h"
#include "sel.h"
#include "doc.h"
#include "disp.h"
#include "screen.h"
#include "print.h"
#define STATLINE
#include "status.h"
#include "layout.h"
#include "debug.h"
#include "format.h"
#include "keys.h"
#include "prompt.h"
#include "inter.h"
#include "border.h"
#include "opuscmd.h"
#include "help.h"       /* For def of cxtStatusBar */
#include "rareflag.h"
#ifdef WIN23
#include "resource.h"
#endif /* WIN23 */

extern HANDLE  vhInstance;
extern HWND  vhwndApp;
extern HWND  vhwndDeskTop;
extern BOOL  vfInitializing;
extern struct WWD  **hwwdCur;
extern struct SCI  vsci;
extern struct SEL  selCur;
extern struct PREF vpref;
extern struct FTI  vfti;
extern struct LCB  vlcb;
extern struct CA caPage;
extern struct CA caPara;
extern struct FLI vfli;
extern int wwCur;
extern BOOL vfOvertypeMode;
extern BOOL vfExtendSel;
extern BOOL vfBlockSel;
extern struct MERR vmerr;
extern int vflm;
extern struct LCB vlcb;
extern HWND        vhwndPrompt;
extern long        vusecTimeoutPrompt;
extern BOOL        vfRestPmtOnInput;
extern CHAR        vstPrompt[];
extern int         vpisPrompt;
extern struct PIR *vppirPrompt;
extern int         vcchStatLinePrompt;
extern int         vlm;
extern BOOL        vfHelp;
extern long        vcmsecHelp;
extern struct TCC       vtcc;
extern struct CA	caTap;
extern struct PAP       vpapFetch;
extern HWND	vhwndCBT;
#ifdef WIN23
extern int          vdbmgDevice;
extern HBRUSH vhbrDkGray;
extern HBRUSH vhbrLtGray;
extern HBRUSH vhbrGray;
int vdxpDigit;
#endif /* WIN23 */
/* GLOBALS */

struct LCB vlcb;            /* Line Cache Block */
struct SLS vsls;  /* current state of the status line */
HWND vhwndStatLine;
#ifdef WIN23
SLM rgslm[isiMax3];			/* status line measurements (dxp values) */
HFONT hfontStatLine;		/* Helv 12 pt. font for status line */
#endif /* WIN23 */
/*  table of unit names used in util2.c, prvw2.c. Order 
	must be same as ut* order (utInch = 0, utPt = 1, etc.) in disp.h

	INTL note: this table is now used to drive FzaFromSs. The way comparisons
	are done, no entry in the table can be a suffix of another unless it
	comes before it in the table. E.g., p and tp will not work, unless
	tp is first. This is because we go through the table in order, comparing
	the # of chars in the table entry against the end of the string. bz
*/
CHAR    *mputsz[utMax] =
{	
	SzGlobalKey("\"",Inch),               /* szInchDef */
	SzGlobalKey("pt",VarsPoint),	      /* szPtDef */
	SzGlobalKey("cm",Centimeter),         /* szCmDef */
	SzGlobalKey("pi",Picas),              /* szPicasDef */
	SzGlobalKey("p10",TenPoint),          /* szP10Def */
	SzGlobalKey("p12",TwelvePoint),       /* szP12Def */
	SzGlobalKey("li",VarsLine)	 	     /* szLineDef */

#ifdef INTL	 /* FUTURE make sure wordwin.h, dialog3 tables updated */
	,SzGlobalKey("in",Inch2)	             /* szInch2Def */
#endif /* INTL */

};


/* ****
*
	Module:StatLineWndProc
*  Author:
*  Copyright: Microsoft 1985
*  Date:
*
*  Description:
*     Window procedure for status line
*
*  Input:
*
*  Output:
*
*  Side Effects:
*
*
** ***/

/*  NOTE: Messages bound for this WndProc are filtered in wprocn.asm */

/* %%Function:StatLineWndProc %%Owner:bryanl */
EXPORT long FAR PASCAL StatLineWndProc(hwnd, message, wParam, lParam)
HWND      hwnd;
unsigned  message;
WORD      wParam;
LONG      lParam;
{
	int isi;
	struct RC rc, rcDummy;
	extern HCURSOR vhcArrow;
	PAINTSTRUCT ps;
	struct SLS sls;
#ifdef WIN23
	short dxpMin, dxp, dyp;
 	HFONT hfontOld;
 	HDC hdc;
#endif /* WIN23 */

	switch (message)
		{
	case WM_CREATE:
		if (vhwndCBT)
			/* this must be the first thing we do under WM_CREATE */
			SendMessage(vhwndCBT, WM_CBTNEWWND, hwnd, 0L);
#ifdef WIN23
		if (vsci.fWin3)
			{
			if (hdc = GetDC( hwnd ))
		 		{
				InitStatLine( hdc );
	 			ReleaseDC( hwnd, hdc );
				}
			else
				return(0L);
			}
#endif /* WIN23 */
		break;

	case WM_LBUTTONDOWN:
		if (vfHelp)
			GetHelp(cxtStatusBar);
		break;

	case WM_LBUTTONDBLCLK:
		if (GetMessageTime() < vcmsecHelp)
			return(fTrue);
		CmdExecBcmKc(bcmGoto, kcNil);
		break;

	case WM_PAINT:
#ifdef WIN23
		if (vsci.fWin3)
			{
			if (selCur.fUpdateStatLine && FGetStatLineState(&sls,usoNormal))
				FUpdateSls(&sls,usoNormal);	/* update data before painting it */

			hdc = BeginPaint(hwnd, (LPPAINTSTRUCT) &ps);
			AssertDo(FSetDcAttribs( hdc, vsci.dccStatLine ));
 			/* Select 12 pt Helv and LtGrey */
			if (hfontStatLine)
	 			hfontOld = SelectObject( hdc, hfontStatLine );
			if (vsci.fWin3Visuals)
				{
 				SetBkColor( hdc, vdbmgDevice != dbmgEGA3  ? rgbLtGray: rgbGray);
				SelectObject( hdc, vdbmgDevice != dbmgEGA3  ? vhbrLtGray : vhbrGray);
				}

			if (vcchStatLinePrompt) {
				GetClientRect( vhwndStatLine, &rc );
				if (vsci.fWin3Visuals)
					{
					PatBlt( hdc, 0, 0, rc.xpRight, 1, WHITENESS );
					rc.ypTop = 1;
					}
				/* Give a small dx margin and try to center the text in dy */
				ExtTextOut( hdc, xpPromptLeft, ypPromptTop,
					vsci.fWin3Visuals ? NULL : ETO_OPAQUE, &rc, 
							&vstPrompt[1], vstPrompt[0], NULL );
			}
			else {
				for (isi = 0 ; isi < isiMax3; isi++ ) {
					SetRectIsi( isi, &rc );

					/* check to see if this field is invalid */
					if (FSectRc((struct RC *) &ps.rcPaint, &rc, &rcDummy))
						PaintSi3( hdc, isi, &rc );
				}
				if (vsci.fWin3Visuals)
					{
					/* 
			 		* Draw a white line at y=0 across entire status line,
			 		* leave background for y=1, a dark gray line for the
			 		* top of the recessions at y=2, and a white line 
			 		* at the bottom of the recessions.
			 		*/
					GetClientRect( vhwndStatLine, &rc );
					dyp = rc.ypBottom - 2;
					SelectObject( hdc, vdbmgDevice == dbmgEGA3  ? vhbrDkGray : vhbrGray );

					PatBlt( hdc, 0, 0, rc.xpRight, 1, WHITENESS );	
					PatBlt( hdc, dxpSep0Gray, 2, 
							(dxp = rgslm[isiSep13].dxpOff - dxpSep0Gray), 1, PATCOPY );
					PatBlt( hdc, dxpSep0Gray, dyp, dxp, 1, WHITENESS );
			
					PatBlt( hdc, (dxpMin = rgslm[isiSep13].dxpOff + dxpSepGray2),
							2, (dxp = rgslm[isiSep23].dxpOff - dxpMin), 1, PATCOPY );
					PatBlt( hdc, dxpMin, dyp, dxp + 1, 1, WHITENESS );

					/* 
			 		* Draw the tops and bottoms of the depressions for the toggles.  
			 		* Brush is already gray. Inc by 2 to jump over the space fields.
			 		* Toggle space looks like this:
			 		*   W---BW---G------EXT--W----G--col--W----G-- . . .
			 		*   ^..............^   ^........^   ^........^
			 		*        Sep2			 SepToggle    SepToggle
			 		*/
					for( isi = isiExBl3; isi < isiLast3; isi += 2 ) {
						PatBlt( hdc, (dxpMin = rgslm[isi].dxpOff - dxpSepToggleEnd),
								2, (dxp = rgslm[isi].dxpFld + dxpSepToggleEnd2), 
								1, PATCOPY );
						PatBlt( hdc, dxpMin, dyp, dxp, 1, WHITENESS );
					}
					SelectObject( hdc, vdbmgDevice != dbmgEGA3  ? vhbrLtGray : vhbrGray);
					}
			}
 			/* save off the Helv font */
			if (hfontStatLine && hfontOld)
 				SelectObject( hdc, hfontOld ); 
			EndPaint(hwnd, (LPPAINTSTRUCT)&ps);
			}
		else
			{
#endif /* WIN23 */
		if (selCur.fUpdateStatLine && FGetStatLineState(&sls,usoNormal))
			FUpdateSls(&sls,usoNormal);	/* update data before painting it */
		BeginPaint(hwnd, (LPPAINTSTRUCT) &ps);
#ifdef WIN23
		if (FSetDcAttribs (ps.hdc, vsci.dccStatLine ))
#else
		if (FSetDcAttribs (ps.hdc, dccStatLine ))
#endif /* WIN23 */

			{
			isi = 0;
			if (vcchStatLinePrompt)
				{
				while (rgsi [isi].dch < vcchStatLinePrompt)
					{
					isi++;
					Assert( isi < isiMax );
					}
				GetClientRect( vhwndStatLine, &rc );
				rc.xpRight = vcchStatLinePrompt * vsci.dxpTmWidth;
				StartUMeas(umStatlineTextOut);
				ExtTextOut( ps.hdc, 0, 0, ETO_OPAQUE,
						&rc, &vstPrompt [1],
						vstPrompt [0], NULL );
				StopUMeas(umStatlineTextOut);
				}

			for ( ; isi < isiLast; isi++ )
				{
				SetRectIsi( isi, &rc );

				/* check to see if this field is invalid */
				if (FSectRc((struct RC *) &ps.rcPaint, &rc, &rcDummy))
					PaintSi( ps.hdc, isi );
				}
			}
		EndPaint(hwnd, (LPPAINTSTRUCT)&ps);
#ifdef WIN23
		}
#endif /* WIN23 */
		break;

#ifdef DEBUG
	default:
		Assert(fFalse);
		return(DefWindowProc(hwnd, message, wParam, lParam));
#endif
		}
	return (0L);
}




/*
*  PaintSi
*
*  Fills in a field of the status line, a field being any uniquely defined
*  area of the status line, including static text, a numerical field, a
*  separator, or variable (indexed) text.
*
*  PaintSi should only be called when a field NEEDS to be updated (i.e. it
*  has changed, or the rectangle it was in became invalid).  The caller
*  should take care of this detail.
*/

/* %%Function:PaintSi %%Owner:bryanl */
PaintSi( hdc, isi )
HDC hdc;  
int isi;
{
	int cch = 0;
	int siv;
	int xpMid;
	int dch, ddch;
	int isz;
	struct SI si;
	struct RC rc;
	CHAR szOut[cchMaxField+1];
	CHAR *pch = szOut;

	bltbx( &rgsi [isi], &si, sizeof (struct SI) );
	siv = SivFromPslsIsi( &vsls, isi );
	dch = si.dch;
	ddch = rgsi [isi+1].dch - dch;

	switch (si.fic)
		{
	case ficIndexed:

/*  Variable text.  This encompases flag fields (on or off text) and text
*  fields that can have one of several strings (i.e. Clipboard Format).
*  The mechanism here deals with both as if they are the latter, with no
*  text being represented by a value of 0.  If no text is to be shown, we
*  just return. If text IS to be shown, vsls will have in that field a
*  value which is an index of the string to display.  In the case of flag
*  fields, the SLS field is a boolean, so that TRUE conveniently
*  represents an index of 1 and FALSE represents no text. */

		Assert( si.fHilite );
		if (siv == 0)
			break;  /* just erase */

/*  Get the string to display.  iszStart is set up at compile time to be
*  the starting isz in rgszIndexed where we can find the strings that are
*  displayed in this field.  For flag fields, there will only be one such
*  string.  The value stored in the SLS for a variable text field acts
*  like a diszP1 (it's off by one because zero is reserved as the indication
*  of no text). */
		isz = si.iszStart + siv - 1;
		goto LArraySz;

/*  Get the value from vsls, which at this point should represent the
	current state of things.  Fill string for TextOut (if appropriate). */

	case ficSeparator:
/* Draw a double vertical line to separate two other fields. */
		SetRectIsi(isi, &rc );
		PatBltRc(hdc, &rc, vsci.ropErase );
		xpMid = (rc.xpLeft + rc.xpRight) / 2;
		SelectObject( hdc, vsci.hbrBorder );
		PatBlt( hdc, xpMid, 0, vsci.dxpBorder, vsci.dypStatLine, PATCOPY );
		SelectObject( hdc, vsci.hbrBkgrnd );
		return;
	case ficStaticQ: /* Conditional static text */
		if (siv == sivNil)
			{  
			cch = 0;  
			break;  
			}
		/* FALL THROUGH */
	case ficStatic: /* Static text. */
		isz = si.iszStatic;
LArraySz:
		bltbx( (CHAR FAR *)rgstStatLine [isz], (CHAR FAR *)szOut,
				*rgstStatLine [isz]+1 );
		cch = szOut[0];
		StToSzInPlace(szOut);
		break;
	case ficIntQ:   /* Conditional integer */
		if (siv == sivNil)
			{  
			cch = 0;  
			break;  
			}
		/* FALL THROUGH */
	case ficInt: /* Integer field. */
		cch = CchIntToPpch( siv, &pch );
		break;
	case ficMeasure:
		/* value in sls is in za's; display in current unit */
		if (siv == sivNil)
			{  
			cch = 0;  
			break;  
			}
		cch = CchMeasure(&pch, siv, ddch);
		break;
	default:
		Assert (fFalse);
		}

	if (si.fRightJust)
		dch += (ddch - cch);

	/* We better have allocated enough room for each field, or it
			will get clipped. */
#ifdef DEBUG
	if (cch > ddch)
		ReportSz( "status line field width exceeded, clipping extra" );
#endif

/* at this point, cch should be the correct character count excluding '\0' */

	SetRectIsi( isi, &rc );
	ExtTextOut( hdc, dch * vsci.dxpTmWidth, 0,
			ETO_CLIPPED | ETO_OPAQUE,
			(LPRECT) &rc, (LPSTR)szOut, cch, (LPINT) NULL );
	if (si.fHilite && cch > 0)
		{
		/* hack for bogus system fonts! */
		int dypOffset = Usgn(vsci.dypTmInternalLeading);
		PatBlt( hdc, dch * vsci.dxpTmWidth, dypOffset, 
				cch * vsci.dxpTmWidth, vsci.dypTmHeight - dypOffset, 
				DSTINVERT );
		}
}
#ifdef WIN23
PaintSi3( hdc, isi, prc )
	HDC hdc;  
	int isi;
	struct RC *prc;				/* rect of the field to paint */
{
	int cch = 0;
	int siv;
	short dxpOff = rgslm[isi].dxpOff,
	  	  dxpFld = rgslm[isi].dxpFld,
		  dyp, dxp;
	int isz;
	struct SI3 si;
	CHAR szOut[cchMaxField+1];
	CHAR *pch = szOut;

#ifdef REVIEW
	bltbx( &rgsi3 [isi], &si, sizeof (struct SI3) );
#else
	/* for win3 visuals, SI3 is just a word, so assign instead of blt */
	si = rgsi3[isi];
#endif /* REVIEW */
	siv = SivFromPslsIsi( &vsls, isi );

	switch (si.fic)
		{
	case ficIndexed:

/*  Variable text.  This encompases flag fields (on or off text) and text
*  fields that can have one of several strings (i.e. Clipboard Format).
*  The mechanism here deals with both as if they are the latter, with no
*  text being represented by a value of 0.  If no text is to be shown, we
*  just return. If text IS to be shown, vsls will have in that field a
*  value which is an index of the string to display.  In the case of flag
*  fields, the SLS field is a boolean, so that TRUE conveniently
*  represents an index of 1 and FALSE represents no text. */

		Assert( si.fHilite );
		if (siv == 0)
			break;  /* just erase */

/*  Get the string to display.  iszStart is set up at compile time to be
*  the starting isz in rgszIndexed where we can find the strings that are
*  displayed in this field.  For flag fields, there will only be one such
*  string.  The value stored in the SLS for a variable text field acts
*  like a diszP1 (it's off by one because zero is reserved as the indication
*  of no text). */
		isz = si.iszStart + siv - 1;
		goto LArraySz;

/*  Get the value from vsls, which at this point should represent the
	current state of things.  Fill string for TextOut (if appropriate). */

	case ficSeparator:
		if (vsci.fWin3Visuals)
			{
			/*
		 	* In 3-D look, bkgnd is always grey for both raised and recessed.
		 	* legend: +=raised (bkgnd), .=recessed, B=black, W=white, G=grey
		 	* ++++GB...........BW++++GB................BW++++++++++++++++++++++
		 	*        Pg  Sec /           At  Ln  Col      COL CAPS NUM OVR REC
		 	*/
			PatBltRc( hdc, prc, PATCOPY );
			/* subtract off shading (3 top, 2 bottom) and borders (1) */
			dyp = vsci.dypStatLine - 6;
			SelectObject( hdc, vdbmgDevice == dbmgEGA3  ? vhbrDkGray : vhbrGray );
			if (isi == isiSep03) {
				PatBlt( hdc, dxpSep0Gray, 2, 1, dyp, PATCOPY );
			}
			else {
				PatBlt( hdc, dxpOff, 3, 1, dyp, WHITENESS );
				PatBlt( hdc, dxpOff + dxpSepGray2, 2, 1, dyp, PATCOPY );
			}
		
			SelectObject( hdc, vdbmgDevice != dbmgEGA3  ? vhbrLtGray : vhbrGray);
			}
		else
			{
			int xpMid;
Win2Sep:
			PatBltRc( hdc, prc, PATCOPY );
			/* Draw a double vertical line to separate two other fields. */
			PatBltRc(hdc, prc, vsci.ropErase );
			xpMid = (prc->xpLeft + prc->xpRight) / 2;
			SelectObject( hdc, vsci.hbrBorder );
			PatBlt( hdc, xpMid, 0, vsci.dxpBorder, vsci.dypStatLine, PATCOPY );
			SelectObject( hdc, vsci.hbrBkgrnd );
			}
		return;

	case ficSepToggle:
		if (vsci.fWin3Visuals)
			{
			/* 
		 	* After erasing the background, draw a vertical white
		 	* line for preceeding depression and a gray one for succeeding
		 	* depression.
		 	*/
			PatBltRc( hdc, prc, PATCOPY );	
			/* subtract off shading (3 top, 2 bottom) and borders (1) */
			dyp = vsci.dypStatLine - 6;
			PatBlt( hdc, dxpOff + dxpSepToggleWhite, 2, 1, dyp, WHITENESS );
			if (isi != isiLast3)
				{
				SelectObject( hdc, vdbmgDevice == dbmgEGA3  ? vhbrDkGray : vhbrGray );
				PatBlt( hdc, dxpOff + dxpSepToggleGray, 2, 1, dyp, PATCOPY );
				SelectObject( hdc, vdbmgDevice != dbmgEGA3  ? vhbrLtGray : vhbrGray);
				}
			}
		else
			PatBltRc( hdc, prc, PATCOPY );	

		return;

	case ficStaticQ: /* Conditional static text and blank spaces */
		if (siv == sivNil)
			{  
			cch = 0;  
			break;  
			}
		/* FALL THROUGH */
	case ficStatic: /* Static text. */
		isz = si.iszStatic;
LArraySz:
		bltbx( (CHAR FAR *)rgstStatLine [isz], (CHAR FAR *)szOut,
				*rgstStatLine [isz]+1 );
		cch = szOut[0];
		StToSzInPlace(szOut);
		break;
	case ficIntQ:   /* Conditional integer */
		if (siv == sivNil)
			{  
			cch = 0;  
			break;  
			}
		/* FALL THROUGH */
	case ficInt: /* Integer field. */
		cch = CchIntToPpch( siv, &pch );
		break;
	case ficMeasure:
		/* value in sls is in za's; display in current unit */
		if (siv == sivNil)
			{  
			cch = 0;  
			break;  
			}
		/* Only YaPp (At) is a measure; don't include the space */
		cch = CchMeasure3(&pch, siv, dxpFld - dxpBetwItems, hdc);
		break;
	default:
		Assert (fFalse);
		}

	if (si.fRightJust)
		dxpOff += dxpFld - LOWORD(GetTextExtent(hdc, szOut, cch));

#ifdef DEBUG
	/* Fields get clipped if not enough space allocated. */
	if ( LOWORD(GetTextExtent(hdc, szOut, cch)) > dxpFld )
		ReportSz( "status line field width exceeded, clipping extra" );
#endif

	/* at this point, cch should be the correct, excluding '\0' */
	ExtTextOut( hdc, dxpOff, 3, vsci.fWin3Visuals ? ETO_CLIPPED :
		ETO_CLIPPED | ETO_OPAQUE, (LPRECT) prc, (LPSTR)szOut,
		cch, (LPINT) NULL );
}

#endif /* WIN23 */

/* U p d a t e  S t a t u s  L i n e  */
/* Update status line to state described by *psls; perform all necessary 
	painting. fTogglesOnly is true when only the state of toggle keys are
	to be updated*/

/* %%Function:UpdateStatusLine %%Owner:bryanl */
UpdateStatusLine(uso)
int uso; /* controls what gets updated */
{
	struct SLS sls;
	BOOL fUpdateStatLineSave;

	Assert( vhwndStatLine );

	if (vlm == lmPreview)
		return;

	/* Fill sls with what SHOULD be displayed on the status line */
	if (uso == usoToggles)
		{
		fUpdateStatLineSave=selCur.fUpdateStatLine;
		blt(&vsls,&sls,cwSLS);
		selCur.fUpdateStatLine=fFalse;
		}
	if (FGetStatLineState(&sls,uso) && FUpdateSls(&sls,uso))
		UpdateWindow(vhwndStatLine);
	if (uso == usoToggles)
		selCur.fUpdateStatLine=fUpdateStatLineSave;
}


/* U p d a t e  S l s */
/* Update status line to state described by *psls */
/* return fTrue if painting needs to be done, fFalse otherwise */
/* %%Function:FUpdateSls %%Owner:bryanl */
FUpdateSls(psls,uso)
struct SLS *psls;
int uso;
{
	int isi;
	struct RC rc;
	struct SLS slsDiff;
#ifdef WIN23
	int isiMac = vsci.fWin3 ? isiMax3 : isiMax;
#endif /* WIN23 */
	Assert( vhwndStatLine );

	if (uso != usoToggles)
		{
		selCur.fUpdateStatLine = fFalse;
		if (uso != usoCache)
			selCur.fUpdateStatLine2 = fTrue;
		}

	SetWords(&slsDiff, 0, cwSLS);

	/* if display state (vsls) does not match desired state (sls),
		then update the mismatched display elements */
	if (FSetRgwDiff(&vsls, psls, &slsDiff, cwSLS))
		{
#ifdef WIN23
		for ( isi = 0; isi < isiMac; isi++ )
#else
		for ( isi = 0; isi < isiMax; isi++ )
#endif /* WIN23 */
			{
		/* invalidate if: statline element depends on SLS word
							AND SLS word has changed. */
#ifdef INEFFICIENT
			if (siv != 0 && siv != sivNil)
#else
				Assert( sivNil == -1 );
			if ((uns)(SivFromPslsIsi( &slsDiff, isi)+1) > 1)
#endif
				{
				SetRectIsi(isi,&rc);
				InvalidateRect( vhwndStatLine, (LPRECT) &rc, fTrue );
				}
			}

		blt( psls, &vsls, cwSLS );
		return fTrue;
		}

	return fFalse;
}



/* F  G e t  S t a t  L i n e  S t a t e */
/* Fill out *psls with the current values for the state used by the status 
	line.

	Return fFalse if interrupted while computing, fTrue otherwise
*/
/* %%Function:FGetStatLineState %%Owner:bryanl */
FGetStatLineState(psls,uso)
struct SLS *psls;
int uso;
{
	extern int vipgd;
	extern int vgrpfKeyBoardState;
	extern BOOL vfRecording;

	struct DOD *pdod;
	struct PGD pgd;
	int f;
	int pgn;
	CP cp;

	if (uso != usoToggles)
		SetWords( psls, sivNil, cwSLS );

	psls->fRec = vfRecording;
	psls->OtMr = vfOvertypeMode ? iszOtMrOT : iszOtMrNothing;
	psls->ExBl = vfExtendSel ? iszExBlEX :
			(vfBlockSel ? iszExBlBL : iszExBlNothing);
	psls->fCL = Usgn(vgrpfKeyBoardState & wKbsCapsLckMask);
	psls->fNL = Usgn(vgrpfKeyBoardState & wKbsNumLckMask);

	if (selCur.doc != docNil && PdodMother(selCur.doc)->dop.fRevMarking)
		psls->OtMr = iszOtMrMR;

	if (selCur.doc == docNil || uso == usoToggles)
		return fTrue;

	if (FOutlineEmpty(wwCur, fFalse /* fBeep */))
		return fFalse;

	f = (uso == usoCache && 
			FCacheLine(selCur.ww, selCur.doc, selCur.cpFirst, selCur.fInsEnd));

	/* if !usoCacheLine use old values in vlcb */

	psls->yaPp = vlcb.yaPp;
	psls->lnn = vlcb.lnn;
	if (vlcb.ich != iNil)
		psls->col = vlcb.ich + 1;
	if (!f)
		return fFalse;	/* interrupted computation */

	if (!(pdod = PdodDoc(selCur.doc))->fShort)
		{
		struct PLC **hplcsed = pdod->hplcsed;
		struct PLC **hplcpgd = pdod->hplcpgd;

		cp = selCur.fForward ? selCur.cpLim : selCur.cpFirst;
		psls->isedP1 = ((hplcsed == hNil) ? 1 : IInPlc(hplcsed, cp) + 1);
		if (hplcpgd != hNil && IMacPlc(hplcpgd) != 0)
			{
			CachePage(selCur.doc, CpMin(cp,CpMacDoc(selCur.doc)-1));
			GetPlc( hplcpgd, vipgd, &pgd );
			if (pgd.pgn != pgnMax)
				{
				psls->pgn = pgd.pgn;
				psls->ipgdDocP1 = vipgd + 1;
				}
			psls->ipgdMac = (*hplcpgd)->iMac;
			}
		}

	return fTrue;
}



/* SivFromPslsIsi
*
*  returns the value in an sls for the given field
*/

/* %%Function:SivFromPslsIsi %%Owner:bryanl */
SivFromPslsIsi(psls, isi)
struct SLS *psls;
int isi;
{
#ifdef WIN23
	int bwSls = vsci.fWin3 ? rgsi3 [isi].bwSls : rgsi [isi].bwSls;
#else
	int bwSls = rgsi [isi].bwSls;
#endif /* WIN23 */

	if (bwSls == bwSlsNil)
		return sivNil;
	else
		return ( *(((int *) psls) + bwSls));
}


csconst char stForHelpPressF1[] = StKey("For Help, press F1",ForHelpF1);
BOOL vfHelpPromptOn = fFalse;

/* D I S P L A Y  H E L P  P R O M P T */
/* Turn on or off the "For Help, press F1 prompt" (prompt can be nested for
dialog Alert messages) */
/* %%Function:DisplayHelpPrompt %%Owner:bryanl */
DisplayHelpPrompt(fOn)
BOOL fOn;
{
	/* caller is supposed to assure this (don't want to pull this module in) */
	Assert(vhwndStatLine);

	if (fOn == vfHelpPromptOn || vrf.fInQueryEndSession)
		return;

	vfHelpPromptOn = fOn;

	if (fOn)
		{
		CopyCsSt(stForHelpPressF1, vstPrompt);
		DisplayStatLinePrompt(pdcMenuHelp);
		}
	else
		{
		vstPrompt[0] = '\0';
		DisplayStatLinePrompt(pdcmSL|pdcmImmed|pdcmPermanent);
		DisplayStatLinePrompt(pdcRestoreImmed);
		}
}





/* D I S P L A Y  S T A T L I N E  P R O M P T */
/*  Puts vstPrompt up on the statusline with pdc.  Avoids bringing in
	prompt.c.
*/
/* %%Function:DisplayStatLinePrompt  %%Owner:bryanl */
#ifdef WIN23
DisplayStatL2 (pdc)
#else
DisplayStatLinePrompt (pdc)
#endif /* WIN23 */
int pdc;
{
	struct RC rc;
	extern struct PPR     **vhpprPRPrompt;


	/* turn off existing prompt if any */
	if (!(pdc & pdcmRestore) && vhwndPrompt)
		{
		if (vhpprPRPrompt)
			PopToHppr(hNil);
		HidePromptWindow();
		}

	if (!vhwndStatLine)
		return;

	PrcSet( &rc, 0, 0, vcchStatLinePrompt * vsci.dxpTmWidth, vsci.dypStatLine);
	/* reset from existing status line prompt if any */

	if (vcchStatLinePrompt != 0)
		{
		InvalidateRect( vhwndStatLine, &rc, fTrue );
		vcchStatLinePrompt = 0;
		}

	/* if not supplying a new prompt, nothing more to do except paint */
	if (pdc & pdcmRestore)
		goto LRet;

	SetPromptRestore (pdc);

	if (pdc & pdcmFullWidth)
		vcchStatLinePrompt = dchMax;
	else 		
		{
		int isi;

		for ( isi = 0 ; isi < isiLast ; isi++ )
			if (rgsi [isi].dch >= vstPrompt [0] && 
					rgsi [isi].fic == ficSeparator)
				break;

		vcchStatLinePrompt = rgsi [isi].dch;
		}

	rc.xpRight = vcchStatLinePrompt * vsci.dxpTmWidth;
	InvalidateRect( vhwndStatLine, (LPRECT) &rc, fTrue );
LRet:
	if (pdc & pdcmImmed)
		UpdateWindow(vhwndStatLine);
}


/* %%Function:SetRectIsi %%Owner:bryanl */
#ifdef WIN23
SetRectIsi2(isi,prc)
#else
SetRectIsi(isi,prc)
#endif /* WIN23 */
int isi; 
struct RC *prc;
{
	Assert( isi < isiLast );

	GetClientRect( vhwndStatLine, (LPRECT)prc );
	prc->xpLeft = rgsi [isi].dch * vsci.dxpTmWidth;
	prc->xpRight = rgsi [isi+1].dch * vsci.dxpTmWidth;
}


/* F  C a c h e  L i n e */
/* Compute column, row, and vertical measurement associated with
	the given doc,  cp.  Relies on information given by the page table.

	return true if completed, false if interrupted
*/

/* %%Function:FCacheLine %%Owner:bryanl */
FCacheLine( ww, doc, cp, fEnd )
int ww;
int doc;
CP cp;
int fEnd;
{
	CP cpScan;
	int flmSave, flmWanted;
	int lnn = sivNil;
	int yaPp = sivNil;
	int ich = sivNil;
	int fReset = fFalse;
	int yaPageLim;
	struct PHE phe;
	int xp, idr;
	int t;
	CP cpFirst, dcp;
	struct PLDR **hpldr;

	Assert( iNil == sivNil );
	Assert( doc != docNil );

#ifdef DEBUG
	if (cp >= CpMacDoc(doc))
		{
		Assert(fFalse);
		return fFalse;
		}
#endif
/* Do not show At and Row if:
	- Draft View
	- DisplayAsPrinted is off
	- Background repagination is off 
	- Printer is unavailable, nil, or invalid
	- in short DOD (header,footnote,etc)
	- busy printing.
*/
	if (vpref.fDraftView 
			|| PwwdWw(ww)->grpfvisi.flm == flmDisplay 
			|| vmerr.fPrintEmerg 
			|| !vpref.fBkgrndPag 
			|| vlm != lmNil 
			|| PdodDoc(doc)->fShort)
		{
LNoPrinter:
		vlcb.ca.doc = docNil;
		vlcb.lnn = sivNil;
		vlcb.yaPp = sivNil;
		goto LRet;
		}

/* if line enclosing cp is already cached, just compute the Col and return */

	if (FInCaFEnd( doc, cp, &vlcb.ca, fEnd) && vlcb.ww == ww)
		goto LRet;

	yaPageLim = PdodDoc(doc)->dop.yaPage - abs (PdodDoc(doc)->dop.dyaBottom);
	CachePage(doc,cp);
	if (fEnd && selCur.fIns && cp == caPage.cpFirst)
		{
		Assert( cp != cp0 );
		CachePage(doc,cp-1);
		}
	cpScan = caPage.cpFirst;

/* set up wwLayout */
	flmSave = vflm;
	fReset = fTrue;
	if (!FInitWwLbsForRepag(ww, doc, lmBRepag, NULL, NULL))
		goto LNoPrinter;
	flmWanted = vflm;

/* if on same page as cache & at or past cache line; 
	use cache info for a head start */
	if (FInCaFEnd(vlcb.ca.doc,vlcb.ca.cpFirst,&caPage,fEnd)
			&& vlcb.ca.cpFirst < cp
			&& vlcb.ww == ww)
		{
		if (vlcb.lnn == lnnStatlineLast)
			{
/* we are beyond line 999 of the current page; don't bother with ya and lnn
	since they won't show, but compute the ich by looking at EDLs */
			goto LRet;
			}
		Assert( (uns)vlcb.lnn < lnnStatlineLast );
		Assert( vlcb.yaPp != sivNil );

/* do not use cache info if in a different table cell */
		if (!vlcb.fInsideCell ||
				(CacheTc(wwNil,doc,vlcb.ca.cpFirst,fFalse,fFalse),
				FInCa( doc,cp,&vtcc.ca)))
			{
/* scan forward from cached line to destination cp */
			lnn = vlcb.lnn;
			yaPp = vlcb.yaPp;
			cpScan = vlcb.ca.cpFirst;
			goto LForward;
			}
		}

/* couldn't leverage cache; start scan from top of page containing cp */

	vlcb.fInsideCell = fFalse;
	lnn = 1;
	yaPp = abs (PdodDoc(doc)->dop.dyaTop);
LForward:
/* scan forward to a cp on the same page */
	for ( ;; )
		{
		CP cpT;
		extern CP vcpFirstTablePara;
#ifdef DEBUG
		if (cp == CpMacDoc(doc) && !fEnd)
			{
			Assert(fFalse);
			return fFalse;
			}
#endif
		if (!vlcb.fInsideCell && FInTableDocCp( doc, cpScan ))
			{
			t = TGetStatlineTable(doc, &cpScan, cp, &yaPp, &lnn, vcpFirstTablePara);
			if (t == tYes)
				continue;
			if (t == tMaybe)
				goto LAbort;
			}
		else  if ((CachePara(doc, cpScan), FGetValidPhe( doc, cpScan, &phe ))
				&& cpScan == caPara.cpFirst
				&& !phe.fDiffLines
				&& !FInCaFEnd(doc,cp,&caPara,fEnd))
			{

		/* we can use the para height cache to zip through this para! */
			lnn += phe.clMac;
			yaPp += phe.clMac * phe.dyaLine + 
					vpapFetch.dyaBefore + vpapFetch.dyaAfter;
			cpScan = caPara.cpLim;
			}
		else
			{
			FormatLine(wwLayout, doc, cpScan);
			if (cp < vfli.cpMac || (fEnd && cp == vfli.cpMac))
				{
			/* SUCCESS! HAVE yaPp,lnn correct */
				vlcb.ca.doc = doc;
				vlcb.ca.cpFirst = vfli.cpMin;
				vlcb.ca.cpLim = vfli.cpMac;
				vlcb.ww = ww;
				vlcb.lnn = lnn;
				vlcb.yaPp = yaPp;
				break;
				}
			cpScan = vfli.cpMac;
			if (vfli.dypLine > 0)	/* can == 0 if field before table */
				lnn++;
			yaPp += UMultDiv( vfli.dypLine, czaInch, vfli.dyuInch );
			}
	/* in case yaPp overflows before lnn reaches lnnStatLineLast */
		yaPp = umin( 22*dxaInch, yaPp );
		if ((uns)lnn >= lnnStatlineLast)
			{
	/* beyond last line that we can or want to display: setup cache so we
	   will quickly show all future lines below or including this one the
	   same way: no "At"; Row == <last one that we can display> */
			vlcb.lnn = lnnStatlineLast;
			vlcb.yaPp = sivNil;
			vlcb.ca.doc = doc;
			vlcb.ww = ww;
			vlcb.ca.cpFirst = cpScan;
			vlcb.ca.cpLim = caPage.cpLim;
			Assert( cpScan <= caPage.cpLim );
			break;
			}
/* periodically, check for interruptions.  Allow blink to occur in
	FMsgPresent so we make progress */
		if ((lnn % 10) == 0)
			{
			if (FMsgPresent(mtyIdle))
				goto LAbort;
			Assert( PmwdWw(ww)->doc == doc );
			if ((vlm != lmBRepag || vflm != flmWanted)
					&& !FInitWwLbsForRepag(ww, doc, lmBRepag, NULL, NULL))
				{
LAbort:
				vlcb.ca.doc = docNil;
				Assert(fReset);
				EndFliLayout(lmNil, flmSave);
				return fFalse;
				}
			}
		}	/* end for */

LRet:
	if (fReset)
		EndFliLayout(lmNil, flmSave);

	if (PwwdWw(ww)->fDirty)
		UpdateWw(ww, fFalse); /* Heap Mvmt! */
	if (DlWhereDocCp(ww, doc, cp, fEnd, &hpldr, &idr, &cpFirst, NULL, fTrue) < 0)
		return iNil;
	FormatLine( ww, doc, cpFirst );
	dcp = CpMin(cp, vfli.cpMac) - cpFirst;
	XpFromDcp( dcp, dcp, &xp, &vlcb.ich );
	Assert( vlcb.ich != iNil );

	return fTrue;
}


/* F  I n  C a  F E n d */
/* %%Function:FInCaFEnd %%Owner:bryanl */
FInCaFEnd(doc, cp, pca, fEnd)
int doc; 
CP cp; 
struct CA *pca; 
int fEnd;
{
	return (pca->doc == doc && pca->cpFirst <= cp &&
			(cp < pca->cpLim || (fEnd && cp == pca->cpLim)));
}


/* Stuff compacted measurement into passed pointer according to current prefs */
int CchMeasure(ppch, za, cchLast)
CHAR **ppch;
register int za;
{
	extern struct ITR vitr;
	extern CHAR    *mputsz[];
	extern unsigned mputczaUt[];
	register int cch = 0;
	unsigned czaUt;
	int zu;
	int ut = vpref.ut;
	int cchPost;
	char *pch;
	int cchBase;

	czaUt = mputczaUt[ut];

	Assert( za >= 0 );

	za += czaUt / 20;      /* round off to one decimal place */

	zu = za / czaUt;        /* Get integral part */

	cch += CchIntToPpch(zu, ppch); /* Expand integral part */

	za -= zu * czaUt; /* Retain fraction part */

	cchBase = cch;
	for  (pch = mputsz [ut]; *pch && cch < cchLast; cch++ )
		*(*ppch)++ = *pch++;

	if (cchLast - cch < 2)
		return cch;

	/* we could have fit the fractional portion, so do it */

	(*ppch) -= (cch - cchBase);
	cch = cchBase;

	if ((za *= 10) >= czaUt)
		{
		*(*ppch)++ = vitr.chDecimal;
		cch++;
		zu = za / czaUt;
		*(*ppch)++ = '0' + zu;
		cch++;
		}

	for  (pch = mputsz [ut]; *pch && cch < cchLast; cch++ )
		*(*ppch)++ = *pch++;

	return cch;
}

#ifdef WIN23

/********************************************************************
	Win 2/3 Layer Routines
********************************************************************/

SetRectIsi(isi,prc)
int isi; 
struct RC *prc;
{
	if (vsci.fWin3)
		SetRectIsi3(isi,prc);
	else
		SetRectIsi2(isi,prc);
}
DisplayStatLinePrompt (pdc)
int pdc;
{
	if (vsci.fWin3)
		DisplayStatL3(pdc);
	else
		DisplayStatL2(pdc);
}

/***********************************************************************
			Win 3 versions
***********************************************************************/
/*--------------------------------------------------------------------------
 * %%Function:SetRectIsi %%Owner:phillipg
 *
 * Modified to use the table of field sizes and offsets by phillipg
 * Note that the ypTop is 3, not 0, so that the top shadow lines aren't 
 * erased.  Bottom is two less because of white shadow and grey line.
 *--------------------------------------------------------------------------*/
SetRectIsi3(isi,prc)
int isi; 
struct RC *prc;
{
	Assert( isi < isiMax3 );

	GetClientRect( vhwndStatLine, (LPRECT)prc );
	prc->xpLeft = rgslm[isi].dxpOff;

	/* if isiLast, leave right at the end of the line to erase empty space */

	if (isi != isiLast3)
		prc->xpRight = prc->xpLeft + rgslm[isi].dxpFld;
	if (vsci.fWin3Visuals)
		{	/* subtract off shading */
		prc->ypTop = 3;
		prc->ypBottom -= 2;
		}
}
/*--------------------------------------------------------------------------
 * %%Function:DisplayStatLinePrompt  %%Owner:bryanl && phillipg
 *
 * Puts vstPrompt up on the statusline with pdc.  Avoids bringing in
 * prompt.c.
 *
 * NOTE: vcchStatLinePrompt is now only used as a flag.  Instead of setting
 * 		 it to the number of characters on the StatLine, it is either 0 or 1.
 *--------------------------------------------------------------------------*/
DisplayStatL3(pdc)
int pdc;
{
	struct RC rc;
	extern struct PPR     **vhpprPRPrompt;

	/* turn off existing prompt if any */
	if (!(pdc & pdcmRestore) && vhwndPrompt)
		{
		if (vhpprPRPrompt)
			PopToHppr(hNil);
		HidePromptWindow();
		}

	if (!vhwndStatLine)
		return;

	/* reset from existing status line prompt if any */
	if (vcchStatLinePrompt != 0)
		{
		InvalidateRect( vhwndStatLine, (LPRECT) NULL, fTrue );
		vcchStatLinePrompt = 0;
		}

	/* if supplying a new prompt, set it up, otherwise just paint. */
	if (!(pdc & pdcmRestore)) {
		SetPromptRestore (pdc);
		vcchStatLinePrompt = 1;
		InvalidateRect( vhwndStatLine, (LPRECT) NULL, fTrue );
	}
	if (pdc & pdcmImmed)
		UpdateWindow(vhwndStatLine);
}


csconst	BYTE rgft[isiMax3] = {
		(char) ftSep	+ (dxpSep0 << 3),			/* isiSep03 */
		(char) ftText	+ (iszSPg << 3),			/* isiSPg3 */
		(char) ftNum	+ (cchDig3 << 3),			/* isiPgn3 */
		(char) ftText	+ (iszSSec << 3),			/* SSec3 */
		(char) ftNum	+ (cchDig2 << 3),			/* IsedP13 */
		(char) ftNum	+ (cchDig4 << 3),			/* IpgdDocP13 */
		(char) ftText	+ (iszSSlash << 3),			/* SSlash3 */
		(char) ftNum	+ (cchDig4 << 3),			/* IpgdMac3 */
		(char) ftSep	+ (dxpSep << 3),			/* Sep13 */
		(char) ftText	+ (iszSAt << 3),			/* SAt3 */
		(char) ftNum	+ (cchDig3 << 3),			/* YaPp3 */
		(char) ftText	+ (iszSLn << 3),			/* SLn3 */
		(char) ftNum	+ (cchDig3 << 3),			/* Ln3 */
		(char) ftText	+ (iszSCol << 3),			/* SCol3 */
		(char) ftNum	+ (cchDig3 << 3),			/* Col3 */
		(char) ftSep	+ (dxpSep2 << 3),			/* Sep23 */
		(char) ftTogDub	+ (iszIExBl << 3),			/* ExBl3 */
		(char) ftSep	+ (dxpSepToggle << 3),		/* ExBl_CLSp3 */
		(char) ftTog	+ (iszICl << 3),			/* CL3 */
		(char) ftSep	+ (dxpSepToggle << 3),		/* CL_NLSp3 */
		(char) ftTog	+ (iszINl << 3),			/* NL3 */
		(char) ftSep	+ (dxpSepToggle << 3),		/* NL_OtMrSp3 */
		(char) ftTogDub + (iszIOtMr << 3),			/* OtMr3 */
		(char) ftSep	+ (dxpSepToggle << 3),		/* OtMr_RecSp3 */
		(char) ftTog	+ (iszIRec << 3),			/* Rec3 */
};

/*--------------------------------------------------------------------------
 * %%Function: InitStatLine		%%Author: phillipg
 *
 * FStatLineInit determines the size of the fields of the status line
 * and their offsets from the left side of the screen based on a
 * Helvetica 12 font created in FInitScreenConstants. Some blank spaces have
 * their own fields in the data structure, while the widths of others
 * are added to the fields *preceeding* the blank space.
 *--------------------------------------------------------------------------*/
void InitStatLine( hdc )
HDC hdc;
{
	HFONT hfontOld;
	int iT, dxpCur, ftTyp, ftDat, isi, dxpFld, dxp, dxpMostSz = 0;
	extern struct ITR vitr;				/* international characters */
	
	if (hfontStatLine)
		hfontOld = SelectObject( hdc, hfontStatLine );

	for( isi = dxpCur = 0; isi < isiLast3; isi++ ) {
		ftTyp = (rgft[isi] & fftType);
		/* shift right to extract encoded data field */
		ftDat = (rgft[isi] & fftData) >> 3;
		switch( (int) ftTyp ) {

			case ftNum:
				dxpFld = vdxpDigit * ftDat;
				/*
				 * "At" (YaPp) field has 3 digits, a possible decimal and a
				 * unit string.  NOTE: the 4 is because only the first 4 items
				 * in mputsz are units that are used (according to RosieP). 
				 */
				if (isi != isiIpgdMac3 ) {
					dxpFld += dxpBetwItems;
					if(isi == isiYaPp3) {
						for( iT = 0; iT < 4; iT++ )
							dxpMostSz = max( dxpMostSz, 
								LOWORD( GetTextExtent( hdc, mputsz[iT],
													CchSz(mputsz[iT]) - 1 )));
							GetCharWidth( hdc, vitr.chDecimal, vitr.chDecimal,
											(LPINT) &dxp );
							dxpFld += dxp + dxpMostSz;
					}
				}
				break;
					
			case ftTogDub:
			case ftTog:
			case ftText:
				dxpFld = LOWORD(GetTextExtent( hdc, &rgstStatLine[ftDat][1],
													rgstStatLine[ftDat][0] ));
				if (ftTyp == ftText)
					dxpFld += dxpLabelField;
				else if (ftTyp == ftTogDub) {
					ftDat++;
					/* the LOWORD . . . should quote to above */
					dxpFld = max( dxpFld, LOWORD( GetTextExtent( hdc, 
											&rgstStatLine[ftDat][1], 
											rgstStatLine[ftDat][0] ) ) );
				}
				break;
				
			case ftSep:
				dxpFld = ftDat;
				break;
		}	/* switch */
		Assert(dxpFld < 256);	/* will overflow the field */
		
		rgslm[isi + 1].dxpOff = (dxpCur += (rgslm[isi].dxpFld = dxpFld));
	}   /* for */

	/* isiLast gets filled to end of line, so size of Fld unnecessary */

	if (hfontStatLine && hfontOld)
		SelectObject( hdc, hfontOld );
}   /* InitStatLine */

/*--------------------------------------------------------------------------
 * %%Function: CchMeasure		%%Owner: ?? && phillipg
 *
 * Take a distance, passed in as za, and, depending on the current preferred
 * output measurement, convert it to a string of digits plus the appropriate
 * unit.  If there is room, include a fractional value, too, but don't cause
 * truncation.
 *
 * *ppch will point to END of string upon return
 *--------------------------------------------------------------------------*/
int CchMeasure3(ppch, za, dxpMost, hdc)
CHAR **ppch;						/* put final string here */
register int za;					/* number to convert to string */
int dxpMost;						/* space in which the string must fit */
HDC hdc;
{
	extern struct ITR vitr;
	extern CHAR    *mputsz[];
	extern unsigned mputczaUt[];

	unsigned czaUt;
	int zu, ut = vpref.ut,
		cch, cchUnit;
	char *szUnit;

	Assert( za >= 0 );
	czaUt = mputczaUt[ut];
	/*
	 * adding in czaUt/20 rounds up the first decimal place, since czaUt/20 ==
	 * czaUt/10 * czaUt/2.  (i.e. to round up, (whole unit)/2, so to round
	 * up the decimal add (whole unit/10)/2 )
	 */
	za += czaUt / 20;				

	zu = za / czaUt;				/* integral part */
	za -= zu * czaUt;				/* fractional part */

	/* Side Effect Note: CchIntToPpch advances ppch */
	cch = CchIntToPpch(zu, ppch);

	for( szUnit = mputsz[ut], cchUnit = 0; *szUnit; szUnit++, cchUnit++ )
		;
	szUnit -= cchUnit;

	/* Add a single decimal now, if != 0 */
	if (za = za * 10 / czaUt) {
		*(*ppch)++ = vitr.chDecimal;
		*(*ppch)++ = '0' + za;
		cch += 2;
		/* If no room for fraction, back up */
		if (LOWORD( GetTextExtent(hdc, (LPSTR)*ppch - cch, cch) ) + 
			LOWORD( GetTextExtent(hdc, (LPSTR)szUnit, cchUnit) )   > dxpMost)
		{
			(*ppch) -= 2;
			cch -= 2;
		}
	}
	/* add units to string */
	cch += cchUnit;
	for( ; cchUnit; cchUnit-- )
		*(*ppch)++ = *szUnit++;

	return cch;
}

#endif /* WIN23 */

