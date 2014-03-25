# Key bindings for Opus
#
# Allowable keys: (No other keys may be used!)
#
#	A	The A key
#	...
#	Z	The Z key
#	0	The 0 key
#	...
#	9	The 9 key
#	+	The +/= key (USA only!)
#	-	The -/_ key (USA only!)
#	[	The [/{ key (USA only!)
#	]	The [/} key (USA only!)
#
#	Back	Backspace key
#	BS	Backspace key
#	Esc	The Esc key
#	Return	The Enter key
#	Tab	The tab key
#
#	Up	The up arrow
#	Down	The down arrow
#	Left	The left arrow
#	Right	The right arrow
#	PgUp	The PgUp key
#	Prior	The PgUp key
#	PgDn	The PgDn key
#	Next	The PgDn key
#	Home	The Home key
#	End	The End key
#	Insert	The Ins key
#	Delete	The Del key
#	Clear	The 5 key on the number pad with NumLock off
#
#	NP1	The 1 key on the number pad with NumLock on
#	NP2	The 2 key on the number pad with NumLock on
#	...	(I'm sure you get the picture)
#	NP+	The + key on the number pad with NumLock on
#	NP-	The - key on the number pad with NumLock on
#	NP*	The * key on the number pad with NumLock on
#	NP.	The . key on the number pad with NumLock on
#
#	F1	The F1 key
#	...
#	F16	The last function key available
#	
#
# Prefixes:
#
#	S Shift+
#	C Ctrl+
#	A Alt+
#

KEYMAP vkmpInit
# NOTE: Most of the Alt+ keys are currently commented out because Windows
# handles them automaticaly.
        F1,             Help
        S F1,           HelpContext

        F2,             MoveText
        S F2,           CopyText
	C F2,		GrowFont
	CS F2,		ShrinkFont

	F3,		ExpandGlossary
	S F3,		ChangeCase
	C F3,		Spike
	CS F3,		UnSpike

        F4,             EditRepeat
        S F4,           RepeatSearch
        C F4,           DocClose
        A F4,           FileExit

        F5,             EditGoTo
        S F5,           GoBack
        C F5,           DocRestore
	SC F5,		InsertBookmark
	A F5,		AppRestore

        F6,             OtherPane
        S F6,           OtherPane
        C F6,           NextWindow
        SC F6,          PrevWindow
	A F6,		NextWindow
	SA F6,		PrevWindow

        F7,             UtilSpellSelection
        S F7,           UtilThesaurus
        C F7,           DocMove
	SC F7,		UpdateSource
#	A F7,		AppMove

        F8,             ExtendSelection
        S F8,           ShrinkSelection
        C F8,           DocSize
	CS F8,		ColumnSelect
#	A F8,		AppSize

        F9,             UpdateFields
        S F9,           ToggleFieldDisplay
        C F9,           InsertFieldChars
	SC F9,		UnLinkFields
	SA F9,		DoFieldClick
	A F9,		AppMinimize

	F10,		MenuMode
	S F10,		IconBarMode
	C F10,		DocMaximize
	SC F10,		RulerMode
	A F10,		AppMaximize

        F11,            NextField
        S F11,          PrevField
        C F11,          LockFields
        SC F11,         UnLockFields

	F12,		FileSaveAs
	S F12,		FileSave
	C F12,		FileOpen
	SC F12,		FilePrint

        C Clear,        EditSelectAll
        C NP5,        EditSelectAll

	Insert,         Overtype
        S Insert,       EditPaste
        C Insert,       EditCopy
        Delete,         EditClear
        S Delete,       EditCut
        C Delete,       DeleteWord
        C Back,         DeleteBackWord
        A Back,         EditUndo

#	C Right,	WordRight
#	A Right,	WordRight  	#Duplicate needed for CUA
#	C Left,		WordLeft
#	A Left,		WordLeft	#Duplicate needed for CUA

	A Return,	EditRepeat	#For Excel compatibility

	C Return,       InsertPageBreak
        SC Return,      InsertColumnBreak

	Esc,            Cancel

        C Space,        ResetChar

# The following three keys are no added at init time because their VK codes
# are not constants...  See FInitCommands() for details.
#        C +,            SubScript
#        SC +,           SuperScript
#        SC 8,           ShowAll

        C 1,            SpacePara1
        C 2,            SpacePara2
        C 5,            SpacePara15


        C B,            Bold
        C C,            CenterPara
        C D,            DoubleUnderline
        C E,            CloseUpPara
        C F,            Font
        C G,            UnHang
        C H,            Hidden
        C I,            Italic
        C J,            JustifyPara
        C K,            SmallCaps
        C L,            LeftPara
        C M,            UnIndent
        C N,            Indent
        C O,            OpenUpPara
        C P,            FontSize
        C R,            RightPara
        C S,            FormatStyles
        C T,           	HangingIndent 
        C U,            Underline
        C V,            CharColor
        C W,            WordUnderline
        C X,            ResetPara

# Outline commands made available even in normal mode.
	AS Left,	OutlinePromote
	AS Right,	OutlineDemote
	AS Up,		OutlineMoveUp
	AS Down,	OutlineMoveDown
	AS Clear,	NormalStyle

# PageView commands
        A Up,		PrevObject
        A Down,		NextObject

# Table commands
        A Home,         StartOfRow
        A End,          EndOfRow
        A PgUp,         StartOfColumn
        A PgDn,         EndOfColumn
        A Clear,        SelectTable

# Table commands with extend
        AS Home,        StartOfRow
        AS End,         EndOfRow
        AS PgUp,        StartOfColumn
        AS PgDn,        EndOfColumn

# Quick Fields
	AS D,		InsertDateField
	AS P,		InsertPageField
	AS T,		InsertTimeField

	AS C,		ClosePane
        AS L,           EditHeaderFooterLink
        AS R,           EditHeaderFooterLink

# Insert keys
        CS Space,       160     # chNonBreakSpace
        S Return,       11      # chCRJ
        C Tab,          9       # chTab

#ifdef DEBUG
        SC 1,           Debug1
        SC 2,           Debug2
        SC 3,           Debug3
        SC 4,           Debug4
        SC 5,           Debug5
        SC 6,           Debug6
        SC 7,           Debug7
# SC 8 reserved for ShowAll
        SC 9,           Debug9
        SC 0,           Debug0

        SC E,           EatMemory
        SC F,           FreeMemory
        SC A,           DebugFailures
        SC S,           SeeAllFieldCps
#endif DEBUG

#ifdef PROFILE
    	SC P,	    	ToggleProfiler
#endif /* PROFILE */


### End of keys.cmd ###
