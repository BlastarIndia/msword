/* menu2.h */

#define imnuFile        0
#define imnuEdit        1
#define imnuView        2
#define imnuInsert      3
#define imnuReformat    4
#define imnuProof       5
#define imnuUtilities   6
#define imnuWindow      7
#define imnuDebug       8

#define imnuDocControl  15


/* Menu iTeM */
/* Special case: (fRemove == fTrue && ibst != 0) renamed std item */
typedef struct _mtm
		{
		int imnu : 4;
		int ibst : 9;
	int fSpare : 1;
	int fUndo : 1;
		int fRemove : 1;
		union
				{
				uns bsy;
				BCM bcm;
				};
		} MTM;

#define cbMTM (sizeof (MTM))
#define cwMTM (sizeof (MTM) / sizeof (int))

#define bsySeparator 0xfffe


/* MenU Delta */
typedef struct _mud
		{
		int imtmMac;
		int imtmMax;
		MTM rgmtm[0];
		} MUD;

#define cbMUD sizeof (MUD)
#define cwMUD (sizeof (MUD) / sizeof (int))

#define cmtmAlloc 4


/* Used in conjunction with BuildMenuSz() */
#define grfBldMenuNoAcc         0x0001
#define grfBldMenuNoElip        0x0002
#define grfBldMenuDef           0x0000  /* With accelerator and ellipses. */


#define chMenuAcc		'&'
#define cchMaxSzMenu    	64
#define bcmFirstMenu    	bcmFileNew
#define bcmLastMenu     	bcmArrangeWnd

#define ibstMaxWndCache         9

#define bcmSeparator		0xfffe
#define bcmFileCache    	0xfffd
#define bcmWndCache     	0xfffc

#define bsySepMax		0xfffb
#define bsySepFirst		0x8000

#define iMenuLongFull            -1
#define iMenuLongMin              0
#define iMenuShortMin             1
#define iMenuShortFull            2

#define FreeHmud(hmud)	FreeH(hmud)
