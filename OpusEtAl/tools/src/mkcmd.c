/*#define DEBUG*/
/* mkcmd.c -- Command Table Compiler for Opus */
/* Written October 24, 1986 by BradCh */

#include <stdio.h>
#include <ctype.h>
#include "vk.h"

extern int * malloc();
extern int * realloc();
extern FILE * fopen();
extern char * strchr();

#define cabiNil -1

typedef int BOOL;
#define fFalse (0)
#define fTrue (!fFalse)


/* Because DOS can't redirect stderr... */
#define STDERR stdout


#define chHelp 8 /* Totaly bogus Windows thingy for the help menu */

/* REVIEW: memcpy() does not work in C5.0! */
/*#define bltb(pbSrc, pbDest, cb)         memcpy((pbDest), (pbSrc), (cb))*/

/* Taken from CASHMERE\KEYS.N */
#define KcCtrl(kc)      ((kc) | 0x100)
#define KcShift(kc)     ((kc) | 0x200)
#define KcAlt(kc)       ((kc) | 0x400)

/* Key Map Entry */
typedef struct _kme
	{
	unsigned kc : 12;
	unsigned kt : 3;
/*        union {*/
	int bsy;
/*                int tmc;*/
/*                };*/
} KME;

#define cbKme 4

#define ktNil           0
#define ktPushButton    1
#define ktCheckBox      2
#define ktRadioButton   3
#define ktEditText      4
#define ktComboBox      5
#define ktMacro         6
#define ktInsert        3

/* Key MaP */
typedef struct _kmp
	{
/*        struct _kmp ** hkmpNext;*/
	char * pszName; /* NULL on file */
	BOOL fStopHere;
	int ikmeMac;
	int ikmeMax; /* equal to ikmeMac on file */
	KME rgkme[1 /*ikmeMax*/];
} KMP;

#define cbKmp 8

/* end of CASHMERE\KEYS.N */


typedef struct _sttb
	{
	int     bMax;
	int     ibstMac;
	int     ibstMax;
	union
		{
		struct
			{
			unsigned     cbExtra:14;
			unsigned     fExternal:1;
			unsigned     fStyleRules:1;
			} s;
		int wStuff;
		} u;
	int     *rgbst;
} STTB;

#define ibstNil -1
#define cbSTTB (sizeof (STTB) - sizeof (int *))


/* Menu IteM */
typedef struct _mtm
	{
	unsigned imnu : 4;
	unsigned ibst : 9;
	unsigned fChecked : 1;
	unsigned fDisabled : 1;
	unsigned fRemove : 1;
	int bsy;
} MTM;

#define cbMtm (sizeof (MTM))
#define bsySeparator 0xfffe
#define bsyFileCache 0xfffd
#define bsyWndCache 0xfffc


/* MenU Delta */
#define cmtmInit 400
typedef struct _mud
	{
	int imtmMac;
	int imtmMax;
	MTM rgmtm[cmtmInit];
} MUD;

#define cbMud (sizeof (MUD) - sizeof (MTM) * cmtmInit)


/* File name extensions */
#define szExtCmd        ".cmd"
#define szExtAsm        ".asm"
#define szExtH          ".h"


char * szDlgDir = "..\\dlg\\";
char * szBuildDir = "";


int cchMaxPcode = 256; /* max length of a pcode function name */

MUD vmud = { 
	0, cmtmInit };


FILE *pflCmd, *pflAsm, *pflMenu, *pflH, *pflHLP, *pflRGBCM, *pflMH, *pflH2;

	int rgicbm[8] = { 
	0,0,0,0,0,0,0,0 	};


int viicbm = 0;
int vicbm = 0;
char vszFileName [32]; /* current input file */
int vcLine = 0; /* current input file line number */
char rgchLine[512];
char * vpch, * vpchStart;
BOOL vfMenus;
BOOL vfUseAt;

int vcWarning = 0;

#define ibcmMax 512
int vibcm = 0;
int rgbcm [ibcmMax];
char * rgszOld [ibcmMax];
int ibcmMacOld = 0;

char * rgszEnglish [ibcmMax];


#define tkFirst         128

#define tkCommands      128
#define tkKeymap        129
#define tkMenubar       130
#define tkMenu          131
#define tkInt           132
#define tkString        133
#define tkFloat         134
#define tkEither        135
#define tkHpsd           136
#define tkSd		137

#define tkQuoted        138
#define tkNumber        139

#define tkFirstKey      140

char * rgpszKeywords [] =
	{
	"COMMANDS",
	"KEYMAP",
	"MENUBAR",
	"MENU",
	"INT",
	"HST",
	"NUM",
	"HPSDNUM",
	"HPSD",
	"SD"
	};


#define ipszMax 10


char * rgszUcc [] =
	{
	"UCCCOPY",
	"UCCMOVE",
	"UCCDELETESEL",
	"UCCFORMAT",
	"UCCPASTE",
	"UCCINSERT",
	"UCCCUT",
	"UCCTYPING"
	};


#define iszMaxUcc 7


#define vkNil (-1)

typedef struct _vkd
	{
	char * psz;
	int vk;
} VKD;

VKD rgvkd [] =
{
	{ "CANCEL",     VK_CANCEL },
	{ "BACK",       VK_BACK },
	{ "BS",         VK_BACK },
	{ "TAB",        VK_TAB },
	{ "CLEAR",      VK_CLEAR },
	{ "RETURN",     VK_RETURN },
	{ "ESC",        VK_ESCAPE },
	{ "SPACE",      VK_SPACE },
	{ "PRIOR",      VK_PRIOR },
	{ "PGUP",       VK_PRIOR },
	{ "PGDN",       VK_NEXT },
	{ "NEXT",       VK_NEXT },
	{ "END",        VK_END },
	{ "HOME",       VK_HOME },
	{ "LEFT",       VK_LEFT },
	{ "UP",         VK_UP },
	{ "RIGHT",      VK_RIGHT },
	{ "DOWN",       VK_DOWN },
	{ "SELECT",     VK_SELECT },
	{ "PRINT",      VK_PRINT },
	{ "EXECUTE",    VK_EXECUTE },
	{ "INSERT",     VK_INSERT },
	{ "INS",        VK_INSERT },
	{ "DELETE",     VK_DELETE },
	{ "DEL",        VK_DELETE },
	{ "HELP",       VK_HELP },
	{ "NP0",        VK_NUMPAD0 },
	{ "NP1",        VK_NUMPAD1 },
	{ "NP2",        VK_NUMPAD2 },
	{ "NP3",        VK_NUMPAD3 },
	{ "NP4",        VK_NUMPAD4 },
	{ "NP5",        VK_NUMPAD5 },
	{ "NP6",        VK_NUMPAD6 },
	{ "NP7",        VK_NUMPAD7 },
	{ "NP8",        VK_NUMPAD8 },
	{ "NP9",        VK_NUMPAD9 },
	{ "NP*",        VK_MULTIPLY },
	{ "NP+",        VK_ADD },
	{ "NP-",        VK_SUBTRACT },
	{ "NP.",        VK_DECIMAL },
	{ "NP/",        VK_DIVIDE },
	{ "F1",         VK_F1 },
	{ "F2",         VK_F2 },
	{ "F3",         VK_F3 },
	{ "F4",         VK_F4 },
	{ "F5",         VK_F5 },
	{ "F6",         VK_F6 },
	{ "F7",         VK_F7 },
	{ "F8",         VK_F8 },
	{ "F9",         VK_F9 },
	{ "F10",        VK_F10 },
	{ "F11",        VK_F11 },
	{ "F12",        VK_F12 },
	{ "F13",        VK_F13 },
	{ "F14",        VK_F14 },
	{ "F15",        VK_F15 },
	{ "F16",        VK_F16 }
	};


#define ivkdMax (sizeof (rgvkd) / sizeof (VKD))

#define chShift         'S'
#define chCtrl          'C'
#define chAlt           'A'


/* From CASHMERE\CMDTBL.H */

typedef struct _ard
	{
	unsigned elp;
	union 
		{
		int w;
		} u;
} ARD;

#define cbARD (sizeof (ARD))

#define cbELP 1


#define wHashMax 127

typedef struct _sy
	{
	unsigned bsyNext;
	char * pszFunc;
	union 
		{
		struct 
			{
			unsigned ucc : 4; /* NOTE: no longer used! */
			unsigned fSetUndo : 1;
			unsigned fRepeatable : 1;
			unsigned fWParam : 1;
			unsigned iidstr : 9;
			} scmd;

		struct 
			{
			unsigned ucc : 4; /* NOTE: no longer used! */
			unsigned fSetUndo : 1;
			unsigned fRepeatable : 1;
			unsigned foo : 1;
			unsigned iidstr : 9;
			} ssdm;

		struct 
			{
			unsigned cagdMin : 4;
			unsigned cagdMax : 4;
			unsigned elr : 3;
			unsigned fStmt : 1;
			unsigned fDyadic : 1;
			unsigned spare : 3;
			} sel;
		} u;
	int mct;

	char st[1];
	/* char rgagd[0];*/
} SY;

#define cbSY 9


#define grfLockedMode           0x8000
#define grfAnnotateMode         0x4000
#define grfOutlineMode          0x2000
#define grfHeaderMode           0x1000
#define grfFootnoteMode         0x0800
#define grfPageMode             0x0400
#define grfMacroMode            0x0200
#define grfBlockMode            0x0100
#define grfExtendMode           0x0080
#define grfNeedsDoc             0x0040
#define grfMomLock              0x0020

#define grfWritten              0x0010


typedef struct _syt
	{
	unsigned bsy;
	unsigned bsyMax;
	unsigned mpwbsy[wHashMax];
	char grpsy[1];
} SYT;

#define cbSYT (4 + wHashMax * 2)


/* Macro Types */

#define mctNil          0
#define mctCmd          1
#define mctEl           2
#define mctSdm		3
#define mctMacro        6
#define mctGlobalMacro  7


/* Argument Types */

#define elpNum		0	/* floating point NUM */
#define elpInt		1	/* integer */
#define elpHst		2	/* short string on application heap */
#define elpHpsd		3	/* string on EL string heap */
#define elpAdNum	4	/* NOT IMPLEMENTED: NUM array descriptor */
#define elpAdInt	5	/* NOT IMPLEMENTED: INT array descriptor */
#define elpAdSd		6	/* NOT IMPLEMENTED: STR array descriptor */
#define elpHpsdNum	7	/* NUM or string (both args passed) */


/* End of CASHMERE\CMDTBL.H */


typedef struct _hsd
	{
	struct _hsd * phsdNext;
	char * sz;
} HSD;

int viszHelp = 0;
HSD * vphsdBase = NULL;
HSD * vphsdEnd = NULL;

BOOL vfDebug;
BOOL vfProfile;
BOOL vfMktgPrvw;


/* ELR (EL entry Return type) -- defines the return value.
*/
typedef int ELR;
#define elrVoid		0	/* no return value */
#define elrNum		1	/* floating point NUM */
#define elrInt		2	/* 16-bit integer */
#define elrSd		3	/* string descriptor */
#define elrAd		4	/* NOT IMPLEMENTED: array */
#define elrDialog	5	/* dialog action statement */

ELR velr = 0;
BOOL vfStmt = fFalse;

SYT * vpsyt = 0;
KMP * vpkmp = 0;


STTB ** vhsttbMenu;
STTB * vpsttb;
STTB vsttb;


bltb(pbSrc, pbDest, cb)
register char * pbSrc;
register char * pbDest;
register int cb;
{
	if (pbSrc == pbDest)
		return;

	if (pbSrc < pbDest && pbSrc + cb > pbDest)
		{
		pbSrc += cb - 1;
		pbDest += cb - 1;
		while (cb-- > 0)
			*pbDest-- = *pbSrc--;
		}
	else
		{
		while (cb-- > 0)
			*pbDest++ = *pbSrc++;
		}
}


STTB ** HsttbInit(cwEstimate)
int cwEstimate;
{
	if (vpsttb != 0)
		{
		Error("too many string tables in use");
		return 0;
		}

	vpsttb = &vsttb;
	if ((vsttb.rgbst = malloc(cwEstimate*2)) == 0)
		{
		Error("out of memory");
		return 0;
		}

	vsttb.bMax = 0;
	vsttb.ibstMac = 0;
	vsttb.ibstMax = 0;
	vsttb.u.s.cbExtra = 0;
	vsttb.u.s.fExternal = fTrue;
	vsttb.u.s.fStyleRules = fFalse;

	return &vpsttb;
}


FreeHsttb(hsttb)
STTB ** hsttb;
{
	if (hsttb != &vpsttb)
		{
		Error("freeing bogus string table");
		return;
		}

	free(vsttb.rgbst);
	vsttb.rgbst = 0;
	vpsttb = 0;
}


IbstAddStToSttb(hsttb, st)
STTB ** hsttb;
char * st;
{
	int ibst;

	InsertStInSttb(hsttb, ibst = (**hsttb).ibstMac, st);

	return ibst;
}


InsertStInSttb(hsttb, ibstNew, st)
STTB ** hsttb;
int ibstNew;
char * st;
{
	int cbStNew;
	int cbNew;
	STTB * psttb;
	int ibst;
	int * pbstNew;
	int cbBlock;
	char * stNew;

	cbStNew = *st + 1; /* for length byte */
	cbNew = sizeof (int) + cbStNew;

	if ((vpsttb->rgbst = realloc(vpsttb->rgbst, vpsttb->bMax + cbNew)) == 0)
		Error("out of memory");
	psttb = *hsttb;

	/* make a hole for the new offset */
	pbstNew = (int *) &psttb->rgbst[ibstNew];
	cbBlock = ((char *) psttb->rgbst) + psttb->bMax - (char *)pbstNew;
	if (cbBlock != 0)
		bltb(pbstNew, pbstNew + 1, cbBlock);

	/* update offsets */
	for (ibst = 0; ibst < psttb->ibstMac + 1; ibst++)
		{
		psttb->rgbst[ibst] += sizeof (int);
		}

	psttb->bMax += sizeof (int);

	/* copy the string to the end of the string table */
	psttb->rgbst[ibstNew] = psttb->bMax;
	stNew = ((char *)psttb->rgbst) + psttb->bMax;
	bltb(st, stNew, cbStNew);
	psttb->bMax += cbStNew;
	psttb->ibstMac += 1;
	psttb->ibstMax += 1;
}


SzToStInPlace(pch)
char * pch;
{
	int cch = strlen(pch);
	bltb(pch, pch + 1, cch);
	*pch = cch;
}


SzToSt(sz, st)
char * sz, * st;
{
	int cch = strlen(sz);
	bltb(sz, st + 1, cch);
	*st = cch;
}


StToSz(st, sz)
char * st, * sz;
{
	bltb(st + 1, sz, st[0]);
	sz[st[0]] = '\0';
}


char * strsave(psz)
char * psz;
{
	char * pszAlloc;

	if ((pszAlloc = (char *) malloc(strlen(psz) + 1)) == 0)
		Error("out of memory");
	strcpy(pszAlloc, psz);

	return pszAlloc;
}



/* M A I N */
main(argc, argv)
int argc;
char * argv [];
{
	char * pchBaseName;
	char rgchFileName[32];
	int w, * pw;
	int argi;

	if (argc < 2)
		{
		GiveUsage();
		exit(1);
		}

	for (argi = 1; argi < argc; ++argi)
		{
		if (argv[argi][0] != '-')
			break;
		switch (argv[argi][1])
			{
		case 't':
		case 'T':
			vfUseAt = argv[argi][2] == '@';
			cchMaxPcode = vfUseAt ? atoi(argv[argi] + 3) : atoi(argv[argi] + 2);
			break;

		case 'd':
		case 'D':
			/* REVIEW: kludge! */
			if (strncmp(argv[argi] + 2, "DEBUG", 5) == 0)
				vfDebug = fTrue;
			else  if (strncmp(argv[argi] + 2, "PROFILE", 7) == 0)
				vfProfile = fTrue;
			else  if (strncmp(argv[argi] + 2, "MKTGPRVW", 8) == 0)
				vfMktgPrvw = fTrue;
			break;

		case 's':
		case 'S':
			szDlgDir = &argv[argi][2];
			break;

		case 'b':
		case 'B':
			szBuildDir = &argv[argi][2];
			break;

		default:
			GiveUsage();
			exit(1);
			}
		}

	pchBaseName = argv[argi];

#ifdef REVIEW /* causes problems when using ..\file */
	if (strchr(pchBaseName, '.') != 0)
		{
		fprintf(STDERR, "MKCMD: do not specify file extension!\n");
		exit(1);
		}
#endif

	/* Initialize the symbol table */
#define bsyMaxInit 10300
	if ((vpsyt = (SYT *) malloc(sizeof(SYT) + bsyMaxInit)) == 0)
		Error("out of memory");
	vpsyt->bsy = 0;
	vpsyt->bsyMax = bsyMaxInit;
	for (pw = &vpsyt->mpwbsy[0], w = 0; w < wHashMax; ++w)
		*pw++ = 0xffff;

	/* Initialize a keymap.  REVIEW: we will need more than one! */
#define ikmeMaxDef 200
	if ((vpkmp = (KMP *) malloc(cbKmp + ikmeMaxDef * sizeof(KME))) == 0)
		Error("out of memory");
	vpkmp->pszName = 0;
	vpkmp->fStopHere = fFalse;
	vpkmp->ikmeMac = 0;
	vpkmp->ikmeMax = ikmeMaxDef;

	/* Initialize menu structures */
	vhsttbMenu = HsttbInit(10/* REVIEW */);

	vfMenus = fFalse;

	/* Create menu file */
	if ((pflMenu = fopen("OPUSMENU.H", "w")) == 0)
		{
		fprintf(STDERR, "MKCMD: can't create OPUSMENU.H\n");
		exit(1);
		}
	fprintf(pflMenu, "/* This file was created by MKCMD!  Do Not Edit! */\n\n");
	fprintf(pflMenu, "csconst struct CBM rgcbm[] = {\n");

	/* Create menu header file */
	strcpy(rgchFileName, pchBaseName);
	strcat(rgchFileName, szExtH);
	if ((pflH = fopen(rgchFileName, "w")) == 0)
		{
		fprintf(STDERR, "MKCMD: can't create %s\n", rgchFileName);
		exit(1);
		}

	fprintf(pflH,
			"/* This file was created by MKCMD!  Do Not Edit! */\n\n");
	fprintf(pflH,
			"#ifndef DEBUG\n#define USEBCM\n#endif\n");
	fprintf(pflH,
			"#ifdef USEBCM\n");
	fprintf(pflH,
			"#define BcmOfBcmSz(bcm, sz) (bcm)\n");
	fprintf(pflH,
			"#else\n");
	fprintf(pflH,
			"#define BcmOfBcmSz(bcm, sz) BcmOfSz(sz)\n");
	fprintf(pflH,
			"#endif\n\n");

	InitRgbcm();

	/* Read in old IBCM.H, if there is one... */
	if ((pflHLP = fopen("IBCM.H", "r")) != 0)
		{
		register int isz;
		char * pch, * pch2;
		char sz [256];

		isz = 0;
		while (fgets(sz, sizeof (sz), pflHLP) != NULL)
			{
			if (sz[0] != '#')
				continue;

			for (pch = sz; *pch != '\0' && *pch != ' '; ++pch)
				;
			if (*pch == '\0')
				continue;

			pch += 1; /* skip space */
			for (pch2 = pch; *pch2 != ' '; ++pch2)
				{
				if (*pch2 == '\0')
					Error("illegal line in ibcm.h");
				}
			*pch2 = '\0';
			pch += 4; /* skip "ibcm" */
			isz = atoi(pch2 + 1);
			if (*(pch2 - 1) == '_')
				*(pch2 - 1) = '$';
			rgszOld[isz] = strsave(pch);
			if (isz + 1 > ibcmMacOld)
				vibcm = ibcmMacOld = isz + 1;
			}


		fclose(pflHLP);
		}

	if ((pflHLP = fopen("IBCM.H", "w")) == 0)
		{
		fprintf(STDERR, "MKCMD: can't create IBCM.H\n");
		exit(1);
		}

	if ((pflRGBCM = fopen("RGBCM.H", "w")) == 0)
		{
		fprintf(STDERR, "MKCMD: can't create RGBCM.H\n");
		exit(1);
		}

	fprintf(pflRGBCM,
			"/* This file was created by MKCMD!  Do Not Edit! */\n\n");

	if ((pflMH = fopen("MENUHELP.TXT", "w")) == 0)
		{
		fprintf(STDERR, "MKCMD: can't create MENUHELP.TXT\n");
		exit(1);
		}

#ifdef OLDSTR
	fprintf(pflMH,
			"/* This file was created by MKCMD!  Do Not Edit! */\n\n");
#endif /* OLDSTR */
	fprintf(pflMH,
			"; This file was created by MKCMD!  Do Not Edit!\n;\n");

	if ((pflH2 = fopen("OPUSCMD2.H", "w")) == 0)
		{
		fprintf(STDERR, "MKCMD: can't create OPUSCMD2.H\n");
		exit(1);
		}
	fprintf(pflH2,
			"/* This file was created by MKCMD!  Do Not Edit! */\n\n");

	/* Read entire source file and generate internal structures */
	/* Also writes resource file */
	for ( ; argi < argc; ++argi)
		{
		/* Open source file */
		strcpy(vszFileName, argv[argi]);
		strcat(vszFileName, szExtCmd);
		if ((pflCmd = fopen(vszFileName, "r")) == 0)
			{
			fprintf(STDERR, "MKCMD: can't open %s\n",
					vszFileName);
			exit(1);
			}
		vcLine = 0;

		ReadLine();
		while (!feof(pflCmd))
			{
			Statement();
			}

		fclose(pflCmd);
		}

	if (vcWarning != 0)
		{
		fprintf(STDERR, "Terminating make due to warnings!\n");
		exit(1);
		}

		/* finish off opusmenu.h */
		{
		register int i;
		fprintf(pflMenu, "};\n\ncsconst int rgicbm[%d] = { ", viicbm);
		for (i = 0; i < viicbm-1; i++)
			fprintf(pflMenu, "%d, ", rgicbm[i]);
		fprintf(pflMenu, "%d };\n",rgicbm[i]);
		}

	fclose(pflMenu);

	fprintf(pflH, "\n#define bcmMacStd %d\n", vpsyt->bsy);
	fclose(pflH);

	fclose(pflHLP);

		{
		HSD * phsd;

		for (phsd = vphsdBase; phsd != 0; phsd = phsd->phsdNext)
			{
#ifdef OLDSTR
			fprintf(pflMH, "\"\\%03o%s\"%c\n",
					strlen(phsd->sz), phsd->sz,
					(phsd->phsdNext == NULL) ? ' ' : ',');
#endif /* OLDSTR */
		/* The x,\t stuff is simply because this is what our
		*  existing string processing code expects. X is normally
		*  the string id name but we ignore it.
		*/
			fprintf(pflMH, "x,\t\"%s\"\n", phsd->sz);
			}

		fclose(pflMH);
		}

		{
		int ibcm;

		vibcm -= 1;
		for (ibcm = 0; ibcm < vibcm; ++ibcm)
			fprintf(pflRGBCM, "%d,\n", rgbcm[ibcm]);
		if (vibcm >= 0)
			fprintf(pflRGBCM, "%d\n", rgbcm[ibcm]);
		fclose(pflRGBCM);
		}

	/* Write assembler file */
	strcpy(rgchFileName, pchBaseName);
	strcat(rgchFileName, szExtAsm);
	if ((pflAsm = fopen(rgchFileName, "w")) == 0)
		{
		fprintf(STDERR, "MKCMD: cannot create %s\n", rgchFileName);
		exit(1);
		}
	WriteAsmFile();
	fclose(pflAsm);

	exit(0);
}


GiveUsage()
{
	fprintf(STDERR, "MKCMD Version 1.8 (April 22, 1988)\n");
	fprintf(STDERR, "Usage: MKCMD [-D<variable>] [-S<dlg dir>] file ...\n");
	/* REVIEW: more */
}


Error(pszMessage)
char * pszMessage;
{
	int ich, cchSpace;

	fprintf(STDERR, "%s\n", rgchLine);
	cchSpace = vpch - rgchLine;
	for (ich = 0; ich < cchSpace; ++ich)
		fputc(rgchLine[ich] == '\t' ? '\t' : ' ', STDERR);
	fprintf(STDERR, "^\n%s:%d: %s error\n", vszFileName, vcLine, 
			pszMessage);

	exit(1); /* REVIEW: unfriendly */
}


Warning(szMessage)
char * szMessage;
{
	int ich, cchSpace;

	fprintf(STDERR, "%s\n", rgchLine);
	cchSpace = vpch - rgchLine;
	for (ich = 0; ich < cchSpace; ++ich)
		fputc(rgchLine[ich] == '\t' ? '\t' : ' ', STDERR);
	fprintf(STDERR, "^\n%s:%d: %s warning\n", vszFileName, vcLine, 
			szMessage);

	vcWarning += 1;
}


WriteAsmFile()
{
	int w, bsy;
	char * pch;
	int * pw;
	int cwSyt;
	int cwKmp;
	int cwSttb;
	int cwMud;

	cwSyt = (cbSYT + vpsyt->bsy + 1) / 2;
	cwKmp = (cbKmp + cbKme * vpkmp->ikmeMac + 1) / 2;
	cwSttb = (vpsttb->bMax + 1) / 2;
	cwMud = 2 + vmud.imtmMac * 2;

	/* write constants to opuscmd2.h */
	fprintf(pflH2, "#define cbsytInit       %d\n", cwSyt*2);
	fprintf(pflH2, "#define ckmeInit        %d\n", vpkmp->ikmeMac);
	fprintf(pflH2, "#define cwsttbMenuBase  %d\n", cbSTTB/sizeof(int));
	fprintf(pflH2, "#define cbsttbMenuRgbst (%d*sizeof(int))\n", cwSttb);
	fprintf(pflH2, "#define cwmudBase       %d\n", cwMud);


	fprintf(pflAsm,
			"\n; This file was generated by MKCMD!  Do not edit!\n\n");

	/* Write code segment */
	fprintf(pflAsm, "\ninitwin_PCODE segment word public 'code'\n\n");

		/* BLOCK: write out hash table statistics */
		{
		int mplvlc[16];
		int w;
		int bsy;
		int lvl;
		int cTot = 0;

		for (lvl = 0; lvl < 16; lvl++)
			mplvlc[lvl] = 0;

		for (w = 0; w < wHashMax; w++)
			{
			lvl = 0;
			for (bsy = vpsyt->mpwbsy[w]; bsy != -1; 
					bsy = ((SY *)((char *)vpsyt->grpsy + bsy))->bsyNext)
				lvl++;
			cTot += lvl;
			if (lvl < 16)
				mplvlc[lvl]++;
			else
				mplvlc[15]++;
			}

		fprintf(pflAsm, "; Hash depths:");
		for (lvl = 0; lvl < 15; lvl++)
			fprintf(pflAsm, " %d", mplvlc[lvl]);
		fprintf(pflAsm, " %d+\n", mplvlc[15]);
		fprintf(pflAsm, ";   wHashMax = %d\n", wHashMax);
		fprintf(pflAsm, ";   total commands = %d\n", cTot);
		fprintf(pflAsm, ";   average depth = %d\n", (cTot+wHashMax/2)/wHashMax);
		fprintf(pflAsm, "\n");
		}

	/* Write out hash table */
	fprintf(pflAsm, "\tpublic\tvsytInit\n");
	fprintf(pflAsm, "vsytInit\tdw\t%d\t; cwTable\n", cwSyt);
	fprintf(pflAsm, "\tdw\t%d\t; bsy\n", vpsyt->bsy);
	fprintf(pflAsm, "\tdw\t%d\t; bsyMax\n", vpsyt->bsy);
	for (w = 0; w < wHashMax; )
		{
		int i;
		fprintf(pflAsm, "\tdw\t%d", vpsyt->mpwbsy[w++]);
		for (i = 1; i < 8 && w < wHashMax; ++i)
			fprintf(pflAsm, ", %d", vpsyt->mpwbsy[w++]);
		fputc('\n', pflAsm);
		}
	/* Write out symbols */
	fprintf(pflAsm, "\n");
	bsy = 0;

	while (bsy < vpsyt->bsy)
		{
		int cagd, cch, mct;
		char * psz;
		SY * psy;
		char sz [258];

		psy = (SY *) ((char *) vpsyt->grpsy + bsy);
		mct = psy->mct & 7;

		fprintf(pflAsm, "; bsy = %d (0x%x)\n", bsy, bsy);
		fprintf(pflAsm, "\tdw\t%d\t; bNext\n", psy->bsyNext);

/* function pointer */
		strncpy(sz, psy->pszFunc, cchMaxPcode);
		sz[cchMaxPcode] = 0;
		if ( vfUseAt )
			fprintf(pflAsm, "extrn\t@%s:abs\n\tdw\t@%s\n", sz, sz);
		else
			fprintf(pflAsm, "extrn\tQ_%s:abs\n\tdw\tQ_%s\n", sz, sz);

/* argument count/menu help id */
		if (mct == mctCmd)
			{
			int ucc;

			ucc = psy->u.scmd.ucc;
			fprintf(pflAsm,
					"\tdw\t%d\t; ucc: %d, iidstr: %d\n",
					(psy->u.scmd.iidstr << 7) + ucc + 
					(psy->u.scmd.fSetUndo << 4) +
					(psy->u.scmd.fRepeatable << 5),
					ucc, psy->u.scmd.iidstr);
			}
		else  if (mct == mctSdm)
			{
			int ucc;

			ucc = psy->u.ssdm.ucc;
			fprintf(pflAsm,
					"\tdw\t%d\t; ucc: %d, iidstr: %d\n",
					(psy->u.scmd.iidstr << 7) + ucc +
					(psy->u.ssdm.fSetUndo << 4) +
					(psy->u.scmd.fRepeatable << 5),
					ucc, psy->u.ssdm.iidstr);
			}
		else  /* mct == mctEl */						
			{
			fprintf(pflAsm,
					"\tdw\t%d\t; cagdMin: %d cagdMax: %d\n",
					(psy->u.sel.cagdMax << 4) + psy->u.sel.cagdMin +
					((psy->u.sel.fStmt << 3) | (psy->u.sel.fDyadic << 4) | 
						psy->u.sel.elr) * 256,
					psy->u.sel.cagdMin, psy->u.sel.cagdMax);
			}

/* type */
		fprintf(pflAsm, "\tdw\t%d\t; mct\n", psy->mct & ~grfWritten);

/* name */
		fprintf(pflAsm, "\tdb\t%d", cch = psy->st[0]);

		bsy += (cch == 0) ? 2 : cch;

		pch = &psy->st[1];
		if (cch > 0)
			{
			fprintf(pflAsm, ", '");
			while (cch-- > 0)
				fputc(*pch++, pflAsm);
			fprintf(pflAsm, "'");
			}
		else
			{
			fprintf(pflAsm, "\n\tdw\t%d", *((int *) pch)++);
			}
		fprintf(pflAsm, "\n");

/* argument descriptors */
		if (mct == mctSdm)
			{
			bsy += 4;
			fprintf(pflAsm, "\tdw\t%d\t; cabi\n", 
					*((int *) pch)++);
			fprintf(pflAsm, "\tdw\t%d\t; ieldi\n",
					*((int *) pch)++);
			}
		else  if (mct != mctCmd) /* mct == mctEl */
			{
			cagd = psy->u.sel.cagdMax;
			if (cagd > 0)
				{
				bsy += cagd;
				fprintf(pflAsm, "\tdb\t%d", *pch++);
				while (--cagd > 0)
					fprintf(pflAsm, ", %d", *pch++);
				fprintf(pflAsm, "\t; rgagd\n");
				}
			}

/* end line */
		fputc('\n', pflAsm);
		bsy += cbSY;
		}

	/* Write out keymaps */
	if (vpkmp->pszName != 0)
		{
		int ikme;
		KME * pkme;

		fprintf(pflAsm, "\n; Keymaps\n");
		fprintf(pflAsm, "\tpublic\t%s\n", vpkmp->pszName);
		fprintf(pflAsm, "%s\tdw\t%d\t; cw\n\tdw\t0\t; hkmpNext\n",
				vpkmp->pszName,
				(cbKmp + cbKme * vpkmp->ikmeMac + 1) / 2);
		fprintf(pflAsm, "\tdw\t0\t; fStopHere\n");
		fprintf(pflAsm, "\tdw\t%d\t; ikmeMac\n", vpkmp->ikmeMac);
		fprintf(pflAsm, "\tdw\t%d\t; ikmeMax\n", vpkmp->ikmeMac);
		for (ikme = 0, pkme = vpkmp->rgkme; ikme < vpkmp->ikmeMac;
				++ikme, ++pkme)
			{
			fprintf(pflAsm, "\tdw\t%d,%d\n", *((int *)pkme),
					pkme->bsy);
			}
		fprintf(pflAsm, "\n");
		}

		/* Write out menus */
		{
		int i;
		char * pch;

		fprintf(pflAsm, "\n; Menus\n");

		/* base */
		fprintf(pflAsm, "\n; Sttb Base\n");
		fprintf(pflAsm, "\tpublic vsttbMenuBase\n");
		fprintf(pflAsm, "vsttbMenuBase\tdw\t%d\t; cw\n", cbSTTB/sizeof(int));
		fprintf(pflAsm, "\tdw\t%d\t; bMax\n", vpsttb->bMax);
		fprintf(pflAsm, "\tdw\t%d\t; ibstMac\n", vpsttb->ibstMac);
		fprintf(pflAsm, "\tdw\t%d\t; ibstMax\n", vpsttb->ibstMax);
		fprintf(pflAsm, "\tdw\t%d\t; cbExtra,fExternal,fStyleRules\n",vpsttb->u.wStuff);

		/* rgbst */
		fprintf(pflAsm, "\n; Sttb Rgbst\n");
		fprintf(pflAsm, "\tpublic vsttbMenuRgbst\n");
		fprintf(pflAsm, "vsttbMenuRgbst\tdw\t%d\t; cw\n", cwSttb);

		/* offsets... */
		for (i = 0; i < vpsttb->ibstMac; ++i)
			fprintf(pflAsm, "\tdw\t%d\n", vpsttb->rgbst[i]);

		/* strings... */
		pch = (char *)vpsttb->rgbst + sizeof (int) * vpsttb->ibstMac;
		for (i = 0; i < vpsttb->ibstMac; ++i)
			{
			int cch;
			fprintf(pflAsm, "\tdb\t%d,'", cch = *pch++);

			while (cch-- > 0)
				{
				if (*pch == '\'')
					fprintf(pflAsm, "', %d, '", *pch++);
				else
					putc(*pch++, pflAsm);
				}
			fprintf(pflAsm, "'\n");
			}

		/* make sure we use an even number of bytes */
		if (vpsttb->bMax & 1)
			fprintf(pflAsm, "\tdb\t0\t; filler\n");

		fprintf(pflAsm, "\n\tpublic vmudInit\n");
		fprintf(pflAsm, "vmudInit\tdw\t%d\t; cw\n", cwMud);
		fprintf(pflAsm, "\tdw\t%d\t; imtmMac\n", vmud.imtmMac);
		fprintf(pflAsm, "\tdw\t%d\t; imtmMax\n", vmud.imtmMac);
		pw = (int *) &vmud.rgmtm[0];
		for (i = 0; i < vmud.imtmMac; ++i)
			{
			int w1, w2;
			w1 = *pw++;
			w2 = *pw++;
			fprintf(pflAsm, "\tdw\t%d,%d\n", w1, w2);
			}
		}

	fprintf(pflAsm, "\ninitwin_PCODE\tends\n\tend\n");
}


Statement()
{
	switch (TkParse())
		{
	case tkCommands:
		DealWithCommands();
		break;

	case tkKeymap:
		DealWithKeymap();
		break;

	case tkMenubar:
		DealWithMenuBar();
		break;

	default:
		Error("syntax");
		}
}


typedef unsigned char CHAR;

#define chFirstUpperTbl  (224)
#define ch(_i)  ((CHAR) (_i))

/* French upper case mapping table - only chars >= E0 (224) are mapped */
CHAR	mpchupFrench[] = {
	/* E0	    E1	     E2       E3       E4	E5	 E6	  E7   */
	ch(65),  ch(65),  ch(65),  ch(195), ch(196), ch(197), ch(198), ch(199),
	/* E8	    E9	     EA       EB       EC	ED	 EE	  EF   */
	ch(69),  ch(69),  ch(69),  ch(69),  ch(73),  ch(73),  ch(73),  ch(73),
	/* F0	    F1	     F2       F3       F4	F5	 F6	  F7   */
	ch(208), ch(209), ch(79),  ch(79),  ch(79),  ch(213), ch(214), ch(247),
	/* F8	    F9	     FA       FB       FC	FD	 FE	  FF   */
	ch(216), ch(85),  ch(85),  ch(85),  ch(220), ch(221), ch(222), ch(255)
};


#undef ch

/* C H U P P E R  */
/*  upper case conversion using international rules (received from jurgenl 10-10-88 bz) */
/*  %%Function: ChUpper  %%Owner: bradv  */

ChUpper(ch)
int ch;
{
	if ((unsigned)(ch - 'a') <= ('z' - 'a'))
		return (ch - ('a' - 'A'));
	else if (ch >= chFirstUpperTbl)  /* intl special chars */
		return (mpchupFrench[ch - chFirstUpperTbl]);
	else
		return (ch);
}


/* W  H A S H */
/* Return hash code for given string.  NOTE: case insensitive */
unsigned WHash(psz)
CHAR * psz;
{
	unsigned wHash, cch;

	wHash = 0;
	while (*psz)
		wHash = ((wHash << 5) + (wHash >> 11) + ChUpper(*psz++));
	wHash = ((wHash & 0x7fff) % wHashMax);

	return wHash;
}


BsyFindSymbol(psz)
char * psz;
{
	unsigned bsy;
	SY * psy;
	char * pgrpsy;

	pgrpsy = &vpsyt->grpsy[0];
	for (bsy = vpsyt->mpwbsy[WHash(psz)]; bsy != 0xffff;
			bsy = psy->bsyNext)
		{
		psy = (SY *) (pgrpsy + bsy);
		if (FSzEqualSt(psz, psy->st))
			break;
		}

	return bsy;
}


BsyFindEnglish(szEnglish)
char * szEnglish;
{
	unsigned bsy;
	int isz;
	SY * psy;

	for (isz = 0; isz < ibcmMax; isz += 1)
		{
		if (strcmpi(szEnglish, rgszEnglish[isz]) == 0)
			{
			bsy = rgbcm[isz];
			psy = (SY *) (vpsyt->grpsy + bsy);
			if (psy->st[0] == 0)
				bsy = *((int *) &psy->st[1]);
			return bsy;
			}
		}

	return 0xffff;
}



FSzEqualSt(psz, pst)
char * psz, * pst;
{
	int cch = *pst++;

	while (cch-- > 0)
		{
		if (*psz++ != *pst++)
			return fFalse;
		}
	return (*psz == 0);
}



/* B S Y  A D D  S Y M B O L */
/* Add a symbol to the table.  The contents of w1 and w2 depends on mct:

	mct		w1		w2
	--------------------------------------------
	mctCmd:		cargs		0	(iidstr filled in later)
	mctCmd:		-1		wParam	(iidstr filled in later)
	mctSdm:		cabi		ieldi	(iidstr filled in later)
	mctEl: 		cargsReq	cargs
*/
BsyAddSymbol(pszMacro, pszFunc, prgchArgs, w1, w2, mct)
char * pszMacro, * pszFunc, * prgchArgs;
int w1, w2, mct;
{
	SY * psy;
	int bsy;
	int cchName;
	int cagd;
	char * pch;
	int cbSy;
	int bsyChain;
	BOOL fChain;

	bsyChain =  BsyFindSymbol(pszMacro);
	fChain = (bsyChain != 0xffff);
	cchName = fChain ? 0 : strlen(pszMacro);

	bsy = vpsyt->bsy;

	cbSy = cbSY + cchName;
	if (mct == mctCmd)
		cbSy += ((w1 < 0) ? 0 : (w1 * cbELP));
	else  if (mct == mctSdm)
		cbSy += sizeof (int) * 2;
	else  /* mct == mctEl */
		cbSy += w2;

	if (fChain)
		cbSy += sizeof (int); /* for back pointer */

	if (bsy + cbSy > vpsyt->bsyMax)
		{
		Error("symbol table full");
		return -1;
		}

	vpsyt->bsy += cbSy;

	psy = (SY *) (&vpsyt->grpsy[0] + bsy);
	psy->pszFunc = strsave(pszFunc);
	psy->mct = mct;
	pch = psy->st;

	*pch++ = cchName;

	if (fChain)
		{
		*((int *) pch)++ = bsyChain; /* back pointer */
		}
	else
		{
		char * pch2;

		pch2 = pszMacro;
		while (cchName-- > 0)
			*pch++ = *pch2++;
		}

	switch (mct)
		{
	default:
		Error("bad mct");
		return;

	case mctCmd:
			{
			int celp = w1;
			psy->u.scmd.iidstr = w2; /* REVIEW: really filled in later! */
			psy->u.scmd.fWParam = celp < 0; /* REVIEW: used? */

			while (celp-- > 0)
				*pch++ = *prgchArgs++;
			}
		break;

	case mctSdm:
		psy->u.ssdm.iidstr = 0;
		*((int *) pch)++ = w1; /* cabi */
		*((int *) pch) = w2; /* ieldi */
		break;

	case mctEl:
			{
			int cagd = w2;
			psy->u.sel.cagdMin = w1;
			psy->u.sel.cagdMax = w2;
			psy->u.sel.elr = velr;
			psy->u.sel.fStmt = vfStmt;
			while (cagd-- > 0)
				*pch++ = *prgchArgs++;
			}
		break;
		}

	if (fChain)
		{
		SY * psyChain;

		psyChain = ((SY *) (&vpsyt->grpsy[0] + bsyChain));
		psy->bsyNext = psyChain->bsyNext;
		psyChain->bsyNext = bsy;
		}
	else
		{
		unsigned * pbsy;

		pbsy = &vpsyt->mpwbsy[WHash(pszMacro)];
		psy->bsyNext = *pbsy;
		*pbsy = bsy;
		}

	return bsy;
}


FetchMacroName(pch, cchMax)
char * pch;
int cchMax;
{
	SkipSpaces();
	while (cchMax-- > 0 && *vpch != '\0' && *vpch != ',')
		*pch++ = *vpch++;
	while (isspace(*(unsigned char *)--pch))
		;
	*++pch = '\0';
}


FetchFileName(pch, cchMax)
char * pch;
int cchMax;
{
	SkipSpaces();
	while (cchMax-- > 0 && *vpch != '\0' && *vpch != ' ' && *vpch != ',')
		*pch++ = *vpch++;
	*pch = '\0';
}


DealWithCommands()
{
	SY * psy;
	unsigned bsy;
	char * szMacro;
	char szIntl [65];
	char rgchFunc[32];
	char rgchArgs[128];
	char * pchArgs;
	int cargs, cargsReq;
	int mct, grfMode;
	BOOL fCab;
	char * pch;
	int iLetter;
	int cabi, ieldi;
	int wParam;
	BOOL fWParam;
	int ucc;
	BOOL fSetUndo;
	BOOL fRepeatable;
	BOOL fDyadic;

	for (;;)
		{
		ReadLine();
		if (feof(pflCmd))
			break;

		FetchMacroName(szIntl, sizeof (szIntl));
		szMacro = szIntl;
		while (*szMacro != '\0' && *szMacro != ':')
			szMacro += 1;
		if (*szMacro == ':')
			*szMacro++ = '\0';
		else
			szMacro = szIntl;


		if (*vpch++ != ',')
			{
			vpch = vpchStart; /* restart line */
			break; /* no comma, must be end of commands */
			}

		SkipSpaces();
		vfStmt = fFalse;
		if (*vpch == ';')
			{
			vpch += 1;
			vfStmt = fTrue;
			}

		/* Move function name to rgchFunc */
		FetchWord(rgchFunc, sizeof (rgchFunc), fFalse /* fUpper */ );

			/* See if we really fetched a type identifier */
			{
			int tk;
			int ipsz;

			tk = 0;
			for (ipsz = 0; ipsz < ipszMax; ++ipsz)
				if (strcmpi(rgchFunc, rgpszKeywords[ipsz]) == 0)
					{
					tk = tkFirst + ipsz;
					break;
					}

			switch (tk)
				{
			default:
				velr = elrVoid;
				vfStmt = fTrue;
				goto LNoType;

			case tkInt:
				velr = elrInt;
				break;

			case tkFloat:
				velr = elrNum;
				break;

			case tkHpsd:
			case tkSd:
				velr = elrSd;
				break;
				}

			/* Move function name to rgchFunc */
			FetchWord(rgchFunc, sizeof (rgchFunc), fFalse /* fUpper */ );
			}

LNoType:
		/* Deal with arguments */
		SkipSpaces();
		cargs = 0;
		cargsReq = -1;
		pchArgs = rgchArgs;
		mct = mctCmd; /* REVIEW: mctSdm */
		fWParam = fFalse;

		if ((fCab = *vpch == '{') || *vpch == '(')
			{
			if (!fCab)
				{
				mct = mctEl;
				}
			++vpch; /* skip paren or brace */
			SkipSpaces();
			if (*vpch == ';')
				{
				++vpch;
				cargsReq = -2;
				}
			for (;;)
				{
				switch (TkParse())
					{
				case tkInt:
					*pchArgs = elpInt;
					break;

				case tkString:
					*pchArgs = elpHst;
					break;

				case tkFloat:
					*pchArgs = elpNum;
					break;

				case tkEither:
					*pchArgs = elpHpsdNum;
					break;

				case tkSd:
				case tkHpsd:
					*pchArgs = elpHpsd;
					break;

				case ')':
				case '}':
					goto LEndOfList;

				case tkNumber:
					if (fWParam)
						{
						Error("only one allowed");
						return;
						}

					wParam = WFetchNumber();
					break;

				default:
					Error("unknown type");
					return;
					}

				++cargs;
				/* next ch must be comma or close */
				SkipSpaces();

				pchArgs += 1;

				if (*vpch == ';')
					cargsReq = cargs;
				else  if (*vpch != ',')
					break;

				++vpch; /* skip comma or semi */
				}

			if (*vpch++ != (fCab ? '}' : ')'))
				{
				Error("missing paren or brace");
				return;
				}
LEndOfList: 
			;
			}

		cabi = cabiNil;
		ieldi = -1;
		SkipSpaces();
		if (*vpch == '@')
			{
			int lno;
			FILE * pfl;
			char szFileName [40];
			char szDlgName [40];
			char * pch;

			/* New SDM dialog; need to read cabi from .hs file */
			vpch += 1; /* skip '@' */
			mct = mctSdm;
			pch = szFileName;
			if (*szBuildDir == '\0')
				strcpy(pch, szDlgDir);
			else
				strcpy(pch, szBuildDir);
			pch += strlen(pch);
			FetchFileName(pch, 
					sizeof (szFileName) - (pch - szFileName));
			strcpy(szDlgName, pch);
			strcat(szFileName, ".hs");
			if ((cabi = CabiFromSzFile(szFileName)) == cabiNil)
				{
				Error("could not get cabi from .hs file");
				return;
				}

			/* Now grep through the elxdefs.h file looking
				for the dialog name.  ieldi is the index into
				this file */
			strcpy(szFileName, szDlgDir);
			strcat(szFileName, "elx.txt");
			if ((pfl = fopen(szFileName, "r")) == NULL)
				{
				Error("cannot open elx.txt file");
				return;
				}
			lno = 0;
			while (fgets(szFileName, sizeof (szFileName), pfl))
				{
				char * pchT;

				if (szFileName[0] == '\0' || szFileName[0] == '\n')
					continue;
				if ((pchT = strchr(szFileName, '.')) == NULL)
					{
					Error("bad elx.txt file");
					fclose(pfl);
					return;
					}
				*pchT = '\0';
				if (strcmpi(szFileName, szDlgName) == 0)
					{
					ieldi = lno;
					break;
					}
				lno += 1;
				}

			fclose(pfl);


			if (vfDebug)
				{
				strcpy(szFileName, szDlgDir);
				strcat(szFileName, "elxdbg.txt");
				if ((pfl = fopen(szFileName, "r")) == NULL)
					{
					Error("cannot open elxdbg.txt file");
					return;
					}
				while (fgets(szFileName, sizeof (szFileName), pfl))
					{
					char * pchT;

					if (szFileName[0] == '\0' || szFileName[0] == '\n')
						continue;
					if ((pchT = strchr(szFileName, '.')) == NULL)
						{
						Error("bad elx.txt file");
						fclose(pfl);
						return;
						}
					*pchT = '\0';
					if (strcmpi(szFileName, szDlgName) == 0)
						{
						ieldi = lno;
						break;
						}
					lno += 1;
					}
				fclose(pfl);
				}
			}

		SkipSpaces();
		grfMode = 0;
		fDyadic = fRepeatable = fFalse;
		if (*vpch == '*')
			{
			/* mode bits */
			vpch++;
			for (;;)
				{
				switch (ChUpper(*vpch))
					{
				case 'R':
					fRepeatable = fTrue;
					break;

				case 'L':
					grfMode |= grfLockedMode;
					break;

				case 'A':
					grfMode |= grfAnnotateMode;
					break;

				case 'O':
					grfMode |= grfOutlineMode;
					break;

				case 'H':
					grfMode |= grfHeaderMode;
					break;

				case 'F':
					grfMode |= grfFootnoteMode;
					break;

				case 'P':
					grfMode |= grfPageMode;
					break;

				case 'M':
					grfMode |= grfMacroMode;
					break;

				case 'B':
					grfMode |= grfBlockMode;
					break;

				case 'E':
					grfMode |= grfExtendMode;
					break;

				case 'D':
					grfMode |= grfNeedsDoc;
					break;

				case 'K':
					grfMode |= grfMomLock;
					break;
					
				case 'Y':
					fDyadic = fTrue;
					break;

				default:
					goto LEndModes;
					}
				vpch++;
				}
			}
LEndModes:

			{
		/* Check for undo stuff */
			int isz;
			char * pch;
			char szBuf [20];

			ucc = 0;
			fSetUndo = fFalse;
			SkipSpaces();
			pch = vpch;
			if (*pch++ == 'u' && *pch++ == 'c' && *pch++ == 'c')
				{
				FetchWord(szBuf, sizeof (szBuf), fTrue);
				for (isz = 0; isz < iszMaxUcc; ++isz)
					{
					if (strcmpi(szBuf, rgszUcc[isz]) == 0)
						{
						fSetUndo = fTrue;
						ucc = 1 + isz;
						goto LFoundUcc;
						}
					}

				Error("unknown undo class");
				return;
				}
LFoundUcc: 
			;
			}


		if (cargsReq == -1)
			cargsReq = cargs;
		else  if (cargsReq == -2)
			cargsReq = 0;

		switch (mct)
			{
		default:
			Error("bad mct");
			return;

		case mctCmd:
			/* REVIEW: arguments */
			bsy = BsyAddSymbol(szIntl, rgchFunc,
					rgchArgs, fWParam ? -1 : cargs, wParam, mct);
			psy = (SY *) ((char *) &vpsyt->grpsy[0] + bsy);
			psy->u.scmd.ucc = ucc;
			psy->u.scmd.fSetUndo = fSetUndo;
			break;

		case mctSdm:
			bsy = BsyAddSymbol(szIntl, rgchFunc, 0,
					cabi, ieldi, mct);
			psy = (SY *) ((char *) &vpsyt->grpsy[0] + bsy);
			psy->u.ssdm.ucc = ucc;
			psy->u.ssdm.fSetUndo = fSetUndo;
			break;

		case mctEl:
			bsy = BsyAddSymbol(szIntl, rgchFunc, rgchArgs,
					cargsReq, cargs, mct);
			break;
			}


		psy = (SY *) ((char *) &vpsyt->grpsy[0] + bsy);
		psy->mct = mct | grfMode;/* get the grfs into mct */
		if (mct == mctCmd || mct == mctSdm)
			{
			psy->u.scmd.fRepeatable = fRepeatable;
			if (fDyadic)
				Error("dyadic flag not available for commands");
			}
		else
			{
			psy->u.sel.fDyadic = fDyadic;
			}


		if (mct == mctCmd || mct == mctSdm)
			psy->u.scmd.iidstr = -1;

		SkipSpaces();

		/* Check for token to put in header file */
		if (*vpch == ',')
			{
			char rgchToken[32];
			vpch++;
			FetchWord(rgchToken, sizeof (rgchToken),
					fFalse /* fUpper */ );
			SkipSpaces();
			fprintf(pflH, "#define %s BcmOfBcmSz(%d, \"%s\")\n",
					rgchToken,
					BsyFindSymbol(szIntl),
					szIntl);
			}

			{
			char szBcm [40];
			char * pch;
			int ibcm;

			ibcm = IbcmAddBcmToRgbcm(bsy, szMacro, szIntl);
			szBcm[0] = 'i';
			szBcm[1] = 'b';
			szBcm[2] = 'c';
			szBcm[3] = 'm';
			if (*(pch = &szMacro[strlen(szMacro) - 1]) == '$')
				*pch = '_';
			strcpy(szBcm + 4, szMacro);
			fprintf(pflHLP, "#define %s %d\n", szBcm, ibcm);
			}

		if (((psy->mct & 7) == mctCmd || (psy->mct & 7) == mctSdm) && 
				*vpch == '?')
			{
			int iszHelp;
			char szHelp [256];

			vpch++;
			FetchWord(szHelp, sizeof (szHelp), fFalse);
			if (!FLookupHelp(szHelp, &iszHelp))
				iszHelp = IszAddHelp(szHelp);
			psy->u.scmd.iidstr = iszHelp;
			SkipSpaces();
			}

		if (*vpch != '\0')
			{
			Error("extraneous info");
			return;
			}
		}

#ifdef DEBUG
		{
		unsigned bsy;
		SY * psy;

		bsy = 0;

		while (bsy < vpsyt->bsy)
			{
			char szBuf [256];

			psy = (SY *) ((char *) vpsyt->grpsy + bsy);
			StToSz(psy->st, szBuf);
			printf("%s\n", szBuf);
			bsy += cbSY + psy->st[0] + ((psy->mct == mctCmd || psy->mct == mctSdm) ?
					/*REVIEW*/ 0 : psy->u.sel.cagdMax);
			}
		}
#endif

}


/* V K  F R O M  P S Z */
/* Given the name of a key, return its Windows Virtual Key Code */
int VkFromPsz(psz)
char * psz;
{
	int ivkd;
	VKD * pvkd;

	/* Letters and digits are not in the table, handle them here */
	if (*(psz + 1) == 0)
		{
		if (isalnum(*psz))
			return *psz; /* vk is same as ASCII */
		switch (*psz)
			{
		case '+':
			return 0xbb; /* REVIEW: Usually standard! */

		case '-':
			return 0xbd; /* REVIEW: Usually standard! */

		case '[':
			return 0xdb;

		case ']':
			return 0xdd;
			}
		}

	for (ivkd = 0, pvkd = rgvkd; ivkd < ivkdMax; ++ivkd, ++pvkd)
		{
		if (strcmpi(psz, pvkd->psz) == 0)
			return pvkd->vk;
		}
	return vkNil;
}


DealWithKeymap()
{
	char * pch;
	VKD * pvkd;
	BOOL f2, fShift, fCtrl, fAlt;
	int ivkd, kc, bsy;
	char rgchName[32];
	char rgchMacro[32];
	char rgch1[16], rgch2[16];
	KME * pkme;

	FetchWord(rgchName, sizeof (rgchName), fFalse /* fUpper */);

	if (vpkmp->pszName != 0) /* REVIEW */
		{
		Error("too many keymaps");
		return;
		}

	vpkmp->pszName = strsave(rgchName);

	for (;;)
		{
		ReadLine();
		if (feof(pflCmd))
			return;

		FetchWord(rgch1, sizeof(rgch1), fTrue /* fUpper */);
		SkipSpaces();
		fShift = fCtrl = fAlt = f2 = fFalse;
		if (*vpch != ',')
			{
			FetchWord(rgch2, sizeof (rgch2), fTrue /* fUpper */);
			SkipSpaces();
			if (*vpch != ',')
				{
				vpch = vpchStart;
				return;
				}
			f2 = fTrue;
			}
		++vpch; /* skip past comma */

		if (f2)
			{
			char * pch;
			/* we have some shift modifiers in rgch1 */
			pch = &rgch1[0];
			while (*pch)
				{
				switch (*pch++)
					{
				case chShift:
					fShift = fTrue;
					break;

				case chCtrl:
					fCtrl = fTrue;
					break;

				case chAlt:
					fAlt = fTrue;
					break;

				default:
					Error("key modifier");
					return;
					}
				}
			kc = VkFromPsz(rgch2);
			}
		else
			{
			kc = VkFromPsz(rgch1);
			}

		if (kc == vkNil)
			{
			Error("key name");
			return;
			}

		/* Apply shift codes to get a real kc */
		if (fCtrl)
			kc = KcCtrl(kc);
		if (fShift)
			kc = KcShift(kc);
		if (fAlt)
			kc = KcAlt(kc);

		/* Fetch macro name (pointer is currently past comma) */
		FetchMacroName(rgchMacro, sizeof (rgchMacro));

		if (isdigit(rgchMacro[0]))
			{
			int w = 0;
			char *pch = &rgchMacro[0];
			while (isdigit(*pch))
				w = (10 * w) + (*pch++ - '0');
			AddKeyToKmp(&vpkmp, kc, w, ktInsert);
			}
		else  if ((bsy = BsyFindEnglish(rgchMacro)) == 0xffff)
			{
			Error("undefined macro");
			return;
			}
		else
			AddKeyToKmp(&vpkmp, kc, bsy, ktMacro);
		}
}


/* F  S E A R C H  K M P */
/* Binary search a keymap for a given kc.  Sets *pikme to where kc is
found (returns fTrue), or where it would be (returns fFalse). */
FSearchKmp(hkmp, kcSearch, pikme)
KMP ** hkmp;
int kcSearch;
int * pikme;
{
	KMP * pkmp;
	KME * rgkme;
	int iMin, iLim, iGuess, kc;

	pkmp = *hkmp;
	rgkme = &pkmp->rgkme[0];
	iMin = 0;
	iLim = pkmp->ikmeMac;
	if (iLim > 0 && kcSearch >= rgkme[0].kc)
		{
		while (iMin < iLim)
			{
			iGuess = (iMin + iLim) >> 1;
			if ((kc = rgkme[iGuess].kc) == kcSearch)
				{
				*pikme = iGuess;
				return fTrue;
				}
			if (kc < kcSearch)
				iMin = iGuess + 1;
			else
				iLim = iGuess;
			}
		}
	*pikme = iMin;

	return fFalse;
}


AddKeyToKmp(hkmp, kc, w, kt)
KMP ** hkmp;
int kc, w, kt;
{
	KMP * pkmp;
	KME * pkme;
	int ikme;
	BOOL fFound;

	fFound = FSearchKmp(hkmp, kc, &ikme);
	pkmp = *hkmp;
	pkme = &pkmp->rgkme[ikme];
	if (!fFound)
		{
		if (pkmp->ikmeMac == pkmp->ikmeMax)
			{
			/* REVIEW: allocate more room */
			Error("too many keys");
			return;
			}
		pkmp = *hkmp;
		pkme = &pkmp->rgkme[ikme];
		bltb(pkme, pkme + 1, sizeof (KME) * (pkmp->ikmeMac - ikme));
		++pkmp->ikmeMac;
		}

	pkme->kc = kc;
	pkme->bsy = w;
	pkme->kt = kt;
}



int ibstMacBar = 0;

DealWithMenuBar()
{
	char rgchName[64];
	SY * psy;
	char rgchImi[32];
	BOOL fBuildSttb;

	fBuildSttb = (ibstMacBar == 0);
	vfMenus = fTrue;

	FetchWord(rgchName, sizeof (rgchName), fFalse /* fCase */);

	if (!fBuildSttb)
		rgicbm[viicbm++] = vicbm;

	ReadLine();
	while (!feof(pflCmd))
		{
		if (TkParse() != tkMenu)
			{
			vpch = vpchStart;
			break;
			}

		FetchWord(rgchName, 32, fFalse /* fCase */);

			{
			char * sz;

			sz = &rgchName[0];

			if (sz[0] == '\\' && sz[1] == 'a')
				{
				sz++;
				*sz = chHelp;
				}

			if (fBuildSttb)
				{
				int imtm;
				MTM * pmtm;

				SzToSt(sz, rgchName);

				InsertStInSttb(vhsttbMenu, ibstMacBar++, rgchName);
				pmtm = &vmud.rgmtm[0];
				for (imtm = 0; imtm < vmud.imtmMac; ++imtm, ++pmtm)
					pmtm->ibst += 1;
				}
			else
				{
				fprintf(pflMenu, "/* %d */  \t{ %d, St(\"%s\") },\n",
						vicbm++, 0xfff0, sz);
				}
			}

		for (;;)
			{
			int bsy;
			char rgchMacro[32];

			ReadLine();
			if (feof(pflCmd))
				break;
			FetchMacroName(rgchName, sizeof (rgchName));
			if (strcmpi(rgchName, "FILECACHE") == 0)
				{
				bsy = bsyFileCache;
				goto LSep;
				}
			else  if (strcmpi(rgchName, "WNDCACHE") == 0)
				{
				bsy = bsyWndCache;
				goto LSep;
				}
			else  if (strcmpi(rgchName, "SEPARATOR") == 0)
				{
				bsy = bsySeparator;
LSep:
				if (fBuildSttb)
					{
					MTM * pmtm;
					pmtm = &vmud.rgmtm[vmud.imtmMac++];
					pmtm->imnu = ibstMacBar - 1;
					pmtm->ibst = 0;
					pmtm->fChecked = fFalse;
					pmtm->fRemove = fFalse;
					pmtm->fDisabled = fFalse;
					pmtm->bsy = bsy;
					}
				else
					{
					fprintf(pflMenu,
							"/* %d */  \t{ %d, St(\"\") },\n",
							vicbm++, bsy);
					}
				continue;
				}
			if (*vpch++ != ',')
				{
				vpch = vpchStart;
				break;
				}
			FetchMacroName(rgchMacro, sizeof (rgchMacro));
			if ((bsy = BsyFindEnglish(rgchMacro)) == 0xffff)
				{
				Error("undefined macro");
				return;
				}

			/* given the function name, make an imiFoo name */
			/* REVIEW: meathod is pretty tacky */
			psy = (SY *) (&vpsyt->grpsy[0] + bsy);
			if (!(psy->mct & grfWritten))
				{
				char szBuf [40];
				StToSz(psy->st, szBuf);

				strcpy(rgchImi, psy->pszFunc);
				rgchImi[0] = 'i';
				rgchImi[1] = 'm';
				rgchImi[2] = 'i';
				fprintf(pflH,
						"#define %s BcmOfBcmSz(%d, \"%s\")\n",
						rgchImi, bsy, szBuf);
				psy->mct |= grfWritten;
				}

			if (fBuildSttb)
				{
				MTM * pmtm;

				SzToStInPlace(rgchName);
				pmtm = &vmud.rgmtm[vmud.imtmMac++];
				pmtm->imnu = ibstMacBar - 1;
				pmtm->ibst = IbstAddStToSttb(vhsttbMenu,
						rgchName);
				pmtm->fChecked = fFalse;
				pmtm->fDisabled = fFalse;
				pmtm->fRemove = fFalse;
				pmtm->bsy = bsy;
				}
			else
				{
				fprintf(pflMenu, "/* %d */  \t{ %d, St(\"%s\") },\n",
						vicbm++, bsy, rgchName);
				}

			}

		}

	if (!fBuildSttb)
		fprintf(pflMenu, "/* %d */  \t{ %d, St(\"\") },\n",
				vicbm++, 0xfff1);
}




/* R E A D  L I N E */
/* Read the next non-blank, non-comment line and advance the global pointer
past any spaces. */
/* REVIEW: add general #ifdef capability */
ReadLine()
{
	char ch;
	char * pch;
	BOOL fIfDef;

	fIfDef = fFalse;
	do
		{
		char * pch;
		int cch;

#define cchMaxLine sizeof (rgchLine)


		for (pch = &rgchLine[0], cch = 0; cch < cchMaxLine; )
			{
			fgets(pch, cchMaxLine - cch, pflCmd);
			++vcLine;
			cch = strlen(rgchLine);
			pch = &rgchLine[cch - 2];
			if (*pch != '\\')
				break;
			}

		/* REVIEW: #ifdef DEBUG is a total kludge! */
		if (rgchLine[0] == '#')
			{
			if (strncmp(rgchLine, "#ifdef DEBUG", 12) == 0)
				{
				if (vfDebug)
					continue;
				fIfDef = fTrue;
				}
			else  if (strncmp(rgchLine, "#ifdef PROFILE", 14) == 0)
				{
				if (vfProfile)
					continue;
				fIfDef = fTrue;
				}
			else  if (strncmp(rgchLine, "#ifdef MKTGPRVW", 15) == 0)
				{
				if (vfMktgPrvw)
					continue;
				fIfDef = fTrue;
				}
			else  if (strncmp(rgchLine, "#endif", 6) == 0)
				{
				fIfDef = fFalse;
				}
			else  if (strncmp(rgchLine, "#else", 5) == 0)
				{
				fIfDef = !fIfDef;
				}
			}

		vpch = &rgchLine[0];
		SkipSpaces();
		ch = *vpch;
		}
	while (!feof(pflCmd) &&
			(fIfDef || ch == '#' || ch == '\0' || ch == '\n'));

	/* Replace newline or sharp with EOL */
	for (pch = rgchLine; (ch = *pch) && ch != '#' && ch != '\n'; ++pch)
		;
	*pch = '\0';

	vpchStart = vpch;
}



/* S K I P  S P A C E S */
/* Advance global pointer past spaces */
SkipSpaces()
{
	while (isspace(*(unsigned char *)vpch))
		++vpch;
}


/* F E T C H  W O R D */
/* Fetch a word consisting of letters, digits, and/or underscores, ignoring
leading blanks.  If fUpper, all letters are converted to upper case. */
FetchWord(szFetch, cchMax, fUpper)
char * szFetch;
int cchMax;
BOOL fUpper;
{
	char * pch;
	int ch;
	BOOL fQuote;

	pch = szFetch;
	SkipSpaces();
	if (fQuote = (*vpch == '"'))
		++vpch;

	while (cchMax-- > 0 && ((ch = ((int) *vpch) & 0xff), 
			(fQuote && ch != '"') ||
			isalnum(ch) || ch == '_' || ch == '+' || ch == '-' ||
			ch == '[' || ch == ']' || ch == '~' || ch == '&' ||
			ch > 127))
		{
		*pch++ = fUpper ? ChUpper(ch) : ch;
		++vpch;
		}

	if (fQuote)
		{
		if (*vpch != '"')
			Error("missing quote");
		else
			++vpch;
		}

	*pch = 0;
}



/* T K  P A R S E */
int TkParse()
{
	char rgch[300];
	int cch;

	SkipSpaces();

	if (isalpha(*vpch))
		{
		int ipsz;

		/* Collect word */
		FetchWord(rgch, sizeof (rgch), fTrue /* fUpper */);

		/* Check for keyword */
		for (ipsz = 0; ipsz < ipszMax; ++ipsz)
			{
			if (strcmpi(rgch, rgpszKeywords[ipsz]) == 0)
				return ipsz + tkFirst;
			}

		Error("keyword");
		return 0;
		}

	else  if (isdigit(*vpch))
		{
		return tkNumber;
		}

	return *vpch++;
}


WFetchNumber()
{
	int w;

	w = 0;
	while (isdigit(*vpch))
		{
		w = 10 * w + *vpch++ - '0';
		}

	return w;
}


FLookupHelp(szHelp, piszHelp)
char * szHelp;
int * piszHelp;
{
	int iszHelp;
	HSD * phsd;

	iszHelp = 0;
	for (phsd = vphsdBase; phsd != NULL; phsd = phsd->phsdNext)
		{
		if (strcmpi(szHelp, phsd->sz) == 0)
			{
			*piszHelp = iszHelp;
			return fTrue;
			}
		++iszHelp;
		}
	return fFalse;
}


IszAddHelp(szHelp)
{
	HSD ** pphsd, * phsd;

	if (vphsdBase == NULL)
		pphsd = &vphsdBase;
	else
		pphsd = &vphsdEnd->phsdNext;

	if ((phsd = (HSD *) malloc(sizeof (HSD))) == NULL)
		{
		Error("out of memory");
		return -1;
		}

	*pphsd = phsd;
	phsd->phsdNext = NULL;
	phsd->sz = strsave(szHelp);
	vphsdEnd = phsd;

	return viszHelp++;
}


/* I B C M  A D D  B C M  T O  R G B C M */
/* Adds a bcm to the rgbcm table, in the same position as it was in before
if possible */
IbcmAddBcmToRgbcm(bcm, szEnglish, szName)
int bcm;
char * szEnglish;
char * szName;
{
	int ibcm, ibcmFree;
	int ibcmNextFree = -1;

	ibcmFree = vibcm;
	for (ibcm = 0; ibcm < ibcmMacOld; ++ibcm)
		{
		if (rgszOld[ibcm] == 0)
			{
			if (ibcmNextFree == -1)
				ibcmNextFree = ibcm;
			continue;
			}
		else  if (rgszOld[ibcm] != (char *)1 &&
				strcmpi(szEnglish, rgszOld[ibcm]) == 0)
			{
#ifdef REVIEW
			if (rgbcm[ibcm] == 0xffff)
#endif
				{
				rgbcm[ibcm] = bcm;
				}
#ifdef REVIEW
			else
				{
				if (strcmpi(szEnglish[ibcm], szEnglish) != 0)
					{
					printf("BUG: (%s) (%s) should be the same!\n", szEnglish[ibcm], szEnglish);
					Error("internal");
					}
				}
#endif
			ibcmFree = ibcm;
			goto LReturn;
			}
		}

	/* re-use an old slot only if this is a new command */
	if (ibcmNextFree != -1)
		{
		ibcmFree = ibcmNextFree;
		rgszOld[ibcmNextFree] = (char *) 1;
		}

	rgbcm[ibcmFree] = bcm;

	if (ibcmFree == vibcm)
		{
		if (vibcm >= ibcmMax)
			{
			Error("too many commands");
			return;
			}

		vibcm += 1;
		}

LReturn:
	if (rgszEnglish[ibcmFree] != NULL)
		{
		SY * psyNew;
		SY * psyOld;
		int bsy;

		psyNew = (SY *) ((char *) vpsyt->grpsy + bcm);
		bsy = psyNew->bsyNext;
		psyOld = (SY *) ((char *) vpsyt->grpsy + bsy);
		if (psyNew->st[0] != 0)
#ifdef REVIEW
			bsy = *((int *) &psyNew->st[1]);
		else
			bsy = psyNew->bsyNext;
		psyOld = (SY *) ((char *) vpsyt->grpsy + bsy);

		if (!FStEqSt(psyOld->st, psyNew->st))
#endif
			{
/*PrintSt(psyOld->st); printf(" <-> "); PrintSt(psyNew->st); printf("\n");*/
			Warning("command/statement translation mis-match");
			}
		}
	else
		{
		rgszEnglish[ibcmFree] = strsave(szEnglish);
		}

	return ibcmFree;
}


InitRgbcm()
{
	register int ibcm;

	for (ibcm = 0; ibcm < ibcmMax; ++ibcm)
		{
		rgszOld[ibcm] = 0;
		rgbcm[ibcm] = 0xffff;
		}
}


CabiFromSzFile(szFileName)
char * szFileName;
{
	int cabi;
	FILE * pfl;
	char szLineBuf [100];

	cabi = cabiNil;
	if ((pfl = fopen(szFileName, "r")) == NULL)
		{
		Error("Can't open .hs file\n");
		return;
		}

	while (fgets(szLineBuf, sizeof (szLineBuf), pfl) != NULL)
		{
		if (strncmp(szLineBuf, "#define cabi", 12) == 0)
			{
			char * pch = szLineBuf + 12;
			while (*pch != ' ' && *pch != '\0')
				pch += 1;
			while (*pch == ' ')
				pch += 1;
			cabi = atoi(pch);
			break;
			}
		}

	fclose(pfl);

	return cabi;
}


FStEqSt(st1, st2)
char * st1, * st2;
{
	int cch;

	if ((cch = *st1++) != *st2++)
		return fFalse;
	while (cch-- > 0 && *st1++ != *st2++)
		;
	return cch == 0;
}


#ifdef REVIEW
PrintSt(st)
char * st;
{
	char szBuf [40];

	StToSz(st, szBuf);
	printf("%s", szBuf);
}


#endif
