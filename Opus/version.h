/* R E L E A S E . H */

/*  This file contains information specific to this version of OPUS */

/* Product version number */
#define nMajorProduct 1
#define nMinorProduct 1
#define szVersionNum "1.1a"

/* Version history:
 * nMajorProduct	nMinorProduct
 *		1				0				Win Word 1.0 (Nov 1989)
 *		1				1				Win Word 1.1 (Jun 1990)
 */

/* ranges:
    nMajorProduct 1..7      - major rev number
    nMinorProduct 0..63     - update number
    nRevProduct 1..63       - testing release number
    nIncrProduct (0 or 1)   - 0 if real testing release, else 1
*/
#define nProductCurrent (((nMajorProduct&0x07)<<13)+((nMinorProduct&0x3f)<<7)+\
                        ((nRevProduct&0x3f)<<1)+(nIncrProduct?1:0))

#define szAppDef        "Microsoft Word"
#define stAppDef	"\016Microsoft Word"

#ifdef MKTGPRVW
extern CHAR szAppTitle[];
#define szAppTitleDef       "Microsoft Word (Preview)"
#else
#ifdef DEMO
extern CHAR szAppTitle[];
#define szAppTitleDef       "Microsoft Word (Demo)"
#define cpMaxDemo 			7000
#else
#define szAppTitle          szApp
#endif /* DEMO */
#endif /* MKTGPRVW */

#define szAppStartDef   "Microsoft Word for Windows"
#define szCopyrightDef	"Copyright © 1989-1990 Microsoft Corporation."
#define szCopyright2Def "Electronic Thesaurus © 1987 HMCo.  All rights reserved."

/* generated files define version and date information */
#include "verdate.h"

/* determines the width of the startup dialog */
#define cchSizeStartup ((sizeof(szCopyright2Def) > sizeof(szVerDateDef) ? \
                        sizeof(szCopyright2Def) : sizeof(szVerDateDef)) + 4)

/* Locale number.  This should be different for each different localized
   version.  It will allow us to know when a file is from a different
   locale. */
#define nLocaleDef               2  /* Our version */
#define nLocaleUS                2  /* US version */

#ifdef LOCALETEST  /* used to test locale translator */
#undef nLocaleDef
#define nLocaleDef 1 /* special test version */
#endif
