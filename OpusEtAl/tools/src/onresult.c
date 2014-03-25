/* O N R E S U L T . C */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* #define DEBUG */

#define fTrue 1
#define fFalse 0

#define ichMaxLine 256

char szTestComplete[] = "\tTest Completed";
char szStartOfTest[] = "*** Test # ";
char szExecuting[] = "Executing ";
char szErrorOut[] = "ERROR.OUT";
char szMacroError[] = "TestError:";
char szAssertFailed[] = "Assertion Failed.";
char szSDMAssertFailed[] = "SDM Assertion Failed.";
char szInt3Encountered[] = "Int 3 Encountered!";
char szRuntimeError[] = "Macro runtime error";
char szDebugReport[] = "Report:";
char szIdleEntered[] = "Idle entered!";
char szMessageBox[] = "MessageBox:";
char szDiskError[] = "DiskError! ";


#define FLineIsSz(sz) (!strncmp(sz, szLine, sizeof(sz)-1))

#define tkUnknown 0
#define tkEOF 1
#define tkTestComplete 2
#define tkStartOfTest 3
#define tkExecuting 4
#define tkErrorOut 5
#define tkMacroError 6
#define tkInternalError 7
#define tkDateTime 8

int tkLine = tkUnknown;
char szLine[ichMaxLine];
char szName[9];

int fHadONSETUP = fFalse;

#define nTestONSETUP	1

FILE *fpInput;
FILE *fopen();

main(cArg, rgszArg)
int cArg;
char *rgszArg[];
{
	if (cArg < 2)
		Error("Missing file name");

	if ((fpInput = fopen(rgszArg[1], "r")) == NULL)
		Error("Cannot open input file");

	/* get user name */
	fscanf(fpInput, "%8s", szName);
	fprintf(stderr, "Processing results for %s\r\n", szName);

	while (!feof(fpInput))
		ProcessTest();

	fcloseall();
	exit(0);
}


Error(sz)
char *sz;
{
	fprintf(stderr, "onresult: error: %s\r\n", sz);
	exit(1);
}


ProcessTest()
{
	int nTest;
	int fDone = fFalse;
	int cErrors = 0;
	int nDayStart=0, nMinStart=0;
	int nDayEnd, nMinEnd, dnMin=0;
	char *szResult;
	char szError[ichMaxLine];

	while (tkLine != tkEOF && tkLine != tkExecuting)
		TkNext();

	if (tkLine == tkEOF)
		return;

	nTest = atoi(&szLine[sizeof(szExecuting)-1]);

	if (nTest < 1000)
		{
		if (strncmp(&szLine[sizeof(szExecuting)-1], "ONSETUP", 7))
			{
			TkNext();
			return;
			}
		/* else test is ONSETUP */
		nTest = nTestONSETUP;
		fHadONSETUP = fTrue;
		}

	if (!fHadONSETUP)
		{
		printf("ONSETUP %-8.8s               Test missing!\n", szName);
		fHadONSETUP = fTrue;
		}

	fprintf(stderr, "\ttest # %d\r\n", nTest);

	while (TkNext() != tkDateTime && tkLine != tkEOF)
		;

	if (tkLine == tkDateTime)
		GetDayMin(&nDayStart, &nMinStart);

	while (TkNext() == tkUnknown || tkLine == tkMacroError)
		if (tkLine == tkMacroError)
			cErrors++;

	szError[0] = 0;

	switch (tkLine)
		{
	case tkEOF:
		nDayStart = 0;
		szResult = "Unexpected end of file\n";
		break;

	case tkTestComplete:
		szResult = "Completed normally\n";
		if (nTest == nTestONSETUP)
			return;
		break;

	case tkInternalError:
		strcpy(szError, szLine);
		szResult = "";
		break;

	case tkStartOfTest:
	case tkExecuting:
		nDayStart = 0;
		/* fall through */

	case tkDateTime:
		szResult = "Unexpected end of macro\n";
		break;

	case tkErrorOut:
		szResult = "ERROR.OUT: ";
		TkNext();
		strcpy(szError, szLine);
		break;

	default:
		szResult = "UNKNOWN TOKEN\n";
		break;
		}

	if (nDayStart != 0)
		{
		while (tkLine != tkDateTime && tkLine != tkEOF)
			TkNext();
		if (tkLine == tkDateTime)
			GetDayMin(&nDayEnd, &nMinEnd);
		else
			nDayStart = 0;
		}

	if (nDayStart != 0)
		{
		if (nDayStart != nDayEnd)
			nMinEnd += (24*60);
		dnMin = nMinEnd - nMinStart;
		}

	if (nTest != nTestONSETUP)
		printf(" %4.4d   %-8.8s   %2d   %4.2d   %s%s", nTest, szName, cErrors, 
				dnMin, szResult, szError);
	else
		printf("ONSETUP %-8.8s               %s%s", szName, szResult, szError);
}


GetDayMin(pnDay, pnMin)
int *pnDay, *pnMin;
{
	int nHour;
	sscanf(szLine, "%*3s %*3s %d %d:%d:%*d", pnDay, &nHour, pnMin);
#ifdef DEBUG
	fprintf(stderr, "day: %d, hour: %d, min: %d\n", *pnDay, nHour, *pnMin);
#endif /* DEBUG */
	*pnMin += (nHour * 60);
}


int TkNext()
{
	fgets(szLine, ichMaxLine, fpInput);

	if (feof(fpInput))
		{
		szLine[0] = 0;
		tkLine = tkEOF;
		}

	else  if (FLineIsSz(szTestComplete))
		tkLine = tkTestComplete;

	else  if (FLineIsSz(szStartOfTest))
		tkLine = tkStartOfTest;

	else  if (FLineIsSz(szExecuting))
		tkLine = tkExecuting;

	else  if (FLineIsSz(szErrorOut))
		tkLine = tkErrorOut;

	else  if (FLineIsSz(szMacroError))
		tkLine = tkMacroError;

	else  if (FLineIsSz(szAssertFailed) || FLineIsSz(szSDMAssertFailed)
			|| FLineIsSz(szInt3Encountered) || FLineIsSz(szRuntimeError)
			|| FLineIsSz(szDebugReport) || FLineIsSz(szIdleEntered)
			|| FLineIsSz(szMessageBox) || FLineIsSz(szDiskError))
		tkLine = tkInternalError;

	else  if (FLineIsSz("Mon ") || FLineIsSz("Tue ") || FLineIsSz("Wed ")
			|| FLineIsSz("Thu ") || FLineIsSz("Fri ") || FLineIsSz("Sat ")
			|| FLineIsSz("Sun "))
		tkLine = tkDateTime;

	else
		tkLine = tkUnknown;

	return tkLine;
}


