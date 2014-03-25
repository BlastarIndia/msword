/* Back Space return Values */
typedef int BSV;
#define bsvNormal	0
#define bsvError	1
#define bsvRevMark	2

/* Typing Loop Variables */
typedef struct _tlv
	{
	CP cpStart; /* selCur.cpFirst at start of insert mode */
	CP cpInsertLow;
	CP cpLow;
	CP cpInsertLine;
	BOOL fInvalPgdOnInsert;
	BOOL fFtnDel; /* footnote or annotation was deleted */
	BOOL fNoIdle;
	BOOL fNextEop; /* set if style next is applied */
	BOOL fInsertLoop; /* set if we're in the insert loop (not in macro) */
	BOOL fReturn; /* used by FNextMsgForInsert() */
	int ccpEopOpen; /* counts Eop's inserted by Open Space */
	int ichFill; /* set by backspace to number of chs put in rgchInsert */
	int ch; /* last character typed */
	int ucm; /* used to distinguish between different EOP's */
	BOOL fInsEnd; /* set iff inserting in front of page or sect */
	BOOL fAddRun; /* if run needs to be added for new paragraph */
	char *ppapx; /* when needed for add run */
	int cchPapx; /* when needed for add run */
	BOOL fRecordBksp; /* set if backspaces potentially need to be recorded */
	BOOL fInvalAgain; /* set if Again should not be set to bcmtyping */
#ifdef MAC
	int	chm;  /* key typed including modifiers */
#endif
	} TLV;
	
#define cwTLV CwFromCch(sizeof (TLV))
