/* Dialog Box Id's  - SDM style */
/* WARNING:  Every dialog should have an idd in idd.h  */

/* I M P O R T A N T:

	This file is used in the help file build process.  That process
	must be able to find all lines and only those lines in this file
	which are defining legal IDD's that will be used for help ids.

	These lines will be uniquely identified by the fact that they
	contain the pattern "#define[ \t]+IDD" and also the pattern "helpid".
	Please put the word helpid in the comment of any new IDD you add
	that should be a help topic (that's all of them except debugging
	ones - and if you aren't sure, ask rosiep), and remove the word
	"helpid" if you remove an IDD and leave it as a spare.

*/


#define IDDMin              1      /* no help */


#define IDDUseCOneVersions  1	   /* debugging only, no help */
#define IDDNewDoc           2      /* helpid */
#define IDDOpen             3      /* helpid */
#define IDDSaveAs           4      /* helpid */
#define IDDInsToc           5      /* helpid */
#define IDDCustomize        6      /* helpid */
#define IDDRunMacro         7      /* helpid */
#define IDDRecorder         8      /* helpid */
#define IDDViewPref         9      /* helpid */
#define IDDGlossary         10      /* helpid */
#define IDDInsBookmark      11      /* helpid */
#define IDDGoto             12      /* helpid */
#define IDDDocSummary       13      /* helpid */
#define IDDDocStat          14      /* helpid */
#define IDDDocument         15      /* helpid */
#define IDDCharacter        16      /* helpid */
#define IDDParaLooks        17      /* helpid */
#define IDDSection          18      /* helpid */
#define IDDInsBreak         19      /* helpid */
#define IDDInsFile          20      /* helpid */
#define IDDCmpFile          21      /* helpid */
#define IDDRenumParas       22      /* helpid */
#define IDDApplyStyle       23      /* helpid */
#define IDDDefineStyle      24      /* helpid */
#define IDDMergeStyle       25      /* helpid */
#define IDDRenameStyle      26      /* helpid */
#define IDDTabs             27      /* helpid */
#define IDDFormatPic        28      /* helpid */
#define IDDNewOpen          29      /* helpid */
#define IDDSort             30      /* helpid */
#define IDDSearch           31      /* helpid */
#define IDDReplace          32      /* helpid */
#define IDDNotImplemented   33      /* no help */

	/* debugging dialogs */
	/* some dialogs are used more than once with different contents.
		these will be gone if debug is gone, so we won't go to
		a lot of effort to make this clean */

	/* no help for debug dialogs */             

#define IDDEnableTests      34      /* no help - debug only */
#define IDDTestFunction     35      /* no help - debug only */
#define IDDRareEvents       36      /* no help - debug only */
#define IDDCkDiskFile       37      /* no help - debug only */

#define IDDHyphenate        38      /* helpid */
#define IDDChangeKeys       39      /* helpid */
#define IDDAssignToMenu     40      /* helpid */
#define IDDHeader           41      /* helpid */
#define IDDPrint            42      /* helpid */
#define IDDPrintMerge       43      /* helpid */
#define IDDAbout            44      /* helpid */
#define IDDEdMacro          45      /* helpid */
#define IDDRenMacro         46      /* helpid */
#define IDDShowVars         47      /* helpid */
#define IDDScribble         48      /* debug only - no help */
#define IDDInsertFtn        49      /* helpid */
#define IDDPrompt           50      /* no help - handled elsehwere */
#define IDDChgPr            51      /* helpid */
#define IDDRevMarking       52      /* helpid */
#define IDDInsPgNum         53      /* helpid */
#define IDDAppRun           54      /* helpid */
#define IDDPasteLink        55      /* helpid */
#define IDDUserName         56      /* helpid */
#define IDDAutosave         57      /* helpid */
#define IDDSpeller          58      /* helpid */
#define IDDSpellerMM        59      /* helpid */

#define IDDIndex            60      /* helpid */
#define IDDIndexEntry       61      /* helpid */
#define IDDThesaurus        62      /* helpid */
#define IDDCatalog          63      /* helpid */
#define IDDCatSearch        64      /* helpid */
#define IDDCatPrint         65      /* helpid */
#define IDDCatSrhProgress   66      /* helpid */
#define IDDInsField         67      /* helpid */
#define IDDFilePswd         68      /* helpid */
#define IDDConfirmRepl      69      /* helpid */

#define IDDDbgMemory        70      /* debug only - no help */
#define IDDDbgMemoryCmd     71      /* debug only - no help */
#define IDDVerifyConvtr     72      /* helpid */
#define IDDDebugPrefs	    73      /* debug only - no help */
#define IDDDbgFail  	    74      /* debug only - no help */

#define IDDInsPic           75      /* helpid */
#define IDDInsertTable      76      /* helpid */
#define IDDEditTable        77      /* helpid */
#define IDDFormatTable      78      /* helpid */
#define IDDTableToText      79      /* helpid */
#define IDDAbsPos           80      /* helpid */

/* iconbar dialogs */
#define IDDIbRibbon         81      /* no help - handled elsewhere */
#define IDDIbRuler          82      /* no help - handled elsewhere */

#define IDDColor	    83
#define IDDUsrDlg	    84

/* IDD's for non-dialogs associated with Search and Replace */
#define IDDSearchChar       85
#define IDDSearchPara       86
#define IDDReplaceChar      87
#define IDDReplacePara      88

#define IDDStyleChar        89
#define IDDStylePara        90
#define IDDStyleAbsPos      91
#define IDDStyleTabs        92

#define IDDUseCTwoVersions  93	   /* debugging only, no help */
#define IDDUseCThreeVersions  94     /* debugging only, no help */
#define IDDUseCFourVersions  95     /* debugging only, no help */

/* WHEN YOU ADD IDDs, ADD ENTRIES TO rgCmdDlg in DEBUGDLG.C */
/* Change this to the last IDD defined (Only used in DEBUGDLG.C) */

#define IDDLast 	    95
