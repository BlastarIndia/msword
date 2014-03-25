#define NOGDICAPMASKS
#define NONCMESSAGES
#define NODRAWFRAME
#define NORASTEROPS
#define NOCTLMGR
#define NOMINMAX
#define NOMSG
#define NORECT
#define NOSCROLL
#define NOKEYSTATE
#define NOCREATESTRUCT
#define NOICON
#define NOPEN
#define NOREGION
#define NODRAWTEXT
#define NOMB 
#define NOWINOFFSETS
/* #define NOMETAFILE    */
#define NOCLIPBOARD
#define NOSOUND
#define NOCOMM
#define NOKANJI
/* #define NOGDI */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "debug.h"
#include "doc.h"
#include "props.h"
#include "prm.h"
#include "border.h"
#include "disp.h"
#include "pic.h"
#include "file.h"
#include "fontwin.h"
#include "ch.h"
#include "inter.h"
#include "filecvt.h"

#define REVMARKING
#include "compare.h"

#define RTFDEFS
#include "rtf.h"
#define RTFTABLES 
#define RTFSUBS
#include "rtftbl.h"
#include "table.h"

#include "doslib.h"

csconst char stCsDOC[] = St(".DOC");
csconst char stCsRT1[] = St(".RT1");
csconst char stCsRT2[] = St(".RT2");
csconst char stCsRTF[] = St(".RTF");


/* These next few routine require rgszRtfSym to be in this module */

#ifdef DEBUG
/* P C H   S Z	 R T F	 M O V E */
/*  %%Function:  C_PchSzRtfMove  %%Owner:  bobz       */

HANDNATIVE char *C_PchSzRtfMove(iszRtf, pch)
int iszRtf;
char *pch;
{
	char far *qch = rgszRtfSym[iszRtf];
	int ch;

	while (ch = *qch++)
		*pch++ = ch;

	return pch;
}


#endif /* DEBUG */


#ifdef DEBUG
/* F  S E A R C H  R G R S Y M
*  Binary-search string table returning true if the string is found.
*  In any case return the index of where the string would be in rgbst at *pirsym.
	Note the assumption that rgrsym and rgszRtfSym are in 1 to 1
	correspondence so searching rgszRtfSym is equivalent to searching rgsym.
*/

/*  %%Function:  C_FSearchRgrsym  %%Owner:  bobz       */

HANDNATIVE int C_FSearchRgrsym(sz, pirsym)
char *sz;
int *pirsym;
{
	int irsymMin = 0;
	int irsym = irsymMin;
	int irsymLim = iszRTFMax;
	int wCompGuess;
	CHAR FAR *qch;
	CHAR *pch;

	while (irsymMin < irsymLim)
		{
		irsym = (irsymMin + irsymLim) >> 1;

		/* open code WCompSzQsz */
		qch = rgszRtfSym[irsym];
		pch = sz;
		while (*pch == *qch)
			{
			if (*pch == 0)
				{
				*pirsym = irsym;
				return fTrue;	    /* found: return index */
				}
			pch++;
			qch++;
			}

		if ((wCompGuess = (*pch - *qch)) < 0)
			irsymLim = irsym;
		else
			irsymMin = ++irsym;
		}
	*pirsym = irsym;
	return fFalse;		    /* not found: return insert point */
}


#endif /* DEBUG */

/* rgrrbWord is a csconst struct table defined in this module.
	This routine is used by both rtfin and rtfout. */

/*  %%Function:  GetRrb  %%Owner:  bobz       */

GetRrb(irrbProp, prrb)
int irrbProp;
struct RRB *prrb;
	/* rgrrbWord is a csconst struct table defined in this module */
{
	*prrb = rgrrbWord[irrbProp];
}


/* called by both rtfin and rtfout. Here so the one doesn't have to
	haul in the other
*/
/*  %%Function:  RtfStandardChp  %%Owner:  bobz       */

RtfStandardChp(pchp)
struct CHP *pchp;
{
	extern int ftcMapDef;

	StandardChp(pchp);
	pchp->hps = fsDefRTF;  /* rtf default different than opus default */
	/* calc'ed at end of font table. It is the
		mapping of the incoming default font to our font */
	pchp->ftc = ftcMapDef;
}


/* ********************** DEBUG ROUTINES ************************** */


#ifdef DEBUG
/* P E R F O R M  R T F  T E S T  S U I T E */
/*  %%Function:  PerformRtfTestSuite   %%Owner:  bobz       */

PerformRtfTestSuite (chTest, stFile)
CHAR chTest, *stFile;

{
	switch (chTest)
		{
	case 'd':
		/* convert foo.doc into foo.rt1 and foo.rt2 */
		WriteRt1Rt2FromDoc (stFile);
		break;
	case 'r':
		/* convert foo.rtf into foo.doc */
		WriteDocFromRtf (stFile);
		break;
	case 'c':
		/* convert foo.doc into foo.rtf */
		WriteRtfFromDoc (stFile);
		break;
	default:
		Assert (fFalse);
		}

	OurExitWindows();   /* does not return */
}


/*  %%Function:  WriteRt1Rt2FromDoc   %%Owner:  bobz       */

WriteRt1Rt2FromDoc (stFile)
CHAR *stFile;

{
	int docDOC, docRT1;
	CHAR stDOC [ichMaxFile];
	CHAR stRT1 [ichMaxFile];
	CHAR stRT2 [ichMaxFile];
	struct FNS fns;

	/*  get full, normalized filename */
	Assert (FNormalizeStFile (stFile, stDOC, nfoNormal));
	StFileToPfns (stDOC, &fns);

	/*  build native filename */
	CopyCsSt (stCsDOC, fns.stExtension);
	PfnsToStFile (&fns, stDOC);

	/*  build first result filename */
	CopyCsSt (stCsRT1, fns.stExtension);
	PfnsToStFile (&fns, stRT1);

	/*  build final result filename */
	CopyCsSt (stCsRT2, fns.stExtension);
	PfnsToStFile (&fns, stRT2);

	/* no need to save/restore as we will soon exitwindows */
	vdbs.fNoInfoRTF = fTrue;

	CommSz (SzShared("RTF Test Suite underway\n\r"));
	CommSzSt (SzShared ("Native file: "), stDOC);
	CommSzSt (SzShared ("First RTF result: "), stRT1);
	CommSzSt (SzShared ("Final RTF result: "), stRT2);

	CommSz (SzShared ("opening native\n\r"));
	Assert ((docDOC = DocOpenStDof (stDOC, dofBackground, NULL)) != docNil);

	CommSz (SzShared ("saving first result\n\r"));
	FFlushDoc (docDOC, stRT1, dffSaveRTF, fTrue /* fReport */);

	/* get rid of the opened file */
	DisposeDoc (docDOC);
	MarkAllReferencedFn();
	DeleteNonReferencedFns(0);

	CommSz (SzShared ("opening first result\n\r"));
	Assert ((docRT1 = DocOpenStDof (stRT1, dofBackground, NULL)) != docNil);

	CommSz (SzShared ("saving final result\n\r"));
	FFlushDoc (docRT1, stRT2, dffSaveRTF, fTrue /* fReport */);

	CommSz (SzShared ("test complete, exiting\n\r"));
}


/*  %%Function:  WriteDocFromRtf   %%Owner:  bobz       */

WriteDocFromRtf (stFile)
CHAR *stFile;

{
	int docRTF;
	CHAR stRTF [ichMaxFile];
	CHAR stDOC [ichMaxFile];
	struct FNS fns;

	/*  get full, normalized filename */
	Assert (FNormalizeStFile (stFile, stRTF, nfoNormal));
	StFileToPfns (stRTF, &fns);

	/*  build original filename */
	CopyCsSt (stCsRTF, fns.stExtension);
	PfnsToStFile (&fns, stRTF);

	/*  build native filename */
	CopyCsSt (stCsDOC, fns.stExtension);
	PfnsToStFile (&fns, stDOC);

	CommSz (SzShared("RTF Converter\n\r"));
	CommSzSt (SzShared ("Original file: "), stRTF);
	CommSzSt (SzShared ("Native file: "), stDOC);

	CommSz (SzShared ("opening original\n\r"));
	Assert ((docRTF = DocOpenStDof (stRTF, dofBackground, NULL)) != docNil);

	CommSz (SzShared ("saving native\n\r"));
	FFlushDoc (docRTF, stDOC, dffSaveNative, fTrue /* fReport */);
}


/*  %%Function:  WriteRtfFromDoc   %%Owner:  bobz       */

WriteRtfFromDoc (stFile)
CHAR *stFile;

{
	int docDOC;
	CHAR stRTF [ichMaxFile];
	CHAR stDOC [ichMaxFile];
	struct FNS fns;

	/*  get full, normalized filename */
	Assert (FNormalizeStFile (stFile, stRTF, nfoNormal));
	StFileToPfns (stRTF, &fns);

	/*  build original filename */
	CopyCsSt (stCsDOC, fns.stExtension);
	PfnsToStFile (&fns, stDOC);

	/*  build rtf filename */
	CopyCsSt (stCsRTF, fns.stExtension);
	PfnsToStFile (&fns, stRTF);

	CommSz (SzShared("DOC Converter\n\r"));
	CommSzSt (SzShared ("Original file: "), stDOC);
	CommSzSt (SzShared ("Native file: "), stRTF);

	CommSz (SzShared ("opening original\n\r"));
	Assert ((docDOC = DocOpenStDof (stDOC, dofBackground, NULL)) != docNil);

	CommSz (SzShared ("saving rtf\n\r"));
	FFlushDoc (docDOC, stRTF, dffSaveRTF, fTrue /* fReport */);
}


#endif /* DEBUG */


#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Rtfsubs_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* Rtfsubs_Last() */
