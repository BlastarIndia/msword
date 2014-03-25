# Menu bindings for Opus


MENUBAR MW_MENU
        MENU &File
                &New...,                FileNew
                &Open...,               FileOpen
                &Close,                 FileClose
                &Save,                  FileSave
                Save &As...,            FileSaveAs
                Sav&e All,              FileSaveAll
                &Find...,               FileFind
                SEPARATOR
                &Print...,              FilePrint
                Print Pre&view,         FilePrintPreview
                Print &Merge...,        FilePrintMerge
                P&rinter Setup...,      FilePrinterSetup
                SEPARATOR
                E&xit,                  FileExit
                FILECACHE

        MENU &Edit
                &Undo,                EditUndo
		&Repeat,		EditRepeat
                Cu&t,                 EditCut
                &Copy,                EditCopy
                &Paste,               EditPaste
                Paste &Link...,       EditPasteLink
                SEPARATOR
                &Search...,           EditSearch
                R&eplace...,          EditReplace
                &Go To...,            EditGoTo
                SEPARATOR
                &Header/Footer...,    EditHeaderFooter
                Summary &Info...,     EditSummaryInfo
                Gl&ossary...,         EditGlossary
                T&able...,            EditTable

        MENU &View
                &Outline,             ViewOutline
                &Draft,               ViewDraft
                &Page,                ViewPage
                SEPARATOR
                Ri&bbon,              ViewRibbon
                &Ruler,               ViewRuler
                &Status Bar,          ViewStatusBar
                &Footnotes,           ViewFootnotes
                &Annotations,         ViewAnnotations
                SEPARATOR
                Field &Codes,         ViewFieldCodes
                Pr&eferences...,      ViewPreferences
                Short &Menus,         ViewShortMenus

        MENU &Insert
                &Break...,            InsertBreak
                Foot&note...,         InsertFootnote
                &File...,             InsertFile
                Book&mark...,         InsertBookmark
                Page N&umbers...,     InsertPageNumbers
                &Table...,            InsertTable
                SEPARATOR
                &Annotation,          InsertAnnotation
                &Picture...,          InsertPicture
                Fiel&d...,            InsertField
                Index &Entry...,      InsertIndexEntry
                &Index...,            InsertIndex
                Table of &Contents..., InsertTableOfContents

        MENU Forma&t
                &Character...,        FormatCharacter
                &Paragraph...,        FormatParagraph
                &Section...,          FormatSection
                &Document...,         FormatDocument
                SEPARATOR
                &Tabs...,             FormatTabs
                St&yles...,           FormatStyles
                P&osition...,         FormatPosition
                SEPARATOR
                De&fine Styles...,    FormatDefineStyles
                Pictu&re...,          FormatPicture
                T&able...,            FormatTable

        MENU &Utilities
                &Spelling...,           UtilSpelling
                &Thesaurus...,          UtilThesaurus
                &Hyphenate...,          UtilHyphenate
                SEPARATOR
                &Renumber...,           UtilRenumber
                Revision &Marks...,     UtilRevisionMarks
                Compare &Versions...,   UtilCompareVersions
                S&ort...,               UtilSort
                &Calculate,             UtilCalculate
                Re&paginate Now,        UtilRepaginateNow
                C&ustomize...,          UtilCustomize

        MENU &Macro
                Re&cord...,             MacroRecord
                &Run...,                MacroRun
                &Edit...,               MacroEdit
                Assign to &Key...,      MacroAssignToKey
                Assign to &Menu...,     MacroAssignToMenu

        MENU &Window
                &New Window,            WindowNewWindow
                &Arrange All,           WindowArrangeAll
                WNDCACHE


#ifdef DEBUG
        MENU " "
                Enable &Tests...,        EnableTests
		Use C Versions &1...,	 UseC1Versions
		Use C Versions &2...,	 UseC2Versions
		Use C Versions &3...,	 UseC3Versions
		Use C Versions &4...,	 UseC4Versions
    	    	&Preferences...,    	 DebugPrefs
                Enable Scri&bbles...,    Scribble
                Test F&unction...,       TestFunction
                Test &X,                 TestX
                &Rare Events...,         RareEvents
                &Enum All Fonts,         EnumAllFonts
                Bitmap &Sizes,           ListBitmapSizes
    	    	&Failures...,	    	 DebugFailures
                Set cw&Heap Avail...,    DbgMemory
		Check &Disk File...,	 CkDiskFile
                Sca&n Errors,            ScanErrors
		File &Cache Info,	 FileCacheInfo
                Selection &Info,         SelectionInfo
		Font Infor&mation, 	 FontInfo
		D&o Tests,	 	 DoTests
		He&ap Information,	 HeapInfo
		F&li Information,	 FliInfo
#endif


#ifdef MKTGPRVW
        MENU "\a   &Help"
#else
        MENU "\a&Help"
#endif
                &Index,                 HelpIndex
                SEPARATOR
                &Keyboard,              HelpKeyboard
                Active &Window,         HelpActiveWindow
                SEPARATOR
                &Tutorial,              HelpTutorial
                Using &Help,            HelpUsingHelp
                SEPARATOR
		&About...,		HelpAbout


# Full No-Document Menu
MENUBAR MW_MINMENU
        MENU &File
                &New...,                FileNew
                &Open...,               FileOpen
                &Find...,               FileFind
                SEPARATOR
                Re&cord Macro...,       MacroRecord
                &Run Macro...,          MacroRun
                SEPARATOR
                E&xit,                  FileExit
                FILECACHE

#ifdef DEBUG
        MENU "  "
                Enable &Tests...,        EnableTests
		Use C Versions &1...,	 UseC1Versions
		Use C Versions &2...,	 UseC2Versions
		Use C Versions &3...,	 UseC3Versions
		Use C Versions &4...,	 UseC4Versions
    	    	&Preferences...,    	 DebugPrefs
                Enable Scri&bbles...,    Scribble
                Test F&unction...,       TestFunction
                Test &X,                 TestX
                &Rare Events...,         RareEvents
                &Enum All Fonts,         EnumAllFonts
                Bitmap &Sizes,           ListBitmapSizes
    	    	&Failures...,	    	 DebugFailures
                Set cw&Heap Avail...,    DbgMemory
		Check &Disk File...,	 CkDiskFile
                Sca&n Errors,            ScanErrors
		File &Cache Info,	 FileCacheInfo
                Selection &Info,         SelectionInfo
		Font Infor&mation, 	 FontInfo
		D&o Tests,	 	 DoTests
		He&ap Information,	 HeapInfo
#endif

#ifdef MKTGPRVW
        MENU "\a   &Help"
#else
        MENU "\a&Help"
#endif
                &Index,                 HelpIndex
                SEPARATOR
                &Keyboard,              HelpKeyboard
                Active &Window,         HelpActiveWindow
                SEPARATOR
                &Tutorial,              HelpTutorial
                Using &Help,            HelpUsingHelp
                SEPARATOR
		&About...,		HelpAbout

MENUBAR MW_SHORTMINMENU
        MENU &File
                &New...,                FileNew
                &Open...,               FileOpen
                SEPARATOR
                E&xit,                  FileExit
                FILECACHE

#ifdef DEBUG
        MENU " "
                Enable &Tests...,        EnableTests
		Use C Versions &1...,	 UseC1Versions
		Use C Versions &2...,	 UseC2Versions
		Use C Versions &3...,	 UseC3Versions
		Use C Versions &4...,	 UseC4Versions
    	    	&Preferences...,    	 DebugPrefs
                Enable Scri&bbles...,    Scribble
                Test F&unction...,       TestFunction
                Test &X,                 TestX
                &Rare Events...,         RareEvents
                &Enum All Fonts,         EnumAllFonts
                Bitmap &Sizes,           ListBitmapSizes
    	    	&Failures...,	    	 DebugFailures
                Set cw&Heap Avail...,    DbgMemory
		Check &Disk File...,	 CkDiskFile
                Run &Macro...,           MacroRun
                Sca&n Errors,            ScanErrors
		File &Cache Info,	 FileCacheInfo
                Selection &Info,         SelectionInfo
		Font Infor&mation, 	 FontInfo
		D&o Tests,		 DoTests
		He&ap Information,	 HeapInfo
#endif

#ifdef MKTGPRVW
        MENU "\a   &Help"
#else
        MENU "\a&Help"
#endif
                &Index,                 HelpIndex
                SEPARATOR
                &Keyboard,              HelpKeyboard
                Active &Window,         HelpActiveWindow
                SEPARATOR
                &Tutorial,              HelpTutorial
                Using &Help,            HelpUsingHelp
                SEPARATOR
		&About...,		HelpAbout

MENUBAR MW_SHORTMENU
        MENU &File
                &New...,                FileNew
                &Open...,               FileOpen
                &Close,                 FileClose
                &Save,                  FileSave
                Save &As...,            FileSaveAs
                SEPARATOR
                &Print...,              FilePrint
                Print Pre&view,         FilePrintPreview
                P&rinter Setup...,      FilePrinterSetup
                SEPARATOR
                E&xit,                  FileExit
                FILECACHE

        MENU &Edit
                &Undo,                  EditUndo
		&Repeat,		EditRepeat
                Cu&t,                   EditCut
                &Copy,                  EditCopy
                &Paste,                 EditPaste
                SEPARATOR
                &Search...,             EditSearch
                R&eplace...,            EditReplace
                &Go To...,              EditGoTo
                SEPARATOR
                &Header/Footer...,      EditHeaderFooter
                Summary &Info...,       EditSummaryInfo
                T&able...,              EditTable

        MENU &View
                &Outline,               ViewOutline
                &Draft,                 ViewDraft
                &Page,                  ViewPage
                SEPARATOR
                Ri&bbon,                ViewRibbon
                &Ruler,                 ViewRuler
                &Footnotes,             ViewFootnotes
                Full &Menus,            ViewFullMenus

        MENU &Insert
                Page &Break,          InsertPageBreak
                Foot&note...,         InsertFootnote
                &File...,             InsertFile
                Book&mark...,         InsertBookmark
                Page N&umbers...,     InsertPageNumbers
                &Table...,            InsertTable

        MENU Forma&t
                &Character...,          FormatCharacter
                &Paragraph...,          FormatParagraph
                &Document...,           FormatDocument
                SEPARATOR
                &Tabs...,               FormatTabs
                T&able...,              FormatTable

        MENU &Utilities
                &Spelling...,           UtilSpelling
                &Thesaurus...,          UtilThesaurus
                &Calculate,             UtilCalculate

        MENU &Window
                &New Window,            WindowNewWindow
                &Arrange All,           WindowArrangeAll
                WNDCACHE


#ifdef DEBUG
        MENU "  "
                Enable &Tests...,        EnableTests
		Use C Versions &1...,	 UseC1Versions
		Use C Versions &2...,	 UseC2Versions
		Use C Versions &3...,	 UseC3Versions
		Use C Versions &4...,	 UseC4Versions
    	    	&Preferences...,    	 DebugPrefs
                Enable Scri&bbles...,    Scribble
                Test F&unction...,       TestFunction
                Test &X,                 TestX
                &Rare Events...,         RareEvents
                &Enum All Fonts,         EnumAllFonts
                Bitmap &Sizes,           ListBitmapSizes
    	    	&Failures...,	    	 DebugFailures
                Set cw&Heap Avail...,    DbgMemory
		Check &Disk File...,	 CkDiskFile
                Run &Macro...,           MacroRun
                Sca&n Errors,            ScanErrors
		File &Cache Info,	 FileCacheInfo
                Selection &Info,         SelectionInfo
		Font Infor&mation, 	 FontInfo
		D&o Tests,	 	 DoTests
		He&ap Information,	 HeapInfo
#endif

#ifdef MKTGPRVW
        MENU "\a   &Help"
#else
        MENU "\a&Help"
#endif
                &Index,                 HelpIndex
                SEPARATOR
                &Keyboard,              HelpKeyboard
                Active &Window,         HelpActiveWindow
                SEPARATOR
                &Tutorial,              HelpTutorial
                Using &Help,            HelpUsingHelp
                SEPARATOR
		&About...,		HelpAbout

### End of menus.cmd ###

