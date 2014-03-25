/* E L D D E . C */
/*  Macro statements for DDE  
	and Key driving code (ported from Excel's DRIVE.C)

	Macro statements:
		Initiate
	Terminate
	TerminateAll
	Request$
	Poke
	Execute
	SendKeys
	ActivateApp
*/

#include "word.h"
#include "heap.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "dde.h"
#include "el.h"
#include "macrocmd.h"
#include "strtbl.h"
#include "debug.h"
#include "rerr.h"
#include "props.h"
#include "ch.h"
#include "doc.h"

#define ELDDE
#include "inter.h"
#include "resource.h"


#ifdef PROTOTYPE
#include "eldde.cpt"
#endif /* PROTOTYPE */

/* globals */
FARPROC lpfnPlaybackHook;
HANDLE hPlaybackHook;
HEVT FAR *lphevtHead;
HANDLE FAR *lphrgbKeyState;
FARPROC FAR *lplpfnPlaybackHookSave;

/* externals */
extern BOOL fElActive;
extern char rgchEop [];
extern ENV *penvMem;
extern HWND vhwndApp;
extern BOOL vfDeactByOtherApp;
extern CHAR stEmpty[];
extern struct MERR      vmerr;
extern struct DDES vddes;
extern BOOL vfInsertMode;
extern struct DCLD **mpdclhdcld[];
extern int dclMac;
extern int flashID;
extern int sbStrings;
extern int vwWinVersion;


/* E L  D D E  I N I T I A T E */
/*  Initiate macro command.  Creates a channel to app, topic.
*/
/* %%Function:ElDDEInitiate %%Owner:peterj */
EL ElDDEInitiate (pstApp, pstTopic)
CHAR ** pstApp, ** pstTopic;

{
	int dcl = dclNil;
	ATOM atomApp, atomTopic;

	DdeDbgCommSz ("El Initiate");

	atomApp = AtomAddSt (*pstApp); /* failure for these checked below (fMemFail) */
	atomTopic = AtomAddSt (*pstTopic);

	if (!vmerr.fMemFail)
		{
		dcl = DclInitiateLaunch (atomApp, atomTopic, dtMacro);
		}

	DeleteAtom (atomApp);
	DeleteAtom (atomTopic);

#ifdef DEBUG
	if (dcl != dclNil)
		DdeRpt2(DdeDbgCommSz ("El Initiate succeeds."));
	else
		DdeRpt2(DdeDbgCommSz ("El Initiate fails."));
#endif /* DEBUG */

	if (dcl == dclNil)
		{
		RtError (rerrCannotInitiate);
		/* never returns */
		Assert(fFalse);
		}

	return dcl;
}



/* E L  D D E  T E R M I N A T E */
/*  Terminate macro channel dcl.
*/
/* %%Function:ElDDETerminate %%Owner:peterj */
EL ElDDETerminate (dcl)
int dcl;

{
	struct DCLD *pdcld;

	DdeDbgCommSz ("El Terminate");

	if (!FResetMacroDcl (dcl))
		{
		RtError (rerrBogusDcl);
		}
	else
		{
		PdcldDcl (dcl)->fTermRequest = fTrue;
		TerminateDcl (dcl);
		}

	return 0;
}


/* E L  D D E  T E R M I N A T E  A L L */
/*  Terminate all active macro dde channels.
*/
/* %%Function:ElDDETermAll %%Owner:peterj */
EL ElDDETermAll ()

{
	struct DCLD **hdcld;
	int dcl;

	DdeDbgCommSz ("El TerminateAll");

	for (dcl = 0; dcl < dclMac; dcl++)
		if ((hdcld = mpdclhdcld [dcl]) != hNil && (*hdcld)->dt == dtMacro)
			{
			(*hdcld)->fTermRequest = fTrue;
			TerminateDcl (dcl);
			}
	return 0;
}



/* E L  D D E  E X E C U T E */
/*  Send an Execute message across channel dcl.  Wait for application to
	Ack or Nack.  Doesn't retry on busy.
*/
/* %%Function:ElDDEExecute %%Owner:peterj */
EL ElDDEExecute (dcl, pstExec)
int dcl;
CHAR ** pstExec;

{
	int rerr = rerrNil;
	BOOL fTimeout = fFalse;
	HANDLE hExec = NULL;
	CHAR FAR *lpch;

	DdeDbgCommSz ("El Execute");

	if (!FResetMacroDcl (dcl))
		{
		RtError (rerrBogusDcl);
		/* never returns */
		}

	if ((hExec = OurGlobalAlloc(GMEM_DDE, (DWORD)**pstExec + 1))
			== NULL || (lpch = GlobalLockClip (hExec)) == NULL)
		/* error: cannot allocate */
		{
		rerr = rerrOutOfMemory;
		goto LReturn;
		}

	bltbx ((CHAR FAR *)*pstExec + 1, lpch, **pstExec);
	lpch [**pstExec] = 0;
	GlobalUnlock (hExec);

	Assert (!PdcldDcl (dcl)->fResponse && !PdcldDcl (dcl)->fAck &&
			!PdcldDcl (dcl)->fBusy);

	if (!FPostDdeDcl (dcl, WM_DDE_EXECUTE, 0, hExec)
			|| (fTimeout = !FWaitOnDclMacro (dcl)))
		/* error: cannot post */
		{
		if (fTimeout)
			/* NOTE: handle will never be freed */
			hExec = NULL;
		rerr = rerrNoResponse;
		goto LReturn;
		}

	if (!PdcldDcl (dcl)->fAck)
		{
		rerr = PdcldDcl (dcl)->fBusy ? rerrAppBusyed : rerrAppNacked;
		goto LReturn;
		}

LReturn:
#ifdef DEBUG
	if (rerr == rerrNil)
		DdeRpt2(DdeDbgCommSz ("El Execute succeeds."));
	else
		DdeRpt2(DdeDbgCommSz ("El Execute fails."));
#endif /* DEBUG */

	if (hExec != NULL)
		GlobalFree (hExec);

	if (rerr != rerrNil)
		RtError (rerr);

	return 0; /* to keep the stack happy */
}


/* E L  D D E  P O K E */
/*  Send a POKE message for item *pstItem across channel dcl with data
	sdData (format CF_TEXT).  Waits for Ack or Nack, does not retry on Busy.
*/
/* %%Function:ElDDEPoke %%Owner:peterj */
EL ElDDEPoke (dcl, pstItem, hpsdData)
int dcl;
CHAR **pstItem;
SD HUGE *hpsdData;

{
	int rerr = rerrNil;
	int cch;
	CHAR huge *hpch;
	BOOL fTimeout = fFalse;
	HANDLE hData = NULL;
	ATOM atomItem;
	CHAR FAR *lpch;
	struct DMS dms;

	DdeDbgCommSz ("El Poke");

	if (!FResetMacroDcl (dcl))
		/* error: bogus dcl */
		{
		RtError (rerrBogusDcl);
		/* never returns */
		}

	atomItem = AtomAddSt (*pstItem); 
	if (vmerr.fMemFail)
		{
		rerr = rerrOutOfMemory;
		goto LReturn;
		}

	cch = CchFromSd (*hpsdData);

	if ((hData = OurGlobalAlloc(GMEM_DDE, (long)(uns)(cch + 1 + cbDMS)))
			== NULL || (lpch = GlobalLockClip (hData)) == NULL)
		/* error: cannot allocate */
		{
		DeleteAtom(atomItem);
		rerr = rerrOutOfMemory;
		goto LReturn;
		}

	SetBytes (&dms, 0, cbDMS);
	dms.cf = CF_TEXT;
	bltbx ((CHAR FAR *)&dms, lpch, cbDMS);
	lpch += cbDMS;  	    	    	/* copy from SD to hData */
	hpch = HpchFromSd (*hpsdData);
	while (cch--)
		*lpch++ = *hpch++;
	*lpch = 0;
	GlobalUnlock (hData);

	Assert (!PdcldDcl (dcl)->fResponse && !PdcldDcl (dcl)->fAck &&
			!PdcldDcl (dcl)->fBusy);

	if (!FPostDdeDcl (dcl, WM_DDE_POKE, hData, atomItem)
			|| (fTimeout = !FWaitOnDclMacro (dcl)))
		/* error: cannot post */
		{
		if (fTimeout)
			/* NOTE: handle will never be freed */
			hData = NULL;
		DeleteAtom(atomItem);
		rerr = rerrNoResponse;
		goto LReturn;
		}

	if (!PdcldDcl (dcl)->fAck)
		{
		rerr = PdcldDcl (dcl)->fBusy ? rerrAppBusyed : rerrAppNacked;
		goto LReturn;
		}

LReturn:
#ifdef DEBUG
	if (rerr == rerrNil)
		DdeRpt2(DdeDbgCommSz ("El Poke succeeds."));
	else
		DdeRpt2(DdeDbgCommSz ("El Poke fails."));
#endif /* DEBUG */

	if (hData != NULL)
		GlobalFree (hData);

	if (rerr != rerrNil)
		RtError (rerr);

	return 0; /* to keep the stack happy */
}


/* E L  S D  D D E  R E Q U E S T */
/*  Send a request for pstItem across channel dcl (format CF_TEXT).  Wait
	for data or Nack.  Does not retry on Busy.
*/
/* %%Function:ElSdDDERequest %%Owner:peterj */
EL SD ElSdDDERequest (dcl, pstItem)
int dcl;
CHAR ** pstItem;

{
	int rerr = rerrNil;
	ATOM atomItem;
	SD sdResult;

	DdeDbgCommSz ("El Request$");

	if (!FResetMacroDcl (dcl))
		/* error: bogus dcl */
		{
		RtError (rerrBogusDcl);
		/* never returns */
		}

	atomItem = AtomAddSt (*pstItem);
	if (vmerr.fMemFail)
		{
		rerr = rerrOutOfMemory;
		goto LReturn;
		}

	sdResult = sdNil;
	Assert (vddes.sdResult == sdNil || vddes.sdResult == 0);

	Assert (!PdcldDcl (dcl)->fResponse && !PdcldDcl (dcl)->fAck &&
			!PdcldDcl (dcl)->fBusy);

	if (!FPostDdeDcl (dcl, WM_DDE_REQUEST, CF_TEXT, atomItem))
		{
		/* error: cannot post or timeout */
		rerr = rerrNoResponse;
		DeleteAtom(atomItem);
		goto LReturn;
		}
	if (!FWaitOnDclMacro (dcl))
		{
		/* error: cannot post or timeout */
		rerr = rerrNoResponse;
		goto LReturn;
		}

	if (!PdcldDcl (dcl)->fAck)
		{
		rerr = PdcldDcl (dcl)->fBusy ? rerrAppBusyed : rerrAppNacked;
		goto LReturn;
		}

	sdResult = vddes.sdResult;

LReturn:
#ifdef DEBUG
	if (rerr == rerrNil)
		{
		Assert (sdResult != sdNil);
		DdeRpt2(DdeDbgCommSz ("El Request$ succeeds."));
		}
	else
		DdeRpt2(DdeDbgCommSz ("El Request$ fails."));
#endif /* DEBUG */

	vddes.sdResult = sdNil;

	if (rerr != rerrNil)
		RtError (rerr);

	return sdResult;
}


#ifdef LATER  /* FUTURE: possible additions to DDE spec */
/* %%Function:ElDDEAdvise %%Owner:NOTUSED */
EL ElDDEAdvise (pstApp, pstTopic, pstItem)
CHAR **pstApp, **pstTopic, **pstItem;

{
	/* Set up a DDE advise.
		Send Advise message
		
		When new data is available invoke AutoDde macro.
		Advise is maintained until session end or Unadvise statement
		executed for that app topic item.
	
		Errors:
		cannot advise
	
	*/

	return 0; /* to keep the stack happy */
}


/* %%Function:ElDDEUnadvise %%Owner:NOTUSED */
EL ElDDEUnadvise (pstApp, pstTopic, pstItem)
CHAR **pstApp, **pstTopic, **pstItem;

{
	/* Remove previously set up DDE advise.
		Unadvise item.
		
		Errors:
		advise not active on item
	*/

	return 0; /* to keep the stack happy */
}


#endif /* LATER */


/* F  R E S E T  M A C R O  D C L */
/*  Determine if dcl is a valid macro channel.  If it is reset all of its
	special flags and return true, else return false.
*/
/* %%Function:FResetMacroDcl %%Owner:peterj */
FResetMacroDcl (dcl)
int dcl;

{
	struct DCLD *pdcld, **hdcld;

	if (dcl > 0 && dcl < dclMac && (hdcld = mpdclhdcld [dcl]) != hNil
			&& (pdcld = *hdcld)->dt == dtMacro && !pdcld->fTerminating &&
			!pdcld->fTermRequest && !pdcld->fTermReceived)
		{
		pdcld->fResponse = pdcld->fAck = pdcld->fBusy = fFalse;
		return fTrue;
		}
	else
		return fFalse;
}


/* F  W A I T  O N  D C L  M A C R O */
/*  Wait until channel dcl receives a response or until a timeout.
*/
/* %%Function:FWaitOnDclMacro %%Owner:peterj */
FWaitOnDclMacro (dcl)
int dcl;

{
	struct DQM dqm;
	struct DCLD *pdcld;
	BOOL fHourGlass = fFalse;
	BOOL fWaiting;
	LONG usecTimeOut;
	LONG usecHourGlass = GetTickCount () + usecHourGlassDef;

	Assert (!vfInsertMode);
	Assert (PdcldDcl (dcl)->dt == dtMacro);

	do
		{
		usecTimeOut = GetTickCount () + UsecDdeTimeOut ();

		do
			{
			while (FDequeue (vddes.hque, &dqm))
				AssertDo (FHandleDdeMsg (dqm.dcl, dqm.dwm + WM_DDE_FIRST,
						dqm.wLow, dqm.wHigh));
			}
		while ((fWaiting = !(pdcld = PdcldDcl(dcl))->fResponse)
				&& !pdcld->fTermReceived && 
				FWaitDdeMessage (usecTimeOut, usecHourGlass, &fHourGlass));

		}
	while (fWaiting && !PdcldDcl(dcl)->fTermReceived && FWaitLongerDde (dcl));

	if (fHourGlass)
		EndLongOp(fFalse);

	return !fWaiting;

}





/* F  M A C R O   D D E  M S G */
/*  Handle all dde messages for macro channels.  Return fFalse if we cannot
	handle the message at this time (we will be given the message back
	later).
*/
/* %%Function:FMacroDdeMsg %%Owner:peterj */
FMacroDdeMsg (dcl, message, wLow, wHigh)
int dcl;
unsigned message;
int wLow, wHigh;

{
	DdeDbgCommMsg ("Macro Message:", dcl, message, wLow, wHigh);

#ifdef DEBUG
	if (PdcldDcl (dcl)->fResponse)
		ReportDdeError ("Macro message received while not waiting.",
				dcl, message, 0);
#endif /* DEBUG */

	/*  anything is a response */
	PdcldDcl (dcl)->fResponse = fTrue;
	/* PdcldDcl (dcl)->fAck = fFalse; */
	/* PdcldDcl (dcl)->fBusy = fFalse; */

	switch (message)
		{
	case WM_DDE_DATA:
			{
			CHAR FAR *lpch;
			struct DMS dms, FAR *lpdms;
			BOOL fMustFree = fFalse;
			BOOL fSuccess = fFalse;

			if (!fElActive || wLow == NULL)
				/* Macro done or DATA message without data */
				{
#ifdef DEBUG
				if (wLow == NULL)
					ReportDdeError ("DATA message without data, NACKing",
						dcl, message, 0);
				else
					ReportDdeError ("Macro DATA message after macro end, NACKing",
						dcl, message, 0);
#endif /* DEBUG */
				goto LDataNack;
				}
			if ((lpch = lpdms = GlobalLockClip (wLow)) == NULL)
				{
				ReportDdeError ("Cannot lock handle, dropping message",
						dcl, message, 0);
				return fTrue;  /* drop the message */
				}
			dms = *lpdms;

			DdeRpt2(CommSzRgNum (SzShared("WM_DDE_DATA: dms (grpf,cf): "),
					&dms, CwFromCch(cbDMS)));

			if (!dms.fResponse || dms.cf != CF_TEXT)
				/* data should be in response to a WM_DDE_REQUEST */
				{
				ReportDdeError ("Unexpected Data (not response) or wrong format to macro.",
						dcl, message, 0);
				GlobalUnlock (wLow);
				goto LDataNack;
				}

			/* get past info block */
			lpch += cbDMS;

				/* Get data and put in sdResult */
				{
				char far * lpchT;
				unsigned cch;

				for (lpchT = lpch, cch = 0; *lpchT != '\0'; ++lpchT, ++cch)
					;

				if (cch < 0 || cch >= 0x7ff0 ||
						(vddes.sdResult = SdCopyLpchCch (lpch, cch)) == sdNil)
					{
					vddes.sdResult = sdNil;
					}
				}

			GlobalUnlock (wLow);


			if (vddes.sdResult != sdNil)
				{
				DdeDbgCommSz ("Macro data received OK");
				PdcldDcl (dcl)->fAck = fTrue;
				fSuccess = fTrue;
				if (!dms.fAck || (fMustFree = !FPostDdeDcl (dcl, 
						WM_DDE_ACK, dasACK, wHigh)))
					DeleteAtom (wHigh);
				}
			else
				/* couldn't read, NACK */
				{
LDataNack:
				if (fMustFree = !FPostDdeDcl (dcl, WM_DDE_ACK, dasNACK, wHigh))
					DeleteAtom (wHigh);
				}

			if (wLow != NULL &&
					(fMustFree || (dms.fRelease && (!dms.fAck || fSuccess))))
				GlobalFree (wLow);

			return fTrue;
			}

	case WM_DDE_ACK:
			{
			struct DAS das;
			bltb (&wLow, &das, cbDAS);
			DeleteAtom (wHigh);
			PdcldDcl (dcl)->fAck = das.fAck;
			PdcldDcl (dcl)->fBusy = das.fBusy;
			return fTrue;
			}

	default:
		/* message that shouldn't be sent to Macro */
		ReportDdeError ("Message shouldn't be sent to macro",
				dcl, message, 0);
		return fTrue;
		}

	Assert (fFalse);
	return fTrue;
}




/* E X C E L ' S   D R I V E . C */


/*----------------------------------------------------------------------------
|	ElActivateApp
|
|	Command equivalent function entry point for APP.ACTIVATE().
----------------------------------------------------------------------------*/
/* %%Function:ElActivateApp %%Owner:peterj */
EL ElActivateApp(pstApp, fImmediate)
CHAR **pstApp;
BOOL fImmediate;
{
	HWND hwnd;
	BYTE sz[258];

	hwnd=vhwndApp;
	StToSz(*pstApp, sz);

	if ((hwnd=FindWindow(NULL, sz))==NULL)
		{
		RtError(rerrNoSuchWin);
		return 0;
		}

	if (hwnd == vhwndApp && !vfDeactByOtherApp)
		return 0; /* success */

	if (!fImmediate && vfDeactByOtherApp)
		WaitActivation();

	if (!FActivateWindow(hwnd))
		RtError(rerrCantActivate);

	return 0;
}


/* %%Function:FActivateWindow %%Owner:peterj */
FActivateWindow(hwnd)
HWND hwnd;
{
	HWND hwndT;

	Assert(hwnd);
	if (FBadWindow(hwnd))
		{
		/* The main window is disabled or hidden, so look for a window
			owned by the main window which isn't disabled or hidden */
		for (hwndT=GetWindow(hwnd, GW_HWNDFIRST); hwndT;
				hwndT=GetWindow(hwndT, GW_HWNDNEXT))
			if (GetWindow(hwndT, GW_OWNER) == hwnd)
				{
				if (FBadWindow(hwndT))
					{
					hwnd=hwndT;
					hwndT=GetWindow(hwnd, GW_HWNDFIRST);
					}
				else
					break;
				}
		if (!hwndT)
			return fFalse;/* failure case */
		hwnd=hwndT;
		}
	SetActiveWindow(hwnd);
	return fTrue;

}


/* %%Function:FBadWindow %%Owner:peterj */
FBadWindow(hwnd)
{
	Assert(IsWindow(hwnd));
	return(!IsWindowEnabled(hwnd) || !IsWindowVisible(hwnd));
}


/*----------------------------------------------------------------------------
|	ElSendKeys
|
----------------------------------------------------------------------------*/
/* %%Function:ElSendKeys %%Owner:peterj */
EL ElSendKeys(pstKeys, fWait)
CHAR **pstKeys;
BOOL fWait;
{
	HANDLE hKeys = NULL;
	CHAR FAR *lpch;
	HEVT hevt = NULL;
	MSG msg;
	ENV *penvSav, env;

	if (!**pstKeys)
		/* no keys */
		return;

	penvSav = penvMem;
	if (SetJmp (penvMem = &env))
		{
		DdeDbgCommSz ("DoJmp'd to ElSendKeys");
LFail:
		if (hevt)
			GlobalFree(hevt);
		if (hKeys != NULL)
			GlobalFree(hKeys);
		penvMem = penvSav;
		RtError(rerrCannotSendKeys);
		return 0; /* failed */
		}

	if ((hKeys = OurGlobalAlloc(GMEM_MOVEABLE, (DWORD)**pstKeys+1)) == NULL
			|| (lpch = GlobalLock (hKeys)) == NULL)
		goto LFail;

	bltbx ((CHAR FAR *)*pstKeys + 1, lpch, **pstKeys);
	lpch [**pstKeys] = 0;
	GlobalUnlock (hKeys);

	if (!FParseKeys(hKeys, NULL, &hevt))
		goto LFail;

	AddKeys(&hevt);
	GlobalFree (hKeys);
	penvMem = penvSav;

	/* we can't wait when excel is active, it would screw everything up */
	if (fWait && vfDeactByOtherApp)
		{
		/* set a timer so that we get kicked out of GetMessage */
		SetTimer(vhwndApp, 69, 250, (LPSTR)NULL);
		while (vfDeactByOtherApp && lphevtHead!=NULL && *lphevtHead)
			{
			GetMessage((LPMSG)&msg, NULL, 0, 0);
			TranslateMessage((LPMSG)&msg);
			DispatchMessage((LPMSG)&msg);
			}
		KillTimer(vhwndApp, 69);
		}
	return 0;
}


/* %%Function:AddEvent %%Owner:peterj */
AddEvent(wm, vk, phevt)
int wm;
int vk;
HEVT *phevt;
{
	unsigned cb;
	HEVT hevt;
	EVT FAR *lpevt;
	extern ENV *penvMem;

	AssertPenv (penvMem);
	if ((hevt=*phevt)==NULL)
		{
		if ((hevt=OurGlobalAlloc(GMEM_SENDKEYS, (DWORD) sizeof(EVT)))==NULL)
			{
			SetErrorMat(matMem);
			DoJmp(penvMem, TRUE);
			}
		lpevt=GlobalLock(hevt);
		Assert(lpevt!=NULL);
		lpevt->ieventCur=0;
		lpevt->ieventMac=0;
		}
	else
		{
		lpevt=GlobalLock(hevt);
		Assert(lpevt!=NULL);
		cb = lpevt->ieventMac*sizeof(EVENT)+sizeof(EVT);
		GlobalUnlock(hevt);
		if (OurGlobalReAlloc(hevt, (long)cb, GMEM_SENDKEYS) == NULL)
			{
			SetErrorMat(matMem);
			GlobalFree(hevt);
			DoJmp(penvMem, TRUE);
			}
		lpevt=GlobalLock(hevt);
		Assert(lpevt!=NULL);
		}
	lpevt->rgevent[lpevt->ieventMac].wm = wm;
	lpevt->rgevent[lpevt->ieventMac].vk = vk;
	lpevt->ieventMac++;
	GlobalUnlock(hevt);
	*phevt = hevt;
}


/*----------------------------------------------------------------------------
|	FParseKeys
|
|	Parses the string in the supplied hpch and converts it into keycodes
|	and opcodes.
|	We build the virtual keys into the hevt structure.
|	If pichOp is non null we stuff in the ich of the begining and end of
|	each opcode into the rgichOp, otherwise opcodes are illegal.
|	
|	Returns:	a virtual key or 0 if there is an error.
----------------------------------------------------------------------------*/
/* %%Function:FParseKeys %%Owner:peterj */
FParseKeys(hKeys, pichOp, phevt)
HANDLE hKeys;
int *pichOp;   /* must be NULL or rgichOp [cichOpMax] */
HEVT *phevt;
{
	int vk;
	int ch;
	int ich = 0;
	int cGrp;
	int wRepeat;
	int cMul;
	BOOL fHaveShift, fHaveCtrl, fHaveAlt;
	int ichT, ichNew;
	int *pichOpMax = pichOp + cichOpMax;
	CHAR FAR *lpch;
	ENV env, *penvSav;

	Assert(phevt);

	cGrp=fHaveShift=fHaveCtrl=fHaveAlt=0;

	if ((lpch = GlobalLock(hKeys)) == NULL)
		return FALSE;

	penvSav = penvMem;
	if (SetJmp(penvMem = &env))
		{
		DdeDbgCommSz ("DoJmp'd to FParseKeys");
ParseErr:
		GlobalUnlock (hKeys);
		penvMem = penvSav;
		return FALSE;
		}

	for (;;)
		{
		ch = lpch [ich++];
		if (!ch)
			break;
		wRepeat=0;
		switch (IchEx(ch))
			{
		case ichExOpEnd:
		case ichExKeyEnd:
			/* if these appear at this point they are out of 
				context, so return an error */
			goto ParseErr;
		case ichExOpStart:
			if (!pichOp || pichOp > pichOpMax - 3)
				goto ParseErr;	/* operators not allowed */
			/* Note pichOp[0] is ichFirstOp and pichOp[1] is
				ichLimOp where op excludes [ and ].  This may
				be different than Excel.
			*/
			*pichOp++ = ich;
			FindMatch(lpch, stEx[ichExOpEnd], &ich, TRUE);
			*pichOp++ = ich++;
			break;
		case ichExKeyStart:
			ichT = ich;
			FindMatch(lpch, stEx[ichExKeyEnd], &ichT, FALSE);
			cMul=1;
			ichNew = ichT + 1;
			if (FDigit(lpch[ichT-1]))
				{
				/* convert trailing digits to repeat count */
				while (FDigit(lpch[--ichT]))
					{
					wRepeat += (lpch[ichT]-'0')*cMul;
					cMul *= 10;
					}
				/* make sure it isn't "{f10}" or something */
				if (lpch[ichT] != ' ')
					{
					if (ichT < ich)
						{
						/* the {123} case */
						vk = wRepeat;
						wRepeat = 0;
						ich = ichNew;
						goto GotVk;
						}
					ichT = ichNew-1;
					wRepeat = 0;
					}
				}
			Assert(ichT>=ich);
			if (ichT==ich)
				{
				/* this is only legal in the "{}}" case */
				ch = stEx[ichExKeyEnd];
				if (!lpch[++ichT] || lpch[ichT] != ch)
					goto ParseErr;
				ich = ichNew+1;
				goto GotKey;
				}
			if (ichT == ich+1)
				{  /* {+} case */
				ch = lpch[ich];
				ich = ichNew;
				goto GotKey;
				}
			if ((vk=VkMatchKeyword(lpch, ich, ichT))<0)
				goto ParseErr;	/* no matching keyword */
			ich = ichNew;
			goto GotVk;
		case ichExShift:
			if (fHaveShift)
				goto ParseErr;
			AddEvent(WM_KEYDOWN, VK_SHIFT, phevt);
			fHaveShift = 10;
			break;
		case ichExCtrl:
			if (fHaveCtrl)
				goto ParseErr;
			AddEvent(WM_KEYDOWN, VK_CONTROL, phevt);
			fHaveCtrl = 10;
			break;
		case ichExAlt:
			if (fHaveAlt)
				goto ParseErr;
			AddEvent(WM_SYSKEYDOWN, VK_MENU, phevt);
			fHaveAlt = 10;
			break;
		case ichExGrpStart:
			/* convert all immediate mode states to group mode */
			cGrp++;
			if (cGrp>3)
				goto ParseErr;
			if (fHaveShift == 10)
				fHaveShift = cGrp;
			if (fHaveCtrl == 10)
				fHaveCtrl = cGrp;
			if (fHaveAlt == 10)
				fHaveAlt = cGrp;
			break;
		case ichExGrpEnd:
			if (cGrp<=0)
				goto ParseErr;
			/* cancel all modifiers in the current group */
			CancelMods(&fHaveShift, &fHaveCtrl, &fHaveAlt,
					cGrp, phevt);
			cGrp--;
			break;
		case ichExEnter:
			vk = VK_RETURN;
			goto GotVk;
		case ichSpace:
			vk = VK_SPACE;
			goto GotVk;
		case ichTab:
			vk = VK_TAB;
			goto GotVk;
		default:
GotKey:
			/* NOTE: Opus does not have VkFromCh() */
			if (ch>='A' && ch<='Z')
				{
				/* these have an implied shift */
				if (!fHaveShift)
					{
					AddEvent(WM_KEYDOWN,
							VK_SHIFT, phevt);
					fHaveShift=10;
					}
				vk = ch;
				}
			else  if (ch>='a' && ch<='z')
				vk = ChUpper(ch);
			else  if (ch>='0' && ch<='9')
				vk = ch;
				/* handle special formatting cases of EditSearch dlg */
			else  if (fHaveCtrl && (ch == '=' || ch == '+'))
				{
				extern int vkPlus;
				vk = vkPlus;
				if (ch == '+' && !fHaveShift)
					{
					AddEvent(WM_KEYDOWN,
							VK_SHIFT, phevt);
					fHaveShift=10;
					}
				}
			else
				{
				/* these must be passed as WM_CHARS */
				do
					AddEvent(WM_CHAR, ch, phevt);
				while (--wRepeat>0);
				goto Cancel;
				}
GotVk:
			do
				{
				AddEvent((fHaveAlt)?WM_SYSKEYDOWN:WM_KEYDOWN,
						vk, phevt);
				AddEvent((fHaveAlt)?WM_SYSKEYUP:WM_KEYUP,
						vk, phevt);
				}
			while (--wRepeat>0);
Cancel:
			CancelMods(&fHaveShift, &fHaveCtrl, &fHaveAlt,
					10, phevt);
			break;
			}
		}
	if (cGrp)
		goto ParseErr;
	if (pichOp)
		{
		*pichOp = 0;	/* terminate operator string */
		Assert (pichOp < pichOpMax);
		}
	CancelMods(&fHaveShift, &fHaveCtrl, &fHaveAlt, 10, phevt);
	Assert(!fHaveShift && !fHaveCtrl && !fHaveAlt);
	GlobalUnlock (hKeys);
	penvMem = penvSav;
	return(TRUE);
}


/* %%Function:FindMatch %%Owner:peterj */
FindMatch(lpch, ch, pich, fOp)
CHAR FAR *lpch;
int ch;
int *pich;
BOOL fOp;
{
	CHAR chT;
	int ich;

	AssertPenv (penvMem);

	for (ich = *pich; (chT = lpch [ich]) != ch; ich++)
		{
		if (!chT)
			DoJmp (penvMem, TRUE);
		if (fOp &&  chT == '"')
			for (ich++; (chT = lpch[ich]) != '"'; ich++)
				if (!chT)
					DoJmp (penvMem, TRUE);
		}
	*pich = ich;
}


/* returns the vk to a keyword if it finds it in the grstKeyword */
/* %%Function:VkMatchKeyword %%Owner:peterj */
VkMatchKeyword(lpch, ich, ichMac)
CHAR FAR *lpch;
int ich, ichMac;

{
	int cch;
	CHAR sz [cchMaxSz];

	if ((cch = ichMac - ich) >= cchMaxSz)
		return idNil;

	bltbx (lpch + ich, (CHAR FAR *)sz, cch);
	sz [cch] = 0;

	return IdFromSzgSz (szgParseKeys, sz);
}


/* %%Function:IchEx %%Owner:peterj */
IchEx(ch)
int ch;
{
	int ich;

	for (ich=stEx[0]; ich>0; ich--)
		if (stEx[ich] == ch)
			return(ich);
	return(0);
}


/* cancel all modifiers which match fMatch */
/* %%Function:CancelMods %%Owner:peterj */
CancelMods(pfShift, pfCtrl, pfAlt, fMatch, phevt)
BOOL *pfShift;
BOOL *pfCtrl;
BOOL *pfAlt;
BOOL fMatch;
HEVT *phevt;
{
	if (*pfShift == fMatch)
		{
		AddEvent(WM_KEYUP, VK_SHIFT, phevt);
		*pfShift = 0;
		}
	if (*pfCtrl == fMatch)
		{
		AddEvent(WM_KEYUP, VK_CONTROL, phevt);
		*pfCtrl = 0;
		}
	if (*pfAlt == fMatch)
		{
		AddEvent(WM_SYSKEYUP, VK_MENU, phevt);
		*pfAlt = 0;
		}
}


/* %%Function:HevtCompactHevt %%Owner:peterj */
HEVT HevtCompactHevt(hevt)
HEVT hevt;
{
	int cevent;
	EVT FAR *lpevt;
	extern ENV *penvMem;

	AssertPenv (penvMem);
	if (!hevt)
		return(hevt);
	lpevt=GlobalLock(hevt);
	Assert(lpevt!=NULL);
	if (lpevt->ieventCur == 0)
		goto Done;
	cevent = lpevt->ieventMac-lpevt->ieventCur;
	Assert(cevent>0);
	bltbx(&lpevt->rgevent[lpevt->ieventCur], &lpevt->rgevent[0],
			cevent*sizeof(EVENT));
	GlobalUnlock(hevt);
	if (OurGlobalReAlloc(hevt, (long)(uns)(sizeof(EVT)+(cevent-1)*sizeof(EVENT)), GMEM_SENDKEYS) == NULL)
		DoJmp(penvMem, TRUE);
	lpevt=GlobalLock(hevt);
	Assert(lpevt!=NULL);
	lpevt->ieventMac -= lpevt->ieventCur;
	lpevt->ieventCur=0;
Done:
	GlobalUnlock(hevt);
	return(hevt);
}


/* %%Function:AddKeys %%Owner:peterj */
AddKeys(phevt)
HEVT *phevt;
{
	int cevent;
	EVT FAR *lpevt;
	EVT FAR *lpevtHead;
	BYTE FAR *lpbKeyState;
	DRVHD FAR *lpdrvhd;
	ENV *penvMemSav, env;
	BYTE rgb[256];
	HANDLE hCode, hData = NULL;
	LONG FAR PASCAL PlaybackHook();
	void FAR PASCAL lpfnPlaybackHookSave();
	void FAR PASCAL hevtHead();
	void FAR PASCAL hrgbKeyState();
	extern ENV *penvMem;

	AssertPenv (penvMem);
	if (!*phevt)
		return;
	penvMemSav = penvMem;
	if (SetJmp(penvMem = &env))
		{
		DdeDbgCommSz ("DoJmp'd to AddKeys");
		/* OOM */
OOM:
		/* According to PeterJ AddKeys cannot be called
		   recursively, so hData can be kept in a local
		   and freed here. */
		if (hPlaybackHook)
			RemovePlaybackHook ();
		if (hData)
			GlobalFree(hData);
		SetErrorMat(matMem);
		GlobalFree(*phevt);
		*phevt = NULL;
		DoJmp(penvMem = penvMemSav, TRUE);
		}
	if (lphevtHead != NULL && *lphevtHead)
		{
		*lphevtHead = HevtCompactHevt(*lphevtHead);
		Assert(*lphevtHead);
		lpevtHead = GlobalLock(*lphevtHead);
		lpevt = GlobalLock(*phevt);
		Assert(lpevtHead != NULL);
		Assert(lpevt != NULL);
		cevent = lpevt->ieventMac+lpevtHead->ieventMac;
		Assert(lpevtHead->ieventCur == 0);
		GlobalUnlock(*phevt);
		GlobalUnlock(*lphevtHead);
		if (OurGlobalReAlloc(*lphevtHead,
				(sizeof(EVT)+(long)(cevent-1)*sizeof(EVENT)),GMEM_SENDKEYS) == NULL)
			DoJmp(penvMem, TRUE);
		lpevtHead = GlobalLock(*lphevtHead);
		lpevt = GlobalLock(*phevt);
		bltbx(&lpevt->rgevent[0],
				&lpevtHead->rgevent[lpevtHead->ieventMac],
				lpevt->ieventMac*sizeof(EVENT));
		lpevtHead->ieventMac = cevent;
		GlobalUnlock(*phevt);
		GlobalUnlock(*lphevtHead);
		penvMem = penvMemSav;
		GlobalFree(*phevt);
		*phevt = NULL;
		}
	else
		{
		/* install playback hook */
		if (hPlaybackHook == NULL)
			{
			hCode = GetCodeHandle(PlaybackHook);
			Assert(hCode);
			if ((hData=OurGlobalAlloc(GMEM_FIXED|GMEM_LOWER, (long)sizeof(struct DRVDATA)))==NULL)
				goto OOM;
			if ((hPlaybackHook=OurGlobalAlloc(GMEM_FIXED|GMEM_LOWER, GlobalSize(hCode)))==NULL)
				goto OOM;
			lpdrvhd = (DRVHD FAR *)GlobalLock(hPlaybackHook);
			bltbx(GlobalLock(hCode), lpdrvhd, GlobalSize(hCode));
			GlobalUnlock(hCode);
			/* create FAR pointers to vars in the hook segment */
			Assert(offset(DRVDATA, hevtHead) == 0);
			lphevtHead=(struct DRVDATA FAR *)GlobalLock(hData);
			lphrgbKeyState=&(((struct DRVDATA FAR *)lphevtHead)->hrgbKeyState);
			lplpfnPlaybackHookSave=&(((struct DRVDATA FAR *)lphevtHead)->lpfnPlaybackHookSave);
			lpdrvhd->wDataSeg = HIWORD(lphevtHead);
			lpdrvhd->wWinVersion = vwWinVersion;
			if (vwWinVersion >= 0x0300)
				/* under win3.0 we have to change the data
				   segment containing PlaybackHook to code. */
				{
				HANDLE hKernel=GetModuleHandle(SzShared("KERNEL"));
				FARPROC lpfn;
				HANDLE hTemp;

				Assert(hKernel != NULL);
				if (hKernel == NULL)
					goto OOM;
				lpfn = GetProcAddress(hKernel,
						MAKEINTRESOURCE(idoAllocSelector));
				Assert(lpfn != NULL);
				hTemp = (*lpfn)(hPlaybackHook);
				if (hTemp == NULL)
					goto OOM;
				lpfn = GetProcAddress(hKernel,
						MAKEINTRESOURCE(idoPrestoChangoSelector));
				Assert(lpfn != NULL);
				(*lpfn)(hTemp, hPlaybackHook);
				lpfn = GetProcAddress(hKernel,
						MAKEINTRESOURCE(idoFreeSelector));
				Assert(lpfn != NULL);
				(*lpfn)(hTemp);
				}
			lpfnPlaybackHook=&lpdrvhd->fnPlaybackHook;
			/* don't need to unlock hPlaybackHook, it's fixed */
			}

		Assert(lphevtHead != NULL);
		*lphevtHead = *phevt;
		/* save and reset key state */
		if ((*lphrgbKeyState=OurGlobalAlloc(GMEM_SENDKEYS, (DWORD) 256)) != NULL)
			{
			if ((lpbKeyState = GlobalLock(*lphrgbKeyState)) == (BYTE FAR *)NULL)
				{
				GlobalFree(*lphrgbKeyState);
				*lphrgbKeyState = NULL;
				}
			else
				{
				/* preserve old key state */
				GetKeyboardState(lpbKeyState);
				GlobalUnlock(*lphrgbKeyState);
				/* reset key state */
				SetBytes (rgb, 0, 256);
				SetKeyboardState((BYTE FAR *)rgb);
				}
			}
		*lplpfnPlaybackHookSave=
				SetWindowsHook(WH_JOURNALPLAYBACK, lpfnPlaybackHook);
		}
	penvMem = penvMemSav;
}


HQ hqrgbDDETokens = hqNil;
int cbDDETokens = 0;

/* F  E X E C U T E  H  C O M M A N D S */
/*  Execute the commands passed to us by DDE_EXECUTE message.  Return fTrue
	if commands executed, else fFalse.  Do not free hCommands.
*/

/* %%Function:FExecuteHCommands %%Owner:peterj */
FExecuteHCommands (hCmds)
HANDLE hCmds;

{
	CP cp;
	HEVT hevt = NULL;
	BOOL fSuccess = fFalse;
	int rgichOp [cichOpMax];
	ENV *penvSav, env;
	RERR rerr;

	extern BOOL vfMcrRunning;
	extern int vdocTemp;

	DdeDbgCommSz ("FExecuteHCommands: entered");

	penvSav = penvMem;
	if (SetJmp (penvMem = &env))
		{
		DdeDbgCommSz ("DoJmp'd to FExecuteHCommands");
		goto LDone;
		}

	if (!FParseKeys (hCmds, rgichOp, &hevt))
		goto LDone;

/*  rgichOp is an array of pairs of ints which point into hCmds:
**                  [Foo ()]
**           ich[2i] ^     ^ ich[2i+1]
**  Each pair is an ichFirst, ichLim of a portion of a macro.
*/
	/* BLOCK - create and run a macro */
		{
		CHAR FAR *lpch;
		CHAR szCmd [cchMaxSz];
		int *pich, cch;
		struct CHP chp;
		struct PAP pap;

		if ((lpch = GlobalLockClip (hCmds)) == NULL)
			goto LDone;

		cp = 0;

		if (DocCreateTemp(docScrap) == docNil) /* mother doc irrelevant */
			goto LUnlockDone;

		Assert(vdocTemp != docNil);
		StandardChp(&chp);
		StandardPap(&pap);
		if (!FInsertRgch(vdocTemp, cp, SzShared("Sub MAIN"), 8, &chp, 0))
			goto LUnlockDone;
		cp += 8;
		if (!FInsertRgch(vdocTemp, cp, rgchEop, (int) ccpEop, &chp, &pap))
			goto LUnlockDone;
		cp += ccpEop;

		for (pich = rgichOp; *pich; pich += 2)
			{
			if ((cch = *(pich+1)-(*pich)) >= cchMaxSz)
				{
LUnlockDone:
				GlobalUnlock (hCmds);
				goto LDone;
				}
			bltbx (lpch + *pich, (CHAR FAR *)szCmd, cch);
			szCmd [cch] = 0;
#ifdef DEBUG
			CommSzSz (SzShared("Ex: "), szCmd);
#endif /* DEBUG */

			if (!FInsertRgch(vdocTemp, cp, szCmd, cch, &chp, 0))
				goto LUnlockDone;
			cp += cch;
			if (!FInsertRgch(vdocTemp, cp, rgchEop, (int) ccpEop, &chp, &pap))
				goto LUnlockDone;
			cp += ccpEop;
			}
		if (!FInsertRgch(vdocTemp, cp, SzShared("End Sub"), 7, &chp, 0))
			goto LUnlockDone;
		cp += 7;

		GlobalUnlock (hCmds);
		}


		/* BLOCK - run the macro */
		{
		extern WORD CchReadDDESource();
		extern int ElaDebug();
		extern int GetInfoElx();
		extern ELI ** HeliNew();
		extern BOOL vfElDisableInput;
		extern BOOL vfElFunc;
		extern BOOL vcElParams;
		extern MES ** vhmes;

		ELI ** heli;
		struct CA ca;

		if (!FTokenizePca(PcaSet(&ca, vdocTemp, cp0, cp),
				&hqrgbDDETokens, &cbDDETokens))
			{
			goto LDone;
			}

		if (vhmes != hNil && (*vhmes)->fCanCont && FSuspended(vhmes))
			FreeEditMacro(iNil);

		if (!FAllocMacroSbs())
			goto LFreeTokDone;

		StartLongOp();

		if ((heli = HeliNew(0, 0, CchReadDDESource, ElaDebug, GetInfoElx, 
				0, 0L)) == hNil)
			{
			FreeMacroSbs();
			EndLongOp(fFalse);
LFreeTokDone:
			FreeHq(hqrgbDDETokens);
			goto LDone;
			}

		vfElDisableInput = fFalse;
		vfMcrRunning = fTrue;
		rerr = RerrRunHeli(heli);
		vfMcrRunning = fFalse;
		FMacroErrMsg(heli, rerr);

		TermMacro(heli);
		FreeMacroSbs();

		EndLongOp(fFalse);
		}

	/* if there were any keystrokes in the string, feed them to opus */
	if (hevt)
		{
		if (vfDeactByOtherApp)
			WaitActivation ();
		AddKeys (&hevt);
		hevt = NULL;   /* freed by the keyhook */
		}

	fSuccess = fTrue;
LDone:
#ifdef DEBUG
	if (fSuccess)
		DdeRpt2(DdeDbgCommSz ("FExecuteHCommands: succeeded"));
	else
		DdeRpt2(DdeDbgCommSz ("FExecuteHCommands: failed"));
#endif /* DEBUG */

	if (hevt != NULL)
		GlobalFree (hevt);
	penvMem = penvSav;
	return fSuccess;
}


/* %%Function:CchReadDDESource %%Owner:peterj */
WORD CchReadDDESource(pb, cbReq, heli)
char * pb;
WORD cbReq;
ELI ** heli;
{
	extern SB sbTds;

	char far * lp;
	long lib;
	ELI huge * hpeli;

	Assert(hqrgbDDETokens != hqNil);

	hpeli = HpeliOfHeli(heli);
	lib = hpeli->lib;
	if (lib + cbReq > cbDDETokens)
		cbReq = cbDDETokens - lib;
	lp = (char far *) LpFromHq(hqrgbDDETokens) + lib;
	bltbx(lp, (char far *) pb, cbReq);

	return cbReq;
}
