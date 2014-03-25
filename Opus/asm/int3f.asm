	include w2.inc
	include noxport.inc
	include consts.inc
	include structs.inc
	include newexe.inc

createSeg	_INT3F,int3f,byte,public,CODE

;
; struct OFSTRUCT definition
;
cBytesOfstruct		equ  [byte ptr 00000h]
fFixedDiskOfstruct	equ  [byte ptr 00001h]
nErrCodeOfstruct	equ  [word ptr 00002h]
reservedOfstruct	equ  [byte ptr 00004h]
szPathNameOfstruct	equ  [byte ptr 00008h]
cbOfstruct		equ  00080h

;
; struct MAPDEF definition
;
map_ptrMapdef		equ  [word ptr 00000h]
lsaMapdef		equ  [word ptr 00002h]
pgm_entMapdef		equ  [word ptr 00004h]
abs_cntMapdef		equ  [word ptr 00006h]
abs_ptrMapdef		equ  [word ptr 00008h]
seg_cntMapdef		equ  [word ptr 0000Ah]
seg_ptrMapdef		equ  [word ptr 0000Ch]
nam_maxMapdef		equ  [byte ptr 0000Eh]
nam_lenMapdef		equ  [byte ptr 0000Fh]
cbMapdef		equ  00010h

;
; struct SEGDEF definition
;
nxt_segSegdef		equ  [word ptr 00000h]
sym_cntSegdef		equ  [word ptr 00002h]
sym_ptrSegdef		equ  [word ptr 00004h]
seg_lsaSegdef		equ  [word ptr 00006h]
seg_in0Segdef		equ  [word ptr 00008h]
seg_in1Segdef		equ  [word ptr 0000Ah]
seg_in2Segdef		equ  [word ptr 0000Ch]
seg_in3Segdef		equ  [word ptr 0000Eh]
seg_linSegdef		equ  [word ptr 00010h]
seg_lddSegdef		equ  [byte ptr 00012h]
seg_cinSegdef		equ  [byte ptr 00013h]
nam_lenSegdef		equ  [byte ptr 00014h]
cbSegdef		equ  00015h

;
; struct SYMDEF definition
;
sym_valSymdef		equ  [word ptr 00000h]
nam_lenSymdef		equ  [byte ptr 00002h]
cbSymdef		equ  00003h

SEGI		STRUC	;* result of GetCodeInfo
sectorSegi	DW  ?   ; logical sector number in file of start of segment
cbFileSegi	DW  ?   ; number bytes in file
flagsSegi	DW  ?   ; segment flags
cbRamSegi	DW  ?   ; minimum number bytes to allocate for segment
hSegi		DW  ?   ; Handle to segment (0 if not loaded)
cbAlignSegi	DW  ?   ; Alignment shift count for segment data
SEGI		ENDS

PCODE_HEADER    struc
ph_code         db      3 dup(?)
ph_enMacReg     db      ?
ph_snReg        dw      ?
ph_rReg         dw      ?
ph_mpenbpc      dw      ?
PCODE_HEADER    ends

chLF		equ	00Ah
chCR		equ	00Dh
ichMaxFile	equ	67

; EXPORTED LABELS

; EXTERNAL FUNCTIONS

externFP	<OpenFile>
externFP	<GlobalAlloc>
externFP	<GlobalFree>
externFP	<GlobalHandle>
externFP	<GetCodeInfo>


; EXTERNAL DATA

sBegin  data

globalW hInt3FCode,0
externW $q_mpsnpfn
externW mptlbxpfn
externW Result0
externW Result2
externW Result4

sEnd    data


; CODE SEGMENT _INT3F

sBegin	int3f
	assumes cs,int3f
	assumes ds,dgroup
	assume es:nothing
	assumes ss,dgroup


;-------------------------------------------------------------------------
;	Int3FHandler( )
;-------------------------------------------------------------------------
;/* I N T  3 F	H A N D L E R */
; %%Function:Int3FHandler %%Owner:BRADV
PUBLIC	Int3FHandler
Int3FHandler:
	mov	wptr (cs:[spInt3Fhandler]),sp
	pushf
	push	wptr (cs:Int3FAddress+2)
	push	wptr (cs:Int3FAddress)
	sti

;	Save all registers
;	NOTE: there is no point in saving es because swapping due to
;	calls by Int3FHandler may invalidate it anyway.  But this should
;	be no problem since the caller may not assume that es is valid
;	after a far call.  But I will save it anyway because it looks like
;	the windows int 3F handler does.
	push	ax
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	push	ds
	push	es

	cld
	mov	bx,wptr (cs:[spInt3Fhandler])
	les	si,ss:[bx+6]
	mov	ax,wptr (es:[si])
	mov	wptr (cs:[wCallersCode]),ax
	les	si,ss:[bx]
	mov	al,bptr (es:[si])
	mov	bptr (cs:[fNotReturnThunk]),al
	cCall	LN_DumpSwapInfo,<>
	mov	bx,wptr (cs:[spInt3Fhandler])
	les	si,ss:[bx+6]
	mov	ax,wptr (es:[si])
	cmp	wptr (cs:[wCallersCode]),ax
	je	I3FH02
	int	3
	db	'BV1'
I3FH02:
	les	si,ss:[bx]
			    ;point to the code after the interrupt
	mov	ax,es:[si-2]
			    ;get the code before the return
	sub	ax,03FCDh   ;is it still an int 3F?
	je	I3FH03
	sub	wptr (ss:[bx]),2
I3FH03:
	push	ax
;	let's get outta this interrupt
	pop	ax
	or	ax,ax
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	je	I3FH04
	add	sp,6	    ;forget the windows int 3F handler,
			    ;the int 3f was removed when we weren't looking.
I3FH04:
	iret

; End of Int3FHandler


CSVariables:
Int3FAddress:	  dd	 0
snMac:		  dw	 0
tlbxMac:	  dw	 0
Off_hExeHead:	  dw	 0
fNotReturnThunk:  db	 0
bTraceFlags:	  db	 0
maskfPcallLocal   equ	 1
maskfPcodeLocal   equ	 2
maskfIretAddLocal equ	 4
spInt3Fhandler:   dw	 0
wCallersCode:	  dw	 0
pRetCurrent:	  dw	 0
RetSeg: 	  dw	 0
RetOffset:	  dw	 0
iSegCaller:	  dw	 0
pBuff:		  dw	 0	;always points into pBuff
BuffStart:	  db	 80 dup (0)
chOutBuff:	  db	 0
hDbgScreen:	  dw	 0
segiSwap:	  db	 SIZE SEGI DUP (0)
MapDefSwap:	  db	 cbMapdef DUP (0)
SegDefSwap:	  db	 cbSegdef DUP (0)
SymDefSwap:	  db	 cbSymdef DUP (0)
prev_SymDef:	  db	 cbSymdef DUP (0)
szSegName:	  db	 128 DUP (0)
pDiskBuff:	  dw	 0
cbDiskBuff:	  dw	 0
LO_posCur:	  dw	 0
HI_posCur:	  dw	 0
DiskBuff:	  db	 1024 DUP (0)
fBufferFlags:	  dw	 0
maskfUsedForStructLocal   equ	 1
maskfUsedStraightLocal	  equ	 2
szSymFile:	  db	 66 dup (0)
cbSzSymFile	  equ	 $ - offset szSymFile
ofStruct:	  db	 cbOfstruct DUP (0)
szFileCur:	  dw	 0
cbCSVariables	  equ	 $ - CSVariables
doshndCur:	  dw	 -1


;* * * String pool

stOPUS: 	  db	 4,'OPUS'
szKernelSym:	  db	 'kernel.sym',0
szOpusSym:	  db	 'opus.sym',0
szPcodeMap:	  db	 'pcodemap.txt',0
szToolbox:	  db	 'toolbox.h',0
szInt3F:	DB	"INT 3F| ", 0
szCRLF:         DB      0dh, 0ah, 0
szReturnThunk:	DB	"Return Thunk Encountered", 0
szPCODE:	DB	'PCODE!',0
szNoSegNoOff:	DB	'?'
szNoOffset:	DB	':?',0
szNoSegment:	DB	'?:',0
szLoaded:	DB	" loaded ", 0
szMsDosExe:	DB	'msdos.exe',0
szHEXEHEAD:	DB	'HEXEHEAD',0
rgchTLBX:	DB	'TLBX('
cbTLBX		equ	$ - offset rgchTLBX
rgchtlbxMac:	DB	'tlbxMac '
cbtlbxMac	equ	$ - offset rgchtlbxMac
szCom1: 	DB	'com1',0
szCALL0:	DB	'CALL0',0
cbCALL0 	equ	$ - offset szCALL0
rgchRetNative:	DB	'RetNative'
cbRetNative	equ	$ - offset rgchRetNative
rgchRetToolbox: DB	'RetToolbox'
cbRetToolbox	equ	$ - offset rgchRetToolbox
szDotSym:	DB	'.sym',0
cbDotSym	equ	$ - offset szDotSym


	assumes ds,int3f


;-------------------------------------------------------------------------
;	LN_DumpSwapInfo()
;-------------------------------------------------------------------------
LN_DumpSwapInfo:

	push	cs
        pop     ds
	push	cs
	pop	es			; maintain assumption es == ds == cs

; init locals
	call	LN_HOpenDbgScreen	; enable printing to the debug screen?
	mov	wptr ([hDbgScreen]),ax	; save handle to debug screen
        cmp     ax,-1                   ; legal handle?
	jne	LN_DSI01		; yes - skip
	ret

LN_DSI01:
	mov	bptr ([bTraceFlags]),0

	mov	si,offset szInt3F
	call	LN_PrintSz		; output "INT 3F - "
	cmp	bptr ([fNotReturnThunk]),fFalse
	jne	LN_DSI02
	mov	si,offset szReturnThunk
	call	LN_PrintSz		; output "Return Thunk Encountered"
	jmp	LN_DSI11		; we're done.

LN_DSI02:
	mov	bx,wptr ([spInt3FHandler])
	add	bx,4
	mov	wptr ([pRetCurrent]),bx

LN_DSI03:
	mov	ax,offset [BuffStart]	; get address of start of strings
	mov	wptr ([pBuff]),ax	; and save it

	mov	bx,wptr ([pRetCurrent])
	mov	ax,ss:[bx+2]		; get offset part of ret addr
	mov	dx,ss:[bx+4]		; get segment part
	mov	wptr ([RetOffset]),ax	; save address for later...
	mov	wptr ([RetSeg]),dx
					; are we looking at the iret address?
	test	bptr ([bTraceFlags]),maskfIretAddLocal
	jz	LN_DSI036		; no - try to print name
					; were we called from the interpreter?
	test	bptr ([bTraceFlags]),maskfPcodeLocal
	jz	LN_DSI034		; no - see if the toolbox was called
					; was it pcode calling pcode?
	test	bptr ([bTraceFlags]),maskfPcallLocal
	jz	LN_DSI034		; no - must have been a toolbox call.
	push	ss:[Result4]
	call	LN_FPrintPcodeAddress	; yes - print the address
	jmp	short LN_DSI042

LN_DSI034:
	call	LN_FPrintToolboxName	; print toolbox name if possible
	jmp	short LN_DSI042

LN_DSI036:
	test	bptr ([bTraceFlags]),maskfPcodeLocal ; is this a real bp?
	jz	LN_DSI04		; yes - far frame

	mov	ax,0FFFFh
	push	ax
	call	LN_FPrintPcodeAddress
	jmp	short LN_DSI042

LN_DSI04:
	push	wptr ([RetSeg]) 	; pass the segment to look up
	call	LN_FFindSeg		; find the segment
LN_DSI042:
        or      ax,ax                   ; successful?
	jnz	LN_DSI046		; yes - skip
	push	ax
	call	LN_PrintSegOff		; no - print segment:offset
	pop	ax
LN_DSI046:
					; did we call FFindSeg?
	test	bptr ([bTraceFlags]),maskfIretAddLocal+maskfPcodeLocal
	jne	LN_DSI047		; no - move on
	or	ax,ax			; were we successful?
	jne	LN_DSI05		; yes - continue processing
LN_DSI047:
	jmp	LN_DSI10

LN_DSI05:
	mov	dx,offset [szSymFile]
;	LN_FOpenFcb takes a pointer to an sz in [ds:dx] and returns
;	zero if opening the file failed, and not zero if it succeeded.
;	The disk transfer address is also set to [DiskBuff+512]
;	ax is 0 upon return iff opening failed, no other registers altered.
	call	LN_FOpenFcb
	je	LN_DSI057
	push	wptr ([iSegCaller])
	cCall	LN_FGetSegSymb,<>
        or      ax,ax                   ; successful?
	je	LN_DSI055		; no - close file and move on
	mov	di,offset szSegName
	mov	si,di
	mov	al,0
	mov	cx,0FFFFh
	repne	scasb			; compute length of sz in szSegName
	mov	cx,di
	sub	cx,si
	mov	di,wptr ([pBuff])	; transfer sz into Buff
	rep	movsb
	mov	byte ptr [di-1],':'	; save seperator in string
	mov	wptr ([pBuff]),di

	push	wptr ([RetOffset])	; pass the offset to lookup
	push	wptr ([pBuff])		; pass the place to put the offset symbol
	call	LN_FGetSymbol		; get the symbol
LN_DSI055:
;	LN_CloseFcb closes doshndCur.  No registers are altered.
	call  LN_CloseFcb
	or	ax,ax			; successful?
	jnz	LN_DSI06		; yes - see if we have a pcode frame
LN_DSI057:
	mov	si,offset szNoSegNoOff
	call	LN_PrintSegOff		; no - just print segment:offset
	jmp	short LN_DSI10

LN_DSI06:

;*	* look for interpreter calls

;*	* possibly "RetNative?"
	mov	si,wptr ([pBuff])	    ; get the pointer to the buffer
	mov	di,offset rgchRetNative
	mov	cx,cbRetNative
	repe	cmpsb
	je	LN_DSI07

;*      * possibly "RetToolbox?"
	mov	si,wptr ([pBuff])	    ; get the pointer to the buffer
	mov	di,offset rgchRetToolbox
	mov	cx,cbRetToolbox
	repe	cmpsb
	jne	LN_DSI075

LN_DSI07:
	cmp	bptr [si],'0'
	jb	LN_DSI075
	cmp	bptr [si],'3'
	ja	LN_DSI075
	cmp	bptr [si+1],0
	je	LN_DSI08

LN_DSI075:
;*	* possibly "CALL0"
	mov	si,wptr ([pBuff])	    ; get the pointer to the buffer
	mov	di,offset szCALL0
	mov	cx,cbCALL0
	repe	cmpsb
	jne	LN_DSI09
	or	bptr ([bTraceFlags]),maskfPcallLocal+maskfPcodeLocal
	mov	ax,ss:[Result2]
	mov	dx,ss:[Result0]
	mov	wptr ([RetOffset]),ax	; save address for later...
	mov	wptr ([RetSeg]),dx
	mov	ax,offset [BuffStart]	; get address of start of strings
	mov	wptr ([pBuff]),ax	; and save it
	mov	ax,0FFFFh
	push	ax
	call	LN_FPrintPcodeAddress
	jmp	LN_DSI042

;*      * Start of Pcode frame
LN_DSI08:
	or	bptr ([bTraceFlags]),maskfPcodeLocal
	mov	wptr ([pRetCurrent]),bp
	jmp	LN_DSI03		; skip to pcode frame...

LN_DSI09:
	call	LN_PrintNativeAddress

LN_DSI10:
	test	bptr ([bTraceFlags]),maskfIretAddLocal
	jne	LN_DSI11
	mov	si,offset szLoaded
	call	LN_PrintSz		; output " loaded "
	mov	bx,wptr ([spInt3FHandler])
	sub	bx,2
	mov	wptr ([pRetCurrent]),bx ; point to iret return address
	or	bptr ([bTraceFlags]),maskfIretAddLocal
	jmp	LN_DSI03

LN_DSI11:
        mov     si,offset szCRLF
        call    LN_PrintSz              ; output <CR><LF>
	mov	bx,offset [hDbgScreen]
	call	LN_CloseOutFile 	; close the debug screen

        ret


;-------------------------------------------------------------------------
;	LN_PrintNativeAddress()
;-------------------------------------------------------------------------
LN_PrintNativeAddress:
	mov	di,offset [szSymFile]	; point to sym file name
	mov	cx,0FFFFh
	mov	al,0
	repne	scasb
	mov	cx,di
	mov	di,offset [szSymFile]	; point to sym file name
	sub	cx,di
	mov	al,'.'			; find file extension
	repne	scasb
	dec	di
	mov	bptr [di],0		; end string here
	push	di			; save pointer to string end
	mov	si,offset [szSymFile]	; point to sym file name
	call	LN_PrintSz		; print the map name
	pop	di			; point to where we ended the string
	mov	bptr [di],'.'		; restore the extension
        mov     al,'!'                  ; seperator between map name and seg name
	call	LN_Dout
	mov	si,offset [BuffStart]	; point to seg and symbol names
	call	LN_PrintSz		; print the map name
	mov	cx,wptr ([RetOffset])	; get offset
	sub	cx,wptr ([SymDefSwap.sym_valSymdef]) ; calc offset from start of proc
	jcxz	LN_DSI10		; skip if no offset
        mov     al,'+'                  ; seperator between symbol name and offset
	call	LN_Dout
        mov     ax,cx                   ; output the offset
	call	LN_DoutHex
	ret


;-------------------------------------------------------------------------
;	LN_FPrintPcodeAddress( wSnProc )
;-------------------------------------------------------------------------
; %%Function:LN_FPrintPcodeAddress %%Owner:BRADV
cProc	LN_FPrintPcodeAddress,<NEAR>,<>
	ParmW	wSnProc

	LocalW	fResult

cBegin
	mov	[fResult],0FFFFh
	mov	si,offset szPCODE
	call	LN_PrintSz
	mov	ax,[wSnProc]
	cmp	ax,0FFFFh
	je	LN_PPA01
	mov	al,ah
	xor	ah,ah
	mov	wptr ([RetSeg]),ax
LN_PPA01:
	push	wptr ([RetSeg]) 	; pass sn
	call	LN_FGetPcodeSegName
        or      ax,ax                   ; seg name found?
	jnz	LN_PPA02		; yes - continue with true name
	mov	al,'?'
	call	LN_Dout
	mov	al,':'
	call	LN_Dout
	jmp	short LN_PPA08		; no - skip
LN_PPA02:

	mov	si,offset [szSegName]
	call	LN_PrintSz
	mov	al,':'
	call	LN_Dout

	mov	dx,[wSnProc]
	inc	dx
	jne	LN_PPA05
	push	wptr ([RetSeg]) 	; get sn
	call	LN_GetPsOfSn		; get physical address	of pcode seg
        or      ax,ax                   ; swapped out?             
	jz	LN_PPA08		; yes - exit

        mov     es,ax                   ; in memory - put seg in es
        mov     bx,offset ph_mpenbpc    ; point to mpenbpc table
        xor     cx,cx
        mov     cl,es:[ph_enMacReg]     ; get count of procs
        mov     dx,0                    ; init proc counter

LN_PPA03:
        mov     ax,es:[bx]              ; get the next entry point
	cmp	ax,wptr ([RetOffset])	; is this one past the one we want?
	ja	LN_PPA04		; yes - skip
        inc     bx                      ; point to next entry
        inc     bx
        inc     dx                      ; bump proc counter
	loop	LN_PPA03		; and loop

LN_PPA04:
        mov     bx,es:[bx-2]            ; save start of the proc
	push	cs
	pop	es
LN_PPA05:
        push    bx
        dec     dx                      ; set proc number back
	mov	dh,bptr ([RetSeg])	; get seg in dh, proc # in dl
        push    dx
	mov	ax,offset szOpusSym
	push	ax
	push	wptr ([pBuff])		; point to buffer
	call	LN_FGetConstSymb
        pop     dx                      ; get back start of proc offset
        or      ax,ax                   ; OK?
	jnz	LN_PPA09		; no - exit
LN_PPA08:
	mov	[fResult],00000h
	mov	al,'?'
	call	LN_Dout
	jmp	short LN_PPA10

LN_PPA09:
	mov	si,offset [BuffStart]	; point to seg and symbol names
	call	LN_PrintSz		; print the map name

	cmp	[wSnProc],0FFFFh
	jne	LN_PPA10
	mov	cx,wptr ([RetOffset])	; get offset
        sub     cx,dx                   ; calc offset from start of proc
        dec     cx
        dec     cx
	jcxz	LN_PPA10		; skip if no offset

        mov     al,'+'                  ; seperator between symbol name and offset
	call	LN_Dout

        mov     ax,cx                   ; output the offset
	call	LN_DoutHex

LN_PPA10:

	mov	ax,[fResult]
	cEnd


;-------------------------------------------------------------------------
;	LN_FPrintToolboxName()
;-------------------------------------------------------------------------
; %%Function:LN_FPrintToolboxName %%Owner:BRADV
cProc	LN_FPrintToolboxName,<NEAR>,<>

	LocalW	wToolboxNumber
	LocalW	fResult

cBegin
	mov	[fResult],00000h
	mov   dx,offset szToolbox
;	LN_FOpenFcb takes a pointer to an sz in [ds:dx] and returns
;	zero if opening the file failed, and not zero if it succeeded.
;	The disk transfer address is also set to [DiskBuff+512]
;	ax is 0 upon return iff opening failed, no other registers altered.
	call  LN_FOpenFcb
	jne   Ltemp002
	jmp   LN_PTN13
Ltemp002:
	mov	dx,wptr ([RetSeg])
	mov	bx,wptr ([RetOffset])
	sub	bx,7
	;Assume that if we got here (we found a call from the pcode
	;interpreter to the int3f), then ss must be the interpreter ss.
	mov	si,offset DGROUP:mptlbxpfn
	mov	di,si
	mov	cx,wptr ([tlbxMac])
	push	ss
	pop	ds
LN_PTN01:
	lodsw
	cmp	ax,bx
	lodsw
	jne	LN_PTN02
	cmp	ax,dx
LN_PTN02:
	loopnz	LN_PTN01
	push	cs
	pop	ds
	jne	Ltemp003
	sub	si,di
	shr	si,1
	shr	si,1
	dec	si
	mov	[wToolboxNumber],si
;	LN_FReadFcb takes a pointer to an sz in [ds:doshndCur] and returns
;	zero if reading the file failed, and not zero if it succeeded.
;	ax is 0 upon return iff reading failed, no other registers altered.
	call  LN_FReadFcb
	jne   Ltemp004
Ltemp003:
	mov   si,wptr ([pBuff])
	mov   wptr [si],(256 * 0) + '?'
	jmp   LN_PTN12
Ltemp004:
	mov   dx,512
LN_PTN04:
	or    dx,dx
	jle   Ltemp003
	push  dx
;	LN_FReadFcb takes a pointer to an sz in [ds:doshndCur] and returns
;	zero if reading the file failed, and not zero if it succeeded.
;	ax is 0 upon return iff reading failed, no other registers altered.
	call  LN_FReadFcb
	pop   cx
	mov   dx,0
	je    LN_PTN05
	mov   dx,512
LN_PTN05:
	mov   di,offset [DiskBuff]
LN_PTN06:
	mov   al,'#'	    ;look for the beginning of '#define'
	repne scasb
	jne   LN_PTN04
	push  cx
	push  dx
	push  di
	dec   di
	inc   cx
	add   cx,dx
	sub   cx,cbTLBX + 5 ; after 'TLBX(' must not have junk
	jl    LN_PTN10
LN_PTN07:
	mov   al,bptr ([rgchTLBX])
	repne scasb
	jne   LN_PTN10
	mov   dx,cx
	mov   bx,di
	dec   di
	mov   cx,cbTLBX
	mov   si,offset rgchTLBX
	repe  cmpsb
	mov   cx,dx
	xchg  bx,di
	jne   LN_PTN07
			    ; found '#...TLBX(' and have whole thing in buffer
	xor   ax,ax
	cwd
	mov   si,bx
LN_PTN08:
	lodsb
	sub   al,'0'
	cmp   al,10
	jae   LN_PTN09
	shl   dx,1
	mov   cx,dx
	shl   dx,1
	shl   dx,1
	add   dx,cx
	add   dx,ax
	xor   cx,cx
	jmp   short LN_PTN08
LN_PTN09:
	cmp   dx,[wToolboxNumber]
	je    LN_PTN11
LN_PTN10:
	pop   di
	pop   dx
	pop   cx
	jmp   short LN_PTN06

LN_PTN11:
			    ; found '#...TLBX(<correct value>'
			    ; and have the whole thing in buffer
	pop   si	    ; beginning of '#...TLBX(<correct value>'
	pop   dx
	pop   cx
	mov   cx,0FFFFh
	mov   al,'#'
	std
	repne scasb	    ; look for last '#' before 'TLBX('
	cld
	inc   di
	mov   al,' '	    ; look for space after '#define'
	repne scasb
	mov   si,di	    ; pointer to beginning of toolbox name
	mov   al,'('
	repne scasb
	mov   bptr [di-1],0
	mov   [fResult],0FFFFh
LN_PTN12:
	push  si
	mov   si,offset szNoSegment
	call  LN_PrintSz
	pop   si
	call  LN_PrintSz

;	LN_CloseFcb closes doshndCur.  No registers are altered.
	call  LN_CloseFcb
	mov   ax,[fResult]
LN_PTN13:

	cEnd


;-------------------------------------------------------------------------
;	LN_PrintSegOff()
;-------------------------------------------------------------------------
;
;  Print return address in form:
;
;       <segment>:<offset>
;
LN_PrintSegOff:
	mov	al,'('
	call	LN_Dout
	mov	ax,wptr ([RetSeg])
	call	LN_DoutHex
	mov	al,':'
	call	LN_Dout
	mov	ax,wptr ([RetOffset])
	call	LN_DoutHex
	mov	al,')'
	call	LN_Dout
        ret


;-------------------------------------------------------------------------
;	LN_PrintSz()
;-------------------------------------------------------------------------
;*
;* PrintSz : DS:SI = "sz" string
;*
;*      print string

LN_PrintSz:
        push    ax                      ; save regs
	jmp	short LN_PS04		; loop
LN_PS02:
	call	LN_Dout 		; output the char
LN_PS04:
	lodsb				; get a char
        cmp     al,0                    ; end of string?
	jnz	LN_PS02 		; yes - exit
        pop     ax
        ret


;-------------------------------------------------------------------------
;	LN_DoutHex()
;-------------------------------------------------------------------------
; print out a 16 bit unsigned in 4 hex digit format
LN_DoutHex:
	push	cx			; save environment
	mov	cx,00404h
LN_DH01:
	rol	ax,cl
	push	ax
	and	al,00Fh
        add     al,'0'
        cmp     al,'9'
	jbe	LN_DH02
        add     al,'A' - '0' - 10
LN_DH02:
	call	LN_Dout
	pop	ax
	dec	ch
	ja	LN_DH01
        pop     cx                      ; restore environment
        ret


;-------------------------------------------------------------------------
;	LN_Dout()
;-------------------------------------------------------------------------
; print out a char
LN_Dout:
        push    bx                      ; save environment
	mov	bx,offset [hDbgScreen]	; get handle of debug screen to print to
	call	LN_Doutfile
        pop     bx                      ; restore environment
        ret


;-------------------------------------------------------------------------
;	LN_DoutFile()
;-------------------------------------------------------------------------
; print out a char to the opened file
LN_Doutfile:
	cmp	bx,0ffffh		; exit if file not open
	je	LN_DF03
        push    ax                      ; save environment
        push    cx
        push    dx

	push	bx			; save file handle
	mov	bx,offset [chOutBuff]
        mov     [bx],al
	mov	dx,bx
	pop	bx			; restore file handle
	push	bx			; save file handle
	mov	bx,[bx]
	mov	cx,1
;    mov     bx,[doshnd]
;    lds     dx,[lpchBuffer]
;    mov     cx,[bytes]
	mov	ah,040h
	int	21h
	jnc	LN_DF01
	neg	ax		; error: return the negative of the error code
LN_DF01:
	or	ax,ax
	jnz	LN_DF02
	pop	bx			; restore file handle
	push	bx			; save file handle
	call	LN_CloseOutFile
LN_DF02:
	pop	bx			; restore file handle

	pop	dx			; restore environment
        pop     cx
        pop     ax

LN_DF03:
        ret


;-------------------------------------------------------------------------
;	LN_CloseOutFile()
;-------------------------------------------------------------------------
; close the output file
LN_CloseOutFile:
        cmp     [bx],0ffffh
	je	LN_COF02

        push    ax                      ; save environment
        push    cx
        push    dx

        push    bx
	mov	bx,[bx]
	mov	ah,03Eh   ;Close file

	int	21h
	mov	ax,00000h
	jc	LN_COF01		; error, leave a zero in ax
	inc	ax
LN_COF01:
        pop     bx
        mov     [bx],0ffffh

        pop     dx                      ; restore environment
        pop     cx
        pop     ax

LN_COF02:
        ret


LGlobalAlloc:
	cCall GlobalAlloc,<>

;-------------------------------------------------------------------------
;	LN_FFindSeg( CSvalue )
;-------------------------------------------------------------------------
; %%Function:LN_FFindSeg %%Owner:BRADV
cProc	LN_FFindSeg,<NEAR>,<>
	ParmW	CSvalue

	LocalW	hExe

cBegin

	mov   bx,wptr ([Off_hExeHead])
	mov   es,wptr ([LGlobalAlloc + 3])
	mov   ax,wptr (es:[bx])
	push  cs
	pop   es
	mov   [hExe],ax
	jmp   short LN_FFS02
LN_FFS01:
;#define NE_PNEXTEXE(x)  (WORD)(x).ne_cbenttab
	mov   es,[hExe]
	mov   ax,es:[ne_cbenttab]
	mov   [hExe],ax
LN_FFS02:
	push  cs
	pop   es
	xor   ax,ax
	cmp   [hExe],0
	je    LN_FFS04
	mov   es,[hExe]
	test  es:[ne_flags],NENOTP	;can look at segments if library
	jne   LN_FFS03
	mov   di,es:[ne_restab]
	mov   si,offset stOPUS
	mov   cl,[si]
	xor   ch,ch
	inc   cx
	repe  cmpsb			;or if it is OPUS
	jne   LN_FFS01
LN_FFS03:
	mov   si,es:[ne_segtab]
	mov   wptr ([iSegCaller]),00001h
	jmp   short LN_FFS06
LN_FFS04:
	jmp   short LN_FFS10
LN_FFS05:
	inc   wptr ([iSegCaller])
	add   si,SIZE New_seg1
LN_FFS06:
	mov   ax,wptr ([iSegCaller])
	mov   es,[hExe]
	cmp   ax,es:[ne_cseg]
	ja    LN_FFS01
	push  es:[si.ns_handle]
;#define MyLock(ps)	 HIWORD(GlobalHandle(ps))
	cCall GlobalHandle,<>
	push  cs
	pop   ds
	cmp   dx,[CSvalue]
	jne   LN_FFS05
	mov   es,[hExe]
	mov   si,es:[ne_restab]
	mov   ds,[hExe]
	lodsb
	xor   ah,ah
	mov   cx,ax
	mov   di,offset [szSymFile]
	push  cs
	pop   es
LN_FFS07:
	lodsb
	cmp   al,'.'
	je    LN_FFS08
	stosb
	loop  LN_FFS07
LN_FFS08:
	push  cs
	pop   ds
	mov   si,offset szDotSym
	mov   cx,cbDotSym
	rep   movsb
	cmp   di,offset [szSymFile] + cbSzSymFile
	jb    LN_FFS09
	int   3
	db	'BV3'
LN_FFS09:
	mov   ax,0FFFFh

LN_FFS10:

	cEnd


;-------------------------------------------------------------------------
;	LN_FGetSegSymb( Seg )
;-------------------------------------------------------------------------
; %%Function:LN_FGetSegSymb %%Owner:BRADV
cProc	LN_FGetSegSymb,<NEAR>,<>
	ParmW	SegArg

	LocalW	iSeg

cBegin

	mov   di,offset MapDefSwap
	mov   cx,cbMapdef
;	LN_ReadRgch reads cx bytes into [ds:di] and returns not zero if cx
;	bytes have been read.
;	only ax and di are altered.
	call  LN_ReadRgch
	jz    LN_FGSS04
	mov   ax,[SegArg]
	dec   ax
	cmp   ax,wptr ([MapDefSwap.seg_cntMapdef])
	mov   ax,0
	jae   LN_FGSS04
	mov   [iSeg],00001h
	mov   ax,wptr ([MapDefSwap.seg_ptrMapdef])
	mov   wptr ([SegDefSwap.nxt_segSegdef]),ax
LN_FGSS01:
	mov   ax,[iSeg]
	cmp   ax,[SegArg]
	ja    LN_FGSS03
	cmp   wptr ([SegDefSwap.nxt_segSegdef]),0
	mov   ax,0
	je    LN_FGSS04
	mov   bx,wptr ([SegDefSwap.nxt_segSegdef])
	mov   cl,4
	rol   bx,cl
	mov   dx,0000Fh
	and   dx,bx
	and   bl,0F0h
	mov   cl,0
;    mov     dx,[SEG_dwSeekpos]
;    mov     bx,[OFF_dwSeekpos]
;    mov     cl,[bSeekfrom]
	cCall LN_DwSeekDw,<>
	jz    LN_FGSS04
	mov   di,offset SegDefSwap
	mov   cx,cbSegdef
;	LN_ReadRgch reads cx bytes into [ds:di] and returns not zero if cx
;	bytes have been read.
;	only ax and di are altered.
	call  LN_ReadRgch
	jz    LN_FGSS04
	inc   [iSeg]
	jmp   short LN_FGSS01
LN_FGSS03:
	mov   di,offset [szSegName]
	mov   cl,bptr ([SegdefSwap.nam_lenSegdef])
	xor   ch,ch
;	LN_ReadSz reads cx bytes into [ds:di], zero terminates the sz,
;	and returns not zero if cx bytes have been read.
;	only ax and di are altered.
	call  LN_ReadSz
LN_FGSS04:

	cEnd


;-------------------------------------------------------------------------
;	LN_FGetSymbol(Offset, szSymbol)
;-------------------------------------------------------------------------
; %%Function:LN_FGetSymbol %%Owner:BRADV
cProc	LN_FGetSymbol,<NEAR>,<>
	ParmW	OffsetArg
	ParmW	szSymbol

	LocalW	iSym

cBegin

	mov   [iSym],00000h
	mov   di,offset SymDefSwap
	mov   cx,cbSymdef
;	LN_ReadRgch reads cx bytes into [ds:di] and returns not zero if cx
;	bytes have been read.
;	only ax and di are altered.
	call  LN_ReadRgch
	jz    LN_FGS03
	mov   di,[szSymbol]
	mov   cl,bptr ([SymDefSwap.nam_lenSymdef])
	xor   ch,ch
;	LN_ReadSz reads cx bytes into [ds:di], zero terminates the sz,
;	and returns not zero if cx bytes have been read.
;	only ax and di are altered.
	call  LN_ReadSz
	jz    LN_FGS03
	mov   cl,bptr ([SymDefSwap]+2)
	mov   ax,wptr ([SymDefSwap])
	mov   wptr ([prev_Symdef]),AX
	mov   bptr ([prev_Symdef]+2),cl
	mov   [iSym],00001h
LN_FGS01:
	mov   ax,[iSym]
	cmp   ax,wptr ([SegDefSwap.sym_cntSegdef])
	jg    LN_FGS02
	mov   di,offset SymdefSwap
	mov   cx,cbSymdef
;	LN_ReadRgch reads cx bytes into [ds:di] and returns not zero if cx
;	bytes have been read.
;	only ax and di are altered.
	call  LN_ReadRgch
	jz    LN_FGS03
	mov   ax,wptr ([SymDefSwap.sym_valSymdef])
	cmp   ax,[OffsetArg]
	ja    LN_FGS02
	mov   di,[szSymbol]
	mov   cl,bptr ([SymdefSwap.nam_lenSymdef])
	xor   ch,ch
;	LN_ReadSz reads cx bytes into [ds:di], zero terminates the sz,
;	and returns not zero if cx bytes have been read.
;	only ax and di are altered.
	call  LN_ReadSz
	jz    LN_FGS03
	mov   cl,bptr ([SymDefSwap]+2)
	mov   ax,wptr ([SymDefSwap])
	mov   wptr ([prev_Symdef]),ax
	mov   bptr ([prev_Symdef]+2),cl
	inc   [iSym]
	jmp   short LN_FGS01
LN_FGS02:
	mov   cl,bptr ([prev_Symdef]+2)
	mov   ax,wptr ([prev_Symdef])
	mov   wptr ([SymDefSwap]),ax
	mov   bptr ([SymDefSwap]+2),cl
	mov   ax,0FFFFh
LN_FGS03:

	cEnd


;-------------------------------------------------------------------------
;	LN_GetPsOfSn()
;-------------------------------------------------------------------------
;*	entry : sn = Pcode segment #
;*	* find physical segment of Pcode segment (if resident)
;*	exit : AX = ps (or 0 if not resident)

; %%Function:LN_GetPsOfSn %%Owner:BRADV
cProc	LN_GetPsOfSn,<NEAR>,<>

	ParmW  sn

cBegin

	mov	bx,sn
	shl	bx,1			;* Make into word pointer
	shl	bx,1			;* DWORD ptr
	add	bx,OFFSET DGROUP:$q_mpsnpfn	;* DS:BX => table
	;Assume that if we got here (we found a call from the pcode
	;interpreter to the int3f), then ss must be the interpreter ss.
	push	wptr ss:[BX+2]		;* segment
	push	wptr ss:[BX]		;* offset (lpthunk on stack)
	mov	bx,offset [segiSwap]	;* address of buffer
	cCall	GetCodeInfo,<cs,bx>	;* lpthunk, lpsegi
	push	cs
	pop	ds
	push	cs
	pop	es
	or	ax,ax
	jz	LN_GPOS01		;* error - assume non-resident
;*	* good segment info, see if segment resident
	mov	ax,wptr ([segiSwap.hSegi])	;* handle or ps if fixed
	test	ax,1
	jnz	LN_GPOS01		;* ax = ps
	cCall	GlobalHandle,<ax>	;* get handle info
	push	cs
	pop	ds
	push	cs
	pop	es
	mov	ax,dx			;* ps (or 0 if not loaded)
LN_GPOS01:

cEnd


;-------------------------------------------------------------------------
;	LN_FGetPcodeSegName(iPcodeSeg, szSegName)
;-------------------------------------------------------------------------
; %%Function:LN_FGetPcodeSegName %%Owner:BRADV
cProc	LN_FGetPcodeSegName,<NEAR>,<>
	ParmW	iPcodeSeg

	LocalW	hSegFile

cBegin

	mov   dx,offset szPcodeMap
;	LN_FOpenFcb takes a pointer to an sz in [ds:dx] and returns
;	zero if opening the file failed, and not zero if it succeeded.
;	The disk transfer address is also set to [DiskBuff+512]
;	ax is 0 upon return iff opening failed, no other registers altered.
	call  LN_FOpenFcb
	je    LN_FGPSN06
	xor   bx,bx ;iSn
	mov   di,offset [DiskBuff+1024]
	mov   si,di
LN_FGPSN01:
;	LN_FReadFcb takes a pointer to an sz in [ds:doshndCur] and returns
;	zero if reading the file failed, and not zero if it succeeded.
;	ax is 0 upon return iff reading failed, no other registers altered.
	call  LN_FReadFcb
	je    LN_FGPSN05
	mov   cx,512
	sub   si,cx
	sub   di,cx
LN_FGPSN02:
	mov   al,chCR
	repne scasb
	jne   LN_FGPSN01
	cmp   bx,[iPcodeSeg]
	je    LN_FGPSN04
	inc   bx
	mov   si,di
	jcxz  LN_FGPSN01
	jmp   short LN_FGPSN02
LN_FGPSN04:
	inc   si	;move past the chLF
	mov   cx,di
	sub   cx,si
	dec   cx
	mov   di,offset [szSegName]
	rep   movsb
	mov   bptr [di],0
	mov   ax,0FFFFh
LN_FGPSN05:
;	LN_CloseFcb closes doshndCur.  No registers are altered.
	call  LN_CloseFcb
LN_FGPSN06:

	cEnd


;-------------------------------------------------------------------------
;	LN_FGetConstSymb(Const, szFile, szConstName)
;-------------------------------------------------------------------------
; %%Function:LN_FGetConstSymb %%Owner:BRADV
cProc	LN_FGetConstSymb,<NEAR>,<>
	ParmW	Const
	ParmW	szFile
	ParmW	szConstName

	LocalW	iConst

cBegin

	mov   dx,[szFile]
;	LN_FOpenFcb takes a pointer to an sz in [ds:dx] and returns
;	zero if opening the file failed, and not zero if it succeeded.
;	The disk transfer address is also set to [DiskBuff+512]
;	ax is 0 upon return iff opening failed, no other registers altered.
	call  LN_FOpenFcb
	jne   LN_FGCS01
	jmp   LN_FGCS08
LN_FGCS01:
	mov   di,offset MapDefSwap
	mov   cx,cbMapdef
;	LN_ReadRgch reads cx bytes into [ds:di] and returns not zero if cx
;	bytes have been read.
;	only ax and di are altered.
	call  LN_ReadRgch
	jz    LN_FGCS07
	mov   bl,bptr ([MapDefSwap.nam_lenMapdef])
	xor   bh,bh
	add   bx,cbMapdef
	inc   bx
	xor   dx,dx
	mov   cl,0
;    mov     dx,[SEG_dwSeekpos]
;    mov     bx,[OFF_dwSeekpos]
;    mov     cl,[bSeekfrom]
	cCall LN_DwSeekDw,<>
	jz    LN_FGCS07
	mov   [iConst],00000h
LN_FGCS02:
	xor   ax,ax
	mov   cx,[iConst]
	cmp   cx,wptr ([MapDefSwap.abs_cntMapdef])
	jge   LN_FGCS07
	mov   di,offset SymDefSwap
	mov   cx,cbSymdef
;	LN_ReadRgch reads cx bytes into [ds:di] and returns not zero if cx
;	bytes have been read.
;	only ax and di are altered.
	call  LN_ReadRgch
	jz    LN_FGCS07
	mov   ax,wptr ([SymDefSwap.sym_valSymdef])
	cmp   ax,[Const]
	jne   LN_FGCS05
	mov   di,[szConstName]
	mov   cl,bptr ([SymDefSwap.nam_lenSymdef])
	xor   ch,ch
;	LN_ReadSz reads cx bytes into [ds:di], zero terminates the sz,
;	and returns not zero if cx bytes have been read.
;	only ax and di are altered.
	call  LN_ReadSz
	jz    LN_FGCS07
	mov   bx,[szConstName]
	cmp   bptr [bx],'@'
	jne   LN_FGCS04
	mov   cl,bptr ([SymDefSwap.nam_lenSymdef])
	xor   ch,ch
	mov   di,[szConstName]
	lea   si,[di+1]
	rep   movsb
LN_FGCS04:
	mov   ax,0FFFFh
	jmp   short LN_FGCS07
LN_FGCS05:
	mov   bl,bptr ([SymdefSwap.nam_lenSymdef])
	xor   bh,bh
	xor   dx,dx
	mov   cl,001h
;    mov     dx,[SEG_dwSeekpos]
;    mov     bx,[OFF_dwSeekpos]
;    mov     cl,[bSeekfrom]
	cCall LN_DwSeekDw,<>
	jz    LN_FGCS07
LN_FGCS06:
	inc   [iConst]
	jmp   short LN_FGCS02
LN_FGCS07:
;	LN_CloseFcb closes doshndCur.  No registers are altered.
	call  LN_CloseFcb
LN_FGCS08:

	cEnd


;-------------------------------------------------------------------------
;	LN_FOpenFcb()
;-------------------------------------------------------------------------
;	LN_FOpenFcb takes a pointer to an sz in [ds:dx] and returns
;	zero if opening the file failed, and not zero if it succeeded.
;	The disk transfer address is also set to [DiskBuff+512]
;	ax is 0 upon return iff opening failed, no other registers altered.
LN_FOpenFcb:
	cmp   wptr ([doshndCur]),0FFFFh
	je    LN_FOF01
	int   3
	db	'BV4'
LN_FOF01:
	mov   wptr ([szFileCur]),dx
	push  bx
	push  cx
	push  dx

	push  ds
	push  dx	;&sz
	mov   ax,offset ofStruct
	push  ds
	push  ax	;&ofStruct
	xor   ax,ax
	push  ax	;OF_READ
	cCall OpenFile,<>

	push  cs
	pop   ds
	push  cs
	pop   es
	pop   dx
	pop   cx
	pop   bx

	inc   ax
	je    LN_FOF02
	dec   ax
	mov   wptr ([doshndCur]),ax
	xor   ax,ax
	mov   wptr ([pDiskBuff]),offset [DiskBuff+1024]
	mov   wptr ([cbDiskBuff]),ax
	mov   wptr ([fBufferFlags]),ax
	mov   wptr ([LO_posCur]),ax
	mov   wptr ([HI_posCur]),ax
	not   ax
LN_FOF02:
	or    ax,ax
	ret


;-------------------------------------------------------------------------
;	LN_ReadSz()
;-------------------------------------------------------------------------
;	LN_ReadSz reads cx bytes into [ds:di], zero terminates the sz,
;	and returns not zero if cx bytes have been read.
;	only ax and di are altered.
LN_ReadSz:
	call  LN_ReadRgch
	mov   bptr [di],000h
	ret


;-------------------------------------------------------------------------
;	LN_ReadRgch()
;-------------------------------------------------------------------------
;	LN_ReadRgch reads cx bytes into [ds:di] and returns not zero if cx
;	bytes have been read.
;	only ax and di are altered.
LN_ReadRgch:
	or    wptr ([fBufferFlags]),maskfUsedForStructLocal
	push  wptr ([fBufferFlags])
	mov   ax,0FFFFh     ; default return success
	cmp   wptr ([cbDiskBuff]),cx
	jae   LN_RR01
;	LN_FReadFcb takes a pointer to an sz in [ds:doshndCur] and returns
;	zero if reading the file failed, and not zero if it succeeded.
;	ax is 0 upon return iff reading failed, no other registers altered.
	call  LN_FReadFcb
	je    LN_RR02
	add   wptr ([cbDiskBuff]),512
	sub   wptr ([pDiskBuff]),512
LN_RR01:
	sub   wptr ([cbDiskBuff]),cx
	push  si
	mov   si,wptr ([pDiskBuff])
	rep   movsb
	mov   wptr ([pDiskBuff]),si
	pop   si
LN_RR02:
	pop   wptr ([fBufferFlags])
	or    ax,ax
	ret


;-------------------------------------------------------------------------
;	LN_DwSeekDw()
;-------------------------------------------------------------------------
;	LN_DwSeekDw takes cb in dx:bx and fReadFromEnd in cl.
;	If cl is false upon entry *doshndCur is closed and opened, otherwise
;	*doshndCur is left at the current file position.  Then dx:bx
;	bytes are skipped in the file.
LN_DwSeekDw:
	or    wptr ([fBufferFlags]),maskfUsedForStructLocal
	push  wptr ([fBufferFlags])
	mov   ax,0FFFFh     ; default return success
	or    cl,cl
	jne   LN_DSD01
	push  ax
	push  cx
	push  dx
	push  bx
	mov   ax,wptr ([pDiskBuff])
	sub   ax,offset [DiskBuff+1024]
	cwd
	add   ax,wptr ([LO_posCur])
	adc   dx,wptr ([HI_posCur])
	mov   cx,dx	    ; actual fc pointed to by pDiskBuff in cx:ax
	pop   bx
	pop   dx
	mov   si,bx
	mov   di,dx
	sub   bx,ax
	sbb   dx,cx	    ; do we want to move to a position beyond fc
	pop   cx
	pop   ax
	jge   LN_DSD01	    ; yes - just read some more
	mov   bx,si
	mov   dx,di	    ; no - close file and read from beginning
	push  dx	    ; save high cb
;	LN_CloseFcb closes doshndCur.  No registers are altered.
	call  LN_CloseFcb
	mov   dx,wptr ([szFileCur])
;	LN_FOpenFcb takes a pointer to an sz in [ds:dx] and returns
;	zero if opening the file failed, and not zero if it succeeded.
;	The disk transfer address is also set to [DiskBuff+512]
;	ax is 0 upon return iff opening failed, no other registers altered.
	call  LN_FOpenFcb
	pop   dx	    ;restore high cb
	je    LN_DSD02
LN_DSD01:
	jmp   short LN_DSD03
LN_DSD02:
;	LN_FReadFcb takes a pointer to an sz in [ds:doshndCur] and returns
;	zero if reading the file failed, and not zero if it succeeded.
;	ax is 0 upon return iff reading failed, no other registers altered.
	call  LN_FReadFcb
	je    LN_DSD05
	sub   bx,512
	sbb   dx,0
LN_DSD03:
	or    dx,dx
	ja    LN_DSD02
	cmp   bx,512
	ja    LN_DSD02
	cmp   wptr ([cbDiskBuff]),bx
	jae   LN_DSD04
;	LN_FReadFcb takes a pointer to an sz in [ds:doshndCur] and returns
;	zero if reading the file failed, and not zero if it succeeded.
;	ax is 0 upon return iff reading failed, no other registers altered.
	call  LN_FReadFcb
	je    LN_DSD05
	add   wptr ([cbDiskBuff]),512
	sub   wptr ([pDiskBuff]),512
LN_DSD04:
	sub   wptr ([cbDiskBuff]),bx
	add   wptr ([pDiskBuff]),bx
LN_DSD05:
	pop   wptr ([fBufferFlags])
	or    ax,ax
	ret


;-------------------------------------------------------------------------
;	LN_FReadFcb()
;-------------------------------------------------------------------------
;	LN_FReadFcb takes a pointer to an sz in [ds:doshndCur] and returns
;	zero if reading the file failed, and not zero if it succeeded.
;	ax is 0 upon return iff reading failed, no other registers altered.
LN_FReadFcb:
	or    wptr ([fBufferFlags]),maskfUsedStraightLocal
	push  cx
	push  si
	push  di
	mov   si,offset [DiskBuff+512]
	mov   di,offset [DiskBuff]
	mov   cx,256
	rep   movsw
	pop   di
	pop   si
	pop   cx

	push  dx

	push  bx
	push  cx
	mov   bx,wptr ([doshndCur])
	mov   dx,offset [DiskBuff+512]
	mov   cx,512
	mov   ah,03Fh	;Sequential disk read
	int   21h
	mov   dx,00000h
	pop   cx
	pop   bx
	jc    LN_FRF02
	or    ax,ax
	jle   LN_FRF02
	cmp   ax,512
	jbe   LN_FRF01
	int   3
	db	'BV5'
LN_FRF01:
	add   wptr ([LO_posCur]),512
	adc   wptr ([HI_posCur]),0
	push  di
	push  cx
	mov   di,offset [DiskBuff+1024]
	mov   cx,512
	sub   cx,ax
	sub   di,cx
	mov   al,000h
	rep   stosb
	not   dx
	pop   cx
	pop   di
LN_FRF02:
	xchg  ax,dx
	pop   dx
	or    ax,ax
	ret


;-------------------------------------------------------------------------
;	LN_CloseFcb()
;-------------------------------------------------------------------------
;	LN_CloseFcb closes doshndCur.  No registers are altered.
LN_CloseFcb:
	cmp   wptr ([doshndCur]),0FFFFh
	jne   LN_CF01
	int   3
	db	'BV6'
LN_CF01:
	cmp   wptr ([fBufferFlags]),maskfUsedForStructLocal + maskfUsedStraightLocal
	jne   LN_CF02
	int   3
	db	'BV7'
LN_CF02:
	push  bx
	push  ax
	mov   bx,0FFFFh
	xchg  bx,wptr ([doshndCur])
	mov   ah,03Eh	;Close file
	int   21h
	pop   ax
	pop   bx
	ret


;-------------------------------------------------------------------------
;	LN_HOpenDbgScreen()
;-------------------------------------------------------------------------
; %%Function:LN_HOpenDbgScreen %%Owner:BRADV
cProc	LN_HOpenDbgScreen,<NEAR>,<>

cBegin

;	Don't look at vdbs in this version because we're not DEBUG

	PUSH  CS
	MOV   AX,offset szCom1
	PUSH  AX
	mov   AX,offset [BuffStart]
	PUSH  DS
	PUSH  AX
	MOV   CX,00001h   ;OF_WRITE
	PUSH  CX
	cCall OpenFile,<>
	push  cs
	pop   ds
	push  cs
	pop   es

	cEnd

EndOfFixedCode:


;-------------------------------------------------------------------------
;	LN_ClearCSVariables()
;-------------------------------------------------------------------------
; %%Function:LN_ClearCSVariables %%Owner:BRADV
cProc	LN_ClearCSVariables,<NEAR>,<>

cBegin

	push  es
	push  di
	push  cx
	push  ax
	push  cs
	pop   es
	mov   di,offset CSVariables
	mov   al,000h
	mov   cx,cbCSVariables
	rep   stosb
	cmp   wptr (cs:[doshndCur]),-1
	je    LN_CCSV01
	int   3
	db	'BV2'
LN_CCSV01:
	pop   ax
	pop   cx
	pop   di
	pop   es

	cEnd


;-------------------------------------------------------------------------
;	LN_GetSnMac()
;-------------------------------------------------------------------------
; %%Function:LN_GetSnMac %%Owner:BRADV
cProc	LN_GetSnMac,<NEAR>,<>

cBegin

	mov   dx,offset szPcodeMap
;	LN_FOpenFcb takes a pointer to an sz in [ds:dx] and returns
;	zero if opening the file failed, and not zero if it succeeded.
;	The disk transfer address is also set to [DiskBuff+512]
;	ax is 0 upon return iff opening failed, no other registers altered.
	call  LN_FOpenFcb
	je    LN_GSM07
	xor   si,si	;iSn
LN_GSM02:
;	LN_FReadFcb takes a pointer to an sz in [ds:doshndCur] and returns
;	zero if reading the file failed, and not zero if it succeeded.
;	ax is 0 upon return iff reading failed, no other registers altered.
	call  LN_FReadFcb
	je    LN_GSM06
	mov   di,offset [DiskBuff+512]
	mov   al,chLF
	mov   cx,512
LN_GSM03:
	repne scasb
	jne   LN_GSM02
	inc   si	;iSn
	jcxz  LN_GSM02
	jmp   short LN_GSM03
LN_GSM06:
;	LN_CloseFcb closes doshndCur.  No registers are altered.
	call  LN_CloseFcb
	xchg  ax,si	;iSn
LN_GSM07:
	mov   wptr ([snMac]),ax

	cEnd


;-------------------------------------------------------------------------
;	LN_GetTlbxMac()
;-------------------------------------------------------------------------
; %%Function:LN_GetTlbxMac %%Owner:BRADV
cProc	LN_GetTlbxMac,<NEAR>,<>

cBegin

	mov   dx,offset szToolbox
;	LN_FOpenFcb takes a pointer to an sz in [ds:dx] and returns
;	zero if opening the file failed, and not zero if it succeeded.
;	The disk transfer address is also set to [DiskBuff+512]
;	ax is 0 upon return iff opening failed, no other registers altered.
	call  LN_FOpenFcb
	je    LN_GTM11
LN_GTM01:
;	LN_FReadFcb takes a pointer to an sz in [ds:doshndCur] and returns
;	zero if reading the file failed, and not zero if it succeeded.
;	ax is 0 upon return iff reading failed, no other registers altered.
	call  LN_FReadFcb
	je    LN_GTM10
	mov   dx,512
LN_GTM02:
	mov   ax,0FFFFh
	or    dx,dx
	jle   LN_GTM10
	push  dx
;	LN_FReadFcb takes a pointer to an sz in [ds:doshndCur] and returns
;	zero if reading the file failed, and not zero if it succeeded.
;	ax is 0 upon return iff reading failed, no other registers altered.
	call  LN_FReadFcb
	pop   cx
	mov   dx,0
	je    LN_GTM03
	mov   dx,512
LN_GTM03:
	mov   di,offset [DiskBuff]
LN_GTM04:
	mov   al,bptr [rgchtlbxMac]
	repne scasb
	jne   LN_GTM02
	push  cx
	push  di
	add   cx,dx
	cmp   cx,cbtlbxMac+5	; after 'tlbxMac ' must not have junk
	jl    LN_GTM057
LN_GTM053:
	dec   di
	mov   cx,cbtlbxMac
	mov   si,offset rgchtlbxMac
	repe  cmpsb
	je    LN_GTM06
LN_GTM057:
	pop   di
	pop   cx
	jmp   short LN_GTM04
LN_GTM06:
	xor   ax,ax
	cwd
	mov   si,di
LN_GTM07:
	lodsb
	sub   al,'0'
	cmp   al,10
	jae   LN_GTM08
	shl   dx,1
	mov   cx,dx
	shl   dx,1
	shl   dx,1
	add   dx,cx
	add   dx,ax
	jmp   short LN_GTM07
LN_GTM08:
	xchg  ax,dx
	pop   di
	pop   cx
LN_GTM10:
;	LN_CloseFcb closes doshndCur.  No registers are altered.
	call  LN_CloseFcb
LN_GTM11:
	mov   wptr ([tlbxMac]),ax

	cEnd


;-------------------------------------------------------------------------
;	LN_GetExeHead()
;-------------------------------------------------------------------------
; %%Function:LN_GetExeHead %%Owner:BRADV
cProc	LN_GetExeHead,<NEAR>,<>

cBegin

	mov   dx,offset szKernelSym
;	LN_FOpenFcb takes a pointer to an sz in [ds:dx] and returns
;	zero if opening the file failed, and not zero if it succeeded.
;	The disk transfer address is also set to [DiskBuff+512]
;	ax is 0 upon return iff opening failed, no other registers altered.
	call  LN_FOpenFcb
	je    LN_GEH03
	mov   ax,offset szHEXEHEAD
	push  ax
	cCall LN_FFindSymbName,<>
	or    ax,ax
	je    LN_GEH02
LN_GEH01:
	mov   bx,wptr ([SymDefSwap.sym_valSymdef])
	mov   wptr ([Off_hExeHead]),bx
LN_GEH02:
;	LN_CloseFcb closes doshndCur.  No registers are altered.
	call  LN_CloseFcb
LN_GEH03:

	cEnd


;-------------------------------------------------------------------------
;	LN_FFindSymbName( szSymbol )
;-------------------------------------------------------------------------
; %%Function:LN_FFindSymbName %%Owner:BRADV
cProc	LN_FFindSymbName,<NEAR>,<>
	ParmW	szSymbol

	LocalW	iSeg
	LocalW	iSym
	LocalW	cbsz

cBegin

	mov   di,[szSymbol]
	mov   cx,0FFFFh
	mov   al,0
	repne scasb
	dec   di
	sub   di,[szSymbol]
	mov   [cbsz],di
	mov   [iSeg],00001h
LN_FFSN03:
	push  [iSeg]
	cCall LN_FGetSegSymb,<>
	or    ax,ax
	je    LN_FFSN07
	mov   [iSym],00000h
LN_FFSN04:
	mov   ax,[iSym]
	cmp   ax,wptr ([SegDefSwap.sym_cntSegdef])
	jge   LN_FFSN06
	mov   di,offset SymDefSwap
	mov   cx,cbSymdef
;	LN_ReadRgch reads cx bytes into [ds:di] and returns not zero if cx
;	bytes have been read.
;	only ax and di are altered.
	call  LN_ReadRgch
	jz    LN_FFSN07
	mov   di,offset [szSegName]
	mov   cl,bptr ([SymDefSwap.nam_lenSymdef])
	xor   ch,ch
;	LN_ReadRgch reads cx bytes into [ds:di] and returns not zero if cx
;	bytes have been read.
;	only ax and di are altered.
	call  LN_ReadRgch
	jz    LN_FFSN07
	mov   al,bptr ([SymDefSwap.nam_lenSymdef])
	xor   ah,ah
	cmp   AX,[cbsz]
	jne   LN_FFSN05
	mov   si,offset [szSegName]
	mov   di,[szSymbol]
	mov   cx,[cbsz]
	repe  cmpsb	;FNeRgch
	mov   ax,0FFFFh
	je    LN_FFSN07
LN_FFSN05:
	inc   [iSym]
	jmp   short LN_FFSN04
LN_FFSN06:
	inc   [iSeg]
	jmp   short LN_FFSN03
LN_FFSN07:

	cEnd


	assumes ds,dgroup

;-------------------------------------------------------------------------
;	InstallInt3FHandler( )
;-------------------------------------------------------------------------
;/* I N S T A L L  I N T  3 F  H A N D L E R */
; %%Function:InstallInt3FHandler %%Owner:BRADV
cProc	InstallInt3FHandler,<FAR,PUBLIC,ATOMIC>,<>

cBegin

	cmp	wptr (ss:[hInt3FCode]),fFalse
	jne	II3H02
	push	ds
	push	es
	db	060h	 ;push all
	push	cs
	pop	ds
	push	cs
	pop	es
	call	LN_GetSnMac
	or	ax,ax
	je	II3H01
	call	LN_GetTlbxMac
	or	ax,ax
	je	II3H01
	call	LN_GetExeHead
	or	ax,ax
	je	II3H01
	mov	ax,offset EndOfFixedCode
	cwd
	mov	bx,GMEM_NOT_BANKED
	cCall	GlobalAlloc,<bx, dx, ax>    ;get fixed segment for int3fcode
	cmp	ax,NULL
	je	II3H01			    ;return if segment non-existant
	mov	wptr (ss:[hInt3fCode]),ax
	push	cs
	pop	ds
	mov	es,ax
	mov	cx,offset EndOfFixedCode
	xor	si,si
	xor	di,di
	rep	movsb			    ;copy new int3f code in
	xor	ax,ax
	mov	ds,ax
	mov	si,3Fh*4
	mov	di,offset Int3fAddress
	movsw
	movsw				    ;copy old int3f into handler
	mov	bx,3fh*4
	mov	wptr ([bx]),offset Int3fHandler
	mov	wptr ([bx+2]),es

II3H01:
	call	LN_ClearCSVariables
	db	061h	 ;pop all
	pop	es
	pop	ds
	jmp	short II3H03
II3H02:
	push	ds
	push	es
	db	060h	 ;push all
	xor	ax,ax
	mov	es,ax
	mov	ds,wptr (ss:[hInt3fCode])
	mov	si,offset Int3fAddress
	mov	di,3Fh*4
	movsw
	movsw
	cCall	GlobalFree,<ds>
	db	061h	 ;pop all
	pop	es
	pop	ds
II3H03:

cEnd

; End of InstallInt3FHandler

sEnd	int3f
	end
