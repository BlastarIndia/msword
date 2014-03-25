typedef int F;
#define fFalse 		0
#define fTrue 		1

typedef int CM;
#define cmPaste		110
#define cmCopy		111
#define cmClear		112
#define cmFileExit      113
#define cmIcon		114
#define cmCursor	115
#define cmBitmap	116
#define cmPrSetup	117
#define cmEnum		118
#define cmEscape	119
#define cmOptions	120
#define cmFont		121
#define cmTestPr	122
#define cmSwMsgMode     123
#define cmUserPr	124

typedef int CMD;
#define cmdOK		0
#define cmdCancelled	1
#define cmdError	2

/* used by MyMessage to determine requested action */
typedef int MM;
#define MM_CREATE	1
#define MM_CHANGEBOTH	2
#define MM_CHANGE1	3
#define MM_CHANGE2	4
#define MM_DESTROY	0


#define	ichMaxSz	256			

/* number of points/inch for points and half points */
#define POINTS_INCH		72
#define HALFPOINTS_INCH		144


/*  P R I  */
/*  Printer structure */
typedef struct {
	HDC	hdc;
	char	szPrinter[ichMaxSz], 		
		szPrDriver[ichMaxSz], 		
		szPrPort[ichMaxSz];		
	}  PRI;


/*  D U D  */
/*  Device Unit Description */
/*    describes the current device page in device units.  */ 
typedef struct {
	short	yLoc,		/* Y Location on page 		*/
		yLin,		/*   Height of one line  	*/
		yOff,		/*   Beginning offset each page */
		yMaxPage,	/*   Height of entire page 	*/
		yInch,		/* Y device units per inch 	*/
		xInch,		/* X device units per inch 	*/
		PtsInch;	/* X,Y Points per inch 		*/
	}  DUD;


/*  F F L  */
/*  Font Facename List  */
typedef struct {
	GLOBALHANDLE	hGMem;
	short		cFace;
	}  FFL;


/* E S C */
/* escape structure */
typedef struct {
	short		esc;
	int		iSt;
	}  ESC;


/* U F D	*/
/* User Font Description */
typedef struct {
	BOOL		fHaveFont;
	LOGFONT 	lf;
	}  UFD;



#define Assert(f)

#ifndef NDEBUG

#define WAssert(exp,lpsz)						\
	{									\
	if (!(exp))								\
	{			     					\
	char szBuffer[ichMaxSz];     					\
										\
	sprintf(szBuffer, "File: %s, Line %d.  ", __FILE__, __LINE__);	\
	SzLpszAppend(szBuffer, lpsz); 					\
	MessageBox (vhwnd, 						\
		(LPSTR) szBuffer, 					\
		(LPSTR) "ASSERTION FAILURE!",				\
		MB_OK | MB_ICONHAND);					\
	}								\
	}
																		
#else

#define WAssert(exp,sz)

#endif

