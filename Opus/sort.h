/******************************************************************************
**
**     COPYRIGHT (C) 1986 MICROSOFT
**
*******************************************************************************
**
** Module: sort.c --- Sort Functions.
**
** Functions included:
**
** REVISIONS
**
** Date         Who Rel Ver     Remarks
**              yxy             Ported from PC Word 3, and revised
**                              for efficiency.
**
*****************************************************************************/

typedef int     SC;     /* Sort Comparison result */
typedef int	SE;	/* Sort Error code --- corresponds to eid */

#define SENone       0
#define SECancelled  1
#define SENoMem      eidNoMemSort
#define SEFldCh      eidSortFldCh
#define SENoRec      eidSortNoRec
#define SERecIgnored eidSortRecIgnored      
#define SETooManyRec	eidCpRollOver

struct SOD
		{ /* SOrt Descriptor */
		CP        cpFirstRecord;
		CP        cpLimRecord;
		union
			{
			NUM		numKey;
		struct DTTM	dttmKey;
			struct
				{
				CP cpFirstKey;
				CP cpLimKey;
				};
			};
		};
#define cSODInit        10
#define cbSOD		sizeof(struct SOD)
#define cwSOD           CwFromCch(cbSOD)



#define iSortSepComma  0
#define iSortSepTab    1

/* Sort Key Types */

/*  WARNING:  these must agree with the szSkt's in globdefs.h in number
				and order!! Also, cSkt in dlbenum.h*/

#define sktAlphaNum      0
#define sktNumeric       1
#define sktDate		 2

#define iSepComma	0
#define iSepTab		1

struct SOT
		{ /* SOrt Type */
		BOOL      fDescending;  /* sort order: ascending or descending     */
		unsigned  skt;          /* sort key type                           */
		unsigned  iSep;     	/* sort separator: comma or tab            */
		unsigned  uFieldNum;    /* key field number                        */
		BOOL      fColumnOnly;  /* what to swap: column only or whole rec. */
		BOOL      fSortCase;    /* fTrue iff case sensitive.               */
	/* End of Compatible section. */
		CHAR      fTableSort;
		CHAR      fInOneCell;
		SC        (*pfnSc)();   /* pointer to a sort comparison function   */
	struct CA ca;		/* section of a document clobbered by sort */
		};

#define cbSOTInit	(offset(SOT, fTableSort))


/* SC function return values */
#define scLess            (-1)
#define scEqual           0
#define scGreater         1
#define FScLessEqual(x)   ((x) <= 0)
#define DlToSc(_dl)	  ((_dl) < 0L ? scLess :			\
				((_dl) > 0L ? scGreater : scEqual))


/* Macros to figure out an appropriate SC function from sot. */
/* Note: These definitions must be changed in sync with rgpfnSc in sort.c */
#define irgpfnScMax     8        /* # of distinctive SC functions */
#define dirgpfnScAlpha  4
#define dirgpfnScNum	2
#define dirgpfnScDttm   2
#define dirgpfnScCI     2

#define PfnScFromSot(_sot)						\
			(rgpfnSc[mpsktipfnScBase[(_sot).skt] +			\
				(((_sot).skt == sktAlphaNum) ?		\
					((_sot).fSortCase ? 0 : dirgpfnScCI) :\
					0) +				\
				(_sot).fDescending])

#define ipfnPKMax	3

#define ipfnSRMax	3
#define ipfnSRNormal	0
#define ipfnSRBlock	1
#define ipfnSROutline	2

#define PfnSrFromSelPwwd(_sel, _pwwd)					\
		(rgpfnSR[(_pwwd)->fOutline ? ipfnSROutline :		\
						((_sel).fBlock ?		\
						ipfnSRBlock :		\
						ipfnSRNormal)])

typedef struct
		{
		int	lvl2Cur;
		int	lvl2Top;
		} OSRIB;	/* Outline Sort Record Info. Block */

#define FChNL_EOL(_ch)  ((_ch) == chReturn || (_ch) == chEol ||		\
			(_ch) == chCRJ || (_ch) == chTable)

#define cchMaxSKFetch	66  /* space for documented 65 chars plus null term */
