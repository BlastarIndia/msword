/* F O R M U L A . H */

/* units changed in case of Windows to get proper representation of formulas
	on high resolution devices (kcm). */

/* parameter block for FormatFormula, including an FMA */
struct FMAL
		{
/* entry for the formula stack */
		union
			{
			struct FMA
				{
				char    fmt : 4;
				char    jc : 2;
				char	fLineOrVar : 1;
				char	fInline : 1;
				unsigned char 	cParen;

				char    ich;

#ifdef MAC
		char 	dypAscent;
		char 	dypDescent;
#endif /* MAC */
				
#ifdef WIN
		char	dummyW;
		int	dypAscent;
		int	dytAscent;
				int     dypDescent;
				int     dytDescent;
#endif /* WIN */

#ifdef MAC
			char    dypFract;
#endif /* MAC */

#ifdef WIN
		int 	dypFract;
#endif /* WIN */
				
		int     xt;
				int     xp;
				int     bchr;

#ifdef WIN
			int     dytFract;
#endif /* WIN */

/* formatting info carried to the right paren. Depends on fmt. */
				union   {
					long lOptions;
						struct {
								union {
									int hpsHMove; /* dxpMove */
										int hpsVMove; /* dypMove */
										struct {

#ifdef MAC
												char hpsHSpace; /* dxpSpace */
												char hpsVSpace; /* dypSpace */
#endif /* MAC */
#ifdef WIN
												unsigned char hpsHSpace; 
								/* dxpSpace */
												unsigned char hpsVSpace; 
								/* dypSpace */
#endif /* WIN */
												};
										};
								union {
						struct {
							unsigned char  dhpsAscent;
							unsigned char  dhpsDescent;
							};
									char cCol;
										struct {
											char chLeft;
												char chRight;
												};

										int grpfSides : 4;
										struct {
												char fTop : 1;
												char fBottom : 1;
												char fLeft : 1;
												char fRight : 1;
												};
										};
								};
						};
				};
			struct FMA fma;
			};
/* stuff not in fma */
		CP           cp;
		int          doc;
		BOOL         fError;
		BOOL	     fLiteral;
		int          bchrChp;
		struct FTI * pftiDxt;
		};



/* NOTE:
	The following #define's must be kept in the same order as the
		arrays rgwFmaCmds and rgwFmaOpts in formula.c
*/

/* define formula types */

#define fmtNil		-1

#define fmtArray	0
#define fmtBracket      1
#define fmtDisplace	2
#define fmtFract        3
#define fmtIntegral     4
#define fmtList		5
#define fmtOver         6
#define fmtRoot         7
#define fmtSubSuper     8
#define fmtBox		9

#define fmtComma        14
#define fmtEnd          15


/* command options */

#define optAC		0
#define optAI		1
#define optAL		2
#define optAR		3
#define optBA		4
#define optBC		5
#define optBO		6
#define optCO		7
#define optDI		8
#define optDO		9
#define optFC		10
#define optFO		11
#define optHS		12
#define optIN		13
#define optLC		14
#define optLE		15
#define optLI		16
#define optPR		17
#define optRC		18
#define optRI		19
#define optSU		20
#define optTO		21
#define optUP		22
#define optVC		23
#define optVS		24


/* option masks */

#define omskAC		(1L << optAC)
#define omskAI		(1L << optAI)
#define omskAL		(1L << optAL)
#define omskAR		(1L << optAR)
#define omskBA		(1L << optBA)
#define omskBC		(1L << optBC)
#define omskBO		(1L << optBO)
#define omskCO		(1L << optCO)
#define omskDI		(1L << optDI)
#define omskDO		(1L << optDO)
#define omskFC		(1L << optFC)
#define omskFO		(1L << optFO)
#define omskHS		(1L << optHS)
#define omskIN		(1L << optIN)
#define omskLC		(1L << optLC)
#define omskLE		(1L << optLE)
#define omskLI		(1L << optLI)
#define omskPR		(1L << optPR)
#define omskRC		(1L << optRC)
#define omskRI		(1L << optRI)
#define omskSU		(1L << optSU)
#define omskTO		(1L << optTO)
#define omskUP		(1L << optUP)
#define omskVC		(1L << optVC)
#define omskVS		(1L << optVS)


/* command option masks */

#define mskArray	(omskAL | omskAR | omskAC | omskCO | omskVS | omskHS)
#define mskBracket	(omskLC | omskRC | omskBC)
#define mskDisplace	(omskFO | omskBA | omskLI)
#define mskFraction	(0L)
#define mskIntegral	(omskIN | omskSU | omskPR | omskFC | omskVC)
#define mskList		(0L)
#define mskRoot		(0L)
#define mskSubSuper	(omskUP | omskDO | omskAI | omskDI)
#define mskOver		(omskAL | omskAR | omskAC)
#define mskBox		(omskTO | omskBO | omskLE | omskRI)




#define ifmaMax 40

#ifdef WIN
#define chRoot 214      /* Root */
#define chProduct 213   /* Pi */
#define chSumma 229     /* Sigma */
#define chIntegral 242  /* curly s-like thing */
#define chRootExt 96    /* Root extension */    
#endif /* WIN */
#ifdef MAC
#define chRoot 214      /* Root */
#define chProduct 213   /* Pi */
#define chSumma 229     /* Sigma */
#define chIntegral 242
#define chRootExt 96    /* Root extension */
#endif /* MAC */

#ifdef MAC
#define hpsSymbolLast  (24*2)
#endif /* MAC */
#ifdef WIN
#define hpsSymbolLast  (22*2)
#endif /* WIN */

#define hpsSubSuperDef	(-6)
