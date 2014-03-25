/* profile.n - include file for profiling and measurement aids */

#ifdef DEBUG


/* following structure stores information about a keypress from the
	time that it is pulled off the queue until the time that it is procesed */

struct PKE {    /* Pending Keyboard Event */
		DWORD   time;
		int     kc;
		};


struct KDL {    /* Key delay log */
		int     kc;
		WORD    dtime;
		int     fLastInUpdate: 1;
		int     rst: 2;
		int     : 13;
		};

#ifdef PROFDEF
#define PROFEXTERN
#else
#define PROFEXTERN extern
#endif

/* Key delays are logged in an array */

#define ikdlMax  100
PROFEXTERN struct KDL rgkdl[ikdlMax];
PROFEXTERN int ikdlMac;

/* Pending keyboard events are stacked in an array */

#define ipkeMax 20

PROFEXTERN struct PKE rgpke[ipkeMax];
PROFEXTERN int ipkeMac;
PROFEXTERN int ipkeMacMax;



/* Response Types */

#define rstIdle         0
#define rstCpBad        1
#define rstFastPath     2



#endif  /* DEBUG */
