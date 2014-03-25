
.xlist
include	grep.mac
include	grep.hm
include word.inc

ifdef OPUS
externFP 	<AnsiUpper>
externFP	<AnsiLower>
externFP	<FUpper>
externFP	<FLower>

createSeg       docman2_PCODE,CODE,byte,public,CODE

else
externNC 	ChUpper
externNC	ChLower
externNC	isupper
externNC	islower

endif
.list
;------------------------------------------------------------------------
;	PUBLICS
;------------------------------------------------------------------------
	PUBLIC	read_in_file, suck_in_file, bgrep, gimme_a_piece_a_buff
	PUBLIC	bgrep_for_expression, make_the_bitmaps
	PUBLIC	build_bitmap, get_RE_char, put_bit, test_bit
ifdef NOT_USED
	PUBLIC	 free_bytes
endif
	PUBLIC  allocmem

;externFP CchReadOsfnLp
;externFP FSetFilePointer
externFP DwSeekDw
externFP CchReadDoshnd

;------------------------------------------------------------------------
;	The Word data segment. The strings to search for are here as well
;------------------------------------------------------------------------

sBegin	DATA
ifdef NOT_USED
previous_free_memory_off	dw	0
previous_free_memory_size	dw	0	; bytes now, not paras
endif
	pword	input_file_handle
	pword	first_bitmap_off
	pword	free_memory_off
	pword	free_memory_size	; bytes now, not paras
	pword	current_bitmap_off
	pword	bit_flags
	pdword	fcFirst
	pdword	cfcSearch
	pword	file_buffer_off
	pword	file_buffer_size
	pword	pStrings
	pword	cStrings
	pword	rgchResult
	pword	abort_stack_ptr
	pword	buffer_seg	; in case we need to restore a seg for BUFFER

sEnd	DATA

;------------------------------------------------------------------------
;	This is a fake segment. All references to variables in the
;	buffer passed from word are prefixed with BUFFER: , and at
;	least one of the segment registers better point here then!
;------------------------------------------------------------------------
createSeg buffer, BUFFER, at, 0
sBegin	BUFFER
sEnd	BUFFER

ifdef REVIEW
externFP	<DOSREAD, DOSCHGFILEPTR>

externNC 	ChUpper
externNC	ChLower
externNC	isupper
externNC	islower
endif 

sBegin	CODE
	assumes	cs, CODE 
	assumes	ss, DATA
	assumes	ds,DATA
	assumes	es,nothing


farPtr		ss_bx, ss, bx
farPtr		ss_ax, ss, ax

bitmap_of_special_chars		label	byte
	db	0,0,0,0,10h,22h,0,0,0,0,0,0,0,0,0,0		;#*.

	rept	16
		db 0
	endm

;-----------------------------------------------------------------------
;  This module thoroughly hacked, 
;  1. 12/86 to extract from the DSR grep code
;  2. 6/88  rewritten to remove OS/2 incompatable segment kludges & speed hacks
;
;  Throughout this module, unless otherwise stated, the code has been 
;  modified to assume the following:
;	DS: 	Points to the buffer passed in from Word
;	ES: 			"
;	CS:	Points to code
;	SS:	Points to the word stack (and data) segment.
;  Offsets of various items into the buffer are stored, and
;  then accessed as DS:BX.foo
;-----------------------------------------------------------------------


;.............................................................................
;..............................PROGRAM..BEGINS................................
;.............................................................................

	assumes	ss,DATA
	assumes	ds,DATA
	assumes es,DATA
; %%Function:wordgrep %%Owner:chic
cProc	wordgrep,<PUBLIC, FAR>,<DI,SI>,,1
	ParmD	fpMemory		; far pointer to memory buffer
	ParmW	cbMemory		; number of bytes of memory
	ParmW	wpStrings		; pointer to array of near pointers
					; each of which points to a string
	ParmW	wcStrings		; number of strings in above array
	ParmW	wpResult		; pointer to array of chars in which
					; to put the results
	ParmW	handle			; handle of file to search.
	ParmW	wordflags		; case? 
	ParmD	fcWStart		; Where to start in file
	ParmD	cfc			; Where to end in file

cBegin	wordgrep
	mov	ax, handle
	mov	word ptr input_file_handle,ax
	mov	ax, wpStrings		; move these off the stack for
	mov	pStrings, ax		; 
	mov	ax, wcStrings		; global use
	mov	cStrings, ax		;
	mov	ax, wpResult
	mov	rgchResult, ax
	mov	bit_flags,DASH_Y_FLAG	; Assume case insensitive.
	test	wordflags,1
	jne	Case_insens
	mov	bit_flags, 0
Case_insens:
	mov	ax, SEG_fpMemory
	mov 	ds, ax
	assumes ds, BUFFER
	mov	buffer_seg, ax
	mov	bx,cbMemory
	mov	ax, SEG_fcWStart
	mov	word ptr fcFirst[2], ax
	mov	ax, OFF_fcWStart
	mov	word ptr fcFirst[0], ax
	mov	ax, SEG_cfc
	mov	word ptr cfcSearch[2], ax
	mov	ax, OFF_cfc
	mov	word ptr cfcSearch[0], ax
;------------------------------------save stack pointer for global aborts
	mov	abort_stack_ptr, sp
;------------------------------------------------------------------------
	xor	ax,ax
	mov	free_memory_off, ax 	; offset into BUFFER
	mov	free_memory_size, bx
;-----------------------------------------------BUILD-BITMAPS---------------

	call	make_the_bitmaps
	mov	ax, es	; make the bitmaps swapped es and ds
	mov	ds, ax
	mov	ax, ss
	mov	es, ax

;-----------------------------------Allocate-Buffers----------------
	call	allocmem
	mov	file_buffer_off,ax
	dec	bx			;leave room for crlf at eof
	dec	bx			;leave room for crlf at eof
	mov	file_buffer_size,bx

do_find_first:
;-----------------------------------Seek to starting position in file--
	call	seek_to_fcFirst
;----------------------------------------SUCK-IN-FILE------------------
get_more:
	mov	ax,file_buffer_off
	mov	bx,file_buffer_size
	mov	cx,input_file_handle

	call	read_in_file
	jc	end_of_file_buffer		;jmp if null file

;------------------------------------------BGREP--------------------------;
	call	bgrep
	mov	ax, ss	; bgrep set es=BUFFER
	mov	es, ax
;------------------------------------------------------------------------;
end_of_file_buffer:

	test	bit_flags,GET_MORE_OF_FILE_FLAG
	jz	the_finish
	jmp	get_more	;get more of this file
;------------------------------------------------------------------------;
the_finish:
;------------------------------------------------------------------------;
;	 return answer to word
;------------------------------------------------------------------------;
	xor	ax,ax		; 0 means everything is okay
	jmp	short return_to_word
ret_error_to_word:
;---------------------------------------------------------------------------
;	We can jump here from just about anywhere, so don't assume any of
;	the segment registers except cs and ss are correct
;--------------------------------------------------------------------------
	mov	sp, abort_stack_ptr
;	xor	ax,ax		; until we get some error codes
return_to_word:
	mov	bx,ss		; restore the word segment regs
	mov	ds,bx
	assumes	ds,data
	mov	es,bx
	assumes	es,data
cEnd	wordgrep		; time to go home.

;-----------------------------------------------------------------------;
; read_in_file								;
; 									;
; Sucks in file, lseeks as necessay, checks if it's null		;
; 									;
; Arguments:								;
;	ax	= file_buffer_off
;	bx	= file_buffer_size
;	cx	= input_file_handle
; 	DS 	= data buffer
; Returns:								;
; 	CF = 0  file not null						;
; 	  DS:DX = last file byte + 1					;
; 	CF = 1  file is null						;
; Alters:								;
; 	AX,BX,CX,DX,DS,SI,DI,BP						;
; Calls:								;
; 	suck_in_file							;
; 	lseek								;
; History:								;
; 									;
;  Thu Aug 22 14:33:49 1985    -by-    Wesley O. Rupel    [wesleyr]	;
; Amassed it								;
;-----------------------------------------------------------------------;
	assumes	es, DATA
	assumes	ds,BUFFER

read_in_file	proc	near
	call	suck_in_file		;Alters AX,BX,CX,DX,DS,SI,DI,BP

; DS:DX pts one byte past the last entry in file-buffer

	jnc	no_err
	jz	platos_retreat
	jmp	ret_error_to_word
platos_retreat:

; We fall thru to here if we did not get the whole file because of memry limits

	or	bit_flags,GET_MORE_OF_FILE_FLAG
	jmp	short	check_for_null_file
no_err:	
	and	bit_flags,not GET_MORE_OF_FILE_FLAG

;------------------------------------check-for-file-of-zero-length-----
check_for_null_file:
;	or	dx,dx		;DS:DX minus one = last file byte in buffer.
	cmp	dx, file_buffer_off
	jne	no_problem
	stc				;file is null
	ret
no_problem:
	clc
	ret
read_in_file	endp

;-----------------------------------------------------------------------;
; suck_in_file								;
; 									;
; Reads in file.  Returns DS:DX pointing one byte past the end of the	;
; file in the buffer.  If out-of-memory you may process what you've	;
; got and then simply call this routine again to get more.		;
; 									;
; Arguments:								;
; 	AX = buffer offset in DS						;
; 	BX = bytes of available memory				;
; 	CX = file handle						;
;	DS = data buffer segment
; Returns:								;
; 	CF = 0   if no error						;
; 	  DS:DX = end of region read in					;
; 	CF = 1   if error						;
; 	  ZF = 1  if out of memory					;
; 	  ZF = 0  if input read error					;
; Alters:								;
; 	AX, BX, CX, DX, DS, SI, DI, BP					;
; Calls:								;
; 	DOS function 3F							;
;	OS/2 DOSREAD
; History:								;
; 									;
;  Mon Jun 10 04:28:21 1985    -by-    Wesley O. Rupel    [wesleyr]	;
; Wrote it.								;
;-----------------------------------------------------------------------;
	assumes	ds, BUFFER
	assumes	es, DATA
	local_variables
local_word	cbRead
local_word	last_read

suck_in_file	proc	near
	
	push	bp
	mov	bp,sp
	add	sp,local_space
ifdef PROFILE
	cCall	StartNMeas,<>
endif ;PROFILE
	mov	dx,ax	; initialize dx to free mem buffer 
	mov	di,bx	; initialize di to # of bytes of available memory
	mov	bx,cx	; input file handle
	mov	word ptr last_read[bp],0	; zero low memory flag
	mov	cx,di		;	put number of bytes requested in cx

;-------------------------------------------
;	Make sure we don't go past cfcSearch
;-------------------------------------------

	cmp	word ptr cfcSearch[2], 0
	jnz	get_more_of_request	; more than 64K to go, fill whole buffer
	cmp	cx, word ptr cfcSearch[0]
	jna	get_more_of_request	; asking for <= amount to go
	mov	cx, word ptr cfcSearch[0]	; asking for more, reset to =
	or	cx,cx
	jz	end_of_file
	inc	word ptr last_read[bp]

get_more_of_request:
	mov	di, ds
	push	ds
	mov	ax, ss
	mov	ds, ax 				; pcode requires ss==ds
	farPtr	di_dx, di, dx
	save 	<bx, cx, dx>
	cCall	CchReadDoshnd,<bx, di_dx, cx>
	pop	ds
	cmp	ax, cx
	ja	error_reading_stdin
	or 	ax, ax
	jz	end_of_file			; if we got no chars then it
						; means EOF.
;-----------------------------------------------------
;	Subtract the amount we read from cfcSearch.
;-----------------------------------------------------
	sub	word ptr cfcSearch[0], ax
	jnc	check_for_all
	dec	word ptr cfcSearch[2] 		; borrow from high word

check_for_all:
	sub	cx,ax	; # of chars requested minus # received.
	jz	got_it_all
	assumes	ss,DATA
	add	dx,ax	; point dx to where next byte goes.
	jmp	get_more_of_request

got_it_all:
	dec word ptr last_read[bp]
	jnz insufficient__memory

end_of_file:		
	add	dx,ax	; Now  ds:dx  pts one byte past last char in buffer.
ifdef PROFILE
	cCall	StopNMeas,<>
endif ;PROFILE
	clc		; clc indicates no errors
	mov	sp,bp
	pop	bp
	ret

insufficient__memory:
	add	dx,ax	; Now  ds:dx  pts one byte past last char in buffer.
	xor	ax,ax	; set the zero flag.
ifdef PROFILE
	cCall	StopNMeas,<>
endif ;PROFILE
	stc		; carry and zero set signifies this error condition.
	mov	sp,bp
	pop	bp
	ret

error_reading_stdin:
ifdef PROFILE
	cCall	StopNMeas,<>
endif ;PROFILE
	or	ax,1	; set not zero.
	stc		; carry and not zero indicate this error.
	mov	sp,bp
	pop	bp
	ret

suck_in_file	endp


;-----------------------------------------------;
;	    End  of  suck_in_file		;
;-----------------------------------------------;


;-----------------------------------------------------------------------;
; bgrep									;
; 									;
; Arguments:								;
; 	ES = DATA_SEG							;
; 	DS:DX = file end + 1						;
; Returns:								;
; 									;
; Alters:								;
; 									;
; Calls:								;
;	gimme_a_piece_a_buff						;
; History:								;
; 									;
;  Thu Aug 22 16:19:36 1985    -by-    Wesley O. Rupel    [wesleyr]	;
; Ripped out of grep main						;
;-----------------------------------------------------------------------;
	assumes	es, DATA
	assumes	ds, BUFFER

bgrep	proc	near
	mov	bx, first_bitmap_off

	mov	cx, BUFFER:anchor_string_length_minus_one[bx]
	mov	si, BUFFER:anchor_string_offset[bx]
	mov	ax, file_buffer_off

	call	gimme_a_piece_a_buff

	ret

bgrep	endp


;-----------------------------------------------------------------------;
; gimme_a_piece_a_buff							;
; 									;
; Arguments:								;
; 	DS:bx  bitmap							;
; 	DS:ax  beginning of buffer.					;
; 	DS:DX  pts one byte past last entry in buffer			;
; Returns:								;
; 	Nothing								;
; Alters:								;
; 	ES, DS, AX, BX, CX, DX, SI, DI					;
; Calls:								;
; 	bgrep_for_expression						;
; History:								;
; 									;
;  Sun Aug 18 03:26:31 1985    -by-    Wesley O. Rupel    [wesleyr]	;
;-----------------------------------------------------------------------;
	local_variables
local_word	buffer_beg_offset
local_word	buffer_end_offset
local_word	bit_map_off
local_word	buffer_piece_end
local_word	number_of_bytes_to_lseek

	assumes	es,nothing
	assumes	ds,BUFFER	

gimme_a_piece_a_buff	proc	near

	push	bp
	mov	bp,sp
	add	sp,local_space
ifdef PROFILE
	cCall	StartNMeas,<>
endif ;PROFILE

	mov	word ptr number_of_bytes_to_lseek[bp],0
	mov	word ptr buffer_end_offset[bp],dx
	mov	word ptr buffer_beg_offset[bp],ax
	mov	cx, ds
	mov	es, cx
	assumes es, BUFFER

	mov	cx, BUFFER:index_of_string_this_bitmap_belongs_to[bx]
	inc	cx	; test for -1
	jz	previous_setting_is_most_significant	; skip this one.
bitmap_next:

	mov	bit_map_off[bp],bx	; save for later restore
	mov	di,word ptr buffer_beg_offset[bp]	; start of the buffer
	mov	si,word ptr buffer_end_offset[bp]	; end of the buffer

finished_with_this_buffer_piece:

	mov	cx,si
	sub	cx,di			; CX = number of bytes left

	mov	buffer_piece_end[bp],si
bgrep1:
	call	bgrep_for_expression
	jc	finished_with_entire_buffer

;-------------------------------------------------------------------------
; 	We have a match, set rgchResult[index_this_string] to true
;-------------------------------------------------------------------------
;	REVIEW
	mov	bx, bit_map_off[bp]
	push	di
	mov	di, rgchResult
	add	di, BUFFER:index_of_string_this_bitmap_belongs_to[bx]
	inc	byte ptr ss:[di]
	;	set index to ffff so we don't look for this one again.
	mov	BUFFER:index_of_string_this_bitmap_belongs_to[bx], 0ffffh
	pop	di
;-------------------------------------------------------------------------

	jmp short previous_setting_is_most_significant

finished_with_entire_buffer:
	mov	bx, bit_map_off[bp]
	sub	ax,buffer_piece_end[bp]
	neg	ax
	cmp	ax,number_of_bytes_to_lseek[bp]
	jbe	previous_setting_is_most_significant
	mov	number_of_bytes_to_lseek[bp],ax

previous_setting_is_most_significant:
	mov	cx,BUFFER:next_bitmap[bx]
	or	cx,cx
	je	really_finished
	mov	bx, cx
	mov	cx, BUFFER:index_of_string_this_bitmap_belongs_to[bx]
	inc	cx	; test for -1
	jz	previous_setting_is_most_significant	; skip this one.
	jmp	bitmap_next

really_finished:
	mov	ax,number_of_bytes_to_lseek[bp]
	call	lseek_for_bgrep
ifdef PROFILE
	cCall	StopNMeas,<>
endif ;PROFILE
	mov	sp,bp
	pop	bp
	ret

gimme_a_piece_a_buff	endp

;-----------------------------------------------------------------------;
; lseek_for_bgrep							;
; 									;
; Arguments:								;
; 									;
; Returns:								;
; 									;
; Alters:								;
; 									;
; Calls:								;
; 									;
; History:								;
; 									;
;  Mon Aug 26 20:22:30 1985    -by-    Wesley O. Rupel    [wesleyr]	;
;-----------------------------------------------------------------------;
	assumes	ds,CODE
	assumes	es,nothing

; %%Function:lseek_for_bgrep %%Owner:chic
cProc	lseek_for_bgrep,<NEAR,PUBLIC>, <ds>
cBegin
	mov	dx,cs
	mov	ds,dx

	test	bit_flags,GET_MORE_OF_FILE_FLAG
	jz	no_reason_to_lseek

	neg	ax
	jz	no_reason_to_lseek

					;AX = minus the number of bytes which
					;we must set the file ptr back.

	cwd				; dx:ax = offset to move in bytes

	farPtr	dx_ax, dx, ax
	mov	bx, 1			; seek from current position
	cCall	DwSeekDw,<input_file_handle, dx_ax, bx>
	cmp	dx, 0FFFFh
	je	lseek_problems
no_reason_to_lseek:
cEnd

seek_to_fcFirst_problems:
lseek_problems:
	jmp	ret_error_to_word


;-----------------------------------------------------------------------;
;	seek_to_fcFirst
;	Moves the file pointer for input_file_handle to fcFirst
;-----------------------------------------------------------------------;
	assumes ds, BUFFER
	assumes	es, DATA
; %%Function:seek_to_fcfirst %%Owner:chic
cProc	seek_to_fcfirst, <NEAR, PUBLIC>
cBegin
	mov	dx, wHigh (fcFirst)
	mov	ax, wLow (fcFirst)
	xor	bx, bx		; Move from start of file
	cCall	DwSeekDw,<input_file_handle, dx_ax, bx>
	cmp	dx, 0FFFFh
	je	lseek_problems
done_with_seek_to_fcFirst:
cEnd

;-----------------------------------------------------------------------;
; bgrep_for_expression							;
; 									;
; Seaches for a match in the buffer piece pointed to by ES:DI using	;
; the bitmap pointed to by DS:00.					;
; 									;
; Arguments:								;
; 	ES:DI	=	buffer piece					;
; 	DS:BX	=	bitmap						;
; 	SI	=	stop value					;
; Returns:								;
; 	CX = # of used buffer bytes.					;
; 	CF = 0  =>  A match was found.					;
; 		ES:AX = beginning of match in line.			;
; 		ES:DI = end of match in line.				;
; 	CF = 1  =>  No match found.					;
; 		ES:AX = last pt where we began looking for match	;
; 		ES:DI = end of buffer					;
; Alters:								;
; 	AX, BX, CX, DX, SI, DI						;
; Calls:								;
; 	Nothing								;
; History:								;
; 									;
;  Mon Aug 19 05:01:59 1985    -by-    Wesley O. Rupel    [wesleyr]	;
; Wrote it!  Modified from check_line_for_match				;
;-----------------------------------------------------------------------;

		local_variables
	local_word	stack_bottom
	local_word	stop_value
	local_word	bitmap_start

	assumes ds, BUFFER
	assumes es, BUFFER

bgrep_for_expression	proc	near

;---------------------------------------
	push	bp
	mov	bp,sp
	add	sp,local_space
ifdef PROFILE
	cCall	StartNMeas,<>
endif ;PROFILE
	mov	stop_value[bp],si
	mov	bitmap_start[bp], bx	; save start of this bitmap
;---------------------------------------
	mov	dx,BUFFER:bitmap_flags[bx]

	test	dl,ANCHOR_FLAG
	jz	b_no_anchor
	test	dl,BOL_FLAG
	jnz	b_no_anchor
	test	dl,POSITION_NOT_MEANINGFUL_FLAG
	jnz	b_no_anchor
;--------------------------------------------------------------------------
;  CX has the buffer piece size.  We will pt DS:SI to the anchor string and
;  repne scasb for it (this uses ES:DI) in the current line.

b_scan_for_anchor_char:

	push	di		;save ptr to BOL
	mov	ah,BUFFER:byte ptr anchor_string_length_minus_one[bx]
	mov	si,BUFFER:anchor_string_offset[bx]
				;Now DS:SI pts to the anchor string
	lodsb			;AL = first char in anchor string
	mov	dx,si		;Save SI in DX
b_rpmore:
	mov	si,dx		;Restore SI (pts to 2nd char in anchor string)
	repne	scasb		;Scan for the first char in anchor string
	jcxz	b_we_did_not_match_fast
	push	cx		;save cx
	mov	bx,di		;save di
	xor	ch,ch
	mov	cl,ah	;mov into cx the length of the anchor string minus one
	repe	cmpsb	;What happens if cx is already zero?
	jcxz	b_anchor_string_found	;prepare to do bitmap
b_false_alarm:
	mov	di,bx		;restore di
	mov	bx, bitmap_start[bp]
	pop	cx		;restore cx
	jmp	b_rpmore

b_anchor_string_found:	;prepare to do bitmap
	jnz	b_false_alarm

	pop	cx
	pop	si	;old di (BOL)
	push	si	;we may be branching back up
	mov	bx, bitmap_start[bp]

; Check if anchor is too close to the front.
; DI = one past end of anchor string in current line
; SI = bol
;  DI - "length" - SI  gives how far into the line the anchor string begins.
;  "position" is how far in it must be to be part of the sought expression.
;---------

	mov	cx, BUFFER:bitmap_flags[bx]
	test	cl, POSITION_NOT_MEANINGFUL_FLAG
	jnz	b_qm_asterisk_plus
				;The expression includes a "?", "*", or "+".
;---------
	mov	cx,di
	sub	cx, BUFFER:anchor_string_length_minus_one[bx]
	sub	cx,si	       ;This puts into CX how far into the line we are.
	cmp	cx, BUFFER:position_in_expression_of_anchor_string[bx]
	jbe	b_false_alarm
	add	sp,2	;We didn't take the branch so put SP back.
	sub	di, BUFFER:position_in_expression_of_anchor_string[bx]
	sub	di, BUFFER:anchor_string_length_minus_one[bx]
	dec	di
;--------------------------------------
b_no_anchor:
	mov	dx, BUFFER:bitmap_flags[bx]		;Flags in dl.
	dec	di
	mov	ax,di
	test	dl,POSITION_NOT_MEANINGFUL_FLAG
	jnz	b_qm_asterisk_plus1

	jmp	short  b_litle

b_we_did_not_match_fast:
	add	sp,2			;pop di
	mov	sp,bp
	pop	bp
	mov	ax,di
ifdef PROFILE
	cCall	StopNMeas,<>
endif ;PROFILE
	stc
	ret

b_qm_asterisk_plus:
	add	sp,2	;undo the push 'si'
	mov	di,si	;point DI to BOL
	dec	di
	mov	ax,di
	mov	dl,cl	;flags
b_qm_asterisk_plus1:			;WORRY did we jumped here correctly
	mov	stack_bottom[bp],sp
	jmp	short b_litle_ast
				;The expression includes a "?", "*", or "+".

;----------------------------------------------------------------------------
;  The following is the search routine which uses the bitmap.
;  Entry Point = litle
;  Arguments:
; 	AX pts one byte before first char in line to search.
; 	DL = flags
;----------------------------------------------------------------------------
b_char_does_not_match:
;	test	dl,BOL_FLAG	;Set means 1 failure and we're out.
;	jnz	b_we_did_not_match
b_litle:	mov	si,BITMAP_HEADER_LENGTH + TFB
	add	si,bx	;reset SI to beg. of bitmap (1st template)
	mov	di,ax	;AX keeps our current char position in the line which
	inc	ax	;must match the first char in the expression.  Once
			;first char is matched then DI steps forward checking
			;the rest of the expression against the line w/o
b_check_next_char:	;incrementing AX -- We need to return to AX if we were
	inc	di	;on a false lead.

	cmp	di,stop_value[bp]
	je	b_we_did_not_match

	mov	cl,es:[di]
;-----------------------------------------------------------------------------
; This is the routine test_bit inserted here to save the 35 cycles of call-ret
	push	bx	; painful, but we need to preserve bx
	mov	bl,cl
	shr	bl,1
	shr	bl,1
	shr	bl,1
	and	cl,07h
	mov	ch,80h
	shr	ch,cl
	xor	bh,bh
	test	[si+bx],ch
	pop	bx
;-----------------------------------------------------------------------------
	jz	b_char_does_not_match
	cmp	byte ptr BUFFER:[si-TFB].last_template_in_this_bitmap,LAST_TEMPLATE
	je	b_we_matched
	add	si,TS+TFB
	jmp	b_check_next_char

b_we_matched:
ifdef PROFILE
	cCall	StopNMeas,<>
endif ;PROFILE
	mov	sp,bp
	pop	bp
	clc			;ES:DI is pointing to the last matching char.
	ret			;CF=0 => we matched.

b_we_in_an_asterisk:
;  Make it appear that asterisk failure occured at the cr
	dec	di
	jmp	short	b_push_asterisk_failure_state

b_we_did_not_match_ast1:
	test	dl,01h			;test asterisk flag
	jnz	b_we_in_an_asterisk

b_we_did_not_match:
ifdef PROFILE
	cCall	StopNMeas,<>
endif ;PROFILE
	stc
	mov	sp,bp
	pop	bp
	ret

;-----------------------------------------------------------------------------
;  We use the following if the position-not-meaningful flag was set -- that is
;  if the expression contained a "?", "+", or "*".
;-----------------------------------------------------------------------------
; Asterisk and plus are the main reasons for the more complicated code here.
; Software Tools by Kernighan and Plauger discusses the algorithm used here
; for asterisk in their discussion of "find".  They refer to the asterisk
; as a "closure" (it's in the index).  My plus algorithm is similer.
;-----------------------------------------------------------------------------
;----------------------------------------------------------------------------
;  The following is the search routine which uses the bitmap.		:
;  Entry Point = litle_ast					    :
;  Arguments:							:
; 	AX pts one byte before first char in line to search.	:
; 	DL = flags						    :
; 	BP = SP?????							:
;----------------------------------------------------------------------------
;-----------------------------------------------------------------------------
b_char_does_not_match_ast:
	cmp	dh,'?'
	je	b_push_asterisk_failure_state
	cmp	dh,'*'
	je	b_push_asterisk_failure_state
	cmp	dh,'+'
	je	b_push_plus_failure_state
b_as_b4:
	cmp	stack_bottom[bp],sp
	jne	b_asterisk_encountered_previously
;	test	dl,BOL_FLAG	;Set means 1 failure and we're out.
;	jnz	b_we_did_not_match
b_litle_ast:
	mov	si,BITMAP_HEADER_LENGTH + TFB
	add	si,bx	;reset SI to beg. of bitmap (1st template)
	mov	di,ax	;AX keeps our current char position in the line which
	inc	ax	;must match the first char in the expression.  Once
			;first char is matched then DI steps forward checking
			;the rest of the expression against the line w/o
b_check_next_char_ast:	;incrementing AX -- We need to return to AX if we were
	inc	di	;on a false lead.
b_question_mark_reentry_point:

	cmp	di,stop_value[bp]
	je	b_we_did_not_match_ast1

	mov	cl,es:[di]

	mov	dh,BUFFER:[si-TFB].asterisk_location
	cmp	dh,'*'
	je	b_push_asterisk_initial_state_info
	cmp	dh,'?'
	je	b_push_asterisk_initial_state_info
b_this_was_pointless:
;-----------------------------------------------------------------------------
; This is the routine test_bit inserted here to save the 35 cycles of call-ret
	push	bx
	mov	bl,cl
	shr	bl,1
	shr	bl,1
	shr	bl,1
	and	cl,07h
	mov	ch,80h
	shr	ch,cl
	xor	bh,bh
	test	[si+bx],ch
	pop	bx
;-----------------------------------------------------------------------------

	jz	b_char_does_not_match_ast
	cmp	byte ptr BUFFER:[si-TFB].last_template_in_this_bitmap,LAST_TEMPLATE
	je	b_we_matched_ast
	cmp	dh,'+'
	je	b_set_plus_flag_and_push_initial_state
	cmp	dh,'*'
	je	b_set_asterisk_flag
	cmp	dh,'?'
	je	b_fail_the_question_mark
	add	si,TS+TFB
	jmp	b_check_next_char_ast

b_fail_the_question_mark:

; We came here because we had a success with the question mark.  The rules are
; that we must not have more than one success.  We are using all the apparatus
; of asterisk so we will just make it look as if we went thru one more pass and
; it failed to match.

	inc	di
	jmp	b_char_does_not_match_ast

b_set_asterisk_flag:
	or	dl,01h			;set asterisk flag
	jmp	b_check_next_char_ast

b_push_plus_failure_state:
	test	dl,01h			;If flag not set then we did not get at
	jz	b_as_b4			;least one match as required.

b_push_asterisk_failure_state:
	push	di
	and	dl,not 01h		;clear asterisk flag

b_failure_okay:
	add	si,TS+TFB
	jmp	b_question_mark_reentry_point

b_asterisk_encountered_previously:
	pop	di
	pop	si
	pop	cx
	cmp	cx,di			;Have we back tracked as far as we can?
	je	b_as_b4
	dec	di
	push	cx
	push	si
	push	di
	jmp	b_question_mark_reentry_point


b_set_plus_flag_and_push_initial_state:

; We only come here if we have already succeeded at matching the current char
; in the current line with the current template (char class).

	test	dl,01h			;test asterisk flag
	jnz	b_check_next_char_ast	;Jmp if we already pushed the initial
					;state of this plus.

	cmp	byte ptr BUFFER:[si-TFB].last_template_in_this_bitmap,LAST_TEMPLATE
	je	b_we_matched_ast		;Jump if this is the end of the
						;expression => line matches.

	or	dl,01h			;set asterisk flag
	inc	di
	push	di
	add	si,TS+TFB
	push	si
	sub	si,TS+TFB
	jmp	b_question_mark_reentry_point

b_push_asterisk_initial_state_info:
	test	dl,01h			;test asterisk flag
	jnz	b_this_was_pointless

; There is no reason to ever put an asterisk at the end of a regular expression
; since the line will match even if the asterisked char is absent. Nevertheless
; people like Tony are apt to do it anyway so the following 2 lines are present
; to avoid the associated problems that would otherwise occur.

	cmp	byte ptr BUFFER:[si-TFB].last_template_in_this_bitmap,LAST_TEMPLATE
	je	b_we_matched_ast

	push	di
	mov	bx,si
	add	bx,TS+TFB
	push	bx
	mov	bx, bitmap_start[bp]
	jmp	b_this_was_pointless	;actually from here it was pointful.

b_we_matched_ast:
ifdef PROFILE
	cCall	StopNMeas,<>
endif ;PROFILE
	mov	sp,bp
	pop	bp
	clc			;ES:DI is pointing to the last matching char.
	ret			;CF=0 => we matched.

bgrep_for_expression	endp

;-----------------------------------------------------------------------;
; make_the_bitmaps							;
; 									;
; Arguments:								;
; 	ES = DATA_SEG							;
; Returns:								;
; 	DS:SI = filename						;
; Alters:								;
; 	AX,BX,CX,DX,SI,DI,DS						;
; Calls:								;
; 	allocmem							;
; 	build_bitmap							;
; History:								;
; 									;
;  Wed Aug 14 23:38:41 1985    -by-    Wesley O. Rupel    [wesleyr]	;
; Sequestered it from grep.asm main					;
;-----------------------------------------------------------------------;

	local_variables
	local_word	bm_seg
	local_word	number_of_available_bytes
	local_word	index_of_current_string

	assumes	es, DATA
	assumes	ds,nothing

make_the_bitmaps	proc	near

; Our next arg should be the regular expression.

	push	bp
	mov	bp,sp
	add	sp,local_space
ifdef PROFILE
	cCall	StartNMeas,<>
endif ;PROFILE
	call	allocmem
	mov	first_bitmap_off,ax
	mov	current_bitmap_off,ax

	mov	number_of_available_bytes[bp],bx
	mov	word ptr index_of_current_string[bp],00h

build_a_bitmap:

	mov	bx, ss
	mov	ds, bx
	assumes	ds, DATA
	mov	es, buffer_seg
	assumes es, BUFFER
	mov	bx, index_of_current_string[bp]
	shl	bx,1		; bx is a word count, this makes it a byte offset
	mov	di, pStrings
	mov	si, [di][bx]	;Now DS:SI pts to the expression

	mov	di,number_of_available_bytes[bp]

	mov	ax,bit_flags
	mov	cx,current_bitmap_off
	shr	bx, 1		; back to a real index
	call	build_bitmap

	jnc	foorj
	jmp	ret_error_to_word
foorj:

	mov	al,dh			;flags
	and	al,80h
	shl	al,1			;make CF = BOL flag

	jnc	no_bol			;We won't do super_grep if BOL flag set
	or	bit_flags,BOL_SET_FLAG
no_bol:

	inc	word ptr index_of_current_string[bp]
	mov	bx, index_of_current_string[bp]
	cmp	bx, cStrings  	; do we have anymore strings to do?
	je	out_of_args	; nope
	jmp	short or_it_is	; yup
	
out_of_args:
	mov	bx, current_bitmap_off
	mov	word ptr BUFFER:[bx],0		;Set last-bitmap mark
	jmp	short	no_more_bitmaps


or_it_is:
	mov	bx,current_bitmap_off
	mov	current_bitmap_off,di		;DI = next free byte
	sub	number_of_available_bytes[bp], di
	add	number_of_available_bytes[bp], bx ; start of last one

	mov	ax,current_bitmap_off
	mov	Word Ptr BUFFER:[bx], ax ;store seg of next bitmap at beginning
	jmp	build_a_bitmap		;of current bitmap

no_more_bitmaps:
	mov	bx,current_bitmap_off
	sub	number_of_available_bytes[bp], di ; end of last one
	add	number_of_available_bytes[bp], bx ; start of last one
	mov	bx,number_of_available_bytes[bp]
	mov	free_memory_off, di 	; offset into BUFFER
	mov	free_memory_size, bx

ifdef PROFILE
	cCall	StopNMeas,<>
endif ;PROFILE
	mov	sp,bp
	pop	bp
	ret

make_the_bitmaps	endp

	assumes	cs,CODE
	assumes	ds,DATA
	assumes	es,BUFFER
	assumes	ss,DATA
;-----------------------------------------------------------------------;
; build_bitmap								;
; 									;
; Builds bitmap from regular expression.				;
; 									;
; Arguments:								;
; 	DS:SI	=	Expression					;
; 	ES:CX	=	Where bitmap will be made.			;
; 	DI	=	# of bytes available			;
;	BX	=	index of current string				;
; 	AX	=	bit_flags					;
; Returns:								;
; 	CF = 0								;
; 	  DI = offset of next free byte					;
; 	  DH = flags							;
; 	CF = 1								;
; 	  ZF = 0  =>  Out of memory					;
; 	  ZF = 1  =>  Illegal expression				;
; Alters:								;
; 	AX,BX,CX,DX,SI,DI						;
; Calls:								;
; 	test_bit, put_bit, get_RE_char					;
; History:								;
; 									;
;  Thu Jul 25 18:50:57 1985    -by-    Wesley O. Rupel    [wesleyr]	;
; Wrote it for GREP							;
;-----------------------------------------------------------------------;

		local_variables
	local_word	lw_bit_flags
	local_word	current_pointer
	local_word	index_of_current_string
	local_word	bitmap_start

build_bitmap	proc	near
;--------------------------------------------------------------------
;_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*
;		BIT-MAP  BUILDING  ROUTINE
;
; Thruout this routine in general:
; ES:DI  pts to current template (the portion of the bitmap against which a
;	 single char in the file will be compared -- single char class).
; DS:SI  pts to current char in expression (in command line).
; DH  will contain internal flags.
; BP  is used as always, the other registers are free to fuck with.

	push	bp
	mov	bp,sp
	add	sp,local_space


ifdef PROFILE
	cCall	StartNMeas,<>
endif ;PROFILE
	mov	lw_bit_flags[bp],ax		;initialize
	mov	index_of_current_string[bp],bx
	mov	bitmap_start[bp], cx

;-------------------------------Check-if-there-is-enuf-memory------
mem_check:
	push	di
	push	es
	mov	ax, ss
	mov	es, ax		; repne scasb insists on es
	mov	di,si
	xor	ax,ax
	mov	cx,0FFFFh
	repne	scasb
	pop	es
	sub	ax,cx	; Now AX has the number of chars in the expression.
	xor	dx,dx
	mov	cx,30h	; We will ask for 3 paragraphs per char for templates
	mul	cx
	or	dx,dx
	je	another_fucking_out_of_range_jump
	jmp	not__enuf__memory

another_fucking_out_of_range_jump:

	pop	di
	cmp	ax,di	; AX has # of bytes we need
	jnb	not_enuf_memory

;------------------------------------------------------------------

	mov	di,BITMAP_HEADER_LENGTH - TEMPLATE_SIZE
	add	di, bitmap_start[bp]
	xor	dh,dh	;initialize (internal) flags

;	sub	di,TEMPLATE_SIZE
next_template:
	add	di,TEMPLATE_SIZE		;pt to next template

ifdef debug
	mov	cx,TFB/2
	mov	ax,09999h	;this is for easy debugging
debugaa:
	add	ax,1111h
	stosw
	loop debugaa

	xor	ax,ax		;Now we zero out the template (before marking
	mov	cx,TS/2		;any bits in it).
else
	xor	ax,ax		;Now we zero out the template (before marking
	mov	cx,(TS+4)/2	;any bits in it).
endif

	rep	stosw
	sub	di,TS	;restore di to what it was before rep stosw

next_char:
	call	get_RE_char	; returns char in al
	jc	illegal_expression

	cmp	dl,"\"
	jz	can_not_be_real_zero
	or	al,al			; we want \00 to work!
	je	end_of_expression_
can_not_be_real_zero:

;  is the char in AL special?

	mov	cl,al		;test_bit wants the char in CL.
	push	di
	push	es
	mov	di,offset bitmap_of_special_chars
	mov	bx,cs
	mov	es,bx
	call	test_bit
	pop	es
	pop	di
	jnz	its_special

not_special:
	mov	cl,al		; put_bit wants its char in cl
	call	put_bit		; mark char in bit-map
	test	word ptr lw_bit_flags[bp],DASH_Y_FLAG
	jnz	y_flag_set

googoogahgah:
	test	dh,BRACKET_FLAG
	jnz	next_char
	test	word ptr lw_bit_flags[bp],DASH_Y_FLAG
	jnz	next_template

	mov	es:[di-TFB].the_literal_char,al
	mov	ah,LIT_CHAR_INDICATOR
	mov	es:[di-TFB].literal_char_indicator,ah

	or	dh,ANCHOR_FLAG		;set anchor flag

	jmp	next_template

not__enuf__memory:
	add	sp,4		;account for the two pushes (ES & DI).
not_enuf_memory:
	or	sp,sp		;clear the zero flag (SP is not 0)
oh_dude:
ifdef PROFILE
	cCall	StopNMeas,<>
endif ;PROFILE
	stc
	mov	sp,bp
	pop	bp
	ret

end_of_expression_:
	jmp	end_of_expression
its_special:
	cmp	dl,"\"
	je	not_special
	mov	bx,ax	; pick-up the special char
	cmp	bx,2ah	;	is it an asterix?
	je	asterisk
	cmp	bx, "."		; is it a dot?
	je	punkt	
	jmp	pound
illegal_expression:
	xor	al,al		;set the zero flag to indicate this
	jmp	oh_dude
y_flag_set:
	or	dh,EXPRESSION_FLAG		;set expression flag

	push	es		; we need to set es=ds=data for calls
	push	ds		; into the word routines. We can restore
	mov 	bx, ss		
	mov	es, bx		; them when we get back
	mov	ds, bx
	assumes	es,DATA
	assumes	ds,DATA
	push	dx		; save non-scratch registers
	push	ax		; save char, islower will kill it
        xor     ah,ah           ; FLower expects this.
	cCall	FLower, ax, 0
	or	ax, ax
	pop	ax		; restore char
	jz 	NotLower
	xor	bx,bx
        xor     ah,ah           ; So AnsiUpper doesn't take this as a string
	cCall	AnsiUpper,<bx,ax>,0
	jmp	short not_special2
NotLower:
	push	ax		; isUpper will hammer it
        xor     ah,ah           ; FUpper expects this.
	cCall	FUpper, ax, 0
	or	ax, ax
	pop	ax		; restore char
	jz	not_alpha
	xor	bx,bx
        xor     ah,ah           ; So AnsiLower doesn't take this as a string
	cCall	AnsiLower,<bx,ax>,0
	clc
	jmp	short not_special2
not_alpha:
	stc
not_special2:
	pop	dx		; restore non-scratch registers
	pop	ds
	pop	es
	assumes es, BUFFER
	assumes	ds,DATA
	jc	done_putting_bits
	mov	cl,al
	call	put_bit
done_putting_bits:
	jmp	googoogahgah

err_illegal_expression:
	jmp	illegal_expression	; I hate out of range jumps!
asterisk:	
question_mark:
plus_sign:
jnz_not_special:
	or	dh,POSITION_NOT_MEANINGFUL_FLAG	OR EXPRESSION_FLAG  ;set expression flag
	mov	BUFFER:[di-(TS+TFB)-TFB].asterisk_location,al
	jmp	next_char
punkt:
	or	dh,EXPRESSION_FLAG	;set expression flag
	mov	ax,0ffffh
	mov	cx,TS/2
	rep	stosw
	sub	di,TS
	jmp	next_template
pound:
	or	dh,EXPRESSION_FLAG	;set expression flag
	mov	ax,0ffffh		;first set all the bits to true
	mov	cx,TS/2
	rep	stosw
	sub	di,TS
	mov	byte ptr BUFFER:[di]+4,07fh    ; now turn off space, 
	mov	byte ptr BUFFER:[di]+1,09bh	; tab,carriage return, newline,
	mov	byte ptr BUFFER:[di],07fh	; and Null
	jmp	next_template
not_special_:
	jmp	not_special
end_of_expression:

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; Now we will store info about the bitmap (in particular, what the best
; anchor string is) in the first few bytes of the bitmap, before the
; first template.


	mov	si,di
	test	dh,ANCHOR_FLAG	;test if at least one anchor char exists
	jz	love
	sub	di,TS+TFB
	mov	bx,BITMAP_HEADER_LENGTH - TS
	add	bx, bitmap_start[bp]
	xor	ax,ax	;AX will hold the len. of the longest lit-string
	xor	cx,cx	;zero the count

not_ff:	cmp	ax,cx
	jae	dowah
	mov	ax,cx
	mov	si,current_pointer[bp]
dowah:	cmp	bx,di		;is this the last template?
	je	now_we_know
	add	bx,TS + TFB
	xor	cx,cx	;zero count
	mov	current_pointer[bp],bx	;zero the count

cam:	cmp	byte ptr BUFFER:[bx-TFB].literal_char_indicator,LIT_CHAR_INDICATOR
	jne	not_ff
; Use of *,+, or ? precludes use of the corresponding template in
; the anchor string
	cmp	byte ptr BUFFER:[bx-TFB].asterisk_location,'*'
	je	not_ff
; only * is still used as a repeating wildcard
;	cmp	byte ptr BUFFER:[bx-TFB].asterisk_location,'?'
;	je	not_ff
;	cmp	byte ptr BUFFER:[bx-TFB].asterisk_location,'+'
;	je	not_ff
	inc	cx
	cmp	bx,di	;is this the last template?
	je	not_ff
	add	bx, TS + TFB
	jmp	cam

now_we_know:
	add	di,TS+TFB	;point DI to one template past the last one
				;used.
;------------
	mov	bx, bitmap_start[bp]
	mov	BUFFER:anchor_string_offset[bx],di
				;store position of its template in bitmap.
	push	ax
	mov	ax,si		;template of 1st char in anchor
	sub	ax,BITMAP_HEADER_LENGTH
	sub	ax, bx
	mov	cl,TS+TFB
	div	cl
	xor	ah,ah
	mov	BUFFER:position_in_expression_of_anchor_string[bx],ax
	pop	cx
	dec	cx
	mov	BUFFER:anchor_string_length_minus_one[bx],cx	
;--------------
	mov	bx,si	;SI pts to the template of the 1st char in anchor
	push	di
	inc	cx
	jcxz	plus_on_only_anchor_char
build_the_anchor_string:
	mov	al, BUFFER:[bx-TFB].the_literal_char
	stosb
	add	bx,TS+TFB
	dec	cx
	or	cx,cx
	jnz	build_the_anchor_string
	mov	si,di
	pop	di

love:
	mov	bx, bitmap_start[bp]
	mov	al,LAST_TEMPLATE
	mov	BUFFER:[di-(TS+TFB)-TFB].last_template_in_this_bitmap,al
	mov	byte ptr BUFFER:bitmap_flags[bx],dh
;  Put index of string in the header.
	mov	cx, index_of_current_string[bp]
	mov	BUFFER:index_of_string_this_bitmap_belongs_to[bx],cx
;  Now put next free byte in di
	mov	di,si	;SI pts 1 past the last char in anchor string

ifdef PROFILE
	cCall	StopNMeas,<>
endif ;PROFILE
	mov	sp,bp
	pop	bp
	clc
	ret
plus_on_only_anchor_char:
	and	dh,not ANCHOR_FLAG	;no anchor after all
	jmp	short	love
build_bitmap	endp
;_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*



;-----------------------------------------------------------------------;
; valid_hex_digit?							;
; 									;
; Converts ASCII digits [0-9,a-f or A-F] to the appropriate binary value.;
; 									;
; Arguments:								;
; 	AL = Char							;
; Returns:								;
; 	CF = 0								;
; 	  BH = binary value						;
; 	CF = 1								;
; 	  Char was not a valid hex digit.				;
; Alters:								;
; 	BH								;
; Calls:								;
; 	Nothing								;
; History:								;
; 									;
;  Sat Jun 08 08:47:53 1985    -by-    Wesley O. Rupel    [wesleyr]	;
; Created for use in GREP						;
;-----------------------------------------------------------------------;

	assumes	ds,nothing
	assumes	es,nothing

valid_hex_digit?	proc	near

	mov	bh,al
	sub	bh,'0'

	cmp	bh,10
	jb	valid

	sub	bh,'A'-'9'-1

	cmp	bh,10
	jb	invalid
	cmp	bh,16
	jb	valid

	sub	bh,32	; 32 = the diff. between smalls and caps

	cmp	bh,10
	jb	invalid
	cmp	bh,16
	jnb	invalid

valid:	clc
	ret
invalid:stc
	ret
valid_hex_digit?	endp


;-----------------------------------------------------------------------;
; get_RE_char								;
; 									;
; Get a logical char pointed to by DS:SI and send it back in AL.	;
; By logical char I mean the chars are interpreted according to the	;
; rules for Regular Expressions.  Thus \. and \* are one char		;
; 									;
; Arguments:								;
; 	DS:SI = char to be read.					;
; Returns:								;
; 	CF = 0								;
; 	  AL = Char							;
; 	  DS:SI = next logical char					;
; 	  DL = the first actual char (so the calling routine can check if;
; 		 it was a "\").						;
; 	CF = 1								;
; 	  Invalid expression.						;
; Alters:								;
; 	SI,BH,CX,DL							;
; Calls:								;
; 	valid_hex_digit?						;
; History:								;
; 									;
;  Sat Jun 08 09:13:42 1985    -by-    Wesley O. Rupel    [wesleyr]	;
; Produced for use in GREP						;
;-----------------------------------------------------------------------;

	assumes	ds,nothing
	assumes	es,nothing

get_RE_char	proc	near
	cld
	lodsb
	mov	dl,al	;calling routine can check dl to see if there was a "\"
	cmp	al,'\'
	jne	not_a_backslash
	lodsb
not_a_backslash:
	clc
	ret

invalid_expression:
	stc
	ret

get_RE_char	endp


;-----------------------------------------------------------------------;
; put_bit								;
; 									;
; Puts bit in bit-map corresponding to the char in cl.			;
; 									;
; Arguments:								;
; 	ES:DI = bitmap template						;
; 	CL = the char whose bit is to be turned on			;
; Returns:								;
; 	Nothing								;
; Alters:								;
; 	CX, BX								;
; Calls:								;
; 	Nothing								;
; History:								;
; 									;
;  Sat Jun 08 10:24:17 1985    -by-    Wesley O. Rupel    [wesleyr]	;
; Written for use in GREP.  The algorithm was stolen from Nathan.	;
;-----------------------------------------------------------------------;

	assumes	ds,nothing
	assumes	es,nothing

put_bit	proc	near
	mov	bl,cl
	shr	bl,1
	shr	bl,1
	shr	bl,1
	and	cl,07h
	mov	ch,80h
	shr	ch,cl
	xor	bh,bh
	or	es:[di+bx],ch
	ret
put_bit	endp


;-----------------------------------------------------------------------;
; test_bit								;
; 									;
; Just like put_bit but tests rather than puts.				;
; 									;
; Arguments:								;
; 	ES:DI = bitmap template						;
; 	CL = the char whose bit is to be tested				;
; Returns:								;
; 	Nothing								;
; Alters:								;
; 	BX,CX								;
; Calls:								;
; 	Nothing								;
; History:								;
; 									;
;  Sat Jun 08 13:47:10 1985    -by-    Wesley O. Rupel    [wesleyr]	;
; Written for use in GREP						;
;-----------------------------------------------------------------------;

	assumes	ds,nothing
	assumes	es,nothing

test_bit	proc	near
	mov	bl,cl
	shr	bl,1
	shr	bl,1
	shr	bl,1
	and	cl,07h
	mov	ch,80h
	shr	ch,cl
	xor	bh,bh
	test	es:[di+bx],ch
	ret
test_bit	endp
ifdef NOT_USED
;-----------------------------------------------------------------------;
; free_bytes								;
; 									;
; Frees unused portion of memory given by last allocmem			;
; 									;
; Arguments:								;
; 	AX = # of bytes used						;
; Returns:								;
; 	Nothing								;
; Alters:								;
; 	AX								;
; Calls:								;
; 	Nothing								;
; History:								;
; 									;
;  Sun Aug 25 15:46:51 1985    -by-    Wesley O. Rupel    [wesleyr]	;
; Wrote it!								;
;-----------------------------------------------------------------------;
	assumes	es,nothing
	assumes	ds,nothing

free_bytes	proc	near
	
	mov	cx,ax
	add	ax,previous_free_memory_off
	mov	free_memory_off,ax

	sub	cx,previous_free_memory_size
	ja	process_used_too_much_memory
	neg	cx
	mov	free_memory_size,cx
	ret

process_used_too_much_memory:
	jmp	ret_error_to_word
free_bytes	endp
endif	; NOT_USED
;-----------------------------------------------------------------------;
; allocmem								;
; 									;
; Arguments:								;
; 	None								;
; Returns:								;
; 	AX = first free byte						;
; 	BX = # of free bytes						;
; Alters:								;
; 	AX,BX								;
; Calls:								;
; 	Nothing								;
; History:								;
; 									;
;  Sun Aug 25 17:14:19 1985    -by-    Wesley O. Rupel    [wesleyr]	;
; Wrote it!								;
;	6/88	RICKS	modified to be byte based instead of para based	;
;-----------------------------------------------------------------------;

	assumes	es,DATA
	assumes	ds,nothing

allocmem	proc	near

	mov	ax,free_memory_off
	mov	bx,free_memory_size
	or	bx,bx
	jz	insuff_mem
ifdef NOT_USED
	mov	previous_free_memory_off,ax
	mov	previous_free_memory_size,bx
endif
	add	free_memory_off,bx
	mov	free_memory_size,0
	ret

insuff_mem:
	jmp	ret_error_to_word
allocmem	endp

sEnd	CODE 	; end of code segment	
	end	

