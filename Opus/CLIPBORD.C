
/* clipbord.c -- Cut/Paste to clipboard */

#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOCTLMGR
#define NOFONT
#define NOPEN
#define NOBRUSH
#define NOCOMM
#define NOWNDCLASS
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "doc.h"
#include "props.h"
#include "sel.h"
#include "pic.h"
#include "file.h"
#include "disp.h"
#include "screen.h"
#include "prm.h"
#include "opuscmd.h"
#include "debug.h"
#include "format.h"

#include "field.h"
#include "ch.h"
#include "error.h"



extern struct SEL	selCur;      /* Current selection */
extern int		vfSeeSel;
extern struct DOD	**mpdochdod[];
extern struct PAP	vpapFetch;
extern CP		vcpLimParaCache;
extern CP		vcpFirstParaCache;
extern struct UAB	vuab;
extern struct FCB	(**hpfnfcb)[];
extern struct MERR	vmerr;
extern HWND		vhwndApp;	   /* handle to parent's window */
extern struct SAB	vsab;
extern struct STTB  **vhsttbFont;
extern struct PREF      vpref;
extern int		cfRTF;
extern int		cfLink;

extern CHAR HUGE        *vhpchFetch;
extern struct CHP	vchpFetch;
extern struct CA	caPara;
extern CP		vcpFetch;
extern int		fnFetch;
extern struct PIC	vpicFetch;
extern int              wwCur;
extern int              vwWinVersion;

#ifdef NOTUSED
/*  %%Function: CmdClear  %%Owner: notused  */

CMD CmdClear()	 /* not implemented [yet?][ever?] */
{

	Assert(fFalse);

	return cmdOK;
}


#endif

/*  %%Function:  CmdCopy  %%Owner:  bobz       */

CMD CmdCopy(pcmb)
CMB *pcmb;
{		/* COPY command: copy selection to clipboard */
	CMD cmd = cmdOK;

	if (selCur.fBlock)
		{
		/* use FDelCkBlockOk to test for partial fields
			in block, which are disallowed */
		if (!FDelCkBlockOk(rpkNil, NULL))
			return cmdError;
		}

	if (!FSetUndoBefore(bcmCopy, uccCopy))
		return cmdCancelled;

	vsab.doc = docNil;

	if (selCur.fTable)
		cmd = CmdCopyTable(&selCur, docScrap);
	else  if (selCur.fBlock)
		ClBlockSelToScrap();
	else
		CmdCopy1(&selCur, &selCur.ca);

	SetUndoAfter(NULL);

	return cmd;
}


/* C M D  C O P Y  1 */
/* just sets the scrap to a plain sel */
/*  %%Function:  CmdCopy1  %%Owner:  bobz       */

CmdCopy1(psel, pca)
struct SEL *psel;	/* describes state of txt in pca */
struct CA *pca;	/* text to be copied */
{
	struct CA caT;

	SetWholeDoc(docScrap, pca);
	AssureNestedPropsCorrect(PcaSetWholeDoc(&caT, docScrap), fTrue);
	vsab.docStsh = DocMother(pca->doc);
	vsab.fFormatted = PdodDoc(vsab.docStsh)->fFormatted;
	vsab.fMayHavePic = PdodMother(vsab.docStsh)->fMayHavePic;
	vsab.caCopy = *pca;  /* for (dde) Link format */
	vsab.fTable = fFalse;
	/* Need to see if we copied part of a table cell.  If so, the
		fInTable flag needs to be turned off */
	if (psel->fWithinCell)
		ApplyTableProps(PcaSetWholeDoc(&caT, docScrap), fFalse);
	ChangeClipboard();     /* Force repaint of clipboard display &
				Set ownership. also sets vsab.fPict */
}


#ifdef DEBUG
int vTestClip = 0;
#endif /* DEBUG */

/* C M D  P A S T E */
/* paste command on the edit menu
*/
/*  %%Function:  CmdPaste  %%Owner:  bobz       */

CMD CmdPaste(pcmb)
CMB *pcmb;
{
	HWND hWndClipOwner;
	struct CHP chpSel;
	struct CA caScrap;
	struct CA caPaste;
	struct CA caRM;
	struct CA caT;
	CMD cmd;
	BOOL fAddPara = fFalse;
	CP cpFirst;

	StartLongOp();	/* Put up an hourglass */

/* We assume multiple Opus instances are not possible - paste cannot
	work between instances of Opus */

#ifdef DEBUG
	/* a slimy way to let us test RTF clipboard stuff and let us use
		slapWnd to put stuff into the clipboard.	If true, we will
		paste out of the clipboard even though we put it there ourselves.
	*/
	if (vTestClip)
		goto ForceClipRead;
#endif /* DEBUG */


	if ( (hWndClipOwner = GetClipboardOwner()) != vhwndApp )
		{
#ifdef DEBUG
ForceClipRead:
#endif
		if (!FReadExtScrap())	/* clipbrd contents --> docScrap */
			{
			SetErrorMat( matLow );
			cmd = cmdError;
			goto PasteEnd;
			}
		}

/* docScrap now has clipboard contents; paste them in */

	/*  Here we reoprt and error if we try to paste into a block selection.
		We are specifically not pasting into a block because it can be
		really disorienting, especially if autodelete is on. We
		generally do not autodelete blocks, but require explicit
		deletion. You can paste a block into a normal selection,
		however. (We could, I suppose, turn the block to an insert
		point then paste, but the autodelete case would be strange. (bz) */

	if (!DcpCa(PcaSetWholeDoc( &caScrap, docScrap)) ||
			selCur.fBlock && !selCur.fTable)
		{
		ErrorEid(eidInvalidSelection, "CmdPaste");
		cmd = cmdError;
		goto  PasteEnd;
		}


	PcaPoint(&caRM, selCur.doc, selCur.ca.cpFirst);
	/* Are we pasting to a table?? */
	if (selCur.fWithinCell || selCur.fTable)
		{
LTablePaste:
		cmd = CmdPasteToTable();
		goto PasteEnd;
		}
	else  if (vsab.fTable && vpref.fAutoDelete && (selCur.cpFirst == cp0 ||
			FInTableDocCp(selCur.doc, selCur.cpFirst-1)) &&
			FInTableDocCp(selCur.doc, selCur.cpLim))
		{
		ErrorEid(eidCantPasteTableInTable, "CmdPaste");
		cmd = cmdError;
		goto  PasteEnd;
		}
	else  if (!vpref.fAutoDelete && FInTableDocCp(selCur.doc, selCur.cpFirst))
		{
		/* selection begins in a table and contains text outside the table */
		TurnOffSel(&selCur);
		SelectIns(&selCur, selCur.cpFirst);
		goto LTablePaste;
		}
	else  if (vsab.fBlock)
		{
		cmd = CmdPasteBlock();
		goto PasteEnd;
		}
	else
		{
#ifdef DAVIDBO
		CommSzNum(SzShared("\tBefore doc:\t"), caRM.doc);
		CommSzLong(SzShared("\tBefore cpFirst:\t"), caRM.cpFirst);
		CommSzLong(SzShared("\tBefore cpLim:\t"), caRM.cpLim);
#endif /* DAVIDBO */
		if (!FSetUndoBefore(bcmPaste, uccPaste))
			return cmdCancelled;
		TurnOffSel(&selCur);
		if ((cmd = CmdAutoDelete(&caRM)) != cmdOK)
			goto PasteEnd;
		if (!vsab.fFormatted)  /* save current char props to apply
		to inserted text */
			{
			GetSelCurChp(fFalse);
			chpSel = selCur.chp;
			}

		if (FInTableDocCp(docScrap, cp0))
			{
			if (!FEopBeforePca(PcaPoint(&caT, selCur.doc, cpFirst = selCur.cpFirst)))
				{
				cmd = cmdNoMemory;
				goto PasteEnd;
				}

			if (caT.cpFirst != cpFirst)
				{
				fAddPara = fTrue;
				selCur.cpFirst += ccpEop;
				if (selCur.fIns)
					selCur.cpLim += ccpEop;
				}
			}

		/* must do after selCur adjusted for fAddPara */
		PcaSetDcp( &caPaste, selCur.doc, selCur.cpFirst, DcpCa( &caScrap ));

		/* do not pass &selCur.ca; AdjustCp might munge it */
		caT = selCur.ca;

		if (!FReplaceCpsRM( &caT, &caScrap ))
			{
			cmd = cmdNoMemory;
			goto PasteEnd;
			}

#ifdef DCLIPPIC
		if (vsab.fPict)
			{
			CachePara(selCur.doc, selCur.cpFirst);
			FetchCp(selCur.doc, selCur.cpFirst, fcmProps);
			CommSzRgNum(SzShared("vchpFetch after insert pic repl cps : "), &vchpFetch, cwCHP);
			}
#endif
		if (vsab.fFormatted)
			{
			CopyStylesFonts( vsab.docStsh != docNil ? vsab.docStsh : docScrap,
					caPaste.doc, caPaste.cpFirst, DcpCa( &caPaste )  );
			}
		else  /* picture or unformatted text from clipboard */		
			{
			int cb;
			CHAR grpprl[cbMaxGrpprl];
			struct CHP chpT;

		/* copy char looks.
			compute difference: chpT is StandardChp, which
			docScrap got when data was read in from clipboard.
			chpSel was the previous selection props.
			Express difference as grpprl.
			Apply grpprl.
			picture notes:
			for a picture, the char props are standard except for
			fSpec and fcPic being set. If we also add in the
			current properties at the insert location now, we don't
			have to special case the picture when we want char props
			and the pic is the start or total of a selection.
		*/
			StandardChp(&chpT);
			/* cb == 0 if no differences */
			if (cb = CbGrpprlFromChp(&grpprl, &chpSel, &chpT))
				ApplyGrpprlCa(grpprl, cb, &caPaste);
			}
		}

	/* this will make saves be native if formerly plain */
	if (vsab.fPict)
		PdodDoc(selCur.doc)->fFormatted = fTrue;
	if (vsab.fMayHavePic)
		PdodMother(selCur.doc)->fMayHavePic = fTrue;

/* "Go back" points before and after the insertion */
	SelectIns( &selCur, caPaste.cpFirst );
	PushInsSelCur();

	AssureNestedPropsCorrect(&caPaste, fTrue);
	SelectPostDelete(caPaste.cpLim);
	PushInsSelCur();
	caRM.cpLim += DcpCa(&caScrap) + (fAddPara ? ccpEop : 0);
#ifdef DAVIDBO
	CommSzNum(SzShared("\tAfter doc:\t"), caRM.doc);
	CommSzLong(SzShared("\tAfter cpFirst:\t"), caRM.cpFirst);
	CommSzLong(SzShared("\tAfter cpLim:\t"), caRM.cpLim);
#endif /* DAVIDBO */
	SetUndoAfter(&caRM);

	cmd = cmdOK;

PasteEnd:
	EndLongOp(fFalse /* fAll */);
	/* this will clear out any eop weirdnesses that would reset
		vfSeeSel
	*/
	UpdateWw( wwCur, fFalse /* fAbortOk*/);
	vfSeeSel = fTrue;
	return cmd;
}


/*  %%Function:  DestroyClip  %%Owner:  bobz       */


DestroyClip()
	{	/* Handles WM_DESTROYCLIPBOARD message.  We are being notified that
		the clipboard is being emptied & we don't need to keep its
	contents around anymore. */
	struct CA ca;

	if (vsab.fDontDestroyClip)
		return;

	vsab.fOwnClipboard = fFalse;

	/* Clear out the scrap document */
	SetWholeDoc(docScrap, PcaSetNil(&ca));

	/* Disable UNDO operations that require the clipboard */
	/* should only care about copy and cut, which change the clipboard state */
	if (vuab.bcm == bcmCut || vuab.bcm == bcmCopy)
		SetUndoNil();
}



/*  %%Function:  ChangeClipboard  %%Owner:  bobz       */



ChangeClipboard()
{   /* Mark clipboard as changed.  If we are not the owner of the clipboard, */
	/* make us the owner (via EmptyClipboard).	The EmptyClipboard call */
	/* will result in a WM_DESTROYCLIPBOARD message being sent to the */
	/* owning instance.  The CloseClipboard call will result in a */
	/* WM_DRAWCLIPBOARD message being sent to the clipboard viewer. */
	/* If the clipboard viewer is CLIP.EXE, we will get a WM_PAINTCLIPBOARD */
	/* message */
	/* Added 10/8/85 by BL: If docScrap is empty, relinquish ownership */
	/* of the clipboard */

	// for more sense in reading
#define CF_UNRENDERABLE CF_TIFF

	int cf = CF_UNRENDERABLE;
	CP cpMacScrap = CpMacDocEdit(docScrap);
	struct CA caT;
	int mm;
	BOOL fDIBInMetafile = fFalse;

#ifdef DEBUG
	/* used for testing paste - forces us to paste in from clipboard
		even if we owned it.  Set in SlapWnd and for debugging, turned off when
		we load something into the clipboard.
	*/
	vTestClip = 0;
#endif /* DEBUG */


	/* we want all fields in the scrap to be consistent with the vpref
		values so we can safely put pictures into the clipboard in CF_TEXT
		format. DO this here, sice both cut and paste go through this routine.
	*/

	ClearFDifferFltg (docScrap, fltgAll);

	if (!OpenClipboard( vhwndApp ))
		{	/* Couldn't open the clipboard, wipe out contents & disable UNDO */
		/* a bit harsh huh? */ /* perhaps, but if we can't open the clipbord
		we are in an indeterminite state; better to wipe out contents
		and be in a known state */
		DestroyClip();
		return;
		}

/* We want to clear out previous data formats in the clipboard.
	Unfortunately, the only way to do this is to call EmptyClipboard(),
	which has the side effect of calling us with a WM_DESTROYCLIP
	message. We use this primitive global comunication to prevent
	docScrap from being wiped out in DestroyClip() */

	vsab.fDontDestroyClip = fTrue;
	EmptyClipboard();
	vsab.fDontDestroyClip = fFalse;

/* Re-validate vsab.fPict (in case a docScrap edit changed what it
		should be) */

	vsab.fPict = FPicRenderable(docScrap, cp0, cpMacScrap, &mm);
	   /* if ca is a rejected pic type,(flase return but
		  mm != MM_NIL) set cf to CF_UNRENDERABLE, which
		  will be rejected below
	   */
	if (!vsab.fPict)
		cf = (mm == MM_NIL ? CF_TEXT : CF_UNRENDERABLE);
	else
		{
		// just a warning. We will ignore other types
		Assert (mm == MM_BITMAP || mm < MM_META_MAX ||
			    mm == MM_DIB);

		switch (mm)
			{
			default:
			// note: if not recognized, cf left at CF_UNRENDERABLE and ignored
				if (mm < MM_META_MAX)
					{
					if (!FDIBInMetafile())
						cf = CF_METAFILEPICT;
					else
						fDIBInMetafile = fTrue;
					}
				break;
			case MM_BITMAP:
				cf = CF_BITMAP;
				break;
			}

		}


#ifdef BZ
   	CommSzNum(SzShared("vsab.fPict "), vsab.fPict);
   	CommSzNum(SzShared("renderable mm "), mm);
   	CommSzNum(SzShared("cf "), cf);
#endif /* BZ */


	vsab.fOwnClipboard = (cpMacScrap != cp0);
	if (vsab.fOwnClipboard)
		{	/* only set handles if we really have something in docScrap */
		SetClipboardData( CF_OWNERDISPLAY, NULL );
			/* don't put out cf_tiff or other rejected pic types, or
			   rtf or link for any of those types
			*/
		if (cf != CF_UNRENDERABLE || fDIBInMetafile)
			{
			SetClipboardData( cfRTF, NULL ); /* always avail */
			if (fDIBInMetafile)
				{
				SetClipboardData( CF_METAFILEPICT, NULL );
				SetClipboardData( CF_DIB, NULL );
				if (vwWinVersion >= 0x0300)
					SetClipboardData( CF_BITMAP, NULL);
				}
			else				
				SetClipboardData( cf, NULL );
				/*	can only link if we can still get at the source w/a bookmark */
			if (FCanLinkScrap ())
				SetClipboardData( cfLink, NULL );
			}

		}

	CloseClipboard();
}


#ifdef NOTUSED
/*  %%Function:  FSameClassHwndSz  %%Owner:  notused       */

FSameClassHwndSz( hwnd, szClass )
HWND hwnd;
CHAR szClass[];
	{   /* Compare the Class name of hWnd with szClass; return TRUE
	if they match, FALSE otherwise. */

#define cchClassMax	40  /* longest class name (for compare purposes) */

	CHAR rgchWndClass[ cchClassMax ];
	int cbCopied;

	/* Count returned by GetClassName does not include terminator */
	/* But, the count passed in to it does. */
	cbCopied = GetClassName( hwnd, (LPSTR) rgchWndClass, cchClassMax ) + 1;
	if (cbCopied <= 1)
		return FALSE;

	rgchWndClass[ cbCopied - 1 ] = '\0';

	return FEqNcSz( rgchWndClass, szClass );
}


#endif /* NOTUSED */



/* F  C A N  L I N K  S C R A P */
/*  Return true if the data in doc scrap can be Paste-Linked to.
	Conditions: must have a non-empty range in a displayed mother
	document.
*/
/*  %%Function:  FCanLinkScrap   %%Owner:  bobz       */

FCanLinkScrap ()
{
	return vsab.doc != docNil && vsab.cpFirst < vsab.cpLim
			&& PdodDoc (vsab.doc)->fMother && 
			WwDisp(vsab.doc,wwNil,fFalse) != wwNil;
}



/* The following was previously in clipdisp.c */

extern struct FLI       vfli;
extern struct SAB       vsab;
extern struct SCI       vsci;
extern CHAR             stEmpty[];



int vwwClipboard=wwNil;
int vmwClipboard=mwNil;

int NEAR FGetClipboardDC( void );
int NEAR ReleaseClipboardDC( void );


csconst char stFmtTextCs[] = StKey("Word Formatted Text",WordFormattedText);

/*  %%Function:  PaintClipboard  %%Owner:  bobz       */


PaintClipboard( hwnd, hps )
HWND   hwnd;
HANDLE hps;
{   /* Paint portion of clipboard window indicated by hps */

	LPPAINTSTRUCT lpps;
	struct WWD *pwwd;

	if (vwwClipboard == wwNil)
		{
		ReportSz("Warning - PaintClipboard with vwwClipBoard nil ignored");
		return;
		}

/* Must set the scroll bar range each time we get a PAINT message;
	CLIPBRD.EXE resets it when it gets WM_DRAWCLIPBOARD */

	pwwd = PwwdWw(vwwClipboard);
	Assert( pwwd->hwnd == hwnd );
	SetScrollRange( pwwd->hwnd, SB_VERT, 0, dqMax-1, TRUE );
	SetScrollRange( pwwd->hwnd, SB_HORZ, 0, vsci.xpRightMax, FALSE );
/* 0 size followed by non-0 size means clipboard app may have been
	taken down, then re-created; so, reset the scroll bars to 
	what they should be. */
	SyncSbHor(vwwClipboard);
	SetElevWw(vwwClipboard);
	pwwd = PwwdWw(vwwClipboard);

	if ( (lpps = (LPPAINTSTRUCT)GlobalLockClip( hps )) != NULL )
		{
		HDC hdc = lpps->hdc;
		struct RC rcT;

		rcT = lpps->rcPaint;
		GlobalUnlock( hps );
		if (FSetDcAttribs( pwwd->hdc = hdc, dccDoc ))
			{   /* Paint the clipboard */
			InvalWwRc(vwwClipboard, &rcT);
			UpdateWw( vwwClipboard, fFalse /*fAbortOK*/ );
			}
	/* BLOCK workaround windows RIP by deselecting our objects */
			{
			HDC hdc = PwwdWw(vwwClipboard)->hdc;
			Assert(hdc != NULL);
			SelectObject(hdc, GetStockObject(SYSTEM_FONT));
			SelectObject(hdc, GetStockObject(WHITE_BRUSH));
			SelectObject(hdc, GetStockObject(BLACK_PEN));
			}

		}

	/* Since the DC is no longer good, we'll set it to NULL */
	PwwdWw(vwwClipboard)->hdc = NULL;
}


/*  %%Function:  SizeClipboard  %%Owner:  bobz       */


SizeClipboard( hwnd, hrc )
HWND    hwnd;
HANDLE  hrc;
{   /* Set clipboard window to be the rect in hRC */
	/* If rectangle is 0 units high or wide, this means we're losing the
		necessity for display until the next size message */
	LPRECT lprc;
	struct RC rc;
	struct WWD *pwwd;

	if ( (lprc = (LPRECT)GlobalLockClip( hrc )) == NULL )
		return;
	rc = *lprc;
	GlobalUnlock( hrc );

/* NULL rectangle means no more displaying until we get a nonnull size */

	if (FValidateWwClipboard(hwnd))
		{
		pwwd = PwwdWw( vwwClipboard );
		if (FEmptyRc(&rc))
			pwwd->dqElevator = 0;
		else
			{
			pwwd->rcwDisp = rc;
			if (pwwd->xhScroll < 0)
				pwwd->rcwDisp.xwLeft += dxwSelBarSci;
			RcToDrc(&pwwd->rcwDisp, &PdrGalley(pwwd)->drcl);
			PdrGalley(pwwd)->drcl.xl = PdrGalley(pwwd)->drcl.yl = 0;
			TrashWw( vwwClipboard );
/* 0 size followed by non-0 size means clipboard app may have been
	taken down, then re-created; so, reset the scroll bars to 
	what they should be. */
			SyncSbHor(vwwClipboard);
			SetElevWw(vwwClipboard);
			}
		}
}


/*  %%Function:  VScrollClipboard  %%Owner:  bobz       */


VScrollClipboard( hwnd,  sbMessage, posNew )
HWND    hwnd;
int     sbMessage;
int     posNew;
{
	struct WWD *pwwd;
	int dypPage;
	CP cpMac, cpT;

	if (vwwClipboard == wwNil)
		{
		Assert( fFalse );
		return;
		}

	Assert( PwwdWw(vwwClipboard)->hwnd == hwnd );

	if (!FGetClipboardDC())
		/* Unable to create clipboard device context */
		return;

	pwwd = PwwdWw(vwwClipboard);
	switch ( sbMessage )
		{
	case SB_THUMBPOSITION:
		cpMac = CpMacDoc( PdrGalley(pwwd)->doc );
		cpT = cpMac > cpMax / (dqMax - 1)
			? (cpMac  / (dqMax - 1)) * posNew
			: (cpMac * posNew) / (dqMax - 1);
		ThumbToCp(vwwClipboard, CpMin(cpT, cpMac-ccpEop), fFalse, fFalse, 0);
		break;

	case SB_LINEUP:
		ScrollDown( vwwClipboard, vsci.dysMinAveLine, vsci.dysMacAveLine );
		break;
		break;
	case SB_LINEDOWN:
		ScrollUp( vwwClipboard, vsci.dysMinAveLine, vsci.dysMacAveLine );
		break;
	case SB_PAGEUP:
	case SB_PAGEDOWN:
		dypPage = pwwd->ywMac - pwwd->ywMin;
		if (dypPage > vsci.dysMacAveLine)
			dypPage -= vsci.dysMacAveLine;
		(*(sbMessage == SB_PAGEUP ? ScrollDown : ScrollUp))
				( vwwClipboard, dypPage, dypPage + vsci.dysMacAveLine - vsci.dysMinAveLine );
		break;
		}

	UpdateWw( vwwClipboard, fFalse );
	ReleaseClipboardDC();
}


/*  %%Function:  HScrollClipboard  %%Owner:  bobz       */



HScrollClipboard( hwnd,  sbMessage, posNew )
HWND    hwnd;
int     sbMessage;
int     posNew;
{

	struct WWD *pwwd;
	int dxpWw;
	int xpScroll;
	int xpThumb;

	if (vwwClipboard == wwNil)
		{
		Assert( fFalse );
		return;
		}

	Assert( PwwdWw(vwwClipboard)->hwnd == hwnd );

	if (!FGetClipboardDC())
		/* Unable to create clipboard device context */
		return;

	pwwd = PwwdWw(vwwClipboard);
	dxpWw = pwwd->xwMac - pwwd->xwMin;
	xpScroll = abs(DxpScrollHorizWw(vwwClipboard));

	switch (sbMessage)
		{
	case SB_LINEUP:     /* line left */
		if (xpScroll == 0)
			Beep();
		else  if (vsci.dxpMinScroll > xpScroll)
			/* scroll to zero */
			ScrollRight(vwwClipboard, xpScroll);
		else
			/* normal scroll */
			ScrollRight(vwwClipboard, vsci.dxpMinScroll);
		break;
	case SB_LINEDOWN:   /* line right */
		ScrollLeft( vwwClipboard, vsci.dxpMinScroll );
		break;
	case SB_PAGEUP:     /* page left */
		ScrollRight( vwwClipboard, min((dxpWw>>1),xpScroll) );
		break;
	case SB_PAGEDOWN:   /* page right */
		ScrollLeft( vwwClipboard, dxpWw>>1 );
		break;
	case SB_THUMBPOSITION:
		xpThumb = NMultDiv(posNew,
				vsci.xpRightMax - DxOfRc(&pwwd->rcwDisp),
				vsci.xpRightMax);
		xpScroll = xpThumb + pwwd->xhScroll;
		if (xpScroll > 0)
			ScrollLeft(vwwClipboard, xpScroll);
		else
			ScrollRight(vwwClipboard, -xpScroll);
		break;
		}

	UpdateWw( vwwClipboard, fFalse );
	ReleaseClipboardDC();
}


/* A s k  C b  F o r m a t  N a m e */
/*  %%Function:  AskCBFormatName  %%Owner:  bobz       */

AskCBFormatName( lpchName, cchNameMac )
LPCH lpchName;
int cchNameMac;
	{   /* Copy the format name for the current contents of the clipboard
		(of which we are the owner) to lpchName, copying no more than
	cchNameMac characters */

	int cchCopy;

	Assert( vsab.fOwnClipboard );

	/* Don't give a format name for pictures; the name is covered by the
		standard types */

	if (!vsab.fPict)
		{
		if ((cchCopy=*stFmtTextCs) >= cchNameMac)
			cchCopy = cchNameMac-1;

		bltbx((LPSTR)stFmtTextCs+1, (LPSTR)lpchName, cchCopy);
		lpchName[cchCopy] = 0;
		}
}


/* F  G e t  C l i p b o a r d  D C */
/*  %%Function:  FGetClipboardDC  %%Owner:  bobz       */

int NEAR FGetClipboardDC()
	{   /* Get a DC for the clipboard window.  Leave it in rgwwd [wwClipboard].
		Select in the background brush for appropriate color behavior.
	return TRUE if all is well, FALSE if something went wrong */

	struct WWD *pwwd;

	if (vwwClipboard == wwNil)
		return fFalse;

	pwwd = PwwdWw(vwwClipboard);
	if ((pwwd->hdc = GetDC( pwwd->hwnd )) == NULL)
		return fFalse;

	if (!FSetDcAttribs( pwwd->hdc, dccDoc ))
		{
		ReleaseClipboardDC();
		return fFalse;
		}

	return fTrue;
}


/* R e l e a s e  C l i p b o a r d  D C */
/*  %%Function:  ReleaseClipboardDC  %%Owner:  bobz       */

int NEAR ReleaseClipboardDC()
{
	struct WWD *pwwd;

	Assert( vwwClipboard != wwNil );
	pwwd = PwwdWw(vwwClipboard);
	ReleaseDC( pwwd->hwnd, pwwd->hdc );
	pwwd->hdc = NULL;
}


/*  %%Function:  FValidateWwClipboard  %%Owner:  bobz       */


int FValidateWwClipboard( hwnd )
HWND hwnd;
{
	struct SELS sels;

	if (vwwClipboard == wwNil)
		{
		sels.ca.doc = docNil;
		if (!FCreateMw( docScrap, wkClipboard, &sels, NULL))
			return fFalse;
		Assert( vwwClipboard != wwNil && vmwClipboard != mwNil );
		}

	PwwdWw(vwwClipboard)->hwnd = hwnd;
	return fTrue;
}

