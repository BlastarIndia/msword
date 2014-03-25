/* P R C S U B S . C */

#ifdef MAC
#define DIALOGS
#define WINDOWS
#define CONTROLS
#include "toolbox.h"
#endif /* MAC */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "doc.h"
#include "props.h"
#include "prm.h"
#include "disp.h"
#include "sel.h"
#include "ruler.h"
#include "pic.h"
#include "cmd.h"
#include "ch.h"
#include "file.h"
#include "style.h"
#include "outline.h"
#include "debug.h"
#include "error.h"

#ifdef WIN
#include "field.h"
#define AUTOSAVE
#include "autosave.h"
#include "idle.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#include "edstyle.hs"
#endif

/* E X T E R N A L S */

extern struct SEL       selCur;
extern struct DOD       **mpdochdod[];
extern struct MERR      vmerr;
extern struct CA        caPara;
extern struct CA        caSect;
extern int              vdocFetch;
extern struct PRC       **vhprc;
extern CP               CpLimNoSpaces();
extern struct CHP	vchpStc;
extern struct PAP       vpapFetch;
extern struct RULSS	vrulss;
extern struct UAB       vuab;
extern int              vdocPapLast;
extern BOOL		vfSeeSel;

#ifdef WIN
extern IDF		vidf;
extern ASD  asd;
extern struct CHP	vchpGraySelCur;
extern struct PAP	vpapSelCur;
extern struct PAP	vpapGraySelCur;
#endif

#ifdef MAC
extern uns              *pfgrMac;
extern DIALOGREC far    *vqDialog; /* far pointer to current dialog record */
#endif /* MAC */
extern int              vfStyleError;
extern int              vdocStsh;
extern int              vstcStyle;

#ifdef JR
extern int              vfJRProps;
#endif


#ifdef PROTOTYPE
#include "prcsubs.cpt"
#endif /* PROTOTYPE */

/* G L O B A L S */
#define pof(field) offset(PAP, field)
#define cof(field) offset(CHP, field)
#define sof(field) offset(SEP, field)
#define tof(field) offset(TAP, field)
/* Description of ESPRM fields:
	fClobber --- when a new grpprl is merged with an old grpprl, a sprm
	of the input grpprl whose fClobber bit is true, supresses copying
	from the old grpprl to the merged grpprl of any sprms whose
	sprm code value is greater than that of the new sprm
	which have a sgc (Sprm Group Code) equal to that of the new sprm.
	For example, applying a sprmPStc, causes all paragraph properties
	to be reset so any paragraph props in the output grpprl may not
	be copied to the merged grpprl.
	b ---------- byte offset of the field in the related
	property to be modified by the sprm for spra != spraBit. Bit offset
	within the first word of the structure for spra == spraBit.
	spra ------- (SPRm Action) specifies how the field is to be modified.
	spraBit can only modify a bit in the first word of
	the property.  spraSpec is processed by special
	handling functions.  (e.g. spraTabs)
	sgc -------- Sprm Group Code.  Identifies the property
	whose field is modified by the sprm.
	possible sgc values are sgcPap, sgcChp, sgcSep, sgcPic, and sgcTle.
	cch -------- defines the length of sprm.  If it is 0,
	the length of the sprm - 2 is stored in the byte after the sprm code.
	If it is 1, no operand follows the sprm code.
	If it is 2, the sprm operand is the byte following the sprm code.
	If it is 3, the sprm operand is the word following the sprm code.
	If it is 4, the sprm operand is the 3 bytes following the sprm code.
	dsprm ------ defines the composition zone.	If it is 0,
	it does not compose and kills old sprm.  If it is
	1, it composes with instances of itself when merges occur. If it is n,
	during merges it composes with instances of itself and clobbers
	instances of the next n - 1 sprms listed in dnsprm.
	fClobberExempt  --- sprm is exempt from being deleted by a fClobber sprm*/

/* description certified to be true by DLL on this 19th day of August, 1987 */

/* NOTE NOTE NOTE Paragraph sprms are stored in PAP FKPS. When a file is
	fast saved any grpprls linked to the piece table are saved with the file.
	This means that once a product is shipped, the descriptions and actions of
	sprms become part of the file format and may not be changed without
	causing invalidation of user's files. Since MacWord 3.01 has shipped
	already, its sprms may not be altered. Once Opus ships, its sprm definitions
	will become unalterable. To acheive new functionality, sprms must be added
	to spare slots or to the end of the table. */

/* This table corresponds to sprm's in prmdefs.h */
	struct ESPRM  dnsprm[sprmMax + 2] = {
	/*sprm  { fClobber, b,         spra,     sgc,      cch, dsprm, fClobberExempt} 


*/

/* 0: sprmNoop */
{    
	0, 0,                     0,        0,         1, 0, fFalse },
	
/* 1: sprmTleReplace */
{ 0, 0,                     spraSpec, 0,         3, 0, fFalse },
	
/* 2: sprmPStc */
{ 1, pof(stc),              spraSpec, sgcPap,    2, 0, fFalse },
	
/* 3: sprmPStcPermute*/
{ 0, pof(stc),              spraSpec, sgcPap,    0, 1, fFalse },
	
/* 4: sprmPIncLvl */
{ 1, pof(stc),              spraSpec, sgcPap,    2, 1, fFalse },
	
/* 5: sprmPJc */
{ 0, pof(jc),               spraByte, sgcPap,    2, 0, fFalse },
	
/* 6: sprmPFSideBySide */
{ 0, pof(fSideBySide),      spraByte, sgcPap,    2, 0, fFalse },
	
/* 7: sprmPFKeep  */
{ 0, pof(fKeep),            spraByte, sgcPap,    2, 0, fFalse },
	
/* 8: sprmPFKeepFollow */
{ 0, pof(fKeepFollow),      spraByte, sgcPap,    2, 0, fFalse },
	
/* 9: sprmPFPageBreakBefore */
{ 0, pof(fPageBreakBefore), spraByte, sgcPap,    2, 0, fFalse },
	
/* 10: sprmPBrcl */
{ 0, pof(brcl),             spraByte, sgcPap,    2, 0, fFalse },
	
/* 11: sprmPBrcp  */
{ 0, pof(brcp),             spraByte, sgcPap,    2, 0, fFalse },
	
/* 12: sprmPNfcSeqNumb */
{ 0, pof(nfcSeqNumb),       spraByte, sgcPap,    2, 0, fFalse },
	
/* 13: sprmPNoSeqNumb  */
{ 0, pof(nnSeqNumb),        spraByte, sgcPap,    2, 0, fFalse },
	
/* 14: sprmPFNoLineNumb */
{ 0, pof(fNoLnn),           spraByte, sgcPap,    2, 0, fFalse },
	
/* used to record tab changes from underlying style in PAPXs. never will be
	applied to document. */
/* 15: sprmPChgTabsPapx  */
{ 0,
			 pof(rgtbd),            spraSpec, sgcPap,  0,    1, fFalse },
	
/* 16: sprmPDxaRight   */
{ 0, pof(dxaRight),         spraWord, sgcPap,  3,    0, fFalse },
	
/* 17: sprmPDxaLeft    */
{ 0, pof(dxaLeft),          spraWord, sgcPap,  3,    2, fFalse },
	
/* 18: sprmPNest   */
{ 0, pof(dxaLeft),          spraSpec, sgcPap,  3,    1, fFalse },
	
/* 19: sprmPDxaLeft1 */
{ 0, pof(dxaLeft1),         spraWord, sgcPap,  3,    0, fFalse },
	
/* 20: sprmPDyaLine  */
{ 0, pof(dyaLine),          spraWord, sgcPap,  3,    0, fFalse },
	
/* 21: sprmPDyaBefore */
{ 0, pof(dyaBefore),        spraWord, sgcPap,  3,    0, fFalse },
	
/* 22: sprmPDyaAfter */
{ 0, pof(dyaAfter),         spraWord, sgcPap,  3,    0, fFalse },
	
/* 23: sprmPChgTabs */
{ 0, pof(rgtbd),            spraSpec, sgcPap,  0,    1, fFalse },
	
/* 24: sprmPFInTable */
{ 0, pof(fInTable),         spraByte, sgcPap,  2,    0, fTrue },
	
/* 25: sprmPFTtp */
{ 0, pof(fTtp),             spraByte, sgcPap,  2,    0, fTrue },
	
/* 26: sprmPDxaAbs */
{ 0, pof(dxaAbs),           spraWord, sgcPap,  3,    0, fFalse },
	
/* 27: sprmPDyaAbs */
{ 0, pof(dyaAbs),           spraWord, sgcPap,  3,    0, fFalse },
	
/* 28: sprmPDxaWidth */
{ 0, pof(dxaWidth),         spraWord, sgcPap,  3,    0, fFalse },
	
/* 29: sprmPPc */
{ 0, 0,                     spraSpec, sgcPap,  2,    0, fFalse },
	
/* 30: sprmPBrcTop */
{ 0, pof(brcTop),           spraWord, sgcPap,  3,    0, fFalse },
	
/* 31: sprmPBrcLeft */
{ 0, pof(brcLeft),          spraWord, sgcPap,  3,    0, fFalse },
	
/* 32: sprmPBrcBottom */
{ 0, pof(brcBottom),        spraWord, sgcPap,  3,    0, fFalse },
	
/* 33: sprmPBrcRight */
{ 0, pof(brcRight),         spraWord, sgcPap,  3,    0, fFalse },
	
/* 34: sprmPBrcBetween */
{ 0, pof(brcBetween),       spraWord, sgcPap,  3,    0, fFalse },
	
/* 35: sprmPBrcBar */
{ 0, pof(brcBar),           spraWord, sgcPap,  3,    0, fFalse },
	
/* 36: sprmPFromText */
{ 0, pof(dxaFromText),      spraWord, sgcPap,  3,    0, fFalse },
	
/* NOTE: if the cch field for the spare sprms are changed to new values
	for future versions of MacWord, old versions of the program will be
	unable to work with the new file format induced by the change. */
/* 37: sprmPSpare2 */
{ 0,
			 0,                     spraWord, sgcPap,  0,    0, fFalse },
	
/* 38: sprmPSpare3 */
{ 0, 0,                     spraWord, sgcPap,  0,    0, fFalse },
	
/* 39: sprmPSpare4 */
{ 0, 0,                     spraWord, sgcPap,  0,    0, fFalse },
	
/* 40: sprmPSpare5 */
{ 0, 0,                     spraWord, sgcPap,  0,    0, fFalse },
	
/* 41: sprmPSpare6 */
{ 0, 0,                     spraWord, sgcPap,  0,    0, fFalse },
	
/* 42: sprmPSpare7 */
{ 0, 0,                     spraWord, sgcPap,  0,    0, fFalse },
	
/* 43: sprmPSpare8 */
{ 0, 0,                     spraWord, sgcPap,  0,    0, fFalse },
	
/* 44: sprmPSpare9 */
{ 0, 0,                     spraWord, sgcPap,  0,    0, fFalse },
	
/* 45: sprmPSpare10*/
{ 0, 0,                     spraWord, sgcPap,  0,    0, fFalse },
	
/* 46: sprmPSpare11*/
{ 0, 0,                     spraWord, sgcPap,  0,    0, fFalse },
	
/* 47: sprmPSpare12*/
{ 0, 0,                     spraWord, sgcPap,  0,    0, fFalse },
	
/* 48: sprmPSpare13*/
{ 0, 0,                     spraWord, sgcPap,  0,    0, fFalse },
	
/* 49: sprmPSpare14*/
{ 0, 0,                     spraWord, sgcPap,  0,    0, fFalse },
	
/* 50: sprmPSpare15*/
{ 0, 0,                     spraWord, sgcPap,  0,    0, fFalse },
	
/* 51: sprmPSpare16*/
{ 0, 0,                     spraWord, sgcPap,  0,    0, fFalse },
	
/* 52: sprmPRuler *** all sgcPara sprms kill sprmPRuler *** */
{ 0, pof(rgtbd),            spraSpec, sgcPap,  0,
			    0, fFalse },
	
/* 53: sprmCFStrikeRM */
{ 0, 2,                      spraBit, sgcChp,  2,    0, fFalse },
	
/* 54: sprmCFRMark */
{ 0, 8,                     spraBit,  sgcChp,  2,    0, fFalse },
	
/* 55: sprmCFFldVanish *** */
{ 0, 4,                     spraBit,  sgcChp,  2,    0, fFalse },
	
/* 56: sprmCSpare0 */
{ 0, 0,                     spraSpec, sgcChp,  0,    0, fFalse },
	
/* 57: sprmCDefault */
{ 0, 0,                     spraSpec, sgcChp,  0,    0, fFalse },
	
/* 58: sprmCPlain */
{ 1, 0,                   spraCPlain, sgcChp,  1,    0, fTrue },
	
/* 59: sprmCSpare00 */
{ 0, 0,                     spraSpec, sgcChp,  0,    0, fFalse },
	
/* 60: sprmCFBold */
{ 0, 0,                      spraBit, sgcChp,  2,    0, fFalse },
	
/* 61: sprmCFItalic */
{ 0, 1,                      spraBit, sgcChp,  2,    0, fFalse },
	
/* 62: sprmCFStrike */
{ 0, 2,                      spraBit, sgcChp,  2,    0, fFalse },
	
/* 63: sprmCFOutline */
{ 0, 3,                      spraBit, sgcChp,  2,    0, fFalse },
	
/* 64: sprmCFShadow */
{ 0, 4,                      spraBit, sgcChp,  2,    0, fFalse },
	
/* 65: sprmCFSmallCaps */
{ 0, 5,                      spraBit, sgcChp,  2,    0, fFalse },
	
/* 66: sprmCFCaps */
{ 0, 6,                      spraBit, sgcChp,  2,    0, fFalse },
	
/* 67: sprmCFVanish */
{ 0, 7,                      spraBit, sgcChp,  2,    0, fFalse },
	
/* 68: sprmCFtc  */
{ 0, 11,                    spraCFtc, sgcChp,  3,    0, fTrue },
	
/* 69: sprmCKul  */
{ 0, 13,                    spraCKul, sgcChp,  2,    0, fFalse },
	
/* 70: sprmCSizePos  */
{ 0, cof(hps),              spraCSizePos, sgcChp,  4,    1, fTrue },
	
/* 71: sprmCQpsSpace */
{ 0, 15,                    spraSpec, sgcChp,  2,    0, fTrue },
	
/* 72: sprmCSpare000 */
{ 0, 0,                     spraSpec, sgcChp,  0,    0, fFalse },
	
/* 73: sprmCIco */
{ 0, 10,                    spraCIco,  sgcChp,  2,    0, fFalse },
	
/* 74: sprmCHps */
{ 0, cof(hps),              spraByte, sgcChp,  2,    0, fTrue },
	
/* 75: sprmCHpsInc */
{ 0, cof(hps),              spraCHpsInc, sgcChp,  2,    0, fTrue },
	
/* 76: sprmCHpsPos */
{ 0, cof(hpsPos),           spraByte, sgcChp,  2,    0, fTrue },
	
/* 77: sprmCHpsPosAdj */
{ 0, cof(hpsPos),           spraCHpsPosAdj, sgcChp,  2,    0, fTrue },
	
/* 78: sprmCMajority */
{ 0, 0,                     spraSpec, sgcChp,  0,    0, fFalse },
	
/* 79: sprmCSpare6 */
{ 0, 0,                     spraSpec, sgcChp,  0,    0, fFalse },
	
/* 80: sprmCSpare7 */
{ 0, 0,                     spraSpec, sgcChp,  0,    0, fFalse },
	
/* 81: sprmCSpare8 */
{ 0, 0,                     spraSpec, sgcChp,  0,    0, fFalse },
	
/* 82: sprmCSpare9 */
{ 0, 0,                     spraSpec, sgcChp,  0,    0, fFalse },
	
/* 83: sprmCSpare10*/
{ 0, 0,                     spraSpec, sgcChp,  0,    0, fFalse },
	
/* 84: sprmCSpare11*/
{ 0, 0,                     spraSpec, sgcChp,  0,    0, fFalse },
	
/* 85: sprmCSpare12*/
{ 0, 0,                     spraSpec, sgcChp,  0,    0, fFalse },
	
/* 86: sprmCSpare13*/
{ 0, 0,                     spraSpec, sgcChp,  0,    0, fFalse },
	
/* 87: sprmCSpare14*/
{ 0, 0,                     spraSpec, sgcChp,  0,    0, fFalse },
	
/* 88: sprmCSpare15*/
{ 0, 0,                     spraSpec, sgcChp,  0,    0, fFalse },
	
/* 89: sprmCSpare16*/
{ 0, 0,                     spraSpec, sgcChp,  0,    0, fFalse },
	
/* 90: sprmCSpare17*/
{ 0, 0,                     spraSpec, sgcChp,  0,    0, fFalse },
	
/* 91: sprmCSpare18*/
{ 0, 0,                     spraSpec, sgcChp,  0,    0, fFalse },
	
/* 92: sprmCSpare19*/
{ 0, 0,                     spraSpec, sgcChp,  0,    0, fFalse },
	
/* 93: sprmCSpare20*/
{ 0, 0,                     spraSpec, sgcChp,  0,    0, fFalse },
	

#ifdef MAC
/* 94: sprmPicFScale */
{ 0, 0,                     spraSpec, sgcPic,  2,    0, fFalse },
	
/* 95: sprmPicScale */
{ 0, 0,                     spraSpec, sgcPic,  0,    1, fFalse },
	
#endif /* MAC */
#ifdef WIN
/* 94: sprmPicBrcl */
{ 0, 0,                     spraSpec, sgcPic,  2,    0, fFalse },
	
/* 95: sprmPicScale */
{ 0, 0,                     spraSpec, sgcPic,  0,    0, fFalse },
	
#endif /* WIN */
/* 96: sprmPicSpare0 */
{ 0, 0,                     spraSpec, sgcPic,  0,    0, fFalse },
	
/* 97: sprmPicSpare1 */
{ 0, 0,                     spraSpec, sgcPic,  0,    0, fFalse },
	
/* 98: sprmPicSpare2 */
{ 0, 0,                     spraSpec, sgcPic,  0,    0, fFalse },
	
/* 99: sprmPicSpare3 */
{ 0, 0,                     spraSpec, sgcPic,  0,    0, fFalse },
	
/*100: sprmPicSpare4 */
{ 0, 0,                     spraSpec, sgcPic,  0,    0, fFalse },
	
/*101: sprmPicSpare5 */
{ 0, 0,                     spraSpec, sgcPic,  0,    0, fFalse },
	
/*102: sprmPicSpare6 */
{ 0, 0,                     spraSpec, sgcPic,  0,    0, fFalse },
	
/*103: sprmPicSpare7 */
{ 0, 0,                     spraSpec, sgcPic,  0,    0, fFalse },
	
/*104: sprmPicSpare8 */
{ 0, 0,                     spraSpec, sgcPic,  0,    0, fFalse },
	
/*105: sprmPicSpare9 */
{ 0, 0,                     spraSpec, sgcPic,  0,    0, fFalse },
	
/*106: sprmPicSpare10*/
{ 0, 0,                     spraSpec, sgcPic,  0,    0, fFalse },
	
/*107: sprmPicSpare11*/
{ 0, 0,                     spraSpec, sgcPic,  0,    0, fFalse },
	
/*108: sprmPicSpare12*/
{ 0, 0,                     spraSpec, sgcPic,  0,    0, fFalse },
	
/*109: sprmPicSpare13*/
{ 0, 0,                     spraSpec, sgcPic,  0,    0, fFalse },
	
/*110: sprmPicSpare14*/
{ 0, 0,                     spraSpec, sgcPic,  0,    0, fFalse },
	
/*111: sprmPicSpare15*/
{ 0, 0,                     spraSpec, sgcPic,  0,    0, fFalse },
	
/*112: sprmPicSpare16*/
{ 0, 0,                     spraSpec, sgcPic,  0,    0, fFalse },
	
/*113: sprmPicSpare17*/
{ 0, 0,                     spraSpec, sgcPic,  0,    0, fFalse },
	
/*114: sprmPicSpare18*/
{ 0, 0,                     spraSpec, sgcPic,  0,    0, fFalse },
	
/*115: sprmPicSpare19*/
{ 0, 0,                     spraSpec, sgcPic,  0,    0, fFalse },
	
/*116: sprmPicSpare20*/
{ 0, 0,                     spraSpec, sgcPic,  0,    0, fFalse },
	


/*117: sprmSBkc */
{ 0, sof(bkc),              spraByte, sgcSep,  2,    0, fFalse },
	
/*118: sprmSFTitlePage */
{ 0, sof(fTitlePage),       spraByte, sgcSep,  2,    0, fFalse },
	
/*119: sprmSCcolumns */
{ 0, sof(ccolM1),           spraWord, sgcSep,  3,    0, fFalse },
	
/*120: sprmSDxaColumns */
{ 0, sof(dxaColumns),       spraWord, sgcSep,  3,    0, fFalse },
	
#ifdef MAC
/*121: sprmSFAutoPgn */
{ 0, sof(fAutoPgn),         spraByte, sgcSep,  2,    0, fFalse },
	
#endif /* MAC */
#ifdef WIN
/*121: sprmSFAutoPgn */
{ 0, 0,                     spraByte, sgcSep,  2,    0, fFalse },
	
#endif /* WIN */
/*122: sprmSNfcPgn */
{ 0, sof(nfcPgn),           spraByte, sgcSep,  2,    0, fFalse },
	
#ifdef MAC
/*123: sprmSDyaPgn */
{ 0, sof(dyaPgn),           spraWord, sgcSep,  3,    0, fFalse },
	
/*124: sprmSDxaPgn */
{ 0, sof(dxaPgn),           spraWord, sgcSep,  3,    0, fFalse },
	
#endif /* MAC */
#ifdef WIN
/*123: sprmSDyaPgn */
{ 0, 0,                     spraWord, sgcSep,  3,    0, fFalse },
	
/*124: sprmSDxaPgn */
{ 0, 0,                     spraWord, sgcSep,  3,    0, fFalse },
	
#endif /* WIN */
/*125: sprmSFPgnRestart  */
{ 0, sof(fPgnRestart),      spraByte, sgcSep,  2,    0, fFalse },
	
/*126: sprmSFEndnote */
{ 0, sof(fEndnote),         spraByte, sgcSep,  2,    0, fFalse },
	
/*127: sprmSLnc */
{ 0, sof(lnc),              spraByte, sgcSep,  2,    0, fFalse },
	
/*128: sprmSGrpfIhdt */
{ 0, sof(grpfIhdt),         spraByte, sgcSep,  2,    0, fFalse },
	
/*129: sprmSNLnnMod */
{ 0, sof(nLnnMod),          spraWord, sgcSep,  3,    0, fFalse },
	
/*130: sprmSDxaLnn */
{ 0, sof(dxaLnn),           spraWord, sgcSep,  3,    0, fFalse },
	
/*131: sprmSDyaHdrTop */
{ 0, sof(dyaHdrTop),        spraWord, sgcSep,  3,    0, fFalse },
	
/*132: sprmSDyaHdrBottom */
{ 0, sof(dyaHdrBottom),     spraWord, sgcSep,  3,    0, fFalse },
	
#ifdef MAC
/*133: sprmSLBetween */
{ 0, 0,                     spraByte, sgcSep,  2,    0, fFalse },
	
/*134: sprmSVjc       */
{ 0, 0,                     spraByte, sgcSep,  2,    0, fFalse },
	
/*135: sprmSLnnMin */
{ 0, 0,                     spraWord, sgcSep,  3,    0, fFalse },
	
/*136: sprmSPgnStart */
{ 0, 0,                     spraByte, sgcSep,  3,    0, fFalse },
	
#endif /* MAC */
#ifdef WIN
/*133: sprmSLBetween */
{ 0, sof(fLBetween),        spraByte, sgcSep,  2,    0, fFalse },
	
/*134: sprmSVjc       */
{ 0, sof(vjc),              spraByte, sgcSep,  2,    0, fFalse },
	
/*135: sprmSLnnMin */
{ 0, sof(lnnMin),           spraWord, sgcSep,  3,    0, fFalse },
	
/*136: sprmSPgnStart */
{ 0, sof(pgnStart),         spraWord, sgcSep,  3,    0, fFalse },
	
#endif /* WIN */
/*137: sprmSSpare2 */
{ 0, 0,                     spraByte, sgcSep,  0,    0, fFalse },
	
/*138: sprmSSpare3 */
{ 0, 0,                     spraWord, sgcSep,  0,    0, fFalse },
	
/*139: sprmSSpare4 */
{ 0, 0,                     spraWord, sgcSep,  0,    0, fFalse },
	
/*140: sprmSSpare5 */
{ 0, 0,                     spraWord, sgcSep,  0,    0, fFalse },
	
/*141: sprmSSpare6 */
{ 0, 0,                     spraWord, sgcSep,  0,    0, fFalse },
	
/*142: sprmSSpare7 */
{ 0, 0,                     spraWord, sgcSep,  0,    0, fFalse },
	
/*143: sprmSSpare8 */
{ 0, 0,                     spraWord, sgcSep,  0,    0, fFalse },
	
/*144: sprmSSpare9 */
{ 0, 0,                     spraWord, sgcSep,  0,    0, fFalse },
	
/*145: sprmSSpare10*/
{ 0, 0,                     spraWord, sgcSep,  0,    0, fFalse },
	


/*146: sprmTJc     */
{ 0, tof(jc),               spraWord, sgcTap,  3,    0, fFalse },
	
/*147: sprmTDxaLeft */
{ 0, 0,                     spraSpec, sgcTap,  3,    0, fFalse },
	
/*148: sprmTDxaGapHalf */
{ 0, tof(dxaGapHalf),       spraSpec, sgcTap,  3,    0, fFalse },
	
/*149: sprmTSpare6 */
{ 0, 0,                     spraSpec, sgcTap,  0,    0, fFalse },
	
/*150: sprmTSpare7 */
{ 0, 0,                     spraSpec, sgcTap,  0,    0, fFalse },
	
/*151: sprmTSpare8 */
{ 0, 0,                     spraSpec, sgcTap,  0,    0, fFalse },
	
/*152: sprmTDefTable */
{ 0, 0,    spraSpec, sgcTap,  0,    0, fFalse },
	
/*153: sprmTDyaRowHeight */
{ 0, tof(dyaRowHeight),     spraWord, sgcTap,  3,    0, fFalse },
	
/*154: sprmTSpare2 */
{ 0, 0,                     spraSpec, sgcTap,  0,    0, fFalse },
	
/*155: sprmTSpare3 */
{ 0, 0,                     spraSpec, sgcTap,  0,    0, fFalse },
	
/*156: sprmTSpare4 */
{ 0, 0,                     spraSpec, sgcTap,  0,    0, fFalse },
	
/*157: sprmTSpare5 */
{ 0, 0,                     spraSpec, sgcTap,  0,    0, fFalse },
	
/*158: sprmTInsert */
{ 0, 0,                     spraSpec, sgcTap,  5,    0, fFalse },
	
/*159: sprmTDelete */
{ 0, 0,                     spraSpec, sgcTap,  3,    0, fFalse },
	
/*160: sprmTDxaCol */
{ 0, 0,                     spraSpec, sgcTap,  5,    0, fFalse },
	
/*161: sprmTMerge */
{ 0, 0,                     spraSpec, sgcTap,  3,    0, fFalse },
	
/*162: sprmTSplit */
{ 0, 0,                     spraSpec, sgcTap,  3,    0, fFalse },
	
/*163: sprmTSetBrc */
{ 0, 0,                     spraSpec, sgcTap,  6,    0, fFalse },
	/* <---sprmMax - 1  */
};


CP CpLimBlock();

/* %%Function:CacheParaPRC %%Owner:davidlu */
CacheParaPRC(doc,cp)
/* hack so we can pass the address of a hand native coded proc */
int doc;
CP cp;
{
	CachePara(doc,cp);
}


/* A P P L Y  G R P P R L  S E L  C U R */
/* applies the group of prl's of length cb to the current selection.
Takes care of undo, and indirectly of invalidation.
*/
/* %%Function:ApplyGrpprlSelCur %%Owner:davidlu */
ApplyGrpprlSelCur(grpprl, cb, fSetUndo)
char *grpprl;
int cb;
int fSetUndo;
{
	struct CA ca;
	struct CA caInval;
	struct DOD **hdod = mpdochdod[selCur.doc];
	CP cp;
	int sgc;
	int ww;

	extern int wwCur;

	/* this may not be true if vssc != sscNil, in which case
		selDotted is really the currently active selection.
		If we get into that situation, we may have to use
		PselActive() to determine the active selection and use that
		instead of selCur. bz 3/7/88
	*/

	Assert (selCur.ww == wwCur);

	if (cb == 0)
		{
		if (fSetUndo)
			SetUndoNil();
		return;
		}

/* ca will be changed to expand according to the sgc of the first sprm
in the grpprl (that is indicative of the type of the change. The grpprl
will be applied to the range of cp's described by the ca */
	if ((sgc = dnsprm[*grpprl/*psprm*/].sgc) != sgcChp &&
			(sgc != sgcPap || !selCur.fColumn))
		{
		if (selCur.fBlock && !selCur.fTable)
			/* turn the block selection into a normal selection */
			Select(&selCur, selCur.cpFirst, CpLimBlock());
		ca = selCur.ca;
		}
	else
		{
		if (selCur.fIns && grpprl[0] != sprmCMajority)
			{
			/* must invalidate cursor */
			ClearInsertLine(&selCur);
			selCur.xpFirst = selCur.xpLim = xpNil;
			if (fSetUndo)
				SetUndoNil();
			(*hdod)->fFormatted = fTrue;
			GetSelCurChp(fFalse);
/* need to CachePara so vchpStc is set up properly to handle sprmCPlain */
			CachePara(selCur.doc, selCur.cpFirst);
#ifdef MAC
			ApplyPrlSgc((char HUGE *)grpprl, cb, &selCur.chp, sgcChp, fFalse);
#else
			ApplyPrlSgc((char HUGE *)grpprl, cb, &selCur.chp, sgcChp);
#endif /* MAC */
			selCur.fUpdateRibbon = fTrue;   /* make Ribbon aware of Trans prop */
			return;
			}
		else  if (selCur.fBlock)
			{
			CP dcp;
			struct BKS bks;
			StartLongOp();  /* this can take a while */
			ca = selCur.ca;
#ifdef WIN
			if (!FDelCkBlockOk(rpkNil, NULL))
				/* selection not legal for delete, same goes for format */
				goto LReturn;
#endif
			if (fSetUndo)
				{
				cp = CpLimBlock();
				ca.cpLim = cp;
				if (!FSetUndoB1(ucmFormatting, uccFormat, &ca))
					goto LReturn;
				}
			InitBlockStat(&bks);
			caInval.doc = ca.doc;
			while (FGetBlockLine(&ca.cpFirst, &dcp, &bks))
				if (dcp)
					{
					ca.cpLim = ca.cpFirst + dcp;
					ExpandCaSprm(&ca, &caInval, grpprl);
					InvalCp(&caInval);
#ifdef WIN
					InvalText (&ca, fFalse /* fEdit */);
#endif
					ApplyNinchChpGrpprlCa(grpprl, cb, &ca);
					if (vmerr.fMemFail) goto LReturn;
					}
			if (!selCur.fColumn)
				TurnOffBlock(&selCur);
			if (fSetUndo)
				SetUndoAfter(0);
LReturn:
			EndLongOp (fFalse /* fAll */);
			return;
			}
		ca = selCur.ca;
/* do not apply prop to trailing blanks, except: if property is vanished
or if selection was made character by character carefully */
		if (selCur.sty > styChar && grpprl[0] != sprmCFVanish &&
				grpprl[0] != sprmCMajority)
			ca.cpLim = CpLimNoSpaces(&ca);
		}
	ExpandCaSprm(&ca, &caInval, grpprl);

	if (fSetUndo)
		{
		/* KLUDGE ALERT */
		/* if we are in the process of applying sprmPRuler, 
		*  we DO NOT want FlushRulerSprms to flush!  So, we
		*  temporarily set vrulss.caRulerSprm.doc to docNil.
		*  We will restore it after SetUndo.
		*/
		int docRulerSave = vrulss.caRulerSprm.doc;
		int fUndoCancel;
		int fRestoreRulerDoc = fFalse;
		if (*grpprl == sprmPRuler && docRulerSave == selCur.doc)
			{
			vrulss.caRulerSprm.doc = docNil; /* so no flush */
			fRestoreRulerDoc = fTrue;
			}
		fUndoCancel = !FSetUndoB1(ucmFormatting, uccFormat, &ca);
		if (fRestoreRulerDoc)
			vrulss.caRulerSprm.doc = docRulerSave;
		if (fUndoCancel) return;
		}

/* flush any pending sprms in ruler sprm cache */
	if (vrulss.caRulerSprm.doc != docNil &&
			grpprl[0] != sprmPRuler)
		FlushRulerSprms();

/* e.g. inval whole first paragraph, even though prm was applied starting only
at cpLim - 1 to save prc space. */
	InvalCp(&caInval);

/* check for style transformations: they must inval outline */
	(*hdod)->fOutlineDirty = fTrue;
	if ((grpprl[0] == sprmPStc || grpprl[0] == sprmPIncLvl) && !(*hdod)->fShort && (*hdod)->hplcpad != hNil)
		for (ww = WwDisp(ca.doc, wwNil, fFalse); ww != wwNil; ww = WwDisp(ca.doc, ww, fFalse))
			if (PwwdWw(ww)->fOutline)
				{
				/* this flag must be set even if !fSetUndo */
				vuab.fOutlineInval = fTrue;
				caInval = ca;
				ExpandOutlineCa(&caInval, fTrue);
				InvalCp(&caInval);
				break;
				}

#ifdef WIN
	/*  dirty any enclosing field result */
	InvalText (&ca, fFalse /* fEdit */);

/* invalidate all subsequent auto-numbered paragraphs (seqlevs) */
	if ((*hdod)->hplcfld != hNil && !(*hdod)->fHasNoSeqLev)
		(*hdod)->fInvalSeqLev = vidf.fInvalSeqLev = fTrue;
#endif

	ApplyNinchChpGrpprlCa(grpprl, cb, &ca);
	if (fSetUndo)
		SetUndoAfter(&ca);
	vfSeeSel = fTrue;
}


/* %%Function:ApplyNinchChpGrpprlCa %%Owner:davidlu */
ApplyNinchChpGrpprlCa(grpprl, cb, pca)
char *grpprl;
int cb;
struct CA *pca;
{
	int doc;
	int i;
	CP cpFirst;
	char *pprl, *pprlTrans, *grpprlApply;
	struct ESPRM *pesprm;
	char sprmTrans;
	int cbCur;
	struct CHP chpStcFirst;
	struct CHP chpNinch;
	char grpprlTrans[cbMaxGrpprl];

	cbCur = cb;
	grpprlApply = grpprl;
	if (dnsprm[grpprl[0]].sgc == sgcChp)
		{
		/* for each of the toggled CHP properties (bold, italic, shadow, etc.)
			we need to know if the properties of any of the styles used within
			the selection are different. */
		i = 0;

		/* initialize bits that represent the toggled properties to fFalse */
		*(int *)(&chpNinch) = 0;
		CachePara(doc = pca->doc, cpFirst = pca->cpFirst);
		FetchCp(doc, cpFirst, fcmProps);
		chpStcFirst = vchpStc;
		while (caPara.cpLim < pca->cpLim)
			{
			CachePara(doc, caPara.cpLim);
			FetchCp(doc, caPara.cpFirst, fcmProps);
			/* set bit for toggle prop in chpNinch to fTrue when the bit value
				for the prop in vchpStc is different than the bit value for
				the first paragraph of the selection. */
			*(int *)(&chpNinch) |= ((*(int *)&vchpStc) ^ (*(int *)&chpStcFirst));
			/* if the selection encompasses too many paragraphs, we give up
				the scan and apply the entire grpprl without transformation.
				This means that the properties being set by the grpprl will
				not toggle when the underlying style definition toggles. */
			if (i++ >= 20)
				goto LApply;
			}
		/* retain only the toggled bit changes. */
		*(int *)&chpNinch &= (~maskFs);

		/* now we will scan through the input grpprl, copying each prl to
			grpprlTrans. For each bit sprm in the grpprl, if it's corresponding
			value in chpNinch is fFalse (meaning that the property is the same
			for all styles within the selection), we will transform the sprms value
			so that when it is interpreted the property value will be derived from
			vchpStc. Otherwise we leave the sprm value untransformed which means
			the property set by the sprm will not toggle when the underlying
			style property toggles. */
		cbCur = 0;
		pprl = grpprl;
		pprlTrans = grpprlApply = grpprlTrans;
		while (cbCur < cb)
			{
			int cbT;
			cbT = CchPrl(pprl);
			if ((cbCur += cbT) <= cbMaxGrpprl)
				{
				bltb(pprl, pprlTrans, cbT);
				if ((pesprm = &dnsprm[sprmTrans = pprlTrans[0]])->spra ==
						spraBit && pesprm->sgc == sgcChp &&
						ValFromPropSprm(&chpNinch, sprmTrans) == fFalse)
					{
					/* if a character bit sprm whose property is invariant
						across all styles within the selection, then
						transform the sprm value. */
					pprlTrans[1] =
							(pprlTrans[1] == ValFromPropSprm(&chpStcFirst, sprmTrans)) ?
							0x80 : 0x81;
					}
				pprl += cbT;
				pprlTrans += cbT;
				}
			}
		}
LApply:
	ApplyGrpprlCa(grpprlApply, cbCur, pca);
}


/* E X P A N D  C A  S P R M */
/* expands the ca to cover the area to be modified according to the
sgc of the sprm passed, also return cp's of area to be inval'd */
/* %%Function:ExpandCaSprm %%Owner:davidlu */
ExpandCaSprm(pca, pcaInval, psprm)
struct CA *pca, *pcaInval; 
char *psprm;
{
	int doc;
	int sgc;
	struct PLC **hplcsed;
	int CacheParaPRC(), CacheSectSedMac();

	pcaInval->doc = pca->doc;
	switch (sgc = dnsprm[*psprm].sgc)
		{
	default:
	/* case sgcChp:	*/
	/* case sgcPic: */
/* sprmCMajority is sent to the area that would be invalidated when a
	sprmPStc is applied to the ca. */
		if (*psprm == sprmCMajority)
			{
			ExpandCaCache(pca, pcaInval, &caPara, sgcChp, CacheParaPRC);
			*pca = *pcaInval;
			}
		else
			*pcaInval = *pca;
		return;
	case sgcPap:
		ExpandCaCache(pca, pcaInval, &caPara, sgc, CacheParaPRC);
		return;
	case sgcTap:
		ExpandCaTap(pca, pcaInval);
		return;
	case sgcSep:
#ifdef JR
		vfJRProps = fFalse;
		caSect.doc = docNil;
		pca->cpFirst = cp0;
#endif /* JR */
		ExpandCaCache(pca, pcaInval, &caSect, sgc, CacheSectSedMac);
#ifdef JR
		pcaInval->cpLim = CpMac1Doc(pca->doc);
		vfJRProps = fTrue;
		caSect.doc = docNil;
#endif /* JR */
		return;
		}
}


/* E X P A N D  C A  C A C H E */
/* cpFirst = last char of cached thing containing cpFirst
cpLim = (last char of cached thing containing cpLim - 1) + 1
In caInval: same, except cpFirst = first char of cached thing.
*/
/* %%Function:ExpandCaCache %%Owner:davidlu */
ExpandCaCache(pca, pcaInval, pcaCache, sgc,  Cache)
struct CA *pca, *pcaInval, *pcaCache; 
int sgc; 
int (*Cache)();
{
	pcaInval->doc = pca->doc;  /* so caller can pass pcaInval */
	(*Cache)(pca->doc, pca->cpFirst);
	pcaInval->cpFirst = pcaCache->cpFirst;
#ifdef MAC
/* avoid passing back a cpFirst that is part of a special Eop */
	pca->cpFirst = CpMin(pcaCache->cpLim - 1,
			((pcaCache == &caSect) ? CpMacDoc(pca->doc) :
			CpMacDocEdit(pca->doc)));
#else
/* avoid breaking up CR-LF pairs */
	pca->cpFirst = CpMax(pcaCache->cpFirst,
			CpMin(pcaCache->cpLim-cchEop,
			((pcaCache == &caSect) ? CpMacDoc(pca->doc) :
			CpMacDocEdit(pca->doc))));
#endif
	(*Cache)(pca->doc, CpMax(pca->cpLim - 1, pca->cpFirst));
	pcaInval->cpLim = pca->cpLim = pcaCache->cpLim;
	if (sgc == sgcPap)
		{
/* paragraphs invalidate line before and line after the main body so that
the display of "box" properties depend on the neighboring paras.
The same applies to the indents and bullets of the paras in outline mode. */
		int ww = WwDisp(pca->doc, wwNil, fTrue);
		struct WWD *pwwd;
		if (ww != wwNil)
			{
			if (pcaInval->cpFirst != cp0) pcaInval->cpFirst--;
			if (pcaInval->cpLim < CpMacDoc(pca->doc)) pcaInval->cpLim++;
			}
		}
	/* pca must be valid for copy/delete WRT fields */
	Win( AssureLegalSel (pca));
}


/* E X P A N D  C A  T A P */
/* Expands to fill a full tap */
/* %%Function:ExpandCaTap %%Owner:davidlu */
ExpandCaTap(pca, pcaInval)
struct CA *pca, *pcaInval;
{
	CP cpCur;

	cpCur = pca->cpFirst;
	pcaInval->doc = pca->doc;

	for (CachePara(pca->doc, cpCur) ; !vpapFetch.fTtp ; )
		{
		cpCur = caPara.cpLim;
		CachePara(pca->doc, cpCur);
		}

	pca->cpFirst = pcaInval->cpFirst = caPara.cpFirst;

	if (pca->cpLim > caPara.cpLim)
		{
		/* Find the end */
		cpCur = pca->cpLim - ccpEop;
		CachePara(pca->doc, cpCur);
		while (!vpapFetch.fTtp)
			{
			cpCur = caPara.cpLim;
			CachePara(pca->doc, cpCur);
			}
		}
	pca->cpLim = pcaInval->cpLim = caPara.cpLim;

}


/* A P P L Y  G R P P R L  C A */
/* failure indicated by vmerr.fFmtFailed */
/* %%Function:ApplyGrpprlCa %%Owner:davidlu */
ApplyGrpprlCa(grpprl, cb, pca)
char *grpprl; 
int cb; 
struct CA *pca;
{
	struct PLC **hplcpcd;
	int ipcdFirst, ipcdLim, ipcd;
	struct DOD **hdod = mpdochdod[pca->doc];
	struct PCD *ppcd;
	char rgb[2];

	Assert(cb > 0);

/* make document dirty. if it's a subdoc, FDirtyDoc will notice and declare
	that the mother doc is dirty. */
	(*hdod)->fFormatted = fTrue;

/* ensure that, for section sprms, the section table exists */
	Assert(dnsprm[*grpprl/*psprm*/].sgc != sgcSep ||
			(!(*hdod)->fShort && (*hdod)->hplcsed != hNil));

/* First get address of piece table and split off desired pieces. */

	ipcdFirst = IpcdSplit(hplcpcd = (*hdod)->hplcpcd, pca->cpFirst);
	ipcdLim = IpcdSplit(hplcpcd, pca->cpLim);
	if (vmerr.fMemFail)
		{
		vmerr.fFmtFailed = fTrue;
		return;
		}

/* Now just add this sprm to the pieces. */
	for (ipcd = ipcdFirst; ipcd < ipcdLim && !vmerr.fMemFail; ++ipcd)
		{
		int prm;
		struct PCD pcd;
		GetPlc(hplcpcd, ipcd, &pcd);
		pcd.prm = PrmAppend(pcd.prm, grpprl, cb);
		/* PrmAppend may call GetPlc, thus PutPlcLast is not appropriate */
		PutPlc(hplcpcd, ipcd, &pcd);
		}

/* check for style transformations: they change meaning of selCur.chp */
	if ((grpprl[0] == sprmPStc || grpprl[0] == sprmPIncLvl) &&
			pca->doc == selCur.doc)
		if (selCur.fIns)
			{
			rgb[0] = sprmCPlain;
			ApplyGrpprlSelCur(rgb, 1, fFalse);
			}
		else
			InvalSelCurProps(fTrue);
#ifdef WIN
	InvalVisiCache();
#endif /* WIN */
}


/* P R M  A P P E N D */
/* failure indicated by vmerr.fFmtFailed */
/* %%Function:PrmAppend %%Owner:davidlu */
int PrmAppend(prmPrev, pgrpprlNew, cbNew)
struct PRM prmPrev;
char *pgrpprlNew;
int cbNew;
{ /* Append <sprm, val> to the chain of sprm's in prm.  Return new prm. */
	struct PRC *pprcOld;
	char *pgrpprlPrev;
	int cbLeft;
	char grpprl[2];	/* For unpacking prm */
	struct MPRC prc;

	Debug(vdbs.fCkDoc ? CkGrpprl(cbNew, pgrpprlNew, fFalse) : 0);

/* get the address and cch of the grpprl and store them as
	pgrpprlPrev and cbLeft */

	pgrpprlPrev = PgrpprlFromPrm(prmPrev, &cbLeft, grpprl);
	if (cbLeft)
		bltb(pgrpprlPrev, &prc.grpprl, cbLeft);

/* merge grpprlNew into prc.grpprl */
	MergeGrpprls(&prc.grpprl, &cbLeft, pgrpprlNew, cbNew);

/* if the grpprl grew too large during the merge, give error message
	and pass back the original prm unchanged */
	if (cbLeft > cbMaxGrpprl)
		{
		ErrorEid(eidFormatTooComplex,"PrmAppend");
		vmerr.fFmtFailed = fTrue;
		return (prmPrev);
		}

/* result was an empty grpprl */
	if (cbLeft == 0)
		return (prmNil);

/* if we have a single prl whose size is <= 2 and
	the sprm can be represented in seven bits or fewer and
	we can replace the current prm value, then pack prl into the prm */
	else  if (cbLeft <= 2 &&
			dnsprm[*prc.grpprl].cch == cbLeft && *prc.grpprl < 128)
		{
		prmPrev = 0;
		/*prmPrev.fComplex = false; IE*/
		prmPrev.sprm = prc.grpprl[0];
		if (cbLeft == 2)
			prmPrev.val = prc.grpprl[1];
		return (prmPrev);
		}

/* store large grpprl in a PRC and return new PRM value */
	prc.bprlMac = cbLeft;
	return(PrmFromPrc(&prc, prmPrev));
}


/* 
assume order of new sprms (all with 1 byte operands):
	Hps
	HpsInc
	HpsPos 
	HpsPosAdjust
need executors in fetch.c for some of these. also emission of sprms by commands/dialogs
will have to be updated.
*/


/* M E R G E  G R P P R L S */

/* %%Function:MergeGrpprls %%Owner:davidlu */
MergeGrpprls(pgrpprlLeft, pcbLeft, pgrpprlRight, cbRight)
char *pgrpprlLeft, *pgrpprlRight; 
int *pcbLeft; 
int cbRight;
{
	int bprlRight, bprlLeft, bprlT;
	int sgcLast, sgcT;
	BOOL fSet;
	int bprlMin, bprlMac, bprlIns, bprlGap;
	int cbLeft = *pcbLeft;
	int cbNew, cbDel;
	int valLeft, valRight;
	int sprmLeft, sprmRight;
	int sprmFirstComb, sprmLimComb;
	int sprmFirstNC, sprmLimNC;
	int sprmAtIns;
	int cch, val;
	int dbprl;
	struct PCVH pcvhLeft, pcvhNew;
	char *pprlLeft, *pprlRight, *pprlNew;
	char prlT[cbMaxGrpprl];

	Debug(vdbs.fCkDoc ? CkGrpprl(cbLeft, pgrpprlLeft, fFalse) : 0);
	Debug(vdbs.fCkDoc ? CkGrpprl(cbRight, pgrpprlRight, fFalse) : 0);

	sgcLast = sgcNil;
	for (bprlRight = 0; bprlRight < cbRight; 
			bprlRight += CchPrl(&pgrpprlRight[bprlRight]))
		{
		sprmRight = pgrpprlRight[bprlRight];
/* add next sprmRight to grpprlLeft */
/* first, make sure bprlMin, bprlMac are set up. They delimit the portion
of grpprlLeft which modifiest sgcLast */
		if ((sgcT = dnsprm[sprmRight].sgc) != sgcLast)
			{
			sgcLast = sgcT;
/* change in sgc: determine new bprlMin, bprlMac */
			fSet = fFalse;
			bprlMac = cbLeft;
			for (bprlT = 0; bprlT < cbLeft;
					bprlT += CchPrl(pprlLeft))
				{
				pprlLeft = &pgrpprlLeft[bprlT];
				sgcT = dnsprm[*pprlLeft].sgc;
				if (!fSet)
					{
					if (sgcT == sgcLast)
						{
						bprlMin = bprlT;
						fSet = fTrue;
						}
					else  if (sgcLast < sgcT)
						{
						fSet = fTrue;
						bprlMin = bprlMac = bprlT;
						break;
						}
					}
				else  if (sgcT != sgcLast)
					{
					bprlMac = bprlT;
					break;
					}
				}
			if (!fSet) bprlMin = cbLeft;
			}
/* rest of the loop body will append prlRight to [Min, Mac) subset of grpprlLeft */
/* determine type sprm intervals Comb (compose) and NC (Not commute) with 
sprmRight */
		/*default:*/
		sprmFirstComb = sprmRight;
		sprmLimComb = sprmRight + 1;
		sprmFirstNC = sprmCMajority;
		sprmLimNC = sprmCMajority + 1;

		switch (sprmRight)
			{
		case sprmPIncLvl:
			sprmFirstComb = sprmPStc;
			break;
		case sprmPDxaLeft:
			sprmLimComb = sprmPNest + 1;
			break;
		case sprmPNest:
			sprmFirstComb = sprmPDxaLeft;
			break;
		case sprmCPlain:
/* sprmCPlain kills all other character sprms so non-commuativity checking is
	disabled for sprmCPlain */
			sprmFirstNC = sprmLimNC = sprmCPlain;
			break;
		case sprmCHps:
			sprmLimComb = sprmCHpsInc + 1;
			break;
		case sprmCHpsInc:
			sprmFirstComb = sprmCHps;
			/* FALL THROUGH */
		case sprmCHpsPos:
			sprmFirstNC = sprmCHpsPosAdj;
			sprmLimNC = sprmCMajority + 1;
			break;
		case sprmCHpsPosAdj:
			sprmFirstComb = sprmCHpsPos;
			sprmFirstNC = sprmCHps;
			sprmLimNC = sprmCMajority + 1;
			break;
		case sprmCMajority:
			sprmFirstNC = sprmCFStrikeRM;
			}
		if (sgcLast == sgcTap && sprmRight >= sprmFirstTNC)
			sprmFirstNC = sprmFirstTNC, 
					sprmLimNC = sprmLimTNC + 1;

/* main loop: find bprlIns s.t. bprlIns > bprl of any NC sprm and
	sprmIns is >= sprmFirstComb if sprmIns exists else bprlIns = bplrMac.
*/
		if (sgcLast == sgcPap)
			{
/* these are not non-commuting, but a trick to find sprmPRuler, so that it can
be deleted. It relies on the coincidence that all sgcPap sprms commute. */
			sprmFirstNC = sprmPRuler;
			sprmLimNC = sprmPRuler + 1;
			}
		fSet = fFalse;
			{{ /* NATIVE - MergeGrpprls */
			for (bprlLeft = bprlMin; bprlLeft < bprlMac; bprlLeft += cch)
				{
				sprmLeft = pgrpprlLeft[bprlLeft];
				if (!fSet && sprmLeft >= sprmFirstComb)
					{
					bprlIns = bprlLeft; 
					fSet = fTrue;
					}
				if (sprmLeft >= sprmFirstNC && sprmLeft < sprmLimNC)
					{
					if (sgcLast != sgcPap)
						fSet = fFalse;
					else
						{{ /* !NATIVE - MergeGrpprls */
/* delete sprmPRuler because some pap sprm is being appended */
						pprlLeft = &pgrpprlLeft[bprlLeft];
						bltb(pprlLeft + 2, pprlLeft,
								cbLeft - bprlLeft - 2);
						bprlMac -= 2;
						bprlLeft -= 2;
						cbLeft -= 2;
						cch = 2;
							{{
							continue;
							}} /* !NATIVE - MergeGrpprls */
						}}
					}
/* open coded CchPrl */
				if ((cch = (dnsprm[sprmLeft].cch)) == 0)
					{
					char *pgrpprlT = pgrpprlLeft + bprlLeft + 1;
					if (sprmLeft == sprmTDefTable)
						bltb(pgrpprlT, &cch, sizeof(int));
					else
						{
						cch = *pgrpprlT;
						if (cch == 255 && sprmLeft == sprmPChgTabs)
							{
							cch = (*(pgrpprlT += 1) * 4) + 1;
							cch += (*(pgrpprlT + cch) * 3) + 1;
							}
						}
					cch += 2;
					}
				}
			}}
		if (!fSet) bprlIns = bprlMac;

/* insert prlRight at bprlIns, or compose it with prlLeft */
		pprlLeft = pgrpprlLeft + bprlIns;
		sprmAtIns = bprlIns == bprlMac ? sprmMax :
				*(pprlLeft);
		pprlNew = pgrpprlRight + bprlRight;
		cbNew = CchPrl(pprlNew);
		switch (sprmRight)
			{
			char *pprlT;
		default:
/* self replacing sprms */
LDefault:
			cbDel = sprmAtIns != sprmRight ? 0 :
					CchPrl(pprlLeft);
			break;
/* self composing sprms, result is collected in pprlNew = &prlT */
		case sprmPStcPermute:
		case sprmPChgTabs:
#ifdef MAC
		case sprmPicScale:
#endif /* MAC */
			if (sprmAtIns == sprmRight)
				{
				pprlRight = pprlNew;
				pprlNew = &prlT;
				switch (sprmRight)
					{
				case sprmPStcPermute:
					ComposeStcPermutations(pprlLeft, pprlRight,
							pprlNew, &cbNew);
					break;
				case sprmPChgTabs:
					ComposeTabs(pprlLeft, pprlRight,
							pprlNew, &cbNew);
					break;
#ifdef MAC
				case sprmPicScale:
					ComposeScalingFactors(pprlLeft, pprlRight,
							pprlNew, &cbNew);
#endif /* MAC */
					}
				}
			goto LDefault;
		case sprmPPc:
			if (sprmAtIns >= sprmLimComb)
				goto LDefault;
			pcvhLeft.op = *(pprlLeft + 1);
			pcvhNew.op =  *(pprlNew + 1);
			/* special gray value used so sprm components may be
			set independently */
			if (pcvhNew.pcVert != pcvhNinch)
				pcvhLeft.pcVert = pcvhNew.pcVert;
			if (pcvhNew.pcHorz != pcvhNinch)
				pcvhLeft.pcHorz = pcvhNew.pcHorz;
			*(pprlLeft + 1) = pcvhLeft.op;
			continue;
		case sprmCMajority:
/* a new sprmCMajority can never replace any earlier instances of 
	sprmCMajority */
			cbDel = 0;
			break;
/* multiple composing sprms */
		case sprmCHpsInc:
			if (sprmAtIns >= sprmLimComb)
				goto LDefault;
/* similar to PNest, below. Left op is Hps or HpsInc. */
			val = *(pprlNew + 1);
			if (sprmAtIns == sprmCHpsInc)
				*(pprlLeft + 1) += val;
			else
				*(pprlLeft + 1) = HpsAlter(*(pprlLeft + 1),
						SignExtendByte(val));
			continue;
		case sprmCHps:
/* FALL THROUGH: CHps replaces plain CHps, or other CHpsInc */
		case sprmCHpsPosAdj:
/* FALL THROUGH: HpsPosAdjust replaces plain HpsPos, or other HpsPosAdjust */
		case sprmPDxaLeft:
			if (sprmAtIns < sprmLimComb)
/* cause Left op (PNest or PDxaLeft) to be replaced by Right op */
				sprmAtIns = sprmRight;
			goto LDefault;
		case sprmPNest:
			if (sprmAtIns >= sprmLimComb)
				goto LDefault;
/* add word op from right to left (which is a PDxaLeft or PNest... */
			bltb(pprlLeft + 1, &valLeft, sizeof (int));
			bltb(pprlNew + 1, &valRight, sizeof (int));
			valLeft += valRight;
			bltb(&valLeft, pprlLeft + 1, sizeof (int));
			continue;
		case sprmPIncLvl:
			if (sprmAtIns >= sprmLimComb)
				goto LDefault;
/* PIncLvl may merge with the LAST in the list of PIncLvl's on the left.
We can also merge with PStc too. */
			for (fSet = fFalse, bprlT = bprlIns; bprlT < bprlMac;
					bprlT += CchPrl(pprlT))
				{
				pprlT = pgrpprlLeft + bprlT;
				if (*pprlT > sprmPIncLvl) break;
/* last prl in the interval, which is known to be non-empty */
				bprlLeft = bprlT;
				if (*pprlT == sprmPIncLvl)
					bprlIns = bprlT, fSet = fTrue;
				}
			pprlLeft = &pgrpprlLeft[bprlLeft];
			if (*pprlLeft == sprmPStc)
				{
/* means we have a PStc which is not followed by a pStcPermute! Such may be combined
with the PIncLvl */
				val = *(pprlLeft + 1);
				if (val >= stcLevMin && val <= stcLevLast)
					*(pprlLeft + 1) = max(min(val -
							SignExtendByte(*(pprlNew + 1)),
							stcLevLast), stcLevMin); /* see ApplySprm() */
				continue;
				}
			if (*pprlLeft == sprmPStcPermute)
				{
				bprlIns = bprlLeft;
				bprlIns += CchPrl(pprlLeft), cbDel = 0;
				}
			else  if (fSet)
				{
/* bprlIns points to the last sprmPIncLvl */
				pprlLeft = pgrpprlLeft + bprlIns;
				if ((*(pprlLeft + 1) ^ *(pprlNew + 1)) & 0200)
/* signs are different, do not compose */
					bprlIns += 2, cbDel = 0;
				else
					{
					*(pprlLeft + 1) += *(pprlNew + 1);
					continue;
					}
				}
			break;
/* dominant (formerly fClobber) sprms */
		case sprmPStc:
		case sprmCDefault:
/* delete sprms in right operand, except those not dominated */
			bprlGap = bprlIns;
			for (bprlT = bprlIns; bprlT < bprlMac;
					bprlT += cch)
				{
				char *pprlT = pgrpprlLeft + bprlT;
				cch = CchPrl(pprlT);
				if (dnsprm[*pprlT].fClobberExempt)
/* bit is set for PFInTable, PFTtp, and those unaffected by CDefault */
					{
/* we must insert a sprmCDefault after the last sprmCPlain we find in the 
	left grpprl, since sprmCPlain and sprmCDefault are non-commutative. */
					int fResetIns = (*pprlT == sprmCPlain && sprmRight == sprmCDefault);

					/* not dominated, delete Gap to T */
					if ((dbprl = bprlGap - bprlT))
						bltb(pprlT, pgrpprlLeft + bprlGap,
								cbLeft - bprlT);
					cbLeft += dbprl;
					bprlMac += dbprl;
					bprlT += dbprl;
					bprlGap = bprlGap + cch;
					if (fResetIns)
						bprlIns = bprlGap;
					}
				}
			if ((dbprl = bprlGap - bprlT))
				bltb(pgrpprlLeft + bprlT, pgrpprlLeft + bprlGap,
						cbLeft - bprlT);
			cbLeft += dbprl;
			bprlMac += dbprl;
			cbDel = 0;
			break;
		case sprmCPlain:
/* CPlain: ignore all following sprms */
			cbDel = bprlMac - bprlIns;
			}
/* common replace code. Replace starting at bprlIns, cbDel bytes with cbNew
bytes from pprlNew */
		dbprl = cbNew - cbDel;
		if (cbLeft + dbprl > cbMaxGrpprl)
			{
			*pcbLeft = cbMaxGrpprl + 1; 
			return;
			}
		bltb(pgrpprlLeft + bprlIns + cbDel,
				pgrpprlLeft + bprlIns + cbNew,
				cbLeft - bprlIns - cbDel);
		bltb(pprlNew, pgrpprlLeft + bprlIns, cbNew);
		cbLeft += dbprl;
		bprlMac += dbprl;
		}

	Debug(vdbs.fCkDoc ? CkGrpprl(cbLeft, pgrpprlLeft, fFalse) : 0);

	*pcbLeft = cbLeft;
}


/* S I G N  E X T E N D  B Y T E */
/* %%Function:SignExtendByte %%Owner:davidlu */
SignExtendByte(b)
{
	b &= 255;
	return b >= 128 ? b - 256 : b;
}


/*      A D D   P R L   S O R T E D

Inputs:
		psebl   local data block
		pprl    prl to copy
		cchPrl  length of prl (superfluous)

The caller guarantees that there is no conflict or composition with
existing prls. Upon entry and exit, psebl->pgrpprlMerge points to
the end of the grpprlMerge.

Inserts prl in grpprlMerge and increments pointer & byte count */

/* %%Function:AddPrlSorted %%Owner:davidlu */
EXPORT AddPrlSorted(psebl, pprl, cchPrl)
struct SEBL *psebl;
char *pprl;
int cchPrl;
{
	int cbRest = psebl->cbMerge;
	char *pgrpprl = psebl->pgrpprlMerge - cbRest;

	if ((psebl->cbMerge += cchPrl) <= psebl->cbMergeMax)
		{ /* We have not exceeded maximum grpprl size */
		int sprm = *pprl;
		while (cbRest != 0 && *pgrpprl < sprm)
			{ /* Loop until we pass all sprms < sprm */
			int cch = CchPrl(pgrpprl);
			pgrpprl += cch;
			cbRest -= cch;
			}
		Assert(cbRest >= 0);
		if (cbRest != 0)
			bltb(pgrpprl, pgrpprl + cchPrl, cbRest);
		bltb(pprl, pgrpprl, cchPrl);
		psebl->pgrpprlMerge += cchPrl;
		}
}


/* C O M P O S E   T A B S

Inputs:
	psebl		local data block
	pprlCur 	current tab prl

merges commands in prlLater into pprlCur.

	Prl Format:

	sprmPChgTabs	(char)
	cch - 2 	(char)
	idxaDelMax	(char)
	rgdxaDel	(int [])
	rgdxaClose	(int [])
	idxaAddMax	(char)
	rgdxaAdd	(int [])
	rgtbdAdd	(char [])

*/
/* %%Function:ComposeTabs %%Owner:davidlu */
ComposeTabs(pprlLeft, pprlRight, pprlNew, pcb)
char *pprlLeft, *pprlRight;
char *pprlNew;
int *pcb;
{
	int idxaAddMax, idxaDelMax;
	int cch, cchAdj;
	int rgdxaDel[itbdMax];
	int rgdxaAdd[itbdMax];
	char rgtbdAdd[itbdMax];
	int rgdxaClose[itbdMax];
	char *pprlRightSave;

	pprlLeft += 2; /* skip sprm, cch */
	idxaDelMax = *pprlLeft++;
	bltb(pprlLeft, rgdxaDel, idxaDelMax*2);
	bltb(pprlLeft + idxaDelMax*2, rgdxaClose, idxaDelMax*2);
	pprlLeft += idxaDelMax*4;

	idxaAddMax = *pprlLeft++;
	bltb(pprlLeft, rgdxaAdd, idxaAddMax*2);
	bltb(pprlLeft + idxaAddMax*2, rgtbdAdd, idxaAddMax);

	pprlRight += 2; /* bypass sprm, cch */
	pprlRightSave = pprlRight;

	/* Remove new deletes from old adds */
	DeleteTabs(rgdxaAdd, rgtbdAdd, NULL, &idxaAddMax,
			(char HUGE *)pprlRight + 1, (char HUGE *)pprlRight + 1 + (*pprlRight)*2, *pprlRight);

	pprlRight += *pprlRight * 4 + 1;  /* skip idxaDel, rgdxaDel, rgdxaClose */

	/* Add new adds to old adds */
	AddTabs(rgdxaAdd, rgtbdAdd, NULL, &idxaAddMax,
			(char HUGE *)pprlRight + 1, (char HUGE *)pprlRight + 1 + (*pprlRight)*2, NULL, *pprlRight);

	/* Remove new adds from old deletes */
	DeleteTabs(rgdxaDel, NULL, rgdxaClose, &idxaDelMax,
			(char HUGE *)pprlRight + 1, LNULL, *pprlRight);

	pprlRight = pprlRightSave; /* Bypass sprm, cch */

	/* Add new deletes to old deletes */
	AddTabs(rgdxaDel, NULL, rgdxaClose, &idxaDelMax,
			(char HUGE *)pprlRight + 1, LNULL, pprlRight + 1 + (*pprlRight)*2, *pprlRight);

	cch = 2 + idxaDelMax * 4 + idxaAddMax * 3;
	*pcb = cch + 2;
	*pprlNew++ = sprmPChgTabs;
	*pprlNew++ = min(255, cch);
	*pprlNew++ = idxaDelMax;
	pprlNew = bltbyte(rgdxaDel, pprlNew, idxaDelMax * 2);
	pprlNew = bltbyte(rgdxaClose, pprlNew, idxaDelMax * 2);
	*pprlNew++ = idxaAddMax;
	pprlNew = bltbyte(rgdxaAdd, pprlNew, idxaAddMax * 2);
	bltb(rgtbdAdd, pprlNew, idxaAddMax);
}







/* C O M P O S E  S T C ... */
/* %%Function:ComposeStcPermutations %%Owner:davidlu */
ComposeStcPermutations(pprlLeft, pprlRight, pprlNew, pcb)
char *pprlLeft, *pprlRight, *pprlNew;
int *pcb;
{
	int     stc, stcLimLeft, stcLimRight, dstc;
	char    *pstcLeft, *mpstcstcRight, *pstcNew;

	bltb(pprlLeft, pprlNew, 2);

	/* Set up old, new permutations.  Zero element is stcLim + 1 */
	pstcLeft = pprlLeft + 1;
	stcLimLeft = *pstcLeft++ + 1;
	mpstcstcRight = pprlRight + 1;
	stcLimRight = mpstcstcRight[0] + 1;
	pstcNew = pprlNew + 2;

	/* First replace each element of the existing permutation with
		its mapping in the new permutation. */
	for (stc = 1; stc < stcLimLeft; stc++, pstcLeft++, pstcNew++)
		{
		int stcT = *pstcLeft;
		*pstcNew = (stcT > 0 && stcT < stcLimRight) ?
				mpstcstcRight[stcT] : stcT;
		}
	/* Now copy the remainder of the new permutation, if any, verbatim */
	dstc = stcLimRight - stcLimLeft;
	if (dstc > 0)
		{
		*(pprlNew + 1) += dstc;
		bltb(&mpstcstcRight[stcLimLeft], pstcNew, dstc);
		}
	*pcb = *(pprlNew + 1) + 2;
}


#ifdef MAC
/* %%Function:ComposeScalingFactors %%Owner:NOTUSED */
ComposeScalingFactors(pprlLeft, pprlRight, pprlNew, pcb)
char *pprlLeft;
char *pprlRight;
char *pprlNew;
int *pcb;
{
	int  i;
	char  *pchLeft, *pchRight, *pchNew;
	struct FRC frcRight;
	struct FRC frcLeft;
	struct FRC frcResult;

	bltb(pprlLeft, pprlNew, 2);
	for (pchLeft = pprlLeft + 2, pchRight = pprlRight + 2,
			pchNew = pprlNew + 2, i = 0;
			i < 2; 
			pchRight += sizeof(struct FRC), pchLeft += sizeof(struct FRC),
			pchNew += sizeof(struct FRC), i++)
		{
		bltb(pchRight, &frcRight, sizeof(struct FRC));
		bltb(pchLeft, &frcLeft, sizeof(struct FRC));
		MultFrc(&frcLeft, &frcRight, &frcResult);
		bltb(&frcResult, pchNew, sizeof(struct FRC));
		}
	*pcb = 2 + 2 * (sizeof(struct FRC));
}


#endif	/* MAC */



/* P R M  F R O M  P R C */
/* failure indicated by vmerr.fFmtFailed */
/* %%Function:PrmFromPrc %%Owner:davidlu */
PrmFromPrc(pprcNew, prm)
struct PRC     *pprcNew;
int     prm;
{
	struct PRC  **hprc;
	struct PRC  **hprcNew;
	struct PRC  *pprc;
	struct PRM  *pprm;
	int    cwRequest;

	pprcNew->fRef = fFalse;
	pprcNew->wChecksum = WChecksumForPrc(pprcNew);
/* set so vhprc chain is checked when we run out of memory */
	vmerr.fReclaimHprcs = fTrue;

/* Check newly created prc to see if same as any of previous prcs stored in
		the heap */
	hprc = vhprc;

	while (hprc)
		{
		pprc = *hprc;
		if (pprcNew->wChecksum == pprc->wChecksum)
			{
			if (pprcNew->bprlMac == pprc->bprlMac &&
					!FNeRgch(&pprcNew->grpprl[0], &pprc->grpprl[0],
					pprcNew->bprlMac))
				{
				/* found match so make prm point at prc */
				LinkPrcToPrm((struct PRM *) &prm, hprc);
				return(prm);
				}
			}
		hprc = pprc->hprcNext;
		}

	/* allocate a new hprc */
	cwRequest = (pprcNew->bprlMac + cbPRCBase + 1) / 2;
	if ((hprcNew = HAllocateCw(cwRequest)) == hNil)
		{
		vmerr.fFmtFailed = fTrue;
		return(prm);
		}

	/* move new prc info to heap block */
	bltb(pprcNew, *hprcNew,pprcNew->bprlMac + cbPRCBase);
	(**hprcNew).hprcNext = vhprc;
	vhprc = hprcNew;

	/* make the prm point to the prc */
	LinkPrcToPrm((struct PRM *)&prm, hprcNew);
	return(prm);
}


/* W  C H E C K S U M  F O R  P R C */
/* %%Function:WChecksumForPrc %%Owner:davidlu */
int WChecksumForPrc(pprc)
struct PRC *pprc;
{
	int     wChecksum;
	int     cch;
	char    *pch;

	wChecksum = pprc->bprlMac;

	cch = pprc->bprlMac;
	pch = pprc->grpprl;

	while (cch)
		{
		wChecksum += *(pch++);
		cch--;
		}
	return(wChecksum);
}


/* L I N K  P R C  T O	P R M */
/* %%Function:LinkPrcToPrm %%Owner:davidlu */
LinkPrcToPrm(pprm, hprc)
struct	PRM *pprm;
struct	PRC **hprc;
{
#ifdef MAC
	pprm->cfgrPrc = ((uns *) pfgrMac - (uns *) hprc);
#else /* WIN */
	Assert(((hprc>>1)<<1) == hprc);
	pprm->cfgrPrc = hprc>>1;
#endif /* MAC */
	pprm->fComplex = fTrue;
}


#ifdef WIN
/* ****
* Description: creates a prl and cchPrl from a sprm and a value.
*    returns cchPrl. cchPrl = valNil if sprm==sprmNOOP
** **** */

/* %%Function:CchPrlFromSprmVal %%Owner:davidlu */
int CchPrlFromSprmVal(prl, sprm, val)
char *prl;
int sprm;
int val;
{
	int cchPrl;

	if ((sprm == sprmNoop))
		return (valNil);

	prl[0] = sprm;
	cchPrl = dnsprm[sprm].cch;
	Assert(cchPrl >= 1 && cchPrl <= 3);
	switch (cchPrl)
		{
		/* case 1: no argument to go with sprm */
	case 2:
		/* byte quantity */
		prl[1] = val;
		break;
	case 3:
		/* word quantity */
		bltbyte(&val, &prl[1], sizeof(int));
		break;
		}

	return ( dnsprm[prl[0]].cch ); /* sprm might have changed */
}


#endif /* WIN */





/* %%Function:ApplyGrpprlToStshPrope %%Owner:davidlu */
ApplyGrpprlToStshPrope(pgrpprl, cchGrpprl, fChp, fUpdBanter)
char *pgrpprl;
int cchGrpprl;
int fChp;
int fUpdBanter;
{
	char *pprope, *ppropeBackup;
	int stcBase;
	int cchBase;
	int stcp, ftc, fFontChange = fFalse;
	int cchResult;
	int cbProp;
	struct DOD *pdod;
	struct STSH stsh;
	struct STTB **hsttb, **hsttbChpe, **hsttbPape;
	struct PLESTCP *pplestcp;
	struct CHP chp;
	struct PAP pap;
	struct CHP chpBackup;
	struct PAP papBackup;
#ifdef MAC
	DIALOGREC far *qDialogSave;
#endif /* MAC */

	if (fUpdBanter)
		{
#ifdef MAC
		qDialogSave = vqDialog;

		if (!FSendMsgToStyleBanter())
			goto LErrRet;
#endif /* MAC */
		SetVdocStsh(selCur.doc);
		}
	RecordStshForDoc(vdocStsh, &stsh, &hsttbChpe, &hsttbPape);

	/* transform stc into style sheet index */
	stcp = (vstcStyle + stsh.cstcStd) & 255;

	/* if style is standard, both hsttbChpe and hsttbPape must be filled
		in, in parallel in order to satisfy MapStc. */
	if (stcStdMin < vstcStyle && vstcStyle <= 255)
		{
		if (FStcpEntryIsNull(hsttbPape, stcp))
			{
			pdod = PdodDoc(vdocStsh);
			MapStc(pdod, 0, &chp, &pap);
			MapStcStandard(vstcStyle, &chp, &pap);
			if (!FStorePropeForStcp(&chp, stcp, hsttbChpe, fTrue))
				{
LMemErr:
				SetErrorMat(matLow);
				goto LErrRet;
				}
			if (!FStorePropeForStcp(&pap, stcp, hsttbPape, fFalse))
				{
				char stNil[1];
				stNil[0] = 255;
				/* shouldn't fail*/
				AssertDo(FChangeStInSttb(hsttbChpe, stcp, stNil));
				goto LMemErr;
				}
			}
		}

	/* retrieve the stcBase for the style */
	stcBase = (*stsh.hplestcp)->dnstcp[stcp].stcBase;

	/* retrieve expanded properties corresponding to style. */
	RetrievePropeForStcp(&chp, stcp, hsttbChpe, fTrue);
	bltb(&chp, &chpBackup, cbCHP);
	RetrievePropeForStcp(&pap, stcp, hsttbPape, fFalse);
	bltb(&pap, &papBackup, cbPAP);

#ifdef WIN
	if (fChp)
		ftc = chp.ftc;
#endif

	/* apply the grpprl to the expanded property */
#ifdef MAC
	ApplyPrlSgc((char HUGE *)pgrpprl, cchGrpprl, fChp ? &chp : &pap, fChp ? sgcChp : sgcPap, fFalse);
#else
	ApplyPrlSgc((char HUGE *)pgrpprl, cchGrpprl, fChp ? &chp : &pap, fChp ? sgcChp : sgcPap);
#endif /* MAC */

	pap.phe.fStyleDirty = fTrue;

	/* store the changed expanded property back into its hsttb */
	if (!(fChp ? FStorePropeForStcp(&chp, stcp, hsttbChpe, fTrue) :
			FStorePropeForStcp(&pap, stcp, hsttbPape, fFalse)))
		goto LMemErr;

	/* if we we're changing CHP, we still need to set fStyleDirty in
		the PAP's phe. */
	if (fChp)
		{
		if (!papBackup.phe.fStyleDirty)
			{
			if (!FStorePropeForStcp(&pap, stcp, hsttbPape, fFalse /* Pap */))
				goto LRestoreDefn;
			}
		Win(fFontChange = ftc != chp.ftc);
		}
	/* record that STSH is dirty */
	PdodDoc(vdocStsh)->fStshDirty = fTrue;
	if (!FGenChpxPapxNewBase(vdocStsh, stcp))
		{
LRestoreDefn:
		StartGuaranteedHeap();
		RestoreStcpDefn(vdocStsh, stcp, &chpBackup, &papBackup);
		EndGuarantee();
		goto LMemErr;
		}
	if (!FRecalcAllDependentChpePape(&stsh, hsttbChpe, hsttbPape, stcp))
		{
		StartLongOp();
		RestoreStcpAndDependentsDefn(vdocStsh, stcp, &chpBackup, &papBackup);
		EndLongOp(fFalse);
		goto LMemErr;
		}

	InvalDoc(vdocStsh);
	vdocFetch = caPara.doc = docNil;
	if (fUpdBanter)
		{
#ifdef WIN
		if (fFontChange)
			SetIbstFontDSFromStc(vstcStyle);
		BanterToTmc(tmcDSBanter, vstcStyle, &stsh, hsttbChpe, hsttbPape);
#else
		BanterToTmc(tmcDSBanter, tmcBanterU, stcp, &stsh, hsttbChpe, hsttbPape);
#endif
		}
LErrRet:
	;
#ifdef MAC
	if (fUpdBanter)
		SelectDlg(qDialogSave);  /* restore dialog state */
#endif /* MAC */
}


#ifdef WIN
/* V A L  F R O M  S P R M
*  returns value of property addressed sprm as an int */
/* %%Function:ValFromSprm %%Owner:davidlu */
ValFromSprm(sprm)
int sprm;
{

/* Change from Mac code: use selCur and pay attention to grayness.
	Otherwise, if there is the same prop at the start and end of a gray
	selection, and you apply the prop at the start, it will not change.
	For sep, use the method that the section dialog does: if the selection
	covers only one section, use vsepFetch, otherwise return a gray value.
	bz 1/18/88
*/

	extern struct SEP vsepFetch;
	extern struct CA  caSect;

	char *prgbProp;
	char *prgbPropGray;

	switch (dnsprm[sprm].sgc)
		{
	case sgcChp:
		Assert (!selCur.fUpdateChp);
		prgbProp = &selCur.chp;
		prgbPropGray = &vchpGraySelCur;
		break;
	case sgcPap:
		Assert (!selCur.fUpdatePap);
		prgbProp = &vpapSelCur;
		prgbPropGray = &vpapGraySelCur;
		break;
	case sgcSep:
		/* see comments above */
		CacheSect(selCur.doc, selCur.cpFirst);
		if (selCur.cpLim > caSect.cpLim)
			return (tmvalNinch);  /* gray return value */
		else
			{
			prgbProp = &vsepFetch;
			goto NoGray;
			}
		}
	/* if gray rgprop field is non zero, prop is gray */
	if (ValFromPropSprm(prgbPropGray, sprm))
		return (tmvalNinch);  /* gray return value */
NoGray:
	return (ValFromPropSprm(prgbProp, sprm));
}


#endif /* WIN */



#ifdef WIN
/* %%Function:SetPropFromSprmVal %%Owner:davidlu */
SetPropFromSprmVal(prgbProps, sprm, val)
char * prgbProps;
int sprm, val;
{
	struct ESPRM esprm;
	struct PCVH pcvh;

	esprm = dnsprm[sprm];
	switch (esprm.spra)
		{
	case spraWord:
		bltb(&val, prgbProps + esprm.b, 2);
		break;

	case spraByte:
		*(prgbProps + esprm.b) = val;
		break;

	case spraBit:
		/* WARNING: shift is machine-dependent */
#ifdef WIN
		if (val)
			*(int *) prgbProps |= (1 << esprm.b);
		else
			*(int *) prgbProps &= ~(1 << esprm.b);
#else /* MAC */
		if (val)
			*(int *) prgbProps |= (1 << (15 - esprm.b));
		else
			*(int *) prgbProps &= ~(1 << (15 - esprm.b));
#endif /* WIN / MAC */
		break;

	case spraCFtc:
		((struct CHP *) prgbProps)->ftc = val;
		break;

	case spraCKul:
		((struct CHP *) prgbProps)->kul = val;
		break;

	case spraCPlain:
		Assert(fFalse); /* NOT SUPPORTED! */
		break;

	case spraCIco:
		((struct CHP *) prgbProps)->ico = val;
		break;

	default:
		switch (sprm)
			{
		case sprmCQpsSpace:
			((struct CHP *) prgbProps)->qpsSpace = val;
			break;

		case sprmPStc:
			((struct PAP *) prgbProps)->stc = val;
			break;

		case sprmTDxaGapHalf:
			((struct TAP *) prgbProps)->dxaGapHalf = val;
			break;

		case sprmPPc:
			pcvh.op = val;
			((struct PAP *) prgbProps)->pcVert = pcvh.pcVert;
			((struct PAP *) prgbProps)->pcHorz = pcvh.pcHorz;
			break;
#ifdef DEBUG
		default:
			Assert(fFalse);
#endif /* DEBUG */
			}
		}
}


#endif /* WIN */


/* %%Function:EmitSprmCMajority %%Owner:davidlu */
EmitSprmCMajority(pchp)
struct CHP *pchp;
{
	int cb;
	char prl[2 + cbCHPBase];
	struct CA ca;

	prl[0] = sprmCMajority;
	prl[1] = cbCHPBase;
	bltb(pchp, prl + 2, cbCHPBase);

	ApplyGrpprlSelCur(prl, 2 + cbCHPBase, fFalse);
}


#ifdef WIN /* not used in MacWord */
/* %%Function:EmitSprmCMajCa %%Owner:davidlu */
EmitSprmCMajCa(pca, pchp)
struct CHP *pchp;
{
	int cb;
	char prl[2 + cbCHPBase];
	struct CA ca;

	prl[0] = sprmCMajority;
	prl[1] = cbCHPBase;
	bltb(pchp, prl + 2, cbCHPBase);

	ApplyGrpprlCa(prl, 2 + cbCHPBase, pca);
}


#endif


/* MACREVIEW davidlu ... I added the cb parameter so I could pass a grpprl.
	Perhaps you could make all your mac only clients pass a 2 so we can
	avoid possible bugs if this routine is ever called from wordtech (dsb) */
#ifdef MAC
/* %%Function:CmdApplySprmPStc %%Owner:NOTUSED */
CmdApplySprmPStc(pprl)
char *pprl;
#else
/* %%Function:CmdApplySprmPStc %%Owner:davidlu */
CmdApplySprmPStc(pprl, cb)
char *pprl;
int cb;
#endif
{
	int stc; 
	struct PAP pap;
	struct CHP chp;
	struct CA caPap, caCMaj, ca, caInval;
	char rgb[2];

/* need to determine the unions of the CP ranges affected by the application
	of sprmCMajority and sprmPStc. */
	caPap = selCur.ca;
	caCMaj = selCur.ca;
	ExpandCaSprm(&caPap,	&caInval, pprl);
	rgb[0] = sprmCMajority;
	ExpandCaSprm(&caCMaj, &caInval, rgb);
	ca.doc = selCur.doc;
	ca.cpFirst = CpMin(caPap.cpFirst, caCMaj.cpFirst);
	ca.cpLim = CpMax(caPap.cpLim, caCMaj.cpLim);
	Win( AssureLegalSel(&ca) );

/* use the CP range union for the undo interval */
	if (!FSetUndoB1(ucmFormatting, uccFormat, &ca))
		return cmdCancelled;
	CacheParaSel(&selCur);
	pap = vpapFetch;
	FlushRulerSprms();

	ApplyGrpprlSelCur(pprl, WinMac(cb, 2), fFalse);
	stc = *(pprl + 1); 
	if (stc < stcLevMin || stc > stcLevLast ||
		stcLevLast - stc > PdodMother(selCur.doc)->lvl)
		PdodMother(selCur.doc)->lvl = lvlNone;

	GetMajoritySelsChp(&selCur, &chp);
	EmitSprmCMajority(&chp);
	SetUndoAfter(0);
	if (selCur.fIns)
		GetSelCurChp(fTrue);
	CacheParaSel(&selCur);
	if (FDestroyParaHeight(&pap, &vpapFetch))
		InvalPageView(selCur.doc);
#ifdef MAC
	FSetAgainGrpprl(pprl, 2, ucmFormatting);
#endif
	return cmdOK;
}


/* P G R P P R L  F R O M  P R M  */
/* given a prm return a pointer to the list of sprm (pgrpprl) and return
	the length of the grpprl */
/* %%Function:PgrpprlFromPrm %%Owner:davidlu */
PgrpprlFromPrm(prm, pcb, grpprl)
struct PRM prm;
int *pcb;
char *grpprl; /* user provided buffer for unloading sprm from piece table */
{
	char *pgrpprl;
	struct PRC *pprc;

	if (prm.fComplex)
		{
		pprc = *HprcFromPprmComplex(&prm);
		pgrpprl = &(pprc->grpprl[0]);
		*pcb =  pprc->bprlMac;
		}
	else
		{
		if (prm.prm == 0)
			{
			/* PRM is null */
			pgrpprl = 0;
			*pcb = 0;
			}
		else
			{
			/* PRM contains exactly 1 prl */
			grpprl[0] = prm.sprm;
			grpprl[1] = prm.val;
			pgrpprl = grpprl;
			*pcb = dnsprm[prm.sprm].cch;
			}
		}
	return pgrpprl;
}


/* %%Function:CacheParaSel %%Owner:davidlu */
CacheParaSel(psel)
struct SEL *psel;
{
	CachePara(psel->doc, (!psel->fTable) ? psel->cpFirst :
			CpFirstForItc(psel->doc, psel->cpFirst, psel->itcFirst));
}


/* list of sprms which cause page view to be recalculated. If you change
	this, change FDestroyParaHeight below also */
char rgsprmParaBad[] =
	{
	sprmPFKeep, sprmPFKeepFollow, sprmPFPageBreakBefore, sprmPDxaAbs,
	sprmPDyaAbs, sprmPDxaWidth, sprmPPc, sprmPFromText,
	Mac(sprmPFSideBySide)
};


/*************************************************/
/* F  D E S T R O Y  P A R A  H E I G H T        */
/* return true if sprms will destroy para height, and should invalidate pageview */
/* %%Function:FDestroyParaHeight %%Owner:davidlu */
FDestroyParaHeight(ppap, ppapBase)
struct PAP *ppap, *ppapBase;
{
/* this set of tests should match rgsprmParaBad above */
	return  (!FMatchAbs(docNew, ppap, ppapBase) || /* docNew works as well as any */
			ppap->fKeep != ppapBase->fKeep ||
			ppap->fKeepFollow != ppapBase->fKeepFollow ||
			Mac( ppap->fSideBySide != ppapBase->fSideBySide || )
			ppap->fPageBreakBefore != ppapBase->fPageBreakBefore);
}


/*************************************************/
/* I n v a l   S o f t l y   P g v w   P a r a   */
/* %%Function:InvalSoftlyPgvwPara %%Owner:davidlu */
InvalSoftlyPgvwPara(doc, pprl, cch)
int doc;
char *pprl;
int cch;
{
/* if any significant para sprms or sectoin sprms are present in the prl,
inval page view */

	struct ESPRM esprm;
	int cchSprm, csprm;
	char *psprm;

	while (cch > 0)
		{
		esprm = dnsprm[*pprl];
		if (esprm.sgc == sgcSep)
			{
			InvalPageView(DocMother(doc));
			return;
			}

		if (esprm.sgc != sgcPap)
			return;
		if ((cchSprm = esprm.cch) == 0)
			cchSprm = *(pprl + 1) + 2;  /* variable-length type */
		for (csprm = sizeof(rgsprmParaBad), psprm = rgsprmParaBad; csprm-- > 0 ; )
			if (*pprl == *psprm++)
				{
				InvalPageView(DocMother(doc));
				return;
				}
		cch -= cchSprm;
		pprl += cchSprm;
		}
}


