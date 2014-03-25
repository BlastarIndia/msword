/*  Include file with headers for functions in DOSLIB.ASM */

#define osfnNil     (-1)


struct TIM {                    /* Time structure returned by OsTime */
	CHAR minutes, hour, hsec, sec;
	};

struct DAT {                    /* Date structure returned by OsDate */
	int  year;
	CHAR month, day, dayOfWeek;
	};

/* Constants for MS-DOS filename sizes: all include xtra byte for st or \0 */

#define ichMaxShortName 9      /* Excluding extension */
#define ichMaxExtension 5      /* Includes the period */
#define ichMaxLeaf      (ichMaxShortName+ichMaxExtension-1)
#define ichMaxPath      (ichMaxFile-ichMaxShortName-ichMaxExtension+2)

#define ichMaxLeafNovell 15    /* for screwy Novell Net directory names */

struct FNS       /* FileName Structure: a decomposed filename */
	{
	CHAR stPath [ichMaxPath];             /* Path, incl drive letter */
	CHAR stShortName [ichMaxLeafNovell];  /* short file name, sans ext */
	CHAR stExtension [ichMaxLeafNovell];  /* file extension, with period */
	};

/* DOS File Attributes */
#define DA_NORMAL       0x00
#define DA_READONLY     0x01
#define DA_HIDDEN       0x02
#define DA_SYSTEM       0x04
#define DA_VOLUME       0x08
#define DA_SUBDIR       0x10
#define DA_ARCHIVE      0x20
#define DA_NIL          0xFFFF  /* Error DA */
#define dosxSharing     32      /* Extended error code for sharing viol. */
#define nErrNoAcc       5       /* OpenFile error code for Access Denied */
#define nErrFnf         2       /* OpenFile error code for File Not Found */

/* Components of the Open mode for OpenSzFfname (DOS FUNC 3DH) */
#define MASK_fINH       0x80
#define MASK_bSHARE     0x70
#define MASK_bACCESS    0x07

#define bSHARE_DENYRDWR 0x10
#define bSHARE_DENYWR   0x20

/* DOS Error codes */
/* These are the negative of the codes returned in AX by DOS functions */
#define ecFnfError      -2      /* File Not Found */
#define ecNoPath        -3      /* path not found */
#define ecTooManyFiles  -4      /* too many open files */
#define ecNoAccError    -5      /* Access Denied */
#define ecBadHndError   -6      /* Bad handle passed in */
#define ecNoDriveError  -15     /* Non-existent drive passed in */
#define ecWriteProtect  -19     /* disk is write protected */
#define ecDrvNotReady   -21     /* drive is not ready */
#define ecInvalLength   -24     /* invalid length */
#define ecSeekError     -25     /* Seek Error */
#define ecShareViolatn  -32     /* sharing violation */
#define ecWrongDisk     -34     /* wrong disk */

#define ecDiskFull      -200    
#define ecHardError     -201    /* somehardcore error */

#define ecNoErr         0
/* Seek-from type codes passed to DOS function 42H */

#define SF_BEGINNING    0       /* Seek from beginning of file */
#define SF_CURRENT      1       /* Seek from current file pointer */
#define SF_END          2       /* Seek from end of file */


/* Normalize File Options */
#define nfoNormal   0
#define nfoPathOK   1
#define nfoWildOK   2
#define nfoPathCk   4
#define nfoDot	    8
#define nfoPic	    16
#define nfoDotExt  32
#define nfoPathWildOK (nfoPathOK | nfoWildOK) /* for convenience */

/* File Name Type */
#define fntInvalid	0
#define fntValid        1
#define fntValidWild	2

#define FValidFile(_rgch, _ichMax, _pichError, _fPathOK) \
		(FntSz((_rgch), (_ichMax), (_pichError), \
			((_fPathOK) ? nfoPathOK : nfoNormal)) == fntValid)

/* buffer structure to be used with FFirst() and FNext() */
struct FINDFILE
	{
	char buff[21];
	char attribute;
	union 
		{
		unsigned time;
		struct 
			{
			unsigned sec : 5;
			unsigned mint: 6;
			unsigned hr  : 5;
			};
		};
	union 
		{
		unsigned date;
		struct 
			{
			unsigned dom : 5;
			unsigned mon : 4;
			unsigned yr  : 7;
			};
		};
	unsigned long cbFile;
	char szFileName[13];
	};

