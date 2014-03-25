
/* debugrep.c */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "props.h"
#include "sel.h"
#include "doc.h"
#include "debug.h"



#ifdef PROTOTYPE
#include "debugrep.cpt"
#endif /* PROTOTYPE */

#ifdef ENABLE

#define CBRUNMAX    256     /* maximum size of a run to be looked at */


extern struct DOD       **mpdochdod[];
extern CP               vcpFetch;
extern int              vdocFetch;
extern int              vccpFetch;
extern char HUGE        *vhpchFetch;
extern struct CHP       vchpFetch;
extern BOOL             vfEndFetch;
extern struct DBS       vdbs;






/* T E S T  R E P L A C E C P S */
/* check operation of ReplaceCps() */

/* %%Function:TestReplaceCps %%Owner:NOTUSED */
TestReplaceCps()
{
	int i, j;
	int docSrc;
	struct DOD **hdodSrc;
	int docDst;
	struct DOD **hdodDst;
	int docRef;
	struct DOD **hdodRef;
	struct PLC **hplcpcd;
	struct PCD *hpcd;
	int fnSrc;
	char **hRun;
	CP vcpFetchDst;
	int vccpFetchDst;
	char *vpchFetchDst;
	struct CHP vchpFetchDst;
	BOOL vfEndFetchDst;
	char *pRunDst;
	char HUGE *hpRunRef;
	CP cpSrc;
	CP cpIncSrc;
	CP cpDst;
	CP cpIncDst;
	CP cpFirstDst;
	CP cpLimDst;
	CP cpFirstRef;
	CP cpLimRef;
	CP cp;
	int cbCheck;
	int grpfScribbleSav;
	struct CA caT1, caT2;

	/* enable scribbling in character position #3 */
	grpfScribbleSav = vdbs.grpfScribble;
	vdbs.grpfScribble |= 0x0008;

	/* open the documents without allocating any windows */
	docSrc = DocOpenStDof(StShared("debugrep.src"), dofNoWindow);

	hdodSrc = mpdochdod[docSrc];
	docDst = DocOpenStDof(StShared("debugrep.dst"), dofNoWindow);

	hdodDst = mpdochdod[docDst];
	docRef = DocOpenStDof(StShared("debugrep.dst"), dofNoWindow);

	hdodRef = mpdochdod[docRef];

	/* allocate some space to store a run */
	hRun = (char **)HAllocateCw(CwFromCch(CBRUNMAX));
	Assert(hRun != hNil);

	/* munge the destination document's piece table */
	cpIncSrc = (*((*hdodSrc)->hplcpcd))->rgcp[(*((*hdodSrc)->hplcpcd))->ipcdMac] / 100;
	cpIncDst = (*((*hdodDst)->hplcpcd))->rgcp[(*((*hdodDst)->hplcpcd))->ipcdMac] / 100;
	for (i = 1, cpSrc = cpIncSrc, cpDst = 0; i < 100; i++)
		{
		ScribbleForTest('1');
		FReplaceCps(PcaSetDcp(&caT1, docDst, cpDst, (CP)0), PcaSetDcp(&caT2, docSrc, cpSrc, cpIncSrc));
		cpSrc += cpIncSrc;
		cpDst += cpIncDst;
		}

	fnSrc = (*hdodSrc)->fn;
	hplcpcd = (*hdodDst)->hplcpcd;
	for (i=0; i < (*hplcpcd)->ipcdMac;)
		{
		ScribbleForTest('2');
		hpcd = PInPlc(*hplcpcd, i);
		if (hpcd->fn == fnSrc)
			{
			FReplaceCps(PcaSetDcp(&caT1, docDst, CpPlc(hplcpcd,i),
					CpPlc(hplcpcd,i+1) - CpPlc(hplcpcd,i)),
					PcaSet(&caT2, docNil, (CP)0, (CP)0));
			i = 0;
			}
		else
			i++;
		}


	/* get cpFirst and cpLim for both the destination and the reference
		* files and make sure both pairs match */
	cpFirstDst = (*((*hdodDst)->hplcpcd))->rgcp[0];
	cpLimDst = (*((*hdodDst)->hplcpcd))->rgcp[(*((*hdodDst)->hplcpcd))->ipcdMac];
	cpFirstRef = (*((*hdodRef)->hplcpcd))->rgcp[0];
	cpLimRef = (*((*hdodRef)->hplcpcd))->rgcp[(*((*hdodRef)->hplcpcd))->ipcdMac];
	Assert(cpFirstDst == cpFirstRef);
	Assert(cpLimDst == cpLimRef);

	/* check each generated run against the original document */
	for (cp = cpFirstDst; cp < cpLimDst;)
		{
		ScribbleForTest('3');

		/* get a run from the destination doc and save it */
		CachePara(docDst, cp);
		FetchCp(docDst, cp, fcmChars | fcmProps | fcmParseCaps);
		Assert(vccpFetch <= CBRUNMAX);
		vcpFetchDst  = vcpFetch;
		vccpFetchDst  = vccpFetch;
		vchpFetchDst  = vchpFetch;
		vfEndFetchDst = vfEndFetch;
		bltbh(vhpchFetch, *hRun, vccpFetch);

		/* get a matching run from the reference doc */
		CachePara(docRef, cp);
		FetchCp(docRef, cp, fcmChars | fcmProps | fcmParseCaps);

		/* check to make sure the runs are the same */
		pRunDst = *hRun + (vcpFetchDst - cp);
		hpRunRef = vhpchFetch + (vcpFetch - cp);
		cbCheck = (int)CpMin((CP)vccpFetchDst - (vcpFetchDst - cp),
				(CP)vccpFetch - (vcpFetch - cp));
		Assert(cbCheck > 0);
		if (FNeHprgch(HpFromPch(pRunDst), hpRunRef, cbCheck))
			{
			Assert(fFalse);
			break;      /* if found different bytes then clean-up and exit */
			}
		if (FNeRgch(&vchpFetchDst, &vchpFetch, cbCHP))
			{
			Assert(fFalse);
			break;      /* if found different bytes then clean-up and exit */
			}

		cp += cbCheck;
		}

	/* check to be sure the docs are still viable */
	CkDoc(docSrc);
	CkDoc(docDst);
	CkDoc(docRef);

	/* clean up the mess we generated */
	DisposeDoc(docSrc);
	DisposeDoc(docDst);
	DisposeDoc(docRef);
	FreePh(&hRun);
	Scribble(3,' ');
	vdbs.grpfScribble = grpfScribbleSav;
}




/* %%Function:ScribbleForTest %%Owner:NOTUSED */
ScribbleForTest(ch)
char ch;
{
	static BOOL fState = fFalse;

	if (fState)
		Scribble(ispTest, ch);
	else
		Scribble(ispTest,' ');
	fState = !fState;
}



#endif  /* ENABLE */
