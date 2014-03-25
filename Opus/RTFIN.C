#define NONCMESSAGES
#define NODRAWFRAME
#define NORASTEROPS
#define NOCTLMGR
#define NOMINMAX
#define NOMSG
#define NORECT
#define NOSCROLL
#define NOKEYSTATE
#define NOCREATESTRUCT
#define NOICON
#define NOPEN
#define NOREGION
#define NODRAWTEXT
/* #define NOMB */
#define NOWINOFFSETS
/* #define NOMETAFILE */
#define NOCLIPBOARD
#define NOSOUND
#define NOCOMM
#define NOKANJI
/* #define NOGDI */

#define FONTS
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "version.h"
#include "heap.h"
#include "doc.h"
#include "props.h"
#include "border.h"
#include "disp.h"
#include "debug.h"
#include "error.h"
#include "prm.h"
#include "sel.h"
#include "file.h"
#include "layout.h"
#include "ch.h"
#include "fontwin.h"

#define REVMARKING
#include "compare.h"

#include "inter.h" 
#include "pic.h"

#define RTFDEFS
#include "rtf.h"

#include "field.h"
#include "strtbl.h"
#define RTFTABLES
#define RTFIN
#include "rtftbl.h"
#include "style.h"
#include "fkp.h"
#include "prompt.h"
#include "message.h"
#include "filecvt.h"


extern struct CHP	vchpNormal;
extern struct PAP	vpapFetch;
extern struct CHP	vchpFetch;
extern int		fnFetch;
extern int              vdocPapLast;

extern struct CA	caPara;
extern struct CA	caSect;
extern struct MERR	vmerr;
extern CP		vcpFetch;
extern int		vccpFetch;
extern int		vfPCFlip;
extern struct PIC	vpicFetch;
extern char HUGE        *vhpchFetch;
extern struct DOD	**mpdochdod[];
extern struct SPX	mpiwspxSep[];
extern int		docGlobalDot;
extern int		rgftc[iftcMax];
extern int		viftcMac;
extern struct FCB	**mpfnhfcb[];
extern struct FKPD	vfkpdText;
extern struct SAB       vsab;
extern struct SEP	vsepFetch;
extern struct CHP	vchpStc;

extern int vstcBackup;
extern int              cfRTF;
extern int		vifnd;

#ifdef NOASM
char   ChMapSpecChar (char, int);
#endif /* NOASM */
long   	LFromSzNumber();
#ifdef NOASM
RtfIn();
#endif /* NOASM */

extern CHAR  rgchEop[];
extern CHAR  rgchTable[];

#ifdef DEBUG
extern struct DBS	vdbs;
extern int vfCheckPlc;  /* check plc.iMac references */
struct RIBL **vhribl;
#endif /* DEBUG */

int cGrStart = 0;
int ftcMapDef = ftcDefault;

#ifdef DEBUG
#ifdef PCJ
/* #define DRTFDTTM */
#endif /* PCJ */
#endif /* DEBUG */

csconst char rgchRTF[] = {
	'{', '\\','r','t','f'};


struct DTR dtrRTF;  /* date time record */

/* from sdm.h . Illegal return value from wFromszrtf */
#define	wError		(-32766)	// Ints. 

#ifdef DEBUG
csconst char mpchMACchANSI[128] =
{
	/* 128   129   130   131   132   133   134   135  */
	0xc4, 0xc5, 0xc7, 0xc9, 0xd1, 0xd6, 0xdc, 0xe1,
			/* 136   137   138   139   140   141   142   143  */
	0xe0, 0xe2, 0xe4, 0xe3, 0xe5, 0xe7, 0xe9, 0xe8,
			/* 144   145   146   147   148   149   150   151  */
	0xea, 0xe8, 0xed, 0xec, 0xee, 0xef, 0xf1, 0xf3,
			/* 152   153   154   155   156   157   158   159  */
	0xf2, 0xf4, 0xf6, 0xf5, 0xfa, 0xf9, 0xfb, 0xfc,
			/* 160   161   162   163   164   165   166   167  */
	0xa0, 0xb0, 0xa2, 0xa3, 0xa7, 0x6f, 0xb6, 0xdf,
			/* 168   169   170   171   172   173   174   175  */
	0xae, 0xa9, 0xaa, 0xb4, 0xa8, 0xad, 0xc6, 0xd8,
			/* 176   177   178   179   180   181   182   183  */
	0xb0, 0xb1, 0xb2, 0xb3, 0xa5, 0xb5, 0xb6, 0xb7,
			/* 184   185   186   187   188   189   190   191  */
	0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xc6, 0xd8,
			/* 192   193   194   195   196   197   198   199  */
	0xbf, 0xa1, 0xac, 0xc3, 0xc4, 0xc5, 0xc6, 0xab,
			/* 200   201   202   203   204   205   206   207  */
	0xbb, 0xc9, 0xa0, 0xc0, 0xc3, 0xd5, 0xce, 0xcf,
			/* 208   209   210   211   212   213   214   215  */
	/* note special printing char mapping in this line */
	0x97, 0x96, 0x93, 0x94, 0x91, 0x92, 0xd6, 0xd7,
			/* 216   217   218   219   220   221   222   223  */
	0xff, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
			/* 224   225   226   227   228   229   230   231  */
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
			/* 232   233   234   235   236   237   238   239  */
	0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
			/* 240   241   242   243   244   245   246   247  */
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
			/* 248   249   250   251   252   253   254   255  */
	0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};


csconst char mpchIBMchANSI[128] =
{
	/* 128   129   130   131   132   133   134   135  */
	0xc7, 0xfc, 0xe9, 0xe2, 0xe4, 0xe0, 0xe5, 0xe7, 
			/* 136   137   138   139   140   141   142   143  */
	0xea, 0xeb, 0xe8, 0xef, 0xee, 0xec, 0xc4, 0xc5, 
			/* 144   145   146   147   148   149   150   151  */
	0xc9, 0xe6, 0xc6, 0xf4, 0xf6, 0xf2, 0xfb, 0xf9, 
			/* 152   153   154   155   156   157   158   159  */
	0xff, 0xd6, 0xdc, 0xa2, 0xa3, 0xa5, 0x70, 0x66, 
			/* 160   161   162   163   164   165   166   167  */
	0xe1, 0xed, 0xf3, 0xfa, 0xf1, 0xd1, 0xaa, 0xba, 
			/* 168   169   170   171   172   173   174   175  */
	0xbf, 0x5f, 0xac, 0xbd, 0xbc, 0xa1, 0xab, 0xbb, 
			/* 176   177   178   179   180   181   182   183  */
	0x5f, 0x5f, 0x5f, 0xa6, 0x5f, 0x5f, 0x5f, 0x5f, 
			/* 184   185   186   187   188   189   190   191  */
	0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 
			/* 192   193   194   195   196   197   198   199  */
	0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 
			/* 200   201   202   203   204   205   206   207  */
	0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 
			/* 208   209   210   211   212   213   214   215  */
	0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 
			/* 216   217   218   219   220   221   222   223  */
	0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0xa6, 0x5f, 0x5f, 
			/* 224   225   226   227   228   229   230   231  */
	0x5f, 0xdf, 0x5f, 0xb6, 0x5f, 0x5f, 0xb5, 0x5f, 
			/* 232   233   234   235   236   237   238   239  */
	0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 
			/* 240   241   242   243   244   245   246   247  */
	0x5f, 0xb1, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 
			/* 248   249   250   251   252   253   254   255  */
	0xb0, 0x95, 0xb7, 0x5f, 0x6e, 0xb2, 0xa8, 0x5f 
};


csconst char mpchPCAchANSI[128] =
{
	/* 128   129   130   131   132   133   134   135  */
	0xc7, 0xfc, 0xe9, 0xe2, 0xe4, 0xe0, 0xe5, 0xe7, 
			/* 136   137   138   139   140   141   142   143  */
	0xea, 0xeb, 0xe8, 0xef, 0xee, 0xec, 0xc4, 0xc5, 
			/* 144   145   146   147   148   149   150   151  */
	0xc9, 0xe6, 0xc6, 0xf4, 0xf6, 0xf2, 0xfb, 0xf9,
			/* 152   153   154   155   156   157   158   159  */
	0xff, 0xd6, 0xdc, 0xf8, 0xa3, 0xd8, 0x5f, 0x66,
			/* 160   161   162   163   164   165   166   167  */
	0xe1, 0xed, 0xf3, 0xfa, 0xf1, 0xd1, 0xaa, 0xba,
			/* 168   169   170   171   172   173   174   175  */
	0xbf, 0xae, 0xac, 0xbd, 0xbc, 0xa1, 0xab, 0xbb,
			/* 176   177   178   179   180   181   182   183  */
	0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0xc1, 0xc2, 0xc0,
			/* 184   185   186   187   188   189   190   191  */
	0xa9, 0x5f, 0x5f, 0x5f, 0x5f, 0xa2, 0xa5, 0x5f,
			/* 192   193   194   195   196   197   198   199  */
	0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0xe3, 0xc3,
			/* 200   201   202   203   204   205   206   207  */
	0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0xa4,
			/* 208   209   210   211   212   213   214   215  */
	0xf0, 0xd0, 0xca, 0xcb, 0xc8, 0x5f, 0xcd, 0xce,
			/* 216   217   218   219   220   221   222   223  */
	0xcf, 0x5f, 0x5f, 0x5f, 0x5f, 0xab, 0xcc, 0x5f,
			/* 224   225   226   227   228   229   230   231  */
	0xd3, 0xdf, 0xd4, 0xd2, 0xf5, 0xd5, 0xb5, 0xde,
			/* 232   233   234   235   236   237   238   239  */
	0xfe, 0xda, 0xdb, 0xd9, 0xfd, 0xdd, 0xaf, 0xb4,
			/* 240   241   242   243   244   245   246   247  */
	0xad, 0xb1, 0x3d, 0xbe, 0xb6, 0xa7, 0x5f, 0xb8,
			/* 248   249   250   251   252   253   254   255  */
	0xb0, 0xa8, 0x95, 0xb9, 0xb3, 0xb2, 0xa8, 0x5f
};


#endif /* DEBUG */


#ifdef DEBUG
/*	R T F  I N

Inputs:
	hribl		RTF IN Block
	pch		RTF Text to input
	cch		Length of input text

Converts RTF text to internal form.
*/
/*  %%Function:  C_RtfIn  %%Owner:  bobz       */

HANDNATIVE C_RtfIn(hribl, pch, cch)
struct RIBL **hribl;
char *pch;
int cch;
{

	char *pchLim;
	char *pchMin;
	char ch;
	int ris;
	int doc;
	int  rds;
	struct RIBL *pribl;

#ifdef DEBUG
	Assert (cbPAP >= cbCHP);
	vhribl = hribl;
#endif /* DEBUG */

	pribl = *hribl;  /* local heap pointer usage only */

#ifdef XBZ
	CommSzNum(SzShared("rtfin ris = "), pribl->ris);
#endif
	if ((ris = pribl->ris) == risExit)
		{
		return;
		}
	FreezeHp();  /* for pribl */
	doc = pribl->doc;
	MeltHp();
	pchLim = pch + cch;


#ifdef BZ
	CommSzRgCch(SzShared("RTFIN text read: "), pch, cch);
#endif /* BZ */

	while (pch < pchLim && !vmerr.fDiskFail && !vmerr.fMemFail)
		{ /* Loop for all characters in input stream */
		switch (ris)
			{ /* Switch on state */
		case risNorm: /* Normal characters; write them out */
			/* Save initial position; scan for escape or end;
			write out chars if any; return if at end;
			set up to parse pmc */
			rds = (**hribl).rds;
			pchMin = pch;

			do
				{
				ch = *pch++;
				if ((**hribl).lcb > 0L)
					goto LRdsDispatch;

#ifdef DEBUG
				if (vdbs.fNoRtfConv)
					goto LRdsDispatch; /* neuter translation of rtf */
#endif /* DEBUG */

				switch (ch)
					{
				case chRTFOpenBrk:
					if (!FStackRtfState(hribl))
						{
						Assert (vmerr.fMemFail);
						goto LRetError;
						}
					goto LBack1;
				case chRTFCloseBrk:
					ris = risDoPop;
					goto LBack1;
				case chBackslash:
					ris = risB4ContSeq;
					goto LBack1;
				case chEop:

#ifdef CRLF
				case chReturn:
#endif /* CRLF */

LBack1:
					--pch;
					goto LDoBreak;
				default:
LRdsDispatch:
					switch (rds)
						{
					case rdsPic:
						ris = risScanPic;
						goto LBackUp;
					case rdsPrivate1:
						ris = risB4Private1;
						goto LBackUp;

					case rdsXe:
					case rdsTc:
						/* these must be escaped */
						if (ch == chColon || ch == chDQuote)
							{
							ris = risB4SpecXeTc;
							/* *(pch -1) is the colon/quote.
								*/
							--pch;
							goto LDoBreak;
							}

						break;
					case rdsStylesheet:
					case rdsFonttbl:
					case rdsColortbl:

					case rdsBkmkEnd:
					case rdsBkmkStart:

					case rdsFldType:
					case rdsGridtbl:

						/* note: for these types we assume that there
						will be no imbedded backslashes or curly braces.
						If that changes, we must process these as we do the
						text info fields - dump to the doc and replace
						out
						*/
						ris = risB4TableName;
LBackUp:
						--pch;
						goto LBreakNoPush;
						}
					if (!(**hribl).fInBody && (**hribl).rds == rdsMain)
						/* first RTF body text found */
						(**hribl).fInBody = fTrue;

					if (ch >= 128 || ch < 32)
						*(pch - 1) = (char)ChMapSpecChar(ch, (**hribl).chs);
					}
				}
			while (pch < pchLim);

LDoBreak:
			/* Note: it is safe to pass a chp stored on the heap to FInsertRgch. */
			if (pch != pchMin)
				{
#ifdef DRTFFTC
				CommSzNum(SzShared("ftc during finsertrgch: "), (**hribl).chp.ftc);
#endif

#ifdef BZTEST
				/* #ifdef DRTFCHP */
				CommSzRgNum(SzShared("chp during finsertrgch: "), &(**hribl).chp, cwCHP);
#endif

				FInsertRgch(doc, CpMacDoc(doc) - ccpEop, pchMin, pch - pchMin,
						&(**hribl).chp, 0);

#ifdef BZTEST
				CommSzRgNum(SzShared("chp after finsertrgch: "), &(**hribl).chp, cwCHP);
#endif
				}
			/* Skip over escape again - ignored if end of input */
			if (ris != risDoPop && ris != risB4SpecXeTc)
				++pch;
LBreakNoPush:
			break;
		case risExit:
			goto LReturn;
		case risB4ContSeq:
				{
				pribl= *hribl;
				FreezeHp();
				ch = *pch;
				if ((ch >= 'a' && ch <= 'z') || ch == ' ' ||
						(ch >= 'A' && ch <= 'Z'))
					{
					/* true when letter or space */
					ris = risContWord;
					pribl->bchSeqLim = 0;
					}
				else
					{
					char *pchSeqLim = pribl->rgch;    /* HEAP pointer! */

					*pchSeqLim++ = ch;
					pch++;
					*pchSeqLim = 0;
					ris = risAfContSeq;
					pribl->bchSeqLim = 1;
					MeltHp();
					break;
					}
				MeltHp();
				}
		case risContWord:
				{
				char *pchSeqLim;
				char *pchNumLim;
				pribl= *hribl;
				FreezeHp();
				pchSeqLim = pribl->rgch + pribl->bchSeqLim;
				pchNumLim = pribl->rgch + pribl->bchNumLim;
				do
					{
					int ch = *pch;
					if (ch == '-' || (ch >= '0' && ch <= '9'))
						{
						/* fTrue when '-' or a digit. */
						*pchSeqLim++ = 0;
						pchNumLim = pchSeqLim;
						*pchNumLim++ = ch;
						pch++;
						ris = risNumScan;
						break;
						}
					else  if (ch == ' ')
						{
						pch++;
						goto LEndContWord;
						}

					else  if (!((ch >= 'a' && ch <= 'z') ||
							(ch >= 'A' && ch <= 'Z')))
						{
						/* Note that CRLF will terminate the control word */
						/* true when not a letter and not a
						digit */
LEndContWord:
						*pchSeqLim = 0;
						ris = risAfContSeq;
						break;
						}
					*pchSeqLim++ = ch;
					pch++;
					}
				while (pch < pchLim);

				pribl->bchSeqLim = pchSeqLim - pribl->rgch;
				pribl->bchNumLim = pchNumLim - pribl->rgch;
				MeltHp();
				break;
				}
		case risNumScan:
				{
				char *pchNumLim;
				FreezeHp();
				pribl= *hribl;
				pchNumLim = pribl->rgch + pribl->bchNumLim;
				do
					{
					ch = *pch;
					if (ch == ' ')
						{
						pch++;
						goto LEndNumScan;
						}
					if (!((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') ||
							(ch >= 'A' && ch <= 'Z')))
						{
						/* true when not a letter and not a
						digit. Note a CRLF will end the num so be sure
						that generated control words are not CRLF interrupted
						*/
LEndNumScan:
						*pchNumLim = 0;
						ris = risAfContSeq;
						break;
						}

					*pchNumLim++ = ch;
					pch++;
					}
				while (pch < pchLim);

				pribl->bchNumLim = pchNumLim - pribl->rgch;
				MeltHp();

				break;
				}
		case risScanPic:
				{
				CHAR *pchData = (**hribl).rgch;
				/* lcb > 0 if binary data only. No spaces, cr/lf, etc allowed */
				/* Assert that we will disregard illegal pictures */
				Assert (((**hribl).iTypePic == iPicMacPict) ?
				    (**hribl).fDisregardPic: fTrue);

				if ((**hribl).lcb > 0L)
					{
					if (!((**hribl).fDisregardPic))
						{
						do
							{
							if (pchData >= (**hribl).rgch + cchMaxRibl)
								{
								int cbPicture = pchData - (**hribl).rgch;
								FcAppendRgchToFn(fnScratch,
										(**hribl).rgch, cbPicture);

								pchData = (**hribl).rgch;
								}
							*pchData++ = *pch++;
							(**hribl).lcb--;
							}
						while (pch < pchLim && (**hribl).lcb > 0L);
						}
					else
						{
						/* there is no lmin */
						long lcch = CpMin((**hribl).lcb, (long)(pchLim - pch));

#ifdef BZ
						CommSzLong(SzShared("Pic disregarding n bytes. n =: "), lcch);
#endif /* BZ */

						(**hribl).lcb -= lcch;
						pch += (int)lcch;
						}
					}
				else
					{
					do
						{
						ch = *pch;

						/*  handle picture switches. Normally this is done in the
						main loop, but if processing a picture we have to jump into this
						code and thus handle the switches ourselves.
						*/
						if ( ch == chRTFOpenBrk || ch == chRTFCloseBrk ||
								ch == chBackslash)
							{
							ris = risNorm;
							break;
							}

						if (!((**hribl).fDisregardPic))
							{
							int w;
							if (ch >= '0' && ch <= '9')
								w = ch - '0';
							else  if (ch >= 'a' && ch <= 'f')
								w = 10 + ch - 'a';
							else  if (ch >= 'A' && ch <= 'F')
								w = 10 + ch - 'A';
							else
								{
								/* ignore CR/LF, white space if non-binary picture */
								if (ch != chEop &&
										ch != chReturn &&
										ch != chTab &&
										ch != chSpace)
									/* bad data - pic discarded! */
									{
									/* extra info at debug */
									ReportSz("Warning - picture with bad data discarded");
									ErrorNoMemory(eidNoMemOperation);
									(**hribl).fDisregardPic = fTrue;
									}
								goto LNextPicChar;
								}
							(**hribl).bPic |= w << (4 * (**hribl).fHiNibble);
							if (!(**hribl).fHiNibble)
								{
								if (pchData >= (**hribl).rgch + cchMaxRibl)
									{
									FcAppendRgchToFn(fnScratch,
											(**hribl).rgch, pchData - (**hribl).rgch);
									pchData = (**hribl).rgch;
									}
								*pchData++ = (**hribl).bPic;
								(**hribl).bPic = 0;
								}
							(**hribl).fHiNibble ^= 1;
							}
LNextPicChar:
						++pch;
						}
					while (pch < pchLim);
					} /* else */

				/* if macpict or pic args, pchData will still be ==
				**hribl.rgch, so FcAppend will not happen */

				if (pchData != (**hribl).rgch)
					{
					int cbPicture = pchData - (**hribl).rgch;
					FcAppendRgchToFn(fnScratch,
							(**hribl).rgch, cbPicture);
					}
				break;
				}
		case risB4Private1:
			(**hribl).ibSea = 0;
			SetBytes((**hribl).rgbSea, 0, cbSEA);
		case risB4BinCode:
			(**hribl).fHiNibble = fTrue;
			(**hribl).b = 0;
			ris = risBinCode;
		case risBinCode:
				{
				int w;
				ch = *pch++;
				if ((**hribl).rds == rdsPrivate1 && (ch == chRTFOpenBrk
						|| ch == chRTFCloseBrk || ch == chBackslash))
					{
					ris = risNorm;
					--pch;
					break;
					}
				w = 0;
				if (ch >= '0' && ch <= '9')
					w = ch - '0';
				else  if (ch >= 'a' && ch <= 'f')
					w = 10 + ch - 'a';
				else  if (ch >= 'A' && ch <= 'F')
					w = 10 + ch - 'A';
				/* broken up due to native compiler bug */
				(**hribl).b |= w << (4 * (**hribl).fHiNibble);
				(**hribl).fHiNibble ^= 1;
				if ((**hribl).fHiNibble)
					{
					if ((**hribl).rds == rdsPrivate1)
						{
						if ((**hribl).ibSea < cbSEA)
							(**hribl).rgbSea[(**hribl).ibSea++] = (**hribl).b;
						ris = risB4BinCode;
						}
					else
						{
						char chOut;
						chOut = (char)ChMapSpecChar((**hribl).b, (**hribl).chs);
						(**hribl).chp.fSpec = fFalse;

						FInsertRgch( doc, CpMacDocEdit(doc), &chOut, 1,
								&(**hribl).chp, 0);

						ris = risNorm;
						}
					}
				}
			break;
		case risB4SpecXeTc:
				{
				char rgchT[2];

				/* we have dumped up to the quote or colon to the doc;
				now insert a backslash and the character */
				Assert (*pch == chDQuote || *pch == chColon);
				*rgchT = chBackslash;
				*(rgchT + 1) = *pch;

				FInsertRgch(doc, CpMacDoc(doc) - ccpEop, rgchT, 2,
						&(**hribl).chp, 0);

				ris = risNorm;
				++pch;
				break;
				}
		case risB4TableName:
			(**hribl).bchData = 0;
#ifdef XBZ
			CommSzNum(SzShared("rtfinrare b4 (**hribl).bchData: "), (**hribl).bchData);
#endif
			ris = risScanTableName;
		case risScanTableName:
			do
				{
				char ch;

				ch = *pch;
				if (ch == ';' || ch == chRTFCloseBrk ||
						ch == chRTFOpenBrk || ch == chBackslash)
					{
					if (ch == ';')
						++pch;
#ifdef XBZ
					CommSzNum(SzShared("rtfinrare end (**hribl).bchData: "), (**hribl).bchData);
					CommSzNum(SzShared("rtfinrare end ch: "), ch);
#endif
					ris = risAddTableName;
					break;
					}
				*((**hribl).rgch + (**hribl).bchData++) = ch;
				++pch;
				} /* end do under risScanTableName */
			while (pch < pchLim);
			break;
		default:
				{
				pribl = *hribl;  /* local heap pointer usage only */
				pribl->ris = ris;
				RtfInRare(hribl, &pch, pchLim);
				pribl = *hribl;  /* local heap pointer usage only */
				ris = pribl->ris;
				if (pribl->fCancel)
					goto LReturn;
				}
			break;
			} /* end switch ris */
		} /* end while pch < pchLim */
LRetError:
	if (vmerr.fDiskFail || vmerr.fMemFail)
		{
		ErrorNoMemory(eidNoMemOperation);
		ris = risExit;
		(*hribl)->fCancel = fTrue;
		}

LReturn:
	pribl = *hribl;  /* local heap pointer usage only */
	FreezeHp();  /* for pribl */
	pribl->ris = ris;
	MeltHp();
}


#endif /* DEBUG */


/*  %%Function:  RtfInRare  %%Owner:  bobz       */

EXPORT RtfInRare(hribl, ppch, pchLim)
struct RIBL **hribl;
char **ppch;
char *pchLim;
{

	int ris;
	int doc;
	char *pch = *ppch;
	struct RIBL *pribl;

	pribl = *hribl;  /* local heap pointer usage only */
	FreezeHp();  /* for pribl */
	doc = pribl->doc;
	ris = pribl->ris;
	MeltHp();

#ifdef XBZ
	CommSzNum(SzShared("rtfinrare ris: "), ris);
#endif

	switch (ris)
		{ /* Switch on state */

	case risAfContSeq:
			{
			int val;
			int fHaveVal;
			int irsym;
			struct RSYM rsym;

			ris = risNorm;
			if (!(**hribl).bchSeqLim)
				break;
			val = 0;
			fHaveVal = fFalse;
				{
				CHAR *pchSeqLim = (**hribl).rgch + (**hribl).bchSeqLim;
				if (*pchSeqLim != 0)
					{
					/* note Opus WfromSzRTF handles leading - and
					returns signed val. It won't skip over CR/LF
					though, but that's been handled above  */

					FreezeHp();
					if ((val = WFromSzRTF(pchSeqLim)) != wError)
						fHaveVal = fTrue;
					MeltHp();
					}
				}
			if (!FSearchRgrsym((**hribl).rgch, &irsym))
				{
				if ((*hribl)->fNextIsDest)
					goto LScanByDest;
				break;
				}
			(*hribl)->fNextIsDest = fFalse;
			rsym = rgrsymWord[irsym];
			if (rsym.fPassVal)
				{
				val = rsym.val;
				fHaveVal = fTrue;
				}
			switch (rsym.rac)
				{
			case racChngDest:
			case racSpecCharAct:
				/* DoQuoteForBinCode sets (**hribl).ris to risB4BinCode.
				If that happened, we set ris, but set the hribl value
				back so the next time we get here and call something other
				than DoQuote... we will not get the bogus bin stuff
				*/
				Assert ((**hribl).ris != risB4BinCode);
				(*rgpfnRtfActions[rsym.ipfnAction])(hribl, val);
				/* if ris was changed to risBinCode avoid
				resetting to risNorm */
				if ((**hribl).ris == risB4BinCode)
					{
					ris = risB4BinCode;
					(**hribl).ris = risNorm;
					goto LRacBreak;
					}
				   /* indicates error - must skip */
				else if ((**hribl).ris == risScanByDest)
					goto LScanByDest;

				break;
			case racChngProp:
				ApplyPropChange(rsym.irrbProp, fHaveVal, val, hribl);
				break;
			case racSpecChar:
					{
					char chOut;
					if (rsym.fSpec)
						(**hribl).chp.fSpec = fTrue;
					chOut = val;

#ifdef DRTFFTC
					CommSzNum(SzShared("ftc during finsertrgch: "), (**hribl).chp.ftc);
#endif

#ifdef DRTFSPEC
					if ((**hribl).chp.fSpec)
						CommSzNum(SzShared("spec char: "), chOut);
#endif
#ifdef BZTEST
					CommSzRgNum(SzShared("chp during single finsertrgch: "), &(**hribl).chp, cwCHP);
#endif

					FInsertRgch( doc, CpMacDocEdit(doc), &chOut, 1,
							&(**hribl).chp, 0);

					(**hribl).chp.fSpec = fFalse;
					break;
					}
			case racBinParm:
					{
					CHAR *pchSeqLim = (**hribl).rgch + (**hribl).bchSeqLim;
					(**hribl).lcb = 0L;
					if (*pchSeqLim != 0)
						(**hribl).lcb = LFromSzNumber(pchSeqLim);
					break;
					}
			case racScanByDest:		/* scan by this destination */
LScanByDest:
				ris = risScanByDest;
				cGrStart = 1;
				PopRtfState(hribl);
				goto LRacBreak;
			default:
				break;
				}
			ris = risNorm;
LRacBreak:
			break;
			} /* risAfContSeq */
	case risScanByDest:
		while (cGrStart > 0 && pch < pchLim)
			{
			if (*pch == chRTFOpenBrk)	      /* Group begin found */
				++cGrStart;
			else  if (*pch == chRTFCloseBrk)
				/* nested Group end found */
				--cGrStart;
			++pch;
			}
		if (cGrStart == 0)
			ris = risNorm;
		break;

	case risDoPop:
		PopRtfState(hribl);
		if ((ris = (**hribl).ris) != risExit)
			{
			ris = risNorm;
			++pch;
			}
		else
			pch = pchLim;
		break;
	case risAddTableName:
			{
			int cch;
			int stc;
			int cstcStd;
			int stcp;
			struct STSH stsh;
			struct STTB **hsttbChpe, **hsttbPape;
			char stName[cchMaxRibl];

			cch = stName[0] = (**hribl).bchData;
			Assert(cch >= 0);

			switch ((**hribl).rds)
				{
				int ftcs;
				int iftc;
				int fFontMatch;
				int icoOld, icoT, iRed, iGreen, iBlue;

#ifdef DEBUG
			default:
				Assert (fFalse);
				break;
#endif

			case rdsStylesheet:
				if (cch == 0 || cch >= cchMaxRibl - 1 || vmerr.fMemFail)
					goto LStyleExit;

				bltb((**hribl).rgch, stName+1, stName[0]);

				/* remove comma separated synonyms, if any */
				/* test above leaves space for null term */
				stName[stName[0] + 1] = 0;
					{
					char *pchT;
					if ((pchT = index(&stName[1], chComma)) != NULL)
						stName[0] = pchT - &stName[1];
					}
				/* limit length of style name (Mac name can be 256) */
				if (*stName >= cchMaxStyle)
					*stName = cchMaxStyle - 1;

				if (!FValidStyleName(stName))
					goto LStyleExit;

				if (!(**hribl).fStcNextOK)
					(**hribl).estcp.stcNext = (**hribl).pap.stc;

				if (!FEnsureRtfStcDefined(doc, stc = (**hribl).pap.stc,
						&cstcStd))
					goto LStyleExit;
#ifdef XBZ
				CommSzNum(SzShared("RTFIN stc before style acts: "), stc);
				if (stc > 200) ReportHeapSz (SzShared("  cbAvailHeap: "), 0 /*cwMinReport */);
#endif
				if (vmerr.fMemFail)
					goto LStyleExit;

				RecordStshForDoc(doc, &stsh, &hsttbChpe, &hsttbPape);
				stcp = StcpFromStc(stc, stsh.cstcStd);

				if (vmerr.fMemFail || !FChangeStInSttb(stsh.hsttbName, stcp, stName))
					goto LStyleExit;

				/* NOTE!!! FStorePropeForStcp blts the chp/pap
				before using it, so it is safe to pass a heap
				structure to it
				*/
				if (!FStorePropeForStcp(&(**hribl).chp, stcp, hsttbChpe, fTrue))
					goto LStyleExit;

				if (!FStorePropeForStcp(&(**hribl).pap, stcp, hsttbPape, fFalse))
					goto LStyleExit;

				SetStcBaseNextForStcp(&stsh, stcp, (**hribl).estcp.stcBase,
						(**hribl).estcp.stcNext);
				goto LStyleNoErr; /* done and ok */

LStyleExit:
				ErrorNoMemory(eidNoMemOperation);
LStyleNoErr:
				(**hribl).estcp.stcBase = stcStdMin;
				(**hribl).fStcNextOK = fFalse;
				break;

			case rdsFldType:   /* this can go away eventually. Only here so we can read old rtf files that used this (bz)*/
				break;

			case rdsBkmkEnd:
			case rdsBkmkStart:
					{
					struct CA caBkmk;
					int ibkf;

					if (cch == 0 || cch >= cchMaxRibl)
						break;

					/* put the passed name in stName */
					bltb((**hribl).rgch, stName+1, stName[0]);
					PcaPoint( &caBkmk, doc, CpMacDocEdit(doc));

					if ((**hribl).rds == rdsBkmkStart)
						/* add bkmk with cpLim == cpFirst for now */
						{

						if (FLegalBkmkName (stName))
							/* if this fails, name is not added to bkmk list, so bkmk is lost. No biggie */
							FInsertStBkmk (&caBkmk, stName, NULL);
#ifdef DEBUG
						else		/* report is a debugging aid only */
							ReportSz("Rtfin bad bkmkstart name");
#endif
						}
					   // don't look for end if no begin ever entered
					else if (PdodDoc(doc)->hsttbBkmk != hNil &&
						(ibkf = IbstFindSt (PdodDoc(doc)->hsttbBkmk, stName))
						    != -1)
						{
						/* get start cp from start entry (at ibkf)
						and reinsert bkmk */
						caBkmk.cpFirst = CpPlc( PdodDoc(doc)->hplcbkf, ibkf );

						if (FLegalBkmkName (stName))
							/* no biggie if this one fails */
							FInsertStBkmk (&caBkmk, stName, NULL);
#ifdef DEBUG
						else
							ReportSz("Rtfin bad bkmkend name 1");
#endif

#ifdef BZ
						CommSzLong(SzShared("Reinsert bkmkend at cpfirst: "), caBkmk.cpFirst);
						CommSzLong(SzShared("Reinsert bkmkend at cplim: "), caBkmk.cpLim);
#endif
						}
#ifdef DEBUG
					else
						ReportSz("Rtfin bad bkmkend name 2");
#endif
					break;
					} /* rdsBkmkStart, rdsBkmkEnd */
			case rdsFonttbl:
					{
					int doc;
					struct FTCMAP **hftcmap = (**hribl).hftcmap;
					struct FFN *pffn = (struct FFN *) stName;

					Assert (sizeof(stName) >= cbFfnLast);

					if (cch == 0 || cch >= cchMaxRibl ||
							hftcmap == hNil || vmerr.fMemFail)
						{
#ifdef BZ
						CommSzNum(SzShared("rtfin font fail cch: "), cch);
						CommSzNum(SzShared("rtfin font fail hftcmap: "), hftcmap);
						CommSzNum(SzShared("rtfin font fail vmerr.fMemFail: "), vmerr.fMemFail);
#endif
						ReportSz("Rtfin font table failure");
						break;
						}

					/* put the passed name in stName/pffn */
					/* max facename is LF_FACESIZE, which includes null	*/
					cch = min (cch, LF_FACESIZE - 1);
					bltb((**hribl).rgch, pffn->szFfn, cch);
					*(pffn->szFfn + cch) = 0;  /* null terminate string */

					/* fmc is a windows FF_ code, so mask bits correctly */
					pffn->ffid = (**hribl).fmc & maskFfFfid;
					pffn->cbFfnM1 = CbFfnFromCchSzFfn(cch);
					/* cch excludes null term. Above is same as: */
					/* pffn->cbFfnM1 = CbFfnFromCchSzFfn(cch + 1) - 1; IE */
					ChsPffn(pffn) = ANSI_CHARSET;
					/* if adding, force to think vector font so it will always print */
					pffn->fGraphics = fTrue;

					doc = (**hribl).doc;

					/* get the system ftc for this font name */
					/* danger - possible heap movement! */
					ftcs = FtcChkDocFfn(doc, pffn);

					if (ftcs == valNil)
						{
						ftcs = ftcDefault;
						ReportSz("Rtfin font not addable - use default");
						}
					if (!FSearchFtcmap(hftcmap, (**hribl).chp.ftc, &ftcs, fTrue /* fAdd */))
						goto LRetError;
#ifdef DRTFFTC
						{
						struct FTCM *pftcm;
						CommSzNum(SzShared("Building ftc map: system ftc "), ftcs);
						CommSzNum(SzShared("Building ftc map: rtf font code "), (**hribl).chp.ftc);
						Assert (FSearchFtcmap(hftcmap, (**hribl).chp.ftc, &ftcs, fFalse /* fAdd */));
						CommSzNum(SzShared("ftc map: stored to "), ftcs);
						CommSzNum(SzShared("ftc map: stored from "), (**hribl).chp.ftc);
						}
#endif

					break;
					} /* rdsFonttbl */
			case rdsColortbl:
				if ((icoOld = (**hribl).icoMac) >= icoRTFMax)
					break;
				if ((**hribl).fNoColor)
					icoT = icoAuto;
				else
					{
					iRed =	((**hribl).cRed > 128);
					iGreen = ((**hribl).cGreen > 128);
					iBlue = ((**hribl).cBlue > 128);
					icoT = ((iRed * 4) + (iGreen * 2) + iBlue) + 1;
					}
				(**hribl).mpicoOldicoNew[icoOld] = rgicoNew[icoT];
				(**hribl).icoMac++;
				(**hribl).cRed = (**hribl).cGreen = (**hribl).cBlue = 0;
				(**hribl).fNoColor = fTrue;
				break;
			case rdsGridtbl:
					{
					if ((**hribl).hsttbGrid != hNil &&
							((**hribl).hstGrid != hNil) )
						{
						/* no title - use stName for local gwc array */
						bltb(*((**hribl).hstGrid), stName,
								**(**hribl).hstGrid + 2);
						/* if allocation fails, we are done; memAlert check will stop us later */
						FInsStInSttb((**hribl).hsttbGrid,
								(**hribl).iGrid, stName);
#ifdef BZTEST
						CommSzNum(SzShared("grid table entry: "), (**hribl).iGrid);
						CommSzNum(SzShared("size of grid table : "), (*stName - 1) >> 1);
						CommSzRgNum(SzShared("gcw table at end of entry: "),
								&stName[2], (*stName - 1) >> 1);
#endif
						}
					break;
					}  /* rdsGridtbl */
				}  /*  switch ((**hribl).rds)  */

			ris = risNorm;
			break;
			} /* risAddTableName */
		} /* switch (ris) */
LRetError:
	if (vmerr.fDiskFail || vmerr.fMemFail)
		{
		ErrorNoMemory(eidNoMemOperation);
		ris = risExit;
		(*hribl)->fCancel = fTrue;
		}

	pribl = *hribl;  /* local heap pointer usage only */
	FreezeHp();  /* for pribl */
	pribl->ris = ris;
	MeltHp();
	*ppch = pch;
}


/* if (fAdd and ftcFrom is not in rgftcFrom) then add ftcFrom to
		rgftcFrom and *pftcTo to rgftcTo.  Return fTrue if
		the add was successful.
	if (fAdd and ftcFrom is in rgftcFrom) then replace the corresponding
		entry in rgftcTo with *pftcTo.  Return fTrue.
	if (!fAdd and ftcFrom is not in rgftcFrom) return fFalse.
	if (!fAdd and ftcFrom is in rgftcFrom) then set *pftcTo to the
		corresponding entry in rgftcTo.	Return fTrue.
*/
/*  %%Function:  FSearchFtcmap  %%Owner:  bobz       */

int FSearchFtcmap(hftcmap, ftcFrom, pftcTo, fAdd)
struct FTCMAP **hftcmap;
int ftcFrom;
int *pftcTo;
int fAdd;
{
	int iftcFrom, iMax;
	int fRet = fFalse;
	struct FTCMAP *pftcmap;

	FreezeHp();
	Assert (hftcmap != hNil);
	pftcmap = *hftcmap;
	iftcFrom = IScanLprgw( (int far *)pftcmap->rgwFtc, ftcFrom, pftcmap->iMac );

	if (!fAdd)
		{
		if (iftcFrom >= 0)
			{
			fRet = fTrue;
			*pftcTo = pftcmap->rgwFtc[pftcmap->iMax + iftcFrom];
			}
		goto LRet;
		}
	fRet = fTrue;
	if (iftcFrom >= 0)
		{
		pftcmap->rgwFtc[pftcmap->iMax + iftcFrom] = *pftcTo;
		goto LRet;
		}
	if (pftcmap->iMac >= pftcmap->iMax)
		{
		iMax = pftcmap->iMax + 10;
		MeltHp();
		if (!FChngSizeHCw(hftcmap,
				1 /* iMac */ + 1 /* iMax */
		+ iMax /* rgFtcFrom */ + iMax /* rgFtcTo */,
				fFalse))
			{
			FreezeHp();
			fRet = fFalse;
			goto LRet;
			}
		FreezeHp();
		pftcmap = *hftcmap;
		blt(&pftcmap->rgwFtc[pftcmap->iMax],
				&pftcmap->rgwFtc[iMax],
				pftcmap->iMax);
		pftcmap->iMax = iMax;
		}
	pftcmap->rgwFtc[pftcmap->iMac] = ftcFrom;
	pftcmap->rgwFtc[pftcmap->iMax + pftcmap->iMac] = *pftcTo;
	pftcmap->iMac++;
LRet:
	MeltHp();
	AssertH( hftcmap );  /* test for possible heap damage */
	return (fRet);
}


#ifdef DEBUG

/*  %%Function:  C_ChMapSpecChar  %%Owner:  bobz       */

HANDNATIVE char C_ChMapSpecChar(ch, chs)
char ch;
int chs;
{
	switch (chs)
		{
	case chsMac:
		if (ch >= 128)
			ch = mpchMACchANSI[ch-128];
		else  if (ch < 32)
			ch = '_'; /* underscore for unrecognized low chars */
		break;
	case chsPC:
		if (ch >= 128)
			ch = mpchIBMchANSI[ch-128];
		else  if (ch < 32)
			{
LSpecLow:
			if (ch == 20)
				ch = 182;
			else  if (ch == 21)
				ch = 167;
			else  if (ch == 15)
				ch = 150;
			else
				ch = '_'; /* underscore for unrecognized low chars */
			}
		break;
	case chsPCA:
		if (ch >= 128)
			ch = mpchPCAchANSI[ch-128];
		else  if (ch < 32)
			goto LSpecLow;
		break;
	case chsAnsi:
		if (ch < 32)
			ch = '_'; /* underscore for unrecognized low chars */
		break;
		}

	return (ch);
}


#endif /* DEBUG */

/*  %%Function:  FStackRtfState  %%Owner:  bobz       */

EXPORT FStackRtfState(hribl)
struct RIBL **hribl;
{
	int rds;
	int bRstn;
	char *rgbChar;
	char *rgbPara;
	char *rgbSect;
	char *rgbStack;
	int bStackLim;
	struct RSTN rstn;

	char  rgb[cbTAP];  /* big enough for any props */

		{
		struct RIBL *pribl;

#ifdef BZTEST
		CommSzNum(SzShared("FStackRtfState cbPAP: "), cbPAP);
		CommSzNum(SzShared("FStackRtfState cbSEP: "), cbSEP);
		CommSzNum(SzShared("FStackRtfState cbCHP: "), cbCHP);
		CommSzNum(SzShared("FStackRtfState cbTAP: "), cbTAP);
#endif
		/* rbg big enough for all */
		Assert (cbTAP >= cbCHP && cbTAP >= cbSEP && cbTAP >= cbPAP);
		pribl = *hribl;    /* very local heap pointer usage */
		FreezeHp();  /* for pribl */

		rstn.bPrev = pribl->bStackLim;
		rstn.rds = pribl->rds;
		rstn.hrcpfld = pribl->hrcpfld; /* this is only used by fields */
		rstn.cpFirst = pribl->cpRdsFirst;
		rstn.bCharInfo = rstn.bParaInfo = rstn.bSectInfo = rstn.bTableInfo = 0;
		bStackLim = pribl->bStackLim;
#ifdef BZTEST
		CommSzNum(SzShared("FStackRtfState start bStacklim: "), bStackLim);
#endif
		MeltHp();
		}


	RetrPropFromRtfStack(&rgb, hribl, pgcChar);
	if (FNeRgch((**hribl).rgbChar, rgb, (**hribl).cbCharInfo))
		{
		rstn.bCharInfo = bStackLim;
		bStackLim += (**hribl).cbCharInfo;
		}

	SetWords(&rgb, 0, cwPAP);
	RetrPropFromRtfStack(&rgb, hribl, pgcPara);
	if (FNeRgch((**hribl).rgbPara, rgb, (**hribl).cbParaInfo))
		{
		rstn.bParaInfo = bStackLim;
		bStackLim += (**hribl).cbParaInfo;
		}

	RetrPropFromRtfStack(&rgb, hribl, pgcSect);
	if (FNeRgch((**hribl).rgbSect, rgb, (**hribl).cbSectInfo))
		{
		rstn.bSectInfo = bStackLim;
		bStackLim += (**hribl).cbSectInfo;
		}
	RetrPropFromRtfStack(&rgb, hribl, pgcTable);
	if (FNeRgch((**hribl).rgbTable, rgb, (**hribl).cbTableInfo))
		{
		rstn.bTableInfo = bStackLim;
		bStackLim += (**hribl).cbTableInfo;
		}

	bRstn = bStackLim;
	bStackLim += sizeof(struct RSTN);

#ifdef BZTEST
	CommSzNum(SzShared("FStackRtfState change bStacklim: "), bStackLim);
#endif
	if (!FChngSizeHCw((**hribl).hrgbStack, CwFromCch(bStackLim), fFalse))
		return (fFalse);

	(**hribl).bStackLim = bStackLim;
	rgbStack = *(**hribl).hrgbStack;
	if (rstn.bCharInfo)
		bltb((**hribl).rgbChar, rgbStack + rstn.bCharInfo, (**hribl).cbCharInfo);
	if (rstn.bParaInfo)
		bltb((**hribl).rgbPara, rgbStack + rstn.bParaInfo, (**hribl).cbParaInfo);
	if (rstn.bSectInfo)
		bltb((**hribl).rgbSect, rgbStack + rstn.bSectInfo, (**hribl).cbSectInfo);
	if (rstn.bTableInfo)
		bltb((**hribl).rgbTable, rgbStack + rstn.bTableInfo, (**hribl).cbTableInfo);

	bltb(&rstn, rgbStack + bRstn, sizeof(struct RSTN));

#ifdef BZTEST
	AssertH( (**hribl).hrgbStack );  /* test for possible heap damage */
#endif /* BZ */

	return (fTrue);

}


/*  %%Function:  RetrPropFromRtfStack  %%Owner:  bobz       */

RetrPropFromRtfStack(pprop, hribl, wPropType)
char *pprop;
struct RIBL **hribl;
int wPropType;
	/* (pj 3/10): takes ~3% of RTF conversion when pcode */
{{ /* NATIVE - RetrPropFromRtfStack */
	char *rgbStack;
	int cbProp;

	int bInfoOffset;
	int *pbProp;
	int bProp;
	struct RSTN rstn;

		{
		struct RIBL *pribl;
		pribl = *hribl;   /* very local heap pointer usage */
		FreezeHp();

		rgbStack = *pribl->hrgbStack;

		switch (wPropType)
			{
		case pgcChar:
			cbProp = pribl->cbCharInfo;

			bInfoOffset = offset(RSTN, bCharInfo);
			break;
		case pgcPara:
			cbProp = pribl->cbParaInfo;

			bInfoOffset = offset(RSTN, bParaInfo);
			break;
		case pgcSect:
			cbProp = pribl->cbSectInfo;

			bInfoOffset = offset(RSTN, bSectInfo);
			break;
		case pgcTable:
			cbProp = pribl->cbTableInfo;

			bInfoOffset = offset(RSTN, bTableInfo);
			}

		MeltHp();   /* for pribl */
		}


#ifdef XBZTEST
	CommSzNum(SzShared("RetrProp...pribl->bStacklim: "), (**hribl).bStackLim);
#endif
	rstn.bPrev = (**hribl).bStackLim;
	bProp = 0;
	while (rstn.bPrev > 1)
		{
		bltb(rgbStack + rstn.bPrev - sizeof(struct RSTN),
				&rstn, sizeof(struct RSTN));
		if ((bProp = *(int *)((char *)&rstn + bInfoOffset)) > 0)
			break;
		}
	if (bProp)
		bltb(rgbStack + bProp, pprop, cbProp);
	else

#ifdef WIN
		switch (wPropType)
			{
		case pgcChar:
			RtfStandardChp(pprop);
			break;
		case pgcPara:
			StandardPap(pprop);
			break;
		case pgcSect:
			RtfStandardSep(pprop);
			break;
		case pgcTable:
			RtfStandardTap(pprop);
			break;
			}
#else
	(*pfnDefault)(pprop);
#endif /* WIN */


}}


/*  %%Function:  PopRtfState  %%Owner:  bobz       */

PopRtfState(hribl)
struct RIBL **hribl;
{
	int rdsOld;
	int docFtn;
	int docAtn;
	int docHdr;
	int stcOld;
	int stcNew;
	int stcp;
	int ihdtNew;
	int ihdd;
	int ihdt;
	int grpfIhdt;
	int fDopHdrs;
	int fNeedEop;
	struct CA caT;
	struct RSTN *prstn;
	struct STSH stshSrc;
	struct STTB **hsttbChpeSrc;
	struct STTB **hsttbPapeSrc;
	struct STSH stshDest;
	struct STTB **hsttbChpeDest;
	struct STTB **hsttbPapeDest;
	struct FTCM *pftcm;
	struct RSTN rstn;
	char rgb[256];

#ifdef XBZTEST
	CommSzNum(SzShared("PopRtfState start bStacklim: "), (**hribl).bStackLim);
#endif
	/* this should not happen except in invalid user input that
		starts with two closing brackets */
	if ((**hribl).bStackLim <= 1)
		{
		(**hribl).ris = risExit;
		Assert (fFalse);  /* temp - warn of probable future problems */
		return;
		}

	if ((**hribl).bStackLim > 1)
		{
		prstn = (*(**hribl).hrgbStack) + (**hribl).bStackLim -
				sizeof(struct RSTN);
		bltb(prstn, &rstn, sizeof(struct RSTN));
		if ((rdsOld = (**hribl).rds) != rstn.rds)
			{
			int doc = (**hribl).doc;

			stcOld = (**hribl).pap.stc;
			if (vmerr.fMemFail || vmerr.fDiskFail)
				{
				ReportSz("Warning - PopRtfState memory failure");
				goto LSkipActions;
				}
			switch (rdsOld)
				{

			case rdsFtn:
				if (!FFinishFtnAtn(hribl, edcDrpFtn))
					goto LSkipActions;
				break;

			case rdsAnnot:
				if (!FFinishFtnAtn(hribl, edcDrpAtn))
					goto LSkipActions;
				break;

			case rdsHdr:
					{
					CP cpMac;

					CachePara(doc, CpMax(cp0, (cpMac = CpMax(cp0, CpMacDoc(doc)) - 2*ccpEop)));
					if (caPara.cpLim == cpMac ||
							(**hribl).cpRdsFirst == CpMacDocEdit(doc))
						DoRtfParaEnd(hribl, chEop);
					if ((docHdr = PdodDoc(doc)->docHdr) == docNil)
						{
						/*	sets up links both ways */
						if ( (docHdr = DocCreateSub(doc, dkHdr)) == docNil )
							{
							ReportSz("Warning - Header doc creation failure");
							goto LSkipActions;
							}
						Assert(PdodDoc(docHdr)->hplchdd == hNil);
						/*	places handle into dod itself */
						if (HplcCreateEdc(mpdochdod[docHdr], edcHdd) == 0)
							{
							ReportSz("Warning - Header plc creation failure");
							DisposeDoc(docHdr);
							PdodDoc(DocMother(doc))->docHdr = docNil;
							goto LSkipActions;
							}
						}
					ihdtNew = (**hribl).ihdt;
					fDopHdrs = (ihdtNew >= ihdtTFtn);
					if (fDopHdrs)
						{
						ihdd = 0;
						ihdt = ihdtTFtn;
						grpfIhdt = PdodDoc(doc)->dop.grpfIhdt;
						}
					else  if ((ihdd = IhddFromDocCp(PcaSet(&caT, doc, cp0,
							(**hribl).cpRdsFirst), 0, 0)) == -1)
						goto LSkipActions;
					else
						{
						ihdt = 0;
						grpfIhdt = (**hribl).sep.grpfIhdt;
						}

					for (; grpfIhdt != 0 && ihdt < ihdtNew; grpfIhdt >>= 1, ihdt++)
						if (grpfIhdt & 1)
							ihdd++;
					if ((grpfIhdt & 1) == 0)
						{
						if (!FInsertIhdd(docHdr, ihdd))
							goto LSkipActions;
						if (fDopHdrs)
							PdodDoc(doc)->dop.grpfIhdt |= (1 << (ihdtNew - ihdtTFtn));
						else
							(**hribl).sep.grpfIhdt |= 1 << ihdtNew;
						}
					caT.doc = doc;
					caT.cpFirst = (**hribl).cpRdsFirst;
					caT.cpLim = CpMacDoc(doc) - ccpEop;

					/* be sure there is an eop here - as a trick, grab it from
					the end of doc if needed for the replaceihdd, but
					grab it back before deleting the text.
					*/
					CachePara(doc, CpMax(caT.cpFirst, caT.cpLim - ccpEop));
					fNeedEop = fFalse;
					if (caT.cpLim != caPara.cpLim)
						{
						caT.cpLim += ccpEop;
						fNeedEop = fTrue;
						}
					ReplaceIhdd(docHdr, ihdd, &caT);
					if (fNeedEop)
						caT.cpLim -= ccpEop;
					FDelete(&caT);
					break;
					}
			case rdsStylesheet:
					{
					int idError;
					struct DOD *pdod;
					CP cp;

					cp = (**hribl).cpRdsFirst;
					FDelete( PcaSet( &caT, doc, cp, CpMacDocEdit(doc)));
					/* fun and games. we're going to save the style sheet we accumulated during
					our scan in stshSrc, hsttbChpeSrc, hsttbPapeSrc. */
					RecordStshForDoc(doc, &stshSrc, &hsttbChpeSrc,
							&hsttbPapeSrc);
					/*  Then we will give the document we're building a STSH that contains only
					stcNormal. */

#ifdef XBZTEST
					CommSz(SzShared("RTF style sheet prior to CopyStyle\n"));
					DumpStsh(&stshSrc, hsttbChpeSrc, hsttbPapeSrc);
#endif /* XBZTEST */

					if (!FCreateStshNormalStc(&stshDest, &hsttbChpeDest,
							&hsttbPapeDest))
						{
						/* extra report info for debug */
						ReportSz("Warning - out of memory inserting style");
						goto LSkipActions;
						}
					pdod = PdodDoc(doc);
					FreezeHp();  /* for pdod */
					pdod->stsh = stshDest;
					pdod->hsttbChpe = hsttbChpeDest;
					pdod->hsttbPape = hsttbPapeDest;
					MeltHp();  /* for pdod */

					/* make backup the first style allocated so files read
					in by RTF and files generated into RTF will have the
					same style numbering in most cases (useful for diff'ing
					RTF files)
					*/
					stcp = StcpCreateNewStyle(doc, fTrue /* fUseReserve */);

#ifdef XBZ
					DumpStshNml(&stshDest, hsttbPapeDest);
#endif /* BZ */


					if (stcp != stcpNil)
						{
						char HUGE *hpst;

						vstcBackup = StcFromStcp(stcp, stshDest.cstcStd);
						/* HACK ALERT - doing this to fake out style code.
						Blame this one on the Daves
						*/
						hpst = HpstFromSttb (stshDest.hsttbName, stcp);
						*hpst = 0;	/* FCopy will leave this alone now */
						}
					else
						vstcBackup = stcNil;

					/* Then we will merge the newly created STSH into the plain STSH using
					CopyStyletoDestStsh. This will do any name reassignments and remappings
					necessary to turn the new STSH into a STSH that is valid for this version
					of Word. */
					/* docSrc is used only for font mapping which we don't need to worry
					about since we are writing to a new doc with no former ftc's.
					docNil means don't map fonts
					*/
					if (!FCopyStyleToDestStsh(docNil, /* docSrc */ &stshSrc,
							hsttbChpeSrc, hsttbPapeSrc, doc, &stshDest,
							hsttbChpeDest, hsttbPapeDest, 0, 0,
							rgb - 2, fTrue/* copy all props*/,
							fTrue /* this is an RTF merge... */, fFalse, &idError))
						{
						if (idError == -1)
							ErrorNoMemory(eidNoMemMerge);
						else
							ErrorEid(eidStyleSheetTooLarge,"PopRtfState");
						}


					/* restore the backup style slot to nullness (mate
					to HACK above) */
					if (vstcBackup != stcNil)
						{
						char HUGE *hpst;

						stcp = StcpFromStc(vstcBackup, stshDest.cstcStd);
						hpst = HpstFromSttb (stshDest.hsttbName, stcp);
						*hpst = 255;	/* unused entry */
						}
#ifdef XBZTEST
					CommSz(SzShared("RTF style sheet after CopyStyle\n"));
					pdod = PdodDoc(doc);
					DumpStsh(&pdod->stsh, pdod->hsttbChpe, pdod->hsttbPape);
					CommSzNum(SzShared("dop.grpfIhdt after copystyle = "), PdodDoc(doc)->dop.grpfIhdt);
#endif /* XBZTEST */


#ifdef XBZ
						{
						int i;
						for (i = 0; i < 256; i++)
							{
							if ((**hribl).rgbStcfSBS[i >> 3] & 1 << (i - ((i >> 3) << 3)))
								CommSzNum(SzShared("Stylesheet sty with true fSbs: "), i);
							}

						}
#endif /* BZ */

					bltb(rgb, (**hribl).rgbStcPermutation, 256);
					(**hribl).fPermOK = fTrue;
					vdocPapLast = docNil;
					caPara.doc = docNil;
					FreeHsttb(stshSrc.hsttbName);
					FreeHsttb(stshSrc.hsttbChpx);
					FreeHsttb(stshSrc.hsttbPapx);
					FreeH(stshSrc.hplestcp);
					FreeHsttb(hsttbChpeSrc);
					FreeHsttb(hsttbPapeSrc);

					/*  we have a backup stc in position 1 but the style
					code will reallocate one and wants this global
					set to nil, so do it now.
					*/
					vstcBackup = stcNil;

					break;
					}
			case rdsPic:

				if (!(**hribl).fDisregardPic)
					{

					FC fcPicFirst;
					int dza;

					fcPicFirst = (**hribl).chp.fcPic;
					/* fill in the PIC  */
					(**hribl).pic.lcb = PfcbFn(fnScratch)->cbMac - fcPicFirst;
					/* overwrite the PIC directly before the Mac picture. */
					/* note that the bm structure for a bitmap, and
					the mfp structure for a metafile have been
					set up already. We put some bitmap values into the mfp
					structure, so do that here
					*/
					if ((**hribl).iTypePic ==  iPicWBitmap)
						{
						(**hribl).pic.mfp.mm = MM_BITMAP;
						(**hribl).pic.mfp.xExt = (**hribl).pic.bm.bmWidth;
						(**hribl).pic.mfp.yExt = (**hribl).pic.bm.bmHeight;
						}

					/* most invalid values cause pic to be tossed out -
					   just don't insert the chPic char so pic data is
					   lost in the fnPic
					*/

						  /* works for bm or mf after above */	 
					if	((**hribl).pic.mfp.xExt <= 0 ||
					     (**hribl).pic.mfp.yExt <= 0)
						 {
						 break;		/* abandon the picture */
						 }
				    else if ((**hribl).iTypePic == iPicWBitmap)
						 {
						 if ((**hribl).pic.bm.bmWidthBytes <= 0 ||
							 (**hribl).pic.bm.bmPlanes <= 0	||
							 (**hribl).pic.bm.bmBitsPixel <= 0)
							 {
							 break;
							 }
						 }

					if ((**hribl).pic.dxaGoal <= 0 ||
					    (**hribl).pic.dyaGoal <= 0)
							{
							METAFILEPICT mfp;

					        if ((**hribl).iTypePic == iPicWBitmap)
							 /* convert goal sizes to 3" x aspect ratio
								using clipbord trick
							 */
								{
								mfp.mm = MM_ISOTROPIC;
								mfp.xExt = -(**hribl).pic.bm.bmWidth;
								mfp.yExt = -(**hribl).pic.bm.bmHeight;
								}
							else
							    /* metafiles */
								mfp = (**hribl).pic.mfp;

							if (!FComputePictSize( &mfp,
								&(**hribl).pic.dxaGoal, &(**hribl).pic.dyaGoal ))
									{
									break;
									}
							}

		   			/* scaling values come in as a %age, so scale to to the
		   				mz100Pct range
		   			*/

		   			(**hribl).pic.mx *= PctTomx100Pct;
		   			(**hribl).pic.my *= PctTomy100Pct;

					 /* FCheckPic will at some level refer to vPicFetch; we
						usually never care about it in rtfin, so it is ok
						to just slam it in here. FCheckPic returns false if
						either the cropping or scaling is illegal; it is hard
						to tell which, so we just throw it all away
					 */
					vpicFetch = (**hribl).pic;
				      /* pass vPicFetch rather than heap pointer */
					if (!FCheckPic(&vpicFetch, NULL  /* pca */))
						{
						(**hribl).pic.mx = mx100Pct;
						(**hribl).pic.my = my100Pct;
						(**hribl).pic.dxaCropLeft =
							(**hribl).pic.dxaCropRight =
							(**hribl).pic.dyaCropTop =
							(**hribl).pic.dyaCropBottom = 0;
						}

					SetFnPos(fnScratch, fcPicFirst);
					WriteRgchToFn(fnScratch, &(**hribl).pic, cbPIC);
					/* write chPicture to doc. */
					rgb[0] = chPicture;
#ifdef XBZTEST
					CommSzRgNum(SzShared("RtfIn picture chp: "), &(**hribl).chp, cwCHP);
#endif
					FInsertRgch(doc, CpMacDoc(doc) - ccpEop, rgb, 1, &(**hribl).chp, 0);
					if (DocMother(doc) == docScrap)
						vsab.fMayHavePic = fTrue;
					else
						PdodMother(doc)->fMayHavePic = fTrue;
					}
				break;

			case rdsFonttbl:
				if ((**hribl).hftcmap != hNil)
					{
					(**hribl).fFonttblDef = fTrue; /* always for win */
					/* now map the incoming deff font to our table */
					FSearchFtcmap((*hribl)->hftcmap, (*hribl)->deff, &ftcMapDef, fFalse /* fAdd */);
					}
				break;

			case rdsGridtbl:
				if ((**hribl).hstGrid != hNil)
					{
					FreeH((**hribl).hstGrid);
					(**hribl).hstGrid = hNil;
					}
				break;

			case rdsGrid:
				(**hribl).iGrid = iNil;
				/* check # of columns */
				if ((*PstFromSttb((**hribl).hsttbGrid, 0) - 1)/2 >= itcMax)
					{
					/* just warn the user so they don't wonder why they got tabs */
					ErrorEid(eidFormatTooComplex,"PopRtfState");
					}
				break;

			case rdsComment:   /* just throw out string */
				break;

			case rdsTitle:
			case rdsSubject:
			case rdsAuthor:
			case rdsKeywords:
			case rdsDoccomm:
			case rdsOperator:

			case rdsTemplate:
			case rdsAtnid:

					{
					/* these we dumped the text into the doc and kept track
					of the cps so imbedded special chars would be
					converted. Now grab out of the doc, store and
					delete from the doc.
					*/

					/* common to all these types: */
					int cch;
					CP dcp = CpMacDocEdit((**hribl).doc) -
							(**hribl).cpRdsFirst;

					/* note: if too long, text stays in doc! */
					if (dcp == 0L || dcp >= (LONG)(sizeof(rgb) - 1))
						break;

					FetchRgch(&cch, rgb + 1, (**hribl).doc,
							(**hribl).cpRdsFirst,
							CpMacDoc((**hribl).doc) - ccpEop, sizeof(rgb) - 1);

					Assert (cch < sizeof(rgb) - 1);
					Assert (cch < 256);	/* for st */
					*rgb = cch;

					switch (rdsOld)
						{
					case rdsTitle:
					case rdsSubject:
					case rdsAuthor:
					case rdsKeywords:
					case rdsDoccomm:
					case rdsOperator:

							{
							struct STTB **hsttbAssoc;

							/* store in info block	(note we want an st) */
							if ((hsttbAssoc = HsttbAssocEnsure ((**hribl).doc))
									!= hNil)
								{
								SetInfoFromIifdPval(IifdFromRds((**hribl).rds),
										rgb, (**hribl).doc, hsttbAssoc);
								}
							break;
							}
					case rdsTemplate:
							{
							ChangeDocDot ((**hribl).doc,
									DocOpenDot (rgb, fTrue, (**hribl).doc));
							break;
							}
					case rdsAtnid:
							{
							/* store it in the hribl */
							bltbyte(&rgb[0], (**hribl).stUsrInitl, rgb[0] + 1);
							break;
							}
						}  /* switch rdsold */

					/* delete text from doc */
					FDelete( PcaSet( &caT, (**hribl).doc, (**hribl).cpRdsFirst,
							(**hribl).cpRdsFirst + dcp ));
					break;
					}

				/* these rds's are time/date fields. The hr, mint, sec, yr,
				mo, dom flags filled up the DTR structure, which we
				cleared to 0's when this rds started. Now convert the
				DTR values to internal form and store in info block
				*/
			case rdsCreatim:
			case rdsRevtim:
			case rdsPrintim:

#ifdef UNUSED
			case rdsBuptim:
#endif /* UNUSED */
					{
					struct DTTM dttm;
					struct DTTM DttmFromPdtr();

					dttm = DttmFromPdtr(&dtrRTF);  /* convert struct to internal date/time form  */
					if (FValidDttm(dttm))
						{
						dttm.wdy = DowFromDttm(dttm);
						SetInfoFromIifdPval(IifdFromRds(rdsOld), &dttm,
								(**hribl).doc, hNil /* unused hsttbAssoc */);
						}

#ifdef DRTFDTTM
					CommSzNum(SzShared("DTR->yr "), dtrRTF.yr);
					CommSzNum(SzShared("DTR->mon "), dtrRTF.mon);
					CommSzNum(SzShared("DTR->dom "), dtrRTF.dom);
					CommSzNum(SzShared("DTR->hr "), dtrRTF.hr);
					CommSzNum(SzShared("DTR->mint"), dtrRTF.mint);
					CommSzNum(SzShared("DTR->sec "), dtrRTF.sec);
					CommSzNum(SzShared("DTR->wdy "), dtrRTF.wdy);
					CommSzNum(SzShared("computed DTTM "), dttm);
#endif
					break;
					}
			case rdsTxe:
			case rdsRxe:
					{
					CP cpDummy;
					int doc = (**hribl).doc;
					/* close off open quote and set cp  */
					CloseQuoteXeTc (hribl, &cpDummy);

					/* if prior to the main text, start quotes for main */
					if ((**hribl).hrcpxetc != hNil)
						{
						struct RCPXETC *prcpxetc;
						prcpxetc = *((**hribl).hrcpxetc);  /* heap pointer! */
						prcpxetc->fInText = fFalse;
						if (prcpxetc->cpFirstEntry == cpNil)
							{
							/* start text back up */
							*rgb = chSpace;
							*(rgb + 1 ) = chDQuote;
							FInsertRgch(doc, CpMacDocEdit(doc), rgb,
									2, &(**hribl).chp, 0);

							prcpxetc = *((**hribl).hrcpxetc);  /* heap pointer! */
							prcpxetc->cpFirstEntry = CpMacDocEdit(doc);
							prcpxetc->fVanishEntry =
									(**hribl).chp.fVanish;
							prcpxetc->fInEntry = fTrue;
							(*hribl)->fXeTcQOpen = fTrue; /* so we can end quotes */
							}
						}
					(*hribl)->fInXeRT = fFalse; /* so we can tell if we nest */
					break;
					}
			case rdsXe:
			case rdsTc:
					{
					FinishXeTc(hribl);
					(*hribl)->fInXeTc = fFalse; /* so we can tell if we nest */
					break;
					}
			case rdsField:
					{
					int doc, ifld, flt;
					CP cpFirst, dcpInst;
					struct RCPFLD *prcpfld;


					/* this code assumes that the ribl contains an
					hrcpfld which was allocated and contains the
					start CP, and dcp's for the instruction and
					result text (result dcp may be 0).
					
						it also assumes the the cpRdsFirst field has
						been pushed and popped by nested destinations and so it
						now the cpFirst of the field.
					*/

					/* heap pointer - will be invalidated below */

					prcpfld = *((**hribl).hrcpfld);

					if ( !((**hribl).rds == rdsField
							&& (**hribl).hrcpfld != hNil) )
						{
						/* error recovery:
						if we fail, chars are in the stream but not fieldified.
						can't think of much better solution bz    */
						goto CleanupFld;
						}

					/* all the characters in the instruction and result
					texts have been written to the CP stream; we saved
					the starting CP when the \field was encountered, and
					also the dcps of the instruction and result texts.
					Now we call field processing routines to turn the CP's
					(with any nexted fields already fieldified) into fields
					and force the field to be parsed.
					*/

					/* local heap pointer prcpfld will be invalidated
					in the call below */

					dcpInst = prcpfld->dcpInst;
					if (!FPutCpsInField (fltUnknownKwd, (doc = (**hribl).doc),
							(cpFirst = (**hribl).cpRdsFirst),
							&dcpInst, prcpfld->dcpRslt, NULL))
						/* error recovery:   fMemFail is set */
						goto CleanupFld;

					flt = FltParseDocCp (doc, cpFirst,
							IfldFromDocCp (doc, cpFirst, fTrue),
							fFalse	/* fChangeView */, fTrue /* fEnglish */);

					/* may have been changed by parse above if dead */
					ifld = IfldFromDocCp (doc, cpFirst, fTrue);
					if (ifld != ifldNil)
						{
						struct RFFLD  *prffld;
						struct FLCD flcd;

						GetIfldFlcd (doc, ifld, &flcd);

						prcpfld = *((**hribl).hrcpfld);
						prffld = &(prcpfld->rffld);  /* local use heap pointer */
						FreezeHp();
						/* note: edit implies dirty, so if edit, set dirty as
						well. On output we will only put out one of
						the two.
						*/
						if (prffld->fResultDirty)
							flcd.fResultDirty = fTrue;
						if (prffld->fResultEdited)
							{
							flcd.fResultDirty = fTrue;
							flcd.fResultEdited = fTrue;
							}
						if (prffld->fLocked)
							flcd.fLocked  = fTrue;

						if (prffld->fPrivateResult)
							{
							flcd.fPrivateResult = fTrue;
							/* at present, only private result field is
							import, and that has an empty result on
							generation unless there was an error, when
							the result will be the error message.
							*/
							}

						MeltHp();

						/*	write the changes to the plc */
						SetFlcdCh (doc, &flcd, chFieldEnd);

						/* force import fields to update if no result
						   (TIFF) or metafile from winword 1.0 which would
						   be missing win org/ext records
						*/
/* ranges:
    nMajorProduct 1..7      - major rev number
    nMinorProduct 0..63     - update number
    nRevProduct 1..63       - testing release number
    nIncrProduct (0 or 1)   - 0 if real testing release, else 1
*/
/* Product version number for 1.1 */
#define nMajor 1
#define nMinor 1
#define nRev 15
#define nIncr 1

#define nProductWW11 (((nMajor&0x07)<<13)+((nMinor&0x3f)<<7)+\
                        ((nRev&0x3f)<<1)+(nIncr?1:0))

						if (flt == fltImport)
							{
							(**hribl).fFldImport = fTrue;


#ifdef BZ
	CommSzNum(SzShared("Import field: vern "), (**hribl).vern);

#endif /* BZ */
							// Only refresh when no result or older version
							// of WinWord (pre 1.1a).
							// Note that for PM Word, we set the high bit
						    // of the vern, so PM Word will never refresh (so vern < 0 )
							if (prcpfld->dcpRslt == cp0 ||
								((**hribl).chs == chsAnsi
								 	&& (**hribl).vern > 0
									&& (**hribl).vern <= nProductWW11))
								{
								FCalcFieldIfld (doc, ifld, frmUser, 0, fFalse);
								}
							}
						}
CleanupFld:
					FreeH((**hribl).hrcpfld); /* free up special field area */
					(**hribl).hrcpfld = hNil;

					}
				break;

			case rdsFldInst:
					{
					struct RCPFLD *prcpfld;

					/* this code assumes that the rds stacked before the
					FldInst was a field; the hrcpfld for the field
					was allocated and has space for the dcp for the
					instruction text, and that cpRdsFirst was preserved
						by any nested destinations.
					*/
					Assert (rstn.rds == rdsField && rstn.hrcpfld != hNil);

					/* all the characters in the instruction text hav been
					written to the CP stream; we saved the starting CP
					when the \field was encountered, so the difference
					between the current end of doc and the saved CP is
					the #char in the instruction text.
					*/

					prcpfld = *(rstn.hrcpfld);   /* heap pointer! */
					FreezeHp();
					prcpfld->dcpInst =
							CpMacDocEdit((**hribl).doc) - (**hribl).cpRdsFirst;
					MeltHp();
					}
				break;
			case rdsFldRslt:
					{
					struct RCPFLD *prcpfld;

					/* this code assumes that the rds stacked before the
					FldRslt was a field; the hrcpfld for the field
					was allocated and has space for the dcp for the
					result text.
					*/
					Assert (rstn.rds == rdsField && rstn.hrcpfld != hNil);

					/* all the characters in the result text have been
					written to the CP stream; we saved the starting CP
					when the \field was encountered, and also the dcp
					of the instruction text, so the difference
					between the current end of doc and the saved CP
					+ dcpInst is the #char in the result text.
					*/

					prcpfld = *(rstn.hrcpfld); /* heap pointer! */
					FreezeHp();
					prcpfld->dcpRslt =
							CpMacDocEdit((**hribl).doc) - (**hribl).cpRdsFirst;

#ifdef DRTFFLD
					CommSzNum(SzShared("field result cpfirst: "), (**hribl).cpRdsFirst);
					CommSzNum(SzShared("field dcpInst: "), prcpfld->dcpInst);
					CommSzNum(SzShared("field dcpRslt: "), prcpfld->dcpRslt);
#endif
					MeltHp();
					}
				break;
				}
			}
		}
#ifdef XBZTEST
	CommSzNum(SzShared("PopRtfState bStacklim after actions: "), (**hribl).bStackLim);
	CommSzNum(SzShared("PopRtfState brstn.bPrev after actions: "), rstn.bPrev);
#endif

	goto LNoError;

LSkipActions:

	ErrorNoMemory(eidNoMemOperation);
LNoError:

	if (rstn.bPrev <= 1)
		{
		int cb;
		CP cpSectLast = cpNil;

		caT.cpLim = CpMacDoc(caT.doc = (**hribl).doc);
		caT.cpFirst = caT.cpLim - ccpEop;
		CachePara(caT.doc, caT.cpFirst);
		if (caPara.cpFirst < caT.cpFirst)
			{
			if ((**hribl).fPermOK)
				vpapFetch.stc = (**hribl).rgbStcPermutation[vpapFetch.stc];
			else 
				vpapFetch.stc = stcNormal;
			if (cb = CbGrpprlFromPap(fTrue, rgb, &(**hribl).pap, &vpapFetch, fFalse))
				ApplyGrpprlCa(rgb, cb, &caT);
			}

#ifdef BZ
		CommSzRgNum(SzShared("end doc caT: "), &caT, 5);
		CommSzRgNum(SzShared("end doc caSect: "), &caSect, 5);
		CommSzLong(SzShared("end doc cpMacDoc: "), CpMacDoc(caT.doc));
#endif
		/* if last char is section mark, we will later replace it with
		   a paragraph mark. It is possible that the values in
           the hribl sep are wrong, particularly the ihdt, as in
           when the last char is {\sect} and the previous sect had headers.
           So use the vsepFetch of the final section in that case.
           Actually, the grpfihdt should be 0, indicating no separate headers
           except in the even weirder case of no text following \sect but
           headers/footers emitted. Icheck whether we actually got headers
           or footers; if not we blow them away.
         */
		if (caT.cpFirst > cp0)
			{
			FetchCpAndPara(caT.doc, caT.cpFirst - 1, fcmChars);
			if (vhpchFetch[0] == chSect)
				{
				CacheSect(caT.doc, caT.cpFirst - 1);
				if (caSect.cpLim == caT.cpFirst)
					{
					Assert (vcpFetch == caT.cpFirst - 1);
					cpSectLast = vcpFetch;
                    (**hribl).sep = vsepFetch;
                    if (!(**hribl).fHdr && !(**hribl).fFtr)
                         (**hribl).sep.grpfIhdt = 0;
#ifdef BZ
					CommSzLong(SzShared("cpLastSect: "), cpSectLast);
		            CommSzRgNum(SzShared("end doc sep: "), &vsepFetch, cwSEP);
#endif
					}
				}
			}

		CacheSectSedMac(caT.doc, caT.cpFirst);
		caPara.doc = docNil;
		caSect.doc = docNil;

		/* ensure that the section props get attached to the hidden section mark
		at document end */
		caT.cpFirst+=ccpEop;
		caT.cpLim+=ccpEop;

          /* note ribl sep may have been modified if chSect was last char */
		if (cb = CbGrpprlProp(fTrue, rgb, cchPapxMax,&(**hribl).sep,
				&vsepFetch, cwSEP - 1, mpiwspxSep, 0 /* mpiwspxSepW3 */))
			{
#ifdef DRTFSEP
			CommSzRgNum(SzShared("bef applygrpprl vsepFetch: "), &vsepFetch, cwSEP);
#endif
			ApplyGrpprlCa(rgb, cb, &caT);
			}

		if (cpSectLast != cpNil) /* last char is chSect */
			{
			struct PAP pap;
			PapForEndSecPara(hribl, &pap);

			  /* insert eop at end, then delete chSect */
	        FInsertRgch(caT.doc, CpMacDocEdit(caT.doc), rgchEop, (int)ccpEop,
				&(**hribl).chp, &pap);
			FDelete(PcaSetDcp(&caT, caT.doc, cpSectLast, (CP)1));
			}

		/* if auto page numbers for last sect, apply */
		DoPageNum(hribl, CpMacDocEdit(caT.doc));
		}

	grpfIhdt = (**hribl).sep.grpfIhdt;
	RetrPropFromRtfStack((**hribl).rgbChar, hribl, pgcChar);
	RetrPropFromRtfStack((**hribl).rgbPara, hribl, pgcPara);
	RetrPropFromRtfStack((**hribl).rgbSect, hribl, pgcSect);
	RetrPropFromRtfStack((**hribl).rgbTable, hribl, pgcTable);

#ifdef XBZTEST
	CommSzNum(SzShared("PopRtfState bStacklim after RetrProp: "), (**hribl).bStackLim);
#endif

	if ((**hribl).bStackLim > 1)
		{
		if (rdsOld == rdsHdr && !fDopHdrs)
			(**hribl).sep.grpfIhdt = grpfIhdt;

			/* note: this code is slightly incorrect. If we are at the end of the fonttbl
			we should scan ALL of the stored CHP's stored in the stack and map their
			ftcs to their new value. */
		else  if (rdsOld == rdsFonttbl && (**hribl).fFonttblDef)
			{
			int *pftc = &((**hribl).chp.ftc);

			FreezeHp();
			if (!FSearchFtcmap((**hribl).hftcmap, *pftc, pftc, fFalse /* fAdd */))
				*pftc = ftcMapDef;
			MeltHp();
			}
		else  if (rdsOld == rdsField && (**hribl).fFldImport)
			{

			/* for pc word tiff conversions: they force line spacing
				   to be negative. This will truncate any picture. So if char
				   set is pc and line spacing is negative for an import field,
				   make that paragraph's spacing its absolute value. The new
				   prop will be set at para end. Need to do this after
				   the state is popped	   bz
				*/
			(**hribl).fFldImport = fFalse;
			if ((**hribl).chs == chsPC)
				{
				(**hribl).pap.dyaLine = abs((**hribl).pap.dyaLine);
#ifdef BZ
				CommSzNum(SzShared("PopRtfState pap.dyaLine after fldImport: "), (**hribl).pap.dyaLine);
#endif
				}
			}

		(**hribl).rds = rstn.rds;
		(**hribl).bStackLim = rstn.bPrev;
		(**hribl).hrcpfld = rstn.hrcpfld; /* used by fields */
		(**hribl).cpRdsFirst = rstn.cpFirst;

		AssertDo(FChngSizeHCw((**hribl).hrgbStack,
				CwFromCch((**hribl).bStackLim), fTrue));
		}

#ifdef XBZTEST
	CommSzNum(SzShared("PopRtfState bStacklim after ChngSize: "), (**hribl).bStackLim);
#endif

	if ((**hribl).bStackLim <= 1) /* true when all rtf input done */
		{
		(**hribl).ris = risExit;
		DoSpecDocProps(hribl);
		}

	if ((**hribl).ris != risExit && (stcNew = (**hribl).pap.stc) != stcOld)
		{
		switch (rdsOld)
			{
			/* don't do for any dest before text begins */
		case rdsStylesheet:
		case rdsFonttbl :
		case rdsColortbl:
		case rdsComment :
		case rdsInfoBlock:
		case rdsTitle   :
		case rdsSubject :
		case rdsAuthor  :
		case rdsKeywords:
		case rdsDoccomm :
		case rdsOperator:
		case rdsCreatim :
		case rdsRevtim  :
		case rdsPrintim :
		case rdsVersion:
		case rdsTemplate:
			break;
		default:
				{
				if ((**hribl).fPermOK)
					stcNew = (**hribl).rgbStcPermutation[stcNew];
				else
					stcNew = stcNormal;
				ApplyStcEndOfDoc((**hribl).doc, stcNew);
				}
			}
		}
}



/*  %%Function:  FFinishFtnAtn  %%Owner:  bobz       */

FFinishFtnAtn(hribl, edc)
struct RIBL **hribl;
int edc;
{
	int docSub;
	int doc = (**hribl).doc;
	struct PLC **hplcSubRd;
	struct PLC **hplcSubNd;
	CP cpSub;
	CP cpText;
	struct FRD frd;
	struct ATRD atrd;
	CP cpRefNew;
	WORD *pw;
	struct PAP pap;


	(**hribl).pap.fInTable = (**hribl).pap.fTtp = fFalse;
	DoRtfParaEnd(hribl, chEop);

	/*
	error recovery here: I think that if we fail as
	set up, the footnote/annot text will be left in the document proper, which
	isn't so bad... bz
	*/

	if ( (docSub = DocSubEnsure(doc, edc)) == docNil)
		{
		ReportSz("Warning - No footnote/annotation doc - data left in doc");
		return fFalse;
		}

	if (edc == edcDrpFtn)
		{
		hplcSubRd = PdodDoc(doc)->hplcfrd;
		hplcSubNd = PdodDoc(docSub)->hplcfnd;
		pw = &frd;
		}
	else
		{
		Assert (edc == edcDrpAtn);
		hplcSubRd = PdodDoc(doc)->hplcatrd;
		hplcSubNd = PdodDoc(docSub)->hplcand;
		pw = &atrd;
		}

	vifnd = IMacPlc(hplcSubRd);

	/* insert footnote or annotation reference */

	cpRefNew = (**hribl).cpRdsFirst - 1;

	if (edc == edcDrpFtn)
		{
		/* look at the reference for the footnote in the main doc. set fAuto in the
		frd if the reference is the footnote special character. */
		FetchCp(doc, cpRefNew, fcmChars);
		SetBytes(&frd, 0, cbFRD);
		frd.fAuto = (*vhpchFetch == chFootnote) ? fTrue : fFalse;
		}
	else
		{
		/* insert user initials for the chatn char or a null
		st if we didn't get initials   */

		Assert (cpRefNew >= cp0);
		Assert((**hribl).stUsrInitl[0] < ichUsrInitlMax);
		CopySt((**hribl).stUsrInitl, &atrd);
		}

	if (!FInsertInPlc(hplcSubRd, vifnd, cpRefNew, pw))
		{
		ReportSz("Warning - Can't insert ftn/atn in plc - data left in doc");
		return fFalse;
		}
	cpSub = CpPlc( hplcSubNd, vifnd );
	if (!FOpenPlc(hplcSubNd, vifnd, 1))
		{
		ReportSz("Warning - Can't open ftn/atn plc - data left in doc");
		return fFalse;
		}
	PutCpPlc( hplcSubNd, vifnd, cpSub );
	/* we temporarily push the cp of the next element so that it is adjusted
	properly when we add the footnote text. */
	PutCpPlc( hplcSubNd, vifnd + 1, CpPlc( hplcSubNd, vifnd + 1) + 1 );
	cpText = cpRefNew + 1;
	/* if no para mark, insert one. Code assumes that a ChEop
			is preceded by a ChReturn, so that finding no ChEop
			means no crlf was present */
	if (ChFetch(doc, CpMacDocEdit(doc) - 1, fcmChars) != chEop)
		{
		/* it is safe to pass a CHP stored on the heap to FInsertRgch but not a PAP */
		pap = (**hribl).pap;
		FInsertRgch(doc, CpMacDocEdit(doc), rgchEop,
				(int)ccpEol, &(**hribl).chp, &pap);
		}

		{
		struct CA caDel;
		struct CA caT;

		PcaPoint( &caT, docSub, cpSub );
		PcaSet( &caDel, doc, cpText, CpMacDocEdit(doc));
		FReplaceCps(  &caT, &caDel );
		FDelete( &caDel );
		}
	/* now we back off from the adjustment since we've added the text. */
	PutCpPlc( hplcSubNd, vifnd + 1, CpPlc( hplcSubNd, vifnd + 1) - 1 );

	return fTrue;
}


/* overcome slm bogosities */
#include "rtfin2.c"


#ifdef PROFILE
/*  this is here so that appended native code does not appear in previous
	function in pcode profiles. */
Rtfin_Last(){}
#endif /* PROFILE */

/* ADD NEW CODE *ABOVE* Rtfin_Last() */
