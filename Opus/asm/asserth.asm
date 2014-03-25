ifdef DEBUG
;   	    AssertHForNative
;   	    	This is to make the general (good handle) case as fast as
;   	    	possible.  If there is any question, it calls
;   	    	AssertHSzLnForNative in debug.c

; %%Function:AssertHForNative %% Owner:BRADV
cProc	AssertHForNative,<NEAR>,<>
    	ParmW	h
	ParmW	mid
	ParmW	lbl

	LocalW	pdrfList
cBegin

;   for (pdrfList = vpdrfHead; pdrfList != NULL; pdrfList = pdrfList->pdrfNext)
	mov	bx,[vpdrfHead]
	mov	ax,[h]
	jmp	short AHFN02
AHFN01:
	mov	bx,[bx.pdrfNextDrf]
AHFN02:
	errnz	<NULL>
	or	bx,bx
	je	AHFN05

;	if (pdrfList->dr.hplcedl == h)
;	    {
	cmp	ax,[bx.drDrf.hplcedlDr]
	jne	AHFN01

;	    if (!(*((struct PLC **)h))->fExtBase)
;		break;
	xchg	ax,bx
	mov	bx,[bx]
	test	[bx.fExtBasePlc],maskFExtBasePlc
	je	AHFN05

;	    Assert (pdrfList->dr.hplcedl == &pdrfList->pplcedl);
	xchg	ax,bx
	lea	ax,[bx.pplcedlDrf]
	cmp	ax,[bx.drDrf.hplcedlDr]
	je	AHFN03
	push	ax
	push	bx
	cCall	AssertProcForNative,<[mid], [lbl]>
	pop	bx
	pop	ax
AHFN03:

;	    Assert (pdrfList->pplcedl == &pdrfList->dr.plcedl);
	lea	ax,[bx.drDrf.plcedlDr]
	cmp	ax,[bx.pplcedlDrf]
	je	AHFN04
	push	ax
	push	bx
	cCall	AssertProcForNative,<[mid], [lbl]>
	pop	bx
	pop	ax
AHFN04:

;	    return;
	jmp	short AHFN06

;	    }
AHFN05:

;if (!FCheckHandle (sbDds, h))
;	{
;	if (!(h == vsccAbove.hplcedl || *h == &vsccAbove))
;		AssertProc(szFile, line );
;	}
    	mov	ax,1 	    	; sbDds
	cCall	FCheckHandle,<ax,[h]>
	or  	ax,ax
	jne	AHFN06
	mov	bx,[h]
	cmp	bx,[vsccAbove.rgdrWwd.hplcedlDr]
	je	AHFN06
	mov	bx,[bx]
	cmp	bx,dataoffset [vsccAbove]
	je	AHFN06
	cCall	AssertProcForNative,<[mid], [lbl]>
AHFN06:
cEnd
endif ;DEBUG

