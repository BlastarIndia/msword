/* I C O N B A R . H */
/*  definitions for iconbars */

#ifdef DEBUG
#ifdef PCJ
/* #define DBGIB */
#endif /* PCJ */
#endif /* DEBUG */


/* Icon bar descriptors used in creation */

#ifdef WIN23
struct IBID /* Icon Bar Item Description */
	{
	int     ibit : 4;   	/* item type */
	int		fDontEraseTopLine: 1;			/* ugly hack for +/- on ruler */
	int 		fAddOnePixel : 1;	/* for +/- on ruler */
	int		fLatch : 1;	/* if a toggle button, does it latch down when pressed? */
	int		fComposite : 1;	/* composite bitmap? */
	int     iibb : 8;   /* item index */
	CHAR    sz[];       /* text */
	int     x;          /* idrb */ 
	int     y;          /* idcb/tmc */
	int     dx;         /* cbDlt */
	int     dy;         /* cabi */
	BCM     bcm;
	};

struct IBD /* Icon Bar Description */
	{
	int     dy;             /* height */
	int     fInvisible : 1; /* don't make visible */
	int     ibc : 4;        /* class - for iconbar mode */
	int     fAddOnePixel:1; /* hack: add 1 to calculated height */
	int		fGrayable:1;	/* can this icon bar be grayed? */
	int     :1;
		int     :8;             /* Pad to put following elements on even-byte boundaries */
	PFN     pfn;            /* call back function */
	struct IBID rgibid[];
	};
/* run time icon bar structures */

struct IBB /* Icon Bar item */
	{
	int     ibit : 3;   /* item type */
	int     fDisabled : 1;
	int     fHidden : 1;
	int     fHilite : 1;
	int     fGray : 1;
	int     fOn : 1;
	int		fComposite : 1;
	int		fLatch : 1;
	int		fAddOnePixel:1;
	int		fDontEraseTopLine: 1;			/* ugly hack for +/- on ruler */
	int     : 4;    /* Pad to put following elements on even-byte boundaries */
	BCM     bcm;
	union
		{
		struct      /* 99% case */
			{
			union
				{
				struct RC rc;
				struct
					{
					int     xpLeft;
					int     ypTop;
					int     xpRight;
					int     ypBottom;
					};
				};
			union
				{
				CHAR      **hsz;    /* text/text toggles */
				int         tmc;    /* dialog items */
				HWND    hwndCus;    /* custom controls */
				struct              /* bitmap toggles */
					{
					int     idrb : 8;
					int     idcb : 8;
					};
				};
			};
		struct      /* dialog case */
			{
			int     xpDlg;
			int     ypDlg;
			WORD    hdlg;
			WORD    hdlt;
			WORD    hcab;
			};
		};
	};
#else
struct IBID /* Icon Bar Item Description */
	{
	int     ibit : 8;   /* item type */
	int     iibb : 8;   /* item index */
	CHAR    sz[];       /* text */
	int     x;          /* idrb */ 
	int     y;          /* idcb/tmc */
	int     dx;         /* cbDlt */
	int     dy;         /* cabi */
	BCM     bcm;
	};

struct IBD /* Icon Bar Description */
	{
	int     dy;             /* height */
	int     fInvisible : 1; /* don't make visible */
	int     ibc : 4;        /* class - for iconbar mode */
	int     fAddOnePixel:1; /* hack: add 1 to calculated height */
	int     :2;
		int     :8;             /* Pad to put following elements on even-byte boundaries */
	PFN     pfn;            /* call back function */
	struct IBID rgibid[];
	};



/* run time icon bar structures */

struct IBB /* Icon Bar item */
	{
	int     ibit : 3;   /* item type */
	int     fDisabled : 1;
	int     fHidden : 1;
	int     fHilite : 1;
	int     fGray : 1;
	int     fOn : 1;
		int     : 8;    /* Pad to put following elements on even-byte boundaries */
	BCM     bcm;
	union
		{
		struct      /* 99% case */
			{
			union
				{
				struct RC rc;
				struct
					{
					int     xpLeft;
					int     ypTop;
					int     xpRight;
					int     ypBottom;
					};
				};
			union
				{
				CHAR      **hsz;    /* text/text toggles */
				int         tmc;    /* dialog items */
				HWND    hwndCus;    /* custom controls */
				struct              /* bitmap toggles */
					{
					int     idrb : 8;
					int     idcb : 8;
					};
				};
			};
		struct      /* dialog case */
			{
			int     xpDlg;
			int     ypDlg;
			WORD    hdlg;
			WORD    hdlt;
			WORD    hcab;
			};
		};
	};
#endif /* WIN23 */

#define cbIBB   sizeof(struct IBB)

struct IBS /* Icon Bar Structure */
	{
	HWND    hwnd;
	PFN     pfn;
	WORD    wRef;       /* user data */
	int     ibc : 4;
	int     : 12;       /* Pad to put following elements on even-byte boundaries */
	int     iibbDlg;
	int     iibbMac;
	int     iibbMinSys;
	struct IBB rgibb[];
	};
#define cbIBSBase   offset(IBS,rgibb)

typedef struct IBS **HIBS;


#define iibbNone        0xff
#define iibbNil         idNil
#define ibcNil          0xf

/* Item Types */
#define ibitNil         0
#define ibitToggleText  1
#define ibitToggleBmp   2
#define ibitText        3
#define ibitBitmap      4 /* does not result in an item */
#define ibitDialog      5
#define ibitDlgItem     6
#define ibitCustomWnd   7
#define ibitMax         8
#define ibitEnd         ibitNil


#ifdef WIN23

#define ibdHeader(ibdFoo,dy,fVanish,ibc,fAO,fGrayable,pfn) \
		csconst struct IBD ibdFoo = { dy,fVanish,ibc,fAO,fGrayable,pfn,{
#define ibdEnd \
		{ ibitEnd,fFalse, fFalse, fFalse, fFalse, 0,"",0,0,0,0,bcmNil }}};
#define ibidToggleText(x,y,dx,dy,sz,iibb,bcm) \
		{ ibitToggleText, fFalse, fFalse, fFalse, fFalse, iibb,sz,x,y,dx,dy,bcm },
#define ibidToggleBmp(x,y,dx,dy,iibb,bcm, fLatch) \
		{ ibitToggleBmp,fFalse, fFalse, fLatch, fTrue, iibb,"",x,y,dx,dy,bcm },
#define ibidToggleBmp2(x,y,dx,dy,iibb,bcm, fLatch, fAddOnePixel, fHack) \
		{ ibitToggleBmp,fHack, fAddOnePixel, fLatch, fTrue, iibb,"",x,y,dx,dy,bcm },
#define ibidText(x,y,dx,dy,sz) \
		{ ibitText,fFalse, fFalse, fFalse, fFalse, iibbNone,sz,x,y,dx,dy,bcmNil },
#define ibidTextIibb(x,y,dx,dy,sz,iibb) \
		{ ibitText,fFalse, fFalse, fFalse, fFalse, iibb,sz,x,y,dx,dy,bcmNil },
#define ibidBitmap(idrb,idcb) \
		{ ibitBitmap,fFalse, fFalse, fFalse, fFalse, iibbNone,"",idrb,idcb,0,0,bcmNil },
#define ibidDialog(x,y,cabi,cbDlt,iibb) \
		{ ibitDialog,fFalse, fFalse, fFalse, fFalse, iibb,"",x,y,cbDlt,cabi,bcmNil },
#define ibidDlgItem(tmc,iibb) \
		{ ibitDlgItem,fFalse, fFalse, fFalse, fFalse, iibb,"",0,tmc,0,0,bcmNil },
#define ibidCustomWnd(x,y,dx,dy,sz,iibb) \
		{ ibitCustomWnd,fFalse, fFalse, fFalse, fFalse, iibb,sz,x,y,dx,dy,bcmNil },

#else

#define ibdHeader(ibdFoo,dy,fVanish,ibc,fAO,pfn) \
		csconst struct IBD ibdFoo = { dy,fVanish,ibc,fAO,pfn,{
#define ibdEnd \
		{ ibitEnd,0,"",0,0,0,0,bcmNil }}};
#define ibidToggleText(x,y,dx,dy,sz,iibb,bcm) \
		{ ibitToggleText,iibb,sz,x,y,dx,dy,bcm },
#define ibidToggleBmp(x,y,dx,dy,iibb,bcm) \
		{ ibitToggleBmp,iibb,"",x,y,dx,dy,bcm },
#define ibidText(x,y,dx,dy,sz) \
		{ ibitText,iibbNone,sz,x,y,dx,dy,bcmNil },
#define ibidTextIibb(x,y,dx,dy,sz,iibb) \
		{ ibitText,iibb,sz,x,y,dx,dy,bcmNil },
#define ibidBitmap(idrb,idcb) \
		{ ibitBitmap,iibbNone,"",idrb,idcb,0,0,bcmNil },
#define ibidDialog(x,y,cabi,cbDlt,iibb) \
		{ ibitDialog,iibb,"",x,y,cbDlt,cabi,bcmNil },
#define ibidDlgItem(tmc,iibb) \
		{ ibitDlgItem,iibb,"",0,tmc,0,0,bcmNil },
#define ibidCustomWnd(x,y,dx,dy,sz,iibb) \
		{ ibitCustomWnd,iibb,sz,x,y,dx,dy,bcmNil },

#endif /* WIN23 */
struct GRPFIBIT /* type info */
	{
	int     fHasIbb : 1;    /* IBB is generated */
	int     fToggle : 1;    /* is a toggle */
	int     fMouseHit : 1;  /* generates WM_COMMAND to IbProc on mouse */
	int     fHasCoord : 1;  /* has coordinates in IBB */
	int     fPaint : 1;     /* some paint action required */
	int     : 3;
	};
	
#ifdef MPIBITGRPF
struct GRPFIBIT mpibitgrpf [ibitMax] = 
	{
		{ fFalse, fFalse, fFalse, fFalse, fFalse  },/* ibitNil */   
		{ fTrue,  fTrue,  fTrue,  fTrue,  fTrue   },/* ibitToggleText */
		{ fTrue,  fTrue,  fTrue,  fTrue,  fTrue   },/* ibitToggleBmp */
		{ fTrue,  fFalse, fFalse, fTrue,  fTrue   },/* ibitText */
		{ fFalse, fFalse, fFalse, fFalse, fFalse  },/* ibitBitmap */
		{ fTrue,  fFalse, fFalse, fFalse, fTrue   },/* ibitDialog */
		{ fTrue,  fFalse, fFalse, fFalse, fFalse  },/* ibitDlgItem */
		{ fTrue,  fFalse, fFalse, fTrue,  fTrue   },/* ibitCustomWnd */
	};
#else
extern  struct GRPFIBIT     mpibitgrpf[];
#endif /* MPIBITGRPF */


/* Icon Bar proc Messages */
#define ibmCreate       1
#define ibmDestroy      2
#define ibmPaint        3
#define ibmClick        4
#define ibmDblClk       5
#define ibmCommand      6
/* icon bar dialog messages */
#define ibmInit         7
#define ibmTerm         8
#define ibmCancel       9
#define ibmChange       10
#define ibmSetItmFocus  11
/* windowing  */
#define ibmMoveSize     12


HWND HwndCreateIconBar();
NATIVE struct IBB *PibbFromHibsIibb();
NATIVE WORD WRefFromHwndIb();
HWND HwndCusFromHwndIibb();


#define HibsFromHwndIb(hwnd) ((HIBS)GetWindowWord(hwnd,IDWIBHIBS))
#define BltIbd(ibdSrc,pibdDest) \
		bltbx((CHAR FAR *)&ibdSrc,(CHAR FAR *)pibdDest,sizeof(ibdSrc))


struct BMID /* BitMap Icon Description */
	{
	HWND hwndParent;
	union
	{
	struct RC rc;
	struct 
			{
		int xpLeft;
		int ypTop;
		int xpRight;
		int ypBottom;
		};
	};
	int idrb; /* resource ID of bitmap */
	int idcb; /* index into multiple bitmaps-in-a-bitmap */
	BCM bcm;  /* bcm for the command to be executed */
	int ww;   /* ww in which the bitmap belongs to */
	};


#ifdef WIN23
/* number of common pixels around a button */
#define cPixBorder	4
#endif /* WIN23 */

