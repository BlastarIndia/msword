/* Definitions for Tables */

#define tblfmtPara      0
#define tblfmtTab       1
#define tblfmtComma     2
#define tblfmtSideBySide        3
#define tblfmtInsert	5


/* Define the different types of command for the Edit Table Dialog */
#define cmdtypeRow              0
#define cmdtypeColumn           1
#define cmdtypeSelection        2

/* Enumeration of tck for return by TckCkTextOkForTable */
#define tckNil				0
#define tckTable			1
#define tckSection		2


#define brckTop         1
#define brckLeft        2
#define brckBottom      4
#define brckRight       8
#define brckAll         (brckTop | brckLeft | brckBottom | brckRight)

#define fselInsertion   0
#define fselSelection   1
#define fselTable       2
#define fselTextToTable 3

#ifdef MAC /* in inter.h for WIN */
#define dxaDefaultGapHalf       (dxaInch * 15 / 200)  /* half of .15" */
#endif
#define dxaMinDefaultColumnWidth        (dxaInch / 4)


struct CORW
	{
	unsigned cColumns, cRows;
	};


#define cKmeTable  2  /* Tab, Shift+Tab */

#define clrNone	0
#define clrStyle	1
#define clrAll		2

/* These are defined as special cases for some of the table
	editing routines */
#define itcNil		-1
#define dtcNil		-1
