/* D I A L O G 4 . C */
/* Rare or special purpose dialog functions used by style, save and macro
	dialogs */


#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "debug.h"
#include "error.h"
#include "doc.h"
#include "disp.h"
#include "ch.h"
#include "resource.h"
#include "wininfo.h"
#include "opuscmd.h"
#include "cmdtbl.h"
#include "menu2.h"
#include "version.h"
#include "screen.h"
#include "idd.h"
#include "prompt.h"
#include "message.h"
#include "doslib.h"
#include "keys.h"
#include "rareflag.h"
#include "props.h"
#include "format.h"
#include "sel.h"
#include "inter.h"
#include "style.h"

#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#define chPath ('\\')

#define dttmNinch (0)
#define dttmError (1)

extern struct SCI  vsci;
extern struct SEL  selCur;
extern int vfShowAllStd;
extern char (**vhmpiststcp)[];
extern int vstcBackup;
extern int vdocStsh;
extern int vistLbMac;
extern struct MERR  vmerr;
extern BOOL fElActive;

/*****************************************/
