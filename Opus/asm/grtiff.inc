ifdef DEBUG
externFP	<AssertProcForNative,ScribbleProc,ShakeHeapSb>
endif ;/* DEBUG */

; *** macros stolen from pc word to support tiff *******


AssertEQ MACRO expr1,expr2,mid,line	; if (expr1 != expr2) halt
	LOCAL	ok
ifdef DEBUG
	push	ax		; save ax
	mov	ax,expr1	;
	cmp	ax,expr2	;
	pop	ax		; restore ax, doesn't affect flags
	je	ok		; ok iff compare was zero
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,mid
	mov	bx,line
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif
ok:
ENDM

AssertGT MACRO expr1,expr2,mid,line	; if (expr1 <= expr2) halt
	LOCAL	ok
ifdef DEBUG
	push	ax		; save ax
	mov	ax,expr1	;
	cmp	ax,expr2	; compared to second
	pop	ax		; restore ax, doesn't affect flags
	jg	ok		; ok iff expr1 < expr2
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,mid
	mov	bx,line
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif
ok:
ENDM



;--------------------------------------------------------------------------
;   cassume
;	Conditional assume macro.  Creates an "assumes" statement only
;	if segreg is a segment register
;--------------------------------------------------------------------------
cassume	MACRO	segreg,segment
	irp	sr,<es,ds,ss>
	ifidni	<segreg>,<sr>
	assumes	<segreg>,<segment>
	endif
	endm
ENDM

EXPMSG	MACRO	name
if1
	%out	Expanding name in-line
endif
	ENDM

CallProc	MACRO	name,parms
	cCall	<&name>,<&parms>
	ENDM

;**************************************************************************
;
; Macros for using SB's
;
; Stolen from cbox's macros.inc, 29 Apr 88
;
;**************************************************************************

externA	_sbMax
ifndef	dos_asm
externFP ReloadSb
endif
;
; only save reg if in save1 - save4 (used in PsFromSb)
;
PushSelectReg MACRO	reg,save1,save2,save3,save4
      ifidn <save1>,<reg>
	push	save1
      endif
      ifidn <save2>,<reg>
	push	save2
      endif
      ifidn <save3>,<reg>
	push	save3
      endif
      ifidn <save4>,<reg>
	push	save4
      endif
ENDM

;
; Restore reg if in save1 - save4 (used in PsFromSb)
;
PopSelectReg MACRO	reg,save1,save2,save3,save4
      ifidn <save1>,<reg>
	pop	save1
      endif
      ifidn <save2>,<reg>
	pop	save2
      endif
      ifidn <save3>,<reg>
	pop	save3
      endif
      ifidn <save4>,<reg>
	pop	save4
      endif
ENDM

;
; Save all registers in save1-4 except reg (used in PsFromSb)
;
PushNReg MACRO	reg,save1,save2,save3,save4
      ifdif <save1>,<>
       ifdif <save1>,<reg>
	push	save1
       endif
      endif

      ifdif <save2>,<>
       ifdif <save2>,<reg>
	push	save2
       endif
      endif

      ifdif <save3>,<>
       ifdif <save3>,<reg>
	push	save3
       endif
      endif

      ifdif <save4>,<>
       ifdif <save4>,<reg>
	push	save4
       endif
      endif
ENDM

;
; Restore all registers in save1-4 except reg. (used in PsFromSb)
;
PopNReg MACRO	reg,save1,save2,save3,save4
      ifdif <save1>,<>
       ifdif <save1>,<reg>
	pop	save1
       endif
      endif

      ifdif <save2>,<>
       ifdif <save2>,<reg>
	pop	save2
       endif
      endif

      ifdif <save3>,<>
       ifdif <save3>,<reg>
	pop	save3
       endif
      endif

      ifdif <save4>,<>
       ifdif <save4>,<reg>
	pop	save4
       endif
      endif
ENDM

;
; Save any specified registers on the stack.
;
PushReg MACRO	save1,save2,save3,save4
      ifdif <save1>,<>
	push	save1
      endif
      ifdif <save2>,<>
	push	save2
      endif
      ifdif <save3>,<>
	push	save3
      endif
      ifdif <save4>,<>
	push	save4
      endif
ENDM
;
; Restore the specified registers from the stack.
;
PopReg MACRO	save1,save2,save3,save4
      ifdif <save1>,<>
	pop	save1
      endif
      ifdif <save2>,<>
	pop	save2
      endif
      ifdif <save3>,<>
	pop	save3
      endif
      ifdif <save4>,<>
	pop	save4
      endif
ENDM


;==========================================================================
;   Segmentation macros.
;	These macros are used to change the current sb and/or load the
;	physical segment (ps).  The scratchReg parameter is optional.
;	If no scratchReg is specified, an extra push/pop usually is done
;	to free up a register to use.  If sb != scratchReg, an extra
;	register move is usually necessary.  An appropriate CMACROS
;	"assumes" statement is included.
;==========================================================================


;--------------------------------------------------------------------------
;   PsFromSb
;	Load the specified segment register with the physical segment
;	specified by sb.  The special cases of sb == sbDds and sb == sbCur
;	are taken into account if the parameter is "sbDds" or "sbCur".
;
;	EMM support:  if sb is other than sbCur or sbDds, then ReloadSb
;	may be called to load a segment.  in this case the save registers
;	will be pushed to preserve values.
;
;	note that if sb is other than sbCur or sbDds you must give
;	a scratch register.  if you don't have one, then also specify it
;	as a save register.  in this case, the most efficient call is with
;	segreg = es and scratchreg = bx (and obviously, no save registers).
;
;	To ensure that 2 segments other than sbCur are resident this macro
;	must be used 3 times -- once with one segment, then with the 2nd,
;	then again with the first again.
;	
;	UPDATE: ds does not need to equal ss on entry - ss overide prefix is
;	now used to access mpsbps. Since ReloadSb loads es, an extra push
;	and pop of es is required if es != segreg.  If you want to use 
;	ds = segreg, it is best to use the macro with es, then set ds = es.
;
;	OS/2 support (DOS5 flag): ReloadSb isn't called, since it doesn't 
;	exist.  Just load up the segreg, and get out.
;
;--------------------------------------------------------------------------
PsFromSb MACRO	segreg,sb,scratchReg,save1,save2,save3,save4
	LOCAL	ok
	LOCAL	ok1

ifidn	<sb>,<sbDds>				; if sb == sbDds == 1
  ifidn <scratchReg>,<>
	errnz	<sbDds-1>		; error at assembly if sbDds != 1
	mov	segreg,ss:mpsbps[2]		; segreg = mpsbps[sbDds]
  else
	mov	scratchReg,ss
	mov	segreg,scratchReg
  endif
	cassume	segreg,DATA			;
else
  ifidn	<sb>,<sbCur>
	mov	segreg,psCur			; segreg = psCur
	cassume	segreg,NOTHING
  else
      PushSelectReg ax,save1,save2,save3,save4	;save ax if in save1-4
      ifdif <scratchReg>,<sb>
	mov	scratchReg,sb
      endif
      ifdef	DEBUG
	cmp	scratchReg,_sbMax
	jb	ok1
	int	3
ok1:
      endif	; DEBUG
	shl	scratchReg,1
	mov	segreg,ss:mpsbps[scratchReg] 	;segreg = mpsbps[sb]

      ifndef	DOS5
	mov	ax,segreg
	shr	ax,1
	jc	ok
    PushNReg ax,save1,save2,save3,save4 	;save all regs except ax
      ifdif <scratchreg>,<bx>
	mov	bx,scratchreg
      endif
      ifdif <segreg>,<es>
	push	es
      endif
	call	ReloadSb
      ifdif <segreg>,<es>
	mov	ax,es
	mov	segreg,ax
	pop	es
      endif
    PopNReg ax,save4,save3,save2,save1	;restore all regs except ax
ok:
      endif	; !DOS5

	PopSelectReg	ax,save4,save3,save2,save1  ;restore ax if in save1-4
	cassume	segreg,NOTHING
  endif
endif
ENDM


; useful for accessing far pointers, longs, or double words in memory
wHigh	equ	2+word ptr	;; upper word (or segment) of a long
wLow	equ	word ptr	;; lower word (or offset) of a long

bPtr	EQU	BYTE PTR

    ; constants - taken from grconsts.db in PC WORD 

icoBitMax	equ	16
cbRowPat	equ	icoBitMax

    ; constants - from grstruct.h compiled with -Si flag 
Scheme1		equ	1

    ; data structures - from grstruct.h compiled with -Si flag 

xGpoint		equ	[word ptr 00000h]
yGpoint		equ	[word ptr 00002h]
hGpoint		equ	[word ptr 00000h]
vGpoint		equ	[word ptr 00002h]
cbGpointMin	equ	00004h

    ; TIFF structure
wCompSchemeTiff	equ	[word ptr 00000h]
crwStripTiff	equ	[word ptr 00002h]
fcStripOffsetsTiff	equ	[dword ptr 00004h]
LO_fcStripOffsetsTiff	equ	[word ptr 00004h]
HI_fcStripOffsetsTiff	equ	[word ptr 00006h]
fShortStripOffsetTiff	equ	[word ptr 00008h]
irwNextTiff	equ	[word ptr 0000Ah]
fcPrevTiff	equ	[dword ptr 0000Ch]
LO_fcPrevTiff	equ	[word ptr 0000Ch]
HI_fcPrevTiff	equ	[word ptr 0000Eh]
fcTrailTiff	equ	[dword ptr 00010h]
LO_fcTrailTiff	equ	[word ptr 00010h]
HI_fcTrailTiff	equ	[word ptr 00012h]
irwTrailTiff	equ	[word ptr 00014h]
cbitsSampleTiff	equ	[word ptr 00016h]
fSwitchBytesTiff	equ	[word ptr 00018h]
cbRowFileTiff	equ	[word ptr 0001Ah]
cbTiffMin	equ	0001Ch

     ; PICT structure
ftPict		equ	[word ptr 00000h]
fnPict		equ	[word ptr 00002h]
vtOrgPict	equ	[dword ptr 00004h]
LO_vtOrgPict	equ	[word ptr 00004h]
HI_vtOrgPict	equ	[word ptr 00006h]
vtExtPict	equ	[dword ptr 00008h]
LO_vtExtPict	equ	[word ptr 00008h]
HI_vtExtPict	equ	[word ptr 0000Ah]
pixOrgPict	equ	[dword ptr 0000Ch]
LO_pixOrgPict	equ	[word ptr 0000Ch]
HI_pixOrgPict	equ	[word ptr 0000Eh]
pixExtPict	equ	[dword ptr 00010h]
LO_pixExtPict	equ	[word ptr 00010h]
HI_pixExtPict	equ	[word ptr 00012h]
bits0Pict	equ	[word ptr 00014h]
bits0recPict	record	fSparePict:12,fReallocPict:1,fGrayDitherPict:1,fColorDitherPict:1,fLSRotatePict:1
cgraphicbytesPict	equ	[word ptr 00016h]
crwSwathPict	equ	[word ptr 00018h]
cbSwathMinPict	equ	[word ptr 0001Ah]
cbRowMinPict	equ	[word ptr 0001Ch]
iPassPict	equ	[word ptr 0001Eh]
cplInPict	equ	[word ptr 00020h]
cplOutPict	equ	[word ptr 00022h]
cbExtraPict	equ	[word ptr 00024h]
cbRowInPict	equ	[word ptr 00026h]
cbRowOutPict	equ	[word ptr 00028h]
cbPassOutPict	equ	[word ptr 0002Ah]
sbBitsPict	equ	[word ptr 0002Ch]
cbBitsPict	equ	[word ptr 0002Eh]
cbPlanePict	equ	[word ptr 00030h]
ibInPict	equ	[word ptr 00032h]
irwInMaxPict	equ	[word ptr 00034h]
irwOutMaxPict	equ	[word ptr 00036h]
irwInMicPict	equ	[word ptr 00038h]
irwOutMicPict	equ	[word ptr 0003Ah]
irwInMacPict	equ	[word ptr 0003Ch]
irwOutMacPict	equ	[word ptr 0003Eh]
irwOutNextPict	equ	[word ptr 00040h]
smPict		equ	[word ptr 00042h]
crwRepNormPict	equ	[word ptr 00044h]
wRowDeltaPict	equ	[word ptr 00046h]
crwRevRepNormPict	equ	[word ptr 00048h]
wRevRowDeltaPict	equ	[word ptr 0004Ah]
cbitRepNormPict	equ	[word ptr 0004Ch]
wBitDeltaPict	equ	[word ptr 0004Eh]
sbPatPict	equ	[word ptr 00050h]
rgicoPict	equ	[00052h]
hbmPict		equ	[word ptr 00062h]
dybBmPict	equ	[word ptr 00064h]
hMDCPict	equ	[word ptr 00066h]
rcClipPict	equ	[00068h]
tiffPict	equ	[00070h]
cbPictMin	equ	0008Ch

  ; STM structure
fnStm		equ	[word ptr 00000h]
fcStm		equ	[dword ptr 00002h]
LO_fcStm		equ	[word ptr 00002h]
HI_fcStm		equ	[word ptr 00004h]
fcMacStm	equ	[dword ptr 00006h]
LO_fcMacStm	equ	[word ptr 00006h]
HI_fcMacStm	equ	[word ptr 00008h]
cbStmMin	equ	0000Ah

; External Data **********************************
sBegin	data

extrn	_sbMax:ABS
sbMax	equ	_sbMax

externW	sbCur
externW	mpsbps
externW	psCur
externW	stmGlobal
sEnd


