/* fontwin.h - definitions for Windows font handling */

#define hfontSpecNil	((HFONT)1)	/* placeholder in cache;
						font handle not gotten yet */
/* P e r f o r m a n c e */

#define chDxpMin        0       /* 1st Char whose width is cached */
#define chDxpMax        256     /* Max char whose width is cached */
#define cftcExtra       (0)     /* extra spaces in new dod font table */
#define iffnProfMax     5       /* # of fonts described in win.ini list */
#define ifceMax         32      /* size of font cache array in DS */
								/* == # of fonts cached */
/* M a s k s */
#define maskPrqLF	0x03	/* mask for {default,fixed,variable} in LOGFONT */
#define maskFVarPitchTM	0x01    /* mask for fVariablePitch in TEXTMETRIC */
#define maskFfFfid	0x70	/* mask for font family in ffid */
#define maskfGraphicsFfid	0x80

/* F o n T  C o d e */
/* an ftc is the font code stored in the CHP.  It is an index into */
/* dod.hmpftcibstFont, the document-specific font table */
#define ftcDefault      0       /* ftc used in StandardChp */
#define ftcValMax       0x1FE   /*  1 less than 9 bits of 1's */
#define ftcNil          (ftcValMax + 1)

/* F o n t  H a l f  P o i n t  S i z e */
#define hpsDefault (20) /* height in half point for default font */

/* i b s t  F o n t */
/* an ibstFont is an index into vhsttbFont, the master font table */
typedef CHAR IBSTFONT;
#define ibstFontMax     255
#define ibstFontNil     ibstFontMax        
#define ibstFontDefault 0       /* ftcDefault always maps to this */
#define ibstCourier 	3      

/* F o n t  F a m i l y  a n d  N a m e */
/* this is what is stored in vhsttbFont; FFN is a funny st */
struct FFN    
	{
	union	{
		struct	{
			CHAR 	cbFfnM1;
			int	ffid: 8;
			};
		struct	{
			int	: 8;
			int	prq: 2;	/* pitch request */
			int	: 1;	/* spare bit of ffid */
			int	fRaster: 1;		/* whether raster font (different from fGraphics) */
			int	ff: 3;	/* Font Family portion of ffid */
			int	fGraphics: 1;	/* whether gdi font */
			};
		};
	CHAR szFfn [];  /* Variable length */
/*	char	chs;		 char set (as in LOGFONT.lfCharSet) */
	};
													
	/* note cch values in these macros should include null terminator */
	/* all these are hacked to account for chs at end */
#define CbFfnFromCchSzFfn(cch) (offset(FFN,szFfn) + (cch)+1)
#define CbSzOfPffn(pffn) ((pffn)->cbFfnM1 - offset(FFN,szFfn))
#define cbFfnLast (offset(FFN,szFfn) + LF_FACESIZE + 1)
#define ChsPffn(pffn)	(*((char *)(pffn)+(pffn)->cbFfnM1))

/* F o n t  C a c h e  I d e n t i f i e r */
/* a compressed CHP, made document-independent by mapping ftc's
	to ibstFont's; fFixedPitch is an added 'hint' for requesting
	screen fonts based on what we got back from the printer */
union FCID 
		{ 
		long lFcid;
		struct
				{
				WORD    wProps;
				WORD    wExtra;
				};
		struct
				{
				/* wProps */
				int             fBold: 1;   /* bold, italics in same position as chp */
				int             fItalic: 1;
				int             fStrike: 1;
				int             kul: 3;
		int		prq: 2;		/* pitch request */
				int             hps: 8;
				/* wExtra */
				int 		ibstFont: 8;    /* index into master font table */
		int		: 8;
				};
		};

#define fcidNil         0xffffffffL

/* F o n t  C a c h e  E n t r y */
struct FCE  
		{
		union FCID fcidActual;  /* what this entry really contains */
		int dxpOverhang;        /* overhang for italic/bold chars */
		int dypAscent;          /* ascent */
		int dypDescent;         /* descent */
		int dypXtraAscent;		/* ascent, but with recommended external
									leading added for printer fonts only */                                     
		int fPrinter: 1;        /* Is this cache entry for a printer font? */
		int fVisiBad: 1;        /* fTrue if space wd != visi space wd */
		int fFixedPitch: 1;	
		int fPrvw : 1;          /* fTrue if font cached while vfPrvwDisp */
	int : 3;
	int fGraphics: 1;	/* fTrue if font reqs printer graphics */
				/* same bit pos in byte as ffn, for native */
	int : 8;        /* spare */
		/* End of same as FTI structure */

	union {
			HQ	 hqrgdxp;	/* Width table for variable pitch font */
			uns  dxpWidth;	/* Width for fixed pitch font */
			};

		union FCID fcidRequest; /* request this entry satisfied */
		struct FCE *pfceNext;   /* next entry in lru list */
		struct FCE *pfcePrev;   /* prev entry in lru list */
		HFONT hfont;            /* windows' font object */
		};

/* F o n T  d e v i c e  I n f o r m a t i o n */
/* there's one of these for each device whose fonts are supplied by the cache */
/* First cbFtiFceSame bytes are identical to FCE structure */
struct FTI 
		{
		union FCID  fcid;       /* Describes font characteristics */
		int dxpOverhang;        /* overhang for italic/bold chars */
		int dypAscent;          /* ascent */
		int dypDescent;         /* descent */
		int dypXtraAscent;		/* ascent, but with recommended external
									leading added for printer fonts only */                                     
		int fPrinter: 1;        /* Is this fti for the printer font? */
		int fVisiBad: 1;	/* fTrue if space wd != visi space wd */
		int fFixedPitch: 1;	
		int fPrvw : 1;
		int         : 12;        /* spare */
		/* End of same as FCE structure */

	int	wSpare1;
		int dxpInch;            /* logical Pixel-per-inch density for this device */
		int dypInch;
		int dxpBorder;          /* border width for this device */
		int dypBorder;
		struct FCE  *pfce;      /* Pointer to cache entry for this font */
		HFONT hfont;            /* windows' font object */
								/* NOT included in shared FCE/FTI info because
									null value in FTI means font not
									actually selected, as for FormatLine
									font load optimization */
		int dxpExpanded;        /* user adjustment to character spacing */
		int fTossedPrinterDC: 1;/* we are managing without a printer DC */
		int         : 15;
	int rgdxp [256];
		};

#define cbFtiFceSame        (offset( FTI, wSpare1 ))

