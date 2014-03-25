/* outline icn's */
#define icnLeft         WinMac(IDOIBPROMOTE,	0)
#define icnRight        WinMac(IDOIBDEMOTE,	1)
#define icnUp           WinMac(IDOIBMOVEUP,	2)
#define icnDown         WinMac(IDOIBMOVEDOWN,	3)
#define icnBody         WinMac(IDOIBBODY,	4)
#define icnPlus         WinMac(IDOIBLVLPLUS1,	5)
#define icnMinus        WinMac(IDOIBLVLMINUS1,	6)
#define icn1            WinMac(IDOIBLVL1,	7)
#define icn2            WinMac(IDOIBLVL2,	8)
#define icn3            WinMac(IDOIBLVL3,	9)
#define icn4            WinMac(IDOIBLVL4,	10)
#ifdef WIN
#define icn5            WinMac(IDOIBLVL5,	11)
#define icn6            WinMac(IDOIBLVL6,	12)
#define icn7            WinMac(IDOIBLVL7,	13)
#define icn8            WinMac(IDOIBLVL8,	14)
#define icn9            WinMac(IDOIBLVL9,	15)
#endif
#define icnAll		WinMac(IDOIBALL,	11) 
#ifdef MAC
#define icnShowBody		12
#define icnShowFmt		13
#endif

#define icnLast		 WinMac(icnAll, icnShowFmt)
#define icnBodyLeftRight (icnLast + 1)
#define icnPass2         (icnLast + 2)
#define icnMinusDirect   (icnLast + 3)
#define icnEllip	 (icnLast + 4)

/* means pad entry created by LbcFormatPara, paph should not be disturbed but
	all else is dirty */
#define lvlUpdate       15

/* outline marker types */

#define mktOff  0
#define mktLeftRight 1
#define mktUpDown 2
#define mktBody 3
#define mktHighlight 4


/* structure to cache effective lvl's for body text.
*/
struct CAP
	{
	int     doc;
	int     ipad;
	int     lvl;
	};

#ifdef MAC
#define LvlFromUcm(ucm)		(ucm - ucmOtl1)
#endif

#ifdef WIN
/* used only in wwd.lvl to indicate none of the toggles in the outline
	icon bar is hilited. */
#define lvlNone         15
#define lvlAll		(icnAll - icn1)

/* Only used in cmd.c */
typedef struct {
			int kc;
			int bcm;
			} KB;

#define LvlFromUcm(ucm)		(icnOtl - icn1)
#endif /* WIN */

