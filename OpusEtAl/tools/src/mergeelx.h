typedef unsigned ELK;
#define	elkNil		0
#define elkMin		1	/* minimum meaningful value */

typedef struct _tdr	/* Temporary Dialog Record */
	{
	unsigned char *stHidName;
	unsigned char *stCabName;
	unsigned char *stCabi;
	struct _tdr *ptdrNext;		/* next in global list of TDR's */
	struct _tcr *ptcrFirst;		/* first child name */
	int ctir;			/* item count */
	struct _tir *ptirFirst;		/* first item */
	} TDR;


typedef struct _tcr
	{
	struct _tcr *ptcrNext;
	unsigned char *stParentField;
	unsigned char *stChildName;
	} TCR;


typedef struct _tir
	{
	struct _tir *ptirNext;
	unsigned char *stName;
	unsigned char *stElv;
	unsigned char *stFieldName;
		int IszElkSync;
	} TIR;

/* Performance */

#define celkQuantum	500
#define cchQuantum	4096

#define Assert(f)	{ if (!(f)) AssertFailed(); }
