;
; auxout.asm
;
;  written by marc singer
;    30 Aug 85
;
; This code allows a string to be written directly to the aux port
; without having to deal with opening it.
;
;       MUST DEFINE AUXOUT TO GET THIS CODE.  IT TAKES TOO MUCH DS.
;
;
.xlist
include w2.inc
include cmacros.inc
.list

sBegin DATA
    staticB iMsg, 0     ; Only lower byte used
    globalW vmsgShowExclude, 118h;
sEnd   DATA

sBegin CODE

assumes CS, CODE
assumes DS, DATA

    staticB msgMap, 0   ; This defines the label and the NULL message
    db 0, 'NULL'        ; WM_NULL
    dw 0001H
    db 'CREA'           ; WM_CREATE
    dw 0002H
    db 'DEST'           ; WM_DESTROY
    dw 0003H
    db 'MOVE'           ; WM_MOVE
    dw 0004H
    db 'SIZW'           ; WM_SIZEWAIT
    dw 0005H
    db 'SIZE'           ; WM_SIZE
    dw 0006H
    db 'ACTI'           ; WM_ACTIVATE
    dw 0007H
    db 'SETF'           ; WM_SETFOCUS
    dw 0008H
    db 'KILF'           ; WM_KILLFOCUS
    dw 0009H
    db 'SVIS'           ; WM_SETVISIBLE
    dw 000aH
    db 'ENAB'           ; WM_ENABLE
    dw 000bH
    db 'SRDW'           ; WM_SETREDRAW
    dw 000cH
    db 'STEX'           ; WM_SETTEXT
    dw 000dH
    db 'GTEX'           ; WM_GETTEXT
    dw 000eH
    db 'GLEN'           ; WM_GETTEXTLENGTH
    dw 000fH
    db 'PAIN'           ; WM_PAINT
    dw 0010H
    db 'CLOS'           ; WM_CLOSE
    dw 0011H
    db 'QEND'           ; WM_QUERYENDSESSION
    dw 0012H
    db 'QUIT'           ; WM_QUIT
    dw 0013H
    db 'QOPN'           ; WM_QUERYOPEN
    dw 0014H
    db 'EBKG'           ; WM_ERASEBKGND
    dw 0015H
    db 'SYCO'           ; WM_SYSCOLORCHANGE
    dw 0016H
    db 'ENDS'           ; WM_ENDSESSION
    dw 0017H
    db 'SYER'           ; WM_SYSTEMERROR
    dw 0018H
    db 'SHOW'           ; WM_SHOWWINDOW
    dw 0019H
    db 'CTLC'           ; WM_CTLCOLOR
    dw 001aH
    db 'WINI'           ; WM_WININICHANGE
    dw 001bH
    db 'DMOD'           ; WM_DEVMODECHANGE
    dw 001cH
    db 'ACAP'           ; WM_ACTIVATEAPP
    dw 001dH
    db 'FONT'           ; WM_FONTCHANGE
    dw 0081H
    db 'NCCR'           ; WM_NCCREATE
    dw 0082H
    db 'NCDE'           ; WM_NCDESTROY
    dw 0083H
    db 'NCCA'           ; WM_NCCALCSIZE
    dw 0084H
    db 'NCHT'           ; WM_NCHITTEST
    dw 0085H
    db 'NCPA'           ; WM_NCPAINT
    dw 0086H
    db 'NCAC'           ; WM_NCACTIVATE
    dw 0087H
    db 'DLGC'           ; WM_GETDLGCODE
    dw 00a0H
    db 'NCMV'           ; WM_NCMOUSEMOVE
    dw 00a1H
    db 'NCLD'           ; WM_NCLBUTTONDOWN
    dw 00a2H
    db 'NCLU'           ; WM_NCLBUTTONUP
    dw 00a3H
    db 'NCLC'           ; WM_NCLBUTTONDBLCLK
    dw 00a4H
    db 'NCRD'           ; WM_NCRBUTTONDOWN
    dw 00a5H
    db 'NCRU'           ; WM_NCRBUTTONUP
    dw 00a6H
    db 'NCRC'           ; WM_NCRBUTTONDBLCLK
    dw 00a7H
    db 'NCMD'           ; WM_NCMBUTTONDOWN
    dw 00a8H
    db 'NCMU'           ; WM_NCMBUTTONUP
    dw 00a9H
    db 'NCMC'           ; WM_NCMBUTTONDBLCLK
    dw 0100H
    db 'KYDN'           ; WM_KEYDOWN
    dw 0101H
    db 'KYUP'           ; WM_KEYUP
    dw 0102H
    db 'CHAR'           ; WM_CHAR
    dw 0103H
    db 'DCHR'           ; WM_DEADCHAR
    dw 0104H
    db 'SYKD'           ; WM_SYSKEYDOWN
    dw 0105H
    db 'SYKU'           ; WM_SYSKEYUP
    dw 0106H
    db 'SYCH'           ; WM_SYSCHAR
    dw 0107H
    db 'SYDC'           ; WM_SYSDEADCHAR
    dw 0108H
    db 'YOMI'           ; WM_YOMICHAR
    dw 0109H
    db 'MVCW'           ; WM_MOVECONVERTWINDOW
    dw 010AH
    db 'CORQ'           ; WM_CONVERTREQUEST
    dw 010BH
    db 'CORE'           ; WM_CONVERTRESULT
    dw 0110H
    db 'IDLG'           ; WM_INITDIALOG
    dw 0111H
    db 'CMMD'           ; WM_COMMAND
    dw 0112H
    db 'SCMD'           ; WM_SYSCOMMAND
    dw 0113H
    db 'TIME'           ; WM_TIMER
    dw 0114H
    db 'HSCR'           ; WM_HSCROLL
    dw 0115H
    db 'VSCR'           ; WM_VSCROLL
    dw 0116H
    db 'IMNU'           ; WM_INITMENU
    dw 0117H
    db 'IPOP'           ; WM_INITMENUPOPUP
    dw 0118H
    db 'SYTM'           ; WM_SYSTIMER
    dw 0200H
    db 'MOUS'           ; WM_MOUSEMOVE
    dw 0201H
    db 'LBDN'           ; WM_LBUTTONDOWN
    dw 0202H
    db 'LBUP'           ; WM_LBUTTONUP
    dw 0203H
    db 'LBCL'           ; WM_LBUTTONDBLCLK
    dw 0204H
    db 'RBDN'           ; WM_RBUTTONDOWN
    dw 0205H
    db 'RBUP'           ; WM_RBUTTONUP
    dw 0206H
    db 'RBCL'           ; WM_RBUTTONDBLCLK
    dw 0207H
    db 'MBDN'           ; WM_MBUTTONDOWN
    dw 0208H
    db 'MBUP'           ; WM_MBUTTONUP
    dw 0209H
    db 'MBCL'           ; WM_MBUTTONDBLCLK
    dw 0300H
    db 'CUT '           ; WM_CUT
    dw 0301H
    db 'COPY'           ; WM_COPY
    dw 0302H
    db 'PAST'           ; WM_PASTE
    dw 0303H
    db 'CLER'           ; WM_CLEAR
    dw 0304H
    db 'UNDO'           ; WM_UNDO
    dw 0305H
    db 'RFMT'           ; WM_RENDERFORMAT
    dw 0306H
    db 'RAFT'           ; WM_RENDERALLFORMATS
    dw 0307H
    db 'DECL'           ; WM_DESTROYCLIPBOARD
    dw 0308H
    db 'DRCL'           ; WM_DRAWCLIPBOARD
    dw 0309H
    db 'PACL'           ; WM_PAINTCLIPBOARD
    dw 030aH
    db 'VSCL'           ; WM_VSCROLLCLIPBOARD
    dw 030bH
    db 'SICL'           ; WM_SIZECLIPBOARD
    dw 030cH
    db 'CBFT'           ; WM_ASKCBFORMATNAME
    dw 030dH
    db 'CBCH'           ; WM_CHANGECBCHAIN
    dw 030eH
    db 'HSCL'           ; WM_HSCROLLCLIPBOARD
    dw 0400H
    db 'USER'           ; WM_USER
    dw 0, 0, 0          ; End of table (570 bytes)


; %%Function:ShowMsg %% Owner:BRADV
cProc ShowMsg, <PUBLIC, FAR>, <si, di>
    parmDP szMsg
    parmW hwnd
    parmW message
    parmW wParam
    parmD lParam

    LocalV sz, 28h
cBegin
        mov ax, vmsgShowExclude
        or ax, ax                       ; Check for an excluded message
        jz allSM
        cmp ax, message                 ; Check if numbers match
        jnz allSM
        jmp endSM                       ; Don't display this message
allSM:  lea si, sz                      ; Copy in id of message
        mov ch, iMsg
        or ch, ch
        jnz oneSM                       ; CrLf on first message
        mov ax, 0a0dh
        mov [si], ax
        add si, 2

oneSM:  mov di, szMsg
        mov ax, [di]
        mov [si], ax
        mov al, ' '
        mov [si+2], al
        mov ax, 771bh                   ; Esc-w for no autowrap
        mov [si+3], ax
        add si, 5                       ; Move string pointer

          ; Put in numbers
        mov ax, hwnd
        mov dx, 2020h                   ; Fill space, terminate space
        call IntToHex
        inc si
        mov ax, message
        call MapMsg
        jc trnsSM
        call IntToHex
trnsSM: inc si
        mov ax, wParam
        call IntToHex
        inc si
        mov dx, 20ffh                   ; Space fill, hiword of a long
        mov ax, SEG_lParam              ; Get hiword of lParam
        call IntToHex
        mov dl, 0                       ; Null terminate.
        mov ax, OFF_lParam              ; Get loword
        call IntToHex

        cmp ch, 2
        jz twoSM                        ; All but last message get trailing #
        mov ax, 0023h
        mov [si], ax
twoSM:  lea si, sz
        cCall WriteToAux                ; Show it

          ; iMsg = (iMsg + 1) % 3
        inc ch                          ; Increment count of messages on line
        cmp ch, 3                       ; Test if overflow (past 0, 1, 2)
        jnz doneSM                      ; Go and save it
        xor ch, ch
        jmp doneSM


doneSM: mov iMsg, ch
endSM:
cEnd


; IntToHex
;  converts the number in ax into the hex digits it represents.
;  The string is written from si to si+4 with dl as the terminator.
;  There is a special case for displaying longs so that 0 will be represented
;  correctly.
;  The side effects are the string being filled and si being incremented
;  by four.

IntToHex:
        push cx
        push di
        push bp
        push si
        mov bx, 4
        mov di, dx              ; Save current fill characters
        cmp dx, 20ffh           ; Check if displaying first word of long
        jnz oneIH
        mov dh, 30h
        or ax, ax               ; If First word of long and a zero
        jnz oneIH               ;   then just fill with fill characters
        mov dh, 20h             ; Next word will be space filled.
        push dx                 ; Save it for the caller
        jmp fill                ; Fill this one
oneIH:  push dx                 ; Save it for the caller
        mov dx, di              ; Get back the current fill characters
        mov di, ax              ; Save number in a register
        mov [bx+si], dl         ; Put in terminating character
        mov cl, 4               ; shift count

loopHx: mov ax, di
        dec bx
        and al, 0fh             ; Mask off lower nybble
        cmp al, 0ah
        jl decml                ; Skip additional offset if 0-9
        add ax, 'a' - '0' - 0ah
decml:  add ax, '0'
        mov [bx+si], al         ; Save the character
        shr di, cl              ; Shift to next nybble
        jnz loopHx

fill:   or bx, bx
        jz doneIH               ; Quit if no more spaces in string
        dec si                  ; Make bx a count, not an offset
loopFl: mov [bx+si], dh
        dec bx
        jnz loopFl
doneIH: pop dx                  ; Get next fill characters
        pop si
        add si, 4               ; Increment string pointer
        pop bp
        pop di
        pop cx
        ret


; MapMsg
;  takes the number in ax to be the current message and trys to match
;  it with a string from the table.  If successful, the string will be
;  added and terminated with the character in dl.  The carry flag will
;  be set if the message translation was successful.

MapMsg:
        push ds
        push cs
        pop  ds
        lea bx, msgMap          ; Get address of start of table
        pop  ds

loopMM:
        cmp cs:[bx], ax             ; Test for matching message number
        jz copyMM                   ; Copy the text.
        add bx, 6                   ; Six bytes to the next message
        cmp word ptr cs:[bx+2], 0   ; Null message text ends table
        jnz loopMM

failMM: clc                     ; Failed
        ret

copyMM: push ax
        mov ax, cs:[bx+2]       ;  Copy text
        mov [si], ax            ;  into the string.
        mov ax, cs:[bx+4]
        mov [si+2], ax
        mov [si+4], dl          ; Terminate
        add si, 4
        pop ax
        clc
        cmc                     ; Set the carry
        ret


; %%Function:WriteToAux %% Owner:BRADV
cProc WriteToAux, <NEAR>, <si, di, dx, bp>
cBegin
loopWA: mov dl, [si]            ; Get the character
        or dl, dl               ; Set flags
        jz doneWA               ; Null character marks end
	mov ah,4
	int 21h
        inc si                  ; Move to next character
        jmp loopWA              ; Loop until a null is found
doneWA:
cEnd

; %%Function: %% Owner:BRADV
cProc AuxOut, <FAR, PUBLIC>, <si>
parmDP <sz>
cBegin
        mov     si, sz          ; Put the parameter to WriteToAux in si
        cCall   WriteToAux      ; Then show it.
cEnd

; %%Function:LongToSz %% Owner:BRADV
cProc LongToSz, <FAR, PUBLIC>, <si, di>
parmD  <Long>
parmDP <sz>
cBegin
        mov     si, sz          ; Put the pointer to output buffer.
        mov     dx, 30ffh       ; '0' fill, hiword of a long
        mov     ax, SEG_Long
        call    IntToHex
        mov     dl, 0           ; Null terminate.
        mov     ax, OFF_Long    ; Get loword
        call    IntToHex
cEnd

sEnd    CODE

        END
