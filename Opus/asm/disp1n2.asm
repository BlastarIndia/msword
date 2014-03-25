        include w2.inc
        include noxport.inc
        include consts.inc
	include structs.inc
	include windows.inc

createSeg	disp1_PCODE,disp1,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midDisp1n2	equ 11		; module ID, for native asserts
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

externFP	<N_PwwdWw>
externFP	<N_LoadFcidFull>
externFP	<DrawPatternLine>
externFP	<FEmptyRc>
externFP	<PmdcdCacheIdrb>
externFP	<HilitePicSel1>
externFP	<NMultDiv>
ifdef DEBUG
externFP	<SelectObjectFP>
externFP	<PatBltFP>
externFP	<StretchBltFP>
else ;!DEBUG
ifdef HYBRID
externFP	<SelectObjectPR>
externFP	<PatBltPR>
externFP	<StretchBltPR>
else ;!HYBRID
externFP	<SelectObject>
externFP	<PatBlt>
externFP	<StretchBlt>
endif ;!HYBRID
endif ;!DEBUG
externFP	<OtlMarkPos>
externFP	<DrawOtlPat>
externFP	<N_PtOrigin>
externFP	<N_FormatDrLine>
externFP	<ClipRectFromDr>
externFP	<ClientToScreen>
externFP	<FSectRc>
externFP	<GetWindow>
externFP	<GetClipBox>
externFP	<IsWindowVisible>
externFP	<N_PdrFetchAndFree>

ifdef DEBUG
externFP	<AssertProcForNative>
externFP	<ShakeHeapSb>
externFP	<S_XpFromDcp>
externFP	<S_LoadFcidFull>
externFP	<ScribbleProc>
externFP	<S_FMarkLine>
externFP	<S_FormatDrLine>
externFP	<S_PdrFetchAndFree>
externFP	<S_PtOrigin>
externFP	<LockHeap, UnlockHeap>
externFP	<S_DrawInsertLine>
endif ;DEBUG

sBegin	data

; 
; /* E X T E R N A L S */
; 
externW 	vhgrpchr	; extern struct CHR	  **vhgrpchr;
externW         mpwwhwwd        ; extern struct WWD       **mpwwhwwd[];
externW         selCur          ; extern struct SEL       selCur;
externW 	vsci		; extern struct SCI	  vsci;
externW 	vfli		; extern struct FLI	  vfli;
externW 	vfti		; extern struct FTI	  vfti;
externW 	selDotted	; extern struct SEL	  selDotted;
externW 	vxpFirstMark	; extern int		  vxpFirstMark;
externW 	vxpLimMark	; extern int		  vxpLimMark;
externW 	vhwwdOrigin	; extern struct WWD	  **vhwwdOrigin;
externW 	vpref		; extern struct PREF	  vpref;
externW 	vhwndApp	; extern HWND		  vhwndApp;
externW 	wwCur		; extern int		  wwCur;

; #ifdef DEBUG
ifdef DEBUG
externW 	cHpFreeze	; extern int		  cHpFreeze;
externW 	vdbs		; extern struct DBS	  vdbs;
; #endif /* DEBUG */
endif

sEnd    data


; CODE SEGMENT _DISP1

sBegin	disp1
	assumes cs,disp1
        assumes ds,dgroup
        assumes ss,dgroup


;-------------------------------------------------------------------------
;	MarkSel( ww, hpldr, idr, psel, pedl, cpMinDl, cpMacDl )
;-------------------------------------------------------------------------
;/* M A R K  S E L */
;/* Mark the selection (i.e. do appropriate inversion) psel in dc hdc
;   for line pedl whose location is given by (xpLine, ypLine).	*/
;/* We do not mark the selection if its hidden bit is on; if the selection */
;/* is in a TLE, we do not mark it and we set the hidden bit to TRUE */
;
;NATIVE MarkSel(ww, hpldr, idr, psel, pedl, cpMinDl, cpMacDl)
;int		 ww;
;struct PLDR	 **hpldr;
;int		 idr;
;struct EDL	 *pedl;
;struct SEL	 *psel;
;CP		 cpMinDl, cpMacDl;
;{
;	 struct DRF drfFetch;

; %%Function:MarkSel %%Owner:BRADV
cProc	MarkSel,<PUBLIC,FAR>,<si>
	ParmW	ww
	ParmW	hpldr
	ParmW	idr
	ParmW	psel
	ParmW	pedl
	ParmD	cpMinDl
	ParmD	cpMacDl

	LocalV	drfFetch,cbDrfMin

cBegin

	lea	ax,[drfFetch]
ifdef DEBUG
	cCall	S_PdrFetchAndFree,<[hpldr],[idr],ax>
else ;not DEBUG
	cCall	N_PdrFetchAndFree,<[hpldr],[idr],ax>
endif ;DEBUG
	xchg	ax,bx

;	if (psel->ww == ww && !psel->fHidden && !psel->fNil &&
;		psel->doc == PdrFetchAndFree(hpldr, idr, &drfFetch)->doc &&
;		psel->cpLim >= cpMinDl && psel->cpFirst <= cpMacDl)
;		{
	mov	si,[psel]
	mov	ax,[ww]
	cmp	[si.wwSel],ax
	jne	MS01
	cmp	[si.fHiddenSel],fFalse
	jne	MS01
	test	[si.fNilSel],maskFNilSel
	jne	MS01
	mov	ax,[si.docSel]
	cmp	ax,[bx.docDr]
	jne	MS01
	mov	ax,[OFF_cpMinDl]
	mov	dx,[SEG_cpMinDl]
	mov	bx,[si.LO_cpLimSel]
	mov	cx,[si.HI_cpLimSel]
	sub	bx,ax
	sbb	cx,dx
	jl	MS01
	mov	bx,[si.LO_cpFirstSel]
	mov	cx,[si.HI_cpFirstSel]
	sub	[OFF_cpMacDl],bx
	sbb	[SEG_cpMacDl],cx
	jl	MS01

;		if (FMarkLine(psel, hpldr, idr, pedl,
;			psel->cpFirst, psel->cpLim, cpMinDl))
;			{
	push	si
	push	[hpldr]
	push	[idr]
	push	[pedl]
	push	cx
	push	bx
	push	[si.HI_cpLimSel]
	push	[si.LO_cpLimSel]
	push	dx
	push	ax
ifdef DEBUG
	cCall	S_FMarkLine,<>
else ;not DEBUG
	push	cs
	call	near ptr N_FMarkLine
endif ;DEBUG
	or	ax,ax
	je	MS01

;			psel->fOn = fTrue;
	mov	[si.fOnSel],fTrue

;			}
;		}
MS01:

;}
cEnd

;End of MarkSel


;-------------------------------------------------------------------------
;	FMarkLine(psel, idr, pedl, cpFirstSel, cpLimSel, cpFirstLine)
;-------------------------------------------------------------------------
;/* F  M A R K	L I N E */
;/* Highlight the visible portion of the selection psel in the line pedl.
;Returns:
;if fIns:
;	 true	 if insert line is drawn
;	 false	 if nothing is drawn
;if !fIns
;	 true	 if selection is complete (cpLimSel < cpFirstLine)
;	 false	 if selection is not yet completely drawn
;Highlight is done effectively in xor mode.
;FormatLine may be called to ensure vfli validity.
;
;implicit: wwCur, psel->fInsEnd if fIns.
;pa is the highlighting mode: invert or dotted.
;xpOut, ypOut give starting coordinates of the display area showing the line
;*/
;
;native int FMarkLine(psel, hpldr, idr, pedl,
;		     cpFirstSel, cpLimSel, cpFirstLine)
;struct SEL *psel;
;struct PLDR	 **hpldr;
;int		 idr;
;struct EDL *pedl;	 /* read-only */
;CP cpFirstSel, cpLimSel, cpFirstLine;
;{
;	 int xpFirst, xpLim, dxp, dyp, ypLine, xwFirst, xwLim, ywLine;
;	 int sk = psel->sk, ichT;
;	 struct WWD *pwwd;
;	 struct DR *pdr;
;	 CP cpLimLine = cpFirstLine + pedl->dcp;
;	 struct RC rc;
;	 struct DRC drcp;
;	 struct RC rcwClip;
;	 BOOL fTableSel;
;	 struct DRF drfFetch;

; %%Function:N_FMarkLine %%Owner:BRADV
cProc	N_FMarkLine,<PUBLIC,FAR>,<si,di>
	ParmW	psel
	ParmW	hpldr
	ParmW	idr
	ParmW	pedl
	ParmD	cpFirstSelArg
	ParmD	cpLimSelArg
	ParmD	cpFirstLine

	LocalW	skShifted
	LocalW	ww
	LocalW	dyp
	LocalW	ypLine
	LocalW	xpFirst
	LocalW	xpLim
	LocalW	xpFirstT
	LocalW	ichT
	LocalD	cpLimLine
	LocalW	pdr
	LocalV	rcVar,cbRcMin
	LocalV	drcp,cbDrcMin
	LocalV	drfFetch,cbDrfMin
ifdef DFLDSELECT
	LocalV	rgw,10
	LocalV	szFrame,40
endif ;/* DFLDSELECT */

cBegin
	mov	si,[psel]
	mov	al,[si.skSel]
	and	al,maskSkSel
	mov	bptr ([skShifted]),al
	mov	di,[pedl]
	mov	bx,[di.LO_dcpEdl]
	mov	cx,[di.HI_dcpEdl]
	add	bx,[OFF_cpFirstLine]
	adc	cx,[SEG_cpFirstLine]
	mov	[OFF_cpLimLine],bx
	mov	[SEG_cpLimLine],cx
;/* since FormatLine can move the heap, and we don't always call it... */
;	 Debug(vdbs.fShakeHeap ? ShakeHeap() : 0);
ifdef DEBUG
;	/* Do this debug code with a call so as not to mess up
;	short jumps */
	call	FML30
endif ;/* DEBUG */

;/* check if sel entirely below or entirely above the line */
;	 if (cpLimSel < cpFirstLine) return (sk != skIns);
	mov	ax,[OFF_cpLimSelArg]
	mov	dx,[SEG_cpLimSelArg]
	sub	ax,[OFF_cpFirstLine]
	sbb	dx,[SEG_cpFirstLine]
	jge	FML01
	mov	al,bptr ([skShifted])
	sub	al,skIns SHL ibitSkSel
	cbw
	jmp	FML29
FML01:

;/* if the sel is a block selection, we want to continue for one line more
;   since its cpLimSel is really the cpFirst of the last line of the
;   selection. */
;	 if (cpLimSel == cpFirstLine && (!psel->fBlock || psel->fTable))
	or	ax,dx
	jne	FML02
	errnz	<(fBlockSel) - (fTableSel)>
	mov	al,[si.fBlockSel]
	and	al,maskfBlockSel + maskfTableSel
	xor	al,maskfBlockSel
	jz	FML02

;		 if (sk != skIns || psel->fInsEnd) return fFalse;
	cmp	bptr ([skShifted]),skIns SHL ibitSkSel
	jne	Ltemp006
	cmp	[si.fInsEndSel],fFalse
	jne	Ltemp006

FML02:

;	 if (cpFirstSel > cpLimLine) return fFalse;
	sub	bx,[OFF_cpFirstSelArg]
	sbb	cx,[SEG_cpFirstSelArg]
	jl	Ltemp006

;	 if (cpFirstSel == cpLimLine)
	or	bx,cx
	jne	FML03

;/* return if fIns sel is at the end of line but fInsEnd is not set;
;or if 0 chars left in a !fIns selection */
;		 if (sk != skIns || !psel->fInsEnd)
;			 return fFalse;
	cmp	bptr ([skShifted]),skIns SHL ibitSkSel
	jne	Ltemp006
	cmp	[si.fInsEndSel],fFalse
	je	Ltemp006

FML03:

;	 dyp = pedl->dyp;
	mov	ax,[di.dypEdl]
	mov	[dyp],ax

;	 ypLine = pedl->ypTop + dyp;
	add	ax,[di.ypTopEdl]
	mov	[ypLine],ax

;	 pdr = PdrFetchAndFree(hpldr, idr, &drfFetch);
	lea	ax,[drfFetch]
ifdef DEBUG
	cCall	S_PdrFetchAndFree,<[hpldr],[idr],ax>
else ;not DEBUG
	cCall	N_PdrFetchAndFree,<[hpldr],[idr],ax>
endif ;DEBUG
	xchg	ax,bx
	mov	[pdr],bx

;	 if (fTableSel = (pdr->fInTable && !(psel->fWithinCell || psel->fIns)))
;		 {
	errnz	<(fInsSel) - (fWithinCellSel) - 1>
	mov	ax,maskfWithinCellSel + (maskfInsSel SHL 8)
	and	ax,wptr ([si.fWithinCellSel])
	jne	FML031
	mov	al,maskfInTableDr
	and	al,[bx.fInTableDr]
	xor	al,maskfInTableDr
FML031:
	push	ax	;save !fTableSel
	mov	dx,[idr]
	jne	FML035	    ;expects dx == idr

;#ifdef REVIEW
;		 Inherited from Mac
;		 Assert(cpFirstSel <= cpFirstLine && cpLimLine <= cpLimSel);
;#endif

;		 if (psel->fColumn &&
;			 (idr < psel->itcFirst || idr >= psel->itcLim))
;			 return fFalse;
	test	[si.fColumnSel],maskfColumnSel
	je	FML033
	cmp	dx,[si.itcFirstSel]
	jl	FML032
	cmp	dx,[si.itcLimSel]
	jl	FML033
FML032:
	pop	ax	;remove !fTableSel from the stack
Ltemp006:
	jmp	FML26
FML033:

;		 drcp = pdr->drcl;
	;FML295 performs drcp = [ax]; no registers are altered.
	lea	ax,[bx.drclDr]
	call	FML295

;		 drcp.yp = 0;
	xor	ax,ax
	mov	[drcp.ypDrc],ax

;		 drcp.xp = xpFirst = idr == 0 ? 0 : -pdr->dxpOutLeft;
	or	dx,dx
	je	FML034
	sub	ax,[bx.dxpOutLeftDr]
FML034:
	mov	[xpFirst],ax
	mov	[drcp.xpDrc],ax

;		 xpLim = pdr->dxl + pdr->dxpOutRight;
	mov	cx,[bx.dxpOutRightDr]
	push	cx
	add	cx,[bx.dxlDr]
	mov	[xpLim],cx
	pop	cx

;		 drcp.dxp += (-xpFirst + pdr->dxpOutRight);
	sub	cx,ax
	add	[drcp.dxpDrc],cx

;		 }
	jmp	short FML036

;	 else
FML035: 	;expects dx == idr

;		 {
;		 drcp = pedl->drcp;
	;FML295 performs drcp = [ax]; no registers are altered.
	lea	ax,[di.drcpEdl]
	call	FML295

;		if (psel->fBlock)
;			{
	test	[si.fBlockSel],maskfBlockSel
	je	FML036

;			drcp.xp = psel->xpFirst;
;			drcp.dxp = psel->xpLim - drcp.xp;
;			}
	mov	ax,[si.xpFirstSel]
	mov	[drcp.xpDrc],ax
	neg	ax
	add	ax,[si.xpLimSel]
	mov	[drcp.dxpDrc],ax

;		 }
FML036: 	;expects dx == idr

;	 ClipRectFromDr(HwwdWw(psel->ww), hpldr, idr, &drcp, &rcwClip);
	mov	bx,[si.wwSel]
	shl	bx,1
	push	[bx.mpwwhwwd]
	push	[hpldr]
	push	dx
	lea	ax,[drcp]
	push	ax
	lea	ax,[si.rcwClipSel]
	push	ax
	cCall	ClipRectFromDr,<>

;/* Pad rcwClip to allow the insertion point to the right of the character */
;        if (psel->fIns && !psel->fBlock)
;	 	rcwClip.xwRight += (vsci.dxpBorder << 1);
;	 psel->rcwClip = rcwClip;
	errnz	<(fInsSel) - (fBlockSel)>
	mov	al,[si.fInsSel]
	and	al,maskFInsSel+maskFBlockSel
	xor	al,maskFInsSel
	jne	FML038
	mov	ax,[vsci.dxpBorderSci]
	shl	ax,1
	add	[si.rcwClipSel.xwRightRc],ax
FML038:
	pop	ax	;restore !fTableSel
;/* check if column or row selection */
;	 if (!fTableSel)
;		 {
	or	ax,ax
	je	Ltemp003

;/* check if block selection */
;		if (psel->fBlock)
;			{
	test	[si.fBlockSel],maskFBlockSel
	je	FML04

;			xpFirst = psel->xpFirst;
	mov	ax,[si.xpFirstSel]
	mov	[xpFirst],ax

;			xpLim = psel->xpLim;
	mov	ax,[si.xpLimSel]
	mov	[xpLim],ax

;			 }
Ltemp003:
	jmp	FML22

;		 else
;/* check if entire line is selected; for pics, we have to do a FormatLine anyway */
;			 {
FML04:

;			 BOOL fNeedFirst = fFalse, fNeedLim = fFalse;
	xor	cx,cx

;			 cpFirstSel = CpMax(cpFirstLine, cpFirstSel);
	mov	ax,[OFF_cpFirstLine]
	mov	dx,[SEG_cpFirstLine]
	mov	bx,ax
	sub	bx,[OFF_cpFirstSelArg]
	mov	bx,dx
	sbb	bx,[SEG_cpFirstSelArg]
	jl	FML05
	mov	[OFF_cpFirstSelArg],ax
	mov	[SEG_cpFirstSelArg],dx
FML05:

;			 cpLimSel = CpMin(cpLimLine, cpLimSel);
	mov	ax,[OFF_cpLimLine]
	mov	dx,[SEG_cpLimLine]
	mov	bx,ax
	sub	bx,[OFF_cpLimSelArg]
	mov	bx,dx
	sbb	bx,[SEG_cpLimSelArg]
	jge	FML06
	mov	[OFF_cpLimSelArg],ax
	mov	[SEG_cpLimSelArg],dx
FML06:

;/* following series of checks is simple in native code. */
;/* REVIEW: BobZ, do you need sk == skIns here?  The current Mac code does not
;	    have it. */
;			 if (psel->fGraphics /* || sk == skIns */)
;				 {
;				 fNeedFirst = fTrue; fNeedLim = fTrue;
;				 goto LFormat;
;				 }
	test	[si.fGraphicsSel],maskFGraphicsSel
	je	FML066
	dec	cx
	jmp	LFormat
FML066:

;			 if (cpFirstSel == cpFirstLine)
;				 xpFirst = pedl->xpLeft;
	mov	ax,[OFF_cpFirstSelArg]
	mov	dx,[SEG_cpFirstSelArg]
	mov	bx,[di.xpLeftEdl]
	cmp	ax,[OFF_cpFirstLine]
	jne	FML07
	cmp	dx,[SEG_cpFirstLine]
	je	FML10

;			 else if (cpFirstSel == psel->cpFirst
;				 && psel->xpFirst != xpNil && sk != skIns)
;				 xpFirst = psel->xpFirst;
;			 else if (cpFirstSel == psel->cpLim
;				 && psel->xpLim != xpNil && sk != skIns)
;				 xpFirst = psel->xpLim;
FML07:
	cmp	bptr ([skShifted]),skIns SHL ibitSkSel
	je	FML09
	cmp	ax,[si.LO_cpFirstSel]
	jne	FML08
	cmp	dx,[si.HI_cpFirstSel]
	jne	FML08
	mov	bx,[si.xpFirstSel]
	cmp	bx,xpNil
	jne	FML10

FML08:
	cmp	ax,[si.LO_cpLimSel]
	jne	FML09
	cmp	dx,[si.HI_cpLimSel]
	jne	FML09
	mov	bx,[si.xpLimSel]
	cmp	bx,xpNil
	jne	FML10

;			 else fNeedFirst = fTrue;
FML09:
	inc	cx

FML10:
	mov	[xpFirst],bx

;			 if (cpLimSel == cpLimLine)
;				 xpLim = pedl->xpLeft + pedl->dxp;
	mov	ax,[OFF_cpLimSelArg]
	mov	dx,[SEG_cpLimSelArg]
	mov	bx,[di.xpLeftEdl]
	add	bx,[di.dxpEdl]
	cmp	ax,[OFF_cpLimLine]
	jne	FML11
	cmp	dx,[SEG_cpLimLine]
	je	FML14

;			 else if (cpLimSel == psel->cpFirst
;				 && psel->xpFirst != xpNil && sk != skIns)
;				 xpLim = psel->xpFirst;
;			 else if (cpLimSel == psel->cpLim
;				 && psel->xpLim != xpNil && sk != skIns)
;				 xpLim = psel->xpLim;
FML11:
	cmp	bptr ([skShifted]),skIns SHL ibitSkSel
	je	FML13
	cmp	ax,[si.LO_cpFirstSel]
	jne	FML12
	cmp	dx,[si.HI_cpFirstSel]
	jne	FML12
	mov	bx,[si.xpFirstSel]
	cmp	bx,xpNil
	jne	FML14

FML12:
	cmp	ax,[si.LO_cpLimSel]
	jne	FML13
	cmp	dx,[si.HI_cpLimSel]
	jne	FML13
	mov	bx,[si.xpLimSel]
	cmp	bx,xpNil
	jne	FML14

 ;			 else fNeedLim = fTrue;
FML13:
	inc	ch

FML14:
	mov	[xpLim],bx

;			 if (fNeedFirst || fNeedLim)
;				 {
	jcxz	FML19

;				 int xpLimT, xpFirstT;
;/* ensure vfli is correct */
;LFormat:
LFormat:
	push	cx	;save fNeed flags

;				 FormatLineDr(psel->ww, cpFirstLine, pdr);
ifdef DEBUG
	cCall	S_FormatDrLine,<[si.wwSel],[SEG_cpFirstLine],[OFF_cpFirstLine],[pdr]>
else ;not DEBUG
	cCall	N_FormatDrLine,<[si.wwSel],[SEG_cpFirstLine],[OFF_cpFirstLine],[pdr]>
endif ;DEBUG

;/* decode cpFirstSel => xpFirst */
;
;				 xpLimT = XpFromDcp(
;					 (cpFirstSel - cpFirstLine),
;					 (cpLimSel - cpFirstLine),
;					 &xpFirstT, &ichT);
	mov	ax,[OFF_cpFirstSelArg]
	mov	dx,[SEG_cpFirstSelArg]
	sub	ax,[OFF_cpFirstLine]
	sbb	dx,[SEG_cpFirstLine]
	push	dx
	push	ax
	mov	ax,[OFF_cpLimSelArg]
	mov	dx,[SEG_cpLimSelArg]
	sub	ax,[OFF_cpFirstLine]
	sbb	dx,[SEG_cpFirstLine]
	push	dx
	push	ax
	lea	cx,[xpFirstT]
	lea	dx,[ichT]
	push	cx
	push	dx
ifdef DEBUG
	cCall	S_XpFromDcp,<>
else ;not DEBUG
	push	cs
	call	near ptr N_XpFromDcp
endif ;DEBUG

;#ifdef DFLDSELECT
;				 {
;				 uns rgw [5];
;				 rgw[0]=(uns)(cpFirstSel-cpFirstLine);
;				 rgw[1]=(uns)(cpLimSel-cpFirstLine);
;				 rgw[2]=(uns)xpFirstT;
;				 rgw[3]=(uns)xpLimT;
;				 rgw[4]=ichT;
;				 CommSzRgNum(
;				     SzShared("XpFromDcp (dcp1,dcp2,xpF,xpL,ich): "),
;				     rgw, 5);
;				 }
;#endif /* DFLDSELECT */
ifdef DFLDSELECT
;	Do this DFLDSELECT stuff with a call so
;	as not to mess up short jumps */
	call	FML32
endif ;/* DFLDSELECT */

	pop	cx	;restore fNeed flags
;				 if (fNeedFirst) xpFirst = xpFirstT;
	or	cl,cl
	je	FML17
	mov	bx,[xpFirstT]
	mov	[xpFirst],bx
FML17:

;				 if (fNeedLim) xpLim = xpLimT;
	or	ch,ch
	je	FML18
	mov	[xpLim],ax
FML18:

;				 }
FML19:

;/* return globals */
;			 if (cpFirstSel > cpFirstLine) vxpFirstMark = xpFirst;
	mov	ax,[OFF_cpFirstLine]
	mov	dx,[SEG_cpFirstLine]
	sub	ax,[OFF_cpFirstSelArg]
	sbb	dx,[SEG_cpFirstSelArg]
	jge	FML20
	mov	ax,[xpFirst]
	mov	[vxpFirstMark],ax
FML20:

;			 if (cpLimSel < cpLimLine) vxpLimMark = xpLim;
	mov	ax,[OFF_cpLimSelArg]
	mov	dx,[SEG_cpLimSelArg]
	sub	ax,[OFF_cpLimLine]
	sbb	dx,[SEG_cpLimLine]
	jge	FML21
	mov	ax,[xpLim]
	mov	[vxpLimMark],ax
FML21:

;			 }
;		 }
FML22:

;/* now we have xpFirst, xpLim in unscrolled line coordinates. Transform: */
;	 ywLine = YwFromYp(hpldr, idr, ypLine);
;	***Begin in line YwFromYp
;	 pt = PtOrigin(hpldr, idr);
ifdef DEBUG
	cCall	S_PtOrigin,<[hpldr], [idr]>
else ;not DEBUG
	cCall	N_PtOrigin,<[hpldr], [idr]>
endif ;DEBUG
;	 yw = yp + (*vhwwdOrigin)->rcePage.yeTop + (*vhwwdOrigin)->ywMin + pt.yl;
	mov	bx,[vhwwdOrigin]
	mov	bx,[bx]
	add	dx,[bx.rcePageWwd.yeTopRc]
	add	dx,[bx.ywMinWwd]
	add	dx,[ypLine]
;	 return(yw);
;	***End in lin YwFromYp

;	 xwFirst = XwFromXp(hpldr, idr, xpFirst);
;	***Begin in line XwFromXp
;	 pt = PtOrigin(hpldr, idr);
;	 cCall	 PtOrigin,<[hpldr],[idr]>
;	 xw = xp + (*vhwwdOrigin)->rcePage.xeLeft + (*vhwwdOrigin)->xwMin + pt.xl;
;	 mov	 bx,[vhwwdOrigin]
;	 mov	 bx,[bx]
	add	ax,[bx.rcePageWwd.xeLeftRc]
	add	ax,[bx.xwMinWwd]
	mov	cx,ax
	add	ax,[xpFirst]
;	 return(xw);
;	***End in lin XwFromXp

;	 xwLim = xwFirst - xpFirst + xpLim;
	add	cx,[xpLim]

;	ax = xwFirst, cx = xwLim, dx = ywLine, si = psel
;	 if (sk != skIns)
;		 {
	cmp	bptr ([skShifted]),skIns SHL ibitSkSel
	je	Ltemp011

;		 HDC hdc;
;		 pwwd = PwwdWw(psel->ww);
	mov	di,[si.wwSel]
	shl	di,1
	mov	di,[di.mpwwhwwd]
	mov	di,[di]

;	Assembler note: get the hdc when we need it, not here.
;		 hdc = pwwd->hdc;
;		 xwFirstNoClip = xwFirst;
	mov	bx,ax

;		 /* clip out the selbar */
;		 xwFirst = max(xwFirst, pwwd->xwMin);
	cmp	ax,[di.xwMinWwd]
	jge	FML23
	mov	ax,[di.xwMinWwd]
FML23:

;/* invert solid highlighting */
;		 PrcSet(&rc, xwFirst, ywLine - dyp, xwLim, ywLine);
;	ax = xwFirst, bx = xwFirstNoClip, cx = xwLim, dx = ywLine,
;	si = psel, di = pwwd
	push	di	;save pwwd
	push	bx	;save xwFirstNoClip
	push	dx	;save ywLine
	lea	di,[rcVar]
	push	di	;argument for FSectRc
	lea	bx,[si.rcwClipSel]
	push	bx	;argument for FSectRc
	push	di	;argument for FSectRc
;	***Begin in line PrcSet
	push	ds
	pop	es
	stosw
	mov	ax,dx
	sub	ax,[dyp]
	stosw
	xchg	ax,cx
	stosw
	xchg	ax,dx
	stosw
;	***End in line PrcSet

;		 if (!FSectRc(&rc, &psel->rcwClip, &rc))
;			return fFalse;
	cCall	FSectRc,<>
	or	ax,ax
	pop	dx	;restore ywLine
	pop	bx	;restore xwFirstNoClip
	pop	di	;restore pwwd
	je	FML26

;		 if (psel->pa != paDotted || sk == skBlock)
;			 {
	cmp	[si.paSel],paDotted
	jne	FML235
	cmp	bptr ([skShifted]),skBlock SHL ibitSkSel
	jne	FML25
FML235:

;			 if (psel->fGraphics)
;				{
	test	[si.fGraphicsSel],maskFGraphicsSel
	je	FML24

;				rc.xwLeft = xwFirstNoClip;
;                               rc.ywBottom = ywLine;
;                               rc.ywTop = ywLine-dyp; /*necessary only to */
;						       /*make valid rect   */
	mov	[rcVar.xwLeftRc],bx
	mov	[rcVar.ywBottomRc],dx
	sub	dx,[dyp]
	mov	[rcVar.ywTopRc],dx

;				HilitePicSel1(psel, &rc, &psel->rcwClip);
;				}
	lea	ax,[rcVar]
	lea	bx,[si.rcwClipSel]
	cCall	HilitePicSel1,<si,ax,bx>
	jmp	short FML26
	;Assembler note: This jump to FML27 has nothing to do with nearby code.
Ltemp011:
	jmp	short FML27

;			 else

FML24:
;				if (rc.ywTop < pwwd->rcwDisp.ywTop)
;					{
;					rc.ywTop = pwwd->rcwDisp.ywTop;
;					}
	mov	ax,[di.rcwDispWwd.ywTopRc]
	cmp	ax,[rcVar.ywTopRc]
	jle	FML245
	mov	[rcVar.ywTopRc],ax
FML245:
;				PatBltRc( hdc, &rc, DSTINVERT );
	lea	bx,[rcVar]
	mov	cx,DSTINVERT_L
	mov	dx,DSTINVERT_H
	push	[di.hdcWwd]
	push	bx
	push	dx
	push	cx
	push	cs
	call	near ptr PatBltRc
	jmp	short FML26

;			 }
;		 else
FML25:

;			 {
;			 /* note: rc has been clipped by the clip rect */
;			 DrawPatternLine( hdc, rc.xpLeft, ywLine,
;				rc.xpRight - rc.xpLeft, ipatHorzGray,
;				pltHorz | pltInvert );
;			 }
;	dx = ywLine, si = psel, di = pwwd
	mov	ax,[rcVar.xpLeftRc]
	mov	cx,[rcVar.xpRightRc]
        push	[di.hdcWwd] 
	push	ax
	push	dx
	sub	cx,ax
	push	cx
	mov	ax,ipatHorzGray
	push	ax
	mov	ax,pltHorz + pltInvert
	push	ax
	cCall	DrawPatternLine,<>

FML26:
;		 return fFalse;
	errnz	<fFalse>
	xor	ax,ax
	jmp	short FML29

;		 }
;	 else
;		 {
FML27:

;/* Set up cursor draw dimensions. */
;		 Assert( vfli.ww == psel->ww );
ifdef DEBUG
;	/* Assert(vfli.ww == psel->ww ) with a call so
;	as not to mess up short jumps */
	call	FML35
endif ;/* DEBUG */

;		 psel->dyp = min(vfli.dypFont, vfli.dypLine - vfli.dypAfter);
;		 psel->yw = ywLine - vfli.dypAfter;
;		 psel->xw = xwFirst;
;	ax = xwFirst, cx = xwLim, dx = ywLine, si = psel
	mov	[si.xwSel],ax
	sub	dx,[vfli.dypAfterFli]
	mov	ax,[vfli.dypLineFli]
	sub	ax,[vfli.dypAfterFli]
	cmp	ax,[vfli.dypFontFli]
	jle	FML28
	mov	ax,[vfli.dypFontFli]
FML28:
	mov	[si.dypSel],ax
	mov	[si.ywSel],dx

;		 psel->tickOld = 0;
	xor	ax,ax
	mov	[si.LO_tickOldSel],ax
	mov	[si.HI_tickOldSel],ax

;		 DrawInsertLine(psel);
	push	si
ifdef DEBUG
	cCall	S_DrawInsertLine,<>
else ;not DEBUG
	push	cs
	call	near ptr N_DrawInsertLine
endif ;DEBUG

;		 return fTrue;
	mov	ax,fTrue

;		 }
FML29:
;}
cEnd

	;FML295 performs drcp = [dx]; no registers are altered.
FML295:
	push	di
	push	ds
	pop	es
	xchg	ax,si
	lea	di,[drcp]
	movsw
	movsw
	movsw
	movsw
	xchg	ax,si
	pop	di
	ret

;			 Debug(vdbs.fShakeHeap ? ShakeHeap() : 0);
ifdef DEBUG
FML30:
	cmp	vdbs.fShakeHeapDbs,fFalse
	je	FML31
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,1
	cCall	ShakeHeapSb,<ax>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FML31:
	ret
endif ;/* DEBUG */


;#ifdef DFLDSELECT
;    {
;			 uns rgw [5];
ifdef DFLDSELECT
FML32:
	push	ax
	push	bx
	push	cx
	push	dx
	push	si
	push	di

;			 rgw[0]=(uns)(cpFirstSel-cpFirstLine);
	mov	bx,[OFF_cpFirstSelArg]
	sub	bx,[OFF_cpFirstLine]
	mov	word ptr [([rgw])+00000h],bx

;			 rgw[1]=(uns)(cpLimSel-cpFirstLine);
	mov	bx,[OFF_cpLimSelArg]
	sub	bx,[OFF_cpFirstLine]
	mov	word ptr [([rgw])+00002h],bx

;			 rgw[2]=(uns)xpFirstT;
	mov	bx,[xpLimT]
	mov	word ptr [([rgw])+00004h],bx

;			 rgw[3]=(uns)xpLimT;
	mov	word ptr [([rgw])+00006h],ax

;			 rgw[4]=ichT;
	mov	ax,ichT
	mov	word ptr [([rgw])+00008h],ax

;			 CommSzRgNum(
;			     SzShared("XpFromDcp (dcp1,dcp2,xpF,xpL,ich): "),
;			     rgw, 5);
FML33:
	db	'XpFromDcp (dcp1,dcp2,xpF,xpL,ich): '
	db	0
FML34:
	push	ds
	push	cs
	pop	ds
	mov	si,FML33
	push	ss
	pop	es
	mov	di,[szFrame]
	mov	cx,(FML34) - (FML33)
	rep	movsb
	pop	ds
	lea	ax,[szFrame]
	lea	bx,rgw
	mov	cx,5
	cCall	CommSzRgNum,<ax, bx, cx>

	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
;    }
;#endif /* DFLDSELECT */
	ret
endif ;/* DFLDSELECT */


;		 Assert( vfli.ww == psel->ww );
ifdef DEBUG
FML35:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[vfli.wwFli]
	cmp	ax,[si.wwSel]
	je	FML36
	mov	ax,midDisp1n2
	mov	bx,1010
	cCall	AssertProcForNative,<ax,bx>
FML36:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */


;-------------------------------------------------------------------------
;	XpFromDcp(dcp1, dcp2, pxpFirst, pich)
;-------------------------------------------------------------------------
;/* X P  F R O M  D C P */
;/* returns the xpFirst integrated from the beginning of the line until the
;front of the char with cp>=cpMin+dcp1 with vanished runs taken into
;account. Continues integration to dcp2 with the position returned as the
;result.
;Assumes dcp's <= dcpMac of the line (no check for chrmEnd.)
;*/
;
;native int XpFromDcp(dcp1, dcp2, pxpFirst, pich)
;uns dcp1, dcp2; uns *pxpFirst;  int *pich;
;{
;	 uns *pdxp = &vfli.rgdxp[0];
;	 int ich, ichNext, xp, chrm;
;	 BOOL fFirst = fTrue;
;	 int dcp = dcp1; /* modified by vanish */
;	 struct CHR *pchr = &(**vhgrpchr)[0];
;	 int xpCeil; /* mac xp seen during scan before formula moves */
;	 int xpBeforeFG;

; %%Function:N_XpFromDcp %%Owner:BRADV
cProc	N_XpFromDcp,<PUBLIC,FAR>,<si,di>
	ParmD	dcp1
	ParmD	dcp2
	ParmW	pxpFirst
	ParmW	pich

	LocalW	pdxp
	LocalW	pchrNew
	LocalW	xpBeforeFG

cBegin
	mov	[pdxp],dataoffset [vfli.rgdxpFli]
	mov	bx,[vhgrpchr]
	mov	bx,[bx]
	mov	ch,fTrue
	mov	si,[OFF_dcp1]
	mov	di,[SEG_dcp1]

;	 Assert( vfli.ww != docNil );
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	vfli.wwFli,wwNil
	jne	XFD01
	mov	ax,midDisp1n2
	mov	bx,1012
	cCall	AssertProcForNative,<ax,bx>
XFD01:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;	 ich = 0;
	xor	dx,dx

;	 xp = vfli.xpLeft;
	mov	ax,[vfli.xpLeftFli]

;	 for (;;)
;		 {
;		 xpBeforeFG = -1;
XFD02:
	mov	[xpBeforeFG],-1

;		 while (pchr->ich == ich)
;			 {
XFD03:
	cmp	[bx.ichChr],dl
	jne	XFD07

;			 if ((chrm = pchr->chrm) == chrmVanish)
;				 dcp -= ((struct CHRV *)pchr)->dcp;
	mov	cl,[bx.chrmChr]
	cmp	cl,chrmVanish
	jne	XFD04
	sub	si,[bx.LO_dcpChrv]
	sbb	di,[bx.HI_dcpChrv]
	jmp	short XFD06

;			 else if (chrm == chrmDisplayField)
;				 {
XFD04:
	cmp	cl,chrmDisplayField
	jne	XFD05

;/* chrmDisplayField has a vfli.rgdxp entry, which takes care of width */
;/* -1 adjusts for the vfli.rgch entry that corresponds to the dxp     */
;				 dcp -= ((struct CHRDF *)pchr)->dcp - 1;
;				 }
	sub	si,[bx.LO_dcpChrdf]
	sbb	di,[bx.HI_dcpChrdf]
	add	si,1
	adc	di,0
	jmp	short XFD06
XFD05:

;			 else if (chrm == chrmFormatGroup)
;				 {
	cmp	cl,chrmFormatGroup
	jne	XFD06

;				 struct CHR *pchrNew = (char *)pchr +
;					 ((struct CHRFG *)pchr)->dbchr;
	push	bx	;save pchr
	add	bx,[bx.dbchrChrfg]

;				 int dich = pchrNew->ich - ich;
	mov	bl,[bx.ichChr]
	xor	bh,bh
	sub	bx,dx

;				 ich += dich;
	add	dx,bx

;				 dcp -= ((struct CHRFG *)pchr)->dcp - dich;
	add	si,bx
	adc	di,0
	pop	bx	;restore pchr
	sub	si,[bx.LO_dcpChrfg]
	sbb	di,[bx.HI_dcpChrfg]

;				 pdxp = &vfli.rgdxp[ich];
	push	dx	;save ich
	shl	dx,1
	add	dx,dataoffset [vfli.rgdxpFli]
	mov	[pdxp],dx
	pop	dx	;restore ich

;				 xpBeforeFG = xp;
	mov	[xpBeforeFG],ax

;				 xp += ((struct CHRFG *)pchr)->dxp;
	add	ax,[bx.dxpChrfg]

;				 pchr = pchrNew;
	add	bx,[bx.dbchrChrfg]

;				 break;
	jmp	short XFD07

;				 }

;			Assert (chrm != chrmFormula); /* hidden by formatgroup */
	;Assembler note: This assert is performed at XFD20
XFD06:

;			 if (chrm == chrmEnd) break;
	cmp	cl,chrmEnd
	je	XFD07

;			 Assert( chrm == chrmTab ||
;				 chrm == chrmDisplayField
;				 chrm == chrmChp || chrm == chrmVanish
;				 || chrm == chrmFormatGroup );
ifdef DEBUG
;	/* Assert( chrm == ... ) with a call so
;	as not to mess up short jumps */
	call	XFD20
endif ;/* DEBUG */

;			 (char *)pchr += chrm;
	add	bl,cl
	adc	bh,0

;			 }
	jmp	short XFD03

;		 if ((CP) ich >= dcp)
;			 {
XFD07:
	push	ax	;save xp
	push	dx	;save ich
	xchg	dx,ax
	cwd
	sub	ax,si
	sbb	dx,di
	pop	dx	;restore ich
	pop	ax	;restore xp
	jl	XFD10

;			 if (fFirst)
;				 {
	errnz	<fFalse>
	or	ch,ch
	je	XFD09

;				 fFirst = fFalse;
	xor	ch,ch

;				 *pxpFirst = dcp1 == 0 ? vfli.xpLeft :
;					 xpBeforeFG == -1 || ich == dcp ?
;					 xp : xpBeforeFG;
	push	bx	;save pchr
	push	ax	;save xp
	;XFD24 performs ax = (xpBeforeFG == -1 || 0:dx == di:si)
	;	? ax : xpBeforeFG;
	;Only ax is altered.
	call	XFD24
	mov	bx,[OFF_dcp1]
	or	bx,[SEG_dcp1]
	mov	bx,[pxpFirst]
	jne	XFD08
	mov	ax,[vfli.xpLeftFli]
XFD08:
	mov	[bx],ax
	pop	ax	;restore xp
	pop	bx	;restore pchr

;/* correct dcp2 by the vanish already seen: dcp = dcp2 - (dcp1 - dcp) */
;				 dcp += dcp2 - dcp1;
	add	si,[OFF_dcp2]
	adc	di,[SEG_dcp2]
	sub	si,[OFF_dcp1]
	sbb	di,[SEG_dcp1]

;				 continue;
;				 }
	jmp	short Ltemp002

;			 else
;				 {
XFD09:

;				 *pich = ich;
	mov	bx,[pich]
	mov	[bx],dx

;				 return xpBeforeFG == -1 || ich == dcp ?
;					 xp : xpBeforeFG;
	;XFD24 performs ax = (xpBeforeFG == -1 || 0:dx == di:si)
	;	? ax : xpBeforeFG;
	;Only ax is altered.
	call	XFD24
	jmp	short XFD17

;				 }
;			 }
XFD10:

;		 if (chrm == chrmEnd)
;			 {
	cmp	cl,chrmEnd
	jne	XFD12

;#ifdef  DEBUG
;			 struct WWD *pwwdT = PwwdWw(vfli.ww);
;			 Assert(vfli.fOutline || pwwdT->fPageView);
;#endif
ifdef DEBUG
	call	XFD22
endif ;DEBUG

;			 if (fFirst)
	errnz	<fFalse>
	or	ch,ch
	je	XFD11

;				 *pxpFirst = xp;
	mov	bx,[pxpFirst]
	mov	[bx],ax
XFD11:

;			 *pich = ich;
	mov	bx,[pich]
	mov	[bx],dx

;			 return xp;
	jmp	short XFD17

;			 }
XFD12:

;/* scan until dcp expires or beginning of chr is reached */
;		 ichNext = CpMin((CP)pchr->ich, dcp);
	push	cx	;save chrm, fFirst
	mov	cl,[bx.ichChr]
	xor	ch,ch
	or	di,di
	js	XFD13
	jnz	XFD14
	cmp	cx,si
	jb	XFD14
XFD13:
	mov	cx,si
XFD14:

;/* next while loop is speed critical section */
;		 while (ich < ichNext)
;			 {
	push	cx	;save ichNext
	sub	cx,dx
	jle	XFD16

;			 xp += *pdxp++;
;			 ich++;
	xchg	ax,dx
	xchg	si,[pdxp]   ;save OFF_dcp
XFD15:
	lodsw
	add	dx,ax
	loop	XFD15
	xchg	si,[pdxp]   ;restore OFF_dcp
	xchg	ax,dx

;			 }

XFD16:
	pop	dx	;ich = ichNext
	pop	cx	;restore chrm,fFirst

;		 }
Ltemp002:
	jmp	XFD02

XFD17:
;}
cEnd


;			 Assert( chrm == chrmTab ||
;				 chrm == chrmDisplayField
;				 chrm == chrmChp || chrm == chrmVanish
;				 || chrm == chrmFormatGroup );
ifdef DEBUG
XFD20:
	cmp	cl,chrmTab
	je	XFD21
	cmp	cl,chrmDisplayField
	je	XFD21
	cmp	cl,chrmChp
	je	XFD21
	cmp	cl,chrmVanish
	je	XFD21
	cmp	cl,chrmFormatGroup
	je	XFD21
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midDisp1n2
	mov	bx,1014
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
XFD21:
	ret
endif ;/* DEBUG */

;			 struct WWD *pwwdT = PwwdWw(vfli.ww);
;			 Assert(vfli.fOutline || pwwdT->fPageView);
ifdef DEBUG
XFD22:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	bx,[vfli.wwFli]
	shl	bx,1
	mov	bx,[bx.mpwwhwwd]
	mov	bx,[bx]
	cmp	[vfli.fOutlineFli],fFalse
	jne	XFD23
	test	[bx.fPageViewWwd],maskFPageViewWwd
	jne	XFD23
	mov	ax,midDisp1n2
	mov	bx,1015
	cCall	AssertProcForNative,<ax,bx>
XFD23:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

	;XFD24 performs ax = (xpBeforeFG == -1 || 0:dx == di:si)
	;	? ax : xpBeforeFG;
	;Only ax is altered.
XFD24:
	cmp	[xpBeforeFG],-1
	je	XFD25
	push	dx
	sub	dx,si
	or	dx,di
	pop	dx
	je	XFD25
	mov	ax,[xpBeforeFG]
XFD25:
	ret


;-------------------------------------------------------------------------
;	PatBltRc( hdc, prc, rop )
;-------------------------------------------------------------------------
;/* P A T  B L T  R C */
;native PatBltRc( hdc, prc, rop )
;HDC hdc;
;struct RC *prc;
;long rop;
;{  /* Pat Blt for our RC type (test for Nil is necessary because it uses
;      our own criteria, not Windows') */

; %%Function:PatBltRc %%Owner:BRADV
cProc	PatBltRc,<PUBLIC,FAR>,<si>
	ParmW	hdc
	ParmW	prc
	ParmD	rop

cBegin
; if (!FEmptyRc( prc ))
	cCall	FEmptyRc,<[prc]>
	or	ax,ax
	jnz	PBR01

;	 PatBlt( hdc, prc->xpLeft, prc->ypTop, prc->xpRight - prc->xpLeft,
;		      prc->ypBottom - prc->ypTop, rop );
	push	[hdc]
	mov	si,[prc]
	errnz	<(xpLeftRc)-0>
	errnz	<(ypTopRc)-2>
	errnz	<(xpRightRc)-4>
	errnz	<(ypBottomRc)-6>
	lodsw
	push	ax
	xchg	ax,cx
	lodsw
	push	ax
	xchg	ax,dx
	lodsw
	sub	ax,cx
	push	ax
	lodsw
	sub	ax,dx
	push	ax
	push	[SEG_rop]
	push	[OFF_rop]
ifdef DEBUG
	cCall	PatBltFP,<>
else ;!DEBUG
ifdef HYBRID
	cCall	PatBltPR,<>
else ;!HYBRID
	cCall	PatBlt,<>
endif ;!HYBRID
endif ;!DEBUG

PBR01:
;}
cEnd


;-------------------------------------------------------------------------
;	AddVisiSpaces(hdc, dxpToXw, ywLine, prcwClip)
;-------------------------------------------------------------------------
;/* A D D  V I S I  S P A C E S */
;/* Put a centered dot in each space character, and show all tabs.
;Graphport and clipping are all set up. Parameters are in vfli
;and from locals in DisplayFli */
;HANDNATIVE C_AddVisiSpaces(hdc, dxpToXw, ywLine, prcwClip)
;HDC hdc;
;int dxpToXw;
;int ywLine;
;struct RC *prcwClip;
;{
;        int ich, ichNext;
;        int chrm, ch;
;        struct CHR *pchr;
;	 int bchrCur;
;        int *pdxp;
;        int xwPos, ywPos;
;        int xwLast;
;        int hps, hpsLast = -1; /* illegal value */
;        int dypDot, dypDotPrev = 0;
;        HBRUSH hbrOld;
;        int dxp, dyp;        
;        GRPFVISI grpfvisi;

; %%Function:N_AddVisiSpaces %%Owner:BRADV
cProc	N_AddVisiSpaces,<PUBLIC,FAR>,<si,di>
	ParmW	hdc
	ParmW	dxpToXw
	ParmW	ywLine
	ParmW	prcwClip

	LocalW	hpsLast
	LocalW	xwLast
	LocalW	dypDotPrev
	LocalW	xwT
	LocalW	ywT
	;frame variables for LN_DrawChVis
	LocalW	idcb
	LocalW	pmdcd
	LocalW	ywPos
	LocalV	rcT,cbRcMin
	LocalV	rcwDest,cbRcMin

cBegin
	mov	bptr ([hpsLast]),-1
	mov	[dypDotPrev],0

;/* position to start of the line. c.f. DisplayFli */
;        xwPos = vfli.xpLeft + dxpToXw;
;        ywPos = ywLine - vfli.dypBase;
	mov	cx,[vfli.xpLeftFli]
	add	cx,[dxpToXw]
	mov	dx,[ywLine]
	sub	dx,[vfli.dypBaseFli]

;	if (vfli.omk != omkNil && vfli.fOutline)
;		{
	mov	al,[vfli.omkFli]
	errnz	<omkNil - 0>
	and	ax,maskOmkFli
	je	AVS007
	cmp	[vfli.fOutlineFli],fFalse
	je	AVS007
	push	dx	; save ywPos
	push	cx	; save xwPos

;		int	xwT, ywT;
;		OtlMarkPos(xwPos, ywLine - vfli.dypLine, vfli.dypLine,
;				&xwT, &ywT);
;		DrawOtlPat(vfli.ww, hdc, vfli.omk == omkPlus ? idcbOtlMarkPlus :
;			(vfli.omk == omkMinus) ?
;				idcbOtlMarkMinus : idcbOtlMarkBody,
;			xwT, ywT);
	errnz	<omkPlus - 1>
	errnz	<omkMinus - 2>
	errnz	<omkBody - 3>
	errnz	<idcbOtlMarkPlus - 0>
	errnz	<idcbOtlMarkMinus - 2>
	errnz	<idcbOtlMarkBody - 4>
	errnz	<maskOmkFli - 0C0H>
	rol	al,1
	rol	al,1
	dec	ax
	rol	al,1
	push	[vfli.wwFli]	;argument for DrawOtlPat
	push	[hdc]		;argument for DrawOtlPat
	push	ax		;argument for DrawOtlPat
	push	cx
	mov	ax, [vfli.dypLineFli]
	mov	dx, [ywLine]
	sub	dx, ax
	push	dx
	push	ax
	lea	ax,[xwT]
	push	ax
	lea	ax,[ywT]
	push	ax
	cCall	OtlMarkPos,<>
	push	[xwT]		;argument for DrawOtlPat
	push	[ywT]		;argument for DrawOtlPat
	cCall	DrawOtlPat,<>

;		if (!vfli.grpfvisi.w) return; /* for speed */
;		}
	pop	cx	; restore xwPos
	pop	dx	; restore ywPos
	cmp	[vfli.grpfvisiFli],fFalse
	jnz	Ltemp004
	jmp	AVS11
Ltemp004:
AVS007:
;	 grpfvisi = vfli.grpfvisi;
	mov	ah,bptr [vfli.grpfvisiFli]

;	 pchr = &(**vhgrpchr)[0];
;	 bchrCur = 0;
	mov	di,[vhgrpchr]
	mov	di,[di]

;	 pdxp = &vfli.rgdxp[0];
	mov	si,dataoffset [vfli.rgdxpFli]

;	 Assert(pchr->ich == 0);
ifdef DEBUG
	cmp	[di.ichChr],0
	je	AVS01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midDisp1n2
	mov	bx,1016
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
AVS01:
endif ;/* DEBUG */

;	 for (ich = 0;;)
;		 {
	xor	bx,bx
AVS02:

;		 ichNext = pchr->ich;
;		 while (ich < ichNext)
;			 {
	jmp	short AVS05
AVS03:

;/* unlike Mac: spaces are handled in FormatLine, non-breaking-spaces
;   are handled like non-breaking-hyphen */
;			 if (vfli.rgch [ich] == chTab &&
;				 (grpfvisi.fvisiTabs || grpfvisi.fvisiShowAll) )
;                                DrawChVis( hdc, idcbChVisTab, xwPos, ich,
;					ywPos, prcwClip );
	cmp	[bx.vfli.rgchFli],chTab
	jne	AVS04
	errnz	<maskfvisiTabsGrpfvisi - 01h>
	errnz	<maskfvisiShowAllGrpfvisi - 20h>
	test	ah,maskfvisiTabsGrpfvisi+maskfvisiShowAllGrpfvisi
	jz	AVS04
	push	ax	;save grpfvisi
	push	bx	;save ich
	push	cx	;save xwPos
	push	dx	;save ywPos
;	LN_DrawChVis performs:
;	DrawChVis( hdc, ax, cx, bx, dx, prcwClip );
;	ax, bx, cx, and dx are altered
	mov	ax,idcbChVisTab
	call	LN_DrawChVis
	pop	dx	;restore ywPos
	pop	cx	;restore xpPos
	pop	bx	;restore ich
	pop	ax	;restore grpfvisi
AVS04:

;			 xwLast = xwPos;
;			 xwPos += *pdxp++;
;			 ich++;
	mov	[xwLast],cx
	add	cx,[si]
	inc	si
	inc	si
	inc	bx

;			 }
AVS05:
	cmp	bl,[di.ichChr]
	jb	AVS03

;	The following line is done below in the C version
;		 (char *)pchr += chrm;
	mov	al,[di.chrmChr]
	push	ax
	cbw
	add	di,ax
	pop	ax

;		 if ((chrm = pchr->chrm) == chrmTab &&
;			 (ch = ((struct CHRT *)pchr)->ch) != 0)
;			 {
	cmp	al,chrmTab
	jne	AVS07
	cmp	[di.chChrt-cbChrt],0
	je	AVS05

;/* weird chrmTab for visi chars */
;/* NOTE: chNonBreakSpace and chNonReqHyphen generate one of these but
;   are currently handled by character substitution */
;			 if (ch == chNonBreakHyphen &&
;				 (grpfvisi.fvisiCondHyphens || grpfvisi.fvisiShowAll) &&
;				 (xwPos >= prcwClip->xwLeft && xwPos < prcwClip->xwRight))
;			     {
;                            int xwLim = min(xwPos + vfli.rgdxp [ich], prcwClip->xwRight);
;                            
;                            DrawPatternLine( hdc,
;				xwPos, ywLine - vfli.dypAfter - 
;				((vfli.dypLine - vfli.dypAfter - vfli.dypBefore)>>1),
;				xwLim - xwPos, ipatHorzBlack, pltHorz );
;			     }
	push	si	;save pdxp
	cmp	[di.chChrt-cbChrt],chNonBreakHyphen
	jne	AVS06
	errnz	<maskfvisiCondHyphensGrpfvisi - 10h>
	errnz	<maskfvisiShowAllGrpfvisi - 20h>
	test	ah,maskfvisiCondHyphensGrpfvisi+maskfvisiShowAllGrpfvisi
	jz	AVS06
	mov	si,[prcwClip]
	cmp	cx,[si.xwLeftRc]
	jl	AVS06
	cmp	cx,[si.xwRightRc]
	jge	AVS06
	push	ax	;save grpfvisi and chrm
	push	bx	;save ich
	push	cx	;save xwPos
	push	dx	;save ywPos
	shl	bx,1
	mov	bx,[bx.vfli.rgdxpFli]
	add	bx,cx
	cmp	bx,[si.xwRightRc]
	jl	AVS055
	mov	bx,[si.xwRightRc]
AVS055:
;PAUSE
	sub	bx,cx
	push	[hdc]
	push	cx
	mov	ax,[vfli.dypAfterFli]
	mov	cx,[vfli.dypLineFli]
	sub	cx,[vfli.dypBeforeFli]
	sub	cx,ax
	sar	cx,1
	mov	dx,[ywLine]
	sub	dx,ax
	sub	dx,cx
	push	dx
	push	bx
	mov	ax,ipatHorzBlack
	push	ax
	errnz	<pltHorz - 0>
	xor	ax,ax
	push	ax
	cCall	DrawPatternLine,<>
	pop	dx	;restore ywPos
	pop	cx	;restore xwPos
	pop	bx	;restore ich
	pop	ax	;restore grpfvisi and chrm
AVS06:
	pop	si	;restore pdxp

;			 xwPos += *pdxp++;
;			 ich++;
	add	cx,[si]
	inc	si
	inc	si
	inc	bx

;			 }
Ltemp001:
	jmp	short AVS05

;		 else if (chrm == chrmChp)
;			 {
AVS07:
	cmp	al,chrmChp
	jne	AVS08

;			 LoadFcidFull(pchr->fcid);
;			 /* reset the pointer into the heap. */
;			 pchr = (char *)*vhgrpchr + bchrCur;
	push	ax	;save grpfvisi and chrm
	push	bx	;save ich
	push	cx	;save xwPos
	push	dx	;save ywPos
	push	[di.HI_fcidChr-cbChr]
	push	[di.LO_fcidChr-cbChr]
	mov	bx,[vhgrpchr]
	sub	di,[bx]
ifdef DEBUG
	cCall	S_LoadFcidFull,<>
else ;not DEBUG
	cCall	N_LoadFcidFull,<>
endif ;DEBUG
	mov	bx,[vhgrpchr]
	add	di,[bx]
	pop	dx	;restore ywPos
	pop	cx	;restore xwPos
	pop	bx	;restore ich
	pop	ax	;restore grpfvisi and chrm

;			 if ((hps = pchr->chp.hps) != hpsLast)
;/* set ywPos according to new font size */
;				 {
	mov	al,[di.chpChr.hpsChp-cbChr]
	cmp	al,bptr ([hpsLast])
	je	Ltemp001

;				 hpsLast = hps;
	mov	bptr ([hpsLast]),al

;#define DypFromHps(hps) (NMultDiv(hps * (dyaPoint / 2), vfli.dysInch, dxaInch))
;				 dypDot = DypFromHps(hps) / 4;
	push	ax	;save grpfvisi
	push	bx	;save ich
	push	cx	;save xpPos
	push	dx	;save ywPos
	errnz	<dyaPoint - 20>
	mov	ah,dyaPoint/2
	imul	ah
	imul	[vfli.dysInchFli]
	errnz	<dxaInch - 1440>
	add	ax,dxaInch SHR 1
	adc	dx,0
	mov	cx,dxaInch SHL 2
ifdef DEBUG
;	Assert we won't overflow during this divide.
	call	AVS12
endif ;DEBUG
	idiv	cx
	pop	dx	;restore ywPos
	pop	cx	;restore xpPos
	pop	bx	;restore ich

;				 ywPos = ywPos - dypDot + dypDotPrev;
;				 dypDotPrev = dypDot;
	sub	dx,ax
	xchg	ax,[dypDotPrev]
	add	dx,ax
	pop	ax	;restore grpfvisi

;				 }
	jmp	short Ltemp001

;			 }

;		 else if (chrm == chrmFormula)
;			 {
AVS08:
	cmp	al,chrmFormula
	jne	AVS10

;			 xwPos += ((struct CHRF *)pchr)->dxp;
;			 ywPos += ((struct CHRF *)pchr)->dyp;
;			 }
	add	cx,[di.dxpChrf-cbChrf]
	add	dx,[di.dypChrf-cbChrf]
Ltemp005:
	jmp	AVS05

;		 else if (chrm == chrmEnd)
;			 break;
AVS10:
	cmp	al,chrmEnd
	jne	Ltemp005

;	The following line is done above in the assembler version
;		 (char *)pchr += chrm;
;		 }

;/* draw CRJ symbol if needed */
;	 if (vfli.chBreak == chCRJ &&
;		 (grpfvisi.fvisiParaMarks || grpfvisi.fvisiShowAll) )
;	     DrawChVis( hdc, idcbChVisCRJ, xwLast, vfli.ichMac-1, ywPos, prcwClip );
	cmp	[vfli.chBreakFli],chCRJ
	jne	AVS11
	errnz	<maskfvisiParaMarksGrpfvisi - 04h>
	errnz	<maskfvisiShowAllGrpfvisi - 20h>
	test	ah,maskfvisiParaMarksGrpfvisi+maskfvisiShowAllGrpfvisi
	jz	AVS11
;	LN_DrawChVis performs:
;	DrawChVis( hdc, ax, cx, bx, dx, prcwClip );
;	ax, bx, cx, and dx are altered
	errnz	<idcbChVisCRJ>
	xor	ax,ax
	mov	cx,[xwLast]
	mov	bl,[vfli.ichMacFli]
	xor	bh,bh
	dec	bx
	call	LN_DrawChVis
AVS11:
;}
cEnd


ifdef DEBUG
AVS12:
	push	ax
	push	bx
	push	cx
	push	dx
	shr	cx,1
	or	dx,dx
	jns	AVS13
	neg	dx
AVS13:
	cmp	dx,cx
	jb	AVS14
	mov	ax,midDisp1n2
	mov	bx,1019
	cCall	AssertProcForNative,<ax,bx>
AVS14:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG


;-------------------------------------------------------------------------
;	DrawChVis( hdc, idcb, xp, ich, ywLine, prcwClip )
;-------------------------------------------------------------------------
;/* D R A W  C H  V I S */
;DrawChVis( hdc, idcb, xp, ich, ywPos, prcwClip )
;HDC hdc;
;int idcb;
;int xp;
;int ich;
;int ywPos;
;struct RC *prcwClip;
;{
;	struct MDCD *pmdcd;
;	int dxp, dyp;
;	int xpSrc, ypSrc, dxpSrc, dypSrc;
;	struct RC rcwDest, rcT;

;	LN_DrawChVis performs:
;	DrawChVis( hdc, ax, cx, bx, dx, prcwClip );
;	ax, bx, cx, and dx are altered

; %%Function:LN_DrawChVis %%Owner:BRADV
PUBLIC LN_DrawChVis
LN_DrawChVis:

;	if ((pmdcd = PmdcdCacheIdrb( idrbChVis, hdc )) == NULL)
;	   return;
	mov	[ywPos],dx
	push	di	;save caller's di
	push	si	;save caller's si
	mov	[idcb],ax
	push	bx	;save ich
	push	cx	;save xp
	mov	ax,idrbChVis
	cCall	PmdcdCacheIdrb,<ax,[hdc]>
	pop	cx	;restore xp
	pop	bx	;restore ich
	errnz	<NULL>
	or	ax,ax
	je	Ltemp012
	mov	[pmdcd],ax

;	if (idcb == idcbChVisTab)
;		{
;		/* center it. */
;		if (dxpChVisEach > vfli.rgdxp[ich])
;			{
;			dxp = vfli.rgdxp[ich];
;			}
;		else
;			{
;			dxp = dxpChVisEach;
;			xp += (vfli.rgdxp[ich] - dxpChVisEach) / 2;
;			}
;		}
;	else /* CRJ */
;		{
;		dxp = vfli.rgdxp[ich];
;		}
	shl	bx,1
	mov	bx,[bx.vfli.rgdxpFli]
	mov	dx,bx
	cmp	[idcb],idcbChVisTab
	jne	LN_DCV01
	sub	bx,dxpChVisEach
	jl	LN_DCV01
	mov	dx,dxpChVisEach
	shr	bx,1
	add	cx,bx
LN_DCV01:


;	dyp = min( dypChVis, vfli.dypLine - vfli.dypBase - vfli.dypBefore );

	mov	ax,[vfli.dypLineFli]
	sub	ax,[vfli.dypBaseFli]
	sub	ax,[vfli.dypBeforeFli]
	cmp	ax,dypChVis
	jle	LN_DCV02
	mov	ax,dypChVis
LN_DCV02:
	
;/* clipping part */
;	PrcSet(&rcT, xp, ywPos - dyp, dxp, dyp);
;	DrcToRc(&rcT, &rcT);
	;Assembler note: here we are performing PrcSet and DrcToRc in-line
	;simultaneously.
	; dx = dxp, ax = dyp, cx = xp
	push	dx	;save dxp
	push	ax	;save dyp
	push	ds
	pop	es
	lea	si,[rcwDest]
	lea	di,[rcT]
	push	di	;argument for FSectRc
	push	[prcwClip]   ;argument for FSectRc
	push	si	;argument for FSectRc
	xchg	ax,cx
	errnz	<(xpLeftRc) - 0>
	stosw
	xchg	ax,bx
	mov	ax,[ywPos]
	mov	si,ax
	sub	ax,cx
	errnz	<(ypTopRc) - 2>
	stosw
	xchg	ax,bx
	add	ax,dx
	errnz	<(xpRightRc) - 4>
	stosw
	xchg	ax,si
	errnz	<(ypBottomRc) - 6>
	stosw

;	if (!FSectRc(&rcT, prcwClip, &rcwDest /*rcResult*/))
;		return;
	cCall	FSectRc,<>
	or	ax,ax
	pop	ax	;restore dyp
	pop	dx	;restore dxp
Ltemp012:
	je	LN_DCV04

;	dxpSrc = NMultDiv(dxpChVisEach, rcwDest.xwRight-rcwDest.xwLeft, dxp);
;	dypSrc = NMultDiv(dypChVis, rcwDest.ywBottom-rcwDest.ywTop, dyp);
;	xpSrc = idcb * dxpChVisEach;
;	if (rcwDest.xpLeft != rcT.xpLeft)
;	     xpSrc += dxpChVisEach - dxpSrc;
;
;	StretchBlt( hdc, rcwDest.xwLeft, rcwDest.ywTop,
;	     rcwDest.xwRight - rcwDest.xwLeft, rcwDest.ywBottom - rcwDest.ywTop,
;	     pmdcd->hdc,		 /* hdcSrc */
;	     xpSrc, 0,	/* xpSrc,ypSrc */
;	     dxpSrc, dypSrc,
;	     vsci.dcibMemDC.ropBitBlt );
	push	[hdc]	    ;argument for StretchBlt
	mov	bx,[rcwDest.xwLeftRc]
	push	bx	    ;argument for StretchBlt
	mov	cx,[rcwDest.ywTopRc]
	push	cx	    ;argument for StretchBlt
	mov	si,[rcwDest.xwRightRc]
	sub	si,bx
	push	si	    ;argument for StretchBlt
	mov	di,[rcwDest.ywBottomRc]
	sub	di,cx
	push	di	    ;argument for StretchBlt
	mov	bx,dxpChVisEach
	mov	cx,dypChVis
	push	bx	    ;argument for first NMultDiv
	push	si	    ;argument for first NMultDiv
	push	dx	    ;argument for first NMultDiv
	push	cx	    ;argument for second NMultDiv
	push	di	    ;argument for second NMultDiv
	push	ax	    ;argument for second NMultDiv
	cCall	NMultDiv,<>
	xchg	ax,di	    ;dypSrc in di
	cCall	NMultDiv,<>
	xchg	ax,si	    ;dxpSrc in si
	mov	bx,[pmdcd]
	push	[bx.hdcMdcd]	;argument for StretchBlt
	;Assembler note: these lines are performed above in the C source
;	xpSrc = idcb * dxpChVisEach;
;	if (rcwDest.xpLeft != rcT.xpLeft)
;	     /* clipped on the left edge, show right edge of bitmap */
;	     xpSrc += dxpChVisEach - dxpSrc;
	mov	ax,dxpChVisEach
	mul	[idcb]
	mov	bx,[rcwDest.xpLeftRc]
	cmp	bx,[rcT.xpLeftRc]
	je	LN_DCV03
	add	ax,dxpChVisEach
	sub	ax,si
LN_DCV03:
	push	ax	    ;argument for StretchBlt
	xor	ax,ax
	push	ax	    ;argument for StretchBlt
	push	si	    ;argument for StretchBlt
	push	di	    ;argument for StretchBlt
	push	[vsci.dcibMemDCSci.HI_ropBitBltDcib]
	push	[vsci.dcibMemDCSci.LO_ropBitBltDcib]
ifdef DEBUG
	cCall	StretchBltFP,<>
else ;!DEBUG
ifdef HYBRID
	cCall	StretchBltPR,<>
else ;!HYBRID
	cCall	StretchBlt,<>
endif ;!HYBRID
endif ;!DEBUG
LN_DCV04:
	pop	si	;restore caller's si
	pop	di	;restore caller's di
;}
	ret

;			 Assert(dx == 0);
ifdef DEBUG
LN_DCV05:
	or	dx,dx
	je	LN_DCV06
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midDisp1n2
	mov	bx,1018
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LN_DCV06:
	ret
endif ;/* DEBUG */


;-------------------------------------------------------------------------
;	FScrollOK( ww )
;-------------------------------------------------------------------------
;/* F  S C R O L L  O  K  */
;/* DypScroll may not be called if this proc returns false meaning not
;all of the window is visible either by its position on the screen or
;because it is not on the top. */
;NATIVE BOOL FScrollOK(ww)
;int ww;
;{
;	extern int	vwwClipboard;
;	extern HWND	vhwndApp;
;	extern int	vfFocus;
;	struct RC	rcwDisp, rcScreen, rcInter;
;	struct WWD	*pwwd = PwwdWw(ww);

;/* Check 1: is any of the window off the screen edge? */

; %%Function:FScrollOK %%Owner:BRADV
cProc	FScrollOK,<PUBLIC,FAR>,<si,di>
	ParmW	ww

	LocalV	rcwDisp, cbRcMin
	LocalV	rcScreen, cbRcMin
	LocalV	rcInter, cbRcMin

cBegin

	cCall	N_PwwdWw,<[ww]>
	xchg	ax,bx

;	FreezeHp();
ifdef	DEBUG
	call	LN_FreezeHp
endif ;DEBUG

;	Assert( pwwd->hdc != NULL );
ifdef DEBUG
	cmp	[bx.hdcWwd],NULL
	jne	FSO01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midDisp1n2
	mov	bx,1060
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FSO01:
endif ;/* DEBUG */

	push	ds
	pop	es

	
;	rcwDisp = pwwd->rcwDisp;
	lea	si,[bx.rcwDispWwd]
	lea	di,[rcwDisp]
	errnz	<cbRcMin - 8>
	movsw
	movsw
	movsw
	movsw

; if (GetClipBox(pwwd->hdc, (LPRECT)&rcScreen) != SIMPLEREGION
	push	[bx.hdcWwd]
	lea	bx,[rcScreen]
	push	ds
	push	bx
	cCall	GetClipBox,<>
	cmp	ax,SIMPLEREGION
ifdef	DEBUG
	jne	FSO02
else ;not DEBUG
	jne	FSO06
endif ;DEBUG

; || !FSectRc( &rcwDisp, &rcScreen, &rcInter ) 
	lea	di,[rcwDisp]
	push	di
	lea	ax,[rcScreen]
	push	ax
	lea	si,[rcInter]
	push	si
	cCall	FSectRc,<>
	or	ax,ax
ifdef	DEBUG
	je	FSO02
else ;not DEBUG
	je	FSO06
endif ;DEBUG

; || FNeRgw(&rcInter, &rcwDisp, cwRC))
;			{
;			MeltHp();
;			return fFalse;
;			}
	push	ds
	pop	es
	mov	cx,cbRcMin/2
	rep	cmpsw
ifdef	DEBUG
	je	FSO03
FSO02:
ifdef	DEBUG
	call	LN_MeltHp
endif ;DEBUG
	jmp	short FSO06
else ;not DEBUG
	jne	FSO06
endif ;DEBUG
FSO03:

;	/* Check 2: is any window covering this ww? */
;	MeltHp();
ifdef	DEBUG
	call	LN_MeltHp
endif ;DEBUG

;	if (ww != wwCur)
;		{
;		return fFalse;
;		}
;	else
	mov	ax,[wwCur]
	cmp	[ww],ax
	jne	FSO06

;		{
;		HWND	hwnd;
;
;		hwnd = GetWindow(vhwndApp, GW_HWNDFIRST);
	errnz	<GW_HWNDFIRST - 0>
	xor	di,di
	cCall	GetWindow,<[vhwndApp], di>
	errnz	<GW_HWNDNEXT - 2>
	inc	di
	inc	di

;		while (hwnd != NULL && !IsWindowVisible(hwnd))
;			{
FSO04:
	errnz	<NULL>
	or	ax,ax
	je	FSO05
	xchg	ax,si
	cCall	IsWindowVisible,<si>
	or	ax,ax
	xchg	ax,si
	jne	FSO05

;			hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	cCall	GetWindow,<ax, di>

;			}
	jmp	short FSO04

FSO05:
;		return (vhwndApp == hwnd);
	cmp	ax,[vhwndApp]
	mov	ax,fTrue
	jne	FSO06
;		}

	db	03Dh	;turns next "xor ax,ax" into "cmp ax,immediate"
FSO06:
	errnz	<fFalse>
	xor	ax,ax
;}
cEnd

;End of FScrollOK


;-------------------------------------------------------------------------
;	DrawInsertLine( psel )
;-------------------------------------------------------------------------
;NATIVE DrawInsertLine(psel)
;struct SEL *psel;
;{
;	struct WWD *pwwd = PwwdWw(psel->ww);
;	struct RC rcIns, rcT;
;	HBRUSH	     hbrOld;
;	int xwLeftT;
;	int plt;

; %%Function:N_DrawInsertLine %%Owner:BRADV
cProc	N_DrawInsertLine,<PUBLIC,FAR>,<si,di>
	ParmW	psel

	LocalV	rcIns,cbRcMin

cBegin

	mov	si,[psel]
	cCall	N_PwwdWw,<[si.wwSel]>
	xchg	ax,bx

;	Assert( pwwd->hdc != NULL );
ifdef DEBUG
	cmp	[bx.hdcWwd],NULL
	jne	DIL01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midDisp1n2
	mov	bx,1061
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
DIL01:
endif ;/* DEBUG */

;	SetRect( (LPRECT) &rcIns, psel->xw, psel->yw - psel->dyp + 1,
;				  psel->xw + (vsci.dxpBorder << 1),
;				  psel->yw - vsci.dypBorder + 1 );
	push	ds
	pop	es
	lea	di,[rcIns]
	mov	ax,[si.xwSel]
	stosw
	xchg	ax,dx
	mov	ax,[si.ywSel]
	inc	ax
	mov	cx,ax
	sub	ax,[si.dypSel]
	stosw
	xchg	ax,dx
	mov	dx,[vsci.dxpBorderSci]
	sub	cx,dx
	shl	dx,1
	add	ax,dx
	stosw
	xchg	ax,cx
	stosw

;	if (!FSectRc( &rcIns, &psel->rcwClip, &rcIns))
;		{
	lea	di,[si.rcwClipSel]
        push    bx      ;save pwwd
	lea	bx,[rcIns]
	cCall	FSectRc,<bx, di, bx>
	pop	bx	;restore pwwd
	or	ax,ax
	jne	DIL02

;		selCur.fOn = fFalse;
	mov	bptr ([selCur.fOnSel]),fFalse

;		return;
	jmp	short DIL07

;		}
DIL02:

;/* following line assumes we do not need to take responsibility for
;   clipping the insert line on the right */

;	hbrOld = SelectObject(pwwd->hdc, vsci.hbrText);
	mov	di,[bx.hdcWwd]
	push	bx	;save pwwd
ifdef DEBUG
	cCall	SelectObjectFP,<di, [vsci.hbrTextSci]>
else ;!DEBUG
ifdef HYBRID
	cCall	SelectObjectPR,<di, [vsci.hbrTextSci]>
else ;!HYBRID
	cCall	SelectObject,<di, [vsci.hbrTextSci]>
endif ;!HYBRID
endif ;!DEBUG
	pop	bx	;restore pwwd
	push	ax	;save hbrOld

;	xwLeftT = rcIns.xpLeft - 1;
	mov	ax,[rcIns.xpLeftRc]
	dec	ax

;	plt = pltVert | pltInvert | pltDouble;
	mov	cx,pltVert OR pltInvert OR pltDouble

;	if (pwwd->fPageView && xwLeftT < psel->rcwClip.xwLeft)
;		{
	test	[bx.fPageViewWwd],maskFPageViewWwd
	je	DIL025
	cmp	ax,[si.rcwClipSel.xwLeftRc]
	jg	DIL025

;		struct RC rcwPage;
;		RceToRcw(pwwd, &pwwd->rcePage, &rcwPage);
	;***Begin in-line RceToRcw for prcw->xwLeft
;HANDNATIVE RceToRcw(pwwd, prce, prcw)
;struct WWD *pwwd;
;struct RC *prce;
;struct RC *prcw;
;{
;	 int dxeToXw = pwwd->xwMin;
;	 int dyeToYw = pwwd->ywMin;
;
;	 prcw->xwLeft = prce->xeLeft + dxeToXw;
;	 prcw->xwRight = prce->xeRight + dxeToXw;
;	 prcw->ywTop = prce->yeTop + dyeToYw;
;	 prcw->ywBottom = prce->yeBottom + dyeToYw;
;}
	mov	dx,[bx.rcePageWwd.xeLeftRc]
	add	dx,[bx.xwMinWwd]
	;***End in-line RceToRcw for prcw->xwLeft

;		if (xwLeftT < rcwPage.xwLeft)
;			{
;/* make ip thin only if on paper edge */
;			plt = pltVert | pltInvert;
;			xwLeftT = rcwPage.xwLeft;
;			}
;		}
;
	cmp	ax,dx
	jge	DIL025
	errnz	<pltVert - 1>
	errnz	<pltInvert - 4>
	errnz	<pltDouble - 8>
	mov	cl,pltVert OR pltInvert
	xchg	ax,dx
DIL025:	

;        DrawPatternLine( pwwd->hdc, xwLeftT, rcIns.ypTop,
;		rcIns.ypBottom - rcIns.ypTop,
;		psel->pa == paDotted ? ipatVertGray : ipatVertBlack,
;		plt );
	push	di
	push	ax
	mov	bx,[rcIns.ypTopRc]
	push	bx
	mov	ax,[rcIns.ypBottomRc]
	sub	ax,bx
	push	ax
	errnz	<ipatVertGray - 0>
	errnz	<ipatVertBlack - 1>
	errnz	<paDotted - 1>
	mov	ax,paDotted
	cmp	[si.paSel],ax
	jne	DIL03
	dec	ax
DIL03:
	push	ax
	push	cx
	cCall	DrawPatternLine,<>

;	if (hbrOld != hNil)
;	    SelectObject(pwwd->hdc, hbrOld);
	pop	cx	;restore hbrOld
	errnz	<hNil>
	jcxz	DIL04
ifdef DEBUG
	cCall	SelectObjectFP,<di, cx>
else ;!DEBUG
ifdef HYBRID
	cCall	SelectObjectPR,<di, cx>
else ;!HYBRID
	cCall	SelectObject,<di, cx>
endif ;!HYBRID
endif ;!DEBUG
DIL04:

;	psel->fOn = 1 - psel->fOn;
	xor	bptr ([si.fOnSel]),1

;	Scribble(ispInsertLine, psel->fOn ? '|' : ' ');
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,ispInsertLine
	mov	bx,07Ch
	cmp	[si.fOnSel],fFalse
	jne	DIL05
	mov	bx,020h
DIL05:
	cmp	[vdbs.grpfScribbleDbs],0
	jz	DIL06
	cCall	ScribbleProc,<ax,bx>
DIL06:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

DIL07:
;}
cEnd

;End of DrawInsertLine

ifdef NOTUSED
;-------------------------------------------------------------------------
;	EraseInDr( hdc, prcw, hpldr, idr, fPartialErase )
;-------------------------------------------------------------------------
;/* E R A S E	I N   D R */
;NATIVE EraseInDr(hdc, prcw, hpldr, idr, fPartialErase)
;HDC hdc;
;struct RC *prcw;	 /* the portion of the dr you want to erase */
;struct PLDR **hpldr;
;int idr;
;int fPartialErase;	 /* ENABLES partial erasing; vpref overrides */
;{
;	struct DR *pdr;
;	struct RC rcwErase;

; %%Function:N_EraseInDr %%Owner:BRADV
cProc	N_EraseInDr,<PUBLIC,FAR>,<si,di>
	ParmW	hdc
	ParmW	prcw
	ParmW	hpldr
	ParmW	idr
	ParmW	fPartialErase

	LocalV	rcwErase,cbRcMin
	LocalV	drfFetch,cbDrfMin

cBegin

	lea	ax,[drfFetch]
ifdef DEBUG
	cCall	S_PdrFetchAndFree,<[hpldr],[idr],ax>
else ;not DEBUG
	cCall	N_PdrFetchAndFree,<[hpldr],[idr],ax>
endif ;DEBUG
	xchg	ax,bx

	push	ds
	pop	es
	mov	si,[prcw]
	lea	di,[rcwErase]
	errnz	<cbRcMin - 8>
	movsw
	movsw
	movsw
	movsw

;	if (fPartialErase && FDrawPageDrsPref)
;		{
	cmp	[fPartialErase],fFalse
	je	EID01

;#define FDrawPageDrsPref   (vpref.grpfvisi.fDrawPageDrs || vpref.grpfvisi.ShowAll)
	errnz	<maskfDrawPageDrsGrpfvisi - 04000h>
	errnz	<maskfvisiShowAllGrpfvisi - 00020h>
	test	[vpref.grpfvisiPref],maskfDrawPageDrsGrpfvisi+maskfvisiShowAllGrpfvisi
	je	EID01

;		/* erase in pieces to avoid border flicker; rcw spans the
;		   dr exactly from left to right */
	;Assembler note: rcwErase = rcw is done above
;		rcwErase = *prcw;
;		pdr = PdrFetchAndFree(hpldr, idr, &drfFetch);
	mov	si,[vsci.dypBorderSci]
	mov	di,[rcwErase.xpRightRc]

;		rcwErase.ywBottom -= vsci.dypBorder;
;		rcwErase.xpRight = rcwErase.xpLeft + pdr->dxpOutLeft;
;		PatBltRc(hdc, &rcwErase, vsci.ropErase);
	sub	[rcwErase.ywBottomRc],si
	mov	dx,[rcwErase.xpLeftRc]
	add	dx,[bx.dxpOutLeftDr]
	mov	[rcwErase.xpRightRc],dx
	;EID02 performs PatBltRc(di, si, vsci.ropErase);
	;ax, cx, dx are altered.
	call	EID02

;		rcwErase.xpLeft = rcwErase.xpRight + vsci.dxpBorder;
;		rcwErase.xpRight = prcw->xpRight - pdr->dxpOutRight - vsci.dxpBorder;
;		PatBltRc(hdc, &rcwErase, vsci.ropErase);
	mov	dx,di
	sub	dx,[bx.dxpOutRightDr]
	sub	dx,si
	xchg	dx,[rcwErase.xpRightRc]
	add	dx,si
	mov	[rcwErase.xpLeftRc],dx
	;EID02 performs PatBltRc(di, si, vsci.ropErase);
	;ax, cx, dx are altered.
	call	EID02

;		rcwErase.xpLeft = rcwErase.xpRight + vsci.dxpBorder;
;		rcwErase.xpRight = prcw->xpRight;
;		PatBltRc(hdc, &rcwErase, vsci.ropErase);
	xchg	di,[rcwErase.xpRightRc]
	add	di,si
	mov	[rcwErase.xpLeftRc],di
 EID01:
	;EID02 performs PatBltRc(di, si, vsci.ropErase);
	;ax, cx, dx are altered.
	call	EID02

;		}
;	else
	;Assembler note: this is performed at EID01
;		PatBltRc(hdc, prcw, vsci.ropErase);
;}
cEnd

;End of EraseInDr

	;EID02 performs PatBltRc(hdc, &rcwErase, vsci.ropErase);
	;ax, cx, dx are altered.
EID02:
	push	bx	;save pdr
	push	[hdc]
	lea	ax,[rcwErase]
	push	ax
	push	[vsci.HI_ropEraseSci]
	push	[vsci.LO_ropEraseSci]
	push	cs
	call	near ptr PatBltRc
	pop	bx	;restore pdr
	ret
endif ; NOTUSED


ifdef	DEBUG
LN_FreezeHp:
;#define FreezeHp()		 (cHpFreeze++?0:LockHeap(sbDds))
	cmp	[cHpFreeze],0
	jne	LN_FH01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,sbDds
	cCall	LockHeap,<ax>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LN_FH01:
	inc	[cHpFreeze]
	ret
endif ;DEBUG


ifdef	DEBUG
LN_MeltHp:
;#define MeltHp()		 (--cHpFreeze?0:UnlockHeap(sbDds))
	dec	[cHpFreeze]
	jne	LN_MH01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,sbDds
	cCall	UnlockHeap,<ax>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LN_MH01:
	ret
endif ;DEBUG

sEnd	disp1_PCODE
        end
