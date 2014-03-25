/* exec.h: private EXEC definitions */

#define ccsrStackMax	128


/* CST -- Control Structure Type
*/
typedef WORD CST;
#define cstNil		0
#define cstBlockElse	1
#define cstBlockEndif	2
#define cstSingleElse	3
#define cstNext		4
#define cstWend		5
#define cstCase		6
#define cstEndProc	7

/* CSR -- Control Structure Record
*/
struct CSR
	{
	int cst;
	LIB libStart;		/* where the thing started */
	union	{
		struct	{	/* cstEndProc */
			struct VAR *pvarEndProc;	/* for EndTemps() */
			};
		struct	{	/* cstNext */
			struct VAR *pvarNext;		/* index variable */
			NUM numNextTo;			/* TO value */
			NUM numNextStep;		/* STEP value */
			int sgnNextStep;
			};
		};
	};

#define HpcsrOfIcsr(icsr)						\
	((struct CSR huge *)						\
		HpOfSbIb(sbTds, *HpOfSbIb(sbTds, ElGlobal(hcsrStack)) +	\
				(icsr) * sizeof(struct CSR)))

#define HpcsrOfPcsr(pcsr)	((struct CSR huge *)HpOfSbIb(sbTds, (pcsr)))

uop void VoidUop();
uop WORD WordUop();
uop NUM NumUop();

#define StackNum(num)	VoidUop(0, (num))
#define	StackW(w)	VoidUop(0, (w))
#define StackL(l)	VoidUop(0, (l))
#define WordFromStack()	WordUop(0)
#define NumFromStack()	NumUop(0)


