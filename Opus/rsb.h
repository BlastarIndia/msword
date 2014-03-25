

struct RSBS {	/* structure stored in "window extra" space of RSB control */
	int 	sps;
	union {
		int	spsLim;
		int	bcm;	/* for page view pgup/pgdn icon */
		};
	int	fBlank;	
	union {
		int	fVert;
		int	ww;	/* for page view pgup/pgdn icon */
		};
	int	izppInvert;
	int	grpfWnd;
	};

#define cbweRSB		sizeof(struct RSBS)

#define maskWndSplitBox	(0x10)
#define maskWndSizeBox	((int)SBS_SIZEBOX)
#define maskWndPgVw	(0x20)
#define maskWnd		(maskWndSplitBox | maskWndSizeBox | maskWndPgVw)

#ifndef RSBSONLY
struct BMS {	/* structure giving a bitmap and its size */
	int	dxp;
	int	dyp;
	HBITMAP	hbm;
	};


/* global structure containing objects for RSB controls */

struct RSBI {
	union {
	struct	{
		struct BMS rgbms [];
		};
	struct	{
		struct BMS	bmsUArrow;
		struct BMS	bmsDArrow;
		struct BMS	bmsLArrow;
		struct BMS	bmsRArrow;
		struct BMS	bmsSizeBox;
#ifdef WIN23
		/* depressed arrow bitmaps */
		struct BMS	bmsUArrowD;
		struct BMS	bmsDArrowD;
		struct BMS	bmsLArrowD;
		struct BMS	bmsRArrowD;
#endif /* WIN23 */
		};
	};
};

#ifdef WIN23
#define ibmsMax2		5
#define ibmsMax3		9
#else
#define ibmsMax		5
#endif /* WIN23 */


struct ZPP {	/* describes a drawing range of horz/vert RSB control */
	int zpMin;
	int zpMac;
	};


union GRPZPP {	/* describes all drawing ranges of horz/vert RSB control */
		struct	{
		struct ZPP rgzpp[];
		};
	struct	{
		struct ZPP zppUArrow;
		struct ZPP zppAboveThumb;
		struct ZPP zppThumb;
		struct ZPP zppBelowThumb;
		struct ZPP zppDArrow;
		};
	};

#define izppUArrow	0
#define izppAboveThumb	1
#define izppThumb	2
#define izppBelowThumb	3
#define izppDArrow	4
#define izppMax	(sizeof (union GRPZPP)/sizeof (struct ZPP))


#endif /* ndef RSBSONLY */

