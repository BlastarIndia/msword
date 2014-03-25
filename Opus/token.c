/* #define DBG */
/* token.c -- Tokenizer for Opel */

#define SBMGR
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "debug.h"
#include "error.h"
#include "heap.h"
#include "doc.h"
#include "cmdtbl.h"
#include "el.h"
#include "props.h"
#include "sel.h"
#include "ch.h"
#include "compare.h"
#include "macrocmd.h"
#include "opuscmd.h"
#include "file.h"
#include "ourmath.h"

#ifdef DEBUG

#include "elxdefs.h"
#include "disp.h"

#endif /* DEBUG */


#ifdef PROTOTYPE
#include "token.cpt"
#endif /* PROTOTYPE */

extern MES ** vhmes;
extern struct CA caPara;
extern int vccpFetch;
extern CHAR HUGE        *vhpchFetch;
extern struct MERR vmerr;
extern struct SEL selCur;
extern CHAR rgchEop [];

NATIVE ELT EltLookupSt(); /* DECLARATION ONLY */
NATIVE BsyElOfSt(); /* DECLARATION ONLY */
NATIVE BcmOrBsyOfSt(); /* DECLARATION ONLY */
NATIVE IbcmFromBcm(); /* DECLARATION ONLY */
NATIVE ELX ElkLookupSt(); /* DECLARATION ONLY */

static BYTE mpelcccb[] =
	{
	CbElcr(rgchEndLineNolabel),
	CbElcr(rgchEndLineIdent),
	CbElcr(rgchEndLineInt),
	CbElcr(rgchEndElt),
	CbElcr(rgchEndElx),
	CbElcr(rgchEndElk),
	CbElcr(rgchEndNum),
	CbElcr(rgchEndIdent),
	CbElcr(rgchEndString),
	CbElcr(rgchEndRem),
	CbElcr(rgchEndInt),
	CbElcr(rgchEndOther),
	CbElcr(rgchEndOther),
	CbElcr(rgchEndBadSyntax),
	CbElcr(rgchEndSpace),
	CbElcr(rgchEndRem),
	CbElcr(rgchEndOther),
	CbElcr(rgchEndUserElk),
	CbElcr(rgchEndOther),
	};


#define cbMaxLine (cchMaxLine + cchIdentMax + 4)

#define elccEol		(elccLim) /* tokenizer use only! */
#define cbTokenQuanta 512
#define fWriteErr (vmerr.fMemFail || vmerr.fDiskFail) /* failed to write buffer to file */

typedef struct _tok /* used for collecting tokens */
	{
	struct CA ca;
	CP cpLine;
	struct ELCR * pelcr;
} TOK;

typedef struct _tokbuf /* used for caching lines to be tokenized */
	{
	BOOL 	fEmpBuf;
	CP	cpStart;
	int	cchElem;
	char	rgchBuf[cbMaxLine];
}	 TOKBUF;



/* F  R E P L A C E  P C A  W I T H  H Q R G B */
/* Fills up a given area described by pca with the token stream	*/
/* from hqrgb. Upto ibLim indexes are scanned in hqrgb.		*/
/* Destroys the original contents of pca. Used to dump tokens	*/
/* in new caches created from templates.			*/
/* Returns success or failure.					*/

/* %%Function:FReplacePcaWithHqrgb %%Owner:krishnam %%reviewed: 7/8/89 */
BOOL FReplacePcaWithHqrgb(pca, hqrgb, ibLim)
struct CA * pca;
HQ hqrgb;
uns ibLim;
{
	CP cp;
	uns ib, cb, doc;
	struct CHP chp;
	char rgb [cchMaxLine];

	StandardChp(&chp);

	doc = pca->doc;
	cp = pca->cpFirst;

	for (ib = 0; ib < ibLim; ib += cb)
		{
		cb = umin(sizeof (rgb), ibLim - ib);
		bltbx((char far *) LpFromHq(hqrgb) + ib, 
				(char far *) rgb, cb);
		if (!FInsertRgch(doc, cp, rgb, cb, &chp, 0))
			{
			FDelete(PcaSet(pca, doc, pca->cpFirst, cp));
			return fFalse;
			}

		cp += cb;
		}

	if (!FDelete(PcaSet(pca, doc, pca->cpFirst+ibLim, pca->cpLim+ibLim)))
		return fFalse;

	return fTrue;
}



/* F E T C H  P C A  T O  H Q R G B */
/* Fetches a tokenized cache  pca, into the token buffer hqrgb. */
/* Used for filling up tokens from tokenized cache when opening */
/* new macro window.						*/
/* Duty of caller to ensure hqrgb is sufficiently large.	*/

/* %%Function:FetchPcaToHqrgb %%Owner:krishnam %%reviewed: 7/8/89 */
FetchPcaToHqrgb(pca, hqrgb)
struct CA * pca;
HQ hqrgb;
{
	uns ib, ibTotal, cbBlt;

	Assert(DcpCa(pca) <= unsMax);

	ibTotal = DcpCa(pca);
	FetchCp(pca->doc, pca->cpFirst, fcmChars);
	for (ib = 0; ib < ibTotal; ib += cbBlt)
		{
		cbBlt = umin(ibTotal - ib, vccpFetch);
		bltbh(vhpchFetch, (CHAR HUGE *) HpOfHq(hqrgb) + ib, cbBlt);
		FetchCp(docNil, cpNil, fcmChars);
		}
}


#define fsyWhite	(128 + 1)
#define fsyIdent	(128 + 2)
#define fsyDigit	(128 + 2 + 4)
#define fsyIdentEnd	(128 + 2 + 8)


csconst char mpchgrfsy [256] = 
{
	0,		0,		0,		0,
			0,		0,		0,		0,
			0,		fsyWhite,	0,		0,
			0,		0,		0,		0,
			0,		0,		0,		0,
			0,		0,		0,		0,
			0,		0,		0,		0,
			0,		0,		0,		0,
			fsyWhite,	0,		0,		eltHash | fsyIdentEnd,
			fsyIdentEnd,	0/*fsyIdentEnd*/,	0,		0,
			eltLParen,	eltRParen,	eltMul,		eltAdd,
			eltComma,	eltSubtract,	0/*fsyDigit*/,	eltDiv,
			fsyDigit,	fsyDigit,	fsyDigit,	fsyDigit,
			fsyDigit,	fsyDigit,	fsyDigit,	fsyDigit,
			fsyDigit,	fsyDigit,	eltColon,	eltSemi,
			eltLt,		eltEq,		eltGt,		eltPrint,
			0,		fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	0,
			0,		0,		0,		0,
			0,		fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	0,
			0,		0,		0,		0,

			/* International characters... */
	0,		0,		0,		0,
			0,		0,		0,		0,
			0,		0,		0,		0,
			0,		0,		0,		0,
			0,		0,		0,		0,
			0,		0,		0,		0,
			0,		0,		0,		0,
			0,		0,		0,		0,
			0,		0,		0,		0,
			0,		0,		0,		0,
			0,		0,		0,		0,
			0,		0,		0,		0,
			0,		0,		0,		0,
			0,		0,		0,		0,
			0,		0,		0,		0,
			0,		0,		0,		0,
			fsyIdent,	fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	0,
			fsyIdent,	fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	0,
			fsyIdent,	fsyIdent,	fsyIdent,	fsyIdent,
			fsyIdent,	fsyIdent,	fsyIdent,	fsyIdent
};


#define FValidSymbolCh(ch, fFirst) \
	((fFirst ? mpchgrfsy[(ch)] : mpchgrfsy[(ch)] & fsyIdent) == fsyIdent)

#define FDigitCh(ch) (mpchgrfsy[(ch)] == fsyDigit)
#define FWhiteCh(ch) (mpchgrfsy[(ch)] == fsyWhite)


/* F  T O K E N I Z E  P C A */
/* Tokenizes the area described by pca.	 */
/* Stores the tokens in *phqrgbTokens. 	 */
/* Returns success or failure.		 */


/* %%Function:FTokenizePca %%Owner:krishnam %%reviewed: 7/8/89 */
BOOL FTokenizePca(pca, phqrgbTokens, pibLimTokens)
struct CA * pca;
HQ * phqrgbTokens;
uns * pibLimTokens;
{
	ELCC elcc;
	TOK tok;
	uns cch;
	char * pbRealStart, * pb, * pb2ndTok;
	HQ hqrgbTokens;
	uns ibTokens, cbTokens;
	CP cpFirst;
	char rgbLine [cbMaxLine];
	int doc;

	if ((hqrgbTokens = HqAllocLcb((long) cbTokenQuanta)) == hqNil)
		{
		ErrorEid(eidNoMemory, "FTokenizePca 1");
		return fFalse;
		}

	StartLongOp();

	ibTokens = 0;
	cbTokens = cbTokenQuanta;

	tok.ca = *pca;
	cpFirst = pca->cpFirst;
	doc = pca->doc;

	do
		{{ /* NATIVE - do loop in FTokenizePca for every line */
		CP cpFirstLine, cpLimLine, cpLim;
		TOKBUF	tokbuf;

#ifdef DEBUG
		BOOL		fSect;
#endif /* DEBUG */

		CacheParaCa(&tok.ca);

		cpLim = tok.ca.cpLim;

#ifdef DEBUG
		fSect = ChFetch(doc, caPara.cpLim-1, fcmChars) == chSect ||
				  ChFetch(doc, caPara.cpLim-1, fcmChars) == chTable;
#endif /* DEBUG */

		cpFirstLine = caPara.cpFirst;
		cpLimLine = caPara.cpLim - ccpEop;

		if (cpLimLine < cpLim)
			{
			char chT;
			int doc = tok.ca.doc;

			if (tok.ca.cpFirst > cpLimLine)
				cpLimLine = tok.ca.cpFirst;
			FetchCp(doc, cpLimLine, fcmChars);
			chT = *vhpchFetch;
			while (cpLimLine < cpLim && chT != chEol && chT != chReturn)
				{
				FetchCp(doc, ++cpLimLine, fcmChars);
				chT = *vhpchFetch;
				}
			}

		cpLim = CpMin(cpLimLine, cpLim);

		/* NOTE: we assume this is the largest of the elccLine's */
		pb=rgbLine+4; /* 1 for elccLineFoo, 1 for cchLine, 2 for cp */
		pb2ndTok = 0;
		tok.cpLine = tok.ca.cpFirst;

		tokbuf.fEmpBuf = fTrue;
		tokbuf.cchElem = 0;

		do 
			{ /* looped once for each token in a line */
			int cbToken;

			tok.pelcr = pb;

				{
				int ch, cch;
				BOOL fNumber, fDecimal;
				BOOL fExpnt;
				CP cp;
				CHAR * pch;
				struct ELCR *pelcr;
				CHAR stSym [cchIdentMax];
				CHAR stNum [cchMaxSt];

				pelcr = tok.pelcr;
				cp = tok.ca.cpFirst;

				if (cp >= tok.ca.cpLim)
					{
					elcc = elccEof;
					goto LGotElcc;
					}

#ifdef DEBUG
				Assert(cp < tok.ca.cpLim);
				Assert(cp < CpMacDoc(doc));
				Assert(doc == caPara.doc);
				Assert(cp >= caPara.cpFirst);
				Assert(fSect || cp <= caPara.cpLim);
				Assert(fSect || cpLim <= caPara.cpLim);
#endif /* DEBUG */

				if (cpLim >= cp + 254)
					{
					elcc = elccLineTooLong;
					goto LGotElcc;
					}

				Assert(cp <= cpLim);

				if (cp == cpLim) /* end of line */
					{
					elcc = elccEol;
					cp += ccpEop;
					goto LReturnElcc;
					}

				if (tokbuf.fEmpBuf)
					{
					Assert(cpLim > cp);
					FetchRgch(&tokbuf.cchElem, tokbuf.rgchBuf, doc, 
							cp, cpLim, 255);

					Assert(tokbuf.cchElem > 0);
					Assert(tokbuf.cchElem == min(cpLim-cp, 255));

					tokbuf.cpStart = cp;
					bltb(rgchEop,tokbuf.rgchBuf+tokbuf.cchElem,
							(int)ccpEop);
					tokbuf.fEmpBuf = fFalse;
					}

			/* Deal with white space */
				pch = &pelcr->st[1];

				Assert(cp-tokbuf.cpStart <= tokbuf.cchElem);
				for (ch=tokbuf.rgchBuf[cp-tokbuf.cpStart]; FWhiteCh(ch);
						cp += 1,
						*pch++ = ch,
						ch = tokbuf.rgchBuf[cp-tokbuf.cpStart]
						);

			/* return space token if more than one space */
				if (pch > &pelcr->st[1] && 
						((pelcr->st[0] = pch - &pelcr->st[1]) > 1 || 
						pelcr->st[1] == chTab))
					{
					elcc = elccSpace;
					goto LReturnElcc;
					}

			/* Check for end of "line" */
				if (cp >= cpLim)
					{
					elcc = elccEol;
					cp += ccpEop;
					goto LReturnElcc;
					}

			/* Fetch a token into stSym.  This token may be:
				A: a word starting with a letter
				B: a word starting with a period 
					followed by a letter 
				C: a number consisting of digits possibly 
					follwed by
					a period or/and [an E/D, optional +/-] and 
					more digits.
				
				fDecimal set if a period is included
				fNumber set if token starts with a digit
				fExpnt set if fNumber and E/D included
			*/

					{
					CP cpLimSym;

					pch = stSym + 1;
					cch = 0;
					cpLimSym = CpMin(cpLim, cp+sizeof(stSym)-1);
					fNumber = fDecimal = fExpnt = fFalse;

					Assert(cp-tokbuf.cpStart <= tokbuf.cchElem);
					ch = tokbuf.rgchBuf[cp-tokbuf.cpStart];

					if (ch == '.')
						{
						fDecimal = fTrue;
						cp += 1;
						ch = tokbuf.rgchBuf[cp-tokbuf.cpStart];
						}

					if (FDigit(ch))
						{
			/* Parse states */
#define spMant	0	/* We are parsing the mantissa */
#define spExp1	1	/* We got a 'D' or 'E' and are expecting exponent 
						This char may be '-' or '+' or a digit */
#define spExp2	2	/* Parsing exponent, this char must be a digit */
								int sp;

						cpLimSym = CpMin(cpLim,cp+sizeof(stNum)-1);
						Assert(cp <= cpLimSym);
						pch = stNum + 1;

						if (fDecimal)
							*pch++ = '.';

						fNumber = fTrue;
						sp = spMant;
						while (cp < cpLimSym)
							{
							Assert(cp-tokbuf.cpStart <= tokbuf.cchElem);
							ch = ChUpper(tokbuf.rgchBuf[cp-tokbuf.cpStart]);

							switch (sp)
								{
							case spMant:
								if (ch == 'D' || ch == 'E')
									{
									fExpnt = fTrue;
									sp += 1;
									}
								else  if (ch == '.')
									{
									fDecimal = fTrue;
									break;
									}
								else  if (!FDigit(ch))
									goto LNumDone;
								break;

							case spExp1:
								if (ch == '-' || ch == '+' || 
										FDigit(ch))
									{
									sp += 1;
									break;
									}
								goto LNumDone;
					/* asking for a syntax error */
							case spExp2:
								if (!FDigit(ch))
									goto LNumDone;
								}

							*pch++ = ch;
							cp += 1;
							}
						if (cp != cpLim)
							goto LLineTooLong;
LNumDone: 
						;
						}
					else  if (FValidSymbolCh(ch, fTrue))
						{
						while (cp < cpLimSym)
							{
							*pch++ = ch;
							cp += 1;

							Assert(cp-tokbuf.cpStart <= tokbuf.cchElem);
							ch = tokbuf.rgchBuf[cp-tokbuf.cpStart];

							if (!FValidSymbolCh(ch, fFalse))
								break;
							}
						}
					}

				if (!fNumber)
					stSym[0] = cch = pch - (stSym + 1);
				else
					stNum[0] = cch = pch - (stNum + 1);

				if (cch > 0)
					{
					if (fDecimal && cch == 1)
						{
						ch = '.';
						goto LSpecial;
						}

					if (fNumber)
						{
						/* token is a number */
LNumber:
						stNum[stNum[0] + 1] = '\0'; /* st --> stz */
						if (fDecimal || fExpnt)
							{
							int cchNum;

							if ((cchNum = CchPackSzPnum(stNum + 1, 
									&pelcr->num)) == 0)
								{
						/* bad number error! */
								CopySt(stNum, pelcr->st);
								elcc = elccBadSyntax;
								}
							else
								elcc = elccNum;
							}
						else
							{
							int w = 0;
							pch = stNum + 1;
							while ((ch = *pch++) != '\0')
								{
					/* Check for overflow and use 
						floating point if it is
						about to happen */
								if (w > 3275)
									{
									fDecimal = fTrue;
									goto LNumber;
									}

								w = w * 10 + ch - '0';
								}

							if ((uns) w > 32767 || 
									pch < stNum + 1 + cch)
								{
								fDecimal = fTrue;
								goto LNumber;
								}

							pelcr->wInt = w;
							elcc = elccInt;
							}
						}
					else
						{
						if (fDecimal)
							{
							elcc = elccElk;

							if ((pelcr->elk = ElkLookupSt(stSym))
									== elkNil)
								{
								CopySt(stSym, pelcr->st);
								elcc = elccUserElk;
								}
							}
						else  if ((pelcr->elt = EltLookupSt(stSym))
								!= eltNil)
							{
							if (pelcr->elt == eltRem)
								{
								elcc = elccRem;
								goto LComment;
								}
							elcc = elccElt;
							}
						else  if ((pelcr->elx = BsyElOfBcm
								(BcmOrBsyOfSt(stSym, 
								docNil, fFalse)))
								!= bsyNil &&
								pelcr->elx < bcmMacStd)
							{
							pelcr->elx = IbcmFromBcm(pelcr->elx);
							elcc = elccElx;
							}
						else
							{
							/* otherwise, token is an identifier */
							elcc = (stSym[0] > cchIdentMax) ? 
									elccBadSyntax : elccIdent;

							CopySt(stSym, pelcr->st);
							}
						}
					}
				else
					{
					/* Special characters */
					if (fDecimal)
						ch = '.';
					else
						cp += 1;

LSpecial:
					elcc = elccElt;
					switch (ch)
						{
						BOOL fInQuote;

					case '\'':
						elcc = elccComment;
						goto LComment;

					case '"':
						elcc = elccString;
LComment:
						fInQuote = elcc == elccString;
					/* NOTE: won't work if lines may be 
							> 256 ch's */
						pch = pelcr->st + 1;
						Assert(cpLim < cp + cchMaxLine);

						while (cp < cpLim)
							{
							Assert(cp-tokbuf.cpStart <= 
									tokbuf.cchElem);
							ch = tokbuf.rgchBuf[cp - 
							tokbuf.cpStart];

							if (elcc != elccString && 
									!fInQuote && 
									ch == '\\' && 
									cp == cpLim - 1)
								break;

							cp += 1;

							if (ch == '"')
								{
								if (elcc == elccString)
									break;

								fInQuote = !fInQuote;
								}

							Assert(ch != chReturn);
							*pch++ = ch;
							}

						Assert(pch-(pelcr->st+1)<=255);
						pelcr->st[0]=pch-(pelcr->st+1);

						if (elcc == elccString && ch != '"')
							{
							/* un-terminated string */
							bltb(pelcr->st + 1, 
									pelcr->st + 2, pelcr->st[0]);
							pelcr->st[0] += 1;
							pelcr->st[1] = '"';
							elcc = elccBadSyntax;
							}
						break;

					case '#':
						elcc = elccHash;
						break;

					case '?':
						pelcr->elt = eltPrint;
						break;

					case '>':
						if (cp < cpLim &&  
								tokbuf.rgchBuf[cp - 
						tokbuf.cpStart] == '=')
							{
							cp += 1;
							pelcr->elt = eltGe;
							}
						else
							pelcr->elt = eltGt;
						break;

					case '<':
						pelcr->elt = eltLt;
						if (cp < cpLim)
							{
							char ch;
							Assert(cp-tokbuf.cpStart
									<= 
									tokbuf.cchElem);
							if ((ch=tokbuf.rgchBuf
									[cp - 
							tokbuf.cpStart])
									== '>')
								{
								pelcr->elt = 
										eltNe;
								cp += 1;
								}
							else  if (ch == '=')
								{
								pelcr->elt = 
										eltLe;
								cp += 1;
								}
							}
						break;

					case '=':
						pelcr->elt = eltEq;
						break;

					case '-':
						pelcr->elt = eltSubtract;
						break;

					case '+':
						pelcr->elt = eltAdd;
						break;

					case '*':
						pelcr->elt = eltMul;
						break;

					case '/':
						pelcr->elt = eltDiv;
						break;

					case '(':
						pelcr->elt = eltLParen;
						break;

					case ')':
						pelcr->elt = eltRParen;
						break;

					case ',':
						pelcr->elt = eltComma;
						break;

					case ';':
						pelcr->elt = eltSemi;
						break;

					case ':':
						pelcr->elt = eltColon;
						break;

					case '\\':
						elcc = elccExtLine;
						break;

					default:
						/* illegal character in macro if nonspace! */
						if (ch >= chSpace)
							{
							elcc = elccBadSyntax;
							pelcr->st[0] = 1;
							pelcr->st[1] = ch;
							}
						else 
							{
							elcc = elccSpace;
							pelcr->st[0] = 0;
							}
						break;
						}
					}
LReturnElcc:
				tok.ca.cpFirst = cp;
				pelcr->elcc = elcc;
				}

LGotElcc:
			if (elcc == elccEol || elcc == elccEof)
				break; /* end of line */

			if (vmerr.fMemFail || vmerr.fDiskFail)
				goto LNoMemory;

			if (elcc == elccLineTooLong)
				goto LLineTooLong;

			/* Insert the token in line buffer */
			cbToken = CbFromElcc(elcc);

			switch (elcc)
				{
			case elccBadSyntax:
			case elccIdent:
			case elccString:
			case elccSpace:
			case elccComment:
			case elccRem:
			case elccUserElk:
				cbToken += tok.pelcr->st[0];
				break;
				}
			pb += cbToken;

			if (pb >= rgbLine + 255 + 4)
				{
LLineTooLong:
				FreeHq(hqrgbTokens);
				EndLongOp(fFalse);
				ErrorEid(eidMacroLineTooBig, "FTokenizePca");
				if (pca->doc == selCur.doc)
					Select(&selCur, cpFirstLine, cpLimLine);
				return fFalse;
				}

			if (pb2ndTok == 0)
				pb2ndTok = pb;
			}
		while (1);

		/* Deal with start of line token */
		/* In case of lines beginning with labels or ints, 
			these are masked out by positioning real start at
			offset of 1. */
		pbRealStart = rgbLine + 1;
		if (rgbLine[4] == elccIdent && *pb2ndTok == elccElt && 
				*(pb2ndTok + 1) == eltColon)
			{
			*pbRealStart = elccLineIdent;
			}
		else  if (rgbLine[4] == elccInt)
			{
			*pbRealStart = elccLineInt;
			}
		else
			{
			pbRealStart -= 1;
			*pbRealStart = elccLineNolabel;
			}

		/* Set line length */
		*(pbRealStart + 1) = (cch = pb - pbRealStart) - 4;
		Assert(cch <= 255);

		/* Set ib (dcp) */
		*((int *) (pbRealStart + 2)) = 
				(int) (tok.cpLine - cpFirst);

		/* Insert line in token buffer */
		if (ibTokens + cch > cbTokens)
			{
			cbTokens = ibTokens + cch + cbTokenQuanta;

			if (cbTokens < ibTokens)
				{
				FreeHq(hqrgbTokens);
				EndLongOp(fFalse);
				ErrorEid(eidMacroTooBig, "FTokenizePca");
				return fFalse;
				}

			/* Resize token buffer */
			if (!FChngSizePhqLcb(&hqrgbTokens, (long) cbTokens))
				{
LNoMemory:
				FreeHq(hqrgbTokens);
				EndLongOp(fFalse);
				ErrorEid(eidNoMemory, "FTokenizePca 2");
				return fFalse;
				}
			}

		bltbh((char huge *) pbRealStart,
				(CHAR HUGE *) HpOfHq(hqrgbTokens) + ibTokens, cch);

		ibTokens += cch;
		}}
	while (elcc != elccEof);

	*phqrgbTokens = hqrgbTokens;
	*pibLimTokens = ibTokens;

	EndLongOp(fFalse);

	return fTrue;
}


typedef struct _tkr
	{
	ELT elt;
	CHAR st [];
} TKR;

#include "rgtkr.h"


/* E L T  L O O K U P  S T */
/* Table driven search to fetch the elt for a given string */

/* %%Function:EltLookupSt %%Owner:krishnam %%reviewed: 7/8/89 */
NATIVE ELT EltLookupSt(st)
CHAR * st;
{
	int itkr, ditkr, dch, cch, cchOrg;
	char *pch, far *lpch, chFirst;

	chFirst = ChLower(st[1]);
	if (chFirst < 'a' || chFirst > 'z')
		return eltNil;
	dch = chFirst-'a';
	itkr = csmpdchitkr[dch];
	ditkr = csmpdchitkr[dch+1] - itkr;
	cchOrg = st[0];
	Assert(cchOrg>0);

	while (--ditkr >= 0)
		{
		if (cchOrg == csrgtkr[itkr].st[0])
			{
			pch = st+2;
			lpch = csrgtkr[itkr].st+2;
			cch = cchOrg-1;
			while (--cch >= 0)
				{
				if (ChLower(*pch++) != ChLower(*lpch++))
					goto NoMatch;
				}
			return(csrgtkr[itkr].elt);
			}
NoMatch:
		itkr++;
		}
	return(eltNil);
}


/* C C H  P A C K  S Z  P N U M */
/* Packs the parsed sz in a num structure. */
/* sz is the numeric token accepted by the tokenizer. It can be */
/* upto cchMaxIdent long and express numbers in sc. notation    */
/* sz = x.xxx.. Exxx */
/* Returns the no of chars in sz if successful else zero. */

/* %%Function:CchPackSzPnum %%Owner:krishnam %%reviewed: 7/8/89 */
CchPackSzPnum(sz, pnum)
CHAR * sz;
NUM * pnum;
{
	CHAR * pch;
	CHAR ch;
	int cchPastDecimal;
	BOOL fNeg;
	BOOL fNegExp, fDiv, fDecimal, fSigDigit;
	int wExp;
	int cchBeforeDecimal, cchSigZerosPastDec;
	ENV env, *penvSave;
	extern ENV *penvMathError;

	pch = PchSkipSpacesPch(sz);
	if (fNeg = (*pch == '-'))
		pch += 1;

	fDecimal=fSigDigit=fFalse;
	cchPastDecimal=cchBeforeDecimal=0;
	cchSigZerosPastDec=-1;

	if (SetJmp(&env) != 0)
		goto LErrMp;
	else
		PushMPJmpEnv(env, penvSave);

	CnumInt(0);
	while ((ch = *pch++) != '\0' && ch != 'E' && ch != 'D')
		{
		if (ch == '.')
			fDecimal=fTrue;
		else
			{
			if ((uns) (ch - '0') > '9' - '0')
				goto LErr;

			CnumInt(10);
			MulNum();
			CNumInt(ch - '0');
			AddNum();
			if (fDecimal)
				{
				if (cchSigZerosPastDec < 0 && ch > '0')
					cchSigZerosPastDec=cchPastDecimal;
				cchPastDecimal++;
				}
			else 			
				{ /* before decimal pt */
				if (!fSigDigit && ch > '0')
					fSigDigit=fTrue;
				if (fSigDigit)
					cchBeforeDecimal++;
				}
			}
		}

	wExp=0;
	fNegExp = fFalse;

	if (ch=='E' || ch == 'D')  /* expnt format */
		{
		if (*pch == '+' || *pch == '-')
			{
			fNegExp = (*pch == '-');
			++pch;
			}
		else
			fNegExp = fFalse;

			/* parse exponent field to check ...
			(1) every member is a digit
			(2) not more than three digits
			*/
			{
			int cchT = 0;
			if (!FDigit(*pch))
				goto LErr;
			while (*pch != '\0')
				{
				if (!FDigit(*pch) || cchT > 2)
					goto LErr;
				wExp=10*wExp+(*pch-'0');
				++pch; 
				++cchT;
				}
			}
		Assert(wExp < 1000);
		if (fNegExp)
			{
			if (cchBeforeDecimal-wExp > cchFormatNumMax)
				goto LErr;
			}
		else 		
			{ /* positive expnt */
			if (cchPastDecimal < wExp)
				{
				/* no is going to increase in size */
				if (cchBeforeDecimal > 0)
					{ /* sig digits before dec pt */
					if (cchBeforeDecimal+wExp > cchFormatNumMax)
						goto LErr;
					}
				else  if /* no sig digits before decimal pt */
				(cchSigZerosPastDec >= 0 && 
						wExp-cchSigZerosPastDec>cchFormatNumMax)
					/* sig digits after dec pt and net value is too large  */
					goto LErr;
				}
			else  if (cchBeforeDecimal+wExp > cchFormatNumMax)
				goto LErr;
			}
		}
	else  if /* not expnt format */
	(cchBeforeDecimal > cchFormatNumMax)
		goto LErr;

	cchPastDecimal += (fNegExp ? wExp : -wExp);
	if (cchPastDecimal > 0)
		fDiv = fTrue;
	else
		{
		fDiv = fFalse;
		cchPastDecimal = -cchPastDecimal;
		}

	if (cchPastDecimal)
		{
		int wQuo,wRem;

		wRem=cchPastDecimal%4;
		wQuo=cchPastDecimal>>2;

		while (--wQuo>=0)
			{
			CNumInt(10000);/* max int which is exponent of 10 */
			if (fDiv)
				DivNum();
			else 
				MulNum();
			}
		if (wRem)
			{
			CNumInt(wRem==1 ? 10 : (wRem==2 ? 100 : 1000));
			if (fDiv)
				DivNum();
			else 
				MulNum();
			}
		}

	if (fNeg)
		DNeg();
	StiNum(pnum);

	PopMPJmp(penvSave);
	return (int) (pch - sz);

LErr:
		{
		NUM num;
	/* pop number */
		StiNum(&num);
LErrMp:
		PopMPJmp(penvSave);
		return 0;
		}
}



#define ChFetchHqrgbIb(hqrgb, ib) (*((char huge *) HpOfHq(hqrgb) + (ib)))
#define WFetchHqrgbIb(hqrgb, ib) \
		(*(int huge *) ((char huge *) HpOfHq(hqrgb) + (ib)))


/* F  E X P A N D  H Q R G B  T O  P C A */
/* Expands tokens from hqrgb[0:ibLim] - detokenizing function.	*/
/* It does the following : 					*/
/*	(1) Expands the token text into a buffer.		*/
/*	(2) Flushes the buffer into a file when nl or full.	*/
/*	(3) Writes the characters from the file onto pcaDest	*/
/*          when all the tokens have been expanded.		*/
/* There is no way of changing the character props of the text  */
/* which is detokenized.					*/
/* Returns success or failure.					*/

/* %%Function:FExpandHqrgbToPca %%Owner:krishnam  %%reviewed: 7/8/89 */
BOOL FExpandHqrgbToPca(hqrgb, ibLim, pcaDest, imei, fIdle)
HQ hqrgb;
uns ibLim;
struct CA * pcaDest;
int imei;
BOOL fIdle;
{
#define cchMax	cbMaxLine
#define cchMarg	20
#define cchLim	(cchMax-cchIdentMax-cchMarg) /* The buffer is considered to be 
	full when there is no more room 
			for one more ident fetch. 20 is
	a reasonable margin to take care
			of Eop's, special chars put for
								Elk's etc.	
			*/

			int cch, cchLine;
	uns ib;
	ELCC elcc, elccPrev;
	BOOL fSpace, fSpaceNext, fNewLine;
	CP cpDest;
	MEI * pmei;
	char *pchCurtok;
	char rgchBuf[cchMax];
	extern int 	vfnNoAlert;
	extern struct FCB ** mpfnhfcb[];
	int fnDest;
	char stName[ichMaxFile + 1];
	CP cpSelFirst, cpSelLim;
#ifdef DEBUG
	CP cpSave = pcaDest->cpFirst;
#endif


	if (imei != iNil)
		{
		pmei = PmeiImei(imei);
		pmei->imelMac = 0;
		if (pmei->hrgmel == hNil && (PmeiImei(imei)->hrgmel = 
				HAllocateCw(cwMEL * (pmei->imelMax = cmelQuanta)))
				== hNil)
			{
			if (!fIdle)
				ErrorEid(eidNoMemory, "ExpandHqrgbPca 1");
			return fFalse;
			}
		}

	if (!fIdle)
		StartLongOp();

	stName[0] = 0;

	if ((fnDest = FnOpenSt(stName, fOstCreate, ofcTemp, NULL)) == fnNil)
		{
		if (!fIdle)
			{
			EndLongOp(fFalse);
			ErrorEid(eidNoMemory, "ExpandHqrgbPca 1");
			}
		return fFalse;
		}

	Assert(fnDest >= fnScratch && mpfnhfcb[fnDest] != hNil);
	Assert(!PfcbFn(fnDest)->fHasFib);
	Assert((*mpfnhfcb[fnDest])->cbMac == 0);

	Assert(vfnNoAlert == fnNil);
	vfnNoAlert = fnDest;

	cpSelFirst = cpNil;
	if (pcaDest->doc == selCur.doc)
		{
		cpSelFirst = selCur.cpFirst;
		if (fIdle)
			{
			cpSelLim = selCur.cpLim;
			Assert(cpSelFirst <= cpSelLim);
			TurnOffSel(&selCur);
			}
		else
			Select(&selCur, cp0, cp0);
		}

	cpDest = pcaDest->cpFirst;

	ib = 0;

	pchCurtok = rgchBuf;
	*pchCurtok = ' ';
	elccPrev = elccNil;

	cchLine = 1;

	fSpaceNext = fTrue;

		{{ /* NATIVE - FExpandHqrgbToPca */
		while (ib < ibLim)
			{
			if (fWriteErr)
				{{
				goto LNoMemory;
				}} /* !NATIVE - FExpandHqrgbToPca */

			fSpace = fSpaceNext;
			fSpaceNext = fTrue;
			cch = 0;

			elcc = ChFetchHqrgbIb(hqrgb, ib);
			ib += 1;

			Assert(cchLine >= 0);
			if (!cchLine) /* new line */
				{
				Assert(sizeof(rgchBuf)-(pchCurtok-rgchBuf) > 
						cchIdentMax+cchMarg);
				bltb(rgchEop, pchCurtok, (int)ccpEop);
				pchCurtok += (int)ccpEop;
				cpDest += ccpEop;
				fNewLine = fTrue;
				}
			else
				{
				fNewLine = fFalse;
				--cchLine;
				}

			*pchCurtok = ' ';

				{{ /* !NATIVE - FExpandHqrgbToPca */
				switch (elcc)
					{
				case elccLineNolabel:
				case elccLineIdent:
				case elccLineInt:
					if (imei != iNil)
						{
						MEL * pmel;

						pmei = PmeiImei(imei);
						if (pmei->imelMac == pmei->imelMax)
							{
							if (!FChngSizeHCw(pmei->hrgmel, 
									cwMEL * (pmei->imelMax + 
									cmelQuanta), fFalse))
								goto LNoMemory;

							pmei = PmeiImei(imei);
							pmei->imelMax += cmelQuanta;
							}

						pmel = &(*pmei->hrgmel)[pmei->imelMac++];
						Assert(cpDest < 0xffffL);
						pmel->cp = (uns) cpDest;
						pmel->ib = ib - 1;
						}

					Assert(cchLine == 0);

					cchLine = ChFetchHqrgbIb(hqrgb, ib++);

			/* set cp */
					WFetchHqrgbIb(hqrgb, ib) = 
							(int) (cpDest - pcaDest->cpFirst);
					ib += 2;

					fSpace = fSpaceNext = fFalse;

					if (elcc == elccLineNolabel)
						break;
					if (elcc == elccLineIdent)
						goto LIdent;

					fSpaceNext = fTrue;
			/* FALL THROUGH */

				case elccInt:
						{
						int w;
						char * pch;

						w = WFetchHqrgbIb(hqrgb, ib);
						ib += sizeof(int);

						cchLine -= sizeof(int);
						pch = pchCurtok + 1;
						cch = CchIntToPpch(w, &pch);
						Assert(cch <= cchIdentMax);
						}
					break;

				case elccNum:
						{
						NUM num;

						bltbx((char far *) LpFromHq(hqrgb) + ib, 
								(char far *) &num, sizeof (NUM));
						ib += sizeof (NUM);

						cchLine -= sizeof (NUM);
						Assert(sizeof(rgchBuf)-(pchCurtok-rgchBuf)-5 > cchMaxNum*2);
						cch = CchPnumToPch(&num, pchCurtok + 1);
						Assert(cch <= cchIdentMax);
						Assert(pchCurtok[1] == ' ' || pchCurtok[1] == '-');
						fSpace = fFalse; /* space is added by CchPnumToPch */
						}
					break;

				case elccElt:
						{{ /* NATIVE - FExpandHqrgbToPca */
						ELT elt;
						int itkr;
						char far * lpst;

						elt = ChFetchHqrgbIb(hqrgb, ib);
						ib += 1;

						cchLine -= 1;

						itkr = csmpeltitkr[elt];
						Assert(itkr > itkrNil);
						Assert(itkr < itkrMax);

						lpst = &csrgtkr[itkr].st;
						cch = *lpst;
						Assert(cch <= cchIdentMax);
						bltbx(lpst + 1, (char far *) pchCurtok + 1, cch);

						if (elt == eltLParen)
							{
							fSpaceNext = fFalse;
							fSpace = fFalse;
							}
						else  if (elt == eltRParen || elt == eltComma)
							fSpace = fFalse;
						}}
					break;

				case elccElk:
						{
						ELK elk;

						elk = WFetchHqrgbIb(hqrgb, ib);
						ib += sizeof(ELK);

						cchLine -= sizeof (ELK);
						GetNameElk(elk, pchCurtok + 1);

						cch = pchCurtok[1] + 1;
						Assert(cch <= cchIdentMax);
						pchCurtok[1] = '.';

						if (elccPrev == elccIdent)
							fSpace = fFalse;
						}
					break;

				case elccUserElk:
						{
						cch = ChFetchHqrgbIb(hqrgb, ib);
						ib += 1;
						Assert(cch <= cchIdentMax);
						bltbx((char far *) LpFromHq(hqrgb) + ib,
								(char far *) (pchCurtok+2), cch);
						ib += cch;
						cchLine -= cch + 1;

						pchCurtok[1] = '.';
						cch += 1;

						if (elccPrev == elccIdent)
							fSpace = fFalse;
						}
					break;

				case elccElx:
						{
						ELX elx;

			/* expand elx and insert at cpDest++ */

						elx = WFetchHqrgbIb(hqrgb, ib);
						ib += sizeof(ELX);

						cchLine -= sizeof (ELX);
						GetNameFromBsy(pchCurtok, BcmFromIbcm(elx));
						cch = *pchCurtok;
						Assert(cch <= cchIdentMax);
						*pchCurtok = ' ';
						}
					break;

				case elccBadSyntax:
					goto LIdent;

				case elccSpace:
					fSpace = fSpaceNext = fFalse;
			/* FALL THROUGH */

				case elccIdent:
LIdent:
			/* insert rgchIdent */
					cch = ChFetchHqrgbIb(hqrgb, ib);
					ib += 1;
					Assert(elcc != elccIdent || cch <= cchIdentMax);
					bltbx((char far *) LpFromHq(hqrgb) + ib, 
							(char far *) (pchCurtok+1), cch);
					ib += cch;
					cchLine -= cch + 1;
					break;

				case elccString:
			/* insert rgchString in quotes */
					cch = ChFetchHqrgbIb(hqrgb, ib);
					ib += 1;

					if (pchCurtok + 2 + cch - rgchBuf >= cchMax)
						{
						Assert(rgchBuf != pchCurtok);

				/* flush buffer and accept token from start */
						if (!FFlush(fnDest,pchCurtok,rgchBuf,0))
							goto LNoMemory;
						pchCurtok = rgchBuf;
						*pchCurtok = ' ';
						}

					Assert(pchCurtok + 2 + cch - rgchBuf < cchMax);
					bltbx((char far *) LpFromHq(hqrgb) + ib, 
							(char far *) (pchCurtok+2), cch);
					ib += cch;
					cchLine -= cch + 1;
					cch += 2;
					pchCurtok[1] = '"';
					pchCurtok[cch] = '"';
					break;

				case elccRem:
					bltb(SzSharedKey("REM",Rem), pchCurtok + 1, cch = 3);
					goto LRem;

				case elccComment:
					pchCurtok[cch = 1] = '\'';
LRem:
						{
						int cchT;

						fSpace = fSpaceNext = fFalse;
						cchT = ChFetchHqrgbIb(hqrgb, ib);
						ib += 1;

						if (pchCurtok + 1 + cch + cchT - rgchBuf >= cchMax)
							{
							Assert(rgchBuf != pchCurtok);
				/* flush buffer and accept token from start */
							if (!FFlush(fnDest,pchCurtok,rgchBuf,0))
								goto LNoMemory;
							bltb(pchCurtok, rgchBuf, cch+1);
							pchCurtok = rgchBuf;
							*pchCurtok = ' ';
							}

						Assert(pchCurtok + 1 + cch + cchT - rgchBuf < cchMax);

						bltbx((char far *) LpFromHq(hqrgb) + ib, 
								(char far *) (pchCurtok+cch+1), cchT);
						cch += cchT;
						ib += cchT;
						cchLine -= cchT + 1;
						}
					break;

				case elccStmtSep:
					pchCurtok[cch = 1] = ':';
					break;

				case elccHash:
					pchCurtok[cch = 1] = '#';
					fSpaceNext = fFalse;
					break;

				case elccExtLine:
					pchCurtok[cch = 1] = '\\';
					fSpaceNext = fFalse;
					break;

#ifdef DEBUG
				default:
					Assert(fFalse);
#endif /* DEBUG */
					}
				}}

			if (cch > 0 || fNewLine)
				{
				Assert(*pchCurtok == ' ');
				if (fSpace)
					cch += 1;
				else
					bltb(pchCurtok+1, pchCurtok, cch);

				cpDest += cch;

				if (pchCurtok+cch-rgchBuf >= cchLim)
					/* overflow: flush buffer and token */
					{
					if (!FFlush(fnDest,pchCurtok,rgchBuf,cch))
						{{
						goto LNoMemory;
						}}
					/* !NATIVE - FExpandHqrgbToPca */
					else
						{
						pchCurtok = rgchBuf;
						goto LDone;
						}
					}
				/* save away token in buff */
				Assert(pchCurtok-rgchBuf+cch < cchLim);
				pchCurtok += cch;
				}
LDone :
			Assert(pchCurtok - rgchBuf < cchLim);
			elccPrev = elcc;
			}
		}}

	Assert(ib == ibLim);

	if (pchCurtok != rgchBuf  &&	/* flush remnants */
			!FFlush(fnDest, pchCurtok, rgchBuf, 0))
		goto LNoMemory;

	pchCurtok = rgchBuf;

	if (!FReplace(pcaDest, fnDest, fc0, (*mpfnhfcb[fnDest])->cbMac))
		goto LNoMemory;

	vfnNoAlert = fnNil;
	pcaDest->cpLim = cpDest;

	Assert(cpSave == pcaDest->cpFirst);

	if (cpSelFirst != cpNil && cpSelFirst <= cpDest)
		{
		extern BOOL vfSeeSel;
#ifdef CRLF
		/* Make sure we don't split a CR/LF pair */
		CachePara(selCur.doc, cpSelFirst);

		if (cpSelFirst == caPara.cpLim - 1)
			cpSelFirst -= 1;
		if (fIdle && cpSelLim == caPara.cpLim - 1)
			cpSelLim -= 1;
#endif
		if (fIdle)
			{
			Select(&selCur, cpSelFirst, cpSelLim);
			TurnOnSelCur();
			}
		else
			{
			Select(&selCur, cpSelFirst, cpSelFirst);
			vfSeeSel = fTrue;
			}
		}

	KillExtraFns();

	if (PmeiImei(imei)->fNotExpanded)
		SetUndoNil();
	
	if (!fIdle)
		EndLongOp(fFalse);
	return fTrue;

LNoMemory :

	if (!fIdle)
		ErrorEid(eidNoMemory, "ExpandHqrgbToPca 2");

	Assert(cpSave == pcaDest->cpFirst);
	DeleteFn(fnDest, fTrue);
	vfnNoAlert = fnNil;

	if (!fIdle)
		EndLongOp(fFalse);

	return fFalse;
}


/* F  F L U S H */
/* Flushes buffer of characters at rgchBuf to file fn. */
/* increments cp and resets current ptr.	       */
/* returns success or failure.			       */

/* %%Function:FFlush %%Owner:krishnam %%reviewed: 7/8/89 */
BOOL FFlush(fn, pchTok, rgchBuf, cchAdd)
int fn;
char *pchTok;
char rgchBuf[];
int cchAdd;
{
	WriteRgchToFn(fn, rgchBuf, pchTok-rgchBuf+cchAdd);
	if (fWriteErr)
		return fFalse;

	return fTrue;
}


#ifdef DEBUG
/* %%Function:ElTestOptions %%Owner:krishnam */
EL ElTestOptions(hst)
char ** hst;
{
	char sz [cchMaxSz];

	StToSz(*hst, sz);
	SetElTestOptions(sz);
}


#endif /* DEBUG */

