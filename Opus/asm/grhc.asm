;*------------------------------------------------------------------------
;*
;*
;* Microsoft Word -- Graphics
;*
;* Hand-code versions of C routines
;*
;*------------------------------------------------------------------------ 


	include w2.inc
	include noxport.inc
        include grtiff.inc

midGrhc     equ 17

        externFP <ReloadSb>
	externFP <N_HpchFromFc>

ifdef DEBUG
	externFP <S_HpchFromFc>
	externFP <AssertProcForNative>
endif ;DEBUG


createSeg	grtiff_PCODE,GRTIFF,BYTE,PUBLIC,CODE

sBegin		GRTIFF

	assumes	cs,GRTIFF
	assumes	ss,data
	assumes	ds,data
	assumes	es,nothing



mpnbb:	db	000h,000h,000h,000h
	db	0ffh,000h,000h,000h
	db	000h,0ffh,000h,000h
	db	0ffh,0ffh,000h,000h
	db	000h,000h,0ffh,000h
	db	0ffh,000h,0ffh,000h
	db	000h,0ffh,0ffh,000h
	db	0ffh,0ffh,0ffh,000h
	db	000h,000h,000h,0ffh
	db	0ffh,000h,000h,0ffh
	db	000h,0ffh,000h,0ffh
	db	0ffh,0ffh,000h,0ffh
	db	000h,000h,0ffh,0ffh
	db	0ffh,000h,0ffh,0ffh
	db	000h,0ffh,0ffh,0ffh
	db	0ffh,0ffh,0ffh,0ffh


;-------------------------------------------------------------------------------
;	ReadTIFFLine(hpc, hpPlane)
;-------------------------------------------------------------------------------
;ReadTIFFLine(hpc, hpPlane)
;PICT		**hpc;
;char HUGE	*hpPlane;
;{
;	int			ibitIn,
;				ibIn,
;				cbRowFile,
;				cbitsSample;
;	BYTE		b;
;	unsigned	cb;
;
;   %%Function: ReadTIFFLine 	  %%Owner:  Bobz
;
cProc	ReadTIFFLineNat,<FAR,PUBLIC>,<SI,DI>
	parmW	hpc
	parmD	hpPlane

cBegin
;
;	ibitIn = ibIn = 0;
;	cbRowFile = (*hpc)->tiff.cbRowFile;
;	cbitsSample = (*hpc)->tiff.cbitsSample;
;	Assembler note: Set ibitIn, ibIn, and cbRowFile later...
	mov	bx,[hpc]
	mov	bx,[bx] 	;bx = *hpc
	mov	ax,[bx.tiffPict.cbitsSampleTiff]

;	if ((*hpc)->tiff.wCompScheme == Scheme1)
;		{ 
	cmp	[bx.tiffPict.wCompSchemeTiff],Scheme1
	jz	Ltemp001
	jmp	RTL11
Ltemp001:

;		if (cbitsSample == 1)
;			{
;			RgbFromStm(hpPlane, cbRowFile);
;			}
	cmp	ax,1
	jnz	RTL01
	mov	di,[OFF_hpPlane]
	mov	cx,[bx.tiffPict.cbRowFileTiff]
	xor	si,si			;make sure we initialize pstream
	;LN_RgbFromStm blts cx bytes pointed to by the stream
	;in ds:si to the hp in hpPlane in es:di.
	;ax, bx, cx, si, di, ds, es are altered.
	call	LN_RgbFromStm
Ltemp002:
	jmp	RTL16

;		else
;			{
;			char HUGE * hpSave;
;			int 		ipl;
;			unsigned	bSpecial = 0x80;
;			unsigned	nb;
;			int		fFirstNibble = true;
;			unsigned	cbPlane = (*hpc)->cbPlane;
RTL01:
	sub	ax,4
	jz	RTL02
	mov	al,1
RTL02:
	mov	dh,1
	xor	dh,al
	mov	cx,[bx.vtExtPict.hGpoint]
	mov	bx,[bx.cbPlanePict]

	mov	di,[OFF_hpPlane]

	mov	dl,080H 		;set bSpecial
	mov	ah,1
	xor	si,si			;make sure we initialize pstream
	jmp	short RTL06

RTL03:
	;LN_SetEsDsSiFromStm sets ds:si pointing at a stream of bytes
	;according to the current file position stored in stmGlobal, and
	;sets es according to [SEG_hpPlane]
	call	LN_SetEsDsSiFromStm
	jmp	short RTL07

RTL04:
	push	ax
	and	al,0fh
	shl	al,1
	shl	al,1
	jmp	short RTL08

;			while (ibitIn < (*hpc)->vtExt.h)
;				{
RTL06:
	jcxz	Ltemp002

;				if (fFirstNibble)
;					{
	xor	ah,dh
	test	ah,dh
	jne	RTL04

;					b = BFromStm();
;					nb = (b & 0xf0) >> 2;
;					if (cbitsSample == 4)
;						fFirstNibble = false;
;					}
	test	si,001FFh
	je	RTL03
RTL07:
	lodsb
	push	ax
	and	al,0f0h
	shr	al,1
	shr	al,1			;si = nb

	;Assembler note: the else clause here is done at RTL04
;				else
;					{
;					nb = (b & 0x0f) << 2;
;					fFirstNibble = true;
;					}
;				hpSave = hpPlane;
RTL08:
	xor	ah,ah
	add	ax,offset [mpnbb]
	push	di	    ;save OFF_hpPlane
	push	si	    ;save OFF_hpStream
	xchg	ax,si

;				for (ipl=0; ipl<4; ipl++)
;					{
;					*hpPlane |= (mpnbb[nb++] & bSpecial);
;					hpPlane += cbPlane;
;					}
	lods	byte ptr cs:[si]
	and	al,dl
	or	es:[di],al
	add	di,bx

	lods	byte ptr cs:[si]
	and	al,dl
	or	es:[di],al
	add	di,bx

	lods	byte ptr cs:[si]
	and	al,dl
	or	es:[di],al
	add	di,bx

	lods	byte ptr cs:[si]
	and	al,dl
	or	es:[di],al
	add	di,bx

;				hpPlane = hpSave;
	pop	si	    ;restore OFF_hpStream
	pop	di	    ;restore OFF_hpPlane
	pop	ax

;				bSpecial >>= 1;
;				if (bSpecial == 0)
;					{
;					bSpecial = 0x80;
;					hpPlane++;
;					}
;				ibitIn++;
;				}
;			}
;		}
	dec	cx		;ibitIn++
	ror	dl,1
	jnc	RTL06
	inc	di		;hpPlane++;
	jmp	short RTL06

RTL09:
	;LN_SetEsDsSiFromStm sets ds:si pointing at a stream of bytes
	;according to the current file position stored in stmGlobal, and
	;sets es according to [SEG_hpPlane]
	call	LN_SetEsDsSiFromStm
	jmp	short RTL13

RTL10:
	;LN_SetEsDsSiFromStm sets ds:si pointing at a stream of bytes
	;according to the current file position stored in stmGlobal, and
	;sets es according to [SEG_hpPlane]
	call	LN_SetEsDsSiFromStm
	jmp	short RTL14

;	else
;		{
;		int w;
;		while (ibIn < cbRowFile)
;			{
;			w = (signed char)BFromTIFF();
RTL11:
	mov	di,[OFF_hpPlane]	;es:di = lpPlane
	mov	bx,[bx.tiffPict.cbRowFileTiff]
	xor	dx,dx			;ibIn = 0;
	xor	si,si			;make sure we initialize pstream
	
RTL12:
	cmp	dx,bx
	jge	RTL16			;goto LEntTIFF

	test	si,001FFh
	je	RTL09
RTL13:
	lodsb				;get w
	xchg	ax,cx
	xor	ch,ch

;			if (w < 0)
;				{
	or	cl,cl
	jge	RTL15

;				cb = -w+1;
;				b = BFromTIFF();
;				bltbcx(b, LpFromHp(hpPlane), cb);
;				hpPlane += cb;
;				ibIn += cb;
;				}
	neg	cl
	inc	cx
	test	si,001FFh
	je	RTL10
RTL14:
	lodsb				;get byte after w

	add	dx,cx		;ibIn+=cb
	rep	stosb
	jmp	RTL12

;			else
;				{
;				cb = w+1;
;				RgbFromStm(hpPlane, cb);
;				hpPlane	+= cb;
;				ibIn += cb;
;				}
;			}
;		}
RTL15:
	inc	cx
	add	dx,cx
	;LN_RgbFromStm blts cx bytes pointed to by the stream
	;in ds:si to the hp in hpPlane in es:di.
	;ax, bx, cx, si, di, ds, es are altered.
	push	bx	;save cbRowFile
	call	LN_RgbFromStm
	pop	bx	;restore cbRowFile
	jmp	short RTL12

;}
RTL16:
	push	ss
	pop	ds		;restore sbApp DS
	and	si,001FFh
	je	RTL17
	sub	si,00200h
	add	[stmGlobal.LO_fcStm],si
	adc	[stmGlobal.HI_fcStm],0FFFFh
RTL17:
cEnd


;-------------------------------------------------------------------------------
;	LN_RgbFromStm
;-------------------------------------------------------------------------------
	;LN_RgbFromStm blts cx bytes pointed to by the stream
	;in ds:si to the hp in hpPlane in es:di.
	;ax, bx, cx, si, di, ds, es are altered.
RFS01:
	;LN_SetEsDsSiFromStm sets ds:si pointing at a stream of bytes
	;according to the current file position stored in stmGlobal, and
	;sets es according to [SEG_hpPlane]
	call	LN_SetEsDsSiFromStm
	mov	bx,00200h
	test	si,001FFh
	je	RFS02
LN_RgbFromStm:
	mov	bx,si
	neg	bx
	and	bh,001h
RFS02:
	mov	ax,cx
	sub	ax,bx
	jl	RFS03
	mov	cx,bx
RFS03:
	rep	movsb
	xchg	ax,cx
	or	cx,cx
	jg	RFS01
	ret


;-------------------------------------------------------------------------------
;	LN_SetESFromHpPlane
;-------------------------------------------------------------------------------
	;Sets es according to hpPlane.	Only es is altered.
LN_SetEsFromHpPlane:
	push	bx
	mov	bx,[SEG_hpPlane]
	;LN_ReloadSb sets ES corresponding to the sb in bx.
	;Only bx and es are altered.
	call	LN_ReloadSb
	pop	bx
	ret


;-------------------------------------------------------------------------------
;	LN_SetEsDsSiFromStm
;-------------------------------------------------------------------------------
	;LN_SetEsDsSiFromStm sets ds:si pointing at a stream of bytes
	;according to the current file position stored in stmGlobal, and
	;sets es according to [SEG_hpPlane]
LN_SetEsDsSiFromStm:
	push	ax
	push	bx
	push	cx
	push	dx
	push	ss
	pop	ds	    ;restore sbApp DS
	push	[stmGlobal.fnStm]
	push	[stmGlobal.HI_fcStm]
	push	[stmGlobal.LO_fcStm]
ifdef DEBUG
	cCall	S_HpchFromFc
else ;not DEBUG
	cCall	N_HpchFromFc
endif ;DEBUG
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	xor	ax,[stmGlobal.LO_fcStm]
	and	ax,001FFh
	je	SDSFS01
	mov	ax,midGrhc
	mov	bx,1000 		   ; label # for native assert
	cCall	AssertProcForNative, <ax, bx>
SDSFS01:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG
	add	[stmGlobal.LO_fcStm],00200h   ; increment fc
	adc	[stmGlobal.HI_fcStm],0
	and	[stmGlobal.LO_fcStm],0FE00h
	;Do this extra LN_SetEsFromHpPlane to avoid an LMEM quirk.
	;It is possible that the call later on to ReloadSb for SEG_hpPlane
	;would force out the ps for SB_stream.	This would happen if now
	;SB_stream was oldest on the LRU list but still swapped in, and
	;SEG_hpPlane was swapped out.  In that event ReloadSb would not
	;be called for SB_stream, the LRU entry would not get updated,
	;and the call to ReloadSb for SEG_hpPlane would swap out SB_stream.
	;Sets es according to hpPlane.	Only es is altered.
	call	LN_SetEsFromHpPlane
	xchg	ax,si
	mov	bx,dx
	;LN_ReloadSb sets ES corresponding to the sb in bx.
	;Only bx and es are altered.
	call	LN_ReloadSb
	push	es
	;Sets es according to hpPlane.	Only es is altered.
	call	LN_SetEsFromHpPlane
	pop	ds
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret


;-------------------------------------------------------------------------------
;	LN_ReloadSb
;-------------------------------------------------------------------------------
	;LN_ReloadSb sets ES corresponding to the sb in bx.
	;Only bx and es are altered.
LN_ReloadSb:
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,ds
	mov	bx,ss
	cmp	ax,bx
	je	LN_RS01
	mov	ax,midGrhc
	mov	bx,1001 		   ; label # for native assert
	cCall	AssertProcForNative, <ax, bx>
LN_RS01:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG
        push    ax
	shl	bx,1
	mov	ax,[bx.mpsbps]
	mov	es,ax
	shr	ax,1
	jc	LN_RS02
	push	cx
	push	dx
;	ReloadSb trashes ax, cx, and dx
	cCall	ReloadSb,<>
	pop	dx
	pop	cx
LN_RS02:
	pop	ax
	ret


;=========================================================================
;
;	UDiv : divide first parm by third parm ( yielding 16 bit result )
;
;	int MultDiv(wNumer, wDenom, pRemain)
;	int wNumer, wDenom;
;	int *pRemain;
;	{
;		*pRemain = wNumer % wDenom;
;		return ( wNumer / wDenom );
;	}
;
;=========================================================================
;   %%Function:  UDiv	  %%Owner:  Bobz
;
cProc	UDiv,<FAR,PUBLIC>,<>
ParmW	<wNumer, wDenom, pRemain>

cBegin UDiv
	xor	dx,dx			; DX:AX = numerator
	mov	ax,[wNumer]		;

	mov	cx,[wDenom]		; CX = denominator
	jcxz	UDOv

	div	cx			; and divide by wDenom
UDret:
	mov	bx,pRemain		;
	mov	[bx],dx 		; store the remainder
cEnd UDiv

UDOv:
	mov	ax,0FFFFH
	jmp	UDret


sEnd	GRTIFF
	end
