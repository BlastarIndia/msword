;-----------------------------------------------------------------------;
; test_EMS								;
;									;
; Tests for basic EMS capability and advanced EEMS capability.		;
; Initializes EMS flags and variables.					;
; If a machine has LIM 4.0 then we assume that there is memory from	;
; 0 to A000h!!								;
;									;
; Arguments:								;
;	none								;
;									;
; Returns:								;
;	nothing 							;
;									;
; Alters:								;
;	nothing 							;
;									;
; Calls:								;
;	nothing 							;
;									;
; History:								;
;									;
;  Wed May 13, 1987 09:59:07p  -by-  David N. Weise	[davidw]	;
; Added the EMSDDEPID.							;
;									;
;  Tue Apr 21, 1987 06:28:26p  -by-  David N. Weise	[davidw]	;
; Added the EMSGlobalPID.						;
;									;
;  Wed Mar 25, 1987 10:26:35p  -by-  David N. Weise	[davidw]	;
; Removed the TDB_EMSBitArray						;
;									;
;  Sun Mar 15, 1987 08:05:28p  -by-  David N.Weise	[davidw]	;
; Have been working on it for a bit.  Todays change is having this	;
; thing called early enough in bootstrapping so that the fake TDB	;
; can have some temporary banks.					;
;									;
;  Sat Aug 30, 1986 08:46:40p  -by-  Charles Whitmer	[chuckwh]	;
; Expanded it to check for advanced EEMS capabilities.			;
;-----------------------------------------------------------------------;

EMMdevname  DB	'EMMXXXX0'

hardware_capabilities	dw	10h dup(0)

cProc	test_EMS,<PUBLIC,NEAR>
cBegin	nogen
	push	ax
	push	bx
	push	cx
	push	dx
	push	di
	push	si
	push	ds
	push	es

; look for basic EMM capability

	xor	ax,ax			; See if EMM is present
	mov	ds,ax
	mov	es,ds:[EMM_API*4][2]
	push	cs
	pop	ds
	mov	si,codeOffset EMMdevname
	mov	di,10			; Point to device name
	mov	cx,8
	repz	cmpsb
	jz	have_device_name
no_advanced_eems:
	jmp	testemm_done
have_device_name:
	mov	ah,bEMM_STATUS		; Ask for status of EMM
	int	EMM_API
	or	ah,ah
	jnz	no_advanced_eems	; Non-zero means not installed.

; Get the LIM context size and remember that we still have to save and
;   restore the mapping registers for LIM 3.2 and below on a per task basis.

	mov	ax,wEMM_CONTEXT_SIZE	; Yes, determine size of save area
	int	EMM_API
	or	ah,ah			; Did we get it?
	jnz	no_advanced_eems	; No, ignore EMM then
	mov	cs:fEMM,al		; Save context size, mark EMM presence

; look for advanced EEMS capability

	mov	ah,bEMM_VERSION 	; get version number
	int	EMM_API
	or	ah,ah
	jnz	no_advanced_eems
	cmp	al,32h
	jb	no_advanced_eems
	or	cs:EMSFlags,EMSF_EXISTS
	call	check_calls
	jc	no_advanced_eems
	test	cs:EMSFlags[1],EMSF1_ADVANCED_OFF  ; User says no advanced EMS?
	jnz	no_advanced_eems	; Yes
	or	cs:EMSFlags[1],EMSF1_AST
	cmp	al,40h
	jb	AST_driver
	and	cs:EMSFlags[1],NOT EMSF1_AST
AST_driver:

; Get a count of how many pages SMARTDrive owns.

	call	check_SMARTDrive

; figure out if we even want to use EMS, we want at least 256K

	mov	ah,bEMM_PAGE_COUNTS
	int	EMM_API
	mov	di,codeOFFSET int13_status
	add	bx,cs:[di].current_size
	sub	bx,cs:[di].minimum_size
	cmp	bx,16			; BX = pages available
	jbe	no_advanced_eems

; pick up the true context size

;	mov	ax,wEMM_REGISTER_SET_LEN
;	int	EMM_API
;	or	ah,ah
;	jnz	no_advanced_eems
;	mov	cs:fEMM,dl

; pick up the page frame

	mov	ah,bEMM_PAGE_FRAME
	int	EMM_API
	or	ah,ah
	jz	got_a_page_frame
	xor	bx,bx
got_a_page_frame:
	mov	cs:EMSPageFrame,bx

; get the raw page size

	push	cs
	pop	es
	mov	di,codeOFFSET hardware_capabilities
	mov	ax,wEMM_HARDWARE_CAPS	; get hardware capability
	int	EMM_API
	or	ah,ah
	jnz	no_advanced_eems

; TEMPORARY BEGIN

	mov	ax,es:[di].ec_context_save_area_size
	mov	cs:fEMM,al		; WORD??????

; TEMPORARY END

	mov	ax,es:[di].ec_raw_page_size
	mov	cs:EMSPageSize,ax

; Calculate where the swap line should go, we take this to be 288K above our
;  PSP.

	mov	ax,cs:topPDB
	add	ax,288  * (KBYTES / PARAGRAPH)
	mov	bx,cs:EMSPageSize
	neg	bx
	and	ax,bx			; round down
	mov	cs:EMS_calc_swap_line,ax

; get the mappable address array

	mov	ax,wEMM_MAPPABLE_ARRAY_LEN ; get number of array entries
	int	EMM_API
	or	cx,cx
	jnz	got_array_length
	jmp	no_advanced_eems
got_array_length:
	mov	cs:EMSPageCount,cx
	shl	cx,1
	shl	cx,1			; 4 bytes per entry
	mov	si,sp			; SI = saved stack pointer
	sub	sp,cx			; make room on the stack
	push	ss
	pop	es
	mov	di,sp
	mov	ax,wEMM_MAPPABLE_ARRAY
	int	EMM_API
	or	ah,ah
	jz	have_array_on_stack
	mov	sp,si
	jmp	no_advanced_eems
have_array_on_stack:

; Obviously the EMS swap line can't be lower than any mappable address.

	mov	ax,es:[di]
	cmp	ax,cs:EMS_calc_swap_line
	jbe	swap_line_okay
	mov	cs:EMS_calc_swap_line,ax
swap_line_okay:

;-----------------------------------------------------------------------;
;  PID 0000 and the ICL machine.  The ICL machine supports expanded	;
; memory on the motherboard.  They may have have the top of memory	;
; at B000 instead of at A000.  Because of this we cannot assume that	;
; the top of memory is at A000. 					;
;  Because of the HP Vectra and the IBM PS|2 series we also have to	;
; worry about memory reserved just below A000.				;
;-----------------------------------------------------------------------;

; 1) Count the number of pages to reserve for PID0.
; 1a) Get the number of pages PID0 presently has to determine where
;     the "top of memory" is.

	mov	ah,bEMM_GET_PIDS_PAGES
	xor	dx,dx
	int	EMM_API
	or	ah,ah
	jz	got_PID0_pages
	mov	sp,si
	jmp	no_advanced_eems
got_PID0_pages:
	mov	cs:PID0_pages,bx

	push	es
	mov	ax,cs:topPDB
	dec	ax
get_DOS_top:
	mov	es,ax
	add	ax,es:[ga_size]
	inc	ax
	cmp	es:[ga_sig],GA_ENDSIG
	jnz	get_DOS_top
	mov	cs:DOS_top_of_memory,ax
	pop	es

	or	bx,bx			; get PID0 top
	jz	tops_same
	dec	bx
	shl	bx,1
	shl	bx,1
	mov	ax,es:[di][bx]
	add	ax,cs:EMSPageSize
tops_same:
	mov	cs:PID0_top_of_memory,ax

; 1b) Remove from the list those pages between the top of our DOS partition
;   and the "Top of EMM memory".

	mov	ax,cs:PID0_top_of_memory
	sub	ax,cs:DOS_top_of_memory
	add	ax,cs:EMSPageSize
	dec	ax
	xor	dx,dx
	div	cs:EMSPageSize
	mov	cx,ax
	jcxz	pages_removed
remove_pages:
	mov	dx,word ptr es:[di][bx][0]
	mov	word ptr es:[di][bx][0],0
	mov	word ptr es:[di][bx][2],0
	sub	bx,4
	loop	remove_pages
pages_removed:
	mov	cs:PID0_pages_high,ax
	mov	cs:PID0_high_start,dx

; 3) Remove pages below the EMS calc swap line ------------------------

	mov	ax,cs:EMS_calc_swap_line
	mov	bx,di
	mov	cx,cs:EMSPageCount
remove_pages_low:
	cmp	ax,es:[bx][0]
	jbe	pages_low_removed
	mov	word ptr es:[bx][0],0
	mov	word ptr es:[bx][2],0
	add	bx,4
	loop	remove_pages_low
pages_low_removed:

; 4) Remove blocks less than 48K in size ------------------------------

	push	di
	mov	cx,cs:EMSPageCount
rm_next_contiguous_set:
	mov	bx,di
	mov	ax,es:[di][0]
	mov	dx,ax
rm_still_contiguous:
	add	dx,cs:EMSPageSize
	add	di,4
	or	ax,ax
	jz	rm_null_set
	cmp	dx,es:[di][0]
	jne	rm_got_a_set
	loop	rm_still_contiguous
rm_got_a_set:
	sub	dx,ax			; get the length
	cmp	dx,48 * (KBYTES / PARAGRAPH)
	jae	rm_null_set
rm_remove_small_blocks:
	mov	word ptr es:[bx][0],0
	mov	word ptr es:[bx][2],0
	add	bx,4
	cmp	bx,di
	jb	rm_remove_small_blocks

rm_null_set:
	loop	rm_next_contiguous_set
	pop	di 

; Figure out if we're going to put library code and resources above the line.
;  This happens only if there is at least 208K (exclusive of a page frame)
;  available to swap.

	xor	ax,ax
	mov	bx,di
	mov	cx,cs:EMSPageCount
amount_left_loop:
	cmp	word ptr es:[bx][0],0
	jz	not_here
	inc	ax
not_here:
	add	bx,4
	loop	amount_left_loop
	mul	cs:EMSPageSize
	cmp	cs:EMSPageFrame,0
	jz	inclusive
	sub	ax,64  * (KBYTES / PARAGRAPH)
inclusive:
	cmp	ax,208 * (KBYTES / PARAGRAPH)
	jb	sharp_knife

; do full scale EMS only if there is at least 512K available.

	push	di
	mov	ah,bEMM_PAGE_COUNTS
	int	EMM_API
	mov	di,codeOFFSET int13_status
	add	bx,cs:[di].current_size
	sub	bx,cs:[di].minimum_size
	cmp	bx,32			; BX = pages available
	pop	di
	jb	sharp_knife
	or	cs:EMSFlags,EMSF_LIM_40
	or	cs:EMSFlags,EMSF_LIB_CODE
	or	cs:EMSFlags,EMSF_DATA_ABOVE
	or	cs:EMSFlags,EMSF_RESOURCES_ABOVE
	jmp	short put_everything_above

; If here then only task code goes above.  Truncate the list to what is
;  above A000h.

sharp_knife:
	mov	bx,di
	mov	cx,cs:EMSPageCount
castrate:
	cmp	es:[bx][0],0A000h
	jae	adjust_PID0_pages
	mov	word ptr es:[bx][0],0
	mov	word ptr es:[bx][2],0
	add	bx,4
	loop	castrate
adjust_PID0_pages:
	mov	cs:PID0_pages_high,0
	mov	cs:EMS_calc_swap_line,0A000h

put_everything_above:

; Calculate the number of pages of PID0 that are below the line.

	xor	ax,ax
	mov	bx,di
count_em:
	cmp	word ptr es:[bx],0
	jnz	got_em
	inc	ax
	add	bx,4
	jmp	count_em
got_em:	cmp	ax,cs:PID0_pages	; Have we counted more pages?
	jbe	sto_em
	mov	ax,cs:PID0_pages
sto_em:	mov	cs:PID0_pages_low,ax

; 2a) Condense the list down into sets of [start seg (para), length (para)]

	xor	bx,bx
	call	condense_list
	cmp	cs:fEEMS,0		; Is there anything left?
	jnz	got_a_swap
	mov	sp,si			; clean the array off the stack!
	jmp	testemm_done
got_a_swap:
	or	cs:EMSFlags,EMSF_LIM_40
	or	cs:EMSFlags,EMSF_RESOURCES_ABOVE

; 3a) Construct a set of entries excluding the 64K LIM Window.

	mov	ax,cs:EMSPageFrame
	or	ax,ax
	jz	no_page_frame

; 3b) Remove the page frame from consideration.

	mov	bx,di
find_it:
	cmp	ax,es:[bx][0]
	jz	found_it
	add	bx,4
	jmp	find_it
found_it:
	mov	word ptr es:[bx][00],0
	mov	word ptr es:[bx][04],0
	mov	word ptr es:[bx][08],0
	mov	word ptr es:[bx][12],0

; 4) Remove blocks less than 48K in size ------------------------------

	push	di
	mov	cx,cs:EMSPageCount
tm_next_contiguous_set:
	mov	bx,di
	mov	ax,es:[di][0]
	mov	dx,ax
tm_still_contiguous:
	add	dx,cs:EMSPageSize
	add	di,4
	or	ax,ax
	jz	tm_null_set
	cmp	dx,es:[di][0]
	jne	tm_got_a_set
	loop	tm_still_contiguous
tm_got_a_set:
	sub	dx,ax			; get the length
	cmp	dx,48 * (KBYTES / PARAGRAPH)
	jae	tm_null_set
tm_remove_small_blocks:
	mov	word ptr es:[bx][0],0
	mov	word ptr es:[bx][2],0
	add	bx,4
	cmp	bx,di
	jb	tm_remove_small_blocks

tm_null_set:
	loop	tm_next_contiguous_set
	pop	di 

; 3c) Condense the list down into sets of [start seg (para), length (para)].

no_page_frame:
	mov	bx,codeOffset AltfEEMS - codeOffset fEEMS
	call	condense_list

	mov	sp,si			; clean the array off the stack!

; Determine the minimum number of free pages required to load a new task.
;  We will only give a task its own set of banks if there are enough
;  pages to completely fill the banking area available or 256K worth
;  of pages, whichever is less.  For example, on an AboveBoard this
;  means 64K, while for an AST it probably means 256K.	256K is an
;  arbitrary number but it seems right.

	xor	bx,bx
	xor	di,di
	mov	cx,cs:fEEMS
there:	mov	ax,cs:segSwapSize[di]
	xor	dx,dx
	div	cs:EMSPageSize
	add	bx,ax
	add	di,2
	loop	there
	mov	ax,256 * (KBYTES / PARAGRAPH)
	xor	dx,dx
	div	cs:EMSPageSize
	cmp	bx,ax
	jb	use_bank_size
	mov	bx,ax
use_bank_size:
	mov	cs:EMSTaskPageSize,bx

; On the PS|2 Model 80 mouse info is stored in the 9F00 BIOS page.
;  Because of this we must have interrupts off while switching the EMS
;  pages around.  Under Win386 no interrupts will be serviced while
;  in protected mode, that is, while in the EMM.  On the Vectra?????

; Readjust the DOS top of memory for WIN200.BIN.

	cmp	cs:PID0_pages_high,0
	jz	testemm_done
	mov	ax,cs:DOS_top_of_memory
	mov	bx,cs:PID0_pages
	mov	dx,cs:EMSPageSize
	neg	dx
	and	ax,dx			; round down to nearest page size
	mov	dx,ax
	mov	es,cs:topPDB
	xchg	es:[2],ax		; new PSP top of memory
	sub	ax,dx			; AX = old - new
	mov	cs:EMSVectraDelta,ax
	mov	dx,es
	dec	dx
	mov	es,dx
	sub	es:[ga_size],ax		; new partition size
	mov	cs:PID0_top_of_memory,ax

testemm_done:
	pop	es
	pop	ds
	pop	si
	pop	di
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
cEnd	nogen


;-----------------------------------------------------------------------;
; condense_list								;
; 									;
; 									;
; Arguments:								;
;	BX = LIM index							;
;									;
; Returns:								;
; 									;
; Error Returns:							;
; 									;
; Registers Preserved:							;
;	DI,SI,DS,ES							;
;									;
; Registers Destroyed:							;
;	AX,BX,CX,DX							;
;									;
; Calls:								;
; 									;
; History:								;
; 									;
;  Sat Oct 24, 1987 12:34:01p  -by-  David N. Weise   [davidw]		;
; Wrote it.								;
;-----------------------------------------------------------------------;

cProc	condense_list,<PUBLIC,NEAR>
cBegin nogen
	push	di
	push	si
	mov	si,bx
	mov	cx,cs:EMSPageCount
next_contiguous_set:
	mov	ax,es:[di][0]
	mov	dx,ax
still_contiguous:
	add	dx,cs:EMSPageSize
	add	di,4
	or	ax,ax
	jz	null_set
	cmp	dx,es:[di][0]
	jne	got_a_set
	loop	still_contiguous
got_a_set:
	sub	dx,ax			; get the length
	mov	cs:segSwapArea[bx],ax
	mov	cs:segSwapSize[bx],dx
	add	bx,2
	inc	cs:fEEMS[si]
	cmp	cs:fEEMS[si],3
	jz	scan_array_done
null_set:
	jcxz	scan_array_done
	loop	next_contiguous_set
scan_array_done:
	pop	si
	pop	di
	ret
cEnd nogen


;-----------------------------------------------------------------------;
; Init_EMS								;
;									;
; The first instance of an app is given its PID (and own banks).	;
; Any instances following share the PID and banks of the first instance.;
; Just enough banks are swapped in to create partition headers and	;
; room for the master object.  A global arena structure is created.	;
;									;
; Arguments:								;
;	none								;
;									;
; Returns:								;
;	nothing 							;
;									;
; Error Returns:							;
;									;
; Registers Preserved:							;
;	DS								;
;									;
; Registers Destroyed:							;
;	AX,BX,CX,DX,SI,DI,ES						;
;									;
; Calls:								;
;	EMMGlobalInit							;
;									;
; History:								;
;									;
;  Fri Apr 24, 1987 07:47:30p  -by-  David N. Weise	[davidw]	;
; Added the special case of WinOldAp					;
;									;
;  Tue Apr 21, 1987 06:07:10p  -by-  David N. Weise	[davidw]	;
; Put in the entry tables in page 0 support.				;
;									;
;  Mon Mar 30, 1987 10:23:58p  -by-  David N. Weise	[davidw]	;
; Finally made the above description correct.				;
;									;
;  Wed Mar 25, 1987 10:23:23p  -by-  David N. Weise	[davidw]	;
; Got rid of the TDB_EMSBitArray.					;
;									;
;  Sat Feb 28, 1987 06:28:51p  -by-  David N. Weise	[davidw]	;
; Put in support for multiple instances sharing the same space. 	;
;									;
;  Tue Sep 02, 1986 04:37:48p  -by-  David N. Weise	[davidw]	;
; Wrote it.								;
;-----------------------------------------------------------------------;

cProc	Init_EMS,<PUBLIC,NEAR>

cBegin	nogen

	test	cs:EMSFlags,EMSF_LIB_CODE
	jnz	init_it
	ret
init_it:

; On the PS|2 Model 80 mouse info is stored in the 9F00 BIOS page.
;  Because of this we must have interrupts off while switching the EMS
;  pages around.  Under Win386 no interrupts will be serviced while
;  in protected mode, that is, while in the EMM.  On the Vectra?????

; Copy any high pages down low that have to be preserved.  They
;  will be mapped back where they were in a moment.

	mov	cx,cs:PID0_pages_high
	jcxz	no_high_pages_to_save
	cli
	push	ds
	mov	ds,cs:PID0_high_start
	xor	si,si
	mov	es,cs:segSwapArea[0]
	xor	di,di
	mov	ax,cs:EMSPageSize
	mul	cx
	mov	cx,ax
	shl	cx,1
	shl	cx,1
	shl	cx,1
	cld
	rep	movsw
	pop	ds
no_high_pages_to_save:

; Map out PID 0's old pages.

	push	ds
	cmp	cs:segSwapSize[0],0
	jz	dont_touch_PID0
	mov	bp,sp
	xor	bx,bx
	mov	si,-1
map_out_all:
	xor	dx,dx
	mov	ax,cs:segSwapSize[0]
	div	cs:EMSPageSize
	mov	cx,ax
	mov	ax,cs:segSwapArea[0]
page_by_page:
	push	ax
	push	si
	add	ax,cs:EMSPageSize
	loop	page_by_page
	mov	cx,cs:PID0_pages_high
	jcxz	no_pages_high
	mov	si,cs:PID0_pages_low
remap_high:
	push	ax
	push	si
	add	ax,cs:EMSPageSize
	inc	si
	loop	remap_high
no_pages_high:
	mov	cx,bp
	sub	cx,sp
	shr	cx,1
	shr	cx,1
	mov	si,sp
	push	ss
	pop	ds
	mov	ax,wEMM_MAP_PHYSICAL_PAGES
	int	EMM_API
	sti
	mov	sp,bp			; clear the stack

; Free up unnecessary pages associated with PID0.

	mov	bx,cs:PID0_pages_low
	add	bx,cs:PID0_pages_high
	xor	dx,dx
	mov	ah,bEMM_REALLOCATE_FOR_PID
	int	EMM_API
dont_touch_PID0:
	pop	ds
	sti


; figure out how many pages we need

	mov	ax,cs:segAltSwapSize[0]
	add	ax,cs:segAltSwapSize[2]
	add	ax,cs:segAltSwapSize[4]
	xor	dx,dx
	div	cs:EMSPageSize
	mov	cx,ax
	test	cs:EMSFlags,EMSF_LIB_CODE
	jz	dont_need_entries
	inc	cx			; and 1 more for entry tables
dont_need_entries:
	xor	bx,bx
	mov	ax,wEMM_GET_RAW_PID
	int	EMM_API
	or	ah,ah
	jnz	eems_error
	mov	cs:PID_for_fake,dx

; get the pages wanted

	xor	ax,ax
	mov	bx,cx
	call	get_EMS_pages
	jc	eems_error

; now map those pages in

	push	bx			; save number of pages allocated
	xor	bx,bx
	mov	cx,cs:AltfEEMS
	push	bp
	mov	bp,sp			; just to save sp somewhere!!!
	mov	di,1
map_WinOldAp_loop:
	mov	si,cx
	mov	ax,cs:segAltSwapSize[bx]
	xor	dx,dx
	div	cs:EMSPageSize
	mov	cx,ax
	mov	ax,cs:segAltSwapArea[bx] ; map in the first boundary
do_this_bank:
	push	ax
	push	di
	add	ax,cs:EMSPageSize
	inc	di			; start on page 1, reserve 0 for e.t.
	loop	do_this_bank
	inc	bx
	inc	bx
	mov	cx,si
	loop	map_WinOldAp_loop

array_in_place:
	mov	cx,bp
	sub	cx,sp
	shr	cx,1
	shr	cx,1			; calculate # of entries on stack
	mov	dx,cs:PID_for_fake
	mov	si,sp
	push	ds
	push	ss
	pop	ds
	mov	ax,wEMM_MAP_PHYSICAL_PAGES
	int	EMM_API
	pop	ds
	mov	sp,bp
	pop	bp
	pop	cx			; number of pages allocated
	or	ah,ah
	jnz	eems_error
no_EEMS:
	ret

eems_error:
	int	3			; TEMPORARY should handle this better
	ret
cEnd	nogen


;-----------------------------------------------------------------------;
; check_calls								;
; 									;
; Because the AST driver is a 3.2 driver we must check do a special	;
; check for it.								;
;									;
; Arguments:								;
;	none								;
;									;
; Returns:								;
;	CF = 0								;
;									;
; Error Returns:							;
;	CF = 1								;
;									;
; Registers Preserved:							;
;	all								;
;									;
; Registers Destroyed:							;
; 									;
; Calls:								;
; 									;
; History:								;
; 									;
;  Tue Nov 10, 1987 07:27:26p  -by-  David N. Weise   [davidw]		;
; Wrote it.								;
;-----------------------------------------------------------------------;

cProc	check_calls,<PUBLIC,NEAR>
cBegin nogen
	push	ax
	push	bx
	push	cx
	push	dx

; check for Function 18

	xor	dx,dx
	mov	ah,bEMM_GET_PIDS_PAGES
	int	EMM_API
	or	ah,ah
	jnz	no_way

	mov	cx,bx
	mov	ah,bEMM_REALLOCATE_FOR_PID
	int	EMM_API
	or	ah,ah
	jnz	no_way
	cmp	bx,cx
	jnz	no_way

; check for Function 25

	mov	ax,wEMM_MAPPABLE_ARRAY_LEN
	int	EMM_API
	or	ah,ah
	jnz	no_way

; check for Function 27

	xor	bx,bx
	mov	ax,wEMM_GET_RAW_PID
	int	EMM_API
	or	ah,ah
	jnz	no_way

	mov	ah,bEMM_FREE_PID
	int	EMM_API
	or	ah,ah
	jnz	no_way

; check for Function 28

	mov	ax,wEMM_REGISTER_SET_LEN
	int	EMM_API
	or	ah,ah
	jnz	no_way

	clc
	jmp	short yes_way

no_way:	stc
yes_way:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret

cEnd nogen


;-----------------------------------------------------------------------;
; get_EMS_pages 							;
;									;
; Gets the pages wanted by doing a realloc call.  If pages aren't       ;
; available we look for SMARTDrive and try to get the pages from him.	;
;									;
; Arguments:								;
;	AX = present # pages owned					;
;	BX = additional # pages wanted					;
;	DX = EMS PID to use						;
;									;
; Returns:								;
;	CF = 0								;
;	BX = total pages owned						;
;									;
; Error Returns:							;
;	CF = 1								;
;									;
; Registers Preserved:							;
;	CX,DX,DI,SI,DS.ES						;
;									;
; Registers Destroyed:							;
;	AX								;
;									;
; Calls:								;
;									;
; History:								;
;									;
;  Sat May 09, 1987 11:16:01p  -by-  David N. Weise   [davidw]		;
; Wrote it.								;
;-----------------------------------------------------------------------;

cProc	get_EMS_pages,<PUBLIC,NEAR>
cBegin nogen
	push	cx
	push	di
	push	si
	push	ds
	push	dx
	push	cs
	pop	ds
	assumes ds,CODE
	mov	ds:return_number,ax	; temp storage
	mov	ds:get_number,bx
	mov	si,bx			; save delta pages
	add	bx,ax
	mov	ah,bEMM_REALLOCATE_FOR_PID
	int	EMM_API
	or	ah,ah
	jz	gep_success

; see how many pages are available

	call	check_SMARTDrive

	pop	dx
	push	dx
	mov	bx,si
	add	bx,ds:return_number
	mov	ah,bEMM_REALLOCATE_FOR_PID
	int	EMM_API
	or	ah,ah
	jnz	gep_failure

gep_success:
	clc
	jmp	short gep_done

gep_failure:
	mov	bx,ds:return_number
	stc
gep_done:
	pop	dx
	pop	ds
	pop	si
	pop	di
	pop	cx
	ret
cEnd nogen


;-----------------------------------------------------------------------;
; check_SMARTDrive							;
;									;
; Checks to see if our disk cacher is installed.			;
;									;
; Arguments:								;
;	none								;
;									;
; Returns:								;
;									;
; Error Returns:							;
;									;
; Registers Preserved:							;
;	all								;
;									;
; Registers Destroyed:							;
;									;
; Calls:								;
;	nothing 							;
;									;
; History:								;
;									;
;  Sun May 10, 1987 05:19:50p  -by-  David N. Weise   [davidw]		;
; Wrote it.								;
;-----------------------------------------------------------------------;

SMARTDrive	db	"SMARTAAR",0

get_sd_pages	db	0Bh
get_number	dw	0

return_sd_pages db	0Ch
return_number	dw	0

int13_status	equ	this byte
SMARTDrive_status	<>

cProc	check_SMARTDrive,<PUBLIC,NEAR>
cBegin nogen
	push	ax
	push	bx
	push	cx
	push	dx
	push	si
	push	ds
	push	cs
	pop	ds
	mov	dx,codeOFFSET SMARTDrive
	mov	ax,3D02h
	int	21h			; open the device
	jc	cfs_done		; no device
	mov	bx,ax
	mov	ax,4400h
	int	21h			; make sure it is a device
	jc	close_no_dev
	test	dx,4080h
	jz	close_no_dev

; see how many pages are available

	mov	cx,SIZE SMARTDrive_status
	mov	dx,codeOFFSET int13_status
	mov	ax,4402h		; IOCTL Read
	int	21h
	jc	close_no_dev
	cmp	ax,cx
	jnz	close_no_dev
	mov	si,dx
	mov	ax,[si].current_size
	sub	ax,[si].minimum_size
	jz	close_no_dev		; not enough pages

; free those pages from SMARTDrive

	mov	cx,3
	mov	dx,codeOFFSET get_sd_pages
	mov	ax,4403h		; IOCTL write
	int	21h
close_no_dev:
	mov	ax,3E00h		; close
	int	21h
cfs_done:
	pop	ds
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret

cEnd nogen
