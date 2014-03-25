/* D D E C L N T . C */
/*  Functions to implement client functionality of DDE */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "dde.h"
#include "doc.h"
#include "field.h"
#include "wininfo.h"
#include "ch.h"
#include "message.h"
#include "error.h"
#include "idle.h"

#include "debug.h"
#include "opuscmd.h"

#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"

#include "pastelnk.hs"
#include "pastelnk.sdm"


#ifdef PROTOTYPE
#include "ddeclnt.cpt"
#endif /* PROTOTYPE */

extern struct SAB       vsab;
extern HWND             vhwndApp;
extern int              cfLink;
extern BOOL             vfInsertMode;
extern BOOL             vfAbortInsert;
extern int              docDde;
extern struct DOD     **mpdochdod[];
extern struct DCLD    **mpdclhdcld[];
extern int              docMac;
extern int              dclMac;
extern BOOL             vfDdeIdle;
extern struct MERR      vmerr;
extern int              vdocScratch;
extern BOOL             vfDeactByOtherApp;
extern struct DDES      vddes;
extern IDF		vidf;

csconst CHAR stAppDde[] = St("WinWord");
csconst CHAR szCSMergeFormat[] = SzSharedKey(" \\* mergeformat",DdeMergeFormat);

/* C M D  P A S T E  L I N K */
/*  Insert a dde field at the current selection based on available paste link
	data and refresh it.
*/
/*  %%Function:CmdPasteLink %%Owner:peterj  */
CMD CmdPasteLink (pcmb)
CMB * pcmb;
{
	CABPASTELINK * pcab;
	CHAR szArg [cchMaxSz];
	CMD cmd = cmdError;

	StartLongOp();

	if (!FFillSzArgPasteLink (szArg))
		goto LReturn;

	if (FCmdFillCab())
		{
		CHAR szClean[cchMaxSz];
		SanitizeSz(szArg, szClean, cchMaxSz, fTrue);
		if (!FSetCabSz(pcmb->hcab, szClean, Iag(CABPASTELINK, hszLink)))
			return cmdNoMemory;

		pcab = *pcmb->hcab;
		pcab->fHot = fFalse;
		}

	if (FCmdDialog())
		{
		char dlt [sizeof (dltPasteLink)];

		BltDlt(dltPasteLink, dlt);
		switch (TmcOurDoDlg(dlt, pcmb))
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			goto LReturn;
#endif
		case tmcCancel:
			cmd = cmdCancelled;
			goto LReturn;

		case tmcError:
			goto LReturn;

		case tmcOK:
			break;
			}
		}

	if (FCmdAction())
		{
		int cch;
		pcab = *pcmb->hcab;

		/* add merge format switch */
		if ((cch = CchSz(szArg)-1) + sizeof(szCSMergeFormat) <= cchMaxSz)
			bltbx(szCSMergeFormat, (CHAR FAR *)&szArg[cch],
					sizeof(szCSMergeFormat));

		cmd = CmdInsFltSzAtSelCur (pcab->fHot ? fltDdeHot : fltDde, szArg, 
				imiPasteLink, fTrue/*fCalc*/, fTrue/*fShowDefault*/, 
				fFalse/*ignored*/);
		}
	else
		cmd = cmdOK;

LReturn:
	EndLongOp(fFalse);
	return cmd;
}


/* F  D L G  P A S T E  L I N K */
/*  %%Function:FDlgPasteLink %%Owner:peterj  */
BOOL FDlgPasteLink(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	CHAR rgch[cchMaxSz];

	if (dlm==dlmInit)
		{
		GetTmcText(tmcLinkTo, rgch, cchMaxSz);
		if (FTruncateTmcSz(tmcLinkTo, rgch))
			SetTmcText(tmcLinkTo, rgch);
		}
	return fTrue;
}


/* E L  P A S T E  L I N K */
/*  %%Function:ElPasteLink %%Owner:peterj  */
ElPasteLink(fHot)
BOOL fHot;
{
	char szArg [cchMaxSz];
	int cch;

	if (!FFillSzArgPasteLink(szArg))
		{
		Beep();
		return;
		}

	/* add merge format switch */
	if ((cch = CchSz(szArg)-1) + sizeof(szCSMergeFormat) <= cchMaxSz)
		bltbx(szCSMergeFormat, (CHAR FAR *)&szArg[cch],
				sizeof(szCSMergeFormat));

	CmdInsFltSzAtSelCur(fHot ? fltDdeHot : fltDde, szArg, imiPasteLink,
			fTrue /* fCalc */, fTrue /* fShowDefault */, 
			fFalse /* ignored */ );
}


/* F  F I L L  S Z A R G  P A S T E  L I N K */
/*  Get the pastelink data and fill szarg with the resulting field
	commands.
*/
/*  %%Function:FFillSzArgPasteLink %%Owner:peterj  */
FFillSzArgPasteLink (szArg)
CHAR *szArg;
{
	CHAR *pchArg = szArg;
	CHAR *pchArgMac = pchArg + cchMaxSz;
	HANDLE hData;
	CHAR FAR *lpch;
	BOOL rgfMustQuote[3];
	int isz;
	CHAR rgsz [3][ichMaxFile];

	if (vsab.fOwnClipboard || !OpenClipboard (vhwndApp))
		return fFalse;

	if ((hData = GetClipboardData (cfLink)) == NULL ||
			(lpch = GlobalLockClip (hData)) == NULL)
		{
		CloseClipboard();
		return fFalse;
		}

	/*  clipboard format is "App\0Topic\0Item\0\0" */
	for (isz = 0; isz < 3; isz++)
		{
		CHAR *pch = rgsz [isz];
		CHAR *pchMac = pch + ichMaxFile - 1;
		rgfMustQuote [isz] = *lpch ? fFalse:fTrue;
		while (pch < pchMac && (*pch = *lpch++) != '\0')
			{
			if (FWhite(*pch))
				{
				rgfMustQuote[isz] = fTrue;
#ifdef CRLF
				if (*pch == chReturn || *pch == chEol)
#else
					if (*pch == chEol)
#endif /* CRLF */
						*pch = ' ';
				}
			else  if (*pch == chGroupExternal)
				{
				*pch++ = chFieldEscape;
				*pch = chGroupExternal;
				}
			else  if (*pch == chFieldEscape)
				{
				*++pch = chFieldEscape;
				}

			pch++;
			}
		*pch = 0;
		}

	GlobalUnlock (hData);
	CloseClipboard();

	/*  output form:  "App Topic Item\0"  Handle whitespace and empty args */
	for (isz = 0; isz < 3; isz++)
		{
		CHAR *pch = rgsz [isz];
		int cch = CchSz(pch)-1;
		if (pchArg + 3 + cch > pchArgMac)
			break;
		if (rgfMustQuote [isz])
			{
			*pchArg++ = chGroupExternal;
			}
		bltb (pch, pchArg, cch);
		pchArg += cch;
		if (rgfMustQuote [isz])
			{
			*pchArg++ = chGroupExternal;
			}
		*pchArg++ = isz < 2 ? ' ' : '\0';
		}

#ifdef DEBUG
	if (vdbs.fCommDde1)
		{
		CommSz (szArg);
		CommSz (SzShared("\n\r"));
		}
#endif /* DEBUG */
	return fTrue;
}


/* F C R  C A L C  F L T  D D E */
/*  Refresh a dde or ddehot field.
	Format:
		{dde app topic [item]}
		{ddehot app topic [item]}
*/
/*  %%Function:FcrCalcFltDde %%Owner:peterj  */
FcrCalcFltDde (doc, ifld, flt, cpInst, cpResult, pffb)
int doc, ifld, flt;
CP cpInst, cpResult;
struct FFB *pffb;
{
	int iddli, dcl, fcr, istErr;
	ATOM atomApp, atomTopic, atomItem;
	struct PLC **hplcddli;
	struct DDLI ddli;
	struct FLCD flcd;
	struct CA caField;

	/*  scan the field instructions for the app, topic and item */
	if (!FGetAppTopicItemPffb (pffb, &atomApp, &atomTopic, &atomItem, 
			&istErr))
		{
		if (istErr == istNil)
			return fcrKeepOld; /* out of memory */
LError:
		CchInsertFieldError (doc, cpResult, istErr);
		return fcrError;
		}


	/* get the data */
	/* use an existing channel to app,topic or open a new one */
	if (((dcl = DclFromAppTopic (atomApp, atomTopic)) != dclNil ||
			(dcl = DclInitiateLaunch (atomApp, atomTopic, dtClient)) != dclNil)
			/* make a connection, or just get the data */
	&& (iddli = IddliAdvise (dcl, atomItem)) != iddliNil)
		/* we were able to get data */
		{
		struct CA ca, caDest;
		DdeDbgCommInt ("Advise available", iddli);
		CaFromIhdd (docDde, iddli, &ca);
		Assert (ca.doc == docDde);
		PcaField( &caField, doc, ifld );
		if (!FMoveOkForTable(doc, caField.cpLim, &ca))
			{
			istErr = istErrTableAction;
			DeleteAtom (atomApp);
			DeleteAtom (atomTopic);
			DeleteAtom (atomItem);
			goto LError;
			}
		if (FReplaceCps (PcaPoint( &caDest, doc, cpResult), &ca))
			{
			caDest.cpLim += DcpCa(&ca);
			CopyStylesFonts (docDde, caDest.doc, caDest.cpFirst, DcpCa(&caDest) );
			if (flt == fltDdeHot)
				{
				hplcddli = PdodDoc(docDde)->hplcddli;
				GetPlc( hplcddli, iddli, &ddli );
				ddli.fHot = fTrue;
				PutPlcLast( hplcddli, iddli, &ddli );
				}
			fcr = fcrNormal;
			}
		else
			/*  out of memory */
			fcr = fcrKeepOld;
		}
	else
		/* couldn't get data, advise the user */
		{
		IdPromptDde (mstDdeDataNotAvail, atomApp, atomTopic,
				atomItem, (MB_OK | MB_ICONEXCLAMATION));
		iddli = iddliRequest;
		fcr = fcrKeepOld;
		}

	/*  update the data stored in the field to reflect link/no link */
	GetIfldFlcd (doc, ifld, &flcd);
	Assert (flcd.bData == bDataInval || flcd.bData == iddliRequest ||
			flcd.bData == iddli);
	flcd.bData = iddli;
	SetFlcdCh (doc, &flcd, chFieldSeparate);

	DeleteAtom (atomApp);
	DeleteAtom (atomTopic);
	DeleteAtom (atomItem);

	return fcr;
}


/* F  G E T  A P P  T O P I C  I T E M  P F F B */
/*  Fetch from pffb an atomApp, topic and item.  Return false and error code
	if error, else true and three atoms.
*/
/*  %%Function:FGetAppTopicItemPffb %%Owner:peterj  */
FGetAppTopicItemPffb (pffb, patomApp, patomTopic, patomItem, pistErr)
struct FFB *pffb;
ATOM *patomApp, *patomTopic, *patomItem;
int *pistErr;
{
	/* read the field arguments */
	CHAR st [cchMaxSz];

	InitFvbBufs (&pffb->fvb, st+1, cchMaxSz-1, NULL, 0);
	SkipArgument (pffb);

	*patomApp = *patomTopic = *patomItem = NULL;

	/* fetch app */
	FFetchArgText (pffb, fTrue/*fTruncate*/);
	if ((*st = pffb->cch) == 0)
		{ /* no app given, error */
		*pistErr = istErrNoDdeApp;
		goto LFail;
		}
	*patomApp = AtomAddSt (st); /* failure checked by fMemFail below */

#define SELFREFER /* if I think of a good reason not to self refer, remove */
#ifndef SELFREFER
	if (*patomApp == AtomFromStNA (StNear(stAppDde)))
		{ /* app given is OPUS */
		*pistErr = istErrNoDdeSelfRefer;
		goto LFail;
		}
#endif /* ~SELFREFER */

	/* fetch topic */
	FFetchArgText (pffb, fTrue);
	if ((*st = pffb->cch) == 0)
		{
		*pistErr = istErrNoDdeTopic;
		goto LFail;
		}
	*patomTopic = AtomAddSt (st); /* failure checked by fMemFail below */

	/* fetch item (optional) */
	FFetchArgText (pffb, fTrue);
	*st = pffb->cch;
	*patomItem = AtomAddSt (st); /* failure checked by fMemFail below */

	if (vmerr.fMemFail)
		{
		*pistErr = istNil;
LFail:
		DeleteAtom(*patomApp);
		DeleteAtom(*patomTopic);
		DeleteAtom(*patomItem);
		return fFalse;
		}

	return fTrue;

}



/* I N I T I A T E  A C K */
/*  Handles the ACK messages received in response to an Initiate message.
*/
/*  %%Function:InitiateAck %%Owner:peterj  */
InitiateAck (hwndUs, hwndThem, atomApp, atomTopic)
HWND hwndUs, hwndThem;
ATOM atomApp, atomTopic;
{
	int dcl = GetWindowWord (hwndUs, IDWDDECHNLDCL);
	struct DCLD *pdcld = PdcldDcl (dcl);

	DdeDbgCommSz ("InitiateAck");

	if (pdcld->hwndThem == NULL)
		/*  make the connection to this window */
		{
		pdcld->hwndThem = hwndThem;
		}

	else
		{
		/* terminate unwanted conversations */
		DdeDbgCommSz ("InitiateAck: duplicate terminated.");
		PostMessage (hwndThem, WM_DDE_TERMINATE, hwndUs, 0L);
		}

	/*  we don't use the atoms they sent us */
	DeleteAtom (atomApp);
	DeleteAtom (atomTopic);

}


/* G A R B A G E  C O L L E C T  D D E */
/*  Find any client links we have which are not being used and delete them.

	Scan all dde and ddehot fields in all docs.  Get an iddli for each.
	Mark fRef and fHot as appropriate for each ddli. After all fields
	marked, delete any links which are not referenced (!fRef).
*/
/*  %%Function:GarbageCollectDde %%Owner:peterj  */
GarbageCollectDde ()
{
	int doc, ifld, ifldMac, iddli, iddliMacM1, dcl;
	struct PLC **hplcddli;
	struct DOD **hdod;
	struct PLC **hplcfld;
	struct DCLD **hdcld;
	struct FLCD flcd;
	ATOM atomApp, atomTopic, atomItem;
	int istErr;
	struct FFB ffb;
	struct DDLI ddli;
	struct FLD fld;
	Debug(BOOL fPostUnadviseFailed = fFalse);

	DdeDbgCommSz ("GarbageCollectDde");

	if (docDde == docNil || (hplcddli = PdodDoc (docDde)->hplcddli) == hNil)
		/*  nothing to do! */
		return;


	/*  reset fRef marks in ddlis and dcls */
	iddliMacM1 = IMacPlc( hplcddli ) - 1;
	for (iddli = 0; iddli < iddliMacM1; iddli++)
		{
		GetPlc( hplcddli, iddli, &ddli );
		ddli.fHot = fFalse;
		ddli.fRef = fFalse;
		PutPlcLast( hplcddli, iddli, &ddli );
		}
	for (dcl = 0; dcl < dclMac; dcl++)
		if ((hdcld = mpdclhdcld [dcl]) != hNil)
			(*hdcld)->fRef = ((*hdcld)->dt != dtClient);


	/*  scan through visible docs and check for dde fields */
	for (doc = 0; doc < docMac; doc++)
		if ((hdod = mpdochdod [doc]) != hNil && WwDisp(doc,wwNil,fFalse) != wwNil
				&& !(*hdod)->fLockForEdit
				&& (hplcfld = (*hdod)->hplcfld) != hNil)
			/*  may have dde fields we are interested in */
			{
			DdeRpt2(DdeDbgCommInt ("Garbage: checking doc", doc));
			if (FMsgPresent (mtyIdle))
				/*  stop if we have waiting messages */
				{
				DdeRpt2(DdeDbgCommSz ("Garbage: interrupted 1"));
				return;
				}

			AssureAllFieldsParsed (doc);
			ifldMac = IMacPlc( hplcfld );
			for (ifld = 0; ifld < ifldMac; ifld++)
				{
				GetPlc( hplcfld, ifld, &fld );
				if (fld.ch == chFieldBegin && fld.flt == fltDdeHot)
					{
					FillIfldFlcd (hplcfld, ifld, &flcd);
					iddli = flcd.bData;
					if (iddli == bDataInval && flcd.ifldChSeparate != ifldNil)
						/* must fetch the field's link info */
						{
						if (FMsgPresent (mtyIdle))
							/*  stop if we have waiting messages */
							{
							DdeRpt2(DdeDbgCommSz ("Garbage: interrupted 2"));
							return;
							}

						DdeRpt2(DdeDbgCommInt("Garbage: by brute force, ifld",ifld));
						SetFfbIfld (&ffb, doc, ifld);
						if (FGetAppTopicItemPffb (&ffb, &atomApp, &atomTopic,
								&atomItem, &istErr))
							{
							if ((dcl = DclFromAppTopic (atomApp, atomTopic))
									!= dclNil && (iddli = IddliFromDclItem (dcl,
									atomItem)) != iddliNil)
								{
								MarkIddliClient (iddli);
								/*  so we don't have to do brute force next
									time, store the iddli. */
								flcd.bData = iddli;
								SetFlcdCh (doc, &flcd, chFieldSeparate);
								}
							DeleteAtom (atomApp);
							DeleteAtom (atomTopic);
							DeleteAtom (atomItem);
							}
						}
					else  if (iddli != iddliRequest)
						MarkIddliClient (iddli);
					}
				}
			}

	AssertH (hplcddli);
	DdeRpt2(DdeDbgCommSz ("Garbage: marking complete"));

	vddes.fHotLinksDirty = fFalse;

	/*  From here on, this function is not interruptable.  If there are
		no channels or links to get rid of, the following should take
		very little time.
	*/

	/* terminate unused channels */
	for (dcl = 0; dcl < dclMac; dcl++)
		if ((hdcld = mpdclhdcld [dcl]) != hNil)
			if (!(*hdcld)->fRef)
				{
				Assert ((*hdcld)->dt == dtClient);
				TerminateDcl (dcl);
				}


	/* now, scan through all of the ddli's and delete any unused ones */
	for (iddli = (*hplcddli)->iMac - 2; iddli >= iddliMinNormal; iddli--)
		{
		GetPlc( hplcddli, iddli, &ddli );
		if (!ddli.fRef)
			/* not used, delete */
			{
			if (ddli.dcl == dclNil || PdcldDcl (ddli.dcl)->fTerminating)
				continue;
			DdeDbgCommInt ("GarbageCollectDde: deleteing", iddli);
			/*  unused ADVISE link, send an UNADVISE */
			UnreferenceClientDdlis (dclNil, iddli);
			GetPlc (hplcddli, iddli, &ddli);
			Assert (ddli.hData == NULL);
			Assert (ddli.dls == dlsClear);
			ddli.dls = dlsUnadvise;
			PutPlcLast (hplcddli, iddli, &ddli);
			ReaddAtom (ddli.atomItem);
			if (!FPostDdeDcl (ddli.dcl, WM_DDE_UNADVISE, 0, ddli.atomItem))
				{
				ddli.dls = dlsClear;
				PutPlc( hplcddli, iddli, &ddli );
				DeleteAtom (ddli.atomItem);
				DdeDbgCommInt ("GarbageCollectDde: could not delete ", iddli);
				Debug(fPostUnadviseFailed = fTrue;)
				}
			}
		else
			{
			if (!ddli.fHot)
				{
				ddli.fHotDirty = fFalse;
				PutPlc( hplcddli, iddli, &ddli );
				}
			else  /* hot links exist */
				vddes.fHotLinksDirty |= ddli.fHotDirty;
			Assert(PdcldDcl (ddli.dcl)->fRef);
			}
		}

	/* wait for all of the UNADVISE messages to come back (may take a while) */
	/*   We must wait because if we did not we would have links out there
			with an Unadvise status.  If we were to try to initiate a
			conversation about the same channel-item we would get all confused.
	*/
	if (!FWaitOnDdliClient (iddliNil))
		/* TIMEOUT: must manually delete remaining UNADVISEd ddlis */
		for (iddli = IMacPlc( hplcddli ) - 2; iddli >= iddliMinNormal; iddli--)
			{
			GetPlc( hplcddli, iddli, &ddli );
			if (!ddli.fRef && ddli.dcl != dclNil && 
					!PdcldDcl (ddli.dcl)->fTerminating)
				DeleteDdliClient (iddli);
			}

#ifdef DEBUG
	else
		for (iddli = (*hplcddli)->iMac - 2; iddli >= iddliMinNormal; iddli--)
			{
			GetPlc( hplcddli, iddli, &ddli );
			if (!ddli.fRef && ddli.dcl != dclNil && 
					!PdcldDcl (ddli.dcl)->fTerminating)
				Assert(fPostUnadviseFailed);
			}
#endif /* DEBUG */

	DdeRpt2(DdeDbgCommSz ("GarbageCollectDde complete"));
}


/* M A R K  I D D L I  C L I E N T */
/*  %%Function:MarkIddliClient %%Owner:peterj  */
MarkIddliClient (iddli)
int iddli;
{
	struct PLC **hplcddli = PdodDoc( docDde )->hplcddli;
	struct DDLI ddli;

	DdeRpt2(DdeDbgCommInt ("Marking iddli ref", iddli));

	GetPlc( hplcddli, iddli, &ddli );
	Assert (ddli.dcl != dclNil);
	Assert (iddli > iddliRequest);
	ddli.fRef = fTrue;
	ddli.fHot = fTrue;
	PutPlcLast( hplcddli, iddli, &ddli );
	Assert (PdcldDcl (ddli.dcl)->dt == dtClient);
	PdcldDcl (ddli.dcl)->fRef = fTrue;
}


/* U P D A T E  H O T  L I N K S */
/*  Scan through all docs which are displayed (have a wwDisp) and for
	any ddehot fields if their ddli has fHotDirty, update them.
*/
/*  %%Function:UpdateHotLinks %%Owner:peterj  */
UpdateHotLinks ()
{
	int doc, ifld, iddli;
	struct PLC **hplcddli = PdodDoc (docDde)->hplcddli;
	int iddliMacM1 = (*hplcddli)->iMac -1;
	struct PLC **hplcfld;
	struct FLCD flcd;
	struct DOD **hdod;
	struct DDLI ddli;
	struct FLD fld;

	DdeDbgCommSz ("UpdateHotLinks");

	Assert (docDde != docNil && hplcddli != hNil);

	for (doc = 0; doc < docMac; doc++)
		if ((hdod = mpdochdod [doc]) != hNil && (hplcfld = (*hdod)->hplcfld)
				!= hNil && WwDisp(doc,wwNil,fFalse) != wwNil 
				&& !(*hdod)->fLockForEdit)
			/*  we have a doc which may have dde fields to update */
			{
			/* scan through fields searching for DDEHOT fields */
			for (ifld = 0; ifld < (*hplcfld)->iMac; ifld++)
				{
				GetPlc( hplcfld, ifld, &fld );
				if (fld.ch == chFieldBegin && fld.flt == fltDdeHot)
					{
					FillIfldFlcd (hplcfld, ifld, &flcd);

					if (flcd.bData != bDataInval && 
							flcd.bData != iddliRequest)
						/*  bData (iddli) updated by garbage collection */
						{
						GetPlc( hplcddli, flcd.bData, &ddli );
						if (ddli.fHotDirty)
							/*  update this field */
							{
							if (FMsgPresent (mtyIdle) || vmerr.fMemFail
									|| vmerr.fDiskFail)
								/*  stop if we have waiting messages */
								/* Note: what if we keep getting
										interupted?  we will keep updating
										the same fields over and over!
										I could save the doc,ifld I am at and
										develop rules for throwing that
										information away.  Is it worth it?  I 
										think not right now.  we'll see! (will
										someone buy this product because I did??)
									*/
								{
								DdeRpt2(DdeDbgCommSz ("UpdateHotLinks interrupted"));
								return;
								}

							SetUndoNil(); /* user is HOSED (LATER) */
							StartLongOp();
							FCalcFieldIfld (doc, ifld, frmDdeHot, 
									0, fFalse);
							EndLongOp(fFalse);
							}
						}
					}
				}
			}

	vddes.fHotLinksDirty = fFalse;

	/*  reset flags */
	for (iddli = iddliMinNormal; iddli < iddliMacM1; iddli++)
		{
		GetPlc( hplcddli, iddli, &ddli );
		ddli.fHotDirty = fFalse;
		PutPlcLast( hplcddli, iddli, &ddli );
		}

	/* if we are not active, update the display */
	if (vfDeactByOtherApp && !vidf.fNotLive)
		UpdateAllWws (fTrue);

	return;
}



/* F  C L I E N T  D D E  M S G */
/*  Handle all dde messages for client channels.  Return fFalse if we cannot
	handle the message at this time (we will be given the message back
	later).
*/
/*  %%Function:FClientDdeMsg %%Owner:peterj  */
FClientDdeMsg (dcl, message, wLow, wHigh)
int dcl;
unsigned message;
int wLow, wHigh;
{
	struct PLC **hplcddli = PdodDoc (docDde)->hplcddli;
	int iddli;
	struct DDLI ddli;

	iddli = IddliFromDclItem (dcl, wHigh);
	if (iddli != iddliNil)
		GetPlc( hplcddli, iddli, &ddli );

	DdeDbgCommMsg ("Client Message:", dcl, message, wLow, wHigh);

	switch (message)
		{
	case WM_DDE_DATA:
			{
			struct DMS dms, FAR *lpdms;
			BOOL fSuccess = fFalse;
			BOOL fMustFree = fFalse;
			BOOL fWait = fFalse;

			if (vfInsertMode)
				/*  cannot receive data while in insert mode */
				{
				DdeDbgCommSz ("Data received in Insert Mode");
				vfAbortInsert = fTrue;
				return fFalse;
				}

			if (wLow == NULL)
				/* DATA message without data */
				{
				ReportDdeError ("DATA message without data, NACKing",
						dcl, message, 0);
				goto LDataNack;
				}
			if ((lpdms = GlobalLockClip (wLow)) == NULL)
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
					return fTrue;  /* drop the message */
					}
				}
			dms = *lpdms;
			GlobalUnlock (wLow);

			DdeRpt2(CommSzRgNum (SzShared("WM_DDE_DATA: dms (grpf,cf): "),
					&dms, CwFromCch(cbDMS)));

			if (dms.fResponse)
				/* data should be in response to a WM_DDE_REQUEST */
				{
				GetPlc( hplcddli, iddli = iddliRequest, &ddli );
				if (ddli.dls != dlsWait || ddli.atomItem != wHigh ||
						ddli.dcl != dcl)
					/* unexpected or unknown! */
					{
					ReportDdeError ("Unexpected response.", dcl,
							message, 0);
					DdeRpt2(DdeDbgCommAtom("their atom:", dcl, wHigh));
					goto LDataNack;
					}
				else
					fWait = fTrue;
				}

			else  if (iddli == iddliNil)
				/* data for item we don't want */
				{
				ReportDdeError ("Receiving data for unknown item",
						dcl, message, 0);
				goto LDataNack;
				}

			else
				/*  data for previous WM_ADVISE */
				{
				if (ddli.dls != dlsClear)
					/*  unexpected or unknown */
					{
#ifdef DEBUG
					if (ddli.dls != dlsUnadvise)
						ReportDdeError ("Unexpected data for Advise.",
								dcl, message, iddli);
						/* else data received after we sent UNADVISE but before
						they ACKed it, not too unusual*/
					else
						DdeDbgCommInt ("Data received after UNADVISE", iddli);
#endif /* DEBUG */
					goto LDataNack;
					}
				}

			StartLongOp();
			if (FReceiveData (iddli, wLow, dms.cf))
				{
				DdeRpt2(DdeDbgCommSz ("Data received OK"));
				fSuccess = fTrue;
				if (fWait)
					{
					GetPlc( hplcddli, iddli, &ddli );
					/*  we are waiting for this, indicate we got it */
					ddli.dls = dlsClear;
					PutPlcLast( hplcddli, iddli, &ddli );
					}
				if (!dms.fAck || (fMustFree = !FPostDdeDcl (dcl, 
						WM_DDE_ACK, dasACK, wHigh)))
					DeleteAtom (wHigh);
				}
			else
				/* couldn't read, NACK */
				{
				if (fWait)
					{
					GetPlc (hplcddli, iddli, &ddli);
					/* we are waiting for this, indicate failure */
					ddli.dls = dlsFail;
					PutPlcLast( hplcddli, iddli, &ddli );
					}
LDataNack:
				if (fMustFree = !FPostDdeDcl (dcl, WM_DDE_ACK, dasNACK, wHigh))
					DeleteAtom (wHigh);
				}
			EndLongOp(fFalse);

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
			if (iddli != iddliNil)
				switch (ddli.dls)
					{
				case dlsWait:  /* waiting on ACK for ADVISE */
					if (!das.fAck)
						{
						if (!das.fBusy)
							/* try the next one */
							ddli.cf = CfNextCf (ddli.cf, fTrue);
						else  if (PdcldDcl (dcl)->fBusyPrev)
							/* two busy's in a row, give up */
							ddli.cf = cfNil;
						PutPlc( hplcddli, iddli, &ddli );
						/* sets state if fails, don't care here */
						FPostAdvise (iddli);
						}
					else
						/* Advise is now set up! */
						{
						DdeRpt2(DdeDbgCommInt ("ADVISE set up", iddli));
						ddli.dls = dlsClear;
						ddli.hData = NULL;
						PutPlc( hplcddli, iddli, &ddli );
						}
					return fTrue;

				case dlsUnadvise: /* waiting on ACK for UNADVISE */
					if (!das.fAck && das.fBusy && 
							!PdcldDcl(dcl)->fBusyPrev)
						{
						ReaddAtom (wHigh);
						if (FPostDdeDcl (dcl, WM_DDE_UNADVISE,
								NULL, wHigh))
							return fTrue;
						DeleteAtom (wHigh);
						}

					DdeRpt2(DdeDbgCommInt ("Unadvise complete", iddli));
					DeleteDdliClient (iddli);
					return fTrue;

				default:
					goto LNotAdvise;
					}


			else  /* iddli == iddliNil */				
				{
				/* NACK or BUSY for REQUEST */
LNotAdvise:
				GetPlc( hplcddli, iddli = iddliRequest, &ddli );
				if (ddli.atomItem != wHigh || ddli.dls != dlsWait ||
						das.fAck || ddli.dcl != dcl)
					/*  we don't know why we got this ACK */
					{
					ReportDdeError ("Unexpected ACK",
							dcl, message, 0);
					return fTrue;
					}

				if (!das.fBusy)
					ddli.cf = CfNextCf (ddli.cf, fTrue);
				else  if (PdcldDcl (dcl)->fBusyPrev)
					/* can only busy us once */
					ddli.cf = cfNil;
				PutPlcLast( hplcddli, iddli, &ddli );
				if (ddli.cf != cfNil)
					{
					DdeRpt2(DdeDbgCommInt ("REQUEST cf no good", ddli.cf));
					ReaddAtom (wHigh);
					if (FPostDdeDcl (dcl, WM_DDE_REQUEST, ddli.cf, wHigh))
						/*  resend succeeded */
						{
						return fTrue;
						}
					/* else resend failed */
					DeleteAtom (wHigh);
					GetPlc ( hplcddli, iddli, &ddli );
					}
				/* else cannot try anything else, failed */
				ddli.dls = dlsFail;
				PutPlcLast( hplcddli, iddli, &ddli );
				return fTrue;
				}
			}

	default:
		/* message that shouldn't be sent to Client */
		ReportDdeError ("Message shouldn't be sent to client",
				dcl, message, 0);
		return fTrue;
		}

	Assert (fFalse);
	return fTrue;
}


/* D C L  I N I T I A T E  L A U N C H */
/*  Attempt to initiate a conversation with App, Topic.  If this
	fails, try to get App to load Topic.  If we still cannot
	initiate, try to launch App giving Topic as an argument. 
	Once more if we cannot initiate, try to get App to load
	Topic.  If we still cannot initiate, give up.
	
	Before loading another app or having it load a topic, get user
	permission.
*/
/*  %%Function:DclInitiateLaunch %%Owner:peterj  */
DclInitiateLaunch (atomApp, atomTopic, dt)
ATOM atomApp, atomTopic;
int dt;
{
	int dcl, iPass;

	DdeDbgCommAtom ("DclInitiateLaunch: App", 0, atomApp);
	DdeRpt2(DdeDbgCommAtom ("DclInitiateLaunch: Topic", 0, atomTopic));

	for (iPass = 0; (dcl = DclInitiate (atomApp, atomTopic, dt)) == dclNil;
			iPass++)
		switch (iPass)
			{
		case 0:
			if (atomApp == AtomFromStNA (StNear(stAppDde)))
				return dclNil; /* app given is OPUS -- don't launch */

			/* get user permission to try to launch */
			if (IdPromptDde (mstOkToLaunchDde, atomTopic, atomApp,
					NULL, (MB_ICONQUESTION | MB_YESNO)) != IDYES)
				return dclNil;

			if (FLaunchTopic (atomApp, atomTopic))
				break;
			else
				/*  couldn't launch topic, try launching the app */
				iPass++;
			/* FALL THROUGH */
		case 1:
			if (FLaunchAppTopic (atomApp, atomTopic))
				break;
			else
				/* couldn't launch topic or app, give up */
				return dclNil;

		case 2:
			/* we launched the app, but still cannot connect.  Try
					launching the topic again. */
			if (FLaunchTopic (atomApp, atomTopic))
				break;

			/* else:  no luck, we have done all we can */
			/* FALL THROUGH */

		case 3:
			return dclNil;
			}

	return dcl;

}


/* D C L  I N I T I A T E */
/*  Try to initiate a dde channel with app, topic.
*/
/*  %%Function:DclInitiate %%Owner:peterj  */
DclInitiate (atomApp, atomTopic, dt)
ATOM atomApp, atomTopic;
int dt;
{
	int dcl = DclCreateChannel (dt);

	if (dcl == dclNil)
		return dclNil;

	DdeRpt2(DdeDbgCommSz ("DclInitiate: trying to initiate"));

	Assert (PdcldDcl (dcl)->hwndThem == NULL);
	ReaddAtom (atomApp);
	ReaddAtom (atomTopic);

	vddes.fInInitiate = fTrue;

	SendMessage (-1, WM_DDE_INITIATE, PdcldDcl (dcl)->hwndUs,
			MAKELONG (atomApp, atomTopic));

	vddes.fInInitiate = fFalse;

	if (PdcldDcl (dcl)->hwndThem == NULL)
		/* no responses */
		{
		DeleteAtom (atomApp);
		DeleteAtom (atomTopic);
		DestroyWindow (PdcldDcl (dcl)->hwndUs);
		FreeCls (clsDCLD, dcl);
		switch (dt)
			{
		case dtClient:
			vddes.cDclClient--;
			break;
		case dtMacro:
			vddes.cDclMacro--;
			break;
#ifdef DEBUG
		default:
			Assert (fFalse);
#endif /* DEBUG */
			}
		return dclNil;
		}
	else  if (dt == dtClient)
		{  /* we use our atoms (may be different than theirs) */
		PdcldDcl (dcl)->atomApp = atomApp;
		PdcldDcl (dcl)->atomTopic = atomTopic;
		}
	else
		{  /* macros don't need to save atoms */
		Assert (dt == dtMacro);
		DeleteAtom (atomApp);
		DeleteAtom (atomTopic);
		}

	vfDdeIdle = fTrue;

	return dcl;
}


/* I D  P R O M P T  D D E */
/*  Prompt the user.
*/
/*  %%Function:IdPromptDde %%Owner:peterj  */
IdPromptDde (mst, atom1, atom2, atom3, mb)
MST mst;
ATOM atom1, atom2, atom3;
int mb;
{
	int rgw [3];
	CHAR sz1 [40];
	CHAR sz2 [40];
	CHAR sz3 [40];

	AtomToSz(atom1, sz1, 40);
	AtomToSz(atom2, sz2, 40);
	AtomToSz(atom3, sz3, 40);
	rgw [0] = sz1;
	rgw [1] = sz2;
	rgw [2] = sz3;
	return IdMessageBoxMstRgwMb (mst, rgw, mb);
}


csconst CHAR szCsNull[] = SzKey("(NULL)",NullTopic);

/* A T O M  T O  S Z */
/*  %%Function:AtomToSz %%Owner:peterj  */
AtomToSz(atom, sz, cch)
ATOM atom;
CHAR *sz;
int cch;
{
	if (atom == NULL)
		CopyCsSz(szCsNull, sz);
	else
		{
		StFromAtom (sz, cch, atom);
		StToSzInPlace (sz);
		}
}


/* F  L A U N C H  T O P I C */
/*  Attempt by whatever means available to get App to Load Topic.
	Exact means TBD, but may include Execute messages sent to apps whose
	Execute syntax we know we understand (i.e., Excel, Omega).
	Return TRUE if there is any chance we succeeded.
*/
/*  %%Function:FLaunchTopic %%Owner:peterj  */
FLaunchTopic (atomApp, atomTopic)
ATOM atomApp, atomTopic;

{
	DdeDbgCommAtom ("FLaunchTopic: App", 0, atomApp);
	DdeRpt2(DdeDbgCommAtom ("FLaunchTopic: Topic", 0, atomTopic));

	return fFalse;
}


/* F  L A U N C H  A P P  T O P I C */
/*  Attempt to launch app and give it topic as a command line argument (in
	the hopes that it will load it).
	Return TRUE if there is any chance we succeeded.
*/
/*  %%Function:FLaunchAppTopic %%Owner:peterj  */
FLaunchAppTopic (atomApp, atomTopic)
ATOM atomApp, atomTopic;

{
	int cch;
	MSG msg;
	CHAR szCmdLine [cchMaxSz];
	CHAR st [cchMaxSz];

	DdeDbgCommAtom ("FLaunchAppTopic: App", 0, atomApp);
	DdeRpt2(DdeDbgCommAtom ("FLaunchAppTopic: Topic", 0, atomTopic));


	if (atomApp == NULL)
		return fFalse;

	StFromAtom (st, cchMaxSz-2, atomApp);
	cch = *st;
	StToSz (st, szCmdLine);
	SzSzAppend(szCmdLine, SzSharedKey(".EXE",EXE));
	cch += 4;
	szCmdLine [cch++] = ' ';
	StFromAtom (st, cchMaxSz-cch, atomTopic);
	StToSz (st, &szCmdLine[cch]);

	return FOurRunApp (szCmdLine, SW_SHOWMINNOACTIVE, eidNoMemLaunchDde,
			fFalse);
}


/* I D D L I  A D V I S E */
/*  Obtain a link for Item on dcl.  Prefer to use an existing link. If none
	try to set up an ADVISE link.  Even if ADVISE link cannot be made, use
	REQUEST to try to get initial (or only if no link) data.  Return iddliNil
	if no data could be obtained, iddliRequest if data was obtained but no
	link established, or iddli of established link.
*/
/*  %%Function:IddliAdvise %%Owner:peterj  */
IddliAdvise (dcl, atomItem)
int dcl;
ATOM atomItem;
{
	int iddli = IddliFromDclItem (dcl, atomItem);
	struct PLC **hplcddli;
	struct DDLI ddli;

#ifdef DEBUG
	/* make sure this can never be reentrant */
	static fInIddliAdvise = fFalse;
	Assert(!fInIddliAdvise++);
#endif /* DEBUG */

	Assert (dcl != dclNil);
	Assert (PdcldDcl (dcl)->dt == dtClient);
	Assert (iddli != iddliRequest);

	if (iddli != iddliNil)
		/* link already exists, use it! */
		{
		DdeDbgCommInt ("IddliAdvise: using existing", iddli);
		Debug(fInIddliAdvise = 0);
		return iddli;
		}

	DdeDbgCommAtom ("IddliAdvise", dcl, atomItem);

	iddli = IddliAddDdliClient (dcl, atomItem);
	if (docDde == docNil || iddli == iddliNil)
		return iddliNil;
	hplcddli = PdodDoc( docDde )->hplcddli;

	if (!FPostAdvise (iddli))
		/*  could not Post an advise, resort to just Request */
		{
		goto LNoAdvise;
		}

	/*  wait for a response to advise */
	else  if (!FWaitOnDdliClient (iddli))
		/* timed out! */
		{
		ReportDdeError ("No response to ADVISE.", dcl, 0, iddli);
		goto LNoAdvise;
		}

	/* check if advise succeeded */
	else 		
		{
		GetPlc (hplcddli, iddli, &ddli);
		if (ddli.dls != dlsClear)
			/* failed */
			{
LNoAdvise:
			DdeDbgCommSz ("IddliAdvise: can't ADVISE");
			DeleteDdliClient (iddli);
			iddli = iddliRequest;
			GetPlc( hplcddli, iddli, &ddli );
			}
		}

	/* now we have or haven't established the ADVISE.  Use a REQUEST to
		get initial data (if ADVISE succeeded) or to get all we are going to
		get (if ADVISE failed). */
	if (!FRequestDcl (dcl, atomItem, ddli.cf))
		/* could not get initial data */
		{
		DdeRpt2(DdeDbgCommSz ("IddliAdvise: REQUEST failed"));
		if (iddli == iddliRequest)
			/* were out of tricks */
			{
			Debug(fInIddliAdvise = 0);
			return iddliNil;
			}
		/* else, no initial data, but we have an advise set up (data may come
			in later).  Call it empty for now. */
		}
	else  if (iddli != iddliRequest)
		/* we have initial data and a link, fill link with initial data */
		{
		struct CA ca, caT;
		CP dcp;
		if (DocCreateScratch (docDde) == docNil)
			/* memory trouble--bag it */
			{
LFail:
			if (vdocScratch != docNil)
				ReleaseDocScratch();
			DdeRpt2(DdeDbgCommSz ("IddliAdvise: out of memory"));
			Debug(fInIddliAdvise = 0);
			return iddliNil;
			}
		CaFromIhdd (docDde, iddliRequest, &ca);
		Assert (ca.doc == docDde);
		if (!FReplaceCps (PcaPoint(&caT, vdocScratch, cp0), &ca))
			goto LFail;
		dcp = DcpCa(&ca);
		CaFromIhdd (docDde, iddli, &ca);
		Assert (ca.doc == docDde);
		if (!FReplaceCps (&ca,PcaSet( &caT, vdocScratch, cp0, dcp )))
			goto LFail;
		ReleaseDocScratch ();
		}

	Debug(fInIddliAdvise = 0);
	return iddli;
}


/* F  R E Q U E S T  D C L */
/*  Place a request over dcl for Item.  If cf is not nil, start with it. 
	Data on success is placed in iddliRequest.  Data should be used promptly!
*/
/*  %%Function:FRequestDcl %%Owner:peterj  */
FRequestDcl (dcl, atomItem, cf)
int dcl;
ATOM atomItem;
int cf;
{
	struct PLC **hplcddli;
	BOOL fSuccess;
	struct DDLI ddli;

	DdeDbgCommAtom ("FRequestDcl", dcl, atomItem);

	/*  assure docDde, iddliRequest set up */
	if (!FEnsureDocDde ())
		return fFalse;

	GetPlc( hplcddli = PdodDoc( docDde )->hplcddli, iddliRequest, &ddli );

	Assert (ddli.dcl == dclNil && ddli.dls == dlsClear && ddli.atomItem == NULL);

	ddli.dcl = dcl;
	ddli.dls = dlsWait;
	ddli.atomItem = atomItem;
	ddli.cf = (cf == cfNil ? CfNextCf (cfNil, fTrue) : cf);
	PutPlcLast( hplcddli, iddliRequest, &ddli );

	ReaddAtom (atomItem);

	/* shrink swap area to give other app more room for data */
	ShrinkSwapArea();

	if (!FPostDdeDcl (dcl, WM_DDE_REQUEST, ddli.cf, atomItem))
		/* Post failed */
		DeleteAtom (atomItem);

	else
		/* wait for response */
		FWaitOnDdliClient (iddliRequest);

	GrowSwapArea();  /* restore */

	GetPlc( hplcddli, iddliRequest, &ddli );
	fSuccess = (ddli.dls == dlsClear);
	ddli.dls = dlsClear;
	ddli.dcl = dclNil;
	ddli.atomItem = NULL;
	ddli.cf = cfNil;
	PutPlcLast( hplcddli, iddliRequest, &ddli );

#ifdef DEBUG
	if (fSuccess)
		DdeDbgCommSz ("FRequestDcl: successful");
	else
		DdeDbgCommSz ("FRequestDcl: failed");
#endif /* DEBUG */
	return fSuccess;
}


/* F  W A I T  O N  D D L I  C L I E N T */
/*  Process DDE messages and yield until the status (dls) of iddli is no
	longer dlsWait (or dlsUnadvise) or until TIMEOUT.  Return fFalse on 
	timeout.

	If iddli==iddliNil then wait until ALL links are no longer waiting or
	until TIMEOUT.
*/
/*  %%Function:FWaitOnDdliClient %%Owner:peterj  */
FWaitOnDdliClient (iddli)
int iddli;
{
	struct DQM dqm;
	struct PLC **hplcddli = PdodDoc (docDde)->hplcddli;
	int iddliT, iddliMacM1;
	int dclWaiting;
	BOOL fHourGlass = fFalse;
	LONG usecTimeOut;
	LONG usecHourGlass = GetTickCount () + usecHourGlassDef;
	struct DDLI ddli;

	Assert (!vfInsertMode);
	Assert (docDde != docNil && hplcddli != hNil);

	DdeRpt2(DdeDbgCommInt ("FWaitOnDdliClient", iddli));

	do
		{
		usecTimeOut = GetTickCount() + UsecDdeTimeOut();

		do
			{
			while (FDequeue(vddes.hque, &dqm))
				{
				if (dqm.dwm == WM_DDE_TERMINATE-WM_DDE_FIRST && 
						iddli != iddliNil)
					/* handle terminate message */
					{
					GetPlc( hplcddli, iddli, &ddli );
					if (dqm.dcl == ddli.dcl)
						{
						FPushQueue(vddes.hque, &dqm);
						dclWaiting = dqm.dcl;
						goto LStopWaiting;
						}
					}

				AssertDo(FHandleDdeMsg (dqm.dcl, dqm.dwm + WM_DDE_FIRST, 
						dqm.wLow, dqm.wHigh));
				}

			if (iddli == iddliNil)
				{
				dclWaiting = dclNil;
				iddliMacM1 = IMacPlc( hplcddli ) - 1;
				for ( iddliT = iddliMinNormal; iddliT < iddliMacM1; iddliT++ )
					{
					GetPlc( hplcddli, iddliT, &ddli );
					dclWaiting = 
							(ddli.dls == dlsWait || ddli.dls == dlsUnadvise) ?
							ddli.dcl : dclWaiting;
					}
				}
			else
				{
				GetPlc( hplcddli, iddli, &ddli );
				dclWaiting = (ddli.dls == dlsWait || ddli.dls == dlsUnadvise) ?
						ddli.dcl : dclNil;
				}

			}
		while (dclWaiting != dclNil && 
				FWaitDdeMessage (usecTimeOut, usecHourGlass, &fHourGlass));

		}
	while (dclWaiting != dclNil && FWaitLongerDde (dclWaiting));

LStopWaiting:
	if (fHourGlass)
		/* took a while, take down the hour glass */
		EndLongOp(fFalse);

	DdeRpt2(DdeDbgCommSz ("Wait complete"));

	return dclWaiting == dclNil;
}


/* F  P O S T  A D V I S E */
/*  Post a WM_DDE_ADVISE message for client link iddli.  Adds and deletes
	atoms.  Allocates data if not already allocated and frees it on failure.
	On failure sets ddli to correct failure values.
*/
/*  %%Function:FPostAdvise %%Owner:peterj  */
FPostAdvise (iddli)
int iddli;
{
	struct PLC **hplcddli = PdodDoc(docDde)->hplcddli;
	HANDLE hData;
	struct DMS FAR *lpdms, dms;
	struct DDLI ddli;

	GetPlc( hplcddli, iddli, &ddli );
	hData = ddli.hData;
	Assert (ddli.hData == NULL || ddli.dls == dlsWait);
	Assert (ddli.dls == dlsWait || ddli.dls == dlsClear);
	Assert (ddli.dls != dlsClear || ddli.cf == cfNil);

	DdeDbgCommInt ("FPostAdvise: iddli =", iddli);

	if (ddli.dls != dlsClear)
		{
		if (ddli.cf == cfNil)
			/*  nothing left to try */
			{
			DdeRpt2(DdeDbgCommSz ("No formats left"));
			goto LFail;
			}
		}
	else
		{
		ddli.dls = dlsWait;
		ddli.cf = CfNextCf (cfNil, fTrue);
		PutPlcLast( hplcddli, iddli, &ddli );
		}

	DdeRpt2(DdeDbgCommInt ("FPostAdvise: cf = ", ddli.cf));

	if ((hData == NULL && (hData = OurGlobalAlloc(GMEM_DDE,
			(DWORD)cbDMS)) == NULL)
			|| (lpdms = GlobalLockClip (hData)) == NULL)
		{
		DdeRpt2(DdeDbgCommSz ("Cannot allocate or lock"));
		goto LFail;
		}

	SetBytes (&dms, 0, cbDMS);
	dms.fAck = fTrue; /* cause them to cause us to send ACK, slow down data */
	/*dms.fNoData = fFalse;*/
	dms.cf = ddli.cf;
	*lpdms = dms;
	GlobalUnlock (hData);

	ReaddAtom (ddli.atomItem);

	if (!FPostDdeDcl (ddli.dcl, WM_DDE_ADVISE, hData, ddli.atomItem))
		/*  could not post, consider failure */
		{
		DeleteAtom (ddli.atomItem);
LFail:
		if (hData)
			GlobalFree (hData);
		ddli.hData = NULL;
		ddli.dls = dlsFail;
		ddli.cf = cfNil;
		PutPlc( hplcddli, iddli, &ddli );
		DdeRpt2(DdeDbgCommSz ("FPostAdviseAdvise: failed"));
		return fFalse;
		}

	return fTrue;
}


/* F  R E C E I V E  D A T A */
/*  Receive data hData over link iddli in format cf. Cause Hot links to be
	updated later.  If iddli == iddliNil, data si for macro request.
*/
/*  %%Function:FReceiveData %%Owner:peterj  */
FReceiveData (iddli, hData, cf)
int iddli;
HANDLE hData;
int cf;
{
	struct PLC **hplcddli = PdodDoc (docDde)->hplcddli;
	struct CA ca;
	struct DDLI ddli;

	DdeDbgCommInt ("FReceiveData: iddli = ", iddli);
	Assert (hData != NULL);

	GetPlc( hplcddli, iddli, &ddli );
	if (ddli.dcl == dclNil || PdcldDcl( ddli.dcl )->fTerminating)
		{
		DdeRpt2(DdeDbgCommSz ("FReceiveData: dead or dying"));
		return fFalse;
		}

	CaFromIhdd (docDde, iddli, &ca);

	Assert (ca.doc == docDde);

	if (!FReadHData (hData, cf, &ca, cbDMS))
		{
		DdeRpt2(DdeDbgCommSz ("FReceiveData: FReadHData failed"));
		return fFalse;
		}

	/* kill any bookmarks that crept into the dde document */
	ScratchBkmks(PcaSetWholeDoc(&ca, docDde));

	/* cause hot links to be updated */
	if (iddli != iddliRequest)
		{
		DdeRpt2(DdeDbgCommInt ("Received data for hot link", iddli));
		vddes.fHotLinksDirty = fTrue;
		ddli.fHotDirty = fTrue;
		PutPlc( hplcddli, iddli, &ddli );
		}

	return fTrue;
}


/* I D D L I  A D D  D D L I  C L I E N T */
/*  Create a new iddliClient and return it.  May create docDde and allocate
	iddliRequest.
*/
/*  %%Function:IddliAddDdliClient %%Owner:peterj  */
IddliAddDdliClient (dcl, atomItem)
int dcl;
ATOM atomItem;
{
	int iddli, iddliMacM1;
	struct PLC **hplcddli;
	struct DDLI ddli;

	if (!FEnsureDocDde ())
		return iddliNil;

	hplcddli = PdodDoc (docDde)->hplcddli;

	/* search for an existing free ddli */
	GetPlc( hplcddli, iddli = iddliMinNormal, &ddli );
	iddliMacM1 = IMacPlc( hplcddli  ) - 1;  /* Mac - 1 because of end of doc EOP */

	while (iddli < iddliMacM1 && ddli.dcl != dclNil)
		{
		GetPlc( hplcddli, ++iddli, &ddli );
		}
	if (iddli == iddliMacM1)
		/*  no free slots, must add a new ddli */
		{
		if (iddli >= iddliMaxClient || !FInsertIhdd (docDde, iddli))
			/*  limit reached or OOM */
			return iddliNil;
		}
	ReaddAtom (atomItem);
	SetBytes( &ddli, 0, cbDDLI );
	ddli.dcl = dcl;
	ddli.atomItem = atomItem;
	PutPlc( hplcddli, iddli, &ddli );

	return iddli;
}


/* F  E N S U R E  D O C  D D E */
/*  If it does not exist, create docDde, initialize its hplcddli, and
	allocate iddliRequest.  Return fTrue if exist and no memory trouble.
*/
/*  %%Function:FEnsureDocDde %%Owner:peterj  */
FEnsureDocDde ()
{
	struct PLC **hplcddli;
	struct DOD **hdod;
	struct DDLI ddli;

	if (docDde != docNil)
		return !vmerr.fMemFail;

	DdeDbgCommSz ("Creating docDde");

	if ((docDde = DocCloneDoc (docNew, dkDoc)) == docNil)
		return fFalse;

	hdod = mpdochdod [docDde];
	AssertH(hdod);
	if (HplcCreateEdc (hdod, edcDdli) == hNil 
			|| !FInsertIhdd (docDde, iddliRequest)
			|| vmerr.fMemFail)
		{
LFailed:
		DisposeDoc (docDde);
		docDde = docNil;
		return fFalse;
		}

	hplcddli = PdodDoc(docDde)->hplcddli;
	SetBytes(&ddli, 0, cbDDLI);
	PutPlc(hplcddli, iddliRequest, &ddli);

	return fTrue;
}


/* U N R E F E R E N C E  D D L I S  C L I E N T */
/*  Scan through all open fields.  If any DDE or DDEHOT field refers to
	iddli (if iddli != iddliNil) or to a ddli of dcl (if dcl != dclNil) then
	make it refer to iddliRequest.
*/
/*  %%Function:UnreferencedClientDdlis %%Owner:peterj  */
UnreferenceClientDdlis (dcl, iddli)
int dcl, iddli;
{
	struct PLC **hplcddli;
	struct PLC **hplcfld;
	int doc, ifld, iddliT;
	struct FLCD flcd;
	struct DOD **hdod;
	struct DDLI ddli;
	struct FLD fld;

	if (docDde == docNil || (hplcddli = PdodDoc (docDde)->hplcddli) == hNil)
		return;

	for (doc = 0; doc < docMac; doc++)
		if ((hdod = mpdochdod [doc]) != hNil &&
				(hplcfld = (*hdod)->hplcfld) != hNil)
			{
			ifld = 0;
			while (ifld < IMacPlc( hplcfld ))
				{
				GetPlc( hplcfld, ifld, &fld );
				if (fld.ch == chFieldBegin &&
						(fld.flt == fltDde || fld.flt == fltDdeHot))
					{
					FillIfldFlcd (hplcfld, ifld, &flcd);

					if ((iddliT = flcd.bData) != bDataInval)
						{
						GetPlc( hplcddli, iddliT = flcd.bData, &ddli );

						if ((dcl != dclNil && iddliT != iddliRequest
								&& ddli.dcl == dcl)  ||
								(iddli != iddliNil && iddliT == iddli))
							{
							flcd.bData = iddliRequest;
							SetFlcdCh (doc, &flcd, chFieldSeparate);
							}
						}
					}
				ifld++;
				}
			}
}


/* D E L E T E  D D L I S  O F  D C L  C L I E N T */
/*  Delete all ddli which are associated with dcl.
*/
/*  %%Function:DeleteDdlisOfDclClient %%Owner:peterj  */
DeleteDdlisOfDclClient (dcl)
int dcl;
{
	struct PLC **hplcddli;
	int iddli;
	struct DDLI ddli;

	if (docDde == docNil || (hplcddli = PdodDoc (docDde)->hplcddli) == hNil)
		return;

	iddli = IMacPlc( hplcddli ) - 1;
	while (iddli--)
		{
		GetPlc( hplcddli, iddli, &ddli );
		if (ddli.dcl == dcl)
			DeleteDdliClient (iddli);
		}
}


/* D E L E T E  D D L I  C L I E N T */
/*  Delete iddli from docDde. References to iddli (in plcfld's) must have
	already been cleared.
*/
/*  %%Function:DeleteDdliClient %%Owner:peterj  */
DeleteDdliClient (iddli)
int iddli;
{
	struct PLC **hplcddli = PdodDoc( docDde )->hplcddli;
	struct CA ca;
	struct DDLI ddli;

	DdeDbgCommInt ("DeleteDdliClient", iddli);
	Assert (docDde != docNil);
	Assert (iddli > iddliRequest && 
			iddli < IMacPlc( hplcddli ) - 1);
	GetPlc( hplcddli, iddli, &ddli );
	DeleteAtom (ddli.atomItem);
	Assert (hplcddli != hNil);
	CaFromIhdd (docDde, iddli, &ca);
	Assert (ca.doc == docDde);
	if (iddli == IMacPlc( hplcddli ) - 2)
		/* Deleteing the last one, really delete it */
		{
		ca.cpLim += ccpEop;
		if (!FDelete( &ca ))
			{
			ca.cpLim -= ccpEop;
			goto LMarkEmpty;
			}
		FOpenPlc (hplcddli, iddli, -1);
		}
	else
		/* just empty it and mark it unused */
		{
LMarkEmpty:
		ddli.dcl = dclNil;
		ddli.dls = dlsClear;
		ddli.atomItem = NULL;
		PutPlc( hplcddli, iddli, &ddli );
		FDelete( &ca );
		}
}


/* D C L  F R O M  A P P  T O P I C */
/*  Scan for an existing channel (not special or terminated) which is
	a conversation to app, topic.
*/
/*  %%Function:DclFromAppTopic %%Owner:peterj  */
DclFromAppTopic (atomApp, atomTopic)
ATOM atomApp, atomTopic;
{
	struct DCLD **hdcld;
	int dcl;

	for (dcl = 0; dcl < dclMac; dcl++)
		if ((hdcld = mpdclhdcld [dcl]) != hNil 
				&& (*hdcld)->dt == dtClient
				&& !(*hdcld)->fTerminating 
				&& (*hdcld)->atomApp == atomApp 
				&& (*hdcld)->atomTopic == atomTopic)
			return dcl;

	return dclNil;
}


