/* D E B U G C M D . C */
#ifdef DEBUG

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "heap.h"
#include "props.h"
#include "format.h"
#include "doc.h"
#include "sel.h"
#include "file.h"
#include "disp.h"
#include "prm.h"
#include "fkp.h"
#include "screen.h"
#include "scc.h"
#include "error.h"
#include "doslib.h"
#include "debug.h"
#include "message.h"

#include "opuscmd.h"


#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#include "dbgscrbl.hs"
#include "dbgscrbl.sdm"
#include "dbgtests.hs"
#include "dbgtests.sdm"
#include "dbgfunc.hs"
#include "dbgfunc.sdm"
#include "dbgpref.hs"
#include "dbgpref.sdm"
#include "dbgrare.hs"
#include "dbgrare.sdm"
#include "dbgfail.hs"
#include "dbgfail.sdm"
#include "dbgmem.hs"
#include "dbgmem.sdm"
#include "dbgusec1.hs"
#include "dbgusec1.sdm"
#include "dbgusec2.hs"
#include "dbgusec2.sdm"
#include "dbgusec3.hs"
#include "dbgusec3.sdm"
#include "dbgusec4.hs"
#include "dbgusec4.sdm"

/* G L O B A L S */

extern int              wwCur;
extern struct WWD       **hwwdCur;
extern struct LLC       vllc;
extern struct DBS       vdbs;
extern struct SCC       *vpsccBelow;
extern struct BMI       vbmiEmpty;
extern struct PREF      vpref;

extern struct SCC              *vpsccSave;
extern struct BPTB    vbptbExt;

#define cbMinFree 0xFFFF   /* lots of free bytes: unsigned long */

/*  C M D   D O   T E S T S  */
CMD CmdDoTests(pcmb)
CMB * pcmb;
{
	StartLongOp();
	DoDebugTests(fFalse);
	EndLongOp(fFalse);
	IdMessageBoxMstRgwMb(mstDebugTestComplete, NULL, MB_OK);
	return cmdOK;
}


/* C M D   R E F R E S H   S P E E D */
/* %%Function:CmdRefreshSpeed %%Owner:BRADV */
CMD CmdRefreshSpeed(pcmb)
CMB * pcmb;
	{/*
	DESCRIPTION:
	Display the current screen 10 times (for timing purposes).
*/
	int i;
	Scribble(ispRefreshSpeed,'X');
	for (i=0; i<10; i++)
		{
		/* where is TrashWw defined? */
		TrashWw(wwCur);
		UpdateWw(wwCur, fFalse);
		}
	Scribble(ispRefreshSpeed,'W');
/* check if fields in wwd are consistent */
	CkWw(wwCur);
	Scribble(ispRefreshSpeed,' ');

	return cmdOK;
}       /* CmdRefreshSpeed */


/* *** Test Menu procedures
*/


#define iCkBoxScribbleMax (sizeof(CABSCRIBBLE) - \
				(sizeof(CABH) + sizeof(WORD))) / sizeof(int)
typedef union
{
	CABSCRIBBLE	cabscribble;
	struct
		{
		CABH	cabh;
		WORD	sab;
		int	rgfScribbleFlags[iCkBoxScribbleMax];
		};
	} CABOVRSCRIBBLE;

/* %%Function:CmdScribble %%Owner:BRADV */
CMD CmdScribble(pcmb)
CMB * pcmb;
{
	int iCkBox;
	CABOVRSCRIBBLE * pcab;

	if (FCmdFillCab())
		{
		pcab = *pcmb->hcab;

		for (iCkBox = 0; iCkBox < iCkBoxScribbleMax; ++iCkBox)
			pcab->rgfScribbleFlags[iCkBox] =
					(vdbs.grpfScribble >> iCkBox) & 1;
		}

	if (FCmdDialog())
		{
		CHAR	dlt[sizeof(dltScribble)];

		BltDlt(dltScribble, dlt);
		switch (TmcOurDoDlg(dlt, pcmb))
			{

		case tmcError:
			return cmdError;

		case tmcCancel:
			return cmdCancelled;

		case tmcOK:
			break;
			}
		}

	if (FCmdAction())
		{
		pcab = *pcmb->hcab;

		vdbs.grpfScribble = 0;
		for (iCkBox = 0; iCkBox < iCkBoxScribbleMax; ++iCkBox)
			{
			vdbs.grpfScribble += 
					pcab->rgfScribbleFlags[iCkBox] << iCkBox;
			}

		WriteDebugStateInfo();
		}

	return cmdOK;
}


/* C M D   E N A B L E   T E S T S
enables or disables all check routines.
*/

#define iCkBoxEnTestsMax (sizeof(CABENABLETESTS) - \
				(sizeof(CABH) + sizeof(WORD))) / sizeof(int)
typedef union
{
	CABENABLETESTS	cabenabletests;
	struct
		{
		CABH	cabh;
		WORD	sab;
		int	rgfTestsFlags[iCkBoxEnTestsMax];
		};
	} CABOVRENABLETESTS;

/* %%Function:CmdEnableTests %%Owner:BRADV */
CMD CmdEnableTests(pcmb)
CMB * pcmb;
{
	CABOVRENABLETESTS *pcab;
	char *pf;
	int iCkBox;

	if (FCmdFillCab())
		{
		pcab = *pcmb->hcab;

		for (iCkBox = 0, pf = pchTestMin;
				iCkBox < iCkBoxEnTestsMax;
				iCkBox++, pf++)
			pcab->rgfTestsFlags[iCkBox] = *pf;
		}

	if (FCmdDialog())
		{
		CHAR	dlt[sizeof(dltEnableTests)];

		BltDlt(dltEnableTests, dlt);
		switch (TmcOurDoDlg(dlt, pcmb))
			{
		case tmcError:
			return cmdError;

		case tmcCancel:
			return cmdCancelled;

		case tmcOK:
			break;
			}
		}

	if (pcmb->fAction)
		{
		extern BOOL fShakeHeap, fCheckHeap, fFillBlock;

		/* get the stuff out of the cab */
		pcab = *pcmb->hcab;
		for (iCkBox = 0, pf = pchTestMin;
				iCkBox < iCkBoxEnTestsMax;
				iCkBox++, pf++)
			*pf = pcab->rgfTestsFlags[iCkBox];

		WriteDebugStateInfo();

		/* set the lmem heap flags */
		fShakeHeap = vdbs.fShakeHeap;
		fCheckHeap = vdbs.fCkHeap;
		fFillBlock = !vdbs.fNoFillBlock;
		}

	return cmdOK;
} /* end CmdEnableTests */


/* C M D   U S E   C  1  V E R S I O N S
enables or disables assembler versions of routines.
*/

#define iCkBoxC1VersionsMax (sizeof(CABUSEC1VERSIONS) - \
				(sizeof(CABH) + sizeof(WORD))) / sizeof(int)
typedef union
{
	CABUSEC1VERSIONS cabusec1versions;
	struct
		{
		CABH	cabh;
		WORD	sab;
		int	rgfUseCFlags[iCkBoxC1VersionsMax];
		};
	} CABOVRUSEC1VERSIONS;

/* %%Function:CmdUseC1Versions %%Owner:BRADV */
CMD CmdUseC1Versions(pcmb)
CMB * pcmb;
{
	CABOVRUSEC1VERSIONS *pcab;
	char *pf;
	int iCkBox;
	int fT;

	if (FCmdFillCab())
		{
		pcab = *pcmb->hcab;

		for (iCkBox = 0, pf = pchUseCMin;
				iCkBox < iCkBoxC1VersionsMax;
				iCkBox++, pf++)
			{
			fT = (*pf & 3);
			pcab->rgfUseCFlags[iCkBox] = (fT == 2 ? -1 : fT);
			}
		}

	if (FCmdDialog())
		{
		CHAR	dlt[sizeof(dltUseC1Versions)];

		BltDlt(dltUseC1Versions, dlt);
		switch (TmcOurDoDlg(dlt, pcmb))
			{
		case tmcError:
			return cmdError;

		case tmcCancel:
			return cmdCancelled;

		case tmcOK:
			break;
			}
		}

	if (pcmb->fAction)
		{
		extern BOOL fShakeHeap, fCheckHeap, fFillBlock;

		/* get the stuff out of the cab */
		pcab = *pcmb->hcab;
		for (iCkBox = 0, pf = pchUseCMin; 
				iCkBox < iCkBoxC1VersionsMax;
				iCkBox++, pf++)
			{
			fT = pcab->rgfUseCFlags[iCkBox];
			if (fT == -1)
				fT = 2;
			*pf &= ~3;
			*pf |= fT;
			}

		WriteDebugStateInfo();

		/* set the lmem heap flags */
		fShakeHeap = vdbs.fShakeHeap;
		fCheckHeap = vdbs.fCkHeap;
		fFillBlock = !vdbs.fNoFillBlock;
		}

	return cmdOK;
} /* end CmdUseC1Versions */



/* %%Function:FDlgUseC1Versions %%Owner:BRADV */
BOOL FDlgUseC1Versions(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	if (dlm != dlmClick)
		return fTrue;
	SetTmcVal(tmc, ((wOld + 2) % 3) - 1);
	return fTrue;
}


/* C M D   U S E   C  2  V E R S I O N S
enables or disables assembler versions of routines.
*/

#define iCkBoxC2VersionsMax (sizeof(CABUSEC2VERSIONS) - \
				(sizeof(CABH) + sizeof(WORD))) / sizeof(int)
typedef union
{
	CABUSEC2VERSIONS cabusec2versions;
	struct
		{
		CABH	cabh;
		WORD	sab;
		int	rgfUseCFlags[iCkBoxC2VersionsMax];
		};
	} CABOVRUSEC2VERSIONS;

/* %%Function:CmdUseC2Versions %%Owner:BRADV */
CMD CmdUseC2Versions(pcmb)
CMB * pcmb;
{
	CABOVRUSEC2VERSIONS *pcab;
	char *pf;
	int iCkBox;
	int fT;

	if (FCmdFillCab())
		{
		pcab = *pcmb->hcab;

		for (iCkBox = 0, pf = pchUseCMin;
				iCkBox < iCkBoxC2VersionsMax;
				iCkBox++, pf++)
			{
			fT = ((*pf >> 2) & 3);
			pcab->rgfUseCFlags[iCkBox] = (fT == 2 ? -1 : fT);
			}
		}

	if (FCmdDialog())
		{
		CHAR	dlt[sizeof(dltUseC2Versions)];

		BltDlt(dltUseC2Versions, dlt);
		switch (TmcOurDoDlg(dlt, pcmb))
			{
		case tmcError:
			return cmdError;

		case tmcCancel:
			return cmdCancelled;

		case tmcOK:
			break;
			}
		}

	if (pcmb->fAction)
		{
		extern BOOL fShakeHeap, fCheckHeap, fFillBlock;

		/* get the stuff out of the cab */
		pcab = *pcmb->hcab;
		for (iCkBox = 0, pf = pchUseCMin;
				iCkBox < iCkBoxC2VersionsMax;
				iCkBox++, pf++)
			{
			fT = pcab->rgfUseCFlags[iCkBox];
			if (fT == -1)
				fT = 2;
			*pf &= ~12;
			*pf |= (fT << 2);
			}

		WriteDebugStateInfo();

		/* set the lmem heap flags */
		fShakeHeap = vdbs.fShakeHeap;
		fCheckHeap = vdbs.fCkHeap;
		fFillBlock = !vdbs.fNoFillBlock;
		}

	return cmdOK;
} /* end CmdUseC2Versions */



/* %%Function:FDlgUseC2Versions %%Owner:BRADV */
BOOL FDlgUseC2Versions(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	if (dlm != dlmClick)
		return fTrue;
	SetTmcVal(tmc, ((wOld + 2) % 3) - 1);
	return fTrue;
}


/* C M D   U S E   C  3  V E R S I O N S
enables or disables assembler versions of routines.
*/

#define iCkBoxC3VersionsMax (sizeof(CABUSEC3VERSIONS) - \
				(sizeof(CABH) + sizeof(WORD))) / sizeof(int)
typedef union
{
	CABUSEC3VERSIONS cabusec3versions;
	struct
		{
		CABH	cabh;
		WORD	sab;
		int	rgfUseCFlags[iCkBoxC3VersionsMax];
		};
	} CABOVRUSEC3VERSIONS;

/* %%Function:CmdUseC3Versions %%Owner:BRADV */
CMD CmdUseC3Versions(pcmb)
CMB * pcmb;
{
	CABOVRUSEC3VERSIONS *pcab;
	char *pf;
	int iCkBox;
	int fT;

	if (FCmdFillCab())
		{
		pcab = *pcmb->hcab;

		for (iCkBox = 0, pf = pchUseCMin;
				iCkBox < iCkBoxC3VersionsMax;
				iCkBox++, pf++)
			{
			fT = ((*pf >> 4) & 3);
			pcab->rgfUseCFlags[iCkBox] = (fT == 2 ? -1 : fT);
			}
		}

	if (FCmdDialog())
		{
		CHAR	dlt[sizeof(dltUseC3Versions)];

		BltDlt(dltUseC3Versions, dlt);
		switch (TmcOurDoDlg(dlt, pcmb))
			{
		case tmcError:
			return cmdError;

		case tmcCancel:
			return cmdCancelled;

		case tmcOK:
			break;
			}
		}

	if (pcmb->fAction)
		{
		extern BOOL fShakeHeap, fCheckHeap, fFillBlock;

		/* get the stuff out of the cab */
		pcab = *pcmb->hcab;
		for (iCkBox = 0, pf = pchUseCMin;
				iCkBox < iCkBoxC3VersionsMax;
				iCkBox++, pf++)
			{
			fT = pcab->rgfUseCFlags[iCkBox];
			if (fT == -1)
				fT = 2;
			*pf &= ~48;
			*pf |= (fT << 4);
			}

		WriteDebugStateInfo();

		/* set the lmem heap flags */
		fShakeHeap = vdbs.fShakeHeap;
		fCheckHeap = vdbs.fCkHeap;
		fFillBlock = !vdbs.fNoFillBlock;
		}

	return cmdOK;
} /* end CmdUseC3Versions */



/* %%Function:FDlgUseC3Versions %%Owner:BRADV */
BOOL FDlgUseC3Versions(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	if (dlm != dlmClick)
		return fTrue;
	SetTmcVal(tmc, ((wOld + 2) % 3) - 1);
	return fTrue;
}


/* C M D   U S E   C  4  V E R S I O N S
enables or disables assembler versions of routines.
*/

#define iCkBoxC4VersionsMax (sizeof(CABUSEC4VERSIONS) - \
				(sizeof(CABH) + sizeof(WORD))) / sizeof(int)
typedef union
{
	CABUSEC4VERSIONS cabusec4versions;
	struct
		{
		CABH	cabh;
		WORD	sab;
		int	rgfUseCFlags[iCkBoxC4VersionsMax];
		};
	} CABOVRUSEC4VERSIONS;

/* %%Function:CmdUseC4Versions %%Owner:BRADV */
CMD CmdUseC4Versions(pcmb)
CMB * pcmb;
{
	CABOVRUSEC4VERSIONS *pcab;
	char *pf;
	int iCkBox;
	int fT;

	if (FCmdFillCab())
		{
		pcab = *pcmb->hcab;

		for (iCkBox = 0, pf = pchUseCMin;
				iCkBox < iCkBoxC4VersionsMax;
				iCkBox++, pf++)
			{
			fT = ((*pf >> 6) & 3);
			pcab->rgfUseCFlags[iCkBox] = (fT == 2 ? -1 : fT);
			}
		}

	if (FCmdDialog())
		{
		CHAR	dlt[sizeof(dltUseC4Versions)];

		BltDlt(dltUseC4Versions, dlt);
		switch (TmcOurDoDlg(dlt, pcmb))
			{
		case tmcError:
			return cmdError;

		case tmcCancel:
			return cmdCancelled;

		case tmcOK:
			break;
			}
		}

	if (pcmb->fAction)
		{
		extern BOOL fShakeHeap, fCheckHeap, fFillBlock;

		/* get the stuff out of the cab */
		pcab = *pcmb->hcab;
		for (iCkBox = 0, pf = pchUseCMin;
				iCkBox < iCkBoxC4VersionsMax;
				iCkBox++, pf++)
			{
			fT = pcab->rgfUseCFlags[iCkBox];
			if (fT == -1)
				fT = 2;
			*pf &= ~192;
			*pf |= (fT << 6);
			}

		WriteDebugStateInfo();

		/* set the lmem heap flags */
		fShakeHeap = vdbs.fShakeHeap;
		fCheckHeap = vdbs.fCkHeap;
		fFillBlock = !vdbs.fNoFillBlock;
		}

	return cmdOK;
} /* end CmdUseC4Versions */



/* %%Function:FDlgUseC4Versions %%Owner:BRADV */
BOOL FDlgUseC4Versions(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wNew, wOld, wParam;
{
	if (dlm != dlmClick)
		return fTrue;
	SetTmcVal(tmc, ((wOld + 2) % 3) - 1);
	return fTrue;
}


/* C M D  D E B U G  P R E F S */
/* enables or disables user debug preferences.
*/

#define iCkBoxPrefsMax (sizeof(CABDBGPREF) - \
				(sizeof(CABH) + sizeof(WORD))) / sizeof(int)
typedef union
{
	CABDBGPREF	cabdbgpref;
	struct
		{
		CABH	cabh;
		WORD	sab;
		int	rgfPrefsFlags[iCkBoxPrefsMax];
		};
	} CABOVRDBGPREF;

/* %%Function:CmdDebugPrefs %%Owner:BRADV */
CMD CmdDebugPrefs(pcmb)
CMB * pcmb;
{

	CABOVRDBGPREF *pcab;
	char *pf;
	int iCkBox;

	if (FCmdFillCab())
		{
		pcab = *pcmb->hcab;
		for (iCkBox = 0, pf = pchPrefMin;
				iCkBox < iCkBoxPrefsMax;
				iCkBox++, pf++)
			pcab->rgfPrefsFlags[iCkBox] = *pf;
		}

	if (FCmdDialog())
		{
		CHAR	dlt[sizeof(dltDebugPrefs)];

		BltDlt(dltDebugPrefs, dlt);
		switch (TmcOurDoDlg(dlt, pcmb))
			{
		case tmcError:
			return cmdError;

		case tmcCancel:
			return cmdCancelled;

		case tmcOK:
			break;
			}
		}

	if (pcmb->fAction)
		{
		/* get the stuff out of the cab */
		pcab = *pcmb->hcab;
		for (iCkBox = 0, pf = pchPrefMin;
				iCkBox < iCkBoxPrefsMax;
				iCkBox++, pf++)
			*pf = pcab->rgfPrefsFlags[iCkBox];

		WriteDebugStateInfo();
		}

	return cmdOK;
} /* end CmdEnableTests */


/* C M D   T E S T   F U N C T I O N
enables or disables all check routines.
*/

#define iCkBoxTestFuncMax (sizeof(CABTESTFUNC) - \
				(sizeof(CABH) + sizeof(WORD))) / sizeof(int)
typedef union
{
	CABTESTFUNC	cabTESTFUNC;
	struct
		{
		CABH	cabh;
		WORD	sab;
		int	rgfTestFunc[iCkBoxPrefsMax];
		};
	} CABOVRTESTFUNC;

/* %%Function:CmdTestFunction %%Owner:BRADV */
CMD CmdTestFunction(pcmb)
CMB * pcmb;
{
	CABOVRTESTFUNC	*pcab;
	int tmc;

	if (FCmdFillCab())
		{
		int	i;

		pcab = *pcmb->hcab;
		for (i = 0; i < iCkBoxTestFuncMax; i++)
			{
			pcab->rgfTestFunc[i] = 0;
			}
		}

	if (FCmdDialog())
		{
		CHAR	dlt[sizeof(dltTestFunction)];

		/* do the dialog */
		BltDlt(dltTestFunction, dlt);
		tmc = TmcOurDoDlg(dlt, pcmb);
		}

	return cmdOK;
}


/**
**
**  FDlgTestFunction
**
**  Dialog box function for "Test Function" dialog box.
**
**/

/* %%Function:FDlgTestFunction %%Owner:BRADV */
BOOL FDlgTestFunction(dlm, tmc, wNew, wOld, wParam)
DLM	dlm;
TMC	tmc;
WORD	wNew, wOld, wParam;
{

	switch (dlm)
		{
	case dlmClick:
		switch (tmc)
			{
		case tmcTestScc:
			FExecCmd(bcmTestScc);
			break;
		case tmcTestPlc:
			FExecCmd(bcmTestPlc);
			break;
		case tmcTestPl:
			FExecCmd(bcmTestPl);
			break;
		case tmcTestSttb:
			FExecCmd(bcmTestSttb);
			break;
		case tmcGlobCompact:
			FExecCmd(bcmGlobalCompact);
			break;
			}
		break;
		}
	return (fTrue);
}



/* %%Function:CmdGlobalCompact %%Owner:BRADV */
CmdGlobalCompact()
{
	GlobalCompact((DWORD) cbMinFree);
}


/* %%Function:TestA %%Owner:BRADV */
TestA()
{
} /* end TestA */


/* %%Function:TestB %%Owner:BRADV */
TestB()
{
} /* end TestB */


/* %%Function:TestC %%Owner:BRADV */
TestC()
{
} /* end TestC */



/**
**
**  FDlgRareEvents
**
**  Dialog box function for the Rare Events dialog box.
**
**/

/* %%Function:FDlgRareEvents %%Owner:BRADV */
BOOL	FDlgRareEvents(dlm, tmc, wNew, wOld, wParam)
DLM	dlm;
TMC	tmc;
WORD	wNew, wOld, wParam;
{
	int i;

	switch (dlm)
		{
	case dlmTerm:
		if (tmc == tmcOK)
			{
			/* check all edit items for legal values before
				exiting */
			for (i = tmcEventNum1; i <= tmcNowAt10; i++)
				{
				if (ValGetTmc(i) == wError)
					{
					return fFalse;
					}
				}
			}
		break;
		}
	return(fTrue);
}


/* %%Function:CmdRareEvents %%Owner:BRADV */
CMD CmdRareEvents(pcmb)
CMB * pcmb;
{
	extern ARG_RAREEVENTS RareEvent[RareEventMax];

	CABDBGRARE	*pcabdbgrare;
	TMC tmc;
	int i;

	tmc = tmcOK;
	if (FCmdFillCab())
		{
		pcabdbgrare = (CABDBGRARE *) *pcmb->hcab;
		bltb(RareEvent, &(pcabdbgrare->evnt1),
				sizeof(ARG_RAREEVENTS) * RareEventMax);
		}

	if (FCmdDialog())
		{
		DLT	*pdlt;
		CHAR	dlt[sizeof(dltRareEvents)];

		BltDlt(dltRareEvents, dlt);

		/* do the dialog */
		tmc = TmcOurDoDlg(dlt, pcmb);
		}

	if (FCmdAction() && tmc == tmcOK)
		{
		/* get results from CAB */
		pcabdbgrare = (CABDBGRARE *) *pcmb->hcab;
		bltb(&(pcabdbgrare->evnt1), RareEvent,
				sizeof(ARG_RAREEVENTS) * RareEventMax);

		/* set all null string results from CAB to 0 */
		for (i=0; i < RareEventMax; i++)
			{
			if (RareEvent[i].wEventNumber == wNinch)
				RareEvent[i].wEventNumber = 0;
			if (RareEvent[i].wRepetitionsMac == wNinch)
				RareEvent[i].wRepetitionsMac = 0;
			if (RareEvent[i].wRepetitions == wNinch)
				RareEvent[i].wRepetitions = 0;
			}
		}

RareRet:
	return cmdOK;
}



/* %%Function:WriteDebugStateInfo %%Owner:BRADV */
WriteDebugStateInfo()
	{ /*
	DESCRIPTION:
	Write out debugging state information into resource file.
*/
	int fn, osfn;
	char sz[cchMaxFile];
	extern CHAR szApp[];
	extern CHAR szEmpty[];
	struct OFS ofs;

	vdbs.nDbsuVer = nDbsuVerCur;

	GetProfileString( (LPSTR) szApp, (LPSTR) SzFrame("dbs"),
			(LPSTR)SzFrame("opus.dbs"), (LPSTR) sz, cchMaxFile - 1 );

	if ((osfn = OpenFile(sz, &ofs, OF_CREATE+OF_READWRITE+bSHARE_DENYRDWR))
			>= 0)
		{
		if (CchWriteDoshnd(osfn,
				((char far *) &vdbs/*.dbsu*/),
				sizeof(struct DBSU)) > 0)
			{
			FCloseDoshnd(osfn);
			return;
			}
		}
	/* file access failure if here */
/*         ErrorEid(eidOpenError, " WriteDebugStateInfo"); */
	Beep();
	FCloseDoshnd(osfn);
}


/* R E A D  D E B U G  S T A T E  I N F O */
/* %%Function:ReadDebugStateInfo %%Owner:BRADV */
ReadDebugStateInfo()
{ /* Read various debugging state variables from resource file */
	int osfn;
	struct OFS ofs;
	char sz[cchMaxFile];
	extern CHAR szApp[];

	GetProfileString( (LPSTR) szApp, (LPSTR) SzFrame("dbs"),
			(LPSTR)SzFrame("opus.dbs"), (LPSTR) sz, cchMaxFile - 1 );

	SetBytes( &vdbs, 0, sizeof (struct DBS) );
	if ((osfn = OpenFile(sz, &ofs, 0)) >= 0)
		{
		if (CchReadDoshnd(osfn,
				((char far *) &vdbs/*.dbsu*/),
				sizeof(struct DBSU)) < 0)
			{
			/* file access failure if here */
/*                         ErrorEid(eidOpenError, " ReadDebugStateInfo"); */
			Beep();
			}
		else  if (vdbs.nDbsuVer != nDbsuVerCur)
			{
			SetBytes(&vdbs, 0, sizeof(struct DBSU));
			}
		FCloseDoshnd(osfn);
		}
}


/* %%Function:CmdScanAlerts %%Owner:BRADV */
CmdScanAlerts()
{
	return cmdOK;
} /* end CmdScanAlerts */


/* C M D   D B G   M E M O R Y */
/* %%Function:CmdDbgMemory %%Owner:BRADV */
CMD CmdDbgMemory(pcmb)
CMB * pcmb;
	{ /*
	DESRIPTION:
	Set the quantity of available heap space.
*/
	int tmc;
	CABDBGMEMORY *pcab;

	tmc = tmcOK;
	if (FCmdFillCab())
		{
		pcab  = (CABDBGMEMORY *) *pcmb->hcab;
		pcab->uSetTo = CbAvailHeap(sbDds);/* poor indicator */
		pcab->uCmd = vdbs.cbHeapWantedCmd;
		}

	if (FCmdDialog())
		{
		CHAR	dlt[sizeof(dltDbgMemory)];

		/* do the dialog */
		BltDlt(dltDbgMemory, dlt);
		tmc = TmcOurDoDlg(dlt, pcmb);
		if (tmc == tmcError)
			return cmdError;
		}

	if (FCmdAction())
		{
		switch (tmc)
			{
		case tmcDbgMemReset:
			FreePh(&vdbs.hDebug);
			break;
		case tmcOK:
			if ((int)pcab->uSetTo >= 0)
				SetHeapSize(pcab->uSetTo, &vdbs.hDebug);
			if ((int)pcab->uCmd >= 0)
				vdbs.cbHeapWantedCmd = pcab->uCmd;
			break;
			}
		}
	return cmdOK;

} /* end CmdDbgMemory */


/**
**
**  FDlgDbgMemory
**
**  Dialog box function for "Set Available Memory" dialog box.
**
**/

/* %%Function:FDlgDbgMemory %%Owner:BRADV */
BOOL	FDlgDbgMemory(dlm, tmc, wNew, wOld, wParam)
DLM	dlm;
TMC	tmc;
WORD	wNew, wOld, wParam;
{
	if (dlm == dlmInit)
		{
		int cbDebug = vdbs.hDebug ? CbOfH(vdbs.hDebug) : 0;
		char rgch[5];
		char *pch = rgch;
		CchIntToPpch(cbDebug, &pch);
		*pch = 0;
		SetTmcText(tmcDbgMemCurrFree, rgch);
		EnableTmc(tmcDbgMemReset, cbDebug>0);
		}
	return (fTrue);
}



/* C M D   E A T   M E M O R Y */
/* %%Function:CmdEatMemory %%Owner:BRADV */
CMD CmdEatMemory(pcmb)
CMB * pcmb;
	{ /*
	DESRIPTION:
	Reduce the quantity of available heap space.
*/
	int cbHeapFree;
	cbHeapFree = CbAvailHeap(sbDds); /* poor indicator */
	EatHeap (cbHeapFree > 20 ? cbHeapFree/2  : 5, &vdbs.hDebug);
	return cmdOK;
}       /* end CmdEatMemory */


/* C M D   F R E E   M E M O R Y */
/* %%Function:CmdFreeMemory %%Owner:BRADV */
CMD CmdFreeMemory(pcmb)
CMB * pcmb;
	{/*
	DESCRIPTION:
	Free up half the memory stolen by CmdEatMemory.
*/
	if (vdbs.hDebug == 0)
		Beep();
	else
		SetHDebugSize (CbOfH(vdbs.hDebug)/2, &vdbs.hDebug);
	return cmdOK;
}       /* end CmdFreeMemory */


/* S E T  H E A P  S I Z E */
/*  Adjust size of debugging heap object so that there are cb bytes of heap
	available.
*/

/* %%Function:SetHeapSize %%Owner:BRADV */
SetHeapSize(cb,ph)
int cb;
VOID ***ph;
{
	int cbHeapFree = CbAvailHeap(sbDds);
	EatHeap(cbHeapFree-cb, ph);
}


/* E A T  H E A P */
/*  Use up an additional cb bytes of heap space.  cb may be negative.
*/

/* %%Function:EatHeap %%Owner:BRADV */
EatHeap(cb, ph)
int cb;
VOID ***ph;
{
	SetHDebugSize (max(0, (*ph ? CbOfH (*ph) : 0) + cb), ph);
}


/* %%Function:SetHDebugSize %%Owner:BRADV */
SetHDebugSize(cb, ph)
int cb;
VOID ***ph;
{
	extern int vfAllocGuaranteed;
	vfAllocGuaranteed++;

	if (cb < 10)
		cb = 0;

	if (*ph)
		{
		AssertH(*ph);
		if (!cb)
			FreePh(ph);
		else
			FChngSizeHCb (*ph, cb, fTrue);
		}
	else if (cb)
		*ph = HAllocateCb(cb);

	ReportHeapSz (SzShared("now available"), 0);
	vfAllocGuaranteed--;
}




/* C M D  D E B U G  F A I L U R E S */
/* %%Function:CmdDebugFailures %%Owner:BRADV */
CMD CmdDebugFailures(pcmb)
CMB * pcmb;
{
	extern int vfAllocGuaranteed;
	CABDBGFAIL *pcab;

	EnterDebug();

	if (FCmdFillCab())
		{
		pcab = *pcmb->hcab;

		pcab->cLmemSucceed = vdbs.cLmemSucceed;
		pcab->cLmemFail = vdbs.cLmemFail;
		pcab->cWinSucceed = vdbs.cWinSucceed;
		pcab->cWinFail = vdbs.cWinFail;
		pcab->cBkgndSucceed = vdbs.cBkgndSucceed;
		pcab->cBkgndFail = vdbs.cBkgndFail;
		}

	if (FCmdDialog())
		{
		DLT	*pdlt;
		CHAR	dlt[sizeof(dltDbgFail)];

		BltDlt(dltDbgFail, dlt);
		switch (TmcOurDoDlg(dlt, pcmb))
			{
		case tmcError:
			ExitDebug();
			return cmdError;

		case tmcCancel:
			ExitDebug();
			return cmdCancelled;

		case tmcOK:
			break;
			}
		}

	if (FCmdAction())
		{
		pcab = *pcmb->hcab;

		vdbs.cLmemSucceed = max(0, pcab->cLmemSucceed);
		vdbs.cLmemFail = max(0, pcab->cLmemFail);
		vdbs.cWinSucceed = max(0, pcab->cWinSucceed);
		vdbs.cWinFail = max(0, pcab->cWinFail);
		vdbs.cBkgndSucceed = max(0, pcab->cBkgndSucceed);
		vdbs.cBkgndFail = max(0, pcab->cBkgndFail);
#ifdef DEBUG
		CommSzRgNum( 
				SzShared("Failure settings (memSuc,memFail,winSuc,winFail): "),
				&vdbs.cLmemSucceed, 4);
#endif /* DEBUG */
		}

	ExitDebug();

	return cmdOK;
}



/* %%Function:FDlgDbgFail %%Owner:BRADV */
BOOL	FDlgDbgFail(dlm, tmc, wNew, wOld, wParam)
DLM	dlm;
TMC	tmc;
WORD	wNew, wOld, wParam;
{
	if (dlm == dlmInit)
		{
		SetTmcText(tmcDbgFailCmdOnly, vdbs.fOutsideCmd ?
				SzFrame("Fail outside Cmd") : SzFrame("Fail in Cmd only"));
		}
	return (fTrue);
}



/*-----------------------------*/
/*  HEAP INFORMATION ROUTINES **/

#include "menu2.h"
#include "el.h"
#include "macrocmd.h"

#define cchBufMax  80

struct DHI				/* Debug Heap Information */
	{
	uns  cNearH;		/* Number of near handles */
	uns  cFarH;
	long lcbNear;		/* Temp sum of near heap space used */
	long lcbFar;
	long lcbNearTotal;	/* Total of near heap space used */
	long lcbFarTotal;
	int  cchBuf;		/* Current position in buffer */
	char szBuf[cchBufMax];		/* Output Buffer */
	};


/***************/
/* OutputBuf
*/

/* %%Function:OutputBuf %%Owner:RobD */
void OutputBuf(pdhi)
struct DHI *pdhi;
{
	if (pdhi->cchBuf != 0)
		{
		pdhi->szBuf[pdhi->cchBuf] = '\0';
		CommSz(pdhi->szBuf);
		CommSz(SzShared("\r\n"));
		pdhi->cchBuf = 0;
		}
}	/* OutputBuf */


/**************/
/* AddSzToBuf
*/

/* %%Function:AddSzToBuf %%Owner:RobD */
void AddSzToBuf(sz,pdhi)
char   *sz;
struct DHI *pdhi;
{
	if ((CchLenSz(sz)+pdhi->cchBuf) >= cchBufMax)
		OutputBuf(pdhi);
	pdhi->cchBuf += CchCopySz(sz,&(pdhi->szBuf[pdhi->cchBuf]));
}	/* AddSzToBuf */


/******************/
/* AddSzLongToBuf
*/

/* %%Function:AddSzLongToBuf %%Owner:RobD */
void AddSzLongToBuf(sz, lnum, pdhi)
char	*sz;
long	lnum;
struct DHI *pdhi;
{
	char szT[cchBufMax];
	char *pch = szT;

	pch += CchCopySz(sz,pch);
	CchLongToPpch(lnum, &pch);
	*pch = '\0';

	AddSzToBuf(szT, pdhi);
}	/* AddSzLongToBuf */



/*******************/
/* OutputSzNearFar
*/

/* %%Function:OutputSzNearFar %%Owner:RobD */
void OutputSzNearFar(sz,cbNear,lcbFar, pdhi)
char	*sz;
uns	cbNear;
long	lcbFar;
struct DHI *pdhi;
{
	if (vdbs.fVerboseHeapInfo)
		{
		char	szT[cchBufMax];
		char	*pch = szT;

		*pch++ = ' ';
		pch += CchCopySz(sz,pch);
		*pch++ = ':';
		CchUnsToPpch(cbNear,&pch);
		if (lcbFar != 0)
			{
			*pch++ = '/';
			CchLongToPpch(lcbFar,&pch);
			}
		*pch = '\0';
		AddSzToBuf(szT,pdhi);
		}
}	 /* OutputSzNearFar */


/**************/
/* SubTotalDhi
*/

/* %%Function:SubTotalDhi %%Owner:RobD */
void SubTotalDhi(pdhi)
struct DHI *pdhi;
{
	char	szT[cchBufMax];
	char	*pch = szT;

	if (vdbs.fVerboseHeapInfo)
		pch += CchCopySz("  [", pch);
	CchLongToPpch(pdhi->lcbNear, &pch);
	*pch++ = '/';
	CchLongToPpch(pdhi->lcbFar, &pch);
	if (vdbs.fVerboseHeapInfo)
		*pch++ = ']';
	*pch = '\0';
	AddSzToBuf(szT,pdhi);
	if (vdbs.fVerboseHeapInfo)
		OutputBuf(pdhi);

	pdhi->lcbNearTotal += pdhi->lcbNear;
	pdhi->lcbFarTotal  += pdhi->lcbFar;
	pdhi->lcbNear = pdhi->lcbFar = 0L;

}	/* SubTotalDhi */
	

/************/
/* AddCbOfH
*/

/* %%Function:AddCbOfH %%Owner:RobD */
void AddCbOfH(h, sz, pdhi)
uns	*h;
char	*sz;
struct DHI *pdhi;
{
	uns	cb;

	if (h != hNil)
		{
		pdhi->cNearH++;
		pdhi->lcbNear += cb = CbOfH(h);
		OutputSzNearFar(sz, cb, 0L, pdhi);
		}
}	/* AddCbOfH */


/**************/
/* AddCbOfHpl
*/

/* %%Function:AddCbOfHpl %%Owner:RobD */
void AddCbOfHpl(hplFoo, sz, pdhi)
struct	PL **hplFoo;
char 	*sz;
struct DHI *pdhi;
{
	uns  cbNear;
	long lcbFar = 0L;
	struct PL *pplFoo = *hplFoo;

	if (hplFoo != hNil)
		{
		pdhi->cNearH++;
		pdhi->lcbNear += cbNear = CbOfH(hplFoo);
		if (pplFoo->fExternal)
			{
			pdhi->cFarH++;
			pdhi->lcbFar += lcbFar = CbOfHq( (HQ *) (((char *)pplFoo)+pplFoo->brgfoo));
			}
		OutputSzNearFar(sz, cbNear, lcbFar, pdhi);
		}
}	/* AddCbOfHpl */


/***************/
/* AddCbOfHplc
*/

/* %%Function:AddCbOfHplc %%Owner:RobD */
void AddCbOfHplc(hplc, sz, pdhi)
struct	PLC **hplc;
char	*sz;
struct DHI *pdhi;
{
	uns  cbNear;
	long lcbFar = 0L;

	if (hplc != hNil)
		{
		pdhi->cNearH++;
		pdhi->lcbNear += cbNear = CbOfH(hplc);
		if (((*hplc)->fExternal) && (((*hplc)->hqplce) != hNil))
			{
			pdhi->cFarH++;
			pdhi->lcbFar += lcbFar = CbOfHq((*hplc)->hqplce);
			}
		OutputSzNearFar(sz,cbNear,lcbFar, pdhi);
		}
}	/* AddCbOfHplc */


/****************/
/* AddCbOfHsttb
*/

/* %%Function:AddCbOfHsttb %%Owner:RobD */
void AddCbOfHSttb(hsttb, sz, pdhi)
struct	STTB **hsttb;
char	*sz;
struct DHI *pdhi;
{
	uns  cbNear;
	long lcbFar = 0;

	if (hsttb != hNil)
		{
		pdhi->cNearH++;
		pdhi->lcbNear += cbNear = CbOfH(hsttb);
		if (((*hsttb)->fExternal) && (((*hsttb)->hqrgbst) != hNil))
			{
			pdhi->cFarH++;
			pdhi->lcbFar += lcbFar = CbOfHq((*hsttb)->hqrgbst);
			}
		OutputSzNearFar(sz,cbNear,lcbFar, pdhi);
		}
}	/* AddCbOfHsttb */


/*****************/
/* OutputDocType
*/

/* %%Function:OutputDocType %%Owner:RobD */
void OutputDocType(doc, pdhi)
int doc;
struct DHI *pdhi;
{
	extern struct SEL selCur;
	extern int	vdocTemp;
	extern int	vdocScratch;
	extern int	docGlobalDot;
	extern int	docGlsy;
	extern int	docDde;

	AddSzToBuf(" (", pdhi);

	switch (PdodDoc(doc)->dk)
		{
	case dkDoc:
		AddSzToBuf("doc", pdhi);
		break;
	case dkDot:
		AddSzToBuf("dot", pdhi);
		break;
	case dkGlsy:
		AddSzToBuf("glsy", pdhi);
		break;
	case dkAtn:
		AddSzToBuf("atn", pdhi);
		break;
	case dkMcr:
		AddSzToBuf("mcr", pdhi);
		break;
	case dkHdr:
		AddSzToBuf("hdr", pdhi);
		break;
	case dkFtn:
		AddSzToBuf("ftn", pdhi);
		break;
	case dkSDoc:
		AddSzToBuf("sdoc", pdhi);
		break;
	default:
		AddSzToBuf("???", pdhi);
		break;
		}

	if (doc < docMinNormal)
		switch (doc)
			{
		case docUndo:
			AddSzToBuf(" Undo", pdhi);
			break;
		case docNew:
			AddSzToBuf(" New", pdhi);
			break;
		case docScrap:
			AddSzToBuf(" Scrap", pdhi);
			break;
			}
	else  if (doc == vdocTemp)
		AddSzToBuf(" Temp", pdhi);
	else  if (doc == vdocScratch)
		AddSzToBuf(" Scratch", pdhi);
	else  if (doc == docDde)
		AddSzToBuf(" Dde", pdhi);
	else  if (doc == docGlobalDot || doc == docGlsy)	/*??*/
		AddSzToBuf(" Global", pdhi);
	else  if (doc == selCur.doc)
		AddSzToBuf(" SelCur", pdhi);

	AddSzToBuf(")", pdhi);

}	/* OutputDocType */



/*******************************/
/*  C M D   H E A P   I N F O  */
/*
** Outputs information about heap usage
**
** Format = abbr:near/far
**          [totalnear/totalfar]
**
** fVerboseHeapInfo controls amount of info displayed
**
*/

/* %%Function:CmdHeapInfo %%Owner:RobD */
CMD CmdHeapInfo (pcmb)
CMB * pcmb;
{
	extern int wwMac;
	extern int docMac;
	extern int fnMac;
	extern struct WWD ** mpwwhwwd[];
	extern struct DOD ** mpdochdod[];
	extern struct FCB ** mpfnhfcb[];
	extern KMP ** hkmpBase;
	extern KMP ** hkmpCur;
	extern KMP ** vhkmpUser;
	extern MUD ** vhmudUser;
	extern MUD ** vhmudBase;
	extern struct STTB ** hsttbMenu;
	extern struct STTB ** vhsttbOpen;
	extern struct STTB ** vhsttbWnd;
	extern char  **vhgrpchr;
	extern MES ** vhmes;

	struct DHI dhi;

	uns		ww;
	struct	WWD **hwwd;
	uns		doc;
	struct	DOD *pdod;
	uns		fn;
	struct	FCB	**hfcb;

	char	szT[16];
	char	*pch;


	dhi.cchBuf = 0;
	dhi.cNearH = dhi.cFarH = 0;
	dhi.lcbNear = dhi.lcbFar = dhi.lcbNearTotal = dhi.lcbFarTotal = 0L;

	CommSz(SzShared("\r\n"));
	if (vdbs.fVerboseHeapInfo)
		{
		CommSz(SzShared("H E A P   I N F O R M A T I O N"));
		CommSz(SzShared("\r\n"));
		}

/** DOCument data **/

	if (!vdbs.fVerboseHeapInfo)
		AddSzToBuf("DOCs",&dhi);

	for (doc = 0; doc < docMac; doc++)
		{
		if (mpdochdod[doc] != hNil)
			{
			pdod  = PdodDoc(doc);

			if (vdbs.fVerboseHeapInfo)
				{
				AddSzLongToBuf("Doc ", (long) doc, &dhi);
				OutputDocType(doc, &dhi);
				}
			else
				{
				AddSzLongToBuf(" ", (long) doc, &dhi);
				AddSzToBuf(":", &dhi);
				}

			AddCbOfHplc(pdod->hplcpcd,"pcd", &dhi);
			AddCbOfHplc(pdod->hplcpgd,"pgd", &dhi);
			AddCbOfHplc(pdod->hplcfld,"fld", &dhi);

			AddCbOfHplc(pdod->hplcfnd,"fnd", &dhi);
			/* the above is a PLC. One of: fnd,hdd,glsy,mcr,ddli,and */

			AddCbOfHplc(pdod->hplcfrd,"frd", &dhi);

			if (!pdod->fShort)
				{
				AddCbOfHplc(pdod->hplcatrd,"atrd",&dhi);
				AddCbOfHplc(pdod->hplcsed, "sed", &dhi);
				AddCbOfHplc(pdod->hplcpad, "pad", &dhi);

				AddCbOfHsttb(pdod->stsh.hsttbName, "name", &dhi);
				AddCbOfHsttb(pdod->stsh.hsttbChpx, "chpx", &dhi);
				AddCbOfHsttb(pdod->stsh.hsttbPapx, "papx", &dhi);

				AddCbOfHpl(pdod->stsh.hplestcp, "stcp", &dhi);

				AddCbOfHsttb(pdod->hsttbChpe, "chpe", &dhi);
				AddCbOfHsttb(pdod->hsttbPape, "pape", &dhi);
				AddCbOfHsttb(pdod->hsttbAssoc,"assoc", &dhi);
				AddCbOfHplc(pdod->hplcsea, "sea", &dhi);
				AddCbOfHplc(pdod->hplcbkf, "bkf", &dhi);
				AddCbOfHplc(pdod->hplcbkl, "bkl", &dhi);
				AddCbOfHsttb(pdod->hsttbBkmk, "bkmk", &dhi);

				AddCbOfH(pdod->hmpftcibstFont, "ftc", &dhi);

				if (pdod->fGlsy)
					AddCbOfHsttb(pdod->hsttbGlsy, "glsy", &dhi);

				if (pdod->fDot)
					{
/**				AddCbOfH(pdod->hkmpUser, "kmp", &dhi);	**/
					AddCbOfH(pdod->hmudUser, "mud", &dhi);
					}
				}

			SubTotalDhi(&dhi);
			}
		}
	OutputBuf(&dhi);

/*** File Descriptor data ***/
	if (!vdbs.fVerboseHeapInfo)
		AddSzToBuf("FCBs", &dhi);
	for (fn = 1; fn < fnMac; fn++)
		{
		if ((hfcb =mpfnhfcb[fn]) != hNil)
			{
			if (vdbs.fVerboseHeapInfo)
				{
				AddSzToBuf("Fn", & dhi);
				pch = &szT[CchCopySz("(",szT)];
				CchUnsToPpch(fn, &pch);
				*pch++ = ')';
				*pch   = '\0';
				}
			else
				{
				AddSzLongToBuf(" ", (long) fn, &dhi);
				AddSzToBuf(":", &dhi);
				}

			AddCbOfH(hfcb, szT, &dhi);
			AddCbOfHplc((*hfcb)->hplcbteChp, "chp", &dhi);
			AddCbOfHplc((*hfcb)->hplcbtePap, "pap", &dhi);
			SubTotalDhi(&dhi);
			}
		}
	OutputBuf(&dhi);

/*** WindoW data ***/
	AddSzToBuf("Windows ", &dhi);
	for (ww = 0; ww < wwMac; ww++)
		{
		if ((hwwd =mpwwhwwd[ww]) != hNil)
			{
			pch = &szT[CchCopySz(" ww(",szT)];
			CchUnsToPpch(ww, &pch);
			*pch++ = ')';
			*pch   = '\0';
			AddCbOfHpl(hwwd, szT, &dhi);
			}
		}
	SubTotalDhi(&dhi);

/*** Key MaP data ***/
	AddSzToBuf(vdbs.fVerboseHeapInfo ? "Key Maps " : "  KMPs ", &dhi);
	AddCbOfH(hkmpBase, "kmpBase", &dhi);
/**	AddCbOfH(hkmpCur,  "kmpCur", &dhi);		**/
	AddCbOfH(vhkmpUser, "kmpUser", &dhi);
	SubTotalDhi(&dhi);

/*** Menu data ***/
	AddSzToBuf(vdbs.fVerboseHeapInfo ? "Menus    " : "  Menus ", &dhi);
	AddCbOfHsttb(hsttbMenu, "stMenu", &dhi);
	AddCbOfH(vhmudBase, "mudBase", &dhi);
	AddCbOfH(vhmudUser, "mudUser", &dhi);
	SubTotalDhi(&dhi);

/*** Other Heap Users ***/
	AddSzToBuf(vdbs.fVerboseHeapInfo ? "Other    " : "  Other ", &dhi);
	AddCbOfHsttb(vhsttbOpen, "stOpen", &dhi);
	AddCbOfHsttb(vhsttbWnd,  "stWnd",  &dhi);
	AddCbOfH(vhgrpchr, "grpchr", &dhi);
	AddCbOfH(vhmes, "mes", &dhi);
	SubTotalDhi(&dhi);

	OutputBuf(&dhi);
	AddSzLongToBuf("Heap space accounted for:  Near(", (long) dhi.cNearH, &dhi);
	AddSzLongToBuf(")=", dhi.lcbNearTotal, &dhi);
	AddSzLongToBuf("  Far(", (long) dhi.cFarH, &dhi);
	AddSzLongToBuf(")=", dhi.lcbFarTotal, &dhi);
	OutputBuf(&dhi);

	return cmdOK;

}	/* CmdHeapInfo */

#endif /* DEBUG */
