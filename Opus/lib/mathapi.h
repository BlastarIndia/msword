/*
* definitions for C# Math Package
*/

typedef struct
{
	union
	{
		char rgb[8];
		double d;
	} num;
} NUM;


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


/* error values passed to MathError */

#define fmerrOver	0x0001	/* numeric overflow */
#define fmerrUnder	0x0002	/* numeric underflow */
#define fmerrDiv0	0x0004	/* division by zero */
#define fmerrTrans	0x0008	/* transcendental function error */

#define fmerrStkOver	0x0100	/* stack overflow */
#define fmerrStkUnder	0x0200	/* stack underflow */

#define fmerrNonfatal	0x000F  /* Non fatal error */
#define fmerrFatal	0x0F00  /* Fatal error */


/* Important constants */
#define wNumBase	2
#define wNumExcess      0x3FF
#define cchMaxSig 18

