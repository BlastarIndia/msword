        include w2.inc
        include noxport.inc
        include consts.inc
        include structs.inc

createSeg	index2_PCODE,index2,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midIndex2n	equ 34		; module ID, for native asserts
endif

; EXTERNAL FUNCTIONS

externFP	<N_WCompSzSrt>
externFP	<ReloadSb>

ifdef DEBUG
externFP	<AssertProcForNative>
externFP	<S_WCompSzSrt>
endif ;DEBUG

sBegin  data

; EXTERNALS

externW 	mpsbps

; #ifdef DEBUG
ifdef DEBUG
externW 	vcComps
externW 	wFillBlock
; #endif /* DEBUG */
endif

sEnd    data

; CODE SEGMENT _INDEX2

sBegin	index2
	assumes cs,index2
        assumes ds,dgroup
        assumes ss,dgroup



;-------------------------------------------------------------------------
;	WCompRgchIndex(hpch1, cch1, hpch2, cch2)
;-------------------------------------------------------------------------
;/* W  C O M P	R G C H  I N D E X */
;/*  Routine to compare RGCH1 and  RGCH2
;	 Return:
;		 0 if rgch1 = rgch2
;		 negative if rgch1 < rgch2
;		 positive if rgch1 > rgch2
;	 This routine is case-insensitive
;	 soft hyphens are ignored in the comparison
;*/
; %%Function:WCompRgchIndex %%Owner:BRADV
;HANDNATIVE int WCompRgchIndex(hpch1, cch1, hpch2, cch2)
;char	 HUGE *hpch1, HUGE *hpch2;
;int	 cch1, cch2;
;{
;	char	*pch;
;	char	HUGE *hpchLim;
;	char	sz1[cchMaxEntry+2];
;	char	sz2[cchMaxEntry+2];

cProc	N_WCompRgchIndex,<PUBLIC,FAR,ATOMIC>,<si,di>
	ParmD	hpch1
	OFFBP_hpch1		=   -4
	ParmW	cch1
	OFFBP_cch1		=   -6
	ParmD	hpch2
	OFFBP_hpch2		=   -10
	ParmW	cch2
	OFFBP_cch2		=   -12

	LocalV	sz1,<cchMaxEntry+2>
	LocalV	sz2,<cchMaxEntry+2>

cBegin

;	Debug(++vcComps);
ifdef DEBUG
	inc	[vcComps]
endif ;DEBUG

;	Assert(cch1 < cchMaxEntry);
;	for (pch = sz1, hpchLim = hpch1 + cch1;
;		hpch1 < hpchLim; hpch1++)
;		{
;		if (*hpch1 != chNonReqHyphen)
;			*pch++ = *hpch1;
;		}
;	*pch = 0;
	lea	si,[sz1]
	push	si	;argument for WCompSzSrt
	lea	di,[hpch1]
	call	WCRI01

;	Assert(cch2 < cchMaxEntry);
;	for (pch = sz2, hpchLim = hpch2 + cch2;
;		hpch2 < hpchLim; hpch2++)
;		{
;		if (*hpch2 != chNonReqHyphen)
;			*pch++ = *hpch2;
;		}
;	*pch = 0;
	lea	si,[sz2]
	push	si	;argument for WCompSzSrt
	lea	di,[hpch2]
	call	WCRI01

;	return (WCompSzSrt(sz1, sz2, fFalse /* not case sensitive */));
	errnz	<fFalse>
	xor	ax,ax
	push	ax
ifdef DEBUG
	cCall	S_WCompSzSrt,<>
else ;not DEBUG
	cCall	N_WCompSzSrt,<>
endif ;DEBUG

;}
cEnd

WCRI01:
;	Assert(cch < cchMaxEntry);
ifdef DEBUG
	errnz	<OFFBP_hpch1 - OFFBP_cch1 - 2>
	errnz	<OFFBP_hpch2 - OFFBP_cch2 - 2>
	cmp	wptr [di+(OFFBP_cch1 - OFFBP_hpch1)],cchMaxEntry
	jb	WCRI02
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midIndex2n
	mov	bx,1001 		; label # for native assert
	cCall	AssertProcForNative, <ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
WCRI02:
endif ;DEBUG

;	for (pch = sz, hpchLim = hpch + cch;
;		hpch < hpchLim; hpch++)
;		{
;		if (*hpch != chNonReqHyphen)
;			*pch++ = *hpch;
;		}
	mov	bx,[di+2]   ;get SEG_hpch
	shl	bx,1
	mov	ax,mpsbps[bx]
	mov	es,ax
	shr	ax,1
	jc	WCRI03
;	reload sb trashes ax, cx, and dx
	cCall	ReloadSb,<>
ifdef DEBUG
	mov	ax,[wFillBlock]
	mov	bx,[wFillBlock]
	mov	cx,[wFillBlock]
	mov	dx,[wFillBlock]
endif ;DEBUG
WCRI03:
	errnz	<OFFBP_hpch1 - OFFBP_cch1 - 2>
	errnz	<OFFBP_hpch2 - OFFBP_cch2 - 2>
	mov	cx,[di+(OFFBP_cch1 - OFFBP_hpch1)]
	mov	di,[di]     ;get OFF_hpch
	mov	al,chNonReqHyphen
WCRI04:
	mov	dx,di
	repne	scasb	    ;look for chNonReqHyphen
	push	cx
	push	di
	push	es
	pop	ds
	push	ss
	pop	es
	mov	cx,di
	mov	di,si
	mov	si,dx
	jne	WCRI05	    ;if we found a chNonReqHyphen, don't copy it
	dec	cx
WCRI05:
	sub	cx,dx
	rep	movsb	    ;copy up to the chNonReqHyphen or end of rgch
	mov	si,di
	push	ds
	pop	es
	push	ss
	pop	ds
	pop	di
	pop	cx
	or	cx,cx	    ;more characters in rgch?
	jne	WCRI04	    ;yes - look for more chNonReqHyphens

;	*pch = 0;
	mov	bptr [si],0
	ret

; End of WCompRgchIndex


sEnd	index2
        end
