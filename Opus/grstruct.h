#define GRERROR (1)
#define GRINTERRUPT (2)
#define GraphicsError(hpc, idpmt) (EndGraphics(GRERROR))

	/* use native version */
#define HCREADTIFFLINE
#define	ReadTIFFLine(hpc, hpPlane) (ReadTIFFLineNat(hpc, hpPlane))

#define StmGlobalFc() (stmGlobal.fc)
#define WFromStm()	(WFromVfcStm())
#define BFromStm()	(BFromVfcStm())
#define RgbFromStm(hprgb, cb) (HprgbFromVfcStm(hprgb, cb))
#define CheckGrAbort() (CheckGrAbortWin(hpc))
#define	MultDivU(w, Numer, Denom) (UMultDiv(w, Numer, Denom))
#define BFromTIFF()	(BFromVfcStm())

extern STM	stmGlobal;
extern int      vlm;

#ifdef DEBUG
#define OutSz(sz) (CommSz (sz))
#define OutInt(w1, w2) (CommSzNum (SzShared(""), w1))
/* #define OutCh(ch) (CommSzRgCch (SzShared(""), &ch, 1)) */
#define OutCh(ch)
#else
#define OutSz(sz)
#define OutInt(w1, w2)
#define OutCh(ch)
#endif /* DEBUG */


#define FreezeHeap()			FreezeHp()
#define MeltHeap()			MeltHp()

#define true 1
#define false 0
#define cbitByte (8)
#define unsMax (65535)
#define IDMSGBadGrFile (0)

/* to handle structure tags in grtiff.h. PCWORD does this too in word.h */

#define U1
#define U2

#define S1
#define S2
#define S3
#define S4

/*---------- Color constants ----------------------------------------------*/

/*	These are the RGB values that translate exactly to each of the
	colors that GR understands */
								/*  red  green blue */
#define	coBlack		0x0000		/*    0    0    0   */
#define	coWhiteGR	0x7FFF		/*   31   31   31   */

typedef struct {
	unsigned blue: 5;
	unsigned green: 5;
	unsigned red: 5;
} CO;   /* one bit is unused */


/* ---------------------- Bitmap Stretch Modes -------------------- */

#define smhMode		0x03		/* bits 0,1 = horizontal stretch mode */

#define smhSame		0x00
#define smhCompress	0x01
#define smhExpand	0x02

#define smvMode		0x0c		/* bits 2,3  = vertical stretch mode */

#define smvSame		0x00
#define smvCompress	0x04
#define smvExpand	0x08

#define cbExtraPerPlane (2)			/* padding for stretchblt */

/*---------- Dithering constants ------------------------------------------*/

#define	rowPatMax	8				/* 8 rows per pattern */
#define	cbitRowPat	8				/* 8 bits per pattern */
#define icoBitMax	16				/* no more than 16 colors or grays */
#define	iplMax		4				/* no more than 4 planes */
#define patMax		65				/* maximum of 65 patterns */
#define cbRowPat	icoBitMax		/* width of rows in PICT.sbPat */
#define cbDitherPat 128				/* 8 bytes per pattern * 16 patterns */

		/* stolen from definitions of SbOfHp in sbmgr.h */
#ifdef CC
#define PsOfLp(lp) ((WORD)(((DWORD)lp >> 16) & 0xffff))
#else
#define PsOfLp(lp) (__FNATIVE__?HIWORDX((char far *)(lp)):_UOP_INT(1,0xda,(char far *)(lp)))
#endif

#define umod(a,b) (IntAssertProc(a) % IntAssertProc(b))

	/* ********************* TIFF file structures ***************** */

typedef struct {
	unsigned	wCompScheme;
	unsigned	crwStrip;
	FC	    	fcStripOffsets;
	int	    	fShortStripOffset;
	unsigned	irwNext;		/* next row in file */
	FC	    	fcPrev;			/* position of last line read */
	FC	    	fcTrail;		/* starting position of last swath read */
	unsigned	irwTrail;		/* first row of last swath read */
	int	    	cbitsSample;
	BOOL		fSwitchBytes;
	unsigned	cbRowFile;  	/* bytes per input row to be read from the tiff file */

} TIFF;

typedef union {
	struct {
		int     x;
		int     y;
		};
	struct {
		int     h;
		int     v;
	};
} GPOINT;

typedef GPOINT TWP;

typedef GPOINT VPOINT;		/* point in world coordinates */


		/* Subset of PCWORD PICT struct for tiff files */

typedef struct SPict {	/* graphic picture descriptor (one per picture) */

	int	  	ft;  			/* picture (file) type */
	int	  	fn;  			/* file to read from */
	VPOINT		vtOrg,			/* origin of picture in virtual units */
				vtExt;			/* size of picture in virtual units */
	GPOINT		pixOrg,			/* origin of picture in device pixels */
				pixExt;			/* size of picture in device pixels */

#ifdef PCWORD
	TWP	       	twpExt;			/* size of picture in twips */
	int		ypOrg,			/* origin of picture in twips/printer units */
			xpOrg,
			dypExt,			/* size of picture in twips/printer units */
			dxpExt;
#endif /* PCWORD */

	int		fLSRotate:1;	/* vfLandscape && fHeadRotates */
	int   		fColorDither:1;	/* true if doing color bitmap on B&W device */
	int   		fGrayDither:1;	/* true if gray scale image */
	int   		fRealloc:1;		/* true if should try a realloc */
		int             fSpare:12;

	/* the following fields are used for printing: */

	unsigned	cgraphicbytes,	/* width of print image, incl. graph spaces */
			crwSwath,		/* # of rows per swath */
			cbSwathMin,		/* minimum swathe size */
			cbRowMin,		/* size of one print row */
								/* (not the same as cbRowOut) */
			iPass;			/* current interlacing pass */

	/* the following fields are used for printing and for bitmaps: */

	unsigned	cplIn,			/* number of color planes in input */
			cplOut,			/* number of color planes in output */
			cbExtra,		/* extra bytes needed (per bitmap) */
			cbRowIn,		/* bytes per input row */
			cbRowOut,		/* bytes per output row */
			cbPassOut;		/* bytes per print-head pass (on output) */
	SB		sbBits;			/* intermediate buffer for bitmaps */
	unsigned	cbBits,			/* size of sbBits */
			cbPlane,		/* size of each plane in sbBits */
			ibIn,			/* offset to read input into in each plane */
			irwInMax,		/* total # of input rows in sbBits */
			irwOutMax,	 	/* total # of output rows in sbBits */
			irwInMic,		/* # of first input row in sbBits */
			irwOutMic,		/* # of first output row in sbBits */
			irwInMac,		/* # of last input row in sbBits */	 
			irwOutMac,		/* # of last output row in sbBits */
			irwOutNext;		/* # of next output row to print */

#ifdef PCWORD
	SB		sbStm;			/* points to our input stream */
#endif /* PCWORD */



	/* the following fields are only used for bitmaps: */

	unsigned	sm;				/* stretch mode */
	int		crwRepNorm,		/* stretch blt normal # of vertical repeats */
			wRowDelta,		/* stretch blt delta (vertical direction) */
			crwRevRepNorm,	/* str blt norm # of repeats, going bkwards */
			wRevRowDelta,	/* str blt delta, goint backwards */
			cbitRepNorm,	/* stretch blt normal # of horiz. repeats */
			wBitDelta;		/* stretch blt delta (horiz. direction) */
	SB		sbPat;			/* the dither patterns to use */

	BYTE		rgico[16];		/* color translation */

#ifdef WIN
		HANDLE          hbm;
		int             dypBm;                 /* bitmap height  */
		HDC             hMDC;
		struct RC       rcClip;
#endif /* WIN */

		TIFF    	tiff;			/* TIFF info */
} PICT;






