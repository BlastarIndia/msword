/******************************************************************************
**
**     COPYRIGHT (C) 1987 MICROSOFT
**
*******************************************************************************
**
** Module: dlbenum.c --- Drop-Down List Box enumeration function
**
** Functions included:
**
** REVISIONS
**
** Date         Who Rel Ver     Remarks
** 4/17/87      yxy             Original
** 5/28/87	bac		Put the strings in CS too
**
*****************************************************************************/

#define NOMINMAX
#define NOBRUSH
#define NOICON
#define NOPEN
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOSYSMETRICS
#define NOCLIPBOARD
#define NOKEYSTATE
#define NORASTEROPS
#define NOSHOWINDOW
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOWH
#define NOWNDCLASS
#define NOREGION
#define NOSOUND
#define NOCOMM

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "debug.h"

#include "dlbenum.h"


csconst ENTBL rgEntbl[] = {
	{ 
	cFNPos,
		/* Must correspond to ordering of fpc in DOP */
		{
		StKey("End of Section",NoteEndSec),
				StKey("Bottom of Page",NoteBottom),
				StKey("Beneath Text",NoteBeneath),
				StKey("End of Document",NoteEndDoc)
		}
	 },
	{ 
			cSkt,
			/* WARNING: these must agree with
				the skt's in sort.h in
		number and order; update cSkt
		in sort.h and rgstSkt in sort.c
		if you add more */
		{
		StKey("Alphanumeric",SortAlphanumeric),
				StKey("Numeric",SortNumeric),
				StKey("Date",SortDate)
		}
	 },
	{ 
			cDffSave,
			/* Must correspond to dffSave order in filecvt.h */
		{
		StKey("Normal",SaveNormal),
				StKey("Document Template",SaveTemplate),
				StKey("Text Only",SaveText),
				StKey("Text+breaks",SaveTextBreaks),
				StKey("Text Only (PC-8)",SaveText8),
				StKey("Text+breaks (PC-8)",SaveText8Breaks),
				StKey("RTF",RTF)
		}
	 },
	{ /* page number format */
	/* If you change this ordering, also change
		mpnfcipg and mpipgnfc in hddwin.c */
	cNfc,
				{
		StShared("1 2 3..."),
				StShared("a b c..."),
				StShared("A B C..."),
				StShared("i ii iii..."),
				StShared("I II III...")
		}
	 },
	{ /* Catalog Sort Factor */
	cCatSort,
				{
		StKey("Name",SortName),
				StKey("Author",SortAuthor),
				StKey("Creation Date",SortCreateDate),
				StKey("Last Saved Date",SortSaveDate),
				StKey("Last Saved by",SortSaveBy),
				StKey("Size",SortSize)
		}
	 },
	{ /* print source */
	cPrSrc,
			/* must correspond to ordering in print.h */
		{
		StKey("Document",PrintDoc),
				StKey("Summary Info",PrintSummary),
				StKey("Annotations",PrintAnnotations),
				StKey("Styles",PrintStyles),
				StKey("Glossary",PrintGlsy),
				StKey("Key Assignments",PrintKeys)
		}
	 },
	{ /* print feed */
	/* must correspond to iidBin ordering in print.h */
	cPrFeed,
				{
		StKey("Manual",PrintBinManual),
				StKey("Envelope",PrintBinEnvelope),
				StKey("Tractor",PrintBinTractor),
				StKey("Auto",PrintBinAuto),
				StKey("Bin 1",PrintBin1),
				StKey("Bin 2",PrintBin2),
				StKey("Bin 3",PrintBin3),
				StKey("Mixed",PrintMixed)
		}
	 },
	{ /* FileOpen converter names */
	/* order must correspond to dffOpen in filecvt.h */
	cDffOpen,
				{
		StSharedKey("Text",OpenText),
				StSharedKey("Text (PC-8)",OpenText8),
				StShared("RTF"),
				StShared("BIFF (Excel 2.X)"),
				StShared("Multiplan 3.0"),
				StShared("WKS")
		}
	 },
	{ 
			cBrclTable,
			/* For table borders (all except outline) */
		{
		StKey("None",BrclNone),
				StKey("Single",BrclSingle),
				StKey("Thick",BrclThick),
				StKey("Double",BrclDouble),
		}
	 },

};


/*  %%Function:  CopyEntblToSz  %%Owner:  bobz       */

CopyEntblToSz(iEntbl, isz, sz)
int     iEntbl;
int     isz;
CHAR    *sz;
{
	CHAR FAR *lpch;

	Assert(isz < rgEntbl[iEntbl].iMax && iEntbl < iEntblMax);

	bltbx((CHAR FAR *) (rgEntbl[iEntbl].rgst[isz]), (CHAR FAR *) sz, 
			rgEntbl[iEntbl].rgst[isz][0]+1);
	StToSzInPlace(sz);
}


/*  %%Function:  FEnumIEntbl  %%Owner:  bobz       */

BOOL FEnumIEntbl(iEntbl, isz, sz)
int     iEntbl;
int     isz;
CHAR    *sz;
{
	Assert(iEntbl < iEntblMax);

	if (isz < rgEntbl[iEntbl].iMax)
		{
		CopyEntblToSz(iEntbl, isz, sz);
		return (fTrue);
		}
	else
		{
		return (fFalse);
		}

}


#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Dlbenum_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* Dlbenum_Last() */
