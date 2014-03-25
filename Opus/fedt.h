
#ifndef RC_INVOKED

/*----------------------------------------------------------------------------
|    FEDT structure
|
|    Data structure defining a FEDT formula edit control.
|
|    Fields:
|        hwnd        window handle of the fedt control.
|        hdc         device context for the window.
|        fSingleLine single line editcontrol
|        fPointSel   if control supports point mode
|        fOverType   text is inserted in over-type mode
|        fMacCaret   Caret is at mac end-point of the selection
|        fShowSel    selection is currently visible, and the
|                     caret is blinking
|        fCaretOn    caret has been created for this window
|        fTypingUndo typing undo is being created
|        fBadBrks    ran out of memory trying to allocate line
|                     break array
|        cchText     number of characters in the text
|        hszText     text in the fedt control
|        ichMicSel   start of current selection
|        ichMacSel   end of current selection
|        liCaret     line the caret is on
|        ptCaret     position of the blinking caret
|        hrgnSel     region describing the inverted part of the
|                     current selection
|        rcFmt       formatting rectangle
|        pfnNextBrk  routine to find next word break position
|        pfnPrevBrk  routine to find previous word break position
|        liMac       number of lines of text displayed in the
|                     control
|        liMax       size of mpliichBrk array
|        ichBrk0     dummy line break position for 0th line
|        mpliichBrk  line break array
----------------------------------------------------------------------------*/


#ifdef CC
#define SEL2 sel2
#define UNSEL unsel
#else
#define SEL2
#define UNSEL
#endif

typedef struct _fedtsel
		{
		BYTE liCaret;
		BYTE fMacCaret;
		int rgichSel[2];
		}
	FEDTSEL;

typedef struct _fedt
		{
		HWND hwnd;
		HWND hwndParent;
		HDC hdc;

		unsigned fSingleLine:1,
			fPointSel:1,
			fOverType:1,
			fShowSel:1,
			fCaretOn:1,
			:1,
			fBadBrks:1,
			:4,
			fCombo:1,
			unused1:4;

		/* text in the control */
		int cchText;
		char **hszText;

		/* selection position */
		union
			{
			struct
				{
				BYTE liCaret;
				BYTE fMacCaret;
				int ichMicSel;
				int ichMacSel;
				} SEL2;
			FEDTSEL sel;
			} UNSEL;

		/* selection as shown on physical display */
		struct PT ptCaret;
		HRGN hrgnSel;

		/* formatting area */
		struct RC rcFmt;
		struct RC rcView;

		/* line breaks */
		BYTE liMac;
		BYTE liMax;
		int ichBrk0;    /* WARNING!! - must preceed mpliichBrk */
		int mpliichBrk[1];
		}
	FEDT;


#define CbFedt(liMac) (sizeof(FEDT)-sizeof(int)+(liMac)*sizeof(int))

#define HfedtFromHwnd(hwnd) ((FEDT **)GetWindowWord(hwnd, 0))
#define HwndListBoxFromFedt(hfedt)  ((HWND)GetWindowWord((*hfedt)->hwndParent,IDWHLBOX))

#define PchFromHsz(hsz) ((char *)(*(hsz)))

/* Fedt notification codes - MUST BE COMPATIBLE WITH WINDOWS EDIT CONTROL
	EN_* NOTIFICATION CODES!! */
#define FN_SETFOCUS 0x0100
#define FN_KILLFOCUS 0x0200
#define FN_CHANGE 0x0300

/* out of memory notification codes */
#define FN_ERRSPACE 0x0500
#define FN_OOMTEXT 0x00
#define FN_OOMLINEBRKS 0x01
#define FN_OOMCOPY 0x02
/* return values from FN_ERRSPACE */
#define FN_OOMALERT 0x00
#define FN_OOMABORT 0x01
#define FN_OOMRETRY 0x02
#define FN_OOMIGNORE 0x03

/* miscellaneous notification codes */
#define FN_VERIFYREPLACE 0x0700

#endif /* RC_INVOKED */


/* Fedt control styles in CreateWindow call */
#define FS_MULTILINE 0x0004L

/* extra space in window */

#define CBWEFEDT    sizeof(char **)
