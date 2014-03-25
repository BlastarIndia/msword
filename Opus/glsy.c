/* glsy.c - glossary code */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "ch.h"
#include "wininfo.h"
#include "doc.h"
#include "debug.h"
#include "props.h"
#include "sel.h"
#include "file.h"
#include "opuscmd.h"
#include "field.h"
#include "message.h"

#include "error.h"
#include "rerr.h"

#include "idd.h"

#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"

#include "glsy.hs"
#include "glsy.sdm"


#ifdef PROTOTYPE
#include "glsy.cpt"
#endif /* PROTOTYPE */

#ifdef DEBUG
#ifdef PCJ
/* #define SHOWGLSYMSG */
#endif /* PCJ */
#endif /* DEBUG */

/* EXTERNALS */

extern int		wwCur;
extern HWND vhwndApp;
extern struct SEL selCur;
extern CHAR szEmpty[];
extern int docGlsy;
extern int docGlobalDot;
extern struct PREF vpref;
extern int mpdochdod [];
extern struct UAB vuab;
extern struct MERR vmerr;
extern BOOL vfSeeSel;
extern BOOL fElActive;
extern struct PAP		vpapFetch;
extern struct CA		caPara;

CP CpLimNoSpaces ();
int IglsyFromSt();




/* F  E N S U R E  G L S Y  I N I T E D */
/*
docGlsy is not opened up when the program is started to save time.
Thus, each command that needs the glossary must first check if it is
inititialized.
*/
/* %%Function:FEnsureGlsyInited %%Owner:krishnam */
FEnsureGlsyInited()
{
	int docDot = DocDotMother (selCur.doc);

	StartLongOp ();  /* possibly creating docs, can take a while */

	/* assure there is a global document type from which to hang docGlsy */
	Assert(docGlobalDot != docNil && PdodDoc(docGlobalDot)->fDot);

	/* assure there is a global glossary, docGlsy */
	if (docGlsy == docNil &&
			(docGlsy = PdodDoc (docGlobalDot)->docGlsy) == docNil &&
			(docGlsy = DocCreateGlsy (docGlobalDot)) == docNil)
		{
		EndLongOp (fFalse);
		return fFalse;
		}

	Assert(PdodDoc(docGlsy)->doc == docGlobalDot);
	Assert(PdodDoc(docGlobalDot)->docGlsy == docGlsy);

	/* if there is a document type, assure it has a docGlsy */
	if (docDot != docNil && PdodDoc (docDot)->docGlsy == docNil)
		DocCreateGlsy (docDot);

	EndLongOp (fFalse);
	return fTrue;
}


/* D O C  C R E A T E  G L S Y */
/*  Create a glossary document and link it to docMother. */
/* %%Function:DocCreateGlsy %%Owner:krishnam */
DocCreateGlsy(docMother)
int docMother;
{
	struct DOD **hdod;
	int doc;

	Assert (docMother != docNil);
	Assert (docMother == DocDotMother (docMother));

	doc = DocCreateSub (docMother, dkGlsy);
	if (doc == docNil)
		return docNil;

	hdod = mpdochdod [doc];

	/* initialize the glossary structures */
	Assert ((*hdod)->hsttbGlsy == hNil && (*hdod)->hplcglsy == hNil);
	if (HplcCreateEdc (hdod, edcGlsy) == hNil || 
			((*hdod)->hsttbGlsy = HsttbInit (0, fTrue/*fExt*/)) == hNil)
		{
		PdodDoc(docMother)->docGlsy = docNil;
		DisposeDoc(doc);
		return docNil;
		}

	Assert ((*hdod)->hsttbGlsy && (*hdod)->hplcglsy);

	return doc;

}


/*  D O C   G L S Y  L O C A L */
/* %%Function:DocGlsyLocal %%Owner:krishnam */
DocGlsyLocal ()
{
	int docDot = DocDotMother (selCur.doc);

	if (docDot == docNil || docDot == docGlobalDot)
		return docNil;
	else
		return PdodDoc (docDot)->docGlsy;

}



/* C M D  D O  E X P A N D  G L S Y */
/*
Replace the selected text with the glsy entry iglsy in doc.
*/
/* %%Function:CmdDoExpandGlsy %%Owner:krishnam */
CMD CmdDoExpandGlsy(doc, iglsy)
int doc, iglsy;
{
	char ch;
	struct CA caGlsy, caSel;
	char st[cchGlsyMax];
	BOOL fAddPara = fFalse;

	CaFromIhdd(doc, iglsy, &caGlsy);
	if (!FDeleteCheck(fTrue, rpkCa, &caGlsy))
		return cmdError;

	if (!FSetUndoBefore(bcmExpandGlsy, uccPaste))
		return cmdCancelled;

	TurnOffSel(&selCur);
	caSel = selCur.ca;

	if (!FMoveOkForTable(selCur.doc, selCur.cpFirst, &caGlsy))
		{
		ErrorEid(eidIllegalTextInTable, "CmdDoExpandGlsy");
		return cmdError;
		}

	if (FInTableDocCp(caGlsy.doc, caGlsy.cpFirst))
		{
		struct CA caT;

		if (!FEopBeforePca(PcaPoint(&caT, caSel.doc, caSel.cpFirst)))
			return cmdNoMemory;

		if (caT.cpFirst != caSel.cpFirst)
			{
			fAddPara = fTrue;
			caSel.cpFirst += ccpEop;
			caSel.cpLim += ccpEop;
			}
		}

	if (!FReplaceCpsRM(&caSel, &caGlsy))
		return cmdNoMemory;

	CopyStylesFonts(doc, caSel.doc, caSel.cpFirst, DcpCa(&caGlsy));

	caSel.cpLim += DcpCa(&caGlsy);
	if (fAddPara)
		caSel.cpFirst -= ccpEop;
	AssureNestedPropsCorrect(&caSel, fTrue);

/* "Go back" points before and after the insertion */
	SelectIns( &selCur, caSel.cpFirst );
	PushInsSelCur();
	SelectIns( &selCur, CpMin(CpMacDocEdit(selCur.doc), caSel.cpLim) );
	PushInsSelCur();

	DirtyDoc (selCur.doc);

	SetUndoAfter(&caSel);
	vfSeeSel = fTrue;
	return cmdOK;
}


/* D E F I N E  G L S Y */
/*
set the glsy entry in doc to the text represented by pca.
*/
/* %%Function:DefineGlsy %%Owner:krishnam */
DefineGlsy(doc, iglsy, pca)
int doc, iglsy;
struct CA *pca;
{
	struct CA caOld, caT;
	CP cpCur, dcp, cpLim;
	int fTable;

	/* set undo for define would allow incomplete, inconsistant
		undo interface since undefine cannot be undone
	*/

	AssureLegalSel(pca);
	CaFromIhdd(doc, iglsy, &caOld);
	ReplaceIhdd(doc, iglsy, pca);
	cpCur = caOld.cpFirst;
	dcp = DcpCa(pca);
	fTable = fFalse;

	cpLim = caOld.cpFirst + dcp;
	while (cpCur < cpLim)
		{
		CachePara(doc, cpCur);
		if (vpapFetch.fTtp)
			goto LExit;
		if (FInTableVPapFetch(doc, cpCur))
			fTable = fTrue;
		cpCur = caPara.cpLim;
		}
	if (fTable)
		ApplyTableProps(PcaSetDcp(&caT, doc, caOld.cpFirst, dcp), fFalse);

LExit:
	DirtyDoc(doc);/* new glsy inserted to glsy doc */
	ClearFDifferFltg (doc, fltgAll); /* fields in default mode */
}


/* I G L S Y  C R E A T E */
/*
Makes sure entry with name st exists. New glsy is created if necessary
with empty contents.
Returns iglsy of entry with name st.
*/

/* %%Function:IglsyCreate %%Owner:krishnam */
int IglsyCreate(doc, pst)
int doc;
char *pst;
{
	int iglsy;

	if (*pst >= cchGlsyMax)
		*pst = cchGlsyMax -1;
	if (!FSearchGlsy(doc, pst, &iglsy))
		{
		if (!FInsStInSttb(PdodDoc(doc)->hsttbGlsy, iglsy, pst)
				|| !FInsertIhdd(doc, iglsy))
			return iNil;
		}
	return iglsy;
}


/* D E L E T E  G L S Y */
/* %%Function:DeleteGlsy %%Owner:krishnam */
DeleteGlsy(doc, ist, pca)
int doc, ist;
struct CA *pca;
{
	struct CA caT;

	DeleteIFromSttb(PdodDoc(doc)->hsttbGlsy, ist);
	caT = *pca;
	caT.cpLim += ccpEop;
	if (FDelete(&caT))
		FOpenPlc(PdodDoc(doc)->hplchdd, ist, -1);
	DirtyDoc (doc); /* glsy deleted from glsy doc */
}


/* I G L S Y  F R O M  S T */
/*
Look for a named glossary entry and return its index, or iNil if
it is not found.
*/
/* %%Function:IglsyFromSt %%Owner:krishnam */
int IglsyFromSt(doc, st)
int doc;
char *st;
{
	int iglsy;

	if (doc != docNil && FSearchGlsy(doc, st, &iglsy))
		return iglsy;
	else
		return iNil;
}


/* F  S E A R C H  G L S Y */
/*
Binary-search the glossary's string table returning true if the
string is found; in any case return the index of where the string
would be in the table at *piglsy.
*/
/* %%Function:FSearchGlsy %%Owner:krishnam */
FSearchGlsy(doc, st, piglsy)
int doc;
char *st;
int *piglsy;
{
	extern int WCompSt();

	if (st[st[0]] == chEop)
		st[0] -= cchEop;

	return FSearchSttb(PdodDoc(doc)->hsttbGlsy, st, piglsy, WCompSt);
}


/* I  F R O M  T E X T */
/*
General Utility.  return an index based on text in the current document.
If there is a selection, it is used as the text.  Otherwise gather words
back from the insertion point until a valid st is found or cchMax - 1
characters have been read.
*/
/* %%Function:IFromText %%Owner:krishnam */
IFromText (doc, pfnIFromDocSt, cchMax)
int doc;
PFN pfnIFromDocSt;
int cchMax;
{
	int cch, i = iNil;
	CP cpLim, cpFirst, cpFirstMin;
	struct CA ca;
	CHAR st [cchMaxSz];

	Assert (cchMaxSz >= cchMax);

	if (doc == docNil || selCur.cpLim == cp0 || selCur.doc == docNil)
		return iNil;

	if (selCur.sk == skSel)
		/* the selection is not an insertion point, use it as name */
		{
		FetchRgch (&cch, &st[1], selCur.doc, selCur.cpFirst,
				CpLimNoSpaces (&selCur.ca), cchMax-1);
		if ((st[0] = (CHAR) cch) > 0)
			i = (*pfnIFromDocSt) (doc, st);
		}

	else
		/* the selection is an insertion point or other.  work back word by word until
			either we get a valid name or we have gone back cchMax chars */
		{
		ca = selCur.ca;
		ca.cpFirst = cp0;
		cpLim =  cpFirst = CpLimNoSpaces (&ca);
		cpFirstMin = CpMax (cp0, (CP)(cpFirst - (cchMax - 1)));

		while ( i == iNil && --cpFirst >= cpFirstMin &&
				(cpFirst = CpFirstSty (wwCur, selCur.doc, cpFirst, styWord, fFalse /*fend*/))
				>= cpFirstMin)
			{
			FetchRgch (&cch, &st[1], selCur.doc, cpFirst, cpLim, cchMax -1);
			st[0] = (CHAR) cch;
			i = (*pfnIFromDocSt) (doc, st);
			}
		if (i != iNil)
			/* select the name we found */
			Select (&selCur, cpFirst, cpLim);
		}

	return i;

}


/* F  I G L S Y  D O C  F R O M  S T */
/* %%Function:FIglsyDocFromSt %%Owner:krishnam */
FIglsyDocFromSt (piglsy, pdoc, st)
int *piglsy, *pdoc;
CHAR *st;
{
	int doc, iglsy;

	doc = DocGlsyLocal();

	if ( (iglsy = IglsyFromSt (doc, st)) == iNil)
		iglsy = IglsyFromSt (doc = docGlsy, st);

	*piglsy = iglsy;
	*pdoc = doc;

	return iglsy != iNil;
}


/* U N D E F I N E  G L S Y */
/* %%Function:UndefineGlsy %%Owner:krishnam */
UndefineGlsy (doc, iglsy)
int doc, iglsy;
{
	struct CA ca;

	if (doc == docNil)
		return;

	CaFromIhdd (doc, iglsy, &ca);
	DeleteGlsy (doc, iglsy, &ca);
}


/*  C M D  E X P A N D   G L S Y  */
/*
command to collect a string, look it up in the docGlsy name table and
expand the glsy entry, if any.
*/
/* %%Function:CmdExpandGlsy %%Owner:krishnam */
CMD CmdExpandGlsy(pcmb)
CMB * pcmb;

{
	int iglsy;
	int doc;

	if (!FEnsureGlsyInited())
		return cmdError;

	doc = DocGlsyLocal();
	if ((iglsy = IFromText (doc, IglsyFromSt, cchGlsyMax)) != iNil ||
			(iglsy = IFromText (doc = docGlsy, IglsyFromSt, cchGlsyMax))
			!= iNil)
		return CmdDoExpandGlsy(doc, iglsy);
	else
		{
		ErrorEid(eidNoGlsy, " CmdExpandGlsy");
		return cmdError;
		}
}




/* C M D  G L O S S A R Y */
/* %%Function:CmdGlossary %%Owner:krishnam */
CMD CmdGlossary (pcmb)
CMB * pcmb;
{
	if (!FEnsureGlsyInited())
		return cmdError;

	if (FCmdFillCab())
		{
		if (!FSetCabSz(pcmb->hcab, szEmpty,
				Iag(CABGLOSSARY, hszGlossary)))
			{
			return cmdNoMemory;
			}
		}

	if (FCmdDialog())
		{
		char dlt [sizeof (dltGlossary)];

		/* make the selection invisible while in the dialog */
		TurnOffSel(&selCur);

		BltDlt(dltGlossary, dlt);

		/* do the dialog */
		switch (TmcOurDoDlg(dlt, pcmb))
			{
#ifdef DEBUG
		default:
			Assert(fFalse);
			return cmdError;
#endif

		case tmcError:
			return cmdNoMemory;

		case tmcCancel: /* or Close, return cmdOK! */
		case tmcInsert:
		case tmcDefine:
			break;
			}
		}

	if (pcmb->fAction)
		{
		switch (pcmb->tmc)
			{
			int doc, iglsy;
			char st [cchGlsyMax + 1];

		case tmcUndefine:
			GetCabSt(pcmb->hcab, st, cchGlsyMax, 
					Iag(CABGLOSSARY, hszGlossary));

			if (!FIglsyDocFromSt(&iglsy, &doc, st))
				{
				ErrorEid(eidNoGlsy, " CmdGlossary (undef)");
				return cmdError;
				}

			UndefineGlsy(doc, iglsy);
			break;

		case tmcOK:
		case tmcInsert:
			/* insert text of glossary into document */
			GetCabStz(pcmb->hcab, st, cchGlsyMax + 1, 
					Iag(CABGLOSSARY, hszGlossary));
			if (!FIglsyDocFromSt(&iglsy, &doc, st))
				{
				ErrorEid(eidNoGlsy, " CmdGlossary (insert)");
				return cmdError;
				}

			if (selCur.fTable || selCur.fBlock)
				{
				Beep();
				return cmdError;
				}

			if (!vpref.fAutoDelete)
				SelectIns(&selCur,selCur.cpFirst);

			return CmdDoExpandGlsy(doc, iglsy);

		case tmcDefine:
			/* define */
			if (selCur.fIns)
				{
				Assert(fElActive);
				goto LModeError;
				}

			GetCabStz(pcmb->hcab, st, cchGlsyMax + 1, 
					Iag(CABGLOSSARY, hszGlossary));
			doc = ((CABGLOSSARY *) *pcmb->hcab)->fLocal ? 
					DocGlsyLocal() : docGlsy;
			if (doc == docNil)
				{
				if (fElActive)
					{
LModeError:
					RtError(rerrModeError);
					}
				else	
					ErrorNoMemory(eidNoMemGlsy);
				return cmdError;
				}
			iglsy = IglsyCreate(doc, st);
			if (iglsy == iNil)
				return cmdNoMemory;

			DefineGlsy(doc, iglsy, &selCur.ca);
			break;
			}
		}

	return cmdOK;
}


/* %%Function:ElSetGlossary %%Owner:krishnam */
ElSetGlossary (hstName, hstText, fDot)
char ** hstName, ** hstText;
BOOL fDot;
{
	char stText[ichMaxBufDlg];
	char stName[cchGlsyMax + 1];
	struct CA caTmp;
	int doc, iglsy;
	struct CHP chp;
	extern int vdocTemp;

	/* Copy to local buffers to prevent problems with heap movement */
	CopySt(*hstText, stText);
	CopySt(*hstName, stName);

	if (!FEnsureGlsyInited())
		{
		RtError(rerrOutOfMemory);
		/* NOT REACHED */
		Assert(fFalse);
		}

	if (DocCreateTemp(docScrap) == docNil) /* mother doc irrelevant */
		goto LNoMem;

	/* Insert text into temp document */
	StandardChp(&chp);
	Assert(vdocTemp != docNil);
	if (!FInsertRgch(vdocTemp, cp0, &stText[1], stText[0], &chp, 0))
		goto LNoMem;
	caTmp.cpFirst = cp0;
	caTmp.cpLim = stText[0];
	caTmp.doc = vdocTemp;

	if ((doc = fDot ? DocGlsyLocal() : docGlsy) == docNil ||
		 PdodDoc(doc)->fLockForEdit)
		{
		RtError(rerrModeError);
		return;
		}

	iglsy = IglsyCreate(doc, stName);
	if (iglsy == iNil)
		{
LNoMem:
		ErrorNoMemory(eidNoMemGlsy);
		return;
		}

	DefineGlsy(doc, iglsy, &caTmp);
}



/* This number needs to be at least as big as the banter */
#define cchMaxGlsyBanter  45


/* F  D L G  G L O S S A R Y */
/* %%Function:FDlgGlossary %%Owner:krishnam */
BOOL FDlgGlossary(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	int doc, iglsy;
	CHAR st [cchGlsyMax+1];

#ifdef SHOWGLSYMSG
	CommSzNum ("FDlgGlossary: dlm = ", dlm);
	CommSzNum ("FDlgGlossary: tmc = ", tmc);
#endif /* SHOWGLSYMSG */

	switch (dlm)
		{
	case dlmInit:
		/* show the selection text, disable controls that
			require some text in EC */
		ShowGlsyOrSelection(docNil, 0, NULL);
		EnableTmc(tmcUndefine, fFalse);
		EnableTmc(tmcInsert, fFalse);
		EnableTmc(tmcDefine, fFalse);
		SetTmcVal(tmcGlobal, docGlsy != docNil ? 0 :
				DocGlsyLocal() != docNil ? 1 : uNinch);
		EnableTmc(tmcGlobal, docGlsy != docNil);
		EnableTmc(tmcTemplate, DocGlsyLocal() != docNil);
		break;

	case dlmClick:
		if (tmc == tmcUndefine)
			{
			CmdDlgAction(CmdGlossary, tmc);
			FillTmcGlossary(tmcGlossary, fTrue);
			SetTmcText(tmcGlossary, szEmpty);
			SetFocusTmc(tmcGlossary);
			ChangeCancelToClose();
			goto LCheckEnable;
			}
		break;

	case dlmChange:
		/* the text in the EC has changed.  if it is the name of a 
			glsy, display the text of the glossary.  set buttons 
			that depend on it */
		if (tmc == (tmcGlossary & ~ftmcGrouped))
			{
			int fDefined, fNonEmpty, fValidSel;
LCheckEnable:
			fDefined = fNonEmpty = fFalse;
			fValidSel = selCur.fTable ? FWholeRowsPsel(&selCur) :
					!(selCur.sk&skNotGlsy);
			*st = CchGetTmcText(tmcGlossary, st+1, cchGlsyMax);
			if ((fNonEmpty = *st > 0) &&
					(fDefined = FIglsyDocFromSt(&iglsy, &doc, st)))
				{
				ShowGlsyOrSelection(doc, iglsy, st);
				}
			else
				{
				ShowGlsyOrSelection(docNil, 0, NULL);
				}

			EnableTmc(tmcUndefine, fDefined);
			EnableTmc(tmcInsert, fDefined);
			EnableTmc(tmcDefine, fNonEmpty && fValidSel &&
					(DocGlsyLocal() != docNil || docGlsy != docNil));

			if (fDefined)
				{
				Assert (vmerr.fMemFail || FEnabledTmc(tmcInsert)); /* OOM mesage displayed later */
				SetDefaultTmc(tmcInsert);
				}
			/* name not defined but we have one */
			else  if (fNonEmpty && fValidSel)
				{
				Assert (vmerr.fMemFail || FEnabledTmc(tmcDefine)); /* OOM message displayed later */
				SetDefaultTmc(tmcDefine);
				}
			else
				SetDefaultTmc(tmcCancel);
			}
		break;

	case dlmTerm:
		if (tmc == tmcDefine)
			/* see if name already in use */
			{
			*st = CchGetTmcText(tmcGlossary, st+1, cchGlsyMax);
			doc = ValGetTmc(tmcGlsyContext) ? DocGlsyLocal() : docGlsy;
			if (IglsyFromSt(doc, st) != iNil &&
					IdMessageBoxMstRgwMb(mstRedefineGlsy, NULL, 
					MB_YESNO | MB_APPLMODAL | MB_DEFBUTTON1 |
					MB_ICONQUESTION) != IDYES)
				{
				SetTmcTxs(tmcGlossary, TxsOfFirstLim(0, ichLimLast));
				return fFalse;
				}
			}

		/* check to see that the glossary name is valid */
		/* what are the criteria (if any) ? */
		break;
		}

	return fTrue;
}


/* %%Function:WListGlsy %%Owner:krishnam */
EXPORT WORD WListGlsy(tmm, sz, isz, filler, tmc, wParam)
TMM tmm;
char * sz;
int isz;
WORD filler;
TMC tmc;
WORD wParam;
{

	/* this function is only used the first time the box is filled */
	if (tmm == tmmCount)
		FillTmcGlossary(tmcGlossary, fTrue);

	return 0;
}


/* F I L L  T M C   G L O S S A R Y */
/* %%Function:FillTmcGlossary %%Owner:krishnam */
FillTmcGlossary (tmc, fTmcCombo)
TMC tmc;
BOOL fTmcCombo;
/*
Add local then global glossary names to the listbox of tmcGlossary.
Global glossaries with the same name as a local glossary are not shown.
This can be called for the glossary dialog and also for the InsertField
dialog.  The latter has a straight list box to fill, and the former has
a combo box.  fTmcCombo is true if this tmc is for a combo box, because
we have to do the funky thing with ~ftmcGrouped in that case only.
*/
{
	int docGlsyLocal = DocGlsyLocal();
	struct STTB **hsttbLocal = (docGlsyLocal == docNil) ? hNil :
	PdodDoc (docGlsyLocal)->hsttbGlsy;
	struct STTB **hsttbGlobal = (docGlsy == docNil) ? hNil :
	PdodDoc (docGlsy)->hsttbGlsy;

	SetTmcVal((fTmcCombo ? ((tmc & ~ftmcGrouped) + 1) : tmc), uNinchList);
	StartListBoxUpdate(tmc); /* empties box and starts redraw */
	AddSttbToLBNew(tmc, hsttbLocal, hNil);
	AddSttbToLBNew(tmc, hsttbGlobal, hsttbLocal);
	EndListBoxUpdate(tmc);
}


csconst CHAR stSelection[] = StSharedKey("Selection",Selection);
csconst CHAR stNoSelection[] = StSharedKey("(No Selection)",NoSelection);

/* S H O W  G L S Y  O R  S E L E C T I O N */
/*  Show glossary doc, iglsy or the current selection in tmcGlsyText.
*/

/* %%Function:ShowGlsyOrSelection %%Owner:krishnam */
ShowGlsyOrSelection (doc, iglsy, stGlsy)
int doc, iglsy;
CHAR *stGlsy;
{

	int cch;
	struct CA ca;
	CHAR rgchText [cchMaxGlsyBanter];
	CHAR rgchCleanText [cchMaxGlsyBanter];
	CHAR rgchGlsy [cchGlsyMax+1];

	if (doc == docNil)
		/* show selection */
		{
		int fValidSel = selCur.fTable ? FWholeRowsPsel(&selCur) :
		!(selCur.sk&skNotGlsy);
		bltbx((CHAR FAR *)&stSelection[1], (CHAR FAR *)rgchGlsy,
				cch = stSelection[0]);
		rgchGlsy[cch] = '\0';
		if (fValidSel && selCur.doc != docNil)
			FetchRgch (&cch, rgchText, selCur.doc, selCur.cpFirst,
					selCur.cpLim, cchMaxGlsyBanter - 1);
		else
			bltbx((CHAR FAR *)&stNoSelection[1], (CHAR FAR *)rgchText, 
					cch = stNoSelection[0]);
		}
	else
		/*  fill with doc,iglsy */
		{
		Assert (stGlsy != NULL);
		StToSz (stGlsy, rgchGlsy);
		rgchGlsy [*stGlsy] = ':';
		rgchGlsy [*stGlsy+1] = 0;
		CaFromIhdd (doc, iglsy, &ca);
		Assert (doc == ca.doc);
		FetchRgch (&cch, rgchText, ca.doc, ca.cpFirst, ca.cpLim,
				cchMaxGlsyBanter - 1);
		}

	rgchText [cch] = '\0';
	SanitizeSz (rgchText, rgchCleanText, cchMaxGlsyBanter, fTrue);
	FTruncateTmcSz(tmcGlsyName, rgchGlsy);
	FTruncateTmcSz(tmcGlsyText, rgchCleanText);
	SetTmcText (tmcGlsyName, rgchGlsy);
	SetTmcText (tmcGlsyText, rgchCleanText);
}



/* A D D  S T T B  T O  L B */
/*
General Utility. add all strings in hsttb which are not in hsttbExclude
to hwndLB.  Handles hsttb == hNil.  hsttbExclude may be hNil (exclude
nothing).
*/
/* %%Function:AddSttbToLBNew %%Owner:krishnam */
AddSttbToLBNew(tmc, hsttb, hsttbExclude)
TMC tmc;
struct STTB **hsttb, **hsttbExclude;
{
	int ibstMac, ibst, ibstExclude;
	CHAR st[cchMaxSt];
	extern int WCompSt();

	if (hsttb == hNil)
		return;

	ibstMac = (*hsttb)->ibstMac;

	for (ibst = 0; ibst < ibstMac; ibst++)
		{
		GetStFromSttb(hsttb, ibst, st);
		/* insert only if it is not in hsttbExclude */
		if (!FSearchSttb (hsttbExclude, st, &ibstExclude, WCompSt))
			{
			StToSzInPlace(st);
			AddListBoxEntry(tmc, st);
			}
		}
}


/* C M D  S P I K E */
/* %%Function:CmdSpike %%Owner:krishnam */
CMD CmdSpike (pcmb)
CMB * pcmb;

{
	int iglsy;
	CP dcp, dcpSel;
	struct CA caGlsy, caGlsyEnd, caSel, caSelEop;

	if (selCur.fTable ? !FWholeRowsPsel(&selCur) : selCur.sk&skNotGlsy)
		{
		Beep();
		return cmdError;
		}

	if (!FEnsureGlsyInited ())
		return cmdError;

	Assert (selCur.doc != docNil);

	if (!FDeleteCheck(fFalse /* fPaste */, rpkText, NULL))
		{
		return cmdError;
		}
	caSel = selCur.ca;

	/* get the index of the new glossary, or an existing one */
	iglsy = IglsyCreate (docGlsy, StSharedKey("Spike",Spike));
	if (iglsy == iNil)
		return cmdNoMemory;

	SetUndoNil();

	TurnOffSel(&selCur);

	/* get the ca of the existing spike glossary */
	CaFromIhdd (docGlsy, iglsy, &caGlsy);

	/* append an EOP to the end of the selection */
	PcaPoint( &caSelEop, caSel.doc, caSel.cpLim );
	if (!FReplace( &caSelEop, fnSpec, (FC)fcSpecEop, (FC)ccpEop))
		return cmdNoMemory;
	caSel.cpLim += ccpEop;
	caSelEop.cpLim += ccpEop;

	PdodDoc(docGlobalDot)->fDirty = fTrue;

	/* append the new text, w/EOP, to the glossary */
	PcaPoint( &caGlsyEnd, caGlsy.doc, caGlsy.cpLim );
	if (!FReplaceCps( &caGlsyEnd, &caSel ))
		return cmdNoMemory;
	CopyStylesFonts (selCur.doc, docGlsy, caGlsy.cpLim, DcpCa(&caSel));
	caGlsyEnd.cpLim += DcpCa(&caSel) - ccpEop;
	AssureNestedPropsCorrect(&caGlsyEnd, fTrue);

	/* nuke the added EOP in case the following delete doesn't really delete
		* anything (because of revision marking) */
	FDelete( &caSelEop );
	caSel.cpLim -= ccpEop;

	/* delete the text from the current document */
	FDeleteRM( &caSel );

	/* make the selection something reasonable */
	SelectIns(&selCur, caSel.cpFirst);

	return cmdOK;
}


/* C M D  U N S P I K E */
/* %%Function:CmdUnspike %%Owner:krishnam */
CMD CmdUnspike (pcmb)
CMB * pcmb;

{
	int iglsy;
	struct CA caGlsy;
	struct CA caIns, caRM;
	CMD cmd;
	BOOL fAddPara = fFalse;

	Assert (selCur.doc != docNil);

	if (selCur.fTable || selCur.fBlock)
		{
		Beep();
		return cmdError;
		}

	if (!FEnsureGlsyInited ())
		return cmdError;

	if ((iglsy = IglsyFromSt (docGlsy, StSharedKey("Spike",Spike))) == iNil)
		{
		/* there is no spike glossary to unspike */
		Beep ();
		return cmdError;
		}

	SetUndoNil();

	TurnOffSel(&selCur);

	/* get the source ca */
	CaFromIhdd (docGlsy, iglsy, &caGlsy);

	if (!FMoveOkForTable(selCur.doc, selCur.cpFirst, &caGlsy))
		{
		ErrorEid(eidIllegalTextInTable, "CmdDoExpandGlsy");
		return cmdError;
		}

	if ((cmd = CmdAutoDelete(&caRM)) != cmdOK)
		return cmd;

	/* move the spike glsy in place of the selection */
	caIns = selCur.ca;
	if (FInTableDocCp(caGlsy.doc, caGlsy.cpFirst))
		{
		struct CA caT;

		if (!FEopBeforePca(PcaPoint(&caT, caIns.doc, caIns.cpFirst)))
			return cmdNoMemory;

		if (caT.cpFirst != caIns.cpFirst)
			{
			fAddPara = fTrue;
			caIns.cpFirst += ccpEop;
			caIns.cpLim += ccpEop;
			}
		}

	if (!FReplaceCpsRM( &caIns,  &caGlsy ))
		return cmdNoMemory;
	caIns.cpLim = caIns.cpFirst + DcpCa(&caGlsy);
	CopyStylesFonts (caGlsy.doc, caIns.doc, caIns.cpFirst, DcpCa(&caIns));

	if (fAddPara)
		caIns.cpFirst -= ccpEop;

	/*  make sure dead field property preserved */
	AssureNestedPropsCorrect(&caIns, fTrue);

	/* get a reasonable selection */
	SelectIns( &selCur, caIns.cpLim );

	/* delete the text of the spike glossary */
	/* the glossary itself is not deleted, allows for undo */
	FDelete( &caGlsy );

	DirtyDoc(selCur.doc);

	return cmdOK;
}


