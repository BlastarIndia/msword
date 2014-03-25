
/* F L T E X P . C */

#ifdef PCJ
/* #define DBGYXY */
#endif /* PCJ */
#ifdef DBGYXY
#define SHOWLEX
#define SHOWCHFETCH
#define SHOWFORMAT
#define SHOWTI
#define SHOWSTACK
#endif

#define NOMINMAX
#define NOPOINT
#define NOSCROLL
#define NOSHOWWINDOW
#define NOREGION
#define NOVIRTUALKEYCODES
#define NOWH
#define NORASTEROPS
#define NOGDI
#define NOMETAFILE
#define NOBITMAP
#define NORECT
#define NOWNDCLASS
#define NOBRUSH
#define NOMSG
#define NOWINOFFSETS
#define NOWINMESSAGES
#define NONCMESSAGES
#define NOKEYSTATE
#define NOWINSTYLES
#define NOCLIPBOARD
#define NOHDC
#define NOCREATESTRUCT
#define NOTEXTMETRIC
#define NOGDICAPMASKS
#define NODRAWTEXT
#define NOCTLMGR
#define NOSYSMETRICS
#define NOMENUS
#define NOMB
#define NOCOLOR
#define NOPEN
#define NOFONT
#define NOOPENFILE
#define NOMEMMGR
#define NORESOURCE
#define NOSYSCOMMANDS
#define NOICON
#define NOKANJI
#define NOSOUND
#define NOCOMM

#define FLTEXP

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "ch.h"
#include "doc.h"
#include "props.h"
#include "disp.h"
#include "sel.h"
#include "border.h"
#include "heap.h"
#include "field.h"
#include "inter.h"
#include "prompt.h"
#include "message.h"
#include "strtbl.h"
#include "ourmath.h"
#include "debug.h"
#include "opuscmd.h"
#include "fltexp.h"

extern BOOL	fInitialized;   /* math pack initialized? */
extern char	szEmpty[];

#ifdef BRYANL
#define BryanL(f)	(f)
#else
#define BryanL(f)	
#endif	      

/* pd: Actual Production						*/
/*----------------------------------------------------------------------*/
/*  1: <fexp> -> \ = <exp>						*/
/*  2: <fexp> -> = <exp>						*/
/*  3: <exp> -> <exp> = <exp>						*/
/*  4: <exp> -> <exp> < <exp>						*/
/*  5: <exp> -> <exp> <= <exp>						*/
/*  6: <exp> -> <exp> > <exp> 						*/
/*  7: <exp> -> <exp> >= <exp>						*/
/*  8: <exp> -> <exp> <> <exp>						*/
/*  9: <exp> -> <exp> + <exp>						*/
/* 10: <exp> -> <exp> - <exp>						*/
/* 11: <exp> -> <exp> * <exp>						*/
/* 12: <exp> -> <exp> / <exp> 						*/
/* 13: <exp> -> <exp> ^ <exp> 						*/
/* 14: <exp> -> - <exp>							*/
/* 15: <exp> -> BOOL							*/
/* 16: <exp> -> <ucnum>							*/
/* 17: <exp> -> BOOKMARK						*/
/* 18: <exp> -> ( <exp> )						*/
/* 19: <exp> -> RDCFNC ( <tbl ref> )					*/
/* 20: <exp> -> BINFNC ( <exp> , <exp> )				*/
/* 21: $$21 -> epsilon							*/
/* 22: $$22 -> epsilon							*/
/* 23: <exp> -> UNRFNC $$21 ( $$22 <exp> )				*/
/* 24: <exp> -> IF ( <exp> , <exp> , <exp> )				*/
/* 25: <tbl ref> -> <bkmk_rel>						*/
/* 26: <tbl ref> -> BOOKMARK <l_bracket> <r_bracket>			*/
/* 27: <tbl ref> -> <texp> , <texp>					*/
/* 28: <l_bracket> -> [							*/
/* 29: <r_bracket> -> ]							*/
/* 30: <bkmk rel> -> BOOKMARK <l_bracket> <loc> <r_bracket>		*/
/* 31: <bkmk rel> -> BOOKMARK <l_bracket> <locbk> : <locbk> <r_bracket>	*/
/* 32: <bkmk rel> -> <l_bracket> <loc> <r_bracket>			*/
/* 33: <bkmk rel> -> <l_bracket> <locbk> : <locbk> <r_bracket>		*/
/* 34: <loc> -> R <sint> C <sint>					*/
/* 35: <loc> -> R <sint>						*/
/* 36: <loc> -> C <sint>						*/
/* 37: <loc> -> R C <sint>						*/
/* 38: <loc> -> R <sint> C						*/
/* 39: <loc> -> R C							*/
/* 40: <loc> -> R							*/
/* 41: <loc> -> C							*/
/* 42: <locbk> -> loc							*/
/* 43: <locbk> -> BOOKMARK						*/
/* 44: <sint> -> INTEGER						*/
/* 45: <sint> -> - INTEGER						*/
/* 46: <sint> -> + INTEGER						*/
/* 47: <texp> -> <tbl ref>						*/
/* 48: <texp> -> <exp>							*/
/* 49: <cur_num> -> PRE_CURRENCY NUMBER					*/
/* 50: <cur_num> -> NUMBER POST_CURRENCY				*/
/* 51: <cur_num> -> PRE_CURRENCY INTEGER				*/
/* 52: <cur_num> -> INTEGER POST_CURRENCY				*/
/* 53: <ucn> -> <cur_num>						*/
/* 54: <ucn> -> NUMINPAREN						*/
/* 55: <ucn> -> NUMBER							*/
/* 56: <ucn> -> INTEGER							*/
/* 57: <ucnum> -> <ucnum> <ucn>						*/
/* 58: <ucnum> -> <ucn>							*/


/*------------ Created by CSD YACC (IBM PC) from "fldexp.grm" ----------------*/
csconst EP    rgep[]     = {
	rgepDef};


csconst short mpictcsNew[] = {
	mpictcsNewDef};


csconst short mpcsict[]  = {
	mpcsictDef};


csconst short rgdict[]   = {
	rgdictDef};


csconst short mppddigt[] = {
	mppddigtDef};


csconst CHAR  mppdcSym[] = {
	mppdcSymDef};


csconst short mpcstiCheck[]     = {
	mpcstiCheckDef};


csconst short mpcspd[]   = {
	mpcspdDef};


csconst FM    mppdfm[]   = {
	mppdfmDef};


/* For Computed Field Lexical Analyzer. */
/* The list of function names is defined in strtbl.c. */
/* If you change either grpfReservedDef,              */
/* or any of szFE's you must recompile fltexp.c       */
/* rgrwDef must be in an ascending order of its text. */
/* Z Y | X W V U | T S R Q | P O N M | L K J I | H G F E | D C B A */
/*     |         | * * *   | * * * * |       * |     *   | * *   * */
/* 0 0 | 0 0 0 0 | 1 1 1 0 | 1 1 1 1 | 0 0 0 1 | 0 0 1 0 | 1 1 0 1 */
/*  0  |    0    |    E    |    F    |    1    |    2    |    D    */
csconst long grpfReserved      = 0x000EF12DL;

/* In the following szFE's, \\ is a place holder for      */
/* a token that caused an error.  Computed field          */
/* evaluator will replace it with the first cchErrTknMax  */
/* characters of the offending token.   If you need to    */
/* use a backslash in an error message, change chFEPH     */
/* in fltexp.h and recompile fltexp.c.                    */
/* cchFEMax is the number of characters in the logest     */
/* szFE...Def's including the null terminator.            */
/* Each error message must start with chFEErrMsg defined  */
/* in ch.n.                                               */
csconst CHAR rgszFE[][] = {
	SzSharedKey("!Expression Too Complex.",ExpressionComplex),
			SzSharedKey("!Syntax Error.",FESyntaxError),
			SzSharedKey("!Zero Divide.",ZeroDivide),
			SzSharedKey("!Invalid Character Setting.",InvalidCharacterSetting),
			SzSharedKey("!Empty Expression.",EmptyExpression),
			SzSharedKey("!Unexpected End of Expression.",UnexpectedEndOfExpression),
			SzSharedKey("!Number Too Large.",NumberTooLarge),
			SzSharedKey("!Number Too Small.",NumberTooSmall),
			SzSharedKey("!Number Too Large To Format.",NumberTooLargeToFormat),
			SzSharedKey("!Exponent Not an Integer.",ExponentNotInteger),
			SzSharedKey("!Call Microsoft Customer Support.",CallMSCustomerSupport),
			SzSharedKey("!Table Index Cannot be Zero.",TableIndexCannotBeZero),
			SzSharedKey("!The Expression Not In Table.",ExpressionNotInTable),
			SzSharedKey("!Mixed Relative/Absolute Cell References.",MixedCellReferences),
			SzSharedKey("!Illegal Index Combination.",IllegalIndexCombination),
			SzSharedKey("!Index Too Large.",IndexTooLarge),
			SzSharedKey("!Out of Memory.",NoMoreMemory),
			SzSharedKey("!Missing Operator.",MissingOperator),
			SzSharedKey("!Syntax Error, \\.",SyntaxErr),
			SzSharedKey("!Undefined Bookmark, \\.",UndefinedBookmark),
			SzSharedKey("!Error at \\.",ErrorAt),
			SzSharedKey("!\\ Is Not In Table.",IsNotInTable)
};


/* cchFEMax may change for localized versions - cchFEMaxIntl in inter.h */

#define cchFEMax	Intl(42, cchFEMaxIntl)

#define cchFEFMax   ((cchNumMax < cchFEMax + cchErrTknMax) ? \
                            cchFEMax + cchErrTknMax : cchNumMax)

/* Following stuff (definitions and structures) implements a code-reducing
  mechanism for the big switch mechanism in FParseExp. many of the branches
  require checking of FEVUndefined for pev [iev] for various iev's.
  these tables allow the checking to be done up front in a table-driven manner */

#define pdCheckMax	49
#define bievNil		255

csconst char mppdbiev [pdCheckMax] = 
{
	255, 255, 255,  12,  12,  12,  12,  12,  12, 12, 	/*  0 -  9 */
	12,  12,  12,  12,  2,   255, 255, 0,   255, 4, 	/* 10 - 19 */
	9,   255, 255, 255, 4,   0,   0,   255, 255, 255, 	/* 20 - 29 */
	12,  15,  255, 6,   6,   2,   2,   4,   2,   255, 	/* 30 - 39 */
	255, 255, 255, 0,   255, 255, 255, 255, 0			/* 40 - 48 */
};


/* mapped into from above, contains ciev + ciev iev's to check */
csconst char grpievCheck [] = 
{
	1, 1,							/* biev == 0 */
	1, 2,							/* biev == 2 */
	1, 3,							/* biev == 4 */
	2, 2, 4,						/* biev == 6 */
	2, 3, 5, 						/* biev == 9 */
	2, 1, 3,						/* biev == 12 */
	3, 1, 3, 5						/* biev == 15 */
};


extern struct ITR vitr;
extern ENV	  *penvMathError;
extern struct SEL selCur;
extern struct PAP vpapFetch;
extern struct TAP vtapFetch;
extern struct CA  caTap;
extern struct CA  caTable;
extern struct TCC vtcc;


#define fLexNumInParen	0x0001
#define fLexInUnrfnc	0x0002
#define fLexRowCol	0x0004

/*   parser for yacc output  */
/* %%Function:FParseExp %%Owner:bryanl */
FParseExp(pfeb, ppes)
FEB	*pfeb;
PES	*ppes;
{
	register CHAR  *pcs;
	register short  cs;
	register EV    *pev;
	int             tiCur;
	int             csNew;
	int             ict, idict;
	short           pd, pdApp;
	EV              ev;
	EV				*pevTop;
	int             cErrors;
	short           cErrRec;
	int		    grpfLex = 0;
	int		    fi;
	EP FAR         *pep;
	BOOL            fEvResValid;
	ENV		    *penvSv1, *penvSv2;
	int		    merr;
	CHAR	   *pcsMax;
	BOOL	    fParsed;
	int		    iRow, iCol;
	int		    doc;
	CP		    cp, cpFirstTbl;
	TR2		   *ptr21, *ptr22;
	int		    fRel, fBkmk;
	ENV		    env1, env2;

	if ((merr = SetJmp(&env1)) != 0)
		{
#ifdef BRYANL
		CommSzSz( SzShared( "Math Excp SetJmp reached!!"),szEmpty );
#endif
		goto lblMathExcp;
		}
	else
		{
		PushMPJmpEnv(env1, penvSv1);
		}

	if (!fInitialized)
		InitMath();

	InitLkAhd(pfeb);
	cs       = 0;
	pfeb->ti = tiNull;
	pfeb->fNoImplicitSubCalc = fFalse;
	cErrors  = 0;
	cErrRec  = 0;

	if (ppes->iNest >= iNestMax)
		{
#ifdef BRYANL
		CommSzSz( SzShared( "Nesting levels exceeded!!"),szEmpty );
#endif
		goto lblOvfl;
		}

	pcsMax = &(ppes->rgcs[icsMax]);

	pcs = &(ppes->rgcs[ppes->icsMac - 1]);
	pev = &(ppes->rgev[ppes->icsMac - 1]);

lblPush:    /* put a state and value onto the stack */
#ifdef SHOWSTACK
	CommSzNum(SzShared("State: "), cs);
#endif
	if (++pcs >= pcsMax)
		{
lblOvfl:
		(pfeb->evResult).iszErrMsg = iszStackOvfl;
		fParsed = fFalse;
		goto lblAccept;
		}
	*pcs = cs;
	++pev;
	pev [0] = ev;

lblNewCs:
	ict = mpcsict[cs];

/* if there is a state transformation we can perform without looking at
   any more tokens, go perform it */

	if (ict <= ICtDef)
		{
		goto lblDefault;
		}

/* get the next token */
	if (pfeb->ti < tiEOI)
		{
		pfeb->ti = (*pfeb->pfnTiLex)(grpfLex, pfeb);
		Assert(pfeb->ti >= tiEOI);
		}

/* if this token means there is now some state transformation to perform, 
   go do it. Note weird Hungarian: add token # to current ict gives
   next ict. That's how yacc does things.  Its variables have so many
   different uses that orderly Hungarian is (I think) impossible */

	ict += pfeb->ti;
	if ((uns) ict >= ictMax)
		goto lblDefault;

#ifdef BRYANL
		{
		int rgw [5];
		int tiCheck = mpcstiCheck [mpictcsNew [ict]];

		rgw [0] = pfeb->ti;
		rgw [1] = cs;
		rgw [2] = mpictcsNew [ict];
		rgw [3] = ict;
		rgw [4] = pev - &ppes->rgev [0];
		CommSzRgNum( SzShared( "Token Pass (ti,csOld,csNew,ict,iev): "), rgw, 5 );
		if (pfeb->ti != tiCheck)
			CommSzNum( SzShared( "  WARNING: ti != tiCheck, tiCheck = "), tiCheck );
		}
#endif

	if (mpcstiCheck[csNew = mpictcsNew[ict]] == pfeb->ti)
		{
		/* valid shift */
		pfeb->ti = tiNull;
		ev = pfeb->evLex;
		cs = csNew;
		if (cErrRec > 0)
			{
			--cErrRec;
			}
		goto lblPush;
		}

lblDefault:
		/* default cs action */
#ifdef BRYANL
		{
		int pd = mpcspd [cs];

		switch (pd)
			{
		default:
			CommSzNum( SzShared( "  Apply pd = "), pd );
			break;
		case pdError:
			CommSzNum( SzShared( "  Apply pdError = "), pd );
			break;
		case pdException:
			CommSzNum( SzShared( "  Apply pdException = "), pd );
			break;
			}
		}
#endif
	if ((pd = mpcspd[cs]) == pdException)
		{
		if (pfeb->ti < tiEOI)
			{
			pfeb->ti = (*pfeb->pfnTiLex)(grpfLex, pfeb);
			Assert(pfeb->ti >= tiEOI);
			}

		/* look through exception table */
		for (pep = rgep; (pep->ti != tiNull) || (pep->cs != cs) ; pep++)
			;

		for (pep++; pep->ti >= 0; pep++)
			if (pep->ti == pfeb->ti)
				break;
		if ((pd = pep->cs) < 0)
			{
			fParsed = fTrue;
			goto lblAccept;   /* accept */
			}
		}

	if (pd == pdError)
		{
		/* error */
		/* error ... attempt to resume parsing */
		fEvResValid = fFalse;
		switch (cErrRec)
			{
		case 0:   /* brand new error */
			fEvResValid = fTrue;
			(pfeb->evResult).iszErrMsg = iszSyntaxErrTkn;
			bltbyte(pfeb->szToken, &((pfeb->evResult).szErrTkn[0]),
					cchErrTknMax);
errlab: /* Whoever jumps to errlab better setup fEvResValid. */
			++cErrors;
		case 1:
		case 2:
			/* incompletely recovered error ... try again */
			cErrRec = 3;

			/* find a state where "error" is a legal shift action */
			while ( pcs >= &(ppes->rgcs[ppes->icsMac]) )
				{
				ict = mpcsict[*pcs] + tiERRCODE;
				if (ict >= 0 && tiCur < ictMax &&
						mpcstiCheck[mpictcsNew[ict]] == tiERRCODE)
					{
					cs = mpictcsNew[ict];  /* simulate a shift of "error" */
					ev.iszErrMsg = iszSyntaxErrTkn;
					bltbyte(pfeb->szToken, &(ev.szErrTkn[0]), cchErrTknMax);
					goto lblPush;
					}

				/* the current pcs has no shift on "error", pop stack */

#ifdef DBGYXY
				CommSzNum(SzShared("Error recovery pops state: "), *pcs);
				CommSzNum(SzShared("and uncovers: "), pcs[-1]);
#endif
				--pcs;
				--pev;
				}

			/* there is no state on the stack with an error shift ... abort */

			if (!fEvResValid)
				{
				/*... and we don't have error message set up in evResult. */
				(pfeb->evResult).iszErrMsg = iszSyntaxErrTkn;
				bltbyte(pfeb->szToken, &((pfeb->evResult).szErrTkn[0]), cchErrTknMax);
				}
lblAbort:       /* Whoever jumps in here must set up its own evResult. */
			Assert( !pfeb->fLookingAhead );
			fParsed = fFalse;
			goto lblAccept;
		case 3:
			/* no shift yet; clobber input char */
#ifdef DBGYXY
			CommSzNum(SzShared("Error recovery discards char: "), pfeb->ti);
#endif
			if (pfeb->ti == 0)
				{
				(pfeb->evResult).iszErrMsg = iszSyntaxErr;
				goto lblAbort; /* don't discard EOF, quit */
				}
			pfeb->ti = tiNull;
			goto lblNewCs;   /* try again in the same state */

			}
		}

	/* reduction by production n */
#ifdef DBGYXY
	CommSzNum(SzShared("Reduce by: "), pd);
#endif
	pcs -= mppdcSym[pd];
	pevTop = pev;
	pev -= mppdcSym[pd];
	pdApp = pd;

	/* consult goto table to find next state */
	idict = mppddigt[pd];
	ict = rgdict[idict] + *pcs + 1;
	if (ict >= ictMax || mpcstiCheck[cs = mpictcsNew[ict]] != -idict)
		{
		cs = mpictcsNew[rgdict[idict]];
		}

/* save some code: do advance checks on validity of ev's we'll need */
	if ( pdApp < pdCheckMax && mppdbiev [pdApp] != bievNil)
		{
		int biev = mppdbiev [pdApp];
		int ciev = grpievCheck [biev];
		int iev;

		while (ciev--)
			{
			iev = grpievCheck [++biev];
			if (FEVUndefined(pev [iev]))
				{
				ev = pev [iev];
				goto lblPush;
				}
			}
		}

	if ((merr = SetJmp(&env2)) != 0)
		{
		PopMPJmp(penvSv2);
		if (merr & fmerrFatal)
			{
			MathError(merr);
			}
		ev.iszErrMsg = IszFEFromMerr(merr);
		goto lblPush;
		}
	else
		{
		PushMPJmpEnv(env2, penvSv2);
		}
	ev.iszErrMsg = iszNoError;
	SynthesizePevFormat(&ev, pev, pdApp);

	/* Beginning of THE BIG SWITCH statement. */
	switch (pdApp)
		{
		int iev;

	case 1: /*  1: <fexp> -> \ = <exp> */
		pfeb->evResult = pev [3];
		break;
	case 2: /*  2: <fexp> -> = <exp> */
		pfeb->evResult = pev [2];
		break;
	case 3: /*  3: <exp> -> <exp> = <exp> */
		fi = fiEQ;
		goto lblApply;
	case 4: /*  4: <exp> -> <exp> < <exp> */
		fi = fiLT;
		goto lblApply;
	case 5: /*  5: <exp> -> <exp> <= <exp> */
		fi = fiLE;
		goto lblApply;
	case 6: /*  6: <exp> -> <exp> > <exp> */
		fi = fiGT;
		goto lblApply;
	case 7: /*  7: <exp> -> <exp> >= <exp> */
		fi = fiGE;
		goto lblApply;
	case 8: /*  8: <exp> -> <exp> <> <exp> */
		fi = fiNE;
		goto lblApply;
	case 9: /*  9: <exp> -> <exp> + <exp> */
		fi = fiPLUS;
		goto lblApply;
	case 10: /* 10: <exp> -> <exp> - <exp> */
		fi = fiMINUS;
		goto lblApply;
	case 11: /* 11: <exp> -> <exp> * <exp> */
		fi = fiMULTIPLY;
		goto lblApply;
	case 12: /* 12: <exp> -> <exp> / <exp> */
		fi = fiDIVIDE;
		goto lblApply;
	case 13: /* 13: <exp> -> <exp> ^ <exp> */
		fi = fiEXP;
lblApply:
		FApplyInfixFi(fi, &(pev[1]), &(pev[3]), &ev);
		break;
	case 14: /* 14: <exp> -> - <exp> */
		LdiNum(&(pev[2].v));
		NegNum();
		StiNum(&(ev.v));
		break;
	case 15: /* 17: <exp> -> BOOL */
	case 56: /* 56: <ucn> -> INTEGER */
		CNumInt(pev[1].i);
		StiNum(&(pev[1].v));
		/* Fall through is intentional */
	case 16: /* 16: <exp> -> <ucnum> */
	case 47: /* 47: <texp> -> <tbl ref> */
	case 42: /* 42: <locbk> -> <loc> */
	case 53: /* 53: <ucn> -> <cur_num> */
	case 54: /* 54: <ucn> -> NUMINPAREN */
	case 55: /* 55: <ucn> -> NUMBER  */
	case 58: /* 58: <ucnum> -> <ucn> */
LIev1:
		iev = 1;
		goto lblCopy;
	case 17: /* 17: <exp> -> BOOKMARK */
			{
			int	dics;

		/* Share PES. */
			ppes->iNest++;
			dics = pevTop - &(ppes->rgev[ppes->icsMac]) + 1;
			ppes->icsMac += dics;

		/* Apply util/calc to the cp range to get NUM. */
			FNumFromBkmkCp(pfeb, &ev, pev[1].cpFirstBkmk, pev[1].cpLimBkmk, ppes);
			ppes->iNest--;
			ppes->icsMac -= dics;
			break;
			}
	case 18: /* 18: <exp> -> ( <exp> ) */
LIev2:
		iev = 2;
lblCopy:
		ev = pev [iev];
		break;
	case 20: /* 20: <exp> -> BINFNC ( <exp> , <exp> ) */
		/* Apply a binary function. */
		FApplyFi(pev[1].i, &(pev[3]), &(pev[5]), &ev);
		break;
	case 21: /* 21: $$21 -> epsilon */
		grpfLex |= fLexInUnrfnc; 
		break;
	case 22: /* 22: $$22 -> epsilon */
		grpfLex &= ~fLexInUnrfnc; 
		break;
	case 23: /* 23: <exp> -> UNRFNC $$21 ( $$22 <exp> ) */
		if (pev[1].i != fiDEFINED && FEVUndefined(pev[5]))
			{
LIev5:
			iev = 5;
			goto lblCopy;
			}
		/* Apply a unary function. */
		FApplyFi(pev[1].i, &(pev[5]), NULL, &(ev));
		break;
	case 24: /* 24: <exp> -> IF ( <exp> , <exp> , <exp> ) */
			{
			NUM	numT;

			num0;
			StiNum(&numT);
			if (FApxNE(WApproxCmpPnum(&(pev[3].v), &numT)))
				goto LIev5;
			iev = 7;
			goto lblCopy;
			}
	case 28: /* 28: <l_bracket> -> [ */
		grpfLex |= fLexRowCol;  
		break;
	case 29: /* 29: <r_bracket> -> ] */
		grpfLex &= ~fLexRowCol;  
		break;
	case 51: /* 51: <cur_num> -> PRE_CURRENCY INTEGER */
		Assert(!FEVUndefined(pev[2]));
		CNumInt(pev[2].i);
		StiNum(&(pev[2].v));
		/* Fall through is intentional. */
	case 49: /* 49: <cur_num> -> PRE_CURRENCY NUMBER  */
		iev = 2;
		goto lblCurrency;
	case 52: /* 52: <cur_num> -> INTEGER POST_CURRENCY */
		Assert(!FEVUndefined(pev[1]));
		CNumInt(pev[1].i);
		StiNum(&(pev[1].v));
		/* Fall through is intentional. */
	case 50: /* 50: <cur_num> -> NUMBER POST_CURRENCY  */
		iev = 1;
lblCurrency:
		if (FEVUndefined(pev[iev]))
			goto lblCopy;
		ev = pev [iev];
		ev.fCurrency = fTrue;
		break;
	case 19: /* 19: <exp> -> RDCFNC ( <tbl ref> ) */
		Assert(!FEVUndefined(pev[1]));
		ApplyReduction(&ev, pev[1].i, pev[3].hrgtre, pev[3].itreMac,
				ppes, pevTop);
		FreePhhacHtre(&(pfeb->hhac), pev[3].hrgtre);
		break;

	case 25: /* 25: <tbl ref> -> <bkmk_rel> */
		PutTreInPev(&ev, fTrue, &(pev[1].tr2), NULL, &(pfeb->hhac));
		break;

	case 26: /* 26: <tbl ref> -> BOOKMARK <l_bracket> <r_bracket> */
			{
			TR2	tr2;

			if (!FBkmkInTable(doc = DocMother(pfeb->pffb->fvb.doc),
					cp = pev[1].cpFirstBkmk, &ev,
					pev[1].ibkf))
				break;
			CacheTable(doc, cp);
			tr2.doc = doc;
			tr2.cpTable = caTable.cpFirst;
		/* It's a whole table. */
			tr2.iRowMin = tr2.iRowLast = tr2.iColMin = tr2.iColLast
					= iStar;
			tr2.fRel = fFalse;
			PutTreInPev(&ev, fTrue, &tr2, NULL, &(pfeb->hhac));
			break;
			}
	case 27: /* 27: <tbl ref> -> <texp> , <texp> */
		if (FEVUndefined(pev[1]))
			{
			if (!FEVUndefined(pev[3]))
				FreePhhacHtre(&(pfeb->hhac), pev[3].hrgtre);
			goto LIev1;
			}
		if (FEVUndefined(pev[3]))
			{
			FreePhhacHtre(&(pfeb->hhac), pev[1].hrgtre);
LIev3:
			iev = 3;
			goto lblCopy;
			}
		/* Combine two hrgtre's */
		MergeTrePev(&ev, &pev[1], &pev[3], &(pfeb->hhac));
		break;
	case 48: /* 48: <texp> -> <exp> */
		PutTreInPev(&ev, fFalse, &(pev[1].v),
				&(pev[1].cDigBlwDec), &(pfeb->hhac));
		break;
	case 30: /* 30: <bkmk rel> -> BOOKMARK <l_bracket> <loc> <r_bracket>*/
		if (!FBkmkInTable(doc = DocMother(pfeb->pffb->fvb.doc),
				cp = pev[1].cpFirstBkmk, &ev,
				pev[1].ibkf))
			break;
		ev.tr2 = pev [3].tr2;
		ev.tr2.doc = doc;
		ev.tr2.cpTable = CpFirstTableFromDocCp(doc, cp);
		if (ev.tr2.fRel)
			{
			/* ??? Mystery Function........ */
			AbsRefPtr2(&ev.tr2, doc, cp);
			}
		break;
	case 32: /* 32: <bkmk rel> -> <l_bracket> <loc> <r_bracket> */
		if (pfeb->cpFirstTable == cpNil)
			{
			ev.iszErrMsg = iszNotInTable;
			break;
			}
		if (FEVUndefined(pev[2]))
			goto LIev2;
		ev.tr2 = pev [2].tr2;
		ev.tr2.doc = pfeb->pffb->fvb.doc;
		ev.tr2.cpTable = pfeb->cpFirstTable;
		if (ev.tr2.fRel)
			{
			AbsRefPtr2(&ev.tr2, ev.tr2.doc, pfeb->cpFirstRel);
			}
		break;
	case 31:
		/* <bkmk rel> -> BOOKMARK <l_bracket> <locbk> :
					    <locbk> <r_bracket>*/
		doc = DocMother(pfeb->pffb->fvb.doc);
		cp = pev[1].cpFirstBkmk;
		cpFirstTbl = CpFirstTableFromDocCp(doc, cp);
		if (!FBkmkInTable(doc, cp, &ev, pev[1].ibkf))
			{
			break;
			}
		ptr21 = &(pev[3].tr2);
		ptr22 = &(pev[5].tr2);
		goto lblCellRange;
	case 33:
		/* 33: <bkmk rel> -> <l_bracket> <locbk> : <locbk> <r_bracket>*/
		doc = pfeb->pffb->fvb.doc;
		cp = pfeb->cpFirstRel;
		cpFirstTbl = pfeb->cpFirstTable;
		if (pfeb->cpFirstRel == cpNil)
			{
			ev.iszErrMsg = iszNotInTable;
			break;
			}
		ptr21 = &(pev[2].tr2);
		ptr22 = &(pev[4].tr2);
lblCellRange:
		if (ptr21->fBkmk != ptr22->fBkmk)
			{
			/* Needs to match TR2 of Bookmark to the other guy. */
			if (ptr21->fBkmk)
				{
				MatchRhoPtr2(ptr21, ptr22);
				}
			else  if (ptr22->fBkmk)
				{
				MatchRhoPtr2(ptr22, ptr21);
				}
			}
		if (ptr21->fRel)
			{
			AbsRefPtr2(ptr21, doc, cp);
			}
		if (ptr22->fRel)
			{
			AbsRefPtr2(ptr22, doc, cp);
			}
		if (RhoFromPtr2(ptr21) != RhoFromPtr2(ptr22))
			{
			ev.iszErrMsg = iszIllegalIxComb;
			break;
			}
		ev.tr2.doc = doc;
		ev.tr2.cpTable = cpFirstTbl;
		ev.tr2.iRowMin = min(ptr21->iRowMin, ptr22->iRowMin);
		ev.tr2.iColMin = min(ptr21->iColMin, ptr22->iColMin);
		ev.tr2.iRowLast = max(ptr21->iRowMin, ptr22->iRowMin);
		ev.tr2.iColLast = max(ptr21->iColMin, ptr22->iColMin);
		break;
	case 34: /* 34: <loc> -> R <sint> C <sint> */
		if (pev[4].loc > itcMax)
			{
			ev.iszErrMsg = iszIndexTooLarge;
			break;
			}
		if (pev[2].fRel != pev[4].fRel)
			{
			ev.iszErrMsg = iszMixedRelAbs;
			break;
			}
		iRow = pev[2].loc;
		iCol = pev[4].loc;
		fRel = pev[2].fRel;
		fBkmk = fFalse;
		goto lblPutRC;
	case 35: /* 35: <loc> -> R <sint> */
		iRow = pev[2].loc;
		iCol = iStar;
		fRel = pev[2].fRel;
		fBkmk = fFalse;
		goto lblPutRC;
	case 36: /* 36: <loc> -> C <sint> */
		if (pev[2].loc > itcMax)
			{
			ev.iszErrMsg = iszIndexTooLarge;
			break;
			}
		iRow = iStar;
		iCol = pev[2].loc;
		fRel = pev[2].fRel;
		fBkmk = fFalse;
		goto lblPutRC;
	case 37: /* 37: <loc> -> R C <sint> */
		if (!pev[3].fRel)
			{
			ev.iszErrMsg = iszSyntaxErrTkn;
			ev.szErrTkn[0] = chRow;
			ev.szErrTkn[1] = '\0';
			break;
			}
		iRow = 0;
		iCol = pev[3].loc;
		fRel = fTrue;
		fBkmk = fFalse;
		goto lblPutRC;
	case 38: /* 38: <loc> -> R <sint> C */
		if (!pev[2].fRel)
			{
			ev.iszErrMsg = iszSyntaxErrTkn;
			ev.szErrTkn[0] = chCol;
			ev.szErrTkn[1] = '\0';
			break;
			}
		iRow = pev[2].loc;
		iCol = 0;
		fRel = fTrue;
		fBkmk = fFalse;
		goto lblPutRC;
	case 39: /* 39: <loc> -> R C */
		iRow = iCol = 0;
		fRel = fTrue;
		fBkmk = fFalse;
lblPutRC:
		ev.tr2.iRowLast = ev.tr2.iRowMin = iRow;
		ev.tr2.iColLast = ev.tr2.iColMin = iCol;
		ev.tr2.fRel = fRel;
		ev.tr2.fBkmk = fBkmk;
		break;

	case 40: /* 40: <loc> -> R */
		iRow = 0;
		iCol = iStar;
		fRel = fTrue;
		fBkmk = fFalse;
		goto lblPutRC;

	case 41: /* 41: <loc> -> C */
		iRow = iStar;
		iCol = 0;
		fRel = fTrue;
		fBkmk = fFalse;
		goto lblPutRC;

	case 43: /* 43: <locbk> -> BOOKMARK */
		doc = DocMother(pfeb->pffb->fvb.doc);
		if (!FBkmkInTable(doc, pev[1].cpFirstBkmk, &ev, pev[1].ibkf))
			{
			break;
			}
		Tr2FromPevDocCp(&ev, doc, pev[1].cpFirstBkmk, rhoRowCol);
		break;
	case 44: /* 44: <sint> -> INTEGER */
		iev = 1;
		fRel = fFalse;
		goto lblIndex;
	case 45: /* 45: <sint> -> - INTEGER */
	case 46: /* 46: <sint> -> + INTEGER */
		iev = 2;
		fRel = fTrue;
lblIndex:
		Assert(!FEVUndefined(pev[iev]));
		if (pev[iev].i == iStar)
			{
			ev.iszErrMsg = iszIndexTooLarge;
			break;
			}
		if (pev[iev].i == 0)
			{
			goto lblIlIndex;
			}
		ev.fRel = fRel;
		ev.loc = pev[iev].i;
		if (pdApp == 45)
			{
			ev.loc *= -1;
			}
		break;
lblIlIndex:
		ev.iszErrMsg  = iszIllegalIndex;
		break;
	case 57: /* 57: <ucnum> -> <ucnum> <ucn> */
		if (pfeb->fSubCalc)
			{
			FApplyInfixFi(fiPLUS, &(pev[1]), &(pev[2]), &ev);
			}
		else
			{
			ev.iszErrMsg = iszMissingBinOp;
			}
		break;
#ifdef DEBUG
	default:
			{
			Assert(fFalse);
			break;
			}
#endif
		/* End of actions */
		}
	PopMPJmp(penvSv2);
	goto lblPush;  /* stack new state and value */

lblMathExcp:
	Assert(merr != 0);
	(pfeb->evResult).iszErrMsg = IszFEFromMerr(merr);
	fParsed = fFalse;
lblAccept:
	PopMPJmp(penvSv1);
	Assert( !pfeb->fLookingAhead );
	return fParsed;
}


/* S Y T H E S I Z E  P E V  F O R M A T */
/* Performs an appropriate formatting information composition
   for a given pd as specified by mppdfm. */
/* %%Function:SynthesizePevFormat %%Owner:bryanl */
NATIVE SynthesizePevFormat(pev, pevStack, pdApp)
EV     *pev;
EV     *pevStack;
short   pdApp;
{
	FM      fm;

	bltbyte(&mppdfm[pdApp - 1], &fm, sizeof(FM));
	if (fm.fmCDig != fmNoop)
		{
		switch (fm.fmCDig)
			{
		case fmClr:
			pev->cDigBlwDec = 0;
			break;
		case fmSpec:
			break;
		case fmCopy:
			pev->cDigBlwDec = pevStack[fm.iOp1].cDigBlwDec;
			break;
		case fmGrtr:
			pev->cDigBlwDec =
					max(pevStack[fm.iOp1].cDigBlwDec,
					pevStack[fm.iOp2].cDigBlwDec);
			break;
#ifdef DEBUG
		default:
			Assert(fFalse);
			break;
#endif /* DEBUG */
			}
		switch (fm.fmGrpf)
			{
		case fmClr:
			pev->gfi = 0;
			break;
		case fmSpec:
			break;
		case fmCopy:
			pev->gfi = pevStack[fm.iOp1].gfi;
			break;
		case fmOr:
			pev->gfi =
					pevStack[fm.iOp1].gfi |
					pevStack[fm.iOp2].gfi;
			break;
#ifdef DEBUG
		default:
			Assert(fFalse);
			break;
#endif /* DEBUG */
			}
		}
}


/* I S Z F E  F R O M  M E R R */
/* Translate Math Pack error code to an appropriate error message
   index. */
/* %%Function:IszFEFromMerr %%Owner:bryanl */
IszFEFromMerr(merr)
int	merr;
{
	switch (merr & (fmerrFatal | fmerrNonfatal))
		{
	case fmerrOver:
		return iszFEOverflow;
	case fmerrUnder:
		return iszFEUnderflow;
	case fmerrDiv0:
		return iszZeroDiv;
	case fmerrTrans:
		return iszFETrans;
	case fmerrStkOver:
		return iszStackOvfl;
	case fmerrStkUnder:
		return iszFEStUnderflow;
#ifdef DEBUG
	default:
		Assert(fFalse);
#endif /* DEBUG */
		}
}


/* F  B K M K  I N  T A B L E */
/* Is a given bookmark specified by doc/cp in table. */
/* If pev is NULL, ibkf is not used and an error message */
/* is not put in *pev. */
/* %%Function:FBkmkInTable %%Owner:bryanl */
BOOL FBkmkInTable(doc, cp, pev, ibkf)
int doc;
CP cp;
EV *pev;
int ibkf;
{
	CHAR HUGE *hpch;
	int	ichTerm;

	if (!FInTableDocCp(doc, cp))
		{
		if (pev != NULL)
			{
			pev->iszErrMsg = iszNotTableRef;
			hpch = HpstFromSttb(PdodDoc(doc)->hsttbBkmk, ibkf);
			ichTerm = min(cchErrTknMax - 1, (int) (*hpch));
			bltbh(hpch + 1, &(pev->szErrTkn[0]), ichTerm);
			pev->szErrTkn[ichTerm] = '\0';
			}
		return fFalse;
		}
	return fTrue;
}


/* C P  F I R S T  T A B L E  F R O M  D O C  C P */
/* Given doc/cp which is in table, returns the cpFirst of the table. */
/* %%Function:CpFirstTableFromDocCp %%Owner:bryanl */
CP CpFirstTableFromDocCp(doc, cp)
int doc;
CP  cp;
{
	CacheTable(doc, cp);
	return caTable.cpFirst;
}


/* P U T  T R E  I N  P E V */
/* Given either TR2 or NUM, put it in hrgtre of pev. */
/* hrgtre of pev is newly allocated. */
/* %%Function:PutTreInPev %%Owner:bryanl */
PutTreInPev(pev, fTR2, pch, pcgfi, phhac)
EV	*pev;
BOOL	fTR2;
CHAR	*pch;
CGFI	*pcgfi;
HAC	***phhac;
{
	TRE	*ptre;

	if ((pev->hrgtre = (TRE **) (HTreAllocCw(cTREAlloc, phhac))) == hNil)
		{
		pev->iszErrMsg = iszOutOfMemory;
		}
	else
		{
		pev->itreMac = 1;
		pev->itreMax = cTREAlloc;
		ptre = (TRE *) (*pev->hrgtre);
		ptre->fTR2 = fTR2;
		if (fTR2)
			{
			bltbyte(pch, &(ptre->tr2), sizeof(TR2));
			}
		else
			{
			bltbyte(pch, &(ptre->num), sizeof(NUM));
			ptre->cDigBlw = pcgfi->cDigBlwDec;
			ptre->gfi = pcgfi->gfi;
			}
		}
}


/* M E R G E  T R E  P E V */
/* Given two hrgtre's stored in *pev1 and *pev2, merge them and
   store in *pevDest. */
/* %%Function:MergeTrePev %%Owner:bryanl */
MergeTrePev(pevDest, pev1, pev2, phhac)
EV	*pevDest, *pev1, *pev2;
HAC	***phhac;
{
	int	iAv1, iAv2;
	TRE	**hrgtre;
	int	itreMax;

	iAv1 = pev1->itreMax - pev1->itreMac;
	iAv2 = pev2->itreMax - pev2->itreMac;

	if (pev2->itreMac <= iAv1)
		{
		hrgtre = pev1->hrgtre;
		/* Copy from 2 to 1. */
		bltbyte(*(pev2->hrgtre),
				(*hrgtre) + pev1->itreMac,
				sizeof(TRE) * pev2->itreMac);
		itreMax = pev1->itreMax;
		FreePhhacHtre(phhac, pev2->hrgtre);
		}
	else  if (pev1->itreMac <= iAv2)
		{
		hrgtre = pev2->hrgtre;
		/* Copy from 1 to 2. */
		bltbyte(*(pev1->hrgtre),
				(*hrgtre) + pev2->itreMac,
				sizeof(TRE) * pev1->itreMac);
		itreMax = pev2->itreMax;
		FreePhhacHtre(phhac, pev1->hrgtre);
		}
	else
		{
		BOOL	fFail;
		TRE	*ptre;

		fFail = fFalse;
		itreMax = (((pev1->itreMac + pev2->itreMac) / cTREAlloc) + 1)
				* cTREAlloc;
		if ((hrgtre = HTreAllocCw(itreMax, phhac)) == hNil)
			{
			pevDest->iszErrMsg = iszOutOfMemory;
			fFail = fTrue;
			}
		else
			{
			bltbyte(*(pev1->hrgtre), (ptre = *hrgtre),
					sizeof(TRE) * pev1->itreMac);
			bltbyte(*(pev2->hrgtre), ptre + pev1->itreMac,
					sizeof(TRE) * pev2->itreMac);
			}
		FreePhhacHtre(phhac, pev1->hrgtre);
		FreePhhacHtre(phhac, pev2->hrgtre);
		if (fFail)
			{
			return;
			}
		}
	Assert(pev1->itreMac + pev2->itreMac <= itreMax);
	pevDest->itreMax = itreMax;
	pevDest->itreMac = pev1->itreMac + pev2->itreMac;
	pevDest->hrgtre  = hrgtre;
}


/* H T R E  A L L O C  C W */
/* Allocate hrgtre and put them on *phhac. */
/* %%Function:HTreAllocCw %%Owner:bryanl */
TRE **HTreAllocCw(ctre, phhac)
int	ctre;
HAC	***phhac;
{
	TRE	**htre;
	HAC	**hhac;

	if ((htre = HAllocateCw(cwTRE * ctre)) != hNil)
		{
		if ((hhac = HAllocateCw(cwHAC)) == hNil)
			FreePh(&htre);
		else
			{
			(*hhac)->hhacNext = *phhac;
			(*hhac)->h = htre;
			*phhac = hhac;
			}
		}
	return htre;
}


/* F R E E  P H H A C  H T R E */
/* Free the specified htre and remove them from *phhac. */
/* %%Function:FreePhhacHtre %%Owner:bryanl */
FreePhhacHtre(phhac, htre)
HAC	***phhac;
TRE	**htre;
{
	HAC	**hhacFree;

	for (; *phhac != hNil; phhac = &(**phhac)->hhacNext)
		{
		if (htre == (**phhac)->h)
			{
			hhacFree = *phhac;
			*phhac = (**phhac)->hhacNext;
			FreeH(htre);
			FreeH(hhacFree);
			return;
			}
		}
	Assert(fFalse);
}


/* F R E E  P H H A C */
/* Free everything on *phhac. */
/* %%Function:FreePhhac %%Owner:bryanl */
FreePhhac(phhac)
HAC	***phhac;
{
	while (*phhac != hNil)
		{
		FreePhhacHtre(phhac, (**phhac)->h);
		}
}


/* A B S  R E F  P T R 2 */
/* Refer to the last page of "Maintaining Field Expression Evaluator". */
/* %%Function:AbsRefPtr2 %%Owner:bryanl */
AbsRefPtr2(ptr2, doc, cp)
TR2	*ptr2;
int	doc;
CP	cp;
{
	EV	ev;

	ev.iszErrMsg = iszNoError;
	Tr2FromPevDocCp(&ev, doc, cp, rhoRowCol);
	Assert(!FEVUndefined(ev));
	if (ptr2->iRowMin != iStar)
		{
		ptr2->iRowLast = ptr2->iRowMin =
				max(1, ptr2->iRowMin + ev.tr2.iRowMin);
#ifdef BRYANL
		CommSzNum( SzShared( " Row XFORM result: "), ptr2->iRowLast );
#endif
		}
	if (ptr2->iColMin != iStar)
		{
		ptr2->iColLast = ptr2->iColMin =
				max(1, ptr2->iColMin + ev.tr2.iColMin);
#ifdef BRYANL
		CommSzNum( SzShared( " Col XFORM result: "), ptr2->iColLast );
#endif
		}
}


/* M A T C H  R H O  P T R 2 */
/* Matches the shape of *ptr2 to the one of *ptr2Templt. */
/* %%Function:MatchRhoPtr2 %%Owner:bryanl */
MatchRhoPtr2(ptr2, ptr2Templt)
TR2	*ptr2, *ptr2Templt;
{
	Assert(RhoFromPtr2(ptr2Templt) != rhoStarStar);
	if (ptr2Templt->iRowMin == iStar)
		{
		ptr2->iRowMin = iStar;
		}
	if (ptr2Templt->iColMin == iStar)
		{
		ptr2->iColMin = iStar;
		}
}


/* T R 2  F R O M  P E V  D O C  C P */
/* Given a doc/cp in a table, get TR2 of the shape rho. */
/* %%Function:Tr2FromPevDocCp %%Owner:bryanl */
Tr2FromPevDocCp(pev, doc, cp, rho)
EV *pev;
int doc;
CP  cp;
int rho;
{
	int	iRow, iCol;
	CP	cpFirstTable, cpFirst;
	CP	cpSave;
	TR2	*ptr2;

	CacheTable(doc, cp);
	cpFirst = cpFirstTable = caTable.cpFirst;

	for (iRow = 1; ; iRow++, cpFirst = cpSave)
		{
		CpFirstTap(doc, cpFirst);
		cpSave = caTap.cpLim;
		if (FInCa(doc, cp, &caTap))
			{
			CacheTc(wwNil, doc, cpFirst, fFalse, fFalse);
			for (iCol = 1;
					!FInCa(doc, cp, &vtcc.ca); iCol++)
				{
				CacheTc(wwNil, doc, vtcc.ca.cpLim, fFalse, fFalse);
				}
			break;
			}
		}
	ptr2 = &pev->tr2;
	ptr2->doc	 = doc;
	ptr2->cpTable = cpFirstTable;
	ptr2->iRowLast = ptr2->iRowMin = (rho & rhoRow) ? iRow : iStar;
	ptr2->iColLast = ptr2->iColMin = (rho & rhoCol) ? iCol : iStar;
	ptr2->fRel    = fFalse;
	ptr2->fBkmk   = fTrue;
}


/* R H O  F R O M  P T R 2 */
/* Returns the shape of TR2. */
/* %%Function:RhoFromPtr2 %%Owner:bryanl */
int RhoFromPtr2(ptr2)
TR2	*ptr2;
{
	Assert((ptr2->iRowMin == ptr2->iRowLast) &&
			(ptr2->iColMin == ptr2->iColLast));
	return (((ptr2->iRowMin != iStar) ? rhoRow : rhoStarStar) |
			((ptr2->iColMin != iStar) ? rhoCol : rhoStarStar));
}



/************************ Lexical Analyzer code. ****************************/

/* lexical analysis routine */
/* This one is for Field Expression typ evaluation. */
int TiLexFPfeb( grpfLex, pfeb)
int grpfLex;
FEB    *pfeb;
{
	uns  ch;
	int  tiRet;
	int  fInteger;
	int  fHasADigit;
	int  fPercent;
	int  ich;
	int  ichMin;

	if (!(grpfLex & fLexNumInParen))
		{
		ResetTokenBuf(pfeb);
		}
	fInteger = fTrue;
	fPercent = fFalse;
	fHasADigit = fFalse;
	(pfeb->evLex).cDigBlwDec = 0;
	(pfeb->evLex).gfi        = 0;

	/* Skip leading spaces */
	ch = GetNextCh(pfeb);
	SkipWhiteSpaces(ch, pfeb);

	if (FAlphaFE(ch)) /* guarantees it's an alpha. */
		{
		if (grpfLex & fLexRowCol)
			{
			switch (ch)
				{
			case chRow:
				StartLkAhd( pfeb, &ichMin );
				ch = GetNextCh(pfeb);
				BackTrackLkAhd(pfeb, ichMin);
				if (ch == chCol || !FAlphaFE(ch))
					{
					tiRet = tiROW;
					break;
					}
				ch = chRow;
				goto lblBkmk;
			case chCol:
				StartLkAhd( pfeb, &ichMin );
				ch = GetNextCh(pfeb);
				BackTrackLkAhd(pfeb, ichMin);
				if (!FAlphaFE(ch))
					{
					tiRet = tiCOL;
					break;
					}
				ch = chCol;
			default:
				goto lblBkmk;
				}
			goto lblComReturn;
			}
		else
			{
lblBkmk:
#ifdef INTL
/* Prevent replacing currency strings with chAbbrevCur */
			pfeb->fTestForCurrency = fFalse;
#endif /* INTL */
			for ( ;; )
				{
				PutChInTokenBuf(ch, pfeb);
				StartLkAhd(pfeb, &ichMin);
				ch = GetNextCh(pfeb);
				if (FAlphaNumFE(ch) || ch == '-')
					{
					AcceptLkAhd(pfeb,ichMin);
					}
				else
					{
					BackTrackLkAhd(pfeb,ichMin);
					break;
					}
				}

			NullTerminateToken(pfeb);
			tiRet = TiClassAlphaTokenPfeb(pfeb);
#ifdef INTL
			pfeb->fTestForCurrency = fTrue;
#endif /* INTL */
			return (tiRet);
			}
		}
	else
		{
		/* because of the language limitation....... */
		if (ch == vitr.chList)
			{
			tiRet = tiCOMSEP;
			goto lblComReturn;
			}
		if (ch == vitr.chDecimal)
			{
			goto lblNoWholeDig;
			}

		switch (ch)
			{
		case chFEErrMsg:
			tiRet = ch;
			goto lblComReturn;
		case chEOI:
			ch = chFieldEndDisp;
			tiRet = tiEOI; /* end marker */
			goto lblComReturn;
		case chFieldEscape:
			tiRet = tiFLDESC;
			goto lblComReturn;
		case chAbbrevCur:
				{
				CHAR	*pch;

				for (pch = &vitr.szCurrency[0]; *pch != '\0'; pch++)
					{
					PutChInTokenBuf(*pch, pfeb);
					}
				NullTerminateToken(pfeb);
				bltbyte(&(pfeb->szToken), &(pfeb->evLex.szErrTkn),
						cchErrTknMax);
				return vitr.fCurPostfix ? tiPOST_CURRENCY : tiPRE_CURRENCY;
				}
		case '=':
			tiRet = tiEQ;
			goto lblComReturn;
		case '<':
			PutChInTokenBuf(ch, pfeb);
			StartLkAhd(pfeb, &ichMin);
			ch = GetNextCh(pfeb);
			switch (ch)
				{
			case '=':
				AcceptLkAhd( pfeb, ichMin );
				tiRet = tiLE;
				goto lblComReturn;
			case '>':
				AcceptLkAhd( pfeb, ichMin );
				tiRet = tiNE;
				goto lblComReturn;
			default:
				BackTrackLkAhd( pfeb, ichMin );
				tiRet = tiLT;
				goto lblNo2Return;
				}
		case '>':
			PutChInTokenBuf(ch, pfeb);
			StartLkAhd(pfeb, &ichMin);
			ch = GetNextCh(pfeb);
			if (ch == '=')
				{
				AcceptLkAhd( pfeb, ichMin );
				tiRet = tiGE;
				goto lblComReturn;
				}
			else
				{
				BackTrackLkAhd( pfeb, ichMin );
				tiRet = tiGT;
				goto lblNo2Return;
				}
		case '-':
		case '+':
		case '*':
		case '/':
		case '[':
		case ']':
		case chColon:
LReturnCh:      
			tiRet = ch;
			goto lblComReturn;
		case '(':
			if (!(grpfLex & (fLexInUnrfnc | fLexNumInParen)))
				{
				int ti;
				int ichMin;

				StartLkAhd(pfeb, &ichMin);
				ti = TiLexFPfeb(grpfLex | fLexNumInParen, pfeb);
				if (ti == tiPRE_CURRENCY)
					{
					(pfeb->evLex).fCurrency = fTrue;
					ResetTokenBuf(pfeb);
					ti = TiLexFPfeb(grpfLex | fLexNumInParen, pfeb);
					}

				if (ti == tiINTEGER || ti == tiNUMBER)
					{
					EV	evT;
					CHAR	szT[ichLkAhdMax];
					int	ichTknMac;
					int	tiSave;

					bltbyte(&(pfeb->evLex), &evT, sizeof(EV));
					bltbyte(&(pfeb->szToken[0]), szT, cchTokenMax);
					ichTknMac = pfeb->ichTokenMac;
					tiSave = ti;

					ti = TiLexFPfeb(grpfLex | fLexNumInParen, pfeb);
					if (ti == tiPOST_CURRENCY)
						{
						evT.fCurrency = fTrue;
						ti = TiLexFPfeb(grpfLex | fLexNumInParen, pfeb);
						}

					if (ti == tiRParen)
						{
						bltbyte(szT, &(pfeb->szToken[0]), cchTokenMax);
						bltbyte(&evT, &(pfeb->evLex), sizeof(EV));
						pfeb->ichTokenMac = ichTknMac;
						if (!FEVUndefined(pfeb->evLex))
							{
							if (tiSave == tiINTEGER)
								{
								CNumInt(pfeb->evLex.i);
								StiNum(&(pfeb->evLex.v));
								}

							LdiNum(&(pfeb->evLex.v));
							NegNum();
							StiNum(&(pfeb->evLex.v));
							}
						AcceptLkAhd(pfeb, ichMin);
						(pfeb->evLex).fNegInParen = fTrue;
						return (tiNUMINPAREN);
						}
					}
				BackTrackLkAhd(pfeb, ichMin);
				}
			ResetTokenBuf(pfeb);
			ch = '(';
			goto LReturnCh;
		default:
			if (!FDigit(ch))
				goto LReturnCh;	/* This should make the parser puke. */

			while (FDigit(ch) || ch == vitr.chThousand)
				{
				if (ch == vitr.chThousand)
					{
/* in case chList == chThousand, as in USA, make sure it's really thousand */
#define ichThouMax	3
					int ich;
					int ichMin;

					StartLkAhd(pfeb, &ichMin);
					for ( ich = 0 ; ich < ichThouMax ; ich++ )
						if (!FDigit(GetNextCh(pfeb)))
							{
							BackTrackLkAhd(pfeb, ichMin);
							goto LEndThou;
							}
					BackTrackLkAhd(pfeb, ichMin);
					fInteger = fFalse;
					(pfeb->evLex).fComma = fTrue;
					}
				fHasADigit = fTrue;
				PutChInTokenBuf(ch, pfeb);
				ch = GetNextCh(pfeb);
				}
LEndThou:
			if (ch == vitr.chDecimal)
				{
lblNoWholeDig:
				fInteger = fFalse;
				(pfeb->evLex).fDecimal = fTrue;
				PutChInTokenBuf(ch, pfeb);

				ch = GetNextCh(pfeb);
				while (FDigit(ch))
					{
					fHasADigit = fTrue;
					(pfeb->evLex).cDigBlwDec++;
					PutChInTokenBuf(ch, pfeb);
					ch = GetNextCh(pfeb);
					}
				}

			if (!(grpfLex & fLexNumInParen) && fHasADigit)
				{
				SkipWhiteSpaces(ch, pfeb);
				if (ch == chPercent)
					{
					fInteger = fFalse;
					fPercent = fTrue;
					(pfeb->evLex).cDigBlwDec += 2;
					ch = GetNextCh(pfeb);
					}
				}
			PushBackCh(ch, pfeb);
			NullTerminateToken(pfeb);

			if (fHasADigit)
				{
				(pfeb->evLex).iszErrMsg = iszNoError;
				if (fInteger)
					{
					if (FIntFromSz(pfeb->szToken,
							&((pfeb->evLex).i)))
						{
#ifdef DBGYXY
						CommSzNum(SzShared("Integer: "),
								(pfeb->evLex).i);
#endif
						return (tiINTEGER);
						}
				/* Fall through. */
					}
				NumFromSz(pfeb->szToken, &((pfeb->evLex).v));
				if (fPercent)
					{
					LdiNum(&((pfeb->evLex).v));
					num100;
					DivNum();
					StiNum(&((pfeb->evLex).v));
					}
#ifdef DBGYXY
				CommSzPnum(SzShared("F. Number:"), &((pfeb->evLex).v));
#endif
				return tiNUMBER;
				}
			else
				{
				tiRet = ch = vitr.chDecimal;
lblComReturn:
				PutChInTokenBuf(ch, pfeb);
lblNo2Return:
				NullTerminateToken(pfeb);
#ifdef SHOWTI
				CommSzSz(SzShared("szToken: "), pfeb->szToken);
#endif
				return (tiRet);
				}
			}
		}
}


/*....and this one is for Utilities Calculate type evaluation. */
/* %%Function:TiLexSubCalc %%Owner:bryanl */
int TiLexSubCalc(grpfLex, pfeb)
WORD grpfLex;
FEB    *pfeb;
{
	FIBK   *pfibk = pfeb->pfibk;
	int     ti, tiT;
	BOOL    fPrevSkip = fFalse;
	int 	 ichMinT, ichMinNum;

	if (pfibk->fFirst)
		{	/* First token, make it tiEQ to ape field calcs */
		pfibk->fFirst = fFalse;
		ResetTokenBuf(pfeb);
		PutChInTokenBuf('=', pfeb);
		NullTerminateToken(pfeb);
		ti = tiEQ;
		goto lblRet0;
		}

	while (fTrue)
		{
		ti = TiLexFPfeb(grpfLex, pfeb);
		if (FSubCalcSkip(ti))
			{
LSkip:
			fPrevSkip = fTrue;
			continue;
			}
#ifdef SHOWTI 
		CommSzSz(SzShared("szToken: "), pfeb->szToken);
#endif

/* motivation of this clause: if we have X <op> Y where X and Y are both
   token types which should be skipped, want to skip the whole thing */

		if (fPrevSkip && (FBinOpTi(ti)
				|| ti == tiPRE_CURRENCY 
				|| ti == tiPOST_CURRENCY))
			{
			StartLkAhd( pfeb, &ichMinT );
			tiT = TiLexFPfeb(grpfLex, pfeb);
			if (FSubCalcSkip(tiT))
				{
				AcceptLkAhd( pfeb, ichMinT );
				goto LSkip;
				}
			BackTrackLkAhd( pfeb, ichMinT );
			if (ti == tiMinus)
				goto lblCheckMinus;
			goto lblRetTi;
			}

		switch (ti)
			{
		default:
			goto lblRet;

/* BL 2/20/89: Handling of minus signs in SubCalc (Util Calculate) turns out
   to be a thorny problem.  Here are the issues and how I have addressed
   them:

   (1) Implicit addition (pd 57) takes place at the <ucn><ucnum> lexical 
       level, meaning that any computed expression not consisting of 
       constants cannot be used for implicit addition. Thus,
       (1+2) 3 produces a syntax error instead of summing 1+2+3.

       HOW ADDRESSED: Punt; the syntax error described above stands.
       The one exception is minus operators; having 1-2 3 produce a syntax
       error is unacceptable because it may be regarded as an implicit 
       summation of 1, -2, and 3.  So, we apply the unary minus to the
       constant in this routine.

   (2) Given that we are applying unary minus to constants in this routine,
       and that the combination of computed expressions and implicit
       addition produces a syntax error, an expression like
       2*3-2 would produce a syntax error, if it were regarded as an 
       implicit summation of 2*3 and -2.

       HOW ADDRESSED: we will not apply unary minus in this routine if
       an operator disallowed in implicit addition has appeared before 
       in the token stream.

   (3) Issue (2) also applies to an expression like -2*3.

       HOW ADDRESSED: we will look ahead to the next operator following
       the constant, and not apply unary minus if that operator is 
       one disallowed in implicit addition.

	FUTURE: The whole design should be changed. The current design of 
	        handling implicit negation at the tokenization level and
			implicit addition at the parsing level simply will not
			work properly.
*/
		case tiMinus:
lblCheckMinus:
#ifdef BRYANL
			CommSz( SzShared( "MINUS CHECK: ") );
#endif

/* if minus sign is followed by a number, and the other conditions described
   in the above verbosity are met, apply the minus unararily to the number
   and return the negated result as a tiNumber token */

			StartLkAhd( pfeb, &ichMinNum );
			if (pfeb->fNoImplicitSubCalc)
				{
#ifdef BRYANL
				CommSz( SzShared( "non-implicit ops already in token stream, returning tiMinus\r\n"));
#endif
				goto lblRetMinus;	/* condition (2) */
				}
			ti = TiLexFPfeb(grpfLex, pfeb);
			if ((ti == tiINTEGER || ti == tiNUMBER || ti == tiNUMINPAREN))
				{
				StartLkAhd( pfeb, &ichMinT );
				tiT = TiLexFPfeb(grpfLex, pfeb);
				BackTrackLkAhd( pfeb, ichMinT );

				switch (tiT)
					{
				default:
#ifdef BRYANL
					CommSzNum( SzShared( "returning tiMinus, operator following -# is non-implicit:"),tiT);
#endif
					goto lblRetMinus;  /* condition (3) */
				case tiPRE_CURRENCY:
				case tiPOST_CURRENCY:
				case tiINTEGER:
				case tiNUMBER:
				case tiNUMINPAREN:	  /* condition (1) */
/* re-fetch number token so pfeb->evLex value is correct */
					BackTrackLkAhd( pfeb, ichMinNum );
					Assert( !pfeb->fLookingAhead );
					tiT = TiLexFPfeb(grpfLex, pfeb);
					Assert( ti == tiT );
					break;
					}

#ifdef BRYANL
				CommSz( SzShared( "performing unary minus in SubCalc\r\n"));
#endif
				if (ti == tiINTEGER)
					CNumInt(pfeb->evLex.i);
				else  /* ti == tiNUMBER || tiNUMINPAREN */
					LdiNum(&(pfeb->evLex.v));
				NegNum();
				StiNum(&(pfeb->evLex.v));
				ti = tiNUMBER;
				goto lblRet;
				}
#ifdef BRYANL
			CommSz( SzShared( "token following minus is not number, returning tiMinus\r\n"));
#endif

/* lookahead did not help us; return a normal minus sign & let the
   parser sort out its role. */
lblRetMinus:
			BackTrackLkAhd( pfeb, ichMinNum );
			ti = tiMinus;
lblRetTi:
			ResetTokenBuf(pfeb);
			PutChInTokenBuf((CHAR) ti, pfeb);
			NullTerminateToken(pfeb);
			goto lblRet;

		case tiLParen:
			StartLkAhd( pfeb, &ichMinT );
			tiT = TiLexFPfeb(grpfLex, pfeb);
			if (FSubCalcSkip(tiT))
				{
				AcceptLkAhd( pfeb, ichMinT );
				pfibk->cParenSkipped++;
				goto LSkip;
				}
			BackTrackLkAhd( pfeb, ichMinT );
			goto lblRetTi;

		case tiRParen:
			if (pfibk->cParenSkipped == 0)
				goto lblRet;
			pfibk->cParenSkipped--;
			continue;
			}	/* end switch */
		}	/* end while (fTrue) */

lblRet:
	if (!FImplicitTi(ti))
		pfeb->fNoImplicitSubCalc = fTrue;
lblRet0:
#ifdef SHOWLEX
	CommSzNum(SzShared("Returning: "), ti);
#endif
	return (ti);
}


/* Non tokens allowed in non-field (Utilities Calculate expressions. */
/* chFEErrMsg is included to cause the parser generate  */
/* an error message.                                    */

#define itiMacImplicit	9	/* mac ti that's ok for use w/implicit add */
csconst int rgtiUtilCalc [] = { 
	tiEOI,
			tiNUMBER,
			tiINTEGER,
			tiPRE_CURRENCY,
			tiPOST_CURRENCY,
			tiNUMINPAREN,
			tiMinus,
			tiLParen,
			tiRParen,
			chFEErrMsg, 
			tiPlus,
			tiMultiply,
			tiDivide,
			tiExp
};


#define itiUtilCalcMax	sizeof(rgtiUtilCalc)/sizeof(int)

/* True iff ti is to be skipped in Utilities Calculate type evaluation. */
/* %%Function:FSubCalcSkip %%Owner:bryanl */
BOOL FSubCalcSkip(ti)
int ti;
{
#ifdef NOASM
/* %%Function:IScanLprgw %%Owner:bryanl */
	NATIVE IScanLprgw(); /* DECLARATION ONLY */
#endif /* NOASM */
	return IScanLprgw( (LPINT)rgtiUtilCalc, ti, itiUtilCalcMax ) == iNil;
}


/* retun fTrue if ti is an allowable token in implicit-add expressions 
	e.g. 1 5 - 1 */

/* %%Function:FImplicitTi %%Owner:bryanl */
BOOL FImplicitTi(ti)
int ti;
{
#ifdef NOASM
/* %%Function:IScanLprgw %%Owner:bryanl */
	NATIVE IScanLprgw(); /* DECLARATION ONLY */
#endif /* NOASM */
	return (uns)IScanLprgw( (LPINT)rgtiUtilCalc, ti, itiUtilCalcMax ) < itiMacImplicit;
}



/* Classifies a given token in pfeb->szToken. */
/* %%Function:TiClassAlphaTokenPfeb %%Owner:bryanl */
NATIVE int TiClassAlphaTokenPfeb(pfeb)
FEB     *pfeb;
{
	int      ti;
	CHAR     ch1;
	CHAR     rgchCopy[cchTokenMax];
	long     grpfMask;

#ifdef DBGYXY
	CommSzSz(SzShared("Alpha Token: "), pfeb->szToken);
#endif
	if (pfeb->fSubCalc && !pfeb->pfibk->fBkmkSub)
		{
		return (tiBOOKMARK);
		}

	ch1 = *(pfeb->szToken);
	/* Because of CS compiler native bug. */
#ifndef INTL  /* REVIEW bryanl(jl) - this doesn't work for intl */
	      /* REVIEW bryanl(pj): I recommend nuking this code and doing this
		 the same for Z and INTL versions */
	grpfMask = 1L << (ch1 - 'A');
	if (('A' <= ch1 && ch1 <= 'Z') && /* strictly alphabets..... */
			(grpfReserved & grpfMask))
#else  /*INTL*/
	if (FAlpha(ch1))
#endif /*INTL*/
		{
		unsigned tfi;

		if ((tfi = IdFromSzgSz(szgFltExpFunc, pfeb->szToken)) != idNil)
			{
			ti = TiFromTfi(tfi);
			}
		else
			{
			goto lblBookMk;
			}

		if (ti == tiBOOLFALSE || ti == tiBOOLTRUE)
			{
			/* Convert it to numeric value. */
			(pfeb->evLex).iszErrMsg = iszNoError;
			(pfeb->evLex).gfi = 0;
			(pfeb->evLex).cDigBlwDec = 0;
			pfeb->evLex.i = (ti == tiBOOLFALSE) ? 0 : 1;
			ti = tiBOOL;
			}
		else
			{
			/* Record the function. */
			(pfeb->evLex).iszErrMsg = iszNoError;
			(pfeb->evLex).i = FiFromTfi(tfi);
			}
#ifdef SHOWLEX
		CommSzNum(SzShared("Returning: "), ti);
#endif
		return (ti);
		}
	else
		{
lblBookMk:
		if (pfeb->fSubCalc)  /* Save the search for bookmark. */
			{
			return (tiBOOKMARK);
			}

#ifdef SHOWLEX
		CommSzNum(SzShared("Returning: "), tiBOOKMARK);
#endif
		SzToSt(pfeb->szToken, rgchCopy);
		if (!FSearchBookmark(DocMother(pfeb->pffb->doc), rgchCopy,
				&((pfeb->evLex).cpFirstBkmk),
				&((pfeb->evLex).cpLimBkmk),
				&((pfeb->evLex).ibkf), bmcUser))
			{
			(pfeb->evLex).iszErrMsg = iszUndBkmkTkn;
			bltbyte(pfeb->szToken, &((pfeb->evLex).szErrTkn[0]), cchErrTknMax);
			}
		else
			{
			(pfeb->evLex).iszErrMsg = iszNoError;
			}
		return (tiBOOKMARK);
		}
}


/* Fetches a character from a field. */
/* %%Function:ChFetchFromFe %%Owner:bryanl */
CHAR ChFetchFromFe(pfeb)
FEB        *pfeb;
{
	struct FFB *pffb;
	CHAR	ch;

	pffb = pfeb->pffb;
	if (pfeb->ichFetch >= pffb->cch)
		{
		if (pffb->fOverflow) /* There's more to fetch. */
			{
			FetchFromField(pffb, fFalse, fFalse);
			pfeb->ichFetch = 0;
			}
		else
			{
			return (chEOI);
			}
		}

	ch = (pffb->rgch)[pfeb->ichFetch++];
	if (ch <= chAbbrevCur)
		{
	/* such char as chFootnote, chPic */
		ch = chSpace;
		}
	return (ch);
}


/* Fetches a character from a cp stream. */
/* %%Function:ChFetchFromBkSel %%Owner:bryanl */
CHAR ChFetchFromBkSel(pfeb)
FEB    *pfeb;
{
	CHAR    ch;
	FIBK   *pfibk;

	pfibk = pfeb->pfibk;
	if (pfeb->ichFetch >= pfibk->fvb.cch)
		{
		if (pfibk->fvb.fOverflow) /* There's more to fetch. */
			{
			FetchVisibleRgch(&(pfibk->fvb), 
					(pfibk->fVanished?fvcHidResults:fvcResults),
					fFalse, fTrue);
			pfeb->ichFetch = 0;
			}
		else
			{
			return (chEOI);
			}
		}

	ch = (pfibk->fvb.rgch)[pfeb->ichFetch++];
	if (ch <= chAbbrevCur)
		{
	/* such char as chFootnote, chPic */
		ch = chSpace;
		}
	return (ch);
}



/* Fetches a character from a block selection. */
/* %%Function:ChFetchFromBlock %%Owner:bryanl */
CHAR ChFetchFromBlock(pfeb)
FEB     *pfeb;
{
	CHAR    ch;
	FIBLK  *pfiblk;

	pfiblk = pfeb->pfiblk;
	Assert(!pfiblk->fVanished);
	if (pfeb->ichFetch >= pfiblk->fvb.cch)
		{
		if (pfiblk->fvb.fOverflow) /* There's more to fetch from this dl. */
			{
			FetchVisibleRgch(&(pfiblk->fvb), fvcResults, fFalse, fTrue);
			pfeb->ichFetch = 0;
			}
		else  if (pfiblk->fMoreBlock) /* There's more dl to fetch. */
			{
			CP  dcp;

			if (pfiblk->fMoreBlock =
					FGetBlockLine(&(pfiblk->fvb.cpFirst), &dcp,
					&(pfiblk->bks)))
				{
				pfiblk->fvb.cpLim = pfiblk->fvb.cpFirst + dcp;
				FetchVisibleRgch(&(pfiblk->fvb), fvcResults, fFalse, fTrue);
				pfeb->ichFetch = 0;
				return (chSpace);  /* The end of a dl breaks a token. */
				}
			else
				{
				return (chEOI);
				}
			}
		else
			{
			return (chEOI);
			}
		}
	ch = (pfiblk->fvb.rgch)[pfeb->ichFetch++];
	if (ch <= chAbbrevCur)
		{
	/* such char as chFootnote, chPic */
		ch = chSpace;
		}
	return (ch);
}


/* Fetches a character from a sz. */
/* %%Function:ChFetchFromSz %%Owner:bryanl */
CHAR ChFetchFromSz(pfeb)
FEB     *pfeb;
{
	CHAR    ch;
	FIBLK  *pfiblk;

	pfiblk = pfeb->pfiblk;
	ch = (pfiblk->fvb.rgch)[pfeb->ichFetch++];
	if (ch == '\0')
		{
		return chEOI;
		}
	if (ch <= chAbbrevCur)
		{
		/* such char as chFootnote, chPic */
		ch = chSpace;
		}

	return (ch);
}


/* Character look-ahead routines. */
/* %%Function:PushBackCh %%Owner:bryanl */
PushBackCh(ch, pfeb)
FEB    *pfeb;
CHAR    ch;
{
#ifdef BRYANL
	int ichMin = pfeb->ichLkAhdMin;
	CommSzNum( SzShared( "PushBackCh, ch: "), ch );
#endif
/* NOTE: under CS, -1 % N produces -1, not N-1, so must use
	i+N-1 instead of i-1 */
	pfeb->ichLkAhdMin = IchMod( pfeb->ichLkAhdMin + (ichLkAhdMax - 1) );
#ifdef BRYANL
	if (ch != pfeb->rgchLkAhd [pfeb->ichLkAhdMin])
		{
		int rgw [3];
		rgw [0] = ichMin;
		rgw [1] = pfeb->ichLkAhdMin;
		rgw [2] = pfeb->rgchLkAhd [pfeb->ichLkAhdMin];
		CommSzRgNum( SzShared( "ichMin from, to; ch in ary: "), rgw, 3 );
		}
#endif
	Assert( pfeb->rgchLkAhd [pfeb->ichLkAhdMin] == ch );
}



/* %%Function:GetNextCh %%Owner:bryanl */
CHAR GetNextCh(pfeb)
FEB    *pfeb;
{
	CHAR ch;
	CHAR *pch, *pchT;
	CHAR szCurLkAhd[cchMaxCurLkAhd];

	if (pfeb->ichLkAhdMac != pfeb->ichLkAhdMin)
		{
		ch = pfeb->rgchLkAhd[pfeb->ichLkAhdMin];
		pfeb->ichLkAhdMin = IchMod( pfeb->ichLkAhdMin + 1 );
#ifdef BRYANL
		CommSzNum( SzShared( "Get next ch from LkAhd: "), ch );
#endif
		return (ch);
		}

/* LkAhd buffer is empty, fetch from input stream */

	ch = ChFetchExp(pfeb);

#ifndef INTL
	if (ch == vitr.szCurrency[0])
#else  /* INTL to prevent catching currency in the middle of Bookmarks (jl) */
	if (ch == ChUpper(vitr.szCurrency[0]) && pfeb->fTestForCurrency)
#endif /* INTL */

		{
		CHAR	*pchCur, *pchNoUse;

		pch = &szCurLkAhd[0];
		*pch++ = ch;
		for (pchCur = &(vitr.szCurrency[1]);
				*pchCur != '\0' &&  (*pch = ChFetchExp(pfeb)) == ChUpper(*pchCur);
				pch++, pchCur++)
			;

		if (*pchCur == '\0')
			{
			if (pfeb->fCurAlphaLastCh)
				{
			/* Fetch next ch and see if it is a break char */
#ifndef INTL /* REVIEW(jl) bryanl - pch is already ++ */
				pch++;
#endif /* INTL */
				*pch = ChFetchExp(pfeb);
#ifndef INTL
				if (!FAlphaNumFE(*pch))
#else  /* INTL handle "Fl12" (=12 Dutch Guilders) */
				if (!FAlphaNumFE(*pch) || (!vitr.fCurPostfix && !vitr.fCurSepBlank && FDigit(*pch)))
#endif /* INTL */
					{
					ch = chAbbrevCur;
					pchNoUse = pch;
					goto lblNoUse;
					}
				else
					goto lblAbandon;
				}
			*(pch + 1) = '\0';
			ch = chAbbrevCur;
			}
		else
			{
lblAbandon:
			ch = szCurLkAhd[0];
			pchNoUse = &szCurLkAhd[1];
lblNoUse:
		/* Adjust the look ahead for the currency. */
			*(pch + 1) = '\0';
			for (pchT = &szCurLkAhd[0], pch = pchNoUse;
					*pch != '\0';
					*pchT++ = *pch++)
				;
			for (pch = &(pfeb->szCurLkAhd[0]); *pch != '\0'; *pchT++ = *pch++)
				;
			*pchT = '\0';
			bltbyte(szCurLkAhd, pfeb->szCurLkAhd, cchMaxCurLkAhd);
			}
		}
	if (ch == chGroupInternal)
		ch = chSpace;

#ifdef BRYANL
	CommSzNum( SzShared( "Get Next Ch from Input stream: "), ch );
#endif

/* record fetched ch in LkAhd round-robin buffer */

	Assert( pfeb->ichLkAhdMac == pfeb->ichLkAhdMin );
	pfeb->rgchLkAhd [pfeb->ichLkAhdMin] = ch;
	pfeb->ichLkAhdMin = pfeb->ichLkAhdMac = IchMod( pfeb->ichLkAhdMin + 1 );
	return (ch);
}



/* Fetch me a character from wherever I'm supposed to get it from. */
/* %%Function:ChFetchExp %%Owner:bryanl */
CHAR ChFetchExp(pfeb)
FEB	*pfeb;
{
	CHAR	ch;
	CHAR	*pch, *pchLkAhd;

	pchLkAhd = &(pfeb->szCurLkAhd[0]);
	if (*pchLkAhd == '\0')
		{
		ch = (*pfeb->pfnChFetch)(pfeb);
		}
	else
		{
		ch = *pchLkAhd;
		for (pch = pchLkAhd; *(pch + 1) != '\0'; pch++)
			{
			*pch = *(pch + 1);
			}
		*pch = '\0';
		}
#ifndef INTL
	ch = ChUpper(ch);
#else /* INTL */
	ch = ChUpperLookup(ch);
#endif /* INTL */
	return ch;
}


/* %%Function:FIntFromSz %%Owner:bryanl */
BOOL FIntFromSz(szToken, pi)
CHAR	*szToken;
int	*pi;
{
	int	w, wPrev;

	for (wPrev = w = 0; *szToken != '\0'; szToken++)
		{
		if (wPrev != (w *= 10) / 10)
			return fFalse;
		w += (*szToken - '0');
		wPrev = w;
		}
	*pi = w;
	return fTrue;
}


#ifdef SHOWCHFETCH
/* %%Function:ShowLkAhdBuf %%Owner:bryanl */
ShowLkAhdBuf(pfeb)
FEB     *pfeb;
{
	int  ich;
	CHAR *pch;
	CHAR szBuf[ichLkAhdMax + 2];

	bltbyte(pfeb->rgchLkAhd, szBuf, ichLkAhdMax);
	szBuf[pfeb->ichLkAhdMac] = '\0';
	for (pch = szBuf; *pch != '\0'; pch++)
		{
		if (*pch == chAbbrevCur)
			{
			*pch = '\\';
			}
		}

	CommSzSz(SzShared("Buffer: "), szBuf);

	for (ich = 0; ich < ichLkAhdMax + 2; szBuf[ich++] = chSpace);
	if (pfeb->ichLkAhdMin == pfeb->ichLkAhdMac)
		{
		szBuf[pfeb->ichLkAhdMac] = 'X';
		}
	else
		{
		szBuf[pfeb->ichLkAhdMin] = 'm';
		szBuf[pfeb->ichLkAhdMac] = 'M';
		}
	szBuf[pfeb->ichLkAhdMac + 1] = '\0';

	CommSzSz(SzShared("Points: "), szBuf);
}


#endif




/* %%Function:FApplyFi %%Owner:bryanl */
BOOL FApplyFi(fi, pevArg1, pevArg2, pev)
int fi;
EV  *pevArg1, *pevArg2;
EV  *pev;
{
	int		wT;
	CMPNUMVAL	cnv;
	NUM		numT0;

	pev->iszErrMsg = iszNoError;
	if (fi == fiAND || fi == fiOR || fi == fiNOT || fi == fiSIGN)
		{
		num0;
		StiNum(&numT0);
		}
	switch (fi)
		{
	case fiABS:
		num0;
		cnv = CmpINum(&(pevArg1->v));
		LdiNum(&(pevArg1->v));
		if (FGtCmp(cnv))
			{
			NegNum();
			}
		pev->cDigBlwDec = pevArg1->cDigBlwDec;
		pev->gfi = pevArg1->gfi;
		break;
	case fiAND:
		if (FApxEQ(WApproxCmpPnum(&(pevArg1->v), &numT0)))
			{
			num0;
			}
		else  if (FApxEQ(WApproxCmpPnum(&(pevArg2->v), &numT0)))
			{
			num0;
			}
		else
			{
			num1;
			}
		pev->cDigBlwDec = 0;
		pev->gfi = 0;
		break;
	case fiDEFINED:
		if (FEVUndefined(*pevArg1))
			{
			num0;
			}
		else
			{
			num1;
			}
		pev->cDigBlwDec = 0;
		pev->gfi = 0;
		break;
	case fiINT:
		LdiNum(&(pevArg1->v));
		FixNum(0);
		pev->cDigBlwDec = 0;
		pev->gfi = pevArg1->gfi;
		break;
	case fiROUND:
		LdiNum(&(pevArg2->v));
		wT = CIntNum();
		LdiNum(&(pevArg1->v));
		RoundNum(-wT);
		pev->cDigBlwDec = max(wT, 0);
		pev->gfi = pevArg1->gfi;
		break;
	case fiMOD:
		LdiNum(&(pevArg1->v));
		ModINum(&(pevArg2->v));
		pev->cDigBlwDec = max(vitr.iDigits,
				pevArg1->cDigBlwDec + pevArg2->cDigBlwDec);
		pev->gfi = pevArg1->gfi | pevArg2->gfi;
		break;
	case fiNOT:
		if (FApxEQ(WApproxCmpPnum(&(pevArg1->v), &numT0)))
			{
			num1;
			}
		else 		    
			{
			num0;
			}
		pev->cDigBlwDec = 0;
		pev->gfi = 0;
		break;
	case fiOR:
		if (FApxEQ(WApproxCmpPnum(&(pevArg1->v), &numT0)) &&
				FApxEQ(WApproxCmpPnum(&(pevArg2->v), &numT0)))
			{
			num0;
			}
		else
			{
			num1;
			}
		pev->cDigBlwDec = 0;
		pev->gfi = 0;
		break;
	case fiSIGN:
		Assert(wapxGT > 0 && wapxLT < 0 && wapxEQ == 0);
		wT = WApproxCmpPnum(&(pevArg1->v), &numT0);
		CNumInt(wT);
		pev->cDigBlwDec = 0;
		pev->gfi = 0;
		break;

	case fiIF:
	case fiNULL:
	default:
		Assert(fFalse);
		return fFalse;
		}
	StiNum(&(pev->v));
	return fTrue;
}


/* %%Function:FApplyInfixFi %%Owner:bryanl */
BOOL FApplyInfixFi(fi, pevArg1, pevArg2, pevRes)
int fi;
EV  *pevArg1, *pevArg2;
EV  *pevRes;
{
	BOOL fBool;
	EV   *pevT;

	pevRes->iszErrMsg = iszNoError;
	if (FEVUndefined(*pevArg1))
		{
		pevT = pevArg1;
		goto lblErrCopy;
		}
	else  if (FEVUndefined(*pevArg2))
		{
		pevT = pevArg2;
lblErrCopy:
		bltbyte(pevT, pevRes, sizeof(EV));
		return fFalse;
		}
	else
		{
		if (fi < fiInfixPreloadMin)
			{
			LdiNum(&(pevArg1->v));
			}
		switch (fi)
			{
		case fiPLUS:
			AddINum(&(pevArg2->v));
			break;
		case fiMINUS:
			SubINum(&(pevArg2->v));
			break;
		case fiMULTIPLY:
			MulINum(&(pevArg2->v));
			break;
		case fiDIVIDE:
			DivINum(&(pevArg2->v));
			pevRes->gfi =
					pevArg1->gfi | pevArg2->gfi;
			pevRes->cDigBlwDec = max(vitr.iDigits,
					pevArg1->cDigBlwDec + pevArg2->cDigBlwDec);
			break;
		case fiEXP:
				{
				int	wAC;
				BOOL	fSign;
				NUM	numT, numT1;
				ENV	env, *penvSv;


				fSign = fFalse;
				num0;
				StiNum(&numT);

				if (FApxEQ(wAC = WApproxCmpPnum(&(pevArg1->v), &numT)))
					{
					if (FApxLT(wAC = WApproxCmpPnum(&(pevArg2->v), &numT)))
						{
						pevRes->iszErrMsg = iszZeroDiv;
						return fFalse;
						}
					else  if (FApxEQ(wAC))
						{
						num1;
						}
					else
						{
						num0;
						}
					goto lblExpDone;
					}
				else
					{
					if (FApxLT(wAC))
						{
						LdiNum(&(pevArg2->v));
						FixNum(0);
						StiNum(&numT1);

						if (FApxNE(
								WApproxCmpPnum(&(pevArg2->v), &numT1)))
							{
							pevRes->iszErrMsg = iszNegFractExp;
							return fFalse;
							}
					/* Convert the exponent to a long, to
					   see if it is an odd number. */
						if (SetJmp(&env) != 0)
							{
							pevRes->iszErrMsg = iszFEOverflow;
							PopMPJmp(penvSv);
							return fFalse;
							}
						else
							{
							PushMPJmpEnv(env, penvSv);
							}
					/* The following test is machine
					   dependent. */
						fSign = LWholeFromNum(&(pevArg2->v), fFalse) &
								0x00000001;
						PopMPJmp(penvSv);

						LdiNum(&(pevArg1->v));
						NegNum();
						}
					else
						{
						LdiNum(&(pevArg1->v));
						}
					}

				LnNum();
				MulINum(&(pevArg2->v));
				ExpNum();
				if (fSign)
					{
					NegNum();
					}
lblExpDone:
				pevRes->gfi =
						pevArg1->gfi | pevArg2->gfi;
				pevRes->cDigBlwDec =
						pevArg1->cDigBlwDec + pevArg2->cDigBlwDec;
				break;
				}
		case fiEQ:
			fBool = FApxEQ(WApproxCmpPnum(&(pevArg1->v), &(pevArg2->v)));
			goto lblBoolPush;
		case fiLT:
			fBool = FApxLT(WApproxCmpPnum(&(pevArg1->v), &(pevArg2->v)));
			goto lblBoolPush;
		case fiLE:
			fBool = FApxLE(WApproxCmpPnum(&(pevArg1->v), &(pevArg2->v)));
			goto lblBoolPush;
		case fiGT:
			fBool = FApxGT(WApproxCmpPnum(&(pevArg1->v), &(pevArg2->v)));
			goto lblBoolPush;
		case fiGE:
			fBool = FApxGE(WApproxCmpPnum(&(pevArg1->v), &(pevArg2->v)));
			goto lblBoolPush;
		case fiNE:
			fBool = FApxNE(WApproxCmpPnum(&(pevArg1->v), &(pevArg2->v)));
lblBoolPush:
			if (fBool)
				{
				num1;
				}
			else
				{
				num0;
				}
			break;
		default:
			Assert(fFalse);
			if (fi < fiInfixPreloadMin)
				{
				/* Pop unused num on the stack. */
				StiNum(&(pevArg1->v));
				}
			return fFalse;
			}
		StiNum(&(pevRes->v));
		}
#ifdef DBGYXY
	CommSzPnum(SzShared("Infix Arg1:"), &(pevArg1->v));
	CommSzPnum(SzShared("Infix Arg2:"), &(pevArg2->v));
	CommSzPnum(SzShared("Infix Result:"), &(pevRes->v));
#endif
	return fTrue;
}


/* %%Function:ApplyReduction %%Owner:bryanl */
ApplyReduction(pev, fi, hrgtre, itreMac, ppes, pevTop)
EV	*pev;
int	fi;
TRE	**hrgtre;
int	itreMac;
PES	*ppes;
EV	*pevTop;
{
	TRE		*ptre;
	int		itre;
	unsigned	c, cAdd;
	CHAR		cDigBlw;
	CHAR		gfi;
	AV		av, avCur;
	TR2		tr2;

	InitPav(&av, fi);

	for (itre = 0; itre < itreMac; itre++)
		{
#ifdef DBGYXY
		CommSzNum(SzShared("itre: "), itre);
		CommSzPnum(SzShared("av: "), &av.num);
#endif
		ptre = (*hrgtre) + itre;
		if (ptre->fTR2)
			{
			bltbyte(&ptre->tr2, &tr2, sizeof(tr2));
			AvFromTr2Fi(fi, &avCur, &tr2, ppes, pevTop);
			}
		else
			{
			bltbyte(&ptre->num, &avCur.num, sizeof(NUM));
			avCur.cDigBlw = ptre->cDigBlw;
			avCur.gfi = ptre->gfi;
			avCur.c = 1;
			}

		if (itre == 0 && ((fi == fiMIN) || (fi == fiMAX)))
			{
			bltbyte(&avCur.num, &av.num, sizeof(NUM));
			}
		CombineAvFi(fi, &av, &avCur);
		}

	if (fi == fiAVERAGE || fi == fiCOUNT)
		{
		if (av.c < 0)
			{
			pev->iszErrMsg = iszFEOverflow;
			return;
			}
		}

	switch (fi)
		{
	case fiAVERAGE:
		LdiNum(&av.num);
		CNumInt(av.c);
		DivNum(); /* Watch this one, av.c can be 0. */
		break;
	case fiCOUNT:
		CNumInt(av.c);
		av.cDigBlw = 0;
		break;
	default:
		LdiNum(&av.num);
		break;
		}
	StiNum(&pev->v);
	pev->gfi = av.gfi;
	pev->cDigBlwDec = (fi != fiAVERAGE) ? av.cDigBlw :
			max(vitr.iDigits, av.cDigBlw);
}


/* %%Function:AvFromTr2Fi %%Owner:bryanl */
AvFromTr2Fi(fi, pav, ptr2, ppes, pevTop)
int	fi;
AV	*pav;
TR2	*ptr2;
PES	*ppes;
EV	*pevTop;
{
	int	iRow, iCol, iRowLast, iColLast;
	CP	cpCur, cpLim, cpNext;
	int	dics;
	AV	avCur;

	InitPav(pav, fi);

	Assert(FInTableDocCp(ptr2->doc, ptr2->cpTable));
	CacheTable(ptr2->doc, ptr2->cpTable);
	Assert(ptr2->cpTable == caTable.cpFirst);
	cpLim = caTable.cpLim;

	iRowLast = ptr2->iRowLast;
	Assert(iStar == 0x7FFF);
	Assert(ptr2->iRowMin <= ptr2->iRowLast);
	iRow = 1;
	cpCur = ptr2->cpTable;
	if (ptr2->iRowMin != iStar)
		{
		for ( ; iRow < ptr2->iRowMin && cpCur < cpLim; iRow++)
			{
			CpFirstTap(ptr2->doc, cpCur);
			cpCur = caTap.cpLim;
			}
		}

	ppes->iNest++;
	dics = pevTop - &(ppes->rgev[ppes->icsMac]) + 1;
	ppes->icsMac += dics;

	for ( ; iRow <= iRowLast && cpCur < cpLim; iRow++)
		{
		CpFirstTap(ptr2->doc, cpCur);
		iCol = (ptr2->iColMin != iStar) ? ptr2->iColMin : 1;
		iColLast = (ptr2->iColLast != iStar) ?
				min(ptr2->iColLast, vtapFetch.itcMac) :
				vtapFetch.itcMac;
		cpNext = caTap.cpLim;
		cpCur = CpFirstForItc(ptr2->doc, cpCur, iCol - 1);
		for ( ; iCol <= iColLast; iCol++)
			{
			CacheTc(wwNil, ptr2->doc, cpCur, fFalse, fFalse);
			cpCur = vtcc.cpLim;

			if (FNumFromCps(ptr2->doc, vtcc.cpFirst, vtcc.cpLim,
					&avCur.num, ppes, fFalse,
					&avCur.cDigBlw, &avCur.gfi))
				{
				avCur.c = 1;

				if (pav->c == 0 &&
						((fi == fiMIN) || (fi == fiMAX)))
					{
					bltbyte(&avCur.num, &pav->num,
							sizeof(NUM));
					}
				CombineAvFi(fi, pav, &avCur);
#ifdef DBGYXY
				CommSzNum(SzShared("iRow: "), iRow);
				CommSzNum(SzShared("iCol: "), iCol);
				CommSzPnum(SzShared("avCur: "), &avCur.num);
#endif
				}
			}
		cpCur = cpNext;
		}

	ppes->iNest--;
	ppes->icsMac -= dics;
}


/* %%Function:CombineAvFi %%Owner:bryanl */
CombineAvFi(fi, pav, pavSrc)
int	fi;
AV	*pav, *pavSrc;
{
	int	w;

	switch (fi)
		{
	case fiCOUNT:
		pav->c += pavSrc->c;
		break;
	case fiAVERAGE:
		pav->c += pavSrc->c;
		/* Fall through is intentional */
	case fiSUM:
		LdiNum(&pavSrc->num);
		AddINum(&pav->num);
		goto lblStore;
	case fiMIN:
	case fiMAX:
		pav->c += pavSrc->c; /* To detect the first time around. */
		w = WApproxCmpPnum(&pav->num, &pavSrc->num);
		if ((fi == fiMIN) ? FApxLT(w) : FApxGT(w))
			{
			LdiNum(&pav->num);
			}
		else
			{
			LdiNum(&pavSrc->num);
			}
		goto lblStore;
	case fiPRODUCT:
		LdiNum(&pavSrc->num);
		MulINum(&pav->num);
lblStore:
		StiNum(&pav->num);
		break;
	default:
		Assert(fFalse);
		}
	pav->gfi |= pavSrc->gfi;
	pav->cDigBlw = max(pav->cDigBlw, pavSrc->cDigBlw);
}


/* %%Function:InitPav %%Owner:bryanl */
InitPav(pav, fi)
AV	*pav;
int	fi;
{
	SetBytes(pav, 0, sizeof(AV));
	if (fi == fiPRODUCT)
		{
		num1;
		}
	else
		{
		num0;
		}
	StiNum(&(pav->num));
}


/* %%Function:FNumFromBkmkCp %%Owner:bryanl */
FNumFromBkmkCp(pfebBk, pev, cpFirst, cpLim, ppes)
FEB *pfebBk;
EV  *pev;
CP   cpFirst, cpLim;
PES *ppes;
{
	FEB         feb;
	FIBK        fibk;
	CHAR        rgch[ichLkAhdMax];
	struct CR   rgcr[ccrFebMax];

	InitFvbBufs(&(fibk.fvb), rgch, ichLkAhdMax, rgcr, ccrFebMax);
	InitFebBkSel(&feb, &fibk, DocMother(pfebBk->pffb->doc), 
			cpFirst, cpLim, fTrue, fFalse);

	FParseExp(&feb, ppes);
	FreePhhac(&feb.hhac);
	if (!FEVUndefined(feb.evResult))
		{
		bltbyte(&(feb.evResult), pev, sizeof(EV));
		}
	else
		{
		pev->iszErrMsg = iszFESyntaxAtBkmk;
		bltbyte(pfebBk->szToken, &(pev->szErrTkn[0]), cchErrTknMax);
		}

	return (fTrue);
}



/* F C R  C A L C  F L T  E X P */
/*  Evaluate an expression
    Format: {=<exp>}
*/

/* %%Function:FcrCalcFltExp  %%Owner:bryanl */
FcrCalcFltExp (doc, ifld, flt, cpInst, cpResult, pffb)
int doc, ifld, flt;
CP  cpInst, cpResult;
struct FFB *pffb;
{
	FEB         feb;
	int         cch;
	struct CHP  chp;
	CHAR        rgch[cchFEFMax];
	PES		pes;

#ifdef BRYANL
		{
		int rgw [4];

		rgw [0] = ifld;
		rgw [1] = flt;
		rgw [2] = (int)cpInst;
		rgw [3] = (int)cpResult;

		CommSzRgNum( SzShared("FcrCalcFltExp( ifld, flt, cpInst, cpResult ): "),rgw,4);
		}
#endif

	if (!vitr.fInvalidChSetting)
		{
		/*  set up ffb to fetch arguments */
		InitFvbBufs(&pffb->fvb, rgch, cchFEFMax, NULL, 0);

		/* We are set to go! */
		InitFebFe(&feb, pffb);
		InitPpes(&pes);
		FParseExp(&feb, &pes);
		FreePhhac(&feb.hhac);
		}
	else
		{
		feb.evResult.iszErrMsg = iszInvalChSetting;
		}

	/* chp is props of first char after chFieldBegin */
	GetPchpDocCpFIns (&chp, doc, cpInst+1, fFalse, wwNil);

	FFormatPev(&(feb.evResult), &chp, rgch, &cch, cchFEFMax);
#ifdef DBGYXY
	CommSzSz(SzShared("FcrCalc END.  Result: "), rgch);
#endif

	if (!FInsertRgch (doc, cpResult, rgch, cch, &chp, NULL))
		return fcrError;

	if (FEVUndefined(feb.evResult))
		{
		return(fcrError);
		}
	else
		{
		return (FcrRegisterRealResult(&(feb.evResult.v)));
		}
}


/* %%Function:FNumFromCps %%Owner:bryanl */
BOOL FNumFromCps(doc, cpFirst, cpLim, pnum, ppes, fVanished, pcDigBlw, pgfi)
int     doc;
CP      cpFirst, cpLim;
NUM    *pnum;
PES    *ppes;
BOOL    fVanished;
CHAR   *pcDigBlw;
CHAR   *pgfi;
{
	FEB       feb;
	FIBK      fibk;
	CHAR      rgch[cchFEFMax];
	struct CR rgcr[ccrFebMax];

	if (!vitr.fInvalidChSetting && cpFirst != cpLim)
		{
		InitFvbBufs(&(fibk.fvb), rgch, cchFEFMax, rgcr, ccrFebMax);
		InitFebBkSel(&feb, &fibk, doc, cpFirst, cpLim, fFalse, fVanished);
		FParseExp(&feb, ppes);
		FreePhhac(&feb.hhac);

		if (FEVUndefined(feb.evResult))
			{
			return (fFalse);
			}
		else
			{
			bltbyte(&(feb.evResult.v), pnum, sizeof(NUM));
			if (pcDigBlw != NULL)
				{
				*pcDigBlw = feb.evResult.cDigBlwDec;
				*pgfi     = feb.evResult.gfi;
				}
			return (fTrue);
			}
		}
	return (fFalse);
}


/* %%Function:FCalculateNumFromSz %%Owner:bryanl */
BOOL FCalculateNumFromSz(sz, pnum, ppes)
CHAR *sz;
NUM  *pnum;
PES  *ppes;
{
	FEB       feb;
	FIBK      fibk;
	CHAR      rgch[cchFEFMax];
	struct CR rgcr[ccrFebMax];

	if (!vitr.fInvalidChSetting && *sz != '\0')
		{
		InitFebSz(&feb, &fibk, sz, fFalse);
		FParseExp(&feb, ppes);
		FreePhhac(&feb.hhac);
		if (FEVUndefined(feb.evResult))
			{
			return (fFalse);
			}
		else
			{
			bltbyte(&(feb.evResult.v), pnum, sizeof(NUM));
			return (fTrue);
			}
		}
	return (fFalse);
}


/* %%Function:CmdCalculate %%Owner:bryanl */
CMD CmdCalculate(pcmb)
CMB * pcmb;
{
	return FEvalSelCur(fTrue, fTrue, NULL, NULL) ? cmdOK : cmdError;
}


/* %%Function:FEvalSelCur %%Owner:bryanl */
BOOL FEvalSelCur(fScrap, fPrompt, pnum, piszErr)
BOOL fScrap, fPrompt;
NUM * pnum;
int * piszErr;
{
	FEB                 feb;
	FIBK               *pfibk;
	FIBLK              *pfiblk;
	int                 cch;
	BOOL                fScrapOK;
	CHAR                rgch[cchFEFMax];
	struct CR           rgcr[ccrFebMax];
	struct CHP          chp;
	CHAR                rgchFiblkBuf[sizeof(FIBK) > sizeof(FIBLK) ?
                                         sizeof(FIBK) : sizeof(FIBLK)];
	PES			pes;
	extern struct SEL   selCur;
	extern struct UAB   vuab;

	fScrapOK = fTrue;

	StartLongOp();

	if (vitr.fInvalidChSetting)
		{
		feb.evResult.iszErrMsg = iszInvalChSetting;
		}
	else  if (selCur.fIns)
		{
		feb.evResult.iszErrMsg = iszEmptyExpression;
		}
	else
		{
		if (selCur.fBlock)
			{
			pfiblk = (FIBLK *) rgchFiblkBuf;
			InitFvbBufs(&(pfiblk->fvb), rgch, cchFEFMax,
					rgcr, ccrFebMax);
			InitFebBlk(&feb, pfiblk);
			}
		else
			{
			pfibk = (FIBK *) rgchFiblkBuf;
			InitFvbBufs(&(pfibk->fvb), rgch, cchFEFMax,
					rgcr, ccrFebMax);
			InitFebBkSel(&feb, pfibk, selCur.doc, selCur.cpFirst, 
					selCur.cpLim, fFalse, fFalse);
			}
		InitPpes(&pes);
		FParseExp(&feb, &pes);
		FreePhhac(&feb.hhac);
		}

	if (pnum != NULL)
		bltbyte(&(feb.evResult.v), pnum, sizeof(NUM));

	if (piszErr != NULL)
		*piszErr = feb.evResult.iszErrMsg;

	if (fScrap)
		{
		GetPchpDocCpFIns (&chp, selCur.doc, selCur.cpFirst, 
				fFalse, selCur.ww);
		FFormatPev(&(feb.evResult), &chp, &rgch[0], &cch, cchFEFMax);
		fScrapOK = FPutRgchInScrap(&rgch[0], cch, &chp, selCur.doc);
		}

	if (fPrompt)
		{
		rgch[cch] = '\0';
		SetPromptWMst(mstMath, (WORD)rgch, 
				pdcAdvise | pdcmCreate | pdcmPmt);
		}

	EndLongOp(fFalse /* fAll */);

	return fScrapOK && !FEVUndefined(feb.evResult);
	;
}


/* Given sz, conforming to the field expression syntax, evaluate
   it as though it appeared at selCur. */
/* %%Function:FEvalExpSz %%Owner:bryanl */
BOOL FEvalExpSz(sz, pnum, piszErr, fUseUtilCalc)
CHAR *sz;
NUM  *pnum;
int  *piszErr;
{
	FEB feb;
	FIBK fibk;
	CHAR rgch [cchFEFMax];
	struct CR rgcr [ccrFebMax];
	PES pes;

	if (vitr.fInvalidChSetting)
		{
		*piszErr = iszInvalChSetting;
		return fFalse;
		}

	if (*sz == '\0')
		{
		*piszErr = iszEmptyExpression;
		return fFalse;
		}

	InitPpes(&pes);
	InitFebSz(&feb, &fibk, sz, fFalse);
	if (FBkmkInTable(selCur.doc, selCur.cpFirst, NULL, 0))
		{
		feb.cpFirstTable = CpFirstTableFromDocCp(selCur.doc,
				selCur.cpFirst);
		feb.cpFirstRel = selCur.cpFirst;
		}
	else
		{
		feb.cpFirstTable = feb.cpFirstRel = cpNil;
		}

	if (fUseUtilCalc)
		{
		feb.pfnTiLex = TiLexSubCalc;
		if (sz[0] == '=')
			sz[0] = ' ';
		}
	else
		{
		feb.pfnTiLex = TiLexFPfeb;
		}

	feb.fSubCalc = fUseUtilCalc;
	FParseExp(&feb, &pes);
	FreePhhac(&feb.hhac);

	*piszErr = feb.evResult.iszErrMsg;
	if (!FEVUndefined(feb.evResult))
		{
		bltbyte(&(feb.evResult.v), pnum, sizeof(NUM));
		return (fTrue);
		}

	return (fFalse);
}


/* %%Function:FFormatPev %%Owner:bryanl */
FFormatPev(pev, pchp, rgch, pcch, cchMax)
EV         *pev;
struct CHP *pchp;
CHAR       *rgch;
int        *pcch;
int		cchMax;
{
	CHAR       *pch;
	int		merr;
	ENV		*penvSave;
	ENV		env;

	if ((merr = SetJmp(&env)) != 0)
		{
		Assert(merr != 0);
		pev->iszErrMsg = IszFEFromMerr(merr);
		goto lblErr;
		}
	else
		{
		PushMPJmpEnv(env, penvSave);
		}

	if (FEVUndefined(*pev))
		{
lblErr:
		if (*(pev->szErrTkn) == chFieldEndDisp)
			{
			Assert(pev->iszErrMsg == iszSyntaxErrTkn);
			pev->iszErrMsg = iszUnexpEOI;
			}
		bltbx((CHAR FAR *) rgszFE[pev->iszErrMsg - 1],
				(CHAR FAR *) rgch, cchFEFMax);
		if (pev->iszErrMsg >= iszErrMsgTknMin)
			{
			CHAR    *pchFEPH;
			int     cchTkn;
			CHAR    szErrTkn[cchErrTknMax + 1];

			bltbyte(pev->szErrTkn, szErrTkn, cchErrTknMax);
			szErrTkn[cchErrTknMax] = '\0';

			cchTkn = CchSz(szErrTkn) - 1;
			pchFEPH = index(rgch, chFEPH);
			Assert(pchFEPH != NULL);

			bltbyte(pchFEPH + 1, pchFEPH + cchTkn,
					(rgch + CchSz(rgch)) - pchFEPH - 1);
			bltbyte(szErrTkn, pchFEPH, cchTkn);
			}

		if (pchp != NULL)
			pchp->fBold = !pchp->fBold;
		*pcch = CchSz(rgch) - 1;
		PopMPJmp(penvSave);
		return (fFalse);
		}
	else
		{
		CHAR	szPic[cchMaxPic];
		CHAR	szTmplt[cchMaxPic];

		if (!FPicTmpltPnumGfi(&(pev->v), (int)(pev->gfi),
				(int)(pev->cDigBlwDec), szTmplt,
				cchMaxPic - DcchSafePicTmplt((int)(pev->gfi),
				(int)(pev->cDigBlwDec))))
			{
			pev->iszErrMsg = iszFETrans;
			goto lblErr;
			}
		LocalSzPicNumTmplt(szTmplt, szPic);
#ifdef BRYANL
		CommSzSz(SzShared("Formatting Number. Pic: "), szPic);
#endif
		*pcch = CchFormatNumPic(&(pev->v), szPic, rgch, cchMax);
#ifdef BRYANL
		{
		int rgchT [100];
		rgchT [0] = *pcch;
		bltb( rgch, &rgchT [1], rgchT [0] );
		StToSzInPlace( rgchT );
		CommSzSz( SzShared("  CchFormatNumPic returns: "), rgchT );
		}
#endif
		PopMPJmp(penvSave);
		return (fTrue);
		}

}



/* %%Function:FPutRgchInScrap %%Owner:bryanl */
BOOL FPutRgchInScrap(rgch, cch, pchp, doc)
CHAR         rgch[];
int          cch;
struct CHP  *pchp;
int          doc;
{
	int          docTemp;
	struct CA	caT;
	extern int 		vdocTemp;
	extern struct UAB   vuab;
	extern struct SAB   vsab;
	extern struct MERR  vmerr;

	if (DocCreateTemp(DocMother(doc)) != docNil)
		{
		if (!FDelete(PcaSetWholeDoc(&caT, vdocTemp))
				|| !FInsertRgch(vdocTemp, cp0, &rgch[0], cch, pchp, NULL))
			return fFalse;

		/* set caCopy so we can paste-link to this later */
		SetWholeDoc(docScrap, PcaSet( &vsab.caCopy, vdocTemp, cp0, (CP)cch ));
		vsab.docStsh = DocMother(vdocTemp);

		if (vmerr.fMemFail || vmerr.fDiskFail)
			return fFalse;

#ifdef STYLES
		PdodDoc(docScrap)->docSsht = PdodDoc(vdocTemp)->docSsht;
#endif

		vsab.fPict = fFalse;

		/* Force repaint of clipboard display & Set ownership */
		ChangeClipboard();

		return (fTrue);
		}
	else
		{
		return (fFalse);
		}
}




/* %%Function:InitFebFe %%Owner:bryanl */
InitFebFe(pfeb, pffb)
FEB         *pfeb;
struct FFB  *pffb;
{
	int doc;
	CP  cp, cpRel;

	/* Check to see if the expression lies within a table. */
	/* We do this up here before FetchFromField() changes  */
	/* pffb->fvb.cpFirst. */
	if (FBkmkInTable(doc = pffb->fvb.doc, cp = pffb->fvb.cpFirst, NULL, 0))
		{
		cp = CpFirstTableFromDocCp(doc, cpRel = cp);
		}
	else
		{
		cpRel = cp = cpNil;
		}

	pffb->fGroupInternal = fTrue;
	FetchFromField(pffb, fFalse, fFalse);

	SetBytes(pfeb, 0, sizeof(FEB));
	Assert(pfeb->hhac == hNil);
	pfeb->cpFirstTable  = cp;
	pfeb->cpFirstRel	= cpRel;
	pfeb->pfnChFetch    = ChFetchFromFe;
	pfeb->pfnTiLex      = TiLexFPfeb;
	pfeb->pffb          = pffb;

	pfeb->ti            = tiNull;

	pfeb->fCurAlphaLastCh = FAlphaFE(vitr.szCurrency[CchSz(vitr.szCurrency) - 2]);
#ifdef INTL
	pfeb->fTestForCurrency = fTrue;
#endif /* INTL */
}


/* %%Function:InitFebBkSel %%Owner:bryanl */
InitFebBkSel(pfeb, pfibk, doc, cpFirst, cpLim, fBkmk, fVanished)
FEB         *pfeb;
FIBK        *pfibk;
int          doc;
CP           cpFirst, cpLim;
BOOL         fBkmk, fVanished;
{
	pfibk->cParenSkipped      = 0;
	pfibk->fFirst             = fTrue;
	pfibk->fBkmkSub           = fBkmk;
	pfibk->fVanished	      = fVanished;

	pfibk->fvb.doc            = doc;
	pfibk->fvb.cpFirst        = cpFirst;
	pfibk->fvb.cpLim          = cpLim;
	FetchVisibleRgch(&(pfibk->fvb), 
			(fVanished?fvcHidResults:fvcResults),
			fFalse, fTrue);

	SetBytes(pfeb, 0, sizeof(FEB));
	Assert(pfeb->hhac == hNil);
	pfeb->pfnChFetch          = ChFetchFromBkSel;
	pfeb->pfnTiLex            = TiLexSubCalc;
	pfeb->pfibk               = pfibk;
	pfeb->ti                  = tiNull;
	pfeb->fSubCalc            = fTrue;
	pfeb->cpFirstRel = pfeb->cpFirstTable = cpNil;

	pfeb->fCurAlphaLastCh = FAlphaFE(vitr.szCurrency[CchSz(vitr.szCurrency) - 2]);
#ifdef INTL
	pfeb->fTestForCurrency = fTrue;
#endif /* INTL */
}


/* %%Function:InitFebBlk %%Owner:bryanl */
InitFebBlk(pfeb, pfiblk)
FEB         *pfeb;
FIBLK       *pfiblk;
{
	CP           dcp;

	pfiblk->cParenSkipped     = 0;
	pfiblk->fFirst            = fTrue;
	pfiblk->fBkmkSub          = fFalse;
	pfiblk->fVanished	      = fFalse;

	InitBlockStat(&(pfiblk->bks));
	pfiblk->fMoreBlock        =
			FGetBlockLine(&(pfiblk->fvb.cpFirst), &dcp, &(pfiblk->bks));
	pfiblk->fvb.doc           = pfiblk->bks.doc;
	pfiblk->fvb.cpLim         = pfiblk->fvb.cpFirst + dcp;
	FetchVisibleRgch(&(pfiblk->fvb), fvcResults, fFalse, fTrue);

	SetBytes(pfeb, 0, sizeof(FEB));
	Assert(pfeb->hhac == hNil);
	pfeb->pfnChFetch          = ChFetchFromBlock;
	pfeb->pfnTiLex            = TiLexSubCalc;
	pfeb->pfiblk              = pfiblk;
	pfeb->ti                  = tiNull;
	pfeb->fSubCalc            = fTrue;
	pfeb->cpFirstRel = pfeb->cpFirstTable = cpNil;

	pfeb->fCurAlphaLastCh = FAlphaFE(vitr.szCurrency[CchSz(vitr.szCurrency) - 2]);
#ifdef INTL
	pfeb->fTestForCurrency = fTrue;
#endif /* INTL */
}


/* %%Function:InitFebSz %%Owner:bryanl */
InitFebSz(pfeb, pfibk, sz, fBkmk)
FEB         *pfeb;
FIBK        *pfibk;
CHAR        *sz;
BOOL         fBkmk;
{
	pfibk->cParenSkipped      = 0;
	pfibk->fFirst             = fTrue;
	pfibk->fBkmkSub           = fBkmk;
	pfibk->fVanished          = fFalse;

	pfibk->fvb.rgch	      = sz;
	pfibk->fvb.doc            = selCur.doc;

	SetBytes(pfeb, 0, sizeof(FEB));
	Assert(pfeb->hhac == hNil);
	pfeb->pfnChFetch          = ChFetchFromSz;
	pfeb->pfnTiLex            = TiLexSubCalc;
	pfeb->pfibk               = pfibk;
	pfeb->ti                  = tiNull;
	pfeb->fSubCalc            = fTrue;
	pfeb->cpFirstRel = pfeb->cpFirstTable = cpNil;

	pfeb->fCurAlphaLastCh = FAlphaFE(vitr.szCurrency[CchSz(vitr.szCurrency) - 2]);
#ifdef INTL
	pfeb->fTestForCurrency = fTrue;
#endif /* INTL */
}
