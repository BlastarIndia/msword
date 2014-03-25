        include w2.inc
        include noxport.inc
	include consts.inc
	include windows.inc

errNull 	equ	0
errDiv0 	equ	(errNull+7)
errVal		equ	(errDiv0+8)
errRef		equ	(errVal+8)
errName 	equ	(errRef+6)
errNum		equ	(errName+7)
errNa		equ	(errNum+6)
errMax		equ	(errNa+5)
errNil		equ	errMax

createSeg	trans,trans,byte,public,CODE

externFP	<DADD,DMUL,DSUB,DDIV,DNEG,DINT,SFLOAT,FIX,DDIV2>
externFP	<SetSbCur, ReloadSb>

OverErr 	EQU	0001H	;Overflow error indicator
UnderErr	EQU	0002H	;Underflow error indicator
DivBy0		EQU	0004H	;Divide by 0 indicator
TransErr	EQU	0008H	;Transcendental error indicator

POLY1	EQU	8000h		; flag for leading 1 in poly
EXPMASK EQU	07FF0h
EXPBIAS EQU	03FF0h
EXPSHFT EQU	4
MANBITS EQU	53
OF_EXP	EQU	6
OF_SGN	EQU	7

sBegin	data
externW mpsbps
externW pdAcc
externW fError
externW AC_HI
externW AC_LO
externB round_flag
externW round_exp
externW f8087

ArgTran 	DQ	?
TempTran1	DQ	?
TempTran2	DQ	?
TempTran3	DQ	?

SgnTrig 	DB	?	; sign/cosine flag


ILNinterval	DQ	03FF71547652B82FER	; 1/ln(2)
ISQRbase	DQ	03FE6A09E667F3BCDR	; 1/sqr(2)

LNXMAX		DQ	040862E42FEFA39EER	;  1024*ln(2)-2eps
LNXMIN		DQ	0C086232BDD7ABCD1R	; -1022*ln(2)+2eps

ExpC1		DQ	03FE6300000000000R	; ln(2)
ExpC2		DQ	0BF2BD0105C610CA8R

LogATab 	DW	2
		DQ	0BFE94415B356BD29R	;-0.78956112887491257267E+00
		DQ	04030624A2016AFEDR	;+0.16383943563021534222E+02
		DQ	0C05007FF12B3B59AR	;-0.64124943423745581147E+02

LogBTab 	DW	3
numOne		DQ	03FF0000000000000R	;+0.10000000000000000000E+01
		DQ	0C041D5804B67CE0FR	;-0.35667977739034646171E+02
		DQ	040738083FA15267ER	;+0.31203222091924532844E+03
		DQ	0C0880BFE9C0D9077R	;-0.76949932108494879777E+03

ExpPTab 	DW	2
		DQ	03EF152A46F58DC1CR	;+0.165203300268279130E-04
		DQ	03F7C70E46FB3F6E0R	;+0.694360001511792852E-02
		DQ	03FD0000000000000R	;+0.249999999999999993E+00

ExpQTab 	DW	2
		DQ	03F403F996FDE3809R	;+0.495862884905441294E-03
		DQ	03FAC718E714251B3R	;+0.555538666969001188E-01
numHalf 	DQ	03FE0000000000000R	;+0.500000000000000000E+00

sEnd	data

sBegin	trans
	assumes cs,trans
	assumes ds,dgroup
	assumes ss,dgroup

; %%Function:LOGNAT %%Owner:bryanl
cProc	LOGNAT,<PUBLIC,NEAR>
	LocalQ	LOG_LO
	LocalQ	LOG_HI
cBegin
	call	ResetSbTrans
	mov	ax,[si+of_exp]		; save copy of sign:exp:himan
	mov	bx,ax
	AND	BX,8000h+expmask	;Taking log of zero?
	jg	LnPos
	mov	[ferror],TransErr
	jmp	LnExit
LnPos:
	call	LogCore
	mov	ax,dataOffset AC_HI
	cCall	DADD,<ax>
	lea	ax,LOG_HI
	cCall	DADD,<ax>		; AC = log(x) (normal precision)
LnExit:
cEnd

LogCore:
	AND	AX,not expmask
	OR	AX,expbias-(1 shl expshft)
	mov	[si+of_exp],ax		   ; Convert DAC to range [.5,1)
	XCHG	AX,BX
	SUB	AX,expbias-(1 shl expshft) ; Convert exponent to 16-bit integer
	mov	cl,expshft
	sar	ax,cl			   ; shift to integer
	PUSH	AX
	mov	di,dataOffset ArgTran
	call	dstdi

; At this point AC = Y

	mov	ax,dataOffset ISQRbase
	cCall	DSUB,<ax>		;AC < 1/SQR(base)
	mov	si,[si+of_exp]
	call	dlddi
	or	si,si
	pop	si			; get exponent
	mov	cl,00010000b		;  assume in range AC < 1.0
	jg	dlog1			; No - in range

	dec	si			;  and adjust exponent down 1
	add	word ptr [pdAcc+of_exp],1 shl expshft  ; * 2
	mov	cl,00001000b		; Y >= 1.0	divide adjustment

dlog1:
	push	cx			; save divide adjustment
	call	dstdi
	cCall	SFLOAT,<si>		; AC = XN
	mov	si,dataOffset TempTran1
	mov	bx,si
	call	dst
	mov	ax,dataOffset ExpC2
	cCall	DMUL,<ax>
	lea	bx,LOG_LO
	call	dst			; LOG_LO = XN*C2
	mov	bx,si
	call	dld
	mov	ax,dataOffset ExpC1
	cCall	DMUL,<ax>
	lea	bx,LOG_HI
	call	dst
	mov	ax,word ptr [pdAcc+of_exp]	; get exponent
	and	ax,expmask
	shl	ax,1			; shift into highest bits
	mov	[ROUND_EXP],ax		; set round threshold

	call	dlddi
	mov	ax,dataOffset numOne
	cCall	DSUB,<ax>		; AC = Y-1
	pop	ax
	mov	[ROUND_FLAG],al 	; set special round flag for divide
	cCall	DDIV2,<di>		; AC = 2*(Y-1)/(Y+1)  (trick divide)

	mov	bx,dataOffset LogATab
	mov	cx,dataOffset LogBTab
	call	z3p_q			; z^3*A(z*z)/B(z*z)
	mov	ax,dataOffset AC_LO
	cCall	DADD,<ax>
	lea	si,LOG_LO
	cCall	DADD,<si>		; LOG_LO = XN*C2+lo(S)+S*r(S^2)
	ret


; %%Function:LN %%Owner:bryanl
cProc	LN,<PUBLIC,FAR>,<si,di>
cBegin
	cCall	LOGNAT
cEnd


;***  EXP - exponential function

; %%Function:ETOX %%Owner:bryanl
cProc	ETOX,<PUBLIC,NEAR>
cBegin
	call	ResetSbTrans
	mov	di,dataOffset ArgTran	; save X in argtran
	call	dstdi
	call	EXPrange		; check for proper range
	MOV	AX,word ptr [SI+of_exp] ; get exponent
	AND	AX,expmask		; mask off sign, hi mantissa
	CMP	AX,expbias-((manbits+1) shl expshft) ; less than 2^(-54)
	jae	exp1
;
;  Return ONE
;
	mov	bx,dataOffset numOne
	call	dld
	jmp	short ExpRet
;
;  Return Zero
;
retzero:
	mov	di,dataOffset pdAcc
	push	ss
	pop	es
	xor	ax,ax
	stosw
	stosw
	stosw
	stosw
	jmp	short ExpRet
;
;  Overflow
;
ovrflw2:
	pop	ax			; toss one more level off stack
ovrflw:
	mov	[fError],OverErr
	jmp	short ExpRet
;
;  No errors, proceed
;
exp1:
	CALL	EXPM1

EXPadjust:				; finish up EXP
	PUSH	AX			; save integer part
	mov	ax,dataOffset numHalf
	cCall	DADD,<ax>		; AC = 0.5+g*P(z)/(Q(z)-g*P(z))
	POP	AX			; restore n
	inc	ax
	mov	cl,expshft
	sal	ax,cl
	add	word ptr [si+of_exp],ax ; adjust exponent
expRet:
cEnd


EXPM1:
	mov	di,dataOffset ILNinterval
	cCall	DMUL,<di>		; x/ln(interval)
	mov	al,[si+of_sgn]		; get sign
	mov	[SgnTrig],al
	and	byte ptr [si+of_sgn],01111111b	; make it positive
	mov	ax,dataOffset numHalf
	cCall	DADD,<ax>		; round to nearest int
	cCall	FIX			; get the int
	test	[SgnTrig],10000000b	; test the sign
	jz	expos
	neg	ax
expos:
	call	LNreduce		; reduce ARG by ax*LN(2)

EXPM1reduced:				; compute (EXP(X)-1)/2
	push	ax			; save n
	mov	bx,dataOffset ExpPTab
	mov	cx,dataOffset ExpQTab
	call	p_q
	mov	si,dataOffset TempTran3 ; point to z
	cCall	DMUL,<si>		; ac = z*P(z*z)
	mov	bx,si
	call	dst			; Temp3 = z*P(z*z)
	cCall	DNEG			; ac = -z*P(z*z)
	mov	di,dataOffset TempTran1 ; point to Q(z*z)
	cCall	DADD,<di>		; ac = Q(z*z)-z*P(z*z)
	call	dstdi			; Temp1 = ac
	mov	bx,si
	call	dld			; ac = z*P(z*z)
	cCall	DDIV,<di>		; AC = z*P(z*z)/(Q(z*z)-z*P(z*z))
	mov	si,dataOffset pdAcc	; si = ac
	pop	ax			; get integer part
	ret


LNreduce:				; reduce ARG by LN(2)
	push	ax			; save n
	cCall	SFLOAT,<ax>		; XN
	mov	bx,dataOffset ExpC1
	mov	cx,dataOffset ExpC2
	call	REDUCE
	pop	ax			; get integer part
	ret


overflowrange:
	mov	ax,transOffset ovrflw2	; overflow on upper limit
	jmp	short range
EXPrange:
	mov	bx,transOffset retzero	; standard returns for EXP
	mov	ax,transOffset ovrflw	; overflow on upper limit
range:
	push	ax
	push	bx
	mov	ax,dataOffset LNXMAX
	cCall	DSUB,<ax>
	cmp	byte ptr [si+of_sgn],0
	jg	rangeax 		; overflow on upper limit
	mov	di,dataOffset ArgTran
	call	dlddi
	mov	ax,dataOffset LNXMIN
	cCall	DSUB,<ax>
	cmp	byte ptr [si+of_sgn],0	; check lower limit
	jl	rangebx
	call	dlddi
	add	sp,4			; clean ax,bx off stack
	ret

rangeax:
	pop	bx
	pop	ax
	pop	bx			; toss return address
	jmp	ax			; return for x > LNXMAX

rangebx:
	pop	bx
	pop	ax
	pop	ax			; toss return address
	jmp	bx			; return for x < LNXMIN

; %%Function:EXP %%Owner:bryanl
cProc	EXP,<PUBLIC,FAR>,<si,di>
cBegin
	cCall	ETOX
cEnd



;*** SQR - Double precision square root
;
;  This routine takes the square root of the number in the accumulator,
;  and leaves the result in the accumulator.
;
;  Algorithm:  x  is  initially  adjusted so that the exponent is even
;  (when the exponent is odd  the  exponent  is  incremented  and  the
;  mantissa is	shifted  right one bit).  The exponent is then divided
;  by two and resaved.	A single word  estimate  of  y	(the  root  of
;  x) accurate to 5 bits is produced using a wordlength implementation
;  of a linear polynomial approximation of  sqrt  x.   Three  or  four
;  Newton-Raphson iterations are then computed as follows:
;
;  1) qa*ya+r1a=xa:0h,ya=(ya+qa)/2
;  2) qa*ya+r1a=xa:xb,ya=(ya+qa)/2
;  3) qa*ya+r1a=xa:xb,qb*ya+r2a=r1a:xc,ya:yb=(ya:yb+qa:qb)/2
;  *** iteration 4 implemented with standard divide ***
;  4) qa*ya+r1a=xa:xb,p1a:p1b=qa*yb,
;     if p1a<r1a continue
;	 if p1a=r1a
;	    if p1b=xc continue else r1a=r1a+ya,qa=qa-1 endif,
;	 else r1a=r1a+ya,qa=qa-1,p1a:p1b=p1a:p1b-yb endif,
;     r1a:r1b=r1a:xc-p1a:p1b,
;     qb:ya+r2a=r1a:r1b,p2a:p2b=qb*yb,
;     if p2a<r2a continue
;	 if p2a=r2a
;	    if p2b=xd continue else r2a=r2a+ya,qb=qb-1 endif,
;	 else r2a=r2a+ya,qb=qb-1,p2a:p2b=p2a:p2b-yb endif,
;     r2a:r2b=r2a:xd-p2a:p2b,
;     qc*ya+r3a=r2a:r2b,p3a:p3b=qc*yb,
;     if p3a>=r3a then r3a=r3a+ya,qc=qc-1 endif,
;     r3a=r3a-p3a,qd*ya+r4a=r3a:0h,
;     ya:yb:yc:yd=(ya:yb:0h:0h+qa:qb:qc:qd)/2

; %%Function:SQRT %%Owner:bryanl
cProc	SQRT,<PUBLIC,NEAR,ATOMIC>
cBegin
	push	bp
	call	ResetSbTrans
	mov	di,dataOffset TempTran1
	MOV	AX,[SI+6]
	MOV	BH,AL			; Save hi mantissa
	TEST	AX,07FF0h		; Test exponent
	jz	sqrxv			;   Zero - return 0

	AND	AX,0FFF0h		; Mask off hi mantissa
	jns	sqr1			;   Negative - error
	mov	[fError],TransErr	;   set transcendental error
sqrxv:	jmp	sqrx

sqr1:
	SUB	AX,03FE0h		; Remove excess from exponent
	MOV	BL,[SI+5]
	MOV	CX,[SI+3]
	MOV	DX,[SI+1]		;get first three mantissa words of x

	SHL	DX,1
	RCL	CX,1
	RCL	BX,1

	SHL	DX,1
	RCL	CX,1
	RCL	BX,1

	SHL	DX,1
	RCL	CX,1
	RCL	BX,1

	OR	BH,080h 		;implied leading 1 now explicit

	TEST	AL,16			;if exponent is even
	JZ	EXPEVEN 		;   bypass adjust
	ADD	AX,16			;increment exponent
	SHR	BX,1
	RCR	CX,1
	RCR	DX,1			;divide mantissa by 2

EXPEVEN:
	SAR	AX,1			;divide exponent by 2

	ADD	AX,03FE0h		; Bias the exponent
	MOV	[DI+6],AX		; Store exponent of y
	CMP	BX,0FFFEH		;if mantissa < 0.FFFEh
	JB	NOTNEARONE1		;   perform main root routine
	STC				;otherwise x to become (1+x)/2
	JMP	SHORT SINGLEDONE	;single precision complete

NOTNEARONE1:
	PUSH	DX			;save third mantissa word
	MOV	AX,0AFB1H		;AX=.AFB1h
	MUL	BX			;DX=.AFB1h*x
	MOV	BP,057D8H		;BP=.57D8h
	ADD	BP,DX			;BP=.AFB1h*x+.57D8h
	JNC	NORMEST 		;if y is more than one
	MOV	BP,0FFFFH		;   replace y with .FFFFh

NORMEST:
	MOV	DX,BX
	XOR	AX,AX			;load divide regs with xa:0h
	DIV	BP			;qa*ya+r1a=xa:0h
	ADD	BP,AX			;ya=ya+qa
	RCR	BP,1			;ya=ya/2
	MOV	DX,BX
	MOV	AX,CX			;load divide regs with xa:xb
	DIV	BP			;qa*ya+r1a=xa:xb
	STC				;add one to qa for better rounding
	ADC	BP,AX			;ya=ya+qa
	RCR	BP,1			;ya=ya/2
	MOV	DX,BX
	MOV	AX,CX			;load divide regs with xa:xb
	DIV	BP			;qa*ya+r1a=xa:xb
	MOV	SI,AX			;save qa
	POP	AX			;load divide regs with r1a:xc
	DIV	BP			;qb*ya+r2a=r1a:xc
	MOV	BX,BP
	MOV	CX,AX			;move qa:qb
	;no need to round with floating point operations following
;	ADD	CX,1			;add one to qa:qb for better rounding
;	ADC	BX,SI			;ya:yb=ya:0h+qa:qb
	ADD	BX,SI			;ya:yb=ya:0h+qa:qb

SINGLEDONE:

	;We now have BX:CX = (ya:yb)*2=ya:0h+qa:qb

	XOR	AX,AX

	SHL	CX,1
	RCL	BX,1
	RCL	AX,1

	SHL	CX,1
	RCL	BX,1
	RCL	AX,1

	SHL	CX,1
	RCL	BX,1
	RCL	AX,1

	SHL	CX,1
	RCL	BX,1
	RCL	AX,1			;back to IEEE format

	OR	[DI+6],AL
	MOV	[DI+4],BX
	MOV	[DI+2],CX
	MOV	WORD PTR [DI],0 	;save ya:yb:0h:0h
	cCall	DDIV,<di>		;accumulator (x)=x/y
	cCall	DADD,<di>		;accumulator (x)=y+x/y
	SUB	WORD PTR [pdAcc+of_exp],16   ;accumulator (x)=(y+x/y)/2
sqrx:
	pop	bp
cEnd

; %%Function:SQR %%Owner:bryanl
cProc	SQR,<PUBLIC,FAR>,<si,di>
cBegin
	cCall	SQRT
cEnd


;*** special common subexpression subroutine for transcendentals
ResetSbTrans:
	mov	ax,sbDds
	cCall	SetSbCur,<ax>
	mov	si,dataOffset pdAcc
	ret

;***	reduce - perform high precision reduction
;
; Function:
;	compute  ac = (x - xn*c1) - xn*c2
;
; Inputs:
;	ARG  = x
;	AC   = xn
;	BX   = c1
;	CX   = c2

REDUCE:
	push	bx
	push	cx
	mov	si,dataOffset TempTran1
	mov	bx,si
	call	dst			; xnT = xn   (TempTran1)
	cCall	DMUL ;,<cx>		  ; ac = xn*c2
	mov	bx,dataOffset TempTran2
	call	dst
	mov	bx,si
	call	dld			; ac = xn
	cCall	DMUL ;,<bx>		  ; ac =  xn*c1
	cCall	DNEG			; ac = -xn*c1
	mov	ax,dataOffset ArgTran
	cCall	DADD,<ax>		; ac = arg-xn*c1
	mov	ax,dataOffset TempTran2
	cCall	DSUB,<ax>		; ac = arg-xn*c1-xn*c2
	ret


; EVEN:
	mov	ax,[si+of_exp]			; get exponent and sign
	mov	ch,al				; save hign mantissa byte
	and	ax,expmask			; test for 0
	jz	evenxit 			;   yes - return even

	sub	ax,expbias-((7-expshft) shl expshft)
	cmp	ax,(manbits-1+7-expshft) shl expshft
	ja	evenxit 			; too big - assume even
	mov	cl,expshft
	shr	ax,cl
	mov	cl,al
	and	cl,7				; get bit within byte
	inc	cx				; bump shift count by 1
	shr	al,1
	shr	al,1
	shr	al,1
	cbw
	mov	bx,ax
	neg	bx				; bx = byte offset
	or	ch,1 shl expshft		; set implied mantissa bit
	xchg	[si+of_exp],ch			; swap with memory
	mov	al,[si+bx+of_exp]
	shl	al,cl				; bit in 'C', to tell us even or odd
	mov	[si+of_exp],ch			; restore high mantissa byte
evenxit:
	ret


;	z2p_q
;
; Function:
;	compute  z*z*P(z*z)/Q(z*z)

z2p_q:
	push	cx
	call	p_q
	pop	cx
	jcxz	nodiv
	mov	ax,dataOffset TempTran1
	cCall	DDIV,<ax>		; P(z*z)/Q(z*z)
nodiv:
	mov	di,dataOffset TempTran2
	cCall	DMUL,<di>
	ret

;	z3p_q
;
; Function:
;	compute  z*z*z*P(z*z)/Q(z*z)

z3p_q:
	call	z2p_q
	mov	di,dataOffset TempTran3
	cCall	DMUL,<di>		; ac = z^3*P(z*z)/Q(z*z)
	ret

;	p_q
;
; Function:
;	computes
;		z = ac
;		zz = z*z
;	if Q
;		pzz = p(zz)
;		ac = q(zz)
;	else
;		ac = p(zz)
;
; Inputs:
;	BX = address of P table
;	CX = address of Q table

p_q:
	push	bx
	push	cx
	mov	di,dataOffset TempTran3 ; temp3=z
	call	dstdi
	cCall	DMUL,<di>
	mov	di,dataOffset TempTran2
	call	dstdi			; temp2 = z*z
	pop	bx			; get Q table
	or	bx,bx
	jz	noq
	call	poly
	mov	bx,dataOffset TempTran1
	call	dst			; TempTran1 = Q(z*z)
	mov	bx,dataOffset TempTran2
	call	dld
noq:	pop	bx
;	call	poly			; ac = P(z*z)
;	ret

;***  POLY - Evaluate DP polynomial
;
; Inputs:
;	BX = Address of list of coefficients. First byte in list is number of
;	     coefficients, followed by the constants ordered from highest
;	     power to lowest.
;	AC = argument.
; Function:
;	Evaluate polynomial by Horner's method
; Outputs:
;	Result in AC.
; Registers:
;	ALL destroyed.

POLY:
	push	ss
	pop	es
	mov	si,dataOffset pdAcc
	mov	di,dataOffset ArgTran
	movsw
	movsw
	movsw
	movsw
	mov	si,bx			; Point to coefficient table
	lodsw				; Number of coefficients
	xchg	ax,cx			; Count in CX
	or	cx,cx			; check for implied 1.0 as 1st
	jns	polymove		;   no - normal poly

	xor	ch,ch			; clear flag so we can count down
	push	cx			; save count
	jmp	short polyone		; jump into middle of poly loop

polymove:
	mov	di,dataOffset pdAcc	; Move first coefficient into AC
	movsw
	movsw
	movsw
	movsw
	jcxz	polyxit

polyloop:
	push	cx
	mov	ax,dataOffset ArgTran
	cCall	DMUL,<ax>		; Multiply by argument

polyone:				; alternate entry for implied 1.0
	cCall	DADD,<si>		; Add coefficient
	pop	cx
	add	si,8			; Bump to next coefficient
	loop	polyloop
polyxit:
	ret


;
;  Load num pointed by AX into pdacc
;
dlddi:
	mov	bx,di
dld:
	mov	ax,[bx	]
	mov	[pdAcc	],ax
	mov	ax,[bx+2]
	mov	[pdAcc+2],ax
	mov	ax,[bx+4]
	mov	[pdAcc+4],ax
	mov	ax,[bx+6]
	mov	[pdAcc+6],ax
	ret

;
;  Load pdacc into num pointed to by AX
;
dstdi:
	mov	bx,di
dst:
	mov	ax,[pdAcc  ]
	mov	[bx  ],ax
	mov	ax,[pdAcc+2]
	mov	[bx+2],ax
	mov	ax,[pdAcc+4]
	mov	[bx+4],ax
	mov	ax,[pdAcc+6]
	mov	[bx+6],ax
	ret



rgnumKonst	DQ	03ff0000000000000R	; 1
		DQ	04000000000000000R	; 2
		DQ	0bff0000000000000R	; -1
		DQ	04024000000000000R	; 10
		DQ	04059000000000000R	; 100
		DQ	040026bb1bbb55516R	; Ln(10)
		DQ	03fe0000000000000R	; 1/2
		DQ	0cca8e45e1df3b078R	; large negative for sort (-2*10**60)

; %%Function:DLdC %%Owner:bryanl
cProc	DLdC,<PUBLIC,FAR>,<si,di,ds>
	ParmW ic
cBegin
	mov	si, ic
	mov	cl, 3
	shl	si, cl
	add	si, offset rgnumKonst
	mov	di, dataOffset pdAcc
	push	ds
	pop	es
	push	cs
	pop	ds
	movsw
	movsw
	movsw
	movsw
cEnd

ifdef NOTUSED
; The next two routines are no longer required by the mathpack.
; %%Function:DldHp %%Owner:NOTUSED
cProc	DldHp,<PUBLIC,FAR>
;	ParmD	hp
cBegin	nogen
	mov	bx,sp
	push	si
	mov	si,[bx+4]		; get offset of huge pointer
	mov	bx,[bx+6]
	call	GetPSfromSB
	cld
	lods	word ptr es:[si]
	mov	word ptr [pdAcc   ],ax
	lods	word ptr es:[si]
	mov	word ptr [pdAcc +2],ax
	lods	word ptr es:[si]
	mov	word ptr [pdAcc +4],ax
	lods	word ptr es:[si]
	mov	word ptr [pdAcc +6],ax
	pop	si
	ret	4
cEnd	nogen

; %%Function:DstHp %%Owner:NOTUSED
cProc	DstHp,<PUBLIC,FAR>
;	ParmD	hp
cBegin	nogen
	mov	bx,sp
	push	si
	push	di
	mov	di,[bx+4]		; get offset of huge pointer
	mov	bx,[bx+6]
	call	GetPSfromSB
	mov	si,dataOffset pdAcc
	cld
	movsw
	movsw
	movsw
	movsw
	pop	di
	pop	si
	ret	4
cEnd	nogen
endif ;NOTUSED

GetPSfromSB:
	shl	bx,1
	mov	ax,[mpsbps+bx]
	mov	es,ax
	shr	ax,1
	jc	mrs
MyReloadSB:
	push	dx
	push	bx
	push	cx
	cCall	ReloadSb,<>
	pop	cx
	pop	bx
	pop	dx
mrs:	ret

sEnd	trans
	end
