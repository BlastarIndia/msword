        include w2.inc
        include noxport.inc
        include consts.inc
        include structs.inc

createSeg	_LAYOUT2,layout2,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midLayout2n	equ 18		 ; module ID, for native asserts
endif

; EXTERNAL FUNCTIONS

externFP	<CpPlc>
externFP	<FInCa>
externFP	<IInPlcRef>
externFP	<GetPlc>
externFP	<FAbsPap>
externFP	<LinkDocToWw>
externFP	<AbortLayout>
externFP	<N_FormatLineDxa>
externFP	<PutPlcLastProc>
externFP	<N_FInsertInPlc>
externFP	<N_GetCpFirstCpLimDisplayPara>
externFP	<N_CachePara>
externFP	<CacheSectProc>
externFP	<N_PdodMother>
externFP	<FQueryAbortCheckProc>
externFP	<FMsgPresent>
externFP	<PutCpPlc>
externFP	<NMultDiv>
externFP	<CpRefFromCpSub>
externFP	<SetAbsLr>
externFP	<ConstrainToAbs>
externFP	<RcToDrc>
externFP	<CpFromCpCl>
externFP	<HpInPl>
externFP	<PInPl>
externFP	<FInsertInPl>
externFP	<FChngSizePhqlcb>
externFP	<FreeHpl>
externFP	<HplInit2>
externFP	<IInPlcCheck>
externFP	<IInPlc>
externFP	<FGetValidPhe>
externFP	<DeleteFromPl>
externFP	<CpMacDoc>
externFP	<SetFlm>
externFP	<PutIMacPlc>
externFP	<AssignAbsXl>
externFP	<DocMother>
externFP	<N_LbcFormatPara>
externFP	<FResetWwLayout>
externFP	<DocMotherLayout>
externFP	<N_FetchCpPccpVisible>

ifdef DEBUG
externFP	<AssertProcForNative>
externFP	<ScribbleProc>
externFP	<ShowLbsProc>
externFP	<S_FAbortLayout>
externFP	<S_FormatLineDxa>
externFP	<S_CachePara>
externFP	<S_GetCpFirstCpLimDisplayPara>
externFP	<PutPlcLastDebugProc>
externFP	<S_PushLbs>
externFP	<S_PopLbs>
externFP	<S_ClFormatLines>
externFP	<S_CopyLbs>
externFP	<S_FAssignLr>
externFP	<S_ReplaceInPllr>
externFP	<S_LbcFormatPara>
externFP	<S_CopyLrs>
externFP	<S_FInsertInPlc>
externFP	<S_FetchCpPccpVisible>
endif ;DEBUG


sBegin  data

; EXTERNALS
externW     vfls
externW     vfli
externW     caParaL
externW     caPara
externW     vfti
externW     caSect
externW     vfInFormatPage
externW     vlm
externW     vpapFetch
externW     mpdochdod
externD     vcpFetch
externW     vsepFetch
externW     vhplcFrl
externW     vhpllbs
externW     vhpllrSpare
externW     vrghdt
externW     vflm
externW     vpisPrompt
externW	  mpwwhwwd

ifdef DEBUG
externD     vcpFirstLayout
externD     vcpLimLayout
externW     mpsbps
externW     vdbs
externW     vfDypsInScreenUnits
externW     vpri
endif ;DEBUG

sEnd    data

; CODE SEGMENT _LAYOUT2

sBegin	layout2
	assumes cs,layout2
        assumes ds,dgroup
        assumes ss,dgroup


;-------------------------------------------------------------------------
;	ClFormatLines(plbs, cpLim, dylFill, dylMax, clLim, dxl, fRefLine, fStopAtPageBreak)
;-------------------------------------------------------------------------
;/* C l  F o r m a t  L i n e s */
;NATIVE int ClFormatLines(plbs, cpLim, dylFill, dylMax, clLim, dxl, fRefLine, fStopAtPageBreak)
;struct LBS *plbs;
;CP cpLim;
;int dylFill;	 /* space to fill */
;int dylMax;	 /* lines can go past dylFill but not dylMax */
;int clLim;	 /* 0 means cache is made to refer to plbs->cp */
;int dxl;	 /* width in which to format */
;int fRefLine;	 /* true means that line containing cpLim is required */
;int fStopAtPageBreak;	 /* care about page breaks? */
;{
;/* returns the cl of the line after the lines that fully fit into dylFIll
;   with [cpFirst,cpLim) but not more than clLim.
;   Updates at least the relevant elements in plcyl and
;   updates clMac if end of para is reached in the process.
;   Does not modify/advance plbs.
;*/
;	int ilnh, ilnh2, ilnhMac;
;	struct PLC **hplclnh;
;	int dyl, dylFirst;
;	int dxa;
;	int fBreakLast;
;	int dylFixed;
;	struct LNH lnh, lnhPrev;
;	BOOL fNotDylFillMax = (dylFill != ylLarge);
;	BOOL fNotDylMax = (dylMax != ylLarge);
;	YLL dyllFill = fNotDylFillMax ? (YLL)dylFill : yllLarge;
;	YLL dyllMax = fNotDylMax ? (YLL)dylMax : yllLarge;
;	CP cp;

; %%Function:N_ClFormatLines %%Owner:BRADV
cProc	N_ClFormatLines,<PUBLIC,FAR>,<si,di>
	ParmW	<plbs>
	ParmD	<cpLim>
	ParmW	<dylFill>
	ParmW	<dylMax>
	ParmW	<clLim>
	ParmW	<dxl>
	ParmW	<fRefLine>
	ParmW	<fStopAtPageBreak>

	LocalW	ilnh
	LocalW	dylFixed
	LocalW	fBreakLast
	LocalW	dxa
	LocalW	ilnhMac
	LocalW	wMaxFlags
	LocalD	dyllFill
	LocalD	dyllMax
	LocalV	lnh,cbLnhMin
	LocalV	lnhPrev,cbLnhMin
ifdef DEBUG
	LocalW	cCallsIInPlcRef
endif ;DEBUG

cBegin
	xor	cx,cx
	mov	ax,[dylFill]
	lea	bx,[dyllFill]
CFL003:
	cwd
	xchg	ch,cl
	cmp	ax,ylLarge
	jne	CFL007
	inc	cx
	mov	dx,HI_yllLarge
	mov	ax,LO_yllLarge
CFL007:
	mov	[bx],ax
	mov	[bx+2],dx
	xchg	ax,bx
	lea	bx,[dyllMax]
	cmp	ax,bx
	mov	ax,[dylMax]
	jne	CFL003
	;fDylFillMax in ch, fDylMax in cl
	mov	[wMaxFlags],cx
	mov	si,[plbs]

;	StartProfile();
;	Win(Assert(vflm == flmRepaginate || vflm == flmPrint ||
;		 vfDypsInScreenUnits || (vlm == lmPagevw && vpri.hdc == NULL)));
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	[vflm],flmRepaginate
	je	CFL008
	cmp	[vflm],flmPrint
	je	CFL008
	cmp	[vfDypsInScreenUnits],fFalse
	jne	CFL008
	mov	ax,[vlm]
	sub	ax,lmPagevw
	errnz	<NULL>
	or	ax,[vpri.hdcPri]
	je	CFL008
	mov	ax,midLayout2n
	mov	bx,1060
	cCall	AssertProcForNative,<ax,bx>
CFL008:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG


;	hplclnh = vfls.hplclnh;
;	CacheParaL(plbs->doc, plbs->cp, plbs->fOutline);
	;LN_CacheParaLbs takes plbs in si
	;and performs CacheParaL(plbs->doc, plbs->cp, plbs->fOutline).
	;ax, bx, cx, dx are altered.
	call	LN_CacheParaLbs

;	if (!FInCa(plbs->doc, plbs->cp, &vfls.ca) || vfls.dxl != dxl ||
;		plbs->cp < CpPlc(hplclnh, 0))
;		{
	xor	cx,cx
	;LN_CpPlc performs CpPlc(vfls.hplclnh, cx)
	;ax, bx, cx, dx are altered.
	call	LN_CpPlc
	mov	bx,[si.LO_cpLbs]
	mov	cx,[si.HI_cpLbs]
	cmp	bx,ax
	mov	ax,cx
	sbb	ax,dx
	jl	CFL01
	mov	ax,[vfls.dxlFls]
	cmp	ax,[dxl]
	jne	CFL01
	mov	ax,dataoffset [vfls.caFls]
	cCall	FInCa,<[si.docLbs], cx, bx, ax>
	or	ax,ax
	jne	CFL04
CFL01:

;		/* reset fls; no knowledge of para's height or lines */
;LResetFls:
;		vfls.ca = caParaL;
LResetFls:
	push	ds
	pop	es
	push	si  ;save plbs
	mov	si,dataoffset [caParaL]
	mov	di,dataoffset [vfls.caFls]
	mov	cx,cbCaMin/2
	rep	movsw
	pop	si  ;restore plbs

;		vfls.ww = plbs->ww;
	errnz	<(wwFls) - (caFls) - cbCaMin>
	mov	ax,[si.wwLbs]
	stosw

;		SetWords(&vfls.phe, 0, cwPHE);
	errnz	<(pheFls) - (wwFls) - 4>
	xor	ax,ax
	inc	di
	inc	di
	errnz	<cbPheMin - 6>
	stosw
	stosw
	stosw

;		vfls.clMac = clNil;
	errnz	<(clMacFls) - (pheFls) - cbPheMin>
	errnz	<(clNil) - (-1)>
	dec	ax
	stosw

;		vfls.fVolatile = vfls.fPageBreak = vfls.fBreakLast = fFalse;
	inc	ax
	errnz	<(fVolatileFls) - (clMacFls) - 3>
	inc	di
	stosb
	errnz	<(fPageBreakFls) - (fVolatileFls) - 3>
	errnz	<(fBreakLastFls) - (fPageBreakFls) - 1>
	inc	di
	inc	di
	stosw

;		vfls.fOutline = plbs->fOutline;
	errnz	<(fOutlineFls) - (fBreakLastFls) - 2>
	inc	di
	mov	al,[si.fOutlineLbs]
	stosb

;		vfls.dxl = dxl;
	errnz	<(dxlFls) - (fOutlineFls) - 1>
	mov	ax,[dxl]
	stosw

;		if (cpLim != cpMax)
;			vfls.ca.cpLim = CpMax(cpLim, caParaL.cpLim);
	mov	ax,[OFF_cpLim]
	mov	dx,[SEG_cpLim]
	errnz	<LO_cpMax - 0FFFFh>
	errnz	<HI_cpMax - 07FFFh>
	mov	cx,dx
	xor	ch,080h
	and	cx,ax
	inc	cx
	je	CFL03
	cmp	ax,[caParaL.LO_cpLimCa]
	mov	cx,dx
	sbb	cx,[caParaL.HI_cpLimCa]
	jge	CFL02
	mov	ax,[caParaL.LO_cpLimCa]
	mov	dx,[caParaL.HI_cpLimCa]
CFL02:
	mov	[vfls.caFls.LO_cpLimCa],ax
	mov	[vfls.caFls.HI_cpLimCa],dx
CFL03:

;		PutIMacPlc(hplclnh, 0);
	xor	ax,ax
	cCall	PutIMacPlc,<[vfls.hplclnhFls],ax>

;		PutCpPlc(hplclnh, 0, plbs->cp);
	;LN_PutCpPlc performs PutCpPlc(vfls.hplclnh, cx, dx:ax)
	;ax, bx, cx, dx are altered.
	xor	cx,cx
	mov	ax,[si.LO_cpLbs]
	mov	dx,[si.HI_cpLbs]
	call	LN_PutCpPlc

;		}
CFL04:

;	if (clLim == 0)
;		{
;		StopProfile();
;		return 0;  /* just wanted to establish cache */
;		}
	xor	ax,ax
	cmp	[clLim],ax
	jne	Ltemp001
	jmp	CFL21
Ltemp001:

;/* find cpFirst */
;	vfls.fFirstColumn = plbs->fFirstColumn;
	mov	al,[si.fFirstColumnLbs]
	mov	[vfls.fFirstColumnFls],al

;	CacheSectL(vfls.ca.doc, vfls.ca.cpFirst, plbs->fOutline);
	mov	bx,[vfls.caFls.docCa]
	mov	dx,[vfls.caFls.HI_cpFirstCa]
	mov	ax,[vfls.caFls.LO_cpFirstCa]
	mov	cl,[si.fOutlineLbs]
	xor	ch,ch
	;LN_CacheSectL takes doc in bx, cp in dx:ax, fOutline in cx
	;ax, bx, cx, dx are altered.
	call	LN_CacheSectL

;	cpLim = CpMin(cpLim, vfls.ca.cpLim);
	mov	dx,[vfls.caFls.HI_cpLimCa]
	mov	ax,[vfls.caFls.LO_cpLimCa]
	cmp	ax,[OFF_cpLim]
	mov	cx,dx
	sbb	cx,[SEG_cpLim]
	jge	CFL05
	mov	[OFF_cpLim],ax
	mov	[SEG_cpLim],dx
CFL05:

;	dxa = NMultDiv(dxl, dxaInch, vfli.dxuInch);
;	LN_NMultDivL performs NMultDiv(ax, dx, bx)
;	ax, bx, cx, dx are altered.
	mov	ax,dxaInch
	mov	dx,[dxl]
	mov	bx,[vfli.dxuInchFli]
	call	LN_NMultDivL
	mov	[dxa],ax

;	ilnh = IInPlcRef(hplclnh, plbs->cp);
;	if (ilnh < 0)
;		{
;		FillLinesUntil(plbs, plbs->cp, yllLarge, clMax, dxa, fFalse, fFalse);
;		ilnh = IInPlcRef(hplclnh, plbs->cp);
;		Assert(ilnh >= 0);
;		}
ifdef DEBUG
	mov	[cCallsIInPlcRef],0
endif ;DEBUG
	jmp	short CFL057
Ltemp032:
	jmp	LResetFls
CFL053:
ifdef DEBUG
	cmp	[cCallsIInPlcRef],0
	je	CFL055
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midLayout2n
	mov	bx,1046
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
CFL055:
	inc	[cCallsIInPlcRef]
endif ;DEBUG
	push	si
	push	[si.HI_cpLbs]
	push	[si.LO_cpLbs]
	mov	bx,HI_yllLarge
	push	bx
	mov	bx,LO_yllLarge
	push	bx
	mov	ax,clMax
	push	ax
	push	[dxa]
	errnz	<fFalse>
	xor	ax,ax
	push	ax
	push	ax
	call	LN_FillLinesUntil
CFL057:
	cCall	IInPlcRef,<[vfls.hplclnhFls], [si.HI_cpLbs], [si.LO_cpLbs]>
	or	ax,ax
	jl	CFL053

;	if (ilnh >= clMaxLbs)
;		goto LResetFls;
	cmp	ax,clMaxLbs
	jge	Ltemp032

;/* find cl beyond cpFirst */
;	vfli.fLayout = fTrue;	/* splats like page view */
	mov	[vfli.fLayoutFli],fTrue

;	ilnh += plbs->cl;
	add	ax,[si.clLbs]
	mov	[ilnh],ax

;	if (ilnh > IMacPlc(hplclnh))
;		{
;		FillLinesUntil(plbs, cpLim, yllLarge, ilnh, dxa, fFalse, fFalse);
	mov	bx,[vfls.hplclnhFls]
	;***Begin in line IMacPlc
	mov	bx,[bx]
	cmp	ax,[bx.iMacPlcStr]
	;***End in line IMacPlc
	jle	CFL06
	push	si
	push	[SEG_cpLim]
	push	[OFF_cpLim]
	mov	bx,HI_yllLarge
	push	bx
	mov	bx,LO_yllLarge
	push	bx
	push	ax
	push	[dxa]
	errnz	<fFalse>
	xor	ax,ax
	push	ax
	push	ax
	call	LN_FillLinesUntil

;		/* this can happen when CpFromCpCl is called after editing */
;		if (ilnh > (ilnhMac = IMacPlc(hplclnh)))
;			goto LEndCl;
;		}
	mov	bx,[vfls.hplclnhFls]
	;***Begin in line IMacPlc
	mov	bx,[bx]
	mov	ax,[bx.iMacPlcStr]
	;***End in line IMacPlc
	mov	[ilnhMac],ax
	cmp	[ilnh],ax
	jg	Ltemp025
CFL06:

;/* now ilnh indicates start of line to start formatting; adjust dyllFill to be
;   the space remaining - but don't count the lines of the para we won't be
;   using */
;	if (fNotDylFillMax)
;		dyllFill -= plbs->yl - plbs->ylColumn;
;	if (fNotDylMax)
;		dyllMax -= plbs->yl - plbs->ylColumn;
	mov	ax,[si.ylColumnLbs]
	sub	ax,[si.ylLbs]
	cwd
	;CFL31 performs:
	;if (fNotDylFillMax)
	;	 dyllFill += dx:ax;
	;if (fNotDylMax)
	;	 dyllMax += dx:ax;
	;cx and dx are altered.
	call	CFL31

;	if (ilnh > 0)
;		{
	mov	cx,[ilnh]
	dec	cx
	jl	CFL08

;		GetPlc(hplclnh, ilnh - 1, &lnh);
	;CFL22 performs GetPlc(vfls.hplclnh, cx, &lnh)
	;ax, bx, cx, dx are altered.
	call	CFL22

;		if (fNotDylFillMax)
;			dyllFill += lnh.yll;
;		if (fNotDylMax)
;			dyllMax += lnh.yll;
	mov	ax,[lnh.LO_yllLnh]
	mov	dx,[lnh.HI_yllLnh]
	;CFL31 performs:
	;if (fNotDylFillMax)
	;	 dyllFill += dx:ax;
	;if (fNotDylMax)
	;	 dyllMax += dx:ax;
	;cx and dx are altered.
	call	CFL31

;		}
CFL08:

;/* find dyllFill, clLim beyond */
;	InvalFli();
;#define InvalFli()	 vfli.ww = wwNil
	errnz	<wwNil - 0>
	xor	ax,ax
	mov	[vfli.wwFli],ax

;	fBreakLast = vfls.fBreakLast = vfls.fEndBreak = fFalse;
	errnz	<fFalse>
	errnz	<(fEndBreakFls) - (fBreakLastFls) - 1>
	mov	wptr ([vfls.fBreakLastFls]),ax
	mov	bptr ([fBreakLast]),al

;	for (ilnhMac = IMacPlc(hplclnh); ; )
;		{
	mov	bx,[vfls.hplclnhFls]
	;***Begin in line IMacPlc
	mov	bx,[bx]
	mov	ax,[bx.iMacPlcStr]
	;***End in line IMacPlc
	mov	[ilnhMac],ax
CFL09:

;		if (ilnh == ilnhMac)
;			{
	mov	ax,[ilnh]
	cmp	ax,[ilnhMac]
	jne	CFL12

;			if (fBreakLast)
;				{
	cmp	bptr ([fBreakLast]),fFalse
	je	CFL10

;				/* last FillLinesUntil hit page break */
;				vfls.fEndBreak = fTrue;
;				break;
	mov	[vfls.fEndBreakFls],fTrue
Ltemp026:
	jmp	CFL15

;				}
CFL10:

;			FillLinesUntil(plbs, cpLim, dyllMax, clLim, dxa, fStopAtPageBreak, fRefLine);
	push	si
	push	[SEG_cpLim]
	push	[OFF_cpLim]
	push	[SEG_dyllMax]
	push	[OFF_dyllMax]
	push	[clLim]
	push	[dxa]
	push	[fStopAtPageBreak]
	push	[fRefLine]
	call	LN_FillLinesUntil

;			fBreakLast = vfls.fBreakLast;
	mov	al,[vfls.fBreakLastFls]
	mov	bptr ([fBreakLast]),al

;			if (ilnh == (ilnhMac = IMacPlc(hplclnh)))
;				{
	mov	bx,[vfls.hplclnhFls]
	;***Begin in line IMacPlc
	mov	bx,[bx]
	mov	cx,[bx.iMacPlcStr]
	;***End in line IMacPlc
	mov	[ilnhMac],cx
	cmp	[ilnh],cx
	jne	CFL12

;				if (fBreakLast)
;					vfls.fEndBreak = fTrue;
;				break;	/* FillLinesUntil hit a limit */
;				}
	or	al,al
	jmp	short CFL135
Ltemp025:
	jmp	LEndCl

;			}
CFL12:

;		if (!fRefLine && CpPlc(hplclnh, ilnh) >= cpLim)
;			break;
	cmp	[fRefLine],fFalse
	jne	CFL123
	mov	cx,[ilnh]
	;LN_CpPlc performs CpPlc(vfls.hplclnh, cx)
	;ax, bx, cx, dx are altered.
	call	LN_CpPlc
	sub	ax,[OFF_cpLim]
	sbb	dx,[SEG_cpLim]
	jge	Ltemp026
CFL123:

;		GetPlc(hplclnh, ilnh, &lnh);
	;CFL22 performs GetPlc(vfls.hplclnh, cx, &lnh)
	;ax, bx, cx, dx are altered.
	mov	cx,[ilnh]
	call	CFL22

;		if (lnh.yll > dyllMax && !lnh.fSplatOnly)
;			break;
	mov	ax,[OFF_dyllMax]
	mov	dx,[SEG_dyllMax]
	sub	ax,[lnh.LO_yllLnh]
	sbb	dx,[lnh.HI_yllLnh]
	jge	CFL125
	test	[lnh.fSplatOnlyLnh],maskfSplatOnlyLnh
	je	CFL15
CFL125:

;		fBreakLast = vfls.fBreakLast = lnh.chBreak;
	mov	al,[lnh.chBreakLnh]
	mov	[vfls.fBreakLastFls],al
	mov	bptr ([fBreakLast]),al

;		cp = CpPlc(hplclnh, ++ilnh);
;		if (cp > cpLim || !fRefLine && cp == cpLim)
;			{
	inc	[ilnh]
	mov	cx,[ilnh]
	;LN_CpPlc performs CpPlc(vfls.hplclnh, cx)
	;ax, bx, cx, dx are altered.
	call	LN_CpPlc
	mov	bx,[OFF_cpLim]
	mov	cx,[SEG_cpLim]
	sub	bx,ax
	sbb	cx,dx
	jl	CFL127
	or	bx,cx
	or	bx,[fRefLine]
	jne	CFL13
CFL127:

;			if (ilnh == ilnhMac && vfls.fBreakLast)
;				vfls.fEndBreak = fTrue;
;			break;
;			}
	mov	ax,[ilnh]
	cmp	ax,[ilnhMac]
	jne	CFL15
	cmp	[vfls.fBreakLastFls],fFalse
	jmp	short CFL135
Ltemp003:
	jmp	CFL20
Ltemp002:
	jmp	CFL09
CFL13:

;		if (fBreakLast && fStopAtPageBreak)
;			{
;			vfls.chBreak = lnh.chBreak;
;			vfls.fEndBreak = fTrue;
;			break;
;			}
	cmp	bptr ([fBreakLast]),fFalse
	je	CFL14
	cmp	[fStopAtPageBreak],fFalse
	je	CFL14
	mov	al,[lnh.chBreakLnh]
	mov	[vfls.chBreakFls],al
CFL135:
	je	CFL15
	mov	[vfls.fEndBreakFls],fTrue
	jmp	short CFL15
CFL14:

;		if (lnh.yll >= dyllFill || ilnh - plbs->cl >= clLim)
;			break;
	mov	ax,[lnh.LO_yllLnh]
	mov	dx,[lnh.HI_yllLnh]
	sub	ax,[OFF_dyllFill]
	sbb	dx,[SEG_dyllFill]
	jge	CFL15
	mov	ax,[ilnh]
	sub	ax,[si.clLbs]
	cmp	ax,[clLim]
	jl	Ltemp002

;		}
;LEndCl:
LEndCl:
CFL15:

;	vfli.fLayout = fFalse;
;	vfli.doc = docNil;
	errnz	<fFalse>
	errnz	<docNil>
	xor	ax,ax
	mov	[vfli.fLayoutFli],al
	mov	[vfli.docFli],ax

;/* ilnh is one past the line with largest dyl <= dyllFill or largest
;   cpLim <= cpLim */
;/* if plc spans whole para, check for lines all same height */

;	if (ilnh > 0 && vfls.clMac == clNil && !vfls.fPageBreak &&
;		CpPlc(hplclnh, ilnhMac) == vfls.ca.cpLim)
;		{
	cmp	[ilnh],0
	jle	Ltemp003
	cmp	[vfls.clMacFls],clNil
	jne	Ltemp003
	cmp	[vfls.fPageBreakFls],fFalse
	jne	Ltemp003
	mov	cx,[ilnhMac]
	;LN_CpPlc performs CpPlc(vfls.hplclnh, cx)
	;ax, bx, cx, dx are altered.
	call	LN_CpPlc
	sub	ax,[vfls.caFls.LO_cpLimCa]
	sbb	dx,[vfls.caFls.HI_cpLimCa]
	or	ax,dx
	jne	Ltemp003

;		vfls.clMac = ilnhMac;
	mov	ax,[ilnhMac]
	mov	[vfls.clMacFls],ax

;		/* vfls.phe.fUnk = vfls.phe.fDiffLines = fFalse; */
;		vfls.phe.w0 = 0;
;		vfls.phe.clMac = -1;
	errnz	<(w0Phe) - 0>
	errnz	<(clMacPhe) - 1>
	mov	[vfls.pheFls.w0Phe],0FF00h

;		vfls.phe.dxaCol = XaFromXl(dxl);
	mov	ax,[dxl]
	;LN_XaFromXl performs XaFromXl(ax); ax, bx, cx, dx are altered.
	call	LN_XaFromXl
	mov	[vfls.pheFls.dxaColPhe],ax

;		if (vfls.fVolatile || ilnh > vfls.phe.clMac)
;			goto LDiffLines;
	cmp	[vfls.fVolatileFls],fFalse
	jne	Ltemp024
	mov	al,[vfls.pheFls.clMacPhe]
	xor	ah,ah
	cmp	[ilnh],ax
	jg	Ltemp024

;		GetPlc(hplclnh, 0, &lnhPrev);
	;CFL23 performs GetPlc(vfls.hplclnh, cx, ax)
	;ax, bx, cx, dx are altered.
	xor	cx,cx
	lea	ax,[lnhPrev]
	call	CFL23

;		Assert(lnhPrev.yll < ylLarge);	/* the height of any single line
ifdef DEBUG
	;Do this assert with a call so as not to mess up short jumps.
	call	CFL25
endif ;DEBUG

;		dylFixed = (int)lnhPrev.yll - 
;			- (lnhPrev.fDypBeforeAdded ? YlFromYa(vpapFetch.dyaBefore) : 0);
	xor	ax, ax
	test	[lnhPrev.fDypBeforeAddedLnh],maskfDypBeforeAddedLnh
	je	CFL155
	mov	ax,[vpapFetch.dyaBeforePap]
	;LN_YlFromYa performs YlFromYa(ax); ax, bx, cx, dx are altered.
	call	LN_YlFromYa
CFL155:
	sub	ax,[lnhPrev.LO_yllLnh]
	neg	ax
	mov	[dylFixed],ax

;		for (ilnh2 = 1; ilnh2 < ilnhMac - 1; ilnh2++, lnhPrev = lnh)
;			{
	mov	cx,1
	mov	di,[ilnhMac]
	dec	di
CFL16:
	cmp	cx,di
	jge	CFL17

;			GetPlc(hplclnh, ilnh2, &lnh);
	;CFL22 performs GetPlc(vfls.hplclnh, cx, &lnh)
	;ax, bx, cx, dx are altered.
	push	cx	;save ilnh2
	call	CFL22
	pop	cx	;restore ilnh2

;			if (lnh.yll - lnhPrev.yll != dylFixed)
;				goto LDiffLines;
	mov	ax,[lnh.LO_yllLnh]
	sub	ax,[lnhPrev.LO_yllLnh]
	sub	ax,[dylFixed]
Ltemp024:
	jne	LDiffLines

;			}
	inc	cx
	mov	ax,[lnh.LO_yllLnh]
	mov	[lnhPrev.LO_yllLnh],ax
ifdef DEBUG
	push	[lnh.HI_yllLnh]
	pop	[lnhPrev.HI_yllLnh]
endif ;DEBUG
	jmp	short CFL16
CFL17:

;		/* ilnh2 is ilnhMac - 1 */
;		dyl = YlFromYa(vpapFetch.dyaAfter);
	mov	ax,[vpapFetch.dyaAfterPap]
	;LN_YlFromYa performs YlFromYa(ax); ax, bx, cx, dx are altered.
	call	LN_YlFromYa

;		if (ilnhMac <= 1)
;			dylFixed -= dyl;
	or	di,di
	jg	CFL175
	sub	[dylFixed],ax
	jmp	short CFL18

;		else
;			{
;			GetPlc(hplclnh, ilnh2, &lnh);
CFL175:
	;CFL22 performs GetPlc(vfls.hplclnh, cx, &lnh)
	;ax, bx, cx, dx are altered.
	mov	cx,di
	xchg	ax,di
	call	CFL22

;			Assert(lnh.yll - lnhPrev.yll < ylLarge);
ifdef DEBUG
	;Do this assert with a call so as not to mess up short jumps.
	call	CFL29
endif ;DEBUG

;			if ((int)(lnh.yll - lnhPrev.yll) - dyl != dylFixed)
;				goto LDiffLines;
	mov	ax,[lnh.LO_yllLnh]
	sub	ax,[lnhPrev.LO_yllLnh]
	sub	ax,di
	cmp	ax,[dylFixed]
	jne	LDiffLines

;			}
CFL18:

;		vfls.phe.clMac = vfls.clMac;
	mov	al,bptr ([vfls.clMacFls])
	mov	[vfls.pheFls.clMacPhe],al

;		vfls.phe.dylLine = YaFromYl(dylFixed);
;#define YaFromYl(yl)		(NMultDiv(yl, czaInch, vfti.dypInch))
;	LN_NMultDivL performs NMultDiv(ax, dx, bx)
;	ax, bx, cx, dx are altered.
	mov	ax,czaInch
	mov	dx,[dylFixed]
	mov	bx,[vfti.dypInchFti]
	call	LN_NMultDivL
	mov	[vfls.pheFls.dylLinePhe],ax

;		}
;	StopProfile();
;	return(ilnh);
	jmp	short CFL20

;LDiffLines:
LDiffLines:

;	GetPlc(hplclnh, ilnhMac - 1, &lnh);
	;CFL22 performs GetPlc(vfls.hplclnh, cx, &lnh)
	;ax, bx, cx, dx are altered.
	mov	cx,[ilnhMac]
	dec	cx
	call	CFL22

;	if (lnh.yll >= ylLarge)
	cmp	[lnh.HI_yllLnh],0
	jl	CFL19
	cmp	[lnh.LO_yllLnh],ylLarge
	jb	CFL19

;		{
;		SetWords(&vfls.phe, 0, cwPHE);
;		}
	errnz	<(pheFls) - (wwFls) - 4>
	push	ds
	pop	es
	xor	ax,ax
	mov	di,dataoffset [vfls.pheFls]
	errnz	<cbPheMin - 6>
	stosw
	stosw
	stosw

;	else
;		{
	jmp	short CFL20
CFL19:

;		vfls.phe.dylHeight = NMultDiv(lnh.yll, czaInch, vfli.dyuInch);
;	LN_NMultDivL performs NMultDiv(ax, dx, bx)
;	ax, bx, cx, dx are altered.
	mov	ax,czaInch
	mov	dx,[lnh.LO_yllLnh]
	mov	bx,[vfli.dyuInchFli]
	call	LN_NMultDivL
	mov	[vfls.pheFls.dylHeightPhe],ax

;		vfls.phe.fDiffLines = fTrue;
;		}
	or	[vfls.pheFls.fDiffLinesPhe],maskFDiffLinesPhe
CFL20:

;	StopProfile();
;	return(ilnh);
	mov	ax,[ilnh]

CFL21:
;}
cEnd


	;CFL22 performs GetPlc(vfls.hplclnh, cx, &lnh)
	;ax, bx, cx, dx are altered.
CFL22:
	lea	ax,[lnh]
	;CFL23 performs GetPlc(vfls.hplclnh, cx, ax)
	;ax, bx, cx, dx are altered.
CFL23:
	cCall	GetPlc,<[vfls.hplclnhFls],cx,ax>
	ret

;		Assert(lnhPrev.yll < ylLarge);	/* the height of any single line
ifdef DEBUG
CFL25:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[lnhPrev.LO_yllLnh]
	mov	dx,[lnhPrev.HI_yllLnh]
	sub	ax,ylLarge
	sbb	dx,0
	jb	CFL26
	mov	ax,midLayout2n
	mov	bx,1002
	cCall	AssertProcForNative,<ax,bx>
CFL26:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG


;			Assert(lnh.yll - lnhPrev.yll < ylLarge);
ifdef DEBUG
CFL29:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[lnh.LO_yllLnh]
	mov	dx,[lnh.HI_yllLnh]
	sub	ax,[lnhPrev.LO_yllLnh]
	sbb	dx,[lnhPrev.HI_yllLnh]
	sub	ax,ylLarge
	sbb	dx,0
	jb	CFL30
	mov	ax,midLayout2n
	mov	bx,1003
	cCall	AssertProcForNative,<ax,bx>
CFL30:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG


	;CFL31 performs:
	;if (fNotDylFillMax)
	;	 dyllFill += dx:ax;
	;if (fNotDylMax)
	;	 dyllMax += dx:ax;
	;cx and dx are altered.
CFL31:
	mov	cx,[wMaxFlags]
	;fDylFillMax in ch, fDylMax in cl
	or	ch,ch
	jne	CFL32
	add	[OFF_dyllFill],ax
	adc	[SEG_dyllFill],dx
CFL32:
	or	cl,cl
	jne	CFL33
	add	[OFF_dyllMax],ax
	adc	[SEG_dyllMax],dx
CFL33:
	ret


;-------------------------------------------------------------------------
;	FillLinesUntil(plbs, cpLim, dyllFill, clLim, dxa, fStopAtPageBreak, fRefLine)
;-------------------------------------------------------------------------
;/* F i l l  L i n e s	U n t i l */
;NATIVE FillLinesUntil(plbs, cpLim, dyllFill, clLim, dxa, fStopAtPageBreak, fRefLine)
;struct LBS *plbs;
;CP cpLim;
;YLL dyllFill;
;int clLim;
;int dxa;
;int fStopAtPageBreak;
;int fRefLine;
;{
;/*	 add lines to plcyl in fls until
;		 clLim lines have been added in total
;		 next line would go over dyllFill
;		 next line would go over cpLim
;		 page break (conditionally)
;*/
;	int chSplat, fSplatOnly, fSeeSplat = fTrue, fAPO = fFalse;
;	struct DOD *pdod;
;	YLL dyll = 0;
;	int ilnhMac;
;	int ww = vfls.ww;
;	CP cp;
;	struct LNH lnh;

Ltemp023:
	jmp	LEndFill

; %%Function:LN_FillLinesUntil %%Owner:BRADV
cProc	LN_FillLinesUntil,<PUBLIC,NEAR>,<si>
	ParmW	<plbs>
	ParmD	<cpLim>
	ParmD	<dyllFill>
	ParmW	<clLim>
	ParmW	<dxa>
	ParmW	<fStopAtPageBreak>
	ParmW	<fRefLine>

	LocalW	ww
	LocalW	chSplat
	LocalW	fAPO
	LocalD	cp
	LocalD	dyll
	LocalV	lnh, cbLnhMin

cBegin

	;Assembler note: it is redundant to initialize fAPO here
	mov	ax,[vfls.wwFls]
	mov	[ww],ax
ifdef DEBUG
	cmp	bptr ([fRefLine + 1]),0
	je	FLU005
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midLayout2n
	mov	bx,1004
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FLU005:
endif ;DEBUG
	xor	ax,ax
	mov	[OFF_dyll],ax
	mov	[SEG_dyll],ax
	;Assembler note: the following line is done below in the C version
;	chSplat = fSplatOnly = fFalse;
	mov	[chSplat],ax	 ;high byte is fSplatOnly

;	StartProfile();
;	Assert(cpLim <= vfls.ca.cpLim);
;	Win(Assert(vflm == flmRepaginate || vflm == flmPrint ||
;		 vfDypsInScreenUnits || (vlm == lmPagevw && vpri.hdc == NULL)));
ifdef DEBUG
	;Do the following asserts with a call so as not to mess up short jumps.
	call	FLU22
endif ;DEBUG

;	cp = CpPlc(vfls.hplclnh, ilnhMac = IMacPlc(vfls.hplclnh));
	mov	bx,[vfls.hplclnhFls]
	;***Begin in line IMacPlc
	mov	bx,[bx]
	mov	si,[bx.iMacPlcStr]
	;***End in line IMacPlc
	mov	cx,si
	;LN_CpPlc performs CpPlc(vfls.hplclnh, cx)
	;ax, bx, cx, dx are altered.
	call	LN_CpPlc
	mov	[OFF_cp],ax
	mov	[SEG_cp],dx

;	if (cp >= vfls.ca.cpLim)
;		goto LEndFill;
	sub	ax,[vfls.caFls.LO_cpLimCa]
	sbb	dx,[vfls.caFls.HI_cpLimCa]
Ltemp004:
	jge	Ltemp023

;	fAPO = FAbsPapM(vfls.ca.doc, &vpapFetch);
	mov	ax,dataoffset [vpapFetch]
	cCall	FAbsPap,<[vfls.caFls.docCa], ax>
	;Ensure the result of FAbsPap exists entirely in the low byte.
ifdef DEBUG
	;Do the following assert with a call so as not to mess up short jumps.
	call	FLU18
endif ;DEBUG

;	pdod = PdodDoc(vfls.ca.doc);	/* two lines for compiler */
	mov	bx,[vfls.caFls.docCa]
	;Takes doc in bx, result in bx.  Only bx is altered.
	call	LN_PdodDocL
	mov	ah,[bx.fShortDod]	;high byte of fAPO is fNoSeeSplat
	mov	[fAPO],ax

;	if (pdod->fShort)
;		{
	or	ah,ah
	je	FLU03

;		if (vflm == flmRepaginate || vflm == flmPrint)
;			{
	mov	ax,[vflm]
	cmp	al,flmRepaginate
	je	FLU022
	cmp	al,flmPrint
	jne	FLU024
FLU022:

;			ww = WinMac(wwLayout, wwTemp);
;			LinkDocToWw(vfls.ca.doc, ww, wwNil);
	mov	ax,wwLayout
	mov	[ww],ax
	errnz	<wwNil>
	xor	bx,bx
	cCall	LinkDocToWw,<[vfls.caFls.docCa], ax, bx>

;			}
FLU024:

	;Assembler note: fNoSeeSplat is set above
;		fSeeSplat = fFalse;
;		}
FLU03:

	;Assembler note: the following line is done above in the .asm version
;	chSplat = fSplatOnly = fFalse;

;	if (ilnhMac)
;		{
	dec	si
	jl	FLU04

;		GetPlc(vfls.hplclnh, ilnhMac - 1, &lnh);
	lea	ax,[lnh]
	cCall	GetPlc,<[vfls.hplclnhFls],si,ax>

;		dyll = lnh.yll;
	mov	ax,[lnh.LO_yllLnh]
	mov	[OFF_dyll],ax
	mov	dx,[lnh.HI_yllLnh]
	mov	[SEG_dyll],dx

;		}
FLU04:
	inc	si	;restore ilnhMac

;	while (ilnhMac < clLim)
;		{
FLU045:
	cmp	si,[clLim]
	jge	Ltemp004


;		if (vfInFormatPage && vlm == lmBRepag && FAbortLayout(vfls.fOutline, plbs))
	cmp	[vfInFormatPage],fFalse
	je	FLU05
	cmp	[vlm],lmBRepag
	jne	FLU05
	mov	al,[vfls.fOutlineFls]
	cbw
	push	ax
	push	[plbs]
ifdef DEBUG
	cCall	S_FAbortLayout,<>
else ;not DEBUG
	push	cs
	call	near ptr N_FAbortLayout
endif ;DEBUG
	or	ax,ax
	je	FLU05

;			AbortLayout();
	cCall	AbortLayout,<>
FLU05:

;		FormatLineDxaL(ww, vfls.ca.doc, cp, dxa);
;#define FormatLineDxaL(ww, doc, cp, dxa)	 FormatLineDxa(ww, doc, cp, dxa)
ifdef DEBUG
	cCall	S_FormatLineDxa,<[ww], [vfls.caFls.docCa], [SEG_cp], [OFF_cp], [dxa]>
else ;not DEBUG
	cCall	N_FormatLineDxa,<[ww], [vfls.caFls.docCa], [SEG_cp], [OFF_cp], [dxa]>
endif ;DEBUG

;		if (vfli.fParaStopped)
;			{
;			/* last line of para is hidden text, limit ourselves
;			   to para */
;			PutCpPlc(vfls.hplclnh, ilnhMac, vfls.ca.cpLim);
;			vfls.fVolatile = fTrue;
;			goto LEndFill;
;			}
	test	[vfli.fParaStoppedFli],maskFParaStoppedFli
	je	FLU055
	;LN_PutCpPlc performs PutCpPlc(vfls.hplclnh, cx, dx:ax)
	;ax, bx, cx, dx are altered.
	mov	cx,si
	mov	ax,[vfls.caFls.LO_cpLimCa]
	mov	dx,[vfls.caFls.HI_cpLimCa]
	call	LN_PutCpPlc
	mov	[vfls.fVolatileFls],fTrue
	jmp	LEndFill
FLU055:

;		cp = vfli.cpMac;
	mov	ax,[vfli.LO_cpMacFli]
	mov	[OFF_cp],ax
	mov	ax,[vfli.HI_cpMacFli]
	mov	[SEG_cp],ax

;		chSplat = fSplatOnly = fFalse;
;		if (vfli.fSplatColumn)
;			chSplat = chColumnBreak;
;		else if (vfli.fSplatBreak)
;			chSplat = chSect;
;
;		if (chSplat)
;			{
	errnz	<(fSplatBreakFli) - (fSplatColumnFli)>
	mov	al,[vfli.fSplatBreakFli]
	;Assembler note: clear ah because by default fSplatOnly is fFalse
	and	ax,maskFSplatBreakFli+maskFSplatColumnFli
	je	FLU06
	test	al,maskFSplatColumnFli
	mov	al,chColumnBreak
	jne	FLU06
	xor	al,chSect XOR chColumnBreak	;xor to clear zero flag
FLU06:
	je	FLU08
	;Assembler note: storing chSplat and fSplatOnly into memory
	;is postponed until just after FLU08 or FLU075.

;			if (fSeeSplat)
;				{
	cmp	bptr ([fAPO+1]),fFalse	;fNoSeeSplat is high byte of fAPO
	jne	FLU075

;				fSplatOnly = vfli.ichMac == 1;
;				if (fAPO && !(fSplatOnly && vfli.cpMin == vfls.ca.cpFirst))
;					fSplatOnly = chSplat = fFalse;
;				else
;					vfls.chBreak = chSplat;
;				}
;			else
;				chSplat = fFalse; /* ignore splat in hdr, apo, etc. */
	cmp	[vfli.ichMacFli],1
	jne	FLU063
	mov	ah,maskfSplatOnlyLnh			;fSplatOnly is high byte of chSplat
FLU063:
	cmp	bptr ([fAPO]),fFalse
	je	FLU067
	or	ah,ah
	je	FLU07
	mov	cx,[vfli.HI_cpMinFli]
	sub	cx,[vfls.caFls.HI_cpFirstCa]
	jne	FLU07
	mov	cx,[vfli.LO_cpMinFli]
	sub	cx,[vfls.caFls.LO_cpFirstCa]
	jne	FLU07
FLU067:
	mov	[vfls.chBreakFls],al
	db	03Dh	;turns next "xor ax,ax" into "cmp ax,immediate"
FLU07:
	xor	ax,ax
	db	03Dh	;turns next "xor al,al" into "cmp ax,immediate"
FLU075:
	xor	al,al
	mov	[chSplat],ax		;high byte is fSplatOnly
	jmp	short FLU10

;			}
FLU08:

;		else if (vfli.chBreak == chSect || vfli.chBreak == chColumnBreak)
;			{
	mov	[chSplat],ax	 ;high byte is fSplatOnly
	mov	al,bptr ([vfli.chBreakFli])
	cmp	al,chSect
	je	FLU09
	cmp	al,chColumnBreak
	jne	FLU10
FLU09:

;			/* we have to be sensitive to splats that are
;			   pending in order to detect end of section
;			   properly */
;			chSplat = vfli.chBreak;
	mov	bptr ([chSplat]),al

;			vfls.chBreak = vfli.chBreak;
	mov	[vfls.chBreakFls],al

;			cp += ccpSect;
	add	[OFF_cp],ccpSect
	adc	[SEG_cp],0

;			}
FLU10:

;		dyll += vfli.dypLine;
	mov	ax,[vfli.dypLineFli]
	cwd
	add	[OFF_dyll],ax
	adc	[SEG_dyll],dx

;		if (!fSplatOnly)
;			{
	test	bptr ([chSplat+1]),maskfSplatOnlyLnh
	jne	FLU103

;			if (dyll > dyllFill || (!chSplat && !fRefLine && cp > cpLim))
;				goto LEndFill;
	mov	ax,[OFF_dyllFill]
	mov	dx,[SEG_dyllFill]
	sub	ax,[OFF_dyll]
	sbb	dx,[SEG_dyll]
Ltemp005:
	jl	Ltemp031
	mov	al,bptr ([fRefLine])
;	Assert(!(fRefLine && 0xFF00h));
ifdef DEBUG
	;Do this assert with a call so as not to mess up short jumps
	call	FLU16
endif ;DEBUG
	or	al,bptr ([chSplat])
	jne	FLU103
	mov	ax,[OFF_cpLim]
	mov	dx,[SEG_cpLim]
	sub	ax,[OFF_cp]
	sbb	dx,[SEG_cp]
	jl	Ltemp005

;			}
FLU103:

;		vfls.fBreakLast = fStopAtPageBreak && chSplat;
;		vfls.fPageBreak |= chSplat;
;		vfls.fVolatile |= vfli.fVolatile;
	mov	al,[vfli.fVolatileFli]
	or	[vfls.fVolatileFls],al
	mov	al,bptr ([chSplat])
	or	[vfls.fPageBreakFls],al
	cmp	[fStopAtPageBreak],fFalse
	jne	FLU107
	mov	al,fFalse
FLU107:
	mov	[vfls.fBreakLastFls],al

;		if (fSplatOnly && ilnhMac > 0)
;			{
FLU11:
	test	bptr ([chSplat+1]),maskfSplatOnlyLnh
	je	FLU12
	mov	cx,si
	dec	cx
	jl	FLU12

;			GetPlc(vfls.hplclnh, ilnhMac - 1, &lnh);
	lea	ax,[lnh]
	cCall	GetPlc,<[vfls.hplclnhFls],cx,ax>

;			if (!lnh.chBreak)
;				{
	cmp	[lnh.chBreakLnh],fFalse
	jne	FLU12

;				lnh.chBreak = chSplat;
	mov	al,bptr ([chSplat])
	mov	[lnh.chBreakLnh],al

;				PutPlcLast(vfls.hplclnh, ilnhMac - 1, &lnh);
ifdef DEBUG
	mov	ax,si
	dec	ax
	lea	bx,[lnh]
	cCall	PutPlcLastDebugProc,<[vfls.hplclnhFls],ax,bx>
else ;not DEBUG
	cCall	PutPlcLastProc,<[vfls.hplclnhFls]>
endif ;DEBUG

;				dyll -= vfli.dypLine;
	mov	ax,[vfli.dypLineFli]
	cwd
	sub	[OFF_dyll],ax
	sbb	[SEG_dyll],dx

;				goto LSetCp;
	jmp	short LSetCp
Ltemp031:
	jmp	LEndFill

;				}
;			}
FLU12:

;		lnh.fSplatOnly = fSplatOnly;
;		lnh.chBreak = chSplat;
;		lnh.fDypBeforeAdded = vfli.dypBefore != 0;
;		lnh.yll = dyll;
	errnz	<(chBreakLnh) - (fSplatOnlyLnh) - 1>
	mov	ax,[chSplat]	    ;high byte is fSplatOnly
	xchg	ah,al
	cmp	[vfli.dypBeforeFli],0
	je	FLU13
	or	al,maskfDypBeforeAddedLnh
FLU13:
	mov	wptr ([lnh.fSplatOnlyLnh]),ax
	mov	ax,[OFF_dyll]
	mov	dx,[SEG_dyll]
	mov	[lnh.LO_yllLnh],ax
	mov	[lnh.HI_yllLnh],dx

;		if (!FInsertInPlc(vfls.hplclnh, ilnhMac++, vfli.cpMin, &lnh))
;			{
	lea	ax,[lnh]
	push	[vfls.hplclnhFls]
	push	si
	push	[vfli.HI_cpMinFli]
	push	[vfli.LO_cpMinFli]
	push	ax
	inc	si
ifdef DEBUG
	cCall	S_FInsertInPlc,<>
else ;not DEBUG
	cCall	N_FInsertInPlc,<>
endif ;DEBUG
	or	ax,ax
	jne	FLU14

;			if (vfInFormatPage)
;				AbortLayout();
;			goto LEndFill;
;			}
	cmp	[vfInFormatPage],fFalse
	je	LEndFill
	cCall	AbortLayout,<>
	;Assembler note: should never reach here

FLU14:

;LSetCp:
LSetCp:
;		PutCpPlc(vfls.hplclnh, ilnhMac, cp);
;		vfls.ca.cpLim = CpMax(vfls.ca.cpLim, cp);
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	cmp	ax,[vfls.caFls.LO_cpLimCa]
	mov	cx,dx
	sbb	cx,[vfls.caFls.HI_cpLimCa]
	jl	FLU15
	mov	[vfls.caFls.LO_cpLimCa],ax
	mov	[vfls.caFls.HI_cpLimCa],dx
FLU15:

	;LN_PutCpPlc performs PutCpPlc(vfls.hplclnh, cx, dx:ax)
	;ax, bx, cx, dx are altered.
	mov	cx,si
	call	LN_PutCpPlc

;		if (dyll >= dyllFill || cp > cpLim || vfls.fBreakLast ||
;			!fRefLine && cp == cpLim)
;			{
;			goto LEndFill;
;			}
;		}
	mov	ax,[OFF_dyll]
	mov	dx,[SEG_dyll]
	sub	ax,[OFF_dyllFill]
	sbb	dx,[SEG_dyllFill]
	jge	LEndFill
	cmp	[vfls.fBreakLastFls],fFalse
	jne	LEndFill
	mov	ax,[OFF_cpLim]
	mov	dx,[SEG_cpLim]
	sub	ax,[OFF_cp]
	sbb	dx,[SEG_cp]
	jl	LEndFill
	or	ax,dx
	or	ax,[fRefLine]
	je	LEndFill
	jmp	FLU045

;LEndFill:
LEndFill:
;	StopProfile();
;}
cEnd

;	Assert(!(fRefLine && 0xFF00h));
ifdef DEBUG
	;Do this assert with a call so as not to mess up short jumps
FLU16:
	cmp	bptr ([fRefLine+1]),fFalse
	je	FLU17
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midLayout2n
	mov	bx,1054
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FLU17:
	ret
endif ;DEBUG

	;Ensure the result of FAbsPap exists entirely in the low byte.
ifdef DEBUG
FLU18:
	or	ah,ah
	je	FLU19
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midLayout2n
	mov	bx,1055
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FLU19:
	ret
endif ;DEBUG

ifdef DEBUG
;	Assert(cpLim <= vfls.ca.cpLim);
;	Win(Assert(vflm == flmRepaginate || vflm == flmPrint ||
;		 vfDypsInScreenUnits || (vlm == lmPagevw && vpri.hdc == NULL)));
FLU22:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[vfls.caFls.LO_cpLimCa]
	mov	dx,[vfls.caFls.HI_cpLimCa]
	sub	ax,[OFF_cpLim]
	sbb	dx,[SEG_cpLim]
	jge	FLU23
	mov	ax,midLayout2n
	mov	bx,1005
	cCall	AssertProcForNative,<ax,bx>
FLU23:
	cmp	[vflm],flmRepaginate
	je	FLU24
	cmp	[vflm],flmPrint
	je	FLU24
	cmp	[vfDypsInScreenUnits],fFalse
	jne	FLU24
	mov	ax,[vlm]
	sub	ax,lmPagevw
	errnz	<NULL>
	or	ax,[vpri.hdcPri]
	je	FLU24
	mov	ax,midLayout2n
	mov	bx,1061
	cCall	AssertProcForNative,<ax,bx>
FLU24:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG



maskFAbsLocal equ 001h

;-------------------------------------------------------------------------
;	FAssignLr(plbs, dylFill, fEmptyOK)
;-------------------------------------------------------------------------
;/* F	A s s i g n   L r */
;NATIVE int FAssignLr(plbs, dylFill, fEmptyOK)
;struct LBS *plbs;
;int dylFill, fEmptyOK;
;{
;/* create or assign a layout rectangle to the upcoming paragraphs;
;   returns fFalse if height is already known */
;	int ilr, ilrMac, ilrT;
;	int doc = plbs->doc;
;	int docMother;
;	int fFtn;
;	int fShort;
;	struct DOD *pdod;
;	int fAbs, fPend, fPrevTable;
;	int xlRightAbs, xlLeftAbs;
;	int xaLeft, xaRight;
;	int dylOverlapNew;
;	LRP lrp;
;	LRP lrpT;
;	struct PAP pap;
;	struct RC rcl, rclPend;
;	struct LR lr;
;	struct LBS lbsT;
;	int ihdt;

; %%Function:N_FAssignLr %%Owner:BRADV
cProc	N_FAssignLr,<PUBLIC,FAR>,<si,di>
	ParmW	<plbs>
	ParmW	<dylFill>
	ParmW	<fEmptyOK>

	LocalW	fFtn
	LocalW	ilrMac
	LocalW	docMotherVar
	LocalW	dylOverlapNew
	LocalW	fPrevTable
	LocalV	lr,cbLrMin
	OFFBP_lr    = -6+cbLrMin
	LocalW	fPend
	OFFBP_fPend = -8+cbLrMin
	LocalV	rclVar,cbRcMin
	LocalV	rclPend,cbRcMin

cBegin

;/* quick check - see if we can reuse previous lr */
;	StartProfile();
;	CacheParaL(doc, plbs->cp, plbs->fOutline);
	;LN_CacheParaLbs takes plbs in si
	;and performs CacheParaL(plbs->doc, plbs->cp, plbs->fOutline).
	;ax, bx, cx, dx are altered.
	mov	si,[plbs]
	call	LN_CacheParaLbs

;	pdod = PdodDoc(doc);
;	fFtn = pdod->fFtn;
;	fShort = pdod->fShort;
	mov	bx,[si.docLbs]
	;Takes doc in bx, result in bx.  Only bx is altered.
	call	LN_PdodDocL
	;Assembler note: since fShort includes fFtn, we will copy the byte
	;pdod->fShort into fFtn, and test with maskFFtnDod when we want
	;fFtn, and compare with 0 when we want fShort.
	errnz	<(fShortDod) - (fFtnDod)>
	mov	al,[bx.fFtnDod]
	xor	ah,ah
	mov	[fFtn],ax

;	fAbs = !fFtn && FAbsPapM(doc, &vpapFetch);
;	fPrevTable = fFalse;
	errnz	<fFalse>
	mov	bptr ([fPrevTable]),ah
	test	al,maskFFtnDod
	jne	FALr01
	mov	ax,dataoffset [vpapFetch]
	cCall	FAbsPap,<[si.docLbs], ax>
	xchg	ax,cx
	jcxz	FALr01
	;Assembler note: fAbs is the high byte of fFtn
	errnz	<maskFAbsLocal - 001h>
	inc	bptr ([fFtn+1])
FALr01:

;	if (!fAbs && plbs->ilrCur >= 0 &&
;		plbs->ilrCur < IMacPllr(plbs))
;		{
	test	bptr ([fFtn+1]),maskFAbsLocal
	jne	FALr02
	mov	cx,[si.ilrCurLbs]
	or	cx,cx
	jl	FALr02
	;***Begin in-line IMacPllr
	mov	bx,[si.hpllrLbs]
	mov	bx,[bx]
	cmp	cx,[bx.iMacPl]
	;***End in-line IMacPllr
	jge	FALr02

;		lrp = LrpInPl(plbs->hpllr, plbs->ilrCur);
	;LN_LrpInPl performs LrpInPl(plbs->hpllr, cx) with the result in es:bx
	;or es:ax.  It assumes plbs is passed in si.
	;ax, bx, dx are altered.
	call	LN_LrpInPl

;		if (lrp->doc == doc && lrp->ihdt == plbs->ihdt &&
;			lrp->xl == plbs->xl)
;			{
;			if (lrp->lrs != lrsFrozen)
;				{
;				if (!plbs->fAbsPresent)
;					{
;					plbs->dylOverlap = 0;
;					StopProfile();
;					return(fFalse);
;					}
;				}
	;FALr30 returns (lrp->doc == plbs->doc && lrp->ihdt == plbs->ihdt)
	;as zero flag set if true, zero flag reset if false.
	;lrp is assumed in es:bx, plbs is assumed in si.
	;Only ax is altered.
	call	FALr30
	jne	FALr02
	mov	ax,es:[bx.xlLr]
	cmp	ax,[si.xlLbs]
	jne	FALr02
	cmp	es:[bx.lrsLr],lrsFrozen
	je	FALr015
	cmp	[si.fAbsPresentLbs],fFalse
	jne	FALr02
	jmp	FALr07
FALr015:

;			else if (plbs->fSimpleLr && plbs->fPrevTable && vpapFetch.fInTable
;					&& plbs->yl == lrp->yl + lrp->dyl)
;				{
	errnz	<fFalse>
	xor	ax,ax
	cmp	[si.fSimpleLrLbs],al
	je	FALr02
	cmp	[si.fPrevTableLbs],al
	je	FALr02
	cmp	[vpapFetch.fInTablePap],al
	je	FALr02
	;LN_LrpInPlCur performs LrpInPl(plbs->hpllr, plbs->ilrCur) with
	;the result in es:bx or es:ax.	It assumes plbs is passed in si.
	;ax, bx, cx, dx are altered.
	call	LN_LrpInPlCur
	mov	cx,es:[bx.ylLr]
	add	cx,es:[bx.dylLr]
	cmp	[si.ylLbs],cx
	jne	FALr02

;				plbs->yl -= plbs->dylBelowTable;
;				lrp->dyl -= plbs->dylBelowTable;
;				plbs->dylBelowTable = 0;
;				fPrevTable = fTrue;
;				}
;			}
;		}
	xor	ax,ax
	xchg	ax,[si.dylBelowTableLbs]
	sub	[si.ylLbs],ax
	sub	es:[bx.dylLr],ax
	mov	bptr ([fPrevTable]),fTrue
FALr02:

;	/* get sect properties */
;	if (fFtn)
;		{
;		docMother = DocMother(doc);
;		CacheSectL(docMother, CpRefFromCpSub(doc, plbs->cp, edcDrpFtn), plbs->fOutline);
;		}
;	else
;		CacheSectL(doc, plbs->cp, plbs->fOutline);
;#define edcDrpFtn	 (offset(DOD,drpFtn)/sizeof(int))
	mov	bx,[si.docLbs]
	mov	ax,[si.LO_cpLbs]
	mov	dx,[si.HI_cpLbs]
	mov	cl,[si.fOutlineLbs]
	xor	ch,ch
	test	bptr ([fFtn]),maskFFtnDod
	je	FALr03
	push	cx	;save plbs->fOutline
	mov	cx,([drpFtnDod]) SHR 1
	push	bx	    ;argument for CpRefFromCpSub
	push	dx	    ;argument for CpRefFromCpSub
	push	ax	    ;argument for CpRefFromCpSub
	push	cx	    ;argument for CpRefFromCpSub
	cCall	DocMother,<bx>
	mov	[docMotherVar],ax
	xchg	ax,di
	cCall	CpRefFromCpSub,<>
	mov	bx,di
	pop	cx	;restore plbs->fOutline
FALr03:
	;LN_CacheSectL takes doc in bx, cp in dx:ax, fOutline in cx
	;ax, bx, cx, dx are altered.
	call	LN_CacheSectL

;/* find an existing LR to which we can assign the new text */
;	if ((ilrMac = IMacPllr(plbs)) > 0)
;		{
	;LN_IMacPllr performs cx = IMacPllr(plbs).  plbs is assumed to be
	;passed in si.	Only bx and cx are altered.
	call	LN_IMacPllr
	mov	[ilrMac],cx
	jcxz	Ltemp007

;		for (lrp = LrpInPl(plbs->hpllr, ilr = 0); ilr < ilrMac; ilr++, lrp++)
;			{
	mov	cx,-1
FALr04:
	inc	cx
ifdef DEBUG
	;Assembler note: If we call LrpInPl with cx >= ilrMac we may assert
	;in HpInPl because we are trying to get a pointer beyond the end
	;of the PL.  This is a debug only problem, however, because right
	;after FALr06 we exit the loop when cx >= ilrMac, and never use
	;the pointer.  This debug code is designed to work around the
	;problem.  The problem doesn't exist in the C versions because
	;there we work with HUGE pointers, and since those are not altered
	;by code movement, there is no need to call HpInPl to restore them,
	;in the assembler version we are using faster far pointers, but
	;this requires dereferencing whenever code movement may have taken
	;place, hence this call to HpInPl (inside LN_LrpInPl).
	cmp	cx,[ilrMac]
	jge	FALr06
endif ;DEBUG
	;LN_LrpInPl performs LrpInPl(plbs->hpllr, cx) with the result in es:bx
	;or es:ax.  It assumes plbs is passed in si.
	;ax, bx, dx are altered.
	call	LN_LrpInPl
	jmp	short FALr06
FALr05:
	inc	cx
	add	bx,cbLrMin
FALr06:
	cmp	cx,[ilrMac]
	jge	Ltemp007

;			/* doc and fAbs must match; ihdt match means both from same header or
;			   both not headers */
;			if (lrp->doc != doc || lrp->ihdt != plbs->ihdt || lrp->lrs == lrsFrozen)
;				continue;
	;FALr30 returns (lrp->doc == plbs->doc && lrp->ihdt == plbs->ihdt)
	;as zero flag set if true, zero flag reset if false.
	;lrp is assumed in es:bx, plbs is assumed in si.
	;Only ax is altered.
	call	FALr30
	jne	FALr05
	cmp	es:[bx.lrsLr],lrsFrozen
	je	FALr05

;			if (fAbs ^ lrp->lrk == lrkAbs)
;				continue;
	mov	al,bptr ([fFtn+1])
	errnz	<maskFAbsLocal - 001h>
	xor	al,es:[bx.lrkLr]
	cmp	al,lrkAbs
	je	FALr05

;			/* if both abs, cp's must match */
;			if (fAbs)
;				{
	test	bptr ([fFtn+1]),maskFAbsLocal
	je	FALr08

;				if (plbs->cp != lrp->cp)
;					continue;
	mov	ax,[si.LO_cpLbs]
	cmp	ax,es:[bx.LO_cpLr]
	jne	FALr05
	mov	ax,[si.HI_cpLbs]
	cmp	ax,es:[bx.HI_cpLr]
	jne	FALr05

;				/* reuse abs block */
;				plbs->ilrCur = ilr;
;				plbs->fPrevTable = fFalse;
	mov	[si.ilrCurLbs],cx
	mov	[si.fPrevTableLbs],fFalse

;				bltLrp(lrp, &lr, sizeof(struct LR));
;				AssignAbsXl(plbs, &lr);
	push	cx	;save ilr
	push	si	;save plbs
	lea	di,[lr]
	push	si	;argument for AssignAbsXl
	push	di	;argument for AssignAbsXl
	push	es
	pop	ds
	push	ss
	pop	es
	mov	si,bx
	errnz	<cbLrMin AND 1>
	mov	cx,cbLrMin SHR 1
	rep	movsw
	push	ss
	pop	ds
	cCall	AssignAbsXl,<>
	pop	si	;restore plbs
	pop	cx	;restore ilr

;				bltLrp(&lr, lrp, sizeof(struct LR));
	;LN_LrpInPl performs LrpInPl(plbs->hpllr, cx) with the result in es:bx
	;or es:ax.  It assumes plbs is passed in si.
	;ax, bx, dx are altered.
	call	LN_LrpInPl
	xchg	ax,di
	lea	si,[lr]
	errnz	<cbLrMin AND 1>
	mov	cx,cbLrMin SHR 1
	rep	movsw
FALr07:

;				plbs->dylOverlap = 0;
	xor	ax,ax	    ;value to store in plbs->dylOverlap
			    ;also clears carry so we will return fFalse

;				StopProfile();
;				return(fFalse);
;				}
	jmp	FALr29
Ltemp007:
	jmp	short FALr11
Ltemp022:
	jmp	short FALr04
FALr08:

;			/* check for different columns */
;			if (lrp->lrk != lrkAbsHit)
;				{
;				if (plbs->xl != lrp->xl)
;					continue;
;				}
;			else if (plbs->xl > lrp->xl)
;				/* apos can only force the xl to be bigger */
;				continue;
	mov	ax,es:[bx.xlLr]
	cmp	es:[bx.lrkLr],lrkAbsHit
	je	FALr09
	cmp	[si.xlLbs],ax
Ltemp027:
	jne	FALr05
FALr09:
	cmp	[si.xlLbs],ax
	jg	Ltemp027

;			/* finally check for same section */
;			if (fFtn)
;				{
;				if (FInCa(docMother, CpRefFromCpSub(lrp->doc, lrp->cp, edcDrpFtn), &caSect))
;					break;
;				}
;			else if (FInCa(lrp->doc, lrp->cp, &caSect))
;				break;
;			}
;#define edcDrpFtn	 (offset(DOD,drpFtn)/sizeof(int))
	push	cx	;save ilr
	push	es:[bx.docLr]
	push	es:[bx.HI_cpLr]
	push	es:[bx.LO_cpLr]
	test	bptr ([fFtn]),maskFFtnDod
	je	FALr10
	mov	cx,([drpFtnDod]) SHR 1
	push	cx
	cCall	CpRefFromCpSub,<>
	push	[docMotherVar]
	push	dx
	push	ax
FALr10:
	lea	ax,[caSect]
	push	ax
	cCall	FInCa,<>
	pop	cx	;restore ilr
	or	ax,ax
	je	Ltemp022

;		if (ilr < ilrMac)
;			{
;			plbs->ilrCur = ilr;
;			plbs->dylOverlap = 0;
;			StopProfile();
;			return(fTrue);
;			}
;		}
	;Assembler note: at this point we know ilr < ilrMac.  Assert this
	;with a call so as not to mess up short jumps.
ifdef DEBUG
	call	FALr32
endif ;DEBUG
	mov	[si.ilrCurLbs],cx
	xor	ax,ax	    ;value to store in plbs->dylOverlap
	jmp	FALr285

FALr11:

;/* need new LR */
;	dylOverlapNew = 0;
;	fPend = fFalse;
;	plbs->fPrevTable = fPrevTable;
;	plbs->ilrCur = ilrMac;
;	plbs->ylMaxLr = plbs->ylMaxBlock;
;	SetWords(&lr, 0, cwLR);
	mov	ax,[ilrMac]
	mov	[si.ilrCurLbs],ax
	mov	ax,[si.ylMaxBlockLbs]
	mov	[si.ylMaxLrLbs],ax
	mov	al,bptr ([fPrevTable])
	mov	[si.fPrevTableLbs],al
	xor	ax,ax
	mov	[dylOverlapNew],ax
	errnz	<OFFBP_lr - OFFBP_fPend - 2>
	push	ds
	pop	es
	lea	di,[fPend]
	errnz	<cbLrMin AND 1>
	mov	cx,(cbLrMin SHR 1) + 1
	rep	stosw

;	lr.doc = doc;
	mov	ax,[si.docLbs]
	mov	[lr.docLr],ax

;	lr.yl = plbs->yl;	/* for abs, these are first approximations */
	mov	cx,[si.ylLbs]
	mov	[lr.ylLr],cx

;	lr.xl = plbs->xl;
	mov	ax,[si.xlLbs]
	mov	[lr.xlLr],ax

;	lr.dxl = plbs->dxlColumn;
	mov	ax,[si.dxlColumnLbs]
	mov	[lr.dxlLr],ax

;	lr.ihdt = plbs->ihdt;
	mov	bx,[si.ihdtLbs]
	mov	[lr.ihdtLr],bx

;	if (lr.ihdt == ihdtNil)
;		lr.dyl = plbs->ylColumn + dylFill - lr.yl;
;	else
;		lr.cpMacCounted = cpNil;
	mov	ax,0FFFFh
	errnz	<ihdtNil - (-1)>
	inc	bx
	jne	FALr12
	mov	bx,[si.ylColumnLbs]
	add	bx,[dylFill]
	sub	bx,cx
	mov	[lr.dylLr],bx
	jmp	short FALr13
FALr12:
	errnz	<LO_cpNil - 0FFFFh>
	errnz	<HI_cpNil - 0FFFFh>
	mov	[lr.LO_cpMacCountedLr],ax
	mov	[lr.HI_cpMacCountedLr],ax
FALr13:

;	lr.lnn = (fAbs || fShort || !FLnnSep(vsepFetch)) ? lnnNil : plbs->lnn;
;#define FLnnSep(sep)		 (sep.nLnnMod)
	errnz	<lnnNil - (-1)>
	mov	cx,ax
	;Assembler note: fShort is the byte at fFtn, fAbs is in the byte
	;at fFtn+1.
	test	[fFtn],ax
	jne	FALr135
	test	[vsepFetch.nLnnModSep],ax
	je	FALr135
	mov	cx,[si.lnnLbs]
FALr135:
	mov	[lr.lnnLr],cx

;	lr.ilrNextChain = ilrNil;
	errnz	<ilrNil - (-1)>
	mov	[lr.ilrNextChainLr],ax

;	lr.lrk = (fAbs) ? lrkAbs : lrkNormal;
	errnz	<lrkNormal - 0>
	errnz	<lrkAbs - 1>
	errnz	<maskFAbsLocal - 001h>
	mov	bl,bptr ([fFtn+1])
	mov	[lr.lrkLr],bl

;	lr.tSoftBottom = tNeg;	    /* ninch for exceed bottom */
	errnz	<tNeg - (-1)>
	or	[lr.tSoftBottomLr],maskTSoftBottomLr

;/* absolute object */
;	if (fAbs)
;		/* for abs. positioning, set cpFirst and cpLim now */
;		SetAbsLr(plbs, &lr);
	or	bl,bl
	je	FALr14
	lea	ax,[lr]
	cCall	SetAbsLr,<si, ax>
Ltemp008:
	jmp	LReplace

;/* normal object, may be affected by absolutes */
;	else
;		{
FALr14:

;		plbs->ylMaxLr = plbs->ylMaxBlock;
;		plbs->dylChain = ylLarge;
	mov	bx,[si.ylMaxBlockLbs]
	mov	[si.ylMaxLrLbs],bx
	mov	[si.dylChainLbs],ylLarge

;		lr.cp = lr.cpLim = cpNil;
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,es
	mov	bx,ds
	cmp	ax,bx
	je	FALr15
	mov	ax,midLayout2n
	mov	bx,1006
	cCall	AssertProcForNative,<ax,bx>
FALr15:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG
	errnz	<(cpLimLr) - (cpLr) - 4>
	lea	di,[lr.cpLr]
	errnz	<LO_cpNil - 0FFFFh>
	errnz	<HI_cpNil - 0FFFFh>
	stosw
	stosw
	stosw
	stosw

;		if (plbs->fAbsPresent && lr.ihdt == ihdtNil)
;			{
;			/* restrict lr so it will miss abs objects */
;			rcl.ylTop = lr.yl;
;LFitLr:
;			rcl.xlLeft = lr.xl;
;			rcl.xlRight = lr.xl + lr.dxl;
;			rcl.ylBottom = plbs->ylColumn + dylFill;
	test	[si.fAbsPresentLbs],al
	je	Ltemp008
	errnz	<ihdtNil - (-1)>
	cmp	[lr.ihdtLr],ax
	jne	Ltemp008
	mov	ax,[lr.ylLr]
LFitLr:
	mov	[rclVar.ylTopRc],ax
	mov	ax,[lr.xlLr]
	mov	[rclVar.xlLeftRc],ax
	add	ax,[lr.dxlLr]
	mov	[rclVar.xlRightRc],ax
	mov	ax,[si.ylColumnLbs]
	add	ax,[dylFill]
	mov	[rclVar.ylBottomRc],ax

;LHaveRcl:
;			ConstrainToAbs(plbs, dylFill, &rcl, &rclPend, &fPend, &lr);
LHaveRcl:
	push	si
	push	[dylFill]
	lea	ax,[rclVar]
	push	ax
	lea	ax,[rclPend]
	push	ax
	lea	ax,[fPend]
	push	ax
	lea	ax,[lr]
	push	ax
	cCall	ConstrainToAbs,<>

;			if (rcl.ylBottom - plbs->yl <= plbs->dylOverlap ||
;				rcl.ylBottom <= rcl.ylTop)
;				{
	mov	ax,[rclVar.ylBottomRc]
	cmp	ax,[rclVar.ylTopRc]
	jle	FALr153
	sub	ax,[si.ylLbs]
	cmp	ax,[si.dylOverlapLbs]
	jg	FALr18
FALr153:

;				/* rectangle is too short */
;				if (fPend)
;					{
;					/* ignore short one, use pending one */
	cmp	[fPend],fFalse
	je	FALr16

;					fPend = fFalse;
;					rcl = rclPend;
;					lr.fConstrainLeft = fTrue;
;					lr.fConstrainRight = fFalse;
;					goto LHaveRcl;
;					}
FALr155:
	mov	[fPend],fFalse
	push	si	;save plbs
	push	ds
	pop	es
	lea	si,[rclPend]
	lea	di,[rclVar]
	errnz	<cbRcMin - 8>
	movsw
	movsw
	movsw
	movsw
	pop	si	;restore plbs
	errnz	<(fConstrainLeftLr) - (fConstrainRightLr)>
	and	[lr.fConstrainLeftLr],NOT (maskfConstrainLeftLr + maskfConstrainRightLr)
	or	[lr.fConstrainLeftLr],maskfConstrainLeftLr
	jmp	short LHaveRcl
FALr16:

;				if (plbs->ilrFirstChain != ilrNil)
;					goto LGiveUp;
	cmp	[si.ilrFirstChainLbs],ilrNil
	jne	LGiveUp

;				if (rcl.ylBottom >= plbs->ylColumn + dylFill)
;					{
	mov	ax,[si.ylColumnLbs]
	add	ax,[dylFill]
	cmp	[rclVar.ylBottomRc],ax
	jl	FALr17

;					/* avoid inf loop when no room by not flowing */
;					if (!fEmptyOK)
;						goto LTakeRcl;
	cmp	[fEmptyOK],fFalse
	je	LTakeRcl

;					/* maybe only the last one is too short */
;LGiveUp:
;					plbs->ilrCur = ilrNil;
LGiveUp:
	mov	[si.ilrCurLbs],ilrNil

;					if (rcl.ylBottom - rcl.ylTop > 0)
;						plbs->dylChain = min(plbs->dylChain, rcl.ylBottom - rcl.ylTop);
;					goto LMendChain;
	mov	ax,[rclVar.ylBottomRc]
	sub	ax,[rclVar.ylTopRc]
	jle	LMendChain
	cmp	[si.dylChainLbs],ax
	jle	LMendChain
	mov	[si.dylChainLbs],ax
	jmp	short LMendChain

;					}
FALr17:
;				/* retry with advanced yl */
;				lr.lrk = lrkAbsHit;
;				rcl.ylTop = rcl.ylBottom;
;				lr.fConstrainLeft = lr.fConstrainRight = fFalse;
;				dylOverlapNew = rcl.ylTop - plbs->yl;
;				goto LFitLr;
;				}
	mov	[lr.lrkLr],lrkAbsHit
	mov	ax,[rclVar.ylBottomRc]
	mov	bx,ax
	sub	bx,[si.ylLbs]
	mov	[dylOverlapNew],bx
	errnz	<(fConstrainLeftLr) - (fConstrainRightLr)>
	and	[lr.fConstrainLeftLr],NOT (maskfConstrainLeftLr + maskfConstrainRightLr)
	jmp	LFitLr
FALr18:

;			if (lr.lrk == lrkAbsHit)
;				{
;LTakeRcl:
;				lr.lrs = lrsIgnore;    /* in case too small */
;				RcToDrc(&rcl, &lr.drcl);
;				}
;			}
;		}
	cmp	[lr.lrkLr],lrkAbsHit
	jne	LReplace
LTakeRcl:
	mov	[lr.lrsLr],lrsIgnore
	lea	ax,[rclVar]
	lea	bx,[lr.drclLr]
	cCall	RcToDrc,<ax,bx>

;LReplace:
;	ReplaceInPllr(plbs->hpllr, plbs->ilrCur, &lr);
LReplace:
	lea	ax,[lr]
	push	[si.hpllrLbs]
	push	[si.ilrCurLbs]
	push	ax
ifdef DEBUG
	cCall	S_ReplaceInPllr,<>

	cmp	[vdbs.fShowLbsDbs],0
	jz	FALr175
	push	si
	mov	ax,irtnAssignLr
	push	ax
	cCall	ShowLbsProc,<>
FALr175:
else ;!DEBUG
	push	cs
	call	near ptr N_ReplaceInPllr
endif ;!DEBUG

;/* rectangle was split by an abs object - create or extend chain and redo fit
;   for the new rectangle also */
;LMendChain:
;	if (fPend)
;		{
LMendChain:
	cmp	[fPend],fFalse
	je	FALr22

;		if (plbs->ilrFirstChain == ilrNil)
;			plbs->ilrFirstChain = plbs->ilrCur;
;		else
;			{
;			lrp = LrpInPl(plbs->hpllr, plbs->ilrCurChain);
;			lrp->ilrNextChain = plbs->ilrCur;
;			}
	cmp	[si.ilrFirstChainLbs],ilrNil
	push	[si.ilrCurLbs]
	jne	FALr19
	pop	[si.ilrFirstChainLbs]
	jmp	short FALr20
FALr19:
	mov	cx,[si.ilrCurChainLbs]
	;LN_LrpInPl performs LrpInPl(plbs->hpllr, cx) with the result in es:bx
	;or es:ax.  It assumes plbs is passed in si.
	;ax, bx, dx are altered.
	call	LN_LrpInPl
	pop	es:[bx.ilrNextChainLr]
FALr20:

;		plbs->ilrCurChain = plbs->ilrCur;
;		lrp = LrpInPl(plbs->hpllr, plbs->ilrCur);
	;LN_IMacPllr performs cx = IMacPllr(plbs).  plbs is assumed to be
	;passed in si.	Only bx and cx are altered.
	call	LN_IMacPllr
	xchg	cx,[si.ilrCurLbs]
	mov	[si.ilrCurChainLbs],cx
	;LN_LrpInPl performs LrpInPl(plbs->hpllr, cx) with the result in es:bx
	;or es:ax.  It assumes plbs is passed in si.
	;ax, bx, dx are altered.
	call	LN_LrpInPl

;		plbs->dylChain = min(plbs->dylChain, lrp->dyl);
	mov	ax,es:[bx.dylLr]
	cmp	[si.dylChainLbs],ax
	jle	FALr21
	mov	[si.dylChainLbs],ax
FALr21:

	;Assembler note: the following line is done just after FALr20 in
	;the assembler version.
;		plbs->ilrCur = IMacPllr(plbs);

;		fPend = fFalse;
;		rcl = rclPend;
;		lr.ilrNextChain = ilrNil;
;		lr.fConstrainLeft = fTrue;
;		lr.fConstrainRight = fFalse;
;		goto LHaveRcl;
;		}
	mov	[lr.ilrNextChainLr],ilrNil
	jmp	FALr155

FALr22:

;/* end of chain - check for single entry, otherwise set height of all chained
;  entries to be the min of all entries' heights */
;	if (plbs->ilrFirstChain != ilrNil)
;		{
;		if (plbs->ilrCurChain == plbs->ilrFirstChain && plbs->ilrCur == ilrNil)
;			{
;			plbs->ilrCur = plbs->ilrFirstChain;
;			plbs->ilrFirstChain = ilrNil;
;			lrp = LrpInPl(plbs->hpllr, plbs->ilrCur);
;			lrp->dyl = min(plbs->dylChain, lrp->dyl);
;			}
	mov	cx,[si.ilrFirstChainLbs]
	mov	dx,ilrNil
	cmp	cx,dx
	je	FALr28
	cmp	[si.ilrCurLbs],dx
	jne	FALr23
	cmp	[si.ilrCurChainLbs],cx
	jne	FALr25
	mov	[si.ilrCurLbs],cx
	mov	[si.ilrFirstChainLbs],dx
	;LN_LrpInPl performs LrpInPl(plbs->hpllr, cx) with the result in es:bx
	;or es:ax.  It assumes plbs is passed in si.
	;ax, bx, dx are altered.
	call	LN_LrpInPl
	mov	ax,[si.dylChainLbs]
	cmp	es:[bx.dylLr],ax
	jle	FALr28
	mov	es:[bx.dylLr],ax
	jmp	short FALr28

;		else
;			{
FALr23:

;			if (plbs->ilrCur != ilrNil)
;				{
;				lrp = LrpInPl(plbs->hpllr, plbs->ilrCur);
;				lrp->dyl = plbs->dylChain = min(plbs->dylChain, lrp->dyl);
;				lrp->ilrNextChain = ilrNil;
;				}
	;Assembler note: we know at this point that plbs->ilrCur != ilrNil
	;LN_LrpInPlCur performs LrpInPl(plbs->hpllr, plbs->ilrCur) with
	;the result in es:bx or es:ax.	It assumes plbs is passed in si.
	;ax, bx, cx, dx are altered.
	call	LN_LrpInPlCur
	mov	cx,es:[bx.dylLr]
	cmp	cx,[si.dylChainLbs]
	jle	FALr24
	mov	cx,[si.dylChainLbs]
FALr24:
	mov	[si.dylChainLbs],cx
	mov	es:[bx.dylLr],cx
	mov	es:[bx.ilrNextChainLr],ilrNil
FALr25:

;			for (ilr = plbs->ilrFirstChain; ; ilr = lrp->ilrNextChain)
;				{
;				lrp = LrpInPl(plbs->hpllr, ilr);
;				lrp->dyl = plbs->dylChain;
;				if (ilr == plbs->ilrCurChain)
;					{
	mov	cx,[si.ilrFirstChainLbs]
	jmp	short FALr27
FALr26:
	mov	cx,es:[bx.ilrNextChainLr]
FALr27:
	;LN_LrpInPl performs LrpInPl(plbs->hpllr, cx) with the result in es:bx
	;or es:ax.  It assumes plbs is passed in si.
	;ax, bx, dx are altered.
	call	LN_LrpInPl
	mov	ax,[si.dylChainLbs]
	mov	es:[bx.dylLr],ax
	cmp	cx,[si.ilrCurChainLbs]
	jne	FALr26

;					/* next-to-last entry, chain in last */
;					lrp->ilrNextChain = plbs->ilrCur;
;					break;
;					}
;				}
;			plbs->ilrCurChain = plbs->ilrCur = plbs->ilrFirstChain;
	mov	ax,[si.ilrFirstChainLbs]
	mov	[si.ilrCurChainLbs],ax
	xchg	ax,[si.ilrCurLbs]
	mov	es:[bx.ilrNextChainLr],ax

;			}
;		}
;	plbs->dylOverlap = dylOverlapNew;
;	StopProfile();
;	return(fTrue);
FALr28:
	mov	ax,[dylOverlapNew]
FALr285:
	stc
FALr29:
	mov	[si.dylOverlapLbs],ax
	sbb	ax,ax
;}
cEnd


	;FALr30 returns (lrp->doc == plbs->doc && lrp->ihdt == plbs->ihdt)
	;as zero flag set if true, zero flag reset if false.
	;lrp is assumed in es:bx, plbs is assumed in si.
	;Only ax is altered.
FALr30:
	mov	ax,es:[bx.docLr]
	cmp	ax,[si.docLbs]
	jne	FALr31
	mov	ax,es:[bx.ihdtLr]
	cmp	ax,[si.ihdtLbs]
FALr31:
	ret


	;Assert (ilr < ilrMac);
ifdef DEBUG
FALr32:
	cmp	cx,[ilrMac]
	jl	FALr33
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midLayout2n
	mov	bx,1047
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FALr33:
	ret
endif ;DEBUG


;-------------------------------------------------------------------------
;	FWidowControl(plbs, plbsNew, dylFill, fEmptyOK)
;-------------------------------------------------------------------------
;/* F  W i d o w  C o n t r o l */
;NATIVE int FWidowControl(plbs, plbsNew, dylFill, fEmptyOK)
;struct LBS *plbs, *plbsNew;
;int dylFill, fEmptyOK;
;{
;/* enforce widow/orphan control; returns fFalse if new paragraph should be
;   rejected entirely */
;	LRP lrp = LrpInPl(plbsNew->hpllr, plbsNew->ilrCur);
;	int doc = plbs->doc;
;	int ilnhMac;
;	int fOutline = plbs->fOutline;
;	CP cp = plbs->cp;
;	int cl = plbs->cl;
;	CP cpNew = plbsNew->cp;
;	int clNew = plbsNew->cl;
;	int dxl, dxa;
;	struct PHE phe;
;	struct LBS lbsT;


; %%Function:N_FWidowControl %%Owner:BRADV
cProc	N_FWidowControl,<PUBLIC,FAR>,<si,di>
	ParmW	<plbs>
	ParmW	<plbsNew>
	ParmW	<dylFill>
	ParmW	<fEmptyOK>

	LocalW	clNew
	OFFBP_clNew = -2
	LocalD	cpNew
	OFFBP_cpNew = -6
	LocalW	dxl
	LocalW	dxa
	LocalV	phe, cbPheMin
	LocalV	lbsT, cbLbsMin

cBegin

;	StartProfile();
	mov	si,[plbsNew]
	push	ds
	pop	es
	add	si,(cpLbs)
	lea	di,[cpNew]
	movsw
	movsw
	errnz	<OFFBP_clNew - OFFBP_cpNew - 4>
	errnz	<(clLbs) - (cpLbs) - 4>
	movsw

;	Assert(cpNew >= cp);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	bx,[plbs]
	mov	ax,[OFF_cpNew]
	mov	dx,[SEG_cpNew]
	sub	ax,[bx.LO_cpLbs]
	sbb	dx,[bx.HI_cpLbs]
	jge	FWC005
	mov	ax,midLayout2n
	mov	bx,1030
	cCall	AssertProcForNative,<ax,bx>
FWC005:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

	sub	si,(clLbs) + 2
	;LN_LrpInPlCur performs LrpInPl(plbs->hpllr, plbs->ilrCur) with
	;the result in es:bx or es:ax.	It assumes plbs is passed in si.
	;ax, bx, cx, dx are altered.
	call	LN_LrpInPlCur

;	Assert(lrp->lrk != lrkAbs);
ifdef DEBUG
	cmp	es:[bx.lrkLr],lrkAbs
	jne	FWC01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midLayout2n
	mov	bx,1007
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FWC01:
endif ;DEBUG

;	if (lrp->lrk != lrkNormal && cl != 0)
;		{
;		cp = plbs->cp = CpFromCpCl(plbs, fTrue);
;		cl = plbs->cl = 0;
;		}
	push	es:[bx.dxlLr]	    ;save lrp->dxl
	mov	si,[plbs]
	xor	ax,ax
	errnz	<lrkNormal - 0>
	cmp	es:[bx.lrkLr],al
	je	FWC015
	cmp	[si.clLbs],ax
	je	FWC015
	mov	[si.clLbs],ax
	errnz	<fTrue - 1>
	inc	ax
	cCall	CpFromCpCl,<si, ax>
	mov	[si.LO_cpLbs],ax
	mov	[si.HI_cpLbs],dx
FWC015:

;/* start of para, check for widow; handles 1-line para case */
;	CacheParaL(doc, cpNew, fOutline);
	;LN_CacheParaLbsCp takes plbs in si
	;and performs CacheParaL(plbs->doc, dx:ax, plbs->fOutline).
	;ax, bx, cx, dx are altered.
	mov	ax,[OFF_cpNew]
	mov	dx,[SEG_cpNew]
	call	LN_CacheParaLbsCp

;	if (cp < caParaL.cpFirst)
;		goto LEndWidow; /* cpNew is the beginning of a new paragraph */
	mov	ax,[si.LO_cpLbs]
	mov	dx,[si.HI_cpLbs]
	sub	ax,[caParaL.LO_cpFirstCa]
	sbb	dx,[caParaL.HI_cpFirstCa]
	pop	ax		;restore lrp->dxl
	jl	Ltemp028

;	Assert(FInCa(doc, cp, &caParaL));
ifdef DEBUG
	;Assert this with a call so as not to mess up short jumps.
	call	FWC10
endif ;DEBUG

;	dxa = XaFromXl(dxl = lrp->dxl);
	mov	[dxl],ax
	;LN_XaFromXl performs XaFromXl(ax); ax, bx, cx, dx are altered.
	call	LN_XaFromXl
	mov	[dxa],ax

;	/* make sure a single line is not stranded at the bottom (orphan)*/
;	if (cp == caParaL.cpFirst && cl == 0)
;		{
	mov	ax,[si.LO_cpLbs]
	mov	dx,[si.HI_cpLbs]
	cmp	ax,[caParaL.LO_cpFirstCa]
	jne	FWC04
	cmp	dx,[caParaL.HI_cpFirstCa]
	jne	FWC04
	cmp	[si.clLbs],0
	jne	FWC04

;		if (cpNew == cp && clNew == 1)
;			goto LAvoidOrphan;
	mov	cx,[clNew]
	dec	cx
	jne	FWC02
	cmp	[OFF_cpNew],ax
	jne	FWC02
	cmp	[SEG_cpNew],dx
	je	LAvoidOrphan
FWC02:

;		if (clNew == 0)
;			{
	inc	cx
	jne	FWC04

;			/* we don't have a cl: format the lines */
;			ClFormatLines(plbs, cpMax, ylLarge, ylLarge, 2,
;				dxl, fFalse, fFalse);
	errnz	<LO_cpMax - 0FFFFh>
	errnz	<HI_cpMax - 07FFFh>
	mov	ax,LO_cpMax
	mov	dx,HI_cpMax
	mov	cx,2
	errnz	<fFalse>
	xor	bx,bx
	;FWC09 performs ClFormatLines(si, dx:ax, ylLarge, ylLarge, cx,
	;			      dxl, bx, fFalse);
	;ax, bx, cx, dx are altered.
	call	FWC09

;			if (IInPlcCheck(vfls.hplclnh, cpNew) == 1)
;				{
	push	[vfls.hplclnhFls]
	push	[SEG_cpNew]
	push	[OFF_cpNew]
	cCall	IInPlcCheck,<>
	dec	ax
	jne	FWC04

;LAvoidOrphan:
;				StopProfile();
;				return (!fEmptyOK);
;				}
;			}
;		}
LAvoidOrphan:
	errnz	<fFalse>
	xor	ax,ax
	cmp	[fEmptyOK],ax
	jne	FWC03
	errnz	<fTrue - fFalse - 1>
	inc	ax
FWC03:
	jmp	FWC08
Ltemp028:
	jmp	LEndWidow
FWC04:

;/* Now check for last line being cut off in lbsNew */
;	if (clNew == 0)
;		{
;LClFormatLines:
	mov	ax,[OFF_cpNew]
	mov	dx,[SEG_cpNew]
	cmp	[clNew],0
	jne	FWC05
LClFormatLines:
	push	[vfls.hplclnhFls]	;argument for IInPlc
	push	dx			;argument for IInPlc
	push	ax			;argument for IInPlc

;		/* we don't have a cl: format the lines */
;		ClFormatLines(plbs, cpNew, ylLarge, ylLarge, clMax,
;			dxl, fTrue, fFalse);
	mov	cx,clMax
	mov	bx,fTrue
	;FWC09 performs ClFormatLines(si, dx:ax, ylLarge, ylLarge, cx,
	;			      dxl, bx, fFalse);
	;ax, bx, cx, dx are altered.
	call	FWC09

;		clNew = IInPlc(vfls.hplclnh, cpNew);
	cCall	IInPlc,<>
	xchg	ax,di

;		ilnhMac = IMacPlc(vfls.hplclnh);
	mov	bx,[vfls.hplclnhFls]
	;***Begin in line IMacPlc
	mov	bx,[bx]
	mov	ax,[bx.iMacPlcStr]
	;***End in line IMacPlc

;		if (CpPlc(vfls.hplclnh, ilnhMac) >= vfls.ca.cpLim)
;			{
	xchg	ax,cx
	push	cx	;save ilnhMac
	;LN_CpPlc performs CpPlc(vfls.hplclnh, cx)
	;ax, bx, cx, dx are altered.
	call	LN_CpPlc
	pop	cx	;restore ilnhMac
	sub	ax,[vfls.caFls.LO_cpLimCa]
	sbb	dx,[vfls.caFls.HI_cpLimCa]
	jl	Ltemp009

;			/* check for 4 lines is still necessary in unusual
;			   circumstances (very tall lines) */
;			if (ilnhMac < 4)
;				/* no way to avoid widow in 2 or 3 lines */
;				goto LAvoidOrphan;
	cmp	cx,4
Ltemp006:
	jb	LAvoidOrphan

;			if (clNew == ilnhMac - 1)
;				{
	dec	cx
	cmp	di,cx
	jne	Ltemp009

;				/* back up a line unless this would violate fEmptyOK */
;				if (fEmptyOK || cl > 0 || CpPlc(vfls.hplclnh, clNew - 1) > cp)
;					goto LHaveWidow;
	cmp	[fEmptyOK],fFalse
	jne	LHaveWidow
	cmp	[si.clLbs],0
	jg	LHaveWidow
	;LN_CpPlc performs CpPlc(vfls.hplclnh, cx)
	;ax, bx, cx, dx are altered.
	mov	cx,di
	dec	cx
	call	LN_CpPlc
	cmp	[si.LO_cpLbs],ax
	mov	ax,[si.HI_cpLbs]
	sbb	ax,dx
	jl	LHaveWidow

;				}
;			}
;		}
Ltemp009:
	jmp	short LEndWidow

;	else
;		{  /* clNew > 0 */
;		CacheParaL(doc, cpNew, fOutline);
FWC05:
	lea	bx,[phe]
	push	[si.docLbs]	    ;argument for FGetValidPhe
	push	dx		    ;argument for FGetValidPhe
	push	ax		    ;argument for FGetValidPhe
	push	bx		    ;argument for FGetValidPhe
	;LN_CacheParaLbsCp takes plbs in si
	;and performs CacheParaL(plbs->doc, dx:ax, plbs->fOutline).
	;ax, bx, cx, dx are altered.
	call	LN_CacheParaLbsCp

;		if (!FGetValidPhe(doc, cpNew, &phe) || phe.fDiffLines ||
;			phe.dxaCol != dxa || phe.dylHeight == 0)
;			{
	cCall	FGetValidPhe,<>
	or	ax,ax
	je	FWC06
	test	[phe.fDiffLinesPhe],maskfDiffLinesPhe
	jne	FWC06
	mov	ax,[dxa]
	cmp	[phe.dxaColPhe],ax
	jne	FWC06
	cmp	[phe.dylHeightPhe],0
	jne	FWC07
FWC06:

;			/* This is rare: the phe ended up getting zeroed somehow */
;			Assert(!phe.fDiffLines && phe.dylHeight == 0);
ifdef DEBUG
	;Assert this with a call so as not to mess up short jumps.
	call	FWC12
endif ;DEBUG

;			cpNew = CpFromCpCl(plbsNew, fTrue);
;			clNew = 0;
;			goto LClFormatLines;
;			}
	xor	ax,ax
	mov	[clNew],ax
	errnz	<fTrue - 1>
	inc	ax
	cCall	CpFromCpCl,<[plbsNew], ax>
	jmp	LClFormatLines
FWC07:

;		Assert(phe.clMac >= clNew);
ifdef DEBUG
	;Assert this with a call so as not to mess up short jumps.
	call	FWC15
endif ;DEBUG
;		if (phe.clMac < 4)
;			/* no way to avoid widow in 2 or 3 lines */
;			goto LAvoidOrphan;
	mov	al,[phe.clMacPhe]
	cmp	al,4
	jb	Ltemp006

;		if (phe.clMac == clNew + 1)
;			{
	xor	ah,ah
	mov	di,[clNew]
	sub	ax,di
	dec	ax
	jne	LEndWidow

;			/* note that rolling back the cl's is not enough:
;			   LbcFormatPara has other side effects such as
;			   advancing the yl, lnn, etc. */
;			/* stranded line */
;LHaveWidow:
;			PushLbs(plbs, &lbsT);
;			CopyLbs(&lbsT, plbsNew);
;			FAssignLr(plbsNew, dylFill, fFalse);
;			LbcFormatPara(plbsNew, dylFill, clNew - 1);
LHaveWidow:
	;expect di = clNew
	mov	ax,[plbsNew]
	lea	bx,[lbsT]
	mov	cx,[dylFill]
	errnz	<fFalse>
	xor	dx,dx
	dec	di
	push	ax	    ;argument for LbcFormatPara
	push	cx	    ;argument for LbcFormatPara
	push	di	    ;argument for LbcFormatPara
	push	ax	    ;argument for FAssignLr
	push	cx	    ;argument for FAssignLr
	push	dx	    ;argument for FAssignLr
	push	bx	    ;argument for CopyLbs
	push	ax	    ;argument for CopyLbs
	push	si
	push	bx
ifdef DEBUG
	cCall	S_PushLbs,<>
else ;!DEBUG
	push	cs
	call	near ptr N_PushLbs
endif ;!DEBUG
ifdef DEBUG
	cCall	S_CopyLbs,<>
else ;!DEBUG
	push	cs
	call	near ptr N_CopyLbs
endif ;!DEBUG
ifdef DEBUG
	cCall	S_FAssignLr,<>
else ;!DEBUG
	push	cs
	call	near ptr N_FAssignLr
endif ;!DEBUG
ifdef DEBUG
	cCall	S_LbcFormatPara,<>
else ;!DEBUG
	push	cs
	call	near ptr N_LbcFormatPara
endif ;!DEBUG

;			}
;		}
;LEndWidow:
LEndWidow:

;	StopProfile();
;	return(fTrue);
	mov	ax,fTrue
FWC08:

;}
cEnd


	;FWC09 performs ClFormatLines(si, dx:ax, ylLarge, ylLarge, cx,
	;			      dxl, bx, fFalse);
	;ax, bx, cx, dx are altered.
FWC09:
	push	si
	push	dx
	push	ax
	mov	ax,ylLarge
	push	ax
	push	ax
	push	cx
	push	[dxl]
	errnz	<ylLarge - 03FFFh>
	errnz	<fFalse>
	cwd
	push	bx
	push	dx
ifdef DEBUG
	cCall	S_ClFormatLines,<>
else ;!DEBUG
	push	cs
	call	near ptr N_ClFormatLines
endif ;!DEBUG
	ret

;	Assert(FInCa(doc, cp, &caParaL));
ifdef DEBUG
FWC10:
	push	ax
	push	bx
	push	cx
	push	dx
	push	[si.docLbs]
	push	[si.HI_cpLbs]
	push	[si.LO_cpLbs]
	mov	ax,dataoffset [caParaL]
	push	ax
	cCall	FInCa,<>
	or	ax,ax
	jne	FWC11
	mov	ax,midLayout2n
	mov	bx,1049
	cCall	AssertProcForNative,<ax,bx>
FWC11:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG


;			Assert(!phe.fDiffLines && phe.dylHeight == 0);
ifdef DEBUG
FWC12:
	push	ax
	push	bx
	push	cx
	push	dx
	test	[phe.fDiffLinesPhe],maskfDiffLinesPhe
	jne	FWC13
	cmp	[phe.dylHeightPhe],0
	je	FWC14
FWC13:
	mov	ax,midLayout2n
	mov	bx,1050
	cCall	AssertProcForNative,<ax,bx>
FWC14:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG

;		Assert(phe.clMac >= clNew);
ifdef DEBUG
FWC15:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	al,[phe.clMacPhe]
	xor	ah,ah
	cmp	ax,[clNew]
	jge	FWC16
	mov	ax,midLayout2n
	mov	bx,1051
	cCall	AssertProcForNative,<ax,bx>
FWC16:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG

;			Assert(lrp->cpLim == plbsNew->cp && lrp->clLim == plbsNew->cl);
ifdef DEBUG
FWC17:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[si.LO_cpLbs]
	cmp	ax,es:[bx.LO_cpLimLr]
	jne	FWC18
	mov	ax,[si.HI_cpLbs]
	cmp	ax,es:[bx.HI_cpLimLr]
	jne	FWC18
	mov	ax,[si.clLbs]
	cmp	ax,es:[bx.clLimLr]
	je	FWC19
FWC18:
	mov	ax,midLayout2n
	mov	bx,1052
	cCall	AssertProcForNative,<ax,bx>
FWC19:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG


;-------------------------------------------------------------------------
;	IfrdGatherFtnRef(plbs, plbsNew, ifrd, fNormal, ylReject, ylAccept)
;-------------------------------------------------------------------------
;/* I f r d  G a t h e r  F t n  R e f */
;NATIVE int IfrdGatherFtnRef(plbs, plbsNew, ifrd, fNormal, ylReject, ylAccept)
;struct LBS *plbs, *plbsNew;
;int ifrd, fNormal, ylReject, ylAccept;
;{
;	struct DOD *pdod;
;	int ifrl;
;	struct PLC **hplcfrd;
;	CP cpLimFnd, cpRef, cpMac = CpMacDocPlbs(plbs);
;	LRP lrp;
;	struct FRL frlNew;

; %%Function:N_IfrdGatherFtnRef %%Owner:BRADV
cProc	N_IfrdGatherFtnRef,<PUBLIC,FAR>,<si,di>
	ParmW	<plbs>
	ParmW	<plbsNew>
	ParmW	<ifrd>
	ParmW	<fNormal>
	ParmW	<ylReject>
	ParmW	<ylAccept>

	LocalD	cpLimFnd
	LocalV	frlNew, cbFrlMin

cBegin

	mov	si,[plbs]

;	StartProfile();
;	cpLimFnd = CpFromCpCl(plbsNew, fTrue);
	mov	ax,fTrue
	cCall	CpFromCpCl,<[plbsNew], ax>
	mov	[OFF_cpLimFnd],ax
	mov	[SEG_cpLimFnd],dx

	;LN_CpMacDocPlbs performs CpMacDocPlbs(si).
	;ax, bx, cx, dx are altered.
	call	LN_CpMacDocPlbs

;	if ((frlNew.fNormal = fNormal) != fFalse)
	mov	di,[ylReject]
	mov	bx,[ylAccept]
	mov	cx,[fNormal]
	jcxz	IGFR01

;		if (plbs->cp >= cpMac)
;			{
;			ylReject = ylAccept = 1;
;			frlNew.fNormal = fFalse;
;			}
;		else
;			{
;			lrp = LrpInPl(plbsNew->hpllr, plbsNew->ilrCur);
;			ylReject = max(plbs->yl, lrp->yl);
;			ylAccept = plbsNew->yl;
;			}
	mov	di,1
	mov	bx,di
	xor	cx,cx
	cmp	[si.LO_cpLbs],ax
	mov	ax,[si.HI_cpLbs]
	sbb	ax,dx
	jge	IGFR01
	push	si	;save plbs
	;LN_LrpInPlCur performs LrpInPl(plbs->hpllr, plbs->ilrCur) with
	;the result in es:bx or es:ax.	It assumes plbs is passed in si.
	;ax, bx, cx, dx are altered.
	mov	si,[plbsNew]
	call	LN_LrpInPlCur
	mov	di,es:[bx.ylLr]
	mov	bx,[si.ylLbs]
	pop	si	;restore plbs
	mov	cx,[fNormal]
	cmp	di,[si.ylLbs]
	jge	IGFR01
	mov	di,[si.ylLbs]

;	frlNew.ylReject = ylReject;
;	frlNew.ylAccept = ylAccept;
IGFR01:
	mov	[frlNew.fNormalFrl],cl
	mov	[frlNew.ylRejectFrl],di
	mov	[frlNew.ylAcceptFrl],bx

;/* all refs between cp of lbs and lbsNew */
;	pdod = PdodDoc(plbs->doc); /* two lines for compiler */
	mov	bx,[si.docLbs]
	;Takes doc in bx, result in bx.  Only bx is altered.
	call	LN_PdodDocL

;	hplcfrd = pdod->hplcfrd;
	mov	di,[bx.hplcfrdDod]

;	for (ifrl = IMacPlc(vhplcfrl); (cpRef = CpPlc(hplcfrd, ifrd)) < cpLimFnd; ifrd++)
;		{
	;***Begin in line IMacPlc
	mov	bx,[vhplcfrl]
	mov	bx,[bx]
	mov	cx,[bx.iMacPlcStr]
	;***End in line IMacPlc
	mov	si,[ifrd]
	dec	si	;adjust for for loop
IGFR02:
	inc	si
	push	cx	;save ifrl
	cCall	CpPlc,<di, si>
	pop	cx	;restore ifrl
	cmp	ax,[OFF_cpLimFnd]
	mov	bx,dx
	sbb	bx,[SEG_cpLimFnd]
	jge	IGFR04

;		frlNew.ifnd = ifrd;
	mov	[frlNew.ifndFrl],si

;		if (!FInsertInPlc(vhplcfrl, ifrl++, cpRef, &frlNew))
;			{
	push	cx	;save ifrl
	lea	bx,[frlNew]
ifdef DEBUG
	cCall	S_FInsertInPlc,<[vhplcfrl], cx, dx, ax, bx>
else ;not DEBUG
	cCall	N_FInsertInPlc,<[vhplcfrl], cx, dx, ax, bx>
endif ;DEBUG
	pop	cx	;restore ifrl
	inc	cx
	or	ax,ax
	jne	IGFR02

;			Assert(vfInFormatPage);
ifdef DEBUG
	cmp	[vfInFormatPage],fFalse
	jne	IGFR03
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midLayout2n
	mov	bx,1008
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
IGFR03:
endif ;DEBUG

;			AbortLayout();
	cCall	AbortLayout,<>

;			}
;		}
IGFR04:

;	StopProfile();
;	return(ifrd);
	xchg	ax,si

;}
cEnd


;-------------------------------------------------------------------------
;	FGetFtnBreak(plbs, ifrl, pfrl, fpc)
;-------------------------------------------------------------------------
;/* F	G e t	F t n	B r e a k */
;NATIVE FGetFtnBreak(plbs, ifrl, pfrl, fpc)
;struct LBS *plbs;
;int ifrl;	 /* negative when we're recursing; ifrl = abs(ifrl) */
;struct FRL *pfrl;
;int fpc;
;{
;/* call ClFormatLines to determine what line contains a footnote reference,
;   set frl accordingly; returns fFalse if no better information available */
;	int ilr, ilrMac = IMacPllr(plbs), ilrBest = -1;
;	LRP lrp;
;	int fWidow, ilnhMac;
;	struct PLC **hplclnh = vfls.hplclnh;
;	int cl, clFirst, clRef;
;	YLL dyllPrevCol;
;	int ylAccept, ylReject;
;	CP cpRef, dcp, dcpBest = cpMax;
;	CP cpLim;
;	struct DOD *pdod;
;	struct PLC **hplcfrd;
;	struct LNH lnh;
;	struct LR lr;
;	struct FRL frl, frlT;
;	struct LBS lbsT;

; %%Function:N_FGetFtnBreak %%Owner:BRADV
cProc	N_FGetFtnBreak,<PUBLIC,FAR>,<si,di>
	ParmW	<plbs>
	ParmW	<ifrl>
	ParmW	<pfrl>
	ParmW	<fpc>

	LocalW	ilrMac
	LocalW	ilrBest
	LocalW	fWidow
	LocalW	clFirst
	LocalW	hplcfrd
	LocalW	clRef
	LocalD	cpRef
	LocalD	dcpBest
	LocalD	dyllPrevCol
	LocalV	frl, cbFrlMin
	LocalV	lnh, cbLnhMin
	LocalV	lr, cbLrMin
	LocalV	lbsT, cbLbsMin
	LocalV	frlT, cbFrlMin

cBegin

	mov	si,[plbs]
	;LN_IMacPllr performs cx = IMacPllr(plbs).  plbs is assumed to be
	;passed in si.	Only bx and cx are altered.
	call	LN_IMacPllr
	mov	[ilrMac],cx
	errnz	<LO_cpMax - 0FFFFh>
	errnz	<HI_cpMax - 07FFFh>
	mov	ax,HI_cpMax
	mov	[SEG_dcpBest],ax
	cbw
	mov	[OFF_dcpBest],ax
	mov	[ilrBest],ax

;/* find the lr containing the reference */
;	StartProfile();
;	if (fpc == fpcEndnote || fpc == fpcEndDoc)
;		goto LNoInfo;	/* endnotes -- essentially no reference */
	mov	al,bptr ([fpc])
	cmp	al,fpcEndNote
	je	Ltemp012
	cmp	al,fpcEndDoc
	je	Ltemp012

;	if (fRecurse = (ifrl < 0))
;		ifrl = -ifrl;
;	cpRef = CpPlc(vhplcfrl, ifrl);
	mov	ax,[ifrl]   ;Not negative, just high bit toggled in .asm
	and	ah,07Fh
	cCall	CpPlc,<[vhplcfrl],ax>
	mov	[OFF_cpRef],ax
	mov	[SEG_cpRef],dx

;	for (lrp = LrpInPl(plbs->hpllr, ilr = 0); ilr < ilrMac; lrp++, ilr++)
;		{
	xor	cx,cx
FGFB01:
	;LN_LrpInPl performs LrpInPl(plbs->hpllr, cx) with the result in es:bx
	;or es:ax.  It assumes plbs is passed in si.
	;ax, bx, dx are altered.
	call	LN_LrpInPl
	jmp	short FGFB03
Ltemp012:
	jmp	FGFB17
FGFB02:
	inc	cx
	add	bx,cbLrMin
FGFB03:
	cmp	cx,[ilrMac]
	jge	FGFB04

;		if (lrp->doc != plbs->doc || cpRef < lrp->cp)
;			continue;
	mov	ax,[si.docLbs]
	cmp	ax,es:[bx.docLr]
	jne	FGFB02
	mov	ax,[OFF_cpRef]
	mov	dx,[SEG_cpRef]
	sub	ax,es:[bx.LO_cpLr]
	sbb	dx,es:[bx.HI_cpLr]
	jl	FGFB02
	push	dx
	push	ax	;save cpRef - lrp->cp

;		cpLim = lrp->cpLim;
;		if (lrp->clLim != 0)
;			{
	mov	ax,es:[bx.LO_cpLimLr]
	mov	dx,es:[bx.HI_cpLimLr]
	cmp	es:[bx.clLimLr],0
	push	dx
	push	ax	;save cpLim
	je	FGFB035

;			lbsT = *plbs;
	push	cx	;save ilr
	push	es:[bx.clLimLr]
	push	si	;save plbs
	push	ds
	pop	es
	lea	di,[lbsT]
	errnz	<cbLbsMin AND 1>
	mov	cx,cbLbsMin SHR 1
	rep	movsw
	pop	si	;restore plbs

;			lbsT.cp = cpLim;
;			lbsT.cl = lrp->clLim;
;			cpLim = CpFromCpCl(&lbsT, fTrue);
	sub	di,cbLbsMin
	mov	[di.LO_cpLbs],ax
	mov	[di.HI_cpLbs],dx
	pop	[di.clLbs]
	mov	ax,fTrue
	cCall	CpFromCpCl,<di,ax>
	pop	cx	;restore ilr

;			lrp = LrpInPl(plbs->hpllr, ilr);
	;LN_LrpInPl performs LrpInPl(plbs->hpllr, cx) with the result in es:bx
	;or es:ax.  It assumes plbs is passed in si.
	;ax, bx, dx are altered.
	call	LN_LrpInPl

;			}
;		if (cpRef >= cpLim)
;			continue;
FGFB035:
	pop	ax
	pop	dx	;restore cpLim
	cmp	[OFF_cpRef],ax
	mov	ax,[SEG_cpRef]
	sbb	ax,dx
	pop	ax
	pop	dx	;restore cpRef - lrp->cp
	jge	FGFB02

;		if ((dcp = cpRef - lrp->cp) < dcpBest)
;			{
	cmp	ax,[OFF_dcpBest]
	mov	di,dx
	sbb	di,[SEG_dcpBest]
	jge	FGFB02

;			ilrBest = ilr;
;			dcpBest = dcp;
	mov	[ilrBest],cx
	mov	[OFF_dcpBest],ax
	mov	[SEG_dcpBest],dx

;			}
	jmp	FGFB02
Ltemp010:
	jmp	FGFB17
FGFB04:

;	Assert(ilrBest != -1);
ifdef DEBUG
	cmp	[ilrBest],-1
	jne	FGFB05
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midLayout2n
	mov	bx,1009
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FGFB05:
endif ;DEBUG

;	if (ilrBest == -1)
;		goto LNoInfo;
	mov	cx,[ilrBest]
	cmp	cx,-1
	je	Ltemp010

;	lrp = LrpInPl(plbs->hpllr, ilrBest);
	;LN_LrpInPl performs LrpInPl(plbs->hpllr, cx) with the result in es:bx
	;or es:ax.  It assumes plbs is passed in si.
	;ax, bx, dx are altered.
	call	LN_LrpInPl
	xchg	si,ax	    ;save plbs

;	if (lrp->lrk == lrkAbs)
;		goto LNoInfo;
	cmp	es:[si.lrkLr],lrkAbs
	je	Ltemp010

;	lr = *lrp;
	push	es
	pop	ds
	push	ss
	pop	es
	lea	di,[lr]
	errnz	<cbLrMin AND 1>
	mov	cx,cbLrMin SHR 1
	rep	movsw
	push	ss
	pop	ds

;/* set up LBS */
;	lbsT = *plbs;
	xchg	ax,si
	lea	di,[lbsT]
	errnz	<cbLbsMin AND 1>
	mov	cx,cbLbsMin SHR 1
	rep	movsw

;	lbsT.cp = cpRef;
	lea	si,[cpRef]
	sub	di,cbLbsMin - (cpLbs)
	movsw
	movsw

;	CacheParaL(lbsT.doc, lbsT.cp, lbsT.fOutline);
;	/* find cp/cl at which to start formatting */
	lea	si,[di - (cpLbs) - 4]
	;LN_CacheParaLbs takes plbs in si
	;and performs CacheParaL(plbs->doc, plbs->cp, plbs->fOutline).
	;ax, bx, cx, dx are altered.
	call	LN_CacheParaLbs

;	if (lr.cp >= caParaL.cpFirst)
;		{
;		lbsT.cp = lr.cp;
;		lbsT.cl = lr.clFirst;
;		}
;	else
;		{
;		lbsT.cp = caParaL.cpFirst;
;		lbsT.cl = 0;
;		}
	mov	ax,[lr.LO_cpLr]
	mov	dx,[lr.HI_cpLr]
	mov	cx,[lr.clFirstLr]
	cmp	ax,[caParaL.LO_cpFirstCa]
	mov	bx,dx
	sbb	bx,[caParaL.HI_cpFirstCa]
	jge	FGFB055
	mov	ax,[caParaL.LO_cpFirstCa]
	mov	dx,[caParaL.HI_cpFirstCa]
	xor	cx,cx
FGFB055:
	mov	[si.LO_cpLbs],ax
	mov	[si.HI_cpLbs],dx
	mov	[si.clLbs],cx

;	GetPlc(vhplcfrl, ifrl, &frl);	 /* NOT the same as *pfrl */
	mov	ax,[ifrl]
	and	ah,07Fh     ;Not negative, just high bit toggled in .asm
	lea	bx,[frl]
	cCall	GetPlc,<[vhplcfrl], ax, bx>

;	lbsT.yl = frl.ylReject;
	mov	ax,[frl.ylRejectFrl]
	mov	[si.ylLbs],ax

;/* calculate above/below based on line with ref; to do widow control we
;   need entire para */
;	pdod = PdodDoc(lbsT.doc);
	mov	bx,[si.docLbs]
	;Takes doc in bx, result in bx.  Only bx is altered.
	call	LN_PdodDocL

;	fWidow = pdod->dop.fWidowControl;
	mov	al,[bx.dopDod.fWidowControlDop]
	mov	bptr ([fWidow]),al

;	hplcfrd = pdod->hplcfrd;
	mov	cx,[bx.hplcfrdDod]
	mov	[hplcfrd],cx

;	ClFormatLines(&lbsT, (fWidow) ? caParaL.cpLim : cpRef,
;		ylLarge, ylLarge, clMax, lr.dxl, !fWidow, fFalse);
	xor	cx,cx
	push	si
	test	al,maskfWidowControlDop
	je	FGFB06
	push	[caParaL.HI_cpLimCa]
	push	[caParaL.LO_cpLimCa]
	jmp	short FGFB07
FGFB06:
	inc	cx
	push	[SEG_cpRef]
	push	[OFF_cpRef]
FGFB07:
	errnz	<ylLarge - 03FFFh>
	errnz	<clMax - 07FFFh>
	mov	ax,ylLarge
	push	ax
	push	ax
	mov	ah,clMax SHR 8
	push	ax
	push	[lr.dxlLr]
	push	cx
	errnz	<fFalse>
	xor	ax,ax
	push	ax
ifdef DEBUG
	cCall	S_ClFormatLines,<>
else ;!DEBUG
	push	cs
	call	near ptr N_ClFormatLines
endif ;!DEBUG

;	Assert(IMacPlc(hplclnh) != 0);
;	Assert(cpRef < CpPlc(hplclnh, IMacPlc(hplclnh)));
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	;***Begin in line IMacPlc
	mov	bx,[vfls.hplclnhFls]
	mov	bx,[bx]
	mov	cx,[bx.iMacPlcStr]
	;***End in line IMacPlc
	cmp	cx,0
	jne	FGFB08
	mov	ax,midLayout2n
	mov	bx,1010
	cCall	AssertProcForNative,<ax,bx>
FGFB08:
	;LN_CpPlc performs CpPlc(vfls.hplclnh, cx)
	;ax, bx, cx, dx are altered.
	call	LN_CpPlc
	mov	bx,[OFF_cpRef]
	sub	bx,ax
	mov	bx,[SEG_cpRef]
	sbb	bx,dx
	jl	FGFB09
	mov	ax,midLayout2n
	mov	bx,1011
	cCall	AssertProcForNative,<ax,bx>
FGFB09:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

;	/* we have to adjust for lines that have already been used in a
;	   previous column or page */
;	if ((clFirst = lbsT.cl + IInPlc(hplclnh, lbsT.cp)) == 0)
;		dyllPrevCol = (YLL)0;
;	else
;		{
;		GetPlc(hplclnh, clFirst - 1, &lnh);
;		dyllPrevCol = lnh.yll;
;		}
	;LN_IInPlc performs IInPlc(vfls.hplclnh, *pcp) where pcp is passed
	;in bx.  ax, bx, cx, dx are altered.
	lea	bx,[si.cpLbs]
	call	LN_IInPlc
	add	ax,[si.clLbs]
	mov	[clFirst],ax
	xchg	ax,cx
	xor	ax,ax
	cwd
	jcxz	FGFB10
	;FGFB19 performs GetPlc(vfls.hplclnh, cx, &lnh)
	;ax, bx, cx, dx are altered.
	dec	cx
	call	FGFB19
	mov	ax,[lnh.LO_yllLnh]
	mov	dx,[lnh.HI_yllLnh]
FGFB10:
	mov	[OFF_dyllPrevCol],ax
	mov	[SEG_dyllPrevCol],dx

;	cl = clRef = IInPlc(hplclnh, cpRef);  /* ref is on line cl + 1 */
	;LN_IInPlc performs IInPlc(vfls.hplclnh, *pcp) where pcp is passed
	;in bx.  ax, bx, cx, dx are altered.
	lea	bx,[cpRef]
	call	LN_IInPlc
	mov	[clRef],ax
	xchg	ax,cx

;	ylReject = lbsT.yl;
	mov	ax,[si.ylLbs]

;	if (cl != clFirst && !(cl == 1 && fWidow))
;		{
	cmp	cx,[clFirst]
	je	FGFB13
	cmp	cx,1
	jne	FGFB11
	test	bptr ([fWidow]),maskfWidowControlDop
	jne	FGFB13
FGFB11:
	push	cx	;save cl
	dec	cx

;		GetPlc(hplclnh, cl - 1, &lnh);
	;FGFB19 performs GetPlc(vfls.hplclnh, cx, &lnh)
	;ax, bx, cx, dx are altered.
	call	FGFB19
	pop	cx	;restore cl

;		Assert(lnh.yll - dyllPrevCol < ylLarge);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[lnh.LO_yllLnh]
	mov	dx,[lnh.HI_yllLnh]
	sub	ax,[OFF_dyllPrevCol]
	sbb	dx,[SEG_dyllPrevCol]
	errnz	<ylLarge - 03FFFh>
	sub	ax,ylLarge
	sbb	dx,0
	jb	FGFB12
	mov	ax,midLayout2n
	mov	bx,1012
	cCall	AssertProcForNative,<ax,bx>
FGFB12:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

;		ylReject = (int)(lnh.yll - dyllPrevCol) + lbsT.yl;
	mov	ax,[lnh.LO_yllLnh]
	sub	ax,[OFF_dyllPrevCol]
	add	ax,[si.ylLbs]

;		}
FGFB13:
	push	ax	;save ylReject

;	if (fWidow && CpPlc(hplclnh, ilnhMac = IMacPlc(hplclnh)) == vfls.ca.cpLim)
;		{
	test	bptr ([fWidow]),maskfWidowControlDop
	je	FGFB14
	push	cx	;save cl
	mov	bx,[vfls.hplclnhFls]
	push	bx
	;***Begin in line IMacPlc
	mov	bx,[bx]
	mov	di,[bx.iMacPlcStr]
	;***End in line IMacPlc
	push	di
	cCall	CpPlc,<>
	pop	cx	;restore cl
	sub	ax,[vfls.caFls.LO_cpLimCa]
	sbb	dx,[vfls.caFls.HI_cpLimCa]
	or	ax,dx
	jne	FGFB14

;		if (ilnhMac < 4)
;			{
;			/* need whole para */
;			ylAccept = frl.ylAccept;
;			ylReject = frl.ylReject;
;			goto LCheckFirst;
;			}
	cmp	di,4
	jge	FGFB133
	pop	ax	;restore ylReject
	push	[frl.ylRejectFrl]
	mov	di,[frl.ylAcceptFrl]
	jmp	short LCheckFirst
FGFB133:

;		if (cl == ilnhMac - 2 || (cl == 0 && vfls.ca.cpFirst == caParaL.cpFirst))
;			cl++; /* reference in first or next-to-last line */
;		}
	dec	di
	dec	di
	cmp	cx,di
	je	FGFB137
	mov	ax,[caParaL.LO_cpFirstCa]
	mov	dx,[caParaL.HI_cpFirstCa]
	sub	ax,[vfls.caFls.LO_cpFirstCa]
	sbb	dx,[vfls.caFls.HI_cpFirstCa]
	or	ax,dx
	or	ax,cx
	jne	FGFB14
FGFB137:
	inc	cx
FGFB14:

;	GetPlc(hplclnh, cl, &lnh);
	;FGFB19 performs GetPlc(vfls.hplclnh, cx, &lnh)
	;ax, bx, cx, dx are altered.
	call	FGFB19

;	Assert(lnh.yll - dyllPrevCol < ylLarge);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[lnh.LO_yllLnh]
	mov	dx,[lnh.HI_yllLnh]
	sub	ax,[OFF_dyllPrevCol]
	sbb	dx,[SEG_dyllPrevCol]
	errnz	<ylLarge - 03FFFh>
	sub	ax,ylLarge
	sbb	dx,0
	jb	FGFB15
	mov	ax,midLayout2n
	mov	bx,1013
	cCall	AssertProcForNative,<ax,bx>
FGFB15:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

;	ylAccept = (int)(lnh.yll - dyllPrevCol) + lbsT.yl;
	mov	di,[lnh.LO_yllLnh]
	sub	di,[OFF_dyllPrevCol]
	add	di,[si.ylLbs]

;/* if not first footnote on line, we can't reject it */
;LCheckFirst:
;	if (!fRecurse)
;		if (cpRef > CpPlc(hplcfrd, 0) &&
;			CpPlc(hplclnh, clRef) <= CpPlc(hplcfrd, IInPlc(hplcfrd, cpRef - 1)))
;			{
;			ylReject = ylAccept;
;			}
LCheckFirst:
	xor	bptr ([ifrl+1]),080h
	jns	FGFB155
	xor	ax,ax
	cCall	CpPlc,<[hplcfrd], ax>
	sub	ax,[OFF_cpRef]
	sbb	dx,[SEG_cpRef]
	jge	FGFB155
	mov	ax,[OFF_cpRef]
	mov	dx,[SEG_cpRef]
	sub	ax,1
	sbb	dx,0
	mov	cx,[hplcfrd]
	push	cx	;argument for CpPlc
	cCall	IInPlc,<cx, dx, ax>
	push	ax
	cCall	CpPlc,<>
	push	dx
	push	ax	;save cp
	;LN_CpPlc performs CpPlc(vfls.hplclnh, cx)
	;ax, bx, cx, dx are altered.
	mov	cx,[clRef]
	call	LN_CpPlc
	pop	cx
	sub	cx,ax
	pop	cx
	sbb	cx,dx
	jge	FGFB152

;		else if (ifrl > 0)
;			{

	mov	ax,[ifrl]
	test	ax,07FFFh
	jz	FGFB155
;			/* if reject point same as previous footnote, we can't reject it */
;			frlT = frl;
;			FGetFtnBreak(plbs, -(ifrl - 1), &frlT, fpc);
;			if (ylReject == frlT.ylReject)
;				ylReject = ylAccept;
;			}
	push	di	;save ylAccept
	push	[plbs]
	dec	ax	;High bit toggled above
	push	ax	;Not negative, just high bit toggled in .asm
	lea	di,[frlT]
	push	di
	push	ds
	pop	es
	lea	si,[frl]
	errnz	<cbFrlMin - 8>
	movsw
	movsw
	movsw
	movsw
	push	[fpc]
	push	cs
	call	near ptr N_FGetFtnBreak
	pop	di	;restore ylAccept
	pop	ax	;restore ylReject
	push	ax	;save ylReject
	cmp	ax,[frlT.ylRejectFrl]
	jne	FGFB155
FGFB152:
	pop	ax	;remove old ylReject
	push	di	;save ylReject = ylAccept
FGFB155:

;	if (ylAccept == frl.ylAccept && ylReject == frl.ylReject)
;		{
;LNoInfo:
;		StopProfile();
;		return(fFalse);
;		}
	pop	ax	;restore ylReject
	cmp	di,[frl.ylAcceptFrl]
	jne	FGFB16
	cmp	ax,[frl.ylRejectFrl]
	je	FGFB17
FGFB16:

;	pfrl->ylAccept = ylAccept;
;	pfrl->ylReject = ylReject;
	mov	bx,[pfrl]
	mov	[bx.ylAcceptFrl],di
	mov	[bx.ylRejectFrl],ax

;	StopProfile();
;	return(fTrue);
	mov	ax,fTrue
	db	03Dh	    ;turns next "xor ax,ax" into "cmp ax,immediate"
FGFB17:
	xor	ax,ax
;}
cEnd


	;FGFB19 performs GetPlc(vfls.hplclnh, cx, &lnh)
	;ax, bx, cx, dx are altered.
FGFB19:
	lea	bx,[lnh]
	cCall	GetPlc,<[vfls.hplclnhFls], cx, bx>
	ret


;-------------------------------------------------------------------------
;	CopyHdtLrs(ihdt, plbs, yl, plbsAbs)
;-------------------------------------------------------------------------
;/* C o p y  H d t  L r s */
;NATIVE CopyHdtLrs(ihdt, plbs, yl)
;int ihdt;
;struct LBS *plbs;
;int yl;
;struct LBS *plbsAbs;	 /* reference for abs objects */
;{

Ltemp030:
	jmp	CHL03

Ltemp029:
	jmp	CHL04

; %%Function:N_CopyHdtLrs %%Owner:BRADV
cProc	N_CopyHdtLrs,<PUBLIC,FAR>,<si,di>
	ParmW	<ihdt>
	ParmW	<plbs>
	ParmW	<yl>
	ParmW	<plbsAbs>

	LocalV	lbsT,cbLbsMin

cBegin

;/* copy some header LRs, if any, onto a page. Footnote separators must have
;   a non-zero height to be used, but headers can be zero height if they are
;   entirely composed of absolute objects */
;	int xl = -1;
;	struct HDT *phdt = &vrghdt[ihdt];
;	struct LBS lbsT;
;	LRP lrp;

	mov	dx,-1
	mov	si,[ihdt]

;	StartProfile();
;	if (phdt->hpllr == hNil)
;		{
;		StopProfile();
;		return;
;		}
	errnz	<cbHdtMin - 8>
	mov	cl,3
	shl	si,cl
	add	si,dataoffset [vrghdt]
	errnz	<hNil>
	xor	ax,ax
	cmp	[si.hpllrHdt],ax
	je	Ltemp029

;	if (ihdt >= ihdtMaxSep)
;		{
	mov	di,[plbs]
	mov	cx,[yl]
	errnz	<fFalse>
	cmp	[ihdt],ihdtMaxSep
	jl	Ltemp030

;		if (phdt->dyl == 0)
;			{
;			StopProfile();
;			return;
;			}
	cmp	[si.dylHdt],ax
	je	Ltemp029

;		if (plbsAbs == 0 || !plbsAbs->fAbsPresent)
;			xl = plbs->xl;
	mov	dx,[di.xlLbs]
	mov	bx,[plbsAbs]
	or	bx,bx
	je	CHL02
	cmp	[bx.fAbsPresentLbs],al
	je	CHL02

;		else
;			{
;			/* get correct xl, yl by requesting an lr */
;			PushLbs(plbsAbs, &lbsT);
	push	bx
	lea	bx,[lbsT]
	push	bx
ifdef DEBUG
	cCall	S_PushLbs,<>
else ;!DEBUG
	push	cs
	call	near ptr N_PushLbs
endif ;!DEBUG

;			lbsT.yl = lbsT.ylColumn = yl;
;			lbsT.doc = docNew;
;			lbsT.ilrFirstChain = ilrNil;
;			lbsT.cp = cp0;
;			lbsT.cl = lbsT.dylOverlap = 0;
	mov	[lbsT.ilrFirstChainLbs],ilrNil
	push	di	;save plbs
	push	ds
	pop	es
	lea	di,[lbsT.docLbs]
	mov	ax,docNew
	stosw
	xor	ax,ax
	mov	[lbsT.dylOverlapLbs],ax
	cwd
	errnz	<(cpLbs) - (docLbs) - 2>
	stosw
	stosw
	errnz	<(clLbs) - (cpLbs) - 4>
	stosw
	errnz	<(ylLbs) - (clLbs) - 2>
	mov	ax,[yl]
	stosw
	pop	di	;restore plbs
	mov	[lbsT.ylColumnLbs],ax

;			FAssignLr(&lbsT, phdt->dyl, fFalse);
	lea	ax,[lbsT]
	push	ax
	push	[si.dylHdt]
	push	dx
ifdef DEBUG
	cCall	S_FAssignLr,<>
else ;!DEBUG
	push	cs
	call	near ptr N_FAssignLr
endif ;!DEBUG

;			xl = plbs->xl;
	push	si	;save phdt
	push	di	;save plbs
	lea	ax,[lbsT]
	push	ax	    ;argument for PopLbs
	xor	bx,bx
	push	bx	    ;argument for PopLbs
	mov	si,[di.xlLbs]
	mov	di,[yl]

;			if (lbsT.ilrCur != iNil)
;				{
	cmp	[lbsT.ilrCurLbs],iNil
	je	CHL01

;				lrp = LrpInPl(lbsT.hpllr, lbsT.ilrCur);
	push	si	;save xl
	xchg	ax,si
	;LN_LrpInPlCur performs LrpInPl(plbs->hpllr, plbs->ilrCur) with
	;the result in es:bx or es:ax.	It assumes plbs is passed in si.
	;ax, bx, cx, dx are altered.
	call	LN_LrpInPlCur
	pop	si	;restore xl

;				if (lrp->dyl != 0)
;					{
;					xl = lrp->xl;
;					yl = lrp->yl;
;					}
;				}
	cmp	es:[bx.dylLr],0
	je	CHL005
	mov	si,es:[bx.xlLr]
	mov	di,es:[bx.ylLr]
CHL005:

;			PopLbs(&lbsT, 0);
;			}
CHL01:
ifdef DEBUG
	cCall	S_PopLbs,<>
else ;!DEBUG
	push	cs
	call	near ptr N_PopLbs
endif ;!DEBUG
	mov	dx,si
	mov	cx,di
	pop	di	;restore plbs
	pop	si	;restore phdt

;		plbs->dylUsedCol += phdt->dyl;
CHL02:
	mov	ax,[si.dylHdt]
	add	[di.dylUsedColLbs],ax

;		plbs->dyllTotal = phdt->dyl + plbs->dyllTotal;
;	Assembler note: Assert(plbs->dyl >= 0) so we don't have to sign extend.
ifdef DEBUG
	call	CHL05
endif ;DEBUG
	add	[di.LO_dyllTotalLbs],ax
	adc	[di.HI_dyllTotalLbs],0

;		plbs->clTotal++; /* assume 1 line; it's indivisible */
;		}
	inc	[di.clTotalLbs]

;	CopyLrs(phdt->hpllr, plbs, (*phdt->hpllr)->ilrMac, yl, fFalse, xl);
CHL03:
	mov	bx,[si.hpllrHdt]
	push	bx
	push	di
	mov	bx,[bx]
	push	[bx.iMacPl]
	push	cx
	errnz	<fFalse>
	xor	ax,ax
	push	ax
	push	dx
ifdef DEBUG
	cCall	S_CopyLrs,<>
else ;!DEBUG
	push	cs
	call	near ptr N_CopyLrs
endif ;!DEBUG

CHL04:
;	StopProfile();
;}
cEnd


;	Assert(plbs->dyl >= 0);
ifdef DEBUG
CHL05:
	or	ax,ax
	jge	CHL06
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midLayout2n
	mov	bx,1053
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
CHL06:
	ret
endif ;DEBUG


;-------------------------------------------------------------------------
;	CopyLrs(hpllrFrom, plbsTo, ilrMac, yl, fCopyIgnore, xl)
;-------------------------------------------------------------------------
;/* C o p y  L r s */
;NATIVE CopyLrs(hpllrFrom, plbsTo, ilrMac, yl, fCopyIgnore, xl)
;struct PLLR **hpllrFrom;
;struct LBS *plbsTo;
;int ilrMac;	 /* number to copy */
;int yl; /* yl for top of LRs */
;int fCopyIgnore;
;int xl;	 /* use -1 to maintain xl */
;{
;/* copy LRs from one LR plex to another, forcing them to live at yl height */
;	int ilrFrom, ilrTo, fFirst;
;	int ylFirst;
;	struct LR lrT;

; %%Function:N_CopyLrs %%Owner:BRADV
cProc	N_CopyLrs,<PUBLIC,FAR>,<si,di>
	ParmW	<hpllrFrom>
	ParmW	<plbsTo>
	ParmW	<ilrMac>
	ParmW	<yl>
	ParmW	<fCopyIgnore>
	ParmW	<xl>

	LocalW	fNotFirst
	LocalW	ilrFrom
	LocalW	fFirst
	LocalW	ilrTo
	LocalV	lrT, cbLrMin

cBegin

;	StartProfile();
;	if (ilrMac <= 0)
;		{
;		StopProfile();
;		return;
;		}
	xor	ax,ax
	cmp	[ilrMac],ax
	jle	Ltemp011

;	fFirst = fTrue;
	dec	ax
	mov	bptr ([fFirst]),al
	xor	di,di	    ;initialize ylFirst

;	for (ilrFrom = 0, ilrTo = IMacPllr(plbsTo); ilrFrom < ilrMac; ilrFrom++)
;		{
	mov	[ilrFrom],ax
	mov	si,[plbsTo]
	;LN_IMacPllr performs cx = IMacPllr(plbs).  plbs is assumed to be
	;passed in si.	Only bx and cx are altered.
	call	LN_IMacPllr
	mov	[ilrTo],cx
CL01:
	inc	[ilrFrom]
	mov	ax,[ilrFrom]
	cmp	ax,[ilrMac]
	jge	Ltemp011

;		/* have to copy: heap moves */
;		bltLrp(LrpInPl(hpllrFrom, ilrFrom), &lrT, sizeof(struct LR));
	cCall	HpInPl,<[hpllrFrom],ax>
ifdef DEBUG
	;Check es with a call so as not to mess up short jumps
	call	CL11
endif ;/* DEBUG */
	push	di	;save ylFirst
	push	es
	pop	ds
	push	ss
	pop	es
	errnz	<cbLrMin AND 1>
	mov	cx,cbLrMin SHR 1
	xchg	ax,si
	lea	di,[lrT]
	rep	movsw
	xchg	ax,si
	push	ss
	pop	ds
	pop	di	;restore ylFirst

;		if (lrT.lrs == lrsIgnore && !fCopyIgnore)
;			continue;
	cmp	[lrT.lrsLr],lrsIgnore
	jne	CL02
	mov	cx,[fCopyIgnore]
	jcxz	CL01
CL02:

;		if (lrT.lrk == lrkAbs)
;			{
	mov	ax,[yl]
	cmp	[lrT.lrkLr],lrkAbs
	jne	CL04

;			plbsTo->fAbsPresent = fTrue;
	mov	[si.fAbsPresentLbs],fTrue

;			if (lrT.ihdt != ihdtNil && lrT.fInline && yl != ylLarge)
;				{
	cmp	[lrT.ihdtLr],ihdtNil
	je	CL09
	test	[lrT.fInlineLr],maskFInlineLr
	je	CL09
	cmp	ax,ylLarge
	je	CL09

;				/* need to position inline header APO */
;				lrT.yl += yl - ((fFirst) ? 0 : ylFirst);
	;Assembler note: di is not zero only if (!fFirst)
	sub	ax,di
	add	[lrT.ylLr],ax

;				}
;			}
	jmp	short CL09
Ltemp011:
	jmp	short CL10

;		else
;			{
CL04:

;			if (yl != ylLarge)
;				{
	cmp	ax,ylLarge
	je	CL06

;				/* displace to required yl */
;				if (fFirst)
;					{
;					ylFirst = lrT.yl;
;					fFirst = fFalse;
;					}
	xor	cx,cx
	xchg	bptr ([fFirst]),cl
	jcxz	CL05
	mov	di,[lrT.ylLr]
CL05:

;				lrT.yl += yl - ylFirst;

	sub	ax,di
	add	[lrT.ylLr],ax

;				plbsTo->yl = lrT.yl + lrT.dyl;
	mov	ax,[lrT.ylLr]
	add	ax,[lrT.dylLr]
	mov	[si.ylLbs],ax

;				}
CL06:

;			if (xl >= 0)
;				lrT.xl = xl;
	mov	ax,[xl]
	or	ax,ax
	jl	CL07
	mov	[lrT.xlLr],ax
CL07:

;			}
CL09:

;		ReplaceInPllr(plbsTo->hpllr, ilrTo++, &lrT);
	lea	ax,[lrT]
	push	[si.hpllrLbs]
	push	[ilrTo]
	push	ax
ifdef DEBUG
	cCall	S_ReplaceInPllr,<>
else ;!DEBUG
	push	cs
	call	near ptr N_ReplaceInPllr
endif ;!DEBUG
	inc	[ilrTo]
	jmp	CL01

;		}
;	StopProfile();
CL10:

;}
cEnd


ifdef DEBUG
CL11:
	push	ax
	push	bx
	push	cx
	push	dx
	push	es	;save es from HpInPl
	mov	bx,dx
	shl	bx,1
	mov	ax,mpsbps[bx]
	mov	es,ax
	shr	ax,1
	jc	CL12
	;Assembler note: There is no way we should have to call ReloadSb here.
;	reload sb trashes ax, cx, and dx
;	cCall	ReloadSb,<>
	mov	ax,midLayout2n
	mov	bx,1042
	cCall	AssertProcForNative,<ax,bx>
CL12:
	pop	ax	;restore es from HpInPl
	mov	bx,es	;compare with es rederived from the SB of HpInPl
	cmp	ax,bx
	je	CL13
	mov	ax,midLayout2n
	mov	bx,1043
	cCall	AssertProcForNative,<ax,bx>
CL13:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */


;-------------------------------------------------------------------------
;	ReplaceInPllr(hpllr, ilr, plr)
;-------------------------------------------------------------------------
;/* R e p l a c e  I n	P l l r */
;NATIVE ReplaceInPllr(hpllr, ilr, plr)
;struct PLLR **hpllr;
;int ilr;
;struct LR *plr;
;{
;	struct PLLR *ppllr = *hpllr;

; %%Function:N_ReplaceinPllr %%Owner:BRADV
cProc	N_ReplaceinPllr,<PUBLIC,FAR>,<si,di>
	ParmW	<hpllr>
	ParmW	<ilr>
	ParmW	<plr>

cBegin

;	StartProfile();
;	if (ilr < ppllr->ilrMax)
;		{
;		ppllr->ilrMac = max(ilr + 1, ppllr->ilrMac);
;		bltLrp(plr, LrpInPl(hpllr, ilr), sizeof(struct LR));
;		}
	mov	di,[hpllr]
	mov	si,[plr]
	mov	ax,[ilr]
	mov	bx,[di]
	cmp	ax,[bx.iMaxPl]
	jge	RIP04
	push	di	;argument for HpInPl
	push	ax	;argument for HpInPl
	inc	ax
	cmp	[bx.iMacPl],ax
	jge	RIP01
	mov	[bx.iMacPl],ax
RIP01:
	cCall	HpInPl,<>
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	push	es	;save es from HpInPl
	mov	bx,dx
	shl	bx,1
	mov	ax,mpsbps[bx]
	mov	es,ax
	shr	ax,1
	jc	RIP02
	;Assembler note: There is no way we should have to call ReloadSb here.
;	reload sb trashes ax, cx, and dx
;	cCall	ReloadSb,<>
	mov	ax,midLayout2n
	mov	bx,1044
	cCall	AssertProcForNative,<ax,bx>
RIP02:
	pop	ax	;restore es from HpInPl
	mov	bx,es	;compare with es rederived from the SB of HpInPl
	cmp	ax,bx
	je	RIP03
	mov	ax,midLayout2n
	mov	bx,1045
	cCall	AssertProcForNative,<ax,bx>
RIP03:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */
	xchg	ax,di
	errnz	<cbLrMin AND 1>
	mov	cx,cbLrMin SHR 1
	rep	movsw
	jmp	short RIP06

;	else if (!FInsertInPl(hpllr, ilr, plr))
;		{
RIP04:
	cCall	FInsertInPl,<di, ax, si>
	or	ax,ax
	jne	RIP06

;		Assert(vfInFormatPage);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	[vfInFormatPage],fFalse
	jne	RIP05
	mov	ax,midLayout2n
	mov	bx,1014
	cCall	AssertProcForNative,<ax,bx>
RIP05:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

;		AbortLayout();
	cCall	AbortLayout,<>

;		}
;	StopProfile();
RIP06:

;}
cEnd


;-------------------------------------------------------------------------
;	CacheParaL(doc, cp, fOutline)
;-------------------------------------------------------------------------
;/* C a c h e  P a r a	L */
;NATIVE CacheParaL(doc, cp, fOutline)
;int doc;
;CP cp;
;int fOutline;
;{

; %%Function:N_CacheParaL %%Owner:BRADV
cProc	N_CacheParaL,<PUBLIC,FAR>,<>
	ParmW	doc
	ParmD	cp
	ParmW	fOutline

cBegin
	mov	bx,[doc]
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	mov	cx,[fOutline]
	call	LN_CacheParaL
cEnd

;End of CacheParaL


	;LN_CacheParaLbs takes plbs in si
	;and performs CacheParaL(plbs->doc, plbs->cp, plbs->fOutline).
	;ax, bx, cx, dx are altered.
; %%Function:LN_CacheParaLbs %%Owner:BRADV
PUBLIC LN_CacheParaLbs
LN_CacheParaLbs:
	mov	ax,[si.LO_cpLbs]
	mov	dx,[si.HI_cpLbs]

	;LN_CacheParaLbsCp takes plbs in si
	;and performs CacheParaL(plbs->doc, dx:ax, plbs->fOutline).
	;ax, bx, cx, dx are altered.
LN_CacheParaLbsCp:
	mov	cl,[si.fOutlineLbs]
	xor	ch,ch
	errnz	<(docLbs) - 0>
	mov	bx,[si.docLbs]
	;fall through

	;LN_CacheParaL takes doc in bx, cp in dx:ax, fOutline in cx
	;ax, bx, cx, dx are altered.
LN_CacheParaL:
	push	di
	push	si
;/* cache current paragraph and, if fOutline, kill all properties that should
;   not be expressed in outline mode */
;	StartProfile();
;	if (!FInCa(doc, cp, &caParaL) || caParaL.doc != caPara.doc ||
;		caPara.cpFirst < caParaL.cpFirst || caPara.cpLim > caParaL.cpLim)
;		{
	;***Begin in line FInCa
;	if (pca->doc == doc)
;		{
;		if (pca->cpFirst <= cp && cp < pca->cpLim)
;			return fTrue;
;		}
;	return fFalse;
	;***End in line FInCa
					;Refresh caParaL if
	push	cx	;save fOutline
	push	dx	;save HI_cp
	push	ax	;save LO_cp
	mov	di,bx	;save doc
	std
	xchg	ax,cx
	mov	si,dataoffset [caParaL.docCa]
	lodsw
	cmp	ax,di
	jne	CPL01			; (caParaL.doc != doc
	cmp	ax,[caPara.docCa]
	jne	CPL01			; || caParaL.doc != caPara.doc
	errnz	<(docCa) - (HI_cpLimCa) - 2>
	errnz	<(HI_cpLimCa) - (LO_cpLimCa) - 2>
	lodsw
	xchg	bx,ax
	lodsw		;caParaL.cpLim in bx:ax, cp in dx:cx
	cmp	cx,ax
	push	dx
	sbb	dx,bx
	pop	dx
	jge	CPL01			; || cp >= caParaL.cpLim
	sub	ax,[caPara.LO_cpLimCa]
	sbb	bx,[caPara.HI_cpLimCa]
	jl	CPL01			; ||caParaL.cpLim < caPara.cpLim
	errnz	<(LO_cpLimCa) - (HI_cpFirstCa) - 2>
	errnz	<(HI_cpFirstCa) - (LO_cpFirstCa) - 2>
	lodsw
	xchg	bx,ax
	lodsw		;caParaL.cpFirst in bx:ax, cp in dx:cx
	sub	cx,ax
	sbb	dx,bx
	jl	CPL01			; || cp < caParaL.cpFirst
	xchg	ax,cx
	mov	ax,[caPara.LO_cpFirstCa]
	sub	ax,cx
	mov	ax,[caPara.HI_cpFirstCa]
	sbb	ax,bx
	jl	CPL01			; || caPara.cpFirst < caParaL.cpFirst)
	db	0A8h	;turns next "stc" into "test al,immediate"
			;also clears the carry flag
CPL01:
	stc
	cld
	pop	bx	;restore LO_cp
	pop	dx	;restore HI_cp
	pop	cx	;restore fOutline
	jnc	CPL02

;		int ccp;

;		caParaL.doc = doc;
	mov	[caParaL.docCa],di

;		LinkDocToWw(doc, wwLayout, wwNil);
;		GetCpFirstCpLimDisplayPara(wwLayout, doc, cp, &caParaL.cpFirst, &caParaL.cpLim);
	push	cx	;save fOutline
	push	dx	;save HI_cp
	push	bx	;save LO_cp
	mov	cx,wwLayout
	push	cx	;argument for GetCpFirstCpLimDisplayPara
	push	di	;argument for GetCpFirstCpLimDisplayPara
	push	dx	;argument for GetCpFirstCpLimDisplayPara
	push	bx	;argument for GetCpFirstCpLimDisplayPara
	mov	ax,dataoffset [caParaL.cpFirstCa]
	push	ax	;argument for GetCpFirstCpLimDisplayPara
	add	ax,(cpLimCa) - (cpFirstCa)
	push	ax	;argument for GetCpFirstCpLimDisplayPara
	errnz	<wwNil>
	xor	ax,ax
	cCall	LinkDocToWw,<di, cx, ax>
ifdef DEBUG
	cCall	S_GetCpFirstCpLimDisplayPara,<>
else
	cCall	N_GetCpFirstCpLimDisplayPara,<>
endif ;DEBUG

;	/* get correct para props in vpap */
;	FetchCpPccpVisible(doc, caParaL.cpFirst, &ccp, wwLayout /* fvc */, 0 /* ffe */);
;	/* now vcpFetch is first visible char after caParaL.cpFirst and
;	   caPara contains vcpFetch; make sure apo's or something didn't
;	   cause caParaL to stop short of a visible character */
;	if (vcpFetch >= caParaL.cpLim)
;		CachePara(doc, caParaL.cpFirst);
	push	ax	;make room for ccp
	mov	bx,sp
	;CPL03 pushes di and caParaL.cpFirst.  Only ax is altered.
	call	CPL03
	push	bx
	mov	ax,wwLayout
	push	ax
	xor	ax,ax
	push	ax
ifdef DEBUG
	cCall	S_FetchCpPccpVisible,<>
else
	cCall	N_FetchCpPccpVisible,<>
endif ;DEBUG
	pop	ax	;remove ccp from stack
	mov	ax,wlo [vcpFetch]
	sub	ax,[caParaL.LO_cpLimCa]
	mov	ax,whi [vcpFetch]
	sbb	ax,[caParaL.HI_cpLimCa]
	jl	CPL013
	;CPL03 pushes di and caParaL.cpFirst.  Only ax is altered.
	call	CPL03
ifdef DEBUG
	cCall	S_CachePara,<>
else ;not DEBUG
	cCall	N_CachePara,<>
endif ;DEBUG
CPL013:
	pop	bx	;restore LO_cp
	pop	dx	;restore HI_cp
	pop	cx	;restore fOutline

;		if (fOutline)
;			{
	jcxz	CPL02

;			vpapFetch.fSideBySide = vpapFetch.fKeep =
;				vpapFetch.fKeepFollow = vpapFetch.fPageBreakBefore = fFalse;
;			if (!vpapFetch.fInTable)
;				{
;				vpapFetch.brcp = brcpNone;
;				vpapFetch.dyaLine = vpapFetch.dyaBefore = vpapFetch.dyaAfter = 0;
;				}
;			vpapFetch.dxaWidth = vpapFetch.dxaAbs = vpapFetch.dyaAbs = 0;
;			vpapFetch.pc = 0;
;			}
	push	di	;save doc
	errnz	<fFalse>
	errnz	<brcpNone>
	xor	ax,ax
	push	ds
	pop	es
	mov	di,dataoffset [vpapFetch.fSideBySidePap]
	errnz	<(fKeepPap) - (fSideBySidePap) - 1>
	errnz	<(fKeepFollowPap) - (fKeepPap) - 1>
	errnz	<(fPageBreakBeforePap) - (fKeepFollowPap) - 1>
	stosw
	stosw
	cmp	[vpapFetch.fInTablePap],al
	jne	CPL015
	errnz	<(brcpPap) - (fPageBreakBeforePap) - 2>
	inc	di
	stosb
	mov	di,dataoffset [vpapFetch.dyaLinePap]
	errnz	<(dyaBeforePap) - (dyaLinePap) - 2>
	errnz	<(dyaAfterPap) - (dyaBeforePap) - 2>
	stosw
	stosw
	stosw
CPL015:
	mov	di,dataoffset [vpapFetch.dxaAbsPap]
	errnz	<(dyaAbsPap) - (dxaAbsPap) - 2>
	errnz	<(dxaWidthPap) - (dyaAbsPap) - 2>
	stosw
	stosw
	stosw
	and	[vpapFetch.pcPap],NOT maskPcPap
	pop	di	;restore doc
CPL02:

;		}

	pop	si
	pop	di
;}
	ret

	;CPL03 pushes di and caParaL.cpFirst.  Only ax is altered.
CPL03:
	pop	ax
	push	di
	push	[caParaL.HI_cpFirstCa]
	push	[caParaL.LO_cpFirstCa]
	jmp	ax


;-------------------------------------------------------------------------
;	CacheSectL(doc, cp, fOutline)
;-------------------------------------------------------------------------
;/* C a c h e  S e c t	L */
;NATIVE CacheSectL(doc, cp, fOutline)
;int doc;
;CP cp;
;int fOutline;
;{

; %%Function:N_CacheSectL %%Owner:BRADV
cProc	N_CacheSectL,<PUBLIC,FAR>,<>
	ParmW	doc
	ParmD	cp
	ParmW	fOutline

cBegin
	mov	bx,[doc]
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	mov	cx,[fOutline]
	call	LN_CacheSectL
cEnd

;End of CacheSectL


	;LN_CacheSectL takes doc in bx, cp in dx:ax, fOutline in cx
	;ax, bx, cx, dx are altered.
; %%Function:LN_CacheSectL %%Owner:BRADV
PUBLIC LN_CacheSectL
LN_CacheSectL:
;/* cache current section and, if fOutline, kill all properties that should
;   not be expressed in outline mode */
;	 struct DOP *pdop;

;/* MacWord 1.05 conversions and PCWord docs can have last
;	character of doc == chSect, so back off */
;	StartProfile();
;	cp = CpMin(cp, CpMacDocEditL(doc, cp));
;#define CpMacDocEditL(doc, cp)  CpMacDocEdit(doc)
	push	cx	;save fOutline
	push	bx	;save doc
	;***Begin in line CpMacDocEdit
;	return(CpMacDoc(doc) - ccpEop);
	;Takes doc in bx, result in bx.  Only bx is altered.
	call	LN_PdodDocL
	mov	cx,[bx.HI_cpMacDod]
	mov	bx,[bx.LO_cpMacDod]
	sub	bx,3*ccpEop
	sbb	cx,0
	;***End in line CpMacDocEdit
	cmp	ax,bx
	push	dx
	sbb	dx,cx
	pop	dx
	jl	CSL01
	mov	ax,bx
	mov	dx,cx
CSL01:
	pop	bx	;restore doc
	pop	cx	;restore fOutline

;	if (!FInCa(doc, cp, &caSect))
	;***Begin in line FInCa
;	if (pca->doc == doc)
;		{
;		if (pca->cpFirst <= cp && cp < pca->cpLim)
;			return fTrue;
;		}
;	return fFalse;
	cmp	[caSect.docCa],bx
	jne	CSL02
	cmp	ax,[caSect.LO_cpFirstCa]
	push	dx
	sbb	dx,[caSect.HI_cpFirstCa]
	pop	dx
	jl	CSL02
	cmp	ax,[caSect.LO_cpLimCa]
	push	dx
	sbb	dx,[caSect.HI_cpLimCa]
	pop	dx
	jl	CSL03
	;***End in line FInCa
CSL02:

;		{
;		CacheSectProc(doc, cp);
	push	cx	;save fOutline
	push	bx	;save doc
	cCall	CacheSectProc,<bx, dx, ax>
	pop	bx	;restore doc
	pop	cx	;restore fOutline

;		if (fOutline)
;			{
	jcxz	CSL03

;			vsepFetch.bkc = bkcNewPage;
	mov	[vsepFetch.bkcSep],bkcNewPage

;			vsepFetch.ccolM1 = vsepFetch.nLnnMod =
;				 vsepFetch.vjc = 0;
	xor	ax,ax
	mov	[vsepFetch.ccolM1Sep],ax
	mov	[vsepFetch.nLnnModSep],ax
	mov	[vsepFetch.vjcSep],al

;			pdop = &PdodMother(doc)->dop;
	cCall	N_PdodMother,<bx>
	xchg	ax,bx

;			vsepFetch.dxaColumnWidth = pdop->xaPage
;				- pdop->dxaLeft - pdop->dxaRight
;				- pdop->dxaGutter;
	mov	ax,[bx.dopDod.xaPageDop]
	sub	ax,[bx.dopDod.dxaLeftDop]
	sub	ax,[bx.dopDod.dxaRightDop]
	sub	ax,[bx.dopDod.dxaGutterDop]
	mov	[vsepFetch.dxaColumnWidthSep],ax

;			}
;		}
CSL03:

;}
	ret


;-------------------------------------------------------------------------
;	PushLbs(plbsFrom, plbsTo)
;-------------------------------------------------------------------------
;/* P u s h   L b s */
;NATIVE PushLbs(plbsFrom, plbsTo)
;struct LBS *plbsFrom, *plbsTo;
;{
;/* saves layout state by copying lbs and its hpllr; the lbs is copied to
;   the target and to the lbs stack for memory cleanup; pop or copy will free
;   the stack entry by comparing hpllr */
;	int ilbs = (*vhpllbs)->ilbsMac;
;	int ilrMac;
;	struct LBS lbsT;

; %%Function:N_PushLbs %%Owner:BRADV
cProc	N_PushLbs,<PUBLIC,FAR>,<si,di>
	ParmW	plbsFrom
	ParmW	plbsTo

	LocalW	ilrMac

cBegin

;	StartProfile();
;	Assert(vfInFormatPage);
;	Assert(plbsTo != 0 && plbsFrom != 0);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	[vfInFormatPage],fFalse
	jne	PuL01
	mov	ax,midLayout2n
	mov	bx,1015
	cCall	AssertProcForNative,<ax,bx>
PuL01:
	cmp	[plbsFrom],0
	jne	PuL02
	mov	ax,midLayout2n
	mov	bx,1016
	cCall	AssertProcForNative,<ax,bx>
PuL02:
	cmp	[plbsTo],0
	jne	PuL03
	mov	ax,midLayout2n
	mov	bx,1017
	cCall	AssertProcForNative,<ax,bx>
PuL03:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

;	ilrMac = IMacPllr(plbsFrom);
	mov	si,[plbsFrom]
	;LN_IMacPllr performs cx = IMacPllr(plbs).  plbs is assumed to be
	;passed in si.	Only bx and cx are altered.
	call	LN_IMacPllr
	mov	[ilrMac],cx

;	*plbsTo = *plbsFrom;
	mov	di,[plbsTo]
	push	di	;save plbsTo
	push	ds
	pop	es
	errnz	<cbLbsMin AND 1>
	mov	cx,cbLbsMin SHR 1
	rep	movsw
	pop	si	;restore plbsTo

;	if ((plbsTo->hpllr = vhpllrSpare) != hNil)
;		{
	errnz	<hNil>
	xor	di,di
	xchg	di,[vhpllrSpare]
	mov	[si.hpllrLbs],di
	mov	cx,[ilrMac]
	errnz	<hNil>
	or	di,di
	je	PuL05

;		if ((*vhpllrSpare)->ilrMax < ilrMac)
;			{
	mov	bx,[di]
	cmp	[bx.iMaxPl],cx
	jge	PuL07

;			struct PL *ppl = *vhpllrSpare;
;			HQ hq = *((HQ *)(((char *)ppl) + ppl->brgfoo));
	add	bx,[bx.brgfooPl]
	push	[bx+2]
	push	[bx]
	mov	ax,sp

;			Assert(ppl->fExternal);
ifdef DEBUG
	call	PuL10
endif ;DEBUG

;			/* FChngSizePhqLcb may cause heap movement! */
;			if (!FChngSizePhqLcb(&hq, (long)((ilrMac + 1) * sizeof(struct LR))))
;				{
	push	ax
	mov	ax,cbLrMin
	inc	cx
	mul	cx
	push	dx
	push	ax
	cCall	FChngSizePhqLcb,<>
	or	ax,ax
	jne	PuL04

;				vhpllrSpare = 0;
;				goto LFreeAbort;
;				}
	add	sp,4
	;vhpllrSpare set to zero above
	jmp	short LFreeAbort
PuL04:

;			ppl = *vhpllrSpare;
	mov	bx,[di]

;			*((HQ *)(((char *)ppl) + ppl->brgfoo)) = hq;
;			(*vhpllrSpare)->ilrMax = ilrMac + 1;
	mov	cx,[ilrMac]
	inc	cx
	mov	[bx.iMaxPl],cx
	add	bx,[bx.brgfooPl]
	pop	[bx]
	pop	[bx+2]
	jmp	short PuL07

;			}
;		vhpllrSpare = hNil;
	;Assembler note: vhpllrSpare is set to hNil above

;		}
;	else if ((plbsTo->hpllr = HpllrInit(sizeof(struct LR), max(3, ilrMac))) == hNil)
;		{
;#define HpllrInit(cb, ilrMac)	 HplInit2(cb, cbPLBase, ilrMac, fTrue /* fExternal */)
PuL05:
	cmp	cx,3
	jge	PuL06
	mov	cx,3
PuL06:
	mov	ax,cbLrMin
	mov	bx,cbPLBase
	cCall	HplInit2,<ax, bx, cx, bx>
	mov	[si.hpllrLbs],ax
	errnz	<hNil>
	or	ax,ax
	je	PuL075

;		AbortLayout();
;		}
PuL07:

;	if (!FInsertInPl(vhpllbs, ilbs, plbsTo))
;		{

	;Assembler note: The following line is done upon entry in the C.
;	int ilbs = (*vhpllbs)->ilbsMac;
	mov	bx,[vhpllbs]
	push	bx
	mov	bx,[bx]
	push	[bx.iMacPl]
	push	si
	cCall	FInsertInPl,<>
	or	ax,ax
	jne	PuL08

;LFreeAbort:
;		FreePhpl(&plbsTo->hpllr);
;		AbortLayout();
;		}
LFreeAbort:
	cCall	FreeHpl,<[si.hpllrLbs]>
	mov	[si.hpllrLbs],hNil
PuL075:
	cCall	AbortLayout,<>
PuL08:

;	plbsTo->fOnLbsStack = fTrue;
	mov	[si.fOnLbsStackLbs],fTrue

;	SetIMacPllr(plbsTo, ilrMac);
;#define SetIMacPllr(plbs, i)	 ((*(plbs)->hpllr)->ilrMac = i)
	mov	bx,[si.hpllrLbs]
	mov	bx,[bx]
	mov	ax,[ilrMac]
	mov	[bx.iMacPl],ax

;	if (ilrMac > 0)
;		{
	or	ax,ax
	jle	PuL09

;		bltLrp(LrpInPl(plbsFrom->hpllr, 0), LrpInPl(plbsTo->hpllr, 0),
;			ilrMac * sizeof(struct LR));
	errnz	<cbLrMin AND 1>
	mov	dx,cbLrMin SHR 1
	mul	dx
	push	ax
;	It is possible that the call later on to ReloadSb for hpllrTo
;	would force out the sb for hpllrFrom.  This would happen if now
;	sbFrom was oldest on the LRU list but still swapped in, and sbTo
;	was swapped out.  In that event ReloadSb would not be called for
;	sbFrom, the LRU entry would not get updated, and the call to ReloadSb
;	for sbTo would swap out sbFrom.
;	This call to LN_LrpInPl makes that state impossible and so works
;	around the LMEM quirk.
	xor	cx,cx
	mov	di,[plbsFrom]
	;LN_LrpInPl performs LrpInPl(plbs->hpllr, cx) with the result in es:bx
	;or es:ax.  It assumes plbs is passed in si.
	;ax, bx, dx are altered.
	call	LN_LrpInPl
	xchg	si,di
	;LN_LrpInPl performs LrpInPl(plbs->hpllr, cx) with the result in es:bx
	;or es:ax.  It assumes plbs is passed in si.
	;ax, bx, dx are altered.
	call	LN_LrpInPl
	push	es
	push	ax
	xchg	si,di
	;LN_LrpInPl performs LrpInPl(plbs->hpllr, cx) with the result in es:bx
	;or es:ax.  It assumes plbs is passed in si.
	;ax, bx, dx are altered.
	call	LN_LrpInPl
	xchg	ax,di
	pop	si
	pop	ds
	pop	cx
	rep	movsw
	push	ss
	pop	ds

;		}
PuL09:

;	StopProfile();
;}
cEnd

;			Assert(ppl->fExternal);
ifdef DEBUG
PuL10:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	bx,[di]
	cmp	[bx.fExternalPl],fFalse
	jne	PuL11
	mov	ax,midLayout2n
	mov	bx,1018
	cCall	AssertProcForNative,<ax,bx>
PuL11:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG


;-------------------------------------------------------------------------
;	PopLbs(plbsId, plbsTo)
;-------------------------------------------------------------------------
;/* P o p  L b s */
;NATIVE int PopLbs(plbsId, plbsTo)
;struct LBS *plbsId, *plbsTo;
;{
;/* removes the entries above the one whose hpllr matches plbsId's; copies
;   the matching stack entry to plbsTo if not zero; pass zero for plbsId to
;   free all of lbs stack */
;	int ilbsTop;
;	struct LBS *plbsTop;
;	struct LBS lbsT;

; %%Function:N_PopLbs %%Owner:BRADV
cProc	N_PopLbs,<PUBLIC,FAR>,<si,di>
	ParmW	plbsId
	ParmW	plbsTo

	LocalW	ilrMac
	LocalV	lbsT, cbLbsMin

cBegin

;	StartProfile();
;	if (vhpllbs == hNil)
;		{
;		Assert(plbsId == 0 && plbsTo == 0);
;		StopProfile();
;		return;
;		}
	mov	si,[vhpllbs]
	errnz	<hNil>
	or	si,si
ifdef DEBUG
	jne	PoL03
	cmp	[plbsId],0
	je	PoL01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midLayout2n
	mov	bx,1019
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
PoL01:
	cmp	[plbsTo],0
	je	PoL02
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midLayout2n
	mov	bx,1020
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
PoL02:
	jmp	short PoL12
PoL03:
else ;!DEBUG
	je	PoL12
endif ;!DEBUG

;	if (plbsId == 0)
;		(plbsId = &lbsT)->hpllr = hNil;
	xor	ax,ax
	cmp	[plbsId],ax
	jne	PoL04
	lea	bx,[lbsT]
	mov	[plbsId],bx
	errnz	<hNil>
	mov	[lbsT.hpllrLbs],ax
PoL04:

;/* free hpllr from target lbs */
;	if (plbsTo != 0)
;		CopyLbs(0, plbsTo);
	cmp	[plbsTo],ax
	je	PoL05
	push	ax
	push	[plbsTo]
ifdef DEBUG
	cCall	S_CopyLbs,<>
else ;not DEBUG
	push	cs
	call	near ptr N_CopyLbs
endif ;DEBUG
PoL05:

;/* pop all higher entries in vhpllbs */
;	ilbsTop = (*vhpllbs)->ilbsMac - 1;
	mov	bx,[si]
	mov	di,[bx.iMacPl]
	dec	di

;	if (ilbsTop >= 0)
	jl	PoL08

;		for (plbsTop = PInPl(vhpllbs, ilbsTop);
;			ilbsTop >= 0 && plbsTop->hpllr != plbsId->hpllr;
;			--ilbsTop, --plbsTop)
;			{
	cCall	PInPl,<si, di>
	xchg	ax,si
	inc	di
	jmp	short PoL07
PoL06:
;			if (vhpllrSpare == hNil)
;				vhpllrSpare = plbsTop->hpllr;
;			else
;				FreePhpl(&plbsTop->hpllr);
	;LN_UseHpllrSpare performs:
	;if (vhpllrSpare == hNil)
	;	 vhpllrSpare = plbs->hpllr;
	;else
	;	 FreePhpl(&plbs->hpllr);
	;Assuming si contains plbs.  ax, bx, cx, dx are altered.
	call	LN_UseHpllrSpare
	sub	si,cbLbsMin
PoL07:
	dec	di
	jl	PoL08
	mov	bx,[plbsId]
	mov	ax,[si.hpllrLbs]
	cmp	ax,[bx.hpllrLbs]

	jne	PoL06

;			}
PoL08:

;/* pop the entry */
;	if (ilbsTop < 0)
;		ilbsTop = 0;
	or	di,di
	jl	PoL10

;	else if (plbsTo != 0)
;		blt(plbsTop, plbsTo, cwLBS);
	mov	ax,[plbsTo]
	or	ax,ax
	je	PoL09
	xchg	ax,di
	push	ds
	pop	es
	errnz	<cbLbsMin AND 1>
	mov	cx,cbLbsMin SHR 1
	rep	movsw
	xchg	ax,di
	jmp	short PoL11

PoL09:
;	else if (vhpllrSpare == hNil)
;		vhpllrSpare = plbsTop->hpllr;
;	else
;		FreePhpl(&plbsTop->hpllr);
	;LN_UseHpllrSpare performs:
	;if (vhpllrSpare == hNil)
	;	 vhpllrSpare = plbs->hpllr;
	;else
	;	 FreePhpl(&plbs->hpllr);
	;Assuming si contains plbs.  ax, bx, cx, dx are altered.
	call	LN_UseHpllrSpare
	db	03Dh	;turns next "xor di,di" into "cmp ax,immediate"
PoL10:
	xor	di,di
PoL11:

;	(*vhpllbs)->ilbsMac = ilbsTop;
	mov	bx,[vhpllbs]
	mov	bx,[bx]
	mov	[bx.iMacPl],di

;	StopProfile();
PoL12:
;}
cEnd

	;LN_UseHpllrSpare performs:
	;if (vhpllrSpare == hNil)
	;	 vhpllrSpare = plbs->hpllr;
	;else
	;	 FreePhpl(&plbs->hpllr);
	;Assuming si contains plbs.  ax, bx, cx, dx are altered.
LN_UseHpllrSpare:
	mov	ax,[si.hpllrLbs]
	cmp	[vhpllrSpare],hNil
	jne	UHS14
	mov	[vhpllrSpare],ax
	jmp	short UHS15
UHS14:
	cCall	FreeHpl,<ax>
	mov	[si.hpllrLbs],hNil
UHS15:
	ret


;-------------------------------------------------------------------------
;	CopyLbs(plbsFrom, plbsTo)
;-------------------------------------------------------------------------
;/* C o p y  L b s */
;NATIVE  CopyLbs(plbsFrom, plbsTo)
;struct LBS *plbsFrom, *plbsTo;
;{
;/* copies lbsFrom to lbsTo; lbsTo's original hpllr is freed. If an lbs stack
;   entry has the same hpllr, that stack entry is deleted */

; %%Function:N_CopyLbs %%Owner:BRADV
cProc	N_CopyLbs,<PUBLIC,FAR>,<si,di>
	ParmW	plbsFrom
	ParmW	plbsTo

	LocalW	ilrMac
	LocalV	lbsT, cbLbsMin

cBegin

;	StartProfile();
	mov	si,[plbsTo]

;	Assert(plbsTo != 0);
ifdef DEBUG
	or	si,si
	jne	CoL01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midLayout2n
	mov	bx,1021
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
CoL01:
endif ;DEBUG

;	UnstackLbs(plbsTo);
	;LN_UnstackLbs takes plbs in si.  ax, bx, cx, dx are altered.
	call	LN_UnstackLbs

;	if (vhpllrSpare == hNil)
;		vhpllrSpare = plbsTo->hpllr;
;	else
;		FreePhpl(&plbsTo->hpllr);
	;LN_UseHpllrSpare performs:
	;if (vhpllrSpare == hNil)
	;	 vhpllrSpare = plbs->hpllr;
	;else
	;	 FreePhpl(&plbs->hpllr);
	;Assuming si contains plbs.  ax, bx, cx, dx are altered.
	call	LN_UseHpllrSpare

;	if (plbsFrom == 0)
;		plbsTo->hpllr = 0;
;	else
;		{
;		*plbsTo = *plbsFrom;
;		plbsFrom->hpllr = 0;
;		UnstackLbs(plbsTo);
;		}
	mov	di,si
	mov	si,[plbsFrom]
	mov	[di.hpllrLbs],si
	or	si,si
	je	CoL02
	push	di	;argument for UnstackLbs
	push	ds
	pop	es
	errnz	<cbLbsMin AND 1>
	mov	cx,cbLbsMin SHR 1
	rep	movsw
	mov	[si.hpllrLbs - cbLbsMin],0
	pop	si
	;LN_UnstackLbs takes plbs in si.  ax, bx, cx, dx are altered.
	call	LN_UnstackLbs
CoL02:

;	StopProfile();
;}
cEnd


;-------------------------------------------------------------------------
;	UnstackLbs(plbs)
;-------------------------------------------------------------------------
;/* U n s t a c k   L b s */
;NATIVE UnstackLbs(plbs)
;struct LBS *plbs;
;{
;/* removes from the lbs stack the first entry whose hpllr matches plbs's */
;	int ilbs;
;	struct LBS *plbsStack;

ifdef DEBUG
; %%Function:LN_UnstackLbs %%Owner:BRADV
PUBLIC LN_UnstackLbs
endif ;DEBUG
	;LN_UnstackLbs takes plbs in si.  ax, bx, cx, dx are altered.
LN_UnstackLbs:

;	StartProfile();
;	if (plbs == 0 || !plbs->fOnLbsStack || (ilbs = (*vhpllbs)->ilbsMac - 1) < 0)
;		{
;		StopProfile();
;		return;
;		}
	push	di	;save caller's di
	or	si,si
	je	UL03
	cmp	[si.fOnLbsStackLbs],fFalse
	je	UL03
	mov	di,[vhpllbs]
	mov	bx,[di]
	mov	ax,[bx.iMacPl]
	dec	ax
	jl	UL03

;	plbs->fOnLbsStack = fFalse;
	mov	[si.fOnLbsStackLbs],fFalse

;	for (plbsStack = PInPl(vhpllbs, ilbs); ilbs >= 0; --plbsStack, --ilbs)
	push	ax	;save ilbs
	cCall	PInPl,<di, ax>
	pop	cx	;restore ilbs
	xchg	ax,bx
	inc	cx
	jmp	short UL02
UL01:
	sub	bx,cbLbsMin
UL02:
	dec	cx
	jl	UL03

;		if (plbsStack->hpllr == plbs->hpllr)
;			{
	mov	ax,[bx.hpllrLbs]
	cmp	ax,[si.hpllrLbs]
	jne	UL01

;			DeleteFromPl(vhpllbs, ilbs);
;			StopProfile();
;			return;
;			}
	cCall	DeleteFromPl,<di,cx>

;	StopProfile();
UL03:
	pop	di	;restore caller's di
;}
	ret

;-------------------------------------------------------------------------
;	FAbortLayout(fOutline, plbs)
;-------------------------------------------------------------------------
;/* F	A b o r t   L a y o u t */
;NATIVE FAbortLayout(fOutline, plbs)
;int fOutline;
;struct LBS *plbs;
;{
;	BOOL fAbort, fLayoutSav;
;	struct CA caSave;
;	int flmSave = vflm;
;	int lmSave = vlm;
;#ifdef DEBUG
;	CP cpFirstSave = vcpFirstLayout;
;	CP cpLimSave = vcpLimLayout;
;#endif

; %%Function:N_FAbortLayout %%Owner:BRADV
cProc	N_FAbortLayout,<PUBLIC,FAR>,<si,di>
	ParmW	fOutline
	ParmW	plbs

	;LocalW	fOutlineWw
	LocalB  fLayoutSav
	LocalV	caSave, cbCaMin
ifdef DEBUG
	LocalD	cpFirstSave
	LocalD	cpLimSave
endif ;DEBUG

cBegin

ifdef DEBUG
	push	wlo [vcpFirstLayout]
	pop	[OFF_cpFirstSave]
	push	whi [vcpFirstLayout]
	pop	[SEG_cpFirstSave]
	push	wlo [vcpLimLayout]
	pop	[OFF_cpLimSave]
	push	whi [vcpLimLayout]
	pop	[SEG_cpLimSave]
endif ;DEBUG

;	caSave = caPara;
	push	ds
	pop	es
	mov	si,dataoffset [caPara]
	lea	di,[caSave]
	mov	cx,cbCaMin/2
	rep	movsw

;	fOutlineWw = PwwdWw(wwLayout)->fOutline
	mov	bx,[mpwwhwwd+(wwLayout SHL 1)]
	mov	bx,[bx]
	push	wptr ([bx.fOutlineWwd])

; /* So that any display routines called will work */
;	fLayoutSav = vfli.fLayout;
;	vfli.fLayout = fFalse;
; PAUSE
	mov	al,[vfli.fLayoutFli]
	mov	bptr [fLayoutSav],al
	sub	[vfli.fLayoutFli],al

	mov	di,[vflm]
	mov	ax,[vlm]
	push	ax	       ;lmSave on stack

;	fAbort = (vlm == lmPagevw) ? fFalse : (vlm == lmBRepag) ?
;		FMsgPresent(mtyIdle) : FQueryAbortCheck();
;#define FQueryAbortCheck() (vpisPrompt==pisNormal?fFalse:FQueryAbortCheckProc())
	xor	si,si
	cmp	al,lmPagevw
	je	FAL023
	cmp	al,lmBRepag
	je	FAL01
	errnz	<pisNormal - 0>
	mov	cx,[vpisPrompt]
	jcxz	FAL023
	cCall	FQueryAbortCheckProc,<>
	jmp	short FAL02
FAL01:
	mov	ax,mtyIdle
	cCall	FMsgPresent,<ax>
FAL02:
	xchg	ax,si
FAL023:
;	vfli.fLayout = fLayoutSav;
	mov	al,bptr [fLayoutSav]
	mov	[vfli.fLayoutFli],al

;	fAbort = fAbort || (vflm != flmSave && !FResetWwLayout(DocMotherLayout(plbs->doc), flmSave, lmSave));
	pop	ax	;restore lmSave
	or	si,si
	jne	FAL025
	errnz	<fFalse>
	xor	si,si
	cmp	[vflm],di
	je	FAL025
	push	ax	;save lmSave
	mov	bx,[plbs]
	cCall	DocMotherLayout,<[bx.docLbs]>
	pop	cx	;restore lmSave
	cCall	FResetWwLayout,<ax, di, cx>
	or	ax,ax
	jne	FAL025
	errnz	<fTrue - fFalse - 1>
	inc	si
FAL025:

;	PwwdWw(wwLayout)->fOutline = fOutlineWw
	mov	bx,[mpwwhwwd+(wwLayout SHL 1)]
	mov	bx,[bx]
	pop	ax
	and	al,maskfOutlineWwd
	and	[bx.fOutlineWwd],NOT maskfOutlineWwd
	or	[bx.fOutlineWwd],al

;/* make sure that processing some of the messages should not change vcpFirstLayout */
;	Assert(vcpFirstLayout == cpFirstSave);
;	Assert(vcpLimLayout == cpLimSave);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,wlo [vcpFirstLayout]
	mov	dx,whi [vcpFirstLayout]
	sub	ax,[OFF_cpFirstSave]
	sbb	dx,[SEG_cpFirstSave]
	or	ax,dx
	je	FAL04
	mov	ax,midLayout2n
	mov	bx,1023
	cCall	AssertProcForNative,<ax,bx>
FAL04:
	mov	ax,wlo [vcpLimLayout]
	mov	dx,whi [vcpLimLayout]
	sub	ax,[OFF_cpLimSave]
	sbb	dx,[SEG_cpLimSave]
	or	ax,dx
	je	FAL05
	mov	ax,midLayout2n
	mov	bx,1024
	cCall	AssertProcForNative,<ax,bx>
FAL05:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

;	/* Abort checks can change caPara...not a good thing! */
;	/* Yes...use caPara, not caParaL */
;	if (!fAbort && FNeRgw(&caSave, &caPara, cwCA))
;		{
	or	si,si
	jne	FAL06
	push	ds
	pop	es
	push	si	;save fAbort
	mov	si,dataoffset [caPara]
	lea	di,[caSave]
	mov	cx,cbCaMin/2
	repe	cmpsw
	pop	si	;restore fAbort
	je	FAL06

;		caParaL.doc = docNil;
	mov	[caParaL.docCa],docNil

;		if (caSave.doc != docNil)
;			CacheParaL(caSave.doc, caSave.cpFirst, fOutline);
	mov	bx,[caSave.docCa]
	errnz	<docNil>
	or	bx,bx
	je	FAL06
	mov	dx,[caSave.HI_cpFirstCa]
	mov	ax,[caSave.LO_cpFirstCa]
	mov	cx,[fOutline]
	;LN_CacheParaL takes doc in bx, cp in dx:ax, fOutline in cx
	;ax, bx, cx, dx are altered.
	call	LN_CacheParaL

;		}
FAL06:

;	return fAbort;
	xchg	ax,si

;}
cEnd


	;LN_XaFromXl performs XaFromXl(ax); ax, bx, cx, dx are altered.
; %%Function:LN_XaFromXl %%Owner:BRADV
PUBLIC LN_XaFromXl
LN_XaFromXl:
;#define XaFromXl(xl)		(NMultDiv(xl, czaInch, vfti.dxpInch))
;	LN_NMultDivL performs NMultDiv(ax, dx, bx)
;	ax, bx, cx, dx are altered.
	mov	dx,czaInch
	mov	bx,[vfti.dxpInchFti]
	jmp	short LN_NMultDivL

	;LN_YlFromYa performs YlFromYa(ax); ax, bx, cx, dx are altered.
; %%Function:LN_YlFromYa %%Owner:BRADV
PUBLIC LN_YlFromYa
LN_YlFromYa:
;#define YlFromYa(ya)		(NMultDiv(ya, vfti.dypInch, czaInch))
	mov	dx,[vfti.dypInchFti]

;	LN_NMultDivCzaInch performs NMultDiv(ax, dx, czaInch)
;	ax, bx, cx, dx are altered.
; %%Function:LN_NMultDivCzaInch %%Owner:BRADV
PUBLIC LN_NMultDivCzaInch
LN_NMultDivCzaInch:
	mov	bx,czaInch

;	LN_NMultDivL performs NMultDiv(ax, dx, bx)
;	ax, bx, cx, dx are altered.
LN_NMultDivL:
	cCall	NMultDiv,<ax,dx,bx>
	ret


; %%Function:LN_PdodDocL %%Owner:BRADV
PUBLIC LN_PdodDocL
LN_PdodDocL:
	;Takes doc in bx, result in bx.  Only bx is altered.
	shl	bx,1
	mov	bx,[bx.mpdochdod]
ifdef DEBUG
	cmp	bx,hNil
	jne	LN_PD101
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midLayout2n
	mov	bx,1025
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LN_PD101:
endif ;/* DEBUG */
	mov	bx,[bx]
	ret


	;LN_PutCpPlc performs PutCpPlc(vfls.hplclnh, cx, dx:ax);
	;ax, bx, cx, dx are altered.
LN_PutCpPlc:
	cCall	PutCpPlc,<[vfls.hplclnhFls], cx, dx, ax>
	ret


	;LN_LrpInPlCur performs LrpInPl(plbs->hpllr, plbs->ilrCur) with
	;the result in es:bx or es:ax.	It assumes plbs is passed in si.
	;ax, bx, cx, dx are altered.
; %%Function:LN_LrpInPlCur %%Owner:BRADV
PUBLIC LN_LrpInPlCur
LN_LrpInPlCur:
	mov	cx,[si.ilrCurLbs]
	;LN_LrpInPl performs LrpInPl(plbs->hpllr, cx) with the result in es:bx
	;or es:ax.  It assumes plbs is passed in si.
	;ax, bx, dx are altered.
LN_LrpInPl:
	;Assembler note: HpInPl also returns LpInPl in es:ax
	push	cx	;save ilr
	cCall	HpInPl,<[si.hpllrLbs],cx>
	pop	cx	;restore ilr
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	push	es	;save es from HpInPl
	mov	bx,dx
	shl	bx,1
	mov	ax,mpsbps[bx]
	mov	es,ax
	shr	ax,1
	jc	LN_LIP01
	;Assembler note: There is no way we should have to call ReloadSb here.
;	reload sb trashes ax, cx, and dx
;	cCall	ReloadSb,<>
	mov	ax,midLayout2n
	mov	bx,1026
	cCall	AssertProcForNative,<ax,bx>
LN_LIP01:
	pop	ax	;restore es from HpInPl
	mov	bx,es	;compare with es rederived from the SB of HpInPl
	cmp	ax,bx
	je	LN_LIP02
	mov	ax,midLayout2n
	mov	bx,1027
	cCall	AssertProcForNative,<ax,bx>
LN_LIP02:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */
	;Assembler note: use "mov" rather than "xchg" here (at the cost of
	;an extra byte), so that we can use "xchg ax,si" or "xchg ax,di"
	;if necessary after the return.
	mov	bx,ax
	ret


	;LN_IMacPllr performs cx = IMacPllr(plbs).  plbs is assumed to be
	;passed in si.	Only bx and cx are altered.
LN_IMacPllr:
	;***Begin in-line IMacPllr
	mov	bx,[si.hpllrLbs]
	mov	bx,[bx]
	mov	cx,[bx.iMacPl]
	;***End in-line IMacPllr
	ret


	;LN_IInPlc performs IInPlc(vfls.hplclnh, *pcp) where pcp is passed
	;in bx.  ax, bx, cx, dx are altered.
; %%Function:LN_IInPlc %%Owner:BRADV
PUBLIC LN_IInPlc
LN_IInPlc:
	cCall	IInPlc,<[vfls.hplclnhFls], [bx+2], [bx]>
	ret


	;LN_CpMacDocPlbs performs CpMacDocPlbs(si).
	;ax, bx, cx, dx are altered.
; %%Function:LN_CpMacDocPlbs %%Owner:BRADV
PUBLIC LN_CpMacDocPlbs
LN_CpMacDocPlbs:
;#define CpMacDocPlbs(plbs)	 CpMacDoc(plbs->doc)
	cCall	CpMacDoc,<[si.docLbs]>
	ret


	;LN_CpPlc performs CpPlc(vfls.hplclnh, cx)
	;ax, bx, cx, dx are altered.
; %%Function:LN_CpPlc %%Owner:BRADV
PUBLIC LN_CpPlc
LN_CpPlc:
	cCall	CpPlc,<[vfls.hplclnhFls],cx>
	ret


sEnd	layout2

        end
