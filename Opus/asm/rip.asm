        page    58,82

;
; stacktrace using BP.  Go through BP chain, attempting to find calls to
; current procedure and displaying name (if it exists symbolicly) and
; arguments.
;

include noxport.inc
include newexe.inc


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

sBegin	DATA

staticB segiT,<SIZE SEGI DUP (?)>	;* SEGI buffer for GetCodeInfo

sEnd	DATA



createSeg	_RIP,rip,byte,public,CODE


PUBLIC  StackTrace

externFP        <FGetSegSymb, FFindSeg, FGetSymbol>
externFP        <GetSnMac, FGetPcodeSegName, FGetConstSymb>
externFP        <HOpenOutFile, DwSeekDw, CchWriteDoshnd, FCloseDoshnd>
externFP        <HOpenDbgScreen>
externFP        <GetCodeInfo,GlobalHandle>
externA 	$q_snMacReal

sBegin data
externW         SymDef
externW         $q_mpsnpfn
externW         vpszStackTrace1
externW         vpszStackTrace2

fDoingRIP       dw      0               ; currently in process of ripping...
sEnd   DATA



; CODE SEGMENT _RIP

sBegin	rip
	assumes cs,rip
	assumes ds,dgroup
	assumes ss,dgroup


csCurrent       equ     bp-8            ; Most recent CS during backtrace
bpCurrent       equ     bp-10           ; Most recent BP during backtrace
bpLast          equ     bp-12           ; Most recent BP during backtrace
fPcodeFrame     equ     bp-14           ; TRUE == Pcode frame
RetSeg          equ     bp-16           ; return segment from frame
RetOffset       equ     bp-18           ; return offset from frame
SnMac           equ     bp-20           ; count of pcode segments
pBuff           equ     bp-22           ; pointer into buffer
BuffStart       equ     bp-(24+80)      ; buffer start
SymFile         equ     bp-(104+66)     ; sym file name
hOutFile        equ     bp-172          ; dos handle for output file
chOutBuff       equ     bp-174          ; buffer for a char to output
pArgStart       equ     bp-176          ; pointer to start of args
pArgEnd         equ     bp-178          ; pointer to end of args
pLocalStart     equ     bp-180          ; pointer to start of locals
pLocalEnd       equ     bp-182          ; pointer to end of locals
cPcodeArgs      equ     bp-184          ; count of pcode args
fPrintDbgScr    equ     bp-186          ; print chars to the aux port?
fThunk          equ     bp-188          ; true iff last far ret was a thunk address
hDbgScreen      equ     bp-190          ; dos handle for debug screen
fNextFrameNat   equ     bp-192          ; true iff switching from pcode to native

cbLocalsMax     equ     194



;* * * String pool

szSnMsg:        DB      "pcode : sn = ", 0
szBpcMsg:       DB      "   bpc = ", 0
szBadBP         DB      0dh, 0ah, "Illegal Frame Pointer = ", 0
szNoStack       DB      0dh, 0ah, "Not enough stack to run RIP code", 0dh, 0ah, 0
szArg           DB      "  Args:", 0dh, 0ah, 0
szLocal         DB      "  Locals:", 0dh, 0ah, 0


StackTrace      proc    far
        cmp     ss:[fDoingRIP],0        ; are we currently ripping?
        jz      traceinit               ; no - skip
        ret                             ; yes - just return

traceinit:
        mov     ax,sp                   ; check to see if we have enough
        sub     ax,cbLocalsMax          ;   stack to run the RIP code
        cmp     ax,ss:[0ah]
        ja      traceinit2

        mov     bx,offset szNoStack     ; send message about out of stack space
        push    cs
        pop     es
        call    PrintSz
        ret

traceinit2:
        mov     ss:[fDoingRIP],1        ; indicate we are currently ripping

        INC     BP                      ; setup a standard frame
        PUSH    BP
        MOV     BP,SP
        PUSH    DS
        PUSH    DI
        PUSH    SI
        SUB     SP,cbLocalsMax          ; save room for locals...

        push    ss                      ; set ds = frame seg
        pop     ds

; init locals
        mov     word ptr [fPrintDbgScr],1       ; indicate printing to the debug screen OK
        call    HOpenDbgScreen          ; enable printing to the debug screen?
        mov     [hDbgScreen],ax         ; save handle to debug screen
        cmp     ax,-1                   ; legal handle?
        jne     traceinit3              ; yes - skip
        mov     word ptr [fPrintDbgScr],0       ; indicate printing to the debug screen not OK

traceinit3:
        mov     bx,[bp]                 ; init past the DoStackTrace proc
        and     bl,0feh
        mov     [bpLast],bx
        mov     word ptr [fPcodeFrame],1        ; start out in pcode mode
        mov     word ptr [fThunk],0     ; clear thunk hit flag
        mov     bx,[bx]
        and     bl,0feh
        mov     [bpCurrent],bx
        mov     [csCurrent],cs
        mov     ax,0
        mov     [pArgStart],ax
        mov     [pArgEnd],ax
        mov     [pLocalStart],ax
        mov     [pLocalEnd],ax
        mov     [fNextFrameNat],ax
        call    GetSnMac
        mov     [SnMac],ax

        call    HOpenOutFile            ; open the output file
        mov     [hOutFile],ax           ; save returned file handle

        cmp     ax,-1                   ; was the file opened?
        je      traceinit4              ; no - skip

        push    ax                      ; handle of file to seek in
        xor     ax,ax                   ; offset to seek to
        push    ax
        push    ax
        mov     ax,2                    ; seek to end of file
        push    ax
        call    DwSeekDw
        test    dx,8000h                ; error?
        jz      traceinit4              ; no - skip
        lea     bx,[hOutFile]
        call    CloseOutFile            ; yes - close the file

traceinit4:
        call    doutcrlf
        call    doutcrlf

        mov     bx,[vpszStackTrace1]    ; get Stack Trace Message
        push    ss
        pop     es
        call    PrintSz

        call    doutcrlf

        mov     bx,[vpszStackTrace2]    ; state info
        push    ss                      ; point es to local vars area
        pop     es
        call    PrintSz

        call    doutcrlf
        call    doutcrlf

traceloop:
        mov     bx,[bpCurrent]
        mov     [bpLast],bx
        mov     bx,[bx]                 ; get previous bp
        and     bl,0feh                 ; strip off near/far flag
        mov     [bpCurrent],bx          ; and save it
        or      bx,bx                   ; end of frames?
        jnz     traceloop2              ; no - skip
        jmp     endtrace                ; yes - quit

traceloop2:
        cmp     bx,[bpLast]             ; is this OK?
        jbe     traceloop4              ; no - skip

        lea     ax,[BuffStart]          ; get address of start of strings
        mov     [pBuff],ax              ; and save it

        cmp     bx,word ptr ds:[0ah]    ; below stack min?
        jb      traceloop4
        cmp     bx,word ptr ds:[0eh]    ; or above stack max?
        jbe     traceloop6

traceloop4:
        jmp     badbp

traceloop6:
        cmp     word ptr [fPcodeFrame],0        ; were we in a pcode frame?
        jz      traceloop8              ; no - skip
        cmp     word ptr [fNextFrameNat],0      ; native code in this frame?
        jz      traceloop7              ; no - skip
        mov     word ptr [fPcodeFrame],0        ; yes - indicate native code
        mov     word ptr [fNextFrameNat],0      ; clear flag
        jmp     traceloop8              ; skip

traceloop7:
        test    byte ptr [bx],1         ; are we switching to native?
        jz      traceloop8              ; no - skip
        mov     word ptr [fNextFrameNat],1      ; yes - indicate native code in next frame
        mov     bx,[bpLast]             ; check for return sn == snMac
        mov     ax,[bx+4]
        and     ax,7fffh
        cmp     ax,[snMac]
        jz      traceloop               ; yes - skip to next frame

traceloop8:
        call    GetRetAddr              ; get next return address

        mov     [RetOffset],ax          ; save address for later...
        mov     [RetSeg],dx

        cmp     word ptr [fPcodeFrame],0        ; are we in a pcode frame?
        jz      traceloop10             ; no - skip
        call    PrintPcodeAddress       ; yes - print sn:bpc
        jmp     traceloop34

traceloop10:
        cmp     word ptr [fThunk],0     ; is this a short ret to a swapped segment?
        jz      traceloop11             ; no - skip
        push    dx                      ; yes - pass the segment number
        lea     bx,[SymFile]            ; pass the address to save the sym file name
        push    bx
        push    [pBuff]                 ; pass the address to save the seg name
        call    FGetSegSymb             ; get the segment symbol
        jmp     traceloop18

traceloop11:
;*	* See if a Windows thunk
        mov     es,[RetSeg]             ; get seg and offset of return
        mov     bx,[RetOffset]

        cmp     word ptr es:[bx],INTVECTOR shl 8 OR INTOPCODE
        jne     traceloop16
        cmp     byte ptr es:[bx+2],0
        jne     traceloop16
        xor     cx,cx
        mov     cl,es:[bx+3]            ; get segment number
        jcxz    traceloop16             ; not thunk if == 0

traceloop12:
        add     bx,4                    ; Scan forwards for end of table
        cmp     word ptr es:[bx],0
        jnz     traceloop12 
        mov     es,word ptr es:[bx+2]   ; Got it, recover EXE header address
        cmp     word ptr es:[ne_magic],NEMAGIC  ; Is it a new exe header?
        jne     traceloop16             ; No, then cant be return thunk.

        push    cx                      ; save segment number
        mov     [csCurrent],cx
        mov     word ptr [fThunk],1     ; indicate hit a thunk

        mov     bx,es:[ne_restab]       ; get pointer to stModuleName
        mov     cl,es:[bx]              ; get length of module name
        lea     di,[SymFile]            ; get the address to save the sym file name

traceloop14:
        inc     bx                      ; point to next char
        mov     al,es:[bx]              ; get a char
        mov     [di],al                 ; save it
        inc     di
        loop    traceloop14

        mov     word ptr [di],'s.'      ; save .sym on end of module name
        mov     word ptr [di+2],'my'
        mov     byte ptr [di+4],0

        mov     bx,[bpLast]             ; get pointer to frame
        mov     bx,[bx-2]               ; get "real" return offset
        mov     [RetOffset],bx          ; and save it

        lea     bx,[SymFile]            ; pass the address to save the sym file name
        push    bx
        push    [pBuff]                 ; pass the address to save the seg name
        call    FGetSegSymb             ; get the segment symbol
        jmp     traceloop18

traceloop16:
        push    [RetSeg]                ; pass the segment to look up
        lea     bx,[SymFile]            ; pass the address to save the sym file name
        push    bx
        push    [pBuff]                 ; pass the address to save the seg name
        call    FFindSeg                ; find the segment

traceloop18:
        or      ax,ax                   ; successful?
        jnz     traceloop20             ; yes - skip
        call    PrintSegOff             ; no - just print segment:offset
        jmp     traceloop34

traceloop20:
        mov     bx,[pBuff]              ; get the pointer to the buffer
        dec     bx

traceloop22:
        inc     bx                      ; point to next char
        cmp     byte ptr [bx],0         ; end of string?
        jnz     traceloop22             ; no - loop
        mov     byte ptr [bx],':'       ; yes - save seperator in string
        inc     bx                      ; point to where to put offset symbol
        mov     [pBuff],bx              ; and save it

        push    [RetOffset]             ; pass the offset to lookup
        push    [pBuff]                 ; pass the place to put the offset symbol
        call    FGetSymbol              ; get the symbol     
        or      ax,ax                   ; successful?
        jnz     traceloop24             ; yes - skip
        call    PrintSegOff             ; no - just print segment:offset
        jmp     traceloop34

traceloop24:
;*      * before printing : compare with special names : RetToolbox? RetNative?

        mov     bx,[pBuff]              ; get the pointer to the buffer

;*      * possibly "RetNative?" (sure it's ugly - but fast & simple)
        cmp     word ptr [bx],"eR"
        jne     traceloop26
        cmp     word ptr [bx+2],"Nt"
        jne     traceloop26
        cmp     word ptr [bx+4],"ta"
        jne     traceloop26
        cmp     word ptr [bx+6],"vi"
        jne     traceloop26
        cmp     byte ptr [bx+8],"e"
        jne     traceloop26
        cmp     byte ptr [bx+9],"0"
        jb      traceloop26
        cmp     byte ptr [bx+9],"3"
        ja      traceloop26
        cmp     byte ptr [bx+10],0
        je      traceloop28

traceloop26:

;*      * possibly "RetToolbox?"
        cmp     word ptr [bx],"eR"
        jne     traceloop30
        cmp     word ptr [bx+2],"Tt"
        jne     traceloop30
        cmp     word ptr [bx+4],"oo"
        jne     traceloop30
        cmp     word ptr [bx+6],"bl"
        jne     traceloop30
        cmp     word ptr [bx+8],"xo"
        jne     traceloop30
        cmp     byte ptr [bx+10],"0"
        jb      traceloop30
        cmp     byte ptr [bx+10],"3"
        ja      traceloop30
        cmp     byte ptr [bx+11],0
        jne     traceloop30

;*      * Start of Pcode frame
traceloop28:
        mov     word ptr [fPcodeFrame],1
        mov     word ptr [fThunk],0     ; clear thunk hit flag
        jmp     traceloop40             ; skip to next frame...

traceloop30:
        lea     bx,[SymFile]            ; point to sym file name

traceloop32:
        inc     bx                      ; point to next char
        cmp     byte ptr [bx],'.'       ; at file extension yet?
        jne     traceloop32             ; no - loop
        mov     byte ptr [bx],0         ; yes - end string here
        mov     dx,bx                   ; and save for a moment

        lea     bx,[SymFile]            ; point to sym file name
        push    ss                      ; point es to local vars area 
        pop     es
        call    PrintSz                 ; print the map name

        mov     bx,dx                   ; point to where we ended the string 
        mov     byte ptr [bx],'.'       ; restore the extension

        mov     al,'!'                  ; seperator between map name and seg name
        call    dout

        lea     bx,[BuffStart]          ; point to seg and symbol names
        push    ss                      ; point es to local vars area 
        pop     es
        call    PrintSz                 ; print the map name

        mov     cx,[RetOffset]          ; get offset
        sub     cx,[SymDef]             ; calc offset from start of proc
        jcxz    traceloop34             ; skip if no offset

        mov     al,'+'                  ; seperator between symbol name and offset
        call    dout

        mov     ax,cx                   ; output the offset
        call    dout16

traceloop34:
        cmp     word ptr [fPcodeFrame],0        ; pcode frame?
        jz      traceloop36             ; no - skip
        cmp     word ptr [fNextFrameNat],0      ; is this really a native frame?
        jnz     traceloop36             ; yes - skip
        call    SetupPcodeArgs          ; yes - setup to print pcode locals and args
        jmp     traceloop38

traceloop36:
        call    SetupNatArgs            ; setup to print native locals and args

traceloop38:
        call    PrintArgs               ; print this frame's locals and args
        call    doutcrlf                ; output a carrage return line feed

traceloop40:
        jmp     traceloop               ; and loop and loop and...

badbp:
        mov     bx,offset szBadBP       ; print bad BP message
        push    cs
        pop     es
        call    PrintSz
        mov     ax,[bpLast]
        call    dout16
        call    doutcrlf


endtrace:
        lea     bx,[hOutFile]
        call    CloseOutFile            ; close the output file
        lea     bx,[hDbgScreen]
        call    CloseOutFile            ; close the debug screen

        sub     bp,0006
        mov     sp,bp
        pop     si
        pop     di
        pop     ds
        pop     bp
        dec     bp

        mov     ss:[fDoingRIP],0        ; indicate we are not currently ripping
        ret
stacktrace      endp



;
; GetRetAddr - peeks at frame to get return address
;
; enter:
; exit:  dx:ax = return address
;
GetRetAddr:
        mov     bx,[bpLast]
        mov     ax,[bx+2]               ; get offset part of ret addr
        mov     dx,[bx+4]               ; get segment part
        test    byte ptr [bx],1         ; near or far frame?
        jz      getnearaddr
;
; far frame
;
        mov     [csCurrent],dx          ; remember current CS
        mov     word ptr [fThunk],0     ; clear thunk hit flag
        ret

getnearaddr:
        test    byte ptr [fPcodeFrame],0ffh
        jz      near_native

;*      * Pcode return
        and     dh,07fh                 ;* strip fValue bit
        ret

near_native:
        mov     dx,[csCurrent]          ; use current CS
        ret



;
;  Print return address in form:
;
;       <segment>:<offset>
;
PrintSegOff:
        mov     ax,[RetSeg]
        call    dout16
        mov     al,":"
        call    dout
        mov     ax,[RetOffset]
        call    dout16
        ret


;*      * Special Pcode routines

;*      print Pcode address

PrintPcodeAddress:
        mov     word ptr [cPcodeArgs],0 ; init count of pcode args
        mov     bx,offset szSnMsg       ; print sn message
        push    cs
        pop     es
        call    PrintSz
        mov     ax,[RetSeg]             ; get sn
        push    ax                      ; save for call to FGetPcodeSegName
        call    dout16
        push    [pBuff]                 ; point to buffer
        call    FGetPcodeSegName
        or      ax,ax                   ; seg name found?
        jz      PrintPcode2             ; no - skip

        mov     al," "                  ; print seg name
        call    dout
        mov     al," "
        call    dout
        mov     al,"("
        call    dout
        push    ds
        pop     es
        mov     bx,[pBuff]
        call    PrintSz
        mov     al,")"
        call    dout

PrintPcode2:
        mov     bx,offset szBpcMsg      ; print bpc message
        push    cs
        pop     es
        call    PrintSz
        mov     ax,[RetOffset]          ; get bpc
        call    dout16

        mov     bx,[RetSeg]             ; get sn
        push    bx
        call    near ptr GetPsOfSn       ; get physical address  of pcode seg
        or      ax,ax                   ; swapped out?             
        jnz     PrintPcode4             ; no - skip
        jmp     PrintPcode12            ; yes - exit

PrintPcode4:
        mov     es,ax                   ; in memory - put seg in es

        mov     bx,offset ph_mpenbpc    ; point to mpenbpc table
        xor     cx,cx
        mov     cl,es:[ph_enMacReg]     ; get count of procs
        mov     dx,0                    ; init proc counter

PrintPcode6:
        mov     ax,es:[bx]              ; get the next entry point
        cmp     ax,[RetOffset]          ; is this one past the one we want?
        ja      PrintPcode8             ; yes - skip
        inc     bx                      ; point to next entry
        inc     bx
        inc     dx                      ; bump proc counter
        loop    PrintPcode6             ; and loop

PrintPcode8:
        mov     bx,es:[bx-2]            ; save start of the proc
        push    bx
        mov     al,es:[bx-1]            ; get seg code byte
        and     ax,001fh                ; strip off all but count of args
        mov     [cPcodeArgs],ax

        dec     dx                      ; set proc number back

        mov     dh,[RetSeg]             ; get seg in dh, proc # in dl
        push    dx

        lea     bx,[SymFile]            ; get pointer to sym file name
        mov     [bx],'po'               ; and save 'opus.sym' in it
        mov     [bx+2],'su'
        mov     [bx+4],'s.'
        mov     [bx+6],'my'
        mov     byte ptr [bx+8],0
        push    bx
        push    [pBuff]                 ; point to buffer
        call    FGetConstSymb
        pop     dx                      ; get back start of proc offset
        or      ax,ax                   ; OK?
        jz      PrintPcode12            ; no - exit

        mov     al,' '                  ; seperator between map name and seg name
        call    dout
        mov     al,' '                  ; seperator between map name and seg name
        call    dout

        mov     al,'('                  ; seperator between map name and seg name
        call    dout

        lea     bx,[BuffStart]          ; point to seg and symbol names
        push    ss                      ; point es to local vars area 
        pop     es
        call    PrintSz                 ; print the map name

        mov     cx,[RetOffset]          ; get offset
        sub     cx,dx                   ; calc offset from start of proc
        dec     cx
        dec     cx
        jcxz    PrintPcode10            ; skip if no offset

        mov     al,'+'                  ; seperator between symbol name and offset
        call    dout

        mov     ax,cx                   ; output the offset
        call    dout16

PrintPcode10:
        mov     al,')'                  ; seperator between map name and seg name
        call    dout

PrintPcode12:
        ret


PrintArgs:
        push    [fPrintDbgScr]             ; save flag
        mov     word ptr [fPrintDbgScr], 0 ; don't print args to the aux port

        call    doutcrlf

        mov     bx,offset szArg         ; point to args string
        push    cs                      ; point es to local vars area 
        pop     es
        call    PrintSz                 ; print the string

        mov     si,-2
        mov     bx,[pArgStart]          ; point to start of args
        mov     cx,bx
        sub     cx,[pArgEnd]            ; calc cb to print
        cmp     cx,0
        jns     PrintArgs2              ; skip if don't need to fix increment
        mov     si,2
        neg     cx

PrintArgs2:
        call    DumpMem                 ; dump the args

        mov     bx,offset szLocal       ; point to locals string
        push    cs                      ; point es to local vars area 
        pop     es
        call    PrintSz                 ; print the map name

        mov     si,-2
        mov     bx,[pLocalStart]        ; point to start of locals
        mov     cx,bx
        sub     cx,[pLocalEnd]          ; calc cb to print
        cmp     cx,0
        jns     PrintArgs4              ; skip if don't need to fix increment
        mov     si,2
        neg     cx

PrintArgs4:
        call    DumpMem                 ; dump the locals

        pop     [fPrintDbgScr]          ; restore flag
        xor     ax,ax                   ; clear pointers
        mov     [pArgStart],ax
        mov     [pArgEnd],ax
        mov     [pLocalStart],ax
        mov     [pLocalEnd],ax
        ret




DumpMem:
        shr     cx,1                    ; make into cwDump
        jcxz    DumpMem6                ; exit if nothing to print
        jmp     DumpMem3

DumpMem2:
        cmp     dl,8                    ; done with this line?
        jb      DumpMem4                ; no - skip
        call    doutcrlf

DumpMem3:
        mov     al,' '
        call    dout
        call    dout
        call    dout
        call    dout
        mov     dl,0

DumpMem4:
        mov     ax,[bx]                 ; get a word
        call    dout16
        mov     al,' '                  ; seperator between numbers
        call    dout
        call    dout

        add     bx,si                   ; point to next word
        inc     dl                      ; bump counter
        loop    DumpMem2                ; loop until all words printed
        call    doutcrlf

DumpMem6:
        ret



SetupPcodeArgs:
        mov     ax,[bpCurrent]          ; set pArgStart and pArgEnd
        dec     ax
        dec     ax
        mov     [pArgStart],ax
        mov     cx,[cPcodeArgs]
        shl     cx,1
        sub     ax,cx
        mov     [pArgEnd],ax

        mov     [pLocalStart],ax        ; set pLocalStart and pLocalEnd
        mov     ax,[bpLast]
        add     ax,4
        mov     [pLocalEnd],ax
        ret



SetupNatArgs:
        mov     ax,[bpCurrent]          ; set pArgStart and pArgEnd
        mov     bx,ax
        add     ax,4
        test    word ptr [bx],0001h
        jz      SetupNat2
        add     ax,2

SetupNat2:
        mov     [pArgStart],ax
        mov     bx,[bx]
        and     bl,0feh
        sub     bx,4
        cmp     bx,-4
        jne     SetupNat4
        mov     bx,ax

SetupNat4:
        mov     [pArgEnd],bx

        mov     ax,[bpCurrent]          ; set pLocalStart and pLocalEnd
        sub     ax,8
        mov     [pLocalStart],ax
        mov     ax,[bpLast]
        add     ax,8
        mov     [pLocalEnd],ax
        ret




;*
;* PrintSz : ES:BX = "sz" string
;*
;*      print string

PrintSz:
        push    ax                      ; save regs

PrintSz2:
        mov     al,es:[bx]              ; get a char
        inc     bx                      ; point to next char
        cmp     al,0                    ; end of string?
        jz      PrintSz4                ; yes - exit
        call    dout                    ; output the char
        jmp     PrintSz2                ; loop

PrintSz4:
        pop     ax
        ret



MakeHexDigit:
        add     al,'0'
        cmp     al,'9'
        jbe     MakeHex2
        add     al,'A' - '0' - 10

MakeHex2:
        ret


; print out a 16 bit unsigned in 4 hex digit format
dout16:
        push    bx                      ; save environment
        push    cx

        mov     bx,ax                   ; val saved in bx
        mov     al,ah
        mov     cl,4
        shr     al,cl
        call    MakeHexDigit
        call    dout
        mov     al,bh
        and     al,0fh
        call    MakeHexDigit
        call    dout
        mov     al,bl
        mov     cl,4
        shr     al,cl
        call    MakeHexDigit
        call    dout
        mov     al,bl
        and     al,0fh
        call    MakeHexDigit
        call    dout

        pop     cx                      ; restore environment
        pop     bx
        ret


; print out a carrage return line feed pair
doutcrlf:
        mov     al,0dh
        call    dout
        mov     al,0ah
        call    dout
        ret


; print out a char
dout:
        push    bx                      ; save environment

        cmp     word ptr [fPrintDbgScr],0       ; print to the debug screen?
        jz      dout2                   ; no - skip
        lea     bx,[hDbgScreen]         ; get handle of debug screen to print to
        call    doutfile

dout2:
        lea     bx,[hOutFile]           ; get handle of file to print to
        call    doutfile

        pop     bx                      ; restore environment
        ret


; print out a char to the opened file
doutfile:
        cmp     [bx],0ffffh             ; exit if file not open
        je      doutfile4

        push    ax                      ; save environment
        push    cx
        push    dx
        push    bx

        push    [bx]                    ; pass file handle
        lea     bx,[chOutBuff]
        push    ss
        push    bx
        mov     [bx],al
        mov     ax,1
        push    ax
        call    CchWriteDoshnd
        test    ax,8000h
        jz      doutfile2
        pop     bx                      ; close this file if error
        push    bx
        call    CloseOutFile

doutfile2: 
        pop     bx                      ; restore environment
        pop     dx
        pop     cx
        pop     ax

doutfile4: 
        ret


; close the output file
CloseOutFile:
        cmp     [bx],0ffffh
        je      CloseOut2

        push    ax                      ; save environment
        push    cx
        push    dx

        push    bx

        push    [bx]
        call    FCloseDoshnd
        pop     bx
        mov     [bx],0ffffh

        pop     dx                      ; restore environment
        pop     cx
        pop     ax

CloseOut2: 
        ret


;********** GetPsOfSn **********
;*	entry : sn = Pcode segment #
;*	* find physical segment of Pcode segment (if resident)
;*	exit : AX = ps (or 0 if not resident)

; %%Function:GetPsOfSn %%Owner:BRADV
cProc	GetPsOfSn, <NEAR, ATOMIC>
    ParmW  sn
cBegin	GetPsOfSn

	XOR	AX,AX			;* return zero if segMac
	MOV	BX,sn
	CMP	BX,$q_snMacReal
	JAE	get_ps_end
	SHL	BX,1			;* Make into word pointer
	SHL	BX,1			;* DWORD ptr
	ADD	BX,OFFSET DGROUP:$q_mpsnpfn	;* DS:BX => table
	PUSH	WORD PTR [BX+2]			;* segment
	PUSH	WORD PTR [BX]			;* offset (lpthunk on stack)
	MOV	BX,OFFSET DGROUP:segiT		;* address of buffer
	cCall	GetCodeInfo,<DS,BX>		;* lpthunk, lpsegi

	OR	AX,AX
	JZ	get_ps_end			;* error - assume non-resident
;*	* good segment info, see if segment resident
	MOV	AX,segiT.hSegi			;* handle or ps if fixed
	TEST	AX,1
	JNZ	get_ps_end			;* ax = ps
	cCall	GlobalHandle,<ax>		;* get handle info
	MOV	AX,DX				;* ps (or 0 if not loaded)
get_ps_end:

cEnd	GetPsOfSn






sEnd    rip

        END
