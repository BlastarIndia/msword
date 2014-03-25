/* WARNING: Most of the following defines and structs are */
/* duplicated in structs.inc and consts.inc - they must be */
/* updated there as well */

#define cbrcParaBorders	6

#ifdef MAC
struct ARG_PARABORDERS
	{
	int ibrcSel;
	struct BPSL *pbpsl;
	int brt;
	int hpsSpace;
	union
		{
		int rgbrc[cbrcParaBorders];
		struct
			{
			int brcTop;
			int brcLeft;
			int brcBottom;
			int brcRight;
			int brcBetween;
			int brcBar;
			};
		};
	};
#define iagParaBordersMac	IagMacOf(ARG_PARABORDERS)
#endif /* MAC */

/* the order here must agree with brc's below */
/* WARNING: these values are duplicated in consts.inc, */
/* change them there, too. */
#define	ibrcTop         0
#define	ibrcLeft        1
#define	ibrcBottom      2
#define	ibrcRight       3
#define	ibrcInside      4
#define cbrcCellBorders	5	/* brc's to describe selection and whole table */

#ifdef MAC
struct ARG_CELLBORDERS
	{
	int ibrcSel;
	struct BTSL *pbtsl;
	BOOL fEveryCell; /* else apply to selection as block */
	union
		{
		int rgbrc[cbrcCellBorders];
		struct
			{
			int brcTop; /* the ordering here must agree with ibrc's above */
			int brcLeft;
			int brcBottom;
			int brcRight;
			int brcInside;
			};
		};
	};
#define iagCellBordersMac	IagMacOf(ARG_CELLBORDERS)
#endif /* MAC */

#ifdef MAC
/* Only in Mac, such measurement is valid. */
#define dzFrameLine	 1   /* width of frame line */
#define dzShadow         1   /* width of a shadow line */
#define dzTableBorderMax 4   /* Current maximum table border size */
#endif

#define dxFrameLine		dxpBorderFti
#define dyFrameLine		dypBorderFti
#define dxShadow		dxpBorderFti
#define dyShadow		dypBorderFti
#define dxTableBorderMax	(dxpBorderFti * 4)
#define dyTableBorderMax	(dypBorderFti * 4)

#define blMax	        6
#define blpWhite        0
#define blpGrey	        1
#define blpBlack        2
#define blpLaser        3
#define blpShadow       4

#define	dbmodHV         1	/* mask for pulling off Horiz/Vert info */
#define	dbmodHoriz      0
#define dbmodVertical	1

#define dbmodReverse	2	/* mask for pulling off reverse info */
#define	dbmodNoReverse	0

#define	dbmodTop        (dbmodHoriz+dbmodReverse)
#define dbmodBottom     (dbmodHoriz+dbmodNoReverse)
#define dbmodLeft       (dbmodVertical+dbmodReverse)
#define dbmodRight      (dbmodVertical+dbmodNoReverse)

#ifdef MAC
/* WARNING Bit-order dependent!!!! */
struct BRC
	{
	union
		{
		struct
			{
			int fSpare : 1;
			int fShadow : 1;
			int dxpSpace : 5;
			int dxpLine1Width : 3;
			int dxpSpaceBetween : 3;
			int dxpLine2Width : 3;
			};
		int brc;
		struct
			{
			int : 7;
			int brcBase : 9;
			};
		};
	};
#else /* WIN */
struct BRC
	{
	union
		{
		int brc;
		struct
			{
			int dxpLine2Width : 3;
			int dxpSpaceBetween : 3;
			int dxpLine1Width : 3;
			int dxpSpace : 5;  /* stored in points for WIN */
			int fShadow : 1;
			int fSpare : 1;
			};
		struct
			{
			int brcBase : 9;
			int : 7;
			};
		};
	};
#endif /* WIN */

/* Definitions for the brc Codes */


/* For the lines, 0-5 describe thicknesses, 6 = dotted, 7 = hairline */
#define brcSpare        0x8000
#define brcfShadow      0x4000
#define brcDxpSpace     0x3e00
#define shiftDxpSpaceBrc     9
#define brcLine1        0x01c0
#define shiftLine1Brc        6
#define brcSpace2       0x0038
#define shiftSpace2Brc       3
#define brcLine2        0x0007


#define	brcNil          0xFFFF	/* no border corresponding to this position */
#define	brcNone	        0x0000  /* an empty border */
#define brcSingle       0x0040  /* A single line */
#define brcTwoSingle    0x0049  /* Two single lines separated by 1 */
#define brcFatSolid     0x0100  /* a four-pixel stripe */
#define brcThick        0x0080  /* a two-pixel stripe */
#define brcDotted       0x0180  /* a single dotted line */
#define brcHairline     0x01C0  /* a hairline, only works on LaserWriter */

#ifdef MAC
#define dxpBorderSpaceMax		0x20
#endif	
#ifdef WIN
#define	dptBrcSpace     1	/* spacing in POINTS used between border and text */
#endif /* WIN */

#define	dxpCellMin      1	/*minimum cell width after border calculation*/

#define	ibrcDraw			(0)
#define	ibrcLeftButt	(1)
#define	ibrcTopButt		(2)
#define	ibrcBottomButt	(3)
#define	cbrcCorner	4	/* number of brc's needed to convey corner info */

#define cbrcTcx         4

/* changes made to TCX should be duplicated in the TCC struct */
struct TCX		/* Table Cell X-direction information */
	{
	int xpCellLeft;
	int xpDrLeft;
	int xpDrRight;
	int xpCellRight;
	int dxpOutLeft;
	int dxpOutRight;
	union
		{
		int	rgbrc[cbrcTcx];
		struct
			{
			int brcLeft;
			int brcTop;
			int brcBottom;
			int brcRight;
			};
		};
	};

struct TCC		/* Table Cell Cache */
	{
	union
		{
		struct
			{
			CP cpFirst;
			CP cpLim;
			int doc;
			};
		struct CA ca;	/* the cached cell */
		};
	int itc;
	unsigned fFirstRow : 1;
	unsigned fLastRow : 1;
	unsigned fXpValid : 1;
	unsigned fDylValid : 1;
	unsigned fOutline : 1;
	unsigned fEasyBorders : 1;
	unsigned : 10;
	union
		{
		struct
			{
			int xpCellLeft;
			int xpDrLeft;
			int xpDrRight;
			int xpCellRight;
			int dxpOutLeft;
			int dxpOutRight;
			int brcLeft;
			int brcTop;
			int brcBottom;
			int brcRight;
			};
		struct TCX tcx;
		};
	int dylAbove;
	int dylBelow;
	int rgbrcEasy[cbrcCellBorders];
#ifdef WIN
	int dxpInch;		/* Used in conjunction w/fXpValid: horz
					device resolution for which xp's are valid */
#else
	int wSpare1;
#endif
	int wSpare2;
	struct CA caTap;	/* the row to which the cell belongs */
#ifdef MAC /* use caTapAux and vtapFetchAux in WIN */
	struct CA caTapPrev;
	struct TAP tapPrev;
#endif /* MAC */
	};
#define cbTCC (sizeof(struct TCC))
#define cwTCC (cbTCC/sizeof(int))

struct BRK /* BoRder Kode - a decoded BRC */
	{
	int dxWidth;
	int dyHeight;
	int blMac;
	int fShadow;
	int mpbldx[blMax];
	int mpbldy[blMax];
	int mpblblp[blMax];
	};

struct TCXC
	{
	int fValid;
	int itcNext;
	struct TCX tcx;
	};
	
#define itcxcMax	10
struct TCXS {
	struct CA ca;
	int grpfTap;	/* to be compared against tap.grpfTap to validate cache */
#ifdef WIN
	int dxpInch;	/* horz device resolution for which xp's are valid */
#else
	int wSpare1;
#endif
	struct TCXC	rgtcxc[itcxcMax];
};

#ifdef MAC
/* In Mac, Dz is valid for both horizontal and vertical directions. */
#define DzFromBrc(brc, fFrameLines) \
		DxyFromBrc(brc, fFrameLines, fTrue)
#define DzOfLineFromBrc(brc, fFrameLines) \
		DxyOfLineFromBrc(brc, fFrameLines, fTrue)
#endif /* MAC */

#define DxFromBrc(brc, fFrameLines) DxyFromBrc(brc, fFrameLines, fTrue)
#define DyFromBrc(brc, fFrameLines) DxyFromBrc(brc, fFrameLines, fFalse)
#define DxOfLineFromBrc(brc, fFrameLines) \
			DxyOfLineFromBrc(brc, fFrameLines, fTrue)
#define DyOfLineFromBrc(brc, fFrameLines) \
			DxyOfLineFromBrc(brc, fFrameLines, fFalse)

#ifdef MAC
/* Define these to speed things along */

#define DxyFromBrc(brc, fFrameLines, fWidth)	\
			WidthHeightFromBrc(brc, fFrameLines, fWidth, fFalse)

#define DxyOfLineFromBrc(brc, fFrameLines, fWidth) \
			WidthHeightFromBrc(brc, fFrameLines, fWidth, fTrue)
#endif /* MAC */
#ifdef WIN
#ifndef DEBUG
/* Define these to speed things along */

#define DxyFromBrc(brc, fFrameLines, fWidth)	\
			WidthHeightFromBrc(brc, fFrameLines | (fWidth << 1))

#define DxyOfLineFromBrc(brc, fFrameLines, fWidth) \
			WidthHeightFromBrc(brc, fFrameLines | (fWidth << 1) | 4)
#endif /* !DEBUG */
#endif /* WIN */
		
