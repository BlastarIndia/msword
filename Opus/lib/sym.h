/* Definitions for symbol table manager */

/* Generic segment definition ...
*/
struct SSEG	/* symbol table segment (either NTAB or VARS) */
	{
	unsigned ibMac, ibMax;		/* current or allocated size */
	};

/* ---------------------- NAME TABLE RECORDS -----------------------
*/
#define cwHash	13	/* size of cbNtab hash table */
#define cbHash	(cwHash * sizeof(struct SY *))

struct NTAB	/* name table */
	{
	struct SSEG;			/* must be first */
	ELG elg;			/* identify macro */
	ELM elm;
	SB sbNext;			/* next NTAB (or 0) */
	LIB libScan;			/* labels and procs defined */
	struct PROC *pprocScan;		/* what PROC the scan is in */
	struct SY *rgpsyHash[cwHash];	/* NTAB ptr: the hash table */
	char rgb[];			/* the rest of the segment */
	};

/* SYT (SYmbol Type) -- a code specifying the meaning of an SY.
*/
typedef WORD SYT;
#define sytNil		0
#define sytProc		1
#define sytVar		2
#define sytExternal	3
#define sytDkd		4		/* Dyna-linK */

struct SY
	{
	struct SY *psyNext;		/* next SY with same hash value */
	struct LAB *plabFirst;		/* label definition */
	union	{
		DKD * pdkd;		/* Dyna-linK Descriptor */
		struct PROC *pproc;	/* procedure definition */
		struct	{		/* innermost variable definition */
			struct VAR *pvarScalar;
			struct VAR *pvarArray;
			struct VAR *pvarRecord;
			};
			};
	SYT syt : 4;			/* SY type */
	ELV elv : 4;			/* ELV represented by name */
	char st[];			/* symbol name */
	};
#define CbOfSy(cch)	(sizeof(struct SY) + (cch) + 1)	/* +1 for '\0' term. */
#define psyNil	((struct SY *)0)
#define HpsyOfPsy(psy)	((struct SY huge *)HpOfSbIb(Global(sbNtab), (psy)))


struct LAB
	{
	struct LAB *plabNext;
	struct PROC *pproc;
	LIB lib;
	};

struct PROC
	{
	struct SY *psy;	/* back-link to name */
	LIB lib;
	ELV elv;	/* return type (if FUNCTION), or elvNil (if SUB) */
	int carg;
	struct ARG {
		struct SY *psy;
		ELV elv;
		unsigned fArray : 1;
		} rgarg[];
	};
#define CbOfProcCarg(carg)	(sizeof(struct PROC) +			\
					(carg) * sizeof(struct ARG))
#define HpprocOfPproc(pproc)	((struct PROC huge *)			\
					HpOfSbIb(Global(sbNtab), (pproc)))

/* Macros for accessing the Name Table.  HpNtab can be used any
* time; NtabVar should only be used when sbCur is Global(sbNtab).
*/
#define NtabVar(var)	(((struct NTAB *)0)->var)
#define HpNtab(var)	((int huge *)HpOfSbIb(Global(sbNtab), &NtabVar(var)))

/* ------------------------ FRAME STACK RECORDS ---------------------- */

struct VAR
	{
	struct SY *psy;		/* name */
	struct VAR *pvarPrev;	/* VAR shadowed by this one */
	ELV elv;		/* type */
	int fShared : 1;	/* whether SHARED */
	unsigned cdim : 4;	/* # of array dims (or 0) */
	union	{
		NUM num;
		WORD w;
		SD sd;
		struct AI ai;
		struct {
			REK rek;
			ELX elx;
			};
		ENV env;
		struct VAR *pvar;	/* elv == elvNil --> an argument */
		};
	};

#define cbNumVar	((int)&((struct VAR *)0)->num + sizeof(NUM))
#define cbIntVar	((int)&((struct VAR *)0)->w + sizeof(WORD))
#define cbSdVar		((int)&((struct VAR *)0)->sd + sizeof(SD))
#define cbAdVar		((int)&((struct VAR *)0)->ai + sizeof(struct AI))
#define cbFormalVar	((int)&((struct VAR *)0)->pvar + sizeof(struct VAR *))
#define cbRecordVar	((int)&((struct VAR *)0)->rek + sizeof(REK) + sizeof (ELX))
#define cbEnvVar	((int)&((struct VAR *)0)->env + sizeof(ENV))


#define HpvarOfPvar(pvar)	((struct VAR huge *)HpOfSbIb(sbFrame, (pvar)))

/* SYM interface procedures */

VOID ClearSymbols(ELG, ELM);
struct SY *PsyLookupSz(char *, BOOL);
struct VAR *PvarFromPsy(struct SY *, ELV, BOOL);
struct PROC *PprocFromPsy(struct SY *, int);
struct LAB *PlabFromPsy(struct SY *, WORD, struct PROC *);
VOID BeginPproc(struct PROC *);
VOID EndProcedure(), EndAllProcedures();
struct VAR *PvarBeginTemps();
VOID EndTemps(struct VAR *);

