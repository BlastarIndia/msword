#include "crmgr.h"

#define cchszExtMax         6
#define cchRTFBuffMax       512
#define cchRTFPassMax       (4*1024)
#define cchRTFToFnMaxEmm    (24*1024)
#define cchRTFToFnMaxNE     (8*1024)

struct RARF    /* RftAppendRgchtoFn parm */
	{
	int fn;
	};

struct EXCR      /* EXternal Converter Record */
	{
	BOOL fInUse;  /* LoadConvtr has been called */
	int hribl;              /* hribl for foreign files */
	HANDLE hLib;
	HSTACK hStack; 
	HANDLE ghszFn;
	HANDLE ghszSubset;
	HANDLE ghBuff;
	HANDLE ghszVersion;
	FARPROC lpfnFIsFormatCorrect;
	FARPROC lpfnForeignToRtf;
	FARPROC lpfnRtfToForeign;
	FARPROC lpfnGetIniEntry;
	struct CA ca;
	int fnTempRTF;
	WORD hppr;
	};




/* File Conversion Errors */
#define fceNoMemory         -8      /* out of global memory */
#define fceDiskError        -4      /* out of disk space or disk error */
#define fceUserCancel       -13     /* user requested cancel operation */


#define ichMaxCvt   256

#define itNil      -1
#define itCvtName   0
#define itCvtPath   1
#define itCvtExtMin 2


/* Document File formats */
#define dffNil          -10

/* win.ini specified converters: dff >= 0*/

/* FileOpen formats */
#define dffOpenText     -6
#define dffOpenText8    -5
#define dffOpenRTF      -4
#define dffOpenBiff     -3
#define dffOpenMp       -2
#define dffOpenWks      -1
#define dffOpenMin      -6

/* FileSaveAs formats */
#define dffSaveNative   -7
#define dffSaveDocType  -6
#define dffSaveText     -5
#define dffSaveTextCR   -4
#define dffSaveText8    -3
#define dffSaveText8CR  -2
#define dffSaveRTF      -1
#define dffSaveMin      -7

#define DffISaveFmt(iFmt) ((iFmt)+dffSaveMin)
#define ISaveFmtDff(dff)  ((dff)-dffSaveMin)

#define gmemLibShare    (GMEM_MOVEABLE|GMEM_DDESHARE)
