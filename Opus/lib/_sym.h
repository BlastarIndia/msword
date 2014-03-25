/* _sym.h: private SYM definitions.
*/
#define cbSymQuantum	512	/* allocated size of segment free areas */

struct VARS	/* sbFrame -- frame stack segment */
	{
	struct SSEG;			/* must be first */
	struct FH *pfhLast;		/* last frame header defined */
	char rgb[];
	};

struct FH	/* Frame Header */
	{
	struct PROC *pproc;		/* PROC that this frame is for */
	struct FH *pfhPrev;		/* link to previous frame */
	ENV *penv;			/* ON ERROR env pointer, or 0 */
	ELG elg;			/* module ID (for module frames) */
	ELM elm;
	struct VAR rgvar[];		/* VAR's in this frame */
	};
#define pfhNil	((struct FH *)0xffff)
#define HpfhFromPfh(pfh)	((struct FH huge *)HpOfSbIb(sbFrame, (pfh)))


/* Macros for accessing the frame stack (sbFrame) ...
*/
#define FrameVar(var)	(((struct VARS *)0)->var)
#define HpFrame(var)	((int huge *)HpOfSbIb(sbFrame, &FrameVar(var)))


/* Allocation macros for symbol table records ...
*/
#define AllocNtab(cbWanted)						\
	(*HpNtab(ibMac) += (cbWanted),					\
		*HpNtab(ibMac) > *HpNtab(ibMax) ? GrowSb(Global(sbNtab)) : 0,	\
		*HpNtab(ibMac) - (cbWanted))

#define AllocFrame(cbWanted)						\
	(*HpFrame(ibMac) += (cbWanted),					\
		*HpFrame(ibMac) > *HpFrame(ibMax) ? GrowSb(sbFrame) : 0,	\
		*HpFrame(ibMac) - (cbWanted))


/* ------------------------------------------------------------------- */
/* ------------------------- KEYWORD TABLE --------------------------- */
/* ------------------------------------------------------------------- */

typedef WORD KEYO;		/* keyword offset */
#define keyoNil		0xffff

struct KEY
	{
	KEYO keyoNext;
	BYTE elt;
	char st[];
	};

#define cbKeyDataMax	2048	/* just matters for initialization */

