	TITLE	WINSTUB - Assembly stub program for Winword

;
;  This guy just boots excel by inserting the word "winword" in front of the
;  command line and starting up windows
;
StackSize	equ	1024


.xlist
include cmacros.inc
.list

sBegin  CODE
assumes CS,CODE
assumes DS,CODE
assumes SS,CODE
;=======================================================================

winx:
	jmp	Entry

szWin2	db	"WIN200.BIN",0
szWin	db	"WIN.COM",0
WinPath db	82 dup (0)

msgBoot 	db	"This program requires Microsoft Windows.",13,10,"$"
msgNoMem	db	"Insufficient memory to run Windows Word.",13,10,"$"
msg386		db	"Type 'WIN386 WINWORD' to run Windows Word.",13,10,"$"

szPATH	  db	"PATH=",0
szApp	  db	"WINWORD"		; default name for DOS 2.0
cchApp	  equ	$-szApp

GrabSeg 	dw	?		; 00 segment of screen grabber code
GrabSize	dw	?		; 02 number of paragraphs in grabber
Xsize		dw	?		; 04 size of screen buffer
pifMsFlags	db	?		; 06 Microsoft Pif bits
AppName db	64 dup (?)	; not really appname, used by ems stuff

	include winemsds.asm

TopPDB	dw	?		; used by ems stuff

Entry:
	mov	ax,es
	mov	ss,ax
	mov	sp,codeOffset LastByte+StackSize+256 ; this should be big enough
	mov	cs:[TopPDB],ds
	mov	word ptr cs:[WinX],codeOffset GrabSeg+256

;
;  Make room in the command line for the word "winword"
;
	push	cs
	pop	ds
	mov	si,codeOffset szApp
	mov	cx,cchApp
	call	FindAppName

	push	ds
	push	si
	push	cx
	mov	ax,ss
	mov	es,ax
	mov	ds,ax
	mov	di,0FFH-2
	mov	si,0FFH-4
	sub	si,cx
	std
	neg	cx
	add	cx,0FFH-4-80H
	mov	word ptr ds:[di+1],"WY"
	rep	movsb
	cld
	pop	cx
	pop	si
	pop	ds

	mov	dx,cx
	mov	di,81H
	mov	al," "
	stosb
	rep	movsb
	stosb

	push	cs
	pop	ds
	mov	di,80H
	add	dl,es:[di]		; get length of command line
	add	dl,2
	cmp	dl,128-3		; is that too long?
	jbe	ee1			; no, continue
	mov	dl,128-3		; force it to be shorter
	mov	byte ptr es:[0FFH-2],13 ; and put this here for good measure.
ee1:
	mov	es:[di],dl
	push	cs
	pop	ds

	mov	dx,codeOffset szWin
	regptr	pPath,ds,dx
	cCall	ExecPathname,<pPath>
	mov	dx,codeOffset szWin2
	regptr	pPath,ds,dx
	cCall	ExecPathname,<pPath>
	push	cs
	pop	ds
	mov	ah,9
	int	21h
	mov	ax,4c02H
	int	21H
;----------------------------------------------------------------------

cProc	ExecPathname,<PUBLIC,NEAR>,<ds>
	ParmD	NameBuffer
	LocalW	pPure
cBegin
	call	GetWindowsPath		; initialize WinPath
	lds	dx,NameBuffer
	call	ExecFile		; is it in current directory?

epn1:	call	FindPath
	jnz	epn8

epn4:	push	cs
	pop	es
	mov	di,codeOffset WinPath
	cmp	byte ptr [si+1],":"	; was drive specified?
	jz	epn5
	mov	ah,19h
	int	21h
	add	al,'A'
	mov	ah,":"
	stosw
epn5:	lodsb
	stosb
	cmp	al,";"
	jz	epn6
	or	al,al
	jnz	epn5
	dec	si

epn6:	mov	al,'\'
	cmp	es:[di-2],al
	jnz	epn6a
	dec	di
epn6a:	mov	es:[di-1],al
	push	ds
	push	si

	lds	si,NameBuffer
epn7:	lodsb
	stosb
	or	al,al
	jnz	epn7

	push	cs
	pop	ds
	mov	dx,codeOffset WinPath
	call	ExecFile
	pop	si
	pop	ds
	mov	dx,codeOffset msg386
	jnc	epnx
epn7a:	cmp	byte ptr [si],0
	jnz	epn4
epn8:
	mov	dx,codeOffset msgBoot
epnx:
cEnd



	public	ExecFile
;
;   Try to run the com program who name is at DS:DX
;
ExecFile:
	mov	ax,4300H	; Novell uses non standard open code...
	int	21H		; ...so we do this call to see if it's...
	jc	efx		; ...around
	mov	ax,3d00H	; try to open
	int	21h
	jnc	BootComFile
efx:	ret

BootComFile:
	mov	si,ax			; save file handle
	mov	bx,ax
	mov	di,dx
	push	ds
	pop	es
	mov	cx,-1
	xor	ax,ax
	repne	scasb
	cmp	word ptr [di-5],"C."
	jnz	BootExeFile

;
;  We really are going to start up WIN.COM.  If we can manage to fit
;  the current directory into the enviroment argv[0], let's do so.
;
	push	ds
	push	bx
	call	FindAppName
	pop	bx
	pop	ax
	jc	ef1		; not there, don't worry.
	add	cx,si
	add	cx,15
	and	cx,0FFF0H
	sub	cx,si		; cx has the amount of room to...
	dec	cx		; ...play with	(dec cx for null termination)
	mov	di,si
	mov	si,dx
	push	ds
	pop	es
	mov	ds,ax
;
;	Let's see if we can trim a little room by removing the drive letter...
;
	cmp	byte ptr ds:[si+1],":"
	jnz	ef0
	mov	ah,19H
	int	21H
	add	al,"A"
	cmp	al,ds:[si]
	jnz	ef0
	add	si,2
ef0:
	lodsb
	stosb
	or	al,al
	jz	ef1		; did we copy the whole thing?
	loop	ef0

	; if we're here, Windows path is longer than space in argv[0]

	mov	ah,3EH		; close file
	int	21H
	stc			; error loading win.com
	ret
ef1:
	mov	cx,ss
	mov	ds,cx
	mov	es,cx
	mov	si,5cH
	mov	word ptr [si+ 0],0FEBCH   ; "MOV SP,FFFEH"
	mov	word ptr [si+ 2],0CDFFH   ; "INT 21H"
	mov	word ptr [si+ 4],0B421H   ; "MOV AH,3eH"
	mov	word ptr [si+ 6],0CD3EH   ; "INT 21H"
	mov	word ptr [si+ 8],0E921H   ; "JMP 100"
	mov	word ptr [si+10],00098H
	mov	word ptr [si+0feh-5cH],0000H   ; cancel out 'WY'
	mov	dx,100H
	mov	cx,0FFFFH		; read this many bytes
	mov	ah,3FH
	push	ss
	push	si
xxx	proc	far
	ret
xxx	endp


;
;  Trying to boot WIN200.BIN.  If WIN86.COM is present, give up.
;
BootExeFile:
	mov	word ptr [di-8],"68"
	mov	word ptr [di-6],"C."
	mov	word ptr [di-4],"MO"
	mov	byte ptr [di-2],0
	mov	ax,3d00H
	int	21H
	mov	cx,word ptr [szWin2+3]
	mov	word ptr [di-8],cx
	mov	cx,word ptr [szWin2+5]
	mov	word ptr [di-6],cx
	mov	cx,word ptr [szWin2+7]
	mov	word ptr [di-4],cx
	mov	cx,word ptr [szWin2+9]
	mov	word ptr [di-2],cx
	mov	cx,ss
	mov	ds,cx
	mov	es,cx
	mov	bx,si
	jc	bef1
	ret
bef1:
	push	bx
	or	cs:EMSFlags[1],EMSF1_ADVANCED_OFF    ; no advanced EMS
	call	Test_EMS
	call	CopyWindowsPath
	call	InitTandy1000
	pop	bx
	push	ss
	pop	ds
;
;  Load in exe header just past the stack
;
	mov	si,codeOffset LastByte+StackSize+256
	mov	dx,si
	mov	cx,512
	mov	ah,3FH
	int	21H			; read in exe header
;
;  To various checks to make us feel good we have what we want
;
	cmp	ax,512
	jnz	BadFile
	cmp	[si+00H],5A4DH
	jnz	BadFile
	mov	ax,[si+06H]
	or	ax,[si+0AH]
	or	ax,[si+0CH]
	jz	GoodFile
BadFile:
	stc
	ret

GoodFile:
	mov	ax,[si+04H]
	dec	ax
	mov	cl,5
	shl	ax,cl			; file is this many paragraphs
	mov	di,word ptr ds:[2]
	sub	di,ax
	mov	bp,di
LoadFileLoop:
	mov	ds,di
	add	di,60*(1024/16)
	mov	cx,60*1024
	xor	dx,dx
	mov	ah,3Fh
	int	21H
	jc	BadFile
	cmp	ax,cx
	jz	LoadFileLoop
;
;  Its all loaded in, now just start it up!
;
	mov	ah,3eH		; be nice and close the file
	int	21H
	push	ss
	pop	ds
	mov	dx,bp		; starting seg
	add	dx,[si+16H]	; relocate cs
	mov	cx,word ptr [si+14H]
	mov	ax,bp
	add	ax,[si+0EH]
	mov	ss,ax
	mov	sp,[si+10H]
	push	dx
	push	cx
yyy	proc	far
	ret
yyy	endp

;--------------------------------
;
;  Find Enviroment String, DI points to string, CX contains length
;
;  On return ZF=1 if string found, DS:SI points at string
;  Otherwise ZF=0
;
FindPath:
	mov	di,codeOffset szPath
	push	ds
	pop	es
	mov	cx,5			; length of string including =
	mov	ds,word ptr ss:[2CH]	; get segment of enviroment
	xor	si,si
fpa1:	push	cx
	push	di
	repz	cmpsb
	pop	di
	pop	cx
	jz	fpa3
fpa2:	lodsb
	or	al,al
	jnz	fpa2
	cmp	byte ptr [si],0
	jnz	fpa1
	or	cx,cx			; ZF=0
fpa3:	ret


;--------------------------------
;
;  Find Application name.
;
;  On entry DS:SI points to the default name to use, CX has length
;  On return DS:SI points to the app name, CX has length
;
FindAppName:
	mov	ah,30h
	int	21h
	cmp	al,3
	jae	fan1
	ret
fan1:
	mov	es,word ptr ss:[2CH]	; get segment of enviroment
	mov	cx,0FFFFH
	xor	di,di
	xor	al,al
fan2:
	repne	scasb
	cmp	es:[di],al
	jnz	fan2
	add	di,3
	push	es
	pop	ds
	mov	si,di
;
;   At this point ds:si should be pointing at null terminated app name
;
GetLength:
	mov	cx,-1
	repne	scasb
	inc	cx
	not	cx		; cx has the length
	ret

	public	GetWindowsPath
GetWindowsPath:
	cld
	mov	ax,cs
	mov	ds,ax
	mov	es,ax
	mov	di,codeOffset WinPath
	mov	ah,19h
	int	21h
	add	al,'A'
	stosb
	mov	ax,'\:'
	stosw
	mov	si,di
	xor	dx,dx
	mov	ah,47h
	int	21h
	mov	si,codeOffset WinPath
gwn1:
	lodsb
	or	al,al
	jnz	gwn1
	mov	al,'\'
	cmp	[si-2],al
	jnz	gwn1a
	dec	si
gwn1a:	mov	[si-1],al
	mov	di,si
	lds	si,NameBuffer
gwn2:	lodsb
	stosb
	or	al,al
	jnz	gwn2
	ret

InitTandy1000:
	mov	ax,0F000H		; point to ROM
	mov	ds,ax
	cmp	byte ptr ds:[0FFFEH],0FFH
	jnz	NoTandy1000
	cmp	byte ptr ds:[0C000H],021H
	jnz	NoTandy1000
	int	12H
	cmp	ax,640
	jnc	NoTandy1000
	push	ss
	pop	es
	mov	ah,4aH
	mov	bx,0FFFFH
	int	21H
	sub	bx,(16*1024)/16+1
	sub	ss:[2],(16*1024)/16+1
	mov	ah,4aH
	int	21H
	mov	bx,(16*1024)/16
	mov	ah,48H
	int	21H
NoTandy1000:
	ret

CopyWindowsPath:
	push	cs
	pop	es
	mov	si,codeOffset WinPath	; point to fully qualified name
	mov	di,si			; compute the length...
	mov	cx,-1
	xor	ax,ax
	repne	scasb
	not	cx			; ...including null
	push	ss
	pop	ds
	mov	di,80h
	xor	ax,ax
	mov	al,ds:[di]		; Get length of command line
	add	al,cl			; followed by length of name
	add	ax,3			; plus 'WX' flag, count byte
	sub	ax,7Eh
	jle	ew1
	sub	ds:[di],al		; Trim command line
ew1:
	push	ss
	pop	es
	xor	ax,ax
	mov	al,ds:[di]		; Get length of command line
	inc	di			; Skip byte count
	add	di,ax			; Skip command line
	mov	al,0Dh			; Terminate with CR
	stosb
	mov	ax,cx			; followed by file name only
	stosb				; save the path
	push	cs
	pop	ds
	rep	movsb			; move the name into the command line
	ret

	include winemscs.asm

	even
LastByte:

sEnd

createSeg	STACK,stack,word,stack,STACK

sBegin	stack
	dw	128 dup (?)
sEnd	stack

end
