/* Clipbrd2.c -- less frequently used clipboard routines */

#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOCTLMGR
#define NOFONT
#define NOPEN     
#define NOBRUSH
#define NOSCROLL
#define NOCOMM
#define NOWNDCLASS
#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "doc.h"
#include "props.h"
#include "sel.h"
#include "pic.h"
#include "file.h"     
#include "disp.h"      
#include "prm.h"
#include "format.h"
#include "dde.h"
#include "field.h"
#include "ch.h"
#include "inter.h"
#include "fkp.h"
#include "winddefs.h"
#include "screen.h"
#include "debug.h"
#include "error.h"
#include "message.h"
#include "ff.h"
#include "grstruct.h"
#include "resource.h"

#ifdef PROTOTYPE
#include "clipbrd2.cpt"
#endif /* PROTOTYPE */

extern char             (**vhgrpchr)[];
extern struct SEL       selCur;      /* Current selection */
extern struct DOD       **mpdochdod[];
extern CP               vcpLimParaCache;
extern struct MERR      vmerr;
extern HWND             vhwndApp;          /* handle to parent's window */
extern HWND				vhwndCBT;
extern struct SAB       vsab;
extern struct PREF      vpref;
extern struct WWD       **hwwdCur;
extern struct FLI       vfli;
extern CHAR             rgchEop[];
extern int              cfRTF;
extern int              cfPrPic;
extern int              cfLink;
extern struct PAP		vpapFetch;
extern struct FKPD      vfkpdText;
extern int              vdocScratch;

extern CHAR HUGE        *vhpchFetch;
extern CP               vcpFetch;
extern struct CHP       vchpFetch;
extern BOOL             vfSingleApp;
extern struct PIC       vpicFetch;
extern FC               vfcFetchPic;    /* fc of last fetched pe */
extern int              fnFetch;
extern char	szEmpty[];
extern struct SCI       vsci;
extern int              vwWinVersion;
extern int				wwCur;

extern LPCH LpchIncr();

VOID BuildMFRecordForDIB(struct MFHDRDIBMF *, struct WINMRTAG *, unsigned long, int, int);
int CbDIBHeader(LPBITMAPINFOHEADER);
#define WIDTHBYTES(i)   (((i)+31)/32*4)      /* ULONG aligned ! */

/*  %%Function:  CchReadLineExt  %%Owner:  bobz       */


CchReadLineExt( lpch, cbRead, rgch, pfEol)
LPCH    lpch;
int     cbRead;
CHAR    rgch[];
int     *pfEol;
{ /* Read from lpch to the next eol or null terminator, whichever comes first */
	/* Return number of bytes read (max is 255) and whether there is a eol at end */
	/* The count does not include the null terminator, but does include the eol */

	CHAR    *pch;

	bltbx( lpch, (LPCH) rgch, cbRead );
	rgch[ cbRead ] = 0;       /* Null terminate the string (so index will work) */

	if (*pfEol = ((pch=index(rgch,chEol)) != NULL))
		{   /* FOUND EOL */
		return (pch - rgch + 1);
		}
	else
		{   /* NO EOL */
		return CchSz(rgch) - 1;
		}
}


/*  %%Function:  FRenderFormat  %%Owner:  bobz       */


FRenderFormat( cf )
int cf;
{       /* Render clipboard data in format specified by cf */

	BOOL fRet = fTrue;
	CP cpMac = CpMacDocEdit(docScrap);

	Assert (cf != CF_OWNERDISPLAY);

	if (FCanWriteCf (cf, docScrap, cp0, cpMac, fFalse /*fRestrictRTF */))
		if ( !(fRet = FWriteExtScrap (cf)) )
			ErrorEid( eidClipNoRender, " RenderFormat" );
	return (fRet);
}


/*  %%Function:  FRenderAll  %%Owner:  bobz       */


FRenderAll(ac)
	{   /* Opus is going away, and we are the owners of the clipboard.
		Render the contents of the clipboard in as many formats as
		we know.  Returns false if we failed to render some format
	and the user says do not throw away the clipboard.    */

	CP cpMac = CpMacDoc( docScrap ) - ccpEop;

	int cf = cfNil;
	int id;
	BOOL fRet = fTrue;

	Assert (vsab.fOwnClipboard);
	if ( (cpMac == cp0) || !vsab.fOwnClipboard)
		{   /* We are not the clipboard owner OR the scrap is empty:
						no actions required */
		return fTrue;
		}
	/* do you want to render this biggie? */
	/* pic will usually take a while to render */
	/* later version bz may want better dialog that allows selective
		abandonment of clipboard formats */

	if (cpMac > 1024L || vsab.fPict)
		{
		switch (ac)
			{
		case acSaveAll:
		case acQuerySave:
			id = IdMessageBoxMstRgwMb(mstSaveLargeClip, 
					NULL, MB_DEFNOQUESTION);
			break;

		case acSaveNoQuery:
			id = IDYES;
			break;

		case acNoSave:
			id = IDNO;
			break;
			}

		if (id == IDCANCEL)
			return(fFalse); /* cause quit to be abandoned */
		else  if (id == IDNO)
			return(fTrue);  /* no render, but continue quit */
		/* YES continues to do rendering. */
		}

	/* Get the handle for the highest-priority type available */

	if (OpenClipboard( vhwndApp ))
		{
		StartLongOp();

        /* We want to clear out previous data formats in the clipboard.
    	Unfortunately, the only way to do this is to call EmptyClipboard(),
    	which has the side effect of calling us with a WM_DESTROYCLIP
    	message. We use this primitive global comunication to prevent
    	docScrap from being wiped out in DestroyClip() */

	    vsab.fDontDestroyClip = fTrue;
	    EmptyClipboard();
	    vsab.fDontDestroyClip = fFalse;

		while ((cf = CfNextCf(cf,fFalse)) != cfNil)
			{
			if (FCanWriteCf(cf, docScrap, cp0, cpMac, fTrue /*fRestrictRTF */))
				{
				if (!FWriteExtScrap(cf))
					{
					/* avoid other out of memory messages */
					if (vmerr.fMemFail)
						vmerr.mat = matNil;
					if (IdMessageBoxMstRgwMb(mstClipLarge, NULL, 
							MB_OKCANCEL|MB_ICONQUESTION) == IDOK)
						{
						fRet = fTrue;
						break;    /* out of while loop */
						}
					else
						{
						fRet = fFalse;
						break;
						}
					}
				}
			}
		CloseClipboard();
		EndLongOp (fFalse /* fAll */);
		}
	return (fRet);
}



/* F  W R I T E  E X T  S C R A P */
/*  Write the scrap to the clipboard in format cf.
*/
/*  %%Function:  FWriteExtScrap  %%Owner:  bobz       */

FWriteExtScrap(cf)
int cf;
{

	HANDLE h;
	int fBlankPic = fFalse;

	Assert(vsab.fOwnClipboard);

	/* render in format cf */
	h = HDataWriteDocCps (cf, docScrap, cp0, 
			CpMacDoc (docScrap) - cchEop, 0, &fBlankPic);

	if (h != NULL)
		/* data sucessfully written */
		{
		SetClipboardData(cf, h);
		return fTrue;
		}
	else  if (fBlankPic) /* no data, but ok */
		{
		return fTrue;
		}
	else
		return fFalse;
}





/* H  D A T A  W R I T E  D O C  C P S */
/*  Write data from doc [cpFirst:cpLim) to a windows global handle.  Prepend
	to that handle cpInitial bytes.  Write in format cf.  Return NULL if
	operation cannot be completed for what ever reason.
*/
/*  %%Function:  HDataWriteDocCps   %%Owner:  bobz       */


HDataWriteDocCps (cf, doc, cpFirst, cpLim, cbInitial, pfBlankPic)
int cf, doc;
CP cpFirst, cpLim;
int cbInitial;
int *pfBlankPic;  /* set true if an empty picture; else ignored */

{
	struct CA ca;

	AssureLegalSel (PcaSet(&ca, doc, cpFirst, cpLim));
	switch (cf)
		{
	case CF_TEXT:

		return HWriteText (ca.doc, ca.cpFirst, ca.cpLim, cbInitial);

	case CF_BITMAP:
	case CF_METAFILEPICT:
	case CF_DIB:
		return HWritePict (ca.doc, ca.cpFirst, ca.cpLim, cbInitial, 
				pfBlankPic, cf);

	default:
		if (cf == cfRTF)
			return HWriteRTF (ca.doc, ca.cpFirst, ca.cpLim, cbInitial);
		else  if (cf == cfLink && FCanLinkScrap())
			{
			Assert (doc == docScrap && cbInitial == 0);
			return HRenderClipLink ();
			}
		else
			return NULL;
		}
}



/* H  W R I T E  T E X T */
/*  Write doc, cpFirst:cpLim to a windows handle in plain text format.
	Plain text format is generated by doing successive FormatLines and
	adding CRLF pairs at the end of each. cbInitial is data prepended to
	the handle's data for use by DDE (& DDE only).

	Note we try to allocate fairly big chunks, but a least enough for
	data and terminating null.
*/
/*  %%Function:  HWriteText   %%Owner:  bobz       */


HWriteText (doc, cpFirst, cpLim, cbInitial)
int doc;
CP cpFirst, cpLim;
int cbInitial;

{

	int wAlloc = cbInitial ? GMEM_DDE : GMEM_MOVEABLE;
	HANDLE h = hNil;
	HANDLE hT;
	LPCH lpch;
	CP cpNow=cpFirst;
	int cLine = 0;
	struct WWD *pwwd;
	GRPFVISI grpfvisi;
	unsigned LONG lcbMac = 0;  /* amount used */
	unsigned LONG lcbMax = 0;  /* amount allocated */
	BOOL fTableCell;
	int cchEopT;

	if (!FAssureHCbGlob( &h, (long)(1+cbInitial), wAlloc, &lcbMac, &lcbMax) )
		goto Failed;

	/* Set up view preferences for rendered text in wwTemp */

	grpfvisi.w = 0;
	grpfvisi.fvisiCondHyphens = fTrue;
	grpfvisi.grpfShowResults = ~0;
	grpfvisi.fNoShowPictures = fTrue;
	grpfvisi.fForceField = fTrue;
	grpfvisi.flm = FlmDisplayFromFlags( vpref.fDisplayAsPrint, vpref.fPageView, vpref.fDraftView);
	PwwdWw(wwTemp)->grpfvisi = grpfvisi;
	LinkDocToWw( doc, wwTemp, wwNil );

	while (cpNow < cpLim)
		{
		int dcpLine;

		fTableCell = fFalse;

		/* Format a line of text for the screen */

		FormatLine( wwTemp, doc, cpNow );
		dcpLine = vfli.ichMac;

		/* Put the chars + a CRLF into the handle */

		if (vfli.cpMac > cpLim)
			/* do not copy unwanted portions of the line */
			dcpLine = IchFromCpVfli (cpLim);

		/* check for tables */
		else if (vfli.fSplatBreak && vfli.chBreak == chTable)
			{
			CachePara(doc, cpNow);
			if (vpapFetch.fTtp)	/* End-of-row marker */
				{
				if (lcbMac - 1 > cbInitial) /* back over preceding tab */
					lcbMac--;
				dcpLine = 0;
				}
			else
				fTableCell = fTrue;
			}

		dcpLine = CchSanitizeRgch(vfli.rgch, dcpLine, ichMaxLine /* sizeof vfli.rgch */, fTrue /*fSpecToCRLF*/);
		if (fTableCell)
			vfli.rgch[dcpLine-1] = chTab;

		/* extra byte for null at end already allocated */
		if (!FAssureHCbGlob(&h, (LONG) (lcbMac + dcpLine +
				(cchEopT = (fTableCell ? 0 : cchEop))), wAlloc, &lcbMac, &lcbMax))
			goto Failed;

		if ((lpch=GlobalLockClip( h )) == NULL)
			goto Failed;

		bltbx( (LPCH) vfli.rgch, lpch+lcbMac-dcpLine-cchEopT-1, dcpLine );
		if (!fTableCell)
			{
			bltbx( (LPCH) rgchEop, lpch+lcbMac-cchEop-1, cchEop );
			cLine++;
			}

		Assert (vfli.cpMac > cpNow);
		cpNow = vfli.cpMac;
		GlobalUnlock( h );
		}

	/* SUCCEEDED!  NULL-terminate the string before returning the handle */

	/* now realloc to free up any extra allocated memory */
	hT = h;
	if (lcbMax != lcbMac)
		{
		Assert (lcbMax > lcbMac);
		if ((h = OurGlobalReAlloc( h, (LONG) (lcbMac), wAlloc )) == NULL)
			{
			h = hT;    /* this should never happen */
			goto Failed;
			}
		}

	if ((lpch = GlobalLock( h )) == NULL)
		goto Failed;

	if (cLine == 1 && !fTableCell)
		/* Special case: < 1 line, do not terminate w/ CRLF */
		/* Back up over crlf already written */
		lcbMac -= cchEop;

	lpch [lcbMac-1] = '\0';
	GlobalUnlock( h );

	InvalFli();

	return h;

Failed:

	if (h != NULL)
		GlobalFree( h );

	InvalFli();

	return NULL;
}



/* H  W R I T E  P I C T */
/*  Write doc, cpFirst:cpLim to a windows handle in bitmap or metafile
	format.
	cbInitial is data prepended to
	the handle's data for use by DDE (& DDE only).
*/
/*  %%Function:  HWritePict   %%Owner:  bobz       */


HWritePict (doc, cpFirst, cpLim, cbInitial, pfBlankPic, cf)
int doc;
CP cpFirst, cpLim;
int cbInitial;
int *pfBlankPic;  /* set true if an empty picture; else ignored */
int cf; /* requested clipboard format */

{

	extern LPCH LpchIncr();

	int wAlloc = cbInitial ? GMEM_DDE : GMEM_MOVEABLE;
	HANDLE hData = NULL;
	HANDLE hDataDescriptor = NULL;
	HANDLE hReturn = NULL;
	LPCH lpch;
	FC fc, fcMacPic;
	long cfcPic;
	long lcbCur;
	int cLine = 0;
	int wPrefT;
	char rgch[256];
	struct CA caT;
	CP cpImport;
	int mm; /* mapping mode */
	FC fcFetchPic;
	BOOL fDIBInMf = fFalse;
	
	  /* for import field, vcpFetch will end up being cpImport */

	AssertDo(FCaIsGraphics(PcaSet(&caT, doc, cpFirst, cpLim),
			fvcResults, &cpImport));

	/* load in the picture prefix to find length */
	Assert(*vhpchFetch == chPicture && vchpFetch.fSpec);
	/* FetchPe will set vchpFetch.fnPic to fnFetch bz */

	/* fill PIC structure vpicFetch */

	FetchPe(&vchpFetch, doc, vcpFetch);
	/* size of pic bytes */
	cfcPic = vpicFetch.lcb - vpicFetch.cbHeader;

	if (cfcPic == 0L)
		/* no pain, no gain */
		{
		*pfBlankPic = fTrue;
		return(NULL);
		}

	if (cf == CF_DIB) 
		{
		// we can render a DIB if it is one of those that are embedded
		// in a metafile 
		if (FDIBInMetafile())
			{
			/* DIB is embedded in a metafile, have to extract it out */
			/* set up cfcPic, fcFetchPic correctly */
			cfcPic -= (cbMFHDRDIBMF + cbWINMRTAG);	/* total overhead of meta records */
			fcFetchPic = vfcFetchPic + cbMFHDRDIBMF; /* advance to beyond the meta hdr record */ 
			mm = MM_DIB;
			}
		else
			goto Failed;
		}
	else if (cf == CF_BITMAP && vwWinVersion >= 0x0300 && FDIBInMetafile())
		{
		// we can also render a bitmap from those metafile that embed a DIB
		// but we only want to do that under win 3.x because win 2 does not
		// have the API to do so
		fDIBInMf = fTrue;
		cfcPic -= (cbMFHDRDIBMF + cbWINMRTAG);	/* total overhead of meta records */
		fcFetchPic = vfcFetchPic + cbMFHDRDIBMF; /* advance to beyond the meta hdr record */ 
		mm = MM_BITMAP;
		}
	else
		{
		/* init mm and fcFetchPic normally */
		mm = vpicFetch.mfp.mm;
		fcFetchPic = vfcFetchPic;
		}

	hData = GlobalAlloc2( wAlloc, cfcPic );
	if (hData == NULL)
		goto Failed;
	if ((lpch = GlobalLock (hData)) == NULL)
		goto Failed;

	/* pull pic bytes out of fn, fc */

	fcMacPic = fcFetchPic + cfcPic;
	for (fc = fcFetchPic ; fc < fcMacPic;
		fc += lcbCur, lpch = LpchIncr(lpch, (unsigned)lcbCur))
		{
		FetchPeData(fc, (char HUGE *)rgch,
				/* using CpMin for missing lmin */
		lcbCur = CpMin((long)(sizeof (rgch)), fcMacPic - fc));
		bltbx((char far *)rgch, lpch, (int)lcbCur);
		}

	GlobalUnlock (hData);

	/* Now we have the whole of picture in a windows Global handle */
	/* See what kind of picture we have */

  	switch (mm)
		{
		default:
			if (vpicFetch.mfp.mm >= MM_META_MAX)
				goto Failed;
			if ( ((hDataDescriptor=OurGlobalAlloc(wAlloc,
					(long)sizeof(METAFILEPICT) ))==NULL) ||
					((lpch=GlobalLock( hDataDescriptor ))==NULL))
				{
				goto Failed;
				}
			else
				{
				vpicFetch.mfp.hMF = hData;
				  //asserts to allow use of bltbx
				Assert (sizeof(METAFILEPICT) < 65535);
				Assert ((unsigned)(-LOWORD(lpch)) >= (unsigned)sizeof(METAFILEPICT) ||
					(unsigned)(-LOWORD(lpch)) == 0);
				bltbx( (LPCH) &vpicFetch.mfp, lpch, sizeof(METAFILEPICT));
				GlobalUnlock( hDataDescriptor );
				}
			break;

		case MM_DIB:
			hDataDescriptor = hData;
			break;  // should just need to pass the data to clip
				
		case MM_BITMAP:
			{
			int yp1mm, xp1mm;

			if (fDIBInMf) // this is the case where we convert a DIB to a bitmap
				{
				// what we got in hData now are the DIB header/color table 
				// and DIB bits

				int cbInfoHdr;
				LPCH lpBits;
				HANDLE hGDI;
				FARPROC lpfn;

				if ((hGDI = GetModuleHandle(SzShared("GDI"))) == NULL)
					goto Failed;

				lpfn = GetProcAddress(hGDI, MAKEINTRESOURCE(idoCreateDIBitmap));
				Assert(lpfn != NULL);

				if ((lpch = GlobalLock(hData)) == NULL)
					{
					goto Failed;
					}

				if (((LPBITMAPINFOHEADER)lpch)->biCompression != BI_RGB)
					{
					// Don't handle DIB that is compressed yet
					GlobalUnlock(hData);
					goto Failed;
					}

				cbInfoHdr = CbDIBHeader(lpch);
				lpBits = LpchIncr(lpch, cbInfoHdr);

				Assert(wwCur != wwNil);
				Assert(PwwdWw(wwCur)->hdc != NULL);
				// calling CreateDIBitmap (a Win 3 call) through GetProcAddress 
				// because we still have to run under Win 2
				if ((hDataDescriptor = (*lpfn)(PwwdWw(wwCur)->hdc, 
					(LPBITMAPINFOHEADER)lpch, (DWORD)CBM_INIT, 
					(LPSTR)lpBits, (LPBITMAPINFO)lpch, DIB_RGB_COLORS)) == NULL)
					{
					GlobalUnlock(hData);
					goto Failed;
					}
				}
			else if ( ((lpch=GlobalLock( hData ))==NULL) ||
					(vpicFetch.bm.bmBits=lpch,
					((hDataDescriptor=
					CreateBitmapIndirect((LPBITMAP)&vpicFetch.bm))==NULL)))
				{
				if (lpch != NULL)
					GlobalUnlock( hData );
				goto Failed;
				}

			// now we got hDataDescriptor being the handle to the bitmap 

			// Specify the "goal size" for this guy 
			// convert twips back to .1mm units.
			LogGdiHandle(hDataDescriptor, 1028);
			xp1mm = NMultDiv (vpicFetch.dxaGoal, 100, czaCm);
			yp1mm = NMultDiv (vpicFetch.dyaGoal, 100, czaCm);
			SetBitmapDimension( hDataDescriptor, xp1mm, yp1mm);
#ifdef XBZTEST
			{
			long lT;

			lT = GetBitmapDimension( hDataDescriptor );
			CommSzNum(SzShared("FWritePict bitmap dimension x: "), LOWORD( lT ));
			CommSzNum(SzShared("FWritePict bitmap dimension y: "), HIWORD( lT ));
			}
#endif /* XBZTEST */


			GlobalUnlock( hData );
			GlobalFree( hData );   /* Bitmap was copied by CreateBitmapIndirect,
						don't need it anymore */
			hData = NULL;
			break;
			}
		} /* switch (mm) */



	/* Scaling/cropping note: the scaling and cropping info is
			maintained within Opus; there is no good way to render only
			a portion of the bitmap or metafile, so the entire picture is
			given to the clipboard. bz 12/23/87
		*/

	/* here, hDataDescriptor has the bitmap or metafile. If this
			if for DDE (cbInitial != 0), then we must allocate another
			block that includes the cbInitial data and hDataDescriptor
		*/

	if (!cbInitial)
		return hDataDescriptor;
	else
		{
		hReturn = GlobalAlloc2( wAlloc,
				(long)(cbInitial + sizeof (HANDLE)) );
		if (hReturn != NULL)
			{
			if ( ( lpch=GlobalLock(hReturn) ) != NULL )
				{
				bltbx( (LPCH) &hDataDescriptor, LpchIncr(lpch, cbInitial),
						sizeof(HANDLE) );
				GlobalUnlock( hReturn );
				return hReturn;
				}
			}
		/* fall through intentional */
		}


Failed:
	if (hDataDescriptor != NULL)
		{		  
		if (vpicFetch.mfp.mm == MM_BITMAP || fDIBInMf)
			{
			UnlogGdiHandle(hDataDescriptor, 1028);
			DeleteObject (hDataDescriptor);
			}
		else  
			// metafile, dib in metafile
			GlobalFree(hDataDescriptor);
		}

	if (hData != NULL)
		GlobalFree( hData );

	if (hReturn != NULL)
		GlobalFree( hReturn);

	return NULL;
}




/* I C H  F R O M  C P  V F L I */
/*  Returns the ich of the character in vfli.rgch representing cp, or
	the first represented character after cp.
*/
/*  %%Function:  IchFromCpVfli   %%Owner:  bobz       */

IchFromCpVfli (cp)
CP cp;

{
	CP cpCur = vfli.cpMin;
	int ich, dch;
	struct CHR *pchr;

	pchr = &(**vhgrpchr)[0];
	ich = 0;
	while (cpCur < cp)
		{
		dch = pchr->ich - ich;
		if (cpCur + dch >= cp)
			return ich + cp - cpCur;
		cpCur += dch;
		ich += dch;
		if (pchr->chrm == chrmVanish)
			cpCur += ((struct CHRV *)pchr)->dcp;
		else  if (pchr->chrm == chrmDisplayField)
			{
			ich++;
			cpCur += ((struct CHRDF *)pchr)->dcp;
			}
		/* result is garbage with formulas!
			PETER - do you know why this is? Before we call this we
			are setting grpfVisi.fForceField to make everything show up in
			instructions mode. Isn't a formula just a field? Why would
			this be a problem? (bz)
			BOB - the problem is that we are asking for (and getting) the
			result of the field, not the instructions.  Formula results,
			if viewed as a stream (which we are doing here), may not be very
			meaningful.  We probably should leave it the way it is, but
			first look at what kind of result we are getting and see just how
			bad it is. (pj)
		
			RE: from bob. We do get funny chars, but we call SanitizeRgch on
			the result, so non-printing chars become . and some others get
			cleaned up. I think this is about the best we can do unless we
			put out field instructions rather than results, which is generally
			less desirable.
			*/
		Assert (ich < vfli.ichMac);
		Assert (pchr->chrm);
		Assert (pchr->chrm != chrmEnd);
		(char *)pchr += pchr->chrm;
		}
	return ich;
}



/* F  R E A D  E X T  S C R A P */
/*  %%Function:  FReadExtScrap  %%Owner:  bobz       */

int FReadExtScrap()
	{       /* Transfer the external scrap to the scrap document.  This means:
		read the contents of the clipboard into docScrap, using whatever
	available standard format we can process. Return fFalse=ERR, fTrue=OK */

	HANDLE hClipboard;
	int cf = cfNil;
	BOOL fOk = fFalse;
	struct CA ca;

#ifdef DEBUG
	/* used for testing paste - forces us to paste in from clipboard
		even if we owned it.  Set in SlapWnd and for debugging, turned off when
		we load something into the clipboard.
	*/
	extern int vTestClip;

	Assert( !vsab.fOwnClipboard  || vTestClip);
#endif /* DEBUG */

	if ( !OpenClipboard( vhwndApp ) )
		{
#ifdef BZ
		CommSzSz(SzShared("FReadExtScrap failure: OpenClipboard"), szEmpty);
#endif
		return fFalse;
		}

	vsab.fMayHavePic = vsab.fExt = vsab.fPict = vsab.fBlock = 
			vsab.fFormatted = fFalse;
	vsab.fExtEqual = fTrue;


	/* FReadHdata will clear out any previously set styles/fonts */

	/* Get the handle for the highest-priority type available */
	while ((cf = CfNextCf (cf,fFalse)) != cfNil)
		{
		hClipboard = GetClipboardData (cf);

#ifdef BZ
		CommSzNum(SzShared("FReadExtScrap cf: "), cf);
		CommSzNum(SzShared("FReadExtScrap hClipboard from GetClipboardData: "),
				hClipboard);
#endif

		if (hClipboard != NULL && 
				FReadHData (hClipboard, cf, PcaSetWholeDoc(&ca,docScrap),0))
			{
			/* data sucessfully read into docScrap */
			fOk = fTrue;
			if (cf == CF_BITMAP || cf == CF_METAFILEPICT || cf == CF_DIB)
				{
				vsab.fMayHavePic = vsab.fPict = fTrue;
				}
			if (cf != CF_TEXT)
				vsab.fFormatted = fTrue;
			break;
			}
#ifdef BZ
		else
			CommSzSz(SzShared("FReadExtScrap failure: FReadHData"), szEmpty);
#endif
		}
	/* styles/fonts are either put into docScrap with RTF or do not
			differ from standard. At any rate, docScrap contains its fonts
			and styles at this point, and it does not come from any
			outside doc.
		*/
	vsab.docStsh = docNil;
	CloseClipboard();

	return fOk;

}


/* F  R E A D  H  D A T A */
/*  Read data from hData to replace text in doc at cpFirst:cpLim.
	Text is in format cf.  Skip the first cbInitial bytes.
	Return fFalse if the text cannot be read in format cf or in case
	of other error.
*/
/*  %%Function:  FReadHData   %%Owner:  bobz       */


FReadHData (hData, cf, pca, cbInitial)
HANDLE hData;
int cf;
struct CA *pca;
int cbInitial;
{
	int docDest;
	BOOL fOk;
	struct CA ca;

#ifdef BZ
		CommSzNum(SzShared("FReadHData cf: "), cf);
#endif

	switch (cf)
		{
	case CF_TEXT:
	case CF_DIB:
	case CF_BITMAP:
	case CF_METAFILEPICT:
		break;
	default:
		if (cf == cfPrPic && cfPrPic != cfNil)
			/* Excel printer metafile format */
			{
			cf = CF_METAFILEPICT;
			break;
			}

		if (cf != cfRTF)
			{
			return fFalse;
			}
		}

	/* get document to put text into */
	if (pca->doc == docScrap)
		/* if pasting to scrap, use docScrap directly */
		docDest = docScrap;
		/* otherwise use vdocScratch */
	else  if ((docDest = DocCreateScratch (pca->doc)) == docNil)
		return fFalse;

	/*  empty the destination doc and set styles, fonts to standard */
	SetWholeDoc (docDest, PcaSetNil(&ca));
	ResetDocStsh(docDest, fTrue /* fMinimal */);
	ResetDocFonts(docDest);

	switch (cf)
		{
	case CF_DIB:
	case CF_BITMAP:
	case CF_METAFILEPICT:
		fOk = FReadPict (PcaPoint(&ca, docDest, cp0 ), cf,
				hData, cbInitial, fFalse /* fEmptyPic */,
				NULL /* pchp */, NULL /* prcWinMF */);
		break;
	default:
		Assert (cf == cfRTF);
		fOk = FReadRTF (docDest, cf, hData, cbInitial);
		break;
	case CF_TEXT:
		fOk = FReadText (docDest, cf, hData, cbInitial);
		}

	fOk &= (!vmerr.fMemFail && !vmerr.fDiskFail);

	if (fOk)
		{
		struct CA caDest;

		ScratchBkmks(PcaSetWholeDoc( &caDest, docDest));

		if (pca->doc != docScrap)
			/* we were sucessfull, copy the new data over the old */
			{
			Assert (docDest == vdocScratch);

			CachePara(pca->doc, pca->cpFirst);
			if (vpapFetch.fTtp ||
					!FMoveOkForTable(pca->doc, pca->cpLim, &caDest) ||
					!FDeleteableEop(pca, rpkCa, &caDest) || 
					!FReplaceCps( pca, &caDest ))
				fOk = fFalse;
			else
				{
				pca->cpLim = pca->cpFirst + DcpCa(&caDest);
				CopyStylesFonts (docDest, pca->doc, pca->cpFirst, DcpCa(pca));
				AssureNestedPropsCorrect(pca, fFalse);
				}
			}
		else
			pca->cpLim = pca->cpFirst + DcpCa(&caDest);
		}

	  /* if we failed, empty the document */
	if (!fOk)  /* fOk may be reset above */
		SetWholeDoc (docDest, PcaSetNil(&ca));

#ifdef DEBUG
	if (pca->doc != docScrap)
		ReleaseDocScratch();
#endif /* DEBUG */

	return fOk;
}



/* F  R E A D  T E X T */
/*  Read text data from hData into docDest.

		Used for  CF_TEXT . FReadRTF in rtfin.c used for rtf
*/
/*  %%Function:  FReadText   %%Owner:  bobz       */


FReadText (docDest, cf, hData, cbInitial)
int docDest;
int cf;
HANDLE hData;
int cbInitial;
{
	BOOL fEol;
	struct RIBL **hribl;
	struct PAP papT;
	struct CHP chpT;
	LPCH lpch;
	unsigned long lcb;
	CHAR rgch[256];
	CP cp = cp0;

	/* NOTE: Under Windows, global handle sizes are rounded up to the
		nearest 16-byte bound.  We compensate for the fact that lcb may
		be larger than the actual size of the scrap object by checking
		for a null terminator. See CchReadLineExt. */

	/* note: if lcb is more than 64K, our simple methods won't work--don't try */
	if ((lcb = GlobalSize(hData)) > 0x00010000L)
		return fFalse;

	if ((lpch = GlobalLockClip(hData)) == NULL )
		return fFalse;

	/*  skip unwanted portion */
	lpch += cbInitial;
	lcb -= cbInitial;

	StandardChp(&chpT);
	StandardPap(&papT);

	while (lcb > 0 && !vmerr.fMemFail && !vmerr.fDiskFail)
		{

		/* Copy bytes from lpch to doc's cp stream */
		unsigned cch = lcb < 255 ? lcb : 255;

		if ((cch = CchReadLineExt( lpch, cch, rgch, &fEol))==0)
			/* Reached terminator */
			break;

		FInsertRgch( docDest, cp, rgch, cch, &chpT,
				(fEol) ? &papT : 0);
		cp += (CP) cch;


		lcb -= cch;
		lpch += cch;
		}

	GlobalUnlock (hData);

	return (fTrue);
}


/* F  R E A D  P I C T */
/*  Read picture data from hData into docDest.

		Used for (at least) CF_BITMAP and CF_METAFILE
*/
/*  %%Function:  FReadPict   %%Owner:  bobz       */


FReadPict (pcaDest, cf, hData, cbInitial, fEmptyPic, pchp, prcWinMF)
struct CA *pcaDest;
int cf;
HANDLE hData;
int cbInitial;
BOOL fEmptyPic;
struct CHP *pchp;
struct RC *prcWinMF;  /* window org/ext rc for metafiles. may be null */
{
	extern LPCH LpchIncr();

	struct CHP chpT;
	CHAR rgch[256];
	struct PIC picInfo;
	HBITMAP hbmMono=NULL;
	HANDLE hBits;
	HANDLE hDIB = NULL;
	LPCH lpch;
	unsigned long lcb;
	unsigned long lcbPicData;
	int cbOverhead = 0;
	int cch;
	BOOL fOk = fFalse;
	BOOL fPMDIB = fFalse; /* if true, needs to convert the DIB to Win 3 format */
	METAFILEPICT mfp;
	struct MFHDRDIBMF mfhdr; /* leading record for DIB in metafile */
	struct WINMRTAG mftag; /* trailing record for DIB in metafile */
#ifdef DEBUG
	LPCH lpchSave;
#endif


	Profile( vpfi == pfiPictPaste ? StartProf( 30) : 0);

	if (cbInitial != 0)   /* true only for DDE */
		{
		HANDLE hDDE;

		if ((lpch = GlobalLockClip(hData)) == NULL )
			return fFalse;

		/* for DDE, the actual data handle is in word 3 of the
			handle passed to us. Since we did not need to free up
			hData anyway, we just set it to the new data location
		*/
		/*  get to actual data handle address */
		lpch += cbInitial;
		bltbx( lpch, &hDDE, sizeof(HANDLE));
		GlobalUnlock( hData );
		hData = hDDE;
		}


	SetBytes( &picInfo, 0, cbPIC);
	picInfo.mx = mx100Pct;
	picInfo.my = my100Pct;

	picInfo.brcl = brclNone;

#ifdef INEFFICIENT
	picInfo.dyaCropTop = 0;
	picInfo.dxaCropLeft = 0;
	picInfo.dyaCropBottom = 0;
	picInfo.dxaCropRight = 0;
#endif /* INEFFICIENT */


	/* make local chp which can be modified */
	if (pchp == NULL)
		StandardChp(&chpT);
	else
		chpT = *pchp;

	ShrinkSwapArea();   /* allow more memory for pics */

	if (cf == CF_BITMAP && vwWinVersion >= 0x0300)
		{
		/* convert a color bitmap into a DIB under win 3 or beyond, 
		so that picture shows up in color instead of our old way of 
		converting it to monochrome bitmap
		*/
		BITMAP bm;
		unsigned long lcbBits;
		int cbInfoHdr;
		LPCH lpBits;
		LPCH lpBitsInfo;
		HANDLE hGDI;
		FARPROC lpfn;
		int cBitsPix;


		if (GetObject(hData, sizeof(BITMAP), &bm) < sizeof(BITMAP))
			goto ReadDone; // error reading bitmap descriptor

		cBitsPix = bm.bmPlanes * bm.bmBitsPixel;
		if (cBitsPix > 1) // color bitmap -> convert
			{
			if (cBitsPix != 4 && cBitsPix != 8 && cBitsPix != 24)
				{
				// don't know how to handle these non standard cases, leave it to
				// the normal procedure
#ifdef BZ
				CommSzSz(SzShared("FReadPict: Non standard bm, can't convert to DIB"), szEmpty);
#endif /* BZ */
				goto LKeepGoing;
				}

			if ((hGDI = GetModuleHandle(SzShared("GDI"))) == NULL)
				goto LKeepGoing;

			Assert(wwCur != wwNil);

			// size of DIB required 
			lcbBits = (long)WIDTHBYTES(cBitsPix * bm.bmWidth) * (long)bm.bmHeight;

			cbInfoHdr = sizeof(BITMAPINFOHEADER);
			// size of color table required
			if (cBitsPix != 24)
				cbInfoHdr += (1 << cBitsPix) * sizeof(RGBQUAD);

			if ((hDIB = GlobalAlloc2(GMEM_MOVEABLE, 
				(DWORD)((long)cbInfoHdr + lcbBits))) == NULL ||
				(lpBitsInfo = GlobalLock(hDIB)) == NULL)
				{
				goto ReadDone;
				}

#define lpbi ((LPBITMAPINFOHEADER) lpBitsInfo)
			bltbcx(0, lpBitsInfo, sizeof(BITMAPINFOHEADER)); // init
			lpbi->biSize = sizeof(BITMAPINFOHEADER);
			lpbi->biWidth = bm.bmWidth;
			lpbi->biHeight = bm.bmHeight;
			lpbi->biBitCount = cBitsPix;
			lpbi->biPlanes = 1;
#undef lpbi
			lpBits = LpchIncr(lpBitsInfo, (unsigned)cbInfoHdr);

			lpfn = GetProcAddress(hGDI, MAKEINTRESOURCE(idoGetDIBits));
			Assert(lpfn != NULL);
			// calling GetDIBits (a Win 3 call) through GetProcAddress 
			// because we still have to run under Win 2
			if ((*lpfn)(PwwdWw(wwCur)->hdc, hData, 0, bm.bmHeight, (LPSTR)lpBits, 
				(LPBITMAPINFO)lpBitsInfo, DIB_RGB_COLORS) != bm.bmHeight)
				{
				GlobalUnlock(hDIB);
				goto ReadDone;
				}

			GlobalUnlock(hDIB);
			hData = hDIB;
			cf = CF_DIB; 
			// we now got a DIB, request DIB format, so it will be turned
			// into a metafile 
			}
		}

LKeepGoing:

	switch (cf)
		{
	case CF_DIB:
		{

		/* at this point hData is a memory handle to a DIB */
		/* embed the DIB into a metafile */

		int cbDIBHdr; /* the header inside a DIB */
		unsigned long lcbDIB; /* DIB bits */
		int bmWidth;
		int bmHeight;
		int bmBitsPixel;

		lpch = GlobalLock(hData);
		picInfo.mfp.mm = MM_ANISOTROPIC; // convert a DIB to be embedded in a metafile 

#define lpbc ((LPBITMAPCOREHEADER) lpch)
#define lpbi ((LPBITMAPINFOHEADER) lpch)

#ifdef DEBUG
		lpchSave = lpch;
#endif
		if ((cbDIBHdr = CbDIBHeader(lpbi)) == 0)
			{
			GlobalUnlock(hData);
			goto ReadDone;
			}

		if (lpbi->biSize == (DWORD)sizeof(BITMAPINFOHEADER))	/* Win 3.0 */
			{

			/* Remove the following check when we work with Run-Length Encoding */
	
			if (lpbi->biCompression != BI_RGB)
				{
				GlobalUnlock(hData);
				goto ReadDone;
				}

			bmWidth     = (WORD) lpbi->biWidth;
			bmHeight    = (WORD) lpbi->biHeight;
			bmBitsPixel = (WORD) lpbi->biBitCount;
			}
		else if (lpbc->bcSize == (DWORD)sizeof(BITMAPCOREHEADER)) /* Old PM 1.1, 1.2 format */
			{
			bmWidth     = lpbc->bcWidth;
			bmHeight    = lpbc->bcHeight;
			bmBitsPixel = lpbc->bcBitCount;

			fPMDIB = fTrue; /* need to convert DIB to Win 3 format later */

			/* add in the difference between Win 3 and PM DIB header size */
			cbDIBHdr += (sizeof(BITMAPINFOHEADER) - sizeof(BITMAPCOREHEADER)) +
				/* color table difference */
				(bmBitsPixel == 24 ? 0 : (1 << bmBitsPixel));
			}
		else /* unknown format */
			{
			GlobalUnlock(hData);
			goto ReadDone;
			}

		lcbDIB = (long) WIDTHBYTES((DWORD)bmWidth * bmBitsPixel) *
			(long)bmHeight;

		/* this will set size based on current screen resolution */
		mfp.mm = MM_BITMAP;
		mfp.xExt = bmWidth;
		mfp.yExt = bmHeight;
		FComputePictSize( &mfp, &picInfo.dxaGoal, &picInfo.dyaGoal );

		/* have to convert from twips to himetrics */
		picInfo.mfp.xExt = UMultDiv(picInfo.dxaGoal, 1000, czaCm);
		picInfo.mfp.yExt = UMultDiv(picInfo.dyaGoal, 1000, czaCm);

		lcbPicData = (unsigned long)cbDIBHdr + lcbDIB;

		/* build the leading and trailing metafile records */
		BuildMFRecordForDIB(&mfhdr, &mftag, lcbPicData, bmWidth, bmHeight);

		cbOverhead = cbMFHDRDIBMF + cbWINMRTAG;

		break;
		}

	case CF_BITMAP:
		{   /* Set up Picinfo structure for bitmap */
		long lBmDimension;

		picInfo.mfp.mm = MM_BITMAP;

		/* at this point hData is a GDI handle to a BITMAP, which
					must be manipulated with GetObject, etc; it is not just
					a global handle with data in it.
				*/

		if (GetObject( hData, sizeof( BITMAP ), (LPSTR)&picInfo.bm )
				<  sizeof (BITMAP))
			{   /* Error reading bitmap descriptor */
			goto ReadDone;
			}

		  /* size used below. Grab it before converting color to mono or that is lost */
		lBmDimension = GetBitmapDimension( hData );

#ifdef BZ
		CommSzNum(SzShared("Bitmap dimension xExt at insert: "), LOWORD( lBmDimension ));
		CommSzNum(SzShared("Bitmap dimension yExt at insert: "), HIWORD( lBmDimension ));
#endif
		/* Again, we are not responsible for freeing the data
				in hData, so we can reuse the variable here.
			*/

		if (picInfo.bm.bmPlanes > 1 || picInfo.bm.bmBitsPixel > 1)
			{   /* Color bitmap --> make it monochrome */

			if ((hbmMono = HbmMonoFromHbmColor( hData )) != NULL)
				{
				hData = hbmMono;
				if (GetObject( hData, sizeof( BITMAP ),
						(LPSTR)&picInfo.bm ) <  sizeof (BITMAP))
					{   /* Error reading bitmap descriptor */
					goto ReadDone;
					}
				}
			else
				/* Error -- could not make it monochrome */
				goto ReadDone;
			}

		/* store Ideal/goal size here */

		/* trick 1: we want to store these values in twips. They
					are in .1mm units as are MM_LOMETRIC metafiles. We
					build a fake metafile with these values and MM_LOMETRIC
					and FComputePictSize will do it for us.
				*/
		if (lBmDimension != 0L)
			{
			mfp.mm = MM_LOMETRIC;
			mfp.xExt = LOWORD( lBmDimension );
			mfp.yExt = HIWORD( lBmDimension );
			FComputePictSize( &mfp, &picInfo.dxaGoal,
					&picInfo.dyaGoal );
			}
		else /* no goal size */
			/* trick 2: this will set size based on current screen resolution */
			{
			mfp.mm = MM_BITMAP;
			mfp.xExt = picInfo.bm.bmWidth;
			mfp.yExt = picInfo.bm.bmHeight;
			FComputePictSize( &mfp, &picInfo.dxaGoal,
					&picInfo.dyaGoal );
			}

		/* redundant, but the ext fields are the actual size, and
					so are consistent for bitmaps and metafiles this way.
				*/

		picInfo.mfp.xExt = picInfo.bm.bmWidth;
		picInfo.mfp.yExt = picInfo.bm.bmHeight;

		lcbPicData = ((unsigned long) picInfo.bm.bmWidthBytes *
				(unsigned long) picInfo.bm.bmHeight *
				(unsigned long) picInfo.bm.bmPlanes );

		/* Get the bitmap bits into a global handle */
		if ( ((hBits = GlobalAlloc2( GMEM_MOVEABLE, lcbPicData ))==NULL) ||
				((lpch = GlobalLock( hBits )) == NULL) ||
				(GetBitmapBits( hData, lcbPicData, lpch ) == 0))
			{
			/* Error */
			goto ReadDoneFree;
			}
		break;
		}

	case CF_TIFF:
		{   /* Set up Picinfo structure for tiff bitmap. Put
					lcb as the size of stFile and point lpch to
					the start of stFile.
						*/

		picInfo.mfp.mm = MM_TIFF;

		/* at this point hData is a global handle with a BITMAP
					struct followed by the normalized stFile for the
					tiff file.
				*/

		if ((lpch=(BITMAP FAR *)GlobalLockClip(hData))==NULL)
			{
			goto ReadDone;
			}

		bltbx( lpch, (BITMAP FAR *)&picInfo.bm,
				sizeof(BITMAP));
		lpch += sizeof(BITMAP);  /* point lpch at stFile */

		/* store Ideal/goal size here */
		GetGrSize(ftTIFF, &picInfo.dyaGoal,
				&picInfo.dxaGoal );

		picInfo.mfp.xExt = picInfo.bm.bmWidth;
		picInfo.mfp.yExt = picInfo.bm.bmHeight;

		/* data is just the file name */
		lcbPicData = (unsigned long) (*lpch + 1);
		break;
		}

	default:   /* metafile */
		{
		int mm;
		METAFILEPICT FAR *lpmfp;

		   /* Set up PicInfo structure for Metafile Picture */
		Assert (cf == CF_METAFILEPICT);
		if ((lpmfp=(METAFILEPICT FAR *)GlobalLockClip(hData))==NULL)
			{
			goto ReadDone;
			}
		bltbx( lpmfp, (METAFILEPICT FAR *)&picInfo.mfp,
				sizeof(METAFILEPICT));
		GlobalUnlock( hData );

		if ((lpch=GlobalLockClip( picInfo.mfp.hMF ))==NULL)
			{
			goto ReadDone;
			}

		lcbPicData = GlobalSize( picInfo.mfp.hMF );
		mfp = picInfo.mfp;

#ifdef BZ
	CommSzRgNum(SzShared("ReadPict - mfp at insert: "), &mfp,
			CwFromCch(sizeof(METAFILEPICT)));
   	CommSzLong(SzShared("Readpict metafile globalsize lcbPicData: "), lcbPicData);
#endif

		/* note: failure implies bogus metafile: abort */
		if (!FComputePictSize( &mfp, &picInfo.dxaGoal,
				&picInfo.dyaGoal ))
			{
			goto ReadDoneFree;
			}

		if (prcWinMF != NULL) /* indicates that the metafile is supplied by convertor */
			{
			/* need to add SetWindowOrg, SetWindowExt records to the metafile */
			/* so increment size */
			cbOverhead = sizeof (struct WMFWINOE);  /* see below */
			}


		break;
		}
		} /* switch (cf) */



	/* Here we write out to the scratch file */

	lcb = lcbPicData + (unsigned long)cbOverhead;
	
	/* Insert Picinfo structure into scratch file */
	picInfo.cbHeader = cbPIC;
	picInfo.lcb = lcb + cbPIC;   /* lcb includes header size */

	/* ensure pic starts on a long word boundary */
	PfcbFn(fnScratch)->cbMac = ((PfcbFn(fnScratch)->cbMac + 3) >> 2) << 2;
	/* this has to be contiguous, don't use holes in scratch file */
	vfkpdText.pn = pnNil;


	/* do this so we actually don't need to put anything in the metafile
	and will still get something the right size that shows up as a bordered
	square. This is done only for metafiles inserted with CndInsPicture.
	Force the size to be 1" square too.
	*/
	if (fEmptyPic)
		{
		picInfo.brcl = brclSingle;
		picInfo.dxaGoal = picInfo.dyaGoal = czaInch;
		picInfo.fFrameEmpty = fTrue;
		}

	chpT.fcPic = FcAppendRgchToFn(fnScratch, &picInfo, cbPIC);

#ifdef BZ
	CommSzRgNum(SzShared("ReadPict - picInfo at insert: "), &picInfo,
			CwFromCch(cbPIC));
#endif

#ifdef DCLIPPIC
	CommSzLong(SzShared("chp.fcPic during FReadPict fcAppend: "), chpT.fcPic);
#endif
	chpT.fSpec = 1;
	chpT.fnPic = 0;  /* will be set at format; clears fDirty */

	/* prcWinMF is only used for grpi special metafiles;
		if null, the rc is 0 and we set the window org to 0,0
			and its extent to the page size.
        If non-null, insert metafile instructions for Window Origin and
        Window Extent into the start of the metafile, following the metafile header.
        That is, increment the header file size, dump the header, then
        these records.

	*/

	if (cf == CF_METAFILEPICT && prcWinMF != NULL)
		{
		int dxp, dyp;
		struct WMFWINOE wmfwinoe;
		struct METAHDR FAR *pMetaHdr = lpch;

		cch = pMetaHdr->mtHeaderSize * sizeof(WORD);
		if (cch > sizeof(rgch) || (long)cch > lcbPicData)
			{
			goto ReadDoneFree;
			}

		pMetaHdr->mtSize += (long)cwWMFWINOE; /* size is in words */

		bltbxHuge(lpch, (char far *) rgch, cch, fTrue /* fSrcHuge */);

#ifdef BZ
		CommSzNum(SzShared("Import metafile header size: "), cch);
#endif /* BZ */

        /* dump the header */
		FcAppendRgchToFn(fnScratch, rgch, cch);
		lcbPicData -= cch;

		wmfwinoe.cwWinOrigin = wmfwinoe.cwWinExtent =
			/* cw for each rec: total cb /4 */
			(long)(sizeof(struct WMFWINOE) >> 2);
		wmfwinoe.kwdWinOrigin = META_SETWINDOWORG;
		wmfwinoe.xpWinOrigin = prcWinMF->xpLeft;
		wmfwinoe.ypWinOrigin = prcWinMF->ypTop;
		wmfwinoe.kwdWinExtent = META_SETWINDOWEXT;
		wmfwinoe.xpWinExtent = prcWinMF->xpRight - prcWinMF->xpLeft;
		wmfwinoe.ypWinExtent = prcWinMF->ypBottom - prcWinMF->ypTop;

		Assert (wmfwinoe.xpWinExtent > 0 && wmfwinoe.ypWinExtent > 0);

#ifdef BZ
		CommSzRgNum(SzShared("ReadPict - WMFWINOE: "), &wmfwinoe,
			CwFromCch(sizeof(struct WMFWINOE)));
#endif /* BZ */

		FcAppendRgchToFn(fnScratch, &wmfwinoe, sizeof (struct WMFWINOE));

		//  will likely never overflow 64k, but calling LpchIncr rather
		//  than having to prove that. cch will never be > 64K, but it would be possible
		//  to advance lpch previously, so this removes any possibility of
		// overflow. bz

		lpch = LpchIncr(lpch, cch);
		}
	else if (cf == CF_DIB)
		{
		/* If handleing DIB, insert the leading metafile record */
		FcAppendRgchToFn(fnScratch, &mfhdr, cbMFHDRDIBMF);

		if (fPMDIB)	/* convert PM DIB header to Win 3 format */
			{ 
			BITMAPINFOHEADER bminfohdr;
			int cbRGB;

			/* lpbc is defined to be lpch, make sure no one have changed 
			lpch since this code was written */
			Assert(lpchSave == lpch);

			/* bitmapinfohdeader to bitmapcoreheader */
			SetBytes(&bminfohdr, 0, sizeof(BITMAPINFOHEADER));
			bminfohdr.biSize = sizeof(BITMAPINFOHEADER);
			bminfohdr.biWidth = (DWORD)lpbc->bcWidth;
			bminfohdr.biHeight = (DWORD)lpbc->bcHeight;
			bminfohdr.biPlanes = lpbc->bcPlanes;
			bminfohdr.biBitCount = lpbc->bcBitCount;
			/* rest of bminfohdr are zeros by SetBytes */

#undef lpbc
#undef lpbi

			FcAppendRgchToFn(fnScratch, &bminfohdr, sizeof(BITMAPINFOHEADER));

			lpch = LpchIncr(lpch, sizeof(BITMAPCOREHEADER));
			lcbPicData -= sizeof(BITMAPINFOHEADER);

			/* color table - rgbTriple to rgbQuad */
			// cbRGB is the number of source bytes to read based on planes.
			// 1 plane: 6 bytes -> 8
			// 4 planes: 48 bytes -> 64
			// 8 planes: 256 * 3 bytes -> 256 * 4
			if (bminfohdr.biBitCount == 24)
				cbRGB = 0;
			else
				cbRGB = (1 << bminfohdr.biBitCount) * 3;

			// for 8 planes we need to make 4 passes, so we fit in rgch,
			// converting 192-> 256 at a time. 1 and 4 planes can be done
			// in 1 pass.

			while (cbRGB && !vmerr.fMemFail && !vmerr.fDiskFail)
				{
				int cbCur;
				char * psrc;
				char * pdst;

				bltbx(lpch, (LPCH)&rgch, (cbCur = min(cbRGB, 192)));
				psrc = rgch + cbCur; // at end of triples 
				pdst = psrc + (cbCur / 3); // where quads will end 

				while (psrc > rgch)
					{
					*--pdst = 0;
					*--pdst = *--psrc;
					*--pdst = *--psrc;
					*--pdst = *--psrc;
					}

				lpch = LpchIncr(lpch, cbCur);
				cbRGB -= cbCur;
				cbCur += cbCur / 3;
				FcAppendRgchToFn(fnScratch, &rgch, cbCur);
				lcbPicData -= cbCur;
				}
			}
		}

	/* put pic bits into fnScratch */
	for (; !vmerr.fMemFail && !vmerr.fDiskFail && lcbPicData > 0L; 
			lcbPicData -= cch, lpch = LpchIncr(lpch, cch))
		{
		bltbxHuge(lpch, (char far *) rgch,
				cch = (lcbPicData > 256L) ? 256 : lcbPicData,
				fTrue /* fSrcHuge */);
		FcAppendRgchToFn(fnScratch, rgch, cch);
		}

	// Write out the trailing metafile record for DIB case, 
	// we got a DIB embeded inside a metafile!
	if (cf == CF_DIB)
		FcAppendRgchToFn(fnScratch, &mftag, cbWINMRTAG);

	fOk = !vmerr.fDiskFail && !vmerr.fMemFail;

	if (fOk && FNewChpIns(pcaDest->doc, pcaDest->cpFirst, &chpT, stcNil))
		{
		/* put special picture char in docDest */
		char ch = chPicture;

#ifdef DCLIPPIC
		CommSzLong(SzShared("chp.fcPic during FReadPict after Fn: "), chpT.fcPic & 0x00FFFFFF);
		CommSzNum(SzShared("chp.fnPic during FReadPict: "), chpT.fnPic);
#endif

		if (pcaDest->doc == selCur.doc)
			FReplaceRM( pcaDest, fnScratch,
					FcAppendRgchToFn(fnScratch, &ch, 1), (FC)1);
		else
			FReplace(pcaDest, fnScratch,
					FcAppendRgchToFn(fnScratch, &ch, 1), (FC) 1);
#ifdef DCLIPPIC
		CachePara(pcaDest->doc, pcaDest->cpFirst);
		FetchCp(pcaDest->doc, pcaDest->cpFirst, fcmProps);
		CommSzRgNum(SzShared("vchpFetch after insert pic: "), &vchpFetch, cwCHP);
#endif

		/* record the fact that document may have a pict */
		PdodMother(pcaDest->doc)->fMayHavePic = fTrue;
		}

	fOk = !vmerr.fDiskFail && !vmerr.fMemFail;

ReadDoneFree:
	switch (picInfo.mfp.mm)
		{
	default: 		
		Assert (picInfo.mfp.mm < MM_META_MAX);
		GlobalUnlock( picInfo.mfp.hMF );
		if (cf == CF_DIB) /* get to here because a DIB turns into a metafile */
			GlobalUnlock(hData);
		break;

	case MM_TIFF:
		GlobalUnlock( hData );
		break;

	case MM_BITMAP:
		GlobalUnlock( hBits );
		GlobalFree( hBits );
		break;
		}

ReadDone:

	GrowSwapArea();   /* restore */

	if (hbmMono != NULL)
		{
		DeleteObject( hbmMono );
		}

	if (hDIB != NULL)
		{
		GlobalFree(hDIB);
		}

	if (vmerr.fDiskFail || vmerr.fMemFail)
		{
		struct CA ca;

		if (pcaDest->doc != selCur.doc)
			SetWholeDoc( pcaDest->doc, PcaSetNil(&ca));
		fOk = fFalse;
		}

	Profile( vpfi == pfiPictPaste ? StopProf() : 0);

	return fOk;
}



/********************************/
/*  %%Function:  HbmMonoFromHbmColor  %%Owner:  bobz       */

HbmMonoFromHbmColor( hbmSrc )
HBITMAP hbmSrc;
	{   /* Return a monochrome copy of the passed bitmap. Return NULL
		if an error occurred.  Assumes that the passed bitmap can be
	selected into a memory DC which is compatible with the doc DC. */

	extern long rgbBkgrnd;
	extern long rgbText;
	extern HWND vhWnd;

	BITMAP bm;
	HBITMAP hbmMono=NULL;
	HDC hMDCSrc = NULL;
	HDC hMDCDst = NULL;

	/* Create memory DC for source, set colors, select in passed bitmap */

	if ( (hMDCSrc = CreateCompatibleDC( (*hwwdCur)->hdc )) == NULL )
		goto BmFailure;
	LogGdiHandle(hMDCSrc, 1038);

#ifdef BOGUS
	/* We can't assume that every window out there has the same window colors that
	we have.  In fact, we have no way to figure out how to convert this color
	bitmap; so white will map to white and everything else will map to black. */
	SetBkColor( hMDCSrc, rgbBkgrnd );
	SetTextColor( hMDCSrc, rgbText );
#endif /* BOGUS */

	if (SelectObject( hMDCSrc, hbmSrc ) == NULL)
		goto BmFailure;

	/* Create memory DC for destination, select in a new monochrome bitmap */

	if ( ((hMDCDst = CreateCompatibleDC( (*hwwdCur)->hdc )) == NULL) ||
			((GetObject( hbmSrc, sizeof (BITMAP), (LPSTR) &bm ) == 0)) ||
			((hbmMono = CreateBitmap( bm.bmWidth, bm.bmHeight,
			1, 1, (LPSTR) NULL )) == NULL) ||
			Debug((hbmMono == NULL) ? fFalse : (LogGdiHandle(hbmMono, 1002), fFalse) ||)
			(SelectObject( hMDCDst, hbmMono ) == NULL) )
		{
		goto BmFailure;
		}
	LogGdiHandle(hMDCDst, 1039);

	/* Now blt the bitmap contents.  The screen driver in the source will
	"do the right thing" in copying color to black-and-white. */

	BitBlt( hMDCDst, 0, 0, bm.bmWidth, bm.bmHeight, hMDCSrc, 0, 0, SRCCOPY );

	UnlogGdiHandle(hMDCSrc, 1038);
	DeleteDC( hMDCSrc );
	UnlogGdiHandle(hMDCDst, 1039);
	DeleteDC( hMDCDst );
	return hbmMono;

BmFailure:

	if (hMDCSrc != NULL)            /* ORDER IS IMPORTANT: DC's before */
		{
		UnlogGdiHandle(hMDCSrc, 1038);
		DeleteDC( hMDCSrc );    /* objects selected into them */
		}
	if (hMDCDst != NULL)
		{
		UnlogGdiHandle(hMDCDst, 1039);
		DeleteDC( hMDCDst );
		}
	if (hbmMono != NULL)
		{
		UnlogGdiHandle(hbmMono, 1002);
		DeleteObject( hbmMono );
		}
	return NULL;
}


/* C F  N E X T  C F */
/*  Enumerate the clipboard formats that we understand in the order
	we like them.  cfNil returned when done, pass in cfNil to get first.
	
	Doesn't deal with CF_LINK which we do understand, but won't normally
	ask for.

	if fPrPic is fTrue, we will ask for the cfPrPic format (Excel specific).
*/
/*  %%Function:  CfNextCf   %%Owner:  bobz       */


CfNextCf (cf, fPrPic)
int cf;
BOOL fPrPic;
{

	/* Desired order (win3 word: Notice text before picture formats):
		 cfRTF
		 CF_TEXT
		 cfPRPic
		 CF_METAFILEPICT 
		 CF_DIB
		 CF_BITMAP
   */

#ifdef DEBUG
	/* a slimy way to let us test clipboard stuff and let us use
		slapWnd to put stuff into the clipboard.  If nonzero, we take its
		value to be the desired format. values are CF_ values
		we set its values with multiple calls to SetTestClip
		a debug function currently just the function set in
		cmddebug7
	*/
		{
		extern int vTestClip;
		if (vTestClip)
				return (vTestClip);
		}
#endif /* DEBUG */

	switch (cf)
		{
	default:
		if (cf == cfRTF)
			return CF_TEXT;     	/* 2nd choice */
		Assert(cf == cfPrPic && fPrPic && cfPrPic != cfNil);
		goto LMetafile;		/* 4th or 5th choice */

	case cfNil:
		return cfRTF;           	/* 1st choice */
	case CF_TEXT:
		if (fPrPic && cfPrPic != cfNil)
			return cfPrPic;			/* 4th choice (Excel specific) */
LMetafile:

		return CF_METAFILEPICT;		/* 4th or 5th choice */
	case CF_METAFILEPICT:
		return CF_DIB;				/* 5th or 6th choice */
	case CF_DIB:
		return CF_BITMAP;       	/* 6th or 7th choice */
	case CF_BITMAP:
		return cfNil;
		}

	Assert (fFalse);
}


/* F  C A N  W R I T E  C F */
/*  Return true iff it is possible to represent doc, [cpFirst:cpLim]
	in format cf.
*/
/*  %%Function:  FCanWriteCf   %%Owner:  bobz       */


FCanWriteCf (cf, doc, cpFirst, cpLim, fRestrictRTF)
int cf, doc;
CP cpFirst, cpLim;
 // if true, (currently only at quit), do not generate rtf if a picture
 // or if unformatted text, since it would be redundant 
BOOL fRestrictRTF;
{
	struct CA caT;
	CP cpImport;
	int mm;

	switch (cf)
		{
	case CF_TEXT:
		/* reject text for either a normal pic or an import field pic,
					since it has no text result and would show nothing */
		if (!FCaIsGraphics(PcaSet(&caT, doc, cpFirst, cpLim),
				fvcResults, &cpImport))
			return (cpLim > cpFirst);
		else
			return (fFalse);

	case CF_DIB:
		/* we can supply this if the ca is a DIB in metafile */
	case CF_BITMAP:
		/* We can supply this if the ca is a bitmap */
	case CF_METAFILEPICT:
		/* We can supply this if the ca is a metafile */

		/* FPicRenderable does a ChFetch. Treats metafile import fields as
			a pic, not tiff. Does a FetchPe so that vpicFetch is set up.
		*/
		if (FPicRenderable(doc, cpFirst, cpLim, &mm))
			{
			Assert (doc != docScrap || vsab.fPict);
			// just a warning. Unrecognized formats safely ignored bz
			Assert (mm == MM_BITMAP || mm < MM_META_MAX || mm == MM_DIB);
			switch (cf)
				{
				case CF_DIB:
					return FDIBInMetafile();
				case CF_BITMAP:
					return (mm == MM_BITMAP || 
						(vwWinVersion >= 0x0300 && FDIBInMetafile()));
				case CF_METAFILEPICT:
					return (mm < MM_META_MAX);
				default:   // currently unrecognized format
					return fFalse;
				}
			}
		else
			return (fFalse);

	case CF_OWNERDISPLAY:
		/* Render rich text to another Opus instance */
		Assert(fFalse);
		return (fFalse);

	default:
		if (cf == cfRTF)
			{
			// if we have a picture or unformatted text, the RTF is redundant, so omit
			if (fRestrictRTF && (!vsab.fFormatted ||
					 FCaIsGraphics(PcaSet(&caT, doc, cpFirst, cpLim),
						fvcResults, &cpImport)))
					{
					return (fFalse);
					}
			else
				return (cpLim > cpFirst);
			}
		else  if (cf == cfLink && doc == docScrap)
			return (FCanLinkScrap ());
		else
			return (fFalse);
		}

	Assert (fFalse);  // should never reach here
}


/* F   A S S U R E   H	 C B  G L O B */
/*  %%Function:  FAssureHCbGlob  %%Owner:  bobz       */


FAssureHCbGlob( ph, lcb, wAlloc, plcbMac, plcbMax )
int ***ph;
LONG lcb;
int wAlloc;
LONG *plcbMac;
LONG *plcbMax;
{
	/* Inputs:	*ph -- a handle
		*plcbMax - amount of space currently allocated to handle
				(0 iff *ph is NULL)
		lcb -- amount of space desired in handle
				wAlloc -- Global mem value. usually GMEM_DDE or
					GMEM_MOVEABLE
		Outputs: *ph -- if successful, a handle with at least cb bytes of
						space else hNil
		*plcbMac -- if successful, == lcb, else unchanged or 0
		*plcbMax -- amount of space allocated to handle on return
	
		Return:	fTrue if we returned with at least cb bytes of space in the
		handle; fFalse if we tried to grow/allocate it and failed.
	
		Based on FAssureHCb. Used by Clipboard handle routines. We try to
			allocate bigger chunks - like 1K bytes; if that fails we try for
			the requested size. If this is to be generalized, CBDESIRED can
			be changed to be a parameter.
	*/

#define CBDESIRED (1024)
	/* round alloc requests to multiples of 16 */
#define CBTOPBOUND(cb)  ((cb + 15) >> 4 << 4)

	HANDLE h;
	long lcbT;

	/* easy case, already have enough space */

	if (lcb <= *plcbMax)
		{
		*plcbMac = lcb;
		return fTrue;
		}

	if (*plcbMax > 0L)        /* aleady been allocated. Try to grow */
		{
#ifdef BZTEST
		CommSz(SzShared("FAssureHCbGlob realloc start: "));
#endif /* BZTEST */

		Assert( *ph != hNil );
		h = *ph;  /* to save original handle */
		if ((*ph = OurGlobalReAlloc( *ph,
				(LONG) (lcbT = CBTOPBOUND (lcb + CBDESIRED)),
				wAlloc )) == NULL)
			{   /* Could not expand handle. Try minimum allocation  */
			*ph = h;
			if ((*ph = OurGlobalReAlloc( *ph, (LONG) (lcbT = CBTOPBOUND(lcb)),
					wAlloc )) == NULL)
				{   /* Could not expand handle */
				*ph = h;   /* So it can still be used */
				/* leave ph, mac and max unchanged */
				return fFalse;
				}
			}

#ifdef BZTEST
		CommSzLong(SzShared("FAssureHCbGlob realloc size: "), lcbT);
#endif /* BZTEST */

		}
	else
		{
		Assert( *ph == hNil );

		if ((*ph = OurGlobalAlloc( wAlloc,
				(LONG) (lcbT = CBTOPBOUND(lcb + CBDESIRED)) )) == hNil)
			{     /* Could not get desired amount. Try minimum allocation  */
			if ((*ph = OurGlobalAlloc( wAlloc,
					(LONG) (lcbT = CBTOPBOUND(lcb)))) == hNil)
				{   /* Could not get minimum */
				*plcbMac = *plcbMax = 0;
				return fFalse;
				}
			}
#ifdef BZTEST
		CommSzLong(SzShared("FAssureHCbGlob first alloc size: "), lcbT);
#endif /* BZTEST */

		}

	*plcbMax = lcbT;
	*plcbMac = lcb;
	return fTrue;
}



/*****************************/
/*  %%Function:  FPicRenderable  %%Owner:  bobz       */

/* returns TRUE is range represents a single picture 
   pmm is the picture type, from vpicfetch.mfp.mm, even
     if FALSE is returned. It will be MM_NIL if the range is not a picture at all.
   will return true for non-TIFF import fields.
   "old format" import metafiles will be rejected here.	   
   Note that FCaIsGraphics returns true for all imports, and
   is the more general routine. This routine is used only by those
   callers who want a renderable picture.

	   vpicfetch will be set up if TRUE
*/

FPicRenderable(doc, cpFirst, cpLim, pmm)
int doc;
CP cpFirst, cpLim;
int *pmm;
{
	struct CA caT;
	CP cpImport;

	PcaSet(&caT, doc, cpFirst, cpLim);
	if (FCaIsGraphics(&caT, fvcResults, &cpImport))
		{
		Assert(*vhpchFetch == chPicture && vchpFetch.fSpec);
		Assert (cpImport == cpNil || cpImport == vcpFetch);
		/* FetchPe will set vchpFetch.fnPic to fnFetch bz */
		FetchPe(&vchpFetch, doc, vcpFetch);

		*pmm = vpicFetch.mfp.mm;  /* set up even if we reject */

		if (vpicFetch.mfp.mm == MM_TIFF ||
			  /* rejects old format import metafiles */
			(vpicFetch.mfp.mm < MM_META_MAX && !FEmptyRc(&vpicFetch.rcWinMF)))
			{
			return fFalse;
			}
		else
			return fTrue;

		}

	*pmm = MM_NIL;  /* set up even if we reject */
	return fFalse;

}


/*  %%Function:  bltbxHuge  %%Owner:  bobz       */


bltbxHuge( lpchSrc, lpchDest, cbCopy, fSrcHuge)
LPCH    lpchSrc;
LPCH    lpchDest;
unsigned  cbCopy;
BOOL fSrcHuge;	// if true, lpchSrc is the huge data, else lpchDest
{
	/* blt from lpchSrc to lpchDest, splitting into 2 blts if cchCopy
	   would cross a 64k segment boundary. Assumes that only 1 of the
	   2 pointers is a huge block that could cross 64k boundaries
	*/

	unsigned cbRemain;

	LPCH lpchHuge = fSrcHuge? lpchSrc : lpchDest;

	  // cbRemain will be the # of bytes left in current segment
	if ((cbRemain = -LOWORD(lpchHuge)) < cbCopy)
		{
		if (cbRemain) // can skip if 0
			{
#ifdef BZ
	   		CommSzLong(SzShared("Splitting blt: lpchHuge = "), lpchHuge);
	   		CommSzNum(SzShared("Splitting blt: cbRemain = "), cbRemain);
	   		CommSzNum(SzShared("Splitting blt: cbCopy = "), cbCopy);
#endif
			bltbx(lpchSrc, lpchDest, cbRemain);
			cbCopy -= cbRemain;
			lpchSrc = LpchIncr(lpchSrc, cbRemain);
			lpchDest = LpchIncr(lpchDest, cbRemain);
			}
		}

	bltbx(lpchSrc, lpchDest, cbCopy);

}


VOID BuildMFRecordForDIB(pmfhdr, pmftag, lcbDIB, bmWidth, bmHeight)
struct MFHDRDIBMF *pmfhdr;	/* destination of leading record */
struct WINMRTAG *pmftag; /* destination of trailing record */
unsigned long lcbDIB; /* bytes of DIB bits */
int bmWidth; /* bitmap width */
int bmHeight; /* bitmap height */
/* 
 * BuildMFRecordForDIB - build some metafile records, a leading and 
 * 	a trailing record for the purpose of embedding DIB inside a metafile.
 * 	The leading record is to be inserted before a DIB and the trailing
 * 	record is inserted after the DIB.
 * 	The leading record consists of :
 * 	1) a standard metafile header (METAHDR),
 * 	2) a window orgin and extent record (WMFWINOE),
 * 	3) a text / background color record (WMFCOLOR),
 * 	4) a stretchdibbitmap record (WMFSDIB).
 * 	The trailing record is just a simple end metafile record. 
 * Return value : none
 * Side effects : none
 * Documentation: 
 */
{

	// Leading metafile record : 

	// windows org and ext records

	pmfhdr->winoe.cwWinOrigin = pmfhdr->winoe.cwWinExtent =
	/* cw for each rec: total cb /4 */
		(long)(sizeof(struct WMFWINOE) >> 2);
	pmfhdr->winoe.kwdWinOrigin = META_SETWINDOWORG;
	pmfhdr->winoe.xpWinOrigin = pmfhdr->winoe.ypWinOrigin = 0;
	pmfhdr->winoe.kwdWinExtent = META_SETWINDOWEXT;
	pmfhdr->winoe.xpWinExtent = bmWidth;
	pmfhdr->winoe.ypWinExtent = bmHeight;

	// text and background color records

	pmfhdr->color.cwTColor = pmfhdr->color.cwBColor =
		/* cw for each rec: total cb /4 */
	   	(long)(sizeof(struct WMFCOLOR) >> 2);
	pmfhdr->color.kwdTColor = META_SETTEXTCOLOR;
	pmfhdr->color.rgbTColor = vsci.rgbText;
	pmfhdr->color.kwdBColor = META_SETBKCOLOR;
	pmfhdr->color.rgbBColor = vsci.rgbBkgrnd;

	// StretchDIBits record

	pmfhdr->stretchdib.kwdSDIB = META_STRETCHDIB;
	// stretchdibits size: lbcDIB + record overhead
	pmfhdr->stretchdib.cwSDIB = (lcbDIB + cbWMSDIB + sizeof(WORD) - 1) / 
		sizeof(WORD);
	pmfhdr->stretchdib.dwRop = SRCCOPY;
	pmfhdr->stretchdib.wUsage = 0;	 // unused
	pmfhdr->stretchdib.srcYExt = bmHeight;
	pmfhdr->stretchdib.srcXExt = bmWidth;
	pmfhdr->stretchdib.srcY = 0;
	pmfhdr->stretchdib.srcX = 0;
	pmfhdr->stretchdib.dstYExt = bmHeight;
	pmfhdr->stretchdib.dstXExt = bmWidth;
	pmfhdr->stretchdib.dstY = 0;
	pmfhdr->stretchdib.dstX = 0;


	// standard metafile header record

	pmfhdr->metahdr.mtType = 1;  // disk metafile
	pmfhdr->metahdr.mtHeaderSize = cwMETAHDR;
	pmfhdr->metahdr.mtVersion = WINMETAVERSION;
	// total size (words): the dib bits + mf overhead
	pmfhdr->metahdr.mtSize = (lcbDIB + cbMFHDRDIBMF + cbWINMRTAG +
		sizeof(WORD) - 1) / sizeof(WORD);
	pmfhdr->metahdr.mtNoOObjects = 0;
	pmfhdr->metahdr.mtNoParameters = 0;   // unused
	pmfhdr->metahdr.mtMaxRecord = pmfhdr->stretchdib.cwSDIB;

	// Trailing metafile record 

	pmftag->rdSize = cwWINMRTAG;
	pmftag->kwd = METAENDREC;

}


BOOL FDIBInMetafile()
/* 
 * FDIBInMetafile - Check to see if a metafile contains just a DIB,
 * 	Check by comparing keywords of metafile record that we put in
 * 	when we embed a DIB into a metafile.
 *
 * Return value : True if a DIB is embedded in a metafile, otherwise, false
 * Side effects : none
 * Documentation: 
 */
{
	long lcb = vpicFetch.lcb;
	struct MFHDRDIBMF mfhdr;

	/* make sure we have the picture fetched */
	Assert(*vhpchFetch == chPicture && vchpFetch.fSpec);
	Assert(vfcFetchPic == ((vchpFetch.fcPic & 0x00FFFFFF) + cbPIC));

	if (vpicFetch.mfp.mm == MM_ANISOTROPIC && /* is a metafile */
		lcb > (long)(cbPIC + cbMFHDRDIBMF + cbWINMRTAG)) /* and at least big enough */
		{
		/* vfcFetchPic points to the begining of the metafile */ 
		/* read the metafile leading record */
		FetchPeData(vfcFetchPic, (char HUGE *)&mfhdr, (long)cbMFHDRDIBMF);

		/* win 3 metafile? in case of randomness, check if all keywords
		that we put in are there
		*/
		if (mfhdr.metahdr.mtVersion >= WINMETAVERSION && 
			mfhdr.winoe.kwdWinOrigin == META_SETWINDOWORG && 
			mfhdr.winoe.kwdWinExtent == META_SETWINDOWEXT &&
			mfhdr.color.kwdTColor == META_SETTEXTCOLOR &&
			mfhdr.color.kwdBColor == META_SETBKCOLOR &&
			mfhdr.stretchdib.kwdSDIB == META_STRETCHDIB)
			{
			/* last check: that DIB is the only thing embedded in this metafile */
			/* i.e. the only thing left after the DIB is a end metafile record */
			if (lcb - (cbPIC + cbMFHDRDIBMF + 
				((mfhdr.stretchdib.cwSDIB - cwWMSDIB) * sizeof(WORD))) == cbWINMRTAG)
				return fTrue;
			}
		}

	return fFalse;
}

