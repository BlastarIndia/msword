/* D E B U G 2 . C */
/*  Miscellaneous debuging code.  Mostly specialized runtime debug code. */


#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "version.h"
#include "heap.h"
#include "menu2.h"
#include "props.h"
#include "doc.h"
#include "format.h"
#include "disp.h"
#include "field.h"
#include "pic.h"
#include "sel.h"
#include "prm.h"
#include "ch.h"
#include "screen.h"
#include "opuscmd.h"
#include "message.h"
#include "cmdtbl.h"
#include "print.h"
#include "autosave.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"
#include "prompt.h"
#include "file.h"
#include "debug.h"


#ifdef PROTOTYPE
#include "debug2.cpt"
#endif /* PROTOTYPE */

/* E X T E R N A L S */

extern struct SEL   selCur;
extern struct PREF  vpref;
extern CP           vcpFetch;
extern CHAR HUGE    *vhpchFetch;
extern int          vdocFetch;
extern int          vccpFetch;
extern struct CHP   vchpFetch;
extern BOOL         vfEndFetch;
extern struct PRI   vpri;
extern struct FLI   vfli;
extern struct FTI   vfti;
extern struct FMTSS vfmtss;
extern struct CA    caAdjust; /* used while traversing fields during calc */
extern struct CA    caPara;
extern int          vdocScratch;
extern struct SCI   vsci;
extern BOOL         vfInsertMode;
extern CP           cpInsert;
extern struct RC    rcLayoutPage;
extern HMENU        vhMenu;
extern int          docMac;
extern int          vifldInsert;
extern struct DOD **mpdochdod[];
extern int          vdocTemp;
extern struct SAB   vsab;
extern HWND         vhwndApp;
extern int          cfLink;
extern struct WWD   **hwwdCur;
extern int	    wwCur;
extern int          vdocTemp;
extern int          vdocScratch;
extern int          docGlobalDot;
extern int          docGlsy;
extern int          docDde;
extern HWND         vhwndRibbon;
extern HWND         vhwndStatLine;
extern struct MWD   **hmwdCur;
extern int          *vpsccBelow;
extern BOOL	    f8087;
extern BOOL	    vfAwfulNoise;
extern BOOL         vfLargeFrameEMS;
extern BOOL         vfSmallFrameEMS;


#ifdef DEBUG


/* %%Function:TestFieldFetchFuncs %%Owner:BRADV */
TestFieldFetchFuncs ()
{
	static iMode = 0;
	switch (iMode++ % 3)
		{
	case 0:
		CkFFetchArgText (fFalse /* fTruncate */);
		break;
	case 1:
		CkFFetchArgText (fTrue);
		break;
	case 2:
		CkFFetchArgExtents ();
		break;
	default:
		Assert (fFalse);
		}
}


/* C K  F E T C H  F U N C */

/* %%Function:CkFetchFunc %%Owner:BRADV */
CkFetchFunc (fArgument)
BOOL fArgument;

{
	int w, ifld = IfldSelCur ();
	struct FFB ffb;
	CHAR rgch [42];
	struct CR rgcr [3];


	InitFvbBufs (&ffb.fvb, rgch, 39, rgcr, 3);
	SetFfbIfld (&ffb, selCur.doc, ifld);

	do
		{
		FetchFromField (&ffb, fArgument);

		for (w = 0; w < ffb.cch; w++)
			if (rgch [w] == chGroupInternal)
				rgch [w] = '`';
			else  if (rgch [w] < ' '|| rgch [w] > '~')
				if (FWhite (rgch [w]))
					rgch [w] = '_';
				else
					rgch [w] = '.';
		rgch [ffb.cch] = '\0';
		CommSz (rgch);
		rgch [0] = '$';
		for (w = 1; w < 40 - ffb.cch; rgch [w++] = ' ');
		rgch [w] = '\0';
		CommSz (rgch);
		ShowFfbFlags (&ffb);
		ShowRgcr (rgcr, ffb.ccr);
		CommSz ("\r\n");
		} 
	while ((fArgument && !ffb.fNoArg) || (!fArgument && ffb.fOverflow));

	ShowFfbSwitches (&ffb, fFalse);
	ShowFfbSwitches (&ffb, fTrue);
	CommSz ("\r\n");

}



/* S H O W  R G C R */

/* %%Function:ShowRgcr %%Owner:BRADV */
ShowRgcr (pcr, ccr)
struct CR *pcr;
int ccr;

{
	int cch;
	CHAR *pch, sz [37];
	pch = sz;

	Assert (ccr <= 3 && ccr >= 0);

	while (ccr-- > 0)
		{
		*pch++ = ' ';
		*pch++ = ' ';
		cch = CchIntToAsciiHex ( (uns)pcr->cp, &pch, 4);
		Assert (cch < 6);
		*pch++ = ',';
		cch += CchIntToAsciiHex ( pcr->ccp, &pch, 2 );
		Assert (cch < 11);
		pcr++;
		}

	*pch++ = '\0';

	CommSz (sz);
}



/* S H O W  F L A G S */

/* %%Function:ShowFfbFlags %%Owner:BRADV */
ShowFfbFlags (pffb)
struct FFB *pffb;

{
	CHAR rgch [7];

	rgch [0] = pffb->fOverflow ? 'O' : ' ';
	rgch [1] = pffb->fGrouped ? 'G' : ' ';
	rgch [2] = pffb->fNested ? 'N' : ' ';
	rgch [3] = pffb->fNoArg ? 'E' : ' ';
	rgch [4] = pffb->cchFieldBegin > 0 ? 'F' : ' ';
	rgch [5] = pffb->fGroupPending ? 'P' : ' ';
	rgch [6] = '\0';

	CommSz (rgch);
}



/* S H O W  F F B  S W I T C H E S */

/* %%Function:ShowFfbSwitches %%Owner:BRADV */
ShowFfbSwitches (pffb, fSys)
struct FFB *pffb;
BOOL fSys;

{
	CHAR chSw;
	CHAR *pch;
	CHAR rgch [60];

	InitFvbBufs (&pffb->fvb, rgch+3, 50, NULL, 0);

	while ((chSw = ChFetchSwitch (pffb, fSys)) != chNil)
		{
		FFetchArgText (pffb, fFalse);
		pch = rgch;
		*pch++ = chSw;
		*pch++ = ':';
		*pch = ' ';
		pch = rgch + 3 + pffb->cch;
		*pch++ = pffb->fOverflow ? '+' : '$';
		*pch++ = '\r';
		*pch++ = '\n';
		*pch = 0;
		CommSz (rgch);
		}
}


/* S H O W  F I E L D  I N F O */

/* %%Function:ShowFieldInfo %%Owner:BRADV */
ShowFieldInfo ()

{

	int ifld = IfldFromDocCp (selCur.doc, selCur.cpFirst, fFalse);
	int w, iNum, cLevel;
	int cch;
	CHAR *pch, rgch [80];
	struct FLCD flcd;

	if (ifld == ifldNil)
		{
		Beep ();
		return;
		}

	pch = rgch;
	GetIfldFlcd (selCur.doc, ifld, &flcd);

	for (iNum = 0; iNum < 6; iNum++)
		{
		switch (iNum)
			{
		case 0:
			w = (int) selCur.cpFirst;
			cLevel = 4;
			break;
		case 1:
			w = ifld;
			cLevel = 2;
			break;
		case 2:
			w = flcd.flt;
			cLevel = 2;
			break;
		case 3:
			w = (int) flcd.cpFirst;
			cLevel = 4;
			break;
		case 4:
			w = (int) flcd.dcpInst;
			cLevel = 4;
			break;
		case 5:
			w = (int) flcd.dcpResult;
			cLevel = 4;
			break;
			}
		cch = CchIntToAsciiHex (w, &pch, cLevel);
		*pch++ = ' ';
		*pch++ = ' ';
		}

	if (flcd.fDirty) *pch++ = 'D';
	if (flcd.fDiffer) *pch++ = 'f';
	if (flcd.fResultDirty) *pch++ = 'd';
	if (flcd.fResultEdited) *pch++ = 'e';
	if (flcd.fLocked) *pch++ = 'l';
	if (flcd.fPrivateResult) *pch++ = 'p';

	*pch++ = '\r';
	*pch++ = '\n';
	*pch = '\0';

	CommSz (rgch);

}


#ifdef ENABLE

/*  these have been tested ok.  i'm leaving them here in case anyone
	needs them.
*/

CP dcpInstTest = cp0;
CP dcpResultTest = cp0;


/*  T E S T  R M  F I E L D */


/* %%Function:RmField %%Owner:peterj */
TestRmField()

{
	CchRmFieldAtDocCp (selCur.doc, selCur.cpFirst,
			&dcpInstTest, &dcpResultTest);

	CommSzNum ("dcpInst: ", (int) dcpInstTest);
	CommSzNum ("dcpResult: ", (int) dcpResultTest);

	return cmdOK;
}


/* T E S T  P U T  C P S */

/* %%Function:TestPutCps %%Owner:peterj */
TestPutCps()

{
	FPutCpsInField (selCur.doc, selCur.cpFirst,
			dcpInstTest, dcpResultTest, NULL);

	return cmdOK;
}



/* T R A S H  F I E L D  I N F O */
/* %%Function:TrashFieldInfo %%Owner:peterj */
TrashFieldInfo()
{
	FInsertFltDcps (fltUnknownKwd, selCur.doc, selCur.cpFirst,
			(CP) 6, (CP) 5, NULL);
}




/* I N S E R T  B O G U S N E S S */
/* %%Function:InsertBogusness %%Owner:peterj */
InsertBogusness()
{
	CHAR rgch [] = "{foo";
	struct CHP chp;

	SetFieldPchp (selCur.doc, selCur.cpFirst, &chp);
	FInsertRgch (selCur.doc, selCur.cpFirst, rgch, 4, &chp, NULL);
}



#endif /* ENABLE */




/* S H O W  C H P */
/* %%Function:ShowChp %%Owner:BRADV */
ShowChp(doc, cp, pchp)
int doc;
CP cp;
unsigned *pchp;

{
	int i;
	CHAR *pch, rgch [60];

	pch = rgch;
	CchIntToPpch (doc, &pch);
	pch += CchCopySz (", ", pch);
	CchIntToPpch ((int)cp, &pch);
	pch += CchCopySz (":  ", pch);

	for (i = 0; i < cwCHP; i++)
		{
		CchIntToAsciiHex (pchp [i], &pch, 4);
		pch += CchCopySz (" ", pch);
		}
	CchCopySz ("\r\n", pch);

	CommSz (rgch);
}



/* %%Function:ShowChpSelCur %%Owner:BRADV */
ShowChpSelCur()

{
	extern struct CHP vchpFetch;

	CachePara (selCur.doc, selCur.cpFirst);

	FetchCp (selCur.doc, selCur.cpFirst, fcmProps);
	ShowChp (selCur.doc, selCur.cpFirst, &vchpFetch);
}





/* C K  F  F E T C H  A R G U M E N T  T E X T */

/* %%Function:CkFFetchArgText %%Owner:BRADV */
CkFFetchArgText(fTruncate)
BOOL fTruncate;

{
	int ifld = IfldSelCur ();
	CHAR rgch [20];
	struct FFB ffb;

	InitFvb (&ffb.fvb);
	InitFvbBufs (&ffb.fvb, rgch, 20, NULL, 0);
	SetFfbIfld (&ffb, selCur.doc, ifld);

	while (FFetchArgText (&ffb, fTruncate))
		{
		CommSz (rgch);
		CommSz ("\r\n");
		}
	ShowFfbSwitches (&ffb, fFalse);
	ShowFfbSwitches (&ffb, fTrue);
}





/* C K  F  F E T C H  A R G U M E N T  E X T E N T S */

/* %%Function:CkFFetchArgExtents %%Owner:BRADV */
CkFFetchArgExtents()

{
	int ifld = IfldSelCur ();
	CP cpFirst, cpLim;
	CHAR *pch, rgch [30];
	struct FFB ffb;

	InitFvb (&ffb.fvb);
	SetFfbIfld (&ffb, selCur.doc, ifld);

	while (FFetchArgExtents (&ffb, &cpFirst, &cpLim))
		{
		pch = rgch;
		CchIntToAsciiHex ( (uns)cpFirst, &pch, 4);
		*pch++ = ":";
		*pch++ = ' ';
		CchIntToAsciiHex ( (uns)cpLim, &pch, 4);
		*pch++ = ',';
		*pch++ = ' ';
		CchIntToAsciiHex ( (uns)(cpLim - cpFirst), &pch, 2);
		*pch++ = '\r';
		*pch++ = '\n';
		*pch++ = '\0';
		CommSz (rgch);
		}
	ShowFfbSwitches (&ffb, fFalse);
	ShowFfbSwitches (&ffb, fTrue);

}


/* S H O W  F E T C H  V I S I B L E */

/* %%Function:ShowFetchVisible %%Owner:BRADV */
ShowFetchVisible()

{
#ifdef REVIEW   /*  IFTIME  (bz) old, needs to be updated */
	int ccp, ich;
	CP cpMac = selCur.cpLim;
	CHAR HUGE *hpch, HUGE *hpchLim;
	CHAR rgch [50];
	static CP cpFirst = cpNil;
	static int fvc = 0;

	if (cpFirst != selCur.cpFirst)
		{
		cpFirst = selCur.cpFirst;
		fvc = 0;
		}

	FetchCpPccpVisible (selCur.doc, selCur.cpFirst, &ccp, fvc, selCur.ww, fFalse);

	while (!vfEndFetch && vcpFetch < cpMac)
		{
		hpch = vhpchFetch;
		hpchLim = hpch + ccp;
		do
			{
			bltbx(LpFromHp(hpch), (char far *)rgch,
					ich = min(hpchLim - hpch, 46));
			rgch [ich] = ich < hpchLim - hpch ? '+' : '$';
			pch += ich;
			rgch [++ich] = '\r';
			rgch [++ich] = '\n';
			rgch [++ich] = '\0';
			for (ich -= 4; ich >= 0; ich--)
				if (rgch [ich] < ' ' || rgch [ich] > '~')
					if (FWhite (rgch [ich]))
						rgch [ich] = '_';
					else
						rgch [ich] = '.';

			CommSz (rgch);
			}
		while (hpch < hpchLim);

		FetchCpPccpVisible (docNil, cpNil, &ccp, fvc, selCur.ww, fFalse);
		}

	fvc = (fvc + 1) % 3;
#endif /* REVIEW   IFTIME  (bz)*/
}


/* S H O W  F E T C H   */

/* %%Function:ShowFetch %%Owner:BRADV */
ShowFetch(doc)
int doc;
{
	/* dumps doc out to comm port */

	int ich;
	CP cpMac = CpMacDoc (doc);
	CHAR HUGE *hpch, HUGE *hpchLim;
	CHAR rgch [50];

	FetchCp(doc, cp0, fcmChars);

	while (!vfEndFetch && vcpFetch < cpMac)
		{
		hpch = vhpchFetch;
		hpchLim = hpch + vccpFetch;
		do
			{
			bltbx(LpFromHp(hpch), (char far *)rgch,
					ich = min(hpchLim - hpch, 46));
			rgch [ich] = ich < hpchLim - hpch ? '+' : '$';
			hpch += ich;
			rgch [++ich] = '\r';
			rgch [++ich] = '\n';
			rgch [++ich] = '\0';
			for (ich -= 4; ich >= 0; ich--)
				if (rgch [ich] < ' ' || rgch [ich] > '~')
					if (FWhite (rgch [ich]))
						{
						if (rgch[ich] == chEop)
							rgch [ich] = 'A';
						else  if (rgch[ich] == chReturn)
							rgch [ich] = 'D';
						else  if (rgch[ich] == chTab)
							rgch [ich] = 'T';
						else
							rgch [ich] = '_';
						}
					else if (rgch [ich] < ' ')
						rgch [ich] += 'a';  /* turn ctl chars into letters */
					else
						rgch [ich] = '.';
			CommSz (rgch);
			}
		while (hpch < hpchLim);

		FetchCp(docNil, cpNil, fcmChars);
		}
}



/* %%Function:InsertFieldWithArgs %%Owner:BRADV */
InsertFieldWithArgs()
{
	int tmc, flt = 0;
	BOOL fNeg = fFalse;
	BOOL fCalc = fFalse, fShowDefault = fFalse, fShowResult = fFalse;
	CHAR *pch, sz [cchMaxSz];

	sz[0] = '\0';
	tmc = TmcInputPmtMst (mstNil, sz, cchMaxSz, icmdNil, mmoNormal);
	if (tmc == tmcCancel) return;
	pch = sz;
	if (*pch == '-')
		{
		fNeg = fTrue;
		pch++;
		}
	while (*pch && FDigit (*pch))
		flt = flt*10 + (*pch++) - '0';
	if (fNeg) flt *= -1;
	sz [0] = '\0';
	tmc = TmcInputPmtMst (mstNil, sz, 4, icmdNil, mmoNormal);
	if (tmc == tmcOK && CchSz (sz) > 3)
		{
		fCalc = sz [0] == '1';
		fShowDefault = sz [1] == '1';
		fShowResult = sz [2] == '1';
		}
	sz [0] = '\0';
	tmc = TmcInputPmtMst (mstNil, sz, cchMaxSz, icmdNil, mmoNormal);
	pch = tmc == tmcCancel ? NULL : sz;

	CmdInsFltSzAtSelCur (flt, pch, fCalc, fShowDefault, fShowResult);
}




/* %%Function:TestUpdateRuler %%Owner:BRADV */
TestUpdateRuler()
{
	CP cp;
	CP cpMac = CpMacDoc (selCur.doc);
	int cUpdates = 4;

#ifdef PROFILE
	vpfi == pfiDebugUpdateRuler ? StartProf(30) : 0;
#endif /* PROFILE */

	while (cUpdates--)
		{
		cp = cp0;
		do
			{
			Select (&selCur,cp, cp);
			UpdateRuler (hmwdCur, fFalse);
			CachePara (selCur.doc, cp);
			cp = caPara.cpLim;
			} 
		while (cp < cpMac);
		}

#ifdef PROFILE
	vpfi == pfiDebugUpdateRuler ? StopProf() : 0;
#endif /* PROFILE */

}



/* %%Function:TestFieldsDisplay %%Owner:BRADV */
TestFieldsDisplay()
{
	int cTests = 10;

#ifdef PROFILE
	vpfi == pfiDebugFieldsDisplay ? StartProf(30) : 0;
#endif /* PROFILE */

	while (cTests--)
		{
		TrashAllWws ();
		UpdateAllWws (fFalse);
		}

#ifdef PROFILE
	vpfi == pfiDebugFieldsDisplay ? StopProf() : 0;
#endif /* PROFILE */

}


/* %%Function:CfLinkToClipTest %%Owner:BRADV */
CfLinkToClipTest()
{
	CHAR szApp[cchMaxSz];
	CHAR szTopic[cchMaxSz];
	CHAR szItem[cchMaxSz];
	CHAR FAR * lpch;
	HANDLE hData;
	int cch, cchT;

	szApp[0]=0;
	if (TmcInputPmtMst (mstNil, szApp, cchMaxSz, bcmNil, mmoNormal) != tmcOK)
		return;
	szTopic[0]=0;
	if (TmcInputPmtMst (mstNil, szTopic, cchMaxSz, bcmNil, mmoNormal) != tmcOK)
		return;
	szItem[0]=0;
	if (TmcInputPmtMst (mstNil, szItem, cchMaxSz, bcmNil, mmoNormal) != tmcOK)
		return;

	if ((hData = OurGlobalAlloc (GMEM_MOVEABLE, (DWORD)
			(CchSz(szApp)+CchSz(szTopic)+CchSz(szItem)+1))) == NULL ||
			(lpch = GlobalLock (hData)) == NULL)
		{
		if (hData != NULL)
			GlobalFree (hData);
		return fFalse;
		}
	cch = 0;
	bltbx ((LPSTR)szApp, (LPSTR)lpch+cch, cchT = CchSz (szApp));
	cch += cchT;
	bltbx ((LPSTR)szTopic, (LPSTR)lpch+cch, cchT = CchSz (szTopic));
	cch += cchT;
	bltbx ((LPSTR)szItem, (LPSTR)lpch+cch, cchT = CchSz (szItem));
	cch += cchT;
	lpch [cch] = '\0';

	GlobalUnlock (hData);

	if (!OpenClipboard(vhwndApp))
		{
		GlobalFree (hData);
		return;
		}
	EmptyClipboard();
	SetClipboardData(cfLink, hData);
	CloseClipboard();

	vsab.fOwnClipboard = fFalse;
}



/* %%Function:DumpDocs %%Owner:BRADV */
DumpDocs()

{
	int doc, ww, cww;
	struct DOD *pdod, **hdod;
	CHAR rgch [85], stFile [ichMaxFile];
	CHAR *pch;

	extern int docMac, fnMac;
	extern struct DOD **mpdochdod[];
	extern struct FCB **mpfnhfcb[];

	for (doc = 0; doc < docMac; doc++)
		if ((hdod = mpdochdod [doc]) != hNil)
			{
			pch = rgch;
			pdod = *hdod;
			CchPchToPpch (SzShared ("doc "), &pch);
			CchIntToAsciiHex (doc, &pch, 2);
			switch (pdod->dk)
				{
			case dkDoc:
				CchPchToPpch (SzShared (" doc"), &pch);
				break;
			case dkDot:
				CchPchToPpch (SzShared (" dot"), &pch);
				break;
			case dkGlsy:
				CchPchToPpch (SzShared (" glsy"), &pch);
				break;
			case dkAtn:
				CchPchToPpch (SzShared (" atn"), &pch);
				break;
			case dkMcr:
				CchPchToPpch (SzShared (" mcr"), &pch);
				break;
			case dkHdr:
				CchPchToPpch (SzShared (" hdr"), &pch);
				break;
			case dkFtn:
				CchPchToPpch (SzShared (" ftn"), &pch);
				break;
			case dkSDoc:
				CchPchToPpch (SzShared (" sdoc"), &pch);
				break;
			default:
				Assert(fFalse);
				break;
				}
			if (doc < docMinNormal)
				switch (doc)
					{
				case docUndo:
					CchPchToPpch (SzShared (" Undo"), &pch);
					break;
				case docNew:
					CchPchToPpch (SzShared (" New"), &pch);
					break;
				case docScrap:
					CchPchToPpch (SzShared (" Scrap"), &pch);
					break;
					}
			else  if (doc == vdocTemp)
				CchPchToPpch (SzShared (" Temp"), &pch);
			else  if (doc == vdocScratch)
				CchPchToPpch (SzShared (" Scratch"), &pch);
			else  if (doc == docDde)
				CchPchToPpch (SzShared (" Dde"), &pch);
			else  if (doc == docGlobalDot || doc == docGlsy)
				CchPchToPpch (SzShared (" Global"), &pch);
			else  if (doc == selCur.doc)
				CchPchToPpch (SzShared (" SelCur"), &pch);

			CchPchIntToPpch (SzShared (" fn="), pdod->fn, &pch);
			CchPchIntToPpch (SzShared (" doc="), pdod->doc, &pch);

			/* count ww's displayed in */
			cww = 0;
			for ( ww = wwNil ; (ww = WwDisp(doc,ww,fFalse)) != wwNil; )
				cww++;
			CchPchIntToPpch (SzShared (" cww="), cww, &pch);
			CchPchIntToPpch (SzShared (" cref="), pdod->crefLock, &pch);
			CchPchToPpch (SzShared (" cpMac="), &pch);
			CchLongToPpch (CpMacDoc(doc), &pch);

			CchPchToPpch (SzShared (" fl:"), &pch);
			if (pdod->fDirty)
				*pch++ = 'd';
			if (pdod->fStshDirty)
				*pch++ = 's';
			if (pdod->fLRDirty)
				*pch++ = 'l';
			if (pdod->fOutlineDirty)
				*pch++ = 'o';
			if (pdod->fMotherStsh)
				*pch++ = 'm';
			if (pdod->fFormatted)
				*pch++ = 'f';
			if (pdod->fRepag)
				*pch++ = 'r';
			if (pdod->fLockForEdit)
				*pch++ = 'e';

			SendBuffer (&pch, rgch);

			if (pdod->fShort)
				{
#ifdef REVIEW  /*  IFTIME  (bz) */
				DumpDocHandles (doc, fFalse);
#endif /* REVIEW  IFTIME  (bz) */
				continue;
				}

			if (doc >= docMinNormal && !pdod->fGlsy)
				{
				GetDocSt (doc, stFile, gdsoFullPath);
				CommSzSt (SzShared ("       name: "), stFile);
				}

			CchPchToPpch (SzShared ("      "), &pch);
			if (pdod->docFtn)
				CchPchIntToPpch (SzShared (" docFtn="), pdod->docFtn, &pch);
			if (pdod->docHdr)
				CchPchIntToPpch (SzShared (" docHdr="), pdod->docHdr, &pch);
			if (pdod->docAtn)
				CchPchIntToPpch (SzShared (" docAtn="), pdod->docAtn, &pch);
			if (pdod->fDoc && pdod->docDot)
				CchPchIntToPpch (SzShared (" docDot="), pdod->docDot, &pch);
			if (pdod->fDot && pdod->docGlsy)
				CchPchIntToPpch (SzShared (" docGlsy="), pdod->docGlsy, &pch);
			if (pdod->fDot && pdod->docMcr)
				CchPchIntToPpch (SzShared (" docMcr="), pdod->docMcr, &pch);
			SendBuffer (&pch, rgch);

#ifdef REVIEW /*  IFTIME  (bz) */
			DumpDocHandles (doc, fFalse);
#endif /* REVIEW  IFTIME  (bz) */
			}


}


/* %%Function:CheckBuffer %%Owner:BRADV */
CheckBuffer(ppch, rgch)
CHAR **ppch;
CHAR *rgch;

{
	if (*ppch - rgch > 60)
		{
		SendBuffer (ppch, rgch);
		CchPchToPpch (SzShared("      "), ppch);
		}
}


/* %%Function:SendBuffer %%Owner:BRADV */
SendBuffer(ppch, rgch)
CHAR **ppch;
CHAR *rgch;
{
	if (*ppch - rgch > 6 || (*ppch - rgch != 0 && rgch [0] != ' '))
		{
		*(*ppch)++ = 0;
		Assert (*ppch - rgch <= 85);
		CommSzCrLf (rgch);
		}
	*ppch = rgch;
}


/* %%Function:CommSzCrLf %%Owner:BRADV */
CommSzCrLf (sz)
CHAR *sz;
{
	CommSz (sz);
	CommSz (SzShared ("\n\r"));
}




/* %%Function:CchPchIntToPpch %%Owner:BRADV */
CchPchIntToPpch(pch, w, ppch)
CHAR *pch;
int w;
CHAR **ppch;

{
	int cch = CchPchToPpch (pch, ppch);
	return cch + CchIntToPpch (w, ppch);
}



/* %%Function:ScribbleSz %%Owner:BRADV */
ScribbleSz(sz)
CHAR *sz;

{
	struct RC rc;
	HDC hdc;
	TEXTMETRIC tm;
	int dyp, yp, dxp, xp, cch = CchSz (sz)-1;


	if (vhwndApp == NULL)
		return;

	SetWords(&rc, 0, cwRC);
	GetWindowRect( vhwndApp, (LPRECT) &rc );
	if (FEmptyRc( &rc ))
		return;
	hdc = GetWindowDC( vhwndApp );
	if  (hdc == NULL )
		return;

	GetTextMetrics( hdc, (LPTEXTMETRIC)&tm );
	dyp = tm.tmHeight + tm.tmInternalLeading;
	yp = GetSystemMetrics(SM_CYDLGFRAME);
	dxp = cch * tm.tmAveCharWidth;
	xp = rc.xpLeft + GetSystemMetrics (SM_CXDLGFRAME) + 1 + (4*vsci.dxpScrlBar);

	PatBlt( hdc, xp, yp, dxp, dyp, WHITENESS );
	TextOut( hdc, xp, yp, (LPSTR) sz, cch);
	ReleaseDC( (HWND)vhwndApp, hdc );
}



/* %%Function:GetStateInfo %%Owner:BRADV */
EXPORT GetStateInfo(rgch1, rgch2)
CHAR *rgch1, *rgch2; /* long form in 1, short in 2 */

{
	int cDBS;
	CHAR *pf;
	uns w, iw;
	int cchLine = 0;
	CHAR *pch1 = rgch1, *pch2 = rgch2;
	int cchT;
	GRPFVISI grpfvisi;
	CHAR *pchT, szT[cchMaxSz+60];
	extern int vsasCur;

	*pch2++ = ' ';
	*pch2++ = ' ';
	*pch2++ = ' ';

	grpfvisi.w = 0;
	if (wwCur !=wwNil)
		grpfvisi = PwwdWw(wwCur)->grpfvisi;
	if (grpfvisi.fvisiShowAll)
		{
		*pch2++ = 'A';
		cchLine += CchPchToPpch (SzShared ("showAll "), &pch1);
		}
	if (vpref.fBkgrndPag)
		{
		*pch2++ = 'B';
		cchLine += CchPchToPpch (SzShared ("Bkgrndpgn "), &pch1);
		}
	/*  C: printer Connected */
	if (vpref.fDraftView)
		{
		*pch2++ = 'D';
		cchLine += CchPchToPpch (SzShared ("Draft "), &pch1);
		}
	if (smtMemory==smtLIM)
		{
		*pch2++ = 'E';
		cchLine += CchPchToPpch (SzShared ("Emm "), &pch1);
		}
	if (docGlsy != docNil)
		{
		*pch2++ = 'G';
		cchLine += CchPchToPpch (SzShared ("Glsys "), &pch1);
		}
	if (grpfvisi.fSeeHidden)
		{
		*pch2++ = 'H';
		cchLine += CchPchToPpch (SzShared ("seeHidden "), &pch1);
		}
	if (vfLargeFrameEMS || vfSmallFrameEMS)
		{
		*pch2++ = 'K';
		if (vfLargeFrameEMS)
			{
			*pch2++ = 'l';
			cchLine += CchPchToPpch (SzShared ("KernelLargeems "), &pch1);
			}
		else
			{
			*pch2++ = 's';
			cchLine += CchPchToPpch (SzShared ("KernelSmallems "), &pch1);
			}
		}
	if (vhwndRibbon)
		{
		*pch2++ = 'L';
		cchLine += CchPchToPpch (SzShared ("Lookhelper "), &pch1);
		}
	if (vpref.dxaStyWnd != 0)
		{
		*pch2++ = 'N';
		cchLine += CchPchToPpch (SzShared ("styleName "), &pch1);
		}
	if (cchLine > 50)
		{
		*pch1++ = '\r';
		*pch1++ = '\n';
		cchLine = 0;
		}
	if (docGlobalDot != docNil && PdodDoc(docGlobalDot)->fn != fnNil)
		{
		*pch2++ = 'O';
		cchLine += CchPchToPpch (SzShared ("Opusgdt "), &pch1);
		}
	if (grpfvisi.flm == flmDisplayAsPrint)
		{
		*pch2++ = 'P';
		cchLine += CchPchToPpch (SzShared ("dispPrint "), &pch1);
		}
	if (hmwdCur != hNil && (*hmwdCur)->hwndRuler != NULL)
		{
		*pch2++ = 'R';
		cchLine += CchPchToPpch (SzShared ("Ruler "), &pch1);
		}
	if (cchLine > 50)
		{
		*pch1++ = '\r';
		*pch1++ = '\n';
		cchLine = 0;
		}
	if (vhwndStatLine)
		{
		*pch2++ = 'S';
		cchLine += CchPchToPpch (SzShared ("Statline "), &pch1);
		}
	if (selCur.doc != docNil && PdodDoc(selCur.doc)->docDot != docNil)
		{
		*pch2++ = 'T';
	/* pch1 below with other selcur info */
		}
	if (vpref.iAS != iASNever)
		{
		*pch2++ = 'V';
		cchLine += CchPchToPpch (SzShared ("autosaVe "), &pch1);
		}
	if (hwwdCur && (*hwwdCur)->fPageView)
		{
		*pch2++ = 'W';
		cchLine += CchPchToPpch (SzShared ("pagevieW "), &pch1);
		}
	if (vpref.fZoomMwd)
		{
		*pch2++ = 'Z';
		cchLine += CchPchToPpch (SzShared ("Zoommw "), &pch1);
		}
	if (f8087)
		{
		*pch2++ = 'M';
		cchLine += CchPchToPpch (SzShared ("80X87 "), &pch1);
		}
	if (vsasCur)
		{
		*pch2++ = '0'+vsasCur;
		}

	/* Last of "flags" */

	/* find out what printer is connected. */
	if (vpri.hszPrinter != hNil)
		{
		pchT = szT;
		cchT = CchPchToPpch (*vpri.hszPrinter, &pchT);
		if (cchT > 35)
			pchT = &szT [35];
		CchPchToPpch (SzShared(" on "), &pchT);
		CchPchToPpch (*vpri.hszPrPort, &pchT);
		*pchT = 0;
		szT [59] = 0;

		*pch2++ = 'C';

		*pch1++ = '\r';
		*pch1++ = '\n';
		CchPchToPpch (SzShared("Connected to: "), &pch1);
		CchPchToPpch (szT, &pch1);
		}

	*pch1++ = '\r';
	*pch1++ = '\n';

	if (selCur.doc != docNil)
		{
		int docDot;
		CHAR stT [ichMaxFile];
		CchPchToPpch (SzShared("selCur: "), &pch1);
		if (PdodDoc(selCur.doc)->fn != fnNil)
			CchPchToPpch (SzShared("(fn) "),&pch1);
		GetDocSt (selCur.doc, stT, gdsoShortName);
		StToSz(stT, szT);
		CchPchToPpch (szT, &pch1);
		if (PdodDoc(selCur.doc)->fDot)
			CchPchToPpch (SzShared(" (is dot)"), &pch1);
		else  if (PdodDoc(selCur.doc)->fDoc
				&& (docDot = PdodDoc (selCur.doc)->docDot) != docNil)
			{
			CchPchToPpch (SzShared(" dot: "), &pch1);
			GetDocSt (docDot, stT, gdsoShortName);
			StToSz(stT, szT);
			CchPchToPpch (szT, &pch1);
			}
		*pch1++ = '\r';
		*pch1++ = '\n';
		}

	*pch2++ =  ' ';
	*pch2++ = *pch1++ = '(';

	cDBS = cTestsUsed;
	pf = pchTestMin;
	while (cDBS > 0)
		{
		for (w = 0, iw = 0; iw < 12; iw++)
			w = (w << 1) | (cDBS-- > 0 && *pf++ ? 1 : 0);
		CchIntToAsciiHex (w, &pch1, 3);
		CchIntToAsciiHex (w, &pch2, 3);
		if (cDBS > 0)
			{
			*pch1++ = '-';
			*pch2++ = '-';
			}
		}
	*pch1++ = '/';
	*pch2++ = '/';

	cDBS = cPrefsUsed;
	pf = pchPrefMin;
	while (cDBS > 0)
		{
		for (w = 0, iw = 0; iw < 12; iw++)
			w = (w << 1) | (cDBS-- > 0 && *pf++ ? 1 : 0);
		CchIntToAsciiHex (w, &pch1, 3);
		CchIntToAsciiHex (w, &pch2, 3);
		if (cDBS > 0)
			{
			*pch1++ = '-';
			*pch2++ = '-';
			}
		}

	pf = pchUseCMin;
	while (pf < pchUseCMin+cUseCUsed)
		if (*pf)
			break;
		else
			pf++;

#define cUseCUsed   36
	if (*(pchUseCMin+cUseCUsed-1) & 0xC0)
		{
		*pch1++ = '/';
		*pch2++ = '/';
		*pch1++ = 'X';
		*pch2++ = 'X';
		}

	else  if (pf < pchUseCMin+cUseCUsed)
		{
		*pch1++ = '/';
		*pch2++ = '/';

		cDBS = cUseCUsed;
		pf = pchUseCMin;
		while (cDBS > 0)
			{
			for (w = 0, iw = 0; iw < 12; iw++)
				w = (w << 1) | (cDBS-- > 0 && (*pf++ & 0x03) ? 1 : 0);
			CchIntToAsciiHex (w, &pch1, 3);
			CchIntToAsciiHex (w, &pch2, 3);
			if (cDBS > 0)
				{
				*pch1++ = '-';
				*pch2++ = '-';
				}
			}
		*pch1++ = '/';
		*pch2++ = '/';

		cDBS = cUseCUsed;
		pf = pchUseCMin;
		while (cDBS > 0)
			{
			for (w = 0, iw = 0; iw < 12; iw++)
				w = (w << 1) | (cDBS-- > 0 && (*pf++ & 0x0C) ? 1 : 0);
			CchIntToAsciiHex (w, &pch1, 3);
			CchIntToAsciiHex (w, &pch2, 3);
			if (cDBS > 0)
				{
				*pch1++ = '-';
				*pch2++ = '-';
				}
			}
		*pch1++ = '/';
		*pch2++ = '/';

		cDBS = cUseCUsed;
		pf = pchUseCMin;
		while (cDBS > 0)
			{
			for (w = 0, iw = 0; iw < 12; iw++)
				w = (w << 1) | (cDBS-- > 0 && (*pf++ & 0x30) ? 1 : 0);
			CchIntToAsciiHex (w, &pch1, 3);
			CchIntToAsciiHex (w, &pch2, 3);
			if (cDBS > 0)
				{
				*pch1++ = '-';
				*pch2++ = '-';
				}
			}
		*pch1++ = '/';
		*pch2++ = '/';

		cDBS = 12;  /* don't put cUseCUsed here, it overflows rgch2 */
		pf = pchUseCMin;
		while (cDBS > 0)
			{
			for (w = 0, iw = 0; iw < 12; iw++)
				w = (w << 1) | (cDBS-- > 0 && (*pf++ & 0xC0) ? 1 : 0);
			CchIntToAsciiHex (w, &pch1, 3);
			CchIntToAsciiHex (w, &pch2, 3);
			if (cDBS > 0)
				{
				*pch1++ = '-';
				*pch2++ = '-';
				}
			}
		}
	CchPchToPpch (SzShared(") Version: "), &pch1);
	CchPchToPpch (SzShared(") v"), &pch2);

	CchIntToPpch ((nRevProduct*100)+nIncrProduct,&pch1);
	CchIntToPpch ((nRevProduct*100)+nIncrProduct,&pch2);

	CchPchToPpch (SzShared(",  Swap Size: "), &pch1);
	CchIntToPpch (vsasCur, &pch1);

	*pch1 = 0;
	*pch2 = 0;

	Assert (pch1-rgch1 < cchDbgBuf1);    /* size of buffers in callers */
	Assert (pch2-rgch2 < cchDbgBuf2);
}


/* %%Function:DumpState %%Owner:BRADV */
DumpState()

{
	CHAR rgch1 [cchDbgBuf1];
	CHAR rgch2 [cchDbgBuf2];

	GetStateInfo (rgch1, rgch2);
	CommSzCrLf (rgch1);
	ScribbleSz (rgch2);

		{
		extern int vcVisiCalls;
		extern int vcVisiUsedCache;
		CommSzNum(SzShared("vcVisiCalls = "), vcVisiCalls);
		CommSzNum(SzShared("vcVisiUsedCache = "), vcVisiUsedCache);
		}
}


/* %%Function:DumpVfli %%Owner:BRADV */
DumpVfli()

{
#ifdef PETERWILLFIXTHISREALLY  /* sure he will.... */
	CHAR rgchInfo [2000];
	Assert (CchAsciiFli (rgchInfo, 2000) < 2000);
	CommMultiLineSz (rgchInfo);
#endif
}


/* %%Function:PCJDbg1 %%Owner:BRADV */
PCJDbg1()
{
}


/* %%Function:PCJDbg2 %%Owner:BRADV */
PCJDbg2()
{
}


/* %%Function:PCJDbg3 %%Owner:BRADV */
PCJDbg3()
{
}


/* %%Function:DumpHeap %%Owner:BRADV */
DumpHeap(sb)
SB sb;
{
	CHAR rgch [256];
	CHAR *pch;
	int cb;

	pch = rgch;
	CchPchToPpch(SzShared("\r\nHeap:  sb = "), &pch);
	CchIntToPpch(sb, &pch);
	CchPchToPpch(SzShared(", cbSb = "), &pch);
	CchLongToPpch((long)CbSizeSb(sb), &pch);
	CchPchToPpch(SzShared(", cbAvail = "), &pch);
	CchLongToPpch((long)CbAvailHeap(sb), &pch);
	*pch = 0;
	CommSzCrLf(rgch);

		/* dump heap info */
		{
		CKL ckl;
		GetHeapInfo(sb, &ckl);
		pch = rgch;
		CchPchToPpch(SzShared("Used/Free: "), &pch);
		CchLongToPpch((long)ckl.chUsed, &pch);
		*pch++ = '/';
		CchLongToPpch((long)ckl.chFree, &pch);
		CchPchToPpch(SzShared(", Fixed cblk/cb: "), &pch);
		CchLongToPpch((long)ckl.cblkFixed, &pch);
		*pch++ = '/';
		CchLongToPpch((long)ckl.cbFixed, &pch);
		CchPchToPpch(SzShared(", Moveable cblk/cb: "), &pch);
		CchLongToPpch((long)ckl.cblkMoveable, &pch);
		*pch++ = '/';
		CchLongToPpch((long)ckl.cbMoveable, &pch);
		*pch = 0;
		CommSzCrLf(rgch);
		}

	CommSz(SzShared("\r\n"));

		/* Fixed blocks */
		{
		VOID *pv = NULL;

		while ((pv = PvWalkFixedHeap (sb, pv)) != NULL)
			{
			pch = rgch;
			CchPchToPpch(SzShared("Fixed: pv = "), &pch);
			CchLongToPpch((long)pv, &pch);
			if (sb == sbDds)
				{
				CchPchToPpch(SzShared(", cb = "), &pch);
				CchLongToPpch((long)(cb = (*((WORD *)(pv)-1))), &pch);
				}
			*pch = 0;
			CommSzCrLf (rgch);

			if (sb == sbDds)
				DumpPv (pv, cb, rgch);
			}
		}

	CommSz(SzShared("\r\n"));

		/* moveable blocks */
		{
		VOID **ppv = NULL;

		while ((ppv = PpvWalkHeap (sb, ppv)) != NULL)
			{
			pch = rgch;
			CchPchToPpch(SzShared("Moveable: ppv = "), &pch);
			CchLongToPpch((long)ppv, &pch);
			if (sb == sbDds)
				{
				CchPchToPpch(SzShared(", pv = "), &pch);
				CchLongToPpch((long)*ppv, &pch);
				CchPchToPpch(SzShared(", cb = "), &pch);
				CchLongToPpch((long)(cb = CbSizePpvSbCur(ppv)), &pch);
				}
			*pch = 0;
			CommSzCrLf (rgch);

			if (sb == sbDds)
				DumpPv (*ppv, cb, rgch);
			}
		}

	DumpFarSbHeaps();
}


/* %%Function:ChCleanFromCh %%Owner:BRADV */
CHAR ChCleanFromCh(ch)
CHAR ch;
{
	CHAR chT = ch & 0x7f;
	if (chT < 32 || chT == 0x7f)
		return '.';
	else
		return ch;
}


/* %%Function:DumpPv %%Owner:BRADV */
DumpPv(pv, cb, rgch)
VOID *pv;
int cb;
CHAR rgch[];
{
	CHAR *pchWords = rgch, *pchChars = rgch + 53;
	CHAR *pchData = (CHAR *)pv;
	CHAR ch;
	unsigned *pwData = (unsigned *)pv;
	int cw = min (cb, 20)/2;

	SetBytes (rgch, ' ', 73);

	while (cw--)
		{
		CchIntToAsciiHex(*pwData++, &pchWords, 4);
		pchWords++;

		*pchChars++ = ChCleanFromCh(*pchData++);
		*pchChars++ = ChCleanFromCh(*pchData++);
		}
	rgch [73] = 0;

	CommSzCrLf(rgch);
}


/* %%Function:DumpHeapUsage %%Owner:BRADV */
DumpHeapUsage()

{
	int doc;
	int cb = 0;
	CHAR rgch [85];
	CHAR *pch = rgch;
	int i;

	extern KMP ** hkmpBase;
	extern KMP ** hkmpCur;
	extern KMP ** vhkmpUser;
	extern MUD ** vhmudUser;
	extern MUD ** vhmudBase;
	extern struct STTB ** hsttbMenu;
	extern char  **vhgrpchr;
	extern struct STTB     **vhsttbFont;   /* Master font table */
	extern struct WWD      **hwwdCur; /* handle to the current pane inside a child window */
	extern struct MWD      **hmwdCur; /* handle to the current child window */
	extern struct STTB  **vhsttbOpen;
	extern struct STTB     **vhsttbWnd;
	extern struct RGS      **vhrgs;    /* ruler global state */
	extern struct PLC   **vhplcfrl;             /* footnote references */
	extern struct PRC      **vhprc;
	extern CHAR  **hstDMQPath;
	extern CHAR  **hszvsumdFile;
	extern struct FINDFILE  **hDMDTA;    /* search results from FFirst and FNext */
	extern char  **rghszDMDirs[];
	extern char  **hszXfOptDir;
	extern char  **hszFullViewFile;
	extern int   chszDMDirs;

	ReportHeapSz(SzShared ("Heap usage"), 0);

	CommSz (SzShared ("Doc heap usage\r\n"));
	for (doc = 0; doc < docMac; doc++)
		if (mpdochdod [doc] != hNil)
			{
			cb += CbDumpDocHandles (doc, fTrue);
			}

	CommSz (SzShared ("Keymap/menu items\r\n"));
	cb += CbHToPpch (SzShared (" stMenu:"  ),hsttbMenu   , &pch);
	cb += CbHToPpch (SzShared (" mudB:"  ), vhmudBase   , &pch);
	cb += CbHToPpch (SzShared (" mudU:"  ), vhmudUser   , &pch);
	cb += CbHToPpch (SzShared (" kmpB:"  ), hkmpBase   , &pch);
	cb += CbHToPpch (SzShared (" kmpC:"  ), hkmpCur   , &pch);
	cb += CbHToPpch (SzShared (" kmpU:"  ), vhkmpUser   , &pch);
	SendBuffer (&pch, rgch);

	CommSz (SzShared ("Sprm/prl/footnote items\r\n"));
	cb += CbHToPpch (SzShared (" plcfrl:"  ),vhplcfrl   , &pch);
	cb += CbHToPpch (SzShared (" prc:"  ),vhprc   , &pch);
	SendBuffer (&pch, rgch);

	CommSz (SzShared ("Font items\r\n"));
	cb += CbHToPpch (SzShared (" stFont:"  ),vhsttbFont   , &pch);
	SendBuffer (&pch, rgch);

	CommSz (SzShared ("Catalog items\r\n"));
	cb += CbHToPpch (SzShared (" DMQPath:"  ),hstDMQPath   , &pch);
	cb += CbHToPpch (SzShared (" sumdF:"  ), hszvsumdFile  , &pch);
	cb += CbHToPpch (SzShared (" DMDTA:"  ),hDMDTA   , &pch);
	for (i = 0; i < chszDMDirs; i++)
		{
		cb += CbHToPpch (SzShared (" Dirs:"  ),rghszDMDirs[i]   , &pch);
		CheckBuffer (&pch, rgch);
		}
	SendBuffer (&pch, rgch);

	CommSz (SzShared ("Misc items\r\n"));
	cb += CbHToPpch (SzShared ("stOpen:"  ), vhsttbOpen , &pch);
	cb += CbHToPpch (SzShared (" grpchr:"  ), vhgrpchr , &pch);
	cb += CbHToPpch (SzShared (" stWnd:"  ), vhsttbWnd , &pch);
	cb += CbHToPpch (SzShared (" wwdC:"  ), hwwdCur , &pch);
	cb += CbHToPpch (SzShared (" mwdC:"  ), hmwdCur , &pch);
	SendBuffer (&pch, rgch);

	CommSzNum (SzShared ("cb total from heap = "), cb);
}


/* %%Function:CbDumpDocHandles %%Owner:BRADV */
int CbDumpDocHandles (doc, fDocToo)
int doc;
BOOL fDocToo;
{
	struct DOD *pdod = PdodDoc (doc);
	CHAR rgch [85];
	CHAR *pch = rgch;
	int cb = 0;

	if (fDocToo)
		{
		CchPchToPpch (SzShared("doc "), &pch);
		CchIntToAsciiHex (doc, &pch, 2);
		}
	else
		CchPchToPpch (SzShared("      "), &pch);

	cb += CbHToPpch (SzShared (" pcd:"  ), pdod->hplcpcd   , &pch);
	cb += CbHToPpch (SzShared (" fld:"  ), pdod->hplcfld   , &pch);
	cb += CbHToPpch (SzShared (" fnd:"  ), pdod->hplcfnd   , &pch);
	cb += CbHToPpch (SzShared (" pgd:"  ), pdod->hplcpgd   , &pch);
	if (!pdod->fShort)
		{
		cb += CbHToPpch (SzShared (" asoc:" ), pdod->hsttbAssoc, &pch);
		cb += CbHToPpch (SzShared (" sed:"  ), pdod->hplcsed   , &pch);
		CheckBuffer (&pch, rgch);
		cb += CbHToPpch (SzShared (" sea:"  ), pdod->hplcsea   , &pch);
		CheckBuffer (&pch, rgch);
		cb += CbHToPpch (SzShared (" frd:"  ), pdod->hplcfrd   , &pch);
		cb += CbHToPpch (SzShared (" pad:"  ), pdod->hplcpad   , &pch);
		CheckBuffer (&pch, rgch);
		cb += CbHToPpch (SzShared (" atrd:" ), pdod->hplcatrd  , &pch);
		cb += CbHToPpch (SzShared (" styn:" ), pdod->stsh.hsttbName,  &pch);
		CheckBuffer (&pch, rgch);
		cb += CbHToPpch (SzShared (" chpx:" ), pdod->stsh.hsttbChpx,  &pch);
		cb += CbHToPpch (SzShared (" papx:" ), pdod->stsh.hsttbPapx,  &pch);
		CheckBuffer (&pch, rgch);
		cb += CbHToPpch (SzShared (" stcp:" ), pdod->stsh.hplestcp,  &pch);
		cb += CbHToPpch (SzShared (" chpe:" ), pdod->hsttbChpe , &pch);
		CheckBuffer (&pch, rgch);
		cb += CbHToPpch (SzShared (" pape:" ), pdod->hsttbPape , &pch);
		cb += CbHToPpch (SzShared (" bkf:"  ), pdod->hplcbkf   , &pch);
		CheckBuffer (&pch, rgch);
		cb += CbHToPpch (SzShared (" bkl:"  ), pdod->hplcbkl   , &pch);
		cb += CbHToPpch (SzShared (" bkmk:" ), pdod->hsttbBkmk , &pch);
		CheckBuffer (&pch, rgch);
		cb += CbHToPpch (SzShared (" ftc:"  ), pdod->hmpftcibstFont, &pch);
		CheckBuffer (&pch, rgch);
		}
	if (pdod->fGlsy)
		cb += CbHToPpch (SzShared (" glsy:" ), pdod->hsttbGlsy , &pch);
	if (pdod->fDot)
		{
		cb += CbHToPpch (SzShared (" kmp:"  ), pdod->hkmpUser  , &pch);
		cb += CbHToPpch (SzShared (" mud:"  ), pdod->hmudUser  , &pch);
		}
	SendBuffer (&pch, rgch);
	return (cb);
}



/* %%Function:CbHToPpch %%Owner:BRADV */
int CbHToPpch(pch, h, ppch)
char *pch;
uns *h;
char **ppch;

{
	int cch = 0;
	int cb = 0;

	if (h != hNil)
		{
		cch += CchPchToPpch (pch, ppch);
		cch += CchIntToPpch ((cb = CbOfH(h)), ppch);
		}
	Assert (cch < 13);
	return (cb);
}


#ifdef DLMEM
/* %%Function:DoFarHeapTest %%Owner:peterj */
DoFarHeapTest()
{
	HQ rghq[6];
	long rgcb[6];
	long cb;
	int ihq;
	int chCmd;
	int cbIncrLarge = 1500;
	int cbIncrSmall = 50;
	int *pcb;

	for (ihq = 0; ihq < 6; ihq++)
		{
		rghq[ihq] = hqNil;
		rgcb[ihq] = 0;
		}
	for (;;)
		{
		CommSz(SzShared("\r\n"));
		for (ihq = 0; ihq < 6; ihq++)
			ShowHq(ihq, &rghq[ihq], &rgcb[ihq]);
		DumpFarSbHeaps();
		CkFarHeaps();
		ReportHeapSz(SzShared("Memory:"),0);
		CommSz(SzShared("\r\n"));

LGetCmd:
		chCmd = ChFHCommand();

		switch (chCmd)
			{

		case 'i':
			pcb = &cbIncrSmall;
			break;
		case 'I':
			pcb = &cbIncrLarge;
			break;
		case 'q': 
		case 'Q':      /* quit */
			for (ihq = 0; ihq < 6; ihq++)
				if (rghq[ihq] != hqNil)
					FreeHq(rghq[ihq]);
			return;
		case 'a': 
		case 'A':
			ihq = IhqCommand();
			if (rghq[ihq] != hqNil)
				{
				Beep(); 
				vfAwfulNoise = fFalse;
				goto LGetCmd;
				}
			else
				break;
		case 'g': 
		case 'G':
		case 's': 
		case 'S':
		case 'f': 
		case 'F':
			ihq = IhqCommand();
			if (rghq[ihq] == hqNil)
				{
				Beep(); 
				vfAwfulNoise = fFalse;
				goto LGetCmd;
				}
			else
				break;
		default:
			Beep(); 
			vfAwfulNoise = fFalse;
			CommSz(SzShared("Unknown command!\r\n"));
			goto LGetCmd;
			}


		CommSz(SzShared("\r\n"));

		switch (chCmd)
			{
		case 'i': 
		case 'I':	/* change increment */
				{
				char rgch[10], *pch = rgch, ch;
				CommSz(SzShared("New increment:\r\n"));
				while (pch < &rgch[9] && 
						(ch = ChFromComm()) != '\r' &&
						ch != '\n')
					*pch++ = ch;
				*pch = 0;
				*pcb = WFromSzNumber(rgch);
				CommSzNum(SzShared("New value="), *pcb);
				goto LGetCmd;
				}

		case 'a': 
		case 'A':     /* allocate */
			cb = 1000;
			if ((rghq[ihq] = HqAllocLcb((long)cb)) == hqNil)
				goto LFailed;
			FillHq(rghq[ihq], 0l, cb);
			break;
		case 'g':
			cb = CpMin(0x0000ffff, rgcb[ihq] + cbIncrSmall);
			goto LChngSize;
		case 'G':
			cb = CpMin(0x0000ffff, rgcb[ihq] + cbIncrLarge);
			goto LChngSize;
		case 's':
			cb = CpMax (10L, rgcb[ihq] - cbIncrSmall);
			goto LChngSize;
		case 'S':
			cb = CpMax (10L, rgcb[ihq] - cbIncrLarge);
LChngSize:
			if (!FChngSizePhqLcb(&rghq[ihq], (long)cb))
				goto LFailed;
			if (cb > rgcb[ihq])
				FillHq(rghq[ihq], rgcb[ihq], cb);
			break;
		case 'f': 
		case 'F':
			FreeHq(rghq[ihq]);
			cb = 0;
			rghq[ihq] = hqNil;
			break;
			}
		rgcb[ihq] = cb;
		continue;
LFailed:
		CommSz(SzShared("\007Failed!\r\n"));
		}
}


/* %%Function:ChFHCommand %%Owner:peterj */
ChFHCommand()
{
	CHAR ch, rgch[4];
	CommSz(SzShared("Command (a,f,g,s,i,q):\r\n"));
	ch = ChFromComm();
	rgch[0]=ch; 
	rgch[1]=' '; 
	rgch[2]=' '; 
	rgch[3]=0;
	CommSz(rgch);
	return ch;
}


/* %%Function:IhqCommand %%Owner:peterj */
IhqCommand()
{
	CHAR ch, rgch[4];
	CommSz(SzShared("ihq (0-5):\r\n"));
	for (;;)
		{
		ch = ChFromComm();
		if ((uns) (ch - '0') <= ('5' - '0'))
			break;
		Beep(); 
		vfAwfulNoise = fFalse;
		}
	rgch[0]=ch; 
	rgch[1]=' '; 
	rgch[2]=' '; 
	rgch[3]=0;
	CommSz(rgch);
	return ch - '0';
}


/* %%Function:FillHq %%Owner:peterj */
FillHq(hq, ichMin, ichLim)
HQ hq;
long ichMin, ichLim;
{{
	CHAR HUGE *hpch = HpOfHq(hq);
	hpch += ichMin;
	while (ichMin < ichLim)
		*hpch++ = (ichMin++ & 0xff);
}}


/* %%Function:ShowHq %%Owner:peterj */
ShowHq(ihq, phq, pcb)
int ihq;
HQ *phq;
long *pcb;
{
	CHAR rgch[90];
	CHAR *pch;
	CHAR HUGE *hpch;
	long cb, ich;

	pch = rgch;
	CchIntToPpch(ihq, &pch);
	if (*phq != hqNil)
		{
		CchPchToPpch(SzShared(": sb="), &pch);
		CchLongToPpch((long)SbOfHp(*phq), &pch);
		CchPchToPpch(SzShared(", ibH="), &pch);
		CchLongToPpch((long)IbOfHp(*phq), &pch);
		CchPchToPpch(SzShared(", ibD="), &pch);
		CchLongToPpch((long)**phq, &pch);
		CchPchToPpch(SzShared(", cb="), &pch);
		CchLongToPpch((cb = *pcb),&pch);
		if (cb != CbOfHq(*phq))
			{
			CchPchToPpch(SzShared(" Cb mis-match! Cb(hq)="), &pch);
			CchLongToPpch(CbOfHq(*phq), &pch);
			}
		hpch = (CHAR HUGE *)HpOfHq(*phq);
		ich = 0;
			{{
			while (ich < cb)
				if (*hpch++ != (ich++ & 0xff))
					{
					CchPchToPpch(SzShared(" Bogus data!"), &pch);
					break;
					}
			}}
		}
	else
		CchPchToPpch(SzShared(": (nil)"), &pch);

	*pch = 0;
	CommSzCrLf(rgch);
}


#define iwFooMax    4
#define nTest       10
#define cbFOO       sizeof(struct FOO)

struct FOO 
	{
	int n;
	int rgw[iwFooMax];
};


/* %%Function:TestExternalPlcs %%Owner:bradv */
TestExternalPlcs()
{
	struct PLC **hplc;
	struct FOO foo;
	int n, i;
	char rgch[10], *pch;

	AssertH( hplc = HplcInit(cbFOO, 2, (CP)(nTest*2), fTrue) );

	for (n=0; n < nTest; n++)
		{
		foo.n = n;
		for (i = 0; i < iwFooMax; i++)
			foo.rgw[i] = i;
		FInsertInPlc (hplc, 0, (CP)(nTest-n-1)*2, &foo);
		}

	for (n=0; n < nTest; n++)
		{
		GetPlc(hplc, n, &foo);
		pch = rgch;
		CchLongToPpch(CpPlc(hplc, n), &pch);
		*pch++ = ':';
		*pch = 0;
		CommSzRgNum(rgch, &foo, CwFromCch(cbFOO));
		foo.n = n;
		PutPlcLast(hplc, n, &foo);
		}

	CommSz(SzShared("\r\n"));

	AdjustHplc(hplc, (CP)9, (CP)100, -1);

	for (n=0; n < nTest; n++)
		{
		GetPlc(hplc, n, &foo);
		pch = rgch;
		CchLongToPpch(CpPlc(hplc, n), &pch);
		*pch++ = ':';
		*pch = 0;
		CommSzRgNum(rgch, &foo, CwFromCch(cbFOO));
		}

		{
		CommSzNumSzNum(SzFrame("IInPlc(50):    "), IInPlc(hplc, 50),
				SzFrame("   (115): "), IInPlc(hplc, 115));

		CommSzNumSzNum(SzFrame("IInPlcRef(50): "), IInPlcRef(hplc, 50),
				SzFrame("   (115): "), IInPlcRef(hplc, 115));

		CommSzNumSzNum(SzFrame("IInPlcMult(50):"), IInPlcMult(hplc, 50),
				SzFrame("   (115): "), IInPlcMult(hplc, 115));

		CommSzNumSzNum(SzFrame("IInPlc2(50):   "), IInPlc2(hplc, 50, 1),
				SzFrame("   (115): "), IInPlc2(hplc, 115, 1));
		}

	FreeHplc(hplc);
}


#endif /* DLMEM */

/* %%Function:CommSzNumSzNum %%Owner:BRADV */
CommSzNumSzNum(sz1, n1, sz2, n2)
char *sz1, *sz2;
int n1, n2;
{
	char rgch [80], *pch = rgch;

	CchPchToPpch(sz1, &pch);
	CchUnsToPpch(n1, &pch);
	CchPchToPpch(sz2, &pch);
	CchUnsToPpch(n2, &pch);
	*pch = 0;
	CommSzCrLf(rgch);
}


/* %%Function:DumpFarSbHeaps %%Owner:BRADV */
DumpFarSbHeaps()
{
	extern struct PLSBHI **vhplsbhi;
	struct PLSBHI *pplsbhi = *vhplsbhi;
	struct SBHI *psbhi = pplsbhi->rgsbhi;
	int isbhi = 0, isbhiMac = pplsbhi->isbhiMac;
	CHAR rgch[90], *pch;

	CommSz(SzShared("\r\nFar sb heaps:\r\n"));

	while (isbhi < isbhiMac)
		{
		SetBytes(rgch, ' ', 38);
		pch = rgch;

		CchIntToPpch(isbhi, &pch);
		CchPchToPpch(SzShared(": sb="), &pch);
		CchLongToPpch((long)psbhi->sb, &pch);
		CchPchToPpch(SzShared(" ch="), &pch);
		CchIntToPpch(psbhi->ch, &pch);
		CchPchToPpch(SzShared(" cb="), &pch);
		CchLongToPpch((long)CbSizeSb(psbhi->sb), &pch);
		*pch++=' ';
		if (psbhi->fHasHeap)
			{
			CchPchToPpch(SzShared("cbA="), &pch);
			CchIntToPpch(CbAvailHeap(psbhi->sb), &pch);
			*pch++=' ';
			}
		else
			CchPchToPpch(SzShared("BIG "), &pch);
		if (psbhi->fHasFixed)
			CchPchToPpch(SzShared("X"), &pch);
		if (FInEmmSb(psbhi->sb))
			CchPchToPpch(SzShared("E"), &pch);
		isbhi++; 
		psbhi++; 
		*pch++ = ' ';

		pch = max(pch, &rgch[38]);
		if (isbhi >= isbhiMac)
			goto LDump;

		CchIntToPpch(isbhi, &pch);
		CchPchToPpch(SzShared(": sb="), &pch);
		CchLongToPpch((long)psbhi->sb, &pch);
		CchPchToPpch(SzShared(" ch="), &pch);
		CchIntToPpch(psbhi->ch, &pch);
		CchPchToPpch(SzShared(" cb="), &pch);
		CchLongToPpch((long)CbSizeSb(psbhi->sb), &pch);
		*pch++=' ';
		if (psbhi->fHasHeap)
			{
			CchPchToPpch(SzShared("cbA="), &pch);
			CchIntToPpch(CbAvailHeap(psbhi->sb), &pch);
			*pch++=' ';
			}
		else
			CchPchToPpch(SzShared("BIG "), &pch);
		if (psbhi->fHasFixed)
			CchPchToPpch(SzShared("X"), &pch);
		if (FInEmmSb(psbhi->sb))
			CchPchToPpch(SzShared("E"), &pch);
		isbhi++; 
		psbhi++;

LDump:
		*pch = 0;
		CommSzCrLf(rgch);
		}
}


/* %%Function:CommStNL %%Owner:BRADV */
CommStNL(st)
CHAR * st;
{
	CHAR sz[256];

	StToSz(st, sz);
	CommSzCrLf(sz);
}


/* %%Function:ShowDisplayPara %%Owner:BRADV */
ShowDisplayPara()
{
	CP cpFirst, cpLim;
	CHAR rgch[80], *pch = rgch;

	GetCpFirstCpLimDisplayPara(selCur.ww, selCur.doc,
			selCur.cpFirst, &cpFirst, &cpLim);
	CchPchToPpch(SzShared("Display para:  selCur.cpFirst="), &pch);
	CchLongToPpch(selCur.cpFirst, &pch);
	CchPchToPpch(SzShared(":  cpFirst="), &pch);
	CchLongToPpch(cpFirst, &pch);
	CchPchToPpch(SzShared(", cpLim="), &pch);
	CchLongToPpch(cpLim, &pch);
	*pch = 0;
	CommSzCrLf(rgch);
}


/* %%Function:ScribbleDirty %%Owner:BRADV */
ScribbleDirty(doc)
int doc;
{
	if (doc == docNil)
		ScribbleSz(SzShared("  "));
	else
		{
		CHAR szMsg[3];

		if (DiDirtyDoc(doc))
			szMsg[0] = 'D';
		else
			szMsg[0] = 'C';

		if (PdodDoc(doc)->fFormatted)
			szMsg[1] = 'F';
		else
			szMsg[1] = 'N';
		szMsg[2] = 0;
		ScribbleSz(szMsg);
		}
}


/* %%Function:BlankBackground %%Owner:BRADV */
NATIVE BlankBackground(hdc, prc)
HDC hdc;
struct RC * prc;
{
	long rgbBkPrev = SetBkColor(hdc, vsci.rgbBkgrnd);
	char rgch[1];

	ExtTextOut(hdc, prc->xpLeft, prc->ypTop, ETO_OPAQUE, (LPSTR)prc, (LPSTR)rgch,
			0, NULL);

	if (rgbBkPrev != vsci.rgbBkgrnd)
		SetBkColor(hdc, rgbBkPrev);
}


/* %%Function:ShowPatBlts %%Owner:BRADV */
ShowPatBlts()
{
	int i;
	HWND hwnd = (*hwwdCur)->hwnd;
	HDC hdc = GetDC(hwnd);
	struct RC rc;
	CHAR rgch[1];
	Assert(hdc != NULL);
	SetBkColor(hdc, vsci.rgbBkgrnd);
	GetClientRect(hwnd, (LPSTR)&rc);

	PatBlt(hdc, rc.xpLeft, rc.ypTop, rc.xpRight - rc.xpLeft,
			rc.ypBottom - rc.ypTop, vsci.ropErase);
	ExtTextOut(hdc, rc.xpLeft, rc.ypTop, ETO_OPAQUE, (LPSTR)&rc, 
			(LPSTR)rgch, 0, NULL);

	DebugBreak(1);

	for (i = 0; i < 100; i++)
		{
		PatBlt(hdc, rc.xpLeft, rc.ypTop, rc.xpRight - rc.xpLeft,
				rc.ypBottom - rc.ypTop, vsci.ropErase);
		ExtTextOut(hdc, rc.xpLeft, rc.ypTop, ETO_OPAQUE, (LPSTR)&rc, 
				(LPSTR)rgch, 0, NULL);
		}

	DebugBreak(1);

}


#ifdef PROTOTYPE
/* %%Function:BlankBackground %%Owner:NOTUSED */
NATIVE BlankBackground(hdc, prc)
HDC hdc;
struct RC * prc;
{
	long rgbBkPrev = SetBkColor(hdc, vsci.rgbBkgrnd);
	char rgch[1];

	ExtTextOut(hdc, prc->xp, prc->yp, ETO_OPAQUE, (LPSTR)prc, (LPSTR)rgch,
			0, NULL);

	if (rgbBkPrev != vsci.rgbBkgrnd)
		SetBkColor(hdc, rgbBkPrev);
}


#endif



/* T E S T  S C A N  F N  F O R  B Y T E S */
/* make sure scanfnforbytes can handle > 32K reads and writes */
/* %%function:testscanfnforbytes %%owner:bradv */
TestScanFnForBytes()
{
#define ibMac (uns)40000
	HQ hq;
	int fn = fnNil;
	int ib;
	CHAR HUGE *hpch;
	CHAR stTemp[ichMaxFile];

	if ((hq = HqAllocLcb((long)ibMac)) == hqNil)
		{
		ReportSz("not enough memory to allocate buffer");
		goto LReturn;
		}

	stTemp[0] = 0;
	if ((fn = FnOpenSt(stTemp, fOstCreate, ofcTemp, NULL)) == fnNil)
		{
		ReportSz("could not create temp file");
		goto LReturn;
		}

	/* fill buffer with known values */
	for (ib = 0, hpch = HpOfHq(hq); ib < ibMac; ib++)
		*hpch++ = (ib & 0xff);

	/* write 1K of buffer to disk */
	SetFnPos(fn, fc0);
	WriteHprgchToFn(fn, HpOfHq(hq), 1024);

	/* write full buffer to disk */
	SetFnPos(fn, fc0);
	WriteHprgchToFn(fn, HpOfHq(hq), ibMac);

	/* clear the buffer */
	for (ib = 0, hpch = HpOfHq(hq); ib < ibMac; ib++)
		*hpch++ = 0;

	/* read full buffer from file */
	SetFnPos(fn, fc0);
	ReadHprgchFromFn(fn, HpOfHq(hq), ibMac);

	/* check values in buffer */
	for (ib = 0, hpch = HpOfHq(hq); ib < ibMac; ib++)
		{
		if (*hpch++ != (ib & 0xff))
			{
			ReportSz("failed 1");
			CommSzNum("", ib);
			break;
			}
		}

	/* read first 1K */
	SetFnPos(fn, fc0);
	ReadHprgchFromFn(fn, HpOfHq(hq), 1024);

	/* check values in buffer */
	for (ib = 0, hpch = HpOfHq(hq); ib < 1024; ib++)
		{
		if (*hpch++ != (ib & 0xff))
			{
			ReportSz("failed 2");
			CommSzNum("", ib);
			break;
			}
		}

LReturn:
	if (hq != hqNil)
		FreeHq(hq);

	if (fn != fnNil)
		DeleteFn(fn, fTrue);
}


#endif /* DEBUG */
