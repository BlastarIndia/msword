;====================================================================
;
; wprocn.asm -- Native portion of window proc & related
;
; The WndProc routines in this module are used to provide a fast evaluation
; of incoming messages, sending those that are not used by
; the app on to DefWindowProc without any intervening pcode.
;
; A second benefit is that window message traffic will produce less swapping
;
        .xlist
        memS    = 1
        ?WIN    = 1
        ?PLM    = 1
        ?NODATA = 1
        ?TF     = 1
        include w2.inc
	include cmacros.inc
	include windows.inc
	include consts.inc
	include structs.inc
        .list
createSeg       wproc_PCODE,wproc,byte,public,CODE
createSeg       _DATA,data,word,public,DATA,DGROUP
defGrp          DGROUP,DATA

externFP        <AppWndProc>
externFP        <MwdWndProc>
externFP        <DeskTopWndProc>
externFP        <WwPaneWndProc>
ifdef HYBRID
externFP        <DefWinProcPR>
else ;!HYBRID
externFP        <DefWindowProc>
endif ;!HYBRID
externFP        <PromptWndProc>
ifdef HYBRID
externFP	<CallWindowProcPR>
else ;!HYBRID
externFP	<CallWindowProc>
endif ;!HYBRID
externFP	<FedtWndProc>
externFP	<OurSetCursor>
externFP        <RulerMarkWndProc>
externFP        <IconBarWndProc>
externFP        <StatlineWndProc>
externFP        <SplitBarWndProc>
externFP        <PgPrvwWndProc>
externFP        <StartWndProc>
externFP        <DdeChnlWndProc>
externFP        <RSBWndProc>
externFP        <StaticEditWndProc>

ifdef DEBUG
externFP	<CommSzSz>
endif ;DEBUG

sBegin  data

externW vhcIBeam
externW vhcArrow
externW vfHelp

ifdef DEBUG
externW vdbs
endif ;DEBUG

sEnd    data


sBegin  wproc
        assumes cs,wproc
        assumes ds,dgroup
        assumes ss,dgroup


;   AppWndProc
rgwmApp:
dw  WM_CREATE
dw  WM_INITMENUPOPUP
dw  WM_MENUCHAR
dw  WM_ENTERIDLE
dw  WM_ACTIVATE
dw  WM_ACTIVATEAPP
dw  WM_MOUSEACTIVATE
dw  WM_SETCURSOR
dw  WM_TIMER
dw  WM_CLOSE
dw  WM_QUERYENDSESSION
dw  WM_ENDSESSION
dw  WM_DESTROY
dw  WM_SIZE
dw  WM_MOVE
dw  WM_COMMAND
dw  WM_SYSCOMMAND
dw  WM_SYSCOLORCHANGE
dw  WM_WININICHANGE
dw  WM_DEVMODECHANGE
dw  WM_DESTROYCLIPBOARD
dw  WM_RENDERFORMAT
dw  WM_PAINTCLIPBOARD
dw  WM_VSCROLLCLIPBOARD
dw  WM_HSCROLLCLIPBOARD
dw  WM_SIZECLIPBOARD
dw  WM_ASKCBFORMATNAME
dw  WM_FONTCHANGE
dw  WM_DDE_INITIATE
dw  WM_MENUSELECT
dw  WM_SYSKEYDOWN
dw  WM_CBTINIT
dw  WM_CBTTERM
dw  WM_CBTSEMEV
dw  WM_SYSTEMERROR
dw  WM_NCLBUTTONDBLCLK
cwmApp = ($ - rgwmApp) SHR 1

;   MwdWndProc
rgwmMwd:
dw  WM_CREATE
dw  WM_MOVE
dw  WM_SIZE
dw  WM_CLOSE
dw  WM_GETMINMAXINFO
dw  WM_VSCROLL
dw  WM_HSCROLL
dw  WM_MOUSEACTIVATE
dw  WM_CHILDACTIVATE
dw  WM_MENUCHAR
dw  WM_SYSCOMMAND
dw  WM_INITMENUPOPUP
dw  WM_MENUSELECT
dw  WM_ENTERIDLE
cwmMwd = ($ - rgwmMwd) SHR 1

;   WwPaneWndProc
rgwmWwPane:
dw  WM_CREATE
dw  WM_SIZE
dw  WM_SETFOCUS
dw  WM_KILLFOCUS
dw  WM_PAINT
dw  WM_MOUSEACTIVATE
dw  WM_SETCURSOR
dw  WM_MOUSEMOVE
dw  WM_LBUTTONDOWN
dw  WM_LBUTTONDBLCLK
dw  WM_RBUTTONDOWN
dw  WM_SYSCOMMAND
dw  WM_MENUCHAR
cwmWwPane = ($ - rgwmWwPane) SHR 1

;   SplitBarWndProc
rgwmSplitBar:
;   WM_MOUSEMOVE  ; special handling
dw  WM_PAINT
cwmSplitBar = ($ - rgwmSplitBar) SHR 1

;   StatLineWndProc
rgwmStatLine:
;   WM_MOUSEMOVE  ; special handling
dw  WM_CREATE
dw  WM_LBUTTONDBLCLK
dw  WM_PAINT
dw  WM_LBUTTONDOWN  ; for Help
cwmStatLine = ($ - rgwmStatLine) SHR 1

;   PgPrvwWndProc
rgwmPgPrvw:
dw  WM_MOUSEMOVE
dw  WM_CREATE
dw  WM_SIZE
dw  WM_ERASEBKGND
dw  WM_PAINT
dw  WM_VSCROLL
dw  WM_LBUTTONDBLCLK
dw  WM_LBUTTONDOWN
dw  WM_CLOSE
dw  WM_SYSCOMMAND
cwmPgPrvw = ($ - rgwmPgPrvw) SHR 1

;   FedtWndProc
rgwmFedt:
dw  WM_NCCREATE
dw  WM_DESTROY
dw  WM_SIZE
dw  WM_SETFOCUS
dw  WM_KILLFOCUS
dw  WM_KEYDOWN
dw  WM_CHAR
dw  WM_KEYUP
dw  WM_LBUTTONDBLCLK
dw  WM_LBUTTONDOWN
dw  WM_SETREDRAW
dw  WM_ERASEBKGND
dw  WM_PAINT
dw  WM_GETTEXTLENGTH
dw  WM_GETTEXT
dw  WM_SETTEXT
dw  EM_SETHANDLE
dw  WM_GETDLGCODE
dw  WM_COPY
dw  WM_CUT
dw  WM_CLEAR
dw  WM_PASTE
dw  EM_SETSEL
dw  EM_GETSEL
dw  EM_GETLINECOUNT
dw  EM_REPLACESEL
dw  EM_GETHANDLE
cwmFedt = ($ - rgwmFedt) SHR 1

;   DeskTopWndProc
rgwmDeskTop:
;   WM_MOUSEMOVE  ; special handling
dw  WM_SIZE
cwmDeskTop = ($ - rgwmDeskTop) SHR 1

;   IconBarWndProc
rgwmIconBar:
;   WM_MOUSEMOVE  ; special handling
dw  WM_NCDESTROY
dw  WM_LBUTTONDOWN
dw  WM_LBUTTONDBLCLK
dw  WM_SYSCOMMAND
dw  WM_SYSKEYDOWN
dw  WM_KEYDOWN
dw  WM_PAINT
dw  WM_SETVISIBLE
dw  WM_MOVE
dw  WM_SIZE
dw  WM_ENABLE
cwmIconBar = ($ - rgwmIconBar) SHR 1

;   RulerMarkWndProc
rgwmRulerMark:
;   WM_MOUSEMOVE  ; special handling
dw  WM_LBUTTONDBLCLK
dw  WM_CREATE
dw  WM_PAINT
dw  WM_LBUTTONDOWN
cwmRulerMark = ($ - rgwmRulerMark) SHR 1


;   PromptWndProc
rgwmPrompt:
;   WM_MOUSEMOVE  ; special handling
dw  WM_SYSCOMMAND
dw  WM_CREATE
dw  WM_PAINT
dw  WM_DESTROY
dw  WM_TIMER
dw  WM_KEYDOWN
dw  WM_CHAR
dw  WM_SETFOCUS
dw  WM_SIZE
dw  WM_KILLFOCUS
cwmPrompt = ($ - rgwmPrompt) SHR 1

;   DdeChnlWndProc
rgwmDdeChnl:
dw  WM_DDE_TERMINATE
dw  WM_DDE_ADVISE
dw  WM_DDE_UNADVISE
dw  WM_DDE_ACK
dw  WM_DDE_DATA
dw  WM_DDE_REQUEST
dw  WM_DDE_POKE
dw  WM_DDE_EXECUTE
cwmDdeChnl = ($ - rgwmDdeChnl) SHR 1

;   RSBWndProc
rgwmRSB:
dw  WM_NCCREATE
dw  WM_CREATE
dw  WM_ERASEBKGND
dw  WM_PAINT
dw  WM_LBUTTONDOWN
dw  WM_LBUTTONDBLCLK
dw  WM_SETCURSOR
cwmRSB = ($ - rgwmRSB) SHR 1

;   StartWndProc
rgwmStart:
;   WM_MOUSEMOVE  ; special handling
dw  WM_NCPAINT
dw  WM_DESTROY
cwmStart = ($ - rgwmStart) SHR 1

;   StaticEditWndProc
rgwmStaticEdit:
dw  WM_CREATE
dw  WM_DESTROY
dw  WM_PAINT
dw  WM_SETFOCUS
dw  WM_KILLFOCUS
dw  WM_COMMAND
dw  WM_GETDLGCODE
dw  WM_SETCURSOR
dw  WM_KEYDOWN
dw  WM_MOUSEMOVE  ; handle this ourselves
dw  WM_LBUTTONDOWN
dw  WM_LBUTTONUP
dw  WM_ENABLE
cwmStaticEdit = ($ - rgwmStaticEdit) SHR 1

;   WARNING the order of the WndProc's is assumed below

; %%Function:NatAppWndProc %%Owner:BRADV
PUBLIC NatAppWndProc
NatAppWndProc LABEL FAR
	call	LN_NatWndProcEngine
dd  AppWndProc
dw  offset rgwmApp, cwmApp

; %%Function:NatMwdWndProc %%Owner:BRADV
PUBLIC NatMwdWndProc
NatMwdWndProc LABEL FAR
	call	LN_NatWndProcEngine
dd  MwdWndProc
dw  offset rgwmMwd, cwmMwd

; %%Function:NatWwPaneWndProc %%Owner:BRADV
PUBLIC NatWwPaneWndProc
NatWwPaneWndProc LABEL FAR
	call	LN_NatWndProcEngine
dd  WwPaneWndProc
dw  offset rgwmWwPane, cwmWwPane

; %%Function:NatDdeChnlWndProc %%Owner:BRADV
PUBLIC NatDdeChnlWndProc
NatDdeChnlWndProc LABEL FAR
	call	LN_NatWndProcEngine
dd  DdeChnlWndProc
dw  offset rgwmDdeChnl, cwmDdeChnl

; %%Function:NatRSBWndProc %%Owner:BRADV
PUBLIC NatRSBWndProc
NatRSBWndProc LABEL FAR
	call	LN_NatWndProcEngine
dd  RSBWndProc
dw  offset rgwmRSB, cwmRSB

;  add WndProcs with no special processing above here

LMouseMove:
;  below here, special MOUSEMOVE handling

; %%Function:NatFedtWndProc %%Owner:BRADV
PUBLIC NatFedtWndProc
NatFedtWndProc LABEL FAR
	call	LN_NatWndProcEngine
dd  FedtWndProc
dw  offset rgwmFedt, cwmFedt

; %%Function:NatStaticEditWndProc %%Owner:BRADV
PUBLIC NatStaticEditWndProc
NatStaticEditWndProc LABEL FAR
	call	LN_NatWndProcEngine
dd  StaticEditWndProc
dw offset rgwmStaticEdit, cwmStaticEdit

LMouseMoveArrow:
;  below here, mouse move results in arrow, above here, I-Beam

; %%Function:NatStartWndProc %%Owner:BRADV
PUBLIC NatStartWndProc
NatStartWndProc LABEL FAR
	call	LN_NatWndProcEngine
dd  StartWndProc
dw  offset rgwmStart, cwmStart

; %%Function:NatDeskTopWndProc %%Owner:BRADV
PUBLIC NatDeskTopWndProc
NatDeskTopWndProc LABEL FAR
	call	LN_NatWndProcEngine
dd  DeskTopWndProc
dw  offset rgwmDeskTop, cwmDeskTop

; %%Function:NatPgPrvwWndProc %%Owner:BRADV
PUBLIC NatPgPrvwWndProc
NatPgPrvwWndProc LABEL FAR
	call	LN_NatWndProcEngine
dd  PgPrvwWndProc
dw  offset rgwmPgPrvw, cwmPgPrvw

; %%Function:NatSplitBarWndProc %%Owner:BRADV
PUBLIC NatSplitBarWndProc
NatSplitBarWndProc LABEL FAR
	call	LN_NatWndProcEngine
dd  SplitBarWndProc
dw  offset rgwmSplitBar, cwmSplitBar

; %%Function:NatStatLineWndProc %%Owner:BRADV
PUBLIC NatStatLineWndProc
NatStatLineWndProc LABEL FAR
	call	LN_NatWndProcEngine
dd  StatLineWndProc
dw  offset rgwmStatLine, cwmStatLine

; %%Function:NatIconBarWndProc %%Owner:BRADV
PUBLIC NatIconBarWndProc
NatIconBarWndProc LABEL FAR
	call	LN_NatWndProcEngine
dd  IconBarWndProc
dw  offset rgwmIconBar, cwmIconBar

; %%Function:NatRulerMarkWndProc %%Owner:BRADV
PUBLIC NatRulerMarkWndProc
NatRulerMarkWndProc LABEL FAR
	call	LN_NatWndProcEngine
dd  RulerMarkWndProc
dw  offset rgwmRulerMark, cwmRulerMark

; %%Function:NatPromptWndProc %%Owner:BRADV
PUBLIC NatPromptWndProc
NatPromptWndProc LABEL FAR
	call	LN_NatWndProcEngine
dd  PromptWndProc
dw  offset rgwmPrompt, cwmPrompt

;  other wnd proc

pWprocDefault:
ifdef HYBRID
dd  DefWinProcPR
else ;!HYBRID
dd  DefWindowProc
endif ;!HYBRID

pWprocCall:
ifdef HYBRID
dd  CallWindowProcPR
else ;!HYBRID
dd  CallWindowProc
endif ;!HYBRID

;=======================================================================
;
; NatWndProcEngine( hwnd, message, wParam, lParam )
; HWND hwnd;  int message, wParam;  long lParam;
;
; Workhorse routine for the Nat...WndProcs.
;-----------------------------------------------------------------------------
LN_NatWndProcEngine:
	pop	bx

; %%Function:NatWndProcEngine %%Owner:BRADV
cProc	NatWndProcEngine,<PUBLIC,FAR,DATA>,<di>
	parmW	hwnd
	parmW	message
	parmW	wParam
	parmD	lParam
cBegin
	mov	ax,[message]
ifdef DEBUG
	cmp	[vdbs.fDumpMessagesDbs],fFalse
	je	NWPE005
	call	OutputMessage
NWPE005:
endif ;DEBUG
	mov	di,cs:[bx+4]
	mov	cx,cs:[bx+6]
ifdef DEBUG
	;There used to be a "cld" instruction here.  Make sure it was
	;not necessary.
	push	ax
	pushf
	pop	ax
	test	ah,4
	je	NWPE01
	int	3	;can't use normal assert procedure if this happens.
NWPE01:
	pop	ax
endif ;DEBUG
	push	cs
	pop	es
	repnz	scasw	    ;look for message in rgwm
	je	NWPE05	    ;if message found call the proper WndProc
	cmp	bx,offset LMouseMove
	jb	NWPE04      ;call DefWndProc
	cmp	ax,WM_MOUSEMOVE
	jne	NWPE04      ;call DefWndProc

        ; MOUSEMOVE message: call OurSetCursor with vhcArrow or other
        mov     ax,vhcArrow
        cmp     bx,offset LMouseMoveArrow
        jae     NWPE02
        mov     ax,vhcIBeam

NWPE02:
        push    ax
	cCall	OurSetCursor
        ; return (long)fFalse
	xor	ax,ax
	cwd
	jmp	short NWPE06

NWPE04:
	mov	bx,offset pWprocDefault
NWPE05:
	push	[hwnd]
	push	[message]
	push	[wParam]
	push	[SEG_lParam]
	push	[OFF_lParam]
	call	dword ptr cs:[bx]
NWPE06:
cEnd

ifdef DEBUG
rgOffWndProc:
    dw	offset NatAppWndProc+3
    dw	offset NatMwdWndProc+3
    dw	offset NatWwPaneWndProc+3
    dw	offset NatDdeChnlWndProc+3
    dw	offset NatRSBWndProc+3
    dw	offset NatFedtWndProc+3
    dw	offset NatStaticEditWndProc+3
    dw	offset NatStartWndProc+3
    dw	offset NatDeskTopWndProc+3
    dw	offset NatPgPrvwWndProc+3
    dw	offset NatSplitBarWndProc+3
    dw	offset NatStatLineWndProc+3
    dw	offset NatIconBarWndProc+3
    dw	offset NatRulerMarkWndProc+3
    dw	offset NatPromptWndProc+3
cOffWndProc = ($ - rgOffWndProc) SHR 1
szNatAppWndProc:
    db	"NatAppWndProc",0
szNatMwdWndProc:
    db	"NatMwdWndProc",0
szNatWwPaneWndProc:
    db	"NatWwPaneWndProc",0
szNatDdeChnlWndProc:
    db	"NatDdeChnlWndProc",0
szNatRSBWndProc:
    db	"NatRSBWndProc",0
szNatFedtWndProc:
    db	"NatFedtWndProc",0
szNatStaticEditWndProc:
    db	"NatStaticEditWndProc",0
szNatStartWndProc:
    db	"NatStartWndProc",0
szNatDeskTopWndProc:
    db	"NatDeskTopWndProc",0
szNatPgPrvwWndProc:
    db	"NatPgPrvwWndProc",0
szNatSplitBarWndProc:
    db	"NatSplitBarWndProc",0
szNatStatLineWndProc:
    db	"NatStatLineWndProc",0
szNatIconBarWndProc:
    db	"NatIconBarWndProc",0
szNatRulerMarkWndProc:
    db	"NatRulerMarkWndProc",0
szNatPromptWndProc:
    db	"NatPromptWndProc",0
szUnknownWndProc:
    db	"Unknown Wnd Proc",0
rgSzWndProc:
    dw	offset szNatAppWndProc
    dw	offset szNatMwdWndProc
    dw	offset szNatWwPaneWndProc
    dw	offset szNatDdeChnlWndProc
    dw	offset szNatRSBWndProc
    dw	offset szNatFedtWndProc
    dw	offset szNatStaticEditWndProc
    dw	offset szNatStartWndProc
    dw	offset szNatDeskTopWndProc
    dw	offset szNatPgPrvwWndProc
    dw	offset szNatSplitBarWndProc
    dw	offset szNatStatLineWndProc
    dw	offset szNatIconBarWndProc
    dw	offset szNatRulerMarkWndProc
    dw	offset szNatPromptWndProc
cSzWndProc = ($ - rgSzWndProc) SHR 1
    errnz   <cSzWndProc - cOffWndProc>
    dw	offset szUnknownWndProc

rgwmDebug:
    dw	EM_GETHANDLE
    dw	EM_GETLINECOUNT
    dw	EM_GETSEL
    dw	EM_REPLACESEL
    dw	EM_SETHANDLE
    dw	EM_SETSEL
    dw	WM_ACTIVATE
    dw	WM_ACTIVATEAPP
    dw	WM_ASKCBFORMATNAME
    dw	WM_CBTINIT
    dw	WM_CBTSEMEV
    dw	WM_CBTTERM
    dw	WM_CHAR
    dw	WM_CHILDACTIVATE
    dw	WM_CLEAR
    dw	WM_CLOSE
    dw	WM_COMMAND
    dw	WM_COPY
    dw	WM_CREATE
    dw	WM_CUT
    dw	WM_DDE_ACK
    dw	WM_DDE_ADVISE
    dw	WM_DDE_DATA
    dw	WM_DDE_EXECUTE
    dw	WM_DDE_INITIATE
    dw	WM_DDE_POKE
    dw	WM_DDE_REQUEST
    dw	WM_DDE_TERMINATE
    dw	WM_DDE_UNADVISE
    dw	WM_DESTROY
    dw	WM_DESTROYCLIPBOARD
    dw	WM_DEVMODECHANGE
    dw	WM_ENABLE
    dw	WM_ENDSESSION
    dw	WM_ENTERIDLE
    dw	WM_ERASEBKGND
    dw	WM_FONTCHANGE
    dw	WM_GETDLGCODE
    dw	WM_GETMINMAXINFO
    dw	WM_GETTEXT
    dw	WM_GETTEXTLENGTH
    dw	WM_HSCROLL
    dw	WM_HSCROLLCLIPBOARD
    dw	WM_INITMENUPOPUP
    dw	WM_KEYDOWN
    dw	WM_KEYUP
    dw	WM_KILLFOCUS
    dw	WM_LBUTTONDBLCLK
    dw	WM_LBUTTONDOWN
    dw	WM_LBUTTONUP
    dw	WM_MENUCHAR
    dw	WM_MENUSELECT
    dw	WM_MOUSEACTIVATE
    dw	WM_MOUSEMOVE
    dw	WM_MOVE
    dw	WM_NCCREATE
    dw	WM_NCDESTROY
    dw	WM_NCLBUTTONDBLCLK
    dw	WM_NCPAINT
    dw	WM_PAINT
    dw	WM_PAINTCLIPBOARD
    dw	WM_PASTE
    dw	WM_QUERYENDSESSION
    dw	WM_RBUTTONDOWN
    dw	WM_RENDERFORMAT
    dw	WM_SETCURSOR
    dw	WM_SETFOCUS
    dw	WM_SETREDRAW
    dw	WM_SETTEXT
    dw	WM_SETVISIBLE
    dw	WM_SIZE
    dw	WM_SIZECLIPBOARD
    dw	WM_SYSCOLORCHANGE
    dw	WM_SYSCOMMAND
    dw	WM_SYSKEYDOWN
    dw	WM_SYSTEMERROR
    dw	WM_TIMER
    dw	WM_VSCROLL
    dw	WM_VSCROLLCLIPBOARD
    dw	WM_WININICHANGE
cwmDebug = ($ - rgwmDebug) SHR 1
szEM_GETHANDLE:
    db	"EM_GETHANDLE",0
szEM_GETLINECOUNT:
    db	"EM_GETLINECOUNT",0
szEM_GETSEL:
    db	"EM_GETSEL",0
szEM_REPLACESEL:
    db	"EM_REPLACESEL",0
szEM_SETHANDLE:
    db	"EM_SETHANDLE",0
szEM_SETSEL:
    db	"EM_SETSEL",0
szWM_ACTIVATE:
    db	"WM_ACTIVATE",0
szWM_ACTIVATEAPP:
    db	"WM_ACTIVATEAPP",0
szWM_ASKCBFORMATNAME:
    db	"WM_ASKCBFORMATNAME",0
szWM_CBTINIT:
    db	"WM_CBTINIT",0
szWM_CBTSEMEV:
    db	"WM_CBTSEMEV",0
szWM_CBTTERM:
    db	"WM_CBTTERM",0
szWM_CHAR:
    db	"WM_CHAR",0
szWM_CHILDACTIVATE:
    db	"WM_CHILDACTIVATE",0
szWM_CLEAR:
    db	"WM_CLEAR",0
szWM_CLOSE:
    db	"WM_CLOSE",0
szWM_COMMAND:
    db	"WM_COMMAND",0
szWM_COPY:
    db	"WM_COPY",0
szWM_CREATE:
    db	"WM_CREATE",0
szWM_CUT:
    db	"WM_CUT",0
szWM_DDE_ACK:
    db	"WM_DDE_ACK",0
szWM_DDE_ADVISE:
    db	"WM_DDE_ADVISE",0
szWM_DDE_DATA:
    db	"WM_DDE_DATA",0
szWM_DDE_EXECUTE:
    db	"WM_DDE_EXECUTE",0
szWM_DDE_INITIATE:
    db	"WM_DDE_INITIATE",0
szWM_DDE_POKE:
    db	"WM_DDE_POKE",0
szWM_DDE_REQUEST:
    db	"WM_DDE_REQUEST",0
szWM_DDE_TERMINATE:
    db	"WM_DDE_TERMINATE",0
szWM_DDE_UNADVISE:
    db	"WM_DDE_UNADVISE",0
szWM_DESTROY:
    db	"WM_DESTROY",0
szWM_DESTROYCLIPBOARD:
    db	"WM_DESTROYCLIPBOARD",0
szWM_DEVMODECHANGE:
    db	"WM_DEVMODECHANGE",0
szWM_ENABLE:
    db	"WM_ENABLE",0
szWM_ENDSESSION:
    db	"WM_ENDSESSION",0
szWM_ENTERIDLE:
    db	"WM_ENTERIDLE",0
szWM_ERASEBKGND:
    db	"WM_ERASEBKGND",0
szWM_FONTCHANGE:
    db	"WM_FONTCHANGE",0
szWM_GETDLGCODE:
    db	"WM_GETDLGCODE",0
szWM_GETMINMAXINFO:
    db	"WM_GETMINMAXINFO",0
szWM_GETTEXT:
    db	"WM_GETTEXT",0
szWM_GETTEXTLENGTH:
    db	"WM_GETTEXTLENGTH",0
szWM_HSCROLL:
    db	"WM_HSCROLL",0
szWM_HSCROLLCLIPBOARD:
    db	"WM_HSCROLLCLIPBOARD",0
szWM_INITMENUPOPUP:
    db	"WM_INITMENUPOPUP",0
szWM_KEYDOWN:
    db	"WM_KEYDOWN",0
szWM_KEYUP:
    db	"WM_KEYUP",0
szWM_KILLFOCUS:
    db	"WM_KILLFOCUS",0
szWM_LBUTTONDBLCLK:
    db	"WM_LBUTTONDBLCLK",0
szWM_LBUTTONDOWN:
    db	"WM_LBUTTONDOWN",0
szWM_LBUTTONUP:
    db	"WM_LBUTTONUP",0
szWM_MENUCHAR:
    db	"WM_MENUCHAR",0
szWM_MENUSELECT:
    db	"WM_MENUSELECT",0
szWM_MOUSEACTIVATE:
    db	"WM_MOUSEACTIVATE",0
szWM_MOUSEMOVE:
    db	"WM_MOUSEMOVE",0
szWM_MOVE:
    db	"WM_MOVE",0
szWM_NCCREATE:
    db	"WM_NCCREATE",0
szWM_NCDESTROY:
    db	"WM_NCDESTROY",0
szWM_NCLBUTTONDBLCLK:
    db	"WM_NCLBUTTONDBLCLK",0
szWM_NCPAINT:
    db	"WM_NCPAINT",0
szWM_PAINT:
    db	"WM_PAINT",0
szWM_PAINTCLIPBOARD:
    db	"WM_PAINTCLIPBOARD",0
szWM_PASTE:
    db	"WM_PASTE",0
szWM_QUERYENDSESSION:
    db	"WM_QUERYENDSESSION",0
szWM_RBUTTONDOWN:
    db	"WM_RBUTTONDOWN",0
szWM_RENDERFORMAT:
    db	"WM_RENDERFORMAT",0
szWM_SETCURSOR:
    db	"WM_SETCURSOR",0
szWM_SETFOCUS:
    db	"WM_SETFOCUS",0
szWM_SETREDRAW:
    db	"WM_SETREDRAW",0
szWM_SETTEXT:
    db	"WM_SETTEXT",0
szWM_SETVISIBLE:
    db	"WM_SETVISIBLE",0
szWM_SIZE:
    db	"WM_SIZE",0
szWM_SIZECLIPBOARD:
    db	"WM_SIZECLIPBOARD",0
szWM_SYSCOLORCHANGE:
    db	"WM_SYSCOLORCHANGE",0
szWM_SYSCOMMAND:
    db	"WM_SYSCOMMAND",0
szWM_SYSKEYDOWN:
    db	"WM_SYSKEYDOWN",0
szWM_SYSTEMERROR:
    db	"WM_SYSTEMERROR",0
szWM_TIMER:
    db	"WM_TIMER",0
szWM_VSCROLL:
    db	"WM_VSCROLL",0
szWM_VSCROLLCLIPBOARD:
    db	"WM_VSCROLLCLIPBOARD",0
szWM_WININICHANGE:
    db	"WM_WININICHANGE",0
szUnknownMessage:
    db	"wm=0x",0
rgSzWmDebug:
    dw	offset szEM_GETHANDLE
    dw	offset szEM_GETLINECOUNT
    dw	offset szEM_GETSEL
    dw	offset szEM_REPLACESEL
    dw	offset szEM_SETHANDLE
    dw	offset szEM_SETSEL
    dw	offset szWM_ACTIVATE
    dw	offset szWM_ACTIVATEAPP
    dw	offset szWM_ASKCBFORMATNAME
    dw	offset szWM_CBTINIT
    dw	offset szWM_CBTSEMEV
    dw	offset szWM_CBTTERM
    dw	offset szWM_CHAR
    dw	offset szWM_CHILDACTIVATE
    dw	offset szWM_CLEAR
    dw	offset szWM_CLOSE
    dw	offset szWM_COMMAND
    dw	offset szWM_COPY
    dw	offset szWM_CREATE
    dw	offset szWM_CUT
    dw	offset szWM_DDE_ACK
    dw	offset szWM_DDE_ADVISE
    dw	offset szWM_DDE_DATA
    dw	offset szWM_DDE_EXECUTE
    dw	offset szWM_DDE_INITIATE
    dw	offset szWM_DDE_POKE
    dw	offset szWM_DDE_REQUEST
    dw	offset szWM_DDE_TERMINATE
    dw	offset szWM_DDE_UNADVISE
    dw	offset szWM_DESTROY
    dw	offset szWM_DESTROYCLIPBOARD
    dw	offset szWM_DEVMODECHANGE
    dw	offset szWM_ENABLE
    dw	offset szWM_ENDSESSION
    dw	offset szWM_ENTERIDLE
    dw	offset szWM_ERASEBKGND
    dw	offset szWM_FONTCHANGE
    dw	offset szWM_GETDLGCODE
    dw	offset szWM_GETMINMAXINFO
    dw	offset szWM_GETTEXT
    dw	offset szWM_GETTEXTLENGTH
    dw	offset szWM_HSCROLL
    dw	offset szWM_HSCROLLCLIPBOARD
    dw	offset szWM_INITMENUPOPUP
    dw	offset szWM_KEYDOWN
    dw	offset szWM_KEYUP
    dw	offset szWM_KILLFOCUS
    dw	offset szWM_LBUTTONDBLCLK
    dw	offset szWM_LBUTTONDOWN
    dw	offset szWM_LBUTTONUP
    dw	offset szWM_MENUCHAR
    dw	offset szWM_MENUSELECT
    dw	offset szWM_MOUSEACTIVATE
    dw	offset szWM_MOUSEMOVE
    dw	offset szWM_MOVE
    dw	offset szWM_NCCREATE
    dw	offset szWM_NCDESTROY
    dw	offset szWM_NCLBUTTONDBLCLK
    dw	offset szWM_NCPAINT
    dw	offset szWM_PAINT
    dw	offset szWM_PAINTCLIPBOARD
    dw	offset szWM_PASTE
    dw	offset szWM_QUERYENDSESSION
    dw	offset szWM_RBUTTONDOWN
    dw	offset szWM_RENDERFORMAT
    dw	offset szWM_SETCURSOR
    dw	offset szWM_SETFOCUS
    dw	offset szWM_SETREDRAW
    dw	offset szWM_SETTEXT
    dw	offset szWM_SETVISIBLE
    dw	offset szWM_SIZE
    dw	offset szWM_SIZECLIPBOARD
    dw	offset szWM_SYSCOLORCHANGE
    dw	offset szWM_SYSCOMMAND
    dw	offset szWM_SYSKEYDOWN
    dw	offset szWM_SYSTEMERROR
    dw	offset szWM_TIMER
    dw	offset szWM_VSCROLL
    dw	offset szWM_VSCROLLCLIPBOARD
    dw	offset szWM_WININICHANGE
cSzWmDebug = ($ - rgSzWmDebug) SHR 1
    errnz   <cSzWmDebug - cwmDebug>
    dw	offset szUnknownMessage

szCommaSpace:
    db	", ",0
szHwnd:
    db	"hwnd=0x",0
szWParam:
    db	"w=0x",0
szLParam:
    db	"l=0x",0

OutputMessage:
	push	ax
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	push	es
	sub	sp,80
	mov	si,sp
	push	cs
	pop	es
	push	ax
	mov	ax,bx
	mov	di,offset rgOffWndProc
	mov	cx,cOffWndProc
	repne	scasw
	jne	OM01
	dec	di
	dec	di
OM01:
	mov	di,cs:[di.(offset rgSzWndProc)-(offset rgOffWndProc)]
	call	AppendCsDiToSsSi
	mov	di,offset szCommaSpace
	call	AppendCsDiToSsSi
	pop	ax
	mov	di,offset rgwmDebug
	mov	cx,cwmDebug
	repne	scasw
	jne	OM02
	dec	di
	dec	di
OM02:
	mov	di,cs:[di.(offset rgSzWmDebug)-(offset rgwmDebug)]
	push	ax  ;save message
	push	di  ;save sz offset
	call	AppendCsDiToSsSi
	pop	di  ;restore sz offset
	pop	ax  ;restore message
	cmp	di,offset szUnknownMessage
	jne	OM03
	call	AppendHexToSsSi
OM03:
	mov	di,offset szCommaSpace
	call	AppendCsDiToSsSi
	mov	di,offset szHwnd
	call	AppendCsDiToSsSi
	mov	ax,[hwnd]
	call	AppendHexToSsSi
	mov	di,offset szCommaSpace
	call	AppendCsDiToSsSi
	mov	di,offset szWParam
	call	AppendCsDiToSsSi
	mov	ax,[wParam]
	call	AppendHexToSsSi
	mov	di,offset szCommaSpace
	call	AppendCsDiToSsSi
	mov	di,offset szLParam
	call	AppendCsDiToSsSi
	mov	ax,[SEG_LParam]
	call	AppendHexToSsSi
	mov	ax,[OFF_LParam]
	call	AppendHexToSsSi
	mov	bptr [si],0
	mov	bx,sp
	cCall	CommSzSz,<bx,si>
	add	sp,80
	pop	es
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret

AppendCsDiToSsSi:
	push	ax
	push	ds
	push	es
	xchg	si,di
	push	cs
	pop	ds
	push	ss
	pop	es
	jmp	short ACDTSS02
ACDTSS01:
	stosb
ACDTSS02:
	lodsb
	or	al,al
	jne	ACDTSS01
	xchg	si,di
	pop	es
	pop	ds
	pop	ax
	ret

AppendHexToSsSi:
	push	cx
	push	es
	xchg	si,di
	push	ss
	pop	es
	mov	cx,00404h
AHTSS01:
	rol	ax,cl
	push	ax
	and	al,00Fh
	cmp	al,10
	jb	AHTSS02
	add	al,'A'-'0'-10
AHTSS02:
	add	al,'0'
	stosb
	pop	ax
	dec	ch
	jnz	AHTSS01
	xchg	si,di
	pop	es
	pop	cx
	ret


endif ;DEBUG

sEnd    wproc
        end
