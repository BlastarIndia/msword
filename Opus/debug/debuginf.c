/* D E B U G C M D . C */
#ifdef DEBUG

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "props.h"
#include "format.h"
#include "doc.h"
#include "sel.h"
#include "file.h"
#include "disp.h"
#include "prm.h"
#include "fkp.h"
#include "screen.h"
#include "scc.h"
#include "error.h"
#include "doslib.h"
#include "debug.h"
#include "message.h"

#include "opuscmd.h"


#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"
#include "print.h"

#include "dbgscrbl.hs"
#include "dbgscrbl.sdm"
#include "dbgtests.hs"
#include "dbgtests.sdm"
#include "dbgfunc.hs"
#include "dbgfunc.sdm"
#include "dbgpref.hs"
#include "dbgpref.sdm"
#include "dbgrare.hs"
#include "dbgrare.sdm"
#include "dbgfail.hs"
#include "dbgfail.sdm"
#include "dbgmem.hs"
#include "dbgmem.sdm"
#include "dbgusec1.hs"
#include "dbgusec1.sdm"
#include "dbgusec2.hs"
#include "dbgusec2.sdm"
#include "dbgusec3.hs"
#include "dbgusec3.sdm"

/* G L O B A L S */

extern struct BPTB    vbptbExt;
extern struct SEL     selCur;
extern struct CA      caPara;
extern struct PAP     vpapFetch;
extern struct CHP     vchpFetch;
extern int            vdocFetch;
extern struct FCB     **mpfnhfcb[];
extern struct ESPRM   dnsprm[];
extern struct PAP     vpapStc;
extern struct CHP     vchpStc;
extern struct FKPD    vfkpdPap, vfkpdChp;
extern CP             vcpFetch, ccpPap, ccpChp;
extern int            ichInsert;
extern struct FTI     vfti;
extern struct FTI     vftiDxt;
extern struct FLI     vfli;
extern struct PRI     vpri;
extern struct SCI     vsci;
extern char (**vhgrpchr)[];

#define cbMinFree 0xFFFF   /* lots of free bytes: unsigned long */

csconst char rgszSprmNames[][] =
{
	"sprmNoop",
	"sprmTleReplace",
	"sprmPStc",
	"sprmPStcPermute",
	"sprmPIncLvl",
	"sprmPJc",
	"sprmPFSideBySide",
	"sprmPFKeep",
	"sprmPFKeepFollow",
	"sprmPFPageBreakBefore",
	"sprmPBrcl",
	"sprmPBrcp",
	"sprmPNfcSeqNumb",
	"sprmPNoSeqNumb",
	"sprmPFNoLineNumb",
	"sprmPChgTabsPapx",
	"sprmPDxaRight",
	"sprmPDxaLeft",
	"sprmPNest",
	"sprmPDxaLeft1",
	"sprmPDyaLine",
	"sprmPDyaBefore",
	"sprmPDyaAfter",
	"sprmPChgTabs",
	"sprmPFInTable",
	"sprmPFTtp",
	"sprmPDxaAbs",
	"sprmPDyaAbs",
	"sprmPDxaWidth",
	"sprmPPc",
	"sprmPBrcTop",
	"sprmPBrcLeft",
	"sprmPBrcBottom",
	"sprmPBrcRight",
	"sprmPBrcBetween",
	"sprmPBrcBar",
	"sprmPFromText",
	"sprmPSpare2",
	"sprmPSpare3",
	"sprmPSpare4",
	"sprmPSpare5",
	"sprmPSpare6",
	"sprmPSpare7",
	"sprmPSpare8",
	"sprmPSpare9",
	"sprmPSpare10",
	"sprmPSpare11",
	"sprmPSpare12",
	"sprmPSpare13",
	"sprmPSpare14",
	"sprmPSpare15",
	"sprmPSpare16",
	"sprmPRuler",
	"sprmCFStrikeRM",
	"sprmCFRMark",
	"sprmCFFldVanish",
	"sprmCSpare0",
	"sprmCDefault",
	"sprmCPlain",
	"sprmCSpare00",
	"sprmCFBold",
	"sprmCFItalic",
	"sprmCFStrike",
	"sprmCFOutline",
	"sprmCFShadow",
	"sprmCFSmallCaps",
	"sprmCFCaps",
	"sprmCFVanish",
	"sprmCFtc",
	"sprmCKul",
	"sprmCSizePos",
	"sprmCQpsSpace",
	"sprmCSpare000",
	"sprmCIco",
	"sprmCHps",
	"sprmCHpsInc",
	"sprmCHpsPos",
	"sprmCHpsPosAdj",
	"sprmCMajority",
	"sprmCSpare6",
	"sprmCSpare7",
	"sprmCSpare8",
	"sprmCSpare9",
	"sprmCSpare10",
	"sprmCSpare11",
	"sprmCSpare12",
	"sprmCSpare13",
	"sprmCSpare14",
	"sprmCSpare15",
	"sprmCSpare16",
	"sprmCSpare17",
	"sprmCSpare18",
	"sprmCSpare19",
	"sprmCSpare20",
	"sprmPicBrcl",
	"sprmPicScale",
	"sprmPicSpare0",
	"sprmPicSpare1",
	"sprmPicSpare2",
	"sprmPicSpare3",
	"sprmPicSpare4",
	"sprmPicSpare5",
	"sprmPicSpare6",
	"sprmPicSpare7",
	"sprmPicSpare8",
	"sprmPicSpare9",
	"sprmPicSpare10",
	"sprmPicSpare11",
	"sprmPicSpare12",
	"sprmPicSpare13",
	"sprmPicSpare14",
	"sprmPicSpare15",
	"sprmPicSpare16",
	"sprmPicSpare17",
	"sprmPicSpare18",
	"sprmPicSpare19",
	"sprmPicSpare20",
	"sprmSBkc",
	"sprmSFTitlePage",
	"sprmSCcolumns",
	"sprmSDxaColumns",
	"sprmSFAutoPgn",
	"sprmSNfcPgn",
	"sprmSDyaPgn",
	"sprmSDxaPgn",
	"sprmSFPgnRestart",
	"sprmSFEndnote",
	"sprmSLnc",
	"sprmSGrpfIhdt",
	"sprmSNLnnMod",
	"sprmSDxaLnn",
	"sprmSDyaHdrTop",
	"sprmSDyaHdrBottom",
	"sprmSLBetween",
	"sprmSVjc",
	"sprmSLnnMin",
	"sprmSPgnStart",
	"sprmSSpare2",
	"sprmSSpare3",
	"sprmSSpare4",
	"sprmSSpare5",
	"sprmSSpare6",
	"sprmSSpare7",
	"sprmSSpare8",
	"sprmSSpare9",
	"sprmSSpare10",
	"sprmTJc",
	"sprmTDxaLeft",
	"sprmTDxaGapHalf",
	"sprmTSpare6",
	"sprmTSpare7",
	"sprmTSpare8",
	"sprmTDefTable",
	"sprmTDyaRowHeight",
	"sprmTSpare2",
	"sprmTSpare3",
	"sprmTSpare4",
	"sprmTSpare5",
	"sprmTInsert",
	"sprmTDelete",
	"sprmTDxaCol",
	"sprmTMerge",
	"sprmTSplit",
	"sprmTSetBrc"
};	


/*  S P R M   T O   P P C H  */
void SprmToPpch(sprm, ppch)
int sprm;
char **ppch;
{
	char far *qch = rgszSprmNames[sprm];
	int ch;

	if (sprm < 164)	 /* number of strings in rgszSprmNames */
	{
			while (ch = *qch++)
			{
					**ppch = ch;
					(*ppch)++;
			}
	}
	**ppch = '\0';
}


/*  P C H   T O   P P C H */
/* Appends one string to another */

void PchToPpch (pchFrom, ppchTo)
char *pchFrom, **ppchTo;
{
		while (*pchFrom != '\0')
			{  
				**ppchTo = *pchFrom;
				(*ppchTo)++;
				pchFrom++;
		}
		(**ppchTo) = '\0';
}


/*  W   T O   H E X   D I G I T  */
char WToHexDigit(w)			/* w must be less than 16, greater than 0 */
int w;
{
		char ch;
			
		if (w<10)	ch = '0' + w;
		else ch = 'A' + w - 10;
		
		return ch;
}


/*  P B Y T E S   T O   H E X   P P C H  */
void PBytesToHexPpch (pBytes, ppch, cBytes) 
char *pBytes;
char **ppch;
int cBytes;
{
		int wT, i;
			
		PchToPpch(SzShared("0x"), ppch);
		for (i = cBytes - 1; i >= 0; i--)
		{
				wT = pBytes[i];		/* assumes 8086 byte order */
				**ppch = WToHexDigit(wT/16);
				(*ppch)++;
				**ppch = WToHexDigit(wT%16);
				(*ppch)++;
			}	
		**ppch = '\0';
}


/*  C O N C A T   N A M E   A N D   E Q U A L S  */
void ConcatNameAndEquals (sz, ppch)
char *sz, **ppch;
{
		PchToPpch (sz, ppch);
		**ppch = '=';
		(*ppch)++;
		**ppch = '\0';
}


/*  A P P E N D   A N D   C H E C K   F O R   E O L N  */
void AppendAndCheckForEoln (ppch, rgch, pchNew)
char **ppch, *rgch, *pchNew;
{							 
		int cch = 0;
		char *pchT;

		pchT = pchNew;
		while (*pchT != '\0')
		{
				cch++;
				pchT++;
		}
		if (((*ppch) - rgch + cch) <= 80)
				PchToPpch(pchNew, ppch);
		else 
		{
				PchToPpch(SzShared("\r\n"), ppch);
				**ppch = '\0';
				CommSz(rgch);
				(*ppch) = rgch;
				PchToPpch(pchNew, ppch);
		}
}


/*  A P P E N D   C O M M A   I F   N E E D E D  */
void AppendCommaIfNeeded (pbAnyProps, ppch, rgch)
int *pbAnyProps;
char **ppch, *rgch;
{
		char rgchT[3];
		char *pchT;

		if ((*pbAnyProps) && ((*ppch) != rgch) && ((*ppch) - rgch < 80))
		{
				pchT = rgchT;
				*pchT++ = ',';
				if ((*ppch) - rgch < 79)
					*pchT++ = ' ';
				*pchT = '\0';
				AppendAndCheckForEoln (ppch, rgch, rgchT);
		}
}


/*  O U T P U T   U N S   I F   D I F F  */
void OutputUnsIfDiff (sz, u, uS, ppch, rgch, pbAnyProps)
char *sz;
uns  u, uS;
char **ppch;
char *rgch;
int *pbAnyProps;
{
		char rgchT[30];
		char *pchT;

		if (u != uS)
		{
				AppendCommaIfNeeded (pbAnyProps, ppch, rgch);
				pchT = rgchT;
				ConcatNameAndEquals (sz, &pchT);
				CchUnsToPpch (u, &pchT);
				*pchT = '\0';
				AppendAndCheckForEoln(ppch, rgch, rgchT);    
				*pbAnyProps = 1;
		}
}
												

/*  O U T P U T   W   I F   D I F F  */
void OutputWIfDiff (sz, w, wS, ppch, rgch, pbAnyProps)
char *sz;
int w, wS;
char **ppch;
char *rgch;
int *pbAnyProps;
{
		char rgchT[30];
		char *pchT;

		if (w != wS)
		{
				AppendCommaIfNeeded (pbAnyProps, ppch, rgch);
				pchT = rgchT;
				ConcatNameAndEquals (sz, &pchT);
				CchIntToPpch (w, &pchT);
				*pchT = '\0';
				AppendAndCheckForEoln (ppch, rgch, rgchT);                
				*pbAnyProps = 1;
		}
}


/*  O U T P U T   L O N G   I F   D I F F  */
void OutputLongIfDiff (sz, l, lS, ppch, rgch, pbAnyProps)
char *sz;
long l, lS;
char **ppch;
char *rgch;
int *pbAnyProps;
{
		char rgchT[30];
		char *pchT;

		if (l != lS)
		{
				AppendCommaIfNeeded (pbAnyProps, ppch, rgch);
				pchT = rgchT;
				ConcatNameAndEquals (sz, &pchT);
				PBytesToHexPpch (&l, &pchT, 4);
				AppendAndCheckForEoln (ppch, rgch, rgchT);                
				*pbAnyProps = 1;
		}
}


/*  O U T P U T   C H   I F   D I F F  */
void OutputChIfDiff (sz, ch, chS, ppch, rgch, pbAnyProps)
char *sz, ch, chS, **ppch, *rgch;
int *pbAnyProps;
{
		char rgchT[30];
		char *pchT;

		if (ch != chS)
		{
				AppendCommaIfNeeded (pbAnyProps, ppch, rgch);
				pchT = rgchT;
				ConcatNameAndEquals (sz, &pchT);
				*pchT++ = ch;
				*pchT = '\0';
				AppendAndCheckForEoln (ppch, rgch, rgchT);                
				*pbAnyProps = 1;
		}
}


/*  O U T P U T   I F   O N E  */
void OutputIfOne (sz, w, ppch, rgch, pbAnyProps)
char *sz;
int w;
char **ppch, *rgch;
int *pbAnyProps;
{
		char rgchT[30];
		char *pchT;

		if (w)
		{
				AppendCommaIfNeeded (pbAnyProps, ppch, rgch);
				pchT = rgchT;
				PchToPpch(sz, &pchT);
				AppendAndCheckForEoln (ppch, rgch, rgchT);                
				*pbAnyProps = 1;
		}
}
		

/*  O U T P U T   R G W  */
void OutputRgwDiffs (sz, i, rgw, rgwS, ppch, rgch, pbAnyProps)
char *sz;
int i;
int *rgw, *rgwS;
char **ppch;
char *rgch;
int *pbAnyProps;
{	
		int iT, bAnyRgwProps;
		char rgchT[50];
		char *pchT;

		bAnyRgwProps = 0;
		for (iT=0; iT<i; iT++) 
		{
				if (rgw[iT] != rgwS[iT])
				{
						if (!bAnyRgwProps)
						{
								AppendCommaIfNeeded (pbAnyProps, ppch, rgch);
								pchT = rgchT;
								PchToPpch(sz, &pchT);
								PchToPpch(SzShared(":  "), &pchT);
								AppendAndCheckForEoln(ppch, rgch, rgchT);
						}
						AppendCommaIfNeeded (&bAnyRgwProps, ppch, rgch);
						pchT = rgchT;
						PchToPpch(SzShared("["), &pchT);
						CchIntToPpch(iT, &pchT);
						*pchT++ = ']';
						*pchT++ = '=';
						CchIntToPpch(rgw[iT], &pchT);
						*pchT = '\0';
						AppendAndCheckForEoln(ppch, rgch, rgchT);
						bAnyRgwProps = 1;
						*pbAnyProps = 1;
				}
		} 
}


/*  O U T P U T   R G C H  */
void OutputRgchDiffs (sz, i, rgch, rgchS, ppch, rgchBuffer, pbAnyProps)
char *sz;
int i;
char *rgch, *rgchS, **ppch, *rgchBuffer;
int *pbAnyProps;
{
		int iT, bAnyRgchProps;
			char rgchT[50];
		char *pchT;
			
		bAnyRgchProps = 0;
		for (iT=0; iT<i; iT++) 
		{
				if (rgch[iT] != rgchS[iT])
				{
						if (!bAnyRgchProps)
						{ 
								AppendCommaIfNeeded (pbAnyProps, ppch, rgch);
								pchT = rgchT;
								PchToPpch(sz, &pchT);
								PchToPpch(SzShared(":  "), &pchT);
								AppendAndCheckForEoln(ppch, rgchBuffer, rgchT);
						}
						AppendCommaIfNeeded (&bAnyRgchProps, ppch, rgch);
						pchT = rgchT;
						PchToPpch (SzShared("["), &pchT);
						CchIntToPpch(iT, &pchT);
						*pchT++ = ']';
						*pchT++ = '=';
						CchIntToPpch(rgch[iT], &pchT);
						*pchT = '\0';
						AppendAndCheckForEoln(ppch, rgchBuffer, rgchT);
						bAnyRgchProps = 1;
						*pbAnyProps = 1;
				}
		}
}


/*  O U T P U T   R G T C   W   I F   D I F F  */
void OutputRgtcWIfDiff (sz, szw, w, wS, ppch, rgch, pbAnyRgtcProps, pbAnyThisElem, pbAnyProps, i)
char *sz, *szw;
int w, wS;
char **ppch, *rgch;
int *pbAnyRgtcProps, *pbAnyThisElem, *pbAnyProps, i;
{
		char rgchT[50];
		char *pchT;

		if (w != wS)
		{
				if (!(*pbAnyRgtcProps))
				{
						AppendCommaIfNeeded (pbAnyProps, ppch, rgch);
						pchT = rgchT;
						PchToPpch(sz, &pchT);
						PchToPpch(SzShared(":  "), &pchT);
						AppendAndCheckForEoln(ppch, rgch, rgchT);
				}
				if (!(*pbAnyThisElem))
				{ 
						pchT = rgchT;
						PchToPpch (SzShared(" ["), &pchT);
						CchIntToPpch(i, &pchT);
						*pchT++ = ']';
						*pchT++ = '=';
						*pchT = '\0';
						AppendAndCheckForEoln(ppch, rgch, rgchT);
				}         
				AppendCommaIfNeeded (pbAnyThisElem, ppch, rgch);
				pchT = rgchT;
				ConcatNameAndEquals (szw, &pchT);
				CchIntToPpch (w, &pchT);
				*pchT = '\0';
				AppendAndCheckForEoln (ppch, rgch, rgchT);
				*pbAnyRgtcProps = 1;
				*pbAnyThisElem = 1;
				*pbAnyProps = 1;
		}
}


/*  O U T P U T   R G T C   R G W   D I F F S  */
void OutputRgtcRgwDiffs (szrgtc, szrgw, irgw, rg, rgS, ppch, rgch, pbAnyRgtcProps, pbAnyThisElem, pbAnyProps, irgtc)
char *szrgtc, *szrgw;
int irgw, *rg, *rgS;
char **ppch, *rgch;
int *pbAnyRgtcProps, *pbAnyThisElem, *pbAnyProps, irgtc;
{
		int iT, bAnyRgwProps;
		char rgchT[50];
		char *pchT;

		bAnyRgwProps = 0;
		for (iT=0; iT<irgw; iT++)
		{
				if (rg[iT] != rgS[iT])
				{
						if (!(*pbAnyRgtcProps))
						{
								AppendCommaIfNeeded (pbAnyProps, ppch, rgch);
								pchT = rgchT;
								PchToPpch(szrgtc, &pchT);
								PchToPpch(SzShared(":  "), &pchT);
								AppendAndCheckForEoln(ppch, rgch, rgchT);
						}
						if (!(*pbAnyThisElem))
						{         
								pchT = rgchT;
								PchToPpch (SzShared(" ["), &pchT);
								CchIntToPpch(irgtc, &pchT);
								*pchT++ = ']';
								*pchT++ = ' ';
								*pchT++ = '=';
								*pchT++ = ' ';
								*pchT = '\0';
								AppendAndCheckForEoln(ppch, rgch, rgchT);
						}
						if (!bAnyRgwProps)
						{
								AppendCommaIfNeeded (pbAnyThisElem, ppch, rgch);
								pchT = rgchT;
								PchToPpch(szrgw, &pchT);
								PchToPpch(SzShared(":  "), &pchT);
								AppendAndCheckForEoln(ppch, rgch, rgchT);
						}
						pchT = rgchT;
						PchToPpch(SzShared(" ["), &pchT);
						CchIntToPpch(iT, &pchT);
						*pchT++ = ']';
						*pchT++ = ' ';
						*pchT++ = '=';
						*pchT++ = ' ';
						*pchT = '\0';
						AppendAndCheckForEoln(ppch, rgch, rgchT);
						pchT = rgchT;
						CchIntToPpch(rg[iT], &pchT);
						*pchT = '\0';
						AppendAndCheckForEoln(ppch, rgch, rgchT);
						bAnyRgwProps = 1;
						*pbAnyRgtcProps = 1;
						*pbAnyThisElem = 1;
						*pbAnyProps = 1;
				}         
		} 
}
						


/*  O U T P U T   R G T C   D I F F S  */
void OutputRgtcDiffs (sz, i, rgtc, rgtcS, ppch, rgch, pbAnyProps)
char *sz;
int i;
struct TC *rgtc, *rgtcS;
char **ppch;
char *rgch;
int *pbAnyProps;
{	
		int iT, bAnyRgtcProps, bAnyThisElem;
		
		bAnyRgtcProps = 0;
		for (iT=0; iT<i; iT++, rgtc++, rgtcS++) 
		{
				bAnyThisElem = 0;
				OutputRgtcWIfDiff(sz,SzShared("grpf"), rgtc->grpf, rgtcS->grpf, ppch, rgch, &bAnyRgtcProps, &bAnyThisElem, pbAnyProps, iT);
				OutputRgtcWIfDiff(sz,SzShared("fFirstMerged"), rgtc->fFirstMerged, rgtcS->fFirstMerged, ppch, rgch, &bAnyRgtcProps, &bAnyThisElem, 
				pbAnyProps, iT);    
				OutputRgtcWIfDiff(sz,SzShared("fMerged"), rgtc->fMerged, rgtcS->fMerged, ppch, rgch, &bAnyRgtcProps, &bAnyThisElem, pbAnyProps, iT);
				OutputRgtcWIfDiff(sz,SzShared("fUnused"), rgtc->fUnused, rgtcS->fUnused, ppch, rgch, &bAnyRgtcProps, &bAnyThisElem, pbAnyProps, iT);
				OutputRgtcRgwDiffs(sz,SzShared("rgbrc"), 4, rgtc->rgbrc, rgtcS->rgbrc, ppch, rgch, &bAnyRgtcProps, &bAnyThisElem, pbAnyProps, iT);
				OutputRgtcWIfDiff(sz,SzShared("brcTop"), rgtc->brcTop, rgtcS->brcTop, ppch, rgch, &bAnyRgtcProps, &bAnyThisElem, pbAnyProps, iT);
				OutputRgtcWIfDiff(sz,SzShared("brcLeft"), rgtc->brcLeft, rgtcS->brcLeft, ppch, rgch, &bAnyRgtcProps, &bAnyThisElem, pbAnyProps, iT);
				OutputRgtcWIfDiff(sz,SzShared("brcBottom"), rgtc->brcBottom, rgtcS->brcBottom, ppch, rgch, &bAnyRgtcProps, &bAnyThisElem, pbAnyProps,iT);
				OutputRgtcWIfDiff(sz,SzShared("brcRight"), rgtc->brcRight, rgtcS->brcRight, ppch, rgch, &bAnyRgtcProps, &bAnyThisElem, pbAnyProps, iT);
			}
}


/*  O U T P U T   P A R A G R A P H   P R O P S  */
void OutputParagraphProps (ppap, ppapS, ppch, rgch)
struct PAP *ppap, *ppapS;
char **ppch, *rgch;
{
			int bAnyProps = 0;

/* UC */OutputWIfDiff(SzShared("jc"), ppap->jc, ppapS->jc, ppch, rgch, &bAnyProps);
/* UC */OutputWIfDiff(SzShared("fSideBySide"), ppap->fSideBySide, ppapS->fSideBySide, ppch, rgch, &bAnyProps);
/* UC */OutputWIfDiff(SzShared("fKeep"), ppap->fKeep, ppapS->fKeep, ppch, rgch, &bAnyProps);
/* UC */OutputWIfDiff(SzShared("fKeepFollow"), ppap->fKeepFollow, ppapS->fKeepFollow, ppch, rgch, &bAnyProps);
/* UC */OutputWIfDiff(SzShared("fPageBreakBefore"), ppap->fPageBreakBefore, ppapS->fPageBreakBefore, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("fBrLnAbove"), ppap->fBrLnAbove, ppapS->fBrLnAbove, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("fBrLnBelow"), ppap->fBrLnBelow, ppapS->fBrLnBelow, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("fUnused"), ppap->fUnused, ppapS->fUnused, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("pc"), ppap->pc, ppapS->pc, ppch, rgch, &bAnyProps);
/* UC */OutputWIfDiff(SzShared("brcp"), ppap->brcp, ppapS->brcp, ppch, rgch, &bAnyProps);
/* UC */OutputWIfDiff(SzShared("pcVert"), ppap->pcVert, ppapS->pcVert, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("pcHorz"), ppap->pcHorz, ppapS->pcHorz, ppch, rgch, &bAnyProps);
/* UC */OutputWIfDiff(SzShared("brcl"), ppap->brcl, ppapS->brcl, ppch, rgch, &bAnyProps);
/* UC */OutputWIfDiff(SzShared("nfcSeqNumb"), ppap->nfcSeqNumb, ppapS->nfcSeqNumb, ppch, rgch, &bAnyProps);
/* UC */OutputWIfDiff(SzShared("nnSeqNumb"), ppap->nnSeqNumb, ppapS->nnSeqNumb, ppch, rgch, &bAnyProps);
/* UC */OutputWIfDiff(SzShared("fNoLnn"), ppap->fNoLnn, ppapS->fNoLnn, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("dxaRight"), ppap->dxaRight, ppapS->dxaRight, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("dxaLeft"), ppap->dxaLeft, ppapS->dxaLeft, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("dxaLeft1"), ppap->dxaLeft1, ppapS->dxaLeft1, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("dyaLine"), ppap->dyaLine, ppapS->dyaLine, ppch, rgch, &bAnyProps);
		OutputUnsIfDiff(SzShared("dyaBefore"), ppap->dyaBefore, ppapS->dyaBefore, ppch, rgch, &bAnyProps);
		OutputUnsIfDiff(SzShared("dyaAfter"), ppap->dyaAfter, ppapS->dyaAfter, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("phe.fSpare"), ppap->phe.fSpare, ppapS->phe.fSpare, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("phe.fUnk"), ppap->phe.fUnk, ppapS->phe.fUnk, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("phe.fDiffLines"), ppap->phe.fDiffLines, ppapS->phe.fDiffLines, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("phe.clMac"), ppap->phe.clMac, ppapS->phe.clMac, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("phe.w0"), ppap->phe.w0, ppapS->phe.w0, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("phe.dxaCol"), ppap->phe.dxaCol, ppapS->phe.dxaCol, ppch, rgch, &bAnyProps);
		OutputUnsIfDiff(SzShared("phe.dylLine"), ppap->phe.dylLine, ppapS->phe.dylLine, ppch, rgch, &bAnyProps);
		OutputUnsIfDiff(SzShared("phe.dyaLine"), ppap->phe.dyaLine, ppapS->phe.dyaLine, ppch, rgch, &bAnyProps);
		OutputUnsIfDiff(SzShared("phe.dylHeight"), ppap->phe.dylHeight, ppapS->phe.dylHeight, ppch, rgch, &bAnyProps);
		OutputUnsIfDiff(SzShared("phe.dyaHeight"), ppap->phe.dyaHeight, ppapS->phe.dyaHeight, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("phe.fStyleDirty"), ppap->phe.fStyleDirty, ppapS->phe.fStyleDirty, ppch, rgch, &bAnyProps);
		OutputChIfDiff(SzShared("fInTable"), ppap->fInTable, ppapS->fInTable, ppch, rgch, &bAnyProps);
		OutputChIfDiff(SzShared("fTtp"), ppap->fTtp, ppapS->fTtp, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("ptap->jc"), ppap->ptap->jc, ppapS->ptap->jc, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("ptap->dxaGapHalf"), ppap->ptap->dxaGapHalf, ppapS->ptap->dxaGapHalf, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("ptap->dyaRowHeight"), ppap->ptap->dyaRowHeight, ppapS->ptap->dyaRowHeight, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("ptap->grpfTap"), ppap->ptap->grpfTap, ppapS->ptap->grpfTap, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("ptap->fCaFull"), ppap->ptap->fCaFull, ppapS->ptap->fCaFull, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("ptap->fFirstRow"), ppap->ptap->fFirstRow, ppapS->ptap->fFirstRow, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("ptap->fLastRow"), ppap->ptap->fLastRow, ppapS->ptap->fLastRow, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("ptap->fOutline"), ppap->ptap->fOutline, ppapS->ptap->fOutline, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("ptap->itcMac"), ppap->ptap->itcMac, ppapS->ptap->itcMac, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("ptap->dxaAdjust"), ppap->ptap->dxaAdjust, ppapS->ptap->dxaAdjust, ppch, rgch, &bAnyProps);
		OutputRgwDiffs(SzShared("ptap->rgdxaCenter"), (itcMax + 1), ppap->ptap->rgdxaCenter, ppapS->ptap->rgdxaCenter, ppch, rgch, &bAnyProps);
		OutputRgtcDiffs(SzShared("ptap->rgtc"), itcMax, ppap->ptap->rgtc, ppapS->ptap->rgtc, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("dxaAbs"), ppap->dxaAbs, ppapS->dxaAbs, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("dyaAbs"), ppap->dyaAbs, ppapS->dyaAbs, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("dxaWidth"), ppap->dxaWidth, ppapS->dxaWidth, ppch, rgch, &bAnyProps);
		OutputRgwDiffs(SzShared("rgbrc"), ibrcPapLim, ppap->rgbrc, ppapS->rgbrc, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("brcTop"), ppap->brcTop, ppapS->brcTop, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("brcLeft"), ppap->brcLeft, ppapS->brcLeft, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("brcBottom"), ppap->brcBottom, ppapS->brcBottom, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("brcRight"), ppap->brcRight, ppapS->brcRight, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("brcBetween"), ppap->brcBetween, ppapS->brcBetween, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("brcBar"), ppap->brcBar, ppapS->brcBar, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("dxaFromText"), ppap->dxaFromText, ppapS->dxaFromText, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("itbdMac"), ppap->itbdMac, ppapS->itbdMac, ppch, rgch, &bAnyProps);
		OutputRgwDiffs(SzShared("rgdxaTab"), ppap->itbdMac, ppap->rgdxaTab, ppap->rgdxaTab, ppch, rgch, &bAnyProps);/*uncond. output up to itbdMac*/
		OutputRgchDiffs(SzShared("rgtbd"), ppap->itbdMac, ppap->rgtbd, ppap->rgtbd, ppch, rgch, &bAnyProps);			/*unconditional output up to itbdMac*/
		CommSz(rgch);
		CommSz(SzShared("\r\n"));
}

/*  O U T P U T   C H A R A C T E R   P R O P S  */
void OutputCharacterProps (pchp, pchpS, ppch, rgch)
struct CHP *pchp, *pchpS;
char **ppch, *rgch;
{
		int bAnyProps = 0;

		OutputIfOne(SzShared("fBold"), pchp->fBold, ppch, rgch, &bAnyProps);
		OutputIfOne(SzShared("fItalic"), pchp->fItalic, ppch, rgch, &bAnyProps);
		OutputIfOne(SzShared("fStrike"), pchp->fStrike, ppch, rgch, &bAnyProps);
		OutputIfOne(SzShared("fOutline"), pchp->fOutline, ppch, rgch, &bAnyProps);
		OutputIfOne(SzShared("fldVanish"), pchp->fFldVanish, ppch, rgch, &bAnyProps);
		OutputIfOne(SzShared("fSmallCaps"), pchp->fSmallCaps, ppch, rgch, &bAnyProps);
		OutputIfOne(SzShared("fCaps"), pchp->fCaps, ppch, rgch, &bAnyProps);
		OutputIfOne(SzShared("fVanish"), pchp->fVanish, ppch, rgch, &bAnyProps);
		OutputIfOne(SzShared("fRMark"), pchp->fRMark, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("fSpec"), pchp->fSpec, pchpS->fSpec, ppch, rgch, &bAnyProps);
		OutputUnsIfDiff(SzShared("ftc"), pchp->ftc, pchpS->ftc, ppch, rgch, &bAnyProps);
/* UC */OutputWIfDiff(SzShared("hps"), pchp->hps, pchpS->hps, ppch, rgch, &bAnyProps);
/* UC */OutputWIfDiff(SzShared("hpsPos"), pchp->hpsPos, pchpS->hpsPos, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("qpsSpace"), pchp->qpsSpace, pchpS->qpsSpace, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("wSpare2"), pchp->wSpare2, pchpS->wSpare2, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("ico"), pchp->ico, pchpS->ico, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("kul"), pchp->kul, pchpS->kul, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("fSysVanish"), pchp->fSysVanish, pchpS->fSysVanish, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("dummy1"), pchp->dummy1, pchpS->dummy1, ppch, rgch, &bAnyProps);
/* UC */OutputWIfDiff(SzShared("dummy2"), pchp->dummy2, pchpS->dummy2, ppch, rgch, &bAnyProps);
/* UC */OutputWIfDiff(SzShared("fnPic"), pchp->fnPic, pchpS->fnPic, ppch, rgch, &bAnyProps);
		OutputLongIfDiff(SzShared("fcPic"), pchp->fcPic, pchpS->fcPic, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("docPic"), pchp->docPic, pchpS->docPic, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("dummy"), pchp->dummy, pchpS->dummy, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("fDirty"), pchp->fDirty, pchpS->fDirty, ppch, rgch, &bAnyProps);
		OutputWIfDiff(SzShared("hpsLarge"), pchp->hpsLarge, pchpS->hpsLarge, ppch, rgch, &bAnyProps);
		CommSz(rgch);  
		CommSz(SzShared("\r\n"));
}

/*	 O U T P U T   C H P X   I N F O  */
void OutputChpXInfo (chpX, fcMin, fcMac, ppch, rgch)
struct CHP chpX;
FC fcMin, fcMac;
char **ppch;
char *rgch;
{
		char rgchT[50];
		char *pchT;
		int bAnyProps = 0;

		PchToPpch (SzShared("fkp fc limits:  "), ppch);
		PBytesToHexPpch(&fcMin, ppch, 4);
		PchToPpch (SzShared(", "), ppch);
		PBytesToHexPpch(&fcMac, ppch, 4);
		PchToPpch (SzShared("  "), ppch);

		OutputIfOne (SzShared("fBold"), chpX.fBold, ppch, rgch, &bAnyProps);
		OutputIfOne (SzShared("fItalic"), chpX.fItalic, ppch, rgch, &bAnyProps);
		OutputIfOne (SzShared("fStrike"), chpX.fStrike, ppch, rgch, &bAnyProps);
		OutputIfOne (SzShared("fOutline"), chpX.fOutline, ppch, rgch, &bAnyProps);
		OutputIfOne (SzShared("fFldVanish"), chpX.fFldVanish, ppch, rgch, &bAnyProps);
		OutputIfOne (SzShared("fSmallCaps"), chpX.fSmallCaps, ppch, rgch, &bAnyProps);
		OutputIfOne (SzShared("fCaps"), chpX.fCaps, ppch, rgch, &bAnyProps);
		OutputIfOne (SzShared("fVanish"), chpX.fVanish, ppch, rgch, &bAnyProps);
		OutputIfOne (SzShared("fRMark"), chpX.fRMark, ppch, rgch, &bAnyProps);
		if (chpX.fsFtc) 
		{ 
				AppendCommaIfNeeded (&bAnyProps, ppch, rgch);
				pchT = rgchT;
				ConcatNameAndEquals (SzShared("ftc"), &pchT);
				CchUnsToPpch (chpX.ftc, &pchT);
				*pchT = '\0';
				AppendAndCheckForEoln (ppch, rgch, rgchT);                
				bAnyProps = 1;
		}
		if (chpX.fsHps) 
		{ 
				AppendCommaIfNeeded (&bAnyProps, ppch, rgch);
				pchT = rgchT;
				ConcatNameAndEquals (SzShared("hps"), &pchT);
				/*UC*/ CchIntToPpch(chpX.hps, &pchT);
				*pchT = '\0';
				AppendAndCheckForEoln (ppch, rgch, rgchT);
				bAnyProps = 1;
		}
		if (chpX.fsKul) 
		{
				AppendCommaIfNeeded (&bAnyProps, ppch, rgch);
				pchT = rgchT;
				ConcatNameAndEquals (SzShared("kul"), &pchT);
				CchIntToPpch(chpX.kul, &pchT);
				*pchT = '\0';
				AppendAndCheckForEoln (ppch, rgch, rgchT);
				bAnyProps = 1;
		}
		if (chpX.fsPos)
		{
				AppendCommaIfNeeded (&bAnyProps, ppch, rgch);
				pchT = rgchT;
				ConcatNameAndEquals (SzShared("hpsPos"), &pchT);
				/*UC*/ CchIntToPpch(chpX.hpsPos, &pchT);
				*pchT = '\0';
				AppendAndCheckForEoln (ppch, rgch, rgchT);
				bAnyProps = 1;
		}
		if (chpX.fsSpace)
		{
				AppendCommaIfNeeded (&bAnyProps, ppch, rgch);
				pchT = rgchT;
				ConcatNameAndEquals (SzShared("qpsSpace"), &pchT);
				CchIntToPpch(chpX.qpsSpace, &pchT);
				*pchT = '\0';
				AppendAndCheckForEoln (ppch, rgch, rgchT);
				bAnyProps = 1;
		}
		if (chpX.fsIco)
		{
				AppendCommaIfNeeded (&bAnyProps, ppch, rgch);
				pchT = rgchT;
				ConcatNameAndEquals (SzShared("ico"), &pchT);
				CchIntToPpch(chpX.ico, &pchT);
				*pchT = '\0';
				AppendAndCheckForEoln (ppch, rgch, rgchT);
				bAnyProps = 1;
		}
		if (chpX.fSpec)
		{
				AppendCommaIfNeeded (&bAnyProps, ppch, rgch);
				pchT = rgchT;
				ConcatNameAndEquals (SzShared("fcPic"), &pchT);
				PBytesToHexPpch(&chpX.fcPic, &pchT, 4);
				AppendAndCheckForEoln (ppch, rgch, rgchT);
				bAnyProps = 1;
		}
		CommSz(rgch);
		CommSz(SzShared("\r\n"));
}

/*  O U T P U T   G R P P R L   I N F O  */
void OutputGrpprlInfo (sgc, cch, hpprl, ppch, rgch)
int sgc, cch;
CHAR HUGE *hpprl;
char **ppch, *rgch;
{
	int val;
	char rgchT[50];
	char *pchT;
	int i;

	while (cch > 0)
	{
		int cchSprm, dsprm;
		struct ESPRM esprm;
		int sprm = *hpprl;

		if (sprm == 0)
		{
			cchSprm = 1;
			goto LNext;
		}
		esprm = dnsprm[sprm];
		val = *(hpprl + 1);
		if ((cchSprm = esprm.cch) == 0)
		{
			if (sprm == sprmTDefTable)
				bltbh(hpprl + 1, &cchSprm, sizeof(int));
			else
			{
				cchSprm = val;
				if (cchSprm == 255 && sprm == sprmPChgTabs)
				{ 
					char HUGE *hpprlT;
					cchSprm = (*(hpprlT = hpprl + 2) * 4) + 1;
					cchSprm += (*(hpprlT + cchSprm) * 3) + 1;
				}
			}
			cchSprm += 2;
		}             

LNext:
		if (esprm.sgc == sgc)
		{
			pchT = rgchT;
			SprmToPpch(sprm, &pchT);
			*pchT++ = '(';
			hpprl++;
			for (i=1; i<cchSprm; i++, hpprl++)
			{	
				int w;
				PBytesToHexPpch((char *)hpprl, &pchT, 1);
				if (cchSprm - i - 1)
					*pchT++ = ' ';
				else
					PchToPpch(SzShared(") "), &pchT);
				*pchT = '\0';
				AppendAndCheckForEoln (ppch, rgch, rgchT);
				pchT = rgchT;
			}
		}
		else
			hpprl += cchSprm;
		cch -= cchSprm;
	}
	**ppch = '\0';
	CommSz(rgch);
	CommSz(SzShared("\r\n"));
}


/*  F I N D   G R P P R L  */
void FindGrpprl (prm, pcch, ppprl, grpprl)
struct PRM prm;
int *pcch;
CHAR **ppprl;
CHAR grpprl[2];
{
		if (prm.fComplex)
		{
				struct PRC *pprc;

				pprc = *HprcFromPprmComplex(&prm);
				*pcch = pprc->bprlMac;
				*ppprl = pprc->grpprl;
		}
		else
		{
				*pcch = 1;
				grpprl[0] = prm.sprm;
				grpprl[1] = prm.val;
				*ppprl = grpprl;
		}
}
			
/*  O U T P U T   S E L   F L A G S  */
void OutputSelFlags (ppch, rgch)
char **ppch, *rgch;
{
		int bAnyProps = 0;

		PchToPpch (SzShared("selCur flags:  "), ppch);
		OutputIfOne(SzShared("fRightward"), selCur.fRightward, ppch, rgch, &bAnyProps);
		OutputIfOne(SzShared("fSelAtPara"), selCur.fSelAtPara, ppch, rgch, &bAnyProps);
		OutputIfOne(SzShared("fWithinCell"), selCur.fWithinCell, ppch, rgch, &bAnyProps);
		OutputIfOne(SzShared("fTableAnchor"), selCur.fTableAnchor, ppch, rgch, &bAnyProps);
		OutputIfOne(SzShared("fColumn"), selCur.fColumn, ppch, rgch, &bAnyProps);
		OutputIfOne(SzShared("fTable"), selCur.fTable, ppch, rgch, &bAnyProps);
		OutputIfOne(SzShared("fGraphics"), selCur.fGraphics, ppch, rgch, &bAnyProps);
		OutputIfOne(SzShared("fBlock"), selCur.fBlock, ppch, rgch, &bAnyProps);
		OutputIfOne(SzShared("fNil"), selCur.fNil, ppch, rgch, &bAnyProps);
		OutputIfOne(SzShared("fIns"), selCur.fIns, ppch, rgch, &bAnyProps);
		*ppch = '\0';
		CommSz(rgch);
		CommSz(SzShared("\r\n"));
}

/*  C M D   S E L E C T I O N   I N F O  */
CMD CmdSelectionInfo (pcmb)
CMB *pcmb;
{ 
		struct PCD **hplcpcd, pcdchp, pcdpap;
		struct CHP chp, chpS, chpX, *pchp, vchpFetchT;
		struct PAP pap, papS, papX, *ppap;
		struct FCB **hfcbpap, **hfcbchp;
		struct FKP HUGE *hpfkp;
		struct PLC **hplcbtePap, **hplcbteChp;
		struct PHE phe;
		struct PRM prmpap, prmchp;
		struct DOD *pdod;
		FC fcFirst, fcLim, fcMin, fcMac, fcpap, fcchp;
		CP cpFirstPap, cpLastPap, cpFirstChp, cpLastChp;
		char rgch[90], *pch, grpprl [512];
		int pn, ifcT, bpapx, bchpx, fn, cbPhe, ifc, i;
		
		CommSz(SzShared("\r\n"));
		CommSz(SzShared("S E L E C T I O N   I N F O R M A T I O N"));
		CommSz(SzShared("\r\n"));
		pch = rgch;
		PchToPpch (SzShared("selCur is:  "), &pch);
		ConcatNameAndEquals (SzShared("doc"), &pch);
		CchIntToPpch(selCur.doc, &pch);
		ConcatNameAndEquals (SzShared(", cpFirst"), &pch);
		PBytesToHexPpch(&selCur.cpFirst, &pch, 4);
		ConcatNameAndEquals (SzShared(", cpLim"), &pch);
		PBytesToHexPpch(&selCur.cpLim, &pch, 4);
		ConcatNameAndEquals (SzShared(", sk"), &pch);
		CchIntToPpch(selCur.sk, &pch);
		PchToPpch (SzShared("\r\n"), &pch);
		*pch = 0;
		CommSz(rgch);
		pch = rgch;
		OutputSelFlags (&pch, rgch);
		
		pdod = PdodDoc(selCur.doc);
		hplcpcd = pdod->hplcpcd;
		pch = rgch;
		PchToPpch (SzShared("pap pcd:  limits = "), &pch);
		cpFirstPap = CpPlc (hplcpcd, IInPlc (hplcpcd, caPara.cpLim - 1));
		PBytesToHexPpch(&cpFirstPap, &pch, 4);
		PchToPpch (SzShared(", "), &pch);
		cpLastPap = CpPlc (hplcpcd, IInPlc (hplcpcd, caPara.cpLim - 1) + 1);
		PBytesToHexPpch(&cpLastPap, &pch, 4);
		GetPlc (hplcpcd, IInPlc (hplcpcd, caPara.cpLim - 1), &pcdpap);
		ConcatNameAndEquals (SzShared("   fn"), &pch);
		CchIntToPpch (pcdpap.fn, &pch);
		ConcatNameAndEquals (SzShared("  fc"), &pch);
		fcpap = pcdpap.fc + caPara.cpLim - 1 - cpFirstPap;
		PBytesToHexPpch(&fcpap, &pch, 4);
		PchToPpch(SzShared("\r\n"), &pch);
		*pch = 0;
		CommSz(rgch);
		pch = rgch;
		PchToPpch (SzShared("chp pcd:  limits = "), &pch);
		cpFirstChp = CpPlc (hplcpcd, IInPlc (hplcpcd, selCur.cpFirst));
		PBytesToHexPpch(&cpFirstChp, &pch, 4);
		PchToPpch (SzShared(", "), &pch);
		cpLastChp = CpPlc (hplcpcd, IInPlc (hplcpcd, selCur.cpFirst) + 1);
		PBytesToHexPpch(&cpLastChp, &pch, 4);
		GetPlc (hplcpcd, IInPlc (hplcpcd, selCur.cpFirst), &pcdchp);
		ConcatNameAndEquals (SzShared("   fn"), &pch);
		CchIntToPpch (pcdchp.fn, &pch);
		ConcatNameAndEquals (SzShared("  fc"), &pch);
		fcchp = pcdchp.fc + selCur.cpFirst - cpFirstChp;
		PBytesToHexPpch(&fcchp, &pch, 4);
		PchToPpch(SzShared("\r\n"), &pch);
		*pch = 0;
		CommSz(rgch);

/* Output properties obtained from style info: */
		
		CachePara (selCur.doc, selCur.cpFirst); 
		pch = rgch;
		ConcatNameAndEquals (SzShared("paragraph properties from stc"), &pch);
		CchIntToPpch(vpapFetch.stc, &pch);
		PchToPpch (SzShared(":  "), &pch);
		*pch = '\0';
		MapStc(pdod, 0, &chpS, &papS); 
		MapStc(pdod, vpapFetch.stc, &chp, &pap); 
		OutputParagraphProps (&pap, &papS, &pch, rgch);
		
		pch = rgch;
		ConcatNameAndEquals (SzShared("character properties from stc"), &pch);
		CchIntToPpch(vpapFetch.stc, &pch);
		PchToPpch (SzShared(":  "), &pch);
		*pch = '\0';
		OutputCharacterProps (&chp, &chpS, &pch, rgch);

/* Output properties obtained from file info: */

		pch = rgch;
		PchToPpch (SzShared("papX: "), &pch);
		*pch = '\0';
		if (pdod->fFormatted && (pcdpap.fn != fnNil))
		{
				hfcbpap = mpfnhfcb[pcdpap.fn];
				hplcbtePap = (*hfcbpap)->hplcbtePap;
				Assert(IMacPlc(hplcbtePap) > 0);
				Assert(fcpap >= CpPlc(hplcbtePap, 0));
				Assert(fcpap < CpPlc(hplcbtePap, IMacPlc(hplcbtePap)));
				pn = PnFromPlcbteFc (hplcbtePap, fcpap);
				hpfkp = (struct FKP HUGE *) HpchGetPn (pcdpap.fn, pn);
				bpapx = BFromFc (hpfkp, fcpap, &fcFirst, &fcLim, &ifc/* dummy variable */);

/* There is an array of bpapx's that is parallel to the array of fc's */
/* The bpapx's are offsets into the fkp */
				if (bpapx) /* a zero offset indicates no prop info (special case) */
				{
						int val;
						CHAR HUGE *hppapx = ((CHAR HUGE *)hpfkp) + bpapx;
/* Now, with the help of the offset, we have a pointer directly to the papx */
						int cch = *hppapx++;
/* The first byte in the papx is the count of characters (cch) */
						cch <<= 1;
						cbPhe = cbPHE;
/* The papx should have: count of bytes, stc, phe, and then the grpprl */
/* Bits 7 up to the bit before cch contain the grpprl info */
						cch -= cbPhe + 1;
						bltbh(hppapx, &papX, cch);
						PchToPpch (SzShared("fkp fc limits:  "), &pch);
						PBytesToHexPpch(&fcFirst, &pch, 4);
						PchToPpch (SzShared(", "), &pch);
						PBytesToHexPpch(&fcLim, &pch, 4);
						PchToPpch (SzShared("  Phe info:  "), &pch);
						for (i = 1; i <= cbPhe; i++)
						{
								CchIntToPpch((*(hppapx + i)), &pch);
								*pch++ = ' ';
						}
						PchToPpch (SzShared("\r\n"), &pch);
						*pch = '\0';
						CommSz(rgch);
						pch = rgch;
						PchToPpch (SzShared("Grpprl from papX:  "), &pch);
						*pch = '\0';
						OutputGrpprlInfo (sgcPap, cch, hppapx + cbPhe + 1, &pch, rgch);
				}
				else
				{
						PchToPpch(SzShared("\r\n"), &pch);
						*pch = '\0';
						CommSz(rgch);
				}
		}
		else
		{
				PchToPpch(SzShared("\r\n"), &pch);
				*pch = '\0';
				CommSz(rgch);
		}


		pch = rgch;
		PchToPpch (SzShared("chpX:  "), &pch);
		*pch = '\0';

/* Fill vchpFetch with char props; length of run to ccpChp */

		FreezeHp();
		blt(&vchpStc, &chpX, cwCHP);
		if (pcdchp.fn == fnSpec)
		{
				if ((int)fcchp >= cchInsertMax)
				{					
						PchToPpch(SzShared("\r\n"), &pch);
						*pch = '\0';
						CommSz(rgch);
				}
				else
				{
						vchpFetchT = vchpFetch;
						blt (&chpX, &vfkpdChp.chp, cwCHP);
						vchpFetch = vchpFetchT;

/* ichInsert points to first "vanished" character in the rgchInsert array */
						if (ichInsert <= (int)fcchp)
						{
/* in the vanished region */
								chpX.fVanish = fTrue;	  
								chpX.fSysVanish = fTrue;	 
						}
						if (fcchp <= ichInsert)
						{
								fcMin = 0;
								fcMac = ichInsert;
						}
						else
						{
								fcMin = ichInsert;
								fcMac = cchInsertMax;
						}		
					OutputChpXInfo (chpX, fcMin, fcMac, &pch, rgch);
				}
		}
		else 
			if (pcdchp.fn == fnScratch
				&& fcchp >= vfkpdChp.fcFirst)
				{
						vchpFetchT = vchpFetch;
						blt (&chpX, &vfkpdChp.chp, cwCHP);
						vchpFetch = vchpFetchT;
						if (fcchp <= ichInsert)
						{
								fcMin = 0;
								fcMac = ichInsert;
						}
						else
						{
								fcMin = ichInsert;
								fcMac = cchInsertMax;
						}		
						OutputChpXInfo (chpX, fcMin, fcMac, &pch, rgch);
				}  
				else
				{
						if (pdod->fFormatted && (pcdchp.fn != fnNil))
						{
								hfcbchp = mpfnhfcb[pcdchp.fn];
								hplcbteChp = (*hfcbchp)->hplcbteChp;
								Assert(IMacPlc(hplcbteChp) > 0);
								Assert(fcchp >= CpPlc(hplcbteChp, 0));
								Assert(fcchp < CpPlc(hplcbteChp, IMacPlc(hplcbteChp)));
								pn = PnFromPlcbteFc (hplcbteChp, fcchp);
								hpfkp = (struct FKP HUGE *) HpchGetPn (pcdchp.fn, pn);
								bchpx = BFromFc (hpfkp, fcchp, &fcMin, &fcMac, &ifc/* dummy variable */);
								if (bchpx)	 /* if it's 0, there are no properties (special case) */
								{ 
										CHAR HUGE *hpchpx = &((CHAR HUGE *)hpfkp)[bchpx];
										int cch = *hpchpx++; /* This is the first byte in the chpX */
/* It equals the number of characters (bytes) in the overall chpX */
										cch <<= 1;
										SetWords(&chpX, 0, cwCHP);
										bltbh(hpchpx, &chpX, cch);
										OutputChpXInfo (chpX, fcMin, fcMac, &pch, rgch);
								}
								else
								{					
										PchToPpch(SzShared("\r\n"), &pch);
										*pch = '\0';
										CommSz(rgch);
								}
						}
						else
						{
								PchToPpch(SzShared("\r\n"), &pch);
								*pch = '\0';
								CommSz(rgch);
						}
				}
		MeltHp();

/* Output properties obtained from grpprl info: */

		prmpap = pcdpap.prm;
		prmchp = pcdchp.prm;
		{
				int cchpap, cchchp;
				CHAR *pprlpap, *pprlchp;
				CHAR grpprlpap[2], grpprlchp[2];

				pch = rgch;
				PchToPpch (SzShared("Grpprl from pap prm:  "), &pch);
				*pch = '\0';
				FindGrpprl (prmpap, &cchpap, &pprlpap, grpprlpap);
				OutputGrpprlInfo (sgcPap, cchpap, (CHAR HUGE *)pprlpap, &pch, rgch);
				pch = rgch;
				PchToPpch (SzShared("Grpprl from chp prm:  "), &pch);
				*pch = '\0';
				FindGrpprl (prmchp, &cchchp, &pprlchp, grpprlchp);
				OutputGrpprlInfo (sgcChp, cchchp, (CHAR HUGE *)pprlchp, &pch, rgch);
		}	
	return cmdOK;
}


/* I N T   T O   P P C H */

/* Converts an integer to the corresponding four-character string of digits */

void IntToPpch (ppch, w)
char **ppch;
int w;
{ 
		int cFactor;

		for (cFactor = 1000;	cFactor >= 1; cFactor/=10)
		{
				**ppch = '0' + (w/cFactor);
				(*ppch)++;
				w%=cFactor;
		}
}


/* C H   C O N C A T */

/* Concatenates to the string buffer the starting and ending ibp values 
	for a line of output */

void ChConcat (w1, w2, ppch)
int w1, w2;
char **ppch;
{
		IntToPpch(ppch, w1);
		**ppch = '-';
		(*ppch)++;
		IntToPpch(ppch, w2);
		**ppch = ' ';
		(*ppch)++;
}



/* C M D   F I L E  C A C H E   I N F O */

/* Outputs information about the file cache:  1) which files are cached
	in each bp, and 2) whether the page cached in a given bp directly follows 
	(in the file) the page cached in the previous bp.  Fn's are represented as 
	letters of the alphabet; lowercase indicates that condition 2) holds.  
*/

CMD CmdFileCacheInfo (pcmb)
CMB * pcmb;
{
	int ibp;
	int fnCur = -1;
	uns pnCur;
	int ispnCur = 0;
	int ibpMin = 0;
	int ispn = 0;
	char rgch[90];
	char rgchT[90];
	char *pch;
	char *pchT;
	struct BPS HUGE *hpbps;
	uns cqbpspn;
	pch = rgch;
	pchT = rgchT;
	hpbps = vbptbExt.hpmpibpbps;
	cqbpspn = vbptbExt.cqbpspn;
	CommSz(SzShared("\r\n"));
	CommSz(SzShared("F I L E   C A C H E   I N F O R M A T I O N"));
	CommSz(SzShared("\r\n"));
	CommSz(SzShared("\'A\' => fn==1, \'B\' => fn==2, etc.   \'?\' => fn==0."));
	CommSz(SzShared("\r\n"));
	CommSz(SzShared("lowercase => (mpibpBps[ibp].pn == mpibpBps[ibp-1].pn + 1)."));
	CommSz(SzShared("\r\n"));
	CommSz(SzShared("Each line begins with the range of ibp\'s displayed."));
	CommSz(SzShared("\r\n"));
	CommSz(SzShared("Spans are separated by spaces."));
	CommSz(SzShared("\r\n"));
	if (!cqbpspn)
		CommSz(SzShared("The variable vbptbExt.cqbpspn must be nonzero.\r\n"));
	for (ibp = 0; ibp <= vbptbExt.ibpMac ; ibp++, hpbps++)
		{
	  	ispn = ((ibp << 2) / cqbpspn);
		if (ispnCur != ispn || ibp == vbptbExt.ibpMac)
			{
			if (ibp % 62 == 0 || ibp == vbptbExt.ibpMac)
				{
				*pchT = '\0';
				pchT = rgchT;
				ChConcat (ibpMin, (ibp-1), &pch);
				PchToPpch (pchT, &pch);
				CommSz (rgch);
				CommSz (SzShared("\r\n"));
				pchT = rgchT;
				pch = rgch;
				ibpMin = ibp;
				}
			else
				{   
				*pchT = ' ';
				pchT++;
				}
			}
		ispnCur = ispn;
		if (ibp != vbptbExt.ibpMac) 
			{	
			if (hpbps->fn)
				{ 
				if ((hpbps->fn == fnCur) && (hpbps->pn == pnCur + 1))
					{  
					*pchT = 'a' + hpbps->fn - 1;
					pchT++;
					}
				else
					{
					*pchT = 'A' + hpbps->fn - 1;
					pchT++;
					}
				pnCur = hpbps->pn;
				fnCur = hpbps->fn;
				} 
			else
				{
				*pchT = '?';
				pchT++;
				fnCur = -1;
				}
			}
		}
	return cmdOK;
}  /* CmdFileCacheInfo */


/*  O U T P U T   I N T  */
void OutputInt(sz, w, ppch, rgch, pbAnyOutput)
char *sz;
int w;
char **ppch, *rgch;
int *pbAnyOutput;
{
    char rgchT[50];
    char *pchT;

    AppendCommaIfNeeded(pbAnyOutput, ppch, rgch);
    pchT = rgchT;
    ConcatNameAndEquals(sz, &pchT);
    CchIntToPpch(w, &pchT);
    *pchT = '\0';
    AppendAndCheckForEoln(ppch, rgch, rgchT);
    *pbAnyOutput = 1;
}


/*  O U T P U T   F C I D  */
void OutputFcid(sz, pfcid, ppch, rgch)
char *sz;
union FCID *pfcid;
char **ppch, *rgch;
{
    char rgchT[85];
    char *pchT;
    int bAnyFcidOutput = 1;

    pchT = rgchT;
    ConcatNameAndEquals(SzShared("lFcid"), &pchT);
    PBytesToHexPpch(&(pfcid->lFcid), &pchT, 4);
    AppendAndCheckForEoln(ppch, rgch, rgchT);
    OutputInt(SzShared("wProps"), pfcid->wProps, ppch, rgch, &bAnyFcidOutput);
    OutputInt(SzShared("wExtra"), pfcid->wExtra, ppch, rgch, &bAnyFcidOutput);
    OutputInt(SzShared("fBold"), pfcid->fBold, ppch, rgch, &bAnyFcidOutput);
    OutputInt(SzShared("fItalic"), pfcid->fItalic, ppch, rgch, &bAnyFcidOutput);
    OutputInt(SzShared("fStrike"), pfcid->fStrike, ppch, rgch, &bAnyFcidOutput);
    OutputInt(SzShared("kul"), pfcid->kul, ppch, rgch, &bAnyFcidOutput);
    OutputInt(SzShared("prq"), pfcid->prq, ppch, rgch, &bAnyFcidOutput);
    OutputInt(SzShared("hps"), pfcid->hps, ppch, rgch, &bAnyFcidOutput);
    OutputInt(SzShared("ibstFont"), pfcid->ibstFont, ppch, rgch, &bAnyFcidOutput);
}


/*  O U T P U T   F C E  */
void OutputFce(sz, szVfti, pfce, ppch, rgch)
char *sz, *szVfti;
struct FCE *pfce;
char **ppch, *rgch;
{
    char rgchT[85];
    char *pchT;
    int bAnyFceOutput = 0;

	 pchT = rgchT;
    PchToPpch(szVfti, &pchT);
	 *pchT++ = '.';
	 PchToPpch(sz, &pchT);
    PchToPpch(SzShared("->fcidActual:  "), &pchT);
    AppendAndCheckForEoln(ppch, rgch, rgchT);
	 OutputFcid(SzShared(""), &(pfce->fcidActual), ppch, rgch);
    pchT = rgchT;
	 PchToPpch(SzShared("remainder of "), &pchT);
	 PchToPpch(szVfti, &pchT);
	 PchToPpch(SzShared(".pfce:  "), &pchT);
	 AppendAndCheckForEoln(ppch, rgch, rgchT);
	 bAnyFceOutput = 0;			  /* supress next comma */
	 OutputInt(SzShared("dxpOverhang"), pfce->dxpOverhang, ppch, rgch, &bAnyFceOutput);
    OutputInt(SzShared("dypAscent"), pfce->dypAscent, ppch, rgch, &bAnyFceOutput);
    OutputInt(SzShared("dypDescent"), pfce->dypDescent, ppch, rgch, &bAnyFceOutput);
    OutputInt(SzShared("dypXtraAscent"), pfce->dypXtraAscent, ppch, rgch, &bAnyFceOutput);
    OutputInt(SzShared("fPrinter"), pfce->fPrinter, ppch, rgch, &bAnyFceOutput);
    OutputInt(SzShared("fVisiBad"), pfce->fVisiBad, ppch, rgch, &bAnyFceOutput);
    OutputInt(SzShared("fFixedPitch"), pfce->fFixedPitch, ppch, rgch, &bAnyFceOutput);
    OutputInt(SzShared("fPrvw"), pfce->fPrvw, ppch, rgch, &bAnyFceOutput);
    OutputInt(SzShared("fGraphics"), pfce->fGraphics, ppch, rgch, &bAnyFceOutput);
    pchT = rgchT;
    AppendCommaIfNeeded(&bAnyFceOutput, ppch, rgch);
    ConcatNameAndEquals(SzShared("dxpWidth"), &pchT);
    CchUnsToPpch(pfce->dxpWidth, &pchT);
    *pchT = '\0';
	 AppendAndCheckForEoln(ppch, rgch, rgchT);
    pchT = rgchT;
	 AppendCommaIfNeeded(&bAnyFceOutput, ppch, rgch);
	 ConcatNameAndEquals(SzShared("hfont"), &pchT);
	 PBytesToHexPpch(&(pfce->hfont), &pchT, 2);
	 *pchT = '\0';
	 AppendAndCheckForEoln(ppch, rgch, rgchT);
}

/*  O U T P U T   V F T I   I N F O  */
void OutputVftiInfo(sz, pvfti, ppch, rgch)
char *sz;
struct FTI *pvfti;
char **ppch;
char *rgch;
{
   int bAnyOutput = 0;
    
	PchToPpch(sz, ppch);
	PchToPpch(SzShared(" info:  "), ppch);
	OutputFce(SzShared("pfce"), sz, pvfti->pfce, ppch, rgch);
	**ppch = '\0';
	CommSz(rgch);
	CommSz(SzShared("\r\n"));
	*ppch = rgch;
	PchToPpch(sz, ppch);
	PchToPpch(SzShared(".fcid:  "), ppch);
	OutputFcid(SzShared(""), &(pvfti->fcid), ppch, rgch);
	**ppch = '\0';
	CommSz(rgch);
	CommSz(SzShared("\r\n"));
	*ppch = rgch;
	PchToPpch(SzShared("remainder of "), ppch);
	PchToPpch(sz, ppch);
	PchToPpch(SzShared(":  "), ppch);
	bAnyOutput = 0;	 /* supress comma */
	OutputInt(SzShared("dxpOverhang"), pvfti->dxpOverhang, ppch, rgch, &bAnyOutput);
   OutputInt(SzShared("dypAscent"), pvfti->dypAscent, ppch, rgch, &bAnyOutput);
   OutputInt(SzShared("dypDescent"), pvfti->dypDescent, ppch, rgch, &bAnyOutput);
   OutputInt(SzShared("dypXtraAscent"), pvfti->dypXtraAscent, ppch, rgch, &bAnyOutput);
   OutputInt(SzShared("fPrinter"), pvfti->fPrinter, ppch, rgch, &bAnyOutput);
   OutputInt(SzShared("fVisiBad"), pvfti->fVisiBad, ppch, rgch, &bAnyOutput);
   OutputInt(SzShared("fFixedPitch"), pvfti->fFixedPitch, ppch, rgch, &bAnyOutput);
   OutputInt(SzShared("fPrvw"), pvfti->fPrvw, ppch, rgch, &bAnyOutput);
   OutputInt(SzShared("dxpInch"), pvfti->dxpInch, ppch, rgch, &bAnyOutput);
   OutputInt(SzShared("dypInch"), pvfti->dypInch, ppch, rgch, &bAnyOutput);
   OutputInt(SzShared("dxpBorder"), pvfti->dxpBorder, ppch, rgch, &bAnyOutput);
   OutputInt(SzShared("dypBorder"), pvfti->dypBorder, ppch, rgch, &bAnyOutput);
   OutputInt(SzShared("dxpExpanded"), pvfti->dxpExpanded, ppch, rgch, &bAnyOutput);
   OutputInt(SzShared("fTossedPrinterDC"), pvfti->fTossedPrinterDC, ppch, rgch, &bAnyOutput);
	**ppch = '\0';
   CommSz(rgch);
   CommSz(SzShared("\r\n"));
}

/*  O U T P U T   T E X T   M E T R I C S  */
void OutputTextMetrics (ptm, ppch, rgch)
TEXTMETRIC *ptm;
char **ppch, *rgch;
{
	int bAnyTmOutput = 0;

	PchToPpch(SzShared("Text Metrics Info:  "), ppch);
	OutputInt(SzShared("tmHeight"), ptm->tmHeight, ppch, rgch, &bAnyTmOutput);
	OutputInt(SzShared("tmAscent"), ptm->tmAscent, ppch, rgch, &bAnyTmOutput);
	OutputInt(SzShared("tmDescent"), ptm->tmDescent, ppch, rgch, &bAnyTmOutput);
	OutputInt(SzShared("tmInternalLeading"), ptm->tmInternalLeading, ppch, rgch, &bAnyTmOutput);
	OutputInt(SzShared("tmExternalLeading"), ptm->tmExternalLeading, ppch, rgch, &bAnyTmOutput);
	OutputInt(SzShared("tmAveCharWidth"), ptm->tmAveCharWidth, ppch, rgch, &bAnyTmOutput);
	OutputInt(SzShared("tmMaxCharWidth"), ptm->tmMaxCharWidth, ppch, rgch, &bAnyTmOutput);
	OutputInt(SzShared("tmWeight"), ptm->tmWeight, ppch, rgch, &bAnyTmOutput);
	OutputInt(SzShared("tmItalic"), ptm->tmItalic, ppch, rgch, &bAnyTmOutput);
	OutputInt(SzShared("tmUnderlined"), ptm->tmUnderlined, ppch, rgch, &bAnyTmOutput);
	OutputInt(SzShared("tmStruckOut"), ptm->tmStruckOut, ppch, rgch, &bAnyTmOutput);
	OutputInt(SzShared("tmFirstChar"), ptm->tmFirstChar, ppch, rgch, &bAnyTmOutput);
	OutputInt(SzShared("tmLastChar"), ptm->tmLastChar, ppch, rgch, &bAnyTmOutput);
	OutputInt(SzShared("tmDefaultChar"), ptm->tmDefaultChar, ppch, rgch, &bAnyTmOutput);
	OutputInt(SzShared("tmBreakChar"), ptm->tmBreakChar, ppch, rgch, &bAnyTmOutput);
	OutputInt(SzShared("tmPitchAndFamily"), ptm->tmPitchAndFamily, ppch, rgch, &bAnyTmOutput);
	OutputInt(SzShared("tmCharSet"), ptm->tmCharSet, ppch, rgch, &bAnyTmOutput);
	OutputInt(SzShared("tmOverhang"), ptm->tmOverhang, ppch, rgch, &bAnyTmOutput);
	OutputInt(SzShared("tmDigitizedAspectX"), ptm->tmDigitizedAspectX, ppch, rgch, &bAnyTmOutput);
	OutputInt(SzShared("tmDigitizedAspectY"), ptm->tmDigitizedAspectY, ppch, rgch, &bAnyTmOutput);
   **ppch = '\0';
   CommSz(rgch);
   CommSz(SzShared("\r\n"));
}


/*  C M D   F O N T   I N F O  */
CMD CmdFontInfo (pcmb)
CMB *pcmb;
{
	char rgch[90], *pch;
	TEXTMETRIC tm;

	CommSz(SzShared("\r\n"));
	CommSz(SzShared("F O N T   I N F O R M A T I O N"));
	CommSz(SzShared("\r\n"));
	FetchCpAndPara(selCur.doc, selCur.cpFirst, fcmProps);
	LoadFont(&vchpFetch, fFalse);    /* fills in vfti, vftiDxt */
	pch = rgch;
	OutputVftiInfo(SzShared("vfti"),	&vfti, &pch, rgch);
	if (vfli.fFormatAsPrint == 0)
  		{
		pch = rgch;
		PchToPpch(SzShared("vftiDxt info is not meaningful."), &pch);
  		CommSz(rgch);
		CommSz(SzShared("\r\n"));
		}
	else
      {
		pch = rgch;
		OutputVftiInfo(SzShared("vftiDxt"), &vftiDxt, &pch, rgch);
  		}
	if (vfti.fPrinter)
		{
		Assert (vpri.hdc != NULL);
		GetTextMetrics (vpri.hdc, (LPTEXTMETRIC) &tm);
		}
	else
		{
		Assert (vsci.hdcScratch != NULL);
		GetTextMetrics (vsci.hdcScratch, (LPTEXTMETRIC) &tm);
		}
	pch = rgch;
	OutputTextMetrics(&tm, &pch, rgch);		 
	return cmdOK;
}	/* CmdFontInfo */


CMD CmdFliInfo(pcmb)
CMB *pcmb;
{
	int chrm, bAnyProps;
	struct CHR *pchr;
	char *pch, rgch[90];

	CommSz(SzShared("\r\n"));
	CommSz(SzShared("F L I   I N F O R M A T I O N"));
	CommSz(SzShared("\r\n"));

	CommSzRgbCb(SzShared("rgch: "), vfli.rgch, vfli.ichMac);
	CommSzRgNum(SzShared("rgdxp: "), vfli.rgdxp, vfli.ichMac);
	pch = rgch;
	pchr = &(**vhgrpchr)[0];
	while ((chrm = pchr->chrm) != chrmEnd)
		{
		switch (chrm)
			{
		case chrmChp:
			AppendAndCheckForEoln(&pch, rgch, SzShared("chrmChp: "));
			OutputInt(SzShared("ich"), pchr->ich, &pch, rgch, &bAnyProps);
			break;
		case chrmTab:
			AppendAndCheckForEoln(&pch, rgch, SzShared("chrmTab: "));
			OutputInt(SzShared("ich"), (int)pchr->ich, &pch, rgch, &bAnyProps);
			OutputInt(SzShared("ch"), (int)((struct CHRT *)pchr)->ch, &pch, rgch, &bAnyProps);
			break;
		case chrmVanish:
			AppendAndCheckForEoln(&pch, rgch, SzShared("chrmVanish: "));
			OutputInt(SzShared("ich"), (int)pchr->ich, &pch, rgch, &bAnyProps);
			OutputLongIfDiff(SzShared("dcp"), ((struct CHRV *)pchr)->dcp, 0L, &pch, rgch, &bAnyProps);
			break;
		case chrmFormula:
			AppendAndCheckForEoln(&pch, rgch, SzShared("chrmFormula: "));
			OutputInt(SzShared("ich"), (int)pchr->ich, &pch, rgch, &bAnyProps);
			OutputInt(SzShared("dxp"), ((struct CHRF *)pchr)->dxp, &pch, rgch, &bAnyProps);
			OutputInt(SzShared("dyp"), ((struct CHRF *)pchr)->dyp, &pch, rgch, &bAnyProps);
			OutputInt(SzShared("fLine"), ((struct CHRF *)pchr)->fLine, &pch, rgch, &bAnyProps);
			break;
		case chrmDisplayField:
			AppendAndCheckForEoln(&pch, rgch, SzShared("chrmDisplayField: "));
			OutputInt(SzShared("ich"), (int)pchr->ich, &pch, rgch, &bAnyProps);
			OutputInt(SzShared("flt"), ((struct CHRDF *)pchr)->flt, &pch, rgch, &bAnyProps);
			OutputInt(SzShared("w"), ((struct CHRDF *)pchr)->w, &pch, rgch, &bAnyProps);
			OutputInt(SzShared("w2"), ((struct CHRDF *)pchr)->w2, &pch, rgch, &bAnyProps);
			OutputLongIfDiff(SzShared("l"), ((struct CHRDF *)pch)->l, 0L, &pch, rgch, &bAnyProps);
			OutputInt(SzShared("dxp"), ((struct CHRDF *)pchr)->dxp, &pch, rgch, &bAnyProps);
			OutputInt(SzShared("dyp"), ((struct CHRDF *)pchr)->dyp, &pch, rgch, &bAnyProps);
			OutputLongIfDiff(SzShared("dcp"), ((struct CHRDF *)pchr)->dcp, 0L, &pch, rgch, &bAnyProps);
			break;
		case chrmFormatGroup:
			AppendAndCheckForEoln(&pch, rgch, SzShared("chrmFormatGroup: "));
			OutputInt(SzShared("ich"), (int)pchr->ich, &pch, rgch, &bAnyProps);
			OutputInt(SzShared("flt"), ((struct CHRFG *)pchr)->flt, &pch, rgch, &bAnyProps);
			OutputInt(SzShared("dbchr"), ((struct CHRFG *)pchr)->dbchr, &pch, rgch, &bAnyProps);
			OutputInt(SzShared("dxp"), ((struct CHRFG *)pchr)->dxp, &pch, rgch, &bAnyProps);
			OutputLongIfDiff(SzShared("dcp"), ((struct CHRFG *)pchr)->dcp, 0L, &pch, rgch, &bAnyProps);
			break;
			}
		if (pch != rgch)
			{
			CommSz(rgch);
			CommSz("\n\r");
			pch = rgch;
			}
		(char *)pchr += chrm;
		}
	return cmdOK;
}
#endif
