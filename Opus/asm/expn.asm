        include w2.inc
        include noxport.inc
        include consts.inc
        include structs.inc

createSeg	exp_PCODE,exp,byte,public,CODE

; DEBUGGING DECLARATIONS

; EXTERNAL FUNCTIONS

externFP	<CbAllocSb, CbReallocSb, FreeSb, ReloadSb, CbSizeSb>

sBegin  data

; EXTERNALS

externW mpsbps
externW sbMac


sEnd    data

; CODE SEGMENT _EXP

sBegin	exp
	assumes cs,exp
        assumes ds,dgroup
        assumes ss,dgroup


;-------------------------------------------------------------------------
;	LPushMacroArgs (qProc, rgw, cw)
;-------------------------------------------------------------------------

; %%Function:LPushMacroArgs %%Owner:bradch
cProc	LPushMacroArgs,<PUBLIC,FAR>,<si,di>
	ParmD	<qProc>
	ParmW	<rgw>
	ParmW	<cw>

cBegin
	mov	di,sp
	mov	si,[rgw]
	mov	cx,[cw]
	jcxz	PMA04
PMA03:
	lodsw
	push	ax
	loop	PMA03
PMA04:
	call	dword ptr ([qProc])
	mov	sp,di
cEnd



;-------------------------------------------------------------------------
;	MemUsed (memType)
;
;	   memType = 1 for conventional, 2 for expanded, 3 for both
;
;	Taken directly from Excel's memutil.asm.   8/12/88 TDK.
;-------------------------------------------------------------------------

; %%Function:MemUsed %%Owner:bradch
cProc	MemUsed,<PUBLIC,FAR>,<si,di>
	ParmW	memType
cBegin
	xor	si,si
	xor	di,di
	mov	bx,1
	dec	bx
mf1a:
	inc	bx
	cmp	bx,[sbMac]
	jae	mfx
	shl	bx,1
	mov	cx,[bx+mpsbps]
	shr	bx,1
	jcxz	mf1a
	push	bx
	cCall	CbSizeSb,<bx>
	pop	bx
;
;  Determine if memory is conventional or expanded
;
	mov	cl,00000001b		; assume conventional
	or	dx,dx			; no Emm, must be conventional
	jz	mf1c
	mov	cl,00000010b		; mark as expanded
mf1c:
	test	byte ptr (memType),cl
	jz	mf1a
	add	si,ax
	adc	di,0
	jmp	mf1a
mfx:
	mov	ax,si
	mov	dx,di
cEnd

sEnd	exp

end

