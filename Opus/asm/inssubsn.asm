        include w2.inc
        include noxport.inc
        include consts.inc
        include structs.inc

createSeg	inssubs_PCODE,inssubs,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midInssubsn	equ 12		; module ID, for native asserts
endif

; EXTERNAL FUNCTIONS

externFP	<ReloadSb>
externFP	<N_HpchGetPn>
externFP	<SetDirty>
externFP	<PutCpPlc>
externFP	<PnAlloc1>
externFP	<PnAlloc2>
externFP	<N_FInsertInPlc>
externFP	<N_CachePara>
externFP	<GetPchpDocCpFIns>
externFP	<CbAppendTapPropsToPapx>
externFP	<N_FReplace>
externFP	<N_MapStc>
externFP	<AddPrlSorted>
externFP	<N_ValFromPropSprm>
externFP	<CbGrpprlFromPap>

ifdef DEBUG
externFP	<AssertProcForNative>
externFP	<S_HpchGetPn>
externFP	<FRareProc>
externFP	<S_FInsertInPlc>
externFP	<S_CachePara>
externFP	<S_CbGenChpxFromChp>
externFP	<S_FNewChpIns>
externFP	<S_FcAppendRgchToFn>
externFP	<S_CbGenPapxFromPap>
externFP	<S_FAddRun>
externFP	<S_FReplace>
externFP	<S_MapStc>
externFP	<S_ScanFnForBytes>
externFP	<S_CbGenPrl>
externFP	<S_ValFromPropSprm>
endif

sBegin  data

; 
; /* E X T E R N A L S */
; 
externW mpsbps		; extern SB		  mpsbps[];
externW vfkpdText	; extern struct FKPDT	  vfkpdText;
externW mpfnhfcb	; extern struct FCB	  **mpfnhfcb[];
externW vibp		; extern int		  vibp;
externW vmerr		; extern struct MERR	  vmerr;
externW dnsprm		; extern struct ESPRM	  dnsprm[];
externW vchpStc 	; extern struct CHP	  vchpStc;
externW vfkpdChp	; extern struct FKPD	  vfkpdChp;
externW vfkpdPap	; extern struct FKPD	  vfkpdPap;
externW mpdochdod	; extern struct DOD	  **mpdochdod[];

ifdef DEBUG
externW 		wFillBlock
endif

sEnd    data


; CODE SEGMENT _INSSUBS

sBegin	inssubs
	assumes cs,inssubs
        assumes ds,dgroup
        assumes ss,dgroup


;-------------------------------------------------------------------------
;	LN_ReloadSb
;-------------------------------------------------------------------------
	;LN_ReloadSb takes an sb in bx and set es to the corresponding value.
	;ax, bx, cx, dx, es altered.
LN_ReloadSb:
	shl	bx,1
	mov	ax,[bx.mpsbps]
	mov	es,ax
	shr	ax,1
	jc	LN_RS01
;	ReloadSb trashes ax, cx, and dx
	cCall	ReloadSb,<>
LN_RS01:
ifdef DEBUG
	mov	ax,[wFillBlock]
	mov	cx,[wFillBlock]
	mov	dx,[wFillBlock]
endif ;DEBUG
	ret

Ltemp004:
	jmp	LNewPage

;-------------------------------------------------------------------------
;	FAddRun(fn, fcLim, pchProp, cchProp, pfkpd, grpf)
;-------------------------------------------------------------------------
;/* F  A D D  R U N */
;NATIVE FAddRun(fn, fcLim, pchProp, cchProp, pfkpd, grpf)
;int fn;		 /* file */
;FC fcLim;		 /* last file character described by this run */
;char *pchProp; 	 /* pointer to PAPX or CHPX to add */
;int  cchProp;		 /* size of PAPX or CHPX */
;struct FKPD *pfkpd;	 /* Contains data about current fkp - current FKP page,
;			    run insertion point, property insertion point. */
;int grpf;
;fPara = grpf & 4;	 /* fTrue if adding PAP, otherwise adding CHP */
;fAllocMac = grpf & 2;	 /* when fTrue allocate new pn at file's fcMac. when
;			    false allocate new pn using file's current fcPos.*/
;fPlcMustExp = grpf & 1; /* when fTrue, will cause fFalse return whenever it
;			    is impossible to expand the hplcbte. */
;{ /*
;DESCRIPTION:
;Add a char or para run to an fn.
;Use, and update, the information in the pertinent FKPD (Formatted disK Page
;Descriptor).
;*/
;	 int fPara = grpf & 4;
;	 int fAllocMac = grpf & 2;
;	 int fPlcMustExp = grpf & 1;
;	 struct FKP HUGE *hpfkp;
;	 struct FCB *pfcb;
;	 int cchPropShare; /* = cchProp or 0 if shared */
;	 struct PLCBTE **hplcbte;
;	 FC fcFirst;
;	 char HUGE *hpch;
;	 int bCur;
;	 int bNewLim;
;	 int crun;
;	 int cbInc;
;	 char HUGE *hpcrun, HUGE *hpchPropLim;
;	 int cbPage;
;	 int cchPropTot;
;	 int cchStored;
;	 int fStoreCw = WinMac(fPara, !fWord3 && fPara);
;	 char HUGE *hpchFprop;
;	 char HUGE *hpbFirst;
;	 char HUGE *hpb;

maskfParaLocal		equ 4
maskfAllocMacLocal	equ 2
maskfPlcMustExpLocal	equ 1

; %%Function:N_FAddRun %%Owner:BRADV
cProc	N_FAddRun,<PUBLIC,FAR>,<si,di>
	ParmW	fn
	ParmD	fcLim
	ParmW	pchProp
	ParmW	cchProp
	ParmW	pfkpd
	ParmW	grpf

	LocalW	pfcb
	LocalW	hplcbte
	LocalW	sbHpfkp
	LocalW	bCur
	LocalW	bNewLim
	LocalD	fcFirst
	LocalW	crun
	LocalW	cchPropShare
	LocalW  pnT

cBegin

;	if (fn == fnScratch && vmerr.fDiskEmerg)
;		return fFalse;
	mov	bx,[fn]
	cmp	bx,fnScratch
	jne	FAR005
	test	[vmerr.fDiskEmergMerr],maskfDiskEmergMerr
	jz	FAR005
	jmp	FAR18
FAR005:

;	pfcb = *mpfnhfcb[fn];
	shl	bx,1
	mov	bx,[bx.mpfnhfcb]
	mov	bx,[bx]
	mov	[pfcb],bx

;	hplcbte = fPara ? pfcb->hplcbtePap :
;		pfcb->hplcbteChp;
	mov	ax,[bx.hplcbtePapFcb]
	test	[grpf],maskfParaLocal
	jne	FAR01
	mov	ax,[bx.hplcbteChpFcb]
FAR01:
	mov	[hplcbte],ax

;	if (pfkpd->bFreeLim == 0)
;		goto LNewPage; /* initial state */
	mov	si,[pfkpd]
	cmp	[si.bFreeLimFkpd],0
	je	Ltemp004

;	else
;		{
;/* if this is a dry run, just use the passed dummy page */
;		hpfkp = (struct FKP HUGE *)HpchGetPn(fn, pfkpd->pn);
;		}
ifdef DEBUG
	cCall	S_HpchGetPn,<[fn],[si.pnFkpd]>
else ;not DEBUG
	cCall	N_HpchGetPn,<[fn],[si.pnFkpd]>
endif ;DEBUG
	xchg	ax,di
	mov	[sbHpfkp],dx

;	Assert(((int)hpfkp & 1) == 0);
ifdef DEBUG
	test	di,1
	je	FAR03
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midInssubsn
	mov	bx,1001
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FAR03:
endif ;DEBUG

	;LN_ReloadSb takes an sb in bx and set es to the corresponding value.
	;ax, bx, cx, dx, es altered.
	mov	bx,[sbHpfkp]
	call	LN_ReloadSb

;/* this loop searches for a the current or next page that has enough room */
;	for (;;)
;		{
FAR04:

;		crun = *(hpcrun = hpchPropLim = &hpfkp->crun);
	mov	al,es:[di.crunFkp]
	xor	ah,ah
	mov	[crun],ax
	lea	dx,[di.crunFkp]     ;register hpchPropLim

;		/* If cchProp is zero, pchProp is the standard property.
;		   Don't add prop to FKP, just add a run */
;		cchPropShare = cchProp;
	mov	cx,[cchProp]

;		if (cchPropShare == 0)
;			bCur = bNil;
	mov	[bCur],bNil
	jcxz	FAR09

;		else
;			{
;			/* Attempt to have the present run "share" a property by
;			   searching through all of the existing props in the FKP
;			   page for an identical copy of pchProp.
;			*/
;/* if we must store a cw at the front of a PAPX, make sure we pad the
;   papx to an even length. */
;		   if (fStoreCw && (cchPropShare & 1) != 0)
;				{
	test	[grpf],maskfParaLocal
	je	FAR045

;/* for the comparison in the loop to work correctly, the papx stored at
;   pchProp must be followed by a byte of 0 */
;				Assert(*(pchProp + cchProp) == 0);
ifdef DEBUG
	;Do this assert with a call so as not to mess up short jumps
	call	FAR22
endif ;DEBUG

;				cchPropShare += 1;
	inc	cx
	and	cl,0FEh

;				}
FAR045:

;			if (crun > 0)
;				{
	or	ax,ax
	je	FAR08

;				hpch = (char HUGE *)hpfkp + pfkpd->bFreeLim;
	push	di	;save OFF_hpfkp
	add	di,[si.bFreeLimFkpd]

;				while (hpch < hpchPropLim)
;					{
FAR05:
	cmp	di,dx
	jae	FAR07

;					cchStored = *hpch;
;					if (fStoreCw)
;						cchStored <<= 1;
;					if ((cchStored == cchPropShare) &&
;						!FNeHprgch((char HUGE *)pchProp, hpch + 1,
;						       cchPropShare))
;						{ /* share existing property */
	mov	al,bptr (es:[di])
	xor	ah,ah
	test	[grpf],maskfParaLocal
	je	FAR055
	shl	ax,1
FAR055:
	cmp	ax,cx
	jne	FAR06
	push	si	;save pfkpd
	push	di	;save OFF_hpch
	push	cx	;save cchPropShare
	mov	si,[pchProp]
	inc	di
	repe	cmpsb
	pop	cx	;restore cchPropShare
	pop	di	;restore OFF_hpch
	pop	si	;restore pfkpd
	jne	FAR06

;						bCur = hpch - hpfkp;
	xchg	ax,di
	pop	di	;restore OFF_hpfkp
	sub	ax,di
	mov	[bCur],ax

;						cchPropShare = 0;
	xor	cx,cx

;						break;	/* exit while */
	jmp	short FAR08

;						}
FAR06:

;					/* inc. to look at next property */
;					hpch += (cbInc = (cchStored + 1));
	inc	ax
	add	ax,di

;/* in the new format the properties must begin on word boundaries. */
;					 /* REVIEW is this dangerous? bz */
;					if (((long)hpch & 1) != 0)
;						{
;						*hpch++;
;						}
	inc	ax
	and	al,0FEh
	xchg	ax,di

;					}	/* end while (properties remaining in FKP) */
	jmp	short FAR05
FAR07:
	pop	di	;restore OFF_hpfkp

;				}
FAR08:

;			}	/* end else (cchPropShare > 0) */
FAR09:

;		cchPropTot = (cchPropShare > 0) ? cchPropShare + 1 : 0;
	mov	[cchPropShare],cx
	jcxz	FAR10
	inc	cx
FAR10:

;		/* If RUN and FPAP/FCHP (PAPX/CHPX with prepended one byte cch) will
;		   NOT fit on this FKP page, set allocate new page */
;		bNewLim = pfkpd->bFreeLim - cchPropTot;
	mov	ax,[si.bFreeLimFkpd]
	sub	ax,cx

;		if (cchPropTot != 0 && (bNewLim & 1) != 0)
;			bNewLim--;
	jcxz	FAR11
	and	al,0FEh
FAR11:
	mov	[bNewLim],ax

;		if (pfkpd->bFreeFirst + sizeof(FC) + 1 > bNewLim)
;			{
;			int pn;
	mov	bx,5
	add	bx,[si.bFreeFirstFkpd]
	cmp	bx,ax
	jg	LNewPage
	jmp	FAR15

;LNewPage:		fcFirst = pfkpd->fcFirst;
;			/* if no dry run, get the pointer to the proper bin table,
;			   and get the bin table Mac. Otherwise claim we have 1
;			   bin table entry for a dry run. */
LNewPage:
	mov	ax,[si.LO_fcFirstFkpd]
	mov	[OFF_fcFirst],ax
	mov	ax,[si.HI_fcFirstFkpd]
	mov	[SEG_fcFirst],ax

;			if (fn == fnScratch)
;				{
	cmp	[fn],fnScratch
	jne	FAR12

;/* check if partial page should be stored for future use */
;				FC fcMac;
;				int b;

;				pn = PnFromFc(fcMac = pfcb->cbMac);
	mov	bx,[pfcb]
	mov	ax,[bx.LO_cbMacFcb]
	mov	dx,[bx.HI_cbMacFcb]
	;***Begin in line PnFromFc
	errnz	<cbSector - 512>
	mov	cl,ah
	mov	ch,dl
	shr	dh,1
	rcr	cx,1	    ;pn in cx
	;***End in line PnFromFc

;				if ((b = (fcMac & (cbSector - 1))))
;					{
	and	ax,cbSector-1
	je	FAR12

;					vfkpdText.bFreeFirst = b;
	mov	[vfkpdText.bFreeFirstFkpdt],ax

;					vfkpdText.pn = pn;
	mov	[vfkpdText.pnFkpdt],cx

;					}
FAR12:

;				}
FAR13:

;/* PnAlloc is divided into allocate and increment phases so that HpchGetPn
;get operate efficiently beyond the Eof */
;
;			pn = PnAlloc1(fn, fAllocMac);
	mov	ax,[grpf]
	and	al,maskfAllocMacLocal
	cCall	PnAlloc1,<[fn],ax>
	mov	[pnT],ax

;			if (fPlcMustExp || !pfkpd->fPlcIncomplete)
;				{
	test	[grpf],maskfPlcMustExpLocal
	jne	FAR14
	cmp	[si.fPlcIncompleteFkpd],fFalse
	jne	FAR147
FAR14:

;				if (FRareT(600,!FInsertInPlc(hplcbte, IMacPlc(hplcbte),
;					   fcFirst, &pn)))
;					{
;#ifdef DEBUG
;#define FRareT(n, f) ((f) || FRareProc(n))
;#else
;#define FRareT(n, f) (f)
;#endif
	mov	bx,[hplcbte]
	push	bx
	;***Begin in line IMacPlc
	mov	bx,[bx]
	push	[bx.iMacPlcStr]
	;***End in line IMacPlc
	push	[SEG_fcFirst]
	push	[OFF_fcFirst]
	lea	ax,[pnT]
	push	ax
ifdef DEBUG
	cCall	S_FInsertInPlc,<>
else ;not DEBUG
	cCall	N_FInsertInPlc,<>
endif ;DEBUG
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,600
	cCall	FRareProc,<ax>
	or	ax,ax
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	je	FAR143
	xor	ax,ax
FAR143:
endif ;/* DEBUG */
	or	ax,ax
	jne	FAR147

;					if (fPlcMustExp)
;						return fFalse;
	test	[grpf],maskfPlcMustExpLocal
	jne	Ltemp002

;					else
;						pfkpd->fPlcIncomplete = fTrue;
	mov	[si.fPlcIncompleteFkpd],fTrue

;					}
;				}
FAR147:


;
;			pfkpd->pn = pn;
	mov	ax,[pnT]
	mov	[si.pnFkpd],ax


;			hpfkp = (struct FKP HUGE *)HpchGetPn(fn, pfkpd->pn);
ifdef DEBUG
	cCall	S_HpchGetPn,<[fn],[si.pnFkpd]>
else ;not DEBUG
	cCall	N_HpchGetPn,<[fn],[si.pnFkpd]>
endif ;DEBUG
	xchg	ax,di
	mov	[sbHpfkp],dx

;			PnAlloc2(fn, pfkpd->pn, fAllocMac);
	mov	ax,[grpf]
	and	al,maskfAllocMacLocal
	cCall	PnAlloc2,<[fn],[si.pnFkpd],ax>

	;LN_ReloadSb takes an sb in bx and set es to the corresponding value.
	;ax, bx, cx, dx, es altered.
	mov	bx,[sbHpfkp]
	call	LN_ReloadSb

;			bltbcx(0, LpFromHp(hpfkp), cbSector);
;			hpfkp->rgfc[0] = fcFirst;
	push	di	;save OFF_hpfkp
	mov	ax,[OFF_fcFirst]
	stosw
	mov	ax,[SEG_fcFirst]
	stosw
	xor	ax,ax
	mov	cx,(cbSector - 4)/2
	rep	stosw
	pop	di	;restore OFF_hpfkp

;			pfkpd->bFreeFirst = sizeof(FC);
;			/* pfkp->crun = 0 */
	mov	[si.bFreeFirstFkpd],4

;			pfkpd->bFreeLim = cbSector - 1;
	mov	[si.bFreeLimFkpd],cbSector-1

;			continue;
	jmp	FAR04

Ltemp002:
	jmp	FAR18

;			}
FAR15:

;		if (cchPropShare > 0)
;			{
	cmp	[cchPropShare],0
	jbe	FAR16

;		/* store papx or chpx below bFreeLim */
;			bCur = pfkpd->bFreeLim = bNewLim;
	mov	bx,[bNewLim]
	mov	[si.bFreeLimFkpd],bx
	mov	[bCur],bx

;			hpchFprop = (char HUGE *) hpfkp + bCur;
	push	di	;save OFF_hpfkp
	add	di,bx

;			*hpchFprop = (!fStoreCw) ? cchProp : (cchPropShare >> 1);
	mov	cx,[cchProp]
	mov	al,cl
	test	[grpf],maskfParaLocal
	je	FAR155
	mov	ax,[cchPropShare]
	shr	ax,1
FAR155:
	stosb

;			bltbh(pchProp, hpchFprop + 1, cchProp);
	mov	si,[pchProp]
	rep	movsb
	pop	di	;restore OFF_hpfkp

;			}
FAR16:

;		/* create a run */
;		hpbFirst = (char HUGE *) &hpfkp->rgfc[crun + 1];
;		bltbh(hpbFirst, hpbFirst + sizeof(FC), crun);
;		*(hpbFirst + sizeof(FC) + crun) = bCur / sizeof(int);
;		*((FC HUGE *)hpbFirst) = fcLim;
	push	di	;save OFF_hpfkp
	mov	cx,[crun]
	mov	bx,cx
	inc	bx
	inc	bx
	shl	bx,1
	shl	bx,1
	add	bx,cx
	push	es
	pop	ds
	std
	lea	di,[bx+di]
	mov	ax,[bCur]
	shr	ax,1
	stosb
	lea	si,[di-4]
	rep	movsb
	dec	di
	mov	ax,[SEG_fcLim]
	stosw
	xchg	ax,dx
	mov	ax,[OFF_fcLim]
	stosw
	cld
	push	ss
	pop	ds
	pop	di	;restore OFF_hpfkp

;		pfkpd->bFreeFirst += sizeof(FC) + 1;
	mov	si,[pfkpd]
	add	[si.bFreeFirstFkpd],5

;		(*hpcrun)++;
	inc	es:[di.crunFkp]

;		Assert(pfkpd->bFreeFirst == *hpcrun * (sizeof(FC) + 1) +
;			 sizeof(FC));
ifdef DEBUG
	;Do this assert with a call so as not to mess up short jumps
	call	FAR20
endif ;DEBUG

;		pfkpd->fcFirst = fcLim;
	mov	[si.LO_fcFirstFkpd],ax
	mov	[si.HI_fcFirstFkpd],dx

;		SetDirty(vibp);
;		PutCpPlc(hplcbte, IMacPlc(hplcbte), fcLim);
	mov	bx,[hplcbte]
	push	bx
	;***Begin in line IMacPlc
	mov	bx,[bx]
	push	[bx.iMacPlcStr]
	;***End in line IMacPlc
	push	dx
	push	ax
	cCall	SetDirty,<[vibp]>
	cCall	PutCpPlc,<>

;		return (fn != fnScratch || !vmerr.fDiskEmerg);
	cmp	[fn],fnScratch
	jne	FAR17
	test	[vmerr.fDiskEmergMerr],maskfDiskEmergMerr
	jnz	FAR18
FAR17:
	db	0B8h	;turns next "xor ax,ax" into "mov ax,immediate"
FAR18:
	errnz	<fFalse>
	xor	ax,ax

;		}
;}
cEnd


ifdef DEBUG
;		Assert(pfkpd->bFreeFirst == *hpcrun * (sizeof(FC) + 1) +
;			 sizeof(FC));
FAR20:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	al,es:[di.crunFkp]
	mov	ah,5
	mul	ah
	add	ax,4
	cmp	[si.bFreeFirstFkpd],ax
	je	FAR21
	mov	ax,midInssubsn
	mov	bx,1002
        cCall   AssertProcForNative,<ax, bx>
FAR21:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG

ifdef DEBUG
;				Assert(*(pchProp + cchProp) == 0);
FAR22:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	bx,[pchProp]
	add	bx,[cchProp]
	cmp	bptr [bx],0
	je	FAR23
	mov	ax,midInssubsn
	mov	bx,1003
        cCall   AssertProcForNative,<ax, bx>
FAR23:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG


;-------------------------------------------------------------------------
;	ScanFnForBytes(fn, hpch, cch, fWrite)
;-------------------------------------------------------------------------
;/* S C A N   F N   F O R   B Y T E S */
;/* Beginning with the current fcPos for fn, cch bytes of fn are paged into
;   memory. If fWrite is true the contents of the rgch pointed to by pch
;   are copied to the file. If fWrite is false, the rgch pointed to by pch
;   is filled with data read from the file. */
;NATIVE ScanFnForBytes(fn, hpch, cch, fWrite)
;int fn;
;char HUGE *hpch;
;uns cch;
;int fWrite;
;{
;	struct FCB *pfcb = *mpfnhfcb[fn];
;	FC fc	 = pfcb->fcPos;
;	FC fcCur = fc;
;	PN pnCur = PnFromFc(fcCur);

; %%Function:N_ScanFnForBytes %%Owner:BRADV
cProc	N_ScanFnForBytes,<PUBLIC,FAR>,<si,di>
	ParmW	fn
	ParmD	hpch
	ParmW	cch
	ParmW	fWrite

	LocalW	pfcb
	LocalW	pnCur

cBegin

	mov	bx,[fn]
	shl	bx,1
	mov	bx,[bx.mpfnhfcb]
	mov	bx,[bx]
	mov	[pfcb],bx
	mov	si,[bx.LO_fcPosFcb]
	mov	di,[bx.HI_fcPosFcb]
	mov	ax,si
	mov	dx,di
	;***Begin in line PnFromFc
	errnz	<cbSector - 512>
	mov	cl,ah
	mov	ch,dl
	shr	dh,1
	rcr	cx,1	    ;pn in cx
	;***End in line PnFromFc
	mov	[pnCur],cx
	cmp	[cch],0
	jmp	SFFB07

;	/* write range of characters out to file page cache memory.  First
;	   fill the current last page before adding new pages to the end of
;	   the file. */
;	while (cch > 0)
;		{
SFFB01:

;		char HUGE *hpchBp = HpchGetPn(fn, pnCur);
;		int cchBp;
;		int cchBlt;
ifdef DEBUG
	cCall	S_HpchGetPn,<[fn],[pnCur]>
else ;not DEBUG
	cCall	N_HpchGetPn,<[fn],[pnCur]>
endif ;DEBUG
	push	di	;save fcCur
	push	si
	xchg	si,ax
	push	ax	;save OFF_fcCur
	push	dx
	;LN_ReloadSb takes an sb in bx and set es to the corresponding value.
	;ax, bx, cx, dx, es altered.
	mov	bx,[SEG_hpch]
	call	LN_ReloadSb
	pop	bx
	call	LN_ReloadSb
	push	es
	mov	bx,[SEG_hpch]
	call	LN_ReloadSb
	pop	ds
	mov	di,[OFF_hpch]

;		cchBp = fcCur & ((long) maskSector);
	pop	ax	;restore OFF_fcCur
	and	ax,maskSector

;		cchBlt = umin(cbSector - cchBp, cch);
	mov	cx,cbSector
	sub	cx,ax
	cmp	cx,[cch]
	jbe	SFFB02
	mov	cx,[cch]
SFFB02:

;		if (fWrite)
;			{
;			bltbh(hpch, hpchBp + cchBp, cchBlt);
;			SetDirty(vibp);
;			}
;		else
;			bltbh(hpchBp + cchBp, hpch, cchBlt);
	add	si,ax
	cmp	[fWrite],fFalse
	je	SFFB03
	push	ds
	push	es
	pop	ds
	pop	es
	xchg	si,di
SFFB03:
	push	cx	;save cchBlt
	rep	movsb
	push	ss
	pop	ds
	je	SFFB04
	cCall	SetDirty,<[vibp]>
SFFB04:
	pop	cx	;restore cchBlt
	pop	si	;restore fcCur
	pop	di

;		fcCur += cchBlt;
	add	si,cx
	adc	di,0

;		if (fcCur > pfcb->cbMac)
;			{
	mov	bx,[pfcb]
	mov	ax,[bx.LO_cbMacFcb]
	mov	dx,[bx.HI_cbMacFcb]
	sub	ax,si
	sbb	dx,di
	jge	SFFB06

;			Assert(fWrite);
ifdef DEBUG
	cmp	[fWrite],fFalse
	jne	SFFB05
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midInssubsn
	mov	bx,1004
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
SFFB05:
endif ;DEBUG

;			if (fWrite)
;				pfcb->cbMac = fcCur;
	cmp	[fWrite],fFalse
	je	SFFB06
	mov	[bx.LO_cbMacFcb],si
	mov	[bx.HI_cbMacFcb],di

;			}
SFFB06:

;		pnCur++;
	inc	[pnCur]

;		hpch += cchBlt;
	add	[OFF_hpch],cx

;		cch -= cchBlt;
	sub	[cch],cx
SFFB07:
	jbe	Ltemp001
	jmp	SFFB01
Ltemp001:

;		}	/* end while (characters still to be written) */
;	pfcb->fcPos = fcCur;
	mov	[bx.LO_fcPosFcb],si
	mov	[bx.HI_fcPosFcb],di

;}
cEnd


;-------------------------------------------------------------------------
;	FNewChpIns(doc, cp, pchp, stc)
;-------------------------------------------------------------------------
;/* F N E W  C H P  I N S */
;/* Make forthcoming inserted characters have the look in pchp.
;Plan:
;	 ensure vchpStc set.
;	 compute chpT that steps vchpStc to *pchp.
;	 if different then current run, start new run.
;*/
;FNewChpIns(doc, cp, pchp, stc)
;int doc; CP cp;
;struct CHP *pchp;
;int stc;
;{
;	int cb;
;	struct CHP chpT;
;	FC fcMac;
;	struct CHP *pchpBase;
;	struct CHP chpBase;
;	struct PAP papT;

; %%Function:N_FNewChpIns %%Owner:BRADV
cProc	N_FNewChpIns,<PUBLIC,FAR>,<si,di>
	ParmW	doc
	ParmD	cp
	ParmW	pchp
	ParmW	stcArg

	LocalV	chpBase,cbChpMin
	LocalV	chpT,cbChpMin
	LocalV	papT,cbPapMin

cBegin

;	if (stc == stcNil)
;		{
;		CachePara(doc, cp);
;		pchpBase = &vchpStc;
;		}
;	else
;		{
;		MapStc(PdodDoc(doc), stc, &chpBase, &papT);
;		pchpBase = &chpBase;
;		}
	mov	cx,[stcArg]
	mov	bx,[doc]
	cmp	cx,stcNil
	je	FNCI02
	;LN_MapStc takes doc in bx, stc in cx, pchp in ax, ppap in dx,
	;and performs MapStc(*mpdochdod[doc], stc, pchp, ppap).
	;ax, bx, cx, dx are altered.
	lea	dx,[papT]
	lea	si,[chpBase]
	mov	ax,si
	call	LN_MapStc
	jmp	short FNCI03
FNCI02:
ifdef DEBUG
	cCall	S_CachePara,<bx, [SEG_cp], [OFF_cp]>
else ;not DEBUG
	cCall	N_CachePara,<bx, [SEG_cp], [OFF_cp]>
endif ;DEBUG
	lea	si,[vchpStc]
FNCI03:

;	CbGenChpxFromChp(&chpT, pchp, pchpBase, fFalse);
;#define CbGenChpxFromChp(pchpResult, pchp, pchpBase, fWord3) \
;      N_CbGenChpxFromChp(pchpResult, pchp, pchpBase)
	lea	di,[chpT]
	push	di
	push	[pchp]
	push	si
ifdef DEBUG
	cCall	S_CbGenChpxFromChp,<>
else ;not DEBUG
	push	cs
	call	near ptr N_CbGenChpxFromChp
endif ;DEBUG

;	if (FNeRgw(&vfkpdChp.chp, &chpT, cwCHP))
;		{ /* Add the run for the previous insertion; our looks differ. */
	;***Begin in-line FNeRgw
	push	ds
	pop	es
	mov	si,dataoffset [vfkpdChp.chpFkpd]
	errnz	<cbChpMin AND 1>
	mov	cx,cbChpMin SHR 1
	repe	cmpsw
	;***End in-line FNeRgw
	je	FNCI06

;		fcMac = (**mpfnhfcb[fnScratch]).cbMac;
	;Assembler note: fcMac is not used so don't compute it.

;		cb = CchNonZeroPrefix(&vfkpdChp.chp, cbCHP);
	;***Begin in-line CchNonZeroPrefix
	mov	di,dataoffset [vfkpdChp.chpFkpd + cbChpMin - 1]
	mov	cx,cbChpMin
	xor	ax,ax
	std
	repe	scasb
	cld
	je	FNCI04
	inc	cx
FNCI04:
	;***End in-line CchNonZeroPrefix

;		if (vfkpdChp.fcFirst < vfkpdText.fcLim)
;			{
	mov	si,dataoffset [vfkpdChp.chpFkpd]
	mov	ax,[vfkpdText.LO_fcLimFkpdt]
	mov	dx,[vfkpdText.HI_fcLimFkpdt]
	cmp	[si.LO_fcFirstFkpd - (chpFkpd)],ax
	mov	bx,[si.HI_fcFirstFkpd - (chpFkpd)]
	sbb	bx,dx
	jge	FNCI05

;			if (!FAddRun(fnScratch,
;				    vfkpdText.fcLim,
;				    &vfkpdChp.chp,
;				    cb,
;				    &vfkpdChp,
;				    fFalse /* CHP run */,
;				    fTrue /* allocate new pns at fcMac */,
;				    fTrue /* plcbte must expand */,
;				    fFalse /* not Word3 format */))
;				return fFalse;
;#define FAddRun(fn, fc, pch, cch, pfkpd, fPara, fAllocMac, fPlcMustExp, fWord3) \
;     N_FAddRun(fn, fc, pch, cch, pfkpd, (fPara << 2) + (fAllocMac << 1) + fPlcMustExp)
	mov	bx,fnScratch
	push	bx
	push	dx
	push	ax
	push	si
	push	cx
	mov	si,dataoffset [vfkpdChp]
	push	si
	errnz	<fnScratch AND 0FF00h>
	mov	bl,(fFalse SHL 2) + (fTrue SHL 1) + fTrue
	push	bx
ifdef DEBUG
	cCall	S_FAddRun,<>
else ;not DEBUG
	push	cs
	call	near ptr N_FAddRun
endif ;DEBUG
	or	ax,ax
	je	FNCI07

;			}
FNCI05:

;		vfkpdChp.chp = chpT;
;		}
	push	ds
	pop	es
	lea	si,[chpT]
	mov	di,dataoffset [vfkpdChp.chpFkpd]
	errnz	<cbChpMin AND 1>
	mov	cx,cbChpMin SHR 1
	rep	movsw

FNCI06:
;	return fTrue;
	mov	ax,fTrue

FNCI07:
;}
cEnd


;-------------------------------------------------------------------------
;	FInsertRgch(doc, cp, rgch, cch, pchp, ppap)
;-------------------------------------------------------------------------
;/* F I N S E R T  R G C H */
;/* Insert cch characters from rgch into doc before cp with props pchp.
;A chEop may be inserted as the last char in rgch if ppap != 0.
;*/
;FInsertRgch(doc, cp, rgch, cch, pchp, ppap)
;int doc, cch;
;CP cp;
;char rgch[];
;struct CHP *pchp;
;struct PAP *ppap;
;{
;	int cchPapx;
;	CP cpM1;
;	FC fc;
;	struct CHP chp;
;	struct PAP papStd;
;	struct TAP tapStd;
;	struct CA caT;
;	char papx[cchPapxMax];

Ltemp003:
	jmp	FIR05

; %%Function:N_FInsertRgch %%Owner:BRADV
cProc	N_FInsertRgch,<PUBLIC,FAR>,<si,di>
	ParmW	doc
	ParmD	cp
	ParmW	rgch
	ParmW	cch
	ParmW	pchp
	ParmW	ppap

	LocalD	fc
	LocalV	caT,cbCaMin
	LocalV	chp,cbChpMin
	LocalV	papStd,cbPapMin
	LocalV	tapStd,cbTapMin
	LocalV	papx,cchPapxMax

cBegin

;	/* First finish off the previous char run if necessary */
;	if (pchp == 0)
;		{
;		GetPchpDocCpFIns(&chp, doc, cp, fTrue, wwNil);
;		pchp = &chp;
;		}
	mov	si,[pchp]
	or	si,si
	jne	FIR01
	lea	si,[chp]
	mov	ax,fTrue
	errnz	<fTrue - 1>
	errnz	<wwNil - 0>
	cwd
	cCall	GetPchpDocCpFIns,<si, [doc], [SEG_cp], [OFF_cp], ax, dx>
FIR01:

;	/* set insertion point properties */
;	if (!FNewChpIns(doc, cp, pchp, (ppap == 0) ? stcNil : ppap->stc))
;		return fFalse;
	push	[doc]
	push	[SEG_cp]
	push	[OFF_cp]
	push	si
	mov	ax,stcNil
	mov	di,[ppap]
	or	di,di
	je	FIR02
	mov	al,[di.stcPap]
	xor	ah,ah
FIR02:
	push	ax
ifdef DEBUG
	cCall	S_FNewChpIns,<>
else ;not DEBUG
	push	cs
	call	near ptr N_FNewChpIns
endif ;DEBUG
	or	ax,ax
	je	Ltemp003

;	/* Now write the characters to the scratch file */
;	fc = FcAppendRgchToFn(fnScratch, rgch, cch);
	mov	ax,fnScratch
	push	ax
	push	[rgch]
	push	[cch]
ifdef DEBUG
	cCall	S_FcAppendRgchToFn,<>
else ;not DEBUG
	push	cs
	call	near ptr N_FcAppendRgchToFn
endif ;DEBUG
	mov	[OFF_fc],ax
	mov	[SEG_fc],dx

;	if (vmerr.fDiskEmerg)
;		return fFalse;
	errnz	<fFalse>
	xor	ax,ax
	test	[vmerr.fDiskEmergMerr],maskfDiskEmergMerr
	jne	Ltemp003

;	/* Now insert a paragraph run if we inserted an EOL */
;	if (ppap != 0)
;		{ /* Inserting EOL--must be last character of rgch */
	or	di,di
	je	FIR04

;		Assert( rgch [cch-1] == chEol || rgch [cch-1] == chSect ||
;			rgch [cch-1] == chTable );
ifdef DEBUG
	;Do this assert with a call so as not to mess up short jumps
	call	FIR06
endif ;DEBUG

;		MapStc(*mpdochdod[doc], ppap->stc, 0, &papStd);
	;LN_MapStc takes doc in bx, stc in cx, pchp in ax, ppap in dx,
	;and performs MapStc(*mpdochdod[doc], stc, pchp, ppap).
	;ax, bx, cx, dx are altered.
	mov	bx,[doc]
	lea	dx,[papStd]
	lea	si,[papx]
	push	si	;Argument for CbGenPapxFromPap
	push	di	;Argument for CbGenPapxFromPap
	push	dx	;Argument for CbGenPapxFromPap
	xor	ax,ax
	mov	cl,[di.stcPap]
	xor	ch,ch
	call	LN_MapStc

;		cchPapx = CbGenPapxFromPap(&papx, ppap, &papStd, fFalse);
;#define CbGenPapxFromPap(ppapx, ppap, ppapBase, fWord3) \
;      N_CbGenPapxFromPap(ppapx, ppap, ppapBase)
ifdef DEBUG
	cCall	S_CbGenPapxFromPap,<>
else ;not DEBUG
	push	cs
	call	near ptr N_CbGenPapxFromPap
endif ;DEBUG

;		if (ppap->fTtp)
;			{
;			SetWords(&tapStd, 0, cwTAP);
;			cchPapx = CbAppendTapPropsToPapx(&papx, cchPapx, ppap->ptap, &tapStd, cchPapxMax - cchPapx);
;			}
	cmp	[di.fTtpPap],fFalse
	je	FIR03
	push	si
	push	ax
	push	[di.ptapPap]
	mov	di,dataoffset [tapStd]
	push	di
	neg	ax
	add	ax,cchPapxMax
	push	ax
	cCall	CbAppendTapPropsToPapx,<>
FIR03:

;		if (!FAddRun(fnScratch,
;			    vfkpdText.fcLim,
;			    &papx,
;			    cchPapx,
;			    &vfkpdPap,
;			    fTrue /* para run */,
;			    fTrue /* alloc at fcMac */,
;			    fTrue /* plcbte must expand */,
;			    fFalse /* not Word 3 format */))
;			return fFalse;
;#define FAddRun(fn, fc, pch, cch, pfkpd, fPara, fAllocMac, fPlcMustExp, fWord3) \
;     N_FAddRun(fn, fc, pch, cch, pfkpd, (fPara << 2) + (fAllocMac << 1) + fPlcMustExp)
	mov	bx,fnScratch
	push	bx
	push	[vfkpdText.HI_fcLimFkpdt]
	push	[vfkpdText.LO_fcLimFkpdt]
	push	si
	push	ax
	mov	si,dataoffset [vfkpdPap]
	push	si
	errnz	<fnScratch AND 0FF00h>
	mov	bl,(fTrue SHL 2) + (fTrue SHL 1) + fTrue
	push	bx
ifdef DEBUG
	cCall	S_FAddRun,<>
else ;not DEBUG
	push	cs
	call	near ptr N_FAddRun
endif ;DEBUG
	or	ax,ax
	je	FIR05

;		}
FIR04:

;	/* Finally, insert the piece into the document */
;	return FReplace(PcaPoint(&caT, doc, cp), fnScratch, fc, (FC) cch);
	;***Begin in-line PcaPoint
	push	ds
	pop	es
	lea	di,[caT]
	push	di
	mov	ax,[OFF_cp]
	stosw
	xchg	ax,dx
	mov	ax,[SEG_cp]
	stosw
	xchg	ax,dx
	stosw
	xchg	ax,dx
	stosw
	mov	ax,[doc]
	stosw
	;***End in-line PcaPoint
	mov	ax,[fnScratch]
	push	ax
	push	[SEG_fc]
	push	[OFF_fc]
	mov	ax,[cch]
	cwd
	push	dx
	push	ax
ifdef DEBUG
	cCall	S_FReplace,<>
else ;not DEBUG
	cCall	N_FReplace,<>
endif ;DEBUG

;}
FIR05:
cEnd

ifdef DEBUG
;	Assert( rgch [cch-1] == chEol || rgch [cch-1] == chSect ||
;		rgch [cch-1] == chTable );
FIR06:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	bx,[rgch]
	add	bx,[cch]
	mov	al,[bx-1]
	cmp	al,chEol
	je	FIR07
	cmp	al,chSect
	je	FIR07
	cmp	al,chTable
	je	FIR07
	mov	ax,midInssubsn
	mov	bx,1005
        cCall   AssertProcForNative,<ax, bx>
FIR07:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG

	;LN_MapStc takes doc in bx, stc in cx, pchp in ax, ppap in dx,
	;and performs MapStc(*mpdochdod[doc], stc, pchp, ppap).
	;ax, bx, cx, dx are altered.
LN_MapStc:
	shl	bx,1
	mov	bx,[bx.mpdochdod]
ifdef DEBUG
	or	bx,bx
	jne	MS01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midInssubsn
	mov	bx,1006
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
MS01:
endif ;DEBUG
	mov	bx,[bx]
ifdef DEBUG
	cCall	S_MapStc,<bx, cx, ax, dx>
else ;not DEBUG
	cCall	N_MapStc,<bx, cx, ax, dx>
endif ;DEBUG
	ret

;-------------------------------------------------------------------------
;	FcAppendRgchToFn(fn, pch, cch)
;-------------------------------------------------------------------------
;/* F C  A P P E N D  R G C H  T O  F N */
;/* Appends characters pointed to by pch, length cch, to end of file fn.
;Returns first fc written.
;If there is a free hole of sufficient size in vfkpdText, it will be
;used instead.
;fcLim updated to point to after the last char written.
;*/
;FC FcAppendRgchToFn(fn, pch, cch)
;int fn;
;char *pch;
;int cch;
;{

; %%Function:N_FcAppendRgchToFn %%Owner:BRADV
cProc	N_FcAppendRgchToFn,<PUBLIC,FAR>,<si,di>
	ParmW	fn
	ParmW	pch
	ParmW	cch

cBegin

;	FC fc = (*mpfnhfcb[fn])->cbMac;
	mov	si,[fn]
	mov	cx,si
	shl	si,1
	mov	si,[si.mpfnhfcb]
ifdef DEBUG
	or	si,si
	jne	FARTF01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midInssubsn
	mov	bx,1007
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FARTF01:
endif ;DEBUG
	mov	si,[si]
	mov	ax,[si.LO_cbMacFcb]
	mov	dx,[si.HI_cbMacFcb]

;	if (fn == fnScratch && vfkpdText.pn != pnNil
;		&& (cbSector - vfkpdText.bFreeFirst) >= cch)
;		{
	mov	bx,pnNil
	errnz	<fnScratch - 1>
	dec	cx
	jne	FARTF02
	cmp	[vfkpdText.pnFkpdt],bx
	je	FARTF02
	mov	di,cbSector
	sub	di,[vfkpdText.bFreeFirstFkpdt]
	cmp	di,[cch]
	jl	FARTF02

;		fc = FcFromPn(vfkpdText.pn) + vfkpdText.bFreeFirst;
	;***Begin in-line FcFromPn
	mov	ax,[vfkpdText.pnFkpdt]
	xor	dx,dx
	errnz	<shftSector - 9>
	xchg	ah,dl
	xchg	al,ah
	shl	ax,1
	rcl	dx,1
	;***End in-line FcFromPn
	add	ax,[vfkpdText.bFreeFirstFkpdt]
	adc	dx,0

;		vfkpdText.bFreeFirst += cch;
	mov	di,[cch]
	add	[vfkpdText.bFreeFirstFkpdt],di

;		}
	jmp	short FARTF03
FARTF02:

;	else
;/* maintain fc's monotonically increasing */
;		vfkpdText.pn = pnNil;
	mov	[vfkpdText.pnFkpdt],bx
FARTF03:

;	if (fn == fnScratch)
;		vfkpdText.fcLim = fc + cch;
	or	cx,cx
	jne	FARTF04
	push	dx
	push	ax
	add	ax,[cch]
	adc	dx,0
	mov	[vfkpdText.LO_fcLimFkpdt],ax
	mov	[vfkpdText.HI_fcLimFkpdt],dx
	pop	ax
	pop	dx
FARTF04:

;	SetFnPos(fn, fc);
	;***Begin in-line SetFnPos
;	struct FCB *pfcb = *mpfnhfcb[fn];
;	if (fc > pfcb->cbMac)
;		pfcb->cbMac = fc;
	cmp	[si.LO_cbMacFcb],ax
	mov	di,[si.HI_cbMacFcb]
	sbb	di,dx
	jge	FARTF05
	mov	[si.LO_cbMacFcb],ax
	mov	[si.HI_cbMacFcb],dx
FARTF05:

;	pfcb->fcPos = fc;
	mov	[si.LO_fcPosFcb],ax
	mov	[si.HI_fcPosFcb],dx
	;***End in-line SetFnPos

;	WriteRgchToFn(fn, pch, cch);
;#define WriteRgchToFn(fn, pch, cch) \
;      N_ScanFnForBytes(fn, (char HUGE *)pch, cch, fTrue /* fWrite */)
	push	dx
	push	ax	;save fc
	errnz	<fnScratch - 1>
	inc	cx
	push	cx
	errnz	<pnNil - (-1)>
	errnz	<sbDds - 1>
	neg	bx
	push	bx
	push	[pch]
	push	[cch]
	push	bx
ifdef DEBUG
	cCall	S_ScanFnForBytes,<>
else ;not DEBUG
	push	cs
	call	near ptr N_ScanFnForBytes
endif ;DEBUG
	pop	ax
	pop	dx	;restore fc

;	return (fc);
;}
cEnd



;-------------------------------------------------------------------------
;	CbGrpprlProp(fNorm, pgrpprl, cbMax, pprop, ppropBase, cwProp, mpiwspx, mpiwspxW3)
;-------------------------------------------------------------------------
;/* C B  G R P P R L  P R O P */
;/* generates a list of prl's that expresses the differences of prop
;from propBase, when the prop's bytes are encoded according to
;mpiwspx (which describes the sprm describing the two bytes of word iw in
;prop.)
;fNorm means that the sprm's must be sorted in normal order (necessary
;if grpprl is to go in a prc, not necessary if grpprl is to be aprt of a
;papx or sepx.)
;Does not do tbd's in pap's (see below.)
;*/
;
;/* OPUS comment: mpiwspx will need to be a FAR variable if we change the
;   structures to be stored as csconst */
;
;NATIVE int CbGrpprlProp(fNorm, pgrpprl, cbMax, pprop, ppropBase, cwProp, mpiwspx, mpiwspxW3)
;BOOL fNorm;
;char *pgrpprl;
;int cbMax;
;char *pprop;
;char *ppropBase;
;int cwProp;
;struct SPX mpiwspx[];
;struct SPX mpiwspxW3[];
;{
;	struct SPX *pspx;
;	int *pw;
;	int *pwBase;
;	int iw;
;	int isprm, sprm;
;	int cbPrl;
;	char rgb[3];
;	struct SEBL sebl;

; %%Function:N_CbGrpprlProp %%Owner:BRADV
cProc	N_CbGrpprlProp,<PUBLIC,FAR>,<si,di>
	ParmW	fNorm
	ParmW	pgrpprl
	ParmW	cbMax
	ParmW	pprop
	ParmW	ppropBase
	ParmW	cwProp
	ParmW	mpiwspx

	LocalW	pwDiff
	LocalV	rgb,4
	LocalV	sebl,cbSeblMin

cBegin

;	sebl.cbEarlier = sebl.cbLater = sebl.cbMerge = 0;
;	sebl.cbMergeMax = cbMax;
;	sebl.pgrpprlLater = rgb;
;	sebl.pgrpprlMerge = pgrpprl;
	xor	ax,ax
	mov	[sebl.cbEarlierSebl],ax
	mov	[sebl.cbLaterSebl],ax
	mov	[sebl.cbMergeSebl],ax
	mov	ax,[cbMax]
	mov	[sebl.cbMergeMaxSebl],ax
	lea	ax,[rgb]
	mov	[sebl.pgrpprlLaterSebl],ax
	mov	ax,[pgrpprl]
	mov	[sebl.pgrpprlMergeSebl],ax

;	for (iw = 0, pw = (int *)pprop, pwBase = (int *) ppropBase;
;		iw < cwProp; iw++, pw++, pwBase++)
;		{
	mov	di,[pprop]
	mov	si,[ppropBase]
	mov	ax,[mpiwspx]
	sub	ax,di
	dec	ax
	dec	ax	;adjust for increment after cmpsw
	mov	[pwDiff],ax
	mov	cx,[cwProp]
ifdef DEBUG
	or	cx,cx
	jge	CGP01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midInssubsn
	mov	bx,1008
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
CGP01:
endif ;DEBUG
	push	ds
	pop	es
CGP02:

;		if (*pw != *pwBase)
;			{
	jcxz	CGP06
	repe	cmpsw
	je	CGP06

;			pspx = &mpiwspx[iw];
	mov	bx,di
	add	bx,[pwDiff]
	errnz	<cbSpxMin - 2>
	mov	ax,[bx]

;			for (isprm = 0; isprm < 2 &&
;				(sprm = pspx->rgsprm[isprm]) != sprmNoop;
;				isprm++)
CGP03:
	push	cx	;save cwRemain
	push	ax	;save rgsprm
	cmp	al,sprmNoop
	je	CGP05

;				if (cbPrl = CbGenPrl(pprop, ppropBase, sprm, rgb))
;					{
	push	[pprop]
	push	[ppropBase]
	xor	ah,ah
	push	ax
	lea	bx,[rgb]
	push	bx
ifdef DEBUG
	cCall	S_CbGenPrl,<>
else ;not DEBUG
	push	cs
	call	near ptr N_CbGenPrl
endif ;DEBUG
	push	ds
	pop	es

;					if (fNorm)
;						AddPrlSorted(&sebl, &rgb[0], sebl.cbLater = cbPrl);
	cmp	[fNorm],fFalse
	je	CGP04
	mov	[sebl.cbLaterSebl],ax
	lea	bx,[sebl]
	lea	cx,[rgb]
	cCall	AddPrlSorted,<bx, cx, ax>
	push	ds
	pop	es
	jmp	short CGP05

;					else
;						{
CGP04:

;						sebl.pgrpprlMerge =
;							bltbyte(&rgb[0], sebl.pgrpprlMerge, cbPrl);
;						sebl.cbMerge += cbPrl;
	add	[sebl.cbMergeSebl],ax
	push	si	;save pw
	push	di	;save pwBase
	lea	si,[rgb]
	mov	di,[sebl.pgrpprlMergeSebl]
	xchg	ax,cx
	rep	movsb
	mov	[sebl.pgrpprlMergeSebl],di
	pop	di	;restore pwBase
	pop	si	;restore pw

;						}
;					 }
CGP05:
	pop	ax	;restore rgsprm
	pop	cx	;restore cwRemain
	mov	al,ah
	mov	ah,sprmNoop
	errnz	<sprmNoop - 0>
	or	ax,ax
	jne	CGP03

;			}
;		}
	jmp	short CGP02

CGP06:
;	return sebl.cbMerge;
	mov	ax,[sebl.cbMergeSebl]

;}
cEnd


;-------------------------------------------------------------------------
;	CbGenPrl(pprop, ppropBase, sprm, rgb)
;-------------------------------------------------------------------------
;/* C B  G E N	P R L */
;/* return prl if values described does not match base value.
;prl is put in rgb, cb of prl is returned.
;If values match, return 0.
;*/
;NATIVE int CbGenPrl(pprop, ppropBase, sprm, rgb)
;char *pprop;
;char *ppropBase;
;int sprm;
;char rgb[];
;{
;	int val;
;	int cbPrl;

; %%Function:N_CbGenPrl %%Owner:BRADV
cProc	N_CbGenPrl,<PUBLIC,FAR>,<si,di>
	ParmW	pprop
	ParmW	ppropBase
	ParmW	sprm
	ParmW	rgb

cBegin

;	val = ValFromPropSprm(pprop, sprm);
	mov	si,[sprm]
ifdef DEBUG
	cCall	S_ValFromPropSprm,<[pprop], si>
else ;not DEBUG
	cCall	N_ValFromPropSprm,<[pprop], si>
endif ;DEBUG
	push	ax  ;save val

;	if (val != ValFromPropSprm(ppropBase, sprm))
;		{
ifdef DEBUG
	cCall	S_ValFromPropSprm,<[ppropBase], si>
else ;not DEBUG
	cCall	N_ValFromPropSprm,<[ppropBase], si>
endif ;DEBUG
	pop	dx  ;restore val
	sub	ax,dx
	je	CGPrl02

;		rgb[0] = sprm;
	mov	bx,[rgb]
	mov	[bx],si

;		cbPrl = dnsprm[sprm].cch;
	errnz	<cbEsprmMin - 4>
	shl	si,1
	shl	si,1
	errnz	<maskCchEsprm - 00Fh>
	mov	al,[si.dnsprm.cchEsprm]
	and	al,maskCchEsprm
	cbw

;		Assert(cbPrl == 2 || cbPrl == 3);
ifdef DEBUG
	cmp	al,2
	je	CGPrl01
	cmp	al,3
	je	CGPrl01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midInssubsn
	mov	bx,1009
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
CGPrl01:
endif ;DEBUG

;		if (cbPrl == 2)
;			rgb[1] = val;
;		else
;			bltb(&val, &rgb[1], 2);
	;Assembler note: Always move two bytes because with the least
	;significant byte first (on the 8086) the caller won't care.
	mov	[bx+1],dx

;		return cbPrl;
;		}
;	return 0;
CGPrl02:

;}
cEnd

;-------------------------------------------------------------------------
;	CbGenChpxFromChp(pchpResult, pchp, pchpBase, fWord3)
;-------------------------------------------------------------------------
;/* C B  G E N	C H P X  F R O M  C H P */
;/* generate a CHPX which expresses the differences between *pchpBase and
;*pchp	*/
;NATIVE int CbGenChpxFromChp(pchpResult, pchp, pchpBase, fWord3)
;struct  CHP *pchpResult;
;struct CHP *pchp;
;struct CHP *pchpBase;
;int fWord3;
;{
;	int cch;

; %%Function:N_CbGenChpxFromChp %%Owner:BRADV
cProc	N_CbGenChpxFromChp,<PUBLIC,FAR>,<si,di>
	ParmW	pchpResult
	ParmW	pchp
	ParmW	pchpBase

cBegin

;	SetWords(pchpResult, 0, cwCHP);
;	*(int *)pchpResult = ((*(int *)pchp) ^ (*(int *)pchpBase)) & (~maskFs);
	mov	bx,[pchpResult]
	mov	si,[pchp]
	mov	di,[pchpBase]
	lodsw
	xchg	ax,dx
	xor	dx,[di]
	errnz	<maskFsChp AND 000FFh>
	and	dh,NOT (maskFsChp SHR 8)
	push	ds
	pop	es
	inc	di
	inc	di

;	if (pchp->ftc != pchpBase->ftc)
;		{
;		pchpResult->ftc = pchp->ftc;
;		pchpResult->fsFtc = fTrue;
;		}
	errnz	<(ftcChp) - 2>
	lodsw
	scasw
	je	CGCFC01
	errnz	<(fsFtcChp) - 1>
	or	dh,maskFsFtcChp
	db	03Dh	;turns next "xor ax,ax" into "cmp ax,immediate"
CGCFC01:
	xor	ax,ax
	mov	[bx.ftcChp],ax

;	if (pchp->hps != pchpBase->hps)
;		{
;		pchpResult->hps = pchp->hps;
;		pchpResult->fsHps = fTrue;
;		}
	errnz	<(hpsChp) - 4>
	lodsb
	scasb
	je	CGCFC02
	errnz	<(fsHpsChp) - 1>
	or	dh,maskFsHpsChp
	db	03Dh	;turns next "xor al,al" into "cmp ax,immediate"
CGCFC02:
	xor	al,al
	mov	[bx.hpsChp],al

;	if (pchp->hpsPos != pchpBase->hpsPos)
;		{
;		pchpResult->hpsPos = pchp->hpsPos;
;		pchpResult->fsPos = fTrue;
;		}
	errnz	<(hpsPosChp) - 5>
	lodsb
	scasb
	je	CGCFC03
	errnz	<(fsPosChp) - 1>
	or	dh,maskFsPosChp
	db	03Dh	;turns next "xor al,al" into "cmp ax,immediate"
CGCFC03:
	xor	al,al
	mov	[bx.hpsPosChp],al

;	if (pchp->qpsSpace != pchpBase->qpsSpace)
;		{
;		pchpResult->qpsSpace = pchp->qpsSpace;
;		pchpResult->fsSpace = fTrue;
;		}
	lodsw
	mov	cx,[di]
	inc	di
	inc	di
	errnz	<(qpsSpaceChp) - 6>
	and	al,maskQpsSpaceChp
	and	cl,maskQpsSpaceChp
	cmp	al,cl
	je	CGCFC04
	errnz	<(fsSpaceChp) - 1>
	or	dh,maskFsSpaceChp
	db	03Dh	;turns next "xor al,al" into "cmp ax,immediate"
CGCFC04:
	xor	al,al
	mov	[bx.qpsSpaceChp],al

;	if (pchp->kul != pchpBase->kul)
;		{
;		pchpResult->kul = pchp->kul;
;		pchpResult->fsKul = fTrue;
;		}
;	if (pchp->ico != pchpBase->ico)
;		{
;		pchpResult->ico = pchp->ico;
;		pchpResult->fsIco = fTrue;
;		}
	errnz	<(kulChp) - 7>
	errnz	<(icoChp) - 7>
	mov	al,ah
	mov	cl,ch
	and	ax,maskKulChp + (maskIcoChp SHL 8)
	and	cx,maskKulChp + (maskIcoChp SHL 8)
	cmp	al,cl
	je	CGCFC05
	errnz	<(fsKulChp) - 1>
	or	dh,maskFsKulChp
	db	03Dh	;turns next "xor al,al" into "cmp ax,immediate"
CGCFC05:
	xor	al,al
	cmp	ah,ch
	je	CGCFC06
	errnz	<(fsIcoChp) - 1>
	or	dh,maskFsIcoChp
	db	03Dh	;turns next "xor ah,ah" into "cmp ax,immediate"
CGCFC06:
	xor	ah,ah
	or	al,ah
	mov	[bx.kulChp],al

;	if (pchp->fSpec)
;		pchpResult->fcPic = pchp->fcPic;
	mov	cx,8
	errnz	<(fSpecChp) - 1>
	test	[si.fSpecChp - 8],maskFSpecChp
	je	CGCFC07
	mov	cl,12
	errnz	<(fcPicChp) - 8>
	lea	di,[bx.fcPicChp]
	movsw
	movsw
CGCFC07:
	mov	[bx],dx

;	return CchNonZeroPrefix(pchpResult, cbCHP);
	;***Begin in-line CchNonZeroPrefix
	mov	di,bx
	add	di,cx
	dec	di
	xor	ax,ax
	std
	repe	scasb
	cld
	je	CGCFC08
	inc	cx
CGCFC08:
	xchg	ax,cx
	;***End in-line CchNonZeroPrefix
;}
cEnd

;-------------------------------------------------------------------------
;	CbGenPapxFromPap(ppapx, ppap, ppapBase, fWord3)
;-------------------------------------------------------------------------
;/* C B  G E N	P A P X  F R O M  P A P */
;/* creates a papx from grpprl.
;*/
;int CbGenPapxFromPap(ppapx, ppap, ppapBase, fWord3)
;char *ppapx;
;struct PAP *ppap;
;struct PAP *ppapBase;
;int fWord3;
;{

; %%Function:N_CbGenPapxFromPap %%Owner:BRADV
cProc	N_CbGenPapxFromPap,<PUBLIC,FAR>,<si,di>
	ParmW	ppapx
	ParmW	ppap
	ParmW	ppapBase

cBegin

;	int cb = CbGrpprlFromPap(fFalse, ppapx + cbPHE + 1, ppap, ppapBase, fFalse) + cbPHE + 1;
	mov	si,[ppap]
	mov	di,[ppapx]
	errnz	<fFalse>
	xor	ax,ax
	lea	bx,[di + cbPheMin + 1]
	cCall	CbGrpprlFromPap,<ax, bx, si, [ppapBase], ax>

;	*ppapx = ppap->stc;
	push	ds
	pop	es
	errnz	<(stcPap) - 0>
	movsb

;		{
;		bltb(&ppap->phe, ppapx + 1, sizeof(struct PHE));
	add	si,(phePap) - 1
	errnz	<cbPheMin - 6>
	movsw
	movsw
	movsw

;/* encode distinguished "standard" papx as length 0 */
;/* must be consistent with "bpapx == 0" case in CachePara */
;/* constant "240" or "15" must match corresponding value in CachePara */
;		if (cb == cbPHE + 1 && ppap->stc == 0 && ppapBase->stc == 0 &&
;			!ppap->phe.fDiffLines == 0 && ppap->phe.clMac == 1 &&
;			ppap->phe.dylLine == WinMac(240, 15) &&
;			ppap->phe.dxaCol == 7980)
;			return 0;
;			/* dylLine == 50 is 12 points on HP LaserJet */
;		}
	or	ax,ax
	jne	CGPFP01
	mov	bx,[ppapBase]
	mov	cl,[si.stcPap - (phePap) - cbPheMin]
	or	cl,[bx.stcPap]
	jne	CGPFP01
	errnz	<(fDiffLinesPhe) - 0>
	errnz	<(clMacPhe) - 1>
	mov	cx,wptr ([si.phePap.fDiffLinesPhe - (phePap) - cbPheMin])
	and	cl,maskFDiffLinesPhe
	cmp	cx,maskfDiffLinesPhe + (1 SHL 8)
	jne	CGPFP01
	cmp	[si.phePap.dylLinePhe - (phePap) - cbPheMin],240
	jne	CGPFP01
	cmp	[si.phePap.dxaColPhe - (phePap) - cbPheMin],7980
	je	CGPFP02

;/* we set the byte after the papx to 0, so FAddRun can properly check for
;   duplicates in FKP. */
;	*(ppapx + cb) = 0;
CGPFP01:
	add	di,ax
	mov	bptr [di],0
	add	ax,cbPheMin + 1

;	return cb;
	db	03Dh	;turns next "xor ax,ax" into "cmp ax,immediate"
CGPFP02:
	xor	ax,ax

;}
cEnd

sEnd	inssubs
        end
