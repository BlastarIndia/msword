/* O U R M A T H . H */

/*  structures, definitions and function declarations for the
	Math Package. */

#ifndef NUMDEFINED
typedef struct
	{
	union
		{
		char rgb[8];
		double d;
		} num;
	} NUM;
#define NUMDEFINED
#endif

#define cwNUM	CwFromCch(sizeof(NUM))
/* The depth of the stack - 1 (for acc.) */
#define inumMax 7


#define TNUM void


/* what the API says CmpNum returns */

typedef struct
	{
	int w1, w2;
	} CMPNUMRET;

/* really CmpNum returns a long */

#define CMPNUMVAL long

typedef union
	{
	CMPNUMRET cnr;
	long l;
	} CMPNUMUN;

/* macros for comparisons */

#define FLtCmp(cnv) (cnv < 0)
#define FGtCmp(cnv) (cnv > 0)

/*
* The following comparison macros are not really meaningful for real
* numbers, since real numbers are inherently imprecise, but are
* included for completeness.  If you really need a test that includes
* a test for equality, you should use a "fuzzy compare".
*/

#define FEqCmp(cnv) (cnv == 0)	/* almost always false */
#define FLeCmp(cnv) (cnv <= 0)
#define FGeCmp(cnv) (cnv >= 0)
#define FNeCmp(cnv) (cnv != 0)


/* Math error masks */
#define fmerrNonfatal	0x000F  /* Non fatal error */
#define fmerrFatal	0x0F00  /* Fatal error */
/* error values passed to MathError */

#define fmerrOver	0x0001	/* numeric overflow */
#define fmerrUnder	0x0002	/* numeric underflow */
#define fmerrDiv0	0x0004	/* division by zero */
#define fmerrTrans	0x0008	/* transcendental function error */

#define fmerrStkOver	0x0100	/* stack overflow */
#define fmerrStkUnder	0x0200	/* stack underflow */


/* Important constants */
#define wNumBase	2
#define wNumExcess      0x3FF
/* includes the cch byte. */
#define cchMaxSig 16


/* temporary simulation stuff until we get the real mathpack */
typedef union {
		struct {
			unsigned w1;
			int	     w0;
			};
		LONG    l;
		} NUMCVT;

/* unpacked num */
struct UPNUM
	{
	int wSign;
	int wExp;
	int cchSig;
	char rgch [cchMaxSig];
	};


#ifndef MATHONLY
/* used for the field calculation result */
union RRU
	{ /* Registered Result Union */
	NUM num;
	struct DTTM dttm;
	long l;
	};
#define rrutNil     0
#define rrutNum     1
#define rrutDttm    2
#define rrutLong    3
#endif


/* functions to/from NUM */
LONG LWholeFromNum ();
MakeNum ();
CchSzWholeNum ();
CchSzFractNum ();
GetSzFromNum ();
NumToPupnum ();
NumFromSz();

#define cDigNoRound	0x7FFF	/* Do not round in NumToPupnum */

#ifndef MATHONLY
/* Some convenient NUM constants */
#define num0	LdComNum(0)
#define num1	LdComNum(1)
#define num2	LdComNum(2)
#define numM1	LdComNum(3)
#define num10	LdComNum(4)
#define num100	LdComNum(5)
#define numL10	LdComNum(6)
#define numHalf	LdComNum(7)
#define numNegBig LdComNum(8)
#define icnumMax	8
#endif


/* Define epsilon used for equality comparison. */
/* For its description see pg 217 - 219 of "The
	Art of Computer Programming vol. 2, 2nd ed.,
	Seminumerical Algorithms."  Donald E. Knuth. */
/* This number came out to be about .18 * 10^-14 */
#define wEpsilonMant	18
#define wEpsilonExp	(-16)

/* Maximum # of ENV's stacked for DoJmp's out of a mathpack. */
#define ienvMax		6

/* A number of digits in 0x7FFFFFFF */
#define cDigLong	10

#define wapxGT		1
#define wapxEQ		0
#define wapxLT		-1

#define FApxGT(w)	((w) > 0)
#define FApxEQ(w)	((w) == 0)
#define FApxLT(w)	((w) < 0)
#define FApxGE(w)	((w) >= 0)
#define FApxLE(w)	((w) <= 0)
#define FApxNE(w)	((w) != 0)

/* Math Error binding stuff. */
#define PushMPJmpEnv(_env, _penv)		\
		((_penv) = penvMathError, penvMathError = &(_env))
#define PopMPJmp(_penvPrev) (penvMathError = (_penvPrev))


long CmpNum();
long CmpINum();
long Cmp2Num();

#ifndef MATHONLY
typedef struct {
	CHAR	rgch[15];
	CHAR	cch;
	uns	es;	/* exponent and sign. */
	} USTR;         /* struct used by Unpack */
#endif
