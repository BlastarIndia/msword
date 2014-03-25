#define cfgrNil 0x3FFF


/* NOTE: The PRM structure is full of machine (byte-sex) dependancies! */

/*  *******************************************************************
*  WARNING *** WARNING *** WARNING *** WARNING *** WARNING *** WARNING
*  the PRM structure is also in structs.inc - be sure to fix it if you
*  change this... (WIN)
*  ******************************************************************* */

struct PRM
		{ /* PropeRty Modifier */
		union
				{
				int     prm;
				struct  {
						int     fComplex : 1; /* == fFalse */
						int     sprm : 7;
						int     val : 8;
						};
				struct
						{
						int     fComplex : 1; /* == fTrue */
#ifdef MAC
/* We can't store the hprc because we have only 15 bits. Instead we do:
		pprc = (struct PRC *) *(pwMax - prm.cfgr);
This result is invariant even as the finger table expands toward lower
addresses */
#else
/* for Win we store h/2 since lmem guarantees all handles will be even. Keep
cfgrPrc for Mac compatibility. */
#endif
						int     cfgrPrc : 15;
						};
				};
		};

#ifdef WIN
#define HprcFromPprmComplex(pprm) ((struct PRC **)(AssertX((pprm)->fComplex),(pprm)->cfgrPrc<<1))
#else
#define HprcFromPprmComplex(pprm) ((struct PRC **)(pfgrMac - (pprm)->cfgrPrc))
#endif /* WIN */

/*  *******************************************************************
*  WARNING *** WARNING *** WARNING *** WARNING *** WARNING *** WARNING
*  the PRC structure is also in structs.inc - be sure to fix it if you
*  change this... (WIN)
*  ******************************************************************* */

/* A PRC is the body of the complex PRM in core (in the heap). Its structure
is:
*/

struct PRC
		{
		int     fRef : 1; /* set during garb.coll. if PRC is referenced */
/* hash code is used to determine quickly if two prc's are not equal */
		int     wChecksum : 15; /* hash code computed on cb and grpprl */
/* pointer to the next older PRC, or hprcNil=0 if oldest PRC. */
		struct PRC **hprcNext;
/* number of bytes in the whole structure */
		int     bprlMac;
/* room for the prl's. */
		char    grpprl[0];
		};
#define cbPRCBase 6
/*
Where the PRL (PRM long) is used as byte array consisting of:
		0: sprm
		1: cb in some cases depending on sprm
		2 - n: contents and n depends on sprm (see CchPsprm)

The hprcNext chain starts with
		vhprc   pointing at the youngest PRC or hprcNil=0.
*/

/* Single property modifiers */

/*      ***     MUST agree with dnsprm in prcSubs.c     ***     */

#define sprmNoop         0       /* NoOp */

/* Table */
#define sprmTleReplace   1       /* Replace tle with handle */

/* Paragraph */
#define sprmPStc	 2	 /* change pap.stc */
#define sprmPStcPermute  3	 /* map stc for one doc into those of another*/
#define sprmPIncLvl	 4	 /* increment/decrement stc used for outlining*/
#define sprmPJc 	 5	 /* change pap.jc */
#define sprmPFSideBySide 6       /* change pap.fSideBySide */
#define sprmPFKeep	 7	 /* change pap.fKeep */
#define sprmPFKeepFollow 8	 /* change pap.fKeepFollow */
#define sprmPFPageBreakBefore 9  /* change pap.fPageBreakBefore */
#define sprmPBrcl	 10	 /* change pap.fBrcl */
#define sprmPBrcp	 11	 /* change pap.fBrcp */
#define sprmPNfcSeqNumb  12	 /* change pap.nfcSeqNumb */
#define sprmPNoSeqNumb	 13	 /* change pap.noSeqNumb */
#define sprmPFNoLineNumb 14	 /* change pap.fNoLineNumb */
#define sprmPChgTabsPapx 15	 /* change tabs. recorded in PAP FKP only */
#define sprmPDxaRight	 16	 /* change pap.dxaRight */
#define sprmPDxaLeft	 17	 /* change pap.dxaLeft */
#define sprmPNest	 18	 /* nest para by + or - from existing pap.dxaLeft */
#define sprmPDxaLeft1	 19	 /* change pap.dxaLeft1 */
#define sprmPDyaLine	 20	 /* change pap.dyaLine */
#define sprmPDyaBefore	 21	 /* change pap.dyaBefore */
#define sprmPDyaAfter	 22	 /* change pap.dyaAfter */
/* reserve spare para sprms since papxs are part of the file format */
#define sprmPChgTabs	 23	 /* change tab settings. version used in PRC*/
#define sprmPFInTable    24      /* change pap.fInTable*/
#define sprmPFTtp        25      /* change pap.fTtp */
#define sprmPDxaAbs      26      /* change pap.dxaAbs */
#define sprmPDyaAbs      27      /* change pap.dyaAbs */
#define sprmPDxaWidth    28      /* change pap.dxaWidth */
#define sprmPPc          29      /* change positioning code */
#define sprmPBrcTop      30
#define sprmPBrcLeft     31
#define sprmPBrcBottom   32
#define sprmPBrcRight    33
#define sprmPBrcBetween  34
#define sprmPBrcBar      35
#define sprmPFromText    36
#define sprmPSpare2      37     /* spare PAP sprms */
#define sprmPSpare3      38
#define sprmPSpare4      39
#define sprmPSpare5      40
#define sprmPSpare6      41
#define sprmPSpare7      42
#define sprmPSpare8      43
#define sprmPSpare9      44
#define sprmPSpare10     45
#define sprmPSpare11     46
#define sprmPSpare12     47
#define sprmPSpare13     48
#define sprmPSpare14     49
#define sprmPSpare15     50
#define sprmPSpare16     51
#define sprmPRuler       52

/* Character */
#define sprmCFStrikeRM   53      /* deleted text */
#define sprmCFRMark      54      /* added text */
#define sprmCFFldVanish  55      /* special hidden text */
#define sprmCSpare0      56
/* Character properties above here are not in user's control */
#define sprmCDefault     57      /* make "normal" character */
#define sprmCPlain       58		/* revert to base char props of stc */
#define sprmCSpare00     59
#define sprmCFBold       60      /* change chp.fBold */
#define sprmCFItalic     61      /* change chp.fItalic */
#define sprmCFStrike     62      /* change chp.fStrike */
#define sprmCFOutline    63      /* change chp.fOutline */
#define sprmCFShadow     64      /* change chp.fShadow */
#define sprmCFSmallCaps  65      /* change chp.fSmallCaps */
#define sprmCFCaps       66      /* change chp.fCaps */
#define sprmCFVanish     67      /* change chp.fVanish */
#define sprmCFtc         68      /* change chp.ftc */
#define sprmCKul         69      /* change chp.kul */
#define sprmCSizePos     70      /* change chp.hps */
#define sprmCQpsSpace    71      /* change chp.qpsSpace */
#define sprmCSpare000    72
#define sprmCIco         73      /* change chp.ico */
#define sprmCHps         74
#define sprmCHpsInc      75
#define sprmCHpsPos      76
#define sprmCHpsPosAdj   77
#define sprmCMajority    78
#define sprmCSpare6      79
#define sprmCSpare7      80
#define sprmCSpare8      81
#define sprmCSpare9      82
#define sprmCSpare10     83
#define sprmCSpare11     84
#define sprmCSpare12     85
#define sprmCSpare13     86
#define sprmCSpare14     87
#define sprmCSpare15     88
#define sprmCSpare16     89
#define sprmCSpare17     90
#define sprmCSpare18     91
#define sprmCSpare19     92
#define sprmCSpare20     93

/* Picture */
#ifdef MAC
#define sprmPicFScale	 94	/* change pic.fScale */
#define sprmPicScale	 95	/* change global pic scaling values */
#endif
#ifdef WIN
#define sprmPicBrcl      94     /* change pic.brcl border code */
#define sprmPicScale     95     /* change pic scaling/cropping values */
#endif
#define sprmPicSpare0    96     /* spare pic sprms */
#define sprmPicSpare1    97
#define sprmPicSpare2    98
#define sprmPicSpare3    99
#define sprmPicSpare4    100
#define sprmPicSpare5    101
#define sprmPicSpare6    102
#define sprmPicSpare7    103
#define sprmPicSpare8    104
#define sprmPicSpare9    105
#define sprmPicSpare10   106
#define sprmPicSpare11   107
#define sprmPicSpare12   108
#define sprmPicSpare13   109
#define sprmPicSpare14   110
#define sprmPicSpare15   111
#define sprmPicSpare16   112
#define sprmPicSpare17   113
#define sprmPicSpare18   114
#define sprmPicSpare19   115
#define sprmPicSpare20   116

/* Section */
#define sprmSBkc         117     /* change sep.bkc */
#define sprmSFTitlePage  118     /* change sep.fTitlePage */
#define sprmSCcolumns    119     /* change sep.ccolumns */
#define sprmSDxaColumns  120     /* change sep.dxaColumns */
#define sprmSFAutoPgn    121     /* change sep.fAutoPgn */
#define sprmSNfcPgn      122     /* change sep.nfcPgn */
#define sprmSDyaPgn      123     /* change sep.dyaPgn */
#define sprmSDxaPgn      124     /* change sep.dxaPgn */
#define sprmSFPgnRestart 125     /* change sep.fPgnRestart */
#define sprmSFEndnote    126     /* change sep.fEndnote */
#define sprmSLnc         127     /* change sep.lnc */
#define sprmSGrpfIhdt    128     /* change sep.grpfIhdt */
#define sprmSNLnnMod     129     /* change sep.nLnnMod */
#define sprmSDxaLnn      130     /* change sep.dxaLnn */
#define sprmSDyaHdrTop   131     /* change sep.dyaHdrTop */
#define sprmSDyaHdrBottom 132    /* change sep.dyaHdrBottom */
#define sprmSLBetween    133     /* change sep.fLBetween */
#define sprmSVjc         134     /* change sep.vjc */
#define sprmSLnnMin      135     /* change sep.lnnMin */
#define sprmSPgnStart    136     /* change sep.PgnStart */
#define sprmSSpare2      137     /* spare sprm */
#define sprmSSpare3      138     /* spare sprm */
#define sprmSSpare4      139     /* spare sprm */
#define sprmSSpare5      140     /* spare sprm */
#define sprmSSpare6      141     /* spare sprm */
#define sprmSSpare7      142
#define sprmSSpare8      143
#define sprmSSpare9      144
#define sprmSSpare10     145

/* Table */
#define sprmTJc          146     /* change tap.jc */
#define sprmTDxaLeft     147     /* change tap.dxaLeft */
#define sprmTDxaGapHalf  148     /* change tap.dxaGapHalf */
#define sprmTSpare6      149     /* spare table sprm */
#define sprmTSpare7      150     /* spare table sprm */
#define sprmTSpare8      151     /* spare table sprm */
#define sprmTDefTable    152     /* change tap.rgdxaCenter and tap.rgtc */
#define sprmTDyaRowHeight 153    /* set tap.dyaRowHeight */
#define sprmTSpare2      154     /* spare table sprm */
#define sprmTSpare3      155     /* spare table sprm */
#define sprmTSpare4      156     /* spare table sprm */
#define sprmTSpare5      157     /* spare table sprm */
#define sprmTInsert      158     /* add cells to table */
#define sprmTDelete      159     /* delete cells from table */
#define sprmTDxaCol      160     /* change column width of cell range */
#define sprmTMerge       161     /* set merge bits in TCs of cell range */
#define sprmTSplit       162     /* clear merge bits in TCs of cell range */
#define sprmTSetBrc      163     /* set brcs in TCs of cell range */

#define sprmFirstTNC     sprmTInsert
#define sprmLimTNC       sprmTSetBrc + 1

#define sprmMax          164     /* UPDATE WHEN ADDING SPRMS */
									/* (WIN) ALSO UPDATE sprmMax IN CONSTS.INC!! */
/* NOTE: sprmMax describes max sprms that may be merged */


/* all sprms above sprmTHistorical are non-commutative, non-composeable and
	must be recorded in historical order. */
#define sprmTHistoricals  sprmTDxaGapHalf

/* WARNING - MAC NATIVE depends on the ordering and number of the spra's
/* (there's a jump table in ApplyPrcSgc for the switch statement)
/**/
#define spraBit         0
#define spraByte        1
#define spraWord        2
#define spraCPlain      3
#define spraCFtc        4
#define spraCKul        5
#define spraCSizePos    6
#define spraSpec        7
#define spraCIco        8
#define spraCHpsInc     9
#define spraCHpsPosAdj  10
#define spraMax         11	/* MAC native depends on this, see above note */

/*  *******************************************************************
*  WARNING *** WARNING *** WARNING *** WARNING *** WARNING *** WARNING
*  the ESPRM structure is also in structs.inc - be sure to fix it if
*  you change this... (WIN)
*  ******************************************************************* */

struct ESPRM
		{
		int     fClobber : 1;
		int     b : 7;
		int     spra : 5;
		int     sgc : 3;
		int     cch : 4;
		int     dsprm : 4;
	int     fClobberExempt : 1;
	int     bExtra : 7;
		};

#define sgcPara         0

#define hpsSuperSub     12

#define dxaTabDelta     50
#define dxaCloseMin     25

/*  *******************************************************************
*  WARNING *** WARNING *** WARNING *** WARNING *** WARNING *** WARNING
*  the SIAP structure is also in structs.inc - be sure to fix it if
*  you change this... (WIN)
*  ******************************************************************* */

/* Structure of parameter for sprmCSizePos */
struct SIAP
		{
		int     hpsSize : 8;    /* Font size or 0 */
		int     cInc : 7;       /* Font size increment, 2's complement */
		int     fAdj : 1;       /* True if adjusting size for super/sub */
		int     hpsPos : 8;     /* Super/sub position, 2's complement, or 128 */
		};
/* MAC: compiler will give even address */
#define cbSIAP          WinMac((sizeof (struct SIAP)),3)
#define hpsPosNil       128
#define cIncMax         64


/* a SEBL is a parameter block which stores the current state of the editing
	process which takes place when a new sprm is to be added to a prl. */
struct SEBL  /* Sprm Edit Block */
		{
/* ptr to the next prl that must be examined in the earlier grpprl instance */
		char    *pgrpprlEarlier;
/* length of the prls in the old grpprl instance that remain to be examined */
		uns     cbEarlier;
/* pointer to the end of the grpprl that stores the results of the merge */
		char    *pgrpprlMerge;
/* length of the grpprl constructed by the merge */
		uns     cbMerge;
/* maximum storage allocated for grpprlMerge */
		uns     cbMergeMax;
/* ptr to grpprl that is to be merged with grpprlLater */
		char    *pgrpprlLater;
/* length of prls in grpprlLater */
		uns     cbLater;
		};
#define cbSEBL (sizeof(struct SEBL))

#define cbMaxPStcPermute 257
#define cbMaxPChgTabs 4 + (5 * itbdMax)
#define cbMaxAllOtherSprms 124

/* cbMaxGrpprl is the maximum number of bytes that can be used to store
	the list of sprms (grpprl) in a complex PRM. The worse case consists
	of a maximal sprmPStcPermute, a maximal sprmPChgTabs, and every other
	CHP, PAP, and SEP property changed.
		*/

#define cbMaxGrpprl  cbMaxPStcPermute + cbMaxPChgTabs + cbMaxAllOtherSprms

#define cbMaxGrpprlTable 198  /* all table sprms, not including sprmTDefTable;
								only used by MergeTableProps, which doesn't
								use sprmTDefTable. Look in fieldpic.c for
								derivation and proof of adequate size bz */

/* cbMaxGrpprlPapx is the size of the largest grpprl which can fit within
	an FKP that contains PAPXs */
#define cbMaxGrpprlPapx cbSector - 2*sizeof(char) - 2*sizeof(FC) - 7

#ifdef MAC /* WIN has vsci.ypSubSuper */
#define ypSubSuper      3
#endif

struct MPRC /* Maximal-length PRC */
		{
		int     fPrch : 1; /* for PRC always false. true for PRCH */
		int     fRef : 1; /* set during garb.coll. if PRC is referenced */
/* hash code is used to determine quickly if two prc's are not equal */
		int     wChecksum : 14; /* hash code computed on cb and grpprl */
/* pointer to the next older PRC, or hprcNil=0 if oldest PRC. */
		struct PRC **hprcNext;
/* number of bytes in the whole structure */
		int     bprlMac;
/* room for the prl's. */
		char    grpprl[cbMaxGrpprl];
		};

#define cchSepxMax 128
#define cchPapxMax cbMaxGrpprlPapx + 7

struct  SPX /* SPrms for eXceptions */
		{
		char    rgsprm[2];
		};

#define pcvhNinch  (3)   /* gray pc value - all 1 bits for a 2 bit field */
struct PCVH /* position code operand for sprmPPc */
	{
	union
		{
		struct
			{
			int : 3;
			int fVert : 1;
			int pcVert : 2;
			int pcHorz : 2;
			int : 8;
			};
		struct
			{
			int op : 8;
			int : 8;
			};
		};
	};


/* Word 3 sprms */
#ifdef MAC

/* Paragraph */
#define sprmPStcW3       2       /* change pap.stc */
#define sprmPStcPermuteW3 3       /* map stc for one doc into those of another*/
#define sprmPIncLvlW3    4       /* increment/decrement stc used for outlining*/
#define sprmPJcW3        5       /* change pap.jc */
#define sprmPFTableW3    6       /* change pap.fTable */
#define sprmPFKeepW3     7       /* change pap.fKeep */
#define sprmPFKeepFollowW3 8       /* change pap.fKeepFollow */
#define sprmPFPageBreakBeforeW3 9  /* change pap.fPageBreakBefore */
#define sprmPBrclW3      10      /* change pap.fBrcl */
#define sprmPBrcpW3      11      /* change pap.fBrcp */
#define sprmPNfcSeqNumbW3 12      /* change pap.nfcSeqNumb */
#define sprmPNoSeqNumbW3   13      /* change pap.noSeqNumb */
#define sprmPFNoLineNumbW3 14      /* change pap.fNoLineNumb */
#define sprmPChgTabsW3   15      /* change tabs.  */
#define sprmPDxaRightW3  16      /* change pap.dxaRight */
#define sprmPDxaLeftW3   17      /* change pap.dxaLeft */
#define sprmPNestW3      18      /* nest para by + or - from existing pap.dxaLeft */
#define sprmPDxaLeft1W3  19      /* change pap.dxaLeft1 */
#define sprmPDyaLineW3   20      /* change pap.dyaLine */
#define sprmPDyaBeforeW3 21      /* change pap.dyaBefore */
#define sprmPDyaAfterW3  22      /* change pap.dyaAfter */
/* reserve spare para sprms since papxs are part of the file format */
#define sprmPSpare0W3    23      /* spare sprms */
#define sprmPSpare1W3    24
#define sprmPSpare2W3    25
#define sprmPRulerW3     26
#define sprmCDefaultW3   27
#define sprmPRuler30W3   28      /* sprm abandoned as of Word 3.01. Don't use
					till file format changes again. */
/* Character */
#define sprmCPlainW3     29      /* revert to base char props of stc */
#define sprmCFBoldW3     30      /* change chp.fBold */
#define sprmCFItalicW3   31      /* change chp.fItalic */
#define sprmCFStrikeW3   32      /* change chp.fStrike */
#define sprmCFOutlineW3  33      /* change chp.fOutline */
#define sprmCFShadowW3   34      /* change chp.fShadow */
#define sprmCFSmallCapsW3  35      /* change chp.fSmallCaps */
#define sprmCFCapsW3     36      /* change chp.fCaps */
#define sprmCFVanishW3   37      /* change chp.fVanish */
#define sprmCFtcW3       38      /* change chp.ftc */
#define sprmCKulW3       39      /* change chp.kul */
#define sprmCSizePosW3   40      /* change chp.hps */
#define sprmCQpsSpaceW3  41      /* change chp.qpsSpace */
#define sprmCDocPicW3    42      /* store doc of graphics subdoc in chp.docPic */
#define sprmCSpare0W3    43      /* spare character sprms */
#define sprmCSpare1W3    44
#define sprmCSpare2W3    45
#define sprmCSpare3W3    46
#define sprmCSpare4W3    47
/* Picture */
#define sprmPicFScaleW3  48     /* change pic.fScale */
#define sprmPicScaleW3   49     /* change global pic scaling values */
#define sprmPicSpare0W3  50     /* spare pic sprms */
#define sprmPicSpare1W3  51
#define sprmPicSpare2W3  52
#define sprmPicSpare3W3  53
/* Section */
#define sprmSBkcW3       54      /* change sep.bkc */
#define sprmSFTitlePageW3 55      /* change sep.fTitlePage */
#define sprmSCcolumnsW3  56      /* change sep.ccolumns */
#define sprmSDxaColumnsW3 57      /* change sep.dxaColumns */
#define sprmSFAutoPgnW3  58      /* change sep.fAutoPgn */
#define sprmSNfcPgnW3    59      /* change sep.nfcPgn */
#define sprmSDyaPgnW3    60      /* change sep.dyaPgn */
#define sprmSDxaPgnW3    61      /* change sep.dxaPgn */
#define sprmSFPgnRestartW3 62      /* change sep.fPgnRestart */
#define sprmSFEndnoteW3  63      /* change sep.fEndnote */
#define sprmSLncW3       64      /* change sep.lnc */
#define sprmSGrpfIhdtW3  65      /* change sep.grpfIhdt */
#define sprmSNLnnModW3   66      /* change sep.nLnnMod */
#define sprmSDxaLnnW3    67      /* change sep.dxaLnn */
#define sprmSDyaHdrTopW3 68      /* change sep.dyaHdrTop */
#define sprmSDyaHdrBottomW3 69     /* change sep.dyaHdrBottom */

#define dsprmCDefault sprmCDefault - sprmCDefaultW3
#define dsprmCPlain sprmCPlain - sprmCPlainW3
#define dsprmPicFScale sprmPicFScale - sprmPicFScaleW3
#define dsprmSBkc sprmSBkc - sprmSBkcW3
#endif /* MAC */
