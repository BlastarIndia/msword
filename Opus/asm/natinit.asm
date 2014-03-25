	include w2.inc
	include noxport.inc
        include windows.inc
        .list

createSeg	initwin_PCODE,natinit,byte,public,CODE

ifdef HYBRID
externFP	<DeleteDCPR>
else ;!HYBRID
externFP	<DeleteDC>
endif ;!HYBRID
externFP	<GetWindowDC, LocalAlloc, LocalFree, GetVersion>
externFP	<GetCurPID, GetNumTasks>

externA		__ahshift

sBegin	data
externW                 vwWinVersion
externW                 vfLargeFrameEMS
externW                 vfSmallFrameEMS
externW                 vfSingleApp
externW					vcbitSegmentShift
sEnd	data

sBegin	natinit
	assumes cs,natinit
	assumes ds,dgroup
	assumes ss,dgroup

        ;/* **** +-+utility+-+string **** */
;-----------------------------------------------------------------------------
; FillBytePattern( pb, rgchPat, cchPat, cb ) - fills pb with cb bytes of
;               repeating pattern given by { rgchPat, cchPat }
;-----------------------------------------------------------------------------

;%%Function:FillBytePattern %%Owner:PETERJ 
cProc FillBytePattern, <FAR, PUBLIC,ATOMIC>, <DI,SI>
parmDP  <pb>
parmW   <rgchPat>
parmW   <cchPat>
parmW   <cb>
        ;/* **** -+-utility-+- **** */

cBegin FillBytePattern

        mov ax,ds               ; we are filling in the data segment
        mov es,ax
        cld                     ; the operation is forward
        mov di,[pb]
        cmp [cchPat],1          ; interesting, faster, special case
        jz LFBP0                ; requires di = pb
LFBP2:  mov ax,[cchPat]
        mov bx,[cb]
        sub ax,bx               ; tricky min
        cwd
        and ax,dx
        add ax,bx
        mov cx,ax               ; cx = min( cchPat, cb );
        mov si,[rgchPat]        ; pchPat = rgchPat
        sub [cb],cx             ; cb -= cx
        rep movsb               ; while (cx--) *pb++ = *pchPat++
        cmp [cb],0              ; if (cb != 0) goto LFBP2
        jne LFBP2
        jmp LFBP1
        ; requires di = pb
LFBP0:
        mov     si,[rgchPat]    ; while (cb--) *pb++ = rgchPat[0];
        mov     al,[si]
        mov     cx,[cb]
        rep     stosb
LFBP1:
cEnd FillBytePattern

ifdef HYBRID
DeleteDCAddr	dd	DeleteDCPR
else ;!HYBRID
DeleteDCAddr	dd	DeleteDC
endif ;!HYBRID
GetWindowDCAddr dd	GetWindowDC


;%%Function:GlobalMemInit %%Owner:PETERJ
cProc	GlobalMemInit,<PUBLIC,FAR>,<si>
cBegin
;
;  Try to grow GDI and USERs local heap so global wires that we
;  do later are more likely to be effective.
;
	lds	bx,cs:[DeleteDCAddr]
	mov	ax,1000H
	mov	cx,0A40H
	call	GrowHeap
	lds	bx,cs:[GetWindowDCAddr]
	mov	ax,3420H
	mov	cx,0010H
	call	GrowHeap
	push	ss
	pop	ds
cEnd

GrowHeap:
	cmp	byte ptr [bx],0b8h	; look for mov ax, ....
	jnz	lax
	mov	ds,[bx+1]		; set DS to GDIs DS
	mov	bx,LMEM_FIXED + LMEM_NODISCARD + LMEM_NOCOMPACT
	push	bx
	push	cx
	cCall	LocalAlloc,<bx,ax>
	mov	si,ax
	cCall	LocalAlloc
	or	ax,ax
	jz	la1
	cCall	LocalFree,<ax>
la1:	or	si,si
	jz	lax
	cCall	LocalFree,<si>
lax:	ret


;-----------------------------------------------------------------------------
; BOOL FNativeInit
;
; Perform init tasks best done in native
;-----------------------------------------------------------------------------

; %%Function:FNativeInit %%Owner:PETERJ
cProc FNativeInit, <FAR, PUBLIC>,<si,di>
cBegin FNativeInit
    cCall   GetNumTasks,<>      ; see if MS-DOS Exec is running
    mov     bx,1
    cmp     ax,bx
    je      FNIsingleapp        ; opus is only app
    dec     bx                  ; some other app is running (esp MS-DOS)
FNIsingleapp:
    mov     [vfSingleApp],bx
	mov		[vcbitSegmentShift],__ahshift
    mov     ah,30h              ; get DOS version
    int     21h
    cmp     al,3                ; see if it is the wrong version (< 3)
    jl      FNIfail

    cCall   GetVersion,<>       ; get WIN version        
    xchg    al,ah               ; make non bogus
    mov     [vwWinVersion],ax
    cmp     ax,00203h           ; see if it is the wrong version (< 2.03)
    jl      FNIfail             ; NOTE: documentation says 2.10, 
                                ; so we can bump this up if necessary

    xor     ax,ax               ; see if windows is using LIM 4.0
    cCall   GetCurPID,<ax,ax>   ; puts EMSFlags in dx
    test    dx,02h              ; EMSF_LIM_40
    jz      FNIok
                                ; kernel is using LIM4.0
                                ; is it using small frame or large frame?
    test    dx,08h              ; EMSF_DATA_ABOVE
    jz      FNIsmall
    mov     [vfLargeFrameEMS],1 ; large frame
    jmp     FNIok
FNIsmall:
    mov     [vfSmallFrameEMS],1 ; small frame

FNIok:
    mov     ax,1                ; return success
FNIret:
cEnd FNativeInit

FNIfail:
    xor     ax,ax               ; return failure
    jmp     FNIret


sEnd	natinit
	end
