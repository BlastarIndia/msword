/* el.h: Embedded Language definitions for use by the application.  Contains
* definitions of all EL data structures which are referred to in the API
* document.
*/
#ifndef HMEM_FIXED
#include <sbmgr.h>
#endif
#ifndef fcmpCompact
#include <lmem.h>
#endif

#ifndef BltDlt
#include <sdmver.h>
#include <sdmdefs.h>	/* copied from Opus -- hack for mixed-version SDM */
#include <sdm.h>
#endif

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

long CmpNum();
long CmpINum();
long LWholeFromNum();
long Cmp2Num();

#define FApxGT(w)	((w) > 0)
#define FApxEQ(w)	((w) == 0)
#define FApxLT(w)	((w) < 0)
#define FApxGE(w)	((w) >= 0)
#define FApxLE(w)	((w) <= 0)
#define FApxNE(w)	((w) != 0)

/* ---------------------------------------------------------------------- */
/* Global variables available outside the interpreter
/* ---------------------------------------------------------------------- */

#ifndef ELMAIN

/* fElActive: flag indicating whether code in the EL interpreter is currently
* running.  This is used for deciding who should handle math errors (and
* also for debugging).  This flag is initialized to FALSE, and maintained by
* the following EL procedures:
*	RerrActivateHeli
*	StopHeli
*	ElvCallElx
*/
extern BOOL fElActive;	/* TRUE in the EL interpreter; FALSE elsewhere */

#endif	/* !ELMAIN */

/* ---------------------------------------------------------------------- */
/* Basic EL definitions
/* ---------------------------------------------------------------------- */

/* ELG (EL Group) -- identifies a group of modules which
* occupy the same code segment (and share a symbol table segment).
*/
typedef WORD ELG;
#define	elgNil	0
#define elgCur	0xFE
#define	elgAll	0xFF

/* ELM (EL Module) -- identifies a module within a group.
*/
typedef WORD ELM;
#define elmNil	0
#define elmCur	0xFE
#define elmAll	0xFF

typedef LONG LIB;	/* long ib: pointer into program */
#define libNil		(-1L)

typedef WORD ELX;	/* EL eXternal statement or function */
#define elxNil		0	/* the only EL-defined value */

typedef WORD ELK;	/* EL Keyword argument -- LIMITED TO 13 BITS */
#define elkNil		0x1fff	/* the only EL-defined value */

typedef WORD RERR;

#ifndef hpNil
#define hpNil	HpOfSbIb(0, 0)
#endif


/* --- Math definitions --- */

#define cbNUM	(sizeof(NUM))

typedef struct /* MaTh Comparison value */
	{
	int w1, w2;
	} MTC;

/* --- Array definitions --- */

typedef WORD AD;
typedef WORD AS;	/* Array Subscript */
struct AHR		/* Array Header Record */
	{
	unsigned cbElement : 4;
	unsigned cas : 4;
	AS rgas[];
	/* Array data follows ... */
	};
#define CbOfAhr(cas,cbData)	(sizeof(struct AHR) + (cas) * sizeof(AS) +   \
					(cbData))
#define PvalOfPahr(pahr,cas)	((char *)(pahr) + sizeof(struct AHR) +	\
					(cas) * sizeof(AS))
#define PahrOfAd(ad)		((struct AHR *)*HpOfSbIb(sbArrays, (ad)))
#define HpahrOfAd(ad)		((struct AHR huge *)			\
					HpOfSbIb(sbArrays,			\
						*HpOfSbIb(sbArrays, (ad))))
#define HpahrOfPahr(pahr)	((struct AHR huge *)HpOfSbIb(sbArrays, (pahr)))



#define casMax	10

/* --- String definitions --- */

struct _string16
	{
	WORD cch;
	BYTE rgch[0];
	};
typedef struct _string16 *S16;
typedef struct _string16 huge *HS16;

typedef WORD SD;
#define sdNil	0xffff
#define CchFromSd(sd)	(*(WORD huge *)HpOfSbIb(sbStrings,		\
						*HpOfSbIb(sbStrings, (sd))))
#define HpchFromSd(sd)	((BYTE huge *)(&CchFromSd(sd) + 1))
#define Hs16FromSd(sd)	((HS16)&CchFromSd(sd))


/* Dialog Structure Strings -- these are strings containing dialog information.
* These strings are not necessarily printable; they may have a bunch of non-
* ASCII stuff in them.
*/
typedef SD DSS;

typedef WORD ELF;	/* EL Field type -- type of a value in a DSS */

#define elfNil		0	/* field is empty */
#define elfInt		1	/* %<byte 0><byte 1> */
#define elfNum		2	/* #<byte 0>...<byte 7> */
#define elfString	3	/* " ... string text ... " */
#define elfArray	4	/* (<elem 0>,<elem 1>, ... ) */
#define elfRecord	5	/* {<elk 0><value 0>,<elk 1><value 1>, ... } */


/* ---------------------------------------------------------------------- */
/* SDM-related definitions
/* ---------------------------------------------------------------------- */
#define tmcNil		0xffff
#define iagNil		0xffff
typedef WORD HID;
#define hidNil		0xffff

typedef struct _rek
	{
	HCAB hcab;
	WORD ielfdButton;
	} REK;

typedef struct _rps
	{
	WORD iag;
	WORD ielfd;
	} RPS;

typedef struct _rl	/* Record Locator -- points to a record in a variable */
	{
	WORD ib;		/* location in array, if any */
	struct VAR *pvar;	/* if ib != ibNil, pvar refers to an array */
	} RL;

/* *** WARNING: RADIOACTIVE ***
* The following stuff is SDM private definitions (!) stolen from the SDM
* header files.  This is necessary for CAB access by EL code which deals
* with records.  These definitions will have to change if the SDM internals
* change.	-jhm
*/
typedef struct _cabx	/* generic CAB */
	{
	CABH	cabh;
	WORD	sab;
	HANDLE	rgh[1];
	}  CABX;

#define PcabxOfHcab(hcab)	((CABX *)(*hcab))
#define PpvFromCab(hcab, iag)	((VOID **)(PcabxOfHcab(hcab)->rgh[iag]))
#define WFromCab(hcab, iag)	((WORD)(PcabxOfHcab(hcab)->rgh[iag]))

/* Here is a new macro which would also come under the category of
* "SDM private" ...
*/
#define PvFromCab(hcab, iag)	((VOID *)(&PcabxOfHcab(hcab)->rgh[iag]))

/* ---------------------------------------------------------------------- */
/* Segment Usage
/* ---------------------------------------------------------------------- */

#define sbModule	3	/* symbol table -- tokenizer use only */

#define	sbProg		24	/* program segment used by ELTEST */
#define sbCoTest	25	/* random segment for coroutine testing */

#define csbNtabMax	16	/* how many symbol table segments can exist */
#define sbElLim		26	/* upper bound on sb's used by EL */


/* ---------------------------------------------------------------------- */
/* Stack space constraints
/* ---------------------------------------------------------------------- */

/* Amount of stack space that must be available in order to start running
* a macro.
*/
#define cbElStackNeeded		3072

/* Amount of stack space that must be available in order to start running
* an EL procedure.
*/
#define cbProcStackNeeded	5120  /* should be about 1/2 total stack */

/* Limit on the size of EL frames on the stack, at the time an ELX is
* called.
*/
#define cbElStackUsage		2048


/* ---------------------------------------------------------------------- */
/* Definitions for non-EL procedures callable from within EL code
/* ---------------------------------------------------------------------- */

#define celpMax		16

/* EL Entry -- describes an external procedure.
*/
typedef struct _ele
	{
	void (*pfn)();			/* function entry */
	WORD ieldi;			/* if elr == elrDialog */
	BYTE celpMin, celpMac;
	unsigned elr : 3;		/* return type */
	unsigned fStmt : 1;		/* callable as statement */
	unsigned fFunc : 1;		/* callable as function */
	unsigned : 3;
	BYTE rgelp[celpMax];
	} ELE;
/* NOTE: "ele.pfn" is not necessarily a void function; it may return any
*	 type (including void).  The field "ele.elr" tells the caller what
*	 return type to expect.
*/

/* EL Dialog Info -- describes a command equivalent statement (and the
* corresponding predefined record type).
*/
typedef struct _elfd
	{
	union	{
		TMC tmc;
		WORD iag;
		};
	unsigned elv;
	ELK elk;
	} ELFD;
	
typedef struct _eldi
	{
	HID hid;
	CABI cabi;
	unsigned celfd;
	ELFD rgelfd[];
	} ELDI;
#define ieldiNil	(-1)
#define ielfdNil	(-1)
#define CbEldiOfCelfd(celfd)	(sizeof(ELDI) + (celfd) * sizeof(ELFD))

/* "heldi" handles point to ELDI's in the temporary data segment ...
*/
#define HpeldiFromHeldi(heldi)	((ELDI huge *)				\
					HpOfSbIb(sbTds,			\
						*HpOfSbIb(sbTds, (heldi))))


/* ELR (EL entry Return type) -- defines the return value.
*/
typedef WORD ELR;
#define elrVoid		0	/* no return value */
#define elrNum		1	/* floating point NUM */
#define elrInt		2	/* 16-bit integer */
#define elrSd		3	/* string descriptor */
#define elrAd		4	/* NOT IMPLEMENTED: array */
#define elrDialog	5	/* dialog action statement (NYI) */

/* ELP (EL entry Parameter type) -- defines the type of a parameter.
* Note: this is a 3-bit quantity (and thus is limited to 8 values).
*/
typedef WORD ELP;
#define elpNum		0	/* floating point NUM */
#define elpInt		1	/* integer */
#define elpHst		2	/* short string on application heap */
#define elpHpsd		3	/* string on EL string heap */
#define elpAdNum	4	/* NOT IMPLEMENTED: NUM array descriptor */
#define elpAdInt	5	/* NOT IMPLEMENTED: INT array descriptor */
#define elpAdSd		6	/* NOT IMPLEMENTED: STR array descriptor */
#define elpHpsdNum	7	/* NUM or string (both args passed) */

/* ARD (ARgument Descriptor)
*/
typedef union _ard
	{
	NUM num;
	WORD w;
	SD huge *hpsd;
	AD huge *hpad;
	} ARD;


/* ---------------------------------------------------------------------- */
/* Definitions for tokenization of EL programs (done by the application)
/* ---------------------------------------------------------------------- */

/* ELT (EL Token) -- identifies a language token.
* The ELT's are ordered in the following way:
*	- eltNil = 0
*	- unary (prefix) operators (ELT < eltUnaryLim)
*	- binary operators (eltUnaryLim <= ELT < eltOprLim)
*	- non-operator ELT's
*/
typedef WORD ELT;
#include "elt.h"

#define eltUnaryLim	3
#define eltOprLim	23
#define eltElccMin	85

#define EltFromElcc(elcc)	((elcc) - elccLineNolabel + eltElccMin)

/* ELCC (EL Control Character) -- begins a token sequence within EL code.
* The tokenizer (within Opus) is responsible for inserting these things.
*/
typedef WORD ELCC;
#define elccNil		0	/* never inserted into document */
#define elccLineTooLong	1	/* never inserted into document */

#define elccMin		14

#define elccLineNolabel	(elccMin + 0)
#define elccLineIdent	(elccMin + 1)
#define elccLineInt	(elccMin + 2)
#define elccElt		(elccMin + 3)
#define elccElx		(elccMin + 4)
#define elccElk		(elccMin + 5)
#define elccNum		(elccMin + 6)
#define elccIdent	(elccMin + 7)
#define elccString	(elccMin + 8)
#define elccComment	(elccMin + 9)
#define elccInt		(elccMin + 10)
#define elccEof		(elccMin + 11)
#define elccStmtSep	(elccMin + 12)	/* Statement separator (":") */
#define elccBadSyntax	(elccMin + 13)
#define elccSpace	(elccMin + 14)
#define elccRem		(elccMin + 15)
#define elccHash	(elccMin + 16)
#define elccUserElk	(elccMin + 17)
#define elccExtLine	(elccMin + 18)

#define elccLim		(elccMin + 19)

#define chEof		0x80
#define cchIdentMax	40

#define FIsElccCh(ch)	((ch) >= elccMin && (ch) < elccLim)

#define CbElcr(rgchLastField)	((int)&((struct ELCR *)0)->rgchLastField)
#define CbFromElcc(elcc)	(mpelcccb[(elcc) - elccLineNolabel])

/* ELCR (EL Control Record) -- contains an ELCC and arguments (variable
* length).  The tokenizer inserts ELCR's into the program text.  For each
* ELCC, the appropriate ELCR size can be found by looking up in the
* following array:
*
* BYTE mpelcccb[] = {
*      CbElcr(rgchEndLineNolabel),
*	CbElcr(rgchEndLineIdent), CbElcr(rgchEndLineInt), CbElcr(rgchEndElt),
*	CbElcr(rgchEndElx), CbElcr(rgchEndElk), CbElcr(rgchEndNum),
*	CbElcr(rgchEndIdent), CbElcr(rgchEndString), CbElcr(rgchEndOther),
*	CbElcr(rgchEndInt), CbElcr(rgchEndOther)
*	};
*/
struct ELCR
	{
	BYTE elcc;
	union	{
		struct	{		/* elccUserElk */
			BYTE cch;
			BYTE rgchEndUserElk[0];
			};
		struct	{		/* elccBadSyntax */
			BYTE cch;
			BYTE rgchEndBadSyntax[0];
			};
		struct	{		/* elccSpace */
			BYTE cch;
			BYTE rgchEndSpace[0];
			};
		struct	{		/* elccRem, elccComment */
			BYTE cch;
			BYTE rgchEndRem[0];
			};
		struct	{		/* elccNum */
			NUM num;
			BYTE rgchEndNum[0];
			};
		struct	{		/* elccLine* */
			BYTE cchLine;
			WORD ib;
			union	{
				struct	{
					WORD ln;
					BYTE rgchEndLineInt[0];
					};
				struct
					{
					BYTE cchLineIdent;
					BYTE rgchEndLineIdent[0];
					};
				BYTE rgchEndLineNolabel[0];
				};
			};
		struct	{		/* elccElx */
			ELX elx;
/* REMOVED: BAC 	BYTE fInteractive;	/* whether "?" specified */
			BYTE rgchEndElx[0];
			};
		struct	{
			ELK elk;	/* elccElk (may change) */
			BYTE rgchEndElk[0];
			};
		struct	{		/* elccIdent */
			BYTE cchIdent;
			BYTE rgchEndIdent[0];
			/* The identifier text follows cchIdent immediately
				* in the code stream, so it looks like a counted
				* string.
				*/
			};
		struct	{		/* elccString */
			BYTE cchString;
			BYTE rgchEndString[0];
			/* The string text follows immediately.  Note that this
				* ELCR is inserted just after the first double-quote.
				*/
			};
		struct	{		/* elccElt */
			BYTE elt;
			BYTE rgchEndElt[0];
			};
		struct	{		/* elccInt */
			WORD wInt;
			BYTE rgchEndInt[0];
			};
		struct {
			char st [1];
			};
		BYTE rgchEndOther[0];
		};
	};

/* ---------------------------------------------------------------------- */
/* Definitions for run-time interface
/* ---------------------------------------------------------------------- */

/* ELD (EL Debug code) -- indicates the reason for a call to the debug
* function.
*/
typedef WORD ELD;
#define eldNormalStep	0
#define eldRtError	1
#define eldExecEnd	2
#define eldExecStop	3
#define eldSubIn	4
#define eldSubOut	5


/* ELV (EL Variable type) -- returned from ElvEnumerateVar(); indicates the
* type of an object on the stack.
*/
typedef WORD ELV;
#define elvNil			0

#define elvNum			1	/* DBL number; pval = pnum */
#define elvInt			2	/* INT number; pval = pint */
#define elvSd			3	/* STR string; pval = psd */
#define elvArray		4	/* Array of strings; pval = pad */
#define elvFormal		5	/* Formal parameter: pval = pvar */
#define elvEnv			6	/* ON ERROR env. (internal use) */
#define elvEndStack		7	/* no more items on stack */
#define elvEndModuleFrame	8	/* no more items in module frame */
#define elvEndProcFrame		9	/* no more items in proc frame */

/* Special ELV's used in the ELDI structure (describing dialog items).
*/
#define elvListBoxInt		elvInt	/* list box (producing an index) */
#define elvListBoxString	elvSd	/* list box (producing a string) */
#define elvListBoxRgint		12	/* list box (multiple selection) */
#define elvIntOrSd		13
#define elvComboEditItem	elvSd	/* edit item in a combo box */

/* ELV's describing the record types which have been defined.
*/
#define elvNilRecord		14	/* ElvFromIeldi(ieldiNil) */
#define elvFirstIeldi		15

#define FRecordElv(elv)		((elv) >= elvNilRecord)
#define ElvFromIeldi(elx)	((elx) + elvFirstIeldi)
#define IeldiFromElv(elv)	((elv) - elvFirstIeldi)


/* Dyna-linK library parameter/return value Types */
typedef int DKT;
#define dktInt		0
#define dktLong		1
#define dktSingle	2
#define dktDouble	3
#define dktString	4
#define dktAny		5
#define dktNone		6

#define CbFromCdkt(cdkt)	(cdkt)

/* Dyna-linK library call Descriptor */
typedef struct _dkd
	{
	int dktRet : 3;
	int idktMacParam : 4;
	int fFreeLib : 1;
	int bSpare : 8;
	
	long (FAR PASCAL * lppasproc)();
	HANDLE hLib;
		
	char rgdkt [0];	/* DKT (rgdktParam : 4) [self.idktMacParam] */
	} DKD;

#define CbOfDkdIdktMac(idktMac)	(sizeof (DKD) + CbFromCdkt(idktMac))

/* ELI (EL Instance)
*/
typedef struct _eli
	{
	/* Information likely to be referenced by the application.  (also
		* see szErrorString)
		*/
	ELG elg;
	ELM elm;
	WORD w;
	LONG l;
	LIB lib;
	union	{
		char huge *hpst;
		RERR rerr;
		};

	/* Saved callback pointers.
		*/
	WORD (*pfnCchReadSource)();
	VOID (*pfnDebug)();
	VOID (*pfnGetInfoElx)();
	VOID (*pfnCallElx)();
	VOID (*pfnGetNameElk)();

	/* ELI status.
		*/
	unsigned fStarted : 1, fSuspended : 1;

	/* "next" ELI -- i.e. the one that was running when this one was
		* created.
		*/
	struct _eli **heliNext;

	/* Pointer to local data.  If this ELI is the active one (i.e.
		* peli == pelgdGlobal->peliFirst), then pelldGlobal == peli->pelld.
		*/
	struct ELLD *pelld;

	/* Error String (valid if rerr == rerrErrorString).
		*/
#define cchErrorStringMax	80
	char szErrorString[cchErrorStringMax];
	} ELI;
#define HpeliOfPeli(peli)	((ELI huge *)HpOfSbIb(sbTds, (peli)))
#define HpeliOfHeli(heli)	((ELI huge *)HpOfSbIb(sbTds,		\
				*HpOfSbIb(sbTds, (heli))))


/* File Open Modes for OPEN statement */
#define fomOutput	0
#define fomInput	1
#define fomAppend	2



/* ---------------------------------------------------------------------- */
/*  EL API Procedures
/* ---------------------------------------------------------------------- */

ELI **HeliNew(ELG, ELM, WORD (*)(), VOID (*)(), VOID (*)(), WORD, LONG);
RERR RerrRunHpeli(ELI huge *);
VOID StopHpeli(ELI huge *, RERR);
VOID FreeHpeli(ELI huge *);
VOID SetDebug(ELG, ELM, VOID (*)());
ELV ElvEnumerateVar(int, int, char *, ELG *, ELM *, VOID **);

SD SdCopySz(char *);
SD SdCopySt(char *);

VOID RtError(RERR);


#define elFTrue -1
#define elFFalse 0

/* picture string for number formatting for El */
#define szElNumPic	SzShared("-0.xxxxxxxxxx")
#define cElDigBlwDec 10

#define unsMax (65535)
#define cchMaxLine	255 /* max value of a byte */
