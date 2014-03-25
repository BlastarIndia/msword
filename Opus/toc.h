/* toc.h - definitions for Table of Contents routines */

#define pgnNil          -1


#define chIndexKeyInit  'C'             /*  Initial table key letter  */

#define cchMaxEntry     255             /*  max length of toc entry string */

#define chSubLevel      chColon         /*  character signifying sublevels  */

#define chQuote         '\''

#define ichRedoFetch     50             /*  same name used in INDEX.H for  */
					/*  same thing  */

#define iTocOutline 0
#define iTocFields  1

#define iLevAll     0
#define iLevFromTo  1

#define iLevFirstDef  1
#define iLevLastDef   9

#define chTblDef  'C'

#define cchMaxLevRange   6   /* includes the '\0' in sz */
#define cchMaxTable      2   /* includes '\0' */
#define cchMaxLevel      3   /* includes '\0' */
#define cchMaxSeparator  4   /* includes the cch in st */


struct TIB    /* Table of contents Info Block */
	{
	int doc;                /* temp doc to build up toc */
	CP cpMac;               /* cpMac of docToc */
	int iLevMin;            /* min level to show */
	int iLevLast;           /* last ... */
	int docMain;            /* doc to insert toc in */
	int fTocFields : 1;     /* collect TC fields (not outline) */
	int fSequence  : 1;     /* show sequence numbers */
	int fTable     : 1;
	int : 5;
	CHAR chTbl;             /* global table character */
	CHAR szSequence[cchSequenceIdMax];    /* sequence to show */
	CHAR stSeqSep[cchMaxSeparator+1];     /* sequence separator */
	};

#																				ifdef EXTMATH
#																				include "mathapi.c"
#																				endif

/* Also in index.h - must match */
#define sdeIndex   0
#define sdeToc     1
#define sdeOutline 2

