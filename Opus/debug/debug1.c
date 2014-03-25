#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "debug.h"
#include "disp.h"
#include "screen.h"
#include "scc.h"
#include "props.h"
#include "sel.h"
#include "heap.h"
#include "doc.h"
#include "format.h"

#ifdef DEBUG

struct SCC              *vpsccSave;
extern struct SCC       *vpsccBelow;
extern struct PREF      vpref;
extern struct SEL       selCur;
extern struct FLI	vfli;

#ifdef REVIEW  /* IFTIME not used now (bz) */
/* these definitions are only for use by MarkProc() */

struct MARK
	{
	int     n;
	int     c;
	uns     dtick;
};

#define imarkMax 100
int     vimark;
struct MARK vrgmark[imarkMax];
uns     vtickMark;

#endif


#ifdef REVIEW  /*  IFTIME not used now (bz) */

/* M A R K   P R O C */
/* Points in code may be marked with a call DbgMark(n)
where n is a constant passed to identify which call to DbgMark()
was executed.  DbgMark() then stores n, and some timing information in
an instance of the MARK structure.  The circular buffer of imarkMax instances
of the MARK structure keep a running backtrace of recent calls to DbgMark()
in the following manner:

	1) if mark.c == 0, then mark.dtick shows the elapsed time (in 60ths of
a second) since the previous call to DbgMark().  mark.c == 0 also implies
the previous call was DbgMark(p) where p != mark.n.
	2) if mark.c > 0, then mark.dtick shows the maximum elapsed time
(in 60ths of a second) between sucessive calls of DbgMark(mark.n).  mark.c
indicates the number of sucessive calls of DbgMark(mark.n).  mark.c > 0
will only appear inside of loops.

	In this way the maximum time of a loop iteration, as well as the
time between loops is logged by DbgMark().
*/
/* REVIEW:  IFTIME Won't work under Windows beyond 65535 ticks
	This is all basically bogus under windows. The interval returned is milliseconds,
	it needs at least a long and will probably still recyle too soon. Since it
	is never used, I am commenting the whole thing out (bz)
*/
/* %%Function:MarkProc %%Owner:NOTUSED */
MarkProc(n)
int n;
{
	struct MARK *pmark = &vrgmark[vimark];
	int cNew = 0;
	uns dtick = (uns)GetTickCount() - vtickMark;

	vtickMark += dtick;
	if (pmark->n == n)
		{
/* repetition of marks */
		if (pmark->c > 0)
			{
			if (dtick > pmark->dtick) pmark->dtick = dtick;
			pmark->c++;
			return;
			}
		else
/* first repetition */
			cNew = 1;
		}
/* create new mark in buffer */
	if (++vimark == imarkMax) vimark = 0;
	pmark = &vrgmark[vimark];
	pmark->n = n;
	pmark->c = cNew;
	pmark->dtick = dtick;
}


#endif


#ifdef ENABLE   /* NO DoJumps in Opus, fortunately */
/* DoJump(&env) jumps so as to appear as if the code is returning from a
previous call SetJump(&env).  The previous SetJump call stores
in env the return address from the SetJump call and the pcode
and native stack pointers; that is, the information required to
recover the state just after returning from the prior SetJump.

DoJump never returns to its caller. */

/* DoJump is only a procedure when the DEBUG compiler flag is set.
Otherwise, it is a system call.  We make it a procedure under DEBUG only
because we might want to set a break point. */
DoJump(penv)
ENV *penv;
{
	DoJumpT(penv);
}


#endif  /* ENABLE */




/* C M D  T E S T  X */
/* %%Function:CmdTestX %%Owner:BRADV */
CMD CmdTestX(pcmb)
CMB *pcmb;
{
	return CmdRepaginate();
}




#endif  /* DEBUG */


/* CMDDEBUG'S  these are here to make it easy to hook up a
	debugging command that can be accessed through the keyboard.
*/

/* C M D  D E B U G  1 */
/* %%Function:CmdDebug1 %%Owner:BRADV */
CMD CmdDebug1 (pcmb)
CMB *pcmb;
{
#ifdef DEBUG
#ifdef BRYANL
	DumpGrpchr();
#else
	DumpState ();
/*     TestUpdateRuler (); */
#endif
#endif /* DEBUG */
	return cmdOK;
}



/* C M D  D E B U G  2 */
/* %%Function:CmdDebug2 %%Owner:BRADV */
CMD CmdDebug2 (pcmb)
CMB *pcmb;
{
#ifdef DEBUG
	DumpHeap (sbDds);
/*     CkFetchFunc (fTrue); */
#endif /* DEBUG */
	return cmdOK;
}




/* C M D  D E B U G  3 */
/* %%Function:CmdDebug3 %%Owner:BRADV */
CMD CmdDebug3 (pcmb)
CMB *pcmb;
{
#ifdef DEBUG
	DumpDocs ();
	/* CkFetchFunc (fFalse); */
#endif /* DEBUG */
	return cmdOK;
}




/* C M D  D E B U G  4 */
/* %%Function:CmdDebug4 %%Owner:BRADV */
CMD CmdDebug4 (pcmb)
CMB *pcmb;
{
#ifdef DEBUG
	DumpHeapUsage ();
#endif /* DEBUG */
	return cmdOK;
}




/* C M D  D E B U G  5 */
/* %%Function:CmdDebug5 %%Owner:BRADV */
CMD CmdDebug5 (pcmb)
CMB *pcmb;
{
#ifdef DEBUG
	ShowFieldInfo ();
#endif /* DEBUG */
	return cmdOK;
}




/* C M D  D E B U G  6 */
/* %%Function:CmdDebug6 %%Owner:BRADV */
CMD CmdDebug6 (pcmb)
CMB *pcmb;
{
#ifdef DEBUG
	TestScanFnForBytes();
#endif /* DEBUG */
	return cmdOK;
}



/* C M D  D E B U G  7 */
/* %%Function:CmdDebug7 %%Owner:BRADV */
CMD CmdDebug7 (pcmb)
CMB *pcmb;
{
#ifdef DEBUG
	/* multiple calls increment this. range is 0 - 4 */
	extern int vTestClip;
	vTestClip = (vTestClip++) % 5;

#ifdef DTESTCLIP
	CommSzNum(SzShared("vTestClip for testing clipbrd: "), vTestClip);
#endif /* DTESTCLIP */


	/* ShowFetch (docScrap); */
	/*  InsertFieldWithArgs(); */
	/* TestEnqueue ();  */
#endif /* DEBUG */
	return cmdOK;
}




/* C M D  D E B U G  9 */
/* %%Function:CmdDebug9 %%Owner:BRADV */
CMD CmdDebug9 (pcmb)
CMB *pcmb;
{
#ifdef DEBUG
	/* FakeRtfClipboard(); */
	ShowFetch (selCur.doc);
#endif /* DEBUG */


	return cmdOK;
}



/* C M D  F I E L D  D B G  0 */
/* %%Function:CmdDebug0 %%Owner:BRADV */
CMD CmdDebug0 (pcmb)
CMB *pcmb;
{
#ifdef DEBUG
	ShowChpSelCur ();
	ShowDisplayPara();
#endif
	return cmdOK;
}


#ifdef RPDEBUG

/* %%Function:FakeRtfClipboard %%Owner:NOTUSED */
FakeRtfClipboard()
{             
	int
	int cch;
	char rgch[1024];
	HANDLE h;
	LPCH lpch;
	extern int cfRTF;

	FetchRgch(&cch, rgch, selCur.doc, cp0,
			CpMacDocEdit(selCur.doc), 1024);
	rgch[cch++] = '\0';
	if ((h = OurGlobalAlloc(GMEM_MOVEABLE, (long)cch)) == hNil ||
			(lpch = GlobalLock(h)) == NULL)
		{
		Beep();
		return;
		}

	bltbx( lpch, (LPCH) rgch, cch);
	GlobalUnlock( h );

	if (OpenClipboard(GetFocus()))
		{
		EmptyClipboard();
		SetClipboardData(cfRTF, h);
		CloseClipboard();
		}

	GlobalFree( h );
	vTestClip = cfRTF;
}


#endif /* RPDEBUG */

#ifdef BRYANL

static int fBegin;


/* %%Function:DumpGrpchr %%Owner:BRADV */
DumpGrpchr()
{
	extern char 		szEmpty[];
	extern char             (**vhgrpchr)[];
	extern int              vbchrMax;
	extern int              vbchrMac;

	struct CHR *pchr = (struct CHR *) &(**vhgrpchr) [0];

	CommSzRgNum( SzShared("vfli.dypLine,dypAfter,dypBefore,dypFont,dypBase: "), vfli.rgdyp, 5 );
	fBegin = fTrue;
	for ( ;; )
		{
		switch (pchr->chrm)
			{
		case chrmChp:
			CommHdr( SzShared( "chrmChp: " ), pchr->ich );
			CommNLFFontAttr(SzShared( ", Attributes: "), 
					pchr->chp.fItalic,
					pchr->chp.fStrike,
					pchr->chp.kul,
					pchr->chp.fBold );
			break;
		case chrmTab:
			CommHdr( SzShared( "chrmTab: " ), pchr->ich );
			CommSzNumNoLf( SzShared( ", chrt.ch = "), ((struct CHRT *)pchr)->ch );
			break;
		case chrmVanish:
			CommHdr( SzShared( "chrmVanish: " ), pchr->ich );
			break;
		case chrmFormula:
			CommHdr( SzShared( "chrmFormula: " ), pchr->ich );
			CommSzNumNoLf( SzShared( ", dxp = "), ((struct CHRF *)pchr)->dxp );
			CommSzNumNoLf( SzShared( ", dyp = "), ((struct CHRF *)pchr)->dyp );
			break;
		case chrmDisplayField:
			CommHdr( SzShared( "chrmDisplayField: " ), pchr->ich );
			break;
		case chrmFormatGroup:
			CommHdr( SzShared( "chrmFormatGroup: " ), pchr->ich );
			break;
		case chrmEnd:
			CommSzNumNoLf( SzShared( "END:    ich = "),pchr->ich);
			CommSzNumNoLf( SzShared( ", ichSpace3 = "), vfli.ichSpace3 );
			CommSz( SzShared( "\r\n" ) );
			return;
			}
		CommSz( SzShared( "\r\n" ) );
		pchr = ((char *)pchr) + pchr->chrm;
		}
}


/* %%Function:CommHdr %%Owner:BRADV */
CommHdr(sz,ich)
char sz[];
int ich;
{
	if (fBegin)
		{
		CommSz( SzShared( "BEGIN: ") );
		fBegin = fFalse;
		}
	else
		CommSz( SzShared( "       ") );
	CommSz( sz );
	CommSzNumNoLF( SzShared( "ich = "), ich );
}


/* %%Function:CommSzNumNoLF %%Owner:BRADV */
CommSzNumNoLF( sz, num )
CHAR *sz;
int num;
{
	CHAR szBuf[ 256 ];
	CHAR *pch = szBuf;

	Assert( CchSz( sz ) <= 256);

	pch = &szBuf[ CchCopySz( sz, szBuf ) ];
	CchIntToPpch( num, &pch );
	*pch = '\0';
	CommSz( szBuf );
}


/* %%Function:CommNLFFontAttr %%Owner:BRADV */
CommNLFFontAttr(szHeader, fItalic, fStrikeOut,fUnderline,fBold)
char szHeader[];
{
	char rgch [128], *pch;

	pch = rgch + CchCopySz( szHeader, rgch );
	if (!fItalic && !fStrikeOut && !fUnderline && !fBold)
		return;
	if (fItalic)
		pch += CchCopySz( SzShared( "ITALIC "), pch );
	if (fStrikeOut)
		pch += CchCopySz( SzShared( "STRIKE "), pch );
	if (fUnderline)
		pch += CchCopySz( SzShared( "UNDERLINE "), pch );
	if (fBold)
		pch += CchCopySz( SzShared( "BOLD "), pch );
	CommSz( rgch );
}


#endif



#ifdef DEBUG
/* %%Function:DumpOrgExt %%Owner:bobz */
DumpOrgExt(hdc)
HDC hdc;
{
	DWORD dw;

	dw = GetWindowOrg(hdc);
	CommSzRgNum(SzShared("Window Org x, y: "),
			&dw, 2);
	dw = GetWindowExt(hdc);
	CommSzRgNum(SzShared("Window Ext x, y: "),
			&dw, 2);
	dw = GetViewportOrg(hdc);
	CommSzRgNum(SzShared("Viewport Org x, y: "),
			&dw, 2);
	dw = GetViewportExt(hdc);
	CommSzRgNum(SzShared("Viewport Ext x, y: "),
			&dw, 2);

}


/* %%Function:DumpMetaRec %%Owner:bobz */
DumpMetaRec(lpMFR)
LPMETARECORD lpMFR;
{
	switch (lpMFR->rdFunction)
		{
	case 0x1e:
		CommSz(SzShared("SaveDC: "));
		break;
	case 0x102:
		CommSz(SzShared("SetBkMode: "));
		break;
	case 0x103:
		CommSz(SzShared("SetMapMode: "));
		break;
	case 0x104:
		CommSz(SzShared("SetRop2: "));
		break;
	case 0x106:
		CommSz(SzShared("SetPolyFillMode: "));
		break;
	case 0x127:
		CommSz(SzShared("RestoreDC: "));
		break;
	case 0x12d:
		CommSz(SzShared("SelectObject: "));
		break;
	case 0x12e:
		CommSz(SzShared("SetTextAlign: "));
		break;
	case 0x201:
		CommSz(SzShared("SetBkColor: "));
		break;
	case 0x209:
		CommSz(SzShared("SetTextColor: "));
		break;
	case 0x20c:
		CommSz(SzShared("SetWindowExt: "));
		break;
	case 0x20b:
		CommSz(SzShared("SetWindowOrg: "));
		break;
	case 0x20e:
		CommSz(SzShared("SetViewportExt: "));
		break;
	case 0x20d:
		CommSz(SzShared("SetViewportOrg: "));
		break;
	case 0x213:
		CommSz(SzShared("LineTo: "));
		break;
	case 0x214:
		CommSz(SzShared("MoveTo: "));
		break;
	case 0x231:
		CommSz(SzShared("SetMapperFlags: "));
		break;
	case 0x2fa:
		CommSz(SzShared("CreatePenIndirect: "));
		break;
	case 0x2fb:
		CommSz(SzShared("CreateFontIndirect: "));
		break;
	case 0x2fc:
		CommSz(SzShared("CreateBrushIndirect: "));
		break;
	case 0x324:
		CommSz(SzShared("Polygon: "));
		break;
	case 0x325:
		CommSz(SzShared("Polyline: "));
		break;
	case 0x418:
		CommSz(SzShared("Ellipse: "));
		break;
	case 0x416:
		CommSz(SzShared("IntersectClipRect: "));
		break;
	case 0x41b:
		CommSz(SzShared("Rectangle: "));
		break;
	case 0x521:
		CommSz(SzShared("TextOut: "));
		break;
	case 0x61c:
		CommSz(SzShared("RoundRect: "));
		break;
	case 0x61d:
		CommSz(SzShared("PatBlt: "));
		break;
	case 0x817:
		CommSz(SzShared("Arc: "));
		break;
	case 0xa32:
		CommSz(SzShared("ExtTextOut: "));
		break;
	default:
		CommSzNum(SzShared("Untranslated metafile keyword: "), lpMFR->rdFunction);
		}

	CommSzLrgNum( SzShared("Args: "), lpMFR->rdParm, (int)(lpMFR->rdSize - 3) );

}


#endif /* DEBUG */

