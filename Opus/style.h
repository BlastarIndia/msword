
/*
*   style.n
*/

#define ipapStcScanLim 50
#define istLbNil uNinchList
#define cwMpiststcp 128
#define cchMaxStyle cchNameMax

typedef struct _cesty
	{
	BOOL fDirty;      /* edit session dirtied stsh */
	BOOL fHasDot;     /* doc has document template */
	int  dlmLast;     /* tracks sequences of dlmClicks/dlmChanges in
							tmcDSDefine combo box */
	BOOL fFake;       /* entry in list box but not in stsh */
	BOOL fIgnore;     /* set when we wish to ignore messages generated
							by our calls to some SDM functions  */
	int istLb;        /* list box index to the style we're working on */
	BOOL fRecorded;   /* recorded current style name already */
	BOOL fStyleDirty;  /* we think vstcStyle is dirty */
	} CESTY;
