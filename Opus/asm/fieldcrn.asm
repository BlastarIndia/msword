        include w2.inc
        include noxport.inc
        include consts.inc
        include structs.inc

createSeg	fieldcr_PCODE,fieldcr,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midFieldcrn	  equ 13	  ; module ID, for native asserts
NatPause equ 1
endif ;DEBUG

ifdef	NatPause
PAUSE	MACRO
	int 3
	ENDM
else
PAUSE	MACRO
	ENDM
endif

; EXPORTED LABELS


; EXTERNAL FUNCTIONS

externFP	<ReloadSb>
externFP	<CpPlc>
externFP	<GetPlc>
externFP	<CpMacDoc>
externFP	<EfltFromFlt>
externFP	<IInPlcCheck>
externFP	<N_FillIfldFlcd>
externFP	<EfltFromFlt>
externFP	<IInPlcCheck>
externFP	<IInPlcRef>
externFP	<N_CachePara>
externFP	<N_FetchCp>
externFP	<FUpdateHplcpad>
externFP	<IInPlc>
externFP	<N_FormatLine>
externFP	<N_DcpSkipFieldChPflcd>
externFP	<N_FShowResultPflcdFvc>
externFP	<FetchCpAndPara>
externFP	<N_FInTableDocCp>
externFP	<N_GetIfldFlcd>
externFP	<N_QcpQfooPlcfoo>
externFP	<FAbsPap>
externFP	<FMatchAbs>

ifdef DEBUG
externFP        <AssertProcForNative,ScribbleProc>
externFP    	<FCheckHandle>
externFP	<S_DcpSkipFieldChPflcd>
externFP	<S_FShowResultPflcdFvc>
externFP	<S_IfldFromDocCp>
externFP	<S_FillIfldFlcd>
externFP	<IMacPlc>
externFP	<S_CachePara>
externFP	<S_FetchCp>
externFP	<S_GetIfldFlcd>
externFP	<S_FetchCpPccpVisible>
externFP	<S_CpVisibleCpField>
externFP	<S_FormatLine>
externFP	<S_CpFormatFrom>
externFP	<S_FInTableDocCp>
externFP	<S_CpVisibleBackCpField>
externFP	<CheckFNestedBits>
externFP	<S_FCpVisiInOutline>
externFP	<S_FVisibleCp>
endif ;DEBUG

ifdef DFORMULA
externFP	<CommSzRgNum>
endif ;DFORMULA


; EXTERNAL DATA

sBegin  data

externW mpsbps		;extern SB	     mpsbps[];
externW vfli		;extern struct FLI   vfli;
externW vdocFetch	;extern int          vdocFetch;
externD vcpFetch	;extern CP           vcpFetch;
externW	vchpFetch	;extern struct CHP   vchpFetch;
externW vpapFetch	;extern struct PAP   vpapFetch;
externW vccpFetch	;extern int          vccpFetch;
externW vhpchFetch	;extern CHAR HUGE *  vhpchFetch;
externW mpdochdod	;extern struct DOD **mpdochdod[];
externW mpwwhwwd	;extern struct WWD **mpwwhwwd[];
externW caPara		;extern struct CA    caPara;
externD vcpFetchFirstVisi
externW vdocFetchVisi
externW vfvcFetchVisi
externD vcpFetchVisi
externD vcpFirstTablePara

ifdef DEBUG
externW vfCheckPlc
externW vfSeeAllFieldCps
externW docLastFetchVisi
externD cpFirstNextFetchVisi
externW vcVisiCalls
externW vcVisiUsedCache
externW wFillBlock
externW vsccAbove
externW vpdrfHead
endif ;DEBUG

sEnd    data


; CODE SEGMENT fieldcr_PCODE

sBegin	fieldcr
	assumes cs,fieldcr
        assumes ds,dgroup
	assume es:nothing
	assume ss:nothing


include	asserth.asm



;-------------------------------------------------------------------------
;	IfldFromDocCp ( doc, cp, fMatch )
;-------------------------------------------------------------------------
;/* I F L D  F R O M  D O C  C P */
;/*  Searches the plcfld for the doc in reverse order.
;    Returns the innermost field containing cp.  If fMatch then returns a valid
;    field iff cp is one of the fields special characters (chFieldBegin,
;    chFieldSeparate or chFieldEnd).  If cp is not in a live field or fMatch
;    and cp is not a special field character then returns ifldNil.
;*/
;
;native IfldFromDocCp (doc, cp, fMatch)
;int doc;
;CP cp;
;BOOL fMatch;
;
;{
;
;	struct PLC **hplcfld = PdodDoc (doc)->hplcfld;
;	int ifld;
;	int cChBegin = 0;

; %%Function:N_IfldFromDocCp %%Owner:BRADV
cProc	N_IfldFromDocCp,<PUBLIC,FAR>,<si,di>
	ParmW	doc
	ParmD	cp
	ParmW	fMatch

	LocalW	hplcfld
	LocalV	fldVar, cbFldMin
;	ifld in si
;	cChBegin in di

cBegin

	mov	bx,[doc]
	;Takes doc in bx, result in bx.  Only bx is altered.
	call	LN_PdodDoc
	mov	di,[bx.hplcfldDod]
	mov	hplcfld,di

;	if (hplcfld == hNil)
;		return ifldNil;
	errnz	<hNil>
	or	di,di
	je	IFDC01

;	/* largest i s.t. rgcp[i] <= cp */
;	ifld = IInPlcCheck (hplcfld, cp); 

	cCall	IInPlcCheck,<di, SEG_cp, OFF_cp>
	xchg	si,ax

;	if (ifld < 0 || (fMatch && CpPlc( hplcfld, ifld ) != cp))

	or	si,si
	jl	IFDC01
	cmp	fMatch,fFalse
	je	IFDC02
	cCall	CpPlc,<di, si>
	cmp	ax,OFF_cp
	jne	IFDC01
	cmp	dx,SEG_cp
	je	IFDC02

IFDC01:
;		return ifldNil;
	mov	ax,ifldNil
	jmp	short IFDC08

IFDC02:

;	GetPlc( hplcfld, ifld, &fld );
	lea	ax,fldVar
	cCall	GetPlc,<di, si, ax>

;	if (fld.ch == chFieldEnd && CpPlc(hplcfld, ifld) == cp)
	mov	al,[fldVar.chFld]
	and	al,maskChFld
	cmp	al,chFieldEnd
	jne	IFDC03
	cCall	CpPlc,<di, si>
	cmp	ax,OFF_cp
	jne	IFDC03
	cmp	dx,SEG_cp
	jne	IFDC03

;		ifld--;
	dec	si

IFDC03:
;	this statement is taken from the initialization of the frame.
;	int cChBegin = 0;
	xor	di,di

IFDC035:

;	while (ifld >= 0)
	or	si,si
	jl	IFDC07

;		{
;		GetPlc( hplcfld, ifld, &fld );
	lea	ax,fldVar
	cCall	GetPlc,<hplcfld, si, ax>

;		if (fld.ch == chFieldBegin && cChBegin == 0)
;			break;
	mov	al,[fldVar.chFld]
	and	al,maskChFld
	cmp	al,chFieldBegin
	jne	IFDC04
	or	di,di
	je	IFDC07

IFDC04:
;		if (fld.ch == chFieldBegin)
	cmp	al,chFieldBegin
	jne	IFDC05	

;			cChBegin++;  /* nested field's begin */
	inc	di

IFDC05:
;		else if (fld.ch == chFieldEnd)
	cmp	al,chFieldEnd
	jne	IFDC06

;		        cChBegin--;  /* nested field */
	dec	di

IFDC06:
;		ifld--;
	dec	si

;		}
	jmp	short IFDC035


IFDC07:
;	return ifld;
	xchg	ax,si

IFDC08:

;}
cEnd

; End of IfldFromDocCp


;-------------------------------------------------------------------------
;	LN_ReloadSbFetch
;-------------------------------------------------------------------------
	;LN_ReloadSbFetch takes the sb from vhpchFetch and sets es to the
	;corresponding value
	;Only es and bx are altered.
; %%Function:LN_ReloadSbFetch %%Owner:BRADV
cProc	LN_ReloadSbFetch,<NEAR,ATOMIC>,<>

cBegin	nogen
	mov	bx,whi ([vhpchFetch])
	push	ax
	shl	bx,1
	mov	ax,[bx.mpsbps]
	mov	es,ax
	shr	ax,1
	jc	LN_RS01
;	ReloadSb trashes ax, cx, and dx
	push	cx
	push	dx
	cCall	ReloadSb,<>
	pop	dx
	pop	cx
LN_RS01:
	pop	ax
cEnd	nogen
	ret
	



;-------------------------------------------------------------------------
;	IfldInsertDocCp ( doc, cp )
;-------------------------------------------------------------------------
;/*  REVIEW: CORE FUNCTION */
;/* I F L D  I N S E R T  D O C  C P */
;/*  For an insertion point at doc, cp, what field is it in?  For this
;    purpose (invalidation in insert mode) an insertion point is in a 
;    field iff the field plc entry immediately preceeding cp is a field begin.
;*/
;
;native IfldInsertDocCp (doc, cp)
;int doc;
;CP cp;
;
;{
;    int ifld;
;    struct PLC **hplcfld = PdodDoc (doc)->hplcfld;
;    struct FLD fld;

; %%Function:N_IfldInsertDocCp %%Owner:BRADV
cProc	N_IfldInsertDocCp,<PUBLIC,FAR>,<si,di>
	ParmW	doc
	ParmD	cp

	LocalV	fldVar,cbFldMin

;	hplcfld is kept in si
;	ifld is kept in di

cBegin
	mov	bx,[doc]
	;Takes doc in bx, result in bx.  Only bx is altered.
	call	LN_PdodDoc
	mov	si,[bx.hplcfldDod]

;    if (hplcfld == hNil)
;        return ifldNil;
	errnz	<hNil>
	or	si,si
	je	IIDC01

;    ifld = IInPlcRef (hplcfld, cp);
	cCall	IInPlcRef,<si, SEG_cp, OFF_cp>
	xchg	di,ax

;    if (--ifld >= 0)
	dec	di
	jl	IIDC01

;    	{
;	GetPlc( hplcfld, ifld, &fld );
	lea	ax,fldVar
	cCall	GetPlc,<si, di, ax>
	
;	if (fld.ch == chFieldBegin)
;		return ifld;
	mov	al,[fldVar.chFld]
	and	al,maskChFld
	cmp	al,chFieldBegin
	xchg	ax,di
	je	IIDC02

;	}
IIDC01:

;    return ifldNil;
	mov	ax,ifldNil

IIDC02:

;}
cEnd

; End of IfldInsertDocCp


;-------------------------------------------------------------------------
;	CorrectFNestedBits(doc)
;-------------------------------------------------------------------------
;/* C O R R E C T  F  N E S T E D  B I T S */
;/*  Correct all field fNested bits in the specified doc.
;*/
;
;NATIVE CorrectFNestedBits(doc)
;int doc;
;{
;    int cNested = 0;
;    int ifld, ifldMac;
;    struct PLC **hplcfld = PdodDoc(doc)->hplcfld;
;    struct FLD fld;
LN_CorrectFNestedBits:

;    PdodDoc(doc)->fFldNestedValid = fTrue;
	or	[bx.fFldNestedValidDod],maskFFldNestedValidDod
	mov	bx,[bx.hplcfldDod]

;    if (hplcfld == hNil)
;	 return;
	errnz	<hNil>
	or	bx,bx
	je	LN_CFNB04

;    ifldMac = IMacPlc(hplcfld);
	mov	di,[bx]
	push	[di.iMacPlcStr]

;    for (ifld = 0; ifld < ifldMac; ifld++)
;	 {
;	QcpQfooPlcfoo takes hplcfoo in bx, ifoo in si, it returns pplcfoo
;	in bx, cbfoo in cx, qcp in es:di, qfoo in es:si.
;	if DEBUG it returns hpcp in dx:di, hpfoo in dx:si.
;	Changes ax, bx, cx, dx, si, di.
	xor	si,si
ifdef DEBUG
	mov	di,1	;O.K. to pass ifldMac
endif ;DEBUG
	cCall	N_QcpQfooPlcfoo,<>
ifdef DEBUG
	;Check that es == mpsbps[dx];
	call	LN_CFNB05
endif ;DEBUG
	pop	di
	shl	di,1
	add	di,si
	xor	cx,cx
LN_CFNB01:
	cmp	si,di
	jae	LN_CFNB02

;	 GetPlc (hplcfld, ifld, &fld);
	errnz	<cbFldMin - 2>
	lods	wptr es:[si]

;	 if (fld.ch == chFieldBegin)
	errnz	<(chFld) - 0>
	and	al,maskChFld
	cmp	al,chFieldBegin
	jne	LN_CFNB015

;	     cNested++;
	inc	cx
	jmp	short LN_CFNB01

;	 else if (fld.ch == chFieldEnd)
;	     {
LN_CFNB015:
	errnz	<(chFld) - 0>
	cmp	al,chFieldEnd
	jne	LN_CFNB01

;	     if (--cNested)
;		 fld.fNested = fTrue;
;	     else
;		 fld.fNested = fFalse;
;	     PutPlcLast(hplcfld, ifld, &fld);
;	     }
	dec	cx
	and	es:[si.fNestedFld-2],NOT maskfNestedFld
	jcxz	LN_CFNB01
	or	es:[si.fNestedFld-2],maskfNestedFld
	jmp	short LN_CFNB01

LN_CFNB02:
;	 }

;    Assert(!cNested);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	jcxz	LN_CFNB03

	mov	ax,midFieldcrn
	mov	bx,1004
	cCall	AssertProcForNative,<ax,bx>
LN_CFNB03:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;}
LN_CFNB04:
	ret

ifdef DEBUG
LN_CFNB05:
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
	jc	LN_CFNB06
	;Assembler note: There is no way we should have to call ReloadSb here.
;	reload sb trashes ax, cx, and dx
;	cCall	ReloadSb,<>
	mov	ax,midFieldcrn
	mov	bx,1028
	cCall	AssertProcForNative,<ax,bx>
LN_CFNB06:
	pop	ax	;restore es from QcpQfooPlcfoo
	mov	bx,es	;compare with es rederived from the SB of QcpQfooPlcfoo
	cmp	ax,bx
	je	LN_CFNB07
	mov	ax,midFieldcrn
	mov	bx,1029
	cCall	AssertProcForNative,<ax,bx>
LN_CFNB07:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */



;/* V I S I B L E  C H A R A C T E R  F E T C H  F U N C T I O N S */



;-------------------------------------------------------------------------
;	InitFvb ( pfvb )
;-------------------------------------------------------------------------
;/* I N I T  F V B */
;/*  Initialize a Fetch Visible Block.
;*/
;
;native InitFvb (pfvb)
;struct FVB *pfvb;
;
;{

; %%Function:InitFvb %%Owner:BRADV
cProc	InitFvb,<PUBLIC,FAR>,<di>
	ParmW	pfvb

cBegin

;    /*  assumes: docNil == cp0 == NULL == fFalse == 0 */
;    SetBytes (pfvb, 0, sizeof (struct FVB));

	mov	di,pfvb
	push	ds
	pop	es
	xor	al,al
	mov	cx,cbFvbMin
	rep	stosb

;}
cEnd

; End of InitFvb





;-------------------------------------------------------------------------
;	InitFvbBufs ( pfvb, rgch, cchMax, rgcr, ccrMax )
;-------------------------------------------------------------------------
;/* I N I T  F V B  B U F S */
;/*  Set up the buffers for a fetch visible block.
;*/
;
;native InitFvbBufs (pfvb, rgch, cchMax, rgcr, ccrMax)
;struct FVB *pfvb;
;CHAR *rgch;
;int cchMax;
;struct CR *rgcr;
;int ccrMax;
;
;{

; %%Function:InitFvbBufs %%Owner:BRADV
cProc	InitFvbBufs,<PUBLIC,FAR>,<>
	ParmW	pfvb
	ParmW	rgch
	ParmW	cchMax
	ParmW	rgcr
	ParmW	ccrMax

cBegin

	mov	bx,pfvb

;    pfvb->rgch = rgch;
	mov	ax,rgch
	mov	[bx.rgchFvb],ax

;    pfvb->cchMax = cchMax;
	mov	ax,cchMax
	mov	[bx.cchMaxFvb],ax

;    pfvb->rgcr = rgcr;
	mov	ax,rgcr
	mov	[bx.rgcrFvb],ax

;    pfvb->ccrMax = ccrMax;
	mov	ax,ccrMax
	mov	[bx.ccrMaxFvb],ax

;}
cEnd

; End of InitFvbBufs






;-------------------------------------------------------------------------
;	FetchVisibleRgch ( pfvb, fvc, fProps, ffe )
;-------------------------------------------------------------------------
;/* F E T C H  V I S I B L E  R G C H */
;/*  Fetch a buffer full of characters from the doc.  Arguments are passes in
;    an FVB.  Specifically: doc, cpFirst, cpLim -- range to be fetched,
;    rgch, cchMax, cch -- buffer for chars, size of buffer & number of chars
;    fetched, rgcr, ccrMax, ccr -- buffer for cp runs, size of buffer &
;    number of cr's returned.  Note that both rgch & rgcr are required by
;    this function.  Fetch stops when either buffer full of cpLim reached.
;    cpFirst & fOverflow are set on return.  If fProps, runs in rgcr will
;    have the same properties.  If ffe & ffeNested then if cpFirst is nested in an
;    invisible portion of a field, you will still get those characters.
;*/
;
;native FetchVisibleRgch (pfvb, fvc, fProps, ffe)
;struct FVB * pfvb;
;int fvc;
;BOOL fProps;
; int ffe;
;
;{
;    int icr = -1;
;    CHAR * pch = pfvb->rgch;
;    int ccp;
;    int ccpFetch;
;    CP cpCur = pfvb->cpFirst;

; %%Function:N_FetchVisibleRgch %%Owner:BRADV
cProc	N_FetchVisibleRgch,<PUBLIC,FAR>,<si,di>
	ParmW	pfvb
	ParmW	fvc
	ParmW	fProps
	ParmW	ffe

	LocalW	icr
	LocalW	ccp
	LocalW	ccpFetch
	LocalD	cpCur

	;pfvb kept in si
	;pch kept in di
cBegin

	mov	si,pfvb
	mov	icr,-1
	mov	di,[si.rgchFvb]
	mov	ax,[si.LO_cpFirstFvb]
	mov	dx,[si.HI_cpFirstFvb]
	mov	OFF_cpCur,ax
	mov	SEG_cpCur,dx

;    pfvb->ccr = pfvb->cch = 0;
	mov	[si.ccrFvb],0
	mov	[si.cchFvb],0

FVR01:
;    while (cpCur < pfvb->cpLim && pfvb->cch < pfvb->cchMax)
;        {
	mov	ax,OFF_cpCur
	mov	dx,SEG_cpCur
	sub	ax,[si.LO_cpLimFvb]
	sbb	dx,[si.HI_cpLimFvb]
	jge	FVR015
	mov	ax,[si.cchFvb]
	cmp	ax,[si.cchMaxFvb]
	jl	Ltemp001
FVR015:
	jmp	FVR08
Ltemp001:

;        FetchCpPccpVisible ((icr<0 ? pfvb->doc : docNil),
;		 cpCur, &ccpFetch, fvc, ffe);
	errnz	<docNil>
	xor	ax,ax
	cmp	icr,0
	jge	FVR02
	mov	ax,[si.docFvb]
FVR02:
	lea	bx,ccpFetch
	push	ax
	push	SEG_cpCur
	push	OFF_cpCur
	push	bx
	push	fvc
	push	ffe
ifdef DEBUG
	cCall	S_FetchCpPccpVisible,<>
else ;not DEBUG
	push	cs
	call	near ptr N_FetchCpPccpVisible
endif ;DEBUG

;        if (ccpFetch == 0)
;            /* reached end of document */
;            break;
        cmp     ccpFetch,0
        je      FVR015 ; jmp FVR08
        

;        if (cpCur != vcpFetch || icr < 0 || fProps)
;            {
	mov	dx,icr
	mov	ax,wlo vcpFetch
	cmp	ax,OFF_cpCur
	jne	FVR03
	mov	ax,whi vcpFetch
	cmp	ax,SEG_cpCur
	jne	FVR03
	or	dx,dx
	jl	FVR03
	cmp	fProps,fFalse
	je	FVR04

FVR03:
;            if (++icr >= pfvb->ccrMax)
;                break;
	inc	dx
	mov	icr,dx
	cmp	dx,[si.ccrMaxFvb]
	jge	FVR015

;            pfvb->ccr++;
	inc	[si.ccrFvb]

;            pfvb->rgcr [icr].cp = vcpFetch;
	errnz	<cbCrMin - 6>
	mov	bx,dx
	shl	bx,1
	add	bx,dx
	shl	bx,1
	add	bx,[si.rgcrFvb]
	mov	ax,wlo vcpFetch
	mov	[bx.LO_cpCr],ax
	mov	ax,whi vcpFetch
	mov	[bx.HI_cpCr],ax
	
;            pfvb->rgcr [icr].ccp = 0;
	mov	[bx.ccpCr],0

;            }

FVR04:
;        ccp = pfvb->cchMax - pfvb->cch;
	mov	ax,[si.cchMaxFvb]
	sub	ax,[si.cchFvb]

;        if (ccp > ccpFetch)
	cmp	ax,ccpFetch
	jle	FVR05

;            ccp = ccpFetch;
	mov	ax,ccpFetch

FVR05:
;	for the following in-line CpMax to work we must have ccp >= 0
;	Assert (ccp >= 0);
ifdef DEBUG
;	/* Assert (ccp >= 0) with a call so as not to mess up
;	short jumps */
	call	FVR09
endif ;/* DEBUG */
;	if (ccp > pfvb->cpLim - vcpFetch)
	mov	bx,[si.LO_cpLimFvb]
	mov	cx,[si.HI_cpLimFvb]
	sub	bx,wlo vcpFetch
	sbb	cx,whi vcpFetch

;            ccp = CpMax (pfvb->cpLim - vcpFetch, cp0);
;	if ((cpT = pfvb->cpLim - vcpFetch) > 2^16) ccp is valid now
	jg	FVR07
;	if (cpT < 0) cpT = 0;
	jge	FVR06
	xor	bx,bx
FVR06:
	cmp	ax,bx
	jbe	FVR07
	mov	ax,bx

FVR07:

;	 bltbh (vhpchFetch, pch, ccp);
;        pch += ccp;
	push	si
	;LN_ReloadSbFetch takes the sb from vhpchFetch and sets es to the
	;corresponding value
	;Only es and bx are altered.
	call	LN_ReloadSbFetch
	mov	si,wlo (vhpchFetch)
	push	es
	pop	ds
	push	ss
	pop	es
	mov	cx,ax
	rep	movsb
	push	ss
	pop	ds
	pop	si

;        pfvb->cch += ccp;
	add	[si.cchFvb],ax

;        pfvb->rgcr [icr].ccp += ccp;
	errnz	<cbCrMin - 6>
	mov	bx,dx
	shl	bx,1
	add	bx,dx
	shl	bx,1
	add	bx,[si.rgcrFvb]
	add	[bx.ccpCr],ax

;        cpCur = vcpFetch + ccp;
	xor	dx,dx
	add	ax,wlo vcpFetch
	adc	dx,whi vcpFetch
	mov	OFF_cpCur,ax
	mov	SEG_cpCur,dx

;        }
	jmp	FVR01

FVR08:
;    pfvb->cpFirst = cpCur;
	mov	ax,OFF_cpCur
	mov	dx,SEG_cpCur
	mov	[si.LO_cpFirstFvb],ax
	mov	[si.HI_cpFirstFvb],dx

;    pfvb->fOverflow = (ccpFetch != 0 && cpCur < pfvb->cpLim);
	xor	bx,bx
	cmp	ccpFetch,bx
        je      FVR11
	sub	ax,[si.LO_cpLimFvb]
	sbb	dx,[si.HI_cpLimFvb]
	adc	bx,0
FVR11:
	mov	[si.fOverflowFvb],bx

;}
cEnd

; End of FetchVisibleRgch


ifdef DEBUG
;	Assert (ccp >= 0);
FVR09:
	or	ax,ax
	jge	FVR10
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,midFieldcrn
	mov	bx,1005
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FVR10:
	ret
endif ;/* DEBUG */


;-------------------------------------------------------------------------
;	FetchCpPccpVisible ( doc, cp, pccp, fvc, ffe )
;-------------------------------------------------------------------------
;/* F E T C H  C P  P C C P  V I S I B L E */
;/*  Fetch a run of characters which are visible according to fvc.  Fvc can
;    be fvcScreen, which is what the user actually sees, or fvcInstructions
;    or fvcResults which shows instructions/results always.  Takes vanished
;    text, dead fields and fields instructions/results into consideration.
;
;    *pccp will be filled with the number of characters fetched.  This is the
;    correct number to use, NOT vccpFetch.  All other v*Fetch values are
;    valid.
;
;    For sequential fetch use doc==docNil and assure that *pccp has not
;    been modified since previous fetch!
;
;    If ffe & ffeNested then if cp is nested in a vanished portion of a field will
;    still fetch cp and characters at the same nesting level (used when
;    getting instructions).
;*/
;
;native FetchCpPccpVisible (doc, cp, pccp, fvc, ffe)
;int doc;
;CP cp;
;int *pccp, fvc;
;int ffe;
;
;{
;    int docFetch = doc;
;    CP cpMac;
;    CP cpIn;
;    BOOL fUseCache = fFalse, fMayUseCache;
;            CHAR * pch;
;            CHAR * pchLim;
;            int ifld;
;            CP dcp;
;            struct FLCD flcd;

; %%Function:N_FetchCpPccpVisible %%Owner:BRADV
cProc	N_FetchCpPccpVisible,<PUBLIC,FAR>,<si,di>
	ParmW	doc
	ParmD	cp
	ParmW	pccp
	ParmW	fvc
	ParmW	ffe

	LocalW	docFetch
	LocalW	OFF_hpchLim
	LocalW	fUseCache	;fMayUseCache is high byte
	LocalD	cpFirstInvisi
	LocalV	flcd,cbFlcdMin

;	doc is kept in si
;	hpch is kept in es:di
;	ifld is kept in ax
;	dcp is kept in dx:ax
	
cBegin

	mov	[fUseCache],fFalse  ;sets fMayUseCache to fFalse also.
	mov	si,[doc]
	mov	[docFetch],si

ifdef DEBUG
;    Assert ((fvc & fvcmWw && !(fvc & ~fvcmWw)) || !(fvc & fvcmWw));
	push	ax
	push	bx
	push	cx
	push	dx
	mov	cx,[fvc]
	errnz	<fvcmWw - 000FFh>
	or	cl,cl
	je	FCPV005
	or	ch,ch
	je	FCPV005

	mov	ax,midFieldcrn
	mov	bx,1006
	cCall	AssertProcForNative,<ax,bx>
FCPV005:
	pop	dx
	pop	cx
	pop	bx
	pop	ax

;    Debug(vcVisiCalls++);
	inc	[vcVisiCalls]
endif ;/* DEBUG */
;    Assert (wwMax <= fvcmWw && wwNil == 0);
	errnz	<wwMax AND (NOT fvcmWw)>
	errnz	<wwNil>

;    /*  sequential fetch, figure out where we are */
;    if (doc == docNil)
;        {
	errnz	<docNil>
	or	si,si
	jne	FCPV02

;        doc = vdocFetch;
	mov	si,[vdocFetch]

;        cp = vcpFetch + *pccp;
	mov	ax,wlo [vcpFetch]
	mov	dx,whi [vcpFetch]
	mov	bx,[pccp]
ifdef DEBUG
;	Need to assert *pccp >= 0 here because I am doing an unsigned extend
;	/* Assert (*pccp >= 0) with a call so as not to mess up
;	short jumps */
	call	FCPV22
endif ;/* DEBUG */
	mov	bx,[bx]
	add	ax,bx
	adc	dx,0
	mov	[OFF_cp],ax
	mov	[SEG_cp],dx

;        if (*pccp != vccpFetch)
	cmp	bx,[vccpFetch]
	je	FCPV01

;            docFetch = doc;
	mov	[docFetch],si

FCPV01:
;        Assert (docLastFetchVisi == doc);
;	 Assert (cpFirstNextFetchVisi == cp);
ifdef DEBUG
;	/* Assert (docLastFetchVisi == doc && cpFirstNextFetchVisi == cp)
;	 with a call so as not to short jumps */
	call	FCPV24
endif ;/* DEBUG */

;        }
	jmp	short FCPV03

FCPV02:

;    else if (!(ffe & ffeNested))
	errnz	<ffeNested - 1>
	test	bptr ([ffe]),ffeNested
	jnz	FCPV03

;        /*  assures that cp is "visible" */
;	 cp = CpVisibleCpField (doc, cp, fvc, fFalse /* fIns */);

	errnz	<fFalse>
	xor	ax,ax
	push	si
	push	[SEG_cp]
	push	[OFF_cp]
	push	fvc
	push	ax
ifdef DEBUG
	cCall	S_CpVisibleCpField,<>
else ;not DEBUG
	push	cs
	call	near ptr N_CpVisibleCpField
endif ;DEBUG
	mov	[OFF_cp],ax
	mov	[SEG_cp],dx
	
FCPV03:
;    cpIn = cp;
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	push	dx
	push	ax

;    /*  can the cache be used? */
;    fMayUseCache = (cp < vcpFetchFirstVisi && doc == vdocFetchVisi &&
;	     fvc == vfvcFetchVisi);
	sub	ax,wlo [vcpFetchFirstVisi]
	sbb	dx,whi [vcpFetchFirstVisi]
	jge	FCPV035
	cmp	si,[vdocFetchVisi]
	jne	FCPV035
	mov	ax,[vfvcFetchVisi]
	cmp	ax,[fvc]
	jne	FCPV035
	;Assembler note: at this point we know fMayUseCache == fFalse;
	inc	byte ptr ([fUseCache+1])
FCPV035:

;    Debug (docLastFetchVisi = doc);
ifdef DEBUG
	mov	[docLastFetchVisi],si
endif ;DEBUG

;	CpMacDoc is performed in line below in the assembler version.
;    cpMac = CpMacDoc (doc);

FCPV04:
;    for (;;)
;        {
;LFetch:
LFetch:
;	 if (fMayUseCache && cp >= vcpFetchVisi)
;	     /* have entered cached area, jump forward */
;	     {
	cmp	bptr ([fUseCache+1]),fFalse
	je	FCPV05
	mov	ax,wlo [vcpFetchFirstVisi]
	mov	dx,whi [vcpFetchFirstVisi]
	mov	bx,[OFF_cp]
	mov	cx,[SEG_cp]
	sub	bx,ax
	sbb	cx,dx
	jl	FCPV05

;	     cp = vcpFetchFirstVisi;
	mov	[OFF_cp],ax
	mov	[SEG_cp],dx

;	     fMayUseCache = fFalse;
;	     fUseCache = fTrue;
	errnz	<fTrue - 1>
	mov	[fUseCache],fTrue   ;also sets fMayUseCache to fFalse

;	     docFetch = doc;
	mov	[docFetch],si

;	     Debug(vcVisiUsedCache++);
ifdef DEBUG
	inc	[vcVisiUsedCache]
endif ;DEBUG

;	     }
FCPV05:

;        if (cp >= cpMac)
;	     /*  end of document reached */
	;LN_CpMacDoc returns CpMacDoc(si) in dx:ax.
	;Only ax and dx are altered.
	call	LN_CpMacDoc
	mov	bx,[OFF_cp]
	mov	cx,[SEG_cp]
	cmp	bx,ax
	mov	ax,cx
	sbb	ax,dx
	jl	FCPV10

;            {
;            *pccp = 0;
	mov	bx,[pccp]
	mov	wptr [bx],0

;	     Debug (cpFirstNextFetchVisi = cp);
ifdef DEBUG
	;Do this with a call so as not to mess up short jumps.
	call	FCPV27
endif ;DEBUG

;	     goto LSetCache;
	jmp	LSetCache

;            }

FCPV10:
;        CachePara (doc, cp);
	push	cx
	push	bx
ifdef DEBUG
	cCall	S_CachePara,<si,cx,bx>
else ;not DEBUG
	cCall	N_CachePara,<si,cx,bx>
endif ;DEBUG
	pop	bx
	pop	cx

;        FetchCp (docFetch, cp, fcmChars+fcmProps);
	mov	ax,fcmChars+fcmProps
ifdef DEBUG
	cCall	S_FetchCp,<[docFetch],cx,bx,ax>
else ;not DEBUG
	cCall	N_FetchCp,<[docFetch],cx,bx,ax>
endif ;DEBUG

;        /*  run of vanished text, skip whole run */
;        if ((vchpFetch.fVanish || vchpFetch.fFldVanish) &&
;		 ( vchpFetch.fSysVanish ||
;		   (!(fvc & fvcmWw) && !(fvc & fvcmProps)) ||
;		   (fvc & fvcmWw &&
;		     !(PwwdWw(fvc/*ww*/)->grpfvisi.fSeeHidden ||
;		     PwwdWw(fvc/*ww*/)->grpfvisi.fvisiShowAll)
;		   )
;		 )
;	     {

	errnz	<(fFldVanishChp) - (fVanishChp)>
	test	[vchpFetch.fVanishChp],maskfVanishChp + maskfFldVanishChp
	je	FCPV12
	test	[vchpFetch.fSysVanishChp],maskfSysVanishChp
	jne	FCPV11
	mov	bx,fvc
	test	bx,fvcmWw + fvcmProps
	je	FCPV11
	errnz	<fvcmWw - 000FFh>
	or	bl,bl
	je	FCPV12
	;Takes ww in bx, result in bx.	Only bx is altered.
	call	LN_PwwdWw
	errnz	<(maskfSeeHiddenGrpfvisi+maskfvisiShowAllGrpfvisi) AND 0FF00h>
	test	bptr [bx.grpfvisiWwd],maskfSeeHiddenGrpfvisi+maskfvisiShowAllGrpfvisi
	jne	FCPV12

;	     if (FInTableDocCp(doc, vcpFetch) &&
;		 vcpFetch + vccpFetch == caPara.cpLim &&
;		 *(vhpchFetch+vccpFetch-1) == chTable)
;		 {
	push	si
	push	whi [vcpFetch]
	push	wlo [vcpFetch]
ifdef DEBUG
	cCall	S_FInTableDocCp,<>
else ;not DEBUG
	cCall	N_FInTableDocCp,<>
endif ;DEBUG
	xchg	ax,cx
	jcxz	LSkipRun
	mov	ax,[vccpFetch]
	mov	bx,[caPara.LO_cpLimCa]
	mov	cx,[caPara.HI_cpLimCa]
	sub	bx,wlo [vcpFetch]
	sbb	cx,whi [vcpFetch]
	jne	LSkipRun
	cmp	ax,bx
	jne	LSkipRun
	;LN_ReloadSbFetch takes the sb from vhpchFetch and sets es to the
	;corresponding value
	;Only es and bx are altered.
	call	LN_ReloadSbFetch
	mov	bx,wlo [vhpchFetch]
	add	bx,ax
	cmp	bptr es:[bx-1],chTable
	jne	LSkipRun

;		 if (vccpFetch > 1)
;		     {
;		     cp += (vccpFetch-1);
;		     docFetch = doc;
;		     continue;
;		     }
;		 /* else: cannot hide chTable */
;		 }
	dec	ax
	jle	FCPV12
	cwd
	jmp	short FCPV115

;LSkipRun:	 cp += vccpFetch;
;		 docFetch = docNil;
;		 continue;
;		 }
;            }
LSkipRun:
FCPV11:
	mov	ax,[vccpFetch]
	cwd
	xor	cx,cx
	db	03Dh	;turns next "mov cx,si" into "cmp ax,immediate"
FCPV115:
	mov	cx,si
FCPV117:
	mov	[docFetch],cx
	add	[OFF_cp],ax
	adc	[SEG_cp],dx
	jmp	LFetch

FCPV12:
;        /*  fSpec run--see how much, if any, we can use.
;            CASES:
;                1) cannot use any: first char in run is not visible.
;                    skip the correct number of chars and do random fetch.
;                2) can use some: non-zero number of chars are visible,
;                    but some char in run is not visible. Set *pccp to
;                    include only visible ones.
;                3) entire run is usable:  all fSpec characters are visible.
;                    return this run in its entirety.
;        */
;        if (vchpFetch.fSpec)
;            {
	test	[vchpFetch.fSpecChp],maskfSpecChp
	je	FCPV133

;	    if (ffe & ffeNoFSpec)
;	    	goto LSkipRun;
	errnz	<ffeNoFSpec - 2>
	test	bptr ([ffe]),ffeNoFSpec
	jnz	LSkipRun

;	     CHAR HUGE * hpch = vhpchFetch;
	mov	di,wlo ([vhpchFetch])

;	     CHAR HUGE * hpchLim = hpch + vccpFetch;
	mov	ax,[vccpFetch]
	add	ax,di
	mov	[OFF_hpchLim],ax

;            int ifld;
;            CP dcp;
;            struct FLCD flcd;

FCPV13:
;	     while (hpch < hpchLim)
;                {
	cmp	di,[OFF_hpchLim]
	jb	Ltemp002
FCPV133:
	jmp	short FCPV17
Ltemp011:
	jmp	short FCPV115
Ltemp002:

;                ifld = IfldFromDocCp (doc, cp, fTrue);
	mov	ax,fTrue
	push	si
	push	[SEG_cp]
	push	[OFF_cp]
	push	ax
ifdef DEBUG
	cCall	S_IfldFromDocCp,<>
else ;not DEBUG
	cCall	N_IfldFromDocCp,<>
endif ;DEBUG

;                if (ifld != ifldNil)
;                    {
	cmp	ax,ifldNil
	je	FCPV137

;                    GetIfldFlcd (doc, ifld, &flcd);
	lea	bx,[flcd]
	push	si
	push	ax
	push	bx
ifdef DEBUG
	cCall	S_GetIfldFlcd,<>
else ;not DEBUG
	cCall	N_GetIfldFlcd,<>
endif ;DEBUG

;		     if ((dcp = DcpSkipFieldChPflcd (*hpch, &flcd,
;			     FShowResultPflcdFvc (&flcd, fvc),
;			     fvc&fvcmWw?1:0/*fFetch*/)) != 0)
	lea	bx,[flcd]
	push	bx
	push	[fvc]
ifdef DEBUG
	cCall	S_FShowResultPflcdFvc,<>
else ;not DEBUG
	cCall	N_FShowResultPflcdFvc,<>
endif ;DEBUG
	;LN_ReloadSbFetch takes the sb from vhpchFetch and sets es to the
	;corresponding value
	;Only es and bx are altered.
	call	LN_ReloadSbFetch
	xor	bx,bx
	mov	bl,es:[di]
	push	bx
	lea	cx,[flcd]
	push	cx
	push	ax
	errnz	<fvcmWw - 000FFh>
	cmp	bptr ([fvc]),1
	sbb	ax,ax
	inc	ax
	push	ax
ifdef DEBUG
	cCall	S_DcpSkipFieldChPflcd,<>
else ;not DEBUG
	cCall	N_DcpSkipFieldChPflcd,<>
endif ;DEBUG
	mov	bx,ax
	or	bx,dx
	jne	FCPV14
FCPV137:
	;Assembler note: perform the loop end code here and avoid
	;some jumps
;		 hpch++;
	inc	di

;                cp++;
	add	[OFF_cp],00001h
	adc	[SEG_cp],00000h

;                }
	jmp	short FCPV13
Ltemp010:
	jmp	FCPV20
	;Assembler note: finished loop end code

;			 if (hpch == vhpchFetch)
;                            {
;                            /*  skip vanished portion of field */
;                            cp += dcp;
;                            docFetch = doc;
;                            goto LFetch;
;                            }
FCPV14:
	cmp	di,wlo ([vhpchFetch])
	je	Ltemp011

;                        else
;                            {
;                            /*  part of run useable */
;			     *pccp = hpch - vhpchFetch;
	mov	ax,di
	sub	ax,wlo ([vhpchFetch])
	mov	bx,[pccp]
	mov	[bx],ax
;                            Assert(cp == vcpFetch + *pccp);
;PAUSE
ifdef DEBUG
	;Do this with a call so as not to mess up short jumps.
	call	FCPV28
endif ;DEBUG

;			     goto LCheckOutline:
	jmp short FCPV171
;                            }
;                    }

	;Assembler note: this code is done just before FCPV14
;		 hpch++;
;                cp++;
;                }
;            } /* if (vchpFetch.fSpec) */

FCPV17:
;	*pccp = vccpFetch;
;PAUSE
	mov	bx,[pccp]
	mov	ax,[vccpFetch]
	mov	[bx],ax

;LCheckOutline:
;	/* See if we have to screen out some characters due to outline mode */
FCPV171:

;	if ((fvc & fvcmWw) && PwwdWw(fvc)->fOutline)
;	    {

	mov	bx,[fvc]
	errnz	<fvcmWw - 000FFh>
	or	bl,bl
	je	Ltemp010
	;Takes ww in bx, result in bx.	Only bx is altered.
	call	LN_PwwdWw
	test	bptr [bx.fOutlineWwd],maskfOutlineWwd
	je	Ltemp010

;	    CP cpFirstInvisi = cpNil;
	errnz	<LO_cpNil - HI_cpNil>
	mov	cx,LO_cpNil
	mov	[OFF_cpFirstInvisi],cx
	mov	[SEG_cpFirstInvisi],cx

;           cp = vcpFetch;
	mov	dx,whi ([vcpFetch])
	mov	[SEG_cp],dx
	mov	cx,wlo ([vcpFetch])
        mov     [OFF_cp],cx

;	    if (!FCpVisiInOutline(fvc /*ww*/, doc, cp, *pccp, &cpFirstInvisi))
;		{
;		/* all visible */
;		*pccp = ccpFetch;
;		}
;PAUSE
	push	[fvc]
	push	si
	push	dx
	push	cx
	mov	bx,[pccp]
	push	[bx]
	lea	bx,[cpFirstInvisi]
	push	bx
ifdef DEBUG
	cCall	S_FCpVisiInOutline,<>
else ;not DEBUG
	push	cs
	call	near ptr N_FCpVisiInOutline
endif ;DEBUG
	or	ax,ax
	jne	FCPV195

;	    if (cpFirstInvisi == cp)
;		{ /* all invisible */
;		docFetch = doc;
;		/* skip all hidden level text */
;		cp = CpNextVisiInOutline(fvc /*ww*/, doc, cp);
;		goto LFetch;
;		}
	mov	cx,[OFF_cpFirstInvisi]
	mov	dx,[SEG_cpFirstInvisi]
	sub	cx,[OFF_cp]
	sbb	dx,[SEG_cp]
	or	dx,cx
	jne	FCPV178
	;***Begin in-line CpNextVisiInOutline
	push	si	;save doc
	push	di	;save OFF_hpch
;/* return the cpFirst of the next pad after cp that is fShow */
;EXPORT CP CpNextVisiInOutline(ww, doc, cp)
;int ww;
;int doc;
;CP cp;
;{
;    int ipad;
;    int ipadMac;
;    struct PAD pad;
;    struct PLC **hplcpad = PdodDoc(doc)->hplcpad;
	mov	bx,si
	;Takes doc in bx, result in bx.  Only bx is altered.
	call	LN_PdodDoc
	mov	si,[bx.hplcpadDod]

;    Assert(PwwdWw(ww)->fOutline && hplcpad != hNil);
ifdef DEBUG
;	/* Assert(PwwdWw(ww)->fOutline && hplcpad != hNil)
;	 with a call so as not to short jumps */
	call	FCPV32
endif ;/* DEBUG */

;    ipad = IInPlc(hplcpad, cp);
	cCall	IInPlc,<si, [SEG_cp], [OFF_cp]>

;    ipadMac = IMacPlc(hplcpad);
	mov	bx,[si]
	mov	cx,[bx.iMacPlcStr]
	push	cx	;save ipadMac
	push	ax	;save ipad
;	QcpQfooPlcfoo takes hplcfoo in bx, ifoo in si, it returns pplcfoo
;	in bx, cbfoo in cx, qcp in es:di, qfoo in es:si.
;	if DEBUG it returns hpcp in dx:di, hpfoo in dx:si.
;	Changes ax, bx, cx, dx, si, di.
	xchg	ax,si
	xchg	ax,bx
ifdef DEBUG
	xor	di,di	;Not O.K. to pass ifldMac
endif ;DEBUG
	cCall	N_QcpQfooPlcfoo,<>
ifdef DEBUG
	;Check that es == mpsbps[dx];
	call	FCPV35
endif ;DEBUG
	pop	cx	;restore ipad
	pop	dx	;restore ipadMac

;    while (++ipad < ipadMac)
;	 {
;	 GetPlc(hplcpad, ipad, &pad);
;	 if (pad.fShow)
;	    return CpPlc(hplcpad, ipad);
;	 }
	errnz	<cbPadMin - 2>
	inc	si
	inc	si
FCPV172:
	inc	cx
	cmp	cx,dx
	jge	FCPV174
	add	di,4
	errnz	<cbPadMin - 2>
	lods	wptr es:[si]
	errnz	<(fShowPad) - 0>
	test	al,maskFShowPad
	je	FCPV172
	;***Begin in-line CpPlc
	mov	ax,es:[di]
	mov	dx,es:[di+2]
	cmp	cx,[bx.icpAdjustPlc]
	jl	FCPV173
	add	ax,[bx.LO_dcpAdjustPlc]
	adc	dx,[bx.HI_dcpAdjustPlc]
FCPV173:
	;***End in-line CpPlc
	db	0A8h	;turns next "stc" into "test al,immediate"
			;also clears the carry flag
FCPV174:
	stc
	pop	di	;restore OFF_hpch
	pop	si	;restore doc
	jnc	FCPV176

;    return CpMacDoc(doc);
;}
	;LN_CpMacDoc returns CpMacDoc(si) in dx:ax.
	;Only ax and dx are altered.
	call	LN_CpMacDoc
FCPV176:
	;***End in-line CpNextVisiInOutline
	mov	[OFF_cp],ax
	mov	[SEG_cp],dx
	mov	[docFetch],si
	jmp	LFetch
FCPV178:

;	    else
;		{
;		/* part of the run is useable */
;		Assert(cpFirstInvisi != cpNil);
;		Assert(cpFirstInvisi <= cp+*pccp);
ifdef DEBUG
;	/* Do these asserts with a call so as not to mess up
;	short jumps */
	call	FCPV29
endif ;/* DEBUG */

;		*pccp = cpFirstInvisi - cp;
;		}
	xchg	ax,cx
FCPV18:
	mov	bx,[pccp]
	mov	[bx],ax

;	    /* this is to restore FetchCp globals because FCpVisiInOutline
;	       calls FetchCp with possibly something other than cp */
;	    FetchCpAndPara(doc, cp, fcmChars+fcmProps);
;	    }
FCPV195:
;PAUSE
	mov	ax,fcmChars+fcmProps
	cCall	FetchCpAndPara,<si,[SEG_cp],[OFF_cp],ax>

FCPV20:

;	 Debug (cpFirstNextFetchVisi = vcpFetch + *pccp);
ifdef DEBUG
	push	ax
	push	bx
	push	dx
	mov	bx,[pccp]
	mov	ax,[bx]
	cwd
	add	ax,wlo [vcpFetch]
	adc	dx,whi [vcpFetch]
	mov	wlo [cpFirstNextFetchVisi],ax
	mov	whi [cpFirstNextFetchVisi],dx
	pop	dx
	pop	bx
	pop	ax
endif ;DEBUG

;LSetCache:
;	 if (!fUseCache || cpIn < vcpFetchVisi)
;	     {
;	     vdocFetchVisi = doc;
;	     vcpFetchVisi = cpIn;
;	     vfvcFetchVisi = fvc;
;	     vcpFetchFirstVisi = *pccp ? vcpFetch : cpMac;
;	     }
LSetCache:
	pop	ax
	pop	dx	;restore cpIn
	cmp	bptr ([fUseCache]),fFalse
	je	FCPV203
	cmp	ax,wlo [vcpFetchVisi]
	push	dx
	sbb	dx,whi [vcpFetchVisi]
	pop	dx
	jge	FCPV207
FCPV203:
	mov	[vdocFetchVisi],si
	mov	wlo [vcpFetchVisi],ax
	mov	whi [vcpFetchVisi],dx
	mov	ax,[fvc]
	mov	[vfvcFetchVisi],ax
	mov	ax,wlo [vcpFetch]
	mov	dx,whi [vcpFetch]
	mov	bx,[pccp]
	cmp	wptr [bx],fFalse
	jne	FCPV205
	;LN_CpMacDoc returns CpMacDoc(si) in dx:ax.
	;Only ax and dx are altered.
	call	LN_CpMacDoc
FCPV205:
	mov	wlo [vcpFetchFirstVisi],ax
	mov	whi [vcpFetchFirstVisi],dx
FCPV207:

;        return;

;        }  /* for (;;) */

FCPV21:
;}
cEnd

; End of FetchCpPccpVisible


ifdef DEBUG
;	Assert (*pccp >= 0);
FCPV22:
	cmp	wptr [bx],0
	jge	FCPV23
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,midFieldcrn
	mov	bx,1007
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FCPV23:
	ret
endif ;/* DEBUG */


ifdef DEBUG
;	Assert (docLastFetchVisi == doc && cpFirstNextFetchVisi == cp);
FCPV24:
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	[docLastFetchVisi],si
	jne	FCPV25
	mov	ax,wlo [cpFirstNextFetchVisi]
	mov	dx,whi [cpFirstNextFetchVisi]
	cmp	ax,[OFF_cp]
	jne	FCPV25
	cmp	dx,[SEG_cp]
	je	FCPV26

FCPV25:
	mov	ax,midFieldcrn
	mov	bx,1008
	cCall	AssertProcForNative,<ax,bx>
FCPV26:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */


;	     Debug (cpFirstNextFetchVisi = cp);
ifdef DEBUG
FCPV27:
	push	ax
	mov	ax,[OFF_cp]
	mov	wlo [cpFirstNextFetchVisi],ax
	mov	ax,[SEG_cp]
	mov	whi [cpFirstNextFetchVisi],ax
	pop	ax
	ret
endif ;DEBUG


;	     Assert(cp == vcpFetch + *pccp);
ifdef DEBUG
FCPV28:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	sub	ax,wlo [vcpFetch]
	sbb	dx,whi [vcpFetch]
	mov	bx,[pccp]
	sub	ax,[bx]
	sbb	dx,0
	or	ax,dx
	je	FCPB281
	mov	ax,midFieldcrn
	mov	bx,1041
	cCall	AssertProcForNative,<ax,bx>
FCPB281:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG


;		Assert(cpFirstInvisi != cpNil);
;		Assert(cpFirstInvisi <= cp+*pccp);
ifdef DEBUG
FCPV29:
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	[OFF_cpFirstInvisi],LO_cpNil
	jne	FCPV30
	cmp	[SEG_cpFirstInvisi],HI_cpNil
	jne	FCPV30
	mov	ax,midFieldcrn
	mov	bx,1009
	cCall	AssertProcForNative,<ax,bx>
FCPV30:
	mov	bx,[pccp]
	mov	ax,[bx]
	cwd
	add	ax,[OFF_cp]
	adc	dx,[SEG_cp]
	sub	ax,[OFF_cpFirstInvisi]
	sbb	dx,[SEG_cpFirstInvisi]
	jge	FCPV31
	mov	ax,midFieldcrn
	mov	bx,1010
	cCall	AssertProcForNative,<ax,bx>
FCPV31:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

ifdef DEBUG
;	Assert(PwwdWw(ww)->fOutline && hplcpad != hNil);
FCPV32:
	push	ax
	push	bx
	push	cx
	push	dx
	;Takes ww in bx, result in bx.	Only bx is altered.
	mov	bx,[fvc]
	call	LN_PwwdWw
	test	bptr [bx.fOutlineWwd],maskfOutlineWwd
	je	FCPV33
	errnz	<hNil>
	or	si,si
	jne	FCPV34
FCPV33:
	mov	ax,midFieldcrn
	mov	bx,1042
	cCall	AssertProcForNative,<ax,bx>
FCPV34:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

ifdef DEBUG
FCPV35:
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
	jc	FCPV36
	;Assembler note: There is no way we should have to call ReloadSb here.
;	reload sb trashes ax, cx, and dx
;	cCall	ReloadSb,<>
	mov	ax,midFieldcrn
	mov	bx,1033
	cCall	AssertProcForNative,<ax,bx>
FCPV36:
	pop	ax	;restore es from QcpQfooPlcfoo
	mov	bx,es	;compare with es rederived from the SB of QcpQfooPlcfoo
	cmp	ax,bx
	je	FCPV37
	mov	ax,midFieldcrn
	mov	bx,1034
	cCall	AssertProcForNative,<ax,bx>
FCPV37:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

;struct WDC /* WwDocCp */
;	{
;	CP	cp;
;	int	doc;
;	int	ww;
;	};
cpWdc		    equ     [dword ptr 0FFFCh]
LO_cpWdc	    equ     [word ptr 0FFFCh]
HI_cpWdc	    equ     [word ptr 0FFFEh]
docWdc		    equ     [word ptr 00000h]
wwWdc		    equ     [word ptr 00002h]

;-------------------------------------------------------------------------
;	GetCpFirstCpLimDisplayPara (ww, doc, cp, pcpFirst, pcpLim)
;-------------------------------------------------------------------------
;/* G E T  C P	F I R S T  C P	L I M  D I S P L A Y  P A R A */
;/*  Return cpLim, the smallest cp > cp such that:
;    CpFormatFrom(cpLim) != (cpFirst = CpFormatFrom (cp))
;*/
;NATIVE void GetCpFirstCpLimDisplayPara (ww, doc, cp, pcpFirst, pcpLim)
;int ww;
;int doc;
;CP cp;
;CP *pcpFirst, *pcpLim;
;{
;    CP cpFrom;
;    CP cpMac = CpMacDoc (doc);
;    CP cpIn = cp;
;    CP cpT, cpTable;
;    int fAbs, fInTable, fInTableStart, ccp;
;    int pap[cwPAPBaseScan];

; %%Function:N_GetCpFirstCpLimDisplayPara %%Owner:BRADV
cProc	N_GetCpFirstCpLimDisplayPara,<PUBLIC,FAR>,<si,di>
	ParmW	ww
	OFFBP_ww		=   -2
	ParmW	doc
	OFFBP_doc		=   -4
	ParmD	cp
	OFFBP_cp		=   -8
	ParmW	pcpFirst
	ParmW	pcpLim

	LocalW	fAbsVar
	LocalW	fTable
	LocalW	fInTableStart
	LocalW	ccp
	LocalD	cpFrom
	LocalD	cpT
	LocalD	cpIn
	LocalD	cpMacDocVar
	LocalD	cpTable
	LocalV	pap, cbPAPBaseScan
	
cBegin

	mov	si,[doc]
	call	LN_CpMacDoc
	mov	[OFF_cpMacDocVar],ax
	mov	[SEG_cpMacDocVar],dx

	lea	si,[doc]	;register pWDC
	errnz	<OFFBP_ww - OFFBP_doc - (wwWDC)>
	errnz	<OFFBP_doc - OFFBP_doc - (docWDC)>
	errnz	<OFFBP_cp - OFFBP_doc - (cpWDC)>
	mov	ax,[OFF_cp]
	mov	[OFF_cpIn],ax
	mov	ax,[SEG_cp]
	mov	[SEG_cpIn],ax

;   Assert (cp < cpMac);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	bx,[OFF_cp]
	mov	cx,[SEG_cp]
	sub	bx,[OFF_cpMacDocVar]
	sbb	cx,[SEG_cpMacDocVar]
	jl	GCFCLDP01
	mov	ax,midFieldcrn
	mov	bx,1011
	cCall	AssertProcForNative,<ax,bx>
GCFCLDP01:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

	;LN_CachePara takes pWdc in si and performs
	;CachePara(pWdc->doc, pWdc->cp)
	;ax, bx, cx, dx are altered.
	call	LN_CachePara

;   if (fAbs = FAbsPap(doc, &vpapFetch))
;	blt(&vpapFetch, pap, cwPAPBaseScan);
	;LN_FAbsAndBlt takes pWdc in si, &pap in di and performs
	;if (ax = FAbsPap(pWdc->doc, &vpapFetch))
	;    blt(&vpapFetch, pap, cwPAPBaseScan);
	;ax, bx, cx, dx, di are altered.
	lea	di,[pap]
	call	LN_FAbsAndBlt
	mov	[fAbsVar],ax

;   fInTableStart = FInTableVPapFetch(doc, cp);
;#define FInTableVPapFetch(doc, cp)  (vpapFetch.fInTable && FInTableDocCp(doc, cp))
	mov	al,[vpapFetch.fInTablePap]
	and	ax,000FFh
	je	GCFCLDP015
	;LN_FInTableDocCp takes pWdc in si and performs
	;FInTableDocCp(pWdc->doc, pWdc->cp)
	;ax, bx, cx, dx are altered.
	call	LN_FInTableDocCp
GCFCLDP015:
	mov	[fInTableStart],ax

;   cpFrom = *pcpFirst = CpFormatFrom (ww, doc, cp);
	push	[si.wwWdc]
	push	[si.docWdc]
	push	[si.HI_cpWdc]
	push	[si.LO_cpWdc]
ifdef DEBUG
	cCall	S_CpFormatFrom,<>
else ;not DEBUG
	push	cs
	call	near ptr N_CpFormatFrom
endif ;DEBUG
	mov	bx,[pcpFirst]
	mov	[bx],ax
	mov	[bx+2],dx
	mov	[OFF_cpFrom],ax
	mov	[SEG_cpFrom],dx

;   do
;	{
GCFCLDP02:

;	cpT = cp;
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	mov	[OFF_cpT],ax
	mov	[SEG_cpT],dx

;	cp = CpVisibleCpField (doc, cp, ww, fFalse);
	;LN_CpVisibleCpField takes pWdc in si and performs
	;dx:ax = CpVisibleCpField (pWdc->doc, pWdc->cp, pWdc->ww, fFalse);
	;ax, bx, cx, dx are altered.
	call	LN_CpVisibleCpField
	mov	[OFF_cp],ax
	mov	[SEG_cp],dx

;	CachePara (doc, cp);
	;LN_CachePara takes pWdc in si and performs
	;CachePara(pWdc->doc, pWdc->cp)
	;ax, bx, cx, dx are altered.
	call	LN_CachePara

;	if (vpapFetch.fInTable)
;	    {
	cmp	[vpapFetch.fInTablePap],fFalse
	jne	Ltemp025
	jmp	GCFCLDP04
Ltemp025:

;	    fInTable = FInTableDocCp(doc,cp);
	;LN_FInTableDocCp takes pWdc in si and performs
	;FInTableDocCp(pWdc->doc, pWdc->cp)
	;ax, bx, cx, dx are altered.
	call	LN_FInTableDocCp
	xchg	ax,di

;	    cpTable = vcpFirstTablePara;
	mov	ax,wlo [vcpFirstTablePara]
	mov	dx,whi [vcpFirstTablePara]
	mov	[OFF_cpTable],ax
	mov	[SEG_cpTable],dx

;	    if (!fInTableStart)
;		    {
	;Assembler note: we always set cp to cpTable after this point, so
	;why not do it here?
	mov	[OFF_cp],ax
	mov	[SEG_cp],dx
	cmp	[fInTableStart],fFalse
	jne	GCFCLDP025

;		    FetchCpPccpVisible(doc, cpIn, &ccp, ww /* fvc */, 0 /* ffe */);
	lea	ax,[ccp]
	push	[doc]
	push	[SEG_cpIn]
	push	[OFF_cpIn]
	push	ax
	push	[ww]
	xor	ax,ax
	push	ax
ifdef DEBUG
	cCall	S_FetchCpPccpVisible,<>
else ;not DEBUG
	push	cs
	call	near ptr N_FetchCpPccpVisible
endif ;DEBUG

;		    if (vcpFetch >= cpTable && cpTable == CpVisibleCpField(doc, cpTable, ww, fTrue))
;			    {
;			    /* invisible characters in front of visible table */
;			    cp = cpTable;
;			    break;
;			    }
;		    }
	mov	ax,wlo [vcpFetch]
	sub	ax,[OFF_cpTable]
	mov	ax,whi [vcpFetch]
	sbb	ax,[OFF_cpTable]
	jge	GCFCLDP025
	;Assembler note: at this point we have already set cp to cpTable.
	;LN_CpVisibleCpField takes pWdc in si and performs
	;dx:ax = CpVisibleCpField (pWdc->doc, pWdc->cp, pWdc->ww, fFalse);
	;ax, bx, cx, dx are altered.
	call	LN_CpVisibleCpField
	sub	ax,[OFF_cpTable]
	sbb	dx,[SEG_cpTable]
	or	ax,dx
	je	Ltemp024
GCFCLDP025:

;	    cp = cpTable;
	;Assembler note: we have already set cp to cpTable above.

;	    if (!FVisibleCp(ww, doc, cp) && cp > cpT)
;		continue;
	mov	ax,[OFF_cpT]
	sub	ax,[OFF_cp]
	mov	ax,[SEG_cpT]
	sbb	ax,[SEG_cp]
	jge	GCFCLDP03
	push	[si.wwWdc]
	push	[si.docWdc]
	push	[si.HI_cpWdc]
	push	[si.LO_cpWdc]
ifdef DEBUG
	cCall	S_FVisibleCp,<>
else ;not DEBUG
	push	cs
	call	near ptr N_FVisibleCp
endif ;DEBUG
	or	ax,ax
	je	GCFCLDP06
GCFCLDP03:

;	    /* table starts in middle of para (because of fields) */
;	    /* second call to FInTable... must NOT pass &cp - it is to
;		check for the case of all vanished text from cpT to the
;		actual beginning of table (which will be cp) */
;	    if (!fInTable || !FInTableDocCp(doc,cpT))
;		break;
;	    }
	mov	ax,[OFF_cpT]
	mov	dx,[SEG_cpT]
	;LN_FInTableDocDxAx takes pWdc in si and performs
	;FInTableDocCp(pWdc->doc, dx:ax)
	;ax, bx, cx, dx are altered.
	call	LN_FInTableDocDxAx
ifdef DEBUG
	;Make sure we can do the following "and ax,di" with impunity
	call	GCFCLDP09
endif ;DEBUG
	and	ax,di
	jne	GCFCLDP05
Ltemp024:
	jmp	short GCFCLDP07
Ltemp021:
	jmp	GCFCLDP02

GCFCLDP04:
;	 else if (fAbs != FAbsPap(doc, &vpapFetch) ||
;		 fAbs && !FMatchAbs(doc, &vpapFetch, pap))
;		 {
	;LN_FCompareAbs takes fAbs in cx, pWdc in si, &pap in di and performs
	;if (fAbs != FAbsPap(pWdc->doc, &vpapFetch) ||
	;   fAbs && !FMatchAbs(pWdc->doc, &vpapFetch, pap))
	;ax, bx, cx, dx, di are altered.
	mov	cx,[fAbsVar]
	lea	di,[pap]
	call	LN_FCompareAbs
	je	GCFCLDP05

;		 /* apo change: from apo to non, non to apo, or apo type.
;		    if we've passed the starting cp, pull back cpLim and stop.
;		    otherwise, push cpFirst. */
;		 if (caPara.cpLim > cpIn)
;			 {
;			 cp = caPara.cpFirst; /* back up, don't include this one */
;			 break;
;			 }
	mov	ax,[OFF_cpIn]
	mov	dx,[SEG_cpIn]
	sub	ax,[caPara.LO_cpLimCa]
	sbb	dx,[caPara.HI_cpLimCa]
	mov	ax,[caPara.LO_cpFirstCa]
	mov	dx,[caPara.HI_cpFirstCa]
	jl	GCFCLDP08

;		 *pcpFirst = caPara.cpLim;
;		 }
;	 cp = caPara.cpLim;
	db	0A8h	;turns next "stc" into "test al,immediate"
			;also clears the carry flag
GCFCLDP05:
	stc
	mov	ax,[caPara.LO_cpLimCa]
	mov	dx,[caPara.HI_cpLimCa]
	mov	[OFF_cp],ax
	mov	[SEG_cp],dx
	jc	GCFCLDP06
	mov	bx,[pcpFirst]
	mov	[bx],ax
	mov	[bx+2],dx

;	 }
;    while (cp < cpMac && cpFrom == CpFormatFrom (ww, doc, cp));
	;LN_CpMacDoc returns CpMacDoc(si) in dx:ax.
	;Only ax and dx are altered.
GCFCLDP06:
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	cmp	ax,[OFF_cpMacDocVar]
	mov	bx,dx
	sbb	bx,[SEG_cpMacDocVar]
	jge	GCFCLDP07
	push	[si.wwWdc]
	push	[si.docWdc]
	push	dx
	push	ax
ifdef DEBUG
	cCall	S_CpFormatFrom,<>
else ;not DEBUG
	push	cs
	call	near ptr N_CpFormatFrom
endif ;DEBUG
	sub	ax,[OFF_cpFrom]
	sbb	dx,[SEG_cpFrom]
	or	ax,dx
	je	Ltemp021

GCFCLDP07:
;    *pcpLim = cp;
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
GCFCLDP08:
	mov	si,[pcpLim]
	mov	[si],ax
	mov	[si+2],dx

;}
cEnd

	;Make sure (ax && di) == (ax & di);
ifdef DEBUG
GCFCLDP09:
	or	ax,ax
	je	GCFCLDP10
	or	di,di
	je	GCFCLDP10
	cmp	ax,di
	je	GCFCLDP10
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFieldcrn
	mov	bx,1032
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
GCFCLDP10:
	ret
endif ;DEBUG


;-------------------------------------------------------------------------
;	CpFormatFrom (ww, doc, cp)
;-------------------------------------------------------------------------
;/* C P  F O R M A T  F R O M */
;/*  This function returns the first cp at or before cp from which
;    formatLine's can begin.
;*/
;NATIVE CP CpFormatFrom (ww, doc, cp)
;int ww;
;int doc;
;CP cp;
;{
;    GRPFVISI grpfvisi;
;    CP cpFirst;
;    int pap[cwPAPBaseScan];
;    int tAbs, fTable;

; %%Function:N_CpFormatFrom %%Owner:BRADV
cProc	N_CpFormatFrom,<PUBLIC,FAR>,<si,di>
	ParmW	ww
	OFFBP_ww		=   -2
	ParmW	doc
	OFFBP_doc		=   -4
	ParmD	cp
	OFFBP_cp		=   -8

	LocalW	tAbs
	LocalW	fTable
	LocalW	grpfvisi
	LocalW	fOutline
	LocalV	pap, cbPAPBaseScan

cBegin

	lea	si,[doc]	;register pWDC
	errnz	<OFFBP_ww - OFFBP_doc - (wwWDC)>
	errnz	<OFFBP_doc - OFFBP_doc - (docWDC)>
	errnz	<OFFBP_cp - OFFBP_doc - (cpWDC)>

;   grpfvisi = PwwdWw(ww)->grpfvisi;
	mov	bx,[si.wwWdc]
	;Takes ww in bx, result in bx.	Only bx is altered.
	call	LN_PwwdWw
	mov	al,[bx.fOutlineWwd]
	mov	bptr [fOutline],al
	mov	ax,[bx.grpfvisiWwd]
	mov	[grpfvisi],ax

;   /* case of a table nested within a field */
;   if ((fTable = FInTableDocCp(doc, cp)) && vcpFirstTablePara != caPara.cpFirst &&
;	CpVisibleCpField (doc, cp, ww/*fvcScreen*/, fFalse /* fIns */) == cp)
;	return vcpFirstTablePara;
	;LN_FInTableDocCp takes pWdc in si and performs
	;FInTableDocCp(pWdc->doc, pWdc->cp)
	;ax, bx, cx, dx are altered.
	call	LN_FInTableDocCp
	mov	[fTable],ax
	or	ax,ax
	je	CFF02
	mov	ax,wlo [vcpFirstTablePara]
	mov	dx,whi [vcpFirstTablePara]
	cmp	ax,[caPara.LO_cpFirstCa]
	jne	CFF01
	cmp	dx,[caPara.HI_cpFirstCa]
	je	CFF02
CFF01:
	;LN_CpVisibleCpField takes pWdc in si and performs
	;dx:ax = CpVisibleCpField (pWdc->doc, pWdc->cp, pWdc->ww, fFalse);
	;ax, bx, cx, dx are altered.
	push	dx
	push	ax	;save vcpFirstTablePara
	call	LN_CpVisibleCpField
	cmp	ax,[OFF_cp]
	jne	CFF015
	cmp	dx,[SEG_cp]
CFF015:
	pop	ax
	pop	dx	;restore vcpFirstTablePara
	je	Ltemp022

;    for (tAbs = tNeg; ; )
;	 {
CFF02:
	mov	[tAbs],tNeg
CFF03:
	jmp	short CFF037

;	 while ((cp = CpVisibleBackCpField(doc, caPara.cpFirst, ww)) != caPara.cpFirst)
;		 CachePara (doc, cp);
CFF033:
	;LN_CachePara takes pWdc in si and performs
	;CachePara(pWdc->doc, pWdc->cp)
	;ax, bx, cx, dx are altered.
	call	LN_CachePara
CFF037:
	push	[si.docWdc]
	push	[caPara.HI_cpFirstCa]
	push	[caPara.LO_cpFirstCa]
	push	[si.wwWdc]
ifdef DEBUG
	cCall	S_CpVisibleBackCpField,<>
else ;not DEBUG
	push	cs
	call	near ptr N_CpVisibleBackCpField
endif ;DEBUG
	mov	[OFF_cp],ax
	mov	[SEG_cp],dx
	cmp	ax,[caPara.LO_cpFirstCa]
	jne	CFF033
	cmp	dx,[caPara.HI_cpFirstCa]
	jne	CFF033

;	 if (cp == cp0 || PwwdWw(ww)->fOutline) /* don't care for hidden eop in outline */
;	     break;
	test	bptr [fOutline],maskFOutlineWwd
	jne	CFF06
	or	ax,dx
Ltemp022:
	je	CFF06

;	if (tAbs == tNeg && (tAbs = FAbsPap(doc, &vpapFetch)))
;		blt(&vpapFetch, pap, cwPAPBaseScan);
	cmp	[tAbs],tNeg
	jne	CFF04
	;LN_FAbsAndBlt takes pWdc in si, &pap in di and performs
	;if (ax = FAbsPap(pWdc->doc, &vpapFetch))
	;    blt(&vpapFetch, pap, cwPAPBaseScan);
	;ax, bx, cx, dx, di are altered.
	lea	di,[pap]
	call	LN_FAbsAndBlt
	mov	[tAbs],ax
CFF04:

;	 CachePara (doc, cp-1);
;	 /* make sure the preceeding EOP is not vanished */
;	 FetchCp (doc, cp-1, fcmProps + fcmChars);
	mov	ax,-1
	cwd
	add	ax,[OFF_cp]
	adc	dx,[SEG_cp]
	mov	bx,fcmProps + fcmChars
	push	dx
	push	ax	;save cp-1
	cCall	FetchCpAndPara,<[si.docWdc],dx,ax,bx>
	pop	ax
	pop	dx	;restore cp-1

;	 if ((!vchpFetch.fVanish && !vchpFetch.fFldVanish) ||
;		 ((grpfvisi.fSeeHidden || grpfvisi.fvisiShowAll || *vhpchFetch == chTable) &&
;		 !vchpFetch.fSysVanish))
;	     break;
	errnz	<(fVanishChp) - (fFldVanishChp)>
	test	[vchpFetch.fVanishChp],maskfVanishChp + maskfFldVanishChp
	je	CFF05
	test	[vchpFetch.fSysVanishChp],maskfSysVanishChp
	jne	CFF03
	test	[grpfvisi],maskfSeeHiddenGrpfvisi+maskfvisiShowAllGrpfvisi
Ltemp020:
	jne	CFF05
	;LN_ReloadSbFetch takes the sb from vhpchFetch and sets es to the
	;corresponding value
	;Only es and bx are altered.
	call	LN_ReloadSbFetch
	mov	bx,wlo [vhpchFetch]
	cmp	bptr es:[bx],chTable
	je	CFF05

;	if (fTable && !FInTableVPapFetch(doc, cp-1) ||
;		tAbs != FAbsPap(doc, &vpapFetch) ||
;		tAbs && !FMatchAbs(doc, &vpapFetch, pap))
;		{
;		break;
;		}
;#define FInTableVPapFetch(doc, cp)  (vpapFetch.fInTable && FInTableDocCp(doc, cp))
	cmp	[fTable],fFalse
	je	CFF045
	cmp	[vpapFetch.fInTablePap],fFalse
	je	CFF05
	;LN_FInTableDocDxAx takes pWdc in si and performs
	;FInTableDocCp(pWdc->doc, dx:ax)
	;ax, bx, cx, dx are altered.
	call	LN_FInTableDocDxAx
	or	ax,ax
	je	CFF05
CFF045:
	;LN_FCompareAbs takes fAbs in cx, pWdc in si, &pap in di and performs
	;if (fAbs != FAbsPap(pWdc->doc, &vpapFetch) ||
	;   fAbs && !FMatchAbs(pWdc->doc, &vpapFetch, pap))
	;ax, bx, cx, dx, di are altered.
	mov	cx,[tAbs]
	lea	di,[pap]
	call	LN_FCompareAbs
	je	Ltemp023

;	 }
;    return cp;
CFF05:
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
CFF06:

;}
cEnd

Ltemp023:
	jmp	CFF03

	;LN_CachePara takes pWdc in si and performs
	;CachePara(pWdc->doc, pWdc->cp)
	;ax, bx, cx, dx are altered.
LN_CachePara:
	push	[si.docWdc]
	push	[si.HI_cpWdc]
	push	[si.LO_cpWdc]
ifdef DEBUG
	cCall	S_CachePara,<>
else ;not DEBUG
	cCall	N_CachePara,<>
endif ;DEBUG
	ret

	;LN_FAbsAndBlt takes pWdc in si, &pap in di and performs
	;if (ax = FAbsPap(pWdc->doc, &vpapFetch))
	;    blt(&vpapFetch, pap, cwPAPBaseScan);
	;ax, bx, cx, dx, di are altered.
LN_FAbsAndBlt:
	push	si
	push	[si.docWDC]
	mov	si,dataoffset [vpapFetch]
	push	si
	cCall	FAbsPap,<>
	or	ax,ax
	je	FAAB01
	push	ds
	pop	es
	errnz	<cbPapBaseScan AND 1>
	mov	cx,cbPapBaseScan SHR 1
	rep	movsw
FAAB01:
	pop	si
	ret

	;LN_FInTableDocCp takes pWdc in si and performs
	;FInTableDocCp(pWdc->doc, pWdc->cp)
	;ax, bx, cx, dx are altered.
LN_FInTableDocCp:
	mov	dx,[si.HI_cpWdc]
	mov	ax,[si.LO_cpWdc]
	;LN_FInTableDocDxAx takes pWdc in si and performs
	;FInTableDocCp(pWdc->doc, dx:ax)
	;ax, bx, cx, dx are altered.
LN_FInTableDocDxAx:
	push	[si.docWdc]
	push	dx
	push	ax
ifdef DEBUG
	cCall	S_FInTableDocCp,<>
else ;not DEBUG
	cCall	N_FInTableDocCp,<>
endif ;DEBUG
	ret

	;LN_FCompareAbs takes fAbs in cx, pWdc in si, &pap in di and performs
	;if (fAbs != FAbsPap(pWdc->doc, &vpapFetch) ||
	;   fAbs && !FMatchAbs(pWdc->doc, &vpapFetch, pap))
	;ax, bx, cx, dx, di are altered.
LN_FCompareAbs:
	push	cx	;save fAbs
	push	[si.docWDC]
	mov	bx,dataoffset [vpapFetch]
	push	bx
	cCall	FAbsPap,<>
	pop	cx	;restore fAbs
	cmp	ax,cx
	jne	FMA02
	push	cx	;save fAbs
	mov	bx,dataoffset [vpapFetch]
	cCall	FMatchAbs,<[si.docWDC], bx, di>
	pop	cx	;restore fAbs
	or	ax,ax
	je	FMA01
	xor	cx,cx
FMA01:
	or	cx,cx
FMA02:
	ret

	;LN_CpVisibleCpField takes pWdc in si and performs
	;dx:ax = CpVisibleCpField (pWdc->doc, pWdc->cp, pWdc->ww, fFalse);
	;ax, bx, cx, dx are altered.
LN_CpVisibleCpField:
	errnz	<fFalse>
	xor	cx,cx
	push	[si.docWdc]
	push	[si.HI_cpWdc]
	push	[si.LO_cpWdc]
	push	[si.wwWdc]
	push	cx
ifdef DEBUG
	cCall	S_CpVisibleCpField,<>
else ;not DEBUG
	push	cs
	call	near ptr N_CpVisibleCpField
endif ;DEBUG
	ret

;-------------------------------------------------------------------------
;	CpVisibleBackCpField(doc, cp, fvc)
;-------------------------------------------------------------------------
;/* C P  V I S I B L E	B A C K  C P  F I E L D */
;CP CpVisibleBackCpField(doc, cp, fvc)
;int doc;
;CP cp;
;int fvc;
;{
;   int ifld;
;   struct FLCD flcd;

; %%Function:N_CpVisibleBackCpField %%Owner:BRADV
cProc	N_CpVisibleBackCpField,<PUBLIC,FAR>,<si,di>
	ParmW	doc
	ParmD	cp
	ParmW	fvc

	LocalV	flcd,cbFlcdMin

cBegin

;   while (cp > cp0 && CpVisibleCpField (doc, cp, fvc, fTrue) != cp)
;	/*  cp is NOT visible by field rules, go back further */
;	{
	mov	si,[OFF_cp]
	mov	di,[SEG_cp]
	jmp	short CVBCF02
CVBCF01:
	sub	si,1
	sbb	di,0
CVBCF02:
	mov	ax,si
	or	ax,di
	je	CVBCF05
	mov	ax,fTrue
	push	[doc]
	push	di
	push	si
	push	[fvc]
	push	ax
ifdef DEBUG
	cCall	S_CpVisibleCpField,<>
else ;not DEBUG
	push	cs
	call	near ptr N_CpVisibleCpField
endif ;DEBUG
	cmp	ax,si
	jne	CVBCF03
	cmp	dx,di
	je	CVBCF05
CVBCF03:

;	ifld = IfldFromDocCp(doc, cp, fFalse);
	errnz	<fFalse>
	xor	ax,ax
ifdef DEBUG
	cCall	S_IfldFromDocCp,<[doc],di,si,ax>
else ;not DEBUG
	cCall	N_IfldFromDocCp,<[doc],di,si,ax>
endif ;DEBUG

;	Assert(ifld != ifldNil);/* if ifldNil, field rules don't apply */
ifdef DEBUG
	;Do this assert with a call so as not to mess up short jumps.
	call	CVBCF07
endif ;DEBUG

;	GetIfldFlcd(doc, ifld, &flcd);
	lea	bx,[flcd]
	push	[doc]
	push	ax
	push	bx
ifdef DEBUG
	cCall	S_GetIfldFlcd,<>
else ;not DEBUG
	cCall	N_GetIfldFlcd,<>
endif ;DEBUG

;	if (flcd.cpFirst == cp ||
;		flcd.cpFirst + flcd.dcpInst + flcd.dcpResult - 1 == cp)
;	    /* cp is cpFirst or cpLast of field */
;	    cp--;
	mov	ax,[flcd.LO_cpFirstFlcd]
	mov	dx,[flcd.HI_cpFirstFlcd]
	cmp	ax,si
	jne	CVBCF04
	cmp	dx,di
	je	CVBCF01
CVBCF04:
	push	dx
	push	ax
	add	ax,[flcd.LO_dcpInstFlcd]
	adc	dx,[flcd.HI_dcpInstFlcd]
	mov	bx,ax
	mov	cx,dx
	add	ax,[flcd.LO_dcpResultFlcd]
	adc	dx,[flcd.HI_dcpResultFlcd]
	sub	ax,si
	sbb	dx,di
	dec	ax
	or	ax,dx
	pop	ax
	pop	dx
	je	CVBCF01

;	else if (flcd.cpFirst + flcd.dcpInst > cp)
;	    /* cp is in instructions */
;	    cp = flcd.cpFirst;
	xchg	ax,si
	xchg	dx,di
	sub	ax,bx
	sbb	dx,cx
	jl	CVBCF02

;	else
;	    /* cp is in results */
;	    cp = flcd.cpFirst + flcd.dcpInst - 1;
;	}
	mov	si,bx
	mov	di,cx
	jmp	short CVBCF01

;   Assert(cp >= cp0);
;   return cp;
CVBCF05:
ifdef DEBUG
	cmp	si,0
	push	di
	sbb	di,0
	pop	di
	jge	CVBCF06
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFieldcrn
	mov	bx,1012
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
CVBCF06:
endif ;DEBUG
	xchg	ax,si
	mov	dx,di

;}
cEnd

;	Assert(ifld != ifldNil);/* if ifldNil, field rules don't apply */
ifdef DEBUG
CVBCF07:
	cmp	ax,ifldNil
	jne	CVBCF08
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFieldcrn
	mov	bx,1013
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
CVBCF08:
	ret
endif ;DEBUG


;-------------------------------------------------------------------------
;	CpVisibleCpField ( doc, cp, fvc, fIns)
;-------------------------------------------------------------------------

; /* %%Function:C_CpVisibleCpField %%Owner:peterj */
; HANDNATIVE CP C_CpVisibleCpField(doc, cp, fvc, fIns)
; int doc;
; CP cp;
; int fvc;
; BOOL fIns;
; {
; 	struct PLC **hplcfld = PdodDoc (doc)->hplcfld;
; 	CP cp2;
; 	CP Cp1VisibleCpField();
; %%Function:N_CpVisibleCpField %%Owner:BRADV
cProc	N_CpVisibleCpField,<PUBLIC,FAR>,<si,di>
	ParmW	doc
	ParmD	cp
	ParmW	fvc
	ParmW	fIns

	LocalW	hplcfld
	; cp2 is a register variable

;	the following are local variables used by LN_Cp1VisibleCpField
	LocalW	ifldBegin
	LocalW	ifldLim
	LocalD	cpNextVisi
	LocalW	ifldBeginNext
	LocalW	ifldLimM1Next
	LocalV	flcd,cbFlcdMin
ifdef DEBUG
	LocalW	ifldMac
endif ;DEBUG
;	cpSeparator and cpLast are local variables used by
;	LN_FCpVisibleInPflcd
	LocalD	cpSeparator
	LocalD	cpLast
cBegin
;PAUSE
;    struct PLC **hplcfld = PdodDoc (doc)->hplcfld;
	mov	bx,[doc]
	;Takes doc in bx, result in bx.  Only bx is altered.
	call	LN_PdodDoc
	mov	ax,[bx.hplcfldDod]
	mov	[hplcfld],ax

;    if (hplcfld == hNil)
;        return cp;
	errnz	<hNil>
	or	ax,ax
	jz	CVCF_20

;PAUSE
;    if (!PdodDoc(doc)->fFldNestedValid)
;	 CorrectFNestedBits(doc);
	test	[bx.fFldNestedValidDod],maskFFldNestedValidDod
	jne	CVCF001
;PAUSE
	;LN_CorrectFNestedBits takes pdod in bx.
	;ax, bx, cx, dx, si, di are altered.
	call	LN_CorrectFNestedBits
CVCF001:

ifdef DEBUG
;    Assert(PdodDoc(doc)->fFldNestedValid);
;    CheckFNestedBits(doc);
	push	ax
	push	bx
	push	cx
	push	dx
	mov	bx,[doc]
	;Takes doc in bx, result in bx.  Only bx is altered.
	call	LN_PdodDoc
	test	[bx.fFldNestedValidDod],maskFFldNestedValidDod
	jne	CVCF_10
	mov	ax,midFieldcrn
	mov	bx,1014
	cCall	AssertProcForNative,<ax,bx>
CVCF_10:
	cCall	CheckFNestedBits,<[doc]>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;PAUSE
; 	while((cp2 = Cp1VisibleCpField(hplcfld, cp, fvc, fIns)) != cp)
;		cp = cp2;
	stc
CVCF_20:
	mov	dx,[SEG_cp]
	mov	ax,[OFF_cp]
	jnc	CVCF_30
CVCFLoop:
	push	dx
	push	ax
	mov	[OFF_cp],ax
	mov	[SEG_cp],dx	;update cp for sake of LN_FCpVisibleInPflcd
;	this is a near call in which LN_Cp1VisibleCpField uses its caller's
;	frame. note that this call _may_have_ effectively done the assignment
; 	cp = cp2 from the C code
	call	LN_Cp1VisibleCpField
	pop	cx
	pop	bx
	sub	cx,ax
	sbb	bx,dx
	or	cx,bx
	jne	CVCFLoop

CVCF_30:
;PAUSE
cEnd


;/* C P  1 V I S I B L E  C P  F I E L D */
;/*  Return the first visible cp at or after cp.  Visibility is by field
;    nesting only (not by character properties) and according to fvc.
;    If fIns then returns an insertion point that should be visible.  If !fIns
;    then a character that should be visible.
;*/
;
;native CP Cp1VisibleCpField (hplcfld, cp, fvc, fIns)
;struct PLC **hplcfld;
;CP cp;
;int fvc;
;BOOL fIns;
;{
;
;    int ifldBegin, ifldLim;
;    CP cpNextVisi;
;    CP cpEnd;
;    int cChBegin;
;    int ifld;
;    int ifldBeginNext;
;    int ifldLimM1Next;
;    struct FLCD flcd;
;    struct FLD fld;
;#ifdef DEBUG 
;    int ifldMac;
;#endif /* DEBUG */
LN_Cp1VisibleCpField:

;	cChBegin is kept in cx
;	fld is kept in ax
;	ifld is kept in bx
;	cpEnd is kept in dx:ax

;cBegin

ifdef DEBUG
;    Debug (ifldMac = IMacPlc(hplcfld));
	push	ax
	push	bx
	push	cx
	push	dx
	cCall	IMacPlc,<[hplcfld]>
	mov	[ifldMac],ax
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;    ifldLim = IInPlcCheck (hplcfld, cp);
	cCall	IInPlcCheck,<[hplcfld], [SEG_cp], [OFF_cp]>

;    ifldBegin = 0;
;    ifldLim++;
	xor	cx,cx
	inc	ax
	mov	[ifldLim],ax

;    if (ifldLim > 0)
;	 {
;	 for (ifldBegin = ifldLim - 1; ifldBegin > 0; ifldBegin--)
;	     {
;	QcpQfooPlcfoo takes hplcfoo in bx, ifoo in si, it returns pplcfoo
;	in bx, cbfoo in cx, qcp in es:di, qfoo in es:si.
;	if DEBUG it returns hpcp in dx:di, hpfoo in dx:si.
;	Changes ax, bx, cx, dx, si, di.
	je	CVCF008
	dec	ax
	mov	bx,[hplcfld]
	xchg	ax,si
	push	si
ifdef DEBUG
	mov	di,1	;O.K. to pass ifldMac
endif ;DEBUG
	cCall	N_QcpQfooPlcfoo,<>
ifdef DEBUG
	;Check that es == mpsbps[dx];
	call	CVCF20
endif ;DEBUG
	pop	cx
	jcxz	CVCF008
	;Assembler note: This is a call to CpPlc done below in the C version.
	;We only need to do it here however because we know at this point
	;that CpPlc(hplcfld, ifldBegin) <= cp, and ifld < ifldBegin ==>
	;CpPlc(hplcfld, ifld) < cp.  From here to CVCF005 we are performing
	;"if (CpPlc(hpldfld, ifldBegin) >= cp) ifldBegin--;"
	;***Begin in-line CpPlc
	mov	ax,es:[di]
	mov	dx,es:[di+2]
	cmp	cx,[bx.icpAdjustPlc]
	jl	CVCF003
	add	ax,[bx.LO_dcpAdjustPlc]
	adc	dx,[bx.HI_dcpAdjustPlc]
CVCF003:
	;***End in-line CpPlc
        sub	ax,[OFF_cp]
	sbb	dx,[SEG_cp]
	jl	CVCF004
	dec	si
	dec	si
	dec	cx
CVCF004:
	std
CVCF005:

;	     GetPlc( hplcfld, ifldBegin, &fld );
;	     if (fld.ch == chFieldEnd && !fld.fNested &&
;                CpPlc(hplcfld, ifldBegin) < cp)
;		 {
	;Assembler note: "CpPlc(hplcfld, ifldBegin) < cp" is done above
	;in the assembler version.
	jcxz	CVCF007
	errnz	<cbFldMin - 2>
	lods	wptr es:[si]
	errnz	<(chFld) - 0>
	errnz	<(fNestedFld) - 1>
	and	ax,maskChFld + (maskFNestedFld SHL 8)
	cmp	ax,chFieldEnd
	loopnz	CVCF005
	jnz	CVCF007
	inc	cx	;to counter unwanted ifldBegin-- in loopnz instruction

;		 ifldBegin++;
CVCF006:
	inc	cx

;		 break;
;		 }
;	     }
;	 }
CVCF007:
	cld
CVCF008:

;    while ( ifldBegin < ifldLim )
;	 {
	mov	[ifldBegin],cx
CVCF01:

;	QcpQfooPlcfoo takes hplcfoo in bx, ifoo in si, it returns pplcfoo
;	in bx, cbfoo in cx, qcp in es:di, qfoo in es:si.
;	if DEBUG it returns hpcp in dx:di, hpfoo in dx:si.
;	Changes ax, bx, cx, dx, si, di.
	mov	bx,[hplcfld]
	mov	si,[ifldBegin]
	push	si
ifdef DEBUG
	mov	di,2	;O.K. to pass ifldMac, 07FFFh
endif ;DEBUG
	cCall	N_QcpQfooPlcfoo,<>
ifdef DEBUG
	;Check that es == mpsbps[dx];
	call	CVCF20
endif ;DEBUG
	pop	cx

CVCF02:
	cmp	cx,[ifldLim]
	jge	Ltemp004

;	ifldBeginNext = 0x7FFF;
	mov	[ifldBeginNext],07FFFh

;	ifldLimM1Next = 0x7FFE;
	mov	[ifldLimM1Next],07FFEh

;	GetPlc( hplcfld, ifldBegin, &fld );
	errnz	<cbFldMin - 2>
	lods	wptr es:[si]

;	if (fld.ch == chFieldSeparate)
;		{
;		ifldBegin++;
;		continue;
;		}
	errnz	<(chFld) - 0>
	and	al,maskChFld
	cmp	al,chFieldSeparate
	je	CVCF006

;        Assert (fld.ch == chFieldBegin);
ifdef DEBUG
;	/* Assert (fld.ch == chFieldBegin) with a call so as not to mess up
;	short jumps */
	call	CVCF12
endif ;/* DEBUG */

;	cChBegin = 0;
;	cChBegin now kept in bx
	xor	bx,bx

;	ifld = ifldBegin;
;	ifld now kept in cx
;	mov	cx,[ifldBegin]

CVCF03:
;	for (;;)
;        	{
;		GetPlc( hplcfld, ++ifld, &fld );
	inc	cx
	errnz	<cbFldMin - 2>
	lods	wptr es:[si]
	add	di,4

;        	Assert (ifld < ifldMac);
ifdef DEBUG
;	/* Assert (ifld < ifldMac) with a call so as not to mess up
;	short jumps */
	call	CVCF14
endif ;/* DEBUG */

;	        switch (fld.ch)
;        		{

;		case chFieldBegin:
	errnz	<(chFld) - 0>
	and	al,maskChFld
	cmp	al,chFieldBegin
	jne	CVCF04

;			cChBegin++;
	inc	bx

;			if (ifldBeginNext == 0x7FFF)
;			    ifldBeginNext = ifld;
	cmp	[ifldBeginNext],07FFFh
;	break and continue in loop
	jne	CVCF03
	mov	[ifldBeginNext],cx

;                	break;
;	break and continue in loop
	jmp	short CVCF03

CVCF04:
;	        case chFieldEnd:
	errnz	<(chFld) - 0>
	cmp	al,chFieldEnd
ifdef DEBUG
	je	Ltemp005
	jmp	CVCF10
Ltemp005:
else
	jne	CVCF03
endif ;DEBUG

;			if (cChBegin-- == 0)
;                		{
;				 cpEnd = CpPlc( hplcfld, ifld);
;	                    	goto LEndFound;
	dec	bx
	jl	LEndFound

;               			}
;			ifldLimM1Next = ifld;
	mov	[ifldLimM1Next],cx

;	break out of switch and continue in for loop
;        	        break;
	jmp	short CVCF03

Ltemp004:
	jmp	short CVCF08

;#ifdef DEBUG
;	        default:
;        	        Assert (fld.ch == chFieldSeparate);
;	see case chFieldEnd for a jump to the code which performs this assert
;#endif /* DEBUG */
;		        }
;		}

LEndFound:
;	here we are performing cpEnd = CpPlc( hplcfld, ifld) in line.
	mov	ax,es:[di]
	mov	dx,es:[di+2]
	push	di
	mov	di,[hplcfld]
	mov	di,[di]
	cmp	cx,[di.icpAdjustPlc]
	jl	CVCF045
	add	ax,[di.LO_dcpAdjustPlc]
	adc	dx,[di.HI_dcpAdjustPlc]
CVCF045:
	pop	di

;	 if (cpEnd + 1 < cp+!fIns)
;            {
	cmp	[fIns],fFalse
	je	CVCF05
	add	ax,00001h
	adc	dx,00000h
CVCF05:
	sub	ax,[OFF_cp]
	sbb	dx,[SEG_cp]
	jge	CVCF07

;            Assert (ifld > ifldBegin);
ifdef DEBUG
;	/* Assert (ifld > ifldBegin) with a call so as not to mess up
;	short jumps */
	call	CVCF16
endif ;/* DEBUG */

CVCF06:
;            ifldBegin = ifld + 1;
	inc	cx
	mov	[ifldBegin],cx
	add	di,4

;            continue;
;	     }
	jmp	CVCF02

CVCF07:
;	/* Limit search for lower level if this level is not low enough */
;	ifldLim = min(ifldLimM1Next + 1, ifldLim);
	mov	cx,[ifldLimM1Next]
	inc	cx
	cmp	cx,[ifldLim]
	jge	CVCF075
	mov	[ifldLim],cx
CVCF075:

;        FillIfldFlcd (hplcfld, ifldBegin, &flcd);

	lea	ax,[flcd]
	push	[hplcfld]
	push	[ifldBegin]
	push	ax
ifdef DEBUG
	cCall	S_FillIfldFlcd,<>
else ;not DEBUG
	cCall	N_FillIfldFlcd,<>
endif ;DEBUG

;        Assert (flcd.cpFirst <= cp);
ifdef DEBUG
;	/* Assert (flcd.cpFirst <= cp) with a call so as not to mess up
;	short jumps */
	call	CVCF18
endif ;/* DEBUG */

;	 if (!FCpVisibleInPflcd (cp, &flcd, &cpNextVisi, fvc, fIns))
;	 this is a near call in which LN_FCpVisibleInPflcd uses its caller's
;	 frame.
	cCall	LN_FCpVisibleInPflcd,<>
	or	ax,ax
	mov	ax,[ifldBeginNext]
	mov	[ifldBegin],ax
	jne	Ltemp009

;            return cpNextVisi;
	mov	ax,[OFF_cpNextVisi]
	mov	dx,[SEG_cpNextVisi]
	jmp	short CVCF09

;	the following line is done above in the assembler version
;	 ifldBegin = ifldBeginNext;

;	this line is also done above in the assembler version
;        }

CVCF08:
;    return cp;

	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]

CVCF09:
;PAUSE
	ret
;}
;cEnd

Ltemp009:
	jmp	CVCF01

; End of CpVisibleCpField


ifdef DEBUG
;	Assert (fld.ch == chFieldSeparate);
CVCF10:
	errnz	<(chFld) - 0>
	cmp	al,chFieldSeparate
	je	CVCF11
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,midFieldcrn
	mov	bx,1015
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
CVCF11:
	jmp	CVCF03
endif ;/* DEBUG */

ifdef DEBUG
;	Assert (fld.ch == chFieldBegin);
CVCF12:
	push	ax
	push	bx
	push	cx
	push	dx
	errnz	<(chFld) - 0>
	and	al,maskChFld
	cmp	al,chFieldBegin
	je	CVCF13

	mov	ax,midFieldcrn
	mov	bx,1016
	cCall	AssertProcForNative,<ax,bx>
CVCF13:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

ifdef DEBUG
;	Assert (ifld < ifldMac);
CVCF14:
	cmp	cx,[ifldMac]
	jl	CVCF15
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,midFieldcrn
	mov	bx,1017
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
CVCF15:
	ret
endif ;/* DEBUG */


ifdef DEBUG
;            Assert (ifld > ifldBegin);
CVCF16:
	cmp	cx,[ifldBegin]
	jg	CVCF17
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,midFieldcrn
	mov	bx,1018
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
CVCF17:
	ret
endif ;/* DEBUG */

ifdef DEBUG
;        Assert (flcd.cpFirst <= cp);
CVCF18:
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	sub	ax,[flcd.LO_cpFirstFlcd]
	sbb	dx,[flcd.HI_cpFirstFlcd]
	jge	CVCF19
	mov	ax,midFieldcrn
	mov	bx,1019
	cCall	AssertProcForNative,<ax,bx>
CVCF19:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

ifdef DEBUG
CVCF20:
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
	jc	CVCF21
	;Assembler note: There is no way we should have to call ReloadSb here.
;	reload sb trashes ax, cx, and dx
;	cCall	ReloadSb,<>
	mov	ax,midFieldcrn
	mov	bx,1030
	cCall	AssertProcForNative,<ax,bx>
CVCF21:
	pop	ax	;restore es from QcpQfooPlcfoo
	mov	bx,es	;compare with es rederived from the SB of QcpQfooPlcfoo
	cmp	ax,bx
	je	CVCF22
	mov	ax,midFieldcrn
	mov	bx,1031
	cCall	AssertProcForNative,<ax,bx>
CVCF22:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */



;-------------------------------------------------------------------------
;	FCpVisibleInPflcd ( cp, pflcd, pcpNextVisi, fvc, fIns )
;-------------------------------------------------------------------------
;/* F  C P  V I S I B L E  I N  P F L C D */
;/*  Returns true if cp is visible within pflcd according to fvc.  If false
;    then *pcpNextVisi is the first visible cp after cp.
;*/
;
;native FCpVisibleInPflcd (cp, pflcd, pcpNextVisi, fvc, fIns)
;CP cp;
;struct FLCD * pflcd;
;CP * pcpNextVisi;
;int fvc;
;BOOL fIns;
;
;{
;
;    CP cpSeparator = pflcd->cpFirst + pflcd->dcpInst - 1 + fIns;
;    CP cpLast = cpSeparator + pflcd->dcpResult;

;	 this is a near routine which uses its caller's frame.
;cProc	 LN_FCpVisibleInPflcd,<PUBLIC,NEAR>,<>
LN_FCpVisibleInPflcd LABEL NEAR
;	ParmD	cp
;	ParmW	pflcd
;	ParmW	pcpNextVisi
;	ParmW	fvc
;	ParmW	fIns

;	LocalD	cpSeparator
;	LocalD	cpLast

;cBegin
	mov	ax,[flcd.LO_cpFirstFlcd]
	mov	dx,[flcd.HI_cpFirstFlcd]
	add	ax,[flcd.LO_dcpInstFlcd]
	adc	dx,[flcd.HI_dcpInstFlcd]
	sub	ax,00001h
	sbb	dx,00000h
ifdef DEBUG
;	Need to make this assert because I am doing an unsigned extend
;        Assert (fIns & 0xFFFE == 0);
	test	fIns,0FFFEh
	je	FCVIP01
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,midFieldcrn
	mov	bx,1020
	cCall	AssertProcForNative,<ax,bx>

	pop	dx
	pop	cx
	pop	bx
	pop	ax
FCVIP01:
endif ;/* DEBUG */
	add	ax,[fIns]
	adc	dx,00000h
	mov	[OFF_cpSeparator],ax
	mov	[SEG_cpSeparator],dx
	add	ax,[flcd.LO_dcpResultFlcd]
	adc	dx,[flcd.HI_dcpResultFlcd]
	mov	[OFF_cpLast],ax
	mov	[SEG_cpLast],dx

;    Assert (fIns == 1 || fIns == 0);
;	done above

;    switch (WPflcdDispFvc (pflcd, fvc))
;	Do WPflcdDispFvc in line since it is only called from here.
;	Because of the switch on the result, code from FCpVisibleInPlfcd
;	and WPflcdDispFvc will be intermingled from here on.

;	the following is from WPflcdDispFvc
;    if (FShowResultPflcdFvc (pflcd, fvc))
	lea	ax,[flcd]
	push	ax
	push	[fvc]
ifdef DEBUG
	cCall	S_FShowResultPflcdFvc,<>
else ;not DEBUG
	cCall	N_FShowResultPflcdFvc,<>
endif ;DEBUG
	or	ax,ax
	jne	Ltemp006
	jmp	FCVIP09
Ltemp006:

	cCall	EfltFromFlt,<[flcd.fltFlcd]>
	mov	di,[fIns]
	mov	cx,[OFF_cpLast]
	mov	dx,[SEG_cpLast]

;	the following is from WPflcdDispFvc
;        if (pflcd->dcpResult && dnflt[pflcd->flt].fResult)
;            {
	mov	si,[flcd.LO_dcpResultFlcd]
	or	si,[flcd.HI_dcpResultFlcd]
	je	FCVIP05
	errnz	<(fResultEflt) - 0>
	test	al,maskfResultEflt
	je	FCVIP05

;	the following is from WPflcdDispFvc
;            return 1 /*result*/;
;            }

;	the following is from FCpVisibleInPlfcd
;        case 1:      /* show result: cp visi if in result, next visi may
;                                            be first of result or lim */
;            if (cp == cpLast)
	mov	ax,[OFF_cp]
	mov	bx,[SEG_cp]
	cmp	ax,cx
	jne	FCVIP02
	cmp	bx,dx
	je	FCVIP03

;                *pcpNextVisi = cpLast + 1;

FCVIP02:
;            else if (cp <= cpSeparator - fIns && 
;                    (!fIns || cp > pflcd->cpFirst))
	mov	cx,[OFF_cpSeparator]
	mov	dx,[SEG_cpSeparator]
	push	cx
	push	dx
	sub	cx,di
	sbb	dx,0
	sub	cx,ax
	sbb	dx,bx
	pop	dx
	pop	cx
	jl	FCVIP04
	or	di,di
	je	FCVIP03
	mov	di,[flcd.LO_cpFirstFlcd]
	sub	di,ax
	mov	di,[flcd.HI_cpFirstFlcd]
	sbb	di,bx
	jge	FCVIP04

FCVIP03:

;                *pcpNextVisi = cpSeparator + 1;
	add	cx,1
	adc	dx,0
	jmp	short FCVIP10

FCVIP04:
;            else  /* in result */
;                return fTrue;

	mov	ax,fTrue
	jmp	short FCVIP12

FCVIP05:
;	the following is from WPflcdDispFvc
;        else
;            return dnflt[pflcd->flt].fDisplay ? -2/*disp*/ : -1/*nothing*/;
	errnz	<(fDisplayEflt) - 0>
	test	al,maskfDisplayEflt
	je	FCVIP08

;	the following is from FCpVisibleInPlfcd
;        {
;        case -2:     /* show single character (display field) */
;            *pcpNextVisi = cpLast;

;            return (fIns && pflcd->cpFirst == cp) || cp == cpLast;
	mov	ax,fTrue
	or	di,di
	mov	di,[OFF_cp]
	mov	bx,[SEG_cp]
	je	FCVIP06
	cmp	[flcd.LO_cpFirstFlcd],di
	jne	FCVIP06
	cmp	[flcd.HI_cpFirstFlcd],bx
	je	FCVIP11
FCVIP06:
	cmp	di,cx
	jne	FCVIP10
	cmp	bx,dx
	jne	FCVIP10
	jmp	short FCVIP11

FCVIP08:

;	the following is from FCpVisibleInPlfcd
;        case -1:     /* show nothing: cp not visible unless ins pt before
;                        cpFirst, cpLim is next visi */
;            *pcpNextVisi = cpLast + 1;
	add	cx,00001h
	adc	dx,00000h

;            return (fIns && pflcd->cpFirst == cp);
	or	di,di
	je	FCVIP10
	mov	ax,OFF_cp
	cmp	ax,[flcd.LO_cpFirstFlcd]
	jne	FCVIP10
	mov	ax,SEG_cp
	cmp	ax,[flcd.HI_cpFirstFlcd]
	jne	FCVIP10
	mov	ax,fTrue
	jmp	short FCVIP11

FCVIP09:
;	the following is from WPflcdDispFvc
;    else
;        return 0 /*instructions*/;

;	the following is from FCpVisibleInPlfcd
;        case 0:      /* show instructions: cp visible if < sep || last,
;                                            next visi is last */
;            *pcpNextVisi = cpLast;
	mov	cx,[OFF_cpLast]
	mov	dx,[SEG_cpLast]

;            return (cp < cpSeparator || cp >= cpLast);
	mov	si,[OFF_cp]
	mov	di,[SEG_cp]
	mov	ax,fTrue
	mov	bx,si
	sub	bx,[OFF_cpSeparator]
	mov	bx,di
	sbb	bx,[SEG_cpSeparator]
	jl	FCVIP11
	sub	si,cx
	sbb	di,dx
	jge	FCVIP11

FCVIP10:
;            return fFalse;
	errnz	<fFalse>
	xor	ax,ax

FCVIP11:
	mov	wlo ([cpNextVisi]),cx
	mov	whi ([cpNextVisi]),dx

FCVIP12:
;        }
;
;}
;cEnd
	ret

; End of LN_FCpVisibleInPflcd




; The code in this routine has been incorporated into FCpVisibleInPlfcd
;/* W  P F L C D  D I S P  F V C */
;/*  Returns a code indicating what portion of field pflcd should be
;    displayed.  Code is:
;        -2:  display a single character for the field (indicates display
;                              field in results mode.
;        -1:  display nothing: indicates show results with either !fResult
;                              or no last result.
;        0:   display field instructions: indicates show instructions mode.
;        1:   display field result: indicates show results && fResult &&
;                              there is a last result.
;*/
;
;native WPflcdDispFvc (pflcd, fvc)
;struct FLCD * pflcd;
;int fvc;
;
;{
;    if (FShowResultPflcdFvc (pflcd, fvc))
;        if (pflcd->dcpResult && dnflt[pflcd->flt].fResult)
;            {
;            return 1 /*result*/;
;            }
;
;        else
;            return dnflt[pflcd->flt].fDisplay ? -2/*disp*/ : -1/*nothing*/;
;    else
;        return 0 /*instructions*/;
;}




;-------------------------------------------------------------------------
;	CpFromIchPcr ( ich, pcr, ccr )
;-------------------------------------------------------------------------
;/*  REVIEW: CORE FUNCTION */
;/* C P  F R O M  I C H  P C R */
;
;native CP CpFromIchPcr (ich, pcr, ccr)
;int ich;
;struct CR *pcr;
;int ccr;
;
;{

; %%Function:CpFromIchPcr %%Owner:BRADV
cProc	CpFromIchPcr,<PUBLIC,FAR>,<>
	ParmW	ich
	ParmW	pcr
	ParmW	ccr

cBegin

;    while (ich >= pcr->ccp && --ccr > 0)
	mov	bx,[pcr]
	mov	cx,[ccr]
ifdef DEBUG
;	Assert (ccr >= 0) so that loop instruction will work.
	or	cx,cx
	jge	CFIP01
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,midFieldcrn
	mov	bx,1021
	cCall	AssertProcForNative,<ax,bx>

	pop	dx
	pop	cx
	pop	bx
	pop	ax
CFIP01:
endif ;/* DEBUG */
	mov	ax,[ich]
	inc	cx	; adjust for loop instruction
	jmp	short CFIP03

CFIP02:
	mov	dx,[bx.ccpCr]
	cmp	ax,dx
	jl	CFIP04

;        ich -= (pcr++)->ccp;
	sub	ax,dx
	add	bx,cbCrMin

CFIP03:
	loop	CFIP02

CFIP04:
;    return pcr->cp + ich;
	cwd
	add	ax,[bx.LO_cpCr]
	adc	dx,[bx.HI_cpCr]

;}
cEnd

; End of CpFromIchPcr



;-------------------------------------------------------------------------
;	FVisibleCp ( ww, doc, cp )
;-------------------------------------------------------------------------
;/* F  V I S I B L E  C P */
;/*  Returns True iff cp should be displayed on the screen in ww (based
;    on view prefs)
;
;native FVisibleCp (ww, cp)
;int ww;
;CP cp;
;
;{
;    BOOL fShowInOutline = fTrue;
;    CP   cpT;
;    CP   CpFormatFrom();

; %%Function:N_FVisibleCp %%Owner:BRADV
cProc	N_FVisibleCp,<PUBLIC,FAR>,<si,di>
	ParmW	ww
	ParmW	doc
	ParmD	cp

	LocalD	cpT
	LocalV	pad,cbPadMin

cBegin


;   if (CpVisibleCpField (doc, cp, ww/*fvcScreen*/, fFalse /* fIns */) != cp)
;	return fFalse;
	mov	di,[SEG_cp]
	mov	si,[OFF_cp]
	errnz	<fFalse>
	xor	ax,ax
	push	[doc]
	push	di
	push	si
	push	[ww]
	push	ax
ifdef DEBUG
	cCall	S_CpVisibleCpField,<>
else ;not DEBUG
	push	cs
	call	near ptr N_CpVisibleCpField
endif ;DEBUG
	sub	ax,si
	sbb	dx,di
	or	ax,dx
	jne	FVC03

;   if (PwwdWw(ww)->fOutline)

	mov	bx,[ww]
	;Takes ww in bx, result in bx.	Only bx is altered.
	call	LN_PwwdWw
	push	[bx.grpfvisiWwd]    ;save wwd grpfvisi
	;put both pwwd->fOutline and pwwd->fEllip in one word for use later.
	mov	ax,fTrue    ;default fShowInOutline
	test	[bx.fOutlineWwd],maskfOutlineWwd
	je	FVC01

;	fShowInOutline = FCpVisiInOutline(ww, doc, cp, 1/*ccp*/, &cpT);
	lea	bx,[cpT]
	push	[ww]
	push	[doc]
	push	di
	push	si
	errnz	<fTrue - 1>
	push	ax
	push	bx
ifdef DEBUG
	cCall	S_FCpVisiInOutline,<>
else ;not DEBUG
	push	cs
	call	near ptr N_FCpVisiInOutline
endif ;DEBUG
FVC01:
	push	ax	;save fShowInOutline

;    CachePara (doc, cp);
ifdef DEBUG
	cCall	S_CachePara,<[doc],di,si>
else ;not DEBUG
	cCall	N_CachePara,<[doc],di,si>
endif ;DEBUG

;    FetchCp (doc, cp, fcmProps);
	mov	ax,fcmProps
ifdef DEBUG
	cCall	S_FetchCp,<[doc],di,si,ax>
else ;not DEBUG
	cCall	N_FetchCp,<[doc],di,si,ax>
endif ;DEBUG

	pop	ax	;restore fShowInOutline
	pop	cx	;restore wwd grpfvisi

;   return fShowInOutline &&
;		! ((vchpFetch.fVanish || vchpFetch.fFldVanish) &&
;	    (vchpFetch.fSysVanish ||
;	    !(PwwdWw(ww)->grpfvisi.fSeeHidden || PwwdWw(ww)->grpfvisi.fvisiShowAll) ));
	or	ax,ax
	je	FVC04
	errnz	<(fVanishChp) - (fFldVanishChp)>
	test	[vchpFetch.fVanishChp],maskfVanishChp + maskfFldVanishChp
	je	FVC02
	test	[vchpFetch.fSysVanishChp],maskfSysVanishChp
	jne	FVC03
	errnz	<(maskfSeeHiddenGrpfvisi+maskfvisiShowAllGrpfvisi) AND 0FF00h>
	test	cl,maskfSeeHiddenGrpfvisi+maskfvisiShowAllGrpfvisi
	je	FVC03
FVC02:
	db	03Dh	;turns next "xor ax,ax" into "cmp ax,immediate"
FVC03:
	xor	ax,ax
FVC04:

;}
cEnd

; End of FVisibleCp


;-------------------------------------------------------------------------
;	FCpVisiInOutline ( ww, doc, cp, ccp, pcpFirstInvisi )
;-------------------------------------------------------------------------
;/* F C P V I S I  I N	O U T L I N E  */
;/* Given a cp range within the same pad, return true if all is visible
;   WRT outline.  Also return the cpFirstInvis if part of it is not visible.
;*/
;HANDNATIVE FCpVisiInOutline(ww, doc, cp, ccp, pcpFirstInvisi)
;int ww;
;int doc;
;CP cp;
;int ccp;
;CP *pcpFirstInvisi;
;{
;    struct DOD     *pdod;
;    struct PLC **hplcpad;
;    int	     ipad;
;    CP cpLim = cp + ccp;
;    struct PAD      pad;
;    BOOL fShowInOutline;

; %%Function:N_FCpVisiInOutline %%Owner:BRADV
cProc	N_FCpVisiInOutline,<PUBLIC,FAR>,<si,di>
	ParmW	ww
	ParmW	doc
	ParmD	cp
	ParmW	ccp
	ParmW	pcpFirstInvisi

	LocalW	ipad
	LocalV	pad,cbPadMin
	LocalV	padT,cbPadMin

cBegin

;   Assert (PwwdWw(ww)->fOutline);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	bx,[ww]
	;Takes ww in bx, result in bx.	Only bx is altered.
	call	LN_PwwdWw
	test	[bx.fOutlineWwd],maskfOutlineWwd
	jne	FCVIO01

	mov	ax,midFieldcrn
	mov	bx,1022
	cCall	AssertProcForNative,<ax,bx>

FCVIO01:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;   pdod = PdodDoc(doc);
	mov	di,[doc]
	mov	bx,di
	;Takes ww in bx, result in bx.	Only bx is altered.
	call	LN_PdodDoc
	    
;   Assert(pdod->fMother);
ifdef DEBUG
	cmp	[bx.fMotherDod],fFalse
	jne	FCVIO02
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,midFieldcrn
	mov	bx,1023
	cCall	AssertProcForNative,<ax,bx>

	pop	dx
	pop	cx
	pop	bx
	pop	ax
FCVIO02:
endif ;/* DEBUG */

;   if (pdod->hplcpad == NULL || pdod->fOutlineDirty)
;	    {
;	    if (!FUpdateHplcpad(doc)) /* HM */
;		return fTrue; 
;	    }

	cmp	[bx.hplcpadDod],NULL
	je	FCVIO03
	test	[bx.fOutlineDirtyDod],maskfOutlineDirtyDod
	je	FCVIO04
FCVIO03:
	cCall	FUpdateHplcpad,<di>
	or	ax,ax
	jne	FCVIO04
	inc	ax
	jmp 	FCVO014
FCVIO04:

;   hplcpad = PdodDoc(doc)->hplcpad;
	mov	bx,di
	;Takes ww in bx, result in bx.	Only bx is altered.
	call	LN_PdodDoc
	mov	si,[bx.hplcpadDod]

;   ipad = IInPlc(hplcpad, cp);
	cCall	IInPlc,<si,[SEG_cp],[OFF_cp]>
	xchg	ax,di

;   Assert(ipad == IInPlc(hplcpad, cpLim-1));
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
;    CP cpLim = cp + ccp;
	mov	ax,[ccp]
	dec	ax
	cwd
	add	ax,[OFF_cp]
	adc	dx,[SEG_cp]
	cCall	IInPlc,<si,dx,ax>
	cmp	ax,di
	je	FCVIO05

	mov	ax,midFieldcrn
	mov	bx,1024
	cCall	AssertProcForNative,<ax,bx>

FCVIO05:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;   GetPlc(hplcpad, ipad, &pad);
	lea	bx,[pad]
	cCall	GetPlc,<si, di, bx>
	mov	cx,si

;   fShowInOutline = pad.fShow;
	mov	al,[pad.fShowPad]

;   if (!fShowInOutline)
;	{
;	*pcpFirstInvisi = cp;
;	}
	and	ax,maskFShowPad
	push	ax	;save fShowInOutline
	mov	si,[OFF_cp]
	mov	di,[SEG_cp]
	je	FCVIO12

;   else if (pad.fBody && PwwdWw(ww)->fEllip)
;	{
	test	[pad.fBodyPad],maskfBodyPad
	je	FCVIO13
	mov	bx,[ww]
	mov	cx,bx
	;Takes ww in bx, result in bx.	Only bx is altered.
	call	LN_PwwdWw
	test	[bx.fEllipWwd],maskFEllipWwd
	je	FCVIO13
	mov	dx,[doc]
	push	cx	;arg for FormatLine
	push	dx	;arg for FormatLine
	push	cx	;arg for CpFormatFrom
	push	dx	;arg for CpFormatFrom
	push	di	;arg for CpFormatFrom
	push	si	;arg for CpFormatFrom

;	CachePara(doc, cp);
	push	dx
	push	di
	push	si
ifdef DEBUG
	cCall	S_CachePara,<>
else ;not DEBUG
	cCall	N_CachePara,<>
endif ;DEBUG

;	FormatLine(ww, doc, CpFormatFrom(ww, doc, cp));
ifdef DEBUG
	cCall	S_CpFormatFrom,<>
else ;not DEBUG
	push	cs
	call	near ptr N_CpFormatFrom
endif ;DEBUG
	push	dx	;arg for FormatLine
	push	ax	;arg for FormatLine
ifdef DEBUG
	cCall	S_FormatLine,<>
else ;not DEBUG
	cCall	N_FormatLine,<>
endif ;DEBUG

;	fShowInOutline = (cpLim - 1) < vfli.cpMac;
	mov	bx,[vfli.LO_cpMacFli]
	mov	cx,[vfli.HI_cpMacFli]
;    CP cpLim = cp + ccp;
	mov	ax,[ccp]
	dec	ax
	cwd
	add	ax,si
	adc	dx,di
	sub	ax,bx
	sbb	dx,cx
	jl	FCVIO11
	pop	ax	;restore fShowInOutline - we know it is fTrue here
	xor	ax,ax
	push	ax	;save fShowInOutline
FCVIO11:

;	*pcpFirstInvisi = CpMax(cp, vfli.cpMac);
	cmp	bx,si
	push	cx
	sbb	cx,di
	pop	cx
	jl	FCVIO12
	mov	si,bx
	mov	di,cx
FCVIO12:
	mov	bx,[pcpFirstInvisi]
	mov	[bx],si
	mov	[bx+2],di
;	}

FCVIO13:
	pop	ax	;restore fShowInOutline
FCVO014:
;   return fShowInOutline;
;}
cEnd

ifdef DEBUG
FCVIO14:
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
	jc	FCVIO15
	;Assembler note: There is no way we should have to call ReloadSb here.
;	reload sb trashes ax, cx, and dx
;	cCall	ReloadSb,<>
	mov	ax,midFieldcrn
	mov	bx,1035
	cCall	AssertProcForNative,<ax,bx>
FCVIO15:
	pop	ax	;restore es from QcpQfooPlcfoo
	mov	bx,es	;compare with es rederived from the SB of QcpQfooPlcfoo
	cmp	ax,bx
	je	FCVIO16
	mov	ax,midFieldcrn
	mov	bx,1036
	cCall	AssertProcForNative,<ax,bx>
FCVIO16:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

ifdef DEBUG
;Assert(cpT > cp0); 
FCVIO17:
	push	ax
	push	bx
	push	cx
	push	dx
	or	dx,dx
	jge	FCVIO18
	mov	ax,midFieldcrn
	mov	bx,1040
	cCall	AssertProcForNative,<ax,bx>
FCVIO18:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

; End of FCpVisiInOutline


LN_PwwdWw:
	;Takes ww in bx, result in bx.	Only bx is altered.
	shl	bx,1
	mov	bx,[bx.mpwwhwwd]
ifdef DEBUG
	cmp	bx,hNil
	jne	LN_PW101
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFieldcrn
	mov	bx,1025
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LN_PW101:
endif ;/* DEBUG */
	mov	bx,[bx]
	ret


LN_PdodDoc:
	;Takes doc in bx, result in bx.  Only bx is altered.
	shl	bx,1
	mov	bx,[bx.mpdochdod]
ifdef DEBUG
	cmp	bx,hNil
	jne	LN_PD101
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFieldcrn
	mov	bx,1026
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LN_PD101:
endif ;/* DEBUG */
	mov	bx,[bx]
	ret


	;LN_CpMacDoc returns CpMacDoc(si) in dx:ax.
	;Only ax and dx are altered.
LN_CpMacDoc:
;	***Do the following code from CpMacDoc in line
;	struct DOD *pdod = PdodDoc(doc);
;	return(pdod->cpMac - 2*ccpEop);
;	***End of CpMacDoc code
	push	si
	shl	si,1
	mov	si,[si.mpdochdod]
ifdef DEBUG
	cmp	si,hNil
	jne	LN_CMD01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFieldcrn
	mov	bx,1027
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LN_CMD01:
endif ;/* DEBUG */
	mov	si,[si]
	mov	ax,-2*ccpEop
	cwd
	add	ax,[si.LO_cpMacDod]
	adc	dx,[si.HI_cpMacDod]
	pop	si
	ret


sEnd	fieldcr_PCODE
	end
