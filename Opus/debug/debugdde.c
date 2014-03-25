/* D E B U G D D E . C */
/*  Debug routines for DDE */

#ifdef DEBUG

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "dde.h"
#include "doc.h"
#include "wininfo.h"
#include "props.h"
#include "sel.h"
#include "message.h"
#include "debug.h"


#ifdef PROTOTYPE
#include "debugdde.cpt"
#endif /* PROTOTYPE */

extern struct SEL      selCur;
extern int             dclMac;
extern int             docDde;
extern BOOL            vfDdeIdle;
extern struct DDES     vddes;
extern HWND            vhwndApp;
extern HANDLE          vhInstance;
extern BOOL            vfInsertMode;
extern BOOL            vfAbortInsert;
extern struct DCLD   **mpdclhdcld[];
extern struct MERR     vmerr;
extern CHAR            szProduct[];







/*  same order as the WM_DDE_ messages! */
csconst CHAR mpdwmszMessage [9][5] =
{ 
	"INIT", "TERM", "ADVI", "UNAD", "ACK",
			"DATA", "REQU", "POKE", "EXEC" };


/*  %%Function:ReportDdeErrorProc %%Owner:peterj  */
/* R E P O R T  D D E  E R R O R  P R O C */
/*  Called to report DDE errors.
	User presses YES for stack trace dump, else NO.  Execution
	continues (we should be OK) except on ABORT when we exit.
*/

ReportDdeErrorProc (sz, dcl, message, n)
CHAR *sz;
int dcl;
unsigned message;
int n;

{
	int rgw [4];
	CHAR szApp [50];

	if (dcl != dclNil)
		if (PdcldDcl (dcl)->dt == dtServer)
			CchCopySz (SzShared("(Client)"), szApp);
		else  if (PdcldDcl (dcl)->dt == dtClient)
			{
			StFromAtom (szApp, 50, PdcldDcl (dcl)->atomApp);
			StToSzInPlace (szApp);
			}
		else
			CchCopySz (SzShared("(Macro)"), szApp);
	else
		CchCopySz (SzShared ("(NULL)"), szApp);

	rgw [0] = sz;
	rgw [1] = szApp;
	rgw [2] = message ? SzNear (mpdwmszMessage[message-WM_DDE_FIRST])
			: SzFrame ("(none)");
	rgw [3] = n;

	switch (IdMessageBoxMstRgwMb ( mstDbgDdeError,
			rgw, (MB_ICONHAND | MB_ABORTRETRYIGNORE | MB_DEFBUTTON3)))
		{
	case IDRETRY:
		DoStackTrace(sz);
		break;
	default:
	case IDIGNORE:
		break;
	case IDABORT:
		OurExitWindows();   /* does not return */
		}
}




/* DDEREPORTS PROCs */

#define cchField1 38
#define cchField2 10
#define cchField3 15
#define cchField4 14
#define cchField34 (cchField3+cchField4)
#define cchAtom cchField34
#define cchLineMax (cchField1+cchField2+cchField3+cchField4)
#define cchSzOutMax (cchLineMax+3)


/*  %%Function:DumpDde %%Owner:peterj  */
/* D U M P  D D E */
/*  Dump a listing of all dde channels and links to the comm port.
*/

DumpDde ()

{
	int dcl;
	int iddli, iddliMac;

	/* global info */
	CommSz (SzShared("DDE State:\n\r"));
	DumpDdes ();

	/* channels */
	if (dclMac > 1)
		{
		CommSz (SzShared("Channels:\n\r"));
		for (dcl = 1; dcl < dclMac; dcl++)
			if (mpdclhdcld [dcl] != hNil)
				DumpDcl (dcl);
		}

	/* Server links */
	if (vddes.hplddli != hNil && (iddliMac = (*vddes.hplddli)->iMac) > 0)
		{
		CommSz (SzShared("Server Links:\n\r"));
		for (iddli = 0; iddli < iddliMac; iddli++)
			DumpIddliServer (iddli);
		}

	/* Client links */
	if (docDde != docNil && 
			(iddliMac = (*PdodDoc(docDde)->hplcddli)->iMac-1) > 0)
		{
		CommSz (SzShared("Client Links:\n\r"));
		for (iddli = 0; iddli < iddliMac; iddli++)
			DumpIddliClient (iddli);
		}
}


/*  %%Function:DumpDdes %%Owner:peterj  */
/* D U M P  D D E S */
DumpDdes ()
{
	CHAR *pch;
	CHAR szOut [cchSzOutMax];

	pch = szOut;
	PutSzInCommSz (SzShared("secTimeOut="), 0, &pch);
	PutIntInCommSz (vddes.secTimeOut, 2, 8, &pch);
	PutSzInCommSz (SzShared("que.c="), 0, &pch);
	if (vddes.hque == hNil)
		PutSzInCommSz (SzShared("(no que)"), 0, &pch);
	else
		PutIntInCommSz ((*vddes.hque)->c, 2, 6, &pch);
	PutSzInCommSz (SzShared("sdResult="), 0, &pch);
	if (vddes.sdResult == NULL)
		PutSzInCommSz (SzShared("NULL"), 0, &pch);
	else
		PutIntInCommSz (vddes.sdResult, 4, 6, &pch);
	EndCommSz (&pch, szOut);
	CommSz (szOut);

	PutSzInCommSz (SzShared("cDclServer="), 18, &pch);
	PutIntInCommSz (vddes.cDclServer, 2, 8, &pch);
	PutSzInCommSz (SzShared("fInvalidDdli="), 18, &pch);
	PutBoolInCommSz (vddes.fInvalidDdli, 6, &pch);
	PutSzInCommSz (SzShared("fDirtyLinks="), 18, &pch);
	PutBoolInCommSz (vddes.fDirtyLinks, 1, &pch);
	EndCommSz (&pch, szOut);
	CommSz (szOut);

	PutSzInCommSz (SzShared("cDclClient="), 18, &pch);
	PutIntInCommSz (vddes.cDclClient, 2, 8, &pch);
	PutSzInCommSz (SzShared("fHotLinksDirty="), 18, &pch);
	PutBoolInCommSz (vddes.fHotLinksDirty, 6, &pch);
	PutSzInCommSz (SzShared("fInInitiate="), 18, &pch);
	PutBoolInCommSz (vddes.fInInitiate, 1, &pch);
	EndCommSz (&pch, szOut);
	CommSz (szOut);

	PutSzInCommSz (SzShared("cDclMacro="), 18, &pch);
	PutIntInCommSz (vddes.cDclMacro, 2, 8, &pch);
	EndCommSz (&pch, szOut);
	CommSz (szOut);
}


/*  %%Function:DumpDcl %%Owner:peterj  */
/* D U M P  D C L */
DumpDcl (dcl)
int dcl;

{
	struct DCLD *pdcld = PdcldDcl (dcl);
	CHAR *pch;
	CHAR szOut [cchSzOutMax];
	CHAR stFile [ichMaxFile];
	CHAR szFile [ichMaxFile];

	pch = szOut;
	PutIntInCommSz (dcl, 2, 4, &pch);
	switch (pdcld->dt)
		{
	case dtServer:
		PutSzInCommSz (SzShared("(srvr)"), 8, &pch);
		break;
	case dtClient:
		PutSzInCommSz (SzShared("(clnt)"), 8, &pch);
		break;
	case dtMacro:
		PutSzInCommSz (SzShared("(mcro)"), 8, &pch);
		break;
	default:
		PutSzInCommSz (SzShared("(????)"), 8, &pch);
		break;
		}
	PutSzInCommSz (SzShared("hwndUs="), 0, &pch);
	PutIntInCommSz (pdcld->hwndUs, 4, 6, &pch);
	PutSzInCommSz (SzShared("hwndThem="), 0, &pch);
	PutIntInCommSz (pdcld->hwndThem, 4, 6, &pch);
	EndCommSz (&pch, szOut);
	CommSz (szOut);

	PutSzInCommSz (SzShared(""), 12, &pch);
	if (pdcld->fTerminating)
		PutSzInCommSz (SzShared("fTerm"), 0, &pch);
	if (pdcld->fBusyPrev)
		PutSzInCommSz (SzShared("fBsyPrv"), 0, &pch);
	if (pdcld->fBusyCur)
		PutSzInCommSz (SzShared("fBsyCur"), 0, &pch);
	if (pdcld->fResponse)
		PutSzInCommSz (SzShared("fResp"), 0, &pch);
	if (pdcld->fAck)
		PutSzInCommSz (SzShared("fAck"), 0, &pch);
	if (pdcld->fBusy)
		PutSzInCommSz (SzShared("fBusy"), 0, &pch);
	if (pdcld->fTermReceived)
		PutSzInCommSz (SzShared("fTmRcv"), 0, &pch);
	if (pdcld->fTermRequest)
		PutSzInCommSz (SzShared("ftmRqs"), 0, &pch);
	if (pch != szOut+12)
		{
		EndCommSz (&pch, szOut);
		CommSz (szOut);
		PutSzInCommSz (SzShared(""), 12, &pch);
		}

	if (pdcld->dt == dtServer)
		{
		PutSzInCommSz (SzShared("doc="), 0, &pch);
		PutIntInCommSz (pdcld->doc, 2, 10, &pch);
		if (pdcld->doc == docSystem)
			PutSzInCommSz (SzShared("(system)"), 15, &pch);
		else
			{
			GetDocSt (pdcld->doc, stFile, gdsoShortName);
			StToSz(stFile, szFile);
			PutSzInCommSz (szFile, 15, &pch);
			}
		}
	else  if (pdcld->dt == dtClient)
		{
		PutSzInCommSz (SzShared("App="), 0, &pch);
		PutAtomInCommSz (pdcld->atomApp, 20, &pch);
		PutSzInCommSz (SzShared("Topic="), 0, &pch);
		PutAtomInCommSz (pdcld->atomTopic, 20, &pch);
		}
	else
		return;
	EndCommSz (&pch, szOut);
	CommSz (szOut);
}


/*  %%Function:DumpIddliServer %%Owner:peterj  */
/* D U M P  I D D L I  S E R V E R */
DumpIddliServer (iddli)
int iddli;
{
	CHAR *pch;
	CHAR szOut [cchSzOutMax];
	struct DDLI ddli;

	GetPl( vddes.hplddli, iddli, &ddli );
	pch = szOut;
	PutIntInCommSz (iddli, 2, 4, &pch);
	PutIntInCommSz (ddli.dcl, 2, 6, &pch);
	PutSzInCommSz (SzShared("dls="), 6, &pch);
	PutIntInCommSz (ddli.dls, 1, 4, &pch);
	PutSzInCommSz (SzShared("fAckReq="), 10, &pch);
	PutBoolInCommSz (ddli.fAckReq, 4, &pch);
	PutSzInCommSz (SzShared("fNoData="), 10, &pch);
	PutBoolInCommSz (ddli.fNoData, 4, &pch);
	PutSzInCommSz (SzShared("fDirty="), 10, &pch);
	PutBoolInCommSz (ddli.fDirty, 4, &pch);
	PutSzInCommSz (SzShared("ibkf="), 0, &pch);
	if (ddli.ibkf == ibkmkNil)
		PutSzInCommSz (SzShared("(nil)"), 7, &pch);
	else
		PutIntInCommSz (ddli.ibkf, 2, 7, &pch);
	EndCommSz (&pch, szOut);
	CommSz (szOut);
	PutSzInCommSz (SzShared(""), 10, &pch);
	PutSzInCommSz (SzShared("cf="), 6, &pch);
	PutIntInCommSz (ddli.cf, 2, 6, &pch);
	*pch++ = '(';
	PutIntInCommSz (ddli.atomItem, 4, 6, &pch);
	pch-=2;
	*pch++ = ')';
	PutAtomInCommSz (ddli.atomItem, 20, &pch);
	PutSzInCommSz (SzShared("hData="), 0, &pch);
	PutIntInCommSz (ddli.hData, 1, 5, &pch);
	EndCommSz (&pch, szOut);
	CommSz (szOut);
}


/*  %%Function:DumpIddliClient %%Owner:peterj  */
/* D U M P  I D D L I  C L I E N T */
DumpIddliClient (iddli)
int iddli;
{
	CHAR *pch;
	CHAR szOut [cchSzOutMax];
	struct DDLI ddli;

	GetPlc( PdodDoc( docDde )->hplcddli, iddli, &ddli );
	pch = szOut;
	PutIntInCommSz (iddli, 2, 4, &pch);
	PutIntInCommSz (ddli.dcl, 2, 6, &pch);
	PutSzInCommSz (SzShared("dls="), 6, &pch);
	PutIntInCommSz (ddli.dls, 1, 4, &pch);
	PutSzInCommSz (SzShared("fHot= "), 10, &pch);
	PutBoolInCommSz (ddli.fHot, 4, &pch);
	PutSzInCommSz (SzShared("fHotDirty="), 10, &pch);
	PutBoolInCommSz (ddli.fHotDirty, 4, &pch);
	EndCommSz (&pch, szOut);
	CommSz (szOut);
	PutSzInCommSz (SzShared(""), 10, &pch);
	PutSzInCommSz (SzShared("cf="), 6, &pch);
	PutIntInCommSz (ddli.cf, 2, 6, &pch);
	*pch++ = '(';
	PutIntInCommSz (ddli.atomItem, 4, 6, &pch);
	pch-=2;
	*pch++ = ')';
	PutAtomInCommSz (ddli.atomItem, 20, &pch);
	PutSzInCommSz (SzShared("hData="), 0, &pch);
	PutIntInCommSz (ddli.hData, 1, 5, &pch);
	EndCommSz (&pch, szOut);
	CommSz (szOut);
}


/*  %%Function:DdeDbgCommAtomProc %%Owner:peterj  */
/* D D E  D B G  C O M M  A T O M  P R O C */
DdeDbgCommAtomProc (sz, dcl, atom)
CHAR *sz;
int dcl;
ATOM atom;

{
	CHAR *pch;
	CHAR szOut [cchSzOutMax];

	if (!vdbs.fCommDde1)
		return;

	pch = szOut;
	PutSzInCommSz (sz, cchField1, &pch);
	PutIntInCommSz (dcl, 2, cchField2, &pch);
	PutAtomInCommSz (atom, cchField34, &pch);
	EndCommSz (&pch, szOut);

	CommSz (szOut);
}


/*  %%Function:DdeDbgCommSzProc %%Owner:peterj  */
/* D D E  D B G  C O M M  S Z  P R O C */
DdeDbgCommSzProc(sz)
CHAR *sz;
{
	CHAR *pch;
	CHAR szOut [cchSzOutMax];

	if (!vdbs.fCommDde1)
		return;

	pch = szOut;
	Assert (CchSz (sz) < cchLineMax);
	PutSzInCommSz (sz, 0, &pch);
	EndCommSz (&pch, szOut);

	CommSz (szOut);
}


/*  %%Function:DdeDbgCommMsgProc %%Owner:peterj  */
/* D D E  D B G  C O M M  M S G  P R O C */
DdeDbgCommMsgProc(sz, dcl, message, wLow, wHigh)
CHAR *sz;
int dcl, message, wLow, wHigh;

{
	CHAR *pch;
	CHAR szOut [cchSzOutMax];

	if (!vdbs.fCommDde1)
		return;

	pch = szOut;

	PutSzInCommSz (SzShared("Msg:"), 6, &pch);
	PutSzInCommSz (sz, cchField1-6, &pch);
	PutMessageInCommSz (dcl, message, wLow, wHigh, &pch);
	EndCommSz (&pch, szOut);

	CommSz (szOut);
}


/*  %%Function:DdeDbgCommPostProc %%Owner:peterj  */
/* D D E  D B G  C O M M  P O S T  P R O C */
DdeDbgCommPostProc (dcl, message, wLow, wHigh)
int dcl, message, wLow, wHigh;
{


	CHAR *pch;
	CHAR szOut [cchSzOutMax];

	if (!vdbs.fCommDde1)
		return;

	pch = szOut;

	PutSzInCommSz (SzShared("Posting message: "), cchField1, &pch);
	PutMessageInCommSz (dcl, message, wLow, wHigh, &pch);
	EndCommSz (&pch, szOut);

	CommSz (szOut);
}


/*  %%Function:DdeDbgCommIntProc %%Owner:peterj  */
/* D D E  D B G  C O M M  I N T  P R O C */
DdeDbgCommIntProc (sz, w)
CHAR *sz;
int w;

{
	CHAR *pch;
	CHAR szOut [cchSzOutMax];

	if (!vdbs.fCommDde1)
		return;

	pch = szOut;
	PutSzInCommSz (sz, cchField1, &pch);
	PutIntInCommSz (w, 4, cchField2, &pch);
	EndCommSz (&pch, szOut);

	CommSz (szOut);
}


/*  %%Function:DdeDbgCommLongProc %%Owner:peterj  */
/* D D E  D B G  C O M M  L O N G  P R O C */
DdeDbgCommLongProc (sz, l)
CHAR *sz;
LONG l;

{
	CHAR *pch;
	CHAR szOut [cchSzOutMax];

	if (!vdbs.fCommDde1)
		return;

	pch = szOut;
	PutSzInCommSz (sz, cchField1, &pch);
	PutIntInCommSz (HIWORD(l), 4, 6, &pch);
	PutIntInCommSz (LOWORD(l), 4, 6, &pch);
	EndCommSz (&pch, szOut);

	CommSz (szOut);
}


/*  %%Function:EndCommSz %%Owner:peterj  */
/* E N D  C O M M  S Z */
EndCommSz (ppch, pch)
CHAR **ppch, *pch;
{
	CchCopySz (SzShared ("\n\r"), *ppch);
	Assert ((*ppch+3) - pch <= cchSzOutMax);
	*ppch = pch;
}


/*  %%Function:PutStInCommSz %%Owner:peterj  */
/* P U T  S T  I N  C O M M  S Z */
PutStInCommSz (st, cch, ppch)
CHAR *st, **ppch;
int cch;
{
	CHAR sz [cchMaxSz];
	Assert (*st < cchMaxSz);
	StToSz (st, sz);
	PutSzInCommSz (sz, cch, ppch);
}


/*  %%Function:PutSzInCommSz %%Owner:peterj  */
/* P U T  S Z  I N  C O M M  S Z  */
PutSzInCommSz (sz, cch, ppch)
CHAR *sz, **ppch;
int cch;
{
	cch -= 2;
	while (*sz && cch--)
		*(*ppch)++ = *sz++;

	while (cch-- > 0)
		*(*ppch)++ = ' ';

	*(*ppch)++ = ' ';
	*(*ppch)++ = ' ';

}


/*  %%Function:PutBoolInCommSz %%Owner:peterj  */
PutBoolInCommSz (f, cch, ppch)
BOOL f;
int cch;
CHAR **ppch;

{
	if (f)
		PutSzInCommSz (SzShared("T"), cch, ppch);
	else
		PutSzInCommSz (SzShared("F"), cch, ppch);
}


/*  %%Function:PutIntInCommSz %%Owner:peterj  */
PutIntInCommSz (w, cDigit, cch, ppch)
int w, cch;
CHAR **ppch;

{

	cch -= CchIntToAsciiHex (w, ppch, min(cDigit, cch-2));
	while (cch--)
		*(*ppch)++ = ' ';
}


/*  %%Function:PutAtomInCommSz %%Owner:peterj  */
PutAtomInCommSz (atom, cch, ppch)
ATOM atom;
int cch;
CHAR **ppch;
{
	CHAR rgch [cchAtom];

	StFromAtom (rgch, cchAtom, atom);
	StToSzInPlace (rgch);
	PutSzInCommSz (rgch, cch, ppch);
}


/*  %%Function:PutMessageInCommSz %%Owner:peterj  */
PutMessageInCommSz (dcl, message, wLow, wHigh, ppch)
int dcl, message, wLow, wHigh;
CHAR **ppch;

{

	PutIntInCommSz (dcl, 2, 4, ppch);

	if (message >= WM_DDE_FIRST && message-WM_DDE_FIRST < 9)
		PutSzInCommSz (SzNear (mpdwmszMessage[message-WM_DDE_FIRST]),
				cchField2-4, ppch);
	else
		PutIntInCommSz (message, 4, cchField2-4, ppch);

	switch (message)
		{
	case WM_DDE_INITIATE:
		PutAtomInCommSz (wLow, cchField3, ppch);
		PutAtomInCommSz (wHigh, cchField4, ppch);
		break;

	case WM_DDE_ACK:
	case WM_DDE_EXECUTE:
	case WM_DDE_TERMINATE:
		PutIntInCommSz (wLow, 4, cchField3, ppch);
		PutIntInCommSz (wHigh, 4, cchField4, ppch);
		break;

	default:
		PutIntInCommSz (wLow, 4, cchField3, ppch);
		PutAtomInCommSz (wHigh, cchField4, ppch);
		break;
		}
}



#ifdef ENABLE
/* QUEUE TEST ROUTINES */

struct QUE **vhqueTest = hNil;
int cEnqueue = 0;
#define cbTest cbDQM
#define iIncrTest idqmMaxInit

/*  %%Function:TestEnqueue %%Owner:peterj  */
TestEnqueue ()
{
	int i;
	CHAR rgch [cbTest];

	if (vhqueTest == hNil)
		vhqueTest = HInitQue (cbTest, iIncrTest);

	CommSzNum (SzShared ("Enqueueing: "), ++cEnqueue);
	for (i = sizeof (int); i < cbTest; i++)
		rgch [i] = i;

	bltb (&cEnqueue, rgch, sizeof (int));

	if (!FEnqueue (vhqueTest, rgch))
		{
		CommSz (SzShared ("Enqueue Failed.\r\n"));
		}

	DumpQueue (vhqueTest);
}


/*  %%Function:TestPushQueue %%Owner:peterj  */
TestPushQueue ()
{
	int i;
	CHAR rgch [cbTest];

	if (vhqueTest == hNil)
		vhqueTest = HInitQue (cbTest, iIncrTest);

	CommSzNum (SzShared ("PushQueueing: "), ++cEnqueue);
	for (i = sizeof (int); i < cbTest; i++)
		rgch [i] = i;

	bltb (&cEnqueue, rgch, sizeof (int));

	if (!FPushQueue (vhqueTest, rgch))
		{
		CommSz (SzShared ("PushQueue Failed.\r\n"));
		}

	DumpQueue (vhqueTest);
}


/*  %%Function:TestDequeue %%Owner:peterj  */
TestDequeue ()
{
	CHAR rgch [cbTest];

	if (FDequeue (vhqueTest, rgch))
		{
		int i;
		BOOL fValid = fTrue;
		int cItem;
		bltb (rgch, &cItem, sizeof (int));
		for (i = sizeof (int); i < cbTest; i++)
			if (rgch [i] != i)
				fValid = fFalse;

		if (fValid)
			CommSzNum (SzShared ("Successful Dequeue: "), cItem);
		else
			CommSzNum (SzShared ("Bogus Dequeue: "), cItem);
		}
	else
		CommSz (SzShared ("Dequeue Failed.\n\r"));

	DumpQueue (vhqueTest);
}


/*  %%Function:DumpQueue %%Owner:peterj  */
DumpQueue (hque)
struct QUE **hque;

{
	int i, cLine, cTotal, n, cb;
	struct QUE *pque;
	CHAR *pch;
	CHAR szOut [cchSzOutMax];

	if (hque == hNil)
		{
		CommSz ("No Queue (hNil).\n\r");
		return;
		}

	FreezeHp ();
	pque = *hque;
	pch = szOut;

	PutSzInCommSz (SzShared ("cb:"), 0, &pch);
	PutIntInCommSz (pque->cb, 2, 4, &pch);

	PutSzInCommSz (SzShared ("iIncr:"), 0, &pch);
	PutIntInCommSz (pque->iIncr, 2, 4, &pch);

	PutSzInCommSz (SzShared ("c:"), 0, &pch);
	PutIntInCommSz (pque->c, 3, 5, &pch);

	PutSzInCommSz (SzShared ("iFirst:"), 0, &pch);
	PutIntInCommSz (pque->iFirst, 3, 5, &pch);

	PutSzInCommSz (SzShared ("iLim:"), 0, &pch);
	PutIntInCommSz (pque->iLim, 3, 5, &pch);

	PutSzInCommSz (SzShared ("iMax:"), 0, &pch);
	PutIntInCommSz (pque->iMax, 3, 5, &pch);

	EndCommSz (&pch, szOut);

	CommSz (szOut);

	cTotal = pque->c;
	i = pque->iFirst;
	cb = pque->cb;

	while (cTotal > 0)
		{
		cLine = 12;
		pch = szOut;
		while (cLine-- && cTotal--)
			{
			bltb (pque->rgb + (cb*i++), &n, sizeof (int));
			if (i >= pque->iMax)
				i = 0;
			PutIntInCommSz (n, 3, 5, &pch);
			}
		EndCommSz (&pch, szOut);
		CommSz (szOut);
		}

	CommSz (SzShared ("----\n\r"));
	MeltHp ();
}


#endif /* ENABLE */


/*  %%Function:TestDdeMisc %%Owner:peterj  */
TestDdeMisc ()
{
}


#endif /* DEBUG */


