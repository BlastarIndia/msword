        TITLE   profctl - profiler on/off control routines

include w2.inc
include cmacros.inc

ifdef PAUSEPROFILER
sBegin DATA
cBackTrace      dw 1
sEnd
endif ;PAUSEPROFILER

sBegin  CODE

assumes cs,CODE
assumes ds,DATA
assumes es,NOTHING

; %%Function:StartProfiler %%Owner:BRADV
cProc   StartProfiler,<PUBLIC, FAR>
ParmW   cFrames
ParmD   lpszFilename
cBegin
        push    seg_lpszFilename
        push    off_lpszFilename

        mov     ax,cFrames
ifdef PAUSEPROFILER
        mov     cBacktrace,ax
endif ;PAUSEPROFILER

        push    ax
        mov     ax,30                       ; start command
        push    ax
        call    StartStopProf
        add     sp,8
cEnd

ifdef PAUSEPROFILER
; %%Function:PauseProfiler %%Owner:BRADV
cProc   PauseProfiler,<PUBLIC, FAR>
ParmW   fPause
cBegin
        xor     dx,dx
        cmp     fPause,dx
        jnz     pp100

        push    dx
        push    dx
        push    cBacktrace              ; last backtrace value
        mov     ax,30                   ; start = 30
        push    ax
        call    StartStopProf
        add     sp,8
        jmp     short pp200

pp100:
        mov     ax,32                   ; pause = 32
        push    ax
        call    StartStopProf
        add     sp,2
pp200:
cEnd
endif ;PAUSEPROFILER

; %%Function:StopProfiler %%Owner:BRADV
cProc   StopProfiler,<PUBLIC, FAR>
cBegin
        mov     ax,31                   ; 31 = stop
        push    ax
        call    StartStopProf
        add     sp,2
cEnd

StartStopProf:
        pop     ax
        push    cs
        push    ax

        xor     ax,ax
        mov     es,ax
        mov     ax,es:[000eh]              ; Get segment down there.
        mov     es,ax

        cmp     word ptr es:[0100h],'S' + 'E' * 256
        jnz     sspexit

        cmp     word ptr es:[0109h],'P' + 'R' * 256
        jnz     sspexit

        jmp     dword ptr es:[00fch]       ; go to the profiler.

sspexit:
        ret

sEnd    CODE

        END
