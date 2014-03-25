        page    58,82

        TITLE   entrypt - Entry points from Opus

?PLM    = 1
?WIN    = 1
?MEDIUM = 1

        .xlist
include cmacros.inc
        .list



cwStack equ     2048
cbHeap  equ     40960


public  _hStackCaller
public  _hwndCaller

externFP        <LocalInit, HstackMyData, CreateStack>


sBegin DATA

Stack           dw      cwStack dup(?)
StackMax        label word
StackEnd        equ     dataOffset StackMax - 2

fHeapInit       db      0
_hStackCaller   dw      ?
_hwndCaller     dw      ?

sEnd   DATA

createSeg ENTRYPT_TEXT,nres,byte,public,CODE
sBegin nres
assumes cs, nres
assumes ds, DATA



; Initializes library local heap.
;
; Exit:  ax = 0 if error
;

cProc   HeapInit,<FAR,PUBLIC>,<si,di>
cBegin  HeapInit
        cmp     [fHeapInit],0
        jnz     HeapInitExitOK          ; just return if already set up

        mov     [fHeapInit],1
        mov     cx,cbHeap
        xor     ax,ax
        cCall   LocalInit,<ax,ax,cx>
        xor     ax,ax
        jcxz    HeapInitExit            ; Fail if no heap

HeapInitExitOK:
        mov     ax,1

HeapInitExit:
cEnd    HeapInit


; InitConverter(hStack, hwnd)
cProc InitConverter, <FAR, PUBLIC>, <>
        parmW   <hStack>
        parmW   <hwnd>

cBegin
        mov     ax,[hStack]
        mov     _hStackCaller, ax
        cCall   HeapInit
        cCall   HstackMyData
        push    ax                      ; save for return
        mov     bx,StackEnd
        cCall   CreateStack, <ax, bx>
	mov	ax,[hwnd]		; save away the owner
	mov	_hwndCaller,ax
        pop     ax
cEnd

sEnd    nres

        END

