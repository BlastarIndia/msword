/* resource.h -- Resource information for OPUS */

/* resouce numbers (index to rgrcds in rcinit.c) */
#define ircdsWordIcon   0
#define ircdsRight      1
#define ircdsSplit      2
#define ircdsColumn     3
#define ircdsNonCoreMin 4
#define ircdsStyWnd     4
#define ircdsRecorder   5
#define ircdsHelp       6
#define ircdsOtlCross   7
#define ircdsOtlVert    8
#define ircdsOtlHorz    9
#define ircdsPrvwCross  10


/* bitmap id's */
/* idrb = resource ID of bitmap */

#define idrbChVis               0       /* visi tab, CRJ */

#define idrbMinDeviceDep        1       /* start numbering of bitmaps
											for which multiple, device-
											dependent versions are available */
#ifdef WIN23
/*** Win 2 set ***/
#define idrbOtlPat2		1	/* outline bullets */
#define idrbScrptPos2            2       /* +-  (ribbon) */
#define idrbRulerToggles2        3       /* ruler tabs & other toggles */
#define idrbRulerMarks2          4       /* ruler indent marks */
#define idrbRibbon2              5       /* Ribbon main bitmaps */
#define idrbHdrIconBar2          6       /* header icon bar */
#define idrbOutlineIcnBr2        7       /* outline iconbar */
#define idrbPageview2            8       /* pageview icons */
#define idrbRulerAlign2          9       /* ruler alignment icons */
#define idrbMax2                 10
/*** Win 3 set ****/
#define idrbOtlPat3					1	/* outline bullets */
#define idrbScrptPos3            2       /* +-  (ribbon) */
#define idrbIScrptPos3           3       /* +-  (ribbon) */
#define idrbRulerToggles3        4       /* ruler tabs & other toggles */
#define idrbIRulerToggles3       5       /* inactive ruler tabs & other toggles */
#define idrbRulerMarks3          6       /* ruler indent marks */
#define idrbRibbon3              7       /* Ribbon main bitmaps */
#define idrbIRibbon3             8        /* Inactive Ribbon main bitmaps */
#define idrbHdrIconBar3          9       /* header icon bar */
#define idrbOutlineIcnBr3        10       /* outline iconbar */
#define idrbPageview3            11       /* pageview icons */
#define idrbRulerAlign3          12       /* ruler alignment icons */
#define idrbIRulerAlign3         13       /* Inactive ruler alignment icons */
#define idrbMax3                 14

#else
#define idrbOtlPat		1	/* outline bullets */
#define idrbScrptPos            2       /* +-  (ribbon) */
#define idrbRulerToggles        3       /* ruler tabs & other toggles */
#define idrbRulerMarks          4       /* ruler indent marks */
#define idrbRibbon              5       /* Ribbon main bitmaps */
#define idrbHdrIconBar          6       /* header icon bar */
#define idrbOutlineIcnBr        7       /* outline iconbar */
#define idrbPageview            8       /* pageview icons */
#define idrbRulerAlign          9       /* ruler alignment icons */
#define idrbMax                 10
#endif /* WIN23 */


#ifdef WIN23
/* Win 2 Device BitMap Groups */
#define dbmg85142    0
#define dbmgSigma2   1
#define dbmgVGA2     2
#define dbmgEGA2     3
#define dbmgCGA2     4
#define dbmgLast2    4  /* last must be smallest */
/* number of bitmaps per device group */
#define cbmdsDbmg3   13

/* Win 3 Device BitMap Groups */
#define dbmg85143    0
#define dbmgVGA3     1
#define dbmgEGA3     2
#define dbmgLast3    2  /* last must be smallest */
/* number of bitmaps per device group */
#define cbmdsDbmg2   9

#else
/* Device BitMap Groups */
#define dbmg8514    0
#define dbmgSigma   1
#define dbmgVGA     2
#define dbmgEGA     3
#define dbmgCGA     4
#define dbmgLast    4  /* last must be smallest */

/* number of bitmaps per device group */
#define cbmdsDbmg   9

#endif /* WIN23 */

/* information about resource bitmaps */
/* idcb = index into multiple-bitmaps-in-one, concatenated horizontally */

#define idcbChVisCRJ            0
#define idcbChVisTab            1
#define idcbMaxChVis            2
#define dxpChVisEach            12
#define dxpChVis                (dxpChVisEach * idcbMaxChVis)
#define dypChVis                7

#define idcbMaxOtlPat		8
#define dxpOtlPat		10
#ifdef WIN23
#define dypOtlPat2		8
#define dypOtlPat3		10
#else
#define dypOtlPat		8
#endif /* WIN23 */

#define idcbOtlMarkPlus		0
#define idcbOtlMarkHiPlus	1
#define idcbOtlMarkMinus	2
#define idcbOtlMarkHiMinus	3
#define idcbOtlMarkBody		4
#define idcbOtlMarkHiBody	5
#define idcbOtlMarkLeftRight	6
#define idcbOtlMarkUpDown	7

#define idcbMaxScrptPos         2
#define idcbMaxRulerToggles     9
#define idcbMaxRulerMarks       12
#define idcbMaxRibbon           7
#define idcbMaxHdrIconBar       3
#define idcbMaxOutlineIcnBr     16
#define idcbMaxPageview		1
#define idcbMaxRulerAlign       5

#define dyIconBar 15

#ifdef WIN23
struct BMDS3
	{
	BITMAPCOREHEADER bch;
	CHAR rgchBits[];
	int cb;
	};
struct BMDS
	{
	BITMAP bm;
	CHAR rgchBits[];
	int cb;
	};
#else
struct BMDS
	{
	BITMAP bm;
	CHAR rgchBits[];
	int cb;
	};
#endif /* WIN23 */


struct RCDS
	{
	CHAR rgchBits[];
	int cb;
	};


/* GetProcAddress ordinal values (taken from appropriate .DEF files) */

/* for KEYBOARD */
#define idoVkKeyScan		129
#define idoSetSpeed			7

/* for KERNEL */
#define idoAllocDsToCsAlias	171
#define idoGetFreeSpace		169
#define idoAllocSelector	175
#define idoFreeSelector		176
#define idoPrestoChangoSelector 177

/* for printer drivers */
#define idoExtTextOut		14
#define idoDeviceMode		13

/* for USER */
#define idoExitWindowsV3    7

/* for GDI */
#define idoSetDIBits	440
#define idoGetDIBits	441
#define idoCreateDIBitmap	442
