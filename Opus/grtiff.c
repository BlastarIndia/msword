/*------------------------------------------------------------------------
*
*
* Microsoft Word -- Graphics
*
* TIFF bitmap reading code
*
*-----------------------------------------------------------------------*/


#ifdef PCWORD

#include "word.h"
#include "file.h"
#include "stream.h"
#include "strmsg.h"
#include "gr.h"
#include "grconsts.h"
#include "grstruct.h"
#include "grtiff.h"
#include "grps.h"
#include "hcdefs1.h"

AssertData

#define BFromTIFF()	(FastBFromStm())
#define FEof()			(StmGlobalFc() >= stmGlobal.fcMac)
#define StmSetPosTIFF(fc)	StmSetPos(fc + ppc->tiff.fcHeader)
#define FLowRes()	(vlm == lmPreview)
#endif /* PCWORD */

#ifdef WIN

#include "word.h"
DEBUGASSERTSZ		 /* WIN - bogus macro for assert string */
#include "file.h"
#include "ff.h"
#include "grstruct.h"
#include "grtiff.h"
#include "pic.h"
#include "heap.h"
#include "debug.h"

#undef Error	/* used as a label here */

#define StmSetPosTIFF(fc)	StmSetTIFF(fc)
#define StmSetPos(fc)	    StmSetTIFF(fc)
#define FLowRes()	(vlm != lmPrint)
#endif /* WIN */


/* Array of default densities to use for dithering */
/* Uses an almost linear scale. */
csconst unsigned rgdns[16] = {
	0,  150,  178,  207,  239,  273,  311,  351,
			396,  447,  503,  569,  646,  739,  858, 2000
};


/*  %%Function:  GetInfoTIFF  %%Owner:  bobz       */

GetInfoTIFF(hpc, rgco)
PICT		**hpc;
unsigned	rgco[];
	/*
	* Fill in rgco with the bitmap's palette or gray scale.
	* Also, fill in vtExt, cplIn, and fGrayDither in **hpc.
	*/
{
	PICT		*ppc;
	DEN			den;
	FC			fcSave;
	int			ide, cde;
	int			ico;
	int			cStrips;
	unsigned	wGrayNum;
	int			wGrayUnit;
	int			wT;
	int			wPhotoInt = 0;
	int			fGrayProvided = false;
	int			wMagicTIFF;

	ppc = *hpc;

	StmSetPos((FC)0);
	wMagicTIFF = WFromStm();
#ifdef PCWORD
	if (wMagicTIFF == wMagicEPS1)
		{
		StmSetPos((FC)&((EPS *)0)->fcTIFF);
		LFromTIFF(&ppc->tiff.fcHeader, fFalse);
		StmSetPos(ppc->tiff.fcHeader);
		wMagicTIFF = WFromStm();
		}
	else
		{
		ppc->tiff.fcHeader = fc0;
		}
#endif

	ppc->tiff.fSwitchBytes = (wMagicTIFF == 0x4d4d);
	ppc->fGrayDither = false;
	ppc->tiff.cbitsSample = 1;
	ppc->cplIn = 1;
	ppc->tiff.wCompScheme = 1;
	ppc->tiff.crwStrip = crwMac;
	wGrayUnit = wGrayUnitDef;

	Assert((*hpc)->fn == stmGlobal.fn);
#ifdef PCWORD
	GetPifdTIFF(&fcSave, ppc->tiff.fSwitchBytes, ppc->tiff.fcHeader);
#else
	GetPifdTIFF(&fcSave, ppc->tiff.fSwitchBytes);
#endif
	StmSetPosTIFF(fcSave);

	cde = WFromTIFF(ppc->tiff.fSwitchBytes);

	for (ide=0; ide<cde; ide++)
		{

		DenFromTIFF(&den, ppc->tiff.fSwitchBytes);

		switch (den.wTag)
			{
		default:				/* ignore de */
			break;
		case BitsPerSample:
			if (den.w1Value != 1 && den.w1Value != 4 && den.w1Value !=8)
				goto Error;
			ppc->tiff.cbitsSample = den.w1Value;
			if (ppc->tiff.cbitsSample == 1)
				ppc->cplIn = 1;
			else
				ppc->cplIn = 4;
			break;
		case Compression:
			if (den.w1Value != Scheme1 && den.w1Value != Scheme32773)
				goto Error;
			ppc->tiff.wCompScheme = den.w1Value;
			break;
		case GrayResponseUnit:
			wGrayUnit = den.w1Value;
			break;
		case GrayResponseCurve:
				{
				int wOffset, iw;
				int rgw[256];

				fGrayProvided = true;
				ppc->fGrayDither = true;
				/* set wGrayNum/wGrayDen to scale to ten-thousandths */
				wGrayNum = 1;
				for (wT = wGrayUnit; wT++ < wGrayUnitLim;)
					wGrayNum *= 10;
				fcSave = StmGlobalFc();
				StmSetPosTIFF(den.lpValue);
				RgbFromStm(HpOfSbIb(sbDds, rgw), ppc->tiff.cbitsSample == 4 ? 32 : 512);
				wOffset = (ppc->tiff.cbitsSample == 4 ? 1 : 16);
				for (iw=0,ico=0; ico<16; ico++,iw+=wOffset)
					{
					if (ppc->tiff.fSwitchBytes)
						SwitchBytes(&rgw[iw], 1);
					rgco[ico] = MultDivU(rgw[iw],wGrayNum, wGrayDen);
					}
				StmSetPos(fcSave);
				break;
				}
		case ImageLength:
			if ((den.wType != typeShort && den.w2Value !=0) || 
					(den.wType == typeShort && den.w1Value > crwMac))
				goto Error;
			ppc->vtExt.y = den.w1Value;
			break;
		case ImageWidth:
			if ((den.wType != typeShort && den.w2Value !=0) || 
					(den.wType == typeShort && den.w1Value > crwMac))
				goto Error;
			ppc->vtExt.x = den.w1Value;
			break;
		case PhotometricInterpretation:
			if (den.w1Value != 0 && den.w1Value != 1)
				goto Error;
			wPhotoInt = den.w1Value;
			break;
		case RowsPerStrip:
			/* if 2**32-1, leave crwstrip at defaulted crwMac */
			if (den.wType == typeLong && den.lValue == 0xFFFFFFFF)
				break;
			if ((den.wType != typeShort && den.w2Value !=0) || 
					(den.wType == typeShort && den.w1Value > crwMac))
				goto Error;
			ppc->tiff.crwStrip = den.w1Value;
			break;
		case SamplesPerPixel:		/* only support 1 sample per pixel */
			if (den.w1Value != 1)
				goto Error;
			break;
		case StripByteCountMax: 	/* ignore for now */
			break;
		case StripByteCounts:		/* ignore for now */
			break;
		case StripOffsets:
			ppc->tiff.fShortStripOffset = (den.wType == typeShort);
			cStrips = den.cValues;
			if (ppc->tiff.fShortStripOffset)
				{
				if (cStrips == 2)
					goto Error;
				ppc->tiff.fcStripOffsets = (FC) den.w1Value;
				}
			else
				ppc->tiff.fcStripOffsets = den.lValue;
			break;
		case TiffClass: 			/* ignore for now */
			switch (den.w1Value)
				{
			default:			/* class B or G: supported */
				break;
			case 3:				/* class P: unsupported */
			case 4:				/* class R: unsupported */
			case 5:				/* class S: unsupported */
				goto Error;
				}
			break;
			}
		}

	if (ppc->tiff.cbitsSample != 1 && !fGrayProvided)
		{
		ppc->fGrayDither = true;
		bltbx((char far *)rgdns, (char far *)rgco, sizeof(rgdns));
		}
	if (wPhotoInt == 0)
		{
		if (ppc->tiff.cbitsSample == 1)
			{
			rgco[0] = coWhiteGR;
			rgco[1] = coBlack;
			}
		}
	else
		{
		if (ppc->tiff.cbitsSample == 1)
			{
			rgco[0] = coBlack;
			rgco[1] = coWhiteGR;
			}
		else  if (!fGrayProvided)
			{
			int iFirst, iLast;
			for (iFirst=0,iLast=15; iFirst<iLast; iFirst++,iLast--)
				{
				wT = rgco[iLast];
				rgco[iLast] = rgco[iFirst];
				rgco[iFirst] = wT;
				}
			}
		}

	StmSetPosTIFF(ppc->tiff.fcStripOffsets);
	if (cStrips != 1)
		{
		if (ppc->tiff.fShortStripOffset)
			fcSave = (FC)WFromTIFF(ppc->tiff.fSwitchBytes);
		else
			LFromTIFF(&fcSave,ppc->tiff.fSwitchBytes);
		StmSetPosTIFF(fcSave);
		}
	ppc->tiff.irwNext = 0;
	ppc->tiff.fcPrev = (FC)0;		/* an invalid FC for TIFF files */
	ppc->tiff.fcTrail = (FC)0;		/* ditto */
	ppc->tiff.irwTrail = unsMax;	/* an invalid row # */
	/* bytes to read from file per row. byte aligned which tiff wants */
	ppc->tiff.cbRowFile = (ppc->vtExt.h + cbitByte - 1) / cbitByte;

	return;
Error:
	OutSz(SzShared("Error with tag:")); 
	OutInt(den.wTag,5); 
	OutCh('\n');
	GraphicsError(hpc, IDMSGBadGrFile);
	return;
}


/*  %%Function:  DenFromTIFF  %%Owner:  bobz       */

DenFromTIFF(pden, fSwitchBytes)
DEN *pden;
BOOL fSwitchBytes;
{
	RgbFromStm(HpOfSbIb(sbDds, pden), sizeof(DEN));
	if (fSwitchBytes)
		{
		SwitchBytes((char *)pden,2);
		SwitchLong(((char *)pden)+4);
		if (pden->wType == typeShort)
			SwitchBytes(((char *)pden)+8,1);
		else
			SwitchLong(((char *)pden)+8);
		}
}


/*  %%Function:  WFromTIFF  %%Owner:  bobz       */

WFromTIFF(fSwitchBytes)
BOOL fSwitchBytes;
{
	if (!fSwitchBytes)
		return(WFromStm());
	else
		return(WSwappedFromStm());
}


/*  %%Function:  LFromTIFF  %%Owner:  bobz       */

LFromTIFF(pl,fSwitchBytes)
long *pl;
BOOL fSwitchBytes;
{
	RgbFromStm(HpOfSbIb(sbDds, pl), sizeof(long));
	if (fSwitchBytes)
		SwitchLong((char *)pl);
	return;
}


/*  %%Function:  SwitchBytes  %%Owner:  bobz       */

SwitchBytes(pb, cw)
char *pb;
int cw;
{
	int bT;

	while (cw--)
		{
		bT = *pb;
		*pb = *(pb+1);
		*(pb+1) = bT;
		pb += 2;
		}
}


/*  %%Function:  SwitchLong  %%Owner:  bobz       */

SwitchLong(pb)
char *pb;
{
	int bT;

	bT = *pb;
	*pb = *(pb+3);
	*(pb+3) = bT;

	bT = *(pb+1);
	*(pb+1) = *(pb+2);
	*(pb+2) = bT;
}


/*  %%Function:  ReadTIFF  %%Owner:  bobz       */

ReadTIFF(hpc)
PICT		**hpc;
/*
* Warning:	some of the FC's here are relative to the start of the
*			physical disk file, but some are relative to the start
*			of the logical TIFF file, which may be embedded in
*			an EPS file.  fcPrev and fcTrail are physical FC's.
*			fcSave is sometimes physical and sometimes logical.
*/
{
	PICT		*ppc;
	unsigned	cbRowIn,
	irwInMic,
	irwInMac,
	irw,
	irwStrip,
	istrip;
	char HUGE	*hpPlane;
	char HUGE	*hpSave;
	FC			fcSave;


	Assert((*hpc)->fn == stmGlobal.fn);

	ppc = *hpc;
	FreezeHp();
	cbRowIn = ppc->cbRowIn;
	irwInMic = ppc->irwInMic;
	irwInMac = ppc->irwInMac;
	hpPlane = HpOfSbIb(ppc->sbBits, ppc->ibIn);

	if (ppc->tiff.cbitsSample != 1)
		{
		int ipl;
		char far * lpPlane;
		char far * lpSave;

		lpPlane = LpLockHp(hpPlane);
		for (ipl=0; ipl<4; ipl++)
			{
			lpSave = lpPlane;
			for (irw = irwInMic; irw < irwInMac; irw++)
				{
				bltbcx(0, lpPlane, cbRowIn);
				lpPlane += cbRowIn;
				}
			lpPlane = ppc->cbPlane + lpSave;
			}
		UnlockHp(hpPlane);
		}

	/* back up some if necessary */
	if (irwInMic < ppc->tiff.irwNext)
		{
		if (ppc->irwInMic == ppc->tiff.irwNext - 1)
			{
			Assert(ppc->tiff.fcPrev != (FC)0);
			StmSetPos(ppc->tiff.fcPrev);
			--ppc->tiff.irwNext;
			}
		else
			{
			Assert(irwInMic >= ppc->tiff.irwTrail && ppc->tiff.fcTrail != (FC)0);
			StmSetPos(ppc->tiff.fcTrail);
			ppc->tiff.irwNext = ppc->tiff.irwTrail;
			}
		}

	/* skip over some lines if necessary */
	istrip = UDiv(ppc->tiff.irwNext, ppc->tiff.crwStrip, &irwStrip);
	for (;ppc->tiff.irwNext < irwInMic; ++ppc->tiff.irwNext, irwStrip++)
		{
		if (irwStrip >= ppc->tiff.crwStrip)
			{
			istrip = UDiv(ppc->tiff.irwNext, ppc->tiff.crwStrip, &irwStrip);
			fcSave = istrip * (ppc->tiff.fShortStripOffset ? sizeof(int) : sizeof(long));
			StmSetPosTIFF(ppc->tiff.fcStripOffsets + fcSave);
			if (ppc->tiff.fShortStripOffset)
				fcSave = (FC)WFromTIFF(ppc->tiff.fSwitchBytes);
			else
				LFromTIFF(&fcSave,ppc->tiff.fSwitchBytes);
			StmSetPosTIFF(fcSave);
			}
		if (FEof())					/* FIX -- print error message */
			{
			MeltHp();
			GraphicsError(hpc, IDMSGBadGrFile);
			}
		Scribble(ispGraphics, 'r');
		ReadTIFFLine(hpc, hpPlane);
		Scribble(ispGraphics, ' ');
		}

	ppc->tiff.irwTrail = irwInMic;
	ppc->tiff.fcTrail = StmGlobalFc();

	for (irw = irwInMic; irw < irwInMac; ++irw, irwStrip++)
		{
		if (irwStrip >= ppc->tiff.crwStrip)
			{
			istrip = UDiv(irw, ppc->tiff.crwStrip, &irwStrip);
			fcSave = istrip * (ppc->tiff.fShortStripOffset ? sizeof(int) : sizeof(long));
			StmSetPosTIFF(ppc->tiff.fcStripOffsets + fcSave);
			if (ppc->tiff.fShortStripOffset)
				fcSave = (FC)WFromTIFF(ppc->tiff.fSwitchBytes);
			else
				LFromTIFF(&fcSave,ppc->tiff.fSwitchBytes);
			StmSetPosTIFF(fcSave);
			}
		/* must do this, so fcSave is a physical (not logical) FC */
		fcSave = StmGlobalFc();
		hpSave = hpPlane;
		if (((irw - irwInMic) & 31) == 0)
			{
			MeltHp();
			CheckGrAbort();  /* for WIN this can move heap */
			ppc = *hpc;
			FreezeHp();
			}
		if (fcSave >= stmGlobal.fcMac)
			{
			MeltHp();
			GraphicsError(hpc, IDMSGBadGrFile);
			}
		Scribble(ispGraphics, 'r');
		ReadTIFFLine(hpc, hpPlane);
		Scribble(ispGraphics, ' ');
		hpPlane = hpSave + cbRowIn;
		}

	ppc->tiff.irwNext = irwInMac;
	ppc->tiff.fcPrev = fcSave;

	MeltHp();
	return;
}


#ifndef HCREADTIFFLINE

csconst BYTE mpnbb[64] = {
	0x00, 0x00, 0x00, 0x00, 
			0xff, 0x00, 0x00, 0x00, 
			0x00, 0xff, 0x00, 0x00, 
			0xff, 0xff, 0x00, 0x00, 
			0x00, 0x00, 0xff, 0x00, 
			0xff, 0x00, 0xff, 0x00, 
			0x00, 0xff, 0xff, 0x00, 
			0xff, 0xff, 0xff, 0x00, 
			0x00, 0x00, 0x00, 0xff, 
			0xff, 0x00, 0x00, 0xff, 
			0x00, 0xff, 0x00, 0xff, 
			0xff, 0xff, 0x00, 0xff, 
			0x00, 0x00, 0xff, 0xff, 
			0xff, 0x00, 0xff, 0xff, 
			0x00, 0xff, 0xff, 0xff, 
			0xff, 0xff, 0xff, 0xff
};


/* Sign-extend character to integer     */
/*  %%Function:  sxt   %%Owner:  bobz       */

int sxt (ch)
int     ch;
{
	return(ch - ((ch & 0x80) << 1));
}


/*  %%Function:  ReadTIFFLine  %%Owner:  bobz       */


EXPORT
ReadTIFFLine(hpc, hpPlane)
PICT		**hpc;
char HUGE	*hpPlane;
	/*
		* Read in a line from the file.  The line to read from is at offset
		* (*hpc)->fcCur in the file.
		*/
{
	int			ibitIn,
	ibIn,
	cbRowFile, /* bytes/row to read */
	cbitsSample;
	BYTE		b;
	unsigned	cb;

	ibitIn = ibIn = 0;
	cbRowFile = (*hpc)->tiff.cbRowFile;
	cbitsSample = (*hpc)->tiff.cbitsSample;

	if ((*hpc)->tiff.wCompScheme == Scheme1)
		{ /* no compression */
		if (cbitsSample == 1)
			{
			RgbFromStm(hpPlane, cbRowFile);
			}
		else
			{
			char HUGE * hpSave;
			int		ipl;
			unsigned	bSpecial = 0x80;
			unsigned	nb;
			int			fFirstNibble = true;
			unsigned	cbPlane = (*hpc)->cbPlane;

			while (ibitIn < (*hpc)->vtExt.h)
				{
				if (fFirstNibble)
					{
					b = BFromStm();
					nb = (b & 0xf0) >> 2;
					if (cbitsSample == 4)
						fFirstNibble = false;
					}
				else
					{
					nb = (b & 0x0f) << 2;
					fFirstNibble = true;
					}
				hpSave = hpPlane;
				for (ipl=0; ipl<4; ipl++)
					{
					*hpPlane |= (mpnbb[nb++] & bSpecial);
					hpPlane += cbPlane;
					}
				hpPlane = hpSave;
				bSpecial >>= 1;
				if (bSpecial == 0)
					{
					bSpecial = 0x80;
					hpPlane++;
					}
				ibitIn++;
				}
			}
		}
	else
		{ /* 32773 Macintosh PackBits compression */
		int w;
		Assert((*hpc)->tiff.wCompScheme == Scheme32773);
		while (ibIn < cbRowFile)
			{
			w = sxt(BFromTIFF());
			if (w < 0)
				{
				cb = -w+1;
				b = BFromTIFF();
				bltbcx(b, LpFromHp(hpPlane), cb);
				hpPlane += cb;
				ibIn += cb;
				}
			else
				{
				Assert (w >= 0 && w <= 127);
				cb = w+1;
				RgbFromStm(hpPlane, cb);
				hpPlane += cb;
				ibIn += cb;
				}
			}
		}
}


#else	/* HCREADTIFFLINE */
#ifdef	STUBREADTIFFLINE

EXPORT

/*  %%Function:  ReadTIFFLine  %%Owner:  bobz       */

ReadTIFFLine(hpc, hpPlane) /* stub */
PICT		**hpc;
char HUGE	*hpPlane;
{
	ReadTIFFLineNat(hpc, hpPlane);
}


/*  %%Function:  GetPifdTIFF  %%Owner:  bobz       */

#endif	/* STUBREADTIFFLINE */
#endif	/* else HCREADTIFFLINE */

#ifdef PCWORD
GetPifdTIFF(pfcIfd,fSwitchBytes, fcHeader)
FC		fcHeader;
#else
GetPifdTIFF(pfcIfd,fSwitchBytes)
#endif
FC *	pfcIfd;
BOOL	fSwitchBytes;
{
	FC		lpfcIfd;
	FC		fcSave;
	FC		fcIfd;
	int		ide,
	cde;
	DEN		den;
	BOOL	fFound=false;

#undef StmSetPosTIFF
#ifdef PCWORD
#define StmSetPosTIFF(fc)	StmSetPos(fc + fcHeader)
#else
#define StmSetPosTIFF(fc)	StmSetPos(fc)
#endif

	lpfcIfd = (FC) ibTIFFIfd;
	StmSetPosTIFF(lpfcIfd);
	LFromTIFF(&fcIfd, fSwitchBytes);
	StmSetPosTIFF(fcIfd);

	fcSave = fcIfd;

	while (fcIfd != 0)
		{
		cde = WFromTIFF(fSwitchBytes);
		for (ide = 0; ide < cde; ide++)
			{
			DenFromTIFF(&den, fSwitchBytes);
			if (den.wTag > OldfileType)
				goto NextIfd;
			switch (den.wTag)
				{
			default:
				break;
			case OldfileType:
				/* reduced image for preview */
				if (den.w1Value == (FLowRes() ? 2 : 1))
					{
					fFound = true;
					goto Found;
					}
				else
					goto NextIfd;
			case SubfileType:
				/* reduced image for preview */
				if (den.lValue & 0x0f == (FLowRes() ? 1 : 0))
					{
					fFound = true;
					goto Found;
					}
				else
					goto NextIfd;
				}
			}
NextIfd:
		lpfcIfd = fcIfd + 2 + cde*12;
		StmSetPosTIFF(lpfcIfd);
		LFromTIFF(&fcIfd, fSwitchBytes);
		 /*  bobz handle bogus files that did not write out
			0's for next dir. If out of range, act as though
			0's were there.
		 */
	    if (fcIfd > stmGlobal.fcMac || fcIfd < (FC)0)
			fcIfd = (FC)0;
		StmSetPosTIFF(fcIfd);

		}

Found:
	if (!fFound)
		*pfcIfd = fcSave;
	else
		*pfcIfd = fcIfd;

}


#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Grtiff_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* Grtiff_Last() */
