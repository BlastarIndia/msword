/* C M D . C */

#define NONCMESSAGES
#define NOMENUS
#define NOKEYSTATE
#define NOSHOWWINDOW
#define NOSYSMETRICS
#define NOCOLOR
#define NOMEMMGR
#define NOMENUS
#define NOMINMAX
#define NOREGION
#define NOSCROLL
#define NOTEXTMETRIC
#define NOWINOFFSETS

#define NOSYSMETRICS
#define NOICON
#define NORASTEROPS
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOHDC
#define NOBRUSH
#define NOPEN
#define NOFONT
#define NOWNDCLASS
#define NOCOMM
#define NOSOUND
#define NORESOURCE
#define NOOPENFILE
#define NOWH
#define NOKANJI

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "props.h"
#include "disp.h"
#include "keys.h"
#include "doc.h"
#include "debug.h"
#include "sel.h"
#include "prompt.h"
#include "message.h"
#include "ibdefs.h"
#define USEBCM /* because we have staticly initialized tables of them! */
#include "cmd.h"
#include "inter.h"
#include "core.h"

#define RULER
#define REPEAT
#include "ruler.h"

#include "error.h"

#include "wininfo.h"
#include "ribbon.h"


#include "fontwin.h"
#include "prm.h"
#include "status.h"
#include "ch.h"
#include "dlbenum.h"

#include "cmdlook.h"

#include "screen.h"

#include "file.h"
#include "field.h"
#include "search.h"
#include "outline.h"
#include "style.h"
#include "cmdtbl.h"
#include "doslib.h"
#include "help.h"
#include "rareflag.h"

#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmparse.h"
#include "tmc.h"

#include "edstyle.hs"

extern BOOL             vfRecording;
extern KMP **           hkmpCur;
extern int              mwCur;
extern int              wwCur;
extern int              vfDefineStyle;
extern int              vfSearchRepl;
extern int              vdocStsh;
extern int              vstcStyle;
extern int              ibstFontDS;
extern struct ESPRM     dnsprm[];
extern struct MWD       **hmwdCur;
extern struct PREF      vpref;
extern struct WWD       **hwwdCur;
extern struct SEL       selCur;
extern HWND             vhwndApp;
extern HWND             vhwndRibbon;
extern CHAR             szEmpty[];
extern struct MWD **    mpmwhmwd[];
extern struct MERR      vmerr;
extern struct STTB      **vhsttbFont;
extern int              vfFileCacheDirty;
extern RRF		rrf;
extern struct UAB	vuab;

DPV DpvPwFromSzW3IdFUt();
unsigned IcoValidateColor();

#define cchszColorMax	(sizeof(StKey("Magenta",LongestColor)))

/* %%Function: CmdFont %%Owner: bobz */
CMD CmdFont(pcmb)
CMB * pcmb;
{
	CMD cmd;
	int tmc;
	CHAR szFont [LF_FACESIZE];
	CHAR szPoint [cchMaxSz];

	Assert( selCur.ww != wwNil );   /* no current window, no get called */

	szFont[0] = '\0';
	szPoint[0] = '\0';

	if (pcmb->fDialog)
		{
		if (vhwndRibbon)
			{
			/* ribbon is up, let them enter the font there */
			SetFocusIibb(vhwndRibbon, IDLKSFONT);
			return cmdError; /* So it doesn't get recorded */
			}
		else
			{
			/* no ribbon, bring up a prompt for them to enter font */
			szFont [0] = '\0';
			tmc = TmcInputPmtMst(mstFontName, szFont, sizeof(szFont), 
					bcmFont, MmoFromCmg(cmgApplyProp));

			switch (tmc)
				{
			case tmcOK:
				cmd = cmdOK;
				break;
			case tmcCmdKc:
				/* The user hit the font key bring up the dialog. */
				ChainCmd(bcmCharacter);
				return cmdOK;
			default:
			case tmcCancel:
				return cmdCancelled;
				}
			}  /* else */
		}
	else
		cmd = cmdOK;

/* so now we have a CAB, apply prop(s) (user can set both font and point
	in ribbon) */

	if (pcmb->fAction && cmd == cmdOK)
		{
		ApplyFontPointPcmd(&cmd, szFont, szPoint);
		if (cmd == cmdError && FCmdDialog())
			{
			Beep();
			ChainCmd(bcmFont);
			}

		if (cmd == cmdOK && vfRecording)
			{
			FlushVrac();
			RecordSt(StSharedKey("Font ", FontCmd));
			SzToStInPlace(szFont);
			RecordQuotedSt(szFont);
			RecordEop();
			}
		}

	return cmd;
}


/* %%Function:CmdFontSize %%Owner: bobz */

CMD CmdFontSize(pcmb)
CMB * pcmb;
{
	CMD cmd;
	int tmc;
	CHAR szPoint[cchMaxSz];
	CHAR szFont [LF_FACESIZE];

	Assert( selCur.ww != wwNil );   /* no doc, no get called */

	szFont[0] = '\0';
	szPoint[0] = '\0';

	if (pcmb->fDialog)
		{
		if (vhwndRibbon)
			{
			/* ribbon is up, let them enter the font there */
			SetFocusIibb(vhwndRibbon, IDLKSPOINT);
			return cmdError; /* so it doesn't get recorded */
			}
		else
			{
			/* No ribbon, so get font size from prompt line */

			*szPoint = '\0';
			tmc = TmcInputPmtMst(mstFontSize, szPoint, sizeof(szPoint),
					bcmFontSize, MmoFromCmg(cmgApplyProp));
			switch (tmc)
				{
			case tmcOK:
				cmd = cmdOK;
				break;
			case tmcCmdKc:
				/* they hit the points so put up a dialog */
				ChainCmd(bcmCharacter);
				return cmdOK;
			default:
				/* tmcCancel, etc... do nothing and terminate. */
				return cmdCancelled;
				}
			}  /* else */
		}
	else
		{
		cmd = cmdOK;
		}

/* so now we have values (but no CAB), apply prop(s) (user can set both font
	and point in ribbon) */

	if (pcmb->fAction && cmd == cmdOK)
		{
		ApplyFontPointPcmd(&cmd, szFont, szPoint);
		if (cmd == cmdError && FCmdDialog())
			{
			Beep();
			ChainCmd(bcmFontSize);
			}

		if (cmd == cmdOK && vfRecording)
			{
			FlushVrac();
			RecordSt(StSharedKey("FontSize ", FontSizeCmd));
			RecordSz(szPoint);
			RecordEop();
			}
		}

	return cmd;
}



/* %%Function: CmdColor %%Owner: bobz */

CMD CmdColor(pcmb)
CMB * pcmb;
{
	CMD cmd = cmdOK;
	CHAR szColor[cchMaxSz];

	Assert( selCur.ww != wwNil );   /* no doc, no get called */

	szColor[0] = '\0';

	if (pcmb->fDialog)
		{
		pcmb->tmc = TmcInputPmtMst(mstColor, szColor, sizeof(szColor),
				bcmColor, MmoFromCmg(cmgApplyProp));
		switch (pcmb->tmc)
			{
		case tmcOK:
			break;

		case tmcCmdKc:
			/* they hit the same key so put up a dialog */
			ChainCmd(bcmCharacter);
		/* FALL THROUGH (so as not to record) */

		default:
			/* tmcCancel, etc... do nothing and terminate. */
			return cmdCancelled;
			}
		}

/* so now we have the value (but no cab), apply prop */

	if (pcmb->fAction && pcmb->tmc == tmcOK)
		{
		ApplyColorPcmd(&cmd, szColor);
		if (cmd == cmdError && pcmb->fDialog)
			{
			Beep();
			ChainCmd(bcmColor);
			}

		if (cmd == cmdOK && vfRecording)
			{
			FlushVrac();
			RecordSt(StSharedKey("CharColor ", CharColor));
			RecordInt(IcoValidateColor(szColor));
			RecordEop();
			}
		}

	return cmd;
}



/* given a filled-out cab in pcmb->hcab for font and point size,
	apply the font and point to the current sel.  Either font, or point,
	or both, may be provided. cmdError will be returned if neither font
	name nor point size has been supplied. */

/* A p p l y  F o n t  P o i n t  C a b  P c m d */

/* %%Function: ApplyFontPointPcmd  %%Owner: bobz */

ApplyFontPointPcmd(pcmd, szFfn, szPoint)
CMD *pcmd;
CHAR * szFfn;
CHAR * szPoint;
{
	CHAR rgchFfn [cbFfnLast];
	struct FFN *pffn = rgchFfn;
	int wHps;
	int ftc;

	*pcmd = cmdError;

	Assert(CchSz(szFfn) <= LF_FACESIZE);
	CchCopySz(szFfn, pffn->szFfn);

	if (rrf.fSelMoved)
		{
		ClearRrf();
		}

	/* apply font name if supplied and valid*/

	ftc = FtcValidateFont(pffn);
	if (ftc != wNinch && ftc != valNil)
		{
		if (!FApplyOneProp (sprmCFtc, ftc))
			{
			*pcmd = cmdCancelled;
			return;
			}
		rrf.ftc = ftc;
		ClearRulerRrf();
		*pcmd = cmdOK;
		}

	/* apply font size if supplied and valid */

	if (*szPoint != '\0')
		{
		int hps;
		DPV dpv;

		dpv = DpvPwFromSzW3IdFUt( &hps, szPoint,
				4, 127, dpvDouble | dpvBlank | dpvSpaces,
				eidOutOfRange, fFalse, 0);

		if (dpv == dpvNormal || dpv == dpvDouble)
			{
			if (!FApplyOneProp(sprmCHps, hps))
				{
				*pcmd = cmdCancelled;
				return;
				}
			rrf.hps = hps;
			ClearRulerRrf();
			*pcmd = cmdOK;
			}
		else
			*pcmd = cmdError;
		}
}


/* given a color name, apply color to current sel.
	cmdError will be returned if color name is invalid
*/


/* %%Function:ApplyColorPcmd %%Owner: bobz */

ApplyColorPcmd(pcmd, szColor)
CMD *pcmd;
CHAR * szColor;
{
	unsigned ico;

	*pcmd = cmdError;

	/* apply font name if supplied and valid*/

	ico = IcoValidateColor(szColor);
	if (ico != uNinch )
		{
		*pcmd = (FApplyOneProp (sprmCIco, ico) ? cmdOK : cmdCancelled);
		}

}


/* %%Function: IcoValidateColor %%Owner: bobz */

unsigned IcoValidateColor(szColor)
CHAR *szColor;
{

	/* note this guy will match on any valid prefix of a color */

	int ico;
	CHAR szColorTbl[cchszColorMax];
	int cch = CchSz (szColor) - 1;
	int cchTbl;

	if (cch > cchszColorMax)
		return (uNinch);

	for (ico = 0; ico < cCharColor; ico++)
		{
		AssertDo (Look1FEnumIEntbl (iEntblCharColor, ico, szColorTbl));
		cchTbl = CchSz (szColorTbl) - 1;
		if (cch > cchTbl)
			continue;
		/* allow us to accept prefix */
		if (FEqNcRgch(szColor, szColorTbl, min(cch, cchTbl)))
			return (ico);
		}

	return (uNinch);

}


/* this is around for the ribbon */
/*-------------------------------------------------------------------
	***** sdm based variant of WPwFromItW3IdFUt ****** bz *******
		***** already have dialog item text in szItem **********

	Purpose:    Parse the item in szItem.  Must be a valid
				integer or dxa in the given range.
	Method:     - Get the text string.
				- Try parse as "".
				- Try parse as string of all spaces
				- Parse as a int/dxa (generic error if can't).
				- Test for ".5".
				- Compare with min and max.
				- Try parse as "Auto".
	Returns:    The return value may be used as a boolean or as a word.
				fFalse (0) -> not parsed
				wNormal (1) -> parsed normally
				wBlank (2) -> parsed a null line
										(*pw is valNil)
				wAuto (4) -> parsed as "Auto" (*pw is 0)
				wSpaces (16) -> parsed a line of all
										spaces (*pw is valNil)

				!fDxa only:
				wDouble (8) -> parsed with ".5" trailing

	Note:        The interval [wLow..wHigh] is closed.
	Note:        Return value is double the parsed value when wDouble.
	Note:        When wDouble, 2*wLow and 2*wHigh must be valid ints.
	Note:        Numbers ending in .5 may have no trailing spaces.
	History:
		04/08/85:    Sdm variant created. Auto spacing code removed.
		07/03/85:    Added wSpaces return
		10/23/84:    Fixed wAuto to return with *pw == 0.
		10/ 5/84:    Added ut parameter.
		10/ 5/84:    Added wMask and combined dxa and w parsing.
			9/26/84:    Created.
--------------------------------------------------------------bz--*/


/* %%Function: DpvPwFromSzW3IdFUt %%Owner: bobz */

DPV DpvPwFromSzW3IdFUt(pw, szItem, wLow, wHigh, dpvMask, eid, fDxa, ut)
int *pw;   /* Return value */
CHAR *szItem;  /* string to be parsed */
int wLow;      /* Smallest and largest allowed values */
int wHigh;
DPV dpvMask;     /* Bit mask for allowed variations */
int eid;        /* eid of error string if out of range */
int fDxa;      /* Parse as dxa (otherwise int) */
int ut;        /* Units to use as default if fDxa */
{
	CHAR *pch;          /* Parse pointer */
	CHAR *pchEnd;       /* End of buffer */
	CHAR *pchError;     /* Position of parse error */
	int fParsed;        /* Parses as number/dxa */
	int fOverflow = fFalse; /* True if the number is parsed but it overflow */
	int dpvGood = dpvNormal;/* return after good range check */
	int cbItem;          /* length of item string */

	/* Get the size of the text */
	cbItem = CchSz (szItem) - 1;

	/* See if blank (null line) */
	if (dpvMask & dpvBlank && cbItem == 0)
		{
		*pw = valNil;
		return dpvBlank;
		}

	pch = szItem;

	/* See if all spaces  */
	if (dpvMask & dpvBlank && dpvMask & dpvSpaces)
		{
		int fAllSpaces = fTrue;

		while (*pch != 0)
			if (*pch++ != ' ')
				{
				fAllSpaces = fFalse;
				break;
				}
		if (fAllSpaces == fTrue)
			{
			*pw = valNil;
			return(dpvSpaces);
			}
		}

	pch = szItem;
	pchEnd = pch + cbItem;

	/* It parses as a number ... */
	fParsed = fDxa ? FZaFromSs(pw, szItem, cbItem, ut, &fOverflow)
			: FPwParsePpchPch(pw, &pch, pchEnd, &fOverflow);

	if (!fDxa && dpvMask & dpvDouble)
		{
		(*pw) *= 2;
		wLow *= 2;
		wHigh *= 2;
		if (!fParsed)
			{
			/* Check if ".5" was reason for bad parse. */
			if (pch != pchEnd && *pch == '.')
				{
				pch++;
				/* Allow "ddddd.0*" */
				pchError = pch;
				if (FAllZeroPpchPch(&pchError, pchEnd))
					fParsed = fTrue;
				/* Allow "ddddd.50*" */
				else  if (pch != pchEnd && *pch == '5' &&
						(pch++, FAllZeroPpchPch(&pch, pchEnd)))
					{
					(*pw)++;
					fParsed = fTrue;
					dpvGood = dpvDouble;
					}
				/* Mark furthest error condition */
				else  if (pchError > pch)
					pch = pchError;
				}
			}
		}

	if (fParsed && !fOverflow)
		{
		/* ... and in range */
		if (*pw >= wLow && *pw <= wHigh)
			return(dpvGood);
		}

		/* All attempts failed - show user where they went wrong vis the attempted
			number parse. */
		{
		unsigned cchStart = fParsed ? 0 : pch - szItem;
		unsigned cchEnd = 32767;
		int eidError = fDxa ? eidNOTDXA : eidNOTNUM;

		if (fParsed)
			eidError = eid; /* reset eidError if we just overflow or fail the range test */

		if (eidError == eidOutOfRange || eidError == eidDxaOutOfRange)
			{
			if (!fDxa && (dpvMask & dpvDouble))
				{        /* restore original range for error reporting */
				wLow >>= 1;
				wHigh >>= 1;
				}
			RangeError(wLow, wHigh, fDxa, ut);
			}
		else
			ErrorEid(eidError, "DpvPwFromSzW3IdFUt");
		return(fFalse);
		}
}


/*  %%Function: EnumFontUpDS  %%Owner: davidbo  */

EnumFontUpDS()
{
	int ibstFontMac = (*vhsttbFont)->ibstMac;
	struct STSH stsh;
	struct STTB **hsttbChpe, **hsttbPape;

	ibstFontDS = (ibstFontDS + 1) % ibstFontMac;
	RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);
	BanterToTmc(tmcDSBanter, vstcStyle, &stsh, hsttbChpe, hsttbPape);
	((CESTY *)PcmbDlgCur()->pv)->fStyleDirty = fTrue;
}


/*  %%Function: EnumFontDownDS  %%Owner: davidbo  */

EnumFontDownDS()
{
	int ibstFontMac = (*vhsttbFont)->ibstMac;
	struct STSH stsh;
	struct STTB **hsttbChpe, **hsttbPape;

	ibstFontDS = (ibstFontDS + ibstFontMac - 1) % ibstFontMac;
	RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);
	BanterToTmc(tmcDSBanter, vstcStyle, &stsh, hsttbChpe, hsttbPape);
	((CESTY *)PcmbDlgCur()->pv)->fStyleDirty = fTrue;
}


/*  %%Function: EnumPtSizeUpDS  %%Owner: davidbo  */

EnumPtSizeUpDS()
{
	DoLooksDS(0 /* not used */, sprmCHps, fTrue /* fUp */);
}


/*  %%Function: EnumPtSizeDnDS  %%Owner: davidbo  */

EnumPtSizeDnDS()
{
	DoLooksDS(0 /* not used */, sprmCHps, fFalse /* fUp */);
}


/*  %%Function: EnumColorUpDS  %%Owner: davidbo  */

EnumColorUpDS()
{
	DoLooksDS(0 /* not used */, sprmCIco, fTrue /* fUp */);
}


/*  %%Function: EnumColorDnDS  %%Owner: davidbo  */

EnumColorDnDS()
{
	DoLooksDS(0 /* not used */, sprmCIco, fFalse /* fUp */);
}


/*  %%Function: DoLooksSR  %%Owner: rosiep  */

DoLooksSR(ilcd, sprm, val)
int ilcd;
CHAR sprm;
int val;
{
	struct FB *pfb;
	int sgc;
	CHAR *pprop, *ppropGray;
	CHAR prl[4];
	int cchPrl;
	struct LCD lcd;

	/* get search/replace formatting block */
	if (!FGetSRFb(&pfb))
		return;

	/* these cases should have been taken care of already */
	Assert (ilcd != ilcdParaNormal && ilcd != ilcdPlainText);

	sgc = dnsprm[sprm].sgc;
	if (sgc == sgcChp)
		{
		pprop = &(pfb->chp);
		ppropGray = &(pfb->chpGray);
		}
	else
		{
		Assert(sgc == sgcPap);
		pprop = &(pfb->pap);
		ppropGray = &(pfb->papGray);
		}

	/* handle cases where the correct value to store is not known
		in advance */

	GetPlcdIlcd(&lcd, ilcd);
	if (lcd.fFlip)
		{
		/* If property is off, turn it on */
		if (ValFromPropSprm(pprop, sprm) == 0)
			val = 1;
		}

	else  if (lcd.fSpecial)
		switch (sprm)
			{

			/* underline sprms clear underline if chp already has the
				right underline style */

		case sprmCKul:
			if (pfb->chpGray.kul == 0 && pfb->chp.kul == val)
				val = kulNone;
			break;


			/* for Sub/SuperScript, same thing as underline for
			turning off if on */

		case sprmCHpsPos:
			if (pfb->chpGray.hpsPos == 0 &&
					pfb->chp.hpsPos == (val & 0xFF))
				val = 0;

			break;

			/* 0 is a valid val for these, so setting to 0 won't turn
			the property into a "don't care" prop; instead, set the
			gray pap */
		case sprmPJc:
			if (pfb->papGray.jc == 0 &&
					pfb->pap.jc == val)
				{
				pfb->papGray.jc = -1;
				GenSRBanter(0, valNil, 0);
				return;
				}

			break;

		case sprmPDyaLine:
			if (pfb->papGray.dyaLine == 0 &&
					pfb->pap.dyaLine == val)
				{
				pfb->papGray.dyaLine = -1;
				GenSRBanter(0, valNil, 0);
				return;
				}

			break;

		case sprmPDyaBefore:
			if (pfb->papGray.dyaBefore == 0 &&
					pfb->pap.dyaBefore == val)
				{
				pfb->papGray.dyaBefore = -1;
				GenSRBanter(0, valNil, 0);
				return;
				}

			break;
#ifdef DEBUG
		default:
			Assert (fFalse);
#endif /* DEBUG */
			}

	cchPrl = CchPrlFromSprmVal (prl, sprm, val);
	GenSRBanter( prl, cchPrl, sgc );
}


/*  %%Function: DoLooksDS  %%Owner: davidbo  */

DoLooksDS(ilcd, sprm, val)
int ilcd;
CHAR sprm;
int val;
{
	struct DOD *pdod = PdodDoc(vdocStsh);
	struct STTB **hsttb;
	struct PAP prop; /* assumes Chp will always fit in a Pap */
	int sgc;
	BOOL fChp;
	CESTY *pcesty;
	CHAR *pprop;
	CHAR prl[4];
	int cchPrl;
	struct LCD lcd;

	if (ilcd != ilcdParaNormal || sprm == sprmCHps)
		{
		sgc = dnsprm[sprm].sgc;
		fChp = sgc == sgcChp;
		hsttb = fChp ? pdod->hsttbChpe : pdod->hsttbPape;
		pprop = &prop;
		RetrievePropeForStcp(pprop, StcpFromStc(vstcStyle, pdod->stsh.cstcStd), hsttb, fChp);
		}
	else
		/*
		*  Special handling for ParaNormal...vstcStyle gets all
		*  properties of its BasedOn ancestor.
		*/
		{
		int stcp, stcBase;
		struct STSH stsh;
		struct STTB **hsttbChpe, **hsttbPape;
		struct CHP chpBase, chpSave;
		struct PAP papBase, papSave;

		Assert(vfDefineStyle);
		SetBytes(&chpBase, 0, cbCHP);
		SetBytes(&papBase, 0, cbPAP);
		RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);
		stcp = StcpFromStc(vstcStyle, stsh.cstcStd);
		stcBase = (*stsh.hplestcp)->dnstcp[stcp].stcBase;
		MapStc(PdodDoc(vdocStsh), stcBase, &chpBase, &papBase);
		papBase.phe.fStyleDirty = fTrue;
		RetrievePropeForStcp(&chpSave, stcp, hsttbChpe, fTrue);
		RetrievePropeForStcp(&papSave, stcp, hsttbPape, fFalse);
		if (!FStorePropeForStcp(&chpBase, stcp, hsttbChpe, fTrue /* CHP */))
			goto LErrRet;
		if (!FStorePropeForStcp(&papBase, stcp, hsttbPape, fFalse /* PAP */))
			{
			AssertDo(FStorePropeForStcp(&chpSave, stcp, hsttbChpe, fTrue));
			goto LErrRet;
			}
		if (!FGenChpxPapxNewBase(vdocStsh, stcp))
			{
			StartGuaranteedHeap();
			RestoreStcpDefn(vdocStsh, stcp, &chpSave, &papSave);
			EndGuarantee();
			goto LErrRet;
			}
		if (!FRecalcAllDependentChpePape(&stsh, hsttbChpe, hsttbPape, stcp))
			{
			RestoreStcpAndDependentsDefn(vdocStsh, stcp, &chpSave, &papSave);
LErrRet:
			Assert (vmerr.fMemFail);
			/* if true, dialog will be brought down by sdm or msg filter */
			return;
			}
		BanterToTmc(tmcDSBanter, vstcStyle, &stsh, hsttbChpe, hsttbPape);
		return;
		}



	/* for these special sprms, val passed in is true (fUp) if enumerating
		up; false otherwise */
	if (sprm == sprmCHps)
		{
		/* what this line does: increase or decrease hps by 1 and wrap
			around to stay within legal point sizes.  The range of legal
			point sizes is [4,127] or [8,254] in hps.  If we subtract 8
			first, we can work modulo 247 [0,246] to wrap around correctly,
			then add the 8 back in.  I have left the -8 separate, instead
			of combining it into the (val ? 1 : 246) expression, to add
			clarity, as if that helps... */

		val = ((((struct CHP *)pprop)->hps - 8 + (val ? 1 : 246)) % 247) + 8;
		}
	else  if (sprm == sprmCIco)
		{
		val = (((struct CHP *)pprop)->ico + (val ? 1 : icoMax - 1)) % icoMax;
		}

	else
		{
	/* handle cases where the correct value to store is not known
		in advance */
		GetPlcdIlcd (&lcd, ilcd);

		if (lcd.fFlip)
			{
			/* If property is off, turn it on */
			if (ValFromPropSprm(pprop, sprm) == 0)
				val = 1;
			}

		else  if (lcd.fSpecial)
			switch (sprm)
				{

				/* underline sprms clear underline if chp already has the
					right underline style */

			case sprmCKul:
				if (((struct CHP *)pprop)->kul == val)
					val = kulNone;
				break;


				/* for Sub/SuperScript, same thing as underline for
				turning off if on */

			case sprmCHpsPos:
				if (((struct CHP *)pprop)->hpsPos == (val & 0xFF))
					val = 0;

				break;

#ifdef DEBUG
				/* special only for search/replace */
			case sprmPJc:
			case sprmPDyaLine:
			case sprmPDyaBefore:
				break;
			default:
				Assert (fFalse);
#endif /* DEBUG */
				}

		}

	cchPrl = CchPrlFromSprmVal (prl, sprm, val);
	ApplyGrpprlToStshPrope(prl, cchPrl, fChp, TRUE /* fUpdBanter */);
	((CESTY *)PcmbDlgCur()->pv)->fStyleDirty = fTrue;
}




/*  %%Function: CmdMenuMode  %%Owner: krishnam  */

CMD CmdMenuMode(pcmb)
CMB * pcmb;
{
	PostMessage(vhwndApp, WM_SYSCOMMAND, SC_KEYMENU, 0L);
	return cmdOK;
}



/* **** +-+utility+-+opus **** */
/* ****
* Description: Applies a value to a single specified sprm
** **** */

/*  %%Function: FApplyOneProp   %%Owner: bobz  */

FApplyOneProp (sprm, val)
int sprm;
int val;
{
	CHAR prl[4];
	int cchPrl;

	if ( (cchPrl = CchPrlFromSprmVal(prl, sprm, val)) != valNil )
		/* now we have: prl == prl to be merged */
		{
		ApplyGrpprlSelCur(prl, cchPrl, fTrue /*fSetUndo*/);

		/* prevent formatting keys from blowing away vaab in these dialogs */
		if (!vfSearchRepl && !vfDefineStyle)
			FSetAgainGrpprl(prl, cchPrl, bcmFormatting);
		}
	return (vmerr.fMemFail ? fFalse : fTrue);
}





extern int  vssc;
KMP ** vhkmpDyadic = hNil;


/* H E L P   D Y A D I C 
	This function will call help for dyadic move, copy, or format.
	(cxts are defined along side the mst's in message.h).
*/
/*  %%Function: HelpDyadic  %%Owner: rosiep  %%reviewed 7/25/89 */

HelpDyadic()
{
	int cxt;
	MST mst;

	switch (vssc)
		{
	case sscMove:
		cxt = cxtMoveTo;
		goto LCkFins;
	case sscCopy:
		cxt = cxtCopyTo;
		goto LCkFins;
	case sscCopyLooks:
		cxt = cxtFormatTo;
LCkFins:
		/* Contexts for "From" messages = contexts for "To" messages + 1
			(non-cxtIndex only)
		*/
		if (selCur.fIns)
			cxt++;
		break;
	case sscNil:
	default:
		cxt = cxtIndex;
		}

	GetHelp(cxt);
}


/* C M D  M O V E  

	This procedure initiates the Move command.
*/

/*  %%Function: CmdMove  %%Owner: bobz  %%reviewed 7/25/89 */

CMD CmdMove(pcmb)
CMB *pcmb;
{
	return (CmdInitDyadic(sscMove, pcmb));
}


/* C M D  C O P Y  T O  F R O M

	This procedure initiates the CopyToFrom (Dyadic Copy) command.
*/

/*  %%Function: CmdCopyToFrom  %%Owner: bobz  %%reviewed 7/25/89 */

CMD CmdCopyToFrom(pcmb)
CMB *pcmb;
{
	return (CmdInitDyadic(sscCopy, pcmb));
}


/* C M D  C O P Y  L O O K S

	This procedure initiates the Copy Formatting command.
	Note: in Opus, this is not normally assigned to a key.
*/

/*  %%Function: CmdCopyLooks  %%Owner: bobz  %%reviewed 7/25/89 */

CMD CmdCopyLooks(pcmb)
CMB *pcmb;
{
	return (CmdInitDyadic(sscCopyLooks, pcmb));
}


/* C M D  I N I T  D Y A D I C

	This procedure initializes all the modal dyadic operations
*/

/*  %%Function: CmdInitDyadic  %%Owner: bobz  %%reviewed 7/25/89 */


#define ckmeDyad (2)

CMD CmdInitDyadic(ssc, pcmb)
int ssc;
CMB *pcmb;
{

	vssc = ssc;

	if (pcmb->fRepeated)
		{
		return CmdEndSsc(fTrue/*fDoIt*/, pcmb);
		}
	/* keymap no longer modal - bagging out handled in FExecCmb */
	if ((vhkmpDyadic = HkmpNew(ckmeDyad, kmfAppModal)) != hNil)
		{
		AddKeyToKmp(vhkmpDyadic, kcReturn, bcmOK);
		AddKeyPfn(vhkmpDyadic, VK_F1, HelpDyadic);

		/* FUTURE: when travel keys are changed to go through the keymap stuff,
		they will have to be added to this kmp  - rp */

		if (FBeginSsc())
			{
			SetPromptVssc();
			return cmdOK;
			}
		else
			{
			Assert (vhkmpDyadic != hNil);
			RemoveKmp(vhkmpDyadic);
			vhkmpDyadic = hNil;
			}
		}

	vssc = sscNil;
	return cmdError;

}


/*   C M D  D O  D Y A D I C

	This procedure is called only from CmdOK
	when the user has pressed kcReturn to end a copy or move command.
*/

/*  %%Function: CmdDoDyadic  %%Owner: bobz  %%reviewed 7/25/89 */

CMD CmdDoDyadic(pcmb)
CMB * pcmb;
{

	CMD cmd;

	Assert (!vrf.fInDyadic);
	vrf.fInDyadic = fTrue;
	cmd = CmdEndSsc(fTrue/*fDoIt*/, pcmb);
	RestorePrompt();
	if (vhkmpDyadic != hNil)
		{
		RemoveKmp(vhkmpDyadic);
		vhkmpDyadic = hNil;
		}
	vrf.fInDyadic = fFalse;
	return (cmd);
}


/* C A N C E L  D Y A D I C  
	This procedure is called only from the Esc key processor when
	the user has pressed VK_ESCAPE to abort a copy or move command or if
	we are in a copy or move mode and the user changes modes.
*/

/*  %%Function: CancelDyadic  %%Owner: bobz  %%reviewed 7/25/89 */

CancelDyadic()
{
	/* if we are processing a dyadic move and something happens,
	like moving a sectionmark will close a header window, that
	will cancel dyadic move, never mind. Flag is set and restored
	in CmdDoDyadic
	*/
	if (vssc == sscNil || vrf.fInDyadic)
		return;

	CmdEndSsc(fFalse/*fDoIt*/, NULL /* pcmb */);
	/* otherwise we will have repeat move or whatever... */
	SetAgain(bcmNil);
	RestorePrompt();
	if (vhkmpDyadic != hNil)
		{
		RemoveKmp(vhkmpDyadic);
		vhkmpDyadic = hNil;
		}
}




/* 

	The following handles the INDENT key (F4) commands which are as follows:

		INDENT         -- Selected paragraphs are indented to the next tabstop.
							If there are no more tabstops defined, the paragraphs
							are indented by 1/2 inch.

		Shift-INDENT   -- Selected paragraphs are un-indented to the previous
							tabstop.  If there are no previous tabstops, nothing
							happens.

		Hanging indent -- If the user is typing the first line of a paragraph
							and presses INDENT, a tab is inserted at the current
							location and all following lines of the paragraph
							will be indented to that tabstop.
*/

extern struct PAP   vpapFetch;
extern struct CA    caPara;
extern struct RULSS vrulss;

/*  %%Function: CacheParaIND  %%Owner: bradv  */

CacheParaIND(doc,cp)
/* hack so we can pass the address of a hand native coded proc */
int doc;
CP cp;
{
	CachePara(doc,cp);
}


/*  %%Function: CmdIndent  %%Owner: bobz  */

CMD CmdIndent(pcmb)
CMB * pcmb;
{
	struct PAP   *ppap;
	struct PAP pap;

	if (vfDefineStyle)
		{
		MapStc(PdodDoc(selCur.doc), vstcStyle, NULL, &pap);
		ppap = &pap;
		}
	else
		{
		CachePara(selCur.doc, selCur.cpFirst);
		ppap = &vpapFetch;
		}
	return CmdIndentByDxa(DxaNextTab(ppap->dxaLeft, ppap) - ppap->dxaLeft);
}


/*  %%Function: CmdUnIndent  %%Owner: bobz  */

CMD CmdUnIndent(pcmb)
CMB * pcmb;
{
	struct PAP   *ppap;
	struct PAP pap;

	if (vfDefineStyle)
		{
		MapStc(PdodDoc(selCur.doc), vstcStyle, NULL, &pap);
		ppap = &pap;
		}
	else
		{
		CachePara(selCur.doc, selCur.cpFirst);
		ppap = &vpapFetch;
		}
	return CmdIndentByDxa(DxaPrevTab(ppap->dxaLeft, ppap) - ppap->dxaLeft);
}



/* Hanging Indent
	advance the insert point, and indent
	all but the first line of the paragraph
*/

/*  %%Function: CmdHangingIndent  %%Owner: bobz  */

CMD CmdHangingIndent(pcmb)
CMB * pcmb;
{
	return CmdHangCommon(fFalse);
}


/*  %%Function: CmdUnHang  %%Owner: bobz  */

CMD CmdUnHang(pcmb)
CMB * pcmb;
{
	return CmdHangCommon(fTrue);
}


/*  %%Function: CmdHangCommon  %%Owner: bobz  */

CmdHangCommon(fPrev)
int fPrev;
{
	struct CA ca, caInval;
	struct PAP   *ppap;
	struct PAP pap;

	ca = selCur.ca;
	if (!vfDefineStyle)
		{
		ExpandCaCache(&ca, &caInval, &caPara, sgcPap, CacheParaIND);
		if (!FSetUndoB1(bcmFormatting, uccFormat, &ca))
			return cmdCancelled;
		CachePara(ca.doc, ca.cpFirst);
		ppap = &vpapFetch;
		}

	else
		{
		MapStc(PdodDoc(ca.doc), vstcStyle, NULL, &pap);
		ppap = &pap;
		}

	IndentToPca(&ca,
			fPrev ? DxaPrevTab(ppap->dxaLeft, ppap) :
			DxaNextTab(ppap->dxaLeft, ppap),
			ppap->dxaLeft + ppap->dxaLeft1, ppap);

	if (!vfDefineStyle)
		{
		InvalCp (&caInval);
		SetUndoAfter(NULL);
		}

	return cmdOK;
}


/* C M D  I N D E N T  B Y  D X A */
/* Indent all paragraphs in the selection by dxa */

/*  %%Function: CmdIndentByDxa    %%Owner: bobz  */

CmdIndentByDxa(dxa)
int dxa;
{
	struct CA ca, caInval;
	char rgb [cchINT + 1];

	rgb[0] = sprmPNest;
	bltbyte(&dxa, &rgb[1], cchINT);

	if (vfDefineStyle)
		{
		ApplyGrpprlToStshPrope(rgb, 3, fFalse, fTrue);
		return cmdOK;
		}

	ca = selCur.ca;
	ExpandCaCache(&ca, &caInval, &caPara, sgcPap, CacheParaIND);
	if (!FSetUndoB1(bcmFormatting, uccFormat, &ca ))
		return cmdCancelled;

	if (vrulss.caRulerSprm.doc != docNil)
		{
		FlushRulerSprms();
		}

	ApplyGrpprlCa(rgb, 3, &ca);

	InvalCp(&caInval);
	/* mark dirty anyone watching the changed text */
	InvalText(&ca, fFalse /* fEdit */);

	SetUndoAfter(NULL);
	return cmdOK;
}


/* I N D E N T  T O  P C A */

/*
		General purpose indenter.  Indent paragraphs described by
		[cpFirst, cpLim) in the current document to dxa, with the first
		line indented to dxa1.
*/

/*  %%Function: IndentToPca  %%Owner: bobz  */

IndentToPca(pca, dxa, dxa1, ppap)
struct CA *pca;
int dxa, dxa1;
struct PAP   *ppap;
{
	int ib, cb;
	char rgb[2*cchINT+2];

	dxa = max(0, dxa);
	dxa1 = max(0, dxa1) - dxa;

	ib = 0;
	cb = 0;
	if (dxa1 != ppap->dxaLeft1)
		{
		rgb[ib++] = sprmPDxaLeft1;
		bltbyte(&dxa1, &rgb[ib], cchINT);
		ib += cchINT;
		cb += 3;
		}

	rgb[ib++] = sprmPDxaLeft;
	bltbyte(&dxa, &rgb[ib], cchINT);
	cb += 3;
	if (vfDefineStyle)
		{
		ApplyGrpprlToStshPrope(rgb, cb, fFalse, fTrue);
		return;
		}

	if (vrulss.caRulerSprm.doc != docNil
			/* and we know grpprl[0] != sprmPRuler*/)
		{
		FlushRulerSprms();
		}

	ApplyGrpprlCa(rgb, cb, pca);
	/*  mark dirty anyone watching the changed text */
	InvalText (pca, fFalse /* fEdit */);
}


/*  %%Function: DxaNextTab  %%Owner: bobz  */

int DxaNextTab(dxa, ppap)
int dxa;
struct PAP *ppap;
{
	int dxaTab;
	int * pdxaTab;
	int ctbd;

	pdxaTab = &ppap->rgdxaTab[0];
	for (ctbd = ppap->itbdMac; --ctbd >= 0; ++pdxaTab)
		if (*pdxaTab > dxa)
			return (*pdxaTab);

	dxaTab = max(1,PdodMother(selCur.doc)->dop.dxaTab);
	return ((dxa / dxaTab + 1) * dxaTab);
}


/*  %%Function: DxaPrevTab  %%Owner: bobz  */

int DxaPrevTab(dxa, ppap)
int dxa;
struct PAP *ppap;
{
	int dxaTab;
	int * pdxaTab;
	int ctbd;

	ctbd = ppap->itbdMac;
	pdxaTab = &ppap->rgdxaTab[0];
	dxaTab = max(1,PdodMother(selCur.doc)->dop.dxaTab);
	if (ctbd == 0 || *pdxaTab >= dxa)
		return (max(0, (dxa / dxaTab - 1) * dxaTab));

	while (--ctbd > 0 && pdxaTab[1] < dxa)
		++pdxaTab;

	return (*pdxaTab);
}


/*  %%Function: CmdAppMaximize  %%Owner: bradch  */

CmdAppMaximize(pcmb)
{
	ElWAppMaximize(1);
	return cmdOK;
}


/*  %%Function: CmdAppRestore  %%Owner: bradch  */

CmdAppRestore(pcmb)
{
	ElWAppRestore();
	return cmdOK;
}


