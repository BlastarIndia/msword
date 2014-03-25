        include w2.inc
        include noxport.inc
        include consts.inc
        include structs.inc

createSeg	fetch1_PCODE,prln,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midPrln 	equ 30		 ; module ID, for native asserts
endif

; EXTERNAL FUNCTIONS

ifdef DEBUG
externFP	<AssertProcForNative>
endif ;DEBUG


sBegin  data

; EXTERNALS

externW dnsprm		; extern struct ESPRM	  dnsprm[];
externW vchpStc 	; extern struct CHP	  vchpStc;

sEnd    data

; CODE SEGMENT _EDIT

sBegin	prln
	assumes cs,prln
        assumes ds,dgroup
        assumes ss,dgroup

VFPS_CALL label word
	dw	VFPS_spraBit
	dw	VFPS_spraByte
	dw	VFPS_spraWord
	dw	VFPS_spraCPlain
	dw	VFPS_spraCFtc
	dw	VFPS_spraCKul
	dw	VFPS_spraCSizePos
	dw	VFPS_spraSpec
	dw	VFPS_spraIco
	dw	VFPS_spraCHpsInc
	dw	VFPS_spraCHpsPosAdj

;-------------------------------------------------------------------------
;	ValFromPropSprm(prgbProps, sprm)
;-------------------------------------------------------------------------
;/* V A L  F R O M  P R O P  S P R M
;   returns value of property addressed by prgbProp and sprm as an int */
;
;NATIVE int ValFromPropSprm(prgbProps, sprm)
;char *prgbProps;
;int sprm;
;{
;	int val = -1;
;	struct ESPRM esprm;
;	struct PCVH pcvh;

; %%Function:N_ValFromPropSprm %%Owner:BRADV
PUBLIC N_ValFromPropSprm
N_ValFromPropSprm:
	mov	bx,sp
	push	si
	push	di
	mov	si,[bx+6]
	mov	di,[bx+4]

;	esprm = dnsprm[sprm];
	errnz	<cbEsprmMin - 4>
	mov	bx,di
	shl	bx,1
	shl	bx,1
	mov	cx,[bx.dnsprm]

;	switch(esprm.spra)
;		{
	errnz	<(spraEsprm) - 1>
	mov	bl,ch
	and	bx,maskspraEsprm
	errnz	<ibitSpraEsprm>
	shl	bx,1

;	Assert (spra < 11);
ifdef DEBUG
	cmp	bx,11 SHL 1
	jb	VFPS01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midPrln
	mov	bx,1001
	cCall	AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
VFPS01:
endif ;DEBUG
	errnz	<maskbEsprm - 0FEh>
	errnz	<(bEsprm) - 0>
	shr	cl,1
	xor	ch,ch
	xor	ah,ah
	call	[bx.VFPS_CALL]

;}
	pop	di
	pop	si
	db	0CAh, 004h, 000h    ;far ret, pop 4 bytes

;	case spraWord:
;		/* sprm has a word parameter that is stored at b */
;		bltb(prgbProps + esprm.b, &val, 2);
;		break;
VFPS_spraWord:
	add	si,cx
	lodsw
	ret

;	case spraByte:
;		/* sprm has a byte parameter that is stored at b */
;		val = *(prgbProps + esprm.b);
;		break;
VFPS_spraByte:
	add	si,cx
	lodsb
	ret

;	case spraBit:
;		/* sprm has byte parameter that is stored at bit b in word 0 */
;		/* WARNING: shift in following versions is machine-dependent */
;		Win(val = (*(int *)prgbProps & (1<<esprm.b)) ? 1 : 0);
;		break;
VFPS_spraBit:
	mov	al,1
	mov	dx,ax
	shl	dx,cl
	test	[si],dx
	jne	VFPS02
	dec	ax
VFPS02:
	ret

;	case spraCFtc:
;		val = ((struct CHP *)prgbProps)->ftc;
;		break;
VFPS_spraCFtc:
	mov	ax,[si.ftcChp]
	ret

;	case spraCKul:
;		val = ((struct CHP *)prgbProps)->kul;
;		break;
VFPS_spraCKul:
	mov	al,[si.kulChp]
	and	al,maskKulChp
	mov	cl,ibitKulChp
	shr	al,cl
	ret

;	case spraCPlain:
;		/* should have done a CachePara prior to this */
;		val = !FNeRgw(&vchpStc, prgbProps, cwCHP);
;		break;
VFPS_spraCPlain:
	push	ds
	pop	es
	mov	di,dataoffset [vchpStc]
	errnz	<cbChpMin AND 1>
	mov	cx,cbChpMin SHR 1
	errnz	<fFalse>
	xor	ax,ax
	repe	cmpsw
	jne	VFPS03
	errnz	<fTrue - fFalse - 1>
	inc	ax
VFPS03:
	ret

;	case spraCIco:
;		val = ((struct CHP *)prgbProps)->ico;
;		break;
VFPS_spraIco:
	mov	al,[si.icoChp]
	errnz	<maskIcoChp - 00Fh>
	and	al,maskIcoChp
	ret

;	default:
VFPS_spraCSizePos:
VFPS_spraSpec:
VFPS_spraCHpsInc:
VFPS_spraCHpsPosAdj:


;		switch(sprm)
;			{
ifdef DEBUG
	test	di,0FF00h
	je	VFPS04
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midPrln
	mov	bx,1002
	cCall	AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
VFPS04:
endif ;DEBUG
	xchg	ax,di

;		case sprmCQpsSpace:
;			val = ((struct CHP *)prgbProps)->qpsSpace;
;			break;
	cmp	al,sprmCQpsSpace
	jne	VFPS05
	mov	al,[si.qpsSpaceChp]
	errnz	<maskQpsSpaceChp - 03fh>
	and	al,maskQpsSpaceChp
	ret
VFPS05:

;		case sprmPStc:
;			val = ((struct PAP *)prgbProps)->stc;
;			break;
	cmp	al,sprmPStc
	jne	VFPS06
	errnz	<(stcPap) - 0>
	lodsb
	ret
VFPS06:

;		case sprmTDxaGapHalf:
;			val = ((struct TAP *)prgbProps)->dxaGapHalf;
;			break;
	cmp	al,sprmTDxaGapHalf
	jne	VFPS07
	mov	ax,[si.dxaGapHalfTap]
	ret
VFPS07:

;		case sprmPPc:
;			pcvh.op = 0;
;			pcvh.pcVert = ((struct PAP *)prgbProps)->pcVert;
;			pcvh.pcHorz = ((struct PAP *)prgbProps)->pcHorz;
;			val = pcvh.op;
;			break;
	cmp	al,sprmPPc
	jne	VFPS08
	errnz	<maskPcVertPap - 030h>
	errnz	<maskPcHorzPap - 0C0h>
	errnz	<(pcVertPap) - (pcHorzPap)>
	errnz	<maskPcVertPcvh - 030h>
	errnz	<maskPcHorzPcvh - 0C0h>
	errnz	<(pcVertPcvh) - (pcHorzPcvh)>
	mov	al,[si.pcVertPap]
	and	al,0F0h
	ret
VFPS08:

;		case sprmCDefault:
;			val = (*(int *)prgbProps & 0xff10) == 0 &&
;			      ((struct CHP *)prgbProps)->kul == kulNone &&
;			      ((struct CHP *)prgbProps)->ico == icoAuto;
;			break;
	cmp	al,sprmCDefault
	jne	VFPS10
	mov	al,fFalse
	test	wptr [si], 0FF10h
	jne	VFPS09
	errnz	<kulNone - 0>
	test	[si.kulChp],maskKulChp
	jne	VFPS09
	errnz	<icoAuto - 0>
	test	[si.icoChp],maskIcoChp
	jne	VFPS09
	errnz	<fTrue - fFalse - 1>
	inc	ax
VFPS09:
	ret
VFPS10:

;			}
;		break;
;		}
	;Assembler note: this is val = -1; taken from the frame declaration
	mov	ax,-1
	ret

;	return val;
;}
	;Assembler note: the return is performed above

; End of ValFromPropSprm


sEnd	prln
        end
