?PLM = 1
?NODATA = 1
.xlist
    include w2.inc
	include cmacros.inc
.list

createSeg	_FIXED,prof,byte,public,CODE
createSeg	_DATA,data,word,public,DATA,DGROUP
defGrp		DGROUP,DATA

PROF_INT	equ	0FBh
PROF_INT_ADDR	equ	4 * PROF_INT

do_prof_int	macro	function
	local	not_installd
	cmp	installed,0
	jz	not_installd
	mov	ah,function
	int	PROF_INT
not_installd:
	mov	ax,installed
	endm



sBegin	prof
assumes cs,prof
	assume	ds:nothing
	assume	es:nothing

ifdef DEBUG
;-----------------------------------------------------------------------;
; hello_profiler							;
; 									;
; Checks if profiler is installed.  Int 3 and prof_int will point to	;
; the same segment if it is installed.					;
; 									;
; Arguments:								;
; 	none								;
; Returns:								;
; 	TRUE if installed						;
; 									;
;  Mon Aug 25, 1986 05:57:03p	-by-	Wesley O. Rupel	  [wesleyr]	;
;-----------------------------------------------------------------------;
	assume	ds:nothing
	assume	es:nothing

installed	dw	0

; %%Function:hello_profiler %%Owner:BRADV
cProc   hello_profiler,<FAR, PUBLIC>,<ds>
cBegin
	xor	ax,ax
	mov	ds,ax
	mov	ax,ds:[2 + 4 * 3]
	cmp	ax,ds:[2 + PROF_INT_ADDR]
	mov	ax,0			; xor would alter flags
	jnz	not_installed
	inc	ax
not_installed:
	mov	cs:installed,ax
cEnd


;-----------------------------------------------------------------------;
; start_recording							;
; stop_recording							;
; clear_buffers								;
; write_buffers								;
; swap_int_9								;
;									;
; Does nothing if hello_profiler has not been called and returned TRUE.	;
;									;
; Arguments:								;
;	None								;
; Returns:								;
;	TRUE if profiler installed and hello_profiler has been done.	;
; Alters:								;
;	AX								;
;									;
;  Mon Aug 18, 1986 02:13:09p	-by-	Wesley O. Rupel   [wor] 	;
;-----------------------------------------------------------------------;

; %%Function:start_recording %%Owner:BRADV
cProc   start_recording,<FAR, PUBLIC>,<>
cBegin
	do_prof_int	0
cEnd

;-----------------------------------------------------------------------;

; %%Function:stop_recording %%Owner:BRADV
cProc   stop_recording,<FAR, PUBLIC>,<>
cBegin
	do_prof_int	1
cEnd

;-----------------------------------------------------------------------;

; %%Function:clear_buffers %%Owner:BRADV
cProc   clear_buffers,<FAR, PUBLIC>,<>
cBegin
	do_prof_int	2
cEnd

;-----------------------------------------------------------------------;

; %%Function:write_buffers %%Owner:BRADV
cProc   write_buffers,<FAR, PUBLIC>,<>
cBegin
	do_prof_int	3
cEnd

;-----------------------------------------------------------------------;

; %%Function:set_timer_rate %%Owner:BRADV
cProc   set_timer_rate,<FAR, PUBLIC>,<>
	ParmB	timer_rate
cBegin
	mov	al,timer_rate
	do_prof_int	6
cEnd

;-----------------------------------------------------------------------;
; get_buffer_location							;
;									;
; Arguments:								;
;	None								;
; Returns:								;
;	DX:AX = FAR PTR to CS:IP buffer.				;
; Alters:								;
;	AX								;
;									;
;  Mon Aug 18, 1986 02:13:09p	-by-	Wesley O. Rupel   [wor] 	;
;-----------------------------------------------------------------------;

; %%Function:get_buffer_location %%Owner:BRADV
cProc   get_buffer_location,<FAR, PUBLIC>,<>
cBegin
	cmp	installed,0
	jz	not_nstalled
	mov	ah,4
	int	PROF_INT
not_nstalled:
cEnd


	public	calibrate_prof
;-----------------------------------------------------------------------;
; calibrate_prof							;
; 									;
; This is a good routine to profile to verify that the profiler is	;
; working correctly.  The number of hits should be proportional to the	;
; label number.  Thus prof_label_6 should get 6 times as many hits as	;
; prof_label_1.								;
; Allowing other interrupts to occur while this is being profiled would	;
; throw off the exact timing, thus they will be masked out.		;
; 									;
; Arguments:								;
; 	none								;
; Returns:								;
; 	nothing								;
; Alters:								;
; 	nothing								;
; Calls:								;
; 	disable_other_ints						;
; 	enable_other_ints						;
; History:								;
; 									;
;  Tue Oct 14, 1986 06:52:32p	-by-	Wesley O. Rupel	  [wesleyr]	;
; Wrote it!								;
;-----------------------------------------------------------------------;

n_loops	macro	number
; %%Function:prof_label_&number %%Owner:BRADV
	public	prof_label_&number
prof_label_&number&:
	rept	number
	loop	$
	endm
	endm

rest	macro
	jmp	$ + 2
	endm

	assume	ds:nothing
	assume	es:nothing

; %%Function:calibrate_prof %%Owner:BRADV
cProc   calibrate_prof,<FAR, PUBLIC>,<cx>
cBegin
	call	disable_other_ints

	count	= 0
	xor	cx,cx

	rept	9		; 9 is about right for prof -t4 (on 8Mhz AT)
	count = count + 1
	n_loops	%count
	endm

	public	end_loop_of_loops
end_loop_of_loops:
	call	enable_other_ints
cEnd


; %%Function:disable_other_ints %%Owner:BRADV
public	disable_other_ints
;-----------------------------------------------------------------------;
; disable_other_ints							;
; 									;
;  Wed Oct 15, 1986 09:55:23a	-by-	Wesley O. Rupel	  [wesleyr]	;
; Wrote it!								;
;-----------------------------------------------------------------------;
	assume	ds:nothing
	assume	es:nothing

master_state	db	?
slave_state	db	?

disable_other_ints      proc    near
	cli
	push	ax
	in	al,021h
	rest
	mov	cs:master_state,al
	in	al,0A1h
	rest
	mov	cs:slave_state,al

	mov	al,0FBh		; allow only IRQ 2 on master 8259
	out	21h,al
	rest
	mov	al,0FEh		; allow only IRQ 0 on slave 8259
	out	0A1h,al
	pop	ax
	sti
	ret
disable_other_ints	endp


; %%Function:enable_other_ints %%Owner:BRADV
public	enable_other_ints
;-----------------------------------------------------------------------;
; enable_other_ints							;
; 									;
;  Wed Oct 15, 1986 10:05:45a	-by-	Wesley O. Rupel	  [wesleyr]	;
; Wrote it!								;
;-----------------------------------------------------------------------;
	assume	ds:nothing
	assume	es:nothing

enable_other_ints	proc	near
	push	ax
	cli
	mov	al,cs:master_state
	out	21h,al
	rest
	mov	al,cs:slave_state
	out	0A1h,al
	pop	ax
	sti
	ret
enable_other_ints	endp

endif                   ; DEBUG for whole module
sEnd	code
	end
