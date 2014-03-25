	include w2.inc
	include noxport.inc
        include consts.inc
	include structs.inc

createSeg	fetchtb_PCODE,fetchtbn,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midFetchtbn	equ 27		; module ID, for native asserts
endif

;ifndef NotNatPause
;macro	 PAUSE
;int 3
;endm
;else
;macro	 PAUSE
;endm
;endif

; EXTERNAL FUNCTIONS
externFP	<FInCa>
externFP	<N_WidthHeightFromBrc>

ifdef	DEBUG
externFP	<AssertProcForNative>
externFP	<S_WidthHeightFromBrc>
externFP	<S_ItcGetTcx>
endif	; DEBUG

sBegin	data

; 
; /* E X T E R N A L S */
; 
externW 	vhgrpchr	; extern struct CHR	  **vhgrpchr;
externW		vtcxs		; extern TCXS vtcxs;
externW		vfti		; extern FTI vfti;
externW		caTap		; extern CA caTap;
externW		vtapFetch	; extern TAP vtapFetch;

sEnd    data


; CODE SEGMENT _FETCHTBN

sBegin	fetchtbn
	assumes cs,fetchtbn
        assumes ds,dgroup
        assumes ss,dgroup
; %%Function:ItcGetTcxCache %%Owner:tomsax
cProc	N_ItcGetTcxCache,<PUBLIC,FAR>,<si,di>
	ParmW	ww
	ParmW	doc
	ParmD	cp
	ParmW	ptap
	ParmW	itc
	ParmW	ptcx

	LocalW	fCache
	LocalW	itcNext
	LocalW	itcxc

cBegin

	; int 3 ; PAUSE
; fCache = ww != wwNil;
	errnz	wwNil
	mov	ax,[ww]
	mov	[fCache],ax
; if (!FInCa(doc, cp, &vtcxs.ca)
	push	[doc]
	push	[SEG_cp]
	push	[OFF_cp]
	mov	ax,dataoffset [vtcxs.caTcxs]
	push	ax
	cCall	FInCa,<>

	xchg	ax,cx
	jcxz	LNotInCache

; || vtcxs.grpfTap != ptap->grpfTap
	; int 3 ; PAUSE
	mov	bx,dataoffset [vtcxs]
	mov	si,[ptap]
	mov	ax,[bx.grpfTapTcxs]
	cmp	ax,[si.grpfTap]
	jne	LNotInCache

; WIN(|| vtcxs.dxpInch != ww))
	mov	ax,[vfti.dxpInchFti]
	cmp	ax,[bx.dxpInchTcxs]
	je 	LInCache
LNotInCache:
; if (!fCache
	; int 3 ; PAUSE
	mov	cx,[fCache]
	jcxz	LCalcTcx
; || !FInCa(doc, cp, &caTap)
;	fCache = fFalse;
	; int 3 ; PAUSE
	push	[doc]
	push	[SEG_cp]
	push	[OFF_cp]
	mov	ax,dataoffset [caTap]
	push	ax
	cCall	FInCa,<>
	xchg	ax,cx
	mov	[fCache],cx	; set fCache to FInCa(...)
	jcxz	LCalcTcx
; else
; for (itcxc = 0; itcxc < itcxcMax; itcxc++)
;	vtcxs.rgtcxc[itcxc].fValid = fFalse;
	; int 3 ; PAUSE
	push	ds
	pop	es
	mov	bx,dataoffset [vtcxs]

	mov	cx,itcxcMax	; count down for efficiency
	mov	di,dataoffset [vtcxs.rgtcxcTcxs.fValidTcxc]
	xor	ax,ax
LClearFValid:
	stosw
	add	di,cbTcxcMin-2
	loop	LClearFValid

; vtcxs.ca = caTap;
	mov	si,dataoffset [caTap]
	mov	di,dataoffset [vtcxs.caTcxs]
errnz	<cbCaMin and 1>
	mov	cx,cbCaMin SHR 1
	rep	movsw

; vtcxs.grpfTap = vtapFetch.grpfTap;
	mov	ax,[vtapFetch.grpfTap]
	mov	[bx.grpfTapTcxs],ax

; Win(vtcxs.dxpInch = vfti.dxpInch;)
	mov	ax,[vfti.dxpInchFti]
	mov	[bx.dxpInchTcxs],ax
	jmp	short LCalcTcx

LInCache:
	; int 3 ; PAUSE
; else if (itc < itcxcMax)
	cmp	[itc],itcxcMax
	jge	LCalcTcx
; && vtcxs.rgtcxc[itc].fValid)
	lea	si,[bx.rgtcxcTcxs]
	mov	ax,cbTcxcMin
	mul 	[itc]
	add	si,ax
	mov	cx,[si.fValidTcxc]
	jcxz	LCalcTcx

	; int 3 ; PAUSE
	; blt(&vtcxs.rgtcxc[itc], ptcx, sizeof(TCX));
	mov	ax,[si.itcNextTcxc]	; return value = vtcxs.rgtcxc[itc].itcNext
	add	si,tcxTcxc
	mov	di,[ptcx]
	errnz	<cbTcxMin and 1>
	mov	cx,cbTcxMin SHR 1
	push	ds
	pop	es
	rep	movsw
	jmp	short LExitIGC

LCalcTcx:
	; int 3 ; PAUSE
	mov	di,[ww]
	mov	si,[ptcx]
	push	di
	push	[ptap]
	push	[itc]
	push	si
ifdef DEBUG
	cCall	S_ItcGetTcx,<>
else ;not DEBUG
	push	cs
	call	near ptr N_ItcGetTcx
endif ;DEBUG
	; ax = itcNext
	; if (fCache
	; int 3	; PAUSE
	mov	cx,[fCache]
	jcxz	LExitIGC
	; int 3	; PAUSE
	; && itc < itcxcMax)
	cmp	[itc],itcxcMax
	jge	LExitIGC

	; int 3 ; PAUSE
	push	ax	; save itcNext
	; blt(ptcx, &vtcxs.rgtcxc[itc].tcx, sizeof(TCX);
	; si = ptcx from above
	mov	di,dataoffset [vtcxs.rgtcxcTcxs.tcxTcxc]
	mov	ax,cbTcxcMin
	mul	[itc]
	add	di,ax
	; before the blt, do:
	;  vtcxs.rgtcxc[itc].itcNext = itcNext;
	;  vtcxs.rgtcxc[itc].itcNext = fTrue;
	pop	ax
	mov	[di.itcNextTcxc-(tcxTcxc)],ax
	mov	[di.fValidTcxc-(tcxTcxc)],fTrue
	errnz	<cbTcxMin and 1>
	mov	cx,cbTcxMin SHR 1
	push	ds
	pop	es
	rep	movsw

LExitIGC:
	; return ax = itcNext;

cEnd

;End of ItcGetTcxCache

; #define	dxpTtpDr	(8*dxpBorderFti)
dxpTtpDrFti	equ	8

; NATIVE ItcGetTcx ( ww, ptap, itc, ptcx )
; struct TAP *ptap;
; struct TCX *ptcx;
; {
; struct WWD *pwwd;
; int itcMac;
; int xaFirst, xaLim;
; int itcNext;
; int dxa, dxp;	; native note: these are done with registers
; %%Function:ItcGetTcx %%Owner:tomsax
cProc	N_ItcGetTcx,<PUBLIC,FAR>,<si,di>
	ParmW	ww
	ParmW	ptap
	ParmW	itc
	ParmW	ptcx

	LocalW	pwwd
	LocalW	itcMac
	LocalW	xaFirst
	LocalW	xaLim
	LocalW	itcNext
ifdef	DEBUG
	LocalW	fRestarted
endif	; DEBUG

cBegin
	; int 3 ; PAUSE
; Debug ( BOOL fRestarted = fFalse; )
ifdef	DEBUG
	mov	[fRestarted],0
endif	; DEBUG

; LRestart:
; Assert ( 0 <= itc && itc <= ptap->itcMac );
	mov	bx,[ptap]
	mov	si,[ptcx]
LRestart:
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[itc]
	cmp	ax,0
	jl	IG010
	cmp	ax,[bx.itcMacTap]
	jle	IG020
IG010:
	mov	ax,midFetchtbn
	mov	bx,241
	cCall	AssertProcForNative,<ax,bx>
IG020:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG

; itcMac = ptap->itcMac;
	mov	ax,[bx.itcMacTap]
	mov	[itcMac],ax
; xaFirst = ptap->rgdxaCenter[itc];
; itcNext = itc + 1;
	lea	di,[bx.rgdxaCenterTap]
	mov	cx,[itc]
	mov	[itcNext],cx
	inc	[itcNext]
	shl	cx,1
	add	di,cx
	mov	ax,[di]
	mov	[xaFirst],ax

; if (itc == itcMac)
; {
	; int 3 ; PAUSE
	mov	di,[itc]
	cmp	di,[itcMac]
	jne	LCheckFirstMerged
; /* handle the fTtp cell */
; Assert(itc > 0);
	; int 3 ; PAUSE
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	or	dx,dx
	jge	IG030
	mov	ax,midFetchtbn
	mov	bx,267
	cCall	AssertProcForNative,<ax,bx>
IG030:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG
; ptcx->brcLeft = ptap->rgtc[itc-1].brcRight;
; ptcx->brcRight =
; ptcx->brcTop =
; ptcx->brcBottom = brcNil;
;
	; bx = ptap, di = itc
	errnz	<rgbrcTcx-brcLeftTcx>
	errnz	<brcLeftTcx+2-brcTopTcx>
	errnz	<brcTopTcx+2-brcBottomTcx>
	errnz	<brcBottomTcx+2-brcRightTcx>
	mov	ax,cbTcMin
	mul	di
	xchg	ax,di
	mov	ax,[bx.di.rgtcTap.brcRightTc-cbTcMin]
	push	ds
	pop	es
	lea	di,[si.rgbrcTcx]
	stosw
	mov	ax,brcNil
	stosw
	stosw
	stosw
; if (ww == wwNil)
;   return itcNext;
	errnz	<wwNil>
	mov	cx,[ww]
	jcxz	LTempExitIG
; xaLim = xaFirst+DxaFromDxp(PwwdWw(ww),dxpTtpDr+1+DxFromBrc(ptcx->brcLeft,fTrue)/2);
;copy macro defn here
	mov	ax,[si.brcLeftTcx]
	call	near ptr Loc_DxFromBrcFTrue
	shr	ax,1	; DxFromBrc/2

	mov	dx,[vfti.dxpBorderFti]
	errnz	<dxpTtpDrFti-8>
	mov	cl,3
	shl	dx,cl
	add	ax,dx
	inc	ax
	call	near ptr Loc_DxaFromDxp
	add	ax,[xaFirst]
	mov	[xaLim],ax
; goto LCalcCellSize; /* skip around border calculation */
; }
	jmp	LCalcCellSize
; else if ( ptap->rgtc[itc].fFirstMerged )
; {
LCheckFirstMerged:
	; bx = ptap, di = itc
	mov	ax,cbTcMin
	mul	di
	xchg	ax,di
	test	[bx.di.rgtcTap.fFirstMergedTc],maskFFirstMergedTc
	je	LCheckMerged
; /* the first in a merged series */
; while ( itcNext < itcMac && ptap->rgtc[itcNext].fMerged )
;   itcNext++;
; }
;
; native note: the next clever little block assumes that itcNext == itc+1
; and that itc < itcMac
	; int 3 ; PAUSE
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[itc]
	inc	ax
	cmp	ax,[itcNext]
	jnz	IG040

	mov	ax,[itc]
	cmp	ax,[itcMac]
	jl	IG050
IG040:
	mov	ax,midFetchtbn
	mov	bx,392
	cCall	AssertProcForNative,<ax,bx>
IG050:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG
	; bx = ptap, di = itc*cbTcMin
	lea	di,[bx.di.rgtcTap.fMergedTc]
	mov	cx,[itcMac]
	sub	cx,[itc]
	; cx == 0 impossible by above assert
ifdef	DEBUG
	jle	IG040	; make damn sure...
endif	; DEBUG
	push	ds
	pop	es
LWhileMerged:
	; note this loop can read rgtc[itcMac], but falls through on
	; cx == 0 regardless of the random value read.
	add	di,cbTcMin
	test	bptr [di],maskFMergedTc
	loopne	LWhileMerged
	sub	cx,[itcMac]
	neg	cx
	mov	[itcNext],cx
	jmp	short LSetXaLim
; native note: this allows guys in the neighborhood to do
; short jumps to LRetIG
LTempExitIG:
	jmp	LRetIG
; else if ( ptap->rgtc[itc].fMerged )
LCheckMerged:
	; bx = ptap, di = itc*cbTcMin
	test	[bx.di.rgtcTap.fMergedTc],maskFMergedTc
	jz	LSetXaLim
; {
; /* a cell in the middle of merged series, doesn't get displayed */
; xaLim = xaFirst;
	; int 3 ; PAUSE
	mov	ax,[xaFirst]
	mov	[xaLim],ax
; ptcx->brcTop = ptcx->brcBottom = ptcx->brcLeft = ptcx->brcRight = brcNone;
	errnz	<rgbrcTcx-brcLeftTcx>
	errnz	<brcLeftTcx+2-brcTopTcx>
	errnz	<brcTopTcx+2-brcBottomTcx>
	errnz	<brcBottomTcx+2-brcRightTcx>
	lea	di,[si.rgbrcTcx]
	mov	ax,brcNone
	push	ds
	pop	es
	stosw
	stosw
	stosw
	stosw
	jmp	short LCalcCellSize
; goto LCalcCellSize; /* skip around border calculation */
; }

; xaLim = ptap->rgdxaCenter[itcNext];
LSetXaLim:
	; bx = ptap
	; int 3 ; PAUSE
	mov	di,[itcNext]
	shl	di,1
	mov	ax,[bx.di.rgdxaCenterTap]
	mov	[xaLim],ax

; /* Get the various border codes.
; /**/
	mov	ax,cbTcMin
	mul	[itc]
	xchg	ax,di
; ptcx->brcTop = ptap->rgtc[itc].brcTop;
	; bx = ptap, di = itcNext*cbTcMin
	mov	cx,[bx.di.rgtcTap.brcTopTc]
	mov	[si.brcTopTcx],cx
; ptcx->brcBottom = ptap->rgtc[itc].brcBottom;
	; bx =ptap, si = ptcx, di = itc*cbTcMin
	mov	cx,[bx.di.rgtcTap.brcBottomTc]
	mov	[si.brcBottomTcx],cx
; if ( (ptcx->brcLeft = ptap->rgtc[itc].brcLeft) == brcNone)
;   if (itc > 0)
	mov	cx,[bx.di.rgtcTap.brcLeftTc]
	errnz	<brcNone>
	or	di,di
	jz	LSetBrcLeft
	or	cx,cx
	jnz	LSetBrcLeft
	; di = itc*cbTcMin
;     ptcx->brcLeft = ptap->rgtc[itc-1].brcRight;
	mov	cx,[bx.di.rgtcTap.brcRightTc-cbTcMin]
LSetBrcLeft:
	mov	[si.brcLeftTcx],cx

; if (itcNext >= itcMac
	mov	ax,cbTcMin
	mov	di,[itcNext]
	mul	di
	xchg	ax,di

	; assume we want rgtc[itcNext-1].brcRight
	mov	dx,[bx.di.rgtcTap.brcRightTc-cbTcMin]
	cmp	ax,[itcMac]
	jge	LSetBrcRight	; assumption wins
; || (ptcx->brcRight = ptap->rgtc[itcNext].brcLeft) == brcNone)
	mov	cx,[bx.di.rgtcTap.brcLeftTc]
	jcxz	LSetBrcRight
	; int 3 ; PAUSE
	mov	dx,cx	; assumption failed, replace with rgtc[itcNext].brcLeft
;   ptcx->brcRight = ptap->rgtc[itcNext-1].brcRight;
LSetBrcRight:
	mov	[si.brcRightTcx],dx

LCalcCellSize:

; if (ww == wwNil)
; return itcNext;
	errnz	<wwNil>
	mov	cx,[ww]
	jcxz	LTempExitIG	; which jumps to LExitIG

; /* Compute the xp's from the xa's. Note that here is where we make
; /* the adjustment for the user's justification code.
; /**/
; pwwd=PwwdWw(ww);
; native note: we don't really need the pwwd
; ptcx->xpCellLeft = DxpFromDxa(pwwd,xaFirst += ptap->dxaAdjust);
; ptcx->xpCellRight = DxpFromDxa(pwwd,xaLim += ptap->dxaAdjust);
	; int 3 ; PAUSE
	mov	cx,[bx.dxaAdjustTap]
	mov	ax,[xaLim]
	add	ax,cx
	mov	[xaLim],ax
	push	ax	; save away...xaLim + ptap->dxaAdjust
	mov	ax,[xaFirst]
	add	ax,cx
	mov	[xaFirst],ax

	call	near ptr Loc_DxpFromDxa ; convert xaFirst + ptap->dxaAdjust
	mov	[si.xpCellLeftTcx],ax

	pop	ax
	call	near ptr Loc_DxpFromDxa ; convert xaFirst + ptap->dxaAdjust
	mov	[si.xpCellRightTcx],ax

; Assert (ptcx->xpCellLeft <= ptcx->xpCellRight);
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[si.xpCellRightTcx]
	cmp	ax,[si.xpCellLeftTcx]
	jge	IG060
	mov	ax,midFetchtbn
	mov	bx,483
	cCall	AssertProcForNative,<ax,bx>
IG060:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG

; if (itc == itcMac)
	mov	di,[itc]
	cmp	di,[itcMac]
	jne	LMergedCellWidth
; {
; ptcx->xpDrLeft = (ptcx->xpDrRight = DxpFromDxa(pwwd,xaLim)) - dxpTtpDr;
	; int 3 ; PAUSE
	mov	ax,[xaLim]
	call	near ptr Loc_DxpFromDxa
	mov	[si.xpDrRightTcx],ax
	mov	dx,[vfti.dxpBorderFti]
	errnz	<dxpTtpDrFti-8>
	mov	cl,3
	sal	dx,cl
	sub	ax,dx
	mov	[si.xpDrLeftTcx],ax
; Assert(ptcx->xpCellLeft+DxFromBrc(ptcx->brcLeft,fTrue)/2 <= ptcx->xpDrLeft);
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[si.brcLeftTcx]
	call	near ptr Loc_DxFromBrcFTrue
	sar	ax,1
	add	ax,[si.xpCellLeftTcx]
	cmp	ax,[si.xpDrLeftTcx]
	jle	IG070

	int 3 ; PAUSE
	mov	ax,midFetchtbn
	mov	bx,506
	cCall	AssertProcForNative,<ax,bx>
IG070:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG
	jmp	short LSetOutLeft
; }
; else
LMergedCellWidth:
; {
; if (ptap->rgtc[itc].fMerged)
	; di = itc, bx = ptap
	mov	ax,cbTcMin
	mul	di
	xchg	ax,di
	test	[bx.di.rgtcTap.fMergedTc],maskFMergedTc
	jz	LNormalCellWidth
; {
; ptcx->xpDrLeft = ptcx->xpDrRight = ptcx->xpCellLeft;
	; int 3 ; PAUSE
	mov	ax,[si.xpCellLeftTcx]
	errnz	<xpDrLeftTcx+2-xpDrRightTcx>
	errnz	<xpDrRightTcx+2-xpCellRightTcx>
	lea	di,[si.xpDrLeftTcx]
	push	ds
	pop	es
	stosw	; ptcx->xpDrLeft = 
	stosw	; ptcx->xpDrRight = 
	errnz	<xpDrRightTcx+2-xpCellRightTcx>
	stosw	; skip over ptcx->xpCellRight = the same thing
; ptcx->dxpOutRight = ptcx->dxpOutLeft = 0;
	errnz	<xpCellRightTcx+2-dxpOutLeftTcx>
	errnz	<dxpOutLeftTcx+2-dxpOutRightTcx>
	xor	ax,ax
	stosw
	stosw
; goto LExit;
; }
	jmp	LExitIG
; ptcx->xpDrLeft = DxpFromDxa(pwwd,min(xaLim,xaFirst+ptap->dxaGapHalf));
; ptcx->xpDrRight = DxpFromDxa(pwwd,max(xaFirst,xaLim-ptap->dxaGapHalf));
LNormalCellWidth:
	mov	ax,[xaFirst]
	add	ax,[bx.dxaGapHalfTap]
	cmp	ax,[xaLim]
	jle	IG080
	mov	ax,[xaLim]
IG080:
	call	near ptr Loc_DxpFromDxa
	mov	[si.xpDrLeftTcx],ax

	mov	ax,[xaLim]
	sub	ax,[bx.dxaGapHalfTap]
	cmp	ax,[xaFirst]
	jge	IG090
	mov	ax,[xaFirst]
IG090:
	call	near ptr Loc_DxpFromDxa
	mov	[si.xpDrRightTcx],ax
; }

; /* NOTE - for the vertical stripes, we call DxFromBrc with 
; /* fDrawDrs=fTrue so that we allocate room for frame lines. This
; /* assumes that dxaGap translates into at least one dxpBorderFti of gap.
; /* NOTE - Split the border down the rgdxaCenter lines so that uneven
; /* borders spill more over onto the left side, quantizing in dxpBorderFti
; /* units.
; /* CLEVERNESS WARNING: The trickery below assumes that
; /* DxFromBrc(brc,fTrue) returns either 1, 2, or 3 times dxpBorderFti.
; /**/

LSetOutLeft:
; ptcx->dxpOutLeft = ptcx->xpDrLeft - ptcx->xpCellLeft;
; if (DxFromBrc(ptcx->brcLeft,fTrue) > dxpBorderFti)
;   ptcx->dxpOutLeft -= dxpBorderFti;
	mov	ax,[si.brcRightTcx]
	call	near ptr Loc_DxFromBrcFTrue
	xchg	ax,di	; save for later

	mov	ax,[si.brcLeftTcx]
	call	near ptr Loc_DxFromBrcFTrue

	mov	cx,[vfti.dxpBorderFti]
	cmp	ax,cx
	jg	IG100
	xor	cx,cx
IG100:
	neg	cx
	add	cx,[si.xpDrLeftTcx]
	sub	cx,[si.xpCellLeftTcx]
	mov	[si.dxpOutLeftTcx],cx

; if (itc < itcMac)
	; bx = ptap, di = DxFromBrc(ptcx->brcRight, fTrue);
	mov	cx,[itc]
	sub	cx,[itcMac]
	jz	LSetDxpOutRight
; {
; int dxpT;
; dxpT = DxFromBrc(ptcx->brcRight,fTrue); 
; native note: done above, in di
; ptcx->dxpOutRight = ptcx->xpCellRight - ptcx->xpDrRight - dxpT;
	mov	cx,[si.xpCellRightTcx]
	sub	cx,[si.xpDrRightTcx]
	sub	cx,di
; if (dxpT > dxpBorderFti)
; ptcx->dxpOutRight += dxpBorderFti;
	cmp	di,[vfti.dxpBorderFti]
	jle	LSetDxpOutRight
	add	cx,[vfti.dxpBorderFti]
	; fall through to next line
; }
LSetDxpOutRight:
; else
; ptcx->dxpOutRight = 0;
	mov	[si.dxpOutRightTcx],cx

; /* If the user has created a cell without enough room to hold the
; /* border (or potential frame line), shrink the cell.
; /* If this action causes the cell width to become negative,
; /* adjust the tap to make room for a wider cell.
; /**/
;
; /* In order to maintain enough room to draw the borders, we must
; /* maintain the two quantities
; /*		xpDrLeft - dxpOutLeft - xpCellLeft
; /*		xpCellRight - (xpDrRight + dxpRight)
; /**/
; if (ptcx->dxpOutLeft < 0)
	xor	dx,dx
	cmp	[si.dxpOutLeftTcx],dx
	jge	LCheckOutRight
; {
; ptcx->xpDrLeft -= ptcx->dxpOutLeft;
	; int 3 ; PAUSE
	mov	ax,[si.dxpOutLeftTcx]
	sub	[si.xpDrLeftTcx],ax
; ptcx->dxpOutLeft = 0;
	mov	[si.dxpOutLeftTcx],dx
; }
; if (ptcx->dxpOutRight < 0)
LCheckOutRight:
	cmp	[si.dxpOutRightTcx],dx
	jge	LCheckDrSides
; {
; ptcx->xpDrRight += ptcx->dxpOutRight;
	; int 3 ; PAUSE
	mov	ax,[si.dxpOutRightTcx]
	add	[si.xpDrRightTcx],ax
; ptcx->dxpOutRight = 0;
	mov	[si.dxpOutRightTcx],dx
; }

; if ((dxp = ptcx->xpDrLeft - ptcx->xpDrRight) > 0)
LCheckDrSides:
	mov	ax,[si.xpDrLeftTcx]
	sub	ax,[si.xpDrRightTcx]
	jle	LExitIG
; {
; /* Add one to the adjustment so that dxa/dxp conversion
; /* roundoff can't hose things.
; /**/
; Assert(itc < itcMac);
; native note: for loop below Assert(itcNext <= itcMac);
	; int 3 ; PAUSE
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[itc]
	cmp	ax,[itcMac]
	jge	IG110

	mov	ax,[itcNext]
	cmp	ax,[itcMac]
	jle	IG115

IG110:
	mov	ax,midFetchtbn
	mov	bx,653
	cCall	AssertProcForNative,<ax,bx>
IG115:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG
; dxa = DxaFromDxp(pwwd,dxp+1);
	; ax = dxp, bx = ptap
	inc	ax
	call	near ptr Loc_DxaFromDxp

; for ( ; itcNext <= itcMac; ++itcNext )
;   ptap->rgdxaCenter[itcNext] += dxa;
; native note use "itcNext < itcMac" for term condition.
	; ax = dxa
	mov	di,[itcNext]
	mov	cx,[itcMac]
	sub	cx,di
	inc	cx
ifdef	DEBUG
	jle	IG110	; Assert if (cx=itcMac+1-itcNext)<= 0
endif	; DEBUG
	shl	di,1
	add	di,rgdxaCenterTap

LAdjustRgdxa:
	add	[bx.di],ax
	add	di,2
	loop	LAdjustRgdxa
LAdjustEnd:

; Assert(!fRestarted); /* no infinite loops allowed */
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	cx,[fRestarted]
	jcxz	IG120

	mov	ax,midFetchtbn
	mov	bx,709
	cCall	AssertProcForNative,<ax,bx>
IG120:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
; Debug ( fRestarted = fTrue; )
	mov	[fRestarted],fTrue
endif	; DEBUG

; /* it is now easier to re-compute with the adjusted TAP than to try
; /* to figure out how to adjust things correctly
; /**/
; goto LRestart;
; }
	jmp	LRestart

; LExit:
; Assert (ptcx->dxpOutLeft >= 0 && ptcx->dxpOutRight >= 0);
; Assert (ptcx->xpCellLeft <= ptcx->xpDrLeft - ptcx->dxpOutLeft);
; Assert (ptcx->xpDrLeft <= ptcx->xpDrRight);
; Assert (ptcx->xpDrRight + ptcx->dxpOutRight <= ptcx->xpCellRight);
LExitIG:
ifdef	DEBUG
; Assert (ptcx->dxpOutLeft >= 0 && ptcx->dxpOutRight >= 0);
	cmp	[si.dxpOutLeftTcx],0
	jl	IG202
	cmp	[si.dxpOutRightTcx],0
	jge	IG204
IG202:
	mov	ax,midFetchtbn
	mov	bx,743
	cCall	AssertProcForNative,<ax,bx>
IG204:

; Assert (ptcx->xpCellLeft <= ptcx->xpDrLeft - ptcx->dxpOutLeft);
	mov	ax,[si.xpDrLeftTcx]
	sub	ax,[si.dxpOutLeftTcx]
	cmp	ax,[si.xpCellLeftTcx]
	jge	IG206
	mov	ax,midFetchtbn
	mov	bx,759
	cCall	AssertProcForNative,<ax,bx>
IG206:
; Assert (ptcx->xpDrLeft <= ptcx->xpDrRight);
	mov	ax,[si.xpDrRightTcx]
	cmp	ax,[si.xpDrLeftTcx]
	jge	IG208
	mov	ax,midFetchtbn
	mov	bx,815
	cCall	AssertProcForNative,<ax,bx>
IG208:
; Assert (ptcx->xpDrRight + ptcx->dxpOutRight <= ptcx->xpCellRight);
	mov	ax,[si.xpDrRightTcx]
	add	ax,[si.dxpOutRightTcx]
	cmp	ax,[si.xpCellRightTcx]
	jle	IG210
	mov	ax,midFetchtbn
	mov	bx,824
	cCall	AssertProcForNative,<ax,bx>
IG210:
endif	; DEBUG

LRetIG:

; return itcNext;
; }
	mov	ax,[itcNext]

cEnd
;End of ItcGetTcx

; Local Version of DxaFromDxp and DxpFromDxa
;
; ax = argument,  dxa or dxp
; ax gets result, dxp or dxa
; only ax, cx, dx altered
Loc_DxaFromDxp:
	db	0A8h	;turns next "stc" into "test al,immediate"
			;also clears the carry flag
Loc_DxpFromDxa:
	stc
	mov	cx,dxaInch
	mov	dx,[vfti.dxpInchFti]
; Assert(vfti.dxpInch != 0)
ifdef	DEBUG
	pushf
	push	ax
	push	bx
	push	cx
	push	dx
	or	dx,dx
	jnz	IG300
	mov	ax,midFetchtbn
	mov	bx,871
	cCall	AssertProcForNative,<ax,bx>
IG300:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	popf
endif	; DEBUG
	jc	IG305
	xchg	cx,dx
IG305:
	push	ax	; save the numerator
	or	ax,ax	; is it negative?
	jns	IG310
	neg	ax
IG310:
	mul	dx	; do the multiply
	push	cx	; save the denominator
	shr	cx,1	; get half of the demoninator to adjust for rounding
    	add     ax,cx	; adjust for possible rounding error
    	adc     dx,0	; this is really a long addition
	pop     cx	; restore the demoninator
	div     cx	; divide
	pop	cx
	or	cx,cx	; restore the sign
	jns	IG315
	neg	ax
IG315:
	ret
	
; Local Version of DxFromBrc(brc, fTrue)
;
; #define DxFromBrc(brc, fFrameLines) DxyFromBrc(brc, fFrameLines, fTrue)
; #define DxyFromBrc(brc, fFrameLines, fWidth)	\
;	WidthHeightFromBrc(brc, fFrameLines, fWidth, fFalse)
;
; ax = brc
; ax gets result, dxp or dxa
; only ax, cx, dx, es altered
Loc_DxFromBrcFTrue:
	push	bx
	push	ax
	mov	ax,3
	push	ax
ifdef DEBUG
	cCall	S_WidthHeightFromBrc,<>
else ;not DEBUG
	cCall	N_WidthHeightFromBrc,<>
endif ;DEBUG
	pop	bx
	ret

sEnd	fetchtbn
	end
