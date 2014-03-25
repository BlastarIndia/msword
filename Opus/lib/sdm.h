// sdm.h : SDM Main PUBLIC include file.				     

#ifndef	SDM_INCLUDED	// Entire file.
#define	SDM_INCLUDED

#ifndef	SDM_ENV_WIN_PM
#ifdef	SDM_ENV_WIN
#define	SDM_ENV_WIN_PM
#endif	//SDM_ENV_WIN

#ifdef	SDM_ENV_PM
#define	SDM_ENV_WIN_PM
#endif	//SDM_ENV_PM
#endif	//!SDM_ENV_WIN_PM



///////////////////////////////////////////////////////////////////////////////
// Standard types and values.						     

#ifndef CSTD_H
typedef	unsigned	BITS;		// Generic bits. 
typedef	unsigned	BIT;		// BOOL bit. 
typedef	char far *	LSZ;
#endif	//!CSTD_H

#ifdef CC
typedef struct _rec
	{
	int	x;
	int	y;
	int	dx;
	int	dy;
	} REC;				// Rectangle. 
#else	//CC
typedef union _pnt
	{
	struct
		{
		int	x;
		int	y;
		};
	int	rgxy[2];
	} PNT;

typedef union _rec
	{
	struct
		{
		int	x;
		int	y;
		int	dx;
		int	dy;
		};
	struct
		{
		PNT	pntXY;
		PNT	pntDxDy;
		};
	PNT	rgpnt[2];
	} REC;
#endif	//!CC



///////////////////////////////////////////////////////////////////////////////
// Text Selection.							     
typedef DWORD	TXS;

#define	ichLimLast	0x7fff		// Go to end of edit item. 
#define TxsOfFirstLim(f, l)	((TXS)MAKELONG((f), (l)))
#define TxsAll()		TxsOfFirstLim(0, ichLimLast)
#define IchFirstOfTxs(txs)	LOWORD(txs)
#define IchLimOfTxs(txs)	HIWORD(txs)



///////////////////////////////////////////////////////////////////////////////
// Base dialog types.							     

typedef char *	STR;			// String (SZ or STZ). 
#define strNull	szNull			// (Use "char *" instead of SZ since 
					// not all apps. use cstd.h.)	     
#ifdef	SDM_MULTI_SB

#ifndef	OPTR
#define	OPTR			huge	// Pointer modifier (near or huge).
#endif	//OPTR

typedef LONG	HDLG;			// A huge handle. 
#define	HdlgOfWNewWOld(wNew, wOld)	MAKELONG((wNew), (wOld))
#define SbDlgOfHdlg(hdlg)		SbOfHp(hdlg)
#else	//SDM_MULTI_SB

#ifndef	OPTR
#define	OPTR				// Pointer modifier (near or huge).
#endif	//OPTR

typedef WORD	HDLG;		// A near handle. 
#define	HdlgOfWNewWOld(wNew, wOld)	(wOld)
#endif	//!SDM_MULI_SB

#define hdlgNull	((HDLG) NULL)
#define hdlgCabError	((HDLG) -1)
#define hdltNull	((struct _dlt **) NULL)



///////////////////////////////////////////////////////////////////////////////
// TMC.									     

typedef WORD	TMC;			// Item codes. 

// Standard item codes (tmc). 
#define	tmcNull		((TMC)0)
#define tmcError	((TMC)-1)
#define	tmcOK		((TMC)1)
#define	tmcOk		tmcOK
#define	tmcCancel	((TMC)2)
#define	tmcSysMin	((TMC)0x10)
#define	tmcSysMax	((TMC)0x400)
#define	ftmcGrouped	0x8000		// OR'd to specify whole group as 
					// opposed to first item. 
#define	tmcUserMin	tmcSysMax
#define	tmcUserMax	((TMC)ftmcGrouped)



///////////////////////////////////////////////////////////////////////////////
// Tmc macros. 

#define	TmcValue(tmc)		((tmc) & ~ftmcGrouped)
#define FIsGroupTmc(tmc)	((tmc & ftmcGrouped) != fFalse)



///////////////////////////////////////////////////////////////////////////////
// DLM.									     

typedef WORD	DLM;			// Dialog proc / item proc messages. 

// Global Messages. 
#define	dlmInit 	((DLM)0x0001)	// Do custom initialization. 
#define	dlmPlayBackInit ((DLM)0x0002)	// Do custom noninteractive init. 
#define	dlmTerm 	((DLM)0x0003)	// Termination for one of many 
					// reasons. 
#define	dlmExit		((DLM)0x0004)	// Dialog is about to be blown away. 

// Item messages. 
#define	dlmChange	((DLM)0x0005)	// Edit control may have changed. 
#define	dlmClick	((DLM)0x0006)	// Item was clicked. 
#define	dlmDblClk	((DLM)0x0007)	// Double click in listbox/radio. 
#define dlmClickDisabled ((DLM)0x0008)	// Noninteractive Dialog-sessions
					// only, the button passed to
					// FSetNoninteractive() has been 
					// disabled by a call to 
					// EnableNoninteractiveTmc() at dlmInit
					// time. 

// Rare Item Messages. 
#define dlmDirFailed	((DLM)0x0009)	// Directory ListBox I/O failure.
#define	dlmKey		((DLM)0x000a)	// Any untrapped key. 
#define	dlmSetItmFocus	((DLM)0x000b)	// Item gets focus. 
#define	dlmKillItmFocus	((DLM)0x000c)	// Item loses focus. 

// Rare Dialog Proc Messages. 
#define	dlmSetDlgFocus	((DLM)0x000d)	// Dialog gets focus. 
#define	dlmKillDlgFocus	((DLM)0x000e)	// Dialog loses focus. 
#define dlmAdjustPos	((DLM)0x000f)	// Adjust item's rec?
#define	dlmTabOut	((DLM)0x0010)	// Tab out of dialog? 
#define	dlmIdle		((DLM)0x0011)	// Idle for modal dialogs. 
#define dlmDlgClick	((DLM)0x0012)	// Click in dialog's window. 
#define dlmDlgDblClick	((DLM)0x0013)	// Double click in dialog's window. 
#define dlmShowCaret	((DLM)0x0014)	// Show a caret.
#define dlmHideCaret	((DLM)0x0015)	// Hide a caret.

#define	dlmUserMin	((DLM)0x0016)	// For App use. 



///////////////////////////////////////////////////////////////////////////////
// TMM Messages.							     

typedef WORD	TMM;			// Control proc messages. 
#define	tmmUserMin	((TMM)0x0010)	// For user extensions. 

// ListboxProc messages. 
#define	tmmCount	((TMM)0x0002)	// Return # of items. 
#define	tmmText 	((TMM)0x0003)	// Return text of n'th item. 
					// (always sent sequentially). 
#define	tmmEditText	((TMM)0x0004)	// Like tmmText but send randomly. 
#define	tmmDirInit	((TMM)0x0005)	// Return if DIR fill should be done
#define	tmmDirMatch	((TMM)0x0006)	// return if DIR entry should be added
#define	cszUnknown	((WORD) -1)	// Return to tmmCount if unknown. 

// Private user extension tmm messages used by directory listbox
// item and control procedures.  These don't really belong here, but
// the app's need access to them if they are subclassing the directory
// listbox control procedure (such as PowerPoint).
#define	tmmFill		tmmUserMin
#define	tmmWParam	tmmUserMin + 1

// ParseProc messages. 
#define	tmmFormat	((TMM)0x0001)	// Format data. 
#define	tmmParse	((TMM)0x0002)	// Parse data. 

// Render Procs 
#define	tmmRender	((TMM)0x0001)	// Repaint entire item. 
#define tmmNewState	((TMM)0x0002)	// State has changed. 
#define tmmRepaint	((TMM)0x0003)	// Repaint everything but text.

// GeneralPictures and ListboxProc 
#define	tmmCreate	((TMM)0x0001)	// Create windows and such. 

// General Pictures 
#define	tmmPaint	((TMM)0x0002)	// Paint yourself. 
#define tmmSetFocus	((TMM)0x0003)	// Show yourself as active.
#define tmmInput	((TMM)0x0004)	// User input received. 
#define	tmmClear	((TMM)0x0005)	// Clear selection. 
#define	tmmCopy		((TMM)0x0006)	// Copy selection to clipboard. 
#define	tmmCut		((TMM)0x0007)	// Cut selection to clipboard. 
#define	tmmPaste	((TMM)0x0008)	// Paste from clipboard. 
#define tmmTabStop	((TMM)0x0009)	// Query if item is tabstop 
#define tmmKillFocus	((TMM)0x000a)	// Show yourself as unactive.
#define tmmDestroy	((TMM)0x000b)	// Inform item is being destroyed.


typedef struct
	{
	VOID **	hlbx;
	WORD	cEntryVisible;		// Number of entries visible in list
	int	dySmall;		// Height of small list
	} LBM;

typedef	struct
	{
	char *	szTmpl;
	char *	szName;
	char *	szType;
	} DRM_SDM;

///////////////////////////////////////////////////////////////////////////////
// Multi-Type string support.						     

#define cchStrMax	255		// Maximum size of SDM string. 

// Size of buffers. 
#ifdef	SDM_STR_SZ
#define cchStrBufMax	256		// Extra byte for zero term. 
#define SetPrefixStr(str)
#define	SzOfStr(str)			(str)
#endif	//SDM_STR_SZ

#ifdef	SDM_STR_STZ
#define cchStrBufMax	257		// Length prefix + zero term. 
#define	SetPrefixStr(str)		(str)[0] = (char)CchLenSz((str) + 1)
#define	SzOfStr(str)			((str) + 1)
#endif	//SDM_STR_STZ



///////////////////////////////////////////////////////////////////////////////
// FTMS - Item States.							     

typedef WORD	FTMS;

#define ftmsNull	((FTMS)0x0000)	// None of the following. 
#define ftmsFocus	((FTMS)0x0001)	// Item has the focus. 
#define ftmsDefault	((FTMS)0x0002)	// Item is default pushbutton. 
#define ftmsPushed	((FTMS)0x0004)	// Button is "depressed". 
#define ftmsInvert	ftmsPushed	// Old name for above.
#define ftmsOn		((FTMS)0x0008)	// Item is "on". 
#define ftmsNinch	((FTMS)0x0010)	// Tri-state only - third state. 

// Note: the contiguity and ordering of the following 4 is assumed. 
#define ftmsEnable	((FTMS)0x0020)	// Item is enabled. 
#define ftmsVisible	((FTMS)0x0040)	// Item is invisible. 
#define ftmsMember	((FTMS)0x0080)	// Member of current subdialog. 



///////////////////////////////////////////////////////////////////////////////
// TMV - Item Values.							     

typedef WORD	TMV;

#define	tmvNoType	((TMV)0)
#define	tmvWord		((TMV)1)
#define	tmvFixed	((TMV)2)
#define	tmvString	((TMV)3)
#define tmvRgw		((TMV)4)


///////////////////////////////////////////////////////////////////////////////
// dlmChange notifications

typedef	WORD	FTMN;

#define	ftmnNull		((FTMN)0x0000)
#define	ftmnCombo		((FTMN)0x0001)
#define	ftmnCharValidated	((FTMN)0x0002)
#define	ftmnKillFocus		((FTMN)0x0004)
#define	ftmnCabVal		((FTMN)0x0008)


///////////////////////////////////////////////////////////////////////////////
// CAB info.								     

typedef void **	HCAB;			// CAB is an abstract data type. 
typedef void *	PCAB;			// Pointer to arbitrary CAB.
typedef WORD	CABI;			// CAB initializer.
#define	Cabi(cwTotal, cwHandle)		((cwTotal) + (cwHandle << 8))
#define hcabNull	((HCAB)NULL)
#define hcabNotFilled	((HCAB)-1)	// Could be returned by HcabFromDlg().

// Command argument block header. 
typedef struct _cabh
	{
	WORD	cwSimple;		// Total size of CAB less CABH. 
	WORD	cwHandle;		// # of handles. 
	} CABH;

// Minimum CAB size : header + SAB. 
#define	cwCabMin	((sizeof(CABH) + sizeof(WORD)) / sizeof(WORD))
#define	cbCabOverhead	(cwCabMin * 2)

// Iag macro - returns iag corresponding to field fld in application structure
//	str. 
#define Iag(str, fld)	\
	((WORD)((int)&(((str *)0)->fld) / sizeof(int) - cwCabMin))

// Macro to get void pointer to general CAB arg given offset.
#define	PvParseArg(hObj, bArg) ((VOID *) (*((WORD *)(hObj)) + (bArg)))

// Macro to get void pointer to general CAB arg given iag.
#define PvFromCabIag(hcab, iag)	\
	((VOID *)(*((WORD **)(hcab)) + cwCabMin + (iag)))


// Cab string/data pointers. 
typedef	char OPTR *	STZ_CAB;
typedef char OPTR *	SZ_CAB;
typedef char OPTR *	RGB_CAB;
typedef char OPTR *	ST_CAB;




///////////////////////////////////////////////////////////////////////////////
// Other special values.						     

// Special ninch (No Input, No CHange) value. 
#define	wNinch		(-32767)	// Ints. 
#define	uNinch		(0xffff)	// Unsigned. 
#define	iszNinchList	uNinch		// Listboxes. 
#define	uNinchList	uNinch		// Other name. 
#define	uNinchRadio	uNinch		// RadioGroups. 
#define	uNinchCheck	uNinch		// CheckBoxes 
#define	wNinchCheck	uNinchCheck	// Old name. 

// Special parse error values. 
#define	wError		(-32766)	// Ints. 
#define	uError		(0xfffe)	// Unsigneds. 

// Default no help. 
#define	hidDlgNull	0		// For no help. 


///////////////////////////////////////////////////////////////////////////////
// Dialog Initialization.						     

typedef	DWORD	FDLG;
#define fdlgNull		((FDLG)0x00000000)
#define fdlgModal		((FDLG)0x00000001)	// Create Modal.
#define fdlgInvisible		((FDLG)0x00000002)	// Start invisible.
#define	fdlgEnableTabOut	((FDLG)0x00000004)	// Send dlmTabOut.
#define fdlgPopup		((FDLG)0x00000008)	// Popup dialog.
#define fdlgNoSaveBits		((FDLG)0x00000010)	// Dont save bits.
#define fdlgClipChildren	((FDLG)0x00000020)	// Clip controls.

#ifdef	SDM_FEDT
#define fdlgFedt		((FDLG)0x00000040)	// EditItem is FEDT.
#endif	//SDM_FEDT

#define fdlgAdjustPos		((FDLG)0x00000080)	// Adjust item rec's.
#define fdlgOwnDC		((FDLG)0x00000100)	// Dialog owns a DC.

#define FTestFdlg(fdlg, fdlgTest)	(((fdlg) & (fdlgTest)) != fdlgNull)
#define ClearFdlg(fdlg, fdlgClear)	((fdlg) &= ~(fdlgClear))
#define SetFdlg(fdlg, fdlgSet)		((fdlg) |= (fdlgSet))
#define	FlipFdlg(fdlg, fdlgFlip)	((fdlg) ^= (fdlgFlip))

typedef struct _dli			// DiaLog Initializer. 
	{
	HWND	hwnd;
	int	dx, dy;
	FDLG	fdlg;
	WORD	wRef;
	BYTE *	rgb;			// App-supplied rgtmw (in sbDlg).

#ifdef	SDM_MULTI_SB
	SB	sb;
#endif	//SDM_MULTI_SB

#ifdef	SDM_ENV_MAC
	BOOL	fDrawAccel;		// Draw underlines for accelerators.
#endif	//SDM_ENV_MAC
	}  DLI;
#define pdliNull	((DLI *)0)



///////////////////////////////////////////////////////////////////////////////
// Misc Functions .

extern WORD	PASCAL	wRefDlgCur;		// Cached value. 
extern HCAB	PASCAL	hcabDlgCur;		// Cached value. 

#define	WRefDlgCur()	wRefDlgCur
#define	HcabDlgCur()	hcabDlgCur

// Sizeof runtime space.
#define CbRuntimeCtm(ctm)						\
	((ctm) * (2 * sizeof(TMC) + sizeof(TMT) + sizeof(REC) +		\
	    6 * sizeof(WORD) +						\
	    (sizeof(HWND) > sizeof(char **) ?				\
		sizeof(HWND) : sizeof (char **)) +			\
	    7 * sizeof(BYTE) + 2 * sizeof(GOBJ) + sizeof(char)))


///////////////////////////////////////////////////////////////////////////////
// RenderProc environment-specific Draw Structure.			     

typedef struct _rds
	{
#ifdef	SDM_ENV_WIN
	HDC	hpdc;
#endif	//SDM_ENV_WIN
#ifdef	SDM_ENV_PM
	HPS	hpdc;
#endif	//SDM_ENV_PM

	HWND	hwnd;

#ifdef	SDM_ENV_PM
	RECTL	rect;
#else	//SDM_ENV_PM
	RECT	rect;
#endif	//!SDM_ENV_PM
	} RDS;

#ifdef	SDM_FEDT
#define SM_SETSECRET		0x800a	// Tell (any) FEDT text is "secret". 
#ifdef	SDM_MULTI_SB
#define	SM_SETGOBJ		0x800b	// Set text via GOBJ.
#endif	//SDM_MULTI_SB
#endif	//SDM_FEDT

// Private message sent from fedt to dialog window.
#define SM_USER			0x8005


///////////////////////////////////////////////////////////////////////////////
// PictureProc message-specific parameter.				     

typedef union _sdmp
	{
	REC *	prec;

#ifdef	SDM_ENV_PM
	PQMSG	lpmsg;
	HPS	hps;
#endif	//SDM_ENV_PM
#ifdef	SDM_ENV_WIN
	LPMSG	lpmsg;
	HDC	hdc;
#endif	//SDM_ENV_WIN
#ifdef SDM_ENV_MAC
	EventPtr lpmsg;
#endif	//SDM_ENV_MAC
	} SDMP;



///////////////////////////////////////////////////////////////////////////////
// Procedure templates (for callbacks).					     

#ifdef	SDM_CALLBACK_PD
#define	SDM_CALLBACK	NEAR		// Near callback. 
#endif	//SDM_CALLBACK_PD

#ifdef	SDM_CALLBACK_FAR
#define	SDM_CALLBACK	FAR PASCAL	// Far callback.
#endif	//SDM_CALLBACK_FAR

typedef BOOL (SDM_CALLBACK * PFN_DIALOG)(DLM, TMC, WORD, WORD, WORD);
typedef PFN_DIALOG	PFN_ITEM;

// EB/EL Cab Save CallBack. 

typedef VOID (SDM_CALLBACK * PFN_SAVECAB)(HCAB, WORD, TMC, BOOL);
#define pfnSaveCabNull	((PFN_SAVECAB)0)

// Top level Modal Message Filter. 
#ifdef	SDM_ENV_PM
typedef BOOL (SDM_CALLBACK * PFN_FILTERMSG)(PQMSG);
#endif	//SDM_ENV_PM

#ifdef	SDM_ENV_WIN
typedef BOOL (SDM_CALLBACK * PFN_FILTERMSG)(LPMSG);
#endif	//SDM_ENV_WIN

#ifdef	SDM_ENV_MAC
typedef BOOL (SDM_CALLBACK * PFN_FILTERMSG)(EventPtr);
#endif	//SDM_ENV_MAC

#define pfnFilterMsgNull	((PFN_FILTERMSG)0)

// General control proc template. 
typedef WORD (SDM_CALLBACK * PFN_CTRL)(TMM, VOID *, WORD, WORD, TMC, WORD);
#define pfnCtrlNull	((PFN_CTRL)0)
typedef WORD (SDM_CALLBACK * PFN_PIC)(TMM, VOID *, HWND, WORD, TMC, WORD);

typedef WORD (SDM_CALLBACK * PFN_PARSE)(TMM, STR, WORD, WORD, TMC, WORD);
typedef PFN_PARSE	PFN_FORMAT;
typedef WORD (SDM_CALLBACK * PFN_LISTBOX)(TMM, STR, WORD, WORD, TMC, WORD);
typedef WORD (SDM_CALLBACK * PFN_RENDER)(TMM, RDS *, FTMS, FTMS, TMC, WORD);
typedef WORD (SDM_CALLBACK * PFN_GENERAL)(TMM, SDMP *, HWND, WORD, TMC, WORD);



///////////////////////////////////////////////////////////////////////////////
// Procedure Templates.							     

#ifdef	CC
#define	SDMPUBLIC	FAR PASCAL
#else	//CC
#define	SDMPUBLIC
#endif	//!CC



///////////////////////////////////////////////////////////////////////////////
// FtmeIsSdmMessage() return values.					     

// Need special return values for functions that normally return fTrue/fFalse.
typedef WORD		FTME;
#define ftmeNull	((FTME)0)
#define ftmeTrue	((FTME)1)
#define ftmeError	((FTME)2)



///////////////////////////////////////////////////////////////////////////////
// Hard-coded callbacks.						     

// Out Of Memory Support. 
typedef	WORD		SEV;
#define	sevMinor	1		// Minor (painting) error.
					// don't cast, since used in MASM
#define sevMajor	((SEV)2)	// Major error.
#define sevLmem		((SEV)3)	// Out of LMEM memory.
#define sevHcabFromDlg	((SEV)4)	// HcabFromDlg() failure.
#define sevDirFill	((SEV)5)	// Directory fill failed.
#define sevList		((SEV)6)	// Non-directory ListBox fill failure.

BOOL	SDMPUBLIC	FSdmDoIdle(BOOL);
BOOL	SDMPUBLIC	FRetrySdmError(WORD, HDLG, SEV);

// Directory ListBox support. 
// A werid case of an SDM-supplied callback. 
WORD	SDM_CALLBACK	WDirListSdm(TMM, STR, WORD, WORD, TMC, WORD);
BOOL	SDM_CALLBACK	FItmDirListSdm(DLM, TMC, WORD, WORD, WORD);


#ifndef	NOBITMAP
// Bitmap support - handle from id. 

#ifdef	SDM_ENV_MAC
typedef	BitMap FAR * FAR *		HBITMAP;
#endif	//SDM_ENV_MAC

HBITMAP	SDMPUBLIC	HbmpFromIBmp(WORD);

#define	hbmpNull	((HBITMAP)NULL)
#endif	//!NOBITMAP



///////////////////////////////////////////////////////////////////////////////
// Pseudo DirectoyListBox Options. 
//	-- passed in the wParam to SDM CallBack. 
//	-- also used as flags to FillDirListTmc(). 

typedef	WORD	FDIR;
#define fdirMain		((FDIR)0x0100)	// ListBox is the main one. 
#define fdirSecondary		((FDIR)0x0200)	// It is the secondary one. 
#define fdirHasSecondary	((FDIR)0x0400)	// Group has secondary LB. 
#define fdirHasStatic		((FDIR)0x0800)	// Group has tracking text. 
#define fdirReadWriteFiles	((FDIR)0x0000)	// Simple read/write files. 
#define fdirReadOnlyFiles	((FDIR)0x0001)	// Read only files. 
#define fdirHiddenFiles		((FDIR)0x0002)	// Hidden files. 
#define fdirSystemFiles		((FDIR)0x0004)	// System files. 
#define fdirSubDirectories	((FDIR)0x0010)	// Subdirectories. 
#define fdirArchives		((FDIR)0x0020)	// Archives. 
#define fdirDrives		((FDIR)0x4000)	// Drives. 
#define fdirXOR			((FDIR)0x8000)	// Don't show normal files. 
#define fdirAttrMask		((FDIR)0xc037)	// Mask for attributres. 



#ifndef	LBOX_INCLUDED
// Return values from FlbfFillDirListTmc()

typedef	WORD	FLBF_LBOX;
// Should never be returned.
#define flbfNil			((FLBF_LBOX)0x0000)

// Couldn't DosSelectDisk().
#define flbfSelectDiskFailed	((FLBF_LBOX)0x0001)

// Couldn't DosQCurDir().
#define flbfDiskReadFailed	((FLBF_LBOX)0x0002)

// Couldn't DosChDir().
#define flbfChDirFailed		((FLBF_LBOX)0x0004)

// The ListBox(es) was filled.
#define flbfListingMade		((FLBF_LBOX)0x0008)

// Path contained a filename.
#define flbfFileOnly		((FLBF_LBOX)0x0010)
#endif	//!LBOX_INCLUDED

// Never returned by LBOX, but used by SDM to indicate error.
#define flbfOOM			((FLBF_LBOX)0x0020)




///////////////////////////////////////////////////////////////////////////////
// Misc types.								     

#define hNull		((VOID **)0)	// Generic null handle. 
#define	ppvNull		((VOID **)0)	// Null lmem handle. 

#ifdef	SDM_ENV_WIN
#define hfontNull	((HFONT)NULL)
#endif	//SDM_ENV_WIN
#ifdef	SDM_ENV_PM
#define ifontNull	((LONG)NULL)
#endif	//SDM_ENV_PM



///////////////////////////////////////////////////////////////////////////////
// SDM Initialization structure.					     
#ifdef	SDM_ENV_PM
typedef int (FAR PASCAL * FARPROC)();
#endif	//SDM_ENV_PM

#ifndef	lpfncompNull
#define	lpfncompNull	((FARPROC)NULL)
#endif	//!lpfncompNull

typedef struct _sdi
	{
#ifdef	SDM_ENV_WIN
	STR	strApp;			// Unique application name. 
	HANDLE	hinstCur;		// Current application instance. 
	HANDLE	hinstPrev;		// Previous application instance. 
	HCURSOR	hcursorArrow;		// For dialog class - PM port later. 
	HDC	hdcMem;			// Memory DC.
	int	dyLeading;		// TextMetrics tmExternalLeading.
#endif	//SDM_ENV_WIN

#ifdef	SDM_ENV_PM
	HPS	hpsMem;			// Memory DC.
	HAB	hab;
	HMQ	hmq;
	int	dySysFontDescent;
#endif	//SDM_ENV_PM

#ifdef	SDM_ENV_WIN_PM
	HWND	hwndApp;		// Application's main window. 
	int	dxSysFontChar;		// Width of system font (average).
	int	dySysFontChar;		// Height of system font (maximum).
	int	dySysFontAscent;	// Height of system font ascenders.
#endif	//SDM_ENV_WIN_PM

#ifdef	SDM_ENV_MAC
	int	idDialog;		// Resource id of dialog template.
	int	idDitl;			// Resource id of itemlist template.
	QD *	pqd;			// Pointer to QuickDraw globals.
	BOOL	fColorQD;		// True if color is supported.
#endif	//SDM_ENV_MAC

	PFN_FILTERMSG	pfnFilterMsg;	// Message filter callback. 

#ifdef	SDM_MULTI_SB
	SB	sbEL;			// SDM's space for EL support. 
#endif	//SDM_MULTI_SB

#ifdef	SDM_FEDT
	STR	strFedtClass;		// Class name for FEDT. 
#endif	//SDM_FEDT

	FARPROC	lpfncomp;
	char *	szScrollClass;		// class for listbox scrollbars

// Font for display.  Replaces system font if non-null.
#ifdef	SDM_ENV_WIN
	HFONT	hfont;
#endif	//SDM_ENV_WIN
#ifdef	SDM_ENV_PM
	LONG	ifont;
#endif	//SDM_ENV_PM
	} SDI;

#define psdiNull	((SDI *)0)



///////////////////////////////////////////////////////////////////////////////
// DLG access. defined here for the following macros that are defined        

typedef	struct _dlh
	{
	struct _dlt **	hdlt;			// Dialog Template. 
	HCAB	hcab;			// Initial CAB for Dialog. 
	HWND	hwndDlg;		// Dialog's (frame) window. 

#ifdef	SDM_ENV_PM
	HWND	hwndDlgClient;		// Dialog's client window. 
#else	//SDM_ENV_PM
#define	hwndDlgClient	hwndDlg		// I know its gross, but it saves a lot
					// of mess in the sdm .c files. 
#endif	//!SDM_ENV_PM

	WORD	wRef;			// User supplied Dialog word. 
	FDLG	fdlg;			// Dialog flags. 
	WORD	hid;			// Help ID. 
	} DLH;				// Public part of DLG structure. 



///////////////////////////////////////////////////////////////////////////////
// SDS access. (Only defined here for the following macros that are defined  
// by the SDM project.  This struct is NOT for public use!)                  

typedef struct _sds_sdm			// State of SDM. 
	{
	SB	sbDlgCur;		// SbDds if not SDM_MULTI_SB. 
	SB	sbDlgFocus;
	void **	ppdlgCur;		// Current Dialog. 
	void **	ppdlgFocus;		// Dialog with input focus. 
	} SDS_SDM;

extern SDS_SDM	PASCAL	sds;



#ifdef	SDM_MULTI_SB
#define	HdlgGetCur()		((HDLG)HpOfSbIb(sds.sbDlgCur, sds.ppdlgCur))
#define	HdlgGetFocus()		((HDLG)HpOfSbIb(sds.sbDlgFocus, sds.ppdlgFocus))
#else	//SDM_MULTI_SB
#define	HdlgGetCur()		((HDLG)sds.ppdlgCur)
#define	HdlgGetFocus()		((HDLG)sds.ppdlgFocus)
#endif	//!SDM_MULTI_SB




///////////////////////////////////////////////////////////////////////////////
// Other common functions 

// Return the value TMC (first TMC in group usually) or do nothing if not
//	grouped.
#define	TmcValue(tmcG)	((tmcG) & ~ftmcGrouped)
#define FIsGroupTmc(tmc)	((tmc & ftmcGrouped) != fFalse)



///////////////////////////////////////////////////////////////////////////////
// General objects support.

#ifdef	SDM_MULTI_SB
typedef LONG	HOBJ;			// A huge handle.

#define	HobjOfSbPpv(sb, ppv)	((HOBJ)HpOfSbIb(sb, ppv))
#else	//SDM_MULTI_SB
typedef VOID **	HOBJ;			// A near handle.

#define	HobjOfSbPpv(sb, ppv)	((HOBJ)(ppv))		// SB ignored.
#endif	//!SDM_MULTI_SB

#define hobjNull	((HOBJ) NULL)

typedef struct _gobj
	{
	HOBJ	hobjBase;	// Handle to base object.
	WORD	bArg;		// Offset from that object.
	} GOBJ;			// Reference to any object.

#ifndef	CC
#ifdef	SDM_MULTI_SB
#define	GobjToGobj(g1, g2)	{(g2).hobjBase = (g1).hobjBase;	\
				(g2).bArg = (g1).bArg;}
#else	//SDM_MULTI_SB
#define	GobjToGobj(g1, g2)	(g2 = g1)
#endif	//!SDM_MULTI_SB
#else	//CC
#define	GobjToGobj(g1, g2)	(g2 = g1)
#endif	//!CC

#define FIsNullGobj(g)		((g).hobjBase == hobjNull && (g).bArg == 0)
#define	FIsNullHobjbArg(h, b)	((h) == hobjNull && (b) == 0)
#define PvFromGobj(h, b)	\
	(VOID*)((h) == hobjDds ? (b) : *(WORD *)(h) + (b))

#ifdef	SDM_MULTI_SB
#define	FIsEmptyGobj(g)		FIsEmptyHobjBArg((g).ghobjBase, (g).bArg)
#define FIsEmptyHobjBArg(hobj, bArg)	\
		(IbOfHp(hobj) == (WORD)ppvZero && (bArg) == 0)
#else	//SDM_MULTI_SB
#define FIsEmptyGobj(g)		FIsEmptyHobjBarg((g).hobjBase, (g).bArg)
#define FIsEmptyHobjBArg(hobj, bArg)	\
		((WORD)(hobj) == (WORD)ppvZero && (bArg) == 0)
#endif	//!SDM_MULTI_SB



///////////////////////////////////////////////////////////////////////////////
// Miscellaneous LBOX stuff (for apps that don't include lbox.h but use the  //
// toolbox.								     //
///////////////////////////////////////////////////////////////////////////////

#ifndef	LBOX_INCLUDED
typedef	struct _FILEFINDBUF FAR *	LPFFB;

typedef	struct	_LBX **	HLBX;

#ifdef	SDM_ENV_WIN
typedef	RECT	LBR;
typedef	WORD	HDIR;
#endif	//SDM_ENV_WIN

#ifdef	SDM_ENV_PM
typedef	struct _LBR
	{
	LONG	left;
	LONG	bottom;
	LONG	right;
	LONG	top;
	LONG	dyConv;
	HWND	hwnd;
	} LBR;
#endif	//SDM_ENV_PM

typedef	WORD	LBC;

#ifdef	LBOX_CALLBACK_PD
#define	LBOX_CALLBACK		NEAR
#define LBOX_CALLBACK_NAT	FAR
#endif	//LBOX_CALLBACK_PD

#ifdef	LBOX_CALLBACK_FAR
#define	LBOX_CALLBACK		FAR PASCAL
#define	LBOX_CALLBACK_NAT	FAR PASCAL
#endif	//LBOX_CALLBACK_FAR

typedef BOOL  (LBOX_CALLBACK_NAT * LPFNIDR)(WORD, char *, BOOL);
typedef BOOL  (LBOX_CALLBACK_NAT * LPFNMDR)(WORD, char *, char *, WORD, char *);
#endif	//!LBOX_INCLUDED

#include <sdmproc.h>

#endif	//!SDM_INCLUDED		Entire file.
