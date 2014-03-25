/*
*   stylesub.c
*/

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NONCMESSAGES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW

#define OEMRESOURCE
#define NOSYSMETRICS
#define NOBITMAP
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOFONT
#define NOGDI
#define NOHDC
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOPEN
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#define NOKANJI

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "keys.h"
#include "props.h"
#include "prm.h"
#include "disp.h"
#include "sel.h"
#include "doc.h"
#include "file.h"
#include "debug.h"
#include "error.h"
#include "ch.h"
#include "core.h"
#include "fontwin.h"
#include "ibdefs.h"
#include "inter.h"
#include "message.h"
#include "opuscmd.h"
#include "prompt.h"
#define NOSTYCAB
#include "ruler.h"
#include "winddefs.h"
#include "doslib.h"
#include "wininfo.h"
#include "style.h"
#include "outline.h"

#include "idd.h"

#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"
#include "tmc.h"

#include "style.hs"
#include "style.sdm"
#include "edstyle.hs"
#include "para.hs"



#ifdef PROTOTYPE
#include "stylesub.cpt"
#endif /* PROTOTYPE */

/* E X T E R N A L S */
extern struct DOD **mpdochdod [];
extern struct SEL selCur;
extern struct CHP vchpFetch;
extern struct PAP vpapFetch;
extern struct CA caPara;
extern struct CA caSect;
extern struct MWD **hmwdCur;
extern struct MERR vmerr;
extern CP		vcpFetch;
extern struct STTB **vhsttbFont;
extern int vstcStyle;
extern int vdocFetch;
extern int fElActive;
extern KMP **hkmpCur;
extern BOOL vfRecording;
extern CHAR szEmpty[];
extern HWND vhwndApp;
extern struct SAB vsab;

/* declared in style.c */
extern int vstcSel;
extern int vfShowAllStd;
extern int vfDefineStyle;
extern int vstcBackup;
extern int ibstFontDS;
extern char (**vhmpiststcp)[];

extern uns **HCopyHeapBlock();
void ToggleShowStd();

/* G L O B A L S */
int vistLbMac;
int vdocStsh = docNil;

csconst char rgszStd[][] =
{
	SzKey("Normal",StyleNormal),      SzKey("Normal Indent",NormalIndent),
			SzKey("heading 1",heading1),      SzKey("heading 2",heading2),
			SzKey("heading 3",heading3),      SzKey("heading 4",heading4),
			SzKey("heading 5",heading5),      SzKey("heading 6",heading6),
			SzKey("heading 7",heading7),      SzKey("heading 8",heading8),
			SzKey("heading 9",heading9),      SzKey("footnote text",FootnoteText),
			SzKey("footnote reference",FootReference),  SzKey("header",header),
			SzKey("footer",footer),           SzKey("index heading",IndexHeading),
			SzKey("line number",LineNumber),  SzKey("index 1",index1),
			SzKey("index 2",index2),          SzKey("index 3",index3),
			SzKey("index 4",index4),          SzKey("index 5",index5),
			SzKey("index 6",index6),          SzKey("index 7",index7),
			SzKey("toc 1",toc1),              SzKey("toc 2",toc2),
			SzKey("toc 3",toc3),              SzKey("toc 4",toc4),
			SzKey("toc 5",toc5),              SzKey("toc 6",toc6),
			SzKey("toc 7",toc7),              SzKey("toc 8",toc8),
			SzKey("annotation text",AnnText), SzKey("annotation reference",AnnotationReference)
};


/* %%Function:FSetStcBackup %%Owner:davidbo */
FSetStcBackup()
{
	struct STTB **hsttbChpe, **hsttbPape;
	struct STSH stsh;

	Assert(vstcBackup != stcNil);
	Assert(vstcStyle != stcNil);

	RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);
	/* if docScrap uses this style, disengage from vdocStsh */
	if (vsab.docStsh == vdocStsh)
		CheckScrapStsh(vdocStsh);
	if (!FCopyStyleWithinStsh(vdocStsh, vstcStyle, vstcBackup))
		{
		InvalStyleEntry(&stsh, hsttbChpe, hsttbPape, StcpFromStc(vstcBackup, stsh.cstcStd));
		return fFalse;
		}
	return fTrue;
}


/*
*  return TRUE if there's a difference between the first run of properties
*  in the current selection and the properties in the current style.
*/
/* %%Function:FRedineCanWork %%Owner:davidbo */
FRedefineCanWork()
{
	int *pw1, *pw2, cw, cwMac;
	char *pb1, *pb2;
	struct CHP chp;
	struct PAP pap;

	CachePara(selCur.doc, selCur.cpFirst);
	FetchCp(selCur.doc, selCur.cpFirst, fcmProps);
	MapStc(PdodDoc(selCur.doc), vpapFetch.stc, &chp, &pap);

	/* compare CHPs */
	pw1 = (int *) &chp;
	pw2 = (int *) &vchpFetch;
	for (cw = 0; cw < cwCHP; cw++)
		if (*pw1++ != *pw2++)
			return TRUE;

	/* compare PAPs up to and including whatever's used in rgdxaTab */
	/* ignore PHE, ptap. ffTp and fIntable for this comparison bz 9/18/89 */

	pap.phe = vpapFetch.phe;
	pap.fInTable = vpapFetch.fInTable;
	pap.fTtp = vpapFetch.fTtp;
	pap.ptap = vpapFetch.ptap;

	pw1 = (int *) &pap;
	pw2 = (int *) &vpapFetch;
	cwMac = cwPAPBase + pap.itbdMac;
	for (cw = 0; cw < cwMac; cw++)
		if (*pw1++ != *pw2++)
			return TRUE;

	/* compare rgtbd array in PAPs */
	pb1 = &pap.rgtbd[0];
	pb2 = &vpapFetch.rgtbd[0];
	cwMac = pap.itbdMac;
	for (cw = 0; cw < cwMac; cw++)
		if (*pb1++ != *pb2++)
			return TRUE;

	return FALSE;
}


/* %%Function:FUseSelProps %%Owner:davidbo */
FUseSelProps(stcp, fErrorMsg)
int stcp, fErrorMsg;
{
	int fChange, stc, stcpSel, eid;
	struct STTB **hsttbChpe, **hsttbPape;
	struct STSH stsh;
	struct CHP chp, chpBackup;
	struct PAP papBackup;

	SetVdocStsh(selCur.doc);
	RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);
	stc = StcFromStcp(stcp, stsh.cstcStd);
	MapStc(PdodDoc(vdocStsh), stc, &chpBackup, &papBackup);

	CacheParaSel(&selCur);
	SetWords(&chp, 0, cwCHP);
	GetMajoritySelsChp(&selCur, &chp);
	stcpSel = StcpFromStc(vpapFetch.stc, stsh.cstcStd);

/* make sure the style we are fetching actually "exists" in a style sheet so
	that we can later generate it's name for the banter. if it doesn't, use the
	default stcBase (Normal). */
	if ((vpapFetch.stc > stcStdMin ||
			stcpSel < (*stsh.hsttbName)->ibstMac) && stc != vpapFetch.stc)
		(*stsh.hplestcp)->dnstcp[stcp].stcBase = vpapFetch.stc;

	if (!FCheckBasedOn(&stsh, &eid))
		{
		if (fErrorMsg)
			{
			ErrorEid(eid, "FUseSelProps");
			return fFalse;
			}
		else
			{
			stc = stcStdMin;
			(*stsh.hplestcp)->dnstcp[stcp].stcBase = stc;
			}
		}

	vpapFetch.stc = stc;
	SetBytes(&vpapFetch.phe, 0, sizeof(struct PHE));
	vpapFetch.phe.fStyleDirty = fTrue;
	vdocFetch = caPara.doc = docNil;
	chp.fSpec = fFalse;
	chp.fcPic = 0L;
	vpapFetch.fInTable = vpapFetch.fTtp = fFalse;

	if (!FStorePropeForStcp(&vpapFetch, stcp, hsttbPape, fFalse))
		goto LErrRet;

	if (!FStorePropeForStcp(&chp, stcp, hsttbChpe, fTrue))
		goto LRestoreDefn;

	if (!FGenChpxPapxNewBase(vdocStsh, stcp))
		{
LRestoreDefn:
		RestoreStcpDefn(vdocStsh, stcp, &chpBackup, &papBackup);
		goto LErrRet;
		}
	else  if (!FRecalcAllDependentChpePape(&stsh, hsttbChpe, hsttbPape, stcp))
		{
		RestoreStcpAndDependentsDefn(vdocStsh, stcp, &chpBackup, &papBackup);
		goto LErrRet;
		}
	return fTrue;

LErrRet:
	return fFalse;
}


/* %%Function:EmitSprmStc %%Owner:davidbo */
EmitSprmStc(stc)
int stc;
{
	char rgb[2];

	rgb[0] = sprmPStc;
	rgb[1] = stc;
	CmdApplySprmPStc(rgb, 2);
}


/* %%Function:FarSzToSt %%Owner:davidbo */
FarSzToSt(lpsz, st)
CHAR FAR *lpsz;
char *st;
{
	CHAR FAR *lpch = lpsz;
	int cch = 0;

	for ( ; *lpch != 0; cch++, lpch++)
		;
	bltbx(lpsz, (CHAR FAR *) &st[1], cch);
	st[0] = cch;
}


/* %%Function:IstLbFromStcp %%Owner:davidbo */
IstLbFromStcp(stcp)
int stcp;
{
	char *mpiststcp;
	int istLb;

	mpiststcp = *vhmpiststcp;
	for (istLb =  0; istLb < vistLbMac; istLb++)
		{
		if (mpiststcp[istLb] == stcp)
			return (istLb);
		}
	return istLbNil;
}


/*
*  Search stsh sttb for stName.  If found, set stc and fMatchStd as appropriate
*  and return TRUE.  Return FALSE otherwise.
*/
/* %%Function:FMatchDefinedAndStdStyles %%Owner:davidbo */
FMatchDefinedAndStdStyles(hsttb, cstcStd, stName, pstc, pfMatchStd)
struct STTB **hsttb;
int cstcStd;
char stName[];
int *pstc;
int *pfMatchStd;
{
	int stc;
	int stcp;
	int ibstMac = (*hsttb)->ibstMac;
	char st[cchMaxStyle];

	*pfMatchStd = fFalse;

	/* Look for match in defined style names */
	for (stcp = StcpFromStc(1,cstcStd); stcp < ibstMac; stcp++)
		{
		if (!FStcpEntryIsNull(hsttb, stcp) && (vstcBackup == stcNil || stcp != StcpFromStc(vstcBackup, cstcStd)))
			{
			GetStFromSttb(hsttb, stcp, st);
			if (FEqNcSt(st, stName))
				{
				*pstc = StcFromStcp(stcp, cstcStd);
				return fTrue;
				}
			}
		}

	/* Look for match in standard style names */
	if (FMatchStdStyles(stName, pstc))
		{
		*pfMatchStd = fTrue;
		return fTrue;
		}

	return fFalse;
}


/* %%Function:FMatchStdStyles %%Owner:davidbo */
FMatchStdStyles(stName, pstc)
char stName[];
int *pstc;
{
	int isz;
	char st[cchMaxStyle];

	for (isz = 0; isz <  cstcMax - stcStdMin ; isz++)
		{
		FarSzToSt((CHAR FAR *) rgszStd[isz], st);
		if (FEqNcSt(st, stName))
			{
			*pstc = (cstcMax - isz) & 255;
			return fTrue;
			}
		}
	return fFalse;
}


/* %%Function:FSearchStc %%Owner:davidbo */
FSearchStc(rgstc, stc, istcMac, pistc)
char rgstc[];
int stc;
int istcMac;
int *pistc;
{
	int stcGuess;
	int istcMin = 0;

	int istcLim = istcMac;
	int istcGuess = istcMin;
	while (istcMin < istcLim)
		{
		istcGuess = (istcMin + istcLim) >> 1;
		if (stc == (stcGuess = rgstc[istcGuess]))
			return *pistc = istcGuess, TRUE;
		else  if (stc < stcGuess)
			istcLim = istcGuess;
		else
			istcMin = ++istcGuess;
		}
	return *pistc = istcGuess, FALSE;
}


/*
*  Mark the specified combo box as needing to be refilled the next time it
*  is dropped down.  If tmc == tmcNull, both combos will be marked.
*/

/* %%Function:RefillDSDropDown %%Owner:davidbo */
RefillDSDropDown()
{
	RedisplayTmc(tmcDSBasedOn);
	RedisplayTmc(tmcDSNext);
}


/* %%Function:SetTmcBanterFromTmcCb %%Owner:davidbo */
SetTmcBanterFromTmcCb(tmcBanter, tmcCb, pstsh, hsttbChpe, hsttbPape)
int tmcBanter, tmcCb;
struct STSH *pstsh;
struct STTB **hsttbChpe;
struct STTB **hsttbPape;
{
	int stc;
	int fMatchStd;
	char st[cchMaxSz];

	GetTmcText(tmcCb, st, cchMaxStyle+1);
	SzToStInPlace(st);
	/* If not a previously unused std style, bag out... */
	if (FMatchDefinedAndStdStyles(pstsh->hsttbName, pstsh->cstcStd, st, &stc, &fMatchStd))
		{
		SetIbstFontDSFromStc(stc);
		GenStyleBanter(st, cchMaxSz - 1, stc, pstsh, hsttbChpe, hsttbPape);
		StToSzInPlace(st);
		}
	else
		st[0] = 0;
	SetTmcText(tmcBanter,st);
}


/* %%Function:BanterToTmc %%Owner:davidbo */
BanterToTmc(tmc, stc, pstsh, hsttbChpe, hsttbPape)
int tmc;
int stc;
struct STSH *pstsh;
struct STTB **hsttbChpe;
struct STTB **hsttbPape;
{
	char sz[cchMaxSz];

	GenStyleBanter(sz, cchMaxSz - 1, stc, pstsh, hsttbChpe, hsttbPape);
	StToSzInPlace(sz);
	SetTmcText(tmc, sz);
}


/* %%Function:GenStyleBanter %%Owner:davidbo */
GenStyleBanter(st, cch, stc, pstsh, hsttbChpe, hsttbPape)
char st[];
int stc, cch;
struct STSH *pstsh;
struct STTB **hsttbChpe;
struct STTB **hsttbPape;
{
	int stcp, cstcStd, stcBase, stcpBase;
	struct DOD *pdod;
	struct CHP chp;
	struct CHP chpBase;
	struct PAP pap;
	struct PAP papBase;

	stcp = StcpFromStc(stc, pstsh->cstcStd);
	if (stcp >= 0 && stcp < (*pstsh->hsttbName)->ibstMac)
		stcBase = ((struct ESTCP *)PInPl(pstsh->hplestcp, stcp))->stcBase;
	else
		stcBase = stcNormal;

	pdod = PdodDoc(vdocStsh);

	MapStc(pdod, stc, &chp, &pap);

	if (stcBase != stcStdMin)
		MapStc(pdod, stcBase, &chpBase, &papBase);
	else
		{
		SetBytes(&chpBase, 0, cbCHP);
		SetBytes(&papBase, 0, cbPAP);
		/* fill in impossible value so we always find a difference
			for jc */
		papBase.jc = 255;
		}
	papBase.stc = stcBase;

	CchFillStWithBanter(st, cch, &chp, &chpBase, &pap, &papBase, ibstFontDS);
}


/*
*  OpenStsh Merges the style sheet docStshNew with that of vdocStsh.
*  If fNuke is TRUE, docStshNew can be disposed at the end of the merge.
*  If fFrom is TRUE, vdocStsh is the destination doc and docStshNew is
*  the source doc.
*/
/* %%Function:OpenStsh %%Owner:davidbo */
OpenStsh(docStshNew, fNuke, fFrom)
int docStshNew, fNuke, fFrom;
{
	int i, iMac, stcpSrc, ftcSrc, ftcDest, idError;
	struct DOD *pdod;
	struct STTB **hsttbChpeSrc, **hsttbPapeSrc;
	struct STTB **hsttbChpeDest, **hsttbPapeDest;
	struct STSH stshSrc;
	struct STSH stshDest;
	char rgb[cstcMax + 2];

	SetVdocStsh(selCur.doc);
	Assert(vdocStsh != docStshNew);

	/* fun and games. We wish the stsh of the doc we are reading to take
		precedence over the stsh in vdocStsh. FCopyStyleToDestStsh gives
		precedence to the destination stsh, so we record the original
		stsh in vdocStsh as the source stsh, and move the stsh we are
		reading directly into vdocStsh before making the call. */

	RecordStshForDoc(vdocStsh, &stshSrc, &hsttbChpeSrc, &hsttbPapeSrc);

	/* clean up backup entry */
	if (vstcBackup != stcNil)
		{
		InvalStyleEntry(&stshSrc, hsttbChpeSrc, hsttbPapeSrc, StcpFromStc(vstcBackup, stshSrc.cstcStd));
		vstcBackup = stcNil;
		}

	/* And this is where we get really sleazy...  If Merging To a file from
		* the current doc, switch vdocStsh and docStshNew.  Are you dizzy?
		*/
	if (!fFrom)
		{
		int docT;

		Assert(!fNuke);
		docT = vdocStsh;
		vdocStsh = docStshNew;
		docStshNew = docT;
		RecordStshForDoc(vdocStsh, &stshSrc, &hsttbChpeSrc, &hsttbPapeSrc);
		}

	if (fNuke)
		{
		Assert(fFrom);
		RecordStshForDoc(docStshNew, &stshDest, &hsttbChpeDest, &hsttbPapeDest);
		pdod = PdodDoc(docStshNew);
		/* We have transferred the stsh information that was in docStshNew to
		 * stshDest, which will then be transferred to vdocStsh.  We zero out
		 * docStshNew's stsh information here so that DisposeDoc does not
		 * deallocate that stuff when we DisposeDoc on docStshNew.
		 */
		SetBytes(&pdod->stsh, 0, cbSTSH);
		pdod->hsttbChpe = 0;
		pdod->hsttbPape = 0;
		}
	else
		{
		struct STSH stsh;
		struct STTB **hsttbChpe, **hsttbPape;

		SetBytes(&stshDest, 0, cbSTSH);
		hsttbChpeDest = hsttbPapeDest = hNil;
		RecordStshForDoc(docStshNew, &stsh, &hsttbChpe, &hsttbPape);
		stshDest.cstcStd = stsh.cstcStd;
		/* HCopyHeapBlock does not report out of memory...check it ourselves */
		if ((stshDest.hsttbName =HCopyHeapBlock(stsh.hsttbName,strcSTTB))==hNil ||
			(stshDest.hsttbChpx = HCopyHeapBlock(stsh.hsttbChpx,strcSTTB))==hNil ||
			(stshDest.hsttbPapx = HCopyHeapBlock(stsh.hsttbPapx,strcSTTB))==hNil ||
			(stshDest.hplestcp = HCopyHeapBlock(stsh.hplestcp,strcPL)) == hNil ||
			(hsttbChpeDest = HCopyHeapBlock(hsttbChpe,strcSTTB)) == hNil ||
			(hsttbPapeDest = HCopyHeapBlock(hsttbPape,strcSTTB)) == hNil)
			{
			FreeHsttb(stshDest.hsttbName);
			FreeHsttb(stshDest.hsttbChpx);
			FreeHsttb(stshDest.hsttbPapx);
			FreeH(stshDest.hplestcp);
			FreeHsttb(hsttbChpeDest);
			FreeHsttb(hsttbPapeDest);
			ErrorNoMemory(eidNoMemMerge);
			return;
			}
		}

	/* Make sure all handles are valid before continuing */
	AssertH(stshDest.hsttbName);
	AssertH(stshDest.hsttbChpx);
	AssertH(stshDest.hsttbPapx);
	AssertH(stshDest.hplestcp);
	AssertH(hsttbChpeDest);
	AssertH(hsttbPapeDest);

	pdod = PdodDoc(vdocStsh);
	pdod->stsh = stshDest;
	pdod->hsttbChpe = hsttbChpeDest;
	pdod->hsttbPape = hsttbPapeDest;

	/* fun and games part 2: windows fonts. Well, boys and girls, in the
		Windows world, ftc font codes are not absolute numbers as they are
		for mac. The stsh we so gleefully stuffed into vdocStsh may have
		ftc's incompatible with that doc. Normally, we map the stsh fonts
		in FCopyStyleToDestStsh, but since we are engaging in trickery
		here, it is easier to do it here first.
			
		So, loop through the stcp's in stshDest, now copied into vdocStsh,
		getting the chpe and checking for non-default fonts, and map any
		that differ in the 2 docs.
		To avoid unneeded copies, the stshDest and hsttbChpeDest are used, but
		refer to the copies of those structures copied into vdocStsh, and are
		considered the source for the mapping into the ftc space of
		destination vdocStsh.
	
		Then we pass docNil as docSrc to FCopyStyleToDestStsh, indicating no
		font mapping to be done.
	*/

	Assert (vdocStsh != docNil);
	iMac =  (*stshDest.hplestcp)->stcpMac;
	for (i = 0; i < iMac; i++)
		{
		CHAR HUGE *hpst;
		stcpSrc = i;
		if (FStcpEntryIsNull(stshDest.hsttbName, stcpSrc))
			continue;

		if (*(hpst = HpstFromSttb(hsttbChpeDest, stcpSrc)) != 255)
			{
			if (*hpst >= 4)    /* ftc, if any, in word 2 */
				{
				ftcSrc = ((struct CHP HUGE *)(hpst+1))->ftc;
				if ( ftcSrc != ftcDefault )
					{
					/* Heap Movement */
					ftcDest = FtcMapDocMotherFtc(docStshNew, vdocStsh,
							ftcSrc);
					if (ftcDest != ftcSrc)
						{
						((struct CHP HUGE *)
								(HpstFromSttb(hsttbChpeDest, stcpSrc)+1))->ftc
								= ftcDest;
#ifdef DSTYFTC
						CommSzNum(SzShared("openstsh map ftc: ftcSrc: "), ftcSrc);
						CommSzNum(SzShared("openstsh map ftc: ftcDest: "), ftcDest);
#endif
						}
					}
				}
			}
		}  /* for */


	if (FCopyStyleToDestStsh(docNil, &stshSrc, hsttbChpeSrc,
			hsttbPapeSrc, vdocStsh, &stshDest, hsttbChpeDest, hsttbPapeDest,
			0, 0, rgb, fFalse, fFalse, fTrue, &idError))
		{
		int stcp, stcpMac, stc;
		struct CHP chp;
		struct PAP pap;

		stcpMac = (**stshDest.hsttbName).ibstMac;
		for (stcp = 0; stcp < stcpMac; stcp++)
			{
			if (!FStcpEntryIsNull(stshDest.hsttbName, stcp) &&
				!FStcpEntryIsNull(hsttbPapeDest, stcp))
				{
				stc = StcFromStcp(stcp, stshDest.cstcStd);
				SetBytes(((char *)&pap) + cbPAPBase, 0, cbPAP-cbPAPBase);
				MapStc(PdodDoc(vdocStsh), stc, &chp, &pap);
				pap.phe.fStyleDirty = fTrue;
				/*
				*  If both chpe and pape are originally nil, we need to expand
				*  both.  Having one nil and one not nil is illegal and
				*  punishable by death.
				*/
				if (!FStorePropeForStcp(&chp, stcp,hsttbChpeDest,fTrue/*CHP*/))
					goto LNoRoom;
				if (!FStorePropeForStcp(&pap, stcp,hsttbPapeDest,fFalse/*PAP*/))
					goto LNoRoom;
				InvalPadPgdForStc(vdocStsh, stc);
				}
			}
		if (rgb[0] != 0)
			ApplyGrpprlDocSubdocs(vdocStsh, rgb, rgb[1] + 2);
		FreeHsttb(stshSrc.hsttbName);
		FreeHsttb(stshSrc.hsttbChpx);
		FreeHsttb(stshSrc.hsttbPapx);
		FreeH(stshSrc.hplestcp);
		FreeHsttb(hsttbChpeSrc);
		FreeHsttb(hsttbPapeSrc);
		}
	else
		{
		/* Copy ran out of room...restore old stsh */
LNoRoom:
		FreeHsttb(stshDest.hsttbName);
		FreeHsttb(stshDest.hsttbChpx);
		FreeHsttb(stshDest.hsttbPapx);
		FreeH(stshDest.hplestcp);
		FreeHsttb(hsttbChpeDest);
		FreeHsttb(hsttbPapeDest);
		pdod = PdodDoc(vdocStsh);
		pdod->stsh = stshSrc;
		pdod->hsttbChpe = hsttbChpeSrc;
		pdod->hsttbPape = hsttbPapeSrc;
		if (idError == -1)
			ErrorNoMemory(eidNoMemMerge);
		else
			ErrorEid(eidStshFull, "");
		}

	if (fNuke)
		{
		DisposeDoc(docStshNew);
		KillExtraFns();
		}
}


/* C O P Y  S T Y L E S  1 */
/* %%Function:CopyStyles1 %%Owner:davidbo */
CopyStyles1(docSrcStsh, docDest, cpDest, dcp)
CP dcp, cpDest;
{
	CP cpLim;
	int ipap;
	int istc;
	int istcCopyMac;
	int cstcSrc, cstcDest;
	int cstcSrcEst;
	int i, iFirst, iMac, idError;
	int stcpSrc, stcSrc;
	int stcpDest;
	CP cpRefFirst, cpRefLim;
	CP cpFtnTextFirst, cpFtnTextLim;
	CP cpAtnTextFirst, cpAtnTextLim;
	int fApplyToFtn = fFalse;
	int fApplyToAtn = fFalse;
	int fApplyToHdr = fFalse;
	int ihddFirst, ihddLim;
	int docHdr;
	int fLowMem = fFalse;
	struct PLCHDD **hplchdd;
	CP  cpHddFirst, cpHddLim;
	struct STSH stshSrc;
	struct STTB **hsttbChpeSrc, **hsttbPapeSrc;
	struct STSH stshDest;
	struct STTB **hsttbChpeDest, **hsttbPapeDest;
	char *pstcInsert;
	int docDestMother = DocMother(docDest);

	struct CA ca, caT;
	struct CA caInval;
	char grpprl[258];

	char rgstcCopy[ipapStcScanLim];

	cpLim = cpDest + dcp;
	CachePara(docDest, cpDest);

	ipap = 0;
	istcCopyMac = 0;
	grpprl[0] = 0;

/* if we're in the main doc, and it has a footnote subdoc, check if the
	text we're scanning contains any footnotes. if it does, setup so we
	copy all of the source styles to the destination. */
	if (docDest == docDestMother)
		{
		if (PdodDoc(docDest)->docFtn != docNil)
			{
			cpFtnTextFirst = CpFirstRef(docDest, cpDest, &cpRefFirst, edcDrpFtn);
			cpFtnTextLim = CpFirstRef(docDest, cpLim, &cpRefLim, edcDrpFtn);
			if (cpRefFirst != cpRefLim)
				{
				fApplyToFtn = fTrue;
				ipap++;
				}
			}
/* if we're in the main doc, and it has an annotation subdoc, check if the
	text we're scanning contains any anotations. if it does, setup so we
	copy all of the source styles to the destination. */
		if (PdodDoc(docDest)->docAtn != docNil)
			{
			cpAtnTextFirst = CpFirstRef(docDest, cpDest, &cpRefFirst, edcDrpAtn);
			cpAtnTextLim = CpFirstRef(docDest, cpLim, &cpRefLim, edcDrpAtn);
			if (cpRefFirst != cpRefLim)
				{
				fApplyToAtn = fTrue;
				ipap++;
				}
			}
/* if the text we're scanning contains any section marks, set so we copy all
	of the source styles to the destination, so we can do the proper mapping
	for any attached headers/footers. */
		if (PdodDoc(docDest)->docHdr != docNil)
			{
			CacheSect(docDest, cpDest);
			if (caSect.cpLim <= cpLim)
				{
				ipap++;
				fApplyToHdr = fTrue;
				}
			}
		if (fApplyToFtn || fApplyToAtn || fApplyToHdr)
			goto LEstimateSize;
		}

/* now we attempt to scan the text and catalog which styles are in use. We
	give up if there are too many paragraphs. */
	while (caPara.cpLim <= cpLim && ipap < ipapStcScanLim)
		{
		if (!FSearchStc(rgstcCopy, vpapFetch.stc, istcCopyMac, &istc))
			{
			pstcInsert = rgstcCopy + istc;
			bltb(pstcInsert, pstcInsert + 1, (istcCopyMac++) - istc);
			*pstcInsert = vpapFetch.stc;
			}
		ipap++;
		if (caPara.cpLim >= cpLim)
			break;
		CachePara(docDest, caPara.cpLim);
		}
LEstimateSize:
/* if the text contains more than one paragraph mark, we must try to copy
	styles from the source doc to the destination doc. We estimate
	how much work we'll have to do for the style merge. If it's too much
	we'll ask the user if he wants to wait around for the merge to complete. */
	if (ipap != 0)
		{
		RecordStshForDoc(DocMother(docSrcStsh), &stshSrc, &hsttbChpeSrc, &hsttbPapeSrc);
		RecordStshForDoc(docDestMother, &stshDest, &hsttbChpeDest, &hsttbPapeDest);

/* if we couldn't reach the end of the text in the time we allowed for the
	the scan, we give up on space-efficency and signal that we want all of the
	source styles to be copied. */
		if (caPara.cpLim < cpLim || istcCopyMac == ipapStcScanLim)
			istcCopyMac = 0;
		cstcSrc = (*stshSrc.hplestcp)->stcpMac;
		cstcDest = (*stshDest.hplestcp)->stcpMac;
/* if we were able to enumerate all of the styles used in the text or there
	are fewer styles in the source than in the destination, we check the
	source and dest style sheet to see if the styles we need to copy exist
	in both stylesheets in the same stc slots. If so, we needn't continue
	the style copy. */
		if (istcCopyMac != 0 ||
				(cstcSrc - stshSrc.cstcStd <= cstcDest - stshDest.cstcStd))
			{
			if (istcCopyMac)
				{
				iFirst = 0;
				iMac = istcCopyMac;
				}
			else
				{
				iFirst = stshSrc.cstcStd;
				iMac = cstcSrc;
				}
			for (i = iFirst; i < iMac; i++)
				{
				if (istcCopyMac == 0)
					{
					stcpSrc = i;
					stcSrc = StcFromStcp(stcpSrc, stshSrc.cstcStd);
					}
				else
					{
					stcSrc = rgstcCopy[i];
					stcpSrc = StcpFromStc(stcSrc, stshSrc.cstcStd);
					}
				stcpDest = StcpFromStc(stcSrc, stshDest.cstcStd);

				if (stcpDest >= cstcDest)
					break;

				if (stcpSrc >= cstcSrc ||
					 FStcpEntryIsNull(stshSrc.hsttbName, stcpSrc))
					{
					if (!FStcpEntryIsNull(stshDest.hsttbName, stcpDest))
						break;
					}
				else
					{
					char stSrc[256];
					char stDest[256];
					GetStFromSttb(stshSrc.hsttbName, stcpSrc, stSrc);
					GetStFromSttb(stshDest.hsttbName, stcpDest, stDest);
					if (FNeNcSt(stSrc, stDest))
						break;
					}
				}
/* if exact match, we need to ensure the styles are physically recorded
	in the style sheet. */
			if (i == iMac)
				{
				iMac = (istcCopyMac != 0) ? istcCopyMac : stshSrc.cstcStd;
				for (i = 0; i < iMac; i++)
					{
					if (istcCopyMac == 0)
						stcSrc = StcFromStcp(i, stshSrc.cstcStd);
					else
						{
						stcSrc = rgstcCopy[i];
						if (StcpFromStc(stcSrc, stshSrc.cstcStd) > stshSrc.cstcStd)
							continue;
						}
					FEnsureStcDefined(docDestMother, stcSrc, &stshDest.cstcStd);
					}
				return;
				}
			}

		StartLongOp();
/* copy styles and generate permutation sprm */
		if (!FCopyStyleToDestStsh(docSrcStsh, &stshSrc, hsttbChpeSrc,
				hsttbPapeSrc, docDestMother, &stshDest, hsttbChpeDest, hsttbPapeDest,
				rgstcCopy, istcCopyMac, grpprl, fFalse, fFalse, fFalse, &idError))
			{
			if (idError == -1)
				ErrorNoMemory(eidNoMemMerge);
			else
				ErrorEid(eidStshFullCopy,"CopyStyles1");
			}
		EndLongOp(fFalse);
		}

	if (grpprl[0] != 0)
		{
		ca.cpFirst = cpDest;
		ca.cpLim = cpDest + dcp;
		ca.doc = docDest;
		ApplyStcPermuteCa(&ca, grpprl);
		if (fApplyToFtn)
			{
			ca.cpFirst = cpFtnTextFirst;
			ca.cpLim = cpFtnTextLim;
			ca.doc = (PdodDoc(docDest))->docFtn;
			ApplyStcPermuteCa(&ca, grpprl);
			}
		if (fApplyToAtn)
			{
			ca.cpFirst = cpAtnTextFirst;
			ca.cpLim = cpAtnTextLim;
			ca.doc = PdodDoc(docDest)->docAtn;
			ApplyStcPermuteCa(&ca, grpprl);
			}
		if (fApplyToHdr)
			{
			ihddLim = IhddFromDocCp(PcaSetDcp(&caT, docDest, cpDest, dcp),
					&ihddFirst, 0);
			docHdr = PdodDoc(docDest)->docHdr;
			hplchdd = PdodDoc(PdodDoc(docDest)->docHdr)->hplchdd;
			cpHddFirst = CpPlc(hplchdd, ihddFirst);
			cpHddLim = CpPlc(hplchdd, ihddLim);
			if (cpHddFirst < cpHddLim)
				{
				ca.cpFirst = cpHddFirst;
				ca.cpLim = cpHddLim;
				ca.doc = docHdr;
				ApplyStcPermuteCa(&ca, grpprl);
				}
			}
		}
	if (fLowMem)
		SetErrorMat(matLow);
}


/* %%Function:ApplyStcPermuteCa %%Owner:davidbo */
ApplyStcPermuteCa(pca, grpprl)
struct CA *pca;
char *grpprl;
{
/*
* can't apply permutation to the paragraph mark that follows the selection
* so we have to do our own version of the processing done in
* ApplyGrpprlSelCur. main difference is we don't do ExpandCaSprm.
*/
	struct CA caInval;

	FlushRulerSprms();
	CachePara(pca->doc, pca->cpFirst);
	Assert(caPara.cpFirst <= pca->cpFirst && caPara.cpLim <= pca->cpLim);
	caInval = *pca;
	pca->cpFirst = CpMax(caPara.cpFirst, caPara.cpLim - ccpEop);
/* if the CA doesn't bracket a paragraph bound, don't bother to apply grpprl.*/
	if (pca->cpFirst < pca->cpLim)
		{
		InvalCp(&caInval);
		Assert(grpprl[0] == sprmPStcPermute ||
				(grpprl[0] == sprmPStc && grpprl[1] == 0));
		ApplyGrpprlCa(grpprl, grpprl[1]+2, pca);
		}
}


/* %%Function:FtcMapDocMotherFtc %%Owner:davidbo */
FtcMapDocMotherFtc(docMotherSrc, docMotherDest, ftcSrc)
int docMotherSrc;
int docMotherDest;
int ftcSrc;
{
	int ibst;
	struct DOD *pdodSrc;
	int ftcDest;

	pdodSrc = PdodDoc(docMotherSrc);

	if (ftcSrc >= pdodSrc->ftcMac)
		{
		ReportSz("style sheet contains out of range ftcs");
		ftcSrc = ftcDefault;
		}

	ibst = (**pdodSrc->hmpftcibstFont) [ftcSrc];

	/* heap movement possible */

	ftcDest = FtcFromDocIbst(docMotherDest, ibst);

	/* test below handles ftcDest == valNil too,
		indicating we could not add the ftc to docDest */

	if ( ftcDest < 0 || ftcDest > 0xFE ) /* out of our range */
		{
		/* no error reporting needed; this should never happen; if it
			does a reasonable action occurs, and we will catch it in debug
			with the assert. (bz)
		*/
		Assert (fFalse);
		ftcDest = ftcDefault;
		}

	return (ftcDest);
}



/* %%Function:FCheckBasedOn %%Owner:davidbo */
FCheckBasedOn(pstsh, peid)
struct STSH *pstsh;
int *peid;
{
	int stc, stcp, stcpMac, cstc, stcBase, stcNext;
	char mpstccstc[256];

	/* first check for circular references... */
	*peid = eidCircularBasedOn;
	stcp = StcpFromStc(vstcStyle, pstsh->cstcStd);
	cstc = 0;
	do
		{
		GetStcBaseNextForStcp(pstsh, stcp, &stcBase, &stcNext);
		stcp = StcpFromStc(stcBase, pstsh->cstcStd);
		cstc++;
		if (cstc >= 2 * cstcMaxBasedOn)
			return fFalse;
		}
	while (stcBase != stcStdMin);

	/* Set map to unknown state, expcept for stcStdMin whose length is 0 */
	SetWords(mpstccstc, 0xffff, 128);
	mpstccstc[stcStdMin] = 0;

	/* ...now check for any chain too long */
	*peid = eidBasedOnFull;
	stc = StcFromStcp(0, pstsh->cstcStd);
	stcpMac = (*pstsh->hsttbChpx)->ibstMac;
	for (stcp = 0; stcp < stcpMac; stcp++, stc++)
		{
		stc &= 0xff;
		/* don't bother checking backup or unused standards */
		if (stc == vstcBackup || (FStcpEntryIsNull(pstsh->hsttbName, stcp) &&
				stc != vstcStyle))
			continue;

		cstc = mpstccstc[stc];
		if (cstc != 0xff)
			{
			Assert(mpstccstc[stc] < cstcMaxBasedOn);
			continue;
			}

		if ((cstc = CstcBasedOnOfStc(stc, pstsh, mpstccstc)) < 0)
			break;
		}

	return (stcp == stcpMac);
}


/*
*  Fills in mpstccstc for stc and all of the unknowns in its based on
*  chain.  Returns the length of the based on chain for stc.  Returns
*  -1 if the chain is too long.
*/
/* %%Function:CstcBasedOnOfStc %%Owner:davidbo */
CstcBasedOnOfStc(stc, pstsh, mpstccstc)
int stc;
struct STSH *pstsh;
char mpstccstc[];
{
	int cstcBasedOn, stcp, stcBase, stcNext;

	stcp = StcpFromStc(stc, pstsh->cstcStd);
	GetStcBaseNextForStcp(pstsh, stcp, &stcBase, &stcNext);
	cstcBasedOn = mpstccstc[stcBase];
	if (cstcBasedOn == 0xff)
		{
		if ((cstcBasedOn = CstcBasedOnOfStc(stcBase, pstsh, mpstccstc)) < 0)
			return cstcBasedOn;
		}
	cstcBasedOn++;
	if (cstcBasedOn >= cstcMaxBasedOn)
		return (-1);
	return (mpstccstc[stc] = cstcBasedOn);
}



/* %%Function:ApplyRulerStyle %%Owner:davidbo */
ApplyRulerStyle(szStyle, fSysModalMsg)
char *szStyle;
BOOL fSysModalMsg;
{
	extern int cmmCache;
	CMB cmb;

	if (!FInitCmb(&cmb, bcmApplyStyleDlg, hNil, cmmAction | cmmBuiltIn))
		return;

	if (!FSetCabSz(cmb.hcab, szStyle, Iag(CABAPPLYSTYLE, hszASStyle)))
		return;

	/* execute Action only...cab is already set up. */
	cmb.wParam = fSysModalMsg;
	cmb.tmc = tmcOK;
	if (CmdExecCmb(&cmb) == cmdOK)
		{
		SetAgainCab(bcmApplyStyleDlg, cmb.hcab);
		if (vfRecording)
			FRecordCab(cmb.hcab, IDDApplyStyle, tmcOK, bcmApplyStyleDlg);
		}
	else
		FreeCab(cmb.hcab); /* cab normally freed by SetAgainCab() */
}


/*
*  Apply the style, stStyle, to the current selection.  Deal with 
*  DefineByExample and RedefineByExample if necessary.  When coming
*  from ApplyStylesDlg, update banter (fBanter is fTrue).  When coming
*  from FormatParaDlg, don't apply style (fApply is fFalse).  Return
*  stc in pstc for Para and Ruler.  If a redef by example can be done,
*  fRedefOk set to true, else if def by example, set false.  Returns
*  IDYES/NO/CANCEL in answer to message box or -1 for bad style name
*  or mem failure.
*/
/* %%Function:IdApplyStyle %%Owner:davidbo */
IdApplyStyle(pstc, stStyle, fBanter, fApply, pfRedefOk, fIdYes, fSysModal)
int *pstc, fBanter, fApply, *pfRedefOk, fIdYes, fSysModal;
char *stStyle;
{
	int stc, stcMatch, stcp, mb, id;
	int fMatch, fMatchStd, fRedef;
	struct STSH stsh;
	struct STTB **hsttbChpe, **hsttbPape;
	char *psz;
	char szStyle[cchMaxStyle+1];

	id = IDYES;
	SetVdocStsh(selCur.doc);
	SetVstcSel();
	RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);

	if (!FValidStyleName(stStyle))
		{
		ErrorEid(fSysModal ? eidBadStyleSysModal : eidBadStyle, "IdApplyStyle");
		id = -1;
		goto LErrRet;
		}

	fMatch = FMatchDefinedAndStdStyles(stsh.hsttbName, stsh.cstcStd, stStyle, &stcMatch, &fMatchStd);
	fRedef = *pfRedefOk && fMatch && vstcSel == stcMatch && FRedefineCanWork();
	*pfRedefOk = fRedef;  /* let caller know whether we did redef or def */

	if (!fMatch || fRedef)
		/*  user requested undefined style, define it for them */
		/*  OR user is redefining an already defined style */
		{
		if (!fElActive)
			{
			StToSz(stStyle, szStyle);
			psz = szStyle;
			/* SystemModal and Hand required because of activation order
				problems */
			mb = fSysModal ? MB_ICONHAND|MB_DEFBUTTON1|MB_SYSTEMMODAL
					: MB_ICONQUESTION|MB_DEFBUTTON1;
			mb |= (fRedef || fBanter) ? MB_YESNOCANCEL : MB_YESNO;
			id = IdMessageBoxMstRgwMb(fRedef ? mstRedefineStyle : mstDefineUndefSty, &psz, mb);
			}
		else
			{
			id = fIdYes ? IDYES : IDNO;
			}

		switch (id)
			{
		default:
		case IDCANCEL:
			goto LErrRet;
		case IDYES:
			if (fRedef)
				stc = stcMatch;
			break;
		case IDNO:
			if (fRedef)
				{
				stc = stcMatch;
				fRedef = fFalse;
				goto LApply;
				}
			else
				return id;
			}

		stcp = fRedef ? StcpFromStc(stcMatch, stsh.cstcStd) :
				StcpCreateNewStyle (vdocStsh, fFalse /* fUseReserve */);
		if (stcp == stcpNil)
			{
			if (!vmerr.fMemFail)
				ErrorEid(eidStshFull, "IdApplyStyle");
			goto LOom;
			}
		stc = vstcStyle = StcFromStcp(stcp, stsh.cstcStd);

		if (!FUseSelProps(stcp, fTrue) ||
				(!fRedef && !FChangeStInSttb(stsh.hsttbName, stcp, stStyle)))
			{
			MakeStcpEntryNull(stcp, &stsh, hsttbChpe, hsttbPape);
			id = -1;
			goto LOom;
			}
		else  if (fApply)
			{
			char prl[2];
			struct CHP chp, chpe;
			struct PAP pap, pape;

			CacheParaSel(&selCur);
			pap = vpapFetch;
			GetMajoritySelsChp(&selCur, &chp);
			CheckForInvalPadPgd(&stsh, hsttbChpe, hsttbPape);
			InvalDoc(vdocStsh);

			/* for Redef, apply now...EmitSprm does Undo which has no
				meaning for Redef */
			if (fRedef)
				{
				prl[0] = sprmPStc;
				prl[1] = stc;
				ApplyGrpprlSelCur(prl, 2, fFalse);
				SetUndoNil();
				fApply = fFalse;
				prl[0] = sprmCPlain;
				ApplyGrpprlSelCur(prl, 1, fFalse);
				if (stc < stcLevMin || stc > stcLevLast ||
					stcLevLast - stc > PdodMother(selCur.doc)->lvl)
					PdodMother(selCur.doc)->lvl = lvlNone;

				EmitSprmCMajority(&chp);
				if (fRedef && selCur.fIns)
					GetSelCurChp(fTrue);
				if (FDestroyParaHeight(&pap, &vpapFetch))
					InvalPageView(selCur.doc);
				}
			}
		stc = vstcStyle;
		PdodDoc(vdocStsh)->fStshDirty = fTrue;
		}
	else
		{
		if (fMatchStd && !FEnsureStcDefined(vdocStsh, stcMatch, &stsh.cstcStd))
			{
LOom:
			id = -1;
			goto LErrRet;
			}
		stc = stcMatch;
		}

LApply:
	if (fApply)
		EmitSprmStc(stc);

LRet:
	*pstc = stc;
	return id;

LErrRet:
	if (fBanter)
		SetTmcBanterFromTmcCb(tmcASBanter,tmcASStyle,&stsh,hsttbChpe,hsttbPape);
	return id;
}


/* %%Function:SetVstcSel %%Owner:davidbo */
SetVstcSel()
{
	int ipap;
	CP cpLim;

	CachePara(selCur.doc, selCur.cpFirst);
	cpLim = selCur.cpLim;
	vstcSel = vpapFetch.stc;
	for (ipap = 0; ipap < ipapStcScanLim; ipap++)
		{
		if (vstcSel != vpapFetch.stc || caPara.cpLim >= cpLim)
			break;
		CachePara(selCur.doc, caPara.cpLim);
		}
	if (vstcSel != vpapFetch.stc || caPara.cpLim < cpLim)
		vstcSel = stcNil;
}


/* %%Function:ToggleShowStd %%Owner:davidbo */
void ToggleShowStd()
{
	int tmc, istLb, hid;
	int stcp, stc;
	CESTY *pcesty;
	struct STTB **hsttbChpe, **hsttbPape;
	struct STSH stsh;
	char sz[cchMaxSz];

	sz[0] = 0;
	hid = HidOfDlg(hdlgNull);
	pcesty = 0;
	switch (hid)
		{
	case IDDParaLooks:
		tmc = tmcParStyle;
		break;
	case IDDApplyStyle:
		tmc = tmcASStyle;
		break;
	case IDDDefineStyle:
		pcesty = PcmbDlgCur()->pv;
		tmc = tmcDSStyle;
		break;
	default:
		Assert(fFalse);
		return;
		}

	istLb = ValGetTmc((tmc & ~ftmcGrouped) + 1);
	stc = stcNil;

	vfShowAllStd = !vfShowAllStd;
	SetVdocStsh(selCur.doc);
	RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);
	if (istLb != istLbNil)
		stc = StcFromStcp((**vhmpiststcp)[istLb], stsh.cstcStd);
	else
		GetTmcText(tmc, sz, cchMaxSz);
	GenLbStyleMapping();
	if (vfDefineStyle)
		{
		if (pcesty)
			pcesty->fIgnore = fTrue;
		RefillDSDropDown();
		if (pcesty)
			pcesty->fIgnore = fFalse;
		}
	if (stc != stcNil)
		istLb = IstLbFromStcp(StcpFromStc(stc, stsh.cstcStd));
	Assert(istLb < vistLbMac);
	RedisplayComboBox(tmc, istLb);
	if (istLb == istLbNil)
		SetTmcText(tmc, sz);
	if (hid != IDDParaLooks)
		BanterToTmc(vfDefineStyle ? tmcDSBanter : tmcASBanter, vstcStyle,
				&stsh, hsttbChpe, hsttbPape);
}


/* %%Function:SetIbstFontDSFromStc %%Owner:davidbo */
SetIbstFontDSFromStc(stc)
int stc;
{
	struct DOD *pdod;
	struct CHP chp;
	struct PAP pap;

	Assert(vdocStsh != docNil);
	pdod = PdodDoc(vdocStsh);
	MapStc(pdod, stc, &chp, &pap);
	ibstFontDS = IbstFontFromFtcDoc(chp.ftc, vdocStsh);
}


/* %%Function:CmdApplyStyDlg %%Owner:davidbo */
CMD CmdApplyStyDlg(pcmb)
CMB * pcmb;
{
	int stc;
	BOOL fDlgBox, fCreate;
	CMD cmd = cmdOK;
	struct STSH stsh;
	char stName[cchMaxStyle + 1];

	SetVdocStsh(selCur.doc);

	fCreate = fFalse;

	if (fDlgBox = (pcmb->kc == kcNil))
		{
		if (vhmpiststcp == hNil &&
				(vhmpiststcp = HAllocateCw(cwMpiststcp)) == hNil)
			{
			cmd = cmdNoMemory;
			goto LRet;
			}
		GenLbStyleMapping();
		}

	if (pcmb->fDefaults)
		{
		/*
		*  If there's a unique style code for the current selection,
		*  have it selected when dlg box is brought up.
		*/
		SetVstcSel();
		RecordStshForDocNoExcp(vdocStsh, &stsh);
		if ((stc = vstcSel) == stcNil)
			stc = stcNormal;
		GenStyleNameForStcp(stName, stsh.hsttbName, stsh.cstcStd, StcpFromStc(stc, stsh.cstcStd));

		if (!FSetCabSt(pcmb->hcab, stName, Iag(CABAPPLYSTYLE, hszASStyle)))
			{
			cmd = cmdNoMemory;
			goto LRet;
			}
		}

	if (pcmb->fDialog)
		{
		if (fDlgBox)
			{
			char dlt[sizeof(dltApplyStyle)];

			BltDlt(dltApplyStyle, dlt);

			switch (TmcOurDoDlg(dlt, pcmb))
				{
#ifdef DEBUG
			default:
				Assert(fFalse);
				cmd = cmdError;
				goto LRet;
#endif

			case tmcError:
				cmd = cmdError;
				goto LRet;

			case tmcCancel:
				cmd = cmdCancelled;
				break;

			case tmcASDefine:
				GetCabSt(pcmb->hcab, stName, cchMaxStyle+1, Iag(CABAPPLYSTYLE, hszASStyle));
				/* if user entered name, make sure it's in DefineStyles when we
					bring it up. */
				cmd = cmdCancelled;
				if (stName[0])
					CmdGotoDefine(stName);
				else
					ChainCmd(bcmStyles);
				break;

			case tmcOK:
				break;
				}
			}
		else  if ((*hmwdCur)->hwndRuler)
			{
			/* ruler is up, let them enter a style there */
			SetFocusIibb((*hmwdCur)->hwndRuler, idRulStyle);
			cmd = cmdCancelled; /* So this is not recorded! */
			goto LRet;
			}
		else
			{
			/* enter style name from prompt line */
			stName[0] = 0;
			pcmb->tmc = TmcInputPmtMst(mstWhichStyle, stName, cchMaxStyle-1, 
					bcmApplyStyleDlg, MmoFromCmg(cmgApplyProp));

			SzToStInPlace(stName);
			if (!FSetCabSt(pcmb->hcab, stName, Iag(CABAPPLYSTYLE, hszASStyle)))
				{
				cmd = cmdNoMemory;
				goto LRet;
				}

			switch (pcmb->tmc)
				{
			case tmcOK:
				break;

			case tmcCmdKc:
				/* The user hit the style key, bring 
					up the dialog. */
				ChainCmd(bcmApplyStyleDlg);
				cmd = cmdCancelled; /* So this is not recorded! */
				goto LRet;

			case tmcCancel:
				cmd = cmdCancelled;
				goto LRet;

#ifdef DEBUG
			default:
				Assert(fFalse);
				cmd = cmdError;
				goto LRet;
#endif
				}
			}
		}

	if ((!pcmb->fDialog || !fDlgBox) && pcmb->fAction && pcmb->tmc == tmcOK)
		{
		int id, fRedef;
		struct STTB **hsttbChpe;
		struct STTB **hsttbPape;

		GetCabSt(pcmb->hcab, stName, cchMaxStyle+1, Iag(CABAPPLYSTYLE,hszASStyle));
		RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);
		fRedef = fTrue;

		id = IdApplyStyle(&stc, stName, fFalse, fTrue, &fRedef, 
				((CABAPPLYSTYLE *) *pcmb->hcab)->fCreate, pcmb->wParam);
		switch (id)
			{
		case IDYES:
			fCreate = fTrue;
			break;
		case IDNO:
			fCreate = fFalse;
			if (!fRedef)
				cmd = cmdCancelled;
			break;
		case IDCANCEL:
			Assert(fRedef);
			cmd = cmdCancelled;
			break;
		default:
			Assert(id == -1);
#ifdef NOTUSED
			/* cmdNoMemory and cmdError are the same value. Old code: */
			cmd = vmerr.fMemFail ? cmdNoMemory : cmdError;
#endif /* NOTUSED */
			cmd = cmdError;
			break;
			}
		}

	if (vfRecording && cmd == cmdOK && pcmb->tmc == tmcOK)
		{
		CABAPPLYSTYLE * pcab;

		pcab = *pcmb->hcab;
		pcab->fCreate = fCreate;
		FRecordCab(pcmb->hcab, IDDApplyStyle, tmcOK, bcmApplyStyleDlg);
		}


LRet:
	if (fDlgBox)
		FreePh(&vhmpiststcp);

	vfShowAllStd = fFalse;
	vdocStsh = docNil;
	return cmd;
}


/* %%Function:FDlgApplyStyle %%Owner:davidbo */
BOOL FDlgApplyStyle(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	int stcp, stc, id;
	struct STSH stsh;
	struct STTB **hsttbChpe;
	struct STTB **hsttbPape;
	char st[cchMaxStyle+1];

	switch (dlm)
		{
	case dlmInit:
		SetVdocStsh(selCur.doc);
		SetVstcSel();
		RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);

		if (vstcSel == stcNil)
			{
			stc = stcNormal;
			stcp = stsh.cstcStd;
			}
		else
			{
			stc = vstcSel;
			stcp = StcpFromStc(vstcSel, stsh.cstcStd);
			}
		Assert(stc > stcStdMin || stcp <= ((*(stsh.hsttbName))->ibstMac));
		Assert(((*(stsh.hsttbName))->ibstMac) >= 1);
		SetIbstFontDSFromStc(stc);
		BanterToTmc(tmcASBanter, stc, &stsh, hsttbChpe, hsttbPape);
		GrayButtonOnBlank(tmcASStyle, tmcOK);
		CompleteComboTmc(tmcASStyle);
		AddKeyPfn(hkmpCur, KcCtrl(vkShowStd), ToggleShowStd);
		if (fElActive)
			EnableTmc(tmcASDefine, fFalse);

		break;


	case dlmChange:
		if (tmc == (tmcASStyle & ~ftmcGrouped))
			{
			GrayButtonOnBlank(tmcASStyle, tmcOK);
			if (ValGetTmc((tmcASStyle & ~ftmcGrouped) + 1) == istLbNil)
				break;
			RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);
			SetTmcBanterFromTmcCb(tmcASBanter, tmcASStyle, &stsh, hsttbChpe, hsttbPape);
			}
		break;

	case dlmTerm:
		if (!PcmbDlgCur()->fAction)
			break;
		GetTmcText(tmcASStyle, st, cchMaxStyle + 1);
		SzToStInPlace(st);
		if (tmc == tmcOK)
			{
			int fRedef = fTrue;
			id = IdApplyStyle(&stc, st, fTrue, fTrue, &fRedef, fFalse, fFalse);
			switch (id)
				{
			case IDYES:
				break;
			case IDNO:
				if (!fRedef)
					return fFalse;
				EndDlg(tmcCancel);     /* ok to call EndDlg here */

				break;
			case IDCANCEL:
				if (fRedef)
					return fFalse;
				EndDlg(tmcCancel);      /* ok to call EndDlg here */

				break;
			default:
				Assert(id == -1);
				/* if not fMemFail, stay in dlg */
				return vmerr.fMemFail;
				}
			}
		break;
		}

	return fTrue;
}


/* %%Function:WListStyles %%Owner:davidbo */
EXPORT WORD WListStyles(tmm, sz, isz, filler, tmc, wParam)
TMM tmm;
char * sz;
int isz;
WORD filler;
TMC tmc;
WORD wParam;
{
	int stcp;
	struct STSH stsh;

	if (vhmpiststcp == hNil)
		return 0;

	switch (tmm)
		{
	case tmmCount:
		return -1;

	case tmmText:
		Assert(isz <= vistLbMac);
		if (isz == vistLbMac)
			return fFalse;

		stcp = (**vhmpiststcp)[isz];
		RecordStshForDocNoExcp(vdocStsh, &stsh);
		GenStyleNameForStcp(sz, stsh.hsttbName, stsh.cstcStd, stcp);
		StToSzInPlace(sz);
		return 1;
		}

	return 0;
}


/* %%Function:CheckForInvalPadPgd %%Owner:davidbo */
CheckForInvalPadPgd(pstsh, hsttbChpe, hsttbPape)
struct STSH *pstsh;
struct STTB **hsttbChpe, **hsttbPape;
{
	int stcp;
	struct PAP pap;

	stcp = StcpFromStc(vstcStyle, pstsh->cstcStd);
	RetrievePropeForStcp(&pap, stcp, hsttbPape, fFalse);
	if (pap.phe.fStyleDirty)
		{
		InvalPadPgdForStc(vdocStsh, vstcStyle);
		InvalAllDependentPadPgd(pstsh, hsttbChpe, hsttbPape, stcp);
		}
}


/*
* Refill the combo box and set its listbox selection to istLb.
*/
/* %%Function:RedisplayComboBox %%Owner:davidbo */
RedisplayComboBox(tmcCb, istLb)
int tmcCb, istLb;
{
	RedisplayTmc(tmcCb);
	if (istLb == uNinchList)
		{
		SetTmcText(tmcCb, szEmpty);
		SetTmcVal((tmcCb & ~ftmcGrouped) + 1, uNinchList);
		}
	else
		{
		char sz[cchStrMax];

		GetListBoxEntry(tmcCb, istLb, sz, cchStrMax);
		SetTmcText(tmcCb, sz);
		CompleteComboTmc(tmcCb);
		}
}


/* C O P Y  S T Y L E S  F O N T S */
/* initial check for text already in Dest, where real stsh of Src is in
docSrcStsh */
/*  %%Function:CopyStylesFonts %%Owner:davidbo */
CopyStylesFonts(docSrcStsh, docDest, cpDest, dcp)
int docSrcStsh, docDest;
CP dcp, cpDest;
{
	int docDestMother;
	struct DOD *pdod;
	Win(struct CA caT);

	if (DocMother(docSrcStsh) == (docDestMother = DocMother(docDest)))
		return;
	Win( StartLongOp() );
	Win( CopyFonts( docSrcStsh, PcaSetDcp(&caT,docDest, cpDest, dcp )) );

	CachePara(docDest, cpDest);
	pdod = PdodDoc(docDestMother);
	if (caPara.cpLim <= cpDest + dcp || pdod->docFtn != docNil ||
			pdod->docHdr != docNil || pdod->docAtn != docNil)
		{
		CopyStyles1(docSrcStsh, docDest, cpDest, dcp);
		}
	Win( EndLongOp(fFalse) );
}


/**********************/
/* C o p y  F o n t s */
/* Called after *pcaDest has just been copied in from docSrc.
	Fix up docDest mpftcibstFont and *pcaDest ftcs as necessary.
*/
/*  %%Function:  CopyFonts  %%Owner:  bobz       */


CopyFonts(docSrc, pcaDest)
int docSrc;
struct CA *pcaDest;
{
	extern struct ESPRM     dnsprm[];
	extern int		vccpFetch;
	int docSrcFtc = DocMother(docSrc);
	int docDestFtc = DocMother(pcaDest->doc);
	int ftcSrc, ftcDest;
	int ftcSrcMac;
	struct CA ca;
	CHAR rgb[3];           /* grpprl for sprmftc */
#define cftcLastRemap	254
	CHAR mpftcSrcftcDest[cftcLastRemap];

	if (docSrcFtc == docDestFtc || DcpCa(pcaDest) == 0)
		return;

	/* font tables of documents are already identical */
	if ((ftcSrcMac = PdodDoc(docSrcFtc)->ftcMac) <= PdodDoc(docDestFtc)->ftcMac &&
			!FNeRgch( *PdodDoc(docSrcFtc)->hmpftcibstFont,
			*PdodDoc(docDestFtc)->hmpftcibstFont, 
			PdodDoc(docSrcFtc)->ftcMac ))
		return;

	Assert(ftcSrcMac >= 0 && ftcSrcMac <= cftcLastRemap);

	rgb[0] = sprmCFtc;
	SetBytes(mpftcSrcftcDest, 0xFF, cftcLastRemap);

	for ( ca = *pcaDest; ca.cpFirst < pcaDest->cpLim ; ca.cpFirst = ca.cpLim )
		{
		FetchCpAndPara( ca.doc, ca.cpFirst, fcmProps );
		ca.cpLim = vcpFetch + vccpFetch;
		if ((ftcDest = mpftcSrcftcDest[ftcSrc = vchpFetch.ftc]) == 0xFF)
			{
/* first time encountering this ftc from docSrcFtc -- add it to mapping */
			int ibst = (**PdodDoc(docSrcFtc)->hmpftcibstFont)
				[ftcSrc >= ftcSrcMac ? ftcDefault : ftcSrc];

			if ((ftcDest = FtcFromDocIbst(docDestFtc, ibst)) == valNil ||
					ftcDest >= cftcLastRemap)
				{
				ErrorEid(eidLowMemIncorrectFonts,"CopyFonts1");
				ftcDest = ftcDefault;
				}
#ifdef DEBUG
			else
                Assert((**(PdodDoc(docDestFtc)->hmpftcibstFont))[ftcDest]==ibst);
#endif /* DEBUG */


			mpftcSrcftcDest [ftcSrc] = (CHAR)ftcDest;

			Assert(ibst < (*vhsttbFont)->ibstMac);
			Assert(ibst >= 0);
			}

#ifdef DEBUG
		Assert(ftcDest < PdodDoc(docDestFtc)->ftcMac);
		Assert(ftcSrc >= 0);
		Assert(ftcDest >= 0);
		Assert(ftcSrc < cftcLastRemap);
		/* this may fail if text was originally copied in low memory */
		if (ftcSrc >= ftcSrcMac && vdbs.fCkText)
			ReportSz("doc contains ftcs out of range");
#endif /* DEBUG */

		if (ftcSrc != ftcDest)
			{
/* ftc changed -- remap it */
			Assert ( dnsprm[sprmCFtc].cch == 3 && sizeof(rgb) >= 3);
			*((int *)&rgb[1]) = ftcDest;
			ApplyGrpprlCa(rgb, 3, &ca);
			if (vmerr.fMemFail)
				{
				ErrorEid(eidLowMemIncorrectFonts,"CopyFonts2");
				vmerr.mat = matNil;
				}
			}
		}
}



