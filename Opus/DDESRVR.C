/* D D E S R V R . C */
/*  Routines for server functionality of DDE */


#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "dde.h"
#include "doc.h"
#include "file.h"
#include "strtbl.h"
#include "ch.h"
#include "doslib.h"
#include "idle.h"

#include "debug.h"

#ifdef PROTOTYPE
#include "ddesrvr.cpt"
#endif /* PROTOTYPE */

extern BOOL            fElActive;
extern struct DCLD  ** mpdclhdcld[];
extern struct DOD   ** mpdochdod[];
extern BOOL            vfDdeIdle;
extern struct DDES     vddes;
extern BOOL            vfDeactByOtherApp;
extern int             docMac;
extern int             cfLink;
extern struct SAB      vsab;
extern struct MERR     vmerr;
extern int             docMac;
extern BOOL            vfInsertMode;
extern BOOL            vfAbortInsert;
extern IDF	       vidf;

csconst CHAR stAppDde[] = St("WinWord");
csconst CHAR stSystemDde[] = St("System");

WCompSt ();

#define cchDdeLinkBkmkBase 9   /* includes terminator */

/* F  S E R V E R  D D E  M S G */
/*  Handle messages that are destined for a channel on which we are
	the server.  Return fFalse if we are not prepared to handle now (message
	will come back next time through).
*/
/*  %%Function:FServerDdeMsg %%Owner:peterj  */
FServerDdeMsg (dcl, message, wLow, wHigh)
int dcl;
unsigned message;
int wLow, wHigh;

{
	BOOL fTerminating = PdcldDcl (dcl)->fTerminating;
	struct DDLI ddli;

	DdeDbgCommMsg ("Server", dcl, message, wLow, wHigh);

	if (vfInsertMode)
		/* can't safely do anything in insert mode */
		{
		vfAbortInsert = fTrue;
		return fFalse;
		}

	switch (message)
		{

	case WM_DDE_EXECUTE:
			/*  wLow  : unused
				wHigh : hCommands
			*/
			{
			int das;
			BOOL f;

			if (fTerminating || PdcldDcl(dcl)->fExecuting || fElActive)
				goto LExeNACK;

			PdcldDcl(dcl)->fExecuting = fTrue;
			f = FExecuteHCommands(wHigh);
			PdcldDcl(dcl)->fExecuting = fFalse;

			if (!f)
				{
LExeNACK:
				das = dasNACK;
				}
			else
				das = dasACK;

			if (!FPostDdeDcl (dcl, WM_DDE_ACK, das, wHigh))
				{
				GlobalFree (wHigh);

				if (PdcldDcl(dcl)->fTermReceived && !PdcldDcl(dcl)->fExecuting)
					/* channel destroyed in course of EXECUTE */
					DestroyDcl(dcl);
				}
			break;
			}

	case WM_DDE_POKE:
			/*  wLow  : hData
				wHigh : atomItem
			*/
			{
			struct DMS FAR * lpdms, dms;
			int doc = PdcldDcl (dcl)->doc;
			int das = dasNACK;
			int ibkmk;

			if (fTerminating)
				/*  not much point in receiving while terminating */
				{
				goto LNoAckPoke;
				}

			if (fElActive)
				/* can't poke while a macro is running */
				{
				goto LNackPoke;
				}
			if ((lpdms = GlobalLockClip (wLow)) == NULL)
				/*  couldn't lock object, should be rare */
				{
				/* incoming data--could easily not be lockable */
				ShrinkSwapArea();
				lpdms = GlobalLockClip (wLow);
				GrowSwapArea();
				if (lpdms == NULL)
					/* still can't lock it */
					{
					ReportDdeError ("Cannot lock handle, dropping message",
							dcl, message, 0);
					break;
					}
				}
			dms = *lpdms;
			GlobalUnlock (wLow);
			if (doc != docSystem && 
					(ibkmk = IbkmkFromDclItem (dcl, wHigh)) != ibkmkNil)
				/*  we have some place to put the data */
				{
				CP ccp;
				CHAR stBkmk [cchBkmkMax];
				struct CA ca;

				PcaSet( &ca, doc, cp0, CpMacDocEdit(doc) );

				if (ibkmk < ibkmkSpecialMin)
					{
					/* we must reconstruct bkmk later, get info */
					BkmkCpsFromIbkf(ca.doc, ibkmk, &ca.cpFirst, &ca.cpLim);
					ca.cpLim = CpMin (CpMacDocEdit (ca.doc), ca.cpLim);
					GetStFromSttb(PdodDoc(doc)->hsttbBkmk, ibkmk, stBkmk);
					Assert (FLegalBkmkName (stBkmk) && *stBkmk < cchBkmkMax);
					}
				else
					FBkmkCpsIbkmkSpecial (doc, ibkmk, &ca.cpFirst, &ca.cpLim);

				AssureLegalSel (&ca);

				/* put the data in doc where the old bkmk was */
				if (FReadHData (wLow, dms.cf, &ca, cbDMS ))
					{
					das = dasACK;
					if (ibkmk < ibkmkSpecialMin)
						{
						/* put the bookmark back in place */
						AssureLegalSel (&ca);
						/* we can assert this because bookmark already exists */
						AssertDo (FInsertStBkmk (&ca, stBkmk, NULL));
						}

					/* if we are not active, update the display */
					if (vfDeactByOtherApp && !vidf.fNotLive)
						UpdateAllWws (fTrue);

					SetUndoNil();
					InvalAgain();
					}

				}
LNackPoke:
			if (!FPostDdeDcl (dcl, WM_DDE_ACK, das, wHigh))
				/*  unable to post ACK */
				{
LNoAckPoke:
				DeleteAtom (wHigh);
				GlobalFree (wLow);
				}
			else  if (das == dasACK && dms.fRelease)
				GlobalFree (wLow);
			break;
			}


	case WM_DDE_ACK:
			/*  wLow  : das
				wHigh : atomItem
			*/
			/* must be in response to some WM_DDE_DATA message */
			{
			int iddli = IddliFromDclItem (dcl, wHigh);

			struct DAS das;
			bltb (&wLow, &das, sizeof (das));

			FreezeHp ();
			if (iddli != iddliNil)
				{
				GetPl( vddes.hplddli, iddli, &ddli );

				if (!das.fAck && ddli.dls == dlsWait)
					{
					/* free the data */
					Assert (ddli.hData != NULL);
					GlobalFree (ddli.hData);
					ddli.hData = NULL;
					}
				if (!das.fAck && das.fBusy)
					/* they were busy, cause us to resend */
					ddli.fDirty = fTrue;

				ddli.dls = dlsClear;
				PutPl( vddes.hplddli, iddli, &ddli );
				}
			DeleteAtom (wHigh);
			MeltHp ();
			break;
			}

	case WM_DDE_REQUEST:
			/*  wLow  : cf
				wHigh : atomItem
			*/
			{
			int doc = PdcldDcl (dcl)->doc;

			if (fTerminating || 
					!FRenderDocItemCf (doc, wHigh, wLow) ||
					!FSendData (dcl, doc, wHigh, wLow, iddliNil))
				if (!FPostDdeDcl (dcl, WM_DDE_ACK, dasNACK, wHigh))
					DeleteAtom (wHigh);
			break;
			}

	case WM_DDE_ADVISE:
			/*  wLow  : hOptions
				wHigh : atomItem
			*/
			{
			struct DMS FAR * lpdms, dms;
			int doc = PdcldDcl (dcl)->doc;
			int das = dasNACK;

			/*  get the options */
			if ((lpdms = GlobalLockClip (wLow)) == NULL)
				{
				ReportDdeError ("Cannot lock handle, dropping message", 
						dcl, message, 0);
				return fTrue;
				}
			dms = *lpdms;
			GlobalUnlock (wLow);

			/*  if item exists and we can render in cf, set up advise */
			/*  (we do not support an ADVISE on the system topic) */
			if (!fTerminating && doc != docSystem &&
					FRenderDocItemCf (doc, wHigh, dms.cf) &&
					FAdviseItem (dcl, wHigh, dms.cf, dms.fAck, dms.fNoData))
				{
				GlobalFree (wLow);
				das = dasACK;
				}

			/*  ACK or NACK as appropriate */
			if (!FPostDdeDcl (dcl, WM_DDE_ACK, das, wHigh))
				{
				DeleteAtom (wHigh);
				if (das == dasNACK)
					GlobalFree (wLow);
				}
			break;
			}

	case WM_DDE_UNADVISE:
			/*  wLow  : unused
				wHigh : atomItem
			*/
			{
			int iddli;
			int das = dasACK;

			if (wHigh != NULL && 
					(iddli = IddliFromDclItem (dcl, wHigh)) != iddliNil)
				{
				GetPl( vddes.hplddli, iddli, &ddli );
				if (ddli.dls == dlsClear)
					{
					DeleteDdliServer (iddli);
					}
				else
					/* we are waiting on an ack, cannot delete */
					das = dasBUSY;
				}

			else  if (wHigh == NULL)
				{
				int iddliMac = IMacPl( vddes.hplddli );

				/*  determine if it is possible first */
				for (iddli = 0; iddli < iddliMac; iddli++)
					{
					GetPl( vddes.hplddli, iddli, &ddli );
					if (ddli.dcl == dcl && ddli.dls != dlsClear)
						/*  can't UNADVISE (waiting on ACK) */
						{
						das = dasBUSY;
						break;
						}
					}
				if (das == dasACK)
					/* no reason not to unadvise found */
					{
					for (iddli = iddliMac-1; iddli >= 0; iddli--)
						{
						GetPl( vddes.hplddli, iddli, &ddli );
						if (ddli.dcl == dcl)
							DeleteDdliServer (iddli);
						}
					}
				}
			else
				/* no advise is set up to delete! */
				{
				ReportDdeError ("Unknown link for UNADVISE", dcl, message,
						0);
				das = dasNACK;
				}

			if (!FPostDdeDcl (dcl, WM_DDE_ACK, das, wHigh))
				DeleteAtom (wHigh);

			break;
			}

#ifdef DEBUG
	default:
		Assert (fFalse);
#endif /* DEBUG */

		}

	return fTrue;
}



/* I N I T I A T E  D D E */
/*  Called by our AppWndProc when an INITIATE message is received.
	Decides if we can serve the requested conversation and, if so,
	creates the necessary channels.
*/
/*  %%Function:InitiateDde %%Owner:peterj  */
InitiateDde (hwndThem, atomApp, atomTopic)
HWND hwndThem;
ATOM atomApp, atomTopic;

{
	if (vidf.fDead || vmerr.fDiskEmerg || vmerr.hrgwEmerg1 == hNil
			|| vmerr.hrgwEmerg2 == hNil || vmerr.hrgwEmerg3 == hNil ||
			(atomApp != NULL && atomApp != AtomFromStNA (StNear(stAppDde))))
		/*  initiate is not directed to us or we are not accepting links */
		{
		DdeDbgCommAtom ("InitiateDde: initiate refused", dclNil, atomApp);
		return;
		}

	DdeDbgCommAtom ("InitiateDde", dclNil, atomTopic);

	if (atomTopic == NULL)
		/*  catalog all topics available */
		{
		int doc;
		CreateServerChannel (docSystem, hwndThem); /* system */
		for (doc = docMinNormal; doc < docMac; doc++)
			if (mpdochdod [doc] != hNil && 
					PdodDoc (doc)->fMother && WwDisp(doc,wwNil,fFalse) != wwNil)
				CreateServerChannel (doc, hwndThem);
		}

	else  if (atomTopic == AtomFromStNA (StNear(stSystemDde)))
		/*  system topic */
		CreateServerChannel (docSystem, hwndThem);

	else
		/*  specific topic */
		{
		int doc;
		CHAR stTopic [cchMaxFile];
		CHAR stNorm [cchMaxFile];
		StFromAtom (stTopic, cchMaxFile, atomTopic);
		if ((doc = DocFromSt (stTopic)) != docNil ||
				(FNormalizeStFile (stTopic, stNorm, nfoNormal) && 
				(doc = DocFromSt (stNorm)) != docNil))
			CreateServerChannel (doc, hwndThem);
		}
}


/* C R E A T E  S E R V E R  C H A N N E L */
/*  create a channel over which we are the server and the topic is doc.
*/
/*  %%Function:CreateServerChannel %%Owner:peterj  */
CreateServerChannel (doc, hwndThem)
int doc;
HWND hwndThem;

{
	int dcl;
	ATOM atomApp, atomTopic;
	struct DCLD *pdcld;
	CHAR st[ichMaxFile];

	/*  get an st for the topic */
	if (doc == docSystem)
		CopyCsSt(stSystemDde, st);
	else
		GetDocSt(doc, st, gdsoFullPath);

	/*  add the atoms in our response */
	Assert (st[0] != 0);
	atomTopic = AtomAddSt (st); /* failure checked by fMemFail below */
	atomApp = AtomAddSt (StNear(stAppDde));

	if (vmerr.fMemFail || (dcl = DclCreateChannel (dtServer)) == dclNil)
		{
		DeleteAtom(atomTopic);
		DeleteAtom(atomApp);
		return; /* OOM - channel not created */
		}

	pdcld = PdcldDcl (dcl);
	pdcld->hwndThem = hwndThem;
	pdcld->doc = doc;

	DdeDbgCommAtom ("CreateServerChannel", dcl, atomTopic);

	/*  send response */
	SendMessage (hwndThem, WM_DDE_ACK, pdcld->hwndUs, 
			MAKELONG (atomApp, atomTopic));
}


/* H  R E N D E R  C L I P  L I N K */
/*  Render doc, cpFirst:cpLim in "LINK" format.  Return a handle to an
	object of the form: "APPNAME\0TOPICNAME\0ITEMNAME\0\0", or NULL in case
	of some failure.
*/
/*  %%Function:HRenderClipLink %%Owner:peterj  */
HRenderClipLink ()

{
	LPCH lpch;
	HANDLE hData;
	int cch, cchT;
	CHAR szApp [cchMaxSz];
	CHAR szTopic [cchMaxFile];
	CHAR szBkmk [cchBkmkMax];

	DdeDbgCommSz ("HRenderClipLink");

	Assert (vsab.fOwnClipboard);
	Assert (FCanLinkScrap());

	StToSz (StNear(stAppDde), szApp);
	GetDocSt(vsab.doc, szTopic, gdsoFullPath);
	StToSzInPlace(szTopic);

	if (!FGetSzBkmkForCps (&vsab.caCopy, szBkmk))
		{
		DdeDbgCommSz ("HRenderClipLink: no bookmark to render");
		return NULL;
		}

	cch = 0;
	if ((hData = OurGlobalAlloc(GMEM_MOVEABLE, (DWORD)
			(CchSz(szApp)+CchSz(szTopic)+CchSz(szBkmk)+1))) == NULL ||
			(lpch = GlobalLockClip (hData)) == NULL)
		{
		if (hData != NULL)
			GlobalFree (hData);
		DdeDbgCommSz ("HRenderClipLink: render unsuccessful");
		return NULL;
		}

	bltbx ((LPSTR)szApp, (LPSTR)lpch+cch, cchT = CchSz (szApp));
	cch += cchT;
	bltbx ((LPSTR)szTopic, (LPSTR)lpch+cch, cchT = CchSz (szTopic));
	cch += cchT;
	bltbx ((LPSTR)szBkmk, (LPSTR)lpch+cch, cchT = CchSz (szBkmk));
	cch += cchT;
	lpch [cch] = '\0';

	GlobalUnlock (hData);

	DdeDbgCommSz ("HRenderClipLink: render successful");

	return hData;
}



/* F  G E T  S Z  B K M K  F O R  C P S */
/*  Called to render a link.  Creates a unique bookmark name for
	a range of text (that was copied to clipboard at some point before)
	and defines the bookmark.
*/
/*  %%Function:FGetSzBkmkForCps %%Owner:peterj  */
FGetSzBkmkForCps (pca, sz)
struct CA *pca;
CHAR *sz;

{
	int wBkmk = 1;
	CHAR st [cchBkmkMax];

	bltb (StSharedKey("DDE_LINK",DdeLinkBkmk), st, cchDdeLinkBkmkBase);

	/* find the next unused dde link bkmk name */
	while (FSearchBookmark (pca->doc, st, NULL, NULL, NULL, bmcUser))
		{
		CHAR *pch = st + cchDdeLinkBkmkBase;
		*st = cchDdeLinkBkmkBase - 1 + CchIntToPpch (++wBkmk, &pch);
		}

	Assert (FLegalBkmkName (st));
	if (!FInsertStBkmk (pca, st, NULL))
		return fFalse;

	StToSz (st, sz);
	return fTrue;
}



/* V A L I D A T E  S E R V E R  L I N K S */
/*  Go through all active links we serve and for each one whose ibkmk
	is ibkmkNil (indicates the bookmark was deleted at some point) try to
	find a bookmark based on atomItem.
*/
/*  %%Function:FValidateServerLinks %%Owner:peterj  */
ValidateServerLinks ()
{
	int iddli;
	struct PLDDLI **hplddli = vddes.hplddli;
	struct DDLI ddli;

	vddes.fInvalidDdli = fFalse;

	if (hplddli == hNil)
		return;

	DdeDbgCommSz ("ValidateServerLinks");

	FreezeHp ();

	for ( iddli = 0; iddli < IMacPl( hplddli ); iddli++ )
		{
		GetPl( hplddli, iddli, &ddli );
		if (ddli.ibkf == ibkmkNil)
			{
			int ibkmk = IbkmkFromDclItem (ddli.dcl, ddli.atomItem);
			if (ibkmk != ibkmkNil)
				/*  bookmark now exists, use it */
				{
				ddli.ibkf = ibkmk;
				ddli.fDirty = fTrue;
				PutPl( hplddli, iddli, &ddli );
				vddes.fDirtyLinks = fTrue;
				DdeDbgCommAtom ("Validating", ddli.dcl, ddli.atomItem);
				}
			else
				/*  we still have invalid links */
				{
				vddes.fInvalidDdli = fTrue;
				DdeDbgCommAtom ("Still Invalid", ddli.dcl, ddli.atomItem);
				}
			}
		}
	MeltHp ();
}




/* U P D A T E  D I R T Y  L I N K S */
/*  For each dirty server link, send new data.
*/
/*  %%Function:UpdateDirtyLinks %%Owner:peterj  */
UpdateDirtyLinks ()
{

	int iddli, iddliMac;
	struct PL **hplddli = vddes.hplddli;
	struct DDLI ddli;

	vddes.fDirtyLinks = fFalse;

	if (hplddli == hNil)
		return; /* no links */

	DdeDbgCommSz ("UpdateDirtyLinks");
	FreezeHp ();
	iddliMac = IMacPl( hplddli );
	for (iddli = 0; iddli < iddliMac; iddli++)
		{
		GetPl( hplddli, iddli, &ddli );
		if (ddli.fDirty && ddli.ibkf != ibkmkNil)
			{
			/* this link is dirty, send an update */
			int dcl = ddli.dcl;
			int doc = PdcldDcl (dcl)->doc;
			ATOM atom = ddli.atomItem;
			BOOL f;
			ReaddAtom (atom);

			MeltHp ();
			f = FSendData (dcl, doc, atom, ddli.cf, iddli);
			DdeDbgCommAtom ("UpdateDirtyLinks: sending update", dcl, atom);
			FreezeHp ();
			if (!f)
				DeleteAtom (ddli.atomItem);
			else
				{
				GetPl (hplddli, iddli, &ddli);
				ddli.fDirty = fFalse;
				PutPl (hplddli, iddli, &ddli);
				}
			}
		}
	MeltHp ();
}


/* D E L E T E  D D L I  S E R V E R */
/*  Delete a link for which we are the server.
*/
/*  %%Function:DeleteDdliServer %%Owner:peterj  */
DeleteDdliServer (iddli)
int iddli;
{
	struct PL **hplddli = vddes.hplddli;
	struct DDLI ddli;

	FreezeHp ();
	Assert (hplddli != hNil);
	Assert (IMacPl( hplddli ) > iddli);

	GetPl( hplddli, iddli, &ddli );
	DeleteAtom (ddli.atomItem);
	MeltHp ();

	DeleteFromPl (hplddli, iddli);

	if (IMacPl( hplddli ) == 0)
		FreePh (&vddes.hplddli);
}



/* F  R E N D E R  D O C  I T E M  C F */
/*  Return true if we can render the bookmark (doc, atomItem) in format
	cf.
*/
/*  %%Function:FRenderDocItemCf %%Owner:peterj  */
FRenderDocItemCf (doc, atomItem, cf)
int doc, cf;
ATOM atomItem;
{

	CP cpFirst, cpLim;

	DdeDbgCommAtom ("Trying to render", cf, atomItem);

	if (doc == docSystem)
		return cf == CF_TEXT && StiFromSystemItem (atomItem) != stiNil;

	else
		/*  first find the bookmark, if it exists check the format */
		return (FCpsFromDocItem (doc, atomItem, &cpFirst, &cpLim)
				&& FCanWriteCf (cf, doc, cpFirst, cpLim, fFalse /*fRestrictRTF */));
}



/* F  C P S  F R O M  D O C  I T E M */
/*  %%Function:FCpsFromDocItem %%Owner:peterj  */
FCpsFromDocItem (doc, atomItem, pcpFirst, pcpLim)
int doc;
ATOM atomItem;
CP *pcpFirst, *pcpLim;
{
	int ibkmk;
	CHAR stBkmk [cchBkmkMax];

	if (doc == docSystem)
		{
		Assert (fFalse);
		return fFalse;
		}

	StFromAtom (stBkmk, cchBkmkMax, atomItem);
	return FSearchBookmark(doc, stBkmk, pcpFirst, pcpLim, NULL, bmcUser|bmcDoc)
			&& *pcpLim > *pcpFirst;
}


/* I B K M K  F R O M  D C L  I T E M */
/*  %%Function:IbkmkFromDclItem %%Owner:peterj  */
IbkmkFromDclItem (dcl, atomItem)
int dcl;
ATOM atomItem;
{
	int ibkmk;
	int doc = PdcldDcl (dcl)->doc;
	CHAR st [cchBkmkMax];

	if (doc == docSystem || !mpdochdod [doc])
		return ibkmkNil;

	Assert (!PdodDoc (doc)->fShort);
	StFromAtom (st, cchBkmkMax, atomItem);
	FSearchBookmark(doc, st, NULL, NULL, &ibkmk, bmcUser|bmcDoc);
	return ibkmk;
}


/* S T I  F R O M  S Y S T E M  I T E M */
/*  Return the system item number identified by atomItem.
*/
/*  %%Function:StiFromSystemItem %%Owner:peterj  */
StiFromSystemItem (atomItem)
ATOM atomItem;
{
	CHAR rgch [cchMaxSz];

	StFromAtom (rgch, cchMaxSz, atomItem);
	StToSzInPlace (rgch);
	return IdFromSzgSz (szgDdeSysTopic, rgch);
}


/* F  S E N D  D A T A */
/*  Sends atomItem over channel dcl.  If iddli != iddliNil then sends over
	link iddli (sets hData and sets fAck, fResponse correctly).  Returns
	false if the data could not be sent.  atomItem should be reused or
	deleted iff this returns false.
*/
/*  %%Function:FSendData %%Owner:peterj  */
FSendData (dcl, doc, atomItem, cf, iddli)
int dcl, doc, cf, iddli;
ATOM atomItem;
{
	HANDLE hData;
	struct DMS dms;
	BOOL fData;
	CP cpFirst, cpLim;
	LPSTR lpch;
	struct DDLI ddli;
	int fBlankPic = fFalse;


	DdeDbgCommAtom ("Sending Data", dcl, atomItem);

	SetBytes (&dms, 0, cbDMS);

	if (iddli != iddliNil)
		{
		/* this is a response to an ADVISE */

		GetPl( vddes.hplddli, iddli, &ddli );
		dms.fAck = ddli.fAckReq;
		fData = !ddli.fNoData;
		dms.fResponse = fFalse;
		}
	else
		{
		/* this is a response to a REQUEST */
		dms.fAck = fFalse;
		dms.fResponse = fTrue;
		fData = fTrue;
		}

	dms.cf = cf;
	dms.fRelease = fTrue;
	Assert (cbDMS == 4); /* cb in hData before actual data */

	if (fData)
		{
		if (doc == docSystem)
			{
			if ((hData = HDataWriteSti (StiFromSystemItem (atomItem)))
					== NULL)
				return fFalse;
			}
		/* find the area refered to and copy it to hData */
		else  if (!FCpsFromDocItem (doc, atomItem, &cpFirst, &cpLim)
				|| (hData = HDataWriteDocCps (cf, doc, cpFirst, cpLim,
				cbDMS, &fBlankPic))
				== NULL)
			/* note: if fBlankPic was true, we had an inserted graphics
				frame selected, and the handle returned is null, but it is
				not an error. Could alert at this point.
			*/
			{
			return fFalse;
			}

		/*  copy out the status portion */
		if ((lpch = GlobalLockClip (hData)) == NULL)
			goto LFailed;
		bltbx ((LPSTR)&dms, lpch, cbDMS);
		GlobalUnlock (hData);
		}
	else
		hData = NULL; /* send null if they ask for no data */


	/*  all set, send the data message */
	if (!FPostDdeDcl (dcl, WM_DDE_DATA, hData, atomItem))
		{
LFailed:
		/* message could not be sent, free data. caller frees atom */
		if (hData != NULL)
			GlobalFree (hData);
		return fFalse;
		}

	else  if (iddli != iddliNil)
		{
		GetPl( vddes.hplddli, iddli, &ddli );
		if (ddli.fAckReq)
			{
			ddli.hData = hData;
			ddli.dls = dlsWait;
			PutPl (vddes.hplddli, iddli, &ddli);
			}
		}

	DdeDbgCommSz ("SendData completed successfully");
	return fTrue;
}


/* H  D A T A  W R I T E  S T I */
/*  Write system topic item sti to a global handle.  Return NULL if it
	cannot be written.  Handle has cbDMS bytes at the begining unused.
*/
/*  %%Function:HDataWriteSti %%Owner:peterj  */
HDataWriteSti (sti)
int sti;
{
	HANDLE h = NULL;
	CHAR FAR *lpch;
	int ich = cbDMS, cch = 1 + cbDMS;
	CHAR st [cchMaxSt];

	if ((h = OurGlobalAlloc(GMEM_DDE, (DWORD)(cch))) == NULL)
		goto LFailed;

	switch (sti)
		{
#ifdef DEBUG
	default:
		Assert (fFalse);
		break;
#endif /* DEBUG */

	case stiSysItems:       /* list item offered on System Topic */
			{
			int stiSend;
			for (stiSend = stiMin; stiSend < stiMac; stiSend++)
				{
				st [0] = CchSzFromSzgId (st+1, szgDdeSysTopic, stiSend,
						ichMaxFile) - 1;
				if (!FAddStToHData (&h, &cch, &ich, st))
					goto LFailed;
				}
			break;
			}

	case stiTopics:         /* list topics offered */
			{
			int doc;
			/* system topic */
			if (!FAddStToHData (&h, &cch, &ich, StNear(stSystemDde)))
				goto LFailed;
			/* each open document (even those not in a window) */
			for (doc = docMinNormal; doc < docMac; doc++)
				{
				GetDocSt (doc, st, gdsoFullPath); /* works for all doc's */
				if (*st)
					if (!FAddStToHData (&h, &cch, &ich, st))
						goto LFailed;
				}

			break;
			}

	case stiFormats:        /* list clipboard formats rendered */
			{
			int cf = cfNil;
			CHAR *pch;

			/*  standard formats */
			while ((cf = CfNextCf (cf, fFalse)) != cfNil)
				{
				GetFormatSt(cf, st);
				if (!FAddStToHData (&h, &cch, &ich, st))
					goto LFailed;
				}
			/* add link format */
			GetFormatSt(cfLink, st);
			if (!FAddStToHData (&h, &cch, &ich, st))
				goto LFailed;

			break;
			}
		}

	/*  null terminate */
	if ((lpch = GlobalLockClip (h)) == NULL)
		goto LFailed;
	lpch [ich] = 0;
	GlobalUnlock (h);

	return h;

LFailed:
	if (h != NULL)
		GlobalFree (h);
	return NULL;
}


csconst CHAR stCsText[] = StKey("TEXT", CfText);
csconst CHAR stCsBitmap[] = StKey("BITMAP", CfBitmap);
csconst CHAR stCsMetafile[] = StKey("METAFILEPICT", CfMetafile);

/* G E T  F O R M A T  S T */
/*  %%Function:GetFormatSt %%Owner:peterj  */
GetFormatSt(cf, st)
int cf;
CHAR *st;
{
	switch (cf)
		{
	case CF_TEXT:
		CopyCsSt(stCsText, st);
		break;
	case CF_BITMAP:
		CopyCsSt(stCsBitmap, st);
		break;
	case CF_METAFILEPICT:
		CopyCsSt(stCsMetafile, st);
		break;
	default:
		st [0] = GetClipboardFormatName(cf, (LPSTR)st+1, cchMaxSt-1);
		break;
		}
}


/* F  A D D  S T  T O  H  D A T A */
/*  Append st to *ph.  *pcch is the size of the block, *pich is the location
	for the next item.  If *pich is > cbDMS, prepend a tab.
*/
/*  %%Function:FAddStToHData %%Owner:peterj  */
FAddStToHData (ph, pcch, pich, st)
HANDLE *ph;
int *pcch, *pich;
CHAR *st;
{
	CHAR FAR *lpch;
	HANDLE h;
	BOOL fAddTab = (*pich > cbDMS ? 1 : 0);

	if ((h = OurGlobalReAlloc(*ph, (long)(uns)(*pcch += (*st + fAddTab)), 
			GMEM_DDE)) == NULL)
		return fFalse;
	*ph = h;

	if ((lpch = GlobalLockClip (h)) == NULL)
		return fFalse;

	if (fAddTab)
		lpch [(*pich)++] = chTab;
	bltbx ((CHAR FAR *)st+1, (CHAR FAR *)&lpch [*pich], *st);
	*pich += *st;

	GlobalUnlock (h);
	return fTrue;
}



/* F  A D V I S E  I T E M */
/*  Set up an advise link on dcl for atomItem.
*/
/*  %%Function:FAdviseItem %%Owner:peterj  */
FAdviseItem (dcl, atomItem, cf, fAckReq, fNoData)
int dcl, cf;
ATOM atomItem;
BOOL fAckReq, fNoData;
{
	struct DDLI ddli;

	if (vddes.hplddli == hNil)
		if ((vddes.hplddli = HplInit (cbDDLI, 1)) == hNil)
			return fFalse;

	ReaddAtom (atomItem);

	DdeDbgCommAtom ("FAdviseItem", dcl, atomItem);

	SetBytes (&ddli, 0, cbDDLI);
	ddli.dcl = dcl;
	ddli.fAckReq = fAckReq;
	ddli.fNoData = fNoData;
	ddli.fDirty = fFalse;
	ddli.cf = cf;
	ddli.dls = dlsClear;
	ddli.ibkf = IbkmkFromDclItem (dcl, atomItem);
	ddli.atomItem = atomItem;
	ddli.hData = NULL;

	/* guaranteed by init call */
	AssertDo(FInsertInPl (vddes.hplddli, (*vddes.hplddli)->iMac, &ddli));

	return fTrue;
}




