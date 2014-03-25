/*---------------------------------------------------------------------------
|                                                                           |
|   External Copy Routines -- Encryption routines                           |
|                                                                           |
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
|   Modification history:                                                   |
|   04/23/86    Created (edj)                                               |
|   05/26/86    Stolen from MP sources (jktg)                               |
|   06/02/87    Why not? (dc)                                               |
|   10/07/87    ported/munged to Opus (dsb)
---------------------------------------------------------------------------*/

/* encrypt  This module contains routines used to encrypt   */
/*      and de-crypt protected files. For details of    */
/*      the MP and 123 encryption schemea see the   */
/*      password document.              */

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NONCMESSAGES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW

#define OEMRESOURCE
#define NOSYSMETRICS
#define NOBITMAP
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOFONT
#define NOGDI
#define NOHDC
#define NOMB
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOPEN
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
#include "debug.h"
#include "file.h"
#include "ff.h"
#include "filecvt.h"
#include "error.h"

#include "idd.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "sdmparse.h"

#include "password.hs"
#include "password.sdm"



extern FF 	*vpff;
extern STM 	stmGlobal;

/* -------------------- special MP defines -------------------------------- */

#define cchMaxPub   20      /* Maximum number of characters
for "PUBLIC" name (i.e. for foreign versions) */
/* ibKey is the offset of the first of the two encryption words in the
	binary file. We use these words to tell if a file is encrypted and (if
	it is) how to de-crypt it. */
#define ibKey   (11*2)

/* -------------------- general MP defines -------------------------------- */


#define RotateLeft1(b) (char)( ((b & 0x7f) << 1) | (b >> 7) )
#define RotateLeft3(b) (char)( ((b & 0x1f) << 3) | (b >> 5) )
#define RotateRight1(b) (char)( ((b & 1) << 7) | (b >> 1) )
#define RotateRight3(b) (char)( ((b & 7) << 5) | (b >> 3) )

/* Increment index into rgbEncrypt, looping at end. */
#define IncIbEnc(ib) ib = ((ib + 1) & IBENCMASK )

/* Globals */
SEQEREF seqerefPublic;

/* Array used to encrypt data in protected files */
char rgbEncrypt[CBENCMAX];

/* End Globals */

/****************************************************************************
*
*  These routines handle checking if a file is protected and
*  encoding/decoding the password.
*
****************************************************************************/

/* Check if a MP binary file or Lotus Wk1 file is encrypted.
	Return fTrue if it is, fFalse otherwise. */

/* %%Function:FIsFileProtected %%Owner:davidbo */
int FIsFileProtected(dff)
int dff;
{
	unsigned *puns;
	unsigned cb, rt;
	char    rgb[512];

	switch (dff)
		{
	default:
		break;

	case dffOpenMp:
		SetStmPos(vpff->fnRead, (FC) ibKey);
		vpff->wKey1 = WFromVfcStm();
		vpff->wKey2 = WFromVfcStm();
		vpff->fHavePassword = fFalse;  /* We don't have the password */
		return(vpff->wKey1 != 0);

	case dffOpenWks:
	case dffOpenBiff:
		/* read in beginning of file record */
		SetStmPos(vpff->fnRead, (FC) 0);
		rt = WFromVfcStm();
		cb = WFromVfcStm();
		Assert(cb <= 512);
		RgbFromVfcStm(rgb, cb);

		/* read in password protection record   */
		rt = WFromVfcStm();
		cb = WFromVfcStm();
		if (cb > 512)
			{
			/* skip record 'cuz we don't read big ones anyway! */
			SetStmPos(vpff->fnRead, stmGlobal.fc + cb);
			return fFalse;
			}
		RgbFromVfcStm(rgb, cb);

		vpff->wKey2 = *(int *)(rgb);
		vpff->wKey1 = 0;

		if (dff == dffOpenWks)
			{
			if (rt == wksPASSWORD)
				return fTrue;
			}
		else  if (rt == rtFilePass)
			{
			Assert (dff == dffOpenBiff);
			return fTrue;
			}
		}
	return fFalse;
}


/* Prompt user for password to a protected file and check if it is correct.
	WKey1 == 0 indicates that we are checking a wks format file. MdDisp
	indicates the type of prompt to display. If mdprompt indicates
	that we should display the filename, then rgchFname contains the filename.

	This routine returns fTrue if the file name is correct, fFalse is it is
	not or the user aborts. If the user enters an incorrect filename
	an error message is displayed. If the password is correct, then
	rgbEncrypt is set up */

/* %%Function:FCheckPassword %%Owner:davidbo */
int FCheckPassword(dff)
int dff;
{
	char rgchPassword[cchPassMax];
	char rgchPasswordT[cchPassMax];

	if (vpff->szPassword[0] != 0)     /* Try previous password first */
		{
		bltb(vpff->szPassword, rgchPassword, cchPassMax);
		if (dff == dffOpenMp)
			AlterPassword(vpff->wKey1&0xf, rgchPassword);
		if (vpff->wKey2 == WGetPasswordKey(rgchPassword))
			goto LDone;
		}

LGetPassword:
	if (!FFilePswdDialog(rgchPassword, cchPassMax))
		return fFalse;

	bltb(rgchPassword, rgchPasswordT, cchPassMax);
	if (dff == dffOpenMp)
		AlterPassword((vpff->wKey1&0xf), rgchPassword);
	if (vpff->wKey2 != WGetPasswordKey(rgchPassword))
		{
LBadPassword:
		ErrorEid(eidBogusPassword, "");
		return fFalse;
		}
	bltb(rgchPasswordT, vpff->szPassword, cchPassMax);
LDone:
	SetRgbEncrypt(rgchPassword, (char)(vpff->wKey2&0xff), (char)(vpff->wKey2>>8));
	vpff->fHavePassword = fTrue;
	return fTrue;
}


/* %%Function:FFilePswdDialog %%Owner:davidbo */
FFilePswdDialog(szPswd, cchMax)
CHAR *szPswd;
int cchMax;
{
	CMB cmb;
	int tmc;
	char dlt [sizeof (dltFilePswd)];
	BOOL fRet = fFalse;

	*szPswd = 0;
	BltDlt(dltFilePswd, dlt);

	if ((cmb.hcab = HcabAlloc(cabiCABFILEPSWD)) == hNil)
		return fFalse;

	if (!FSetCabSz(cmb.hcab, szPswd, Iag(CABFILEPSWD, hszPswd)))
		goto LRet;

	cmb.cmm = cmmNormal;
	cmb.pv = NULL;
	cmb.bcm = bcmNil;

	if (TmcOurDoDlg(dlt, &cmb) == tmcOK)
		{
		GetCabSz(cmb.hcab, szPswd, cchMax, Iag(CABFILEPSWD, hszPswd));
		fRet = fTrue;
		}

LRet:
	FreeCab(cmb.hcab);
	return fRet;
}


/* %%Function:FDlgFilePswd %%Owner:davidbo */
BOOL FDlgFilePswd(dlm, tmc, wNew, wOld, wParam)
DLM dlm;
TMC tmc;
WORD wOld, wNew, wParam;
{
	if (dlm == dlmChange || dlm == dlmInit)
		GrayButtonOnBlank(tmcPswd, tmcOK);
	return fTrue;
}


/* Alter the password based upon a random key. This will make it harder
	to break the encryption scheme. This is a good candidate for
	hand coding to get the index array into the code segment.
	Note: this routine assumes that the low four bits of wKey are between
	0 and 14 (i.e. last hex digit is not 0xf)

	Warning: the functionality of this routine, and the static (csconst) data
	must NEVER CHANGE. If they do users will be unable to read their
	old protected files. */

/* %%Function:AlterPassword %%Owner:davidbo */
AlterPassword(wKey,szPassword)
unsigned wKey;
char *szPassword;
{
	csconst char rgchPad[15] =
				{
		'\012','N','Q','o','n','a','p','2','3','q','[','0','#','z'		};
	csconst int rgwIch[15] = 
		{
		9,4,1,3,14,11,6,0,12,7,2,10,8,13,5		};
	char rgchTemp[cchPassMax];
	int i, ich, cchPassword;

	cchPassword = CchSz(szPassword)-1;
	/* check password length    */
	Assert(cchPassword < cchPassMax && cchPassword > 0);
	/* check password key       */
	Assert((wKey&0xf) != 0xf);

	bltb(szPassword, rgchTemp, cchPassword);
	/* Pad out password with ascii data */
	bltbx((char far *)rgchPad,(char far *)rgchTemp+cchPassword, 15-cchPassword);

	/* Change one of the characters. This will prevent passwords
		that are all the same character from generating the same
		key on repeated saves. */
	rgchTemp[wKey]++;

	/* Permute the order of the characters in the password */
	for (i=0; i < 15; i++)
		{
		*szPassword++ = rgchTemp[rgwIch[wKey]];
		wKey = (wKey == 14 ? 0 : wKey+1);
		}
	*szPassword = 0;
}


/* Initialize the encryption array and index, using the keys provided.
	WARNING: the functionality of this routine must NEVER CHANGE. */

/* %%Function:SetRgbEncrypt %%Owner:davidbo */
SetRgbEncrypt(pchPassword, Key1, Key2)
char *pchPassword;
char Key1,Key2;
{
	char b;
	csconst char rgbPad[15] = 
		{
		0xbb,0xff,0xff,0xba,0xff,0xff,0xb9,0x80,0x00,0xbe,0x0f,0,0xbf,0x0f,0		};
	int cch,ib;

	cch = CchSz(pchPassword)-1;
	/* check password length    */
	Assert(cch < cchPassMax && cch >0);

	/* Build record from a copy of the password */
	bltb(pchPassword, rgbEncrypt, cch);
	bltbx((char far *)rgbPad, (char far *)rgbEncrypt+cch, cchPassMax-cch);

	Assert(!(cchPassMax&1));
	for (ib = 0; ib < cchPassMax ; )
		{
		b = rgbEncrypt[ib]^Key1;
		rgbEncrypt[ib++] = RotateRight1(b);
		b = rgbEncrypt[ib]^Key2;
		rgbEncrypt[ib++] = RotateRight1(b);
		}
	vpff->ibEncrypt = 0;
}


/* %%Function:WGetPasswordKey %%Owner:davidbo */
NATIVE WGetPasswordKey(pchPassword)
char *pchPassword;
{
	/* Encryption Matricies */
	csconst int rgwInitialCodes[15] =
				{ 
		0xe1f0,0x1d0f,0xcc9c,0x84c0,0x110c,
				0x0e10,0xf1ce,0x313e,0x1872,0xe139,
				0xd40f,0x84f9,0x280c,0xa96a,0x4ec3 		};

	csconst int rgrgwMatrix[15][7] =
				{
		0xaefc,0x4dd9,0x9bb2,0x2745,0x4e8a,0x9d14,0x2a09,
				0x7b61,0xf6c2,0xfda5,0xeb6b,0xc6f7,0x9dcf,0x2bbf,
				0x4563,0x8ac6,0x05ad,0x0b5a,0x16b4,0x2d68,0x5ad0,
				0x0375,0x06ea,0x0dd4,0x1ba8,0x3750,0x6ea0,0xdd40,
				0xd849,0xa0b3,0x5147,0xa28e,0x553d,0xaa7a,0x44d5,
				0x6f45,0xde8a,0xad35,0x4a4b,0x9496,0x390d,0x721a,
				0xeb23,0xc667,0x9cef,0x29ff,0x53fe,0xa7fc,0x5fd9,
				0x47d3,0x8fa6,0x0f6d,0x1eda,0x3db4,0x7b68,0xf6d0,
				0xb861,0x60e3,0xc1c6,0x93ad,0x377b,0x6ef6,0xddec,
				0x45a0,0x8b40,0x06a1,0x0d42,0x1a84,0x3508,0x6a10,
				0xaa51,0x4483,0x8906,0x022d,0x045a,0x08b4,0x1168,
				0x76b4,0xed68,0xcaf1,0x85c3,0x1ba7,0x374e,0x6e9c,
				0x3730,0x6e60,0xdcc0,0xa9a1,0x4363,0x86c6,0x1dad,
				0x3331,0x6662,0xccc4,0x89a9,0x0373,0x06e6,0x0dcc,
				0x1021,0x2042,0x4084,0x8108,0x1231,0x2462,0x48c4
		};

	char ch;
	int cchPassword, wCode;
	int far *qwMat;

	cchPassword = CchSz(pchPassword) - 1;
	/* check password length    */
	Assert(cchPassword < cchPassMax && cchPassword > 0);

	wCode = rgwInitialCodes[cchPassword - 1];
	qwMat = &rgrgwMatrix[cchPassMax - 1 - cchPassword][0];

	while ((ch = *pchPassword++) != 0)
		{
		if (ch & 0x1)
			wCode^=*qwMat;
		qwMat++;
		if (ch & 0x2)
			wCode^=*qwMat;
		qwMat++;
		if (ch & 0x4)
			wCode^=*qwMat;
		qwMat++;
		if (ch & 0x8)
			wCode^=*qwMat;
		qwMat++;
		if (ch & 0x10)
			wCode^=*qwMat;
		qwMat++;
		if (ch & 0x20)
			wCode^=*qwMat;
		qwMat++;
		if (ch & 0x40)
			wCode^=*qwMat;
		qwMat++;
		}

	return(wCode);
}


/****************************************************************************
*
*  These routines handle stuff related to Public areas on
*  protected sheets.
*
****************************************************************************/

/* Check if there is a public area defined on the present sheet. If there
	is no such area, return fFalse. If there is, mark the public entries
	and return fTrue. */

/* Checks an open binary file to see if there are any public areas defined.
	If a public area is defined, read the referneces into seqerefPublic */

/* %%Function:FCheckExtForPublic %%Owner:davidbo */
int FCheckExtForPublic()
{
	int fFirst, cch, cchPublic;
	char rgchT[sizeof(LBL20)];
	char szPublic[cchMaxPub+1];
	int rgw[cwMPGlobals];

	cchPublic = CchCopySz(SzFrame("PUBLIC"), szPublic);
	Assert(cchPublic < cchMaxPub);

	if (CwGetGlobalsMP20(rgw))
		return fFalse;

	for (fFirst=fTrue; seqerefPublic.ierefMac=
			CrefNextNameMP20(seqerefPublic.rgeref,20,&cch,rgchT,sizeof(LBL20),fFirst,rgw)
			; fFirst = fFalse)
		{
		rgchT[cch] = '\0';
		if (cch == cchPublic && FEqNcRgch(szPublic, rgchT, cch))
			return(fTrue);
		}

	return(fFalse);
}


/* Checks if a given cell is in the public area or not */

/* %%Function:FIsPublic %%Owner:davidbo */
int FIsPublic(rw,col)
int rw,col;
{
	EREF *peref,*perefMac;

	peref = seqerefPublic.rgeref;
	perefMac = &seqerefPublic.rgeref[seqerefPublic.ierefMac];

	for (; peref < perefMac ; peref++)
		{
		if ( (peref->row0 <= rw) && (rw <= peref->row1) &&
				(peref->col0 <= col) && (col <= peref->col1) )
			{
			return(fTrue);
			}
		}
	return(fFalse);
}


/****************************************************************************
*
*  These routines do the actual encryption and decryption of data.
*
****************************************************************************/


/* Decrypt cb bytes of an individual record. If bent == 0 then ibEncrypt
	is set appropriately for the decryption, otherwise it must be initialized
	using the bent. */

/* %%Function:DecryptHDS %%Owner:davidbo */
DecryptHDS(qbHDS, cb, bent)
char far *qbHDS;
unsigned cb;
unsigned bent;
{
	char b;

	if (bent != 0)
		vpff->ibEncrypt = bent & IBENCMASK;

	while (cb-- > 0)
		{
		b = (*qbHDS) ^ rgbEncrypt[vpff->ibEncrypt];
		*qbHDS++ = b;
		IncIbEnc(vpff->ibEncrypt);
		}
}


/*****************************************************************/
/*                                                              */
/*  These routines are specific to encrypting and decrypting    */
/*  Lotus 123 V2 files.                                         */
/*                                                              */
/*****************************************************************/


/* Decode a record. */
/* %%Function:DecryptWrt %%Owner:davidbo */
DecryptWrt(pbWrt, cbWrt)
char *pbWrt;
unsigned cbWrt;
{
	char b;
	while (cbWrt-- > 0)
		{
		b = *pbWrt;
		b ^= rgbEncrypt[vpff->ibEncrypt];
		*pbWrt++ = RotateLeft3(b);
		IncIbEnc(vpff->ibEncrypt);
		}
}


