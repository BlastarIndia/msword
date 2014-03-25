/* pic.h -- definitions for pictures.  very Windows-specific. */

/* following constant needed to prevent dragging to arbitrarily small size */
#define dypPicSizeMin       16  /* Smallest y-extent of a picture, in pixels */

#define MM_NIL          -1
#define MM_BITMAP       99      /* phony mapping mode, means "this is a bitmap,
									not a metafile" */
#define MM_TIFF         98      /* another phony mapping mode */
#define MM_DIB          97
#define MM_META_MAX     (MM_ANISOTROPIC + 1)     /* 1 past highest metafile index in windows.h */

/* file type codes for graphics files. Add new codes here. */

#define ftNil (-1)
#define ftfBitmap	128		/* marks bitmap file types */
#define ftfUnknown	 64		/* marks file types we can't readily recognize */

/*supported graphics files*/
#define ftClipboard   	(32 | ftfBitmap)
#define ftPCPaint   	(33 | ftfBitmap)
#define ftTIFF	      	(34 | ftfBitmap)
#define ftCapture   	(35 | ftfBitmap)
#define ftDIB	      	(40 | ftfBitmap)


#define dzPicBase	      	(3)   // largest side of unsized pic is 3" 

/* Following is the definition of a picture, either a bitmap or a Windows
	MetaFile.  structure appears at chp.fcPic in fn containing chPicture */

struct PIC 
{
long       lcb;            /* # bytes total including header. First field 
								to ease use of some Mac code */
uns        cbHeader;       /* # bytes in this header (for future expansion) */
METAFILEPICT mfp;          /* metafile picture descriptor (WINDOWS)
								for bitmaps, the only significant fields are:
									mfp.mm == MM_BITMAP or MM_TIFF
									mfp.xExt == bitmap width in pixels
									mfp.yExt == bitmap height in pixels */
union
	{
	BITMAP     bm;             /* bitmap descriptor; not significant for metafiles*/
	struct RC  rcWinMF;        /* rect for window org and extents - ignored if 0's */
	};

int        dxaGoal;        /* goal size; 0 == no goal size; nonzero only */
int        dyaGoal;        /* for imported bitmaps with BitmapDimension set.
								affects scaling during: import, printer change.
								may not actually scale to this size if it would
								require a non-power-of-two StretchBlt) */
unsigned   mx, my;         /* scale factor applied by the user */

			/* offsets for cropping based on original size in dxa/yaGoal */
union
	{
	struct
		{
		int        dxaCropLeft;
		int        dyaCropTop;
		int        dxaCropRight;
		int        dyaCropBottom;
		};
   	int rgCrop[4];
	};

uns        brcl: 4;
uns        fFrameEmpty: 1;
uns        : 11;
char rgb[0];              /* variable size; bitmap bits or metafile contents */
};

#define cbPIC           (sizeof (struct PIC))

#define iCropLeft 	(0)
#define iCropTop 	(1)
#define iCropRight 	(2)
#define iCropBottom (3)

struct PICRC
{
	struct RC frOut;
	struct RC frIn;
	struct RC picScl;
	struct RC picUnscl;
};

struct PICDIM  /* dxp, dyp for picture rc's */
{
	struct PT frOut;
	struct PT frIn;
	struct PT picScl;
	struct PT picUnscl;
	struct PT frOutZt;  /* scaled size in vftiDxt units */
};

struct WMFWINOE  /* hand coded windows origin and extent for import field metafile */
{
	DWORD cwWinOrigin;
	int kwdWinOrigin;
	int ypWinOrigin; /* note reverse order for args */
	int xpWinOrigin;
	DWORD cwWinExtent;
	int kwdWinExtent;
	int ypWinExtent;
	int xpWinExtent;
};

#define cwWMFWINOE (sizeof(struct WMFWINOE) / sizeof(int))

#define WINMETAVERSION (0x0300)

struct METAHDR  /* metafile header - should be in Windows h file */
{
	WORD  mtType;
	WORD  mtHeaderSize;
	WORD  mtVersion;
	DWORD mtSize;
	WORD  mtNoOObjects;
	DWORD mtMaxRecord;
	WORD  mtNoParameters;
};
#define cwMETAHDR (sizeof(struct METAHDR) / sizeof(int))


#define METAENDREC (0)  /* keyword of metafile end record */

struct WINMRTAG	 // metafile record tag
{
	DWORD rdSize;
	int kwd;
};
#define cbWINMRTAG (sizeof(struct WINMRTAG))
#define cwWINMRTAG (sizeof(struct WINMRTAG) / sizeof(int))

struct WMFCOLOR  /* hand coded text/bg color recs */
{
	DWORD cwTColor;	 // 5
	int kwdTColor;	 // x209
	DWORD rgbTColor;
	DWORD cwBColor;	 // 5
	int kwdBColor;	 // x201
	DWORD rgbBColor;
};

#define cwWMFCOLOR (sizeof(struct WMFCOLOR) / sizeof(int))

struct WMFSDIB  /* hand coded stretchdibitmap */
{
	DWORD cwSDIB;	 
	int   kwdSDIB;	 // xf43
	DWORD dwRop;
	WORD  wUsage;
	WORD  srcYExt;
	WORD  srcXExt;
	WORD  srcY;
	WORD  srcX;
	WORD  dstYExt;
	WORD  dstXExt;
	WORD  dstY;
	WORD  dstX;
};

#define cbWMSDIB (sizeof(struct WMFSDIB))
#define cwWMSDIB (sizeof(struct WMFSDIB) / sizeof(int))

struct MFHDRDIBMF  /* hand coded metafile header for dib converted to WMF */
{
    struct METAHDR metahdr;
	struct WMFWINOE winoe; /* hand coded windows origin and extent for import field metafile */
	struct WMFCOLOR color; /* hand coded text/bg color recs */
	struct WMFSDIB stretchdib; /* hand coded stretchdibitmap */
};
#define cbMFHDRDIBMF (sizeof(struct MFHDRDIBMF))
#define cwMFHDRDIBMF (sizeof(struct MFHDRDIBMF) / sizeof(int))

/* masks for pic grpf */

#define picgFrOut (1)
#define picgFrIn (2)
#define picgPicScl (4)
#define picgPicUnscl (8)
#define picgAll (-1)

#define mx100Pct        1000    /* Scale factor corresponding to 100% size */
#define my100Pct        1000    /* Scale factor corresponding to 100% size */
#define PctTomx100Pct   10      /* Factor for converting dialog % to mx */
#define PctTomy100Pct   10      /* Factor for converting dialog % to my */

#define cbrDragBox 6
#define dxpDragBox	(vsci.dxpBorder * cbrDragBox)	/* size of drag boxes in frame */
#define dypDragBox	(vsci.dypBorder * cbrDragBox)	/* size of drag boxes in frame */

#define dxaPicMin	(173) /* min picture size ( .12" nearest .01" to 1/8")*/
#define dyaPicMin	(173) /* min picture size */
#define dxaPicMinDrag (180) /* some slack to handle dialog rounding */
#define dyaPicMinDrag (180) 
#define dxaPicMax	(czaMax) /* max picture size (about 22" and around 32K) */
#define dyaPicMax       (czaMax) /* max picture size */

    /* note: max is scale BEFORE mult by 10, min is scale AFTER mult by 10 */
#define mzPicMax	(6550) /* max scale, so 10 * will fit unsigned */
#define mzPicMin	(10) /* min scale * 10, so 1/10 will be 1% */


/* drag handle locations order is that which we use to check for
	mouse hits, so most likely drag locations are first. */

/* WARNING: GetRcDragLim() [and perhaps other fns] uses the numerical */
/* values defined here, don't change them without modifying this      */
/* function (MaxB 9/8/88)                                             */

#define ircsBC	0
#define ircsBR	1
#define ircsCR	2
#define ircsTR	3
#define ircsTC	4
#define ircsTL	5    /* Top Left */
#define ircsCL	6    /* Center Left */
#define ircsBL	7    /* Bottom Left */
#define ircsMax 	8


		/* In the case of monochrome devices, this raster op will map white in
		the bitmap to the background color and black to the foreground color. */
#define ropMonoBm 0x00990066


#ifdef PICTDRAG
	/* prompts for pic drag */

csconst CHAR mstpCropL[] =  StKey ("Cropping:    0.00\"   Left",CropLeft);
csconst CHAR mstpCropR[] =  StKey ("Cropping:    0.00\"   Right",CropRight);
csconst CHAR mstpCropT[] =  StKey ("Cropping:    0.00\"   Top",CropTop);
csconst CHAR mstpCropB[] =  StKey ("Cropping:    0.00\"   Bottom",CropBottom);
csconst CHAR mstpCropTL[] = StKey ("Cropping:    0.00\"   Left     0.00\"   Top",CropLeftTop);
csconst CHAR mstpCropTR[] = StKey ("Cropping:    0.00\"   Right    0.00\"   Top",CropRightTop);
csconst CHAR mstpCropBL[] = StKey ("Cropping:    0.00\"   Left     0.00\"   Bottom",CropLeftBottom);
csconst CHAR mstpCropBR[] = StKey ("Cropping:    0.00\"   Right    0.00\"   Bottom",CropRightBottom);
		/* these will change for internationalization */
		/* where changing portions start */
#ifndef INTL
#define ichmstCrop1 (10)
#define ichmstCrop2 (27)
#endif

#define cchmstCrop (11)  /* # of chars to change */

csconst CHAR mstpScaleH[] =  StKey ("Scaling:   100% High",ScaleHigh);
csconst CHAR mstpScaleW[] =  StKey ("Scaling:   100% Wide",ScaleWide);
csconst CHAR mstpScaleHW[] = StKey ("Scaling:   100% High   100% Wide",ScaleHighWide);

#ifndef INTL
#define ichmstScale1 (10)
#define ichmstScale2 (22)
#endif

#define cchmstScale (5)  /* # of chars to change */

#endif /* PICTDRAG */
