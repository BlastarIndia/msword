/*
    -- opustlbx : convert toolbox defintions to toolbox header
    --     and toolbox linkage definitions


 * * History * *
  11 DEC  1989  (raygr)	Added Super-Toolbox stuff
  28 JUN  1988  (ALB)	Ported to OS/2
  04 FEB  1988  (SAR)	toggle case fpr -m kludge
  22 DEC  1987  (SAR)	-m option proper NATIVE support (kludge!)
  15 DEC  1987  (SAR)	void & VOID supported
  26 OCT  1987  (RGA)   Support for new ToolBox.TM file.
  27 MAY  1987  (SAR)	make on Xenix (LINT_ARGS)
  18 MAR  1987	(SAR)	clean up (1.5 years later), -W3
			add
  23 JAN  1986	(SAR)	Change to _DATA segment, DGROUP group
  23 JAN  1986	(SAR)	Port to 286 Xenix
			  Make C procedures work properly
   3 DEC  1985	(SAR)	Add extra stuff to .h file for OS calls
  25 NOV  1985	(SAR)	fix so  () no parameters => error
  24 OCT  1985	(SAR)	move TOOLBOX object code to DATA segment
  12 SEPT 1985	(SAR)	Program inception
*/

#if D86 || OS2
#define LINT_ARGS	1		/* prototypes */
#endif

#define ver 1
#define rel 9

#define cparmMax 30

/* the max toolbox entry number.  after this number, super toolbox
   entries are made */
#define tlbxSuperMax		768

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#if D86 || OS2
#include <process.h>	/* for exit */
#endif

#include "opustlbx.h"


/* forward */
#ifdef LINT_ARGS
int	main(int, char *[]);
VOID	Usage();
VOID	InitArgsAndFiles(int*, char***);
VOID	ProcessInputFile();
VOID	CloseFiles();
VOID	BeginHOut(FILE *);
VOID	MiddleHOut(FILE *, char *, char *, BOOL, int, char *[], BOOL);
VOID	WriteParms(FILE *, int, char *[], BOOL, BOOL, BOOL);
VOID	FinishHOut(FILE *);
VOID	MiddleExtraOut(FILE *, char *);
VOID	MiddleTmOut(char *);
VOID	BeginAsmOut(FILE *);
VOID	MiddleAsmOut(FILE *, char *, BOOL);
VOID	MiddleAsmOutSuperTlbx(FILE *);
VOID	FinishAsmOut(FILE *);
VOID	InitParser(char *);
char *  SzLocalProcNameSz(char *);
char *	SzNativeProcNameSz(char *);
char *	SzCleanString(char *);
char *	SzGetTillTerm(char *);
char *  SzGetSimpleToken();
char *	SzLastWord(char **);
VOID	AddLibSz(char *);
BOOL	FSearchLibSz(char *);
BOOL	FDefinedSz(char *);
#if !(D86 || OS2)
char * strupr(char *);
#endif

#else /* no LINT args */
int	main();
VOID	Usage();
VOID	InitArgsAndFiles();
VOID	ProcessInputFile();
VOID	CloseFiles();
VOID	BeginHOut();
VOID	MiddleHOut();
VOID	WriteParms();
VOID	FinishHOut();
VOID	MiddleExtraOut();
VOID	MiddleTmOut();
VOID	BeginAsmOut();
VOID	MiddleAsmOut();
VOID	MiddleAsmOutSuperTlbx();
VOID	FinishAsmOut();
VOID	InitParser();
char *  SzLocalProcNameSz();
char *	SzNativeProcNameSz();
char *	SzCleanString();
char *	SzGetTillTerm();
char *  SzGetSimpleToken();
char *	SzLastWord();
VOID	AddLibSz();
BOOL	FSearchLibSz();
BOOL	FDefinedSz();
#if !(D86 || OS2)
char *	strupr();
#endif
#endif


/*****************************************************************************/
/* Global Variables */

char	szVoid1[]   = "void";	/* either case */
char	szVoid2[]   = "VOID";
char	szFar[]    = "FAR";	/* may be either case */
char	szPascal[] = "PASCAL";	/* may be either case */

int	tlbxMac = 0;		/* # of toolbox calls defined */
int	cprocMult = 0;		/* # of multiple word procedure types */

char	rgchBuffer[256];	/* line input */
char	rgchParsed[256];	/* copy of input line for parser */

FILE	*pfileSource,		/* source file, may be stdin */
     	*pfileHOut,		/* header output file */
     	*pfileAsmOut,		/* Asm output file */
     	*pfileExtraOut;		/* Extra header output file */

char	*szSource;		/* source file name */
int	lnSource = 0;		/* source line # */

BOOL	fExtraHeader = FALSE;	/* generate extra header ?? */
BOOL 	fError = FALSE;		/* => error in processing */
BOOL	fTypeCast = FALSE;	/* Type cast .H file */
BOOL	fTmOut = FALSE; 	/* Write .TM file to stdout */
    
/****************************************************************************/

int
main(cszArgs,rgszArgs)
/* Main procedure :
    -- get arguements, process flags, open files
    -- parse input, generate output
    -- close files & end.
*/
int  cszArgs;
char *rgszArgs[];
    {

    InitArgsAndFiles(&cszArgs, &rgszArgs);
    ProcessInputFile();
    CloseFiles();

    return(fError != 0);
    }

/****************************************************************************/

VOID
InitArgsAndFiles(pcszArgs, prgszArgs)
int  *pcszArgs;
char ***prgszArgs;
/*
   -- Parse input flags
   -- Open input files
 */
    {
    /* Get rid of opustlbx.exe name. */
    --(*pcszArgs);
    ++(*prgszArgs);

    while ((*pcszArgs)>0 
	&& ( (*prgszArgs)[0][0] == '-' || (*prgszArgs)[0][0] == '/') )
	{
	char *pch = &((*prgszArgs)[0][1]);
	while (*pch)
	    {
	    switch (*pch)
		{
	    default:
		fprintf(stderr, "opustlbx: unknown flag '-%c'.\n", *pch);
		exit(1);

	    case 't':
		fTypeCast = TRUE; 
		break;

	    case 'm':
		fTmOut = TRUE; 
		break;

		}
	    pch++;
	    }
	(*pcszArgs)--;
	(*prgszArgs)++;
	}
	
    /* -- test for proper # of arguements & open files */
    if (*pcszArgs == 3)
        {
        /* normal case */
        }
    else if ((*pcszArgs) == 4)
        fExtraHeader = TRUE;
    else
        {
        Usage();
        exit(1);
        }

    /* -- open files -- */
    szSource = (*prgszArgs)[0];
    if ((pfileSource = fopen(szSource, szROText)) == 0) 
        {
        fprintf(stderr, "opustlbx : can't open %s\n", (*prgszArgs)[0]);
        exit(1);
        }
    if ((pfileHOut = fopen((*prgszArgs)[1], szWOText)) == 0) 
	{
	fprintf(stderr, "opustlbx : can't open %s\n", (*prgszArgs)[1]);
	exit(1);
	}
    if ((pfileAsmOut = fopen((*prgszArgs)[2], szWOText)) == 0) 
	{
	fprintf(stderr, "opustlbx : can't open %s\n", (*prgszArgs)[2]);
	exit(1);
	}
    if (fExtraHeader && (pfileExtraOut =fopen((*prgszArgs)[3], szWOText)) == 0) 
	{
	fprintf(stderr, "opustlbx : can't open %s\n", (*prgszArgs)[3]);
	exit(1);
	}

    BeginHOut(pfileHOut);
    BeginAsmOut(pfileAsmOut);

    if (fExtraHeader)
	BeginHOut(pfileExtraOut);
    }


/****************************************************************************/

VOID
ProcessInputFile()
/*
  -- Read the input file and write the output file(s).
 */
    {
    char *szInput;		/* input line */
    char chTerm;		/* parsed terminator */

    char *szType;		/* type returned by Procedure */
    BOOL fSimpleType;		/* => szType is 1 word */
    char *szProcName;		/* Procedure name */
    char *rgszParmType[cparmMax];	/* Parameter type names */
    int  cparm;			/* # of parameters for procedure */
    char *szKeyword;		/* Keyword to check for Far / or PASCAL */

    BOOL fPascal = FALSE;	/* TRUE=> PASCAL specified */

#ifdef NODEFINE
    BOOL fNoDefine = FALSE;	/* TRUE=> Do not put out  #define, just
				 * copy source line verbatim. */
#endif

    while ((szInput=fgets(rgchBuffer, sizeof(rgchBuffer), pfileSource)) !=NULL )
	{
#	ifdef NODEFINE
		fNoDefine = FALSE;
#	endif
	lnSource++;
	strcpy(rgchParsed, szInput);
	InitParser(rgchParsed);
	/* process 1 line */
	if ((szType = SzGetTillTerm(&chTerm))==NULL)
	    continue;		/* get next line */

	if (chTerm != '(')	/* must be terminated by starting '(' */
	    {
	    fprintf(stderr, 
		"opustlbx : Invalid declaration in line %d of %s : '%s'\n",
		lnSource, szSource, szInput);
	    fError = TRUE;
	    continue;
	    }

	/* at this point szType => type and name */
	szProcName = SzLastWord(&szType);	/* break up */
	if (szProcName==NULL)
	    {
	    fprintf(stderr,
		"opustlbx : Invalid procedure name in line %d of %s : '%s'\n",
		lnSource, szSource, szInput);
	    fError = TRUE;
	    continue;
	    }

	szKeyword = strupr(SzLastWord(&szType));
		/* Either PASCAL or FAR */

	if (fPascal = (strcmp(szPascal,szKeyword)==0))
	    /* name is pascal */
	    szKeyword = strupr(SzLastWord(&szType));	/* it better be FAR */

	if (strcmp(szFar,szKeyword)!=0)
	    {
	    fprintf(stderr,
	    "opustlbx: missing FAR/far before procedure name in line %d of %s : '%s'\n",
		lnSource, szSource, szInput);
	    fError = TRUE;
	    continue;
	    }

	fSimpleType = (strchr(szType,' ')==NULL) && (*szType!='\0');

	/* Get the parameters from the parameter list */
	cparm = 0;
	chTerm = 0;
        while (cparm < cparmMax && chTerm !=')')
	    {
	    if ((rgszParmType[cparm] = SzGetTillTerm(&chTerm)) == NULL ||
		(chTerm != ',' && chTerm != ')'))
		{
		fprintf(stderr,
		    "opustlbx : Invalid parameter list in line %d of %s : '%s'\n",
		    lnSource, szSource, szInput);
		fError = TRUE;
		goto GetNextLine;	/* Oh, for a labeled break */
		}
            if (*rgszParmType[cparm] == '\0')
		{
		fprintf(stderr,
		    "opustlbx : Empty parameter list in line %d of %s :\n",
		    lnSource, szSource);
		fprintf(stderr, "\t: '%s'\n", szInput);
		fError = TRUE;
		continue;
		}
	    cparm++;
	    } /* end while */

#ifdef NODEFINE
	/* 
	   -- See if there is a NoDefine entry at the end of the line
	 */
	{
	char* sz = SzGetSimpleToken();

	while (sz != NULL)
	    	{
		if (sz[0]=='/' && sz[1]=='*')
			{
			/* 
			  -- We have the start of a comment.
			 */
			if (sz[2]=='\0')
				sz = SzGetSimpleToken();
			else
				sz = &sz[2];

			if (sz != NULL)
				{
				char *pch = sz;
				while (isalpha(*pch))
					++pch;
				*pch = '\0';
				if (strcmpi("NODEFINE", sz) == 0)
					{
					fNoDefine = TRUE; 
					}
				}
			break;
			}
		sz = SzGetSimpleToken();
		}	
	}
#endif

	if (cparm >= cparmMax)
	    {
	    fprintf(stderr,
		"opustlbx : Too many parameters in line %d of %s : %s\n",
		lnSource, szSource, szInput);
	    fError = TRUE;
	    continue;
	    }

	/* test to see if 1 parameter == "void" */
	if (cparm == 1 &&
	  (strcmp(rgszParmType[0], szVoid1) == 0 ||
	  strcmp(rgszParmType[0], szVoid2) == 0))
	    cparm = 0;

	/* send data to .H files */
#ifdef NODEFINE
	if (fNoDefine)
	    {
	    if (!fPascal)
		{
		fprintf(stderr,
		    "/* NODEFINE */ procedures must be pascal. %s is not.\n",
		    szProcName);
		}
	    else
		{
		/*
		  -- Just output the source line to the .H file as is!
		 */
		fprintf(pfileHOut, szInput);
		}
	    }
	else
#endif
	    {
	    MiddleHOut(pfileHOut, szProcName, szType, fSimpleType, cparm,
		rgszParmType, fPascal);
	    }

	/* send data to .ASM file */
	if (tlbxMac == tlbxSuperMax - 1)
		MiddleAsmOutSuperTlbx(pfileAsmOut);
	
	MiddleAsmOut(pfileAsmOut, szProcName, fPascal);
	
	if (fExtraHeader)
	    MiddleExtraOut(pfileExtraOut, szProcName);
	
	if (fTmOut)
	    MiddleTmOut(szProcName);

	tlbxMac++;

	/* continue input loop here */
GetNextLine:
	;
	} /* end while input */

   }

 
/****************************************************************************/

VOID
CloseFiles()
/*
   -- Close all open files
 */
    {
    FinishHOut(pfileHOut);
    FinishAsmOut(pfileAsmOut);

    fclose(pfileSource);
    fclose(pfileHOut);
    fclose(pfileAsmOut);
    if (fExtraHeader)
	fclose(pfileExtraOut);
    }

/****************************************************************************/

VOID
Usage()
/* Print usage message.
 */
    {
    fprintf(stderr,"opustlbx Version %d.%d: Usage:\n", ver, rel);
    fprintf(stderr,
	"\topustlbx [-mt] source_in header_out asm_out [extra_header_file]\n");
    }


/****************************************************************************/

VOID BeginHOut(pfile)
/* Add header to .H file */
FILE *pfile;
	{
        fprintf(pfile, 
    	"/* HEADER FILE - created by OPUSTLBX %d.%d */\n", ver, rel);
        fprintf(pfile, "/* * * Must compile with CS ! ! * * */\n");
	}

/****************************************************************************/

VOID MiddleHOut(pfile, szName, szType, fSimpleType, cparm, rgszParm, fPasc)
/* 
  -- Add middle to .H file, which is passed in as pfile.
*/
FILE *pfile;			/* file to put stuff */
char *szName;			/* procedure name */
char *szType;			/* Type of procedure */
BOOL fSimpleType;		/* => simple Type (1 word) */
int  cparm;			/* # of parameters */
char *rgszParm[];		/* the parameters (i.e. types) */
BOOL fPasc;			/* 1=> Pascal, 0=> C parm. passing convention */
    {
    int  iparm;			/* index for parameter */
    char *szLocal = SzLocalProcNameSz(szName);		/* _ prefix */
    char *szNative = SzNativeProcNameSz(szName);	/* all uppercase */
    int tlbxOut;

    if (!fTmOut)
	{
	/* tlbx calls only with the old switches. */
	if (fSimpleType)
	    {
	    if (!FDefinedSz(szType))
		fprintf(pfile, "tlbx %s %sTLBX();\n", szType, szType);
	    }
	else
	    {
	    fprintf(pfile, "tlbx %s T%dTLBX();\n", szType, cprocMult);
	    }
	}

    fprintf(pfile, "#define %s(", szName);

    /* -- add parm list - always add forward P1, P2, ... */
    for (iparm = 0; iparm < cparm; iparm++)
	fprintf(pfile, (iparm != cparm-1) ? "P%d," : "P%d", iparm+1);

    if (!fTmOut)
	{
        /* don't go past tlbx limit */
        if (tlbxMac >= tlbxSuperMax - 1)
        	{
        	if (!fPasc)
			{
			fprintf(stderr, "opustlbx error : cannot use super-toolbox for non-PASCAL entries.\n");
			exit(3);
			}
        	tlbxOut = tlbxSuperMax - 1;
        	}
        else
        	tlbxOut = tlbxMac;
        
        if (fSimpleType)
            fprintf(pfile, ") %sTLBX(%d", szType, tlbxOut);
        else
            fprintf(pfile, ") T%dTLBX(%d", cprocMult++, tlbxOut);

	/* -- add parm list again, forwards if pascal, backwards if C */
	WriteParms(pfile, cparm, rgszParm, fPasc, !fTmOut, TRUE);
        
        /* write actual tlbx value at end of parm list for 
           super-toolbox entries 
        */
        if (tlbxMac >= tlbxSuperMax - 1)
		fprintf(pfile, ", %d)\n", tlbxMac + 1);
	else
		fprintf(pfile, ")\n");
	}
    else
	{
	/* special case for native */
	/* "#define xxx(...) (__FNATIVE__ ? XXX(...) : _xxx(...))" */

	fprintf(pfile, ") (__FNATIVE__ ? %s(", szNative);
	WriteParms(pfile, cparm, rgszParm, fPasc, !fTmOut, TRUE);
	fprintf(pfile, ") : \\\n");
	fprintf(pfile, "\t%s(", szLocal);
	WriteParms(pfile, cparm, rgszParm, fPasc, !fTmOut, TRUE);
	fprintf(pfile, "))\n");
	}


    if (fTmOut)
	{
	/* Pcode definition */
	fprintf(pfile, "%s far pascal %s(", szType, szLocal);
	WriteParms(pfile, cparm, rgszParm, fPasc, !fTmOut, FALSE);
        fprintf(pfile, ");\n");
	/* Native definition */
	fprintf(pfile, "%s far pascal %s(", szType, szNative);
	WriteParms(pfile, cparm, rgszParm, fPasc, !fTmOut, FALSE);
        fprintf(pfile, ");\n");
	}

    free(szLocal);
    free(szNative);
    }


/****************************************************************************/


VOID WriteParms(pfile, cparm, rgszParm, fForwards, fInitialComma, fArgs)
/*
  -- Write the parameters to the given file, possibly backwards
 */
FILE *pfile;
int  cparm;
char *rgszParm[];		/* the parameters (i.e. types) */
BOOL fForwards;
BOOL fInitialComma;
BOOL fArgs;
    {
    int iparm;
    if (fInitialComma && cparm != 0)
	{
	fprintf(pfile, ", ");
	}
    for (iparm = 0; iparm < cparm; iparm++)
	{
	int  nparm;
	/* set nparm to one based parameter number */
	nparm = (fForwards) ? iparm+1 : cparm-iparm;

	if (fTypeCast)
	    {
	    if (fArgs)
		fprintf(pfile, "(");
	    fprintf(pfile, "%s", rgszParm[nparm-1]);
	    if (fArgs)
		fprintf(pfile, ") ");
	    }
	if (fArgs)
	    fprintf(pfile, "(P%d)", nparm);
	if (iparm < cparm-1)
	    fprintf(pfile, ", ");
	}
    }

/****************************************************************************/

VOID FinishHOut(pfile)
/* Add end to .H file */
FILE *pfile;
    	{
	int tlbx = tlbxMac < tlbxSuperMax - 1 ? tlbxMac : tlbxMac + 1;
	fprintf(pfile, "\n#define tlbxMac %d\n", tlbx);
	fprintf(pfile, "\n/* OS CALL DEFINITONS */\n");
	fprintf(pfile, "sys void VSYS_MAKETOOL();\n");
	fprintf(pfile, "#ifndef exit\n");
	fprintf(pfile, "#define exit(ex) VSYS_MAKETOOL(4, (int) ex);\n");
	fprintf(pfile, "#endif\n");
	fprintf(pfile, "#ifndef DebugBreak\n");
	fprintf(pfile, "#define DebugBreak(bn) VSYS_MAKETOOL(1, (int) bn);\n");
	fprintf(pfile, "#endif\n");
	fprintf(pfile, "\n/* END OF HEADER FILE */\n");
	}

/****************************************************************************/

VOID MiddleExtraOut(pfile, szName)
/*
  -- Add middle to extra .H file
  -- tlbxMac == toolbox index
*/
FILE *pfile;			/* file to put stuff */
char *szName;			/* procedure name */
    {
    int tlbx = tlbxMac < tlbxSuperMax - 1 ? tlbxMac : tlbxMac + 1;
    fprintf(pfile, "#define tlbx%s %d\n", szName, tlbx);
    }

/****************************************************************************/

VOID MiddleTmOut(szName)
/*
  -- Write an entry in the for the .TM file.
  -- tlbxMac == toolbox index
*/
char *szName;
    {
    char *sz = SzLocalProcNameSz(szName);
    int tlbx = tlbxMac < tlbxSuperMax - 1 ? tlbxMac : tlbxMac + 1;
    fprintf(stdout, "%s %d\n", sz, tlbx);
    free(sz);
    }

/****************************************************************************/

VOID BeginAsmOut(pfile)
/* Add header to .ASM file */
FILE *pfile;
    {
    fprintf(pfile, ";* TOOLBOX ASM FILE - created by OPUSTLBX %d.%d\n",
	ver, rel);
    fprintf(pfile, ";* * * Must Assemble with MASM * * *\n");
    fprintf(pfile, "DGROUP\tGROUP\t_DATA\n");
    fprintf(pfile, "_DATA\tSEGMENT\tWORD PUBLIC 'DATA'\n");
    fprintf(pfile, "\tASSUME\tCS:DGROUP\t\t;* DATA in DGROUP\n");
    fprintf(pfile, "\tPUBLIC\tmptlbxpfn,tlbxMac\n");
    fprintf(pfile, "mptlbxpfn:\n");
    fprintf(pfile, ";* toolbox table follows :\n");
    fprintf(pfile, ";*\n");
    }

/****************************************************************************/

VOID MiddleAsmOut(pfile, szName, fPasc)
/* Add middle to .ASM file */
FILE *pfile;		/* file to put stuff */
char *szName;		/* procedure name */
BOOL fPasc;		/* 0 => C => prepend '_' to external name */
    {
    fprintf(pfile, (fPasc) ? "\tEXTRN\t%s:FAR\n\tDD\t%s\n"
	: "\tEXTRN\t_%s:FAR\n\tDD\t_%s\n", szName, szName);
    }


VOID MiddleAsmOutSuperTlbx(pfile) 
/* Add SuperTlbx to .ASM file */
FILE *pfile;		/* file to put stuff */
    {
    fprintf(pfile, "\tDD\tSuperTlbx\n");
    }

/****************************************************************************/

VOID FinishAsmOut(pfile)
/* Add end to .ASM file */
FILE *pfile;
    {
    fprintf(pfile, ";*\n");
    fprintf(pfile, "tlbxMac\tEQU\t%d\n", tlbxMac);
    fprintf(pfile, ";*\n");
    fprintf(pfile, "_DATA\tENDS\n");

    if (tlbxMac > tlbxSuperMax)
    	{
	/* write the SuperTlbx asm routine */
	/* OS/2: Note: we can't just do a far jmp to an entry point running
	   at a higher privilege level. We need to use a call instruction 
	   to go thru a call-gate.   So all kernel and PM API calls must
	   be in the first 768 toolbox entries.  THEY CAN NOT GO THRU 
	   THIS SuperTlbx ROUTINE!
	*/
	fprintf (pfile,"WINTER2 SEGMENT WORD PUBLIC 'CODE'\n");
	fprintf (pfile,"        ASSUME CS:WINTER2\n\n");

	fprintf (pfile,"        PUBLIC SuperTlbx\n\n");
	fprintf (pfile,"SuperTlbx:\n\n");
	
	fprintf (pfile,"        POP CX                 ; Save return address .\n");
	fprintf (pfile,"        POP DX                 ; Save return address .\n");
	
	fprintf (pfile,"        POP BX                    ; Pop toolbox index.\n");
	
	fprintf (pfile,"        PUSH DX                 ; push return address .\n");
	fprintf (pfile,"        PUSH CX                 ; push return address .\n");
	
	fprintf (pfile,"        SHL BX,1                  ; Adjust toolbox index.\n");
	fprintf (pfile,"        SHL BX,1                  ; Adjust toolbox index.\n");
	fprintf (pfile,"        JMP DWORD PTR (DS:mptlbxpfn[BX])   ; Jump to appropriate toolbox routine.\n");
	
	fprintf (pfile,"WINTER2 ENDS\n\n");
	fprintf (stdout,"OPUSTLBX : Add the line \"WINTER2 FIXED PRELOAD PURE\" to WINPROJ.DEF.\n");
	}

    fprintf(pfile, "\tEND\n");
    }

/****************************************************************************/

/* -- Cheap Parser routines -- */


static char *szParser;				/* Parser pointer */

/****************************************************************************/

VOID InitParser(szStart)
char *szStart;
    {
    szParser = szStart;
    }

/****************************************************************************/

char * SzCleanString(sz)
/*
   -- remove any leading or trailing blanks from a string
*/
char *sz;
    {
    char *szEnd;	/* points to last character in the string */
    while (*sz== ' ')
	sz++;
    szEnd = sz + strlen(sz) - 1;
    while (*szEnd == ' ')
	*szEnd-- = '\0';
    return(sz);
    }

/****************************************************************************/

char * SzGetTillTerm(pchTerm)
/*
   -- get valid characters ( alpha &/or number &/or whitespace &/or '*' ) until
	terminator.
   -- convert whitespace to blanks
   -- skip over leading or trailing blanks
   -- return NULL if end of string and no terminator, otherwise
	answer the beginning of the next "token"
   -- return terminating character in *pchTerm
*/
char *pchTerm;		/* where to put the terminator */
    {
    char *szBegin;
    int  cchToken = 0;		/* length of token */
    char ch;			/* parsed character */

    szBegin = szParser;
    while (1)
	{
	if ((ch = *szParser) == '\0')
	    return(NULL);
	szParser++;

	if (ch == '\t' || ch == '\n')
	    *(szParser-1) = ch = ' ';		/* convert tab to blank */
	if (ch == ' ' && cchToken == 0)
	    continue;				/* skip over leading blanks */

	if (isalnum(ch) || ch == ' ' || ch == '*' || ch == '_')
	    /* part of proper token */
	    cchToken++;
	else
	    {
	    /* Termination */
	    *pchTerm = ch;
	    *(szParser-1) = '\0';	/* make null terminated string */
	    return(SzCleanString(szBegin));
	    }
	} /* end while */
    }

/****************************************************************************/

char * SzGetSimpleToken()
/*
  -- Essentially a simplified version of SzGetTillTerm.
  -- Answer a pointer to the next white space delimited token, or NULL if
     there isn't one.
  -- Turn that token into a null terminated string.
 */
    {
    char *szBegin;

    if (*szParser == '\0')
	return NULL;

    while (isspace(*szParser))
	if (*(++szParser) == '\0')
		return NULL;

    szBegin = szParser;
    while (!isspace(*szParser))
	if (*szParser == '\0')
		return szBegin;
	else
		++szParser;

    *szParser++ = '\0';
    return szBegin;
    }

/****************************************************************************/

char * SzLastWord(pszOld)
/*
   -- get last word of string
   -- words are delimited by ' '
   -- patch up old string to reflect the removal of the last word
   -- both returned strings will not contain leading/trailing blanks
*/
char **pszOld;	/* pointer to old string */
    {
    char *szOld;	/* local copy of old string */
    char *szEnd;	/* end pointer */
    int  cbOld;		/* old length */

    szOld = SzCleanString(*pszOld);
    cbOld = strlen(szOld);
    szEnd = szOld + cbOld - 1;
    while (szEnd >= szOld)
	{
	if (*szEnd == ' ')
	    {
	    *szEnd = '\0';
	    *pszOld = SzCleanString(szOld);
	    return(szEnd+1);
	    }
	else
	    szEnd--;
	}
    /* otherwise use the full string */
    *pszOld = szOld + cbOld;		/* point to last '\0' */
    return(szOld);
    }


/****************************************************************************/

char * SzLocalProcNameSz(szProcName)
/*
  -- return the proc name that we will #define szProcName to
*/
char * szProcName;
	{
	char *pch = (char*) malloc(strlen("_")+strlen(szProcName)+1);
	strcpy(pch, "_");
	strcat(pch, szProcName);
	return pch;
	}


char * SzNativeProcNameSz(szProcName)
/*
  -- return a native version of the #define'd szProcName
  -- DOUBLE KLUDGE : we use the fact that links are case insensitive to define
	the native entry.
	Since all upper case names are sometimes present, we toggle the case of
	each character.
*/
char * szProcName;
	{
	char * sz;
	char * pch;

	sz = strdup(szProcName);
	for (pch = sz; *pch != '\0'; pch++)
		*pch = isupper(*pch) ? tolower(*pch) : toupper(*pch);
	return sz;
	}


/****************************************************************************/

/* -- Library Routines for keeping types -- */


#define iszLibMax 100
#define cbLibMax 10000

static char grpszLib[cbLibMax];		/* pool area for library */
static char *pbLib = grpszLib;			/* pointer to free pool area */
static int  iszLibMac = 0;			/* count of items */
static int  cbLibFree = sizeof(grpszLib);	/* # bytes free in pool */
static char *rgszLib[iszLibMax];	/* Library item pointers */

/****************************************************************************/

VOID AddLibSz(sz)
/* add library item */
char *sz;
    {
    int  cb;

    cb = strlen(sz) + 1;	/* need room for '\0' */
    if (cb > cbLibFree)
	{
	fprintf(stderr, "opustlbx error : have run out of library space\n");
	exit(3);
	}
    if (iszLibMac >= iszLibMax-1)
	{
	fprintf(stderr, 
	    "opustlbx error : have run out of library item space\n");
	exit(3);
	}
    rgszLib[iszLibMac++] = pbLib;
    strcpy(pbLib, sz);
    pbLib += cb;
    cbLibFree -= cb;
    }

/****************************************************************************/

BOOL FSearchLibSz(sz)
/* search for library item - return true if found */
char *sz;
    {
    int  iszLib;

    for (iszLib=0; iszLib < iszLibMac; iszLib++)
	if (strcmp(sz, rgszLib[iszLib]) == 0)
	    return(1);
    return(0);
    }

/****************************************************************************/

BOOL FDefinedSz(sz)
/* see if item is defined :
    -- if defined return true
    -- if undefined then add to library then return false
*/
char *sz;
    {
    if (FSearchLibSz(sz))
	return TRUE;
    /* define item */
    AddLibSz(sz);
    return FALSE;
    }

/****************************************************************************/

/* Extra stuff for Xenix library deficiencies */
#if !(D86 || OS2)
char *
strupr(sz)
char * sz;
	{
	char * szRet = sz;
	while (*sz++ = toupper(*sz));
	return (szRet);
	}
#endif /* Xenix */


