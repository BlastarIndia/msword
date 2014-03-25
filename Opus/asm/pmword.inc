PushArg	MACRO	arg_list		;; Push multiple registers
	irp	x,<arg_list>
		push	x
	endm
ENDM
wo	equ	word ptr
by	equ	byte ptr

FDE_OS2	STRUC
	wDateCreateFdeOS2	dw	?
	wTimeCreateFdeOS2	dw	?
	wDateAccessFdeOS2	dw	?
	wTimeAccessFdeOS2	dw	?
	wDateWriteFdeOS2	dw	?
	wTimeWriteFdeOS2	dw	?
	OFF_cbFileFdeOS2	dw	?	; The size of
	SEG_cbFileFdeOS2	dw	?	;   the file.
	OFF_cbFileAllocFdeOS2	dw	?	; The space alloc'ed on the
	SEG_cbFileAllocFdeOS2	dw	?	;   disk to hold the file.
	attrFileFdeOS2		dw	?
	cchNameFdeOS2		db	?
	szNameFdeOS2		db	13 dup (?)
FDE_OS2	ENDS

FDE	STRUC
	dtaFde		DB	21 DUP (0)	;* 21 bytes filler
	atrFde		DB	0
	wTimeWriteFde	DW	0
	wDateWriteFde	DW	0
	cbFileFde	DD	0
	szNameFde	DB	13 DUP (0)	;* file name
FDE	ENDS


qfs	STRUC
	idFileSystem	dd	?	; Specifies the file-system id'er.
	cSectorUnit	dd	?	; sectors/cluster
	cUnit		dd	?	; clusters/drive
	cUnitAvail	dd	?	; free clusters
	cbSect		dw	?	; bytes/sector
qfs	ENDS

DATETIME	STRUC
	hours		db	?
	minutes		db	?
	seconds		db	?
	hundreths	db	?
	day		db	?
	month		db	?
	year		dw	?
	timezone	dw	?
	weekday		db	?
DATETIME	ENDS
