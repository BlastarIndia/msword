/* D D E S U B . C */
/*  Common DDE functions */


#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "dde.h"
#include "doc.h"
#include "wininfo.h"
#include "props.h"
#include "sel.h"
#include "message.h"
#include "idle.h"

#include "debug.h"


#ifdef PROTOTYPE
#include "ddesub.cpt"
#endif /* PROTOTYPE */

extern char szClsDdeChnl[];
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
extern CHAR            szApp[];
extern IDF	       vidf;


int             docDde = docNil;
BOOL            vfDdeIdle = fFalse;
struct DDES     vddes = 
	{
	0, fFalse, fFalse,  /* server */
	0, fFalse, fFalse,  /* client */
	0,                  /* macro */
	0, hNil, hNil,      /* global */
	0                   /* request */
	};


csconst CHAR stAppDde[] = St("WinWord");

/* D O  D D E  I D L E */
/*  Performs the idle time operations required for dde.
*/
/*  %%Function:DoDdeIdle %%Owner:peterj  */
DoDdeIdle ()

{
	struct DQM dqm;

	DdeDbgCommSz ("DoDdeIdle: entered");
	Scribble (ispDdeIdle, 'D');

	/*  process messages from the dde message queue */
	while (FDequeue (vddes.hque, &dqm))
		{
		if (!FHandleDdeMsg (dqm.dcl, dqm.dwm + WM_DDE_FIRST, dqm.wLow,
				dqm.wHigh))
			{
			/* we can not handle this message at the moment, push it back! */
			/* can fail but there is nothing we can do if it does (unlikely). */
			FPushQueue (vddes.hque, &dqm);
			goto LInterrupt;
			}

		if (FMsgPresent (mtyIdle))
			goto LInterrupt;
		}

	if (FMsgPresent (mtyIdle))
		goto LInterrupt;

	/*  scan documents and determine if any open links or channels can be
		closed.  Also updates fHotLinksDirty. */
	if (vddes.cDclClient)
		if (!vfInsertMode)
			GarbageCollectDde ();
		else
			{
			vfAbortInsert = fTrue;
			goto LInterrupt;
			}

	if (FMsgPresent (mtyIdle))
		goto LInterrupt;

	/*  refresh ddehot fields for which there is new data */
	if (vddes.fHotLinksDirty)
		{
		Assert (vddes.cDclClient && !vfInsertMode);
		UpdateHotLinks ();
		}

	if (FMsgPresent (mtyIdle))
		goto LInterrupt;

	/*  for all server links, try to validate any that have been invalidated */
	if (vddes.fInvalidDdli)
		ValidateServerLinks ();

	if (FMsgPresent (mtyIdle))
		goto LInterrupt;

	/*  for any server links for which data has changed, send new data */
	if (vddes.fDirtyLinks)
		UpdateDirtyLinks ();

	if ((vddes.hque == hNil || (*vddes.hque)->c == 0) && 
			!vddes.fDirtyLinks && !vddes.fHotLinksDirty)
		vfDdeIdle = fFalse;

	DdeRpt2(DdeDbgCommSz ("DoDdeIdle: completed"));
	Debug( vdbs.fCkHeap ? CkHeap () : 0 );
	Scribble (ispDdeIdle, '*');
	return;

LInterrupt:
	DdeDbgCommSz ("DoDdeIdle: interrupted");
#ifdef DEBUG
		{
		extern MSG vmsgLast;
		DdeRpt2(ShowMsg ("di",vmsgLast.hwnd, vmsgLast.message, vmsgLast.wParam,
				vmsgLast.lParam));
		}
#endif
	Debug( vdbs.fCkHeap ? CkHeap () : 0 );
	Scribble (ispDdeIdle, '-');
	return;
}


/* D D E  C H N L  W N D  P R O C */
/* WndProc for dde channels */
/*  NOTE: Messages bound for this WndProc are filtered in wprocn.asm */
/*  %%Function:DdeChnlWndProc %%Owner:peterj  */
EXPORT LONG FAR PASCAL DdeChnlWndProc (hwnd, message, wParam, lParam)
HWND hwnd;
unsigned message;
WORD wParam;
LONG lParam;
{

	/*  this window only pays attention to DDE messages */
	Assert(message >= WM_DDE_FIRST && message <= WM_DDE_LAST
			&& message != WM_DDE_INITIATE);

#ifdef SHOWDDEMSG
		{
		int dcl = GetWindowWord (hwnd, IDWDDECHNLDCL);
		DdeDbgCommMsg ("WndProc", dcl, message, LOWORD(lParam),
				HIWORD(lParam));
		}
#endif /* SHOWDDEMSG */

	if (message == WM_DDE_ACK && vddes.fInInitiate)
		/*  is an ACK to our initiate, must handle now. */
		InitiateAck (hwnd, wParam, LOWORD (lParam), HIWORD (lParam));

	else
		{
		int dcl = GetWindowWord (hwnd, IDWDDECHNLDCL);
		struct DQM dqm;

		if (PdcldDcl (dcl)->hwndThem != wParam)
			/*  message from a different window, we ignore */
			{
			/*  terminate case is when multiple responses are received
				for our WM_DDE_INITIATE message.  Other cases are bogus.*/
			/*  Note: not freeing atoms & handles in other cases!! */
			Assert (message == WM_DDE_TERMINATE && 
					PdcldDcl (dcl)->dt != dtServer);
			DdeDbgCommMsg ("Tossing", dcl, message, LOWORD (lParam),
					HIWORD (lParam));
			}

		else
			{
#ifdef DEBUG
			struct DCLD *pdcld;
			pdcld  = PdcldDcl (dcl);
			Assert (pdcld->hwndUs == hwnd);
			Assert (pdcld->hwndThem == wParam);
			Assert (message >= WM_DDE_FIRST);
			Assert (message <= WM_DDE_LAST);
			Assert (message != WM_DDE_INITIATE);
#endif /* DEBUG */

			dqm.dcl = dcl;
			dqm.dwm = message - WM_DDE_FIRST;
			dqm.wLow = LOWORD (lParam);
			dqm.wHigh = HIWORD (lParam);

			/*  if queue does not exist yet, create it */
			if (vddes.hque == hNil)
				/* if this fails, we're still ok (enqueue and dequeue will fail) */
				vddes.hque = HInitQue (cbDQM, idqmMaxInit);

			/* if this fails we are pretty hosed.  but that just means we will
			   lose an incoming message.  if we are waiting for that message then
			   eventually we will time out.  if we weren't then we are fine. 
			   hopefully the app that sent the message knows how to time out too!
			*/
			if (FEnqueue (vddes.hque, &dqm))
				vfDdeIdle = fTrue;
			}
		}

	return fTrue;
}



/* F  H A N D L E  D D E  M S G */
/*  %%Function:FHandleDdeMsg %%Owner:peterj  */
FHandleDdeMsg (dcl, message, wLow, wHigh)
int dcl;
unsigned message;
int wLow, wHigh;

{

	BOOL f;
	struct DCLD *pdcld = PdcldDcl (dcl);
	BOOL fBusyPrevSave = pdcld->fBusyPrev;
	Assert (dcl != dclNil);

	pdcld->fBusyPrev = pdcld->fBusyCur;
	pdcld->fBusyCur = message == WM_DDE_ACK && wLow == dasBUSY;

	if (message == WM_DDE_TERMINATE)
		{
		DdeDbgCommMsg ("Terminate", dcl, message, wLow, wHigh);
		if (!PdcldDcl (dcl)->fTerminating)
			FPostDdeDcl (dcl, WM_DDE_TERMINATE, 0, 0);
		DestroyDcl (dcl);
		return fTrue;
		}
	else  
		switch (PdcldDcl (dcl)->dt)
			{
		case dtServer:
			f = FServerDdeMsg (dcl, message, wLow, wHigh);
			break;
		case dtClient:
			f = FClientDdeMsg (dcl, message, wLow, wHigh);
			break;
		case dtMacro:
			f = FMacroDdeMsg (dcl, message, wLow, wHigh);
			break;
#ifdef DEBUG
		default:
			Assert (fFalse);
			f = fTrue;
			break;
#endif /* DEBUG */
			}

	if (!f)
		{
		pdcld = PdcldDcl (dcl);
		pdcld->fBusyCur = pdcld->fBusyPrev;
		pdcld->fBusyPrev = fBusyPrevSave;
		Assert (vfInsertMode);
		DdeDbgCommMsg ("Cannot handle message", dcl, message, wLow, wHigh);
		}

	return f;

}



/* T E R M I N A T E  D C L */
/*  Sends a WM_DDE_TERMINATE message over channel dcl.
*/
/*  %%Function:TerminateDcl %%Owner:peterj  */
TerminateDcl (dcl)
int dcl;

{
	if (!PdcldDcl (dcl)->fTerminating)
		{
		struct DCLD *pdcld;

		FPostDdeDcl (dcl, WM_DDE_TERMINATE, 0, 0);
		pdcld = PdcldDcl (dcl);
		pdcld->fTerminating = fTrue;
		if (pdcld->dt == dtServer)
			pdcld->doc = docNil;
		}
	else  if (PdcldDcl (dcl)->fTermReceived)
		DestroyDcl (dcl);
}


/* T E R M I N A T E  D D E  D O C */
/*  Terminate all dde conversations regarding doc.
*/
/*  %%Function:TerminateDdeDoc %%Owner:peterj  */
TerminateDdeDoc (doc)
int doc;

{
	int dcl;
	struct DCLD **hdcld;

	DdeDbgCommInt ("Terminating DDE channels for :", doc);

	for (dcl = 0; dcl < dclMac; dcl++)
		if ((hdcld = mpdclhdcld [dcl]) != hNil && 
				(*hdcld)->dt == dtServer && (*hdcld)->doc == doc)
			TerminateDcl (dcl);
}


/* T E R M I N A T E  A L L  D D E  D C L */
/*  Terminate all active dde channels.
*/
/*  %%Function:TerminateAllDdeDcl %%Owner:peterj  */
TerminateAllDdeDcl (fMacroToo)
BOOL fMacroToo;

{
	int dcl;
	struct DCLD **hdcld;

	DdeDbgCommSz ("Terminating all DDE channels");

	for (dcl = 0; dcl < dclMac; dcl++)
		if ((hdcld = mpdclhdcld [dcl]) != hNil)
			{
			if ((*hdcld)->dt == dtMacro)
				if (fMacroToo)
					(*hdcld)->fTermRequest = fTrue;
				else
					continue;
			TerminateDcl (dcl);
			}
}


/* W A I T  D D E  T E R M I N A T E D */
/*  Wait until all dde channels are terminated.
*/
/*  %%Function:WaitDdeTerminated %%Owner:notused  */
WaitDdeTerminated ()

{
	struct DQM dqm;
	int dcl;
	int dclOpened;
	LONG usecTimeOut;

	DdeDbgCommSz ("Waiting for all DDE channels to terminate");
	DdeDbgCommLong ("Timeout at", usecTimeOut);

	/*  once vfDead is set we will accept no new conversations */
	Assert (vidf.fDead);
	Assert (!vfInsertMode);

	/*  assure all TERMINATE messages sent */
	TerminateAllDdeDcl (fTrue);

	/* wait for all channels to terminate OR for time out & user Ok */
	do
		{
		usecTimeOut = GetTickCount() + UsecDdeTimeOut ();

		do
			{
			DdeDbgCommLong ("Time now", GetTickCount ());

			/*  handle all waiting messages */
			while (FDequeue (vddes.hque, &dqm))
				AssertDo (FHandleDdeMsg (dqm.dcl, dqm.dwm+WM_DDE_FIRST,
						dqm.wLow, dqm.wHigh));

			/*  see if any channels are still active */
			dclOpened = dclNil;
			for (dcl = 0; dcl < dclMac; dcl++)
				if (mpdclhdcld [dcl] != hNil)
					{
					dclOpened = dcl;
					break;
					}

			/*  if channels active, wait for another DDE message 
				(yield if necessary) or until timeout. */
			}
		while (dclOpened != dclNil && 
				FWaitDdeMessage (usecTimeOut, 0L, NULL));

		}
	while (dclOpened != dclNil && FWaitLongerDde (dcl));

#ifdef DEBUG
	DdeDbgCommLong ("Complete at", GetTickCount());
	if (dclOpened != dclNil)
		DdeDbgCommSz ("Timeout during termination");
	else
		DdeDbgCommSz ("DDE terminated correctly");
#endif /* DEBUG */
}


/* F  W A I T  D D E  M E S S A G E */
/*  Wait for a dde message to arrive.  When one does, dispatch it then
	return.  Do not wait beyond time usecTimeOut.  Return false if timeout.
*/
/*  %%Function:FWaitDdeMessage %%Owner:peterj  */
FWaitDdeMessage (usecTimeOut, usecHourGlass, pfHourGlass)
LONG usecTimeOut, usecHourGlass;
BOOL *pfHourGlass;

{
	MSG msg;

	while (!PeekMessage ((LPMSG)&msg, NULL, WM_DDE_FIRST, WM_DDE_LAST,
			PM_REMOVE))
		{
		if (vddes.hque != hNil && (*vddes.hque)->c > 0)
			/* there is something on the queue, process it! */
			return fTrue;

		if (pfHourGlass != NULL && !*pfHourGlass && 
				GetTickCount() >= usecHourGlass)
			/* taking too long, put up HourGlass */
			{
			DdeDbgCommSz ("Putting up hour glass!");
			*pfHourGlass = fTrue;
			StartLongOp ();
			}
		if (GetTickCount() >= usecTimeOut)
			{
			DdeDbgCommSz ("TIME OUT!!");
			return fFalse;
			}
		OurYield (fTrue);
		}

	DispatchMessage ((LPMSG) &msg);
	return fTrue;
}


/* U S E C  D D E  T I M E  O U T */
/*  Get the timeout time for dde operations from the win.ini file.
*/
/*  %%Function:UsecDdeTimeOut %%Owner:peterj  */
LONG UsecDdeTimeOut ()

{
	if (!vddes.secTimeOut)
		{
		vddes.secTimeOut = GetProfileInt ((LPSTR)szApp,
				(LPSTR)SzSharedKey ("DdeTimeOut",DdeTimeOutDef), secDdeTimeOutDef);
		if (!vddes.secTimeOut)
			vddes.secTimeOut++;
		}
	return (LONG)vddes.secTimeOut * 1000l;
}


/* F  W A I T  L O N G E R  D D E */
/*  Prompt the user to determine if we should wait longer.
*/
/*  %%Function:FWaitLongerDde %%Owner:peterj  */
FWaitLongerDde (dcl)
int dcl;

{
	struct DCLD *pdcld = PdcldDcl (dcl);
	int rgw [2];

	return IdMessageBoxMstRgwMb (mstDdeTimeOut, rgw,
			(MB_SYSTEMMODAL | MB_ICONHAND | MB_YESNO | MB_DEFBUTTON2))
			== IDYES;
}


/* D E S T R O Y  D C L */
/*  Destroy channel dcl.  Must destroy any references to the channel.  There
	are no active external references (i.e., WM_DDE_TERMINATE already
	received).
*/
/*  %%Function:DestroyDcl %%Owner:peterj  */
DestroyDcl (dcl)
int dcl;

{
	struct DDLI ddli;
	struct DCLD *pdcld = PdcldDcl (dcl);

	switch (pdcld->dt)
		{
#ifdef DEBUG
	default:
		Assert (fFalse);
		break;
#endif /* DEBUG */
	case dtServer:
		/*  delete any links which use this channel */
		if (vddes.hplddli != hNil)
			{
			struct PL **hplddli = vddes.hplddli;
			int iddli = IMacPl( hplddli );

			while (iddli--)
				{
				GetPl( hplddli, iddli, &ddli );
				if (ddli.dcl == dcl)
					{
					DeleteDdliServer (iddli);
					}
				}
			}
		if (PdcldDcl(dcl)->fExecuting)
			{
			PdcldDcl(dcl)->fTermReceived = fTrue;
			return;
			}
		Assert (vddes.cDclServer > 0);
		if (--vddes.cDclServer)
			vddes.fInvalidDdli = vddes.fDirtyLinks = fFalse;
		break;

	case dtClient:
		DeleteAtom (pdcld->atomApp);
		DeleteAtom (pdcld->atomTopic);
		UnreferenceClientDdlis (dcl, iddliNil);
		DeleteDdlisOfDclClient (dcl);
		Assert (vddes.cDclClient > 0);
		if (--vddes.cDclClient == 0)
			vddes.fHotLinksDirty = fFalse;
		break;

	case dtMacro:
		if (!pdcld->fTermRequest)
			/*  We do not have the user's permission to actually destroy.
				Indicate that the other app has terminated but keep the
				structure around.
			*/
			{
			pdcld->fTermReceived = fTrue;
			pdcld->fTerminating = fTrue;
			return;
			}
		Assert (vddes.cDclMacro > 0);
		vddes.cDclMacro--;
		break;
		}
	DestroyWindow (PdcldDcl (dcl)->hwndUs);
	FreeCls (clsDCLD, dcl);
}




/* F  P O S T  D D E  D C L */
/*  Posts a message over channel dcl.  Does not post if channel is
	being terminated.  Tries several times if PostMessage is failing.
	Returns true if the message was posted.
*/
/*  %%Function:FPostDdeDcl %%Owner:peterj  */
FPostDdeDcl (dcl, message, wLow, wHigh)
int dcl;
unsigned message;
int wLow, wHigh;

{
	HWND hwnd = PdcldDcl (dcl)->hwndThem;

	DdeDbgCommPost (dcl, message, wLow, wHigh);

	if (!IsWindow (hwnd) || PdcldDcl(dcl)->fTerminating)
		{
		/* cannot post message, they don't exist or we have already sent
			a TERMINATE message */
		DdeDbgCommSz ("Post Failed: no window or terminating");
		return fFalse;
		}

	else
		{
		int cTries = 100;
		MSG msg;
		while (!PostMessage (hwnd, message, PdcldDcl (dcl)->hwndUs, 
				MAKELONG (wLow, wHigh)) && --cTries)
			/*  their queue must be full, yield so they can empty it */
			OurYield (fFalse);
#ifdef DEBUG
		if (!cTries)
			DdeDbgCommSz("Post Failed: queue full!");
#endif /* DEBUG */
		return cTries > 0;
		}
}



/* I D D L I  F R O M  D C L  I T E M */
/*  %%Function:IddliFromDclItem %%Owner:peterj  */
IddliFromDclItem (dcl, atomItem)
int dcl;
ATOM atomItem;

{
	struct DCLD *pdcld = PdcldDcl (dcl);
	struct DDLI ddli;

	Assert (dcl != dclNil);

	switch (pdcld->dt)
		{
	case dtServer:
			{
			int iddli;
			struct PL **hplddli;

			if ((hplddli = vddes.hplddli) == hNil)
				break;

			for ( iddli = 0; iddli < IMacPl( hplddli ); iddli++ )
				{
				GetPl( hplddli, iddli, &ddli );
				if (ddli.dcl == dcl && ddli.atomItem == atomItem)
					return iddli;
				}
			break;
			}

	case dtClient:
			{
			int iddli, iddliMacM1;
			struct PLCDDLI **hplcddli;

			if (docDde == docNil)
				return iddliNil;

			hplcddli = PdodDoc (docDde)->hplcddli;
			iddliMacM1 = IMacPlc( hplcddli ) - 1; /* -1 due to entry at EOD */

			for ( iddli = iddliMinNormal; iddli < iddliMacM1; iddli++ )
				{
				GetPlc( hplcddli, iddli, &ddli );
				if (ddli.dcl == dcl && ddli.atomItem == atomItem)
					return iddli;
				}
			break;
			}

#ifdef DEBUG
	default:
		Assert (fFalse);
	case dtMacro:
		break;
#endif /* DEBUG */
		}

	return iddliNil;
}


/* P D C L D  D C L */
/*  %%Function:PdcldDcl %%Owner:peterj  */
NATIVE struct DCLD *PdcldDcl (dcl)
int dcl;
{
	extern struct DCLD  **mpdclhdcld[];
	Assert (mpdclhdcld[dcl] != hNil);
	return *mpdclhdcld[dcl];
}


/* D C L  C R E A T E  C H A N N E L */
/*  Create a channel and its window.
*/
/*  %%Function:DclCreateChannel %%Owner:peterj  */
DclCreateChannel (dt)
int dt;

{
	int dcl;
	HWND hwnd;
	CHAR *pch;
	CHAR szWindowName [cchMaxSz];

	if (vmerr.fDclFull|| (dcl = IAllocCls (clsDCLD, cwDCLD)) == dclNil)
		return dclNil;

	/* get dcl name */
	StToSz (StNear(stAppDde), szWindowName);
	pch = szWindowName + stAppDde[0];
	*pch++ = '_';
	CchIntToPpch (dcl, &pch);
	*pch = 0;

	if ((hwnd = 
			CreateWindow ((LPSTR)szClsDdeChnl, (LPSTR)szWindowName, 
			WS_CHILD, 0, 0, 0, 0, vhwndApp, NULL, vhInstance, 
			(LPSTR) NULL)) == NULL)
		{
		FreeCls (clsDCLD, dcl);
		return dclNil;
		}

	SetWindowWord (hwnd, IDWDDECHNLDCL, dcl);
	PdcldDcl (dcl)->hwndUs = hwnd;
	PdcldDcl (dcl)->dt = dt;
	switch (dt)
		{
	case dtServer:
		vddes.cDclServer++;
		break;
	case dtClient:
		vddes.cDclClient++;
		break;
	case dtMacro:
		vddes.cDclMacro++;
		break;
#ifdef DEBUG
	default:
		Assert (fFalse);
#endif /* DEBUG */
		}

	return dcl;
}



/* S T  F R O M  A T O M */
/*  %%Function:StFromAtom %%Owner:peterj  */
StFromAtom (st, cchMax, atom)
CHAR *st;
int cchMax;
ATOM atom;

{
	int cch;
	if (atom == NULL)
		*st = 0;
	else
		{
		cch = GlobalGetAtomName (atom, (LPSTR)st+1, cchMax-1);
		Assert (cch > 0);
		*st = cch;
		}
}


/* A T O M  A D D  S T */
/* Add an atom for st.  sets vmerr.fMemFail if failed (and returns NULL) */
/*  %%Function:AtomAddSt %%Owner:peterj  */
ATOM AtomAddSt (st)
CHAR *st;

{
	ATOM atom;
	CHAR sz [cchMaxSz];
	if (!*st)
		return NULL;
	StToSz (st, sz);
	atom = GlobalAddAtom ((LPSTR)sz);
	if (atom == NULL)
		SetErrorMat(matMem);
	return atom;
}


/* D E L E T E  A T O M */
/*  %%Function:DeleteAtom %%Owner:peterj  */
DeleteAtom (atom)
ATOM atom;

{
	if (atom != NULL)
		{
		GlobalDeleteAtom (atom);
		}
}


/* A T O M  F R O M  S T  N A */
/*  get the atom for st then delete it again. Meaningful only if someone
	else has the atom too.
*/
/*  %%Function:AtomFromStNA %%Owner:peterj  */
ATOM AtomFromStNA (st)
CHAR *st;

{
	CHAR sz [cchMaxSz];
	if (!*st)
		return NULL;
	StToSz (st, sz);
	return GlobalFindAtom ((LPSTR)sz);
}


/* R E A D D  A T O M */
/*  %%Function:ReaddAtom %%Owner:peterj  */
ReaddAtom (atom)
ATOM atom;

{
	CHAR st [cchMaxSz];
	StFromAtom (st, cchMaxSz, atom);
	/* since this is just bumping a count, it is guaranteed to succeed */
	StartGuaranteedHeap();
	AtomAddSt (st);
	EndGuarantee();
}



/* H  I N I T  Q U E */
/*  Initialize a queue with structure size cb and in increment of
	iIncr strucutres.  Return handle or hNil.  Note: will not fail
	if !vmerr.fMemFail (unless cb or iIncr large).
*/
/*  %%Function:HInitQue %%Owner:peterj  */
HInitQue (cb, iIncr)
int cb, iIncr;

{
	struct QUE **hque, *pque;

	if ((hque = HAllocateCw (CwFromCch (cb * iIncr + cbQUEBase))) != hNil)
		{
		pque = *hque;
		pque->c = pque->iFirst = pque->iLim = 0;
		pque->iMax = pque->iIncr = iIncr;
		pque->cb = cb;
		}

	return hque;
}



/* F  E N Q U E U E */
/*  Place pch into queue hque. Return false if failed.
	May cause heap movement.
*/
/*  %%Function:FEnqueue %%Owner:peterj  */
FEnqueue (hque, pch)
struct QUE **hque;
CHAR *pch;

{
	struct QUE *pque;
	int cb;

	if (hque == hNil)
		return fFalse;

	FreezeHp ();
	pque = *hque;
	cb = pque->cb;

	/*  check to see if queue is full */
	if (pque->c == pque->iMax)
		/* must expand queue, increase by iIncr */
		{
		int iIncr = pque->iIncr;
		int cbExpand = cbQUEBase + ((pque->iMax + iIncr) * cb);
		MeltHp ();
		if (!FChngSizeHCw (hque, CwFromCch (cbExpand), fFalse))
			return fFalse;
		FreezeHp ();
		pque = *hque;
		Assert (pque->iLim >= pque->iFirst); /* because full */
		if (pque->iLim <= pque->iFirst)
			{
			/*  queue had wrap around, must move the upper portion up */
			int iFirstOld = pque->iFirst;
			bltb (pque->rgb + iFirstOld * cb,
					pque->rgb + (iFirstOld + iIncr) * cb,
					(pque->iMax - iFirstOld) * cb);
			pque->iFirst += iIncr;
			}
		pque->iMax += iIncr;
		}

	/* Insert the new element */

	/*  handle wrap around */
	Assert (pque->iLim <= pque->iMax);
	if (pque->iLim == pque->iMax)
		pque->iLim = 0;

	bltb (pch, pque->rgb + pque->iLim * cb, cb);
	pque->iLim++;
	pque->c++;
	MeltHp ();

	return fTrue;
}


/* F  D E Q U E U E */
/*  Removes first item from queue.  May cause heap movement.
	Returns true if there was an element in the queue, false if the
	queue was empty.
*/
/*  %%Function:FDequeue %%Owner:peterj  */
FDequeue (hque, pch)
struct QUE **hque;
CHAR *pch;

{
	struct QUE *pque;
	int cb;

	/*  are there any entries? */
	if (hque == hNil || (pque = *hque)->c == 0)
		return fFalse;

	FreezeHp ();
	cb = pque->cb;

	/*  get the first message in the queue */
	bltb (pque->rgb + (pque->iFirst++ * cb), pch, cb);

	/*  handle wrap around */
	Assert (pque->iFirst <= pque->iMax);
	if (pque->iFirst == pque->iMax)
		pque->iFirst = 0;

	if (--pque->c < (pque->iMax / 2) && pque->iMax > pque->iIncr)
		/* shrink the queue */
		{
		int iIncr = pque->iIncr;
		int cShrink = ((pque->iMax / 2)/iIncr) * iIncr;
		int cbNew, iMaxNew;
		cbNew = cbQUEBase + ((iMaxNew = pque->iMax - cShrink) * cb);
		Assert (iMaxNew % iIncr == 0 && iMaxNew >= iIncr
				&& iMaxNew > pque->c);

		/* shift data around? Not handdling case of wrap around */
		if (pque->c != 0)
			{
			if (pque->iLim > pque->iFirst)
				{
				int dShift = pque->iFirst;
				bltb (pque->rgb + (dShift * cb),
						pque->rgb, (pque->iLim -= dShift) * cb);
				pque->iFirst = 0;
				}
			}
		else
			pque->iFirst = pque->iLim = 0;

		if (pque->iFirst < iMaxNew && pque->iLim <= iMaxNew)
			{
			MeltHp ();
			FChngSizeHCw (hque, CwFromCch (cbNew), fTrue);
			FreezeHp ();
			(*hque)->iMax = iMaxNew;
			}
		}

	MeltHp ();
	return fTrue;
}


/* F  P U S H  Q U E U E */
/*  Place pch at the fron of queue hque. Return false if failed.  
	May cause heap movement.
*/
/*  %%Function:FPushQueue %%Owner:peterj  */
FPushQueue (hque, pch)
struct QUE **hque;
CHAR *pch;

{
	struct QUE *pque;
	CHAR *pchFirst;
	int cb;

	if (!FEnqueue (hque, pch))
		return fFalse;

	pque = *hque;
	cb = pque->cb;
	pchFirst = pque->rgb + (pque->iFirst * cb);

	if (pque->iFirst >= pque->iLim)
		{
		bltb (pque->rgb, pque->rgb+cb, (pque->iLim-1) * cb);
		bltb (pque->rgb + (pque->iMax - 1) * cb, pque->rgb, cb);
		}

	bltb (pchFirst, pchFirst + cb, (pque->iMax - pque->iFirst - 1) * cb);
	bltb (pch, pchFirst, cb);

	return fTrue;
}



