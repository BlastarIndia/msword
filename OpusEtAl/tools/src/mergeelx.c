/* mergeelx.c: utility to combine the .ELX files for a set of dialogs, and
* produce the following files:
*	ELKDEFS.H	-- ELK symbol definitions
*	ELXINFO.H	-- data structures for dialog information
*/
#include <stdio.h>
#include <malloc.h>

#include "opl.h"
#include "mergeelx.h"

void Error(char *, unsigned);
ATMT AtmtGet();
void GetStVal(int);

char stVal[80];


int WCmpiStSt(unsigned char *, unsigned char *), AddElkSt(unsigned char *);
void OpenInput(char *), ProcessFile(char *), ProcessScriptFile(char *);
void CloseInput(void), Error(char *, unsigned);
void ReadPlistEntry(TDR *, TIR ***, TCR ***);
void ErrSyntax(void), AssertFailed(void);
void PrintElkName(unsigned char *, FILE *);
void WriteElxDefs(void), WriteElxInfo(void);
void PrintSt(FILE *, unsigned char *);
TDR *PtdrReadDialog(void), *PtdrFindSt(unsigned char *);
unsigned char *StAtmString(void), *StAtmKeyword(void);
unsigned char *StCopySt(unsigned char *);
FILE *PfileOpenOutput();
ELK ElkLookupSt(unsigned char *);
char *PbAllocCb(unsigned), *PbReallocPbCb(char *, unsigned);

char chNext;
TDR *ptdrFirst, **pptdrLast;

static unsigned char stDialog[]		= "\006dialog";
static unsigned char stItem[]		= "\004item";
static unsigned char stChild[]		= "\005child";

static char szOutputDefs[]		= "ELXDEFS.H";
static char szOutputInfo[]		= "ELXINFO.H";

static unsigned *mpelkichName;
ELK elkMac, elkMax;
static unsigned char *rgchElkNames;
unsigned ichElkNameMac, ichElkNameMax;
int fTest = FALSE, fNoTmcs = FALSE, fListboxes = FALSE, fSync = FALSE;

#define iszElkSyncMax	1000
	char * rgszElkSync [iszElkSyncMax] = { 
	0 	};


int iszElkSyncMac;
int fDirtySync = FALSE;
char * szSyncFile;

int mpistiszSync [iszElkSyncMax];


main(argc, argv)
int argc;
char *argv[];
{
	int iarg;

	if (argc < 2)
		{
		printf("Usage: MERGEELX [-t] [-f file] [-s sync-file] file1.elx file2.elx file3.elx ...\n");
		exit(1);
		}

	InitElkTable();

	ptdrFirst = 0;
	pptdrLast = &ptdrFirst;
	for (iarg = 1; iarg < argc; iarg++)
		{
		if (argv[iarg][0] == '-')
			{
			switch (argv[iarg][1])
				{
			default:
				Error("unrecognized switch: \"%c\"",
						argv[iarg][1]);
				/* NOTREACHED */
			case 't':
				fTest = TRUE;
				break;

			case 's':
					{
					if (argv[iarg][2] == '\0')
						{
						if (++iarg >= argc ||
								argv[iarg][0] == '-')
							Error("filename must follow -s", 0);
						szSyncFile = &argv[iarg][0];
						}
					else
						szSyncFile = &argv[iarg][2];

					ProcessSyncFile(szSyncFile);
					}
				break;

			case 'f':
					{
					char *szScriptFile;

					if (argv[iarg][2] == '\0')
						{
						if (++iarg >= argc ||
								argv[iarg][0] == '-')
							Error("filename must follow -f", 0);
						szScriptFile = &argv[iarg][0];
						}
					else
						szScriptFile = &argv[iarg][2];

					ProcessScriptFile(szScriptFile);
					}
				break;
			case 'n':	/* no TMC's */
				fNoTmcs = TRUE;
				break;
			case 'l':	/* include listboxes (for Omega) */
				fListboxes = TRUE;
				break;
			/* end switch */
				}
			}
		else
			ProcessFile(argv[iarg]);
		}

	/* We now have all the TDR's in memory.
		*/
	WriteElxDefs();
	WriteElxInfo();
	WriteSyncFile();

	/* The ELK mapping table was created while writing out the ELX info.
		*/
	if (fTest)
		DumpElkTable();

	exit(0);
}


void ProcessFile(szName)
char *szName;
{	/* -- filename -- */
/*	printf("processing \"%s\"\n", szName);*/
	OpenInput(szName);

	while ((*pptdrLast = PtdrReadDialog()) != 0)
		pptdrLast = &(*pptdrLast)->ptdrNext;
	CloseInput();
}


void ProcessScriptFile(szName)
char *szName;
{
	FILE *pfileScript = fopen(szName, "rt");
	int ch, ichElxFile;
	char rgchElxFile[100];

	if (pfileScript == 0)
		Error("script file \"%s\" not found", (unsigned)szName);

	while (1)
		{
		while ((ch = fgetc(pfileScript)) == ' ' || ch == '\t' ||
				ch == '\n' || ch == EOF)
			{
			if (ch == EOF)
				goto BreakWhile1;
			}

		ichElxFile = 0;
		do
			{
			rgchElxFile[ichElxFile++] = ch;
			ch = fgetc(pfileScript);
			}
		while (ch != ' ' && ch != '\t' && ch != '\n' && ch != EOF);

		rgchElxFile[ichElxFile] = '\0';
		ProcessFile(rgchElxFile);
		if (ch == EOF)
			goto BreakWhile1;
		}
BreakWhile1:
	fclose(pfileScript);
}


ProcessSyncFile(szName)
char * szName;
{
	FILE * pfile;
	char szLine [256];

	if ((pfile = fopen(szName, "r")) == NULL)
		return;

	while (fgets(szLine, sizeof (szLine), pfile) != NULL)
		{
		int i;
		char * pch;

		for (pch = szLine; *pch > ' '; pch += 1)
			;

		if (*pch == '\0')
			Error("bad line is sync file: %s", (unsigned) szLine);

		*pch++ = '\0';

		if ((i = atoi(pch)) >= iszElkSyncMax)
			Error("bad elk index in sync file (max is %d)", iszElkSyncMax);

		if (rgszElkSync[i] != NULL)
			Error("duplicate elk index in sync file: %d", i);

		rgszElkSync[i] = PbAllocCb(strlen(szLine) + 1);
		strcpy(rgszElkSync[i], szLine);
		if (i > iszElkSyncMac - 1)
			iszElkSyncMac = i + 1;
		}

	fclose(pfile);

	fSync = TRUE;
}


int IszSyncSz(sz)
char * sz;
{
	int isz;

	for (isz = 0; isz < iszElkSyncMax; isz += 1)
		if (strcmp(sz, rgszElkSync[isz]) == 0)
			return isz;

	for (isz = 0; isz < iszElkSyncMax; isz += 1)
		if (rgszElkSync[isz] == NULL)
			{
			fDirtySync = TRUE;
			rgszElkSync[isz] = PbAllocCb(strlen(sz) + 1);
			strcpy(rgszElkSync[isz], sz);
			if (isz > iszElkSyncMac - 1)
				iszElkSyncMac = isz + 1;

			return isz;
			}

	/* if this happens, iszElkSyncMax must be increased! */
	Error("rgszElkSync is full", 0);
}


TDR *PtdrReadDialog()
{
	ATMT atmt;
	TDR *ptdr;
	TIR **pptirLast;
	TCR **pptcrLast;

	if ((atmt = AtmtGet()) == atmtEof)
		return 0;

	ptdr = (TDR *)PbAllocCb(sizeof(TDR));

	if (atmt != atmtLParen || AtmtGet() != atmtKeyword ||
			WCmpiStSt(stDialog, stVal) != 0)
		ErrSyntax();

	/* Read the "hid" name for the dialog.
		*/
	ptdr->stHidName = StAtmKeyword();

	/* Read the CAB name for the dialog.
		*/
	ptdr->stCabName = StAtmKeyword();

	/* Read the CABI.
		*/
	ptdr->stCabi = StAtmKeyword();

	/* Read property list entries.
		*/
	ptdr->ctir = 0;
	pptirLast = &ptdr->ptirFirst;
	pptcrLast = &ptdr->ptcrFirst;
	while (1)
		{
		if ((atmt = AtmtGet()) == atmtLParen)
			ReadPlistEntry(ptdr, &pptirLast, &pptcrLast);
		else  if (atmt == atmtRParen)
			break;
		else
			ErrSyntax();
		}
	*pptirLast = 0;
	*pptcrLast = 0;

	/* We just read the ")" at the end of the property list.
		*/
	return ptdr;
}


void ReadPlistEntry(ptdr, ppptirLast, ppptcrLast)
TDR *ptdr;
TIR ***ppptirLast;
TCR ***ppptcrLast;
{
	ATMT atmt;

	if (AtmtGet() != atmtKeyword)
		ErrSyntax();

	if (WCmpiStSt(stChild, stVal) == 0)
		{
		ATMT atmt;

		**ppptcrLast = (TCR *)PbAllocCb(sizeof (TCR));

		if (AtmtGet() != atmtKeyword)	/* field-name in parent */
			ErrSyntax();
		(**ppptcrLast)->stParentField = StCopySt(stVal);

		if (AtmtGet() != atmtKeyword)	/* child name */
			ErrSyntax();
		(**ppptcrLast)->stChildName = StCopySt(stVal);

		*ppptcrLast = &(**ppptcrLast)->ptcrNext;
		**ppptcrLast = 0;

		if (AtmtGet() != atmtRParen)
			ErrSyntax();
		}
	else  if (WCmpiStSt(stItem, stVal) == 0)
		{
		**ppptirLast = (TIR *)PbAllocCb(sizeof(TIR));
		(**ppptirLast)->stName = StAtmString();
		(**ppptirLast)->stElv = StAtmKeyword();
		(**ppptirLast)->stFieldName = StAtmKeyword();

		(**ppptirLast)->IszElkSync = AddElkSt((**ppptirLast)->stName);
		*ppptirLast = &(**ppptirLast)->ptirNext;

		if (AtmtGet() != atmtRParen)
			ErrSyntax();

		ptdr->ctir++;
		}
	else
		ErrSyntax();
}


void WriteElxDefs()
{
	ELK elk;
	FILE *pfile;
	int ieldi;
	TDR *ptdrT;

/*	printf("generating ELXDEFS.H\n");*/
	pfile = PfileOpenOutput(szOutputDefs);

	/* Generate #define's for all items which have "elx" symbols
		* specified.
		*/
	for (ptdrT = ptdrFirst; ptdrT != 0; ptdrT = ptdrT->ptdrNext)
		{
		TIR *ptir;

/* 		for (ptir = ptdrT->ptirFirst; ptir != 0; ptir = ptir->ptirNext)
/* 			{
/* 			if (ptir->stElk != 0)
/* 				{
/* 				fprintf(pfile, "#define\t");
/* 				PrintSt(pfile, ptir->stElk);
/* 				fprintf(pfile, "\t%d\n",
/* 					ElkLookupSt(ptir->stName));
/* 				}
/* 			}
/**/
		}

	fprintf(pfile, "\n");

	/* Generate #define's for the "ieldi"'s (Index to EL Dialog Info).
		*/
	for (ieldi = 0, ptdrT = ptdrFirst;
			ptdrT != 0;
			ieldi++, ptdrT = ptdrT->ptdrNext)
		{
		unsigned char *stHidName;
		unsigned chHidName;

		fprintf(pfile, "#define\tieldi");

		/* Use the "hid name", omitting the first three letters if
			* they are "hid" ...
			*/
		stHidName = StCopySt(ptdrT->stHidName);
		if ((chHidName = stHidName[0]) > 3)
			stHidName[0] = 3;
		if (WCmpiStSt(stHidName, "\003HID") == 0)
			{
			unsigned char chT;

			stHidName[0] = chHidName;
			chT = stHidName[3];
			stHidName[3] = chHidName - 3;
			PrintElkName(&stHidName[3], pfile);
			stHidName[3] = chT;
			}
		else
			{
			stHidName[0] = chHidName;
			PrintElkName(stHidName, pfile);
			}
		free(stHidName);

		fprintf(pfile, "\t%d\n", ieldi);
		}

	fprintf(pfile, "\n#define\telkAppMac\t%d\n", /*elkMac*/iszElkSyncMac);
	fprintf(pfile, "#define\tibstElkAppMac\t%d\n", elkMac);

	fclose(pfile);
}


void PrintElkName(st, pfile)
unsigned char *st;
FILE *pfile;
{
	int ich;
	unsigned char ch;

	for (ich = 0; ich < st[0]; ich++)
		{
		ch = st[ich + 1];
		if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
				(ch >= '0' && ch >= '9') || ch == '_')
			{
			fputc(ch, pfile);
			}
		}
}


void WriteElxInfo()
{
	FILE *pfile;
	TDR *ptdrCur;
	unsigned ieldi;
	ELK elk;
	int ich, isz;

/*	printf("generating ELXINFO.H\n");*/

	pfile = PfileOpenOutput(szOutputInfo);
	fprintf(pfile, "#ifdef elkAppMac\n");
	fprintf(pfile, "csconst ELDI rgeldi[] = {\n");

	for (ieldi = 0, ptdrCur = ptdrFirst;
			ptdrCur != 0;
			ieldi++, ptdrCur = ptdrCur->ptdrNext)
		{
		TDR *ptdrT;
		TIR *ptir;
		TCR *ptcr;
		unsigned ctirAll;
		int fFirstField;
		int fInChild = FALSE;
		char rgchIagPrefix[100];

		/* Display basic information.
			*/
		fprintf(pfile, "\t{");
		PrintSt(pfile, ptdrCur->stHidName);
		fprintf(pfile, ", ");
		PrintSt(pfile, ptdrCur->stCabi);
		fprintf(pfile, ", %d, ", CtirFrTdr(ptdrCur));

		/* Generate the "rgelfd" array, including fields from child
			* dialogs as well as fields from this dialog.
			*/
		fprintf(pfile, "{\n");

		ptir = ptdrCur->ptirFirst;
		ptcr = ptdrCur->ptcrFirst;
		sprintf(rgchIagPrefix, "Iag(%s, ", ptdrCur->stCabName + 1);
		fFirstField = TRUE;
		while (1)
			{
			while (ptir == 0)
				{
				TDR *ptdrChild;

				if (ptcr == 0)
					goto BreakWhile1;
				fInChild = TRUE;
				ptdrChild = PtdrFindSt(ptcr->stChildName);
				ptir = ptdrChild->ptirFirst;
				sprintf(rgchIagPrefix, "Iag(%s, %s) + cwCabMin + Iag(%s, ",
						ptdrCur->stCabName + 1,
						ptcr->stParentField + 1,
						ptdrChild->stCabName + 1);
				ptcr = ptcr->ptcrNext;
				}

			if (fListboxes ||
					WCmpiStSt(ptir->stElv, "\015elvListBoxInt") &&
					WCmpiStSt(ptir->stElv, "\020elvListBoxString") &&
					WCmpiStSt(ptir->stElv, "\017elvListBoxRgint") &&
					WCmpiStSt(ptir->stElv, "\013elvComboBox"))
				{
				if (!fFirstField)
					fprintf(pfile, ",\n");
				fFirstField = FALSE;
				PrintItem(pfile, ptir, rgchIagPrefix);
				}

			ptir = ptir->ptirNext;
			}
BreakWhile1:
		if (!fFirstField)
			fprintf(pfile, "\n");
		if (ptdrCur->ptdrNext != 0)
			fprintf(pfile, "\t }},\n");
		else
			fprintf(pfile, "\t }}\n");
		}
	fprintf(pfile, "};\n\n");

	/* Generate the istName <--> name mapping data.
		*/
	fprintf(pfile, "csconst unsigned rgichName[] = {");
	for (elk = 0; elk < elkMac; elk++)
		{
		fprintf(pfile, (elk & 7) == 0 ? "\n\t%d" : "%d",
				mpelkichName[elk]);
		if (elk != elkMac - 1)
			fprintf(pfile, ", ");
		}
	fprintf(pfile, "};\ncsconst unsigned char rgchElkNames[] = {");
	for (ich = 0; ich < ichElkNameMac; ich++)
		{
		fprintf(pfile, (ich & 7) == 0 ? "\n\t0x%02x" : "0x%02x",
				rgchElkNames[ich]);
		if (ich != ichElkNameMac - 1)
			fprintf(pfile, ", ");
		}
	fprintf(pfile, "};\n\n");

	/* Generate the ELK <--> istName mapping data */
	fprintf(pfile, "csconst unsigned mpelkistName [] = {");
	for (isz = 0; isz < iszElkSyncMac; isz += 1)
		{
		int elk;

		for (elk = 1; elk < elkMac; elk += 1)
			if (mpistiszSync[elk] == isz)
				break;
		if (elk == elkMac)
			elk = -1;
/*if (elk != -1) printf("%s %d\n", rgszElkSync[mpistiszSync[elk]], mpistiszSync[elk]);*/
		fprintf(pfile, (isz & 7) == 0 ? "\n\t%d" : "%d", elk);

		if (isz != iszElkSyncMac - 1)
			fprintf(pfile, ", ");
		}
	fprintf(pfile, "};\n\n#endif\n#ifdef STID\n");
	fprintf(pfile, "csconst char rgksp [] = StringMap(\"SUPO\", 0, 1);\n");

	fprintf(pfile, "csconst char mpstiderc [] =\n");
	fprintf(pfile, "{\n");
	fprintf(pfile, "\t39, 19, 25, 116, 80, 90, 75, 86, 74, 86, 95, 77, 25, 110, 86, 75, 93, \n");
	fprintf(pfile, "\t25, 95, 86, 75, 25, 110, 80, 87, 93, 86, 78, 74, 25, 19, 55, 20, 25, \n");
	fprintf(pfile, "\t122, 75, 92, 88, 77, 92, 93, 25, 91, 64, 25, 20, 26, 88, 85, 88, 87, \n");
	fprintf(pfile, "\t92, 21, 25, 91, 86, 91, 67, 21, 25, 91, 75, 88, 93, 90, 81, 21, 25, \n");
	fprintf(pfile, "\t91, 75, 88, 93, 79, 21, 25, 91, 75, 64, 88, 87, 85, 21, 19, 90, 81, \n");
	fprintf(pfile, "\t88, 75, 85, 92, 74, 74, 21, 25, 90, 81, 80, 90, 21, 25, 90, 81, 75, \n");
	fprintf(pfile, "\t80, 74, 84, 21, 25, 90, 85, 92, 84, 92, 87, 74, 78, 21, 25, 93, 88, \n");
	fprintf(pfile, "\t79, 80, 93, 91, 86, 21, 27, 93, 88, 79, 80, 93, 85, 76, 21, 25, 93, \n");
	fprintf(pfile, "\t88, 79, 80, 93, 84, 90, 82, 21, 25, 95, 86, 75, 93, 84, 21, 25, 83, \n");
	fprintf(pfile, "\t76, 75, 94, 92, 87, 85, 21, 18, 82, 75, 80, 74, 81, 87, 88, 84, 21, \n");
	fprintf(pfile, "\t25, 84, 88, 75, 82, 74, 92, 88, 21, 25, 73, 92, 77, 92, 75, 83, 21, \n");
	fprintf(pfile, "\t25, 75, 88, 64, 94, 75, 21, 25, 75, 80, 90, 81, 88, 75, 93, 91, 21, \n");
	fprintf(pfile, "\t30, 75, 80, 90, 82, 74, 21, 25, 75, 86, 74, 80, 92, 73, 21, 25, 77, \n");
	fprintf(pfile, "\t86, 84, 74, 88, 65, 21, 25, 77, 86, 87, 64, 82, 75, 21, 25, 64, 86, \n");
	fprintf(pfile, "\t74, 81, 80, 77, 86, 64, 54, 20, 25, 120, 74, 74, 80, 74, 77, 92, 93, \n");
	fprintf(pfile, "\t25, 91, 64, 25, 20, 29, 91, 75, 88, 87, 93, 64, 77, 21, 25, 93, 88, \n");
	fprintf(pfile, "\t87, 73, 21, 25, 93, 86, 76, 94, 74, 90, 21, 25, 93, 86, 76, 94, 77, \n");
	fprintf(pfile, "\t21, 25, 81, 92, 75, 91, 82, 21, 16, 83, 92, 95, 95, 75, 76, 21, 25, \n");
	fprintf(pfile, "\t85, 88, 76, 75, 92, 85, 85, 21, 25, 84, 88, 75, 80, 88, 91, 21, 25, \n");
	fprintf(pfile, "\t84, 80, 82, 92, 81, 86, 73, 21, 25, 74, 64, 85, 79, 80, 88, 81, 44, \n");
	fprintf(pfile, "\t20, 25, 105, 92, 87, 94, 76, 80, 87, 25, 109, 75, 88, 80, 87, 92, 75, \n");
	fprintf(pfile, "\t74, 25, 20, 25, 16, 88, 87, 87, 92, 81, 88, 21, 25, 90, 81, 75, 80, \n");
	fprintf(pfile, "\t74, 77, 80, 80, 21, 25, 93, 88, 79, 92, 88, 77, 21, 25, 93, 88, 79, \n");
	fprintf(pfile, "\t80, 93, 80, 21, 25, 94, 80, 87, 88, 93, 88, 21, 18, 94, 80, 87, 87, \n");
	fprintf(pfile, "\t80, 74, 78, 21, 25, 83, 88, 87, 67, 21, 25, 83, 86, 77, 21, 25, 85, \n");
	fprintf(pfile, "\t92, 74, 85, 92, 64, 73, 21, 25, 85, 80, 67, 81, 21, 25, 73, 88, 84, \n");
	fprintf(pfile, "\t92, 85, 88, 83, 21, 17, 73, 88, 76, 85, 84, 90, 21, 25, 73, 92, 87, \n");
	fprintf(pfile, "\t87, 64, 84, 21, 25, 75, 80, 90, 81, 91, 85, 21, 25, 74, 81, 92, 85, \n");
	fprintf(pfile, "\t85, 92, 64, 91, 21, 25, 74, 81, 80, 81, 90, 21, 37, 74, 77, 88, 87, \n");
	fprintf(pfile, "\t81, 21, 25, 74, 76, 75, 92, 82, 81, 92, 21, 25, 77, 88, 84, 84, 80, \n");
	fprintf(pfile, "\t81, 21, 25, 77, 80, 84, 84, 52, 20, 25, 109, 92, 74, 77, 92, 93, 25, \n");
	fprintf(pfile, "\t91, 64, 25, 20, 11, 88, 88, 74, 81, 92, 92, 84, 88, 21, 25, 88, 87, \n");
	fprintf(pfile, "\t87, 92, 78, 21, 25, 91, 88, 75, 77, 85, 92, 64, 81, 21, 25, 91, 75, \n");
	fprintf(pfile, "\t88, 93, 91, 21, 25, 91, 75, 88, 93, 81, 80, 21, 25, 91, 75, 80, 88, \n");
	fprintf(pfile, "\t87, 84, 76, 21, 31, 90, 81, 88, 74, 92, 95, 21, 25, 90, 81, 76, 90, \n");
	fprintf(pfile, "\t82, 86, 21, 25, 92, 79, 88, 87, 90, 21, 25, 94, 85, 92, 87, 73, 86, \n");
	fprintf(pfile, "\t21, 25, 83, 88, 87, 92, 77, 74, 21, 29, 83, 88, 87, 74, 91, 21, 25, \n");
	fprintf(pfile, "\t83, 92, 95, 95, 84, 76, 21, 25, 83, 80, 84, 74, 21, 25, 82, 92, 80, \n");
	fprintf(pfile, "\t77, 81, 94, 21, 25, 82, 92, 86, 82, 80, 88, 21, 18, 82, 80, 92, 75, \n");
	fprintf(pfile, "\t87, 86, 87, 75, 21, 25, 85, 80, 74, 88, 84, 86, 21, 25, 84, 92, 85, \n");
	fprintf(pfile, "\t80, 74, 74, 88, 93, 21, 25, 84, 80, 90, 81, 88, 92, 85, 21, 25, 87, \n");
	fprintf(pfile, "\t92, 80, 85, 81, 21, 29, 87, 92, 85, 74, 86, 87, 84, 21, 25, 73, 92, \n");
	fprintf(pfile, "\t77, 92, 75, 91, 88, 82, 21, 25, 73, 81, 80, 85, 95, 88, 21, 25, 75, \n");
	fprintf(pfile, "\t86, 91, 92, 75, 77, 93, 86, 21, 37, 74, 77, 92, 79, 92, 80, 21, 25, \n");
	fprintf(pfile, "\t77, 92, 75, 75, 64, 77, 21, 25, 77, 80, 84, 90, 88, 21, 25, 77, 75, \n");
	fprintf(pfile, "\t88, 84, 91, 36, 20, 25, 118, 77, 81, 92, 75, 25, 93, 92, 79, 92, 85, \n");
	fprintf(pfile, "\t86, 73, 84, 92, 87, 77, 25, 78, 86, 75, 82, 25, 91, 64, 25, 20, 26, \n");
	fprintf(pfile, "\t91, 86, 91, 94, 76, 21, 25, 93, 88, 75, 64, 85, 74, 21, 25, 94, 75, \n");
	fprintf(pfile, "\t92, 94, 90, 21, 25, 93, 88, 79, 80, 93, 78, 21, 25, 83, 86, 87, 84, \n");
	fprintf(pfile, "\t21, 19, 87, 92, 92, 85, 84, 88, 81, 21, 25, 73, 88, 76, 85, 88, 84, \n");
	fprintf(pfile, "\t21, 25, 73, 88, 76, 85, 91, 86, 21, 25, 75, 86, 91, 92, 75, 77, 91, \n");
	fprintf(pfile, "\t76, 21, 25, 74, 90, 86, 77, 74, 77, 21, 27, 74, 90, 86, 77, 77, 91, \n");
	fprintf(pfile, "\t92, 21, 25, 74, 90, 86, 77, 77, 75, 88, 21, 25, 74, 75, 80, 93, 81, \n");
	fprintf(pfile, "\t88, 75, 21, 25, 74, 76, 75, 64, 88, 87, 75, 45, 20, 25, 125, 86, 90, \n");
	fprintf(pfile, "\t76, 84, 92, 87, 77, 88, 77, 80, 86, 87, 25, 91, 64, 25, 20, 26, 91, \n");
	fprintf(pfile, "\t86, 91, 91, 92, 85, 85, 21, 25, 91, 64, 75, 86, 87, 82, 21, 25, 90, \n");
	fprintf(pfile, "\t86, 85, 85, 92, 92, 87, 21, 25, 93, 88, 75, 85, 92, 87, 92, 75, 21, \n");
	fprintf(pfile, "\t11, 93, 88, 64, 85, 92, 84, 21, 25, 93, 86, 76, 94, 74, 21, 25, 92, \n");
	fprintf(pfile, "\t85, 88, 80, 87, 92, 74, 21, 25, 81, 86, 85, 85, 64, 91, 21, 25, 80, \n");
	fprintf(pfile, "\t87, 94, 75, 80, 93, 77, 21, 25, 82, 88, 77, 81, 64, 91, 76, 85, 21, \n");
	fprintf(pfile, "\t17, 85, 88, 76, 75, 92, 87, 93, 21, 25, 85, 88, 76, 75, 80, 92, 91, \n");
	fprintf(pfile, "\t21, 25, 85, 80, 74, 88, 93, 21, 25, 85, 80, 74, 88, 84, 88, 64, 21, \n");
	fprintf(pfile, "\t25, 84, 88, 75, 64, 93, 21, 18, 75, 76, 74, 74, 91, 21, 25, 74, 90, \n");
	fprintf(pfile, "\t86, 77, 77, 85, 88, 21, 25, 74, 81, 88, 75, 80, 74, 75, 21, 25, 74, \n");
	fprintf(pfile, "\t77, 92, 73, 81, 92, 87, 88, 21, 25, 78, 88, 85, 77, 92, 75, 78, 92, \n");
	fprintf(pfile, "\t16, 20, 25, 116, 88, 87, 88, 94, 92, 84, 92, 87, 77, 21, 25, 106, 76, \n");
	fprintf(pfile, "\t73, 73, 86, 75, 77, 25, 88, 87, 93, 25, 116, 88, 75, 82, 92, 77, 80, \n");
	fprintf(pfile, "\t87, 94, 25, 91, 64, 25, 20, 25, 31, 88, 93, 75, 80, 88, 87, 78, 21, \n");
	fprintf(pfile, "\t25, 91, 80, 85, 85, 94, 21, 25, 93, 80, 88, 87, 88, 73, 92, 21, 25, \n");
	fprintf(pfile, "\t94, 75, 92, 94, 74, 21, 25, 83, 92, 95, 95, 75, 21, 17, 83, 92, 95, \n");
	fprintf(pfile, "\t95, 74, 88, 21, 25, 83, 80, 85, 85, 93, 86, 21, 25, 83, 86, 81, 87, \n");
	fprintf(pfile, "\t73, 88, 21, 25, 83, 86, 87, 75, 92, 21, 25, 82, 86, 75, 87, 92, 85, \n");
	fprintf(pfile, "\t84, 88, 21, 19, 84, 92, 85, 80, 87, 93, 88, 95, 21, 25, 84, 80, 90, \n");
	fprintf(pfile, "\t81, 92, 85, 94, 21, 25, 84, 80, 82, 92, 84, 88, 73, 21, 25, 84, 86, \n");
	fprintf(pfile, "\t87, 80, 90, 88, 81, 21, 25, 75, 86, 87, 74, 86, 56, 25, 29, 120, 87, \n");
	fprintf(pfile, "\t93, 25, 106, 73, 92, 90, 80, 88, 85, 25, 109, 81, 88, 87, 82, 74, 25, \n");
	fprintf(pfile, "\t77, 86, 25, 123, 92, 75, 82, 92, 25, 123, 75, 92, 88, 77, 81, 92, 93, \n");
	fprintf(pfile, "\t25, 95, 86, 75, 25, 80, 87, 74, 73, 80, 75, 88, 77, 80, 86, 87, 25, \n");
	fprintf(pfile, "\t88, 87, 93, 25, 90, 86, 84, 80, 90, 25, 75, 92, 85, 80, 92, 95, 57, \n");
	fprintf(pfile, "\t19, 109, 81, 80, 74, 25, 75, 92, 85, 92, 88, 74, 92, 25, 80, 74, 25, \n");
	fprintf(pfile, "\t93, 92, 93, 80, 90, 88, 77, 92, 93, 25, 77, 86, 25, 77, 81, 92, 25, \n");
	fprintf(pfile, "\t84, 92, 84, 86, 75, 64, 25, 86, 95, 41, 125, 88, 79, 80, 93, 25, 106, \n");
	fprintf(pfile, "\t23, 25, 123, 86, 76, 75, 87, 92, 25, 38, 120, 76, 94, 76, 74, 77, 25, \n");
	fprintf(pfile, "\t11, 11, 21, 25, 8, 0, 15, 11, 25, 20, 25, 115, 76, 87, 92, 25, 8, \n");
	fprintf(pfile, "\t14, 21, 25, 8, 0, 0, 9, \n");
	fprintf(pfile, "\t};\n");
	fprintf(pfile, "\n");
	fprintf(pfile, "csconst int rgstid [] =\n");
	fprintf(pfile, "\t{\n");
	fprintf(pfile, "\t0, 31, 46, 82, 125, 160, 204, 244, 260, 297, 339, 361, 403, 447, 488, 517, 531, \n");
	fprintf(pfile, "\t582, 621, 658, 702, 739, 768, 798, 834, 877, 912, 933, 969, 1020, 1061, 1105, 1147, 1186, \n");
	fprintf(pfile, "\t1227, 1270, 1272, 1309, 1342, 1343, 1386, 1403, \n");
	fprintf(pfile, "\t};\n\n");
	fprintf(pfile, "csconst long rgrgb [] =\n");
	fprintf(pfile, "\t{\n");
	fprintf(pfile, "\t0x00000000L, 0x00ff0000L, 0x0000ffffL, 0x0000ff00L, \n");
	fprintf(pfile, "\t0x000000ffL, 0x00ff00ffL, 0x00ffff00L, 0x00ffffffL \n");
	fprintf(pfile, "\t};\n");
	fprintf(pfile, "#endif\n");
}


/* IszFromSt(st) now handled by tir->IszElkSync (jurgenl)
/*
/* 	IszFromSt(st)
/* 	char * st;
/* 		{
/* 		int isz;
/* 		char sz [256];
/*
/* 		StToSz(st, sz);
/* 		for (isz = 0; isz < iszElkSyncMac; ++isz)
/*			if (strcmp(sz, rgszElkSync[isz]) == 0)
/*				return isz;
/*	Assert(FALSE);
/*	}
/**/


PrintItem(pfile, ptir, szIagPrefix)
FILE *pfile;
TIR *ptir;
char *szIagPrefix;
{
	fprintf(pfile, "\t\t{");
	if (WCmpiStSt(ptir->stElv, "\006elvNil") == 0 ||
			WCmpiStSt(ptir->stElv, "\013elvComboBox") == 0)
		{
		if (fNoTmcs)
			fprintf(pfile, "0 /* no TMC's */");
		else
			PrintSt(pfile, ptir->stFieldName);
		}
	else
		{
		fprintf(pfile, szIagPrefix);
		PrintSt(pfile, ptir->stFieldName);
		fprintf(pfile, ")");
		}
	fprintf(pfile, ", ");
	PrintSt(pfile, ptir->stElv);

	fprintf(pfile, ", %d}", ptir->IszElkSync);
}


WriteSyncFile()
{
	FILE * pfile;
	int isz;

	if (!fDirtySync)
		return;

	if (szSyncFile == NULL)
		Error("no sync file specified", 0);

	if ((pfile = fopen(szSyncFile, "w")) == NULL)
		Error("cannot create sync file", 0);

	for (isz = 0; isz < iszElkSyncMac; isz += 1)
		{
		if (rgszElkSync[isz] != NULL)
			fprintf(pfile, "%s %d\n", rgszElkSync[isz], isz);
		}

	fclose(pfile);
}


int
CtirFrTdr(ptdr)
/* Counts all the fields in *<ptdr> and its child dialogs.
*/
TDR *ptdr;
{
	TCR *ptcr;
	int ctirResult;

	ctirResult = ptdr->ctir;
	for (ptcr = ptdr->ptcrFirst; ptcr != 0; ptcr = ptcr->ptcrNext)
		ctirResult += PtdrFindSt(ptcr->stChildName)->ctir;

	return ctirResult;
}


TDR *PtdrFindSt(st)
unsigned char *st;
{
	TDR *ptdr;

	for (ptdr = ptdrFirst; ptdr != 0; ptdr = ptdr->ptdrNext)
		{
		if (WCmpiStSt(st, ptdr->stHidName) == 0)
			{
			/* Found it.
				*/
			return ptdr;
			}
		}
	Error("dialog not found", 0);
}


/* ---------------------------------------------------------------------- */
/* Random utilities
/* ---------------------------------------------------------------------- */

void Error(sz, wArg)
char *sz;
unsigned wArg;
{
	printf("mergeelx: ");
	printf(sz, wArg);
	printf("\n");
	exit(1);
}


void ErrSyntax()
{
	Error("syntax error", 0);
}


void AssertFailed()
{
	Error("assert failed", __LINE__);
}


unsigned char *StAtmKeyword()
{
	unsigned char *st;
	int ich;

	if (AtmtGet() != atmtKeyword)
		ErrSyntax();
	st = (unsigned char *)PbAllocCb(stVal[0] + 2);
	for (ich = 0; ich < stVal[0] + 1; ich++)
		st[ich] = stVal[ich];
	st[stVal[0] + 1] = '\0';	/* zero-terminate */
	return st;
}


unsigned char *StAtmString()
{
	unsigned char *st;
	int ich;

	if (AtmtGet() != atmtString)
		ErrSyntax();
	st = (unsigned char *)PbAllocCb(stVal[0] + 2);
	for (ich = 0; ich < stVal[0] + 1; ich++)
		st[ich] = stVal[ich];
	st[stVal[0] + 1] = '\0';	/* zero-terminate */
	return st;
}


int WCmpiStSt(st1, st2)
unsigned char *st1, *st2;
{
	unsigned cch1, cch2;

	cch1 = *st1++;
	cch2 = *st2++;
	while (cch1 != 0 && cch2 != 0)
		{
		int wDiff;
		unsigned char ch1 = *st1++, ch2 = *st2++;

		if (ch1 != ch2)
			{
			ch1 |= 0x20;
			ch2 |= 0x20;
			if (!(ch1 == ch2 && ch1 >= 'a' && ch1 <= 'z'))
				return ch1 - ch2;
			}
		cch1--;
		cch2--;
		}
	return cch1 - cch2;
}


unsigned char *StCopySt(stIn)
/* Makes a new copy of a counted string, including the zero terminator at the
* end.
*/
unsigned char *stIn;
{
	unsigned char *stOut = (unsigned char *)PbAllocCb(stIn[0] + 2);
	int ich;

	for (ich = 0; ich < stIn[0] + 1; ich++)
		stOut[ich] = stIn[ich];
	stOut[stIn[0] + 1] = '\0';		/* zero-terminate */

	return stOut;
}


/* ---------------------------------------------------------------------- */
/* File handling code
/* ---------------------------------------------------------------------- */

static FILE *pfileIn;

void OpenInput(szFilename)
char *szFilename;
{
	if ((pfileIn = fopen(szFilename, "rt")) == NULL)
		Error("can't open file \"%s\"\n", (unsigned)szFilename);

	chNext = '\0';		/* initialize lookahead */
}


FILE *PfileOpenOutput(szFilename)
char *szFilename;
{
	FILE *pfile;

	if ((pfile = fopen(szFilename, "wt")) == NULL)
		Error("can't open file \"%s\" for output", (unsigned)szFilename);
	return pfile;
}


void CloseInput()
{
	fclose(pfileIn);
}


char ChGet()
{
	return fgetc(pfileIn);
}


void PrintSt(pfile, st)
FILE *pfile;
unsigned char *st;
{
	int ich;

	for (ich = 0; ich < st[0]; ich++)
		fprintf(pfile, "%c", st[ich + 1]);
}


/* ---------------------------------------------------------------------- */
/* ELK table routines
/* ---------------------------------------------------------------------- */

InitElkTable()
{
	mpelkichName = (unsigned *)PbAllocCb(celkQuantum * sizeof(unsigned));
	elkMax = celkQuantum;
	elkMac = elkMin;
	mpelkichName[0] = 0;	/* elkNil */

	rgchElkNames = (unsigned char *)PbAllocCb(cchQuantum * sizeof(char));
	ichElkNameMax = cchQuantum;
	ichElkNameMac = 0;
}


/* AddElkSt(st) now return Isz of st - (jurgenl) */

int AddElkSt(st)
unsigned char *st;
{
	int elkFirst, elkLast, elkT;
	unsigned ichT, cch, iszSync;
	char * pchS, * pchD;
	char ch;
	char stElk [256];
	char szElkEnglish [256];

	for (pchD = stElk + 1, pchS = st + 1, cch = *st; cch-- > 0; )
		{
		ch = *pchS;
		if (ch == ':')
			break;
		pchS += 1;
		*pchD++ = ch;
		}

	stElk[0] = pchD - stElk - 1;

	if (ch == ':')
		{
		*pchS = cch;
		StToSz(pchS, szElkEnglish);
		}
	else
		{
		StToSz(stElk, szElkEnglish);
		}

	iszSync = IszSyncSz(szElkEnglish);
	/* REVIEW: do something with iszSync */

	/* Look for the name in the hash table, or the position where it
		* should be.
		*/
	elkFirst = elkMin;
	elkLast = elkMac - 1;
	while (elkFirst <= elkLast)
		{
		int elkMid = (elkFirst + elkLast) / 2;
		int wDiff;

		wDiff = WCmpiStSt(stElk, &rgchElkNames[mpelkichName[elkMid]]);
		if (wDiff == 0)
			{
			/* Found it.
				*/
			return iszSync;
			}
		else  if (wDiff < 0)
			{
			/* Table entry was bigger than the one we want.
				*/
			elkLast = elkMid - 1;
			}
		else
			{
			/* Table entry was too small.
				*/
			elkFirst = elkMid + 1;
			}
		}
	Assert(elkFirst - elkLast == 1);

	/* We will insert the new entry before elkFirst.  First move all the
		* rest of the entries forward ...
		*/
	if (elkMac + 1 > elkMax)
		{
		mpelkichName = (unsigned *)PbReallocPbCb((char *)mpelkichName,
				(elkMax + celkQuantum) *
				sizeof(int));
		elkMax += celkQuantum;
		}
	for (elkT = elkMac; elkT > elkFirst; elkT--)
		mpistiszSync[elkT] = mpistiszSync[elkT - 1];
	for (elkT = elkMac; elkT > elkFirst; elkT--)
		mpelkichName[elkT] = mpelkichName[elkT - 1];
	elkMac++;
	mpelkichName[elkFirst] = ichElkNameMac;
	mpistiszSync[elkFirst] = iszSync;

	/* Add the name to the end of the string block, growing it if
		* necessary.
		*/
	cch = stElk[0] + 1;
	while (ichElkNameMac + cch > ichElkNameMax)
		{
		rgchElkNames = (unsigned char *)
				PbReallocPbCb((char *)rgchElkNames,
				(ichElkNameMax + cchQuantum) *
				sizeof(char));
		ichElkNameMax += cchQuantum;
		}
	for (ichT = 0; ichT < cch; ichT++)
		rgchElkNames[ichElkNameMac + ichT] = stElk[ichT];
	ichElkNameMac += cch;
	return iszSync;
}


DumpElkTable()
{
	ELK elk;

	printf("TEST: ELK table dump ...\n", 0);

	for (elk = elkMin; elk < elkMac; elk++)
		{
		printf("TEST: ELK %u: \"", elk);
		PrintSt(stdout, &rgchElkNames[mpelkichName[elk]]);
		printf("\"\n");
		}

	printf("TEST: end of ELK table dump.\n");
}


StToSz(st, sz)
char * st;
char * sz;
{
	int cch;

	cch = *st++;
	while (cch-- > 0)
		*sz++ = *st++;
	*sz = '\0';
}


SzToSt(sz, st)
char * sz;
char * st;
{
	int cch;
	char * pch;

	pch = st + 1;
	while (*sz)
		*pch++ = *sz++;
	st[0] = pch - st - 1;
}




/* ---------------------------------------------------------------------- */
/* Dialog access procedures
/* ---------------------------------------------------------------------- */

ELK ElkLookupSt(st)
/* A version of this procedure will eventually be exported as part of this
* interface.
*/
unsigned char *st;
{
	int elkFirst, elkLast;

	if (fTest)
		{
		printf("TEST: final lookup of ELK: \"", 0);
		PrintSt(stdout, st);
		printf("\"\n", 0);
		}

	/* Look for the name in the hash table, or the position where it
		* should be.
		*/
	elkFirst = elkMin;
	elkLast = elkMac - 1;
	while (elkFirst <= elkLast)
		{
		int elkMid = (elkFirst + elkLast) / 2;
		int wDiff;

		if (fTest)
			{
			printf("TEST:     compare to \"", 0);
			PrintSt(stdout, &rgchElkNames[mpelkichName[elkMid]]);
			printf("\"\n", 0);
			}
		wDiff = WCmpiStSt(st, &rgchElkNames[mpelkichName[elkMid]]);
		if (wDiff == 0)
			{
			/* Found it.
				*/
			return elkMid;
			}
		else  if (wDiff < 0)
			{
			/* Table entry was bigger than the one we want.
				*/
			elkLast = elkMid - 1;
			}
		else
			{
			/* Table entry was too small.
				*/
			elkFirst = elkMid + 1;
			}
		}

	if (fTest)
		{
		printf("unknown ELK (name size = %u)\n", (unsigned)st[0]);
		DumpElkTable();
		Error("giving up ...", 0);
		}
	else
		Error("internal error: unknown ELK %u\n", (unsigned)st[0]);
}


void GetNameElk(elk, stOut)
ELK elk;
unsigned char *stOut;
{
	unsigned char *stElk = &rgchElkNames[mpelkichName[elk]];
	unsigned cch = stElk[0] + 1;

	while (--cch >= 0)
		*stOut++ = *stElk++;
}


char *
PbAllocCb(cb)
unsigned cb;
{
	char *pb;

	if ((pb = (char *)malloc(cb)) == 0)
		Error("out of memory in PbAllocCb (%d)", cb);
	return pb;
}


char *
PbReallocPbCb(pbIn, cb)
char *pbIn;
unsigned cb;
{
	char *pbOut;

	if ((pbOut = (char *)realloc(pbIn, cb)) == 0)
		Error("out of memory in PbReallocPbCb (%d)", cb);
	return pbOut;
}


/* opl.c: routines to create (and destroy) Object Property Lists
*/
/*#include <stdio.h>*/

/*#include "opl.h"*/

/* #define ReturnAtmt(atmt)	{ printf("AtmtGet() = %u\n", (atmt));	\
/* 				  return atmt; }
/**/
#define ReturnAtmt(atmt)	{ return atmt; }


ATMT AtmtGet()
{
	if (chNext == '\0')
		chNext = ChGet();

	while (1)
		{
		switch (chNext)
			{
		case EOF:
			ReturnAtmt(atmtEof);
		case '\"':
			GetStVal(TRUE);
			chNext = '\0';
			ReturnAtmt(atmtString);
		default:
			GetStVal(FALSE);
			ReturnAtmt(atmtKeyword);
		case '(':
			chNext = '\0';
			ReturnAtmt(atmtLParen);
		case ')':
			chNext = '\0';
			ReturnAtmt(atmtRParen);
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			chNext = ChGet();
			break;
			}
		}
}




void GetStVal(fQuoted)
int fQuoted;
{
	int ichVal;

	if (fQuoted)
		chNext = ChGet();

	for (ichVal = 0; ichVal < 80; ichVal++)
		{
		switch (chNext)
			{
		case EOF:
			if (fQuoted)
				Error("EOF in quoted string", 0);
			/* fall through */
		case '\n':
		case '\r':
			if (fQuoted)
				Error("newline in quoted string", 0);
			/* fall through */
		case ' ':
		case '\t':
		case '(':
		case ')':
		case '\"':
			if (!fQuoted || chNext == '\"')
				{
				int ichT;

				stVal[0] = ichVal;

				if (chNext == '\"' && fQuoted)
					chNext = '\0';
				return;
				}
			/* fall through */
		default:
			stVal[1 + ichVal] = chNext;
			chNext = ChGet();
			break;
			}
		}
	Error("string too long", 0);
}


