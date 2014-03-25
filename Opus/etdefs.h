/* etdefs.h */

/* Limits defined by HM Co */

#define cchMaxETString      1060    /* Longest synonym/definition string */
#ifdef SA_THES
#define cchMaxETWord	    34
#else
#define cchMaxETWord	    26	    /* Longest word that can be looked up */
#endif
#define idefMax             26      /* Max # definitions for one word */

#define chEndDef            '.'     /* Terminates a single definition */
#define chEndSyn            ','     /* Terminates a single synonym */
#define chEndSynAlt         '/'     /* Separates similar synonyms */
