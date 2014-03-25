#define rgicmteTblSize  499             /* Hash Table Size */
#define ccmte           100             /* dynamic memory block size */
										/* must be multiple of sizeof(int) */
#define ParHc           long
#define SizeParHc       (8*sizeof(ParHc))
#define AbsMask         ((((long) 1) << SizeParHc-1) - 1)
#define cbcmte          (sizeof(struct Cmte))

#define hcmteMemNull 0
#define icmteNull (-1)
#define TRUE 1
#define FALSE 0

struct Cmte {                           /* Compare Table Entry structure */
		ParHc   parhc;
		int     icmteNext;
} ;

#ifdef REVMARKING

#define irmBarNone    0
#define irmBarLeft    1
#define irmBarRight   2
#define irmBarOutside 3

/*
* WARNING...if you change the numbers here, you must change the jump table
*  in formatn.asm
*/
#define irmPropsNone       0
#define irmPropsBold       1
#define irmPropsItalic     2
#define irmPropsUnder      3
#define irmPropsDblUnder   4

#endif /* REVMARKING */

#define rrmNil    0
#define rrmRemove 1
#define rrmUndo   2
#define rrmDelete 3
#define rrmStripHidden  4
