/* idle.h */

/* OnTime Descriptor (for Opel) */
typedef struct _otd
	{
	unsigned long lTime;
	unsigned long lTolerance;
	char stMacro [1];
	} OTD;


typedef union _idf    /* idle flags */
	{
	struct 	
			{
		int	fCorrectCursor: 1;
		int	fIBDlgMode: 1;
			int fInvalSeqLev : 1;
			int fInIdle : 1;
			int : 4;
		int	fDead: 1;
		int	fIconic: 1;
		int	: 6;
		};
	struct 
			{
		int	: 8;
		int	fNotLive: 8;
		};

	} IDF;


