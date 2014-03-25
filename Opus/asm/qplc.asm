; 07/04/89 - Put comment in here so that ownership of module can be 
;    determined since there is no usage of cproc or public in module
; %%Function:QPlc %%Owner:BRADV
;	expect pplc in bx, return HpOfHq(hqplce) in es:di, bx is not altered.
;   	ax, cx, dx trashed
	lea	di,[bx.rgcpPlc]
	push	ds
	pop	es
	test	[bx.fExternalPlc],maskFExternalPlc
	je	QPLC2
	push	bx
	mov	di,[bx.LO_hqplcePlc]
	mov	bx,[bx.HI_hqplcePlc]
	shl	bx,1
	mov	ax,mpsbps[bx]
	mov	es,ax
	shr	ax,1
	jc	QPLC1
;	reload sb trashes ax, cx, and dx
	cCall	ReloadSb,<>
QPLC1:
	mov	di,es:[di]
	pop	bx
QPLC2:
ifdef DEBUG
	mov	ax,[wFillBlock]
	mov	cx,[wFillBlock]
	mov	dx,[wFillBlock]
endif ;DEBUG
