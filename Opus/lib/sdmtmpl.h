/*
	sdmtmpl.h : SDM Template info
	NOTE : no application code should rely on the contents of this file !!
*/

/*****************************************************************************/
/* TMTs */

typedef	WORD	TMT;		/* TM types */

#define tmtEnd			0
#define tmtNormalMin		1

/* Items with a title */
#define tmtStaticText		1
#define tmtPushButton		2
#define tmtCheckBox		3
#define tmtRadioButton		4
#define tmtGroupBox		5
#define	tmtWithTitleMax		6

/* Items without a title */
#define tmtEdit			6
#define tmtFormattedText	7
#define tmtListBox		8
#define	tmtDropList		9

#define tmtBitmap		10
#define tmtGeneralPicture	11

#define	tmtNormalIgnore		12	/* ignore this item */

#define tmtUserMin			40	/* Sdm never sees this range */
#define tmtUserMax			50

#define tmtNormalMax		50

#define tmtExtensionMin		50	/* Extensions are still special */
#define tmtConditional		50
#define tmtExtDummyText		51
#define tmtExtItemProc		52
#define tmtExt1			53
#define tmtExt2			54
#define tmtExt3			55
#define tmtExtEdit		56

#define tmtExtensionMax		64

#define tmtMax			64	/* 6 bits */



///////////////////////////////////////////////////////////////////////////////
// ALT : Text alignment for StaticText and EditItems.
typedef WORD	ALT;

#define altLeft			((ALT)0)
#define altCenter		((ALT)1)
#define altRight		((ALT)2)



///////////////////////////////////////////////////////////////////////////////
// FOST : StaticText options.
typedef	WORD	FOST;
#define fostNoAccel		((FOST)1)	// No acclerator.
#define fostMultiLine		((FOST)2)



///////////////////////////////////////////////////////////////////////////////
// FOEI : EditItem options.
typedef	WORD	FOEI;
#define foeiCharValidated	((FOEI)1)
#define foeiMultiLine		((FOEI)2)
#define foeiVerticalScroll	((FOEI)4)




///////////////////////////////////////////////////////////////////////////////
// FOLB : ListBox options.
typedef WORD	FOLB;

#define	folbComboAtomic		((FOLB)0x0001)	// Atomic Combo ListBox.
#define	folbString		((FOLB)0x0002)	// ListBox returns string.
#define	folbSorted		((FOLB)0x0004)	// Sorted ListBox.
#define	folbMultiSelectable	((FOLB)0x0008)	// MutliSelectable ListBox.
#define	folbNoScrollBar		((FOLB)0x0010)	// No vertical scroll bar.
#define	folbDropDownSibling	((FOLB)0x0020)	// Sibling DropDown.
#define	folbDropAtomic		((FOLB)0x0040)	// Atomic Combo DropDown
						// ListBox.
#define folbSiblingAtomic	((FOLB)0x0080)	// Atomic Combo Sibling
						// DropDown ListBox.
#define folbExtended		((FOLB)0x0100)	// Send tmmCreate message.



/*****************************************************************************/
/* -- defined Dialog Item */

typedef struct _tm
	{
	BITS	tmt:6;		/* item type */
	BIT	fThinBdr:1;
	BIT	fAction:1;
	BITS	bitSlot:8;

#ifdef	CC
	WORD	wSlot;		/* based pointer or wParam */
#else	//CC
#ifdef	SDMTMW_PRIVATE
	WORD	wSlot;		/* based pointer or wParam */
#else	//SDMTMW_PRIVATE
	char	wSlot[];	/* based pointer or wParam */
#endif	//!SDMTMW_PRIVATE
#endif	//!CC

	LONG	l;		/* compact rectangle or PFN */
	} TM;


/* Compact Rectangle */
typedef struct _crc
	{
#ifdef	SDM_ENV_MAC
	BYTE	dy, dx, y, x;		/* order critical */
#else	/*SDM_ENV_MAC*/
	BYTE	x, y, dx, dy;		/* order critical */
#endif	/*SDM_ENV_MAC*/
	} CRC;		/* Compact Rectangle */



/*****************************************************************************/
/* BDR */
typedef	WORD	BDR;

/* Note: the order and contiguity of the following 5 #defines is assumed. */
/*	Do not change.  PRB */
#define bdrNone		((BDR) 0x0000)
#define bdrThin		((BDR) 0x0001)
#define bdrThick	((BDR) 0x0002)
#define bdrCaption	((BDR) 0x0003)	/* dialog has caption */
#define bdrSysMenu	((BDR) 0x0004)	/* system menu - implies caption. */
#define bdrMask		((BDR) 0x0007)	/* actual border bits. */
#define bdrAutoPosX	((BDR) 0x0008)	/* Auto position along x-axis. */
#define bdrAutoPosY	((BDR) 0x0010)	/* Auto position along y-axis. */



/*****************************************************************************/

/* dialog template (or template header) */
typedef struct _dlt
	{
	REC	rec;		/* rectangle of dialog */
	WORD	hid;		/* Help ID. */
	TMC	tmcSelInit;	/* Item with initial selection */
	PFN_DIALOG pfnDlg;	/* DialogProc.  NULL for none. */
	WORD	ctmBase;	/* # of base TMs */
	BDR	bdr;		/* Border bits. */

	/* title string : based pointer */
#ifdef	CC
	WORD	bpTitle;			/* based pointer to title */
#else	/*CC*/
#ifdef	SDMTMW_PRIVATE
	WORD	bpTitle;			/* based pointer to title */
#else	//SDMTMW_PRIVATE
	char	bpTitle[];
#endif	//!SDMTMW_PRIVATE
#endif	//!CC

#ifndef CC
	TM	rgtm[];			/* variable length for CS */
#else	/*!CC*/
#ifdef	SDMTMW_PRIVATE
	TM	rgtm[1];		/* array starts here */
#endif	//SDMTMW_PRIVATE
#endif	//CC
	} DLT;	/* Dialog Template */

typedef DLT ** HDLT;		// A near handle. 

/*****************************************************************************/
/* misc equates */

#define	bpStringFromCab		((WORD) 1)

/*****************************************************************************/
