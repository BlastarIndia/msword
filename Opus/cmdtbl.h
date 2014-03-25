/* cmdtbl.h */

#define wHashMax 127

#define bsyNil 0xffff
#define icmdNil bsyNil

/* Macro Types */

typedef unsigned MCT;

#define mctNil		0
#define mctCmd          1
#define mctEl           2
#define mctSdm		3
/* 4 */
/* 5 */
#define mctMacro        6
#define mctGlobalMacro  7

#define mctAll          8 /* used with EnumMacros() to get all non-El mct's */
#define mctGlobal       9 /* same, but for commands and global macros only */
#define mctMacros	10

#define FElMct(mct)     ((mct) == mctEl)


#ifdef REMOVE /* we use elp's now (see el.h) */
/* Argument Types */

#define agtEither       0 /* number or string */
#define agtInt          1
#define agtReal         2
#define agtString       3
#define agtSd           4
#endif

/* Argument Descriptor */

#ifdef FAKED
struct AGD
		{
		char agt : 3;
		char iLetter : 5;
		};
#endif /* FAKED */

#define cbAGD   1



/* SYmbol */

/* One of these exists for each command, statement, function, and
macro available.

NOTES:
		* When st[0] == 0, this symbol has the same name as the previous one.
*/
typedef struct _sy
		{
		unsigned bsyNext;
		union {
				struct {                /* Commands */
						PFN pfnCmd;
						uns ucc : 4;
			uns fSetUndo : 1;
			uns fRepeatable : 1;
			uns fWParam : 1;
						uns iidstr : 9;
						};
				struct {                /* EL Statements and Functions */
						PFN pfnEl;
						uns cagdMin : 4;
						uns cagdMax : 4;
						uns elr : 3;
			uns fStmt : 1;
			uns fDyadic : 1;
			uns spare : 3;
						};
				struct {                /* Macros */
						uns imcr;
						uns docDot : 8;
						uns ibstMenuHelpP1 : 8;
						};
				};

		union {
				struct {
						uns mct : 3;
						uns fRef : 1; /* used during save */
						uns : 1;
						uns fMomLock : 1;
						uns fNeedsDoc : 1;
						uns fExtendMode : 1;
						uns fBlockMode : 1;
						uns fMacroMode : 1;
						uns fPreviewMode : 1;
						uns fFootnoteMode : 1;
						uns fHeaderMode : 1;
						uns fOutlineMode : 1;
						uns fAnnotateMode : 1;
						uns fLockedMode : 1;
						};
				int grf;
				};

		char stName[1];

#ifdef FAKED
		union {
		struct {
			CABI cabi;
			int ieldi;
			};
				ARD rgard [0];
				AGD rgagd [0];
				char rgchMenuHelp [0];
				};
#endif
		} SY;


#define cbSy (sizeof (SY)) /* REMOVE */
#define cbSY (sizeof (SY))


typedef SY HUGE * HPSY;
typedef SY FAR * LPSY;

#define iidstrNil 0x1ff

#define iardMax 32
#define cchMaxSyName 42 /* 40 characters + length + '\0' */
#define cbMaxSy (cbSY + cchMaxSyName + iardMax * 4)


#define CabiFromPsy(psy)	(*((int *) (PstFromPsy(psy) + 1 + (psy)->stName[0])))
#define IeldiFromPsy(psy)	(*((int *) (PstFromPsy(psy) + 3 + (psy)->stName[0])))

/* Symbol Table */
typedef struct _syt
		{
		unsigned bsy;
		unsigned bsyMax;
		unsigned mpwbsy[wHashMax];
		char grpsy[0];
		} SYT;

#define cbSyt (sizeof (SYT)) /* REMOVE */
#define cbSYT (sizeof (SYT))

typedef SYT HUGE * HPSYT;

#define cbAllocSymbols 512 /* Amount to grow SYT by */


/* MCD Access Macros */

#define PstFromPsy(psy)         (&(psy)->stName[0])

#define AgtOfAgd(agd)           ((agd) & 7)



/* Macro LoCation */

/* This structure is used in symbols for user macros (mctMacro + mctfGlobal
or mctfDotType) to store the location of the text of the macro and the
document type the macro is associated with. */

typedef struct _mlc
		{
		int imcr;
		union {
				PFN pfn;
				int docDot;
				};
		} MLC;

#define cbMLC (sizeof (MLC))

/* Repeat ruler/ribbon formatting. */
#define bcmRRFormat (-4)
/* If bcm is less than this value, it has been translated by
	undo already. */
#define bcmSpecLast  bcmDeleteSel


#ifndef NOEXTERN
extern char vrgchsyFetch [];
#endif

#define vpsyFetch ((SY *) &vrgchsyFetch[0])
