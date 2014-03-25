ifndef OPUS
	?QC = 1
	memM = 1
	?PLM = 1
	?WIN = 0
	?TF = 0
	?DF = 0

	.xlist
	include hcmacros.inc
	include pcode.inc
	include macros.inc
	.list
else

	.xlist
        memS    = 1
        ?WIN    = 1
        ?PLM    = 1
        ?NODATA = 1
        ?TF     = 1
        include w2.inc
        include noxport.inc
	.list
endif
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;	Extra declarations
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

cpMax	equ	07fffffffh
seg_cpMax equ	7fffh
off_cpMax equ	0ffffh
dcpMax	equ	0ff00h

fcNil	equ	0ffffffffh
seg_fcNil equ	0ffffh
off_fcNil equ	0ffffh
