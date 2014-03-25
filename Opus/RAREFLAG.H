/* consolidate all those rarely used global flags into bitfields, 
	trading rarely-used-code space for precious DS space */

struct RF {	/* Rare flag */
	int fTablePropsNinch: 1;
	int fForcePara: 1;
	int fNormalPrev: 1;
	int fWaitForDraw: 1;
	int fChangeKeys: 1;
	int fHotZValid: 1;
	int fHyphHaveWord: 1;
	int fRanCBT: 1;

	int fPgv: 1;
	int fCbtInit: 2;
	int fRibbonCBT: 2;
	int fInExternalCall: 1;
	int fPauseRecorder: 1;
	int fBorderCropPic : 1;

	int fInQueryEndSession: 1;
	int fMwCreate: 1;
	int fHyphCapitals: 1;
	int fSplIgnoreCaps: 1;
	int udcDefault: 2;
	int fPreloadSelFont: 1;
	int fInFormatLine : 1;	/* Flag to avoid recursion */

	int fInsPgNum: 1;
	int fMustStopCBT: 1;
	int fNotFirstDlg: 1;
	int fCkPgMarg: 1;
	int fVetoViewRef: 1; /* true if double click in ref mark */
	int fInDyadic: 1;
	int fExtendedMemory: 1;
	int fInDisplayFli : 1;	/* Flag to avoid recursion */

	int unused: 7;
	int fInFetchCp : 1;	/* Flag to avoid recursion */
};


#ifdef DEFVARS
struct RF vrf = { 0,0,0,0,0,1,1,0,
			0,2,2,0,0,0,
			0,0,1,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0 };
#else
extern struct RF 	vrf;
#endif
