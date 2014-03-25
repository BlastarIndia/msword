/* formatting bin */
#define cbFkp   (512/*cbSector*/ - sizeof(FC) - 1)
struct FKP
	{
	FC      rgfc[1]; /* crun + 1 entries from fcFirst to fcLim */
	/*char  rgb[1];   crun entries based on FKP, points to chpx or papx*/
	char    rgb[cbFkp];
	char    crun;   /* number of runs */
	};
#ifdef MAC
#define ifcFkpNil (-1)	/* native code depends on this */
#else
#define ifcFkpNil (512 /*cbSector*/ / sizeof(FC))
#endif

#define cbFkpPre35   (128/*cbSector*/ - sizeof(FC) - 1)
struct FKPO
	{
	FC      rgfc[1]; /* crun + 1 entries from fcFirst to fcLim */
	/*char  rgb[1];   crun entries based on FKP, points to chpx or papx*/
	char    rgb[cbFkpPre35];
	char    crun;   /* number of runs */
	};

/* There are never more than three of these: 2 global for the scratch file
	(chp and pap), and one for the file we are currently writing. */
struct FKPD
	{ /* FKP Descriptor (used for maintaining insert properties) */
	int     bFreeFirst;   /* offset to next run to add */
	int     bFreeLim;       /* offset to byte after last unused byte */
	PN      pn;     /* pn of working FKP in scratch file */
	FC      fcFirst;
	int     fPlcIncomplete; 
	struct  CHP     chp;
	};

struct FKPDP
	{ /* subset for Paras */
	int     bFreeFirst;
	int     bFreeLim;
	PN      pn;
	FC      fcFirst;
	};

struct FKPDT
	{ /* subset for Text */
	int     bFreeFirst;     /* first unused byte on page pn if != pnNil */
	int     bFreeLim;       /* not used */
	PN      pn;             /* if != pnNil, pn of partially full page */
	FC      fcLim;          /* end of last written rgch. */
	};


#ifdef COMMENT
/* since the following "stuctures" are not word aligned in an FKP,
the definition here is just a comment */
/* Character properties encoded as a differential */
struct CHPX
	{
	char    cb;     /* Number of bytes stored in chp */
	struct CHP chp;
	};

/* Paragraph properties encoded as a differential */
struct PAPX
	{
	char    cb;     /* Number of bytes stored in rest of PAPX */
	char    stc;
	int     paph;
	struct PRL grpprl;
	};
#endif
