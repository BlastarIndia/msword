        include w2.inc
        include noxport.inc
        include consts.inc
        include structs.inc

createSeg	disptbl_PCODE,disptbln,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midDisptbln	equ 25		; module ID, for native asserts
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

externFP	<FSetRgchDiff>

ifdef DEBUG
externFP	<AssertProcForNative>
endif


sBegin  data

; 
; /* E X T E R N A L S */
; 
externW 	vfti		; extern struct FTI	  vfti;

sEnd    data


; CODE SEGMENT _disptbln

sBegin	disptbln
	assumes cs,disptbln
        assumes ds,dgroup
        assumes ss,dgroup

;-------------------------------------------------------------------------
;	WidthHeightFromBrc(brc, grpf)
;-------------------------------------------------------------------------
;HANDNATIVE C_WidthHeightFromBrc(brc, grpf)
;struct BRC brc;
;int grpf;
;{
;	int dzp, dz = 0, dxp;
;	int fFrameLines, fWidth, fLine;
; %%Function:WidthHeightFromBrc %%Owner:tomsax
PUBLIC N_WidthHeightFromBrc
N_WidthHeightFromBrc:
	mov	bx,sp
	mov	cx,[bx+4]
	errnz	<cbBrcMin - 2>
	mov	dx,[bx+6]
	;We now have brc in dx, grpf in cx

;	fFrameLines = grpf & 1;
;	fWidth = grpf & 2;
;	fLine = grpf & 4;
maskFFrameLinesLocal	equ 1
maskFWidthLocal 	equ 2
maskFLineLocal		equ 4

;	dzp = fWidth ? dxpBorderFti : dypBorderFti;
;#define dypBorderFti		 vfti.dypBorder
;#define dxpBorderFti		 vfti.dxpBorder
	mov	ax,[vfti.dxpBorderFti]
	test	cl,maskFWidthLocal
	jne	WHFB01
	mov	ax,[vfti.dypBorderFti]
WHFB01:

;	if ((int) brc == brcNone && fFrameLines)
;		return dzp;
	mov	bx,ax
	errnz	<brcNone - 0>
	or	dx,dx
	jne	WHFB02
	test	cl,maskFFrameLinesLocal
	jne	WHFB10
WHFB02:

;	if ((int) brc == brcNone || (int) brc == brcNil)
;		return 0;
	xor	ax,ax
	errnz	<brcNil - (-1)>
	inc	dx
	je	WHFB10
	errnz	<brcNone - 0>
	dec	dx
	je	WHFB10

;		/* All the more difficult cases */
;		if (!fLine)
;			/* brc.dxpSpace is in points - Hungarian name is wrong */
;			dz = NMultDiv(brc.dxpSpace,
;				fWidth ? vfti.dxpInch : vfti.dypInch,
;				cptInch);
	test	cl,maskFLineLocal
	jne	WHFB07
	push	bx	;save dzp
	push	dx	;save brc
	mov	ax,[vfti.dxpInchFti]
	test	cl,maskFWidthLocal
	jne	WHFB04
	mov	ax,[vfti.dypInchFti]
WHFB04:
	errnz	<(dxpSpaceBrc) - 1>
	errnz	<maskDxpSpaceBrc - 03Eh>
	mov	dl,dh
	and	dx,maskDxpSpaceBrc
	shr	dx,1
	imul	dx
	add	ax,cptInch SHR 1
	adc	dx,0
ifdef DEBUG
	;Assert the following idiv will not overflow with a call
	;so as not to mess up short jumps.
	call	WHFB11
endif ;DEBUG
	mov	bx,cptInch
	idiv	bx
	pop	dx	;restore brc
	pop	bx	;restore dzp
WHFB07:

;		if (brc.fShadow)
;			dz += fWidth ? dxShadow : dyShadow;
;#define dxShadow		 dxpBorderFti
;#define dyShadow		 dypBorderFti
;PAUSE
	errnz	<(fShadowBrc) - 1>
	test	dh,maskFShadowBrc
	je	WHFB08
	add	ax,bx
WHFB08:

;		switch(brc.brcBase)
;			{
;		case brcSingle:
;			return dz + dzp;
;		case brcTwoSingle:
;			return dz + 3 * dzp;
;		case brcThick:
;			return dz + 2 * dzp;
;			}
;		Assert(fFalse);
;		return 0;
	errnz	<maskBrcBaseBrc - 001FFh>
	and	dh,maskBrcBaseBrc SHR 8
	add	ax,bx
	cmp	dx,brcSingle
	je	WHFB09
	add	ax,bx
	cmp	dx,brcThick
	je	WHFB09
	add	ax,bx
	cmp	dx,brcTwoSingle
	je	WHFB09
ifdef DEBUG
	;Assert (fFalse) with a call so as not to mess up short jumps.
	call	WHFB14
endif ;DEBUG
	xor	ax,ax
	jmp	short WHFB10
WHFB09:

;	return dz;
WHFB10:

;}
	db	0CAh, 004h, 000h    ;far ret, pop 4 bytes


ifdef DEBUG
WHFB11:
	push	ax
	push	bx
	push	cx
	push	dx
	or	dx,dx
	jns	WHFB12
	neg	dx
WHFB12:
	cmp	dx,cptInch SHR 1
	jb	WHFB13
	mov	ax,midDisptbln
	mov	bx,1001
	cCall	AssertProcForNative,<ax,bx>
WHFB13:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG

ifdef DEBUG
WHFB14:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midDisptbln
	mov	bx,1002
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG

sEnd	disptbln
        end
