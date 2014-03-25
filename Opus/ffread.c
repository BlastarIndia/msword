/*---------------------------------------------------------------------------
|                                                                           |
|   Internal Convertd Routines                                              |
|                                                                           |
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
|   Modification history:                                                   |
|   7/9/85      Created from Convertd sources (jktg)                        |
|   10/9/85     Split FillBQb call into 2 calls, each with <=32K cb (jma)   |
|   10/18/85    Modified string parsers to return error condition (jktg)    |
|   11/4/85     Now ignores uninteresting sylk sub-records (jma)            |
|   11/5/85     qchMac,qddFirst buffer collision checked before return from |
|               each QstGetValue call. (jma)                                |
|   3/10/86     Moved all auxiliary routines to xlUtil.c (jktg)             |
|   3/24/86     Rewrote ReadMatrix and dependent operations. (jktg)         |
|   4/9/86      Split DBASE reading from delimited to handle quotes (jktg)  |
|   9/22/86     Modified workspace allocation to use new heap manager (jktg)|
|   10/87       ported/munged to opus (dsb)                                 |
---------------------------------------------------------------------------*/

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NONCMESSAGES
#define NOWINSTYLES
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
#define NOCTLMGR
#define NODRAWTEXT
#define NOFONT
#define NOGDI
#define NOHDC
#define NOMB
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOPEN
#define NOPOINT
#define NORECT
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
#include "debug.h"
#include "prompt.h"
#include "message.h"
#include "props.h"
#include "doc.h"
#include "file.h"
#include "ch.h"
#include "field.h"
#include "inter.h"
#include "ourmath.h"
#include "ff.h"
#include "filecvt.h"


#define cwFC (sizeof(FC) / sizeof(int))
#define cbFC (sizeof(FC))

/* ------------ Global Variables ---------------------------------------- */
EREF    *perefConv;
SEQEREF *pseqerefConv=NULL;
FF 	*vpff;

extern STM stmGlobal;
extern struct MERR vmerr;
extern struct ITR vitr;
extern ENV *penvMathError;

/* %%Function:FReadDffSs %%Owner:davidbo */
FReadDffSs()
{
	int is, fRet;
	unsigned cRow, cCol;
	struct PPR **hppr;
	FC cfc, cfcMax;
	CHAR st[ichMaxFile];

	fRet = fFalse;
	st[0] = 0;
	vpff->fnTemp = FnOpenSt(st, fOstCreate, ofcTemp, NULL);
	if (vpff->fnTemp == fnNil)
		goto LRet2;

	hppr = HpprStartProgressReport (mstConverting, NULL,
			NIncrFromL(PfcbFn(vpff->fnRead)->cbMac), fTrue);
	if (FQueryAbortCheck())
		goto LRet1;

#ifdef DAVIDBO
	CommSz(SzShared(" ReadDffSs\n\r"));
#endif

	if (!FInitialize())
		goto LRet1;

	/* count total number of array cells */
	cfcMax = 0;
	for (is = 0; is < vpff->isMac; is += 2)
		{
		cfcMax += (FC)(vpff->prgsRow[is+1] - vpff->prgsRow[is] + 1) * (FC)(vpff->prgsCol[is + 1] - vpff->prgsCol[is] + 1);
		}
	cfcMax *= 2;    /* Once for ReadMatrix, once for InsertMatrix */

	cfc = 0;
	for (is = 0; is < vpff->isMac; is += 2)
		{
		cRow = vpff->prgsRow[is+1] - vpff->prgsRow[is] + 1;
		cCol = vpff->prgsCol[is+1] - vpff->prgsCol[is] + 1;
		if (FReadMatrix(vpff->prgsRow[is], vpff->prgsCol[is], vpff->prgsRow[is+1], 
				vpff->prgsCol[is+1], &cfc, cfcMax, hppr))
			{
			if (!FInsertMatrix(cRow, cCol, &cfc, cfcMax, hppr))
				break;
			}
		else  if (FQueryAbortCheck())
			goto LRet1;
		else
			cfc += 2 * cRow * cCol;
		}
	fRet = fTrue;

LRet1:
	StopProgressReport (hppr, pdcRestore);
	DeleteFn(vpff->fnTemp, fTrue /* fDelete */);
LRet2:
	FreePh(&vpff->edMenu.w3Ed);
	return fRet;
}


#define CBMAXBLT ((long)0xfff0)

/*
*  Purpose: read selection into temp file
*  Returns: True, if successful, False otherwise.
*/
/* %%Function:FReadMatrix %%Owner:davidbo */
FReadMatrix(uRowTop, uColLeft, uRowBottom, uColRight, pcfc, cfcMax, hppr)
unsigned uRowTop, uRowBottom, uColLeft, uColRight;
FC *pcfc, cfcMax;
struct PPR **hppr;
{
	int cch, wks, rt, fEnd = fFalse;
	unsigned uRow, uCol, uRowMac, uColMac;
	FC fcWrite, ifcOld;
	URC urc;
	CHAR st[cchMaxSz];
	CHAR rgbRecord[512];

#ifdef DAVIDBO
	CommSz(SzShared("  FReadMatrix\n\r"));
#endif

	Assert(uRowBottom >= uRowTop);
	Assert(uColRight >= uColLeft);
	uColMac = uColRight - uColLeft + 1;
	uRowMac = uRowBottom - uRowTop + 1;
	fcWrite = cbFC * (long)uColMac * (long)uRowMac;
	NullifyFcTbl(vpff->fnTemp, fcWrite);
	vpff->fcRead = fc0;
	ifcOld = fc0;

	Assert(vpff->dff == dffOpenMp || vpff->dff == dffOpenWks || vpff->dff == dffOpenBiff);

	urc.uColLeft = uColLeft;
	urc.uColRight = uColRight;
	urc.uRowTop = uRowTop;
	urc.uRowBottom = uRowBottom;
	for (;;)
		{
		ProgressReportPercent(hppr, (long)0, (long)cfcMax, (long)*pcfc, NULL);
		if (FQueryAbortCheck())
			return fFalse;

		SetStmPos(vpff->fnRead, vpff->fcRead);
		switch (vpff->dff)
			{
		case dffOpenWks:
			if (!FGetWksRecord(&uRow, &uCol, &urc, &wks, rgbRecord))
				return fTrue;
			FillStFromWksRecord(st, rgbRecord, wks);
			break;

		case dffOpenBiff:
			if (!FGetBiffRecord(&uRow, &uCol, &urc, &rt, rgbRecord))
				return fTrue;
			FillStFromBiffRecord(st, rgbRecord, rt);
			break;

		case dffOpenMp:
			if (!FGetMpRecord(&uRow, &uCol, &urc, rgbRecord, &cch))
				return fTrue;

			switch (vpff->edMenu.w1Ed)
				{
			default:
				cch = 0;   /* For now return 0 length string */
				break;
			case EXNUMBER:
			case EXSTRING:
				break;
				}

			bltbyte(rgbRecord, st+1, cch);
			*st = cch;
			break;
			}

		vpff->fcRead = stmGlobal.fc;

		if (*st)
			{
			int cb;
			FC ifc = (uRow - uRowTop) * (long)uColMac + (uCol - uColLeft);

#ifdef DAVIDBO
			CommSzNum(SzShared("RM uRow = "), uRow);
			CommSzNum(SzShared("RM uCol = "), uCol);
			CommSzLong(SzShared("RM ifc "), ifc);
			CommSzLong(SzShared("RM fcWrite "), fcWrite);
			CommSzSt(SzShared("RM value = "), st);
#endif

			if (ifc > ifcOld)
				{
				(*pcfc) += ifc - ifcOld;
				ifcOld = ifc;
				}
			SetStmPos(vpff->fnTemp, cbFC * ifc);
			RgbToVfcStm((CHAR *) &fcWrite, cbFC);
			SetStmPos(vpff->fnTemp, fcWrite);
			cb = *st + 1;
			RgbToVfcStm(st, cb);
			fcWrite += (FC)cb;
			}
		}
}


/* %%Function:FInsertMatrix %%Owner:davidbo */
FInsertMatrix(cRow, cCol, pcfc, cfcMax, hppr)
unsigned cRow, cCol;
FC *pcfc, cfcMax;
struct PPR **hppr;
{
	unsigned iRow, iCol;
	int cchBuf;
	FC ifc, fcRead;
	struct CHP chp;
	struct PAP pap;
	CHAR rgchBuf[cchMaxSz];

#ifdef DAVIDBO
	CommSz(SzShared("  InsertMatrix\n\r"));
#endif

	Assert(PdodDoc(vpff->doc)->fMother);
	MapStc(PdodDoc(vpff->doc), stcNormal, &chp, &pap);

	ifc = 0;
	for (iRow = 0; iRow < cRow; iRow++)
		{
		for (iCol = 0; ; iCol++)
			{
			ProgressReportPercent(hppr, 0L, (long)cfcMax, (long)*pcfc, NULL);
			if (FQueryAbortCheck())
				return fFalse;
			(*pcfc)++;

			SetStmPos(vpff->fnTemp, cbFC * ifc);
			RgbFromVfcStm(&fcRead, cbFC);

#ifdef DAVIDBO
			CommSzLong(SzShared("IM ifc "), ifc);
			CommSzLong(SzShared("IM fcRead "), fcRead);
#endif

			ifc++;
			if (fcRead == fcNil)
				goto LInsertTab;

			SetStmPos(vpff->fnTemp, fcRead);
			cchBuf = BFromVfcStm();
			Assert(cchBuf < cchMaxSz);
			RgbFromVfcStm(rgchBuf, cchBuf);
			RgchToDoc(rgchBuf, cchBuf, &chp, 0);
LInsertTab:
			if (iCol == cCol - 1)
				break;
			rgchBuf[0] = chTab;
			RgchToDoc(rgchBuf, 1, &chp, 0);
			}
		rgchBuf[0] = chReturn;
		rgchBuf[1] = chEol;
		RgchToDoc(rgchBuf, 2, &chp, &pap);
		}

	return fTrue;
}


/* ------------ Lotus specific routines ----------------------------------- */

/* %%Function:FNextNameWks %%Owner:davidbo */
FNextNameWks(pcch, pch, fFirst)
int *pcch;
CHAR *pch;
int fFirst;
{
	int wrt, cb, cch;
	CHAR rgch[512];

	if (fFirst)
		{
		SetStmPos(vpff->fnRead, vpff->fcNames);
		vpff->ibEncrypt = vpff->ibEncryptNames;
		}

	while (fTrue)
		{
		wrt = WFromVfcStm();
		cb = WFromVfcStm();

		Assert(cb <= sizeof(rgch));

		ReadRecord(rgch, cb);

		switch (wrt)
			{
		default:
			continue;
		case END:
		case BLANK:
		case INTEGER:
		case REAL:
		case LABEL:
		case FORMULA:
			return(fFalse);     /* No named areas after this */

		case NNAME: /* Used in Symphony only */
		case NAMED_RANGE:
		case GRAPHNAME: /* Used in symphony only */
		case GRAPH2:    /* Used in symphony only */
		case NAMED_GRAPH:
			if (vpff->fcNames == 0)
				{
				vpff->fcNames = stmGlobal.fc - cb - sizeof(unsigned)*2;
				vpff->ibEncryptNames = (vpff->ibEncrypt - cb) & CBENCMASK;
				}
			if ((cch = CchSz(rgch) - 1) == 0)
				return(fFalse);
			bltbyte(rgch, pch, *pcch = cch);
			return(fTrue);
			}
		}
}


/*
*  Purpose: find next record within bounds of purc
*  Method: sequentially scan from current stream position
*  Returns: fTrue if found, fFalse if end of file.
*/
/* %%Function:FGetWksRecord %%Owner:davidbo */
FGetWksRecord(puRow, puCol, purc, pwks, rgbRecord)
unsigned *puRow, *puCol;
URC *purc;
int *pwks;
CHAR *rgbRecord;
{
	int wks, cb;

	while (fTrue)
		{
		*pwks = WFromVfcStm();
		cb = WFromVfcStm();
		ReadRecord(rgbRecord, cb);
		switch (*pwks)
			{
		default:
			/* if not recognized, skip to next record */
			continue;

		case END:
			return fFalse;

		case LABEL:
			if (*(rgbRecord+6) == '\0')
				continue;
			break;

		case INTEGER:
		case REAL:
			break;

		case FORMULA:
			wks = WFromVfcStm();
			SetStmPos(vpff->fnRead, stmGlobal.fc - sizeof(int));
			if (wks == STRING)
				continue;
			break;

		case STRING:
			break;
			}

		(*puCol) = *(unsigned*)((CHAR *)rgbRecord+1);  /* current cell col */
		(*puRow) = *(unsigned*)((CHAR *)rgbRecord+3);  /* current cell row */
		if (FRowColInPurc(*puRow, *puCol, purc))
			return fTrue;
		}
}


/*
*  Convert Wks record into a value and stuff in st.
*/
/* %%Function:FillStFromWksRecord %%Owner:davidbo */
FillStFromWksRecord(st, rgbRecord, wks)
CHAR *st, *rgbRecord;
int wks;
{
	int cch, fNum, wFmtWks, cdgs, fDate;
	int gfi;
	NUM num;
	ENV env, *penvSave;
	CHAR szTmplt[cchMaxPic+1], szPic[cchMaxPic+1];

	fNum = fFalse;
	gfi = 0;

	if (SetJmp(&env) != 0)
		{
		*st = 0;
		goto LRet;
		}
	else
		PushMPJmpEnv(env, penvSave);

	switch (wks)
		{
	default:
		Assert(fFalse);
		*st = 0;
		goto LRet;

	case LABEL:
		Assert(*(rgbRecord+6) != '\0');
		/* we use (b+6) so we will skip the label-formatting character */
		cch = min(CchSz(rgbRecord+6)-1, cchMaxSz-1);
		bltbyte(rgbRecord+6, st+1, cch);
		*st = cch;
		break;

	case INTEGER:
		fNum = fTrue;
		CNumInt(*(int *)(rgbRecord+5));
		break;

	case REAL:
		fNum = fTrue;
		LdiNum((NUM *)(rgbRecord+5));
		break;

	case FORMULA:
		LdiNum((NUM *)(rgbRecord+5));
		fNum = fTrue;
		break;

	case STRING:
		cch = min(CchSz(rgbRecord+5)-1, cchMaxSz-1);
		bltbyte((CHAR *)rgbRecord+5, st+1, cch);
		*st = cch;
		break;
		}

	if (fNum)
		{           /* Number is in accumulator */
		StiNum(&num);

#ifdef DAVIDBO
		CommSzPnum(SzShared("number read: "), &num);
#endif

		wFmtWks = (*(CHAR *) rgbRecord >> 4) & 7;
		cdgs = (*(CHAR *) rgbRecord) & 0xf;
		fDate = fFalse;
		switch (wFmtWks)
			{
		case 0:
			break;
		case 1:
/* FUTURE: we might, someday, wish to add a gfiExponent to our code (dsb)
				fmt = FMTEXPONENT;
*/
			break;
		case 2:
			gfi |= gfiCurrency;
			break;
		case 3:
			gfi |= gfiPercent;
			break;
		case 4:
			gfi |= (gfiComma | gfiNegInParen);
			break;
		case 7:
			wFmtWks = FMTFromFmtWks(cdgs);
			fDate = wFmtWks != 0;
			break;
		default:
			break;
			}
		if (fDate)
			{
			*st = CchFormatDate(&num, st+1, wFmtWks);
			}
		else
			{
			if (cdgs > 0)
				gfi |= gfiDecimal;
			FPicTmpltPnumGfi(&num, gfi, cdgs, szTmplt,
					cchMaxPic - DcchSafePicTmplt(gfi, cdgs));
			LocalSzPicNumTmplt(szTmplt, szPic);
			*st = CchFormatNumPic(&num, szPic, st+1, cchMaxSz-1);
			}
		}
LRet:
	PopMPJmp(penvSave);
}


/* %%Function:GetWksCol %%Owner:davidbo */
GetWksCol (ppch, rgw)
CHAR **ppch;
unsigned *rgw;
/*-----------------------------------------------------------------------
|   Purpose: Parse the string into a list of columns.                   |
|   Method: Parse the string characters building their integer values   |
|           into rgw.                                                   |
|   Returns: A pointer to the next character after the reference.       |
------------------------------------------------------------------------*/
{
	CHAR    *pch;

	pch = *ppch;
	pch[0] = ChLower(pch[0]);
	pch[1] = ChLower(pch[1]);
	if (FLower(pch[0]))
		{
		if (FLower(pch[1]))
			{       /* column is between AA and IV */
			*rgw = 26* (pch[0]-'a' + 1) + pch[1] - 'a' + 1;
			*ppch = pch + 2;
			return(1);
			}
		*rgw = pch[0] - 'a' + 1;        /* column is between A and Z */
		*ppch = pch + 1;
		return (1);
		}
	return (0);
}


/* %%Function:FGetWksRange %%Owner:davidbo */
int FGetWksRange(id, prow, pcol)
CHAR *id;           /* named range, named graph, specified range, or FFFF */
unsigned *prow, *pcol; /* arrays of ranges returned */
/*-----------------------------------------------------------------------
|   Purpose: Get the Wks range for the Wks name.                        |
|   Method: Find the named reference and build its ranges into the      |
|           reference arrays.                                           |
|   Returns: fTrue if good range, fFalse otherwise.                     |
------------------------------------------------------------------------*/
{
	int wks, cb, i;
	int nranges = 0;    /* assume no ranges defined yet */
	CHAR *r;            /* temp pointer */
	CHAR *pchSrc, *pchDes;
	CHAR idcaps[cchMaxSz]; /* for capitalized version of id */
	CHAR rgbRecord[512];   /* buffer */

	SetStmPos(vpff->fnRead, fc0);
	/* convert to caps */
	pchSrc = id;
	pchDes = &idcaps[0];
	while (*pchSrc)
		*pchDes++ = ChUpper(*pchSrc++);
	*pchDes = '\0';

	for (;;)
		{ /* exits after WksPos set at beginning of worksheet, or error */
		wks = WFromVfcStm();
		cb = WFromVfcStm();
		Assert(cb <= sizeof(rgbRecord));
		ReadRecord(rgbRecord,cb);

		switch (wks)
			{
		default:
			/* Skip the record */
			break;

		case END:
			return fFalse;

		case TOTAL_SIZE:
			SetRange(0,rgbRecord,prow,pcol);    /* init = total size */
			if (*(unsigned*)id==FFFF) /* Entire worksheet? */
				return fTrue;

		case NNAME: /* Used in Symphony only */
		case NAMED_RANGE:
			if (FEqNcSz(idcaps,rgbRecord))
				nranges = SetRange(0,(CHAR *)rgbRecord+16,prow,pcol);
			break;

		case GRAPH2:    /* Used in symphony only */
			/* treat symphony GRAPH2's like a named graph (NGRAPH) */
			wks = NAMED_GRAPH;
			/* Fall through */

		case CURR_GRAPH:
		case NAMED_GRAPH:
			/* Save away location if first graph seen */
			if (wks == NAMED_GRAPH && FEqNcSz(idcaps,rgbRecord))

				{
				/* point at "A" range. (Skip over name in named graph) */
				r = (CHAR *)rgbRecord + (wks==NAMED_GRAPH ? 24: 8);
				/* check each data range "A"-"F" */
				for (i=1; i<7; i++, r+=8)
					{
					/* defined if 1st col not FFFF */
					if (*(unsigned*)r != FFFF)
						nranges = SetRange(nranges,r,prow,pcol);
					}
				}
			break;

		case BLANK:
		case INTEGER:
		case REAL:
		case LABEL:
		case FORMULA:

			if (nranges)
				return (nranges); /* already defined */

			return fFalse;
			/* no longer checks for numeric range */
			}
		}
}


/* %%Function:FParseWks %%Owner:davidbo */
FParseWks (rgwr, rgwc, pch, pisMac)
unsigned *rgwr, *rgwc;
CHAR *pch;
int *pisMac;
/*------------------------------------------------------------------------
|   Purpose: Converts the cols and rows specified in WKS format to the  |
|           corresponding numeric column number.                        |
|   Method: Lotus names its columns from A to Z and AA to IV. This      |
|           routine reads a Lotus column name and calculates its        |
|           corresponding column number, where A corresponds to column  |
|           number 0, Z corresponds to column 25 and IV corresponds to  |
|           column 255. Likewise for the rows.                          |
|   Returns: fTrue if successful, fFalse if not                         |
------------------------------------------------------------------------*/
{
	CHAR    chT;
	unsigned isRow=0;
	unsigned cRange=0;

	/* for each i, rgwr[2i] and rgwr[2i+1] defines the row range,
	rgwc[2i] and rgwc[2i+1] define the column range */
	while (isRow < isMax-1)
		{
		if (!GetWksCol(&pch, rgwc+isRow) ||
				*pch=='-' || !FGetInt(&pch, rgwr + isRow) || *(rgwr+isRow)==0)
			return(fFalse);

		isRow++;

		/* If we haven't just found the upper bound of a range (!cRange),
		assume the integer will be both lower and upper bound. */
		if (!cRange)
			{
			rgwr[isRow] = rgwr[isRow-1];
			rgwc[isRow] = rgwc[isRow-1];
			isRow++;
			}

		if ((chT = *pch++) == chComma)
			cRange = 0;
		else
			{
			switch (chT)
				{
			case '\0':
				Assert(isRow>0 && !(isRow&1));
				*pisMac = isRow;
				pseqerefConv = NULL;
				return(fTrue);

			case '.':
				/* if only 1 '.', error */
				if (*pch++ != '.')
					return fFalse;
			case ':':
				if (cRange++)
				default:
					return(fFalse);
				/* Regress for the upper bound of the range */
				isRow--;
				break;
				}
			}
		}
	/* Overflowed array */
	return(fFalse);
}


/* %%Function:SetRange %%Owner:davidbo */
int SetRange (j,b,prow,pcol)    /* define j'th range and returns j+1 */
int j;              /* current range to set */
CHAR *b;            /* buffer pointer */
unsigned *prow, *pcol; /* storage defined elsewhere */
/*-----------------------------------------------------------------------
|   Purpose: Set the current range.                                     |
|   Method: Get the information from the buffer b and set the col and   |
|           row of the jth range.                                       |
|   Returns: The next range.                                            |
------------------------------------------------------------------------*/
{
	int i = j * 2;

	pcol[i] = *(unsigned*)(b+0) + 1;
	prow[i] = *(unsigned*)(b+2) + 1;
	pcol[i+1] = *(unsigned*)(b+4) + 1;
	prow[i+1] = *(unsigned*)(b+6) + 1;
	return(++j); /* pre-increment and ret */
}


/*
*  Purpose: find next fc within bounds of purc
*  Method: sequentially scan from current stream position
*  Returns: fc if found, fcNil if end of file.
*/
/* %%Function:FGetBiffRecord %%Owner:davidbo */
FGetBiffRecord(puRow, puCol, purc, prt, rgbRecord)
unsigned *puRow, *puCol;
URC *purc;
int *prt;
CHAR *rgbRecord;
{
	int cb, fGetString = fFalse;
	uns uRow, uCol;
	

	while (fTrue)
		{
		*prt = WFromVfcStm();
		cb = WFromVfcStm();
		ReadRecord(rgbRecord, cb);
		switch (*prt)
			{
		default: /* must be code we don't use */
			/* Skip to next record */
			continue;

		case rtEOF:
			return fFalse;

		case rt1904:
			vpff->f1904 = (*((int *)rgbRecord) != fFalse);
			continue;

		case rtLabel:
		case rtInteger:
		case rtNumber:
			break;

		case rtString:
			break;

		case rtArray:
			if (fGetString)
				continue;
			break;

		case rtFormula:
				if (*(rgbRecord+13) == 0xff && *(rgbRecord+14) == 0xff &&
					*(rgbRecord+7) == 0)
				{
				fGetString = fTrue;
				/* remember these for string record that follows */
				uRow = *(unsigned*)(rgbRecord);
				uCol = *(unsigned*)(rgbRecord+2);
				continue;
				}
			break;
			}

		if (*prt == rtString)
			{
			Assert(fGetString);
			fGetString = fFalse;
			(*puRow) = uRow;
			(*puCol) = uCol;
			}
		else
			{
			(*puRow) = *(unsigned*)(rgbRecord);    /* current cell row */
			(*puCol) = *(unsigned*)(rgbRecord+2);  /* current cell col */
			}
		if (FRowColInPurc(*puRow, *puCol, purc))
			return fTrue;
		}
}


csconst unsigned mpipicMPipicXL[ipicMPMax] =
{
	ipicGeneral, ipicGeneral, ipicGeneral, ipicFixed2,   ipicGeneral,
	ipicFixed0,  ipicDollar2, ipicGeneral, ipicPercent0, ipicGeneral,
	ipicComma0,  ipicComma2,  ipicDollar0, ipicPercent2, ipicGeneral,
	ipicMMDDYY,  ipicMMDD,    ipicDDMMMYY, ipicDDMMM,    ipicMMMYY,
	ipicHHMMAP,  ipicHHMMSSAP,ipicHHMM,    ipicHHMMSS,   ipicMDYHM
};

/*
*  Convert Biff record into a value and stuff in st.
*/
/* %%Function:FillStFromBiffRecord %%Owner:davidbo */
FillStFromBiffRecord(st, rgbRecord, rt)
CHAR *st, *rgbRecord;
int rt;
{
	unsigned cch, ipic, gfi;
	int fNum, cdgs, fDate;
	NUM num;
	ENV env, *penvSave;
	CHAR szTmplt[cchMaxPic+1], szPic[cchMaxPic+1];

	fNum = fFalse;
	gfi = 0;

	if (SetJmp(&env) != 0)
		{
		*st = 0;
		goto LRet;
		}
	else
		PushMPJmpEnv(env, penvSave);

	switch (rt)
		{
	default:
		Assert(fFalse);
		*st = 0;
		goto LRet;

	case rtLabel:
		cch = min(*(rgbRecord+7), cchMaxSz-1);
		bltbyte(rgbRecord+8, st+1, cch);
		*st = cch;
		break;

	case rtInteger:
		fNum = fTrue;
		CNumUns(*(unsigned *)(rgbRecord+7));
		break;

	case rtNumber:
		LdiNum((NUM *)(rgbRecord+7));
		fNum = fTrue;
		break;

	case rtFormula:
		LdiNum((NUM *)(rgbRecord+7));
		fNum = fTrue;
		break;

	case rtString:
		fNum = fFalse;
		cch = min(*rgbRecord, cchMaxSz-1);
		bltbyte(rgbRecord + 1, st+1, cch);
		*st = cch;
		break;
		}

	if (fNum)
		{           /* Number is in accumulator */
		StiNum(&num);

#ifdef DAVIDBO
		CommSzPnum(SzShared("number read: "), &num);
#endif

		ipic = (*(rgbRecord + 5)) & 31;
		/*  Surprise, Surprise!  Excel ipics and MP ipics are different! */
		if (vpff->fBiffMP)
			ipic = (ipic < ipicMPMax) ? mpipicMPipicXL[ipic] : ipicGeneral;
		else if (ipic >= ipicXLMax)
			ipic = ipicGeneral;
		fDate = fFalse;
		switch (ipic)       /* Map Biff formats to Multiplan formats */
			{
		case ipicFixed0:
		case ipicFixed2:
			cdgs = (ipic==ipicFixed2) ? 2 : 0;
			break;
		case ipicComma0:
		case ipicComma2:
			gfi |= gfiComma;
			cdgs = (ipic==ipicComma2) ? 2 : 0;
			break;
		case ipicDollar0:
		case ipicDollar0R:
			cdgs = 0;
			goto LDollarCommon;
		case ipicDollar2:
		case ipicDollar2R:
			cdgs = vitr.iDigits;
LDollarCommon:
			gfi |= gfiCurrency;
			break;
		case ipicPercent0:
		case ipicPercent2:
			cdgs = (ipic == ipicPercent2) ? 2 : 0;
			gfi |= gfiPercent;
			break;
		case ipicMMDDYY:
		case ipicDDMMMYY:
		case ipicDDMMM:
		case ipicMMMYY:
		case ipicMDYHM:
		case ipicHHMMAP:
		case ipicHHMMSSAP:
		case ipicHHMM:
		case ipicHHMMSS:
			fDate = fTrue;
			break;
		case ipicGeneral:
		case ipicExp:
		default:
			cdgs = 0;
			break;
			}
		if (fDate)
			{
			*st = CchFormatDate(&num, st+1, ipic);
			}
		else
			{
			if (cdgs > 0)
				gfi |= gfiDecimal;
			FPicTmpltPnumGfi(&num, gfi, cdgs, szTmplt, cchMaxPic - DcchSafePicTmplt(gfi, cdgs));
			LocalSzPicNumTmplt(szTmplt, szPic);
			*st = CchFormatNumPic(&num, szPic, st+1, cchMaxSz-1);
			}
		}
LRet:
	PopMPJmp(penvSave);
}


/* %%Function:FFindNameBiff %%Owner:davidbo */
FFindNameBiff(prgsRow, prgsCol, szRange, pisMac)
unsigned *prgsRow, *prgsCol;
CHAR *szRange;
unsigned *pisMac;
{
	if (FNameLookup(szRange, CchSz(szRange)-1, pseqerefConv->rgeref))
		{
		pseqerefConv->ierefMac = 1;
		*pisMac = 2;
		prgsRow[0] = pseqerefConv->rgeref[0].row0 + 1;
		prgsRow[1] = pseqerefConv->rgeref[0].row1 + 1;
		prgsCol[0] = pseqerefConv->rgeref[0].col0 + 1;
		prgsCol[1] = pseqerefConv->rgeref[0].col1 + 1;
		return (fTrue);
		}
	return (fFalse);
}


/* %%Function:FNameLookup %%Owner:davidbo */
FNameLookup(rgch, cch, peref)
/* look up the name in the external file. file has been opened.
	If found, returns fTrue and sets *peref to ref of name.
	If definition is not a simple area or if in can't be found
	then returns fFalse.
*/
CHAR *rgch;
int cch;
EREF *peref;
{
	int cchT;
	int fFirst;
	EREF eref;
	CHAR rgchT[256];

	fFirst = fTrue;
	while (FNextName(&eref, &cchT, rgchT, fFirst))
		{
		if (cch == cchT && FEqNcRgch(rgch, rgchT, cch))
			{
			*peref = eref;
			return(fTrue);
			}
		fFirst = fFalse;
		}
	return(fFalse);
}


/* %%Function:FNextName %%Owner:davidbo */
FNextName(peref, pcch, rgch, fFirst)
/* Enumerate the the names in the file. Returns True if a name was found. */
EREF *peref;
int *pcch;
CHAR *rgch;
int fFirst;
{
	int cref, rw;
	int cch, cce;
	int rt;
	int cb;
	CHAR *pce;
	CHAR rgb[300];
#define prtname ((RTNAME *)rgb)


	/* start over with first name */
	/* no index record! so search for the first name */
	if (fFirst)
		SetStmPos(vpff->fnRead, vpff->fcNames);

	do
		{
		rt=WFromVfcStm();
		cb = WFromVfcStm();
		ReadRecord(prtname, cb);
		if (rt==rtEOF)
			return(fFalse);
		}
	while (rt != rtLbl);

	if (vpff->fcNames == 0)
		vpff->fcNames = stmGlobal.fc - cb - sizeof(unsigned)*2;
	goto LGotRecord;

	for (;;)
		{
		/* loop until valid name or no more name */
		rt = WFromVfcStm();
		if (rt != rtLbl)
			/* no more names */
			return(fFalse);
		cb = WFromVfcStm();
		ReadRecord(prtname, cb);
LGotRecord:
		cch = prtname->cch;
		/* ignore macros and functions, also */
		/* ignore names without expressions and names too long */
		if (PlblFromPgrbit(&prtname->grbit)->grbit.fProc ||
				prtname->cce == 0)
			continue;

		/* get area from rgce. if we can handle it, return name */
		pce = prtname->rgce+cch;
		switch (*pce++)
			{
		default:
			/* don't know this ptg */
			break;
		case ptgRefN:
			/* cell reference */
			rw = *((int *)pce)++;
			if (rw & grbitFRel)
			/* ignore relative references */
				break;
			peref->row0 =
					peref->row1 = rw & grbitRw;
			/* store column */
			peref->col0 =
					peref->col1 = *pce++;
			goto LGetName;
		case ptgAreaN:
			rw = *((int *)pce)++;
			if (rw & grbitFRel)
				/* ignore relative references */
				break;
			peref->row0 = rw & grbitRw;
			rw = *((int *)pce)++;
			if (rw & grbitFRel)
				/* ignore relative references */
				break;
			peref->row1 = rw & grbitRw;
			peref->col0 = *pce++;
			peref->col1 = *pce++;
LGetName:
			bltb(prtname->rgce, rgch, cch);
			*pcch = cch;
			return(fTrue);
			}
		}
}


/* %%Function:ReadRecord %%Owner:davidbo */
ReadRecord(pb, cb)
CHAR *pb;
int cb;
{
	if (cb < 512)
		{
		RgbFromVfcStm(pb, cb);
		if (vpff->fEncrypted)
			DecryptRecord(pb, cb);
		}
	else
		SetStmPos(vpff->fnRead, stmGlobal.fc + cb);
}


/* %%Function:DecryptRecord %%Owner:davidbo */
DecryptRecord(pb, cb)
CHAR *pb;
int cb;
{
	if (stmGlobal.fc - cb <= ((4+4) + (4))) /* wrtBOF record + next wrt and cb */
		{
		vpff->ibEncrypt = 0;
		return;
		}

	if (vpff->dff == dffOpenBiff)
		vpff->ibEncrypt = stmGlobal.fc & CBENCMASK;

	if (cb < 512)
		DecryptWrt(pb, cb);
	else  if (vpff->dff != dffOpenBiff)
		vpff->ibEncrypt = (vpff->ibEncrypt + cb) & CBENCMASK;
}


/* Read the bounds of the biff file */
/* %%Function:GetBiffBounds %%Owner:davidbo */
GetBiffBounds(peref)
EREF *peref;
{
	int rt,cb;
	CHAR rgch[512];

	SetBytes(peref, 0,  sizeof(EREF));

	SetStmPos(vpff->fnRead, fc0);
	while ((rt = WFromVfcStm()) != rtEOF)
		{
		cb = WFromVfcStm();
		ReadRecord(rgch, cb);
		if (rt == rtDimensions)
			{
			int *pw = (int *) rgch;

			peref->row0 = *pw++;
			peref->row1 = (*pw++) - 1;
			peref->col0 = *pw++;
			peref->col1 = (*pw) - 1;
			break;
			}
		}
	SetStmPos(vpff->fnRead, fc0);
}


/* ------------ initialization routines ----------------------------------- */

/*
*  Purpose: Initialize the state of ReadDffSs before scanning ranges.
*  Method: Initialize all the variables as necessary and reset the 
*          stream pointer. 
*  Returns: No value.
*/
/* %%Function:InitReadDffSs %%Owner:davidbo */
InitReadDffSs()
{
	vpff->fDefSet = fFalse;
	vpff->fcNames = 0;
	vpff->f1904 = fFalse;

#ifdef DAVIDBO
	CommSz(SzShared(" InitReadDffSs\n\r"));
#endif

	SetWords(vpff->prgsRow, 0, isMax);
	SetWords(vpff->prgsCol, 0, isMax);
	vpff->prgsRow[0] = 1;         /* selected row range */
	vpff->prgsRow[1] = 64000; /* selected row range */
	vpff->prgsCol[0] = 1;         /* selected column range */
	vpff->prgsCol[1] = 64000; /* selected column range */
	vpff->isMac = 2;

	vpff->fPublic = fFalse;
	vpff->fEncrypted = fFalse;
	vpff->fHavePassword = fFalse;
	vpff->szPassword[0] = '\0';

	SetWords(&vpff->edMenu, 0, cwED);

	/* set the stream pointer to zero   */
	SetStmPos(vpff->fnRead, fc0);
}


/*
*   Purpose: Initialize ReadDffSs after scanning ranges.
*   Returns: fTrue if successful, fFalse otherwise.
*/
/* %%Function:FInitialize %%Owner:davidbo */
FInitialize()
{
	int ieref;
	CHAR sz[3];
	EREF *peref;

#ifdef DAVIDBO
	CommSz(SzShared("  Initialize\n\r"));
#endif

	switch (vpff->dff)
		{
#ifdef DEBUG
	default:
		Assert(fFalse);
#endif
	case dffOpenMp:
		if (!FInitExtEnum(-1, perefConv, fFalse))
			return fFalse;
		if (!FGetMPBounds(&perefConv->row1, &perefConv->col1))
			return fFalse;

		SetStmPos(vpff->fnRead, fc0);
		perefConv->row0 = vpff->prgsRow[0];
		perefConv->row1 = umin(perefConv->row1, vpff->prgsRow[vpff->isMac-1]);
		perefConv->col0 = vpff->prgsCol[0];
		perefConv->col1 = umin(perefConv->col1, vpff->prgsCol[vpff->isMac-1]);
		FixSells();
		break;

	case dffOpenWks:
		sz[0] = sz[1] = 0xff;
		sz[2] = 0;
		if (!FGetWksRange(sz, &perefConv->row0, &perefConv->col0))
			return fFalse;

#ifdef DAVIDBO
		CommSz(SzShared("Wks Bounds"));
		CommSzNum(SzShared("row0 "), perefConv->row0);
		CommSzNum(SzShared("row1 "), perefConv->row1);
		CommSzNum(SzShared("col0 "), perefConv->col0);
		CommSzNum(SzShared("col1 "), perefConv->col1);
#endif

		SetStmPos(vpff->fnRead, fc0);
		perefConv->row0--;
		perefConv->row1--;
		perefConv->col0--;
		perefConv->col1--;
		FixSells();
		break;

	case dffOpenBiff:
		GetBiffBounds(perefConv);

#ifdef DAVIDBO
		CommSz(SzShared("Biff Bounds"));
		CommSzNum(SzShared("row0 "), perefConv->row0);
		CommSzNum(SzShared("row1 "), perefConv->row1);
		CommSzNum(SzShared("col0 "), perefConv->col0);
		CommSzNum(SzShared("col1 "), perefConv->col1);
#endif

		SetStmPos(vpff->fnRead, fc0);
		FixSells();
		}
	return fTrue;
}


/* Reduce prgscol and prgsrow into area covered by erefConv */
/* %%Function:FixSells %%Owner:davidbo */
FixSells()
{
	int iw;

	FixSel(vpff->prgsRow, &vpff->isMac, perefConv->row0, perefConv->row1);

	FixSel(vpff->prgsCol, &vpff->isMac, perefConv->col0, perefConv->col1);
}


/* %%Function:FixSel %%Owner:davidbo */
FixSel(pw, piwMac, rwFirst, rwLast)
unsigned *pw;
int *piwMac;
unsigned rwFirst, rwLast;
{
	int iw;
	unsigned *pwT;

	for (iw = *piwMac/sizeof(int), pwT = pw; iw ; iw--, pw += 2)
		{
		if (*pwT >= rwFirst)
			break;
		if (*(pwT+1) >= rwFirst)
			{
			*pwT = rwFirst;
			break;
			}
		}

	blt(pwT, pw, iw*sizeof(int));

	for (pwT = pw+(iw-1)*sizeof(int); iw ; iw--, pw -= 2)
		{
		if (*(pwT+1) <= rwLast)
			break;
		if (*pwT <= rwLast)
			{
			*(pwT+1) = rwLast;
			break;
			}
		}

	*piwMac = iw*sizeof(int);
}


/* %%Function:RgchToDoc %%Owner:davidbo */
RgchToDoc(rgchIns, cchIns, pchp, ppap)
CHAR *rgchIns;
int cchIns;
struct CHP *pchp;
struct PAP *ppap;
{
	if (!FInsertRgch(vpff->doc, vpff->cpCur, rgchIns, cchIns, pchp, ppap))
		return;
	vpff->cpCur += cchIns;
}


/* %%Function:NullifyFcTbl %%Owner:davidbo */
NullifyFcTbl(fn, lcb)
int fn;
long lcb;
{
	long lcbWrite;
	CHAR rgb[512];

	SetWords(rgb, -1, 256);
	SetStmPos(fn, fc0);
	for (; lcb; lcb -= lcbWrite)
		{
		lcbWrite = lcb > 512 ? 512 : lcb;
		RgbToVfcStm(rgb, (int) lcbWrite);
		}
}


/* %%Function:FRowColInPurc %%Owner:davidbo */
FRowColInPurc(uRow, uCol, purc)
unsigned uRow, uCol;
URC *purc;
{
	if (uRow >= purc->uRowTop && uRow <= purc->uRowBottom &&
			uCol >= purc->uColLeft && uCol <= purc->uColRight)
		return fTrue;
	return fFalse;
}


/*------------- Multiplan 2.0 routines ------------------------------------*/

#define rowMac20         (rgwGlobals[0])
#define colMac20         (rgwGlobals[1])
#define ictyMacLbl20     (rgwGlobals[3])
#define mpictyposLbl20   ((unsigned *)&rgwGlobals[4])
#define mpictyibLbl20    ((unsigned *)&rgwGlobals[20])
#define ictyMacExp20     (rgwGlobals[36])
#define mpictyposExp20   ((unsigned *)&rgwGlobals[37])
#define mpictyibExp20    ((unsigned *)&rgwGlobals[53])
#define ictyMacEnt20     (rgwGlobals[69])
#define mpictyposEnt20   ((unsigned *)&rgwGlobals[70])
#define mpictyibEnt20    ((unsigned *)&rgwGlobals[86])
#define rowFirst20       (rgwGlobals[102])
#define rowLast20        (rgwGlobals[103])
#define rowCur20         (rgwGlobals[104])
#define colFirst20       (rgwGlobals[105])
#define colLast20        (rgwGlobals[106])
#define colCur20         (rgwGlobals[107])
#define ierefCur20       (rgwGlobals[108])
#define blbl20           (*(unsigned *)&rgwGlobals[109])
#define blblMac20        (*(unsigned *)&rgwGlobals[110])
#define ictyLbl20        (rgwGlobals[111])

#define rowValue       (rgwOutput[0])
#define colValue       (rgwOutput[1])
#define cbValue        (rgwOutput[2])
#define mdValue        (rgwOutput[3])
#define nfmtcValue     (rgwOutput[4])
#define cdgsValue      (rgwOutput[5])
#define alcValue       (rgwOutput[6])

/* %%Function:CwGetGlobalsMP20 %%Owner:davidbo */
CwGetGlobalsMP20 (rgwGlobals)
int *rgwGlobals;
{
	int icty;

	Stream3Setpos(0, 0, ibRwMac20);
	rowMac20 = WFromVfcStm();
	colMac20 = WFromVfcStm();

	Stream3Setpos(0, 0, ibFmtDef20);
	vpff->fmtDef = WFromVfcStm();
	vpff->cdgsDef = WFromVfcStm();
	vpff->fDefSet = fTrue;

	/* position the file at the mpictyDSfo data structures */
	Stream3Setpos(0,0,ibmpfo20);

	ictyMacLbl20=BFromVfcStm();
	for (icty = 0; icty < ICTYMAX; icty++)
		{
		mpictyposLbl20[icty]=WFromVfcStm();
		mpictyibLbl20[icty]=WFromVfcStm();
		}
	ictyMacExp20 = BFromVfcStm();
	for (icty = 0; icty < ICTYMAX; icty++)
		{
		mpictyposExp20[icty]=WFromVfcStm();
		mpictyibExp20[icty]=WFromVfcStm();
		}
	ictyMacEnt20 = BFromVfcStm();
	for (icty = 0; icty < ICTYMAX; icty++)
		{
		mpictyposEnt20[icty]=WFromVfcStm();
		mpictyibEnt20[icty]=WFromVfcStm();
		}
	return(0);
}


/* %%Function:FSetposValueMP20 %%Owner:davidbo */
FSetposValueMP20 (rgwOutput, rgeref, ierefMac, fFirst, rgwGlobals)
int *rgwOutput;
EREF *rgeref;
int ierefMac;
BOOL fFirst;
int *rgwGlobals;
{
	NUM numT;
	int pos;
	unsigned ib;
	int icolMac;
	int bent, icty;
	mpCP cp;
	EREF *peref;
	ENT20 ent;

	Assert(!vpff->fNumExtern);

	if (fFirst)
		{
		ierefCur20=0;
		goto NextRef;
		}
	else
		colCur20++;
	while (ierefCur20<ierefMac)
		{
		for (; rowCur20<=rowLast20; rowCur20++)
			{

			/* position at fo for current row */
			Stream3Setpos(0,0, ibmprwfo20+4*rowCur20);
			pos = WFromVfcStm();
			/* position at htRW and skip over htRW, and cw */
			ib = WFromVfcStm();
			/* if the file position is zero, then the row
			is not defined */
			if (!pos) continue;
			Stream3Setpos(pos,ib,sizeof(HEAPHEAD20));
			icolMac = WFromVfcStm();

			for (; colCur20<=colLast20; colCur20++)
				{
				if (colCur20>=icolMac) break;
				Stream3Setpos(pos,ib,colCur20*cbEID20+cbRWHEAD20);
				bent = WFromVfcStm();
#ifdef GL
				icty = BFromVfcStm()&BMASKCTY;
#else
				icty = BFromVfcStm();
#endif
				if (bent!=0)
					{
					rowValue=rowCur20;
					colValue=colCur20;
					Stream3Setpos(mpictyposEnt20[icty], mpictyibEnt20[icty],bent);
					ent.cwExp=BFromVfcStm();
					if (vpff->fEncrypted)
						{
						vpff->fEncHDS = !(vpff->fPublic && FIsPublic(rowValue, colValue));
						if (vpff->fEncHDS)
							{
							/* Not a Public Cell */
							if (vpff->fHavePassword == 0)
								{
								/* We haven't asked for the password yet */
								if (FCheckPassword(vpff->dff))
									vpff->fHavePassword = 1;
								else
									{
									/* Indicate use doesn't know the password */
									vpff->fHavePassword = -1;
									continue;
									}
								}
							/* If the user doesn't know the password, skip this cell */
							if (vpff->fHavePassword == -1) continue;
							}
						}
					else
						vpff->fEncHDS = fFalse;
					if (vpff->fEncHDS)
						DecryptHDS((char far *)&ent, 1, bent);
					RgbFromVfcStm(((char *)&ent)+1, CbFExp20(ent.cwExp)-1);
					if (vpff->fEncHDS)
						DecryptHDS((CHAR far *)(((char *)&ent)+1), CbFExp20(ent.cwExp)-1, 0);
					mdValue=ent.valtype;
					cbValue=ent.cbVal;
					nfmtcValue=ent.format;
					cdgsValue=ent.digits;
					alcValue=ent.justify;
					if (ent.fCommon && ent.cwExp==0)
						{
						RgbFromVfcStm((char *)&cp, sizeof(mpCP));
						if (vpff->fEncHDS)
							DecryptHDS((char far *)&cp, sizeof(mpCP), 0);
						icty=cp.icty;
						Stream3Setpos(mpictyposExp20[icty], mpictyibExp20[icty],cp.bp+3);
						/* extract cwShExp */
						/* From now on we'll be reading from a shared
							expression record.  Whether or not it is encrypted
							depends on whether or not there are any public areas.
							If there are public areas then the EXP blocks are not
							encrypted. */
						if (vpff->fEncHDS && vpff->fPublic)
							vpff->fEncHDS = fFalse;

						cbValue = BFromVfcStm();
						if (vpff->fEncHDS)
							DecryptHDS((char far *)&cbValue, 1, cp.bp+3);
						cbValue *= sizeof(int);

						/* Don't trim if word aligned . It's okay to put a
						pad byte in a non-shared value. */
						/* trim off the '\0' if the string has an odd number of
						characters */
						if (mdValue == EXSTRING)
							{
							char b;
							Stream3Setpos(mpictyposExp20[icty], mpictyibExp20[icty],cp.bp+3+cbValue);
							b = BFromVfcStm();
							if (vpff->fEncHDS)
								DecryptHDS((char far *)&b,1,cp.bp+3+cbValue);
							if (b == '\0')
								{
								Assert(cbValue!=0);
								cbValue--;
								}
							Stream3Setpos(mpictyposExp20[icty], mpictyibExp20[icty],cp.bp+4);
							/* Reset ibEncrypt */
							if (vpff->fEncHDS)
								DecryptHDS((char far *)0,0,cp.bp+4);
							}
						}
					if (ent.valtype == EXNUMBER)
						{
						RgbFromVfcStm((char *)&numT, sizeof(NUM));
						if (vpff->fEncHDS)
							DecryptHDS((char far *)&numT, sizeof(NUM), 0);

						LdiNum(&numT);
						vpff->fNumExtern = fTrue;
						}
					return(fTrue);
					}
				}
			colCur20 = colFirst20;
			}
		/* Go through next ref */
		ierefCur20++;
NextRef:
		if (ierefCur20<ierefMac)
			{
			peref = &rgeref[ierefCur20];
			rowFirst20 = peref->row0;
			colFirst20 = peref->col0;
			rowLast20 = peref->row1;
			if ((unsigned)rowLast20 >= rowMac20)
				rowLast20 = rowMac20 - 1;
			colLast20 = peref->col1;
			if ((unsigned)colLast20 >= colMac20)
				colLast20 = colMac20-1;
			rowCur20 = rowFirst20;
			colCur20 = colFirst20;
			}
		}
	return(fFalse);
}


/* %%Function:CrefNextNameMP20 %%Owner:davidbo */
CrefNextNameMP20(pwEref, ierefMac, pcch, rgch, ichMac, fFirst, rgwGlobals)
CHAR *pwEref;
int ierefMac;
int *pcch;
CHAR *rgch;
int ichMac;
BOOL fFirst;
int *rgwGlobals;
{
	int cref;
	int cb;
	unsigned ib;
	unsigned pos;
	int ieref;
	LBL20 lbl;
	char rgbFoo[20*sizeof(REF20)];

	if (fFirst)
		{
		ictyLbl20=0;
		blbl20=BENTFIRST20;
		}
	/* Go through external name table, reading in one label at a time. */
	for (; ictyLbl20<ictyMacLbl20; ictyLbl20++)
		{
		ib=mpictyibLbl20[ictyLbl20];
		pos=mpictyposLbl20[ictyLbl20];
		if (blbl20==BENTFIRST20)
			{
			Stream3Setpos(pos,ib,sizeof(HEAPHEAD20));
			blblMac20=WFromVfcStm();
			}
		while (blbl20<blblMac20)
			{
			Stream3Setpos(pos,ib,blbl20);
			RgbFromVfcStm((char *)&lbl, sizeof(LBL20));
			cb=lbl.namlen+lbl.szval+sizeof(LBL20);
			blbl20+=cb;

			/* If length of name same, go ahead and get rest of record */
			cref=lbl.refcnt;

			/* get the macro status, this is only done for MP 2.0 files */

			if (cref!=0 && lbl.namlen<=ichMac)
				{
				/* Get references */
				for (ieref=0; ieref<ierefMac && ieref<cref; ieref++)
					{
					((EREF *)pwEref)->row0=WFromVfcStm();
					((EREF *)pwEref)->row1=WFromVfcStm();
					((EREF *)pwEref)->col0=BFromVfcStm();
					((EREF *)pwEref)->col1=BFromVfcStm();
					pwEref+= sizeof(EREF);
					}
				if (ierefMac<cref)
					{
					RgbFromVfcStm((CHAR *)rgbFoo,(cref-ierefMac)*sizeof(REF20));
					cref=ierefMac;
					}
				/* Get name */
				RgbFromVfcStm((CHAR *)rgch, lbl.namlen);
				*pcch=lbl.namlen;
				return(cref);
				}
			}
		blbl20=BENTFIRST20;
		}
	return(0);
}


/* %%Function:Stream3Setpos %%Owner:davidbo */
Stream3Setpos(pos, ib1, ib2)
unsigned pos, ib1, ib2;
{
	SetStmPos(vpff->fnRead, ((FC)pos*128) + ib1 + ib2);
}


/*
*  Purpose: find next fc within bounds of purc
*  Method: sequentially scan from current stream position
*  Returns: fc if found, fcNil if end of file.
*/
/* %%Function:FGetMpRecord %%Owner:davidbo */
FGetMpRecord(puRow, puCol, purc, rgbRecord, pcch)
unsigned *puRow, *puCol;
URC *purc;
CHAR *rgbRecord;
int *pcch;
{
	int cch;

	while (fTrue)
		{
		if ((cch = (*vpff->edMenu.pfnEd)(&vpff->edMenu,rgbRecord,511)) == cchNil)
			return fFalse;

		(*puRow) = vpff->edMenu.w4Ed - 1;    /* current cell row */
		(*puCol) = vpff->edMenu.w5Ed - 1;  /* current cell col */
		if (FRowColInPurc(*puRow, *puCol, purc))
			{
			*pcch = cch;
			return fTrue;
			}
		}
}


/* %%Function:CchBinCellEnum %%Owner:davidbo */
int
CchBinCellEnum (ped, pch, ichMac)
ED   *ped;        /* enumeration descriptor        */
char *pch;        /* text buffer                */
int  ichMac;        /* maxmimum length at pch        */
/*------------------------------------------------------------------------
| Purpose: To find the next non-blank cell in the Multiplan worksheet,   |
|           and squirrel away its value, type, etc.                      |
| Method:  The previous state information is held in the ED. The value   |
|           is put in at pch. inmEd is ignored; inmCur is incremented    |
|           when another cell is found. w1Ed will hold the cell type,    |
|           w2Ed contains a pointer to the ref of the desired area, w3Ed |
|           has a pointer to rgwGlobals, w4Ed is the cell row, and       |
|           w5Ed is the cell column.                                     |
| Returns: The length of the data.  Puts a '\0' on the end of the data.  |
------------------------------------------------------------------------*/
{
	int cb,                    /* byte count (temporary)        */
	fFirst;                /* true if we are starting the enum */

	char *pchT;
	int rgwOutput[7];        /* array of output values        */
	ENV env, *penvSave;
	CHAR szTmplt[cchMaxPic+1], szPic[cchMaxPic+1];
	CHAR st[cchMaxSz];


	if (SetJmp(&env) != 0)
		{
		*st = 0;
		goto LRet;
		}
	else
		PushMPJmpEnv(env, penvSave);

	/* Restart the enumeration, or initialize for the first time if needed. */
	fFirst = fFalse;
	if (ped->inmEd<=0 || ped->inmLast<0)
		{
		fFirst = fTrue;
		ped->inmLast = inmNil;
		ped->inmEd = 1;
		}

	vpff->fNumExtern = fFalse;

	/* Get the next value, and store the information that pertains to this */
	/* cell entry in the ED struct, and return.                    */
	if (!FSetposValueMP20(rgwOutput, (EREF *)ped->w2Ed, 1, fFirst, *(int **)ped->w3Ed))
		{
		cb = cchNil;
		goto LRet;
		}

	++ped->inmLast;
	vpff->edMenu.w1Ed = mdValue;
	vpff->edMenu.w4Ed = rowValue+1;
	vpff->edMenu.w5Ed = colValue+1;
	cb = cbValue;

	if (vpff->fNumExtern)
		{
		Assert(cb == sizeof(NUM));
		StiNum((NUM *) pch);
		}
	else
		{
		cb = min(cb, ichMac-1);
		RgbFromVfcStm(pch, cb);
		if (vpff->fEncHDS)
			DecryptHDS((char far *)pch, cb, 0);
		pch[cb] = '\0';
		}

	if (rgwOutput[3] == EXNUMBER)
		{
		int gfi, fmt, cdgs;

		gfi = 0;
		cdgs = cdgsValue;
		fmt = nfmtcValue;
		if (fmt == 0)
			{
			Assert(vpff->fDefSet);
			cdgs = vpff->cdgsDef;
			fmt = vpff->fmtDef;
			}
		if (fmt == FMTDOLLAR)
			gfi |= gfiCurrency;
		else  if (fmt == FMTPERCENT)
			gfi |= gfiPercent;
		FPicTmpltPnumGfi((NUM *)pch, gfi, cdgs, szTmplt,
				cchMaxPic - DcchSafePicTmplt(gfi, cdgs));
		LocalSzPicNumTmplt(szTmplt, szPic);
		*st = CchFormatNumPic((NUM *)pch, szPic, st+1, cchMaxSz-1);
		cb = *st;
		bltbyte(st+1, pch, cb);
		}

LRet:
	PopMPJmp(penvSave);
	return(cb);
}


/* %%Function:CchBinNNEnum %%Owner:davidbo */
int
CchBinNNEnum (ped, pch, ichMac)
ED        *ped;      /* enumeration descriptor        */
char      *pch;      /* target buffer for the name        */
int       ichMac;    /* maximum length of name to return */
/*---------------------------------------------------------------------
| Purpose: To get the next name from the Multiplan binary file.        |
| Method:  The order of the names is the order of definition in the    |
|           MP file. The name must have a valid row-column reference,  |
|           which will be parsed and put in *peref, which is obtained  |
|           from ped->w2Ed.    w3Ed points to rgwGlobals.              |
| Returns: The length of the name if all goes according to plan, else  |
|           we return cchNil.                                          |
----------------------------------------------------------------------*/
{
	int  cch,            /* count of chars in name        */
	inmCur,         /* current name index            */
	inmEd;          /* desired name index            */
	int  ieref;

	char    rgchT[cchMaxSz+1];    /* temporary buffer for name        */

	/* (Re-)Initialize the enumeration if required.                */
	inmEd = ped->inmEd;
	inmCur = ped->inmLast;
	if (inmEd<=0 || inmEd<=inmCur || inmCur<0)
		{
		ped->inmLast = inmCur = inmNil;
		}

	/* Loop until we have the name requested in the ED struct.            */
	while (inmCur < inmEd)
		{
		if ((ieref=CrefNextNameMP20((EREF *)ped->w2Ed, 20, &cch, rgchT,
				ichMac, inmCur==inmNil, *(int **)ped->w3Ed))==0)
			{
			ped->inmLim = inmCur;
			return(cchNil);
			}
		pseqerefConv->ierefMac=ieref;
		++inmCur;
		}
	ped->inmLast = inmCur;
	if (cch > ichMac)
		cch = ichMac;
	bltbyte(rgchT, pch, cch);
	return(cch);
}


/* %%Function:FGetMPBounds %%Owner:davidbo */
FGetMPBounds(prowMax, pcolMax)
unsigned    *prowMax,
*pcolMax;
/*----------------------------------------------------------------------
| Purpose: Get the bounds of the Multiplan worksheet.                   |
| Method:  Retrieve the necessary information from the file depending   |
|           on the version.                                             |
| Returns: true if valid bounds found, false if not.                    |
-----------------------------------------------------------------------*/
{
	int        *pw;

	pw = *(int **)vpff->edMenu.w3Ed;
	*prowMax = *pw++;
	*pcolMax = *pw++;

	if ((*prowMax < 1 || *prowMax > 4095) ||
			(*pcolMax < 1 || *pcolMax > 255))
		return(fFalse);
	return(fTrue);
}


/* %%Function:FGetMPRange %%Owner:davidbo */
FGetMPRange (pch, prowsel, pcolsel)
char        *pch;
unsigned    *prowsel,
*pcolsel;
/*-----------------------------------------------------------------------
|    Purpose: Get the range of the named Multiplan area.                 |
|    Method:  Retrieve the necessary information from the file depending |
|            on the version.                                             |
|    Returns: true if a valid range found, false if not.                 |
------------------------------------------------------------------------*/
{
	BOOL    fNameEnum;
	unsigned row0,col0,row1,col1;
	EREF *peref;
	int ieref;


	/* Determine if we have an absolute row/column reference, */
	/* or a name from the stream.  If the latter, go get the  */
	/* area corresponding to the name.                        */
	if (!FInitExtEnum(-1, pseqerefConv->rgeref, fTrue))
		return fFalse;
	fNameEnum = FFindPchEnum(&vpff->edMenu, pch, CchSz(pch)-1);
	FreePh(&vpff->edMenu.w3Ed);
	if (!fNameEnum)
		return fFalse;

	peref = pseqerefConv->rgeref;
	row0 = col0 = 64000;
	row1 = col1 = 0;

	for (ieref = pseqerefConv->ierefMac; ieref--; peref++)
		{
		row0 = umin(row0,peref->row0);
		col0 = umin(col0,peref->col0);
		row1 = umax(row1,peref->row1);
		col1 = umax(col1,peref->col1);
		}
	*prowsel = row0+1;
	*(prowsel + 1) = row1+1;
	*pcolsel = col0+1;
	*(pcolsel + 1) = col1+1;
	vpff->isMac = 2;
	return(fTrue);
}


/* %%Function:FFindPchEnum %%Owner:davidbo */
int
FFindPchEnum (ped, pch, cchMac)
ED        *ped;    /* enumeration descriptor         */
char      *pch;    /* enumerated string we seek      */
int        cchMac; /* max number of chars to compare */
/*----------------------------------------------------------------------
|  Purpose: To call the enumeration described in *ped, and sets inmEd   |
|            if the name given by pch appears.  Resets inmEd to 0       |
|            if the name is not found.                                  |
|  Method: Call the enumerator, and compare strings.  If we find a      |
|           match within cchMac characters, we set up for that inm and  |
|           return.    If we enumerate all the strings and don't find a |
|           match, we set inmEd to 0, so that we will initialize the    |
|           enumeration from the beginning.                             |
|  Returns: true if found, false if not.                                |
------------------------------------------------------------------------*/
{
	int     cch,        /* number of chars in enumerated name */
	fFound,     /* true if a match has been found     */
	eid;      /* prompt for this enumeration        */

	char    rgchNm[cchMaxSz];  /* temporary buffer for enum. names */

	ped->inmEd = 0;
	if (cchMac <= 0)
		return(fFalse);
	eid = ped->eidED;

	do
		{
		cch = (*ped->pfnEd)(ped, rgchNm, cchMac+1);
		++ped->inmEd;
		fFound = (cch==cchMac && FEqNcRgch(pch, rgchNm, cch));
		}
	while (!fFound && cch>0);

	if (fFound)
		ped->inmEd = ped->inmLast;
	return(fFound);
}


/* %%Function:DocCreateDffSsFn %%Owner:davidbo */
DocCreateDffSsFn(dff, fn, szSubset)
int dff, fn;
char *szSubset;
{
	int doc;
	uns w, *rguns;
	struct DOD *pdod;
	FF ff;
	EREF erefConv;
	unsigned rgsRow[isMax],rgsCol[isMax];
	CHAR rgch[cbSector];

	SEQEREF seqeref;

#ifdef DAVIDBO
	CommSz(SzShared("DocCreateDffSsFn\n\r"));
#endif

	pseqerefConv = &seqeref;
	vpff = &ff;
	perefConv = &erefConv;
	ff.prgsRow = rgsRow;
	ff.prgsCol = rgsCol;

	ff.fnRead = fn;
	ff.dff = dff;

	InitReadDffSs();
	bltbh(HpchGetPn(fn, pn0), rgch, cbSector);
	rguns = (uns *)rgch;

	switch (dff)
		{
	case dffOpenBiff:
		ff.fBiffMP = (rguns[2] >= 0x100);
		if (!FParseRangeBiff(szSubset))
			return docNil;

		ff.fEncrypted = FIsFileProtected(dffOpenBiff);
		if (ff.fEncrypted && !FCheckPassword(dff))
			return docNil;
		break;

	case dffOpenWks:
		ff.fEncrypted = FIsFileProtected(dffOpenWks);
		if (ff.fEncrypted && !FCheckPassword(dff))
			return docNil;
		if (!FParseRangeWks(szSubset))
			return docNil;
		break;

	case dffOpenMp:
		if (!FParseRangeMP(szSubset))
			{
			/* Error -- bad range */
			return docNil;
			}

		w = rguns[0] & 0xff08;
		Assert(w == wMagicMPV20 || w == wMagicMPV30);
		if (w == wMagicMPV30)
			{
			ff.fEncrypted = FIsFileProtected(dffOpenMp);
			if (ff.fEncrypted)
				{
				ff.fPublic = FCheckExtForPublic();
				if (!ff.fPublic)
					{
					if (!FCheckPassword(dff))
						return(fFalse);
					}
				}
			}

		break;

	default:
		Assert(fFalse);
		return docNil;
		}

	if ((doc = DocCreate(fnNil)) == docNil)
		return docNil;

	ff.doc = doc;
	ff.cpCur = cp0;

#ifdef DAVIDBO 
		{
		int is;
		CommSzNum(SzShared("ff.doc = "), ff.doc);
		CommSzNum(SzShared("ff.fnRead = "), ff.fnRead);
		CommSzNum(SzShared("ff.dff = "), ff.dff);
		CommSzNum(SzShared("ff.fEncrypted = "), ff.fEncrypted);
		if (ff.fEncrypted)
			{
			CommSzNum(SzShared("ff.fHavePassword = "), ff.fHavePassword);
			if (ff.fHavePassword)
				CommSzSz(SzShared("password: "), ff.szPassword);
			}
		CommSzRgNum(SzShared("Rows: "), ff.prgsRow, ff.isMac);
		CommSzRgNum(SzShared("Cols: "), ff.prgsCol, ff.isMac);
		}
#endif

	if (FReadDffSs())
		{
		pdod = PdodDoc(doc);
		pdod->fn = fn;
		pdod->fDirty = fTrue;
		pdod->fFormatted = fTrue;
		ApplyDocMgmtNew(doc, fnNil);
		}
	else
		{
		DisposeDoc(doc);
		doc = docCancel;
		}

	pseqerefConv=NULL;
	return doc;
}


/* %%Function:FParseRangeWks %%Owner:davidbo */
FParseRangeWks(szRange)
char szRange[];
{
	int isw;
	char sz[3];

#ifdef DAVIDBO
	CommSz(SzShared(" FParseRangeWks\n\r"));
	CommSzSz(SzShared(" szRange: "), szRange);
#endif

	sz[0] = sz [1] = 0xff;
	sz[2] = 0;
	if (!FGetWksRange(sz, vpff->prgsRow, vpff->prgsCol)) /* Set up sheet bounds */
		return fFalse;

	if (szRange[0] != 0 &&
			!FGetWksRange(szRange, vpff->prgsRow, vpff->prgsCol) &&
			!FParseWks(vpff->prgsRow, vpff->prgsCol, szRange, &vpff->isMac) && 
			!FParseMP(vpff->prgsRow, vpff->prgsCol, szRange, &vpff->isMac))
		return (fFalse);

	for (isw = 0; isw < vpff->isMac; isw++)
		vpff->prgsRow[isw]--;
	for (isw = 0; isw < vpff->isMac; isw++)
		vpff->prgsCol[isw]--;

	return(fTrue);
}


/* %%Function:FParseRangeMP %%Owner:davidbo */
FParseRangeMP(szRange)
char szRange[];
{
	int isw;

#ifdef DAVIDBO
	CommSz(SzShared(" FParseRangeMP\n\r"));
#endif

	if (szRange[0] != 0 &&
			!FParseMP(vpff->prgsRow, vpff->prgsCol, szRange, &vpff->isMac) &&
			!FParseWks(vpff->prgsRow, vpff->prgsCol, szRange, &vpff->isMac) && 
			!FGetMPRange(szRange, vpff->prgsRow, vpff->prgsCol))
		return(fFalse);

	FixSelect(vpff->prgsRow, vpff->isMac);
	for (isw = 0; isw < vpff->isMac; isw++)
		vpff->prgsRow[isw]--;

	FixSelect(vpff->prgsCol, vpff->isMac);
	for (isw = 0; isw < vpff->isMac; isw++)
		vpff->prgsCol[isw]--;

	if (szRange[0] == 0)
		pseqerefConv=NULL;
	return(fTrue);
}


/* %%Function:FParseRangeBiff %%Owner:davidbo */
FParseRangeBiff(szRange)
char szRange[];
{
	int isw;

#ifdef DAVIDBO
	CommSz(SzShared(" FParseRangeBiff\n\r"));
	CommSzSz(SzShared(" szRange: "), szRange);
#endif

	if (szRange[0] != 0 &&
			!FParseMP(vpff->prgsRow, vpff->prgsCol, szRange, &vpff->isMac) &&
			!FParseWks(vpff->prgsRow, vpff->prgsCol, szRange, &vpff->isMac) &&
			!FFindNameBiff(vpff->prgsRow, vpff->prgsCol, szRange, &vpff->isMac))
		return(fFalse);

	FixSelect(vpff->prgsRow, vpff->isMac);
	for (isw = 0; isw < vpff->isMac; isw++)
		vpff->prgsRow[isw]--;

	FixSelect(vpff->prgsCol, vpff->isMac);
	for (isw = 0; isw < vpff->isMac; isw++)
		vpff->prgsCol[isw]--;

	if (szRange[0] == 0)
		{
		pseqerefConv=NULL;
		}

	return(fTrue);
}


/* %%Function:FInitExtEnum %%Owner:davidbo */
FInitExtEnum(eid, peref, fNames)
int     eid;      /* id of this enumeration           */
EREF    *peref;     /* area of worksheet to consider    */
int     fNames;     /* true if names, false if cells    */
/*-----------------------------------------------------------------------
|   Purpose: To initialize for an external file access enumeration.     |
|   Method: Determine the type of the stream (Multiplan binary)         |
|           do assignments and allocations to prepare for it.           |
|           Set edMenu.idpmtEd to idpmtNil if we have an error.         |
|   Returns: fTrue if success, fFalse if out of mem.                    |
-----------------------------------------------------------------------*/
{
	extern  CchBinCellEnum(),   /* enumerates MP binary cells       */
	CchBinNNEnum();     /* enumerates MP binary names       */

	int     wT,
	**hw;             /* array of globals         */

	vpff->edMenu.eidED = eid;
	vpff->edMenu.inmEd = 0;
	vpff->edMenu.inmLast = vpff->edMenu.inmLim = vpff->edMenu.inmPage =
			vpff->edMenu.inmRef = inmNil;
	vpff->edMenu.fMovePage = vpff->edMenu.fUseScreen = fFalse;

	if ((hw = HAllocateCw(cwMPGlobals)) == hNil)
		return fFalse;
	wT = CwGetGlobalsMP20(*hw);

	vpff->edMenu.pfnEd = (int (*)())(fNames ? CchBinNNEnum : CchBinCellEnum);
	vpff->edMenu.cnmLine = 4;
	vpff->edMenu.csxNm = 20;
	vpff->edMenu.w1Ed = EXUNDEF;
	vpff->edMenu.w2Ed = (int)peref;
	vpff->edMenu.w3Ed = (int)hw;
	vpff->edMenu.w4Ed = 0;
	vpff->edMenu.w5Ed = 0;
	return fTrue;
}


/* %%Function:FixSelect %%Owner:davidbo */
FixSelect(rgw, iwMac)
unsigned rgw[], iwMac;
	/*-----------------------------------------------------------------------
	|   Purpose: Fixes any inverted ranges. eg. 9:5 becomes 5:9             |
	|   Returns: no value.                                                  |
	-----------------------------------------------------------------------*/
{
	unsigned i,j,k, temp1, temp2;

	for (i=0; i < iwMac; i += 2)        /* check for inverted range */
		if (rgw[i] > rgw[i+1])
			{
			temp1 = rgw[i];
			rgw[i] = rgw[i+1];
			rgw[i+1] = temp1;
			}
}


/* %%Function:FParseMP %%Owner:davidbo */
FParseMP (rgwr, rgwc, pch, pisMac)
unsigned *rgwr,     /* place to store selected row bounds */
*rgwc;
CHAR    *pch;       /* pointer to beginning of string to be parsed */
int     *pisMac;    /* number of elements in the selected-rows array */
/*-----------------------------------------------------------------------
|   Purpose: Converts the cols and rows specified in the Multiplan      |
|           reference to their corresponding row and column numbers.    |
|   Returns: No value                                                   |
-----------------------------------------------------------------------*/
{
	BOOL fFound;
	int ich, isMac;
	EREF *peref;

	pseqerefConv->ierefMac=0;

	peref = pseqerefConv->rgeref;

	isMac = 0;
	fFound = fFalse;

	while (fTrue)
		{
		/* This indicates an expression of the form r1c1:r4c4 */
		if ((ich = IchParseRef(pch, 0, peref)) < 0)
			{               /* an error occurred in parsing */
			pch -= ich;              /* set ptr to next expression   */
			rgwr[isMac] = peref->row0--;
			rgwr[isMac+1] = peref->row1--;
			rgwc[isMac++] = peref->col0--;
			rgwc[isMac++] = peref->col1--;
			peref++;
			pseqerefConv->ierefMac++;
			fFound = fTrue;
			if (*pch++ != chComma) /* commas separate the cell references*/
				break;
			}
		else  /* Parsing error */
			break;
		}

	*pisMac = isMac;
	return fFound;
}


/* %%Function:IchParseRef %%Owner:davidbo */
int IchParseRef (rgchRef, ichStart, peref)
CHAR            *rgchRef;           /* buffer containing reference  */
int             ichStart;           /* starting index of interest   */
EREF            *peref;             /* location in which to store ref   */
/*------------------------------------------------------------------------
|   Purpose: To parse the reference at rgchRef + ichStart and store the |
|           information in *peref.                                      |
|   Method: Look for the letters R and C, and the standard delimiters,  |
|           from left to right in the string. Store the values thru     |
|           peref (0-based) as we parse them.                           |
|   Returns: -(ichMac+ichStart) of reference if the ref was satisfactory. |
|           Otherwise, the index to the error.                          |
------------------------------------------------------------------------*/
{
	extern CHAR *PchSkipSpacesPch();   /* skips blanks in a buffer         */

	int     chFirst,                /* first character parsed           */
	col0,                   /* starting column reference        */
	col1;                   /* maximum column in reference      */
	BOOL    fFirst;                 /* true if first index              */
	BOOL    fNextRef;               /* true if second index is needed   */
	BOOL    fRow;                   /* true if row given explicitly     */
	int     indx;                   /* column or row index              */
	CHAR    *pchCur;                /* index into buffer                */
	int     row0,                   /* starting row reference           */
	row1;                   /* maximum row in reference         */

	/* Get the first non-blank character in the buffer. If it is neither a */
	/* 'R' or a 'C', then return an error indication. */
	pchCur = PchSkipSpacesPch(rgchRef + ichStart);
	chFirst = (int)ChUpper(*pchCur);
	fRow = (chFirst == chRow);
	if (!fRow && chFirst != chCol)
		return(ichStart);
	fNextRef = fTrue;
	fFirst = fTrue;

	while (fTrue)
		{
		if (!fFirst)
			{
			if (chFirst != (int)ChUpper(*pchCur))
				break;
			}

		/* Get the number (either row or column) and check whether in bounds.*/
		/* Look for and process a range if need be. */
		pchCur++;
		if (!FGetInt(&pchCur, &indx))
			return(ichStart);

		if (fFirst)
			{
			row0 = col0 = indx;
			}
		else
			{
			if (fRow)
				row1 = indx;
			else
				col1 = indx;
			}

		if (*pchCur==':' && FDigit(*(pchCur+1)))
			{
			if (!fFirst)
				return(ichStart);
			pchCur++;
			if (!FGetInt(&pchCur, &indx))
				return(ichStart);
			/* Limit to the defined bounds of the sheet */
			fNextRef = fFalse;
			}

		if (fFirst)
			{
			row1 = col1 = indx;
			}


		/* Check to see if we have both rows and columns specified, or if */
		/* one dimension is implicit; i.e. we have entire rows or columns. */
		if (((int)ChUpper(*pchCur))==chRow+chCol-chFirst && FDigit(*(pchCur+1)))
			{
			pchCur++;
			if (!FGetInt(&pchCur, &indx))
				return(ichStart);
			if (fFirst)
				{
				if (fRow)
					col0 = indx;
				else
					row0 = indx;
				}
			else
				{
				if (fRow)
					col1 = indx;
				else
					row1 = indx;
				}
			if (*pchCur==':' && FDigit(*(pchCur+1)))
				{
				if (!fFirst)
					return(ichStart);
				pchCur++;
				if (!FGetInt(&pchCur, &indx))
					return(ichStart);
				fNextRef = fFalse;
				}
			if (fFirst)
				{
				if (fRow)
					col1 = indx;
				else
					row1 = indx;
				}
			}
		else
			{
			if (fRow)
				{
				col0 = 1;
				col1 = 32767;
				}
			else
				{
				row0 = 1;
				row1 = 32767;
				}
			fNextRef = fFalse;
			}
		if (fNextRef)
			{
			fFirst = fFalse;
			if (*pchCur != ':')
				break;
			pchCur++;
			}
		else
			break;
		}

	pchCur = PchSkipSpacesPch(pchCur);

	/* Sort indices so that low one comes first */
	peref->row0 = umin(row0, row1);
	peref->col0 = umin(col0, col1);
	peref->row1 = umax(row0, row1);
	peref->col1 = umax(col0, col1);
	return(-(ichStart + (pchCur - rgchRef)));
}


/* %%Function:FGetInt %%Owner:davidbo */
int FGetInt (ppch, pw)
CHAR    **ppch;             /* address of buffer with int   */
int     *pw;                /* place to stash int when we get it*/
/*-----------------------------------------------------------------------
|   Purpose: To get an integer out of a buffer.                         |
|   Method: Straight-forward parse for signs, digits, etc.              |
|   Returns: true if a good int was found, else false.                  |
-----------------------------------------------------------------------*/
{
	int     ch;                 /* current character being handled  */
	BOOL    fNeg;               /* true if number is negative */
	int     intT;               /* integer we're building */
	CHAR    *pch,               /* pointer to the buffer */
	*pchT;              /* another pointer to the buffer */

	if ((pch = *ppch) == NULL)
		return(fFalse);
	fNeg = fFalse;
	intT = 0;
	while ((ch = *pch++) == ' ')
		;
	switch (ch)
		{
	default:
		break;
	case '-':
		fNeg = fTrue;
	case '+':
		ch = *pch++;
		break;
		}
	pchT = pch;
	while (FDigit(ch))
		{
		intT = intT*10 + ch - '0';
		ch = *pch++;
		}
	if (pch == pchT)
		return(fFalse);
	*ppch = pch - 1;
	if (fNeg)
		intT=-intT;
	*pw = intT;
	return(fTrue);
}


csconst unsigned mpFmtwksFMT[] = 
{
	0, 0, FMTDDMMMYY, FMTDDMMM, FMTMMMYY, 0, 0, FMTHHMMSSAP,
			FMTHHMMAP, FMTMMDDYY, FMTMMDD, FMTHHMMSS, FMTHHMM 
};
#define wfsMax (sizeof(mpFmtwksFMT)/sizeof(unsigned))


FMTFromFmtWks(fmtWks)
int fmtWks;
{
	Assert(fmtWks >= 0);

	if (fmtWks < wfsMax)
		return (mpFmtwksFMT[fmtWks]);

	return 0;
}


csconst CHAR rgstMonths[][] =
{
	StKey("Jan", Jan),
			StKey("Feb", Feb),
			StKey("Mar", Mar),
			StKey("Apr", Apr),
			StKey("May", May),
			StKey("Jun", Jun),
			StKey("Jul", Jul),
			StKey("Aug", Aug),
			StKey("Sep", Sep),
			StKey("Oct", Oct),
			StKey("Nov", Nov),
			StKey("Dec", Dec)
};


/*     J   F   M   A   M   J   J   A   S   O   N   D */
csconst CHAR mpiMoncDay[24] = {
	31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
			31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};


csconst CHAR szPM[] = SzKey("PM", PM);
csconst CHAR szAM[] = SzKey("AM", AM);

csconst char szFmtError[] = SzKey("########", FmtError);
#define cchFmtError (sizeof(szFmtError) - 1);


/*---------------------------------------------------------------------------
CchFormatDate:
	Used to format dates imported via Library Link Spreadsheet.
	This routine takes a char* to where to put the result.
	It then formats the date into any of the Excel default date formats.
	Currently, only Biff spreadsheet dates get formatted.
	(moved from lsmp2.c to use the csconst array of month-idstrs)

	BE WARNED -- some of this code appears wrong, for instance, the 
	calculation of Leap Year.  This is intentional, and it is necessary
	to compensate for Lotus123, which uses that very brain-dead calculation
	of LeapYear.  Lotus also starts counting days at 1, not 0.  If you
	really must change any of this code, it is imperative you test 
	spreadsheets calculated with Base-year 1900 AND base-year 1904.
---------------------------------------------------------------------------*/
CchFormatDate(pnum, pch, fmt)
NUM  *pnum;
char *pch;
int   fmt;
{
	int fDate, fTime;
	NUM numDateTime, numDate, numTime;
	unsigned cYear, iMon, cDay, dDay;
	unsigned cHour, cMin, cSec, dSec;
	int cch, cchT;
	int fSec;     /* valid only if fTime == fTrue */
	int tAmPm;    /* tOff (0), tAM (1), tPM (2) */

	fDate = fFalse;
	fTime = fFalse;
	switch (fmt)
		{
	case FMTMMDDYY:
	case FMTDDMMMYY:
	case FMTDDMMM:
	case FMTMMMYY:
	case FMTMMDD:
		fDate = fTrue;
		break;
	case FMTMDYHM:
		fDate = fTrue;
		/* fall through */
	case FMTHHMMAP:
	case FMTHHMMSSAP:
	case FMTHHMM:
	case FMTHHMMSS:
		fTime = fTrue;
		break;
		}

	LdiNum(pnum);
	if (vpff->f1904)
		{ /* normalize to 1900 */
		CNumInt(BaseYearOffset);
		AddNum();
		}

	StiNum(&numDateTime);	/* d.hms */
	LdiNum(&numDateTime);
	FixNum(0);
	StiNum(&numDate);		/* save d */
	LdiNum(&numDateTime);
	SubINum(&numDate);		/* d.hms - d */
	StiNum(&numTime);		/* save .hms */

	if (fDate)
		{
		LdiNum(&numDate);
		/* touch this and die; see header comment */
		cDay = CUnsNum() + (vpff->f1904);
		if (cDay < DaySpreadsheetMax)
			{
			int fLeapYear;

			cYear = dDay = 0;
			fLeapYear = FSpreadsheetLeapYear(BaseYear + cYear);
			while ((dDay + 365 + fLeapYear) < cDay)
				{
				dDay += 365 + fLeapYear;
				cYear++;
				fLeapYear = FSpreadsheetLeapYear(BaseYear + cYear);
				}
			cDay -= dDay;
			/* assert no fraction of day */

			fLeapYear = FSpreadsheetLeapYear(cYear + BaseYear);
			iMon = (fLeapYear ? 0 : 12);
			dDay = 0;
			while ((dDay + mpiMoncDay[iMon]) < cDay)
				{
				dDay += mpiMoncDay[iMon++];
				}
			cDay -= dDay;
			if (!fLeapYear)
				iMon -= 12;
			while (cYear > 99)
				cYear -= 100;
			Assert(cYear >= 0 && cYear <= 99);
			Assert(iMon >= 0 && iMon < 12);
			Assert(cDay >= (vpff->f1904 ? 1 : 0) && cDay <= 31);
			/* now cYear, iMon, and cDay are set */
			}
		else
			{/* show overflow */
			fmt = FMTError;
			fTime = fFalse;
			}
		}

	if (fTime)
		{
		NUM numTimeRemain;

		/* add in half-second to compensate for round-off */
		CNumInt(24);                /* hours in the day */
		MulINum(&numTime);
		StiNum(&numTimeRemain);		/* h.ms */

		LdiNum(&numTimeRemain);
		FixNum(0);
		StiNum(&numTime);			/* save h */

		LdiNum(&numTimeRemain);
		SubINum(&numTime);			/* h.ms - h */
		StiNum(&numTimeRemain);		/* .ms */

		LdiNum(&numTime);
		cHour = CIntNum();
		Assert(cHour >= 0 && cHour <= 24);

		CNumInt(60);                /* minutes in the day */
		MulINum(&numTimeRemain);
		StiNum(&numTimeRemain);		/* m.s */

		LdiNum(&numTimeRemain);
		FixNum(0);
		StiNum(&numTime);			/* save m */

		LdiNum(&numTimeRemain);
		SubINum(&numTime);
		StiNum(&numTimeRemain);		/* .s */

		LdiNum(&numTime);
		cMin = CIntNum();
		Assert(cMin >= 0 && cMin <= 60);

		CNumInt(60);                /* seconds in the day */
		MulINum(&numTimeRemain);
		FixNum(0);
		cSec = CIntNum();            /* rounding error ? */
		Assert(cSec >= 0 && cSec <= 60);
		fSec  = fFalse;
		tAmPm = tOff;
		/* now cHour, cMin, and cSec are set */
		}

	/* convert to chars stored in pch */
	cch = 0;
	switch (fmt)
		{
	case FMTDDMMMYY:
	case FMTDDMMM:
		/* cDay can be zero if !f1904 */
		cch += CchIntToPpch(cDay, &pch);
		*pch++ = chHyphen;
		cch++;
		/* fall through */
	case FMTMMMYY:
		Assert(fDate && !fTime);
		cchT = rgstMonths[iMon][0];
		bltbx((CHAR FAR *)&(rgstMonths[iMon][1]), (CHAR FAR *)pch, cchT);
		pch += cchT;
		cch += cchT;
		if (fmt == FMTDDMMM)
			break;
		*pch++ = chHyphen;
		cch++;
LPutYY:
		if (cYear < 10)
			{
			*pch++ = '0';
			cch++;
			}
		cch += CchIntToPpch(cYear, &pch);
		break;
	case FMTMDYHM:
		Assert(fDate && fTime);
		/* fall through */
	case FMTMMDD:
	case FMTMMDDYY:
	default:
		cch += CchIntToPpch((iMon+1), &pch);
		*pch++ = vitr.chDate;
		cch++;
		cch += CchIntToPpch(cDay, &pch);
		if (fmt == FMTMMDD)
			break;
		*pch++ = vitr.chDate;
		cch++;
		goto LPutYY;
	case FMTHHMMAP:
		Assert(!fDate && fTime);
		tAmPm = !tOff;
		break;
	case FMTHHMMSSAP:
		tAmPm = !tOff;
		/* fall through */
	case FMTHHMMSS:
		fSec  = fTrue;
		/* fall through */
	case FMTHHMM:
		Assert(!fDate && fTime);
		break;
	case FMTError:
		Assert(!fTime);
		cch = cchFmtError;
		bltbx((CHAR FAR *)szFmtError, (CHAR FAR *)pch, cch);
		break;
		}

	if (fTime)
		{
		if (fDate)
			{
			*pch++ = chSpace;
			cch++;
			}
		if (tAmPm != tOff)
			{
			tAmPm  = (cHour > 12) ? tPM : tAM;
			cHour -= (cHour > 12) ? 12 : 0;     /* 12 hour clock */
			if (cHour == 0)
				cHour = 12;
			}
		cch += CchIntToPpch(cHour, &pch);
		*pch++ = vitr.chTime;
		cch++;
		if (cMin < 10)
			{
			*pch++ = '0';
			cch++;
			}
		cch += CchIntToPpch(cMin, &pch);
		if (fSec)
			{
			*pch++ = vitr.chTime;
			cch++;
			if (cSec < 10)
				{
				*pch++ = '0';
				cch++;
				}
			cch += CchIntToPpch(cSec, &pch);
			}
		if (tAmPm != tOff)
			{
			*pch++ = chSpace;
			cch++;
			cchT = (tAmPm == tPM ? sizeof(szPM) : sizeof(szAM)) - 1;
			bltbx((CHAR FAR *) ((tAmPm == tPM) ? szPM : szAM),
					(CHAR FAR *) pch, cchT);
			cch += cchT;
			pch += cchT;
			}
		}
	return cch;
}


FSpreadsheetLeapYear(year)
int year;
{
	int fLeapYear = fFalse;

	if ((year % 4) == 0)
		{
#ifdef BOGUS
		/* Lotus123 does not use the extra checks below,
			* so to remain consistent with them, we can't
			* either.  It would make Word's imported dates
			* different from what appears in the spreadsheet.
			* DO NOT put these checks in.  If you want a true
			* LeapYear function, copy and rename this.
			*/
		if ((year % 100) != 0)
			fLeapYear = fTrue;
		else  if ((year % 400) == 0)
#endif
			fLeapYear = fTrue;
		}
	return fLeapYear;
}
