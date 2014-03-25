# Command definitions for Opus

# Format of a command:
#
#       MacroName, CmdName [ <parameters> ] [ <*options> ] , [bcmName] \
#               ? "Menu help text"
#
# Parameters may be listed in braces for commands, or parentheses for
# OpEL statements.  Parameters following a semicolon are optional.
#
# IMPORTANT: Supply a menu help string for all commands, whether or not
# they normally appear on a menu!
#
# Backslashes before new-lines work just like they do in C.
#
# Explanation of the *ABDEFHKLMOP options:
#
#     Set...    If command cannot be executed when...
#       A       the current document is annotation
#       D       there is no current document
#       F       the current document is a footnote
#       H       the current document is a header or footer
#       K       the current document's mother is locked
#       L       the current document is locked
#       M       current document is a macro
#       O       current window pane is in outline mode
#
#     Set...    If command can be executed when...
#       P       in preview mode
#
#     Set...    If command will not terminate...
#       B       block (column) mode
#       E       extend mode
#
#     Set...
#       R	If command is repeatable
#	Y	If macro statement does NOT cancel dyadic operations


COMMANDS

    Help, CmdHelp *P, bcmHelp \
        ? "Menu of Help choices"

    HelpContext, CmdHelpContext *P \
        ? "Displays help for current task or command"

    HelpUsingHelp, CmdHelpUsingHelp *P \
        ? "Displays instructions about how to use help"

    HelpActiveWindow, CmdHelpActiveWindow *P \
        ? "Displays information about the active pane or document view"

    HelpKeyboard, CmdHelpKeyboard *P \
        ? "Lists keys and their actions"

    HelpIndex, CmdHelpContents *P \
        ? "Lists Help topics"

    HelpTutorial, CmdHelpTutorial *P \
        ? "Lists lessons for learning Word"

    HelpAbout, CmdAbout @about *P, bcmAbout \
        ? "Displays program information, Word version number and copyright"



# Keyboard commands

    GrowFont, CmdGrowFont *LMDR, bcmGrowFont \
	? "Increases the point size of the selection"
	   
    ShrinkFont, CmdShrinkFont *LMDR, bcmShrinkFont \
	? "Decreases the point size of the selection"

    Overtype, CmdOvertype, bcmOverType \
        ? "Toggles typing mode between replacing and inserting"

    ExtendSelection, CmdExtend *DE, bcmExtendSel \
        ? "Turns on extend selection mode; then expands the selection with direction keys"

    Spike, CmdSpike *DLR, bcmSpike \
        ? "Deletes selection and adds it to special glossary"

    UnSpike, CmdUnspike *DL, bcmUnspike \
        ? "Empties spike glossary and inserts all contents into document"

    ChangeCase, CmdSwapCase *DLR, bcmChangeCase \
        ? "Changes the case of letters in the selection"

    MoveText, CmdMove *DLR, bcmMoveSp \
        ? "Moves selection to specified location"

    CopyText, CmdCopyToFrom *DR, bcmCopySp \
        ? "Makes copy of selection at specified location"

    ExpandGlossary, CmdExpandGlsy *DLR, bcmExpandGlsy \
        ? "Replaces name of glossary item with its contents"

    OtherPane, CmdOtherPane *D, bcmOtherPane \
        ? "Switches to the other window pane"

    NextWindow, CmdNextMwd *D, bcmNextMwd \
        ? "Switches to the next document window"

    PrevWindow, CmdPrevMwd *D, bcmPrevMwd \
        ? "Switches back to the previous document window"

    RepeatSearch, CmdFindNext *DE, bcmFindNext \
        ? "Repeats Go To or Search to find the next occurrence"

    NextField, CmdNextField *MDE, bcmNextField \
        ? "Moves to the next field"

    PrevField, CmdPreviousField *MDE, bcmPreviousField \
        ? "Moves to the previous field"

    ColumnSelect, CmdBlkExtend *DBO, bcmBlockSel \
        ? "Selects a columnar block of text"

    DeleteWord, CmdDelWordClear *DLR, bcmDeleteWord \
        ? "Deletes next word; does not put it on the Clipboard"

    DeleteBackWord, CmdDelBackWordClear *DLR, bcmDeleteBackWord \
        ? "Deletes previous word; does not put it on the Clipboard"

    EditClear, CmdDelClear *DLR, bcmClear \
        ? "Removes selection; does not put it on the Clipboard"

    InsertFieldChars, CmdInsertField *MDLR, bcmInsFieldChars \
        ? "Inserts a field with enclosing field characters"

    UpdateFields, CmdCalcSel *MDR, bcmCalcFields \
        ? "Updates and displays results of selected fields"

    UnLinkFields, CmdReplaceField *MDLR, bcmDerefField \
        ? "Permanently replaces field codes with results"

    ToggleFieldDisplay, CmdToggleFieldDisplay *MDLR \
        ? "Shows field codes or results for selection (toggle)"

    LockFields, CmdFldLocksOn *MDLR \
        ? "Locks selected fields to prevent updating"

    UnLockFields, CmdFldLocksOff *MDLR \
        ? "Unlocks selected fields for updating"

    UpdateSource, CmdUpdateSource *MDR \
        ? "Copies modified text of a linked file back to its source"

    Indent, CmdIndent *MDLR, bcmIndent \
        ? "Moves left indent to next tab stop"

    UnIndent, CmdUnIndent *MDLR, bcmUnIndent \
        ? "Moves left indent to previous tab stop"

    HangingIndent, CmdHangingIndent *MDLR, bcmHangingIndent \
        ? "Increases hanging indent"

    UnHang, CmdUnHang *MDLR, bcmUnHang \
        ? "Decreases hanging indent"

    Font, CmdFont *MDLR, bcmFont \
        ? "Chooses font for selection"

    FontSize, CmdFontSize *MDLR, bcmFontSize \
        ? "Chooses point size for selection"

    RulerMode, CmdEditRuler *DLMOR, bcmEditRuler \
        ? "Activates ruler"

    Bold, CmdBold *MDLR, bcmBold \
        ? "Makes selection bold (toggle)"

    Italic, CmdItalic *MDLR, bcmItalic \
        ? "Makes selection italic (toggle)"

    SmallCaps, CmdSmallCaps *MDLR, bcmSmallCaps \
        ? "Makes selection small capitals (toggle)"

    Hidden, CmdHideText *MDLR, bcmHideText \
        ? "Makes selection hidden text (toggle)"

    Underline, CmdULine *MDLR, bcmULine \
        ? "Underlines selection continuously (toggle)"

    DoubleUnderline, CmdDULine *MDLR, bcmDULine \
        ? "Double underlines selection (toggle)"

    WordUnderline, CmdWULine *MDLR, bcmWULine \
        ? "Underlines words but not spaces (toggle)"

    SuperScript, CmdSuperscript *MDLR, bcmSuperscript \
        ? "Raises selection 3 points above base line"

    SubScript, CmdSubscript *MDLR, bcmSubscript \
        ? "Lowers selection 2 points below base line"

    ResetChar, CmdPlainText *MDLR, bcmPlainText \
        ? "Makes selection default character format of applied style"

    CharColor, CmdColor *MDLR, bcmColor \
        ? "Changes color of selected text"

    LeftPara, CmdParaLeft *MDLR, bcmParaLeft \
        ? "Aligns paragraph at left indent"

    CenterPara, CmdParaCenter *MDLR, bcmParaCenter \
        ? "Centers paragraph between indents"

    RightPara, CmdParaRight *MDLR, bcmParaRight \
        ? "Aligns paragraph at right indent"

    JustifyPara, CmdParaBoth *MDLR, bcmParaBoth \
        ? "Aligns paragraph at both left and right indent"

    SpacePara1, CmdSpace1 *MDLR, bcmSpace1 \
        ? "Sets line spacing to single space"

    SpacePara15, CmdSpace15 *MDLR, bcmSpace15 \
        ? "Sets line spacing to one and one-half space"

    SpacePara2, CmdSpace2 *MDLR, bcmSpace2 \
        ? "Sets line spacing to double space"

    CloseUpPara, CmdParaClose *MDLR, bcmParaClose \
        ? "Eliminates open spacing between paragraphs"

    OpenUpPara, CmdParaOpen *MDLR, bcmParaOpen \
        ? "Opens a line of space between paragraphs"

    ResetPara, CmdParaNormal *MDLR, bcmParaNormal \
        ? "Makes selection default paragraph format of applied style"

    EditRepeat, CmdRepeat, bcmRepeat \
        ? "Repeats the last action"

    GoBack, CmdPrevInsert *DR \
        ? "Returns to previous insertion point"

    SaveTemplate, CmdSaveDot *MD, bcmSaveDot \
        ? "Saves the document template of the active document"


    OK, CmdOK, bcmOK

    Cancel, CmdEscape *EBP, bcmCancel 


    CopyFormat, CmdCopyLooks *MRD, bcmCopyLooks \
        ? "Copies the formatting of the selection to a specified location"

    PrevPage, CmdPgvPrevPage *MD, bcmPgvPrevPage \
        ? "Moves to the previous page"

    NextPage, CmdPgvNextPage *MD, bcmPgvNextPage \
        ? "Moves to the next page"

    NextObject, CmdDrCurskeys *MD, bcmNextDr \
        ? "Moves to the next object on the page"

    PrevObject, CmdDrCurskeys *MD, bcmPrevDr \
        ? "Moves to the previous object on the page"


# File Menu Commands

    FileNew, CmdNew @new *R, bcmFileNew \
        ? "Creates a new document or template"

    FileOpen, CmdOpen @open *R \
        ? "Opens an existing document or template"

    FileSave, CmdSave *MDP, bcmSave \
        ? "Saves the active document or template"

    FileSaveAs, CmdSaveAs @saveas *MDP \
        ? "Saves the active document with a new name"

    FileSaveAll, CmdSaveAll *PR \
        ? "Saves all open files, macros and glossary; prompts for each"
    
    FilePrint, CmdPrint @print *DP, bcmPrint \
        ? "Prints the active document"

    FilePrintPreview, CmdPrintPreview *MDP, bcmPrintPreview \
        ? "Displays full pages; no editing"

    FilePrintMerge, CmdPrintMerge @printmrg *MDLKR \
        ? "Combines files to produce form letters"

    FilePrinterSetup, CmdChangePrinter @chgpr, bcmChgPr \
        ? "Changes the printer and printing options"

    FileExit, CmdFileExit *P, bcmExit \
        ? "Quits Microsoft Word; prompts to save documents"

    FileFind, CmdCatalog @catalog, bcmCatalog \
        ? "Locates documents in any directory or drive"

    File1, CmdWkOn1, imiWkOn1 \
        ? "Opens this document"

    File2, CmdWkOn2, imiWkOn2 \
        ? "Opens this document"

    File3, CmdWkOn3, imiWkOn3 \
        ? "Opens this document"

    File4, CmdWkOn4, imiWkOn4 \
        ? "Opens this document"


# Edit Menu Commands

    EditUndo, CmdUndo *D, bcmUndo \
        ? "Reverses the last action"

    EditCut, CmdDelCut *DLR, bcmCut \
        ? "Cuts the selection and puts it on the Clipboard"

    EditCopy, CmdCopy *DR, bcmCopy \
        ? "Copies the selection and puts it on the Clipboard"

    EditPaste, CmdPaste *DLR, bcmPaste \
        ? "Inserts Clipboard contents at the insertion point"

    EditPasteLink, CmdPasteLink @pastelnk *MDLR, bcmPasteLink \
        ? "Inserts Clipboard contents and a link to its source"

    EditSearch, CmdSearch @search *DE, bcmSearch \
        ? "Finds text or formatting"

    EditSearchChar, CmdSearchChar @char, bcmSearchChar 

    EditSearchPara, CmdSearchPara @para, bcmSearchPara

    EditReplace, CmdReplace @replace *DLR \
        ? "Finds text or formatting and changes it"

    EditReplaceChar, CmdReplaceChar @char, bcmReplaceChar

    EditReplacePara, CmdReplacePara @para, bcmReplacePara

    EditGoTo, CmdGoto @goto *DEM, bcmGoto \
        ? "Jumps to a specified place in the active document"

    EditHeaderFooter, CmdInsHeader @header *MDAFL \
        ? "Shows list of headers and footers for editing"

    EditSummaryInfo, CmdSummaryInfo @docsum *MDLOK, bcmSummaryInfo \
        ? "Shows summary information about the active document"

    EditGlossary, CmdGlossary @glsy *DLK \
        ? "Inserts or defines glossary items"

    EditTable, CmdEditTable @tablecmd *MDL \
        ? "Insert, delete table rows or columns and merge, split table cells"


# View Menu Commands

    ViewOutline, CmdOutline *MDFAHP \
        ? "Displays a document's outline (toggle)"

    ViewDraft, CmdDraftView *DP \
        ? "Displays document without formatting and pictures (toggle); for faster editing"

    ViewPage, CmdPageView *MDP, bcmPageView \
        ? "Displays page as printed; permits editing (toggle)"

    ViewRibbon, CmdRibbon *M, bcmRibbon \
        ? "Toggles the ribbon on/off"

    ViewRuler, CmdRuler *MDO, bcmRuler \
        ? "Toggles the ruler on/off"

    ViewStatusBar, CmdStatusArea *BE,  bcmStatusArea \
        ? "Toggles the status bar on/off"

    ViewFootnotes, CmdViewFootnote *MDAH, bcmViewFootnote \
        ? "Opens the Footnote window for viewing and editing footnotes (toggle)"

    ViewAnnotations, CmdViewAnnotation *MDFH, bcmViewAnnotation \
        ? "Opens the Annotation window for reading annotations (toggle)"

    ViewFieldCodes, CmdFieldCodes *MD \
        ? "Shows field codes or results for all fields (toggle)"

    ViewPreferences, CmdViewPreferences @viewpref *D \
        ? "Sets document window display options"

    ViewShortMenus, CmdShortMenus \
        ? "Switches to the short version of menus"

    ViewFullMenus, CmdLongMenus \
        ? "Switches to the full-length version of menus"


# Insert Menu Commands

    InsertTable, CmdInsTable @tableins *MDLR, bcmInsTable \
        ? "Inserts a table"

    InsertTableToText, CmdTableToText @tabletxt *MDLR, bcmTableToText \
        ? "Converts a table to text"

    InsertBreak, CmdInsBreak @insbreak *MDAFHLOR, bcmInsBreak \
        ? "Ends a page, column, or section at the insertion point"

    InsertFootnote, CmdInsFootnote @footnote *MDAHLR, bcmInsFootnote \
        ? "Inserts a footnote reference at the insertion point"

    InsertAnnotation, CmdInsAnnotation *MDFHR, bcmInsAnnotation \
        ? "Inserts a comment; activates the annotations pane"

    InsertPicture, CmdInsPicture @inspic *MDLR, bcmInsPic \
        ? "Inserts a picture from a graphic file, or inserts an empty picture frame"

    InsertFile, CmdInsFile @insfile *MDLR, bcmInsFile \
        ? "Inserts the text of another file into the active document"

    InsertField, CmdInsField @insfield *MDLR, bcmInsField \
        ? "Inserts instructions for inserting computed contents"

    InsertBookmark, CmdInsBookmark @bookmark *MDFAHL, bcmInsBookmark \
        ? "Assigns a name to the selection"

    InsertIndexEntry, CmdIndexEntry @indexent *MDFHAL, bcmIndexEntry \
        ? "Inserts the text you want to include in the index"

    InsertIndex, CmdIndex @index *MDFAHL, bcmIndex \
        ? "Collects index entries into an index"

    InsertTableOfContents, CmdTOC @toc *MDFAHL \
        ? "Collects headings or table of contents entries into a table of contents"


# Format Menu Commands

    FormatCharacter, CmdCharacter @char *MDLR, bcmCharacter \
        ? "Changes the appearance of the selected characters"

    FormatParagraph, CmdParagraph @para *MDLOR, bcmParagraph \
        ? "Changes the appearance and line numbering of the selected paragraphs"

    FormatSection, CmdSection @sect *MDFAHLR, bcmSection \
        ? "Changes the page format within sections"

    FormatDocument, CmdDocument @doc *MDLKPR, bcmDocument \
        ? "Changes the page format for the entire document"

    FormatTabs, CmdTabs @tabs *MDLOR, bcmTabs \
        ? "Sets and clears tab stops for the selected paragraphs"

    FormatStyles, CmdApplyStyDlg @style *MDLKR, bcmApplyStyleDlg \
        ? "Applies styles; creates and changes styles using the selection"

    FormatDefineStyles, CmdDefineStyle @edstyle *MDLKR, \
        bcmStyles \
        ? "Creates and changes styles using commands"

    FormatDefineStylesChar, CmdStyChar @char *MDLKR, bcmStyChar
    FormatDefineStylesPara, CmdStyPara @para  *MDLKR, bcmStyPara
    FormatDefineStylesTabs, CmdStyTabs @tabs *MDLKR, bcmStyTabs
    FormatDefineStylesPosition, CmdStyPos @abspos *MDLKR, bcmStyPos

    FormatPosition, CmdPosition @abspos *MDLAFKR, bcmPosition \
        ? "Positions the selected objects or paragraphs on a page"

    FormatPicture, CmdPicture @pict *MDLO \
        ? "Changes the borders and size of a picture frame and scales the picture"

    FormatTable, CmdFormatTable @tablefmt *MDLR, bcmFormatTable \
        ? "Changes the appearance of rows and columns in a table"


# Utilities Menu Commands

    UtilSpelling, CmdSpelling @spell *MDLOR \
        ? "Checks spelling in the active document"

    UtilSpellSelection, CmdSpellDoc *MDLOR, bcmSpellSelection \
        ? "Checks spelling of the selected text"

    UtilThesaurus, CmdThesaurus *MDLOR \
        ? "Finds a synonym for the selected word"

    UtilHyphenate, CmdHyphenate @hyphen *MDLOR \
        ? "Hyphenates the selection or entire document"

    UtilRenumber, CmdRenumParas @renum *MDLR \
        ? "Changes the paragraph numbering"

    UtilRevisionMarks, CmdRevMarking @revmark *MDLK \
        ? "Toggles revision marking on/off"

    UtilCompareVersions, CmdCompare @cmpfile *MDLKR \
        ? "Compares the active document with an earlier version"

    UtilSort, CmdSort @sort *MDLAFR \
        ? "Rearranges the selection into a specified order"

    UtilCalculate, CmdCalculate *MDOR \
        ? "Checks the arithmetic of the selection"

    UtilRepaginateNow, CmdRepaginate *MDLKRP \
        ? "Recalculates page breaks"

    UtilCustomize, CmdCustomize @cust, bcmCustomize \
        ? "Sets Auto-save, typing mode, unit of measure, and background pagination"


# Macro Menu Commands

    MacroRun, CmdMacro @runmacro *PR \
        ? "Executes a prerecorded or preprogrammed sequence of actions"

    MacroRecord, CmdRecorder @recorder *P, bcmRecorder \
        ? "Records actions as a macro"

    MacroEdit, CmdEditMacro @edmacro, bcmEditMacro \
        ? "Creates, deletes, or revises a macro"

    MacroAssignToKey, CmdChangeKeys @asgn2key *R, bcmChangeKeys \
        ? "Changes the keyboard assignments"

    MacroAssignToMenu, CmdAssignToMenu @asgn2mnu *R, bcmAssignToMenu \
        ? "Adds, deletes, and moves menu commands"

    PauseRecorder, CmdPauseRecorder *RP, bcmPauseRecorder \
    	? "Pauses the macro recorder (toggle)"


# Window Menu Commands

    WindowNewWindow, CmdNewWnd *MDR \
        ? "Opens another window for the active document"

    WindowArrangeAll, CmdArrangeWnd *D, bcmArrangeWnd \
        ? "Arranges windows as non-overlapping tiles"

    Window1, CmdMwd1 *D, imiWnd1 \
        ? "Switches to the window containing this document"

    Window2, CmdMwd2 *D, imiWnd2 \
        ? "Switches to the window containing this document"

    Window3, CmdMwd3 *D, imiWnd3 \
        ? "Switches to the window containing this document"

    Window4, CmdMwd4 *D, imiWnd4 \
        ? "Switches to the window containing this document"

    Window5, CmdMwd5 *D, imiWnd5 \
        ? "Switches to the window containing this document"

    Window6, CmdMwd6 *D, imiWnd6 \
        ? "Switches to the window containing this document"

    Window7, CmdMwd7 *D, imiWnd7 \
        ? "Switches to the window containing this document"

    Window8, CmdMwd8 *D, imiWnd8 \
        ? "Switches to the window containing this document"

    Window9, CmdMwd9 *D, imiWnd9 \
        ? "Switches to the window containing this document"



# Hyphen Menu Commands

    DocSplit, CmdSplit *DE, bcmSplit \
        ? "Splits the active window horizontally; then adjusts the split"

    DocSize, CmdSizeWnd *DE, bcmSizeWnd \
        ? "Changes window size"

    DocMove, CmdMoveWnd *DE, bcmMoveWnd \
        ? "Changes window position"

    DocMaximize, CmdZoomWnd *DE, bcmZoomWnd \
        ? "Enlarges the active window to full size"

    DocRestore, CmdRestoreWnd *DE, bcmRestoreWnd \
        ? "Restores window to normal size"

    DocClose, CmdCloseWnd *DEP, bcmCloseWnd \
        ? "Closes the active window; prompts to save documents"



# Space Menu Commands

    ControlRun, CmdControlRun @apprun *P, bcmControlRun \
        ? "Displays the Control Panel or the Clipboard"

    ShrinkSelection, CmdShrinkExtend *DE, bcmShrinkExtend \
        ? "Shrinks the selection to the next smaller size"

    EditSelectAll, CmdSelectWholeDoc *DR, bcmSelectAll \
        ? "Selects the entire document"

    IconBarMode, CmdIconBarMode *DLP, bcmIconBarMode \
        ? "Activates the icon bar"

### Special Header Doc Commands ###
    InsertPageField, CmdInsertPageField *MDLR, bcmHdrPage \
        ? "Inserts page number field"

    InsertDateField, CmdInsertDateField *MDLR, bcmHdrDate \
        ? "Inserts date field"

    InsertTimeField, CmdInsertTimeField *MDLR, bcmHdrTime \
        ? "Inserts time field"

    EditHeaderFooterLink, CmdHdrLinkPrev *MDFAL, bcmHdrLinkPrev \
        ? "Links this header/footer to the previous section"

    ClosePane, CmdCloseLowerPane *D, bcmHdrRetToDoc \
        ? "Closes the window pane"


### Special Outline Commands ###
    OutlinePromote, CmdPromote *MDLR, bcmPromote \
        ? "Promotes selected paragraphs one level"

    OutlineDemote, CmdDemote *MDLR, bcmDemote \
        ? "Demotes selected paragraphs one level"

    OutlineMoveUp, CmdMoveUp *DLR, bcmMoveUp \
        ? "Moves selection above previous item in outline"

    OutlineMoveDown, CmdMoveDown *DLR, bcmMoveDown \
        ? "Moves selection below next item in outline"

    NormalStyle, CmdNormalStyle *MDLR, bcmConvertToBody \
        ? "Applies normal style; converts headings to body text"

    OutlineExpand, CmdExpand *MDLR, bcmExpand \
        ? "Displays next level of subtext of selection"

    OutlineCollapse, CmdCollapse *MDLR, bcmCollapse \
        ? "Hides lowest subtext of selection"

    ShowHeading1, CmdShowToLvl1 *MDLR, bcmShowToLevel1 \
        ? "Displays level 1 headings only"

    ShowHeading2, CmdShowToLvl2 *MDLR, bcmShowToLevel2 \
        ? "Displays level 1 and 2 headings"

    ShowHeading3, CmdShowToLvl3 *MDLR, bcmShowToLevel3 \
        ? "Displays level 1 through 3 headings"

    ShowHeading4, CmdShowToLvl4 *MDLR, bcmShowToLevel4 \
        ? "Displays level 1 through 4 headings"

    ShowHeading5, CmdShowToLvl5 *MDLR, bcmShowToLevel5 \
        ? "Displays level 1 through 5 headings"

    ShowHeading6, CmdShowToLvl6 *MDLR, bcmShowToLevel6 \
        ? "Displays level 1 through 6 headings"

    ShowHeading7, CmdShowToLvl7 *MDLR, bcmShowToLevel7 \
        ? "Displays level 1 through 7 headings"

    ShowHeading8, CmdShowToLvl8 *MDLR, bcmShowToLevel8 \
        ? "Displays level 1 through 8 headings"

    ShowHeading9, CmdShowToLvl9 *MDLR, bcmShowToLevel9 \
        ? "Displays level 1 through 9 headings"

    ShowAllHeadings, CmdExpandAll *MDLR, bcmExpandAll \
        ? "Displays all heading levels and body text"

    OutlineShowFirstLine, CmdToggleEllip *MDLR, bcmToggleEllip \
        ? "Toggles showing first line only, or all of body text in outline"


# Special macro commands

    ShowVars, CmdShowVars *EBDP, bcmShowVars \
        ? "Lists the active macro's variables"

    StepMacroSUBs, CmdTraceMacro *EBDP, bcmTraceMacro \
        ? "Runs the active macro one step at a time (steps through SUBs)"

    StepMacro, CmdStepMacro *EBDP, bcmStepMacro \
        ? "Runs the active macro one step at a time (SUBs are one step)"

    ContinueMacro, CmdContinueMacro *EBDP, bcmContinueMacro \
        ? "Continues running the active macro from current point"

    TraceMacro, CmdAnimateMacro *EBDP, bcmAnimateMacro \
        ? "Highlights each statement as the active macro executes it"

### Special Preview Commands ###

    FilePrintPreviewBoundaries, CmdPrvwBound *MPR, bcmPrvwBound \
        ? "Shows/Hides margins, etc. in Print Preview" 

    FilePrintPreviewPages, CmdPrvwPages *MPR, bcmPrvwPages \
        ? "Switches between one and two page view in Print Preview"

# Special table commands

    NextCell, CmdNextCell *MD, bcmNextCell \
        ? "Moves to the next table cell"

    PrevCell, CmdPrevCell *MD, bcmPrevCell \
        ? "Moves to the previous table cell"

    StartOfRow, CmdBeginRow *EMD, bcmBeginRow \
        ? "Moves to first cell in current row"

    EndOfRow, CmdEndRow *EMD, bcmEndRow \
        ? "Moves to last cell in current row"

    StartOfColumn, CmdTopColumn *EMD, bcmTopColumn \
        ? "Moves to first cell in current column"

    EndOfColumn, CmdBottomColumn *EMD, bcmBottomColumn \
        ? "Moves to last cell in current column"

    SelectTable, CmdSelectTable *EMDR, bcmSelectTable \
        ? "Selects entire table"

# Special leftover commands

    ShowAll, CmdShowAll *D, bcmShowAll \
        ? "Displays all hidden characters in window (toggle)"

    InsertPageBreak, CmdInsPageBreak *MDAFHLOR \
        ? "Inserts a page break at the insertion point"

    InsertColumnBreak, CmdInsColumnBreak *MDLR, bcmInsColumnBreak \
        ? "Inserts a column break at the insertion point"

    AppMinimize, ElWAppMinimize *EPR, bcmAppMinimize \
        ? "Reduces window to an icon"

    AppMaximize,            CmdAppMaximize *PR \
    	? "Enlarges application window to full size"

    AppRestore,             CmdAppRestore *PR \
    	? "Restores window to normal size"

    DoFieldClick, CmdDoFieldHit *MDR \
        ? "Executes action associated with button fields"

    FileClose, CmdFileClose *DPR \
        ? "Closes all windows on the current document"

# Enter menu mode (normaly bound to F10 and Alt) 
    MenuMode, CmdMenuMode *P, bcmMenuMode \
    	? "Activates the menu bar"

    InsertPageNumbers, CmdPageNumbers @inspgnum *MDAFLOR, bcmInsPgNum \
    	? "Adds page numbers to the top or bottom of pages"

    ChangeRulerMode, CmdRulerMode *MR, bcmRulerMode \
        ? "Changes the display mode of the ruler (paragraph, table and document)."

    EditPic, CmdEditPic *MDLR, bcmEditPic \
        ? ""

    UserDialog, CmdUserDialog @usrdlg *R, bcmUserDialog


#### M A C R O  I N T E R P R E T E R  C O M M A N D S ####

        Abs,            NUM	NumElAbsNum ( NUM ) *PEBY
        Sgn,            INT	IntElSgnNum ( NUM ) *PEBY
        Int,            INT	IntElIntInt ( INT ) *PEBY

        Len,            INT	IntElLenHpsd ( HPSD ) *PEBY

        Asc,            INT	IntElAscHpsd ( HPSD ) *PEBY
        Chr$,           SD	SdElChrInt ( INT ) *PEBY

        Val,            NUM	NumElValHpsd ( HPSD ) *PEBY
        Str$,           SD	SdElStrNum ( NUM ) *PEBY

	Left$,		SD	SdElLeftHpsdInt ( HPSD, INT ) *PEBY
	Right$,		SD	SdElRightHpsdInt ( HPSD, INT ) *PEBY
	Mid$,		SD	SdElMidHpsdIntOInt ( HPSD, INT ; INT ) *PEBY
	
	String$,	SD	SdElStringInt ( INT, HPSDNUM ) *PEBY
	
	Date$,		SD	ElSdDate () *PEBY
	Time$,		SD	ElSdTime () *PEBY

	Rnd,		NUM	ElNumRnd ( ; INT ) *PEBY

	InStr,		INT	IntElInstr ( HPSDNUM, HPSD ; HPSD ) *PEBY

	ShowAll,		ElShowAll ( ; INT ) *DLY
        ColumnSelect,            ElBlockSel () *DEB

        Insert,                 ElInsert ( HPSD ) *DL
        InsertPara,             ElInsPara () *DL
        Selection$,	SD	ElSdFetch () *DEBY

        GetBookmark$,   SD      ElSdBookmark( HST ) *DPMEBY
        CmpBookmarks,	INT	ElWCmpBk ( HST, HST ) *DPMEBY
        CopyBookmark,           ElCopyBk ( HST, HST ) *DPMLEBY
        SetStartOfBookmark,        ElStartOfBk ( HST ; HST ) *DPMLEBY
        SetEndOfBookmark,          ElEndOfBk ( HST ; HST ) *DPMLEBY
        ExistingBookmark,	INT	ElFNilBk ( HST ) *DPMEBY
        EmptyBookmark,	INT	ElFEmptyBk ( HST ) *DPMEBY
        CountBookmarks,	INT	ElWCountBk () *DPMEBY
        BookmarkName$,	SD	ElSdBkName ( INT ) *DPMEBY

	CountStyles,	INT	ElWCountStyles ( ; INT, INT) *DPEBMY
	StyleName$,	SD	ElSdStyleName ( ; INT, INT, INT ) *DPEBMY

        IsDirty,	INT	ElFIsDirty () *DPEBY
        SetDirty,               ElSetDirty ( ; INT ) *DPLEBY
        FileName$,	SD	ElSdFileName ( ; INT ) *PEBY
        CountFiles,	INT	ElWCountFiles () *PEBY

        GetGlossary$,	SD	ElSdGloss ( HST ; INT ) *DPEBY
        CountGlossaries,INT	ElWCountGloss ( ; INT ) *DPEBY
        GlossaryName$,	SD	ElSdGlossName ( INT ; INT ) *DPEBY
	SetGlossary,		ElSetGlossary ( HST, HST ; INT ) *DPEBY

        MsgBox,		; INT	ElMsgBox ( HST ; HPSDNUM, INT ) *PEBY
        Beep,                   ElBeep ( ; INT ) *PEBY
        Shell,                  ElLaunch ( HST ; INT ) *PEBY

	ResetChar,	; INT	ElWResetChar () *DML
	ResetPara,	; INT	ElWResetPara () *DML

	TabType,	INT	ElWTabType ( NUM ) *DMEBY
	TabLeader$,	SD	ElSdTabLeader ( NUM ) *DMEBY

        DocMove,                ElMoveWindow ( INT, INT ) *DEY
        DocSize,                ElSizeWindow ( INT, INT ) *DEY
        VLine,                  ElVline ( ; INT ) *DBEY
        HLine,                  ElHLine ( ; INT ) *DBEY
        VPage,                  ElVPage ( ; INT ) *DPBEY
        HPage,                  ElHPage ( ; INT ) *DBEY
        VScroll,	; NUM	ElNumVScroll ( ; NUM ) *DBEY
        HScroll,	; NUM	ElNumHScroll ( ; NUM ) *DBEY

        CountWindows,	INT	ElWCountWindows () *PBEY
        WindowName$,	SD	ElSdWindowName ( ; INT ) *DPBEY
        WindowPane,	INT	ElWPane () *DPBEY
        DocSplit,       ; INT   ElWSplitWindow ( ; INT ) *DEY
	Window,		INT	ElWWindow () *PBEY

        AppSize,                ElAppSize ( INT, INT ) *PBEY
        AppMove,                ElAppMove ( INT, INT ) *PBEY

	AppMinimize,	; INT	ElWAppMinimize ( ; INT ) *PBEY
	AppMaximize,	; INT	ElWAppMaximize ( ; INT ) *PBEY
	AppRestore,	; INT	ElWAppRestore () *PBEY

	DocMaximize,	; INT	ElWDocMaximize ( ; INT ) *DBEY

        GetProfileString$, SD	ElSdGetProfileString ( HST ; HST ) *PBEY
        SetProfileString,       ElSetProfileString ( HST, HST ; HST ) *PBEY


	CharColor,	; INT	ElCharColor ( ; INT ) *DM
        Bold,		; INT	ElBold ( ; INT ) *DM
        Italic,		; INT	ElItalic ( ; INT ) *DM
        SmallCaps,	; INT	ElSmallCaps ( ; INT ) *DM
        Hidden,		; INT	ElVanish ( ; INT ) *DM
        Underline,	; INT	ElUnderline ( ; INT ) *DM
        DoubleUnderline,; INT	ElDblUnderline ( ; INT ) *DM
        WordUnderline,	; INT	ElWordUnderline ( ; INT ) *DM
        SuperScript,	; INT	ElSuperscript ( ; INT ) *DM
        SubScript,	; INT	ElSubscript ( ; INT ) *DM

        CenterPara,	; INT	ElCenterPara () *DM
        LeftPara,	; INT	ElLeftPara () *DM
	RightPara,	; INT	ElRightPara () *DM
        JustifyPara,	; INT	ElJustifyPara () *DM
        SpacePara1,	; INT	ElSpace1 () *DM
        SpacePara15,	; INT	ElSpace15 () *DM
        SpacePara2,	; INT	ElSpace2 () *DM
        OpenUpPara,		ElOpenPara () *DM
        CloseUpPara,		ElClosePara () *DM


        DDEInitiate,	INT	ElDDEInitiate ( HST, HST ) *PBE
        DDETerminate,           ElDDETerminate ( INT ) *PBE
        DDETerminateAll,        ElDDETermAll () *PBE
        DDEExecute,             ElDDEExecute ( INT, HST ) *PBE
        DDEPoke,                ElDDEPoke ( INT, HST, SD ) *PBE
        DDERequest$,	SD	ElSdDDERequest ( INT, HST ) *PBE
#       DDEAdvise,              ElDDEAdvise ( INT, HST ) *P
#       DDEUnadvise,            ElDDEUnadvise ( INT, HST ) *P

        Activate,		ElActivate ( HST ; INT ) *PBEY
        AppActivate,            ElActivateApp ( HST ; INT ) *PBEY
        SendKeys,               ElSendKeys ( HST ; INT ) *PBEY
				
	EditFootnoteSep,	ElEditFNSep () *DML
	EditFootnoteContSep,	ElEditFNContSep () *DML
	EditFootnoteContNotice,	ElEditFNContNotice () *DML
	ResetFootnoteSep,	ElRsetFNSep () *DML
	ResetFootnoteContSep,	ElRsetFNContSep () *DML
	ResetFootnoteContNotice, ElRsetFNContNotice () *DML

        ViewFootnotes,  ; INT   ElViewFootnotes ( ; INT ) *MD
	ViewAnnotations,; INT	ElViewAnnotations ( ; INT ) *MD
        ViewFieldCodes, ; INT   ElViewFieldCodes ( ; INT ) *MD
        ViewDraft,      ; INT   ElViewDraft ( ; INT ) *DP
        ViewStatusBar,  ; INT   ElViewStatusArea ( ; INT ) *BE
        ViewRuler,      ; INT   ElViewRuler ( ; INT ) *M
        ViewRibbon,     ; INT   ElViewRibbon ( ; INT ) *M
        ViewShortMenus, ; INT   ElViewShortMenus ()
        ViewFullMenus,  ; INT   ElViewFullMenus ()
        ViewPage,       ; INT   ElViewPage ( ; INT ) *MDP
        ViewOutline,    ; INT   ElViewOutline ( ; INT ) *MDP
	ViewMenus,	; INT	ElViewMenus ()

	OutlineShowFirstLine, ; INT ElOutlineShowFirstLine ( ; INT ) *DLMR

	Overtype,	; INT	ElOvertype ( ; INT ) *Y

        Font$,		SD	ElFontName ( ; INT ) *DPBEY
        CountFonts,     INT     ElCountFonts () *DPBEY
	Font,			ElFont ( HST ; NUM ) *DML
	FontSize,	; NUM	ElNumFontSize ( ; NUM ) *DMY

	EditClear,		ElDelete ( ; INT ) *DL

	CharLeft,	; INT	ElLeftChar ( ; INT, INT ) *DBEY
	CharRight,	; INT	ElRightChar ( ; INT, INT ) *DBEY
	LineUp,		; INT	ElUpLine ( ; INT, INT ) *DBEY
	LineDown,	; INT	ElDownLine ( ; INT, INT ) *DBEY
	StartOfLine,	; INT	ElStartOfLine ( ; INT ) *DBEY
	EndOfLine,	; INT	ElEndOfLine ( ; INT ) *DBEY
	PageUp,		; INT	ElUpWindow ( ; INT, INT ) *DBEY
	PageDown,	; INT	ElDownWindow ( ; INT, INT ) *DBEY
	WordLeft,	; INT	ElLeftWord ( ; INT, INT ) *DBEY
	WordRight,	; INT	ElRightWord ( ; INT, INT ) *DBEY
	SentLeft,	; INT	ElLeftSent ( ; INT, INT ) *DBEY
	SentRight,	; INT	ElRightSent ( ; INT, INT ) *DBEY
	ParaUp,		; INT	ElUpPara ( ; INT, INT ) *DBEY
	ParaDown,	; INT	ElDownPara ( ; INT, INT ) *DBEY
	StartOfDocument,; INT	ElStartOfDoc ( ; INT ) *DBEY
	EndOfDocument,	; INT	ElEndOfDoc ( ; INT ) *DBEY
	StartOfWindow,	; INT	ElStartOfWindow ( ; INT ) *DBEY
	EndOfWindow,	; INT	ElEndOfWindow ( ; INT ) *DBEY

	NextPage,	; INT	FElNextPage ( ) *MDY
	PrevPage,	; INT	FElPrevPage ( ) *MDY
	NextObject,	; INT	FElNextObj ( ) *MDY
	PrevObject,	; INT	FElPrevObj ( ) *MDY

	ExtendSelection,	ElExtendSel ( ; HPSD ) *DEY

	SelType,	; INT	WElSelType ( ; INT ) *DBEY

	OutlineLevel,	INT	WElOutlineLevel () *DMBEY

	NextTab,	NUM	NumElNextTab ( NUM ) *DMBEY
	PrevTab,	NUM	NumElPrevTab ( NUM ) *DMBEY

	DisableInput,		ElDisableInput ( ; INT ) *PBEY

	RecordNextCommand,	CmdRecordNext *P, bcmRecordNext \
	    ? "Records the next command executed"

	DocClose,		ElDocClose ( ; INT ) *PED
	FileClose,		ElFileClose ( ; INT ) *DPR
	Files$,		SD	ElSdFiles ( ; HST ) *PEBY
	FileExit,	; INT	FElFileExit ( ; INT ) *P
	FileSaveAll,		ElFileSaveAll ( ; INT ) *PR

	FilePrintPreview, ; INT ElPrintPrvw ( ; INT ) *MDP
	FilePrintPreviewBoundaries, ; INT WElPrvwBoundaries ( ; INT ) *MPR
	FilePrintPreviewPages, ; INT WElPrvwPages ( ; INT ) *MPR

#	Open,			ElOpen ( HST, INT, HST ) *PBEY
	Close,			ElClose ( ; INT ) *PBEY
	Input$,		SD	ElSdInputn ( INT, INT ) *PBEY
#	Print,			ElWriteSt ( INT, HST ) *PBEY
	Seek,		; NUM	ElNumSeek ( INT ; NUM ) *PBEY
	Eof,		INT	ElWEof ( INT ) *PBEY
	Lof,		NUM	ElNumLof ( INT ) *PBEY
	Kill,			ElKill ( HST ) *PBEY
#	Name,			ElName ( HST, HST ) *PBEY
	ChDir,			ElChdir ( HST ) *PBEY
	MkDir,			ElMkdir ( HST ) *PBEY
	RmDir,			ElRmdir ( HST ) *PBEY

	UCase$,		SD	ElSdUcase ( HPSD ) *PBEY
	LCase$, 	SD	ElSdLcase ( HPSD ) *PBEY

	InputBox$,	SD	ElSdInputBox ( HST ; HST, HST ) *PBEY

	RenameMenu,		ElRenameMenu ( INT, HST ) *PBEY
	OnTime,			ElOnTime ( HPSDNUM, HST ; NUM ) *PBEY
	ChangeCase,	; INT	ElWChangeCase ( ; INT ) *DLR
	AppInfo$,	SD	ElSdAppInfo ( INT ) *PBEY

	CountMacros,	INT	ElWCountMacros ( ; INT, INT ) *PBEY
	MacroName$,	SD	ElSdMacroName ( INT ; INT, INT ) *PBEY

	CountMenuItems,	INT	ElCountMenuItems ( INT ; INT ) *PEBY
	MenuMacro$,	SD	ElSdMenuMacro ( INT, INT ; INT ) *PEBY
	MenuText$,	SD	ElSdMenuText ( INT, INT ; INT ) *PEBY
	MacroDesc$,	SD	ElSdMacroDesc ( HST ) *PEBY
	CountKeys,	INT	ElCountKeys ( ; INT ) *PEBY
	KeyCode,	INT	ElKeyCode ( INT ; INT ) *PEBY
	KeyMacro$,	SD	ElSdKeyMacro ( INT ; INT ) *PEBY

# NOTE: The rest of the user defined dialog box statements are ELT type
#	reserved words because of their need for ELK arguments.  See
#	token.c and interp/exec.c.

	OKButton,		ElOKButton ( INT, INT , INT, INT ) *PBEY
	CancelButton,		ElCancelButton ( INT, INT , INT, INT ) *PBEY
	Text,			ElDialogText ( INT, INT, INT, INT, HPSD ) *PBEY
	GroupBox,		ElGroupBox ( INT, INT, INT, INT, HPSD ) *PBEY
	OptionButton,		ElOptionButton ( INT, INT, INT, INT, HPSD ) *PBEY

	NextField,	; INT	FNextField () *MDEY
	PrevField,	; INT	FPrevField () *MDEY

	NextCell,	; INT	FNextCell () *MDY
	PrevCell,	; INT	FPrevCell () *MDY
	StartOfRow,	; INT	ElStartOfRow ( ; INT ) *MDEY
	EndOfRow,	; INT	ElEndOfRow ( ; INT ) *MDEY
	StartOfColumn,	; INT	ElStartOfCol ( ; INT ) *MDEY
	EndOfColumn,	; INT	ElEndOfCol ( ; INT ) *MDEY

	ExitWindows,		OurExitWindows () *P

	DisableAutoMacros,	ElDisableAutoMacros ( ; INT ) *PEBY

	EditSearchFound,  INT	FEditSearchFound () *PEBY

#ifdef DEBUG
        RefreshRate,    ; INT   ElRefreshRate ( ; INT ) *P

	MemAllocFail,   ; INT	ElMemAllocFail ( ; INT, INT ) *P
	WinAllocFail,	; INT	ElWinAllocFail ( ; INT, INT ) *P
        DbgErrorLevel,  ; INT   ElDbgErrorLevel ( ; INT ) *P

	TestOptions,		ElTestOptions ( HST ) *P
	ClearTestOptions,	ClearTestOptions () *P

        RareEvent,              ElRareEvent ( INT, INT, INT, INT, INT ) *P
        WriteDBS,               WriteDebugStateInfo () *P

        DoDebugTests,           ElDoDebugTests () *P


#### D E B U G G I N G  C O M M A N D S ####

        EnableTests, 	    	CmdEnableTests @dbgtests *P
    	DebugPrefs,		CmdDebugPrefs @dbgpref *P
	UseC1Versions,		CmdUseC1Versions @dbgusec1 *P
	UseC2Versions,		CmdUseC2Versions @dbgusec2 *P
	UseC3Versions,		CmdUseC3Versions @dbgusec3 *P
	UseC4Versions,		CmdUseC4Versions @dbgusec4 *P
    	DebugFailures,	    	CmdDebugFailures @dbgfail *P

	Scribble,		CmdScribble @dbgscrbl *P

	TestFunction,		CmdTestFunction @dbgfunc *P
	TestX,			CmdTestX *P

	CkDiskFile,		CmdCkDiskFile @dbgfile *P

	RareEvents,		CmdRareEvents @dbgrare *P
	ScanDialogs,		CmdScanDialogs *P,	   imiScanDlgs
	DbgMemory,		CmdDbgMemory @dbgmem *P
        EatMemory,              CmdEatMemory *P, bcmEatMemory
        FreeMemory,             CmdFreeMemory *P, bcmFreeMemory
        ScanErrors,             CmdScanErrors *P  
	FileCacheInfo,		CmdFileCacheInfo *P
	SelectionInfo,          CmdSelectionInfo *P
	FontInfo,		CmdFontInfo *P
	DoTests,		CmdDoTests *P
	HeapInfo,		CmdHeapInfo *P
	FliInfo,		CmdFliInfo *P

#debug routines...See debug1.c
        Debug1,                CmdDebug1 *P
        Debug2,                CmdDebug2 *P
        Debug3,                CmdDebug3 *P
        Debug4,                CmdDebug4 *P
        Debug5,                CmdDebug5 *P
        Debug6,                CmdDebug6 *P
        Debug7,                CmdDebug7 *P
        Debug9,                CmdDebug9 *P
        Debug0,                CmdDebug0 *P

# field debug routines
        SeeAllFieldCps,        CmdSeeAllCps *P


        RefreshSpeed,           CmdRefreshSpeed *P
        TestScc,                TestScc *P, bcmTestScc
        TestPlc,                TestPlc *P, bcmTestPlc
        TestPl,                 TestPl *P, bcmTestPl
        TestSttb,               TestSttb *P, bcmTestSttb
        GlobalCompact,          CmdGlobalCompact *P, bcmGlobalCompact
        EnumAllFonts,           CmdEnumAllFonts *P
        ListBitmapSizes,        CmdDumpBitmapSizes *P

        DumpKeymap,             CmdDumpKeymap *P

#endif DEBUG

#ifdef PROFILE
    	ToggleProfiler,	    	CmdToggleProfiler *P
#endif /* PROFILE */

### End of Command Table ###

