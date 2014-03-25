        TITLE   when.asm- DOS access library routines
; =====================================================================
;   This file contains DOS access routines.
;   These routines are general, simple calls to DOS and
;   are likely to be generally useful.  They assume /PLM calling
;   is used.
;   FAR calls are used throughout
;
;
; =====================================================================


        .xlist
        memS    = 1
        ?WIN    = 0
        ?PLM    = 1
        ?NODATA = 1
        ?TF     = 1
        include cmacros.inc
        .list


sBegin  CODE
        assumes CS,CODE

;-----------------------------------------------------------------------------
; void OsTime( pTime )
; struct TIME *ptime;
;
;   pTime is a pointer to a structure of the form:
;       struct {
;               char min;       Minutes (0-59)
;               char hour;      Hours (0-23)
;               char hsec;      Hundredths of seconds (0-99)
;               char sec;       Seconds (0-59)
;              }
;
;   Get current time into structure
;-----------------------------------------------------------------------------

cProc OsTime, <FAR, PUBLIC>
parmDP  <pTime>
cBegin OsTime

    mov     ah,2ch
    int     21h

    mov     bx,pTime
    mov     WORD PTR [bx], cx
    mov     WORD PTR [bx+2], dx

cEnd OsTime


;-----------------------------------------------------------------------------
; void OsDate( pDate )
;
;   pDate is a pointer to a structure of the form:
;       struct {
;               int  year;      Year (1980 - 2099)
;               char month;     Month (1-12)
;               char day;       Day (1-31)
;               char dayOfWeek; Week day (0 = Sunday, 6 = Saturday)
;              }
;
;   Get current date into structure
;-----------------------------------------------------------------------------

cProc OsDate, <FAR, PUBLIC>
parmDP  <pDate>
cBegin OsDate

    mov     ah,2ah
    int     21h

    mov     bx,pDate
    mov     WORD PTR [bx], cx     ; year
    mov     BYTE PTR [bx+2], dh   ; month
    mov     BYTE PTR [bx+3], dl   ; day
    mov     BYTE PTR [bx+4], al   ; week day

cEnd OsDate


sEnd    CODE

        END

