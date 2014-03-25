/* mkdlg -- Generate parsing tables for el from dialog descriptions */

#ifdef COMMENT
Usage:	
mkdlg @elx.txt

Generates:

RNG rgrng [] = 
{
		{ 0, 1 },
		/* 2 Radio buttons */
	{ 0, 2 },
		/* Checkbox, 3 radio buttons */
	{ 0, 3 },
		/* 4 Radio buttons */
	{ 0, 4 },
	/* 5 Radio buttons */
	...
};


	int mphidibstRng [] = { 
	-1, 0, 3, ... 	};


	int rgbstRng [] = { 
	0, ... 	};


/* The high bit of each of these bytes is an fAllowNinch flag */
CHAR grpstRng [] =
{
	1, 0,
			3, 4, 5, 2,
			...
};


Each dialog has an stRng associated with it.  This st contains one
character for each cab item that is used as an index into rgrng.

CheckBoxes	0, 1, 0xffff
ListBoxes	0..32767, 0xffff
RadioButtons	0..n-1, 0xffff
ParsedEdits	n..m
Strings		-32768..32767

#endif /* COMMENT */


#include <stdio.h>
#include <ctype.h>


typedef unsigned char CHAR;
typedef unsigned uns;
typedef void (* PFN )();
typedef FILE * PFL;
typedef CHAR * SZ;
typedef int F;
typedef void * PV;

#define fFalse	0
#define fTrue	1

extern PFL fopen();
extern PV malloc();

SZ SzAlloc();


PFL pflCur = NULL;


typedef struct _rng
	{
	int wLow;	/* wLow == wHigh == 0 implies string */
	int wHigh;
	int wMult;
	int wDiv;
	F fAllowNinch;
} RNG;


#define irngMax		256	 /* because index is a char */
RNG rgrng [irngMax] = { 
	0 };


int irngMac = 1; /* 0 is reseverd for wLow==0:wHigh==0 (string) */


#define ibstMax		512	/* arbitrary */
	int rgbstRng [ibstMax] = { 
	0 	};


int ibstMac = 0;

SZ mpibstrnghid [ibstMax] = { 
	0 };


#define ibstNil		-1


#define bstMaxRng	4096	/* arbitary */
CHAR grpstRng [bstMaxRng] = { 
	0 };


int bstMacRng = 0;


char * mptksz [] =
	{
	"edit",
	"list_box",
	"check_box",
	"radio_group",
	"help_id",
	"text",
	/* add new tmt's here */

	"parse_proc",
	"return_string",
	"radio_button",
	"no_ninch",
	"combo_atomic",
	"sz_from_cab",
	"low",
	"high",
	"mult",
	"div",
	"date_type",

	"`optAnyUnit`",
	"`optLineUnit`",
	"`optPosUnit`",
	"`optPosNZUnit`",
	"`optAutoAnyUnit`",
	"`optAutoPosUnit`",
	"`optAutoLineUnit`",
	"`optPosLineUnit`",

	"`optAnyInt`",
	"`optPosInt`",
	"`optPosNZInt`",
	"`optAutoAnyInt`",
	"`optAutoPosInt`",

	"multi_selectable",

	/* add new options here */
	};


#define tkNil			-1
#define tkEof			-2

#define tkEdit			0
#define tkListBox		1
#define tkCheckBox		2
#define tkRadioGroup		3
#define tkHelpId		4
#define tkText			5

#define tkLimTmt		6

#define tkParseProc		(tkLimTmt + 0)
#define tkReturnString		(tkLimTmt + 1)
#define tkRadioButton		(tkLimTmt + 2)
#define tkNoNinch		(tkLimTmt + 3)
#define tkComboAtomic		(tkLimTmt + 4)
#define tkSzFromCab		(tkLimTmt + 5)
#define tkLow			(tkLimTmt + 6)
#define tkHigh			(tkLimTmt + 7)
#define tkMult			(tkLimTmt + 8)
#define tkDiv			(tkLimTmt + 9)
#define tkDateType		(tkLimTmt + 10)

#define tkOptAnyUnit		(tkLimTmt + 11)
#define tkOptLineUnit		(tkLimTmt + 12)
#define tkOptPosUnit		(tkLimTmt + 13)
#define tkOptPosNZUnit		(tkLimTmt + 14)
#define tkOptAutoAnyUnit	(tkLimTmt + 15)
#define tkOptAutoPosUnit	(tkLimTmt + 16)
#define tkOptAutoLineUnit	(tkLimTmt + 17)
#define tkOptPosLineUnit	(tkLimTmt + 18)

#define tkOptAnyInt		(tkLimTmt + 19)
#define tkOptPosInt		(tkLimTmt + 20)
#define tkOptPosNZInt		(tkLimTmt + 21)
#define tkOptAutoAnyInt		(tkLimTmt + 22)
#define tkOptAutoPosInt		(tkLimTmt + 23)

#define tkMultiSelectable	(tkLimTmt + 24)

#define tkLim			(tkLimTmt + 25)


#define FOptUnitTk(tk) ((tk) >= tkOptAnyUnit && (tk) <= tkOptPosLineUnit)


int tkPush = tkNil;
char szToken [256] = "";


main(argc, argv)
int argc;
SZ argv [];
{
	if (argc != 3)
		UsageExit();

	if (argv[1][0] == '@')
		ParseFileList(argv[1] + 1);
	else
		ParseFile(argv[1]);

	WriteOutput(argv[2]);
}


UsageExit()
{
	printf("Usage: MKDLG (file | @file-list) out-file\n");
	exit(1);
}


ParseFileList(szName)
SZ szName;
{
	PFL pfl;
	CHAR szLine [128];

	if ((pfl = fopen(szName, "r")) == NULL)
		{
		printf("MKDLG: Cannot open %s!\n", szName);
		exit(1);
		}

	while (fgets(szLine, sizeof (szLine), pfl) != NULL)
		{
		extern char * strchr();
		char * pchT;

		if ((pchT = strchr(szLine, '.')) != NULL)
			*pchT = '\0';
		ParseFile(szLine);
		}

	fclose(pfl);
}


void ParseEdit(), ParseListBox(), ParseCheckBox();
void ParseRadioGroup(), ParseHelpId(), ParseText();

PFN mptktmtpfn [] = 
{
	ParseEdit,
			ParseListBox,
			ParseCheckBox,
			ParseRadioGroup,
			ParseHelpId,
			ParseText
};


void ParseHelpId(prng)
RNG * prng;
{
	prng->wLow = -1;
	prng->wHigh = -1;
	TkFetch();
	mpibstrnghid[ibstMac] = SzAlloc(szToken);
}


ParseFile(szName)
SZ szName;
{
	int tk;
	int irngDlg, crngDlg;
	RNG rgrngDlg [64];
	CHAR * pch;
	F fAllowNinch;

	if ((pflCur = fopen(szName, "r")) == NULL)
		{
		char szBuf [128];

		strcpy(szBuf, szName);
		strcat(szBuf, ".des");
		if ((pflCur = fopen(szBuf, "r")) == NULL)
			{
			printf("MKDLG: Cannot open %s!\n", szName);
			exit(1);
			}
		}

	irngDlg = 0;
	crngDlg = 0;
	while ((tk = TkFetch()) != tkEof)
		{
		if ((uns) tk < tkLimTmt)
			{
			rgrngDlg[irngDlg].wLow = 0;
			rgrngDlg[irngDlg].wHigh = 0;
			rgrngDlg[irngDlg].wMult = 0;
			rgrngDlg[irngDlg].wDiv = 0;
			rgrngDlg[irngDlg].fAllowNinch = fTrue;

			(*mptktmtpfn[tk])(&rgrngDlg[irngDlg]);

			if (rgrngDlg[irngDlg].wLow == -1 &&
					rgrngDlg[irngDlg].wHigh == -1)
				{
				/* Do not generate a value for this! */
				continue;
				}

			if (rgrngDlg[irngDlg].wLow == 1 &&
					rgrngDlg[irngDlg].wHigh == 1)
				{
				/* Do not generate a value for this! */
				crngDlg += 1;
				}

			irngDlg += 1;
			crngDlg += 1;
			}
		}

	pch = grpstRng + bstMacRng;
	*pch++ = crngDlg;

	rgbstRng[ibstMac++] = bstMacRng;
	bstMacRng += crngDlg + 1;

	/* String items first... */
	for (irngDlg = 0; irngDlg < crngDlg; irngDlg += 1)
		{
		if (rgrngDlg[irngDlg].wLow == 0 && 
				rgrngDlg[irngDlg].wHigh == 0)
			{
			*pch++ = 0;
			}
		}

	/* Then the others... */
	for (irngDlg = 0; irngDlg < crngDlg; irngDlg += 1)
		{
		int wLow, wHigh, wMult, wDiv;

		wLow = rgrngDlg[irngDlg].wLow;
		wHigh = rgrngDlg[irngDlg].wHigh;
		wMult = rgrngDlg[irngDlg].wMult;
		wDiv = rgrngDlg[irngDlg].wDiv;

		if (wLow != 0 || wHigh != 0)
			{
			*pch++ = IrngFromWWWW(wLow, wHigh, wMult, wDiv) | 
					(rgrngDlg[irngDlg].fAllowNinch ? 0x80 : 0);

			if (wLow == 1 && wHigh == 1)
				*pch++ = 0; /* DTTM */
			}
		}

	fclose(pflCur);
	pflCur = NULL;
}


IrngFromWWWW(wLow, wHigh, wMult, wDiv)
int wLow, wHigh, wMult, wDiv;
{
	int irng;

	for (irng = 0; irng < irngMac; irng += 1)
		{
		if (wLow == rgrng[irng].wLow && wHigh == rgrng[irng].wHigh &&
				wMult == rgrng[irng].wMult && wDiv == rgrng[irng].wDiv)
			return irng;
		}

	rgrng[irng].wLow = wLow;
	rgrng[irng].wHigh = wHigh;
	rgrng[irng].wMult = wMult;
	rgrng[irng].wDiv = wDiv;

	irngMac += 1;

	return irng;
}



HandleHighLowMultDiv(prng, tk)
RNG * prng;
int tk;
{
	int w;

	TkFetch(); /* REVIEW: should make sure it's tkNil! */
	w = atoi(szToken);

	switch (tk)
		{
	case tkLow:
		prng->wLow = w;
		break;

	case tkHigh:
		prng->wHigh = w;
		break;

	case tkMult:
		prng->wMult = w;
		break;

	case tkDiv:
		prng->wDiv = w;
		break;
		}
}


void ParseText(prng)
RNG * prng;
{
	int tk;

	prng->wLow = prng->wHigh = -1;

	for (;;)
		{
		tk = TkFetch();
		switch (tk)
			{
		default:
			tkPush = tk;
			return;

		case tkNil:
			break;

		case tkSzFromCab:
			prng->wLow = prng->wHigh = 0;
			break;
			}
		}
}


void ParseEdit(prng)
RNG * prng;
{
	int tk;

	for (;;)
		{
		tk = TkFetch();

		switch (tk)
			{
		default:
			tkPush = tk;
			return;

		case tkNil:
			break;

		case tkNoNinch:
			prng->fAllowNinch = fFalse;
			break;

		case tkParseProc:
			prng->wLow = -32768;
			prng->wHigh = 32767;
			break;

		case tkLow:
		case tkHigh:
		case tkMult:
		case tkDiv:
			HandleHighLowMultDiv(prng, tk);
			break;

		case tkOptAnyUnit:
			prng->wMult = 20;
			prng->wLow = -31681;
			prng->wHigh = 31681;
			break;

		case tkOptLineUnit:
			prng->wMult = 20;
			prng->wLow = -31681;
			prng->wHigh = 31681;
			break;

		case tkOptPosUnit:
			prng->wMult = 20;
			prng->wLow = 0;
			prng->wHigh = 31681;
			break;

		case tkOptPosNZUnit:
			prng->wMult = 20;
			prng->wLow = 1;
			prng->wHigh = 31681;
			break;

		case tkOptAutoAnyUnit:
			prng->wMult = 20;
			prng->wLow = -31681;
			prng->wHigh = 31681;
			break;

		case tkOptAutoPosUnit:
			prng->wMult = 20;
			prng->wLow = 0;
			prng->wHigh = 31681;
			break;

		case tkOptAutoLineUnit:
			prng->wMult = 20;
			prng->wLow = -31681;
			prng->wHigh = 31681;
			break;

		case tkOptPosLineUnit:
			prng->wMult = 20;
			prng->wLow = 0;
			prng->wHigh = 31681;
			break;


		case tkOptAnyInt:
			break;

		case tkOptPosInt:
			prng->wLow = 0;
			break;

		case tkOptPosNZInt:
			prng->wLow = 1;
			break;

		case tkOptAutoAnyInt:
			break;

		case tkOptAutoPosInt:
			prng->wLow = 0;
			break;


		case tkDateType:
			prng->wLow = prng->wHigh = 1;
			break;
			}
		}
}


void ParseRadioGroup(prng)
RNG * prng;
{
	int tk;

	prng->wLow = 0;
	prng->wHigh = -1;
	for (;;)
		{
		tk = TkFetch();
		switch (tk)
			{
		default:
			tkPush = tk;
			return;

		case tkNoNinch:
			prng->fAllowNinch = fFalse;
			break;

		case tkNil:
			break;

		case tkRadioButton:
			prng->wHigh += 1;
			break;

		case tkLow:
		case tkHigh:
		case tkMult:
		case tkDiv:
			HandleHighLowMultDiv(prng, tk);
			break;
			}
		}
}


void ParseCheckBox(prng)
RNG * prng;
{
	prng->wLow = 0;
	prng->wHigh = 1;

	for (;;)
		{
		int tk;

		tk = TkFetch();
		switch (tk)
			{
		default:
			tkPush = tk;
			return;

		case tkNoNinch:
			prng->fAllowNinch = fFalse;
			break;

		case tkNil:
			break;

		case tkLow:
		case tkHigh:
		case tkMult:
		case tkDiv:
			HandleHighLowMultDiv(prng, tk);
			break;
			}
		}
}


void ParseListBox(prng)
RNG * prng;
{
	int tk;

	prng->wLow = 0;
	prng->wHigh = 32767;
	for (;;)
		{
		tk = TkFetch();
		switch (tk)
			{
		default:
			tkPush = tk;
			return;

		case tkNoNinch:
			prng->fAllowNinch = fFalse;
			break;

		case tkReturnString:
			prng->wHigh = 0;
			return;

		case tkComboAtomic:
			prng->wLow = prng->wHigh = -1;
			return;

		case tkNil:
			break;

		case tkLow:
		case tkHigh:
		case tkMult:
		case tkDiv:
			HandleHighLowMultDiv(prng, tk);
			break;

		case tkMultiSelectable:
			prng->wLow = prng->wHigh = 0;
			break;
			}
		}
}


TkFetch()
{
	int tk;
	CHAR * pch, ch;

	if (tkPush != tkNil)
		{
		tk = tkPush;
		tkPush = tkNil;
		return tk;
		}

	while ((ch = getc(pflCur)) <= ' ')
		;

	if (ch == 0xff)
		return tkEof;

	pch = szToken;
	do
		*pch++ = ch;
	while ((ch = getc(pflCur)) > ' ' && ch != 0xff);
	*pch = '\0';

	for (tk = 0; tk < tkLim; tk += 1)
		{
		if (strcmpi(szToken, mptksz[tk]) == 0)
			return tk;
		}

	return tkNil;
}


WriteOutput(szName)
SZ szName;
{
	PFL pfl;

	if ((pfl = fopen(szName, "w")) == NULL)
		{
		printf("MKDLG: cannot create output file!\n");
		exit(2);
		}

	fprintf(pfl, "/* This file was generated by MKDLG, do not edit! */\n\n");

	fprintf(pfl, "typedef struct _rng\n\t{\n\tint wLow;\n");
	fprintf(pfl, "\tint wHigh;\n\tint wMult;\n\tint wDiv;\n\t} RNG;\n\n");

		/* BLOCK: write mpibstrnghid */
		{
		int ibst;

		fprintf(pfl, "csconst int mpibstrnghid [] =\n\t{\n");

		for (ibst = 0; ibst < ibstMac; ibst += 1)
			{
			fprintf(pfl, "\t%s", mpibstrnghid[ibst]);
			if (ibst < ibstMac - 1)
				putc(',', pfl);
			putc('\n', pfl);
			}

		fprintf(pfl, "\t};\n\n");
		}

		/* BLOCK: write rgrng */
		{
		int irng;

		fprintf(pfl, "csconst RNG rgrng [] =\n\t{\n");
		for (irng = 0; irng < irngMac; irng += 1)
			{
			fprintf(pfl, "\t{ %d, %d, %d, %d }", 
					rgrng[irng].wLow, rgrng[irng].wHigh,
					rgrng[irng].wMult, rgrng[irng].wDiv);
			if (irng < irngMac - 1)
				putc(',', pfl);
			putc('\n', pfl);
			}
		fprintf(pfl, "\t};\n\n");
		}

		/* BLOCK: write rgbstRng */
		{
		int ibst;

		fprintf(pfl, "csconst int rgbstRng [] =\n\t{\n");
		for (ibst = 0; ibst < ibstMac; ibst += 1)
			{
			if ((ibst % 10) == 0)
				putc('\t', pfl);
			fprintf(pfl, "%d", rgbstRng[ibst]);
			if (ibst < ibstMac - 1)
				fprintf(pfl, ", ");
			if ((ibst % 10) == 9 || ibst == ibstMac - 1)
				putc('\n', pfl);
			}
		fprintf(pfl, "\t};\n\n");
		}

		/* BLOCK: write grpstRng */
		{
		int ibst, bst, cch;
		CHAR * pch;

		fprintf(pfl, "csconst CHAR grpstRng [] =\n\t{\n");
		for (ibst = bst = 0, pch = grpstRng; bst < bstMacRng; ibst++)
			{
			fprintf(pfl, "/* %s */\n\t", mpibstrnghid[ibst]);
			cch = *pch + 1;
			bst += cch;
			while (cch-- > 0)
				{
				fprintf(pfl, "%d", *pch++);
				if (bst < bstMacRng - 1 || cch > 0)
					fprintf(pfl, ", ");
				}
			fprintf(pfl, "\n");
			}
		fprintf(pfl, "\t};\n\n");
		}

	fclose(pfl);
}


SZ SzAlloc(sz)
SZ sz;
{
	SZ szRet;

	szRet = malloc(strlen(sz) + 1);
	strcpy(szRet, sz);

	return szRet;
}


