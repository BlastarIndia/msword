; movecmd.asm -- move the command table

?PLM = 1
?WIN = 1

include w2.inc
include cmacros.inc

createSeg initwin_PCODE, INIT, BYTE, PUBLIC, CODE

sBegin  INIT
        assumes CS, INIT

externB	vsytInit
externB vkmpInit
externB vmudInit
externB vsttbMenuBase
externB vsttbMenuRgbst

; %%Function:MoveCmds %%Owner:bradch
cProc	MoveCmds, <PUBLIC, FAR>, <di, si, ds>
    parmD lpsyt
    parmD lpkmp
    parmD lpmud
    parmD lpsttb
    parmD lprgbst
cBegin	MoveCmds

	push	cs
	pop	ds

; do command table

	les	di,lpsyt
	mov	si, initOffset vsytInit
	lodsw				; ax = *pSource++ (cw)
	mov	cx,ax
	rep	movsw

; now do keymap

	les	di, lpkmp
	mov	si, initOffset vkmpInit
	lodsw
	mov	cx,ax
	rep	movsw

; menu delta

	les	di,lpmud
	mov	si,initOffset vmudInit
	lodsw
	mov	cx,ax
	rep	movsw

; menu string table base

	les	di,lpsttb
	mov	si,initOffset vsttbMenuBase
	lodsw
	mov	cx,ax
	rep	movsw

; menu string table rgbst & strings

	les	di,lprgbst
	mov	si,initOffset vsttbMenuRgbst
	lodsw
	mov	cx,ax
	rep	movsw

cEnd	MoveCmds

sEnd	INIT

	end
