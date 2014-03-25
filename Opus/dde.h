/* D D E . H */
/*  include file for dde support */


#ifdef DEBUG
#ifdef PCJ
#define SHOWDDEMSG
#endif /* PCJ */
#endif /* DEBUG */

#define secDdeTimeOutDef 20
/*  after this long we put up an hour glass */
#define usecHourGlassDef (7 * 1000L)

/* DDE window messages (why not in qwindows.h?? Ask the Windows people!) */
#define WM_DDE_FIRST     0x03e0
#define WM_DDE_INITIATE  (WM_DDE_FIRST+0)
#define WM_DDE_TERMINATE (WM_DDE_FIRST+1)
#define WM_DDE_ADVISE    (WM_DDE_FIRST+2)
#define WM_DDE_UNADVISE	 (WM_DDE_FIRST+3)
#define WM_DDE_ACK       (WM_DDE_FIRST+4)
#define WM_DDE_DATA      (WM_DDE_FIRST+5)
#define WM_DDE_REQUEST   (WM_DDE_FIRST+6)
#define WM_DDE_POKE      (WM_DDE_FIRST+7)
#define WM_DDE_EXECUTE   (WM_DDE_FIRST+8)
#define WM_DDE_LAST      (WM_DDE_FIRST+8)

#define GMEM_DDE         (GMEM_DDESHARE | GMEM_MOVEABLE)
#define GMEM_SENDKEYS    (GMEM_LOWER|GMEM_MOVEABLE)



struct QUE  /* a circular QUEue */
	{
	int iFirst;     /* element at head of queue */
	int iLim;       /* end of the queue */
	int c;          /* number of elements in the queue */
	int iMax;       /* available number of slots in queue */
	int cb;         /* size of each queue slot */
	int iIncr;      /* expansion increment */
	CHAR rgb [1];   /* actual data */
	};
#define cbQUEBase (offset (QUE, rgb))

struct DDES /* DDE Status */
	{
	/* Server */
	int     cDclServer : 5;   /* number of active server channels */
	int     fInvalidDdli : 1;  /* some srvr ddli does not have bkmk */
	int     fDirtyLinks : 1;  /* some srvr ddli needs to be updated */

	/* Client */
	int     cDclClient : 5;   /* number of active client channels */
	int     fHotLinksDirty:1; /* some hot link field needs to be recalcd */
	int     fInInitiate : 1;  /* we are in the process of sending INITIATE */

	/* Macro */
	int     cDclMacro : 5;    /* number of macro channels open */

	/*  Global */
	int     secTimeOut: 13;   /* how long to timeout (up to 136 mins) */
	struct QUE      **hque;   /* dde message queue */
	struct PL       **hplddli; /* server links */
	uns/*SD*/ sdResult;	      /* result of fSpecial REQUEST */
	};


struct DMS /* Dde Message Status */
	{
	int : 12;
	int fResponse : 1;
	int fRelease : 1;
	int fNoData : 1;
	int fAck : 1;
	int cf;
	};

#define cbDMS sizeof (struct DMS)  /* size of data header */

struct DAS /* Dde Ack message Status */
	{
	int : 8;
	int : 6;
	int fBusy : 1;
	int fAck : 1;
	};

#define cbDAS (sizeof (struct DAS))

/* DAS values */
#define dasACK          0x8000
#define dasNACK         0x0000
#define dasBUSY         0x4000


struct DDLI /* Dde Link Info */
	{
	union {
		struct
			{
			int dcl : 5;    /* channel link conducted over */
			int dls : 2;    /* Dde link status */
			int : 9;
			int : 16;
			};
		struct    /* dtServer */
			{
			int : 15;
			int fAckReq : 1;/* when sending data, request an ack */
			int fNoData : 1;/* do not send data with WM_DDE_DATA */
			int fDirty : 1; /* linked obj has changed, need to send update */
			int ibkf : 14;  /* bookmark link is to */
			};
		struct    /* dtClient */
			{
			int : 15;
			int fRef : 1;   /* garbage collection: some field refers to this */
			int fHot : 1;   /* some hot link refers to this */
			int fHotDirty:1;/* hot link may exist that needs to be updated */
			int : 14;       /* spares */
			};
		};
	int cf;                 /* format to send/request data in */
	ATOM atomItem;          /* item link references */
	HANDLE hData;           /* handle of last message */
	};

#define cbDDLI (sizeof (struct DDLI))

#define iddliSpecial     iNil
#define iddliNil         iNil
#define iddliRequest     0
#define iddliMinNormal   (iddliRequest+1)
#define iddliMaxClient   127


struct DCLD     /* Dde ChanneL Descriptor */
	{
	HWND hwndUs;        /* handle to our window for the channel */
	HWND hwndThem;      /* handle to their window for the channel */
	struct
		{
		int dt : 2;         /* what type channel (server,client,macro) */
		int fTerminating :1;/* we have sent WM_DDE_TERMINATE message */
		int fBusyPrev : 1;  /* we were busy'd on the last message */
		int fBusyCur : 1;   /* we were busy'd on this message */
		int fRef: 1;        /* for garbage collection */
		int fTermReceived:1;/* WM_DDE_TERMINATE received */
		int fExecuting : 1; /* server: performing EXECUTE */
		int :4;             /* spares */
		/* for macro/special only */
		/* if fResponse & !fAck & !fBusy then NACK */
		int fResponse : 1;  /* a response has been received */
		int fAck : 1;       /* response was ACK */
		int fBusy : 1;      /* response was BUSY */
		int fTermRequest:1; /* Terminate statement executed */
		};

	union {
		struct   /* dtServer */
			{
			int doc;
			int wSpare;
			};
		struct  /* dtClient */
			{                   /* these are the atoms we requested */
			ATOM atomApp;       /* Atom of application */
			ATOM atomTopic;     /* Atom of topic */
			};
		};
	};

#define dclMax 30
#define dclNil 0
#define cbDCLD (sizeof (struct DCLD))
#define cwDCLD (cbDCLD/sizeof (int))

/* Dcl Types */
#define dtServer    	0
#define dtClient    	1
#define dtMacro	    	2
/* #define dtSpecial	    3 (unused) */

/* System Topic Items */
#define docSystem       docNil

#define stiNil          iNil
#define stiMin          0
#define stiSysItems     0
#define stiTopics       1
#define stiFormats      2
#define stiMac          3



/* Dde Link Status */
#define dlsClear    0
#define dlsWait     1
#define dlsUnadvise 2
#define dlsFail     3


struct DQM  /* Dde Queued Message */
	{
	BYTE dcl;       /* channel message came over */
	BYTE dwm;       /* the message (- WM_DDE_FIRST) */
	int wLow;       /* low word of lParam */
	int wHigh;      /* high word of lParam */
};

#define cbDQM (sizeof (struct DQM))
#define idqmMaxInit 4

/* F U N C T I O N S */
NATIVE struct DCLD * PdcldDcl ();
ATOM AtomAddSt ();
ATOM AtomFromStNA ();
LONG UsecDdeTimeOut ();



#ifdef DEBUG
#define ReportDdeError(sz, dcl, message, n) \
			ReportDdeErrorProc (SzShared(sz), dcl, message, n)
#else
#define ReportDdeError(sz, dcl, message, n)
#endif /* DEBUG */


#ifdef DEBUG
#define DdeRpt2(e)  (vdbs.fCommDde2?(e):0)
#define DdeDbgCommAtom(sz, dcl, atom) \
			DdeDbgCommAtomProc (SzShared(sz), dcl, atom)
#define DdeDbgCommSz(sz) \
			DdeDbgCommSzProc (SzShared(sz))
#define DdeDbgCommMsg(sz, dcl, message, wLow, wHigh) \
			DdeDbgCommMsgProc (SzShared(sz), dcl, message, wLow, wHigh)
#define DdeDbgCommPost(dcl, message, wLow, wHigh) \
			DdeDbgCommPostProc (dcl, message, wLow, wHigh)
#define DdeDbgCommInt(sz, w) \
			DdeDbgCommIntProc (SzShared (sz), w)
#define DdeDbgCommLong(sz, l) \
			DdeDbgCommLongProc (SzShared (sz), l)
#else
#define DdeRpt2(e)
#define DdeDbgCommAtom(sz, dcl, atom)
#define DdeDbgCommSz(sz)
#define DdeDbgCommMsg(sz, dcl, message, wLow, wHigh)
#define DdeDbgCommPost(dcl, message, wLow, wHigh)
#define DdeDbgCommInt(sz, w)
#define DdeDbgCommLong(sz, l)
#endif /* DEBUG */


/*  DRIVE code from Excel */
/* WARNING: don't change these without also changing eldden.asm */

/* Event structure - same as the SysMsg structure in USER.H */
typedef struct
		{
	unsigned message;
	WORD paramL;
	WORD paramH;
	DWORD time;
}	SYSMSG;

typedef struct
		{
	unsigned wm;
	unsigned vk;
}	EVENT;

/*----------------------------------------------------------------------------
|	EVT structure 
|
|		This is an element in the keyboard event queue, to be fed
|		to windows through the playback hook.
|
|		Fields:
|			hevtNext	Handle to next evt
|			event		event structure
|
----------------------------------------------------------------------------*/
typedef struct
		{
	int ieventCur;
	int ieventMac;
	EVENT rgevent[1];
}	EVT;

typedef HANDLE HEVT;

typedef struct
	{
	WORD wDataSeg;
	WORD wWinVersion;
	int fnPlaybackHook;	/* the real function */
	}	DRVHD;

struct DRVDATA
	{
	HEVT hevtHead;
	HANDLE hrgbKeyState;
	FARPROC lpfnPlaybackHookSave;
	};

#define cichOpMax   255
