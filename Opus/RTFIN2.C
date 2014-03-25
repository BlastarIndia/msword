/* R T F I N 2 . C */
/* this file is #include'd into rtfin.c (broken up for SLM's sake) */


/*  %%Function:  ApplyPropChange  %%Owner:  bobz       */
ApplyPropChange(irrbProp, fHaveVal, val, hribl)
int irrbProp;
int fHaveVal;
int val;
struct RIBL **hribl;
{
	int *pw;
	int f;
	int mask;
	struct RRB rrb;
	char *pprop;
	char *pch;
	long lval;
	struct BRC *pbrc, *rgbrc;
	int ibrc;
	struct TC *ptc;
	int valUse;

	GetRrb(irrbProp, &rrb);
	valUse = fHaveVal ? val : rrb.w;

#ifdef BZTEST
	CommSzNum(SzShared("ApplyPropChange valUse = "), valUse);
#endif /* BZ */

	switch (rrb.pgc)
		{
	case pgcChar:
		pprop = &(**hribl).chp;
		break;
	case pgcPara:
	case pgcBrc:
		pprop = &(**hribl).pap;
		break;
	case pgcSect:
		pprop = &(**hribl).sep;
		break;
	case pgcTable:
		pprop = &(**hribl).tap;
		break;
	case pgcDoc:
		pprop = PdopDoc((**hribl).doc);
		break;
	case pgcPic:
		pprop = &(**hribl).pic;
		break;
	case pgcRibl:
		pprop = (*hribl);
		break;
	case pgcDtr:
		pprop = &dtrRTF;
		break;
	default:
		/* this is an error case. Bag out in non-debug */
		Assert (fFalse);
#ifdef BZ
		CommSzNum(SzShared("ApplyPropChange invalid pgc = "), rrb.pgc);
#endif /* BZ */
		return;
		}
	switch (rrb.rrba)
		{
CaseFlag:
	case rrbaFlag:
		pprop[rrb.b] = (!fHaveVal) ? fTrue : (val != 0);
		break;
CaseBit:
	case rrbaBit:
		pw = &((int *)pprop)[rrb.w];
		mask = (1 << rrb.b);
		f = ((!fHaveVal) ? fTrue : (val != 0));
		if (f)
			*pw |= mask;
		else
			*pw &= ~mask;
		break;
	case rrbaUns:
	case rrbaWord:
CaseWord:
		*(int *)(pprop + rrb.b) = valUse;
		break;
	case rrbaByte:
CaseByte:
		pprop[rrb.b] = valUse;
		break;
	case rrbaSpec:
		switch (irrbProp)
			{
			int itbd;
			char stc, stcOld;
			char rgb[5];
			struct CA ca;
		case irrbRs:
			stcOld = ((struct PAP *)pprop)->stc;
			stc = ((struct PAP *)pprop)->stc = valUse;
			if ((**hribl).rds != rdsStylesheet)
				{
				if (stc != stcOld)
					{
					if ((**hribl).fPermOK)
						stc = (**hribl).rgbStcPermutation[stc];
					else
						stc = stcNormal;
					ApplyStcEndOfDoc((**hribl).doc, stc);
					}
				}
			break;
		case irrbRtlc:
			(**hribl).tbd.tlc = val;
			break;
		case irrbRtjc:
			(**hribl).tbd.jc = val;
			break;
		case irrbRtb:
			/* fall through */
		case irrbRtx:
			itbd = ((struct PAP *)pprop)->itbdMac;
			if (fHaveVal && itbd < itbdMax)
				{
				int dxa;
				struct TBD tbd;
				tbd = (**hribl).tbd;
				AddTabs(&((struct PAP *)pprop)->rgdxaTab,
						((struct PAP *)pprop)->rgtbd, NULL, &((struct PAP *)pprop)->itbdMac,
						(char HUGE *) &val, (char HUGE *) &tbd, NULL, 1);
				}
			(**hribl).tbd.tlc = tlcNone;
			(**hribl).tbd.jc = jcLeft;
			break;
		case irrbRcf:
			((struct CHP *)pprop)->ico = (valUse < 0 || valUse >= (**hribl).icoMac) ? icoAuto :
					(**hribl).mpicoOldicoNew[valUse];
			break;
		case irrbRsbys:
			(**hribl).fSideBySide = fTrue;  /* so we will post-process */
			goto CaseFlag;
			break;

		case irrbRv:

#ifdef WIN
			/* for xe and tc, vanished is not put into the chp but
				is a signal to omit the entry texts from the document
				stream
				*/

			f = ((!fHaveVal) ? fTrue : (val != 0));
			if (FVanishXeTc(hribl, f))
				break;
			else
#endif /* WIN */
				goto CaseBit;
		case irrbRdeff:
			/* set up ftcMapDef for chps set up before font table done */
			(**hribl).deff = ftcMapDef = valUse;
			break;
		case irrbRf:

			if (!fHaveVal)
				valUse = ftcMapDef;
			else  if ((**hribl).fFonttblDef)
				{
				/* NOTE: this differs from Mac */
				if (!FSearchFtcmap((**hribl).hftcmap, val, &valUse, fFalse /* fAdd */))
					valUse = ftcMapDef;
				}
			else if /* if building font table, not def but not nil use value
			from valuse; use default
			only if we couldn't build the table due to oom */
			((**hribl).hftcmap == hNil)
				valUse = ftcDefault; /* if no font table use default font */

			goto CaseWord;
		case irrbRfs:
 			// limit font size to fit in byte
 			if ((unsigned)valUse > hpsOldMax)
 				valUse = hpsOldMax;
 			goto CaseByte;
 
		case irrbRkul:
			((struct CHP *)pprop)->kul = valUse;
			break;
		case irrbRexpnd:
			((struct CHP *)pprop)->qpsSpace = valUse;
			break;
		case irrbRup:
			pprop[rrb.b] = valUse;
			break;
		case irrbRdn:
			pprop[rrb.b] = 256 - (valUse);
			break;
		case irrbRfpc:
			/* always use val - these are all passval entries */
			/* for enddoc, we generate both \enddoc and \endnote, so when
			reading back in we want enddoc to override endnote, so we
			ignore endnote if already enddoc. The other order takes
			care of itself.
			*/
			Assert (fHaveVal);

			if (!(val == fpcEndnote && ((struct DOP *)pprop)->fpc == fpcEndDoc))
				((struct DOP *)pprop)->fpc = val;
			break;
		case irrbRftnstart:
			((struct DOP *)pprop)->nFtn = valUse;
			break;
		case irrbRpgnx:
			(**hribl).dxaPgn = valUse;
			break;
		case irrbRpgny:
			(**hribl).dyaPgn = valUse;
			break;
		case irrbRpgnstarts:
			/* if we get a non zero val for this, the fPgnRestart flag
			must be set also. For Opus generated files we should
			always get  a fPgnRestart as well if val != 0. */
			((struct SEP *)pprop)->pgnStart = valUse;
			((struct SEP *)pprop)->fPgnRestart = (valUse != 0);
			break;

		case irrbRcols:
		case irrbRlinestarts:
			/* these come in with 1 based values, stored 0 based */
			if (!fHaveVal)
				val = rrb.w;
			*(int *)(pprop + rrb.b) = (val > 0 ? val - 1 : 0);
			break;

			/* note differences here from Mac code */

		case irrbRmacpict: /* setup in ChngtoRdsPic */
			break;
		case irrbRwbitmap:
			(**hribl).iTypePic = iPicWBitmap;
			(**hribl).pic.bm.bmType = val;
			(**hribl).fDisregardPic = fFalse;
			break;
		case irrbRwmetafile:
			(**hribl).iTypePic = iPicWMetafile;
			(**hribl).pic.mfp.mm = val;
			(**hribl).fDisregardPic = fFalse;
			break;
		case irrbRpich:  /* same code for bm, metafile */

			if ( (**hribl).iTypePic == iPicWBitmap)
				goto CaseWord;  /* set up for bitmap */
			else
				(**hribl).pic.mfp.yExt = val;
			break;
		case irrbRpicw:  /* same code for bm, metafile */

			if ( (**hribl).iTypePic == iPicWBitmap)
				goto CaseWord;  /* set up for bitmap */
			else
				(**hribl).pic.mfp.xExt = val;
			break;

		case irrbRplain:
			RtfStandardChp(&(**hribl).chp);

#ifdef WIN
			FVanishXeTc(hribl, fFalse);
#endif /* WIN */

			break;
		case irrbRpard:
				{
				struct PAP *ppap = &(**hribl).pap;
				int stc = ppap->stc;
				SetWords(ppap, 0, cwPAP);
				StandardPap(ppap);  /* clears out to cwPapBase */
				Assert (ppap->stc == stcNormal);  /* so we don't need to map stc below */
				if (stc != ppap->stc)
					ApplyStcEndOfDoc((**hribl).doc, ppap->stc);
				}
			break;
		case irrbRsectd:
			RtfStandardSep(&(**hribl).sep);
			break;
		case irrbRsnext:
			(**hribl).fStcNextOK = fTrue;
			goto CaseByte;
		case irrbRIgnore:
			break;

			/* for Opus, these doc props get converted to a first section
			prop. We store them in the ribl until input is complete,
			then throw them into the first section
			*/
		case irrbRpgnstart:
		case irrbRlinestart:
			goto CaseWord;

		case irrbRrevprop:

			/* this is a 7 bit field in the dop */
			if (!fHaveVal)
				val = rrb.w;

			((struct DOP *)pprop)->irmProps = val;
			break;

		case irrbRgcw:	    /* grid column width */
				{
				int cb;

				if (!fHaveVal)
					val = rrb.w;
				if ((**hribl).hsttbGrid != hNil &&
						((**hribl).hstGrid != hNil) &&
						(cb = **((**hribl).hstGrid)) < cchMaxSz - 2   )

					/* We are using sttb's here to simplify use of data
					structures. This limits us to 127 int values for
					column widths. We throw out any columns past 127.
					The first word is used for the st count
					and byte 1 is left unused. This first gcw value is in
					the 2nd word.
					*/
					{
					char *pch = (*((**hribl).hstGrid) + cb + 1);  /* where val will go */
					*(int *)pch = val;
					**((**hribl).hstGrid) += sizeof(int);
					}

				break;
				}
		case irrbRedmins:
			lval = (long)val;
			pch = &lval;
			goto SetInfo;

		case irrbRversion:
			if (val <= 0)
				{
				ReportSz("bogus version number in source rtf");
				/* leave it the default 1 */
				break;
				}
		case irrbRnofchars:
		case irrbRnofpages:
		case irrbRnofwords:
			pch = &val;
SetInfo:
			SetInfoFromIifdPval(IifdFromRds(rrb.w), pch,
					(**hribl).doc, hNil /* unused hsttbAssoc */);
			break;

		case irrbRflddirty:
		case irrbRfldedit:
		case irrbRfldlock:
		case irrbRfldpriv:
			/* these all store fTrue into the RFFLD component of the
			hrcpfld in the ribl. This is hNil except immediately after
			a \field, so we check validity before storing.
			*/

			if ((**hribl).rds == rdsField && (**hribl).hrcpfld != hNil)
				{
				pprop = &((*((**hribl).hrcpfld))->rffld);
				goto CaseBit;
				}

#ifdef DEBUG
			else
				Assert (fFalse);	/* thrown out if in wrong place bz */
#endif /* DEBUG */

			break;

		case irrbRpbrc:
			(**hribl).ibrc = val;
			(**hribl).fParaBrc = fTrue;
			rgbrc = &(**hribl).pap.brcTop;
			if (val < 6)
				rgbrc[val].brc = 0;
			else
				SetBytes(rgbrc, 0, 4 * sizeof(int));
			break;

		case irrbRclbrc:
			(**hribl).ibrc = val;
			(**hribl).fParaBrc = fFalse;
			rgbrc = (**hribl).tap.rgtc[(**hribl).tap.itcMac].rgbrc;
			rgbrc[val].brc = 0;
			break;

		case irrbRbrcBase:
		case irrbRbrdrsh:
		case irrbRbrsp:
			/* same code for pic, para, table props */

			if ((**hribl).rds == rdsPic)
				{
				if (irrbProp == irrbRbrcBase)
					(**hribl).pic.brcl = BrclFromIbrc(val);
				else  if (irrbProp == irrbRbrdrsh)
					(**hribl).pic.brcl = brclShadow;
				/* ignore brsp for pic */
				break;
				}

			rgbrc = ((**hribl).fParaBrc) ?
					&(**hribl).pap.brcTop :
					(**hribl).tap.rgtc[(**hribl).tap.itcMac].rgbrc;
			if ((**hribl).fParaBrc && (**hribl).ibrc == 6)  /* box */
				{
				for (ibrc = 0, pbrc = rgbrc; ibrc < 4; ibrc++, pbrc++)
					SetBrcForIrrb(pbrc, irrbProp, valUse);
				}
			else
				SetBrcForIrrb(&rgbrc[(**hribl).ibrc], irrbProp, valUse);
			break;

		case irrbRpcVert:
			((struct PAP *)pprop)->pcVert = valUse;
			break;
		case irrbRpcHorz:
			((struct PAP *)pprop)->pcHorz = valUse;
			break;

		case irrbRdxaAbs:
			/* values >= 0 are biased by 1 internally */
			((struct PAP *)pprop)->dxaAbs = valUse + 1;
			break;
		case irrbRdxaAbsNeg:
			((struct PAP *)pprop)->dxaAbs = -valUse;
			break;
		case irrbRdyaAbs:
			/* values >= 0 are biased by 1 internally */
			((struct PAP *)pprop)->dyaAbs = valUse + 1;
			break;
		case irrbRdyaAbsNeg:
			((struct PAP *)pprop)->dyaAbs = -valUse;
			break;

		case irrbRcellx:
			if (fHaveVal && (**hribl).tap.itcMac < itcMax)
				{
				((struct TAP *)
						pprop)->rgdxaCenter[(**hribl).tap.itcMac + 1] = valUse;
				(**hribl).tap.itcMac++;
				}
			break;

		case irrbRtcelld:
		case irrbRclmgf:
		case irrbRclmrg:
			ptc = &((struct TAP *)pprop)->rgtc[(**hribl).tap.itcMac];
			switch (irrbProp)
				{
			case irrbRclmgf:
				ptc->fFirstMerged = ((!fHaveVal) ? fTrue : (val != 0));
				break;
			case irrbRclmrg:
				ptc->fMerged = ((!fHaveVal) ? fTrue : (val != 0));
				break;
			case irrbRtcelld:
				SetBytes(ptc, 0, sizeof(struct TC));
				break;
				}
			break;
		case irrbRblue:
		case irrbRgreen:
		case irrbRred:
			/* this lets us tell that we want to use Auto color */
			(*hribl)->fNoColor = fFalse;
			goto CaseByte;

		case irrbRtrowd:
			SetBytes(pprop, 0, cbTAP);
			(**hribl).tap.itcMac = 0;
			break;

			}  /*  switch (irrbProp)  */

		}
#ifdef DRTFSEP
	if (rrb.pgc == pgcSect)
		{
		CommSzNum(SzShared("applypropchange irrbprop: "), irrbProp);
		CommSzRgNum(SzShared("applypropchange sep hribl.sep: "), &(**hribl).sep, cwSEP);
		}
#endif
}


/*	H R I B L  C R E A T E	D O C

Inputs:
	docIn		Doc used by hribl or docNil if doc to be created

Returns:
	mfn		Handle to created file
*/
/*  %%Function:  HriblCreateDoc  %%Owner:  bobz       */

struct RIBL  **HriblCreateDoc(docIn)
int docIn;   /* if docNil, create doc */
{
	int doc;
	char **hrgbStack;
	struct RIBL **hribl, *pribl;

	if ((hribl = HAllocateCw(CwFromCch(sizeof(struct RIBL)))) == hNil)
		{
		/* vmerr.fMemFail already set in FEnsureFreeCw */
		return (hNil);
		}

#ifdef BZTESTX
	CommSzNum(SzShared("hribl allocated at cwRIBL = "), cwRIBL);
#endif /* BZTESTX */

	if ((hrgbStack = HAllocateCw(1)) == hNil)
		{
		FreeH(hribl);
		/* vmerr.fMemFail already set in FEnsureFreeCw */
		return (hNil);
		}

	if (docIn == docNil)
		{
#ifdef BZ
		CommSzNum(SzShared("HriblCreateDoc vdbs.cLmemSucceed = "), vdbs.cLmemSucceed);
#endif /* BZ */

		doc = DocCloneDoc(docNew, dkDoc);

		if (doc == docNil)
			{
			FreeH(hrgbStack);
			FreeH(hribl);
			return (hNil);
			}
		else
			{
			RtfStandardDop(PdopDoc(doc));
#ifdef BZTEST
			CommSzNum(SzShared("HriblCreateDoc hsttbChpe = "),
					PdodDoc(doc)->hsttbChpe);
#endif /* BZ */
			}
		}
	else
		doc = docIn;

#ifdef BZTESTX
	CommSzNum(SzShared("dop.grpfIhdt after hribl alloc = "), PdopDoc(doc)->grpfIhdt);
#endif /* BZTESTX */

	/* All commented statements below in this routine are initialized by this
		SetWords call */
	  /* this has to be reset on each rtfin */
	ftcMapDef = ftcDefault;

	SetWords((*hribl), 0, cwRIBL);
	Assert (hNil == 0);

	RtfStandardSep(&(**hribl).sep);
	/* SetWords(&(**hribl).pap, 0, cwPAP); */
	StandardPap(&(**hribl).pap);
	(**hribl).pap.stc = stcNil;

	/* (**hribl).tbd = (char)0; */
	/* (**hribl).tbd.tlc = tlcNone; */
	/* (**hribl).tbd.jc = jcLeft; */
	RtfStandardChp(&(**hribl).chp);

	pribl = *hribl;  /* local heap pointer usage only */
	FreezeHp();  /* for pribl */


	pribl->ris = risNorm;
	pribl->doc = doc;
	pribl->rds = rdsMain;
	pribl->chs = chsAnsi;
	/* pribl->lcb = 0L; */
	/* pribl->cpRdsFirst = cp0; */
	pribl->hrgbStack = hrgbStack;
	pribl->bStackLim = 1;
	/* pribl->fFonttblDef = fFalse; */
	/* pribl->hftcmap = hNil; */
	Assert (ftcDefault == 0);
	/* pribl->deff = ftcDefault; */
	/* pribl->fPermOK = fFalse; */
	pribl->cbCharInfo = sizeof(struct CHP);
	pribl->cbParaInfo = sizeof(struct PAP);
	pribl->cbSectInfo = sizeof(struct SEP);
	pribl->cbTableInfo = sizeof(struct TAP);
	/* pribl->fCancel = fFalse; */
	/* pribl->fSideBySide = fFalse; */
	/* pribl->hrcpfld = hNil; */
	pribl->pgnStart = iNil;
	pribl->lineStart = iNil;
	pribl->iTypePic = iPicMacPict;	/* default to illegal, disregarded pic */
	pribl->fDisregardPic = fTrue;
	pribl->iGrid = iNil;
	/* pribl->hsttbGrid = hNil; */
	/* pribl->hstGrid = hNil; */
	/* pribl->stUsrInitl[0] = 0; */
	MeltHp();  /* for pribl */

	/* set up document management properties.
			note that this puts in default values which may be overriden
			by values in the rtf/foreign file
		*/
	if (docIn == docNil)
		ApplyDocMgmtNew (doc, fnNil);

	return hribl;
}


/*  %%Function:  FreeHribl  %%Owner:  bobz       */

FreeHribl(hribl)
struct RIBL **hribl;
{
	Assert (hribl != hNil);

	Assert ((*hribl)->hrgbStack != hNil);
	FreeH((*hribl)->hrgbStack);

	if ((*hribl)->hftcmap != hNil)
		FreeH((*hribl)->hftcmap);

	if ((*hribl)->hsttbGrid != hNil)
		FreeHsttb((*hribl)->hsttbGrid);

	if ((**hribl).hstGrid != hNil)
		FreeH((**hribl).hstGrid);

	if ((*hribl)->hrcpxetc != hNil)
		{
		if (((**(**hribl).hrcpxetc).docTemp) != docNil)
			DisposeDoc((**(**hribl).hrcpxetc).docTemp);

		FreeH((**hribl).hrcpxetc);
		}

	FreeH(hribl);
}




/* this is called out of DocCreateFn to convert a file using RTF. */

/* D O C  C R E A T E  F N  R T F */
/*  Fn is a file which contains RTF to be interpreted.  Create an opus
	doc which contains it.
*/

/*  %%Function:  DocCreateRtf  %%Owner:  bobz       */

DocCreateRtf(fn)
int fn;
{
	struct RIBL **hribl;
	int doc;
	BOOL fFinishRtf;
	struct PPR **hppr;
	struct DOD *pdod;

	if ((hribl = HriblCreateDoc(docNil)) == hNil)
		{
		return docNil;
		}

	pdod = PdodDoc ((*hribl)->doc);
	pdod->fn = fn;
	pdod->fFormatted = fTrue;
	Assert (pdod->dk == dkDoc);

	DisableInval();

	hppr = HpprStartProgressReport(mstConverting, NULL, nIncrPercent, fTrue);

	RtfGetChars(fn, hribl, hppr);

		/* if not risexit, did not complete doc and so cancel out */

#ifdef DEBUG
		{
		if (vdbs.fNoRtfConv)
			fFinishRtf = fTrue;
		else
			fFinishRtf = !(*hribl)->fCancel && (*hribl)->ris == risExit;
		}
#else

	fFinishRtf = !(*hribl)->fCancel && (*hribl)->ris == risExit;
#endif /* DEBUG */
	doc = DocCreateRtf1(fn, hribl, !fFinishRtf);

	if (!fFinishRtf)
		doc = docNil;  /* already disposed of */

	EnableInval();

	if (doc != docNil && doc != docCancel)
		{
		InvalCp2(doc, cp0, CpMacDocEdit(doc) /* dcp */, fTrue  /* fSetPlcUnk */);
		ChangeProgressReport (hppr, 100);
		}

	StopProgressReport (hppr, pdcRestore);
	return (doc);
}



/* D O C  C R E A T E  F N  R T F  1*/
/*  Fn is a file which contains RTF to be interpreted.  Create an opus
	doc which contains it.
*/

/*  %%Function:  DocCreateRtf1  %%Owner:  bobz       */

DocCreateRtf1(fn, hribl, fFail)
int fn;
struct RIBL **hribl;
BOOL fFail; /* used if creation should fail due to outside forces */
{
	int doc;
	struct DOD *pdod;
	CP cpMac;
	int fCancel;

	doc = (*hribl)->doc;

	Assert(PdodDoc(doc)->fn == fn);

#ifdef BZ
	CommSzNum(SzShared("DocCreateRtf fCancel after RtfGetChars"), (*hribl)->fCancel);
	CommSzNum(SzShared("DocCreateRtf ris after RtfGetChars"), (*hribl)->ris);
	CommSzNum(SzShared("DocCreateRtf fDiskFail after RtfGetChars"), vmerr.fDiskFail);
	CommSzNum(SzShared("DocCreateRtf fMemFail after RtfGetChars"), vmerr.fMemFail);
#endif /* BZTEST */

	if (fFail || (*hribl)->fCancel || vmerr.fDiskFail || vmerr.fMemFail)
		{
		DisposeDoc(doc);
		doc = docCancel; /* to avoid not enough mem to run opus message */

		/* report here, since memory will be freed. If initializing,
		   reset flag temporarily so error will be reported;
		   otherwise will get not enought memory to run Opus
		   message.
		*/

		if (vmerr.fMemFail)
			{
			ErrorNoMemory(eidNoMemOperation);

#ifdef NOTNEEDED    /* startup changed so just doc won't come up */
			vmerr.mat = matNil;
			vmerr.fHadMemAlert = fFalse;
#endif 

			}

		goto Cleanup;
		}

	Debug( vdbs.fCkDoc ? CkDoc(doc) : 0 );

	/* this prevents CRLF's building up at the end
	   of doc if there was already one at the end.
	   If there are 2, delete the second, which should not have
	   any properties attached.    
	*/


#ifdef BZTEST
	CommSzSz(SzShared("DocCreateRtf after RtfGetChars"), SzShared(""));
#endif /* BZTEST */

	if ((cpMac = CpMacDoc(doc)) > 2 * ccpEop)
		{
		CachePara(doc, cpMac - 2 * ccpEop);
		if (caPara.cpLim == cpMac - ccpEop)
			{
			struct CA ca;
			if (!FInTableVPapFetch(doc, caPara.cpFirst))
				FReplace(PcaSetDcp(&ca, doc, CpMacDocEdit(doc), ccpEop),
						fnNil, fc0, fc0);
			}

		Debug( vdbs.fCkDoc ? CkDoc(doc) : 0 );
#ifdef BZTEST
		CommSzSz(SzShared("DocCreateRtf after RtfGetChars and CachePara"), SzShared(""));
#endif /* BZTEST */

		}

	Assert (PfcbFn(fn)->fForeignFormat);

	/* handle side by side to table and field translation */
	RtfPostProcess(hribl, fn);

	/* so if we just close after convert, no save will be done */
	UndirtyDoc (doc);

Cleanup:
	FreeHribl(hribl);

	return doc;
}


/* handle side by side to table and field translation */
/*  %%Function:  RtfPostProcess  %%Owner:  bobz       */

RtfPostProcess(hribl, fn)
struct RIBL **hribl;
int fn;   /* the original fn of the file read in */
{
	struct CA ca;
	extern struct ITR	vitr;
	int doc;

	/* Note: recurse on subdocs, generally. For sbys, we are not doing
		subdocs, since we are not sure where one component (e.g., a
		footnote)  ends and another begins
	*/

	if ((**hribl).fSideBySide)
		{
#ifdef BZ
		CommSzLong(SzShared("Postprocessing size by side to cp: "),
				CpMacDoc((**hribl).doc));
#endif /* BZ */
		if (!FTableFromSideBySide(PcaSet(&ca, (**hribl).doc, cp0,
				CpMacDocEdit((**hribl).doc)), fn))
			{
			/* in this case, only the table conversion failed, and we
				leave the side by side, so don't cause the doc to be
				killed. Might fail without setting vmerr.
			*/
			ErrorNoMemory(eidNoMemOperation);
			if (vmerr.fMemFail)
				{
				vmerr.mat = matNil;
				vmerr.fHadMemAlert = fFalse;
				}
			}
		}

#ifdef INTL
		{
		doc = (*hribl)->doc;
		if (!FTranslFields(doc, fFalse /* fLocal */))
			{
			ErrorNoMemory(eidNoMemOperation);
			DisposeDoc(doc);
			}
		}
#endif /* INTL */
}


/*  %%Function:  RtfGetChars  %%Owner:  bobz       */

RtfGetChars(fn, hribl, hppr)
int fn;
struct RIBL **hribl;
struct PPR **hppr;
{
	FC fcLim;
	char *pch;
	int cch;
	char rgbBuf[cbSector];
	int iCheckPlc;

	FC fcNext = 0;
	FC fc = fc0;
	PN pn = pn0;
#define iCheckLim  10

	fcLim = PfcbFn(fn)->cbMac;

	EnablePreload(fn);

	if (FQueryAbortCheck())
		{
		(**hribl).fCancel = fTrue;
		goto EndGetChars;
		}

	iCheckPlc = 0;
	while (fc < fcLim && !vmerr.fDiskFail && !vmerr.fMemFail)
		{
		if (FQueryAbortCheck())
			(**hribl).fCancel = fTrue;
		if (fc >= fcNext)
			{
			ProgressReportPercent (hppr, (FC)0, fcLim, fc, &fcNext);
			}

		if (iCheckPlc++ > iCheckLim  Debug(|| vdbs.fCompressOften))
			{
			/* if the piece table is getting big ~32K, do a full save
					unless we are in the process of doing a table. Reset
					checklim if not > 32k or > 32k and not in a table.
					If > 32k and not in a table, leave on so save will be
					done asap.
				*/

			if (!(**hribl).pap.fInTable)
				{
				iCheckPlc = 0;
				CheckCompressDoc((**hribl).doc);
				}
			}

		if ((**hribl).fCancel)
			goto EndGetChars;

		bltbh(HpchGetPn(fn, pn), rgbBuf, cbSector);
		pch = rgbBuf;
		cch = (int) CpMin((FC)cbSector, fcLim - pn * cbSector);
		RtfIn(hribl, pch, cch);
#ifdef BZ
		CommSzNum(SzShared("ris after rtfin:"), (*hribl)->ris);
#endif /* BZTEST */
		pn++;
		fc += cbSector;
		}


	if (FQueryAbortCheck())
		(**hribl).fCancel = fTrue;

EndGetChars:

#ifdef BZTEST
	CommSzNum(SzShared("RtfGetChars fCancel at end"), (*hribl)->fCancel);
	CommSzNum(SzShared("RtfGetChars fDiskFail at end"), vmerr.fDiskFail);
#endif /* BZTEST */

	DisablePreload();

#ifdef DRTFSEP
		{
		CP cp;
		int doc = (**hribl).doc;

		for (cp = cp0; cp <  CpMacDoc(doc); cp = caSect.cpLim)
			{
			CacheSect(doc, cp);
			CommSzRgNum(SzShared("afer get chars, caSect: "), &caSect, 5);
			CommSzRgNum(SzShared("after get chars, vsepFetch: "), &vsepFetch, cwSEP);
			}
		caSect.doc = docNil;
		}
#endif
}


/*  %%Function:  ApplyStcEndOfDoc  %%Owner:  bobz       */

ApplyStcEndOfDoc(doc, stc)
int doc;
int stc;
{
	char rgb[2];
	struct CA ca;

	rgb[0] = sprmPStc;
	rgb[1] = stc;
	ca.cpLim = CpMacDoc(ca.doc = doc);
	ca.cpFirst = ca.cpLim - ccpEop;
	ApplyGrpprlCa(rgb, 2, &ca);
	caPara.doc = docNil;
}



/*  %%Function:  ResetPropsToDefault  %%Owner:  bobz       */

ResetPropsToDefault(pribl)
struct RIBL *pribl;
{
	/* note that pribl is a pointer to a heap object that may be
			invalidated (moved) after this routine completes.
				WIN - can't use pfn for native/toolbox calls, so make
				direct calls    */


#ifdef WIN
	RtfStandardChp(pribl->rgbChar);
	StandardPap(pribl->rgbPara);
	RtfStandardSep(pribl->rgbSect);
	RtfStandardTap(pribl->rgbTable);
#else
	(*pribl->pfnCharDefault)(pribl->rgbChar);
	(*pribl->pfnParaDefault)(pribl->rgbPara);
	(*pribl->pfnSectDefault)(pribl->rgbSect);
	(*pribl->pfnTableDefault)(pribl->rgbTable);
#endif /* WIN */

	SetBytes(pribl->rgbPara, 0, pribl->cbParaInfo);
}


/*  %%Function:  DoRtfTab  %%Owner:  bobz       */

DoRtfTab(hribl, val)
struct RIBL **hribl;
int val;
{
	char ch;
	int doc;

	doc = (**hribl).doc;
	ch = chTab;
	/* note pchp is a heap pointer. Still operating on assumption it
				is ok to pass heap pchp to FInsertrgch
			*/
	FInsertRgch(doc, CpMacDocEdit(doc), &ch, 1,
			&(**hribl).chp, 0);
}


/*  %%Function:  DoRtfParaEnd  %%Owner:  bobz       */

DoRtfParaEnd(hribl, val)
struct RIBL **hribl;
int val;
{
	int doc;
	struct PAP pap;
	CHAR *pch;

	if (val == chEop)
		pch = rgchEop;
	else  if (val == chTable)
		pch = rgchTable;

#ifdef DEBUG
	else
		Assert (fFalse);
#endif /* DEBUG */


	doc = (**hribl).doc;
	PapForEndSecPara(hribl, &pap);

	/* note pchp is a heap pointer. Still operating on assumption it
				is ok to pass heap pchp to FInsertrgch
			*/
	if (FInsertRgch(doc, CpMacDocEdit(doc), pch, (int)ccpEop,
			&(**hribl).chp, &pap))
		{
		if (val == chEop)
			{
			if ((**hribl).rds == rdsGrid)
				{
				FFormatTableHribl(hribl);
				(**hribl).cpRdsFirst = CpMacDocEdit(doc);
				}
			}
		}
}


/*  %%Function:  DoRtfSectEnd  %%Owner:  bobz       */

DoRtfSectEnd(hribl, val)
struct RIBL **hribl;
int val;
{
	struct CA ca;
	int doc = (**hribl).doc;
	struct PAP pap;
	struct CHP chp;
	struct SEP sep;
	CP cp;
	chp = (**hribl).chp;
	sep = (**hribl).sep;

	PapForEndSecPara(hribl, &pap);

#ifdef BZ
	CommSzNum(SzShared("DoRtfectEnd hribl.sep.grpfIhdt = "),
	        (**hribl).sep.grpfIhdt);
#endif /* BZ */

#ifdef DRTFSEP
	CommSzRgNum(SzShared("DoRtfSectEnd pre-KeyCmdSect hribl.sep: "), &(**hribl).sep, cwSEP);
#endif
	/* Note: for other users of CmdInsertSect1, the sep is for the
				text that FOLLOWS the chSect; we need it to be for the preceding
				text as we don't have the props for the following text yet. bz
				*/

	CmdInsertSect1(PcaPoint( &ca, doc, (cp = CpMacDocEdit(doc))), &sep,
			&chp, &pap, fFalse /* fSepNext */, fFalse /* fRM */);

	/* if we had headers in a sect, and then don't get a \sectd, this
			will keep the next section from mistakenly thinking it has
			its own headers if they don't get set. bz.
			*/
	(**hribl).sep.grpfIhdt = 0;
	/* cp is immediately before the section break */
	DoPageNum(hribl, cp);
		/* reset for next section regardless of whether we had page # */
 	(**hribl).dxaPgn = (**hribl).dyaPgn = 0;
 	(**hribl).fHdr =  (**hribl).fFtr = fFalse;


#ifdef DRTFSEP
	CommSzRgNum(SzShared("DoRtfSectEnd ca: "), &ca, 5);
	CommSzRgNum(SzShared("DoRtfectEnd hribl.sep: "), &(**hribl).sep, cwSEP);
#endif

}


PapForEndSecPara(hribl, ppap)
struct RIBL **hribl;
struct PAP *ppap;
{

	/* to avoid including 5 sdm include files... */
#define	uNinch		(0xffff)	// Unsigned.


	int ibrcl, ibrcp;

	*ppap = (**hribl).pap;

	if ((**hribl).fPermOK)
		ppap->stc = (**hribl).rgbStcPermutation[ppap->stc];
	else
		ppap->stc = stcNormal;

	/* map unwanted papa border codes  */
	/* will map l,r to bar and unknown combos to ninch */
	IbrclIbrcpFromBrcs(&ppap->brcTop, &ibrcl,
			&ibrcp, fTrue /* fMapLR */);
	/* clear (to none) for unknown; set real for bar */
	if (ibrcp == ibrcpBar || ibrcl == uNinch || ibrcp == uNinch )
		BrcsFromIbrclIbrcp(&ppap->brcTop, ibrcl,
            ibrcp == ibrcpBar ? ibrcpBar : ibrcpNone);

}


/*  %%Function:  DoPageNum  %%Owner:  bobz       */

DoPageNum(hribl, cp)
struct RIBL **hribl;
CP cp;
{
	struct CA ca;
	int doc = (**hribl).doc;

	if ((**hribl).dxaPgn || (**hribl).dyaPgn)
		{
		int fHdr, jc;
		unsigned xaPage;

		/* if only one specified, default the other */
		if (!(**hribl).dxaPgn)
			(**hribl).dxaPgn = pgnxyDef;
		else  if (!(**hribl).dyaPgn)
			(**hribl).dyaPgn = pgnxyDef;

		/* convert pgn info to put page fields in header/footer */

		CacheSect(doc, cp);
		fHdr = (**hribl).dyaPgn <= vsepFetch.dyaHdrTop;

		/* if a real header/footer would be replaced, bag out.
					We will replace linked h/f's
				*/

		if ((fHdr && (**hribl).fHdr) ||  (!fHdr && (**hribl).fFtr))
            return;
		ca = caSect;

		if (DocCreateScratch(docNil) == docNil)
            return;

		/* dxa is dist from right, not left corner */
		/* if < 1/4 page, right justify, if > 3/4. left, else center */
		xaPage = PdopDoc(doc)->xaPage;
		jc = (**hribl).dxaPgn < (xaPage >> 2) ? jcRight :
				(**hribl).dxaPgn > ((xaPage >> 2) * 3) ? jcLeft :
				jcCenter;
		CmdDoPgNum(fHdr, jc, &ca, fTrue /* fFromRTF */);
		ReleaseDocScratch();
		}
}



/*  %%Function:  DoRtfTableRowEnd  %%Owner:  bobz       */

DoRtfTableRowEnd(hribl, val)
struct RIBL **hribl;
int val;
{
	struct RIBL *pribl = *hribl;
	struct TAP tap;

	tap = pribl->tap;
	pribl->pap.fTtp = fTrue;
	pribl->pap.ptap = &tap;
	DoRtfParaEnd(hribl, val);
	(**hribl).pap.ptap = NULL;
	pribl->pap.fTtp = fFalse;
}


/*  %%Function:  DoQuoteForBinCode  %%Owner:  bobz       */

DoQuoteForBinCode(hribl, val)
struct RIBL **hribl;
int val;
{
	(**hribl).ris = risB4BinCode;
}


/*  %%Function:  RtfStandardDop  %%Owner:  bobz       */

RtfStandardDop(pdop)
struct DOP *pdop;
{
	/* this has to match the defaults in the RTF spec, so should not
	change even if other defaults change. Force all values in case
	the docNew values change.
		*/
	SetBytes(pdop, 0, cbDOP);

	pdop->xaPage = 12240;
	pdop->yaPage = 15840;
	pdop->dxaLeft = 1800;
	pdop->dxaRight = 1800;
	pdop->dyaTop = 1440;
	pdop->dyaBottom = 1440;
	pdop->dxaTab = 720;
	pdop->nFtn = 1;


#ifdef INEFFICIENT
	pdop->fpc = fpcEndnote;
	pdop->dxaGutter = 0;
	pdop->fFacingPages = fFalse;
	pdop->fWidowControl = fFalse;
	pdop->fFtnRestart = fFalse;
#endif /* INEFFICIENT */


	/* rtf doesn't specify these */

	pdop->fWide = fWideDopDef;
	pdop->grpfIhdt = grpfIhdtDopDef;
	pdop->fRevMarking = fRevMarkingDef;
	pdop->irmBar = irmBarDef;
	pdop->irmProps = irmPropsDef;
	pdop->dxaHotZ = dxaHotZDopDef;
}


/*  %%Function:  RtfStandardSep  %%Owner:  bobz       */

RtfStandardSep(psep)
struct SEP *psep;
{
	StandardSep(psep);
	/* psep->dxaLnn = 360; */
	psep->fEndnote = fFalse;  /* flag will turn it on */
}


/*  %%Function:  RtfStandardTap  %%Owner:  bobz       */

RtfStandardTap(ptap)
struct TAP *ptap;
{
	SetBytes(ptap, 0, cbTAP);
}


/*  %%Function:  SetBrcForIrrb  %%Owner:  bobz       */

SetBrcForIrrb(pbrc, irrbProp, val)
struct BRC *pbrc;
int irrbProp;
int val;
{
	switch (irrbProp)
		{
	case irrbRbrcBase:
		/* this maps hairline and dotted to single */
		pbrc->brcBase = rgwBrcBaseVals[BrclFromIbrc(val)];
		break;
	case irrbRbrdrsh:
		pbrc->fShadow = val;
		/* should never have shadow without a base type */
		if (pbrc->brcBase == brcNone)
			pbrc->brcBase = brcSingle;
		break;
	case irrbRbrsp:
		pbrc->dxpSpace = val / czaPoint;  /* Opus stores this in points */
#ifdef DEBUG
		break;
	default:
		Assert (fFalse);
#endif 

		}
}


/*  %%Function:  ChngToRdsHdr  %%Owner:  bobz       */

ChngToRdsHdr(hribl, val)
struct RIBL **hribl;
int val;
{
	int grpfIhdt;
#ifdef BZ
	CommSzNum(SzShared("dop.grpfIhdt at hdr change state = "),
			PdopDoc((**hribl).doc)->grpfIhdt);
	CommSzNum(SzShared("sep.grpfIhdt = "),
	        (**hribl).sep.grpfIhdt);
#endif /* BZ */

	grpfIhdt = (**hribl).sep.grpfIhdt;
	ResetPropsToDefault((*hribl));
	(**hribl).ihdt = val;
	(**hribl).cpRdsFirst = CpMacDocEdit((**hribl).doc);
	(**hribl).rds = rdsHdr;
	(**hribl).sep.grpfIhdt = grpfIhdt;
	/* this so pgnx, pgny can know whether to whomp a header/footer
					(does so only if linked and no real h/f entered)
				*/
	switch (val)
		{
	case ihdtBRight:
	case ihdtBFirst:
	case ihdtBLeft:
		(**hribl).fFtr = fTrue;
		break;
	case ihdtTRight:
	case ihdtTFirst:
	case ihdtTLeft:
		(**hribl).fHdr = fTrue;
		break;
		}
}


/*  %%Function:  ChngToRdsStylesheet  %%Owner:  bobz       */

ChngToRdsStylesheet(hribl, val)
struct RIBL **hribl;
int val;
{
	struct STSH stsh;
	int doc = (**hribl).doc;
	int stcp;

	ResetPropsToDefault(*hribl);
	(**hribl).cpRdsFirst = CpMacDocEdit(doc);
	(**hribl).rds = rdsStylesheet;
	(**hribl).estcp.stcBase = stcStdMin;
	(**hribl).fStcNextOK = fFalse;
}


/*  %%Function:  ChngToRdsNewStd  %%Owner:  bobz       */

ChngToRdsNewStd(hribl, val)
struct RIBL **hribl;
int val;
	/* general purpose destination change handler: basically, set
		the rds and the default props and the cpRdsFirst, then
		maybe a little more.
			*/
{
	struct FTCMAP **hftcmap;

	ResetPropsToDefault(*hribl);

	/* note that val is the rds code to set to */

	(**hribl).rds = val;
	(**hribl).cpRdsFirst = CpMacDocEdit((**hribl).doc);

	/* special processing (minimal only - otherwise do separate rtn) */

	switch (val)
		{
	case rdsFonttbl:
		if ((**hribl).hftcmap == hNil)
			/*  HplInit can fail. vmerr.fMemFail set so error will be reported */
			{
			if ((hftcmap = HAllocateCw(1 /* iMac */ + 1 /* iMax */
					+ 10 /* rgFtcFrom */ + 10 /* rgFtcTo */)) != hNil)
				{
				(*hftcmap)->iMac = 0;
				(*hftcmap)->iMax = 10;
				(**hribl).hftcmap = hftcmap;
				}
#ifdef DEBUG
			else
				ReportSz("Warning - font map allocation failure - all fonts will be default");

#endif
			}
		break;
	case rdsGridtbl:
		if ((**hribl).hsttbGrid == hNil)
			/* arbitrary estimate guess of 10 columns */
			/*  HsttbInit can fail. vmerr.fMemFail set so error will be reported */
			(**hribl).hsttbGrid = HsttbInit(10, fFalse/*fExt*/);


		if ((**hribl).hstGrid == hNil)
			/* 256 bytes for an st */
			if (((**hribl).hstGrid = HAllocateCw(128)) != hNil)
				{
				**((**hribl).hstGrid) = 1;  /* this keeps us on word boundaries */
#ifdef BZTEST
				CommSzNum(SzShared("Grid table set up - hsttbGrid = "), (**hribl).hsttbGrid);
				CommSzNum(SzShared("Grid table set up - hstGrid = "), (**hribl).hstGrid);
				CommSzNum(SzShared("Grid table set up - hstGrid[0] = "), **((**hribl).hstGrid));
#endif
				}
		/* else vmerr.fMemFail already set in FEnsureFreeCw */
		break;
	case rdsColortbl:
		(*hribl)->cRed = (*hribl)->cGreen = (*hribl)->cBlue = 0;
		(*hribl)->fNoColor = fTrue; /* so we can find Auto */
		break;
	case rdsCreatim:
	case rdsRevtim:
	case rdsPrintim:
	case rdsBuptim:

		/* fields that need times will clear the DTR here. The yr, mo, dom,
			hr, mint, and sec flags will load the DTR and when the state for
			these rds's is popped, we write the date info into the info block.
		*/
		/* be sure DTR is even word sized */
		Assert(sizeof (struct DTR) % sizeof (int) == 0);
		SetWords(&dtrRTF, 0, cwDTR);

		break;
		}
}


/*  %%Function:  ChngToRdsRtf  %%Owner:  bobz       */

ChngToRdsRtf(hribl, val)
struct RIBL **hribl;
int val;
{
	ResetPropsToDefault(*hribl);

	/* note: val is the rtf parameter, which opus sets to be its
		magic number. We don't use this now
	*/
}


/*  %%Function:  ChngToRdsFld  %%Owner:  bobz       */

ChngToRdsFld(hribl, val)
struct RIBL **hribl;
int val;
{
	struct RCPFLD **hrcpfld;
	struct RCPFLD *prcpfld;

	/* allocate area to hold start cp, dcpInst and dcpRslt for field */

	if ((hrcpfld = HAllocateCw(cwRCPFLD)) == hNil)
		{
		(**hribl).hrcpfld = hNil;
		/* vmerr.fMemFail already set in FEnsureFreeCw */
		return;
		}

	/* set up start cp for field, invalid val for dcpInst */
	prcpfld = *hrcpfld;  /* heap pointer! */
	FreezeHp();
	prcpfld->dcpInst = (CP)0;
	prcpfld->dcpRslt = (CP)0; /* noncalc'ed field will have 0 dcpRslt */
	SetWords(&prcpfld->rffld, 0, cwRFFLD);
	MeltHp();

	(**hribl).hrcpfld = hrcpfld;
	(**hribl).cpRdsFirst = CpMacDocEdit((**hribl).doc);

	(**hribl).rds = rdsField;
}


/*  %%Function:  ChngToRdsFldRslt  %%Owner:  bobz       */

ChngToRdsFldRslt(hribl, val)
struct RIBL **hribl;
int val;
{
	struct RFFLD  *prffld;
	struct RCPFLD *prcpfld;



#ifdef DEBUG
	Assert ((**hribl).hrcpfld != hNil);
	prcpfld = *((**hribl).hrcpfld);
	prffld = &(prcpfld->rffld);  /* local use heap pointer */
	FreezeHp();
	/* we are curently not supporting special handling of
			private result fields, which probably requires grabbing
			the text and converting to binary. Code will probably be
			added here when/if we do that.
	
				At the moment (8/88), we have private results in import fields,
				but we don't do anything with them, and in fact the field
				result should always be empty. Check at the point of fieldifying
				instead and comment this check out. bz  
	
		if (prffld->fPrivateResult)  <- commented out bz
			Assert (fFalse);
	
		*/


	MeltHp();
#endif

	(**hribl).rds = rdsFldRslt;
	(**hribl).cpRdsFirst = CpMacDocEdit((**hribl).doc);
}


/*  %%Function:  ChngToRdsFldIT  %%Owner:  bobz       */

ChngToRdsFldIT(hribl, val)
struct RIBL **hribl;
int val;
	/* field instruction handler
			*/
{

	/* note that val is the rds code to set to */

	(**hribl).rds = val;
	(**hribl).cpRdsFirst = CpMacDocEdit((**hribl).doc);

	Assert ((**hribl).hrcpfld != hNil);
}


/* ****
*  Description: Given an ASCII string containing a (base 10) number,
*      return the number represented.  Ignores leading and trailing spaces.
*      returns wError if value too large for int
** **** */
/*  %%Function:  WFromSzRTF  %%Owner:  bobz       */


int WFromSzRTF( szNumber )
CHAR * szNumber;
{
	CHAR * PchSkipSpacesPch();
	unsigned w = 0;
	BOOL fNeg = fFalse;
	int ch;

	szNumber = PchSkipSpacesPch( szNumber );

	if (*szNumber == '-')
		{
		fNeg = fTrue;
		szNumber = PchSkipSpacesPch(szNumber + 1);
		}

	while ( ((ch = *szNumber++) >= '0') && (ch <= '9') )
		{
		if ((w = (w * 10) + (ch - '0')) > czaMax)
			return wError;
		}
	return fNeg ? -w : w;
}


