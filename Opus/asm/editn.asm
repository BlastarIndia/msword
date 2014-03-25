        include w2.inc
        include noxport.inc
        include consts.inc
        include structs.inc

createSeg	edit_PCODE,edit,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midEditn	equ 15		 ; module ID, for native asserts
endif

; EXTERNAL FUNCTIONS

externFP	<CorrectDodPlcs>
externFP	<N_PdodMother>
externFP	<GetSelCurChp>
externFP	<ErrorEidProc>
externFP	<ReloadSb>
externFP	<IInPlcRef>
externFP	<CpPlc>
externFP	<GetPlc>
externFP	<PutCpPlc>
externFP	<N_PdrFetch>
externFP	<N_FreePdrf>
externFP	<N_FStretchPlc>
externFP	<FStretchSttbCb>
externFP	<CloseUpSttb>
externFP	<DisposeWwHdr>
externFP	<CloseBkmkStructs>
externFP	<FChngSizePhqLcb>
externFP	<SetSelCellBits>
externNP	<LN_PdodDocEdit>

ifdef DEBUG
externFP	<S_AdjustHplcedlCps>
externFP	<S_PdrFetch>
externFP	<S_FreePdrf>
externFP	<AssertProcForNative>
externFP	<ScribbleProc>
externFP	<N_PwwdWw>
externFP	<S_FStretchPlc>
externFP	<FCheckHandle>
externNP	<LN_FreezeHpEdit>
externNP	<LN_MeltHpEdit>
endif ;DEBUG


sBegin  data

; EXTERNALS
externW     caAdjust
externW     caAdjustL
externW     caHdt
externW     caPage
externW     caPara
externW     caSect
externW     caTable
externW     caTap
externW     mpdochdod
externW     mpwwhwwd
externW     rgselsInsert
externW     selCur
externW     vdocFetch
externW     vfls
externW     vitcMic
externW     vmpitccp
externW     vlcb
externW     vrulss
externW     vsccAbove
externW     vtapFetch
externW     vtcc
externW     vtcxs
externW     vsab
externW     mpsbps
externW     vfInCommit
externW     vmerr
externW		caTapAux
externW		vhplbmc

ifdef DEBUG
externW     cCaAdjust
externW     cCaAdjustL
externW     cHpFreeze
externW     vdbs
externW     wFillBlock
externW     wwMac
externW     vpdrfHead
externW     caTapAux
externW     vmpitccpAux
externW     vtapFetchAux
externW     vitcMicAux
endif ;DEBUG


sEnd    data

; CODE SEGMENT _EDIT

sBegin	edit
	assumes cs,edit
        assumes ds,dgroup
        assumes ss,dgroup

include asserth.asm


;-------------------------------------------------------------------------
;	AdjustCp(pca, dcpIns)
;-------------------------------------------------------------------------
;/* A D J U S T  C P */
;/* Adjust all cp's > cpFirst by dcpIns - dcpDel.
;Invalidation must have been done prior to this.
;Piece tables are adjusted elsewhere.
;*/
;AdjustCp(pca, dcpIns)
;struct CA *pca;
;CP dcpIns;
;{
;	 int ww;
;	 int doc = pca->doc;
;	 struct WWD *pwwd;
;	 struct DOD *pdod = PdodDoc(doc);
;	 int fFixMacEntry;
;	 int isels;
;	 int idr, idrMac;
;	 struct DR *pdr;
;	 struct SELS *pselsT;
;	 CP dcpAdj = dcpIns - DcpCa(pca);
;	 CP cp1, cpFirstDel, cpLimDel, cpMac, cpT;
;	 extern struct PL **vhplbmc;

; %%Function:N_AdjustCp %%Owner:BRADV
cProc	N_AdjustCp,<PUBLIC,FAR>,<si,di>
	ParmW	<pca>
	ParmD	<dcpIns>

	LocalW	doc
	LocalW	pdod
	LocalW	fDontFixMacEntry
	LocalW	pselsLim
	LocalW	pcpMac
	LocalW	pdrMac
	LocalD	dcpAdj
	LocalD	cp1
	LocalW	pbmcFirst
ifdef DEBUG
	LocalV	rgchAdjustCp,10
endif ;DEBUG

cBegin
ifdef DEBUG
	push	cx
	push	si
	push	di
	push	ds
	push	es
	push	cs
	pop	ds
	push	ss
	pop	es
	mov	si,offset szAdjustCp
	lea	di,[rgchAdjustCp]
	mov	cx,cbSzAdjustCp
	rep	movsb
	jmp	short AC005
szAdjustCp:
	db	'AdjustCp',0
cbSzAdjustCp equ $ - szAdjustCp
	errnz	<cbSzAdjustCp - 9>
AC005:
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	cx
endif ;DEBUG
	mov	si,[pca]
	mov	bx,[si.docCa]
	mov	[doc],bx
	;LN_PdodDocEdit assumes doc passed in bx and performs
	;PdodDoc(doc).	Only bx is altered.
	call	LN_PdodDocEdit
	mov	[pdod],bx

;	 CP dcpAdj = dcpIns - DcpCa(pca);
	mov	ax,[OFF_dcpIns]
	mov	dx,[SEG_dcpIns]
	;***Begin in line DcpCa
	;return (pca->cpLim - pca->cpFirst);
	sub	ax,[si.LO_cpLimCa]
	sbb	dx,[si.HI_cpLimCa]
	add	ax,[si.LO_cpFirstCa]
	adc	dx,[si.HI_cpFirstCa]
	;***End in line DcpCa
	mov	[OFF_dcpAdj],ax
	mov	[SEG_dcpAdj],dx

;	Scribble(ispAdjustCp,'A');
ifdef DEBUG
	cmp	[vdbs.grpfScribbleDbs],0
	jz	AC01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,ispAdjustCp
	mov	bx,'A'
	cCall	ScribbleProc,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
AC01:
endif ;DEBUG

;	if (dcpAdj == 0)
;		return;
	mov	cx,ax
	or	cx,dx
	jne	Ltemp003
	jmp	AC37
Ltemp003:

;	Assert(cHpFreeze==0);
ifdef DEBUG
	cmp	[cHpFreeze],0
	je	AC02
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn
	mov	bx,1001
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
AC02:
endif ;DEBUG

;	FreezeHp();
ifdef	DEBUG
	call	LN_FreezeHpEdit
endif ;DEBUG

;	pdod = PdodDoc(doc);
	;Assembler note: pdod already exists in bx.

;	fFixMacEntry = (pca->cpFirst == CpMac2Doc(doc));
	mov	cx,[si.LO_cpFirstCa]
	mov	di,[si.HI_cpFirstCa]
	;***Begin in line CpMac2Doc
	;return ((*mpdochdod[doc]->cpMac);
	sub	cx,[bx.LO_cpMacDod]
	sbb	di,[bx.HI_cpMacDod]
	;***End in line CpMac2Doc
	or	cx,di
	mov	[fDontFixMacEntry],cx

;	cpMac = (pdod->cpMac += dcpAdj);
	add	[bx.LO_cpMacDod],ax
	adc	[bx.HI_cpMacDod],dx
	mov	ax,[bx.LO_cpMacDod]
	mov	dx,[bx.HI_cpMacDod]

;	if (cpMac >= cpWarnTooBig || cpMac < 0)
;	    {
	or	dx,dx
	jl	AC03
	errnz	<LO_cpWarnTooBig>
	cmp	dx,HI_cpWarnTooBig
	jb	AC05
AC03:

;	    MeltHp();
ifdef	DEBUG
	call	LN_MeltHpEdit
endif ;DEBUG

;			/* this alert will probably be our last act... */
;		    ErrorEid(eidCpRollOver, "AdjustCp");
	mov	ax,eidCpRollOver
	push	ax
ifdef DEBUG
	lea	ax,[rgchAdjustCp]
	push	ax
endif ;DEBUG
	cCall	ErrorEidProc,<>

;	    FreezeHp();
ifdef	DEBUG
	call	LN_FreezeHpEdit
endif ;DEBUG

	mov	bx,[doc]
	;LN_PdodDocEdit assumes doc passed in bx and performs
	;PdodDoc(doc).	Only bx is altered.
	call	LN_PdodDocEdit
	mov	[pdod],bx

;	    }
AC05:

;	cp1 = pca->cpFirst + 1;
	mov	ax,1
	cwd
	add	ax,[si.LO_cpFirstCa]
	adc	dx,[si.HI_cpFirstCa]
	mov	[OFF_cp1],ax
	mov	[SEG_cp1],dx

;	/* note: this corrects hplchdd, hplcfnd, hplcglsy which all reside at
;	   the same dod location */
;	AdjustHplc(pdod->hplchdd, cp1, dcpAdj, -1);
;	AdjustHplc(hplc = pdod->hplcphe, cp1, dcpAdj, -1);
;	if (hplc && (CpPlc(hplc, 0) < cp0))
;		PutCpPlc(hplc, 0, cp0);
;	AdjustHplc(pdod->hplcpgd, cp1, dcpAdj, -1);
;	AdjustHplc(pdod->hplcfld, pca->cpFirst, dcpAdj, -1);
;	if (!pdod->fShort)
;		{
;		AdjustHplc(pdod->hplcbkf, cp1, dcpAdj, -1);
;		AdjustHplc(pdod->hplcbkl, cp1, dcpAdj, -1);
;		AdjustHplc(pdod->hplcatrd, pca->cpFirst/*sic*/, dcpAdj, -1);
;		AdjustHplc(pdod->hplcfrd, pca->cpFirst/*sic*/, dcpAdj, -1);
;		AdjustHplc(pdod->hplcsed, cp1, dcpAdj, -1);
;		if (pdod->fSea)
;			AdjustHplc(pdod->hplcsea, cp1, dcpAdj, -1);
;		AdjustHplc(pdod->hplcpad, cp1, dcpAdj, -1);
;		}
	mov	si,offset rgOffsetHplcShort
	mov	di,offset rgOffsetHplcLong
AC06:
	lods	wptr cs:[si]
	mov	bl,ah
	xor	bh,bh
	add	bx,[pdod]
	mov	cx,[bx]
	errnz	<hNil>
	jcxz	AC08
	push	cx	;push hplc
	cbw
	cwd
	add	ax,[OFF_cp1]
	adc	dx,[SEG_cp1]
	push	dx
	push	ax
	push	[SEG_dcpAdj]
	push	[OFF_dcpAdj]
	mov	cx,-1
	push	cx
	push	cs
	call	near ptr AdjustHplc
AC08:
	cmp	si,di
	jb	AC06
	cmp	di,offset rgOffsetHplcMac
	je	AC10
	mov	bx,[pdod]

;	if (hplc && (CpPlc(hplc, 0) < cp0))
;		PutCpPlc(hplc, 0, cp0);
	mov	cx,[bx.hplcPheDod]
	jcxz	AC087
	push	bx	;save pdod
	push	cx	;save hplc
	xor	ax,ax
	cCall	CpPlc,<cx, ax>
	pop	cx	;restore hplc
	or	dx,dx
	jge	AC083
	xor	ax,ax
	cCall	PutCpPlc,<cx, ax, ax, ax>
AC083:
	pop	bx	;restore pdod
AC087:
	cmp	[bx.fShortDod],fFalse
	jne	AC10
	mov	di,offset rgOffsetHplcMac
	;Assert (pdod->fSea || pdod->hplcsea == hNil);
ifdef DEBUG
	call	AC42
endif ;DEBUG
	jmp	short AC06

rgOffsetHplcShort:
	dw	00000h + ((hplchddDod) SHL 8)
	dw	00000h + ((hplcpheDod) SHL 8)
	dw	00000h + ((hplcpgdDod) SHL 8)
	dw	000FFh + ((hplcfldDod) SHL 8)
rgOffsetHplcLong:
	dw	00000h + ((hplcbkfDod) SHL 8)
	dw	00000h + ((hplcbklDod) SHL 8)
	dw	000FFh + ((hplcatrdDod) SHL 8)
	dw	000FFh + ((hplcfrdDod) SHL 8)
	dw	00000h + ((hplcsedDod) SHL 8)
	dw	00000h + ((hplcseaDod) SHL 8)
	dw	00000h + ((hplcpadDod) SHL 8)
rgOffsetHplcMac:

AC10:
;	/* if we were inserting at end of document, the CP for iMac will still
;	   be set to cpFirst for those PLCs for which no entries were added.
;	   We force the iMac CP for each PLC to be the newly adjusted
;	   CpMacDoc() value. */
;	if (fFixMacEntry)
;		CorrectDodPlcs(doc, CpMac2Doc(doc));
	cmp	[fDontFixMacEntry],fFalse
	jne	AC11
	push	[doc]
	;***Begin in line CpMac2Doc
	;return ((*mpdochdod[doc]->cpMac);
	mov	bx,[pdod]
	push	[bx.HI_cpMacDod]
	push	[bx.LO_cpMacDod]
	;***End in line CpMac2Doc
	cCall	CorrectDodPlcs,<>
AC11:

;	/* no need to adjust caRulerSprm, it should have been flushed */
;	Assert( doc == docUndo || vrulss.caRulerSprm.doc == docNil );
ifdef DEBUG
	cmp	[doc],docUndo
	je	AC12
	cmp	[vrulss.caRulerSprmRulss.docCa],docNil
	je	AC12
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn
	mov	bx,1003
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
AC12:
endif ;DEBUG

;	AdjustCa(&caPara, doc, pca->cpFirst, pca->cpLim, dcpAdj);
;	AdjustCa(&caPage, doc, pca->cpFirst, pca->cpLim, dcpAdj);
;	AdjustCa(&caSect, doc, pca->cpFirst, pca->cpLim, dcpAdj);
;	AdjustCa(&caHdt, doc, pca->cpFirst, pca->cpLim, dcpAdj);
;	AdjustCa(&vfls.ca, doc, pca->cpFirst, pca->cpLim, dcpAdj);

;	AdjustCa(&caTable, doc, pca->cpFirst, pca->cpLim, dcpAdj);
;	AdjustCa(&caTap, doc, pca->cpFirst, pca->cpLim, dcpAdj);
;	AdjustCa(&vtcc.ca, doc, pca->cpFirst, pca->cpLim, dcpAdj);
;	AdjustCa(&caTapAux, doc, pca->cpFirst, pca->cpLim, dcpAdj);
;	AdjustCa(&vtcxs.ca, doc, pca->cpFirst, pca->cpLim, dcpAdj);

;	AdjustCa(&vlcb.ca, doc, pca->cpFirst, pca->cpLim, dcpAdj);
;	AdjustCa(&vsab.caCopy, doc, pca->cpFirst, pca->cpLim, dcpAdj);

;	Assert(caAdjust.doc == docNil || cCaAdjust == 1);
;	AdjustCa(&caAdjust, doc, pca->cpFirst, pca->cpLim, dcpAdj);
;	Assert(caAdjustL.doc == docNil || cCaAdjustL == 1);
;	AdjustCa(&caAdjustL, doc, pca->cpFirst, pca->cpLim, dcpAdj);
ifdef DEBUG
	cmp	[caAdjust.docCa],docNil
	je	AC13
	cmp	[cCaAdjust],1
	jne	AC131
AC13:
	cmp	[caAdjustL.docCa],docNil
 	je	AC132
	cmp	[cCaAdjustL],1
	je	AC132
AC131:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn
	mov	bx,1004
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
AC132:
endif ;DEBUG

	mov	cx,[doc]
	mov	di,[pca]
	mov	si,offset rgOffsetCa - 2
	push	si
AC14:
	pop	si
	cmp	si,offset rgOffsetCaMac - 2
	je	AC145
	inc	si
	inc	si
	push	si
	mov	si,cs:[si]

	;LN_AdjustCa can only be used as a near call from AdjustCp
	;or near subroutines of AdjustCp.
	;It takes pca in si, doc in cx, cpFirstDel in di.cpFirstCa,
	;cpLimDel in di.cpLimCa, dcp in AdjustCp's dcpAdj.
	;ax, bx and dx are altered.
	call	LN_AdjustCa
	jmp	short AC14

;		 }
;}

rgOffsetCa:
	dw	dataoffset [caPara]
	dw	dataoffset [caPage]
	dw	dataoffset [caSect]
	dw	dataoffset [caHdt]
	dw	dataoffset [vfls.caFls]
	dw	dataoffset [caTable]
	dw	dataoffset [caTap]
	dw	dataoffset [vtcc.caTcc]
	dw	dataoffset [vtcc.caTapTcc]
	dw	dataoffset [caTapAux]
	dw	dataoffset [vtcxs.caTcxs]
	dw	dataoffset [vlcb.caLcb]
	dw	dataoffset [vsab.caCopySab]
	dw	dataoffset [caAdjust]
	dw	dataoffset [caAdjustL]
rgOffsetCaMac:

AC145:
;	if (vhplbmc != hNil)
;		{

ifdef	DEBUG
	call	LN_FreezeHpEdit
endif ;DEBUG
	mov	si,[vhplbmc]
	errnz	<hNil>
	or	si,si
	je AC16

;		int ibmc;
;		struct BMC *pbmc;
;
;		FreezeHp();
;		for (ibmc = (*vhplbmc)->iMac - 1, pbmc = PInPl(vhplbmc,ibmc);
;			ibmc >= 0; ibmc--, pbmc--)
;			{
	mov si,[si]
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp		[si.fExternalPl],fFalse
	je		AC147
	mov	ax,midEditn
	mov	bx,1060
	cCall	AssertProcForNative,<ax,bx>
AC147:
	cmp		[si.cbPl],cbBmcMin
	je		AC148
	mov	ax,midEditn
	mov	bx,1061
	cCall	AssertProcForNative,<ax,bx>
AC148:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG
	mov		ax,cbBmcMin
	mul		[si.iMacPl]
	add		si,[si.brgfooPl]
	mov		[pbmcFirst],si
	add		si,ax
AC15:
	sub		si,cbBmcMin
	cmp		si,[pbmcFirst]
	jb		AC16

;			AdjustCa(&pbmc->ca, doc, cpFirstDel, cpLimDel, dcpAdj);
;			}
;		MeltHp();
;		}
	;LN_AdjustCa can only be used as a near call from AdjustCp
	;or near subroutines of AdjustCp.
	;It takes pca in si, doc in cx, cpFirstDel in di.cpFirstCa,
	;cpLimDel in di.cpLimCa, dcp in AdjustCp's dcpAdj.
	;ax, bx and dx are altered.
	errnz	<(caBmc) - 0>
	call	LN_AdjustCa
	jmp		short AC15
AC16:
ifdef	DEBUG
	call	LN_MeltHpEdit
endif ;DEBUG

;	/* BLOCK - Adjust vmpitccp used to cache known cell boundaries for
;		the current table row. */
;	    {
;	    CP *pcp, *pcpMac;
;	    int ccp;
;
;	    if ( caTap.doc==doc && caTap.cpLim >= cp1 &&
;		    (ccp=vtapFetch.itcMac+1+1-vitcMic) > 0 )
;		    /* plus 1 for ttp, 1 for fencepost rule */
;		{
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov		ax,dataoffset [caTap]
	sub		ax,dataoffset [caTapAux]
	mov		bx,dataoffset [vmpitccp]
	sub		bx,dataoffset [vmpitccpAux]
	cmp		ax,bx
	je		AC162
	mov	ax,midEditn
	mov	bx,1050
	cCall	AssertProcForNative,<ax,bx>
AC162:
	mov		ax,dataoffset [caTap]
	sub		ax,dataoffset [caTapAux]
	mov		bx,dataoffset [vtapFetch]
	sub		bx,dataoffset [vtapFetchAux]
	cmp		ax,bx
	je		AC163
	mov	ax,midEditn
	mov	bx,1053
	cCall	AssertProcForNative,<ax,bx>
AC163:
	mov		ax,dataoffset [caTap]
	sub		ax,dataoffset [caTapAux]
	mov		bx,dataoffset [vitcMic]
	sub		bx,dataoffset [vitcMicAux]
	cmp		ax,bx
	je		AC164
	mov	ax,midEditn
	mov	bx,1054
	cCall	AssertProcForNative,<ax,bx>
AC164:
	pop		dx
	pop		cx
	pop		bx
	pop		ax
endif ;DEBUG
	xor di,di
AC165:
	mov	bx,[pca]
	mov	cx,[bx.HI_cpFirstCa]
	mov	bx,[bx.LO_cpFirstCa]
	mov	ax,[OFF_dcpAdj]
	mov	dx,[SEG_dcpAdj]
	mov	si,[doc]
	cmp	[di.caTap.docCa],si
	jne	AC20
	;Assembler note: we are really using one less than the cp1, so
	;we are looking for "<=" rather than "<" here.
	cmp	bx,[di.caTap.LO_cpLimCa]
	push	cx
	sbb	cx,[di.caTap.HI_cpLimCa]
	pop	cx
	jge	AC20
	mov	si,[di.vtapFetch.itcMacTap]
	inc	si
	inc	si
	sub	si,[di.vitcMic]
	jle	AC20

;		pcpMac = (pcp = &vmpitccp[vitcMic]) + ccp;
	shl	si,1
	shl	si,1
	mov	[pcpMac],si
	mov	si,[di.vitcMic]
	shl	si,1
	shl	si,1
	add	si,dataoffset [vmpitccp]
	add si,di
	add	[pcpMac],si

	;Assembler note: we are really using one less than the cp1, so
	;we are looking for "<=" rather than "<" here.
;		while ( *(pcp++) < cp1 )
;		    ;
AC17:
	add	si,4
	cmp	bx,[si-4]
	push	cx
	sbb	cx,[si-2]
	pop	cx
	jge	AC17

;		--pcp;
	sub	si,8

;		while ( pcp < pcpMac )
;		    *(pcp++) += dcpAdj;
	jmp	short AC19
AC18:
	add	[si],ax
	adc	[si+2],dx
AC19:
	add	si,4
	cmp	si,[pcpMac]
	jb	AC18

;		}
;	    }
AC20:

;		if ( caTapAux.doc==doc && caTapAux.cpLim >= cp1 &&
;				(ccp=vtapFetchAux.itcMac+1+1-vitcMicAux) > 0 )
;			/* plus 1 for ttp, 1 for fencepost rule */
;			{
;			pcpMac = (pcp = &vmpitccpAux[vitcMicAux]) + ccp;
;			while ( *(pcp++) < cp1 )
;				;
;			--pcp;
;			while ( pcp < pcpMac )
;				*(pcp++) += dcpAdj;
;			}
	mov	cx,di
	mov	di,dataoffset [caTapAux]
	sub	di,dataoffset [caTap]
	jcxz	AC165
	mov	di,[doc]

;	for (isels = 0, pselsT = &rgselsInsert[0]; isels < iselsInsertMax; isels++, pselsT++)
;		{
;		if (pselsT->doc == doc)
; 			{
;			AdjustSels(pselsT, cp1, dcpAdj);
;			if (pselsT->sk == skBlock)
;				pselsT->sk = skSel;	/* prevent illegal block sel */
;			}
;		}
	mov	[pselsLim],dataoffset [rgselsInsert + iselsInsertMax * cbSelsMin]
	mov	si,dataoffset [rgselsInsert]
AC21:
	errnz	<iselsInsertMax - 4>
	cmp	[si.docSel],di
	jne	AC22
	;AC44 takes psel in si, doc in di, and pca, dcpAdjust from AdjustCp's
	;stack.  It performs AdjustSels(psel, cpFirst, cpLim, dcpAdj) and
	;if (psels->sk == skBlock)
	;	 psels->sk = skSel;	/* prevent illegal block sel */
	;ax, bx, cx, dx are altered.
	call	AC44
AC22:
	add	si,cbSelsMin
	cmp	si,[pselsLim]
	jb	AC21

;	FreezeHp();
ifdef	DEBUG
	call	LN_FreezeHpEdit
endif ;DEBUG

;	/* adjust all dr's in all wwd's displaying doc */
;	for (ww = PdodMother(doc)->wwDisp; ww != wwNil; ww = pwwd->wwDisp)
;		{
	cCall	N_PdodMother,<di>
	xchg	ax,si
	mov	al,[si.wwDispDod]
	xor	ah,ah
	xchg	ax,si
AC23:
	errnz	<wwNil - 0>
	or	si,si
	je	Ltemp002

;		pwwd = PwwdWw(ww);
	shl	si,1
	mov	si,[si.mpwwhwwd]
	;Assert (hwwd != hNil) with a call so as not to mess up short jumps.
ifdef DEBUG
	call	AC40
endif ;DEBUG
	mov	si,[si]

;		idrMac = pwwd->idrMac;
	push	si	;save pwwd
	mov	ax,cbDrMin
	mul	[si.idrMacWwd]
	add	si,(rgdrWwd)
	add	ax,si
	mov	[pdrMac],ax
	jmp	short AC245

Ltemp002:
	jmp	AC335

	;Assembler note: fDocFound is not used, so don't set it.
;		fDocFound = fFalse;

;		for (pdr = pwwd->rgdr, idr = 0; idr < idrMac; ++idr, ++pdr)
;			{
AC24:
	add	si,cbDrMin
AC245:
	cmp	si,[pdrMac]
	jb	Ltemp001
	jmp	AC31
Ltemp001:

;			if (pdr->doc != doc)
;				continue;
	cmp	[si.docDr],di
	jne	AC24

	;Assembler note: fDocFound is not used, so don't set it.
;			fDocFound = fTrue;

;			/* check dr.cpFirst */
;			if (pdr->cpFirst >= cp1)
;				{
	mov	ax,[si.LO_cpFirstDr]
	mov	dx,[si.HI_cpFirstDr]
	cmp	ax,[OFF_cp1]
	push	dx
	sbb	dx,[SEG_cp1]
	pop	dx
	jl	AC25

;				if (pdr->cpFirst < pca->cpLim)
;					{
;					pdr->cpFirst = pca->cpFirst;
;					pdr->fCpBad = fTrue;
;					pwwd->fSetElev = fTrue;
;					}
;				else
;					pdr->cpFirst += dcpAdj;
	mov	cx,[OFF_dcpAdj]
	add	[si.LO_cpFirstDr],cx
	mov	cx,[SEG_dcpAdj]
	adc	[si.HI_cpFirstDr],cx
	mov	bx,[pca]
	sub	ax,[bx.LO_cpLimCa]
	sbb	dx,[bx.HI_cpLimCa]
	jge	AC25
	mov	ax,[bx.LO_cpFirstCa]
	mov	dx,[bx.HI_cpFirstCa]
	mov	[si.LO_cpFirstDr],ax
	mov	[si.HI_cpFirstDr],dx
	or	[si.fCpBadDr],maskFCpBadDr
	pop	bx	;restore pwwd
	push	bx	;save pwwd
	or	[bx.fSetElevWwd],maskFSetElevWwd

;				}
AC25:

;			/* check dr.cpLim */
;			if (pdr->cpLim != cpNil && pdr->cpLim >= cp1)
;				{
	mov	ax,[si.LO_cpLimDr]
	mov	dx,[si.HI_cpLimDr]
	errnz	<LO_cpNil - 0FFFFh>
	errnz	<HI_cpNil - 0FFFFh>
	mov	cx,ax
	and	cx,dx
	inc	cx
	je	AC29
	cmp	ax,[OFF_cp1]
	push	dx
	sbb	dx,[SEG_cp1]
	pop	dx
	jl	AC29

;				if (pdr->cpLim < pca->cpLim && pdr->idrFlow != idrNil)
;					{
	mov	bx,[pca]
	cmp	ax,[bx.LO_cpLimCa]
	push	dx
	sbb	dx,[bx.HI_cpLimCa]
	pop	dx
	jge	AC26
	cmp	[si.idrFlowDr],idrNil
	je	AC26

;					pdr->cpLim = cpNil;
;					/* REVIEW: pwwd->fSetElev = fTrue; */
;					}
	errnz	<LO_cpNil - 0FFFFh>
	errnz	<HI_cpNil - 0FFFFh>
	mov	ax,LO_cpNil
	cwd
	jmp	short AC28

;				else
;					{
AC26:

;					/* adjust dr.cpLim; it is now suspect */
;					pdr->cpLim += dcpAdj;
;					pdr->fLimSuspect = pdr->idrFlow != idrNil;
	and	[si.fLimSuspectDr],NOT maskFLimSuspectDr
	cmp	[si.idrFlowDr],idrNil
	je	AC27
	or	[si.fLimSuspectDr],maskFLimSuspectDr
AC27:
	add	ax,[OFF_dcpAdj]
	adc	dx,[SEG_dcpAdj]
AC28:
	mov	[si.LO_cpLimDr],ax
	mov	[si.HI_cpLimDr],dx

;					}

;				}
AC29:
;			if (pdr->hplcedl != hNil)
;				AdjustHplcedlCps(pdr->hplcedl, cp1, dcpAdj);
	mov	cx,[si.hplcedlDr]
	;AC38 performs AdjustHplcedlCps(cx,cp1,dcpAdj) if cx != 0;
	;ax, bx, cx, dx are altered.
	call	AC38
AC30:

;			} /* next dr */
	jmp	AC24

AC31:
	pop	si	;restore pwwd

;		if (doc == pwwd->sels.doc)
;			{
;			AdjustSels(&pwwd->sels, cp1, dcpAdj);
;			if (pwwd->sels.sk == skBlock)
;				pwwd->sels.sk = skSel;	/* prevent illegal block sel */
;			}
	cmp	di,[si.selsWwd.docSel]
	jne	AC32
	push	si	;save pwwd
	add	si,(selsWwd)
	;AC44 takes psel in si, doc in di, and pca, dcpAdjust from AdjustCp's
	;stack.  It performs AdjustSels(psel, cpFirst, cpLim, dcpAdj) and
	;if (psels->sk == skBlock)
	;	 psels->sk = skSel;	/* prevent illegal block sel */
	;ax, bx, cx, dx are altered.
	call	AC44
	pop	si	;restore pwwd
AC32:

;		} /* next ww */
	mov	si,[si.wwDispWwd]
	jmp	AC23

AC335:
;	MeltHp();
ifdef	DEBUG
	call	LN_MeltHpEdit
endif ;DEBUG

;	if (doc == selCur.doc)
;		{
	cmp	di,[selCur.docSel]
	jne	AC33

;		selCur.cpFirstLine = cpNil;	/* no anchor for block sel */
	errnz	<LO_cpNil - 0FFFFh>
	errnz	<HI_cpNil - 0FFFFh>
	mov	ax,LO_cpNil
	mov	[selCur.LO_cpFirstLineSel],ax
	mov	[selCur.HI_cpFirstLineSel],ax

;		AdjustSels(&selCur, cp1, dcpAdj);
	;LN_AdjustSels takes psel in si, doc in di, and pca, dcpAdjust
	;from AdjustCp's stack.  ax, bx, cx, dx are altered.
	mov	si,dataoffset [selCur]
	call	LN_AdjustSels

;		}
AC33:

;	if (doc == vsccAbove.doc)
;		AdjustHplcedlCps(vsccAbove.hplcedl, cp1, dcpAdj);
	cmp	di,[vsccAbove.rgdrWwd.docDr]
	jne	AC35
	mov	cx,[vsccAbove.rgdrWwd.hplcedlDr]
	;AC38 performs AdjustHplcedlCps(cx,cp1,dcpAdj) if cx != 0;
	;ax, bx, cx, dx are altered.
	call	AC38
AC35:

;/* invalidate sequential fetch */
;	if (doc == vdocFetch)
;		vdocFetch = docNil;
	cmp	di,[vdocFetch]
	jne	AC355
	mov	[vdocFetch],docNil
AC355:

;	Scribble(ispAdjustCp,' ');
ifdef DEBUG
	cmp	[vdbs.grpfScribbleDbs],0
	jz	AC36
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,ispAdjustCp
	mov	bx,' '
	cCall	ScribbleProc,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
AC36:
endif ;DEBUG

;	MeltHp();
ifdef	DEBUG
	call	LN_MeltHpEdit
endif ;DEBUG

AC37:
;}
cEnd


	;AC38 performs AdjustHplcedlCps(cx,cp1,dcpAdj) if cx != 0;
	;ax, bx, cx, dx are altered.
AC38:
	jcxz	AC39
ifdef DEBUG
	cCall	S_AdjustHplcedlCps,<cx,[SEG_cp1],[OFF_cp1],[SEG_dcpAdj],[OFF_dcpAdj]>
else ;!DEBUG
	cCall	LN_AdjustHplcedlCps,<cx,[SEG_cp1],[OFF_cp1],[SEG_dcpAdj],[OFF_dcpAdj]>
endif ;!DEBUG
AC39:
	ret


	;Assert (hwwd != hNil);
ifdef DEBUG
AC40:
	cmp	si,hNil
	jne	AC41
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn
	mov	bx,1005
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
AC41:
	ret
endif ;DEBUG

	;Assert (pdod->fSea || pdod->hplcsea == hNil);
ifdef DEBUG
AC42:
	test	[bx.fSeaDod],maskFSeaDod
	jne	AC43
	cmp	[bx.hplcseaDod],hNil
	je	AC43
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn
	mov	bx,1002
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
AC43:
	ret
endif ;DEBUG

	;AC44 takes psel in si, doc in di, and pca, dcpAdjust from AdjustCp's
	;stack.  It performs AdjustSels(psel, cpFirst, cpLim, dcpAdj) and
	;if (psels->sk == skBlock)
	;	 psels->sk = skSel;	/* prevent illegal block sel */
	;ax, bx, cx, dx are altered.
AC44:
	;LN_AdjustSels takes psel in si, doc in di, and pca, dcpAdjust
	;from AdjustCp's stack.  ax, bx, cx, dx are altered.
	call	LN_AdjustSels
	mov	al,[si.skSel]
	and	al,maskSkSel
	cmp	al,skBlock SHL ibitSkSel
	jne	AC45
	errnz	<skSelConst>
	and	[si.skSel],NOT maskSkSel
AC45:
	ret


;/* A D J U S T  S E L S */
;/* adjust selection
;*/
;AdjustSels(psels, cpFirst, cpLim, dcpAdj)
;struct SELS *psels;
;CP cpFirst, cpLim, dcpAdj;
;{
	;LN_AdjustSels takes psel in si, doc in di, and pca, dcpAdjust
	;from AdjustCp's stack.  ax, bx, cx, dx are altered.
PUBLIC LN_AdjustSels
LN_AdjustSels:
	;Assert (psels->doc == doc);
ifdef DEBUG
	cmp	[si.docSel],di
	je	AS003
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn
	mov	bx,1006
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
AS003:
endif ;DEBUG
;	 if (psels->cpFirst >= cpFirst && psels->cpFirst < cpLim ||
;	     psels->fTable && psels->cpLim > cpFirst && psels->cpFirst <= cpLim)
;	coded as (you're gonna love this...):
;	 if (psels->cpFirst <= cpLim &&
;	      (psels->cpFirst != cpLim && psels->cpFirst >= cpFirst
;	      || psels->fTable && psels->cpLim > cpFirst))
;/* part of selection has been deleted */
;		 {
	mov	bx,[pca]
	mov	ax,[bx.LO_cpLimCa]
	mov	dx,[bx.HI_cpLimCa]
	sub	ax,[si.LO_cpFirstSel]
	sbb	dx,[si.HI_cpFirstSel]
	jl	AS01
	or	ax,dx
	jz	AS0045
AS004:
	mov	ax,[si.LO_cpFirstSel]
	mov	dx,[si.HI_cpFirstSel]
	sub	ax,[bx.LO_cpFirstCa]
	sbb	dx,[bx.HI_cpFirstCa]
	jge	AS005
AS0045:
	test	[si.fTableSel],maskfTableSel
	jz	AS01
	mov	ax,[bx.LO_cpFirstCa]
	mov	dx,[bx.HI_cpFirstCa]
	sub	ax,[si.LO_cpLimSel]
	sbb	dx,[si.HI_cpLimSel]
	jge	AS01
AS005:
;		 psels->cpFirst = cpFirst;
;		 psels->cpLim = cpFirst;
;		 psels->cpAnchor = cpFirst;
;		 psels->cpAnchorShrink = cpFirst;
;		 /*psels->sk = skIns; done below */
	mov	ax,[bx.LO_cpFirstCa]
	mov	dx,[bx.HI_cpFirstCa]
	mov	[si.LO_cpFirstSel],ax
	mov	[si.HI_cpFirstSel],dx
	mov	[si.LO_cpLimSel],ax
	mov	[si.HI_cpLimSel],dx
	mov	[si.LO_cpAnchorSel],ax
	mov	[si.HI_cpAnchorSel],dx
	mov	[si.LO_cpAnchorShrinkSel],ax
	mov	[si.HI_cpAnchorShrinkSel],dx
	jmp	AS07

;		 }
;	 else
AS01:

;		 {
	;FUTURE: bradv (bcv): The following two lines appear to be attempting
	;redundant coding of what is being done in AdjustCa, but I can't
	;be sure.  I would love to get rid of this, but I don't dare on
	;the eve of RC2.
;		if (psels->cpLim >= cpFirst && psels->cpLim < cpLim)
;			dcpAdj = cpFirst  - psels->cpLim;
	push	[SEG_dcpAdj]
	push	[OFF_dcpAdj]	;save dcpAdj
	mov	ax,[si.LO_cpLimSel]
	mov	dx,[si.HI_cpLimSel]
	cmp	ax,[bx.LO_cpLimCa]
	push	dx
	sbb	dx,[bx.HI_cpLimCa]
	pop	dx
	jns	AS02
	sub	ax,[bx.LO_cpFirstCa]
	sbb	dx,[bx.HI_cpFirstCa]
	js	AS02
	neg	dx
	neg	ax
	sbb	dx,0
	mov	[OFF_dcpAdj],ax
	mov	[SEG_dcpAdj],dx
AS02:

;		AdjustCa(&psels->ca, psels->doc, cpFirst, cpLim, dcpAdj);
	;LN_AdjustCa can only be used as a near call from AdjustCp
	;or near subroutines of AdjustCp.
	;It takes pca in si, doc in cx, cpFirstDel in di.cpFirstCa,
	;cpLimDel in di.cpLimCa, dcp in AdjustCp's dcpAdj.
	;ax, bx and dx are altered.
	push	si
	push	di
	mov		cx,di
	mov		di,bx
	add		si,(caSel)
	call	LN_AdjustCa
	mov		bx,di
	pop		di
	pop		si
	pop	[OFF_dcpAdj]	;restore dcpAdj
	pop	[SEG_dcpAdj]

	;Assembler note: We are really checking "> cpFirst" here.
;		 if (psels->cpAnchor >= cpFirst + 1)
;			 psels->cpAnchor += dcpAdj;

	mov	ax,[OFF_dcpAdj]
	mov	dx,[SEG_dcpAdj]
	mov	cx,[bx.HI_cpFirstCa]
	mov	bx,[bx.LO_cpFirstCa]
	cmp	bx,[si.LO_cpAnchorSel]
	push	cx
	sbb	cx,[si.HI_cpAnchorSel]
	pop	cx
	jge	AS05
	add	[si.LO_cpAnchorSel],ax
	adc	[si.HI_cpAnchorSel],dx
AS05:

;		 if (psels->cpAnchorShrink >= cpFirst + 1)
;			 psels->cpAnchorShrink += dcpAdj;
	sub	bx,[si.LO_cpAnchorShrinkSel]
	sbb	cx,[si.HI_cpAnchorShrinkSel]
	jge	AS06
	add	[si.LO_cpAnchorShrinkSel],ax
	adc	[si.HI_cpAnchorShrinkSel],dx
AS06:

;		 }
AS07:

;	 /*	 "Correct" behavior is not well-defined here. We have edited
;	 /*	 characters within the selection. To be safe, we turn the
;	 /*	 selection into and insertion point so as to not cause problems
;	 /*	 elsewhere (particulary with table and block selections).
;	 /**/
;	 if (psels->sk != skIns && psels->cpFirst == psels->cpLim)
;		 {
	mov	al,[si.skSel]
	and	al,maskSkSel
	cmp	al,skIns SHL ibitSkSel
	je	AS10
	mov	ax,[si.LO_cpFirstSel]
	mov	dx,[si.HI_cpFirstSel]
	sub	ax,[si.LO_cpLimSel]
	sbb	dx,[si.HI_cpLimSel]
	or	ax,dx
	jne	AS10

;		 psels->sk = skIns;
	and	[si.skSel],NOT maskSkSel
	or	[si.skSel],skIns SHL ibitSkSel

;/* selCur at Mac can happen temporarily because of deletions at the end
;of the doc */
;		 if (psels->cpFirst < CpMacDoc(psels->doc))
;			 SetSelCellBits(psels);
	mov	bx,[si.docSel]
	;***Begin in line CpMacDoc
	;return ((*mpdochdod[doc]->cpMac - 2*ccpEop);
	;LN_PdodDocEdit assumes doc passed in bx and performs
	;PdodDoc(doc).	Only bx is altered.
	call	LN_PdodDocEdit
	mov	ax,2*ccpEop
	cwd
	sub	ax,[bx.LO_cpMacDod]
	sbb	dx,[bx.HI_cpMacDod]
	;***End in line CpMacDoc
	add	ax,[si.LO_cpFirstSel]
	adc	dx,[si.HI_cpFirstSel]
	jge	AS09
	cCall	SetSelCellBits,<si>
AS09:

;		 if (psels == &selCur)
;			 GetSelCurChp(fTrue);
	cmp	si,dataoffset [selCur]
	jne	AS10
	mov	ax,fTrue
	cCall	GetSelCurChp,<ax>

;		 }

AS10:
;}
	ret


;-------------------------------------------------------------------------
;	AdjustCa(pca, doc, cpFirst, dcp)
;-------------------------------------------------------------------------
;/* A D J U S T  C A */
;/* adjust all cp's >= cpFirst by dcp
;*/
;/*  %%Function:AdjustCa %%Owner:peterj %%reviewed: 6/28/89 */
;/*	 We have just replaced [cpFirstDel,cpLimDel) with new text
;/* which has "dcp" more characters than the original. (dcp will
;/* be negative if the replacement was shorter.) We now have some
;/* CA which may have to be adjusted. If the deleted range and the
;/* CA range overlap, then we must leave CA pointing to "safe" CPs.
;/* There are six ways in which the deleted and CA ranges may
;/* intersect.
;/*
;/*	   cp0	  [ inserted )		 .	   -> cpMac
;/*		  .	     | <- dcp -> |
;/*		  [ deleted		 )
;/*   Case:	  .			 .		    Action:
;/*	1   [)	  .			 .	    Do nothing
;/*	2   [	  .)			 .	    cpLim = cpFirstDel
;/*	3   [	  .			 )	    cpLim += dcp
;/*	4	  [)			 .	    set both CPs to cpFirstDel
;/*	5	  [			 )	    cpFirst = cpFirstDel
;/*	6	  .			 [	)   adjust both CPs by dcp
;/*
;/*   Each "case" is represented by the left-most element of the class.
;/*   That is, each '[' can be moved to the right until it hits the ')'.
;/*   If the '[' in the next case begins in the same position, then the ')'
;/*   in the current case can advance to just in front of the ')' in the
;/*   next case; otherwise it can advance arbitrarily.
;/*
;/**/
;AdjustCa(pca, doc, cpFirstDel, cpLimDel, dcp)
;struct CA *pca;
;int doc;
;CP cpFirstDel, cpLimDel, dcp;
;{
	;LN_AdjustCa can only be used as a near call from AdjustCp
	;or near subroutines of AdjustCp.
	;It takes pca in si, doc in cx, cpFirstDel in di.cpFirstCa,
	;cpLimDel in di.cpLimCa, dcp in AdjustCp's dcpAdj.
	;ax, bx and dx are altered.
LN_AdjustCa:
;	 Assert (pca->cpFirst <= pca->cpLim);
;	 Assert((cpLimDel - cpFirstDel) + dcp >= cp0);
	 ;Do these asserts with a call so as not to mess up short jumps.
ifdef DEBUG
	call	ACa06
endif ;DEBUG

;	 if (pca->doc == doc)
;		 {
	cmp	[si.docCa],cx
	jne	ACa05
	push	cx	;save doc
	mov	bx,(cpLimCa) - (cpFirstCa)
ACa01:
;		 if (pca->cpFirst > cpFirstDel)
;			 {
;			 if (pca->cpFirst < cpLimDel)
;				 pca->cpFirst = cpFirstDel;
;			 else
;				 pca->cpFirst += dcp;
;			 }
;		 if (pca->cpLim > cpFirstDel)
;			 {
;			 if (pca->cpLim < cpLimDel)
;				 pca->cpLim = cpFirstDel;
;			 else
;				 pca->cpLim += dcp;
;			 }
	;Assembler note: use AdjustCp's pca->cpFirst for cpFirstDel,
	;AdjustCp's pca->cpLim for cpLimDel, and AdjustCp's
	;dcpAdjust for dcpAdjust.
	mov	ax,[bx+si.LO_cpFirstCa]
	mov	dx,[bx+si.HI_cpFirstCa]
	cmp	[di.LO_cpFirstCa],ax
	mov	cx,[di.HI_cpFirstCa]
	sbb	cx,dx
	jge	ACa04
	cmp	ax,[di.LO_cpLimCa]
	mov	cx,dx
	sbb	cx,[di.HI_cpLimCa]
	jl	ACa02
	add	ax,[OFF_dcpAdj]
	adc	dx,[SEG_dcpAdj]
	jmp	short ACa03
ACa02:
	mov	ax,[di.LO_cpFirstCa]
	mov	dx,[di.HI_cpFirstCa]
ACa03:
	mov	[bx+si.LO_cpFirstCa],ax
	mov	[bx+si.HI_cpFirstCa],dx
ACa04:
	errnz	<(cpFirstCa) - 0>
	errnz	<(cpLimCa) - 4>
	sub	bx,(cpLimCa)-(cpFirstCa)
	jge	ACa01
	pop	cx	;restore doc
ACa05:
	ret


;	 Assert (pca->cpFirst <= pca->cpLim);
;	 Assert((cpLimDel - cpFirstDel) + dcp >= cp0);
	;Assembler note: use AdjustCp's pca->cpFirst for cpFirstDel,
	;AdjustCp's pca->cpLim for cpLimDel, and AdjustCp's
	;dcpAdjust for dcpAdjust.
ifdef DEBUG
ACa06:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[si.LO_cpLimCa]
	mov	dx,[si.HI_cpLimCa]
	sub	ax,[si.LO_cpFirstCa]
	sbb	dx,[si.HI_cpFirstCa]
	jge	ACa07
	mov	ax,midEditn
	mov	bx,1051
	cCall	AssertProcForNative,<ax,bx>
ACa07:
	mov	ax,[di.LO_cpLimCa]
	mov	dx,[di.HI_cpLimCa]
	sub	ax,[di.LO_cpFirstCa]
	sbb	dx,[di.HI_cpFirstCa]
	add	ax,[OFF_dcpAdj]
	adc	dx,[SEG_dcpAdj]
	jge	ACa08
	mov	ax,midEditn
	mov	bx,1052
	cCall	AssertProcForNative,<ax,bx>
ACa08:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG


;-------------------------------------------------------------------------
;	LprgcpForPlc( pplc )
;-------------------------------------------------------------------------
;/* L P R G C P  F O R	P L C */
;native CP far *LprgcpForPlc(pplc)
;struct PLC *pplc;
;{
;#ifdef ENABLE
;	if (pplc->fExternal)
;		return ((CP far *)LpFromHp(HpOfHq(pplc->hqplce)));
;	else
;#endif
;		return ((CP far *)pplc->rgcp);

cProc	LN_LprgcpForPlc,<NEAR,ATOMIC>,<>
;	LN_LprgcpForPlc takes pplc in register bx,
;	and returns the result in registers es:di.
;	bx is not altered, ax, cx, dx are trashed.

cBegin	nogen
;	expect pplc in bx, return HpOfHq(hqplce) in es:di, bx is not altered.
include qplc.asm

;}
cEnd	nogen
	ret

; End of LN_LprgcpForPlc


;-------------------------------------------------------------------------
;	AdjustHplc(hplc, cp, dcpNew, ipcdMin)
;-------------------------------------------------------------------------
;/* A D J U S T  H P L C */
;/* does lazy adjustment of hplc by maintaining an adjustment interval in PLC.
;   the adjustment interval is [pplc->icpAdjust, pplc->iMac]. The amount to
;   bias the rgcp entries in this interval is pplc->dcpAdjust. */
;native AdjustHplc(hplc, cp, dcpNew, icpCertain)
;struct PLC **hplc; CP cp, dcpNew;
;int icpCertain; /* when not = -1, new adjustment index is known. */
;{
;	 struct PLC *pplc;
;	 int icpFenceOld, icpAdjustNew, icpFenceAfter;
;	 CP  dcpOld, dcpAdjustAfter;
;	 int icpAdjustFirst, icpAdjustLim;
;	 CP  dcpAdjust;
;	 int iMac;
;	 CP far *lprgcp;

cProc	AdjustHplc,<PUBLIC,FAR>,<si,di>
	ParmW	hplc
	ParmD	cp
	ParmD	dcpNew
	ParmW	icpCertain

ifdef DEBUG
	LocalW	bxSave
endif ;DEBUG

cBegin

;	 if ((int)hplc == 0 || dcpNew == 0) return;
	mov	bx,[hplc]
	or	bx,bx
	je	Ltemp004
	mov	ax,[OFF_dcpNew]
	or	ax,[SEG_dcpNew]
	je	Ltemp004

;	 pplc = *hplc;
	mov	bx,[bx]

;	 iMac = pplc->iMac;

;/* REVIEW before shipment icpLinSrchThreshold should be set equal to the
;   icpLim below which IInPlc performs linear searches and the following
;   commented clause activated. When IInPlc does linear searches it's just as
;   expensive as AdjustHplcCps() so lazy adjustment doesn't pay.*/
;/*	 if (iMac < icpLinSrchThreshold && icpCertain == -1)
;		 {
;		 if (pplc->icpAdjust <= iMac)
;			 CompletelyAdjustHplcCps(hplc);
;		 AdjustHplcCps(hplc, cp, dcp, (icpCertain != 0) ? icpCertain : 0);
;		 return;
;		 }
;*/

;	 icpFenceOld = pplc->icpAdjust;

;	 dcpOld = pplc->dcpAdjust;

;	 /* this is verbose because of native compiler bug */
;	 if (icpCertain != -1)
;		 icpAdjustNew = icpCertain;
	mov	ax,[icpCertain]
	cmp	ax,-1
	jne	AH01

;	 else if ((icpAdjustNew = IInPlcRef(hplc, cp)) == -1)
;		 return; /* nothing needs to be done */
	cCall	IInPlcRef,<[hplc],[SEG_cp],[OFF_cp]>
	cmp	ax,-1
	jne	AH01
Ltemp004:
	jmp	AH09

AH01:

;	the following line is done below in the assembler version
;	 lprgcp = LprgcpForPlc(pplc);

;	 dcpAdjustAfter = dcpOld;
;	 icpFenceAfter = icpFenceOld;
	xor	cx,cx	    ;clears fUseDcpNew, fUseIcpAdjustNew

;	 dcpAdjust = dcpNew;
	mov	si,[OFF_dcpNew]
	mov	di,[SEG_dcpNew]

;/* REVIEW this is the heuristic which decides whether the rgcp range in the
;   middle of the plc or the rgcp range at the tail of the plc is incremented
;   by cpAdjust. Executing either branch will produce correct results. We may
;   try to develop heuristic which accounts for change of locality of new
;   adjustments. */
;	 if (iMac < 2 * max(icpFenceOld, icpAdjustNew) -
;		 min(icpFenceOld, icpAdjustNew))
;		 {
	mov	dx,[bx.icpAdjustPlc]
	push	ax
	cmp	dx,ax
	jge	AH02
	xchg	ax,dx
AH02:
	shl	dx,1
	sub	dx,ax
	pop	ax
	cmp	[bx.iMacPlcSTR],dx
	jge	AH03

;/* this branch executed when we are going to execute AddDcpToCps for the
;   tail of the plc. */

;		 icpAdjustLim = iMac + 1;
	mov	dx,[bx.iMacPlcSTR]
	inc	dx

;		 if (icpAdjustNew <= icpFenceOld)
;			 {
	cmp	ax,[bx.icpAdjustPlc]
	jg	AH05

;			 dcpAdjustAfter = dcpNew;
;			 icpFenceAfter = icpAdjustNew;
	dec	cx	    ;sets fUseDcpNew, fUseIcpAdjustNew

;			 dcpAdjust = dcpOld;
	mov	si,[bx.LO_dcpAdjustPlc]
	mov	di,[bx.HI_dcpAdjustPlc]
;			}
;		}
	jmp	short AH05

AH03:
;	 else
;		 {
;/* this branch executed when we are going to execute AddDcpToCps for the
;   middle part of the plc. The tail will be the new adjustment interval.
;   It's dcp will be the sum of the new and old dcp. */

;		 dcpAdjustAfter = dcpOld + dcpNew;
	mov	dx,[bx.LO_dcpAdjustPlc]
	add	[OFF_dcpNew],dx
	mov	dx,[bx.HI_dcpAdjustPlc]
	adc	[SEG_dcpNew],dx
	inc	ch	    ;sets fUseDcpNew

;		 if (icpAdjustNew <= icpFenceOld)
	cmp	ax,[bx.icpAdjustPlc]
	jg	AH04

;			  icpAdjustLim = icpFenceOld;
	mov	dx,[bx.icpAdjustPlc]
	jmp	short AH05

AH04:
;		 else
;			 {
;			 icpAdjustLim = icpAdjustNew;
	mov	dx,ax

;			 dcpAdjust = dcpOld;
	mov	si,[bx.LO_dcpAdjustPlc]
	mov	di,[bx.HI_dcpAdjustPlc]

;			 icpFenceAfter = icpAdjustNew;
	inc	cx	    ;sets fUseIcpAdjustNew
;			 }
;		 }

AH05:
	push	dx	;save icpAdjustLim
;	 icpAdjustFirst = icpAdjustNew + icpFenceOld -
;		 (pplc->icpAdjust = icpFenceAfter);
;	NOTE: we have
;	icpAdjustNew in ax
;	icpFenceOld in pplc->icpAdjust
;	icpFenceAfter in cl ? ax : pplc->icpAdjust
;	==> icpAdjustFirst = cl ? pplc->icpAdjust : ax
	or	cl,cl	    ;test fUseIcpAdjustNew
	je	AH06
	xchg	ax,[bx.icpAdjustPlc]
AH06:

;	 pplc->dcpAdjust = dcpAdjustAfter;
	or	ch,ch	    ;test fUseDcpNew
	je	AH07
	mov	dx,[OFF_dcpNew]
	mov	[bx.LO_dcpAdjustPlc],dx
	mov	dx,[SEG_dcpNew]
	mov	[bx.HI_dcpAdjustPlc],dx
AH07:

	push	ax	    ;save icpAdjustFirst
	push	di
	push	si	    ;save dcpAdjust

;	 lprgcp = LprgcpForPlc(pplc);
ifdef DEBUG
	mov	[bxSave],bx
endif ;DEBUG
;	LN_LprgcpForPlc takes pplc in register bx,
;	and returns the result in registers es:di.
;	bx is not altered, ax, cx, dx are trashed.
	cCall	LN_LprgcpForPlc,<>
ifdef DEBUG
;    Assert (bx == bxSave);
	cmp	bx,[bxSave]
	je	AH08
	push	ax
	push	bx
	push	cx
	push	dx
	push	es

	mov	ax,midEditn
	mov	bx,1007
	cCall	AssertProcForNative,<ax,bx>

	pop	es
	pop	dx
	pop	cx
	pop	bx
	pop	ax
AH08:
endif ;DEBUG
	pop	ax
	pop	dx	    ;restore dcpAdjust
	pop	bx	    ;restore icpAdjustFirst
	pop	cx	    ;restore icpAdjustLim

;	 AddDcpToCps(lprgcp, icpAdjustFirst, icpAdjustLim, dcpAdjust);
;	LN_AddDcpToCps takes lprgcp in es:di, icpFirst in bx,
;	icpLim in cx, and dcp in dx:ax
	cCall	LN_AddDcpToCps,<>
AH09:
;}
cEnd

; End of AdjustHplc


;-------------------------------------------------------------------------
;	CompletelyAdjustHplcCps(hplc)
;-------------------------------------------------------------------------
;/*  C O M P L E T E L Y  A D J U S T	H P L C  C P S */
;/* adds pplc->dcpAdjust to rgcp entries in interval
;   [pplc->icpAdjust, pplc->iMac] and invalidates the adjustment interval
;   recorded in plc. Should be called for each document's plcs when we
;   return to the Idle loop. */
;CompletelyAdjustHplcCps(hplc)
;struct PLC **hplc;
;{
; %%Function:CompletelyAdjustHplcCps %%Owner:BRADV
cProc	CompletelyAdjustHplcCps,<PUBLIC,FAR>,<>
	ParmW	hplc

cBegin

;	 AdjustHplcCpsToLim(hplc, (*hplc)->iMac + 1);
	mov	bx,[hplc]
	push	bx
	mov	bx,[bx]
	mov	ax,[bx.iMacPlcSTR]
	inc	ax
	push	ax
	push	cs
	call	near ptr AdjustHplcCpsToLim
cEnd

;}

; End of CompletelyAdjustHplcCps


;-------------------------------------------------------------------------
;	AdjustHplcCpsToLim(hplc, icpLim)
;-------------------------------------------------------------------------
;/* A D J U S T  H P L C  C P S  T O  L I M */
;/* adds pplc->dcpAdjust to rgcp entries in the interval
;   [pplc->icpAdjust, icpLim). if rgcp[iMac] was adjusted we invalidate the
;   plc's adjustment interval. */
;AdjustHplcCpsToLim(hplc, icpLim)
;struct PLC **hplc;
;int icpLim;
;{
;	 struct PLC *pplc;

; %%Function:AdjustHplcCpsToLim %%Owner:BRADV
cProc	AdjustHplcCpsToLim,<PUBLIC,FAR>,<si,di>
	ParmW	hplc
	ParmW	icpLim

ifdef DEBUG
	LocalW	bxSave
endif ;DEBUG

cBegin

;	 if (hplc == 0 || (*hplc)->dcpAdjust == cp0)
;		 return;
	mov	bx,[hplc]
	or	bx,bx
	je	AHCTL02
	mov	bx,[bx]
	mov	ax,[bx.LO_dcpAdjustPlc]
	or	ax,[bx.HI_dcpAdjustPlc]
	je	AHCTL02

;	 pplc = *hplc;

;	 AddDcpToCps(LprgcpForPlc(pplc), pplc->icpAdjust, icpLim, pplc->dcpAdjust);
ifdef DEBUG
	mov	[bxSave],bx
endif ;DEBUG
;	LN_LprgcpForPlc takes pplc in register bx,
;	and returns the result in registers es:di.
;	bx is not altered, ax, cx, dx are trashed.
	cCall	LN_LprgcpForPlc,<>
ifdef DEBUG
;    Assert (bx == bxSave);
	cmp	bx,[bxSave]
	je	AHCTL01
	push	ax
	push	bx
	push	cx
	push	dx
	push	es

	mov	ax,midEditn
	mov	bx,1008
	cCall	AssertProcForNative,<ax,bx>
	pop	es
	pop	dx
	pop	cx
	pop	bx
	pop	ax
AHCTL01:
endif ;DEBUG
;	LN_AddDcpToCps takes lprgcp in es:di, icpFirst in bx,
;	icpLim in cx, and dcp in dx:ax
	push	bx	;save pplc
	mov	ax,[bx.LO_dcpAdjustPlc]
	mov	dx,[bx.HI_dcpAdjustPlc]
	mov	bx,[bx.icpAdjustPlc]
	mov	cx,[icpLim]
	cCall	LN_AddDcpToCps,<>
	pop	bx	;restore pplc

;	if ((pplc->icpAdjust = icpLim) == pplc->iMac + 1)
;		pplc->dcpAdjust = cp0;

	mov	ax,[icpLim]
	mov	[bx.icpAdjustPlc],ax
	sub	ax,[bx.iMacPlcSTR]
	dec	ax
	jne	AHCTL02
	;Assembler note: We know ax is zero at this point.
	mov	[bx.LO_dcpAdjustPlc],ax
	mov	[bx.HI_dcpAdjustPlc],ax

AHCTL02:
;}
cEnd

; End of AdjustHplcCpsToLim


ifdef NOTUSED
;-------------------------------------------------------------------------
;	AdjustHplcCps(hplc, cp, dcp, ipcdMin)
;-------------------------------------------------------------------------
;/* A D J U S T  H P L C  C P S  */
;/* hand native */
;/* adjusts all cp's with i >= ipcdMin from (>=) cp by dcp. Backward scan
;is for efficiency */
;AdjustHplcCps(hplc, cp, dcp, ipcdMin)
;struct PLC **hplc;
;CP cp;
;CP dcp;
;int ipcdMin;
;{
;	 CP far *lprgcp;
;	 CP far *lpcp, far *lpcpMin;
;	 struct PLC *pplc;

; %%Function:AdjustHplcCps %%Owner:BRADV
cProc	AdjustHplcCps,<PUBLIC,FAR>,<si,di>
	ParmW	hplc
	ParmD	cp
	ParmD	dcp
	ParmW	ipcdMin

ifdef DEBUG
	LocalW	bxSave
endif ;DEBUG

cBegin

;	 if ((int)hplc == 0 || dcp == 0) return;
	mov	bx,[hplc]
	or	bx,bx
	je	AHC04
	mov	bx,[bx]
	mov	ax,[OFF_dcp]
	or	ax,[SEG_dcp]
	je	AHC04

;	 pplc = *hplc;

;	 lprgcp = LprgcpForPlc(pplc);
ifdef DEBUG
	mov	bxSave,bx
endif ;DEBUG
;	LN_LprgcpForPlc takes pplc in register bx,
;	and returns the result in registers es:di.
;	bx is not altered, ax, cx, dx are trashed.
	cCall	LN_LprgcpForPlc,<>
ifdef DEBUG
;    Assert (bx == bxSave);
	cmp	bx,[bxSave]
	je	AHC01
	push	ax
	push	bx
	push	cx
	push	dx
	push	es

	mov	ax,midEditn
	mov	bx,1009
	cCall	AssertProcForNative,<ax,bx>
	pop	es
	pop	dx
	pop	cx
	pop	bx
	pop	ax
AHC01:
endif ;DEBUG

;	 lpcpMin = &lprgcp[ipcdMin];
	mov	si,[ipcdMin]
	shl	si,1
	shl	si,1
	add	si,di

;	 lpcp = &lprgcp[pplc->iMac];
	mov	ax,[bx.iMacPlcSTR]
	shl	ax,1
	shl	ax,1
	add	di,ax

;	 while (lpcp >= lpcpMin && *lpcp >= cp)
;		 *(lpcp--) += dcp;
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	mov	bx,[OFF_dcp]
	mov	cx,[SEG_dcp]
	jmp	short AHC03

AHC02:
	add	es:[di],bx
	adc	es:[di+2],cx
	sub	di,4

AHC03:
	cmp	di,si
	jb	AHC04
	cmp	es:[di+2],dx
	jg	AHC02
	jl	AHC04
	cmp	es:[di],ax
	jae	AHC02

AHC04:
;}
cEnd

; End of AdjustHplcCps
endif ;NOTUSED


;-------------------------------------------------------------------------
;	AddDcpToCps(lprgcp, icpFirst, icpLim, dcp)
;-------------------------------------------------------------------------
;/* A D D  D C P  T O  C P S  */
;/* add dcp to all entries of lprgcp within the interval [icpFirst, icpLim). */
;AddDcpToCps(lprgcp, icpFirst, icpLim, dcp)
;CP far *lprgcp;
;int icpFirst;
;int icpLim;
;CP dcp;
;{
;	 CP far *lpcp, lpcpLim;

; %% Function:LN_AddDcpToCps %%Owner:BRADV
PUBLIC LN_AddDcpToCps
LN_AddDcpToCps:
;	LN_AddDcpToCps takes lprgcp in es:di, icpFirst in bx,
;	icpLim in cx, and dcp in dx:ax


;	 for (lpcp = &lprgcp[icpFirst], lpcpLim = &lprgcp[icpLim];
	mov	si,bx
	shl	si,1
	shl	si,1
	add	di,si
	sub	cx,bx
	jle	ADTC03
ADTC02:

;		 lpcp < lpcpLim;  lpcp++)
;		 *lpcp += dcp;
	add	es:[di],ax
	adc	es:[di+2],dx
	add	di,4
	loop	ADTC02

ADTC03:
;}
	ret

; End of LN_AddDcpToCps


;-------------------------------------------------------------------------
;	AdjustHplcedlCps(hplcedl, cp, dcp)
;-------------------------------------------------------------------------
;/* A D J U S T  H P L C E D L	C P S  */
;/* hand native */
;/* adjusts all cp's with i >= 0 from (>=) cp by dcp. Backward scan
;is for efficiency. */
;HANDNATIVE AdjustHplcedlCps(hplcedl, cp, dcp)
;struct PLCEDL **hplcedl;
;CP cp;
;CP dcp;
;{
;	 CP cpAdjust;
;	 int dl;
;	 struct EDL edl;
;	 struct DR *pdr;
;	 struct DRF drfFetch;

ifdef DEBUG
; %%Function:N_AdjustHplcedlCps %%Owner:BRADV
cProc	N_AdjustHplcedlCps,<PUBLIC,FAR>,<si,di>
else ;!DEBUG
; %%Function:LN_AdjustHplcedlCps %%Owner:BRADV
cProc	LN_AdjustHplcedlCps,<PUBLIC,NEAR>,<si,di>
endif ;!DEBUG
	ParmW	hplcedl
	ParmD	cp
	ParmD	dcp

	LocalV	edl,cbEdlMin
	LocalV	drfFetch,cbDrfMin

cBegin

;	if ((int)hplcedl == 0 || dcp == 0)
;		return;
	mov	si,[OFF_dcp]
	or	si,[SEG_dcp]
	je	AHedlC06
	mov	si,[hplcedl]
	or	si,si
	je	AHedlC06

;	cpAdjust = CpMacPlc(hplcedl);
;	dl = IMacPlc(hplcedl);
;	goto LTest;
	mov	di,[si]
	mov	di,[di.iMacPlcStr]
	jmp	short AHedlC04

;	while (dl >= 0)
;		{
AHedlC01:

;/* note: the first entry which is not adjusted may be still adjusted in its
;interior. Eg. entry is from [2, 4), adjust >= 3, still has to adjust what is
;in the inside. */
;		GetPlc(hplcedl, dl, &edl);
	lea	ax,[edl]
	cCall	GetPlc,<si, di, ax>

;		if (edl.hpldr != hNil)
;			{
	push	si	;save hplcedl
	push	di	;save dl

;			int idr;
;			int idrMac = (*((struct PLDR **)edl.hpldr))->idrMac;
;			struct DR HUGE *hpdr = HpInPl(edl.hpldr, 0);
	mov	si,[edl.hpldrEdl]
	or	si,si
	je	AHedlC03
	mov	di,[si]
	mov	di,[di.iMacPl]

;			Assert((*((struct PLDR **)edl.hpldr))->fExternal);
;			for (idr = 0; idr < idrMac; hpdr++, idr++)
;				{
AHedlC02:
	dec	di
	jns	AHedlC07
	;Assembler note: I don't think this recoding of PdrFetch and
	;AdjustHplc buys us much.
	;Assembler note: Jump to AHedlC07 here to avoid messing up short
	;jumps.

;				CP HUGE *hpcp, HUGE *hpcpMin;
;				Assert(hpdr->hplcedl != hNil);
;				if (hpdr->cpFirst >= cp)
;					{
;					if (vpdrfHead == NULL)
;						hpdr->cpFirst += dcp;
;					else
;						{
;						pdr = PdrFetch(edl.hpldr, idr, &drfFetch);
;						pdr->cpFirst += dcp;
;						FreePdrf(&drfFetch);
;						}
;					}
;				Assert(hpdr->plcedl.fExternal);
;				hpcpMin = HpOfHq(((struct PLC HUGE *)&hpdr->plcedl)->hqplce);
;				hpcp = &hpcpMin[hpdr->plcedl.dlMac];
;/* copied from AdjustHplcCps */
;				while (hpcp >= hpcpMin && *hpcp >= cp)
;					*(hpcp--) += dcp;
;				}
;			}
AHedlC03:
	pop	di	;restore dl
	pop	si	;restore hplcedl


;		cpAdjust = CpPlc(hplcedl, dl);
;LTest:
AHedlC04:
	cCall	CpPlc,<si, di>

;		if (cpAdjust < cp)
;			break;
	mov	bx,[OFF_cp]
	mov	cx,[SEG_cp]
	cmp	ax,bx
	push	dx
	sbb	dx,cx
	pop	dx
	jl	AHedlC06

;		cpAdjust = CpPlc(hplcedl, dl) + dcp;
	add	ax,[OFF_dcp]
	adc	dx,[SEG_dcp]

;		if (cpAdjust < cp - 1)
;			cpAdjust = cp - 1;
	sub	bx,1
	sbb	cx,0
	cmp	ax,bx
	push	dx
	sbb	dx,cx
	pop	dx
	jge	AHedlC05
	xchg	ax,bx
	mov	dx,cx
AHedlC05:

;		PutCpPlc(hplcedl, dl--, cpAdjust);
	cCall	PutCpPlc,<si, di, dx, ax>


;		}
	dec	di
	jns	AHedlC01
AHedlC06:

;}
cEnd

AHedlC07:
	lea	ax,[drfFetch]
	push	ax	;argument for FreePdrf
ifdef DEBUG
	cCall	S_PdrFetch,<si,di,ax>
else ;!DEBUG
	cCall	N_PdrFetch,<si,di,ax>
endif ;!DEBUG
	xchg	ax,bx
	push	[bx.hplcedlDr]	;argument for CompletelyAdjustHplcCps
	push	[bx.hplcedlDr]
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	push	dx
	push	ax
	cmp	[bx.LO_cpFirstDr],ax
	mov	cx,[bx.HI_cpFirstDr]
	sbb	cx,dx
	mov	ax,[OFF_dcp]
	mov	dx,[SEG_dcp]
	push	dx
	push	ax
	jl	AHedlC08
	add	[bx.LO_cpFirstDr],ax
	adc	[bx.HI_cpFirstDr],dx
AHedlC08:
	mov	ax,-1
	push	ax
	push	cs
	call	near ptr AdjustHplc
	;REVIEW - This call should be unnecessary, in that dcpAdjust != 0
	;should be allowed.  But I fail an assert that cp == caTap.cpLim in
	;FUpdateTable if I don't call CompletelyAdjustHplcCps.
	push	cs
	call	near ptr CompletelyAdjustHplcCps
ifdef DEBUG
	cCall	S_FreePdrf,<>
else ;!DEBUG
	cCall	N_FreePdrf,<>
endif ;!DEBUG
	jmp	AHedlC02

; End of N_AdjustHplcedlCps


;/* P X S  I N I T */
;/* initialize xbc for an Xaction and return the first pxs */
;char *PxsInit(pxs, ixsMax, pxbc)
;char *pxs;
;int ixsMax;
;struct XBC *pxbc;
;{
;    pxbc->ixsMax = ixsMax;
;    pxbc->ixsMac = 1;
;    pxbc->itnMac = 0;
;    pxbc->rgxs = pxs;
;    SetBytes(pxs, 0, ixsMax*cbXSR);
;    return pxs;
;}

ifdef DEBUG
; %%Function:N_PxsInit %%Owner:BRADV
cProc	N_PxsInit,<PUBLIC,FAR>,<si,di>
	ParmW	pxs
	ParmW	ixsMax
	ParmW	pxbc

cBegin
	mov	si,[pxs]
	mov	cx,[ixsMax]
	mov	bx,[pxbc]
	call	LN_PxsInit
	xchg	ax,si
cEnd
endif ;DEBUG

	;LN_PxsInit takes pxs in si, ixsMax in cx, pxbc in bx
	;and performs PxsInit(pxs, ixsMax, pxbc).
	;ax, bx, cx, dx, di are altered.  The result is returned in si.
; %%Function: LN_PxsInit %%Owner:BRADV
PUBLIC LN_PxsInit
LN_PxsInit:
	mov	[bx.ixsMaxXbc],cx
	mov	[bx.rgxsXbc],si
	push	ds
	pop	es
	mov	di,si
	errnz	<cbXsrMin AND 1>
	mov	ax,cbXsrMin SHR 1
	mul	cx
	xchg	ax,cx
	xor	ax,ax
	rep	stosw
	mov	[bx.itnMacXbc],ax
	inc	ax
	mov	[bx.ixsMacXbc],ax
	ret

;/* P O S T  T N */
;/* add an intention to the intentions list in xbc */
;PostTn(pxbc, tnt, h, c)
;struct XBC *pxbc;
;int tnt;
;void **h; /* meaning of h and c depend on tnt */
;int c;
;{

; %%Function:N_PostTn %%Owner:BRADV
cProc	N_PostTn,<PUBLIC,FAR>,<>
	ParmW	pxbc
	ParmW	tnt
	ParmW	hArg
	ParmW	cArg

cBegin
	mov	bx,[pxbc]
	mov	ax,[tnt]
	mov	cx,[cArg]
	mov	dx,[hArg]
	call	LN_PostTn
cEnd

	;LN_PostTn takes pxbc in bx, tnt in ax, c in cx, h in dx and
	;performs PostTn(pxbc, tnt, h, c).
	;ax, bx, cx, dx are altered.
; %%Function:LN_PostTn %%Owner:BRADV
PUBLIC LN_PostTn
LN_PostTn:
;    struct TN *ptn = &pxbc->rgtn[pxbc->itnMac++];
;   Assert(pxbc->itnMac <= itnMax);
ifdef DEBUG
	cmp	[bx.itnMacXbc],itnMax
	jl	PT01	    ;not jle because itnMac has not been incremented.
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn
	mov	bx,1010
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
PT01:
endif ;/* DEBUG */
	push	si	;save caller's si
	errnz	<cbTnMin - 6>
	mov	si,[bx.itnMacXbc]
	shl	si,1
	add	si,[bx.itnMacXbc]
	shl	si,1
	inc	[bx.itnMacXbc]
	lea	bx,[bx+si.rgtnXbc]
	pop	si	;restore caller's si

;#ifdef DEBUG
;   switch(tnt)
;	{
;	case tntAbort:
;	case tntNil:
;	    break;
;	case tntHplc:
;	case tntHsttb:
;	    AssertH(h);
;	    break;
;	case tntCloseHdrWw:
;	    Assert((int)h >= wwDocMin && (int)h < wwMac);
;	    Assert(PwwdWw((int)h)->wk == wkHdr);
;	    break;
;	case tntDelBkmk:
;	    Assert(!PdodDoc((int)h/*doc*/)->fShort);
;	    Assert(c == 0);
;	    break;
;	default:
;	    Assert(fFalse);
;	}
;#endif /* DEBUG */
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	ax,tntAbort
	je	Ltemp005
	cmp	ax,tntNil
	je	Ltemp005
	cmp	ax,tntHplc
	je	PT02
	cmp	ax,tntHsttb
	jne	PT03
PT02:
	mov	ax,midEditn
	mov	bx,1011
	cCall	AssertHForNative,<dx, ax, bx>
	jmp	short PT09
PT03:
	cmp	ax,tntCloseHdrWw
	jne	PT06
	cmp	dx,wwDocMin
	jl	PT04
	cmp	dx,[wwMac]
	jl	PT05
PT04:
	mov	ax,midEditn
	mov	bx,1012
	cCall	AssertProcForNative,<ax,bx>
Ltemp005:
	jmp	short PT09
PT05:
	cCall	N_PwwdWw,<dx>
	xchg	ax,bx
	cmp	[bx.wkWwd],wkHdr
	je	PT09
	mov	ax,midEditn
	mov	bx,1013
	cCall	AssertProcForNative,<ax,bx>
	jmp	short PT09
PT06:
	cmp	ax,tntDelBkmk
	jne	PT08
	mov	bx,dx
	;LN_PdodDocEdit assumes doc passed in bx and performs
	;PdodDoc(doc).	Only bx is altered.
	call	LN_PdodDocEdit
	cmp	[bx.fShortDod],fFalse
	jne	PT07
	jcxz	PT09
PT07:
	mov	ax,midEditn
	mov	bx,1030
	cCall	AssertProcForNative,<ax,bx>
	jmp	short PT09
PT08:
	mov	ax,midEditn
	mov	bx,1031
	cCall	AssertProcForNative,<ax,bx>
PT09:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;   ptn->tnt = tnt;
;   ptn->h = h;
;   ptn->c = c;
;}
	mov	[bx.tntTn],ax
	mov	[bx.hTn],dx
	mov	[bx.cTn],cx
	ret

    
;/* F  D O  T N S */
;/* "execute" the intentions stored in rgtn */
;FDoTns(pxbc)
;struct XBC *pxbc;
;{
;   int c, cMax;
;   struct TN *ptn, *ptnT, *ptnMac;
;   void **h;
;   int tnt;
;   BOOL fReturn = fFalse;

ifdef DEBUG
; %%Function:N_FDoTns %%Owner:BRADV
cProc	N_FDoTns,<PUBLIC,FAR>,<di>
	ParmW	pxbc

cBegin
	mov	di,[pxbc]
	call	LN_FDoTns
	sbb	ax,ax
cEnd
endif ;DEBUG

	;LN_FDoTns takes pxbc in di and performs FDoTns(pxbc).
	;fTrue is returned iff carry is true upon return.
	;ax, bx, cx, dx are altered.
; %%Function:LN_FDoTns %%Owner:BRADV
PUBLIC LN_FDoTns
LN_FDoTns:
	push	si	;save caller's si

;   for (ptn = pxbc->rgtn, ptnMac = ptn + pxbc->itnMac; ptn < ptnMac; ptn++)
	lea	si,[di.rgtnXbc - cbTnMin]
	mov	ax,cbTnMin
	mul	[di.itnMacXbc]
	xchg	ax,bx
	lea	dx,[bx+di.rgtnXbc]
FDT01:
	add	si,cbTnMin
	cmp	si,dx
	jae	FDT07

;	switch (tnt = ptn->tnt)
;	    {
;	    default:
;		Assert(fFalse);
;		/* fall through */
;	    case tntNil:
;	    case tntCloseHdrWw:
;	    case tntDelBkmk:
;		continue;
;	    case tntAbort:
;		goto LReturn;
;	    case tntHplc:
;	    case tntHsttb:
	mov	ax,[si.tntTn]
	cmp	al,tntNil
	je	FDT01
	cmp	al,tntCloseHdrWw
	je	FDT01
	cmp	al,tntDelBkmk
	je	FDT01
	cmp	al,tntAbort
	je	FDT08
ifdef DEBUG
	cmp	al,tntHplc
	je	FDT02
	cmp	al,tntHsttb
	je	FDT02
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn
	mov	bx,1015
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	jmp	short FDT01
FDT02:
endif ;/* DEBUG */

;/* coalesce the several intentions to allocate into a single
;maximal intention.  (i.e., determine the largest that this heap
;block will get during the course of the operation by accumlating
;all of the intentions that relate to this heap block and taking
;the max that that value obtains). */
	;ax = tnt, dx = ptnMac, si = ptn
;		c = cMax = ptn->c;
;		h = ptn->h;
	push	di	;save pxbc
	push	si	;save ptn
	mov	cx,[si.cTn]
	mov	di,cx
	mov	bx,[si.hTn]

;		for (ptnT = ptn + 1; ptnT < ptnMac; ptnT++)
FDT03:
	add	si,cbTnMin
	cmp	si,dx
	jae	FDT04

;		    if (ptnT->tnt == tnt && ptnT->h == h)
;			{
	cmp	[si.tntTn],ax
	jne	FDT03
	cmp	[si.hTn],bx
	jne	FDT03

;			if ((c += ptnT->c) > cMax)
;			    cMax = c;
;			/* to avoid double counting */
;			ptnT->tnt = tntNil;
	mov	[si.tntTn],tntNil
	add	di,[si.cTn]
	cmp	di,cx
	jle	FDT03
	mov	cx,di
	jmp	short FDT03

;			}
FDT04:
	pop	si	;restore ptn
	pop	di	;restore pxbc

;/* do allocation, if necessary. fail process if allocation cannot be made */
;		if (cMax > 0)
	or	cx,cx
	jle	FDT01

;		    if (tnt == tntHplc && !FStretchPlc(h, cMax))
;			goto LReturn;
;		    else if (tnt == tntHsttb && !FStretchSttbCb(h, cMax))
;			goto LReturn;
;		break;
;	    }
	push	dx	;save ptnMac
	push	bx
	push	cx
	cmp	al,tntHplc
	jne	FDT05
ifdef DEBUG
	cCall	S_FStretchPlc,<>
else ;not DEBUG
	cCall	N_FStretchPlc,<>
endif ;DEBUG
	jmp	short FDT06
FDT05:
	cCall	FStretchSttbCb,<>
FDT06:
	pop	dx	;restore ptnMac
	or	ax,ax
	jne	FDT01
	db	0A8h	;turns next "stc" into "test al,immediate"
			;also clears the carry flag

FDT07:

;   fReturn = fTrue;
;
;/* reset state in preperation for !fPlan phase */
;   pxbc->ixsMac = 1; /* reset to first xs */
	stc
	mov	[di.ixsMacXbc],1

;LReturn:
;/* failure or success - don't need to handle this prm special anymore */
;   vmerr.prmDontReclaim = prmNil;
;
;   return fReturn;
FDT08:
	mov	[vmerr.prmDontReclaimMerr],prmNil

;}
	pop	si	;restore caller's si
	ret


;/* C L O S E  T N S */
;/* Close up unneeded space in each hplc or hsttb used by pxbc.
;*/
;CloseTns(pxbc)
;struct XBC *pxbc;
;{
;   struct TN *ptn, *ptnMac;

ifdef DEBUG
; %%Function:N_CloseTns %%Owner:BRADV
cProc	N_CloseTns,<PUBLIC,FAR>,<di>
	ParmW	pxbc

cBegin
	mov	di,[pxbc]
	call	LN_CloseTns
cEnd
endif ;DEBUG

	;LN_CloseTns takes pxbc in di and performs CloseTns(pxbc)
	;ax, bx, cx, dx are altered.
; %%Function:LN_CloseTns Owner:BRADV
PUBLIC LN_CloseTns
LN_CloseTns:
;   Assert(!vfInCommit);
ifdef DEBUG
	cmp	[vfInCommit],0
	je	CT01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn
	mov	bx,1016
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
CT01:
endif ;DEBUG

	push	si	;save caller's si

;   for (ptn = pxbc->rgtn, ptnMac = ptn + pxbc->itnMac; ptn < ptnMac; ptn++)
	lea	si,[di.rgtnXbc - cbTnMin]
	mov	ax,cbTnMin
	mul	[di.itnMacXbc]
	xchg	ax,bx
	lea	dx,[bx+di.rgtnXbc]
CT02:
	add	si,cbTnMin
	cmp	si,dx
	jae	CT10

;	switch (ptn->tnt)
;	    {
;#ifdef DEBUG
;	    case tntAbort:
;	    default:
;		Assert(fFalse);
;	    case tntNil:
;		break;
;#endif /* DEBUG */
	mov	ax,[si.tntTn]
	cmp	al,tntNil
	je	CT02
ifdef DEBUG
	jmp	short CT11
CT03:
endif ;/* DEBUG */

;	    case tntHplc:
;		CloseUpPlc(ptn->h);
;		break;
;	    case tntHsttb:
;		CloseUpSttb(ptn->h);
;		break;
;	    case tntCloseHdrWw:
;		DisposeWwHdr(ptn->h);
;		break;
;	    case tntDelBkmk:
;		/* if all bookmarks were deleted, nuke structs */
;		{
;		struct DOD *pdod = PdodDoc((int)ptn->h/*doc*/);
;		FreezeHp();
;		Assert(!pdod->fShort);
;		if (pdod->hplcbkf != hNil && IMacPlc(pdod->hplcbkf) == 0)
;		    {
;		    FreePhsttb(&pdod->hsttbBkmk);
;		    FreePhplc(&pdod->hplcbkf);
;		    FreePhplc(&pdod->hplcbkl);
;		    }
;		MeltHp();
;		break;
;		}
	push	dx	;save ptnMac
	mov	bx,[si.hTn]
	cmp	al,tntHplc
	jne	CT04
	call	LN_CloseUpPlc
	jmp	short CT09
CT04:
	cmp	al,tntDelBkmk
	jne	CT07
	;LN_PdodDocEdit assumes doc passed in bx and performs
	;PdodDoc(doc).	Only bx is altered.
	call	LN_PdodDocEdit
ifdef	DEBUG
	call	LN_FreezeHpEdit
endif ;DEBUG
ifdef DEBUG
	cmp	[bx.fShortDod],fFalse
	je	CT05
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn
	mov	cx,1037
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
CT05:
endif ;/* DEBUG */
	mov	cx,[bx.hplcbkfDod]
	jcxz	CT06
	xchg	bx,cx
	mov	bx,[bx]
	cmp	[bx.iMacPlcStr],0
	jne	CT06
	cCall	CloseBkmkStructs,<cx>
CT06:
ifdef	DEBUG
	call	LN_MeltHpEdit
endif ;DEBUG
	jmp	short CT09
CT07:
	push	bx
	cmp	al,tntHsttb
	jne	CT08
	cCall	CloseUpSttb,<>
	jmp	short CT09
CT08:
	cCall	DisposeWwHdr,<>
CT09:
	pop	dx	;restore ptnMac

;	    }
	jmp	short CT02

;}
CT10:
	pop	si	;restore caller's si
	ret

;#ifdef DEBUG
;	    case tntAbort:
;	    default:
;		Assert(fFalse);
;	    case tntNil:
;		break;
;#endif /* DEBUG */
ifdef DEBUG
CT11:
	cmp	al,tntHplc
	je	CT12
	cmp	al,tntHsttb
	je	CT12
	cmp	al,tntCloseHdrWw
	je	CT12
	cmp	al,tntDelBkmk
	je	CT12
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn
	mov	bx,1017
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	jmp	CT02
CT12:
	jmp	CT03
endif ;/* DEBUG */


;/* C L O S E  U P  P L C */
;/* Remove unneeded space from hplc.
;*/
;CloseUpPlc(hplc)
;struct PLC **hplc;
;{
;	struct PLC *pplc = *hplc;
;	long lcb = (long)(pplc->iMax-1)*(pplc->cb+sizeof(CP)) + sizeof(CP);
;
;	AssertH(hplc);
;	Assert(!vfInCommit);
; %%Function:LN_CloseUpPlc %%Owner:BRADV
PUBLIC LN_CloseUpPlc
LN_CloseUpPlc:
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn
	mov	cx,1018
	cCall	AssertHForNative,<bx, ax, cx>
	cmp	[vfInCommit],0
	je	CUP01
	mov	ax,midEditn
	mov	bx,1019
	cCall	AssertProcForNative,<ax,bx>
CUP01:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */
	mov	cx,bx
;	long lcb = (long)(pplc->iMax-1)*(pplc->cb+sizeof(CP)) + sizeof(CP);
	mov	bx,[bx] ;compute pplc
	mov	ax,[bx.iMaxPlc]
	dec	ax
	mov	dx,[bx.cbPlc]
	add	dx,4
	mul	dx
	add	ax,4

;	if (pplc->fExternal)
;		{
	;Assembler note: assert we have an external plc here because no
	;one creates internal PLC's anymore.
ifdef DEBUG
	test	[bx.fExternalPlc],maskfExternalPlc
	jne	CUP015
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn
	mov	cx,1037
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
CUP015:
endif ;/* DEBUG */

;		HQ hq = pplc->hqplce;
	mov	dx,[bx.LO_hqplcePlc]
	mov	bx,[bx.HI_hqplcePlc]
	push	bx
	push	dx	;save hq in memory
	push	cx	;save hplc

;		if (CbOfHq(hq) > lcb+20)
;#define CbOfHq(hq)	 (*((uns HUGE *)HpOfHq(hq)-1))
	push	dx	;save low hq
	push	ax	;save lcb
	shl	bx,1
	mov	ax,mpsbps[bx]
	mov	es,ax
	shr	ax,1
	jc	CUP02
;	reload sb trashes ax, cx, and dx
	cCall	ReloadSb,<>
CUP02:
	pop	ax	;restore lcb
	pop	bx	;restore low hq
	mov	bx,es:[bx]
	mov	cx,es:[bx-2]
	sub	cx,20
	jb	CUP04
	cmp	cx,ax
	jb	CUP04

;		    /* don't bother if it is close */
;		    {
;		    AssertDo(FChngSizePhqLcb(&hq, lcb)); /* HM */
;		    (*hplc)->hqplce = hq;
;		    }
;		}
	mov	bx,sp
	inc	bx
	inc	bx
	xor	cx,cx
	cCall	FChngSizePhqLcb,<bx, cx, ax>
ifdef DEBUG
	or	ax,ax
	jne	CUP03
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn
	mov	bx,1020
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
CUP03:
endif ;DEBUG
CUP04:
	pop	bx	;restore hplc
	mov	bx,[bx]
	pop	[bx.LO_hqplcePlc]
	pop	[bx.HI_hqplcePlc]
	ret

sEnd	edit
        end
