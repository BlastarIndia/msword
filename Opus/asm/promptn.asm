        PAGE    58,82

; REVIEW: How to make intra-module calls NEAR?
;
        include w2.inc
        include noxport.inc
        include consts.inc
        include structs.inc

createSeg       prompt_PCODE,prompt,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midPromptn	equ 9		 ; module ID, for native asserts
endif

; EXTERNAL FUNCTIONS

externFP	<AdjustPrompt>
externFP        <PopToHppr>

ifdef DEBUG
externFP	<AssertProcForNative>
endif ;DEBUG


sBegin  data

; EXTERNALS
externW                 vhpprPRPrompt


sEnd    data

; CODE SEGMENT _PROMPT

sBegin  prompt
        assumes cs,prompt
        assumes ds,dgroup
        assumes ss,dgroup


;-------------------------------------------------------------------------
;	ProgressReportPercent (hppr, lLow, lHigh, l, plNext)
;-------------------------------------------------------------------------
;/* P R O G R E S S  R E P O R T  P E R C E N T */
;native ProgressReportPercent (hppr, lLow, lHigh, l, plNext)
;struct PPR **hppr;
;long lLow, lHigh, l, *plNext;
;{
;	int	wPartitions = 100 / pppr->nIncr;
;	if (hppr == hNil) return;
;	wPartitions = 100 / pppr->nIncr;

; %%Function:ProgressReportPercent %%Owner:BRADV
cProc	ProgressReportPercent,<PUBLIC,FAR>,<si,di>
	ParmW	<hppr>
	ParmD	<lLow>
	ParmD	<lHigh>
	ParmD	<l>
	ParmW	<plNext>

	LocalW	wPartitions
	LocalW	nIncr

cBegin
	mov	ax,100
	mov	bx,hppr
	cmp	bx,0
	je	PRPTemp1
        mov     bx,[bx]    ; bx = pppr
	mov	cx,[bx.nIncrPpr]
	mov	nIncr,cx
	div	cl
	mov	wPartitions,ax

;	 Assert (100 % pppr->nIncr == 0);
ifdef DEBUG
	call	PRP08
endif ;/* DEBUG */

;    ChangeProgressReport (hppr, (l-lLow)*wPartitions
;			  +(lHigh-lLow)>>1/(lHigh-lLow)(*hppr)->nIncr);
	mov	bx,OFF_lLow
	mov	dx,SEG_lLow
	mov	cx,OFF_l
	mov	ax,SEG_l
	sub	cx,bx
	sbb	ax,dx
	;we now have (l-lLow) in ax:cx

	mov	si,OFF_lHigh
	mov	di,SEG_lHigh
	sub	si,bx
	sbb	di,dx
	;we now have (lHigh-lLow) in di:si

;	 Assert (lHigh-lLow > 0);
ifdef DEBUG
	call	PRP10
endif ;/* DEBUG */

	mul	wPartitions
	xchg	ax,cx
	mov	bx,dx
	mul	wPartitions
	add	dx,cx
	adc	bx,0
	;we now have (l-lLow)*wPartitions in bx:dx:ax

	push	di
	push	si
	shr	di,1
	rcr	si,1
	add	ax,si
	adc	dx,di
	adc	bx,0
	pop	si
	pop	di
	;we now have (l-lLow)*wPartitions+(lHigh-lLow)>>1 in bx:dx:ax

	;perform long division with shifts and adds
	mov	cx,16
PRP04:
	shl	ax,1
	rcl	dx,1
	rcl	bx,1
PRP05:
	inc	ax
	sub	dx,si
	sbb	bx,di
	jnc	PRP06
	dec	ax
	add	dx,si
	adc	bx,di
PRP06:
	loop	PRP04

	;we now have ((l-lLow)*wPartitions+(lHigh-lLow)>>1)/(lHigh-lLow) in ax
	mul	nIncr
	push	hppr
	push	ax
	call	far ptr ChangeProgressReport

;    if (plNext != NULL)
	cmp	plNext,NULL
PRPTemp1:
	je	PRP07

;	*plNext = ((lHigh-lLow)*((*hppr)->nLast/(*hppr)->nIncr+1)
;		 -(lHigh-lLow)>>1+(wPartitions-1))/wPartitions+lLow;
	mov	bx,hppr
        mov     bx,[bx]      ; bx = pppr
	mov     ax,[bx.nLastPpr]
	div	bptr (nIncr)
	inc	ax
	xchg	ax,cx
	mov	ax,di
	mul	cx
	xchg	ax,bx
	mov	ax,si
	mul	cx
	;we now have (lHigh-lLow)*(pppr->nLast/pppr->nIncr+1) in (bx+dx:ax)
	mov	cx,wPartitions
	dec	cx
	add	ax,cx
	adc	bx,dx
	shr	di,1
	rcr	si,1
	sub	ax,si
	sbb	bx,di
	;we now have (lHigh-lLow)*(pppr->nLast/pppr->nIncr+1)
	;	 -(lHigh-lLow)>>1+(wPartitions-1) in bx:ax
	inc	cx
	xchg	ax,bx
	xor	dx,dx
	div	cx
	xchg	ax,bx
	div	cx
    	;we now have ((lHigh-lLow)*(pppr->nLast/pppr->nIncr+1)
        ;        -(lHigh-lLow)>>1+(wPartitions-1))/wPartitions in bx:ax
	mov	cx,SEG_lLow
	mov	dx,OFF_lLow
	add	ax,dx
	adc	bx,cx
	;we now have the result in bx:ax
	mov	si,plNext
	mov	[si],ax
	mov	[si+2],bx
PRP07:
;}
cEnd

;	 Assert (100 % pppr->nIncr == 0);
ifdef DEBUG
PRP08:
	or	ah,ah
	je	PRP09
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midPromptn
	mov	bx,1001
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
PRP09:
	ret
endif ;/* DEBUG */

;	 Assert (lHigh-lLow > 0);
ifdef DEBUG
PRP10:
	push	si
	or	si,di
	jne	PRP11
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midPromptn
	mov	bx,1003
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
PRP11:
	pop	si
	ret
endif ;/* DEBUG */


;-------------------------------------------------------------------------
;	ChangeProgressReport (hppr, nNew)
;-------------------------------------------------------------------------
;/* C H A N G E  P R O G R E S S  R E P O R T */
;native ChangeProgressReport (hppr, nNew)
;struct PPR **hppr;
;unsigned nNew;
;{
;    struct PPR *pppr;
;    unsigned nIncr;
;    CHAR rgch [6], *pch;
;    int cch;

; %%Function:ChangeProgressReport %%Owner:BRADV
cProc	ChangeProgressReport,<PUBLIC,FAR>,<di,si>
	ParmW	<hppr>
	ParmW	<nNew>

	LocalW	nIncr
	LocalV	rgch,6

cBegin
;    if (hppr == hNil) return;
;    if (hppr != vhpprPRPrompt)
;        PopToHppr(hppr);
;
;    pppr = *hppr;
;    nIncr = pppr->nIncr;

	mov     bx,hppr
        cmp     bx, 0
	je	CPRTemp1
        cmp     bx,vhpprPRPrompt
        je      CPR07
        push    bx
        cCall   PopToHppr,<hppr>
        pop     bx

CPR07:  
        mov     si,[bx]     ; si = pppr
	mov	cx,[si.nIncrPpr]

;    Assert (pppr->cch <= 6 && pppr->cch > 0);
ifdef DEBUG
    	mov	ax,[si.cchPpr]
	cmp	ax,6
	jg	CPR01
	cmp     ax,0
	jg	CPR01b
CPR01:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midPromptn
	mov	bx,1004
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
CPR01b:
endif ;/* DEBUG */
;
;    if (nIncr != 1)
;    	nNew = (((nNew + nIncr/2) / nIncr) * nIncr);
	mov	ax,nNew
	push	cx
	shr	cx,1
	add	ax,cx
	pop	cx
	cwd
	div	cx
	mul	cx

;    if (nNew > pppr->nLast)
;    	{
	cmp	ax,[si.nLastPpr]
CPRTemp1:
	je	CPR06

;	pppr->nLast = nNew;
	mov	[si.nLastPpr],ax

;	pch = &rgch[6];
	lea	di,rgch+5

;	cch = pppr->cch;
	mov	cx,[si.cchPpr]

;	do
;	    {
	push	ss
	pop	es

;	    *(--pch) = '0' + nNew%10;
;	    nNew /= 10;
;	    cch--;
;	    } while (nNew && cch);
;
;	while (cch--)
;	    *(--pch) = ' ';

	mov 	bx,10
	mov 	dx,ax
    	; dx = nNew, cx = cch, di = pch, bx = 10
	std
    	mov 	al,'0'
CPR03:
	or 	dx,dx
	je 	CPR04
	mov 	ax,dx
    	xor 	dx,dx
	div 	bx
	add 	dx,'0'
	xchg 	ax,dx
CPR04:
	stosb
	mov 	al,' '
	loop	 CPR03
	cld
	inc 	di

;	Assert (&rgch[6] - pch == pppr->cch);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	lea	dx,rgch+6
	sub	dx,di
	cmp	dx,[si.cchPpr]
	je	CPR05
	mov	ax,midPromptn
	mov	bx,1006
	cCall	AssertProcForNative,<ax,bx>
CPR05:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

ifdef WIN23
;	AdjustPrompt (pppr->ich, pppr->cch, pppr->xp, pch);
	cCall	AdjustPrompt,<[si.ichPpr],[si.cchPpr],[si.xpPpr], di>
else
;	AdjustPrompt (pppr->ich, pppr->cch, pch);
	cCall	AdjustPrompt,<[si.ichPpr],[si.cchPpr],di>
endif 

;	}
CPR06:
;}
cEnd



sEnd    prompt
        end
