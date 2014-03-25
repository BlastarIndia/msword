	include w2.inc
	include noxport.inc
	include consts.inc
	include structs.inc

createSeg	res_PCODE,resn,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midResn     equ 7      ; module ID, for native asserts
endif ;/* DEBUG */

; EXPORTED LABELS

; EXTERNAL FUNCTIONS

externFP	<ReloadSb>
externFP	<AdjustHplcCpsToLim>

ifdef DEBUG
externFP	<AssertProcForNative>
externFP    	<FCheckHandle>
externFP	<AssertDbgInt3>
externFP	<N_FetchCp>
externFP	<C_FetchCp>
externFP	<V_FetchCp>
externFP	<N_CachePara>
externFP	<C_CachePara>
externFP	<V_CachePara>
externFP	<N_FInTableDocCp>
externFP	<C_FInTableDocCp>
externFP	<V_FInTableDocCp>
externFP	<N_FormatLineDxa>
externFP	<C_FormatLineDxa>
externFP	<V_FormatLineDxa>
externFP	<N_LoadFont>
externFP	<C_LoadFont>
externFP	<N_CpSearchSz>
externFP	<C_CpSearchSz>
externFP	<V_CpSearchSz>
externFP	<N_CpSearchSzBackward>
externFP	<C_CpSearchSzBackward>
externFP	<V_CpSearchSzBackward>
externFP	<N_FfcFormatFieldPdcp>
externFP	<C_FfcFormatFieldPdcp>
externFP	<N_DcpSkipFieldChPflcd>
externFP	<C_DcpSkipFieldChPflcd>
externFP	<V_DcpSkipFieldChPflcd>
externFP	<N_FShowResultPflcdFvc>
externFP	<C_FShowResultPflcdFvc>
externFP	<V_FShowResultPflcdFvc>
externFP	<N_IfldFromDocCp>
externFP	<C_IfldFromDocCp>
externFP	<V_IfldFromDocCp>
externFP	<N_FillIfldFlcd>
externFP	<C_FillIfldFlcd>
externFP	<V_FillIfldFlcd>
externFP	<N_GetIfldFlcd>
externFP	<C_GetIfldFlcd>
externFP	<V_GetIfldFlcd>
externFP	<N_SetFlcdCh>
externFP	<C_SetFlcdCh>
externFP	<N_IfldInsertDocCp>
externFP	<C_IfldInsertDocCp>
externFP	<V_IfldInsertDocCp>
externFP	<N_FetchVisibleRgch>
externFP	<C_FetchVisibleRgch>
externFP	<N_FetchCpPccpVisible>
externFP	<C_FetchCpPccpVisible>
externFP	<V_FetchCpPccpVisible>
externFP	<N_CpVisibleCpField>
externFP	<C_CpVisibleCpField>
externFP	<V_CpVisibleCpField>
externFP	<N_FVisibleCp>
externFP	<C_FVisibleCp>
externFP	<N_DisplayFliCore>
externFP	<C_DisplayFliCore>
externFP	<N_FMarkLine>
externFP	<C_FMarkLine>
externFP	<N_XpFromDcp>
externFP	<C_XpFromDcp>
externFP	<V_XpFromDcp>
externFP	<N_MapStc>
externFP	<C_MapStc>
externFP	<N_AddVisiSpaces>
externFP	<C_AddVisiSpaces>
externFP	<N_ApplyPrlSgc>
externFP	<C_ApplyPrlSgc>
externFP	<V_ApplyPrlSgc>
externFP	<N_FGetParaState>
externFP	<C_FGetParaState>
externFP	<V_FGetParaState>
externFP	<N_FGraphicsFcidToPlf>
externFP	<C_FGraphicsFcidToPlf>
externFP	<V_FGraphicsFcidToPlf>
externFP	<N_PnFromPlcbteFc>
externFP	<C_PnFromPlcbteFc>
externFP	<N_BFromFc>
externFP	<C_BFromFc>
externFP	<N_HpchFromFc>
externFP	<C_HpchFromFc>
externFP	<N_HpchGetPn>
externFP	<C_HpchGetPn>
externFP	<N_IbpCacheFilePage>
externFP	<C_IbpCacheFilePage>
externFP	<N_DoPrmSgc>
externFP	<C_DoPrmSgc>
externFP	<N_LoadFcidFull>
externFP	<C_LoadFcidFull>
externFP	<N_ResetFont>
externFP	<C_ResetFont>
externFP	<N_FCpVisiInOutline>
externFP	<C_FCpVisiInOutline>
externFP	<V_FCpVisiInOutline>
externFP	<N_DisplayFli>
externFP	<C_DisplayFli>
externFP	<N_FormatLine>
externFP	<C_FormatLine>
externFP	<N_FormatDrLine>
externFP	<C_FormatDrLine>
externFP	<N_FAddRun>
externFP	<C_FAddRun>
externFP	<N_ScanFnForBytes>
externFP	<C_ScanFnForBytes>
externFP	<N_AdjustCp>
externFP	<C_AdjustCp>
externFP	<N_IbpLru>
externFP	<C_IbpLru>
externFP	<V_IbpLru>
externFP	<N_ClFormatLines>
externFP	<C_ClFormatLines>
externFP	<N_LbcFormatPara>
externFP	<C_LbcFormatPara>
externFP	<N_CacheParaL>
externFP	<C_CacheParaL>
externFP	<N_CacheSectL>
externFP	<C_CacheSectL>
externFP	<N_FAbortLayout>
externFP	<C_FAbortLayout>
externFP	<N_PdrFetch>
externFP	<C_PdrFetch>
externFP	<N_PdrFetchAndFree>
externFP	<C_PdrFetchAndFree>
externFP	<N_PdrFreeAndFetch>
externFP	<C_PdrFreeAndFetch>
externFP	<N_FreePdrf>
externFP	<C_FreePdrf>
externFP	<N_PtOrigin>
externFP	<C_PtOrigin>
externFP	<N_FUpdateDr>
externFP	<C_FUpdateDr>
externFP	<N_ScrollDrUp>
externFP	<C_ScrollDrUp>
externFP	<N_GetCpFirstCpLimDisplayPara>
externFP	<C_GetCpFirstCpLimDisplayPara>
externFP	<V_GetCpFirstCpLimDisplayPara>
externFP	<N_CpFormatFrom>
externFP	<C_CpFormatFrom>
externFP	<V_CpFormatFrom>
externFP	<N_CpVisibleBackCpField>
externFP	<C_CpVisibleBackCpField>
externFP	<V_CpVisibleBackCpField>
externFP	<N_FAssignLr>
externFP	<C_FAssignLr>
externFP	<N_FWidowControl>
externFP	<C_FWidowControl>
externFP	<N_IfrdGatherFtnRef>
externFP	<C_IfrdGatherFtnRef>
externFP	<N_FGetFtnBreak>
externFP	<C_FGetFtnBreak>
externFP	<N_CopyHdtLrs>
externFP	<C_CopyHdtLrs>
externFP	<N_CopyLrs>
externFP	<C_CopyLrs>
externFP	<N_ReplaceInPllr>
externFP	<C_ReplaceInPllr>
externFP	<N_PushLbs>
externFP	<C_PushLbs>
externFP	<N_PopLbs>
externFP	<C_PopLbs>
externFP	<N_CopyLbs>
externFP	<C_CopyLbs>
externFP	<N_MiscPlcLoops>
externFP	<C_MiscPlcLoops>
externFP	<N_FFillRgwWithSeqLevs>
externFP	<C_FFillRgwWithSeqLevs>
externFP	<V_FFillRgwWithSeqLevs>
externFP	<N_WidthHeightFromBrc>
externFP	<C_WidthHeightFromBrc>
externFP	<V_WidthHeightFromBrc>
externFP	<N_AdjustHplcedlCps>
externFP	<C_AdjustHplcedlCps>
externFP	<N_PxsInit>
externFP	<C_PxsInit>
externFP	<N_PostTn>
externFP	<C_PostTn>
externFP	<N_FDoTns>
externFP	<C_FDoTns>
externFP	<N_CloseTns>
externFP	<C_CloseTns>
externFP	<N_FReplace>
externFP	<C_FReplace>
externFP	<N_XReplace>
externFP	<C_XReplace>
externFP	<N_IpcdSplit>
externFP	<C_IpcdSplit>
externFP	<N_XRepl1>
externFP	<C_XRepl1>
externFP	<N_InvalCaFierce>
externFP	<C_InvalCaFierce>
externFP	<N_DrawInsertLine>
externFP	<C_DrawInsertLine>
externFP	<N_FInsertInPlc>
externFP	<C_FInsertInPlc>
externFP	<N_FOpenPlc>
externFP	<C_FOpenPlc>
externFP	<N_ShrinkPlc>
externFP	<C_ShrinkPlc>
externFP	<N_FStretchPlc>
externFP	<C_FStretchPlc>
externFP	<N_FNewChpIns>
externFP	<C_FNewChpIns>
externFP	<N_FInsertRgch>
externFP	<C_FInsertRgch>
externFP	<N_FcAppendRgchToFn>
externFP	<C_FcAppendRgchToFn>
externFP	<N_CbGrpprlProp>
externFP	<C_CbGrpprlProp>
externFP	<N_CbGenPrl>
externFP	<C_CbGenPrl>
externFP	<N_CbGenChpxFromChp>
externFP	<C_CbGenChpxFromChp>
externFP	<N_CbGenPapxFromPap>
externFP	<C_CbGenPapxFromPap>
externFP	<N_ItcGetTcxCache>
externFP	<C_ItcGetTcxCache>
externFP	<V_ItcGetTcxCache>
externFP	<N_ItcGetTcx>
externFP	<C_ItcGetTcx>
externFP	<V_ItcGetTcx>
externFP	<N_FUpdateTable>
externFP	<C_FUpdateTable>
externFP	<N_FUpdTableDr>
externFP	<C_FUpdTableDr>
externFP	<N_FrameEasyTable>
externFP	<C_FrameEasyTable>
externFP	<N_RtfIn>
externFP	<C_RtfIn>
externFP	<N_ChMapSpecChar>
externFP	<C_ChMapSpecChar>
externFP	<V_ChMapSpecChar>
externFP	<N_FSearchRgrsym>
externFP	<C_FSearchRgrsym>
externFP	<V_FSearchRgrsym>
externFP	<N_ValFromPropSprm>
externFP	<C_ValFromPropSprm>
externFP	<V_ValFromPropSprm>
externFP	<N_XDelFndSedPgdPad>
externFP	<C_XDelFndSedPgdPad>
externFP	<N_PchSzRtfMove>
externFP	<C_PchSzRtfMove>
externFP	<V_PchSzRtfMove>
externFP	<N_FRepl1>
externFP	<C_FRepl1>
externFP	<N_FReplaceCps>
externFP	<C_FReplaceCps>
externFP	<N_XReplaceCps>
externFP	<C_XReplaceCps>
externFP	<N_WCompSzSrt>
externFP	<C_WCompSzSrt>
externFP	<V_WCompSzSrt>
externFP	<N_WCompChCh>
externFP	<C_WCompChCh>
externFP	<V_WCompChCh>
externFP	<N_WCompRgchIndex>
externFP	<C_WCompRgchIndex>
externFP	<V_WCompRgchIndex>
externFP	<N_IbstFindSzFfn>
externFP	<C_IbstFindSzFfn>
externFP	<V_IbstFindSzFfn>
externFP        <OurExitWindows>
endif ;/* DEBUG */

; EXTERNAL DATA

sBegin  data

externW mpfnhfcb	; extern struct FCB	**mpfnhfcb[];
externW mpwwhwwd	; extern struct WWD	**mpwwhwwd[];
externW mpmwhmwd	; extern struct MWD	**mpmwhmwd[];
externW mpdochdod	; extern struct DOD	**mpdochdod[];
externW vfpc		; extern struct FPC	vfpc;
externW mpsbps		; extern SB		mpsbps[];
externW vpdrfHead	; extern struct DRF	*vpdrfHead;
externW vitr		; extern struct ITR	vitr;
externW vfti		; extern struct FTI	vfti;
ifdef DEBUG
externW vfCheckPlc	; extern BOOL		vfCheckPlc;
externW vdbs		; extern struct DBS	vdbs;
externW wFillBlock
externW vsccAbove
externW vpdrfHeadUnused
endif ;/* DEBUG */

sEnd    data


; CODE SEGMENT res_PCODE

sBegin	resn
	assumes cs,resn
	assumes ds,dgroup
	assume es:nothing
	assume ss:nothing

include asserth.asm


ifdef DEBUG
;-------------------------------------------------------------------------
;	InstallInt3Handler( )
;-------------------------------------------------------------------------
;/* I N S T A L L  I N T  3  H A N D L E R */
; %%Function:InstallInt3Handler %%Owner:BRADV
cProc	InstallInt3Handler,<FAR,PUBLIC,ATOMIC>,<>

cBegin
;	Save old int3 handler for debugging purposes
	call	Int3Swap
cEnd

; End of InstallInt3Handler


Int3ThunkCall:
	call	far ptr Int3Handler
Int3Message:
    db	0Dh,0Ah
Int3Sz:
    db	'Int 3 Encountered',0
Int3SzMac:

Int3Swap:
	push	ds
	push	ax
	push	bx
	xor	ax,ax
	mov	ds,ax
	mov	bx,3*4+2
	xchg	ax,[bx]
	xchg	ax,wptr (cs:Int3ThunkCall+3)
	xchg	ax,[bx]
	mov	bx,3*4
	xchg	ax,[bx]
	xchg	ax,wptr (cs:Int3ThunkCall+1)
	xchg	ax,[bx]
	pop	bx
	pop	ax
	pop	ds
	ret

;-------------------------------------------------------------------------
;	Int3Handler( )
;-------------------------------------------------------------------------
;/* I N T  3  H A N D L E R */
; %%Function:Int3Handler %%Owner:BRADV
cProc	Int3Handler,<FAR,PUBLIC>,<ax,bx,cx,dx,si,di,ds,es>

cBegin
;	Restore old int3 handler for debugging purposes
;	call	Int3Swap	;commented out so Assert retry will work
	push	cs
	pop	ds
	mov	si,offset Int3Message
	cld
I3H01:
	lodsb
	or	al,al
	je	I3H02
	push	ds
	push	si
	mov	dl,al
	mov	ah,004h
	int	021h
	pop	si
	pop	ds
	jmp	I3H01
I3H02:
	push	ss
	pop	ds
;	AssertProcMst(mstDbgInt3, NULL, 0, NULL);
;       **  (Replaced by AssertDbgInt3();  **
;	mov	ax,mstDbgInt3
;	errnz	<NULL>
;	push	ax
;	xor	ax,ax
;	push	ax
;	push	ax
;	push	ax
	cCall	AssertDbgInt3,<>
cEnd

; End of Int3Handler

; Quick exit routine
; %%Function:Abort %%Owner:BRADV
PUBLIC  Abort
Abort:
        cCall   OurExitWindows,<>
endif ;/* DEBUG */


ifdef DEBUG
; %%Function:InstallInt3FHandler %%Owner:BRADV
PUBLIC	InstallInt3FHandler
InstallInt3FHandler:
; %%Function:LGetTime %%Owner:BRADV
PUBLIC  LGetTime
LGetTime:
; %%Function:TimerInit %%Owner:BRADV
PUBLIC  TimerInit
TimerInit:
; %%Function:TimerStart %%Owner:BRADV
PUBLIC  TimerStart
TimerStart:
; %%Function:TimerStop %%Owner:BRADV
PUBLIC  TimerStop
TimerStop:
; %%Function:TimerReset %%Owner:BRADV
PUBLIC  TimerReset
TimerReset:
	nop	;nop so that symdeb will not display InstallInt3FHandler
		;when we have a call to S_FetchCp
endif ;/* DEBUG */

ifdef DEBUG
;Note - these routines must be in the same order as the CUseFlags in vdbs
FirstSelectRoutine:
; %%Function:S_FetchCp %%Owner:BRADV
PUBLIC	S_FetchCp
S_FetchCp:
    call    SelectDesiredVersion
    dd	    N_FetchCp
    dd	    C_FetchCp
    dd	    V_FetchCp
; %%Function:S_CachePara %%Owner:BRADV
PUBLIC	S_CachePara
S_CachePara:
    call    SelectDesiredVersion
    dd	    N_CachePara
    dd	    C_CachePara
    dd	    V_CachePara
; %%Function:S_FInTableDocCp %%Owner:BRADV
PUBLIC	S_FInTableDocCp
S_FInTableDocCp:
    call    SelectDesiredVersion
    dd	    N_FInTableDocCp
    dd	    C_FInTableDocCp
    dd	    V_FInTableDocCp
; %%Function:S_LoadFont %%Owner:BRADV
PUBLIC	S_LoadFont
S_LoadFont:
    call    SelectDesiredVersion
    dd	    N_LoadFont
    dd	    C_LoadFont
    dd	    C_LoadFont
; %%Function:S_CpSearchSz %%Owner:BRADV
PUBLIC	S_CpSearchSz
S_CpSearchSz:
    call    SelectDesiredVersion
    dd	    N_CpSearchSz
    dd	    C_CpSearchSz
    dd	    V_CpSearchSz
; %%Function:S_CpSearchSzBackward %%Owner:BRADV
PUBLIC	S_CpSearchSzBackward
S_CpSearchSzBackward:
    call    SelectDesiredVersion
    dd	    N_CpSearchSzBackward
    dd	    C_CpSearchSzBackward
    dd	    V_CpSearchSzBackward
; %%Function:S_FfcFormatFieldPdcp %%Owner:BRADV
PUBLIC	S_FfcFormatFieldPdcp
S_FfcFormatFieldPdcp:
    call    SelectDesiredVersion
    dd	    N_FfcFormatFieldPdcp
    dd	    C_FfcFormatFieldPdcp
    dd	    C_FfcFormatFieldPdcp
; %%Function:S_DcpSkipFieldChPflcd %%Owner:BRADV
PUBLIC	S_DcpSkipFieldChPflcd
S_DcpSkipFieldChPflcd:
    call    SelectDesiredVersion
    dd	    N_DcpSkipFieldChPflcd
    dd	    C_DcpSkipFieldChPflcd
    dd	    V_DcpSkipFieldChPflcd
; %%Function:S_FShowResultPflcdFvc %%Owner:BRADV
PUBLIC	S_FShowResultPflcdFvc
S_FShowResultPflcdFvc:
    call    SelectDesiredVersion
    dd	    N_FShowResultPflcdFvc
    dd	    C_FShowResultPflcdFvc
    dd	    V_FShowResultPflcdFvc
; %%Function:S_IfldFromDocCp %%Owner:BRADV
PUBLIC	S_IfldFromDocCp
S_IfldFromDocCp:
    call    SelectDesiredVersion
    dd	    N_IfldFromDocCp
    dd	    C_IfldFromDocCp
    dd	    V_IfldFromDocCp
; %%Function:S_FillIfldFlcd %%Owner:BRADV
PUBLIC	S_FillIfldFlcd
S_FillIfldFlcd:
    call    SelectDesiredVersion
    dd	    N_FillIfldFlcd
    dd	    C_FillIfldFlcd
    dd	    V_FillIfldFlcd
; %%Function:S_GetIfldFlcd %%Owner:BRADV
PUBLIC	S_GetIfldFlcd
S_GetIfldFlcd:
    call    SelectDesiredVersion
    dd	    N_GetIfldFlcd
    dd	    C_GetIfldFlcd
    dd	    V_GetIfldFlcd
; %%Function:S_SetFlcdCh %%Owner:BRADV
PUBLIC	S_SetFlcdCh
S_SetFlcdCh:
    call    SelectDesiredVersion
    dd	    N_SetFlcdCh
    dd	    C_SetFlcdCh
    dd	    C_SetFlcdCh
; %%Function:S_IfldInsertDocCp %%Owner:BRADV
PUBLIC	S_IfldInsertDocCp
S_IfldInsertDocCp:
    call    SelectDesiredVersion
    dd	    N_IfldInsertDocCp
    dd	    C_IfldInsertDocCp
    dd	    V_IfldInsertDocCp
; %%Function:S_FetchVisibleRgch %%Owner:BRADV
PUBLIC	S_FetchVisibleRgch
S_FetchVisibleRgch:
    call    SelectDesiredVersion
    dd	    N_FetchVisibleRgch
    dd	    C_FetchVisibleRgch
    dd	    C_FetchVisibleRgch
; %%Function:S_FetchCpPccpVisible %%Owner:BRADV
PUBLIC	S_FetchCpPccpVisible
S_FetchCpPccpVisible:
    call    SelectDesiredVersion
    dd	    N_FetchCpPccpVisible
    dd	    C_FetchCpPccpVisible
    dd	    V_FetchCpPccpVisible
; %%Function:S_CpVisibleCpField %%Owner:BRADV
PUBLIC	S_CpVisibleCpField
S_CpVisibleCpField:
    call    SelectDesiredVersion
    dd	    N_CpVisibleCpField
    dd	    C_CpVisibleCpField
    dd	    V_CpVisibleCpField
; %%Function:S_FVisibleCp %%Owner:BRADV
PUBLIC	S_FVisibleCp
S_FVisibleCp:
    call    SelectDesiredVersion
    dd	    N_FVisibleCp
    dd	    C_FVisibleCp
    dd	    C_FVisibleCp
; %%Function:S_DisplayFliCore %%Owner:BRADV
PUBLIC	S_DisplayFliCore
S_DisplayFliCore:
    call    SelectDesiredVersion
    dd	    N_DisplayFliCore
    dd	    C_DisplayFliCore
    dd	    C_DisplayFliCore
; %%Function:S_FMarkLine %%Owner:BRADV
PUBLIC	S_FMarkLine
S_FMarkLine:
    call    SelectDesiredVersion
    dd	    N_FMarkLine
    dd	    C_FMarkLine
    dd	    C_FMarkLine
; %%Function:S_XpFromDcp %%Owner:BRADV
PUBLIC	S_XpFromDcp
S_XpFromDcp:
    call    SelectDesiredVersion
    dd	    N_XpFromDcp
    dd	    C_XpFromDcp
    dd	    V_XpFromDcp
; %%Function:S_MapStc %%Owner:BRADV
PUBLIC	S_MapStc
S_MapStc:
    call    SelectDesiredVersion
    dd	    N_MapStc
    dd	    C_MapStc
    dd	    C_MapStc
; %%Function:S_AddVisiSpaces %%Owner:BRADV
PUBLIC	S_AddVisiSpaces
S_AddVisiSpaces:
    call    SelectDesiredVersion
    dd	    N_AddVisiSpaces
    dd	    C_AddVisiSpaces
    dd	    C_AddVisiSpaces
; %%Function:S_ApplyPrlSgc %%Owner:BRADV
PUBLIC	S_ApplyPrlSgc
S_ApplyPrlSgc:
    call    SelectDesiredVersion
    dd	    N_ApplyPrlSgc
    dd	    C_ApplyPrlSgc
    dd	    V_ApplyPrlSgc
; %%Function:S_FGetParaState %%Owner:BRADV
PUBLIC	S_FGetParaState
S_FGetParaState:
    call    SelectDesiredVersion
    dd	    N_FGetParaState
    dd	    C_FGetParaState
    dd	    V_FGetParaState
; %%Function:S_FGraphicsFcidToPlf %%Owner:BRADV
PUBLIC	S_FGraphicsFcidToPlf
S_FGraphicsFcidToPlf:
    call    SelectDesiredVersion
    dd	    N_FGraphicsFcidToPlf
    dd	    C_FGraphicsFcidToPlf
    dd	    C_FGraphicsFcidToPlf
; %%Function:S_PnFromPlcbteFc %%Owner:BRADV
PUBLIC	S_PnFromPlcbteFc
S_PnFromPlcbteFc:
    call    SelectDesiredVersion
    dd	    N_PnFromPlcbteFc
    dd	    C_PnFromPlcbteFc
    dd	    C_PnFromPlcbteFc
; %%Function:S_BFromFc %%Owner:BRADV
PUBLIC	S_BFromFc
S_BFromFc:
    call    SelectDesiredVersion
    dd	    N_BFromFc
    dd	    C_BFromFc
    dd	    C_BFromFc
; %%Function:S_HpchFromFc %%Owner:BRADV
PUBLIC	S_HpchFromFc
S_HpchFromFc:
    call    SelectDesiredVersion
    dd	    N_HpchFromFc
    dd	    C_HpchFromFc
    dd	    C_HpchFromFc
; %%Function:S_HpchGetPn %%Owner:BRADV
PUBLIC	S_HpchGetPn
S_HpchGetPn:
    call    SelectDesiredVersion
    dd	    N_HpchGetPn
    dd	    C_HpchGetPn
    dd	    C_HpchGetPn
; %%Function:S_IbpCacheFilePage %%Owner:BRADV
PUBLIC	S_IbpCacheFilePage
S_IbpCacheFilePage:
    call    SelectDesiredVersion
    dd	    N_IbpCacheFilePage
    dd	    C_IbpCacheFilePage
    dd	    C_IbpCacheFilePage
; %%Function:S_DoPrmSgc %%Owner:BRADV
PUBLIC	S_DoPrmSgc
S_DoPrmSgc:
    call    SelectDesiredVersion
    dd	    N_DoPrmSgc
    dd	    C_DoPrmSgc
    dd	    C_DoPrmSgc
; %%Function:S_LoadFcidFull %%Owner:BRADV
PUBLIC	S_LoadFcidFull
S_LoadFcidFull:
    call    SelectDesiredVersion
    dd	    N_LoadFcidFull
    dd	    C_LoadFcidFull
    dd	    C_LoadFcidFull
; %%Function:S_DisplayFli %%Owner:BRADV
PUBLIC	S_DisplayFli
S_DisplayFli:
    call    SelectDesiredVersion
    dd	    N_DisplayFli
    dd	    C_DisplayFli
    dd	    C_DisplayFli
; %%Function:S_ResetFont %%Owner:BRADV
PUBLIC	S_ResetFont
S_ResetFont:
    call    SelectDesiredVersion
    dd	    N_ResetFont
    dd	    C_ResetFont
    dd	    C_ResetFont
; %%Function:S_FCpVisiInOutline %%Owner:BRADV
PUBLIC	S_FCpVisiInOutline
S_FCpVisiInOutline:
    call    SelectDesiredVersion
    dd	    N_FCpVisiInOutline
    dd	    C_FCpVisiInOutline
    dd	    V_FCpVisiInOutline
; %%Function:S_FormatLine %%Owner:BRADV
PUBLIC	S_FormatLine
S_FormatLine:
    call    SelectDesiredVersion
    dd	    N_FormatLine
    dd	    C_FormatLine
    dd	    C_FormatLine
; %%Function:S_FormatDrLine %%Owner:BRADV
PUBLIC	S_FormatDrLine
S_FormatDrLine:
    call    SelectDesiredVersion
    dd	    N_FormatDrLine
    dd	    C_FormatDrLine
    dd	    C_FormatDrLine
; %%Function:S_FormatLineDxa %%Owner:BRADV
PUBLIC	S_FormatLineDxa
S_FormatLineDxa:
    call    SelectDesiredVersion
    dd	    N_FormatLineDxa
    dd	    C_FormatLineDxa
    dd	    V_FormatLineDxa
; %%Function:S_FAddRun %%Owner:BRADV
PUBLIC	S_FAddRun
S_FAddRun:
    call    SelectDesiredVersion
    dd	    N_FAddRun
    dd	    C_FAddRun
    dd	    C_FAddRun
; %%Function:S_ScanFnForBytes %%Owner:BRADV
PUBLIC	S_ScanFnForBytes
S_ScanFnForBytes:
    call    SelectDesiredVersion
    dd	    N_ScanFnForBytes
    dd	    C_ScanFnForBytes
    dd	    C_ScanFnForBytes
; %%Function:S_PtOrigin %%Owner:BRADV
PUBLIC	S_PtOrigin
S_PtOrigin:
    call    SelectDesiredVersion
    dd	    N_PtOrigin
    dd	    C_PtOrigin
    dd	    C_PtOrigin
; %%Function:S_FUpdateDr %%Owner:BRADV
PUBLIC	S_FUpdateDr
S_FUpdateDr:
    call    SelectDesiredVersion
    dd	    N_FUpdateDr
    dd	    C_FUpdateDr
    dd	    C_FUpdateDr
; %%Function:S_ScrollDrUp %%Owner:BRADV
PUBLIC	S_ScrollDrUp
S_ScrollDrUp:
    call    SelectDesiredVersion
    dd	    N_ScrollDrUp
    dd	    C_ScrollDrUp
    dd	    C_ScrollDrUp
; %%Function:S_AdjustCp %%Owner:BRADV
PUBLIC	S_AdjustCp
S_AdjustCp:
    call    SelectDesiredVersion
    dd	    N_AdjustCp
    dd	    C_AdjustCp
    dd	    C_AdjustCp
; %%Function:S_IbpLru %%Owner:BRADV
PUBLIC	S_IbpLru
S_IbpLru:
    call    SelectDesiredVersion
    dd	    N_IbpLru
    dd	    C_IbpLru
    dd	    V_IbpLru
; %%Function:S_ClFormatLines %%Owner:BRADV
PUBLIC	S_ClFormatLines
S_ClFormatLines:
    call    SelectDesiredVersion
    dd	    N_ClFormatLines
    dd	    C_ClFormatLines
    dd	    C_ClFormatLines
; %%Function:S_LbcFormatPara %%Owner:BRADV
PUBLIC	S_LbcFormatPara
S_LbcFormatPara:
    call    SelectDesiredVersion
    dd	    N_LbcFormatPara
    dd	    C_LbcFormatPara
    dd	    C_LbcFormatPara
; %%Function:S_CacheParaL %%Owner:BRADV
PUBLIC	S_CacheParaL
S_CacheParaL:
    call    SelectDesiredVersion
    dd	    N_CacheParaL
    dd	    C_CacheParaL
    dd	    C_CacheParaL
; %%Function:S_CacheSectL %%Owner:BRADV
PUBLIC	S_CacheSectL
S_CacheSectL:
    call    SelectDesiredVersion
    dd	    N_CacheSectL
    dd	    C_CacheSectL
    dd	    C_CacheSectL
; %%Function:S_FAbortLayout %%Owner:BRADV
PUBLIC	S_FAbortLayout
S_FAbortLayout:
    call    SelectDesiredVersion
    dd	    N_FAbortLayout
    dd	    C_FAbortLayout
    dd	    C_FAbortLayout
; %%Function:S_PdrFetch %%Owner:BRADV
PUBLIC	S_PdrFetch
S_PdrFetch:
    call    SelectDesiredVersion
    dd	    N_PdrFetch
    dd	    C_PdrFetch
    dd	    C_PdrFetch
; %%Function:S_PdrFetchAndFree %%Owner:BRADV
PUBLIC	S_PdrFetchAndFree
S_PdrFetchAndFree:
    call    SelectDesiredVersion
    dd	    N_PdrFetchAndFree
    dd	    C_PdrFetchAndFree
    dd	    C_PdrFetchAndFree
; %%Function:S_PdrFreeAndFetch %%Owner:BRADV
PUBLIC	S_PdrFreeAndFetch
S_PdrFreeAndFetch:
    call    SelectDesiredVersion
    dd	    N_PdrFreeAndFetch
    dd	    C_PdrFreeAndFetch
    dd	    C_PdrFreeAndFetch
; %%Function:S_FreePdrf %%Owner:BRADV
PUBLIC	S_FreePdrf
S_FreePdrf:
    call    SelectDesiredVersion
    dd	    N_FreePdrf
    dd	    C_FreePdrf
    dd	    C_FreePdrf
; %%Function:S_GetCpFirstCpLimDisplayPara %%Owner:BRADV
PUBLIC	S_GetCpFirstCpLimDisplayPara
S_GetCpFirstCpLimDisplayPara:
    call    SelectDesiredVersion
    dd	    N_GetCpFirstCpLimDisplayPara
    dd	    C_GetCpFirstCpLimDisplayPara
    dd	    V_GetCpFirstCpLimDisplayPara
; %%Function:S_CpFormatFrom %%Owner:BRADV
PUBLIC	S_CpFormatFrom
S_CpFormatFrom:
    call    SelectDesiredVersion
    dd	    N_CpFormatFrom
    dd	    C_CpFormatFrom
    dd	    V_CpFormatFrom
; %%Function:S_CpVisibleBackCpField %%Owner:BRADV
PUBLIC	S_CpVisibleBackCpField
S_CpVisibleBackCpField:
    call    SelectDesiredVersion
    dd	    N_CpVisibleBackCpField
    dd	    C_CpVisibleBackCpField
    dd	    V_CpVisibleBackCpField
; %%Function:S_FAssignLr %%Owner:BRADV
PUBLIC	S_FAssignLr
S_FAssignLr:
    call    SelectDesiredVersion
    dd	    N_FAssignLr
    dd	    C_FAssignLr
    dd	    C_FAssignLr
; %%Function:S_FWidowControl %%Owner:BRADV
PUBLIC	S_FWidowControl
S_FWidowControl:
    call    SelectDesiredVersion
    dd	    N_FWidowControl
    dd	    C_FWidowControl
    dd	    C_FWidowControl
; %%Function:S_IfrdGatherFtnRef %%Owner:BRADV
PUBLIC	S_IfrdGatherFtnRef
S_IfrdGatherFtnRef:
    call    SelectDesiredVersion
    dd	    N_IfrdGatherFtnRef
    dd	    C_IfrdGatherFtnRef
    dd	    C_IfrdGatherFtnRef
; %%Function:S_FGetFtnBreak %%Owner:BRADV
PUBLIC	S_FGetFtnBreak
S_FGetFtnBreak:
    call    SelectDesiredVersion
    dd	    N_FGetFtnBreak
    dd	    C_FGetFtnBreak
    dd	    C_FGetFtnBreak
; %%Function:S_CopyHdtLrs %%Owner:BRADV
PUBLIC	S_CopyHdtLrs
S_CopyHdtLrs:
    call    SelectDesiredVersion
    dd	    N_CopyHdtLrs
    dd	    C_CopyHdtLrs
    dd	    C_CopyHdtLrs
; %%Function:S_CopyLrs %%Owner:BRADV
PUBLIC	S_CopyLrs
S_CopyLrs:
    call    SelectDesiredVersion
    dd	    N_CopyLrs
    dd	    C_CopyLrs
    dd	    C_CopyLrs
; %%Function:S_ReplaceInPllr %%Owner:BRADV
PUBLIC	S_ReplaceInPllr
S_ReplaceInPllr:
    call    SelectDesiredVersion
    dd	    N_ReplaceInPllr
    dd	    C_ReplaceInPllr
    dd	    C_ReplaceInPllr
; %%Function:S_PushLbs %%Owner:BRADV
PUBLIC	S_PushLbs
S_PushLbs:
    call    SelectDesiredVersion
    dd	    N_PushLbs
    dd	    C_PushLbs
    dd	    C_PushLbs
; %%Function:S_PopLbs %%Owner:BRADV
PUBLIC	S_PopLbs
S_PopLbs:
    call    SelectDesiredVersion
    dd	    N_PopLbs
    dd	    C_PopLbs
    dd	    C_PopLbs
; %%Function:S_CopyLbs %%Owner:BRADV
PUBLIC	S_CopyLbs
S_CopyLbs:
    call    SelectDesiredVersion
    dd	    N_CopyLbs
    dd	    C_CopyLbs
    dd	    C_CopyLbs
; %%Function:S_MiscPlcLoops %%Owner:BRADV
PUBLIC	S_MiscPlcLoops
S_MiscPlcLoops:
    call    SelectDesiredVersion
    dd	    N_MiscPlcLoops
    dd	    C_MiscPlcLoops
    dd	    C_MiscPlcLoops
; %%Function:S_FFillRgwWithSeqLevs %%Owner:BRADV
PUBLIC	S_FFillRgwWithSeqLevs
S_FFillRgwWithSeqLevs:
    call    SelectDesiredVersion
    dd	    N_FFillRgwWithSeqLevs
    dd	    C_FFillRgwWithSeqLevs
    dd	    V_FFillRgwWithSeqLevs
; %%Function:S_WidthHeightFromBrc %%Owner:BRADV
PUBLIC	S_WidthHeightFromBrc
S_WidthHeightFromBrc:
    call    SelectDesiredVersion
    dd	    N_WidthHeightFromBrc
    dd	    C_WidthHeightFromBrc
    dd	    V_WidthHeightFromBrc
; %%Function:S_AdjustHplcedlCps %%Owner:BRADV
PUBLIC	S_AdjustHplcedlCps
S_AdjustHplcedlCps:
    call    SelectDesiredVersion
    dd	    N_AdjustHplcedlCps
    dd	    C_AdjustHplcedlCps
    dd	    C_AdjustHplcedlCps
; %%Function:S_PxsInit %%Owner:BRADV
PUBLIC	S_PxsInit
S_PxsInit:
    call    SelectDesiredVersion
    dd	    N_PxsInit
    dd	    C_PxsInit
    dd	    C_PxsInit
; %%Function:S_PostTn %%Owner:BRADV
PUBLIC	S_PostTn
S_PostTn:
    call    SelectDesiredVersion
    dd	    N_PostTn
    dd	    C_PostTn
    dd	    C_PostTn
; %%Function:S_FDoTns %%Owner:BRADV
PUBLIC	S_FDoTns
S_FDoTns:
    call    SelectDesiredVersion
    dd	    N_FDoTns
    dd	    C_FDoTns
    dd	    C_FDoTns
; %%Function:S_CloseTns %%Owner:BRADV
PUBLIC	S_CloseTns
S_CloseTns:
    call    SelectDesiredVersion
    dd	    N_CloseTns
    dd	    C_CloseTns
    dd	    C_CloseTns
; %%Function:S_FReplace %%Owner:BRADV
PUBLIC	S_FReplace
S_FReplace:
    call    SelectDesiredVersion
    dd	    N_FReplace
    dd	    C_FReplace
    dd	    C_FReplace
; %%Function:S_XReplace %%Owner:BRADV
PUBLIC	S_XReplace
S_XReplace:
    call    SelectDesiredVersion
    dd	    N_XReplace
    dd	    C_XReplace
    dd	    C_XReplace
; %%Function:S_IpcdSplit %%Owner:BRADV
PUBLIC	S_IpcdSplit
S_IpcdSplit:
    call    SelectDesiredVersion
    dd	    N_IpcdSplit
    dd	    C_IpcdSplit
    dd	    C_IpcdSplit
; %%Function:S_XRepl1 %%Owner:BRADV
PUBLIC	S_XRepl1
S_XRepl1:
    call    SelectDesiredVersion
    dd	    N_XRepl1
    dd	    C_XRepl1
    dd	    C_XRepl1
; %%Function:S_InvalCaFierce %%Owner:BRADV
PUBLIC	S_InvalCaFierce
S_InvalCaFierce:
    call    SelectDesiredVersion
    dd	    N_InvalCaFierce
    dd	    C_InvalCaFierce
    dd	    C_InvalCaFierce
; %%Function:S_DrawInsertLine %%Owner:BRADV
PUBLIC	S_DrawInsertLine
S_DrawInsertLine:
    call    SelectDesiredVersion
    dd	    N_DrawInsertLine
    dd	    C_DrawInsertLine
    dd	    C_DrawInsertLine
; %%Function:S_FInsertInPlc %%Owner:BRADV
PUBLIC	S_FInsertInPlc
S_FInsertInPlc:
    call    SelectDesiredVersion
    dd	    N_FInsertInPlc
    dd	    C_FInsertInPlc
    dd	    C_FInsertInPlc
; %%Function:S_FOpenPlc %%Owner:BRADV
PUBLIC	S_FOpenPlc
S_FOpenPlc:
    call    SelectDesiredVersion
    dd	    N_FOpenPlc
    dd	    C_FOpenPlc
    dd	    C_FOpenPlc
; %%Function:S_ShrinkPlc %%Owner:BRADV
PUBLIC	S_ShrinkPlc
S_ShrinkPlc:
    call    SelectDesiredVersion
    dd	    N_ShrinkPlc
    dd	    C_ShrinkPlc
    dd	    C_ShrinkPlc
; %%Function:S_FStretchPlc %%Owner:BRADV
PUBLIC	S_FStretchPlc
S_FStretchPlc:
    call    SelectDesiredVersion
    dd	    N_FStretchPlc
    dd	    C_FStretchPlc
    dd	    C_FStretchPlc
; %%Function:S_FNewChpIns %%Owner:BRADV
PUBLIC	S_FNewChpIns
S_FNewChpIns:
    call    SelectDesiredVersion
    dd	    N_FNewChpIns
    dd	    C_FNewChpIns
    dd	    C_FNewChpIns
; %%Function:S_FInsertRgch %%Owner:BRADV
PUBLIC	S_FInsertRgch
S_FInsertRgch:
    call    SelectDesiredVersion
    dd	    N_FInsertRgch
    dd	    C_FInsertRgch
    dd	    C_FInsertRgch
; %%Function:S_FcAppendRgchToFn %%Owner:BRADV
PUBLIC	S_FcAppendRgchToFn
S_FcAppendRgchToFn:
    call    SelectDesiredVersion
    dd	    N_FcAppendRgchToFn
    dd	    C_FcAppendRgchToFn
    dd	    C_FcAppendRgchToFn
; %%Function:S_CbGrpprlProp %%Owner:BRADV
PUBLIC	S_CbGrpprlProp
S_CbGrpprlProp:
    call    SelectDesiredVersion
    dd	    N_CbGrpprlProp
    dd	    C_CbGrpprlProp
    dd	    C_CbGrpprlProp
; %%Function:S_CbGenPrl %%Owner:BRADV
PUBLIC	S_CbGenPrl
S_CbGenPrl:
    call    SelectDesiredVersion
    dd	    N_CbGenPrl
    dd	    C_CbGenPrl
    dd	    C_CbGenPrl
; %%Function:S_CbGenChpxFromChp %%Owner:BRADV
PUBLIC	S_CbGenChpxFromChp
S_CbGenChpxFromChp:
    call    SelectDesiredVersion
    dd	    N_CbGenChpxFromChp
    dd	    C_CbGenChpxFromChp
    dd	    C_CbGenChpxFromChp
; %%Function:S_CbGenPapxFromPap %%Owner:BRADV
PUBLIC	S_CbGenPapxFromPap
S_CbGenPapxFromPap:
    call    SelectDesiredVersion
    dd	    N_CbGenPapxFromPap
    dd	    C_CbGenPapxFromPap
    dd	    C_CbGenPapxFromPap
; %%Function:S_ItcGetTcxCache %%Owner:BRADV
PUBLIC	S_ItcGetTcxCache
S_ItcGetTcxCache:
    call    SelectDesiredVersion
    dd	    N_ItcGetTcxCache
    dd	    C_ItcGetTcxCache
    dd	    V_ItcGetTcxCache
; %%Function:S_ItcGetTcx %%Owner:BRADV
PUBLIC	S_ItcGetTcx
S_ItcGetTcx:
    call    SelectDesiredVersion
    dd	    N_ItcGetTcx
    dd	    C_ItcGetTcx
    dd	    V_ItcGetTcx
; %%Function:S_FUpdateTable %%Owner:BRADV
PUBLIC	S_FUpdateTable
S_FUpdateTable:
    call    SelectDesiredVersion
    dd	    N_FUpdateTable
    dd	    C_FUpdateTable
    dd	    C_FUpdateTable
; %%Function:S_FUpdTableDr %%Owner:BRADV
PUBLIC	S_FUpdTableDr
S_FUpdTableDr:
    call    SelectDesiredVersion
    dd	    N_FUpdTableDr
    dd	    C_FUpdTableDr
    dd	    C_FUpdTableDr
; %%Function:S_FrameEasyTable %%Owner:BRADV
PUBLIC	S_FrameEasyTable
S_FrameEasyTable:
    call    SelectDesiredVersion
    dd	    N_FrameEasyTable
    dd	    C_FrameEasyTable
    dd	    C_FrameEasyTable
; %%Function:S_RtfIn %%Owner:BRADV
PUBLIC	S_RtfIn
S_RtfIn:
    call    SelectDesiredVersion
    dd	    N_RtfIn
    dd	    C_RtfIn
    dd	    C_RtfIn
; %%Function:S_ChMapSpecChar %%Owner:BRADV
PUBLIC	S_ChMapSpecChar
S_ChMapSpecChar:
    call    SelectDesiredVersion
    dd	    N_ChMapSpecChar
    dd	    C_ChMapSpecChar
    dd	    V_ChMapSpecChar
; %%Function:S_FSearchRgrsym %%Owner:BRADV
PUBLIC	S_FSearchRgrsym
S_FSearchRgrsym:
    call    SelectDesiredVersion
    dd	    N_FSearchRgrsym
    dd	    C_FSearchRgrsym
    dd	    V_FSearchRgrsym
; %%Function:S_ValFromPropSprm %%Owner:BRADV
PUBLIC	S_ValFromPropSprm
S_ValFromPropSprm:
    call    SelectDesiredVersion
    dd	    N_ValFromPropSprm
    dd	    C_ValFromPropSprm
    dd	    V_ValFromPropSprm
; %%Function:S_XDelFndSedPgdPad %%Owner:BRADV
PUBLIC	S_XDelFndSedPgdPad
S_XDelFndSedPgdPad:
    call    SelectDesiredVersion
    dd	    N_XDelFndSedPgdPad
    dd	    C_XDelFndSedPgdPad
    dd	    C_XDelFndSedPgdPad
; %%Function:S_PchSzRtfMove %%Owner:BRADV
PUBLIC	S_PchSzRtfMove
S_PchSzRtfMove:
    call    SelectDesiredVersion
    dd	    N_PchSzRtfMove
    dd	    C_PchSzRtfMove
    dd	    V_PchSzRtfMove
; %%Function:S_FRepl1 %%Owner:BRADV
PUBLIC	S_FRepl1
S_FRepl1:
    call    SelectDesiredVersion
    dd	    N_FRepl1
    dd	    C_FRepl1
    dd	    C_FRepl1
; %%Function:S_FReplaceCps %%Owner:BRADV
PUBLIC	S_FReplaceCps
S_FReplaceCps:
    call    SelectDesiredVersion
    dd	    N_FReplaceCps
    dd	    C_FReplaceCps
    dd	    C_FReplaceCps
; %%Function:S_XReplaceCps %%Owner:BRADV
PUBLIC	S_XReplaceCps
S_XReplaceCps:
    call    SelectDesiredVersion
    dd	    N_XReplaceCps
    dd	    C_XReplaceCps
    dd	    C_XReplaceCps
S_Unused107:
    call    SelectDesiredVersion
    dd	    0
    dd	    0
    dd	    0
; %%Function:S_WCompSzSrt %%Owner:BRADV
PUBLIC	S_WCompSzSrt
S_WCompSzSrt:
    call    SelectDesiredVersion
    dd	    N_WCompSzSrt
    dd	    C_WCompSzSrt
    dd	    V_WCompSzSrt
; %%Function:S_WCompChCh %%Owner:BRADV
PUBLIC	S_WCompChCh
S_WCompChCh:
    call    SelectDesiredVersion
    dd	    N_WCompChCh
    dd	    C_WCompChCh
    dd	    V_WCompChCh
; %%Function:S_WCompChCh %%Owner:BRADV
PUBLIC	S_WCompRgchIndex
S_WCompRgchIndex:
    call    SelectDesiredVersion
    dd	    N_WCompRgchIndex
    dd	    C_WCompRgchIndex
    dd	    V_WCompRgchIndex
PUBLIC	S_IbstFindSzFfn
S_IbstFindSzFfn:
    call    SelectDesiredVersion
    dd	    N_IbstFindSzFfn
    dd	    C_IbstFindSzFfn
    dd	    V_IbstFindSzFfn
; %%Function:SelectDesiredVersion %%Owner:BRADV
PUBLIC SelectDesiredVersion
SelectDesiredVersion:
    pop     dx
    mov     bx,8
    test    [vdbs.fDisableAsmDbs],080h
    jne     SDV02
    mov     bx,4
    test    [vdbs.fDisableAsmDbs],040h
    jne     SDV02
    mov     ax,dx
    sub     ax,offset FirstSelectRoutine+3
    mov     bl,15
    div     bl
    or	    ah,ah
    je	    SDV01
    int     3
SDV01:
    mov     bl,(fDisableAsmDbs) - (fUseC01Dbs) + 1
    div     bl
    mov     bl,ah   ;remainder is used as offset
    mov     cl,al
    shl     cl,1    ;quotient * 2 is shift amount
    mov     al,[bx+vdbs.fUseC01Dbs]
    shr     al,cl
    and     al,3
    shl     al,1
    shl     al,1
    mov     bl,al
SDV02:
    add     bx,dx
    push    wptr cs:[bx+2]
    push    wptr cs:[bx]
    db	    0CBh	;far ret
endif ;/* DEBUG */


;-------------------------------------------------------------------------
;	LN_StackAdjust()
;-------------------------------------------------------------------------
;LN_StackAdjust is a helper routine to reduce the size of routines
;in resn.asm.  It takes the number of words of arguments to
;the routine minus one in the byte following the call to LN_StackAdjust.
;It then moves the far return address and si and di to above the
;arguments, moves the top argument into bx, and puts the other arguments
;below the saved si and di.  This allows the use of short pop register
;instructions to access arguments passed to the caller of LN_StackAdjust.
;ax, bx, cx, dx are altered.
; %%Function:LN_StackAdjust %%Owner:BRADV
PUBLIC LN_StackAdjust
LN_StackAdjust:
	pop bx		    ;get LN_StackAdjust return address
	pop ax
	pop dx		    ;far return address in dx:ax
	sub sp,6	    ;make room for argument to move into
	push dx
	push ax 	    ;save far return address
	push si
	push di 	    ;save si and di
	mov di,sp
	add di,8
	lea si,[di+6]
	mov cl,cs:[bx]
	xor ch,ch
	inc bx
	push ds
	pop es
	rep movsw
	lodsw
	xchg ax,bx	    ;top argument to caller in bx
	mov si,sp	    ;point to saved far return address, si, di
	movsw
	movsw
	movsw
	movsw		    ;move them to above the arguments
	mov sp,si	    ;point sp at arguments
	jmp ax


;-------------------------------------------------------------------------
;	QcpQfldPlcfld( hplcfld, ifld )
;-------------------------------------------------------------------------
;/* Q C P  Q F L D  P L C F L D */
;native QcpQfldPlcfld(hplcfld, ifld)
;struct PLC *hplcfld;
;int	ifld;
;{

;cProc	 N_QcpQfooPlcfoo,<PUBLIC,FAR>,<>
; %%Function:N_QcpQfooPlcfoo %%Owner:BRADV
PUBLIC N_QcpQfooPlcfoo
N_QcpQfooPlcfoo:
;	QcpQfooPlcfoo takes hplcfoo in bx, ifoo in si, it returns pplcfoo
;	in bx, cbfoo in cx, qcp in es:di, qfoo in es:si.
;	if DEBUG it returns hpcp in dx:di, hpfoo in dx:si.
;	Changes ax, bx, cx, dx, si, di.
;	Note:  This routine does not follow C calling conventions,
;	it should only be called from assembler routines and should
;	not be placed in the toolbox.

;cBegin
;	AssertH( hplcfld );
;	Assert(!vfCheckPlc || (ifld >= 0 && ifld < *hplcfld->iMac));
ifdef DEBUG
	inc	bp
	push	bp
	mov	bp,sp	    ;set up bp chain for far call
	push	ds
	push	ax
	push	bx
	push	cx
	push	dx
	push	bx	;save hplcfld
	mov	ax,midResn
	mov	cx,1021 		  ; label # for native assert
	cCall	AssertHForNative,<bx, ax, cx>
	pop	bx	;restore hplcfld
	cmp	vfCheckPlc,fFalse
	je	QQP02
	cmp	si,0
	jl	QQP01
	mov	bx,[bx]
	mov	bx,[bx.iMacPlcSTR]
	;di is a value passed in by the caller's to avoid triggering
	;this assert in the one case when it is O.K to pass back a pointer
	;beyond the end of the rgfld.
	cmp	di,2
	jb	QQP005
	mov	di,1
	cmp	si,07FFFh
	je	QQP02
QQP005:
	add	bx,di
	cmp	si,bx
	jl	QQP02
QQP01:
	mov	ax,midResn
	mov	bx,1022 		  ; label # for native assert
	cCall	AssertProcForNative, <ax, bx>
QQP02:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	ds
	pop	bp
	dec	bp  ;restore bp
	push	ds  ;ds may now be a handle
	push	ss
	pop	ds  ;restore valid ds
endif ;/* DEBUG */
	mov	bx,[bx]
ifdef DEBUG
	push	ax	;save old ax
	push	bx	;save pplcfld
	push	si	;save ifld
endif ;/* DEBUG */
;	LN_LprgcpForPlc takes pplc in register bx, and returns an
;	lp in es:di.  Only dx, di, es are altered.
	call	LN_LprgcpForPlc
ifdef DEBUG
	pop	ax
	cmp	ax,si
	pop	ax
	jne	QQP03	;Assert (ifld was not trashed in qplc.asm)
	cmp	ax,bx
QQP03:
	pop	ax	;restore old ax
	je	QQP04	;Assert (pplcfld was not trashed in qplc.asm)
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,midResn
	mov	bx,1023 		  ; label # for native assert
	cCall	AssertProcForNative, <ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
QQP04:
endif ;/* DEBUG */
	mov	ax,si
	mov	cx,[bx.cbPlc]
ifdef DEBUG
	push	dx	;save sb
endif ;/* DEBUG */
	mul	cx
ifdef DEBUG
	pop	dx	;restore sb
endif ;/* DEBUG */
	xchg	ax,si
	add	si,di
	shl	ax,1
	shl	ax,1
	add	di,ax
	mov	ax,[bx.iMaxPlc]
	shl	ax,1
	shl	ax,1
	add	si,ax
ifdef DEBUG
	pop	ds	;restore segment handle
endif ;DEBUG
;}
;cEnd
	db	0CBh	    ;far ret

; End of N_QcpQfooPlcfoo


;-------------------------------------------------------------------------
;	LN_LrgbstFromSttb
;-------------------------------------------------------------------------
	;LN_LrgbstFromSttb takes psttb in bx and returns hrgbst in dx:di
	;and lrgbst in es:di.  Only dx, di, and es are altered.
; %%Function:LN_LrgbstFromSttb %%Owner:BRADV
PUBLIC LN_LrgbstFromSttb
LN_LrgbstFromSttb:
	lea	di,[bx.rgbstSttb]
	test	[bx.fExternalSttb],maskFExternalSttb
	;LN_LpFromPossibleHq takes pFoo in di, and if the zero flag is
	;set then converts pFoo into lpFoo in es:di.  If the zero flag
	;is reset then it assumes pFoo points to an hqFoo, and converts
	;the hqFoo to an lpFoo in es:di.  In both cases hpFoo is left
	;in dx:di.  Only dx, di and es are altered.
	jmp	short LN_LpFromPossibleHq


;-------------------------------------------------------------------------
;	LprgcpForPlc
;-------------------------------------------------------------------------
;/* L P R G C P  F O R	P L C */
;native CP far *LprgcpForPlc(pplc)
;struct PLC *pplc;
;{
;	if (pplc->fExternal)
;		return ((CP far *)LpFromHp(HpOfHq(pplc->hqplce)));
;	else
;		return ((CP far *)pplc->rgcp);

;	LN_LprgcpForPlc takes pplc in register bx, and returns an
;	lp in es:di.  Only dx, di, es are altered.
LN_LprgcpForPlc:
	lea	di,[bx.rgcpPlc]
	test	[bx.fExternalPlc],maskFExternalPlc
	;LN_LpFromPossibleHq takes pFoo in di, and if the zero flag is
	;set then converts pFoo into lpFoo in es:di.  If the zero flag
	;is reset then it assumes pFoo points to an hqFoo, and converts
	;the hqFoo to an lpFoo in es:di.  In both cases hpFoo is left
	;in dx:di.  Only dx, di and es are altered.
;fall through

	;LN_LpFromPossibleHq takes pFoo in di, and if the zero flag is
	;set then converts pFoo into lpFoo in es:di.  If the zero flag
	;is reset then it assumes pFoo points to an hqFoo, and converts
	;the hqFoo to an lpFoo in es:di.  In both cases hpFoo is left
	;in dx:di.  Only dx, di and es are altered.
; %%Function:LN_LpFromPossibleHq %%Owner:BRADV
PUBLIC LN_LpFromPossibleHq
LN_LpFromPossibleHq:
	mov	dx,sbDds
	push	ds
	pop	es
	je	LFPH2
	push	bx	;save caller's bx
	mov	bx,whi ([di])
	push	bx	;save sb
	mov	di,wlo ([di])
	shl	bx,1
	push	ax	;save caller's ax
	mov	ax,mpsbps[bx]
	mov	es,ax
	shr	ax,1
	jc	LFPH1
	push	cx
;	reload sb trashes ax, cx, and dx
	cCall	ReloadSb,<>
	pop	cx
LFPH1:
	pop	ax	;restore caller's ax
	mov	di,es:[di]
	pop	dx	;restore sb
	pop	bx	;restore caller's bx
LFPH2:
	ret


;-------------------------------------------------------------------------
;       IMacPlc( hplc )
;-------------------------------------------------------------------------
;/* I  M A C  P L C */
;native int IMacPlc(hplc)
;struct PLC **hplc;
;{
; %%Function:IMacPlc %%Owner:BRADV
PUBLIC	IMacPlc
IMacPlc:
	pop	cx
	pop	dx	;far return address in dx:cx
	pop	bx
	push	dx
	push	cx

;	if (hplc == hNil)
;		return 0;
	xor	ax,ax
	errnz	<hNil>
	or	bx,bx
	je	IMP01

;	else
;		{
;		AssertH( hplc );
ifdef DEBUG
	inc	bp
	push	bp
	mov	bp,sp	    ;set up bp chain for far call
	push	ds
	push	ax
	push	bx
	push	cx
	push	dx
	mov ax, midResn
	mov cx, 3		       ; label # for native assert
	cCall	AssertHForNative,<bx, ax, cx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	ds
	pop	bp
	dec	bp  ;restore bp
	push	ds  ;ds may now be a handle
	push	ss
	pop	ds  ;restore valid ds
endif ;/* DEBUG */

;		return (*hplc)->iMac;
	mov	bx,[bx]
	mov	ax,[bx.iMacPlcSTR]
ifdef DEBUG
	pop	ds	;restore segment handle
endif ;DEBUG

;		}
IMP01:
;}
	db	0CBh	    ;far ret

; End of IMacPlc


;-------------------------------------------------------------------------
;       GetPlc( hplc, i, pchFoo )
;-------------------------------------------------------------------------
;/* G E T  P L C */
;native GetPlc(hplc, i, pchFoo)
;struct PLC **hplc;
;int i;
;char *pchFoo;
;{
;/* also update vfpc with computed values for subsequent PutPlcLast call */
; %%Function:GetPlc %%Owner:BRADV
PUBLIC	GetPlc
GetPlc:
;LN_StackAdjust is a helper routine to reduce the size of routines
;in resn.asm.  It takes the number of words of arguments to
;the routine minus one in the byte following the call to LN_StackAdjust.
;It then moves the far return address and si and di to above the
;arguments, moves the top argument into bx, and puts the other arguments
;below the saved si and di.  This allows the use of short pop register
;instructions to access arguments passed to the caller of LN_StackAdjust.
;ax, bx, cx, dx are altered.
	call	LN_StackAdjust
	db	2

;	struct PLC *pplc = *(vfpc.hplc = hplc);
;	CP far *lprgcp = LprgcpForPlc(pplc);
;	char far *lpfoo;
	mov	vfpc.hplcFpc,bx

;	AssertH( hplc );
;	Assert(!vfCheckPlc || (i >= 0 && i < pplc->iMac));
ifdef DEBUG
	pop	cx
	pop	ax  ;remove arguments from stack
	pop	di
	pop	si
	inc	bp
	push	bp
	mov	bp,sp	    ;set up bp chain for far call
	push	ds
	push	si
	push	di
	push	ax
	push	cx  ;put arguments back on stack
	push	ax
	push	bx
	push	cx
	push	dx
	push	ax  ;save i
	push	bx  ;save hplc
	mov ax, midResn
	mov cx, 5		       ; label # for native assert
	cCall	AssertHForNative,<bx, ax, cx>
	pop	bx  ;restore hplc
	pop	ax  ;restore i
	cmp	vfCheckPlc,0
	je	GP02
	cmp	ax,0
	jl	GP01
	mov	bx,[bx]
	mov	dx,[bx.iMacPlcSTR]
	cmp	ax,dx
	jl	GP02
GP01:
	mov ax, midResn
	mov bx, 6		       ; label # for native assert
	cCall AssertProcForNative, <ax, bx>
GP02:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	cx
	pop	ax  ;remove arguments from stack
	pop	di
	pop	si
	pop	ds
	pop	bp
	dec	bp  ;restore bp
	push	ds  ;ds may now be a handle
	push	ss
	pop	ds  ;restore valid ds
	push	si
	push	di
	push	ax
	push	cx  ;put arguments back on stack
endif ;/* DEBUG */
	mov	bx,[bx]

;	LN_LprgcpForPlc takes pplc in register bx, and returns an
;	lp in es:di.  Only dx, di, es are altered.
	cCall	LN_LprgcpForPlc,<>

;	vfpc.cbFoo = pplc->cb;
	mov	cx,[bx.cbPlc]
	mov	vfpc.cbFooFpc,cx
;	bltbx(lpfoo = ((char far *)&(lprgcp[pplc->iMax])) + (vfpc.cbFoo * i),
;		(char far *)(vfpc.pchFoo = pchFoo), vfpc.cbFoo);
	mov	si,di
	pop	di	;get pchFoo off of stack
	mov	vfpc.pchFooFpc,di
	pop	ax	;get i off of stack

;	The following statement is done below in the C source.
;	Debug(vfpc.ifoo = i);
ifdef DEBUG
	mov	vfpc.ifooFpc,ax
endif ;/* DEBUG */

	push	si	;save offset lprgcp;
	mul	cx
	add	si,ax
	mov	ax,[bx.iMaxPlc]
	shl	ax,1
	shl	ax,1
	add	si,ax
	pop	ax	;restore offset lprgcp;
	push	si	;save offset of lpfoo;
	push	es
	pop	ds
	push	ss
	pop	es
;	Don't try to do movsw's here because the blt's are short
	rep	movsb
	pop	si	;restore offset of lpfoo;
	push	ss
	pop	ds  	; DS restored
	
;	vfpc.bfoo = (char far *)lpfoo - (char far *)lprgcp;
	sub	si,ax
	mov	vfpc.bfooFpc,si
;}
	pop	di
	pop	si
ifdef DEBUG
	pop	ds	;restore segment handle
endif ;DEBUG
	db	0CBh	    ;far ret

; End of GetPlc

;-------------------------------------------------------------------------
;       PutPlc( hplc, i, pchFoo )
;-------------------------------------------------------------------------
;/* P U T  P L C */
;native PutPlc(hplc, i, pchFoo)
;struct PLC **hplc;
;int i;
;char *pchFoo;
;{
; %%Function:PutPlc %%Owner:BRADV
PUBLIC	PutPlc
PutPlc:
;LN_StackAdjust is a helper routine to reduce the size of routines
;in resn.asm.  It takes the number of words of arguments to
;the routine minus one in the byte following the call to LN_StackAdjust.
;It then moves the far return address and si and di to above the
;arguments, moves the top argument into bx, and puts the other arguments
;below the saved si and di.  This allows the use of short pop register
;instructions to access arguments passed to the caller of LN_StackAdjust.
;ax, bx, cx, dx are altered.
	call	LN_StackAdjust
	db	2

;	struct PLC *pplc = hplc;
;	CP far *lprgcp = LprgcpForPlc(pplc);
;	char far *lpfoo;

;	AssertH( hplc );
;	Assert(!vfCheckPlc || (i >= 0 && i < pplc->iMac));
ifdef DEBUG
	pop	cx
	pop	ax  ;remove arguments from stack
	pop	di
	pop	si
	inc	bp
	push	bp
	mov	bp,sp	    ;set up bp chain for far call
	push	ds
	push	si
	push	di
	push	ax
	push	cx  ;put arguments back on stack
	push	ax
	push	bx
	push	cx
	push	dx
	push	ax  ;save i
	push	bx  ;save hplc
	mov ax, midResn
	mov cx, 7		       ; label # for native assert
	cCall	AssertHForNative,<bx, ax, cx>
	pop	bx  ;restore hplc
	pop	ax  ;restore i
	cmp	vfCheckPlc,0
	je	PP02
	cmp	ax,0
	jl	PP01
	mov	bx,[bx]
	mov	dx,[bx.iMacPlcSTR]
	cmp	ax,dx
	jl	PP02
PP01:
	mov ax, midResn
	mov bx, 8		       ; label # for native assert
	cCall AssertProcForNative, <ax, bx>
PP02:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	cx
	pop	ax  ;remove arguments from stack
	pop	di
	pop	si
	pop	ds
	pop	bp
	dec	bp  ;restore bp
	push	ds  ;ds may now be a handle
	push	ss
	pop	ds  ;restore valid ds
	push	si
	push	di
	push	ax
	push	cx  ;put arguments back on stack
endif ;/* DEBUG */
	mov	bx,[bx]

;	LN_LprgcpForPlc takes pplc in register bx, and returns an
;	lp in es:di.  Only dx, di, es are altered.
	cCall	LN_LprgcpForPlc,<>

;	bltbx((char far *)pchFoo, ((char far *)&(lprgcp[pplc->iMax])) + (cbPlc * i), cbPlc);
	mov	cx,[bx.cbPlc]
	pop	si	;get pchFoo off of stack
	pop	ax	;get i off of stack
	mul	cx
	add	di,ax
	mov	ax,[bx.iMaxPlc]
	shl	ax,1
	shl	ax,1
	add	di,ax
;	Don't try to do movsw's here because the blt's are short
	rep	movsb
;}
	pop	di
	pop	si
ifdef DEBUG
	pop	ds	;restore segment handle
endif ;DEBUG
	db	0CBh	    ;far ret

; End of PutPlc

;-------------------------------------------------------------------------
;       PutPlcLastProc( )
;-------------------------------------------------------------------------
;/* P U T  P L C  L A S T  P R O C */
;native PutPlcLastProc()
;{
; %%Function:PutPlcLastProc %%Owner:BRADV
PUBLIC	PutPlcLastProc
PutPlcLastProc:
	push	si
	push	di

;	CP far *lprgcp = LprgcpForPlc(*vfpc.hplc);
	mov	bx,vfpc.hplcFpc
	mov	bx,[bx]
;	LN_LprgcpForPlc takes pplc in register bx, and returns an
;	lp in es:di.  Only dx, di, es are altered.
	cCall	LN_LprgcpForPlc,<>
;	/* use cached info in vfpc for hplc, pchFoo, bfoo, and cbFoo */
;	bltbx((char far *)vfpc.pchFoo, (char far *)lprgcp + vfpc.bfoo, vfpc.cbFoo);
	mov	cx,vfpc.cbFooFpc
	mov	si,vfpc.pchFooFpc
	add	di,vfpc.bfooFpc
;	Don't try to do movsw's here because the blt's are short
	rep	movsb
;}
	pop	di
	pop	si
	db	0CBh	    ;far ret

; End of PutPlcLastProc

;-------------------------------------------------------------------------
;       CpPlc( hplc, i )
;-------------------------------------------------------------------------
;/* C P  P L C */
;native CP CpPlc(hplc, i)
;int	i;
;struct PLC **hplc;
;{
; %%Function:CpPlc %%Owner:BRADV
PUBLIC	CpPlc
CpPlc:
	pop	ax
	pop	dx	;return address in dx:ax
	pop	cx
	pop	bx
	push	dx
	push	ax

;	CP far *lprgcp = LprgcpForPlc(*hplc);

;	AssertH( hplc );
;	Assert(!vfCheckPlc || (i >= 0 && i <= pplc->iMac));
ifdef DEBUG
	inc	bp
	push	bp
	mov	bp,sp	    ;set up bp chain for far call
	push	ds
	push	ax
	push	bx
	push	cx
	push	dx
	push	cx  ;save i
	push	bx  ;save hplc
	mov ax, midResn
	mov cx, 10		       ; label # for native assert
	cCall	AssertHForNative,<bx, ax, cx>
	pop	bx  ;restore hplc
	pop	ax  ;restore i
	cmp	vfCheckPlc,0
	je	CP02
	cmp	ax,0
	jl	CP01
	mov	bx,[bx]
	mov	dx,[bx.iMacPlcSTR]
	cmp	ax,dx
	jle	CP02
CP01:
	mov ax, midResn
	mov bx, 11		       ; label # for native assert
	cCall AssertProcForNative, <ax, bx>
CP02:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	ds
	pop	bp
	dec	bp  ;restore bp
	push	ds  ;ds may now be a handle
	push	ss
	pop	ds  ;restore valid ds
endif ;/* DEBUG */
	push	di	;save caller's di
	mov	bx,[bx]

;	LN_LprgcpForPlc takes pplc in register bx, and returns an
;	lp in es:di.  Only dx, di, es are altered.
	cCall	LN_LprgcpForPlc,<>

;	return lprgcp[i] + DcpAdjust(*hplc, i);
	mov	ax,cx
	shl	ax,1
	shl	ax,1
	add	di,ax
	mov	ax,es:[di]
	mov	dx,es:[di+2]
	cmp	cx,[bx.icpAdjustPlc]
	jl	CP03
	add	ax,[bx.LO_dcpAdjustPlc]
	adc	dx,[bx.HI_dcpAdjustPlc]
CP03:
	pop	di	;restore caller's di
	
;}
ifdef DEBUG
	pop	ds	;restore segment handle
endif ;DEBUG
	db	0CBh	    ;far ret

; End of CpPlc

;-------------------------------------------------------------------------
;       PutCpPlc( hplc, i, cp )
;-------------------------------------------------------------------------
;/* P U T  C P  P L C */
;native int PutCpPlc(hplc, i, cp)
;struct PLC **hplc;
;int	i;
;CP cp;
;{
; %%Function:PutCpPlc %%Owner:BRADV
PUBLIC	PutCpPlc
PutCpPlc:
;LN_StackAdjust is a helper routine to reduce the size of routines
;in resn.asm.  It takes the number of words of arguments to
;the routine minus one in the byte following the call to LN_StackAdjust.
;It then moves the far return address and si and di to above the
;arguments, moves the top argument into bx, and puts the other arguments
;below the saved si and di.  This allows the use of short pop register
;instructions to access arguments passed to the caller of LN_StackAdjust.
;ax, bx, cx, dx are altered.
	call	LN_StackAdjust
	db	3

;	CP far *lprgcp = LprgcpForPlc(*hplc);

;	AssertH( hplc );
;	Assert(!vfCheckPlc || (i >= 0 && i <= pplc->iMac));
ifdef DEBUG
	pop	dx
	pop	cx
	pop	ax  ;remove arguments from stack
	pop	di
	pop	si
	inc	bp
	push	bp
	mov	bp,sp	    ;set up bp chain for far call
	push	ds
	push	si
	push	di
	push	ax
	push	cx
	push	dx  ;put arguments back on stack
	push	ax
	push	bx
	push	cx
	push	dx
	push	ax  ;save i
	push	bx  ;save hplc
	mov ax, midResn
	mov cx, 13		       ; label # for native assert
	cCall	AssertHForNative,<bx, ax, cx>
	pop	bx  ;restore hplc
	pop	ax  ;restore i
	cmp	vfCheckPlc,0
	je	PCP02
	cmp	ax,0
	jl	PCP01
	mov	bx,[bx]
	mov	dx,[bx.iMacPlcSTR]
	cmp	ax,dx
	jle	PCP02
PCP01:
	mov ax, midResn
	mov bx, 14		       ; label # for native assert
	cCall AssertProcForNative, <ax, bx>
PCP02:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	dx
	pop	cx
	pop	ax  ;remove arguments from stack
	pop	di
	pop	si
	pop	ds
	pop	bp
	dec	bp  ;restore bp
	push	ds  ;ds may now be a handle
	push	ss
	pop	ds  ;restore valid ds
	push	si
	push	di
	push	ax
	push	cx
	push	dx  ;put arguments back on stack
endif ;/* DEBUG */
	mov	bx,[bx]

;	LN_LprgcpForPlc takes pplc in register bx, and returns an
;	lp in es:di.  Only dx, di, es are altered.
	cCall	LN_LprgcpForPlc,<>

;	lprgcp[i] = cp - DcpAdjust(*hplc, i);
	pop	ax  ;get OFF_cp off of stack
	pop	dx  ;get SEG_cp off of stack
	pop	cx  ;get i off of stack
	cmp	cx,[bx.icpAdjustPlc]
	jl	PCP03
	sub	ax,[bx.LO_dcpAdjustPlc]
	sbb	dx,[bx.HI_dcpAdjustPlc]
PCP03:
	shl	cx,1
	shl	cx,1
	add	di,cx
	mov	es:[di],ax
	mov	es:[di+2],dx
	
;}
	pop	di
	pop	si
ifdef DEBUG
	pop	ds	;restore segment handle
endif ;DEBUG
	db	0CBh	    ;far ret

; End of PutCpPlc


;------------------------------------------------------------------------------
; bltbyte (pbFrom, pbTo, cb) - a block transfer of bytes from pbFrom to
; pbTo.  The size of the block is cb bytes.  This bltbyte() handles the
; case of overlapping source and destination.  bltbyte() returns a pointer
; to the end of the destination buffer (pbTo + cb).  NOTE - use this
; bltbyte to transfer within the current DS only--use the bltbx for
; FAR blts.
;-----------------------------------------------------------------------------

;NOTE: Except for cb in place of cw, bltbyteNat and bltNat
;must have the same arguments.
; %%Function:bltbyteNat %%Owner:BRADV
PUBLIC	bltbyteNat
bltbyteNat:
    mov     cl,0
    jmp     short bltEngine


        ;/* **** +-+utility+-+string **** */

;-----------------------------------------------------------------------------
; bltbxNat (qbFrom, qbTo, cb) - same as bltbyte above except everything is
; handled as FAR pointers.
;-----------------------------------------------------------------------------

;NOTE: Except for cb in place of cw, bltbxNat and bltxNat
;must have the same arguments.
; %%Function:bltbxNat %%Owner:BRADV
PUBLIC	bltbxNat
bltbxNat:
    mov     cl,0
    jmp     short bltxEngine


        ;/* **** +-+utility+-+string **** */

;------------------------------------------------------------------------------
; bltNat (pFrom, pTo, cw) - a block transfer of wFills from pFrom to pTo;
; The size of the block is cw wFills.  This blt() handles the case of
; overlapping source and destination.  blt() returns a pointer to the
; end of the destination buffer (pTo + cw).  NOTE - use this blt() to
; to transfer within the current DS only--use the bltx for FAR blts.
;-----------------------------------------------------------------------------

;NOTE: Except for cb in place of cw, bltbyteNat and bltNat
;must have the same arguments.
; %%Function:bltNat %%Owner:BRADV
PUBLIC	bltNat
bltNat:
    mov     cl,1

; %%Function:bltEngine %%Owner:BRADV
cProc bltEngine, <FAR, ATOMIC>, <si,di>
parmW	<pFrom>
parmW	<pTo>
parmW   <cw>

cBegin
    mov     si,pFrom            ; get pointers and length of blt
    mov     di,pTo
    push    ds                  ; set up segment registers
    pop     es
    mov     ax,cw
    shl     ax,cl
    xchg    ax,cx
    call    LN_bltCore
cEnd bltEngine

        ;/* **** +-+utility+-+string **** */

;-----------------------------------------------------------------------------
; bltxNat (qFrom, qTo, cw) - same as blt() above except everything is
; handled as FAR pointers.
;-----------------------------------------------------------------------------

;NOTE: Except for cb in place of cw, bltbxNat and bltxNat
;must have the same arguments.
; %%Function:bltxNat %%Owner:BRADV
PUBLIC	bltxNat
bltxNat:
    mov     cl,1

; %%Function:bltxEngine %%Owner:BRADV
cProc bltxEngine, <FAR, PUBLIC,ATOMIC>, <si,di>
parmD	<qFrom>
parmD	<qTo>
parmW   <cw>

cBegin
    les     di,qTo
    lds     si,qFrom
    mov     ax,cw
    shl     ax,cl
    xchg    ax,cx
    call    LN_bltCore
    mov     dx,es
cEnd bltxEngine



;-------------------------------------------------------------------------
;	LN_bltCore()
;-------------------------------------------------------------------------
;	core routine for bltbyteNat, bltbxNat, bltNat, bltxNat
;	LN_bltCore expects source in ds:si, destination in es:di, and
;	cb in cx.  Only ax, cx, si, and di are altered

LN_bltCore:
    mov     ax,di               ; calculate return value
    add     ax,cx
    cmp     si,di               ; reverse direction of the blt if
    jae     LN_bC01		;  necessary
    add     si,cx
    add     di,cx
    std
    dec     si
    dec     di
LN_bC01:
    shr     cx,1
    jnc     LN_bC02
    movsb
LN_bC02:
    cmp     si,di
    jae     LN_bC03
    dec     si
    dec     di
LN_bC03:
    rep     movsw
    cld
    ret


        ;/* **** +-+utility+-+string **** */

;-----------------------------------------------------------------------------
; bltcxNat (wFill, qTo, cw) - fills cw words of memory starting at FAR location
; qTo with wFill.
;-----------------------------------------------------------------------------

; %%Function:bltcxNat %%Owner:BRADV
cProc bltcxNat, <FAR, PUBLIC,ATOMIC>, <DI>
parmW   <wFill>
parmD   <qTo>
parmW   <cw>
        ;/* **** -+-utility-+- **** */

cBegin bltcxNat
    les     di,qTo              ; get the destination, constant, and count
    mov     ax,wFill
    mov     cx,cw
    rep     stosw               ; fill memory
cEnd bltcxNat


        ;/* **** +-+utility+-+string **** */

;-----------------------------------------------------------------------------
; bltbcxNat (bFill, qTo, cb) - fills cb bytes of memory starting at FAR location
; qTo with bFill.
;-----------------------------------------------------------------------------

; %%Function:bltbcxNat %%Owner:BRADV
cProc bltbcxNat, <FAR, PUBLIC,ATOMIC>, <DI>
parmB   <bFill>
parmD   <qTo>
parmW   <cb>
        ;/* **** -+-utility-+- **** */

cBegin bltbcxNat
    les     di,qTo              ; get the destination, constant, and count
    mov     al,bFill
    mov     cx,cb
    rep     stosb               ; fill memory
cEnd bltbcxNat


;-------------------------------------------------------------------------
;	CpMax(cp1,cp2)
;-------------------------------------------------------------------------
;NOTE: CpMax and CpMin must have the same arguments.
; /* C P  M A X */
; native CP CpMax(cp1, cp2)
; CP cp1, cp2;
; {
;	  return (cp1 > cp2) ? cp1 : cp2;
; }

;-------------------------------------------------------------------------
;	CpMin(cp1,cp2)
;-------------------------------------------------------------------------
;NOTE: CpMax and CpMin must have the same arguments.
; /* C P  M I N */
; native CP CpMin(cp1, cp2)
; CP cp1, cp2;
; {
;	  return (cp1 < cp2) ? cp1 : cp2;
; }

; %%Function:CpMax %%Owner:BRADV
PUBLIC CpMax
CpMax:
	call CM03
	jl CM01
	db	0CBh	    ;far ret

; %%Function:CpMin %%Owner:BRADV
PUBLIC CpMin
CpMin:
	call CM03
	jl CM02
CM01:
	xchg ax,bx
	xchg dx,cx
CM02:
	db	0CBh	    ;far ret

CM03:
	pop cx		;return address from CM03
	mov bx,sp
	pop ax		;offset of long return address
	xchg ax,[bx+8]	;offset of first argument
	pop dx		;segment of long return address
	xchg dx,[bx+10] ;segment of first argument
	xchg cx,[bx+6]	;segment of second argument
	pop bx		;offset of second argument
	cmp ax,bx
	push dx
	sbb dx,cx
	pop dx
	ret


;-------------------------------------------------------------------------
;	LN_PopTwoArgs()
;-------------------------------------------------------------------------
;LN_PopTwoArgs is a helper routine to reduce the size of the integer
;compare routines.  It takes the two arguments on the frame of the caller
;and replaces them in the caller's far return address.  The arguments are
;put into ax and dx, and cmp ax,dx is performed.
;ax, bx, cx, dx are altered.
LN_PopTwoArgs:
	pop cx		;return address from LN_PopTwoArgs
	mov bx,sp
	pop ax		;offset of long return address
	xchg ax,[bx+4]	;second argument
	pop dx		;segment of long return address
	xchg dx,[bx+6]	;first argument
	cmp ax,dx	;do the compare
	jmp cx		;return

;-------------------------------------------------------------------------
;	umax(x,y)
;-------------------------------------------------------------------------
; /* U M A X */
; native uns umax(u1, u2)
; uns u1, u2;
; {
; %%Function:umax %%Owner:BRADV
PUBLIC umax
umax:
;	  return (u1 > u2) ? u1 : u2;
;LN_PopTwoArgs is a helper routine to reduce the size of the integer
;compare routines.  It takes the two arguments on the frame of the caller
;and replaces them in the caller's far return address.  The arguments are
;put into ax and dx, and cmp ax,dx is performed.
;ax, bx, cx, dx are altered.
	call LN_PopTwoArgs
	ja umax01
	xchg ax,dx
umax01:
; }
	db	0CBh	    ;far ret

;-------------------------------------------------------------------------
;	umin(x,y)
;-------------------------------------------------------------------------
; /* U M I N */
; native uns umin(u1, u2)
; uns u1, u2;
; {
; %%Function:umin %%Owner:BRADV
PUBLIC umin
umin:
;	  return (u1 < u2) ? u1 : u2;
;LN_PopTwoArgs is a helper routine to reduce the size of the integer
;compare routines.  It takes the two arguments on the frame of the caller
;and replaces them in the caller's far return address.  The arguments are
;put into ax and dx, and cmp ax,dx is performed.
;ax, bx, cx, dx are altered.
	call LN_PopTwoArgs
	jb umin01
	xchg ax,dx
umin01:
; }
	db	0CBh	    ;far ret


;-------------------------------------------------------------------------
;	min(x,y)
;-------------------------------------------------------------------------
; /* M I N */
; native int min(x, y)
; int x, y;
; {
; %%Function:min %%Owner:BRADV
PUBLIC min
min:
;	  return(x < y ? x : y);
;LN_PopTwoArgs is a helper routine to reduce the size of the integer
;compare routines.  It takes the two arguments on the frame of the caller
;and replaces them in the caller's far return address.  The arguments are
;put into ax and dx, and cmp ax,dx is performed.
;ax, bx, cx, dx are altered.
	call LN_PopTwoArgs
	jl min01
	xchg ax,dx
min01:
; }
	db	0CBh	    ;far ret


;-------------------------------------------------------------------------
;	max(x,y)
;-------------------------------------------------------------------------
; /* M A X */
; native int max(x, y)
; int x, y;
; {
; %%Function:max %%Owner:BRADV
PUBLIC max
max:
;	  return(x > y ? x : y);
;LN_PopTwoArgs is a helper routine to reduce the size of the integer
;compare routines.  It takes the two arguments on the frame of the caller
;and replaces them in the caller's far return address.  The arguments are
;put into ax and dx, and cmp ax,dx is performed.
;ax, bx, cx, dx are altered.
	call LN_PopTwoArgs
	jg max01
	xchg ax,dx
max01:
	db	0CBh	    ;far ret


;-------------------------------------------------------------------------
;	abs(x)
;-------------------------------------------------------------------------
; /* A B S */
; native int abs(x)
; int x;
; {
; %%Function:N_abs %%Owner:BRADV
PUBLIC N_abs
N_abs:
;	  return(x < 0 ? -x : x);
	pop cx	    ;offset of long return address
	pop dx	    ;segment of long return address
	pop ax
	push dx
	push cx
	or ax,ax
	jge abs01
	neg ax
abs01:
	db	0CBh	    ;far ret
; }


        ;/* **** +-+utility+-+math **** */

;/* D X A  F R O M  D X P  - return xa value from xp */
;NATIVE DxaFromDxp(pwwd, dxp)
;int dxp;
;struct WWD *pwwd;
;{
;	 struct MWD *pmwd = PmwdMw(pwwd->mw);
;
;	 return NMultDiv(dxp, dxaInch, WinMac(vfti.dxpInch, DxsSetDxsInch(pmwd->doc)));
;}
; %%Function:N_DxaFromDxp %%Owner:BRADV
PUBLIC N_DxaFromDxp
N_DxaFromDxp:
	db	0A8h	;turns next "stc" into "test al,immediate"
			;also clears the carry flag

;/* D X P  F R O M  D X A  - return dxp value from dxa */
;NATIVE DxpFromDxa(pwwd, dxa)
;int dxa;
;struct WWD *pwwd;
;{
;	 struct MWD *pmwd = PmwdMw(pwwd->mw);
;
;	 return NMultDiv(dxa, WinMac(vfti.dxpInch, DxsSetDxsInch(pmwd->doc)), dxaInch);
;}
; %%Function:N_DxpFromDxa %%Owner:BRADV
PUBLIC N_DxpFromDxa
N_DxpFromDxa:
	stc
	;Assembler note: N_DxaFromDxp and N_DxpFromDxa have only one argument
	;as a result of macros in debug.h (pwwd is not needed for WIN).
	pop	dx
	pop	cx
	mov	ax,[vfti.dxpInchFti]
	mov	bx,dxaInch
	jnc	DFD01
	xchg	ax,bx
DFD01:
	push	bx
	push	ax
	push	cx
	push	dx	    ;fall through to NMultDiv

;-----------------------------------------------------------------------------
; NMultDiv(w, Numer, Denom) returns (w * Numer) / Denom rounded to the nearest
; integer.  A check is made so that division by zero is not attempted.
; This version uses signed numbers; for unsigned, use UMultDiv, below.
;-----------------------------------------------------------------------------

; %%Function:NMultDiv %%Owner:BRADV
cProc NMultDiv, <FAR, PUBLIC,ATOMIC>
parmW  <w, Numer, Denom>
        ;/* **** -+-utility-+- **** */

cBegin NMultDiv
    mov     bx,Denom    ; get the demoninator
    mov     cx,bx       ; cx holds the final sign
    or      bx,bx       ; ensure the denominator is positive
    jns     md1
    neg     bx
md1:
    mov     ax,w        ; get the word we are multiplying
    xor     cx,ax       ; make cx reflect any sign change
    or      ax,ax       ; ensure this word is positive
    jns     md2
    neg     ax
md2:
    mov     dx,Numer    ; get the numerator
    xor     cx,dx       ; make cx reflect any sign change
    or      dx,dx       ; ensure the numerator is positive
    jns     md3
    neg     dx
md3:
    mul     dx          ; multiply
    push    bx
    shr     bx,1	; get half of the demoninator to adjust for rounding
    add     ax,bx       ; adjust for possible rounding error
    adc     dx,0        ; this is really a long addition
    pop     bx		; restore the demoninator
                        ; REVIEW overflow code (see UMULTDIV, below)
    cmp     dx,bx       ; check for overflow
    jae     md6
    div     bx          ; divide
    or      ax,ax       ; if sign is set, then overflow occured
    js	    md6
md4:
    or      cx,cx       ; put the sign on the result
    jns     md5
    neg     ax
md5:

cEnd NMultDiv

md6:
    mov     ax,7FFFh    ; return the largest integer
    jmp     short md4


        ;/* **** +-+utility+-+math **** */

;-----------------------------------------------------------------------------
; UMultDiv(w, Numer, Denom) returns (w * Numer) / Denom rounded to the nearest
; integer.  A check is made so that division by zero is not attempted.
;
; UMultDiv is the UNSIGNED version.  For SIGNED, see MultDiv above.
;-----------------------------------------------------------------------------

; %%Function:UMultDiv %%Owner:BRADV
cProc UMultDiv, <FAR, PUBLIC,ATOMIC>
parmW  <w, Numer, Denom>
        ;/* **** -+-utility-+- **** */

cBegin UMultDiv
    mov     bx,Denom    ; bx = Denom

    mov     ax,w        
    mov     dx,Numer
    mul     dx          ; dx:ax = w * Numer

    mov     cx, bx      ; dx:ax += Denom / 2, to adjust for rounding
    shr     cx, 1
    add     ax, cx
    adc     dx, 0
    jc      umdOv       ; brif Numeric overflow


; Overflow checks:      (1) Denom == 0
;                       (2) result will be > 0xFFFF
;                       (div instruction generates div-by-0 in this case too)
; Following sequence catches both these conditions by this method:
;   quotient = numer/denom - rem/denom =>  numer = quotient * denom + rem
;   numer is in dx:ax. if quotient is < 10000h (i.e., will fit in 16 bits)
;   then  numer/denom - rem/denom < 10000h and rem/denom is always 0 in integer math.
;   so dx:ax < denom * 10000h. Since multiplying by 10000h is a 16 bit shift,
;   if dx < denom (bx) we are ok.
;    got that? took me a while (bz)

    cmp     dx,bx
    jae     umdOv

    div     bx          ; ax = dx:ax / bx, dx := dx:ax % bx
umdEnd:
cEnd UMultDiv

umdOv:                  ; Overflow, return 0xFFFF
    mov     ax,0FFFFh
    jmp     umdEnd

        ;/* **** +-+utility+-+string **** */

; /* **** +-+utility+-+string **** */

; /* ****
; *  Description: Returns pointer to first occurrence of character ch found
; *     in null-terminated string pch, or 0 if ch does not appear.
; *     If ch==0, we return a pointer to the null terminator.
; ** **** */

; CHAR *index(pch, ch)

; %%Function:index %%Owner:BRADV
PUBLIC index
index:
	pop ax
	pop dx	    ;far return address in dx:ax
	pop cx
	pop bx
	push dx
	push ax

indx000:
        mov al,[bx]
        cmp al,cl
        jz indx001
        inc bx
        or al,al
        jnz indx000
        xor bx,bx
indx001:
        xchg ax,bx
	db	0CBh	    ;far ret


ifdef DEBUG
;-------------------------------------------------------------------------
;	BltInPlc(bpm, hplc, i, dicp, difoo, c)
;-------------------------------------------------------------------------
;/* B L T  I N  P L C */
;EXPORT BltInPlc(bpm, hplc, i, dicp, difoo, c)
;int bpm;
;struct PLC **hplc;
;uns i, c;
;int dicp, difoo;
;{
;	long	ib, dib, lcb, lcbFoo;
;	struct PLC *pplc;
; %%Function:BltInPlc %%Owner:BRADV
cProc	BltInPlc,<PUBLIC,FAR>,<si,di>
	ParmW	bpm
	ParmW	hplc
	ParmW	iArg
	ParmW	dicp
	ParmW	difoo
	ParmW	cArg

cBegin
;	AssertH( hplc );
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	bx,[hplc]
	mov ax, midResn
	mov cx, 17		       ; label # for native assert
	cCall	AssertHForNative,<bx, ax, cx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */
;	pplc = *hplc;
	mov	bx,[hplc]
	mov	bx,[bx]
;	LN_LprgcpForPlc takes pplc in register bx, and returns an
;	lp in es:di.  Only dx, di, es are altered.
	cCall	LN_LprgcpForPlc,<>

;	ib = i;
;	dib = dicp;
;	dib <<= 2;
	mov	si,[dicp]
	shl	si,1
	shl	si,1

;	lcb = c;
	mov	ax,[cArg]	;ib in iArg, dib in si, lcb in ax

;	if (bpm == bpmCp)
;		{
	cmp	[bpm],bpmCp
	jne	BIP02

;		Assert(difoo == 0);
;		lcb <<= 2;
;		ib <<= 2;
;		}
	xchg	ax,cx
	shl	cx,1
	shl	cx,1
	mov	bx,[iArg]
	shl	bx,1
	shl	bx,1
	jmp	short BIP03	;ib in bx, dib in si, lcb in cx

;	else
;		{
BIP02:

;		lcbFoo = pplc->cb;
;		lcb *= lcbFoo;
	mov	cx,[bx.cbPlc]
	mul	cx
	xchg	ax,cx

;		dib += difoo*lcbFoo;
	mul	[difoo]
	add	si,ax

;		ib *= pplc->cb;
;		ib += pplc->iMax << 2;
	mov	ax,[bx.cbPlc]
	mul	[iArg]
	mov	bx,[bx.iMaxPlc]
	shl	bx,1
	shl	bx,1
	add	bx,ax		;ib in bx, dib in si, lcb in cx

;		}
BIP03:

;	if (!(*hplc)->fExternal)
;		{
;		char *pchFrom = (char *)pplc + cbPLCBase + ib;
;		bltb(pchFrom, pchFrom + dib, (uns)lcb);
;		}
;	else
;		{
;		char HUGE *hpchFrom = (char HUGE *)*pplc->hqplce + ib;
;		bltbx(pchFrom, pchFrom + dib, (uns)lcb);
;		}
	push	es
	pop	ds
	xchg	si,di
	add	si,bx
	add	di,si
;	LN_bltCore expects source in ds:si, destination in es:di, and
;	cb in cx.  Only ax, cx, si, and di are altered
	call	LN_bltCore
	push	ss
	pop	ds
;}
cEnd

; End of BltInPlc
endif ;DEBUG


;-------------------------------------------------------------------------
;	FNeRgch(rgb1, rgb2, cb)
;-------------------------------------------------------------------------
; /* F  N E  R G C H */
; /* true iff rgb's are not equal */
; native int FNeRgch(rgb1, rgb2, cb)
; char *rgb1, *rgb2; int cb;
; {
;       int ib;
;       for (ib = 0; ib < cb; ib++)
;               if (rgb1[ib] != rgb2[ib])
;                       return fTrue;
;       return fFalse;
; }


;-------------------------------------------------------------------------
;	FNeRgw(rgw1, rgw2, cw)
;-------------------------------------------------------------------------
; /* F  N E  R G W */
; /* true iff rgw's are not equal */
; native int FNeRgw(rgw1, rgw2, cw)
; int *rgw1, *rgw2, cw;
; {
;       int iw;
;       for (iw = 0; iw < cw; iw++)
;               if (rgw1[iw] != rgw2[iw])
;                       return fTrue;
;       return fFalse;
; }


;-------------------------------------------------------------------------
;	FNeSt(st1, st2)
;-------------------------------------------------------------------------
; /* F  N E  S T */
; /* true iff st's are not equal */
; native int FNeSt(st1, st2)
; char *st1, *st2;
; {
;       return(*st1 != *st2 || FNeRgch(st1 + 1, st2 + 1, *st1));
; }

; %%Function:FNeSt %%Owner:BRADV
PUBLIC	FNeSt
FNeSt:
	pop	ax	;far return address in dx:ax
	pop	dx
	pop	bx
	mov	cl,[bx]
	xor	ch,ch
	inc	cx
	push	bx
	push	cx
	push	dx
	push ax


; %%Function:FNeRgch %%Owner:BRADV
PUBLIC	FNeRgch
FNeRgch:
	db	0A8h	;turns next "stc" into "test al,immediate"
			;also clears the carry flag
			;fall through to FNeRgw

; %%Function:FNeRgw %%Owner:BRADV
PUBLIC	FNeRgw
FNeRgw:
	stc
	mov	bx,sp
	xchg	si,[bx+6]
	xchg	di,[bx+8]
	push	ds
	pop	es
	pop	bx
	pop	dx	;far return address in dx:bx
	pop	cx
	rcl	al,1
	test	al,0	;set zero because "repz cmpsw" doesn't if cx is 0
	rcr	al,1
	jnc	FNR01
	repz	cmpsw
	db	0B8h	;turns "repsz cmpsb" to "mov ax,immediate"

FNR01:
	repz	cmpsb
FNR02:
	jnz	FNR03
	db	0A8h	;turns next "stc" into "test al,immediate"
			;also clears the carry flag
FNR03:
	stc
	sbb	ax,ax
	pop	si
	pop	di
	push	dx
	push	bx
	db	0CBh	    ;far ret


;-------------------------------------------------------------------------
;	N_PFcbFn( fn )
;-------------------------------------------------------------------------
; %%Function:N_PfcbFn %%Owner:BRADV
PUBLIC	N_PfcbFn
N_PfcbFn:
	mov	ax,dataOffset mpfnhfcb
ifdef DEBUG
	mov	cx,301
endif
	jmp	short LRghEngine

;-------------------------------------------------------------------------
;	N_PwwdWw( ww )
;-------------------------------------------------------------------------
; %%Function:N_PwwdWw %%Owner:BRADV
PUBLIC	N_PwwdWw
N_PwwdWw:
	mov	ax,dataOffset mpwwhwwd
ifdef DEBUG
	mov	cx,302
endif
	jmp	short LRghEngine

;-------------------------------------------------------------------------
;	N_PmwdMw( mw )
;-------------------------------------------------------------------------
; %%Function:N_PmwdMw %%Owner:BRADV
PUBLIC	N_PmwdMw
N_PmwdMw:
	mov	ax,dataOffset mpmwhmwd
ifdef DEBUG
	mov	cx,303
endif
	jmp	short LRghEngine

;-------------------------------------------------------------------------
;	N_PdodDoc( doc )
;-------------------------------------------------------------------------
; %%Function:N_PdodDoc %%Owner:BRADV
PUBLIC	N_PdodDoc
N_PdodDoc:
	mov	ax,dataOffset mpdochdod
ifdef DEBUG
	mov	cx,304
endif
;	 jmp	 short LRghEngine


; %%Function:LRghEngine %%Owner:BRADV
PUBLIC	LRghEngine
LRghEngine:

cBegin
;	  Assert (mpifoohfood[ifoo] != hNil);
;	  return *mpwwhwwd[ww];
ifdef DEBUG
	push	es
	push	si
	push	di
	push	ds
	pop	es
	mov	si,sp
	mov	bx,[si+10]
	add	si,8
	lea	di,[si+2]
	std
	movsw
	movsw
	cld
	pop	di
	pop	si
	pop	es
	add	sp,2
else
	pop	cx
	pop	dx	;far return address in dx:cx
	pop	bx
	push	dx
	push	cx
endif ;DEBUG
	shl	bx,1
	add	bx,ax
	mov	bx,[bx]
ifdef DEBUG
	or	bx,bx
	jnz	L301
	inc	bp
	push	bp
	mov	bp,sp
	push	ds
	push	bx
	mov	ax,midResn
; ASSERT VALUES: (assert means null handle passed in)
;	301		PfcbFn
;	302		PwwdWw
;	303		PmwdMw
;	304		PdodDoc
	mov	bx,cx
	cCall	AssertProcForNative,<ax,bx>
	pop	bx
	pop	ds
	pop	bp
	dec	bp
L301:
	push	ds  ;ds may now be a handle
	push	ss
	pop	ds  ;restore valid ds
endif ;DEBUG
	mov	ax,[bx]
; }
ifdef DEBUG
	pop	ds	;restore segment handle
endif ;DEBUG
	db	0CBh	    ;far ret


ifdef NOTUSED
;-------------------------------------------------------------------------
;	FHplcNil(hplc)
;-------------------------------------------------------------------------
; native char *HPLcNil(hplc)
; struct PLC **hplc;
; {
; %%Function:FHplcNil %%Owner:NOTUSED
PUBLIC	FHplcNil
FHplcNil:
	pop cx
	pop dx	    ;far return address in dx:cx
	pop bx
	push dx
	push cx

; 	return ((hplc == hNil) || ((*hplc)->iMac == 0))
	errnz	<fFalse>
	xor	ax,ax
        or      bx,bx
	jz	FHplcNil1
	mov	bx,[bx]
        cmp     [bx.iMacPlcSTR],0
	jnz	FHplcNil2
FHplcNil1:
	inc	ax
FHplcNil2:

; }
	db	0CBh	    ;far ret
endif ;NOTUSED

sEnd	resn
	end
