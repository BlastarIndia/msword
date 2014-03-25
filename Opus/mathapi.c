#ifndef EXTMATH
#include "word.h"
#include "debug.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "ourmath.h"
#include "inter.h"

union RRU       vrruCalc;                /* Field Results */
ENV		*penvMathError;
STATIC NUM	rgnumMath[inumMax];
BOOL		fInitialized = fFalse;
int		inumTop = 0;
BOOL		fInRegister = fFalse;
NUM		numRndSeed;

#define POP	0
#define PUSH	1

/* InitMath - Initialise the math pack. This routine should be called at the
*  begining of each routine that does a Dld. It should only be called when
*  fInitialised == fFalse.
*/
/* %%Function:InitMath %%Owner:bryanl */
InitMath()
{
	Assert(!fInitialized);
	fInitialized = fTrue;
	InitMathPack();
	/* Do NOT call ResetMathPack as it could screw up the stack! */
	DError(0);
	InitRnd();
}


/* %%Function:InitRnd %%Owner:bryanl */
InitRnd()
{
	Assert(fInitialized);
	MakeNum(&numRndSeed, 0L, DttmCur(), 5);
}


/* %%Function:ResetMathPack %%Owner:bryanl */
ResetMathPack()
{
	extern BOOL	fError;

	Assert(fInitialized);
	fInRegister = fError = fFalse;
	inumTop = 0;
	DError(0);
}


/* %%Function:AddNum %%Owner:bryanl */
AddNum()
{
	NUM	numAdd;

	Assert(fInitialized);
	StiNum(&numAdd);
	AddINum(&numAdd);
}


/* %%Function:SubNum %%Owner:bryanl */
SubNum()
{
	NUM	numSub;

	Assert(fInitialized);
	StiNum(&numSub);
	SubINum(&numSub);
}



/* %%Function:MulNum %%Owner:bryanl */
MulNum()
{
	NUM	numMul;

	Assert(fInitialized);
	StiNum(&numMul);
	MulINum(&numMul);
}


/* %%Function:DivNum %%Owner:bryanl */
DivNum()
{
	NUM	numDiv;

	Assert(fInitialized);
	StiNum(&numDiv);
	DivINum(&numDiv);
}


/* %%Function:ModNum %%Owner:bryanl */
ModNum()
{
	NUM	numMod;

	Assert(fInitialized);
	StiNum(&numMod);
	ModINum(&numMod);
}


/* %%Function:AddINum %%Owner:bryanl */
AddINum(pnum)
NUM	*pnum;
{
	Assert(fInitialized);
	if (!fInRegister)
		{
		MathStackCheck(POP);
		StackToAcc();
		}
	DAdd(pnum);
	MathCheck();
}


/* %%Function:SubINum %%Owner:bryanl */
SubINum(pnum)
NUM	*pnum;
{
	Assert(fInitialized);
	if (!fInRegister)
		{
		MathStackCheck(POP);
		StackToAcc();
		}
	DSub(pnum);
	MathCheck();
}


/* %%Function:MulINum %%Owner:bryanl */
MulINum(pnum)
NUM	*pnum;
{
	Assert(fInitialized);
	if (!fInRegister)
		{
		MathStackCheck(POP);
		StackToAcc();
		}
	DMul(pnum);
	MathCheck();
}


/* %%Function:DivINum %%Owner:bryanl */
DivINum(pnum)
NUM	*pnum;
{
	Assert(fInitialized);
	if (!fInRegister)
		{
		MathStackCheck(POP);
		StackToAcc();
		}
	DDiv(pnum);
	MathCheck();
}


/* %%Function:ModINum %%Owner:bryanl */
ModINum(pnum)
NUM	*pnum;
{
	NUM	numT;

	if (!fInitialized)
		InitMath(); /* As we do some Dld's */
	if (!fInRegister)
		{
		MathStackCheck(POP);
		StackToPnum(&numT);
		}
	else
		{
		AccToPnum(&numT);
		}
	Assert(!fInRegister);
	Dld(&numT);
	DDiv(pnum);
	MathCheck();
	fInRegister = fTrue;
	FixNum(0);
	Assert(fInRegister);
	DMul(pnum);
	DNeg();
	DAdd(&numT);
	fInRegister = fTrue;
	MathCheck();
}


/* %%Function:NegNum %%Owner:bryanl */
NegNum()
{
	Assert(fInitialized);
	if (!fInRegister)
		{
		MathStackCheck(POP);
		StackToAcc();
		}
	DNeg();
}



/* %%Function:CmpNum %%Owner:bryanl */
long CmpNum()
{
	NUM	numCmp;

	if (!fInitialized)
		InitMath();
	StiNum(&numCmp);
	return (CmpINum(&numCmp));
}


/* %%Function:CmpINum %%Owner:bryanl */
long CmpINum(pnum)
NUM	*pnum;
{
	CMPNUMUN	cl;

	if (!fInitialized)
		InitMath();
	if (!fInRegister)
		{
		MathStackCheck(POP);
		StackToAcc();
		}
	DSub(pnum);
	cl.cnr.w2 = DCond();
	cl.cnr.w1 = 0;
	fInRegister = fFalse;
	MathCheck();
	return (cl.l);
}


/* %%Function:CNumInt %%Owner:bryanl */
CNumInt(w)
int	w;
{
	BOOL	fMinus;

	if (!fInitialized)
		InitMath();
	if (fMinus = w < 0)
		{
		w *= -1;
		}
	if (fInRegister)
		{
		MathStackCheck(PUSH);
		AccToStack();
		}
	DFloat(w);
	if (fMinus)
		{
		DNeg();
		}
	fInRegister = fTrue;
	MathCheck();
}


/* %%Function:CNumUns %%Owner:bryanl */
CNumUns(w)
unsigned	w;
{
	if (!fInitialized)
		InitMath();
	if (fInRegister)
		{
		MathStackCheck(PUSH);
		AccToStack();
		}
	DFloat(w);
	fInRegister = fTrue;
	MathCheck();
}


/* %%Function:CIntNum %%Owner:bryanl */
int CIntNum()
{
	unsigned	w;
	int		sign;

	if (!fInitialized)
		InitMath();
	if (!fInRegister)
		{
		MathStackCheck(POP);
		StackToAcc();
		}
	/* BUG: the mathpack has a bug in Fix when a math chip is installed.
		The following 'if' works around it.  This bug has been reported
		in \\bughotel\tools\math.bug #3 (14-July-89). */
	if ((sign = DCond()) < 0)
		DNeg();
	w = Fix();
	fInRegister = fFalse;
	if (w & 0x8000)
		{
		MathError(fmerrOver);
		}
	MathCheck();
	return w * sign;
}


/* %%Function:CUnsNum %%Owner:jurgenl */

/*
** This is the same a CIntNum() except that it returns an unsigned integer.
** This was added so that dates in BIFF files after 9/16/89 (32767) don't
** cause overflow: see ffread.c. (jl 2/15/90)
*/

unsigned CUnsNum()
{
	unsigned	w;

	if (!fInitialized)
		InitMath();
	if (!fInRegister)
		{
		MathStackCheck(POP);
		StackToAcc();
		}
	/* BUG: the mathpack has a bug in Fix when a math chip is installed.
		The following 'if' works around it.  This bug has been reported
		in \\bughotel\tools\math.bug #3 (14-July-89). */
	if (DCond() < 0)
		DNeg();
	w = Fix();
	fInRegister = fFalse;
	MathCheck();
	return w;
}



/* %%Function:LdComNum %%Owner:bryanl */
LdComNum(icnum)
int	icnum;
{
	NUM	num;

	if (!fInitialized)
		InitMath();
	if (fInRegister)
		{
		MathStackCheck(PUSH);
		AccToStack();
		}
	if (icnum == 0)
		{
		DClr();
		}
	else
		{
		DLdC(icnum - 1);
		}
	fInRegister = fTrue;
}


/* %%Function:UnpackNum %%Owner:bryanl */
UnpackNum(pwExp, pwSign, rgch)
int	*pwExp, *pwSign;
CHAR	rgch[];
{
	USTR	ustr;

	if (!fInitialized) InitMath();
	if (!fInRegister)
		{
		MathStackCheck(POP);
		StackToAcc();
		}
	if (DCond() == 0)
		{
		*pwSign = *pwExp = 0;
		rgch[1] = '0';
		rgch[0] = 1;
		fInRegister=fFalse;
		return;
		}
	Unpack(&ustr);
	fInRegister = fFalse;
	MathCheck();

	rgch[0] = ustr.cch;
	bltbyte(&ustr.rgch[0], &rgch[1], ustr.cch);
	*pwSign = ((0x8000 & ustr.es) != 0) ? -1 : 1;
	*pwExp  = (0x7FFF & ustr.es) - 0x4000;
}


/* %%Function:FixNum %%Owner:bryanl */
FixNum(w)
{
	BOOL	fNegative;
	int	i;
	NUM	num, numT;

	if (!fInitialized) InitMath(); /* As we do some Dld's */
	StiNum(&num);
	Dld(&num);
	fNegative = DCond() < 0;

	if (w != 0)
		{
		TenTo(w);
		Dst(&numT);
		Dld(&num);
		DDiv(&numT);
		}
	else
		{
		Dld(&num);
		}
	Dst(&num);
	DInt();
	fInRegister = fTrue;
	if (fNegative)
		{
		StiNum(&numT);
		if (FApxNE(WApproxCmpPnum(&num, &numT)))
			{
			num1;
			DNeg();
			DAdd(&numT);
			}
		else
			{
			Dld(&numT);
			}
		}
	if (w != 0)
		{
		DMul(&numT);
		}
	fInRegister = fTrue;
	MathCheck();
}


/* %%Function:RoundNum %%Owner:bryanl */
RoundNum(w)
int	w;
{
	BOOL	fNegative;
	NUM	num, numPower, numHalfP, numQuantum;

	if (!fInitialized) InitMath(); /* As we do some Dld's */
	StiNum(&num);
	Dld(&num);
	fNegative = DCond() < 0;
	Assert(!fInRegister);

/* BL shameful hack to avoid rounding 0.5 down but 0.5000000000000001 up */
	TenTo(-14);
	Dst(&numQuantum);

	numHalf;
/* BL shameful hack to avoid rounding 0.5 down but 0.5000000000000001 up */
	DAdd(&numQuantum);

	if (fNegative)
		{
		DNeg();
		}
	Dst(&numHalfP);

	if (w != 0)
		{
		TenTo(w);
		Dst(&numPower);
		Dld(&num);
		DDiv(&numPower);
		}
	else
		{
		Dld(&num);
		}

	DAdd(&numHalfP);
	DInt();
	if (w != 0)
		{
		DMul(&numPower);
		}
	fInRegister = fTrue;
	MathCheck();
}


/* %%Function:ExpNum %%Owner:bryanl */
ExpNum()
{
	Assert(fInitialized);
	if (!fInRegister)
		{
		MathStackCheck(POP);
		StackToAcc();
		}
	Exp();
	MathCheck();
}


/* %%Function:LnNum %%Owner:bryanl */
LnNum()
{
	Assert(fInitialized);
	if (!fInRegister)
		{
		MathStackCheck(POP);
		StackToAcc();
		}
	Ln();
	MathCheck();
}



/*------------ Misc. functions for math pack api ------------*/
/* %%Function:MathCheck %%Owner:bryanl */
MathCheck()
{
	int		w;

	Assert(fInitialized);
	if ((w = DError(0)) != 0)
		{
		MathError(w);
		}
}


/* %%Function:MathStackCheck %%Owner:bryanl */
MathStackCheck(dir)
int dir;
{
	Assert(fInitialized);
	if (dir == POP)
		{
		if (inumTop <= 0)
			{
			MathError(fmerrStkUnder);
			}
		}
	else
		{
		Assert(dir == PUSH);
		if (inumTop >= inumMax)
			{
			MathError(fmerrStkOver);
			}
		}
}


/* %%Function:AccToPnum %%Owner:bryanl */
AccToPnum(pnum)
NUM	*pnum;
{
	Assert(fInitialized);
	Assert(fInRegister);
	Dst(pnum);
	fInRegister = fFalse;
}


/* %%Function:AccToStack %%Owner:bryanl */
AccToStack()
{
	Assert(fInitialized);
	Assert(fInRegister);
	Assert(0 <= inumTop && inumTop < inumMax);
	Dst(&rgnumMath[inumTop++]);
	fInRegister = fFalse;
}


/* %%Function:StackToPnum %%Owner:bryanl */
StackToPnum(pnum)
NUM	*pnum;
{
	Assert(fInitialized);
	Assert(0 < inumTop && inumTop <= inumMax);
	bltbyte(&rgnumMath[--inumTop], pnum, sizeof(NUM));
}


/* %%Function:StackToAcc %%Owner:bryanl */
StackToAcc()
{
	Assert(fInitialized);
	Assert(0 < inumTop && inumTop <= inumMax);
	PnumToAcc(&rgnumMath[--inumTop]);
}


/* %%Function:PnumToAcc %%Owner:bryanl */
PnumToAcc(pnum)
NUM	*pnum;
{
	if (!fInitialized) InitMath(); /* As we do some Dld's */
	Dld(pnum);
	fInRegister = fTrue;
}


/* %%Function:PnumToStack %%Owner:bryanl */
PnumToStack(pnum)
NUM	*pnum;
{
	Assert(fInitialized);
	Assert(0 <= inumTop && inumTop < inumMax);
	bltbyte(pnum, &rgnumMath[inumTop++], sizeof(NUM));
}


/* %%Function:MathError %%Owner:bryanl */
EXPORT FAR PASCAL MathError(merr)
{
	extern BOOL fElActive;
	extern ENV *penvMathError;

	Assert(fInitialized);
	ResetMathPack();

	if (fElActive && penvMathError == NULL)
		{
		ElMathError(merr);
		}
	else
		{
		Assert(penvMathError != NULL);
		DoJmp(penvMathError, merr);
		Assert(fFalse);
		}
}


/* %%Function:NumIToI %%Owner:bryanl */
void NumIToI(w0, w1, pnum)
int	w0, w1;
NUM	*pnum;
{
	BOOL	fSign = fFalse;

	if (!fInitialized)
		InitMath();

	switch (w0)
		{
	case 2:
		num2;
		LnNum();
		break;
	case 10:
		numL10;
		break;
	default:
		/* The following two lines of code are machine dependent */
		fSign = (w0 < 0) && (w1 & 0x0001) /* odd */;
		w0 &= 0x7FFF;
		CNumInt(w0);
		LnNum();
		break;
		}
	CNumInt(w1);
	MulNum();
	ExpNum();

	if (fSign)
		{
		numM1;
		MulNum();
		}
	StiNum(pnum);
}


	/* FUTURE: bradv (bcv) This is hideous.  We are doing a bunch
	exponentials and logs for no good reason.  I think it's because
	Excel wanted this so that comparing numbers while having precision
	as displayed would work out.  We don't have precision as displayed.
	Instead we should do
	numDiff = num1-num2;
	w = sign(numDiff);
	if ((base 2 exp in numDiff) - (base 2 exp of max(abs(num1),abs(num2)))
	   < base 2 expEps)
	   w = 0;
	Every thing but the initial floating point subtract can be done
	in fixed point in assembler.
	*/

/* %%Function:WApproxCmpPnum %%Owner:bryanl */
int WApproxCmpPnum(pnum1, pnum2)
NUM	*pnum1;
NUM	*pnum2;
{
	int	wExp, wExpMax, wSign;
	NUM	numE, numD;
	CMPNUMVAL cnv;
	static BOOL	fHasEpsilon = fFalse;
	static NUM	numEpsilon;

	if (!fInitialized)
		InitMath(); /* so we don't trash stuff later */

	Assert(wNumBase > 0);
	if (!fHasEpsilon)
		{
		CNumInt(wEpsilonMant);
		NumIToI(10, wEpsilonExp, &numEpsilon);
		MulINum(&numEpsilon);
		StiNum(&numEpsilon);
		fHasEpsilon = fTrue;
		}

	wExp = ExpBase2Pnum(pnum1);
	wExpMax = ExpBase2Pnum(pnum2);
	if (wExp > wExpMax)
		{
		wExpMax = wExp;
		}
	LdiNum(&numEpsilon);
	NumIToI(wNumBase, wExpMax, &numE);
	MulINum(&numE);
	StiNum(&numE);	/* epsilon * b ^ max(e(u)-q, e(v)-q) */

	LdiNum(pnum1);  /* u - v */
	SubINum(pnum2);
	StiNum(&numD);
#ifdef DBGYXY
	CommSzPnum(SzShared("numE: "), &numE);
	CommSzPnum(SzShared("numD: "), &numD);
#endif

	LdiNum(&numD);
	cnv = CmpINum(&numE);
	if (FGtCmp(cnv))
		{
		return wapxGT;
		}
	else
		{
		LdiNum(&numD);
		NegNum();
		cnv = CmpINum(&numE);
		if (FGtCmp(cnv))
			{
			return wapxLT;
			}
		else
			{
			return wapxEQ;
			}
		}
}




/* Becasue there's no way to get to this number in MathPack. */
/* %%Function:ExpBase2Pnum %%Owner:bryanl */
int ExpBase2Pnum(pnum)
NUM	*pnum;
{
	int	w;
	ENV	*penvSave;
	CMPNUMVAL cnv;
	ENV	env;
	NUM	num;

	if (!fInitialized)
		InitMath();
	if (SetJmp(&env) != 0)
		{
		PopMPJmp(penvSave);
		return 1;
		}
	else
		{
		PushMPJmpEnv(env, penvSave);
		}

	bltbyte(pnum, &num, sizeof(NUM));
#ifdef DBGYXY
	CommSzPnum(SzShared("ExpBase2Pnum: "), &num);
#endif
	num0;
	cnv = CmpINum(&num);
	if (FGtCmp(cnv))
		{
		LdiNum(&num);
		NegNum();
		StiNum(&num);
		}

	LdiNum(&num);
	LnNum();
	num2;
	LnNum();
	DivNum();
	w = CIntNum() + 1;
	PopMPJmp(penvSave);
	return w;
}


/******************** This is machine dependent *******************/
/* %%Function:NumFromL %%Owner:bryanl */
void NumFromL(l, pnum)
long	l;
NUM	*pnum;
{
	NUM	numT;
	BOOL	fMinus, f64K;
	NUMCVT	numcvt;

	if (!fInitialized)
		InitMath();
	numcvt.l = l;
	if (fMinus = l < 0)
		{
		numcvt.w0 *= -1;
		}
	f64K = (numcvt.w1 & 0x8000) != 0;
	numcvt.w1 &= 0x7FFF;

	CNumInt(numcvt.w0);

	NumIToI(2, 16, &numT);/* 2 ^ 16 */
	MulINum(&numT);

	CNumInt(numcvt.w1);
	if (f64K)
		{
		NumIToI(2, 15, &numT);
		AddINum(&numT);
		}
	AddNum();

	if (fMinus)
		{
		numM1;
		MulNum();
		}
	StiNum(pnum);
#ifdef YYDEBUG
	CommSzPnum(SzShared("From NumFromL:"), pnum);
#endif
}


/* M A K E  N U M */
/*  Make a NUM out of lWhole, lFract and put it into *pnum.
*/
/* %%Function:MakeNum  %%Owner:bryanl */
MakeNum (pnum, lWhole, lFract, cd)
NUM *pnum;
LONG lWhole, lFract;
int cd;

{
	NUMCVT	numcvt;
	NUM		num, numT;

	if (!fInitialized)
		InitMath();
	if (lWhole != 0L)
		{
		NumFromL(lWhole, &num);
		if (lFract == 0L)
			{
			bltbyte(&num, pnum, sizeof(NUM));
#ifdef YYDEBUG
			CommSzPnum(SzShared("From MakeNum:"), pnum);
#endif
			return;
			}
		LdiNum(&num);
		}
	else
		{
		num0;
		}

	NumFromL(lFract, &num);
	LdiNum(&num);

	/* 10 ^ cd */
	NumIToI(10, cd, &numT);
	DivINum(&numT);

	AddNum();

	StiNum(pnum);
#ifdef YYDEBUG
	CommSzPnum(SzShared("From MakeNum:"), pnum);
#endif
}


/* %%Function:NumFromSz %%Owner:bryanl */
NumFromSz(sz, pnum)
CHAR    *sz;
NUM	*pnum;
{
	int	cd;
	NUM	numT;
	extern struct ITR       vitr;

	if (!fInitialized)
		InitMath();
	num0;
	for (; (*sz != vitr.chDecimal) && (*sz != '\0'); sz++)
		{
		if (*sz != vitr.chThousand)
			{
			num10;
			MulNum();
			CNumInt(*sz - '0');
			AddNum();
			}
		}
	if (*sz != '\0')
		{
		num0;
		for (cd = 0, sz++; *sz != '\0'; sz++, cd++)
			{
			num10;
			MulNum();
			CNumInt(*sz - '0');
			AddNum();
			}
		if (cd != 0)
			{
			NumIToI(10, cd, &numT);
			LdiNum(&numT);
			DivNum();
			}
		AddNum();
		}
	StiNum(pnum);
}


/* N U M  T O  P U P N U M */
/*  Unpack Num into the upnum structure.
*/

/* %%Function:NumToPupnum  %%Owner:bryanl */
NumToPupnum (pnum, pupnum, cDigBlw)
NUM *pnum;
struct UPNUM *pupnum;
int cDigBlw;
{
	NUM	numT;

	if (!fInitialized)
		InitMath();
	LdiNum(pnum);
	if (cDigBlw != cDigNoRound)
		{
		RoundNum(-cDigBlw);
		}

	UnpackNum(&(pupnum->wExp), &(pupnum->wSign), pupnum->rgch);
	pupnum->cchSig = (int)(pupnum->rgch[0]);
	StToSzInPlace(pupnum->rgch);
}


/* L  W H O L E  F R O M  N U M */
/*  Return a long containing the whole portion of *pnum.
	(Understood that *pnum may not fit into a long in which case the
	return value is undefined).
*/

/* %%Function:LWholeFromNum  %%Owner:bryanl */
LONG LWholeFromNum (pnum, fTruncate)
NUM *pnum;
BOOL fTruncate;
{
	long	  l, lPrev;
	CHAR	 *rgch, *pch;
	struct UPNUM  upnum;

	if (!fInitialized)
		InitMath();
	NumToPupnum(pnum, &upnum, (fTruncate ? cDigNoRound : 0));

	rgch = upnum.rgch;
	if (upnum.wExp <= 0)
		{
		return 0L;
		}
	else  if (upnum.wExp > cDigLong)
		{
lblOvrflw:
		MathError(fmerrOver);
		}
	else
		{
		for (pch = rgch + upnum.cchSig;
				upnum.cchSig < upnum.wExp; upnum.cchSig++)
			{
			*pch++ = '0';
			}
		rgch[upnum.wExp] = '\0';
		for (pch = rgch, l = 0L; *pch != '\0'; pch++)
			{
			lPrev = l;
			l = (l * 10) + (*pch - '0');
			if (l < lPrev)
				{
				/* We just overflowed. */
				goto lblOvrflw;
				}
			}
		return (upnum.wSign * l);
		}
}


/* C C H  S Z  F R A C T  N U M */
/*  Give an ascii representation of the fractional portion of a num.
	Null terminates string.  cch does not include NULL terminator. (but
	cchMax does). Rounds number to fit into cchMax characters.
*/

/* %%Function:CchSzFractNum  %%Owner:bryanl */
CchSzFractNum (pnum, pch, cchMax)
NUM *pnum;
CHAR *pch;
int cchMax;
{
	int cch;
	struct UPNUM upnum;

	if (!fInitialized)
		InitMath();
	NumToPupnum (pnum, &upnum, cchMax - 1);
	if (-upnum.wExp >= cchMax - 1 || upnum.cchSig <= upnum.wExp)
		{
		*pch = '\0';
		return 0;
		}
	else
		{
		CHAR *pchF, *pchDest, *pchLim;
		BOOL	fAllZero;

		pchDest = pch;
		pchLim = pch + cchMax - 1;
		fAllZero = fTrue;
		for (; upnum.wExp < 0 && pchDest < pchLim;
				pchDest++, upnum.wExp++)
			{
			*pchDest = '0';
			}
		pchF = upnum.rgch + upnum.wExp;
		for (cch = 0; (cch + upnum.wExp) < upnum.cchSig &&
				pchDest < pchLim; pchF++, pchDest++, cch++)
			{
			*pchDest = *pchF;
			fAllZero &= *pchF == '0';
			}
		*pchDest = '\0';
		if (fAllZero)
			{
			*(pchDest = pch) = '\0';
			}
		return pchDest - pch;
		}
}


#else
extern AOB ** hrgaob;
/* %%Function:PaobNew %%Owner: */
AOB * PaobNew(aot)
{
	int iaob;
	AOB * paob;
	
	paob = *hrgaob;
	for (iaob = 0; iaob < iaobMax; iaob += 1, paob += 1)
		{
		if (paob->aot == 0)
			{
			SetBytes(paob, 0, sizeof (AOB));
			paob->aot = aot;
			return paob;
			}
		}
	
	return NULL;
}


/* %%Function:CreateAob %%Owner: */
CreateAob(dxWidth, dyHeight)
{
	AOB * paob;

	if (paob = PaobNew(2))
		{
		paob->x = (dxWidth / 4) * (WRand(3) + 1);
		paob->y = dyHeight;
		paob->dx = 2;
		paob->dy = 4;
		paob->irgb = 7;
		paob->ddy = -1;
		paob->xGuess = dxWidth / 2;
		paob->yGuess = dyHeight / 2;
		}
}


/* %%Function:SpawnAobs %%Owner: */
SpawnAobs(x, y)
{
	AOB * paob;
	int i;
	
	for (i = 1; i < 25 + WRand(5); i++)
		{
		if (paob = PaobNew(1))
			{
			paob->x = x + WRand(8) - 4;
			paob->y = y + WRand(8) - 4;
			paob->dx = paob->dy = WRand(3) + 2;
			paob->irgb = 1 + WRand(7);
		
			if ((paob->dxMove = WRand(7) - 4) == 0)
				paob->dxMove = 1;
			paob->dxMove *= 3;
			if ((paob->dyMove = WRand(7) - 4) == 0)
				paob->dyMove = 1;
			paob->dyMove *= 3;
			paob->ddy = 1;
			}
		}
}

/* %%Function:WRand %%Owner: */
WRand(w)
unsigned int w;
{
	static unsigned foo;
	
	foo = (foo << 11) + (foo >> 5) + 13;
	
	return (abs(foo % w));
}
#endif

#ifndef EXTMATH
/*****************************************************************************/
/* 	optimized set of functions for the macro interpreter - kcm 5/23/89   */
/*****************************************************************************/

/* %%Function:StiNum %%Owner:bryanl */
StiNum(pnum)
NUM	*pnum;
{{ /* NATIVE - StiNum */
	if (!fInitialized)
		InitMath();
	if (fInRegister)
		{
		Assert(fInitialized);
		Dst(pnum);
		fInRegister = fFalse;
		}
	else
		{
		Assert(fInitialized);
		Assert(inumTop <= inumMax);
		if (inumTop <= 0)
			MathError(fmerrStkUnder);
		else
			bltbyte(&rgnumMath[--inumTop], pnum, sizeof(NUM));
		}
}}


/* %%Function:LdiNum %%Owner:bryanl */
LdiNum(pnum)
NUM	*pnum;
{{ /* NATIVE - LidNum */
	if (!fInitialized)
		InitMath();
	if (fInRegister)
		{
		Assert(fInitialized);
		Assert(inumTop >= 0);
		if (inumTop >= inumMax)
			MathError(fmerrStkOver);
		else
			Dst(&rgnumMath[inumTop++]);
		}
	Assert(fInitialized);
	Dld(pnum);
	fInRegister = fTrue;
}}


/* %%Function:WSignFromPnum %%Owner:bryanl */
int WSignFromPnum(pnum)
NUM * pnum;
{{ /* NATIVE - WSignFromPnum */
	USTR	ustr;

	LdiNum(pnum);

	Assert(fInitialized);
	Assert(fInRegister);

	if (DCond() == 0)
		{
		fInRegister = fFalse;
		return 0;
		}

	Unpack(&ustr);
	fInRegister = fFalse;
	MathCheck();

	return ((0x8000 & ustr.es) != 0) ? -1 : 1;
}}


/* %%Function:Cmp2Num %%Owner:bryanl */
long Cmp2Num(pnumLeft, pnumRight)
NUM *pnumLeft, *pnumRight;
{{ /* NATIVE - Cmp2Num */
	CMPNUMUN	cl;
	NUM numT;
	uns wHighLeft, wHighRight, wHighNumerator, wHighDenominator;

	LdiNum(pnumLeft);

	Assert(fInitialized);
	Assert(fInRegister);

	DSub(pnumRight);
	StiNum(&numT);
	fInRegister = fTrue; /* numT is still on the floating point stack */

	cl.cnr.w2 = DCond();
	cl.cnr.w1 = 0;

	/* Bypass the approximate comparison if the numbers are not
		the same sign. */
	if (!((*(((uns *)pnumLeft)+3) ^ *(((uns *)pnumRight)+3)) & 0x8000))
		{
		/* Note: The following method of taking absolute value is
			machine dependent. */
		wHighNumerator = *(((uns *)&numT)+3) & 0x7FFF;
		wHighLeft = *(((uns *)pnumLeft)+3) & 0x7FFF;
		wHighRight = *(((uns *)pnumRight)+3) & 0x7FFF;
		wHighDenominator = umax(wHighLeft, wHighRight);
		if (wHighDenominator >= (50 << 4)
				&& wHighNumerator < wHighDenominator - (50 << 4))
			cl.cnr.w2 = 0;
		}


	fInRegister = fFalse;
	MathCheck();
	return(cl.l);
}}


/* %%Function:Add2Num %%Owner:bryanl */
Add2Num(pnumLeft, pnumRight)
NUM *pnumLeft, *pnumRight;
{{ /* NATIVE - Add2Num */
	LdiNum(pnumLeft);

	Assert(fInitialized);
	Assert(fInRegister);

	DAdd(pnumRight);
	MathCheck();
}}



/* %%Function:Sub2Num %%Owner:bryanl */
Sub2Num(pnumLeft, pnumRight)
NUM *pnumLeft, *pnumRight;
{{ /* NATIVE - Sub2Num */
	LdiNum(pnumLeft);

	Assert(fInitialized);
	Assert(fInRegister);

	DSub(pnumRight);
	MathCheck();
}}



/* %%Function:Mul2Num %%Owner:bryanl */
Mul2Num(pnumLeft, pnumRight)
NUM *pnumLeft, *pnumRight;
{{ /* NATIVE - Mul2Num */
	LdiNum(pnumLeft);

	Assert(fInitialized);
	Assert(fInRegister);

	DMul(pnumRight);
	MathCheck();
}}



/* %%Function:Div2Num %%Owner:bryanl */
Div2Num(pnumLeft, pnumRight)
NUM *pnumLeft, *pnumRight;
{{ /* NATIVE - Div2Num */
	LdiNum(pnumLeft);

	Assert(fInitialized);
	Assert(fInRegister);

	DDiv(pnumRight);
	MathCheck();
}}


/******************************************************************************/


#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Mathapi_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* Mathapi_Last() */
#endif
