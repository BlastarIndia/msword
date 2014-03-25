	include w2.inc
        include noxport.inc
        include consts.inc
        include structs.inc

createSeg	_LAYOUT2,layout2,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midLayout22	equ 21		 ; module ID, for native asserts
NatPause equ 1
endif

ifdef	NatPause
PAUSE	MACRO
	int 3
	ENDM
else
PAUSE	MACRO
	ENDM
endif

; EXTERNAL FUNCTIONS

externFP	<GetPlc>
externFP	<AbortLayout>
externFP	<CpFromCpCl>
externFP	<N_FInTableDocCp>
externFP	<CpMacDocEdit>
externFP	<IInPlcCheck>
externFP	<CpFirstTap>
externFP	<DxlFromTable>
externFP	<FTableHeight>
externFP	<FSavePhe>
externFP	<IInPlc>
externFP	<FGetValidPhe>
externFP	<LbcFormatChain>
externFP	<N_FAbortLayout>
externFP	<N_ClFormatLines>
externNP	<LN_CacheParaLbs>
externNP	<LN_CacheSectL>
externNP	<LN_XaFromXl>
externNP	<LN_YlFromYa>
externNP	<LN_NMultDivCzaInch>
externNP	<LN_PdodDocL>
externNP	<LN_LrpInPlCur>
externNP	<LN_IInPlc>
externNP	<LN_CpMacDocPlbs>
externNP	<LN_CpPlc>

ifdef DEBUG
externFP	<AssertProcForNative>
externFP	<ScribbleProc>
externFP	<S_FAbortLayout>
externFP	<S_ClFormatLines>
externFP	<S_FInTableDocCp>
externFP	<ShowLbsProc>
endif ;DEBUG


sBegin  data

; EXTERNALS
externW     vfls
externW     vfli
externW     caParaL
externW     vfti
externW     caSect
externW     vlm
externW     vpapFetch
externW     vsepFetch
externW     caTap
externW     vdocTemp
externW     vylTopPage
externW     vcBalance

ifdef DEBUG
externW     vdbs
externW     vfInFormatPage
endif ;DEBUG

sEnd    data

; CODE SEGMENT _LAYOUT2

sBegin	layout2
	assumes cs,layout2
        assumes ds,dgroup
        assumes ss,dgroup


;-------------------------------------------------------------------------
;	LbcFormatPara(plbs, dylFill, clLim)
;-------------------------------------------------------------------------
;/* L b c  F o r m a t	P a r a */
;NATIVE int LbcFormatPara(plbs, dylFill, clLim)
;struct LBS *plbs;
;int dylFill, clLim;
;{
;/* adds rectangle starting at lbs and ending before dylFill is exhausted and
;   clLim is reached; advances lbs; returns lbc:
;	 lbcEndOfSect
;	 lbcEndOfColumn
;	 lbcEndOfPage
;	 lbcEndOfDoc
;	 lbcYlLim
;	 lbcNil means para fits
;   saves the height information obtained in the process. advances lbs
;*/
;	 YLL dyllCur, dyllAdvance;
;	 YLL dyllRemain;
;	 int dylLine, dylBefore, dylT, dylMax;
;	 int dcl = -1, dxlT;
;	 int fParaStart, fEndBreak = fFalse, fChain = fFalse;
;	 int cl, clT, lbc;
;	 int fLnn, fTableRow = fFalse;
;	 int fNoYlLimit = (dylFill == ylLarge);
;	 int dylAbove, dylBelow, dylOverlapNew = plbs->dylOverlap;
;	 int doc = plbs->doc;
;	 struct PAD pad;
;	 struct PAD **hplcpad;
;	 struct DOD **hdod;
;	 int fShort;
;	 struct PHE phe;
;	 CP cpFirst, cpLim, cpMac, cpMacDoc;
;	 struct LNH lnh, lnhT;
;	 LRP lrp;
;	 struct LR lr;
;#ifdef DEBUG
;	 CP cpStart = plbs->cp;
;	 int clStart = plbs->cl;
;#endif /* DEBUG */

maskFChainLocal     = 1
maskFTableRowLocal  = 2
maskFParaStartLocal = 4

; %%Function:N_LbcFormatPara %%Owner:BRADV
cProc	N_LbcFormatPara,<PUBLIC,FAR>,<si,di>
	ParmW	plbs
	ParmW	dylFill
	ParmW	clLim

	LocalW	dcl
	LocalW	fEndBreak	;high byte contains fChain, fTableRow
	LocalW	lbc
	LocalW	fShort
	LocalW	fLnn
	LocalW	fNoYlLimit
	LocalW	clVar
	LocalW	dylText
	LocalW	dylBefore
	LocalW	dylAbove
	LocalW	dylBelow
	LocalW	dylOverlapNew
	LocalD	dyllRemain
	LocalD	cpLim
	LocalD	dyllCur
	LocalD	dyllAdvance
	LocalD	cpMacDocVar
	LocalV	phe, cbPheMin
	LocalV	pad, cbPadMin
	LocalV	lnh, cbLnhMin
	LocalV	lnhT, cbLnhMin
	LocalV	lr, cbLrMin
	LocalV	rgw, cbPAPBaseScan
ifdef DEBUG
	LocalD	cpStart
	LocalW	clStart
endif ;DEBUG

cBegin

	xor	ax,ax
	mov	[fEndBreak],ax	    ;also clears fChain, fTableRow
	dec	ax
	mov	[dcl],ax
	cmp	[dylFill],ylLarge
	je	LFP00
	inc	ax
LFP00:
	mov	[fNoYlLimit],ax
	mov	si,[plbs]
	mov	ax,[si.dylOverlapLbs]
	mov	[dylOverlapNew],ax
ifdef DEBUG
	push	[si.LO_cpLbs]
	pop	[OFF_cpStart]
	push	[si.HI_cpLbs]
	pop	[SEG_cpStart]
	push	[si.clLbs]
	pop	[clStart]
endif ;DEBUG

;	StartProfile();
;	Assert(vfInFormatPage);
ifdef DEBUG
	cmp	[vfInFormatPage],fFalse
	jne	LFP01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midLayout22
	mov	bx,1028
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LFP01:
endif ;/* DEBUG */

;	CacheParaL(doc, plbs->cp, plbs->fOutline);
	;LN_CacheParaLbs takes plbs in si
	;and performs CacheParaL(plbs->doc, plbs->cp, plbs->fOutline).
	;ax, bx, cx, dx are altered.
	call	LN_CacheParaLbs

;	cpLim = caParaL.cpLim;
	mov	ax,[caParaL.LO_cpLimCa]
	mov	[OFF_cpLim],ax
	mov	ax,[caParaL.HI_cpLimCa]
	mov	[SEG_cpLim],ax

;	cpMacDoc = CpMacDocPlbs(plbs);
	;LN_CpMacDocPlbs performs CpMacDocPlbs(si).
	;ax, bx, cx, dx are altered.
	call	LN_CpMacDocPlbs
	mov	[OFF_cpMacDocVar],ax
	mov	[SEG_cpMacDocVar],dx

;	bltLrp(LrpInPl(plbs->hpllr, plbs->ilrCur), &lr, sizeof(struct LR));
	;LFP82 performs
	;bltLrp(LrpInPl(plbs->hpllr, plbs->ilrCur), &lr, sizeof(struct LR));
	;ax, bx, cx, dx, di are altered.
	call	LFP82

;	if (lr.lrk == lrkAbsHit)
;		{
	cmp	[lr.lrkLr],lrkAbsHit
	jne	LFP03

;		if (dylFill != ylLarge)
;			dylFill = lr.yl + lr.dyl - plbs->ylColumn;
	cmp	[dylFill],ylLarge
	je	LFP02
	mov	ax,[lr.ylLr]
	add	ax,[lr.dylLr]
	sub	ax,[si.ylColumnLbs]
	mov	[dylFill],ax
LFP02:

;		if (plbs->cl != 0)
;			{
	cmp	[si.clLbs],0
	je	LFP03

;			/* for absolute-affected rectangles, we need raw
;			   cps since we can't save rectangle */
;			plbs->cp = CpFromCpCl(plbs, fTrue);
;			plbs->cl = 0;
	mov	ax,fTrue
	cCall	CpFromCpCl,<si, ax>
	;LFP76 performs:
	;plbs->cl = bx = 0;
	;plbs->cp = dx:ax;
	;plbs is assumed to be in si.
	;bx is altered.
	call	LFP76

;			}
;		}
LFP03:

;		if (FAbortLayout(plbs->fOutline, plbs))
;			{
;			AbortLayout();
;			}
	mov	al,[si.fOutlineLbs]
	cbw
	push	ax
	push	si
ifdef DEBUG
	cCall	S_FAbortLayout,<>
else ;!DEBUG
	push	cs
	call	near ptr N_FAbortLayout
endif ;!DEBUG
	or	ax,ax
	je	LFP06
	cCall	AbortLayout,<>
LFP06:

;	vfls.fFirstPara = plbs->fFirstPara;
	mov	al,[si.fFirstParaLbs]
	mov	[vfls.fFirstParaFls],al

;	fTableRow = FInTableVPapFetch(plbs->doc, plbs->cp);
;	if (plbs->fOutline)
;		caParaL.doc = docNil;	/* props got refreshed */
;	CacheParaL(doc, plbs->cp, plbs->fOutline);
;#define FInTableVPapFetch(doc, cp)  (vpapFetch.fInTable && FInTableDocCp(doc, cp))
	mov	al,[vpapFetch.fInTablePap]
	or	al,al
	je	LFP065
	push	[si.docLbs]
	push	[si.HI_cpLbs]
	push	[si.LO_cpLbs]
ifdef DEBUG
	cCall	S_FInTableDocCp,<>
else ;not DEBUG
	cCall	N_FInTableDocCp,<>
endif ;DEBUG
	push	ax	;save result of FInTableDocCp
	errnz	<fFalse>
	xor	ax,ax
	cmp	[si.fOutlineLbs],al
	je	LFP063
	errnz	<hNil>
	mov	[caParaL.docCa],ax
LFP063:
	;LN_CacheParaLbs takes plbs in si
	;and performs CacheParaL(plbs->doc, plbs->cp, plbs->fOutline).
	;ax, bx, cx, dx are altered.
	call	LN_CacheParaLbs
	pop	cx	;restore result of FInTableDocCp
	jcxz	LFP065
	or	bptr ([fEndBreak+1]),maskFTableRowLocal
LFP065:

;	hdod = mpdochdod[doc];
;	fShort = (*hdod)->fShort;
	mov	bx,[si.docLbs]
	call	LN_PdodDocL
	mov	al,[bx.fShortDod]
	mov	bptr ([fShort]),al

;/* in outline mode, ignore paragraphs not being shown */
;	if (plbs->fOutline && !fShort && (hplcpad = (*hdod)->hplcpad) != hNil)
;		{
	cmp	[si.fOutlineLbs],fFalse
	je	LFP08
	or	al,al
	jne	LFP08
	mov	cx,[bx.hplcpadDod]
	errnz	<hNil>
	jcxz	LFP08

;		GetPlc(hplcpad, IInPlc(hplcpad, plbs->cp), &pad);
	push	cx	    ;argument for GetPlc
	cCall	IInPlc,<cx, [si.HI_cpLbs], [si.LO_cpLbs]>
	push	ax
	lea	bx,[pad]
	push	bx
	cCall	GetPlc,<>

;		if (!pad.fShow)
;			{
	test	[pad.fShowPad],maskFShowPad
	jne	LFP08

;			/* collapsed text */
;			plbs->cl = 0;
;			lbc = ((plbs->cp = cpLim) >= cpMacDoc) ? lbcEndOfDoc : lbcNil;
	;LFP74 performs:
	;plbs->cl = bx = 0;
	;dx:ax = plbs->cp = cpLim;
	;plbs is assumed to be in si.
	;ax, bx, dx are altered.
	call	LFP74
	cmp	ax,[OFF_cpMacDocVar]
	mov	cx,dx
	sbb	cx,[SEG_cpMacDocVar]
	errnz	<lbcNil>
	jl	LFP07
	errnz	<(lbcEndOfDoc XOR lbcNil) AND 0FF00h>
	mov	bl,lbcEndOfDoc
LFP07:
	mov	bptr ([lbc]),bl

;			vfls.ca.doc = docNil;
;			vfls.ca.cpLim = cpLim;
	mov	[vfls.caFls.LO_cpLimCa],ax
	mov	[vfls.caFls.HI_cpLimCa],dx
	errnz	<docNil>
	xor	ax,ax
	mov	[vfls.caFls.docCa],ax

;			goto LAdjustDyl;
;			}
	jmp	LAdjustDyl

;		}
LFP08:

;/* chained across absolute object */
;/* some of the test for lnn was done by FAssignLr */
;	fLnn = lr.lnn != lnnNil && IfWinElse(fTrue, !vpapFetch.fSideBySide) &&
;		FLnnPap(vpapFetch) && !fTableRow;
;#define FLnnPap(pap)		 (!pap.fNoLnn)
	errnz	<fFalse>
	xor	di,di
	cmp	[lr.lnnLr],lnnNil
	je	LFP09
	mov	al,[vpapFetch.fNoLnnPap]
        or      al,al
	jne	LFP09
	test	bptr ([fEndBreak+1]),maskFTableRowLocal
	jne	LFP09
	errnz	<fTrue - fFalse - 1>
	inc	di
LFP09:
	mov	[fLnn],di

;	if (plbs->ilrFirstChain != ilrNil)
;		{
	cmp	[si.ilrFirstChainLbs],ilrNil
	je	LFP10

;		fChain = fTrue;
	or	bptr ([fEndBreak+1]),maskFChainLocal

;		lbc = LbcFormatChain(plbs, cpLim, dylFill, clLim, fLnn);
	cCall	LbcFormatChain,<si, [SEG_cpLim], [OFF_cpLim], [dylFill], [clLim], di>
	mov	bptr ([lbc]),al

;		dylOverlapNew = plbs->dylOverlap;
	mov	ax,[si.dylOverlapLbs]
	mov	[dylOverlapNew],ax

;		goto LSetLbc;
	jmp	LSetLbc

;		}
LFP10:

;	cl = plbs->cl;
;	lbc = lbcNil;
;	CacheSectL(caParaL.doc, caParaL.cpFirst, plbs->fOutline);
	mov	bptr ([lbc]),lbcNil
	mov	bx,[caParaL.docCa]
	mov	ax,[caParaL.LO_cpFirstCa]
	mov	dx,[caParaL.HI_cpFirstCa]
	mov	cl,[si.fOutlineLbs]
	xor	ch,ch
	;LN_CacheSectL takes doc in bx, cp in dx:ax, fOutline in cx
	;ax, bx, cx, dx are altered.
	call	LN_CacheSectL
	mov	ax,[si.clLbs]
	mov	[clVar],ax

;	fParaStart = (cl == 0 && plbs->cp == caParaL.cpFirst);
;	dylBefore = fParaStart ? YlFromYa(vpapFetch.dyaBefore) : 0;
ifdef DEBUG
	test	bptr ([fEndBreak+1]),maskFParaStartLocal
	je	LFP11
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midLayout22
	mov	bx,1029
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LFP11:
endif ;/* DEBUG */
	or	ax,ax
	jne	LFP12
	;LFP77 performs plbs->cp == caParaL.cpFirst.
	;plbs is assumed in si.  Only ax is altered.
	call	LFP77
	jne	LFP12
	or	bptr ([fEndBreak+1]),maskFParaStartLocal
	;LN_YlFromYa performs YlFromYa(ax); ax, bx, cx, dx are altered.
	mov	ax,[vpapFetch.dyaBeforePap]
	call	LN_YlFromYa
	db	03dh	;turns next "xor ax,ax" into "cmp ax,immediate"
LFP12:
	xor	ax,ax
	mov	[dylBefore],ax

;	dylOverlapNew = 0;
	xor	ax,ax
	mov	[dylOverlapNew],ax

;/* ignore heights in some cases */
;	if (plbs->fOutline || lr.lrk == lrkAbsHit || plbs->doc == vdocTemp ||
;		fTableRow)
;		SetWords(&phe, 0, cwPHE);
	errnz	<fFalse>
	cmp	[si.fOutlineLbs],al
	jne	LFP13
	cmp	[lr.lrkLr],lrkAbsHit
	je	LFP13
	mov	ax,[vdocTemp]
	cmp	ax,[si.docLbs]
	je	LFP13
	test	bptr ([fEndBreak+1]),maskFTableRowLocal
	je	LFP14
LFP13:
	push	ds
	pop	es
	xor	ax,ax
	lea	di,[phe]
	errnz	<cbPheMin - 6>
	stosw
	stosw
	stosw
	jmp	short LHavePhe
Ltemp013:
	jmp	LEmptyPara
LFP14:

;/* if we don't know the para's height, calculate it now and save the height in
;   hplcphe so we (hopefully) don't have to do this again */
;	else if (!FGetValidPhe(doc, plbs->cp, &phe) ||
	lea	ax,[phe]
	cCall	FGetValidPhe,<[si.docLbs], [si.HI_cpLbs], [si.LO_cpLbs], ax>
	or	ax,ax
	je	LFP15

;		phe.dxaCol != XaFromXl(lr.dxl))
;		{
	;LN_XaFromXl performs XaFromXl(ax); ax, bx, cx, dx are altered.
	mov	ax,[lr.dxlLr]
	call	LN_XaFromXl
	cmp	ax,[phe.dxaColPhe]
	je	LHavePhe
LFP15:

;		if (plbs->cp != caParaL.cpFirst)
;			{
;			SetWords(&phe, 0, cwPHE);
;			goto LHavePhe;
;			}
	;LFP77 performs plbs->cp == caParaL.cpFirst.
	;plbs is assumed in si.  Only ax is altered.
	call	LFP77
	jne	LFP13

;		/* limit the number of lines: 255 is the largest clMac that will fit
;		   in phe and represents 53" of 15-pt lines */
;		clT = ClFormatLines(plbs, cpLim, ylLarge, ylLarge, 255, lr.dxl, fFalse, lr.lrk != lrkAbs);
	push	si
	push	[SEG_cpLim]
	push	[OFF_cpLim]
	mov	ax,ylLarge
	push	ax
	push	ax
	errnz	<ylLarge - 03FFFh>
	mov	ah,0
	push	ax
	push	[lr.dxlLr]
	errnz	<fFalse>
	xor	ax,ax
	push	ax
	mov	al,[lr.lrkLr]
	sub	al,lrkAbs
	push	ax
ifdef DEBUG
	cCall	S_ClFormatLines,<>
else ;!DEBUG
	push	cs
	call	near ptr N_ClFormatLines
endif ;!DEBUG

;		if (vfls.fPageBreak || (clT != 0 && vfls.clMac == clNil))
;			SetWords(&phe, 0, cwPHE);
;		else if (clT == 0)
;			goto LEmptyPara;
	cmp	[vfls.fPageBreakFls],fFalse
	jne	LFP13
	or	ax,ax
	je	Ltemp013
	cmp	[vfls.clMacFls],clNil
	je	LFP13

;		else if (vfls.fVolatile)
;			/* para has vanished chars but we know its height */
;			/* FUTURE chrism: remember vanished state (dsb) */
;			phe = vfls.phe;
	cmp	[vfls.fVolatileFls],fFalse
	jne	LFP16

;		else
;			{
;			/* para is not interrupted, has no vanished chars */
;			if (!FSavePhe(&vfls.phe))
;				{
	mov	ax,dataoffset [vfls.pheFls]
	cCall	FSavePhe,<ax>
	or	ax,ax
	jne	LFP16

;				AbortLayout();
;				}
	cCall	AbortLayout,<>
LFP16:

;			phe = vfls.phe;
;			}
	;LFP80 performs phe = vfls.phe.  No registers are altered.
	call	LFP80

;		}
;/* here we have a phe; it may or may not be of use */
;LHavePhe:
;/* first para for LR */
;	if (lr.cp == cpNil)
;		{
LHavePhe:
	errnz	<LO_cpNil - 0FFFFh>
	errnz	<HI_cpNil - 0FFFFh>
	mov	ax,[lr.LO_cpLr]
	and	ax,[lr.HI_cpLr]
	inc	ax
	jne	LFP18

;		Assert(lr.lrk != lrkAbs);
ifdef DEBUG
	cmp	[lr.lrkLr],lrkAbs
	jne	LFP17
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midLayout22
	mov	bx,1030
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LFP17:
endif ;/* DEBUG */

;		lr.cp = plbs->cp;
	mov	ax,[si.LO_cpLbs]
	mov	[lr.LO_cpLr],ax
	mov	ax,[si.HI_cpLbs]
	mov	[lr.HI_cpLr],ax

;		lr.clFirst = plbs->cl;
	mov	ax,[si.clLbs]
	mov	[lr.clFirstLr],ax

;		lr.lrs = lrsNormal;
	mov	[lr.lrsLr],lrsNormal

;/* ignore space before at top of page, and here's the rule:
;   not table or side by side, never at top of section, only in 1-column
;   sections, not page break before, and top not affected by apo */
;		if (lr.yl == vylTopPage && dylBefore != 0 && fParaStart &&
;			!fTableRow && !lr.fConstrainTop &&
;			!vpapFetch.fPageBreakBefore && Mac(!vpapFetch.fSideBySide &&)
;			vsepFetch.ccolM1 == 0 && lr.cp != caSect.cpFirst)
;			{
	mov	ax,[vylTopPage]
	cmp	[lr.ylLr],ax
	jne	LFP175
	mov	cx,[dylBefore]
	jcxz	LFP175
	mov	al,bptr ([fEndBreak+1])     ;Local flags
	mov	ah,[lr.fConstrainTopLr]
	and	ax,maskFParaStartLocal+maskFTableRowLocal+(maskFConstrainTopLr SHL 8)
	cmp	ax,maskFParaStartLocal
	jne	LFP175
	mov	ax,[vsepFetch.ccolM1Sep]
	or	al,ah
	jne	LFP175
	errnz	<fFalse>
	cmp	[vpapFetch.fPageBreakBeforePap],al
	jne	LFP175
	mov	ax,[caSect.LO_cpFirstCa]
	sub	ax,[lr.LO_cpLr]
	jne	LFP173
	mov	ax,[caSect.HI_cpFirstCa]
	sub	ax,[lr.HI_cpLr]
	je	LFP175
LFP173:

;			lr.yl -= dylBefore;
	sub	[lr.ylLr],cx

;			lr.fSpaceBefore = fTrue;
	or	[lr.fSpaceBeforeLr],maskFSpaceBeforeLr

;			}
LFP175:

;		plbs->yl = lr.yl;	/* update to current position */
	mov	ax,[lr.ylLr]
	mov	[si.ylLbs],ax

;		}
LFP18:

;	InvalFli();	      /* clear fli cache */
;#define InvalFli()	 vfli.ww = wwNil
	mov	[vfli.wwFli],wwNil

;	dyllRemain = fNoYlLimit ? yllLarge :
;		(YLL)dylFill - plbs->yl + plbs->ylColumn; /* space left */
	mov	ax,0FFFFh
	mov	dx,07FFFh
	test	[fNoYlLimit],ax
	jnz	LFP19
	mov	ax,[dylFill]
	sub	ax,[si.ylLbs]
	add	ax,[si.ylColumnLbs]
	cwd
LFP19:
	mov	[OFF_dyllRemain],ax
	mov	[SEG_dyllRemain],dx

;/* table */
;	if (fTableRow)
;		{
	test	bptr ([fEndBreak+1]),maskFTableRowLocal
	jne	Ltemp014
	jmp	LFP24
Ltemp015:
	jmp	LFP264
Ltemp014:

;		Assert(plbs->cl == 0);
ifdef DEBUG
	;Do this assert with a call so as not to mess up short jumps
	call	LFP95
endif ;/* DEBUG */

;		if (lr.lrk == lrkAbsHit)
;			{
	cmp	[lr.lrkLr],lrkAbsHit
	jne	LFP195

;			dxlT = DxlFromTable(plbs->ww, doc, plbs->cp);
	push	[si.wwLbs]
	push	[si.docLbs]
	push	[si.HI_cpLbs]
	push	[si.LO_cpLbs]
	cCall	DxlFromTable,<>

;			if (dxlT > lr.dxl && clLim == clMax &&
;				(lr.fConstrainLeft || lr.fConstrainRight))
;				{
;				lbc = lbcYlLim;
;				goto LNoChange;
;				}
	cmp	ax,[lr.dxlLr]
	jle	LFP195
	cmp	[clLim],clMax
	jne	LFP195
	errnz	<(fConstrainLeftLr) - (fConstrainRightLr)>
	test	[lr.fConstrainLeftLr],maskfConstrainLeftLr + maskfConstrainRightLr
	jne	Ltemp015

;			}
LFP195:
; 		if (!FTableHeight(plbs->ww, doc, plbs->cp, vlm == lmBRepag/*fAbortOK*/,
; 				!plbs->fPrevTable, fTrue, &dylText, &dylAbove, &dylBelow))
; 			AbortLayout();
; 		dyllAdvance = dyllCur = dylText + dylAbove + dylBelow;
	push	[si.wwLbs]
	push	[si.docLbs]
	push	[si.HI_cpLbs]
	push	[si.LO_cpLbs]
	mov	ax,fTrue ; we'll use this a couple of times...
	mov	cx,ax
	cmp	[vlm],lmBRepag
	jz	LFP197
	errnz	<fTrue - 1>
	dec	cx
LFP197:
	push	cx	; fAbort
	;Assembler note: dcl is set to one below in the C source
	errnz	<fTrue - 1>
	mov	[dcl],ax
	cwd
	errnz	<fTrue - 1>
	cmp	[si.fPrevTableLbs],al
	adc	dx,dx
	push	dx
	push	ax
	lea	ax,[dylText]
	push	ax
	lea	ax,[dylAbove]
	push	ax
	lea	ax,[dylBelow]
	push	ax
	cCall	FTableHeight,<>

;		dyllCur = dylText + dylAbove + dylBelow;
	mov	ax,[dylText]
	add	ax,[dylAbove]
	add	ax,[dylBelow]
	cwd
	mov	[OFF_dyllCur],ax
	mov	[SEG_dyllCur],dx

;		dyllAdvance = dyllCur;
;		dcl = 1;
;		if (plbs->fPrevTable)
;			{
;			if (plbs->cp == lr.cp)
;				/* 1st thing in lr is table following table */
;				lr.yl -= plbs->dylBelowTable;
;			dyllAdvance -= plbs->dylBelowTable;
;			}
;		else if (plbs->cp == lr.cp)
;			lr.fForceFirstRow = fTrue;
	;Assembler note: dcl is set to one above in the assembler version
	mov	bx,[lr.LO_cpLr]
	mov	cx,[lr.HI_cpLr]
	sub	bx,[si.LO_cpLbs]
	sbb	cx,[si.HI_cpLbs]
	or	cx,bx	; cx = fNeqCps
	mov	bx,[si.dylBelowTableLbs]
	cmp	[si.fPrevTableLbs],fFalse
	jnz	LFP20
	xor	bx,bx	; make subtractions below virtual nop's
	or	cx,cx
	jnz	LFP21	; nothing to do
	or	[lr.fForceFirstRowLr],maskfForceFirstRowLr
LFP20:
	sub	ax,bx
	sbb	dx,0
	jcxz	LFP205
	xor	bx,bx	; nop next subtraction
LFP205:
	sub	[lr.ylLr],bx
LFP21:
	mov	[OFF_dyllAdvance],ax
	mov	[SEG_dyllAdvance],dx

;		if (lr.tSoftBottom == tPos)
;			dyllRemain = plbs->ylMaxLr - plbs->yl;
	;LFPa2 compares lr.tSoftBottom with tPos.
	;only flags are altered.
	call	LFPa2
	jne	LFP22
	mov	bx,[si.ylMaxLrLbs]
	sub	bx,[si.ylLbs]
	sbb	cx,cx		;sign extend to long
	mov	[OFF_dyllRemain],bx
	mov	[SEG_dyllRemain],cx
LFP22:

;		if (dyllAdvance > dyllRemain)
;			{
	xchg	ax,bx
	mov	cx,dx
	cmp	[OFF_dyllRemain],bx
	mov	ax,[SEG_dyllRemain]
	sbb	ax,cx
	jge	LFP23


;			lbc = lbcYlLim;
	mov	bptr ([lbc]),lbcYlLim

;			if (vylTopPage + dyllAdvance < plbs->ylMaxColumn)
;				{
	mov	ax,[vylTopPage]
	cwd
	add	bx,ax
	adc	cx,dx
	; Assert (plbs->ylMaxColumn >= 0); so we can sign extend with 0
ifdef DEBUG
	;Do this assert with a call so as not to mess up short jumps
	call	LFP97
endif ;/* DEBUG */
	sub	bx,[si.ylMaxColumnLbs]
	sbb	cx,0
	jge	LFP23

;				Assert(dylFill != ylLarge);	 /* are we lying? */
ifdef DEBUG
	;Do this assert with a call so as not to mess up short jumps
	call	LFP93
endif ;/* DEBUG */
;				if (lr.lrk != lrkNormal || plbs->yl != vylTopPage)
;					goto LNoChange;
;				}
;			}
	cmp	[lr.lrkLr],lrkNormal
	jne	Ltemp002
	cmp	ax,[si.ylLbs]
	jne	Ltemp002
LFP23:

;		CpFirstTap(doc, plbs->cp);
	cCall	CpFirstTap,<[si.docLbs], [si.HI_cpLbs], [si.LO_cpLbs]>

;		plbs->cp = caTap.cpLim;
	mov	ax,[caTap.LO_cpLimCa]
	mov	[si.LO_cpLbs],ax
	mov	ax,[caTap.HI_cpLimCa]
	mov	[si.HI_cpLbs],ax

;		fLnn = fFalse;
;		plbs->dylBelowTable = dylBelow;
	mov	[fLnn],fFalse
	mov	ax,[dylBelow]
	mov	[si.dylBelowTableLbs],ax

;		}
	jmp	LHaveDylCur
Ltemp016:
	jmp	LFormatPara
Ltemp017:
	jmp	LFP38
Ltemp002:
	jmp	LNoChange

;	else
LFP24:

;/* all equal lines. this is the common, important, fast case */
;	    if (!phe.fDiffLines)
;		{
	test	[phe.fDiffLinesPhe],maskFDiffLinesPhe
	jne	Ltemp017

;		dylLine = YlFromYa(phe.dyaLine);
	;LN_YlFromYa performs YlFromYa(ax); ax, bx, cx, dx are altered.
	mov	ax,[phe.dyaLinePhe]
	call	LN_YlFromYa
	xchg	ax,di

;		if (dylLine == 0 || plbs->cp != caParaL.cpFirst)
;			goto LFormatPara;	/* invalid phe */
	or	di,di
	je	Ltemp016
	;LFP77 performs plbs->cp == caParaL.cpFirst.
	;plbs is assumed in si.  Only ax is altered.
	call	LFP77
	jne	Ltemp016

;		ClFormatLines(plbs, cp0, 0, 0, 0, lr.dxl, fFalse, fFalse);
;		vfls.phe = phe;
	;LFP79 performs:
	;ClFormatLines(plbs, cp0, 0, 0, 0, lr.dxl, fFalse, fFalse);
	;vfls.phe = phe;
	;ax, bx, cx, dx are altered.
	call	LFP79

;		clLim = min(clLim, vfls.clMac = phe.clMac);
	mov	al,[phe.clMacPhe]
	xor	ah,ah
	mov	[vfls.clMacFls],ax
	cmp	[clLim],ax
	jle	LFP25
	mov	[clLim],ax
LFP25:

;		dyllRemain -= dylBefore;
	mov	bx,[OFF_dyllRemain]
	mov	cx,[SEG_dyllRemain]
	mov	ax,[dylBefore]
	cwd
	sub	bx,ax
	sbb	cx,dx

;		if (lr.tSoftBottom == tPos && clLim == clMax)
;			{
;			/* dyllRemain is long for OPUS and PCWORD - can't use min */
;			dyllRemain += dylLine - 1;
;			if (dyllRemain > plbs->ylMaxLr - plbs->yl)
;				dyllRemain = plbs->ylMaxLr - plbs->yl;
;			}
	;LFPa2 compares lr.tSoftBottom with tPos.
	;only flags are altered.
	call	LFPa2
	jne	LFP26
	cmp	[clLim],clMax
	jne	LFP26
	mov	ax,di
	dec	ax
	cwd
	add	bx,ax
	adc	cx,dx
	mov	ax,[si.ylMaxLrLbs]
	sub	ax,[si.ylLbs]
	cwd
	cmp	ax,bx
	push	dx
	sbb	dx,cx
	pop	dx
	jge	LFP26
	xchg	ax,bx
	mov	cx,dx
LFP26:

;		if (dylLine > dyllRemain)
;			{
;			lbc = lbcYlLim;
;			Assert(dylFill != ylLarge);	 /* are we lying? */
;			goto LNoChange;
;			}
	mov	[OFF_dyllRemain],bx
	mov	[SEG_dyllRemain],cx
	mov	ax,di
	cwd
	cmp	bx,ax
	mov	ax,cx
	sbb	ax,dx
	jge	LFP265
ifdef DEBUG
	;Do this assert with a call so as not to mess up short jumps
	call	LFP93
endif ;/* DEBUG */
LFP264:
	mov	al,lbcYlLim
	jmp	LFP46
LFP265:


;		dcl = clLim - cl;
;		if (!fNoYlLimit && dcl > dyllRemain / dylLine)
;			dcl = dyllRemain / dylLine;
	mov	ax,[clLim]
	sub	ax,[clVar]
	cmp	[fNoYlLimit],fFalse
	jnz	LFP31
	xchg	ax,bx
	mov	dx,cx

;#ifndef WIN	 /* Assert causes CS compiler native bug (bl) */
;		Assert(dyllRemain / dylLine == (int)(dyllRemain / dylLine));
;#endif
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	cx,di
	cmp	cx,08000h
	je	LFP30
	or	cx,cx
	jns	LFP27
	neg	cx
LFP27:
	or	dx,dx
	jns	LFP28
	neg	dx
LFP28:
	shl	cx,1
	cmp	cx,dx
	ja	LFP30
LFP29:
	mov	ax,midLayout22
	mov	bx,1031
	cCall	AssertProcForNative,<ax,bx>
LFP30:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

	idiv	di
	cmp	bx,ax
	jg	LFP31
	xchg	ax,bx
LFP31:
	mov	[dcl],ax

;		dyllCur = dcl * dylLine;
	imul	di
	mov	[OFF_dyllCur],ax
	mov	[SEG_dyllCur],dx

;		Assert(dyllCur == (int)dyllCur);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	cx,dx
	cwd
	cmp	cx,dx
	je	LFP32
	mov	ax,midLayout22
	mov	bx,1032
	cCall	AssertProcForNative,<ax,bx>
LFP32:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;		if (phe.clMac == cl + dcl)
;			{
	mov	ax,[clVar]
	add	ax,[dcl]
	sub	al,[phe.clMacPhe]
	or	ax,ax
	jne	LFP35

;			/* last line: must carry space after as well;
;			   if text fits and dyaAfter doesn't, keep text */
;			/* two lines for brain dead native generator! */
;			YLL ddyll = YlFromYa(vpapFetch.dyaAfter);
	;LN_YlFromYa performs YlFromYa(ax); ax, bx, cx, dx are altered.
	mov	ax,[vpapFetch.dyaAfterPap]
	call	LN_YlFromYa
	cwd

;			if (dyllRemain - dyllCur < ddyll)
;				ddyll = dyllRemain - dyllCur;
	mov	bx,[OFF_dyllRemain]
	mov	cx,[SEG_dyllRemain]
	sub	bx,[OFF_dyllCur]
	sbb	cx,[SEG_dyllCur]
	cmp	bx,ax
	mov	di,cx
	sbb	di,dx
	jge	LFP34
	xchg	ax,bx
	mov	dx,cx
LFP34:

;			dyllCur += ddyll;
;			plbs->cp = cpLim;
;			plbs->cl = 0;
	add	[OFF_dyllCur],ax
	adc	[SEG_dyllCur],dx
	;LFP74 performs:
	;plbs->cl = bx = 0;
	;dx:ax = plbs->cp = cpLim;
	;plbs is assumed to be in si.
	;ax, bx, dx are altered.
	call	LFP74

;			}
	jmp	short LFP37

;		else
;			{
LFP35:

;			Assert(dcl >= 0);
;			plbs->cl += dcl;
;			lbc = lbcYlLim;
;			}
	mov	ax,[dcl]
ifdef DEBUG
	or	ax,ax
	jge	LFP36
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midLayout22
	mov	bx,1034
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LFP36:
endif ;/* DEBUG */
	add	[si.clLbs],ax
	mov	bptr ([lbc]),lbcYlLim
LFP37:

;		dyllCur += dylBefore;
;		}
	mov	ax,[dylBefore]
	cwd
	add	[OFF_dyllCur],ax
	adc	[SEG_dyllCur],dx
	jmp	short Ltemp018
LFP38:

;/* still a common case: height is known and it fits */
;	else if (fParaStart && phe.dxaCol != 0 && !fLnn && clLim == clMax &&
;		(dyllCur = YlFromYa(phe.dyaHeight)) <= dyllRemain)
;		{
	test	bptr ([fEndBreak+1]),maskFParaStartLocal
	je	LFormatPara
	cmp	[phe.dxaColPhe],0
	je	LFormatPara
	mov	ax,[clLim]
	sub	ax,clMax
	or	ax,[fLnn]
	jne	LFormatPara
	;LN_YlFromYa performs YlFromYa(ax); ax, bx, cx, dx are altered.
	mov	ax,[phe.dyaHeightPhe]
	call	LN_YlFromYa
	cwd
	mov	[OFF_dyllCur],ax
	mov	[SEG_dyllCur],dx
	cmp	[OFF_dyllRemain],ax
	mov	cx,[SEG_dyllRemain]
	sbb	cx,dx
	jl	LFormatPara

;		ClFormatLines(plbs, cp0, 0, 0, 0, lr.dxl, fFalse, fFalse);
;		vfls.phe = phe;
	;LFP79 performs:
	;ClFormatLines(plbs, cp0, 0, 0, 0, lr.dxl, fFalse, fFalse);
	;vfls.phe = phe;
	;ax, bx, cx, dx are altered.
	call	LFP79

;		plbs->cp = cpLim;
;		plbs->cl = 0;
	;LFP74 performs:
	;plbs->cl = bx = 0;
	;dx:ax = plbs->cp = cpLim;
	;plbs is assumed to be in si.
	;ax, bx, dx are altered.
	call	LFP74

;		dcl = 1;	/* treat it as indivisible */
	inc	bx
	mov	[dcl],bx

;		}
Ltemp018:
	jmp	LHaveDylCur

;/* we don't know para's height, or lines are different heights and cl != 0;
;   we have to format the lines */
;	else
;		{
;		/* we only measure the amount of TEXT -- if dyaAfter doesn't
;		   fit but the text does, that's OK */
;LFormatPara:
;		if (dylFill == ylLarge)
;			dylT = dylMax = ylLarge;
LFormatPara:
	mov	ax,ylLarge
	mov	cx,ax
	cmp	[dylFill],ax
	je	LFP41

;		else
;			{
;			dylT = (lr.lrk == lrkAbsHit) ? 0 : YlFromYa(vpapFetch.dyaAfter);
	mov	al,[lr.lrkLr]
	sub	al,lrkAbsHit
	cbw
	je	LFP39
	mov	ax,[vpapFetch.dyaAfterPap]
	;LN_YlFromYa performs YlFromYa(ax); ax, bx, cx, dx are altered.
	call	LN_YlFromYa
LFP39:

;			dylMax = dylT + ((lr.tSoftBottom != tPos) ? dylFill : plbs->ylMaxLr - plbs->ylColumn);
	mov	cx,[si.ylMaxLrLbs]
	sub	cx,[si.ylColumnLbs]
	;LFPa2 compares lr.tSoftBottom with tPos.
	;only flags are altered.
	call	LFPa2
	je	LFP40
	mov	cx,[dylFill]
LFP40:
	add	cx,ax

;			dylT += dylFill;
;			}
	add	ax,[dylFill]
LFP41:

;		clLim = min(clLim, ClFormatLines(plbs, cpLim, dylT, dylMax, clLim, lr.dxl, fFalse, fTrue));
	push	si
	push	[SEG_cpLim]
	push	[OFF_cpLim]
	push	ax
	push	cx
	push	[clLim]
	push	[lr.dxlLr]
	errnz	<fFalse>
	xor	ax,ax
	push	ax
	errnz	<fTrue - fFalse - 1>
	inc	ax
	push	ax
ifdef DEBUG
	cCall	S_ClFormatLines,<>
else ;!DEBUG
	push	cs
	call	near ptr N_ClFormatLines
endif ;!DEBUG
	cmp	[clLim],ax
	jl	LFP42
	mov	[clLim],ax
LFP42:

;		if (clLim == 0 || (!fParaStart &&
;			(cl = plbs->cl + IInPlc(vfls.hplclnh, plbs->cp)) >= clLim))
;			{
	cmp	[clLim],0
	je	LFP43
	test	bptr ([fEndBreak+1]),maskFParaStartLocal
	jne	LFP48
	;LN_IInPlc performs IInPlc(vfls.hplclnh, *pcp) where pcp is passed
	;in bx.  ax, bx, cx, dx are altered.
	lea	bx,[si.cpLbs]
	call	LN_IInPlc
	add	ax,[si.clLbs]
	mov	[clVar],ax
	cmp	ax,[clLim]
	jl	LFP48
LFP43:

;			/* no lines used */
;			if (!vfli.fParaStopped)
;				{
;				/* not even one line fit */
;				lbc = lbcYlLim;
;				goto LNoChange;
;				}
	mov	al,lbcYlLim
	test	[vfli.fParaStoppedFli],maskFParaStoppedFli
	je	LFP46

;			/* entire para is invisible, simply skip it; we ignore
;			   section boundaries for short docs */
;LEmptyPara:
;			plbs->cl = 0;
;			plbs->cp = vfls.ca.cpLim;
LEmptyPara:
	;LFP73 performs:
	;plbs->cl = bx = 0;
	;dx:ax = plbs->cp = vfls.ca.cpLim
	;plbs is assumed to be in si.
	;ax, bx, dx are altered.
	call	LFP73
;			lbc = (vfls.ca.cpLim >= cpMacDoc) ? lbcEndOfDoc :
;				  (fShort || vfls.ca.cpLim != caSect.cpLim) ?
;				   lbcNil : lbcEndOfSection;
	mov	bl,lbcEndOfDoc
	cmp	ax,[OFF_cpMacDocVar]
	mov	di,dx
	sbb	di,[SEG_cpMacDocVar]
	jge	LFP45
	mov	bl,lbcNil
	errnz	<lbcNil - fFalse>
	cmp	bptr ([fShort]),bl
	jne	LFP45
	cmp	dx,[caSect.HI_cpLimCa]
	jne	LFP45
	cmp	ax,[caSect.LO_cpLimCa]
	jne	LFP45
	mov	bl,lbcEndOfSection
LFP45:
	mov	bptr ([lbc]),bl

;			if (lbc == lbcNil)
;				{
	errnz	<lbcNil>
	or	bl,bl
	jne	LNoChange

;				Debug(Assert(plbs->cp > cpStart ||
;					(plbs->cp == cpStart && plbs->cl > clStart)));
;				goto LExitLbcFormatPara;
;				}
	;Do this assert with a call so as not to affect short jumps.
ifdef DEBUG
	call	LFP90
endif ;/* DEBUG */
	jmp	LExitLbcFormatPara

LFP46:
	mov	bptr [lbc],al

;LNoChange:
;			bltLrp(LrpInPl(plbs->hpllr, plbs->ilrCur), &lr, sizeof(struct LR));
LNoChange:
	;LFP82 performs
	;bltLrp(LrpInPl(plbs->hpllr, plbs->ilrCur), &lr, sizeof(struct LR));
	;ax, bx, cx, dx, di are altered.
	call	LFP82

;			if (lbc == lbcYlLim && lr.lrk == lrkAbsHit)
;				{
	mov	ah,bptr ([lbc])
	mov	al,[lr.lrkLr]
	cmp	ax,(lbcYlLim SHL 8) + lrkAbsHit
	jne	Ltemp019

;				dylOverlapNew = plbs->dylOverlap + max(0, lr.yl + lr.dyl - plbs->yl);
;				}
;			goto LAdjustDyl;
;			}
	mov	ax,[lr.ylLr]
	add	ax,[lr.dylLr]
	sub	ax,[si.ylLbs]
	jge	LFP47
	xor	ax,ax
LFP47:
	add	ax,[si.dylOverlapLbs]
	mov	[dylOverlapNew],ax
Ltemp019:
	jmp	LAdjustDyl
LFP48:

;		dcl = clLim - cl;
	mov	cx,[clLim]
	mov	ax,cx
	sub	ax,[clVar]
	mov	[dcl],ax

;		GetPlc(vfls.hplclnh, clLim - 1, &lnh);
;		dyllCur = lnh.yll;
	;LFP85 performs GetPlc(vfls.hplclnh, cx, bx)
	;dx:ax <== bx.yll;
	;ax, bx, cx, dx are altered.
	dec	cx
	lea	bx,[lnh]
	call	LFP85
	mov	[OFF_dyllCur],ax
	mov	[SEG_dyllCur],dx

;		if (cl > 0)
;			{
;			GetPlc(vfls.hplclnh, cl - 1, &lnhT);
;			dyllCur -= lnhT.yll;
;			}
	mov	cx,[clVar]
	dec	cx
	js	LFP49
	;LFP85 performs GetPlc(vfls.hplclnh, cx, bx)
	;dx:ax <== bx.yll;
	;ax, bx, cx, dx are altered.
	lea	bx,[lnhT]
	call	LFP85
	sub	[OFF_dyllCur],ax
	sbb	[SEG_dyllCur],dx
LFP49:

;		/* give height to splat-only lines only at start of column */
;		if (lnh.fSplatOnly && dcl-- == 1)
;			{
	xor	ax,ax
	errnz	<fFalse>
	test	[lnh.fSplatOnlyLnh],maskfSplatOnlyLnh
	je	LFP495
	dec	[dcl]
	jne	LFP495

;			fLnn = fFalse; /* don't count splats as lines */
	errnz	<fFalse>
	mov	[fLnn],ax

;			if (plbs->yl > plbs->ylColumn)
	mov	bx,[si.ylLbs]
	cmp	bx,[si.ylColumnLbs]
	jle	LFP495

;				dyllCur = 1;
	mov	[SEG_dyllCur],ax
	inc	ax
	mov	[OFF_dyllCur],ax

;			}
LFP495:

;		/* this works even if plclnh is empty: rgcp[iylMac = 0] != cpLim */
;		plbs->cp = CpPlc(vfls.hplclnh, clLim);
;		plbs->cl = 0;
	mov	cx,[clLim]
	;LN_CpPlc performs CpPlc(vfls.hplclnh, cx)
	;ax, bx, cx, dx are altered.
	call	LN_CpPlc
	;LFP76 performs:
	;plbs->cl = bx = 0;
	;plbs->cp = dx:ax;
	;plbs is assumed to be in si.
	;bx is altered.
	call	LFP76

;		if (plbs->cp != vfls.ca.cpLim && !vfls.fEndBreak)
;			{
	mov	cl,[vfls.fEndBreakFls]
	or	cl,cl
	jne	LFP51
	cmp	dx,[vfls.caFls.HI_cpLimCa]
	jne	LFP50
	cmp	ax,[vfls.caFls.LO_cpLimCa]
	je	LFP51

;			/* ran out of space */
;			lbc = lbcYlLim;
LFP50:
	mov	bptr ([lbc]),lbcYlLim

;			}
LFP51:

;		fEndBreak = vfls.fEndBreak;
	mov	bptr ([fEndBreak]),cl

;		}
;LHaveDylCur:
LHaveDylCur:
;	if (dyllCur > ylLarge - plbs->yl - 1)
;		dyllCur = ylLarge - plbs->yl -1;
	mov	ax,ylLarge-1
	sub	ax,[si.ylLbs]
	cwd
	cmp	ax,[OFF_dyllCur]
	sbb	dx,[SEG_dyllCur]
	jge	LFP52
	cwd
	mov	[OFF_dyllCur],ax
	mov	[SEG_dyllCur],dx
	
LFP52:
;	Assert(dyllCur < ylLarge); /* after substracting height of lines in
;	in previous columns, we should be back in the integer range */
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[OFF_dyllCur]
	mov	dx,[SEG_dyllCur]
	sub	ax,ylLarge
	sbb	dx,0
	jl	LFP53
	mov	ax,midLayout22
	mov	bx,1035
	cCall	AssertProcForNative,<ax,bx>
LFP53:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;	if (!fTableRow)
;		dyllAdvance = dyllCur;
	test	bptr ([fEndBreak+1]),maskFTableRowLocal
	jne	LFP54
	mov	ax,[OFF_dyllCur]
	mov	dx,[SEG_dyllCur]
	mov	[OFF_dyllAdvance],ax
	mov	[SEG_dyllAdvance],dx
LFP54:

;/* now we know amount of text from para that will be used in this lr */
;	Scribble(ispLayout3, (((int)IMacPllr(plbs) / 10) % 10) + '0');
;	Scribble(ispLayout4, ((int)IMacPllr(plbs) % 10) + '0');
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	[vdbs.grpfScribbleDbs],0
	jz	LFP55
	mov	bx,[si.hpllrLbs]
	mov	bx,[bx]
	mov	ax,[bx.iMacPl]
	mov	bl,10
	div	bl
	mov	dl,ah
	push	dx
	xor	ah,ah
	div	bl
	xor	ah,ah
	mov	cx,ispLayout3
	cCall	ScribbleProc,<cx,ax>
	pop	ax
	xor	ah,ah
	mov	cx,ispLayout4
	cCall	ScribbleProc,<cx,ax>
LFP55:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

;	lrp = LrpInPl(plbs->hpllr, plbs->ilrCur);
	;LN_LrpInPlCur performs LrpInPl(plbs->hpllr, plbs->ilrCur) with
	;the result in es:bx or es:ax.	It assumes plbs is passed in si.
	;ax, bx, cx, dx are altered.
	call	LN_LrpInPlCur

;	if (lr.fSpaceBefore && lrp->cp == cpNil)
;		{
	test	[lr.fSpaceBeforeLr],maskFSpaceBeforeLr
	je	LFP555
	errnz	<LO_cpNil - 0FFFFh>
	errnz	<HI_cpNil - 0FFFFh>
	mov	ax,es:[bx.LO_cpLr]
	and	ax,es:[bx.HI_cpLr]
	inc	ax
	jne	LFP555

;		/* don't let space before at top of page change the LR */
;		lr.yl += dylBefore;
	mov	ax,[dylBefore]
	add	[lr.ylLr],ax

;		dyllCur -= dylBefore;
	cwd
	sub	[OFF_dyllCur],ax
	sbb	[SEG_dyllCur],dx

;		}
LFP555:

;	if (lr.lrk == lrkAbs || lr.ihdt != ihdtNil)
;		lr.dyl += dyllCur;
	cmp	[lr.lrkLr],lrkAbs
	je	LFP56
	cmp	[lr.ihdtLr],ihdtNil
	je	LFP57
LFP56:
	mov	ax,[OFF_dyllCur]
	add	[lr.dylLr],ax
	jmp	short LFP58

;	else if (lr.dyl <= plbs->yl + (int)dyllAdvance - lr.yl)
;		{
;		/* if tSoftBottom, we can extend past end of lr */
;		lr.dyl = plbs->yl + (int)dyllAdvance - lr.yl;
;		if (dyllAdvance > dyllRemain)
;			lbc = lbcYlLim; /* non-apo footnotes don't set ylLim */
;		}
LFP57:
	mov	ax,[si.ylLbs]
	add	ax,[OFF_dyllAdvance]
	sub	ax,[lr.ylLr]
	cmp	[lr.dylLr],ax
	jg	LFP58
	mov	[lr.dylLr],ax
	mov	bx,[OFF_dyllRemain]
	sub	bx,[OFF_dyllAdvance]
	mov	bx,[SEG_dyllRemain]
	sbb	bx,[SEG_dyllAdvance]
	jge	LFP58
	mov	bptr ([lbc]),lbcYlLim
LFP58:

;	if (lr.lrk != lrkAbs)
;		{
	mov	bx,[dcl]
	cmp	[lr.lrkLr],lrkAbs
	je	LFP59

;		lr.cpLim = plbs->cp;
;		lr.clLim = plbs->cl;
	mov	ax,[si.LO_cpLbs]
	mov	[lr.LO_cpLimLr],ax
	mov	ax,[si.HI_cpLbs]
	mov	[lr.HI_cpLimLr],ax
	mov	ax,[si.clLbs]
	mov	[lr.clLimLr],ax

;		plbs->yl += dyllAdvance;
	mov	cx,[OFF_dyllAdvance]
	mov	dx,[SEG_dyllAdvance]
	add	[si.ylLbs],cx

;		if (plbs->fSimpleLr)
;			{
	cmp	[si.fSimpleLrLbs],fFalse
	je	LFP585

;			lr.lrs = lrsFrozen;
	mov	[lr.lrsLr],lrsFrozen

;			/* will be off slightly for unusual borders because
;			   we don't come back and adjust lr.dyl */
;			lr.dyl = plbs->yl - lr.yl;
;			}
	mov	ax,[si.ylLbs]
	sub	ax,[lr.ylLr]
	mov	[lr.dylLr],ax
LFP585:

;		plbs->dylUsedCol += dyllAdvance;	/* amount of linear space used */
	add	[si.dylUsedColLbs],cx

;		if (dcl > 0)
;			{
	or	bx,bx
	jle	LFP59

;			plbs->dyllTotal += dyllAdvance;
;			plbs->clTotal += dcl;
;			}
	add	[si.LO_dyllTotalLbs],cx
	adc	[si.HI_dyllTotalLbs],dx
	add	[si.clTotalLbs],bx

;		}
LFP59:

;	bltLrp(&lr, lrp, sizeof(struct LR));
;	if (fLnn)
;		plbs->lnn += dcl;
	cmp	[fLnn],fFalse
	je	LFP60
	add	[si.lnnLbs],bx
LFP60:
	;LFP83 performs
	;bltLrp(&lr, LrpInPl(plbs->hpllr, plbs->ilrCur), sizeof(struct LR));
	;ax, bx, cx, dx, di are altered.
	call	LFP83

;	plbs->fFirstPara = fFalse;	/* at least one done */
	mov	[si.fFirstParaLbs],fFalse

;	plbs->fPrevTable = fTableRow;	/* remember if we're in a table */
	mov	al,bptr ([fEndBreak+1])
	and	al,maskFTableRowLocal
	mov	[si.fPrevTableLbs],al

;/* determine why we stopped */
;LSetLbc:
;	if (plbs->cp >= cpMacDoc)
;		lbc = lbcEndOfDoc;
LSetLbc:
	mov	ax,[si.LO_cpLbs]
	mov	dx,[si.HI_cpLbs]
	sub	ax,[OFF_cpMacDocVar]
	sbb	dx,[SEG_cpMacDocVar]
	mov	al,lbcEndOfDoc
	jge	Ltemp020

;	else if (fEndBreak)
;		{
	cmp	bptr ([fEndBreak]),fFalse
	je	LFP62
;		/* terminated by chSect/chColumnBreak;
;		   note: short docs can't get in here */
;LPageBreak:	Assert(!fShort);
LPageBreak:
ifdef DEBUG
	;Do this assert with a call so as not to mess up short jumps.
	call	LFP86
endif ;DEBUG

;		if (vfls.ca.cpLim == caSect.cpLim && plbs->cp == vfls.ca.cpLim)
;			lbc = lbcEndOfSection;
	mov	ax,[vfls.caFls.LO_cpLimCa]
	cmp	ax,[caSect.LO_cpLimCa]
	jne	LFP61
	cmp	ax,[si.LO_cpLbs]
	jne	LFP61
	mov	ax,[vfls.caFls.HI_cpLimCa]
	cmp	ax,[caSect.HI_cpLimCa]
	jne	LFP61
	cmp	ax,[si.HI_cpLbs]
	mov	al,lbcEndOfSection
	je	Ltemp020
LFP61:

;		else if (CColM1Sep(vsepFetch) == 0 || vfls.chBreak == chSect)
;			lbc = lbcEndOfPage;
;#define CColM1Sep(sep) 	 (sep.ccolM1)
	mov	al,lbcEndOfPage
	mov	cx,[vsepFetch.ccolM1Sep]
	jcxz	Ltemp020
	cmp	[vfls.chBreakFls],chSect
	je	Ltemp020

;		else
;			{
;			Assert(vfls.chBreak == chColumnBreak);
ifdef DEBUG
	;Do this assert with a call so as not to mess up short jumps.
	call	LFP88
endif ;DEBUG

;			plbs->fNoBalance = fTrue;
	mov	[si.fNoBalanceLbs],fTrue

;			lbc = lbcEndOfColumn;
	mov	al,lbcEndOfColumn
Ltemp020:
	jmp	short LFP64

;			}
;		}
LFP62:

;	else if (!fChain && !fShort && lr.lrk != lrkAbs && plbs->cp == caSect.cpLim - ccpSect &&
;		plbs->cp < CpMacDocEditPlbs(plbs))
;		{
;#define CpMacDocEditPlbs(plbs)  CpMacDocEdit(plbs->doc)
	mov	al,bptr ([fEndBreak+1])
	and	al,maskFChainLocal
	or	al,bptr ([fShort])
	jne	LAdjustDyl
	cmp	[lr.lrkLr],lrkAbs
	je	LAdjustDyl
	cCall	CpMacDocEdit,<[si.docLbs]>
	mov	bx,[si.LO_cpLbs]
	mov	cx,[si.HI_cpLbs]
	cmp	bx,ax
	mov	ax,cx
	sbb	ax,dx
	jge	LAdjustDyl
	mov	ax,[caSect.LO_cpLimCa]
	mov	dx,[caSect.HI_cpLimCa]
	sub	ax,bx
	sbb	dx,cx
	sub	ax,ccpSect
	or	ax,dx
	jne	LAdjustDyl

;		/* special check: last line of section exactly filled space;
;			especially useful when balancing */
;		if (FInTableDocCp(plbs->doc, plbs->cp))
;			{
;			caParaL.doc = docNil;	/* make sure caParaL gets redone */
;			goto LAdjustDyl;
;			}
	push	[si.docLbs]
	push	cx
	push	bx
ifdef DEBUG
	cCall	S_FInTableDocCp,<>
else ;not DEBUG
	cCall	N_FInTableDocCp,<>
endif ;DEBUG
	xchg	ax,cx
	jcxz	LFP625
	errnz	<hNil>
	xor	ax,ax
	mov	[caParaL.docCa],ax
	jmp	short LAdjustDyl
LFP625:

;		/* special check: last line of section exactly filled space;
;		   especially useful when balancing */
;		lbc = ((plbs->cp = lr.cpLim = caSect.cpLim) >= cpMacDoc) ?
;			lbcEndOfDoc : lbcEndOfSection;
;		/* update cpLim for dcpDepend */
;		vfls.ca.cpLim = CpMax(vfls.ca.cpLim, caSect.cpLim);
;		plbs->cl = lr.clLim = 0;
	mov	ax,[caSect.LO_cpLimCa]
	mov	dx,[caSect.HI_cpLimCa]
	;LFP76 performs:
	;plbs->cl = bx = 0;
	;plbs->cp = dx:ax;
	;plbs is assumed to be in si.
	;bx is altered.
	call	LFP76
	mov	[lr.LO_cpLimLr],ax
	mov	[lr.HI_cpLimLr],dx
	mov	[lr.clLimLr],bx
	cmp	ax,[vfls.caFls.LO_cpLimCa]
	mov	cx,dx
	sbb	cx,[vfls.caFls.HI_cpLimCa]
	jl	LFP63
	mov	[vfls.caFls.LO_cpLimCa],ax
	mov	[vfls.caFls.HI_cpLimCa],dx
LFP63:
	sub	ax,[OFF_cpMacDocVar]
	sbb	dx,[SEG_cpMacDocVar]
	mov	al,lbcEndOfSection
	jl	LFP64
	mov	al,lbcEndOfDoc

;		}
LFP64:
	mov	bptr ([lbc]),al

;LAdjustDyl:
;	if (!fChain && lbc != lbcNil)
;		{
LAdjustDyl:
	mov	al,bptr ([lbc])
	mov	ah,[lr.lrkLr]
	test	bptr ([fEndBreak+1]),maskFChainLocal
	jne	Ltemp022
	cmp	al,lbcNil
	je	Ltemp022

;		/* keep advances past this lr; others need to shorten lr and
;		   try to use remaining space */
;		if (lr.cp != cpNil && lr.ihdt == ihdtNil)
	errnz	<LO_cpNil - 0FFFFh>
	errnz	<HI_cpNil - 0FFFFh>
	mov	cx,[lr.LO_cpLr]
	and	cx,[lr.HI_cpLr]
	inc	cx
	je	LFP648
	cmp	[lr.ihdtLr],ihdtNil
	jne	LFP648

;			if (lbc == lbcYlLim)
;				{
;				if (fTableRow || !FKeepPap(vpapFetch))
;					lr.dyl = plbs->yl - lr.yl;
;				}
;			else if (lr.lrk != lrkAbs)
;				lr.dyl = plbs->yl - lr.yl;
;#define FKeepPap(pap)		 (pap.fKeep)
	cmp	al,lbcYlLim
	jne	LFP642
	cmp	[vpapFetch.fKeepPap],fFalse
	je	LFP646
	test	bptr ([fEndBreak+1]),maskFTableRowLocal
	jmp	short LFP644
Ltemp022:
	jmp	LFP67
LFP642:
	cmp	ah,lrkAbs
LFP644:
	je	LFP648
LFP646:
	mov	cx,[si.ylLbs]
	sub	cx,[lr.ylLr]
	mov	[lr.dylLr],cx
LFP648:

;		/* can't maintain width for rectangles altered by APO */
;		/* need raw cp's for accurate page breaks on screen */
;		/* keeping cl for footnotes raises too many problems */
;		/* Mac: because yl is int, allowing large cl can overflow
;		   ClFormatLines' clFirst scan; in any case, having a big
;		   cl is NOT an optimization */
;		if (lr.clLim != 0 && (lr.lrk != lrkNormal ||
;			vlm == lmBRepag || fShort || plbs->cl >= clMaxLbs))
;			{
	mov	di,[lr.clLimLr]
	or	di,di
	je	LFP663
	errnz	<lrkNormal - 0>
	or	ah,ah
	jne	LFP65
	cmp	[vlm],lmBRepag
	je	LFP65
	cmp	bptr ([fShort]),ah
	jne	LFP65
	cmp	[si.clLbs],clMaxLbs
	jl	LFP663
LFP65:

;			/* reduce cl; this is a smart version of CpFromCpCl */
;			cl = lr.clLim + max(0, IInPlcCheck(vfls.hplclnh, plbs->cp));
	cCall	IInPlcCheck,<[vfls.hplclnhFls], [si.HI_cpLbs], [si.LO_cpLbs]>
	or	ax,ax
	jl	LFP655
	add	di,ax
LFP655:

;			if (IMacPlc(vfls.hplclnh) < cl)
;				ClFormatLines(plbs, vfls.ca.cpLim, ylLarge, ylLarge, cl, lr.dxl, fFalse, fTrue);
	mov	bx,[vfls.hplclnhFls]
	mov	bx,[bx]
	cmp	[bx.iMacPl],di
	jge	LFP66
	mov	ax,ylLarge
	push	si
	push	[vfls.caFls.HI_cpLimCa]
	push	[vfls.caFls.LO_cpLimCa]
	push	ax
	push	ax
	push	di
	push	[lr.dxlLr]
	errnz	<fFalse>
	xor	ax,ax
	push	ax
	errnz	<fTrue - fFalse - 1>
	inc	ax
	push	ax
ifdef DEBUG
	cCall	S_ClFormatLines,<>
else ;!DEBUG
	push	cs
	call	near ptr N_ClFormatLines
endif ;!DEBUG
LFP66:

;			plbs->cp = lr.cpLim = CpPlc(vfls.hplclnh, cl);
;			plbs->cl = lr.clLim = 0;
	;LN_CpPlc performs CpPlc(vfls.hplclnh, cx)
	;ax, bx, cx, dx are altered.
	mov	cx,di
	call	LN_CpPlc
	;LFP76 performs:
	;plbs->cl = bx = 0;
	;plbs->cp = dx:ax;
	;plbs is assumed to be in si.
	;bx is altered.
	call	LFP76
	mov	[lr.clLimLr],bx
	mov	[lr.LO_cpLimLr],ax
	mov	[lr.HI_cpLimLr],dx

;			}
LFP663:

;		if (lr.lrk == lrkAbs && plbs->cp < vfls.ca.cpLim)
;			lr.cpLim = plbs->cp;
	cmp	[lr.lrkLr],lrkAbs
	jne	LFP667
	mov	ax,[si.LO_cpLbs]
	mov	dx,[si.HI_cpLbs]
	cmp	ax,[vfls.caFls.LO_cpLimCa]
	mov	cx,dx
	sbb	cx,[vfls.caFls.HI_cpLimCa]
	jge	LFP667
	mov	[lr.LO_cpLimLr],ax
	mov	[lr.HI_cpLimLr],dx
LFP667:

;		lr.lrs = lrsFrozen;
	mov	[lr.lrsLr],lrsFrozen

;		/* check height/width, but balancing causes restricted room
;		   cases all the time */
;		Assert(vcBalance > 0 || lr.cp == cpNil || (lr.dxl >= 0 && lr.dyl >= 0));
ifdef DEBUG
	;Do this assert with a call so as not to mess up short jumps.
	call	LFPa3
endif ;DEBUG

;		bltLrp(&lr, LrpInPl(plbs->hpllr, plbs->ilrCur), sizeof(struct LR));
	;LFP83 performs
	;bltLrp(&lr, LrpInPl(plbs->hpllr, plbs->ilrCur), sizeof(struct LR));
	;ax, bx, cx, dx, di are altered.
	call	LFP83

;		}
LFP67:

;	/* notice that dcpDepend ignores cl */
;	plbs->dcpDepend = max(0,(int) CpMin(0x7fffL, vfls.ca.cpLim - plbs->cp));
	mov	ax,[vfls.caFls.LO_cpLimCa]
	mov	dx,[vfls.caFls.HI_cpLimCa]
	sub	ax,[si.LO_cpLbs]
	sbb	dx,[si.HI_cpLbs]
	jl	LFP675
	cmp	ax,07FFFh
	sbb	dx,0
	jl	LFP68
	mov	ax,07FFFh
	db	03dh	;turns next "xor ax,ax" into "cmp ax,immediate"
LFP675:
	xor	ax,ax
LFP68:
	mov	[si.dcpDependLbs],ax

;	plbs->dylOverlap = dylOverlapNew;
	mov	ax,[dylOverlapNew]
	mov	[si.dylOverlapLbs],ax

;LExitLbcFormatPara:
;	/* I certainly hope we accomplished something! */
;	/* We may not have made any progress if the first line we tried
;	   would not fit. (lbc == lbcYlLim) */
LExitLbcFormatPara:

;	Debug(Assert(lbc == lbcYlLim || lbc == lbcEndOfDoc ||
;		(plbs->cp > cpStart ||
;		(plbs->cp == cpStart && plbs->cl > clStart))));
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	bptr ([lbc]),lbcYlLim
	je	LFP70
	cmp	bptr ([lbc]),lbcEndOfDoc
	je	LFP70
	mov	ax,[OFF_cpStart]
	mov	dx,[SEG_cpStart]
	sub	ax,[si.LO_cpLbs]
	sbb	dx,[si.HI_cpLbs]
	jl	LFP70
	or	ax,dx
	jne	LFP69
	mov	ax,[clStart]
	cmp	[si.clLbs],ax
	jle	LFP70
LFP69:
	mov	ax,midLayout22
	mov	bx,1036
	cCall	AssertProcForNative,<ax,bx>
LFP70:
	pop	dx
	pop	cx
	pop	bx
	pop	ax

; ShowLbs(plbs, irtnFormPara);
	cmp	[vdbs.fShowLbsDbs],0
	jz	LFP71
	push	plbs
	mov	ax,irtnFormPara
	push	ax
	cCall	ShowLbsProc,<>
LFP71:

endif ;/* DEBUG */

;	StopProfile();
;	return(lbc);
	mov	al,bptr ([lbc])
	cbw

;}
cEnd


	;LFP73 performs:
	;plbs->cl = bx = 0;
	;dx:ax = plbs->cp = vfls.ca.cpLim
	;plbs is assumed to be in si.
	;ax, bx, dx are altered.
LFP73:
	mov	bx,dataoffset [vfls.caFls.cpLimCa]
	jmp	short LFP75
	;LFP74 performs:
	;plbs->cl = bx = 0;
	;dx:ax = plbs->cp = cpLim;
	;plbs is assumed to be in si.
	;ax, bx, dx are altered.
LFP74:
	lea	bx,[cpLim]
LFP75:
	mov	ax,[bx]
	mov	dx,[bx+2]
	;LFP76 performs:
	;plbs->cl = bx = 0;
	;plbs->cp = dx:ax;
	;plbs is assumed to be in si.
	;bx is altered.
LFP76:
	mov	[si.LO_cpLbs],ax
	mov	[si.HI_cpLbs],dx
	xor	bx,bx
	mov	[si.clLbs],bx
	ret


	;LFP77 performs plbs->cp == caParaL.cpFirst.
	;plbs is assumed in si.  Only ax is altered.
LFP77:
	mov	ax,[caParaL.HI_cpFirstCa]
	cmp	ax,[si.HI_cpLbs]
	jne	LFP78
	mov	ax,[caParaL.LO_cpFirstCa]
	cmp	ax,[si.LO_cpLbs]
LFP78:
	ret

	;LFP79 performs:
	;ClFormatLines(plbs, cp0, 0, 0, 0, lr.dxl, fFalse, fFalse);
	;vfls.phe = phe;
	;ax, bx, cx, dx are altered.
LFP79:
	xor	ax,ax
	push	si
	push	ax
	push	ax
	push	ax
	push	ax
	push	ax
	push	[lr.dxlLr]
	errnz	<fFalse>
	push	ax
	push	ax
ifdef DEBUG
	cCall	S_ClFormatLines,<>
else ;!DEBUG
	push	cs
	call	near ptr N_ClFormatLines
endif ;!DEBUG
	db	0A8h	;turns next "stc" into "test al,immediate"
			;also clears the carry flag
	;LFP80 performs phe = vfls.phe.  No registers are altered.
LFP80:
	stc
	push	si	;save plbs
	push	di	;save caller's di
	push	ds
	pop	es
	mov	si,dataoffset [vfls.pheFls]
	lea	di,[phe]
	jc	LFP81
	xchg	si,di
LFP81:
	errnz	<cbPheMin - 6>
	movsw
	movsw
	movsw
	pop	di	;restore caller's di
	pop	si	;restore plbs
	ret


	;LFP82 performs
	;bltLrp(LrpInPl(plbs->hpllr, plbs->ilrCur), &lr, sizeof(struct LR));
	;ax, bx, cx, dx, di are altered.
LFP82:
	db	0A8h	;turns next "stc" into "test al,immediate"
			;also clears the carry flag
	;LFP83 performs
	;bltLrp(&lr, LrpInPl(plbs->hpllr, plbs->ilrCur), sizeof(struct LR));
	;ax, bx, cx, dx, di are altered.
LFP83:
	stc
	pushf
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	jnc	LFP837
	cmp	[lr.LO_cpLr],LO_cpNil
	jne	LFP833
	cmp	[lr.HI_cpLr],HI_cpNil
	je	LFP837
LFP833:
	cmp	[lr.dxlLr],0
	jl	LFP835
	cmp	[lr.dylLr],0
	jge	LFP837
LFP835:
	mov	ax,midLayout22
	mov	bx,6666
	cCall	AssertProcForNative,<ax,bx>
LFP837:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG
	;LN_LrpInPlCur performs LrpInPl(plbs->hpllr, plbs->ilrCur) with
	;the result in es:bx or es:ax.	It assumes plbs is passed in si.
	;ax, bx, cx, dx are altered.
	call	LN_LrpInPlCur
	popf
	push	si
	xchg	ax,di
	lea	si,[lr]
	jc	LFP84
	push	es
	pop	ds
	push	ss
	pop	es
	xchg	si,di
LFP84:
	errnz	<cbLrMin AND 1>
	mov	cx,cbLrMin SHR 1
	rep	movsw
	push	ss
	pop	ds
	pop	si
	ret


	;LFP85 performs GetPlc(vfls.hplclnh, cx, bx)
	;dx:ax <== bx.yll;
	;ax, bx, cx, dx are altered.
LFP85:
	push	bx	;save plnh
	cCall	GetPlc,<[vfls.hplclnhFls], cx, bx>
	pop	bx	;restore plnh
	mov	ax,[bx.LO_yllLnh]
	mov	dx,[bx.HI_yllLnh]
	ret


;		Assert(!fShort);
ifdef DEBUG
LFP86:
	cmp	bptr ([fShort]),fFalse
	je	LFP87
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midLayout22
	mov	bx,1037
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LFP87:
	ret
endif ;/* DEBUG */

;			Assert(vfls.chBreak == chColumnBreak);
ifdef DEBUG
LFP88:
	cmp	bptr ([vfls.chBreakFls]),chColumnBreak
	je	LFP89
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midLayout22
	mov	bx,1038
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LFP89:
	ret
endif ;DEBUG

;				Debug(Assert(plbs->cp > cpStart ||
;					(plbs->cp == cpStart && plbs->cl > clStart)));
ifdef DEBUG
LFP90:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[OFF_cpStart]
	mov	dx,[SEG_cpStart]
	sub	ax,[si.LO_cpLbs]
	sbb	dx,[si.HI_cpLbs]
	jl	LFP92
	or	ax,dx
	jne	LFP91
	mov	ax,[clStart]
	cmp	[si.clLbs],ax
	jle	LFP92
LFP91:
	mov	ax,midLayout22
	mov	bx,1039
	cCall	AssertProcForNative,<ax,bx>
LFP92:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */


;			Assert(dylFill != ylLarge);	 /* are we lying? */
ifdef DEBUG
LFP93:
	cmp	[dylFill],ylLarge
	jne	LFP94
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midLayout22
	mov	bx,1040
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LFP94:
	ret
endif ;/* DEBUG */


;		Assert(plbs->cl == 0);
ifdef DEBUG
LFP95:
	cmp	[si.clLbs],0
	je	LFP96
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midLayout22
	mov	bx,1041
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LFP96:
	ret
endif ;/* DEBUG */


	; Assert (plbs->ylMaxColumn >= 0);
ifdef DEBUG
LFP97:
	cmp	[si.ylMaxColumnLbs],0
	jge	LFP98
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midLayout22
	mov	bx,1042
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LFP98:
	ret
endif ;/* DEBUG */

	
	;LFPa2 compares lr.tSoftBottom with tPos.
	;only flags are altered.
LFPa2:
	push	ax
	mov	al,[lr.tSoftBottomLr]
	and	al,maskTSoftBottomLr
	errnz	<maskTSoftBottomLr - 0F0h>
	cmp	al,tPos SHL 4
	pop	ax
	ret

;		/* check height/width, but balancing causes restricted room
;		   cases all the time */
;		Assert(vcBalance > 0 || lr.cp == cpNil || (lr.dxl >= 0 && lr.dyl >= 0));
ifdef DEBUG
LFPa3:
	cmp	[vcBalance],0
	jg	LFPa5
	cmp	[lr.LO_cpLr],LO_cpNil
	jne	LFPa35
	cmp	[lr.HI_cpLr],HI_cpNil
	je	LFPa5
LFPa35:
	cmp	[lr.dxlLr],0
	jl	LFPa4
	cmp	[lr.dylLr],0
	jge	LFPa5
LFPa4:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midLayout22
	mov	bx,1043
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LFPa5:
	ret
endif ;/* DEBUG */


sEnd	layout2

        end
