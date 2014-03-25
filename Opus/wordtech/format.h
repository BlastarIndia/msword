/* F O R M A T . H */

/*
Types:

xp      x coordinate measured in device units/screen units depending on
	some mode.
xu      x coordinate measured strictly in device units
xt      x coordinate measured in device units/screen units (see FormatLine)

yp      y coordinate measured in points. screen/device differences generally
	elided.

chp     character properties
chr     a run as output by FormatLine, variats include
chr    contains CHP
chrt   tab
chrv   vanished

fti     font information for the current font for display or for
	"printer" mode formatting. Has enough info to get to char
	widths efficiently.
fmo     font manager output record, sand structure.
ftr     font record, sand structure.


*/

#ifdef WIN
#include "fontwin.h"
#endif


/* length of rgch, rgdxp */
#define ichMaxLine  255
#define ichNil  255

/* Formatted line structure. Inputs to and results of FormatLine */
/* booleans in bytes to simplify machine code */

struct FLI
	{
/* the source of the line. defines cache */
	CP      cpMin;
	CP      cpMac;
	int     doc;
	int	ww;
	int     dxa;
/* xp corresponding to ich=0 */
	int     xpLeft;
/* xp corresponding to ich=ichMac */
	int     xpRight;

/* the number of characters after cpMac that were taken into consideration
while formatting the line. If they change, the formatting will have to be
re-done */
	int     dcpDepend;
/* fSplatBreak is set if cpMin points to a page/division break. cpMac = cpMin + 1,
doc is set, dcpDepend =0, everything else must be ignored */
	union
		{
		struct
			{
			unsigned char     fSplats;
			/* used by layout to force halt at para end */
			unsigned char     fStopAtPara;
			};
		struct
			{
			int     fSplatBreak : 1;
			int     fSplatDots : 1; /* used by outline */
			int     fSplatSub : 1; /* used by outline */
			int     fParaStopped : 1; /* stopped by fStopAtPara */
			int     fSplatColumn : 1; /* column break */
			int     fPropMark : 1; /* draw mark for para props e.g. keep */
			int     omk : 2; /* outline handle format */
#define omkNil          0
#define omkPlus         1
#define omkMinus        2
#define omkBody         3
			int     : 8;
			};
		struct
			{
			int	dummy1 : 7;
			int	fParaAndSplat : 1; /* overlaps omk */
			unsigned char     fStopAtPara;
			};
		};

/* length of rgch/rgdxp */
	unsigned char     ichMac;

/* special justification means that justification will not take place,
instead the spaces in the line will be left on a chain started at ichSpace */
	char    fSpecialJust;

/* the interesting y positions in order from top to bottom: (yp is abs
coordinate of the bottommost point)

	top:                  yp+dypLine
	top of ascenders:     yp+dypAfter+dypFont
	base line:            yp+dypBase
	bottom of descenders: yp+dypAfter
	bottom of line:       yp
distances between the points can be determined by algebraic subtraction.
e.g. space before = yp+dypLine - (yp+dypAfter+dypFont)
*/
	union {
		int rgdyp[5];   /* for printDaisy.c */
		struct {
			int     dypLine;
			int     dypAfter;
			int     dypBefore;
			int     dypFont;
			int     dypBase;
			};
		};

	union {
		int     xpSub; /* used by outline only */
		int     xpMarginRight; /* used by borders only */
		};
/* borders: line thickness, line format code, flags */
	int     xpMarginLeft;
	union {
		struct {
			int     grpfBrc;
			};
		struct {
			int     fTop : 1;
			int     fLeft : 1;
			int     fBottom : 1;
			int     fRight : 1;
			int     fTopEnable : 1;
			int     fBottomEnable : 1;
			int     fBetweenTop : 1;
			int     fBetweenBottom : 1;

			unsigned char     fBarTabs;
			};
		};
		int     dypBrcTop;
		int     dypBrcBottom;
		int     brcTop;
		int     brcLeft;
		int     brcBottom;
		int     brcRight;

/* justification parameters */
/* justification consists of adding 0 units to spaces in the
interval [0, ichSpace1), adding Extra units to [ichSpace1, ichSpace2)
adding Extra+1 units to [ichSpace2, ichSpace3)
and adding 0 units to [ichSpace3, ichMac)

Essentially, the areas are:
	before the last tab or leading spaces in the line: normal width
	justified spaces in the line, first the short then the long ones
	trailing spaces to the right of the right margin: normal width
*/
/* if no justification, ichSpace2 is ichNil (255). Also ichSpace3 will
always point to the first white space in the line from where underlining
must be suppressed */

	uns     dxpExtra;
/* these three fields must be in in sequence (c.f. DisplayFli) */
	unsigned char     ichSpace1;
	unsigned char     ichSpace2;
	unsigned char     ichSpace3;

/* line formatted for page view; splats, ichMac, borders treated differently */
	unsigned char     fPageView;

/* see comment at FormatLine heading */
/* these should be strictly 0/1 values - assumed by native code */
	char    fPrint;
	char    fFormatAsPrint;
	char    fOutline; /* set if outline formatting is in effect */
	char    fLayout;  /* called on behalf of layout */

/* screen and device units given as units per inch. */
	int     dxsInch;
	int     dxuInch;

/* special results */
	char    ichSpace; /* used if fSpecialJust */
/* used by layout: line contains text which may change with no invalidation,
	such as chDate, chFtn, hidden text */
	char    fVolatile;
/* must use abs char posns to compensate for visi space wd != space wd */
		char    fAdjustForVisi;
/* returns true iff grpchrm is incomplete */
	char    fError;
/* returns chNil=-1 if line is broken due to overflow, otherwise
terminating character (Eop, CRJ, Sect, SectJ, NRH, Space, Hyphen) is returned
*/
	int     chBreak;

/*  System specific fields */

#ifdef MAC 
	unsigned char     cchSpace;   /* number of spaces used in justification */
	int	: 8;
	union   {
		int     grpfvisi;
		struct {
			int fvisiTabs : 1;
			int fvisiSpaces : 1;
			int fvisiParaMarks : 1;
			int fvisiFtnRefmarks : 1;
			int fvisiCondHyphens : 1;
			int : 2;
			int fSeeHidden : 1;
			int fShowFormulas : 1;
			int : 7;
			};
		};
	char	fFracOn;	/* fractional widths current on */
	char	fFrac;		/* fractional widths should be used */
	int     dxuFrac;	/* nil setting for vfti.dxuFrac */
#endif /* MAC */
#ifdef WIN
	GRPFVISI	grpfvisi;
		int     dysInch;
		int     dyuInch;
/* fTrue if line contains CHP with fRMark set */
	int	fRMark : 1;
/* fTrue if picture in line */
	int	fPicture : 1;
/* fTrue if metafile in line. implies fPicture also true */
	int	fMetafile : 1;
/* fTrue if formula or vector fonts in line */
	int	fGraphics : 1;
	int : 12;
	int     dytLine;   /* For page view/preview, ht of ln in pr units */
#endif /* WIN */
	int     wSpare1;   /* spare for future use */

/* arrays of visible characters and their widths: */
	char    rgch[ichMaxLine + 1];	/* +1 so that rgdxp will be on a word boundary */
	int     rgdxp[ichMaxLine];
	};

#define cwFLI (sizeof(struct FLI) / sizeof(int))

/* the runs define:
	points where looks change
	points where tabs are
	vanished text (cp's but no ich's)
	created text (ich's but no cp's) used for section numbering (future)
so the hgrpchr table must be consulted to establish ich - dcp correspondence.
This table is in the heap and its size varies (vbchrMax and vbchrMac.)
The variant is defined by chrm.
*/

struct CHR
	{
	char    chrm;   /* chrmChp, also used for chmEnd */
	char    ich;    /* ich at which the looks take effect */
	struct CHP chp;
#ifdef WIN
		union FCID  fcid;       /* fcid for actual font used */
#endif /* WIN */
	};

#define cbCHR sizeof(struct CHR)
#define cwCHR (sizeof(struct CHR) / sizeof(int))
#define cbCHRE 2

/* CHR created for tab */
/* with ch != 0, represents visible char with ch as its original code:
chNonBreakHyphen, chNonReqHyphen, chFormula */
struct CHRT
	{
	char    chrm;   /* chrmTab */
	char    ich;    /* ich of the tab */
	char    ch;     /* used for visible chars if != chNil */
	char    tlc;    /* tab leader code, to be decoded into leader char */
	};

#define cbCHRT sizeof(struct CHRT)

/* CHRV created for a vanished run */
struct CHRV
	{
	char    chrm;   /* chrmVanish */
	char    ich;    /* ich in front of which the vanished run appears */
	CP      dcp;    /* CP length of vanished run */
	};

#define cbCHRV sizeof(struct CHRV)

/* CHRF created for a formulas */
struct CHRF
	{
	char    chrm;   /* chrmFormula */
	char    ich;    /* ich in front of which the formula command appears */
	int     dxp;    /* x movement */
	int     dyp;    /* y movement */
	int     fLine;  /* draw line? */
	};

#define cbCHRF sizeof(struct CHRF)

/* CHRDF created for display programs (fields and others) */
struct CHRDF
		{
		char    chrm;   /* chrmDisplayField */
		char    ich;    /* ich in front of which the field appears */
		int     flt;    /* field type */
		int     w;      /* values stored by flt processor (cache) */
		int     w2;
		long    l;
		int     dxp;    /* size of object */
		int     dyp;
		CP      dcp;    /* characters accounted for by object */
		};

#define cbCHRDF sizeof(struct CHRDF)

/*  CHRFG created for "format group" fields */
struct CHRFG
		{
		char    chrm;   /* chrmFormatGroup */
		char    ich;    /* ich begining group */
		int     flt;    /* field type group created for */
		int     dbchr;  /* bchr following group */
		int     dxp;    /* total size of group */
		CP      dcp;    /* dcp of group */
		};

#define cbCHRFG sizeof(struct CHRFG)

/* chrm's are also used as the lengths of the structures so that grpchr
can be scanned easily. Note that in case of cbCHRT = cbCHRV we fudge the
numbers so that they come out all different */

#define chrmChp cbCHR                   /* 14(mac) 18(win) */
#define chrmTab cbCHRT                  /* 4 */
#define chrmVanish cbCHRV               /* 6 */
#define chrmFormula cbCHRF              /* 8 */
#define chrmDisplayField cbCHRDF        /* 20 */
#define chrmFormatGroup cbCHRFG         /* 12 */
/* the chrmEnd type chr has no separate definition, we just use the first
two bytes of a chr */
#define chrmEnd cbCHRE                  /* 2 */


#ifdef MAC
/* font information, deposited by LoadFont necessary for calculating
the width of characters
*/
struct FTI
	{
/* set if info is in own heap else it is in resource */
	unsigned char     fHeap;
	unsigned char     fPS;  /* set if prop space, used only if fHeap */
	int     dxu;    /* fixed width if !fPS and fHeap */
	int     chFirst;
	int     cch;    /* chLast + 1 - chFirst */
	uns     dxuStyleExtra;
/* this includes the StyleExtra and any font kerning/letterspacing */
	int     dxuExpanded;
	uns     dxuMissing;
/* 32 bit handle to fontrec */
	struct FONTREC far * far *qqftr;
/* displacement of width table in fontrec. Also includes -chFirst bias */
	long    bmpchdxu;
/* 16 bit fraction is maintained between characters */
	uns     dxuFrac;
	uns     wNumer;
/* scaling if wDenom != 0 */
	uns     wDenom;
/* this is StyleExtra, the setting of dxuExpanded with 0 expansion */
	int     dxuExpand0;

/* other font properties */
	int     dypAscent;
	int     dypDescent;
	int     ftc;
	int     catr;
	int     ps;
	};

#define cwFTI (sizeof(struct FTI) / sizeof(int))
#endif /* MAC */


#ifdef WIN
typedef union {
	struct  {
		int	dyp;
		int	dyt;
		};
	long 	wlAll;
	}	DYY;
#else
typedef union {
	int	dyp;
	int	wlAll;
	} DYY;
#endif


/* format special char state (see formatSpec.c) */
struct FMTSS
	{
	long    tick;
	CP      cpRef;
	int     pgn;
	uns     lnn;
	int     doc;
#ifdef MAC
	int     pgnStart; /* used to continue numbering for next file chain */
#endif /* MAC */
#ifdef WIN
/* following are used for "display program" fields (formulas, etc.)
	when showing results of these fields */
		int     flt;
		int     w;
		int     w2;
		long    l;
		int     dxp;
		int     dyp;   /* =dypAscent+dypDescent */
		int     dxt;
	int	dyt;
	DYY	dyyDescent;
#endif /* WIN */
	};

/* minimum tab width in xt units */
#define dxtTabMin 4     

/* border lines */

#define dxpLeftRightSpace (2 * dxpBorderFti)
#define dypLeftRightSpace (2 * dypBorderFti)

#ifdef WIN
/* widths and spacings for paragraph borders */
#define dxaLineWidth 20
#define dyaLineHeight 20
#define dxaLineSpacing 20
#define dyaLineSpacing 20

struct BRDR
	{
	int dxpLineWidth;
	int dypLineHeight; 
	int dxpLineSpacing;
	int dypLineSpacing;
	};
#endif /* WIN */

/* special xpRight used for splats */
#define xpSplat         16384


#define InvalFli()      vfli.ww = wwNil



