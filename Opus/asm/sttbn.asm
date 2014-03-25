        include w2.inc
        include noxport.inc
        include consts.inc
        include structs.inc

createSeg	sttb_PCODE,sttb,byte,public,CODE

; DEBUGGING DECLARATIONS

; EXTERNAL FUNCTIONS

sBegin  data

; EXTERNALS

sEnd    data

; CODE SEGMENT _STTB

sBegin	sttb
	assumes cs,sttb
        assumes ds,dgroup
        assumes ss,dgroup



;-------------------------------------------------------------------------
;	AddDcbToLprgbst(lpbstFirst, ibstMac, dcb, bstThreshold)
;-------------------------------------------------------------------------
;AddDcbToLprgbst(lpbstFirst, ibstMac, dcb, bstThreshold)
;int far *lpbstFirst;
;int ibstMac;
;int dcb;
;int bstThreshold;
;{
;	int far *lpbst = lpbstFirst;
;
;	for (ibst = 0; ibst < ibstMac; ibst++, lpbst++)
;		{
;		if (*lpbst >= bstThreshold)
;			*lpbst += dcb;
;		}
;}
; %%Function:AddDcbToLprgbst %%Owner:BRADV
cProc	AddDcbToLprgbst,<PUBLIC,FAR,ATOMIC>,<>
	ParmD	lpbstFirst
	ParmW	ibstMac
	ParmW	dcb
	ParmW	bstThreshold

cBegin
	mov	cx,[ibstMac]
	jcxz	ADTL04
	mov	dx,[dcb]
	les	bx,[lpbstFirst]
	mov	ax,[bstThreshold]
	or	ax,ax
	je	ADTL03
ADTL01:
	cmp	es:[bx],ax
	jb	ADTL02
	add	es:[bx],dx
ADTL02:
	inc	bx
	inc	bx
	loop	ADTL01
	jmp	short ADTL04
ADTL03:
	add	es:[bx],dx
	inc	bx
	inc	bx
	loop	ADTL03
ADTL04:

cEnd

; End of AddDcbToLprgbst


sEnd	sttb
        end
