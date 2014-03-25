	include w2.inc
	include noxport.inc
        include consts.inc
	include structs.inc

createSeg	disptbl_PCODE,disptbn2,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midDisptbn2	equ 31		; module ID, for native asserts
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
externFP	<CacheTc>
externFP	<ClearRclInParentDr>
externFP	<CpFirstTap>
externFP	<CpFirstTap1>
externFP	<CpMacDocEdit>
externFP	<CpMacPlc>
externFP	<CpMin>
externFP	<CpPlc>
externFP	<DlkFromVfli>
externFP	<DrawEndMark>
externFP	<DrawTableRevBar>
externFP	<DrawStyNameFromWwDL>
externFP	<DrcToRc>
externFP	<DrclToRcw>
externFP	<FEmptyRc>
externFP	<FillRect>
externFP	<FInCa>
externFP	<FInitHplcedl>
externFP	<FInsertInPl>
externFP	<FMatchAbs>
externFP	<FrameTable>
externFP	<FreeDrs>
externFP	<FreeEdl>
externFP	<FreeEdls>
externFP	<FreePpv>
externFP	<FreeHpl>
externFP	<FreeHq>
externFP	<FShowOutline>
externFP	<GetPlc>
externFP	<HplInit2>
externFP	<IMacPlc>
externFP	<N_PdodMother>
externFP	<NMultDiv>
externFP	<PatBltRc>
externFP	<PutCpPlc>
externFP	<PutIMacPlc>
externFP	<PutPlc>
externFP	<N_PwwdWw>
externFP	<RcwPgvTableClip>
externFP	<ScrollDrDown>
externFP	<FSectRc>
externFP	<XwFromXl>
externFP	<XwFromXp>
externFP	<YwFromYl>
externFP	<YwFromYp>
externFP	<SetErrorMatProc>

ifdef	DEBUG
externFP	<AssertProcForNative>
externFP	<DypHeightTc>
externFP	<PutPlcLastDebugProc>
externFP	<S_CachePara>
externFP	<S_DisplayFli>
externFP	<S_FInTableDocCp>
externFP	<S_FormatDrLine>
externFP	<S_FreePdrf>
externFP	<S_FUpdateDr>
externFP	<S_ItcGetTcxCache>
externFP	<S_PdrFetch>
externFP	<S_PdrFetchAndFree>
externFP	<S_PdrFreeAndFetch>
externFP	<S_WidthHeightFromBrc>
externFP	<S_FUpdTableDr>
else	; !DEBUG
externFP	<N_CachePara>
externFP	<N_DisplayFli>
externFP	<N_FInTableDocCp>
externFP	<N_FormatDrLine>
externFP	<N_FreePdrf>
externFP	<N_FUpdateDr>
externFP	<N_ItcGetTcxCache>
externFP	<N_PdrFetch>
externFP	<N_PdrFetchAndFree>
externFP	<N_PdrFreeAndFetch>
externFP	<N_WidthHeightFromBrc>
externFP	<PutPlcLastProc>
endif	; DEBUG

sBegin	data

; 
; /* E X T E R N A L S */
; 
externW	caTap
externW dxpPenBrc
externW dypPenBrc
externW hbrPenBrc
externW vfmtss
externW	vfli
externW	vfti
externW vhbrGray
externW	vihpldrMac
externW vmerr
externW	vpapFetch
externW	vrghpldr
externW vsci
externW	vtapFetch
externW vtcc

sEnd    data

; CODE SEGMENT _DISPTBN2

sBegin	disptbn2
	assumes cs,disptbn2
        assumes ds,dgroup
        assumes ss,dgroup

; /*
; /* F  U P D A T E  T A B L E
; /*
; /*  Description: Given the cp beginning a table row, format and display
; /*    that table row. Returns fTrue iff the format was successful, with
; /*    cp and ypTop correctly advanced. If format fails, cp and ypTop
; /*    are left with their original values.
; /**/
; NATIVE FUpdateTable ( ww, doc, hpldr, idr, pcp, pypTop, hplcedl, dlNew, dlOld, dlMac,
; ypFirstShow, ypLimWw, ypLimDr, rcwInval, fScrollOK )
; int ww, doc, idr, dlNew, dlOld, dlMac;
; int ypFirstShow, ypLimWw, ypLimDr, fScrollOK;
; struct PLCEDL **hplcedl;
; struct PLDR **hpldr;
; CP *pcp;
; int *pypTop;
; struct RC rcwInval;
; {
;
; NATIVE NOTE, USE OF REGISTERS: Whenever there is a dr currently fetched,
; or recently FetchedAndFree'd, the pdr is kept in SI. After the early
; set-up code (see LUpdateTable), DI is used to store &drfFetch. This
; is used in scattered places through the code and should not be disturbed
; carelessly. The little helper routines also assume these uses.
;
; %%Function:FUpdateTable %%Owner:tomsax
cProc	N_FUpdateTable,<PUBLIC,FAR>,<si,di>
	ParmW	ww
	ParmW	doc
	ParmW	hpldr
	ParmW	idr
	ParmW	pcp
	ParmW	pypTop
	ParmW	hplcedl
	ParmW	dlNew
	ParmW	dlOld
	ParmW	dlMac
	ParmW	ypFirstShow
	ParmW	ypLimWw
	ParmW	ypLimDr
	ParmW	rcwInvalYwBottomRc
	ParmW	rcwInvalXwRightRc
	ParmW	rcwInvalYwTopRc
	ParmW	rcwInvalXwLeftRc
	ParmW	fScrollOK
; int idrTable, idrMacTable, itcMac;
	LocalW	<idrTable,idrMacTable>
	LocalW	itcMac
; int dylOld, dylNew, dylDr, dylDrOld;
	LocalW	dylOld
	LocalW	dylNew
	LocalW	dylDrOld
	; native note dylDr kept in register when need
; int dylAbove, dylBelow, dylLimPldr, dylLimDr;
	LocalW	dylAbove
	LocalW	dylBelow
	LocalW	dylLimPldr
	LocalW	dylLimDr
; int dylBottomFrameDirty = 0;
	LocalW	dylBottomFrameDirty
; int dyaRowHeight, ypTop, ylT; native note: ylT registerized
	LocalW	<dyaRowHeight, ypTop>
; Mac(int cbBmbSav);
; int dlLast, dlMacOld, lrk;
	LocalW	dlLast
	LocalW	dlMacOld
	LocalB	lrk	; byte-size makes life easier
; BOOL fIncr, fTtpDr, fReusePldr, fFrameLines, fOverflowDrs;
	LocalB	fIncr
	LocalB	fTtpDr
	LocalB	fReusePldr
	LocalB	fFrameLines
	LocalB	fOverflowDrs
; BOOL fSomeDrDirty, fLastDlDirty, fLastRow, fFirstRow, fAbsHgtRow;
	LocalB	fSomeDrDirty
	LocalB	fLastDlDirty
	LocalB	fLastRow
	LocalB	fFirstRow
	LocalB	fAbsHgtRow
; BOOL fOutline, fPageView, fDrawBottom;
	LocalB	fOutline
	LocalB	fPageView
	LocalB	fDrawBottom
; Win(BOOL fRMark;)
	LocalB	fRMark
; CP cp = *pcp;
	LocalD	cp
; struct WWD *pwwd;	; native note: registerized
	LocalW	pdrTable
; struct DR *pdrT, *pdrTable;	; native note: registerized
; struct PLDR **hpldrTable, *ppldrTable;
	LocalW	hpldrTable
	; LocalW	ppldrTable -- registerized
; struct RC rcwTableInval;
	LocalV	rcwTableInval,cbRcMin
; struct CA caTapCur;
	LocalV	caTapCur,cbCaMin
; struct EDL edl, edlLast, edlNext;
	LocalV	edl,cbEdlMin
	LocalV	edlLast,cbEdlMin
	LocalV	edlNext,cbEdlMin
; struct DRF drfT,drfFetch;
	LocalV	drfFetch,cbDrfMin
	LocalV	drfT,cbDrfMin
; struct TCX tcx;
	LocalV	tcx,cbTcxMin
; struct PAP papT;
	LocalV	papT,cbPapMin
; this trick works on the assumption that *pypTop is not altered
; until the end of the routine.
cBegin

;PAUSE

; ypTop = *pypTop; assume & assert *pypTop doesn't change until the end.
	mov	bx,[pypTop]
	mov	ax,[bx]
	mov	[ypTop],ax

; dylBottomFrameDirty = Win(fRMark =) 0;
;PAUSE
	xor	ax,ax
	mov	[dylBottomFrameDirty],ax
	mov	[fRMark],al
; cp = *pcp;
	mov	si,pcp
	mov	ax,[si+2]
	mov	[SEG_cp],ax
	mov	ax,[si]
	mov	[OFF_cp],ax
; pwwd = PwwdWw(ww);
	call	LN_PwwdWw
; fOutline = pwwd->fOutline;
	; ax = pwwd
	xchg	ax,di	; move to a more convenient register
	mov	al,[di.fOutlineWwd]
	and	al,maskFOutlineWwd
	mov	[fOutline],al
; fPageView = pwwd->fPageView;
	mov	al,[di.fPageViewWwd]
	and	al,maskFPageViewWwd
	mov	[fPageView],al

; CacheTc(ww,doc,cp,fFalse,fFalse);	/* Call this before CpFirstTap for efficiency */
	push	[ww]
	push	[doc]
	push	[SEG_cp]
	push	[OFF_cp]
	xor	ax,ax
	push	ax
	push	ax
	cCall	CacheTc,<>
; CpFirstTap1(doc, cp, fOutline);
;PAUSE
	push	[doc]
	push	[SEG_cp]
	push	[OFF_cp]
	mov	al,[fOutline]
	cbw
	push	ax
	cCall	CpFirstTap1,<>
; Assert(cp == caTap.cpFirst);
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	sub	ax,[caTap.LO_cpFirstCa]
	sbb	dx,[caTap.HI_cpFirstCa]
	or	ax,dx
	je	UT001
	mov	ax,midDisptbn2
	mov	bx,303
	cCall	AssertProcForNative,<ax,bx>
UT001:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG
; caTapCur = caTap;
	mov	si,dataoffset [caTap]
	lea	di,[caTapCur]
	push	ds
	pop	es
	errnz	<cbCaMin-10>
	movsw
	movsw
	movsw
	movsw
	movsw
; itcMac = vtapFetch.itcMac;
	mov	ax,[vtapFetch.itcMacTap]
	mov	[itcMac],ax
; fAbsHgtRow = (dyaRowHeight = vtapFetch.dyaRowHeight) < 0;
;PAUSE
	mov	cx,[vtapFetch.dyaRowHeightTap]
	mov	[dyaRowHeight],cx
	xor	ax,ax ; assume correct value is zero
	or	cx,cx
	jge	UT010
	errnz	<fTrue-1>
;PAUSE
	inc	ax
UT010:
	mov	[fAbsHgtRow],al
; Assert ( FInCa(doc,cp,&caTapCur) );
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	push	[doc]
	push	[SEG_cp]
	push	[OFF_cp]
	lea	ax,[caTapCur]
	push	ax
	cCall	FInCa,<>
	or	ax,ax
	jnz	UT020

	mov	ax,midDisptbn2
	mov	bx,169
	cCall	AssertProcForNative,<ax,bx>
UT020:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG

; pdrT = PdrFetchAndFree(hpldr, idr, &drfT);
; native note: this doesn't count as setting up di yet,
; we'll need di for some other things first...
	lea	di,[drfT]

	push	[hpldr]
	push	[idr]
	push	di
ifdef	DEBUG
	cCall	S_PdrFetchAndFree,<>
else
	cCall	N_PdrFetchAndFree,<>
endif
	xchg	ax,si
; lrk = pdrT->lrk;
	; si = pdrT
	mov	al,[si.lrkDr]
	mov	[lrk],al
; DrclToRcw(hpldr,&pdrT->drcl,&rcwTableInval);
	; si = pdrT
	push	[hpldr]
	errnz	<drclDr>
	push	si
	lea	ax,[rcwTableInval]
	push	ax
	cCall	DrclToRcw,<>

; /* check to see if we need to force a first row or last row condition */
; fFirstRow = fLastRow = fFalse;	/* assume no override */
	; si = pdrT
	xor	ax,ax
	mov	[fFirstRow],al
	mov	[fLastRow],al
; if (fPageView)
	; ax = 0, si = pdrT
	cmp	[fPageView],al
	jnz	UT025
	jmp	LChkOutline
; {
; if (pdrT->fForceFirstRow)
; {
UT025:
;PAUSE
	test	[si.fForceFirstRowDr],maskfForceFirstRowDr
	jz	UT060
; if (caTap.cpFirst == pdrT->cpFirst || dlNew == 0)
;   fFirstRow = fTrue;
;PAUSE
	; ax = 0, si = pdrT
	cmp	[dlNew],ax
	jz	UT050
;PAUSE
	mov	cx,[caTap.LO_cpFirstCa]
	cmp	cx,[si.LO_cpFirstDr]
	jnz	UT040
;PAUSE
	mov	cx,[caTap.HI_cpFirstCa]
	cmp	cx,[si.HI_cpFirstDr]
	jz	UT050
; else
; {
UT040:
; Assert(dlNew > 0);
; dlLast = dlNew - 1;
; GetPlc(hplcedl,dlLast,&edlLast);
;PAUSE
	lea	ax,[edlLast]
	mov	cx,[dlNew]
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	or	cx,cx
	jg	UT005
	mov	ax,midDisptbn2
	mov	bx,1001
	cCall	AssertProcForNative,<ax,bx>
UT005:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG
	dec	cx
	mov	[dlLast],cx
	call	LN_GetPlcParent
; if (caTapCur.cpFirst != CpPlc(hplcedl,dlLast) + edlLast.dcp)
;   fFirstRow = fTrue;
	push	[hplcedl]
	push	[dlLast]
	cCall	CpPlc,<>
	add	ax,[edlLast.LO_dcpEdl]
	adc	dx,[edlLast.HI_dcpEdl]
	sub	ax,[caTapCur.LO_cpFirstCa]	; sub to re-zero ax
	sbb	dx,[caTapCur.HI_cpFirstCa]
	or	ax,dx
	je	UT060
UT050:
;PAUSE
	mov	[fFirstRow],fTrue
	xor	ax,ax
; }
; }
UT060:
; if (pdrT->cpLim != cpNil && pdrT->cpLim <= caTap.cpLim)
; {
	; ax = 0, si = pdrT
	errnz	<LO_cpNil+1>
	errnz	<HI_cpNil+1>
	mov	cx,[si.LO_cpLimDr]
	and	cx,[si.HI_cpLimDr]
	inc	cx
	jz	UT062	; to the else clause
	mov	ax,[caTap.LO_cpLimCa]
	mov	dx,[caTap.HI_cpLimCa]
	sub	ax,[si.LO_cpLimDr]
	sbb	dx,[si.HI_cpLimDr]
	js	UT062	; to the else clause
; if (pdrT->idrFlow == idrNil)
;   fLastRow = fTrue;
;PAUSE
	cmp	[si.idrFlowDr],0
	js	UT065	; set fLastRow = fTrue, or fall through for else
; else
;   {
;   /* use the pdrTable and drfFetch momentarily... */
;   pdrTable = PdrFetchAndFree(hpldr, pdrT->idrFlow, &drfFetch);
; native note: this doesn't count as setting up di yet,
; we'll need di for some other things first...
;PAUSE
	lea	di,[drfFetch]
	push	[hpldr]
	push	[si.idrFlowDr]
	push	di
ifdef	DEBUG
	cCall	S_PdrFetchAndFree,<>
else
	cCall	N_PdrFetchAndFree,<>
endif
	xchg	ax,bx
;   Assert(pdrT->doc == pdrTable->doc);
ifdef	DEBUG
;Did this DEBUG stuff with a call so as not to mess up short jumps.
	call	UT2130
endif	; DEBUG
;   fLastRow = pdrT->xl != pdrTable->xl;
	mov	cx,[si.xlDr]
	cmp	cx,[bx.xlDr]
	jne	UT065	; set fLastRow = fTrue, or fall through for else
;PAUSE
	jmp	short UT070
;   }
; }
; else
;   {
;   if (FInTableDocCp(doc,caTapCur.cpLim))
UT062:
;PAUSE
	push	[doc]
	push	[caTapCur.HI_cpLimCa]
	push	[caTapCur.LO_cpLimCa]
ifdef	DEBUG
	cCall	S_FInTableDocCp,<>
else
	cCall	N_FInTableDocCp,<>
endif
	or	ax,ax
	jz	UT070
;   {
;   CachePara(doc, caTapCur.cpLim);
;PAUSE
	push	[doc]
	push	[caTapCur.HI_cpLimCa]
	push	[caTapCur.LO_cpLimCa]
ifdef	DEBUG
	cCall	S_CachePara,<>
else
	cCall	N_CachePara,<>
endif
;   papT = vpapFetch;
	push	si	; save pdrT
	mov	si,dataoffset [vpapFetch]
	lea	di,[papT]
	push	ds
	pop	es
	errnz	<cbPapMin and 1>
	mov	cx,cbPapMin SHR 1
	rep	movsw
	pop	si	; restore pdrT
;   CachePara(doc, cp);
	push	[doc]
	push	[SEG_cp]
	push	[OFF_cp]
ifdef	DEBUG
	cCall	S_CachePara,<>
else
	cCall	N_CachePara,<>
endif
;   if (!FMatchAbs(caPara.doc, &papT, &vpapFetch))
	push	[doc]
	lea	ax,[papT]
	push	ax
	mov	ax,dataoffset [vpapFetch]
	push	ax
	cCall	FMatchAbs,<>
	or	ax,ax
	jnz	UT070
;     fLastRow = fTrue;
;PAUSE
UT065:
;PAUSE
	mov	[fLastRow],fTrue
;   }
; }
UT070:
; }
; native note: end of if fPageView clause
; else if (pwwd->fOutline)
; native note: use fOutline instead
; REVIEW - C should also use fOutline
LChkOutline:
	; si = pdrT
	cmp	[fOutline],0
	jz	LChkOverride
; {
; if (!FShowOutline(doc, CpMax(caTap.cpFirst - 1, cp0)))
;   fFirstRow = fTrue;
;PAUSE
	mov	ax,[caTap.LO_cpFirstCa]
	mov	dx,[caTap.HI_cpFirstCa]
	sub	ax,1
	sbb	dx,0
	jns	UT075
	xor	ax,ax
	cwd
UT075:
	push	[doc]
	push	dx	; SEG
	push	ax	; OFF
	cCall	FShowOutline,<>
	or	ax,ax
	jnz	UT080
;PAUSE
	mov	[fFirstRow],fTrue
UT080:
; if (!FShowOutline(doc, CpMin(caTap.cpLim, CpMacDocEdit(doc))))
;   fLastRow = fTrue;
	push	[doc]
	cCall	CpMacDocEdit,<>
	push	dx	; SEG
	push	ax	; OFF
	push	[caTap.HI_cpLimCa]
	push	[caTap.LO_cpLimCa]
	cCall	CpMin,<>
	push	[doc]
	push	dx	; SEG
	push	ax	; OFF
	cCall	FShowOutline,<>
	or	ax,ax
	jnz	UT085
;PAUSE
	mov	[fLastRow],fTrue
UT085:
; }

LChkOverride:
; /* Rebuild the cache if we need to override. */
; if ((fFirstRow && !vtcc.fFirstRow) || (fLastRow && !vtcc.fLastRow))
;   CacheTc(ww,doc,cp,fFirstRow,fLastRow);
	; si = pdrT
	xor	ax,ax	; a zero register will be handy
	cmp	[fFirstRow],al
	jz	UT090
	test	[vtcc.fFirstRowTcc],maskFFirstRowTcc
	jz	UT100
UT090:
	cmp	[fLastRow],al
	jz	UT110
	test	[vtcc.fLastRowTcc],maskFLastRowTcc
	jnz	UT110
UT100:
;PAUSE
	; ax = 0
	push	[ww]
	push	[doc]
	push	[SEG_cp]
	push	[OFF_cp]
	mov	al,[fFirstRow]
	push	ax
	mov	al,[fLastRow]
	push	ax
	cCall	CacheTc,<>
UT110:
; fFirstRow = vtcc.fFirstRow;
; fLastRow = vtcc.fLastRow;
	errnz	<fFirstRowTcc-fLastRowTcc>
	mov	al,[vtcc.fFirstRowTcc]
	push	ax
	and	al,maskFFirstRowTcc
	mov	[fFirstRow],al
	pop	ax
	and	al,maskFLastRowTcc
	mov	[fLastRow],al
; dylAbove = vtcc.dylAbove;
;PAUSE
	mov	ax,[vtcc.dylAboveTcc]
	mov	[dylAbove],ax
; dylBelow = vtcc.dylBelow;
	mov	ax,[vtcc.dylBelowTcc]
	mov	[dylBelow],ax

; /* NOTE: The height available for an non-incremental update is extended
; /* past the bottom of the bounding DR so that the bottom border
; /* or frame line will not be shown if the row over flows.
; /**/
	; si = pdrT
; fFrameLines = FDrawTableDrsWw(ww);
; dylLimPldr = pdrT->dyl - *pypTop + dylBelow;
;PAUSE
	mov	ax,[si.dylDr]
	sub	ax,[ypTop]
	add	ax,[dylBelow]
	mov	[dylLimPldr],ax
	; native note - do fFrameLines here so that we can jump
	; over the next block without retesting it
; #define FDrawTableDrsWw(ww)  \
; (PwwdWw(ww)->grpfvisi.fDrawTableDrs || PwwdWw(ww)->grpfvisi.fvisiShowAll)
	call	LN_PwwdWw
	xchg	ax,bx
	xor	ax,ax
	mov	[fFrameLines],al ; assume fFalse
	test	[bx.grpfvisiWwd],maskfDrawTableDrsGrpfvisi or maskfvisiShowAllGrpfvisi
	jz	UT130
	mov	[fFrameLines],fTrue ; assumption failed
	; fall through to next block with fFrameLines already tested true
; if (dylBelow == 0 && fFrameLines && !fPageView)
;   dylLimPldr += DyFromBrc(brcNone,fTrue/*fFrameLines*/);
	; si = pdrT, ax = 0
	cmp	[dylBelow],ax
	jnz	UT130
	cmp	[fPageView],al
	jnz	UT130
; #define DyFromBrc(brc, fFrameLines) DxyFromBrc(brc, fFrameLines, fFalse)
; #define DxyFromBrc(brc, fFrameLines, fWidth)	\
;   WidthHeightFromBrc(brc, fFrameLines | (fWidth << 1))
	errnz	<brcNone>
	; si = pdrT, ax = 0
	push	ax
	inc	ax	; fFrameLines | (fWidth << 1) == 1
	push	ax
ifdef	DEBUG
	cCall	S_WidthHeightFromBrc,<>
else
	cCall	N_WidthHeightFromBrc,<>
endif
	
	add	[dylLimPldr],ax
	xor	ax,ax
UT130:
; if (fAbsHgtRow)
;   dylLimPldr = min(dylLimPldr,DypFromDya(-dyaRowHeight) + dylBelow);
	; si = pdrT, ax = 0
;PAUSE
	cmp	[fAbsHgtRow],al
	jz	UT135	; do screwy jump to set up ax for next block
	mov	ax,[dyaRowHeight]
	neg	ax
	call	LN_DypFromDya
	add	ax,[dylBelow]
	cmp	ax,[dylLimPldr]
	jle	UT140
UT135:
	mov	ax,[dylLimPldr]
UT140:
	mov	[dylLimPldr],ax

UT150:
; dylLimDr = dylLimPldr - dylAbove - dylBelow;
	; si = pdrT, ax = dylLimPldr
	sub	ax,[dylAbove]
	sub	ax,[dylBelow]
	mov	[dylLimDr],ax

; /* Check for incremental update */
; Break3();	/* Mac only thing */
; if (dlOld == dlNew && dlOld < dlMac && cp == CpPlc(hplcedl,dlNew))
	mov	ax,[dlOld]
	cmp	ax,[dlNew]
	jne	LChkFreeEdl
;PAUSE
	cmp	ax,[dlMac]
	jge	LChkFreeEdl
	push	[hplcedl]
	push	ax	; since dlNew == dlOld
	cCall	CpPlc,<>
;PAUSE
	sub	ax,[OFF_cp]
	sbb	dx,[SEG_cp]
	or	ax,dx
	jne	LChkFreeEdl
; {
; /* we are about to either use dlOld, or trash it. The next
; /* potentially useful dl is therefore dlOld+1
; /**/
; GetPlc ( hplcedl, dlNew, &edl );
;PAUSE
	lea	ax,[edl]
	mov	cx,[dlNew]
	call	LN_GetPlcParent
; if ( edl.ypTop == *pypTop && edl.hpldr != hNil )
	mov	ax,[ypTop]
	sub	ax,[edl.ypTopEdl]
	jne	UT175
	; ax = 0
	errnz	<hNil>
	cmp	[edl.hpldrEdl],ax
	jz	UT175
; {
; hpldrTable = edl.hpldr;
	mov	ax,[edl.hpldrEdl]
	mov	[hpldrTable],ax
; Assert ( !edl.fDirty );
; Assert((*hpldrTable)->idrMac == itcMac + 1);
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	push	di
	test	[edl.fDirtyEdl],maskFDirtyEdl
	jz	UT160
	mov	ax,midDisptbn2
	mov	bx,507
	cCall	AssertProcForNative,<ax,bx>
UT160:
	mov	di,[hpldrTable]
	mov	di,[di]
	mov	ax,[di.idrMacPldr]
	sub	ax,[itcMac]
	dec	ax
	jz	UT170
	mov	ax,midDisptbn2
	mov	bx,517
	cCall	AssertProcForNative,<ax,bx>
UT170:
	pop	di
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG
; fIncr = fTrue;
	mov	[fIncr],fTrue
; ++dlOld;
	inc	[dlOld]
; goto LUpdateTable;
	jmp	LUpdateTable
; }
UT175:
; }
LChkFreeEdl:
; if (dlNew == dlOld && dlOld < dlMac) /* existing edl is useless, free it */
	mov	ax,[dlOld]
	cmp	ax,[dlNew]
	jne	LClearFIncr
	cmp	ax,[dlMac]
	jge	LClearFIncr
;   FreeEdl(hplcedl, dlOld++);
;PAUSE
	push	[hplcedl]
	push	ax
	inc	ax
	mov	[dlOld],ax
	cCall	FreeEdl,<>

; Break3();	native note: Mac only
; fIncr = fReusePldr = fFalse;
LClearFIncr:
	xor	ax,ax
	mov	[fIncr],al
	mov	[fReusePldr],al

; /* new table row; init edl */
; if (vihpldrMac > 0)
	mov	cx,[vihpldrMac]
	jcxz	LHpldrInit
; {
; hpldrTable = vrghpldr[--vihpldrMac];
; vrghpldr[vihpldrMac] = hNil;	/* we're gonna use or lose it */
;PAUSE
	dec	cx
	mov	[vihpldrMac],cx
	sal	cx,1
	mov	di,cx
	mov	bx,[vrghpldr.di]
	mov	[vrghpldr.di],hNil
	mov	[hpldrTable],bx
	
; ppldrTable = *hpldrTable;
	mov	di,[bx]
; if (ppldrTable->idrMax != itcMac+1 || !ppldrTable->fExternal)
	mov	ax,[di.idrMaxPldr]
	sub	ax,[itcMac]
	dec	ax
	jnz	LFreeHpldr
	; ax = 0
	cmp	[di.fExternalPldr],ax
	jnz	LReusePldr
LFreeHpldr:
; {
; /* If ppldrTable->idrMac == 0, LcbGrowZone freed up all of
; /* the far memory associated with this hpldr. We now need to
; /* free only the near memory. */
; if (ppldrTable->idrMax > 0)
	; bx = hpldrTable, di = ppldrTable
;PAUSE
	mov	cx,[di.idrMaxPldr]	; technically an uns
	jcxz	LFreeHpldr2
; {
;   FreeDrs(hpldrTable, 0);
;   if ((*hpldrTable)->fExternal)
;     FreeHq((*hpldrTable)->hqpldre);
	push	bx
	xor	ax,ax
	push	ax
	cCall	FreeDrs,<>

	mov	cx,[di.fExternalPldr]
	jcxz	LFreeHpldr2
	push	[di.HI_hqpldrePldr]
	push	[di.LO_hqpldrePldr]
	cCall	FreeHq,<>
LFreeHpldr2:
; }
; FreeH(hpldrTable);	
; #define FreeH(h)    	    	FreePpv(sbDds,(h))
	mov	ax,sbDds
	push	ax
	push	[hpldrTable]
	cCall	FreePpv,<>
; hpldrTable = hNil;
	mov	[hpldrTable],hNil
	jmp	short LHpldrInit
; }
; else
; {
; Assert(ppldrTable->brgdr == offset(PLDR, rgdr));
LReusePldr:
	; bx = hpldrTable, di = ppldrTable
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	[di.brgdrPldr],rgdrPldr
	je	UT180
	mov	ax,midDisptbn2
	mov	bx,632
	cCall	AssertProcForNative,<ax,bx>
UT180:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG
; fReusePldr = fTrue;
	mov	[fReusePldr],fTrue
	jmp	short LChkHpldr
; }
; }
; if (!fReusePldr)
;	native note: we jump directly into the right clause from above
; {
; hpldrTable = HplInit2(sizeof(struct DR),offset(PLDR, rgdr), itcMac+1, fTrue);
LHpldrInit:
	mov	ax,cbDrMin
	push	ax
	mov	ax,rgdrPldr
	push	ax
	mov	ax,[itcMac]
	inc	ax
	push	ax
	mov	ax,fTrue
	push	ax
	cCall	HplInit2,<>
	mov	[hpldrTable],ax
	xchg	ax,bx
; Assert(hpldrTable == hNil || (*hpldrTable)->idrMac == 0);
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	bx,[hpldrTable]
	or	bx,bx
	jz	UT190
	mov	bx,[bx]
	cmp	[bx.idrMacPldr],0
	jz	UT190
	mov	ax,midDisptbn2
	mov	bx,716
	cCall	AssertProcForNative,<ax,bx>
UT190:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG
; }

LChkHpldr:
	; bx = hpldrTable
; edl.hpldr = hpldrTable;
	mov	[edl.hpldrEdl],bx
; if (hpldrTable == hNil || vmerr.fMemFail)
	or	bx,bx
	jz	UT200
	cmp	[vmerr.fMemFailMerr],0
	jz	UT210
; {
; SetErrorMat(matDisp);

UT200:
	mov	ax,matDisp
	push	ax
	cCall	SetErrorMatProc,<>

; return fFalse; /* operation failed */
	xor	ax,ax
	jmp	LExitUT
; }

	; bx = hpldrTable
; edl.ypTop = *pypTop;
UT210:
	mov	ax,[ypTop]
	mov	[edl.ypTopEdl],ax

; native note: the order of these next instructions has been altered
; from the C version for efficiency...
;
; /* this allows us to clobber (probably useless) dl's below */
; Assert(FInCa(doc,cp,&vtcc.ca) && vtcc.itc == 0);
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	push	[doc]
	push	[SEG_cp]
	push	[OFF_cp]
	mov	ax,dataoffset [vtcc.caTcc]
	push	ax
	cCall	FInCa,<>
	or	ax,ax
	jz	UT220
	cmp	[vtcc.itcTcc],0
	jz	UT230
UT220:
	mov	ax,midDisptbn2
	mov	bx,908
	cCall	AssertProcForNative,<ax,bx>
UT230:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG
; dylDrOld = dylLimDr;
	mov	ax,[dylLimDr]
	mov	[dylDrOld],ax
; edl.dyp = dylLimPldr;
	mov	ax,[dylLimPldr]
	mov	[edl.dypEdl],ax
; edl.dcpDepend = 1;
; edl.dlk = dlkNil;
;	REVIEW is this right?
	errnz	<dcpDependEdl+1-(dlkEdl)>
	mov	[edl.grpfEdl],00001h
; edl.xpLeft = vtcc.xpCellLeft;
	mov	ax,[vtcc.xpCellLeftTcc]
	mov	[edl.xpLeftEdl],ax
; ppldrTable = *hpldrTable;
; set-up for fast moves..
	mov	di,[bx]
	add	di,hpldrBackPldr
	push	ds
	pop	es
; ppldrTable->hpldrBack = hpldr;
	mov	ax,[hpldr]
	stosw
; ppldrTable->idrBack = idr;
	errnz	<hpldrBackPldr+2-idrBackPldr>
	mov	ax,[idr]
	stosw
; ppldrTable->ptOrigin.xp = 0;
	errnz	<idrBackPldr+2-ptOriginPldr>
	errnz	<xwPt>
	xor	ax,ax
	stosw
; ppldrTable->ptOrigin.yp = edl.ypTop;
	mov	ax,[edl.ypTopEdl]
	stosw
; ppldrTable->dyl = edl.dyp;
	errnz	<ptOriginPldr+ypPt+2-(dylPldr)>
	mov	ax,[edl.dypEdl]
	stosw
; PutCpPlc ( hplcedl, dlNew, cp );
; PutPlc ( hplcedl, dlNew, &edl );
	; bx = hpldrTable
	; push arguments for PutCpPlc call
	push	[hplcedl]
	push	[dlNew]
	push	[SEG_cp]
	push	[OFF_cp]

	cCall	PutCpPlc,<>
	call	LN_PutPlc

; Break3();
LUpdateTable:
; native note: set up register variable, this is
; assumed by the remainder of the routine (and the
; little helper routines)
;PAUSE
	lea	di,[drfFetch]
; fSomeDrDirty = fFalse;	/* for second pass */
; CpFirstTap1( doc, cp, fOutline );
; idrMacTable = fIncr ? itcMac + 1 : 0;
	xor	ax,ax
	mov	[fSomeDrDirty],al
	cmp	[fIncr],al
	jz	UT300
;PAUSE
	mov	ax,[itcMac]
	inc	ax
UT300:
	mov	[idrMacTable],ax
;PAUSE
	push	[doc]
	push	[SEG_cp]
	push	[OFF_cp]
	mov	al,[fOutline]
	cbw
	push	ax
	cCall	CpFirstTap1,<>
; dylNew = DypFromDya(abs(vtapFetch.dyaRowHeight)) - dylAbove - dylBelow;
	mov	ax,[vtapFetch.dyaRowHeightTap]
	or	ax,ax
	jns	UT310
;PAUSE
	neg	ax
UT310:
	call	LN_DypFromDya
	sub	ax,[dylAbove]
	sub	ax,[dylBelow]
	mov	[dylNew],ax
; dylOld = (*hpldrTable)->dyl;
	mov	bx,[hpldrTable]
	mov	bx,[bx]
	mov	ax,[bx.dylPldr]
	mov	[dylOld],ax

; for ( idrTable = fTtpDr = 0; !fTtpDr; ++idrTable )
; {
	xor	ax,ax
	mov	[idrTable],ax
	mov	[fTtpDr],al
LIdrLoopTest:
	cmp	fTtpDr,0
	jz	UT320
	jmp	LIdrLoopEnd
; fTtpDr = idrTable == itcMac;
UT320:
	mov	ax,[idrTable]
	cmp	ax,[itcMac]
	jne	UT330
	mov	[fTtpDr],fTrue
UT330:
; Assert ( FInCa(doc,cp,&caTapCur) );
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	push	[doc]
	push	[SEG_cp]
	push	[OFF_cp]
	lea	ax,[caTapCur]
	push	ax
	cCall	FInCa,<>
	or	ax,ax
	jnz	UT340
	mov	ax,midDisptbn2
	mov	bx,828
	cCall	AssertProcForNative,<ax,bx>
UT340:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG
	; ax = idrTable
; if ( idrTable >= idrMacTable )
	; ax = idrTable
	sub	ax,[idrMacTable] ; sub for zero register on branch
	jge	LNewDr
; native note: we'll do the else clause here...
; else
;   pdrTable = PdrFetch(hpldrTable,idrTable,&drfFetch);
;PAUSE
	call	LN_PdrFetchTable
	jmp	LValidateDr
; native note: we now return you to our regularly scheduled if statement
; native note: assert idrTable == idrMacTable, hence ax == 0
LNewDr:
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	xor	ax,ax
	jz	UT350
	mov	ax,midDisptbn2
	mov	bx,849
	cCall	AssertProcForNative,<ax,bx>
UT350:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG
; {	/* this is a new cell in the table */
; if (fReusePldr)
	; ax = 0
	cmp	fReusePldr,al
	jz	UT370	; native note: branch expects ax = 0
; {
;   pdrTable = PdrFetch(hpldrTable, idrTable, &drfFetch);
;PAUSE
	call	LN_PdrFetchTable
;   Assert(drfFetch.pdrfUsed == 0);
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	[drfFetch.pdrfUsedDrf],0
	jz	UT360
	mov	ax,midDisptbn2
	mov	bx,860
	cCall	AssertProcForNative,<ax,bx>
UT360:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG
;   PutIMacPlc(drfFetch.dr.hplcedl, 0);
	push	[drfFetch.drDrf.hplcedlDr]
	xor	ax,ax
	push	ax
	cCall	PutIMacPlc,<>
	jmp	short UT380
; }
; else
; {
UT370:
;   SetWords ( &drfFetch, 0, sizeof(struct DRF) / sizeof(int) );
	; ax = 0, di = &drfFetch
	errnz	<cbDrfMin and 1>
	mov	cx,cbDrfMin SHR 1
	push	di	; save register variable
	push	ds
	pop	es
	rep	stosw
	pop	di
; }

; /* NOTE We are assuming that the 'xl' coordiates in the table frame (PLDR)
; /* are the same as the 'xp' coordinates of the bounding DR.
; /**/
; ItcGetTcxCache(ww, doc, cp, &vtapFetch, idrTable/*itc*/, &tcx);
UT380:
;PAUSE
	push	[ww]
	push	[doc]
	push	[SEG_cp]
	push	[OFF_cp]
	mov	ax,dataoffset [vtapFetch]
	push	ax
	push	[idrTable]
	lea	ax,[tcx]
	push	ax
ifdef	DEBUG
	cCall	S_ItcGetTcxCache
else
	cCall	N_ItcGetTcxCache
endif

; native note: the following block of code has been rearranged from
; the C version of efficiency...
;
	push	di	; save register variable
	push	ds
	pop	es
	lea	di,[drfFetch.drDrf.xlDr]
; drfFetch.dr.xl = tcx.xpDrLeft;
	mov	ax,[tcx.xpDrLeftTcx]
	stosw
; drfFetch.dr.yl = fTtpDr ? max(dyFrameLine,dylAbove) : dylAbove;
	errnz	<xlDr+2-ylDr>
	mov	ax,[dylAbove]	; right more often than not
	cmp	[fTtpDr],0
	jz	UT390
;PAUSE
	mov	ax,[dylAbove]
; #define dyFrameLine		dypBorderFti
; #define dypBorderFti		vfti.dypBorder
	cmp	ax,[vfti.dypBorderFti]
	jge	UT390
	mov	ax,[vfti.dypBorderFti]
UT390:
	stosw
; drfFetch.dr.dxl = tcx.xpDrRight - drfFetch.dr.xl;
	errnz	<ylDr+2-dxlDr>
	mov	ax,[tcx.xpDrRightTcx]
	sub	ax,[drfFetch.drDrf.xlDr]
	stosw
; drfFetch.dr.dyl = edl.dyp - drfFetch.dr.yl - dylBelow;
;PAUSE
	errnz	<dxlDr+2-dylDr>
	mov	ax,[edl.dypEdl]
	sub	ax,[drfFetch.drDrf.ylDr]
	sub	ax,[dylBelow]
	stosw
; drfFetch.dr.cpLim = cpNil;
; drfFetch.dr.doc = doc;
	errnz	<dylDr+2+4-cpLimDr>
	errnz	<LO_cpNil+1>
	errnz	<HI_cpNil+1>
	add	di,4
	mov	ax,-1
	stosw
	stosw
	errnz	<cpLimDr+4-docDr>
	mov	ax,[doc]
	stosw
; drfFetch.dr.fDirty = fTrue;
; drfFetch.dr.fCpBad = fTrue;
; drfFetch.dr.fInTable = fTrue;
; drfFetch.dr.fForceWidth = fTrue;
; this next one is silly since it is already set to zero...
; drfFetch.dr.fBottomTableFrame = fFalse; /* we'll set this correctly later */
	errnz	<docDr+2-dcpDependDr>
	errnz	<dcpDependDr+1-fDirtyDr>
	errnz	<fDirtyDr-fCpBadDr>
	errnz	<fDirtyDr-fInTableDr>
	errnz	<fDirtyDr-fForceWidthDr>
	; native note: this stores 0 in dr.dcpDepend, which has
	; no net effect (it's already zero by the SetWords)
;PAUSE
	errnz	<maskFDirtyDr-00001h>
	errnz	<maskFCpBadDr-00002h>
	errnz	<maskFInTableDr-00020h>
	errnz	<maskFForceWidthDr-00080h>
	;mov	ax,(maskFDirtyDr or maskFCpBadDr or fInTableDr or fForceWidthDr) SHL 8
	mov	ax,0A300h
	stosw	; how's that for C to 8086 compression?
;
; drfFetch.dr.dxpOutLeft = tcx.dxpOutLeft;
	errnz	<dcpDependDr+6-(dxpOutLeftDr)>
	add	di,4	; advance di
	mov	ax,[tcx.dxpOutLeftTcx]
	stosw
; drfFetch.dr.dxpOutRight = tcx.dxpOutRight;
	errnz	<dxpOutLeftDr+2-dxpOutRightDr>
	mov	ax,[tcx.dxpOutRightTcx]
	stosw
; drfFetch.dr.idrFlow = idrNil;
	errnz	<dxpOutRightDr+2-idrFlowDr>
	errnz	<idrNil+1>
	mov	ax,-1
	stosw
; drfFetch.dr.lrk = lrk;
	; ax = -1
	errnz	<idrFlowDr+2-ccolM1Dr>
	errnz	<ccolM1Dr+1-lrkDr>
	inc	ax	; now it's zero
	mov	ah,[lrk]
	stosw	; also stores 0 in ccolM1, again no net change
; drfFetch.dr.fCantGrow = fPageView || fAbsHgtRow;
	errnz	<lrkDr+1-fCantGrowDr>
	mov	al,[fPageView]
	or	al,[fAbsHgtRow]
	jz	UT400
;PAUSE
	or	bptr [di],maskFCantGrowDr
UT400:
;PAUSE
	pop	di	; restore register variable

; if (!fReusePldr)
	cmp	[fReusePldr],0
	jnz	UT430
; {
; if (!FInsertInPl ( hpldrTable, idrTable, &drfFetch.dr )
	push	[hpldrTable]
	push	[idrTable]
	lea	ax,[drfFetch.drDrf]
	push	ax
	cCall	FInsertInPl,<>
	or	ax,ax
	jz	LAbortUT	; if we branch, ax is zero as required
; || !FInitHplcedl(1, cpMax, hpldrTable, idrTable))
	; native note: hold on to your hats, this next bit is trickey
	errnz	<LO_cpMax-0FFFFh>
	errnz	<HI_cpMax-07FFFh>
	mov	ax,1
	push	ax
	neg	ax
	cwd
	shr	dx,1
	push	dx	; SEG
	push	ax	; OFF
	push	[hpldrTable]
	push	[idrTable]
	cCall	FInitHplcedl,<>
	or	ax,ax
	jnz	UT420
; {
; native note: as usual, the order of the following has been changed
; to save a few bytes...
LAbortUT:	; NOTE ax must be zero when jumping to this label!!!
;PAUSE
	; ax = 0
; edl.hpldr = hNil;
	errnz	<hNil>
	mov	[edl.hpldrEdl],ax
; FreeDrs(hpldrTable, 0);
	push	[hpldrTable]
	push	ax
	cCall	FreeDrs,<>
; FreeHpl(hpldrTable);
	push	[hpldrTable]
	cCall	FreeHpl,<>
; PutPlc(hplcedl, dlNew, &edl);
	call	LN_PutPlc
; return fFalse; /* operation failed */
	xor	ax,ax
	jmp	LExitUT
; }
; pdrTable = PdrFetch(hpldrTable,idrTable,&drfFetch);
UT420:
	call	LN_PdrFetchTable
; }

UT430:
; ++idrMacTable;
	inc	[idrMacTable]
; Assert (fReusePldr || (*hpldrTable)->idrMac == idrMacTable );
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	[fReusePldr],0
	jnz	UT440
	mov	bx,[hpldrTable]
	mov	bx,[bx]
	mov	ax,[bx.idrMacPldr]
	cmp	ax,[idrMacTable]
	je	UT440
	mov	ax,midDisptbn2
	mov	bx,1097
	cCall	AssertProcForNative,<ax,bx>
UT440:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG
; }
; native note: else clause done above
LValidateDr:
	; si = pdrTable

; if (pdrTable->fCpBad || pdrTable->cpFirst != cp)
	; native note: this next block is somewhat verbose,
	; to leave [cp] in ax,dx
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	test	[si.fCpBadDr],maskFCpBadDr
	jnz	UT500
;PAUSE
	cmp	ax,[si.LO_cpFirstDr]
	jnz	UT500
	cmp	dx,[si.HI_cpFirstDr]
	jz	UT510
; {
; pdrTable->cpFirst = cp;
UT500:
	; si = pdrTable, ax,dx = cp
	mov	[si.HI_cpFirstDr],dx
	mov	[si.LO_cpFirstDr],ax
; pdrTable->fCpBad = fFalse;
	and	[si.fCpBadDr],not maskFCpBadDr
; pdrTable->fDirty = fTrue;
	or	[si.fDirtyDr],maskFDirtyDr
; }
; if (pdrTable->yl != (ylT = fTtpDr ? max(dylAbove,dyFrameLine) : dylAbove))
UT510:
	mov	ax,[dylAbove]
	cmp	[fTtpDr],0
	jz	UT520
; #define dyFrameLine		dypBorderFti
; #define dypBorderFti		vfti.dypBorder
;PAUSE
	cmp	ax,[vfti.dypBorderFti]
	jge	UT520
	mov	ax,[vfti.dypBorderFti]
UT520:
	sub	ax,[si.ylDr]	; sub for zero register on branch
	je	UT530
; {
; pdrTable->yl = ylT;
	; ax = ylT - pdrTable->ylDr
;PAUSE
	add	[si.ylDr],ax
; FreeEdls(pdrTable->hplcedl,0,IMacPlc(pdrTable->hplcedl));
; PutIMacPlc(pdrTable->hplcedl,0);
	mov	bx,[si.hplcedlDr]
	xor	cx,cx	; zero register
	push	bx  ; arguments for
	push	cx  ; PutIMacPlc call

	push	bx  ; some arguments for
	push	cx  ; FreeEdls call

	call	LN_IMacPlcTable

	push	ax  ; last argument for FreeEdls - IMacPlc result
	cCall	FreeEdls,<>
	cCall	PutIMacPlc,<>
; pdrTable->fDirty = fTrue;
	or	[si.fDirtyDr],maskFDirtyDr
	jmp	short LChkFTtpDr
; }
; else if (pdrTable->fBottomTableFrame != fLastRow || pdrTable->fDirty)
UT530:
	; ax = 0
	test	[si.fBottomTableFrameDr],maskFBottomTableFrameDr
	jz	UT540
	inc	ax
UT540:
	cmp	[fLastRow],0
	jz	UT550
	xor	ax,1
UT550:
	or	ax,ax
	jnz	UT560
;PAUSE
	test	[si.fDirtyDr],maskFDirtyDr
	jz	LChkFTtpDr
; {
; if ((dlLast = IMacPlc(pdrTable->hplcedl) - 1) >= 0)
UT560:
	call	LN_IMacPlcTable
	dec	ax
	mov	[dlLast],ax
	jl	UT570
; {
; GetPlc(pdrTable->hplcedl,dlLast,&edlLast);
;PAUSE
	xchg	ax,cx
	call	LN_GetPlcTable
; edlLast.fDirty = fTrue;
	or	[edlLast.fDirtyEdl],maskFDirtyEdl
; PutPlcLast(pdrTable->hplcedl,dlLast,&edlLast);
; #ifdef DEBUG
; #	define PutPlcLast(h, i, pch)  PutPlcLastDebugProc(h, i, pch)
; #else
; #	define PutPlcLast(h, i, pch)  PutPlcLastProc()
; #endif
; }
ifdef	DEBUG
	push	[si.hplcedlDr]
	push	[dlLast]
	lea	ax,[edlLast]
	push	ax
	cCall	PutPlcLastDebugProc,<>
else	; !DEBUG
	call	LN_PutPlcLast
endif	; DEBUG
UT570:
; pdrTable->fDirty = fTrue;
	or	[si.fDirtyDr],maskFDirtyDr
; }

; if (fTtpDr)
LChkFTtpDr:
	cmp	[fTtpDr],0
	jz	UT620
; {
; if (fAbsHgtRow)
;PAUSE
	cmp	[fAbsHgtRow],0
	jz	UT590
;   dylNew = DypFromDya(abs(vtapFetch.dyaRowHeight)) + dylBelow;
;PAUSE
	mov	ax,[vtapFetch.dyaRowHeightTap]
; native note: Assert(vtapFetch.dyaRowHeight < 0);
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	or	ax,ax
	js	UT580
	mov	ax,midDisptbn2
	mov	bx,1482
	cCall	AssertProcForNative,<ax,bx>
UT580:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG
	neg	ax
	call	LN_DypFromDya
	add	ax,[dylBelow]
	jmp	short UT600
; else
;   dylNew += dylAbove + dylBelow;
UT590:
;PAUSE
	mov	ax,[dylNew]
	add	ax,[dylAbove]
	add	ax,[dylBelow]
UT600:
	mov	[dylNew],ax	; leave dylNew in ax for next block...
; pdrTable->dyl = min(pdrTable->dyl, dylNew - pdrTable->yl - dylBelow);
	; ax = dylNew
	sub	ax,[si.ylDr]
	sub	ax,[dylBelow]
	cmp	ax,[si.dylDr]
	jge	UT610
	mov	[si.dylDr],ax	
UT610:
; }
UT620:
; Assert ( pdrTable->cpFirst == cp );
;PAUSE
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[OFF_cp]
	cmp	ax,[si.LO_cpFirstDr]
	jne	UT630
	mov	ax,[SEG_cp]
	cmp	ax,[si.HI_cpFirstDr]
	je	UT640
UT630:
	mov	ax,midDisptbn2
	mov	bx,1284
	cCall	AssertProcForNative,<ax,bx>
UT640:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG

; if (pdrTable->fDirty)
	test	[si.fDirtyDr],maskFDirtyDr
	jnz	UT650
	jmp	LSetDylNew
; {
; /* set DR to full row height */
; if (fIncr)
UT650:
	cmp	[fIncr],0
	jz	LSetFBottom
; {
; /* we're betting that the row height doesn't change.
; /* record some info to check our bet and take corrective
; /* measures if we lose.
; /**/
; dlMacOld = IMacPlc(pdrTable->hplcedl);
;PAUSE
	call	LN_IMacPlcTable
	mov	[dlMacOld],ax
; dylDrOld = pdrTable->dyl;
	mov	ax,[si.dylDr]
	mov	[dylDrOld],ax
; pdrTable->fCantGrow = pdrTable->dyl >= dylLimDr && (fPageView || fAbsHgtRow);
	call	LN_SetFCantGrow
; pdrTable->dyl = edl.dyp - pdrTable->yl - dylBelow;
	mov	ax,[edl.dypEdl]
	sub	ax,[si.ylDr]
	sub	ax,[dylBelow]
	mov	[si.dylDr],ax
; }

; /* enable drawing the bottom table frame */
; pdrTable->fBottomTableFrame = fLastRow;
LSetFBottom:
	and	[si.fBottomTableFrameDr],not maskFBottomTableFrameDr  ; assume fFalse
	cmp	[fLastRow],0
	jz	UT660
	or	[si.fBottomTableFrameDr],maskFBottomTableFrameDr  ; assumption failed
UT660:
; /* write any changes we have made, native code will need them */
; pdrTable = PdrFreeAndFetch(hpldrTable,idrTable,&drfFetch);
	push	[hpldrTable]
	push	[idrTable]
	push	di
ifdef	DEBUG
	cCall	S_PdrFreeAndFetch,<>
else
	cCall	N_PdrFreeAndFetch,<>
endif
	xchg	ax,si
; if (!FUpdTableDr(ww,hpldrTable,idrTable))
; if (!FUpdateDr(ww,hpldrTable,idrTable,rcwInval,fFalse,udmodTable,cpNil))
; {
; LFreeAndAbort:
;  FreePdrf(&drfFetch);
;  goto LAbort;
; }
; NOTE: the local routine also does the assert with DypHeightTc
;PAUSE
	xor	ax,ax	; flag to helper routine
	mov	cx,[idrTable]
	errnz	<xwLeftRc>
	lea	bx,[rcwInvalXwLeftRc]
	call	LN_FUpdateOneDr
	jnz	UT670
LFreeAndAbortUT:
	call	LN_FreePdrf
	xor	ax,ax
	jmp	LAbortUT	; note ax is zero, as required for the jump
UT670:
	
; /* check for height change */
; pdrTable->fCantGrow = pdrTable->dyl >= dylLimDr && (fPageView || fAbsHgtRow);
	call	LN_SetFCantGrow
; if (pdrTable->dyl != dylDrOld)
	mov	ax,[dylDrOld]
	cmp	ax,[si.dylDr]
	je	LSetFSomeDirty
; {
; if (fIncr && fLastRow && dylDrOld == dylOld - dylAbove - dylBelow
; && IMacPlc(pdrTable->hplcedl) >= dlMacOld)
	; ax = [dylDrOld]
	cmp	[fIncr],0
	jz	LSetFSomeDirty
;PAUSE
	cmp	[fLastRow],0
	jz	LSetFSomeDirty
	sub	ax,[dylOld]
	add	ax,[dylAbove]
	add	ax,[dylBelow]
	jnz	LSetFSomeDirty
	call	LN_IMacPlcTable
	cmp	ax,[dlMacOld]
	jl	LSetFSomeDirty
; {
; /* we probably have a dl that has a frame line in the
; /* middle of a DR.
; /**/
; pdrTable->fDirty = fTrue;
;PAUSE
	or	[si.fDirtyDr],maskFDirtyDr
; GetPlc(pdrTable->hplcedl,dlMacOld-1,&edlLast);
	mov	cx,[dlMacOld]
	dec	cx
	call	LN_GetPlcTable
; edlLast.fDirty = fTrue;
	or	[edlLast.fDirtyEdl],maskFDirtyEdl
; PutPlcLast(pdrTable->hplcedl,dlMacOld-1,&edlLast);
; #ifdef DEBUG
; #	define PutPlcLast(h, i, pch)  PutPlcLastDebugProc(h, i, pch)
; #else
; #	define PutPlcLast(h, i, pch)  PutPlcLastProc()
; #endif
; }
ifdef	DEBUG
	push	[si.hplcedlDr]
	mov	ax,[dlMacOld]
	dec	ax
	push	ax
	lea	ax,[edlLast]
	push	ax
	cCall	PutPlcLastDebugProc,<>
else	; !DEBUG
	call	LN_PutPlcLast
endif	; DEBUG
; }
; }

; fSomeDrDirty |= pdrTable->fDirty && !fTtpDr;
LSetFSomeDirty:
	test	[si.fDirtyDr],maskFDirtyDr
	jz	LSetDylNew
;PAUSE
	cmp	[fTtpDr],0
	jnz	LSetDylNew
	mov	[fSomeDrDirty],fTrue
; }

; if (!fTtpDr)
LSetDylNew:
	cmp	[fTtpDr],0
	jnz	UT690
; dylNew = max(dylNew, pdrTable->dyl);
;PAUSE
	mov	ax,[si.dylDr]
	cmp	ax,[dylNew]
	jle	UT690
	mov	[dylNew],ax
UT690:

; Win(fRMark |= pdrTable->fRMark;)
;PAUSE
	test	[si.fRMarkDr],maskFRMarkDr
	je	UT695
	mov	[fRMark],fTrue
UT695:

; /* advance the cp */
; cp = CpMacPlc ( pdrTable->hplcedl );
	push	[si.hplcedlDr]
	cCall	CpMacPlc,<>
	mov	[OFF_cp],ax
	mov	[SEG_cp],dx
; FreePdrf(&drfFetch);
	call	LN_FreePdrf

; } /* end of for idrTable loop */
	inc	[idrTable]
	jmp	LIdrLoopTest
LIdrLoopEnd:

; idrMacTable = (*hpldrTable)->idrMac = itcMac + 1;
	mov	bx,[hpldrTable]
	mov	bx,[bx]
	mov	cx,[itcMac]
	inc	cx
	mov	[bx.idrMacPldr],cx

; /* adjust the TTP DR to the correct height, don't want it to dangle
; /* below the PLDR/outer EDL
; /**/
; pdrTable = PdrFetch(hpldrTable, itcMac, &drfFetch);
	; cx = itcMac + 1
	dec	cx
	call	LN_PdrFetch
; if (pdrTable->yl + pdrTable->dyl + dylBelow > dylNew)
	mov	ax,[dylNew]
	sub	ax,[si.ylDr]
	sub	ax,[dylBelow]
	cmp	ax,[si.dylDr]
	jge	LFreeDrMac
; {
; pdrTable->dyl = dylNew - pdrTable->yl - dylBelow;
	; ax = dylNew - pdrTable->yl - dylBelow
	mov	[si.dylDr],ax
; Assert(IMacPlc(pdrTable->hplcedl) == 1);
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	push	[si.hplcedlDr]
	cCall	IMacPlc,<>
	dec	ax
	jz	UT700
	mov	ax,midDisptbn2
	mov	bx,1559
	cCall	AssertProcForNative,<ax,bx>
UT700:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG
; if (dylOld >= dylNew)
	mov	ax,[dylOld]
	cmp	ax,[dylNew]
	jl	LFreeDrMac
; {
; GetPlc(pdrTable->hplcedl, 0, &edlLast);
	xor	cx,cx
	call	LN_GetPlcTable
; edlLast.fDirty = fFalse;
	and	[edlLast.fDirtyEdl],not maskFDirtyEdl
; PutPlcLast(pdrTable->hplcedl, 0, &edlLast);
; #ifdef DEBUG
; #	define PutPlcLast(h, i, pch)  PutPlcLastDebugProc(h, i, pch)
; #else
; #	define PutPlcLast(h, i, pch)  PutPlcLastProc()
; #endif
; }
ifdef	DEBUG
	push	[si.hplcedlDr]
	xor	ax,ax
	push	ax
	lea	ax,[edlLast]
	push	ax
	cCall	PutPlcLastDebugProc,<>
else	; !DEBUG
	call	LN_PutPlcLast
endif	; DEBUG
; pdrTable->fDirty = fFalse;
	and	[si.fDirtyDr],not maskFDirtyDr
; }
; }
LFreeDrMac:
; FreePdrf(&drfFetch);
	call	LN_FreePdrf

; Assert ( cp == caTap.cpLim );
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	cmp	ax,[caTap.LO_cpLimCa]
	jnz	UT710
	cmp	dx,[caTap.HI_cpLimCa]
	jz	UT720
UT710:
	mov	ax,midDisptbn2
	mov	bx,1622
	cCall	AssertProcForNative,<ax,bx>
UT720:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG

; /* At this point, we have scanned the row of the table,
; /* found the fTtp para, and updated the cell's DR's up
; /* to the height of the old EDL. We know
; /* the number of cells/DRs in the row of the table. We now
; /* adjust the height of the mother EDL to the correct row height.
; /**/
; if (fAbsHgtRow)
	cmp	[fAbsHgtRow],0
	jz	LUpdateHeight
; {
; dylNew -= dylAbove + dylBelow;
; native note: we'll keep this transient value of dylNew in registers
;PAUSE
	mov	si,[dylNew]
	sub	si,[dylAbove]
	sub	si,[dylBelow]
; for ( idrTable = itcMac; idrTable--; FreePdrf(&drfFetch))
	; di = &drfFetch, si = dylNew
	mov	cx,[itcMac]
LLoop2Body:
	dec	cx
	push	cx	; save idrTable
; {
; /* this depends on the theory that we never need a second
; /* pass for an absolute height row
; /**/
; pdrTable = PdrFetch(hpldrTable, idrTable, &drfFetch);
	; si = dylNew, di = &drfFetch, cx = idrTable
	call	LN_PdrFetch
	; now, ax = dylNew, si = pdrTable, di = &drfFetch
; if (pdrTable->dyl >= dylNew)
	cmp	ax,[si.dylDr]
	jg	UT725
; {
; pdrTable->fCantGrow = fTrue;
;PAUSE
	or	[si.fCantGrowDr],maskFCantGrowDr
; pdrTable->dyl = dylNew;
	mov	[si.dylDr],ax
	jmp	short LLoop2Next
; }
; else
UT725:
; pdrTable->fCantGrow = fFalse;
	and	[si.fCantGrowDr],not maskFCantGrowDr
; pdrTable->fDirty = fFalse;
	and	[si.fDirtyDr],not maskFDirtyDr
; }
LLoop2Next:
	xchg	ax,si	; save dylNew
	call	LN_FreePdrf
	pop	cx	; restore itcTable
	jcxz	LLoop2Exit
	jmp	short LLoop2Body
LLoop2Exit:
; dylNew += dylAbove + dylBelow;
; native note: didn't store the transient value in [dylNew], so no
; need to restore it...
; }

LUpdateHeight:
; /* update hpldr and edl to correct height now for second scan,
; old height still in dylOld */
; (*hpldrTable)->dyl = edl.dyp = dylNew;
	mov	ax,[dylNew]
	mov	[edl.dypEdl],ax
	mov	bx,[hpldrTable]
	mov	bx,[bx]
	mov	[bx.dylPldr],ax

; if (dylNew == dylOld || (!fSomeDrDirty && !fLastRow))
	; ax = dylNew
	cmp	ax,[dylOld]
	je	UT730
	mov	al,[fSomeDrDirty]
	or	al,[fLastRow]
	jnz	UT735
UT730:
; goto LFinishTable;
;PAUSE
	jmp	LFinishTable
UT735:

; if (fOverflowDrs = (fPageView && dylNew > dylLimPldr - dylBelow))
;PAUSE
	xor	cx,cx	; clear high byte
	mov	cl,[fPageView]
	mov	ax,[dylNew]
	sub	ax,[dylLimPldr]
	add	ax,[dylBelow]
	jg	UT740
	xor	cx,cx
UT740:
	mov	[fOverflowDrs],cl
	jcxz	LChkLastRow
; {
; pdrT = PdrFetchAndFree(hpldr,idr,&drfFetch);
	; di = &drfFetch
;PAUSE
	push	[hpldr]
	push	[idr]
	push	di
ifdef	DEBUG
	cCall	S_PdrFetchAndFree,<>
else
	cCall	N_PdrFetchAndFree,<>
endif
	xchg	ax,si
; if ((dylNew < pdrT->dyl || pdrT->lrk == lrkAbs) && !pdrT->fCantGrow)
	mov	ax,[dylNew]
	cmp	ax,[si.dylDr]
	jl	UT745
	cmp	[si.lrkDr],lrkAbs
	jne	UT750
UT745:
	test	[si.fCantGrowDr],maskFCantGrowDr
	jnz	UT750
; /* force a repagination next time through */
; {
; /* to avoid infinite loops, we won't repeat if the
; DRs are brand new */
; pwwd = PwwdWw(ww);
;PAUSE
	call	LN_PwwdWw
	xchg	ax,bx
; pwwd->fDrDirty |= !pwwd->fNewDrs;
	test	[bx.fNewDrsWwd],maskfNewDrsWwd
	jnz	UT750
	or	[bx.fDrDirtyWwd],maskfDrDirtyWwd
UT750:
; }
; for ( idrTable = itcMac; idrTable--; FreePdrf(&drfFetch))
	mov	si,[dylLimDr]
	mov	cx,[itcMac]
LLoop3Body:
	dec	cx
	push	cx	; save idrTable
; {
; pdrTable = PdrFetch(hpldrTable, idrTable, &drfFetch);
	; cx = idrTable, si = dylLimDr, di = &drfFetch
	call	LN_PdrFetch
; REVIEW - do I need to specify the stack segment?
	pop	cx	; recover idrTable
	push	cx	; save it back again
; if (pdrTable->dyl > dylLimDr)
	; cx = idrTable, ax = dylLimDr, si = pdrTable, di = &drfFetch
	cmp	ax,[si.dylDr]
	jge	UT760
; pdrTable->dyl = dylLimDr;
	mov	[si.dylDr],ax
UT760:
	pop	cx
	push	cx
	xchg	ax,si	; save dylLimDr in si
	call	LN_FreePdrf
	pop	cx	; restore idrTable
	jcxz	UT770
	jmp	short LLoop3Body
UT770:
; }
; /* doctor dylNew and fix earlier mistake */
; dylNew = dylLimPldr;
	mov	ax,[dylLimPldr]
	mov	[dylNew],ax
; (*hpldrTable)->dyl = edl.dyp = dylNew;
	; ax = dylNew
	mov	[edl.dypEdl],ax
	mov	bx,[hpldrTable]
	mov	bx,[bx]
	mov	[bx.dylPldr],ax
; }

LChkLastRow:
; if (fLastRow && fFrameLines)
	cmp	[fLastRow],0
	jz	UT800
	cmp	[fFrameLines],0
	jnz	UT810
UT800:
;PAUSE
	jmp	LSecondPass
UT810:
; {
; /* Here we decide what cells have to be drawn because of the bottom frame
; /* line. State: All cells are updated to the original height of the edl,
; /* the last EDL of every cell that was drawn in the first update pass
; /* has been set dirty (even if the fDirty bit for the DR is not set)
; /* and does not have a bottom frame line.
; /**/
; pwwd = PwwdWw(ww);
; fDrawBottom = dylBelow == 0 && YwFromYl(hpldrTable,dylNew) <= pwwd->rcwDisp.ywBottom;
	xor	cx,cx
	cmp	[dylBelow],cx
	jnz	UT820
	call	LN_PwwdWw
	xchg	ax,si	; save in less volatile register
	push	[hpldrTable]
	push	[dylNew]
	cCall	YwFromYl,<>
	xor	cx,cx
	cmp	ax,[si.rcwDispWwd.ywBottomRc]
	jg	UT820
	errnz	<fTrue-1>
	inc	cl
UT820:
	mov	[fDrawBottom],cl

; Mac(cbBmbSav = vcbBmb);
ifdef	MAC
	move.w	vcbBmb,cbBmbSav	; mem-to-mem moves, what a concept!!
endif
; fLastDlDirty = fFalse;
	mov	[fLastDlDirty],fFalse
; for ( idrTable = 0; idrTable < itcMac/*skip TTP*/; FreePdrf(&drfFetch),++idrTable )
; native note: run the loop in the other direction...
;PAUSE
	mov	si,[dylOld]
	mov	cx,[itcMac]
LLoop4Body:
;PAUSE
	dec	cx	; decrement idrTable
	push	cx	; save idrTable
; {
; pdrTable = PdrFetch(hpldrTable, idrTable, &drfFetch);
	; cx = idrTable, si = dylOld
	call	LN_PdrFetch
; if (pdrTable->fDirty)
	; now ax = dylOld, si = pdrTable, di = &drfFetch
	push	ax	; save dylOld for easy restore
	test	[si.fDirtyDr],maskFDirtyDr
	jz	UT840
; {
; /* If the DR is dirty, there is little chance of a frame line
; /* problem. The test below catches the possible cases.
; /**/
; if (pdrTable->yl + pdrTable->dyl >= dylOld && dylOld < dylNew)
	; ax = dylOld, si = pdrTable, di = &drfFetch
;PAUSE
	mov	cx,[si.ylDr]
	add	cx,[si.dylDr]
	cmp	cx,ax
	jl	UT830
	cmp	ax,[dylNew]
	jge	UT830
; dylBottomFrameDirty = 1;
	mov	[dylBottomFrameDirty],1
; continue;
UT830:
;PAUSE
	jmp	short LLoop4Cont
; }
UT840:

; dylDr = pdrTable->yl + pdrTable->dyl;
; GetPlc ( pdrTable->hplcedl, dlLast = IMacPlc(pdrTable->hplcedl) - 1, &edlLast );
	call	LN_IMacPlcTable
	dec	ax
	mov	[dlLast],ax
	xchg	ax,cx
	call	LN_GetPlcTable
	mov	ax,[si.ylDr]
	add	ax,[si.dylDr]
	mov	cx,ax
	pop	ax	; restore dylOld
	push	ax	; re-save it
; /* does the last dl of the cell lack a needed bottom frame ? */
; if ( (dylDr == dylNew && dylNew < dylOld && fDrawBottom)
	; ax = dylOld, cx = dylDr, si = pdrTable, di = &drfFetch
	cmp	cx,[dylNew]
	jne	UT850
	cmp	cx,ax	; native note by first condition, cx = dylDr = dylNew
	jge	UT850
	cmp	[fDrawBottom],0
	jnz	UT860
; /* OR does the last dl have an unneeded bottom frame? */
; || (dylDr == dylOld && dylOld < dylNew ) )
UT850:
	; ax = dylOld, cx = dylDr, si = pdrTable, di = &drfFetch
;PAUSE
	cmp	ax,cx
	jne	UT870
	cmp	ax,[dylNew]
	jge	UT870
;PAUSE
UT860:
; {
; /* verbose for native compiler bug */
; fLastDlDirty = fTrue;
;PAUSE
	mov	[fLastDlDirty],fTrue
; edlLast.fDirty = fTrue;
	or	[edlLast.fDirtyEdl],maskFDirtyEdl
; pdrTable->fDirty = fTrue;
	or	[si.fDirtyDr],maskFDirtyDr
; PutPlcLast ( pdrTable->hplcedl, dlLast, &edlLast );
ifdef	DEBUG
	push	[si.hplcedlDr]
	push	[dlLast]
	lea	ax,[edlLast]
	push	ax
	cCall	PutPlcLastDebugProc,<>
else	; !DEBUG
	call	LN_PutPlcLast
endif	; DEBUG
UT870:
; }
LLoop4Cont:
	; now di = &drfFetch; dylOld and idrTable stored on stack
	call	LN_FreePdrf
	pop	si	; recover dylOld
	pop	cx	; recover idrTable
	jcxz	UT900
	jmp	short LLoop4Body
UT900:
; } /* end of for pdrTable loop */

; /* If we are doing the second update scan only to redraw the last
; /* rows with the bottom border, then have DisplayFli use an
; /* off-screen buffer to avoid flickering.
; /**/
; if ( fLastDlDirty && !fSomeDrDirty )
; {
; Mac(vcbBmb = vcbBmbPerm);
; fSomeDrDirty = fTrue;
; }
; native note: because we don't have to play games with an offscreen
; buffer, the net effect of the above for !MAC is
;	fSomeDrDirty |= fLastDlDirty;
	mov	al,[fLastDlDirty]
	or	[fSomeDrDirty],al
; }

LSecondPass:
; /* If some DRs were not fully updated, finish updating them */
; if (fSomeDrDirty)
	cmp	[fSomeDrDirty],0
	jnz	UT905
;PAUSE
	jmp	LFinishTable
; {
; Break3();	some dopey mac thing
; /* If there's a potentially useful dl that's about to get
; /* blitzed, move it down some.
; /**/
; if (dylNew > dylOld && dlOld < dlMac && dlNew < dlOld && fScrollOK)
UT905:
	mov	ax,[dylNew]
	cmp	ax,[dylOld]
	jle	LSetRcwInval
;PAUSE
	mov	ax,[dlOld]
	cmp	ax,[dlMac]
	jge	LSetRcwInval
	cmp	ax,[dlNew]
	jle	LSetRcwInval
	cmp	[fScrollOK],0
	jz	LSetRcwInval
; {
; GetPlc ( hplcedl, dlOld, &edlNext );
	; ax = dlOld
	xchg	ax,cx
	lea	ax,[edlNext]
	call	LN_GetPlcParent
; if (edlNext.ypTop < *pypTop + dylNew)
	mov	ax,[ypTop]
	add	ax,[dylNew]
	cmp	ax,[edlNext.ypTopEdl]
	jle	UT910
; ScrollDrDown ( ww, hpldr, idr, hplcedl, dlOld,
; dlOld, edlNext.ypTop, *pypTop + dylNew,
; ypFirstShow, ypLimWw, ypLimDr);
	; ax = *pypTop + dylNew
	push	[ww]
	push	[hpldr]
	push	[idr]
	push	[hplcedl]
	push	[dlOld]
	push	[dlOld]
	push	[edlNext.ypTopEdl]
	push	ax
	push	[ypFirstShow]
	push	[ypLimWw]
	push	[ypLimDr]
	cCall	ScrollDrDown,<>
UT910:
; dlMac = IMacPlc ( hplcedl );
	mov	ax,[hplcedl]
	call	LN_IMacPlc
	mov	[dlMac],ax
; }
; 
; rcwTableInval.ywTop += *pypTop + min(dylNew,dylOld) - dylBottomFrameDirty;
LSetRcwInval:
	mov	ax,[dylNew]
	cmp	ax,[dylOld]
	jle	UT920
	mov	ax,[dylOld]
UT920:
	add	ax,[ypTop]
	sub	ax,[dylBottomFrameDirty];
	add	[rcwTableInval.ywTopRc],ax
; rcwTableInval.ywBottom += *pypTop + max(dylNew,dylOld);
	mov	ax,[dylNew]
	cmp	ax,[dylOld]
	jge	UT930
	mov	ax,[dylOld]
UT930:
	add	ax,[ypTop]
	add	[rcwTableInval.ywBottomRc],ax
; 
; DrawEndmark ( ww, hpldr, idr, *pypTop + edl.dyp, *pypTop + dylNew, emkBlank );
	push	[ww]
	push	[hpldr]
	push	[idr]
	mov	ax,[ypTop]
	add	ax,[edl.dypEdl]
	push	ax
	mov	ax,[ypTop]
	add	ax,[dylNew]
	push	ax
	errnz	<emkBlank>
	xor	ax,ax
	push	ax
	cCall	DrawEndMark,<>
; 
; for ( idrTable = 0; idrTable < idrMacTable; ++idrTable )
; native note: try the backwards loop and see how it looks...
;PAUSE
	mov	cx,[idrMacTable]
LLoop5Body:
	dec	cx
	push	cx	; save idrTable
; {
; pdrTable = PdrFetch(hpldrTable,idrTable,&drfFetch);
	; cx = idrTable
	call	LN_PdrFetch
; if ( pdrTable->fDirty ) 
	test	[si.fDirtyDr],maskFDirtyDr
	jz	LLoop5Cont
; {
; /* Since we have already filled in the PLCEDL for the DR,
; /* the chance of this failing seems pretty small. It won't
; /* hurt to check, however.
; /**/
; if (fIncr || !FUpdTableDr(ww,hpldrTable,idrTable))
; if (!FUpdateDr(ww,hpldrTable,idrTable,rcwTableInval,fFalse,udmodTable,cpNil))
; goto LFreeAndAbort;
; NOTE: the local routine also does the assert with DypHeightTc
	mov	al,[fIncr]	; flag to helper routine
	pop	cx		; recover idrTable
	push	cx		; re-save idrTable
	lea	bx,[rcwTableInval]
;PAUSE
	call	LN_FUpdateOneDr
	jnz	UT950
;PAUSE
	pop	cx
	jmp	LFreeAndAbortUT
UT950:
; if (fOverflowDrs && pdrTable->dyl > dylLimDr)
;PAUSE
	cmp	[fOverflowDrs],0
	jz	UT960
;PAUSE
	mov	ax,[dylLimDr]
	cmp	ax,[si.dylDr]
	jge	UT960
; pdrTable->dyl = dylLimDr;
	mov	[si.dylDr],ax
UT960:
; }
LLoop5Cont:
; Win(fRMark |= pdrTable->fRMark;)
;PAUSE
	test	[si.fRMarkDr],maskFRMarkDr
	je	UT965
	mov	[fRMark],fTrue
UT965:
; FreePdrf(&drfFetch);
	; di = &drfFetch
	call	LN_FreePdrf
	pop	cx
	jcxz	UT970
	jmp	short LLoop5Body
; }
UT970:	; ick! Mac stuff
; if ( fLastRow )
; Mac(vcbBmb = cbBmbSav);
; } /* end of if fSomeDrDirty */

LFinishTable:
; /* Now, clear out bits in edl not already drawn and draw cell
; /* borders.
; /**/
; Assert(edl.dyp == dylNew);
;PAUSE
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[edl.dypEdl]
	cmp	ax,[dylNew]
	je	UT980
	mov	ax,midDisptbn2
	mov	bx,2324
	cCall	AssertProcForNative,<ax,bx>
UT980:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG
; FrameTable(ww,doc,caTapCur.cpFirst,hpldrTable, dylNew, fFirstRow, fLastRow );
	push	[ww]
	push	[doc]
	push	[caTapCur.HI_cpFirstCa]
	push	[caTapCur.LO_cpFirstCa]
	push	[hpldrTable]
	lea	ax,[edl]
	push	ax
	xor	ax,ax
	mov	al,[fFirstRow]
	push	ax
	mov	al,[fLastRow]
	push	ax
	cCall	FrameTable,<>

; /* update edl and pldrT to reflect what we have done */
; edl.fDirty = edl.fTableDirty = fFalse;
	errnz	<fDirtyEdl-fTableDirtyEdl>
	and	[edl.fDirtyEdl],not (maskFDirtyEdl or maskFTableDirtyEdl)
; edl.dcp = cp - CpPlc(hplcedl,dlNew);
	push	[hplcedl]
	push	[dlNew]
	cCall	CpPlc,<>
	mov	bx,[OFF_cp]
	mov	cx,[SEG_cp]
	sub	bx,ax
	sbb	cx,dx
	; now, <bx,cx> = cp - CpPlc()
	mov	[edl.LO_dcpEdl],bx
	mov	[edl.HI_dcpEdl],cx
;
; /* because of the way borders and frame lines are drawn,
; /* a table row always depends on the next character
; /**/
; edl.dcpDepend = 1;
	mov	[edl.dcpDependEdl],1
; 		
; PutPlc ( hplcedl, dlNew, &edl );
	call	LN_PutPlc

ifdef WIN
ifndef BOGUS
; if (fRMark)
;   DrawTableRevBar(ww, idr, dlNew);
;PAUSE
	cmp	[fRMark],0
	jz	UT990
	push	[ww]
	push	[idr]
	push	[dlNew]
	cCall	DrawTableRevBar,<>
UT990:
else ; BOGUS
; if (vfRevBar)
	mov	cx,[vfRevBar]
	jcxz	UT1190
; {
; int xwRevBar;
; 
; if (!(pwwd = PwwdWw(ww))->fPageView)
;#define dxwSelBarSci		vsci.dxwSelBar
;PAUSE
	push	di	; save &drfFetch
	mov	di,[vsci.dxwSelBarSci]	; store value is safe register
	sar	di,1			; for use later
	call	LN_PwwdWw
	xchg	ax,si
	test	[si.fPageViewWwd],maskfPageViewWwd
	jnz	UT1100
; xwRevBar = pwwd->xwSelBar + dxwSelBarSci / 2;
	; si = pwwd, di = [dxwSelBarSci]/2
	mov	ax,[si.xwSelBarWwd]
	jmp	short UT1180
; else
; {
; switch (PdodMother(doc)->dop.irmBar)
UT1100:
	; si = pwwd, di = [dxwSelBarSci]/2
	push	[doc]
	cCall	N_PdodMother,<>
	xchg	ax,bx
	mov	al,[bx.dopDod.irmBarDop]
	errnz	<irmBarLeft-1>
	dec	al
	je	UT1130
	errnz	<irmBarRight-irmBarLeft-1>
	dec	al
	je	UT1140
; #ifdef DEBUG
; default:
; Assert(fFalse);
; /* fall through */
; #endif
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	dec	al
	je	UT1110
	mov	ax,midDisptbn2
	mov	bx,2423
	cCall	AssertProcForNative,<ax,bx>
UT1110:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG
; {
; case irmBarOutside:
; if (vfmtss.pgn & 1)
; goto LRight;
	test	[vfmtss.pgnFmtss],1
	jnz	UT1140
; /* fall through */
; case irmBarLeft:
UT1130:
; xwRevBar = XwFromXp( hpldrTable, 0, edl.xpLeft )
;		- (dxwSelBarSci / 2);
	; si = pwwd, di = [dxwSelBarSci]/2
	neg	di	; so that it will get subtracted
	db	0A8h	;turns next "stc" into "test al,immediate"
			;also clears the carry flag
; break;
; case irmBarRight:
; LRight:
; xwRevBar = XwFromXp( hpldrTable, 0, edl.xpLeft + edl.dxp)
;		+ (dxwSelBarSci / 2);
UT1140:
	stc
	mov	ax,[edl.xpLeftEdl]
	jnc	UT1150
	add	ax,[edl.dxpEdl]
UT1150:
	push	[hpldrTable]
	xor	cx,cx
	push	cx
	push	ax
	cCall	XwFromXp,<>
; break;
; }
; }
; DrawRevBar( PwwdWw(ww)->hdc, xwRevBar, 
; YwFromYl(hpldrTable,0), dylNew);
UT1180:
	; ax + di = xwRevBar, si = pwwd
	add	ax,di
	; si = pwwd, ax = xwRevBar
	push	[si.hdcWwd]	; some arguments
	push	ax		; for DrawRevBar

	push	[hpldrTable]
	xor	ax,ax
	push	ax
	cCall	YwFromYl,<>

	push	ax
	push	[dylNew]
	cCall	DrawRevBar,<>
	pop	di	; restore &drfFetch
UT1190:
; }
endif	; BOGUS

; /* draw the style name area and border for a table row */
; if (PwwdWw(ww)->xwSelBar)
	call	LN_PwwdWw
	xchg	ax,si
	mov	cx,[si.xwSelBarWwd]
	jcxz	UT1200
; {
; DrawStyNameFromWwDl(ww, hpldr, idr, dlNew);
;PAUSE
	push	[ww]
	push	[hpldr]
	push	[idr]
	push	[dlNew]
	cCall	DrawStyNameFromWwDl,<>
; }
UT1200:
endif	; WIN
; /* advance the caller's cp and ypTop */
; *pcp = cp;
	mov	bx,[pcp]
	mov	ax,[OFF_cp]
	mov	[bx],ax
	mov	ax,[SEG_cp]
	mov	[bx+2],ax
; *pypTop += dylNew;
; Assert(*pypTop == ypTop);
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	bx,[pypTop]
	mov	ax,[bx]
	cmp	ax,[ypTop]
	jz	UT1210
	mov	ax,midDisptbn2
	mov	bx,2213
	cCall	AssertProcForNative,<ax,bx>
UT1210:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG
	mov	bx,[pypTop]
	mov	ax,[dylNew]
	add	[bx],ax
; return fTrue; /* it worked! */
	mov	ax,fTrue
; }
	
LExitUT:

cEnd

; NATIVE NOTE: these are short cut routines used by FUpdateTable to
; reduce size of common calling seqeuences.

; upon entry: ax = dya
; upon exit: ax = dyp, + the usual registers trashed
; #define DypFromDya(dya) NMultDiv((dya), vfti.dypInch, czaInch)
LN_DypFromDya:
;PAUSE
	push	ax
	push	[vfti.dypInchFti]
	mov	ax,czaInch
	push	ax
	cCall	NMultDiv,<>
	ret

; upon entry: di = pdrf
; upon exit: the usual things munged
LN_FreePdrf:
	push	di
ifdef	DEBUG
	cCall	S_FreePdrf,<>
else
	cCall	N_FreePdrf,<>
endif
	ret

; LN_GetPlcTable = GetPlc(((DR*)si)->hplcedl, cx, &edlLast);
; LN_GetPlcParent = GetPlc(hplcedl, cx, ax);
LN_GetPlcTable:
	lea	ax,[edlLast]
	push	[si.hplcedlDr]
	jmp	short UT2000
LN_GetPlcParent:
	push	[hplcedl]
UT2000:
	push	cx
	push	ax
	cCall	GetPlc,<>
	ret

; upon entry: si = pdrTable
; upon exit: ax = IMacPlc(pdrTable->hplcedl);
; 	plus the usual things munged
LN_IMacPlcTable:
	mov	ax,[si.hplcedlDr]

; upon entry: ax = hplcedl
; upon exit: ax = IMacPlc(pdrTable->hplcedl);
; 	plus the usual things munged
LN_IMacPlc:
	push	ax
	cCall	IMacPlc,<>
	ret

; call this one for PdrFetch(hpldrTable, idrTable, &drfFetch);
; upon entry: di = &drfFetch
; upon exit: ax = old si value, si = pdr  NOTE RETURN VALUE IN SI!!!!
; 	plus the usual things munged
LN_PdrFetchTable:
	mov	cx,[idrTable]
; call this one for PdrFetch(hpldrTable, ax, &drfFetch);
; upon entry: cx = idr, di = &drfFetch
; upon exit: ax = old si value, si = pdr  NOTE RETURN VALUE IN SI!!!!
; 	plus the usual things munged
LN_PdrFetch:
	push	[hpldrTable]
	push	cx
	push	di
ifdef	DEBUG
	cCall	S_PdrFetch,<>
else
	cCall	N_PdrFetch,<>
endif
	xchg	ax,si
	ret

; does PutPlc(hplcedl, dlNew, &edl)
LN_PutPlc:
	push	[hplcedl]
	push	[dlNew]
	lea	ax,[edl]
	push	ax
	cCall	PutPlc,<>
	ret

ifndef	DEBUG
LN_PutPlcLast:
	cCall	PutPlcLastProc,<>
	ret
endif	; not DEBUG

LN_PwwdWw:
	push	[ww]
	cCall	N_PwwdWw,<>
	ret

LN_SetFCantGrow:
; pdrTable->fCantGrow = pdrTable->dyl >= dylLimDr && (fPageView || fAbsHgtRow);
	; si = pdrTable
	and	[si.fCantGrowDr],not maskFCantGrowDr ; assume fFalse
	mov	ax,[si.dylDr]
	cmp	ax,[dylLimDr]
	jl	UT2050
;PAUSE
	mov	al,[fPageView]
	or	al,[fAbsHgtRow]
	jz	UT2050
;PAUSE
	or	[si.fCantGrowDr],maskFCantGrowDr ; assumption failed, fix it
UT2050:
	ret

LN_FUpdateOneDr:
; this is equivalent to:
; if ((al != 0) || !FUpdTableDr(ww, hpldrTable, cx=idrTable))
;	if (!FUpdateDr(ww,hpldrTable,cx,*(RC*)bx,fFalse,udmodTable,cpNil))
;		return fFalse;
; return fTrue;
;
; upon entry: ax = fTrue iff FUpdTableDr should be SKIPPED
; upon exit: ax = fTrue iff things worked, AND condition codes set accordingly
;
; if ((al != 0) || !FUpdTableDr(ww,hpldrTable,idrTable))
	or	al,al
	jnz	UT2100
	push	bx	; save prc
	push	cx	; save idrTable
	push	[ww]
	push	[hpldrTable]
	push	cx
ifdef	DEBUG
	cCall	S_FUpdTableDr,<>
else
	call	LN_FUpdTableDr
endif
	pop	cx	; recover idrTable
	pop	bx	; recover prcwInval
	or	ax,ax
	jnz	UT2110
; if (!FUpdateDr(ww,hpldrTable,idrTable,rcwTableInval,fFalse,udmodTable,cpNil))
UT2100:
;PAUSE
	push	[ww]
	push	[hpldrTable]
	push	cx
	errnz	<cbRcMin-8>
	push	[bx+6]
	push	[bx+4]
	push	[bx+2]
	push	[bx]
	xor	ax,ax
	push	ax
	errnz	<udmodTable-2>
	inc	ax
	inc	ax
	push	ax
	errnz	<LO_CpNil+1>
	errnz	<HI_CpNil+1>
	mov	ax,LO_CpNil
	push	ax
	push	ax
ifdef	DEBUG
	cCall	S_FUpdateDr,<>
else
	cCall	N_FUpdateDr,<>
endif	; DEBUG
UT2110:
ifdef ENABLE
; /* FUTURE: this is bogus for windows, since DypHeightTc returns the height
; /* in printer units, not screen units. Further, it is questionable to call
; /* DypHeightTc without setting up vlm and vflm.
; /*
; /* Enable this next line to check that FUpdate table yields the same height
; /* as does DypHeightTc using the FormatLine and PHE's, slows down redrawing.
; /**/
ifdef DEBUG
; CacheTc(ww, doc, pdrTable->cpFirst, fFirstRow, fLastRow);
;PAUSE
	push	ax
	push	bx
	push	cx
	push	dx
	push	[ww]
	push	[doc]
	push	[si.HI_cpFirstDr]
	push	[si.LO_cpFirstDr]
	xor	ax,ax
	mov	al,[fFirstRow]
	push	ax
	mov	al,[fLastRow]
	push	ax
	cCall	CacheTc,<>
; Assert ( DypHeightTc(ww,doc,pdrTable->cpFirst) == pdrTable->dyl );
	push	[ww]
	push	[doc]
	push	[si.HI_cpFirstDr]
	push	[si.LO_cpFirstDr]
	cCall	DypHeightTc,<>
	cmp	ax,[si.dylDr]
	je	UT2120
	mov	ax,midDisptbn2
	mov	bx,2391
	cCall	AssertProcForNative,<ax,bx>
UT2120:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif
endif
	or	ax,ax
	ret

;Did this DEBUG stuff with a call so as not to mess up short jumps.
;   Assert(pdrT->doc == pdrTable->doc);
ifdef	DEBUG
UT2130:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	cx,[si.docDr]
	cmp	cx,[bx.docDr]
	jz	UT2140
	mov	ax,midDisptbn2
	mov	bx,2869
	cCall	AssertProcForNative,<ax,bx>
UT2140:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif	;DEBUG

; /*	F  U P D A T E  T A B L E  D R
; /*
; /*  Description: Attempt to handle the most common table dr update
; /*  in record time by simply calling FormatLine and DisplayFli.
; /*  If simple checks show we have updated the entire DR, return
; /*  fTrue, else record the EDL we wrote and return fFalse so that
; /*  FUpdateDr() gets called to finish the job.
; /**/
; %%Function:FUpdTableDr %%Owner:tomsax
; NATIVE FUpdTableDr(ww, hpldr, idr)
; int ww;
; struct PLDR **hpldr;
; int idr;
; {
; 	int dlMac;
; 	BOOL fSuccess, fOverflow; native note: fOverflow not necessary
; 	struct PLC **hplcedl;     native note: won't be using this one
; 	struct DR *pdr;           native note: register variable
; 	struct WWD *pwwd;
; 	struct EDL edl;
; 	struct DRF drf;
;
; register usage:
;	si = pdr
;

ifdef	DEBUG
cProc	N_FUpdTableDr,<PUBLIC,FAR>,<si,di>
else
cProc	LN_FUpdTableDr,<PUBLIC,NEAR>,<si,di>
endif
	ParmW	ww
	ParmW	hpldr
	ParmW	idr

	LocalW	dlMac
	LocalW	rgf		; first byte is fSuccess, second is fOverflow
	LocalW	pwwd
	LocalV	edl,cbEdlMin
	LocalV	drf,cbDrfMin

cBegin

; pdr = PdrFetch(hpldr, idr, &drf);
;PAUSE
	push	[hpldr]
	push	[idr]
	lea	bx,[drf]
	push	bx
ifdef	DEBUG
	cCall	S_PdrFetch,<>
else
	cCall	N_PdrFetch,<>
endif
	xchg	ax,si

; fSuccess = fFalse;
	; native note: di is used as a zero register
	xor	di,di
	; native note: fOverflow also initialized to zero
	errnz <fFalse>
	mov	[rgf],di
; hplcedl = pdr->hplcedl;	; native note, forget this...
; Assert(hplcedl != hNil && (*hplcedl)->iMax > 1);
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	[si.hplcedlDr],hNil
	jz	UTD010
	mov	bx,[si.hplcedlDr]
	mov	bx,[bx]
	cmp	[bx.iMaxPlc],1
	jg	UTD020
UTD010:
	mov	ax,midDisptbn2
	mov	bx,2809
	cCall	AssertProcForNative,<ax,bx>
UTD020:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG
; if ((dlMac = IMacPlc(hplcedl)) > 0)
	; di = 0
	mov	bx,[si.hplcedlDr]
	mov	bx,[bx]
	cmp	[bx.iMacPlcSTR],di
	jle	UTD040
; {
; GetPlc(hplcedl, 0, &edl);
;PAUSE
	; di = 0
	push	[si.hplcedlDr]
	push	di	; 0
	lea	ax,[edl]
	push	ax
	cCall	GetPlc,<>
; Assert(edl.hpldr == hNil);	/* no need to FreeEdl */
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	[edl.hpldrEdl],hNil
	je	UTD030
	mov	ax,midDisptbn2
	mov	bx,2839
	cCall	AssertProcForNative,<ax,bx>
UTD030:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	; DEBUG
; if (!edl.fDirty)
; goto LRet;
	test	[edl.fDirtyEdl],maskFDirtyEdl
	jnz	UTD040
;PAUSE
UTDTemp:
	jmp	LExitUTD
; }
UTD040:
; FormatLineDr(ww, pdr->cpFirst, pdr);
	; di = 0
	push	[ww]
	push	[si.HI_cpFirstDr]
	push	[si.LO_cpFirstDr]
	push	si
ifdef	DEBUG
	cCall	S_FormatDrLine,<>
else
	cCall	N_FormatDrLine,<>
endif
        
; /* cache can be blown by FormatLine */
; CpFirstTap(pdr->doc, pdr->cpFirst);
	; di = 0
	push	[si.docDr]
	push	[si.HI_cpFirstDr]
	push	[si.LO_cpFirstDr]
	cCall	CpFirstTap,<>

; pwwd = PwwdWw(ww);	/* verbose for native compiler bug */
; fOverflow = vfli.dypLine > pdr->dyl;
; native note: not a problem here! wait till we need it to compute it
; same for fOverflow

; if (fSuccess = vfli.fSplatBreak
	; fSuccess set to false above...
	; di = 0
	test	[vfli.fSplatBreakFli],maskFSplatBreakFli
	jz	UTDTemp
; && vfli.chBreak == chTable
;PAUSE
	cmp	[vfli.chBreakFli],chTable
	jne	UTDTemp
; && (!fOverflow
;PAUSE
	mov	cx,[vfli.dypLineFli]
	cmp	cx,[si.dylDr]
	jle	UTD060
	errnz	<maskFDirtyDr-1>
	inc	bptr [rgf+1]	; sleazy bit trick, fill in the bit we want
; || idr == vtapFetch.itcMac
;PAUSE
	mov	ax,[idr]
	cmp	ax,[vtapFetch.itcMacTap]
	je	UTD060
; || YwFromYp(hpldr,idr,pdr->dyl) >= pwwd->rcwDisp.ywBottom))
;PAUSE
	; di = 0
	push	[ww]
	cCall	N_PwwdWw,<>
	push	ax	; we'll want this in a second here...
	push	[hpldr]
	push	[idr]
	push	[si.dylDr]
	cCall	YwFromYp,<>
	pop	bx	; recover pwwd from above
	cmp	ax,[bx.rcwDispWwd.ywBottomRc]
	jl	LExitUTD
;PAUSE
UTD060:
	errnz	<fTrue-1>
	inc	bptr [rgf]	; fSuccess = fTrue ** we win!!!
; {
; DisplayFli(ww, hpldr, idr, 0, vfli.dypLine);
	; di = 0
	push	[ww]
	push	[hpldr]
	push	[idr]
	push	di
	push	[vfli.dypLineFli]
ifdef	DEBUG
	cCall	S_DisplayFli,<>
else
	cCall	N_DisplayFli,<>
endif
; pdr->dyl = vfli.dypLine;
	mov	ax,[vfli.dypLineFli]
	mov	[si.dylDr],ax
; pdr->fDirty = fOverflow;
	and	[si.fDirtyDr],not maskFDirtyDr
	mov	al,bptr [rgf+1]
	or	[si.fDirtyDr],al
; pdr->fRMark = vfli.fRMark;
;PAUSE
	and	[si.fRMarkDr],not maskFRMarkDr
	test	[vfli.fRMarkFli],maskFRMarkFli
	je	UTD070
	or	[si.fRMarkDr],maskFRMarkDr
UTD070:

; DlkFromVfli(hplcedl, 0);
; PutCpPlc(hplcedl, 1, vfli.cpMac);
; PutIMacPlc(hplcedl, 1);
;PAUSE
	; di = 0
	mov	bx,[si.hplcedlDr]  ; arguments
	push	bx
	inc	di		; == 1
	push	di              ; PutIMacPlc

	push	bx		; arguments for
	push	di              ; PutCpPlc
	push	[vfli.HI_cpMacFli]
	push	[vfli.LO_cpMacFli]

	push	bx
	dec	di
	push	di
	cCall	DlkFromVfli,<>
	cCall	PutCpPlc,<>
	cCall	PutIMacPlc,<>
; }

; LRet:
LExitUTD:
; FreePdrf(&drf);
	lea	ax,[drf]
	push	ax
ifdef	DEBUG
	cCall	S_FreePdrf,<>
else
	cCall	N_FreePdrf,<>
endif
; return fSuccess;
	mov	al,bptr [rgf]
	cbw
; }
cEnd

; /*	F R A M E  E A S Y  T A B L E
; /*	Description: Under certain (common) circumstances, it is possible
; /*	to draw the table borders without going through all of the hoops
; /*	to correctly join borders at corners. This routines handles those
; /*	cases.
; /*
; /*	Also uses info in caTap, vtapFetch and vtcc (itc = 0 cached).
; /**/
; int ww;
; struct PLDR **hpldr;
; struct DR *pdrParent;
; int dyl;
; BOOL fFrameLines, fDrFrameLines, fFirstRow, fLastRow;
; HANDNATIVE C_FrameEasyTable(ww, doc, cp, hpldr, prclDrawn, pdrParent, dyl, fFrameLines, fDrFrameLines, fFirstRow, fLastRow)
; %%Function:FrameEasyTable %%Owner:tomsax
cProc	N_FrameEasyTable,<PUBLIC,FAR>,<si,di>
	ParmW	<ww, doc>
	ParmD	cp
	ParmW	<hpldr, prclDrawn, pdrParent, dyl>
	ParmW	<fFrameLines, fDrFrameLines, fFirstRow, fLastRow>
; {
; int dxwBrcLeft, dxwBrcRight, dxwBrcInside, dywBrcTop, dywBrcBottom;
	LocalW	<dxwBrcLeft,dxwBrcRight,dxwBrcInside,dywBrcTop,dywBrcBottom>
; int dxLToW, dyLToW;
	LocalW	<dxLToW, dyLToW>
; int xwLeft, xwRight, dylDrRow, dylDrRowM1, ywTop;
	; native note: dylDrRowM1 used only in Mac version
	LocalW	<xwLeft, xwRight, dylDrRow, ywTop>
; int itc, itcNext, itcMac;
	LocalW	<itc, itcNext, itcMac>
; int brcCur;
	LocalW	brcCur
; BOOL fRestorePen, fBottomFrameLine;
	LocalW	<fRestorePen, fBottomFrameLine>
; struct DR *pdr;                       ; native note: registerized
; struct RC rclErase, rcw;
	LocalV	rclErase,cbRcMin
	LocalV	rcw,cbRcMin
; struct TCX tcx;
	LocalV	tcx,cbTcxMin
; struct DRF drf;
	LocalV	drf,cbDrfMin
; #ifdef WIN
; HDC hdc = PwwdWw(ww)->hdc;
	LocalW	hdc
; struct RC rcDraw, rcwClip;
	LocalV	rcDraw,cbRcMin
	LocalV	rcwClip,cbRcMin
; int xwT, ywT;                         ; native note: registerized
; struct WWD *pwwd = PwwdWw(ww);	; native note: registerized
cBegin
;PAUSE
	; native note: do auto initializations...
	call	LN_PwwdWwFET

	mov	ax,[bx.hdcWwd]
	mov	[hdc],ax
; 
; if (pwwd->fPageView)
	; bx = pwwd
	lea	di,[rcwClip]
	test	[bx.fPageViewWwd],maskFPageViewWwd
	jz	FET010
; RcwPgvTableClip(ww, (*hpldr)->hpldrBack, (*hpldr)->idrBack, &rcwClip);
;PAUSE
	; di = &rcwClip
	push	[ww]
	mov	bx,[hpldr]
	mov	bx,[bx]
	push	[bx.hpldrBackPldr]
	push	[bx.idrBackPldr]
	push	di
	cCall	RcwPgvTableClip,<>
	jmp	short FET020
; else
; rcwClip = pwwd->rcwDisp;
FET010:
	; bx = pwwd, di = &rcwClip
	lea	si,[bx.rcwDispWwd]
	push	ds
	pop	es
	errnz	<cbRcMin-8>
	movsw
	movsw
	movsw
	movsw
FET020:
; #endif
; 
; Assert(FInCa(doc, cp, &vtcc.ca));
; Assert(vtcc.itc == 0);
; Assert(dyl > 0);
;PAUSE
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	
	push	[doc]
	push	[SEG_cp]
	push	[OFF_cp]
	mov	ax,dataoffset [vtcc.caTcc]
	push	ax
	cCall	FInCa,<>
	or	ax,ax
	mov	bx,3088	; assert line number
	jz	FET030

	cmp	[vtcc.itcTcc],0
	mov	bx,3092	; assert line number
	jnz	FET030

	mov	ax,[dyl]
	or	ax,ax
	jg	FET035
	mov	bx,3097	; assert line number
FET030:
	mov	ax,midDisptbn2
	cCall	AssertProcForNative,<ax,bx>
FET035:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	;DEBUG

; native note: we clear fRestorePen here so we don't have it clear
; it every time through the loops.
	xor	ax,ax
	mov	[fRestorePen],ax
; dxLToW = XwFromXl(hpldr, 0);
; dyLToW = YwFromYl(hpldr, 0);
	; ax = 0
	push	[hpldr] ; arguments
	push	ax      ; for YwFromYl
	push	[hpldr] ; arguments
	push	ax      ; for XwFromXl
	cCall	XwFromXl,<>
	mov	[dxLToW],ax
	cCall	YwFromYl,<>
	mov	[dyLToW],ax
; 
; LN_DxFromIbrc Does:
;     DxFromBrc(*(int*)(si + rgbrcEasyTcc + bx), fFrameLines)
	lea	si,[vtcc]
; dxwBrcLeft = DxFromBrc(vtcc.rgbrcEasy[ibrcLeft],fTrue/*fFrameLines*/);
	mov	bx,ibrcLeft SHL 1
	call	LN_DxFromIbrc
	mov	[dxwBrcLeft],ax
; dxwBrcRight = DxFromBrc(vtcc.rgbrcEasy[ibrcRight],fTrue/*fFrameLines*/);
	mov	bx,ibrcRight SHL 1
	call	LN_DxFromIbrc
	mov	[dxwBrcRight],ax
; dxwBrcInside = DxFromBrc(vtcc.rgbrcEasy[ibrcInside],fTrue/*fFrameLines*/);
	mov	bx,ibrcInside SHL 1
	call	LN_DxFromIbrc
	mov	[dxwBrcInside],ax

; dywBrcTop = vtcc.dylAbove;
; dywBrcBottom = vtcc.dylBelow;
	; si = &vtcc
	mov	ax,[si.dylAboveTcc]
	mov	[dywBrcTop],ax
	mov	cx,[si.dylBelowTcc]
	mov	[dywBrcBottom],cx
	; leave si = &vtcc, ax = dywBrcTop, cx = dywBrcBottom
; 
; Assert(dywBrcTop == DyFromBrc(vtcc.rgbrcEasy[ibrcTop],fFalse/*fFrameLines*/));
; Assert(dywBrcBottom == (fLastRow ? DyFromBrc(vtcc.rgbrcEasy[ibrcBottom],fFalse/*fFrameLines*/) : 0));
; #define DyFromBrc(brc, fFrameLines) DxyFromBrc(brc, fFrameLines, fFalse)
; #define DxyFromBrc(brc, fFrameLines, fWidth)	\
;   WidthHeightFromBrc(brc, fFrameLines | (fWidth << 1))
ifdef	DEBUG
	; si = &vtcc, ax = dywBrcTop, cx = dywBrcBottom
	push	ax
	push	bx
	push	cx
	push	dx

	mov	bx,ibrcTop SHL 1
	push	[bx.si.rgbrcEasyTcc]
	xor	ax,ax
	push	ax
ifdef	DEBUG
	cCall	S_WidthHeightFromBrc,<>
else
	cCall	N_WidthHeightFromBrc,<>
endif
	cmp	ax,[dywBrcTop]
	mov	bx,3158	; assert line number
	jne	FET040

	mov	cx,[fLastRow]
	jcxz	FET050
;PAUSE
	mov	bx,ibrcBottom SHL 1
	push	[bx.si.rgbrcEasyTcc]
	xor	ax,ax
	push	ax
ifdef	DEBUG
	cCall	S_WidthHeightFromBrc,<>
else
	cCall	N_WidthHeightFromBrc,<>
endif
	cmp	ax,[dywBrcBottom]
	mov	bx,3171	; assert line number
	je	FET050
FET040:
	mov	ax,midDisptbn2
	cCall	AssertProcForNative,<ax,bx>
FET050:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	;DEBUG
; 
; ywTop = dyLToW + dywBrcTop;
	; si = &vtcc, ax = dywBrcTop, cx = dywBrcBottom
	mov	dx,ax
	add	dx,[dyLToW]
	mov	[ywTop],dx
; dylDrRow = dyl - dywBrcTop - dywBrcBottom;
	add	cx,ax
	mov	ax,[dyl]
	sub	ax,cx
	mov	[dylDrRow],ax
; dylDrRowM1 = dylDrRow - 1;	naitve note: needed only in Mac version
; itcMac = vtapFetch.itcMac;
	mov	ax,[vtapFetch.itcMacTap]
	mov	[itcMac],ax

; /* erase bits to the left of the PLDR */
	lea	di,[rclErase]
	push	ds
	pop	es
; rclErase.xlLeft = - pdrParent->dxpOutLeft;
	errnz	<xlLeftRc>
	mov	bx,[pdrParent]
	mov	ax,[bx.dxpOutLeftDr]
	neg	ax
	stosw
; rclErase.ylTop = 0;
	errnz	<ylTopRc-2-xlLeftRc>
	xor	ax,ax
	stosw
; rclErase.xlRight = vtcc.xpDrLeft - vtcc.dxpOutLeft - dxwBrcLeft;
	errnz	<xlRightRc-2-ylTopRc>
	mov	ax,[si.xpDrLeftTcc]
	sub	ax,[si.dxpOutLeftTcc]
	sub	ax,[dxwBrcLeft]
	stosw
	xchg	ax,cx	; stash for later
; rclErase.ylBottom = dyl;
	errnz	<ylBottomRc-2-xlRightRc>
	mov	ax,[dyl]
	stosw
; xwLeft = rclErase.xlRight + dxLToW;	/* will be handy later */
	; si = &vtcc, cx = rclErase.xlRight
	add	cx,[dxLToW]
	mov	[xwLeft],cx
; if (!FEmptyRc(&rclErase))
;   ClearRclInParentDr(ww,hpldr,rclErase,&rcwClip);
	call	LN_ClearRclIfNotMT

; PenNormal();
; #define PenNormal()	/* some bogus Mac thing... */

; /* the left border */
; SetPenForBrc(ww, brcCur = vtcc.rgbrcEasy[ibrcLeft], fFalse/*fHoriz*/, fFrameLines);
;PAUSE
	mov	bx,ibrcLeft SHL 1
	call	LN_SetPenBrcV
; #ifdef MAC	
;   MoveTo(xwLeft, ywTop);
;   Line(0, dylDrRowM1);
; #else /* WIN */
;   PrcSet(&rcDraw, xwLeft, ywTop,
;     xwLeft + dxpPenBrc, ywTop + dylDrRow);
;   SectRc(&rcDraw, &rcwClip, &rcDraw);
;   FillRect(hdc, (LPRECT)&rcDraw, hbrPenBrc);
	mov	ax,[xwLeft]
	push	ax
	mov	cx,[ywTop]
	push	cx
	add	ax,[dxpPenBrc]
	push	ax
	add	cx,[dylDrRow]
	push	cx
	call	LN_SetSectAndFillRect
; #endif

; /* the inside borders */
; itc = 0;
; itcNext = ItcGetTcxCache ( ww, doc, cp, &vtapFetch, itc, &tcx);
	xor	cx,cx
	call	LN_ItcGetTcxCache
; /* pre-compute loop invariant */
; fBottomFrameLine = fLastRow && fFrameLines && dywBrcBottom == 0;
	mov	cx,[fLastRow]
	jcxz	FET055
;PAUSE
	mov	cx,[fFrameLines]
	jcxz	FET055
	; cx = 0
	xor	cx,cx
	cmp	[dywBrcBottom],cx
	jnz	FET055
	errnz	<fTrue-1>
;PAUSE
	inc	cx
FET055:
	mov	[fBottomFrameLine],cx
; 
; 
; if (itcNext >= itcMac)
	mov	cx,[itcNext]
	cmp	cx,[itcMac]
	jl	FET060
; {
; /* BEWARE the case of a single cell row, we have to hack things
; /* up a bit to avoid trying to draw a between border.
; /**/
; if (brcCur != brcNone)
;PAUSE
	errnz	<brcNone>
	mov	cx,[brcCur]
	jcxz	FET080
; SetPenForBrc(ww, brcCur = brcNone, fFalse/*fHoriz*/, fFrameLines);
	call	LN_SetPenBrcVNone
	jmp	short FET080
; }
; else if (brcCur != brcNone || brcCur != vtcc.rgbrcEasy[ibrcInside])
;   SetPenForBrc(ww, brcCur = vtcc.rgbrcEasy[ibrcInside], fFalse/*fHoriz*/, fFrameLines);
FET060:
;PAUSE
	mov	bx,ibrcInside SHL 1
	call	LN_ChkSetPenBrcV
FET080:

; for ( ; ; )
; {
LInsideLoop:
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	[fRestorePen],0
	je	FET085
	mov	ax,midDisptbn2
	mov	bx,3414	; assert line number
	cCall	AssertProcForNative,<ax,bx>
FET085:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	;DEBUG
;PAUSE
; pdr = PdrFetchAndFree(hpldr, itc, &drf);
	push	[hpldr]
	push	[itc]
	lea	ax,[drf]
	push	ax
ifdef	DEBUG
	cCall	S_PdrFetchAndFree,<>
else
	cCall	N_PdrFetchAndFree,<>
endif	; DEBUG
	xchg	ax,si
; if (pdr->dyl < dylDrRow)
	; si = pdr
	mov	ax,[dylDrRow]
	cmp	ax,[si.dylDr]
	jg	FET087
	jmp	FET120
FET087:
; {
; DrclToRcw(hpldr, &pdr->drcl, &rcw);
;PAUSE
	; si = pdr
	lea	di,[rcw]
	push	[hpldr]
	errnz	<drclDr>
	push	si
	push	di
	cCall	DrclToRcw,<>
; native note: these next four lines rearranged for efficiency...
	; si = pdr, di = &rcw
	push	ds
	pop	es
; rcw.xwLeft -= pdr->dxpOutLeft;
	errnz	<xwLeftRc>
	mov	ax,[di]
	sub	ax,[si.dxpOutLeftDr]
	stosw
; rcw.ywTop += pdr->dyl;
	errnz	<ywTopRc-2-xwLeftRc>
	mov	ax,[di]
	add	ax,[si.dylDr]
	stosw
; rcw.xwRight += pdr->dxpOutRight;
	errnz	<xwRightRc-2-ywTopRc>
	mov	ax,[di]
	add	ax,[si.dxpOutRightDr]
	stosw
; rcw.ywBottom += dylDrRow - pdr->dyl;
	errnz	<ywBottomRc-2-xwRightRc>
	mov	ax,[di]
	add	ax,[dylDrRow]
	sub	ax,[si.dylDr]
	stosw
; #ifdef WIN
; SectRc(&rcw, &rcwClip, &rcw);
	sub	di,cbRcMin
	push	di
	lea	ax,[rcwClip]
	push	ax
	push	di
; #define SectRc(prc1,prc2,prcDest) FSectRc(prc1,prc2,prcDest)
	cCall	FSectRc,<>
; #endif
; if (fBottomFrameLine)
	mov	cx,[fBottomFrameLine]
	jcxz	FET110
; {
; --rcw.ywBottom;
	; si = pdr, di = &rcw
	dec	[di.ywBottomRc]
; if (fRestorePen = brcCur != brcNone)
	; native note: fRestorePen was cleared above,
	; so we only have to clear it if we set it
	mov	cx,[brcCur]
	jcxz	FET090
;PAUSE
; SetPenForBrc(ww, brcCur = brcNone, fFalse/*fHoriz*/, fFrameLines);
	call	LN_SetPenBrcVNone
	inc	[fRestorePen]
FET090:
; #ifdef MAC
; MoveTo(rcw.xwLeft,rcw.ywBottom);
; LineTo(rcw.xwRight-1,rcw.ywBottom);
; #else /* WIN */
; FillRect(hdc, (LPRECT)PrcSet(&rcDraw,
;   rcw.xwLeft, rcw.ywBottom,
;   rcw.xwRight, rcw.ywBottom + dypPenBrc),
; hbrPenBrc);
	; si = pdr, di = &rcw
	push	[rcw.xwLeftRc]
	mov	ax,[rcw.ywBottomRc]
	push	ax
	push	[rcw.xwRightRc]
	add	ax,[dypPenBrc]
	push	ax
	call	LN_SetAndFillRect
; #endif
; if (fRestorePen)
	; si = pdr, di = &rcw
	mov	cx,[fRestorePen]
	jcxz	FET100
; SetPenForBrc(ww, brcCur = vtcc.rgbrcEasy[ibrcInside], fFalse/*fHoriz*/, fFrameLines);
;PAUSE
	mov	bx,ibrcInside SHL 1
	call	LN_SetPenBrcV
	dec	[fRestorePen]	; clear fRestorePen
	; native note: Assert(fRestorePen == fFalse);
FET100:
; if (rcw.ywTop >= rcw.ywBottom)
; goto LCheckItc;
	; si = pdr, di = &rcw
	mov	ax,[rcw.ywTopRc]
	cmp	ax,[rcw.ywBottomRc]
	jge	LCheckItc
FET110:
; }
; EraseRc(ww, &rcw);
	; si = pdr, di = &rcw
; #define EraseRc(ww, _prc)	\
;    PatBltRc(PwwdWw(ww)->hdc, (_prc), vsci.ropErase)
	call	LN_PwwdWwFET
	push	[bx.hdcWwd]
	push	di
	push	[vsci.HI_ropEraseSci]
	push	[vsci.LO_ropEraseSci]
	cCall	PatBltRc,<>
FET120:
; }
LCheckItc:
; if (itcNext == itcMac)
; break;
	mov	ax,[itcNext]
	cmp	ax,[itcMac]
	je	FET130
; #ifdef MAC
; MoveTo(tcx.xpDrRight + tcx.dxpOutRight + dxLToW, ywTop);
; Line(0, dylDrRowM1);
; #else /* WIN */
; xwT = tcx.xpDrRight + tcx.dxpOutRight + dxLToW;
	mov	cx,[tcx.xpDrRightTcx]
	add	cx,[tcx.dxpOutRightTcx]
	add	cx,[dxLToW]
; PrcSet(&rcDraw, xwT, ywTop,
; xwT + dxpPenBrc, ywTop + dylDrRow);
; SectRc(&rcDraw, &rcwClip, &rcDraw);
; FillRect(hdc, (LPRECT)&rcDraw, hbrPenBrc);
	push	cx
	mov	ax,[ywTop]
	push	ax
	add	cx,[dxpPenBrc]
	push	cx
	add	ax,[dylDrRow]
	push	ax
	call	LN_SetSectAndFillRect
; #endif
; itc = itcNext;
; itcNext = ItcGetTcxCache ( ww, doc, cp, &vtapFetch, itc, &tcx);
	mov	cx,[itcNext]
	call	LN_ItcGetTcxCache
	jmp	LInsideLoop
; }
FET130:

; LTopBorder:
; Assert(itcNext == itcMac);
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[itcNext]
	cmp	ax,[itcMac]
	je	FET140
	mov	ax,midDisptbn2
	mov	bx,3414	; assert line number
	cCall	AssertProcForNative,<ax,bx>
FET140:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	;DEBUG

; xwRight = tcx.xpDrRight + tcx.dxpOutRight + dxLToW;
	mov	ax,[tcx.xpDrRightTcx]
	add	ax,[tcx.dxpOutRightTcx]
	add	ax,[dxLToW]
	mov	[xwRight],ax

; /* top */
; if (brcCur != brcNone || brcCur != vtcc.rgbrcEasy[ibrcTop])
;   SetPenForBrc(ww, brcCur = vtcc.rgbrcEasy[ibrcTop], fTrue/*fHoriz*/, fFrameLines);
	errnz	<ibrcTop>
	xor	bx,bx
	call	LN_ChkSetPenBrcH
	
; if (dywBrcTop > 0)
	mov	cx,[dywBrcTop]
	jcxz	FET150
; {
; #ifdef MAC
; MoveTo(xwLeft, dyLToW);
; LineTo(xwRight + dxwBrcRight - 1, dyLToW);
; #else /* WIN */
; PrcSet(&rcDraw, xwLeft, dyLToW,
;   xwRight + dxwBrcRight, dyLToW + dypPenBrc);
; SectRc(&rcDraw, &rcwClip, &rcDraw);
; FillRect(hdc, (LPRECT)&rcDraw, hbrPenBrc);
	push	[xwLeft]
	mov	ax,[dyLToW]
	push	ax
	mov	cx,[xwRight]
	add	cx,[dxwBrcRight]
	push	cx
	add	ax,[dypPenBrc]
	push	ax
	call	LN_SetSectAndFillRect
; #endif
; }
FET150:

; /* right */
; if (brcCur != brcNone || brcCur != vtcc.rgbrcEasy[ibrcRight])
;   SetPenForBrc(ww, brcCur = vtcc.rgbrcEasy[ibrcRight], fFalse/*fHoriz*/, fFrameLines);
	mov	bx,ibrcRight SHL 1
	call	LN_ChkSetPenBrcV
; #ifdef MAC	
; MoveTo(xwRight, ywTop);
; Line(0, dylDrRowM1);
; #else /* WIN */
; PrcSet(&rcDraw, xwRight, ywTop,
;   xwRight + dxwBrcRight, ywTop + dylDrRow);
; SectRc(&rcDraw, &rcwClip, &rcDraw);
; FillRect(hdc, (LPRECT)&rcDraw, hbrPenBrc);
	mov	ax,[xwRight]
	push	ax
	mov	cx,[ywTop]
	push	cx
	add	ax,[dxwBrcRight]
	push	ax
	add	cx,[dylDrRow]
	push	cx
	call	LN_SetSectAndFillRect
; #endif

; /* bottom */
; if (dywBrcBottom > 0)
	mov	cx,[dywBrcBottom]
	jcxz	FET165
; {
; Assert(fLastRow);
;PAUSE
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	[fLastRow],0
	jnz	FET160
	mov	ax,midDisptbn2
	mov	bx,3568	; assert line number
	cCall	AssertProcForNative,<ax,bx>
FET160:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	;DEBUG
; if (brcCur != brcNone || brcCur != vtcc.rgbrcEasy[ibrcBottom])
;   SetPenForBrc(ww, brcCur = vtcc.rgbrcEasy[ibrcBottom], fTrue/*fHoriz*/, fFrameLines);
	mov	bx,ibrcBottom SHL 1
	call	LN_ChkSetPenBrcH
; #ifdef MAC
; MoveTo(xwLeft, dyLToW + dyl - dywBrcBottom);
; Line(xwRight + dxwBrcRight - xwLeft - 1, 0);
; #else /* WIN */
      ; ywT = dyLToW + dyl - dywBrcBottom;
	mov	cx,[dyLToW]
	add	cx,[dyl]
	sub	cx,[dywBrcBottom]
; PrcSet(&rcDraw, xwLeft, ywT,
;   xwRight + dxwBrcRight, ywT + dypPenBrc);
; SectRc(&rcDraw, &rcwClip, &rcDraw);
; FillRect(hdc, (LPRECT)&rcDraw, hbrPenBrc);
	push	[xwLeft]
	push	cx
	mov	ax,[xwRight]
	add	ax,[dxwBrcRight]
	push	ax
	add	cx,[dypPenBrc]
	push	cx
	call	LN_SetSectAndFillRect
; #endif
; }
FET165:

; /* clear any space above or below the fTtp DR */
; pdr = PdrFetchAndFree(hpldr, itcMac, &drf);
	push	[hpldr]
	push	[itcMac]
	lea	ax,[drf]
	push	ax
ifdef	DEBUG
	cCall	S_PdrFetchAndFree,<>
else
	cCall	N_PdrFetchAndFree,<>
endif
	xchg	ax,si
; DrcToRc(&pdr->drcl, &rclErase);
	lea	di,[rclErase]
	errnz	<drclDr>
	push	si
	push	di
	cCall	DrcToRc,<>
	; native note: the following lines have been rearranged
	; for more hard core native trickery...
	; di = &rclErase
	push	ds
	pop	es
; rclErase.xlLeft -= pdr->dxpOutLeft;
	errnz	<xlLeftRc>
	mov	ax,[di]
	sub	ax,[si.dxpOutLeftDr]
	stosw
; if (fFrameLines && rclErase.ylTop == 0)
;   rclErase.ylTop = dyFrameLine;
	errnz	<ylDr-2-xlDr>
	mov	ax,[di]
	mov	cx,[fFrameLines]
	jcxz	FET170
	or	ax,ax
	jnz	FET170
; #define dyFrameLine		dypBorderFti
; #define dypBorderFti		vfti.dypBorder
	mov	ax,[vfti.dypBorderFti]
FET170:
	stosw
	xchg	ax,cx	; save rclErase.ylTop
; rclErase.xlRight += pdr->dxpOutRight;
	errnz	<xlRightRc-2-ylDr>
	mov	ax,[di]
	add	ax,[si.dxpOutRightDr]
	stosw
; if (rclErase.ylTop > 0)
	; cx = rclErase.ylTop
	jcxz	FET180
; {
; rclErase.ylBottom = rclErase.ylTop;
	mov	ax,[rclErase.ylTopRc]
	mov	[rclErase.ylBottomRc],ax
; rclErase.ylTop = 0;
	sub	[rclErase.ylTopRc],ax
; ClearRclInParentDr(ww,hpldr,rclErase,&rcwClip);
	call	LN_ClearRcl
; rclErase.ylBottom += pdr->drcl.dyl;
	mov	ax,[si.drclDr.dylDrc]
	add	[rclErase.ylBottomRc],ax
; }
FET180:
; if (rclErase.ylBottom < dyl)
	mov	cx,[dyl]
	cmp	cx,[rclErase.ylBottomRc]
	jle	FET190
; {
; rclErase.ylTop = rclErase.ylBottom;
;PAUSE
	; cx = dyl
	mov	ax,[rclErase.ylBottomRc]
	mov	[rclErase.ylTopRc],ax
; rclErase.ylBottom = dyl;
	; cx = dyl
	mov	[rclErase.ylBottomRc],cx
; ClearRclInParentDr(ww,hpldr,rclErase,&rcwClip);
	call	LN_ClearRcl
; }
FET190:

;PAUSE
	mov	di,[prclDrawn]
	push	ds
	pop	es
; prclDrawn->xlLeft = xwLeft - dxLToW;
	mov	ax,[xwLeft]
	sub	ax,[dxLToW]
	stosw
; prclDrawn->ylTop = 0;
	xor	ax,ax
	stosw
; prclDrawn->xlRight = rclErase.xlRight;
	mov	ax,[rclErase.xlRightRc]
	stosw
; prclDrawn->ylBottom = dywBrcTop;
	mov	ax,[dywBrcTop]
	stosw

; /* erase bits to the right of the PLDR */
; native note: more rearranging...
	lea	di,[rclErase]
	push	ds
	pop	es
; rclErase.xlLeft = rclErase.xlRight;
	errnz	<xlLeftRc>
	mov	ax,[di.xlRightRc-(xlLeftRc)]
	stosw
; rclErase.ylTop = 0;
	errnz	<ylTopRc-2-xlLeftRc>
	xor	ax,ax
	stosw
; rclErase.xlRight = pdrParent->dxl + pdrParent->dxpOutRight;
	errnz	<xlRightRc-2-ylTopRc>
	mov	bx,pdrParent
	mov	ax,[bx.dxlDr]
	add	ax,[bx.dxpOutRightDr]
	stosw
; rclErase.ylBottom = dyl;
	errnz	<ylBottomRc-2-xlRightRc>
	mov	ax,[dyl]
	stosw
; if (!FEmptyRc(&rclErase))
;   ClearRclInParentDr(ww,hpldr,rclErase,&rcwClip);
	call	LN_ClearRclIfNotMT
; }

cEnd

; Some utility routines for FrameEasyTable...

; LN_ClearRclIfNotMT Does:
; if (!FEmptyRc(&rclErase))
;   ClearRclInParentDr(ww,hpldr,rclErase,&rcwClip);
LN_ClearRclIfNotMT:
;PAUSE
	lea	ax,[rclErase]
	push	ax
	cCall	FEmptyRc,<>
	or	ax,ax
	jnz	FED1000
LN_ClearRcl:
;PAUSE
	push	[ww]
	push	[hpldr]
	push	[rclErase.ylBottomRc]
	push	[rclErase.xlRightRc]
	push	[rclErase.YwTopRc]
	push	[rclErase.xwLeftRc]
	lea	ax,[rcwClip]
	push	ax
	cCall	ClearRclInParentDr,<>
FED1000:
	ret

; LN_DxFromIbrc Does:
;     DxFromBrc(*(int*)(si + rgbrcEasyTcc + bx), fTrue/*fFrameLines*/)
; Upon Entry: bx = ibrc, di = &vtcc.rgbrcEasy
; Upon Exit: ax = DxFromBrc(vtcc.rgbrcEasy[ibrcInside],fTrue/*fFrameLines*/);
LN_DxFromIbrc:
; #define DxFromBrc(brc, fFrameLines) DxyFromBrc(brc, fFrameLines, fTrue)
; #define DxyFromBrc(brc, fFrameLines, fWidth)	\
;   WidthHeightFromBrc(brc, fFrameLines | (fWidth << 1))
	push	[bx.si.rgbrcEasyTcc]
	mov	ax,3
	push	ax
ifdef	DEBUG
	cCall	S_WidthHeightFromBrc,<>
else
	cCall	N_WidthHeightFromBrc,<>
endif
	ret

; %%Function:ItcGetTcxCache %%Owner:tomsax
; LN_ItcGetTcxCache does three things:
; itc = cx;
; ax = ItcGetTcxCache ( ww, doc, cp, &vtapFetch, itc, &tcx);
; itcNext = ax
LN_ItcGetTcxCache:
;PAUSE
	mov	[itc],cx
	push	[ww]
	push	[doc]
	push	[SEG_cp]
	push	[OFF_cp]
	mov	ax,dataoffset [vtapFetch]
	push	ax
	push	cx
	lea	ax,[tcx]
	push	ax
ifdef	DEBUG
	cCall	S_ItcGetTcxCache
else
	cCall	N_ItcGetTcxCache
endif
	mov	[itcNext],ax
	ret

; Upon entry: who cares, apart from FrameEasyTable's stack frame...
; Upon exit: bx = PwwdWw([ww]), + the usual registers trashed...
LN_PwwdWwFET:
;PAUSE
	push	[ww]
	cCall	N_PwwdWw,<>
	xchg	ax,bx
	ret

;   PrcSet(&rcDraw, xwLeft, ywTop,
;     xwLeft + dxpPenBrc, ywTop + dylDrRow);
;   SectRc(&rcDraw, &rcwClip, &rcDraw);
;   FillRect(hdc, (LPRECT)&rcDraw, hbrPenBrc);
LN_SetSectAndFillRect:
;PAUSE
	db	0A8h	;turns next "stc" into "test al,immediate"
			;also clears the carry flag
LN_SetAndFillRect:
	stc
;PAUSE
	; native note: blt from the stack into rcwDraw.
	pop	bx	; save the return address
	mov	dx,di	; save current di
	push	ds
	pop	es
	std	; blt backwards
	errnz	<xwLeftRc>
	errnz	<ywTopRc-2-xwLeftRc>
	errnz	<xwRightRc-2-ywTopRc>
	errnz	<ywBottomRc-2-xwRightRc>
	errnz	<ywBottomRc+2-cbRcMin>
	lea	di,[rcDraw.ywBottomRc]
	pop	ax
	stosw
	pop	ax
	stosw
	pop	ax
	stosw
	pop	ax
	stosw
	cld	; clear the direction flag
	push	bx	; restore the return address
	push	dx	; save original di value
	lea	di,[di+2]	; doesn't affect condition flags
	jc	FET1100	; use that bit set above
	push	di
	lea	ax,[rcwClip]
	push	ax
	push	di
; #define SectRc(prc1,prc2,prcDest) FSectRc(prc1,prc2,prcDest)
	cCall	FSectRc,<>
FET1100:
	; bx = &rcDraw
	push	[hdc]
	push	ds
	push	di
	push	[hbrPenBrc]
	cCall	FillRect,<>
	pop	di	; restore original di
	ret

; %%Function:SetPenForBrc %%Owner:tomsax
; NATIVE SetPenForBrc(ww, brc, fHoriz, fFrameLines)
; int ww;
; int brc;
; BOOL fHoriz, fFrameLines;
; {
; extern HBRUSH vhbrGray;

; Upon Entry: bx = ibrc*2 (ignored by SetPenBrcVNone)
;LN_ChkSetPenBrcH Does:
; if (brcCur != brcNone || brcCur != vtcc.rgbrcEasy[bx/2/*ibrc*/])
;   SetPenForBrc(ww, brcCur = vtcc.rgbrcEasy[ibrcInside], fTrue/*fHoriz*/, fFrameLines);
;LN_ChkSetPenBrcV Does:
; if (brcCur != brcNone || brcCur != vtcc.rgbrcEasy[bx/2/*ibrc*/])
;   SetPenForBrc(ww, brcCur = vtcc.rgbrcEasy[ibrcInside], fFalse/*fHoriz*/, fFrameLines);
;LN_SetPenBrcV Does:
;	SetPenForBrc(ww, brcCur = vtcc.rgbrcEasy[ax/2], fFalse/*fHoriz*/, fFrameLines)
;LN_SetPenBrcVNone Does:
;	SetPenForBrc(ww, brcCur = brcNone, fFalse/*fHoriz*/, fFrameLines)

LN_ChkSetPenBrcH:
;PAUSE
	db	0B8h	;this combines with the next opcode
			; to become B8C031 = mov ax,c031
LN_ChkSetPenBrcV:
	xor	ax,ax	; don't alter this opcode, see note above
;PAUSE
	; si = &vtcc, bx = ibrcInside * 2
	mov	cx,[bx.vtcc.rgbrcEasyTcc]
	cmp	cx,[brcCur]
	jne	FET1200
	errnz	<brcNone>
	jcxz	FET1250	; jump to rts
	jmp	short FET1200
LN_SetPenBrcV:
	xor	ax,ax
;PAUSE
	mov	cx,[bx.vtcc.rgbrcEasyTcc]
	db	03Dh	; this combines with the next opcode
			; to become 3D33C9 = cmp ax,C933
LN_SetPenBrcVNone:
	errnz	<brcNone>
	xor	cx,cx	; don't alter this opcode, see note above
			; NOTE: fHoriz doesn't get used if brc==brcNone,
			; therefore it is OK to leave it uninitialized...
;PAUSE

FET1200:
	; cx = brc, ax = fHoriz (not restricted to 0,1 values)
	; brcCur = whatever was passed for the new brc;
	mov	[brcCur],cx

; switch (brc)
; {
	jcxz	LBrcNone
	cmp	cx,brcDotted
	je	LBrcDotted
; default:
;   Assert(fFalse);
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	cx,brcSingle
	je	FET1210
	cmp	cx,brcHairline
	je	FET1210
	cmp	cx,brcThick
	je	FET1210
	mov	ax,midDisptbn2
	mov	bx,3881	; assert line number
	cCall	AssertProcForNative,<ax,bx>
FET1210:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	;DEBUG
; case brcSingle:
; case brcHairline:
; case brcThick:
;   hbrPenBrc = vsci.hbrText;
	mov	bx,[vsci.hbrTextSci]
	jmp	short FET1220
;   break;
; case brcNone:
LBrcNone:
;   if (!fFrameLines)
;   {
;   hbrPenBrc = vsci.hbrBkgrnd;
;   break;
;   }
	mov	bx,[vsci.hbrBkgrndSci]
	cmp	[fFrameLines],0
	jz	FET1220
;   /* else fall through */
; case brcDotted:
;   hbrPenBrc = vhbrGray;
LBrcDotted:
	mov	bx,[vhbrGray]
;   break;
; }
FET1220:
	mov	[hbrPenBrc],bx

	; cx = brc, ax = fHoriz (not restricted to 0,1 values)
; dxpPenBrc = dxpBorderFti;
	mov	dx,[vsci.dxpBorderSci]
	mov	[dxpPenBrc],dx
; dypPenBrc = dypBorderFti;
	mov	dx,[vsci.dypBorderSci]
	mov	[dypPenBrc],dx

	; cx = brc, ax = fHoriz (not restricted to 0,1 values)
; if (brc == brcThick)
	cmp	cx,brcThick
	jne	FET1250
; {
; if (fHoriz)
;PAUSE
	xchg	ax,cx
	jcxz	FET1240
;   dypPenBrc = 2 * dypBorderFti;
	sal	[dypPenBrc],1
	jmp	short FET1250
; else
FET1240:
;   dxpPenBrc = 2 * dxpBorderFti;
	sal	[dxpPenBrc],1
; }
FET1250:
	ret
; }

sEnd	fetchtbn
	end
