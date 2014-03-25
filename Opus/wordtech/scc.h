#define dlSccMax 25

/* NOTE: the start of an SCC structure is identical to a WWD structure
so that DisplayFli may be used for either */

struct SCC
	{
	int     idrMac;         /* These are to be compatible with WWD */
	int     idrMax;
	int     cbDr;
	int     brgdr;
	int	fExternal;
	int     hpldrBack;  /* set to hNil to distinguish WWD from PLDR */

	int     wk;
	int     : 1;
	int     fDirty : 1;
	int     : 14;

#ifdef MAC
	int /*GRAFPORT*/ far *wwptrDummy;
	int /* struct IB ** */ hibList;/* handle to list of iconbars */
#endif /* MAC */
#ifdef WIN
	GRPFVISI	grpfvisi;   

		HWND    hwnd;           /* Windows window handle */
		HDC     hdc;            /* DC for window */
		HWND    hwndVScroll;    /* Windows window handle for vert scroll bar */
		KMP **  hkmpCur;        /* Current keymap for this pane */
		HWND    hwndIconBar;
	HWND	hwndPgvUp;	/* handle to PAGEUP icon in page view */
	HWND	hwndPgvDown;	/* handle to PAGEDOWN icon in page view */
#endif /* WIN */
	int     mw; /* back pointer to mw */
	union {
		struct RC rcwDisp;
/* note: text display area starts at: xpSelBar, see dispdefs */
		struct {
#ifdef MAC
			int     ywMin; /* pos of top of writeable area of window */
			int     xwMin;
			int     ywMac; /* pos of bottom of window */
			int     xwMac; /* window rel position of last displayable pixel +1 */
#else /* WIN */
			int     xwMin;
			int     ywMin; /* pos of top of writeable area of window */
			int     xwMac; /* window rel position of last displayable pixel +1 */
			int     ywMac; /* pos of bottom of window */
#endif /* ~MAC */
			};
		};
/* The rectangle of the page, in xe,ye coordinates.  Determines scroll state */
	union
		{
		struct RC       rcePage;
		struct {
#ifdef MAC
			int	yeScroll;
			int	xeScroll;
#else
			int	xeScroll;
			int	yeScroll;
#endif /* MAC */
			};
		};

	struct SELS     sels;

/* Invalid area of window */
	union
		{
		struct RC rcwInval; 
		struct  /* note order of xp and yp's in rect */
			{
#ifdef WIN
			int     xwFirstInval;
			int     ywFirstInval;
			int     xwLimInval;
			int     ywLimInval;
#else /* macintosh */
			int     ywFirstInval;
			int     xwFirstInval;
			int     ywLimInval;
			int     xwLimInval;
#endif
			};
		};
#ifdef MAC
	int		bppCur;
#endif
	int     wwDisp;     /* chain of ww's displaying doc */
	int     dqElevator; /* dr where elevator is currently */
	int     xwSelBar;
#ifdef WIN
		int     dxwStyWnd;  /* WIN remember width of styname area in galley vw */
#endif
	int     ipgd;   /* in page view, page table index */
	int     xhScroll;       /* Absolute horizontal scroll */
	int     wSpare2; /* spare */
	int     ww; /* the cache is valid for this window */
	union
		{
		struct DR rgdr[1];      /* WWD is a pldr */
		struct {                /* This is just the definition of */
			union           /*  a DR.  This must be changed */
				{           /*  when the DR definition is changed */
				struct {
#ifdef WIN
					int     xl;
					int     yl;
					int     dxl;
					int     dyl;
#else
					int     yl;
					int     xl;
					int     dyl;
					int     dxl;
#endif
					};
				struct DRC drcl;
				};
			union
				{
				struct {
					CP      cpFirst;
					CP      cpLim;
					int     doc;    /* <0 for fSpec such as auto pgn */
					};
				struct CA ca;
				};
			int     dcpDepend : 8;
			int     fDirty : 1;
			int     fCpBad : 1;
			int     fLimSuspect: 1;
			int     : 5;
			struct PLCEDL **hplcedl;
			int     dypAbove;
					int     dxpOutLeft;
					int     dxpOutRight;
					int     idrFlow;        /* text should flow to this dr */
					int     lrk;            /* type of lr from which this came */
					int     fNoPrev : 1;    /* redundant bit: 1 means there is no
									dr with this dr as successor */
					int     fConstrainLeft : 1;     /* constrained by abs on left */
					int     fConstrainRight : 1;    /* on right */
					int     wSpare1 : 13;  /* spare for future use */
			/* Note: the DR definition now includes a PLCEDL
				header, but this is only for far PLCEDL's.  Since
				the SCC is using a near PLCEDL defined at the
				end of the SCC, the following line is commented
				out (bcv 10-4-88):
			struct PLCEDL plcedl;
			*/
				};
		};

	int     dlMax; /* one less than dlMax in plc */
	int     ywMax;
#ifdef WIN
		union {
			struct BMI bmi;
			struct {
				HBITMAP hbmDummy;
				int     dxsDummy;
				int     ypMax;
				};
		};
#endif /* WIN */
#ifdef MAC
	struct BITMAP far * far * qqbitmap;      /* Really a pixmap */
	unsigned cbSccBmb; /* bitmap length */
	int far * /* HGDEVICE */	hgdMax;
#endif /* MAC */
	int     dlBreak; /* see sccAbove */
	int     fFull:8; /* see sccAbove */
	int     fBreak:8; /* see sccAbove */
	int     dlFirstInval; /* see sccBelow */

/* used to fake a hdndl for WWD compatibility */
	struct PLCEDL *pplcedl;
	union {
		/* Note: the DR definition now includes a PLCEDL
			header, but this is only for far PLCEDL's.  Since
			the SCC is using a near PLCEDL, the PLCEDL defined
			in the DR is commented out. (bcv 10-4-88) */
		/* This must stay in sync with the EDL description !!! */
		struct PLCEDL plcedl;
		struct {
			int     dlMac;
			int     dlMaxDummy;
			int     cbDummy;
			int     icpAdjust;
			CP      dcpAdjust;
			int     icpHint;
			int     fExternal;
			CP      rgcp[dlSccMax + 1];
			struct EDL dndl[dlSccMax];
			};
		};
	};

#define cwSCC (sizeof (struct SCC) / sizeof (int))

#define cbBmbAbove      20000
#define cbBmbBelow      20000
#define cbBmbHoriz      30000
#define cbBmb           1200
