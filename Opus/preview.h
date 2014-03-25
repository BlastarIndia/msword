/* locators for absolute objects */
struct AOR
	{
	struct RC rc;    /* in screen units */
	int ilr;
	};
#define cwAOR   (sizeof(struct AOR) / sizeof(int))

struct PLAOR
	{
	uns iaorMac;
	uns iaorMax;
	int cb;
	int brgaor;
	struct AOR rgaor[1];
	};

struct PUCHK
	{
	HWND hwnd;
	struct RC rc;
	int fPopupOverlap;
	};

#define dipgdRandom     5
#define dzpHandle       (5*vsci.dypBorder)

#define dxpScrl         (vsci.dxpScrlBar)
#define dypScrl         (vsci.dypScrlBar)
#define dxpPage		(vpvs.rcPagePrint.xpRight)
#define dypPage		(vpvs.rcPagePrint.ypBottom)
#define dypCaption      (GetSystemMetrics(SM_CYCAPTION))
#define dypMenu         (GetSystemMetrics(SM_CYMENU))
#define dypIconPrvw     (vsci.dypRibbon)
#define xpMinPrvw       0       /* Upper left corner of usable area of */
								/*  page preview window */
#define ypMinPrvw       0
#define ipgdLost        (-4)    /* tell Preview it doesn't know where it is */
#define zpUnknown	-1000
#define xpClose		(2*vsci.dxpBorder)
#define ypClose		(2*vsci.dypBorder)

#define bitDragXa     1    /* show xa value when dragging */
#define bitDragYa     2    /* show ya value when dragging */
#define bitTopRight   4    /* show xa relative to top/right */
#define bitBottomLeft 8    /* show ya relative to bottom/left */

/* kinds of page break */
#define pbkForced       1       /* chSect page break */
#define pbkNatural      2       /* ypLim page break */
#define pbkSection      3       /* section break */

/* tri-state values for vtShowAreas */
#define tAreasOff	0
#define tAreasLeft	1
#define tAreasRight	2
#define TAreasOpposite(tAreas)	(3 - tAreas)

/* Indices into RC Structure (and relavent macros) */
/* WARNING: MUST match Margin iobjs below */
#define ircsLeft	0
#define ircsRight	2
#define ircsTop		1
#define ircsBottom	3
#define FHorzIrcs(ircs) ((ircs) & 1)
#define IrcsOppIrcs(ircs) (((ircs) + 2) % 4)
#define ZpFromRcIrcs(rc,ircs) (*((int *)&rc + (ircs)))
#define FTopLeftIrcs(ircs)  ((ircs) < 2)

/* object ids for keyboard/mouse tracking */
/* WARNING: Margin iobjs MUST match ircs above */
#define iobjNil          (-1)
#define iobjMarginLeft    0
#define iobjMarginTop     1
#define iobjMarginRight   2
#define iobjMarginBottom  3
#define iobjHdr           4
#define iobjFtr           5
#define iobjPgBrk         6
#define iobjMinAbs        7

#define maxww(a,b)	((a) > (b) ? (a) : (b))

typedef struct _pvs    /* PreView State */
	{
	uns     fFacing : 1;         /* facing pages is on */
	uns     fPrvwTab : 1;        /* processing tab key */
	uns     fPrintableArea : 1;  /* clip to printable area */
	uns     fLeftEmpty : 1;
	uns     fRightEmpty : 1;
	uns     fLeft : 1;
	uns     fFirstPaint : 1;
	uns     fFirstBlank : 1;

	uns     fAreasKnown : 1;
	uns     fModeMargin : 1;
	uns     fLoadPrvwFon : 1;
	uns     f1To2 : 1;           /* 1 to 2 page transition in PrvwPages */
	uns     tShowAreas : 2;      /* margins showing: 0=no, 1=left, 2=right */
	uns     pbk : 2;             /* page break type */
	int     ipgdPrvw;            /* ipgd of left page */
	int     ipgdLbs;             /* ipgd -> lbsText position */
	int     lmSave, flmSave;
	int     iobj;                /* object index (margins, apos etc. */
	CP      cpMacPrvw;           /* max cp for which we know pgd is valid */
	int     docPrvw : 6;         /* doc being previewed, not a sub-doc */
	int     fCreating: 1;        /* true when creating preview hwnds */
	int     : 5;                 /* spare */
	int     ypScrlBar;           /* Current position of thumb in scroll bar of
									non-zoom page preview window */
	int     ypLastScrlBar;       /* Current maximum allowable scroll position
									of non-zoom page preview window */
	int     xpMacPrvw,ypMacPrvw; /* Client area of page preview window,
									minus applicable scrollbars */
	int     dxpCli,dypCli;       /* size of client area of preview window */
	int     dxaPage,dyaPage;     /* size of document page in twips */
	int     cchIbLast;           /* # chars last shown on Icon Bar */
	int     ypPageBreak;
	int     dxpGutter;           /* facing pages gutter */
	int     xpMaxPrvw,ypMaxPrvw; /* Lower right corner of page preview
									window when Opus is maximized */
	KMP     **hkmpPrvw;          /* keymap for preview mode */
	struct PLAOR    **hplaor;    /* absolute object rectangles */
	struct RC       rcMargin;    /* interior margin rectangle */
	struct RC       rcHdr,rcFtr; /* positions of boxes */
	struct RC       rcPagePrint; /* size of document page in screen units */
	struct RC       rcPage0, rcPage1;     /* screen coordinates of pages */
	struct LBS      lbsText, lbsFtn;      /* layout state */
	struct LBS      lbsLeft, lbsLeftFtn;  /* left page of two page display */
	} PVS;
