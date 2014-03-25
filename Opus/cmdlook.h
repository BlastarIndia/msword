/* C M D L O O K . H */

/*  contains the DoLooks tables */




/* Looks codes.  Used by Ruler, Lookshelper and DoLooks() in cmd.c */

#define FLHIlcd(ilcd)         ((ilcd) >= ilcdBold && (ilcd) <= ilcdSubscript)
#define FRulerIlcd(ilcd)      ((ilcd) >= ilcdParaLeft && (ilcd) <= ilcdParaOpen)

#define IdLHFromIlcd(ilcd)    ((ilcd) - ilcdBold + IDLKSBOLD)
#define IlcdFromIdLH(idLH)    ((idLH) + ilcdBold - IDLKSBOLD)
#define IdRulFromIlcd(ilcd)   ((ilcd) - ilcdParaLeft + idRulParaLeft)
#define IlcdFromIdRul(idRul)  ((idRul) + ilcdParaLeft - idRulParaLeft)

/* character properties  - order matches Looks Helper ids from ribbon.h */
#define ilcdBold              (0)
#define ilcdItalic            (1)
#define ilcdSmallCaps         (2)
#define ilcdKulSingle         (3)
#define ilcdKulWord           (4)
#define ilcdKulDouble         (5)
#define ilcdSuperscript       (6)
#define ilcdSubscript         (7)

/* neither ruler nor ribbon */
#define ilcdVanish            (8)   /* Annotations */
#define ilcdPlainText         (9)
#define ilcdStrike            (10)
#define ilcdRMark             (11)
#define ilcdParaNormal        (12)


/* paragraph properties  - order matches Ruler idRuls from ruler.n */
#define ilcdParaLeft          (13)
#define ilcdParaCenter        (14)
#define ilcdParaRight         (15)
#define ilcdParaBoth          (16)
#define ilcdSpace1            (17)
#define ilcdSpace15           (18)
#define ilcdSpace2            (19)
#define ilcdParaClose         (20)
#define ilcdParaOpen          (21)

#define ilcdMin               ilcdBold
#define ilcdLast              ilcdParaOpen
/* Look Command Property */
#define lcpNone                 0
#define lcpChp                  1
#define lcpPap                  2

/* look command descriptor */
/* variant of the lcd table used in mac word, but indexed by
	ilcd codes rather than containing kc codes and searching.
	Also, val field is an int not a char.
*/
struct LCD
		{
		int     lcp : 2;
		int     fFlip:1;
		int     fSpecial:1;
		int     fNoUpdate:1;
		int     :3;
		char    sprm;
		int     val;
		};




#ifdef RGLCD
csconst struct LCD rglcd[] =
		{

		/* character properties: note that these ilcd values
			must be in the same order as the ribbon id's defined
			in ribbon.h   AND in the Cmd structure above.
		*/

		/* ilcdBold */         {lcpChp, 1,0,0, sprmCFBold,     0            },
		/* ilcdItalic */       {lcpChp, 1,0,0, sprmCFItalic,   0            },
		/* ilcdSmallCaps */    {lcpChp, 1,0,0, sprmCFSmallCaps,0            },
		/* ilcdKulSingle */    {lcpChp, 0,1,0, sprmCKul,       kulSingle    },
		/* ilcdKulWord */      {lcpChp, 0,1,0, sprmCKul,       kulWord      },
		/* ilcdKulDouble */    {lcpChp, 0,1,0, sprmCKul,       kulDouble    },
		/* ilcdSuperscript */  {lcpChp, 0,1,0, sprmCHpsPos,    6            },
		/* ilcdSubscript */    {lcpChp, 0,1,0, sprmCHpsPos,    -6           },


		/* these are character properties but not a ribbon ilcd */
		/* ilcdVanish */       {lcpChp, 1,0,0, sprmCFVanish,   0            },
		/* ilcdPlainText */    {lcpNone,0,0,0, sprmCPlain,     0            },
		/* NOTE: these are used only in Search-Replace */
		/* ilcdStrike */       {lcpNone,1,0,0, sprmCFStrikeRM, 0            },
		/* ilcdRMark */        {lcpNone,1,0,0, sprmCFRMark,    0            },
		/* paragraph property -- not ruler */
		/* ilcdParaNormal */   {lcpPap, 0,1,1, sprmPStc,       0            },


		/* paragraph properties: note that these ilcd values
			must be in the same order as the idRul's defined in
			ibdefs.h.
		*/

		/* ilcdParaLeft */     {lcpPap, 0,1,0, sprmPJc,        jcLeft       },
		/* ilcdParaCenter */   {lcpPap, 0,1,0, sprmPJc,        jcCenter     },
		/* ilcdParaRight */    {lcpPap, 0,1,0, sprmPJc,        jcRight      },
		/* ilcdParaBoth */     {lcpPap, 0,1,0, sprmPJc,        jcBoth       },
		/* ilcdSpace1 */       {lcpPap, 0,1,0, sprmPDyaLine,   dyaSpace1    },
		/* ilcdSpace15 */      {lcpPap, 0,1,0, sprmPDyaLine,   dyaSpace15   },
		/* ilcdSpace2 */       {lcpPap, 0,1,0, sprmPDyaLine,   dyaSpace2    },
		/* ilcdParaClose */    {lcpPap, 0,1,0, sprmPDyaBefore, dyaParaClose },
		/* ilcdParaOpen */     {lcpPap, 0,1,0, sprmPDyaBefore, dyaParaOpen  },

		};
#endif /* RGLCD */




#ifdef MPILCDBCM
/* NOTICE: must be kept in same order as other lcd tables! */
/* ALSO NOTICE:  if you add to this table, adjust the ilcdMax values that
	follow. */
csconst int mpilcdbcm [] =
		{
		bcmBold,
		bcmItalic,
		bcmSmallCaps,
		bcmULine,
		bcmWULine,
		bcmDULine,
		bcmSuperscript,
		bcmSubscript,

		bcmHideText,
		bcmPlainText,
		bcmNil,
		bcmNil,
		bcmNil,

		bcmParaLeft,
		bcmParaCenter,
		bcmParaRight,
		bcmParaBoth,
		bcmSpace1,
		bcmSpace15,
		bcmSpace2,
		bcmParaClose,
		bcmParaOpen,
		bcmParaNormal,

		bcmIndent,
		bcmUnIndent,
		bcmHangingIndent,
		bcmUnHang
		};
#endif /* MPILCDBCM */

#define ilcdMaxSearch 23
#define ilcdMaxStyle 27

