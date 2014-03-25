/* hyphdefs.h -- definitions for hyphenation routine */

/* #define trieMax 5839 */
/* #define opMax 166 */

#define cchMaxWord 40
#define chHyphBase 0140

struct TRIE
	{
	unsigned ch : 8;
	unsigned op : 8;
	unsigned yes : 16;
	};

struct OUTP
	{
	unsigned dot : 5;
	unsigned val : 3;
	unsigned next : 8;
	};

struct TRIE_NODE
	{
	unsigned ch : 8;
	unsigned op : 8;
	unsigned yes : 16;
	};

/* HYPD hyphenation descriptor defines possible and selected hyphenation
points in a word */
struct HYPD
	{
	int     ichMac;
/* -1: not visible, ich: hyphen before character ich in rgch, shown as
insertion bar if position is not in rgich, otherwise - is highlighted */
	int     ichSel;
/* position after ich is beyond limit of effective hyphenation (marked in text field) */
	int     ichLimEffective;
	int     iichMac;
/* selected as the best choice based on vfli */
	int     iichBest;
	CP      cpFirst;
	char    rgich[cchMaxWord]; /* hyph point after ich */
	char    rgch[cchMaxWord];
	CP      rgdcp[cchMaxWord];
	};


/* header block read from Word Hyphenation file */
struct HYPFH
	{
	/* byte addresses of sections of the data file */
	unsigned cbTrieBase,    /* start of trie transitions */
		cbOutpBase,     /* start of hyphenation outputs */
		cbCttBase,      /* start of translation table */
		cbMac;          /* end of file */
	};

#define cchHYPFH 8

/* describes the state of the hyphenation file */
struct HYPFD
	{
	uns     cbTrieBase;
	int     chHttFirst;
	int     chHttLast;
	/* heap pointers to sections of the data file read into memory */
	struct OUTP	(**hrgoutp)[];
	char    (**hrgbctt)[];
	int     fn;     /* if fnNil, file is not open */
	};

#define cchTRIE (sizeof (struct TRIE_NODE))
#define cchOUTP (sizeof (struct OUTP))
#define cwOUTP (cchOUTP / sizeof(int))



#define trie_ch(hptrie)  (hptrie->ch)
/* W A R N I N G ! ! !   W A R N I N G ! ! ! */
/* Byte order dependency                     */
#ifdef MAC
#define trie_yes(hptrie) ((hptrie->yes << 8) + (hptrie->yes >> 8))
#else
#define trie_yes(hptrie) (hptrie->yes)
#endif /* if-else-def MAC */
#define trie_op(hptrie)  (hptrie->op)


struct HYPB  /* HYphenation Parameter Block */
	{
	BOOL    fWholeDocScan;
	BOOL    fHyphWord;
	BOOL    fDirty;
	int	grpfvisiSave;
	BOOL	fForceWidth;
	uns	dxa;
	CP      cpStart, cpLim;
	CP      cpLine;
	CP      cpLinePrev; 
	CP 	cpLimDr;


	struct HYPD     hypd;
#ifdef WIN
	struct CA	caUndo;
	int		cHyph;
	BOOL		fClearHyph;
#endif
	int		sty;
	};
#define cbHYPB (sizeof(struct HYPB))

/* Prompt id's for alert notes used within hyph.c */
#ifdef MAC
#define anHyphEOD	SzFrameKey("End of document reached.",EndHyph)
#define anHyphContinue	\
	SzFrameKey("Continue hyphenation from beginning of document?",ContinHyph)
#define anHyphFini	SzFrameKey("Finished hyphenating selection.",EndHyphSel)
#else /* WIN */
#define anHyphEOD       1                 /* mstHyphComplete */
#define anHyphContinue  2                 /* IDPMTHypRepeat */
#define anHyphFini      1                 /* mstHyphComplete */

#define dxaHotZMin	((czaInch / 100) + 1)
#endif /* WIN */

typedef struct 
		{
	int **hppr;
	CP  cpRptNext;
	} CPR;


