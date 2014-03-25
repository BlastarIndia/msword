/* tmc.h */

/* This file contains tmc's that are shared by several dialogs.  If you need
to add one, make sure you click the Import check box in the dialog editor
dfor the control with the tmc. */


#ifdef SEARCHTMC
/* tmc's exported to dialog descriptions. */
#define tmcSearchBanter		(tmcUserMin + 0)
#define tmcWholeWord		(tmcUserMin + 1)
#define tmcReplace		(tmcUserMin + 2)
#define tmcReplaceBanter	(tmcUserMin + 3)

#define tmcSearchNil		(-1)
#endif /* SEARCHTMC */


#define tmcNONew  (tmcUserMin + 4)
#define tmcNOOpen (tmcUserMin + 5)

#define tmcSetDesc (tmcUserMin)
#define tmcDesc    (tmcUserMin + 1)


#define tmcCmdKc 103
