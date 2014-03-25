/* M E S S A G E . H */

/* Opus messages which require user input or have more than one possible
	reply.  All error messages (those which have only one possible reply)
	are defined in wordtech\error.c and wordtech\error.h.
*/

/* Built mst's: first char: 
		\001=> sz
		\002=> int
		\003=> long
		\004=> stFile (leaf name in result)
		\005=> za (allow room in width for units)
		\006=> name of doc (short form)
		\007=> st
		\010=> optional int (omit if 0)

	2nd char is width (right just fixed field for ints, max field for szs) 

	Special char: \037 is just a "marker" so that a location in the result
	can be found (via pppr passed to BuildStMstRgw).  Width character is
	cch changed.
*/

typedef struct
		{
		int     cxt;
		char    *st;
		} MST;

#define Mst(cxt,st)     ((MST)MakeLong((cxt),(st)))

/* I M P O R T A N T:

The hex numbers in the listings below are cxt's (context id's) used as
hid's (help id's) for giving the user context-sensitive help.  These
numbers are FIXED, hard coded into the help application, and MAY NOT
BE CHANGED!

The numbers from 7000 to 7FFF have been reserved for cxt's associated
with mst strings.  

If you add a new mst, put it at the bottom of this list and give it 
the next number in sequence.  Be certain that you do not use a number
already assigned to another mst.

Do not use numbers missing from the sequence below, they were defined
at some point and may not be reused.
*/

#define mstNil                Mst(-1    , StSharedKey ("",EmptyString))
#define mstGoto               Mst(0x7000, StSharedKey ("Go to: ",Goto))
#define mstWhichStyle         Mst(0x7001, StSharedKey ("Which style? ",WhichStyle))
#define mstFontName           Mst(0x7002, StSharedKey ("Which font name? ",FontName))
#define mstFontSize           Mst(0x7003, StSharedKey ("Which font size? ",FontSize))
#define mstInsBookmark        Mst(0x7004, StSharedKey ("Insert bookmark: ",InsBookmark))
#define mstMoveTo             Mst(0x7005, StSharedKey ("Move to where?",MoveToWhere))
#define cxtMoveTo           0x7005
#define mstMoveFrom           Mst(0x7006, StSharedKey ("Move from where?",MoveFrom))
#define mstHelpMode           Mst(0x7007, StSharedKey ("Select command to get help on.",HelpMode))
#define mstSearching          Mst(0x7008, StSharedKey ("Searching...  Press Esc to cancel.",Searching))
#define mstCatCannotRead      Mst(0x7009, StSharedKey ("Cannot read \001\000",CatCannotRead))
#define mstSpellCheck         Mst(0x700A, StSharedKey ("Checking...  Press Esc to cancel.",SpellCheck))
#define mstSpellDone          Mst(0x700B, StSharedKey ("Spell check completed.",SpellDone))
#define mstDMSort             Mst(0x700C, StSharedKey ("Sorting...",DMSort))
#define mstBuildingIndex      Mst(0x700D, StSharedKey ("Building index...  Press Esc to cancel.",BuildingIndex))
#define mstRefreshIndex       Mst(0x700E, StSharedKey ("Updating Index,  \037\003  0% complete.  Press Esc to cancel.",RefreshIndex))
#define mstCalcFields         Mst(0x700F, StSharedKey ("Updating Fields, \037\003  0% complete.  Press Esc to cancel.",CalcFields))
#define mstMultiChanges       Mst(0x7010, StSharedKey ("\002\000 changes.",MultiChanges))
#define mstReplacing          Mst(0x7011, StSharedKey ("Replacing, \037\003  0% complete.  Press Esc to cancel.",Replacing))
#define mstHyphenate          Mst(0x7012, StSharedKey ("Hyphenating, \037\003  0% complete.  Press Esc to cancel.",Hyphenate))
#define mstPrintingPage       Mst(0x7013, StSharedKey ("Printing \006\000 page \037\004\002\004.  Press Esc to cancel.",PrintingPage))
#define mstNumChars           Mst(0x7014, StSharedKey ("\006\000: \003\000 Chars.",NumChars))
#define mstSaving             Mst(0x7015, StSharedKey ("Saving \004\000, \037\003  0% complete.",Saving))
#define mstFastSave           Mst(0x7016, StSharedKey ("Fast saving \004\000, \037\003  0% complete.",FastSave))
#define mstFullSave           Mst(0x7017, StSharedKey ("Fast save failed; saving \004\000, \037\003  0% complete.",FullSave))
#define mstMath               Mst(0x7018, StSharedKey ("The result of computation is: \001\000",Math))
#define mstMergingRecord      Mst(0x7019, StSharedKey ("Merging record \037\004\002\004.  Press Esc to cancel.",MergingRecord))
#define mstPMIllegalBkmk      Mst(0x701A, StSharedKey ("Not a valid bookmark name: \001\000",PMIllegalBkmk))
#define mstPMMismatch         Mst(0x701B, StSharedKey ("Number of fields does not match number of names in record \002\000. Continue with Print Merge?",PMMismatch))
#define mstReplaceFile        Mst(0x701C, StSharedKey ("Replace existing \001\000?",ReplaceFile))
#define mstOverwriteForeign   Mst(0x701E, StSharedKey ("Overwrite foreign format \001\000 with \001\000?",OverwriteForeign))
#define mstDdeDataNotAvail    Mst(0x701F, StSharedKey ("Cannot obtain data from \001\000 for \001\000, \001\000.",DdeDataNotAvail))
#define mstOkToLaunchDde      Mst(0x7020, StSharedKey ("Remote data (\001\000) not accessible; start application \001\000?",NoRemoteData))
#define mstStartDdeHot        Mst(0x7021, StSharedKey ("Start external automatic links for \006\000?",StartDdeHot))
#define mstDefineUndefSty     Mst(0x7022, StSharedKey ("Define Style \"\001\000\" based on selection?",DefineUndefSty))
#define mstRepaginating       Mst(0x7023, StSharedKey ("Repaginating \006\000, pg \037\004\002\004.  Press Esc to cancel.",Repaginating))
#define mstScanningIndex      Mst(0x7024, StSharedKey ("Searching for index entries, pg \037\004   1.  Press Esc to cancel.",ScanningIndex))
#define mstPrintingSub        Mst(0x7025, StSharedKey ("Printing \006\000\001\000.  Press Esc to cancel.",PrintingSub))
#define mstRenumber           Mst(0x7026, StSharedKey ("Renumbering, \037\003  0% complete.  Press Esc to cancel.",Renumber))
#define mstBuildingToc        Mst(0x7027, StSharedKey ("Building table of contents, pg \037\004   1.  Press Esc to cancel.",BuildingToc))
#define mstConverting         Mst(0x7028, StSharedKey ("Converting, \037\003  0% complete.  Press Esc to cancel.",Converting))
#define mstDdeTimeOut         Mst(0x7029, StSharedKey ("DDE timed out.  Continue waiting?",DdeTimeOut))
#define mstRedefineStyle      Mst(0x702A, StSharedKey ("Redefine Style \"\001\000\" based on selection?",RedefineStyle))
#define mstNumCharsRO         Mst(0x702B, StSharedKey ("\006\000: \003\000 Chars.  (Read Only)",NumCharsRO))
#define mstNumCharsLFA        Mst(0x702C, StSharedKey ("\006\000: \003\000 Chars.  (Locked by \001\000)",NumCharsLFA))
#define mstCopyTo             Mst(0x702D, StSharedKey ("Copy to where?",CopyTo))
#define cxtCopyTo           0x702D
#define mstCopyFrom           Mst(0x702E, StSharedKey ("Copy from where?",CopyFrom))
#define mstFormatTo           Mst(0x702F, StSharedKey ("Format to where?",FormatTo))
#define cxtFormatTo         0x702F
#define mstFormatFrom         Mst(0x7030, StSharedKey ("Format from where?",FormatFrom))
#define mstCountingDoc        Mst(0x7031, StSharedKey ("Counting words, \037\003  0% complete.",CountingDoc))
#define mstCountingDocFtn     Mst(0x7032, StSharedKey ("Counting words in footnote, \037\003  0% complete.",CountingDocFtn))
#define mstCatDelete          Mst(0x7033, StSharedKey ("Delete \001\366?",CatDelete))
#define mstCatDeleted         Mst(0x7034, StSharedKey ("Deleted \001\103",CatDeleted))
#define mstCatCantDelete      Mst(0x7035, StSharedKey ("Cannot delete. \001\103 is \001\017.",CatCantDelete))
#define mstAboutKB            Mst(0x7036, StSharedKey ("\003\000 KB Free",AboutKB))
#define mstSaveChangesDoc     Mst(0x7037, StSharedKey ("Save changes to \006\000?",SaveChangesDoc))
#define mstSaveChangesGdt     Mst(0x7038, StSharedKey ("Save global glossary and command changes?",SaveChangesGdt))
#define mstPrinting           Mst(0x703A, StSharedKey ("Printing \006\000.  Press Esc to cancel.",Printing))
#define mstRepagNoAbort       Mst(0x703B, StSharedKey ("Repaginating \006\000, pg \037\004\002\004.",RepagNoAbort) /* no abort */)
#define mstColor              Mst(0x703C, StSharedKey ("Which color? ",WhichColor))
#define mstSort               Mst(0x703D, StSharedKey ("Sorting...  Press Esc to cancel.",Sort))
#define mstHyphComplete       Mst(0x703E, StSharedKey ("Hyphenation complete.",HyphComplete))
#define mstSearchWrapFwd      Mst(0x703F, StSharedKey ("Reached end of document.  Continue search at beginning?",SearchWrapFwd))
#define mstSearchWrapBkwd     Mst(0x7040, StSharedKey ("Reached beginning of document.  Continue search at end?",SearchWrapBkwd))
#define mstHdrConfirmDelete   Mst(0x7041, StSharedKey ("Delete this header/footer and link to previous section?",HdrConfirmDelete))
#define mstReplaceHdr         Mst(0x7042, StSharedKey ("Replace existing header/footer with page numbers?",ReplaceHdr))
#define mstHypRepeat          Mst(0x7043, StSharedKey ("Continue hyphenation from beginning of document?",HypRepeat))
#define mstPageMismatch       Mst(0x7044, StSharedKey ("Document page size different from printer page size. Check values in Format Document, File Printer Setup and portrait/landscape options.  Continue printing?",PageMismatch))
#define mstPageMismatchNP     Mst(0x7044, StSharedKey ("Document page size different from printer page size. Check values in Format Document, File Printer Setup and portrait/landscape options.",PageMismatchNP))
#define mstBadMargins         Mst(0x7045, StSharedKey ("Margins set outside printable area of page.  Continue?",BadMargins))
#define mstBadMarginsNP       Mst(0x7045, StSharedKey ("Margins set outside printable area of page.",BadMarginsNoPrint))
#define mstSaveDotToo         Mst(0x7046, StSharedKey ("Save changes to document template too?",SaveDotToo))
#define mstNukeStdStcProps    Mst(0x7047, StSharedKey ("Change properties of standard style?",NukeStdStcProps))
#define mstSaveStyleChange    Mst(0x7048, StSharedKey ("Save changes to \007\000?",SaveStyleChange))
#define mstDeleteStyle        Mst(0x7049, StSharedKey ("Delete \007\000?",DeleteStyle))
#define mstMergeStyleMunge    Mst(0x704A, StSharedKey ("Merge replaces styles with the same name.  Continue?",MergeStyleMunge))
#define mstClipLarge          Mst(0x704B, StSharedKey ("Not enough memory for large clipboard.  Discard?",ClipLarge))
#define mstAcceptAllRM        Mst(0x704C, StSharedKey ("Accept all revisions?",AcceptAllRM))
#define mstUndoAllRM          Mst(0x704D, StSharedKey ("Undo all revisions?",UndoAllRM))
#define mstManyStyles         Mst(0x704E, StSharedKey ("Operation involves copying many styles. Use Normal style instead?",ManyStyles))
#define mstSpellCreateDict    Mst(0x704F, StSharedKey ("User dictionary not found. Create?",SpellCreateDict))
#define mstSpellWrapToBegin   Mst(0x7050, StSharedKey ("Continue checking at beginning of document?",SpellWrapToBegin))
#define mstReplaceIndex       Mst(0x7051, StSharedKey ("Replace existing index?",ReplaceIndex))
#define mstReplaceToc         Mst(0x7052, StSharedKey ("Replace existing table of contents?",ReplaceToc))
#define mstUndo               Mst(0x7053, StSharedKey ("Not enough memory for Undo. Continue command without Undo?",Undo))
/*                                0x7054 no longer used */
#define mstSaveRescue         Mst(0x7055, StSharedKey ("Insufficient memory, save \006\000 as \007\000?",SaveRescue))
#define mstConvertNA          Mst(0x7056, StSharedKey ("Converting, \037\003  0% complete.",ConvertNA))
#define mstSaveLargeClip      Mst(0x7057, StSharedKey ("Save large clipboard?",SaveLargeClipBoard))
#define mstSaveChangesMcr     Mst(0x7058, StSharedKey ("Keep changes to \001\000?",SaveChangesMcr))
#define mstSaveMacroRecording Mst(0x7059, StSharedKey ("Keep macro recording?",SaveMacroRecording))
#define mstOneChange          Mst(0x705A, StSharedKey ("\002\000 change.",OneChange))
#define mstLaunchDraw         Mst(0x705B, StSharedKey ("\001\000 /L \"\001\000\"", DrawCmdLine))
#define mstRedefineGlsy       Mst(0x705C, StSharedKey ("Redefine glossary?",RedefineGlsy))
#define mstRescueDoc          Mst(0x705D, StSharedKey ("RESCUE\010\000.DOC",RescueDoc))
#define mstWidthOfColumn      Mst(0x705E, StSharedKey ("&Width of Column \002\000:", WidthOfColumn))
#define mstWidthOfColumns     Mst(0x705F, StSharedKey ("&Width of Columns \002\000-\002\000:", WidthOfColumns))
#define mstReplMacro	      Mst(0x7060, StSharedKey ("Replace existing macro?", ReplaceExistingMacro))
#define mstCantUndoOp         Mst(0x7061, StSharedKey ("Cannot Undo this operation. Continue without Undo?", CannotUndoOp))
#define mstNukeMatchingStyle  Mst(0x7062, StSharedKey ("Style already exists.  Define with new properties?", NukeMatchingStyle))
#define mstCompare            Mst(0x7063, StSharedKey ("Comparing documents, \037\003  0% complete.  Press Esc to cancel.",Comparing))
#define mstPageSwap           Mst(0x7064, StSharedKey ("Change current document size (Format Document) to fit on new page size (File Printer Setup)?",PageSwap))


		/*******************************************************/
		/***  Add new mst definitions here, above this line. ***/
		/*******************************************************/

#ifdef DEBUG
			/* Debug strings do not need valid cxt values. */
#define mstStackTraceMat      Mst(0, StShared ("Memory error set (\002\000). Generate stack trace?"))
#define mstStackTraceErr      Mst(0, StShared ("Generate stack trace?"))
#define mstDbgDdeError        Mst(0, StShared ("DDE ERROR: \001\000  App: \001\000, message: \001\000 (\002\000)."))
#define mstDbgNextDlg         Mst(0, StShared ("Next dialog: "))
#define mstDbgCkTlbx          Mst(0, StShared ("Toolbox trashed!  tlbx between \002\000 and \002\000. (Assert will follow)."))
#define mstDbgFont            Mst(0, StShared ("Font Arena data for \002\000 fonts is in the clipboard."))
#define mstDbgAssert          Mst(0, StShared ("Assertion Failed. File: \001\000 Line: \002\000."))
#define mstDbgReport          Mst(0, StShared ("Report: \001\000, line \002\000: \001\000."))
#define mstDbgAssertSz        Mst(0, StShared ("Assertion Failed. File: \001\000 Line: \002\000. \001\000."))
#define mstDbgInt3            Mst(0, StShared ("Int 3 Encountered!"))
#define mstDbgBmpSize         Mst(0, StShared ("Bitmap size information is in the clipboard."))
#define mstDbgSDMAssert       Mst(0, StShared ("SDM Assertion Failed. File: \001\000 Line: \002\000."))
#define mstDbgScanErrors      Mst(0, StShared ("Continue Error Message Scan? \002\000 of \002\000"))
#define mstDbgErrorStart      Mst(0, StShared ("Starting iemd value: "))
#define mstDbgErrorIemd       Mst(0, StShared (" iemd = \002\000    eid = \002\000   For Help, Press F1"))
#define mstDebugTestComplete  Mst(0, StShared ("Debug Test Complete."))
#endif /* DEBUG */


/* Progress Abort Types */

#define patmReport          0x0001
#define patmAbort           0x0002
#define patmChain           0x0004
#define patmDelayReport     0x0008	/* report only if operation is long */

#define patSilent           (0)
#define patReport           (patmReport)
#define patAbort            (patmReport | patmAbort)

