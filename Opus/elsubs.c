/* elsubs.c -- Support for the EL interpreter */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "debug.h"
#include "heap.h"
#include "el.h"
#include "sdmparse.h"
#include "idd.h"
#include "tmc.h"
#include "dlbenum.h"

#include "apprun.hs"
#include "cust.hs"
#include "username.hs"
#include "docsum.hs"
#include "docstat.hs"
#include "viewpref.hs"
#include "index.hs"
#include "indexent.hs"
#include "revmark.hs"
#include "chgpr.hs"
#include "recorder.hs"
#include "pastelnk.hs"
#include "confirmr.hs"
#include "bookmark.hs"
#include "sect.hs"
#include "doc.hs"
#include "autosave.hs"
#include "para.hs"
#include "printmrg.hs"
#include "sort.hs"
#include "footnote.hs"
#include "print.hs"
#include "pict.hs"
#include "vrfcnvtr.hs"
#include "prompt.hs"
#include "edmacro.hs"
#include "renmacro.hs"
#include "runmacro.hs"
#include "showvars.hs"
#include "glsy.hs"
#include "goto.hs"
#include "newopen.hs"
#include "renum.hs"
#include "thesaur.hs"
#include "search.hs"
#include "replace.hs"
#include "tabs.hs"
#include "char.hs"
#include "style.hs"
#include "edstyle.hs"
#include "renstyle.hs"
#include "asgn2key.hs"
#include "asgn2mnu.hs"
#include "about.hs"
#include "hyphen.hs"
#include "open.hs"
#include "new.hs"
#include "insfile.hs"
#include "inspic.hs"
#include "toc.hs"
#include "spell.hs"
#include "spellmm.hs"
#include "catalog.hs"
#include "catprog.hs"
#include "catsrch.hs"
#include "saveas.hs"
#include "header.hs"
#include "mrgstyle.hs"
#include "cmpfile.hs"
#include "insfield.hs"
#include "password.hs"
#include "insbreak.hs"
#include "abspos.hs"
#include "tablecmd.hs"
#include "tablefmt.hs"
#include "tableins.hs"
#include "tabletxt.hs"
#include "inspgnum.hs"
#include "usrdlg.hs"

#ifdef DEBUG
#include "dbgfail.hs"
#include "dbgfile.hs"
#include "dbgfunc.hs"
#include "dbgmem.hs"
#include "dbgpref.hs"
#include "dbgrare.hs"
#include "dbgscrbl.hs"
#include "dbgtests.hs"
#include "dbgusec1.hs"
#include "dbgusec2.hs"
#include "dbgusec3.hs"
#include "dbgusec4.hs"
#endif


#include "elxdefs.h"
#include "elxinfo.h"

#include "elxprocs.c"


#include "rerr.h"

/* %%Function:ElkUserSt %%Owner:bradch */
ELK ElkUserSt(st)
char * st;
{
	int ibst;
	extern struct STTB ** vhsttbElkUser;

	if (vhsttbElkUser == hNil)
		{
		if ((vhsttbElkUser = HsttbInit(0, fTrue)) == hNil)
			goto Loom;
		goto LAddSt;
		}

	if (!FSearchSttbUnsorted(vhsttbElkUser, st, &ibst))
		{
LAddSt:
		if ((ibst = IbstAddStToSttb(vhsttbElkUser, st)) == ibstNil)
			{
Loom:
			RtError(rerrOutOfMemory);
			Assert(fFalse); /* NOT REACHED */
			}
		}

	return ibst + elkAppMac;
}




/* EL Parsed Item Descriptor */
/* NOTE: The bitfields in this structure happened to just work out
perfectly for the first Opus.  If new parse procs are added, or more
parsed items are added in the future, however, all the bits are already
used.  I suggest that the opt field be reworked if this happens, we
do not really need all 9 bits there, it just made the code simpler.

Note bz: I made opt 8 bits and ipfn 4 so we have another 8 parse proces to work with
*/

typedef struct _elpid
	{
	int ihid : 4;
	int ipfnWParse : 4;
	int opt : 8;
	TMC tmc;
} ELPID;

csconst int csrghid [] =
{
	IDDAbsPos,
			IDDAutosave,
		/*	IDDCatSearch,*/
	IDDCatalog,
			IDDCharacter,
			IDDDocument,
			IDDHeader,
			IDDParaLooks,
			IDDFormatPic,
			IDDPrint,
			IDDPrintMerge,
			IDDSection,
			IDDSort,
			IDDFormatTable,
			IDDInsertTable,
			IDDInsToc,
			IDDViewPref
};


#define ihidMax (sizeof (csrghid) / sizeof (int))


#define ihidAbsPos		0
#define ihidAutosave		1
#define ihidCatSearch		2
#define ihidCharacter		3
#define ihidDocument		4
#define ihidHeader		5
#define ihidParaLooks		6
#define ihidFormatPic		7
#define ihidPrint		8
#define ihidPrintMerge		9
#define ihidSection		10
#define ihidSort		11
#define ihidFormatTable		12
#define ihidInsertTable		13
#define ihidInsToc		14
#define ihidViewPref		15


extern WParseOpt();
extern WParsePos();
extern WParseDttm();
extern WParseFontName();
extern WParseFontSize();
extern WParseHpsQps();
extern WParseToc();
extern WParseStyleNameSize();


csconst PFN csrgpfnWParse [] =
{
	WParseOpt,
			WParsePos,
			WParseDttm,
			WParseFontName,
			WParseFontSize,
			WParseHpsQps,
			WParseToc,
			WParseStyleNameSize
};


#define ipfnMax (sizeof (csrgpfnWParse) / sizeof (PFN))


#define ipfnOpt			0
#define ipfnPos			1
#define ipfnDttm		2
#define ipfnFontName		3
#define ipfnFontSize		4
#define ipfnHpsQps		5
#define ipfnToc			6
#define ipfnStyleNameSize	7


csconst ELPID csrgelpid [] =
{
	/* IDDAbsPos */
	ihidAbsPos, ipfnPos, iEntblPosH, tmcDxaHorz, 
			ihidAbsPos, ipfnPos, iEntblPosV, tmcDyaVert, 
			ihidAbsPos, ipfnOpt, optPosUnit, tmcPosFromText, 
			ihidAbsPos, ipfnOpt, optAutoPosUnit, tmcPosWidth, 

			/* IDDAutosave */
	ihidAutosave, ipfnOpt, optPosNZInt, tmcASDMinute, 

			/* IDDCatSearch */
	ihidCatSearch, ipfnDttm, 0, tmcCatSrhCreateFrom, 
			ihidCatSearch, ipfnDttm, 0, tmcCatSrhCreateTo,
			ihidCatSearch, ipfnDttm, 0, tmcCatSrhReviseFrom,
			ihidCatSearch, ipfnDttm, 0, tmcCatSrhReviseTo,

			/* IDDCharacter */
	ihidCharacter, ipfnFontName, 0, tmcCharName,
			ihidCharacter, ipfnFontSize, 0, tmcCharSize,
			ihidCharacter, ipfnHpsQps, 0, tmcCharHpsPos,
			ihidCharacter, ipfnHpsQps, 0, tmcCharQpsSpacing,

			/* IDDDocument */
	ihidDocument, ipfnOpt, optPosUnit, tmcDocPageWidth,
			ihidDocument, ipfnOpt, optPosUnit, tmcDocPageHeight,
			ihidDocument, ipfnOpt, optPosUnit, tmcDocDefTabs,
			ihidDocument, ipfnOpt, optAnyUnit, tmcDocTMargin,
			ihidDocument, ipfnOpt, optAnyUnit, tmcDocBMargin,
			ihidDocument, ipfnOpt, optPosUnit, tmcDocLMargin,
			ihidDocument, ipfnOpt, optPosUnit, tmcDocRMargin,
			ihidDocument, ipfnOpt, optPosUnit, tmcDocGutter,
			ihidDocument, ipfnOpt, optPosNZInt, tmcDocFNStartAt,

			/* IDDHeader */
	ihidHeader, ipfnOpt, optAutoPosInt, tmcHdrPgStartAt,
			ihidHeader, ipfnOpt, optPosUnit, tmcHdrFromTop,
			ihidHeader, ipfnOpt, optPosUnit, tmcHdrFromBottom,

			/* IDDParaLooks */
	ihidParaLooks, ipfnOpt, optAnyUnit, tmcParLeftIn,
			ihidParaLooks, ipfnOpt, optAnyUnit, tmcParRightIn,
			ihidParaLooks, ipfnOpt, optAnyUnit, tmcParFirstIn,
			ihidParaLooks, ipfnOpt, optPosLineUnit, tmcParSpaceBf,
			ihidParaLooks, ipfnOpt, optPosLineUnit, tmcParSpaceAf,
			ihidParaLooks, ipfnOpt, optAutoLineUnit, tmcParIntLSp,

			/* IDDFormatPic */
	ihidFormatPic, ipfnOpt, optPosNZInt, tmcScaleMy,
			ihidFormatPic, ipfnOpt, optPosNZInt, tmcScaleMx,
			ihidFormatPic, ipfnOpt, optAnyUnit, tmcCropTop,
			ihidFormatPic, ipfnOpt, optAnyUnit, tmcCropLeft,
			ihidFormatPic, ipfnOpt, optAnyUnit, tmcCropBottom,
			ihidFormatPic, ipfnOpt, optAnyUnit, tmcCropRight,

			/* IDDPrint */
	ihidPrint, ipfnOpt, optPosNZInt, tmcPrCopies,

			/* IDDPrintMerge */
	ihidPrintMerge, ipfnOpt, optPosNZInt, tmcPMFrom,
			ihidPrintMerge, ipfnOpt, optPosNZInt, tmcPMTo,

			/* IDDSection */
	ihidSection, ipfnOpt, optPosNZInt, tmcSecColNum,
			ihidSection, ipfnOpt, optPosUnit,tmcSecColSpacing,
			ihidSection, ipfnOpt, optPosNZInt, tmcSecLNStartAt,
			ihidSection, ipfnOpt, optAutoPosUnit, tmcSecLNFrom,
			ihidSection, ipfnOpt, optPosNZInt, tmcSecLNCountBy,

			/* IDDSort */
	ihidSort, ipfnOpt, optPosNZInt, tmcSortFieldNum,

			/* IDDFormatTable */
	ihidFormatTable, ipfnOpt, optPosInt, tmcFTElFromCol,
			ihidFormatTable, ipfnOpt, optPosInt, tmcFTElCol,
			ihidFormatTable, ipfnOpt, optAutoPosUnit, tmcColumnWidth,
			ihidFormatTable, ipfnOpt, optPosUnit, tmcGapSpace,
			ihidFormatTable, ipfnOpt, optAnyUnit, tmcIndent,
			ihidFormatTable, ipfnOpt, optAutoAnyUnit, tmcRowHeight,

			/* IDDInsertTable */
	ihidInsertTable, ipfnOpt, optPosNZInt, tmcColumns,
			ihidInsertTable, ipfnOpt, optPosNZInt, tmcRows,
			ihidInsertTable, ipfnOpt, optAutoAnyUnit, tmcInitColWidth, 

			/* IDDInsToc */
	ihidInsToc, ipfnToc, 0, tmcTocFrom,
			ihidInsToc, ipfnToc, 0, tmcTocTo,

			/* IDDViewPref */
	ihidViewPref, ipfnStyleNameSize, 0, tmcVPNmAreaSize,
};



#define ielpidMax (sizeof (csrgelpid) / sizeof (PFN))


#define ValGetIag(hcab, iag) \
	(*(((int *) *(hcab)) + cwCabMin + (iag)))
#define LValGetIag(hcab, iag) \
	(* ((long *) (((int *) *(hcab)) + cwCabMin + (iag))))


/* %%Function:IelpidTarget %%Owner:bradch */
IelpidTarget(hid, iag)
int hid, iag;
{
	int ielpidTarget, ieldi, celfd, iagSimple, ielfd;
	ELFD * pelfd;
	ELDI ** heldi;

	if (hid == IDDSearchChar || hid == IDDReplaceChar)
		hid = IDDCharacter;
	else  if (hid == IDDSearchPara || hid == IDDReplacePara)
		hid = IDDParaLooks;

	ieldi = IeldiFromHid(hid);
	if ((heldi = HAllocateCb(CbGetInfoIeldi(ieldi, 
			(ELDI huge *) NULL))) == hNil)
		{
		RtError(rerrOutOfMemory);
		}
	CbGetInfoIeldi(ieldi, (ELDI huge *) *heldi);

	ielpidTarget = 0;
	celfd = (*heldi)->celfd;
	pelfd = (*heldi)->rgelfd;
	iagSimple = ((*heldi)->cabi >> 8) & 0xff;
	for (ielfd = 0; ielfd < celfd; ielfd++, pelfd++)
		{
		if (pelfd->elv == elvSd && pelfd->iag >= iagSimple)
			{
			if (pelfd->iag == iag)
				break;
			ielpidTarget += 1;
			}
		}

	FreeH(heldi);

	return ielpidTarget;
}


/* %%Function:PfnTmcOptFromHidIag %%Owner:bradch */
PFN PfnTmcOptFromHidIag(hid, iag, ptmc, popt, pfPt)
int hid;
int iag;
int * ptmc;
int * popt;
BOOL * pfPt;
{
	int ihid, ielpid, ipfn, opt;

	if (hid == IDDSearchChar || hid == IDDReplaceChar)
		hid = IDDCharacter;
	else  if (hid == IDDSearchPara || hid == IDDReplacePara)
		hid = IDDParaLooks;

	for (ihid = 0; ihid < ihidMax && csrghid[ihid] != hid; ihid += 1)
		;
	if (ihid == ihidMax)
		return NULL;

	for (ielpid = 0; ielpid < ielpidMax; ielpid += 1)
		{
		if (csrgelpid[ielpid].ihid == ihid)
			break;
		}

	ielpid += IelpidTarget(hid, iag);
	if (ielpid >= ielpidMax)
		return NULL;

	*ptmc = csrgelpid[ielpid].tmc;
	*popt = opt = csrgelpid[ielpid].opt;
	ipfn = csrgelpid[ielpid].ipfnWParse;
	*pfPt = (ipfn == ipfnOpt) && FUnitOpt(opt);
	return csrgpfnWParse[ipfn];
}



/* NOTE: This assumes the correct CM has already been fetched! */
/* %%Function:RecordParsedItem %%Owner:bradch */
RecordParsedItem(hid, hcab, iag, ielpidTarget)
int hid;
HCAB hcab;
int iag;
int ielpidTarget;
{
	int ihid, ielpid, ipfn;
	WORD w;
	LONG l;
	void * pv;
	char szBuf [256];

	if (hid == IDDSearchChar || hid == IDDReplaceChar)
		hid = IDDCharacter;
	else  if (hid == IDDSearchPara || hid == IDDReplacePara)
		hid = IDDParaLooks;

	for (ihid = 0; ihid < ihidMax && csrghid[ihid] != hid; ihid += 1)
		;
	Assert(ihid != ihidMax);

	for (ielpid = 0; ielpid < ielpidMax; ielpid += 1)
		{
		if (csrgelpid[ielpid].ihid == ihid)
			break;
		}
	ielpid += ielpidTarget;

	Assert(ielpid < ielpidMax);
	Assert(csrgelpid[ielpid].ihid == ihid);

	ipfn = csrgelpid[ielpid].ipfnWParse;
	Assert(ipfn < ipfnMax);

	if (ipfn == ipfnDttm) /* the only parsed long we use... */
		{
		l = LValGetIag(hcab, iag);
		pv = &l;
		}
	else
		{
		w = ValGetIag(hcab, iag);
		pv= &w;
		}

	/* Call the parse function... */
#ifdef BRADCH
	if (ipfn == ipfnOpt && FUnitOpt(csrgelpid[ielpid].opt))
		{
		RecordInt(w / 20);
		}
	else
#endif
			{
			(*(csrgpfnWParse[ipfn]))
					(tmmFormat, szBuf, &pv, 0, csrgelpid[ielpid].tmc, 
					csrgelpid[ielpid].opt);
			SzToStInPlace(szBuf);
			RecordQuotedSt(szBuf);
			}
}
