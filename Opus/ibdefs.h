/* I B D E F S . H */
/*  Definitions of iconbars */


/** WARNING ***  The numbers below are used to compute hid's for iconbar **/
/**              help.  If you change them, you must insure that the cxt **/
/**              can still be computed by CxtFromIbc (in file help.h)    **/
/** WARNING ***  The numbers below are also hard-coded into the CBT   **/
/**              semantic event hooks.  If you change them, make sure **/
/**              that we still pass the correct numbers to CBT with   **/
/**              the icon bar semantic event messages.  Or better     **/
/**              yet, don't change them at all.                       **/
#define ibcHdr      0
#define ibcOutline  1
#define ibcMcr      2
#define ibcRibbon   3
#define ibcRuler    4
#define ibcPreview  5


#define iibbHdrPage     0
#define iibbHdrDate     1
#define iibbHdrTime     2
#define iibbHdrLinkPrev 3
#define iibbHdrRetToDoc 4
#define iibbHdrIBarDesp 5

#ifdef WIN23

/*****
Win 2 style iconbars for WIN23 version
*****/

#ifdef HDRIB
ibdHeader(ibdHdr2, dyIconBar, fTrue, ibcHdr, fFalse, fFalse, NULL)
	ibidBitmap(idrbHdrIconBar2, 0)
	ibidToggleBmp(5,1,10,dyIconBar-4,iibbHdrPage, bcmHdrPage, fFalse)
	ibidToggleBmp(20,1,10,dyIconBar-4,iibbHdrDate, bcmHdrDate, fFalse)
	ibidToggleBmp(35,1,10,dyIconBar-4,iibbHdrTime, bcmHdrTime, fFalse)
	ibidTextIibb(50,2,96,dyIconBar-5,"",iibbHdrIBarDesp)
	ibidToggleText(162,1,72,dyIconBar-4,"",iibbHdrLinkPrev,
			bcmHdrLinkPrev)
	ibidToggleText(237,1,30,dyIconBar-4,SzSharedKey("&Close",MenuClose),
			iibbHdrRetToDoc, bcmHdrRetToDoc)
ibdEnd
#endif /* HDRFTRIB */


#define IDMCRGO         0
#define IDMCRTRACE      1
#define IDMCRSTEP       2
#define IDMCRANI        3
#define IDMCRVARS       4
#define IDMCRNAME       5

#ifdef MCRIB
#define IDMCRNAME   5
extern IbProcMacroEdit();
ibdHeader(ibdMcr2, dyRibbon, fTrue, ibcMcr, fTrue, fFalse, IbProcMacroEdit)
	ibidToggleText(5,1,40,dyRibbon-4,SzSharedKey("&Start",Start),IDMCRGO, bcmContinueMacro)
	ibidToggleText(50,1,40,dyRibbon-4,SzSharedKey("St&ep",Step),IDMCRTRACE, bcmTraceMacro)
	ibidToggleText(95,1,40,dyRibbon-4,SzSharedKey("Step S&UBs",StepSUBs),IDMCRSTEP, bcmStepMacro)
	ibidToggleText(140,1,40,dyRibbon-4,SzSharedKey("T&race",Trace),IDMCRANI,bcmAnimateMacro)
	ibidToggleText(185,1,40,dyRibbon-4,SzSharedKey("&Vars...",Vars),IDMCRVARS,bcmShowVars)
	ibidTextIibb(230,3,100,dyRibbon-4,"",IDMCRNAME)
ibdEnd


#endif /* MCRIB */


#define IDOIBPROMOTE      0
#define IDOIBDEMOTE       1
#define IDOIBMOVEUP       2
#define IDOIBMOVEDOWN     3
#define IDOIBBODY         4
#define IDOIBLVLPLUS1     5
#define IDOIBLVLMINUS1    6
#define IDOIBLVL1         7
#define IDOIBLVL2         8
#define IDOIBLVL3         9
#define IDOIBLVL4        10
#define IDOIBLVL5        11
#define IDOIBLVL6        12
#define IDOIBLVL7        13
#define IDOIBLVL8        14
#define IDOIBLVL9        15
#define IDOIBALL         16

/* The number of various types of buttons: used by CBT, should be updated
	if any buttons are added to the list above. */
#define cbtnOutlinePromote    5
#define cbtnOutlinePlus       2

#ifdef OUTLINEIB
ibdHeader(ibdOutline2, dyIconBar, fTrue, ibcOutline, fFalse, fFalse, NULL)
	ibidBitmap(idrbOutlineIcnBr2,0)
	ibidToggleBmp(5,1,10,dyIconBar-4,IDOIBPROMOTE,bcmPromote, fFalse)
	ibidToggleBmp(17,1,10,dyIconBar-4,IDOIBDEMOTE,bcmDemote, fFalse)
	ibidToggleBmp(29,1,10,dyIconBar-4,IDOIBMOVEUP,bcmMoveUp, fFalse)
	ibidToggleBmp(41,1,10,dyIconBar-4,IDOIBMOVEDOWN,bcmMoveDown, fFalse)
	ibidToggleBmp(53,1,10,dyIconBar-4,IDOIBBODY,bcmConvertToBody, fFalse)
	ibidToggleBmp(77,1,10,dyIconBar-4,IDOIBLVLPLUS1,bcmExpand, fFalse)
	ibidToggleBmp(89,1,10,dyIconBar-4,IDOIBLVLMINUS1,bcmCollapse, fFalse)
	ibidText(111,2,20,dyIconBar-4, SzSharedKey("Show:",Show))
	ibidToggleBmp(135,1,10,dyIconBar-4,IDOIBLVL1,bcmShowToLevel1, fFalse)
	ibidToggleBmp(145,1,10,dyIconBar-4,IDOIBLVL2,bcmShowToLevel2, fFalse)
	ibidToggleBmp(155,1,10,dyIconBar-4,IDOIBLVL3,bcmShowToLevel3, fFalse)
	ibidToggleBmp(165,1,10,dyIconBar-4,IDOIBLVL4,bcmShowToLevel4, fFalse)
	ibidToggleBmp(175,1,10,dyIconBar-4,IDOIBLVL5,bcmShowToLevel5, fFalse)
	ibidToggleBmp(185,1,10,dyIconBar-4,IDOIBLVL6,bcmShowToLevel6, fFalse)
	ibidToggleBmp(195,1,10,dyIconBar-4,IDOIBLVL7,bcmShowToLevel7, fFalse)
	ibidToggleBmp(205,1,10,dyIconBar-4,IDOIBLVL8,bcmShowToLevel8, fFalse)
	ibidToggleBmp(215,1,10,dyIconBar-4,IDOIBLVL9,bcmShowToLevel9, fFalse)
	ibidToggleText(225,1,20,dyIconBar-4,SzSharedKey("All",All),IDOIBALL,bcmExpandAll)
ibdEnd
#endif /* OUTLINEIB */

#define IDLKSDIALOG         0
#define IDLKSBOLD           1
#define IDLKSITALIC         2
#define IDLKSSMALLCAPS      3
#define IDLKSUNDERLINE      4
#define IDLKSWORDULINE      5
#define IDLKSDOUBLEULINE    6
#define IDLKSSUPERSCRIPT    7
#define IDLKSSUBSCRIPT      8
#define IDLKSSHOWALL        9
#define IDLKSFONT           10
#define IDLKSPOINT          11

#ifdef RIBBON
#include "ribbon.hs"
#include "ribbon.sdm"
IbProcRibbon();
ibdHeader(ibdRibbon2, dyRibbon, fTrue, ibcRibbon, fTrue, fFalse, IbProcRibbon)
	ibidDialog(0,0,cabiCABRIBBON,sizeof(dltRibbon),IDLKSDIALOG)
	ibidDlgItem(tmcFont,IDLKSFONT)
	ibidDlgItem(tmcPoints,IDLKSPOINT)
	ibidBitmap(idrbRibbon2, 0)
	ibidToggleBmp(175,1,10,dyRibbon-ddyIconLH,IDLKSBOLD, bcmNil, fFalse)
	ibidToggleBmp(186,1,10,dyRibbon-ddyIconLH,IDLKSITALIC, bcmNil, fFalse)
	ibidToggleBmp(197,1,10,dyRibbon-ddyIconLH,IDLKSSMALLCAPS, bcmNil, fFalse)
	ibidBitmap(idrbRibbon2, 3)
	ibidToggleBmp(211,1,10,dyRibbon-ddyIconLH,IDLKSUNDERLINE, bcmNil, fFalse)
	ibidToggleBmp(222,1,10,dyRibbon-ddyIconLH,IDLKSWORDULINE, bcmNil, fFalse)
	ibidToggleBmp(233,1,10,dyRibbon-ddyIconLH,IDLKSDOUBLEULINE, bcmNil, fFalse)
	ibidToggleBmp(281,1,10,dyRibbon-ddyIconLH,IDLKSSHOWALL, bcmShowAll, fFalse)
	ibidBitmap(idrbScrptPos2, 0)
	ibidToggleBmp(247,1,8,6,IDLKSSUPERSCRIPT, bcmNil, fFalse)
	ibidToggleBmp(247,6,8,6,IDLKSSUBSCRIPT, bcmNil, fFalse)
ibdEnd


#endif /* RIBBON */


#define idRulDialog     0 
#define idRulParaLeft   1     /* order here to paraopen important */
#define idRulParaCenter 2     /*  LCD struct in cmd.c depends on it */
#define idRulParaRight  3    /*  also paraleft must be one !!     */
#define idRulParaBoth   4
#define idRulSpace1     5
#define idRulSpace15    6
#define idRulSpace2     7
#define idRulParaClose  8
#define idRulParaOpen   9
#define idRulTabLeft    10
#define idRulTabCenter  11
#define idRulTabRight   12
#define idRulTabDecimal 13
#define idRulStyle      14
#define idRulMode       15
#define idRulMark       16

/* The number of various types of buttons: used by CBT, should be updated
	if any buttons are added to the list above. */
#define cbtnRulerAlign    4
#define cbtnRulerSpacing  5
#define cbtnRulerTabs     (4+1)  /* real value + 1 to step over idRulStyle */

#ifdef RULERIB
#include "ruler.hs"
#include "ruler.sdm"
IbProcRuler();
ibdHeader(ibdRuler2, dyRuler2, fTrue, ibcRuler, fFalse, fFalse, IbProcRuler)
	ibidDialog(0,0,cabiCABRULER,sizeof(dltRuler),idRulDialog)
	ibidDlgItem(tmcStyle,idRulStyle)
	ibidBitmap(idrbRulerAlign2, 0)
	ibidToggleBmp(115,1,12,12,idRulParaLeft,bcmNil, fFalse)
	ibidToggleBmp(128,1,12,12,idRulParaCenter,bcmNil, fFalse)
	ibidToggleBmp(141,1,12,12,idRulParaRight,bcmNil, fFalse)
	ibidToggleBmp(154,1,12,12,idRulParaBoth,bcmNil, fFalse)
	ibidBitmap(idrbRulerToggles2, 0)
	ibidToggleBmp(175,1,8,12,idRulSpace1,bcmNil, fFalse)
	ibidToggleBmp(184,1,8,12,idRulSpace15,bcmNil, fFalse)
	ibidToggleBmp(193,1,8,12,idRulSpace2,bcmNil, fFalse)
	ibidToggleBmp(205,1,8,12,idRulParaClose,bcmNil, fFalse)
	ibidToggleBmp(214,1,8,12,idRulParaOpen,bcmNil, fFalse)
	ibidToggleBmp(234,1,8,12,idRulTabLeft,bcmNil, fFalse)
	ibidToggleBmp(243,1,8,12,idRulTabCenter,bcmNil, fFalse)
	ibidToggleBmp(252,1,8,12,idRulTabRight,bcmNil, fFalse)
	ibidToggleBmp(261,1,8,12,idRulTabDecimal,bcmNil, fFalse)
	ibidBitmap(idrbRulerAlign2, 4)
	ibidToggleBmp(279,1,12,12,idRulMode,bcmRulerMode, fFalse)
	ibidCustomWnd(1,14,10,4,"",idRulMark)
ibdEnd
#endif /* RULERIB */


#define iibbPrvwPrint     0
#define iibbPrvwBound     1
#define iibbPrvwPages     2
#define iibbPrvwPgvw      3
#define iibbPrvwClose     4
#define iibbPrvwStats     5

#ifdef PREVIEWIB
ibdHeader(ibdPrvw2, dyRibbon, fFalse, ibcPreview, fTrue, fFalse, NULL)
	ibidToggleText(5,1,40,dyRibbon-4,SzSharedKey("&Print...",Print),iibbPrvwPrint,bcmPrint)
	ibidToggleText(50,1,45,dyRibbon-4,SzSharedKey("&Boundaries",Boundaries),iibbPrvwBound,bcmPrvwBound)
	ibidToggleText(100,1,40,dyRibbon-4,"",iibbPrvwPages,bcmPrvwPages)
	ibidToggleText(145,1,40,dyRibbon-4,SzSharedKey("Page &View",PageView),iibbPrvwPgvw,bcmPageView)
	ibidToggleText(190,1,40,dyRibbon-4,SzSharedKey("&Cancel",Cancel),iibbPrvwClose,bcmPrintPreview)
	ibidTextIibb(235,3,80,dyRibbon-4, "", iibbPrvwStats)
ibdEnd
#endif /* PREVIEWIB */

/*******
Win 3 style iconbars for WIN23 version
*******/


#ifdef HDRIB
ibdHeader(ibdHdr3, dyIconBar, fTrue, ibcHdr, fFalse, fFalse, NULL)
	ibidBitmap(idrbHdrIconBar3, 0)
	ibidToggleBmp(5,1,16,dyIconBar-4,iibbHdrPage, bcmHdrPage, fFalse)
	ibidToggleBmp(23,1,16,dyIconBar-4,iibbHdrDate, bcmHdrDate, fFalse)
	ibidToggleBmp(41,1,16,dyIconBar-4,iibbHdrTime, bcmHdrTime, fFalse)
	ibidTextIibb(66,2,140,dyIconBar-5,"",iibbHdrIBarDesp)
	ibidToggleText(216,1,96,dyIconBar-4,"",iibbHdrLinkPrev,
			bcmHdrLinkPrev)
	ibidToggleText(316,1,40,dyIconBar-4,SzSharedKey("&Close",MenuClose),
			iibbHdrRetToDoc, bcmHdrRetToDoc)
ibdEnd
#endif /* HDRFTRIB */


#define IDMCRGO         0
#define IDMCRTRACE      1
#define IDMCRSTEP       2
#define IDMCRANI        3
#define IDMCRVARS       4
#define IDMCRNAME       5

#ifdef MCRIB
#define IDMCRNAME   5
extern IbProcMacroEdit();
ibdHeader(ibdMcr3, dyRibbon, fTrue, ibcMcr, fTrue, fFalse, IbProcMacroEdit)
	ibidToggleText(5,1,53,dyRibbon-4,SzSharedKey("&Start",Start),IDMCRGO, bcmContinueMacro)
	ibidToggleText(65,1,53,dyRibbon-4,SzSharedKey("St&ep",Step),IDMCRTRACE, bcmTraceMacro)
	ibidToggleText(125,1,53,dyRibbon-4,SzSharedKey("Step S&UBs",StepSUBs),IDMCRSTEP, bcmStepMacro)
	ibidToggleText(185,1,53,dyRibbon-4,SzSharedKey("T&race",Trace),IDMCRANI,bcmAnimateMacro)
	ibidToggleText(245,1,53,dyRibbon-4,SzSharedKey("&Vars...",Vars),IDMCRVARS,bcmShowVars)
	ibidTextIibb(305,3,133,dyRibbon-4,"",IDMCRNAME)
ibdEnd


#endif /* MCRIB */


#define IDOIBPROMOTE      0
#define IDOIBDEMOTE       1
#define IDOIBMOVEUP       2
#define IDOIBMOVEDOWN     3
#define IDOIBBODY         4
#define IDOIBLVLPLUS1     5
#define IDOIBLVLMINUS1    6
#define IDOIBLVL1         7
#define IDOIBLVL2         8
#define IDOIBLVL3         9
#define IDOIBLVL4        10
#define IDOIBLVL5        11
#define IDOIBLVL6        12
#define IDOIBLVL7        13
#define IDOIBLVL8        14
#define IDOIBLVL9        15
#define IDOIBALL         16

/* The number of various types of buttons: used by CBT, should be updated
	if any buttons are added to the list above. */
#define cbtnOutlinePromote    5
#define cbtnOutlinePlus       2

#ifdef OUTLINEIB
ibdHeader(ibdOutline3, dyIconBar, fTrue, ibcOutline, fFalse, fFalse, NULL)
	ibidBitmap(idrbOutlineIcnBr3,0)
	ibidToggleBmp(6,1,16,dyIconBar-4,IDOIBPROMOTE,bcmPromote, fFalse)
	ibidToggleBmp(22,1,16,dyIconBar-4,IDOIBDEMOTE,bcmDemote,  fFalse)
	ibidToggleBmp(38,1,16,dyIconBar-4,IDOIBMOVEUP,bcmMoveUp,  fFalse)
	ibidToggleBmp(54,1,16,dyIconBar-4,IDOIBMOVEDOWN,bcmMoveDown, fFalse)
	ibidToggleBmp(70,1,16,dyIconBar-4,IDOIBBODY,bcmConvertToBody, fFalse)
	ibidToggleBmp(96,1,16,dyIconBar-4,IDOIBLVLPLUS1,bcmExpand, fFalse)
	ibidToggleBmp(112,1,16,dyIconBar-4,IDOIBLVLMINUS1,bcmCollapse, fFalse)
	ibidText(141,2,26,dyIconBar-4, SzSharedKey("Show:",Show))
	ibidToggleBmp(172,1,16,dyIconBar-4,IDOIBLVL1,bcmShowToLevel1, fTrue)
	ibidToggleBmp(188,1,16,dyIconBar-4,IDOIBLVL2,bcmShowToLevel2, fTrue)
	ibidToggleBmp(204,1,16,dyIconBar-4,IDOIBLVL3,bcmShowToLevel3, fTrue)
	ibidToggleBmp(220,1,16,dyIconBar-4,IDOIBLVL4,bcmShowToLevel4, fTrue)
	ibidToggleBmp(236,1,16,dyIconBar-4,IDOIBLVL5,bcmShowToLevel5, fTrue)
	ibidToggleBmp(252,1,16,dyIconBar-4,IDOIBLVL6,bcmShowToLevel6, fTrue)
	ibidToggleBmp(268,1,16,dyIconBar-4,IDOIBLVL7,bcmShowToLevel7, fTrue)
	ibidToggleBmp(284,1,16,dyIconBar-4,IDOIBLVL8,bcmShowToLevel8, fTrue)
	ibidToggleBmp(300,1,16,dyIconBar-4,IDOIBLVL9,bcmShowToLevel9, fTrue)
	ibidToggleText(316,1,26,dyIconBar-4,SzSharedKey("All",All),IDOIBALL,bcmExpandAll)
ibdEnd
#endif /* OUTLINEIB */

#define IDLKSDIALOG         0
#define IDLKSBOLD           1
#define IDLKSITALIC         2
#define IDLKSSMALLCAPS      3
#define IDLKSUNDERLINE      4
#define IDLKSWORDULINE      5
#define IDLKSDOUBLEULINE    6
#define IDLKSSUPERSCRIPT    7
#define IDLKSSUBSCRIPT      8
#define IDLKSSHOWALL        9
#define IDLKSFONT           10
#define IDLKSPOINT          11

#ifdef RIBBON
#include "ribbon3.hs"
#include "ribbon3.sdm"
IbProcRibbon();
ibdHeader(ibdRibbon3, dyRibbon, fTrue, ibcRibbon, fTrue, fTrue, IbProcRibbon)
	ibidDialog(0,0,cabiCABRIBBON3,sizeof(dltRibbon3),IDLKSDIALOG)
	ibidDlgItem(tmcFont,IDLKSFONT)
	ibidDlgItem(tmcPoints,IDLKSPOINT)
	ibidBitmap(idrbRibbon3, 0)
	ibidToggleBmp(257,1,16,11,IDLKSBOLD, bcmNil, fTrue)
	ibidToggleBmp(273,1,16,11,IDLKSITALIC, bcmNil, fTrue)
	ibidToggleBmp(289,1,16,11,IDLKSSMALLCAPS, bcmNil, fTrue)
	ibidBitmap(idrbRibbon3, 3)
	ibidToggleBmp(314,1,16,11,IDLKSUNDERLINE, bcmNil, fTrue)
	ibidToggleBmp(330,1,16,11,IDLKSWORDULINE, bcmNil, fTrue)
	ibidToggleBmp(346,1,16,11,IDLKSDOUBLEULINE, bcmNil, fTrue)
	ibidToggleBmp(383,1,16,11,IDLKSSHOWALL, bcmShowAll, fTrue)
	ibidBitmap(idrbScrptPos3, 0)
	ibidToggleBmp2(362,1,16,6,IDLKSSUPERSCRIPT, bcmNil, fTrue, fTrue, fFalse)
	ibidToggleBmp2(362,6,16,6,IDLKSSUBSCRIPT, bcmNil, fTrue, fTrue, fTrue)
ibdEnd


#endif /* RIBBON */


#define idRulDialog     0 
#define idRulParaLeft   1     /* order here to paraopen important */
#define idRulParaCenter 2     /*  LCD struct in cmd.c depends on it */
#define idRulParaRight  3    /*  also paraleft must be one !!     */
#define idRulParaBoth   4
#define idRulSpace1     5
#define idRulSpace15    6
#define idRulSpace2     7
#define idRulParaClose  8
#define idRulParaOpen   9
#define idRulTabLeft    10
#define idRulTabCenter  11
#define idRulTabRight   12
#define idRulTabDecimal 13
#define idRulStyle      14
#define idRulMode       15
#define idRulMark       16

/* The number of various types of buttons: used by CBT, should be updated
	if any buttons are added to the list above. */
#define cbtnRulerAlign    4
#define cbtnRulerSpacing  5
#define cbtnRulerTabs     (4+1)  /* real value + 1 to step over idRulStyle */

#ifdef RULERIB
#include "ruler3.hs"
#include "ruler3.sdm"
IbProcRuler();
ibdHeader(ibdRuler3, dyRuler3, fTrue, ibcRuler, fFalse, fTrue, IbProcRuler)
	ibidDialog(0,0,cabiCABRULER3,sizeof(dltRuler3),idRulDialog)
	ibidDlgItem(tmcStyle,idRulStyle)
	ibidBitmap(idrbRulerAlign3, 0)
	ibidToggleBmp(143,1,16,11,idRulParaLeft,bcmNil, fTrue)
	ibidToggleBmp(159,1,16,11,idRulParaCenter,bcmNil, fTrue)
	ibidToggleBmp(175,1,16,11,idRulParaRight,bcmNil, fTrue)
	ibidToggleBmp(191,1,16,11,idRulParaBoth,bcmNil, fTrue)
	ibidBitmap(idrbRulerToggles3, 0)
	ibidToggleBmp(216,1,16,11,idRulSpace1,bcmNil, fTrue)
	ibidToggleBmp(232,1,16,11,idRulSpace15,bcmNil, fTrue)
	ibidToggleBmp(248,1,16,11,idRulSpace2,bcmNil, fTrue)
	ibidToggleBmp(273,1,16,11,idRulParaClose,bcmNil, fTrue)
	ibidToggleBmp(289,1,16,11,idRulParaOpen,bcmNil, fTrue)
	ibidToggleBmp(314,1,16,11,idRulTabLeft,bcmNil, fTrue)
	ibidToggleBmp(330,1,16,11,idRulTabCenter,bcmNil, fTrue)
	ibidToggleBmp(346,1,16,11,idRulTabRight,bcmNil, fTrue)
	ibidToggleBmp(362,1,16,11,idRulTabDecimal,bcmNil, fTrue)
	ibidBitmap(idrbRulerAlign3, 4)
	ibidToggleBmp(383,1,16,11,idRulMode,bcmRulerMode, fFalse)
	ibidCustomWnd(1,14,10,4,"",idRulMark)
ibdEnd
#endif /* RULERIB */


#define iibbPrvwPrint     0
#define iibbPrvwBound     1
#define iibbPrvwPages     2
#define iibbPrvwPgvw      3
#define iibbPrvwClose     4
#define iibbPrvwStats     5

#ifdef PREVIEWIB
ibdHeader(ibdPrvw3, dyRibbon, fFalse, ibcPreview, fTrue, fFalse, NULL)
	ibidToggleText(5,1,53,dyRibbon-4,SzSharedKey("&Print...",Print),iibbPrvwPrint,bcmPrint)
	ibidToggleText(65,1,60,dyRibbon-4,SzSharedKey("&Boundaries",Boundaries),iibbPrvwBound,bcmPrvwBound)
	ibidToggleText(132,1,53,dyRibbon-4,"",iibbPrvwPages,bcmPrvwPages)
	ibidToggleText(192,1,53,dyRibbon-4,SzSharedKey("Page &View",PageView),iibbPrvwPgvw,bcmPageView)
	ibidToggleText(252,1,53,dyRibbon-4,SzSharedKey("&Cancel",Cancel),iibbPrvwClose,bcmPrintPreview)
	ibidTextIibb(312,3,80,dyRibbon-4, "", iibbPrvwStats)
ibdEnd
#endif /* PREVIEWIB */


#else

/******
Old iconbar defines 
******/

#ifdef HDRIB
ibdHeader(ibdHdr, dyIconBar, fTrue, ibcHdr, fFalse, NULL)
	ibidBitmap(idrbHdrIconBar, 0)
	ibidToggleBmp(5,1,10,dyIconBar-4,iibbHdrPage, bcmHdrPage)
	ibidToggleBmp(20,1,10,dyIconBar-4,iibbHdrDate, bcmHdrDate)
	ibidToggleBmp(35,1,10,dyIconBar-4,iibbHdrTime, bcmHdrTime)
	ibidTextIibb(50,2,96,dyIconBar-5,"",iibbHdrIBarDesp)
	ibidToggleText(162,1,72,dyIconBar-4,"",iibbHdrLinkPrev,
			bcmHdrLinkPrev)
	ibidToggleText(237,1,30,dyIconBar-4,SzSharedKey("&Close",MenuClose),
			iibbHdrRetToDoc, bcmHdrRetToDoc)
ibdEnd
#endif /* HDRFTRIB */


#define IDMCRGO         0
#define IDMCRTRACE      1
#define IDMCRSTEP       2
#define IDMCRANI        3
#define IDMCRVARS       4
#define IDMCRNAME       5

#ifdef MCRIB
#define IDMCRNAME   5
extern IbProcMacroEdit();
ibdHeader(ibdMcr, dyRibbon, fTrue, ibcMcr, fTrue, IbProcMacroEdit)
	ibidToggleText(5,1,40,dyRibbon-4,SzSharedKey("&Start",Start),IDMCRGO, bcmContinueMacro)
	ibidToggleText(50,1,40,dyRibbon-4,SzSharedKey("St&ep",Step),IDMCRTRACE, bcmTraceMacro)
	ibidToggleText(95,1,40,dyRibbon-4,SzSharedKey("Step S&UBs",StepSUBs),IDMCRSTEP, bcmStepMacro)
	ibidToggleText(140,1,40,dyRibbon-4,SzSharedKey("T&race",Trace),IDMCRANI,bcmAnimateMacro)
	ibidToggleText(185,1,40,dyRibbon-4,SzSharedKey("&Vars...",Vars),IDMCRVARS,bcmShowVars)
	ibidTextIibb(230,3,100,dyRibbon-4,"",IDMCRNAME)
ibdEnd


#endif /* MCRIB */


#define IDOIBPROMOTE      0
#define IDOIBDEMOTE       1
#define IDOIBMOVEUP       2
#define IDOIBMOVEDOWN     3
#define IDOIBBODY         4
#define IDOIBLVLPLUS1     5
#define IDOIBLVLMINUS1    6
#define IDOIBLVL1         7
#define IDOIBLVL2         8
#define IDOIBLVL3         9
#define IDOIBLVL4        10
#define IDOIBLVL5        11
#define IDOIBLVL6        12
#define IDOIBLVL7        13
#define IDOIBLVL8        14
#define IDOIBLVL9        15
#define IDOIBALL         16

/* The number of various types of buttons: used by CBT, should be updated
	if any buttons are added to the list above. */
#define cbtnOutlinePromote    5
#define cbtnOutlinePlus       2

#ifdef OUTLINEIB
ibdHeader(ibdOutline, dyIconBar, fTrue, ibcOutline, fFalse, NULL)
	ibidBitmap(idrbOutlineIcnBr,0)
	ibidToggleBmp(5,1,10,dyIconBar-4,IDOIBPROMOTE,bcmPromote)
	ibidToggleBmp(17,1,10,dyIconBar-4,IDOIBDEMOTE,bcmDemote)
	ibidToggleBmp(29,1,10,dyIconBar-4,IDOIBMOVEUP,bcmMoveUp)
	ibidToggleBmp(41,1,10,dyIconBar-4,IDOIBMOVEDOWN,bcmMoveDown)
	ibidToggleBmp(53,1,10,dyIconBar-4,IDOIBBODY,bcmConvertToBody)
	ibidToggleBmp(77,1,10,dyIconBar-4,IDOIBLVLPLUS1,bcmExpand)
	ibidToggleBmp(89,1,10,dyIconBar-4,IDOIBLVLMINUS1,bcmCollapse)
	ibidText(111,2,20,dyIconBar-4, SzSharedKey("Show:",Show))
	ibidToggleBmp(135,1,10,dyIconBar-4,IDOIBLVL1,bcmShowToLevel1)
	ibidToggleBmp(145,1,10,dyIconBar-4,IDOIBLVL2,bcmShowToLevel2)
	ibidToggleBmp(155,1,10,dyIconBar-4,IDOIBLVL3,bcmShowToLevel3)
	ibidToggleBmp(165,1,10,dyIconBar-4,IDOIBLVL4,bcmShowToLevel4)
	ibidToggleBmp(175,1,10,dyIconBar-4,IDOIBLVL5,bcmShowToLevel5)
	ibidToggleBmp(185,1,10,dyIconBar-4,IDOIBLVL6,bcmShowToLevel6)
	ibidToggleBmp(195,1,10,dyIconBar-4,IDOIBLVL7,bcmShowToLevel7)
	ibidToggleBmp(205,1,10,dyIconBar-4,IDOIBLVL8,bcmShowToLevel8)
	ibidToggleBmp(215,1,10,dyIconBar-4,IDOIBLVL9,bcmShowToLevel9)
	ibidToggleText(225,1,20,dyIconBar-4,SzSharedKey("All",All),IDOIBALL,bcmExpandAll)
ibdEnd
#endif /* OUTLINEIB */

#define IDLKSDIALOG         0
#define IDLKSBOLD           1
#define IDLKSITALIC         2
#define IDLKSSMALLCAPS      3
#define IDLKSUNDERLINE      4
#define IDLKSWORDULINE      5
#define IDLKSDOUBLEULINE    6
#define IDLKSSUPERSCRIPT    7
#define IDLKSSUBSCRIPT      8
#define IDLKSSHOWALL        9
#define IDLKSFONT           10
#define IDLKSPOINT          11

#ifdef RIBBON
#include "ribbon.hs"
#include "ribbon.sdm"
IbProcRibbon();
ibdHeader(ibdRibbon, dyRibbon, fTrue, ibcRibbon, fTrue, IbProcRibbon)
	ibidDialog(0,0,cabiCABRIBBON,sizeof(dltRibbon),IDLKSDIALOG)
	ibidDlgItem(tmcFont,IDLKSFONT)
	ibidDlgItem(tmcPoints,IDLKSPOINT)
	ibidBitmap(idrbRibbon, 0)
	ibidToggleBmp(175,1,10,dyRibbon-ddyIconLH,IDLKSBOLD, bcmNil)
	ibidToggleBmp(186,1,10,dyRibbon-ddyIconLH,IDLKSITALIC, bcmNil)
	ibidToggleBmp(197,1,10,dyRibbon-ddyIconLH,IDLKSSMALLCAPS, bcmNil)
	ibidBitmap(idrbRibbon, 3)
	ibidToggleBmp(211,1,10,dyRibbon-ddyIconLH,IDLKSUNDERLINE, bcmNil)
	ibidToggleBmp(222,1,10,dyRibbon-ddyIconLH,IDLKSWORDULINE, bcmNil)
	ibidToggleBmp(233,1,10,dyRibbon-ddyIconLH,IDLKSDOUBLEULINE, bcmNil)
	ibidToggleBmp(281,1,10,dyRibbon-ddyIconLH,IDLKSSHOWALL, bcmShowAll)
	ibidBitmap(idrbScrptPos, 0)
	ibidToggleBmp(247,1,8,6,IDLKSSUPERSCRIPT, bcmNil)
	ibidToggleBmp(247,6,8,6,IDLKSSUBSCRIPT, bcmNil)
ibdEnd


#endif /* RIBBON */


#define idRulDialog     0 
#define idRulParaLeft   1     /* order here to paraopen important */
#define idRulParaCenter 2     /*  LCD struct in cmd.c depends on it */
#define idRulParaRight  3    /*  also paraleft must be one !!     */
#define idRulParaBoth   4
#define idRulSpace1     5
#define idRulSpace15    6
#define idRulSpace2     7
#define idRulParaClose  8
#define idRulParaOpen   9
#define idRulTabLeft    10
#define idRulTabCenter  11
#define idRulTabRight   12
#define idRulTabDecimal 13
#define idRulStyle      14
#define idRulMode       15
#define idRulMark       16

/* The number of various types of buttons: used by CBT, should be updated
	if any buttons are added to the list above. */
#define cbtnRulerAlign    4
#define cbtnRulerSpacing  5
#define cbtnRulerTabs     (4+1)  /* real value + 1 to step over idRulStyle */

#ifdef RULERIB
#include "ruler.hs"
#include "ruler.sdm"
IbProcRuler();
ibdHeader(ibdRuler, dyRuler, fTrue, ibcRuler, fFalse, IbProcRuler)
	ibidDialog(0,0,cabiCABRULER,sizeof(dltRuler),idRulDialog)
	ibidDlgItem(tmcStyle,idRulStyle)
	ibidBitmap(idrbRulerAlign, 0)
	ibidToggleBmp(115,1,12,12,idRulParaLeft,bcmNil)
	ibidToggleBmp(128,1,12,12,idRulParaCenter,bcmNil)
	ibidToggleBmp(141,1,12,12,idRulParaRight,bcmNil)
	ibidToggleBmp(154,1,12,12,idRulParaBoth,bcmNil)
	ibidBitmap(idrbRulerToggles, 0)
	ibidToggleBmp(175,1,8,12,idRulSpace1,bcmNil)
	ibidToggleBmp(184,1,8,12,idRulSpace15,bcmNil)
	ibidToggleBmp(193,1,8,12,idRulSpace2,bcmNil)
	ibidToggleBmp(205,1,8,12,idRulParaClose,bcmNil)
	ibidToggleBmp(214,1,8,12,idRulParaOpen,bcmNil)
	ibidToggleBmp(234,1,8,12,idRulTabLeft,bcmNil)
	ibidToggleBmp(243,1,8,12,idRulTabCenter,bcmNil)
	ibidToggleBmp(252,1,8,12,idRulTabRight,bcmNil)
	ibidToggleBmp(261,1,8,12,idRulTabDecimal,bcmNil)
	ibidBitmap(idrbRulerAlign, 4)
	ibidToggleBmp(279,1,12,12,idRulMode,bcmRulerMode)
	ibidCustomWnd(1,14,10,4,"",idRulMark)
ibdEnd
#endif /* RULERIB */


#define iibbPrvwPrint     0
#define iibbPrvwBound     1
#define iibbPrvwPages     2
#define iibbPrvwPgvw      3
#define iibbPrvwClose     4
#define iibbPrvwStats     5

#ifdef PREVIEWIB
ibdHeader(ibdPrvw, dyRibbon, fFalse, ibcPreview, fTrue, NULL)
	ibidToggleText(5,1,40,dyRibbon-4,SzSharedKey("&Print...",Print),iibbPrvwPrint,bcmPrint)
	ibidToggleText(50,1,45,dyRibbon-4,SzSharedKey("&Boundaries",Boundaries),iibbPrvwBound,bcmPrvwBound)
	ibidToggleText(100,1,40,dyRibbon-4,"",iibbPrvwPages,bcmPrvwPages)
	ibidToggleText(145,1,40,dyRibbon-4,SzSharedKey("Page &View",PageView),iibbPrvwPgvw,bcmPageView)
	ibidToggleText(190,1,40,dyRibbon-4,SzSharedKey("&Cancel",Cancel),iibbPrvwClose,bcmPrintPreview)
	ibidTextIibb(235,3,80,dyRibbon-4, "", iibbPrvwStats)
ibdEnd
#endif /* PREVIEWIB */
#endif /* WIN23 */

