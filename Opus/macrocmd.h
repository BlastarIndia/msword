/* macrocmd.h */

#define cchMaxMacroName 32


/* VmnValidMacroNameSt() return values */
#define vmnBad  0       /* Name won't do */
#define vmnValid 1      /* Name is fine, but is undefined */
#define vmnExists -1    /* Name if fine, and is defined */


/* Macro Editor Line */
typedef struct _mel
	{
	uns cp;
	uns ib;
	} MEL;

#define cwMEL	(CwFromCch(sizeof (MEL)))

#define cmelQuanta	10


/* Macro Editor Instance */
typedef struct _mei
		{
		BCM bcm;                /* This macro's BCM */
		ELG elg;                /* This macro's ELG (docDot) */
		ELM elm;                /* This macro's ELM (imcr) */
		char ** pstName;        /* This macro's name */
		int docEdit;            /* This instance's editing doc */
		int mw;                 /* This instance's editing window */
		int fNew : 1;           /* This is a new macro */
	int fRunning : 1;	/* This macro is currently running */
	int fHlt : 1;		/* Macro contains hilighting */
	int fDirty : 1;		/* Saved macro editor dirty state */
	int fSuspended : 1;	/* This macro is in a suspended state */
	int fRestartable : 1;	/* May be restarted when interrupted */
	int fNotExpanded : 1;  	/* The hqrgb not yet expanded into docDest */
	int fSpare : 9;
		ELI ** heli;            /* This macro's EL instance */
	CP cpHlt;		/* Para containing hilighting */
	uns cbTokens;		/* Size of token buffer */
	HQ hqrgbTokens;		/* This macro's token buffer */
	uns imelMax, imelMac;	/* Allocated/used mels */
	MEL ** hrgmel;		/* Maps ibToken <--> cpText */
		} MEI;

#define cwMEI CwFromCch(sizeof (MEI))


/* Macro Editor State */
typedef struct _mes
		{
		uns cmwOpen : 4;        /* Number of macro editing windows open */
		uns fCanCont : 1;
		uns fAnimate : 1;       /* Current macro is being animated */
	uns fStep : 1;		/* Single stepping */
	uns fStepSUBs : 1;	/* Single stepping, step over SUBs */
		uns : 8;
	KMP ** hkmp;		/* Macro edit mode keys */
		int imeiCur;            /* Current MEI */
		int imeiMax;            /* Number of allocated MEIs (in rgmei) */
		MEI rgmei [0];          /* The MEI's */
		} MES;

#define cwMES CwFromCch(sizeof (MES))

#define PmeiImei(imei)	(&(*vhmes)->rgmei[(imei)])
#define PmeiCur() 	PmeiImei((*vhmes)->imeiCur)



#define imtmMaxMenu	64	/* Max items on one menu */
#define cchMaxMenuText	32	/* Max no chars in menu text */

#define iContextGlobal 0
#define iContextDocType 1
