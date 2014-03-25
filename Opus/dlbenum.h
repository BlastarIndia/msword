/* DLBENUM.H --- contains the definition for index to
					drop-down list box content table. */

#define iEntblDocFNPrintAt	0
#define iEntblSkt		1
#define iEntblDffSave		2
#define iEntblNfc		3
#define iEntblCatSort		4
#define iEntblPrSrc		5
#define iEntblPrFeed		6
#define iEntblDffOpen		7
#define iEntblBrclTable		8

/* add new ones here and update max */

#define iEntblMax		9


/* separate Entbl in cmdLook1.c */

#define iEntblCharColor		0
#define iEntblBrcp		1
#define iEntblBrcl		2
#define iEntblCmdLook1Max       3

/* separate Entbl in cmdLook2.c */

#define iEntblSecStart		0
#define iEntblPosH		1
#define iEntblPosV		2
#define iEntblLook2Max	  	3




#define cFNPos              4
#define cBrcp               5
#define cBrcl               5
#define cSecStarts          5
#define cSkt                3
#define cDffSave            7
#define cNfc                5
#define cCatSort            6
#define cPrSrc              6
#define cPrFeed             8
#define cPosH               5
#define cPosV               4
#define cCharColor          9
#define cDffOpen            6
#define cBrclTable          4

typedef struct _entbl {
				int  iMax;
				CHAR rgst[][];
				}            ENTBL;



/* SPrm and Ninch Type */
typedef struct _spnt
	{
	char sprm;
	int  wNinchVal;
	} SPNT;


