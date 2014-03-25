/* recorder.h */

/* Recorder ACcumulator */
struct RAC
		{
		int racop;
		int sty;
		int dSty;
		};

#define racopNil        0
#define racopDelete     1
#define racopMove       2
#define racopSelect     3

#define racopChar       4
#define racopPara       5
#define racopSect       6
#define racopDoc        7

#define racopHScroll	8
#define racopVScroll	9

#define racopInsert	10


/* ComMand Recorder */
typedef struct _cmr
	{
	int iContext;
	char stName [cchMaxSyName + 1]; /* +1 because it is really an stz */
	char stDesc [0]; /* allocated to size */
	} CMR;

#define cbCMR sizeof (CMR)

