/*
	mathpack.h:
		public definitions for C# Math Package
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



#ifdef CC
/* definitions of the procedures */

BOOL		FAR PASCAL FInitMathPack(void);
void		FAR PASCAL TermMathPack(void);
void		FAR PASCAL ResetMathPack(void);
TNUM		FAR PASCAL LdiNum(NUM *);
TNUM		FAR PASCAL LdiTnum(TNUM *);
NUM		FAR PASCAL LdiCvtNum(NUM *);
void		FAR PASCAL LdComNum(int);
void		FAR PASCAL LdfiNum(NUM far *);
void		FAR PASCAL StiNum(NUM *);
void		FAR PASCAL StiTnum(TNUM *);
NUM		FAR PASCAL CvtTnum(TNUM);
void		FAR PASCAL CopyNum(NUM *, NUM *);
TNUM		FAR PASCAL AddNum(void); 
TNUM		FAR PASCAL SubNum(void);
TNUM		FAR PASCAL MulNum(void);
TNUM		FAR PASCAL DivNum(void);
TNUM		FAR PASCAL ModNum(void);
TNUM		FAR PASCAL AddINum(NUM *);
TNUM		FAR PASCAL SubINum(NUM *);
TNUM		FAR PASCAL MulINum(NUM *);
TNUM		FAR PASCAL DivINum(NUM *);
TNUM		FAR PASCAL ModINum(NUM *);
TNUM		FAR PASCAL NegNum(void);
CMPNUMVAL	FAR PASCAL CmpNum(void);
CMPNUMVAL	FAR PASCAL CmpINum(NUM *);
int		FAR PASCAL CIntNum(void);
TNUM		FAR PASCAL CNumInt(int);
void		FAR PASCAL UnpackNum(int *, int *, char *);
TNUM		FAR PASCAL RoundNum(int);
TNUM		FAR PASCAL FixNum(int);
TNUM		FAR PASCAL ExpNum(void);
TNUM		FAR PASCAL LnNum(void);

char *		FAR PASCAL SzMathErr(char *, char *, unsigned);

#endif
