struct BMI  /* Bitmap Info */
		{
		HBITMAP     hbm;        /* Handle to bitmap */
		int         dxp;        /* x-size of bitmap */
		int         dyp;        /* y-size of bitmap */
		int         :1;
		int         fDiscardable:1; /* bitmap may be discarded */
		int         :6;
		int         idcbMax:8; /* # of imbedded bitmaps (incl. gray) */
		int         dxpEach;    /* width of each sub-bitmap in this bitmap */
		};

/* height of Ribbon window in 1/8 char units */
/* Actually, we add one pixel to the height of the ribbon after
calculating it.  this is because of weirdnesses with some screen
resolutions. */
#define dyRibbon    15 
#define ddyIconLH 3
#ifdef WIN23
#define dyRuler2   31    /* height of Ruler window in 1/8 char units */
#define dyRuler3	28	/* height of Ruler under Win 3 */
#else
#define dyRuler   31    /* height of Ruler window in 1/8 char units */
#endif /* WIN23 */

/* following constants relate to the x-coordinate limits; users of these
	must generally be careful to assure that 16-bit overflow of the
	pixel-equivalents is not an issue.  */

/* xa limit: about .5 in less than 16 bits */
#define xaRightMaxRaw (22*dxaInch)
#define xaLeftMinRaw (-xaRightMaxRaw/2)    /* raw value; limit applied in initwin.c */

#define xaMax (xaRightMaxRaw)
#define xaMin (-xaRightMaxRaw)
#define xaNil (xaMin - 1)

/* Twip constants to be stored in pixels for our screen in SCI structure */

/* amount of gray to reveal above or below page when selection scrolling */
#define dyaPastPage 	     720

/* amount of page real estate to display initially when new page is reached
	via selection scrolling */
#define dyaPageReveal	    240

#define dyaMinAveLine       240
#define dyaMacAveLine       600
#define xaSelBar            288
#define dxaMinScroll        1000
#define yaSubSuper          60
#define dyaMinWwInit        60

#define dyaWwMin            720

#define dxaGrayOutside	    720	   /* PAGE VIEW: Gray area outside page */
#define dyaGrayOutside	    720

#define dccmTextColor        0x0001
#define dccmBkgrnd           0x0002
#define dccmTransparent      0x0004
#define dccmHpenBorder       0x0008
#define dccmHpen             0x0010
#define dccmCurFont          0x0020
#define dccmOpaque           0x0100
#define dccmUnlockBrushPen   0x0200 

#define dccmColors      (dccmTextColor | dccmBkgrnd)

#define dccDoc          (dccmColors | dccmOpaque | dccmHpen | dccmCurFont)
#define dccScratch      (dccmOpaque | dccmCurFont)
#define dccRuler        (dccmColors | dccmTransparent)
#define dccPrompt       (dccmColors)
#define dccSEdit        (dccmTextColor | dccmBkgrnd | dccmHpen)
#ifdef WIN23
#define dccStatLine2     (dccmColors | dccmOpaque)
#define dccStatLine3     (dccmTransparent)
#define dccIconBar2      (dccmColors | dccmOpaque)
#define dccIconBar3      (dccmTextColor | dccmOpaque)
#else
#define dccStatLine     (dccmColors | dccmOpaque)
#define dccIconBar      (dccmColors | dccmOpaque)
#endif /* WIN23 */

/* Raster Operations */
#define ROP_S          (DWORD) 0x00CC0020   /* same as SRCCOPY */
#define ROP_Sn         (DWORD) 0x00330008
#define ROP_PSo        (DWORD) 0x00FC008A
#define ROP_PSno       (DWORD) 0x00F3022A
#define ROP_DPSxx      (DWORD) 0x00960169
#define ROP_PDSxxn     (DWORD) 0x00690145
#define ROP_Pn         (DWORD) 0x000f0001
#ifdef WIN23
#define ROP_DPSnao	  (DWORD) 0x00B00E05
#define ROP_DSnx		  (DWORD) 0x00990066
#define ROP_DPo        (DWORD) 0x00FA0089
#endif /* WIN23 */


struct MDCD  /* Memory DC Descriptor */
		{
		HDC             hdc;
		struct BMI      *pbmi;
		};


struct DCIB  /* Display Context Information Block */
		{
		long ropBitBlt;
		long ropErase;
		};


struct SCI  /* Screen Info */
		{

		int        fMonochrome; /* whether we have a monochrome display */
		int        fInvertMonochrome : 1; /* monochrome windows w/white on black */
		int        fUseResCursors : 1; /* cursor resolution not normal */
#ifdef WIN23
		int			fWin3Visuals: 1;	/* Win 3 ruler, ribbon, statline, etc */
		int			fWin3: 1;			/* Win 3 font, dialogs */
		int        :12;
#else
		int        :14;
#endif /* WIN23 */

		long       rgbBkgrnd;   /* COLOR_WINDOW */
		long       rgbText;     /* COLOR_WINDOWTEXT */
		long       rgbBorder;   /* COLOR_WINDOWFRAME */
	WORD	   wSpare1;

		HBRUSH     hbrScrollBar; /* brush for drawing scroll bar */
		HBRUSH     hbrBkgrnd;   /* Background brush */
		HBRUSH     hbrText;     /* handle to text-colored brush */
		HBRUSH     hbrDesktop;  /* brush for desktop background color */
		HBRUSH     hbrBorder;   /* brush for drawing borders */

		HPEN       hpen;        /* Foreground pen, for text etc */
		HPEN       hpenBorder;  /* Pen for drawing borders */

		long       ropErase;    /* raster op for erasing screen */

		union { /* scratchpad memory DC: for screen cache and misc uses */
				struct     MDCD mdcdScratch;    
				struct {
						HDC        hdcScratch;     /* direct access to fields */
						struct BMI *pbmiScratch;
						};
				};

		struct     MDCD mdcdBmp;        /* extra memory DC for resource bitmaps */

		struct DCIB dcibMemDC;  /* Info for Memory DC */
		struct DCIB dcibScreen; /* Info for Screen DC */

		struct FTI *pfti;       /* Ptr to fti struct crntly rep screen */

		int        dxpScreen;   /* Width of display device in pixels */
		int        dypScreen;   /* Height of display device in pixels */
		int        dxpScrlBar;  /* Width of vert scroll bar */
		int        dypScrlBar;  /* Height of horiz scroll bar */
		int        dxpBorder;   /* Width of vert window border */
		int        dypBorder;   /* Height of horiz window border */
		int        dypSplitBox; /* Height of split box */
		int        dypSplitBar; /* Height of split bar */
		int        dypRibbon;   /* Height of ribbon */
		int        dypStatLine; /* Height of status line */
		int        dypRuler;    /* Height of ruler */
		int        dxpTmWidth;  /* Width of system font */
		int        dypTmHeight; /* Height of system font */
		int        dypScrollArrow; /* Height of arrow in vertical scroll bar */
		int        dypTmAscent;
		int        xpScrollMax; /* Largest possible x-scroll offset */
		int        dypComboSmall;  /* size of combo with no list showing */
		int        dxpComboDelta;  /* with change for changed combos */
		int        xaLeftMin;   /* lowest possible xaLeft for FormatLine */
		int        xaRightMax;  /* xaRightMaxRaw, limited to 16-bit pixel equivalent */
		int        dypIconBar; /* height of icon bar */
		int        dxmmScreen;   /* Width of display device in mm */
		int        dymmScreen;   /* Height of display device in mm */
	int	   dxpBmpSystem;/* Width of doc window system menu bitmap */
		int        dypMenuBar;	/* Height of a one line menu bar */
		int        dypBdrCapt;	/* Height of thick border + caption bar */
		/* Start of table of twip-to-pixel constants */
		/* Must match rgdxaConvert array in initwin.c */
		/* dxp */
		int        dxpMinScroll;    /* Horiz scrolling increment */
		int        dxwSelBar;       /* width of selbar */
		int        xpLeftMin;       /* amt to scroll left of left margin */        
		int        xpRightMax;      /* Largest possible screen xp */
	int	   dxpGrayOutside;  /* PAGE VIEW: Gray area outside page */
		/* dyp */
		int        dysMinAveLine;   /* Lo Avg line size, for scroll,alloc */
		int        dysMacAveLine;   /* Lo Avg line size, for scroll,alloc */
		int        dypMinWwInit;    /* White space above top line */
		int        ypSubSuper;      /* raise/lower amount for subscript/super */
		int        dypWwMin;        /* Min height of window */
	int	   dypPastPage;
	int	   dypPageReveal;
	int	   dypGrayOutside;  /* PAGE VIEW: Gray area outside page */
		/* end of table of twip-to-pixel constants */
		int        dypTmInternalLeading; /* internal leading of system font */
#ifdef WIN23
		long		rgbButtonShadow;
		HBRUSH     hbrButtonText;     /* handle to button text-colored brush */
		HBRUSH		hbrLitButton;
		int		dyRuler;	/* this is variable under 10A */
		int		dccStatLine;
		int		dccIconBar;
		int		dypOtlPat;
		FARPROC lpfnCreateDIBitmap;	/* so we only get it under Win 3 */
		int		dxpFakeTmWidth;
		int		dypFakeTmHeight;
		long	rgbButtonFace;
		long	rgbButtonText;
#endif /* WIN23 */
		};


/* These defines are used in shared code; for Opus they are vsci fields  */

#define dysMinAveLineSci	vsci.dysMinAveLine
#define dysMacAveLineSci	vsci.dysMacAveLine

#define dxpMinScrollSci	        vsci.dxpMinScroll
#define dypBorderFti		vfti.dypBorder
#define dxpBorderFti		vfti.dxpBorder
#define dypPastPageSci		vsci.dypPastPage
#define dypPageRevealSci	vsci.dypPageReveal
#define dypMinWwInitSci         vsci.dypMinWwInit

#define dxpGrayOutsideSci	vsci.dxpGrayOutside
#define dypGrayOutsideSci	vsci.dypGrayOutside

#define xaRightMaxSci	    	vsci.xaRightMax
#define xaLeftMinSci            vsci.xaLeftMin

#define dxwSelBarSci		vsci.dxwSelBar
#define xpRightMaxSci           vsci.xpRightMax
#define xpLeftMinSci            vsci.xpLeftMin

#define xwMaxSci		xpRightMaxSci

#define cxpAutoSci      	vsci.dxpMinScroll

#define dxaOtlMark		(dxaInch / 4)
#define rzOtlPat		9




/* Info about table of twip-to-pixel constants imbedded in struct SCI */

#define brgdxpConvert    (offset( SCI, dxpMinScroll ))
#define brgdypConvert    (offset( SCI, dysMinAveLine ))
#define cDxpConvert      ((brgdypConvert - brgdxpConvert) / sizeof (int))
#define cDypConvert      ((offset( SCI, dypTmInternalLeading) - brgdypConvert) / sizeof (int))

/* patterns available in pattern bitmaps */
/* must track structures in initwin.c used by FCreatePatternBitmaps */

#define ipatVertGray    0
#define ipatVertBlack   1

#define ipatHorzGray    0
#define ipatHorzLtGray  1
#define ipatHorzBlack   2

/* size of pattern bitmaps. must be even multiple of all repeat intervals and
	of 8. For the horz pattern, it must also be a multiple of 16  and 3 */
#define dxpHbmHorz      576
#define dypHbmVert      360

/* Pattern Line Types, for DrawPatternLine */

#define pltHorz         0x00    /* for clarity in the caller */
#define pltVert         0x01    
#define pltOnePixel     0x02    /* limit height/width to one pixel */
#define pltInvert       0x04    /* set if two draws should == no action */
#define pltDouble       0x08    /* Double height/width */


/* following constant is least common multiple of all repeating patterns
	used by DrawPatternLine.  This is helpful for brush alignment. */
	
#define lcmPat          3       


struct BMC {   /* BitMap Cache block for picture display */
	struct	CA ca;
	HBITMAP	hbm;
	int		flm;
	long		lcb;
};

/* Move Pane Direction */
#define mpdPaneOnly  0
#define mpdTopDown   1
#define mpdBottomUp  2


/* For consequtive mouse scroll. */
/* The following sb's better not be used by Windows. */
#define SB_NIL		(-1)
#define SB_MOUSESEL	(0x7FFF)

/* rgb values for character colors. Valid pure Windows colors (bz) */

#define rgbBlack   (RGB(0, 0, 0))
#define rgbBlue    (RGB(0, 0, 255))
#define rgbCyan    (RGB(0, 255, 255))
#define rgbGreen   (RGB(0, 255, 0))
#define rgbMagenta (RGB(255, 0, 255))
#define rgbRed     (RGB(255, 0, 0))
#define rgbYellow  (RGB(255, 255, 0))
#define rgbWhite   (RGB(255, 255, 255))

#ifdef WIN23
#define rgbLitButton	(RGB(224, 224, 224))
#define rgbLtGray		(RGB(192, 192, 192))
#define rgbGray			(RGB(128, 128, 128))
#endif /* WIN23 */

#define FlmDisplayFromFlags(fDispAsPrint,fPageView,fDraftView)	(((fDispAsPrint)||(fPageView))&&!(fDraftView))
