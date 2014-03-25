#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOSYSMETRICS
#define NOBITMAP
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NOCTLMGR
#define NODRAWTEXT
#define NOFONT
#define NOGDI
#define NOHDC
#define NOMB
#define NOMENUS
#define NOMETAFILE
#define NOMSG
#define NOPEN
#define NOPOINT
#define NORECT
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#define NOKANJI
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */

#include "newexe.h"  
#include "doslib.h"
#include "debug.h"



#ifdef PROTOTYPE
#include "ripaux.cpt"
#endif /* PROTOTYPE */

#define MyLock(ps)      HIWORD(GlobalHandle(ps))

#define chLF    0x0a
#define chCR    0x0d


/* Debug Symbol Table Structures
	-----------------------------
For each symbol table (map): (MAPDEF)
-------------------------------------------------------------------------------------------------
| map_ptr | lsa | pgm_ent | abs_cnt | abs_ptr | seg_cnt | seg_ptr | nam_max | nam_len | name... |
------------------------------------------------------------------------------------------------- */
struct MAPDEF
	{
	unsigned    map_ptr;    /* 16 bit ptr to next map (0 if end)    */
	unsigned    lsa    ;    /* 16 bit Load Segment address          */
	unsigned    pgm_ent;    /* 16 bit entry point segment value     */
	int         abs_cnt;    /* 16 bit count of constants in map     */
	unsigned    abs_ptr;    /* 16 bit ptr to   constant chain       */
	int         seg_cnt;    /* 16 bit count of segments in map      */
	unsigned    seg_ptr;    /* 16 bit ptr to   segment chain        */
	char        nam_max;    /*  8 bit Maximum Symbol name length    */
	char        nam_len;    /*  8 bit Symbol table name length      */
};

struct MAPEND
	{
	unsigned        chnend;         /* end of map chain (0) */
	char            rel;            /* release              */
	char            ver;            /* version              */
};

/* For each segment/group within a symbol table: (SEGDEF)
--------------------------------------------------------------
| nxt_seg | sym_cnt | sym_ptr | seg_lsa | name_len | name... |
-------------------------------------------------------------- */
struct SEGDEF
	{
	unsigned    nxt_seg;    /* 16 bit ptr to next segment(0 if end) */
	int         sym_cnt;    /* 16 bit count of symbols in sym list  */
	unsigned    sym_ptr;    /* 16 bit ptr to symbol list            */
	unsigned    seg_lsa;    /* 16 bit Load Segment address          */
	unsigned    seg_in0;    /* 16 bit instance 0 physical address   */
	unsigned    seg_in1;    /* 16 bit instance 1 physical address   */
	unsigned    seg_in2;    /* 16 bit instance 2 physical address   */
	unsigned    seg_in3;    /* 16 bit instance 3 physical address   */
	unsigned    seg_lin;    /* 16 bit ptr to line number record     */
	char        seg_ldd;    /*  8 bit boolean 0 if seg not loaded   */
	char        seg_cin;    /*  8 bit current instance              */
	char        nam_len;    /*  8 bit Segment name length           */
};

/*  Followed by a list of SYMDEF's..
	for each symbol within a segment/group: (SYMDEF)
-------------------------------
| sym_val | nam_len | name... |
------------------------------- */
struct SYMDEF
	{
	unsigned    sym_val;            /* 16 bit symbol addr or const  */
	char        nam_len;            /*  8 bit symbol name length    */
	/*  char        nam[nam_len]; */    /* name                         */
};


int hSymFile = -1;          /* .sym file handle */
struct MAPDEF MapDef;
struct SEGDEF SegDef;
struct SYMDEF SymDef;

csconst char szPcodeMap[] = "pcodemap.txt";
csconst char szCom1[] = "com1";
csconst char szOpusRip[] = "opus.rip";
csconst char stOpus[] = "\004OPUS";


/* %%Function:FGetSegSymb %%Owner:BRADV */
EXPORT FGetSegSymb(Seg, szSymFile, szSegName)
/* scan a .sym file, stop at the desired segment info, and return
* the segment name
*/
unsigned Seg;
char szSymFile[];
char szSegName[];
{
	int iSeg;
	OFSTRUCT SymReOpenBuff;     /* .sym file reopen buffer */

	if (hSymFile != -1)
		FCloseDoshnd(hSymFile);

	/* open the .sym file */
	if ((hSymFile = HOpenFileSearch (szSymFile, &SymReOpenBuff, OF_READ))
			== -1)
		goto ErrorExit;

	/* get the map file definition data */
	if (CchReadDoshnd(hSymFile, (struct MAPDEF far *)(&MapDef),
			sizeof(struct MAPDEF)) != sizeof(struct MAPDEF))
		goto ErrorExit;

	/* exit if looking for non-existant segment */
	if ((Seg - 1) >= MapDef.seg_cnt)
		goto ErrorExit;

	/* skip thru the .sym file until at the desired seg entry */
	for (iSeg = 1, SegDef.nxt_seg = MapDef.seg_ptr; iSeg <= Seg; iSeg++)
		{
		if (SegDef.nxt_seg == 0)
			goto ErrorExit;
		if (DwSeekDw(hSymFile, ((unsigned long)SegDef.nxt_seg) << 4, 0) < 0L)
			goto ErrorExit;
		if (CchReadDoshnd(hSymFile, (struct SEGDEF far *)(&SegDef),
				sizeof(struct SEGDEF)) != sizeof(struct SEGDEF))
			goto ErrorExit;
		}

	/* get the segment name */
	if (CchReadDoshnd(hSymFile, (LPSTR)szSegName, SegDef.nam_len) != SegDef.nam_len)
		goto ErrorExit;
	szSegName[SegDef.nam_len] = 0;

	return (fTrue);


ErrorExit:
	if (hSymFile != -1)
		{
		FCloseDoshnd(hSymFile);
		hSymFile = -1;
		}
	return (fFalse);
}



/* %%Function:FGetSymbol %%Owner:BRADV */
EXPORT FGetSymbol(Offset, szSymbol)
/* find a symbol in a segment data area of a .sym file and return the
* symbol name
*/
unsigned Offset;
char szSymbol[];
{
	int iSym = 0;
	struct SYMDEF prev_SymDef;

	/* scan thru the entries until either:
		*   1) an entry with an offset > what we are passed is found
		*   2) we run out of table (it is assumed that the desired
		*      entry must be the last one found)
		*/
	if (CchReadDoshnd(hSymFile, (struct SYMDEF far *)(&SymDef),
			sizeof(struct SYMDEF)) != sizeof(struct SYMDEF))
		goto ErrorExit;
	if (CchReadDoshnd(hSymFile, (LPSTR)szSymbol, SymDef.nam_len) != SymDef.nam_len)
		goto ErrorExit;
	szSymbol[SymDef.nam_len] = 0;
	prev_SymDef = SymDef;

	for (iSym = 1; iSym <= SegDef.sym_cnt; iSym++)
		{
		if (CchReadDoshnd(hSymFile, (struct SYMDEF far *)(&SymDef),
				sizeof(struct SYMDEF)) != sizeof(struct SYMDEF))
			goto ErrorExit;

		if (SymDef.sym_val > Offset)
			break;

		if (CchReadDoshnd(hSymFile, (LPSTR)szSymbol, SymDef.nam_len) != SymDef.nam_len)
			goto ErrorExit;
		szSymbol[SymDef.nam_len] = 0;

		prev_SymDef = SymDef;
		}

	SymDef = prev_SymDef;

	return (fTrue);


ErrorExit:
	if (hSymFile != -1)
		{
		FCloseDoshnd(hSymFile);
		hSymFile = -1;
		}
	return (fFalse);
}



/* %%Function:GetExeHead %%Owner:BRADV */
GetExeHead()
	/* get the contents of hExeHead in the kernel data space */
{
#undef GlobalAlloc
	extern int far PASCAL GlobalAlloc();
	int far *lp_hExeHead;

	/* get the offset of hExeHead */
	if (!FFindSymbName("kernel.sym", "HEXEHEAD"))
		return (0);

	/* *** KLUDGE ALERT * KLUDGE ALERT * KLUDGE ALERT * KLUDGE ALERT ***
		* since the kernel's CS = DS we can use a kernel's proc segment
		* to get the kernel's data segment
		* (and now for my next magical trick...)
		*/
	lp_hExeHead = (int far *)(((unsigned long)(&GlobalAlloc) & 0xffff0000) + SymDef.sym_val);

	return (*lp_hExeHead);
}





/* %%Function:FFindSymbName %%Owner:BRADV */
FFindSymbName(szFileName, szSymbol)
/* scan through a .sym file looking for a particular symbol
* exit with MapDef, Segdef, and Symdef setup if we found it
*/
char szFileName[];
char szSymbol[];
{
	int iSeg, iSym;
	int cbsz;
	char rgch[256];

	/* find out the string length of the symbol */
	for (cbsz = 0; szSymbol[cbsz] != 0; cbsz++)
		;

	/* walk through all segments of the .sym file for the symbol */
	for (iSeg = 1; ; iSeg++)
		{
		/* point ourselves to the beginning of a segment */
		if (!FGetSegSymb(iSeg, szFileName, rgch))
			goto ErrorExit;

		/* walk thru all symbols of this segment looking for the symbol */
		for (iSym = 0; iSym < SegDef.sym_cnt; iSym++)
			{
			if (CchReadDoshnd(hSymFile, (struct SYMDEF far *)(&SymDef),
					sizeof(struct SYMDEF)) != sizeof(struct SYMDEF))
				goto ErrorExit;

			if (CchReadDoshnd(hSymFile, (LPCH)rgch, SymDef.nam_len) != SymDef.nam_len)
				goto ErrorExit;

			if (SymDef.nam_len != cbsz)
				continue;

			if (!FNeRgch(rgch, szSymbol, cbsz))
				return (fTrue);
			}
		}

ErrorExit:
	if (hSymFile != -1)
		{
		FCloseDoshnd(hSymFile);
		hSymFile = -1;
		}
	return (fFalse);
}


/* %%Function:FFindSeg %%Owner:BRADV */
EXPORT FFindSeg(CSvalue, szSymFile, szSegName)
/* find the segment name for a given CS segment
* exit with MapDef and Segdef setup if we found it
*/
WORD CSvalue;
char szSymFile[];
char szSegName[];
{
	HANDLE hExe;
	struct new_exe far *pExe;
	struct new_seg1 far *pSeg;
	char far *pFileName;
	BYTE c;
	int iSeg, cch, isz;
	char *pstOpus;
	char rgchFileName[10];

	hExe = GetExeHead();    /* get the handle to Window's exe headers */

	/* walk thru all exe headers if needed */
	while (hExe)
		{
		pExe = (struct new_exe far *)((DWORD)hExe << 16);
		pSeg = (struct new_seg1 far *)( ((DWORD)hExe << 16) |
				pExe->ne_segtab );

		/* check all segments of this exe file */
		for (iSeg = 1; iSeg <= pExe->ne_cseg; iSeg++, pSeg++)
			{
			if (!((HANDLE)pExe->ne_flags & NENOTP))   /* not a library */
				{
				pFileName = (LPSTR)( ((DWORD)hExe << 16) |
						pExe->ne_restab );
				pstOpus = SzNear(stOpus);
				bltbxNat(pFileName, rgchFileName, pstOpus[0] + 1);
				if (FNeRgch(rgchFileName, pstOpus, pstOpus[0] + 1))
					continue;
				}
			if (MyLock( (HANDLE)pSeg->ns_handle ) == CSvalue)
				{
				pFileName = (LPSTR)( ((DWORD)hExe << 16) |
						pExe->ne_restab );
				cch = (int)((BYTE)*pFileName++);

				/* copy the module name from the exe header and append
					* .sym to it so we have the .sym file name
					*/
				for (isz = 0; cch > 0; cch--, isz++)
					if ((c = *pFileName++) == '.')
						break;
					else
						szSymFile[isz] = c;
				szSymFile[isz++] = '.';
				szSymFile[isz++] = 'S';
				szSymFile[isz++] = 'Y';
				szSymFile[isz++] = 'M';
				szSymFile[isz]   = 0;

				/* get the segment name for this segment */
				return (FGetSegSymb(iSeg, szSymFile, szSegName));
				}
			}

		/* get the handle of the next exe header in the chain */
		hExe = (HANDLE)NE_PNEXTEXE( *pExe );
		}

	return (fFalse);
}


CHAR *vpszStackTrace1, *vpszStackTrace2;


/* %%Function:DoStackTrace %%Owner:BRADV */
void DoStackTrace(sz)
CHAR *sz;
{
	CHAR chNull = 0;
	CHAR rgch1 [cchDbgBuf1], rgch2 [cchDbgBuf2];

	EnterDebug();

	vpszStackTrace1 = (sz == NULL ? &chNull : sz );
	vpszStackTrace2 = rgch1;
	GetStateInfo (rgch1, rgch2);
	ScribbleSz (rgch2);
	StackTrace();

	ExitDebug();
}



/* %%Function:GetSnMac %%Owner:BRADV */
EXPORT GetSnMac()
	/* return the count of pcode segments */
{
	int iSn;
	int cbRead;
	int hSegFile;
	int iBuff;
	OFSTRUCT SegReOpenBuff;
	char Buff[128];

	if ((hSegFile = HOpenFileSearch (SzNear(szPcodeMap), &SegReOpenBuff, 
			OF_READ)) == -1)
		return (-1);

	for (iSn = 0; ;)
		{
		if ((cbRead = CchReadDoshnd(hSegFile, (LPCH)Buff, 128)) <= 0)
			break;

		for (iBuff = 0; iBuff < cbRead; iBuff++)
			if (Buff[iBuff] == chLF)
				iSn++;
		}

	FCloseDoshnd(hSegFile);
	return (iSn);
}


/* %%Function:FGetPcodeSegName %%Owner:BRADV */
EXPORT FGetPcodeSegName(iPcodeSeg, szSegName)
/* get the ith pcode segment name out of pocdemap.txt */
int iPcodeSeg;
char szSegName[];
{
	int iSn;
	int cbRead;
	int hSegFile;
	int iBuff;
	int isz;
	OFSTRUCT SegReOpenBuff;
	char Buff[128];

	if ((hSegFile = HOpenFileSearch (SzNear(szPcodeMap), &SegReOpenBuff, 
			OF_READ)) == -1)
		return (fFalse);

	for (iSn = 0; ;)
		{
		if ((cbRead = CchReadDoshnd(hSegFile, (LPCH)Buff, 128)) <= 0)
			goto ErrorExit;

		for (iBuff = 0; iBuff < cbRead; iBuff++)
			{
			if (iSn == iPcodeSeg)
				goto SegNameFnd;

			if (Buff[iBuff] == chLF)
				iSn++;
			}
		}

SegNameFnd:
	for (isz = 0; ;)
		{
		if (iBuff == cbRead)
			{
			if ((cbRead = CchReadDoshnd(hSegFile, (LPCH)Buff, 128)) <= 0)
				goto ErrorExit;
			iBuff = 0;
			}

		if ((Buff[iBuff] == chCR) || (Buff[iBuff] == chLF))
			{
			szSegName[isz] = 0;
			FCloseDoshnd(hSegFile);
			return (fTrue);
			}
		else
			szSegName[isz++] = Buff[iBuff++];
		}

ErrorExit:
	FCloseDoshnd(hSegFile);
	return (fFalse);
}



/* %%Function:FGetConstSymb %%Owner:BRADV */
EXPORT FGetConstSymb(Const, szSymFile, szConstName)
/* get the symbol out of the constants section of the desired .sym file
* that matches the passed Const
*/
unsigned Const;
char szSymFile[];
char szConstName[];
{
	int iConst;
	int isz;
	OFSTRUCT SymReOpenBuff;     /* .sym file reopen buffer */

	if (hSymFile != -1)
		FCloseDoshnd(hSymFile);

	if ((hSymFile = HOpenFileSearch (szSymFile, &SymReOpenBuff, OF_READ))
			== -1)
		goto ErrorExit;

	if (CchReadDoshnd(hSymFile, (struct MAPDEF far *)(&MapDef),
			sizeof(struct MAPDEF)) != sizeof(struct MAPDEF))
		goto ErrorExit;

	/* move to start of constant symbols area */
	if (DwSeekDw(hSymFile, (unsigned long)(sizeof(struct MAPDEF) + MapDef.nam_len) + 1, 0) < 0L)
		goto ErrorExit;

	/* find the first constant that has a value = Const */
	for (iConst = 0; iConst < MapDef.abs_cnt; iConst++)
		{
		if (CchReadDoshnd(hSymFile, (struct SYMDEF far *)(&SymDef),
				sizeof(struct SYMDEF)) != sizeof(struct SYMDEF))
			goto ErrorExit;

		if (SymDef.sym_val == Const)
			{
			if (CchReadDoshnd(hSymFile, (LPSTR)szConstName, SymDef.nam_len) !=
					SymDef.nam_len)
				goto ErrorExit;
			szConstName[SymDef.nam_len] = 0;

			/* get rid of 'Q_' before symbol if necessary */
			if ((szConstName[0] == 'Q') && (szConstName[1] == '_'))
				for (isz = 0; isz < SymDef.nam_len; isz++)
					szConstName[isz] = szConstName[isz + 2];

			return (fTrue);
			}

		/* skip past this name */
		if (DwSeekDw(hSymFile, (unsigned long)(SymDef.nam_len), 1) < 0L)
			goto ErrorExit;
		}

ErrorExit:
	if (hSymFile != -1)
		{
		FCloseDoshnd(hSymFile);
		hSymFile = -1;
		}
	return (fFalse);
}


/* %%Function:HOpenOutFile %%Owner:BRADV */
EXPORT HOpenOutFile()
	/* open the output file for writing - create the file if needed */
{
	OFSTRUCT OutReOpenBuff;     /* output file reopen buffer */
	char *pszRip = SzNear(szOpusRip);

	if (OpenFile((LPSTR)pszRip, (OFSTRUCT far *)(&OutReOpenBuff),
			OF_EXIST) == -1)
		{
		return (OpenFile((LPSTR)pszRip, (OFSTRUCT far *)(&OutReOpenBuff),
				OF_WRITE | OF_CREATE));
		}
	else
		{
		return (OpenFile((LPSTR)pszRip, (OFSTRUCT far *)(&OutReOpenBuff),
				OF_WRITE));
		}
}


/* %%Function:HOpenDbgScreen %%Owner:BRADV */
EXPORT HOpenDbgScreen()
	/* open the debug screen for writing */
{
	OFSTRUCT DbgReOpenBuff;     /* debug screen file reopen buffer */
	char *pszCom1 = SzNear(szCom1);

	/* if not enabled then pretend unable to open */
	if (!vdbs.fRipToDbgScreen)
		return (-1);

	return (OpenFile((LPSTR)pszCom1, (OFSTRUCT far *)(&DbgReOpenBuff),
			OF_WRITE));
}


HOpenFileSearch (szFile, pofstruct, of)
char szFile [];
OFSTRUCT *pofstruct;
int of;

{

	int h;
	HANDLE hInstanceWin;
	char szBuffer [ichMaxFile];
	char *pch;

#ifdef DOPENFILE
	CommSzSz (SzShared("trying to open: "), szFile);
#endif /* DOPENFILE */

	if ((h = OpenFile ((LPSTR)szFile, (OFSTRUCT far *) pofstruct, of)) != -1)
		{
#ifdef DOPENFILE
		CommSz (SzShared("Succedded\r\n"));
#endif /* DOPENFILE */
		return h;
		}

	/* try OPUS directory */
	CchGetProgramDir (szBuffer, ichMaxFile);
	SzSzAppend (szBuffer, szFile);

#ifdef DOPENFILE
	CommSzSz (SzShared("trying to open: "), szBuffer);
#endif /* DOPENFILE */

	h = OpenFile ((LPSTR)szBuffer, (OFSTRUCT far *) pofstruct, of);

#ifdef DOPENFILE
	if (h != -1)
		CommSz (SzShared("Succedded\r\n"));
#endif /* DOPENFILE */

	return h;
}


