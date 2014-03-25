#ifdef NOASM
int RtfIn();
#endif /* NOASM */
int RtfOut();
int RtfStandardTap();
BOOL FTranslFields();

#define wVersionRTF (1)   /* used in the \rtf statement */

#define rooDoc          0x0001
#define rooInfo         0x0002
#define rooBegin        0x0003
#define rooEnd          0x0004
#define rooAll          0x000f
#define rooInternal     0x0010
#define rooNoPict       0x0020
#define rooSub          0x0040
#define roomSub         0xfff0

#define cchMaxRibl      256
#define cbRtfBuf        (cchMaxSz - 1)
#define ipcdLarge       2500  /* piece table is about 32K */

#define fsDefRTF (24)	 /* default rtf font size in half points */

#ifdef DEBUG
#define cchMaxLineRTF   79  /* widest expanse of RTF text w/o EOP */
#else
#define cchMaxLineRTF   255  /* widest expanse of RTF text w/o EOP */
#endif /* DEBUG */

#define icoRTFMax 50

#ifdef RTFDEFS

#ifdef MAC
#define slWord 0
#define slMacwrite 1
#define slWorks 2
#define slMacWord1 3
#define slIBMWord3 4
#endif /* MAC */


#define iBkmkEnd   1
#define iBkmkStart 2

#define iPicWBitmap 0
#define iPicWMetafile 1
#define iPicMacPict 2

#define racChngDest     0
#define racChngProp     1
#define racSpecChar     2
#define racSpecCharAct  3
#define racBinParm      4
#define racIgnore       5
#define racScanByDest   6

#define rrbaFlag        0
#define rrbaBit         1
#define rrbaByte        2
#define rrbaWord        3
#define rrbaUns		4
#define rrbaSpec        7

#define pgcPara 0
#define pgcChar 1
#define pgcSect 2
#define pgcDoc  3
#define pgcPic  4
#define pgcRibl 5
#define pgcBrc  6
#define pgcTable 7
#define pgcDtr  8

#define dof(fld)        offset(DOP, fld)
#define sof(fld)        offset(SEP, fld)
#define pof(fld)        offset(PAP, fld)
#define cof(fld)        offset(CHP, fld)

#define riblof(fld)     offset(RIBL, fld)
#define rffldof(fld)    offset(RFFLD, fld)  /* used by fields */
#define dtrof(fld)      offset(DTR, fld)    /* date/time rtf components */

#define picof(fld)      offset(PIC, fld)
#define tapof(fld)      offset(TAP, fld)
#define tcof(fld)       offset(TC, fld)

#define chRTFBegin      27
#define chRTFEnd        28
#define chRTFBinBreak   29
#define chBackslash     92

#define chRTFOpenBrk     '{'
#define chRTFCloseBrk    '}'


#define cwRIBL  (CwFromCch(sizeof (struct RIBL)))

#ifdef MAC
#define risNorm 	0
#define risB4ContSeq    1
#define risContWord     2
#define risNumScan      3
#define risAfContSeq    4
#define risScanPic      5
#define risB4TableName  6
#define risScanTableName 7
#define risDoPop        8
#define risExit         9
#define risB4BinCode    10
#define risBinCode      11
#define risScanBinPic   12
#define risScanByDest   13
#define risB4Private1   14
#define risB4SpecXeTc   15
#else /* WIN */
/* Reorder the ris codes to simplify the assembler jump table in N_RtfIn. */
/* Begin assembler ris cases */
#define risNorm 	0
#define risExit 	1
#define risB4ContSeq	2
#define risContWord	3
#define risNumScan	4
#define risScanPic      5
#define risB4Private1	6
#define risB4BinCode	7
#define risBinCode	8
#define risB4SpecXeTc	9
#define risB4TableName	10
#define risScanTableName 11
/* Begin pcode ris cases */
#define risAfContSeq	12
#define risScanByDest	13
#define risDoPop	14
#define risAddTableName 15
#endif /* WIN */


#define chsMac  0
#define chsPC   1
#define chsAnsi 2
#define chsPCA  3

#define fmcNil 		FF_DONTCARE
#define fmcRoman	FF_ROMAN
#define fmcSwiss	FF_SWISS
#define fmcModern	FF_MODERN
#define fmcDecor	FF_DECORATIVE
#define fmcScript	FF_SCRIPT

#define pgnxyDef (720)


/* bit and word "offsets" for bit fields used in the rgrrbword table
	ibitXname is the bit index of field name in struct X
	iwordeXname is the word index of field name in struct X

		where X is
			D for DOP
			S for SEP
			P for PAP
			C for CHP
			B for BRC
			T for TC
			PIC for PIC
			R for RIBL

	these defines are used in CkDefinesRtf(), a test routine that assures 
	that the structures referenced do not change and invalidate the values.
	Please update that routine (in debug.c) when you add defines.

*/

#define ibitCfBold 0 
#define iwordCfBold 0

#define ibitCfItalic 1 
#define iwordCfItalic 0

#define ibitCfStrike 2
#define iwordCfStrike 0

#define ibitCfOutline 3
#define iwordCfOutline 0

#ifdef MAC  /* fFldVanish for win */
#define ibitCfShadow 4 
#define iwordCfShadow 0
#endif /* MAC */

#define ibitCfSmallCaps 5 
#define iwordCfSmallCaps 0

#define ibitCfCaps 6
#define iwordCfCaps 0

#define ibitCfVanish 7 
#define iwordCfVanish 0

#define ibitCfRMark 8 
#define iwordCfRMark 0

#define ibitDfFacingPages 0 
#define iwordDfFacingPages 0

#define ibitDfWidowControl 1 
#define iwordDfWidowControl 0

#define ibitDfFtnRestart 0 
#define iwordDfFtnRestart 1

#define ibitDfWide 7 
#define iwordDfWide 0

#define ibitDfRevMarking 15 
#define iwordDfRevMarking 2

#define ibitDfBackup 0 
#define iwordDfBackup 3

#define ibitDfMirrorMargins 5 
#define iwordDfMirrorMargins 3 

#define ibitTfFirstMerged 0 
#define iwordTfFirstMerged 0 

#define ibitTfMerged 1 
#define iwordTfMerged 0 

#define ibitFFfResDirty 0 
#define iwordFFfResDirty 0 

#define ibitFFfResEdit 1 
#define iwordFFfResEdit 0 

#define ibitFFfLocked 2 
#define iwordFFfLocked 0 

#define ibitFFfPrivRes 3 
#define iwordFFfPrivRes 0 

#define ibitRfRevisions 13 
#define iwordRfRevisions 0 

#ifdef MAC
#define cbBlockPic (cbPIC + cbPEBase)
#endif /* MAC */

#ifdef WIN
#define cbBlockPic (cbPIC)
#endif /* WIN */


/* macros */

#define IifdFromRds(rds)  (rds - rdsTitle)

/* return true if the range cpFirst - cpLim contains the start or end of
	a bookmark. Only callable from within RtfOut.
*/

#define FBkmksInRun(cpFirst, cpLim)  ( (bkmkRtf.cpFirstBkmk >= cpFirst && \
				bkmkRtf.cpFirstBkmk <= cpLim) || (bkmkRtf.cpLimBkmk >= cpFirst \
				&& bkmkRtf.cpLimBkmk <= cpLim) )



struct BKMKRTF
	{
	int fHaveBkmks;
	int fHaveBkmkInRun;
	int ibkmkMac;
	int icpLimBkmk;
	int icpFirstBkmk;
	CP	cpLimBkmk;
	CP	cpFirstBkmk;
	int **hmpIbklIbst;
	};

#endif /* RTFDEFS */





/* Date and Time - Excel's structure */

struct DTR
		{
		int yr;         /* 4-digit */
		int mon;        /* 1-12 */
		int dom;        /* 1-31 */
		int hr;         /* 0-23 */
		int mint;       /* 0-59 */
		int sec;        /* 0-59 */
		int wdy;        /* 1-7 for Sunday-Saturday */
		};

#define cwDTR  (CwFromCch(sizeof (struct DTR)))


struct RSYM  /* RTF symbol table entry */
	{
	int     val : 8;
	int     fPassVal : 1;
	int     rac: 7;
	union
		{
		int ipfnAction;
		int irrbProp;
		int fSpec;
		};
	};

/* Note: be sure the following structures stay multi
ples of words */

struct RFFLD
	/* FLD structure flags */
	{
		int fResultDirty : 1; /* user has edited or formatted the result */
		int fResultEdited : 1;/* user has inserted or deleted to/fm result */
		int fLocked : 1;    /* this field is locked from recalc */
		int fPrivateResult : 1; /* never show the result of this field */
		int fSpare : 12;
	};
#define cwRFFLD  (CwFromCch(sizeof (struct RFFLD)))

struct RCPFLD  /* RTF CP area for fields */
	{
		CP      dcpInst;  /* size of instruction text */
		CP      dcpRslt;  /* size of result text */
	struct  RFFLD rffld; /* flags */
		};
#define cwRCPFLD  (CwFromCch(sizeof (struct RCPFLD)))

struct RCPXETC  /* RTF CP area for Xe and TC entries */
	{
		CP      cpFirstEntry; 
		CP      cpLimEntry; 
		CP      cpFirstText; 
		CP      cpLimText; 
	struct  
			{
			int fVanishEntry : 1;
			int fVanishText : 1;
			int fInEntry : 1;
			int fInText : 1;
			int fSpare : 12;
			};
		int docTemp;   /* holder for moving text */
		};
#define cwRCPXETC  (CwFromCch(sizeof (struct RCPXETC)))


struct RHPCCHW   /* used in some RtfOut calls */
		{
		HANDLE h;
		long lcbMac;   /* current handle used size */
		long lcbMax;   /* current handle allocated size */
		int wAlloc;     /* GMEM for allocations */
		};

struct RSTN   /* RTF Stack Node */
	{
	int     bPrev;
	int     bCharInfo;
	int     bParaInfo;
	int     bSectInfo;
	int     bTableInfo;
	int     rds;
	CP      cpFirst;
		struct  RCPFLD **hrcpfld;   /* used only by fields */
	};

struct RRB
	{ /* RTF Record Block */
	int     rrba : 3;
	int     pgc : 7;
	int     b : 6;
	int     w;
	};




struct REBL
	{ /* RTF Environment Block */

#ifdef MAC
	int	sl;
#endif /* MAC */

	int     (*pfnWrite)();
	int     *pArgWrite;
	char    rgch[cchMaxSz - 1];
	};
/* cch of worse case RTF tab string: \tqdec\tlhyph\tx99999  */
#define cchMaxTabRtf 21
/* cch of worse case RTF property string: \linerestart99999  */
#define cchMaxPropRtf 17

struct RARF    /* RftAppendRgchtoFn parm */
	{
	int fn;
	};



#ifdef MAC

struct FTCM     /* ftc map entry */
	{
	int ftcTo;   /* order is significant for searching */
	int ftcFrom;
	};

#else /* !MAC */

	/* Really the first iftcMac entries of rgwFtc are the rgwFtcFrom
		while the iftcMac entries beginning at rgwFtc[iftcMax] are the
		rgwFtcTo.  This is better for opus than using a plex of ftcm's
		because it allows us to use IScanLprgw on the rgwFtcFrom to
		find an entry for an ftcFrom, and simplifies a bunch of code.
	*/
struct FTCMAP	/* ftc map structure */
	{
	int iMac;
	int iMax;
	int rgwFtc[1];
	};

#endif /* !MAC */

struct FTF    /* ftc to fmc lookup entry */
	{
	int ftc;
	int fmc;
	};

struct RIBL
	{
	int	fInBody : 1;
	int	fCancel : 1;
	int	fNoColor : 1;
	int	fXeTcQOpen : 1;
	int	fSideBySide : 1;
	int	fHdr: 1;  /* hdr/ftr entered in this section */
	int	fFtr: 1;
#ifdef WIN
	int	fSpares: 3;
	int	fInXeTc : 1;  /* for hack of killing nested xetc/xert */
	int	fInXeRT : 1;
	int	fFldImport: 1;	/* true if we just processed an import field */
	int	fRevisions : 1;
	int	fDisregardPic : 1;
	int	fHiNibble : 1;
#else /* !WIN */
	int	fSpares: 9;
#endif /* !WIN */

#ifdef MAC
	char    sl;
#endif /* MAC */

	char    ris;
	char    i;
	
	char    b;
	char    bPic;
	int     bchSeqLim;
	int     bchNumLim;
	int     bchData;
	
#ifdef MAC
	char    *pdfpMacWr;
#endif /* MAC */

	char    fStcNextOK;
	char    fNextIsDest; /* if true and don't recognize dest, skip */

	int     iGrid;
	int     mts;

	int     chs;     /* type of character set */
	int     fmc;     /* font family code */
	int     deff;    /* default ftc to emit or read in */
	int     pgnStart; /* doc props Opus stores in first section */
	int     lineStart;
	int		vern;	/* rtf generator version #, from info block */
	struct ESTCP estcp;
	char    fFonttblDef;
	char    cRed;
	char    cGreen; 
	char    cBlue; 

	long    lcb;
	uns     dxaPgn;
	uns     dyaPgn;
	uns     dyaHdrBottom;
	uns     dxaRight;
	uns     dyaBottom;
	int     rds;    /* RTF destination */
	int     doc;
	int     ihdt;
	CP      cpRdsFirst;
	int     iNibble;

#ifdef WIN
	int     iTypePic;
#endif /* WIN */

#ifdef MAC
	int     fMacPict;
	int     itbdm1Mac;
	struct  TBDM1 tbdm1;
	int     fBeginMacWrPic;
#endif /* MAC */

		char    stUsrInitl[ichUsrInitlMax];
	char    **hrgbStack;
	int     bStackLim;

	int rhc;
	FC  fcPicFirst;

#ifdef MAC
	/* struct CAB **hcab;   * opus code never uses this */
#endif /* MAC */

	char fWindowMessage;
	char fDopPropsRecorded;
#ifdef MAC
	struct	PL **hplftcm;
#else /* !MAC */
	struct	FTCMAP **hftcmap;
#endif /* !MAC */
	int     fPermOK;
#ifdef MAC
	int     (*pfnCharDefault)();
#endif /* MAC */
	int     cbCharInfo;

#ifdef MAC
	int     (*pfnParaDefault)();
#endif /* MAC */
	int     cbParaInfo;

#ifdef MAC
	int     (*pfnSectDefault)();
#endif /* MAC */
	int     cbSectInfo;

#ifdef MAC
	int     (*pfnTableDefault)();
#endif /* MAC */
	int     cbTableInfo; 
	int     fParaBrc;
	int     itc; 
	int     ibrc; 

	union
			{
			struct TBD     tbd;
			int    dummy;
			};
		struct RCPFLD **hrcpfld;   /* used only by fields */
		struct STTB   **hsttbGrid; /* used only by Excel grids */
		struct RCPXETC **hrcpxetc;   /* used only by index, toc entries */
	int ibSea;
		char    rgbSea[cbSEA];
	char    rgch[cchMaxRibl];
	union
			{
			struct PAP      pap;
#ifdef MAC
			struct PFP      pfp;
			struct PAPM1    papm1;
#endif /* MAC */

			char   rgbPara[sizeof(struct PAP)];
			};
	union
		{
		struct CHP      chp;
#ifdef MAC
		struct CFP      cfp;
#endif /* MAC */

		char   rgbChar[sizeof(struct CHP)];
		};
	union
		{
		struct SEP      sep;

#ifdef MAC
		struct SFP      sfp;
		struct SEPM1    sepm1;
#endif /* MAC */

		char   rgbSect[(sizeof(struct SEP))];
		};
	union
			{
			struct TAP tap;
			char rgbTable[(sizeof(struct TAP))];
			};
	char rgbStcPermutation[256];
		char **hstGrid;
	int icoMac; 
	int mpicoOldicoNew[icoRTFMax]; 
			/* note: variable size PIC must be at end of struct RIBL! */
	struct PIC pic;
	};

struct FLDS  /* field state for stack */
	{
	int flt:8;
	int fOpenQuote:1;
	int fInTRSwText:1; /* have found text associated with t or r switch */
	int grpfOpenSw:3; 
	int fParInFld:1;
	int fTableOut:1; /* have written out table props in field */

	int fSpare:1;
	};

#define grpfSwNone (0)
#define grpfSwXeR  (1)
#define grpfSwXeT  (2)
#define grpfSwTcF  (3)
#define grpfSwTcL  (4)
