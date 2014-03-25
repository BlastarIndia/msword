        include w2.inc
        include noxport.inc
        include consts.inc
        include structs.inc

createSeg	clsplc_PCODE,clsplcn,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midClsplcn	equ 26		; module ID, for native asserts
endif

; EXTERNAL FUNCTIONS

externFP	<FChngSizePhqLcb>
externFP	<ReloadSb>
externFP	<N_QcpQfooPlcfoo>
externFP	<AdjustHplcCpsToLim>

ifdef DEBUG
externFP	<AssertProcForNative>
externFP    	<FCheckHandle>
externFP	<S_FOpenPlc>
externFP	<S_FStretchPlc>
externFP	<S_ShrinkPlc>
endif


sBegin  data

; 
; /* E X T E R N A L S */
; 
externW mpsbps		; extern SB	      mpsbps[];
externW vmerr           ; extern struct MERR  vmerr;
externW vfUrgentAlloc	; extern int	      vfUrgentAlloc;
externW vfInCommit	; extern int	      vfInCommit;

ifdef DEBUG
externW wFillBlock
externW vsccAbove
externW vpdrfHead
externW vpdrfHeadUnused
endif ;/* DEBUG */

sEnd    data


; CODE SEGMENT _clsplcn

sBegin	clsplcn
	assumes cs,clsplcn
        assumes ds,dgroup
        assumes ss,dgroup

include asserth.asm

;-------------------------------------------------------------------------
;	FInsertInPlc(hplc, i, cp, pch)
;-------------------------------------------------------------------------
;/* F  I N S E R T  I N  P L C */
;/* open space in PLC and copy passed cp to plc.rgcp[i], and move structure
;   foo pointed to by pch to the ith entry in the range of foos. */
;EXPORT FInsertInPlc(hplc, i, cp, pch)
;struct  PLC **hplc;
;int i;
;CP cp;
;char *pch;
;{
; %%Function:N_FInsertInPlc %%Owner:BRADV
cProc	N_FInsertInPlc,<PUBLIC,FAR>,<si,di>
	ParmW	hplc
	ParmW	iArg
	ParmD	cpArg
	ParmW	pch

cBegin

;	Assert(hplc);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov ax, midClsplcn
	mov bx, 1001		       ; label # for native assert
	cCall	AssertHForNative,<[hplc], ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;	if (!FOpenPlc(hplc, i, 1))
;		return fFalse;
	mov	bx,[hplc]
	mov	si,[iArg]
	mov	ax,1
	push	bx  ;save hplc
	push	bx
	push	si
	push	ax
ifdef DEBUG
	cCall	S_FOpenPlc,<>
else ;not DEBUG
	push	cs
	call	near ptr N_FOpenPlc
endif ;DEBUG
	pop	bx  ;restore hplc
	or	ax,ax
	je	FIIP03

;	Assert((*hplc)->cb == 0 || pch != NULL);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	bx,[hplc]
	mov	bx,[bx]
	cmp	[bx.cbPlc],0
	je	FIIP01
	cmp	[pch],NULL
	jne	FIIP01
	mov	ax,midClsplcn
	mov	bx,1022
	cCall	AssertProcForNative,<ax,bx>
FIIP01:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;	if ((*hplc)->cb) PutPlc(hplc, i, pch);
;	PutCpPlc(hplc, i, cp);
;	QcpQfooPlcfoo takes hplcfoo in bx, ifoo in si, it returns pplcfoo
;	in bx, cbfoo in cx, qcp in es:di, qfoo in es:si.
;	if DEBUG it returns hpcp in dx:di, hpfoo in dx:si.
;	Changes ax, bx, cx, dx, si, di.
ifdef DEBUG
	xor	di,di	;Not O.K. to pass ifldMac
endif ;DEBUG
	cCall	N_QcpQfooPlcfoo,<>
ifdef DEBUG
	;Check that es == mpsbps[dx];
	call	FIIP04
endif ;DEBUG
	;***Begin in-line PutCpPlc
	mov	dx,[iArg]
	cmp	dx,[bx.icpAdjustPlc]
	mov	ax,[OFF_cpArg]
	mov	dx,[SEG_cpArg]
	jl	FIIP02
	sub	ax,[bx.LO_dcpAdjustPlc]
	sbb	dx,[bx.HI_dcpAdjustPlc]
FIIP02:
	mov	es:[di],ax
	mov	es:[di+2],dx
	;***End in-line PutCpPlc
	;***Begin in-line PutPlc
	mov	di,si
	mov	si,[pch]
	rep	movsb
	;***End in-line PutPlc

;	return fTrue;
	mov	ax,fTrue
FIIP03:

cEnd

ifdef DEBUG
FIIP04:
	push	ax
	push	bx
	push	cx
	push	dx
	push	es	;save es from QcpQfooPlcfoo
	mov	bx,dx
	shl	bx,1
	mov	ax,mpsbps[bx]
	mov	es,ax
	shr	ax,1
	jc	FIIP05
	;Assembler note: There is no way we should have to call ReloadSb here.
;	reload sb trashes ax, cx, and dx
;	cCall	ReloadSb,<>
	mov	ax,midClsplcn
	mov	bx,1002
	cCall	AssertProcForNative,<ax,bx>
FIIP05:
	pop	ax	;restore es from QcpQfooPlcfoo
	mov	bx,es	;compare with es rederived from the SB of QcpQfooPlcfoo
	cmp	ax,bx
	je	FIIP06
	mov	ax,midClsplcn
	mov	bx,1003
	cCall	AssertProcForNative,<ax,bx>
FIIP06:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

; End of FInsertInPlc


;-------------------------------------------------------------------------
;	FOpenPlc(hplc, i, di)
;-------------------------------------------------------------------------
;/* F  O P E N	P L C */
;/* when di > 0, FOpenPlc inserts di empty entries at position i in the PLC.
;   when di < 0, FOpenPlc deletes the -di entries beginning at position i
;	 in the PLC. */
;NATIVE FOpenPlc(hplc, i, di)
;struct PLC **hplc;
;int i;
;int di;
;{
;	struct PLC *pplc;
;	int cpPlc;
;	int iMacOld;
;	int iMaxNew;
;	int cbPlc;

Ltemp002:
	jmp	FOP08

; %%Function:N_FOpenPlc %%Owner:BRADV
cProc	N_FOpenPlc,<PUBLIC,FAR>,<si,di>
	ParmW	hplc
	ParmW	iArg
	ParmW	diArg

cBegin

;/* if no change requested, there's nothing to do. return. */
;	if (di == 0)
;		return(fTrue);
	mov	di,[diArg]
	or	di,di
	jz	Ltemp002

;/* must unwind the fence up to pplc->rgcp[i], because caller is signaling
;   intention to alter plc beginning with ith entry (even when di == 0). */
;	pplc = *hplc;
;	if (pplc->icpAdjust < i)
;		AdjustHplcCpsToLim(hplc, i);
	mov	si,[hplc]
	mov	bx,[si]
	mov	ax,[iArg]
	cmp	[bx.icpAdjustPlc],ax
	jge	FOP01
	cCall	AdjustHplcCpsToLim,<si,ax>
FOP01:

;	Assert(i <= pplc->iMac);
ifdef DEBUG
;	/* Assert (i < pplc->iMac) with a call so as not to mess up
;	short jumps */
	call	FOP10
endif ;/* DEBUG */

;	cbPlc = pplc->cb;
;	iMacOld = pplc->iMac;
	;Assembler note: get these values when they are needed.

;	if (di > 0)
;		{
	or	di,di
	jle	FOP03

;		/* we are expanding the plc */
;		if (!FStretchPlc(hplc, di))
;			return fFalse;
	;LN_FStretchPlc takes hplc in si and di in di.
	;The result is returned in ax.	ax, bx, cx, dx, di are altered.
	call	LN_FStretchPlc
	or	ax,ax
	je	Ltemp001

;		pplc = *hplc;
;	QcpQfooPlcfoo takes hplcfoo in bx, ifoo in si, it returns pplcfoo
;	in bx, cbfoo in cx, qcp in es:di, qfoo in es:si.
;	if DEBUG it returns hpcp in dx:di, hpfoo in dx:si.
;	Changes ax, bx, cx, dx, si, di.
	mov	bx,si
	mov	si,[bx]
	mov	si,[si.iMacPlcStr]
ifdef DEBUG
	mov	di,1	;O.K. to pass iMac
endif ;DEBUG
	cCall	N_QcpQfooPlcfoo,<>
ifdef DEBUG
	;Check that es == mpsbps[dx];
	call	FOP12
endif ;DEBUG

;/* move rgfoo entries */
;		BltInPlc(bpmFoo, hplc, i, 0, di, iMacOld - i);
	;***Begin in-line BltInPlc
	push	di	;save pcp
	mov	ax,[bx.iMacPlcStr]
	push	es
	pop	ds
	std
	sub	ax,[iArg]
	push	ax	;save iMacOld - i
	mul	cx
	xchg	ax,cx
	dec	si
	dec	si	;adjust for post-decrement
	mov	di,si
	mov	dx,[diArg]
	mul	dx
	add	di,ax
ifdef DEBUG
;	/* Assert (!(cx & 1)) with a call so as not to mess up
;	short jumps */
	call	FOP15
endif ;/* DEBUG */
	shr	cx,1
	rep	movsw
	pop	cx	;restore iMacOld - i
	pop	di	;restore pcp
	;***End in-line BltInPlc

;/* move rgcp entries */
;		BltInPlc(bpmCp, hplc, i, di, 0, iMacOld - i + 1);
	;***Begin in-line BltInPlc
	;have iMacOld - i in cx
	inc	cx
	shl	cx,1
	inc	di
	inc	di	;adjust for post-decrement and position after iMac
	mov	si,di
	mov	ax,[diArg]
	mov	dx,ax	;save di for later
	shl	ax,1
	shl	ax,1
	add	di,ax
	rep	movsw
	;***End in-line BltInPlc
	cld
	push	ss
	pop	ds

;		pplc->iMac = iMacOld + di;
	add	[bx.iMacPlcStr],dx

;		if (pplc->icpAdjust < i)
;			pplc->icpAdjust = i;
	mov	ax,[iArg]
	cmp	[bx.icpAdjustPlc],ax
	jge	FOP02
	mov	[bx.icpAdjustPlc],ax
FOP02:

;		pplc->icpAdjust += di;
	add	[bx.icpAdjustPlc],dx

;		}
	jmp	short FOP08
Ltemp001:
	jmp	short FOP09

;	else if (di < 0)
;		{
	;Assembler note: No need to test for di < 0 here.  We know it is.
FOP03:

;		/* in this case we will be removing di entries */
;		di = -di;
	neg	di

;		Assert(i + di <= iMacOld);
ifdef DEBUG
;	/* Assert (i + di <= iMacOld) with a call so as not to mess up
;	short jumps */
	call	FOP17
endif ;/* DEBUG */

;		iMaxNew = pplc->iMax;
;		/* if the Mac is less than half of the Max, we will later
;		   shift rgcp and foo entries in the PLC and reduce the
;		   size of the allocation */
;		if ((iMacOld - di) * 2 <= iMaxNew)
;			iMaxNew = iMacOld - di;
	mov	bx,[si]
	mov	dx,[bx.iMaxPlc]
	mov	ax,[bx.iMacPlcStr]
	sub	ax,di
	shl	ax,1
	cmp	ax,dx
	ja	FOP04
	shr	ax,1
	xchg	ax,dx
FOP04:

;		if (pplc->icpAdjust > i + di)
;			pplc->icpAdjust -= di;
;		else if (pplc->icpAdjust > i)
;			pplc->icpAdjust = i;
	mov	ax,[iArg]
	mov	cx,ax
	add	cx,di
	cmp	[bx.icpAdjustPlc],cx
	jle	FOP05
	sub	[bx.icpAdjustPlc],di
	jmp	short FOP06
FOP05:
	cmp	[bx.icpAdjustPlc],ax
	jle	FOP06
	mov	[bx.icpAdjustPlc],ax
FOP06:

;		ShrinkPlc(hplc, iMaxNew, i, di);
	push	si
	push	dx
	push	ax
	push	di
ifdef DEBUG
	cCall	S_ShrinkPlc,<>
else ;not DEBUG
	push	cs
	call	near ptr N_ShrinkPlc
endif ;DEBUG

;/* blow the hint for the binary search. */
;		pplc = *hplc;
;		if (pplc->icpHint >= pplc->iMac)
;			pplc->icpHint = 0;
	mov	bx,[si]
	mov	ax,[bx.iMacPlcStr]
	cmp	[bx.icpHintPlc],ax
	jl	FOP07
	mov	[bx.icpHintPlc],0
FOP07:

;		}
;	return (fTrue);
FOP08:
	mov	ax,fTrue
FOP09:
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	bx,[hplc]
	mov	bx,[bx]
	mov	ax,[bx.iMacPlcStr]
	cmp	ax,[bx.iMaxPlc]
	jb	FOP095
	mov	ax,midClsplcn
	mov	bx,1020
	cCall	AssertProcForNative,<ax,bx>
FOP095:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;}
cEnd

ifdef DEBUG
;	Assert(i <= pplc->iMac);
FOP10:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[iArg]
	mov	bx,[hplc]
	mov	bx,[bx]
	cmp	ax,[bx.iMacPlcStr]
	jle	FOP11
	mov	ax,midClsplcn
	mov	bx,1004
	cCall	AssertProcForNative,<ax,bx>
FOP11:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

ifdef DEBUG
FOP12:
	push	ax
	push	bx
	push	cx
	push	dx
	push	es	;save es from QcpQfooPlcfoo
	mov	bx,dx
	shl	bx,1
	mov	ax,mpsbps[bx]
	mov	es,ax
	shr	ax,1
	jc	FOP13
	;Assembler note: There is no way we should have to call ReloadSb here.
;	reload sb trashes ax, cx, and dx
;	cCall	ReloadSb,<>
	mov	ax,midClsplcn
	mov	bx,1005
	cCall	AssertProcForNative,<ax,bx>
FOP13:
	pop	ax	;restore es from QcpQfooPlcfoo
	mov	bx,es	;compare with es rederived from the SB of QcpQfooPlcfoo
	cmp	ax,bx
	je	FOP14
	mov	ax,midClsplcn
	mov	bx,1006
	cCall	AssertProcForNative,<ax,bx>
FOP14:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

ifdef DEBUG
;	Assert(!(cx & 1));
FOP15:
	test	cl,1
	je	FOP16
	push	ax
	push	bx
	push	cx
	push	dx
	push	ss
	pop	ds
	cld
	mov	ax,midClsplcn
	mov	bx,1007
	cCall	AssertProcForNative,<ax,bx>
	std
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FOP16:
	ret
endif ;/* DEBUG */

ifdef DEBUG
;		Assert(i + di <= iMacOld);
FOP17:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[iArg]
	add	ax,di
	mov	bx,[hplc]
	mov	bx,[bx]
	cmp	ax,[bx.iMacPlcStr]
	jle	FOP18
	mov	ax,midClsplcn
	mov	bx,1008
	cCall	AssertProcForNative,<ax,bx>
FOP18:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */


;-------------------------------------------------------------------------
;	ShrinkPlc(hplc, iMaxNew, i, di)
;-------------------------------------------------------------------------
;/* S H R I N K  P L C */
;/* shrink size to iMaxNew while deleting di entries starting with i */
;ShrinkPlc(hplc, iMaxNew, i, di)
;struct PLC **hplc; int iMaxNew, i, di;
;{
;	struct PLC *pplc = *hplc;
;	int iMaxOld = pplc->iMax;
;	int iMacOld = pplc->iMac;
;	int iLim = i + di;
;	int cbPlc = pplc->cb;
;	int dicp;

; %%Function:N_ShrinkPlc %%Owner:BRADV
cProc	N_ShrinkPlc,<PUBLIC,FAR>,<si,di>
	ParmW	hplc
	ParmW	iMaxNew
	ParmW	iArg
	ParmW	diArg

cBegin

;	QcpQfooPlcfoo takes hplcfoo in bx, ifoo in si, it returns pplcfoo
;	in bx, cbfoo in cx, qcp in es:di, qfoo in es:si.
;	if DEBUG it returns hpcp in dx:di, hpfoo in dx:si.
;	Changes ax, bx, cx, dx, si, di.
	mov	bx,[hplc]
	mov	si,[iArg]
	add	si,[diArg]
	push	si	;save i + di
ifdef DEBUG
	mov	di,1	;O.K. to pass iMac
endif ;DEBUG
	cCall	N_QcpQfooPlcfoo,<>
ifdef DEBUG
	;Check that es == mpsbps[dx];
	call	SP03
endif ;DEBUG
	pop	ax	;restore i + di

;	pplc->iMac = iMacOld - di;
	;Assembler note: This is done later in the assembler version.

;	/* shift down rgcp[j] for j >= i + di */
;	BltInPlc(bpmCp, hplc, iLim, -di, 0, (iMacOld + 1) - iLim);
	;***Begin in-line BltInPlc
	mov	cx,[bx.iMacPlcStr]
	push	es
	pop	ds
	push	si	;save pfoo
	mov	si,di	;&rgcp[i+di]
	mov	dx,[diArg]
	shl	dx,1
	shl	dx,1
	sub	di,dx	;&rgcp[i]
	inc	cx
	sub	cx,ax
	shl	cx,1
	rep	movsw
	pop	si	;restore pfoo
	;***End in-line BltInPlc

;	/* shift down rgfoo[j] for 0 <=j < i to cover any rgcp entries
;	   that must be reclaimed. */
;	Assert(iMaxNew <= iMaxOld);
;	iMaxNew = min(iMaxNew + 5, iMaxOld); /* leave some room */
;	dicp = iMaxOld - iMaxNew;
;	if (dicp && i > 0)
;		BltInPlc(bpmFoo, hplc, 0, -dicp, 0, i);
	xor	dx,dx	    ;default no movement due to iMax change
	mov	cx,[iMaxNew]
	add	cx,5
	sub	cx,ss:[bx.iMaxPlc]
	jge	SP01
	push	si	;save pfoo
	add	ss:[bx.iMaxPlc],cx
	;***Begin in-line BltInPlc
	mul	ss:[bx.cbPlc]	;already have i+di in ax
	sub	si,ax	    ;&rgcp[iMaxNew]
	mov	di,si
	mov	ax,[iArg]
	mul	ss:[bx.cbPlc]
	mov	dx,cx
	shl	dx,1
	shl	dx,1
	add	di,dx	    ;&rgcp[iMaxOld]
	xchg	ax,cx
ifdef DEBUG
;	/* Assert (!(cx & 1)) with a call so as not to mess up
;	short jumps */
	call	SP06
endif ;/* DEBUG */
	shr	cx,1
	rep	movsw
	;***End in-line BltInPlc
	pop	si	;restore pfoo
SP01:
	push	dx	;save diMax << 2

	;Assembler note: the following line is done above in the C version.
;	pplc->iMac = iMacOld - di;
	mov	ax,[diArg]
	sub	ss:[bx.iMacPlcStr],ax

;	/* shift down rgfoo[i] for j >= i + di to cover space reclaimed
;	   from deleted rgfoos */
;	BltInPlc(bpmFoo, hplc, iLim, -dicp, -di, iMacOld - iLim);
	;***Begin in-line BltInPlc
	mov	di,si
	add	di,dx	    ;adjust for iMax change
	mov	cx,ss:[bx.cbPlc]
	mul	cx
	sub	di,ax	    ;adjust for di foo's
	;Assembler note: At this point pplc->iMac is iMacOld - di, and we
	;want iMacOld - iLim = iMacOld - (i + di) = (iMacOld - di) - i
	mov	ax,ss:[bx.iMacPlcStr]
	sub	ax,[iArg]
	mul	cx
	xchg	ax,cx
ifdef DEBUG
;	/* Assert (!(cx & 1)) with a call so as not to mess up
;	short jumps */
	call	SP06
endif ;/* DEBUG */
	shr	cx,1
	rep	movsw
	;***End in-line BltInPlc
	push	ss
	pop	ds

;	pplc->iMax = iMaxNew;
;	if (iMaxNew < iMaxOld && !vfInCommit)
;		{
;		if (!pplc->fExternal)
;			FChngSizeHCw(hplc, CwFromCch(cbPLCBase + (iMaxNew-1)
;				* (sizeof(CP)+cbPlc) + sizeof(CP)), fTrue);
;		else
;			{
;			HQ hqplce = pplc->hqplce; /* WARNING: heap may move */
;			FChngSizePhqLcb(&hqplce, (((long)iMaxNew - 1) *
;				(sizeof(CP) + cbPlc) + sizeof(CP)));
;			(*hplc)->hqplce = hqplce;
;			}
;		}
;}
	pop	cx	;restore diMax << 2
	jcxz	SP025
	mov	si,[hplc]
	xor	di,di	;don't let LN_FSetSizeByDiMax alter pplc->iMax
	;LN_FSetSizeByDiMax takes hplc in si, diMax in di and returns
	;carry set iff the size of the hplc could be altered by diMax.
	;(*hplc)->iMax is adjusted by this routine also.
	call	LN_FSetSizeByDiMax
ifdef DEBUG
	jc	SP02
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midClsplcn
	mov	bx,1009
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
SP02:
endif ;/* DEBUG */
SP025:
cEnd

ifdef DEBUG
SP03:
	push	ax
	push	bx
	push	cx
	push	dx
	push	es	;save es from QcpQfooPlcfoo
	mov	bx,dx
	shl	bx,1
	mov	ax,mpsbps[bx]
	mov	es,ax
	shr	ax,1
	jc	SP04
	;Assembler note: There is no way we should have to call ReloadSb here.
;	reload sb trashes ax, cx, and dx
;	cCall	ReloadSb,<>
	mov	ax,midClsplcn
	mov	bx,1010
	cCall	AssertProcForNative,<ax,bx>
SP04:
	pop	ax	;restore es from QcpQfooPlcfoo
	mov	bx,es	;compare with es rederived from the SB of QcpQfooPlcfoo
	cmp	ax,bx
	je	SP05
	mov	ax,midClsplcn
	mov	bx,1011
	cCall	AssertProcForNative,<ax,bx>
SP05:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

ifdef DEBUG
;	Assert(!(cx & 1));
SP06:
	test	cl,1
	je	SP07
	push	ax
	push	bx
	push	cx
	push	dx
	push	ss
	pop	ds
	mov	ax,midClsplcn
	mov	bx,1012
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
SP07:
	ret
endif ;/* DEBUG */


;-------------------------------------------------------------------------
;	FStretchPlc(hplc, di)
;-------------------------------------------------------------------------
;/* F  S T R E T C H  P L C */
;/* Assure room in hplc for di more entries.  Changes iMax if necessary but does
;not change any other data.  Returns fFalse iff no room.
;*/
;EXPORT FStretchPlc(hplc, di)
;struct PLC **hplc;
;int di;
;{
;	struct PLC *pplc = *hplc;
;	int diNeeded, diRequest;

; %%Function:N_FStretchPlc %%Owner:BRADV
cProc	N_FStretchPlc,<PUBLIC,FAR>,<si,di>
	ParmW	hplc
	ParmW	diArg

cBegin
	mov	di,[diArg]
	mov	si,[hplc]
	;LN_FStretchPlc takes hplc in si and di in di.
	;The result is returned in ax.	ax, bx, cx, dx, di are altered.
	call	LN_FStretchPlc
cEnd


	;LN_FStretchPlc takes hplc in si and di in di.
	;The result is returned in ax.	ax, bx, cx, dx, di are altered.
LN_FStretchPlc:
;	Assert(vfUrgentAlloc);
ifdef DEBUG
	cmp	[vfUrgentAlloc],fFalse
	jne	FSP01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midClsplcn
	mov	bx,1013
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FSP01:
endif ;DEBUG

;	AssertH(hplc);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midClsplcn
	mov	cx,1014 		  ; label # for native assert
	cCall	AssertHForNative,<si, ax, cx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;	if ((diNeeded = di - (pplc->iMax - pplc->iMac - 1)) <= 0)
;/* there is already sufficient space, do nothing */
;		return fTrue;
	mov	bx,[si]
	mov	cx,di
	sub	di,[bx.iMaxPlc]
	add	di,[bx.iMacPlcStr]
	inc	di
	mov	ax,fTrue
	jle	FSP03

;	else
;/* we need to expand beyond current max */
;		{
;		if (di == 1 && !vfInCommit &&
;			(diRequest = pplc->iMax/4) > diNeeded)
;/* if just growing by one, try to grow by a larger increment first */
;			{
	;ax = 1, bx = pplc, cx = di, si = hplc, di = diNeeded
	errnz	<fTrue - 1>
	cmp	cx,ax
	jne	FSP02
	cmp	[vfInCommit],fFalse
	jne	FSP02
	mov	dx,[bx.iMaxPlc]
	shr	dx,1
	shr	dx,1
	cmp	dx,di
	jle	FSP02

;			BOOL f;
;			int matSave;
;/* we don't want to hand over swap space just to give a plc extra entries so
;   we declare that the first allocation we will try is non-urgent. */
;			vfUrgentAlloc = fFalse;
	errnz	<fTrue - fFalse - 1>
	dec	ax
	mov	[vfUrgentAlloc],ax

;			matSave = vmerr.mat;
	push	[vmerr.matMerr]

;			f = FStretchPlc2(hplc, diRequest);
	push	di	;save diNeeded
	mov	di,dx
	;LN_FStretchPlc2 takes hplc in si, di in di and returns
	;the result in ax.  ax, bx, cx, dx, di are altered.
	call	LN_FStretchPlc2
	pop	di	;restore diNeeded

;			vfUrgentAlloc = fTrue;
	errnz	<fTrue - fFalse - 1>
	inc	[vfUrgentAlloc]

;			if (f)
;			    return fTrue;
	pop	dx	;remove matSave from stack
	or	ax,ax
	jne	FSP03

;			 /* if 1st alloc failed, restore vmerr flag in case
;			    2nd try succeeds.
;			 */
;			vmerr.mat = matSave;
	mov	[vmerr.matMerr],dx

;			}
FSP02:

;		return FStretchPlc2(hplc, diNeeded);
	;LN_FStretchPlc2 takes hplc in si, di in di and returns
	;the result in ax.  ax, bx, cx, dx, di are altered.
	call	LN_FStretchPlc2

;		}
;}

FSP03:
	ret


;-------------------------------------------------------------------------
;	FStretchPlc2(hplc, di)
;-------------------------------------------------------------------------
;/* F  S T R E T C H  P L C  2 */
;/* Grows iMax by exactly di.  Returns fFalse iff no room.
;*/
;FStretchPlc2(hplc, di)
;struct PLC **hplc;
;int di;
;{
;	struct PLC *pplc = *hplc;
;	long lcb;
;	int iMaxNew;
	;LN_FStretchPlc2 takes hplc in si, di in di and returns
	;the result in ax.  ax, bx, cx, dx, di are altered.
LN_FStretchPlc2:

;/* don't let plc overflow iMac */
;	if ((iMaxNew = pplc->iMax + di) < 0)
;		{
;		SetErrorMat(matMem);
;		return fFalse;
;		}
	mov	bx,[si]
	mov	dx,di
	add	dx,[bx.iMaxPlc]
	clc
	jl	FSP201

;	if (pplc->fExternal)
;		if (vfInCommit)
;/* should already be big enough */
;			Assert(CbOfHq(pplc->hqplce) >= lcb);
;		else
;			{
;			HQ hq = pplc->hqplce;
;			if (!FChngSizePhqLcb(&hq, lcb)) /* HM */
;				return fFalse;
;			(*hplc)->hqplce = hq;
;			}
;	else
;		if (vfInCommit)
;/* should already be big enough */
;			Assert(CbOfH(hplc) >= lcb + cbPLCBase);
;		else
;			{
;/* protect against cb overflow */
;			if ((lcb += cbPLCBase) > 0x00007fff)
;				{
;				SetErrorMat(matMem);
;				return fFalse;
;				}
;			if (!FChngSizeHCw(hplc, CwFromCch((uns)lcb), fFalse))
;				return fFalse;
;			}
	;LN_FSetSizeByDiMax takes hplc in si, diMax in di and returns
	;carry set iff the size of the hplc could be altered by diMax.
	;(*hplc)->iMax is adjusted by this routine also.
	call	LN_FSetSizeByDiMax
FSP201:
	sbb	ax,ax
	je	FSP202

;	pplc = *hplc;
;/* push rgfoo tables from old pos, up by di CP's */
;	BltInPlc(bpmFoo, hplc, 0, di, 0, pplc->iMac);
;	pplc->iMax += di;
	;***Begin in-line BltInPlc
;	QcpQfooPlcfoo takes hplcfoo in bx, ifoo in si, it returns pplcfoo
;	in bx, cbfoo in cx, qcp in es:di, qfoo in es:si.
;	if DEBUG it returns hpcp in dx:di, hpfoo in dx:si.
;	Changes ax, bx, cx, dx, si, di.
	push	si	;save hplc
	push	di	;save di argument
	mov	bx,si
	mov	si,[si]
	mov	si,[si.iMacPlcStr]
ifdef DEBUG
	mov	di,1	;O.K. to pass iMac
endif ;DEBUG
	cCall	N_QcpQfooPlcfoo,<>
ifdef DEBUG
	;Check that es == mpsbps[dx];
	call	FSP203
endif ;DEBUG
	pop	ax	;restore di argument
	dec	si
	dec	si	;adjust for post-decrement
	mov	di,si
	shl	ax,1
	shl	ax,1
	sub	si,ax
	mov	ax,[bx.iMacPlcStr]
	mul	cx
	xchg	ax,cx
ifdef DEBUG
;	/* Assert (!(cx & 1)) with a call so as not to mess up
;	short jumps */
	call	FSP206
endif ;/* DEBUG */
	shr	cx,1
	push	es
	pop	ds
	std
	rep	movsw
	cld
	push	ss
	pop	ds
	;***End in-line BltInPlc
	pop	si	;restore hplc

;	return fTrue;
	mov	ax,fTrue

;}
FSP202:
	ret


ifdef DEBUG
FSP203:
	push	ax
	push	bx
	push	cx
	push	dx
	push	es	;save es from QcpQfooPlcfoo
	mov	bx,dx
	shl	bx,1
	mov	ax,mpsbps[bx]
	mov	es,ax
	shr	ax,1
	jc	FSP204
	;Assembler note: There is no way we should have to call ReloadSb here.
;	reload sb trashes ax, cx, and dx
;	cCall	ReloadSb,<>
	mov	ax,midClsplcn
	mov	bx,1015
	cCall	AssertProcForNative,<ax,bx>
FSP204:
	pop	ax	;restore es from QcpQfooPlcfoo
	mov	bx,es	;compare with es rederived from the SB of QcpQfooPlcfoo
	cmp	ax,bx
	je	FSP205
	mov	ax,midClsplcn
	mov	bx,1016
	cCall	AssertProcForNative,<ax,bx>
FSP205:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

ifdef DEBUG
;	Assert(!(cx & 1));
FSP206:
	test	cl,1
	je	FSP207
	push	ax
	push	bx
	push	cx
	push	dx
	push	ss
	pop	ds
	mov	ax,midClsplcn
	mov	bx,1021
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FSP207:
	ret
endif ;/* DEBUG */


	;LN_FSetSizeByDiMax takes hplc in si, diMax in di and returns
	;carry set iff the size of the hplc could be altered by diMax.
	;(*hplc)->iMax is adjusted by this routine also.
LN_FSetSizeByDiMax:
;	lcb = (long)(iMaxNew-1)*(pplc->cb + sizeof(CP)) + sizeof(CP);
	mov	bx,[si]
	;Assembler note: assert we have an external plc here because no
	;one creates internal PLC's anymore.
ifdef DEBUG
	test	[bx.fExternalPlc],maskFExternalPlc
	jne	FSSBIM01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midClsplcn
	mov	bx,1017
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FSSBIM01:
endif ;DEBUG
	mov	ax,[bx.cbPlc]
	add	ax,4
	mov	dx,[bx.iMaxPlc]
	add	dx,di
	dec	dx
ifdef DEBUG
	jge	FSSBIM02
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midClsplcn
	mov	bx,1018
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FSSBIM02:
endif ;DEBUG
	mul	dx
	add	ax,4
	adc	dx,0
	clc
	jnz	FSSBIM06

;	if (vfInCommit)
;		Assert(CbOfHq(pplc->hqplce) >= lcb);
;#define CbOfHq(hq)	 (*((uns HUGE *)HpOfHq(hq)-1))
	mov	dx,[bx.LO_hqplcePlc]
	mov	bx,[bx.HI_hqplcePlc]
	push	bx
	push	dx	;save hqplce in memory

ifdef DEBUG
	push	dx	;save low hqplce
endif ;DEBUG
	call	LN_ReloadSb
ifdef DEBUG
	pop	bx	;restore low hqplce
endif ;DEBUG
	cmp	[vfInCommit],fFalse
ifdef DEBUG
	jne	FSSBIM07
else ;not DEBUG
	jne	FSSBIM04
endif ;DEBUG

;	else
;		{
;		HQ hqplce = pplc->hqplce; /* WARNING: heap may move */
;		if (!FChngSizePhqLcb(&hqplce, lcb)) /* HM */
;			return fFalse;
;		(*hplc)->hqplce = hqplce;
;		}
	mov	bx,sp
	xor	cx,cx
	cCall	FChngSizePhqLcb,<bx, cx, ax>
	or	ax,ax
	je	FSSBIM05
FSSBIM04:
	mov	bx,[si]
	pop	[bx.LO_hqplcePlc]
	pop	[bx.HI_hqplcePlc]
	add	[bx.iMaxPlc],di
	stc
	db	0B8h	;turns "pop dx, pop bx" to "mov ax,immediate"
FSSBIM05:
	pop	dx
	pop	bx	;remove hqplce from stack
FSSBIM06:
	ret

ifdef DEBUG
FSSBIM07:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	bx,es:[bx]
	cmp	ax,es:[bx-2]
	push	bx
	push	ax
	jbe	FSSBIM08
	mov	ax,midClsplcn
	mov	bx,1019
	cCall	AssertProcForNative,<ax,bx>
FSSBIM08:
	pop	ax
	pop	bx
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	jmp	FSSBIM04
endif ;DEBUG


LN_ReloadSb:
	push	ax	;save lcb
	shl	bx,1
	mov	ax,mpsbps[bx]
	mov	es,ax
	shr	ax,1
	jc	RS01
;	reload sb trashes ax, cx, and dx
	cCall	ReloadSb,<>
RS01:
ifdef DEBUG
	mov	bx,[wFillBlock]
	mov	cx,[wFillBlock]
	mov	dx,[wFillBlock]
endif ;DEBUG
	pop	ax	;restore lcb
	ret


;-------------------------------------------------------------------------
;	MiscPlcLoops(hplc, iFirst, iLim, pResult, wRoutine)
;-------------------------------------------------------------------------
;HANDNATIVE C_MiscPlcLoops(hplc, iFirst, iLim, pResult, wRoutine)
;struct PLC **hplc;
;int iFirst;
;int iLim;
;char *pResult;
;int wRoutine;
;{
;	union {
;		struct PAD pad;
;		struct PGD pgd;
;		struct PHE phe;
;		struct SED sed;
;		struct FRD frd;
;		struct PCD pcd;
;	} foo;
;	int ifoo;

; %%Function:N_MiscPlcLoops %%Owner:BRADV
cProc	N_MiscPlcLoops,<PUBLIC,FAR>,<si,di>
	ParmW	hplc
	ParmW	iFirst
	ParmW	iLim
	ParmW	pResult
	ParmW	wRoutine

cBegin

;	QcpQfooPlcfoo takes hplcfoo in bx, ifoo in si, it returns pplcfoo
;	in bx, cbfoo in cx, qcp in es:di, qfoo in es:si.
;	if DEBUG it returns hpcp in dx:di, hpfoo in dx:si.
;	Changes ax, bx, cx, dx, si, di.
	mov	bx,[hplc]
	mov	si,[iFirst]
ifdef DEBUG
	mov	di,1	;O.K. to pass ifldMac
endif ;DEBUG
	cCall	N_QcpQfooPlcfoo,<>
ifdef DEBUG
	;Check that es == mpsbps[dx];
	call	MSL09
endif ;DEBUG
	xchg	ax,cx
	mov	cx,[iLim]
	sub	cx,[iFirst]
	jle	MSL08
	mov	di,[pResult]

;	if (wRoutine == 0 /* SetPlcUnk */)
;		{
	cmp	bptr ([wRoutine]),1
	jae	MSL02

;		for (ifoo = iFirst; ifoo < iLim; ifoo++)
;			{
;			GetPlc(hplc, ifoo, &foo);
;			foo.pad.fUnk = fTrue;
;			PutPlcLast(hplc, ifoo, &foo);
;			}
MSL01:
	or	es:[si.fUnkPgd],maskFUnkPgd
	add	si,ax
	loop	MSL01

;		}
	jmp	short MSL08

;	if (wRoutine == 1 /* NAutoFtn */)
;		{
MSL02:
	ja	MSL05

;		for (ifoo = iFirst; ifoo < iLim; ifoo++)
;			{
;			GetPlc(hplc, ifoo, &foo);
;			*((int *)pResult) += foo.frd.fAuto;
;			}
	mov	bx,[di]

;	Assert(hplc->cb == 2);
ifdef DEBUG
	cmp	ax,2
	je	MSL03
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midClsplcn
	mov	bx,1023
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
MSL03:
endif ;/* DEBUG */

MSL04:
	lods	wptr es:[si]
	add	bx,ax
	loop	MSL04
	mov	[di],bx
	jmp	short MSL08

;		}
;	if (wRoutine == 2 /* MarkAllReferencedFn */)
;		{
MSL05:

;		for (ifoo = iFirst; ifoo < iLim; ifoo++)
;			{
;			GetPlc(hplc, ifoo, &foo);
;			Assert(foo.pcd.fn < fnMax);
;			Assert(fnNil == 0);
;			pResult[foo.pcd.fn] = fTrue;
;			}
	xor	bx,bx
MSL06:
	mov	bl,es:[si.fnPcd]
ifdef DEBUG
	cmp	bl,fnMax
	jb	MSL07
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midClsplcn
	mov	bx,1024
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
MSL07:
endif ;/* DEBUG */
	mov	bptr [bx+di],fTrue
	add	si,ax
	loop	MSL06

;		}
MSL08:

;}
cEnd

ifdef DEBUG
MSL09:
	push	ax
	push	bx
	push	cx
	push	dx
	push	es	;save es from QcpQfooPlcfoo
	mov	bx,dx
	shl	bx,1
	mov	ax,mpsbps[bx]
	mov	es,ax
	shr	ax,1
	jc	MSL10
	;Assembler note: There is no way we should have to call ReloadSb here.
;	reload sb trashes ax, cx, and dx
;	cCall	ReloadSb,<>
	mov	ax,midClsplcn
	mov	bx,1025
	cCall	AssertProcForNative,<ax,bx>
MSL10:
	pop	ax	;restore es from QcpQfooPlcfoo
	mov	bx,es	;compare with es rederived from the SB of QcpQfooPlcfoo
	cmp	ax,bx
	je	MSL11
	mov	ax,midClsplcn
	mov	bx,1026
	cCall	AssertProcForNative,<ax,bx>
MSL11:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

; End of MiscPlcLoops


;-------------------------------------------------------------------------
;       CopyMultPlc( cFoo, hplcSrc, ifooSrc, hplcDest, ifooDest,
;		dcp, di, dc )
;-------------------------------------------------------------------------
;/* C O P Y  M U L T  P L C */
;/* this routine can copy a block of entries from one plc to another (needs
;separate base registers in 8086 architecture).
;Dcp optionally (!=0) added to the copied cp's.
;di (0 or 1) is optional displacement for cp indeces. In some plc's it is
;necessary to move cp[i + 1] with foo[i].
;dc (0 or 1) is optional extra count for cp copying. In some plc's it is
;necessary to move one more cp's than foo's.
;*/
;native CopyMultPlc(cFoo, hplcSrc, ifooSrc, hplcDest, ifooDest, dcp, di, dc)
;int cFoo;
;struct PLC **hplcSrc, **hplcDest;
;int ifooSrc, ifooDest;
;CP dcp;
;int di, dc;
;{
; %%Function:CopyMultPlc %%Owner:BRADV
cProc	CopyMultPlc,<PUBLIC,FAR>,<si,di>
	ParmW	cFoo
	ParmW	hplcSrc
	ParmW	ifooSrc
	ParmW	hplcDest
	ParmW	ifooDest
	ParmD	dcp
	ParmW	diArg
	ParmW	dcArg

	LocalW	<OFF_lprgcpSrc>
	LocalW	<OFF_lprgcpDest>

cBegin
;	AssertH( hplcSrc );
;	AssertH( hplcDest );
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midClsplcn
	mov	bx,1027
	cCall	AssertHForNative,<hplcSrc, ax, bx>
	mov	ax,midClsplcn
	mov	bx,1028
	cCall	AssertHForNative,<hplcDest, ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;	save old cFoo, ifooSrc, ifooDest for blt.
	push	[cFoo]

;	ifooSrc += di; ifooDest += di;
	;Assembler note: add diArg to the ifoos only when we need them.
	mov	di,[diArg]

;	cFoo += dc;
	mov	ax,[dcArg]
	add	[cFoo],ax

;/* unwind the fenced PLC optimization, so that cps are accurate. */
;	if ((*hplcDest)->icpAdjust < ifooDest)
;		AdjustHplcCpsToLim(hplcDest, ifooDest);
;	if ((*hplcSrc)->icpAdjust < ifooSrc + cFoo)
;		AdjustHplcCpsToLim(hplcSrc, ifooSrc + cFoo);

	mov	bx,[hplcSrc]
	mov	si,[bx]
	mov	ax,[ifooSrc]
	add	ax,di
	add	ax,[cFoo]
	cmp	ax,[si.icpAdjustPlc]
	jle	CMP03
	cCall	AdjustHplcCpsToLim,<bx,ax>
CMP03:
	mov	bx,[hplcDest]
	mov	si,[bx]
	mov	ax,[ifooDest]
	add	ax,di
	cmp	ax,[si.icpAdjustPlc]
	jle	CMP04
	cCall	AdjustHplcCpsToLim,<bx,ax>
CMP04:

;/* we must move the destination adjustment fence past the end of the range
;   we're going to write over. */
;	if ((*hplcDest)->icpAdjust < ifooDest + cFoo)
; 		(*hplcDest)->icpAdjust = ifooDest + cFoo;
;	di = *hplcDest
	mov	ax,[ifooDest]
	add	ax,di
	add	ax,[cFoo]
	cmp	ax,[si.icpAdjustPlc]
	jle	CMP05
	mov	[si.icpAdjustPlc],ax
CMP05:


;	/* transfer foo's */
;	NOTE: We don't check for cFoo == 0 in the assembler version
;	because the blt code does necessary segment reloading.
;	if (cFoo > 0)
;		bltbx(QInPlc(hplcSrc, ifooSrc), QInPlc(hplcDest, ifooDest), (*hplcSrc)->cb * cFoo);

;	It is possible that the call later on to ReloadSb for hplcDest
;	would force out the sb for hplcSrc.  This would happen if now
;	sbSrc was oldest on the LRU list but still swapped in, and sbDest
;	was swapped out.  In that event ReloadSb would not be called for
;	sbSrc, the LRU entry would not get updated, and the call to ReloadSb
;	for sbDest would swap out sbSrc.
;	This call to QcpQfooPlcfoo makes that state impossible and so works
;	around the LMEM quirk.

;	QcpQfooPlcfoo takes hplcfoo in bx, ifoo in si, it returns pplcfoo
;	in bx, cbfoo in cx, qcp in es:di, qfoo in es:si.
;	if DEBUG it returns hpcp in dx:di, hpfoo in dx:si.
;	Changes ax, bx, cx, dx, si, di.
	mov	bx,[hplcDest]
	xor	si,si
ifdef DEBUG
	mov	di,1	;O.K. to pass ifldMac
endif ;DEBUG
	cCall	N_QcpQfooPlcfoo,<>
ifdef DEBUG
	;Check that es == mpsbps[dx];
	call	CMP12
endif ;DEBUG

;	QcpQfooPlcfoo takes hplcfoo in bx, ifoo in si, it returns pplcfoo
;	in bx, cbfoo in cx, qcp in es:di, qfoo in es:si.
;	if DEBUG it returns hpcp in dx:di, hpfoo in dx:si.
;	Changes ax, bx, cx, dx, si, di.
	mov	bx,[hplcSrc]
	mov	si,[ifooSrc]
ifdef DEBUG
	mov	di,1	;O.K. to pass ifldMac
endif ;DEBUG
	cCall	N_QcpQfooPlcfoo,<>
ifdef DEBUG
	;Check that es == mpsbps[dx];
	call	CMP12
endif ;DEBUG
	mov	[OFF_lprgcpSrc],di

	push	es
	push	si  ;save lpFooSrc

;	QcpQfooPlcfoo takes hplcfoo in bx, ifoo in si, it returns pplcfoo
;	in bx, cbfoo in cx, qcp in es:di, qfoo in es:si.
;	if DEBUG it returns hpcp in dx:di, hpfoo in dx:si.
;	Changes ax, bx, cx, dx, si, di.
	mov	bx,[hplcDest]
	mov	si,[ifooDest]
ifdef DEBUG
	mov	di,1	;O.K. to pass ifldMac
endif ;DEBUG
	cCall	N_QcpQfooPlcfoo,<>
ifdef DEBUG
	;Check that es == mpsbps[dx];
	call	CMP12
endif ;DEBUG
	mov	[OFF_lprgcpDest],di
	mov	di,si

	pop	si
	pop	ds  ;restore lpFooSrc
	pop	ax  ;restore old cFoo
	mul	ss:[bx.cbPlc]
	xchg	ax,cx

	cmp	si,di		    ; reverse direction of the blt if
	jae	CMP08		    ;  necessary
	add	si,cx
	add	di,cx
	std
	dec	si
	dec	di
	dec	si
	dec	di
CMP08:
	shr	cx,1
ifdef DEBUG
	jnc	CMP085
	push	ax
	push	bx
	push	cx
	push	dx
	cld
	mov	ax,midClsplcn
	mov	bx,1029
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
CMP085:
endif ;/* DEBUG */
	rep	movsw
	cld
	 
;	CP far *lprgcpSrc = LprgcpForPlc(*hplcSrc);
;	CP far *lprgcpDest = LprgcpForPlc(*hplcDest);
	mov	ax,[diArg]
	shl	ax,1
	shl	ax,1
	mov	si,[OFF_lprgcpSrc]
	add	si,ax
	mov	di,[OFF_lprgcpDest]
	add	di,ax

;	/* transfer cp's */
;	bltbx((char far *)(lprgcpSrc) + ifooSrc * sizeof(CP),
;		(char far *)(lprgcpDest) + ifooDest * sizeof(CP),
;		cFoo * sizeof(CP));

;	if (dcp != cp0)
;		while (cFoo--)
;			{
;			PutCpPlc(hplcDest, ifooDest, CpPlc(hplcDest, ifooDest) + dcp);
;			ifooDest++;
;			}
	mov	bx,[OFF_dcp]
	mov	dx,[SEG_dcp]
	mov	cx,[cFoo]
	push	bp
	xor	bp,bp
	cmp	si,di
	ja	CMP09
	mov	bp,cx
	dec	bp
	shl	bp,1
	shl	bp,1
	add	si,bp
	add	di,bp
	mov	bp,8
CMP09:
	inc	cx		;adjust for loop instruction
	jmp	short CMP11
CMP10:
	lodsw
	add	ax,bx
	stosw
	lodsw
	adc	ax,dx
	stosw
	sub	si,bp
	sub	di,bp
CMP11:
	loop	CMP10
	pop	bp
	push	ss
	pop	ds
;}
cEnd

ifdef DEBUG
CMP12:
	push	ax
	push	bx
	push	cx
	push	dx
	push	es	;save es from QcpQfooPlcfoo
	mov	bx,dx
	shl	bx,1
	mov	ax,mpsbps[bx]
	mov	es,ax
	shr	ax,1
	jc	CMP13
	;Assembler note: There is no way we should have to call ReloadSb here.
;	reload sb trashes ax, cx, and dx
;	cCall	ReloadSb,<>
	mov	ax,midClsplcn
	mov	bx,1030
	cCall	AssertProcForNative,<ax,bx>
CMP13:
	pop	ax	;restore es from QcpQfooPlcfoo
	mov	bx,es	;compare with es rederived from the SB of QcpQfooPlcfoo
	cmp	ax,bx
	je	CMP14
	mov	ax,midClsplcn
	mov	bx,1031
	cCall	AssertProcForNative,<ax,bx>
CMP14:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

; End of CopyMultPlc

sEnd	clsplcn
        end
