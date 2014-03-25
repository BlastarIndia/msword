	.xlist
	?USEDS equ 1
	?USEBP equ 1
	?WIN = 0
	include cmacros.inc
	.list

	;Must be different from eldde_PCODE
createSeg	_ELDDE,eldden,byte,public,CODE

ifdef HYBRID
externFP	<GlobalLockPR>
externFP	<GlobalUnlockPR>
externFP	<GlobalFreePR>
else ;!HYBRID
externFP	<GlobalLock>
externFP	<GlobalUnlock>
externFP	<GlobalFree>
endif ;!HYBRID
externFP	<DefHookProc>
externFP	<UnhookWindowsHook,FreeProcInstance,SetKeyboardState>
externFP	<ToAscii>

;
;  system queue message structure (same as above only different)
;
SYSMSG STRUC
        smMessage   dw  ?
        smParamL    dw  ?
        smParamH    dw  ?
        smTime      dd  ?
SYSMSG ENDS

; These structures are duplicated in dde.h
EVENT	STRUC
wmEv		dw	?
vkEv		dw	?
EVENT	ENDS

EVT	STRUC
ieventCur	dw	?
ieventMac	dw	?
rgevent		db	(SIZE EVENT) DUP (?)
EVT	ENDS

HC_GETNEXT	equ	1
HC_SKIP		equ	2
WH_JOURNALPLAYBACK	equ	1
VK_SHIFT	equ	10H

;
; struct DRVDATA definition
;
hevtHeadDrvdata 	    equ     [word ptr 00000h]
hrgbKeyStateDrvdata	    equ     [word ptr 00002h]
lpfnPlaybackHookSaveDrvdata equ     [word ptr 00004h]
cbDrvdataMin		    equ     00008h

sBegin	eldden
	assumes cs,eldden
	assumes ds,nothing
	assumes ss,nothing

; don't mess with this unless you change the DRVHD declaration in dde.h
; this MUST be the first thing in this segment!
globalW wDataSeg,0
globalW wWinVersion,0

; %%Function:PlaybackHook %%Owner:peterj
cProc	PlaybackHook,<PUBLIC,FAR>,<si,di>
	ParmW	hc
	ParmW	wParam
	ParmD	lpsysmsg

cBegin
	mov	di,hc
	cmp	di,HC_SKIP
	je	ph10
	cmp	di,HC_GETNEXT
	je	ph10
	mov	ax,(lpfnPlaybackHookSaveDrvdata)
	cCall	DefHookProc,<hc, wParam, lpsysmsg, cs:[wDataSeg], ax>
	jmp	phx1
ph10:
	mov	es,cs:[wDataSeg]
	mov	ax,es:[hevtHeadDrvdata]
	push	ax
ifdef HYBRID
	push	ds
	push	ss
	pop	ds
	cCall	GlobalLockPR,<ax>
	pop	ds
else ;!HYBRID
	cCall	GlobalLock,<ax>
endif ;!HYBRID
	push	ds
	mov	ds,dx
	mov	si,ax
	cmp	di,HC_SKIP
	je	ph13
        jmp     ph20
ph13:
; Handle Skip message
	inc	[si].ieventCur
	mov	di,[si].ieventCur
	cmp	di,[si].ieventMac
	jge	ph15
	jmp	ph30
ph15:
; Uninstall windows hook
	pop	ds
	mov	ax,WH_JOURNALPLAYBACK
	lea	bx,[PlaybackHook]
	cCall	UnhookWindowsHook,<ax, cs, bx>
	pop	ax
	push	ax
	push	ax	;duplicate hevtHead on the stack
ifdef HYBRID
	pop	ax
	push	ds
	push	ss
	pop	ds
	push	ax
	cCall	GlobalUnlockPR;<es:[hevtHeadDrvdata]> args already pushed
	pop	ds	
else ;!HYBRID
	cCall	GlobalUnlock;<es:[hevtHeadDrvdata]> args already pushed
endif ;!HYBRID
ifdef HYBRID
	pop	ax
	push	ds
	push	ss
	pop	ds
	push	ax
	cCall	GlobalFreePR;<es:[hevtHeadDrvdata]> args already pushed
	pop	ds	
else ;!HYBRID
	cCall	GlobalFree;<es:[hevtHeadDrvdata]> args already pushed
endif ;!HYBRID
        push    es
        mov     es,cs:[wDataSeg]
	mov	es:[hevtHeadDrvdata],0
	mov	ax,es:[hrgbKeyStateDrvdata]
        pop     es
	or	ax,ax
	je	Ltemp001
ifdef HYBRID
	push	ds
	push	ss
	pop	ds
	cCall	GlobalLockPR,<ax>
	pop	ds	
else ;!HYBRID
	cCall	GlobalLock,<ax>
endif ;!HYBRID
	mov	cx,ax
	or	cx,dx
	je	phx
	push	dx
	push	ax  ;save hp
	cCall	SetKeyboardState,<dx, ax>
	pop	ax
	pop	dx  ;restore hp

	; Kludge alert!!  For those of us not priviledged to know secret
	; windows internals ToAscii (according to Peter Belew) is "NOT
	; a part of the Windows API, and should NOT be documented in the
	; SDK documentation.  It is an INTERNAL Windows function.  Its
	; output is not a deterministic result of its parameter values,
	; since it has an internal state.  This means that Microsoft apps,
	; as well as others. should not use it."
	;
	; However, according to Ed Fries of the Excel group the only way
	; "reset the lock lights" was to call ToAscii.
	;
	; Hopefully, SetKeyboardState will take care of the lock lights
	; in version 3.00 and we won't need to call it.  Calling it as
	; it is coded here causes problems under version 3.00.	In protect
	; mode it causes GP faults and if you aren't under protect mode
	; it will smash interrupt vectors.
	;
	; Don't you love this kludge?

	cmp	cs:[wWinVersion],00300h
	jae	ph17	;Don't call ToAscii if version >= 3.00
	mov	cx,VK_SHIFT
	push	cx
	xor	cx,cx
	push	cx
	push	dx
	push	ax
	push	cx
	push	cx
	push	cx
	cCall	ToAscii,<>
ph17:

        push    es
        mov     es,cs:[wDataSeg]
	mov	ax,es:[hrgbKeyStateDrvdata]
        pop     es
	push	ax
ifdef HYBRID
	push	ds
	push	ss
	pop	ds
	cCall	GlobalUnlockPR,<ax>
	pop	ds	
else ;!HYBRID
	cCall	GlobalUnlock,<ax>
endif ;!HYBRID
ifdef HYBRID
	pop	ax
	push	ds
	push	ss
	pop	ds
	push	ax
	cCall	GlobalFreePR;<ax> args already pushed
	pop	ds	
else ;!HYBRID
	cCall	GlobalFree;<ax> args already pushed
endif ;!HYBRID
        push    es
        mov     es,cs:[wDataSeg]
	mov	es:[hrgbKeyStateDrvdata],0
        pop     es
Ltemp001:
	jmp	short phx
ph20:
; Handle Get message
	les	di,lpsysmsg
	mov	bx,[si].ieventCur
	shl	bx,1
	shl	bx,1
	errnz	<(SIZE EVENT)-4>
	mov	ax,[si+rgevent][bx+wmEv]
	errnz	<smMessage>
	stosw
	mov	ax,[si+rgevent][bx+vkEv]
	errnz	<smParamL-2>
	stosw
	mov	ax,1
	errnz	<smParamH-4>
	stosw
	xor	ax,ax
	errnz	<smTime-6>
	stosw
	stosw
ph30:
	pop	ds	
ifdef HYBRID
	pop	ax
	push	ds
	push	ss
	pop	ds
	push	ax
	cCall	GlobalUnlockPR;<es:[hevtHeadDrvdata]> args already pushed
	pop	ds	
else ;!HYBRID
	cCall	GlobalUnlock;<es:[hevtHeadDrvdata]> args already pushed
endif ;!HYBRID
phx:
	xor	ax,ax
	xor	dx,dx
phx1:
cEnd
sEnd	eldden
	end
