#define NOGDICAPMASKS
#define NOWINSTYLES
#define NOMENUS
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NODRAWTEXT
#define NOOPENFILE
#define NOWH
#define NOWINOFFSETS
#define NOOPENFILE
#define NOSOUND
#define NOCOMM

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "disp.h"
#include "debug.h"
#include "ourmath.h"
#include "message.h"

/* includes for structure sizes */
#include "doc.h"
#include "file.h"
#include "props.h"
#include "screen.h"
#include "format.h"
#include "pic.h"
#include "prm.h"
#include "sel.h"
#include "scc.h"
#include "layout.h"
#include "ruler.h"
#include "wininfo.h"
#include "print.h"
#define PROFDEF
#include "profile.h"
#include "keys.h"
#include "ch.h"
#include "dmdefs.h"
#define RTFDEFS
#include "rtf.h"
#include "field.h"
#include "outline.h"
#include "border.h"


#ifdef DEBUG
BOOL vfInCacheParaVerify = fFalse;

extern struct FLI       vfli;
extern struct SCI       vsci;
extern struct PRI       vpri;
extern struct WWD       **hwwdCur;
extern struct DBS       vdbs;
extern struct MWD       **hmwdCur;
extern struct SEL       selCur;
extern struct CHP	vchpGraySelCur;
extern struct PAP	vpapSelCur;
extern struct PAP	vpapGraySelCur;
extern struct DOD       **mpdochdod[];
extern int              docMac;
extern CHAR             szEmpty[];
extern struct CA        vcaCell;
extern struct TCXS      vtcxs;

extern beep();

csconst CHAR szFreeze[] = "Heap is frozen.";

/* %%Function:TestFormat %%Owner:BRADV */
fnTest()
{
	beep();
	TestFormat();
	beep();
	beep();
	dbgWait();      /* for use by symdeb to check variables */
}


/* %%Function:TestFormat %%Owner:BRADV */
TestFormat()
{
}


/* %%Function:dbgWait %%Owner:BRADV */
dbgWait()
{
}




/*      COMM Output routines        */

#define cchSzCommMax    256


/* %%Function:CommSzNum %%Owner:BRADV */
EXPORT CommSzNum( sz, num )
CHAR *sz;
int num;
{
	CHAR szBuf[ cchSzCommMax ];
	CHAR *pch = szBuf;

	Assert( CchSz( sz ) <= cchSzCommMax );

	pch = &szBuf[ CchCopySz( sz, szBuf ) ];
	CchIntToPpch( num, &pch );

	CchCopySz( SzShared("\r\n"), pch );

	CommSz( szBuf );
}


/* %%Function:CommSzLong %%Owner:BRADV */
CommSzLong( sz, lval )
CHAR *sz;
long lval;
{
	CHAR szBuf[ cchSzCommMax ];
	CHAR *pch = szBuf;

	Assert( CchSz( sz ) <= cchSzCommMax );

	pch = &szBuf[ CchCopySz( sz, szBuf ) ];
	CchLongToPPch( lval, &pch );

	CchCopySz( SzShared("\r\n"), pch );

	CommSz( szBuf );
}


/* %%Function:CommSzRgNum %%Owner:BRADV */
CommSzRgNum( sz, rgw, iwMax )
CHAR *sz;
int rgw[];
int iwMax;
{
	CHAR szBuf[ cchSzCommMax ];
	CHAR *pch = szBuf;
	int iw;

	Assert( CchSz( sz ) <= cchSzCommMax );

	pch = &szBuf[ CchCopySz( sz, szBuf ) ];
	iw = 0;
	while (iw < iwMax)
		{
		for ( ;  iw < iwMax && pch < &szBuf [cchSzCommMax - 6]; iw++ )
			{
			CchUnsToPpch( rgw [iw], &pch );
			*pch++ = ' ';
			}

		Assert( pch <= &szBuf [cchSzCommMax] ); /* if you can read this, you are
		already dead */
		CchCopySz( SzShared("\r\n"), pch );

		CommSz( szBuf );
		pch = szBuf;
		}
}


/* %%Function:CommSzLrgNum %%Owner:BRADV */
CommSzLrgNum( sz, lprgw, iwMax )     /* for far data */
CHAR *sz;
LPINT lprgw;
int iwMax;
{
	CHAR szBuf[ cchSzCommMax ];
	CHAR *pch = szBuf;
	int iw;

	Assert( CchSz( sz ) <= cchSzCommMax );

	pch = &szBuf[ CchCopySz( sz, szBuf ) ];
	iw = 0;
	while (iw < iwMax)
		{
		for ( ;  iw < iwMax && pch < &szBuf [cchSzCommMax - 6]; iw++ )
			{
			CchUnsToPpch( lprgw [iw], &pch );
			*pch++ = ' ';
			}

		Assert( pch <= &szBuf [cchSzCommMax] ); /* if you can read this, you are
		already dead */
		CchCopySz( SzShared("\r\n"), pch );

		CommSz( szBuf );
		pch = szBuf;
		}
}


/* %%Function:CommSzRgbCb %%Owner:BRADV */
CommSzRgbCb(sz, rgb, cbMax)
CHAR *sz, *rgb;
int cbMax;
{
	CHAR szBuf[ cchSzCommMax ];
	CHAR *pch = szBuf;
	int cb;

	Assert( CchSz( sz ) <= cchSzCommMax );

	pch = &szBuf[ CchCopySz( sz, szBuf ) ];
	for ( cb = 0;  cb < cbMax && pch < &szBuf [cchSzCommMax - 6]; cb++ )
		{
		CchIntToPpch((int) *rgb++, &pch );
		*pch++ = ' ';
		}

	Assert( pch <= &szBuf [cchSzCommMax] );
	CchCopySz( SzShared("\r\n"), pch );
	CommSz( szBuf );
}


/* %%Function:CommSzSz %%Owner:BRADV */
EXPORT CommSzSz( sz1, sz2 )
CHAR *sz1, *sz2;
{
	CHAR szBuf[ cchSzCommMax ];
	int cch;

	Assert( CchSz( sz1 ) + CchSz( sz2 ) - 1 <= cchSzCommMax );

	cch = CchCopySz( sz1, szBuf );
	cch += CchCopySz( sz2, &szBuf[ cch ] );
	CchCopySz( SzShared("\r\n"), &szBuf[ cch ] );

	CommSz( szBuf );
}


/* %%Function:CommSzSz %%Owner:BRADV */
CommSzSzAnsi( sz, szAnsi )
CHAR *sz, *szAnsi;
{
	CHAR szBuf[ cchSzCommMax ];
	int cch;
	char ch;
	char *pchAnsi, *pchOut;

	CommSz( sz );
	for ( pchAnsi = szAnsi ; (ch = *pchAnsi) != '\0' ; pchAnsi++ )
		{
		pchOut = &szBuf [0];
		if (ch >= '!' && ch <= '~')
			*pchOut++ = ch;
		else
			{
			*pchOut++ = '[';
			CchIntToPpch( ch, &pchOut );
			*pchOut++ = ']';
			}
		*pchOut = '\0';
		CommSz( szBuf );
		}
	CommSz(SzShared("\r\n"));
}


/* %%Function:CommSzSt %%Owner:BRADV */
CommSzSt(sz, st)
CHAR *sz, *st;
{
	CHAR szBuf [cchSzCommMax];
	int cch;

	Assert (CchSz (sz) + *st <= cchSzCommMax);
	cch = CchCopySz (sz, szBuf);
	bltb (st+1, szBuf+cch, *st);
	cch += *st;
	CchCopySz( SzShared("\r\n"), &szBuf[ cch ] );

	CommSz( szBuf );
}



/* %%Function:CommSzRgCch %%Owner:BRADV */
CommSzRgCch(sz, rgch, cchIn)
CHAR *sz, *rgch;
int cchIn;
{
	CHAR szBuf [cchSzCommMax];
	int cch;
	int cchTxt;

	/* copy string in 1st time only */
	cch = CchCopySz (sz, szBuf);

	while (cchIn)
		{
		cchTxt = (cchIn > cchSzCommMax - cch - 3 ) ?
				cchSzCommMax - cch - 3 : cchIn;
		bltb (rgch, szBuf+cch, cchTxt);
		cch += cchTxt;
		CchCopySz( SzShared("\r\n"), &szBuf[ cch ] );
		CommSz( szBuf );
		rgch += cchTxt;
		cchIn -= cchTxt;
		cch = 0;
		}
}


/* %%Function:CommNumRgch %%Owner:BRADV */
CommNumRgch(num, rgch, cchIn)
int num;
CHAR *rgch;
int cchIn;
{
	CHAR szBuf [cchSzCommMax];
	CHAR *pch = szBuf;

	Assert(cchIn+10 <= cchSzCommMax);
	CchIntToPpch( num, &pch);
	*pch = ' ';
	pch++;
	pch = bltb(rgch, pch, cchIn);
	CchCopySz(SzShared("\r\n"), pch);
	CommSz(szBuf);
}


#define cszMore 23

/* %%Function:CommSz %%Owner:BRADV */
CommSz(sz)
char *sz;
{
	static int cszComm = 0;
	CHAR szBuf [20];
	int cch;

	if (!vdbs.fCommSz)
		return;
	if (vdbs.fMoreComm && index (sz, chEop) && ++cszComm % cszMore == 0)
		{
		CHAR ch;

		cch = CchCopySz( SzShared("[more]"),szBuf);
		CchCopySz( SzShared("\r\n"), &szBuf[ cch ] );
		CommSz( szBuf );
		if ((ch = ChFromComm()) == 0x0D || ch == 0x0A)
			cszComm = cszMore - 1;
		}
	CommSzLib(sz);
}


/* %%Function:CommMultiLineSz %%Owner:BRADV */
CommMultiLineSz (sz)
CHAR *sz;

{
	CHAR *pchBegin, *pchEnd;
	int cLine;

	if (!vdbs.fCommSz)
		return;
	if (vdbs.fMoreComm)
		{
		CommSz (SzShared("[more]\r\n"));
		ChFromComm();
		}

	pchEnd = sz - 2;
	while (pchEnd != NULL)
		{
		for (pchBegin = (pchEnd += 2), cLine = 1; 
				cLine < cszMore && pchEnd != NULL;
				cLine++)
			pchEnd = index (pchEnd+1, '\r');

		if (pchEnd != NULL)
			*pchEnd = 0;

		CommSzLib (pchBegin);
		if (cLine == cszMore)
			{
			CommSzLib (SzShared("\r\n[more]\r\n"));
			ChFromComm();
			}
		}
}


/* %%Function:CommSzPnum %%Owner:BRADV */
CommSzPnum(sz, pnum)
CHAR *sz;
NUM *pnum;
{
	struct UPNUM upnum;

	LdiNum(pnum);
	UnpackNum(&(upnum.wExp), &(upnum.wSign), upnum.rgch);
	CommSz(sz);
	CommSz(SzShared("\n\r"));
	CommSzNum(SzShared("sign: "), upnum.wSign);
	CommSzNum(SzShared("exp:  "), upnum.wExp);
	CommSzSt( SzShared("mant: "), upnum.rgch);
}


/* Assert/Report intermediate routines */

#define midFormatn	0
#define midFetchn	1
#define midDisp3n	2
#define midUtiln	3
#define midFieldfmn	4
#define midFormatn2	5
#define midFetchn2      6
#define midResn 	7
#define midSearchn	8
#define midPromptn      9
#define midDisp1n	10
#define midDisp1n2	11
#define midInssubsn	12
#define midFieldcrn	13
#define midFetchn3	14
#define midEditn	15
#define midGrbitx	16
#define midGrhc		17
#define midLayout2n	18
#define midDisp2n	19
#define midFilewinn	20
#define midLayout22	21
#define midEditn2	22
#define midFieldspn	23
#define midFetch2n	24
#define midDisptbln	25
#define midClsplcn	26
#define midFetchtbn	27
#define midRtfinn	28
#define midRtfsubsn	29
#define midPrln 	30
#define midDisptbn2	31
#define midResn2	32
#define midDialog1n	33
#define midIndex2n	34
#define midMac		35
#define StCS(s) StMap(s, 1)

csconst CHAR rgstMid [midMac][] =
{
	StCS ("formatn.asm"),
	StCS ("fetchn.asm"),
	StCS ("disp3n.asm"),
	StCS ("utiln.asm"),
	StCS ("fieldfmn.asm"),
	StCS ("formatn2.asm"),
	StCS ("fetchn2.asm"),
	StCS ("resn.asm"),
	StCS ("searchn.asm"),
	StCS ("promptn.asm"),
	StCS ("disp1n.asm"),
	StCS ("disp1n2.asm"),
	StCS ("inssubsn.asm"),
	StCS ("fieldcrn.asm"),
	StCS ("fetchn3.asm"),
	StCS ("editn.asm"),
	StCS ("grbitx.asm"),
	StCS ("grhc.asm"),
	StCS ("layout2n.asm"),
	StCS ("disp2n.asm"),
	StCS ("filewinn.asm"),
	StCS ("layout22.asm"),
	StCS ("editn2.asm"),
	StCS ("fieldspn.asm"),
	StCS ("fetch2n.asm"),
	StCS ("disptbln.asm"),
	StCS ("clsplcn.asm"),
	StCS ("fetchtbn.asm"),
	StCS ("rtfinn.asm"),
	StCS ("rtfsubsn.asm"),
	StCS ("prln.asm"),
	StCS ("disptbn2.asm"),
	StCS ("resn2.asm"),
	StCS ("dialog1n.asm"),
	StCS ("index2n.asm"),
};


/* %%Function:AssertProcForNative %%Owner:BRADV */
EXPORT AssertProcForNative( mid, lbl )
int mid, lbl;
{
	char szFile [30];
	char far *stFile;

	Assert( mid >= 0 && mid < midMac);

	stFile = rgstMid[mid];
	bltbx((LPSTR)&stFile[1],(LPSTR)szFile,stFile[0]);
	szFile[stFile[0]] = 0;

	AssertProcMst(mstDbgAssert, szFile, lbl, NULL, fFalse /* fReport */);
	NatDummy();    /* for easy breakpointing after native Assert */
}


#ifndef SDMASSERTINFO

/* %%Function:AssertForSdm %%Owner:bobz */
AssertForSdm(szFile, line)
CHAR *szFile;
int line;
{
#else
	AssertForSdm()
				{
		CHAR *szFile = SzFrame("??");
		int line = 0;
#endif /* SDMASSERTINFO */

		AssertProcMst(mstDbgSDMAssert, szFile, line, NULL, fFalse /*fReport*/);
		}

/* %%Function:AssertProc %%Owner:BRADV */
	AssertProc( szFile, line )
			char szFile[];
	int line;
		{
		AssertProcMst(mstDbgAssert, szFile, line, NULL, fFalse /*fReport*/);
		}

/* %%Function:AssertSzProc %%Owner:BRADV */
	EXPORT AssertSzProc( szExtra, szFile, line )
			CHAR szExtra[];
	char szFile[];
	int line;
		{
		AssertProcMst(mstDbgAssertSz, szFile, line, szExtra, fFalse /*fReport*/);
		}


/* %%Function:ReportSzProc %%Owner:BRADV */
	EXPORT ReportSzProc( szExtra, szFile, line )
			CHAR szExtra[];
	char szFile[];
	int line;
		{
		if (vdbs.fReports)
			AssertProcMst(mstDbgReport, szFile, line, szExtra, fTrue /*fReport*/);
		}

/* %%Function:FreezeProc %%Owner:BRADV */
	EXPORT FreezeProc( szFile, line )
			CHAR szFile[];
	int line;
		{
		AssertSzProc((CHAR *)szFreeze, szFile, line );
		}


/* indirectly invoked by Assert, AssertId and Report macros */

	STATIC BOOL fInAssert = fFalse;

/* %%Function:AssertDbgInt3 %%Owner:BRADV */
	EXPORT AssertDbgInt3()
				{
		AssertProcMst(mstDbgInt3, NULL, 0, NULL, fFalse);
		}

/* %%Function:AssertProcMst %%Owner:BRADV */
	EXPORT AssertProcMst(mst, szFile, line, szExtra, fReport)
			MST mst;
	char *szFile;
	int line;
	char *szExtra;
	int fReport;        /* (mst == mstDbgReport) */
		{
		extern HWND    vhwndMsgBoxParent;
		extern FARPROC lpDialogAlert;
		extern int cHpFreeze;
		extern HWND vhwndApp;
		extern HWND vhwndDeskTop;

		int idi;
		int mb = MB_ABORTRETRYIGNORE | MB_SYSTEMMODAL |
		( !fReport ? (vdbs.fRetryDef ? MB_DEFBUTTON2 : MB_DEFBUTTON1) :
		MB_DEFBUTTON3);
		int rgw[3];
		CHAR sz [256];
		CHAR szHeader [40];

		rgw[0] = szFile;
		rgw[1] = line;
		rgw[2] = szExtra;

		FreezeHp();
		EnterDebug();
		BuildStMstRgw(mst, rgw, sz, 256, NULL);
		StToSzInPlace(sz);
		CommSzSz(sz, szEmpty);

#ifdef BATCH
		if (vfBatchMode)
			BatchModeError(sz, NULL, 0, 0);
#endif /* BATCH */

/*  Assert is not allowed during an assert! */
		if (fInAssert)
			{
			MeltHp();
			ExitDebug();
			return;
			}
		fInAssert = fTrue;

/* Melt the heap, as we will perturb it when we lose the focus to the
	Assertion message box */

		CchCopySz( fReport ? SzShared("Debugging Report (ignorable)") :
				SzShared("Debugging Assert"),
				szHeader);

		do
			{
			WORD FAR * lpw;
			BYTE FAR * lpb;

			idi = MessageBox(NULL, (LPSTR)sz, (LPSTR)szHeader, mb);

			switch (idi)
				{
			case IDABORT:
				OurExitWindows();   /* does not return */
			case IDCANCEL:
			case IDIGNORE:
				break;
			default:
			case IDRETRY:
				DoStackTrace(sz);
				break;
				}
			}  
		while (idi == IDRETRY);

		fInAssert = fFalse;
		MeltHp();
		ExitDebug();
		} /* end of AssertProc */

/* %%Function:ScribbleProc %%Owner:BRADV */
	EXPORT ScribbleProc( dchPos, ch )
			int dchPos;
	CHAR ch;
		{    /* Scribble a char dchPos char positions from the UR screen corner */

		extern HWND vhwndApp;
		extern struct DBS vdbs;

		static unsigned dxpScribbleChar=0;
		static unsigned dypScribbleChar;
		static unsigned ypScribble;

		struct RC rc;
		HDC hdc;
		int xp;

/* See if this scribble is masked out */

		if (dchPos == ispNil || !(vdbs.grpfScribble & 1 << dchPos))
			return;

		EnterDebug();

		if (vhwndApp == NULL)
			goto LRet;

		GetWindowRect( vhwndApp, (LPRECT) &rc );
		if (FEmptyRc( &rc ))
			goto LRet;

		hdc = GetWindowDC( vhwndApp );
		if (hdc == NULL)
			goto LRet;

		if ( dxpScribbleChar == 0 )
			{   /* First time through */
			TEXTMETRIC tm;

			GetTextMetrics( hdc, (LPTEXTMETRIC)&tm );
			dxpScribbleChar = tm.tmAveCharWidth;
			dypScribbleChar = tm.tmHeight + tm.tmInternalLeading;
			ypScribble = GetSystemMetrics(SM_CYDLGFRAME);
/*             + GetSystemMetrics (SM_CYMENU)/2 - dypScribbleChar/2; */
			}
/*  note: using DLGFRAME since THICKBORDER not yet defined */
		xp = (rc.xpRight - rc.xpLeft - GetSystemMetrics (SM_CXDLGFRAME) - 1 -
				4 * vsci.dxpScrlBar)
				- (dxpScribbleChar * (dchPos+1));

		PatBlt( hdc, xp, ypScribble, dxpScribbleChar, dypScribbleChar, WHITENESS );
		TextOut( hdc, xp, ypScribble, (LPSTR) &ch, 1 );
		ReleaseDC( (HWND)vhwndApp, hdc );

LRet:
		ExitDebug();
		return;
		}


/* C K   S T R U C T */
/* DESCRIPTION:
* Checks that cbFOO is even for all our structs, and that it is
* twice cwFOO (if both exist).
*/

/* %%Function:CkStruct %%Owner:BRADV */
	CkStruct()
				{
		int iAssert;

		if (!vdbs.fCkStruct) return;

		iAssert = 1;
		Assert(!(sizeof(struct EDL) % sizeof(int)));
		iAssert = 2;
		Assert(!(sizeof(struct AONNODE) % sizeof(int)));
		iAssert = 3;
		Assert(!(sizeof(struct WWD) % sizeof(int)));
		iAssert = 4;
		Assert(!(sizeof(struct PCD) % sizeof(int)));
		iAssert = 10;
		Assert(!(sizeof(struct SED) % sizeof(int)));
		iAssert = 11;
		Assert(!(sizeof(struct PGD) % sizeof(int)));
		iAssert = 12;
		Assert(!(sizeof(struct ESTCP) % sizeof(int)));
		iAssert = 13;
		Assert(!(sizeof(struct DOD) % sizeof(int)));
		iAssert = 14;
		Assert(!(sizeof(struct BTE) % sizeof(int)));
		iAssert = 15;
		Assert(!(sizeof(struct FCB) % sizeof(int)));
		iAssert = 16;
		Assert(!(sizeof(struct FIB) % sizeof(int)));
		iAssert = 17;
		Assert(!(sizeof(struct CHR) % sizeof(int)));
		iAssert = 18;
		Assert(!(sizeof(struct DR) % sizeof(int)));
		iAssert = 19;
		Assert(!(sizeof(struct LR) % sizeof(int)));
		iAssert = 20;
		Assert(!(sizeof(struct LBS) % sizeof(int)));
		iAssert = 23;
		Assert(!(sizeof(struct CHP) % sizeof(int)));
		iAssert = 24;
		Assert(!(sizeof(struct PAP) % sizeof(int)));
		iAssert = 25;
		Assert(!(sizeof(struct SEP) % sizeof(int)));
		iAssert = 26;
		Assert(!(sizeof(struct SCC) % sizeof(int)));
		iAssert = 27;
		Assert(!(sizeof(struct SEL) % sizeof(int)));
		iAssert = 29;
		Assert(!(sizeof(struct RC) % sizeof(int)));
		iAssert = 30;
		Assert(!(sizeof(struct DOP) % sizeof(int)));
		iAssert = 31;
		Assert(!(sizeof(struct PREF) % sizeof(int)));
		iAssert = 32;
		Assert(!(sizeof(struct CA) % sizeof(int)));
		iAssert = 33;
		Assert(!(sizeof(struct SELS) % sizeof(int)));
		iAssert = 35;
		Assert(!(cbSector % sizeof(int)));
		iAssert = 37;
		Assert(!(cbPAPBase % sizeof(int)));
		iAssert = 38;
		Assert(!(offset(STTB, rgbst) % sizeof(int)));
		iAssert = 39;
		Assert(!(sizeof(struct PAD) % sizeof(int)));
		iAssert = 40;
		Assert(!(sizeof(struct OFH) % sizeof(int)));
		iAssert = 41;
		Assert(!(sizeof(struct PAPS) % sizeof(int)));
		CkDefinesRtf();

		/*  this is just to assure no two chrms have the same value */
		switch (iAssert)
			{
		case chrmChp:
		case chrmTab:
		case chrmVanish:
		case chrmFormula:
		case chrmDisplayField:
		case chrmFormatGroup:
		case chrmEnd:
		default:
			Assert (fTrue);
			}


		}




/* %%Function:CkFont %%Owner:BRADV */
	EXPORT CkFont()
				{
		extern int vflm;
		extern struct FCE rgfce[];
		extern struct FTI vfti;
		extern struct FTI vftiDxt;
		extern struct PRI vpri;
		int iAssert;
		int cfceNotFree;
		int cfceChain;
		int ifce;

		iAssert = -1;
		cfceChain = CfceCkPfti( &vfti );
		iAssert = -2;
		cfceChain += CfceCkPfti( &vftiDxt );

		for ( ifce = 0, cfceNotFree = 0; ifce < ifceMax; ifce++ )
			if (rgfce [ifce].hfont != NULL)
				cfceNotFree++;

		iAssert = 1;
		Assert( cfceNotFree == cfceChain );

/* Assure that FormatLine mode and associated stuff is set up correctly */
		switch (vflm)
			{
		default:
			iAssert = 2;
			Assert( fFalse );
			break;
		case flmPrint:
		case flmRepaginate:
			iAssert = 3;
			Assert( vfli.fPrint );
			iAssert = 4;
			Assert( vfti.fPrinter );
			Assert( vfli.dxuInch != 0 && vfli.dyuInch != 0 );
			iAssert = 5;
			Assert( vfti.dxpInch == vfli.dxuInch && vfti.dypInch == vfli.dyuInch );
/* FormatLine expects the following non-intuitive condition */
			iAssert = 7;
			Assert( vftiDxt.dxpInch == vfli.dxuInch && vftiDxt.dypInch == vfli.dyuInch );
			break;
		case flmDisplayAsPrint:
			iAssert = 8;
			Assert( vfli.fFormatAsPrint && !vfli.fPrint );
			iAssert = 9;
			Assert( !vfti.fPrinter && vftiDxt.fPrinter );
			iAssert = 10;
			Assert( vfti.dxpInch == vfli.dxsInch && vfti.dypInch == vfli.dysInch );
			iAssert = 11;
			Assert( vftiDxt.dxpInch == vfli.dxuInch && vftiDxt.dypInch == vfli.dyuInch );
			break;
		case flmDisplay:
			iAssert = 13;
			Assert( !vfli.fFormatAsPrint && !vfli.fPrint );
			iAssert = 14;
			Assert( !vfti.fPrinter );
			iAssert = 15;
			Assert( vfti.dxpInch == vfli.dxsInch && vfti.dypInch == vfli.dysInch );
/* FormatLine expects the following non-intuitive condition */
			iAssert = 17;
			Assert( vftiDxt.dxpInch == vfli.dxsInch && vftiDxt.dypInch == vfli.dysInch );
			break;
		case flmIdle:
			break;
			}
		}



/* %%Function:CfceCkPfti %%Owner:BRADV */
	CfceCkPfti( pfti )
			struct FTI *pfti;
		{
		int iAssert;
		int ibDiff = -1;
		char rgbFti [cbFtiFceSame];
		char rgbFce [cbFtiFceSame];
		int fFoo = 0xDDDD;
		int fPrinter = pfti->fPrinter;
		int ipfce;
		struct FCE *pfce, *rgpfce [ifceMax];


		if (pfti->fcid.lFcid != fcidNil)
			{
			iAssert = 2;
			Assert( pfti->pfce != NULL );
			iAssert = 3;
			if (FNeRgch( pfti, pfti->pfce, cbFtiFceSame ))
				{
/* for deciphering bug 701: put more info on the frame so we 
	can figure out what's going on from the RIP file. */
				bltbyte ( pfti, rgbFti, cbFtiFceSame );
				bltbyte ( pfti->pfce, rgbFce, cbFtiFceSame );
				Assert( fFalse );
				}
			iAssert = 5;
			Assert( pfti->hfont == NULL || pfti->hfont == pfti->pfce->hfont );
			}

/* Now check chain of fonts leading from pfti */

		rgpfce [0] = pfti->pfce;
		for ( ipfce = 0; (pfce = rgpfce [ipfce]) != NULL; )
			{
			int ipfceT;

		/* Check for circularities in the chain */
			for ( ipfceT = 0; ipfceT < ipfce; ipfceT++ )
				{
				iAssert = 7;
				Assert( rgpfce [ipfceT] != pfce );
				}
			CkPfcid( &pfce->fcidActual );
			CkPfcid( &pfce->fcidRequest );

/* following checks for a special case placeholder used for
	DisplayAsPrint when we couldn't get a printer DC -- it causes
	the system font to be selected into the corresponding screen DC */
			if (pfce->fcidActual.ibstFont == ibstFontNil)
				{
				goto LSkipFceChecks;
				}

			iAssert = 8;
/* because of weirdness in PSCRIPT & HPPLOT drivers, following are
	the only statements we can make */
			Assert( pfce->dxpOverhang >= 0 );
			Assert( pfce->dypAscent + pfce->dypDescent > 0 );
			iAssert = 9;
			Assert( pfce->hfont != NULL );
			iAssert = 10;
			Assert( pfce->fPrinter == fPrinter );
			if (pfce->fFixedPitch)
				{
				iAssert = 11;
				Assert( (int)pfce->dxpWidth > 0 );
				}
			else
				{
				iAssert = 12;
				Assert( pfce->hqrgdxp != hqNil );
				}

LSkipFceChecks:
			if (ipfce == 0)
				{
				iAssert = 14;
				Assert( pfce->pfcePrev == NULL );
				}
			else
				{
				iAssert = 15;
				Assert( pfce->pfcePrev = rgpfce [ipfce - 1] );
				iAssert = 16;
				Assert( pfce->pfcePrev->pfceNext == pfce );
				}
			++ipfce;

			if (pfce->pfceNext == NULL)
				break;

			iAssert = 17;
			Assert( ipfce < ifceMax );
			iAssert = 18;
			Assert( pfce->pfceNext->pfcePrev == pfce );

			rgpfce [ipfce] = pfce->pfceNext;
			}

		return ipfce;
		}

/* %%Function:CkPfcid %%Owner:BRADV */
	CkPfcid( pfcid )
			struct FCID *pfcid;
		{
		int iAssert;
		extern struct STTB **vhsttbFont;

		iAssert = 1;
		Assert( pfcid->lFcid != fcidNil );
		iAssert = 2;
		Assert( pfcid->prq == DEFAULT_PITCH || pfcid->prq == FIXED_PITCH ||
				pfcid->prq == VARIABLE_PITCH );
		iAssert = 3;
		Assert( pfcid->ibstFont < (*vhsttbFont)->ibstMac ||
				pfcid->ibstFont == ibstFontNil );
		}

/* globals used for CkLHRUpd - done when DEBUG off */
/*  these are set in FGetChar/ParaState and indicate the selection at that
	time */
	struct CA  vcaLHRUpd;
	int   vwwLHRUpd; /* window where selection is shown */

	CkLHRUpd()
				{
#ifdef REVIEW /* REVIEW iftime: most of this is bogus (rp) */
/* if we got this far, we claim that selection and properties
	are unchanged, or the LH and Ruler were updated, so the
	ruler and looks helper do not need updating. Verify that claim.
*/
		int iAssert;
		extern struct CHP  chpRibbon;   /* these hold current state of lookshelper */
		extern struct CHP  chpRibbonGray;
		extern HWND vhwndRibbon;
		struct CHP  chp;
		struct CHP  chpGray;
		struct PAP  pap;
		struct PAP  papGray;
		struct RSD ** hrsd;

		if (hwwdCur == hNil || (vhwndRibbon == NULL &&
				((*hmwdCur)->hwndRuler == hNil )))
			return;

		/* has selection changed since last update?  - test range and pane */
		/* be sure this is checked before FGet????State is called */

		iAssert = 1;

		/* you cannot assert this because of some games played by insert */
/*         Assert (!FNeRgch(&selCur.ca, &vcaLHRUpd, sizeof (struct CA))); */
		Assert (selCur.ww == vwwLHRUpd);

		if (FMsgPresent(mtyAny | mtyYield))
			return fTrue;

		/* test chps */
		iAssert = 2;
		if (vhwndRibbon != NULL)
			if (selCur.fUpdateChp || selCur.fUpdateChpGray ||
					selCur.fUpdateRibbon)
				{
				Assert (fFalse);
				return;
				}

		if (FMsgPresent(mtyAny | mtyYield))
			return fTrue;

		iAssert = 3;
		if ((*hmwdCur)->hwndRuler != hNil )
			if (selCur.fUpdatePap || selCur.fUpdateRuler)
				{
				Assert (fFalse);
				return;
				}
			else
				{
				/* save current paps, get props in selCur paps and compare */
				/*  comparing all except individual tabs (do check # tabs) */
				pap = vpapSelCur;
				papGray = vpapGraySelCur;
				if (FGetParaState(fFalse, /*fAll*/ fTrue /*fAbortOk*/) == fFalse)
					return;
				Assert (!FNeRgch (&vpapSelCur, &pap, cbPAPBase));
				Assert (!FNeRgch (&vpapGraySelCur, &papGray, cbPAPBase));
				/* are ruler paps up to date? */
				hrsd = (struct RSD **)
						GetWindowWord ((*hmwdCur)->hwndRuler, IDWRULHRSD);
				Assert (!FNeRgch(&vpapSelCur, &(*hrsd)->pap, cbPAPBase));
				Assert (!FNeRgch(&vpapGraySelCur, &(*hrsd)->papGray,
						cbPAPBase));
				}

		if (FMsgPresent(mtyAny | mtyYield))
			return fTrue;

		/* has selection changed since last update?  - test range and pane */

		iAssert = 3;

		/* as above, you cannot assert this */
/*         Assert (!FNeRgch(&selCur.ca, &vcaLHRUpd, sizeof (struct CA))); */
		Assert (selCur.ww == vwwLHRUpd);
#endif /* REVIEW iftime */
		}

/* %%Function:CkDefinesRtf %%Owner:BRADV */
	CkDefinesRtf()
				{
	/* RTF uses hard coded defines for bit fields. This is a test to ensure
		that the bit positions don't change.   Defines are in rtf.h
	*/
/* bit and word "offsets" for bit fields used in the rgrrbword table
	ibitXname is the bit index of field name in struct X
	iwordeXname is the word index of field name in struct X

		where X is
			D for DOP
			S for SEP
			P for PAP
			C for CHP
			PIC for PIC
			R for RIBL
	these should be used in a test routine that assures that the structures
	referenced do not change and invalidate the values.

*/

/* Scheme: clear out a structure, then assign true to each field
	using the defines as a location. Then test that the field is
	true using the field name

	When adding such defines to rtf, update this function

*/

		int iAssert;
		struct CHP chp;
		struct DOP dop;
		struct RIBL ribl;

		int *pw;
		int *pprop;

		SetBytes(&dop, 0, cbDOP);
		SetBytes(&chp, 0, cbCHP);
		SetWords(&ribl, 0, cwRIBL);

		pprop = (int *)&chp;

		iAssert = 1;
		pw = &pprop[iwordCfBold];
		*pw |= (1 << ibitCfBold);
		Assert (chp.fBold);

		iAssert = 2;
		pw = &pprop[iwordCfItalic];
		*pw |= (1 << ibitCfItalic);
		Assert (chp.fItalic);

		iAssert = 3;
		pw = &pprop[iwordCfStrike];
		*pw |= (1 << ibitCfStrike);
		Assert (chp.fStrike);

		iAssert = 4;
		pw = &pprop[iwordCfOutline];
		*pw |= (1 << ibitCfOutline);
		Assert (chp.fOutline);

		iAssert = 6;
		pw = &pprop[iwordCfSmallCaps];
		*pw |= (1 << ibitCfSmallCaps);
		Assert (chp.fSmallCaps);

		iAssert = 7;
		pw = &pprop[iwordCfCaps];
		*pw |= (1 << ibitCfCaps);
		Assert (chp.fCaps);

		iAssert = 8;
		pw = &pprop[iwordCfVanish];
		*pw |= (1 << ibitCfVanish);
		Assert (chp.fVanish);

		iAssert = 9;
		pw = &pprop[iwordCfRMark];
		*pw |= (1 << ibitCfRMark);
		Assert (chp.fRMark);



		pprop = (int *)&dop;

		iAssert = 10;
		pw = &pprop[iwordDfFacingPages];
		*pw |= (1 << ibitDfFacingPages);
		Assert (dop.fFacingPages);

		iAssert = 11;
		pw = &pprop[iwordDfWidowControl];
		*pw |= (1 << ibitDfWidowControl);
		Assert (dop.fWidowControl);

		iAssert = 12;
		pw = &pprop[iwordDfFtnRestart];
		*pw |= (1 << ibitDfFtnRestart);
		Assert (dop.fFtnRestart);

		iAssert = 13;
		pw = &pprop[iwordDfWide];
		*pw |= (1 << ibitDfWide);
		Assert (dop.fWide);

		iAssert = 14;
		pw = &pprop[iwordDfRevMarking];
		*pw |= (1 << ibitDfRevMarking);
		Assert (dop.fRevMarking);

		iAssert = 15;
		pw = &pprop[iwordDfBackup];
		*pw |= (1 << ibitDfBackup);
		Assert (dop.fBackup);

		iAssert = 16;
		pw = &pprop[iwordDfMirrorMargins];
		*pw |= (1 << ibitDfMirrorMargins);
		Assert (dop.fMirrorMargins);

		/* these asserts assure that used riblof values will fit in 6 bits */
		iAssert = 17;
		Assert(riblof(pgnStart) <= 0x3f);
		Assert(riblof(lineStart) <= 0x3f);
		Assert(riblof(chs) <= 0x3f);
		Assert(riblof(fmc) <= 0x3f);
		Assert(riblof(cBlue) <= 0x3f);
		Assert(riblof(cGreen) <= 0x3f);
		Assert(riblof(cRed) <= 0x3f);
		Assert(riblof(estcp.stcBase) <= 0x3f);
		Assert(riblof(estcp.stcNext) <= 0x3f);
		Assert(riblof(iGrid) <= 0x3f);

		pprop = (int *)&ribl;

		iAssert = 18;
		pw = &pprop[iwordRfRevisions];
		*pw |= (1 << ibitRfRevisions);
		Assert (ribl.fRevisions);



		}




/* %%Function:SlapWnd %%Owner:BRADV */
	SlapWnd()
				{
		/* dumps either the dialog, if up, or the window with the focus
			to the clipboard */

		HDC  hdcWnd;
		HDC  hdcShadow;
		RECT rc;
		HBITMAP hbmShadow;
		int width, height;
		HWND hwndDump;
		extern int vTestClip;
		extern int vfAwfulNoise;
		int yp1mm, xp1mm;

		Beep();  /* just a warning */
		if ( (hwndDump = GetFocus()) == NULL )
			return;


		hdcWnd = GetWindowDC(hwndDump);
		hdcShadow = CreateCompatibleDC(hdcWnd);
		GetWindowRect(hwndDump, (LPRECT)&rc);
		hbmShadow = CreateCompatibleBitmap(hdcShadow,
				width = rc.right - rc.left, height = rc.bottom - rc.top);
		LogGdiHandle(hbmShadow, 1017);
		SelectObject(hdcShadow, hbmShadow);
		BitBlt(hdcShadow, 0,0, width, height, hdcWnd, 0, 0, SRCCOPY);

		/* convert size to .1mm and use as bitmap dim */

		xp1mm = NMultDiv (width, czaInch, vfli.dxsInch);
		xp1mm = NMultDiv (xp1mm, 100, czaCm);

		yp1mm = NMultDiv (height, czaInch, vfli.dysInch);
		yp1mm = NMultDiv (yp1mm, 100, czaCm);

		SetBitmapDimension( hbmShadow, xp1mm, yp1mm);


		if (OpenClipboard(hwndDump))
			{
			EmptyClipboard();
			SetClipboardData(CF_BITMAP, hbmShadow);
			CloseClipboard();
			}
		ReleaseDC(hwndDump, hdcWnd);
		DeleteDC(hdcShadow);


		vfAwfulNoise = fFalse;
		Beep();  /* just a warning  - done */

	/* used for testing paste - forces us to paste in from clipboard
		even if we owned it.  Set in SlapWnd and for debugging, turned off when
		we load something into the clipboard.
	*/
		vTestClip = CF_BITMAP;
		}


/* C A C H E   P A R A   V E R I F Y */
/* called instead of CachePara when vdbs.CCachePara == 2 is on. */
/* calls both C and native versions of CachePara, and compares
	the results */

/* %%Function:V_CachePara %%Owner:BRADV */
EXPORT V_CachePara(doc, cp)
	int doc;
	CP cp;
		{
		extern struct PAP       vpapFetch;
		extern struct CA        caPara;
		extern struct CHP       vchpStc;
		extern struct PAP	    vpapStc;
		extern struct TAP	    vtapStc;
		extern int		    vstcLast;
		extern int		    vcchPapxLast;
		extern int		    vdocPapLast;
		extern int		    vfnPapLast;
		extern int		    vpnPapLast;
		extern int		    vbpapxPapLast;

		struct PAP	     papFetchNew, papFetchSav;
		struct CA	     caParaNew, caParaSav;
		struct CHP	     chpStcNew, chpStcSav;
		struct PAP	     papStcNew, papStcSav;
		struct TAP	     tapStcNew, tapStcSav;
		int 	     stcLastNew, stcLastSav;
		int 	     cchPapxLastNew, cchPapxLastSav;
		int 	     docPapLastNew, docPapLastSav;
		int 	     fnPapLastNew, fnPapLastSav;
		int 	     pnPapLastNew, pnPapLastSav;
		int 	     bpapxPapLastNew, bpapxPapLastSav;

		bltbyte(&vpapFetch, &papFetchSav, sizeof (struct PAP));
		bltbyte(&caPara, &caParaSav, sizeof (struct CA));
		bltbyte(&vchpStc, &chpStcSav, sizeof (struct CHP));
		bltbyte(&vpapStc, &papStcSav, sizeof (struct PAP));
		bltbyte(&vtapStc, &tapStcSav, sizeof (struct TAP));
		stcLastSav = vstcLast;
		cchPapxLastSav = vcchPapxLast;
		docPapLastSav = vdocPapLast;
		fnPapLastSav = vfnPapLast;
		pnPapLastSav = vpnPapLast;
		bpapxPapLastSav = vbpapxPapLast;

		vfInCacheParaVerify = fTrue;
		C_CachePara(doc,cp);
		vfInCacheParaVerify = fFalse;

/* (1) Duplicate pre-CachePara environment from before C call
	(2) Call native version of CachePara
	(3) Compare results
*/

		bltbyte(&vpapFetch, &papFetchNew, sizeof (struct PAP));
		bltbyte(&caPara, &caParaNew, sizeof (struct CA));
		bltbyte(&vchpStc, &chpStcNew, sizeof (struct CHP));
		bltbyte(&vpapStc, &papStcNew, sizeof (struct PAP));
		bltbyte(&vtapStc, &tapStcNew, sizeof (struct TAP));
		stcLastNew = vstcLast;
		cchPapxLastNew = vcchPapxLast;
		docPapLastNew = vdocPapLast;
		fnPapLastNew = vfnPapLast;
		pnPapLastNew = vpnPapLast;
		bpapxPapLastNew = vbpapxPapLast;

		bltbyte(&papFetchSav, &vpapFetch, sizeof (struct PAP));
		bltbyte(&caParaSav, &caPara, sizeof (struct CA));
		bltbyte(&chpStcSav, &vchpStc, sizeof (struct CHP));
		bltbyte(&papStcSav, &vpapStc, sizeof (struct PAP));
		bltbyte(&tapStcSav, &vtapStc, sizeof (struct TAP));
		vstcLast = stcLastSav;
		vcchPapxLast = cchPapxLastSav;
		vdocPapLast = docPapLastSav;
		vfnPapLast = fnPapLastSav;
		vpnPapLast = pnPapLastSav;
		vbpapxPapLast = bpapxPapLastSav;

		N_CachePara(doc,cp);

		Assert(!FNeRgch(&vpapFetch, &papFetchNew, sizeof (struct PAP)));
		Assert(!FNeRgch(&caPara, &caParaNew, sizeof (struct CA)));
		Assert(!FNeRgch(&vchpStc, &chpStcNew, sizeof (struct CHP)));
		Assert(!FNeRgch(&vpapStc, &papStcNew, sizeof (struct PAP)));
		Assert(!FNeRgch(&vtapStc, &tapStcNew, sizeof (struct TAP)));
		Assert(vstcLast == stcLastNew);
		Assert(vcchPapxLast == cchPapxLastNew);
		Assert(vdocPapLast == docPapLastNew);
		Assert(vfnPapLast == fnPapLastNew);
		Assert(vbpapxPapLast == bpapxPapLastNew);
		}



/* F O R M A T   L I N E   V E R I F Y */
/* called instead of FormatLine when vdbs.CFormatLine == 2. */
/* calls both C and native versions of FormatLine, and compares
	the results */

/* %%Function:V_FormatLineDxa %%Owner:BRADV */
EXPORT V_FormatLineDxa( ww, doc, cp, dxa)
	int ww, doc;
	CP cp;
	int dxa;
		{
		extern int vflm;
		extern int vbchrMac;
		extern int vbchrMax;
		extern char **vhgrpchr;
		char *pb1, *pb2;
		int ib;
		int cb;
		char **hgrpchrSave;
		int bchrMac;
		int cDifference=0;
		struct FLI fli, fliT;


/* (1) remember/set-up pre-FormatLine environment
	(2) call C version of FormatLine
	(3) save results
*/

		InvalFli();		/* force FormatLine to act */
		bltb( &vfli, &fliT, sizeof (struct FLI) );
		SetBytes( *vhgrpchr, 0x71, vbchrMax );
		C_FormatLineDxa(ww,doc,cp,dxa);
		bltb( &vfli, &fli, sizeof (struct FLI) );
			{{ /* native compiler bug */
			hgrpchrSave = HAllocateCw(CwFromCch(bchrMac = vbchrMac));
			}}
		Assert( hgrpchrSave );
		bltb( *vhgrpchr, *hgrpchrSave, bchrMac );

/* (1) Duplicate pre-FormatLine environment from before C call
	(following fields are part of the global FLM and are preserved:
		fPrint, fFormatAsPrint, fOutline(?), d??Inch
	(2) Call native version of FormatLine
	(3) Compare results
*/

		bltb( &fliT, &vfli, offset(FLI,fPrint) );
		ib = offset(FLI,ichSpace);
		bltb( ((char *)&fliT) + ib, ((char *)&vfli) + ib, sizeof(struct FLI)-ib);
		SetBytes( *vhgrpchr, 0x71, vbchrMax );
		N_FormatLineDxa(ww,doc,cp,dxa);
		if (vfli.fOutline)
			vfli.fOutline = fTrue;	/* Set to 0x80 by N_FormatLineDxa */
		if (vfli.fPageView)
			vfli.fPageView = fTrue;	/* Set to 0x08 by N_FormatLineDxa */
		if (vfli.fAdjustForVisi)
			vfli.fAdjustForVisi = fTrue;     /* Set to 0x02 by N_FormatLineDxa */
		pb1 = &fli;
		pb2 = &vfli;
		for ( ib = 0; ib < sizeof (struct FLI); ib++ )
			{
			if (*pb1++ != *pb2++)
				{
				CommSzNum(SzShared("\007FLI difference at offset\007 = "),ib );
				CommSzNum(SzShared("ww = "),ww );
				CommSzNum(SzShared("cp = "), (int)cp);
				CommSzNum(SzShared("CS byte = "),*(pb1-1) );
				CommSzNum(SzShared("Native byte = "),*(pb2-1) );
				if (cDifference++ >= 3)
					goto LRet;
				}
			}

		if (vbchrMac != bchrMac)
			{
			CommSzSz(SzShared("\007bchrMac difference\007"),szEmpty);
			CommSzNum(SzShared("ww = "),ww );
			CommSzNum(SzShared("cp = "), (int)cp);
			CommSzNum(SzShared("CS value = "),bchrMac );
			CommSzNum(SzShared("Native value"),vbchrMac );
			if (cDifference++ >= 3)
				goto LRet;
			}

		pb1 = *hgrpchrSave;
		pb2 = *vhgrpchr;
		for ( ib = 0; ib < bchrMac; ib++ )
			{
			if (*pb1++ != *pb2++)
				{
				CommSzNum(SzShared("\007GRPCHR difference at offset\007 = "),ib );
				CommSzNum(SzShared("ww = "),ww );
				CommSzNum(SzShared("cp = "), (int)cp);
				CommSzNum(SzShared("CS byte = "),*(pb1-1) );
				CommSzNum(SzShared("Native byte = "),*(pb2-1) );
				if (cDifference++ >= 3)
					goto LRet;
				}
			}

		Debug(vdbs.fCkFli ? CkVfli() : 0);
LRet:   
		FreeH(hgrpchrSave);
		}


/* F E T C H  C P  V E R I F Y */
/* called instead of FetchCp when vdbs.CFetchCp == 2. */
/* Checks native FetchCp */

/* %%Function:V_FetchCp %%Owner:BRADV */
EXPORT V_FetchCp(doc,cp,fcm)
	int doc;
	CP cp;
	int fcm;
		{
		extern int vdocFetch;
		extern CP vcpFetch;
		extern int vccpFetch;
		extern int fcmFetch;
		extern FC fcFetch;
		extern CP ccpChp;
		extern CP ccpPcd;
		extern int ccpFile;
		extern int vfEndFetch;
		extern int ipcdFetch;
		extern int fnFetch;
		extern int prmFetch;
		extern struct CHP vchpFetch;
		extern char rgchCaps[];
		extern CHAR HUGE *vhpchFetch;
		CP cpFetchOld = vcpFetch;
		int docFetchOld = vdocFetch;
		int fcmFetchOld = fcmFetch;
		int ccpFetchOld = vccpFetch;
		int docFetchNew;
		CP cpFetchNew;
		int ccpFetchNew;
		int fcmFetchNew;
		FC fcFetchNew;
		CP ccpChpNew;
		CP ccpPcdNew;
		int ccpFileNew;
		int fEndFetchNew;
		int ipcdFetchNew;
		int fnFetchNew;
		int prmFetchNew;
		CHAR rgchNew [128];
		struct CHP chpFetchNew;

		ClearFetchVariables();
		if (doc == docNil)
			{
			C_CachePara( docFetchOld, cpFetchOld );
			C_FetchCp( docFetchOld, cpFetchOld, fcmFetchOld );
			C_CachePara( docFetchOld, cpFetchOld + ccpFetchOld );
			}
		C_FetchCp( doc, cp, fcm );

/* Verify everything possible about the results */
		if (doc == docNil)
			{
			Assert( vdocFetch < docMac && mpdochdod [vdocFetch] != hNil );
			fcmFetch = fcm;
			}
		else
			{
			Assert( vcpFetch == cp );
			Assert( vdocFetch == doc );
			}
		Assert( fcmFetch == fcm );
		Assert( vfEndFetch || vccpFetch > 0 );
		Assert( doc == docNil || vfEndFetch == (cp >= CpMacDoc(vdocFetch)) );
		Assert( vfEndFetch || vccpFetch <= ccpPcd );
		if ((fcmFetch & fcmChars) && !vfEndFetch)
			{
			if (vhpchFetch == (CHAR HUGE *)rgchCaps)
				Assert( vccpFetch <= ichCapsMax );
			}

		docFetchNew = vdocFetch;
		cpFetchNew = vcpFetch;
		ccpFetchNew = vccpFetch;
		fcmFetchNew = fcmFetch;
		fcFetchNew = fcFetch;
		ccpChpNew = ccpChp;
		ccpPcdNew = ccpPcd;
		ccpFileNew = ccpFile;
		fEndFetchNew = vfEndFetch;
		ipcdFetchNew = ipcdFetch;
		fnFetchNew = fnFetch;
		prmFetchNew = prmFetch;
		if (fcm & fcmChars)
			{
			Assert( vccpFetch <= 512 );
			bltbh( vhpchFetch, rgchNew, umin(vccpFetch,128) );
			}
		blt( &vchpFetch, &chpFetchNew, cwCHP );

		ClearFetchVariables();
		if (doc == docNil)
			{
			C_CachePara( docFetchOld, cpFetchOld );
			C_FetchCp( docFetchOld, cpFetchOld, fcmFetchOld );
			C_CachePara( docFetchOld, cpFetchOld + ccpFetchOld );
			}
		N_FetchCp( doc, cp, fcm );

/* Verify everything possible about the results */
		if (doc == docNil)
			{
			Assert( vdocFetch < docMac && mpdochdod [vdocFetch] != hNil );
			fcmFetch = fcm;
			}
		else
			{
			Assert( vcpFetch == cp );
			Assert( vdocFetch == doc );
			}
		Assert( fcmFetch == fcm );
		Assert( vfEndFetch || vccpFetch > 0 );
		Assert( doc == docNil || vfEndFetch == (cp >= CpMacDoc(vdocFetch)) );
		Assert( vfEndFetch || vccpFetch <= ccpPcd );
		if ((fcmFetch & fcmChars) && !vfEndFetch)
			{
			Assert( !FNeHprgch( HpFromPch(rgchNew), vhpchFetch, umin(vccpFetch,128)) );
			if (vhpchFetch == (CHAR HUGE *)rgchCaps)
				Assert( vccpFetch <= ichCapsMax );
			}

		Assert( docFetchNew == vdocFetch );
		Assert( cpFetchNew == vcpFetch );
		Assert( ccpFetchNew == vccpFetch );
		Assert( fcmFetchNew == fcmFetch );
		Assert( fcFetchNew == fcFetch );
		Assert( ccpChpNew == ccpChp );
		Assert( ccpPcdNew == ccpPcd );
		Assert( ccpFileNew == ccpFile );
		Assert( fEndFetchNew == vfEndFetch );
		Assert( ipcdFetchNew == ipcdFetch );
		Assert( fnFetchNew == fnFetch );
		Assert( prmFetchNew == prmFetch );
		Assert( !FNeRgw(&vchpFetch,&chpFetchNew,cwCHP) );
		}


/* %%Function:ClearFetchVariables %%Owner:BRADV */
ClearFetchVariables()
		{
		extern int vdocFetch;
		extern CP vcpFetch;
		extern int vccpFetch;
		extern int fcmFetch;
		extern FC fcFetch;
		extern CP ccpChp;
		extern CP ccpPcd;
		extern int ccpFile;
		extern int vfEndFetch;
		extern int ipcdFetch;
		extern int fnFetch;
		extern int prmFetch;
		extern CHAR HUGE *vhpchFetch;
		extern struct CHP vchpFetch;
		extern char rgchCaps[];
		extern int vibpProtect;

		vdocFetch = 0xCCCC;
		vcpFetch = 0x2CCCCCCCL; /* must be >0 for FetchCp to work */
		vccpFetch = 0x2CCC;	/* must be >0 for FetchCp to work */
		fcmFetch = 0xCCCC;
		fcFetch = 0x2CCCCCCCL;	/* must be >0 for FetchCp to work */
		ccpChp = 0x2CCCCCCCL;	/* must be >0 for FetchCp to work */
		ccpPcd = 0x2CCCCCCCL;	/* must be >0 for FetchCp to work */
		ccpFile = 0x2CCC;	/* must be >0 for FetchCp to work */
		vfEndFetch = 0xCCCC;
		ipcdFetch = 0x2CCC;	/* must be >0 for FetchCp to work */
		fnFetch = 0xCCCC;
		prmFetch = 0xCCCC;
		vhpchFetch = 0xCCCCCCCCL;
		SetBytes(&rgchCaps, 0xCC, ichCapsMax);
		vibpProtect = 0xCCCC;
		SetBytes(&vchpFetch, 0xCC, sizeof(struct CHP));
		}


/* %%Function:V_DcpSkipFieldChPflcd %%Owner:BRADV */
EXPORT CP V_DcpSkipFieldChPflcd (ch, pflcd, fShowResult, fFetch)
	CHAR ch;
	struct FLCD *pflcd;
	BOOL fShowResult, fFetch;

		{
		CP dcpNew, dcpReturn;
		CP C_DcpSkipFieldChPflcd();

		dcpNew = C_DcpSkipFieldChPflcd (ch, pflcd, fShowResult, fFetch);
		dcpReturn = N_DcpSkipFieldChPflcd (ch, pflcd, fShowResult, fFetch);
		Assert ( dcpNew == dcpReturn );
		return dcpReturn;
		}


/* %%Function:V_FShowResultPflcdFvc %%Owner:BRADV */
EXPORT V_FShowResultPflcdFvc(pflcd, fvc)
	struct FLCD * pflcd;
	int fvc;
		{
		int fNew, fReturn;

		fNew = C_FShowResultPflcdFvc(pflcd, fvc);
		fReturn = N_FShowResultPflcdFvc(pflcd, fvc);
		Assert ( fNew == fReturn );
		return fReturn;
		}


/* %%Function:V_IfldFromDocCp %%Owner:BRADV */
EXPORT V_IfldFromDocCp (doc, cp, fMatch)
	int doc;
	CP cp;
	BOOL fMatch;

		{
		int ifldNew, ifldReturn;

		ifldNew = C_IfldFromDocCp (doc, cp, fMatch);
		ifldReturn = N_IfldFromDocCp (doc, cp, fMatch);
		Assert ( ifldNew == ifldReturn );
		return ifldReturn;
		}


/* %%Function:V_FillIfldFlcd %%Owner:BRADV */
EXPORT V_FillIfldFlcd (hplcfld, ifld, pflcd)
	struct PLC **hplcfld;
	int ifld;
	struct FLCD *pflcd;

		{
		struct FLCD flcdNew;

		C_FillIfldFlcd (hplcfld, ifld, &flcdNew);
		N_FillIfldFlcd (hplcfld, ifld, pflcd);
		if (pflcd->fDirty)
			pflcd->fDirty = fTrue;
		Assert (!FNeRgch(&flcdNew, pflcd, sizeof (struct FLCD)));
		}


/* %%Function:V_GetIfldFlcd %%Owner:BRADV */
EXPORT V_GetIfldFlcd (doc, ifld, pflcd)
	int doc, ifld;
	struct FLCD *pflcd;

		{
		struct FLCD flcdNew;

		C_GetIfldFlcd (doc, ifld, &flcdNew);
		N_GetIfldFlcd (doc, ifld, pflcd);
		Assert (!FNeRgch(&flcdNew, pflcd, sizeof (struct FLCD)));
		}


/* %%Function:V_IfldInsertDocCp %%Owner:BRADV */
EXPORT V_IfldInsertDocCp (doc, cp)
	int doc;
	CP cp;

		{
		int ifldNew, ifldReturn;

		ifldNew = C_IfldInsertDocCp (doc, cp);
		ifldReturn = N_IfldInsertDocCp (doc, cp);
		Assert ( ifldNew == ifldReturn );
		return ifldReturn;
		}


/* %%Function:V_FetchCpPccpVisible %%Owner:BRADV */
EXPORT V_FetchCpPccpVisible (doc, cp, pccp, fvc, fNested)
	int doc;
	CP cp;
	int *pccp, fvc;
	BOOL fNested;
		{
		extern CP vcpFetch;
		extern int vdocFetch;
		CP cpFetchSav = vcpFetch;
		int ccpSav = *pccp;
		int docFetchSav = vdocFetch;
		CP cpFetchNew;
		int ccpNew;

		C_FetchCpPccpVisible (doc, cp, pccp, fvc, fNested);
		cpFetchNew = vcpFetch;
		ccpNew = *pccp;
		*pccp = ccpSav;
		if (doc == docNil)
			C_FetchCpPccpVisible (docFetchSav, cpFetchSav, &ccpSav, fvc, fNested);
		N_FetchCpPccpVisible (doc, cp, pccp, fvc, fNested);
		Assert ( *pccp == 0 || vcpFetch == cpFetchNew );
		Assert ( *pccp == ccpNew );
		}


/* %%Function:V_CpVisibleCpField %%Owner:BRADV */
EXPORT CP V_CpVisibleCpField (doc, cp, fvc, fIns)
	int doc;
	CP cp;
	int fvc;
	BOOL fIns;

		{
		CP cpNew, cpReturn;

		cpNew = C_CpVisibleCpField (doc, cp, fvc, fIns);
		cpReturn = N_CpVisibleCpField (doc, cp, fvc, fIns);
		Assert ( cpNew == cpReturn );
		return cpReturn;
		}


/* %%Function:V_FCpVisiInOutline %%Owner:BRADV */
EXPORT CP V_FCpVisiInOutline(ww, doc, cp, ccp, pcpFirstInvisi)
	int ww;
	int doc;
	CP cp;
	int ccp;
	CP *pcpFirstInvisi;
		{
		int fNew, fReturn;
		CP cpFirstInvisiSav, cpFirstInvisiNew;

		cpFirstInvisiSav = *pcpFirstInvisi;
		fNew = C_FCpVisiInOutline(ww, doc, cp, ccp, pcpFirstInvisi);
		cpFirstInvisiNew = *pcpFirstInvisi;
		*pcpFirstInvisi = cpFirstInvisiSav;
		fReturn = N_FCpVisiInOutline(ww, doc, cp, ccp, pcpFirstInvisi);
		if (fReturn)
			fReturn = fTrue;
		Assert ( fNew == fReturn );
		Assert ( cpFirstInvisiNew == *pcpFirstInvisi );
		return fReturn;
		}


/* %%Function:V_CpSearchSz %%Owner:BRADV */
EXPORT CP V_CpSearchSz (pbmib, cpFirst, cpLim, pcpNextRpt, hppr)
	struct BMIB *pbmib;
	CP      cpFirst;
	CP      cpLim;
	CP      *pcpNextRpt;
	struct PPR **hppr;

		{
		extern CP	   vcpLimWrap;
		extern CP	   vcpMatchLim;
		CP cpMatchLimSav, cpLimWrapSav;
		CP cpNew, cpReturn, cpMatchLimNew, cpLimWrapNew;

		cpMatchLimSav = vcpMatchLim;
		cpLimWrapSav = vcpLimWrap;
		cpNew = C_CpSearchSz (pbmib, cpFirst, cpLim, pcpNextRpt, hppr);
		cpMatchLimNew = vcpMatchLim;
		cpLimWrapNew = vcpLimWrap;
		vcpMatchLim = cpMatchLimSav;
		vcpLimWrap = cpLimWrapSav;
		cpReturn = N_CpSearchSz (pbmib, cpFirst, cpLim, pcpNextRpt, hppr);
		Assert ( cpNew == cpReturn );
		Assert ( cpReturn == cpNil || vcpMatchLim == cpMatchLimNew );
		Assert ( vcpLimWrap == cpLimWrapNew );
		return cpReturn;
		}


/* %%Function:V_CpSearchSzBackward %%Owner:BRADV */
EXPORT CP V_CpSearchSzBackward (pbmib, cpFirst, cpLim)
	struct BMIB *pbmib;
	CP    cpFirst;
	CP    cpLim;

		{
		extern CP	   vcpLimWrap;
		extern CP	   vcpMatchLim;
		CP cpMatchLimSav, cpLimWrapSav;
		CP cpNew, cpReturn, cpMatchLimNew, cpLimWrapNew;

		cpMatchLimSav = vcpMatchLim;
		cpLimWrapSav = vcpLimWrap;
		cpNew = C_CpSearchSzBackward (pbmib, cpFirst, cpLim);
		cpMatchLimNew = vcpMatchLim;
		cpLimWrapNew = vcpLimWrap;
		vcpMatchLim = cpMatchLimSav;
		vcpLimWrap = cpLimWrapSav;
		cpReturn = N_CpSearchSzBackward (pbmib, cpFirst, cpLim);
		Assert ( cpNew == cpReturn );
		Assert ( cpReturn == cpNil || vcpMatchLim == cpMatchLimNew );
		Assert ( vcpLimWrap == cpLimWrapNew );
		return cpReturn;
		}


/* %%Function:V_FInTableDocCp %%Owner:BRADV */
EXPORT BOOL V_FInTableDocCp (doc, cp)
	int doc;
	CP cp;

		{
		CP cpFirstTableParaSav;
		CP cpFirstTableParaNew;
		CP cpFirstTableCellSav;
		CP cpFirstTableCellNew;
		int fNew;
		int fReturn;
		struct CA caCellSav;
		struct CA caCellNew;
		extern CP vcpFirstTablePara;
		extern CP vcpFirstTableCell;

		caCellSav = vcaCell;
		cpFirstTableParaSav = vcpFirstTablePara;
		cpFirstTableCellSav = vcpFirstTableCell;
		fNew = C_FInTableDocCp (doc, cp);
		caCellNew = vcaCell;
		cpFirstTableParaNew = vcpFirstTablePara;
		cpFirstTableCellNew = vcpFirstTableCell;
		vcaCell = caCellSav;
		vcpFirstTablePara = cpFirstTableParaSav;
		vcpFirstTableCell = cpFirstTableCellSav;
		fReturn = N_FInTableDocCp (doc, cp);
		if (fReturn) fReturn = fTrue;
		Assert ( fNew == fReturn );
		Assert ( !FNeRgch(&caCellNew, &vcaCell, sizeof(struct CA)) );
		Assert ( vcpFirstTablePara == cpFirstTableParaNew );
		Assert ( vcpFirstTableCell == cpFirstTableCellNew );
		return fReturn;
		}


/* %%Function:V_XpFromDcp %%Owner:BRADV */
EXPORT int V_XpFromDcp (dcp1, dcp2, pxpFirst, pich)
	CP dcp1, dcp2;
	int *pxpFirst, *pich;
		{
		int xpFirstSav, xpFirstNew;
		int ichSav, ichNew;
		int xpNew, xpReturn;

		xpFirstSav = *pxpFirst;
		ichSav = *pich;
		xpNew = C_XpFromDcp (dcp1, dcp2, pxpFirst, pich);
		xpFirstNew = *pxpFirst;
		ichNew = *pich;
		*pxpFirst = xpFirstSav;
		*pich = ichSav;
		xpReturn = N_XpFromDcp (dcp1, dcp2, pxpFirst, pich);
		Assert ( xpNew == xpReturn );
		Assert ( *pxpFirst == xpFirstNew );
		Assert ( *pich == ichNew );
		return xpReturn;
		}


/* %%Function:V_IbpLru %%Owner:BRADV */
EXPORT int V_IbpLru ()
		{
		int ibpNew, ibpReturn;

		ibpNew = C_IbpLru ();
		ibpReturn = N_IbpLru ();
		Assert ( ibpNew == ibpReturn );
		return ibpReturn;
		}


/* %%Function:V_GetCpFirstCpLimDisplayPara %%Owner:BRADV */
EXPORT void V_GetCpFirstCpLimDisplayPara(ww, doc, cp, pcpFirst, pcpLim)
	int ww;
	int doc;
	CP cp;
	CP *pcpFirst, *pcpLim;

		{
		CP cpFirstSav = *pcpFirst;
		CP cpLimSav = *pcpLim;
		CP cpFirstNew;
		CP cpLimNew;

		C_GetCpFirstCpLimDisplayPara(ww, doc, cp, pcpFirst, pcpLim);
		cpFirstNew = *pcpFirst;
		cpLimNew = *pcpLim;
		*pcpFirst = cpFirstSav;
		*pcpLim = cpLimSav;
		N_GetCpFirstCpLimDisplayPara(ww, doc, cp, pcpFirst, pcpLim);
		Assert ( *pcpFirst == cpFirstNew );
		Assert ( *pcpLim == cpLimNew );
		}


/* %%Function:V_CpFormatFrom %%Owner:BRADV */
EXPORT CP V_CpFormatFrom(ww, doc, cp)
	int ww;
	int doc;
	CP cp;
		{
		CP cpNew;
		CP cpResult;

		cpNew = C_CpFormatFrom(ww, doc, cp);
		cpResult = N_CpFormatFrom(ww, doc, cp);
		Assert ( cpNew == cpResult );
		return cpResult;
		}


/* %%Function:V_CpVisibleBackCpField %%Owner:BRADV */
EXPORT CP V_CpVisibleBackCpField(doc, cp, fvc)
	int doc;
	CP cp;
	int fvc;
		{
		CP cpNew;
		CP cpResult;

		cpNew = C_CpVisibleBackCpField(doc, cp, fvc);
		cpResult = N_CpVisibleBackCpField(doc, cp, fvc);
		Assert ( cpNew == cpResult );
		return cpResult;
		}


#ifdef REVIEW /* REVIEW iftime: needs significant work to ensure the global
	variables it messes with are saved and restored between the calls to
			the N_ and C_ versions before it can be used (rp, quoting BradV) */

		/* M A P S T C   V E R I F Y */
/* called instead of MapStc when vdbs.CMapStc == 2. */
/* calls both C and native versions of MapStc and compares
	the results */

/* %%Function:V_MapStc %%Owner:bradv */
EXPORT V_MapStc(pdod, stc, pchp, ppap)
	struct DOD *pdod;
	int stc; 
	struct CHP *pchp; 
	struct PAP *ppap;
		{
		extern int              vstcpMapStc;

		int cDifference;

		char *pb1, *pb2;
		int ib;
		int vstcpMapStcT;
		int fError = fFalse;
		struct CHP       chpT;
		struct PAP       papT;

		C_MapStc(pdod, stc, pchp, ppap);

		vstcpMapStcT = vstcpMapStc;

		bltbyte(pchp, &chpT, sizeof(struct CHP));
		bltbyte(ppap, &papT, sizeof(struct PAP));

		N_MapStc(pdod, stc, pchp, ppap);

		if (vstcpMapStcT != vstcpMapStc)
			{
			fError = fTrue;
			CommSzNum(SzShared("\007MapStc - vstcpMapStc difference"),SzShared(""));
			CommSzNum(SzShared("CS     vstcpMapStc = "), vstcpMapStcT);
			CommSzNum(SzShared("Native vstcpMapStc = "), vstcpMapStc);
			}

		if (pchp)
			{
			pb1 = &chpT;
			pb2 = pchp;
			for ( ib = 0, cDifference = 0; ib < sizeof (struct CHP); ib++ )
				{
				if (*pb1++ != *pb2++)
					{
					fError = fTrue;
					CommSzNum(SzShared("\007MapStc - CHP difference at offset\007 = "),ib );
					CommSzNum(SzShared("CS byte = "),*(pb1-1) );
					CommSzNum(SzShared("Native byte = "),*(pb2-1) );
					if (cDifference++ >= 3)
						break;
					}
				}
			}

		pb1 = &papT;
		pb2 = ppap;
		for ( ib = 0, cDifference = 0; ib < sizeof (struct PAP); ib++ )
			{
			if (*pb1++ != *pb2++)
				{
				fError = fTrue;
				CommSzNum(SzShared("\007MapStc - PAP difference at offset\007 = "),ib );
				CommSzNum(SzShared("CS byte = "),*(pb1-1) );
				CommSzNum(SzShared("Native byte = "),*(pb2-1) );
				if (cDifference++ >= 3)
					break;
				}
			}

		vstcpMapStc = vstcpMapStcT;

		if (fError)
			{
			Assert(fFalse);
			C_MapStc(pdod, stc, pchp, ppap);
			N_MapStc(pdod, stc, pchp != 0 ? &chpT : 0, &papT);
			}

		vstcpMapStc = vstcpMapStcT;
		}
#endif

#define maxm(a,b)	((a) > (b) ? (a) : (b))

/* %%Function:V_ApplyPrlSgc %%Owner:BRADV */
EXPORT V_ApplyPrlSgc(hpprl, cch, prgbProps, sgc)
	char HUGE *hpprl;
	char *prgbProps;
	int cch, sgc;
		{
		int ib, cDifference = 0;
		int cb;
		char rgbOrig[maxm(cbPAP, maxm(cbCHP, cbSEP))];
		char rgbNatResult[maxm(cbPAP, maxm(cbCHP, cbSEP))];

		switch (sgc)
			{
		case sgcPap:	
			cb = cbPAP;  	
			break;
		case sgcChp:	
			cb = cbCHP;  	
			break;
		case sgcSep:	
			cb = cbSEP;  	
			break;
		default:	
			cb = 0;		
			break;
			}

		if (cb != 0)
			bltbyte( prgbProps, rgbOrig, cb );
		N_ApplyPrlSgc(hpprl, cch, prgbProps, sgc);

		if (cb != 0)
			{
			bltbyte(prgbProps, rgbNatResult, cb);
			bltbyte(rgbOrig, prgbProps, cb );
			C_ApplyPrlSgc(hpprl, cch, prgbProps, sgc);

			for ( ib = 0, cDifference = 0; ib < cb; ib++, prgbProps++)
				{
				if (*prgbProps != rgbNatResult[ib])
					{
					switch (sgc)
						{
					case sgcPap:
						CommSzNum(SzShared("\007ApplyPrlSgc - PAP difference at offset\007 = "),ib );
						break;

					case sgcChp:
						CommSzNum(SzShared("\007ApplyPrlSgc - CHP difference at offset\007 = "),ib );
						break;

					case sgcSep:
						CommSzNum(SzShared("\007ApplyPrlSgc - SEP difference at offset\007 = "),ib );
						break;
						}
					CommSzNum(SzShared("CS byte = "), *prgbProps);
					CommSzNum(SzShared("Native byte = "), rgbNatResult[ib]);
					if (cDifference++ >= 3)
						break;
					}
				}
			if (cDifference > 0)
				Assert(fFalse);
			}
		}


/* %%Function:V_GetParaState %%Owner:BRADV */
EXPORT BOOL V_FGetParaState(fAll, fAbortOk)
	BOOL fAll;
	BOOL fAbortOk;
		{
		char *pb1;
		char *pb2;
		int ib, cDifference;
		BOOL fRetValNat;
		BOOL fRetValC;
		struct PAP papSav;
		struct PAP papGraySav;
		BOOL fUpdatePapSav;
		BOOL fUpdateRulerSav;
		struct PAP papTest;
		struct PAP papGrayTest;
		BOOL fUpdatePapTest;
		BOOL fUpdateRulerTest;

		blt(&vpapSelCur, &papSav, cwPAP);
		blt(&vpapGraySelCur, &papGraySav, cwPAP);
		fUpdatePapSav = selCur.fUpdatePap;
		fUpdateRulerSav = selCur.fUpdateRuler;

		fRetValNat = N_FGetParaState(fAll, fAbortOk);

		if (fRetValNat)
			{
			blt(&vpapSelCur, &papTest, cwPAP);
			blt(&vpapGraySelCur, &papGrayTest, cwPAP);
			fUpdatePapTest = selCur.fUpdatePap;
			fUpdateRulerTest = selCur.fUpdateRuler;

			blt(&papSav, &vpapSelCur, cwPAP);
			blt(&papGraySav, &vpapGraySelCur, cwPAP);
			selCur.fUpdatePap = fUpdatePapSav;
			selCur.fUpdateRuler = fUpdateRulerSav;

			fRetValC = C_FGetParaState(fAll, fAbortOk);

			if (fRetValC)
				{
				for (pb1 = &vpapSelCur, pb2 = &papTest, ib = 0, cDifference = 0;
						(ib < sizeof (struct PAP)) && (cDifference < 3); ib++ )
					{
					if (*pb1++ != *pb2++)
						{
						CommSzNum(SzShared("\007FGetParaState - PAP difference at offset\007 = "),
								ib );
						CommSzNum(SzShared("CS byte = "),*(pb1-1) );
						CommSzNum(SzShared("Native byte = "),*(pb2-1) );
						cDifference++;
						}
					}
				for (pb1 = &vpapGraySelCur, pb2 = &papGrayTest, ib = 0;
						(ib < sizeof (struct PAP)) && (cDifference < 3); ib++ )
					{
					if (*pb1++ != *pb2++)
						{
						CommSzNum(SzShared("\007FGetParaState - PAPGRAY difference at offset\007 = "),
								ib );
						CommSzNum(SzShared("CS byte = "),*(pb1-1) );
						CommSzNum(SzShared("Native byte = "),*(pb2-1) );
						cDifference++;
						}
					}
				if (selCur.fUpdatePap != fUpdatePapTest)
					{
					CommSzSz(SzShared("\007FGetParaState - selCur.fUpdatePap difference"), szEmpty);
					CommSzNum(SzShared("CS selCur.fUpdatePap = "), selCur.fUpdatePap);
					CommSzNum(SzShared("Native selCur.fUpdatePap = "), fUpdatePapTest);
					cDifference++;
					}
				if (selCur.fUpdateRuler != fUpdateRulerTest)
					{
					CommSzSz(SzShared("\007FGetParaState - selCur.fUpdateRuler difference"), szEmpty);
					CommSzNum(SzShared("CS selCur.fUpdateRuler = "), selCur.fUpdateRuler);
					CommSzNum(SzShared("Native selCur.fUpdateRuler = "), fUpdateRulerTest);
					cDifference++;
					}
				if (cDifference > 0)
					Assert(fFalse);
				}
			return fRetValC;
			}
		else
			return fRetValNat;
		}


/* %%Function:V_FGraphicsFcidToPlf %%Owner:BRADV */
EXPORT V_FGraphicsFcidToPlf(fcid, plf, fPrinterFont)
	union FCID fcid;
	LOGFONT *plf;
	int fPrinterFont;
		{
		char *pb1;
		char *pb2;
		int fGraphicsN, fGraphicsC;
		int ib, cDifference;
		LOGFONT lfT;

		fGraphicsN = N_FGraphicsFcidToPlf(fcid, &lfT, fPrinterFont);
		fGraphicsC = C_FGraphicsFcidToPlf(fcid, plf, fPrinterFont);

		if (!fGraphicsN != !fGraphicsC)	/* only logical vals must match */
			{
			CommSzSz(SzShared("\007FGraphicsFcidToPlf - return value difference"), szEmpty);
			CommSzNum(SzShared("CS return value = "), fGraphicsC);
			CommSzNum(SzShared("Native return value = "), fGraphicsN);
			cDifference++;
			}

		for (pb1 = plf, pb2 = &lfT, ib = 0, cDifference = 0;
				(ib < sizeof(LOGFONT)) && (cDifference < 3); ib++ )
			{
			if (*pb1++ != *pb2++)
				{
				CommSzNum(SzShared("\007FGraphicsFcidToPlf - LOGFONT difference at offset\007 = "),ib );
				CommSzNum(SzShared("CS byte = "),*(pb1-1) );
				CommSzNum(SzShared("Native byte = "),*(pb2-1) );
				cDifference++;
				}
			}

		if (cDifference > 0)
			Assert(fFalse);
		}

#define lvlBody 9

/* %%Function:V_FFilRgwWithSeqLevs %%Owner:BRADV */
EXPORT V_FFillRgwWithSeqLevs(doc, cp, ipad, ifld, hplcpad, hplcfld, rgw)
	int doc;
	CP cp;
	int ipad;
	int ifld;
	struct PLC **hplcpad;
	struct PLC **hplcfld;
	int *rgw;
		{
		int fNew;
		int fResult;
		int rgwSav[lvlBody+1];
		int rgwNew[lvlBody+1];

		blt(rgw, rgwSav, lvlBody+1);
		fNew = C_FFillRgwWithSeqLevs(doc, cp, ipad, ifld, hplcpad, hplcfld, rgw);
		blt(rgw, rgwNew, lvlBody+1);
		blt(rgwSav, rgw, lvlBody+1);
		fResult = N_FFillRgwWithSeqLevs(doc, cp, ipad, ifld, hplcpad, hplcfld, rgw);
		if (fResult)
			fResult = fTrue;
		Assert ( fNew == fResult );
		Assert (!FNeRgw(rgwNew, rgw, lvlBody+1));
		return fResult;
		}


/* %%Function:V_WidthHeightFromBrc %%Owner:BRADV */
EXPORT V_WidthHeightFromBrc(brc, grpf)
	struct BRC brc;
	int grpf;
		{
		int wNew, wResult;

		wNew = C_WidthHeightFromBrc(brc, grpf);
		wResult = N_WidthHeightFromBrc(brc, grpf);
		Assert ( wNew == wResult );
		return wResult;
		}


/* %%Function:V_ItcGetTcxCache %%Owner:BRADV */
EXPORT V_ItcGetTcxCache(ww, doc, cp, ptap, itc, ptcx)
	int ww, doc, itc;
	CP cp;
	struct TAP *ptap;
	struct TCX *ptcx;
		{
		int itcNext, itcResult;
		struct TCX tcx;
		struct TCXS tcxs, tcxsSav;
		struct TAP tap, tapSav;

		tcxsSav = vtcxs;
		tapSav = *ptap;
		itcNext = C_ItcGetTcxCache(ww, doc, cp, ptap, itc, &tcx);

		tcxs = vtcxs;
		vtcxs = tcxsSav;
		tap = *ptap;
		*ptap = tapSav;
		itcResult = N_ItcGetTcxCache(ww, doc, cp, ptap, itc, ptcx);

		Assert ( itcNext == itcResult );
		if (ww == wwNil)
			Assert ( !FNeRgch(tcx.rgbrc, ptcx->rgbrc, cbrcTcx*sizeof(int)) );
		else
			Assert ( !FNeRgch(&tcx, ptcx, sizeof(struct TCX)) );
		Assert (!FNeRgch(ptap, &tap, sizeof(struct TAP)) );
		Assert (!FNeRgch(&tcxs, &vtcxs, sizeof(struct TCXS)) );
		return itcResult;
		}

/* %%Function:V_ItcGetTcx %%Owner:BRADV */
EXPORT V_ItcGetTcx(ww, ptap, itc, ptcx)
	int ww, itc;
	struct TAP *ptap;
	struct TCX *ptcx;
		{
		int itcNext, itcResult;
		struct TCX tcx;
		struct TAP tap, tapSav;

		tapSav = *ptap;
		itcNext = C_ItcGetTcx(ww, ptap, itc, &tcx);

		tap = *ptap;
		*ptap = tapSav;
		itcResult = N_ItcGetTcx(ww, ptap, itc, ptcx);

		Assert ( itcNext == itcResult );
		if (ww == wwNil)
			Assert ( !FNeRgch(tcx.rgbrc, ptcx->rgbrc, cbrcTcx*sizeof(int)) );
		else
			Assert ( !FNeRgch(&tcx, ptcx, sizeof(struct TCX)) );
		Assert (!FNeRgch(ptap, &tap, sizeof(struct TAP)) );
		return itcResult;
		}

/* %%Function:V_ChMapSpec %%Owner:BRADV */
EXPORT char V_ChMapSpecChar(ch, chs)
	char ch;
	int chs;
		{
		int wNew, wResult;

		wNew = C_ChMapSpecChar(ch, chs);
		wResult = N_ChMapSpecChar(ch, chs);
		Assert ( wNew == wResult );
		return wResult;
		}

/* %%Function:V_FSearchRgrsym %%Owner:BRADV */
EXPORT char *V_FSearchRgrsym(sz, pirsym)
	char *sz;
	int *pirsym;
		{
		int fNew, fResult;
		int wSav, wNew;
		char rgchResultNew[20];

		wSav = *pirsym;
		fNew = C_FSearchRgrsym(sz, pirsym);
		wNew = *pirsym;
		*pirsym = wSav;
		fResult = N_FSearchRgrsym(sz, pirsym);
		if (fResult)
			fResult = fTrue;
		Assert ( fNew == fResult );
		Assert (wNew == *pirsym);
		return fResult;
		}

/* %%Function:V_ValFromPropSprm %%Owner:BRADV */
EXPORT int V_ValFromPropSprm(prgbProps, sprm)
	char *prgbProps;
	int sprm;
		{
		int wNew, wResult;

		wNew = C_ValFromPropSprm(prgbProps, sprm);
		wResult = N_ValFromPropSprm(prgbProps, sprm);
		Assert ( wNew == wResult );
		return wResult;
		}

/* %%Function:V_PchSzRtfMove %%Owner:BRADV */
EXPORT int V_PchSzRtfMove(iszRtf, pch)
	int iszRtf;
	char *pch;
		{
		int pchNew, pchResult;
		char rgchSav[20], rgchNew[20];

		bltb(pch, rgchSav, 20);
		pchNew = C_PchSzRtfMove(iszRtf, pch);
		Assert (pchNew - pch < 20);
		bltb(pch, rgchNew, pchNew - pch);
		bltb(rgchSav, pch, pchNew - pch);
		pchResult = N_PchSzRtfMove(iszRtf, pch);
		Assert ( pchNew == pchResult );
		Assert (!FNeRgch(pch, rgchNew, pchNew - pch));
		return pchResult;
		}

/* %%Function:V_WCompSzSrt %%Owner:BRADV */
EXPORT int V_WCompSzSrt(psz1,psz2,fCase)
	char *psz1;
	char *psz2;
	int fCase;
		{
		int wNew, wResult;

		wNew = C_WCompSzSrt(psz1,psz2,fCase);
		wResult = N_WCompSzSrt(psz1,psz2,fCase);
		Assert ( wNew == wResult );
		return wResult;
		}

/* %%Function:V_WCompChCh %%Owner:BRADV */
EXPORT int V_WCompChCh(ch1, ch2)
	CHAR ch1, ch2;
		{
		int wNew, wResult;

		wNew = C_WCompChCh(ch1, ch2);
		wResult = N_WCompChCh(ch1, ch2);
		Assert ( wNew == wResult );
		return wResult;
		}

/* %%Function:V_WCompRgchIndex %%Owner:BRADV */
EXPORT int V_WCompRgchIndex(hpch1, cch1, hpch2, cch2)
	char	HUGE *hpch1, HUGE *hpch2;
	int	cch1, cch2;
		{
		int wNew, wResult;

		wNew = C_WCompRgchIndex(hpch1, cch1, hpch2, cch2);
		wResult = N_WCompRgchIndex(hpch1, cch1, hpch2, cch2);
		Assert ( wNew == wResult );
		return wResult;
		}

/* %%Function:V_IbstFindSzFfn(hsttb, pffn) %%Owner:BRADV */
EXPORT int V_IbstFindSzFfn(hsttb, pffn)
	struct STTB **hsttb;
	struct FFN *pffn;
		{
		int wNew, wResult;

		wNew = C_IbstFindSzFfn(hsttb, pffn);
		wResult = N_IbstFindSzFfn(hsttb, pffn);
		Assert ( wNew == wResult );
		return wResult;
		}


/* T R A C K   G D I   H A N D L E S */
/*    Here we attempt to keep track of the GDI handles because when we quit,
	these are the only ones that are not automatically flushed by windows.
	To do this whenever we get a GDI handle we should call LogGdiHandle
	with a handle and a wId unique to that handle.  We should later call
	UnlogGdiHandle with that same handle and a wId either equal to the
	wId that was passed in, or a wId equal to negative 1.  Before every
	call to DeleteObject there should be a call to UnlogGdiHandle.  Just
	before quitting QuitTrackGdiHandles is called and it checks to make
	sure that every allocated GDI handle has been deleted, and if not
	uses CommSz to report the error.
		The reasoning behind these wId rules is we want to perform two
	functions.  One, we want to have a unique ID so that when an error
	occurs we know who did the allocation.  Two, we want to be able to
	deal with non-unique handles that can arise if we select the same
	system object into two different handles.  If there are two equal
	handles we must pass the proper wId to UnlogGdiHandle, if not we
	can pass -1.  Why allow passing a wId of -1 to UnlogGdiHandle at all?
	because some routines perform DeleteObject without knowing who exactly
	created the handle, and so cannot pass the proper wId.  Hopefully
	there will be no cases where we don't know who created a non-unique
	handle, at that point this algorithm breaks down.

		Here is a list of the wId's as of 4-20-89 (Brad Verheiden)
	1002 for hbmMono in HbmMonoForHbmColor in clipbrd2.c
	1004 for hrgnT in FedtComputeRgnSel in dialog1.c
	1005 for hrgnT in FedtInvertRgn in dialog1.c
	1008 for hbmpT in FCreateSysMenu in init2.c
	1010 for hbm in FDrawMetaFile in pic.c
	1011 for vpri.hbrText in EndPrintJob in print.c
	1012 for hpen in DrawPrvwLine in prvw2.c
	1013 for vhfntStyWnd in quit.c, dispspec.c
	1014 for vsci.hbrBkgrnd in quit.c, syschg.c
	1015 for vsci.hbrText in quit.c, syschg.c
	1016 for vsci.hbrDesktop in quit.c, syschg.c
	1017 for vsci.hbrBorder in quit.c, syschg.c
	1018 for vsci.hbrScrollbar in quit.c, syschg.c
	1019 for vsci.hpen in quit.c, syschg.c
	1020 for vsci.hpenBorder in quit.c, syschg.c
	1021 for vhbmPatVert in quit.c, initwin.c
	1022 for vhbmPatHorz in quit.c, initwin.c
	1024 for vhrgnPage in quit.c, pagevw.c
	1025 for vhrgnGrey in quit.c, pagevw.c
	1027 for hbmDest in FLoadResourceIdrb in screen2.c
	1028 for hDataDescriptor in HWritePict in clipbrd2.c
	1030 for (*hpc)->hbm in FSendBitmap in grspec.c
	1031 for hbrNew in DrawPicFrame in pic.c
	1037 for hbmp in HFromIbmds0 in rcinit.c

Additions on 8-7-89:
	1038 for hMDCSrc in HbmMonoFromHbmColor in clipbrd2.c
	1039 for hMDCDst in HbmMonoFromHbmColor in clipbrd2.c
	1045 for (*hpc)->hMDC in FDrawBitmap in grspec.c
	1046 for hdc in HReadPgribSt in grspec.c
	1071 for hdc in FCreateSysMenu in init2.c
	1072 for hdc2 in FCreateSysMenu in init2.c
	1073 for vsci.mdcdScratch in FInitScreenConstants in initwin.c, quit.c
	1074 for vsci.mdcdBmp in FInitScreenConstants in initwin.c, quit.c
	1075 for hMDC in FDrawMetafile in pic.c
	1078 for hpen in DrawFormulaLine in print1.c
	1080 for pbms->hbm in FInitRSB in rsb.c	(ibms = 0)
	1081 for pbms->hbm in FInitRSB in rsb.c	(ibms = 1)
	1082 for pbms->hbm in FInitRSB in rsb.c	(ibms = 2)
	1083 for pbms->hbm in FInitRSB in rsb.c	(ibms = 3)
	1084 for pbmc->hbm in FInitRSB in rsb.c	(ibms = 4)
	1085 for vpri.hdc in AllocHdcPrinter in screen2.c
	1086 for hDCMeta in in CmdInsPicture in pic2.c
	1087 for hMeta in CmdInsPicture in pic2.c
	10000+ for pfce->hfont in LoadFcid in loadfont.c, formatn2.asm,
	(the font handles all have wId's >= 10000, and each is unique)
	15000+ for pfedt->hgrnSel in FedtInit in dialog1.c
	(these handles all have wId's >= 15000, and each is unique)
	20000+ for hbmp in HFromIbmds1 in rcbmp1.c,  
			for hbmp in HFromIbmds2 in rcbmp2.c,
			for hbmp in HFromIbmds3 in rcbmp3.c, and
			for hbmp in HFromIbmds4 in rcbmp4.c
	(these handles all have wId's >= 20000, and each is unique)
	25000+ for hrgn in FedtUpdateSel in dialog1.c
	(these handles all have wId's >= 25000, and each is unique)
*/

#define cGdiHandlesMax 200
	struct GDIS		/* GDI State */
		{
		int wMiscUniqueCount;
		int wCount;
		int rghGdi[cGdiHandlesMax];
		int rgwCount[cGdiHandlesMax];
		int rgwId[cGdiHandlesMax];
		};

/* %%Function:InitTrackGdiHandles %%Owner:BRADV */
	InitTrackGdiHandles()
		{
		struct GDIS *pgdis;

		if (!vdbs.fTrackGdiHandles)
			return;
		if ((vhgdis = HAllocateCb(sizeof(struct GDIS))) == hNil)
			return;
		Assert (hNil == 0);
		pgdis = *((struct GDIS **)vhgdis);
		SetBytes((char far *)pgdis, hNil, sizeof(struct GDIS));
		}

/* %%Function:LogGdiHandleProc %%Owner:BRADV */
	EXPORT LogGdiHandleProc(hGdi,wId)
			int hGdi, wId;
		{
		int iFree;
		int ihGdi;
		struct GDIS *pgdis;

		Assert(hGdi != hNil);
		Assert(wId != 0);
		Assert(vhgdis);
		pgdis = *((struct GDIS **)vhgdis);
		iFree = IScanLprgw((char far *)pgdis->rghGdi,
				hNil, cGdiHandlesMax);
		Assert(iFree != iNil);
		if (iFree == iNil)
			return;
		if (wId >= 10000)		/* Font handle */
			wId += pgdis->wMiscUniqueCount++;
		if (IScanLprgw((char far *)pgdis->rgwId, wId, cGdiHandlesMax) != -1)
				{
				CommSzNum(SzShared("ihGdi = "), ihGdi);
				CommSzNum(SzShared("wId = "), wId);
				}
		Assert(IScanLprgw((char far *)pgdis->rgwId, wId, cGdiHandlesMax) == -1);
		pgdis->rghGdi[iFree] = hGdi;
		pgdis->rgwCount[iFree] = ++(pgdis->wCount);
		pgdis->rgwId[iFree] = wId;
		}

/* %%Function:UnlogGdiHandleProc %%Owner:BRADV */
	UnlogGdiHandleProc(hGdi, wId)
			int hGdi, wId;
		{
		int ihGdi;
		struct GDIS *pgdis;

		Assert(hGdi != hNil);
		Assert(vhgdis);
		pgdis = *((struct GDIS **)vhgdis);
		if (wId != -1)
			{
			ihGdi = IScanLprgw((char far *)pgdis->rgwId,
					wId, cGdiHandlesMax);
			if (ihGdi < 0 || ihGdi >= cGdiHandlesMax)
				{
				CommSzNum(SzShared("ihGdi = "), ihGdi);
				CommSzNum(SzShared("wId = "), wId);
				}
			Assert (ihGdi >= 0 && ihGdi < cGdiHandlesMax);
			Assert (ihGdi == -1
					|| IScanLprgw((char far *)(&pgdis->rgwId[ihGdi + 1]),
					wId, cGdiHandlesMax - ihGdi - 1) == -1);
			Assert (pgdis->rghGdi[ihGdi] == hGdi);
			}
		else
			{
			ihGdi = IScanLprgw((char far *)pgdis->rghGdi,
					hGdi, cGdiHandlesMax);
			if (ihGdi < 0 || ihGdi >= cGdiHandlesMax)
				{
				CommSzNum(SzShared("ihGdi = "), ihGdi);
				CommSzNum(SzShared("wId = "), wId);
				}
			Assert (ihGdi >= 0 && ihGdi < cGdiHandlesMax);
			Assert (ihGdi == -1
					|| IScanLprgw((char far *)(&pgdis->rghGdi[ihGdi + 1]),
					hGdi, cGdiHandlesMax - ihGdi - 1) == -1);
			}
		if (ihGdi == -1)
			return;
		pgdis->rghGdi[ihGdi] = 0;
		pgdis->rgwCount[ihGdi] = 0;
		pgdis->rgwId[ihGdi] = 0;
		}

/* %%Function:QuitTrackGdiHandles %%Owner:BRADV */
	QuitTrackGdiHandles()
				{
		int ihGdi;
		struct GDIS *pgdis;

		if (!vhgdis)
			return;
		pgdis = *((struct GDIS **)vhgdis);
		for (ihGdi = 0; ihGdi < cGdiHandlesMax; ihGdi++)
			{
			if (pgdis->rghGdi[ihGdi] == hNil)
				continue;
			CommSzNum(SzShared("GDI handle = "), pgdis->rghGdi[ihGdi]);
			CommSzNum(SzShared("Count = "), pgdis->rgwCount[ihGdi]);
			CommSzNum(SzShared("ID = "), pgdis->rgwId[ihGdi]);
			}
		}


/* P D O P D O C */
/* %%Function:PdopDoc %%Owner:bradv */
	struct DOP *PdopDoc(doc)
		int doc;
		{
		Assert (PdodDoc(doc)->fMother);  /* should not be a subdoc  */
		return (&(PdodDoc(doc)->dop));
		}

#endif  /* DEBUG */
