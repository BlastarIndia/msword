	include w2.inc
	include noxport.inc
	include consts.inc
	include structs.inc

createSeg	_FETCH,fetch,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midFetchn3	equ 14		; module ID, for native asserts
endif

; EXPORTED LABELS

; EXTERNAL FUNCTIONS

externFP	<IbpReadFilePage>
externFP	<HpsAlter,ApplySprm>
externFP	<AnsiLower,MapStcStandard>
externFP	<IInPlcRef>
externFP	<IInPlcCheck>
externFP	<N_FetchCp>
externFP	<N_CachePara>
externFP	<IbpLoadFn>
externNP	<LN_IcpInRgcp>
externNP	<LN_LprgcpForPlc>
externNP	<LN_PdodOrNilFromDoc>
externNP	<LN_ReloadSb>
externNP	<LN_CpFromPlcIcp>
externNP	<LN_FInCa>
externFP	<CpMacDocEdit>
externFP	<CpFirstTap>

ifdef DEBUG
externFP	<AssertProcForNative>
externFP	<S_CachePara,S_FetchCp>
externFP	<S_ApplyPrlSgc>
externFP	<CpMacDoc>
externFP	<FInCa>
endif


; EXTERNAL DATA

sBegin	data

externW vchpStc 	; extern struct CHP	  vchpStc;
externW vhpchFetch	; extern HUGE		  *vhpchFetch;
externW mpfnhfcb	; extern struct FCB	  **mpfnhfcb[];
externW vsab		; extern struct SAB	  vsab;
externW vibp		; extern int		  vibp;
externW vbptbExt	; extern struct BPTB	  vbptbExt;
externW vstcpMapStc	; extern int		  vstcpMapStc;
externW dnsprm		; extern struct ESPRM	  dnsprm[];
externW vfnPreload	; extern int		  vfnPreload;
externW vpapFetch	; extern struct PAP	  vpapFetch;
externW caPara		; extern struct CA	  caPara;
externW vdocFetch	; extern int		  vdocFetch;
externD vcpFetch	; extern CP		  vcpFetch;
externW fcmFetch	; extern int		  fcmFetch;
externW vcaCell 	; extern struct CA	  vcaCell;
externD vcpFirstTablePara   ; extern CP 	  vcpFirstTablePara;
externD vcpFirstTableCell   ; extern CP 	  vcpFirstTableCell;
externW vdocTemp	; extern int		  vdocTemp;
externD vmpitccp	; extern CP vmpitccp[];
externW caTap		; extern struct CA	  caTap;
externW vtapFetch	; extern struct TAP   vtapFetch;

ifdef DEBUG
externW 		wFillBlock
endif

sEnd	data


; CODE SEGMENT _FETCH

sBegin	fetch
	assumes cs,fetch
	assumes ds,dgroup
	assumes ss,dgroup


;-------------------------------------------------------------------------
;	BFromFc(hpfkp,fc,pfcFirst,pfcLim,pifc)
;-------------------------------------------------------------------------
; %%Function:N_BFromFc %%Owner:BRADV
cProc	N_BFromFc,<PUBLIC,FAR>,<si,di>
	ParmD	hpfkp
	ParmD	fc
	ParmW	pfcFirst
	ParmW	pfcLim
	ParmW	pifc

ifdef DEBUG
	LocalW	diSave
endif ;DEBUG

; /* B	 F R O M   F C */
; /* Return the b, fcFirst & fcLim for the first run with fcLim > fc. */
; native int BFromFc(hpfkp, fc, pfcFirst, pfcLim, pifc)
; struct FKP HUGE *hpfkp;
; FC fc, *pfcFirst, *pfcLim;
; {
cBegin

	mov	bx,[SEG_hpfkp]
	;LN_ReloadSb takes an sb in bx and set es to the corresponding value.
	;Only es and bx are altered.
	call	LN_ReloadSb
	mov	di,[OFF_hpfkp]
	mov	ax,[OFF_fc]
	mov	dx,[SEG_fc]
	call	LN_BFromFcCore
	mov	di,[pfcFirst]
	movsw
	movsw
	mov	di,[pfcLim]
	movsw
	movsw
	push	ss
	pop	ds
	mov	bx,[pifc]
	mov	[bx],cx
; }
cEnd


;	LN_BFromFcCore takes qfkp in es:di, fc in dx:ax.
;	The result is returned in ax.  ifc is returned in cx.
;	Upon exit ds == es upon input, es == psDds,
;	and ds:si points to (qfkp->rgfc[ifc])
; %%Function:LN_BFromFcCore %%Owner:BRADV
PUBLIC LN_BFromFcCore
LN_BFromFcCore:

;	  struct RUN *prun, *rgrun;
;	  int crun, ifc;
;
;	  crun = hpfkp->crun;
;	  *pifc = ifc = IcpInRgcp(LpFromHp(hpfkp->rgfc), crun, (CP) fc);

	errnz <(rgfcFkp)>
	mov	cl,es:[di.crunFkp]
	xor	ch,ch
	push	cx	;save crun
;	 LN_IcpInRgcp expects lprgcp in es:di, icpLim in cx, and cp in ax:dx.
;	 bx and di are not altered.
	cCall LN_IcpInRgcp
	pop	cx	;restore crun

;	  *pfcFirst = (hpfkp->rgfc)[ifc];
;	  *pfcLim = (hpfkp->rgfc)[ifc + 1];

	; ax = ifc, es:di = pfkp, cx = crun
	push	es
	pop	ds
	push	ss
	pop	es
	mov	si,ax
	shl	si,1
	shl	si,1
	add	si,di
	push	si
	mov	si,di

;	  return (((char *)&((hpfkp->rgfc)[hpfkp->crun + 1]))[ifc]) << 1);

	; ax = ifc
	; cx = crun
	errnz	<(rgfcFkp)>
	inc	cx
	shl	cx,1
	shl	cx,1
	add	si,cx
	add	si,ax
	mov	cx,ax
;	NOTE: we know ah is zero because ifc < (cbFkp / 4)
	lodsb
	shl	ax,1
	pop	si

	ret


;-------------------------------------------------------------------------
;	MapStc(pdod, stc, pchp, ppap)
;-------------------------------------------------------------------------
; %%Function:N_MapStc %%Owner:BRADV
cProc	N_MapStc,<PUBLIC,FAR>,<si,di>
	ParmW	pdod
	ParmW	stcArg
	ParmW	pchp
	ParmW	ppap
 ; /* M A P   S T C */
; /* maps pdod, stc into
;	  *pchp
;	  *ppap
; */
; native MapStc(pdod, stc, pchp, ppap)
; struct DOD *pdod; int stc; struct CHP *pchp; struct PAP *ppap;
; {
;	  int cch, stcpT;
;	  CHAR HUGE *hpchpe, HUGE *hppape;
;
cBegin
	mov	bx,pdod
	mov	cx,stcArg
	mov	dx,pchp
	mov	di,ppap
	cCall	LN_MapStc,<>
; }
cEnd

;	LN_MapStc takes pdod in bx, stc in cx, pchp in dx, ppap in di.
;	ax, bx, cx, dx, si, di are altered.
; %%Function:LN_MapStc %%Owner:BRADV
PUBLIC LN_MapStc
LN_MapStc:

; #ifdef DEBUG
;	  int cMothers = 0;
; #endif /* DEBUG */
ifdef DEBUG
	push	cx	;save stc
	xor	cx,cx	;cMothers = 0;
endif ;DEBUG

;	  while (!pdod->fMother || pdod->fMotherStsh)
;		  {
MS01:
	cmp	[bx.FMotherDod],fFalse
	je	MS015
	test	[bx.FMotherStshDod],maskFMotherStshDod
	je	MS02
MS015:

ifdef DEBUG
;	/* Assert (cMothers ++ < 5 && pdod->doc != docNil) with a call
;	so as not to mess up short jumps */
	call	MS07
endif ;/* DEBUG */

;		  pdod = PdodDoc(pdod->doc);
;	PdodOrNilFromDoc takes a doc in bx, and returns hNil in ax and bx
;	if mpdochdod[doc] == hNil.  Otherwise it returns doc in ax
;	and pdod = *mpdochdod[doc] in bx.  The zero flag is set according
;	to the value of bx upon return.  Only ax and bx are altered.
	mov	bx,[bx.docDod]
	call	LN_PdodOrNilFromDoc
	jmp	short MS01
;		  }
MS02:
ifdef DEBUG
	pop	cx	;restore stc
endif ;DEBUG

; cx = stc, dx = pchp, bx = pdod, di = ppap
;	  if (pdod == PdodDoc(docScrap) && vsab.docStsh != docNil)
	push	bx	;save pdod
	mov	bx,docScrap
;	PdodOrNilFromDoc takes a doc in bx, and returns hNil in ax and bx
;	if mpdochdod[doc] == hNil.  Otherwise it returns doc in ax
;	and pdod = *mpdochdod[doc] in bx.  The zero flag is set according
;	to the value of bx upon return.  Only ax and bx are altered.
	call	LN_PdodOrNilFromDoc
	pop	ax	;restore pdod
	xchg	ax,bx
	cmp	ax,bx
	jne	MS03
	cmp	[vsab.docStshSab],docNil
	je	MS03

;		  pdod = PdodDoc(vsab.docStsh);
	mov	bx,[vsab.docStshSab]
;	PdodOrNilFromDoc takes a doc in bx, and returns hNil in ax and bx
;	if mpdochdod[doc] == hNil.  Otherwise it returns doc in ax
;	and pdod = *mpdochdod[doc] in bx.  The zero flag is set according
;	to the value of bx upon return.  Only ax and bx are altered.
	call	LN_PdodOrNilFromDoc
MS03:

ifdef DEBUG
;	/* Assert (pdod->hsttbChpe && ppap != 0) with a call
;	so as not to mess up short jumps */
	call	MS10
endif ;/* DEBUG */

;	  psttbChpe = *pdod->hsttbChpe;
	mov	si,[bx.hsttbChpeDod]
	mov	si,[si]
	push	si	;save psttbChpe

; si = [sp] = psttbChpe, cx = stc, dx = pchp, bx = pdod, di = ppap
;	  vstcpMapStc = (stc + pdod->stsh.cstcStd) & 0377;
	mov	al,cl
	add	al,bptr ([bx.stshDod.cstcStdStsh])
	xor	ah,ah

; si = [sp] = psttbChpe, cx = stc, dx = pchp, bx = pdod, ax = vstcpMapStc
; di = ppap
;	  if (vstcpMapStc >= psttbChpe->ibstMac)
;		  {
	cmp	ax,[si.ibstMacSttb]
	jl	MS04

;		  vstcpMapStc = pdod->stsh.cstcStd;
	mov	ax,[bx.stshDod.cstcStdStsh]

;		  if (stc >= stcStdMin) goto LStcStandard;
	cmp	cx,stcStdMin
	jl	MS04
	mov	[vstcpMapStc],ax

; si = [sp] = psttbChpe, cx = stc, dx = pchp, bx = pdod, ax = vstcpMapStc
; di = ppap
	jmp	short LStcStandard
;		  }

; si = [sp] = psttbChpe, cx = stc, dx = pchp, bx = pdod, ax = vstcpMapStc
; di = ppap
MS04:
ifdef DEBUG
;	/* Assert (pdod->hsttbPape) with a call
;	so as not to mess up short jumps */
	call	MS13
endif ;/* DEBUG */

;	  psttbPape = *pdod->hsttbPape;
	mov	si,[bx.hsttbPapeDod]
	mov	si,[si]

; [sp] = psttbChpe, cx = stc, dx = pchp, bx = pdod, ax = vstcpMapStc
; di = ppap, si = psttbPape
ifdef DEBUG
;	/* Assert (psttbPape->ibstMac > vstcpMapStc) with a call
;	so as not to mess up short jumps */
	call	MS15
endif ;/* DEBUG */

;	  hppape = HpstFromSttb(pdod->hsttbPape,vstcpMapStc);
	mov	[vstcpMapStc],ax
	;LN_LpstFromSttb takes a psttb in si, an ibst in ax, and returns an
	;lpst in es:si,  Only ax, bx, si, es are altered.
	push	bx	;save pdod in case we goto LStcStandard
	call	LN_LpstFromSttb
	pop	bx	;restore pdod

; [sp] = psttbChpe, cx = stc, dx = pchp, bx = pdod
; di = ppap, es:si = hppape
;	  if ((cch = *hppape++) == 0377) goto LStcStandard;
	lods	byte ptr es:[si]
	cmp	al,0377O
	je	LStcStandard
	push	cx	;save stc
	push	di	;save ppap

;	  bltbh(hppape, ppap, cch);
	push	ss
	push	es
	pop	ds
	pop	es
	xor	ah,ah
	mov	cx,ax
	rep	movsb
	push	ss
	pop	ds	;restore ds

;	  SetBytes(((char *)ppap) + cch, 0,
;		  ((cch < cbPAPBase) ?
;		  cbPAPBase : cbPAP) - cch);
; dx = pchp, di = ppap+cch, ax = cch
	mov	cx,cbPAPBase
	cmp	ax,cx
	jl	MS05
	mov	cx,cbPAP
MS05:
	sub	cx,ax
	xor	ax,ax
	rep	stosb

	pop	si	;restore ppap
	pop	cx	;restore stc

;	  ppap->stc = stc; /* return proper stc even if we faked the style */
	mov	[si.stcPap],cl
	pop	si	;restore psttbChpe

; si = psttbChpe, cx = stc, dx = pchp
;	  if (pchp)
;		  {
	or	dx,dx
	jz	MS06

ifdef DEBUG
;	/* Assert (psttbChpe->ibstMac > vstcpMapStc) with a call
;	so as not to mess up short jumps */
	call	MS17
endif ;/* DEBUG */

;		  hpchpe = HpstFromSttb(pdod->hsttbChpe, vstcpMapStc);
	mov	ax,[vstcpMapStc]
	;LN_LpstFromSttb takes a psttb in si, an ibst in ax, and returns an
	;lpst in es:si,  Only ax, bx, si, es are altered.
	call	LN_LpstFromSttb

; es:si = hpchpe, cx = stc, dx = pchp
;		  cch = *hpchpe++;
	push	ss
	push	es
	pop	ds
	pop	es
	lodsb

;		  bltbh(hpchpe, pchp, cch);
	cbw
	mov	di,dx
	mov	cx,ax
	rep	movsb
	push	ss
	pop	ds	;restore ds

;		  SetBytes(((char *)pchp) + cch, 0, cwCHP*sizeof(int)-cch);
; assumes di = pchp + cch
	mov	cx,cbCHP
	sub	cx,ax
	xor	ax,ax
	rep	stosb

;		  }
;	  return;
	jmp	short MS06


; LStcStandard:
; [sp] = psttbChpe, cx = stc, dx = pchp, bx = pdod, di = ppap
LStcStandard:
	pop	ax	;remove psttbChpe from the stack.

;	  stcpT = vstcpMapStc;
	push	[vstcpMapStc]

;	  MapStc(pdod, 0, pchp, ppap);
;	  MapStcStandard(stc, pchp, ppap);
	push	cx	;stc argument for MapStcStandard
	push	dx	;pchp argument for MapStcStandard
	push	di	;ppap argument for MapStcStandard
	xor	cx,cx
;	LN_MapStc takes pdod in bx, stc in cx, pchp in dx, ppap in di.
;	ax, bx, cx, dx, si, di are altered.
	call	LN_MapStc
	cCall	MapStcStandard,<>

;	  vstcpMapStc = stcpT;
	pop	[vstcpMapStc]

MS06:
; }
	ret


ifdef DEBUG
MS07:
;		  Assert (cMothers++ < 5);/* avoid a loop */
	inc	cx
	cmp	cx,6
	jl	MS08
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFetchn3
	mov	bx,250
	cCall	AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	push	ds
	pop	es
MS08:
;		  Assert (pdod->doc != docNil);
	cmp	[bx.docDod],docNil
	jne	MS09
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFetchn3
	mov	bx,251
	cCall	AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	push	ds
	pop	es
MS09:
	ret
endif ;DEBUG


ifdef DEBUG
;	  Assert(pdod->hsttbChpe);
MS10:
	cmp	[bx.hsttbChpeDod],0
	jnz	MS11
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFetchn3
	mov	bx,252
	cCall	AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	push	ds
	pop	es
MS11:
;	  Assert(ppap != 0);
	cmp	di,0
	jne	MS12
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFetchn3
	mov	bx,253
	cCall	AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	push	ds
	pop	es
MS12:
	ret
endif ;DEBUG


ifdef DEBUG
;	  Assert(pdod->hsttbPape);
MS13:
	cmp	[bx.hsttbPapeDod],0
	jnz	MS14
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFetchn3
	mov	bx,254
	cCall	AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	push	ds
	pop	es
MS14:
	ret
endif ;DEBUG


ifdef DEBUG
;	  Assert(psttbPape->ibstMac > vstcpMapStc);
MS15:
; assumes ax = vstcpMapStc
	cmp	ax,[si.ibstMacSttb]
	jl	MS16
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFetchn3
	mov	bx,255
	cCall	AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	push	ds
	pop	es
MS16:
	ret
endif ;DEBUG


ifdef DEBUG
;		  Assert(psttbChpe->ibstMac > vstcpMapStc);
MS17:
	push	ax
	mov	ax,[si.ibstMacSttb]
	cmp	ax,[vstcpMapStc]
	pop	ax
	jg	MS18
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFetchn3
	mov	bx,256
	cCall	AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	push	ds
	pop	es
MS18:
	ret
endif ;DEBUG

	;LN_LpstFromSttb takes a psttb in si, an ibst in ax, and returns an
	;lpst in es:si,  Only ax, bx, si, es are altered.
LN_LpstFromSttb:
ifdef DEBUG
	mov	bx,[wFillBlock]
endif ;DEBUG
	push	ss
	pop	es
	add	si,([rgbstSttb])    ;default es:si for !fExternal sttb
	test	[si.fExternalSttb - (rgbstSttb)],maskFExternalSttb
	je	LN_LFS01
	mov	bx,[si.SB_hqrgbstSttb - (rgbstSttb)]
	;LN_ReloadSb takes an sb in bx and set es to the corresponding value.
	;Only es and bx are altered.
	call	LN_ReloadSb
	mov	si,[si.OFF_hqrgbstSttb - (rgbstSttb)]
	mov	si,es:[si]
LN_LFS01:
	xchg	ax,bx
	shl	bx,1
	add	si,es:[bx+si]
	ret


;-------------------------------------------------------------------------
;	LowerRgch(pch, cch)					GregC
;-------------------------------------------------------------------------
; %%Function:LowerRgch %%Owner:BRADV
cProc	LowerRgch,<PUBLIC,FAR>,<si,di>
	ParmW	pch
	ParmW	cch
; /* L O W E R	R G C H */
; /* converts characters to lower case */
; native LowerRgch(pch, cch)
; char *pch;
; int cch;
; {
cBegin
;	  for (; cch-- > 0; pch++)
	xor	bx,bx
	mov	di,[cch]
	mov	si,[pch]
	or	di,di
	jz	LowerRgch4
;		  *pch = AnsiLower(*pch);

LowerRgch2:
	lodsb
	cCall	AnsiLower,<bx,ax>
	mov	[si-1],al
	sub	di,1
	jnz	LowerRgch2

LowerRgch4:
; }
cEnd

;-------------------------------------------------------------------------
;	HpchFromFc(fn, fc)
;-------------------------------------------------------------------------
; %%Function:N_HpchFromFc %%Owner:BRADV
cProc	N_HpchFromFc,<PUBLIC,FAR>,<si,di>
	ParmW	fn
	ParmD	fc
; /* H P C H   F R O M	 F C */
; /* Returns pch to ch at fc in fn.
; Buffer number is left in vibp so that the buffer may be set dirty by caller.
; */
; native char HUGE *HpchFromFc(fn, fc)
; int fn;
; FC fc;
; {
cBegin
	mov	bx,[OFF_fc]
	mov	dx,[SEG_fc]
	mov	ax,[fn]
	call	LN_HpchFromFc
; }
cEnd

;	LN_HpchFromFc takes fn in ax, and fc in dx:bx.
;	The result is returned in dx:ax.
;	ax, bx, cx, dx, si, di are altered.
; %%Function:LN_HpchFromFc %%Owner:BRADV
PUBLIC LN_HpchFromFc
LN_HpchFromFc:

;	Assert(fn >= fnScratch && mpfnhfcb[fn] != hNil && fn < fnMax);
;	Assert(fc < fcMax);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	ax,fnScratch
	jl	HFF01
	push	bx
	mov	bx,ax
	shl	bx,1
	cmp	mpfnhfcb[bx],hNil
	pop	bx
	je	HFF01
	cmp	ax,fnMax
	jge	HFF01
	sub	bx,LO_fcMax
	sbb	dx,HI_fcMax
	jl	HFF02
HFF01:
	mov	ax,midFetchn3
	mov	bx,1032
	cCall	AssertProcForNative,<ax, bx>
HFF02:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

;	  vibp = IbpCacheFilePage(fn, (PN)(fc >> shftSector));
	errnz <shftSector - 9>
	push	bx	;save low fc
	xchg	dl,dh
	xchg	bh,dl
	shr	bh,1
	rcr	dx,1
;	LN_IbpCacheFilePage takes fn in ax, pn in dx.
;	The result is returned in ax.  ax, bx, cx, dx, di are altered.
	call	LN_IbpCacheFilePage
	mov	[vibp],ax

;	  bOffset = (int)fc & maskSector; /* beware native compiler bug */
;	  hpch = HpBaseForIbp(vibp) + bOffset;
;	  return (hpch);
	;***Begin in line HpBaseForIbp
	xor	dx,dx
	div	[vbptbExt.cbpChunkBptb] 	; ax = result, dx = remainder
	add	ax,[vbptbExt.SB_hprgbpExtBptb]
	xchg	ax,dx
	errnz	<cbSector - 512>
	mov	cl,9
	shl	ax,cl
	;***End in line HpBaseForIbp
	pop	bx	;restore low fc
	errnz	<maskSector - 001FFh>
	and	bh,(maskSector SHR 8)
	add	ax,bx

	ret


;-------------------------------------------------------------------------
;	HpchGetPn(fn, pn)
;-------------------------------------------------------------------------
; %%Function:N_HpchGetPn %%Owner:BRADV
cProc	N_HpchGetPn,<PUBLIC,FAR>,<si,di>
	ParmW	fn
	ParmW	pn
; /* H P C H   G E T   P N */
; /* Returns pch to start of buffer page containing page pn in file fn.
; Buffer number is left in vibp so that the buffer may be set dirty by caller.
; */
; native char HUGE *HpchGetPn(fn, pn)
; int fn;
; PN pn;
; {
cBegin
	mov	ax,[fn]
	mov	dx,[pn]
	call	LN_HpchGetPn
; }
cEnd

;	LN_HpchGetPn takes fn in ax, and pn in dx.
;	The result is returned in bx:ax.  ax, bx, cx, dx, si, di are altered.
; %%Function:LN_HpGetPn %%Owner:BRADV
PUBLIC LN_HpchGetPn
LN_HpchGetPn:
;	Assert(fn >= fnScratch && mpfnhfcb[fn] != hNil && fn < fnMax);
;	Assert(pn >= 0 && pn <= 0x7fff);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	ax,fnScratch
	jl	HGP01
	mov	bx,ax
	shl	bx,1
	cmp	mpfnhfcb[bx],hNil
	je	HGP01
	cmp	ax,fnMax
	jge	HGP01
	or	dx,dx
	jge	HGP02
HGP01:
	mov	ax,midFetchn3
	mov	bx,1033
	cCall	AssertProcForNative,<ax, bx>
HGP02:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

;	  vibp = IbpCacheFilePage(fn, pn);
;	LN_IbpCacheFilePage takes fn in ax, pn in dx.
;	The result is returned in ax.  ax, bx, cx, dx, di are altered.
	call	LN_IbpCacheFilePage
	mov	[vibp],ax
;	  hpch = HpBaseForIbp(vibp);	    /* possible compiler problem */
;	  return(hpch);
	;***Begin in line HpBaseForIbp
	xor	dx,dx
	div	[vbptbExt.cbpChunkBptb] 	; ax = result, dx = remainder
	add	ax,[vbptbExt.SB_hprgbpExtBptb]
	xchg	ax,dx
	errnz	<cbSector - 512>
	mov	cl,9
	shl	ax,cl
	;***End in line HpBaseForIbp
	mov	bx,dx

	ret


;-------------------------------------------------------------------------
;	SetDirty(ibp)
;-------------------------------------------------------------------------
; /* S E T  D I R T Y */
; /* ibp in internal cache */
; native SetDirty(ibp)
; int ibp;
; {
; %%Function:SetDirty %%Owner:BRADV
PUBLIC	SetDirty
SetDirty:
;	 struct BPS HUGE *hpbps;
;	 hpbps = &((struct BPS HUGE *)vbptbExt.hpmpibpbps)[ibp];
;	 hpbps->fDirty = fTrue;
	mov	bx,[vbptbExt.SB_hpmpibpbpsBptb]
	;LN_ReloadSb takes an sb in bx and set es to the corresponding value.
	;Only es and bx are altered.
	call	LN_ReloadSb
	mov	bx,sp
	mov	bx,[bx+4]
	errnz	<cbBpsMin - 8>
	shl	bx,1
	shl	bx,1
	shl	bx,1
	add	bx,[vbptbExt.OFF_hpmpibpbpsBptb]
	mov	es:[bx.fDirtyBps],fTrue

;	 Assert(hpbps->fn != fnNil);
ifdef DEBUG
	cmp	es:[bx.fnBps],fnNil
	jnz	SD01
	inc	bp
	push	bp
	mov	bp,sp
	push	bx
	mov	ax,midFetchn3
	mov	bx,1001
	cCall	AssertProcForNative,<ax,bx>
	pop	bx
	pop	bp
	dec	bp
SD01:
endif ;DEBUG

;	 PfcbFn(hpbps->fn)->fDirty = fTrue;
	mov	bl,es:[bx.fnBps]
	xor	bh,bh
	shl	bx,1
	mov	bx,[bx.mpfnhfcb]
	mov	bx,[bx]
	or	[bx.fDirtyFcb],maskfDirtyFcb

; }
	db	0CAh, 002h, 000h    ;far ret, pop 2 bytes


;/*
;REVIEW(robho): Possible improvements to IbpCacheFilePage swapping strategy
;robho 3/29/85: Would be nice some day to incorporate a list of the easily
;accessable volumes (disks currently in the drives) into the file page cache
;swapping out algorithm - try to avoid swaps that involve the user changing
;disks.
;*/

;-------------------------------------------------------------------------
;	IbpCacheFilePage(fn, pn)
;-------------------------------------------------------------------------
;/* I B P   C A C H E	F I L E   P A G E */
;/* Get page pn of file fn into file cache pbptb.
;Return ibp.
;See w2.rules for disk emergencies.
;*/
;native int IbpCacheFilePage(fn, pn)
;int fn;
;PN pn;
;{
;	 int ibp, iibp;
;/* NOTE: IibpHash macro has changed */

; %%Function:N_IbpCacheFilePage %%Owner:BRADV
cProc	N_IbpCacheFilePage,<PUBLIC,FAR>,<si,di>
	ParmW	fn
	ParmW	pn

cBegin

	mov	ax,fn
	mov	dx,pn
	call	LN_IbpCacheFilePage
cEnd

;End of IbpCacheFilePage


;	LN_IbpCacheFilePage takes fn in ax, pn in dx.
;	The result is returned in ax.  ax, bx, cx, dx, di are altered.
; %%Function:LN_IbpCacheFilePage %%Owner:BRADV
PUBLIC LN_IbpCacheFilePage
LN_IbpCacheFilePage:
;	Assert(fn >= fnScratch && mpfnhfcb[fn] != hNil && fn < fnMax);
;	Assert(pn >= 0 && pn <= 0x7fff);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	ax,fnScratch
	jl	ICFP01
	mov	bx,ax
	shl	bx,1
	cmp	mpfnhfcb[bx],hNil
	je	ICFP01
	cmp	ax,fnMax
	jge	ICFP01
	or	dx,dx
	jge	ICFP02
ICFP01:
	mov	ax,midFetchn3
	mov	bx,1000
	cCall	AssertProcForNative,<ax, bx>
ICFP02:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

;	 iibp = IibpHash(fn, pn, vbptbExt.iibpHashMax);
;#define IibpHash(fn, pn, iibpHashMax) (((fn) * 33 + (pn) * 5) & ((iibpHashMax) - 1))
	mov	di,ax
	mov	cl,5
	shl	di,cl
	add	di,ax
	xchg	ax,cx
	mov	ax,dx
	add	di,ax
	shl	ax,1
	shl	ax,1
	add	di,ax
	mov	ax,[vbptbExt.iibpHashMaxBptb]
	dec	ax
	and	di,ax

;	 ibp = ((int far *)LpFromHp(vbptbExt.hprgibpHash))[iibp];
	mov	bx,[vbptbExt.SB_hprgibpHashBptb]
	;LN_ReloadSb takes an sb in bx and set es to the corresponding value.
	;Only es and bx are altered.
	call	LN_ReloadSb
	mov	bx,[vbptbExt.OFF_hprgibpHashBptb]
	shl	di,1
	mov	ax,es:[bx+di]

ICFP04:
;/* search list of buffers with the same hash code */
;	 while (ibp != ibpNil)
;		 {
	cmp	ax,ibpNil
	je	ICFP07

;		 qbps = QbpsHpIbp(vbptbExt.hpmpibpbps, ibp);
;#define QbpsHpIbp(hpmpibpbps, ibp)  \
;	 (&((struct BPS far *)LpFromHp(hpmpibpbps))[ibp])
	;Assert ([vbptbExt.SB_hpmpibpbpsBptb] == [vbptbExt.SB_hprgibpHashBptb])
	;with a call so as not to mess up short jumps.
ifdef DEBUG
	call	ICFP11
endif ;DEBUG
	errnz	<cbBpsMin - 8>
	mov	bx,ax
	shl	bx,1
	shl	bx,1
	shl	bx,1
	add	bx,[vbptbExt.OFF_hpmpibpbpsBptb]

;		 if (qbps->pn == pn && qbps->fn == fn)
;			 {
	cmp	dx,es:[bx.pnBps]
	jne	ICFP06
	cmp	cl,es:[bx.fnBps]
	jne	ICFP06

;/* page found in the cache */
;			 qbps->ts = ++(vbptbExt.tsMruBps);
	inc	[vbptbExt.tsMruBpsBptb]
	mov	cx,[vbptbExt.tsMruBpsBptb]
	mov	es:[bx.tsBps],cx

;			 vbptbExt.hpmpispnts[(ibp << 2) / vbptbExt.cqbpspn]
;			     = vbptbExt.tsMruBps;
	;Assert ([vbptbExt.SB_hpmpispnts] == [vbptbExt.SB_hprgibpHash])
	;with a call so as not to mess up short jumps.
ifdef DEBUG
	call	ICFP13
endif ;DEBUG
	push	ax	;save ibp
	shl	ax,1
	shl	ax,1
	cwd
	div	[vbptbExt.cqbpspnBptb]
	mov	bx,ax
	pop	ax	;restore ibp
	shl	bx,1
	add	bx,[vbptbExt.OFF_hpmpispntsBptb]
	mov	es:[bx],cx

;			 return(ibp);
;			 }
	jmp	short ICFP10

;		 ibp = qbps->ibpHashNext;
;		 }
ICFP06:
	mov	ax,es:[bx.ibpHashNextBps]
	jmp	short ICFP04

ICFP07:
;	if (fn == vfnPreload)
;		{	/* Read in big chunks! */
;		if ((ibp = IbpLoadFn(fn,pn)) != ibpNil)
;			return ibp;
;		}
	cmp	cx,[vfnPreload]
	jnz 	ICFP09
	push	cx	;save fn
	push	dx	;save pn
	cCall	IbpLoadFn,<cx,dx>
	pop	dx	;restore pn
	pop	cx	;restore fn
	cmp	ax,ibpNil
	jne	ICFP10
ICFP09:
;/* page not found, read page into cache */
;	 return IbpReadFilePage(fn, pn, iibp);
	shr	di,1
	cCall	IbpReadFilePage,<cx, dx, di>

ICFP10:
;}
	ret

ifdef DEBUG
	;Assert ([vbptbExt.SB_hpmpibpbps] == [vbptbExt.SB_hprgibpHash]);
ICFP11:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[vbptbExt.SB_hpmpibpbpsBptb]
	cmp	ax,[vbptbExt.SB_hprgibpHashBptb]
	je	ICFP12
	mov	ax,midFetchn3
	mov	bx,1030
	cCall	AssertProcForNative,<ax, bx>
ICFP12:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG

ifdef DEBUG
	;Assert ([vbptbExt.SB_hpmpispnts] == [vbptbExt.SB_hprgibpHash]);
ICFP13:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[vbptbExt.SB_hpmpispntsBptb]
	cmp	ax,[vbptbExt.SB_hprgibpHashBptb]
	je	ICFP14
	mov	ax,midFetchn3
	mov	bx,1031
	cCall	AssertProcForNative,<ax, bx>
ICFP14:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG

;-------------------------------------------------------------------------
;	DoPrmSgc(prm, prgbProps, sgc)
;-------------------------------------------------------------------------
; %%Function:N_DoPrmSgc %%Owner:BRADV
cProc	N_DoPrmSgc,<PUBLIC,FAR>,<si,di>
	ParmW	prm
	ParmW	prgbProps
	ParmW	sgc
; /* D O  P R M  S G C */
; /* apply prm to prgbProps that is of type sgc */
; native DoPrmSgc(prm, prgbProps, sgc)
; struct PRM prm; int sgc; char *prgbProps;
; {
;	  int cch;
;	  char *pprl;
;	  char grpprl[2];
cBegin

	mov	cx,[prm]
	mov	di,[prgbProps]
	mov	dx,[sgc]
	cCall	LN_DoPrmSgc,<>
; }
cEnd


;	LN_DoPrmSgc takes prm in cx,
;	prgbProps in di, sgc in dx.
;	ax, bx, cx, dx, si, di are altered.
; %%Function:LN_DoPrmSgc %%Owner:BRADV
PUBLIC LN_DoPrmSgc
LN_DoPrmSgc:

;	  if (prm.fComplex)
	mov	bx,cx
	errnz	<maskfComplexPrm-1>
	test	bl,maskfComplexPrm
	jz	DPS01

;		  {
;		  struct  PRC *pprc;
;
;		  pprc = *HprcFromPprmComplex(&prm);
;#define HprcFromPprmComplex(pprm) ((struct PRC **)((pprm)->cfgrPrc<<1))
	errnz <maskcfgrPrcPrm - 0FFFEh>
	and	bl,0FEh
	mov	bx,[bx]

;		  cch = pprc->bprlMac;
	mov	cx,[bx.bprlMacPrc]

;		  pprl = pprc->grpprl;
	lea	si,[bx.grpprlPrc]
	push	ax	;room for grpprl (not used by complex prms)
;		  }
	jmp	short DPS02

DPS01:
;	  else
;		  {
; /* cch = 1 will pick up one sprm, no matter what its length */
;		  cch = 1;
	mov	cx,1
;		  grpprl[0] = prm.sprm;
;		  grpprl[1] = prm.val;
;   assumed bx = prm
	errnz <(sprmPrm) - 0>
	errnz <masksprmPrm - 000FEh>
	errnz <(valPrm) - 1>
	shr	bl,1
	push	bx
;		  pprl = grpprl;
	mov	si,sp
;		  }
DPS02:
;	  ApplyPrlSgc((char HUGE *)pprl, cch, prgbProps, sgc);
	mov	ax,sbDds
	push	ax
	push	si
	push	cx
	push	di
	push	dx
ifdef DEBUG
	cCall	S_ApplyPrlSgc,<>
else ;not DEBUG
	call	far ptr N_ApplyPrlSgc
endif ;DEBUG

	pop	dx	;remove "grpprl" from stack
; }
	ret


APS_CALL label word
	dw	APS_spraBit
	dw	APS_spraByte
	dw	APS_spraWord
	dw	APS_spraCPlain
	dw	APS_spraCFtc
	dw	APS_spraCKul
	dw	APS_spraCSizePos
	dw	APS_spraSpec
	dw	APS_spraIco
	dw	APS_spraCHpsInc
	dw	APS_spraCHpsPosAdj


;-------------------------------------------------------------------------
;	N_ApplyPrlSgc(hpprlFirst, cch, prgbProps, sgc)
;-------------------------------------------------------------------------
; %%Function:N_ApplyPrlSgc %%Owner:BRADV
cProc	N_ApplyPrlSgc,<PUBLIC,FAR>,<si,di>
	ParmD	hpprlFirst
	ParmW	cch
	ParmW	prgbProps
	ParmW	sgc

; /* A P P L Y	P R L  S G C */
; /* apply sprms of type sgc in grpprl of length cch to prgbProps */
; native ApplyPrlSgc(pprl, cch, prgbProps, sgc)
; char *pprl; struct CHP *prgbProps; int cch, sgc;
; {
;	  int val, val2;
;	  struct SIAP siap;
;	  CHAR HUGE *hpprl = hpprlFirst;
cBegin
	mov	si,[OFF_hpprlFirst]
	mov	di,[prgbProps]
	mov	dx,[sgc]
	mov	cl,ibitSgcEsprm
	shl	dl,cl
	mov	cx,[cch]
	;APS16 performs ReloadSb for hpprlFirst.  bx, es are altered.
	call	APS16

; cx = cch, dl = sgc shl ibitsgcEsprm, es:si = qprl, di = prgbProps
;	  while (cch > 0)
;		  {
	or	cx,cx
	jle	APS04

APS01:
;		  int cchSprm;
;		  struct ESPRM esprm;
;
;		  Assert(*qprl < sprmMax);
ifdef DEBUG
;	/* Assert (*qprl < sprmMax) with a call
;	so as not to mess up short jumps */
	call	APS11
endif ;/* DEBUG */
;		  esprm = dnsprm[*qprl];
;/*	 if we encounter a pad character at the end of a grpprl of a PAPX we
;   set the length to 1 and continue. */
;		 if (sprm == 0)
;			 {
;			 cchSprm = 1;
;			 goto LNext;
;			 }
	mov	al,es:[si]
	inc	si					; es:si = qprl + 1
	xor	ah,ah
	mov	bx,ax
	inc	ax
	or	bl,bl
	je	APS03
	errnz	<cbEsprmMin - 4>
	shl	bx,1
	shl	bx,1
	errnz	<maskCchEsprm AND 0FF00h>
	mov	al,[bx.dnsprm.cchEsprm]
	mov	bx,[bx.dnsprm]				; bx = esprm

;		if ((cchSprm = esprm.cch) == 0)
;			{
	and	ax,maskCchEsprm
	errnz	<ibitcchEsprm - 0>
	jnz	APS02
;			if (sprm == sprmTDefTable)
;				bltbh(hpprl + 1, &cchSprm, sizeof(int));
	mov	ax,es:[si]
	cmp	bptr es:[si-1],sprmTDefTable
	je	APS015
;			else
;				{
;				cchSprm = val;
	xor	ah,ah

;				if (cchSprm == 255 && sprm == sprmPChgTabs)
;					{
	cmp	bptr es:[si-1],sprmPChgTabs
	jne	APS015

;					char HUGE *hpprlT;
;					cchSprm = (*(hpprlT = hpprl + 2) * 4) + 1;
;					cchSprm += (*(hpprlT + cchSprm) * 3) + 1;
;					}
;				}
;	Assert (cchSprm > 255 || cchSprm == esprm.cch);
ifdef DEBUG
;	/* Assert (cchSprm > 255 || cchSprm == esprm.cch) with a call
;	so as not to mess up short jumps */
	call	APS17
endif ;/* DEBUG */
	push	bx
	mov	bl,es:[si+1]
	xor	bh,bh
	shl	bx,1
	shl	bx,1
	mov	al,es:[bx+si+2]
	add	bx,ax
	inc	ax
	shl	ax,1
	add	ax,bx
	pop	bx

;			cchSprm += 2;
;			}
APS015:
	add	ax,2

;			 }
APS02:

; ax = cchSprm, bx = esprm, cx = cch, dl = sgc shl ibitsgcEsprm,
; es:si = qprl+1, di = prgbProps

;		  if (esprm.sgc == sgc)
	errnz	<(sgcEsprm) - 1>
	xor	bh,dl
	test	bh,masksgcEsprm
	jne	APS03
	push	ax	;save cchSprm
	push	cx	;save cch
	push	dx	;save sgc

;		  val = *(qprl + 1);
	mov	dh,es:[si]				; ch = val

;			  {
;			  switch (esprm.spra)
	mov	ax,bx
	errnz	<(spraEsprm) - 1>
	mov	bl,bh
	and	bx,maskspraEsprm
	errnz	<ibitSpraEsprm>
	shl	bx,1

;	Assert (spra < 11);
ifdef DEBUG
;	/* Assert (spra < 11) with a call
;	so as not to mess up short jumps */
	call	APS13
endif ;/* DEBUG */

;	all of the switch cases return to APS03
; ax = esprm, dh = val, si = qprl+1, di = prgbProps
	call	[bx.APS_CALL]
	pop	dx	;restore sgc
	pop	cx	;restore cch
	pop	ax	;restore cchSprm
;				  {

;LNext:
APS03:
;	This code from after the switch is performed below in the C version.
;		  qprl += cchSprm;
; ax = cchSprm, cx = cch, dl = sgc shl ibitsgcEsprm, es:si = qprl+1
; di = prgbProps
	add	si,ax
	dec	si

;		  cch -= cchSprm;
	sub	cx,ax
	jg	APS01
;		  }

APS04:
; }
cEnd


; ax = esprm, dh = val, es:si = qprl+1, di = prgbProps
APS_spraWord:
;			  case spraWord:
; /* sprm has a word parameter that is to be stored at b */
;				  bltbx(LpFromHp(pprl+1), (CHAR *)prgbProps + esprm.b, 2);
	xchg	ax,bx
	and	bx,maskbEsprm
	errnz	<maskbEsprm - 0FEh>
	errnz	<(bEsprm) - 0>
	shr	bx,1
	mov	ax,es:[si]
	mov	[bx.di],ax
;				  break;
	ret


; ax = esprm, dh = val, es:si = qprl+1, di = prgbProps
APS_spraByte:
;			  case spraByte:
; /* sprm has a byte parameter that is to be stored at b */
;				  *((char *)prgbProps + esprm.b) = val;
	xchg	ax,bx
	and	bx,maskbEsprm
	errnz	<maskbEsprm - 0FEh>
	errnz	<(bEsprm) - 0>
	shr	bx,1
	mov	[bx.di],dh
;				  break;
	ret


; ax = esprm, dh = val, es:si = qprl+1, di = prgbProps
APS_spraBit:
;			  case spraBit:
;				maskBit = (1<<(shftBit = esprm.b));
	mov	cl,al
	and	cl,maskbEsprm
	errnz	<ibitbEsprm-1>
	errnz	<(bEsprm) - 0>
	shr	cl,1		
	; cl = shftBit
	mov	bx,1
	shl	bx,cl

;/* if the high bit of the operand is on and the low bit of the operand is off
;   we will make prop to be same as vchpStc prop. if the high operand bit is on
;   and the low operand bit is on, we will set the prop to the negation of the
;   vchpStc prop. */
;				if (val & 0x80)
;					{
;					*((int *)prgbProps) &= ~maskBit;
;				*((int *)prgbProps) |=
;					(((*(int *)&vchpStc) & maskBit) ^
;					((val & 0x01)<<shftBit));
;					}
;				else if (val == 0)
;					*((int *)prgbProps) &= ~(maskBit);
;				else
;					*((int *)prgbProps) |= maskBit;
;				break;
; ax = esprm, dh = val, es:si = qprl+1, di = prgbProps,
; cl = shftBit, bx = maskBit
	or	dh,dh
	js	APS045
	jnz	APS05
APS045:
	not	bx
	and	[di],bx
	or	dh,dh
	jns	APS06
	not	bx
	and	bx,wptr ([vchpStc])
	xor	ax,ax
	shr	dh,1
	inc	cx
	rcl	ax,cl
	xor	bx,ax
APS05:
	or	[di],bx
APS06:

;				  break;
	ret


; ax = esprm, dh = val, es:si = qprl+1, di = prgbProps
APS_spraCPlain:
;			  case spraCPlain:
;				 /*  fSpec and fFldVanish are properties that
;				     the user is not allowed to modify! */
;				 val = (*(int *)prgbProps) & maskFNonUser;
	mov	dx,wptr [di]
	and	dx,maskfNonUser

;				 blt(&vchpStc, prgbProps, cwCHPBase);
	push	di
	push	es
	push	ds
	pop	es
	mov	ax,dataOffset vchpStc
	xchg	ax,si
	errnz	<cwCHPBase - 4>
	movsw
	movsw
	movsw
	movsw
	xchg	ax,si
	pop	es
	pop	di

;				 Assert((*(int *)&vchpStc & maskFNonUser) == 0);
ifdef DEBUG
	test	wptr [vchpStc],maskfNonUser
	je	APS063
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFetchn3
	mov	bx,701
	cCall	AssertProcForNative,<ax, bx>
	;APS16 performs ReloadSb for hpprlFirst.  bx, es are altered.
	call	APS16
	pop	dx
	pop	cx
	pop	bx
	pop	ax
APS063:
endif ;/* DEBUG */

;				 (*(int *)prgbProps) |= val;
	or	wptr [di],dx

;				 break;
	ret


; ax = esprm, dh = val, es:si = qprl+1, di = prgbProps
APS_spraCFtc:
;			  case spraCFtc:
;				  bltb(LpFromHp(pprl+1), &val, 2);
	mov	ax,es:[si]
;				  prgbProps->ftc = val;
	mov	[di.ftcChp],ax
;				  break;
	ret


; ax = esprm, dh = val, es:si = qprl+1, di = prgbProps
APS_spraCKul:
;			  case spraCKul:
;				  Assert(val <= 7); /* hand native assumption */
ifdef DEBUG
	cmp	dh,7
	jbe	APS067
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFetchn3
	mov	bx,702
	cCall	AssertProcForNative,<ax, bx>
	;APS16 performs ReloadSb for hpprlFirst.  bx, es are altered.
	call	APS16
	pop	dx
	pop	cx
	pop	bx
	pop	ax
APS067:
endif ;/* DEBUG */

;				  prgbProps->kul = val;
	mov	cl,ibitKulChp
	shl	dh,cl
	and	[di.kulChp],NOT maskKulChp
	or	[di.kulChp],dh
;				  break;
	ret


; ax = esprm, dh = val, es:si = qprl+1, di = prgbProps
APS_spraCSizePos:
;			  case spraCSizePos:
;				  bltb(LpFromHp(pprl+1), &siap, cbSIAP);
;				  if ((val = siap.hpsSize) != 0)
	mov	al,es:[si.hpsSizeSiap]
	or	al,al
	jz	APS07
;					  prgbProps->hps = val;
	mov	[di.hpsChp],al

APS07:
;				  if ((val = siap.cInc) != 0)
	mov	al,es:[si.cIncSiap]
	errnz	<maskcIncSiap - 0007Fh>
	shl	al,1
	jz	APS08
;					  prgbProps->hps = HpsAlter(prgbProps->hps,
;						  val >= 64 ? val - 128 : val);
	mov	cl,1
	;APS15 performs prgbProps->hps
	;= HpsAlter(prgbProps->hps, (al >= 128 ? al - 256 : al) >> cl);
	;ax, bx, cx, dx are altered.
	call	APS15

APS08:
;				  if ((val = siap.hpsPos) != hpsPosNil)
	errnz	<(hpsPosSiap) - (fAdjSiap) - 1)
	mov	ax,wptr (es:[si.fAdjSiap])
	cmp	ah,hpsPosNil
	je	APS10
;					  {
;					  if (siap.fAdj)
	push	ax	;save val
	test	al,maskfAdjSiap
	jz	APS09

;						  {
;						  if (val != 0)
;							  { /* Setting pos to super/sub */
;							  if (prgbProps->hpsPos == 0)
;								  prgbProps->hps = HpsAlter(prgbProps->hps, -1);
;							  }
;						  else
;							  { /* Restoring pos to normal */
;							  if (prgbProps->hpsPos != 0)
;								  prgbProps->hps = HpsAlter(prgbProps->hps, 1);
;							  }
APS085:
	cmp	ah,1
	rcr	al,1
	cmp	[di.hpsPosChp],0
	rcr	ah,1
	xor	al,ah
;	do nothing if ((val == 0) ^ (prgbProps->hpsPos) == 0) == fFalse
	jns	APS09
	cwd	    ;dx = (prgbProps->hpsPos == 0 ? -1 : 0)
	shl	dx,1
	inc	dx  ;dx = (prgbProps->hpsPos == 0 ? -1 : 1)
	mov	al,[di.hpsChp]
	xor	ah,ah
	cCall	HpsAlter,<ax,dx>
	;APS16 performs ReloadSb for hpprlFirst.  bx, es are altered.
	call	APS16
	mov	[di.hpsChp],al

APS09:
;					  prgbProps->hpsPos = val;
	pop	ax	;restore val
	mov	[di.hpsPosChp],ah
;					  }
APS10:
;				  break;
	ret

;			case spraCHpsInc:
;				val &= 255;
;				prgbProps->hps = HpsAlter(prgbProps->hps,
;					val >= 128 ? val - 256 : val);
;				break;
APS_spraCHpsInc:
	mov	al,dh
	mov	cl,0
	;APS15 performs prgbProps->hps
	;= HpsAlter(prgbProps->hps, (al >= 128 ? al - 256 : al) >> cl);
	;ax, bx, cx, dx are altered.
	call	APS15
	ret

;			case spraCHpsPosAdj:
;				if (val != 0)
;					{ /* Setting pos to super/sub */
;
;					if (prgbProps->hpsPos == 0)
;					prgbProps->hps = HpsAlter(prgbProps->hps, -1);
;					}
;				else
;					{ /* Restoring pos to normal */
;					if (prgbProps->hpsPos != 0)
;						prgbProps->hps = HpsAlter(prgbProps->hps, 1);
;					}
;				prgbProps->hpsPos = val;
;				break;
APS_spraCHpsPosAdj:
	mov	ah,dh
	push	ax	;save val
	jmp	short APS085

; ax = esprm, dh = val, es:si = qprl+1, di = prgbProps
APS_spraIco:
;			  case spraCIco:
;				  prgbProps->ico = val;
	errnz	<maskIcoChp - 00Fh>
	and	dh,maskIcoChp
	and	[di.icoChp],NOT maskIcoChp
	or	[di.icoChp],dh
;				  break;
	ret

; #ifdef DEBUG
;			  default:Assert(fFalse); /* hand native assumption */
; #endif
; /* other special actions */


; ax = esprm, dh = val, es:si = qprl+1, di = prgbProps
APS_spraSpec:
;			  case spraSpec:
;/* if sprmPStcPermute is the first sprm of a grpprl, it would have been
;   interpreted in CachePara before the papx grpprl was applied. */
;				 if (hpprl != hpprlFirst || sprm != sprmPStcPermute)
;					 ApplySprm(hpprl, sprm, val, prgbProps);
	dec	si
	mov	bx,si
	lods	bptr es:[si]
	cmp	bx,[OFF_hpprlFirst]
	jne	APS103
	cmp	al,sprmPStcPermute
	je	APS107
APS103:
	push	[SEG_hpprlFirst]
	push	bx
	xor	ah,ah
	push	ax
	mov	al,dh
	push	ax
	push	di
	cCall	ApplySprm,<>
	;APS16 performs ReloadSb for hpprlFirst.  bx, es are altered.
	call	APS16
APS107:
	ret


;				  }
;			  }
;	The following code is done above in the assembler version
;	at APS03
;		  cch -= cchSprm;
;		  pprl += cchSprm;
;		  }
; }

ifdef DEBUG
;		  Assert(*qprl < sprmMax);
APS11:
	cmp	byte ptr es:[si],sprmMax
	jb	APS12
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFetchn3
	mov	bx,700
	cCall	AssertProcForNative,<ax, bx>
	;APS16 performs ReloadSb for hpprlFirst.  bx, es are altered.
	call	APS16
	pop	dx
	pop	cx
	pop	bx
	pop	ax
APS12:
	ret
endif ;DEBUG

ifdef DEBUG
;		  Assert(spra < 11);
APS13:
	cmp	bx,11 SHL 1
	jb	APS14
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFetchn3
	mov	bx,1021
	cCall	AssertProcForNative,<ax, bx>
	;APS16 performs ReloadSb for hpprlFirst.  bx, es are altered.
	call	APS16
	pop	dx
	pop	cx
	pop	bx
	pop	ax
APS14:
	ret
endif ;DEBUG


	;APS15 performs prgbProps->hps
	;= HpsAlter(prgbProps->hps, (al >= 128 ? al - 256 : al) >> cl);
	;ax, bx, cx, dx are altered.
APS15:
	mov	bl,[di.hpsChp]
	xor	bh,bh
	cbw
	sar	ax,cl
	cCall	HpsAlter,<bx,ax>
	;APS16 performs ReloadSb for hpprlFirst.  bx, es are altered.
	call	APS16
	mov	[di.hpsChp],al
	ret


	;APS16 performs ReloadSb for hpprlFirst.  bx, es are altered.
APS16:
	mov	bx,[SEG_hpprlFirst]
	;LN_ReloadSb takes an sb in bx and set es to the corresponding value.
	;Only es and bx are altered.
	call	LN_ReloadSb
	ret

ifdef DEBUG
;	Assert (cchSprm > 255 || cchSprm == esprm.cch);
APS17:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	cx,ax	;save old cchSprm
	mov	bl,es:[si+1]
	xor	bh,bh
	shl	bx,1
	shl	bx,1
	mov	al,es:[bx+si+2]
	add	bx,ax
	inc	ax
	shl	ax,1
	add	ax,bx
	cmp	ax,255
	ja	APS18
	cmp	ax,cx
	je	APS18
	mov	ax,midFetchn3
	mov	bx,1024
	cCall	AssertProcForNative,<ax, bx>
APS18:
	;APS16 performs ReloadSb for hpprlFirst.  bx, es are altered.
	call	APS16
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG


;
;
;-------------------------------------------------------------------------
;	FetchCpAndPara(doc, cp, fcm)				GregC
;-------------------------------------------------------------------------
; %%Function:FetchCpAndPara %%Owner:BRADV
cProc	FetchCpAndPara,<PUBLIC,FAR>,<>
	ParmW	doc
	ParmD	cp
	ParmW	fcm
; /* F E T C H	C P  A N D  P A R A */
; native FetchCpAndPara( doc, cp, fcm )
; int doc;
; CP cp;
; int fcm;
; {
cBegin
;  CachePara( doc, cp );
;  FetchCp( doc, cp, fcm );
	mov	bx,[doc]
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	push	bx
	push	dx
	push	ax
	push	[fcm]
	push	bx
	push	dx
	push	ax
ifdef DEBUG
	cCall	S_CachePara,<>
	cCall	S_FetchCp,<>
else ;not DEBUG
	cCall	N_CachePara,<>
	cCall	N_FetchCp,<>
endif ;DEBUG
; }
cEnd
;
;-------------------------------------------------------------------------
;	FetchCpAndParaCa(pca,fcm)
;-------------------------------------------------------------------------
; %%Function:FetchCpAndParaCa %%Owner:BRADV
cProc	FetchCpAndParaCa,<PUBLIC,FAR>,<si>
	ParmW	pca
	ParmW	fcm
;/* F E T C H  C P  A N D  P A R A  C A */
;native FetchCpAndParaCa( pca, fcm )
;struct CA *pca;
;int fcm;
;{
cBegin
;	CacheParaCa(pca);
;	FetchCp(pca->doc, pca->cpFirst, fcm);
	mov	si,[pca]
	errnz <(cpFirstCa) - 0>
	lodsw
	xchg	ax,dx
	lodsw
	mov	cx,[si.docCa-4]
	push	cx
	push	ax
	push	dx
	push	[fcm]
	push	cx
	push	ax
	push	dx
ifdef DEBUG
	cCall	S_CachePara,<>
	cCall	S_FetchCp,<>
else ;not DEBUG
	cCall	N_CachePara,<>
	cCall	N_FetchCp,<>
endif ;DEBUG
;}
cEnd

;-------------------------------------------------------------------------
;	CacheParaCa(pca)
;-------------------------------------------------------------------------
; %%Function:CacheParaCa %%Owner:BRADV
cProc	CacheParaCa,<PUBLIC,FAR>,<>
	ParmW	pca
;/* C A C H E  P A R A  C A */
;/* alternative entry point */
;CacheParaCa(pca)
;struct CA *pca;
;{
cBegin
;	CachePara(pca->doc, pca->cpFirst);
	mov	bx,[pca]
ifdef DEBUG
	cCall	S_CachePara,<[bx.docCa],[bx.HI_cpFirstCa],[bx.LO_cpFirstCa]>
else ;not DEBUG
	cCall	N_CachePara,<[bx.docCa],[bx.HI_cpFirstCa],[bx.LO_cpFirstCa]>
endif ;DEBUG
;}
cEnd

;-------------------------------------------------------------------------
;	ChFetch(pca)
;-------------------------------------------------------------------------
; %%Function:ChFetch %%Owner:BRADV
cProc	ChFetch,<PUBLIC,FAR>,<>
	ParmW	doc
	ParmD	cp
	ParmW	fcm
;/* C H  F E T C H  */
;NATIVE ChFetch(doc, cp, fcm)
;int doc;
;CP cp;
;int fcm;
;{
;	 int ch;
cBegin

;	 if (fcm != fcmChars)
	mov	ax,[fcm]
	mov	bx,[OFF_cp]
	mov	cx,[SEG_cp]
	mov	dx,[doc]
	push	dx
	push	cx
	push	bx
	push	ax	;args for FetchCp
	cmp	ax,fcmChars
	je	CF01

;		 CachePara(doc, cp);
ifdef DEBUG
	cCall	S_CachePara,<dx,cx,bx>
else ;not DEBUG
	cCall	N_CachePara,<dx,cx,bx>
endif ;DEBUG
CF01:

;	 FetchCp(doc, cp, fcm);
ifdef DEBUG
	cCall	S_FetchCp,<>
else ;not DEBUG
	cCall	N_FetchCp,<>
endif ;DEBUG

;	 ch = *vhpchFetch;
	mov bx,whi ([vhpchFetch])
	;LN_ReloadSb takes an sb in bx and set es to the corresponding value.
	;Only es and bx are altered.
	call	LN_ReloadSb
	mov	bx,wlo ([vhpchFetch])

;	 return (ch);
	xor	ax,ax
	mov	al,es:[bx]

;}
cEnd


;-------------------------------------------------------------------------
;	FAbsPap(doc,ppap)
;-------------------------------------------------------------------------
; %%Function:FAbsPap %%Owner:BRADV
PUBLIC FAbsPap
FAbsPap:
;/*********************/
;/* F	A b s	P a p */
;NATIVE int FAbsPap(doc, ppap)
;int doc;
;struct PAP *ppap;
;{
;/* returns fTrue if pap describes an absolutely positioned object */
;	 struct DOD *pdod = PdodDoc(doc);
;

	mov	bx,sp
	mov	dx,[bx+4]
	mov	bx,[bx+6]
	;doc in bx, ppap in dx
;	PdodOrNilFromDoc takes a doc in bx, and returns hNil in ax and bx
;	if mpdochdod[doc] == hNil.  Otherwise it returns doc in ax
;	and pdod = *mpdochdod[doc] in bx.  The zero flag is set according
;	to the value of bx upon return.  Only ax and bx are altered.
	call	LN_PdodOrNilFromDoc
ifdef DEBUG
	jnz	FAP01
	inc	bp
	push	bp
	mov	bp,sp	;set up bp chain for call
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFetchn3
	mov	bx,1022
	cCall	AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	bp	;restore old bp chain
	dec	bp
FAP01:
endif ;DEBUG

;	int fAbs = doc != vdocTemp && !pdod->fFtn Win (&& !pdod->fAtn) &&
;		(ppap->dxaAbs != 0 || ppap->dyaAbs != 0 || 
;		ppap->pcHorz != pcHColumn || ppap->dxaWidth != 0);
;
;	 return(fAbs);
	errnz	<fFalse>
        mov     ax,[vdocTemp]
        sub     ax,[doc]
	jz	FAP02
	xor	ax,ax	;return fFalse
	errnz	<(fFtnDod) - (fAtnDod)>
	test	[bx.fFtnDod],maskfFtnDod+maskfAtnDod
	jnz	FAP02
	mov	bx,dx
	mov	cl,[bx.pcHorzPap]
	errnz	<pcHColumn - 0>
	and	cx,maskPcHorzPap
	or	cx,[bx.dxaAbsPap]
	or	cx,[bx.dyaAbsPap]
	or	cx,[bx.dxaWidthPap]
	jz	FAP02
	errnz	<fTrue - fFalse - 1>
	inc	ax
FAP02:

; }
	db	0CAh, 004h, 000h    ;far ret, pop 4 bytes


;-------------------------------------------------------------------------
;	FInTableDocCp(doc, cp)
;-------------------------------------------------------------------------
;/* F  I N  T A B L E  D O C  C P */
;/* Returns whether a doc,cp is in a table according to field structures.
;	vcpFirstTablePara is set to the cpFirst of the table in this paragraph.
;	vcpFirstCellPara is set to the cpFirst of the table cell.
;*/
;/* %%Function:FInTableDocCp %%Owner:davidlu */
;HANDNATIVE BOOL C_FInTableDocCp(doc, cp)
;int doc;
;CP cp;
;{
;	extern struct CA caPara;
;	extern struct PAP vpapFetch;
;	extern CP vcpFirstTablePara;
;	extern CP vcpFirstTableCell;
;	extern CP vmpitccp[];
;	int icp;
;	CP cpFirstCell, cpLimCell;
;	int docFetchSav;
;	CP cpFetchSav;
;	int fcmFetchSav;

Ltemp001:
	jmp		FITDC09

; %%Function:N_FInTableDocCp %%Owner:BRADV
cProc	N_FInTableDocCp,<PUBLIC,FAR>,<si,di>
	ParmW	doc
	ParmD	cp

cBegin

;	CachePara(doc, cp);
ifdef DEBUG
	cCall	S_CachePara,<[doc], [SEG_cp], [OFF_cp]>
else ;not DEBUG
	cCall	N_CachePara,<[doc], [SEG_cp], [OFF_cp]>
endif ;DEBUG

;	vcpFirstTableCell = vcpFirstTablePara = caPara.cpFirst;
	mov		cx,[caPara.LO_cpFirstCa]
	mov		dx,[caPara.HI_cpFirstCa]

;	if (!vpapFetch.fInTable)
;		return fFalse;
	errnz	<fFalse>
	xor		ax,ax
	mov		bx,fTrue		;Do this to allow code sharing at FITDC09
	cmp		[vpapFetch.fInTablePap],al
	je		Ltemp001

;	if (!vtapFetch.fCaFull || !FInCa(doc, caPara.cpLim - 1, &caTap))
;		{
	mov		si,dataoffset [caTap]
	mov		di,[doc]
	mov		cx,0FFFFh
	mov		dx,cx
	add		cx,[caPara.LO_cpLimCa]
	adc		dx,[caPara.HI_cpLimCa]
	;LN_FInCa takes a doc in di, a cp in dx:cx and a pca in si and
	;returns the result in bx.  ax, bx, cx and dx are altered.
	push	dx
	push	cx		;save caPara.cpLim - 1;
	call 	LN_FInCa
	pop		cx
	pop		dx		;restore caPara.cpLim - 1;
	or		bx,bx
	je		FITDC005
	test	[vtapFetch.fCaFullTap],maskFCaFullTap
	jne		FITDC05
FITDC005:

;		docFetchSav = vdocFetch;
;		cpFetchSav = vcpFetch;
;		fcmFetchSav = fcmFetch;
	push	[fcmFetch]		;fcmFetchSav
	push	whi [vcpFetch]
	push	wlo	[vcpFetch]	;cpFetchSav
	push	[vdocFetch]		;docFetchSav

;		/* Optimization to reduce time spent by CpFirstTap looking for
;		   the beginning of the row. */
;		if (CpFirstTap(doc, caPara.cpFirst) == cpNil)
	push	dx
	push	cx			;save caPara.cpLim - 1
	cCall	CpFirstTap,<di,[caPara.HI_cpFirstCa],[caPara.LO_cpFirstCa]>
	and	ax,dx			;returned cpNil if it failed
	inc	ax
	pop	cx
	pop	dx
	jne	FITDC008

;			CpFirstTap(doc, caPara.cpLim - 1);
	cCall	CpFirstTap,<di, dx, cx>

;		if (docFetchSav != docNil && cpFetchSav <= CpMacDocEdit(docFetchSav))
;			FetchCpAndPara(docFetchSav, cpFetchSav, fcmFetchSav);
;		else
;			vdocFetch = docNil;
FITDC008:
	pop		si				;docFetchSav
	or		si,si
	je		FITDC01
	cCall	CpMacDocEdit,<si>
FITDC01:
	pop		bx
	pop		cx		;cx:bx = cpFetchSav
	pop		di		;fcmFetchSav
	errnz	<docNil>
	or		si,si
	je		FITDC02
	sub		ax,bx
	sbb		dx,cx
	jge		FITDC03
FITDC02:
	mov		[vdocFetch],docNil
	jmp		short FITDC04
FITDC03:
	cCall	FetchCpAndPara,<si,cx,bx,di>
FITDC04:

;		CachePara(doc, cp);
ifdef DEBUG
	cCall	S_CachePara,<[doc], [SEG_cp], [OFF_cp]>
else ;not DEBUG
	cCall	N_CachePara,<[doc], [SEG_cp], [OFF_cp]>
endif ;DEBUG

;		}
FITDC05:

;	Assert(FInCa(doc, caPara.cpLim - 1, &caTap));
;	Assert(caTap.cpFirst == vmpitccp[0]);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov		ax,-1
	cwd
	add		ax,[caPara.LO_cpLimCa]
	adc		dx,[caPara.HI_cpLimCa]
	mov		bx,dataoffset [caTap]
	cCall	FInCa,<[doc],dx,ax,bx>
	or		ax,ax
	jne		FITDC06
	mov		ax,midFetchn3
	mov		bx,1023
	cCall	AssertProcForNative,<ax, bx>
FITDC06:
	mov		ax,[caTap.LO_cpFirstCa]
	mov		dx,[caTap.HI_cpFirstCa]
	sub		ax,wlo [vmpitccp]
	sbb		dx,whi [vmpitccp]
	or		ax,dx
	je		FITDC07
	mov		ax,midFetchn3
	mov		bx,1024
	cCall	AssertProcForNative,<ax, bx>
FITDC07:
	pop		dx
	pop		cx
	pop		bx
	pop		ax
endif ;DEBUG

;	for (icp = 0; cp >= vmpitccp[icp+1]; icp++);
	mov		bx,dataoffset [vmpitccp]
	mov		cx,[OFF_cp]
	mov		dx,[SEG_cp]
FITDC08:
	add		bx,4
	cmp		cx,[bx]
	mov		ax,dx
	sbb		ax,[bx+2]
	jge		FITDC08

	;Assembler note: compute fResult = (cp >= caTap.cpFirst) now because
	;we have cp handy.
	sub		cx,[caTap.LO_cpFirstCa]
	sbb		dx,[caTap.HI_cpFirstCa]
	cmc
	sbb		ax,ax

;	vcpFirstTableCell = vmpitccp[icp];
	mov		cx,[bx-4]
	mov		dx,[bx-2]

;	vcpFirstTablePara = FInCa(doc, vcpFirstTableCell, &caPara)
;		? vcpFirstTableCell : caPara.cpFirst;
	mov		si,dataoffset [caPara]
	mov		di,[doc]
	;LN_FInCa takes a doc in di, a cp in dx:cx and a pca in si and
	;returns the result in bx.  ax, bx, cx and dx are altered.
	push	dx
	push	cx		;save vcpFirstTableCell;
	push	ax		;save fResult
	call 	LN_FInCa
	pop		ax		;restore fResult
	pop		cx
	pop		dx		;restore vcpFirstTableCell;
FITDC09:
	mov		wlo [vcpFirstTableCell],cx
	mov		whi [vcpFirstTableCell],dx
	or		bx,bx
	jne		FITDC10
	mov		cx,[caPara.LO_cpFirstCa]
	mov		dx,[caPara.HI_cpFirstCa]
FITDC10:
	mov		wlo [vcpFirstTablePara],cx
	mov		whi [vcpFirstTablePara],dx

;	return (cp >= caTap.cpFirst);
;}

cEnd


sEnd	fetch
	end
