typedef struct _ndd
		{
		int           doc;
		CHAR         *stFName;
		struct STTB **hsttbAssoc;
		struct DOP   *pdop;
	BOOL	      fAppendApprox;
	BOOL	      fWordsApprox;
	BOOL	      fPagesApprox;
	BOOL	      fUseSt;
		} NDD;

typedef struct _dsi
	{
	BOOL    fFullPrint;
	long    cWords;
	long    cCh;
	}   DSI;        /* Doc Size Information */
