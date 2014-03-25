;*------------------------------------------------------------------------
;*
;*
;* Microsoft Word -- Graphics
;*
;* Hand-coded stretch-blt
;*
;*------------------------------------------------------------------------ 


ifdef PCWORD
	include	word.inc

	AssertDataAsm	'grbitx.asm'

	include grconsts.db
	include grstruct.db
	include grdriver.db
endif

ifdef WIN
	include w2.inc
	include noxport.inc
        include grtiff.inc

ifdef DEBUG
midGrbitx	equ 16		 ; module ID, for native asserts
endif

endif

createSeg	grbit_PCODE,GRBIT,BYTE,PUBLIC,CODE
sBegin		GRBIT

	assumes	cs,grbit
	assumes	ss,data
	assumes	ds,data
	assumes	es,nothing


;-------------------------------------------------------------------------
; StrBltRow(ppc, pbSrc, pbDst)
; PICT	*ppc;
; char	*pbSrc,
;	*pbDst;
;
; Stretch-blts a single row, copying from pbSrc to pbDst.
; Can handle compression or expansion.
;
; WARNING: DS is changed for the duration of this routine.
;
; The comments alongside the code do not really correspond
; to the original C version.
;
; Changes made to this routine should also be made to MonoStrBltRow
; as appropriate.
;-------------------------------------------------------------------------


cProc	StrBltRow,<FAR,PUBLIC>,<si,di,ds>

ParmW	<ppc>				; PICT *ppc;
ParmW	<pbSrc, pbDst>			; char *pbSrc, *pbDst;

LocalW	<cplOut, cplIn>		 	; these are taken right from ppc
LocalW	<rgico>				; and put here for faster access
LocalW	<cbitRepNorm, wBitDelta>
LocalW	<virtSrcMac>			; (ppc->vtExt.h)

LocalW	<wRem>				; the current stretchblt remainder
LocalW	<ibitSrc>			; current bit in source word
LocalW	<ibitDst>			; current bit in dest word
LocalW	<virtSrc>			; current source pixel
LocalW	<cbitRep>			; # of times to repeat this pixel
LocalW	<ibLastPlane>			; cbPlane * 3

ifdef DEBUG
LocalW	<cbitReallySent>
endif

;
;   %%Function:  StrBltRow      %%Owner:  bobz
;

cBegin	StrBltRow

	mov	si,ppc			; copy stuff over from PICT
	mov	ax,[si].cplOutPict
	mov	cplOut,ax
	mov	ax,[si].cplInPict
	mov	cplIn,ax
	lea	ax,[si].rgicoPict
	mov	rgico,ax
	mov	ax,[si].cbitRepNormPict
	mov	cbitRepNorm,ax
	mov	ax,[si].wBitDeltaPict
	mov	wBitDelta,ax
	mov	ax,[si].vtExtPict.hGpoint
	mov	virtSrcMac,ax

ifdef DEBUG
	mov	ax,[si].pixExtPict.hGpoint
	mov	cbitReallySent,ax
endif
	xor	ax,ax
	mov	wRem,ax
	mov	virtSrc,ax
	mov	cx,[si].cplInPict
	mov	dx,[si].cbPlanePict	; DX = cbPlane (always)
	jmp	short SB081
SB080:	add	ax,dx
SB081:	loop	SB080
	mov	ibLastPlane,ax		; offset to last plane

	PsFromSb ds,[si].sbBitsPict,bx,dx  ; set DS to point to sbBits
	assumes ds,nothing


	; don't make any assumptions about ax,bx,si,di at this point
	
	mov	ibitDst,7
	mov	ibitSrc,8	; (this is different from C source)

	mov	si,pbSrc	; these point into sbBits
	mov	di,pbDst


SBNextSrc:
	mov	ax,virtSrc	; for (virtSrc = 0;
	cmp	ax,virtSrcMac	;	virtSrc < ppc->vtExt.h;
	jl	SB100		;	++virtSrc)
	jmp	SBCleanup
SB100:
	inc	virtSrc
	mov	cx,cbitRepNorm	; cbitRep = ppc->cbitRepNorm;
	mov	ax,wRem
	add	ax,wBitDelta	; wRem += wBitDelta;
	mov	bx,virtSrcMac
	cmp	ax,bx		; if (wRem >= ppc->vtExt.h)
	jl	SB110		;	{
	inc	cx		;	++cbitRep;
	sub	ax,bx		; 	wRem -= ppc->vtExt.h
SB110:				;	}
	mov	cbitRep,cx
	mov	wRem,ax
	dec	ibitSrc		; if (ibitSrc-- == 0)
	jns	SBLoadBits	;	{
	mov	ibitSrc,7	; 	ibitSrc = 7; (7 bits left after this)
	inc	si		;	++pbSrc;
SBLoadBits:			;	}
	mov	cx,cplIn	; ipl = cplIn;
	mov	bx,ibLastPlane	; ibPlane = ibLastPlane;
	xor	ax,ax		; ico = 0;
SBLoadLoop:			; while (ipl > 0)
	shl	byte ptr ds:[si+bx],1 ;	{
	rcl	ax,1		; 	LSB of ico = MSB of *(pbSrc+ibPlane);
	sub	bx,dx		; 	ibPlane -= cbPlane;
	loop	SBLoadLoop	; 	}

	dec	cbitRep		; if (cbitRep-- > 0)
	js	SBNextSrc	; {

	mov	bx,rgico
	xlat	DGROUP:[bx]	; ico = ppc->rgico;
	mov	ah,al		; (save away ico)

SBNextDst:			; do {

ifdef DEBUG
	dec	cbitReallySent
endif
	mov	bx,cx		; ibPlane = 0;
	mov	cx,cplOut	; for (ipl = cplOut; ipl > 0; --ipl)
SBStoreLoop:			;     {
	shr	al,1		;     shift LSB into carry
	rcl	byte ptr ds:[di+bx],1 ; shift carry into LSB of *(pbDst+ibPlane)
	add	bx,dx		;     ibPlane += cbPlane;
	loop	SBStoreLoop	;     }

	dec	ibitDst		; if (ibitDst-- == 0)
	jns	SB200		;     {
	inc	di		;     ++pbDst;
	mov	ibitDst,7	;     ibitDst = 7;
SB200:				;     }
	mov	al,ah		; restore ico
	dec	cbitRep		; } while (cbitRep-- > 0);
	jns	SBNextDst
	jmp	SBNextSrc
	
SBCleanup:
	mov	cl,byte ptr ibitDst
	cmp	cl,7
	je	SBRet
	inc	cx		; number of bits to shift left last bytes
	mov	ax,cplOut
	xor	bx,bx
SBFixupLoop:
	shl	byte ptr ds:[di+bx],cl	; shift over remaining bits
	add	bx,dx			; in last bytes
	dec	ax
	jnz	SBFixupLoop

	
SBRet:

ifdef PCWORD
	AssertEQ cbitReallySent,0
endif

ifdef WIN
	AssertEQ cbitReallySent,0,midGrbitx,100
endif

cEnd	StrBltRow


ifdef NEVER

; Patterns for representing colors on a black and white device
; These patterns were created with pattern.c.  The patterns
; are not stored in the standard Chart/Works format, in which
; the 8 bytes of a pattern are stored consecutively.
; Here the bytes from each pattern for a row are stored consecutively;
; that is we have a 2-dimensional byte array, with the major index
; being the row and the minor index being the pattern.
;
; This ordering helps us in the stretchblt; we determine which row table
; to look at once per row, and then use the normalized color
; (from ppc->rgico) as an offset into the selected row table
; when we want to translate a color to a pattern.

labelB	<PUBLIC,rgirwicoPat>

ifdef BAYER
	db	0000h, 0088h, 0088h, 00aah, 00aah, 00aah, 00aah, 00aah
	db	00aah, 00eeh, 00eeh, 00ffh, 00ffh, 00ffh, 00ffh, 00ffh

	db	0000h, 0000h, 0000h, 0000h, 0000h, 0044h, 0044h, 0055h
	db	0055h, 0055h, 0055h, 0055h, 0055h, 00ddh, 00ddh, 00ffh

	db	0000h, 0000h, 0022h, 0022h, 00aah, 00aah, 00aah, 00aah
	db	00aah, 00aah, 00bbh, 00bbh, 00ffh, 00ffh, 00ffh, 00ffh

	db	0000h, 0000h, 0000h, 0000h, 0000h, 0000h, 0011h, 0011h
	db	0055h, 0055h, 0055h, 0055h, 0055h, 0055h, 0077h, 00ffh

	db	0000h, 0088h, 0088h, 00aah, 00aah, 00aah, 00aah, 00aah
	db	00aah, 00eeh, 00eeh, 00ffh, 00ffh, 00ffh, 00ffh, 00ffh

	db	0000h, 0000h, 0000h, 0000h, 0000h, 0044h, 0044h, 0055h
	db	0055h, 0055h, 0055h, 0055h, 0055h, 00ddh, 00ddh, 00ffh

	db	0000h, 0000h, 0022h, 0022h, 00aah, 00aah, 00aah, 00aah
	db	00aah, 00aah, 00bbh, 00bbh, 00ffh, 00ffh, 00ffh, 00ffh

	db	0000h, 0000h, 0000h, 0000h, 0000h, 0000h, 0011h, 0011h
	db	0055h, 0055h, 0055h, 0055h, 0055h, 0055h, 0077h, 00ffh

else ; course fatting

	db	0000h, 0000h, 0000h, 0000h, 0000h, 0000h, 0000h, 0030h
	db	00f0h, 00f3h, 00ffh, 00ffh, 00ffh, 00ffh, 00ffh, 00ffh

	db	0000h, 0040h, 0060h, 00e0h, 00e0h, 00e0h, 00f0h, 00f0h
	db	00f0h, 00f0h, 00f0h, 00f8h, 00f8h, 00f8h, 00f9h, 00ffh

	db	0000h, 0040h, 0060h, 00e0h, 00e0h, 00e0h, 00f0h, 00f0h
	db	00f0h, 00f0h, 00f0h, 00f8h, 00f8h, 00f8h, 00f9h, 00ffh

	db	0000h, 0000h, 0000h, 0000h, 00c0h, 00f0h, 00f0h, 00f0h
	db	00f0h, 00f0h, 00f0h, 00f0h, 00fch, 00ffh, 00ffh, 00ffh

	db	0000h, 0000h, 0000h, 0000h, 0000h, 0000h, 0000h, 0003h
	db	000fh, 003fh, 00ffh, 00ffh, 00ffh, 00ffh, 00ffh, 00ffh

	db	0000h, 0004h, 0006h, 000eh, 000eh, 000eh, 000fh, 000fh
	db	000fh, 000fh, 000fh, 008fh, 008fh, 008fh, 009fh, 00ffh

	db	0000h, 0004h, 0006h, 000eh, 000eh, 000eh, 000fh, 000fh
	db	000fh, 000fh, 000fh, 008fh, 008fh, 008fh, 009fh, 00ffh

	db	0000h, 0000h, 0000h, 0000h, 000ch, 000fh, 000fh, 000fh
	db	000fh, 000fh, 000fh, 000fh, 00cfh, 00ffh, 00ffh, 00ffh

endif ;!BAYER

cbrowPat = ($-rgirwicoPat) / 8	; size of each row table

endif ;NEVER


	assumes	cs,grbit
	assumes	ss,data
	assumes	ds,data
	assumes	es,nothing


;-------------------------------------------------------------------------
; MonoStrBltRow(ppc, pbSrc, pbDst, irw, psPat)
; PICT	*ppc;
; char	*pbSrc,		/* get source data from here */
;	*pbDst;		/* put result here */
; unsigned irw;		/* row number */
; WORD	psPat;		/* physical segment containing dither patterns */
;
; Stretch-blts a single row, copying from pbSrc to pbDst.
; Can handle compression or expansion.
; The output row is a single plane B&W image;
; the input row is a multi-plane color image.
; The colors or gray levels are translated to B&W patterns.
;
; IMPORTANT: The source row is not changed, unless the destination
; happens to overwrite it.  This is necessary here since we may stretchblt
; from the same source row to several destination rows.
;
; WARNING: DS is changed for the duration of this routine.
;
; Beware of the differences between StrBltRow and MonoStrBltRow:
; MonoStrBltRow always has only one ouput plane.
; ibitDst is initialized to 8 for MonoStrBltRow and 7 for StrBltRow.
; irw is passed to MonoStrBltRow but not to StrBltRow.
; StrBltRow always destroys the source row; MonoStrBltRow only destroys
; it if the destination overwrites the source.
;
;-------------------------------------------------------------------------


cProc	MonoStrBltRow,<FAR,PUBLIC>,<si,di,ds>

ParmW	<ppc>				; PICT *ppc;
ParmW	<pbSrc, pbDst>			; char *pbSrc, *pbDst;
ParmW	<irw>				; unsigned irw;
ParmW	<psPat>				; WORD psPat;

LocalW	<cplIn>			 	; these are taken right from ppc
LocalW	<cbitRepNorm, wBitDelta>	; and put here for faster access
LocalW	<virtSrcMac>			; (ppc->vtExt.h)

LocalW	<wRem>				; the current stretchblt remainder
LocalW	<ibitSrc>			; current bit in source word
LocalW	<ibitDst>			; current bit in dest word
LocalW	<virtSrc>			; current source pixel
LocalW	<cbitRep>			; # of times to repeat this pixel
LocalW	<ibLastPlane>			; cbPlane * cplIn
LocalW	<rgpat>				; the row table to use

ifdef DEBUG
LocalW	<cbitReallySent>
endif

;     %%Function:  MonoStrBltRow     %%Owner:  bobz
;

cBegin	MonoStrBltRow

	mov	si,ppc			; copy stuff over from PICT
ifdef PCWORD
	AssertEQ [si].cplOutPict,1	; single plane output
	AssertGT [si].cplInPict,1	; but color input
endif
ifdef WIN
	AssertEQ [si].cplOutPict,1,midGrbitx,200	; single plane output
	AssertGT [si].cplInPict,1,midGrbitx,300	; but color input
endif
	mov	ax,[si].cplInPict
	mov	cplIn,ax
	mov	ax,[si].cbitRepNormPict
	mov	cbitRepNorm,ax
	mov	ax,[si].wBitDeltaPict
	mov	wBitDelta,ax
	mov	ax,[si].vtExtPict.hGpoint
	mov	virtSrcMac,ax
	errnz	cbRowPat-16
	mov	bx,irw
	and	bx,7			; irw = irw % 8
	shl	bx,1
	shl	bx,1
	shl	bx,1
	shl	bx,1			; irw * 16
	mov	rgpat,bx		; save ptr to row table to use

ifdef DEBUG
	mov	ax,[si].pixExtPict.hGpoint
	mov	cbitReallySent,ax
endif
	xor	ax,ax
	mov	wRem,ax
	mov	virtSrc,ax
	mov	cx,[si].cplInPict
	mov	dx,[si].cbPlanePict	; DX = cbPlane (always)
	jmp	short MSB081
MSB080:	add	ax,dx
MSB081:	loop	MSB080
	mov	ibLastPlane,ax		; offset to last plane

	PsFromSb ds,[si].sbBitsPict,bx,dx  ; set DS to point to sbBits
	assumes ds,nothing

	mov	es,psPat	; get segment containing dither patterns
	assumes	es,nothing

	; don't make any assumptions about ax,bx,si,di at this point
	
	mov	ibitDst,8	; *** this is different from StrBltRow ***
	mov	ibitSrc,8	; (this is different from C source)

	mov	si,pbSrc	; these point into sbBits
	mov	di,pbDst


MSBNextSrc:
	mov	ax,virtSrc	; for (virtSrc = 0;
	cmp	ax,virtSrcMac	;	virtSrc < ppc->vtExt.h;
	jl	MSB100		;	++virtSrc)
	jmp	MSBCleanup
MSB100:
	inc	virtSrc
	mov	cx,cbitRepNorm	; cbitRep = ppc->cbitRepNorm;
	mov	ax,wRem
	add	ax,wBitDelta	; wRem += wBitDelta;
	mov	bx,virtSrcMac
	cmp	ax,bx		; if (wRem >= ppc->vtExt.h)
	jl	MSB110		;	{
	inc	cx		;	++cbitRep;
	sub	ax,bx		; 	wRem -= ppc->vtExt.h
MSB110:				;	}
	mov	cbitRep,cx
	mov	wRem,ax
	dec	ibitSrc		; if (ibitSrc-- == 0)
	jns	MSBLoadBits	;	{
	mov	ibitSrc,7	; 	ibitSrc = 7; (7 bits left after this)
	inc	si		;	++pbSrc;
MSBLoadBits:			;	}
	mov	cx,cplIn	; ipl = cplIn;
	mov	bx,ibLastPlane	; ibPlane = ibLastPlane;
	xor	ax,ax		; ico = 0;
MSBLoadLoop:			; while (ipl > 0)
	rol	byte ptr ds:[si+bx],1 ;	{
	rcl	ax,1		; 	LSB of ico = MSB of *(pbSrc+ibPlane);
	sub	bx,dx		; 	ibPlane -= cbPlane;
	loop	MSBLoadLoop	; 	}

	dec	cbitRep		; if (cbitRep-- > 0)
	js	MSBNextSrc	; {

	mov	bx,rgpat
	xlat	es:[bx]		; pattern byte = rgpat[ico];
	mov	cx,ibitDst
	ror	al,cl		; align pattern so MSB is first bit used

MSBNextDst:			; do {

ifdef DEBUG
	dec	cbitReallySent
endif
	rol	al,1		; shift pattern into carry
	rcl	byte ptr ds:[di],1 ; shift carry into LSB of *pbDst
	loop	MSB200		; if (--ibitDst == 0) {
	inc	di		;     ++pbDst;
	mov	cl,8		;     ibitDst = 8;
MSB200:				;     }
	dec	cbitRep		; } while (cbitRep-- > 0);
	jns	MSBNextDst
	mov	byte ptr ibitDst,cl
	jmp	MSBNextSrc
	
MSBCleanup:
	mov	cl,byte ptr ibitDst
	shl	byte ptr ds:[di],cl	; shift over remaining bits
	mov	cl,byte ptr ibitSrc
	mov	ax,cplIn
MSBCleanLoop:
	rol	byte ptr ds:[si],cl	; clean up last source byte
	add	si,dx			; note that we trash SI
	dec	ax
	jnz	MSBCleanLoop
	
MSBRet:	
ifdef PCWORD
	AssertEQ cbitReallySent,0
endif
ifdef WIN
	AssertEQ cbitReallySent,0,midGrbitx,100
endif

cEnd	MonoStrBltRow




ifdef NEVER

; This is not being used because the case in which an expansion
; or compression does not occur is extremely rare.

;-------------------------------------------------------------------------
; ColorTransRow(ppc, pbSrc)
; PICT	*ppc;
; char	*pbSrc;
;
; Translates the colors of a row in a bitmap.
;
; The basic structure is as follows:
;
; For each byte in the row
;	Get the most significant bit
;	Translate it
;	For each of the remaining 7 bits
;		Shift in the translated color and shift out the next bit
;		Translate the color
;	Now we have the last bit, translated
;	Stuff it into the byte
;	Prepare for next byte
;
;
; Note that we may process more bits than necessary with this method,
; since the main loop works on a byte-by-byte basis.
;
; WARNING: DS is changed for the duration of this routine.
;-------------------------------------------------------------------------


cProc	ColorTransRow,<FAR,PUBLIC>,<si,di,ds>

ParmW	<ppc>				; PICT *ppc;
ParmW	<pbSrc>				; char *pbSrc, *pbDst;

LocalW	<cplOut, cplIn>		 	; these are taken right from ppc
LocalW	<rgico>				; and put here for faster access
LocalW	<virtSrcMac>			; (ppc->vtExt.h)

LocalW	<cbLeft>			; # of bytes remaining
LocalW	<cpl>				; max(cplOut, cplIn)
LocalW	<cbitCoShift>			; 8 - max(cplIn, cplOut)
LocalW	<coMask>			; mask to AND src color with

;
;    %%Function:  ColorTransRow    %%Owner:  bobz
;

cBegin	ColorTransRow

	mov	si,ppc			; copy stuff over from PICT
	mov	cx,[si].cplInPict
	mov	cplIn,cx
	mov	ax,1
	shl	ax,cl
	dec	ax			; form input color mask
	mov	coMask,ax
	mov	ax,[si].cplOutPict
	mov	cplOut,ax
	sub	ax,cx			; this is the fancy max(ax,cx)
	cwd
	not	dx
	and	ax,dx
	add	ax,cx
	mov	cpl,ax			; cpl = max(cplIn, cplOut);
	lea	bx,[si].rgicoPict
	mov	ax,[si].vtExtPict.hGpoint
	mov	virtSrcMac,ax
	mov	ax,[si].cbRowInPict	; (for cbLeft = cbRowIn;
	mov	cbLeft,ax		;     cbLeft > 0; --cbLeft)

	mov	dx,[si].cbPlanePict	; DX = cbPlane (always)

	PsFromSb ds,[si].sbBitsPict,di,bx,dx ; set DS to point to sbBits
	assumes ds,nothing

	; don't assume anything about the contents of ax,cx,si
	
	mov	di,pbSrc

	xor	ax,ax
	mov	cx,ax

CTNextByte:
	mov	si,di
	mov	cl,byte ptr cplIn	; for each plane
	AssertEQ cx,<word ptr cplIn>
CTFirstPix:				;	{
	shl	byte ptr [si],1		; 	shift out MSB
	rcr	ah,1			; 	stack 'em up in AH
	add	si,dx
	loop	CTFirstPix		;	}

	mov	cl,byte ptr cplIn	; rotate into AL
	rol	ax,cl
	xlat	DGROUP:[bx]		; get dest color
	
	mov	ch,7			; for each remaining bit in word
CTNextPix:
	mov	cl,byte ptr cpl		; for each plane
	mov	si,di			;	{
CTFormPix:
	shr	al,1			;	this goes in the right side
	rcl	byte ptr [si],1	 	;	...as we shift out
	rcr	ah,1			;	...the next bit into AH
	add	si,dx
	dec	cl
	jnz	CTFormPix		;	}
	mov	cl,byte ptr cpl
	rol	ax,cl			; AL = source color
	and	al,byte ptr coMask	; strip off extra bits
	xlat	DGROUP:[bx]		; get dest color
	dec	ch
	jnz	CTNextPix

	mov	cl,byte ptr cplOut	; now stuff last bit in byte
	AssertEQ cx,<word ptr cplOut>
	mov	si,di			; for each plane
CTLastPix:				;	{
	shr	al,1			;	get translated bit
	rcl	byte ptr [si],1		;	and stuff it in
	add	si,dx			;	}
	loop	CTLastPix

	inc	di			; point to next byte
	dec	cbLeft
	jz	CTDone
	jmp	CTNextByte
CTDone:

cEnd	ColorTransRow

endif	;NEVER



sEnd	GRBIT

	END
