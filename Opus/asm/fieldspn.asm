        include w2.inc
        include noxport.inc
        include consts.inc
        include structs.inc

createSeg	fieldsp_PCODE,fieldspn,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midFieldspn	  equ 23	  ; module ID, for native asserts
endif ;DEBUG

; EXPORTED LABELS


; EXTERNAL FUNCTIONS

externFP	<N_QcpQfooPlcfoo>
externFP	<N_PdodDoc>
externFP	<CompletelyAdjustHplcCps>

ifdef DEBUG
externFP	<AssertProcForNative>
endif ;DEBUG


; EXTERNAL DATA

sBegin  data

externW mpdochdod	;extern struct DOD **mpdochdod[];

ifdef DEBUG
externW mpsbps		;extern SB	     mpsbps[];
endif ;DEBUG

sEnd    data


; CODE SEGMENT fieldspn_PCODE

sBegin	fieldspn
	assumes cs,fieldspn
        assumes ds,dgroup
	assume es:nothing
	assume ss:nothing



;-------------------------------------------------------------------------
;	FFillRgwWithSeqLevs(doc, cp, ipad, ifld, hplcpad, hplcfld, rgw)
;-------------------------------------------------------------------------
;NATIVE FFillRgwWithSeqLevs(doc, cp, ipad, ifld, hplcpad, hplcfld, rgw)
;int doc;
;CP cp;
;int ipad;
;int ifld;
;struct PLC **hplcpad;
;struct PLC **hplcfld;
;int *rgw;
;{
;   CP cpCur;
;   int ipadMac = IMacPlc(hplcpad);
;   int ifldMac = IMacPlc(hplcfld);
;   struct PAD pad;
;   struct FLD fld;

; %%Function:N_FFillRgwWithSeqLevs %%Owner:BRADV
cProc	N_FFillRgwWithSeqLevs,<PUBLIC,FAR>,<si,di>
	ParmW	doc
	ParmD	cp
	ParmW	ipad
	ParmW	ifld
	ParmW	hplcpad
	ParmW	hplcfld
	ParmW	rgw

	LocalW	LO_qPad

cBegin

	;Assembler note: Since we may be running through the entire
	;plc anyway, let's unwind the lazy plc optimization and speed
	;up the rest of the code.
	cCall	CompletelyAdjustHplcCps,<[hplcpad]>
	cCall	CompletelyAdjustHplcCps,<[hplcfld]>

;	It is possible that the call later on to QcpQfooPlcfoo for hplcpad
;	would force out the sb for hplcfld.  This would happen if now
;	sbFld was oldest on the LRU list but still swapped in, and sbPad
;	was swapped out.  In that event ReloadSb would not be called for
;	sbFld, the LRU entry would not get updated, and the call to ReloadSb
;	for sbPad would swap out sbFld.
;	This call to QcpQfooPlcfoo makes that state impossible and so works
;	around the LMEM quirk.

;	QcpQfooPlcfoo takes hplcfoo in bx, ifoo in si, it returns pplcfoo
;	in bx, cbfoo in cx, qcp in es:di, qfoo in es:si.
;	if DEBUG it returns hpcp in dx:di, hpfoo in dx:si.
;	Changes ax, bx, cx, dx, si, di.
	mov	bx,[hplcpad]
	xor	si,si
ifdef DEBUG
	mov	di,1	;O.K. to pass ifldMac
endif ;DEBUG
	cCall	N_QcpQfooPlcfoo,<>
ifdef DEBUG
	;Check that es == mpsbps[dx];
	call	FRWSL13
endif ;DEBUG

;	QcpQfooPlcfoo takes hplcfoo in bx, ifoo in si, it returns pplcfoo
;	in bx, cbfoo in cx, qcp in es:di, qfoo in es:si.
;	if DEBUG it returns hpcp in dx:di, hpfoo in dx:si.
;	Changes ax, bx, cx, dx, si, di.
	mov	bx,[hplcfld]
	mov	si,[ifld]
ifdef DEBUG
	mov	di,1	;O.K. to pass ifldMac
endif ;DEBUG
	cCall	N_QcpQfooPlcfoo,<>
	push	es	;save LO_qcpFld
	push	si	;save HI_qFld
	push	di	;save HI_qcpFld
ifdef DEBUG
	;Check that es == mpsbps[dx];
	call	FRWSL13
endif ;DEBUG
	mov	ax,[bx.iMacPlcStr]
	neg	ax
	add	[ifld],ax   ;ifld now really contains ifld - (*hplcfld)->iMac

;	QcpQfooPlcfoo takes hplcfoo in bx, ifoo in si, it returns pplcfoo
;	in bx, cbfoo in cx, qcp in es:di, qfoo in es:si.
;	if DEBUG it returns hpcp in dx:di, hpfoo in dx:si.
;	Changes ax, bx, cx, dx, si, di.
	mov	bx,[hplcpad]
	mov	si,[ipad]
ifdef DEBUG
	mov	di,1	;O.K. to pass ifldMac
endif ;DEBUG
	cCall	N_QcpQfooPlcfoo,<>
ifdef DEBUG
	;Check that es == mpsbps[dx];
	call	FRWSL13
endif ;DEBUG
	mov	ax,[bx.iMacPlcStr]
	neg	ax
	add	[ipad],ax   ;ipad now really contains ipad - (*hplcpad)->iMac
	mov	[LO_qPad],si
	pop	bx	;restore HI_qcpFld
	pop	si	;restore HI_qFld
	pop	ds	;restore LO_qcpFld

	;ds:si = qFld, ds:bx = qcpFld, es:[LO_qPad] = qPad, es:di = qcpPad
;   cpCur = CpPlc(hplcpad, ipad);
	mov	cx,es:[di]
	mov	dx,es:[di+2]

;   while (ipad < ipadMac && cpCur <= cp)
;	{
FRWSL01:
	cmp	[ipad],0    ;really contains ipad - (*hplcpad)->iMac
	jge	FRWSL015
	cmp	[OFF_cp],cx
	mov	ax,[SEG_cp]
	sbb	ax,dx
	jl	FRWSL015

;	while (ifld < ifldMac && CpPlc(hplcfld, ifld) < cpCur)
;	    /* advance to this para */
;	    ifld++;
	dec	[ifld]	    ;cancel following inc [ifld]
	jmp	short FRWSL03
FRWSL015:
	push	ss
	pop	ds	    ;restore ds
	stc		    ;return fTrue
	jmp	FRWSL10

	;dx:cx = cpCur, ds:si = qFld, ds:bx = qcpFld,
	;es:[LO_qPad] = qPad, es:di = qcpPad
FRWSL02:
	errnz	<cbFldMin - 2>
	inc	si
	inc	si
	add	bx,4
FRWSL03:
	inc	[ifld]	    ;really contains ifld - (*hplcfld)->iMac
	jge	FRWSL04
	cmp	[bx],cx
	mov	ax,[bx+2]
	sbb	ax,dx
	jl	FRWSL02
FRWSL04:

;	cpCur = CpPlc(hplcpad, ipad+1);
	;ipad++ is done below in the C version.
	;dx:cx = cpCur, ds:si = qFld, ds:bx = qcpFld,
	;es:[LO_qPad] = qPad, es:di = qcpPad
	inc	[ipad]	    ;really contains ipad - (*hplcpad)->iMac
	add	[LO_qPad],cbPadMin
	add	di,4
	mov	cx,es:[di]
	mov	dx,es:[di+2]

;	while (ifld < ifldMac && CpPlc(hplcfld, ifld) < cpCur)
;	    {
;	    GetPlc(hplcfld, ifld++, &fld);
;	    if (fld.ch == chFieldBegin &&
;		    (uns)(fld.flt-fltSeqLevOut)<=(fltSeqLevNum - fltSeqLevOut))
;		/* this para does have an auto number, count it */
;		{
FRWSL05:
	cmp	[ifld],0    ;really contains ifld - (*hplcfld)->iMac
	jge	FRWSL01
	cmp	[bx],cx
	mov	ax,[bx+2]
	sbb	ax,dx
	jge	FRWSL01
	inc	[ifld]	    ;really contains ifld - (*hplcfld)->iMac
	errnz	<cbFldMin - 2>
	lodsw
	add	bx,4
	errnz	<(chFld) - 0>
	errnz	<(fltFld) - 1>
	and	al,maskChFld
ifdef DEBUG
	;Make sure we don't carry from al to ah when doing the following
	;subtract.
	call	FRWSL16
endif ;DEBUG
	sub	ax,(fltSeqLevOut SHL 8) + chFieldBegin
	xchg	al,ah
	cmp	ax,fltSeqLevNum - fltSeqLevOut
	ja	FRWSL05

;		GetPlc (hplcpad, ipad, &pad);
	;dx:cx = cpCur, ds:si = qFld, ds:bx = qcpFld,
	;es:[LO_qPad] = qPad, es:di = qcpPad

;		if (pad.lvl == lvlUpdate)
;		    {
;		    Assert(fFalse);
;		    PdodDoc(doc)->fOutlineDirty = fTrue;
;		    return fFalse;
;		    }
	xchg	bx,[LO_qPad]
	mov	al,es:[bx.lvlPad - cbPadMin]
	xchg	bx,[LO_qPad]
	mov	ah,al
	and	al,maskLvlPad
	errnz	<maskLvlPad - 0F0h>
	cmp	al,lvlUpdate SHL 4
	je	FRWSL08

;		if (!pad.fBody && !pad.fInTable)
;		    {
	;dx:cx = cpCur, ds:si = qFld, ds:bx = qcpFld,
	;es:[LO_qPad] = qPad, es:di = qcpPad
	push	bx	;save LO_qcpFld
	mov	bx,lvlBody SHL 1
	and	ah,maskFBodyPad + maskFInTablePad
	jne	FRWSL07

;		    Assert (pad.lvl < lvlBody && pad.lvl >= 0);
ifdef DEBUG
	;Do this assert with a call so as not to mess up short jumps
	call	FRWSL11
endif ;/* DEBUG */

;		    rgw [pad.lvl]++;
;		    SetWords (rgw + pad.lvl + 1, 0, lvlBody - pad.lvl);
;		    }
	push	cx	;save LO_cpCur
	push	di	;save LO_qcpPad
	push	es	;save HI_qcpPad
	errnz	<maskLvlPad - 0F0h>
	mov	cl,3
	shr	al,cl
	sub	bx,ax
	mov	cx,bx
	shr	cx,1
	mov	di,[rgw]
	xchg	ax,bx
	lea	di,[bx+di+2]
	xor	ax,ax
	push	ss
	pop	es
	rep	stosw
	pop	es	;restore HI_qcpPad
	pop	di	;restore LO_qcpPad
	pop	cx	;restore LO_cpCur

;		else
;		    rgw [lvlBody]++;
;		break;
FRWSL07:
	add	bx,[rgw]
	inc	wptr ss:[bx]
	pop	bx	;restore LO_qcpFld
	jmp	FRWSL01

;		}
;	    }
	;ipad++ is done above in the assembler version.
;	ipad++;
;	}

FRWSL08:
	push	ss
	pop	ds
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFieldspn
	mov	bx,1002
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */
	cCall	N_PdodDoc,<[doc]>
	xchg	ax,bx
	or	[bx.fOutlineDirtyDod],maskFOutlineDirtyDod
		;Note that the preceding "or" clears the carry flag
FRWSL10:
;   return fTrue;
	sbb	ax,ax

;}
cEnd

ifdef DEBUG
FRWSL11:
	cmp	al,lvlBody SHL 4
	jb	FRWSL12
	push	ax
	push	bx
	push	cx
	push	dx
	push	ds
	push	ss
	pop	ds
	mov	ax,midFieldspn
	mov	bx,1001
	cCall	AssertProcForNative,<ax,bx>
	pop	ds
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FRWSL12:
	ret
endif ;/* DEBUG */

ifdef DEBUG
FRWSL13:
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
	jc	FRWSL14
	;Assembler note: There is no way we should have to call ReloadSb here.
;	reload sb trashes ax, cx, and dx
;	cCall	ReloadSb,<>
	mov	ax,midFieldspn
	mov	bx,1003
	cCall	AssertProcForNative,<ax,bx>
FRWSL14:
	pop	ax	;restore es from QcpQfooPlcfoo
	mov	bx,es	;compare with es rederived from the SB of QcpQfooPlcfoo
	cmp	ax,bx
	je	FRWSL15
	mov	ax,midFieldspn
	mov	bx,1004
	cCall	AssertProcForNative,<ax,bx>
FRWSL15:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

ifdef DEBUG
FRWSL16:
	cmp	al,chFieldBegin
	jae	FRWSL17
	push	ax
	push	bx
	push	cx
	push	dx
	push	ss
	pop	ds
	mov	ax,midFieldspn
	mov	bx,1005
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FRWSL17:
	ret
endif ;/* DEBUG */

sEnd	fieldspn_PCODE
	end
