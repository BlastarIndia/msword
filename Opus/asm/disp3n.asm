        include w2.inc
        include noxport.inc
        include consts.inc
        include structs.inc

createSeg       disp3_PCODE,disp3,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midDisp3n       equ 2           ; module ID, for native asserts
endif

; EXTERNAL FUNCTIONS

ifdef DEBUG
externFP	<ScrollDCFP>
else ;!DEBUG
ifdef HYBRID
externFP	<ScrollDCPR>
else ;!HYBRID
externFP	<ScrollDC>
endif ;!HYBRID
endif ;!DEBUG
externFP	<PatBltRc>
externFP	<N_CachePara>
externFP	<FMsgPresent>
externFP	<FSectRc>
externFP	<CpFirstTap1>

ifdef DEBUG
externFP	<AssertProcForNative, S_CachePara>
endif


sBegin  data

; 
; /* E X T E R N A L S */
; 
externW         hwwdCur         ; extern struct WWD       **hwwdCur;
externW         mpwwhwwd        ; extern struct WWD       **mpwwhwwd[];
externW         vpapFetch       ; extern struct PAP       vpapFetch;
externW         caPara          ; extern struct CA        caPara;
externW         selCur          ; extern struct SEL       selCur;
externW         vsci            ; extern struct SCI       vsci;
externW 	vmpitccp	; extern CP		  vmpitccp[];
externW 	caTap		; extern struct CA	  caTap;
externW 	vtapFetch	; extern struct TAP	  vtapFetch;

; #ifdef DEBUG
ifdef DEBUG
; extern int              cHpFreeze;
externW         vdbs            ; extern struct DBS vdbs;
externW         vcaLHRUpd       ; extern struct CA  vcaLHRUpd;
externW         vwwLHRUpd       ; extern int vwwLHRUpd;
; #endif /* DEBUG */
endif

sEnd    data


; CODE SEGMENT _DISP3

sBegin	disp3
        assumes cs,disp3
        assumes ds,dgroup
        assumes ss,dgroup



;-------------------------------------------------------------------------
;       ScrollWw(ww, prcScroll, dxp, dyp, prcUpdate, prcClip)  GregC
;-------------------------------------------------------------------------
; %%Function:N_ScrollWw %%Owner:BRADV
cProc   N_ScrollWw,<PUBLIC,FAR>,<si,di>
        ParmW   ww
        ParmW   prcScroll
        ParmW   dxp
        ParmW   dyp
        ParmW   prcUpdate
        ParmW   prcClip

; /* Windows-specific scrolling function that handles the case of overlapping
;    popup windows */
; /* Returns fTrue iff update rect is larger than exposed scroll area
;    (due to a popup in the way) */
; 
; native ScrollWw( ww, prcScroll, dxp, dyp, prcUpdate, prcClip )
; int ww;
; struct RC *prcScroll;
; int dxp, dyp;
; struct RC *prcUpdate;     /* Area to update returned through here */
; struct RC *prcClip;
; {
;  struct WWD *pwwd = *mpwwhwwd [ww];
;  HDC hdc = pwwd->hdc;
        localW  hdc
;  struct RC rcT;
        localV  rcT,cbRcMin
cBegin
        mov     si,[dxp]
        mov     di,[dyp]

        mov     bx,[ww]
        shl     bx,1
        mov     bx,[bx.mpwwhwwd]
        mov     bx,[bx]
        mov     ax,[bx.hdcWwd]
        mov     [hdc],ax
; 
;  Assert( !(dxp && dyp) );
ifdef DEBUG
L100:
        or      si,si
	jz	SW4
        or      di,di
	jz	SW4
        mov     ax,midDisp3n
	mov	bx,1001
        cCall   AssertProcForNative,<ax, bx>

SW4:
endif
; 
;  if (!FSectRc( prcScroll, prcClip, &rcT))
;         return(fFalse);
        lea     ax,[rcT]
	cCall	FSectRc,<[prcScroll], [prcClip], ax>
        or      ax,ax
	jz	SW14
; 
;  ScrollDC( hdc, dxp, dyp, (LPRECT)&rcT, (LPRECT)&rcT /*lprcClip*/,
;            (HRGN)NULL /*hrgnUpdate*/, (LPRECT)prcUpdate );
        push    [hdc]
        push    si
        push    di
        lea     ax,[rcT]
        push    ds
        push    ax
        push    ds
        push    ax
        xor     ax,ax
        push    ax
        push    ds
        push    [prcUpdate]
ifdef DEBUG
        call    ScrollDCFP
else ;!DEBUG
ifdef HYBRID
	call	ScrollDCPR
else ;!HYBRID
	call	ScrollDC
endif ;!HYBRID
endif ;!DEBUG
; 
; /* only blank on horizontal scrolls; on vertical ones, the effect is
;    not too noticable and it is a speed win to not blank */
;  if (dxp)
        or      si,si
	jz	SW6
;     PatBltRc( hdc, prcUpdate, vsci.ropErase );
	cCall	PatBltRc,<[hdc], [prcUpdate], [vsci.HI_ropEraseSci], [vsci.LO_ropEraseSci]>
; 
SW6:
SW14:
; }
cEnd


ifdef NOTUSED
;-------------------------------------------------------------------------
;       UnionRc( prcDest, prc1, prc2 )                          GregC
;-------------------------------------------------------------------------
; %%Function:UnionRc %%Owner:BRADV
cProc   UnionRc,<PUBLIC,FAR>,<si,di>
        ParmW   prcDest
        ParmW   prc1
        ParmW   prc2
; /* U N I O N   R C */
; /* prcDest gets the smallest rectangle that encloses *prc1 and *prc2. */
; /* Correctly handles empty rects (as defined by FEmptyRc), and the */
; /* case when prcDest == prc1 OR prcDest == prc2 */
; native UnionRc( prcDest, prc1, prc2 )
; struct RC *prcDest;
; struct RC *prc1;
; struct RC *prc2;
; {
cBegin
; 
;  Assert( prcDest != NULL );
ifdef DEBUG
L200:
        cmp     [prcDest],0
	jnz	UR2
        mov     ax,midDisp3n
	mov	bx,1002
        cCall   AssertProcForNative,<ax, bx>

UR2:
endif
; 
        mov     si,[prc1]
        mov     di,[prcDest]

;  if (FEmptyRc( prc1 ))
        push    si
	call	far ptr FEmptyRc
        or      ax,ax
	jz	UR6
;         {
;         blt( prc2, prcDest, cwRC );
        mov     si,[prc2]

UR4:
        mov     ax,ds
        mov     es,ax
        mov     cx,cwRC
        rep     movsw
	jmp	short UR16
;         }
UR6:
;  else if (FEmptyRc( prc2 ))
;         {
;         blt( prc1, prcDest, cwRC );
;         }
        push    [prc2]
	call	far ptr FEmptyRc
        or      ax,ax
	jnz	UR4
;  else
;         {
;         prcDest->xpLeft =   min( prc1->xpLeft, prc2->xpLeft );
        mov     bx,[prc2]
        mov     ax,[si.xpLeftRc]
        mov     cx,[bx.xpLeftRc]
        cmp     ax,cx
	jle	UR8
        mov     ax,cx

UR8:
        mov     [di.xpLeftRc],ax
;         prcDest->ypTop =    min( prc1->ypTop, prc2->ypTop );
        mov     ax,[si.ypTopRc]
        mov     cx,[bx.ypTopRc]
        cmp     ax,cx
	jle	UR10
        mov     ax,cx

UR10:
        mov     [di.ypTopRc],ax
;         prcDest->xpRight =  max( prc1->xpRight, prc2->xpRight );
        mov     ax,[si.xpRightRc]
        mov     cx,[bx.xpRightRc]
        cmp     ax,cx
	jge	UR12
        mov     ax,cx

UR12:
        mov     [di.xpRightRc],ax
;         prcDest->ypBottom = max( prc1->ypBottom, prc2->ypBottom );
        mov     ax,[si.ypBottomRc]
        mov     cx,[bx.ypBottomRc]
        cmp     ax,cx
	jge	UR14
        mov     ax,cx

UR14:
        mov     [di.ypBottomRc],ax
;         }
UR16:
; }
cEnd
endif ;NOTUSED

sEnd	disp3
        end
