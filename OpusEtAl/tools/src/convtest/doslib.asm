        page    58,82

        TITLE   doslib - DOS access library routines
; =====================================================================
;   This file contains DOS access routines.
;   These routines are general, simple calls to DOS and
;   are likely to be generally useful.
;   FAR calls are used throughout
; =====================================================================

        ?MEDIUM = 1
        ?WIN    = 1
        ?PLM    = 0

        .xlist
	include cmacros.inc
        .list

createSeg       DOSLIB_TEXT, nres, byte, public, CODE

cchMaxFile  EQU     66
EXTRN   ANSITOOEM:FAR

sBegin      nres
        assumes cs,nres
        assumes ds,DATA
        assumes ss,DATA


 
;-----------------------------------------------------------------------------
; int FAR FCloseDoshnd( doshnd )
;
; Close file given DOS handle, return 0 = error, nonzero = no error
;-----------------------------------------------------------------------------

cProc FCloseDoshnd, <FAR, PUBLIC>
parmW   <doshnd>
cBegin FCloseDoshnd

    mov     bx,doshnd
    mov     ah,3eh

    int     21h
    mov     ax,0000
    jc      cdhskip     ; error, leave a zero in ax
    inc     ax
cdhskip:
cEnd FCloseDoshnd


;-----------------------------------------------------------------------------
; int CchReadDoshnd ( doshnd, lpchBuffer, bytes )
;
; Read bytes from an open file, place into buffer
; Returns # of bytes read (should be == bytes unless EOF or error)
; If an error occurs, returns the negative of the error code
;-----------------------------------------------------------------------------

cProc CchReadDoshnd, <FAR, PUBLIC>, <DS>
parmW   <doshnd>
parmD   <lpchBuffer>
parmW   <bytes>
cBegin CchReadDoshnd

    mov     bx,doshnd
    lds     dx,lpchBuffer
    mov     cx,bytes
    mov     ah,3fh

    int     21h
    jnc     crdone

    neg     ax          ; error - return value is the negative of the error code
crdone:
cEnd CchReadDoshnd




;-----------------------------------------------------------------------------
; int CchWriteDoshnd ( doshnd, lpchBuffer, bytes )
;
; Write bytes from an open file, place into buffer
; Returns # of bytes written (should be == bytes unless EOF or error)
; If an error occurs, returns the negative of the error code
; Disk full is not an "error"; detect it by return code != bytes
;-----------------------------------------------------------------------------

cProc CchWriteDoshnd, <FAR, PUBLIC>,<DS>
parmW   <doshnd>
parmD   <lpchBuffer>
parmW   <bytes>
cBegin CchWriteDoshnd

    mov     bx,doshnd
    lds     dx,lpchBuffer
    mov     cx,bytes
    mov     ah,40h

    int     21h
    jnc     cwdone

    neg     ax              ; error: return the negative of the error code
cwdone:

cEnd CchWriteDoshnd




;-----------------------------------------------------------------------------
; long DwSeekDw ( doshnd, dwSeekpos, bSeekfrom )
;
; Seek to requested position in file
; bSeekfrom is:  0 = seek relative to beginning of file
;                1 = seek relative to current pointer location
;                2 = seek relative to end of file
;
; Returns the new location of the read/write pointer (a long)
; If an error occurs, returns the negative of the error code (long)
;-----------------------------------------------------------------------------

cProc DwSeekDw, <FAR, PUBLIC>
parmW   <doshnd>
parmD   <dwSeekpos>
parmB   <bSeekfrom>
cBegin DwSeekDw

    mov     bx,doshnd
    mov     cx,SEG_dwSeekpos
    mov     dx,OFF_dwSeekpos
    mov     al,bSeekfrom
    mov     ah,42h

    int     21h
    jnc     seekdone

    neg     ax              ; Error: return the negative of the error code
    mov     dx,0ffffH

seekdone:

cEnd DwSeekDw


sEnd        nres

            END
