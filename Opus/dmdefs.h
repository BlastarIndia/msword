/* definitions for Document Management Structures */

struct SUMD
	{   /* SUMmary Descriptor */
/* IMPORTANT: 
	hsz in SUMD and dmqsd in DMQD and fFooTest in DMQFLAGS has to be the same order 
	(first 5 items only)
*/
	CHAR   **hszTitle;     /* Document Title */
	CHAR   **hszSubject;   /* Document Subject */
	CHAR   **hszAuthor;    /* Document Author */
	CHAR   **hszKeywords;  /* String of keyword of importance */
	CHAR   **hszRevisor;   /* Name of revisor  */
	struct DTTM dttmRevision; /* date of last revision of the document */
	struct DTTM dttmCreation; /* date of creation of the document */
	CP     cchDoc;     /* number of chars in the doc */
	int    fQuickSave; /* true if file is quick saved */
	};


#define cchMaxDMQPath    255 /* not counting null terminator */
#define cchMaxAONExp     80
#define MAXLEAFLENGTH    256
#define MAXNUMLEAVES     16
/* These definitions are for building AND-OR-NOT evaluation trees for the 
	Query tests. */

/* Operator defines for the AONTrees */
#define OPNIL   255
#define OPLEAF  0
#define OPOR    1
#define OPAND   2
#define OPNOT   3
#define OPSTART    4

#define chAND   '&'
#define chNOT   '~'
/* chOR is a variable depending on chFldSep, and is defined in HBuildAONTree */


struct AONNODE
	{
	struct AONNODE  **hLeftBranch;    /* Left decendent */
	struct AONNODE  **hRightBranch;   /* Right decendent */
	CHAR   operator;        /* Is this node an AND,OR,NOT,or LEAF*/
	CHAR   iLeaf;            /* If LEAF node, which item in */
	};              /* rgchLeaves does it correspond to */

#define   cwAONNODE    ( sizeof(struct AONNODE) / sizeof(int) )

struct DATEINFO
	{
	struct DTTM dttmFrom;
	struct DTTM dttmTo;
	};


/* Document Management Query Sub-Descriptor */
struct DMQSD
	{
	CHAR **hstSpec;         /* query specification */
	struct AONNODE **hTree; /* heap ptr to Tree */
	CHAR **hrgdchLeaves;    /* heap ptrs to arrays of chars */
	CHAR **hrgLenLeaves;    /* heap ptrs to arrays of chars */
	CHAR cLeaves;           /* count of leaves */
		CHAR unused;            /* Padding byte so that structure is an even
								number of bytes long. */
	};

#define   cDMQSpecMac 6

/* Document Management Query Descriptor */
struct DMQD
	{
/* IMPORTANT: 
	hsz in SUMD and dmqsd in DMQD and fFooTest in DMQFLAGS has to be the same order 
	(first 5 items only)
*/
	struct DMQSD dmqsdTitle;
	struct DMQSD dmqsdSubject;
	struct DMQSD dmqsdAuthor;
	struct DMQSD dmqsdKeyword;
	struct DMQSD dmqsdSavedBy;
	struct DMQSD dmqsdText;
	int fCase;
	CHAR **(**hrghszTLeaves); /* heap array of heap strings for grep*/
	struct DATEINFO CDateInfo;  /* Creation Date info */
	struct DATEINFO SDateInfo;  /* Saved Date info */
	};

/* WATCH OUT : cbDMQSD may not be even number of bytes and so is cbDMQD */
#define   cbDMQD    (sizeof(struct DMQD))



struct DMQFLAGS /* Query Flags grouped in one place */
	{
	union
		{
		int grpf;
/* IMPORTANT: 
	hsz in SUMD and dmqsd in DMQD and fFooTest in DMQFLAGS has to be the same order 
	(first 5 items only)
*/
		struct 
			{
			int fTitleTest       :1 ;
			int fSubjectTest     :1 ;
			int fAuthorTest      :1 ;
			int fKeywordTest     :1 ;

			int fSavedByTest     :1 ;
			int fTextTest        :1 ;
			int fCFromDateTest   :1 ;
			int fCToDateTest     :1 ;

			int fSFromDateTest   :1 ;
			int fSToDateTest     :1 ;
			int fSpare           :6 ;
			};
		};
	};


#define dmf_SortField           0
#define dmf_FullFileName        2
#define dmf_CompactFileName     4

/* Document Management Sort Code, same as index into the sort list */
#define dmsMin                  0
#define dmsName                 0
#define dmsAuthor               1
#define dmsCreationDate         2
#define dmsRevisionDate         3
#define dmsLastSavedBy          4
#define dmsSize                 5
#define dmsMax                  6

/* Document Management Action */
#define dma_StartFromScratch    0
#define dma_Resort              1
#define dma_NarrowSearch        2
#define dma_ReadFcs             3

struct DMFLAGS /* Document Management Flags grouped in one place */
	{
	int dms             :3;   /* which field are we sorting on? */
	int dma             :3;   /* what kind of action to FDMEnum */
	int fForceStartFromScratch :1;/* true if we want to force a StartFromScratch */
	int fSrhInProgress  :1;   /* true if Cat/Search In Progress db is up */

	int fDMOutOfMemory  :1;   /* Did we run out of Heap during search? */
	int fIniQPath       :1;   /* was there a path stored in opus.ini? */
	int fIncompleteQuery:1;   /* true if query list may be incomplete */
	int fFileInUse      :1;   /* true for doing a ShowFileInUse */
	int fUpdateBanter   :1;   /* true if banter needs to be updated */
	int fTmpFile        :1;   /* true if we encounter a temp file */
	int fSpare          :2;
	};

/* offset in the display line of Sort By Field */
#define ichDMSortMin    15

#define cchMaxFDisp     40
#define cchMaxSIDisp    70
#define cchMaxDMFileSt  cchMaxSIDisp+ichDMSortMin+1

#define MAXSEARCHFIELD   256
#define cEnumInit 40    /* intial number of files to allocate space for */
#define cwMaxLeaves (MAXNUMLEAVES/sizeof(int))
#define chstDMDirsMax    12


#define ichMaxSortField 41
#define ichMaxFullFileName      (ichMaxFile+1) /* big enough for a stz */
#define ichMaxCompactFileName   41

