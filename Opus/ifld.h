/* ****************************************************************************
**
**      COPYRIGHT (C) 1987 MICROSOFT
**
** ****************************************************************************
*
*  Module: header file for Insert Field
*
**
** REVISIONS
**
** Date         Who Rel Ver     Remarks
**
** 10/15/87     yxy		new file
**
** ************************************************************************* */
typedef struct _fd
	{
	int	(*pfnFAbstEnum)();
	CHAR	stzName[];
	CHAR	stzSyntax[];
	CHAR	stzHelp[];
	}	FD;

typedef struct _aelp
	{
	CHAR	stzInstName[];
	CHAR	stzInstActual[];
	}	AELP;

typedef struct _aepic
	{
	CHAR	stzPic[];
	}	AEPIC;



#define stzApFldSwFormatDef	{'\004', chFieldEscape, chFldSwSysFormat, \
				(CHAR) (chSpace), '\0'}
#define cchApFldSwFormat	4


#define stzEqCmdArrayDef	{'\005', chFieldEscape, 'A',      \
				'(', ')', '\0'}
#define stzEqCmdBoxDef		{'\005', chFieldEscape, 'X',      \
				'(', ')', '\0'}
#define stzEqCmdBracketDef	{'\005', chFieldEscape, 'B',   	  \
				'(', ')', '\0'}
#define stzEqCmdDisplaceDef	{'\005', chFieldEscape, 'D',  	  \
				'(', ')', '\0'}
#define stzEqCmdFractionDef	{'\006', chFieldEscape, 'F',      \
				'(', ',', ')', '\0'}
#define stzEqCmdIntegralDef	{'\007', chFieldEscape, 'I',	  \
				'(', ',', ',', ')', '\0'}
#define stzEqCmdListDef		{'\005', chFieldEscape, 'L',      \
				'(', ')', '\0'}
#define stzEqCmdOverDef		{'\005', chFieldEscape, 'O',      \
				'(', ')', '\0'}
#define stzEqCmdRadicalDef	{'\006', chFieldEscape, 'R',      \
				'(', ',', ')', '\0'}
#define stzEqCmdScriptDef	{'\005', chFieldEscape, 'S',	  \
				'(', ')', '\0'}
#define cchEqCmdMax		7

#define stzTOCSwOutline		{'\003', chFieldEscape, chFldTocOutline, '\0'}
#define stzTOCSwField		{'\003', chFieldEscape, chFldTocFields, '\0'}
#define cchMaxTOCActual 	3 /* came from \003 */

#define stzIndexSwRunin         {'\003', chFieldEscape, chFldIndexRunin, '\0'}
#define stzIndexSwHeading       {'\007', chFieldEscape, chFldIndexHeading, \
								(CHAR) chSpace, chGroupExternal, 'A', \
								chGroupExternal, '\0'}
#define stzIndexSwBlank         {'\007', chFieldEscape, chFldIndexHeading, \
								(CHAR) chSpace, chGroupExternal, \
								(CHAR) chSpace, chGroupExternal, '\0'}
#define cchMaxIndexActual       7 /* came from \007 */

#define stzPrintSwPage		{'\010', chFieldEscape, chFldPrintPs,	\
				(CHAR) chSpace, 'p', 'a', 'g', 'e', '\0'}
#define stzPrintSwPara		{'\010', chFieldEscape, chFldPrintPs,	\
				(CHAR) chSpace, 'p', 'a', 'r', 'a', '\0'}
#define stzPrintSwPict		{'\007', chFieldEscape, chFldPrintPs,	\
				(CHAR) chSpace, 'p', 'i', 'c', '\0'}
#define cchMaxPrintActual	8 /* came from \010 */

#define stzDTPicSwDef		{'\004', chFieldEscape, chFldSwSysDateTime, \
				(CHAR) chSpace, '\0'}
#define cchDTPicSwMax		4
#define stzNumPicSwDef		{'\004', chFieldEscape, chFldSwSysNumeric, \
				(CHAR) chSpace, '\0'}
#define cchNumPicSwMax		4
				

#define abmEnumInit		0
#define abmEnum			1
#define abmAppend		2

#define abmMin			0
#define abmMax			(abmAppend + 1)

#define hwndInstLB		(GetDlgItem(vhDlg, tmcIFldInst))

