        include w2.inc
        include noxport.inc
        include consts.inc
        include structs.inc

createSeg	disp2_PCODE,disp2,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midDisp2n	equ 19		 ; module ID, for native asserts
NatPause equ 1
endif

ifdef	NatPause
PAUSE	MACRO
	int 3
	ENDM
else
PAUSE	MACRO
	ENDM
endif

; EXTERNAL FUNCTIONS

externFP	<AdjustHplc>
externFP	<CorrectDodPlcs>
externFP	<N_PdodMother>
externFP	<GetSelCurChp>
externFP	<ErrorEidProc>
externFP	<CpPlc>
externFP	<N_PmwdWw>
externFP	<N_FormatDrLine>
externFP	<FScrollOK>
externFP	<N_FUpdateTable>
externFP	<N_PdrFetch>
externFP	<FInitHplcEdl>
externFP	<N_PtOrigin>
externFP	<SetElevWw>
externFP	<N_CachePara>
externFP	<FUpdateHplcpad>
externFP	<ClearCap>
externFP	<CpOutlineAdvance>
externFP	<N_FOpenPlc>
externFP	<InitPlcedl>
externFP	<FreeEdl>
externFP	<N_FInTableDocCp>
externFP	<CpFirstTap>
externFP	<DysHeightTable>
externFP	<ScrollDrDown>
externFP	<N_DisplayFli>
externFP	<DlkFromVFli>
externFP	<CacheTc>
externFP	<FFormatLineFSpec>
externFP	<FMsgPresent>
externFP	<FrameDr>
externFP	<YwFromYp>
externFP	<N_FreePdrf>
externFP	<FAbsPap>
externFP	<FreeEdls>
externFP	<GetPlc>
externFP	<PutCpPlc>
externFP	<PutPlc>
externFP	<DrawEndmark>
externFP	<ReloadSb>
externFP	<ScrollWw>
externFP	<CpLimFtnCp>
externFP	<PutIMacPlc>
externFP	<XwFromXp>
externFP	<N_WidthHeightFromBrc>
externFP	<CpFirstTap1>
externFP	<CacheSectProc>
externFP	<FInCa>
externFP	<ChFetch>
externFP	<SetErrorMatProc>

ifdef DEBUG
externFP	<AssertProcForNative>
externFP    	<FCheckHandle>
externFP	<ScribbleProc>
externFP	<S_FormatDrLine>
externFP	<S_PdrFetch>
externFP	<S_PtOrigin>
externFP	<S_CachePara>
externFP	<S_ScrollDrUp>
externFP	<S_FInTableDocCp>
externFP	<S_DisplayFli>
externFP	<FRareProc>
externFP	<S_FreePdrf>
externFP	<ShakeHeapSb>
externFP	<N_PwwdWw>
externFP	<S_WidthHeightFromBrc>
externFP	<S_FOpenPlc>
externFP	<S_FUpdateTable>
endif ;DEBUG


sBegin  data

; EXTERNALS
externW     caAdjust
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
externW     vfEndPage
externW     vsab
externD     vcpFirstLayout
externD     vfli
externW     vhwwdOrigin
externW     vpapFetch
externW     vmerr
externW     vpref
externW     mpsbps
externW     dypCS
externW     vfti
externW     vsci

ifdef DEBUG
externW     vpdrfHead
externW     cCaAdjust
externW     cHpFreeze
externW     vdbs
externW     wFillBlock
externW     vfCheckPlc
endif ;DEBUG


sEnd    data

; CODE SEGMENT _DISP2

sBegin	disp2
	assumes cs,disp2
        assumes ds,dgroup
        assumes ss,dgroup

include asserth.asm


;-------------------------------------------------------------------------
;	FUpdateDr(ww, hpldr, idr, rcwInval, fAbortOK, udmod, cpUpd)
;-------------------------------------------------------------------------
;/* F  U P D A T E  D R */
;Updates dr specified by idr in hpldr(or hwwd) in window ww.
;Returns true if update was completed with no errors or interruptions.
;
;ww: drawing environment, + fOutline, fPageView, etc. modes.
;hpldr: a hpldr or hwwd in which the dr lives
;idr: the index of the dr
;rcwInval : explicit invalid RC to be redrawn
;
;udmodNormal: update whole dr whether dl's are visible or not. Stop after
;cpUpd has been included in hplcedl ( < Mac!) if cpUpd is != cpNil.
;udmodLastDr: stops when dl's which are below the visible area are encountered.
;(fComplete will be set to fFalse in that case)
;umodTable: updating will continue below the dyl limit of the dr, although
;dl's will not be displayed.  Dl's not drawn will be left dirty. height of
;dr will be updated to full height.  Scan will end with end of dr/end of
;page, end of document character only.
;umodNoDisplay: only dirty edl's are created with correct cp's and yp,
;with no display. We can assume dt starts empty in this case and that
;fPageView is true.
;udmodLimit: update is stoped after cpUpd is updated.
;
;Also returns:
;fComplete in dr is set iff all dl's in the dr are updated. This need not
;be the case in the last dr on a page which will not be normally updated
;where it is not visible.
;end of scan is returned in cpLim of hplcedl of dr (or cpNil if dlMac == 0)
;if udmodTable, dyl of dr will be updated to actual height. if this is
;shorter than previous height, space between will be blanked.
;vfEndPage is set iff the following dr's in the text stream should be
;cleared because this dr ended with an end page character.
;*/
;NATIVE FUpdateDr(ww, hpldr, idr, rcwInval, fAbortOK, udmod, cpUpd)
;struct PLDR **hpldr;
;struct RC rcwInval;
;CP cpUpd;
;{
;	 struct WWD **hwwd, *pwwd;
;	 struct DR *pdr;
;	 int dlMac;
;	 int dlOld;
;	 int dlNew;
;	 int doc, docMother;
;	 struct PLCEDL **hplcedl;
;	 int ypTop;
;	 int dyl;
;	 int ypFirstInval;
;	 int ypLimInval;
;	 int ypBottomOld;
;	 int ypMacOld;
;	 int dlMacT;
;	 int emk;
;	 BOOL fPageView, dlk = dlkNil;
;	 BOOL fPVNTS; /* PageViewNotTable and Successor */
;	 BOOL fCacheValid, fLimSuspect;
;	 BOOL fOutline;
;	 BOOL fScrollOK, fIncomplete;
;	 BOOL fReturn;
;	 BOOL fRMark;
;	 int ypFirstShow, ypLimShow, ypLimWw;
;	 struct MWD *pmwd;
;	 struct EDL *pedl, edl;
;	 struct DOD *pdod;
;	 BOOL fFtn;
;	 CP cp, cpT, cpLim, cpMac;
;	 struct DRF drfFetch;
;
;#ifdef DEBUG
;	 int fCheckPlcSave = vfCheckPlc;
;	 BOOL fInTable;
;#endif

; %%Function:N_FUpdateDr %%Owner:BRADV
cProc	N_FUpdateDr,<PUBLIC,FAR>,<si,di>
	ParmW	<ww>
	ParmW	<hpldr>
	ParmW	<idr>
	ParmW	<rcwInvalYpBottomRc>
	ParmW	<rcwInvalXpRightRc>
	ParmW	<rcwInvalYpTopRc>
	ParmW	<rcwInvalXpLeftRc>
	ParmW	<fAbortOK>
	ParmW	<udmod>
	ParmD	<cpUpd>

	LocalW	dlk
	LocalW	hwwd
	LocalW	bFlagsWwd
	LocalD	cpLim
	LocalD	cp
	LocalD	cpMac
	LocalW	ypFirstShow
	LocalW	dyl
	LocalW	ypLimWw
	LocalW	ypLimShow
	LocalW	ypFirstInval
	LocalW	ypLimInval
	LocalW	ypTop
	LocalW	dlNew
	LocalW	dlOld
	LocalW	fScrollOKVar
	LocalW	fRMark
	LocalW	fPVNTS
	LocalW	fLimSuspect
	LocalW	dlMac
	LocalW	ypMacOld
	LocalD	cpT
	LocalW	dypT1
	LocalW	dypT2
	LocalW	ypMacOld
	LocalV	edl, cbEdlMin
	LocalV	drfFetch, cbDrfMin
ifdef DEBUG
	LocalW	fCheckPlcSave
	LocalW	fInTable
	LocalW	pdrT
endif ;DEBUG

cBegin

	mov	bptr ([dlk]),dlkNil

;	Debug(vfCheckPlc = fFalse);
ifdef DEBUG
	push	ax
	mov	ax,fFalse
	xchg	ax,[vfCheckPlc]
	mov	[fCheckPlcSave],ax
	pop	ax
endif ;DEBUG

;	hwwd = HwwdWw(ww);
	mov	bx,[ww]
	shl	bx,1
	mov	bx,[bx.mpwwhwwd]
	mov	[hwwd],bx

;	vfEndPage = fFalse;
	errnz	<fFalse>
	xor	ax,ax
	mov	[vfEndPage],ax
	mov	[fRMark],ax

;	pdr = PdrFetch(hpldr, idr, &drfFetch);
;LRestart:
	lea	ax,[drfFetch]
ifdef DEBUG
	cCall	S_PdrFetch,<[hpldr], [idr], ax>
else ;not DEBUG
	cCall	N_PdrFetch,<[hpldr], [idr], ax>
endif ;DEBUG
	xchg	ax,si
LRestart:

;	if (pdr->doc == docNil)
;		{
;		/* header couldn't allocate a doc */
;		pdr->fCpBad = pdr->fDirty = fFalse;
;		Debug(vfCheckPlc = fCheckPlcSave);
;		fReturn = fTrue;
;		goto LRet;
;		}
	cmp	[si.docDr],docNil
	jne	FUD04
	errnz	<(fCpBadDr) - (fDirtyDr)>
	and	[si.fCpBadDr],NOT (maskFCpBadDr+maskFDirtyDr)
FUD01:
ifdef DEBUG
	test	[si.fCpBadDr],maskFCpBadDr
	je	FUD02
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midDisp2n
	mov	bx,1001
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FUD02:
	push	[fCheckPlcSave]
	pop	[vfCheckPlc]
endif ;DEBUG
	xor	ax,ax	    ;"dec ax" at FUD74 will force ax non-zero
FUD03:
	jmp	FUD74

;	if ( !pdr->fDirty )
;		{
;		/* don't need to update a DR that's not dirty! */
;		Assert ( !pdr->fCpBad );
;		Debug(vfCheckPlc = fCheckPlcSave);
;		fReturn = fTrue;
;		goto LRet;
;		}
FUD04:
	test	[si.fDirtyDr],maskFDirtyDr
	je	FUD01

;       pdr->xwLimScroll = 0;
	xor	ax,ax
	mov	[si.xwLimScrollDr],ax

;/* create plcedl if there is none */
;	if (pdr->hplcedl == hNil)
;		{
;		if (!FInitHplcedl(1, pdr->cpFirst, hpldr, idr))
;			{
;			fReturn = fFalse;
;			goto LRet;
;			}
;		}
;	hplcedl = pdr->hplcedl;
	errnz	<hNil>
	cmp	[si.hplcedlDr],ax
	jne	FUD045
	inc	ax
	cCall	FInitHplcedl,<ax,[si.HI_cpFirstDr],[si.LO_cpFirstDr],[hpldr],[idr]>
	or	ax,ax
	je	FUD03	    ;fReturn (in ax) already fFalse
FUD045:

;	pwwd = *hwwd;
;	fOutline = pwwd->fOutline;
;	fPageView = pwwd->fPageView;
;	fScrollOK = FScrollOK(ww);
	cCall	FScrollOK,<[ww]>
	mov	[fScrollOKVar],ax
	mov	di,[hwwd]
	mov	di,[di]
	errnz	<(fOutlineWwd) - (fPageViewWwd)>
	mov	al,[di.fOutlineWwd]
	mov	bptr ([bFlagsWwd]),al

;	fPVNTS = fPageView && udmod != udmodTable && pdr->idrFlow != idrNil;
;	fIncomplete = fFalse;
	and	ax,maskFPageViewWwd
	cmp	[udmod],udmodTable
	je	FUD05
	cmp	[si.idrFlowDr],idrNil
	jne	FUD06
FUD05:
	mov	al,fFalse
FUD06:
	;Assembler note: fIncomplete kept in high byte of fPVNTS
	mov	[fPVNTS],ax

;/* displacement to convert yw to yl */
;	dlMac = IMacPlc(hplcedl);
	;LN_IMacPlc performs dlMac = IMacPlc(hplcedl);	ax, bx are altered.
	call	LN_IMacPlc

;	doc = pdr->doc;
;	if (doc < 0)
;		{
;		cpMac = ccpEop + 1;
;		fFtn = fFalse;
;		}
;	else
;		{
;		cpMac = CpMacDoc(doc);
;		pdod = PdodDoc(doc);	/* verbose for native compiler bug */
;		fFtn = fPageView && pdod->fFtn;
;		}
	mov	ax,ccpEop + 1
	cwd
	mov	ch,fFalse	;default fFtn = fFalse
	mov	bx,[si.docDr]
	or	bx,bx
	jl	FUD07
	;***Begin in line CpMacDoc
;	 struct DOD *pdod = PdodDoc(doc);
;	 return(pdod->cpMac - 2*ccpEop );
	;Takes doc in bx, result in bx.  Only bx is altered.
	call	LN_PdodDoc
	mov	ax,-2*ccpEop
	cwd
	add	ax,[bx.LO_cpMacDod]
	adc	dx,[bx.HI_cpMacDod]
	;***End in line CpMacDoc
	test	bptr ([bFlagsWwd]),maskFPageViewWwd
	je	FUD07
	mov	ch,[bx.fFtnDod]
FUD07:
	mov	[OFF_cpMac],ax
	mov	[SEG_cpMac],dx

;	fLimSuspect = pdr->fLimSuspect;
	mov	cl,[si.fLimSuspectDr]
	mov	bptr ([fLimSuspect]),cl

;	if (pdr->cpLim == cpNil || (pdr->fIncomplete && fLimSuspect))
;		{
;		cpLim = cpMac;
;		if (fFtn && dlMac > 0)
;			cpLim = CpLimFtnCp(doc, CpMax(cp0, CpPlc(hplcedl, dlMac - 1) - 1));
;		}
;	else
;		{
;		cpLim = pdr->cpLim;
;		if (fFtn && fLimSuspect)
;			{
;			fLimSuspect = fFalse;
;			cpLim = CpLimFtnCp(doc, cpLim - 1);
;			}
;		}
	errnz	<LO_cpNil - 0FFFFh>
	errnz	<HI_cpNil - 0FFFFh>
	mov	bx,[si.LO_cpLimDr]
	and	bx,[si.HI_cpLimDr]
	inc	bx
	je	FUD09
	errnz	<(fIncompleteDr) - (fLimSuspectDr)>
	xor	cl,maskFIncompleteDr+maskFLimSuspectDr
	test	cl,maskFIncompleteDr+maskFLimSuspectDr
	je	FUD09
FUD08:
	mov	ax,[si.LO_cpLimDr]
	mov	dx,[si.HI_cpLimDr]
	test	cl,maskFLimSuspectDr
	jne	FUD096
	test	ch,maskFFtnDod
	je	FUD096
	mov	bptr ([fLimSuspect]),cl
	jmp	short FUD092
FUD09:
	test	ch,maskFFtnDod
	je	FUD096
	mov	ax,[dlMac]
	dec	ax
	jl	FUD096
	cCall	CpPlc,<[si.hplcedlDr],ax>
FUD092:
	sub	ax,1
	sbb	dx,0
	jge	FUD094
	xor	ax,ax
	cwd
FUD094:
	cCall	CpLimFtnCp,<[si.docDr],dx,ax>
FUD096:
	mov	[OFF_cpLim],ax
	mov	[SEG_cpLim],dx

	;***Begin in line YpFromYw
	;pt = PtOrigin(hpldr, idr);
	;yp = yw - (*vhwwdOrigin)->rcePage.yeTop - (*vhwwdOrigin)->ywMin - pt.yl;
	;return(yp);
ifdef DEBUG
	cCall	S_PtOrigin,<[hpldr],[idr]>
else ;not DEBUG
	cCall	N_PtOrigin,<[hpldr],[idr]>
endif ;DEBUG
	mov	bx,[vhwwdOrigin]
	mov	bx,[bx]
	add	dx,[bx.rcePageWwd.yeTopRc]
	add	dx,[bx.ywMinWwd]
	;***End in line YpFromYw
;	ypFirstShow = max(0, YpFromYw(hpldr, idr,
;		pwwd->ywMin + max(0,pwwd->rcePage.yeTop)));
	xor	ax,ax
	cmp	[bx.rcePageWwd.yeTopRc],ax
	jl	FUD098
	mov	ax,[bx.rcePageWwd.yeTopRc]
FUD098:
	add	ax,[di.ywMinWwd]
	sub	ax,dx
	jge	FUD10
	xor	ax,ax
FUD10:
	mov	[ypFirstShow],ax

;	ypLimShow = min(dyl = pdr->dyl,
;		ypLimWw = YpFromYw(hpldr, idr,
;			min(pwwd->ywMac, pwwd->ywMin + pwwd->rcePage.yeBottom)));
	mov	ax,[si.dylDr]
	mov	[dyl],ax
	mov	cx,[di.ywMinWwd]
	add	cx,[di.rcePageWwd.yeBottomRc]
	cmp	cx,[di.ywMacWwd]
	jl	FUD105
	mov	cx,[di.ywMacWwd]
FUD105:
	sub	cx,dx
	mov	[ypLimWw],cx
	cmp	ax,cx
	jle	FUD11
	mov	ax,cx
FUD11:
	mov	[ypLimShow],ax

;	ypFirstInval = YpFromYw(hpldr, idr, rcwInval.ywTop);
	mov	ax,[rcwInvalYpTopRc]
	sub	ax,dx
	mov	[ypFirstInval],ax

;	ypLimInval =  YpFromYw(hpldr, idr, rcwInval.ywBottom);
	mov	ax,[rcwInvalYpBottomRc]
	sub	ax,dx
	mov	[ypLimInval],ax

;	Assert(!pdr->fCpBad);
ifdef DEBUG
	test	[si.fCpBadDr],maskFCpBadDr
	je	FUD12
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midDisp2n
	mov	bx,1002
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FUD12:
endif ;DEBUG

;/* calculate desired elevator position dq. */
;	if (pwwd->fSetElev)
;		{
	test	[di.fSetElevWwd],maskFSetElevWwd
	je	FUD13

;		pwwd->fSetElev = fFalse;
;		SetElevWw(ww);
	and	[di.fSetElevWwd],NOT maskFSetElevWwd
	cCall	SetElevWw,<[ww]>

;		pwwd = *hwwd;
;		}
	;Assembler note: compute pwwd from hwwd when needed, do nothing here.
FUD13:

;	ypTop = - pdr->dypAbove;
	mov	ax,[si.dypAboveDr]
	neg	ax
	mov	[ypTop],ax

;	if (dlMac == 0)
;		ypMacOld = 0;
;	else
;		{
;		GetPlc(hplcedl, dlMac - 1, &edl);
;		ypMacOld = min(dyl, edl.ypTop + edl.dyp);
;		}
;	dlNew = 0;
	xor	ax,ax
	mov	[dlNew],ax
	mov	cx,[dlMac]
	jcxz	FUD14
	dec	cx
	;LN_GetPlc performs GetPlc(hplcedl, cx, &edl)
	;ax, bx, cx, dx are altered.
	call	LN_GetPlc
	mov	ax,[edl.ypTopEdl]
	add	ax,[edl.dypEdl]
	cmp	ax,[dyl]
	jle	FUD14
	mov	ax,[dyl]
FUD14:
	mov	[ypMacOld],ax

;	if ((cp = pdr->cpFirst) == cpNil)
;		goto LEndDr;
	mov	ax,[si.LO_cpFirstDr]
	mov	dx,[si.HI_cpFirstDr]
	mov	[OFF_cp],ax
	mov	[SEG_cp],dx
	errnz	<LO_cpNil - 0FFFFh>
	errnz	<HI_cpNil - 0FFFFh>
	mov	cx,ax
	and	cx,dx
	inc	cx
	je	Ltemp010

;	if (pdr->fNoParaStart)
;		{
	test	[si.fNoParaStartDr],maskFNoParaStartDr
	je	FUD15

;		CachePara(doc, cp);
ifdef DEBUG
	cCall	S_CachePara,<[si.docDr],dx,ax>
else ;not DEBUG
	cCall	N_CachePara,<[si.docDr],dx,ax>
endif ;DEBUG

;		if (cp == caPara.cpFirst)
;			goto LEndDr;
	mov	ax,[caPara.LO_cpFirstCa]
	cmp	ax,[OFF_cp]
	jne	FUD15
	mov	ax,[caPara.HI_cpFirstCa]
	cmp	ax,[SEG_cp]
Ltemp010:
	je	Ltemp001

;		}
FUD15:

;	if (fOutline)
;		{
	test	bptr ([bFlagsWwd]),maskFOutlineWwd
	je	FUD17

;		pdod = PdodDoc(doc);
	;Takes doc in bx, result in bx.  Only bx is altered.
	mov	bx,[si.docDr]
	call	LN_PdodDoc

;		if (pdod->fOutlineDirty)
;			FUpdateHplcpad(doc);
	test	[bx.fOutlineDirtyDod],maskFOutlineDirtyDod
	je	FUD16
	cCall	FUpdateHplcpad,<[si.docDr]>
FUD16:

;		ClearCap();
;		}
	cCall	ClearCap,<>
FUD17:

;	for (dlOld = 0;;)
;/* we have: cp points to text desired on the coming line dlNew
;ypTop: desired position for top of dlNew
;dlOld: next line to be considered for re-use
;*/
;		{
	mov	[dlOld],0
FUD18:

;		if (fOutline && udmod != udmodTable)
;			cp = CpOutlineAdvance(doc, cp);
	test	bptr ([bFlagsWwd]),maskFOutlineWwd
	je	FUD19
	cmp	[udmod],udmodTable
	je	FUD19
	cCall	CpOutlineAdvance,<[si.docDr], [SEG_cp], [OFF_cp]>
	mov	[OFF_cp],ax
	mov	[SEG_cp],dx
FUD19:

;/* check for having to extend hplcedl array */
;/* call ShakeHeap() here because we check if hplcedl needs to be enlarged
;in this routine rather than always calling a routine which calls
;ShakeHeap() for us. */
;		Debug(vdbs.fShakeHeap ? ShakeHeap() : 0);
ifdef DEBUG
	call	LN_ShakeHeap
endif ;DEBUG

;		if (dlNew >= (*hplcedl)->dlMax - 1)
;			{
	mov	bx,[si.hplcedlDr]
	mov	bx,[bx]
	mov	ax,[bx.dlMaxPlcedl]
	dec	ax
	cmp	[dlNew],ax
	jl	FUD21

;/* extend the array with uninitialized dl's, increment max, break if no space.
;We assume that dlMac(Old) was <= dlMax, so the dl's will not be looked at
;but used only to store new lines */
;			PutIMacPlc(hplcedl, (*hplcedl)->dlMax - 1);
	xchg	ax,di	;save dlMacT
	cCall	PutIMacPlc,<[si.hplcedlDr],di>

;			if (!FOpenPlc(hplcedl, dlMacT = IMacPlc(hplcedl), 1/*cdlIncr*/))
;				{
	mov	ax,1
ifdef DEBUG
	cCall	S_FOpenPlc,<[si.hplcedlDr], di, ax>
else ;not DEBUG
	cCall	N_FOpenPlc,<[si.hplcedlDr], di, ax>
endif ;DEBUG
	or	ax,ax
	jne	FUD20

;				dlk = dlkEndPage;
	mov	bptr ([dlk]),dlkEndPage

;				SetErrorMat(matDisp);
	mov	ax,matDisp
	push	ax
	cCall	SetErrorMatProc,<>

;				goto LEndDr;
;				}
Ltemp001:
	jmp	LEndDr
Ltemp009:
	jmp	FUD36
FUD20:

;			InitPlcedl(hplcedl, dlMacT, 1);
	mov	ax,1
	cCall	InitPlcedl,<[si.hplcedlDr],di,ax>

;			}
FUD21:

;/* discard unusable dl's */
;		for (; dlOld < dlMac; dlOld++)
;			{
	mov	cx,[dlOld]
	cmp	cx,[dlMac]
	jge	Ltemp009

;/* Set dlOld and edl to the next good dl */
;/* REVIEW: MAC/WIN */
;			GetPlc(hplcedl, dlOld, &edl);
	;LN_GetPlc performs GetPlc(hplcedl, cx, &edl)
	;ax, bx, cx, dx are altered.
	call	LN_GetPlc

;/* loop if: invalid or passed over in cp space or passed over in dl space,
;passed over in yp space or in invalid band */
;			cpT = CpPlc(hplcedl, dlOld);
;			if (edl.fDirty || dlOld < dlNew
;				|| cpT < cp
;				|| edl.ypTop < ypTop
;				|| (edl.ypTop + edl.dyp > ypFirstInval
;					&& edl.ypTop < ypLimInval))
;				{
	;LN_CpPlcEdl performs CpPlc(hpcledl, dlOld); ax, bx, cx, dx are altered.
	call	LN_CpPlcEdl
	mov	bx,[dlOld]
	test	[edl.fDirtyEdl],maskFDirtyEdl
	jne	FUD24
	cmp	bx,[dlNew]
	jl	FUD24
	cmp	ax,[OFF_cp]
	mov	cx,dx
	sbb	cx,[SEG_cp]
	jl	FUD24
	mov	cx,[edl.ypTopEdl]
	cmp	cx,[ypTop]
	jl	FUD24
	cmp	cx,[ypLimInval]
	jge	FUD26
	add	cx,[edl.dypEdl]
	cmp	cx,[ypFirstInval]
	jle	FUD26
FUD24:

;				if (dlOld >= dlNew)
	cmp	bx,[dlNew]
	jl	FUD21

;					FreeEdl(hplcedl, dlOld);
	cCall	FreeEdl,<[si.hplcedlDr],bx>

;				continue;
Ltemp002:
	inc	[dlOld]
	jmp	short FUD21

;				}
FUD26:

;/* now we have dlOld, an acceptable if not necessarily useful dl.
;now compute dlNew either from scratch or by re-using dlOld. To be
;re-useable, dlOld must start at the right cp. */
;			if (cpT == cp)
;				{
	mov	[OFF_cpT],ax
	mov	[SEG_cpT],dx
	sub	ax,[OFF_cp]
	sbb	dx,[SEG_cp]
	or	ax,dx
	je	Ltemp008
	jmp	FUD35
Ltemp008:

;/* Re-use this dl */
;   				pdr->xwLimScroll = max( pdr->xwLimScroll, 
;  				  XwFromXp( hpldr, idr, edl.xpLeft + edl.dxp ));

	;FUD96 performs
	;pdr->xwLimScroll = max( pdr->xwLimScroll,
	;	XwFromXp( hpldr, idr, edl.xpLeft + edl.dxp ) );
	;ax, bx, cx, dx are altered.
	call	FUD96

;/* if new place is not visible */
;				if ((ypTop >= ypLimShow || ypTop + edl.dyp <= ypFirstShow) &&
;					(edl.hpldr == hNil || !edl.fTableDirty))
;					{
	mov	ax,[ypTop]
	cmp	ax,[ypLimShow]
	jge	FUD27
	add	ax,[edl.dypEdl]
	cmp	ax,[ypFirstShow]
	jg	FUD28
FUD27:
	cmp	[edl.hpldrEdl],hNil
	je	FUD275
	test	[edl.fTableDirtyEdl],maskFTableDirtyEdl
	jne	FUD28
FUD275:

;					if (dlNew != dlOld)
;						{
;						struct PLDR **hpldrT;
	mov	ax,[dlNew]
	cmp	ax,[dlOld]
	je	FUD31

;						PutCpPlc(hplcedl, dlNew, cp);
;						PutPlc(hplcedl, dlNew, &edl);
;						hpldrT = edl.hpldr;
;						edl.hpldr = hNil;
;						PutPlc(hplcedl, dlOld, &edl);
;						edl.hpldr = hpldrT;
;						}
	;LN_PutCpPlc performs PutCpPlc(hpcledl, dlNew, cp);
	;ax, bx, cx, dx are altered.
	call	LN_PutCpPlc
	;LN_PutPlcNew performs PutPlc(hplcedl, dlNew, &edl);
	;ax, bx, cx, dx are altered.
	call	LN_PutPlcNew
	errnz	<hNil>
	xor	di,di
	xchg	di,[edl.hpldrEdl]
	mov	cx,[dlOld]
	;LN_PutPlc performs PutPlc(hplcedl, cx, &edl);
	;ax, bx, cx, dx are altered.
	call	LN_PutPlc
	mov	[edl.hpldrEdl],di

;					}
	jmp	short FUD31

;				else if (dlOld != dlNew || edl.ypTop != ypTop)
;					{
FUD28:

	mov	cx,[edl.ypTopEdl]
	mov	ax,[dlOld]
	cmp	ax,[dlNew]
	jne	FUD29
	cmp	cx,[ypTop]
	je	FUD31
FUD29:

;/* if old place is not completely visible below */
;					if (edl.ypTop + edl.dyp > ypLimShow || !fScrollOK)
;						continue;
	add	cx,[edl.dypEdl]
	cmp	cx,[ypLimShow]
	jg	Ltemp002
	cmp	[fScrollOKVar],fFalse
	je	Ltemp002

;					Assert(udmod != udmodNoDisplay);
ifdef DEBUG
	cmp	[udmod],udmodNoDisplay
	jne	FUD30
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midDisp2n
	mov	bx,1003
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FUD30:
endif ;DEBUG

;					ScrollDrUp(ww, hpldr, idr, hplcedl, dlOld, dlNew, edl.ypTop, ypTop, ypFirstShow, ypLimShow);
	push	[ww]
	push	[hpldr]
	push	[idr]
	push	[si.hplcedlDr]
	push	ax
	push	[dlNew]
	push	[edl.ypTopEdl]
	push	[ypTop]
	push	[ypFirstShow]
	push	[ypLimShow]
ifdef DEBUG
	cCall	S_ScrollDrUp,<>
else ;not DEBUG
	call	far ptr N_ScrollDrUp
endif ;DEBUG

;					dlMac = IMacPlc(hplcedl);
	;LN_IMacPlc performs dlMac = IMacPlc(hplcedl);	ax, bx are altered.
	call	LN_IMacPlc

;					dlOld = dlNew;
	mov	ax,[dlNew]
	mov	[dlOld],ax

;					}
FUD31:

;/* If the dl is for a complex object (i.e. table row), and its internal
;/* structure has been dirtied, break out and use this dl to perform an
;/* incremental update.
;/**/
;				if (edl.hpldr != hNil && edl.fTableDirty)
;					{
;					Assert(dlOld == dlNew);
;					break;
;					}
	cmp	[edl.hpldrEdl],hNil
	je	FUD33
	test	[edl.fTableDirtyEdl],maskFTableDirtyEdl
ifdef DEBUG
	je	FUD33
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[dlOld]
	cmp	ax,[dlNew]
	je	FUD32
	mov	ax,midDisp2n
	mov	bx,1004
	cCall	AssertProcForNative,<ax,bx>
FUD32:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	jmp	short FUD36
else ;not DEBUG
	jne	FUD36
endif ;DEBUG
FUD33:

;				if (fPVNTS && ypTop + edl.dyp > dyl && dlNew != 0)
;					goto LEndDr;

	mov	ax,[ypTop]
	add	ax,[edl.dypEdl]
	cmp	bptr ([fPVNTS]),fFalse
	je	FUD34
	cmp	ax,[dyl]
	jle	FUD34
	cmp	[dlNew],0
	jne	Ltemp011
FUD34:

;				cp = cpT + edl.dcp;
;				ypTop += edl.dyp;
	mov	[ypTop],ax
	mov	ax,[edl.LO_dcpEdl]
	mov	dx,[edl.HI_dcpEdl]
	add	ax,[OFF_cpT]
	adc	dx,[SEG_cpT]
	mov	[OFF_cp],ax
	mov	[SEG_cp],dx

;				++dlOld;
	inc	[dlOld]

;				dlk = edl.dlk;
	mov	al,[edl.dlkEdl]
	and	al,maskDlkEdl
	errnz	<maskDlkEdl - 007h>
	mov	bptr ([dlk]),al

;				fRMark |= edl.fRMark;
; PAUSE
	test	[edl.fRMarkEdl],maskFRMarkEdl
	je	FUD345
	mov	[fRMark],maskFRMarkDr	; special value for efficiency
FUD345:
;				goto LNextDlNew;
	jmp	LNextDlNew

;				}
FUD35:

;			break;
	;Assembler note: Fall through out of for loop for break.

;			}
FUD36:

;/* cpMin > cp, the line is not anywhere on the screen so either:
;line is the max height end line that is new, or was moving up in
;	 the dr, so it was not scrolled
;or line exists in sccBelow
;or it will have to be formatted from scratch.
;At any rate, advance cp and ypTop.
;*/
;		if (fPageView && ypTop > ypLimShow)
;			{
	test	bptr ([bFlagsWwd]),maskFPageViewWwd
	je	LCheckCpMac
	mov	ax,[ypTop]
	cmp	ax,[ypLimShow]
	jle	LCheckCpMac

;/* line is completely below visible area. update depends on various params */
;			if (udmod == udmodLastDr ||
;				(cpUpd != cpNil && cp > cpUpd))
;				{
	cmp	[udmod],udmodLastDr
	je	FUD39
	mov	ax,[OFF_cpUpd]
	mov	dx,[SEG_cpUpd]
	errnz	<LO_cpNil - 0FFFFh>
	errnz	<HI_cpNil - 0FFFFh>
	mov	cx,ax
	and	cx,dx
	inc	cx
	je	LCheckCpMac
	sub	ax,[OFF_cp]
	sbb	dx,[SEG_cp]
	jge	LCheckCpMac
FUD39:

;				fIncomplete = fTrue;
	;Assembler note: fIncomplete kept in high byte of fPVNTS
	mov	bptr ([fPVNTS+1]),maskFIncompleteDr

;				goto LEndDr;
Ltemp011:
	jmp	short Ltemp005

;				}
;			}
;LCheckCpMac:
LCheckCpMac:

;		if (cp >= cpMac)
;			{
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	cmp	ax,[OFF_cpMac]
	mov	cx,dx
	sbb	cx,[SEG_cpMac]
	jl	FUD41

;			if (fPageView)
;				{
	test	bptr ([bFlagsWwd]),maskFPageViewWwd
	je	FUD40

;				dlk = dlkEndPage;
;				goto LEndDr;
;				}
	mov	bptr ([dlk]),dlkEndPage
	jmp	short Ltemp005
FUD40:

;/* draw tall white block with the endmark in the upper left corner */
	;Assembler note: emk is only used in the following call, set it there.
;			emk = emkEndmark;
;			edl.dlk = dlkEnd;
;			edl.fDirty = fFalse;
;			edl.dcpDepend = 0;
	errnz	<(dlkEdl) - (dcpDependEdl) - 1>
	errnz	<(dlkEdl) - (fDirtyEdl)>
	and	wptr ([edl.dcpDependEdl]),(NOT (maskDlkEdl + maskFDirtyEdl)) SHL 8
	errnz	<maskDlkEdl - 007h>
	or	[edl.dlkEdl],dlkEnd

;			edl.dcp = 1;
;			edl.ypTop = ypTop;
;			edl.dyp = 0x4000 - ypTop;
;			edl.hpldr = hNil;
	mov	ax,[ypTop]
	mov	[edl.ypTopEdl],ax
	neg	ax
	add	ax,04000h
	mov	[edl.dypEdl],ax
	errnz	<hNil>
	xor	ax,ax
	mov	[edl.hpldrEdl],ax
	mov	[edl.HI_dcpEdl],ax
	inc	ax
	mov	[edl.LO_dcpEdl],ax

;			pdr->xwLimScroll = max( pdr->xwLimScroll, 
;			    XwFromXp( hpldr,
;	  			idr, 
;				(edl.xpLeft = 0) + (edl.dxp = vsci.dxpScrlBar) ));
	dec	ax
	mov	[edl.xpLeftEdl],ax
	mov	ax,[vsci.dxpScrlBarSci]
	mov	[edl.dxpEdl],ax
	;FUD96 performs
	;pdr->xwLimScroll = max( pdr->xwLimScroll,
	;	XwFromXp( hpldr, idr, edl.xpLeft + edl.dxp ) );
	;ax, bx, cx, dx are altered.
	call	FUD96

;			PutCpPlc(hplcedl, dlNew, cp);
	;LN_PutCpPlc performs PutCpPlc(hpcledl, dlNew, cp);
	;ax, bx, cx, dx are altered.
	call	LN_PutCpPlc

;			PutPlc(hplcedl, dlNew, &edl);
	;LN_PutPlcNew performs PutPlc(hplcedl, dlNew, &edl);
	;ax, bx, cx, dx are altered.
	call	LN_PutPlcNew

;			DrawEndmark(ww, hpldr, idr, ypTop, dyl, emk);
	;LN_DrawEndmark performs DrawEndmark(ww, hpldr, idr, ypTop, ax, bx);
	;ax, bx, cx, dx are altered.
	mov	ax,[dyl]
	mov	bx,emkEndmark
	call	LN_DrawEndmark

;			cp++; dlNew++; ypTop = 0x4000;
	add	[OFF_cp],1
	adc	[SEG_cp],0
	inc	[dlNew]
	mov	[ypTop],04000h

;			goto LEndDr;
Ltemp005:
	jmp	LEndDr
Ltemp007:
	jmp	LFSpec

;			}
;		else if (doc < 0)
;			goto LFSpec;
FUD41:
	mov	cx,[si.docDr]
	or	cx,cx
	jl	Ltemp007

;		else
;			{
;			CachePara(doc, cp);
ifdef DEBUG
	cCall	S_CachePara,<cx,dx,ax>
else ;not DEBUG
	cCall	N_CachePara,<cx,dx,ax>
endif ;DEBUG

;			Debug(fInTable = FInTableVPapFetch(doc, cp));
;			Assert ( udmod != udmodTable || fInTable );
;#define FInTableVPapFetch(doc, cp)  (vpapFetch.fInTable && FInTableDocCp(doc, cp))
;			/* pdr->fInTable should be set iff udmod == udmodTable */
;#ifdef DEBUG
;			Assert (pdr->fInTable == (udmod == udmodTable));
;#endif
ifdef DEBUG
	;Do this assert with a call so as not to mess up short jumps.
	call	FUD90
endif ;DEBUG

;			if (FInTableVPapFetch(doc, cp) && udmod != udmodTable)
;				{
;#define FInTableVPapFetch(doc, cp)  (vpapFetch.fInTable && FInTableDocCp(doc, cp))
	errnz	<fFalse>
	xor	ax,ax
	cmp	[vpapFetch.fInTablePap],al
	je	Ltemp006
ifdef DEBUG
	cCall	S_FInTableDocCp,<[si.docDr], [SEG_cp], [OFF_cp]>
else ;not DEBUG
	cCall	N_FInTableDocCp,<[si.docDr], [SEG_cp], [OFF_cp]>
endif ;DEBUG
	xchg	ax,cx
	jcxz	Ltemp006
	cmp	[udmod],udmodTable
	je	Ltemp006

;				if (fPageView && pdr->lrk == lrkAbs ^ FAbsPap(pdr->doc, &vpapFetch))
;					{
	test	bptr ([bFlagsWwd]),maskFPageViewWwd
	je	FUD47
	;FUD80 sets flags in evaluating
	;pdr->lrk == lrkAbs ^ FAbsPap(pdr->doc, &vpapFetch)
	;ax, bx, cx, dx are altered.
	call	FUD80
	je	FUD47

;					if (pdr->lrk == lrkAbs)
;						/* normal text can't flow into abs */
;						goto LEndDr;
	cmp	[si.lrkDr],lrkAbs
	je	Ltemp005

;					/* skip abs object embedded in a non-abs table DR;
;					   it is described in a different DR */
;					CpFirstTap(doc, cp);
	cCall	CpFirstTap,<[si.docDr], [SEG_cp], [OFF_cp]>

;					if (cp == pdr->cpFirst)
;						pdr->cpFirst = caTap.cpLim;
;					cp = caTap.cpLim;
	;FUD81 performs
	;if (cp == pdr->cpFirst)
	;	pdr->cpFirst = dx:ax;
	;cp = dx:ax;
	;ax, bx, cx, dx are altered.
	mov	ax,[caTap.LO_cpLimCa]
	mov	dx,[caTap.HI_cpLimCa]
	call	FUD81

;					goto LCheckCpMac;
	jmp	LCheckCpMac
Ltemp006:
	jmp	FUD49

;					}
FUD47:

;				if (udmod == udmodNoDisplay)
;					{
	cmp	[udmod],udmodNoDisplay
	jne	FUD48

;/* create fake vfli "describing" the whole table row */
;					int dypT1, dypT2;
;					vfli.dypLine = DysHeightTable(ww, doc, cp, fFalse, fFalse, &dypT1, &dypT2);
;					vfli.cpMin = cp;
;					CpFirstTap(doc, cp);
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	mov	[vfli.LO_cpMinFli],ax
	mov	[vfli.HI_cpMinFli],dx
	mov	cx,[si.docDr]
	push	cx	    ;argument for CpFirstTap
	push	dx	    ;argument for CpFirstTap
	push	ax	    ;argument for CpFirstTap
	push	[ww]
	push	cx
	push	dx
	push	ax
	errnz	<fFalse>
	xor	cx,cx
	push	cx
	push	cx
	lea	cx,[dypT1]
	push	cx
	lea	cx,[dypT2]
	push	cx
	cCall	DysHeightTable,<>
	cCall	CpFirstTap,<>

;					vfli.cpMac = caTap.cpLim;
;					vfli.doc = docNil;
;					vfli.fSplats = fFalse;
;					goto LGotFli;
;					}
	mov	ax,[caTap.LO_cpLimCa]
	mov	[vfli.LO_cpMacFli],ax
	mov	ax,[caTap.HI_cpLimCa]
	mov	[vfli.HI_cpMacFli],ax
	mov	[vfli.docFli],docNil
	mov	[vfli.fSplatsFli],fFalse
	jmp	LGotFli
FUD48:

;				if ( FUpdateTable(ww, doc, hpldr, idr, &cp, &ypTop,
;					hplcedl, dlNew, dlOld, dlMac, ypFirstShow,
;					ypLimWw, dyl, rcwInval, fScrollOK ) )
;					{
	push	[ww]
	push	[si.docDr]
	push	[hpldr]
	push	[idr]
	lea	ax,[cp]
	push	ax
	lea	ax,[ypTop]
	push	ax
	push	[si.hplcedlDr]
	push	[dlNew]
	push	[dlOld]
	push	[dlMac]
	push	[ypFirstShow]
	push	[ypLimWw]
	push	[dyl]
	push	[rcwInvalYpBottomRc]
	push	[rcwInvalXpRightRc]
	push	[rcwInvalYpTopRc]
	push	[rcwInvalXpLeftRc]
	push	[fScrollOKVar]
ifdef DEBUG
	cCall	S_FUpdateTable,<>
else ;not DEBUG
	cCall	N_FUpdateTable,<>
endif ;DEBUG
	xchg	ax,cx
	jcxz	FUD487

;					GetPlc(hplcedl, dlNew, &edl);
;					pdr->xwLimScroll = max( pdr->xwLimScroll,
;						XwFromXp( hpldr, idr, edl.xpLeft + edl.dxp ) );
	mov	cx,[dlNew]
	;LN_GetPlc performs GetPlc(hplcedl, cx, &edl)
	;ax, bx, cx, dx are altered.
	call	LN_GetPlc
	;FUD96 performs
	;pdr->xwLimScroll = max( pdr->xwLimScroll,
	;	XwFromXp( hpldr, idr, edl.xpLeft + edl.dxp ) );
	;ax, bx, cx, dx are altered.
	call	FUD96

;					if (dlOld == dlNew)
;						dlOld++;
	mov	bx,[dlOld]
	cmp	bx,[dlNew]
	jne	FUD483
	inc	[dlOld]
FUD483:

;					if (ypTop <= 0)
;						{
;	/* this is a case where dypAbove was left larger than the height of the
;	first line (which recently had its height reduced by some edit.) In such
;	a case, we reset dypAbove and start again */
;						pdr->dypAbove = 0;
;						Assert(dlNew == 0);
;						goto LRestart;
;						}
	;FUD83 performs:
	;if (ypTop <= 0)
	;	{
	;	pdr->dypAbove = 0;
	;	Assert(dlNew == 0);
	;	goto LRestart
	;	}
	;Only ax is altered.
	call	FUD83

;					goto LNextDlNew;
	jmp	LNextDlNew

;					}
FUD487:

;				if (vmerr.fMemFail)
;					{
	cmp	[vmerr.fMemFailMerr],fFalse
	je	FUD49

;					SetErrorMat(matDisp);
	mov	ax,matDisp
        cCall   SetErrorMatProc,<ax>

;					pdr->fDirty = fFalse;
	and	[si.fDirtyDr],NOT maskFDirtyDr

;					fReturn = fFalse;
;					FreeEdls(hplcedl, max(dlOld,dlNew), dlMac);	   /* Free the rest of the edls */
;					PutIMacPlc(hplcedl, dlNew);
;					PutCpPlc(hplcedl, dlNew, cp);
	;FUD87 performs
	;FreeEdls(hplcedl, max(dlOld,dlNew), dlMac);
	;PutIMacPlc(hplcedl, dlNew);
	;PutCpPlc(hplcedl, dlNew, cp);
	;ax, bx, cx, dx are altered.
	call	FUD87

;					goto LRet;
	jmp	FUD79

;					}
;				}
FUD49:

;/* format and display the next line */

;			if (fPageView && udmod != udmodTable &&
;				(pdr->lrk == lrkAbs ^ FAbsPap(pdr->doc, &vpapFetch)))
;				{
	test	bptr ([bFlagsWwd]),maskFPageViewWwd
	je	FUD51
	cmp	[udmod],udmodTable
	je	FUD51
	;FUD80 sets flags in evaluating
	;pdr->lrk == lrkAbs ^ FAbsPap(pdr->doc, &vpapFetch)
	;ax, bx, cx, dx are altered.
	call	FUD80
	je	FUD51

;				if (pdr->lrk == lrkAbs)
;					/* normal text can't flow into abs */
;					goto LEndDr;
	cmp	[si.lrkDr],lrkAbs
	je	Ltemp004

;				/* skip abs object embedded in a non-abs DR;
;				   it is described in a different DR */
;				if (cp == pdr->cpFirst)
;					pdr->cpFirst = caPara.cpLim;
;				cp = caPara.cpLim;
	;FUD81 performs
	;if (cp == pdr->cpFirst)
	;	pdr->cpFirst = dx:ax;
	;cp = dx:ax;
	;ax, bx, cx, dx are altered.
	mov	dx,[caPara.HI_cpLimCa]
	mov	ax,[caPara.LO_cpLimCa]
	call	FUD81

;				goto LCheckCpMac;
;				}
	jmp	LCheckCpMac
FUD51:

;			else if (doc >= 0)
;                               {
;				FormatLineDr(ww, cp, pdr);
	cmp	[si.docDr],0
	jl	FUD52
ifdef DEBUG
	cCall	S_FormatDrLine,<[ww], [SEG_cp], [OFF_cp], si>
else ;not DEBUG
	cCall	N_FormatDrLine,<[ww], [SEG_cp], [OFF_cp], si>
endif ;DEBUG

;				/* FormatLine can blow the caTap, etc. */
;				if (udmod == udmodTable)
;        				CpFirstTap(doc, cp);
	cmp	[udmod],udmodTable
	jne	FUD515

; *** Begin inline CpFirstTap
;	return CpFirstTap1(doc,cpFirst,caTap.doc==doc?vtapFetch.fOutline:fFalse);
	mov	bx,[si.docDr]
	push	bx
	push	[SEG_cp]
	push	[OFF_cp]
	mov	al,[vtapFetch.fOutlineTap]
	and	ax,maskfOutlineTap
	cmp	bx,[caTap.docCa]
	je	FUD513
	mov	al,fFalse
FUD513:
	push	ax
	cCall	CpFirstTap1,<>
; *** End inline CpFirstTap

FUD515:
;				if (vfli.ichMac == 0)
;					{
;					/* note: fParaStopped does not mean
;					   there are no characters on the
;					   line */
;					cp = vfli.cpMac;
;					goto LCheckCpMac;
;					}
;                               }
	cmp	[vfli.ichMacFli],0
	jne	FUD54
	mov	ax,[vfli.LO_cpMacFli]
	mov	dx,[vfli.HI_cpMacFli]
	mov	[OFF_cp],ax
	mov	[SEG_cp],dx
	jmp	LCheckCpMac
Ltemp004:
	jmp	LEndDr

;			else
;				{
FUD52:

;				CP cpFirstSave;
;				/* dr is an fSpec character, -doc is the ch.
;				   need to set vcpFirstLayout so section
;				   props are obtained from mom */
;LFSpec:			Assert(fPageView);
LFSpec:
ifdef DEBUG
	;Do this assert with a call so as not to mess up short jumps.
	call	FUD88
endif ;DEBUG

;				cpFirstSave = vcpFirstLayout;
	;Assembler note: this is done below in the assembler version

;				pmwd = PmwdWw(ww);
;				docMother = pmwd->doc;
	push	si	;save pdr
	mov	ax,[ww]
	push	ax	;argument to FFormatLineFspec
	cCall	N_PmwdWw,<ax>
	xchg	ax,bx
	mov	bx,[bx.docMwd]
	push	bx	;argument to FFormatLineFspec
	push	[si.dxlDr] ;argument to FFormatLineFspec
	xor	cx,cx
	sub	cx,[si.docDr]
	push	cx	;argument to FFormatLineFspec

;				pdod = PdodDoc(docMother);
	;Takes doc in bx, result in bx.  Only bx is altered.
	call	LN_PdodDoc

;				vcpFirstLayout = CpPlc(pdod->hplcpgd, (*hwwd)->ipgd);
	push	[bx.hplcpgdDod]
	mov	bx,[hwwd]
	mov	bx,[bx]
	push	[bx.ipgdWwd]
	cCall	CpPlc,<>
	xchg	ax,si
	mov	di,dx
	xchg	wlo [vcpFirstLayout],si
	xchg	whi [vcpFirstLayout],di

;				if (!FFormatLineFspec(ww, docMother, pdr->dxl, -doc))
;					{
;					vcpFirstLayout = cpFirstSave;
;					goto LEndDr;
;					}
;				vcpFirstLayout = cpFirstSave;
	cCall	FFormatLineFspec,<>
	mov	wlo [vcpFirstLayout],si
	mov	whi [vcpFirstLayout],di
	pop	si	;restore pdr
	xchg	ax,cx
	jcxz	Ltemp012

;				}
FUD54:

;			if (fPageView)
;				{
	test	bptr ([bFlagsWwd]),maskFPageViewWwd
	je	FUD56

;				if (dlNew == 0 && pdr->fSpaceBefore)
;					ypTop -= vfli.dypBefore;
	cmp	[dlNew],0
	jne	LGotFli
	test	[si.fSpaceBeforeDr],maskFSpaceBeforeDr
	je	LGotFli
	mov	ax,[vfli.dypBeforeFli]
	sub	[ypTop],ax

;				/* have to subtract dypAfter because layout allows a
;				   line when dypAfter is only reason to reject it */
;LGotFli:			if (fPVNTS && (ypTop + vfli.dypLine - vfli.dypAfter > dyl
;					&& dlNew != 0) &&
;					!((vfli.fSplatBreak || vfli.fSplatColumn) && vfli.ichMac == 1))
;					goto LEndDr;
LGotFli:
	mov	ax,[vfli.dypLineFli]
	add	ax,[ypTop]
	cmp	bptr ([fPVNTS]),fFalse
	je	FUD55
	mov	cx,[vfli.dypAfterFli]
	add	cx,[dyl]
	cmp	ax,cx
	jle	FUD55
	cmp	[dlNew],0
	je	FUD55
;PAUSE
	errnz	<(fSplatBreakFli) - (fSplatColumnFli)>
	test	[vfli.fSplatBreakFli],maskFSplatBreakFli+maskFSplatColumnFli
	je	Ltemp012
	cmp	[vfli.ichMacFli],1
	jne	Ltemp012
FUD55:

;				ypTop += vfli.dypLine;
	mov	[ypTop],ax
	jmp	short FUD58
Ltemp012:
	jmp	LEndDr

;				}
;			else
;				{
FUD56:

;				ypTop += vfli.dypLine;
	mov	ax,[vfli.dypLineFli]
	add	[ypTop],ax

;				if (ypTop <= 0)
;					{
;/* this is a case where dypAbove was left larger than the height of the
;first line (which recently had its height reduced by some edit.) In such
;a case, we reset dypAbove and start again */
;					pdr->dypAbove = 0;
;					Assert(dlNew == 0);
;					goto LRestart;
;					}
	;FUD83 performs:
	;if (ypTop <= 0)
	;	{
	;	pdr->dypAbove = 0;
	;	Assert(dlNew == 0);
	;	goto LRestart
	;	}
	;Only ax is altered.
	call	FUD83

;				}
FUD58:

;			cp = vfli.cpMac;
	mov	ax,[vfli.LO_cpMacFli]
	mov	[OFF_cp],ax
	mov	ax,[vfli.HI_cpMacFli]
	mov	[SEG_cp],ax

;/* cp, ypTop have been advanced */
;			if (dlOld < dlMac)
;				{
	mov	ax,[dlOld]
	cmp	ax,[dlMac]
	jge	FUD61

;				if (CpPlc(hplcedl, dlOld) == cp && fScrollOK)
;					{
	cmp	[fScrollOKVar],fFalse
	je	FUD605
	;LN_CpPlcEdl performs CpPlc(hpcledl, dlOld); ax, bx, cx, dx are altered.
	call	LN_CpPlcEdl
	sub	ax,[OFF_cp]
	sbb	dx,[SEG_cp]
	or	ax,dx
	jne	FUD605

;					int ddl = 0;
;					GetPlc(hplcedl, dlOld, &edl);
	;LN_GetPlc performs GetPlc(hplcedl, cx, &edl)
	;ax, bx, cx, dx are altered.
	mov	cx,[dlOld]
	call	LN_GetPlc

;/* line at dlOld is a valid, existing line that will abutt the line just about
;to be displayed. */
;					if (dlOld == dlNew)
;/* the line about to be overwritten will be re-used in the next time around
;in the loop. Hence, it is worthwhile to save this line and its dl */
;						ddl = 1;
;/* else ddl = 0. Move the next line to its abutting position. We know that
;it has not yet been overwritten (yp > ypTop and dlOld > dlNew) */
;					if (ddl != 0 || ypTop > edl.ypTop)
;						{
	mov	bx,[ypTop]
	mov	ax,[edl.ypTopEdl]
	mov	dx,[dlOld]
	mov	cx,dx
	inc	cx
	cmp	dx,[dlNew]
	je	FUD60
	dec	cx
	cmp	bx,ax
	jle	FUD605
FUD60:

;						ypMacOld += ypTop - edl.ypTop;
	add	[ypMacOld],bx
	sub	[ypMacOld],ax

;						ScrollDrDown(ww, hpldr, idr, hplcedl,
;							dlOld, dlOld + ddl,
;							edl.ypTop
;							max(ypTop, edl.ypTop),
;							ypFirstShow, ypLimWw, dyl);
;						dlMac = IMacPlc(hplcedl);
;						dlOld += ddl;
	mov	[dlOld],cx
	push	[ww]
	push	[hpldr]
	push	[idr]
	push	[si.hplcedlDr]
	push	dx
	push	cx
	push	ax
	cmp	ax,bx
	jge	FUD603
	xchg	ax,bx
 FUD603:
	push	ax
	push	[ypFirstShow]
	push	[ypLimWw]
	push	[dyl]
	cCall	ScrollDrDown,<>
	;LN_IMacPlc performs dlMac = IMacPlc(hplcedl);	ax, bx are altered.
	call	LN_IMacPlc

;						}
;					}
FUD605:

;				if (dlOld == dlNew)
;					FreeEdl(hplcedl, dlOld++);
	mov	ax,[dlOld]
	cmp	ax,[dlNew]
	jne	FUD61
	cCall	FreeEdl,<[si.hplcedlDr],ax>
	inc	[dlOld]

;				}
FUD61:

;			fRMark |= vfli.fRMark;
; PAUSE
	test	[vfli.fRMarkFli],maskFRMarkFli
	je	FUD615
	mov	[fRMark],maskFRMarkDr	; special value for efficiency
FUD615:
;			if (udmod == udmodTable ? ypTop <= ypLarge : ypTop - vfli.dypLine < dyl)
;				{
;PAUSE
	cmp	[udmod],udmodTable
	jne	FUD618
	cmp	[ypTop],ypLarge+1
	jmp	short FUD619
FUD618:
	mov	dx,[ypTop]
	sub	dx,[vfli.dypLineFli]
	cmp	dx,[dyl]
FUD619:
	jge	FUD64
FUD62:

;				DisplayFli(udmod == udmodNoDisplay ? wwNil : ww,
;					hpldr, idr, dlNew, ypTop);
	errnz	<wwNil>
	xor	cx,cx
	cmp	[udmod],udmodNoDisplay
	je	FUD63
	mov	cx,[ww]
FUD63:
	push	cx
	push	[hpldr]
	push	[idr]
	push	[dlNew]
	push	[ypTop]
ifdef DEBUG
	cCall	S_DisplayFli,<>
else ;not DEBUG
	cCall	N_DisplayFli,<>
endif ;DEBUG

;				if (ypTop > dyl && (fPageView || udmod == udmodTable))
;/* means that line was admitted only because it fit w.o. dypAfter. But we do not
;want to reuse it, ever. In the udmodTable case, we may be clipping a partial
;line because of abs row height setting */
;					{
;					/* grab the edl that DisplayFli put in, we may be
;					/* interested in some of the info there.
;					/**/
;					GetPlc(hplcedl, dlNew, &edl);
;					goto LSetDirty;
;					}
	mov	ax,[ypTop]
	cmp	ax,[dyl]
	jle	FUD65
	test	bptr ([bFlagsWwd]),maskFPageViewWwd
	jne	FUD635
	cmp	[udmod],udmodTable
	jne	FUD65
FUD635:
	;LN_GetPlc performs GetPlc(hplcedl, cx, &edl)
	;ax, bx, cx, dx are altered.
	mov	cx,[dlNew]
	call	LN_GetPlc
	jmp	short LSetDirty

;				}
;			else
;/* special case for lines for tables which would expand the table row.
;dl's are left dirty so that next scan can update them, could also be
;a partial line at the bottom of an abs height row, in which case we
;will need additional fields in the edl to be reasonable.
;*/
;				{
FUD64:

;				PutCpPlc(hplcedl, dlNew, vfli.cpMin);
	;LN_PutCpPlcReg performs PutCpPlc(hpcledl, dlNew, dx:ax);
	;ax, bx, cx, dx are altered.
	mov	ax,[vfli.LO_cpMinFli]
	mov	dx,[vfli.HI_cpMinFli]
	call	LN_PutCpPlcReg

;				edl.hpldr = hNil;
;				edl.ypTop = ypTop - vfli.dypLine;
;				edl.dyp = vfli.dypLine;
;        edl.dxp = ((vfli.fRight || vfli.fTop || vfli.fBottom) ?
;			vfli.xpMarginRight + dxpLeftRightSpace +  DxFromBrc(vfli.brcRight,fFalse/*fFrameLines*/):
;			vfli.xpRight) - (edl.xpLeft = vfli.xpLeft);
;				edl.dcp = vfli.cpMac - vfli.cpMin;
;				edl.dcpDepend = vfli.dcpDepend;
;				edl.grpfEdl = 0;
;#define DxFromBrc(brc, fFrameLines) DxyFromBrc(brc, fFrameLines, fTrue)
;#define DxyFromBrc(brc, fFrameLines, fWidth)	 \
;			 WidthHeightFromBrc(brc, fFrameLines | (fWidth << 1))
;/* NOTE - Anyone who jumps to this label had better have already
;/* set the above fields in the EDL.
;/**/
;LSetDirty:
;				edl.fDirty = fTrue;
	push	ds
	pop	es
	lea	di,[edl.hpldrEdl]
	errnz	<hNil>
	xor	ax,ax
	stosw
	mov	ax,[vfli.xpLeftFli]
	errnz	<(xpLeftEdl) - (hpldrEdl) - 2>
	stosw
	xchg	ax,bx
	mov	ax,[ypTop]
	sub	ax,[vfli.dypLineFli]
	errnz	<(ypTopEdl) - (xpLeftEdl) - 2>
	stosw

	errnz	<(fRightFli)-(fTopFli)>
	errnz	<(fRightFli)-(fBottomFli)>
	mov	ax,[vfli.xpRightFli]
	test	[vfli.fRightFli],maskFBottomFli OR maskFTopFli OR maskFRightFli
	jz	FUD645
	push	bx	;save vfli.xpLeft
	push	[vfli.brcRightFli]
	mov	ax,2
	push	ax
ifdef DEBUG
	cCall	S_WidthHeightFromBrc,<>
else ;not DEBUG
	cCall	N_WidthHeightFromBrc,<>
endif ;DEBUG
	pop	bx	;restore vfli.xpLeft
	add	ax,[vfli.xpMarginRightFli]
	add	ax,[vfti.dxpBorderFti]
	add	ax,[vfti.dxpBorderFti]
FUD645:
	sub	ax,bx
	errnz	<(dxpEdl) - (ypTopEdl) - 2>
	stosw

	mov	ax,[vfli.dypLineFli]
	errnz	<(dypEdl) - (dxpEdl) - 2>
	stosw
	mov	ax,[vfli.LO_cpMacFli]
	sub	ax,[vfli.LO_cpMinFli]
	errnz	<(LO_dcpEdl) - (dypEdl) - 2>
	stosw
	mov	ax,[vfli.HI_cpMacFli]
	sbb	ax,[vfli.HI_cpMinFli]
	errnz	<(HI_dcpEdl) - (LO_dcpEdl) - 2>
	stosw
	mov	al,bptr ([vfli.dcpDependFli])
	errnz	<(grpfEdl) - (dcpDependEdl)>
	xor	ah,ah
	mov	[edl.grpfEdl],ax
LSetDirty:
	or	[edl.fDirtyEdl],maskFDirtyEdl

;				PutPlc(hplcedl, dlNew, &edl);
	;LN_PutPlcNew performs PutPlc(hplcedl, dlNew, &edl);
	;ax, bx, cx, dx are altered.
	call	LN_PutPlcNew

;				}
FUD65:

;			if (vfli.fSplatBreak)
;				{
	test	[vfli.fSplatBreakFli],maskFSplatBreakFli
	je	FUD66

;				pdod = PdodDoc(vfli.doc);
	;Takes doc in bx, result in bx.  Only bx is altered.
	mov	bx,[vfli.docFli]
	call	LN_PdodDoc

;				if (fPageView && !pdod->fShort && pdr->lrk != lrkAbs ||
;					udmod == udmodTable && vfli.chBreak == chTable)
;					dlk = DlkFromVfli(hplcedl, dlNew);
	test	bptr ([bFlagsWwd]),maskFPageViewWwd
	je	FUD653
	cmp	[bx.fShortDod],fFalse
	jne	FUD653
	cmp	[si.lrkDr],lrkAbs
	jne	FUD657
FUD653:
	cmp	[udmod],udmodTable
	jne	FUD66
	cmp	[vfli.chBreakFli],chTable
	jne	FUD66
FUD657:
	cCall	DlkFromVfli,<[si.hplcedlDr],[dlNew]>
	mov	bptr ([dlk]),al

;				}
FUD66:
;			}
;/* check for yp overflow in galley mode and cpLim overflow in Pageview */
;LNextDlNew:
LNextDlNew:
;		if (udmod == udmodTable && ypTop > ypLarge)
;			/* Oops!  We are going to have too large a table cell */
;			{
	cmp	[udmod],udmodTable
	jne	FUD67
	cmp	[ypTop],ypLarge
	jle	FUD67
;PAUSE
;			CacheTc(wwNil, doc, cp, fFalse, fFalse);
	errnz	<fFalse>
	xor	ax,ax
	cCall	CacheTc,<[ww],[si.docDr],[SEG_cp],[OFF_cp],ax,ax>

;			cp = vtcc.cpLim;
	mov	ax,[vtcc.LO_cpLimTcc]
	mov	[OFF_cp],ax
	mov	ax,[vtcc.HI_cpLimTcc]
	mov	[SEG_cp],ax

;			ypTop -= edl.dyp;
	mov	ax,[edl.dypEdl]
	sub	[ypTop],ax

;			break;
	jmp	LEndDr

;			}
FUD67:

;		dlNew++;
	inc	[dlNew]

;		if (dlk != dlkNil)
;			break;
	cmp	bptr ([dlk]),dlkNil
	jne	LEndDr

;		if (udmod != udmodTable)
;			{
	cmp	[udmod],udmodTable
	je	FUD683

;			if (fPageView && cp >= cpLim)
;				{
	test	bptr ([bFlagsWwd]),maskFPageViewWwd
	je	FUD68
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	cmp	ax,[OFF_cpLim]
	mov	cx,dx
	sbb	cx,[SEG_cpLim]
	jl	FUD68

; if (!fLimSuspect || ypTop >= ypMacOld)
;   break;
; PAUSE
	; <ax,dx> == cp
	test	bptr ([fLimSuspect]),maskFLimSuspectDr
	je	LEndDr
	mov	cx,[ypTop]
	cmp	cx,[ypMacOld]
	jge	LEndDr

; pdr->cpLim = cpNil;
; cpLim = cpMac;
; PAUSE
	; <ax,dx> == cp
	errnz	<LO_cpNil - 0FFFFh>
	errnz	<HI_cpNil - 0FFFFh>
	mov	cx,LO_cpNil
	mov	[si.LO_cpLimDr],cx
	mov	[si.HI_cpLimDr],cx
	mov	cx,[OFF_cpMac]
	mov	[OFF_cpLim],cx
	mov	cx,[SEG_cpMac]
	mov	[SEG_cpLim],cx

; if (cp >= cpMac)
;   {
;   dlk = dlkEndPage;
;   break;
;   }
; PAUSE
	; <ax,dx> == cp
	sub	ax,[OFF_cpMac]
	sbb	dx,[SEG_cpMac]
	jl	FUD675
	mov	bptr ([dlk]),dlkEndPage
	jmp	LEndDr
FUD675:

;				}
FUD68:
;			if (ypTop >= dyl)
;				break;
;			}
;PAUSE
	mov	ax,[ypTop]
	cmp	ax,[dyl]
	jg	LEndDr
; if (ypTop == dyl)
	jne	FUD683
; 	{
;	int ch;
;	/* we have to allow a splat at the beginning
;	   of a para to be tacked on even if no room */
;	if (!fPageView)
;		break;
;PAUSE
	test	bptr ([bFlagsWwd]),maskFPageViewWwd
	je	FUD683
;	CachePara(doc, cp);
;	if (cp != caPara.cpFirst)
;		break;
;	CacheSect(doc, cp);
;	if (cp == caSect.cpLim - ccpSect ||
;		(ch = ChFetch(doc, cp, fcmChars)) != chSect && ch != chColumnBreak)
;		{
;		break;
;		}
; native note: do this relatively rare processing in a subroutine to avoid
; breaking numerous near branches.
	cCall	FUD98
; 	}
;PAUSE
FUD683:

;		/* Before doing the next line, check for user abort. */
;		/* Check only every other line, because it is so slow to check. */
;		if (fAbortOK && (dlNew & 1) && FRareT(5, FMsgPresent(mtyUpdateWw)))
;			goto LRetInval;
	;Assembler note:  For now WIN will check every line
	cmp	[fAbortOK],fFalse
	je	FUD687
	mov	ax,mtyUpdateWw
	cCall	FMsgPresent,<ax>
;#ifdef DEBUG
;#define FRareT(n, f) ((f) || FRareProc(n))
;#else
;#define FRareT(n, f) (f)
;#endif
ifdef DEBUG
	;Do this debug stuff with a call so as not to mess up short jumps
	call	FUD94
endif ;/* DEBUG */
	or	ax,ax
	jne	Ltemp003

;		}
FUD687:
	jmp	FUD18
Ltemp003:
	jmp	LRetInval

;/* at the end of the dr: clear remaining white space (if contents shrunk)

;We have:
;	cp: to be stored as limit for last dl
;	dlNew: new dlMac
;	Reason for ending dr is in dlk:
;		End	end of document
;		EndPage
;		EndDr
;		Nil	normal i.e. dl or yp full
;*/
;LEndDr:
;	FreeEdls(hplcedl, max(dlOld,dlNew), dlMac);	   /* Free the rest of the edls */
;	PutIMacPlc(hplcedl, dlNew);
;	PutCpPlc(hplcedl, dlNew, cp);
LEndDr:
	;FUD87 performs
	;FreeEdls(hplcedl, max(dlOld,dlNew), dlMac);
	;PutIMacPlc(hplcedl, dlNew);
	;PutCpPlc(hplcedl, dlNew, cp);
	;ax, bx, cx, dx are altered.
	call	FUD87

;	pdr->fIncomplete = fIncomplete;
	and	[si.fIncompleteDr],NOT maskFIncompleteDr
	;Assembler note: fIncomplete kept in high byte of fPVNTS
	mov	al,bptr ([fPVNTS+1])
	or	[si.fIncompleteDr],al
	mov	di,[ypTop]

;	pdr->fRMark = fRMark && udmod == udmodTable;
; PAUSE
	and	[si.fRMarkDr],not maskFRMarkDr
	cmp	[udmod],udmodTable
	jne	FUD685
	mov	ax,[fRMark]
	or	[si.fRMarkDr],al
FUD685:
	
;	if (udmod != udmodNoDisplay)
;		{
	cmp	[udmod],udmodNoDisplay
	je	FUD71

;		int fFat;
;		pdr->fDirty = fFalse;
	and	[si.fDirtyDr],NOT maskFDirtyDr

;		if (udmod != udmodTable)
;			{
	cmp	[udmod],udmodTable
	je	FUD71

;			fFat = pdr->idrFlow == idrNil && doc >= 0 &&
;					((cp != cpNil && cp < pdr->cpLim) || ypTop > dyl);
	mov	cx,[si.idrFlowDr]
	errnz	<idrNil - (-1)>
	cmp	cx,idrNil
	jne	FUD69
	cmp	[si.docDr],0
	jl	FUD69
	cmp	di,[dyl]
	jg	FUD70
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	errnz	<LO_cpNil - 0FFFFh>
	errnz	<HI_cpNil - 0FFFFh>
	mov	bx,ax
	and	bx,dx
	inc	bx
	je	FUD69
	sub	ax,[si.LO_cpLimDr]
	sbb	dx,[si.HI_cpLimDr]
	jge	FUD69
	db	03Dh	;turns next "xor cx,cx" into "cmp ax,immediate"
FUD69:
	xor	cx,cx
FUD70:

;			if (fPageView && (fFat || FDrawPageDrsWw(ww)))
;				FrameDr( ww, hpldr, idr, fFat);
;			}
;		}
;#define FDrawPageDrsWw(ww)   (PwwdWw(ww)->grpfvisi.fDrawPageDrs || PwwdWw(ww)->grpfvisi.fvisiShowAll)
	test	bptr ([bFlagsWwd]),maskFPageViewWwd
	je	FUD71
	errnz	<maskfDrawPageDrsGrpfvisi - 04000h>
	errnz	<maskfvisiShowAllGrpfvisi - 00020h>
	mov	bx,[hwwd]
	mov	bx,[bx]
	test	[bx.grpfvisiWwd],maskfDrawPageDrsGrpfvisi+maskfvisiShowAllGrpfvisi
	jne	FUD705
	jcxz	FUD71
FUD705:
	cCall	FrameDr,<[ww], [hpldr], [idr], cx>

;		}
FUD71:

;/* clear area at bottom of PageView display which stopped early.
;We must do this even if NoDisplay mode is on, otherwise the screen state
;will not be accurately reflected by the dr with respect to the trailing
;white band.
;*/
;	if (ypTop < ypMacOld)
;		DrawEndmark(ww, hpldr, idr, ypTop, ypMacOld, emkBlank);
	cmp	di,[ypMacOld]
	jge	FUD72
	;LN_DrawEndmark performs DrawEndmark(ww, hpldr, idr, ypTop, ax, bx);
	;ax, bx, cx, dx are altered.
	mov	ax,[ypMacOld]
	mov	bx,emkBlank
	call	LN_DrawEndmark
FUD72:

;	if (udmod == udmodTable)
;/* save new height in dr. if container's height is exceeded, set dirty bit */
;		{
;		if (ypTop > pdr->dyl)
;			pdr->fDirty = fTrue;
;		pdr->dyl = ypTop;
;		}
	cmp	[udmod],udmodTable
	jne	FUD73
	cmp	di,[si.dylDr]
	mov	[si.dylDr],di
	jle	FUD73
	or	[si.fDirtyDr],maskFDirtyDr
FUD73:

;	vfEndPage = dlk == dlkEndPage;
	mov	al,bptr ([dlk])
	sub	al,dlkEndPage
	sub	al,1
	sbb	ax,ax
	mov	[vfEndPage],ax

;	Debug(vfCheckPlc = fCheckPlcSave);
ifdef DEBUG
	push	[fCheckPlcSave]
	pop	[vfCheckPlc]
endif ;DEBUG

FUD74:
;	fReturn = !vmerr.fMemFail;
;PAUSE
	xor	ax,ax
	cmp	[vmerr.fMemFailMerr],ax
	jne	LRet
	dec	ax	;forces ax to be non-zero

;	goto LRet;
	jmp	short LRet

;/* before returning from an interrupt, invalidate lines that were
;overwritten within the present update */
;LRetInval:
LRetInval:
	mov	di,[ypTop]

;	for (; dlOld < dlMac; dlOld++)
;		{
	mov	cx,[dlOld]
FUD75:
	cmp	cx,[dlMac]
	jge	FUD76

;		GetPlc(hplcedl, dlOld, &edl);
	;LN_GetPlc performs GetPlc(hplcedl, cx, &edl)
	;ax, bx, cx, dx are altered.
	push	cx	;save dlOld
	call	LN_GetPlc
	pop	cx	;restore dlOld

;		if (edl.ypTop < ypTop)
;			edl.fDirty = fTrue;
;		else
;			break;
	cmp	[edl.ypTopEdl],di
	jge	FUD76
	or	[edl.fDirtyEdl],maskFDirtyEdl

;		PutPlcLast(hplcedl, dlOld, &edl);
	;LN_PutPlc performs PutPlc(hplcedl, cx, &edl);
	;ax, bx, cx, dx are altered.
	push	cx	;save dlOld
	call	LN_PutPlc
	pop	cx	;restore dlOld

;		}
	inc	cx
	jmp	short FUD75

FUD76:
;	PutIMacPlc(hplcedl, dlMac = max(dlMac, dlNew));
;	if (dlNew == dlMac)
;		PutCpPlc(hplcedl, dlNew, cp);
	mov	cx,[dlMac]
	cmp	cx,[dlNew]
;PAUSE
	jg	FUD765
	;LN_PutCpPlc performs PutCpPlc(hpcledl, dlNew, cp);
	;ax, bx, cx, dx are altered.
	call	LN_PutCpPlc
	mov	cx,[dlNew]
FUD765:
	push	cx	;save dlMac
	cCall	PutIMacPlc,<[si.hplcedlDr], cx>
	pop	cx	;restore dlMac

;	if (dlMac > 0)
;		{
	dec	cx
	jl	FUD78

;		GetPlc(hplcedl, dlMac - 1, &edl);
	;LN_GetPlc performs GetPlc(hplcedl, cx, &edl)
	;ax, bx, cx, dx are altered.
	push	cx	;save dlMac - 1
	call	LN_GetPlc
	pop	cx	;restore dlMac - 1

;		if (ypTop > ypMacOld)
;			edl.ypTop = ypTop - edl.dyp;
	cmp	di,[ypMacOld]
	jle	FUD77
	sub	di,[edl.dypEdl]
	mov	[edl.ypTopEdl],di
FUD77:

;		edl.fDirty = fTrue;
	or	[edl.fDirtyEdl],maskFDirtyEdl

;		PutPlcLast(hplcedl, dlMac - 1, &edl);
	;LN_PutPlc performs PutPlc(hplcedl, cx, &edl);
	;ax, bx, cx, dx are altered.
	call	LN_PutPlc

;		}
FUD78:

;/* advance invalid band so that update can resume after an interruption */
;	(*hwwd)->ywFirstInval = YwFromYp(hpldr, idr, ypTop);
	cCall	YwFromYp,<[hpldr], [idr], [ypTop]>
	mov	bx,[hwwd]
	mov	bx,[bx]
	mov	[bx.ywFirstInvalWwd],ax

;	Debug(vfCheckPlc = fCheckPlcSave);
ifdef DEBUG
	push	[fCheckPlcSave]
	pop	[vfCheckPlc]
endif ;DEBUG

;	fReturn = fFalse;
FUD79:
	errnz	<fFalse>
	xor	ax,ax

;LRet:
;	FreePdrf(&drfFetch);
LRet:
	push	ax	;save fReturn
	lea	ax,[drfFetch]
	push	ax
ifdef DEBUG
	cCall	S_FreePdrf,<>
else ;not DEBUG
	cCall	N_FreePdrf,<>
endif ;DEBUG
	pop	ax	;restore fReturn

;	return fReturn;
;}
cEnd


	;FUD80 sets flags in evaluating
	;pdr->lrk == lrkAbs ^ FAbsPap(pdr->doc, &vpapFetch)
	;ax, bx, cx, dx are altered.
FUD80:
	mov	ax,dataoffset [vpapFetch]
	cCall	FAbsPap,<[si.docDr],ax>
	cmp	[si.lrkDr],lrkAbs
	jne	FUD805
	xor	al,1
FUD805:
	or	ax,ax
	ret


	;FUD81 performs
	;if (cp == pdr->cpFirst)
	;	pdr->cpFirst = dx:ax;
	;cp = dx:ax;
	;ax, bx, cx, dx are altered.
FUD81:
	mov	bx,ax
	mov	cx,dx
	xchg	bx,[OFF_cp]
	xchg	cx,[SEG_cp]
	cmp	bx,[si.LO_cpFirstDr]
	jne	FUD82
	cmp	cx,[si.HI_cpFirstDr]
	jne	FUD82
	mov	[si.LO_cpFirstDr],ax
	mov	[si.HI_cpFirstDr],dx
FUD82:
	ret


	;FUD83 performs:
	;if (ypTop <= 0)
	;	{
	;	pdr->dypAbove = 0;
	;	Assert(dlNew == 0);
	;	goto LRestart
	;	}
	;Only ax is altered.
FUD83:
	xor	ax,ax
	cmp	[ypTop],ax
	jle	FUD85
	ret
FUD85:
	pop	cx	;remove return address from stack
	mov	[si.dypAboveDr],ax
;						Assert(dlNew == 0);
ifdef DEBUG
	cmp	[dlNew],0
	je	FUD86
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midDisp2n
	mov	bx,1005
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FUD86:
endif ;DEBUG

;						goto LRestart;
	jmp	LRestart


	;FUD87 performs
	;FreeEdls(hplcedl, max(dlOld,dlNew), dlMac);
	;PutIMacPlc(hplcedl, dlNew);
	;PutCpPlc(hplcedl, dlNew, cp);
	;ax, bx, cx, dx are altered.
FUD87:
	mov	ax,[dlOld]
	cmp	ax,[dlNew]
	jge	FUD875
	mov	ax,[dlNew]
FUD875:
	cCall	FreeEdls,<[si.hplcedlDr], ax, [dlMac]>
	cCall	PutIMacPlc,<[si.hplcedlDr],[dlNew]>
	;LN_PutCpPlc performs PutCpPlc(hpcledl, dlNew, cp);
	;ax, bx, cx, dx are altered.
	call	LN_PutCpPlc
	ret

;				Assert(fPageView);
ifdef DEBUG
FUD88:
	test	bptr ([bFlagsWwd]),maskFPageViewWwd
	jne	FUD89
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midDisp2n
	mov	bx,1006
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FUD89:
	ret
endif ;DEBUG


;			Debug(fInTable = FInTableVPapFetch(doc, cp));
;			Assert ( udmod != udmodTable || fInTable );
;#define FInTableVPapFetch(doc, cp)  (vpapFetch.fInTable && FInTableDocCp(doc, cp))
;			/* pdr->fInTable should be set iff udmod == udmodTable */
;			Assert (pdr->fInTable == (udmod == udmodTable));
ifdef DEBUG
FUD90:
	push	ax
	errnz	<fFalse>
	xor	ax,ax
	cmp	[vpapFetch.fInTablePap],al
	je	FUD91
ifdef DEBUG
	cCall	S_FInTableDocCp,<[si.docDr], [SEG_cp], [OFF_cp]>
else ;not DEBUG
	cCall	N_FInTableDocCp,<[si.docDr], [SEG_cp], [OFF_cp]>
endif ;DEBUG
FUD91:
	cmp	[udmod],udmodTable
	jne	FUD92
	or	ax,ax
	jne	FUD92
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midDisp2n
	mov	bx,1007
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FUD92:
	mov	ax,[udmod]
	sub	ax,udmodTable
	sub	ax,1
	mov	al,0
	sbb	al,0
	;Now al is -1 if udod == udmodTable, 0 otherwise
	xor	al,[si.fInTableDr]
	and	al,maskFInTableDr
	pop	ax
	je	FUD93
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midDisp2n
	mov	bx,1008
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FUD93:
	ret
endif ;DEBUG


ifdef DEBUG
FUD94:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,5
	cCall	FRareProc,<ax>
	or	ax,ax
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	je	FUD95
	mov	ax,fTrue
FUD95:
	ret
endif ;/* DEBUG */

	;FUD96 performs
	;pdr->xwLimScroll = max( pdr->xwLimScroll,
	;	XwFromXp( hpldr, idr, edl.xpLeft + edl.dxp ) );
	;ax, bx, cx, dx are altered.
FUD96:
	mov	ax,[edl.xpLeftEdl]
	add	ax,[edl.dxpEdl]
	cCall	XwFromXp,<[hpldr],[idr],ax>
	cmp	[si.xwLimScrollDr],ax
	jg	FUD97
	mov	[si.xwLimScrollDr],ax
FUD97:
	ret

FUD98:
; performs: special tests for a DR which gets exactly filled
;
; int ch;
; /* we have to allow a splat at the beginning
; of a para to be tacked on even if no room */
; CachePara(doc, cp);
;		CachePara(doc, cp);
;PAUSE
ifdef DEBUG
	cCall	S_CachePara,<[si.docDr],[SEG_cp],[OFF_cp]>
else ;not DEBUG
	cCall	N_CachePara,<[si.docDr],[SEG_cp],[OFF_cp]>
endif ;DEBUG
; if (cp != caPara.cpFirst)
;   break;
	mov	dx,[SEG_cp]
	mov	ax,[OFF_cp]
	sub	ax,[caPara.LO_cpFirstCa]
	sbb	dx,[caPara.HI_cpFirstCa]
	or	ax,dx
	jne	FUD985
; CacheSect(doc, cp);
;PAUSE
	stc
FUD981:
	push	[si.docDr]
	push	[SEG_cp]
	push	[OFF_cp]
	jnc	FUD9815
	mov	ax,dataoffset [caSect]
	push	ax
	cCall	FInCa,<>
	or	ax,ax
	jne	FUD982
	jmp	short FUD981
FUD9815:
	cCall	CacheSectProc,<>
FUD982:
; if (cp == caSect.cpLim - ccpSect ||
	mov	dx,[SEG_cp]
	mov	cx,dx
	mov	ax,[OFF_cp]
	mov	bx,ax
	add	bx,ccpSect
	adc	cx,0
	sub	bx,[caSect.LO_cpLimCa]
	sbb	cx,[caSect.HI_cpLimCa]
	or	cx,bx
	jz	FUD985
;PAUSE
;   (ch = ChFetch(doc, cp, fcmChars)) != chSect && ch != chColumnBreak)
;   {
;   break;
;   }
	push	[si.docDr]
	push	dx
	push	ax
	mov	ax,fcmChars
	push	ax
	cCall	ChFetch,<>
	cmp	al,chSect
	je	FUD983
;PAUSE
	cmp	al,chColumnBreak
	jne	FUD985
FUD983:
	ret
FUD985:
;PAUSE
	pop	ax	; remove unneeded return address
	jmp	LEndDr

;-------------------------------------------------------------------------
;	IMacPlc( hplcedl )
;-------------------------------------------------------------------------
	;LN_IMacPlc performs dlMac = IMacPlc(hplcedl);	ax, bx are altered.
LN_IMacPlc:
	;***Begin in line IMacPlc
;		return (*hplc)->iMac;
	mov	bx,[si.hplcedlDr]
	mov	bx,[bx]
	mov	ax,[bx.iMacPlcSTR]
	;***End in line IMacPlc
	mov	[dlMac],ax
	ret


;-------------------------------------------------------------------------
;	GetPlc( hplcedl, iedl, &edl )
;-------------------------------------------------------------------------
	;LN_GetPlc performs GetPlc(hplcedl, cx, &edl)
	;ax, bx, cx, dx are altered.
LN_GetPlc:
	lea	ax,[edl]
	cCall	GetPlc,<[si.hplcedlDr],cx,ax>
	ret


;-------------------------------------------------------------------------
;	PdodDoc( doc )
;-------------------------------------------------------------------------
LN_PdodDoc:
	;Takes doc in bx, result in bx.  Only bx is altered.
	shl	bx,1
	mov	bx,[bx.mpdochdod]
ifdef DEBUG
	cmp	bx,hNil
	jne	PD01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midDisp2n
	mov	bx,1010
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
PD01:
endif ;/* DEBUG */
	mov	bx,[bx]
	ret


;-------------------------------------------------------------------------
;	CpPlcEdl( hplcedl, dlOld )
;-------------------------------------------------------------------------
	;LN_CpPlcEdl performs CpPlc(hpcledl, dlOld); ax, bx, cx, dx are altered.
LN_CpPlcEdl:
	cCall	CpPlc,<[si.hplcedlDr],[dlOld]>
	ret


;-------------------------------------------------------------------------
;	PutCpPlc( hplcedl, dlNew, cp )
;-------------------------------------------------------------------------
	;LN_PutCpPlc performs PutCpPlc(hpcledl, dlNew, cp);
	;ax, bx, cx, dx are altered.
LN_PutCpPlc:
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	;LN_PutCpPlcReg performs PutCpPlc(hpcledl, dlNew, dx:ax);
	;ax, bx, cx, dx are altered.
LN_PutCpPlcReg:
	cCall	PutCpPlc,<[si.hplcedlDr],[dlNew],dx,ax>
	ret


;-------------------------------------------------------------------------
;	PutPlc( hplcedl, dlNew, &edl )
;-------------------------------------------------------------------------
	;LN_PutPlcNew performs PutPlc(hplcedl, dlNew, &edl);
	;ax, bx, cx, dx are altered.
LN_PutPlcNew:
	mov	cx,[dlNew]
	;LN_PutPlc performs PutPlc(hplcedl, cx, &edl);
	;ax, bx, cx, dx are altered.
LN_PutPlc:
	lea	ax,[edl]
	cCall	PutPlc,<[si.hplcedlDr],cx,ax>
	ret


;-------------------------------------------------------------------------
;	DrawEndMark( ww, hpldr, ypTop, ypMac, emk )
;-------------------------------------------------------------------------
	;LN_DrawEndmark performs DrawEndmark(ww, hpldr, idr, ypTop, ax, bx);
	;ax, bx, cx, dx are altered.
LN_DrawEndMark:
	cCall	DrawEndmark,<[ww], [hpldr], [idr], [ypTop], ax, bx>
	ret


ifdef DEBUG
;  Debug( vdbs.fShakeHeap ? ShakeHeap() : 0 );
LN_ShakeHeap:
        cmp     [vdbs.fShakeHeapDbs],0
	jz	LN_SH01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,1
        cCall   ShakeHeapSb,<ax>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LN_SH01:
	ret
endif ;DEBUG


;-------------------------------------------------------------------------
;	ScrollDrUp(ww, hpldr, idr, hplcedl, dlFrom, dlTo, ypFrom, ypTo, ypFirstShow, ypLimShow)
;-------------------------------------------------------------------------
;/* S C R O L L  D R  U P */
;/* ypLimShow needs to define visibility limits, not necessarily dr limits */
;NATIVE ScrollDrUp(ww, hpldr, idr, hplcedl, dlFrom, dlTo, ypFrom, ypTo, ypFirstShow, ypLimShow)
;struct PLDR **hpldr;
;struct PLCEDL **hplcedl;
;{
;	int ypLim, dypChange;
;	int dl, dlMac;
;	struct EDL edl;

; %%Function:N_ScrollDrUp %%Owner:BRADV
cProc	N_ScrollDrUp,<PUBLIC,FAR>,<si,di>
	ParmW	<ww>
	ParmW	<hpldr>
	ParmW	<idr>
	ParmW	<hplcedl>
	ParmW	<dlFrom>
	ParmW	<dlTo>
	ParmW	<ypFrom>
	ParmW	<ypTo>
	ParmW	<ypFirstShow>
	ParmW	<ypLimShow>

	LocalW	<dlMac>
	LocalW	<dypChange>
	LocalW	<dlMax>
	LocalW	<ypLim>

cBegin

;	Assert(dlTo <= dlFrom);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[dlTo]
	cmp	ax,[dlFrom]
	jle	SDU01
	mov	ax,midDisp2n
	mov	bx,1011
	cCall	AssertProcForNative,<ax,bx>
SDU01:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

	mov	si,[hplcedl]

;	ypLim = ypFrom;
;	dypChange = ypTo - ypFrom;
	mov	ax,[ypFrom]
	mov	[ypLim],ax
	mov	cx,[ypTo]
	sub	cx,ax
	mov	[dypChange],cx

;	dlMac = IMacPlc(hplcedl);
	;***Begin in line IMacPlc
;		return (*hplc)->iMac;
	mov	bx,[si]
	mov	ax,[bx.iMacPlcSTR]
	;***End in line IMacPlc
	mov	[dlMac],ax

;	FreeEdls(hplcedl, dlTo, dlFrom);
;	PutIMacPlc(hplcedl, dlMac + dlTo - dlFrom);
	mov	cx,[dlTo]
	mov	dx,[dlFrom]
	add	ax,cx
	sub	ax,dx
	push	si	    ;argument for PutIMacPlc
	push	ax	    ;argument for PutIMacPlc
	cCall	FreeEdls,<si, cx, dx>
	cCall	PutIMacPlc,<>
	mov	bx,[si]
	push	[bx.iMaxPlc]

;	for (dl = dlFrom; dl < dlMac; dl++)
;		{
;		PutCpPlc(hplcedl, dlTo, CpPlc(hplcedl, dl));
;		GetPlc(hplcedl, dl, &edl);

;		if (edl.fEnd)
;			{
;			int ypT = edl.ypTop - dypChange + 10 * vsci.dypBorder;
;
;			if (ypT <= ypLimShow)
;/* endmark now showing is tall enough so that scrolling it will do all blanking */
;/* we can accomplish all necessary blanking by scrolling the endmark */
;				ypLim = ypT;
;			else			  
;/* can't do any good by scrolling it, so dirty it */
;				edl.fDirty = fTrue;
;			}
;		else if (edl.ypTop + edl.dyp <= ypLimShow)
;			ypLim = edl.ypTop + edl.dyp;
;		else if (edl.ypTop + dypChange < ypLimShow)
;			edl.fDirty = fTrue;
;		edl.ypTop += dypChange;
;		if (edl.hpldr != hNil)
;			(*((struct PLDR **)edl.hpldr))->ptOrigin.yp = edl.ypTop;
;		PutPlc(hplcedl, dlTo++, &edl);
;		}
;	PutCpPlc(hplcedl, dlTo, CpPlc(hplcedl, dl));
	;***Begin in-line plcedl movement
	lea	di,[bx.rgcpPlc]
	push	ds
	pop	es
	test	[bx.fExternalPlc],maskFExternalPlc
	je	SDU03
	mov	bx,whi ([di])
	mov	di,wlo ([di])
	shl	bx,1
	mov	ax,mpsbps[bx]
	mov	es,ax
	shr	ax,1
	jc	SDU02
	cCall	ReloadSb,<>
SDU02:
	mov	di,es:[di]
SDU03:
ifdef DEBUG
	mov	ax,[wFillBlock]
	mov	bx,[wFillBlock]
	mov	cx,[wFillBlock]
	mov	dx,[wFillBlock]
endif ;DEBUG
	;es:di now points to rgcp

	push	es
	pop	ds
	push	di	;save low lprgcp
	mov	cx,[dlMac]
	inc	cx
	mov	bx,[dlFrom]
	sub	cx,bx
	shl	bx,1
	shl	bx,1
	lea	si,[bx+di]
	mov	bx,[dlTo]
	shl	bx,1
	shl	bx,1
	add	di,bx
	shl	cx,1
	rep	movsw	    ;move the CP's
	pop	di	;restore low rgcp

	pop	bx	;restore dlMax
	shl	bx,1
	shl	bx,1
	mov	ax,cbEdlMin
	mul	[dlFrom]
	lea	si,[bx+di]
	add	si,ax
	mov	ax,cbEdlMin
	mul	[dlTo]
	add	di,bx
	add	di,ax
	mov	dx,[dlFrom]
SDU04:
	cmp	dx,[dlMac]
	jge	SDU08
	errnz	<cbEdlMin AND 1>
	mov	cx,cbEdlMin SHR 1
	rep	movsw

	mov	ax,[di.ypTopEdl - cbEdlMin]
	mov	cx,ax
	add	ax,[di.dypEdl - cbEdlMin]
	add	cx,[dypChange]

;		if (edl.fEnd)
;			{
	test	[di.fEndEdl - cbEdlMin],maskFEndEdl
	je	SDU05

;			int ypT = edl.ypTop - dypChange + 10 * vsci.dypBorder;
	push	dx	;save dl
	mov	ax,10
	mul	ss:[vsci.dypBorderSci]
	pop	dx	;restore dl
	add	ax,[di.ypTopEdl - cbEdlMin]
	sub	ax,[dypChange]

;			if (ypT <= ypLimShow)
;/* endmark now showing is tall enough so that scrolling it will do all blanking */
;/* we can accomplish all necessary blanking by scrolling the endmark */
;				ypLim = ypT;
;			else			  
;/* can't do any good by scrolling it, so dirty it */
;				edl.fDirty = fTrue;
;			}
	cmp	ax,[ypLimShow]
	jg	SDU055
SDU045:
	mov	[ypLim],ax
	jmp	short SDU06
SDU05:

;		else if (edl.ypTop + edl.dyp <= ypLimShow)
;			ypLim = edl.ypTop + edl.dyp;
	cmp	ax,[ypLimShow]
	jle	SDU045

;		else if (edl.ypTop + dypChange < ypLimShow)
;			edl.fDirty = fTrue;
	cmp	cx,[ypLimShow]
	jge	SDU06
SDU055:
	or	[di.fDirtyEdl - cbEdlMin],maskFDirtyEdl
SDU06:

;		edl.ypTop += dypChange;
	mov	[di.ypTopEdl - cbEdlMin],cx

;		if (edl.hpldr != hNil)
;			(*((struct PLDR **)edl.hpldr))->ptOrigin.yp = edl.ypTop;
	mov	bx,[di.hpldrEdl - cbEdlMin]
	errnz	<hNil - 0>
	or	bx,bx
	je	SDU07
	mov	bx,ss:[bx]
	mov	ss:[bx.ptOriginPldr.ypPt],cx
SDU07:
	inc	dx
	jmp	short SDU04
SDU08:
	push	ss
	pop	ds
	;***End in-line plcedl movement

;	Assert(ypLim >= ypFrom);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[ypLim]
	cmp	ax,[ypFrom]
	jge	SDU09
	mov	ax,midDisp2n
	mov	bx,1012
	cCall	AssertProcForNative,<ax,bx>
SDU09:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;	Win(dypCS = dypChange);
	mov	ax,[dypChange]
	mov	[dypCS],ax

;	if (dypChange != 0)
;		{
	or	ax,ax
	je	SDU11

;		if (ypTo < ypFirstShow)
;			{
;			ypFrom += ypFirstShow - ypTo;
;			ypTo = ypFirstShow;
;			}
	mov	ax,[ypFrom]
	mov	bx,[ypTo]
	mov	dx,[ypFirstShow]
	sub	dx,bx
	jle	SDU10
	add	ax,dx
	add	bx,dx
SDU10:

;		ScrollWw(ww, hpldr, idr, ypFrom, ypTo, ypLim - ypFrom);
	mov	cx,[ypLim]
	sub	cx,ax
	cCall	ScrollWw,<[ww], [hpldr], [idr], ax, bx, cx>

;		}
SDU11:

;}
cEnd

sEnd	disp2

        end
