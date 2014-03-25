	include w2.inc
	include noxport.inc
	include consts.inc
	include structs.inc

createSeg	res_PCODE,resn2,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midResn2	equ 32	   ; module ID, for native asserts
endif ;/* DEBUG */

; EXPORTED LABELS

; EXTERNAL FUNCTIONS

externNP	<LN_LpFromPossibleHq>
externNP	<LN_LrgbstFromSttb>
externNP	<LN_StackAdjust>

ifdef DEBUG
externFP	<AssertProcForNative>
externFP    	<FCheckHandle>
externFP	<S_PdrFetch>
externFP	<S_FreePdrf>
externFP	<S_PtOrigin>
endif ;/* DEBUG */

; EXTERNAL DATA

sBegin  data

externW mpwwhwwd	; extern struct WWD	**mpwwhwwd[];
externW mpmwhmwd	; extern struct MWD	**mpmwhmwd[];
externW mpdochdod	; extern struct DOD	**mpdochdod[];
externW vhwwdOrigin	; struct WWD		**vhwwdOrigin;
externW vpdrfHead	; extern struct DRF	*vpdrfHead;
externW vitr		; extern struct ITR	vitr;
externW vfti		; extern struct FTI	vfti;
ifdef DEBUG
externW vfCheckPlc	; extern BOOL		vfCheckPlc;
externW vdbs		; extern struct DBS	vdbs;
externW wFillBlock
externW vsccAbove
externW vpdrfHeadUnused
endif ;/* DEBUG */

sEnd    data


; CODE SEGMENT res_PCODE

sBegin	resn2
	assumes cs,resn2
	assumes ds,dgroup
	assume es:nothing
	assume ss:nothing

include asserth.asm


;-------------------------------------------------------------------------
;	CpMac1Doc
;-------------------------------------------------------------------------
;/* C P  M A C 1  D O C */
;native CP CpMac1Doc(doc)
;int doc;
;{
;	 struct DOD *pdod = PdodDoc(doc);
;	 return(pdod->cpMac - ccpEop );
;}
; %%Function:CpMac1Doc %%Owner:BRADV
PUBLIC	CpMac1Doc
CpMac1Doc:
	mov	al,-ccpEop
	db	03Dh	;turns next "mov al,immediate" into "cmp ax,immediate"

;-------------------------------------------------------------------------
;	CpMacDocEdit
;-------------------------------------------------------------------------
;/* C P  M A C	D O C  E D I T*/
;native CP CpMacDocEdit(doc)
;int doc;
;{
;	 return(CpMacDoc(doc) - ccpEop);
;}
; %%Function:CpMacDocEdit %%Owner:BRADV
PUBLIC	CpMacDocEdit
CpMacDocEdit:
	mov	al,-3*ccpEop
	db	03Dh	;turns next "mov al,immediate" into "cmp ax,immediate"

;-------------------------------------------------------------------------
;	CpMac2Doc
;-------------------------------------------------------------------------
;/* C P  M A C 2  D O C  */
;native CP CpMac2Doc(doc)
;int doc;
;{
;	 return((*mpdochdod[doc])->cpMac);
;}
; %%Function:CpMac2Doc %%Owner:BRADV
PUBLIC	CpMac2Doc
CpMac2Doc:
	mov	al,0
	db	03Dh	;turns next "mov al,immediate" into "cmp ax,immediate"
    	
;-------------------------------------------------------------------------
;	CpMacDoc
;-------------------------------------------------------------------------
;/* C P  M A C	D O C */
;native CP CpMacDoc(doc)
;int doc;
;{
;	 struct DOD *pdod = PdodDoc(doc);
;	 return(pdod->cpMac - 2*ccpEop );
;}
; %%Function:CpMacDoc %%Owner:BRADV
PUBLIC	CpMacDoc
CpMacDoc:
	mov	al,-2*ccpEop

;-------------------------------------------------------------------------
;	CpMacDocEngine
;-------------------------------------------------------------------------
;	return ( PdodDoc(doc)->cpMac + al );
;	where al and ah represent those register values upon entry.
; %%Function:CpMacDocEngine %%Owner:BRADV
PUBLIC	CpMacDocEngine
CpMacDocEngine:
	pop cx
	pop dx	    ;far return address in dx:cx
	pop bx
	push dx
	push cx
	shl bx,1
	mov bx,[bx.mpdochdod]
	mov bx,[bx]
	cbw
	cwd
	add ax,[bx.LO_cpMacDod]
	adc dx,[bx.HI_cpMacDod]
; }
	db	0CBh	    ;far ret


;-------------------------------------------------------------------------
;	PcaSet
;-------------------------------------------------------------------------
;/* P C A  S E T */
;native struct CA *PcaSet(pca, doc, cpFirst, cpLim)
;struct CA *pca;
;int doc;
;CP cpFirst, cpLim;
;{
;	 pca->doc = doc;
;	 pca->cpFirst = cpFirst;
;	 pca->cpLim = cpLim;
;	 return(pca);
;}
; %%Function:PcaSet %%Owner:PcaSet
PUBLIC	PcaSet
PcaSet:
;LN_StackAdjust is a helper routine to reduce the size of routines
;in resn.asm.  It takes the number of words of arguments to
;the routine minus one in the byte following the call to LN_StackAdjust.
;It then moves the far return address and si and di to above the
;arguments, moves the top argument into bx, and puts the other arguments
;below the saved si and di.  This allows the use of short pop register
;instructions to access arguments passed to the caller of LN_StackAdjust.
;ax, bx, cx, dx are altered.
	call	LN_StackAdjust
	db	5
LN_PopCache:
	pop	[bx.LO_cpLimCa]
	pop	[bx.HI_cpLimCa]
	pop	[bx.LO_cpFirstCa]
	pop	[bx.HI_cpFirstCa]
	pop	[bx.docCa]
	xchg	ax,bx
	pop	di
	pop	si
	db	0CBh	    ;far ret

;-------------------------------------------------------------------------
;	PcaSetDcp
;-------------------------------------------------------------------------
;/* P C A  S E T  D C P */
;native struct CA *PcaSetDcp(pca, doc, cpFirst, dcp)
;struct CA *pca;
;int doc;
;CP cpFirst, dcp;
;{
;	 pca->doc = doc;
;	 pca->cpFirst = cpFirst;
;	 pca->cpLim = cpFirst + dcp;
;	 return(pca);
;}
; %%Function:PcaSetDcp %%Owner:BRADV
PUBLIC	PcaSetDcp
PcaSetDcp:
;LN_StackAdjust is a helper routine to reduce the size of routines
;in resn.asm.  It takes the number of words of arguments to
;the routine minus one in the byte following the call to LN_StackAdjust.
;It then moves the far return address and si and di to above the
;arguments, moves the top argument into bx, and puts the other arguments
;below the saved si and di.  This allows the use of short pop register
;instructions to access arguments passed to the caller of LN_StackAdjust.
;ax, bx, cx, dx are altered.
	call	LN_StackAdjust
	db	5
	pop	ax	    ;low dcp
	pop	dx	    ;high dcp
	pop	si
	pop	di
	add	ax,si	    ;low cpFirst
	adc	dx,di	    ;high cpFirst
	push	di
	push	si
	push	dx
	push	ax
	jmp	short LN_PopCache

;-------------------------------------------------------------------------
;	PcaSetWholeDoc
;-------------------------------------------------------------------------
;/* P C A  S E T  W H O L E  D O C */
;native struct CA *PcaSetWholeDoc(pca, doc)
;struct CA *pca;
;int doc;
;{
;	 return PcaSet(pca, doc, cp0, CpMacDocEdit(doc));
;}
; %%Function:PcaSetWholeDoc %%Owner:BRADV
PUBLIC	PcaSetWholeDoc
PcaSetWholeDoc:
;LN_StackAdjust is a helper routine to reduce the size of routines
;in resn.asm.  It takes the number of words of arguments to
;the routine minus one in the byte following the call to LN_StackAdjust.
;It then moves the far return address and si and di to above the
;arguments, moves the top argument into bx, and puts the other arguments
;below the saved si and di.  This allows the use of short pop register
;instructions to access arguments passed to the caller of LN_StackAdjust.
;ax, bx, cx, dx are altered.
	call	LN_StackAdjust
	db	1
	pop	di	;doc
	push	bx	;save pca
	push	di
	push	cs
	call	CpMacDocEdit
	pop	bx	;restore pca
	push	di
	xor	cx,cx
	push	cx
	push	cx
	push	dx
	push	ax
	jmp	short LN_PopCache

;-------------------------------------------------------------------------
;	PcaSetNil
;-------------------------------------------------------------------------
;/* P C A  S E T  N I L */
;native struct CA *PcaSetNil(pca)
;struct CA *pca;
;{
;	 return PcaSet(pca, docNil, cp0, cp0);
;}
; %%Function:PcaSetNil %%Owner:BRADV
PUBLIC	PcaSetNil
PcaSetNil:
;LN_StackAdjust is a helper routine to reduce the size of routines
;in resn.asm.  It takes the number of words of arguments to
;the routine minus one in the byte following the call to LN_StackAdjust.
;It then moves the far return address and si and di to above the
;arguments, moves the top argument into bx, and puts the other arguments
;below the saved si and di.  This allows the use of short pop register
;instructions to access arguments passed to the caller of LN_StackAdjust.
;ax, bx, cx, dx are altered.
	call	LN_StackAdjust
	db	0
	xor	cx,cx
	errnz	<docNil>
	push	cx
	push	cx
	push	cx
	push	cx
	push	cx
	jmp	short LN_PopCache

;-------------------------------------------------------------------------
;	PcaPoint
;-------------------------------------------------------------------------
;/* P C A  P O I N T */
;native struct CA *PcaPoint(pca, doc, cp)
;struct CA *pca;
;int doc;
;CP cp;
;{
;	 return PcaSet(pca, doc, cp, cp);
;}
; %%Function:PcaPoint %%Owner:BRADV
PUBLIC	PcaPoint
PcaPoint:
;LN_StackAdjust is a helper routine to reduce the size of routines
;in resn.asm.  It takes the number of words of arguments to
;the routine minus one in the byte following the call to LN_StackAdjust.
;It then moves the far return address and si and di to above the
;arguments, moves the top argument into bx, and puts the other arguments
;below the saved si and di.  This allows the use of short pop register
;instructions to access arguments passed to the caller of LN_StackAdjust.
;ax, bx, cx, dx are altered.
	call	LN_StackAdjust
	db	3
	pop	ax	    ;low cp
	pop	dx	    ;high cp
	push	dx
	push	ax
	push	dx
	push	ax
	jmp	short LN_PopCache

;-------------------------------------------------------------------------
;	DcpCa
;-------------------------------------------------------------------------
;/* D C P  C A */
;native CP DcpCa(pca)
;struct CA *pca;
;{
;	 return(pca->cpLim - pca->cpFirst);
;}
; %%Function:DcpCa %%Owner:BRADV
PUBLIC	DcpCa
DcpCa:
	pop	cx
	pop	dx	;far return address in dx:cx
	pop	bx
	push	dx
	push	cx
	mov	ax,[bx.LO_cpLimCa]
	mov	dx,[bx.HI_cpLimCa]
	sub	ax,[bx.LO_cpFirstCa]
	sbb	dx,[bx.HI_cpFirstCa]
	db	0CBh	    ;far ret


;-------------------------------------------------------------------------
;	PtOrigin
;-------------------------------------------------------------------------
;/* P T  O R I G I N */
;/* this procedure finds the origin in outermost l space of the p's in a frame
;identified by hpldr,idr.
;If idr == -1, it finds the origin for l's in hpldr.
;The outermost containing hwwd is returned in vhwwdOrigin.
;
;It is useful to remember:
;	 l's are frame relative coordinates of objects in frames, e.g.
;		 positions of dr's
;	 p's are object relative coordinates of parts of objects, e.g.
;		 lines of text, or frames of text.
;	 DR's are objects which may contain text and frames
;	 PLDR's are frames which can contain objects.
;*/
;NATIVE struct PT PtOrigin(hpldr, idr)
;struct PLDR **hpldr; int idr;
;{
;	 struct PT pt;
;	 struct DR *pdr;
;	 struct DRF drfFetch;

; %%Function:N_PtOrigin %%Owner:BRADV
cProc	N_PtOrigin,<PUBLIC,FAR>,<>
	ParmW	hpldr
	ParmW	idr

ifdef DEBUG
	LocalW	pdr
	LocalV	drfFetch,cbDrfMin
endif ;/* DEBUG */

cBegin
	mov	bx,[hpldr]
	mov	cx,[idr]

;	 pt.xp = pt.yp = 0;
	xor	ax,ax
	cwd

;	for (;;)
;		{
PO01:

ifdef DEBUG
;	 AssertH(hpl) with a jump so as not to
;	 mess up short jumps.
	jmp	short PO06
PO02:
endif ;DEBUG
	mov	[vhwwdOrigin],bx
	mov	bx,[bx]
;		 if (idr != -1)
;			 {
	cmp	cx,-1
	je	PO04

;			 Debug (pdr = PdrFetch(hpldr, idr, &drfFetch));
	;Do this with a call so as not to mess up short jumps
ifdef DEBUG
	call	PO07
endif ;DEBUG

;			 hpdr = HpInPl(hpldr, idr);
	push	di	;save caller's di
	push	dx	;save pt.yp
	push	ax	;save pt.xp
	mov	dx,[vhwwdOrigin]
	;LN_LpInPl takes hplFoo in dx and iFoo in cx to
	;return lpFoo in es:di.  bx, cx and si are not altered.
	call	LN_LpInPl
	pop	ax	;restore pt.xp
	pop	dx	;restore pt.yp

;			 Assert (pdr->xl == hpdr->xl);
;			 Assert (pdr->yl == hpdr->yl);
;			 Debug (FreePdrf(&drfFetch));
	;Do this debug stuff with a call so as not to mess up short jumps
ifdef DEBUG
	call	PO08
endif ;DEBUG

;			 pt.xp += pdr->xl;
;			 pt.yp += pdr->yl;
;			 }
	add	ax,es:[di.xlDr]
	add	dx,es:[di.ylDr]
	pop	di	;restore caller's di

PO04:

;		if ((*hpldr)->hpldrBack == hNil)
;			{
;			vhwwdOrigin = hpldr;
;			return pt;
;			}
	;Assembler note: we have already performed vhwwdOrigin = hpldr above.
	cmp	[bx.hpldrBackPldr],hNil
	je	PO05

;		 pt.xp += (*hpldr)->ptOrigin.xp;
	add	ax,[bx.ptOriginPldr.xpPt]

;		 pt.yp += (*hpldr)->ptOrigin.yp;
	add	dx,[bx.ptOriginPldr.ypPt]

;		 idr = (*hpldr)->idrBack;
;		 hpldr = (*hpldr)->hpldrBack;
	mov	cx,[bx.idrBackPldr]
	mov	bx,[bx.hpldrBackPldr]
;		 }
	jmp	short PO01

;}
PO05:
cEnd

;End of PtOrigin


ifdef DEBUG
PO06:
;	 AssertH(hpl);
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midResn2
	mov	cx,1000
	cCall	AssertHForNative,<bx, ax, cx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	jmp	short PO02
endif ;/* DEBUG */

;			 Debug (pdr = PdrFetch(hpldr, idr, &drfFetch));
PO07:
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	lea	ax,[drfFetch]
	push	[vhwwdOrigin]
	push	cx
	push	ax
	call	far ptr S_PdrFetch
	mov	[pdr],ax
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG


;			 Assert (pdr->xl == hpdr->xl);
;			 Assert (pdr->yl == hpdr->yl);
;			 Debug (FreePdrf(&drfFetch));
ifdef DEBUG
PO08:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	bx,[pdr]
	mov	ax,[bx.xlDr]
	cmp	ax,es:[di.xlDr]
	je	PO09
	mov	ax,midResn2
	mov	bx,1001 		   ;label # for native assert
	cCall	AssertProcForNative, <ax, bx>
PO09:
	mov	ax,[bx.ylDr]
	cmp	ax,es:[di.ylDr]
	je	PO10
	mov	ax,midResn2
	mov	bx,1002 		   ;label # for native assert
	cCall	AssertProcForNative, <ax, bx>
PO10:
	lea	ax,[drfFetch]
	push	ax
	call	far ptr S_FreePdrf
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG


;-------------------------------------------------------------------------
;	FcFromPn(pn)
;-------------------------------------------------------------------------
; /* F C  F R O M  P N */
; native FC FcFromPn(pn)
; PN	  pn;
; {
;	  return (((FC) pn) << shftSector);
; %%Function:FcFromPn %%Owner:BRADV
PUBLIC	FcFromPn
FcFromPn:
	pop	bx
	pop	cx	; ret add in cx:bx
	pop	ax
	push	cx
	push	bx
	xor	dx,dx
	errnz	<shftSector - 9>
	xchg	ah,dl
	xchg	al,ah
	shl	ax,1
	rcl	dx,1
; }
	db	0CBh	    ;far ret

;-------------------------------------------------------------------------
;	PnFromFc(fc)
;-------------------------------------------------------------------------
; /* P N  F R O M  F C */
; native PN PnFromFc(fc)
; FC	  fc;
; {
;	  return ((PN)	(fc >> shftSector));
; %%Function:PnFromFc %%Owner:BRADV
PUBLIC	PnFromFc
PnFromFc:
	pop	bx
	pop	cx	; ret add in cx:bx
	pop	ax
	pop	dx
	push	cx
	push	bx

;	Assert(fc < fcMax);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	sub	ax,LO_fcMax
	sbb	dx,HI_fcMax
	jl	PFF01
	mov	ax,midResn2
	mov	cx,1034
	cCall	AssertHForNative,<bx, ax, cx>
PFF01:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */
	mov	al,ah
	mov	ah,dl
	errnz	<shftSector - 9>
	shr	dh,1
	rcr	ax,1
; }
	db	0CBh	    ;far ret

;-------------------------------------------------------------------------
;	PstFromSttb(hsttb, i)
;-------------------------------------------------------------------------
; %%Function:PstFromSttb %%Owner:BRADV
ifdef DEBUG
PUBLIC PstFromSttb
PstFromSttb:
;	Assert(!psttb->fExternal);
	push	ax
	push	bx
	push	cx
	push	dx
	mov	bx,sp
	mov	bx,[bx+14]  ;/* hsttb */
	mov	bx,[bx]
	test	[bx.fExternalSttb],maskFExternalSttb
	je	PFS01
	mov	ax,midResn2
	mov	bx,1040 		   ;label # for native assert
	cCall	AssertProcForNative, <ax, bx>
PFS01:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	jmp	short HpstFromSttb
endif ;DEBUG

;-------------------------------------------------------------------------
;	FStcpEntryIsNull(hsttb, stcp)
;-------------------------------------------------------------------------
;FStcpEntryIsNull(hsttb, stcp)
;struct STTB **hsttb;
;int stcp;
;{
;	 return(*(HpstFromSttb(hsttb, stcp)) == 255);
;}
; %%Function:FStcpEntryIsNull %%Owner:BRADV
PUBLIC	FStcpEntryIsNull
FStcpEntryIsNull:
	mov	cx,1
	db	03Dh	;turns next "xor cx,cx" into "cmp ax,immediate"

;-------------------------------------------------------------------------
;	HpstFromSttb(hsttb, i)
;-------------------------------------------------------------------------
;/* H P S T  F R O M  S T T B */
;/* Return huge pointer to string i in hsttb */
;NATIVE CHAR HUGE *HpstFromSttb(hsttb, i)
;struct STTB **hsttb;
;int i;
;{
;	 struct STTB *psttb = *hsttb;

; %%Function:HpstFromSttb %%Owner:BRADV
ifndef DEBUG
PUBLIC PstFromSttb
PstFromSttb:
endif ;!DEBUG
PUBLIC	HpstFromSttb
HpstFromSttb:
	xor	cx,cx
HpstFromSttbCore:
	pop	ax
	pop	dx
	mov	bx,sp
	xchg	ax,[bx]
	xchg	dx,[bx+2]
	mov	bx,dx

;	 AssertH(hsttb);
;	 Assert(i < (*hsttb)->ibstMac);
ifdef DEBUG
	inc	bp
	push	bp
	mov	bp,sp	    ;set up bp chain for far call
	push	ds
	push	ax
	push	bx
	push	cx
	push	dx
	push	bx	;save hsttb
	push	ax	;save i
	mov	ax,midResn2
	mov	cx,1003
	cCall	AssertHForNative,<bx, ax, cx>
	pop	ax	;restore i
	pop	bx	;restore hsttb
	mov	bx,[bx]
	cmp	ax,[bx.ibstMacSttb]
	jb	HFS01
	mov	ax,midResn2
	mov	bx,1004 		   ;label # for native assert
	cCall	AssertProcForNative, <ax, bx>
HFS01:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	ds
	pop	bp
	dec	bp  ;restore bp
	push	ds  ;ds may now be a handle
	push	ss
	pop	ds  ;restore valid ds
endif ;DEBUG

;	 if (!psttb->fExternal)
;		 return (CHAR HUGE *)((char *)(psttb->rgbst) + psttb->rgbst[i]);
;	 else
;		 {
;		 int HUGE *hprgbst = HpOfHq(psttb->hqrgbst);
;		 return (CHAR HUGE *)hprgbst + hprgbst[i];
;		 }
	mov	bx,[bx]
	push	di
	;LN_LrgbstFromSttb takes psttb in bx and returns hrgbst in dx:di
	;and lrgbst in es:di.  Only dx, di, and es are altered.
	call	LN_LrgbstFromSttb
	xchg	ax,bx
	shl	bx,1
	add	di,es:[bx+di]
	xchg	ax,di
	mov	bl,[di.fStyleRulesSttb] ;needed by GetStFromSttbExit
	pop	di
ifdef DEBUG
	pop	ds	;restore segment handle
endif ;DEBUG
	cmp	cx,1
	ja	GetStFromSttbExit
	jcxz	HFS02
	;This is actually the last part of FStcpEntryIsNull
	xchg	ax,bx
	cmp	bptr es:[bx],255
	cmc
	sbb	ax,ax	    ;result is -1 if 255, 0 otherwise
HFS02:
;}
	db	0CBh	    ;far ret


;-------------------------------------------------------------------------
;	GetStFromSttb(hsttb, i, st)
;-------------------------------------------------------------------------
;/* G E T  S T	F R O M  S T T B */
;/* Fetch the contents of i in hsttb into st.  st must have sufficient
;   space (up to cchMaxSt). */
;NATIVE GetStFromSttb(hsttb, i, st)
;struct STTB **hsttb;
;int i;
;CHAR *st;
;{
;	 CHAR HUGE *hpst = HpstFromSttb(hsttb, i);

; %%Function:GetStFromSttb %%Owner:BRADV
PUBLIC	GetStFromSttb
GetStFromSttb:
	pop	ax
	pop	dx
	pop	cx	;st in cx
	push	dx
	push	ax
	;does not alter cx, lpst returned in es:ax
	jmp	short HpstFromSttbCore

;	 if ((*hsttb)->fStyleRules && *hpst == 0xFF)
;		 st[0] = 0xFF;	     /* avoid GP-FAULTs */
;	 else
;		 bltbh(hpst, (CHAR HUGE *)st, *hpst+1);
;}
GetStFromSttbExit:
	push	si
	push	di
	push	ds	;save current ds (may now be a handle)
	push	es
	pop	ds
	push	ss
	pop	es
	xchg	ax,si
	mov	di,cx
	mov	cl,[si]
	errnz	<maskfStyleRulesSttb - 080h>
	or	bl,bl
	jns	GetStFromSttbStyleRules
	cmp	cl,0FFh
	jne	GetStFromSttbStyleRules
	xor	cl,cl
GetStFromSttbStyleRules:
	xor	ch,ch
	inc	cx
	rep	movsb
	pop	ds	;restore ds (possible a handle)
	pop	di
	pop	si
	db	0CBh	    ;far ret


;-------------------------------------------------------------------------
;	IbstFindSt(hsttb, st)
;-------------------------------------------------------------------------
;/* I B S T  F I N D  S T */
;EXPORT IbstFindSt(hsttb, st)
;struct STTB **hsttb;
;char *st;
;{
;	 struct STTB *psttb = *hsttb;
;	 int ibst;
;	 int cch = *st;
;	 CHAR HUGE *hpst2;
; %%Function:IbstFindSt %%Owner:BRADV
PUBLIC	IbstFindSt
IbstFindSt:
	pop	ax
	pop	dx
	pop	cx
	pop	bx
	push	dx
	push	ax

;	 AssertH(hsttb);
;	 Assert(i < (*hsttb)->ibstMac);
ifdef DEBUG
	inc	bp
	push	bp
	mov	bp,sp	    ;set up bp chain for far call
	push	ds
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midResn2
	mov	cx,1005
	cCall	AssertHForNative,<bx, ax, cx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	ds
	pop	bp
	dec	bp  ;restore bp
	push	ds  ;ds may now be a handle
	push	ss
	pop	ds  ;restore valid ds
endif ;DEBUG
	mov	bx,[bx]
	mov	ax,[bx.ibstMacSttb]
	shl	ax,1
	push	si
	push	di
	;LN_LrgbstFromSttb takes psttb in bx and returns hrgbst in dx:di
	;and lrgbst in es:di.  Only dx, di, and es are altered.
	call	LN_LrgbstFromSttb
	mov	si,cx
	mov	cl,[si]
	xor	ch,ch
	inc	cx
	xchg	ax,dx
	mov	ax,-2
	mov	bx,ax

;	 for (ibst = 0; ibst < psttb->ibstMac; ibst++)
;		 {
;		 hpst2 = HpstFromSttb(hsttb, ibst);
;		 if (*hpst2 == cch && !FNeHprgch((CHAR HUGE *)st+1, hpst2+1,
;			     cch))
;		 return(ibst);
;		 }
;	 return(-1);
;}
IFS02:
	inc	bx
	inc	bx
	cmp	bx,dx
	jae	IFS03
	push	cx
	push	si
	push	di
	add	di,es:[bx+di]
	repe	cmpsb
	pop	di
	pop	si
	pop	cx
	jne	IFS02
	xchg	ax,bx
IFS03:
	sar	ax,1
	pop	di
	pop	si
ifdef DEBUG
	pop	ds	;restore segment handle
endif ;DEBUG
	db	0CBh	    ;far ret

maskFreeAfterFetchLocal     equ 001h
maskFreeBeforeFetchLocal    equ 002h

;-------------------------------------------------------------------------
;	PdrFetchAndFree(hpldr, idr, pdrf)
;-------------------------------------------------------------------------
;EXPORT struct DR *PdrFetchAndFree(hpldr, idr, pdrf)
;struct PLDR **hpldr;
;int idr;
;struct DRF *pdrf;
;{
;    struct DR *pdr;
;
;    pdr = PdrFetch(hpldr, idr, pdrf);
;    FreePdrf(pdrf);
;    return pdr;
;}
; %%Function:N_PdrFetchAndFree %%Owner:BRADV
PUBLIC N_PdrFetchAndFree
N_PdrFetchAndFree:
	mov	al,maskFreeAfterFetchLocal
	db	03Dh	;turns next "mov al,immediate" into "cmp ax,immediate"

;-------------------------------------------------------------------------
;	PdrFreeAndFetch(hpldr, idr, pdrf)
;-------------------------------------------------------------------------
;EXPORT struct DR *PdrFreeAndFetch(hpldr, idr, pdrf)
;struct PLDR **hpldr;
;int idr;
;struct DRF *pdrf;
;{
;
;    FreePdrf(pdrf);
;    return PdrFetch(hpldr, idr, pdrf);
;}
; %%Function:N_PdrFreeAndFetch %%Owner:BRADV
PUBLIC N_PdrFreeAndFetch
N_PdrFreeAndFetch:
	mov	al,maskFreeBeforeFetchLocal
	db	03Dh	;turns next "mov al,immediate" into "cmp ax,immediate"

;-------------------------------------------------------------------------
;	PdrFetch(hpldr, idr, pdrf)
;-------------------------------------------------------------------------
;EXPORT struct DR *PdrFetch(hpldr, idr, pdrf)
;struct PLDR **hpldr;
;int idr;
;struct DRF *pdrf;
;{
;    struct DRF *pdrfList;
; %%Function:N_PdrFetch %%Owner:BRADV
PUBLIC N_PdrFetch
N_PdrFetch:
	mov	al,0
cProc	PdrFetchEngine,<PUBLIC,FAR>,<si, di>
	ParmW	hpldr
	ParmW	idr
	ParmW	pdrf

	LocalW	wFlags

cBegin
	mov	bx,[pdrf]
	mov	bptr ([wFlags]),al
	test	al,maskFreeBeforeFetchLocal
	je	PFE01
	;LN_FreePdrf takes a pdrf in bx and performs FreePdrf(pdrf);
	;bx is not altered.
	call	LN_FreePdrf
PFE01:

ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx

;   Assert (hpldr != hNil);
	cmp	[hpldr],hNil
	jne	PFE02
	mov	ax,midResn2
	mov	bx,1007 		   ;label # for native assert
	cCall	AssertProcForNative, <ax, bx>
PFE02:

;   Assert (idr < (*hpldr)->idrMac);
	mov	bx,[hpldr]
	mov	bx,[bx]
	mov	ax,[bx.idrMacPldr]
	cmp	[idr],ax
	jb	PFE03
	mov	ax,midResn2
	mov	bx,1008 		   ;label # for native assert
	cCall	AssertProcForNative, <ax, bx>
PFE03:

;#ifdef DEBUG
;   for (pdrfList = vpdrfHeadUnused;
;	pdrfList != NULL; pdrfList = pdrfList->pdrfNext)
;	{
;	Assert (pdrfList->wLast == wLastDrf);
;	Assert (pdrfList != pdrf);
;	}
;#endif /* DEBUG */
	mov	bx,[vpdrfHeadUnused]
	jmp	short PFE06
PFE04:
	cmp	[bx.wLastDrf],0ABCDh
	je	PFE05
	push	bx	;save pdrfList
	mov	ax,midResn2
	mov	bx,1009 		   ;label # for native assert
	cCall	AssertProcForNative, <ax, bx>
	pop	bx	;restore pdrfList
PFE05:
	cmp	bx,[pdrf]
	jne	PFE055
	push	bx	;save pdrfList
	mov	ax,midResn2
	mov	bx,1010 		   ;label # for native assert
	cCall	AssertProcForNative, <ax, bx>
	pop	bx	;restore pdrfList
PFE055:
	mov	bx,[bx.pdrfNextDrf]
PFE06:
	cmp	bx,NULL
	jne	PFE04

;   Debug (pdrf->wLast = wLastDrf);
	mov	bx,[pdrf]
	mov	[bx.wLastDrf],0ABCDh
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

;   pdrf->hpldr = hpldr;
	mov	dx,[hpldr]
	mov	[bx.hpldrDrf],dx

;   pdrf->idr = idr;
	mov	cx,[idr]
	mov	[bx.idrDrf],cx

;   for (pdrfList = vpdrfHead; pdrfList != NULL; pdrfList = pdrfList->pdrfNext)
;	{
	mov	si,[vpdrfHead]
	jmp	short PFE09
PFE08:
	mov	si,[si.pdrfNextDrf]
PFE09:
	errnz	<NULL>
	or	si,si
	je	PFE10

;	Assert (pdrfList->wLast == wLastDrf);
;	Assert (pdrfList != pdrf);
	;Do these asserts with a call so as not to mess up short jumps
ifdef DEBUG
	call	PFE14
endif ;DEBUG

;	if (pdrfList->hpldr == hpldr && pdrfList->idr == idr)
;	    {
	cmp	[si.hpldrDrf],dx
	jne	PFE08
	cmp	[si.idrDrf],cx
	jne	PFE08

;	    pdrf->pdrfUsed = pdrfList;
	mov	[bx.pdrfUsedDrf],si

;#ifdef DEBUG
;	    pdrf->pdrfNext = vpdrfHeadUnused;
;	    vpdrfHeadUnused = pdrf;
;#endif /* DEBUG */
ifdef DEBUG
	push	[vpdrfHeadUnused]
	pop	[bx.pdrfNextDrf]
	mov	[vpdrfHeadUnused],bx
endif ;DEBUG

;	    return &pdrfList->dr;
	lea	ax,[si.drDrf]
	jmp	short PFE12

;	    }
;	}

PFE10:

;   bltbh(HpInPl(hpldr, idr), &pdrf->dr, sizeof(struct DR));
	;LN_LpInPl takes hplFoo in dx and iFoo in cx to
	;return lpFoo in es:di.  bx, cx and si are not altered.
	call	LN_LpInPl
	mov	si,di
	push	es
	pop	ds
	lea	di,[bx.drDrf]
	push	ss
	pop	es
	errnz	<cbDrMin AND 1>
	mov	cx,cbDrMin SHR 1
	rep	movsw
	push	ss
	pop	ds

;   pdrf->pdrfUsed = NULL;
	mov	[bx.pdrfUsedDrf],NULL

;   pdrf->pdrfNext = vpdrfHead;
;   vpdrfHead = pdrf;
	mov	cx,bx
	xchg	[vpdrfHead],cx
	mov	[bx.pdrfNextDrf],cx

;   if ((*hpldr)->fExternal && pdrf->dr.hplcedl)
;	{
	cmp	[bx.drDrf.hplcedlDr],0
	je	PFE11
	mov	si,[hpldr]
	mov	si,[si]
	cmp	[si.fExternalPldr],fFalse
	je	PFE11

;	pdrf->pplcedl = &(pdrf->dr.plcedl);
;	pdrf->dr.hplcedl = &(pdrf->pplcedl);
	lea	si,[bx.pplcedlDrf]
	mov	[bx.drDrf.hplcedlDr],si
	lea	cx,[bx.drDrf.plcedlDr]
	mov	[si],cx
;	}
PFE11:

;   return &pdrf->dr;
	lea	ax,[bx.drDrf]
PFE12:
	test	bptr ([wFlags]),maskFreeAfterFetchLocal
	je	PFE13
	;LN_FreePdrf takes a pdrf in bx and performs FreePdrf(pdrf);
	;bx is not altered.
	push	ax	;save pdr
	call	LN_FreePdrf
	pop	ax	;restore pdr
PFE13:

;}
cEnd

;	Assert (pdrfList->wLast == wLastDrf);
;	Assert (pdrfList != pdrf);
	;Do these asserts with a call so as not to mess up short jumps
ifdef DEBUG
PFE14:
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	[si.wLastDrf],0ABCDh
	je	PFE15
	mov	ax,midResn2
	mov	bx,1011 		   ;label # for native assert
	cCall	AssertProcForNative, <ax, bx>
PFE15:
	cmp	si,bx
	jne	PFE16
	mov	ax,midResn2
	mov	bx,1012 		   ;label # for native assert
	cCall	AssertProcForNative, <ax, bx>
PFE16:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG


;-------------------------------------------------------------------------
;	FreePdrf(pdrf)
;-------------------------------------------------------------------------
;EXPORT FreePdrf(pdrf)
;struct DRF *pdrf;
;{
;   struct DR *pdr;
; %%Function:N_FreePdrf %%Owner:BRADV
PUBLIC N_FreePdrf
N_FreePdrf:
	pop	ax
	pop	dx
	pop	bx
	push	dx
	push	ax
	push	si	;save caller's si
	push	di	;save caller's di
	;LN_FreePdrf takes a pdrf in bx and performs FreePdrf(pdrf);
	;bx is not altered.
	call	LN_FreePdrf
	pop	di	;restore caller's di
	pop	si	;restore caller's si
	db	0CBh	    ;far ret

	;LN_FreePdrf takes a pdrf in bx and performs FreePdrf(pdrf);
	;bx is not altered.
LN_FreePdrf:
;   Assert (pdrf->wLast == wLastDrf);
;   Debug (pdrf->wLast = 0);
;   Assert (pdrf->pdrfUsed != NULL || vpdrfHead == pdrf);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	[bx.wLastDrf],0ABCDh
	je	FP01
	mov	ax,midResn2
	mov	bx,1013 		   ;label # for native assert
	cCall	AssertProcForNative, <ax, bx>
FP01:
	mov	[bx.wLastDrf],0
	cmp	[bx.pdrfUsedDrf],NULL
	jne	FP02
	cmp	bx,[vpdrfHead]
	je	FP02
	mov	ax,midResn2
	mov	bx,1014 		   ;label # for native assert
	cCall	AssertProcForNative, <ax, bx>
FP02:

;   if (pdrf->pdrfUsed == NULL)
;	Assert(vpdrfHead == pdrf);
;#ifdef DEBUG
;   else
;	{
;	Assert(vpdrfHeadUnused == pdrf);
;	vpdrfHeadUnused = pdrf->pdrfNext;
;	}
;#endif /* DEBUG */
	cmp	[bx.pdrfUsedDrf],NULL
	jne	FP04
	cmp	bx,[vpdrfHead]
	je	FP03
	mov	ax,midResn2
	mov	bx,1015 		   ;label # for native assert
	cCall	AssertProcForNative, <ax, bx>
FP03:
	jmp	short FP06
FP04:
	cmp	bx,[vpdrfHeadUnused]
	je	FP05
	mov	ax,midResn2
	mov	bx,1016 		   ;label # for native assert
	cCall	AssertProcForNative, <ax, bx>
FP05:
	mov	ax,[bx.pdrfNextDrf]
	mov	[vpdrfHeadUnused],ax
FP06:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

;   pdr = (pdrf->pdrfUsed == NULL ? &pdrf->dr : &pdrf->pdrfUsed->dr);
;   bltbh(pdr, HpInPl(pdrf->hpldr, pdrf->idr), sizeof(struct DR));
	;LN_LpInPl takes hplFoo in dx and iFoo in cx to
	;return lpFoo in es:di.  bx, cx and si are not altered.
	mov	dx,[bx.hpldrDrf]
	mov	cx,[bx.idrDrf]
	call	LN_LpInPl
	mov	si,[bx.pdrfUsedDrf]
	errnz	<NULL>
	or	si,si
	jne	FP07
;   if (pdrf->pdrfUsed == NULL)
;	vpdrfHead = pdrf->pdrfNext;
	mov	si,[bx.pdrfNextDrf]
	mov	[vpdrfHead],si
	mov	si,bx
FP07:
	add	si,(drDrf)
	errnz	<cbDrMin AND 1>
	mov	cx,cbDrMin SHR 1
	rep	movsw

ifdef DEBUG
;   Debug (pdrf->dr.hplcedl = NULL);
;   Debug (pdrf->pplcedl = NULL);
	mov	[bx.drDrf.hplcedlDr],NULL
	mov	[bx.pplcedlDrf],NULL
endif ;DEBUG
;}
	ret


;-------------------------------------------------------------------------
;	PInPl(hpl, i)
;-------------------------------------------------------------------------
; %%Function:PInPl %%Owner:BRADV
PUBLIC PInPl
PInPl:
ifdef DEBUG
;	Assert(!ppl->fExternal);
	push	ax
	push	bx
	push	cx
	push	dx
	mov	bx,sp
	mov	bx,[bx+14]  ;/* hpl */
	mov	bx,[bx]
	cmp	[bx.fExternalPl],fFalse
	je	PIP01
	mov	ax,midResn2
	mov	bx,1031 		   ;label # for native assert
	cCall	AssertProcForNative, <ax, bx>
PIP01:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG
	;fall through

;-------------------------------------------------------------------------
;	HpInPl(hplFoo, iFoo)
;-------------------------------------------------------------------------
;int HUGE *HpInPl(hplFoo, iFoo)
;struct PL **hplFoo;
;int iFoo;
;{
;    struct PL *pplFoo = *hplFoo;
;    char      *rgFoo;
;
;    Assert (iFoo >= 0 && iFoo < pplFoo->iMac);
;    rgFoo = ((char *)pplFoo) + pplFoo->brgfoo;
;    if (pplFoo->fExternal)
;	 hpFoo = ((char HUGE *)HpOfHq(*((HQ *)rgFoo)));
;    else
;	 hpFoo = ((char HUGE *)rgFoo);
;    return hpFoo + iFoo * pplFoo->cb;
;}
; %%Function:HpInPl %%Owner:BRADV
PUBLIC HpInPl
HpInPl:
	pop	ax
	pop	bx
	pop	cx
	pop	dx
	push	bx
	push	ax
	push	di	;save caller's di
	call	LN_LpInPl
	xchg	ax,di
	pop	di	;restore caller's di
	db	0CBh	    ;far ret

	;LN_LpInPl takes hplFoo in dx and iFoo in cx to
	;return lpFoo in es:di.  bx, cx and si are not altered.
LN_LpInPl:
	mov	di,dx
	mov	di,[di]
ifdef DEBUG
	or	cx,cx
	jge	LIP01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midResn2
	mov	bx,1017 		   ;label # for native assert
	cCall	AssertProcForNative, <ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LIP01:
	cmp	cx,[di.iMacPl]
	jl	LIP02
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midResn2
	mov	bx,1018 		   ;label # for native assert
	cCall	AssertProcForNative, <ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LIP02:
endif ;DEBUG
	mov	dx,[di.fExternalPl]
	mov	ax,[di.cbPl]
	add	di,[di.brgfooPl]
	or	dx,dx
	;LN_LpFromPossibleHq takes pFoo in di, and if the zero flag is
	;set then converts pFoo into lpFoo in es:di.  If the zero flag
	;is reset then it assumes pFoo points to an hqFoo, and converts
	;the hqFoo to an lpFoo in es:di.  In both cases hpFoo is left
	;in dx:di.  Only dx, di and es are altered.
	call	LN_LpFromPossibleHq
	push	dx	;save sb
	mul	cx
	add	di,ax
	pop	dx	;restore sb
	ret

;-------------------------------------------------------------------------
;	CchSz(sz)
;-------------------------------------------------------------------------
;   	    Returns length of sz, including trailing 0
; %%Function:CchSz %%Owner:BRADV
PUBLIC	CchSz
CchSz:
	pop cx
	pop dx	    ;far return address in dx:cx
	pop bx
	push dx
	push cx
	;CchSzNoFrame performs CchSz(bx), and sets es to ds.
	;bx, dx, si, di are not altered.
CchSzNoFrame:
	push	ds
	pop	es
	push	di
	mov	di,bx
	mov	al,0
	mov	cx,0FFFFh
	repne 	scasb
	xchg	ax,di
	sub	ax,bx
	pop	di
	db	0CBh	    ;far ret


;-------------------------------------------------------------------------
;	CchCopySz(pch1, pch2)
;-------------------------------------------------------------------------
;/* ****
;*  Description: Copies string at pch1 to pch2i, including null terminator.
;*		 Returns number of chars moved, excluding null terminator.
;** **** */
;int CchCopySz(pch1, pch2)
;register PCH pch1;
;register PCH pch2;
;{
;    int     cch = 0;
;    while ((*pch2++ = *pch1++) != 0)
;	 cch++;
;    return cch;
;} /* end of  C c h C o p y S z  */
; %%Function:CchCopySz %%Owner:BRADV
PUBLIC CchCopySz
CchCopySz:
	pop	ax
	pop	cx
	pop	dx
	pop	bx
	push	cx
	push	ax
	;CchSzNoFrame performs CchSz(bx), and sets es to ds.
	;bx, dx, si, di are not altered.
	push	cs
	call	CchSzNoFrame
	mov	cx,ax
	push	si
	push	di
	mov	si,bx
	mov	di,dx
	rep	movsb
	dec	ax
	pop	di
	pop	si
	db	0CBh	    ;far ret


;-------------------------------------------------------------------------
;	PchInSt(st, ch)
;-------------------------------------------------------------------------
;CHAR *PchInSt(st, ch)
;CHAR *st;
;int ch;
;{
;    CHAR *pch, *pchEnd;
;
;    for (pch = &st[1], pchEnd = &st[1] + st[0]; pch < pchEnd; pch++)
;	 {
;	 if (*pch == ch)
;	     return(pch);
;	 }
;    return(NULL);   /* character never found */
;}
; %%Function:PchInSt %%Owner:BRADV
PUBLIC PchInSt
PchInSt:
	pop	dx
	pop	cx
	pop	ax
	pop	bx
	push	cx
	push	dx
	push	ds
	pop	es
	push	di
	mov	di,bx
	mov	cl,[di]
	xor	ch,ch
	inc	di
	repne	scasb
	xchg	ax,di
	pop	di
	jne	PIS01
	errnz	<NULL>
	dec	ax
	db	03Dh	;turns next "xor ax,ax" into "cmp ax,immediate"
PIS01:
	xor	ax,ax
	db	0CBh	    ;far ret


maskfLowerLocal     equ     1
maskfUpperLocal     equ     2
maskfDigitLocal     equ     4


;-------------------------------------------------------------------------
;	FAlphaNum(ch)
;-------------------------------------------------------------------------
;/* F  A L P H A  N U M */
;FAlphaNum(ch)
;CHAR ch;
;{
;    return FAlpha(ch) || FDigit(ch);
;}
; %%Function:FAlphaNum %%Owner:BRADV
PUBLIC FAlphaNum
FAlphaNum:
	mov	bl,maskfLowerLocal+maskfUpperLocal+maskfDigitLocal
	db	03Dh	;turns next "mov bl,immediate" into "cmp ax,immediate"


;-------------------------------------------------------------------------
;	FDigit(ch)
;-------------------------------------------------------------------------
;/* ****
;*  Description: Returns TRUE if ch is a digit, FALSE otherwise.
;** **** */
;FDigit(ch)
;CHAR ch;
;{
;	 return((uns)(ch - '0') <= ('9' - '0'));
;}
; %%Function:FDigit %%Owner:BRADV
PUBLIC FDigit
FDigit:
	mov	bl,maskfDigitLocal
	db	03Dh	;turns next "mov bl,immediate" into "cmp ax,immediate"


;-------------------------------------------------------------------------
;	FAlpha(ch)
;-------------------------------------------------------------------------
;/* F  A L P H A */
;/* ****
;*  Description: Returns TRUE if ch is a letter, FALSE otherwise.
;*	Note: DF and FF are treated as lowercase, even though they have no
;*	corresponding uppercase.
;** **** */
;FAlpha(ch)
;CHAR ch;
;{
;    return(FLower(ch) || FUpper(ch));
;}
; %%Function:FAlpha %%Owner:BRADV
PUBLIC FAlpha
FAlpha:
	mov	bl,maskfLowerLocal+maskfUpperLocal
	db	03Dh	;turns next "mov bl,immediate" into "cmp ax,immediate"


;-------------------------------------------------------------------------
;	FUpper(ch)
;-------------------------------------------------------------------------
;/* ****
;*  Description: Returns TRUE if ch is an uppercase letter, FALSE otherwise.
;** **** */
;
;NATIVE FUpper(ch)
;CHAR ch;
;{
;    return  (((uns)(ch - 'A') <= ('Z' - 'A')) ||
;	     /* foreign */
;	     ((uns)(ch - 0x00C0) <= (0x00D6 - 0x00C0)) ||
;	     ((uns)(ch - 0x00D8) <= (0x00DE - 0x00D8)));
;}
; %%Function:FUpper %%Owner:BRADV
PUBLIC FUpper
FUpper:
	mov	bl,maskfUpperLocal
	db	03Dh	;turns next "mov bl,immediate" into "cmp ax,immediate"


;-------------------------------------------------------------------------
;	FLower(ch)
;-------------------------------------------------------------------------
;/* F  L O W E R */
;/* ****
;*  Description: Returns TRUE if ch is a lowercase letter, FALSE otherwise.
;*	Note: even though DF and FF are lowercase, they have no
;*	corresponding uppercase. They are included in the lowercase
;*	set so they will appear as characters.
;** **** */
;    /*  this is not using ChUpper/ChLower because that would be very slow -
;	 you would generally need 2 calls to determine the case. Since the
;	 ANSI set is fairly immutable, we are doing it this way. (bz)
;    */
;NATIVE FLower(ch)
;CHAR ch;
;{
;    return (((uns)(ch - 'a') <= ('z' - 'a')) ||
;	     /* foreign */
;	     ((uns)(ch - 0x00DF) <= (0x00F6 - 0x00DF)) ||
;	     ((uns)(ch - 0x00F8) <= (0x00FF - 0x00F8)));
;}
; %%Function:FLower %%Owner:BRADV
PUBLIC FLower
FLower:
	mov	bl,maskfLowerLocal
FClassEngine:
	pop	cx
	pop	dx
	pop	ax
	push	dx
	push	cx
	test	bl,maskfLowerLocal
	je	FC01

;    return (((uns)(ch - 'a') <= ('z' - 'a')) ||
	sub al,'a'
	cmp al,'z'-'a'+1
	jc  FC03
;	     /* foreign */
;	     ((uns)(ch - 0x00DF) <= (0x00F6 - 0x00DF)) ||
	sub al,0DFh-'a'
	cmp al,0F6h-0DFh+1
	jc  FC03
;	     ((uns)(ch - 0x00F8) <= (0x00FF - 0x00F8)));
	sub al,0F8h-0DFh
	cmp al,0FFh-0F8h+1
	jc  FC03
	sub al,000h-0F8h
FC01:
	test	bl,maskfUpperLocal
	je	FC02

;    return  (((uns)(ch - 'A') <= ('Z' - 'A')) ||
	sub al,'A'
	cmp al,'Z'-'A'+1
	jc  FC03
;	     /* foreign */
;	     ((uns)(ch - 0x00C0) <= (0x00D6 - 0x00C0)) ||
	sub al,0C0h-'A'
	cmp al,0D6h-0C0h+1
	jc  FC03
;	     ((uns)(ch - 0x00D8) <= (0x00DE - 0x00D8)));
;	     ((uns)(ch - 0x00F8) <= (0x00FF - 0x00F8)));
	sub al,0D8h-0C0h
	cmp al,0DEh-0D8h+1
	jc  FC03
	sub al,000h-0D8h
FC02:

	test	bl,maskfDigitLocal
	je	FC03

;	 return((uns)(ch - '0') <= ('9' - '0'));
	sub al,'0'
	cmp al,'9'-'0'+1
FC03:
	sbb ax,ax
	db	0CBh	    ;far ret


;-------------------------------------------------------------------------
;	PrcSet( prc, xpLeft, ypTop, xpRight, ypBottom )
;-------------------------------------------------------------------------
;struct RC *PrcSet( prc, xpLeft, ypTop, xpRight, ypBottom )
;struct RC *prc;
;{
; prc->xpLeft = xpLeft;
; prc->xpRight = xpRight;
; prc->ypTop = ypTop;
; prc->ypBottom = ypBottom;
;
; return prc;
;}
; %%Function:PrcSet %%Owner:BRADV
PUBLIC	PrcSet
PrcSet:
;LN_StackAdjust is a helper routine to reduce the size of routines
;in resn.asm.  It takes the number of words of arguments to
;the routine minus one in the byte following the call to LN_StackAdjust.
;It then moves the far return address and si and di to above the
;arguments, moves the top argument into bx, and puts the other arguments
;below the saved si and di.  This allows the use of short pop register
;instructions to access arguments passed to the caller of LN_StackAdjust.
;ax, bx, cx, dx are altered.
	call	LN_StackAdjust
	db	4
	pop	[bx.ypBottomRc]
	pop	[bx.xpRightRc]
	pop	[bx.ypTopRc]
	pop	[bx.xpLeftRc]
	xchg	ax,bx
	pop	di
	pop	si
	db	0CBh	    ;far ret


;-------------------------------------------------------------------------
;	FNeNcRgch(pch1, pch2, cch)
;-------------------------------------------------------------------------
;/* F  N E  N C  R G C H  */
;/*  case insensitive rgch compare */
;FNeNcRgch(pch1, pch2, cch)
;CHAR *pch1, *pch2;
;int cch;
;{
;    while(cch--)
;	 if (ChUpperLookup(*pch1++) != ChUpperLookup(*pch2++))
;	     return fTrue;
;    return fFalse;
;}
; %%Function:FNeNcRgch %%Owner:BRADV
PUBLIC FNeNcRgch
FNeNcRgch:
;LN_StackAdjust is a helper routine to reduce the size of routines
;in resn.asm.  It takes the number of words of arguments to
;the routine minus one in the byte following the call to LN_StackAdjust.
;It then moves the far return address and si and di to above the
;arguments, moves the top argument into bx, and puts the other arguments
;below the saved si and di.  This allows the use of short pop register
;instructions to access arguments passed to the caller of LN_StackAdjust.
;ax, bx, cx, dx are altered.
	call	LN_StackAdjust
	db	2
	mov	si,bx
	pop	cx
	pop	di
	push	wptr ([vitr.fFrenchItr])
	mov	[vitr.fFrenchItr],fTrue     ;Simulate ChUpperLookup
	jmp	short FNNE03

;-------------------------------------------------------------------------
;	FNeNcSz(sz1, sz2)
;-------------------------------------------------------------------------
;/* F  N E  N C  S Z */
;/*  case insensitive string compare */
;EXPORT FNeNcSz(sz1, sz2)
;CHAR *sz1, *sz2;
;{
;    int cch = CchSz(sz1)-1;
;    if (cch != CchSz(sz2)-1)
;	 return fTrue;
;    return FNeNcRgch(sz1, sz2, cch);
;}
; %%Function:FNeNcSz %%Owner:BRADV
PUBLIC FNeNcSz
FNeNcSz:
	db	0A8h	;turns next "stc" into "test al,immediate"
			;also clears the carry flag

;-------------------------------------------------------------------------
;	FNeNcSt(st1, st2)
;-------------------------------------------------------------------------
;/* F  N E  N C  S T */
;/*  case insensitive string compare */
;FNeNcSt(st1, st2)
;CHAR *st1, *st2;
;{
;    int cch = *st1++;
;    if (cch != *st2++)
;	 return fTrue;
;    return FNeNcRgch(st1, st2, cch);
;}
; %%Function:FNeNcSt %%Owner:BRADV
PUBLIC FNeNcSt
FNeNcSt:
	stc
FNeNcEngine:
	pop	ax
	pop	dx
	pop	cx
	pop	bx
	push	dx
	push	ax
	push	si
	push	di
	push	wptr ([vitr.fFrenchItr])
	mov	[vitr.fFrenchItr],fTrue     ;Simulate ChUpperLookup
	mov	si,bx
	mov	di,cx
	jc	FNNE01
	;CchSzNoFrame performs CchSz(bx), and sets es to ds.
	;bx, dx, si, di are not altered.
	push	cs
	call	CchSzNoFrame
	dec	ax
	push	ax
	mov	bx,di
	;CchSzNoFrame performs CchSz(bx), and sets es to ds.
	;bx, dx, si, di are not altered.
	push	cs
	call	CchSzNoFrame
	dec	ax
	pop	cx
	cmp	ax,cx
	jmp	short FNNE02
FNNE01:
	lodsb
	mov	cl,[di]
	inc	di
	xor	ch,ch
	cmp	al,cl
FNNE02:
	jne	FNNE05
FNNE03:
	jcxz	FNNE06
FNNE04:
	;LN_ChUpper performs ChUpper(al).  Only al and bx are altered.
	lodsb
	call	LN_ChUpper
	mov	ah,al
	mov	al,[di]
	inc	di
	;LN_ChUpper performs ChUpper(al).  Only al and bx are altered.
	call	LN_ChUpper
	cmp	al,ah
	loope	FNNE04
	je	FNNE06
FNNE05:
	db	0B8h	;turns next "xor ax,ax" into "mov ax,immediate"
FNNE06:
	errnz	<fFalse>
	xor	ax,ax
	pop	wptr ([vitr.fFrenchItr])
	pop	di
	pop	si
	db	0CBh	    ;far ret


;-------------------------------------------------------------------------
;	QszUpper(pch)
;-------------------------------------------------------------------------
;/* Q S Z  U P P E R  */
;/*  upper case conversion using international rules (received from jurgenl 10-10-88 bz) */
;EXPORT QszUpper(pch)
;CHAR *pch;
;{
;  for (; *pch; pch++)
;	 *pch = ChUpper(*pch);
;}
; %%Function:QszUpper %%Owner:BRADV
PUBLIC QszUpper
QszUpper:
	pop	cx
	pop	dx
	pop	bx
	push	dx
	push	cx
	jmp	short QU02
QU01:
	push	bx
	;LN_ChUpper performs ChUpper(al).  Only al and bx are altered.
	call	LN_ChUpper
	pop	bx
	mov	[bx],al
	inc	bx
QU02:
	mov	al,[bx]
	or	al,al
	jne	QU01
	db	0CBh	    ;far ret


chFirstUpperTbl     = 224
;#define chFirstUpperTbl  (224)
;/* French upper case mapping table - only chars >= E0 (224) are mapped */
;csconst CHAR	 mpchupFrench[] = {
;/*   E0       E1	E2	 E3	  E4	   E5	    E6	     E7   */
;   ch(65),  ch(65),  ch(65),  ch(195), ch(196), ch(197), ch(198), ch(199),
;/*   E8       E9	EA	 EB	  EC	   ED	    EE	     EF   */
;   ch(69),  ch(69),  ch(69),  ch(69),	ch(73),  ch(73),  ch(73),  ch(73),
;/*   F0       F1	F2	 F3	  F4	   F5	    F6	     F7   */
;   ch(208), ch(209), ch(79),  ch(79),	ch(79),  ch(213), ch(214), ch(247),
;/*   F8       F9	FA	 FB	  FC	   FD	    FE	     FF   */
;   ch(216), ch(85),  ch(85),  ch(85),	ch(220), ch(221), ch(222), ch(255)
;   };
mpchupFrench:
	db	65,65,65,195,196,197,198,199
	db	69,69,69,69,73,73,73,73
	db	208,209,79,79,79,213,214,247
	db	216,85,85,85,220,221,222,255

;-------------------------------------------------------------------------
;	ChUpper(ch)
;-------------------------------------------------------------------------
; %%Function:ChUpper %%Owner:BRADV
PUBLIC ChUpper
ChUpper:
	pop	cx
	pop	dx
	pop	ax
	push	dx
	push	cx
	;LN_ChUpper performs ChUpper(al).  Only al and bx are altered.
	call	LN_ChUpper
	db	0CBh	    ;far ret


;/* C H U P P E R  */
;/*  upper case conversion using international rules (received from jurgenl 10-10-88 bz) */
;EXPORT ChUpper(ch)
;int ch;
;{
;   if ((uns)(ch - 'a') <= ('z' - 'a'))
;	return (ch - ('a' - 'A'));
	;LN_ChUpper performs ChUpper(al).  Only al and bx are altered.
LN_ChUpper:
ifdef DEBUG
	mov	bx,[wFillBlock]
endif ;DEBUG
	cmp	al,'a'
	jb	CU01
	cmp	al,'z'
	ja	CU01
	sub	al,'a' - 'A'
CU01:
;   else if (ch >= chFirstUpperTbl)  /* intl special chars */
;	{
	cmp	al,chFirstUpperTbl
	jae	CU02
	ret
CU02:
;	if (!vitr.fFrench)
;	    return ((ch == 247 || ch == 255) ? ch : ch - 32);
	cmp	[vitr.fFrenchItr],fFalse
	jne	CU04
	cmp	al,247
	je	CU03
	cmp	al,255
	je	CU03
	sub	al,32
CU03:
	ret

;	else
;	    return (mpchupFrench[ch - chFirstUpperTbl]);
CU04:
	mov	bx,offset [mpchupFrench - chFirstUpperTbl]
	xlat	cs:[bx]
	ret

;	}
;   else
;	return (ch);
;}


;-------------------------------------------------------------------------
;	StandardChp(pchp)
;-------------------------------------------------------------------------
;/* S T A N D A R D  C H P */
;/* creates most basic chp */
;NATIVE StandardChp(pchp)
;struct CHP *pchp;
;{
;	 SetWords(pchp, 0, cwCHP);
;	 pchp->hps = hpsDefault;
;}


;-------------------------------------------------------------------------
;	StandardPap(ppap)
;-------------------------------------------------------------------------
;/* S T A N D A R D  P A P */
;/* creates most basic pap */
;NATIVE StandardPap(ppap)
;struct PAP *ppap;
;{
;	 SetWords(ppap, 0, cwPAPBase);
;}


;-------------------------------------------------------------------------
;	StandardSep(psep)
;-------------------------------------------------------------------------
;/* S T A N D A R D  S E P */
;/* creates most basic sep */
;NATIVE StandardSep(psep)
;struct SEP *psep;
;{
;#define dxaCm 567
;	 SetWords(psep, 0, cwSEP);
;	 Mac( psep->dyaPgn = psep->dxaPgn = 36 * dyaPoint );
;	 if (vitr.fMetric)
;	    psep->dxaColumns = psep->dyaHdrTop = psep->dyaHdrBottom = NMultDiv(dxaCm, 5, 4);
;	 else
;	    psep->dxaColumns = psep->dyaHdrTop = psep->dyaHdrBottom = dxaInch / 2;
;	 /* psep->dxaColumnWidth = undefined */
;	 psep->bkc = bkcNewPage;
;	 psep->fEndnote = fTrue;
;}

dxaCm = 567
; %%Function:StandardChp %%Owner:BRADV
PUBLIC StandardChp
StandardChp:
	errnz	<cbChpMin AND 1>
	mov	cx,cbChpMin SHR 1
	jmp	short StandardStruct

; %%Function:StandardPap %%Owner:BRADV
PUBLIC StandardPap
StandardPap:
	errnz	<cbPapBase AND 1>
	mov	cx,cbPapBase SHR 1
	jmp	short StandardStruct

; %%Function:StandardSep %%Owner:BRADV
PUBLIC StandardSep
StandardSep:
	errnz	<cbSepMin AND 1>
	mov	cx,cbSepMin SHR 1
StandardStruct:
	pop	dx
	pop	bx
	pop	ax	;pstruct in ax
	push	bx
	push	dx
	mov	dx,cx	;save cw of struct
	push	di	;save caller's di
	xchg	ax,di
	xor	ax,ax
	push	ds
	pop	es
	rep	stosw
ife cbPapBase-cbChpMin
.err
endif
ife cbPapBase-cbSepMin
.err
endif
	cmp	dx,cbPapBase SHR 1
	je	SS02
ife cbChpMin-cbSepMin
.err
endif
	cmp	dx,cbChpMin SHR 1
	jne	SS01

;	 pchp->hps = hpsDefault;
	mov	[di.hpsChp - cbChpMin],hpsDefault
	jmp	short SS02
SS01:
;	if (vitr.fMetric)
;	   psep->dxaColumns = psep->dyaHdrTop = psep->dyaHdrBottom = NMultDiv(5, dxaCm, 4);
;	else
;	   psep->dxaColumns = psep->dyaHdrTop = psep->dyaHdrBottom = dxaInch / 2;
	mov	ax, dxaInch / 2
	cmp	[vitr.fMetricItr], fFalse
	jz	SS011
	mov	ax, 709 ; /* == NMultDiv(5, dxaCm, 4) */
SS011:
	mov	[di.dyaHdrTopSep - cbSepMin], ax
	mov	[di.dyaHdrBottomSep - cbSepMin], ax
	mov	[di.dxaColumnsSep - cbSepMin], ax

;	 psep->bkc = bkcNewPage;
	mov	[di.bkcSep - cbSepMin],bkcNewPage

;	 psep->fEndnote = fTrue;
	mov	[di.fEndnoteSep - cbSepMin],fTrue

SS02:
	pop	di	;restore caller's di
	db	0CBh	    ;far ret


;-------------------------------------------------------------------------
;	PmwdWw(ww)
;-------------------------------------------------------------------------
;/* P M W D  W W */
;NATIVE struct MWD *PmwdWw(ww)
;{
;#ifdef DEBUG
;	 int mw;
;	 struct WWD **hwwd;
;	 struct MWD **hmwd;
;	 Assert(ww > wwNil && ww < wwMax && (hwwd = mpwwhwwd[ww]));
;	 mw = (*hwwd)->mw;
;	 Assert(mw > mwNil && mw < mwMax && (hmwd = mpmwhmwd[mw]));
;	 return *hmwd;
;#endif
;	 return *mpmwhmwd[(*mpwwhwwd[ww])->mw];
;}
; %%Function:N_PmwdWw %%Owner:BRADV
PUBLIC N_PmwdWw
N_PmwdWw:
	mov	cl,1
	db	03Dh	;turns next "xor cx,cx" into "cmp ax,immediate"

;-------------------------------------------------------------------------
;	HwwdWw(ww)
;-------------------------------------------------------------------------
;/* H W W D  W W */
;NATIVE struct WWD **HwwdWw(ww)
;{
;	 Assert(ww > wwNil && ww < wwMax && mpwwhwwd[ww]);
;	 return mpwwhwwd[ww];
;}
; %%Function:N_HwwdWw %%Owner:BRADV
PUBLIC N_HwwdWw
N_HwwdWw:
	xor	cx,cx
	pop	ax
	pop	dx
	pop	bx
	push	dx
	push	ax
ifdef DEBUG
	cmp	bx,wwNil
	jle	PW01
	cmp	bx,wwMax
	jge	PW01
endif ;DEBUG
	shl	bx,1
	mov	ax,[bx.mpwwhwwd]
ifdef DEBUG
	or	ax,ax
	jne	PW02
PW01:
	inc	bp
	push	bp
	mov	bp,sp
	push	ds
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midResn2
	mov	bx,1019
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	ds
	pop	bp
	dec	bp
PW02:
endif ;DEBUG
	jcxz	PW05
	xchg	ax,bx
	mov	bx,[bx]
	mov	bx,[bx.mwWwd]
ifdef DEBUG
	cmp	bx,mwNil
	jle	PW03
	cmp	bx,mwMax
	jge	PW03
endif ;DEBUG
	shl	bx,1
	mov	bx,[bx.mpmwhmwd]
ifdef DEBUG
	or	bx,bx
	jne	PW04
PW03:
	inc	bp
	push	bp
	mov	bp,sp
	push	ds
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midResn2
	mov	bx,1020
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	ds
	pop	bp
	dec	bp
PW04:
endif ;DEBUG
	mov	ax,[bx]
PW05:
	db	0CBh	    ;far ret


;-------------------------------------------------------------------------
;	DrcToRc(pdrc, prc)
;-------------------------------------------------------------------------
;/* D R C  T O	R C  - convert from drc-style rect into a rc-style rect */
;NATIVE DrcToRc(pdrc, prc)
;struct DRC *pdrc;
;struct RC *prc;
;{
;	 prc->ptTopLeft = pdrc->ptTopLeft;
;	 prc->ypBottom = prc->ypTop + pdrc->dyp;
;	 prc->xpRight = prc->xpLeft + pdrc->dxp;
;}
; %%Function:DrcToRc %%Owner:BRADV
PUBLIC DrcToRc
DrcToRc:
	db	0A8h	;turns next "stc" into "test al,immediate"
			;also clears the carry flag

;-------------------------------------------------------------------------
;	RcToDrc(prc, pdrc)
;-------------------------------------------------------------------------
;/* R C  T O  D R C  - convert from rc-style rect into a drc-style rect */
;NATIVE RcToDrc(prc, pdrc)
;struct RC *prc;
;struct DRC *pdrc;
;{
;	 pdrc->ptTopLeft = prc->ptTopLeft;
;	 pdrc->dxp = prc->xpRight - prc->xpLeft;
;	 pdrc->dyp = prc->ypBottom - prc->ypTop;
;}
PUBLIC RcToDrc
RcToDrc:
	stc
	pop	dx
	pop	cx	;return address in dx:cx
	pop	bx
	pop	ax
	push	cx
	push	dx
	push	ds
	pop	es
	push	si
	push	di
	xchg	ax,si
	mov	di,bx
	errnz	<(xpDrc) - 0> ;if DrcToRc
	errnz	<(xpLeftRc) - 0> ;if RcToDrc
	lodsw
	errnz	<(xpLeftRc) - 0> ;if DrcToRc
	errnz	<(xpDrc) - 0> ;if RcToDrc
	stosw
	xchg	ax,cx
	errnz	<(ypDrc) - 2> ;if DrcToRc
	errnz	<(ypTopRc) - 2> ;if RcToDrc
	lodsw
	errnz	<(ypTopRc) - 2> ;if DrcToRc
	errnz	<(ypDrc) - 2> ;if RcToDrc
	stosw
	xchg	ax,dx
	jnc	DTRRTD01
	neg	cx	;negate xpLeftRc
	neg	dx	;negate ypTopRc
DTRRTD01:
	errnz	<(dxpDrc) - 4> ;if DrcToRc
	errnz	<(xpRightRc) - 4> ;if RcToDrc
	lodsw
	add	ax,cx
	errnz	<(xpRightRc) - 4> ;if DrcToRc
	errnz	<(dxpDrc) - 4> ;if RcToDrc
	stosw
	errnz	<(dypDrc) - 6> ;if DrcToRc
	errnz	<(ypBottomRc) - 6> ;if RcToDrc
	lodsw
	add	ax,dx
	errnz	<(ypBottomRc) - 6> ;if DrcToRc
	errnz	<(dypDrc) - 6> ;if RcToDrc
	stosw
	pop	di
	pop	si
	db	0CBh	    ;far ret

bitIdrIsNeg1	    equ 001h
bitCallPtOrigin     equ 002h
bitUseVHwwdOrigin   equ 004h
bitNegate	    equ 008h
bitFromDrc	    equ 010h
bitReturnY	    equ 020h

;/* R C L  T O	R C W  - converts rect in frame coord. to rect in window coord */
;NATIVE RclToRcw(hpldr, prcl, prcw)
;struct PLDR **hpldr;
;struct RC *prcl;
;struct RC *prcw;
;{
;	 struct WWD *pwwd;
;	 int dxlToXw;
;	 int dylToYw;
;	 struct PT pt;
;
;	 pt = PtOrigin(hpldr, -1);
;
;	 pwwd = *vhwwdOrigin;
;	 dxlToXw = pwwd->rcePage.xeLeft + pwwd->xwMin + pt.xp;
;	 dylToYw = pwwd->rcePage.yeTop + pwwd->ywMin + pt.yp;
;
;	 prcw->xwLeft = prcl->xlLeft + dxlToXw;
;	 prcw->xwRight = prcl->xlRight + dxlToXw;
;	 prcw->ywTop = prcl->ylTop + dylToYw;
;	 prcw->ywBottom = prcl->ylBottom + dylToYw;
;}
; %%Function:RclToRcw %%Owner:BRADV
PUBLIC RclToRcw
RclToRcw:
	mov	al,bitIdrIsNeg1 + bitCallPtOrigin + bitUseVHwwdOrigin
	db	03Dh	;turns next "mov al,immediate" into "cmp ax,immediate"

;/* R C W  T O	R C L  - converts rect in window coord. to rect in page coord */
;NATIVE RcwToRcl(hpldr, prcw, prcl)
;struct PLDR **hpldr;
;struct RC *prcw;
;struct RC *prcl;
;{
;	 struct WWD *pwwd;
;	 struct PT pt;
;	 int dxwToXl, dywToYl;
;
;	 pt = PtOrigin(hpldr,-1);
;	 pwwd = *vhwwdOrigin;
;	 dxwToXl = pwwd->rcePage.xeLeft + pwwd->xwMin + pt.xp;
;	 dywToYl = pwwd->rcePage.yeTop + pwwd->ywMin + pt.yp;
;
;	 prcl->xlLeft = prcw->xwLeft - dxwToXl;
;	 prcl->xlRight = prcw->xwRight - dxwToXl;
;	 prcl->ylTop = prcw->ywTop - dywToYl;
;	 prcl->ylBottom = prcw->ywBottom - dywToYl;
;}
; %%Function:RcwToRcl %%Owner:BRADV
PUBLIC RcwToRcl
RcwToRcl:
	mov	al,bitIdrIsNeg1 + bitCallPtOrigin + bitUseVHwwdOrigin + bitNegate
	db	03Dh	;turns next "mov al,immediate" into "cmp ax,immediate"

;/* R C W  T O	R C P */
;/* Converts rc in dr coords into rc in window coords */
;NATIVE RcwToRcp(hpldr/* or hwwd*/, idr, prcw, prcp)
;struct PLDR **hpldr;
;struct RC *prcp, *prcw;
;{
;	 struct PT pt;
;	 struct WWD *pwwd;
;	 int dxPToW, dyPToW;
;
;	 pt = PtOrigin(hpldr, idr);
;
;	 pwwd = *vhwwdOrigin;
;	 dxPToW = pwwd->rcePage.xeLeft + pwwd->xwMin + pt.xl;
;	 dyPToW = pwwd->rcePage.yeTop + pwwd->ywMin + pt.yl;
;
;	 prcp->xwLeft = prcw->xpLeft - dxPToW;
;	 prcp->xwRight = prcw->xwRight - dxPToW;
;	 prcp->ywTop = prcw->ywTop - dyPToW;
;	 prcp->ywBottom = prcw->ywBottom - dyPToW;
;}
; %%Function:RcwToRcp %%Owner:BRADV
PUBLIC RcwToRcp
RcwToRcp:
	mov	al,bitCallPtOrigin + bitUseVHwwdOrigin + bitNegate
	db	03Dh	;turns next "mov al,immediate" into "cmp ax,immediate"

;/* R C P  T O	R C W */
;/* Converts rc in dr coords into rc in window coords */
;NATIVE RcpToRcw(hpldr/* or hwwd*/, idr, prcp, prcw)
;struct PLDR **hpldr;
;struct RC *prcp, *prcw;
;{
;	 struct PT pt;
;	 struct WWD *pwwd;
;	 int dxPToW, dyPToW;
;
;	 pt = PtOrigin(hpldr, idr);
;
;	 pwwd = *vhwwdOrigin;
;	 dxPToW = pwwd->rcePage.xeLeft + pwwd->xwMin + pt.xl;
;	 dyPToW = pwwd->rcePage.yeTop + pwwd->ywMin + pt.yl;
;
;	 prcw->xwLeft = prcp->xpLeft + dxPToW;
;	 prcw->xwRight = prcp->xwRight + dxPToW;
;	 prcw->ywTop = prcp->ywTop + dyPToW;
;	 prcw->ywBottom = prcp->ywBottom + dyPToW;
;}
; %%Function:RcpToRcw %%Owner:BRADV
PUBLIC RcpToRcw
RcpToRcw:
	mov	al,bitCallPtOrigin + bitUseVHwwdOrigin
	db	03Dh	;turns next "mov al,immediate" into "cmp ax,immediate"

;/* R C E  T O	R C W  */
;/* converts a page rectangle into window coordinates. */
;NATIVE RceToRcw(pwwd, prce, prcw)
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
; %%Function:RceToRcw %%Owner:BRADV
PUBLIC RceToRcw
RceToRcw:
	mov	al,bitIdrIsNeg1
	db	03Dh	;turns next "mov al,immediate" into "cmp ax,immediate"

;/* D R C L  T O  R C W
;*  Converts drc rect in page coord to rc rect in window coordinates */
;/* Useful for getting a rcw from a dr.drcl */
;NATIVE DrclToRcw(hpldr/*or hwwd*/, pdrcl, prcw)
;struct PLDR **hpldr;
;struct DRC *pdrcl;
;struct RC *prcw;
;{
;	 struct RC rcl;
;	 DrcToRc(pdrcl, &rcl);
;	 RclToRcw(hpldr, &rcl, prcw);
;}
; %%Function:DrclToRcw %%Owner:BRADV
PUBLIC DrclToRcw
DrclToRcw:
	mov	al,bitIdrIsNeg1 + bitCallPtOrigin + bitUseVHwwdOrigin + bitFromDrc
	db	03Dh	;turns next "mov al,immediate" into "cmp ax,immediate"

;/* D R C P  T O  R C L
;*  Converts drc rect in DR coordinates to rc rect in page coordinates
;/* Useful for getting a rcl from a edl.drcp
;/* WARNING - this translates to the outermost l coordinate system,
;/* not necessarily the immediately enclosing l system. 'Tis
;/* better to use the w coordinate system in most cases, since
;/* it is unambiguous.
;/**/
;NATIVE DrcpToRcl(hpldr, idr, pdrcp, prcl)
;struct PLDR **hpldr;
;int idr;
;struct DRC *pdrcp;
;struct RC *prcl;
;{
;	 struct PT pt;
;	 pt = PtOrigin(hpldr, idr);
;
;	 prcl->xlLeft = pdrcp->xp + pt.xl;
;	 prcl->xlRight = prcl->xlLeft + pdrcp->dxp;
;	 prcl->ylTop = pdrcp->yp + pt.yl;
;	 prcl->ylBottom = prcl->ylTop + pdrcp->dyp;
;}
; %%Function:DrcpToRcl %%Owner:BRADV
PUBLIC DrcpToRcl
DrcpToRcl:
	mov	al,bitCallPtOrigin + bitFromDrc
	db	03Dh	;turns next "mov al,immediate" into "cmp ax,immediate"

;/* D R C P  T O  R C W
;   Converts drc rect in DR coordinates to rc rect in window coordinates */
;/* Useful for getting a rcw from a edl.drcp */
;NATIVE DrcpToRcw(hpldr/*or hwwd*/, idr, pdrcp, prcw)
;struct PLDR **hpldr;
;int idr;
;struct DRC *pdrcp;
;struct RC *prcw;
;{
;	 struct RC rcl;
;	 int dxlToXw;
;	 int dylToYw;
;	 struct WWD *pwwd;
;
;	 DrcpToRcl(hpldr, idr, pdrcp, &rcl);
;/* native code note: vhwwdOrigin is known to have a 0 backpointer! */
;	 RclToRcw(vhwwdOrigin, &rcl, prcw);
;}
; %%Function:DrcpToRcw %%Owner:BRADV
PUBLIC DrcpToRcw
DrcpToRcw:
	mov	al,bitCallPtOrigin + bitUseVHwwdOrigin + bitFromDrc
;	db	03Dh	;turns next "mov al,immediate" into "cmp ax,immediate"
	test	al,bitIdrIsNeg1
	je	RCTC01
	push	ax	;adjust sp
	push	ds
	pop	es
	push	si
	push	di
	mov	di,sp
	add	di,4
	lea	si,[di+2]
	movsw
	movsw
	movsw
	movsw			;make room for the idr
	mov	wptr [di],-1	;set the idr to -1
	pop	di
	pop	si
RCTC01:

; %%Function:RcToRcCommon %%Owner:BRADV
cProc	RcToRcCommon,<PUBLIC,FAR>,<si,di>
	ParmW	hpldr
	ParmW	idr
	ParmW	prcSrc
	ParmW	prcDest

cBegin

;	 dxeToXw = pwwd->xwMin;
;	 dyeToYw = pwwd->ywMin;
	;Assembler note: use hpldr as pwwd if bitCallPtOrigin is false
	mov	bx,[hpldr]
	test	al,bitCallPtOrigin
	jne	RCTC02
	mov	cx,[bx.xwMinWwd]
	mov	dx,[bx.ywMinWwd]
RCTC02:
	je	RCTC03
	push	ax	;save flags

;	 pt = PtOrigin(hpldr, idr);
	push	bx
	push	[idr]
ifdef DEBUG
	call	far ptr S_PtOrigin
else ;not DEBUG
	push	cs
	call	near ptr N_PtOrigin
endif ;DEBUG
	xchg	ax,cx

	pop	ax	;restore flags
RCTC03:

	test	al,bitUseVHwwdOrigin
	je	RCTC04
;	 pwwd = *vhwwdOrigin;
;	 dxPToW = pwwd->rcePage.xeLeft + pwwd->xwMin + pt.xl;
;	 dyPToW = pwwd->rcePage.yeTop + pwwd->ywMin + pt.yl;
	mov	bx,[vhwwdOrigin]
	mov	bx,[bx]
	add	cx,[bx.rcePageWwd.xeLeftRc]
	add	cx,[bx.xwMinWwd]
	add	dx,[bx.rcePageWwd.yeTopRc]
	add	dx,[bx.ywMinWwd]
RCTC04:

	test	al,bitNegate
	je	RCTC05
	neg	cx
	neg	dx
RCTC05:

;	 prcl->xlLeft = pdrcp->xp + pt.xl;
;	 prcl->xlRight = prcl->xlLeft + pdrcp->dxp;
;	 prcl->ylTop = pdrcp->yp + pt.yl;
;	 prcl->ylBottom = prcl->ylTop + pdrcp->dyp;

	xchg	ax,bx
	push	ds
	pop	es
	mov	si,[prcSrc]
	mov	di,[prcDest]
	errnz	<(xpLeftRc) - 0>
	lodsw
	add	ax,cx
	errnz	<(xpLeftRc) - 0>
	stosw
	test	bl,bitFromDrc
	je	RCTC06
	xchg	ax,cx
RCTC06:
	errnz	<(ypTopRc) - 2>
	lodsw
	add	ax,dx
	errnz	<(ypTopRc) - 2>
	stosw
	test	bl,bitFromDrc
	je	RCTC07
	xchg	ax,dx
RCTC07:
	errnz	<(xpRightRc) - 4>
	lodsw
	add	ax,cx
	errnz	<(xpRightRc) - 4>
	stosw
	errnz	<(ypBottomRc) - 6>
	lodsw
	add	ax,dx
	errnz	<(ypBottomRc) - 6>
	stosw
cEnd


;NATIVE XwFromXl(hpldr, xl)
;struct PLDR *hpldr;
;{
;	 struct PT pt;
;	 int xw;
;
;	 pt = PtOrigin(hpldr, -1);
;	 xw = xl + pt.xl + (*vhwwdOrigin)->rcePage.xeLeft + (*vhwwdOrigin)->xwMin;
;	 return(xw);
;}
; %%Function:XwFromXl %%Owner:BRADV
PUBLIC XwFromXl
XwFromXl:
	db	0A8h	;turns next "stc" into "test al,immediate"
			;also clears the carry flag

;NATIVE YwFromYl(hpldr, yl)
;struct PLDR *hpldr;
;{
;	 struct PT pt;
;	 int yw;
;
;	 pt = PtOrigin(hpldr, -1);
;	 yw = yl + pt.yl + (*vhwwdOrigin)->rcePage.yeTop + (*vhwwdOrigin)->ywMin;
;	 return(yw);
;}
; %%Function:YwFromYl %%Owner:BRADV
PUBLIC YwFromYl
YwFromYl:
	stc
	pop	dx
	pop	cx
	pop	bx
	mov	ax,-1
	push	ax
	push	bx
	push	cx
	push	dx
	sbb	al,al
	and	al,bitReturnY	    ;0 if XwFromXl, bitReturnY if YwFromYl
	db	03Dh	;turns next "mov al,immediate" into "cmp ax,immediate"

;NATIVE YwFromYp(hpldr/*hwwd*/, idr, yp)
;struct PLDR **hpldr;
;int idr;
;int yp;
;{
;	 struct PT pt;
;	 int yw;
;
;	 pt = PtOrigin(hpldr, idr);
;	 yw = yp + (*vhwwdOrigin)->rcePage.yeTop + (*vhwwdOrigin)->ywMin + pt.yl;
;	 return(yw);
;}
; %%Function:YwFromYp %%Owner:BRADV
PUBLIC YwFromYp
YwFromYp:
	mov	al,bitReturnY
	db	03Dh	;turns next "mov al,immediate" into "cmp ax,immediate"

;/* Y P  F R O M  Y W */
;NATIVE YpFromYw(hpldr/*hwwd*/, idr, yw)
;struct PLDR **hpldr;
;int idr;
;int yw;
;{
;	 struct PT pt;
;	 int yp;
;
;	 pt = PtOrigin(hpldr, idr);
;	 yp = yw - (*vhwwdOrigin)->rcePage.yeTop - (*vhwwdOrigin)->ywMin - pt.yl;
;	 return(yp);
;}
; %%Function:YpFromYw %%Owner:BRADV
PUBLIC YpFromYw
YpFromYw:
	mov	al,bitNegate + bitReturnY
	db	03Dh	;turns next "mov al,immediate" into "cmp ax,immediate"

;/* X W  F R O M  X P */
;NATIVE XwFromXp(hpldr/*hwwd*/, idr, xp)
;struct PLDR **hpldr;
;int idr;
;int xp;
;{
;	 struct PT pt;
;	 int xw;
;
;	 pt = PtOrigin(hpldr, idr);
;	 xw = xp + (*vhwwdOrigin)->rcePage.xeLeft + (*vhwwdOrigin)->xwMin + pt.xl;
;	 return(xw);
;}
; %%Function:XwFromXp %%Owner:BRADV
PUBLIC XwFromXp
XwFromXp:
	mov	al,0
	db	03Dh	;turns next "mov al,immediate" into "cmp ax,immediate"

;//* X P  F R O M  X W */
;NATIVE XpFromXw(hpldr/*hwwd*/, idr, xw)
;struct PLDR **hpldr;
;int idr;
;int xw;
;{
;	 struct PT pt;
;	 int xp;
;
;	 pt = PtOrigin(hpldr, idr);
;	 xp = xw - (*vhwwdOrigin)->rcePage.xeLeft - (*vhwwdOrigin)->xwMin - pt.xl;
;	 return(xp);
;}
; %%Function:XpFromXw %%Owner:BRADV
PUBLIC XpFromXw
XpFromXw:
	mov	al,bitNegate
;	db	03Dh	;turns next "mov al,immediate" into "cmp ax,immediate"

; %%Function:XOrYFromXOrY %%Owner:BRADV
cProc	XOrYFromXOrY,<PUBLIC,FAR>,<>
	ParmW	hpldr
	ParmW	idr
	ParmW	XOrY

cBegin

	push	ax	;save flags

;	 pt = PtOrigin(hpldr, idr);
	push	[hpldr]
	push	[idr]
ifdef DEBUG
	call	far ptr S_PtOrigin
else ;not DEBUG
	push	cs
	call	near ptr N_PtOrigin
endif ;DEBUG
	xchg	ax,cx

	pop	ax	;restore flags

;	 pwwd = *vhwwdOrigin;
;	 dxPToW = pwwd->rcePage.xeLeft + pwwd->xwMin + pt.xl;
;	 dyPToW = pwwd->rcePage.yeTop + pwwd->ywMin + pt.yl;
	mov	bx,[vhwwdOrigin]
	mov	bx,[bx]
	add	cx,[bx.rcePageWwd.xeLeftRc]
	add	cx,[bx.xwMinWwd]
	add	dx,[bx.rcePageWwd.yeTopRc]
	add	dx,[bx.ywMinWwd]

	test	al,bitNegate
	je	XOYFXOY02
	neg	cx
	neg	dx
XOYFXOY02:

	add	cx,[XOrY]
	add	dx,[XOrY]

	test	al,bitReturnY
	jne	XOYFXOY03
	mov	dx,cx
XOYFXOY03:
	xchg	ax,dx
cEnd


;-------------------------------------------------------------------------
;	FSectRc( prc1, prc2, prcDest )
;-------------------------------------------------------------------------
; %%FunctioN:FSectRc %%Owner:BRADV
cProc	FSectRc,<PUBLIC,FAR,ATOMIC>,<si,di>
        ParmW   prc1
        ParmW   prc2
        ParmW   prcDest
; /* F	S E C T   R C */
; /* prcDest gets the rectangle that is the intersection of prc1 and prc2. */
; /* Return value is TRUE iff prcDest is non-null */
; /* case when prcDest == prc1 OR prcDest == prc2 */
; native FSectRc( prc1, prc2, prcDest )
; struct RC *prc1, *prc2, *prcDest;
; {
cBegin

;  Assert( prcDest != NULL );
ifdef DEBUG
        cmp     [prcDest],0
	jnz	FSR01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midResn2
	mov	bx,1024
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FSR01:
endif ;DEBUG

        mov     si,[prc1]
        mov     di,[prc2]
	mov	bx,[prcDest]

	;Assembler note: instead of doing all of this checking, just
	;compute the result rc, and see if it is not empty
;  if (!FEmptyRc( prc1 ) && !FEmptyRc( prc2 ) &&
;      prc1->xpRight > prc2->xpLeft && prc2->xpRight > prc1->xpLeft &&
;      prc1->ypBottom > prc2->ypTop && prc2->ypBottom > prc1-> ypTop)
;         {
	push	ds
	pop	es

;	  prcDest->xpLeft = max( prc1->xpLeft, prc2->xpLeft );
;	  prcDest->ypTop = max( prc1->ypTop, prc2->ypTop );
	errnz	<(xpLeftRc) - 0>
	errnz	<(ypTopRc) - 2>
	xor	cx,cx
	db	03Ch	;turns next "inc cx" into "cmp al,immediate"
FSR02:
	inc	cx
	lodsw
	scasw
	jge	FSR03
	mov	ax,[di-2]
FSR03:
	mov	[bx],ax
	inc	bx
	inc	bx
	jcxz	FSR02

;	  prcDest->xpRight  = min( prc1->xpRight, prc2->xpRight );
;	  prcDest->ypBottom = min( prc1->ypBottom, prc2->ypBottom );
	errnz	<(xpRightRc) - 4>
	errnz	<(ypBottomRc) - 6>
	xor	cx,cx
	db	03Ch	;turns next "inc cx" into "cmp al,immediate"
FSR04:
	inc	cx
	lodsw
	scasw
	jle	FSR05
	mov	ax,[di-2]
FSR05:
	mov	[bx],ax
	inc	bx
	inc	bx
	jcxz	FSR04

;         return fTrue;
;         }
;	Assembler note: replace above test for
;
;  if (!FEmptyRc( prc1 ) && !FEmptyRc( prc2 ) &&
;      prc1->xpRight > prc2->xpLeft && prc2->xpRight > prc1->xpLeft &&
;      prc1->ypBottom > prc2->ypTop && prc2->ypBottom > prc1-> ypTop)
;
;	with a call to FNotEmptyPrc for the result rectangle
	sub	bx,cbRcMin
	;LN_FNotEmptyPrc takes prc in bx and returns the result in ax
	;Only ax and cx are altered.
	call	LN_FNotEmptyRc
	jne	FSR07

;  SetWords( prcDest, 0, cwRC );
	mov	di,bx
        mov     cx,cwRC
        rep     stosw

;  return fFalse;
FSR07:
; }
cEnd


;-------------------------------------------------------------------------
;	FEmptyRc( prc )
;-------------------------------------------------------------------------
; /* F	E M P T Y  R C */
; native FEmptyRc( prc )
; struct RC *prc;
; {   /* Return TRUE if the passed rect is empty (0 or negative height or width),
;        FALSE otherwise. */
; %%Function:FEmptyRc %%Owner:BRADV
PUBLIC FEmptyRc
FEmptyRc:
	pop	ax
	pop	dx
	pop	bx
	push	dx
	push	ax
	xor	ax,ax
	or	bx,bx
	je	FER01
	call	LN_FNotEmptyRc
FER01:
	inc	ax
	db	0CBh	    ;far ret

	;LN_FNotEmptyPrc takes prc in bx and returns the result in ax
	;Only ax and cx are altered.
LN_FNotEmptyRc:
;  return !((prc == NULL) ||
;         (prc->xpLeft >= prc->xpRight) ||
;	  (prc->ypTop >= prc->ypBottom));
	errnz	<fFalse>
	xor	ax,ax
	mov	cx,[bx.xpLeftRc]
	cmp	cx,[bx.xpRightRc]
	jge	FEP1
	mov	cx,[bx.ypTopRc]
	cmp	cx,[bx.ypBottomRc]
	jge	FEP1
	dec	ax
FEP1:
        or      ax,ax
; }
	ret


;-----------------------------------------------------------------------------
; FSetRgchDiff (pchBase,pchNew, pchDiff,cch) - compare 2 even sized char arrays,
;   setting a non-zero value into the corresponding fields of a 3rd such
;   array. The 3rd array is assumed to have been set to 0's befire this routine
;   was called. Returns non-zero if any differences were detected.
;
;     This is an implementation of the following C code:
;
;   /* how this works:
;      Each word of the 2 test arrays in XOR'ed together. If they are the same,

;      the result is 0, else non zero. The result is then OR'ed into the
;      difference array, effectively turning on bits in any fields where there
;      is a difference. The field in the difference array can then be tested
;      for non-zero to see if it was different in the 2 arrays.
;
;      We also keep an indicator, fDiff, originally set to 0,  which gets
;      or'ed into, so we can tell easily if ANY differences were detected.
;   */
;
;   while (cch--)
;       {
;	*pchDiff++ |=  (chT = *pchBase++ ^ *pchNew++);
;	fDiff |= chT;
;       }
;
;   return (fDiff != 0);
;}
;
;-----------------------------------------------------------------------------

; %%Function:FSetRgchDiff %%Owner:BRADV
cProc FSetRgchDiff,<FAR,PUBLIC,ATOMIC>,<SI,DI>
	parmDP	<pchBase>
	parmDP	<pchNew>
	parmDP	<pchDiff>
	parmW	<cch>
                ;/* **** -+-utility-+- **** */

	cBegin

	mov	si,[pchBase]
	mov	di,[pchNew]
	mov	bx,[pchDiff]
	mov	cx,[cch]
        xor     dx,dx      ;fDiff
	shr	cx,1
	jnc	FSRD01
	lodsb		   ; value in pchBase into ax, increment si
	xor	al,[di]    ; xor pchBase with pchNew
        inc     di
	or	[bx],al    ; result into pchDiff
	inc	bx
	or	dl,al	   ; or result into fDiff
FSRD01:
	jcxz	FSRD03	   ;cch == 0 ? finished
FSRD02:
	lodsw		   ; value in pchBase into ax, increment si
	xor	ax,[di]    ; xor pchBase with pwNew
        inc     di
        inc     di
	or	[bx],ax    ; result into pchDiff
	inc	bx
	inc	bx
        or      dx,ax      ; or result into fDiff
	loop	FSRD02	   ; loop if cch-- != 0
FSRD03:
	xchg	ax,dx
cEnd

;-------------------------------------------------------------------------
;	IScanLprgw( lprgw, w, iwMax )
;-------------------------------------------------------------------------
;/* I  S C A N	L P R G W */
;NATIVE IScanLprgw( lprgw, w, iwMax )
;WORD far *lprgw;
;WORD w;
;int iwMax;
;{
; int iw;
;
; for ( iw = 0 ; iw < iwMax ; iw++, lprgw++ )
;	 if (*lprgw == w)
;		 return iw;
;
; return iNil;
;}
; %%Function:IScanLprgw %%Owner:BRADV
PUBLIC IScanLprgw
IScanLprgw:
	mov	bx,sp
	push	di	;save caller's di
	les	di,[bx+8]
	mov	ax,[bx+6]
	mov	cx,[bx+4]
	mov	dx,cx
	repne	scasw
	je	ISL01
	;Assembler note: we know cx is zero at this point
	errnz	<iNil - (-1)>
	mov	cx,dx
ISL01:
	sub	dx,cx
	dec	dx
	xchg	ax,dx
	pop	di	;restore caller's di
	db	0CAh, 008h, 000h	;far ret pop 8 bytes


;-------------------------------------------------------------------------
;	CchNonZeroPrefix(rgb, cbMac)
;-------------------------------------------------------------------------
; /* C C H   N O N   Z E R O  P R E F I X  */
; /* returns the cbMac reduced over the trailing zeroes in rgb. This is
; the number of non-zero previx characters.
; */
; native int CchNonZeroPrefix(rgb, cbMac)
; char *rgb;
; int cbMac;
; {
; %%Function:CchNonZeroPrefix %%Owner:BRADV
PUBLIC CchNonZeroPrefix
CchNonZeroPrefix:
	pop	ax
	pop	dx
	pop	cx
	pop	bx
	push	dx
	push	ax
	push	di	    ;save caller's di

;	  for (rgb += cbMac - 1; cbMac > 0 && *rgb == 0; rgb--, cbMac--)
;		  ;
	push	ds
	pop	es
	mov	di,bx
	add	di,cx
	dec	di
	xor	ax,ax
	std
	repe	scasb
	cld
	je	CNZP01
	inc	cx
CNZP01:

;	  return(cbMac);
	xchg	ax,cx

; }
	pop	di	    ;restore caller's di
	db	0CBh	    ;far ret

;-------------------------------------------------------------------------
;	CchCopyLpszCchMax(lpch1, lpch2, cchMax)
;-------------------------------------------------------------------------
;/* C C H  C O P Y  L P S Z  C C H  M A X */
;/* Copy one sz to another, but not more than cchMax characters (incl '\0') */
;/* %%Function:CchCopyLpszCchMax %%Owner:rosiep */
;int CchCopyLpszCchMax(lpch1, lpch2, cchMax)
;char far *lpch1;
;char far *lpch2;
;int cchMax;
;{
;	int	cch = 0;
;	while (cch < cchMax && (*lpch2++ = *lpch1++) != 0)
;		cch++;
;
;	/* make sure it's null-terminated if we overflowed buffer */
;	if (cch == cchMax)
;		*(lpch2-1) = '\0';
;
;	return cch;
;}
cProc CchCopyLpszCchMax,<FAR,PUBLIC,ATOMIC>,<si,di>
	parmD	<lpch1>
	parmD	<lpch2>
	parmW	<cchMax>

cBegin
	les	di,[lpch1]
	push	di
	mov	al,0
	mov	cx,0FFFFh
	repne 	scasb
	pop	si
	push	es
	pop	ds
	sub	di,si
	mov	cx,[cchMax]
	cmp	cx,di
	jbe	CCLCM01
	mov	cx,di
CCLCM01:
	push	cx
	les	di,[lpch2]
	rep	movsb
	push	ss
	pop	ds
	dec	di
	stosb
	pop	ax
cEnd


;/* I B S T  F I N D  S Z F F N */
;/* **** +-+utility+-+string **** */
;/* ****
;*  Description: Given an hsttb of ffn's  and a pointer to an ffn, return
;*    the ibst for the string that matches the szFfn portion of the ffn,
;*    ignoring the ffid portion. If no match, return iNil.
;*    This is a variant of IbstFindSt in sttb.c.
;*
;*    This routine assumes user input of font names and so it does
;*    a CASE-INSENSITIVE string compare.  It no longer
;*    assumes you have already tested for the name "Default" since Tms Rmn
;*    is now the default font, so we start looking at 0
;*
;** **** */
;
;/*  %%Function: IbstFindSzFfn	%%Owner: davidlu  */
;
;EXPORT IbstFindSzFfn(hsttb, pffn)
;struct STTB **hsttb;
;struct FFN *pffn;
;{
;	struct STTB *psttb = *hsttb;
;	char *st2;
;	int ibst;
PUBLIC N_IbstFindSzFfn
N_IbstFindSzFfn:
	pop	ax
	pop	dx
	pop	cx	;pffn
	pop	bx	;hsttb
	push	dx
	push	ax
	push	di	    ;save caller's di
	push	si	    ;save caller's si
	mov	di,cx
	add	di,(szFfn)
	mov	bx,[bx]

;	Assert(!(*hsttb)->fExternal);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	test	[bx.fExternalSttb],maskFExternalSttb
	je	IFSF01
	mov	ax,midResn2
	mov	bx,1041 		   ;label # for native assert
	cCall	AssertProcForNative, <ax, bx>
IFSF01:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

;	for (ibst = 0; ibst < psttb->ibstMac; ibst++)
;		{
	xor	si,si
	mov	dx,[bx.ibstMacSttb]
	shl	dx,1
	add	bx,(rgbstSttb)
IFSF02:
	errnz	<iNil - (-1)>
	xor	ax,ax
	cmp	si,dx
	jae	IFSF04

;		st2 = PstFromSttb(hsttb, ibst);
	push	bx	;save &psttb->rgbst
	;***Begin in-line PstFromSttb (assume !fExternal)
	add	bx,[bx+si]
	;***End in-line PstFromSttb (assume !fExternal)

;		if ( *st2 == *(CHAR *)pffn &&
	mov	al,[bx]
	sub	al,[di-(szFfn)]
	jne	IFSF03

;				!FNeNcRgch(pffn->szFfn, ((struct FFN *)st2)->szFfn,
;				CbSzOfPffn(pffn) - 1) )
	;Call FNeNcSz here rather than FNeNcRgch to save code in the
	;assembler version.
	push	dx	;save 2*ibstMac
	add	bx,(szFfn)
	push	di
	push	bx
	push	cs
	call	near ptr FNeNcSz
	pop	dx	;restore 2*ibstMac
IFSF03:

;			return(ibst);
	pop	bx	;restore &psttb->rgbst

;		}
	inc	si
	inc	si
	or	ax,ax
	jne	IFSF02
	xchg	ax,si
	shr	ax,1

;	return(iNil);
;}
IFSF04:
	dec	ax
	pop	si	    ;restore caller's si
	pop	di	    ;restore caller's di
	db	0CBh	    ;far ret


sEnd	resn2
	end
