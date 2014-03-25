/*
	QWINDOWS.H : Version of WINDOWS.H for Pcode Interpreter
	Version 1.2.0
	Date : 27 May 1987
	

	-- adapted for Windows 1.2 entries
	-- Also LOBYTE changed for proper CS code generation
*/

/*  If defined, the following flags inhibit definition
	of the indicated constants.

	NOGDICAPMASKS	CC_*, LC_*, PC_*, CP_*, TC_*, RC_
	NOVIRTUALKEYCODES	VK_*
	NOWINMESSAGES	WM_*
	NONCMESSAGES	WM_NC* and HT*
	NOWINSTYLES 	WS_*, CS_*, ES_*, LBS_*
	NOSYSMETRICS	SM_*
	NOMENUS		MF_*
	NOICON		IDI_*
	NOKEYSTATE		MK_*
	NOSYSCOMMANDS	SC_*
	NORASTEROPS 	binary and tertiary raster ops
	NOSHOWWINDOW	SHOW_* and HIDE_*

	The following flags inhibit declarations of the following groups
	of procedures and type definitions.
	"associated routines" refers to routines with parameters or return
		values of the given type.

	OEMRESOURCE     - define this and get the Oem Resource values.
	NOSYSMETRICS    - GetSystemMetrics
	NOATOM	    - Atom Manager routines
	NOBITMAP	    - typedef HBITMAP and associated routines
	NOBRUSH	    - typedef HBRUSH and associated routines
	NOCLIPBOARD     - clipboard routines
	NOCOLOR
	NOCREATESTRUCT  - typedef CREATESTRUCT
	NOCTLMGR	    - control and dialog routines
	NODRAWTEXT	    - DrawText() and DT_*
	NOFONT	    - typedef FONT and associated routines
	NOGDI	    - StretchBlt modes and gdi logical objects
	NOHDC	    - typedef HDC and associated routines
	NOMB	    - MB_* and MessageBox()
	NOMEMMGR	    - GMEM_*, LMEM_*, GHND, LHND, associated routines
	NOMENUS	    - HMENU and associated routines
	NOMETAFILE	    - typedef METAFILEPICT
	NOMINMAX	    - Macros min(a,b) and max(a,b)
	NOMSG	    - typedef MSG and associated routines
	NOOPENFILE	    - OpenFile(), OemToAnsi, AnsiToOem, and OF_*
	NOPEN	    - typedef HPEN and associated routines
	NOPOINT	    - typedef POINT and associated routines
	NORECT	    - typedef RECT and associated routines
	NOREGION	    - typedef HRGN and associated routines
	NOSCROLL	    - SB_* and scrolling routines
	NOSOUND	    - Sound driver routines
	NOTEXTMETRIC    - typedef TEXTMETRIC and associated routines
	NOWH	    - SetWindowsHook and WH_*
	NOWINOFFSETS    - GWL_*, GCL_*, associated routines
	NOWNDCLASS	    - typedef WNDCLASS and associated routines
	NOCOMM	    - COMM driver routines
	NOKANJI	    - Kanji support stuff.
*/

#ifdef RC_INVOKED
/*
Turn off a bunch of stuff to ensure that RC files compile OK
*/
#define NOCOMM
#define NOKANJI
#define NOWH
#define NOTEXTMETRIC
#define NOSOUND
#define NOSCROLL
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NORASTEROPS
#define NOATOM
#define NOFONT
#define NOREGION
#define NORECT
#define NOPEN
#define NOPOINT
#define NOOPENFILE
#define NOMSG
#define NOMINMAX
#define NOMETAFILE
#define NOHDC
#define NOGDI
#endif


/* !!!!! - define EXPORT macro */
#ifndef EXPORT
#define EXPORT	export
#endif

/* !!!!! - define NATIVE macro */
#ifndef NATIVE
#define NATIVE	native
#endif

#ifndef PASCAL
#define PASCAL	pascal
#endif

#define FALSE	0
#define TRUE	1
#define NULL	0

#define FAR	far
#define NEAR
#define LONG	long
#define VOID	void

typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned long  DWORD;
typedef int	  BOOL;
typedef char	 *PSTR;
typedef char NEAR*NPSTR;
typedef char FAR *LPSTR;
typedef int  FAR *LPINT;

#ifndef NOMINMAX
#define max(a,b)	((a) > (b) ? (a) : (b))
#define min(a,b)	((a) < (b) ? (a) : (b))
#endif

/* Dumb versions of word-swapping macros.  Use these in native code only,
* not Pcode.
*/
#define MAKELONGX(a, b)  ((long)(((unsigned)(a)) | ((unsigned long)((unsigned)(b))) << 16))
#define LOWORDX(l)	 ((WORD)(l))
#define HIWORDX(l)	 ((WORD)(((DWORD)(l) >> 16) & 0xffff))

/* Smart versions of word-swapping macros.  These generate better Pcode than
*  the dumb versions.
*/
uop WORD WUOP_QWIN();
uop long LUOP_QWIN();
#define LOWORD(l) (__FNATIVE__?LOWORDX(l):WUOP_QWIN(2,0xe0,0xda,(long)(l)))
#define HIWORD(l) (__FNATIVE__?HIWORDX(l):WUOP_QWIN(1,0xda,(long)(l)))
#define MAKELONG(lo,hi) (__FNATIVE__?MAKELONGX(lo,hi):LUOP_QWIN(0,(int)(hi),(int)(lo)))

/* !!!!! - proper pcode LOBYTE macro */
#define LOBYTE(w)	((BYTE)(w) & 0xff)
#define HIBYTE(w)	(((WORD)(w) >> 8) & 0xff)

#ifndef NOPOINT
#define MAKEPOINT(l)	(*((POINT *)&(l)))
#endif

#ifdef OEMRESOURCE

#ifdef WIN23

#define OBM_CLOSE           32754
#define OBM_UPARROW         32753
#define OBM_DNARROW         32752
#define OBM_RGARROW         32751
#define OBM_LFARROW         32750
#define OBM_REDUCE          32749
#define OBM_ZOOM            32748
#define OBM_RESTORE         32747
#define OBM_REDUCED         32746
#define OBM_ZOOMD           32745
#define OBM_RESTORED        32744
#define OBM_UPARROWD        32743
#define OBM_DNARROWD        32742
#define OBM_RGARROWD        32741
#define OBM_LFARROWD        32740
#define OBM_MNARROW         32739
#define OBM_COMBO           32738

#define OBM_OLD_CLOSE       32767
#define OBM_SIZE            32766
#define OBM_OLD_UPARROW     32765
#define OBM_OLD_DNARROW     32764
#define OBM_OLD_RGARROW     32763
#define OBM_OLD_LFARROW     32762
#define OBM_BTSIZE          32761
#define OBM_CHECK           32760
#define OBM_CHECKBOXES      32759
#define OBM_BTNCORNERS      32758
#define OBM_OLD_REDUCE      32757
#define OBM_OLD_ZOOM        32756
#define OBM_OLD_RESTORE     32755

#else
#define OBM_CLOSE	32767
#define OBM_SIZE	32766
#define OBM_UPARROW	32765
#define OBM_DNARROW	32764
#define OBM_RGARROW	32763
#define OBM_LFARROW	32762
#define OBM_BTSIZE	32761
#define OBM_CHECK	32760
#define OBM_CHECKBOXES	32759
#define OBM_BTNCORNERS	32758
#define OBM_REDUCE	32757
#define OBM_ZOOM	32756
#define OBM_RESTORE	32755
#define OCR_NORMAL	32512
#define OCR_IBEAM	32513
#define OCR_WAIT	32514
#define OCR_CROSS	32515
#define OCR_UP		32516
#define OCR_SIZE	32640
#define OCR_ICON	32641
#define OCR_SIZENWSE	32642
#define OCR_SIZENESW	32643
#define OCR_SIZEWE	32644
#define OCR_SIZENS	32645
#define OCR_SIZEALL     32646

#define OIC_SAMPLE	32512
#define OIC_HAND	32513
#define OIC_QUES	32514
#define OIC_BANG	32515
#define OIC_NOTE	32516
#endif /* WIN23 */

#endif

/*  Scroll bar constants */
#ifndef NOSCROLL
#define SB_HORZ 	    0
#define SB_VERT 	    1
#define SB_CTL		    2
#define SB_BOTH             3

/*  Scroll Commands */
#define SB_LINEUP	    0
#define SB_LINEDOWN	    1
#define SB_PAGEUP	    2
#define SB_PAGEDOWN	    3
#define SB_THUMBPOSITION    4
#define SB_THUMBTRACK	    5
#define SB_TOP		    6
#define SB_BOTTOM	    7
#define SB_ENDSCROLL	    8
#endif

#ifndef NOSHOWWINDOW

/* ShowWindow commands */

/* ShowWindow commands */

#define SW_HIDE 	    0
#define SW_SHOWNORMAL	    1
#define SW_RESTORE	    1
#define SW_NORMAL	    1
#define SW_SHOWMINIMIZED    2
#define SW_SHOWMAXIMIZED    3
#define SW_MAXIMIZE	    3
#define SW_SHOWNOACTIVATE   4
#define SW_SHOW 	    5
#define SW_MINIMIZE	    6
#define SW_SHOWMINNOACTIVE  7
#define SW_SHOWNA	    8

#define SW_ICON 	    SW_SHOWMINIMIZED
#define SW_FULLSCREEN	    SW_SHOWMAXIMIZED

/* Old showwindow commands */

#define HIDE_WINDOW	0
#define SHOW_OPENWINDOW 1
#define SHOW_ICONWINDOW 2
#define SHOW_FULLSCREEN 3
#define SHOW_OPENNOACTIVATE 4

/* identifiers for the WM_SHOWWINDOW message */
#define SW_PARENTCLOSING    1
#define SW_OTHERZOOM	    2
#define SW_PARENTOPENING    3
#define SW_OTHERUNZOOM	    4
#endif

/* flags for regions */
#ifndef NOREGION
#define ERROR		0
#define NULLREGION	1
#define SIMPLEREGION	2
#define COMPLEXREGION	3

/* styles for CombineRgn */
#define RGN_AND  1
#define RGN_OR	 2
#define RGN_XOR  3
#define RGN_DIFF 4
#define RGN_COPY 5
#endif

#ifndef NOVIRTUALKEYCODES
/* Virtual Keys, Standard Set */

#define VK_LBUTTON  0x01
#define VK_RBUTTON  0x02
#define VK_CANCEL   0x03
#define VK_MBUTTON  0x04    /* NOT contiguous with L & RBUTTON */
#define VK_BACK     0x08
#define VK_TAB	    0x09
#define VK_CLEAR    0x0c
#define VK_RETURN   0x0d
#define VK_SHIFT    0x10
#define VK_CONTROL  0x11
#define VK_MENU     0x12
#define VK_PAUSE    0x13
#define VK_CAPITAL  0x14
#define VK_ESCAPE   0x1b
#define VK_SPACE    0x20

#define VK_PRIOR    0x21
#define VK_NEXT     0x22
#define VK_END	    0x23
#define VK_HOME     0x24
#define VK_LEFT     0x25
#define VK_UP	    0x26
#define VK_RIGHT    0x27
#define VK_DOWN     0x28

/* VK_A thru VK_Z are the same as their ASCII equivalents: 'A' thru 'Z' */
/* VK_0 thru VK_9 are the same as their ASCII equivalents: '0' thru '0' */

#define VK_SELECT   0x29
#define VK_PRINT    0x2a
#define VK_EXECUTE  0x2b

/* 10/21/86 for Japan Keyboards linsh */
#define VK_COPY 	0x2c

#define VK_INSERT   0x2d
#define VK_DELETE   0x2e
#define VK_HELP     0x2f

#define VK_NUMPAD0  0x60
#define VK_NUMPAD1  0x61
#define VK_NUMPAD2  0x62
#define VK_NUMPAD3  0x63
#define VK_NUMPAD4  0x64
#define VK_NUMPAD5  0x65
#define VK_NUMPAD6  0x66
#define VK_NUMPAD7  0x67
#define VK_NUMPAD8  0x68
#define VK_NUMPAD9  0x69
#define VK_MULTIPLY 0x6A
#define VK_ADD	    0x6B
#define VK_SEPARATOR 0x6C
#define VK_SUBTRACT 0x6D
#define VK_DECIMAL  0x6E
#define VK_DIVIDE   0x6F

#define VK_F1	    0x70
#define VK_F2	    0x71
#define VK_F3	    0x72
#define VK_F4	    0x73
#define VK_F5	    0x74
#define VK_F6	    0x75
#define VK_F7	    0x76
#define VK_F8	    0x77
#define VK_F9	    0x78
#define VK_F10	    0x79
#define VK_F11	    0x7a
#define VK_F12	    0x7b
#define VK_F13	    0x7c
#define VK_F14	    0x7d
#define VK_F15	    0x7e
#define VK_F16	    0x7f

#define VK_NUMLOCK  0x90

#endif

/* SetWindowsHook codes */
#ifndef NOWH
#define WH_MSGFILTER	    -1
#define WH_JOURNALRECORD    0
#define WH_JOURNALPLAYBACK  1
#define WH_KEYBOARD	    2
#define WH_GETMESSAGE	    3
#define WH_CALLWNDPROC	    4
#define WH_CBT		    5
#define WH_SYSMSGFILTER     6
#define WH_WINDOWMGR        7

/* HC_* Hook Codes */
#define HC_LPLPFNNEXT -2
#define HC_LPFNNEXT   -1
#define HC_ACTION      0
#define HC_GETNEXT     1
#define HC_SKIP        2
#define HC_NOREM       3

/* CBT hook codes */
#define HCBT_MOVESIZE  0
#define HCBT_MINMAX    1

/* WH_MSGFILTER filter proc codes */
#define MSGF_DIALOGBOX	    0
#define MSGF_MESSAGEBOX     1
#define MSGF_MENU	    2
#define MSGF_MOVE	    3
#define MSGF_SIZE	    4
#define MSGF_SCROLLBAR	    5
#define MSGF_NEXTWINDOW     6

/* Define window manager hook codes */
#define WC_INIT 	    1
#define WC_SWP		    2
#define WC_DEFWINDOWPROC    3
#define WC_MINMAX	    4
#define WC_MOVE 	    5
#define WC_SIZE 	    6
#define WC_DRAWCAPTION	    7
#endif

/*  Binary raster ops */
#ifndef NORASTEROPS
#define R2_BLACK	    1	/*  0	    */
#define R2_NOTMERGEPEN	    2	/* DPon     */
#define R2_MASKNOTPEN	    3	/* DPna     */
#define R2_NOTCOPYPEN	    4	/* PN	    */
#define R2_MASKPENNOT	    5	/* PDna     */
#define R2_NOT		    6	/* Dn	    */
#define R2_XORPEN	    7	/* DPx	    */
#define R2_NOTMASKPEN	    8	/* DPan     */
#define R2_MASKPEN	    9	/* DPa	    */
#define R2_NOTXORPEN	   10	/* DPxn     */
#define R2_NOP		   11	/* D	    */
#define R2_MERGENOTPEN	   12	/* DPno     */
#define R2_COPYPEN	   13	/* P	    */
#define R2_MERGEPENNOT	   14	/* PDno     */
#define R2_MERGEPEN	   15	/* DPo	    */
#define R2_WHITE	   16	/*  1	    */

/*  Ternary raster operations */
#define SRCCOPY     (DWORD)0x00CC0020  /* dest=source			    */
#define SRCPAINT    (DWORD)0x00EE0086  /* dest=source OR dest		    */
#define SRCAND	    (DWORD)0x008800C6  /* dest = source AND dest	     */
#define SRCINVERT   (DWORD)0x00660046  /* dest = source XOR	 dest	     */
#define SRCERASE    (DWORD)0x00440328  /* dest = source AND (not dest )      */
#define NOTSRCCOPY  (DWORD)0x00330008  /* dest = (not source)		     */
#define NOTSRCERASE (DWORD)0x001100A6  /* dest = (not source) AND (not dest) */
#define MERGECOPY   (DWORD)0x00C000CA  /* dest = (source AND pattern)	     */
#define MERGEPAINT  (DWORD)0x00BB0226  /* dest = (NOT source) OR dest	     */
#define PATCOPY     (DWORD)0x00F00021  /* dest = pattern		     */
#define PATPAINT    (DWORD)0x00FB0A09  /* dest = DPSnoo 		     */
#define PATINVERT   (DWORD)0x005A0049  /* dest = pattern XOR	 dest	     */
#define DSTINVERT   (DWORD)0x00550009  /* dest = (not dest)		     */
#define BLACKNESS   (DWORD)0x00000042  /* dest = BLACK			     */
#define WHITENESS   (DWORD)0x00FF0062  /* dest = WHITE			     */
#endif

#ifndef NOGDI
/* StretchBlt() modes */
#define BLACKONWHITE	1
#define WHITEONBLACK	2
#define COLORONCOLOR	3

/* PolyFill modes */
#define ALTERNATE	1
#define WINDING 	2

/* text alignment options */
#define TA_UPDATECP	1
#define TA_NOUPDATECP	0

#define TA_LEFT 	0
#define TA_RIGHT	2
#define TA_CENTER	6

#define TA_TOP		0
#define TA_BOTTOM	8
#define TA_BASELINE	24

#define ETO_GRAYED      1
#define ETO_OPAQUE      2
#define ETO_CLIPPED     4


#define ASPECT_FILTERING 0x00000001

#ifndef NOMETAFILE

/* Metafile Functions */
#define META_SETBKCOLOR 	     0x0201
#define META_SETBKMODE		     0x0102
#define META_SETMAPMODE 	     0x0103
#define META_SETROP2		     0x0104
#define META_SETRELABS		     0x0105
#define META_SETPOLYFILLMODE	     0x0106
#define META_SETSTRETCHBLTMODE	     0x0107
#define META_SETTEXTCHAREXTRA	     0x0108
#define META_SETTEXTCOLOR	     0x0209
#define META_SETTEXTJUSTIFICATION    0x020a
#define META_SETTEXTJUSTIFICATION    0x020A
#define META_SETWINDOWORG	     0x020B
#define META_SETWINDOWEXT	     0x020C
#define META_SETVIEWPORTORG	     0x020D
#define META_SETVIEWPORTEXT	     0x020E
#define META_OFFSETWINDOWORG	     0x020F
#define META_SCALEWINDOWEXT	     0x0400
#define META_OFFSETVIEWPORTORG	     0x0211
#define META_SCALEVIEWPORTEXT	     0x0412
#define META_LINETO		     0x0213
#define META_MOVETO		     0x0214
#define META_EXCLUDECLIPRECT	     0x0415
#define META_INTERSECTCLIPRECT	     0x0416
#define META_ARC		     0x0817
#define META_ELLIPSE		     0x0418
#define META_FLOODFILL		     0x0419
#define META_PIE		     0x081A
#define META_RECTANGLE		     0x041B
#define META_ROUNDRECT		     0x061C
#define META_PATBLT		     0x061D
#define META_SAVEDC		     0x001E
#define META_SETPIXEL		     0x041F
#define META_OFFSETCLIPRGN	     0x0220
#define META_TEXTOUT		     0x0521
#define META_BITBLT		     0x0922
#define META_STRETCHBLT		     0x0B23
#define META_POLYGON		     0x0324
#define META_POLYLINE		     0x0325
#define META_ESCAPE		     0x0626
#define META_RESTOREDC		     0x0127
#define META_FILLREGION		     0x0228
#define META_FRAMEREGION	     0x0429
#define META_INVERTREGION	     0x012A
#define META_PAINTREGION	     0x012B
#define META_SELECTCLIPREGION	     0x012C
#define META_SELECTOBJECT	     0x012D
#define META_SETTEXTALIGN	     0x012E
#define META_DRAWTEXT		     0x062F

#define	META_CHORD		     0x0830
#define	META_SETMAPPERFLAGS	     0x0231
#define	META_EXTTEXTOUT		     0x0a32 
#define	META_SETDIBTODEV	     0x0d33
#define	META_SELECTPALETTE	     0x0234
#define	META_REALIZEPALETTE	     0x0035
#define	META_ANIMATEPALETTE	     0x0436
#define	META_SETPALENTRIES	     0x0037
#define	META_POLYPOLYGON	     0x0538
#define	META_RESIZEPALETTE	     0x0139

#define	META_DIBBITBLT		     0x0940
#define	META_DIBSTRETCHBLT	     0x0b41
#define	META_DIBCREATEPATTERNBRUSH   0x0142
#define	META_STRETCHDIB		     0x0f43

#define	META_DELETEOBJECT	     0x01f0

#define	META_CREATEPALETTE	     0x00f7
#define META_CREATEBRUSH	     0x00F8
#define META_CREATEPATTERNBRUSH	     0x01F9
#define META_CREATEPENINDIRECT	     0x02FA
#define META_CREATEFONTINDIRECT	     0x02FB
#define META_CREATEBRUSHINDIRECT     0x02FC
#define META_CREATEBITMAPINDIRECT    0x02FD
#define META_CREATEBITMAP	     0x06FE
#define META_CREATEREGION	     0x06FF

#endif /* NOMETAFILE */

/* GDI escapes */
#define NEWFRAME	    1
#define ABORTDOC	    2
#define NEXTBAND	    3
#define SETCOLORTABLE	    4
#define GETCOLORTABLE	    5
#define FLUSHOUTPUT	    6
#define DRAFTMODE	    7
#define QUERYESCSUPPORT     8
#define SETABORTPROC	    9
#define STARTDOC	    10
#define ENDDOC		    11
#define GETPHYSPAGESIZE     12
#define GETPRINTINGOFFSET   13
#define GETSCALINGFACTOR    14
#define MFCOMMENT	    15	      /* meta file comment escape */
#define GETPENWIDTH         16
#define SETCOPYCOUNT        17
#define SELECTPAPERSOURCE   18
#define PASSTHROUGH         19
#define GETTECHNOLGY	    20
#define SETENDCAP	    21
#define SETLINEJOIN	    22
#define SETMITERLIMIT	    23
#define BANDINFO	    24
#define DRAWPATTERNRECT     25
#define GETVECTORPENSIZE    26
#define GETVECTORBRUSHSIZE  27
#define ENABLEDUPLEX        28
#define ENABLEMANULAFEED    29



#define GETEXTENDEDTEXTMETRICS  256
#define GETEXTENTTABLE          257
#define GETPAIRKERNTABLE        258
#define GETTRACKKERNTABLE       259

#define EXTTEXTOUT              512

#define ENABLERELATIVEWIDTHS    768
#define ENABLEPAIRKERNING       769
#define SETKERNTRACK            770
#define STRETCHBLT              2048


/* spooler error code */
#define SP_NOTREPORTED	    0x4000  /* set if GDI did not report error */
#define SP_ERROR	    (-1)    /* general errors who know what went wrong */
#define SP_APPABORT	    (-2)    /* app aborted the job - callback function returned false */
#define SP_USERABORT	    (-3)    /* user aborted the job through spooler's front end */
#define SP_OUTOFDISK	    (-4)    /* not enough disk space to spool */
#define SP_OUTOFMEMORY	    (-5)
#define PR_JOBSTATUS        0x0000
#endif

/* Object definitions for GDI EnumObjects. */
#define OBJ_PEN 	1
#define OBJ_BRUSH	2

#ifndef WIN_INTERNAL
typedef WORD HANDLE;
#endif
typedef HANDLE	     *PHANDLE;
typedef HANDLE	NEAR *SPHANDLE;
typedef HANDLE	FAR *LPHANDLE;

typedef int (FAR PASCAL *FARPROC)();
typedef int (NEAR PASCAL *NEARPROC)();
typedef HANDLE GLOBALHANDLE;
typedef HANDLE LOCALHANDLE;

#ifndef NOBITMAP
typedef struct tagBITMAP {
	short      bmType;
	short      bmWidth;
	short      bmHeight;
	short      bmWidthBytes;
	BYTE       bmPlanes;
	BYTE       bmBitsPixel;
	LPSTR      bmBits;
} BITMAP;
typedef BITMAP *PBITMAP;
typedef BITMAP NEAR *NPBITMAP;
typedef BITMAP FAR *LPBITMAP;

#ifdef WIN23
/* stolen from the Win 3.0 windows.h */

typedef struct tagRGBTRIPLE {
        BYTE    rgbtBlue;
        BYTE    rgbtGreen;
        BYTE    rgbtRed;
} RGBTRIPLE;

typedef struct tagRGBQUAD {
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
} RGBQUAD;

/* structures for defining DIBs */

typedef struct tagBITMAPCOREHEADER {
        DWORD   bcSize;                 /* used to get to color table */
        WORD    bcWidth;
        WORD    bcHeight;
        WORD    bcPlanes;
        WORD    bcBitCount;
} BITMAPCOREHEADER;
typedef BITMAPCOREHEADER FAR *LPBITMAPCOREHEADER;
typedef BITMAPCOREHEADER *PBITMAPCOREHEADER;

typedef struct tagBITMAPINFOHEADER{
        DWORD      biSize;
        DWORD      biWidth;
        DWORD      biHeight;
        WORD       biPlanes;
        WORD       biBitCount;

        DWORD      biCompression;
        DWORD      biSizeImage;
        DWORD      biXPelsPerMeter;
        DWORD      biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHEADER;

typedef BITMAPINFOHEADER FAR *LPBITMAPINFOHEADER;
typedef BITMAPINFOHEADER *PBITMAPINFOHEADER;

/* constants for the biCompression field */
#define BI_RGB      0L
#define BI_RLE8     1L
#define BI_RLE4     2L

typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[1];
} BITMAPINFO;
typedef BITMAPINFO FAR *LPBITMAPINFO;
typedef BITMAPINFO *PBITMAPINFO;

typedef struct tagBITMAPCOREINFO {
    BITMAPCOREHEADER    bmciHeader;
    RGBTRIPLE           bmciColors[1];
} BITMAPCOREINFO;
typedef BITMAPCOREINFO FAR *LPBITMAPCOREINFO;
typedef BITMAPCOREINFO *PBITMAPCOREINFO;

typedef struct tagBITMAPFILEHEADER {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHEADER;
typedef BITMAPFILEHEADER FAR *LPBITMAPFILEHEADER;
typedef BITMAPFILEHEADER *PBITMAPFILEHEADER;

/* DIB color table identifiers */

#define DIB_RGB_COLORS  0       /* color table in RGBTriples */
#define DIB_PAL_COLORS  1       /* color table in palette indices */

/* constants for CreateDIBitmap */
#define CBM_INIT        0x04    /* initialize bitmap */

#define BFT_BMAP			0x4d42	/* 'BM' */

#endif /* WIN23 */
#endif

typedef HANDLE	HSTR;
typedef HANDLE	HICON;
typedef HANDLE	HDC;
typedef HANDLE	HMENU;
typedef HANDLE	HPEN;
typedef HANDLE	HFONT;
typedef HANDLE	HBRUSH;
typedef HANDLE	HBITMAP;
typedef HANDLE	HCURSOR;
typedef HANDLE	HRGN;

typedef struct tagPOINT {
	int x;
	int y;
} POINT;
typedef POINT *PPOINT;
typedef POINT NEAR *NPPOINT;
typedef POINT FAR *LPPOINT;

#ifndef NORECT
#ifndef WIN_INTERNAL
typedef struct tagRECT {
	int left;
	int top;
	int right;
	int bottom;
} RECT;
#endif

typedef RECT *PRECT;
typedef RECT NEAR *NPRECT;
typedef RECT FAR *LPRECT;
#endif

#ifndef NOWNDCLASS
#ifndef NOBRUSH
typedef struct tagWNDCLASS {
	WORD    style;
	long    (FAR PASCAL *lpfnWndProc)();
	int     cbClsExtra;
	int     cbWndExtra;
	HANDLE  hInstance;
	HICON   hIcon;
	HCURSOR hCursor;
	HBRUSH  hbrBackground;
	LPSTR   lpszMenuName;
	LPSTR   lpszClassName;
} WNDCLASS;
typedef WNDCLASS     *PWNDCLASS;
typedef WNDCLASS NEAR *NPWNDCLASS;
typedef WNDCLASS FAR *LPWNDCLASS;
#endif
#endif

#ifndef WIN_INTERNAL
typedef HANDLE HWND;
#endif

#ifndef NOMSG
#ifndef NOPOINT
/* Message structure */
typedef struct tagMSG {
	HWND hwnd;
	WORD message;
	WORD wParam;
	LONG lParam;
	DWORD time;
	POINT pt;
} MSG;
typedef MSG *PMSG;
typedef MSG NEAR *NPMSG;
typedef MSG FAR *LPMSG;
#endif
#endif

/* Window field offsets for GetWindowLong & GetWindowWord */
#ifndef NOWINOFFSETS
#define GWL_WNDPROC	-4
#define GWW_HINSTANCE	-6
#define GWW_HWNDPARENT	-8
#define GWW_HWNDTEXT	-10
#define GWW_ID		-12
#define GWL_STYLE	-16

/* Class field offsets for GetClassLong & GetClassWord */
#define GCL_MENUNAME	    -8
#define GCW_HBRBACKGROUND   -10
#define GCW_HCURSOR	    -12
#define GCW_HICON	    -14
#define GCW_HMODULE	    -16
#define GCW_CBWNDEXTRA	    -18
#define GCW_CBCLSEXTRA	    -20
#define GCL_WNDPROC	    -24
#define GCW_STYLE	    -26
#endif

#ifndef NOWINMESSAGES
/* ** Window Procedure Messages */

#define WM_NULL 	    0x0000
#define WM_CREATE	    0x0001
#define WM_DESTROY	    0x0002
#define WM_MOVE 	    0x0003
#define WM_SIZEWAIT	    0x0004
#define WM_SIZE 	    0x0005
#define WM_ACTIVATE	    0x0006
#define WM_SETFOCUS	    0x0007
#define WM_KILLFOCUS	    0x0008
#define WM_SETVISIBLE	    0x0009
#define WM_ENABLE	    0x000a
#define WM_SETREDRAW	    0x000b
#define WM_SETTEXT	    0x000c
#define WM_GETTEXT	    0x000d
#define WM_GETTEXTLENGTH    0x000e
#define WM_PAINT	    0x000f
#define WM_CLOSE	    0x0010
#define WM_QUERYENDSESSION  0x0011
#define WM_QUIT 	    0x0012
#define WM_QUERYOPEN	    0x0013
#define WM_ERASEBKGND	    0x0014
#define WM_SYSCOLORCHANGE   0x0015
#define WM_ENDSESSION	    0x0016
#define WM_SYSTEMERROR	    0x0017
#define WM_SHOWWINDOW	    0x0018
#define WM_CTLCOLOR	    0x0019
#define WM_WININICHANGE     0x001a
#define WM_DEVMODECHANGE    0x001b
#define WM_ACTIVATEAPP	    0x001c
#define WM_FONTCHANGE	    0x001d
#define WM_TIMECHANGE	    0x001e
#define WM_CANCELMODE	    0x001f
#define WM_SETCURSOR	    0x0020
#define WM_MOUSEACTIVATE    0x0021
#define WM_CHILDACTIVATE    0x0022
#define WM_QUEUESYNC	    0x0023
#define WM_GETMINMAXINFO    0x0024
/* #define WM_OTHERWINDOWDESTROY 0x0025  (deleted by Raor) */
#define WM_PAINTICON	    0x0026
#define WM_ICONERASEBKGND   0x0027
#define WM_NEXTDLGCTL	    0x0028
#define WM_ALTTABACTIVE     0x0029	    /* for win386 only */
#define WM_SPOOLERSTATUS    0x002A

#ifndef NONCMESSAGES
#define WM_NCCREATE	    0x0081
#define WM_NCDESTROY	    0x0082
#define WM_NCCALCSIZE	    0x0083
#define WM_NCHITTEST	    0x0084
#define WM_NCPAINT	    0x0085
#define WM_NCACTIVATE	    0x0086
#define WM_GETDLGCODE	    0x0087
#define WM_SYNCPAINT	    0x0088
#define WM_SYNCTASK	    0x0089

#define ST_BEGINSWP	    0
#define ST_ENDSWP	    1

#define WM_NCMOUSEMOVE	    0x00a0
#define WM_NCLBUTTONDOWN    0x00a1
#define WM_NCLBUTTONUP	    0x00a2
#define WM_NCLBUTTONDBLCLK  0x00a3
#define WM_NCRBUTTONDOWN    0x00a4
#define WM_NCRBUTTONUP	    0x00a5
#define WM_NCRBUTTONDBLCLK  0x00a6
#define WM_NCMBUTTONDOWN    0x00a7
#define WM_NCMBUTTONUP	    0x00a8
#define WM_NCMBUTTONDBLCLK  0x00a9

/* WINWhere area codes */
#define HTERROR       -2
#define HTTRANSPARENT -1
#define HTNOWHERE      0
#define HTCLIENT       1
#define HTCAPTION      2
#define HTSYSMENU      3
#define HTGROWBOX      4
#define HTSIZE HTGROWBOX
#define HTMENU	       5
#define HTHSCROLL      6
#define HTVSCROLL      7
#define HTREDUCE       8
#define HTZOOM	       9
#define HTLEFT	       10
#define HTRIGHT        11
#define HTTOP	       12
#define HTTOPLEFT      13
#define HTTOPRIGHT     14
#define HTBOTTOM       15
#define HTBOTTOMLEFT   16
#define HTBOTTOMRIGHT  17
#define HTSIZEFIRST HTLEFT
#define HTSIZELAST  HTBOTTOMRIGHT
#endif

/* WM_MOUSEACTIVATE return codes */
#define MA_ACTIVATE	    1
#define MA_ACTIVATEANDEAT   2
#define MA_NOACTIVATE	    3

#define WM_KEYFIRST	    0x0100
#define WM_KEYLAST	    0x0108

#define WM_KEYDOWN	    0x0100
#define WM_KEYUP	    0x0101
#define WM_CHAR 	    0x0102
#define WM_DEADCHAR	    0x0103
#define WM_SYSKEYDOWN	    0x0104
#define WM_SYSKEYUP	    0x0105
#define WM_SYSCHAR	    0x0106
#define WM_SYSDEADCHAR	    0x0107
#define WM_YOMICHAR	    0x0108
#define WM_MOVECONVERTWINDOW 0x0109
#define WM_CONVERTREQUEST   0x010A
#define WM_CONVERTRESULT    0x010B

#define WM_INITDIALOG	    0x0110
#define WM_COMMAND	    0x0111
#define WM_SYSCOMMAND	    0x0112
#define WM_TIMER	    0x0113
#define WM_HSCROLL	    0x0114
#define WM_VSCROLL	    0x0115
#define WM_INITMENU	    0x0116
#define WM_INITMENUPOPUP    0x0117
#define WM_SYSTIMER	    0x0118
#define WM_DOQUIT           0x0119
#define WM_DOSETWINDOWPOS   0x011a
#define WM_DOSUSPEND        0x011b
#define WM_DOMINIMIZE       0x011c
#define WM_DOMAXIMIZE       0x011d
#define WM_DOACTIVATE       0x011e
#define WM_MENUSELECT       0x011f
#define WM_MENUCHAR         0x0120
#define WM_ENTERIDLE	    0x0121

#define WM_MOUSEFIRST	    0x0200
#define WM_MOUSELAST	    0x0209

#define WM_MOUSEMOVE	    0x0200	/* mouse related constants */
#define WM_LBUTTONDOWN	    0x0201
#define WM_LBUTTONUP	    0x0202
#define WM_LBUTTONDBLCLK    0x0203
#define WM_RBUTTONDOWN	    0x0204
#define WM_RBUTTONUP	    0x0205
#define WM_RBUTTONDBLCLK    0x0206
#define WM_MBUTTONDOWN	    0x0207
#define WM_MBUTTONUP	    0x0208
#define WM_MBUTTONDBLCLK    0x0209

/* clipboard messages */
#define WM_CUT		    0x0300
#define WM_COPY 	    0x0301
#define WM_PASTE	    0x0302
#define WM_CLEAR	    0x0303
#define WM_UNDO 	    0x0304
#define WM_RENDERFORMAT     0x0305
#define WM_RENDERALLFORMATS 0x0306
#define WM_DESTROYCLIPBOARD 0x0307
#define WM_DRAWCLIPBOARD    0x0308
#define WM_PAINTCLIPBOARD   0x0309
#define WM_VSCROLLCLIPBOARD 0x030a
#define WM_SIZECLIPBOARD    0x030b
#define WM_ASKCBFORMATNAME  0x030c
#define WM_CHANGECBCHAIN    0x030d
#define WM_HSCROLLCLIPBOARD 0x030e

/* 0x03f0 to 0x03ff are reserved */
/* private window messages start here */
#define WM_USER 	    0x0400

/* Size message commands */
#define SIZENORMAL	0
#define SIZEICONIC	1
#define SIZEFULLSCREEN	2
#define SIZEZOOMSHOW	3
#define SIZEZOOMHIDE	4
#endif

/* Key state masks for mouse messages */
#ifndef NOKEYSTATE
#define MK_LBUTTON	0x0001
#define MK_RBUTTON	0x0002
#define MK_SHIFT	0x0004
#define MK_CONTROL	0x0008
#define MK_MBUTTON	0x0010
#endif

#ifndef NOWINSTYLES
/* Window styles */
#define WS_TILED	0x00000000L
#define WS_OVERLAPPED	WS_TILED
#define WS_ICONICPOPUP	0xc0000000L
#define WS_POPUP	0x80000000L
#define WS_CHILD	0x40000000L
#define WS_MINIMIZE	0x20000000L
#define WS_VISIBLE	0x10000000L
#define WS_DISABLED	0x08000000L
#define WS_CLIPSIBLINGS 0x04000000L
#define WS_CLIPCHILDREN 0x02000000L
#define WS_MAXIMIZE	0x01000000L

#define WS_BORDER	0x00800000L
#define WS_CAPTION	0x00c00000L
#define WS_DLGFRAME	0x00400000L
#define WS_VSCROLL	0x00200000L
#define WS_HSCROLL	0x00100000L
#define WS_SYSMENU	0x00080000L
#define WS_SIZEBOX	0x00040000L
#define WS_THICKFRAME   0x00040000L
#define WS_GROUP	0x00020000L
#define WS_TABSTOP	0x00010000L

#define WS_MINIMIZEBOX  0x00020000L
#define WS_MAXIMIZEBOX  0x00010000L

#define WS_ICONIC	WS_MINIMIZE

/* Class styles */
#define CS_VREDRAW	0x0001
#define CS_HREDRAW	0x0002
#define CS_KEYCVTWINDOW 0x0004
#define CS_DBLCLKS	0x0008
#define CS_OEMCHARS	0x0010
#define CS_OWNDC	0x0020
#define CS_CLASSDC	0x0040
#define CS_PARENTDC     0x0080
#define CS_NOKEYCVT	0x0100
#define CS_SAVEBITS     0x0800
#define CS_NOCLOSE      0x0200
#define CS_BYTEALIGNCLIENT 0x1000
#define CS_BYTEALIGNWINDOW 0x2000

/* Shorthand for the common cases */
#define WS_TILEDWINDOW   (WS_TILED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
#define WS_OVERLAPPEDWINDOW  WS_TILEDWINDOW
#define WS_POPUPWINDOW	 (WS_POPUP | WS_BORDER | WS_SYSMENU)
#define WS_CHILDWINDOW	 (WS_CHILD)
#endif

/* clipboard metafile picture structure */
#ifndef NOMETAFILE

typedef struct tagHANDLETABLE {
	HANDLE  objectHandle[1];
} HANDLETABLE;
typedef HANDLETABLE	*PHANDLETABLE;
typedef HANDLETABLE FAR *LPHANDLETABLE;

typedef struct tagMETARECORD {
	DWORD rdSize;
	WORD  rdFunction;
	WORD  rdParm[1];
} METARECORD;
typedef METARECORD	  *PMETARECORD;
typedef METARECORD    FAR *LPMETARECORD;

typedef struct tagMETAFILEPICT {
	int mm;
	int xExt, yExt;
	HANDLE hMF;
} METAFILEPICT;
typedef METAFILEPICT FAR *LPMETAFILEPICT;
#endif

#ifndef NOCLIPBOARD
/* predefined clipboard formats */
#define CF_TEXT 	1
#define CF_BITMAP	2
#define CF_METAFILEPICT 3
#define CF_SYLK 	4
#define CF_DIF		5
#define CF_TIFF 	6
#define CF_OEMTEXT      7
#define CF_DIB		8

#define CF_OWNERDISPLAY     0x80       /* owner display */
#define CF_DSPTEXT	    0x81       /* display text */
#define CF_DSPBITMAP	    0x82       /* display bitmap */
#define CF_DSPMETAFILEPICT  0x83       /* display metafile */

/* Private clipboard format range */
#define CF_PRIVATEFIRST 0x200	    /* Anything in this range doesn't */
#define CF_PRIVATELAST	0x2ff	    /* get GlobalFree'd */
#define CF_GDIOBJFIRST	0x300	    /* Anything in this range gets */
#define CF_GDIOBJLAST	0x3ff	    /* DeleteObject'ed */
#endif

#ifndef NOHDC
#ifndef NORECT
typedef struct tagPAINTSTRUCT {
	HDC hdc;
	BOOL fErase;
	RECT rcPaint;
	BOOL fRestore;
	BOOL fIncUpdate;
	BYTE rgbReserved[16];
} PAINTSTRUCT;
typedef PAINTSTRUCT *PPAINTSTRUCT;
typedef PAINTSTRUCT NEAR *NPPAINTSTRUCT;
typedef PAINTSTRUCT FAR *LPPAINTSTRUCT;
#endif
#endif

#ifndef NOCREATESTRUCT
typedef struct tagCREATESTRUCT {
	LPSTR lpCreateParams;
	HANDLE hInstance;
	HANDLE hMenu;
	HWND hwndParent;
	int cy;
	int cx;
	int y;
	int x;
	long style;
	LPSTR lpszName;
	LPSTR lpszClass;
} CREATESTRUCT;
typedef CREATESTRUCT FAR *LPCREATESTRUCT;
#endif

#ifndef NOTEXTMETRIC
/* TextMetric structure */
typedef struct tagTEXTMETRIC {
	short int tmHeight;
	short int tmAscent;
	short int tmDescent;
	short int tmInternalLeading;
	short int tmExternalLeading;
	short int tmAveCharWidth;
	short int tmMaxCharWidth;
	short int tmWeight;
	BYTE      tmItalic;
	BYTE      tmUnderlined;
	BYTE      tmStruckOut;
	BYTE      tmFirstChar;
	BYTE      tmLastChar;
	BYTE      tmDefaultChar;
	BYTE      tmBreakChar;
	BYTE      tmPitchAndFamily;
	BYTE      tmCharSet;
	short int tmOverhang;
	short int tmDigitizedAspectX;
	short int tmDigitizedAspectY;
} TEXTMETRIC;
typedef TEXTMETRIC	*PTEXTMETRIC;
typedef TEXTMETRIC NEAR *NPTEXTMETRIC;
typedef TEXTMETRIC FAR *LPTEXTMETRIC;
#endif

#ifndef NOGDI
/* GDI logical objects */
/* Pel Array */
typedef struct tagPELARRAY {
	short int paXCount;
	short int paYCount;
	short int paXExt;
	short int paYExt;
	BYTE      paRGBs;
} PELARRAY;
typedef PELARRAY     *PPELARRAY;
typedef PELARRAY NEAR *NPPELARRAY;
typedef PELARRAY FAR *LPPELARRAY;

/* Logical Brush */
typedef struct tagLOGBRUSH {
	WORD      lbStyle;
	DWORD     lbColor;
	short int lbHatch;
} LOGBRUSH;
typedef LOGBRUSH	*PLOGBRUSH;
typedef LOGBRUSH NEAR	*NPLOGBRUSH;
typedef LOGBRUSH    FAR *LPLOGBRUSH;

/* A PATTERN and a LOGBRUSH are the same thing */
typedef LOGBRUSH     PATTERN;
typedef PATTERN     *PPATTERN;
typedef PATTERN NEAR *NPPATTERN;
typedef PATTERN FAR *LPPATTERN;

#ifndef NOPOINT
/* Logical Pen */
typedef struct tagLOGPEN {
	WORD   lopnStyle;
	POINT  lopnWidth;
	DWORD  lopnColor;
} LOGPEN;
typedef LOGPEN	    *PLOGPEN;
typedef LOGPEN	NEAR *NPLOGPEN;
typedef LOGPEN	FAR *LPLOGPEN;
#endif

/* Logical Font */

#define LF_FACESIZE 32

typedef struct tagLOGFONT {
	short int lfHeight;
	short int lfWidth;
	short int lfEscapement;
	short int lfOrientation;
	short int lfWeight;
	BYTE      lfItalic;
	BYTE      lfUnderline;
	BYTE      lfStrikeOut;
	BYTE      lfCharSet;
	BYTE      lfOutPrecision;
	BYTE      lfClipPrecision;
	BYTE      lfQuality;
	BYTE      lfPitchAndFamily;
	BYTE      lfFaceName[LF_FACESIZE];
} LOGFONT;
typedef LOGFONT     *PLOGFONT;
typedef LOGFONT NEAR *NPLOGFONT;
typedef LOGFONT FAR *LPLOGFONT;


/* Logical font constants */

#define OUT_DEFAULT_PRECIS    0
#define OUT_STRING_PRECIS     1
#define OUT_CHARACTER_PRECIS  2
#define OUT_STROKE_PRECIS     3

#define CLIP_DEFAULT_PRECIS   0
#define CLIP_CHARACTER_PRECIS 1
#define CLIP_STROKE_PRECIS    2

#define DEFAULT_QUALITY       0
#define DRAFT_QUALITY	      1
#define PROOF_QUALITY	      2

#define DEFAULT_PITCH	      0
#define FIXED_PITCH	      1
#define VARIABLE_PITCH	      2

#define ANSI_CHARSET	      0
#define SHIFTJIS_CHARSET      128	    /* Kanji CharSet */
#define OEM_CHARSET	      255

/* GDI font families. */
#define FF_DONTCARE	(0<<4)	/* Don't care or don't know. */
#define FF_ROMAN	(1<<4)	/* Variable stroke width, serifed. */
				/* Times Roman, Century Schoolbook, etc. */
#define FF_SWISS	(2<<4)	/* Variable stroke width, sans-serifed. */
				/* Helvetica, Swiss, etc. */
#define FF_MODERN	(3<<4)	/* Constant stroke width, serifed or sans-serifed. */
				/* Pica, Elite, Courier, etc. */
#define FF_SCRIPT	(4<<4)	/* Cursive, etc. */
#define FF_DECORATIVE	(5<<4)	/* Old English, etc. */

/* Font weights lightest to darkest. */
#define FW_DONTCARE	0
#define FW_THIN 	100
#define FW_EXTRALIGHT	200
#define FW_LIGHT	300
#define FW_NORMAL	400
#define FW_MEDIUM	500
#define FW_SEMIBOLD	600
#define FW_BOLD 	700
#define FW_EXTRABOLD	800
#define FW_HEAVY	900

#define FW_ULTRALIGHT	FW_EXTRALIGHT
#define FW_REGULAR	FW_NORMAL
#define FW_DEMIBOLD	FW_SEMIBOLD
#define FW_ULTRABOLD	FW_EXTRABOLD
#define FW_BLACK	FW_HEAVY


/* EnumFonts masks. */
#define RASTER_FONTTYPE 0x0001
#define DEVICE_FONTTYPE 0X0002


/* GDI rgb values packed into a dword */

#define RGB(r,g,b)          ((DWORD)(((BYTE)(r)|((WORD)(g)<<8))|(((DWORD)(BYTE)(b))<<16)))
#define GetRValue(rgb) ((BYTE)(rgb))
#define GetGValue(rgb) ((BYTE)(((WORD)(rgb)) >> 8))
#define GetBValue(rgb) ((BYTE)((rgb)>>16))

/* GDI Background Modes */

#define TRANSPARENT    1
#define OPAQUE	       2

/* GDI map modes */
#define MM_TEXT 	    1
#define MM_LOMETRIC	    2
#define MM_HIMETRIC	    3
#define MM_LOENGLISH	    4
#define MM_HIENGLISH	    5
#define MM_TWIPS	    6
#define MM_ISOTROPIC	    7
#define MM_ANISOTROPIC	    8

/* GDI coordinate modes */

#define ABSOLUTE	    1
#define RELATIVE	    2

/* Stock Logical Objects */
#define WHITE_BRUSH	    0
#define LTGRAY_BRUSH	    1
#define GRAY_BRUSH	    2
#define DKGRAY_BRUSH	    3
#define BLACK_BRUSH	    4
#define NULL_BRUSH	    5
#define HOLLOW_BRUSH	    NULL_BRUSH
#define WHITE_PEN	    6
#define BLACK_PEN	    7
#define NULL_PEN	    8
#define OEM_FIXED_FONT	   10
#define ANSI_FIXED_FONT    11
#define ANSI_VAR_FONT	   12
#define SYSTEM_FONT	   13
#define DEVICEDEFAULT_FONT 14

/* GDI Brush Style definitions. */

#define BS_SOLID	    0
#define BS_NULL 	    1
#define BS_HOLLOW	    BS_NULL
#define BS_HATCHED	    2
#define BS_PATTERN	    3
#define BS_INDEXED	    4

/* GDI Hatch Style definitions. */

#define HS_HORIZONTAL	    0	    /* ----- */
#define HS_VERTICAL	    1	    /* ||||| */
#define HS_FDIAGONAL	    2	    /* ///// */
#define HS_BDIAGONAL	    3	    /* \\\\\ */
#define HS_CROSS	    4	    /* +++++ */
#define HS_DIAGCROSS	    5	    /* xxxxx */


/* GDI Pen Style definitions */
#define PS_SOLID	    0	    /* solid pen */
#define PS_DASH 	    1	    /* -------	*/
#define PS_DOT		    2	    /* .......	*/
#define PS_DASHDOT	    3	    /* _._._._	*/
#define PS_DASHDOTDOT	    4	    /* _.._.._	*/
#define PS_NULL 	    5	    /*		*/

/* Device Parameters for GetDeviceCaps() */

#define DRIVERVERSION 0     /*	Device driver version		      */
#define TECHNOLOGY    2     /*	Device classification		      */
#define HORZSIZE      4     /*	Horizontal size in millimeters	      */
#define VERTSIZE      6     /*	Vertical   size in millimeters	      */
#define HORZRES       8     /*	Horizontal width in pixels	      */
#define VERTRES       10    /*	Vertical   width in pixels	      */
#define BITSPIXEL     12    /*	Number of bits per pixel	      */
#define PLANES	      14    /*	Number of planes		      */
#define NUMBRUSHES    16    /*	Number of brushes the device has      */
#define NUMPENS       18    /*	Number of pens the device has	      */
#define NUMMARKERS    20    /*	Number of markers the device has      */
#define NUMFONTS      22    /*	Number of fonts the device has	      */
#define NUMCOLORS     24
#define PDEVICESIZE   26    /*	Size required for device descriptor   */
#define CURVECAPS     28    /*	Curves	  capabilities		      */
#define LINECAPS      30    /*	Line	  capabilities		      */
#define POLYGONALCAPS 32    /*	Polygonal capabilities		      */
#define TEXTCAPS      34    /*	Text	  capabilities		      */
#define CLIPCAPS      36    /*	Clipping  capabilities		      */
#define RASTERCAPS    38    /*	Bitblt	  capabilities		      */
#define ASPECTX       40    /*	Length of the X leg		      */
#define ASPECTY       42    /*	Length of the Y leg		      */
#define ASPECTXY      44    /*	Length of the hypotenuse	      */

#define LOGPIXELSX    88    /*	Logical pixels/inch in X	      */
#define LOGPIXELSY    90    /*	Logical pixels/inch in Y	      */

#ifndef NOGDICAPMASKS

/* Device capability masks */
/*   Device Technologies */

#define DT_PLOTTER	0   /* Vector plotter	       */
#define DT_RASDISPLAY	1   /* Raster display	       */
#define DT_RASPRINTER	2   /* Raster printer	       */
#define DT_RASCAMERA	3   /* Raster camera	       */
#define DT_CHARSTREAM	4   /* Character-stream, PLP   */
#define DT_METAFILE	5   /* Metafile, VDM	       */
#define DT_DISPFILE	6   /* Display-file	       */

/*   Curve Capabilities */

#define CC_NONE 	0   /* Curves not supported    */
#define CC_CIRCLES	1   /* Can do circles	       */
#define CC_PIE		2   /* Can do pie wedges       */
#define CC_CHORD	4   /* Can do chord arcs       */
#define CC_ELLIPSES	8   /* Can do ellipese	       */
#define CC_WIDE 	16  /* Can do wide lines       */
#define CC_STYLED	32  /* Can do styled lines     */
#define CC_WIDESTYLED	64  /* Can do wide styled lines*/
#define CC_INTERIORS	128 /* Can do interiors        */

/*   Line Capabilities */

#define LC_NONE 	0   /* Lines not supported     */
#define LC_POLYLINE	2   /* Can do polylines        */
#define LC_MARKER	4   /* Can do markers	       */
#define LC_POLYMARKER	8   /* Can do polymarkers      */
#define LC_WIDE 	16  /* Can do wide lines       */
#define LC_STYLED	32   /* Can do styled lines	*/
#define LC_WIDESTYLED	64   /* Can do wide styled lines*/
#define LC_INTERIORS	128  /* Can do interiors	*/

/*   Polygonal Capabilities */

#define PC_NONE 	0   /* Polygonals not supported*/
#define PC_POLYGON	1   /* Can do polygons	       */
#define PC_RECTANGLE	2   /* Can do rectangles       */
#define PC_TRAPEZOID	4   /* Can do trapezoids       */
#define PC_SCANLINE	8   /* Can do scanlines        */
#define PC_WIDE 	16  /* Can do wide borders     */
#define PC_STYLED	32   /* Can do styled borders	*/
#define PC_WIDESTYLED	64   /* Can do wide styled borders*/
#define PC_INTERIORS	128  /* Can do interiors	*/

/*   Polygonal Capabilities */

#define CP_NONE 	0   /* no clipping of Output   */
#define CP_RECTANGLE	1   /* Output clipped to Rects */

/*   Text Capabilities */

#define TC_OP_CHARACTER 0x0001	/* Can do OutputPrecision   CHARACTER	   */
#define TC_OP_STROKE	0x0002	/* Can do OutputPrecision   STROKE	   */
#define TC_CP_STROKE	0x0004	/* Can do ClipPrecision     STROKE	   */
#define TC_CR_90	0x0008	/* Can do CharRotAbility    90		   */
#define TC_CR_ANY	0x0010	/* Can do CharRotAbility    ANY 	   */
#define TC_SF_X_YINDEP	0x0020	/* Can do ScaleFreedom	    X_YINDEPENDENT */
#define TC_SA_DOUBLE	0x0040	/* Can do ScaleAbility	    DOUBLE	   */
#define TC_SA_INTEGER	0x0080	/* Can do ScaleAbility	    INTEGER	   */
#define TC_SA_CONTIN	0x0100	/* Can do ScaleAbility	    CONTINUOUS	   */
#define TC_EA_DOUBLE	0x0200	/* Can do EmboldenAbility   DOUBLE	   */
#define TC_IA_ABLE	0x0400	/* Can do ItalisizeAbility  ABLE	   */
#define TC_UA_ABLE	0x0800	/* Can do UnderlineAbility  ABLE	   */
#define TC_SO_ABLE	0x1000	/* Can do StrikeOutAbility  ABLE	   */
#define TC_RA_ABLE	0x2000	/* Can do RasterFontAble    ABLE	   */
#define TC_VA_ABLE	0x4000	/* Can do VectorFontAble    ABLE	   */
#define TC_RESERVED	0x8000	/* Reserved.				   */
#endif

/*   Raster Capabilities */

#define RC_BITBLT	1   /* Can do standard non-stretching, non-inverting BLT. */
#define RC_BANDING	2   /* Device requires banding support			  */
#define RC_SCALING	4   /* Device requires scaling support */

#endif


#ifndef NOMSG

/* PeekMessage options */
#define PM_REMOVE   TRUE
#define PM_NOREMOVE FALSE
#define PM_NOYIELD  0x02
#endif

#define     CW_USEDEFAULT	0x8000	    /*used on both x and cx */

/* SetWindowPos flags */

#define SWP_NOSIZE     0x01
#define SWP_NOMOVE     0x02
#define SWP_NOZORDER   0x04
#define SWP_NOREDRAW   0x08
#define SWP_NOACTIVATE 0x10
#define SWP_DRAWFRAME  0x20
#define SWP_SHOWWINDOW	0x40
#define SWP_HIDEWINDOW	0x80
#define SWP_NOCOPYBITS  0x0100

#ifndef NODRAWFRAME
#ifndef NORECT
#ifndef NOHDC

/* DrawFrame and associated defines */
#define DF_SHIFT0           0x0000
#define DF_SHIFT1           0x0001
#define DF_SHIFT2           0x0002
#define DF_SHIFT3           0x0003
#define DF_PATCOPY          0x0000
#define DF_PATINVERT        0x0004

#define DF_SCROLLBAR        (COLOR_SCROLLBAR << 3)
#define DF_BACKGROUND       (COLOR_BACKGROUND << 3)
#define DF_ACTIVECAPTION    (COLOR_ACTIVECAPTION << 3)
#define DF_INACTIVECAPTION  (COLOR_INACTIVECAPTION << 3)
#define DF_MENU             (COLOR_MENU << 3)
#define DF_WINDOW           (COLOR_WINDOW << 3)
#define DF_WINDOWFRAME      (COLOR_WINDOWFRAME << 3)
#define DF_MENUTEXT         (COLOR_MENUTEXT << 3)
#define DF_WINDOWTEXT       (COLOR_WINDOWTEXT << 3)
#define DF_CAPTIONTEXT      (COLOR_CAPTIONTEXT << 3)
#define DF_ACTIVEBORDER     (COLOR_ACTIVEBORDER << 3)
#define DF_INACTIVEBORDER   (COLOR_INACTIVEBORDER << 3)
#define DF_APPWORKSPACE     (COLOR_APPWORKSPACE << 3)
#define DF_GRAY             (DF_APPWORKSPACE + (1 << 3))

#endif
#endif
#endif


/* DrawText format flags */
#ifndef NODRAWTEXT
#define DT_LEFT 	   0x00
#define DT_CENTER	   0x01
#define DT_RIGHT	   0x02
#define DT_TOP		   0x00
#define DT_VCENTER	   0x04
#define DT_BOTTOM	   0x08
#define DT_WORDBREAK	   0x10
#define DT_SINGLELINE	   0x20
#define DT_EXPANDTABS	   0x40
#define DT_TABSTOP	   0x80
#define DT_NOCLIP	   0x100
#define DT_EXTERNALLEADING 0x200
#define DT_CALCRECT	   0x400
#define DT_NOPREFIX	   0x800

#endif

/* Windows Functions */

#ifndef NOSYSMETRICS

/* GetSystemMetrics codes */
#define SM_CXSCREEN	   0
#define SM_CYSCREEN	   1
#define SM_CXVSCROLL	   2
#define SM_CYHSCROLL	   3
#define SM_CYCAPTION	   4
#define SM_CXBORDER	   5
#define SM_CYBORDER	   6
#define SM_CXDLGFRAME	   7
#define SM_CYDLGFRAME	   8
#define SM_CYVTHUMB	   9
#define SM_CXHTHUMB	  10
#define SM_CXICON	  11
#define SM_CYICON	  12
#define SM_CXCURSOR	  13
#define SM_CYCURSOR	  14
#define SM_CYMENU	  15
#define SM_CXFULLSCREEN   16
#define SM_CYFULLSCREEN   17
#define SM_CYKANJIWINDOW  18
#define SM_MOUSEPRESENT   19
#define SM_CYVSCROLL	  20
#define SM_CXHSCROLL	  21
#define SM_DEBUG	  22
#define SM_SWAPBUTTON	  23
#define SM_FULLSCREEN	  24
#define SM_CURSORLEVEL	  25
#define SM_CXICONSLOT	  26	    /* new additions since 2.0 */
#define SM_CYICONSLOT	  27
#define SM_CXMIN	  28
#define SM_CYMIN	  29
#define SM_CXSIZE	  30
#define SM_CYSIZE	  31
#define SM_CXFRAME        32
#define SM_CYFRAME        33
#define SM_CXMINTRACK     34
#define SM_CYMINTRACK     35
#define SM_CMETRICS       36

#endif

/* MessageBox type flags */
#ifndef NOMB
#define MB_OK		    0x0000
#define MB_OKCANCEL	    0x0001
#define MB_ABORTRETRYIGNORE 0x0002
#define MB_YESNOCANCEL	    0x0003
#define MB_YESNO	    0x0004
#define MB_RETRYCANCEL	    0x0005

#define MB_ICONHAND	    0x0010
#define MB_ICONQUESTION     0x0020
#define MB_ICONEXCLAMATION  0x0030
#define MB_ICONASTERISK     0x0040

#define MB_DEFBUTTON1	    0x0000
#define MB_DEFBUTTON2	    0x0100
#define MB_DEFBUTTON3	    0x0200

#define MB_APPLMODAL	    0x0000
#define MB_SYSTEMMODAL	    0x1000
#define MB_NOFOCUS	    0x8000
#define MB_MISCMASK	    0xc000
#define MB_TYPEMASK	    0x000f
#define MB_ICONMASK	    0x00f0
#define MB_DEFMASK	    0x0f00
#define MB_MODEMASK	    0x3000

#endif




/* color type indices		*/
#ifndef NOCOLOR
/* for the WM_CTLCOLOR message */
#define CTLCOLOR_MSGBOX 	0
#define CTLCOLOR_EDIT		1
#define CTLCOLOR_LISTBOX	2
#define CTLCOLOR_BTN		3
#define CTLCOLOR_DLG		4
#define CTLCOLOR_SCROLLBAR	5
#define CTLCOLOR_STATIC 	6
#define CTLCOLOR_MAX		8     /* three bits max */

#define COLOR_SCROLLBAR 	0
#define COLOR_BACKGROUND	1
#define COLOR_ACTIVECAPTION	2
#define COLOR_INACTIVECAPTION	3
#define COLOR_MENU		4
#define COLOR_WINDOW		5
#define COLOR_WINDOWFRAME	6
#define COLOR_MENUTEXT		7
#define COLOR_WINDOWTEXT	8
#define COLOR_CAPTIONTEXT	9
#define COLOR_ACTIVEBORDER	10
#define COLOR_INACTIVEBORDER	11
#define COLOR_APPWORKSPACE      12
#ifdef WIN23
#define COLOR_HIGHLIGHT         13
#define COLOR_HIGHLIGHTTEXT     14
#define COLOR_BTNFACE           15
#define COLOR_BTNSHADOW         16
#define COLOR_GRAYTEXT          17
#define COLOR_BTNTEXT           18
#endif /* WIN23 */
#endif

#define CP_GETBEEP      1
#define CP_SETBEEP      2
#define CP_GETMOUSE     3
#define CP_SETMOUSE     4
#define CP_GETBORDER    5
#define CP_SETBORDER    6
#define CP_TIMEOUTS     7
#define CP_KANJIMENU	8



#ifndef NOOPENFILE

typedef struct tagOFSTRUCT {
	BYTE    cBytes;		    /* length of structure */
	BYTE    fFixedDisk; 	    /* non-zero if file located on non- */
					/* removeable media */
	WORD    nErrCode;		    /* DOS error code if OpenFile fails */
	BYTE    reserved[4];
	BYTE    szPathName[120];
} OFSTRUCT;
typedef OFSTRUCT      *POFSTRUCT;
typedef OFSTRUCT NEAR *NPOFSTRUCT;
typedef OFSTRUCT FAR  *LPOFSTRUCT;



/* Flags for GetTempFileName */

#define TF_FORCEDRIVE	(BYTE)0x80  /* Forces use of current dir of passed */
					/* drive */
/* Flags for OpenFile */

#define OF_REOPEN 0x8000
#define OF_EXIST  0x4000
#define OF_PROMPT 0x2000
#define OF_CREATE 0x1000
#define OF_CANCEL 0x0800
#define OF_VERIFY 0x0400
#define OF_DELETE 0x0200
#define OF_PARSE  0x0100

#define OF_READ      0
#define OF_WRITE     1
#define OF_READWRITE 2
#endif

#ifndef NOMEMMGR
/* Interface to global memory manager */
#define GMEM_FIXED	    0x0000
#define GMEM_MOVEABLE	    0x0002
#define GMEM_NOCOMPACT	    0x0010
#define GMEM_NODISCARD	    0x0020
#define GMEM_ZEROINIT	    0x0040
#define GMEM_MODIFY	    0x0080
#define GMEM_DISCARDABLE    0x0F00
#define GHND	(GMEM_MOVEABLE | GMEM_ZEROINIT)
#define GPTR	(GMEM_FIXED    | GMEM_ZEROINIT)
#define GMEM_SHARE          0x2000
#define GMEM_DDESHARE       0x2000
#define GMEM_LOWER          0x1000
#define GMEM_NOT_BANKED     0x1000

#define GlobalDiscard( h ) GlobalReAlloc( h, 0L, GMEM_MOVEABLE )

/* Flags returned by GlobalFlags (in addition to GMEM_DISCARDABLE) */
#define GMEM_DISCARDED	    0x4000
#define GMEM_SWAPPED	    0x8000
#define GMEM_LOCKCOUNT	    0x00FF

#define LockData( dummy )   LockSegment( 0xFFFF )
#define UnlockData( dummy ) UnlockSegment( 0xFFFF )

/* Interface to local memory manager */

#define LMEM_FIXED	    0x0000
#define LMEM_MOVEABLE	    0x0002
#define LMEM_NOCOMPACT	    0x0010
#define LMEM_NODISCARD	    0x0020
#define LMEM_ZEROINIT	    0x0040
#define LMEM_MODIFY	    0x0080
#define LMEM_DISCARDABLE    0x0F00
#define LHND		(LMEM_MOVEABLE | LMEM_ZEROINIT)
#define LPTR		(LMEM_FIXED    | LMEM_ZEROINIT)
#define NONZEROLHND	(LMEM_MOVEABLE)
#define NONZEROLPTR	(LMEM_FIXED)

#define LNOTIFY_OUTOFMEM 0
#define LNOTIFY_MOVE	 1
#define LNOTIFY_DISCARD  2

#define LocalDiscard( h ) LocalReAlloc( h, 0, LMEM_MOVEABLE )

/* !!!!! -- pLocalHeap : changed to extern type for CS */
extern WORD    NEAR * PASCAL pLocalHeap;

#define LocalFreeze( dummy ) ( *(pLocalHeap+1) += 1 )
#define LocalHandleDelta( delta ) ( (delta) ? (*(pLocalHeap+9) = (delta)) : *(pLocalHeap+9))
#define LocalMelt( dummy )   ( *(pLocalHeap+1) -= 1 )
/* Flags returned by LocalFlags (in addition to LMEM_DISCARDABLE) */
#define LMEM_DISCARDED	    0x4000
#define LMEM_LOCKCOUNT	    0x00FF
#endif


/* Interface to the resource manager */





#define MAKEINTRESOURCE(i)  (LPSTR)((DWORD)((WORD)(i)))

#ifndef NORESOURCE
/* Predefined resource types */
#define RT_CURSOR	MAKEINTRESOURCE( 1 )
#define RT_BITMAP	MAKEINTRESOURCE( 2 )
#define RT_ICON 	MAKEINTRESOURCE( 3 )
#define RT_MENU 	MAKEINTRESOURCE( 4 )
#define RT_DIALOG	MAKEINTRESOURCE( 5 )
#define RT_STRING	MAKEINTRESOURCE( 6 )
#define RT_FONTDIR	MAKEINTRESOURCE( 7 )
#define RT_FONT 	MAKEINTRESOURCE( 8 )
#define RT_ACCELERATOR	MAKEINTRESOURCE( 9 )
#define RT_RCDATA       MAKEINTRESOURCE( 10 )
#endif

/* Interface to the task scheduler */


/* Interface to the atom manager */

#ifndef NOATOM
typedef WORD ATOM;
#endif


#ifndef NOATOM
#define MAKEINTATOM(i)	(LPSTR)((DWORD)((WORD)(i)))
#endif

typedef int CATCHBUF[ 9 ];
typedef int FAR *LPCATCHBUF;


/* GetWindow() and constants */
#define GW_HWNDFIRST 0
#define GW_HWNDLAST  1
#define GW_HWNDNEXT  2
#define GW_HWNDPREV  3
#define GW_OWNER     4
#define GW_CHILD     5

/* Key conversion window */
#ifndef NOMENUS

/* Menu flags for Add/Check/EnableMenuItem */
#define MF_CHANGE	0x0080
#define MF_INSERT	0x0000
#define MF_APPEND	0x0100
#define MF_DELETE	0x0200
#define MF_BYPOSITION	0x0400
#define MF_SEPARATOR	0x0800
#define MF_REMOVE	0x1000
#define MF_BYCOMMAND	0x0000
#define MF_GRAYED	0x0001
#define MF_DISABLED	0x0002
#define MF_ENABLED	0x0000
#define MF_CHECKED	0x0008
#define MF_UNCHECKED	0x0000
#define MF_BITMAP	0x0004
#define MF_STRING	0x0000
#define MF_POPUP	0x0010
#define MF_MENUBARBREAK 0x0020
#define MF_MENUBREAK	0x0040
#define MF_HILITE	0x0080
#define MF_UNHILITE	0x0000
#define MF_HELP         0x4000
#define MF_SYSMENU      0x2000
#define MF_MOUSESELECT  0x8000

#endif /* of NOMENU */

/* System Menu Command Values */
#ifndef NOSYSCOMMANDS
#define SC_SIZE 	0xf000
#define SC_MOVE 	0xf010
#define SC_MINIMIZE	0xf020
#define SC_MAXIMIZE	0xf030
#define SC_NEXTWINDOW	0xf040
#define SC_PREVWINDOW	0xf050
#define SC_CLOSE	0xf060
#define SC_VSCROLL	0xf070
#define SC_HSCROLL	0xf080
#define SC_MOUSEMENU	0xf090
#define SC_KEYMENU	0xf100
#define SC_ARRANGE	0xf110
#define SC_RESTORE	0xf120
#define SC_ICON 	SC_MINIMIZE
#define SC_ZOOM 	SC_MAXIMIZE
#endif

/* Standard cursor IDs */
#define IDC_ARROW	MAKEINTRESOURCE(32512)
#define IDC_IBEAM	MAKEINTRESOURCE(32513)
#define IDC_WAIT	MAKEINTRESOURCE(32514)
#define IDC_CROSS	MAKEINTRESOURCE(32515)
#define IDC_UPARROW	MAKEINTRESOURCE(32516)
#define IDC_SIZE	MAKEINTRESOURCE(32640)
#define IDC_ICON	MAKEINTRESOURCE(32641)



#ifndef NOICON
/* Standard icon IDs */
#define IDI_APPLICATION MAKEINTRESOURCE(32512)
#define IDI_HAND	MAKEINTRESOURCE(32513)
#define IDI_QUESTION	MAKEINTRESOURCE(32514)
#define IDI_EXCLAMATION MAKEINTRESOURCE(32515)
#define IDI_ASTERISK	MAKEINTRESOURCE(32516)
#endif


#ifndef NOKANJI

#define CP_HWND 		0
#define CP_OPEN 		1
#define CP_DIRECT		2

/* VK from the keyboard driver */
#define VK_KANA 	0x15
#define VK_ROMAJI	0x16
#define VK_ZENKAKU	0x17
#define VK_HIRAGANA	0x18
#define VK_KANJI	0x19

/* VK to send to Applications */
#define VK_CONVERT	0x1C
#define VK_NONCONVERT	0x1D
#define VK_ACCEPT	0x1E
#define VK_MODECHANGE	0x1F


/* Conversion function numbers */
#define KNJ_START	0x01
#define KNJ_END 	0x02
#define KNJ_QUERY	0x03

#define KNJ_LEARN_MODE	0x10
#define KNJ_GETMODE	0x11
#define KNJ_SETMODE	0x12

#define KNJ_CODECONVERT 0x20
#define KNJ_CONVERT	0x21
#define KNJ_NEXT	0x22
#define KNJ_PREVIOUS	0x23
#define KNJ_ACCEPT	0x24

#define KNJ_LEARN	0x30
#define KNJ_REGISTER	0x31
#define KNJ_REMOVE	0x32
#define KNJ_CHANGE_UDIC 0x33

/* note: DEFAULT	= 0
		JIS1		= 1
		JIS2		= 2
		SJIS2		= 3
		JIS1KATAKANA	= 4
		SJIS2HIRAGANA	= 5
		SJIS2KATAKANA	= 6
		OEM		= f
*/

#define KNJ_JIS1toJIS1KATAKANA	0x14
#define KNJ_JIS1toSJIS2 	0x13
#define KNJ_JIS1toSJIS2HIRAGANA 0x15
#define KNJ_JIS1toSJIS2KATAKANA 0x16
#define KNJ_JIS1toDEFAULT	0x10
#define KNJ_JIS1toSJIS2OEM	0x1F
#define KNJ_JIS2toSJIS2 	0x23
#define KNJ_SJIS2toJIS2 	0x32

/* see KNJ_GETMODE for definition */
#define KNJ_MD_ALPHA		0x01
#define KNJ_MD_HIRAGANA 	0x02
#define KNJ_MD_HALF		0x04
#define KNJ_MD_JIS		0x08
#define KNJ_MD_SPECIAL		0x10

/* conversion modes, low word of lParam when VK_CONVERT is sent to the app */
#define KNJ_CVT_NEXT		0x01
#define KNJ_CVT_PREV		0x02
#define KNJ_CVT_KATAKANA	0x03
#define KNJ_CVT_HIRAGANA	0x04
#define KNJ_CVT_JIS1		0x05
#define KNJ_CVT_SJIS2		0x06
#define KNJ_CVT_DEFAULT 	0x07
#define KNJ_CVT_TYPED		0x08

#define WM_YOMICHAR		0x0108
#define WM_MOVECONVERTWINDOW	0x0109
#define WM_CONVERTREQUEST	0x010A
#define WM_CONVERTRESULT	0x010B

typedef struct
{
	short   fnc;
	short   wParam;
	LPSTR   lpSource;
	LPSTR   lpDest;
	short   wCount;
	LPSTR   lpReserved1;
	LPSTR   lpReserved2;
} KANJISTRUCT, FAR *LPKANJISTRUCT;

#endif

/* Conventional dialog box and message box command IDs */
#define IDOK	  1
#define IDCANCEL  2
#define IDABORT   3
#define IDRETRY   4
#define IDIGNORE  5
#define IDYES	  6
#define IDNO	  7

#ifndef NOCTLMGR

/* Control manager structures & definitions */
/* Edit control class stuff */

/* styles */
#ifndef NOWINSTYLES
#define ES_LEFT 	  0L
#define ES_CENTER	  1L
#define ES_RIGHT	  2L
#define ES_MULTILINE	  4L
#define ES_AUTOVSCROLL	  64L
#define ES_AUTOHSCROLL	  128L
#define ES_NOHIDESEL	  256L
#endif

/* notification codes */
#define EN_SETFOCUS   0x0100
#define EN_KILLFOCUS  0x0200
#define EN_CHANGE     0x0300
#define EN_UPDATE     0x0400
#define EN_ERRSPACE   0x0500
#define EN_HSCROLL    0x0601
#define EN_VSCROLL    0x0602

/* control messages: */
#ifndef NOWINMESSAGES
#define EM_GETSEL	 (WM_USER+0)
#define EM_SETSEL	 (WM_USER+1)
#define EM_GETRECT	 (WM_USER+2)
#define EM_SETRECT	 (WM_USER+3)
#define EM_SETRECTNP	 (WM_USER+4)
#define EM_SCROLL	 (WM_USER+5)
#define EM_LINESCROLL	 (WM_USER+6)
#define EM_GETMODIFY	 (WM_USER+8)
#define EM_SETMODIFY	 (WM_USER+9)
#define EM_GETLINECOUNT  (WM_USER+10)
#define EM_LINEINDEX	 (WM_USER+11)
#define EM_SETHANDLE	 (WM_USER+12)
#define EM_GETHANDLE	 (WM_USER+13)
#define EM_GETTHUMB	 (WM_USER+14)
#define EM_LINELENGTH	 (WM_USER+17)
#define EM_REPLACESEL	 (WM_USER+18)
#define EM_SETFONT	 (WM_USER+19)
#define EM_GETLINE	 (WM_USER+20)
#define EM_LIMITTEXT	 (WM_USER+21)
#define EM_CANUNDO	 (WM_USER+22)
#define EM_UNDO 	 (WM_USER+23)
#define EM_FMTLINES	 (WM_USER+24)
#define EM_LINEFROMCHAR  (WM_USER+25)
#define EM_SETWORDBREAK  (WM_USER+26)
#endif

/* button control styles */
#define BS_PUSHBUTTON	 0L
#define BS_DEFPUSHBUTTON 1L
#define BS_CHECKBOX	 2L
#define BS_AUTOCHECKBOX  3L
#define BS_RADIOBUTTON	 4L
#define BS_3STATE	 5L
#define BS_AUTO3STATE	 6L
#define BS_GROUPBOX	 7L
#define BS_USERBUTTON	 8L
#define BS_AUTORADIOBUTTON 9L
#define BS_PUSHBOX	 10L
#define BS_LEFTTEXT	 0x20L

/* user button notification codes */
#define BN_CLICKED	 0
#define BN_PAINT	 1
#define BN_HILITE	 2
#define BN_UNHILITE	 3
#define BN_DISABLE	 4
#define BN_DOUBLECLICKED 5

/* control messages */
#define BM_GETCHECK	WM_USER+0
#define BM_SETCHECK	WM_USER+1
#define BM_GETSTATE	WM_USER+2
#define BM_SETSTATE	WM_USER+3
#define BM_SETSTYLE	WM_USER+4

/* Static control constants */

#define SS_LEFT       0L
#define SS_CENTER     1L
#define SS_RIGHT      2L
#define SS_ICON       3L
#define SS_BLACKRECT  4L
#define SS_GRAYRECT   5L
#define SS_WHITERECT  6L
#define SS_BLACKFRAME 7L
#define SS_GRAYFRAME  8L
#define SS_WHITEFRAME 9L
#define SS_USERITEM   10L

/* Dialog manager routines */

#ifndef NOCTLMGR

/* Dialog style bits */
#define DS_ABSALIGN   0x000000001L
#define DS_SYSMODAL   0x000000002L
#define DS_LOCALEDIT  0x000000020L  /* Edit items get Local storage. */

#define DM_GETDEFID (WM_USER+0)
#define DM_SETDEFID (WM_USER+1)
#define DC_HASDEFID (0x534b)

/*  Dialog codes (returned by WM_GETDLGCODE message): */

#define DLGC_WANTARROWS 	0x01	/* control wants arrow keys */
#define DLGC_WANTTAB		0x02	/* control wants tab keys */
#define DLGC_WANTALLKEYS	0x04	/* control wants all keys */
#define DLGC_WANTMESSAGE	0x04	/* pass message to control */
#define DLGC_HASSETSEL		0x08	/* understands EM_SETSEL message */
#define DLGC_DEFPUSHBUTTON      0x10    /* Default pushbutton     */
#define DLGC_UNDEFPUSHBUTTON	0x20	/* Non-default pushbutton */
#define DLGC_RADIOBUTTON        0x40    /* radio button           */
#define DLGC_WANTCHARS          0x80    /* Want WM_CHAR messages  */
#define DLGC_STATIC             0x100   /* Static item: don't include */
#define DLGC_BUTTON             0x2000  /* Button item: can be checked */

#define LB_CTLCODE   0L

/* Listbox control return values */
#define LB_OKAY      0
#define LB_ERR	     -1
#define LB_ERRSPACE  -2

/* listbox notification codes */
#define LBN_ERRSPACE	-2
#define LBN_SELCHANGE	1
#define LBN_DBLCLK	2
#endif

/* listbox messages */
#ifndef NOWINMESSAGES
#define LB_ADDSTRING	 (1+WM_USER)
#define LB_INSERTSTRING  (2+WM_USER)
#define LB_DELETESTRING  (3+WM_USER)
#define LB_REPLACESTRING (4+WM_USER)
#define LB_RESETCONTENT  (5+WM_USER)
#define LB_SETSEL	 (6+WM_USER)
#define LB_SETCURSEL	 (7+WM_USER)
#define LB_GETSEL	 (8+WM_USER)
#define LB_GETCURSEL	 (9+WM_USER)
#define LB_GETTEXT	 (10+WM_USER)
#define LB_GETTEXTLEN	 (11+WM_USER)
#define LB_GETCOUNT	 (12+WM_USER)
#define LB_SELECTSTRING  (13+WM_USER)
#define LB_DIR		 (14+WM_USER)
#define LB_MSGMAX	 (15+WM_USER)
#endif

/* listbox style bits */
#ifndef NOWINSTYLES
#define LBS_NOTIFY	  0x0001L
#define LBS_SORT	  0x0002L
#define LBS_NOREDRAW	  0x0004L
#define LBS_MULTIPLESEL   0x0008L
#define LBS_STANDARD	  (LBS_NOTIFY | LBS_SORT | WS_VSCROLL | WS_BORDER)
#endif

/* scroll bar styles */
#ifndef NOWINSTYLES
#define SBS_HORZ		    0x0000L
#define SBS_VERT		    0x0001L
#define SBS_TOPALIGN		    0x0002L
#define SBS_LEFTALIGN		    0x0002L
#define SBS_BOTTOMALIGN 	    0x0004L
#define SBS_RIGHTALIGN		    0x0004L
#define SBS_SIZEBOXTOPLEFTALIGN     0x0002L
#define SBS_SIZEBOXBOTTOMRIGHTALIGN 0x0004L
#define SBS_SIZEBOX		    0x0008L
#endif
#endif

#ifndef NOSOUND

/* constants used to specify return condition for WaitSoundState */

#define QUEUEEMPTY   0
#define THRESHOLD    1
#define ALLTHRESHOLD 2

/* constants used to specify accent mode */

#define S_NORMAL      0
#define S_LEGATO      1
#define S_STACCATO    2

/* constants used to specify source in SetSoundNoise */
#define S_PERIOD512   0   /* freq = N/512 high pitch, less coarse hiss */
#define S_PERIOD1024  1   /* freq = N/1024 */
#define S_PERIOD2048  2   /* freq = N/2048 low pitch, more coarse hiss */
#define S_PERIODVOICE 3   /* source is frequency from voice channel (3) */

#define S_WHITE512    4   /* freq = N/512 high pitch, less coarse hiss */
#define S_WHITE1024   5   /* freq = N/1024 */
#define S_WHITE2048   6   /* freq = N/2048 low pitch, more coarse hiss */
#define S_WHITEVOICE  7   /* source is frequency from voice channel (3) */

#define S_SERDVNA     -1  /* device not available */
#define S_SEROFM      -2  /* out of memory */
#define S_SERMACT     -3  /* music active */
#define S_SERQFUL     -4  /* queue full */
#define S_SERBDNT     -5  /* invalid note */
#define S_SERDLN      -6  /* invalid note length */
#define S_SERDCC      -7  /* invalid note count */
#define S_SERDTP      -8  /* invalid tempo */
#define S_SERDVL      -9  /* invalid volume */
#define S_SERDMD      -10 /* invalid mode */
#define S_SERDSH      -11 /* invalid shape */
#define S_SERDPT      -12 /* invalid pitch */
#define S_SERDFQ      -13 /* invalid frequency */
#define S_SERDDR      -14 /* invalid duration */
#define S_SERDSR      -15 /* invalid source */
#define S_SERDST      -16 /* invalid state */
#endif


#ifndef NOCOMM
/*************************************************************************
**
** dcb field definitions.
**
*************************************************************************/

#define NOPARITY    0
#define ODDPARITY   1
#define EVENPARITY  2
#define MARKPARITY  3
#define SPACEPARITY 4

#define ONESTOPBIT  0
#define ONE5STOPBITS 1
#define TWOSTOPBITS 2

#define IGNORE	    0	    /* Ignore signal		    */
#define INFINITE    0xffff  /* Infinite timeout 	    */



/*************************************************************************
**
** Comm Device Driver Error Bits.
**
*************************************************************************/

#define CE_RXOVER   0x0001  /* Receive Queue overflow	    */
#define CE_OVERRUN  0x0002  /* Receive Overrun Error	    */
#define CE_RXPARITY 0x0004  /* Receive Parity Error	    */
#define CE_FRAME    0x0008  /* Receive Framing error	    */
#define CE_BREAK    0x0010  /* Break Detected		    */
#define CE_CTSTO    0x0020  /* CTS Timeout		    */
#define CE_DSRTO    0x0040  /* DSR Timeout		    */
#define CE_RLSDTO   0x0080  /* RLSD Timeout		    */
#define CE_TXFULL   0x0100  /* TX QUEUE IS FULL 	    */
#define CE_PTO	    0x0200  /* LPTx Timeout		    */
#define CE_IOE	    0x0400  /* LPTx I/O Error		    */
#define CE_DNS	    0x0800  /* LPTx Device not selected     */
#define CE_OOP	    0x1000  /* LPTx Out-Of-Paper	    */
#define CE_MODE     0x8000  /* Requested mode unsupported   */


/*************************************************************************
**
** Initialization Error Codes
**
*************************************************************************/

#define IE_BADID    -1	/* Invalid or unsupported id	*/
#define IE_OPEN     -2	/* Device Already Open		*/
#define IE_NOPEN    -3	/* Device Not Open		*/
#define IE_MEMORY   -4	/* Unable to allocate queues	*/
#define IE_DEFAULT  -5	/* Error in default parameters	*/
#define IE_HARDWARE -10 /* Hardware Not Present 	*/
#define IE_BYTESIZE -11 /* Illegal Byte Size		*/
#define IE_BAUDRATE -12 /* Unsupported BaudRate 	*/


/*************************************************************************
**
** Event Definitions
**
*************************************************************************/

#define EV_RXCHAR   0x0001  /* Any Character received	    */
#define EV_RXFLAG   0x0002  /* Received certain character   */
#define EV_TXEMPTY  0x0004  /* Transmitt Queue Empty	    */
#define EV_CTS	    0x0008  /* CTS changed state	    */
#define EV_DSR	    0x0010  /* DSR changed state	    */
#define EV_RLSD     0x0020  /* RLSD changed state	    */
#define EV_BREAK    0x0040  /* BREAK received		    */
#define EV_ERR	    0x0080  /* Line status error occurred   */
#define EV_RING     0x0100  /* Ring signal detected	    */
#define EV_PERR     0x0200  /* Printer error occured	    */


/*************************************************************************
**
** Escape Functions
**
*************************************************************************/

#define SETXOFF   1 /* Simulate XOFF received	    */
#define SETXON	  2 /* Simulate XON received	    */
#define SETRTS	  3 /* Set RTS high		    */
#define CLRRTS	  4 /* Set RTS low		    */
#define SETDTR	  5 /* Set DTR high		    */
#define CLRDTR	  6 /* Set DTR low		    */
#define RESETDEV  7 /* Reset device if possible     */



/*************************************************************************
**
** Device Descriptor Block Definition
**
*************************************************************************/
/* !!!!! NOTE : for QWINDOWS.H the CS compiler does not allow bit fields
	to be of type BYTE -- here we have changed BYTE bit fields
	into unsigned bit fields */

#define LPTx	0x80			/* Set if ID is for LPT device	*/

typedef struct {
	BYTE Id;		  /* Internal Device ID 	  */
	WORD BaudRate;	  /* Baudrate at which runing	  */
	BYTE ByteSize;	  /* Number of bits/byte, 4-8	  */
	BYTE Parity;	  /* 0-4=None,Odd,Even,Mark,Space */
	BYTE StopBits;	  /* 0,1,2 = 1, 1.5, 2		  */
	WORD RlsTimeout;	  /* Timeout for RLSD to be set   */
	WORD CtsTimeout;	  /* Timeout for CTS to be set	  */
	WORD DsrTimeout;	  /* Timeout for DSR to be set	  */

				
	unsigned fBinary: 1;	/* Binary Mode (skip EOF check	*/
	unsigned fRtsDisable:1;	/* Don't assert RTS at init time*/
	unsigned fParity: 1;	/* Enable parity checking	*/
	unsigned fOutxCtsFlow:1;	/* CTS handshaking on output	*/
	unsigned fOutxDsrFlow:1;	/* DSR handshaking on output	*/
	unsigned fDummy: 2; 	/* Reserved			*/
	unsigned fDtrDisable:1;	/* Don't assert DTR at init time*/

	unsigned fOutX: 1;		/* Enable output X-ON/X-OFF	*/
	unsigned fInX: 1;		/* Enable input X-ON/X-OFF	*/
	unsigned fPeChar: 1;	/* Enable Parity Err Replacement*/
	unsigned fNull: 1;		/* Enable Null stripping	*/
	unsigned fChEvt: 1; 	/* Enable Rx character event.	*/
	unsigned fDtrflow: 1;	/* DTR handshake on input	*/
	unsigned fRtsflow: 1;	/* RTS handshake on input	*/
	unsigned fDummy2: 1;

	char XonChar;	  /* Tx and Rx X-ON character	  */
	char XoffChar;	  /* Tx and Rx X-OFF character	  */
	WORD XonLim;	  /* Transmit X-ON threshold	  */
	WORD XoffLim;	  /* Transmit X-OFF threshold	  */
	char PeChar;	  /* Parity error replacement char*/
	char EofChar;	  /* End of Input character	  */
	char EvtChar;	  /* Recieved Event character	  */
	WORD TxDelay;	  /* Amount of time between chars */
} DCB;

/*************************************************************************
**
** Status record returned by GetCommError
**
*************************************************************************/


/* !!!!! NOTE : for QWINDOWS.H the CS compiler does not allow bit fields
	to be of type BYTE -- here we have changed BYTE bit fields
	into unsigned bit fields */

typedef struct {
	unsigned fCtsHold: 1;	/* Transmit is on CTS hold	*/
	unsigned fDsrHold: 1;	/* Transmit is on DSR hold	*/
	unsigned fRlsdHold: 1;	/* Transmit is on RLSD hold	*/
	unsigned fXoffHold: 1;	/* Received handshake		*/
	unsigned fXoffSent: 1;	/* Issued handshake		*/
	unsigned fEof: 1;		/* End of file character found	*/
	unsigned fTxim: 1;		/* Character being transmitted	*/
	WORD cbInQue;		/* count of characters in Rx Que*/
	WORD cbOutQue;		/* count of characters in Tx Que*/
} COMSTAT;

#endif
