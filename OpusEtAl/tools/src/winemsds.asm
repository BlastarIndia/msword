EMSF_EXISTS		EQU	01h	; EMS exits.
EMSF_LIM_40		EQU	02h	; LIM 4.0 is available.
EMSF_LIB_CODE		EQU	04h	; Library code segs above the line.
EMSF_DATA_ABOVE 	EQU	08h	; Task data segments above the line.
EMSF_RESOURCES_ABOVE	EQU	10h	; Task resources above the line.
EMSF_SEARCH_PID_STACKS	EQU	20h	; PatchStack search this PIDs stacks.
EMSF_SMARTDRIVE 	EQU	40h	; SMARTDrive exists.
EMSF_DONT_FREE		EQU	80H	; Do not free this bank 

EMSF1_PREV_HANDLE	EQU	01h	; A user of EMS exists before Windows.
EMSF1_ADVANCED_OFF	EQU	02h	; User said don't use advanced EMS (/N)
EMSF1_AST		EQU	04h	; This is the AST 3.2 driver.
EMSF1_COMPACT_EMS	EQU	08h	; Kernel wants EMS heap compacted.



include eems.inc
include winkern.inc

; DO NOT TOUCH THE FOLLOWING VARIABLES

globalW fEEMS,0 	; Non-zero if an advanced EEMS driver is present
			; and is the number of swap areas
globalW segSwapArea,-1,3	; List of up to 3 EEMS swap areas
globalW segSwapSize,0,3 	; List of up to 3 EEMS swap area sizes
globalW altfEEMS,0		; Number of swap areas excluding LIM WINDOW
globalW segAltSwapArea,-1,3	; EEMS swap areas excluding LIM WINDOW
globalW segAltSwapSize,0,3	; EEMS swap area sizes excluding LIM WINDOW
globalW PID0_pages,0		; #pages PID0 originally started with
globalW PID0_pages_low,0	; #pages PID0 has below the line
globalW PID0_pages_high,0	; #pages PID0 has above the line
globalW PID0_high_start,0	; seg address of where high pages start
globalB fEMM,0		; Non-zero if Intel/Above board Extended Memory
			; Manager is present and is the size of the
			; context save area
globalB EMSFlags,0,2		; Flags are always a good idea.
globalW EMSPageFrame,0		; The segment of the EMM page frame
globalW EMSPageSize,0		; The raw EEMS page size
globalW EMSPageCount,0		; Total number of mappable banks.
globalW EMSTaskPageSize,0	; Min free banks required to start a task.
globalW PID_for_fake,0		; the PID allocated for the fake task
globalW DOS_top_of_memory,0
globalW PID0_top_of_memory,0
globalW EMS_calc_swap_line,0	; The calculated swap line
globalW EMSVectraDelta,0		; Don't ask.

; DO NOT TOUCH THE ABOVE VARIABLES
