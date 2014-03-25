        include w2.inc
        include noxport.inc
        include consts.inc
        include structs.inc

createSeg	edit_PCODE,edit2,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midEditn2	equ 22		 ; module ID, for native asserts
endif

; EXTERNAL FUNCTIONS

externFP	<N_PdodMother>
externFP	<IInPlcRef>
externFP	<CpPlc>
externFP	<GetPlc>
externFP	<PutCpPlc>
externFP	<InvalParaSect>
externFP	<XDeleteFields>
externFP	<XDeleteBkmks>
externFP	<N_FOpenPlc>
externFP	<N_QcpQfooPlcfoo>
externFP	<InvalCp1>
externFP	<InvalText>
externFP	<IInPlcCheck>
externFP	<FlushRulerSprms>
externFP	<FChngSizePhqLcb>
externFP	<CpMac2Doc>
externFP	<CpMacDoc>
externFP	<XDelReferencedText>
externFP	<XDelInHplcEdit>
externFP	<XDeleteHdrText>
externFP	<CpMac1Doc>
externFP	<XCopyFields>
externFP	<XCopyBkmks>
externFP	<XAddHdrText>
externFP	<FSectLimAtCp>
externFP	<XAddToHplcEdit>
externFP	<XAddReferencedText>
externFP	<CopyMultPlc>
externFP	<PutPlc>
externFP	<AdjustHplcCpsToLim>
externNP	<LN_PxsInit>
externNP	<LN_PostTn>
externNP	<LN_FDoTns>
externNP	<LN_CloseTns>
externFP	<AdjustHplc>
externFP	<N_AdjustCp>
externFP	<IInPlc>
externFP	<SetErrorMatProc>

ifdef DEBUG
externFP	<AssertProcForNative>
externFP	<FCkFldForDelete>
externFP	<S_XReplace>
externFP	<S_AdjustCp>
externFP	<S_XRepl1>
externFP	<AssertSzProc>
externFP	<LockHeap, UnlockHeap>
externFP	<S_FOpenPlc>
externFP	<S_FStretchPlc>
externFP	<S_XDelFndSedPgdPad>
externFP	<S_XReplaceCps>
endif ;DEBUG


sBegin  data

; EXTERNALS
externW     caPage
externW     caPara
externW     caSect
externW     caTable
externW     caTap
externW     mpdochdod
externW     vdocFetch
externW     vlcb
externW     vrulss
externW     vtcc
externW     vsab
externW     mpsbps
externW     caHdt
externW     vtcxs
externW     vdocFetchVisi
externW     vcaCell
externW     asd
externW     vfInCommit
externW     vfNoInval
externW     vmerr
externW     docSeqCache
externW     vcbc
externW     vdocScratch
externW		caTapAux

ifdef DEBUG
externW     cHpFreeze
externW     vdbs
externW     fDocScratchInUse
endif ;DEBUG


sEnd    data

; CODE SEGMENT _EDIT

sBegin	edit2
	assumes cs,edit2
        assumes ds,dgroup
        assumes ss,dgroup


;-------------------------------------------------------------------------
;	FReplace(pca, fn, fc, dfc)
;-------------------------------------------------------------------------
;/* F  R E P L A C E */
;/* Replace cpFirst through (cpLim-1) in doc by fc through (fc+dfc-1) in fn */
;BOOL FReplace(pca, fn, fc, dfc)
;struct CA *pca;
;int fn;
;FC fc, dfc;
;{
;    struct XBC xbc;
;    struct XSR *pxsr;
;#define ixsrReplaceMax 4
;    struct XSR rgxsr[ixsrReplaceMax];

; %%Function:N_FReplace %%Owner:BRADV
cProc	N_FReplace,<PUBLIC,FAR>,<si,di>
	ParmW	pca
	OFFBP_pca		=   -2
	ParmW	fn
	OFFBP_fn		=   -4
	ParmD	fc
	OFFBP_fc		=   -8
	ParmD	dfc
	OFFBP_dfc		=   -12

	LocalV	xbc,cbXbcMin
	LocalV	rgxsr,4*cbXsrMin

cBegin

;   if (vrulss.caRulerSprm.doc != docNil)
;	FlushRulerSprms();
	;LN_FlushRulerSprms performs
	;if (vrulss.caRulerSprm.doc != docNil)
	;    FlushRulerSprms();
	;ax, bx, cx, dx are altered.
	call	LN_FlushRulerSprms

;   pxsr = PxsInit(rgxsr, ixsrReplaceMax, &xbc);
	;LN_PxsInit takes pxs in si, ixsMax in cx, pxbc in bx
	;and performs PxsInit(pxs, ixsMax, pxbc).
	;ax, bx, cx, dx, di are altered.  The result is returned in si.
	lea	si,[rgxsr]
	mov	cx,4
	lea	bx,[xbc]
	call	LN_PxsInit

;   pxsr->pcaDel = pca;
;   pxsr->fn = fn;
;   pxsr->fc = fc;
;   pxsr->dfc = dfc;
	errnz	<OFFBP_fc - OFFBP_dfc - 4>
	errnz	<fcXsr - dfcXsr - 4>
	errnz	<OFFBP_fn - OFFBP_fc - 4>
	errnz	<fnXsr - fcXsr - 4>
	errnz	<OFFBP_pca - OFFBP_fn - 2>
	errnz	<pcaDelXsr - fnXsr - 2>
ifdef DEBUG
	;Assert es == ds with a call so as not to mess up short jumps.
	call	FR06
endif ;DEBUG
	push	si	;save pxsr
	mov	di,si
	lea	si,[dfc]
	mov	cx,(OFFBP_pca - OFFBP_dfc + 2) SHR 1
	rep	movsw
	pop	si	;restore pxsr


;   XReplace(fTrue, &xbc, pxsr);
	lea	di,[xbc]
	mov	ax,fTrue
	push	ax
	push	di
	push	si
ifdef DEBUG
	cCall	S_XReplace,<>
else ;!DEBUG
	push	cs
	call	near ptr N_XReplace
endif ;!DEBUG

;   if (!FDoTns(&xbc))
;	{
	;LN_FDoTns takes pxbc in di and performs FDoTns(pxbc).
	;fTrue is returned iff carry is true upon return.
	;ax, bx, cx, dx are altered.
	call	LN_FDoTns
	jc	FR02

;	SetErrorMat(matReplace);
	mov	ax,matReplace
        cCall   SetErrorMatProc,<ax>

;	return fFalse;
;	}
	jmp	short FR05

FR02:
;   BeginCommit();
;#define BeginCommit()	 AssertDo(!vfInCommit++)
ifdef DEBUG
	cmp	[vfInCommit],0
	je	FR03
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn2
	mov	bx,1001
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FR03:
endif ;DEBUG
	inc	[vfInCommit]

;   XReplace(fFalse, &xbc, pxsr);
	xor	ax,ax
	push	ax
	push	di
	push	si
ifdef DEBUG
	cCall	S_XReplace,<>
else ;!DEBUG
	push	cs
	call	near ptr N_XReplace
endif ;!DEBUG

;   EndCommit();
;#define EndCommit()	 AssertDo(!--vfInCommit)
	dec	[vfInCommit]
ifdef DEBUG
	je	FR04
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn2
	mov	bx,1002
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FR04:
endif ;DEBUG

;   CloseTns(&xbc);
	;LN_CloseTns takes &xbc in di and performs CloseTns.
	;ax, bx, cx, dx are altered.
	call	LN_CloseTns

;   return fTrue;
	db	0B8h	;turns next "xor ax,ax" into "mov ax,immediate"
FR05:
	xor	ax,ax
;}
cEnd

ifdef DEBUG
FR06:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,ds
	mov	bx,es
	cmp	ax,bx
	je	FR07
	mov	ax,midEditn2
	mov	bx,1003
	cCall	AssertProcForNative,<ax,bx>
FR07:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG


;-------------------------------------------------------------------------
;	XReplace(fPlan, pxbc, pxsr)
;-------------------------------------------------------------------------
;/* X  R E P L A C E */
;/* Perform a bifuricated replacement */
;XReplace(fPlan, pxbc, pxsr)
;BOOL fPlan;
;struct XBC *pxbc;
;struct XSR *pxsr;
;{
;    struct CA *pca = pxsr->pcaDel;

; %%Function:N_XReplace %%Owner:BRADV
cProc	N_XReplace,<PUBLIC,FAR>,<si,di>
	ParmW	fPlan
	ParmW	pxbc
	ParmW	pxsr

ifdef DEBUG
	LocalV	rgchAttempt,42
	LocalV	rgchEditn,10
endif ;DEBUG

cBegin
	mov	di,[pxsr]
	mov	si,[di.pcaDelXsr]

;   Assert(pca->cpFirst >= cp0 && pca->cpLim <= CpMac1Doc(pca->doc));
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	[si.HI_cpFirstCa],0
	jge	XR01
	mov	ax,midEditn2
	mov	bx,1004
	cCall	AssertProcForNative,<ax,bx>
XR01:
	cCall	CpMac1Doc,<[si.docCa]>
	sub	ax,[si.LO_cpLimCa]
	sbb	dx,[si.HI_cpLimCa]
	jge	XR02
	mov	ax,midEditn2
	mov	bx,1005
	cCall	AssertProcForNative,<ax,bx>
XR02:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

;   /*	check that deleted portion may be deleted WRT fields */
;   AssertSz(!vdbs.fCkFldDel || FCkFldForDelete(pca->doc, pca->cpFirst, pca->cpLim),
;	    "Attempt to delete unmatched field char! " );
;#define AssertSz(f,sz) ((f) ? 0 : AssertSzProc(SzFrame(sz),(CHAR *)szAssertFile,__LINE__))
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	[vdbs.fCkFldDelDbs],fFalse
	je	Ltemp005
	push	[si.docCa]
	push	[si.HI_cpFirstCa]
	push	[si.LO_cpFirstCa]
	push	[si.HI_cpLimCa]
	push	[si.LO_cpLimCa]
	cCall	FCkFldForDelete,<>
	or	ax,ax
	je	Ltemp006
Ltemp005:
	jmp	XR04
Ltemp006:
	push	si
	push	di
	push	ds
	push	es
	push	cs
	pop	ds
	push	ss
	pop	es
	mov	si,offset szAttempt1
	lea	di,[rgchAttempt]
	mov	cx,cbSzAttempt1
	rep	movsb
	jmp	short XR03
szAttempt1:
	db	'Attempt to delete unmatched field char! ',0
cbSzAttempt1 equ $ - szAttempt1
	errnz	<cbSzAttempt1 - 41>
XR03:
	mov	si,offset szEditn1
	lea	di,[rgchEditn]
	mov	cx,cbSzEditn1
	rep	movsb
	jmp	short XR035
szEditn1:
	db	'editn.asm',0
cbSzEditn1 equ $ - szEditn1
	errnz	<cbSzEditn1 - 10>
XR035:
	pop	es
	pop	ds
	pop	di
	pop	si
	lea	ax,[rgchAttempt]
	lea	bx,[rgchEditn]
	mov	cx,1027
	cCall	AssertSzProc,<ax,bx,cx>
XR04:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

;   if (DcpCa(pca) != 0)
;/* delete structures for text being deleted */
;	XDeleteStruct(fPlan, pxbc, pxsr);
	;Assumes bx = pxbc, cx = fPlan, si = pca, di = pxsr
	;ax, bx, cx, dx are altered.
	mov	bx,[pxbc]
	mov	cx,[fPlan]
	call	LN_XDeleteStruct

;	 if (!fPlan && !vfNoInval)
;		 /* may call CachePara or FetchCp, must call BEFORE changing piece tbl */
;		 InvalText (pca, fTrue /* fEdit */);
	mov	ax,[vfNoInval]
	or	ax,[fPlan]
	jne	XR05
	errnz	<fTrue - fFalse - 1>
	inc	ax
	cCall	InvalText,<si,ax>
XR05:

;   XRepl1(fPlan, pxbc, pxsr);
	push	[fPlan]
	push	[pxbc]
	push	di
ifdef DEBUG
	cCall	S_XRepl1,<>
else ;!DEBUG
	call	LN_XRepl1
endif ;!DEBUG

;   if (!fPlan)
;	{
	cmp	[fPlan],fFalse
	jne	XR10

;	if (!vfNoInval)
;	    InvalCp1(pca);
;	else
;	    /* inval the caches even if vfNoInval on */
;	    InvalCaFierce();
	;LN_DoInval assumes pca passed in si and performs
	;if (!vfNoInval) { InvalCp1(pca); }
	;else InvalCaFierce();
	;ax, bx, cx, dx are altered.
	call	LN_DoInval

;	AdjustCp(pca, pxsr->dfc);
;	}
	push	si
	push	[di.HI_dfcXsr]
	push	[di.LO_dfcXsr]
ifdef DEBUG
	cCall	S_AdjustCp,<>
else ;!DEBUG
	push	cs
	call	near ptr N_AdjustCp
endif ;!DEBUG
XR10:

;}
cEnd


;-------------------------------------------------------------------------
;	IpcdSplit(hplcpcd, cp)
;-------------------------------------------------------------------------
;/* I P C D  S P L I T */
;/* Ensure cp is the beginning of a piece. Return index of that piece. */
;/* NATIVE (pj 3/9): pcode version takes 4% of detokenize time and 2.4% of RTF time */
;NATIVE int IpcdSplit(hplcpcd, cp)
;struct PLC **hplcpcd;
;CP cp;
;{
;   int ipcd;
;   struct PCD pcd;
;   CP dcp;

; %%Function:N_IpcdSplit %%Owner:BRADV
cProc	N_IpcdSplit,<PUBLIC,FAR>,<di>
	ParmW	hplcpcd
	ParmD	cp

cBegin
	mov	di,[hplcpcd]
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	call	LN_IpcdSplit
cEnd


	;LN_IpcdSplit takes hplcpcd in di, cp in dx:ax and performs
	;IpcdSplit(hplcpcd, cp).  The result is returned in ax.
	;ax, bx, cx, dx are altered.
; %%Function:LN_IpcdSplit %%Owner:BRADV
PUBLIC LN_IpcdSplit
LN_IpcdSplit:
;   vdocFetch = docNil; /* ensure fetch cache isn't lying */
;   if ((ipcd = IInPlcCheck(hplcpcd, cp)) == -1)
;	return(IMacPlc(hplcpcd));
	push	si	;save caller's si
	push	dx
	push	ax	;save cp
	push	di	;argument for IInPlcCheck
	push	dx	;argument for IInPlcCheck
	push	ax	;argument for IInPlcCheck
	errnz	<docNil>
	xor	ax,ax
	mov	[vdocFetch],ax
	cCall	IInPlcCheck,<>
	xchg	ax,si
	inc	si
	je	IS02
	dec	si

;   if ((dcp = cp - CpPlc(hplcpcd, ipcd)) != cp0)
;	{{ /* !NATIVE (at least 50% of calls don't hit this code) */
	cCall	CpPlc,<di, si>
	pop	bx
	pop	cx	;restore cp
	sub	ax,bx
	sbb	dx,cx
	push	ax
	or	ax,dx
	pop	ax
	jne	IS05
IS005:
	xchg	ax,si
IS01:
	pop	si	;restore caller's si
	ret

IS02:
	mov	bx,[di]
	mov	bx,[bx]
	mov	si,[bx.iMacPlcStr]
IS03:
	pop	ax
	pop	dx	;restore cp
	jmp	short IS005
IS04:
	pop	di	;restore hplcpcd
	pop	ax
	pop	dx	;restore -dcp
	mov	ax,matReplace
        cCall   SetErrorMatProc,<ax>
	mov	si,iNil
	jmp	short IS03
IS05:

;	Assert(!vfInCommit);
ifdef DEBUG
	call	IS07
endif ;DEBUG

;/* Insert a new piece flush with the one at ipcd */
;	if (!FOpenPlc(hplcpcd, ++ipcd, 1))
;	    {
	push	di	;save hplcpcd
	push	dx
	push	ax	;save -dcp
	push	cx
	push	bx	;save cp
	inc	si
	mov	ax,1
ifdef DEBUG
	cCall	S_FOpenPlc,<di, si, ax>
else ;not DEBUG
	cCall	N_FOpenPlc,<di, si, ax>
endif ;DEBUG
	xchg	ax,cx
	jcxz	IS04

;	    SetErrorMat( matReplace );
;	    return iNil;
;	    }

;	QcpQfooPlcfoo takes hplcfoo in bx, ifoo in si, it returns pplcfoo
;	in bx, cbfoo in cx, qcp in es:di, qfoo in es:si.
;	if DEBUG it returns hpcp in dx:di, hpfoo in dx:si.
;	Changes ax, bx, cx, dx, si, di.
	push	si	;save ipcd
	mov	bx,di
ifdef DEBUG
	xor	di,di	;Not O.K. to pass ifldMac
endif ;DEBUG
	cCall	N_QcpQfooPlcfoo,<>
ifdef DEBUG
	;Check that es == mpsbps[dx];
	call	IS09
endif ;DEBUG
	pop	ax	;restore ipcd
	pop	cx
	pop	dx	;restore cp

;	PutCpPlc(hplcpcd, ipcd, cp);
	cmp	ax,[bx.icpAdjustPlc]
	jl	IS06
	sub	cx,[bx.LO_dcpAdjustPlc]
	sbb	dx,[bx.HI_dcpAdjustPlc]
IS06:
	mov	es:[di],cx
	mov	es:[di+2],dx

;	/* We are doing effectively:
;	    pcd.fn = pcdPrev.fn;
;	    pcd.fc = pcdPrev.fc + dcp;
;	    pcd.prm = pcdPrev.prm;
;	    pcd.fNoParaLastValid = pcdPrev.fNoParaLast;
;	    pcd.fNoParaLast = pcdPrev.fNoParaLast;
;	*/
;	GetPlc(hplcpcd, ipcd - 1, &pcd);
;	pcd.fc += dcp;
;	PutPlc(hplcpcd, ipcd, &pcd);
	pop	cx
	pop	dx	;restore -dcp
	push	es
	pop	ds
	mov	di,si
	sub	si,cbPcdMin
	push	ax	;save ipcd
	errnz	<cbPcdMin - 8>
	movsw
	errnz	<(LO_fcPcd) - 2>
	lodsw
	sub	ax,cx
	stosw
	errnz	<(HI_fcPcd) - 4>
	lodsw
	sbb	ax,dx
	stosw
	movsw
	push	ss
	pop	ds
	pop	ax	;restore ipcd
	pop	di	;restore hplcpcd

;	}}
;   return ipcd;
;}
	jmp	short IS01

ifdef DEBUG
	;Do this assert with a call so as not to mess up short jumps.
IS07:
	cmp	[vfInCommit],fFalse
	je	IS08
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn2
	mov	bx,1006
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
IS08:
	ret
endif ;DEBUG

ifdef DEBUG
IS09:
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
	jc	IS10
	;Assembler note: There is no way we should have to call ReloadSb here.
;	reload sb trashes ax, cx, and dx
;	cCall	ReloadSb,<>
	mov	ax,midEditn2
	mov	bx,1007
	cCall	AssertProcForNative,<ax,bx>
IS10:
	pop	ax	;restore es from QcpQfooPlcfoo
	mov	bx,es	;compare with es rederived from the SB of QcpQfooPlcfoo
	cmp	ax,bx
	je	IS11
	mov	ax,midEditn2
	mov	bx,1008
	cCall	AssertProcForNative,<ax,bx>
IS11:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */


;-------------------------------------------------------------------------
;	XRepl1(fPlan, pxbc, pxsr)
;-------------------------------------------------------------------------
;/* X  R E P L 1 */
;/* perform Bifuricated replacement */
;/* NATIVE (pj 3/9): taking 5.6% of RTF time, 9.9% of detokenize time */
;NATIVE XRepl1(fPlan, pxbc, pxsr)
;BOOL fPlan;
;struct XBC *pxbc;
;struct XSR *pxsr;
;{
;   struct PLC **hplcpcd;
;   int ipcdFirst, ipcdLim;
;   int cpcd;
;   struct PCD pcd;
;   struct PCD pcdPrev;

ifdef DEBUG
; %%Function:N_XRepl1 %%Owner:BRADV
cProc	N_XRepl1,<PUBLIC,FAR>,<si,di>
else ;!DEBUG
; %%Function:LN_XRepl1 %%Owner:BRADV
cProc	LN_XRepl1,<PUBLIC,NEAR>,<si,di>
endif ;!DEBUG
	ParmW	fPlan
	ParmW	pxbc
	ParmW	pxsr

	LocalW	OFF_qpcd
cBegin

;   hplcpcd = PdodDoc(pxsr->pcaDel->doc)->hplcpcd;
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	mov	di,[pxsr]
	mov	si,[di.pcaDelXsr]
	call	LN_PdodDocCa
	mov	di,[bx.hplcpcdDod]

;   if (fPlan)
;	{
	mov	ax,[si.LO_cpFirstCa]
	mov	dx,[si.HI_cpFirstCa]
	cmp	[fPlan],fFalse
	je	XR103
	;Requires dx:ax = pxsr->pcaDel->cpFirst,
	;si is pxsr->pcaDel, di is hplcpcd

;	pxsr->fNotEmptyPcaDel = (pxsr->pcaDel->cpFirst != pxsr->pcaDel->cpLim);
	mov	bx,[si.LO_cpLimCa]
	mov	cx,[si.HI_cpLimCa]
	sub	bx,ax
	sbb	cx,dx
	or	cx,bx
	mov	bx,[pxsr]
	mov	[bx.fNotEmptyPcaDelXsr],cx

;	if ((ipcdFirst = IpcdSplit(hplcpcd, pxsr->pcaDel->cpFirst))
;		== iNil)
;	    {
;LPostAbort:
;	    PostTn(pxbc, tntAbort, NULL, 0);
;	    return;
;	    }
	;LN_IpcdSplit takes hplcpcd in di, cp in dx:ax and performs
	;IpcdSplit(hplcpcd, cp).  The result is returned in ax.
	;ax, bx, cx, dx are altered.
	call	LN_IpcdSplit
	mov	bx,[pxsr]
	errnz	<iNil - (-1)>
	inc	ax
	je	XR102
	dec	ax

;	if (!pxsr->fNotEmptyPcaDel)
;	    cpcd = 0;
	mov	cx,[bx.fNotEmptyPcaDelXsr]
	jcxz	XR101

;	else
;	    {
;	    int ipcdLim;
;	    if ((ipcdLim = IpcdSplit(hplcpcd, pxsr->pcaDel->cpLim))
;		    == iNil)
;		goto LPostAbort;
	push	ax	;save ipcdFirst
	mov	ax,[si.LO_cpLimCa]
	mov	dx,[si.HI_cpLimCa]
	;LN_IpcdSplit takes hplcpcd in di, cp in dx:ax and performs
	;IpcdSplit(hplcpcd, cp).  The result is returned in ax.
	;ax, bx, cx, dx are altered.
	call	LN_IpcdSplit
	mov	bx,[pxsr]
	pop	cx	;restore ipcdFirst
	errnz	<iNil - (-1)>
	inc	ax
	je	XR102
	dec	ax

;	    cpcd = ipcdFirst - ipcdLim;
;	    }
	sub	cx,ax

XR101:
;	pxsr->cpcd = cpcd;
;	}
	mov	[bx.cpcdXsr],cx

	;Assembler note: the following assert is performed below in the
	;C source.
;   /* number of pieces to be added (negative or zero) */
;   Assert(cpcd <= 0);
ifdef DEBUG
	;Do this assert with a call so as not to mess up short jumps.
	call	XR110
endif ;DEBUG
	;Assembler note: Why check fPlan twice?  Do the stuff in the following
	;"if (fPlan)" here.
;	PostTn(pxbc, tntHplc, hplcpcd, cpcd+1);
	;LN_PostTn takes pxbc in bx, tnt in ax, c in cx, h in dx and
	;performs PostTn(pxbc, tnt, h, c).
	;ax, bx, cx, dx are altered.
	mov	ax,tntHplc
	mov	dx,di
	inc	cx
XR1015:
	mov	bx,[pxbc]
	call	LN_PostTn
	jmp	XR109

XR102:
	;LN_PostTn takes pxbc in bx, tnt in ax, c in cx, h in dx and
	;performs PostTn(pxbc, tnt, h, c).
	;ax, bx, cx, dx are altered.
	mov	ax,tntAbort
	errnz	<NULL>
	xor	cx,cx
	xor	dx,dx
	jmp	short XR1015

;   else
;	{
XR103:
	;Assume dx:ax = pxsr->pcaDel->cpFirst,
	;si is pxsr->pcaDel, di is hplcpcd
;	ipcdFirst = IpcdSplit(hplcpcd, pxsr->pcaDel->cpFirst);
	;LN_IpcdSplit takes hplcpcd in di, cp in dx:ax and performs
	;IpcdSplit(hplcpcd, cp).  The result is returned in ax.
	;ax, bx, cx, dx are altered.
	call	LN_IpcdSplit
	mov	bx,[pxsr]

;	cpcd = pxsr->cpcd;
;	Assert(!pxsr->fNotEmptyPcaDel ||
;		IInPlc(hplcpcd, pxsr->pcaDel->cpLim) == ipcdFirst - cpcd);
ifdef DEBUG
	;Do this assert with a call so as not to mess up short jumps.
	call	XR112
endif ;DEBUG

;	/* set so vhprc chain is checked when we run out of memory */
;	vmerr.fReclaimHprcs = fTrue;
	or	[vmerr.fReclaimHprcsMerr],maskFReclaimHprcsMerr

;	}

;   /* number of pieces to be added (negative or zero) */
;   Assert(cpcd <= 0);
ifdef DEBUG
	;Do this assert with a call so as not to mess up short jumps.
	call	XR110
endif ;DEBUG

;   if (fPlan)
;	/* simplified, may be one less */
;	PostTn(pxbc, tntHplc, hplcpcd, cpcd+1);
	;Assembler note: the "if (fPlan)" case is done above
	;in the assembler version.

;   else
;	{
;	if (ipcdFirst > 0)
;	    GetPlc(hplcpcd, ipcdFirst - 1, &pcdPrev);
;
;	if (pxsr->dfc == fc0 ||
;		(ipcdFirst > 0 && pcdPrev.fn == pxsr->fn &&
;		pcdPrev.prm == prmNil && pcdPrev.fc +
;		(pxsr->pcaDel->cpFirst - CpPlc(hplcpcd,ipcdFirst-1))
;		== pxsr->fc))
;	    /* Either pure delete or extension of previous piece */
;	    {
	mov	cx,[bx.LO_dfcXsr]
	or	cx,[bx.HI_dfcXsr]
	je	XR106	    ;carry clear, pass cpcd to FOpenPlc
	cmp	ax,1
	jc	XR106	    ;carry set, pass cpcd+1 to FOpenPlc
;	QcpQfooPlcfoo takes hplcfoo in bx, ifoo in si, it returns pplcfoo
;	in bx, cbfoo in cx, qcp in es:di, qfoo in es:si.
;	if DEBUG it returns hpcp in dx:di, hpfoo in dx:si.
;	Changes ax, bx, cx, dx, si, di.
	push	di	    ;save hplcpcd
	push	ax	    ;save ipcdFirst
	push	si	    ;save pxsr->pcaDel
	xchg	ax,si
	dec	si
	mov	bx,di
ifdef DEBUG
	xor	di,di	;Not O.K. to pass ifldMac
endif ;DEBUG
	cCall	N_QcpQfooPlcfoo,<>
ifdef DEBUG
	;Check that es == mpsbps[dx];
	call	XR114
endif ;DEBUG

;		(ipcdFirst > 0 && pcdPrev.fn == pxsr->fn &&
;		pcdPrev.prm == prmNil && pcdPrev.fc +
;		(pxsr->pcaDel->cpFirst - CpPlc(hplcpcd,ipcdFirst-1))
;		== pxsr->fc))
	mov	[OFF_qpcd],si	    ;save for later
	push	bx	;save pplcpcd
	mov	bx,[pxsr]
	mov	cx,[bx.fnXsr]
	mov	ax,[bx.LO_fcXsr]
	mov	dx,[bx.HI_fcXsr]
	pop	bx	;restore pplcpcd
	cmp	es:[si.fnPcd],cl
	jne	XR105
	cmp	es:[si.prmPcd],prmNil
	jne	XR105
	sub	ax,es:[si.LO_fcPcd]
	sbb	dx,es:[si.HI_fcPcd]
	pop	si	;restore pxsr->pcaDel
	pop	cx	;restore ipcdFirst
	push	cx	;save ipcdFirst
	push	si	;save pxsr->pcaDel
	sub	ax,[si.LO_cpFirstCa]
	sbb	dx,[si.HI_cpFirstCa]
	;***Begin in-line CpPlc
	add	ax,es:[di]
	adc	dx,es:[di+2]
	cmp	cx,[bx.icpAdjustPlc]
	;Assembler note: use jle instead of jl here because cx is ipcd+1,
	;not ipcd.
	jle	XR104
	add	ax,[bx.LO_dcpAdjustPlc]
	adc	dx,[bx.HI_dcpAdjustPlc]
XR104:
	;***End in-line CpPlc
	or	ax,dx
XR105:
	pop	si	;restore pxsr->pcaDel
	pop	ax	;restore ipcdFirst
	pop	di	;restore hplcpcd
;	    FOpenPlc(hplcpcd, ipcdFirst, cpcd);
;	    if (pxsr->dfc != fc0)
;		/* If extending, say we might have inserted Eop*/
;		{
;		Debug(pcdPrev.fNoParaLastValid = fFalse);
;		pcdPrev.fNoParaLast = fFalse;
;		PutPlc(hplcpcd, ipcdFirst - 1, &pcdPrev);
;		}
;	    }
;	else
;	    /* Insert one piece */
;	    {
;	    AssertDo(FOpenPlc(hplcpcd, ipcdFirst, cpcd + 1));
	stc
	jne	XR106	;carry set, pass cpcd+1 to FOpenPlc
	;Assembler note: set the FNoParaLast flags before the call
	;to FOpenPlc rather than after because we have the pointers now.
	mov	bx,[OFF_qpcd]	    ;restore from above
ifdef DEBUG
	errnz	<(fNoParaLastValidPcd) - (fNoParaLastPcd)>
	and	es:[bx.fNoParaLastPcd],NOT (maskFNoParaLastPcd + maskFNoParaLastValidPcd)
else ;!DEBUG
	and	es:[bx.fNoParaLastPcd],NOT maskFNoParaLastPcd
endif ;!DEBUG
			;carry clear, pass cpcd to FOpenPlc
	;ax = ipcdFirst, si = pxsr->pcaDel, di = hplcpcd,
	;we want to pass cpcd + carry to FOpenPlc
XR106:
	push	ax	;save ipcdFirst
	pushf
	mov	bx,[pxsr]
	mov	cx,[bx.cpcdXsr]
	adc	cx,0
ifdef DEBUG
	cCall	S_FOpenPlc,<di, ax, cx>
else ;not DEBUG
	cCall	N_FOpenPlc,<di, ax, cx>
endif ;DEBUG
ifdef DEBUG
	;Perform the AssertDo with a call so as not to mess up short jumps.
	call	XR117
endif ;DEBUG
	popf
	pop	ax	;restore ipcdFirst
	jnc	XR108	;if we called FOpenPlc with cpcd we're done
;	QcpQfooPlcfoo takes hplcfoo in bx, ifoo in si, it returns pplcfoo
;	in bx, cbfoo in cx, qcp in es:di, qfoo in es:si.
;	if DEBUG it returns hpcp in dx:di, hpfoo in dx:si.
;	Changes ax, bx, cx, dx, si, di.
	push	di	    ;save hplcpcd
	push	ax	    ;save ipcdFirst
	push	si	    ;save pxsr->pcaDel
	xchg	ax,si
	mov	bx,di
ifdef DEBUG
	xor	di,di	;Not O.K. to pass ifldMac
endif ;DEBUG
	cCall	N_QcpQfooPlcfoo,<>
ifdef DEBUG
	;Check that es == mpsbps[dx];
	call	XR114
endif ;DEBUG

;	    GetPlc(hplcpcd, ipcdFirst, &pcd);
;	    PutCpPlc(hplcpcd, ipcdFirst, pxsr->pcaDel->cpFirst);
;	    pcd.fn = pxsr->fn;
;	    pcd.fc = pxsr->fc;
;	    pcd.prm = prmNil;
;	    Debug(pcd.fNoParaLastValid = fFalse);
;	    pcd.fNoParaLast = fFalse; /* Para state unknown */
;	    pcd.fPaphNil = fFalse;
;	    pcd.fCopied = fTrue;
;	    PutPlc(hplcpcd, ipcdFirst, &pcd);
	push	bx	;save pplcpcd
	mov	bx,[pxsr]
ifdef DEBUG
	errnz	<(fNoParaLastValidPcd) - (fNoParaLastPcd)>
endif ;!DEBUG
	errnz	<(fPaphNilPcd) - (fNoParaLastPcd)>
	errnz	<(fCopiedPcd) - (fNoParaLastPcd)>
	mov	cl,maskFCopiedPcd
	errnz	<(fnPcd) - (fNoParaLastPcd) - 1>
	mov	ch,bptr ([bx.fnXsr])
	mov	wptr (es:[si.fNoParaLastPcd]),cx
	mov	ax,[bx.LO_fcXsr]
	mov	dx,[bx.HI_fcXsr]
	mov	es:[si.LO_fcPcd],ax
	mov	es:[si.HI_fcPcd],dx
	pop	bx	;restore pplcpcd
	mov	es:[si.prmPcd],prmNil
	pop	si	;restore pxsr->pcaDel
	pop	ax	;restore ipcdFirst
	mov	cx,[si.LO_cpFirstCa]
	mov	dx,[si.HI_cpFirstCa]
	;***Begin in-line PutCpPlc
	cmp	ax,[bx.icpAdjustPlc]
	jl	XR107
	sub	cx,[bx.LO_dcpAdjustPlc]
	sbb	dx,[bx.HI_dcpAdjustPlc]
XR107:
	mov	es:[di],cx
	mov	es:[di+2],dx
	;***End in-line PutCpPlc
	pop	di	    ;restore hplcpcd

;	    ipcdFirst++;
;	    }
	inc	ax
XR108:

;	AdjustHplc(hplcpcd, pxsr->pcaDel->cpLim, pxsr->dfc - pxsr->pcaDel->cpLim +
;		pxsr->pcaDel->cpFirst, ipcdFirst);
	;ax = ipcdFirst, si = pxsr->pcaDel, di = hplcpcd
	mov	bx,[pxsr]
	push	di
	push	[si.HI_cpLimCa]
	push	[si.LO_cpLimCa]
	mov	dx,[bx.HI_dfcXsr]
	mov	cx,[bx.LO_dfcXsr]
	sub	cx,[si.LO_cpLimCa]
	sbb	dx,[si.HI_cpLimCa]
	add	cx,[si.LO_cpFirstCa]
	adc	dx,[si.HI_cpFirstCa]
	push	dx
	push	cx
	push	ax
	push	cs
	call	near ptr AdjustHplc

;	InvalVisiCache();
;#define InvalVisiCache() vdocFetchVisi = docNil; vcbc.w = 0
	errnz	<docNil - 0>
	xor	ax,ax
	mov	[vdocFetchVisi],ax
        mov     [vcbc],ax

;	InvalCellCache();
;#define InvalCellCache() (vcaCell.doc = docNil)
	mov	[vcaCell.docCa],ax

;	}
XR109:

;}
cEnd

;   Assert(cpcd <= 0);
ifdef DEBUG
XR110:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	bx,[pxsr]
	cmp	[bx.cpcdXsr],0
	jle	XR111
	mov	ax,midEditn2
	mov	bx,1009
	cCall	AssertProcForNative,<ax,bx>
XR111:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG

;	Assert(!pxsr->fNotEmptyPcaDel ||
;		IpcdSplit(hplcpcd, pxsr->pcaDel->cpLim) == ipcdFirst - cpcd);
ifdef DEBUG
XR112:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	bx,[pxsr]
	cmp	[bx.fNotEmptyPcaDelXsr],0
	je	XR113
	sub	ax,[bx.cpcdXsr]
	push	ax	;save ipcdFirst - cpcd
	mov	dx,[si.HI_cpLimCa]
	mov	ax,[si.LO_cpLimCa]
	;LN_IpcdSplit takes hplcpcd in di, cp in dx:ax and performs
	;IpcdSplit(hplcpcd, cp).  The result is returned in ax.
	;ax, bx, cx, dx are altered.
	call	LN_IpcdSplit
	pop	cx	;restore ipcdFirst - cpcd
	cmp	ax,cx
	je	XR113
	mov	ax,midEditn2
	mov	bx,1010
	cCall	AssertProcForNative,<ax,bx>
XR113:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG

ifdef DEBUG
XR114:
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
	jc	XR115
	;Assembler note: There is no way we should have to call ReloadSb here.
;	reload sb trashes ax, cx, and dx
;	cCall	ReloadSb,<>
	mov	ax,midEditn2
	mov	bx,1011
	cCall	AssertProcForNative,<ax,bx>
XR115:
	pop	ax	;restore es from QcpQfooPlcfoo
	mov	bx,es	;compare with es rederived from the SB of QcpQfooPlcfoo
	cmp	ax,bx
	je	XR116
	mov	ax,midEditn2
	mov	bx,1012
	cCall	AssertProcForNative,<ax,bx>
XR116:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

;	    AssertDo(FOpenPlc(hplcpcd, ipcdFirst, cpcd + carry));
ifdef DEBUG
XR117:
	or	ax,ax
	jne	XR118
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn2
	mov	bx,1013
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
XR118:
	ret
endif ;DEBUG


	;LN_DoInval assumes pca passed in si and performs
	;if (!vfNoInval) { InvalCp1(pca); }
	;else InvalCaFierce();
	;ax, bx, cx, dx are altered.
LN_DoInval:
	mov	ax,[vfNoInval]
	or	ax,ax
	jne	LN_InvalCaFierce
	errnz	<fTrue - 1>
	inc	ax
	cCall	InvalCp1,<si>
	ret

;-------------------------------------------------------------------------
;	InvalCaFierce()
;-------------------------------------------------------------------------
;/* I n v a l	C a   F i e r c e */
;/* NATIVE (pj 3/9) called very frequently for operations with vfNoInval */
;NATIVE InvalCaFierce()
;{
;/* unconditionally invalidate CAs and other important docs */
;	 InvalLlc();
;#define InvalLlc()
; %%Function:N_InvalCaFierce %%Owner:BRADV
PUBLIC N_InvalCaFierce
N_InvalCaFierce:
	call	LN_InvalCaFierce
	db	0CBh	    ;far ret

LN_InvalCaFierce:
	errnz	<docNil>
	xor	ax,ax

;	 vdocFetch = docNil;
	mov	[vdocFetch],ax

;	 caSect.doc = docNil;
	mov	[caSect.docCa],ax

;	 caPara.doc = docNil;
	mov	[caPara.docCa],ax

;	 caPage.doc = docNil;
	mov	[caPage.docCa],ax

;	 caTable.doc = docNil;
	mov	[caTable.docCa],ax

;	 caTap.doc = docNil;
	mov	[caTap.docCa],ax

;	 caHdt.doc = docNil;
	mov	[caHdt.docCa],ax

;	 vtcc.ca.doc = docNil;
	mov	[vtcc.caTcc.docCa],ax

;	 vtcc.caTap.doc = docNil;
	mov	[vtcc.caTapTcc.docCa],ax

;	 caTapAux.doc = docNil;
	mov	[caTapAux.docCa],ax

;	 vtcxs.ca.doc = docNil;
	mov	[vtcxs.caTcxs.docCa],ax

;	 Win( vlcb.ca.doc = docNil );
	mov	[vlcb.caLcb.docCa],ax

;	 Win( InvalVisiCache() );
;#define InvalVisiCache() (vdocFetchVisi = docNil)
	mov	[vdocFetchVisi],ax

;	 Win( InvalCellCache() );
;#define InvalCellCache() (vcaCell.doc = docNil)
	mov	[vcaCell.docCa],ax

;	Win( docSeqCache = docNil );
	mov	[docSeqCache],ax

;}
	ret


;-------------------------------------------------------------------------
;	FRepl1(fPlan, pxbc, pxsr)
;-------------------------------------------------------------------------
;/* F  R E P L 1 */
;/* delete pca and insert the specified piece (does not do checking or
;adjustment) */
;BOOL FRepl1(pca, fn, fc, dfc)
;struct CA *pca;
;int fn;
;FC fc, dfc;
;{
;   struct XBC xbc;
;   struct XSR *pxsr;
;#define ixsrRepl1Max 1
;   struct XSR rgxsr[ixsrRepl1Max];

; %%Function:N_FRepl1 %%Owner:BRADV
cProc	N_FRepl1,<PUBLIC,FAR>,<si,di>
	ParmW	pca
	OFFBP_pca		=   -2
	ParmW	fn
	OFFBP_fn		=   -4
	ParmD	fc
	OFFBP_fc		=   -8
	ParmD	dfc
	OFFBP_dfc		=   -12

	LocalV	xbc,cbXbcMin
	LocalV	rgxsr,1*cbXsrMin
cBegin

;   pxsr = (struct XSR *)PxsInit(rgxsr, ixsrRepl1Max, &xbc);
	;LN_PxsInit takes pxs in si, ixsMax in cx, pxbc in bx
	;and performs PxsInit(pxs, ixsMax, pxbc).
	;ax, bx, cx, dx, di are altered.  The result is returned in si.
	lea	si,[rgxsr]
	mov	cx,1
	lea	bx,[xbc]
	call	LN_PxsInit

;   pxsr->pcaDel = pca;
;   pxsr->fn = fn;
;   pxsr->fc = fc;
;   pxsr->dfc = dfc;
	errnz	<OFFBP_fc - OFFBP_dfc - 4>
	errnz	<fcXsr - dfcXsr - 4>
	errnz	<OFFBP_fn - OFFBP_fc - 4>
	errnz	<fnXsr - fcXsr - 4>
	errnz	<OFFBP_pca - OFFBP_fn - 2>
	errnz	<pcaDelXsr - fnXsr - 2>
ifdef DEBUG
	;Assert es == ds with a call so as not to mess up short jumps.
	call	FR105
endif ;DEBUG
	push	si	;save pxsr
	mov	di,si
	lea	si,[dfc]
	mov	cx,(OFFBP_pca - OFFBP_dfc + 2) SHR 1
	rep	movsw
	pop	si	;restore pxsr

;   XRepl1(fTrue, &xbc, pxsr);
	mov	ax,fTrue
	lea	di,[xbc]
	push	ax
	push	di
	push	si
ifdef DEBUG
	cCall	S_XRepl1,<>
else ;!DEBUG
	call	LN_XRepl1
endif ;!DEBUG

;   if (!FDoTns(&xbc))
;	{
	;LN_FDoTns takes pxbc in di and performs FDoTns(pxbc).
	;fTrue is returned iff carry is true upon return.
	;ax, bx, cx, dx are altered.
	call	LN_FDoTns
	jc	FR101

;	SetErrorMat(matReplace);
;	return fFalse;
	mov	ax,matReplace
        cCall   SetErrorMatProc,<ax>
	jmp	short FR104

;	}
FR101:

;   BeginCommit();
;#define BeginCommit()	 AssertDo(!vfInCommit++)
ifdef DEBUG
	cmp	[vfInCommit],0
	je	FR102
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn2
	mov	bx,1014
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FR102:
endif ;DEBUG
	inc	[vfInCommit]

;   XRepl1(fFalse, &xbc, pxsr);
	errnz	<fFalse>
	xor	ax,ax
	push	ax
	push	di
	push	si
ifdef DEBUG
	cCall	S_XRepl1,<>
else ;!DEBUG
	call	LN_XRepl1
endif ;!DEBUG

;   EndCommit();
;#define EndCommit()	 AssertDo(!--vfInCommit)
	dec	[vfInCommit]
ifdef DEBUG
	je	FR103
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn2
	mov	bx,1015
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FR103:
endif ;DEBUG

;   CloseTns(&xbc);
	;LN_CloseTns takes &xbc in di and performs CloseTns.
	;ax, bx, cx, dx are altered.
	call	LN_CloseTns

;   return fTrue;
	db	0B8h	;turns next "xor ax,ax" into "mov ax,immediate"
FR104:
	xor	ax,ax
;}
cEnd

ifdef DEBUG
FR105:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,ds
	mov	bx,es
	cmp	ax,bx
	je	FR106
	mov	ax,midEditn2
	mov	bx,1016
	cCall	AssertProcForNative,<ax,bx>
FR106:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG


;-------------------------------------------------------------------------
;	FReplaceCps(pcaDel, pcaIns)
;-------------------------------------------------------------------------
;/* F  R E P L A C E  C P S */
;/* General replace routine */
;/* Replace characters from [pcaDel->cpFirst,pcaDel->cpLim) with characters
;   from [pcaIns->cpFirst,pcaIns->cpLim) */
;BOOL FReplaceCps(pcaDel, pcaIns)
;struct CA *pcaDel, *pcaIns;
;{
;   struct XBC xbc;
;   struct XSR *pxsr;
;#define ixsrReplaceCpsMax WinMac(10,8)
;   struct XSR rgxsr[ixsrReplaceCpsMax];

; %%Function:N_FReplaceCps %%Owner:BRADV
cProc	N_FReplaceCps,<PUBLIC,FAR>,<si,di>
	ParmW	pcaDel
	ParmW	pcaIns

	LocalV	xbc,cbXbcMin
	LocalV	rgxsr,10*cbXsrMin
cBegin

;   Assert(pcaDel->cpFirst >= cp0 && pcaDel->cpLim <= CpMac1Doc(pcaDel->doc));
;   Assert(DcpCa(pcaDel) >= cp0 && DcpCa(pcaIns) >= cp0);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	push	si
	mov	si,[pcaDel]
	cmp	[si.HI_cpFirstCa],0
	jge	FRC01
	mov	ax,midEditn2
	mov	bx,1017
	cCall	AssertProcForNative,<ax,bx>
FRC01:
	cCall	CpMac1Doc,<[si.docCa]>
	sub	ax,[si.LO_cpLimCa]
	sbb	dx,[si.HI_cpLimCa]
	jge	FRC02
	mov	ax,midEditn2
	mov	bx,1018
	cCall	AssertProcForNative,<ax,bx>
FRC02:
	;LN_DcpCa assumes pca passed in si and returns DcpCa in dx:ax.
	;Only ax and dx are altered.
	call	LN_DcpCa
	or	dx,dx
	jge	FRC03
	mov	ax,midEditn2
	mov	bx,1019
	cCall	AssertProcForNative,<ax,bx>
FRC03:
	mov	si,[pcaIns]
	;LN_DcpCa assumes pca passed in si and returns DcpCa in dx:ax.
	;Only ax and dx are altered.
	call	LN_DcpCa
	or	dx,dx
	jge	FRC04
	mov	ax,midEditn2
	mov	bx,1020
	cCall	AssertProcForNative,<ax,bx>
FRC04:
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

;   if (vrulss.caRulerSprm.doc != docNil)
;	FlushRulerSprms();
	;LN_FlushRulerSprms performs
	;if (vrulss.caRulerSprm.doc != docNil)
	;    FlushRulerSprms();
	;ax, bx, cx, dx are altered.
	call	LN_FlushRulerSprms

;   pxsr = (struct XSR *)PxsInit(rgxsr, ixsrReplaceCpsMax, &xbc);
	;LN_PxsInit takes pxs in si, ixsMax in cx, pxbc in bx
	;and performs PxsInit(pxs, ixsMax, pxbc).
	;ax, bx, cx, dx, di are altered.  The result is returned in si.
	lea	si,[rgxsr]
	mov	cx,10
	lea	bx,[xbc]
	call	LN_PxsInit

;   pxsr->pcaDel = pcaDel;
;   pxsr->pcaIns = pcaIns;
	mov	ax,[pcaDel]
	mov	[si.pcaDelXsr],ax
	mov	ax,[pcaIns]
	mov	[si.pcaInsXsr],ax

;   XReplaceCps(fTrue, &xbc, pxsr);
	mov	ax,fTrue
	lea	di,[xbc]
	push	ax
	push	di
	push	si
ifdef DEBUG
	cCall	S_XReplaceCps,<>
else ;!DEBUG
	push	cs
	call	near ptr N_XReplaceCps
endif ;!DEBUG

;   if (!FDoTns(&xbc))
;	{
	;LN_FDoTns takes pxbc in di and performs FDoTns(pxbc).
	;fTrue is returned iff carry is true upon return.
	;ax, bx, cx, dx are altered.
	call	LN_FDoTns
	jc	FRC05

;	SetErrorMat(matReplace);
;	return fFalse;
	mov	ax,matReplace
        cCall   SetErrorMatProc,<ax>
	jmp	short FRC08

;	}
FRC05:

;   BeginCommit();
;#define BeginCommit()	 AssertDo(!vfInCommit++)
ifdef DEBUG
	cmp	[vfInCommit],0
	je	FRC06
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn2
	mov	bx,1021
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FRC06:
endif ;DEBUG
	inc	[vfInCommit]

;   XReplaceCps(fFalse, &xbc, pxsr);
	errnz	<fFalse>
	xor	ax,ax
	push	ax
	push	di
	push	si
ifdef DEBUG
	cCall	S_XReplaceCps,<>
else ;!DEBUG
	push	cs
	call	near ptr N_XReplaceCps
endif ;!DEBUG

;   EndCommit();
;#define EndCommit()	 AssertDo(!--vfInCommit)
	dec	[vfInCommit]
ifdef DEBUG
	je	FRC07
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn2
	mov	bx,1022
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FRC07:
endif ;DEBUG

;   CloseTns(&xbc);
	;LN_CloseTns takes &xbc in di and performs CloseTns.
	;ax, bx, cx, dx are altered.
	call	LN_CloseTns

;   return fTrue;
	db	0B8h	;turns next "xor ax,ax" into "mov ax,immediate"
FRC08:
	xor	ax,ax

;}
cEnd


;-------------------------------------------------------------------------
;	XReplaceCps(fPlan, pxbc, pxsr)
;-------------------------------------------------------------------------
;/* X  R E P L A C E  C P S */
;XReplaceCps(fPlan, pxbc, pxsr)
;BOOL fPlan;
;struct XBC *pxbc;
;struct XSR *pxsr;
;{
;   struct PLC **hplcpcdDest, **hplcpcdSrc;
;   int ipcdFirst, ipcdInsFirst, ipcdLim;
;   int cpcd;
;   struct CA *pcaDel = pxsr->pcaDel;
;   struct CA *pcaIns = pxsr->pcaIns;
;   int docDel = pcaDel->doc;
;   int docIns = pcaIns->doc;
;   CP dcpDel = DcpCa(pcaDel);
;   CP dcpIns = DcpCa(pcaIns);
;   struct DOD **hdodSrc = mpdochdod[docIns];
;   struct DOD **hdodDest = mpdochdod[docDel];
;   struct PCD pcd;

; %%Function:N_XReplaceCps %%Owner:BRADV
cProc	N_XReplaceCps,<PUBLIC,FAR>,<si,di>
	ParmW	fPlan
	ParmW	pxbc
	ParmW	pxsr

	LocalW	ipcdFirst
	LocalW	ipcdLim
	LocalW	ipcdInsFirst
	LocalW	hplcpcdSrc
	LocalW	hplcpcdDest
	LocalD	dcpIns
	LocalD	dcpDel
	LocalV	pcd,cbPcdMin
	LocalV	caDel,cbCaMin
	LocalV	caIns,cbCaMin
	LocalV	xbc,cbXbcMin
	LocalV	rgxsr,10*cbXsrMin
ifdef DEBUG
	LocalV	rgchAttempt,42
	LocalV	rgchEditn,10
endif ;DEBUG

cBegin

;#ifdef DEBUG
ifdef DEBUG

;   if (fPlan)
;	/* no point doing all this twice! */
;	{
	cmp	[fPlan],fFalse
	jne	Ltemp009
	jmp	XRC12
Ltemp009:
	push	ax
	push	bx
	push	cx
	push	dx
	push	si

;	Assert(pcaDel->cpFirst >= cp0 && pcaDel->cpLim <= CpMac1Doc(docDel));
	mov	bx,[pxsr]
	mov	si,[bx.pcaDelXsr]
	cmp	[si.HI_cpFirstCa],0
	jge	XRC01
	mov	ax,midEditn2
	mov	bx,1023
	cCall	AssertProcForNative,<ax,bx>
XRC01:
	cCall	CpMac1Doc,<[si.docCa]>
	sub	ax,[si.LO_cpLimCa]
	sbb	dx,[si.HI_cpLimCa]
	jge	XRC02
	mov	ax,midEditn2
	mov	bx,1024
	cCall	AssertProcForNative,<ax,bx>
XRC02:

;	Assert(dcpDel >= 0 && dcpIns >= cp0);
	;LN_DcpCa assumes pca passed in si and returns DcpCa in dx:ax.
	;Only ax and dx are altered.
	mov	bx,[pxsr]
	mov	si,[bx.pcaDelXsr]
	call	LN_DcpCa
	or	dx,dx
	jge	XRC03
	mov	ax,midEditn2
	mov	bx,1025
	cCall	AssertProcForNative,<ax,bx>
XRC03:
	;LN_DcpCa assumes pca passed in si and returns DcpCa in dx:ax.
	;Only ax and dx are altered.
	mov	bx,[pxsr]
	mov	si,[bx.pcaInsXsr]
	call	LN_DcpCa
	or	dx,dx
	jge	XRC04
	mov	ax,midEditn2
	mov	bx,1026
	cCall	AssertProcForNative,<ax,bx>
XRC04:

;	Assert(docDel != docIns);
	mov	bx,[pxsr]
	mov	si,[bx.pcaDelXsr]
	mov	ax,[si.docCa]
	mov	si,[bx.pcaInsXsr]
	cmp	ax,[si.docCa]
	jne	XRC05
	mov	ax,midEditn2
	mov	bx,1027
	cCall	AssertProcForNative,<ax,bx>
XRC05:

;	/* assured by caller */
;	Assert(vrulss.caRulerSprm.doc == docNil);
	cmp	[vrulss.caRulerSprmRulss.docCa],docNil
	je	XRC06
	mov	ax,midEditn2
	mov	bx,1028
	cCall	AssertProcForNative,<ax,bx>
XRC06:

;	/* assure that if vdocScratch is being used, it has been "Acquired" */
;	Assert ((vdocScratch == docNil || (docDel != vdocScratch &&
;		docIns != vdocScratch) || fDocScratchInUse));
	mov	ax,[vdocScratch]
	cmp	ax,docNil
	je	XRC08
	cmp	[fDocScratchInUse],fFalse
	jne	XRC08
	cmp	ax,[si.docCa]
	je	XRC07
	mov	bx,[pxsr]
	mov	bx,[bx.pcaDelXsr]
	cmp	ax,[bx.docCa]
	jne	XRC08
XRC07:
	mov	ax,midEditn2
	mov	bx,1029
	cCall	AssertProcForNative,<ax,bx>
XRC08:

;	/*  check that deleted portion is legal WRT fields */
;	AssertSz(!vdbs.fCkFldDel || FCkFldForDelete(docDel, pcaDel->cpFirst, pcaDel->cpLim)
;		&& FCkFldForDelete(pcaIns->doc, pcaIns->cpFirst, pcaIns->cpLim),
;		"Attempt to delete unmatched field char!");
;#define AssertSz(f,sz) ((f) ? 0 : AssertSzProc(SzFrame(sz),(CHAR *)szAssertFile,__LINE__))
	cmp	[vdbs.fCkFldDelDbs],fFalse
	je	Ltemp007
	mov	bx,[pxsr]
	mov	si,[bx.pcaDelXsr]
	push	[si.docCa]
	push	[si.HI_cpFirstCa]
	push	[si.LO_cpFirstCa]
	push	[si.HI_cpLimCa]
	push	[si.LO_cpLimCa]
	cCall	FCkFldForDelete,<>
	or	ax,ax
	je	Ltemp008
	mov	bx,[pxsr]
	mov	si,[bx.pcaInsXsr]
	push	[si.docCa]
	push	[si.HI_cpFirstCa]
	push	[si.LO_cpFirstCa]
	push	[si.HI_cpLimCa]
	push	[si.LO_cpLimCa]
	cCall	FCkFldForDelete,<>
	or	ax,ax
	je	Ltemp008
Ltemp007:
	jmp	XRC11
Ltemp008:
	push	si
	push	di
	push	ds
	push	es
	push	cs
	pop	ds
	push	ss
	pop	es
	mov	si,offset szAttempt2
	lea	di,[rgchAttempt]
	mov	cx,cbSzAttempt2
	rep	movsb
	jmp	short XRC09
szAttempt2:
	db	'Attempt to delete unmatched field char! ',0
cbSzAttempt2 equ $ - szAttempt2
	errnz	<cbSzAttempt2 - 41>
XRC09:
	mov	si,offset szEditn2
	lea	di,[rgchEditn]
	mov	cx,cbSzEditn2
	rep	movsb
	jmp	short XRC10
szEditn2:
	db	'editn.asm',0
cbSzEditn2 equ $ - szEditn2
	errnz	<cbSzEditn2 - 10>
XRC10:
	pop	es
	pop	ds
	pop	di
	pop	si
	lea	ax,[rgchAttempt]
	lea	bx,[rgchEditn]
	mov	cx,1027
	cCall	AssertSzProc,<ax,bx,cx>
XRC11:
;	}
;#endif /* DEBUG */
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
XRC12:
endif ;DEBUG

;   if (dcpIns == 0)
;	/* This is just too easy . . . */
;	{
	mov	di,[pxsr]
	mov	si,[di.pcaInsXsr]
	;LN_DcpCa assumes pca passed in si and returns DcpCa in dx:ax.
	;Only ax and dx are altered.
	call	LN_DcpCa
	mov	[OFF_dcpIns],ax
	mov	[SEG_dcpIns],dx
	or	ax,dx
	jne	XRC13

;	pxsr->fn = fnNil;
;	pxsr->fc = fc0;
;	pxsr->dfc = fc0;
	errnz	<fcXsr - dfcXsr - 4>
	errnz	<fnXsr - fcXsr - 4>
	push	ds
	pop	es
	errnz	<fnNil - 0>
	xor	ax,ax
	push	di	;save pxsr
	errnz	<([dfcXsr]) - 0>
	errnz	<(([fnXsr]) - ([dfcXsr])) AND 1>
	mov	cx,(([fnXsr]) - ([dfcXsr]) + 2) SHR 1
	rep	stosw
	pop	di	;restore pxsr

;	XReplace(fPlan, pxbc, pxsr);
	push	[fPlan]
	push	[pxbc]
	push	di
ifdef DEBUG
	cCall	S_XReplace,<>
else ;!DEBUG
	push	cs
	call	near ptr N_XReplace
endif ;!DEBUG

;	return;
;	}
	jmp	XRC40
XRC13:

;   hplcpcdDest = (*hdodDest)->hplcpcd;
	;Assembler note: We initialize hplcpcdDest when
	;it is most convenient, not here.

;   hplcpcdSrc = (*hdodSrc)->hplcpcd;
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	call	LN_PdodDocCa
	mov	cx,[bx.hplcpcdDod]
	mov	[hplcpcdSrc],cx

;   if (dcpDel != cp0)
;	{
;/* delete structures for text being deleted */
;	XDeleteStruct(fPlan, pxbc, pxsr);
	;Assumes bx = pxbc, cx = fPlan, si = pca, di = pxsr
	;ax, bx, cx, dx are altered.
	mov	bx,[pxbc]
	mov	cx,[fPlan]
	call	LN_XDeleteStruct

;   if (!fPlan && !vfNoInval)
;	InvalParaSect(pcaDel, pcaIns, fFalse);
	mov	ax,[vfNoInval]
	or	ax,[fPlan]
	jne	XRC14
	cCall	InvalParaSect,<si, [di.pcaInsXsr], ax>
XRC14:

;/* Get the first and last pieces for insertion */
;   if (fPlan)
;	{
;	ipcdInsFirst = pxsr->ipcdInsFirst =
;		IInPlc(hplcpcdSrc, pcaIns->cpFirst);
;	pxsr->ipcdInsLast = IInPlc(hplcpcdSrc, pcaIns->cpLim - 1);
;	pxsr->fNotEmptyPcaDel = (dcpDel != 0);
;	}
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	call	LN_PdodDocCa
	mov	cx,[bx.hplcpcdDod]
	mov	[hplcpcdDest],cx
	;LN_DcpCa assumes pca passed in si and returns DcpCa in dx:ax.
	;Only ax and dx are altered.
	call	LN_DcpCa
	mov	[OFF_dcpDel],ax
	mov	[SEG_dcpDel],dx
	cmp	[fPlan],fFalse
	je	XRC15
	or	ax,dx
	mov	[di.fNotEmptyPcaDelXsr],ax
	mov	bx,[di.pcaInsXsr]
	mov	cx,[hplcpcdSrc]
	push	cx		    ;for IInPlc(hplcpcdSrc, pcaIns->cpFirst);
	push	[bx.HI_cpFirstCa]   ;for IInPlc(hplcpcdSrc, pcaIns->cpFirst);
	push	[bx.LO_cpFirstCa]   ;for IInPlc(hplcpcdSrc, pcaIns->cpFirst);
	mov	ax,-1
	cwd
	add	ax,[bx.LO_cpLimCa]
	adc	dx,[bx.HI_cpLimCa]
	cCall	IInPlc,<cx,dx,ax>
	mov	[di.ipcdInsLastXsr],ax
	cCall	IInPlc,<>
	mov	[di.ipcdInsFirstXsr],ax

;   else
;	ipcdInsFirst = pxsr->ipcdInsFirst;
XRC15:
	mov	ax,[di.ipcdInsFirstXsr]
	mov	[ipcdInsFirst],ax

;/* get the limiting pieces for deletion */
;   if (fPlan)
;	{
;	ipcdFirst = IpcdSplit(hplcpcdDest, pcaDel->cpFirst);
	;LN_IpcdSplit takes hplcpcd in di, cp in dx:ax and performs
	;IpcdSplit(hplcpcd, cp).  The result is returned in ax.
	;ax, bx, cx, dx are altered.
	push	[di.fNotEmptyPcaDelXsr]
	mov	di,[hplcpcdDest]
	mov	dx,[si.HI_cpFirstCa]
	mov	ax,[si.LO_cpFirstCa]
	call	LN_IpcdSplit
	mov	[ipcdFirst],ax
	pop	cx	;restore pxsr->fNotEmptyPcaDel

;	ipcdLim = (pxsr->fNotEmptyPcaDel) ?
;		IpcdSplit(hplcpcdDest, pcaDel->cpLim) : ipcdFirst;
;
;/* check for failure of IpcdSplit */
;	if (ipcdFirst == iNil || ipcdLim == iNil)
;	    {
;	    PostTn(pxbc, tntAbort, NULL, 0);
;	    return;
;	    }
	cmp	[fPlan],fFalse
	je	XRC19
	jcxz	XRC16
	;LN_IpcdSplit takes hplcpcd in di, cp in dx:ax and performs
	;IpcdSplit(hplcpcd, cp).  The result is returned in ax.
	;ax, bx, cx, dx are altered.
	mov	dx,[si.HI_cpLimCa]
	mov	ax,[si.LO_cpLimCa]
	call	LN_IpcdSplit
XRC16:
	mov	[ipcdLim],ax
	errnz	<iNil - (-1)>
	inc	ax
	je	XRC17
	mov	ax,[ipcdFirst]
	errnz	<iNil - (-1)>
	inc	ax
	jne	XRC18
XRC17:
	;LN_PostTn takes pxbc in bx, tnt in ax, c in cx, h in dx and
	;performs PostTn(pxbc, tnt, h, c).
	;ax, bx, cx, dx are altered.
	mov	ax,tntAbort
	mov	bx,[pxbc]
	errnz	<NULL>
	xor	cx,cx
	xor	dx,dx
	call	LN_PostTn
	jmp	XRC40
XRC18:

;	/* number of pieces to be added */
;	pxsr->cpcd = cpcd = ipcdFirst - ipcdLim + pxsr->ipcdInsLast - ipcdInsFirst +1;
;	}
	mov	dx,di	    ;save hplcpcdDest for call to PostTn
	mov	di,[pxsr]
	mov	ax,[ipcdFirst]
	sub	ax,[ipcdLim]
	add	ax,[di.ipcdInsLastXsr]
	sub	ax,[ipcdInsFirst]
	inc	ax
	mov	[di.cpcdXsr],ax

	;Assembler note: This PostTn call has been moved from below
	;LN_PostTn takes pxbc in bx, tnt in ax, c in cx, h in dx and
	;performs PostTn(pxbc, tnt, h, c).
	;ax, bx, cx, dx are altered.
;	PostTn(pxbc, tntHplc, hplcpcdDest, cpcd);
	xchg	ax,cx
	mov	ax,tntHplc
	mov	bx,[pxbc]
	call	LN_PostTn
	jmp	XRC27

;   else
;	{
;	ipcdFirst = IpcdSplit(hplcpcdDest, pcaDel->cpFirst);
;	cpcd = pxsr->cpcd;
XRC19:
	;Assembler note: ipcdFirst has already been computed
	mov	di,[pxsr]

;	Assert(cpcd == ipcdFirst + pxsr->ipcdInsLast - ipcdInsFirst + 1
;		- ((pxsr->fNotEmptyPcaDel) ?
;		IpcdSplit(hplcpcdDest, pcaDel->cpLim) : ipcdFirst));
;	}
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[ipcdFirst]
	;LN_IpcdSplit takes hplcpcd in di, cp in dx:ax and performs
	;IpcdSplit(hplcpcd, cp).  The result is returned in ax.
	;ax, bx, cx, dx are altered.
	cmp	[di.fNotEmptyPcaDelXsr],fFalse
	je	XRC20
	push	di	;save pxsr
	mov	di,[hplcpcdDest]
	mov	dx,[si.HI_cpLimCa]
	mov	ax,[si.LO_cpLimCa]
	call	LN_IpcdSplit
	pop	di	;restore pxsr
XRC20:
	neg	ax
	add	ax,[ipcdFirst]
	add	ax,[di.ipcdInsLastXsr]
	sub	ax,[ipcdInsFirst]
	inc	ax
	cmp	ax,[di.cpcdXsr]
	je	XRC21
	mov	ax,midEditn2
	mov	bx,1030
	cCall	AssertProcForNative,<ax,bx>
XRC21:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

;   if (fPlan)
;	PostTn(pxbc, tntHplc, hplcpcdDest, cpcd);
;
;   else
;	{
	;Assembler note:  The fPlan test, and the call to PostTn have been
	;done above
;		if (!vfNoInval)
;			/* may call CachePara or FetchCp, must call BEFORE changing piece tbl */
;			InvalText (pcaDel, fTrue /* fEdit */);
	mov	ax,[vfNoInval]
	or	ax,ax
	jne	XRC215
	errnz	<fTrue - fFalse - 1>
	inc	ax
	cCall	InvalText,<si,ax>
XRC215:

;/* set so vhprc chain is checked when we run out of memory */
;	vmerr.fReclaimHprcs = fTrue;
	or	[vmerr.fReclaimHprcsMerr],maskFReclaimHprcsMerr

;/* adjust pctb size; get pointer to the first new piece, ppcdDest, and to the
;first piece we are inserting. */
;	AssertDo(FOpenPlc(hplcpcdDest, ipcdFirst, cpcd));
	push	[hplcpcdDest]
	push	[ipcdFirst]
	push	[di.cpcdXsr]
ifdef DEBUG
	cCall	S_FOpenPlc,<>
else ;not DEBUG
	cCall	N_FOpenPlc,<>
endif ;DEBUG
ifdef DEBUG
	;Perform the AssertDo with a call so as not to mess up short jumps.
	call	XRC41
endif ;DEBUG
    
;	FreezeHp();
ifdef	DEBUG
	call	LN_FreezeHpEdit
endif ;DEBUG

;/* ensure rgcp in hplcpcdSrc is adjusted before we copy cps. */
;	if (((struct PLC *)*hplcpcdSrc)->icpAdjust < ipcdInsFirst + 1)
;	    AdjustHplcCpsToLim(hplcpcdSrc, ipcdInsFirst + 1);
	mov	si,[hplcpcdSrc]
	mov	bx,[si]
	mov	ax,[ipcdInsFirst]
	push	ax	;save ipcdInsFirst
	inc	ax
	cmp	[bx.icpAdjustPlc],ax
	jge	XRC22
	push	si
	push	ax
	push	cs
	call	near ptr AdjustHplcCpsToLim
XRC22:
	pop	ax	;restore ipcdInsFirst
    
;/* fill first new piece and split it appropriately */
;	GetPlc(hplcpcdSrc, ipcdInsFirst, &pcd);
	push	si	;argument for CpPlc
	push	ax	;argument for CpPlc
	lea	bx,[pcd]
	cCall	GetPlc,<si, ax, bx>

;	pcd.fc += (pcaIns->cpFirst - CpPlc(hplcpcdSrc, ipcdInsFirst));
	cCall	CpPlc,<>
	mov	si,[di.pcaInsXsr]
	sub	ax,[si.LO_cpFirstCa]
	sbb	dx,[si.HI_cpFirstCa]
	sub	[pcd.LO_fcPcd],ax
	sbb	[pcd.HI_fcPcd],dx

;	pcd.fCopied = fTrue; /* para heights invalid */
	or	[pcd.fCopiedPcd],maskFCopiedPcd

;	PutPlc(hplcpcdDest, ipcdFirst, &pcd);
;	PutCpPlc(hplcpcdDest, ipcdFirst, pcaDel->cpFirst);
;	ipcdLim = ipcdFirst + 1;
	mov	si,[di.pcaDelXsr]
	mov	cx,[hplcpcdDest]
	mov	ax,[ipcdFirst]
	push	cx		    ;argument for PutCpPlc
	push	ax		    ;argument for PutCpPlc
	push	[si.HI_cpFirstCa]   ;argument for PutCpPlc
	push	[si.LO_cpFirstCa]   ;argument for PutCpPlc
	lea	bx,[pcd]
	push	cx
	push	ax
	push	bx
	inc	ax
	mov	[ipcdLim],ax
	cCall	PutPlc,<>
	cCall	PutCpPlc,<>
    
;	if ((cpcd = pxsr->ipcdInsLast - ipcdInsFirst) != 0)
;	    {
	mov	cx,[di.ipcdInsLastXsr]
	sub	cx,[ipcdInsFirst]
	jcxz	XRC23

;/* fill in rest of inserted pieces */
;	    ipcdLim += cpcd;
	add	[ipcdLim],cx

;	    CopyMultPlc(cpcd, hplcpcdSrc, ipcdInsFirst + 1,
;		    hplcpcdDest, ipcdFirst + 1,
;		    pcaDel->cpFirst - pcaIns->cpFirst, 0, 0);
	push	cx
	push	[hplcpcdSrc]
	mov	cx,[ipcdInsFirst]
	inc	cx
	push	cx
	push	[hplcpcdDest]
	mov	cx,[ipcdFirst]
	inc	cx
	push	cx
	mov	ax,[si.LO_cpFirstCa]
	mov	dx,[si.HI_cpFirstCa]
	mov	bx,[di.pcaInsXsr]
	sub	ax,[bx.LO_cpFirstCa]
	sbb	dx,[bx.HI_cpFirstCa]
	push	dx
	push	ax
	xor	cx,cx
	push	cx
	push	cx
	cCall	CopyMultPlc,<>

;	    }
XRC23:
    
;/* adjust rest of pieces in destination doc */
;	AdjustHplc(hplcpcdDest, pcaDel->cpLim, /*dcpAdj*/dcpIns - dcpDel,
;		ipcdLim);
	push	[hplcpcdDest]
	push	[si.HI_cpLimCa]
	push	[si.LO_cpLimCa]
	mov	ax,[OFF_dcpIns]
	mov	dx,[SEG_dcpIns]
	sub	ax,[OFF_dcpDel]
	sbb	dx,[SEG_dcpDel]
	push	dx
	push	ax
	push	[ipcdLim]
	push	cs
	call	near ptr AdjustHplc
    
;/* and inform anyone else who cares */
    
;	(*hdodDest)->fFormatted |= (pcaIns->doc == docScrap) ?
;		vsab.fFormatted : (*hdodSrc)->fFormatted;
;	PdodMother(docDel)->fMayHavePic |= (docIns == docScrap) ?
;		vsab.fMayHavePic : PdodMother(docIns)->fMayHavePic;
	mov	si,[di.pcaInsXsr]
	errnz	<(fMayHavePicSab) - (fFormattedSab) - 1>
	mov	ax,wptr ([vsab.fFormattedSab])
	and	ax,maskFFormattedSab + (maskFMayHavePicSab SHL 8)
	add	al,0FFh
	sbb	al,al
	add	ah,0FFh
	sbb	ah,ah
	cmp	[si.docCa],docScrap
	je	XRC24
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	call	LN_PdodDocCa
	mov	al,[bx.fFormattedDod]
	push	ax	;save flags
	cCall	N_PdodMother,<[si.docCa]>
	xchg	ax,bx
	pop	ax	;restore flags
	mov	ah,[bx.fMayHavePicDod]
XRC24:
	and	ax,maskFFormattedDod + (maskFMayHavePicDod SHL 8)
	mov	si,[di.pcaDelXsr]
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	call	LN_PdodDocCa
	or	[bx.fFormattedDod],al
	push	ax	;save flags
	cCall	N_PdodMother,<[si.docCa]>
	xchg	ax,bx
	pop	ax	;restore flags
	or	[bx.fMayHavePicDod],ah

;	if (!vfNoInval)
;	    {
;	    InvalCp1(pcaDel);
;	    Win( InvalText (pcaDel, fTrue /* fEdit */) );
;	    }
;	else
;	    /* inval the caches even if vfNoInval on */
;	    InvalCaFierce();
	;LN_DoInval assumes pca passed in si and performs
	;if (!vfNoInval) { InvalCp1(pca); }
	;else InvalCaFierce();
	;ax, bx, cx, dx are altered.
	call	LN_DoInval

;	/* invalidate FetchCpPccpVisible */
;#define InvalVisiCache() (vdocFetchVisi = docNil)
;#define InvalCellCache() (vcaCell.doc = docNil)
;	InvalVisiCache();
;	InvalCellCache();
	;FUTURE:  It seems like we should not have to invalidate
	; vdocFetchVisi and vcaCell here because both InvalCaFierce
	; and InvalCp1 do it for us, but InvalText calls InvalDde
	; which indirectly causes vcaCell to get set up again if
	; there is a table.
	errnz	<docNil>
	xor	ax,ax
	mov	[vdocFetchVisi],ax
	mov	[vcaCell.docCa],ax

;	MeltHp();
ifdef	DEBUG
	call	LN_MeltHpEdit
endif ;DEBUG

;	AdjustCp(pcaDel, dcpIns);
;/* NOTE after this point pcaDel->cpLim may be untrustworthy because it may
;   have been adjusted as a side effect of AdjustCp (eg. selCur.ca) */
	push	si
	push	[SEG_dcpIns]
	push	[OFF_dcpIns]
ifdef DEBUG
	cCall	S_AdjustCp,<>
else ;!DEBUG
	push	cs
	call	near ptr N_AdjustCp
endif ;!DEBUG

;	}
XRC27:

;/* copy enclosed structures and subdocs */
;/*  copy any enclosed fields */
;   if (FFieldsInPca(pcaIns))
;	XCopyFields(fPlan, pxbc, pcaIns, pcaDel);
	;LN_FFieldsInPca assumes pca passed in si and performs
	;FFieldsInPca(pca).  ax, bx, cx, dx are altered.
	;The sign bit set reflects a true result
	mov	si,[di.pcaInsXsr]
	call	LN_FFieldsInPca
	jns	XRC28
	cCall	XCopyFields,<[fPlan], [pxbc], si, [di.pcaDelXsr]>
XRC28:

;/* page table: if there is a table to be updated, call routine. Even if
;the source table is empty, the destination will have to be invalidated. */
;   if ((*hdodDest)->hplcpgd)
;	{
;	XAddToHplcEdit(fPlan, pxbc, &pxsr->xsaPgdCopy, pcaIns, pcaDel,
;		edcPgd);
;	InvalLlc();
;#define InvalLlc()
;	}
;#define edcPgd  (offset(DOD, hplcpgd) / sizeof(int))
	;XRC43 performs
	;hplc = *(((int *)(PdodDoc(((struct CA *)si)->doc))) + al);
	;if (hplc != hNil)
	;   XAddToHplcEdit(fPlan, pxbc, ((char *)pxsr) + dx,
	;	((struct XSR *)di)->pcaIns, ((struct XSR *)di)->pcaDel, al);
	;ax, bx, cx and dx are altered.
	mov	si,[di.pcaDelXsr]
	mov	dx,([xsaPgdDelXsr])
	mov	al,([hplcpgdDod]) SHR 1
	call	XRC43

;   if (!(*hdodSrc)->fShort && !(*hdodDest)->fShort)
;	{
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	call	LN_PdodDocCa
	push	wptr ([bx.fShortDod])
	mov	si,[di.pcaInsXsr]
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	call	LN_PdodDocCa
	pop	ax
	or	al,[bx.fShortDod]
	je	Ltemp011
	jmp	XRC40
Ltemp011:

;/*  copy any bookmarks in the source document which are fully enclosed
;    in the insertion range to the destination document */
;	if ((*hdodSrc)->hsttbBkmk != hNil)
;	    XCopyBkmks(fPlan, pxbc, &pxsr->xsbCopy, pcaIns, pcaDel);
	cmp	[bx.hsttbBkmkDod],hNil
	je	XRC29
	lea	dx,[di.xsbCopyXsr]
	cCall	XCopyBkmks,<[fPlan], [pxbc], dx, si, [di.pcaDelXsr]>
XRC29:

;/* copy any anotations along with their reference marks */
;	if ((*hdodSrc)->docAtn != docNil)
;	    XAddReferencedText(fPlan, pxbc, &pxsr->xsfAtnCopy, pcaIns, pcaDel,
;		    edcDrpAtn);
;#define edcDrpAtn	 (offset(DOD,drpAtn)/sizeof(int))
	;XRC47 performs
	;hplc = *(((int *)(PdodDoc(((struct CA *)si)->doc))) + al);
	;if (hplc != hNil)
	;   XAddReferencedText(fPlan, pxbc, ((char *)pxsr) + dx,
	;	((struct XSR *)di)->pcaIns, ((struct XSR *)di)->pcaDel, al);
	;ax, bx, cx and dx are altered.
	mov	dx,([xsfAtnCopyXsr])
	mov	al,([drpAtnDod]) SHR 1
	call	XRC47

;/* copy any footnotes along with their reference marks */
;	if ((*hdodSrc)->docFtn != docNil)
;	    XAddReferencedText(fPlan, pxbc, &pxsr->xsfFtnCopy, pcaIns, pcaDel,
;		    edcDrpFtn);
;#define edcDrpFtn	 (offset(DOD,drpFtn)/sizeof(int))
	;XRC47 performs
	;hplc = *(((int *)(PdodDoc(((struct CA *)si)->doc))) + al);
	;if (hplc != hNil)
	;   XAddReferencedText(fPlan, pxbc, ((char *)pxsr) + dx,
	;	((struct XSR *)di)->pcaIns, ((struct XSR *)di)->pcaDel, al);
	;ax, bx, cx and dx are altered.
	mov	dx,([xsfFtnCopyXsr])
	mov	al,([drpFtnDod]) SHR 1
	call	XRC47

;/* if there are any sections call AddHplcEdit to copy entries from
;one hplcsed to the other */
;	if ((*hdodSrc)->hplcsed)
;	    {
	xor	ax,ax
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	call	LN_PdodDocCa
	errnz	<docNil - 0>
	cmp	[bx.hplcSedDod],ax
	je	XRC31

;	    caSect.doc = docNil;
	mov	[caSect.docCa],ax

;	    if ((*hdodSrc)->docHdr != docNil || (*hdodDest)->docHdr != docNil)
;		{
	mov	ax,[bx.docHdrDod]
	push	si	;save pcaIns
	mov	si,[di.pcaDelXsr]
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	call	LN_PdodDocCa
	pop	si	;restore pcaIns
	errnz	<docNil - 0>
	or	ax,[bx.docHdrDod]
	je	XRC30

;		XAddHdrText(fPlan, pxbc, pcaIns, pcaDel);
	cCall	XAddHdrText,<[fPlan], [pxbc], si, [di.pcaDelXsr]>

;		caSect.doc = docNil;	/* XAdd.. called CacheSect */
	errnz	<docNil - 0>
	xor	ax,ax
	mov	[caSect.docCa],ax

;		}
XRC30:

;	    XAddToHplcEdit(fPlan, pxbc, &pxsr->xsaSedCopy, pcaIns, pcaDel,
;		    edcSed);
;	    InvalLlc();
;#define InvalLlc()
	;XRC43 performs
	;hplc = *(((int *)(PdodDoc(((struct CA *)si)->doc))) + al);
	;if (hplc != hNil)
	;   XAddToHplcEdit(fPlan, pxbc, ((char *)pxsr) + dx,
	;	((struct XSR *)di)->pcaIns, ((struct XSR *)di)->pcaDel, al);
	;ax, bx, cx and dx are altered.
	mov	dx,([xsaSedCopyXsr])
	mov	al,([hplcsedDod]) SHR 1
	call	XRC43

;	    }
XRC31:

;	if ((*hdodSrc)->fSea && (*hdodSrc)->hplcsea)
;	    XAddToHplcEdit(fPlan, pxbc, &pxsr->xsaSeaCopy, pcaIns, pcaDel,
;		    edcSea);
;#define edcSea  (offset(DOD, hplcsea) / sizeof(int))
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	call	LN_PdodDocCa
	test	[bx.fSeaDod],maskFSeaDod
	je	XRC32
	;XRC43 performs
	;hplc = *(((int *)(PdodDoc(((struct CA *)si)->doc))) + al);
	;if (hplc != hNil)
	;   XAddToHplcEdit(fPlan, pxbc, ((char *)pxsr) + dx,
	;	((struct XSR *)di)->pcaIns, ((struct XSR *)di)->pcaDel, al);
	;ax, bx, cx and dx are altered.
	mov	dx,([xsaSeaCopyXsr])
	mov	al,([hplcseaDod]) SHR 1
	call	XRC43
XRC32:

;/* outline table: as for page table */
;	if (fPlan)
;		pxsr->fNotDelPassCpMac = (pcaDel->cpFirst < (CpMacDoc(docDel)+dcpIns-dcpDel));
	mov	si,[di.pcaDelXsr]
	cmp	[fPlan],fFalse
	je	XRC34
	cCall	CpMacDoc,<[si.docCa]>
	mov	bx,[OFF_dcpDel]
	mov	cx,[SEG_dcpDel]
	sub	bx,[OFF_dcpIns]
	sbb	cx,[SEG_dcpIns]
	sub	bx,ax
	sbb	cx,dx
	errnz	<fFalse>
	xor	ax,ax
	add	bx,[si.LO_cpFirstCa]
	adc	cx,[si.HI_cpFirstCa]
	jge	XRC33
	errnz	<fTrue - fFalse - 1>
	inc	ax
XRC33:
	mov	[di.fNotDelPassCpMacXsr],ax
XRC34:

;	if (((*hdodDest)->hplcpad || (*hdodSrc)->hplcpad) &&
;		pxsr->fNotDelPassCpMac && (*hdodDest)->dk != dkGlsy)
;		{
;		XAddToHplcEdit(fPlan, pxbc, &pxsr->xsaPadCopy, pcaIns, pcaDel,
;			edcPad);
;		}
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	call	LN_PdodDocCa
	cmp	[bx.dkDod],dkGlsy
	je	XRC35
	cmp	[di.fNotDelPassCpMacXsr],fFalse
	je	XRC35
	push	si	;save pcaDel
	mov	si,[di.pcaInsXsr]
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	call	LN_PdodDocCa
	pop	si	;restore pcaDel
	;XRC44 performs
	;hplc = *(((int *)(PdodDoc(((struct CA *)si)->doc))) + al);
	;if ((cx | hplc) != hNil)
	;   XAddToHplcEdit(fPlan, pxbc, ((char *)pxsr) + dx,
	;	((struct XSR *)di)->pcaIns, ((struct XSR *)di)->pcaDel, al);
	;ax, bx, cx and dx are altered.
	mov	cx,[bx.hplcpadDod]
	mov	dx,([xsaPadCopyXsr])
	mov	al,([hplcpadDod]) SHR 1
	call	XRC44
XRC35:

;/* height table */
;	if ((*hdodDest)->hplcphe)
;	    XAddToHplcEdit(fPlan, pxbc, &pxsr->xsaPheCopy, pcaIns, pcaDel,
;		    edcPhe);
	;XRC43 performs
	;hplc = *(((int *)(PdodDoc(((struct CA *)si)->doc))) + al);
	;if (hplc != hNil)
	;   XAddToHplcEdit(fPlan, pxbc, ((char *)pxsr) + dx,
	;	((struct XSR *)di)->pcaIns, ((struct XSR *)di)->pcaDel, al);
	;ax, bx, cx and dx are altered.
	mov	dx,([xsaPheCopyXsr])
	mov	al,([hplcpheDod]) SHR 1
	call	XRC43

;/* we will replace docDest's hidden section character with the hidden section
;   mark from docSrc only if we are not already copying the section mark,
;   we are copying the tail of docIns to the tail of docDel, the text
;   copied from docIns does not end with a section mark, and if docSrc is
;   guarded (ie. == docScrap or docUndo) the hidden section character had to
;   have been copied from the original document. */

;	if (fPlan)
;	    {
	cmp	[fPlan],fFalse
	jne	Ltemp010
	jmp	XRC39
Ltemp010:

;	    CP cpTailIns = CpTail(docIns);
;	    struct DOD *pdodIns;
;/* Note that no cps have been adjusted yet */
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	call	LN_PdodDocCa
	mov	cx,si
	mov	si,[di.pcaInsXsr]
	mov	di,bx
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	call	LN_PdodDocCa

;	    pxsr->fReplHidnSect =
;		    pcaDel->cpFirst + dcpDel <= CpMacDoc(docDel) &&
	;*hdodSrc in bx, pcaDel in cx, pcaIns in si, *hdodDest in di
	;***Begin in line CpMacDoc
	;return ((*mpdochdod[doc]->cpMac - 2*ccpEop);
	mov	ax,-2*ccpEop
	cwd
	add	ax,[di.LO_cpMacDod]
	adc	dx,[di.HI_cpMacDod]
	;***End in line CpMacDoc
	xchg	cx,si
	sub	ax,[si.LO_cpFirstCa]
	sbb	dx,[si.HI_cpFirstCa]
	xchg	cx,si
	sub	ax,[OFF_dcpDel]
	sbb	dx,[SEG_dcpDel]
	jl	XRC38

;		    pcaIns->cpFirst < CpMacDoc(docIns) &&
	;*hdodSrc in bx, pcaDel in cx, pcaIns in si, *hdodDest in di
	;***Begin in line CpMacDoc
	;return ((*mpdochdod[doc]->cpMac - 2*ccpEop);
	mov	ax,2*ccpEop
	cwd
	sub	ax,[bx.LO_cpMacDod]
	sbb	dx,[bx.HI_cpMacDod]
	;***End in line CpMacDoc
	add	ax,[si.LO_cpFirstCa]
	adc	dx,[si.HI_cpFirstCa]
	jge	XRC38

;		    pcaIns->cpLim >= cpTailIns &&
;		    !FSectLimAtCp(docIns, cpTailIns) &&
	;Assembler note: do these two lines involving cpTailIns later
	;to avoid messing up registers

;		    pcaDel->cpLim >= CpTail(docDel) -
;		    ((!PdodDoc(docDel)->fGuarded) ? ccpEop : cp0) &&
	;*hdodSrc in bx, pcaDel in cx, pcaIns in si, *hdodDest in di
	;***Begin in line CpTail
	;return (PdodDoc(doc)->fGuarded ? CpMacDocEdit(doc) : CpMacDoc(doc));
	;Assembler note: The following line is CpMacDocEdit.
	;return(CpMacDoc(doc) - ccpEop);
	;Assembler note: This means that the above expression involving
	;CpTail reduces to (CpMacDoc(doc) - ccpEop).
	mov	ax,3*ccpEop
	cwd
	sub	ax,[di.LO_cpMacDod]
	sbb	dx,[di.HI_cpMacDod]
	;***End in line CpTail
	xchg	cx,si
	add	ax,[si.LO_cpLimCa]
	adc	dx,[si.HI_cpLimCa]
	xchg	cx,si
	jl	XRC38

;		    (!(pdodIns=PdodDoc(docIns))->fGuarded ||
	test	[bx.fGuardedDod],maskFGuardedDod
	je	XRC36

;		    (pdodIns->fSedMacEopCopied && (dcpDel > cp0 || dcpIns == CpMacDocEdit(docIns))));
	test	[bx.fSedMacEopCopiedDod],maskFSedMacEopCopiedDod
	je	XRC38
	xor	ax,ax
	cmp	ax,[OFF_dcpDel]
	sbb	ax,[SEG_dcpDel]
	jl	XRC36
	;***Begin in line CpMacDocEdit
	;return(CpMacDoc(doc) - ccpEop);
	mov	ax,-3*ccpEop
	cwd
	add	ax,[bx.LO_cpMacDod]
	adc	dx,[bx.HI_cpMacDod]
	;***End in line CpMacDocEdit
	sub	ax,[OFF_dcpIns]
	sbb	dx,[SEG_dcpIns]
	or	ax,dx
	jne	XRC38
XRC36:

;	    CP cpTailIns = CpTail(docIns);
;		    pcaIns->cpLim >= cpTailIns &&
;		    !FSectLimAtCp(docIns, cpTailIns) &&
	;Assembler note: These three lines are done above in the C version
	;*hdodSrc in bx, pcaDel in cx, pcaIns in si, *hdodDest in di
	;***Begin in line CpTail
	;return (PdodDoc(doc)->fGuarded ? CpMacDocEdit(doc) : CpMacDoc(doc));
	;Assembler note: The following line is CpMacDocEdit.
	;return(CpMacDoc(doc) - ccpEop);
	mov	al,-3*ccpEop
	test	[bx.fGuardedDod],maskFGuardedDod
	jne	XRC37
	mov	al,-2*ccpEop
XRC37:
	cbw
	cwd
	add	ax,[bx.LO_cpMacDod]
	adc	dx,[bx.HI_cpMacDod]
	;***End in line CpTail
	cmp	[si.LO_cpLimCa],ax
	mov	cx,[si.HI_cpLimCa]
	sbb	cx,dx
	jl	XRC38
	cCall	FSectLimAtCp,<[si.docCa], dx, ax>
	or	ax,ax
	jne	XRC38

;	    }
	db	0B8h	;turns next "xor ax,ax" into "mov ax,immediate"
XRC38:
	xor	ax,ax
	mov	di,[pxsr]
	mov	[di.fReplHidnSectXsr],ax
XRC39:

;	if (pxsr->fReplHidnSect)
;	    {
	cmp	[di.fReplHidnSectXsr],fFalse
	je	XRC40

;	    struct XSR *pxsrT = PxsAlloc(pxbc);
	;***Begin in line PxsAlloc
	mov	bx,[pxbc]
;   Assert(pxbc->ixsMac < pxbc->ixsMax);
ifdef DEBUG
	;Do this assert with a call so as not to mess up short jumps
	call	XRC50
endif ;DEBUG
;   return pxbc->rgxs + (cbXSR * pxbc->ixsMac++);
	mov	ax,cbXsrMin
	mul	[bx.ixsMacXbc]
	inc	[bx.ixsMacXbc]
	mov	si,[bx.rgxsXbc]
	add	si,ax
	;***End in line PxsAlloc

;	    struct CA caDel, caIns;
;	    CheckPxs(fPlan, pxsrT, nxsHidnSect, 0);
;#ifdef DEBUG
;#define CheckPxs(fPlan,pxs,nxsT,wT) (fPlan ? (pxs->nxs=nxsT, pxs->w=wT) : \
;				       Assert(pxs->nxs==nxsT && pxs->w==wT))
;#else
;#define CheckPxs(fPlan,pxs,nxsT,wT)
;#endif /* DEBUG */
ifdef DEBUG
	;Do this assert with a call so as not to mess up short jumps
	call	XRC52
endif ;DEBUG

;	    pxsrT->pcaDel = PcaSetDcp(&caDel, docDel, CpMac1Doc(docDel)-ccpEop,
;		    ccpEop);
	mov	bx,[di.pcaDelXsr]
	lea	ax,[caDel]
	mov	[si.pcaDelXsr],ax
	call	LN_PcaSetDcp

;	    pxsrT->pcaIns = PcaSetDcp(&caIns, docIns, CpMac1Doc(docIns)-ccpEop,
;		    ccpEop);
	mov	bx,[di.pcaInsXsr]
	lea	ax,[caIns]
	mov	[si.pcaInsXsr],ax
	call	LN_PcaSetDcp

;	    XReplaceCps(fPlan, pxbc, pxsrT);
	push	[fPlan]
	push	[pxbc]
	push	si
ifdef DEBUG
	cCall	S_XReplaceCps,<>
else ;!DEBUG
	push	cs
	call	near ptr N_XReplaceCps
endif ;!DEBUG

;	    if (!fPlan && PdodDoc(docDel)->fGuarded)
;		PdodDoc(docDel)->fSedMacEopCopied = fTrue;
	cmp	[fPlan],fFalse
	jne	XRC40
	mov	si,[di.pcaDelXsr]
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	call	LN_PdodDocCa
	test	[bx.fGuardedDod],maskFGuardedDod
	je	XRC40
	or	[bx.fSedMacEopCopiedDod],maskFSedMacEopCopiedDod

;	    }
;	}
XRC40:

;}
cEnd

;	    AssertDo(FOpenPlc(hplcpcd, ipcdFirst, cpcd));
ifdef DEBUG
XRC41:
	or	ax,ax
	jne	XRC42
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn2
	mov	bx,1032
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
XRC42:
	ret
endif ;DEBUG


	;XRC43 performs
	;hplc = *(((int *)(PdodDoc(((struct CA *)si)->doc))) + al);
	;if (hplc != hNil)
	;   XAddToHplcEdit(fPlan, pxbc, ((char *)pxsr) + dx,
	;	((struct XSR *)di)->pcaIns, ((struct XSR *)di)->pcaDel, al);
	;ax, bx, cx and dx are altered.
XRC43:
	xor	cx,cx
	;XRC44 performs
	;hplc = *(((int *)(PdodDoc(((struct CA *)si)->doc))) + al);
	;if ((cx | hplc) != hNil)
	;   XAddToHplcEdit(fPlan, pxbc, ((char *)pxsr) + dx,
	;	((struct XSR *)di)->pcaIns, ((struct XSR *)di)->pcaDel, al);
	;ax, bx, cx and dx are altered.
XRC44:
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	call	LN_PdodDocCa
ifdef DEBUG
	or	al,al
	jns	XRC45
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn2
	mov	bx,1033
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
XRC45:
endif ;/* DEBUG */
	cbw
	add	bx,ax
	add	bx,ax
	or	cx,[bx]
	jcxz	XRC46
	push	[fPlan]
	push	[pxbc]
	add	dx,di
	push	dx
	push	[di.pcaInsXsr]
	push	[di.pcaDelXsr]
	push	ax
	cCall	XAddToHplcEdit,<>
XRC46:
	ret

	;XRC47 performs
	;hplc = *(((int *)(PdodDoc(((struct CA *)si)->doc))) + al);
	;if (hplc != hNil)
	;   XAddReferencedText(fPlan, pxbc, ((char *)pxsr) + dx,
	;	((struct XSR *)di)->pcaIns, ((struct XSR *)di)->pcaDel, al);
	;ax, bx, cx and dx are altered.
XRC47:
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	call	LN_PdodDocCa
ifdef DEBUG
	or	al,al
	jns	XRC48
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn2
	mov	bx,1034
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
XRC48:
endif ;/* DEBUG */
	;   XAddReferencedText(fPlan, pxbc, ((char *)pxsr) + dx,
	;	((struct XSR *)di)->pcaIns, ((struct XSR *)di)->pcaDel, al);
	cbw
	add	bx,ax
	add	bx,ax
	mov	cx,[bx]
	jcxz	XRC49
	push	[fPlan]
	push	[pxbc]
	add	dx,di
	push	dx
	push	[di.pcaInsXsr]
	push	[di.pcaDelXsr]
	push	ax
	cCall	XAddReferencedText,<>
XRC49:
	ret

ifdef DEBUG
XRC50:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[bx.ixsMacXbc]
	cmp	ax,[bx.ixsMaxXbc]
	jl	XRC51
	mov	ax,midEditn2
	mov	bx,1035
	cCall	AssertProcForNative,<ax,bx>
XRC51:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG

ifdef DEBUG
XRC52:
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	[fPlan],fFalse
	je	XRC53
	mov	[si.nxsXsr],nxsHidnSect
	mov	[si.wXsr],0
	jmp	short XRC55
XRC53:
	cmp	[si.nxsXsr],nxsHidnSect
	jne	XRC54
	cmp	[si.wXsr],0
	je	XRC55
XRC54:
	mov	ax,midEditn2
	mov	bx,1036
	cCall	AssertProcForNative,<ax,bx>
XRC55:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG

LN_PcaSetDcp:
	mov	cx,[bx.docCa]
	xchg	ax,bx
	mov	[bx.docCa],cx
	push	bx
	cCall	CpMac1Doc,<cx>
	pop	bx
	mov	[bx.LO_cpLimCa],ax
	mov	[bx.HI_cpLimCa],dx
	sub	ax,ccpEop
	sbb	dx,0
	mov	[bx.LO_cpFirstCa],ax
	mov	[bx.HI_cpFirstCa],dx
	ret


;-------------------------------------------------------------------------
;	XDelFndSedPgdPad(fPlan, pxbc, pxsr)
;-------------------------------------------------------------------------
;/* X  D E L  F N D  S E D  P G D  P A D */
;/* Delete all footnote/annotation text corresponding to refs in [cpFirst:cpLim)
;Also delete SED's for section marks and invalidate PGD's in the page table.
;*/
;EXPORT XDelFndSedPgdPad(fPlan, pxbc, pxsr)
;BOOL fPlan;
;struct XBC *pxbc;
;struct XSR *pxsr;
;{
;   struct PLC **hplc;
;   struct DOD **hdod;
;   struct CA caT;

ifdef DEBUG
; %%Function:N_XDelFndSedPgdPad %%Owner:BRADV
cProc	N_XDelFndSedPgdPad,<PUBLIC,FAR>,<si,di>
else ;!DEBUG
; %%Function:LN_XDelFndSedPgdPad %%Owner:BRADV
cProc	LN_XDelFndSedPgdPad,<PUBLIC,NEAR>,<si,di>
endif ;!DEBUG
	ParmW	fPlan
	ParmW	pxbc
	ParmW	pxsr

	LocalV	caT,cbCaMin

cBegin

;   caT = *pxsr->pcaDel;
	push	ds
	pop	es
	mov	di,[pxsr]
	push	di	;save pxsr
	mov	si,[di.pcaDelXsr]
	lea	di,[caT]
	push	di
	errnz	<cbCaMin AND 1>
	mov	cx,cbCaMin SHR 1
	rep	movsw
	pop	si
	pop	di	;restore pxsr

;   hdod = mpdochdod[caT.doc];
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	call	LN_PdodDocCa

;/* FUTURE: why does this have to be done here?  Can it be done below with the
;rest of the !fShort processing? */
;   if (!(*hdod)->fShort)
	cmp	[bx.fShortDod],fFalse
	jne	XDFSPP02

;	if ((hplc = (*hdod)->hplcsed) != 0)
;	    {
	mov	cx,[bx.hplcsedDod]
	jcxz	XDFSPP02

	mov	dx,[fPlan]
	push	dx	;argument for XDelInHplcEdit
	push	[pxbc]	;argument for XDelInHplcEdit
	errnz	<NULL>
	xor	ax,ax
	push	ax	;argument for XDelInHplcEdit
	push	[di.pcaDelXsr]	    ;argument for XDelInHplcEdit
	push	cx	;argument for XDelInHplcEdit
	errnz	<edcNone - 0>
	push	ax	;argument for XDelInHplcEdit

;	    if ((*hdod)->docHdr != docNil)
;		XDeleteHdrText(fPlan, pxbc, &pxsr->xsp, pxsr->pcaDel);
	errnz	<docNil - 0>
	cmp	[bx.docHdrDod],ax
	je	XDFSPP01
	lea	bx,[di.xspXsr]
	cCall	XDeleteHdrText,<dx, [pxbc], bx, [di.pcaDelXsr]>
XDFSPP01:

;	    XDelInHplcEdit(fPlan, pxbc, NULL, pxsr->pcaDel, hplc,
;		    edcNone);
	cCall	XDelInHplcEdit,<>

;	    InvalLlc();
;#define InvalLlc()

;	    }
XDFSPP02:

;/* protect PLCs from lookups with cp > cpMac */
;   Assert(caT.cpLim <= CpMac2Doc(caT.doc));
ifdef DEBUG
	call	XDFSPP12
endif ;/* DEBUG */

;   caT.cpLim = CpMin(caT.cpLim, CpMac2Doc(caT.doc));
	cCall	CpMac2Doc,<[caT.docCa]>
	cmp	ax,[caT.LO_cpLimCa]
	mov	cx,dx
	sbb	cx,[caT.HI_cpLimCa]
	jge	XDFSPP03
	mov	[caT.LO_cpLimCa],ax
	mov	[caT.HI_cpLimCa],dx
XDFSPP03:

;   if (caT.cpLim <= caT.cpFirst)
;	return;
	mov	ax,[caT.LO_cpFirstCa]
	mov	dx,[caT.HI_cpFirstCa]
	sub	ax,[caT.LO_cpLimCa]
	sbb	dx,[caT.HI_cpLimCa]
	jge	XDFSPP05

;/* these PLCs are in short and long docs */
;   if ((hplc = (*hdod)->hplcpgd) != 0)
;	{
;	XDelInHplcEdit(fPlan, pxbc, &pxsr->xsaPgdDel, &caT, hplc, edcPgd);
;	InvalLlc();
;	}
;#define InvalLlc()
;#define edcPgd  (offset(DOD, hplcpgd) / sizeof(int))
	;XDFSPP06 performs
	;hplc = *(((int *)(PdodDoc(((struct CA *)si)->doc))) + al);
	;if (hplc != hNil)
	;   XDelInHplcEdit(fPlan, pxbc, ((char *)pxsr) + dx, si, hplc, al);
	;ax, bx, cx and dx are altered.
	mov	dx,([xsaPgdDelXsr])
	mov	al,([hplcpgdDod]) SHR 1
	call	XDFSPP06

;   if ((hplc = (*hdod)->hplcphe) != 0)
;	XDelInHplcEdit(fPlan, pxbc, &pxsr->xsaPheDel, &caT, hplc, edcPhe);
;#define edcPhe  (offset(DOD, hplcphe) / sizeof(int))
	;XDFSPP06 performs
	;hplc = *(((int *)(PdodDoc(((struct CA *)si)->doc))) + al);
	;if (hplc != hNil)
	;   XDelInHplcEdit(fPlan, pxbc, ((char *)pxsr) + dx, si, hplc, al);
	;ax, bx, cx and dx are altered.
	mov	dx,([xsaPheDelXsr])
	mov	al,([hplcpheDod]) SHR 1
	call	XDFSPP06

;   if ((*hdod)->fShort)
;	return;
;/* PLCs for long docs only */
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	call	LN_PdodDocCa
	cmp	[bx.fShortDod],fFalse
	jne	XDFSPP05

;   if ((*hdod)->docFtn != docNil)
;	XDelReferencedText(fPlan, pxbc, &pxsr->xsfFtnDel, &caT, edcDrpFtn);
;#define edcDrpFtn	 (offset(DOD,drpFtn)/sizeof(int))
	;XDFSPP09 performs
	;hplc = *(((int *)(PdodDoc(((struct CA *)si)->doc))) + cx);
	;if (hplc != hNil)
	;   XDelReferencedText(fPlan, pxbc, ((char *)pxsr) + dx, si, al);
	;ax, bx, cx and dx are altered.
	mov	dx,([xsfFtnDelXsr])
	mov	al,([drpFtnDod]) SHR 1
	mov	cx,([docFtnDod])
	call	XDFSPP09

;   if ((*hdod)->docAtn != docNil)
;	XDelReferencedText(fPlan, pxbc, &pxsr->xsfAtnDel, &caT, edcDrpAtn);
;#define edcDrpAtn	 (offset(DOD,drpAtn)/sizeof(int))
	;XDFSPP09 performs
	;hplc = *(((int *)(PdodDoc(((struct CA *)si)->doc))) + cx);
	;if (hplc != hNil)
	;   XDelReferencedText(fPlan, pxbc, ((char *)pxsr) + dx, si, al);
	;ax, bx, cx and dx are altered.
	mov	dx,([xsfAtnDelXsr])
	mov	al,([drpAtnDod]) SHR 1
	mov	cx,([docAtnDod])
	call	XDFSPP09

;   if ((hplc = (*hdod)->hplcpad) != 0 && caT.cpFirst < CpMacDoc(caT.doc))
;	XDelInHplcEdit(fPlan, pxbc, &pxsr->xsaPadDel, &caT, hplc, edcPad);
;#define edcPad  (offset(DOD, hplcpad) / sizeof(int))
	cCall	CpMacDoc,<[caT.docCa]>
	cmp	[caT.LO_cpFirstCa],ax
	mov	cx,[caT.HI_cpFirstCa]
	sbb	cx,dx
	jge	XDFSPP04
	;XDFSPP06 performs
	;hplc = *(((int *)(PdodDoc(((struct CA *)si)->doc))) + al);
	;if (hplc != hNil)
	;   XDelInHplcEdit(fPlan, pxbc, ((char *)pxsr) + dx, si, hplc, al);
	;ax, bx, cx and dx are altered.
	mov	dx,([xsaPadDelXsr])
	mov	al,([hplcpadDod]) SHR 1
	call	XDFSPP06
XDFSPP04:

;   if ((*hdod)->fSea && (hplc = (*hdod)->hplcsea) != 0)
;	XDelInHplcEdit(fPlan, pxbc, &pxsr->xsaSeaDel, &caT, hplc, edcSea);
;#define edcSea  (offset(DOD, hplcsea) / sizeof(int))
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	call	LN_PdodDocCa
	test	[bx.fSeaDod],maskfSeaDod
	je	XDFSPP05
	;XDFSPP06 performs
	;hplc = *(((int *)(PdodDoc(((struct CA *)si)->doc))) + al);
	;if (hplc != hNil)
	;   XDelInHplcEdit(fPlan, pxbc, ((char *)pxsr) + dx, si, hplc, al);
	;ax, bx, cx and dx are altered.
	mov	dx,([xsaSeaDelXsr])
	mov	al,([hplcseaDod]) SHR 1
	call	XDFSPP06
XDFSPP05:

;}
cEnd

	;XDFSPP06 performs
	;hplc = *(((int *)(PdodDoc(((struct CA *)si)->doc))) + al);
	;if (hplc != hNil)
	;   XDelInHplcEdit(fPlan, pxbc, ((char *)pxsr) + dx, si, hplc, al);
	;ax, bx, cx and dx are altered.
XDFSPP06:
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	call	LN_PdodDocCa
ifdef DEBUG
	or	al,al
	jns	XDFSPP07
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn2
	mov	bx,1037
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
XDFSPP07:
endif ;/* DEBUG */
	cbw
	add	bx,ax
	add	bx,ax
	mov	cx,[bx]
	jcxz	XDFSPP08
	push	[fPlan]
	push	[pxbc]
	add	dx,di
	push	dx
	push	si
	push	cx
	push	ax
	cCall	XDelInHplcEdit,<>
XDFSPP08:
	ret

	;XDFSPP09 performs
	;hplc = *(((int *)(PdodDoc(((struct CA *)si)->doc))) + cx);
	;if (hplc != hNil)
	;   XDelReferencedText(fPlan, pxbc, ((char *)pxsr) + dx, si, al);
	;ax, bx, cx and dx are altered.
XDFSPP09:
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	call	LN_PdodDocCa
ifdef DEBUG
	or	al,al
	jns	XDFSPP10
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn2
	mov	bx,1038
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
XDFSPP10:
endif ;/* DEBUG */
	cbw
	add	bx,cx
	mov	cx,[bx]
	jcxz	XDFSPP11
	push	[fPlan]
	push	[pxbc]
	add	dx,di
	push	dx
	push	si
	push	ax
	cCall	XDelReferencedText,<>
XDFSPP11:
	ret


;   Assert(caT.cpLim <= CpMac2Doc(caT.doc));
ifdef DEBUG
XDFSPP12:
	push	ax
	push	bx
	push	cx
	push	dx
	cCall	CpMac2Doc,<[caT.docCa]>
	sub	ax,[caT.LO_cpLimCa]
	sbb	dx,[caT.HI_cpLimCa]
	jge	XDFSPP13
	mov	ax,midEditn2
	mov	bx,1040
	cCall	AssertProcForNative,<ax,bx>
XDFSPP13:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

;   if (DcpCa(pca) != 0)
;/* delete structures for text being deleted */
;	XDeleteStruct(fPlan, pxbc, pxsr);

;/* X  D E L E T E  S T R U C T */
;/* Delete structures from the deletion range */
;/* %%Function:XDeleteStruct %%owner:peterj %%reviewed: 6/28/89 */
;XDeleteStruct(fPlan, pxbc, pxsr)
;BOOL fPlan;
;struct XBC *pxbc;
;struct XSR *pxsr;
;{
;   struct CA *pca = pxsr->pcaDel;
;
;/* check for deleting para and sect boundaries; delete entries from parallel
;structures */
;   if (!fPlan && !vfNoInval)
;	InvalParaSect(pca, pca, fTrue);
;   if (FFieldsInPca(pca))
;	XDeleteFields(fPlan, pxbc, &pxsr->xslDelete, pca);
;
;   if (!PdodDoc(pca->doc)->fShort)
;	if (PdodDoc(pca->doc)->hsttbBkmk != hNil)
;	    XDeleteBkmks(fPlan, pxbc, pca, fFalse);
;   XDelFndSedPgdPad(fPlan, pxbc, pxsr);
;}

PUBLIC LN_XDeleteStruct
LN_XDeleteStruct:
	;Assumes bx = pxbc, cx = fPlan, si = pca, di = pxsr
	;ax, bx, cx, dx are altered.
;   if (dcpDel != cp0)
;	{
	;LN_DcpCa assumes pca passed in si and returns DcpCa in dx:ax.
	;Only ax and dx are altered.
	mov	si,[di.pcaDelXsr]
	call	LN_DcpCa
	or	ax,dx
	je	XDS05

;	if (!fPlan && !vfNoInval)
;	    /* check for deleting para and sect boundaries; delete entries
;	       from parallel structures */
;	    InvalParaSect(pcaDel, pcaDel, fTrue);
	push	cx	;save fPlan
	or	cx,[vfNoInval]
	jne	XDS01
	errnz	<fTrue - 1>
	inc	cx
	push	bx	;save pxbc
	cCall	InvalParaSect,<si, si, cx>
	pop	bx	;restore pxbc
XDS01:
	pop	cx	;restore fPlan

;	if (FFieldsInPca(pcaDel))
;	    {
	;LN_FFieldsInPca assumes pca passed in si and performs
	;FFieldsInPca(pca).  ax, bx, cx, dx are altered.
	;The sign bit set reflects a true result
	push	bx	;save pxbc
	push	cx	;save fPlan
	call	LN_FFieldsInPca
	pop	cx	;restore fPlan
	pop	bx	;restore pxbc
	jns	XDS03

;	    if (!fPlan)
;		(*hdodDest)->fFldNestedValid = fFalse;
	or	cx,cx
	jne	XDS02
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	push	bx	;save pxbc
	call	LN_PdodDocCa
	and	[bx.fFldNestedValidDod],NOT maskFFldNestedValidDod
	pop	bx	;restore pxbc
XDS02:

;	    XDeleteFields(fPlan, pxbc, &pxsr->xslDelete, pcaDel);
	lea	ax,[di.xslDeleteXsr]
	push	bx	;save pxbc
	push	cx	;save fPlan
	cCall	XDeleteFields,<cx, bx, ax, si>
	pop	cx	;restore fPlan
	pop	bx	;restore pxbc

;	    }
XDS03:

;	if (!(*hdodDest)->fShort)
;	    if ((*hdodDest)->hsttbBkmk != hNil)
;		XDeleteBkmks(fPlan, pxbc, pcaDel, fFalse);
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	push	bx	;save pxbc
	call	LN_PdodDocCa
	xor	ax,ax
	errnz	<fFalse>
	cmp	[bx.fShortDod],al
	jne	XDS035	;do this extra conditional jmp to avoid GP faults
	mov	dx,[bx.hsttbBkmkDod]
XDS035:
	pop	bx	;restore pxbc
	jne	XDS04
	errnz	<hNil>
	or	dx,dx
	je	XDS04
	errnz	<fFalse>
	push	bx	;save pxbc
	push	cx	;save fPlan
	cCall	XDeleteBkmks,<cx, bx, si, ax>
	pop	cx	;restore fPlan
	pop	bx	;restore pxbc
XDS04:

;	XDelFndSedPgdPad(fPlan, pxbc, pxsr);
	push	cx
	push	bx
	push	di
ifdef DEBUG
	cCall	S_XDelFndSedPgdPad,<>
else ;!DEBUG
	call	LN_XDelFndSedPgdPad
endif ;!DEBUG

;	}
XDS05:
	ret


	;LN_FlushRulerSprms performs
	;if (vrulss.caRulerSprm.doc != docNil)
	;    FlushRulerSprms();
	;ax, bx, cx, dx are altered.
LN_FlushRulerSprms:
	cmp	[vrulss.caRulerSprmRulss.docCa],docNil
	je	FRS01
	cCall	FlushRulerSprms,<>
FRS01:
	ret


;/* F  F I E L D S  I N  P C A */
;/*  return fTrue if there are any field characters in pca */
;BOOL FFieldsInPca(pca)
;struct CA *pca;
;{
;    struct PLC **hplcfld = PdodDoc(pca->doc)->hplcfld;
;
;    return (hplcfld != hNil &&
;	     IInPlcRef(hplcfld, pca->cpFirst) <=
;	     IInPlcCheck(hplcfld, CpMax(0,pca->cpLim-1)));
;}
	;LN_FFieldsInPca assumes pca passed in si and performs
	;FFieldsInPca(pca).  ax, bx, cx, dx are altered.
	;The sign bit set reflects a true result
; %%Function:LN_FFieldsInPca %%Owner:BRADV
PUBLIC LN_FFieldsInPca
LN_FFieldsInPca:
	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
	call	LN_PdodDocCa
	mov	cx,[bx.hplcfldDod]
	or	cx,cx
	je	LN_FFIP02
	push	di		;save caller's di
	mov	ax,[si.LO_cpLimCa]
	mov	dx,[si.HI_cpLimCa]
	sub	ax,1
	sbb	dx,0
	jge	LN_FFIP01
	xor	ax,ax
	cwd
LN_FFIP01:
	push	cx		;argument for IInPlcCheck
	push	dx		;argument for IInPlcCheck
	push	ax		;argument for IInPlcCheck
	cCall	IInPlcRef,<cx, [si.HI_cpFirstCa], [si.LO_cpFirstCa]>
	xchg	ax,di
	cCall	IInPlcCheck,<>
	sub	di,ax
	dec	di
	pop	di		;restore caller's di
LN_FFIP02:
	ret

	;LN_PdodDocCa assumes pca passed in si and performs
	;PdodDoc(pca->doc).  Only bx is altered.
; %%Function:LN_PdodDocCa %%Owner:BRADV
PUBLIC LN_PdodDocCa
LN_PdodDocCa:
	mov	bx,[si.docCa]
	;LN_PdodDocEdit assumes doc passed in bx and performs
	;PdodDoc(doc).	Only bx is altered.
; %%Function:LN_PdodDocEdit %%Owner:BRADV
PUBLIC LN_PdodDocEdit
LN_PdodDocEdit:
	shl	bx,1
	mov	bx,[bx.mpdochdod]
ifdef DEBUG
	cmp	bx,hNil
	jne	LN_PDC01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midEditn2
	mov	bx,1039
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LN_PDC01:
endif ;/* DEBUG */
	mov	bx,[bx]
	ret


	;LN_DcpCa assumes pca passed in si and returns DcpCa in dx:ax.
	;Only ax and dx are altered.
LN_DcpCa:
	;***Begin in line DcpCa
	;return (pca->cpLim - pca->cpFirst);
	mov	ax,[si.LO_cpLimCa]
	mov	dx,[si.HI_cpLimCa]
	sub	ax,[si.LO_cpFirstCa]
	sbb	dx,[si.HI_cpFirstCa]
	;***End in line DcpCa
	ret


ifdef	DEBUG
PUBLIC LN_FreezeHpEdit
LN_FreezeHpEdit:
;#define FreezeHp()		 (cHpFreeze++?0:LockHeap(sbDds))
	cmp	[cHpFreeze],0
	jne	LN_FH01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,sbDds
	cCall	LockHeap,<ax>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LN_FH01:
	inc	[cHpFreeze]
	ret
endif ;DEBUG


ifdef	DEBUG
; %%Function:LN_MeltHpEdit %%Owner:BRADV
PUBLIC LN_MeltHpEdit
LN_MeltHpEdit:
;#define MeltHp()		 (--cHpFreeze?0:UnlockHeap(sbDds))
	dec	[cHpFreeze]
	jne	LN_MH01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,sbDds
	cCall	UnlockHeap,<ax>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LN_MH01:
	ret
endif ;DEBUG

sEnd	edit2
        end
