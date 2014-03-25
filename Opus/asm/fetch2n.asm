        include w2.inc
        include noxport.inc
        include consts.inc
        include structs.inc

createSeg       fetch2_PCODE,fetch2,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midFetch2n	equ 24		; module ID, for native asserts
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
externFP	<FSetRgchDiff>

ifdef DEBUG
externFP	<AssertProcForNative>
externFP	<S_CachePara>
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
externW		vchpGraySelCur	; extern struct CHP	vchpGraySelCur;
externW		vpapSelCur	; extern struct PAP	vpapSelCur;
externW		vpapGraySelCur	; extern struct PAP	vpapGraySelCur;
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


; CODE SEGMENT _fetch2

sBegin	fetch2
	assumes cs,fetch2
        assumes ds,dgroup
        assumes ss,dgroup

;-------------------------------------------------------------------------
;	FGetParaState(fAll, fAbortOk)
;-------------------------------------------------------------------------
; %%Function:N_FGetParaState %%Owner:BRADV
cProc   N_FGetParaState,<PUBLIC,FAR>,<si,di>
        ParmW   fAll
        ParmW   fAbortOK
; /* ****
; *  Description:  Return properties for the paragraph menu.
; *  The current selection nongray para props are left in vpapSelCur and the
; *  paragraph attributes in vpapGraySelCur are set to non-zero if that attribute
; *  differs from that in the previous paragraph. Up to cparaMax paragraphs
; *  will be checked
; *
; *    returns fFalse if interrupted, fTrue if complete. If interrupted,
; *      selCur paps remain unchanged.
; ** **** */
; 
; native FGetParaState(fAll, fAbortOk)
; BOOL fAll;
; BOOL fAbortOk;   /* if true, this routine is interruptible */
; {
; /* REVIEW do we want to use fALL ?
; */
; 
;                   /* max number of calls to CachePara */
; #define cparaMax 50
cparaMax        equ     50
; 
; int cpara = 0;
; struct PAP pap, papGray;
; CP cpNext;
        localD  cpNext
        localW  cPara
	localW	skVar	    ;fPapInitted in high byte
        localV  pap,cbPap
        localV  papGray,cbPap
cBegin

	;Assert (fAll == 0 || fAll == 1);
ifdef DEBUG
	test	[fAll],0FFFEh
	jz	FGPS01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFetch2n
	mov	bx,1004
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FGPS01:
endif ;DEBUG
	;Assembler note: we keep !fAll in the high byte of cpara so
	;that (fAll || (cpara <= cparaMax)) is really performed as
	;!(((!fAll << 8) + cpara) > (fTrue << 8) + cparaMax).
	errnz	<(cparaMax+1) AND 0FF00h>
	mov	ax,00100h
	xor	ah,bptr ([fAll])
	mov	[cpara],ax

;	/* we do some word operations, so be sure sizes are good */
;     Assert (!(cbPAPBase % sizeof(int)));
;     Assert (!(cbPAP % sizeof(int)));
	errnz	<cbPAPBase AND 1>
	errnz	<cbPAPMin AND 1>

;           /* gray out everything if no child window up */
	push	ds
	pop	es

;     if (hwwdCur == hNil)
;         {
	errnz	<hNil>
	xor	ax,ax
	cmp	[hwwdCur],ax
	jne	FGPS02

;        /* init gray pap to all gray */
;         SetWords(&vpapGraySelCur, 0xFFFF, cwPAP);
	mov	di,dataoffset [vpapGraySelCur]
	dec	ax
	errnz	<cbPAPMin AND 1>
	mov	cx,cbPAPMin SHR 1
        rep     stosw

;         return (fTrue);  /* leave fUpdate bit on */
	jmp	FGPS14

;         }
FGPS02:

;        /* init entire local gray pap to all non gray */
;     SetWords(&papGray, 0, cwPAP);
        lea     di,[papGray]
	errnz	<cbPAPMin AND 1>
	mov	cx,cbPAPMin SHR 1
        rep     stosw

;   fCol = (selCur.sk == skColumn);
	mov	al,[selCur.skSel]
	and	al,maskSkSel
	mov	[skVar],ax	;also resets fPapInitted in high byte

;#ifdef DEBUG
;   if (fCol)
;	Assert (selCur.itcFirst >= 0 && selCur.itcLim >= 0);
;#endif /* DEBUG */
ifdef DEBUG
	cmp	al,skColumn SHL ibitSkSel
	jne	FGPS04
	cmp	[selCur.itcFirstSel],0
	jl	FGPS03
	cmp	[selCur.itcLimSel],0
	jge	FGPS04
FGPS03:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFetch2n
	mov	bx,1005
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FGPS04:
endif ;DEBUG

;   cpNext = selCur.cpFirst;
	mov	ax,[selCur.LO_cpFirstSel]
	mov	dx,[selCur.HI_cpFirstSel]
	mov	[OFF_cpNext],ax
	mov	[SEG_cpNext],dx

;   /* Need to initialize pap for insertion points and one line block
;      selections because they don't get inside the following while loop. */
;   CachePara (selCur.doc, cpNext);
	push	[selCur.docSel]
	push	dx
	push	ax
ifdef DEBUG
	cCall	S_CachePara,<>
else ;not DEBUG
	cCall	N_CachePara,<>
endif ;DEBUG

;   blt( &vpapFetch, &pap, cwPAP );
	push	ds
	pop	es
	mov	si,dataoffset [vpapFetch]
        lea     di,[pap]
	errnz	<cbPAPMin AND 1>
	mov	cx,cbPAPMin SHR 1
        rep     movsw

;     while (cpNext < selCur.cpLim &&
;	      (fAll || (cpara <= cparaMax)))
;         {
FGPS06:
	mov	ax,[OFF_cpNext]
	mov	dx,[SEG_cpNext]
        sub     ax,[selCur.LO_cpLimSel]
        sbb     dx,[selCur.HI_cpLimSel]
	jge	Ltemp001
	;Assembler note: we keep !fAll in the high byte of cpara so
	;that (fAll || (cpara <= cparaMax)) is really performed as
	;!(((!fAll << 8) + cpara) > (fTrue << 8) + cparaMax).
	errnz	<(cparaMax+1) AND 0FF00h>
	cmp	[cpara],(fTrue SHL 8) + cparaMax
	ja	Ltemp001

;               /* interrupted? */
;         if ( fAbortOk &&
; /* take heed: FMsgPresent can call CachePara, move the heap, etc */
;              FMsgPresent(mtyIdle) )
;             {
	cmp	[fAbortOk],fFalse
	je	FGPS07
        mov     ax,mtyIdle
	cCall	FMsgPresent,<ax>
        or      ax,ax
	jz	FGPS07

;             return (fFalse);
;             }
	jmp	FGPS15
Ltemp001:
	jmp	FGPS12
FGPS07:

;	if (fCol)
;	    CpFirstTap(selCur.doc, cpNext);
;	cpT = (!fCol) ? cpNext : vmpitccp[selCur.itcFirst];
;	   /* will make non col case go only once */
;	cpLimT = (!fCol) ? cpNext + 1 : vmpitccp[selCur.itcLim];
	mov	bx,[OFF_cpNext]
	mov	cx,[SEG_cpNext]
	mov	ax,1
	cwd
	add	ax,bx
	adc	dx,cx
	cmp	bptr ([skVar]),skColumn SHL ibitSkSel
	jne	FGPS08
	;***Begin in line CpFirstTap
;	 return CpFirstTap1(doc,cpFirst,caTap.doc==doc?vtapFetch.fOutline:fFalse);
	mov	dx,[selCur.docSel]
	cmp	dx,[caTap.docCa]
	mov	al,[vtapFetch.fOutlineTap]
	je	FGPS073
	mov	al,fFalse
FGPS073:
	and	ax,maskFOutlineTap
	cCall	CpFirstTap1,<dx, cx, bx, ax>
	;***End in line CpFirstTap
	mov	ax,[selCur.itcFirstSel]
	shl	ax,1
	shl	ax,1
	add	ax,dataoffset [vmpitccp]
	xchg	ax,si
	lodsw
	xchg	ax,bx
	lodsw
	xchg	ax,cx
	mov	ax,[selCur.itcLimSel]
	shl	ax,1
	shl	ax,1
	add	ax,dataoffset [vmpitccp]
	xchg	ax,si
	lodsw
	xchg	ax,dx
	lodsw
	xchg	ax,dx
FGPS08:

;	while (cpT < cpLimT)
;	    {
	cmp	bx,ax
	mov	si,cx
	sbb	si,dx
	jge	FGPS095
	push	dx
	push	ax	;save cpLimT

;	    /* If any props are different, set appropriate flags */
;	    CachePara (selCur.doc, cpT);
	push	[selCur.docSel]
	push	cx
	push	bx
ifdef DEBUG
	cCall	S_CachePara,<>
else ;not DEBUG
	cCall	N_CachePara,<>
endif ;DEBUG

;	    cpT = caPara.cpLim;
	push	[caPara.HI_cpLimCa]
	push	[caPara.LO_cpLimCa]

;	    if (++cpara == 1)
;		blt( &vpapFetch, &pap, cwPAP );     /* save 1st paragraph for compares */
	inc	bptr ([cpara])
	push	ds
	pop	es
	mov	al,fTrue
	xchg	al,bptr ([skVar+1])
	or	al,al	    ;fPapInitted
	jne	FGPS09
	mov	si,dataoffset [vpapFetch]
	lea	di,[pap]
	errnz	<cbPAPMin AND 1>
	mov	cx,cbPapMin SHR 1
	rep	movsw
	;Assembler note: just fall through and perform harmless operations.
	;This case only occurs once per call to FGetParaState
FGPS09:

;	    else if (!vpapFetch.fTtp)
;		{
       cmp	bptr [vpapFetch.fTtpPap],fFalse
       jne	FGPS093

;            /* get base of pap */
;		FSetRgwDiff (&pap, &vpapFetch, &papGray,
;			cbPAPBase / sizeof (int));
	;LN_FSetPapDiff takes an offset in di, cb in cx and performs
	;FSetRgchDiff(pap+di, vpapFetch+di, papGray+di, cx);
	;ax, bx, cx, dx, si, di are altered.
	xor	di,di
	mov	cx,cbPAPBase
	call	LN_FSetPapDiff

;		  /* also check in tab tables */
;		FSetRgwDiff (pap.rgdxaTab, vpapFetch.rgdxaTab,
;		    papGray.rgdxaTab, pap.itbdMac);
;		 /* note chars, not words here */
	;LN_FSetPapDiff takes an offset in di, cb in cx and performs
	;FSetRgchDiff(pap+di, vpapFetch+di, papGray+di, cx);
	;ax, bx, cx, dx, si, di are altered.
	mov	di,(rgdxaTabPap)
	mov	cx,[pap.itbdMacPap]
	shl	cx,1
	call	LN_FSetPapDiff

;		FSetRgchDiff (pap.rgtbd, vpapFetch.rgtbd,
;		    papGray.rgtbd, pap.itbdMac);
	;LN_FSetPapDiff takes an offset in di, cb in cx and performs
	;FSetRgchDiff(pap+di, vpapFetch+di, papGray+di, cx);
	;ax, bx, cx, dx, si, di are altered.
	mov	di,(rgtbdPap)
	mov	cx,[pap.itbdMacPap]
	call	LN_FSetPapDiff

FGPS093:
	pop	bx
	pop	cx	;restore cpT
	pop	ax
	pop	dx	;restore cpLimT

;		}
;	    }
	jmp	FGPS08

FGPS095:
;	cpNext = (!fCol) ? caPara.cpLim : caTap.cpLim;
	mov	si,dataoffset [caPara.LO_cpLimCa]
	cmp	bptr ([skVar]),skColumn SHL ibitSkSel
	jne	FGPS10
	mov	si,dataoffset [caTap.LO_cpLimCa]
FGPS10:
	push	ds
	pop	es
	lea	di,[cpNext]
	movsw
	movsw

;	}
	jmp	FGPS06

FGPS12:
;           /* if we ran out, say everything is gray */

;     if (!fAll && cpara > cparaMax)
	;Assembler note: we keep !fAll in the high byte of cpara so
	;that (fAll || (cpara <= cparaMax)) is really performed as
	;!(((!fAll << 8) + cpara) > (fTrue << 8) + cparaMax).
	push	ds
	pop	es
	mov	di,dataoffset [vpapGraySelCur]
	mov	cx,cbPapMin SHR 1
        lea     si,[papGray]
	errnz	<(cparaMax+1) AND 0FF00h>
	cmp	[cpara],(fTrue SHL 8) + cparaMax
	jbe	FGPS13

;          SetWords(&vpapGraySelCur, 0xFFFF, cwPAP);
        mov     ax,0ffffh
        rep     stosw
	db	03Dh	;turns next "rep movsw" into "cmp ax,immediate"

FGPS13:
;     else  /* normal finish */
;          blt(&papGray, &vpapGraySelCur, cwPAP);
        rep     movsw

;    blt(&pap, &vpapSelCur, cwPAP);
        lea     si,[pap]
	mov	di,dataoffset [vpapSelCur]
	mov	cx,cbPapMin SHR 1
        rep     movsw
; 
;    selCur.fUpdatePap = fFalse;
        and     [selCur.fUpdatePapSel],NOT maskfUpdatePapSel

;            /* if someone besides the ruler called me, set this to
;               ensure ruler update at idle  */

;    selCur.fUpdateRuler = fTrue;
        or      [selCur.fUpdateRulerSel],maskfUpdateRulerSel

; #ifdef DEBUG  /* save selection range and ww for CkLHRUpd debug test */
ifdef DEBUG
;         {
;         extern struct CA  vcaLHRUpd;
;         extern int   vwwLHRUpd; /* window where selection is shown */
; 
;         vcaLHRUpd = selCur.ca;
	mov	si,dataoffset [selCur.caSel]
	mov	di,dataoffset [vcaLHRUpd]
	mov	cx,cbCaMin SHR 1
        rep     movsw

;         vwwLHRUpd = selCur.ww;
        mov     ax,[selCur.wwSel]
        mov     [vwwLHRUpd],ax

;         }
; #endif
endif

;    return (fTrue);
FGPS14:
	db	0B8h	;turns next "xor ax,ax" into "mov ax,immediate"
FGPS15:
	errnz	<fFalse>
	xor	ax,ax

; }
cEnd


	;LN_FSetPapDiff takes an offset in di, cb in cx and performs
	;FSetRgchDiff(pap+di, vpapFetch+di, papGray+di, cx);
	;ax, bx, cx, dx, si, di are altered.
LN_FSetPapDiff:
	lea	si,[pap+di]
	lea	bx,[papGray+di]
	add	di,dataoffset [vpapFetch]
	cCall	FSetRgchDiff,<si, di, bx, cx>
	ret

sEnd	fetch2
        end
