;NOT USED!!

; CMDLOOK.ASM
;    Multiple entry points for DoLooks function

	include w2.inc
        include noxport.inc
        include consts.inc

createSeg       cmdlook_PCODE,cmdlook,byte,public,CODE

ifdef DEBUG
midCmdLookn      equ 8
endif ; DEBUG


; EXPORTED LABELS

PUBLIC CmdBold
PUBLIC CmdItalic
PUBLIC CmdSmallCaps
PUBLIC CmdHideText
PUBLIC CmdULine
PUBLIC CmdDULine
PUBLIC CmdWULine
PUBLIC CmdSuperscript
PUBLIC CmdSubscript
PUBLIC CmdParaLeft
PUBLIC CmdParaRight
PUBLIC CmdParaCenter
PUBLIC CmdParaBoth
PUBLIC CmdSpace1
PUBLIC CmdSpace15
PUBLIC CmdSpace2
PUBLIC CmdParaClose
PUBLIC CmdParaOpen

; EXTERNAL FUNCTIONS

externFP	<DoLooks>

; CODE SEGMENT cmdlook_PCODE

sBegin	cmdlook
    	assumes cs,cmdlook
	assumes ds,dgroup
	assume es:nothing
	assume ss:nothing



CmdBold:
    	mov ax,ilcdBold
	jmp CmdLookCommon

CmdItalic:
    	mov ax,ilcdItalic
	jmp CmdLookCommon

CmdSmallCaps:
    	mov ax,ilcdSmallCaps
	jmp CmdLookCommon

CmdHideText:
    	mov ax,ilcdVanish
	jmp CmdLookCommon

CmdULine:
    	mov ax,ilcdKulSingle
	jmp CmdLookCommon

CmdDULine:
    	mov ax,ilcdKulDouble
	jmp CmdLookCommon

CmdWULine:
    	mov ax,ilcdKulWord
	jmp CmdLookCommon

CmdSuperscript:
    	mov ax,ilcdSuperscript
	jmp CmdLookCommon

CmdSubscript:
    	mov ax,ilcdSubscript
	jmp CmdLookCommon

CmdParaLeft:
    	mov ax,ilcdParaLeft
	jmp CmdLookCommon

CmdParaRight:
    	mov ax,ilcdParaRight
	jmp CmdLookCommon

CmdParaCenter:
    	mov ax,ilcdParaCenter
	jmp CmdLookCommon

CmdParaBoth:
    	mov ax,ilcdParaBoth
	jmp CmdLookCommon

CmdSpace1:
    	mov ax,ilcdSpace1
	jmp CmdLookCommon

CmdSpace15:
    	mov ax,ilcdSpace15
	jmp CmdLookCommon

CmdSpace2:
    	mov ax,ilcdSpace2
	jmp CmdLookCommon

CmdParaClose:
    	mov ax,ilcdParaClose
	jmp CmdLookCommon

CmdParaOpen:
    	mov ax,ilcdParaOpen


cProc	CmdLookCommon,<FAR>,<>
    	
	; ax = ilcd

cBegin
    	mov bx,0
	mov cx,0
	cCall DoLooks,<ax,bx,cx>
	mov ax,cmdOK
cEnd

sEnd	cmdlook_PCODE
    	end
