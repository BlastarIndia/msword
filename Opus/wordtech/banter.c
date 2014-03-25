/*
*   banter.c	Generates banter for styles and search/replace formatting
*      dialogs.
*/

#ifdef	MAC
/* See not in block below */
#define BANTER
#include "globdefs.h"
#undef BANTER
#endif	/* MAC */

#ifdef WIN
/*********************************************************************/
/* These #defines were evacuated into this file when globdefs.h was  */
/* deleted.   They shouldn't stay here permanently.                  */
/* MAC and WIN should agree how to do this and handle it consistently */
/*********************************************************************/

#define szBoldDef       SzSharedKey("Bold",Bold)
#define iszBold   0
#define szItalicDef     SzSharedKey("Italic",Italic)
#define iszItalic 1
#define szNoUnderlineDef SzSharedKey("No Underline",NoUnderline)
#define iszNoUnderline 2
#define szUnderlineDef  SzSharedKey("Underline",Underline)
#define iszUnderline 3
#define szWordULDef     SzSharedKey("Word u.l.",WordUL)
#define iszWordUL 4
#define szDoubleULDef   SzSharedKey("Double u.l.",DoubleUL)
#define iszDoubleUL 5
#define szDottedULDef   SzSharedKey("Dotted u.l.",DottedUL)
#define iszDottedUL 6
#define szStrikeDef     SzSharedKey("Strikethru",Strikethru)
#define iszStrikethru 7
#define szOutlineDef    SzSharedKey("Outline",Outline)
#define iszOutline 8
#define szShadowDef     SzSharedKey("Shadow",Shadow)
#define iszShadow  9
#define szSmCapsDef     SzSharedKey("Small Caps",SmallCaps)
#define iszSmCaps 10
#define szCapsDef       SzSharedKey("Caps",Caps)
#define iszCaps 11
#define szHiddenDef     SzSharedKey("Hidden",Hidden)
#define iszHidden 12
#define szPosDef        SzSharedKey("Position:",Position)
#define iszPos 13
#define szSpacingDef    SzSharedKey("Spacing:",SpacingColon)
#define iszSpacing 14
#define szFontDef       SzSharedKey("Font:",Font)
#define iszFont 15
#define szLeftInDef     SzSharedKey("Left ",LeftSpace)
#define iszLeftIn 16
#define szFirstInDef    SzSharedKey("First ",First)
#define iszFirstIn 17
#define szRightInDef    SzSharedKey("Right ",RightSpace)
#define iszRightIn 18
#define szLineSpaceDef  SzSharedKey("Line Spacing:",LineSpacing)
#define iszLineSpace 19
#define szSpacBeforeDef SzSharedKey("Before",Before)
#define iszSpacBefore 20
#define szSpacAfterDef  SzSharedKey(" After", After)
#define iszSpacAfter 21
#define szTableDef      SzSharedKey("Side-by-Side",SideBySide)
#define iszTable 22
#define szPageBreakBeforeDef SzSharedKey("Page Break Before",PageBreakBefore)
#define iszPageBreakBefore 23
#define szKeepNextDef   SzSharedKey("Keep With Next",KeepWithNext)
#define iszKeepNext 24
#define szKeepTogetherDef SzSharedKey("Keep Lines Together",KeepLinesTogether)
#define iszKeepTogether 25
#define szLineNumberingDef SzSharedKey("Line Numbering",LineNumbering)
#define iszLineNumbering 26
#define szBorder        SzSharedKey("Border:",Border)
#define iszBorder 27
#define szSingle        SzSharedKey(WinMac("Single", "Single "),Single)
#define iszSingle 28
#define szThick         SzSharedKey(WinMac("Thick", "Thick "),Thick)
#define iszThick 29 
#define szDouble        SzSharedKey(WinMac("Double", "Double "),Double)
#define iszDouble 30
#define szDotted        SzSharedKey("Dotted ",Dotted)
#define iszDotted 31
#define szHairline      SzSharedKey("Hairline ",Hairline)
#define iszHairline 32
#define szDoubleHairline SzSharedKey("Double Hairline ",DoubleHairline)
#define iszDoubleHairline 33
#define szShadowBrcDef  SzSharedKey("Shadowed ",Shadowed)
#define iszShadowBrc 34
#define szNoBorderDef   SzSharedKey("No Border",NoBorder)
#define iszNoBorder 35
#define szFlushLDef     SzSharedKey("Flush left",FlushLeft)
#define iszFlushL 36
#define szCenterDef     SzSharedKey("Centered",Centered)
#define iszCenter 37
#define szFlushRDef     SzSharedKey("Flush Right",FlushRight)
#define iszFlushR 38
#define szFullDef       SzSharedKey("Justified",Justified)
#define iszFull 39
#define szTabsChangedDef SzSharedKey("Tab stops:",TabStops)
#define iszTabsChanged 40
#define szIndentDef     SzSharedKey("Indent: ",Indent)
#define iszIndent 41
#define szSpaceDef      SzSharedKey("Space",Space)
#define iszSpace 42
#define szPt25Def "25"
#define iszPt25 43
#define szPt5Def "5"
#define iszPt5 44
#define szPt75Def "75"
#define iszPt75 45
#define szDottedLeaderDef SzSharedKey("...",Elipses)
#define iszDottedLeader 46
#define szHyphenLeaderDef SzSharedKey("---",ThreeDashes)
#define iszHyphenLeader 47
#define szLineLeaderDef SzSharedKey("___",UnderLine)
#define iszLineLeader 48
#define szHeavyLeaderDef SzSharedKey("Heavy",Heavy)
#define iszHeavyLeader 49
#define szEmptyAlignDef ""
#define iszEmptyAlign 50
#define szCenteredAlignDef SzSharedKey(" Centered",CenteredAlign)
#define iszCenteredAlign 51
#define szRightFlAlignDef SzSharedKey(" Right Flush",RightFlush)
#define iszRightFlAlign 52
#define szNumberAlignedDef SzSharedKey(" Number Aligned",NumberAligned)
#define iszNumberAligned 53
#define szCommaDef      SzSharedKey(",",Comma)
#define iszComma 54
#define szSpDef         " "
#define iszSp 55
#define szNotDef        SzSharedKey("Not ",Not)
#define iszNot 56
#define szSemicolonDef  SzSharedKey(";",SemiColon)
#define iszSemicolon 57
#define szHalfDef       "5"
#define iszHalf 58
#define szPointDef      SzSharedKey(" Point",Point)
#define iszPoint 59
#define szPluralDef     SzSharedKey("s",Plural)
#define iszPlural 60
#define szNotAtDef      SzSharedKey("Not at ",NotAt)
#define iszNotAt 61
#define szPlusDef       SzSharedKey(" + ",Plus)
#define iszPlus 62
#define szSuperDef      SzSharedKey("Superscript ",Superscript)
#define iszSuper 63
#define szSubDef        SzSharedKey("Subscript ",Subscript)
#define iszSub 64
#define szExpDef        SzSharedKey("Expanded ",Expanded)
#define iszExp 65
#define szConDef        SzSharedKey("Condensed ",Condensed)
#define iszCon 66
#define szRelativeTo    SzSharedKey("Relative To ",RelativeTo)
#define iszRelativeTo 67
#define szPosition      SzSharedKey("Position: ",PositionColon)
#define iszPosition 68
#define szWidth         SzSharedKey("Width: ",Width)
#define iszWidth 69
#define szAbsLeft       SzSharedKey("Left",Left)
#define iszAbsLeft 70
#define szAbsCenterH    SzSharedKey("Center",Center)
#define iszAbsCenterH 71
#define szAbsRight      SzSharedKey("Right",Right)
#define iszAbsRight 72
#define szAbsInside     SzSharedKey("Inside",Inside)
#define iszAbsInside 73
#define szAbsOutside    SzSharedKey("Outside",Outside)
#define iszAbsOutside 74
#define szAbsInLine     SzSharedKey("In Line",InLine)
#define iszAbsInLine 75
#define szAbsTop        SzSharedKey("Top",Top)
#define iszAbsTop 76
#define szAbsCenterV    SzSharedKey("Center",Center)
#define iszAbsCenterV 77
#define szAbsBottom     SzSharedKey("Bottom",Bottom)
#define iszAbsBottom 78
#define szColumn        SzSharedKey("Column",Column)
#define iszColumn 79
#define szMargin        SzSharedKey("Margin",Margin)
#define iszMargin 80
#define szPage          SzSharedKey("Page",Page)
#define iszPage 81
#define szHorizontal    SzSharedKey(" Horiz. ",Horiz)
#define iszHorizontal 82
#define szVertical      SzSharedKey(" Vert. ",Vert)
#define iszVertical 83
#define szH             SzSharedKey("h: ",h)
#define iszH 84
#define szV             SzSharedKey("v: ",v)
#define iszV 85
#define szW             SzSharedKey("w: ",w)
#define iszW 86
#define szNormalPosition SzSharedKey("Normal Position ",NormalPosition)
#define iszNormalPosition 87
#define szLeftParen     SzSharedKey("(",LeftParen)
#define iszLeftParen 88
#define szRightParen    SzSharedKey(")",RightParen)
#define iszRightParen 89
#define szSpacingBrc    SzSharedKey(" Spacing",Spacing)
#define iszSpacingBrc 90
#define szTop           SzSharedKey(WinMac("Above ", "Top "),TopSpace)
#define iszTop 91
#define szBottom        SzSharedKey(WinMac("Below ", "Bottom "),BottomSpace)
#define iszBottom 92
#define szBetween       SzSharedKey("Between ",Between)
#define iszBetween 93
#define szColor         SzSharedKey("Color: ",Color)
#define iszColor 94
#define szAuto          SzSharedKey("Auto",Auto)
#define iszAuto 95
#define szBlack         SzSharedKey("Black",Black)
#define iszBlack 96
#define szBlue          SzSharedKey("Blue",Blue)
#define iszBlue 97
#define szCyan          SzSharedKey("Cyan",Cyan)
#define iszCyan 98
#define szGreen         SzSharedKey("Green",Green)
#define iszGreen 99
#define szMagenta       SzSharedKey("Magenta",Magenta)
#define iszMagenta 100
#define szRed           SzSharedKey("Red",Red)
#define iszRed 101
#define szYellow        SzSharedKey("Yellow",Yellow)
#define iszYellow 102
#define szWhite         SzSharedKey("White",White)
#define iszWhite 103
#define szRMarkDef      SzSharedKey("New",New)
#define iszRMark 104
#define szOutsideBar SzSharedKey(WinMac("Bar ","Outside Bar "), OutsideBar)
#define iszOutsideBar 105
#define szBox           SzSharedKey("Box ",Box)
#define iszBox 106

#define szDefaultFontDef SzSharedKey("Default",Default)

/********************** End of globdefs.h insertion ********************/
#endif /* WIN */


#ifdef WIN
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NONCMESSAGES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW

#define OEMRESOURCE
#define NOSYSMETRICS
#define NOBITMAP
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOFONT
#define NOGDI
#define NOHDC
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOPEN
#define NOPOINT
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#define NOKANJI
#endif /* WIN */

#ifdef  MAC
#define FONTS
#include "toolbox.h"
#endif /* MAC */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "props.h"
#include "border.h"
#include "banter.h"
#include "prm.h"
#include "disp.h"
#include "sel.h"
#include "doc.h"
#include "file.h"
#include "inter.h"
#include "debug.h"
#include "ch.h"
#include "banter.h"

#ifdef WIN
#include "keys.h"
#include "fontwin.h"
#include "winddefs.h"
#include "doslib.h"
#include "wininfo.h"
#endif


#ifdef PROTOTYPE
#include "banter.cpt"
#endif /* PROTOTYPE */

/* E X T E R N A L S */
extern struct PREF vpref;
extern struct ESPRM dnsprm[];
extern struct STTB **vhsttbFont;
extern struct ITR vitr;
extern int vdocStsh;
#ifdef MAC
extern int CbGenChpxFromChp();
#endif /* MAC */
extern BOOL vfSearchRepl;
#ifdef WIN
extern int vfDefineStyle;
#endif /* WIN */
#ifdef MAC
extern int (*vrgftc)[];
extern int viftcMac;
#endif /* MAC */

/* G L O B A L S */
int vibnd;
int vibndCFlagLast;
char *(*vprgpprl)[];


void WriteFlagToBanter(int, int, int, struct TAD *);
void WriteRadioButtonToBanter(int, int, int, struct TAD *);
void WritePosSpacingToBanter(int, int, int, struct TAD *);
void WriteZaToBanter(int, int, int, struct TAD *);
void WriteHpsToBanter(int, int, int, struct TAD *);
void WriteTabsToBanter(int, int, int, struct TAD *);
void WriteHeaderToBanter(int, int, int, struct TAD *);
void WriteCFlagToBanter(int, int, int, struct TAD *);
void WriteBrcpToBanter(int, int, int, struct TAD *);
void WritePcToBanter(int, int, int, struct TAD *);
void WriteDxaAbsToBanter(int, int, int, struct TAD *);
void WriteDyaAbsToBanter(int, int, int, struct TAD *);
void WriteAbsPosToBanter(int, int, int, struct TAD *);
void WriteBrcToBanter(int, int, int, struct TAD *);



#ifdef WIN
void WriteIbstToBanter(int, int, int, struct TAD *);
#endif /* WIN */
#ifdef MAC
void WriteFtcToBanter(int, int, int, struct TAD *);
#endif /* MAC */

csconst void (*rgpfnBanter[])() =
{
	WriteFlagToBanter, WriteRadioButtonToBanter, WritePosSpacingToBanter,
			WriteZaToBanter,
#ifdef WIN
			WriteIbstToBanter,
#endif /* WIN */
#ifdef MAC
			WriteFtcToBanter,
#endif /* MAC */
			WriteHpsToBanter,WriteTabsToBanter, WriteHeaderToBanter,
			WriteCFlagToBanter,WriteBrcpToBanter, WritePcToBanter,
			WriteDxaAbsToBanter, WriteDyaAbsToBanter, WriteBrcToBanter
};


csconst char rgszLabels[][] =
{
	szBoldDef,szItalicDef,szNoUnderlineDef,szUnderlineDef,szWordULDef,
			szDoubleULDef,szDottedULDef,szStrikeDef,szOutlineDef,szShadowDef,
			szSmCapsDef,szCapsDef,szHiddenDef,szPosDef,szSpacingDef,szFontDef,
			szLeftInDef,szFirstInDef,szRightInDef,szLineSpaceDef,szSpacBeforeDef,
			szSpacAfterDef,szTableDef,szPageBreakBeforeDef,szKeepNextDef,
			szKeepTogetherDef,szLineNumberingDef,szBorder,szSingle, szThick,
			szDouble, szDotted, szHairline, szDoubleHairline, szShadowBrcDef,
			szNoBorderDef,szFlushLDef,szCenterDef,szFlushRDef,szFullDef,
			szTabsChangedDef,szIndentDef,szSpaceDef,szPt25Def,szPt5Def,szPt75Def,
			szDottedLeaderDef,szHyphenLeaderDef,szLineLeaderDef,szHeavyLeaderDef,
			szEmptyAlignDef,szCenteredAlignDef,szRightFlAlignDef,szNumberAlignedDef,
#ifdef MAC
			szTabBarDef, 
#endif /* MAC */
			szCommaDef,szSpDef,szNotDef,szSemicolonDef,szHalfDef,szPointDef,
			szPluralDef,szNotAtDef,szPlusDef,szSuperDef,szSubDef,szExpDef,szConDef,
			szRelativeTo, szPosition, szWidth, szAbsLeft, szAbsCenterH, szAbsRight,
			szAbsInside, szAbsOutside, szAbsInLine, szAbsTop, szAbsCenterV,
			szAbsBottom, szColumn, szMargin, szPage, szHorizontal, szVertical,
			szH, szV, szW, szNormalPosition, szLeftParen, szRightParen,
			szSpacingBrc, szTop, szBottom, szBetween, szColor,
#ifdef WIN
			szAuto,
#endif
			szBlack, szBlue, szCyan, szGreen, szMagenta, szRed, szYellow, szWhite,
			szRMarkDef, szOutsideBar, szBox
};


struct BND
	{
	int  fExcp : 1;
	int  fLabelImmediate: 1;
	int  fHeaderEntry: 1;
	int  fDelim : 1;
	int  sprm : 12;
	int  ipfnTextGen : 8;
	int  iszLabel : 8;
};

struct BND *rgbnd;

csconst struct BND rgbndDef[] =
{
	{ fFalse, fFalse, fTrue,  fFalse, 2, ipfnHeader, iszFont },
	{ fFalse, fFalse, fFalse, fFalse, sprmCFtc, ipfnIbst, 0 },
	{ fFalse, fFalse, fFalse, fFalse, sprmCHps, ipfnHps, 0 },
	{ fFalse, fFalse, fFalse, fFalse, sprmCFBold, ipfnCFlag, iszBold },
	{ fFalse, fFalse, fFalse, fFalse, sprmCFItalic, ipfnCFlag, iszItalic },
	{ fFalse, fFalse, fFalse, fFalse, sprmCKul, ipfnRadio, iszNoUnderline },
	{ fFalse, fFalse, fFalse, fFalse, WinMac(sprmCFStrikeRM, sprmCFStrike), ipfnCFlag, iszStrikethru },
	{ fFalse, fFalse, fFalse, fFalse, sprmCFOutline, ipfnCFlag, iszOutline },
	{ fFalse, fFalse, fFalse, fFalse, sprmCFShadow, ipfnCFlag, iszShadow },
	{ fFalse, fFalse, fFalse, fFalse, sprmCFSmallCaps, ipfnCFlag, iszSmCaps },
	{ fFalse, fFalse, fFalse, fFalse, sprmCFCaps, ipfnCFlag, iszCaps },
	{ fFalse, fFalse, fFalse, fFalse, sprmCFVanish, ipfnCFlag, iszHidden },
	{ fFalse, fFalse, fFalse, fFalse, sprmCFRMark, ipfnCFlag, iszRMark },
	{ fFalse, fFalse, fFalse, fFalse, sprmCHpsPos, ipfnPosSpacing, 0 },
	{ fFalse, fFalse, fFalse, fFalse, sprmCQpsSpace, ipfnPosSpacing, 0 },
	{ fFalse, fFalse, fTrue,  fFalse, 1, ipfnHeader, iszColor },
	{ fFalse, fFalse, fFalse, fFalse, sprmCIco, ipfnRadio, WinMac(iszAuto, iszBlack) },
	{ fFalse, fFalse, fTrue,  fFalse, 3, ipfnHeader, iszIndent },
	{ fFalse, fTrue,  fFalse, fFalse, sprmPDxaLeft, ipfnZa, iszLeftIn },
	{ fFalse, fTrue,  fFalse, fFalse, sprmPDxaLeft1, ipfnZa, iszFirstIn },
	{ fFalse, fTrue,  fFalse, fFalse, sprmPDxaRight, ipfnZa, iszRightIn },
	{ fFalse, fFalse, fFalse, fFalse, sprmPJc, ipfnRadio, iszFlushL },
	{ fFalse, fTrue,  fFalse, fFalse, sprmPDyaLine, ipfnZa, iszLineSpace },
	{ fFalse, fFalse, fTrue,  fFalse, 2, ipfnHeader, iszSpace },
	{ fFalse, fTrue,  fFalse, fFalse, sprmPDyaBefore, ipfnZa, iszSpacBefore },
	{ fFalse, fTrue,  fFalse, fFalse, sprmPDyaAfter, ipfnZa, iszSpacAfter },
	{ fFalse, fFalse, fFalse, fFalse, sprmPFSideBySide, ipfnFlag, iszTable },
	{ fFalse, fFalse, fFalse, fFalse, sprmPFPageBreakBefore, ipfnFlag, iszPageBreakBefore },
	{ fFalse, fFalse, fFalse, fFalse, sprmPFKeepFollow, ipfnFlag, iszKeepNext },
	{ fFalse, fFalse, fFalse, fFalse, sprmPFKeep, ipfnFlag, iszKeepTogether },
	{ fFalse, fFalse, fFalse, fFalse, sprmPFNoLineNumb, ipfnFlag, iszLineNumbering },
	{ fFalse, fFalse, fTrue,  fFalse, 7, ipfnHeader, iszBorder },
	{ fFalse, fTrue,  fFalse, fFalse, sprmPBrcTop, ipfnBorder, iszTop },
	{ fFalse, fTrue,  fFalse, fFalse, sprmPBrcBottom, ipfnBorder, iszBottom },
	{ fFalse, fTrue,  fFalse, fFalse, sprmPBrcLeft, ipfnBorder, WinMac(iszAbsLeft, iszLeftIn) },
	{ fFalse, fTrue,  fFalse, fFalse, sprmPBrcRight, ipfnBorder, WinMac(iszAbsRight, iszRightIn) },
	{ fFalse, fTrue,  fFalse, fFalse, sprmPBrcBetween, ipfnBorder, iszBetween },
	{ fFalse, fTrue,  fFalse, fFalse, sprmPBrcBar, ipfnBorder, iszOutsideBar },
	{ fFalse, fTrue,  fFalse, fFalse, sprmNoop, ipfnBorder, iszBox },
	{ fFalse, fFalse, fTrue,  fFalse, 4, ipfnHeader, iszPosition },
	{ fFalse, fFalse, fFalse, fFalse, sprmPDxaAbs, ipfnDxaAbs, 0}


{ 
	fFalse, fTrue,  fFalse, fFalse, sprmPPc, ipfnPc, iszRelativeTo },
	{ fFalse, fFalse, fFalse, fFalse, sprmPDyaAbs, ipfnDyaAbs, 0}


{ 
	fFalse, fTrue,  fFalse, fFalse, sprmPPc, ipfnPc, iszRelativeTo },
	{ fFalse, fTrue,  fFalse, fFalse, sprmPDxaWidth, ipfnZa, iszWidth },
	{ fFalse, fTrue,  fFalse, fFalse, sprmPChgTabsPapx, ipfnTabs, iszTabsChanged}


};


/* %%Function:CchFillStWithBanter %%Owner:davidbo */
int CchFillStWithBanter(st, cchMax, pchp, pchpBase, ppap, ppapBase, ibstFont)
char st[];
int cchMax;
struct CHP *pchp, *pchpBase;
struct PAP *ppap, *ppapBase;
int ibstFont;
{
	int ibnd, ibndT;
	struct BND *pbnd;
	int sprm;
	struct ESPRM esprm;
	int val;
	int cchSprm;
	int iszLabel;
	int fHaveLast;
	int fHaveDxa;
	int fHaveCharFlag;
	int fHaveBrc;
	char *pch;
	int cch;
	char *pprl;
	char rgb1[2], rgb2[2], rgb3[3], rgb4[3];
	struct PCVH pcvh;
	struct DOD *pdod;
	struct TAD tad;
	struct CHP chpx;
	struct CHP chp;
	struct CHP chpBase;
	struct PAP pap;
	struct PAP papBase;
	struct BND rgbndT [ibndMac];
	char papx[cchPapxMax];
	char *rgpprl[ibndPapLast - ibndPapFirst + 1];

	vprgpprl = rgpprl;
	bltbx( rgbndDef, (char far *)rgbndT, sizeof (rgbndDef) );
	rgbnd = rgbndT;

	tad.pchBegin = &st[1];
	tad.cch = 0;
	tad.cchMax = cchMax;
	tad.fOverflow = fFalse;
	tad.fDidntAppend = fFalse;

	chp = *pchp;
	chpBase = *pchpBase;
	pap = *ppap;
	papBase = *ppapBase;

	for (ibnd = 0, pbnd = &rgbnd[0]; ibnd < ibndMac; ibnd++, pbnd++)
		{
		pbnd->fExcp = fFalse;
		pbnd->fDelim = fFalse;
		}

	if (papBase.stc != stcStdMin)
		{
		int stcpBase;
		int cstcStd = PdodDoc(vdocStsh)->stsh.cstcStd;
		struct STTB *hsttbName = PdodDoc(vdocStsh)->stsh.hsttbName;

		stcpBase = StcpFromStc(papBase.stc, cstcStd);
		AppendBaseNameToText(hsttbName, cstcStd, stcpBase, &tad);
		}

	SetBytes(&chpx, 0, cbCHP);
	CbGenChpxFromChp(&chpx, &chp, &chpBase, fFalse);
	if (papBase.stc == stcStdMin && !vfSearchRepl)
		{
		chpx.fsFtc = fTrue;
		chpx.ftc = chp.ftc;
		rgbnd[ibndJc].fExcp = fTrue;
#ifdef MAC
		rgbnd[ibndFtc].fExcp = fTrue;
#endif /* MAC */
		}
#ifdef WIN
	else  if (vfDefineStyle)
		chpx.fsFtc = (ibstFont != IbstFontFromFtcDoc(chpBase.ftc, vdocStsh));
#endif /* WIN */

	cch = CbGrpprlFromPap(fFalse, pprl = &papx, &pap, &papBase, fFalse);

	for (ibnd = ibndBold, pbnd = &rgbnd[ibndBold]; ibnd <= WinMac(ibndRMark, ibndVanish); ibnd++, pbnd++)
		{
		pbnd->fExcp = (ibnd != ibndKul) ?
				ValFromPropSprm(&chpx, pbnd->sprm) : fFalse;
		}

	rgbnd[ibndKul].fExcp = chpx.fsKul;
	rgbnd[ibndFtc].fExcp = chpx.fsFtc;
	rgbnd[ibndHps].fExcp = chpx.fsHps;
	rgbnd[ibndHpsPos].fExcp = chpx.fsPos;
	rgbnd[ibndQpsSpace].fExcp = chpx.fsSpace;
	rgbnd[ibndIco].fExcp = chpx.fsIco;

	rgb1[0] = rgb2[0] = sprmPPc;
	pcvh.op = 0;
	pcvh.pcHorz = pap.pcHorz;
	pcvh.pcVert = pap.pcVert;
	rgb2[1] = pcvh.op;
	pcvh.fVert = fTrue;
	rgb1[1] = pcvh.op;
	rgpprl[ibndPcVert - ibndPapFirst] = rgb1;
	rgpprl[ibndPcHorz - ibndPapFirst] = rgb2;

	rgb3[0] = sprmPDxaAbs;
	bltb(&pap.dxaAbs, &rgb3[1], sizeof(int));
	rgpprl[ibndDxaAbs - ibndPapFirst] = rgb3;

	rgb4[0] = sprmPDyaAbs;
	bltb(&pap.dyaAbs, &rgb4[1], sizeof(int));
	rgpprl[ibndDyaAbs - ibndPapFirst] = rgb4;

	while (cch > 0)
		{
		esprm = dnsprm[sprm = *pprl];
		val = *(pprl + 1);
		if ((cchSprm = esprm.cch) == 0)
			cchSprm = val + 2;
		for (ibnd = ibndPapFirst, pbnd = &rgbnd[ibndPapFirst];
				ibnd <= ibndPapLast; ibnd++, pbnd++)
			{
#ifdef WIN
			Assert(sprm != sprmPChgTabs);
#endif
			if (!pbnd->fHeaderEntry && sprm == pbnd->sprm)
				{
				pbnd->fExcp = fTrue;
				if (sprm == sprmPPc)
					{
					pcvh.op = val;
					if (ibnd == ibndPcVert)
						{
						pcvh.fVert = fTrue;
						rgb1[1] = pcvh.op;
						}
					else
						rgb2[1] = pcvh.op;
					}
				else
					rgpprl[ibnd - ibndPapFirst] = pprl;
				break;
				}
			}
		cch -= cchSprm;
		pprl += cchSprm;
		}

	if (rgbnd[ibndDxaAbs].fExcp || rgbnd[ibndPcHorz].fExcp)
		rgbnd[ibndDxaAbs].fExcp = rgbnd[ibndPcHorz].fExcp = fTrue;
	if (rgbnd[ibndDyaAbs].fExcp || rgbnd[ibndPcVert].fExcp)
		rgbnd[ibndDyaAbs].fExcp = rgbnd[ibndPcVert].fExcp = fTrue;

#ifdef WIN
	/* if (top, bottom, left, right) change to (box) */
	for (ibnd = ibndBrcTop, pbnd = &rgbnd[ibnd]; ; pbnd++)
		{
		if (!pbnd->fExcp)
			break;
		if (ibnd++ == ibndBrcRight)
			{
			pbnd = &rgbnd[ibndBrcTop];
			for (ibnd = ibndBrcTop; ibnd <= ibndBrcRight; ibnd++, pbnd++)
				pbnd->fExcp = fFalse;
			rgbnd[ibndBox].fExcp = fTrue;
			break;
			}
		}
#endif

	/* now all non- header entries of rgbnd that will be displayed have
		had fExcp set to true. */

	vibndCFlagLast = -1;
	fHaveLast = fHaveBrc = fHaveDxa = fHaveCharFlag = fFalse;
	for (ibnd = ibndLast, pbnd = &rgbnd[ibndLast]; ibnd > 0; ibnd--,pbnd--)
		{
		if (pbnd->fHeaderEntry)
			continue;
		if (pbnd->fExcp)
			{
			switch (ibnd)
				{
			case ibndDyaBefore:
				if (fHaveLast && !rgbnd[ibndDyaAfter].fExcp)
					goto LSetDelim;
				break;
			case ibndDxaAbs:
			case ibndDyaAbs:
				break;
			case ibndDxaLeft:
			case ibndDxaLeft1:
			case ibndDxaRight:
				if (fHaveLast && !fHaveDxa && !rgbnd[ibndJc].fExcp)
					{
					fHaveDxa = fTrue;
					goto LSetDelim;
					}
				fHaveDxa = fTrue;
				break;
			case ibndBold:
			case ibndItalic:
			case ibndKul:
			case ibndStrike:
			case ibndOutline:
			case ibndShadow:
			case ibndSmallCaps:
			case ibndCaps:
			case ibndVanish:
			case ibndRMark:
				if (vibndCFlagLast == -1)
					vibndCFlagLast = ibnd;
				if (fHaveLast && !fHaveCharFlag)
					{
					fHaveCharFlag = fTrue;
					goto LSetDelim;
					}
				fHaveCharFlag = fTrue;
				break;
			case ibndBrcTop:
			case ibndBrcBottom:
			case ibndBrcLeft:
			case ibndBrcRight:
			case ibndBrcBetween:
			case ibndBrcBar:
			case ibndBox:
				continue;
			case ibndFtc:
				if (fHaveLast && !rgbnd[ibndHps].fExcp)
					goto LSetDelim;
				break;
			default:
				if (fHaveLast)
					{
LSetDelim:
					pbnd->fDelim = fTrue;
					}
				}
			fHaveLast = fTrue;
			}
		}
/* now all entries of rgbnd which will end with a delimiter have
	fDelim set */

	for (ibnd = 0, pbnd = &rgbnd[0]; ibnd < ibndMac; ibnd++, pbnd++)
		{
		if (pbnd->fExcp || pbnd->fHeaderEntry)
			{
			vibnd = ibnd;
			sprm = pbnd->sprm;
			iszLabel = pbnd->iszLabel;
			if (pbnd->fLabelImmediate)
				AppendLabelToText(iszLabel, &tad);
#ifdef WIN
			if (ibnd == ibndFtc)  /* special case */
				val = ibstFont;
			else
#endif /* WIN */
				if (pbnd->fHeaderEntry)
					val = ibnd;
				else  if (ibnd < ibndPapFirst)
					{
					val = ValFromPropSprm(&chp, sprm);
					if (ibnd == ibndHpsPos || ibnd == ibndQpsSpace)
						val = val | (ValFromPropSprm(&chpBase, sprm) << 8);
					}
				else
					{
					ibndT = ibnd;
#ifdef WIN
					/* hack for Box property */
					if (sprm == sprmNoop)
						{
						ibndT = ibndBrcRight;
						sprm = sprmPBrcRight;
						}
#endif
					pprl = rgpprl[ibndT - ibndPapFirst];
					cch = dnsprm[sprm].cch;
					if (cch == 2)
						val = *(pprl + 1);
					else  if (cch == 3)
						bltb(pprl + 1, &val, sizeof(int));
					else
						val = (uns) pprl;
					}
			(*rgpfnBanter[pbnd->ipfnTextGen])(val, sprm, iszLabel, &tad);
			if (pbnd->fDelim)
				AppendLabelToText(iszComma, &tad);
			if (tad.fDidntAppend)
				tad.fDidntAppend = fFalse;
			else
				AppendLabelToText(iszSp, &tad);
			}
		}

	if (tad.fOverflow)
		{
#ifdef WIN
		Assert(cchMax >= 3);
		st[cchMax] = st[cchMax-1] = st[cchMax-2] = /*chEllipsis*/'.';
#endif /* WIN */
#ifdef MAC
		if (tad.fOverflow)
			st[255] = chEllipsis;
#endif /* MAC */
		}
	st[0] = tad.cch;
	return (tad.cch);
}


/* %%Function:WriteFlagToBanter %%Owner:davidbo */
void WriteFlagToBanter(val, sprm, iszLabel, ptad)
int val;
int sprm;
int iszLabel;
struct TAD *ptad;
{
	char *sz;

	if (sprm == sprmPFNoLineNumb)
		val = 1 - val;
	if (val == 0)
		AppendLabelToText(iszNot, ptad);
	AppendLabelToText(iszLabel, ptad);
}


/* %%Function:WriteCFlagToBanter %%Owner:davidbo */
void WriteCFlagToBanter(val, sprm, iszLabel, ptad)
int val;
int sprm;
int iszLabel;
struct TAD *ptad;
{
	WriteFlagToBanter(val, sprm, iszLabel, ptad);
	if (val == 0 && vibnd != vibndCFlagLast)
		AppendLabelToText(iszSemicolon, ptad);
}


/* %%Function:WriteBrcpToBanter %%Owner:davidbo */
void WriteBrcpToBanter(val, sprm, iszLabel, ptad)
int val;
int sprm;
int iszLabel;
struct TAD *ptad;
{
	switch (val)
		{
	case brcpBox: /*15 */
	case brcpBar: /*16*/
		val -= 14;
		break;
	case brcpAbove: /* 1 */
	case brcpBelow: /* 2 */
		val += 2;
		break;
		}
	WriteRadioButtonToBanter(val, sprm, iszLabel, ptad);
}


/* %%Function:WriteRadioButtonToBanter %%Owner:davidbo */
void WriteRadioButtonToBanter(val, sprm, iszLabel, ptad)
int val;
int sprm;
int iszLabel;
struct TAD *ptad;
{
	AppendLabelToText(iszLabel + val, ptad);
}


/* %%Function:WritePosSpacingToBanter %%Owner:davidbo */
void WritePosSpacingToBanter(val, sprm, iszLabel, ptad)
int val;
int sprm;
int iszLabel;
struct TAD *ptad;
{
	int fPos, fPositive;
	int valOld;
	int valNew;
	int valBreak;
	int valCompBase;
	int isz1;
	int isz2;
	int ps;
	char *pch;
	char szVal[ichMaxNum];
	char szDecimal[2];

	valNew = val & 0xff;
	valOld = val >> 8;
	if (fPos = (sprm == sprmCHpsPos))
		{
		valBreak = 128;
		valCompBase = 256;
		isz1 = iszSuper;
		isz2 = iszSub;
		}
	else
		{
		valBreak = 57;
		valCompBase = 64;
		isz1 = iszExp;
		isz2 = iszCon;
		}

	if (valNew == 0)
		{
		AppendLabelToText(iszNot, ptad);
		AppendLabelToText((valOld < valBreak) ? isz1 : isz2, ptad);
		return;
		}

	AppendLabelToText((fPositive = valNew < valBreak) ? isz1 : isz2, ptad);

	if (!fPositive)
		valNew = valCompBase - valNew;
	if (fPos)
		ps = valNew >> 1;
	else
		ps = valNew >> 2;
	pch = szVal;
	CchIntToPpch(ps, &pch);
	*pch = 0;
	AppendLpszToText((char far *) szVal, ptad);
	szDecimal[0] = vitr.chDecimal;
	szDecimal[1] = '\0';
	if (fPos)
		{
		if (valNew & 0x1)
			{
			AppendLpszToText((char far *) szDecimal, ptad);
			AppendLabelToText(iszHalf, ptad);
			}
		}
	else
		{
		int isz;
		if (isz = valNew & 0x3)
			{
			AppendLpszToText((char far *) szDecimal, ptad);
			AppendLabelToText(iszPt25 + isz - 1, ptad);
			}
		}
	AppendLabelToText(iszPoint, ptad);
	if (!fPos)
		AppendLabelToText(iszPlural, ptad);
}


/* %%Function:WriteZaToBanter %%Owner:davidbo */
void WriteZaToBanter(val, sprm, iszLabel, ptad)
int val;
int sprm;
int iszLabel;
struct TAD *ptad;
{
	int utSave;
	char *pch;
	char sz[21];

	utSave = -1;
	if (sprm == sprmPDyaLine || sprm == sprmPDyaBefore || sprm == sprmPDyaAfter ||
			(sprm >= sprmPBrcTop && sprm <= sprmPBrcBetween))
		{
		utSave = vpref.ut;
		vpref.ut = utPt;
		}
	sz[0] = ' ';
	pch = sz + 1;
#ifdef WIN
	CchExpZa(&pch, val, vpref.ut, 21, fTrue);
#endif /* WIN */
#ifdef MAC
	CchExpZa(&pch, val, fTrue);
#endif /* WIN */
	*pch = 0;
	AppendLpszToText((char far *) sz, ptad);
	if (utSave != -1)
		vpref.ut = utSave;
}


#ifdef MAC
/* %%Function:WriteFtcToBanter %%Owner:NOTUSED */
void WriteFtcToBanter(val, sprm, iszLabel, ptad)
int val;
int sprm;
int iszLabel;
struct TAD *ptad;
{
	int iftc;
	char stBuf[64];

	for (iftc = 0; (*vrgftc)[iftc] != val && iftc < viftcMac; iftc++) ;
	if (iftc == viftcMac)
		return;
	GetFontName((*vrgftc)[iftc], stBuf);
	stBuf[stBuf[0] + 1] = 0;
	AppendLpszToText((char far *)stBuf + 1, ptad);
}


#endif /* MAC */

#ifdef WIN
/* %%Function:WriteIbstToBanter %%Owner:davidbo */
void WriteIbstToBanter(ibst, sprm, iszLabel, ptad)
int ibst;
int sprm;
int iszLabel;
struct TAD *ptad;
{
	Assert((uns)ibst < (*vhsttbFont)->ibstMac);

	AppendLpszToText( (char far *)((struct FFN *)
			PstFromSttb(vhsttbFont,ibst))->szFfn,
			ptad );
}


#endif /* WIN */

/* %%Function:WriteHpsToBanter %%Owner:davidbo */
void WriteHpsToBanter(val, sprm, iszLabel, ptad)
int val;
int sprm;
int iszLabel;
struct TAD *ptad;
{
	int ps;
	char *pch;
	char sz[ichMaxNum];

	ps = val >> 1;
	pch = sz;
	CchIntToPpch(ps, &pch);
	*pch = 0;
	if (val % 2)
		{
		*pch++ = vitr.chDecimal;
		*pch++ = '\0';
		}
	AppendLpszToText((char far *) sz, ptad);
	if (val % 2)
		AppendLabelToText(iszHalf, ptad);
	AppendLabelToText(iszPoint, ptad);
}


/* %%Function:WriteTabsToBanter %%Owner:davidbo */
void WriteTabsToBanter(val, sprm, iszLabel, ptad)
int val;
int sprm;
int iszLabel;
struct TAD *ptad;
{
	char *pprlCur;
	int idxa;
	int dxa;
	int idxaAdd;
	int idxaDel;
	int *pdxaAdd;
	int *pdxaDel;
	char *ptbdAdd;
	struct TBD tbd;
	int jc;
	int tlc;

	pprlCur = (char *) val + 2;
	idxaDel = *pprlCur++;
	pdxaDel = pprlCur;
	idxaAdd = *(pprlCur += idxaDel * 2);
	pdxaAdd = ++pprlCur;
	ptbdAdd = (pprlCur += idxaAdd * 2);
	for (idxa = 0; idxa < idxaAdd; idxa++, pdxaAdd++, ptbdAdd++)
		{
		bltb(pdxaAdd, &dxa, sizeof(int));
		WriteZaToBanter(dxa, sprm, iszLabel, ptad);

		bltb(ptbdAdd, &tbd, 1);
		jc = tbd.jc;
		WriteRadioButtonToBanter(jc, sprm, iszEmptyAlign, ptad);
		if (tlc = tbd.tlc)
			{
			AppendLabelToText(iszSp, ptad);
			WriteRadioButtonToBanter(tlc, sprm, iszDottedLeader-1, ptad);
			}
		if (idxaDel != 0 || idxa + 1 < idxaAdd)
			AppendLabelToText(iszSemicolon,ptad);
		}

	for (idxa = 0; idxa < idxaDel; idxa++, pdxaDel++)
		{
		if (idxa == 0)
			AppendLabelToText(iszNotAt, ptad);
		bltb(pdxaDel, &dxa, sizeof(int));
		WriteZaToBanter(dxa, sprm, iszLabel, ptad);
		if (idxa + 1 < idxaDel)
			{
			AppendLabelToText(iszComma, ptad);
			AppendLabelToText(iszSp, ptad);
			}
		}
}


/* %%Function:WriteHeaderToBanter %%Owner:davidbo */
void WriteHeaderToBanter(val, sprm, iszLabel, ptad)
int val;
int sprm;
int iszLabel;
struct TAD *ptad;
{
	int ibnd;
	int ibndLim;

	ibndLim = val + sprm + 1;
	for (ibnd = val + 1; ibnd < ibndLim; ibnd++)
		{
		if (rgbnd[ibnd].fExcp)
			{
			AppendLabelToText(iszLabel, ptad);
			return;
			}
		}
	ptad->fDidntAppend = fTrue;
}


/* %%Function:WritePcToBanter %%Owner:davidbo */
void WritePcToBanter(val, sprm, iszLabel, ptad)
int val;
int sprm;
int iszLabel;
struct TAD *ptad;
{
	struct PCVH pcvh;
	pcvh.op = val;
	val = pcvh.fVert ? pcvh.pcVert : pcvh.pcHorz;
	WriteRadioButtonToBanter(val, sprm, pcvh.fVert ? iszMargin : iszColumn, ptad);
}


/* %%Function:WriteDxaAbsToBanter %%Owner:davidbo */
void WriteDxaAbsToBanter(val, sprm, iszLabel, ptad)
int val;
int sprm;
int iszLabel;
struct TAD *ptad;
{
	WriteAbsPosToBanter(val, sprm, iszAbsLeft, ptad);
	AppendLabelToText(iszHorizontal, ptad);
}


/* %%Function:WriteDyaAbsToBanter %%Owner:davidbo */
void WriteDyaAbsToBanter(val, sprm, iszLabel, ptad)
int val;
int sprm;
int iszLabel;
struct TAD *ptad;
{
	WriteAbsPosToBanter(val, sprm, iszAbsInLine, ptad);
	AppendLabelToText(iszVertical, ptad);
}


/* %%Function:WriteAbsPosToBanter %%Owner:davidbo */
void WriteAbsPosToBanter(val, sprm, iszLabel, ptad)
int val;
int sprm;
int iszLabel;
struct TAD *ptad;
{
	if (val > 0)
		WriteZaToBanter(--val, sprm, iszLabel, ptad);
	else
		{
		val = (- val) / 4;
		WriteRadioButtonToBanter(val, sprm, iszLabel, ptad);
		}
}


/* %%Function:WriteBrcToBanter %%Owner:davidbo */
void WriteBrcToBanter(val, sprm, iszLabel, ptad)
int val;
int sprm;
int iszLabel;
struct TAD *ptad;
{
	int ibnd;
	int tmval;
	char *pprl;
	struct BRC brc, brcLater;

/* find the index of the BND entry after the one that corresponds to sprm */
	switch (sprm)
		{
	case sprmPBrcTop:
		ibnd = ibndBrcTop + 1;
		break;
	case sprmPBrcBottom:
		ibnd = ibndBrcBottom + 1;
		break;
	case sprmPBrcLeft:
		ibnd = ibndBrcLeft + 1;
		break;
	case sprmPBrcRight:
		ibnd = ibndBrcRight + 1;
		break;
	case sprmPBrcBetween:
		ibnd = ibndBrcBetween + 1;
		break;
	case sprmPBrcBar:
		ibnd = ibndBrcBar + 1;
		break;
#ifdef WIN
	case sprmNoop:
		ibnd = ibndBrcBar + 1;
		break;
#endif
		}
	bltb(&val, &brc, sizeof(int));
/* if there is a following brc that has the same brc value as this one
	we return */
	for (; ibnd <= ibndBrcBar; ibnd++)
		{
		if (rgbnd[ibnd].fExcp)
			{
			pprl = (*vprgpprl)[ibnd - ibndPapFirst];
			bltb(pprl + 1, &brcLater, sizeof(int));
			if (brc.brc == brcLater.brc)
				return;
			break;
			}
		}
/* else we must display the current brc contents */
	AppendLabelToText(iszLeftParen, ptad);
	if ((tmval = IstFromBrc(brc)) == 0)
		AppendLabelToText(iszNoBorder, ptad);
	else
		{
#ifdef WIN
		if (brc.fShadow)
			AppendLabelToText(iszShadow, ptad);
		else
			WriteRadioButtonToBanter(tmval, sprm, iszBorder, ptad);
#else
/* FUTURE: If Opus ever has the user interface to change border
					spacing, remove the #ifdef and resurrect this code! */
		WriteRadioButtonToBanter(tmval, sprm, iszBorder, ptad);
		if (brc.fShadow)
			{
			AppendLabelToText(iszSp, ptad);
			AppendLabelToText(iszShadowBrc, ptad);
			}
		if (brc.dxpSpace)
			{
			WriteZaToBanter(brc.dxpSpace * czaPoint, sprm, iszLabel, ptad);
			AppendLabelToText(iszSpacingBrc, ptad);
			}
#endif
		}
	AppendLabelToText(iszRightParen, ptad);
}


/* %%Function:AppendLpszToText %%Owner:davidbo */
AppendLpszToText(lpch, ptad)
char far *lpch;
struct TAD *ptad;
{
	char *pchT;
	int cch = ptad->cch;
	int cchMax = ptad->cchMax;

		{{ /* NATIVE - short loop in AppendLpszToText */
		for (pchT = ptad->pchBegin + cch; cch < cchMax && *lpch != 0; cch++, pchT++, lpch++)
			*pchT = *lpch;
		}}

	if (*lpch != 0)
		ptad->fOverflow = fTrue;
	ptad->cch = cch;
}


/* %%Function:AppendBaseNameToText %%Owner:davidbo */
AppendBaseNameToText(hsttbName, cstcStd, stcpBase, ptad)
struct STTB **hsttbName;
int cstcStd;
int stcpBase;
struct TAD *ptad;
{
	char st[cchMaxSz];

	GenStyleNameForStcp(st, hsttbName, cstcStd, stcpBase);
	st[st[0] + 1] = 0;
	AppendLpszToText((char far *) &st[1], ptad);
	AppendLabelToText(iszPlus, ptad);
}


/* %%Function:AppendLabelToText %%Owner:davidbo */
AppendLabelToText(iLabel, ptad)
int iLabel;
struct TAD *ptad;
{
	AppendLpszToText((char far *) rgszLabels[iLabel], ptad);
}


#ifdef WIN

#define ibrcNone        0
#define ibrcPalFirst    1
#define ibrcSingle      1
#define ibrcThick       2
#define ibrcTwoSingle   3
#define ibrcDotted      4
#define ibrcHairline    5

/* I S T  F R O M  B R C */
/* Takes a brc and returns the correct ist */
/* %%Function:IstFromBrc %%Owner:davidbo */
IstFromBrc(brc)
int brc;
{
	int ist = tmvalNinch;

	switch (brc & (brcLine1 | brcSpace2 | brcLine2))
		{
	case brcNone:    /* Nothing */
		ist = ibrcNone;
		break;
	case brcSingle:    /* Single line */
		ist = ibrcSingle;
		break;
	case brcThick:    /* Thick (2-pixel) Line */
		ist = ibrcThick;
		break;
	case brcTwoSingle:    /* Double line */
		ist = ibrcTwoSingle;
		break;
	case brcDotted:    /* Dotted */
		ist = ibrcDotted;
		break;
	case brcHairline:    /* Hairline */
		ist = ibrcHairline;
		break;
		}

	return ist;
}


#endif
