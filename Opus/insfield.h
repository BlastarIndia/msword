/* ***************************************************************************
**
**      COPYRIGHT (C) 1987 MICROSOFT
**
** ***************************************************************************
*
*  Module: insfield.h contains strings used in the Insert Field dialog box.
*
*  Functions included: None
*
**
** REVISIONS
**
** Date         Who Rel Ver     Remarks
**
** 10/21/87     russb/yxy	More fields
**
** 10/17/87     yxy		Refitted for C macros
**
** 10/7/87      russb           Added Glossary field
**
** 8/26/87      russb
**
** **************************************************************************/

#ifdef DESCRIPTION
Structure:  Field Descriptor consists of following four entries:

	pfnFAbstEnum:	This is the first entry.  If it is NULL,
			it indicates that the instruction list box
			and its associated static text is grayed and
			there is no entry listed in the list box.
			Otherwise, a function pointed by pfnFAbstEnum
			will enumerate a string for the list box on
			abEnum message.  It, also, takes care of
			appending an appropriate field format string
			upon abAppend message.  Currently available
			functions for this purpose is listed below
			along with the strings enumerated by them.

	stzName:	This is an stz string containing a name of
			the field.  For a description of an stz string
			see below.

	stzSyntax:	This is an stz string describing syntax of
			an individual field.  The first word in this
			string separated by a blank (and excluding '['
			and ']') must be a valid field keyword.

	stzHelp:	This is an stz string containing a help text.

	-- An stz string is denoted by a regular C string notation, in
		which the first byte denotes the number of characters 
		including the null terminator in the "meaningful" part of
		the string.  This number is denoted by '\' followed by three
		octal digits.
			(NOTE: The stz's used here are nonstandard, in that the length
					includes the null terminator.  Also, the method of
					definition has been changed from the above; we now
					declare these strings as StShared strings and place
					the \000 terminator on the end.  This achieves the same
					effect, but makes counting the characters unnecessary.

	-- For stzSyntax and stzHelp, you may not use '&' in any part
		of a string definition. (Windows restriction)

The order in which field names are listed in the "Field" dialog is 
determined by the definition of rgfdDef below.

List of functions for pfnFAbstEnum:

	Name			Lists		Comments
	----			-----		--------
	FAbstEnumDateInst			date instruction
				M/D/Y
				D-MMM-YY
				MMM-YY
				D MMMM, Y
				MMMM D, YYYY

	FAbstEnumTimeInst			time instruction
				h:mm AM/PM
				h:mm

	FAbstEnumGlossInst			glossary instruction
				Names of
				defined glossaries

	FAbstEnumInfoInst			summary info instruction
				Author			author
				Comments		comments
				Create date		createdate
				Edit time		edittime
				File name		filename
				Keywords		keywords
				Last saved by		lastsavedby
				# of chars		numchars
				# of pages		numpages
				# of words		numwords
				Print date		printdate
				Rev. number		revnum
				Save date		savedate
				Subject			subject
				Template		template
				Title			title

	FAbstEnumNumInst			numerical instruction
				0
				0.00
				#,##0
				#,##0.00
				$#,##0.00;($#,##0.00)
				0%
				0.00%

	FAbstEnumPNumInst			page number instruction
				1 2 3...	\* arabic
				a b c...	\* alphabetic
				A B C...	\* ALPHABETIC
				i ii iii...	\* roman
				I II III...	\* ROMAN

	FAbstEnumBkmkInst			bookmark instruction
				Fill with the
				defined bookmark
				names.

	FAbstEnumStyleInst			style instruction
				Fill with
				currently defined
				style names.
	FAbstEnumTOCInst			toc instruction
				Use Outline		\o
				Use Field Entries	\f

	FAbstEnumEqInst				formula instructions.
				Array			\A()
				Box			\X()
				Bracket			\B()
				Displace		\D()
				Fraction		\F(,)
				Integral		\I(,,)
				List			\L()
				Overstrike		\O()
				Radical			\R(,)
				Superscript		\S()
	FAbstEnumMacroInst			for macrotext
				Currently
				defined macros
	FAbstEnumPrintInst      Paragraph		\p para
				Picture			\p pic
				Page			\p page


The definitions strings used by these FAbstEnumX functions can be found
in the second section of this file.
#endif

/* Maximum number of characters (including the null terminator) currently
	allowed in field descriptor strings. */
#define cchSzFdMax	55

/* The number of fields handled by the insert field dialog box. */
#define ifdMax		52


/* characters excluded from the field keyword derivation from the syntax
	string. */
#define chOptBeg	'['
#define chOptEnd	']'



/* - - - - Definition of entries used by various FAbstEnumX's. - - - - */

/* for FAbstEnumPNumInst */
/* This must be listed in sorted order.  Also, the ordering should
	be the same to the order page number format is listed in header/footer
	dialog.  (Refer to dlbenum.c) */
#define rgaelpPNumDef {					\
	{StShared("1 2 3...\000"),	StShared("arabic\000")},		\
	{StShared("A B C...\000"),	StShared("ALPHABETIC\000")},	\
	{StShared("a b c...\000"),	StShared("alphabetic\000")},	\
	{StShared("I II III...\000"),	StShared("ROMAN\000")},		\
	{StShared("i ii iii...\000"),	StShared("roman\000")}}
#define iaelpPNumMax	  5
#define cchMaxPNumActual 12 /* came from \014. */

/* for FAbstEnumInfoInst */
/* for this one we take an advantage of the fact that an fd exists
	corresponding to each instructions to be listed. */
#define rgifdInfoDef	{					\
				2,	/* Author */		\
				6,	/* Comments */		\
				7,	/* Create date */ 	\
				12,	/* Edit time */		\
				13,	/* File name */		\
				24,	/* Keywords */		\
				25,	/* Last saved by */	\
				30,	/* # of chars */	\
				31,	/* # of pages */	\
				32,	/* # of words */	\
				36,	/* Print date */	\
				40,	/* Rev. number */	\
				41,	/* Save date */		\
				46,	/* Subject */		\
				48,	/* Template */		\
				50,	/* Title */}
#define iifdInfoMax	(sizeof(rgifdInfo) / sizeof(CHAR))

/* for FAbstEnumTOCInst */
/* for this one, we must define full AE literal pair. */
/* this list must be in sorted order */
#define rgaelpTOCDef {						\
	{StShared("Use field entries\000"),	stzTOCSwField},		\
	{StShared("Use outline\000"),		stzTOCSwOutline}}

#define iaelpTOCMax		2


/* for FAbstEnumIndexInst */
/* for this one, we must define full AE literal pair. */
/* this list must be in sorted order */
#define rgaelpIndexDef {                                        \
		{StShared("Blank line\000"),              stzIndexSwBlank},       \
		{StShared("Heading letters\000"),         stzIndexSwHeading},     \
		{StShared("Runin index\000"),             stzIndexSwRunin}}

#define iaelpIndexMax           3

/* for FAbstEnumPrintInst */
/* for this one, we must define full AE literal pair. */
/* this list must be in sorted order */
#define rgaelpPrintDef {					\
	{StShared("Page\000"),			stzPrintSwPage},	\
	{StShared("Paragraph\000"),		stzPrintSwPara},	\
	{StShared("Picture\000"),		stzPrintSwPict}}

#define iaelpPrintMax		3

/* for FAbstEnumEqInst */
#define rgaelpEqDef {							\
	{StShared("Array\000"),		    stzEqCmdArrayDef},			\
	{StShared("Box\000"),	    	stzEqCmdBoxDef},			\
	{StShared("Bracket\000"),		stzEqCmdBracketDef},		\
	{StShared("Displace\000"),	    stzEqCmdDisplaceDef},		\
	{StShared("Fraction\000"),	    stzEqCmdFractionDef},		\
	{StShared("Integral\000"),	    stzEqCmdIntegralDef},		\
	{StShared("List\000"),		    stzEqCmdListDef},			\
	{StShared("Overstrike\000"),	    stzEqCmdOverDef},			\
	{StShared("Radical\000"),		stzEqCmdRadicalDef},		\
	{StShared("Superscript\000"),	stzEqCmdScriptDef}}
#define iaelpEqMax		10
#define cchMaxEqActual		cchEqCmdMax

/* for FAbstEnumDateInst */
#define rgaepicDateDef {						\
	{StShared("d MMMM, yyyy\000")},						\
	{StShared("MMMM d, yyyy\000")},						\
	{StShared("d-MMM-yy\000")},						\
	{StShared("MMMM, yy\000")},						\
	{StShared("MMM-yy\000")}}
#define iaepicDateMax		6

/* for FAbstEnumTimeInst */
#define rgaepicTimeDef {						\
	{'\006', chPicHourHldr, chTimeSepHldr, chPicMinHldr, ' ',	\
		chPicAPDUCHldr, '\0'}						\
	{'\004', chPic24HourHldr, chTimeSepHldr, chPicMinHldr, '\0'}}
#define iaepicTimeMax		2

/* for FAbstEnumNumInst */
#define rgaepicNumDef {							\
	{'\002', chPicReqDigHldr, '\0'},				\
	{'\004', chPicReqDigHldr, chPicDecimalHldr,			\
		chPicFractDigHldr, '\0'},				\
	{'\006', chPicOptDigHldr, chPicThousandHldr,			\
		chPicOptDigHldr, chPicOptDigHldr,			\
		chPicReqDigHldr, '\0'},					\
	{'\010', chPicOptDigHldr, chPicThousandHldr,			\
		chPicOptDigHldr, chPicOptDigHldr,			\
		chPicReqDigHldr, chPicDecimalHldr,			\
		chPicFractDigHldr, '\0'},				\
	{'\026', chPicPreCurHldr, chPicOptDigHldr,			\
		chPicThousandHldr, chPicOptDigHldr,			\
		chPicOptDigHldr, chPicReqDigHldr, chPicDecimalHldr,	\
		chPicFractDigHldr, chPicPostCurHldr,			\
		chPicExpSepHldr, '(',					\
		chPicPreCurHldr, chPicOptDigHldr,			\
		chPicThousandHldr, chPicOptDigHldr,			\
		chPicOptDigHldr, chPicReqDigHldr, chPicDecimalHldr,	\
		chPicFractDigHldr, chPicPostCurHldr,			\
		')', '\0'},						\
	{'\003', chPicReqDigHldr, '%', '\0'},				\
	{'\005', chPicReqDigHldr, chPicDecimalHldr,			\
		chPicFractDigHldr, '%', '\0'}}
#define iaepicNumMax		7

/* Field Descriptors defined separately for easier sorting in rgfdDef */

#define fdExpDef { \
	&FAbstEnumNumInst, \
	StShared("= expression\000"), \
	StShared("= expression\000"), \
	StShared("Select format; then type numeric expression\000")}
#define fdAskDef { \
	NULL, \
	StShared("Ask\000"), \
	StShared("ASK bookmark prompt\000"), \
	StShared("Type bookmark name and text for prompt\000")}
#define fdAuthorDef { \
	NULL, \
	StShared("Author\000"), \
	StShared("AUTHOR [new-name]\000"), \
	StShared("Choose OK to insert; type new name to change\000")}
#define fdAutoNumDef { \
	NULL, \
	StShared("Auto No.\000"), \
	StShared("AUTONUM\000"), \
	StShared("Choose OK to insert an automatic arabic number\000")}
#define fdAutoNumLglDef { \
	NULL, \
	StShared("Auto No. legal\000"), \
	StShared("AUTONUMLGL\000"), \
	StShared("Choose OK to insert an automatic legal number\000")}
#define fdAutoNumOutDef { \
	NULL, \
	StShared("Auto No. outline\000"), \
	StShared("AUTONUMOUT\000"), \
	StShared("Choose OK to insert an automatic outline number\000")}
#define fdCommentsDef { \
	NULL, \
	StShared("Comments\000"), \
	StShared("COMMENTS [new-comments]\000"), \
	StShared("Choose OK to insert; type new text to change\000")}
#define fdCreatedateDef { \
	&FAbstEnumDateInst, \
	StShared("Create date\000"), \
	StShared("CREATEDATE\000"), \
	StShared("Choose OK to insert date document was created\000")}
#define fdDataDef { \
	NULL, \
	StShared("Data\000"), \
	StShared("DATA data-file [header-file]\000"), \
	StShared("Type data file name [header-file] for merging\000")}
#define fdDateDef { \
	&FAbstEnumDateInst, \
	StShared("Date\000"), \
	StShared("DATE [date-format-picture]\000"), \
	StShared("Type or select desired date format\000")}
#define fdDDEDef { \
	NULL, \
	StShared("DDE\000"), \
	StShared("DDE app-name file-name [place-reference]\000"), \
	StShared("Type program name, file name; optional place\000")}
#define fdDDEAutoDef { \
	NULL, \
	StShared("DDE Auto\000"), \
	StShared("DDEAUTO app-name file-name [place-reference]\000"), \
	StShared("Type program name, file name; optional place\000")}
#define fdEditTimeDef { \
	NULL, \
	StShared("Edit time\000"), \
	StShared("EDITTIME\000"), \
	StShared("Choose OK to insert the minutes of editing time\000")}
#define fdFilenameDef { \
	NULL, \
	StShared("File name\000"), \
	StShared("FILENAME\000"), \
	StShared("Choose OK to insert file name of document\000")}
#define fdFillinDef { \
	NULL, \
	StShared("Fill-in\000"), \
	StShared("FILLIN [prompt]\000"), \
	StShared("Type text for prompt\000")}
#define fdEQDef { \
	&FAbstEnumEqInst, \
	StShared("Formulas\000"), \
	StShared("EQ instructions\000"), \
	StShared("Select or type a command; type options and elements\000")}
#define fdGlossaryDef { \
	&FAbstEnumGlossInst, \
	StShared("Glossary\000"), \
	StShared("GLOSSARY glossary-name\000"), \
	StShared("Type or select a glossary name\000")}
#define fdGotoButtonDef { \
	NULL, \
	StShared("Goto Button\000"), \
	StShared("GOTOBUTTON go-to-instruction display-text\000"), \
	StShared("Type Go To instruction and text for field's result\000")}
#define fdIfDef { \
	NULL, \
	StShared("If\000"), \
	StShared("IF exp op exp if-true-text if-false-text\000"), \
	StShared("Type condition, if-true text, and if-false text\000")}
#define fdImportDef { \
	NULL, \
	StShared("Import\000"), \
	StShared("IMPORT file-name\000"), \
	StShared("Type file name\000")}
#define fdIncludeDef { \
	NULL, \
	StShared("Include\000"), \
	StShared("INCLUDE file-name [place-reference]\000"), \
	StShared("Type file name; optional place reference\000")}
#define fdIndexDef { \
	&FAbstEnumIndexInst, \
	StShared("Index\000"), \
	StShared("INDEX [switches]\000"), \
	StShared("Type or select index formatting switches\000")}
#define fdXEDef { \
	NULL, \
	StShared("Index entry\000"), \
	StShared("XE text [switches]\000"), \
	StShared("Type text of index entry and optional switches\000")}
#define fdInfoDef { \
	&FAbstEnumInfoInst, \
	StShared("Info\000"), \
	StShared("[INFO] info-type [new-value]\000"), \
	StShared("Type or select type of Summary Info\000")}
#define fdKeyWordsDef { \
	NULL, \
	StShared("Keywords\000"), \
	StShared("KEYWORDS [new-key-words]\000"), \
	StShared("Choose OK to insert; type new text to change\000")}
#define fdLastSavedByDef { \
	NULL, \
	StShared("Last saved by\000"), \
	StShared("LASTSAVEDBY\000"), \
	StShared("Choose OK to insert name who last saved file\000")}
#define fdMacroButtonDef { \
	&FAbstEnumMacroInst, \
	StShared("Macro Button\000"), \
	StShared("MACROBUTTON macro-name display-text\000"), \
	StShared("Select or type macro name and text for field's result\000")}
#define fdMergeRecDef { \
	NULL, \
	StShared("Merge rec.\000"), \
	StShared("MERGEREC\000"), \
	StShared("Choose OK to insert merge record number\000")}
#define fdNextDef { \
	NULL, \
	StShared("Next\000"), \
	StShared("NEXT\000"), \
	StShared("Choose OK to insert a NEXT print merge field\000")}
#define fdNextIfDef { \
	NULL, \
	StShared("Next if\000"), \
	StShared("NEXTIF exp op exp\000"), \
	StShared("Type condition for when to merge next record\000")}
#define fdNumCharsDef { \
	NULL, \
	StShared("No. of chars\000"), \
	StShared("NUMCHARS\000"), \
	StShared("Choose OK to insert number of characters\000")}
#define fdNumPagesDef { \
	NULL, \
	StShared("No. of pages\000"), \
	StShared("NUMPAGES\000"), \
	StShared("Choose OK to insert number of pages\000")}
#define fdNumWordsDef { \
	NULL, \
	StShared("No. of words\000"), \
	StShared("NUMWORDS\000"), \
	StShared("Choose OK to insert number of words\000")}
#define fdPageDef { \
	&FAbstEnumPNumInst, \
	StShared("Page\000"), \
	StShared("PAGE [page-format-picture]\000"), \
	StShared("Type or select desired page number format\000")}
#define fdPageRefDef { \
	&FAbstEnumBkmkInst, \
	StShared("Page ref.\000"), \
	StShared("PAGEREF bookmark\000"), \
	StShared("Type or select bookmark name for reference\000")}
#define fdPrintDef { \
	FAbstEnumPrintInst, \
	StShared("Print\000"), \
	StShared("PRINT \"printer-instructions\"\000"), \
	StShared("Type printer codes or PostScript code\000")}
#define fdPrintDateDef { \
	&FAbstEnumDateInst, \
	StShared("Print date\000"), \
	StShared("PRINTDATE\000"), \
	StShared("Select date format and choose OK\000")}
#define fdQuoteDef { \
	NULL, \
	StShared("Quote\000"), \
	StShared("QUOTE \"literal-text\"\000"), \
	StShared("Type text you want as field's result\000")}
#define fdRefDef { \
	&FAbstEnumBkmkInst, \
	StShared("Reference\000"), \
	StShared("REF bookmark\000"), \
	StShared("Type or select bookmark name for reference\000")}
#define fdRDDef { \
	NULL, \
	StShared("Referenced doc.\000"), \
	StShared("RD file-name\000"), \
	StShared("Type file name for table of contents or index\000")}
#define fdRevNumDef { \
	NULL, \
	StShared("Rev. number\000"), \
	StShared("REVNUM\000"), \
	StShared("Choose OK to insert revision number\000")}
#define fdSaveDateDef { \
	&FAbstEnumDateInst, \
	StShared("Save date\000"), \
	StShared("SAVEDATE\000"), \
	StShared("Select date format and choose OK\000")}
#define fdSeqDef { \
	&FAbstEnumBkmkInst, \
	StShared("Sequence\000"), \
	StShared("SEQ [bookmark]\000"), \
	StShared("Type switch or bookmark name\000")}
#define fdSetDef { \
	NULL, \
	StShared("Set\000"), \
	StShared("SET bookmark data\000"), \
	StShared("Type bookmark name and associated data\000")}
#define fdSkipIfDef { \
	NULL, \
	StShared("Skip if\000"), \
	StShared("SKIPIF exp op exp\000"), \
	StShared("Type condition for when to skip a record\000")}
#define fdStyleRefDef { \
	&FAbstEnumStyleInst, \
	StShared("Style ref.\000"), \
	StShared("STYLEREF style-identifier [switch]\000"), \
	StShared("Type or select style name or a number 1 to 9\000")}
#define fdSubjectDef { \
	NULL, \
	StShared("Subject\000"), \
	StShared("SUBJECT [new-subject]\000"), \
	StShared("Choose OK to insert; type new text to change\000")}
#define fdTCDef { \
	NULL, \
	StShared("TC\000"), \
	StShared("TC text [switches]\000"), \
	StShared("Type text; optional switches for table id, TOC level\000")}
#define fdTemplateDef { \
	NULL, \
	StShared("Template\000"), \
	StShared("TEMPLATE\000"), \
	StShared("Choose OK to insert name of document template\000")}
#define fdTimeDef { \
	&FAbstEnumTimeInst, \
	StShared("Time\000"), \
	StShared("TIME [time-format-picture]\000"), \
	StShared("Type or select desired time format\000")}
#define fdTitleDef { \
	NULL, \
	StShared("Title\000"), \
	StShared("TITLE [new-title]\000"), \
	StShared("Choose OK to insert; type new title to change\000")}
#define fdTOCDef { \
	&FAbstEnumTOCInst, \
	StShared("TOC\000"), \
	StShared("TOC [switches]\000"), \
	StShared("Type or select optional switches\000")}

#define rgfdDef {\
		fdExpDef, \
		fdAskDef, \
		fdAuthorDef, \
		fdAutoNumDef, \
		fdAutoNumLglDef, \
		fdAutoNumOutDef, \
		fdCommentsDef, \
		fdCreatedateDef, \
		fdDataDef, \
		fdDateDef, \
		fdDDEDef, \
		fdDDEAutoDef, \
		fdEditTimeDef, \
		fdFilenameDef, \
		fdFillinDef, \
		fdEQDef, \
		fdGlossaryDef, \
		fdGotoButtonDef, \
		fdIfDef, \
		fdImportDef, \
		fdIncludeDef, \
		fdIndexDef, \
		fdXEDef, \
		fdInfoDef, \
		fdKeyWordsDef, \
		fdLastSavedByDef, \
		fdMacroButtonDef, \
		fdMergeRecDef, \
		fdNextDef, \
		fdNextIfDef, \
		fdNumCharsDef, \
		fdNumPagesDef, \
		fdNumWordsDef, \
		fdPageDef, \
		fdPageRefDef, \
		fdPrintDef, \
		fdPrintDateDef, \
		fdQuoteDef, \
		fdRefDef, \
		fdRDDef, \
		fdRevNumDef, \
		fdSaveDateDef, \
		fdSeqDef, \
		fdSetDef, \
		fdSkipIfDef, \
		fdStyleRefDef, \
		fdSubjectDef, \
		fdTCDef, \
		fdTemplateDef, \
		fdTimeDef, \
		fdTitleDef, \
		fdTOCDef \
		}
