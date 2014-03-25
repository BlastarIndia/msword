/* This file defines the keyboard interface used by Opus. */


/* Key Types (used in keymap structure) */
#define ktNil           -1
#define ktKeyMacro      0       /* keyboard macro (not used) */
#define ktIgnore        1       /* just ignore the key */
#define ktBeep          2       /* beep and ignore the key */
#define ktInsert        3       /* kc is meant for insert loop */

#define ktMacro         6       /* Opel macro */
#define ktFunc          7       /* CS function */


/* kcModal is a special key code used to simulate a keypress ending a mode */
#define kcModal 0x0fff


/* Keyboard Character Codes */

/*

A Keyboard Character Code (KC) is a 16-bit code that describes a key
and its Shift, Control, and Alt states.

*/

#define kcNil                   0xFFFF   /* No key, ignore it */

#define kcMinVisi 0x20
#define kcMaxVisi 0x7f
#define FCmdKc(kc) ((uns)((kc) - kcMinVisi) >= kcMaxVisi - kcMinVisi)


/* Local names for Windows key codes */

#define kcBackSpace     (VK_BACK)
#define kcDelNext       (VK_DELETE)
#define kcTab           (VK_TAB)
#define kcReturn        (VK_RETURN)
#define kcEscape        (VK_ESCAPE)

#define kcSpace         (VK_SPACE)
#define kcInsert        (VK_INSERT)
#define kcDelete        (VK_DELETE)
#define kcHelp          (VK_HELP)
#define kcF1            (VK_F1)
#define kcF16           (VK_F16)


#define kcPageBreak             KcCtrl(kcReturn)

#define kcColumnBreak           KcCtrl(KcShift(kcReturn))
#define kcNonReqHyphen          KcCtrl(kcMinus) /* BEWARE: non-standard VK */
#define kcNonBreakHyphen        KcShift(kcNonReqHyphen)
#define kcNonBreakSpace         KcCtrl(KcShift(' '))
#define kcNewLine               KcShift(kcReturn)

/* Cursor movement keys */

#define kcUp            (VK_UP)
#define kcDown          (VK_DOWN)
#define kcLeft          (VK_LEFT)
#define kcRight         (VK_RIGHT)
#define kcBeginLine     (VK_HOME)
#define kcEndLine       (VK_END)
#define kcPageUp        (VK_PRIOR)
#define kcPageDown      (VK_NEXT)
#define kcClear         (VK_CLEAR)

#define kcPrevPara	KcCtrl(kcUp)
#define kcNextPara	KcCtrl(kcDown)
#define kcWordLeft	KcCtrl(kcLeft)
#define kcWordRight	KcCtrl(kcRight)
#define kcWordLeftAlt	KcAlt(kcLeft)
#define kcWordRightAlt	KcAlt(kcRight)
#define kcTopScreen	KcCtrl(kcPageUp)
#define kcEndScreen	KcCtrl(kcPageDown)
#define kcTopDoc	KcCtrl(kcBeginLine)
#define kcEndDoc	KcCtrl(kcEndLine)


#define kcNPPlus	(VK_ADD)
#define kcNPMult	(VK_MULTIPLY)
#define kcNPMinus	(VK_SUBTRACT)
#define kcNPDiv		(VK_DIVIDE)

#ifdef DEBUG
	/* used for dumping windows to clipboard */
#define kcPrintScr     KcCtrl(VK_MULTIPLY)
	/* dumps user state to comm and title bar */
#define kcSysState     KcCtrl(KcShift(VK_ADD))
	/* does a stack trace */
#define kcStackTrace   KcCtrl(KcShift(VK_SUBTRACT))
	/* causes immediate exit from windows */
#define kcExitWin1     KcCtrl(KcShift(VK_F12))
#define kcExitWin2     KcAlt(KcCtrl(KcShift(VK_F2)))
#endif   /* DEBUG */


/* These are the special keys that we use that do not have definate VK codes.
	The vkFoo values are set up during initialization by calling the keyboard
	driver VkKeyScan() function. */
extern int vkPlus, vkMinus, vkStar;
#define kcPlus		(vkPlus)
#define kcMinus		(vkMinus)
#define kcStar		(vkStar)

#ifdef INTL
extern int vkUnderline, vkEquals, vkQuestionmark;
#define kcUnderline	(vkUnderline)
#define kcEquals	(vkEquals)
#define kcQuestionmark	(vkQuestionmark)

#define vkPlusDef	KcShift(0xbb)
#define vkMinusDef	0xbd
#define vkStarDef	KcShift('8')
#define vkUnderlineDef	KcShift(0xbd)
#define vkEqualsDef	0xbb
#define vkQuestionmarkDef KcShift('/')
#endif /* INTL */

#define kcSubscript	KcCtrl(kcPlus)
#define kcSuperscript	KcShift(KcCtrl(kcPlus))
#define kcShowAll	KcCtrl(KcShift(kcStar))

/* Macros to set key code modifier bits */

#define KcCtrl(kc)      ((kc) | 0x100)
#define KcShift(kc)     ((kc) | 0x200)
#define KcAlt(kc)       ((kc) | 0x400)


/* Test key code modifier bits */

#define FCtrlKc(kc)     ((kc) & 0x100)
#define FShiftKc(kc)    ((kc) & 0x200)
#define FAltKc(kc)      ((kc) & 0x400)


/* Remove key code modifier bits */

#define KcStrip(kc)     ((kc) & 0xff)


/* Key Board State Masks */

#define wKbsShiftMask   0x0200
#define wKbsOptionMask  0x0400
#define wKbsControlMask 0x0100

#define wKbsCapsLckMask 0x0800
#define wKbsNumLckMask  0x1000
#define wKbsExtendMask  0x2000

#define wKbsOptionCLMask (wKbsOptionMask | wKbsCapsLckMask)
#define wKbsShiftOptionCLMask (wKbsShiftMask | wKbsOptionCLMask)


/* Fake variables describing the state of modifier keys */

extern int vgrpfKeyBoardState;
#define vfShiftKey (vgrpfKeyBoardState & wKbsShiftMask)
#define vfOptionKey (vgrpfKeyBoardState & wKbsOptionMask)
#define vfControlKey (vgrpfKeyBoardState & wKbsControlMask)


/* Only used in the following kc defs'. */
#define kcPeriod        0xBE
#define kcComma         0xBC
#define kcSlash         0xBF

#define StripCtrl(_kc) (0x00FF & (_kc))

/* If you change the following key assignment, you must change
	tables in outline.c and cmd.c */
#define kcExpand1      (VK_ADD)
#define kcExpand2      (KcAlt(KcShift(kcPlus)))
#define kcExpand3      (KcAlt(KcShift(VK_ADD)))
#define kcCollapse1    (VK_SUBTRACT)
#define kcCollapse2    (KcAlt(KcShift(kcMinus)))
#define kcCollapse3    (KcAlt(KcShift(VK_SUBTRACT)))

#define kcLevel1       (KcAlt(KcShift('1')))
#define kcLevel2       (KcAlt(KcShift('2')))
#define kcLevel3       (KcAlt(KcShift('3')))
#define kcLevel4       (KcAlt(KcShift('4')))
#define kcLevel5       (KcAlt(KcShift('5')))
#define kcLevel6       (KcAlt(KcShift('6')))
#define kcLevel7       (KcAlt(KcShift('7')))
#define kcLevel8       (KcAlt(KcShift('8')))
#define kcLevel9       (KcAlt(KcShift('9')))

#define kcExpandAll1   (VK_MULTIPLY)
#define kcExpandAll2   (KcAlt(KcShift('A')))
#define kcMoveUp       (KcAlt(KcShift(kcUp)))
#define kcMoveDown     (KcAlt(KcShift(kcDown)))
#define kcConvToBody   (KcAlt(KcShift(VK_NUMPAD5)))

#define kcToggleEllip  (KcAlt(KcShift('F')))


/* for the Macro Edit Icon Bar in edmacro.c */

#define kcTraceMacro		KcShift(KcAlt('E'))
#define kcAnimateMacro		KcShift(KcAlt('R'))
#define kcContinueMacro 	KcShift(KcAlt('S'))
#define kcContinueMacro2	KcShift(KcAlt('O'))
#define kcStepMacro		KcShift(KcAlt('U'))
#define kcShowVars		KcShift(KcAlt('V'))


#ifdef PREVIEWC /* from preview.c */

#define rgkmePrvwDef	\
	{ kcTab,	ktFunc,   PrvwTab },			\
	{ kcReturn,	ktFunc,   PrvwReturn }, 		\
	{ kcEscape,	ktMacro,  bcmPrintPreview },		\
	{ kcPageUp,	ktFunc,   PrvwPageUp }, 		\
	{ kcPageDown,	ktFunc,   PrvwPageDown },		\
	{ 'A',		ktMacro,  bcmPrvwPages },		\
	{ 'B',		ktMacro,  bcmPrvwBound },		\
	{ 'C',		ktMacro,  bcmPrintPreview },		\
	{ 'P',		ktMacro,  bcmPrint },			\
	{ 'V',		ktMacro,  bcmPageView },		\
	{ VK_F1,	ktFunc,   PrvwF1 },			\
	{ VK_F10,	ktMacro,  bcmMenuMode },		\
	{ KcShift(kcTab),      ktFunc,	 PrvwTab },		\
	{ KcShift(VK_F1),      ktFunc,	 PrvwShiftF1 }, 	\
	{ KcAlt(KcShift('A')), ktMacro, bcmPrvwPages }, 	\
	{ KcAlt(KcShift('B')), ktMacro, bcmPrvwBound }, 	\
	{ KcAlt(KcShift('C')), ktMacro, bcmPrintPreview },	\
	{ KcAlt(KcShift('P')), ktMacro, bcmPrint },		\
	{ KcAlt(KcShift('V')), ktMacro, bcmPageView },

#endif /* PREVIEWC */


#ifdef ICONBAR3C /* from iconbar3.c */

#define rgKmeOutlineDef \
	{ kcExpandAll1,  ktMacro, bcmExpandAll },	\
	{ kcExpand1,	 ktMacro, bcmExpand },		\
	{ kcCollapse1,	 ktMacro, bcmCollapse },	\
	{ kcLevel1,	 ktMacro, bcmShowToLevel1 },	\
	{ kcLevel2,	 ktMacro, bcmShowToLevel2 },	\
	{ kcLevel3,	 ktMacro, bcmShowToLevel3 },	\
	{ kcLevel4,	 ktMacro, bcmShowToLevel4 },	\
	{ kcLevel5,	 ktMacro, bcmShowToLevel5 },	\
	{ kcLevel6,	 ktMacro, bcmShowToLevel6 },	\
	{ kcLevel7,	 ktMacro, bcmShowToLevel7 },	\
	{ kcLevel8,	 ktMacro, bcmShowToLevel8 },	\
	{ kcLevel9,	 ktMacro, bcmShowToLevel9 },	\
	{ (KcAlt(KcShift('A'))),	ktMacro, bcmExpandAll },	\
	{ (KcAlt(KcShift('F'))),	ktMacro, bcmToggleEllip },	\
	{ kcExpand3,	 ktMacro, bcmExpand },		\
	{ kcCollapse3,	 ktMacro, bcmCollapse },


#define rgKmeOutlineIBDef \
	{ kcTab,		    ktFunc, IBTab },		\
	{ kcReturn,		    ktFunc, IBReturn }, 	\
	{ kcEscape,		    ktFunc, IBCancel }, 	\
	{ kcSpace,		    ktFunc, IBReturn	},	\
	{ kcLeft,		    ktFunc, IBMoveLeft },	\
	{ StripCtrl(kcMoveUp),	    ktFunc, IBMoveUp }, 	\
	{ kcRight,		    ktFunc, IBMoveRight },	\
	{ StripCtrl(kcMoveDown),    ktFunc, IBMoveDown },	\
	{ StripCtrl(kcLevel1),	    ktFunc, IBLevel1 }, 	\
	{ StripCtrl(kcLevel2),	    ktFunc, IBLevel2 }, 	\
	{ StripCtrl(kcLevel3),	    ktFunc, IBLevel3 }, 	\
	{ StripCtrl(kcLevel4),	    ktFunc, IBLevel4 }, 	\
	{ StripCtrl(kcLevel5),	    ktFunc, IBLevel5 }, 	\
	{ StripCtrl(kcLevel6),	    ktFunc, IBLevel6 }, 	\
	{ StripCtrl(kcLevel7),	    ktFunc, IBLevel7 }, 	\
	{ StripCtrl(kcLevel8),	    ktFunc, IBLevel8 }, 	\
	{ StripCtrl(kcLevel9),	    ktFunc, IBLevel9 }, 	\
	{ 'A',			    ktFunc, IBExpandAll },	\
	{ StripCtrl(kcConvToBody),  ktFunc, IBConvertToBody },	\
	{ StripCtrl(kcExpandAll1),  ktFunc, IBExpandAll },	\
	{ StripCtrl(kcExpand1),     ktFunc, IBExpand }, 	\
	{ StripCtrl(kcCollapse1),   ktFunc, IBCollapse },	\
	{ VK_F1,		    ktFunc, IBGetHelpOutline },


#define rgKmeHdrIBDef \
	{ kcTab,	ktFunc, IBTab },		\
	{ kcReturn,	ktFunc, IBReturn    },		\
	{ kcEscape,	ktFunc, IBCancel    },		\
	{ kcSpace,	ktFunc, IBReturn    },		\
	{ kcLeft,	ktFunc, IBMoveLeft  },		\
	{ kcUp, 	ktFunc, IBMoveLeft  },		\
	{ kcRight,	ktFunc, IBMoveRight },		\
	{ kcDown,	ktFunc, IBMoveRight },		\
	{ 'C',		ktFunc, IBHdrRetToDoc },	\
	{ 'D',		ktFunc, IBHdrDate },		\
	{ 'L',		ktFunc, IBHdrLinkPrev },	\
	{ 'P',		ktFunc, IBHdrPage },		\
	{ 'R',		ktFunc, IBHdrLinkPrev },	\
	{ 'T',		ktFunc, IBHdrTime },		\
	{ VK_F1,	ktFunc, IBGetHelpHdr },


#define rgKmeDbgIBDef \
	{ kcTab,	ktFunc, IBTab },		\
	{ kcReturn,	ktFunc, IBReturn },		\
	{ kcEscape,	ktFunc, IBCancel },		\
	{ kcSpace,	ktFunc, IBReturn    },		\
	{ kcLeft,	ktFunc, IBMoveLeft },		\
	{ kcUp, 	ktFunc, IBMoveLeft },		\
	{ kcRight,	ktFunc, IBMoveRight },		\
	{ kcDown,	ktFunc, IBMoveRight },		\
	{ 'E',		ktFunc, IBTraceMacro }, 	\
	{ 'R',		ktFunc, IBAnimateMacro },	\
	{ 'S',		ktFunc, IBContinueMacro },	\
	{ 'U',		ktFunc, IBStepMacro },		\
	{ 'V',		ktFunc, IBShowVars },		\
	{ VK_F1,	ktFunc, IBGetHelpMacro },


#define rgKmePreviewIBDef \
	{ kcTab,	ktFunc, IBTab	    },		\
	{ kcReturn,	ktFunc, IBReturn    },		\
	{ kcEscape,	ktFunc, IBCancel    },		\
	{ kcSpace,	ktFunc, IBReturn    },		\
	{ kcLeft,	ktFunc, IBMoveLeft  },		\
	{ kcUp, 	ktFunc, IBMoveLeft  },		\
	{ kcRight,	ktFunc, IBMoveRight },		\
	{ kcDown,	ktFunc, IBMoveRight },		\
	{ 'A',		ktFunc, IBPrvwPages },		\
	{ 'B',		ktFunc, IBPrvwBound },		\
	{ 'C',		ktFunc, IBPrvwClose },		\
	{ 'P',		ktFunc, IBPrvwPrint },		\
	{ 'V',		ktFunc, IBPageView  },		\
	{ VK_F1,	ktFunc, IBGetHelpPreview },


#endif /* ICONBAR3C */


#ifdef RULRIBC /* from rulrib.c */

#define rgKmeRulerDef \
	{ kcTab,	    ktFunc, RETab	},	\
	{ kcReturn,	    ktFunc, REReturn	},	\
	{ kcEscape,	    ktFunc, REEscape	},	\
	{ VK_END,	    ktFunc, REEnd	},	\
	{ VK_HOME,	    ktFunc, REHome	},	\
	{ kcLeft,	    ktFunc, RELeft	},	\
	{ kcRight,	    ktFunc, RERight	},	\
	{ kcInsert,	    ktFunc, REInsert	},	\
	{ kcDelete,	    ktFunc, REDelete	},	\
	{ '1',		    ktFunc, RETabLeft	},	\
	{ '2',		    ktFunc, RETabCenter },	\
	{ '3',		    ktFunc, RETabRight	},	\
	{ '4',		    ktFunc, RETabDecimal },	\
	{ 'F',		    ktFunc, REIndLeft1	},	\
	{ 'L',		    ktFunc, REIndLeft	},	\
	{ 'R',		    ktFunc, REIndRight	},	\
	{ VK_F1,	    ktFunc, REGetHelp	},


#endif /* RULRIBC */
