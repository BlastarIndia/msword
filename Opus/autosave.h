#ifdef AUTOSAVE
typedef struct _asd
			{
			long	mscBase;
			long    dcpEdit;
			long	mscPostpone;
			long	dmscLastPostpone;
			}   ASD;

typedef struct _tmr
			{
			long	dmscMin;
			long	dmscDelta;
			int     kdcpRange;
			}   TMR;    /* Time Range. */

#define dmscMinute	(60L * 1000L)

#define TmrForMinMaxKdcp(mMin,mMax,kdcp) { (mMin)*dmscMinute, \
		(((mMax)-(mMin))*dmscMinute)/elMost, (kdcp)	}

#define elMost            6

#define dmscPostponeDef  (5 * dmscMinute)

/* Special values returned from DMinuteUserPostpone. */
#define dMinuteCancel      -1
#define dMinuteError       -2
#define AccumulateDcpEdit(dcp)  (asd.dcpEdit += (long) (dcp))

int DMinuteUserPostpone();
#endif /* AUTOSAVE */


#define iASHigh     0
#define iASMedium   1
#define iASLow      2
#define iASNever    3
#define iASMac      4
