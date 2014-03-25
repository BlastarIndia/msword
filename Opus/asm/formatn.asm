;	Assembly flags:
;		BOTH -- define different names for functions so that
;			FORMAT.C and FORMATN.ASM can coexist.
;			Native names are formed by appending "N"

	include w2.inc
	include noxport.inc
	include consts.inc
	include structs.inc

	.list

createSeg	_FORMAT,format,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
BOTH		equ 0		; enable different name
midFormatn	equ 0		; module ID, for native asserts
NatPause equ 1
endif

ifdef	NatPause
PAUSE	MACRO
	int 3
	ENDM
else
PAUSE	MACRO
	ENDM
endif
; EXPORTED LABELS

ifdef DEBUG
PUBLIC  LNotFirstLine
PUBLIC  LNoDxtTopOfLoop
PUBLIC  LDxtTopOfLoop
PUBLIC  LNoDxtHaveCh
PUBLIC  LDxtHaveCh
PUBLIC  LNoDxtHaveDxp
PUBLIC  LDxtHaveDxp
PUBLIC  LNoDxtDef
PUBLIC  LDxtDef
PUBLIC  LNoDxtPChar
PUBLIC  LDxtPChar
PUBLIC  LBreak
PUBLIC  LFetch
PUBLIC  LChNonReqHyphen
PUBLIC  LNonBreakHyphen
PUBLIC  LChNonBreakSpace
PUBLIC  LChHyphen
PUBLIC  LChNull
PUBLIC  LChSpace
PUBLIC  LBreakOppR
PUBLIC  LChReturn
PUBLIC  LChEop
PUBLIC  LChCRJ
PUBLIC  LEol
PUBLIC  LChColumnBreak
PUBLIC  LChSect
PUBLIC  LChTab
PUBLIC  LChrT
PUBLIC  LNewRun
PUBLIC  LVanish
PUBLIC  LVanish1
PUBLIC  LEndBreak0
PUBLIC  LEndJ
PUBLIC  LEndNoJ
PUBLIC  LNonDefaultCh
PUBLIC  LEndFormatLine
PUBLIC  LChRightParen
PUBLIC  LChLeftParen
PUBLIC  LChComma
PUBLIC  LChFormula
endif ;DEBUG


; EXTERNAL FUNCTIONS

externFP	<CacheSectProc,N_CachePara,N_FetchCp>
externFP	<CpMacDoc,N_PdodMother>
externFP	<CpMacDocEdit>
externFP	<OutlineProps,OutlineChp>
externFP	<FExpandGrpchr,DxtLeftOfTabStop>
externFP	<N_FfcFormatFieldPdcp>
externFP	<FormatChSpec>
externFP	<FormatFormula>
externFP	<N_LoadFont>
externFP	<SetFlm>
externFP	<CacheTc>
externFP	<FInCa>
externFP	<ReloadSb>
externFP	<ChFetch>
externFP	<FormatBorders>
externFP	<FormatChPubs>
externFP	<CpLimField>
externFP	<N_FInTableDocCp>
externFP	<FAbsPap>
externFP	<FMatchAbs>

ifdef DEBUG
externFP	<CkVfli,S_FetchCp,S_CachePara>
externFP	<AssertProcForNative,ScribbleProc,ShakeHeapSb>
externFP	<S_FfcFormatFieldPdcp,S_LoadFont>
externFP	<S_FInTableDocCp>
externFP	<LockHeap, UnlockHeap>
externFP	<S_FormatLineDxa>
endif ;DEBUG


; EXTERNAL DATA

sBegin	data
externW 		vhgrpchr
externW 		vbchrMax
externW 		vbchrMac
externW 		vlm
externW 		mpwwhwwd
externW 		caPara
externW 		caSect
externW 		vpffs
externW 		vchpFetch
externW 		vsepFetch
externW 		vpapFetch
externD 		vcpFetch
externD 		vhpchFetch
externW 		vccpFetch
externW 		fnFetch
externW 		vdocFetch
externW 		mpdochdod
externW 		vsci
externW 		vfli
externW 		vfti
externW 		vftiDxt
externW 		vitr
externW 		vfmtss
externW 		vflm
externW 		vifmaMac
externW 		vtcc
externW 		rgfma
externW 		mpsbps
externD 		vcpFirstTablePara
externW 		vrf


ifdef DEBUG
externW 		cHpFreeze
externW 		vdbs
externW 		wFillBlock
endif


sEnd	data


; CODE SEGMENT _FORMAT

sBegin	format
	assumes cs,format
	assumes ds,dgroup
	assumes ss,dgroup

; DO NOT ADD THINGS HERE; CRITICAL FORMATLINE CODE ASSUMES mpchilbl == 0!

; table mapping ch --> ilbl for the big switch statement in FormatLine
iWLChNull	    = ((offset WLChNull) - (offset rglbl))
iWLChTab	    = ((offset WLChTab) - (offset rglbl))
iWLChEop	    = ((offset WLChEop) - (offset rglbl))
iWLChCRJ	    = ((offset WLChCRJ) - (offset rglbl))
iWLChSect	    = ((offset WLChSect) - (offset rglbl))
iWLChReturn	    = ((offset WLChReturn) - (offset rglbl))
iWLChNonReqHyphen   = ((offset WLChNonReqHyphen) - (offset rglbl))
iWLChSpace	    = ((offset WLChSpace) - (offset rglbl))
iWLChHyphen	    = ((offset WLChHyphen) - (offset rglbl))
iWLChNonBreakHyphen = ((offset WLChNonBreakHyphen) - (offset rglbl))
iWLChColumnBreak    = ((offset WLChColumnBreak) - (offset rglbl))
iWLChNonBreakSpace  = ((offset WLChNonBreakSpace) - (offset rglbl))
iWLChLeftParen	    = ((offset WLChLeftParen) - (offset rglbl))
iWLChRightParen     = ((offset WLChRightParen) - (offset rglbl))
iWLChFormula	    = ((offset WLChFormula) - (offset rglbl))
iWLChComma	    = ((offset WLChComma) - (offset rglbl))
iWLChTable	    = ((offset WLChTable) - (offset rglbl))
iWLChPubs	    = ((offset WLChPubs) - (offset rglbl))
iWLChMax	    = ((offset rglblMax) - (offset rglbl))

;Assembler note:  mpchilbl has become complex because it does a lot of
;things.  First of all if (mpchilbl[ch] AND 03Eh) != 0 we want to jump
;to (rglbl - 2)[(mpchilbl[ch] AND 03Eh)] whenever ch is encountered.
;This leaves three bits in each byte that can be used for other purposes.
;The purpose most suited to these spare bits is to reduce the time spent
;in the inner loop of FormatLine checking for the vitr.chList character.
;To do this we keep (while in the inner loop) a mask in register dh for
;a bit in mpchilbl[ch] to test for whether ch may be chList.  Optimally we
;would like to have (mpchilbl[ch] AND register dh) != 0 iff
;vitr.chList == ch.  But vitr.chList may be any character, so the optimal
;arrangement just described is impossible.  However, in all of the languages
;supported by windows, vitr.chList is by default either ',' or ';', any
;other character is rarely used for vitr.chList, so optimizing for these
;two characters would be nearly as good as optimizing for all possible
;chList characters.  So a quick test for whether we have a special character
;which requires leaving the inner loop of FormatLine could be made if
;we have one bit that is on only for chComma, chSemicolon, and characters
;in which (mpchilbl[ch] AND 03Eh) != 0 (in particular bit 040h).  If
;vitr.chList is either chComma or chSemicolon then we set that same bit
;in register dh.  If vitr.chList is some other character we set register
;dh to some other bit that is on for all characters (bit 001h).  This way
;the inner loop of FormatLine runs at almost the same speed as if there
;were no vitr.chList unless vitr.chList is not ',' or ';'.
mpchilbl:
db	iWLChNull + 043h		; chNull == 0
db	(7-1) dup (001h)
db	iWLChTable + 043h		; chTable == 7
db	(9-8) dup (001h)
db	iWLChTab + 043h 		; chTab == 9
db	iWLChEop + 043h 		; chEop == 10
db	iWLChCRJ + 043h 		; chCRJ == 11
db	iWLChSect + 043h		; chSect == 12
db	iWLChReturn + 043h		; chReturn == 13
db	iWLChColumnBreak + 043h 	; chColumnBreak == 14
db	(30-15) dup (001h)
db	iWLChNonBreakHyphen + 043h	; chNonBreakHyphen == 30
db	iWLChNonReqHyphen + 043h	; chNonReqHyphen == 31
db	iWLChSpace + 043h		; chSpace == 32
db	(40-33) dup (001h)
db	iWLChLeftParen + 043h		; chLeftParen == 40
db	iWLChRightParen + 043h		; chRightParen == 41
db	(44-42) dup (001h)
db	041h				; chComma == 44
db	iWLChHyphen + 043h		; chHyphen == 45
db	(59-46) dup (001h)
db	041h				; chSemicolon == 59
db	(92-60) dup (001h)
db	iWLChFormula + 043h		; chFormula == 92
db	(147-93) dup (001h)
db	(152-147) dup (iWLChPubs + 043h); chPubs == 147 - 151
db	(160-152) dup (001h)
db	iWLChNonBreakSpace + 043h	; chNonBreakSpace == 160
db	(256-161) dup (001h)
					; chMax == 256

; array of jump labels for the big switch statement in FormatLine

rglbl:
WLChNull:		dw	offset LChNull
WLChTab:		dw	offset LChTab
WLChEop:		dw	offset LChEop
WLChCRJ:		dw	offset LChCRJ
WLChSect:		dw	offset LChSect
WLChReturn:		dw	offset LChReturn
WLChNonReqHyphen:	dw	offset LChNonReqHyphen
WLChSpace:		dw	offset LChSpace
WLChHyphen:		dw	offset LChHyphen
WLChNonBreakHyphen:	dw	offset LChNonBreakHyphen
WLChColumnBreak:	dw	offset LChColumnBreak
WLChNonBreakSpace:	dw	offset LChNonBreakSpace
WLChLeftParen:		dw	offset LChLeftParen
WLChRightParen: 	dw	offset LChRightParen
WLChFormula:		dw	offset LChFormula
WLChComma:		dw	offset LChComma
WLChTable:		dw	offset LChTable
WLChPubs		dw	offset LChPubs
rglblMax:


;/* F O R M A T  L I N E */
;/* formats line in doc in ww starting at cp. */
;/* works for text in tables */
;NATIVE FormatLine(ww, doc, cp)
;int ww, doc;
;CP cp;
;{
;	 if (!FInCa(doc, cp, &caSect))
;		 CacheSect(doc, cp);
;	 FormatLineDxa(ww, doc, cp, vsepFetch.dxaColumnWidth);
;}
;
;/* formats line in doc in ww starting at cp possibly according to the
;width of pdr */
;NATIVE FormatLineDr(ww, cp, pdr)
;int ww;
;CP cp;
;struct DR *pdr;
;{
;	 int dxa;
;	 int doc = pdr->doc;
;	 if (!pdr->fForceWidth)
;		 {
;		 if (!FInCa(doc, cp, &caSect))
;			 CacheSect(doc, cp);
;		 dxa = vsepFetch.dxaColumnWidth;
;		 }
;	 else
;		 {
;#ifdef WIN
;/* set up at least vfti.dxpInch right for DxaFromDxp */
;		 int flmWw = PwwdWw(ww)->grpfvisi.flm;
;		 if (flmWw != vflm)
;			 SetFlm(flmWw);
;#endif
;		 dxa = pdr->dxa;
;		 }
;	 FormatLineDxa(ww, doc, cp, dxa);
;}
; %%Function:N_FormatDrLine %%Owner:BRADV
PUBLIC N_FormatDrLine
N_FormatDrLine:
	db	0B9h	;turns next "xor cx,cx" into "mov cx,immediate"
; %%Function:N_FormatLine %%Owner:BRADV
PUBLIC N_FormatLine
N_FormatLine:
	xor	cx,cx

cProc	N_FormatLineCommon,<PUBLIC,FAR>,<si,di>
	ParmW	ww
	ParmW	doc
	ParmD	cp

cBegin

	;***Begin modifying arguments if really FormatLineDr or FormatLine
	mov	bx,[doc]
	mov	di,[SEG_cp]
	mov	si,[OFF_cp]
	jcxz	LN_FormatLine

LN_FormatDrLine:
;	 int doc = pdr->doc;
;we have cp in bx:di, pdr in si
	xchg	si,bx
	xchg	si,di
;we have cp in di:si, pdr in bx
	mov	al,[bx.fForceWidthDr]
	mov	dx,[bx.dxaDr]
	mov	bx,[bx.docDr]
;we have cp in di:si, doc in bx, fForceWidth in al, pdr->dxl in dx

;	 if (!pdr->fForceWidth)
;		 {
;		 if (!FInCa(doc, cp, &caSect))
;			 CacheSect(doc, cp);
;		 dxa = vsepFetch.dxaColumnWidth;
;		 }
;	 else
;		 {
;/* set up at least vfti.dxpInch right for DxaFromDxp */
;		 int flmWw = PwwdWw(ww)->grpfvisi.flm;
;		 if (flmWw != vflm)
;			 SetFlm(flmWw);
;		 dxa = pdr->dxa;
;		 }
;	 FormatLineDxa(ww, doc, cp, dxa);
	test	al,maskFForceWidthDr
	je	LN_FormatLine
	push	bx	;save doc
	mov	bx,[ww]
	shl	bx,1
	mov	bx,[bx.mpwwhwwd]
	mov	bx,[bx]
	errnz	<shftFlmGrpfvisi-8>
	mov	al,bptr ([bx+grpfvisiWwd+1])
	and	ax,maskflmGrpfvisi SHR 8
	cmp	ax,[vflm]
	je	FL005
	push	dx	;save dxa
	cCall	SetFlm,<ax>
	pop	dx	;restore dxa
FL005:
	pop	bx	;restore doc
	jmp	short FL01

LN_FormatLine:
;	 if (!FInCa(doc, cp, &caSect))
;		 CacheSect(doc, cp);
;	 FormatLineDxa(ww, doc, cp, vsepFetch.dxaColumnWidth);
	;Assembler note: we have ww, doc, cp and a junk word as arguments
	;on the stack, we want to change the junk word to dxa
	;LN_CacheSect performs
	;if (!FInCa(bx, di:si, &caSect)) CacheSect(bx, di:si);
	;ax, bx, cx, dx are altered.
	push	bx	;save doc
	call	LN_CacheSect
	pop	bx	;restore doc
	mov	dx,[vsepFetch.dxaColumnWidthSep]
FL01:
	;***End modifying arguments if really FormatLineDr or FormatLine
	push	[ww]
	push	bx	;doc
	push	di
	push	si	;cp
	push	dx	;dxa
ifdef DEBUG
	cCall	S_FormatLineDxa,<>
else ;!DEBUG
	push	cs
	call	near ptr N_FormatLineDxa
endif ;!DEBUG
cEnd

maskFPrevSpace	    =	001h
maskFHeightPending  =	002h

;-------------------------------------------------------------------------
;	N_FormatLineDxa( ww, doc, cp, dxa )
;
;
;	Format a line of text starting at doc, cp.  Fill out vfli
;	and vhgrpchr.
;-------------------------------------------------------------------------
; %%Function:N_FormatLineDxa %%Owner:BRADV
cProc	N_FormatLineDxa,<PUBLIC,FAR>,<si,di>
	ParmW	ww
	ParmW	doc
	ParmD	cp
	ParmW	dxa


; /* F O R M A T  L I N E */
; /* formats line in doc starting at cp. */

; native C_FormatLine(ww, doc, cp)
; int ww; int doc; CP cp;
; {
	localW docNext		;	int docNext;
	OFFBP_docNext		=   -2
	localD cpNext		;	CP cpNext;
	OFFBP_cpNext		=   -6
	localW xpVar		;	int xp;
	OFFBP_xpVar		=   -8
	localW xtVar		;	int xt;
	OFFBP_xtVar		=   -10
	localW dytDescentMac	;	int dytDescentMac;
	OFFBP_dytDescentMac	=   -12
	localW dypDescentMac	;	int dypDescentMac;
	OFFBP_dypDescentMac	=   -14
	localW dytAscentMac	;	int dytAscentMac;
	OFFBP_dytAscentMac	=   -16
	localW dypAscentMac	;	int dypAscentMac;
	OFFBP_dypAscentMac	=   -18
	localW ichVar		;	int ich;
	OFFBP_ichVar		=   -20
	localW cParen		;	char cParen;
	OFFBP_cParen		=   -22
	localW ichSpace 	;	int ichSpace;
	OFFBP_ichSpace		=   -24
	localW itbd		;	int itbd;
	OFFBP_itbd		=   -26
				;	int chReal;
	localW dytDescentBreak	;	int dytDescentBreak;
	OFFBP_dytDescentBreak	=   -28
	localW dypDescentBreak	;	int dypDescentBreak;
	OFFBP_dypDescentBreak	=   -30
	localW dytAscentBreak	;	int dytAscentBreak;
	OFFBP_dytAscentBreak	=   -32
	localW dypAscentBreak	;	int dypAscentBreak;
	OFFBP_dypAscentBreak	=   -34
	localW fInhibitJust	;	BOOL fInhibitJust;
	OFFBP_fInhibitJust	=   -36
	localW fPrevSpace	;	BOOL fPrevSpace;
	OFFBP_fPrevSpace	=   -38
	localW cchSpace 	;	int cchSpace;
	OFFBP_cchSpace		=   -40
	localW chVar		;	int ch;
	OFFBP_chVar		=   -42
	localW dytDescent	;	int dytDescent;
	OFFBP_dytDescent	=   -44
	localW dypDescent	;	int dypDescent;
	OFFBP_dypDescent	=   -46
	localW dytAscent	;	int dytAscent;
	OFFBP_dytAscent 	=   -48
	localW dypAscent	;	int dypAscent;
	OFFBP_dypAscent 	=   -50
	localW xpJust		;	int xpJust;
	OFFBP_xpJust		=   -52
	localW bchrChp		;	int bchrChp;
	localW fPassChSpec	;	BOOL fPassChSpec;
	localW dxtTabMinT	;	int dxtTabMinT
	localW jcVar		;	int jc;
	localW jcTab		;	int jcTab;
; /* set if separate xp/xt calculations are needed */
	localW fDxt		;	BOOL fDxt;
	localW dxt		;	uns dxt;
	localW dxp		;	uns dxp;
	localW dypBefore	;	int dypBefore;
; /* set if font changed and dyp*Mac will have to be updated when non-blank
; character appears */
	; bit 1 of fPrevSpace is used for fHeightPending
				;	BOOL fHeightPending;
	localW <OFF_hpch>	;	char HUGE *hpch;
	localW <OFF_hpchLim>	;	char HUGE *hpchLim;
	localW xtJust		;	int xtJust;
	localW xtRight		;	int xtRight;
	localW xaRight		;	int xaRight;
	localW xaLeft		;	int xaLeft;
				;	int cchWide;
	localW bchrPrev 	;	int bchrPrev; /* used to back up over ch if no break opportunity */
	localW bchrNRH		;	int bchrNRH;
	localW bchrBreak	;	int bchrBreak;

	localW xtTabStart	;	int xtTabStart;
	localW xpTabStart	;	int xpTabStart;
	localW xtTabStop	;	int xtTabStop;
				;	int chT;
				;	int ichT;
				;	int ichT1;
	;tlc in bptr [jcTab+1]	;	int tlc;
	localW pdxpFli
				;	struct CHR *pchr;
	localD cpCurPara	;	CP cpCurPara;
				;	struct DOD *pdodT;
				;	struct WWD *pwwd;
	localD dcpVanish	;	CP dcpVanish;
				;	struct FMA rgfma[ifmaMax];
	; high byte of fInTable is used for fAbs
				;	int fAbs;
	localW fInTable 	;	BOOL fInTable;
	localD cpFirstPara	;	CP cpFirstPara;

				;	CP cpLim;
				;	int dxaMarginRight
				;	int dxaMarginLeft
	localD cpT		;	CP cpT;
				;	int ffc;
				;	int ddxp;
	localV chpFLDxa,cbChpMin;	struct CHP chpFLDxa;
;	/* below here offsets take 2 bytes instead of one */
	localW ichTab		;	int ichTab;
	localW itbdMac		;	int itbdMac;
	localW fStopFormat	;	BOOL fStopFormat;
				;	int dichSpace3;
; /* pointer to either vfti or vftiDxt */
	localW pftiDxt		;	struct FTI *pftiDxt;
	localW dxpT		;	int dxpT;
	localW fcm		;	int fcm;
	localD fcid		;	union FCID fcid;
	localW <SEG_hpch>
	localV fmal,cbFmalMin	;	struct FMAL fmal;
	localV ffs,cbFfsMin	;	struct FFS ffs;
	localW mpXtXp
	localV pap,cbPAPBaseScan

cBegin		; end of local declarations; beginning of FormatLine code


	;Assembler note: do this just after FL09
;	dyyAscent.wlAll = 0;
;	dyyDescent.wlAll = 0;
;	xpJust = 0;
;	dichSpace3 = 0;

	;Assert ss == ds
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,ds
	mov	bx,ss
	cmp	ax,bx
	je	FL013
	mov	ax,midFormatn
	mov	bx,1007
	cCall	AssertProcForNative,<ax,bx>
FL013:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;	vrf.fInFormatLine = !vrf.fInFormatLine;
;	Assert (vrf.fInFormatLine);
;	if (!vrf.fInFormatLine)
;		{
;		vrf.fInFormatLine = !vrf.fInFormatLine;
;		return;
;		}
	xor	[vrf.fInFormatLineRf],maskfInFormatLineRf
	errnz	<maskFInFormatLineRf - 080h>
ifdef DEBUG
	js	FL015
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFormatn
	mov	bx,1060 		; label # for native assert
	cCall	AssertProcForNative, <ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	jmp	LEndFormatLine
FL015:
else ;!DEBUG
	jns	LEndFormatLine
endif ;!DEBUG

;	pwwd = *mpwwhwwd[ww];
	mov	si,[ww]
	mov	bx,si
	mov	di,[bx+si+mpwwhwwd]
	mov	di,[di]

;	if (pwwd->grpfvisi.flm != vflm)
;		{
;		SetFlm( pwwd->grpfvisi.flm );
;		pwwd = *mpwwhwwd [ww];
;		}
	errnz	<shftFlmGrpfvisi-8>
	mov	al,bptr ([di+grpfvisiWwd+1])
	and	ax,maskflmGrpfvisi SHR 8

;	Assert( vflm == flmDisplayAsPrint || vflm == flmDisplay ||
;		vflm == flmPrint || vflm == flmRepaginate );
ifdef DEBUG
	cmp	al,flmDisplayAsPrint
	je	FL017
	cmp	al,flmDisplay
	je	FL017
	cmp	al,flmPrint
	je	FL017
	cmp	al,flmRepaginate
	je	FL017
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFormatn
	mov	bx,1001
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FL017:
endif ;/* DEBUG */

	cmp	ax,[vflm]
	je	FL02
	cCall	SetFlm,<ax>
	mov	bx,si
	mov	di,[bx+si+mpwwhwwd]
	mov	di,[di]
FL02:

;/* check cache for fli current */
;	if (ww == vfli.ww && vfli.doc == doc && vfli.cpMin == cp &&
;			pwwd->grpfvisi.w == vfli.grpfvisi.w &&
;			vfli.fPageView == pwwd->fPageView && vfli.dxa == dxa)
;		    {
;		    vrf.fInFormatLine = !vrf.fInFormatLine;
;		    return; /* Just did this one */
;		    }
	mov	bx,[doc]
	mov	dx,[SEG_cp]
	mov	cx,[OFF_cp]

	; bx = doc, dx:cx = cp, si = ww, di = pwwd
	cmp	si,[vfli.wwFli]
	jne	FL03
	cmp	bx,[vfli.docFli]
	jnz	FL032
	mov	ax,[vfli.grpfvisiFli]
	cmp	ax,[di.grpfvisiWwd]
	jnz	FL034
	xor	ax,ax
	mov	al,[vfli.fPageViewFli]
	dec	ax	;not vfli.fPageView in ah
	xor	ah,[di.fPageViewWwd]
	and	ah,maskfPageViewWwd
	jz	FL036
	mov	ax,[vfli.dxaFli]
	cmp	ax,[dxa]
	jnz	FL038
	cmp	cx,[vfli.LO_cpMinFli]
	jnz	FL038
	cmp	dx,[vfli.HI_cpMinFli]
	jnz	FL038
FL027:
LEndFormatLine:
	xor	[vrf.fInFormatLineRf],maskfInFormatLineRf
cEnd

;/* obtain any format modes from ww */
;	vfli.ww = ww;
;	vfli.fOutline = pwwd->fOutline;
;	vfli.grpfvisi = pwwd->grpfvisi;
;	 Assert( doc != docNil );
;	 vfli.omk = omkNil;	 /* no outline mark */
;	 vfli.doc = doc;
;	 vfli.fPageView = pwwd->fPageView;

	; bx = doc, dx:cx = cp, si = ww, di = pwwd
FL03:
	mov	[vfli.wwFli],si
FL032:
	mov	[vfli.docFli],bx
FL034:
	mov	ax,[di.grpfvisiWwd]
	mov	[vfli.grpfvisiFli],ax
FL036:
	mov	al,[di.fPageViewWwd]
	and	al,maskfPageViewWwd
	mov	[vfli.fPageViewFli],al
FL038:
ifdef PROFILE
; %%Function:N_FormatLineCore %%Owner:BRADV
PUBLIC N_FormatLineCore
N_FormatLineCore:
	cCall	StartNMeas,<>
endif ;PROFILE
	mov	al,[di.fOutlineWwd]
	and	al,maskfOutlineWwd
	mov	[vfli.fOutlineFli],al

ifdef DEBUG
	cmp	bx,docNil
	jne	FL05
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFormatn
	mov	bx,1002
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FL05:
endif ;/* DEBUG */

	; bx = doc, dx:cx = cp, si = ww, di = pwwd
	mov	[vfli.omkFli],omkNil
	push	bx	;argument for FAbsPap
	mov	ax,dataoffset [vpapFetch]
	push	ax	;argument for FAbsPap

;	 if (!FInCa(doc, cp, &caPara))
;		 CachePara(doc, cp);
	; bx = doc, dx:cx = cp, si = ww, di = pwwd
	;Assembler note: Don't bother with the FInCa here, CachePara
	;is in assembler and does a very quick FInCa, so just call it.
ifdef DEBUG
	cCall	S_CachePara,<bx,dx,cx>
else ;not DEBUG
	cCall	N_CachePara,<bx,dx,cx>
endif ;DEBUG

;	 fAbs = FAbsPap(doc, &vpapFetch);
	cCall	FAbsPap,<>
ifdef DEBUG
	or	ah,ah
	je	FL053
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFormatn
	mov	bx,1051
	cCall	AssertProcForNative, <ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FL053:
endif ;DEBUG
	mov	bptr ([fInTable+1]),al

;	 if (FInTableVPapFetch(doc, cp))
;		 {
;		 /* in the interest of speed, steal some of GetTableCell's job */
;		 FreezeHp();	/* make sure we can use pwwd */
;		 CacheTc(ww, doc, cp, fFalse, fFalse);
;		 dxa = DxaFromDxp(pwwd, vtcc.xpDrRight - vtcc.xpDrLeft);
;		 if (dxa == 0)
;			 dxa = xaRightMaxSci;
;		 MeltHp();
;		 }
;#define FInTableVPapFetch(doc, cp)  (vpapFetch.fInTable && FInTableDocCp(doc, cp))
	cmp	[vpapFetch.fInTablePap],fFalse
FL057:
	mov	ax,[dxa]
	je	FL06
ifdef DEBUG
	cCall	S_FInTableDocCp,<[doc],[SEG_cp],[OFF_cp]>
else ;not DEBUG
	cCall	N_FInTableDocCp,<[doc],[SEG_cp],[OFF_cp]>
endif ;DEBUG
	or	ax,ax
	je	FL057
ifdef	DEBUG
	call	LN_FreezeHp
endif ;DEBUG
        xor     ax,ax
	cCall	CacheTc,<si,[doc],[SEG_cp],[OFF_cp],ax,ax>
	mov	ax,[vtcc.xpDrRightTcc]
	sub	ax,[vtcc.xpDrLeftTcc]
	;Begin in-line DxaFromDxp
;	return NMultDiv(dxp, dxaInch, vfti.dxpInch);
	mov	dx,dxaInch
	mov	bx,[vfti.dxpInchFti]
	call	LN_NMultDiv
	;End in-line DxaFromDxp
	or	ax,ax
	jne	FL058
	mov	ax,xaRightMaxSciConst
FL058:
ifdef	DEBUG
	call	LN_MeltHp
endif ;DEBUG
	jmp	short FL07

;	 else if (vpapFetch.dxaWidth != 0 && fAbs)
;		 dxa = vpapFetch.dxaWidth;
FL06:
	mov	cx,[vpapFetch.dxaWidthPap]
	jcxz	FL07
	cmp	bptr ([fInTable+1]),fFalse
	je	FL07
	xchg	ax,cx

;	 vfli.dxa = dxa;
FL07:
	mov	[dxa],ax
	mov	[vfli.dxaFli],ax

;	 vfli.fError = 0;
;	 fcm = fcmChars + fcmProps;
;	 if (!pwwd->fOutline || pwwd->fShowF)
;		 fcm += fcmParseCaps;
;/* rest of format loads up cache with current data */
	; si = ww, di = pwwd
	xor	ax,ax
	mov	[vfli.fErrorFli],al
	mov	bx,fcmChars+fcmProps+fcmParseCaps
	test	[di.fOutlineWwd],maskfOutlineWwd
	jz	FL09
	test	[di.fShowFWwd],maskfShowFWwd
	jnz	FL09
	errnz	<fcmParseCaps - 1>
	dec	bx
FL09:
	mov	[fcm],bx

;	 vfli.cpMin = cp;
;	 tlc = 0;
;	 ffs.ifldError = ifldNil;
;	 ffs.ifldWrap = ifldNil;
;	 fStopFormat = fFalse;
	; ax = 0, si = ww, di = pwwd
	;Assembler note: the following four lines have been moved from
	;the beginning of FormatLineDxa
;	dyyAscent.wlAll = 0;
;	dyyDescent.wlAll = 0;
;	xpJust = 0;
;	dichSpace3 = 0;
	push	ds
	pop	es
	lea	di,[xpJust]
	errnz	<OFFBP_dypAscent - OFFBP_xpJust - 2>
	errnz	<OFFBP_dytAscent - OFFBP_dypAscent - 2>
	errnz	<OFFBP_dypDescent - OFFBP_dytAscent - 2>
	errnz	<OFFBP_dytDescent - OFFBP_dypDescent - 2>
	stosw
	stosw
	stosw
	stosw
	stosw
	mov	[fStopFormat],ax     ;high byte is dichSpace3
	mov	si,[OFF_cp]
	mov	[vfli.LO_cpMinFli],si
	mov	di,[SEG_cp]
	mov	[vfli.HI_cpMinFli],di
	;The tlc is initialized as part of jcTab
	errnz	<fFalse>
	errnz	<ifldNil - (-1)>
	dec	ax
	mov	[ffs.ifldErrorFfs],ax
	mov	[ffs.ifldWrapFfs],ax

;	if (!FInCa(doc, cp, &caSect))
;		CacheSect(doc, cp);
	mov	bx,[doc]
	;LN_CacheSect performs
	;if (!FInCa(bx, di:si, &caSect)) CacheSect(bx, di:si);
	;ax, bx, cx, dx are altered.
	call	LN_CacheSect

;LRestartLine:
;/* we retart from here if an error in a formula (or other format group field)
;   forces a reformat */
;	 cpCurPara = cp;
;	 cpNext = cp;
LRestartLine:
	mov	ax,[OFF_cp]
	mov	[OFF_cpCurPara], ax
	mov	[OFF_cpNext], ax
	mov	ax,[SEG_cp]
	mov	[SEG_cpCurPara],ax
	mov	[SEG_cpNext], ax

;/* initialize run table */
;	 vifmaMac = 0;
;	 vbchrMac = 0;
;	 bchrChp = -1;
;	 dxtTabMinT = 1;
;	 fPassChSpec = fFalse;
;	 vfli.fAdjustForVisi = fFalse;
;	 vfli.fRMark = 0;
;     vfli.fGraphics = 0;
;	 vfli.fPicture = 0;
;	 vfli.fMetafile = fFalse;
;	 vpffs = &ffs;
;	 ffs.fFormatting = fFalse;
;	 fmal.fLiteral = fFalse;
;	 vfli.fVolatile = fFalse;
	xor	ax,ax
	mov	[vifmaMac],ax
	mov	[vbchrMac],ax
	errnz	<fFalse>
	mov	[vfli.fVolatileFli],al
	mov	[vfli.fAdjustForVisiFli],al
	errnz	<(fRMarkFli) - (fPictureFli)>
	errnz	<(fRMarkFli) - (fMetafileFli)>
	errnz	<(fRMarkFli) - (fGraphicsFli)>
	mov	[vfli.fRMarkFli],al
	errnz	<fFalse>
	mov	[ffs.fFormattingFfs],ax
	errnz	<fFalse>
	mov	[fmal.fLiteralFmal],ax
	mov	[fPassChSpec],ax
	dec	ax
	mov	[bchrChp],ax
	neg	ax
	mov	[dxtTabMinT],ax
	lea	ax,[ffs]
	mov	[vpffs],ax

;/* we restart from here if a vanished Eop appeared on the line. The format
;of paragraph at cpCurPara will be effective. */
;LRestartPara:
;	 vfli.fSplats = 0;
;	 ich = 0;
;	 ichSpace = ichNil;
;	 cchSpace = 0;
;	 fPrevSpace = fFalse;
; /* in lines ending with chEop etc. justification will be inhibited */
;	 fInhibitJust = fFalse;
;	 dyyAscentBreak.wlAll = 0;
;	 dyyDescentBreak.wlAll = 0;
;	 dyyAscentMac.wlAll = 0;
;	 dyyDescentMac.wlAll = 0;
;	 itbd = 0;
;	 cParen = 255;
;	 docNext = doc;
LRestartPara:
	errnz	<fFalse>
	xor	ax,ax
	mov	[vfli.fSplatsFli],al
	errnz	<(OFFBP_dypAscentMac) - (OFFBP_ichVar) - 2>
	errnz	<(OFFBP_dytAscentMac) - (OFFBP_dypAscentMac) - 2>
	errnz	<(OFFBP_dypDescentMac) - (OFFBP_dytAscentMac) - 2>
	errnz	<(OFFBP_dytDescentMac) - (OFFBP_dypDescentMac) - 2>
	push	ds
	pop	es
	lea	di,[ichVar]
	mov	cx,((OFFBP_dytDescentMac) - (OFFBP_ichVar))/2 + 1
	rep	stosw
	;high byte of chVar needs to be zero
	errnz	<(OFFBP_cchSpace) - (OFFBP_chVar) - 2>
	errnz	<(OFFBP_fPrevSpace) - (OFFBP_cchSpace) - 2>
	errnz	<(OFFBP_fInhibitJust) - (OFFBP_fPrevSpace) - 2>
	errnz	<(OFFBP_dypAscentBreak) - (OFFBP_fInhibitJust) - 2>
	errnz	<(OFFBP_dytAscentBreak) - (OFFBP_dypAscentBreak) - 2>
	errnz	<(OFFBP_dypDescentBreak) - (OFFBP_dytAscentBreak) - 2>
	errnz	<(OFFBP_dytDescentBreak) - (OFFBP_dypDescentBreak) - 2>
	errnz	<(OFFBP_itbd) - (OFFBP_dytDescentBreak) - 2>
	lea	di,[chVar]
	mov	cx,((OFFBP_itbd) - (OFFBP_chVar))/2 + 1
	rep	stosw
	dec	ax
	errnz	<(OFFBP_ichSpace) - (OFFBP_itbd) - 2>
	errnz	<ichNil - 255>
	;Assembler note: we will only use the low byte of ichSpace
	stosw
	errnz	<(OFFBP_cParen) - (OFFBP_ichSpace) - 2>
	stosw
	mov	ax,[doc]
	mov	[docNext],ax

;	Assert(cHpFreeze == 0);
;	Scribble(ispFormatLine,'F');
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	[cHpFreeze],0
	jz	FL10
	mov	ax,midFormatn
	mov	bx,1003
	cCall	AssertProcForNative,<ax,bx>
FL10:
	mov	ax,ispFormatLine
	mov	bx,046h
	cmp	[vdbs.grpfScribbleDbs],0
	jz	FL11
	cCall	ScribbleProc,<ax,bx>
FL11:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

; /* cache paragraph properties */
;	CachePara(doc, cpCurPara);
	;FLh7 performs CachePara(doc, cpCurPara); ax,bx,cx,dx are altered.
	call	FLh7

;	if (fInTable = vpapFetch.fInTable)
;	    {
;	    extern CP vcpFirstTablePara;
;	    fInTable = FInTableDocCp(doc, cpCurPara);
;	    cpFirstPara = vcpFirstTablePara;
;	    }
;	else
;	    cpFirstPara = caPara.cpFirst;
	mov	bx,dataoffset [caPara.cpFirstCa]
	errnz	<fFalse>
	mov	al,[vpapFetch.fInTablePap]
	or	al,al
	je	FL114
ifdef DEBUG
	cCall	S_FInTableDocCp,<[doc],[SEG_cpCurPara],[OFF_cpCurPara]>
else ;not DEBUG
	cCall	N_FInTableDocCp,<[doc],[SEG_cpCurPara],[OFF_cpCurPara]>
endif ;DEBUG
	mov	bx,dataoffset [vcpFirstTablePara]
	or	al,ah
FL114:
	mov	bptr ([fInTable]),al
	mov	ax,[bx]
	mov	[OFF_cpFirstPara],ax
	mov	ax,[bx+2]
	mov	[SEG_cpFirstPara],ax

;	vfli.grpfBrc = 0;
;	itbdMac = vpapFetch.itbdMac;
;	if (vpapFetch.itbdMac) vfli.fBarTabs = fTrue;
	mov	ax,[vpapFetch.itbdMacPap]
	mov	[itbdMac],ax
	xchg	ax,cx
	xor	ax,ax
	jcxz	FL12
	errnz	<(fBarTabsFli) - (grpfBrcFli) - 1>
	mov	ah,fTrue
FL12:
	mov	[vfli.grpfBrcFli],ax

;	 /* this can be done quite quickly in hand native */
;	 if ((vpapFetch.brcTop != brcNone || vpapFetch.brcLeft != brcNone
;			 || vpapFetch.brcBottom != brcNone || vpapFetch.brcRight != brcNone
;			 || vpapFetch.brcBetween != brcNone || vpapFetch.brcBar != brcNone)
;		 && !vfli.fOutline)
;		 FormatBorders(doc, cp, cpCurPara, cpFirstPara);
	xor	ax,ax
	cmp	[vfli.fOutlineFli],al
	jne	FL13
	errnz	<(brcLeftPap) - (brcTopPap) - 2>
	errnz	<(brcBottomPap) - (brcLeftPap) - 2>
	errnz	<(brcRightPap) - (brcBottomPap) - 2>
	errnz	<(brcBetweenPap) - (brcRightPap) - 2>
	errnz	<(brcBarPap) - (brcBetweenPap) - 2>
	mov	di,dataoffset [vpapFetch.brcTopPap]
	push	ds
	pop	es
	mov	cx,((brcBarPap) - (brcTopPap)) / 2 + 1
	errnz	<brcNone>
;	xor	ax,ax	    ;already done above
	repe	scasw
	je	FL13
	push	[doc]
	push	[SEG_cp]
	push	[OFF_cp]
	push	[SEG_cpCurPara]
	push	[OFF_cpCurPara]
	push	[SEG_cpFirstPara]
	push	[OFF_cpFirstPara]
	cCall	FormatBorders,<>

;#ifdef DEBUG
;	 pdodT = PdodDoc(doc);
;	 if (pdodT->fShort && vfli.fOutline)
;		 {
;		 Assert(fFalse);
;		 vfli.fOutline = fFalse;
;		 }
;#endif
FL13:
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	bx,[doc]
	shl	bx,1
	mov	bx,[bx.mpdochdod]
	mov	bx,[bx]
	cmp	[bx.fShortDod],fFalse
	je	FL14
	cmp	[vfli.fOutlineFli],fFalse
	je	FL14
	mov	ax,midFormatn
	mov	bx,1004
	cCall	AssertProcForNative,<ax,bx>
FL14:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

;	xaRight = dxa - vpapFetch.dxaRight;
;	xaLeft = vpapFetch.dxaLeft;
	mov	ax,[dxa]
	sub	ax,[vpapFetch.dxaRightPap]
	mov	[xaRight],ax
	mov	ax,[vpapFetch.dxaLeftPap]
	mov	[xaLeft],ax

;	dypBefore = 0;
	mov	[dypBefore],0

;	 if (!vfli.fOutline || fInTable || PdodDoc(doc)->hplcpad == hNil)
;		 {
	xor	ax,ax
	errnz	<fFalse>
	cmp	[vfli.fOutlineFli],al
	je	FL145
	cmp	bptr ([fInTable]),al
	jne	FL145
	mov	bx,[doc]
	;***Begin in line PdodDoc
	shl	bx,1
	mov	bx,[bx.mpdochdod]
	mov	bx,[bx]
	;***End in line PdodDoc
	errnz	<hNil>
	cmp	[bx.hplcpadDod],ax
	jne	FL16
FL145:

;		 jc = vpapFetch.jc;
;		 if (cpCurPara - cpFirstPara <= 1)
;			 {
	mov	al,[vpapFetch.jcPap]
	mov	bptr ([jcVar]),al
	mov	ax,[OFF_cpCurPara]
	mov	dx,[SEG_cpCurPara]
	mov	bx,[OFF_cpFirstPara]
	mov	cx,[SEG_cpFirstPara]
	sub	ax,bx
	sbb	dx,cx
	cmp	ax,2
	sbb	dx,0
	jge	LNotFirstLine

;/* special case for leading page break char in paragraph: first line
;properties are still maintained */
;			 if (cpCurPara != cpFirstPara &&
;				 (ch = ChFetch(doc, cpFirstPara, fcmChars))
;                                != chSect && ch != chColumnBreak)
;                               goto LNotFirstLine;
	or	ax,ax
	je	FL15
	mov	ax,fcmChars
	cCall	ChFetch,<[doc],cx,bx,ax>
	cmp	al,chSect
        je      FL15
        cmp     al,chColumnBreak
	jne	LNotFirstLine

;			 dypBefore = DypFromDya(vpapFetch.dyaBefore);
FL15:
;	FLi1 performs DypFromDya(ax)
;	ax, bx, cx, dx are altered.
	mov	ax,[vpapFetch.dyaBeforePap]
	call	FLi1

;/* leave room for the boxes */
;			 if (vfli.fTopEnable)
;				 {
;				 vfli.fTop = fTrue;
;				 dypBefore += vfli.dypBrcTop;
;				 }
	test	[vfli.fTopEnableFli],maskFTopEnableFli
	je	FL155
	or	[vfli.fTopFli],maskFTopFli
	add	ax,[vfli.dypBrcTopFli]
FL155:
	mov	[dypBefore],ax

;/* note: we permit xaLeft to become negative */
;			 xaLeft += vpapFetch.dxaLeft1;
	mov	ax,[vpapFetch.dxaLeft1Pap]
	add	[xaLeft],ax

;			 }
;		 }
;	 else
	jmp	short LNotFirstLine
FL16:

;		 {
;#ifdef DEBUG
;		 /* create this local pdod
;		 to avoid native compiler assert */
;		 struct DOD *pdodT = PdodDoc(doc);
;		 Assert(!pdodT->fShort);
;#endif /* DEBUG */
ifdef DEBUG
;	/* Assert (!PdodDoc(doc)->fShort) with a call so
;	as not to mess up short jumps */
	call	FLi5
endif ;/* DEBUG */

;		 vfli.grpfBrc = 0;
;		 OutlineProps(&xaLeft, &xaRight, &dypBefore);
;		 itbdMac = 0;
;		 jc = jcLeft;
;		 }
	mov	[vfli.grpfBrcFli],0
	lea	ax,[xaLeft]
	push	ax
	lea	ax,[xaRight]
	push	ax
	lea	ax,[dypBefore]
	push	ax
	cCall	OutlineProps,<>
	xor	ax,ax
	mov	[itbdMac],ax
	errnz	<jcLeft - 0>
	mov	bptr ([jcVar]),al

;LNotFirstLine:
;/* now do necessary checks on xaRight and xaLeft */
;	 if (xaRight > xaRightMaxSci) xaRight = xaRightMaxSci;
;	 if (xaLeft > xaRightMaxSci) xaLeft = xaRightMaxSci;
;	 if (xaRight < xaLeft) xaRight = xaLeft;
FL18:
LNotFirstLine:
	mov	cx,xaRightMaxSciConst
	mov	ax,[xaRight]
	mov	bx,[xaLeft]
	cmp	ax,cx
	jle	FL19
	mov	ax,cx
FL19:
	cmp	bx,cx
	jle	FL195
	mov	bx,cx
FL195:
	cmp	ax,bx
	jge	FL20
	mov	ax,bx
FL20:
	mov	[xaRight],ax
	mov	[xaLeft],bx

;/* determine:
;	 xt, xp, vfli.xpLeft from xaLeft
;	 xtRight from xaRight
;*/
;/* make sure that these long scales work on signed xaLeft and xt! */
;	 xt = XtFromXa(xaLeft);
;	 xtRight = XtFromXa(xaRight);
	; ax = xaRight, bx = xaLeft
	push	bx	;save xaLeft
	push	bx	;save xaLeft
;	LN_XtFromXa performs XtFromXa(ax)
;	ax, bx, cx, dx are altered.
	call	LN_XtFromXa
	mov	[xtRight],ax
	pop	ax	;restore xaLeft
;	LN_XtFromXa performs XtFromXa(ax)
;	ax, bx, cx, dx are altered.
	call	LN_XtFromXa
	mov	[xtVar],ax

;	 vfli.xpLeft = xp = NMultDiv(xaLeft, vfti.dxpInch, dxaInch);
;	FLi3 performs NMultDiv(ax, dx, dxaInch)
;	ax, bx, cx, dx are altered.
	pop	ax	;restore xaLeft
	mov	dx,[vfti.dxpInchFti]
	call	FLi3
	mov	[xpVar],ax
	mov	[vfli.xpLeftFli],ax

;/* xt/xp is different iff !fPrint && fFormatAsPrint */
;	 fmal.pftiDxt =
;	 pftiDxt = (fDxt = !vfli.fPrint && vfli.fFormatAsPrint) ?
;		 &vftiDxt : &vfti;
	mov	ax,dataoffset [vfti]
	;Assembler note:  See the comment just before mpchilbl for an
	;explanation of this code to set bits in [fDxt].
	mov	bx,00040h
	cmp	[vfli.fPrintFli],fFalse
	jnz	FL201
	cmp	[vfli.fFormatAsPrintFli],fFalse
	jz	FL201
	mov	ax,dataoffset [vftiDxt]
	mov	bl,0C0h
FL201:
	mov	[pftiDxt],ax
	mov	[fmal.pftiDxtFmal],ax
	mov	al,[vitr.chListItr]
	cmp	al,','
	je	FL202
	cmp	al,';'
	je	FL202
	xor	bl,041h
FL202:
	mov	[fDxt],bx

;/* initialize other format variables */
;	 vfli.chBreak = chNil;
;	 vfli.ichSpace1 = 0;
;	 vfli.ichSpace2 = ichNil;
;	 vfli.ichSpace3 = ichNil;
;
;	 goto LFetch;
	xor	ax,ax
	errnz	<jcLeft - 0>
	cwd
	mov	[vfli.ichSpace1Fli],al
	dec	ax
	errnz	<ichNil - 255>
	errnz	<(ichSpace3Fli) - (ichSpace2Fli) - 1>
	mov	wptr ([vfli.ichSpace2Fli]),ax
	errnz	<chNil - (-1)>
	mov	[vfli.chBreakFli],ax

;/* special initialization for decimal table entries */
;	 if (fInTable && vpapFetch.itbdMac == 1)
;		 {
	cmp	bptr ([fInTable]),fFalse
	je	FL203
	cmp	[vpapFetch.itbdMacPap],1
	jne	FL203

;		 jcTab = ((struct TBD *)vpapFetch.rgtbd + itbd)->jc;
	mov	bx,[itbd]
	mov	al,[bx.vpapFetch.rgtbdPap]
	errnz	<maskJcTbd - 007H>
	and	al,maskJcTbd

;		 if (jcTab == jcDecimal)
;			 {
	cmp	al,jcDecimal
	jne	FL203

;			 ichTab = -1;
	mov	[ichTab],-1

;			 xtTabStart = xt;
	mov	ax,[xtVar]
	mov	[xtTabStart],ax

;			 xpTabStart = xp;
	mov	ax,[xpVar]
	mov	[xpTabStart],ax

;			 xtTabStop = XtFromXa(vpapFetch.rgdxaTab[0]);
	mov	ax,[vpapFetch.rgdxaTabPap]
;	LN_XtFromXa performs XtFromXa(ax)
;	ax, bx, cx, dx are altered.
	call	LN_XtFromXa
	mov	[xtTabStop],ax

;			 jcTab = jcDecTable;
	mov	dx,jcDecTable

;			 }
;		 else
;			 jcTab = jcLeft;
;		 }
;	 else
;		 jcTab = jcLeft;

FL203:
	mov	wptr ([jcTab]),dx	;initializes [tlc] also.
	jmp	LFetch

; /* main loop */
; /* loop variables:
;	xp	current position, incremented at LHaveDxp
;	xt	same as xp in xt units.
;	ich	next char will be stored at rgch[ich], rgdxp[ich]
;	pch
;		points to next character
;	fPrevSpace
;		true iff prev char was space; means "we have already seen
;		the start of the current run of white space and have acted
;		on it by remembering its position for justification"
;	pchLim	ends current run. If different from vhpchFetch + vccpFetch
;		cpNext, docNext must be set.
;	cpNext	if not cpNil, the cp to be read when pch reaches pchLim.

;	xpJust	is the place which will be justified to xtRight.

;	cchSpace	number of spaces on ichSpace list
;	ichSpace	start of the list of spaces to be justified (except
;		for those trailing the line.) Spaces appear in reverse ich
;		order. List is terminated by ichNil.

; Current break opportunity is stored in vfli:
;	chBreak
;	cpMac
;	ichMac
;	xpRight
;	bchrBreak	(stored in local)
; */
; /* note that the loop is organized so that all code that needs to be executed
; for every character in the line appears in the front for easy native coding.
; */
;	for (;;)
;		{

;**************************************************************************
;****************   HIGH-FREQUENCY CODE  **********************************
;**************************************************************************
;*** The high-frequency code is organized into four sections.  The middle
;*** two sections contain the instructions that are actually executed, per-
;*** character, in the most common cases.  Surrounding this are two
;*** sections of exception conditions, which are taken out of line so
;*** that the most common case can execute in-line, without taking any
;*** jumps except the one back to the top of the loop
;
;**************************************************************************
;*************	SECTION 1: CODE RELOCATED BEFORE THE HIGH-FREQUENCY CODE
;**************************************************************************

; relocated code from switch(ch)
; we have a non-default switch (ch) case.  Store our register variables
; and dispatch to the appropriate switch case handler.
LNonDefaultCh:
	; di = pchFli, bh = 0
	mov	si,[pdxpFli]
	mov	bl,dh
	and	bl,03Eh
	jne	FL204
	errnz	<iWLChComma - 30>
	mov	bl,32	    ;using iWLChComma+2 here generates a nop
FL204:
ifdef DEBUG
	cmp	bx,iWLChMax+2
	jae	FL205
	or	bx,bx
	jne	FL206
FL205:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFormatn
	mov	bx,1006
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FL206:
endif ;DEBUG
	jmp	wptr (cs:[bx+rglbl-2])	;assume si = pdxpFli, di = pchFli


FL21:
	push	ss
	pop	ds
	mov	[OFF_hpch],si
	mov	bptr ([fPrevSpace]),dh
	mov	bptr ([chVar]),al
	mov	[dxp],bx
	xchg	ax,bx
	mov	dh,bptr (cs:[bx])
	mov	[xtVar],cx
	;See OverhangChr comment just below for an explanation of what is
	;going on here.
	add	cx,[mpXtXp]
	xchg	[xpVar],cx
	test	bptr ([fDxt]),080h
	je	FL212
	mov	[xpVar],cx
	shl	bx,1
	mov	ax,[bx.vftiDxt.rgdxpFti]
	add	ax,[vftiDxt.dxpExpandedFti]
FL212:
	mov	[dxt],ax
	mov	si,di
	sub	si,dataoffset [vfli.rgchFli]
	mov	[ichVar],si
	xor	bx,bx
	mov	bl,dl
	jmp	wptr (cs:[bx.LExitInnerLoop])	  ; assumes di = pchFli

LExitInnerLoop:
	dw	offset LNewRun
	dw	offset LNonDefaultCh
	dw	offset LBreak


LPChar:
	mov	dh,008h
	db	03Dh	;turns next "mov dh,immediate" into "cmp ax,immediate"
LTopOfLoop:
	mov	dh,000h
	db	03Dh	;turns next "mov dh,immediate" into "cmp ax,immediate"
LHaveCh:
	mov	dh,004h
	db	03Dh	;turns next "mov dh,immediate" into "cmp ax,immediate"
LHaveDxp:
	mov	dh,006h
	db	03Dh	;turns next "mov dh,immediate" into "cmp ax,immediate"
LDef:
	mov	dh,002h
	;LN_SetEsSiFetch takes sets es:si according to hpch.
	;ax, bx, cx, si, es are altered.
	call	LN_SetEsSiFetch
	mov	ax,[chVar]
ifdef DEBUG
;	Assert ( ch & 0xFF00 == 0) with a call so not to mess up short jumps.
	call	FLk0
endif ;DEBUG
	;Because of the OverhangChr optimization, !fDxt no longer means
	;xp == xt.  However, since the difference can change only in
	;OverhangChr, the amount added in this loop to xp will be
	;the same as that added to xt.	We save xp-xt here and add it back
	;into xp later so that xp will be correctly maintained.  If
	;fDxt we later overwrite xp so no harm done.
	mov	cx,[xtVar]
	mov	bx,[xpVar]
	sub	bx,cx
	mov	[mpXtXp],bx
	mov	bx,[ichVar]
	lea	di,[bx.vfli.rgchFli]
	lea	bx,[bx+di.rgdxpFli-rgchFli]
	mov	[pdxpFli],bx
	xor	bx,bx
	mov	bl,dh
	mov	dl,bptr ([fDxt])
	or	dl,dl
	jns	FL214
	add	bl,(offset LEnterInnerLoopDxt)-(offset LEnterInnerLoopNoDxt)
FL214:
	push	wptr (cs:[bx.LEnterInnerLoop])
	mov	bx,[dxp]
	; ax = ch, bx = dxt, cx = xt,
	; ds:si = qch, es:di = pchFli = &vfli.rgch[ich]
	cmp	dh,006h
	jne	FL216
	xchg	ax,bx	;have to swap if LHaveDxp
	inc	di	;pchFli = vfli.rgch[ich+1] if LHaveDxp
	test	bptr ([fDxt]),080h
	je	FL216
	;Have to perform xt += dxt here if LHaveDxp && fDxt
	add	cx,[dxt]
FL216:
	mov	dh,bptr ([fPrevSpace])
	push	es
	pop	ds
	push	ss
	pop	es
	db	0C3h	    ;near ret (ret generates far ret opcode here)

LEnterInnerLoop:
LEnterInnerLoopNoDxt:
	dw	offset LNoDxtTopOfLoop
	dw	offset LNoDxtDef
	dw	offset LNoDxtHaveCh
	dw	offset LNoDxtHaveDxp
	dw	offset LNoDxtPChar
LEnterInnerLoopDxt:
	dw	offset LDxtTopOfLoop
	dw	offset LDxtDef
	dw	offset LDxtHaveCh
	dw	offset LDxtHaveDxp
	dw	offset LDxtPChar

Ltemp021:
	jmp	FL254

FL218:
	cmp	al,chSpace
	jne	Ltemp021

; /* space: count and enter in list */
;		case chSpace: /* == 32 */
;			if (vifmaMac != 0)    /* REVIEW: Opus want this? */
;				{
;				xp -= dxp;
;				xt -= dxt;
;				ich--;
;				ch = chNonBreakSpace; goto LHaveCh;
;				}
;			ichM1 = ich - 1;
LChSpaceInnerLoop:
	; ax = ch, bx = dxp, cx = xt,
	; dh bit 0 = fPrevSpace, dh bit 1 = fHeightPending, dl = fDxt
	; ds:si = qch, es:di = pchFli
	cmp	ss:[vifmaMac],0
	jne	Ltemp021
	push	ds	;save seg from vhpchFetch
	push	ax	;save ch
	push	cx	;save xt
	mov	ax,ss
	mov	ds,ax

;			if (!fPrevSpace)
; /* first in a series of spaces */
;				{
;				xpJust = xp - dxp;
;				xtJust = xt - dxt;
;				vfli.ichSpace3 = ichM1;
;				}
;			fPrevSpace = fTrue;
	mov	ax,di
	sub	ax,dataoffset [vfli.rgchFli + 1]
	test	dh,maskFPrevSpace
	jne	FL22
	push	cx
	sub	cx,[vftiDxt.rgdxpFti+chSpace+chSpace]
	sub	cx,[vftiDxt.dxpExpandedFti]
	mov	[xtJust],cx
	pop	cx
FL22:
	add	cx,[mpXtXp]
	or	dl,dl	    ;fDxt
	jns	FL225
	mov	cx,[xpVar]
FL225:
	test	dh,maskFPrevSpace
	jne	FL23
	push	cx
	sub	cx,bx
	mov	[xpJust],cx
	pop	cx
	mov	[vfli.ichSpace3Fli],al
FL23:
	or	dh,maskFPrevSpace

;			 if (FVisiSpacesFli)
;			    vfli.rgch [ichM1] = chVisSpace;
	; ax = ich - 1, bx = dxp, cx = xp,
	; dh bit 0 = fPrevSpace, dh bit 1 = fHeightPending, dl = fDxt
	; di = pchFli
	test	bptr ([vfli.grpfvisiFli]),maskfvisiSpacesGrpfvisi OR maskfvisiShowAllGrpfvisi
	jz	FL24
	mov	bptr ([di-1]),chVisSpace
FL24:

; /* spaces are entered in a list iff jcBoth */
;			if (jc == jcBoth)
;				if (ichM1 == vfli.ichSpace1)
; /* except leading spaces in the line are ignored */
;					vfli.ichSpace1++;
;				else
;					{
;					cchSpace++;
;					vfli.rgch[ichM1] = ichSpace;
;					ichSpace = ichM1;
;					}
	; ax = ich - 1, bx = dxp, cx = xp,
	; dh bit 0 = fPrevSpace, dh bit 1 = fHeightPending, dl = fDxt
	; di = pchFli
	push	ax	;save ich - 1
	cmp	bptr ([jcVar]),jcBoth
	jne	LBreakOppRInnerLoop
	cmp	[vfli.ichSpace1Fli],al
	jne	FL25
	inc	[vfli.ichSpace1Fli]
	jmp	short LBreakOppRInnerLoop
FL25:
	inc	[cchSpace]
	xchg	[ichSpace],ax
	mov	bptr ([di-1]),al

; LBreakOppR:
; /* set up break opportunity to the right of the char */
; /* ch is current, pch, xp, ich all incremented */
;			vfli.chBreak = ch;
;			vfli.cpMac = (hpch - vhpchFetch) + vcpFetch;
;			vfli.xpRight = xp;
;			vfli.ichMac = ich;
;			bchrBreak = vbchrMac;
;			dyyAscentBreak = dyyAscentMac;
;			dyyDescentBreak = dyyDescentMac;
;			continue;
LBreakOppRInnerLoop:
	pop	ax	;restore ich - 1
	; ax = ich - 1, bx = dxp, cx = xp,
	; dh bit 0 = fPrevSpace, dh bit 1 = fHeightPending, dl = fDxt
	; di = pchFli
	inc	ax
	mov	[vfli.ichMacFli],al
	push	dx	;save fPrevSpace, fHeightPending, fDxt
	mov	[vfli.chBreakFli],chSpace
	mov	ax,si
	sub	ax,wlo ([vhpchFetch])
	cwd
	add	ax,wlo ([vcpFetch])
	adc	dx,whi ([vcpFetch])
	mov	[vfli.LO_cpMacFli],ax
	mov	[vfli.HI_cpMacFli],dx
	mov	[vfli.xpRightFli],cx
	mov	ax,[vbchrMac]
	mov	[bchrBreak],ax
	push	si	;save low qch
	push	di	;save pchFli
	errnz	<(OFFBP_dytAscentMac) - (OFFBP_dypAscentMac) - 2>
	errnz	<(OFFBP_dypDescentMac) - (OFFBP_dytAscentMac) - 2>
	errnz	<(OFFBP_dytDescentMac) - (OFFBP_dypDescentMac) - 2>
	errnz	<(OFFBP_dytAscentBreak) - (OFFBP_dypAscentBreak) - 2>
	errnz	<(OFFBP_dypDescentBreak) - (OFFBP_dytAscentBreak) - 2>
	errnz	<(OFFBP_dytDescentBreak) - (OFFBP_dypDescentBreak) - 2>
	lea	si,[dypAscentMac]
	lea	di,[dypAscentBreak]
	movsw
	movsw
	movsw
	movsw
	pop	di	;restore pchFli
	pop	si	;restore low qch
	pop	dx	;restore fPrevSpace, fHeightPending
	pop	cx	;restore xt
	pop	ax	;restore ch
	pop	ds	;restore seg from vhpchFetch
	or	dl,dl
	js	LDxtTopOfLoop

;**************************************************************************
;*************	SECTION 2: THE HIGH-FREQUENCY CODE - fDxt == fFalse
;**************************************************************************

;LTopOfLoop:
;		if (hpch == hpchLim) goto LNewRun;
;		ch = *hpch++;
LNoDxtTopOfLoop:
	; ax = ch, bx = dxp, cx = xt = xp,
	; dh bit 0 = fPrevSpace, dh bit 1 = fHeightPending,
	; ds:si = qch, es:di = pchFli
	cmp	si,[OFF_hpchLim]	    ;clocks = 6     total = 6
	je	FL257			    ;clocks = 3     total = 9
	lodsb				    ;clocks = 5     total = 14

;LHaveCh:
;		vfli.rgch[ichVar] = ch;
	; ax = ch, bx = dxp, cx = xt = xp,
	; dh bit 0 = fPrevSpace, dh bit 1 = fHeightPending
	; ds:si = qch, es:di = pchFli
LNoDxtHaveCh:
	stosb				    ;clocks = 3     total = 17

; /* calculate the dxp and dxt width of ch */
;		dxt = dxp = DxpFromCh(ch, &vfti);
	xchg	ax,bx			    ;clocks = 3     total = 20
	shl	bx,1			    ;clocks = 2     total = 22
	mov	ax,ss:[bx.vfti.rgdxpFti]    ;clocks = 5     total = 27
	add	ax,ss:[vfti.dxpExpandedFti] ;clocks = 7     total = 34

;		if (fDxt)
;			dxt = DxpFromCh(ch, &vftiDxt);
	shr	bx,1			    ;clocks = 2     total = 36

;LHaveDxp:
;		xp += dxp;
;		xt += dxt;
LNoDxtHaveDxp:
	add	cx,ax			    ;clocks = 3     total = 39

;		vfli.rgdxp[ich++] = dxp;
; /* now we are at the right of the current character */
	; ax = dxp, bx = ch, cx = xt = xp,
	; dh bit 0 = fPrevSpace, dh bit 1 = fHeightPending,
	; ds:si = qch, es:di = pchFli
	xchg	di,[pdxpFli]		    ;clocks = 5     total = 44
	stosw				    ;clocks = 3     total = 47
	xchg	di,[pdxpFli]		    ;clocks = 5     total = 52

;		switch (ch)
;			{
;/* non-default characters are: 0,    9,  11, 12,  13 or 10,  14,  31,	32,
;			       Null,Tab,CRJ,Sect,Eop,	    SectJ,NRH,Space
;
;			       40,	 41,	    45,    92 or 6, 145-151
;			       LeftParen,RightParen,Hyphen,Formula, Publishing
;*/
	; ax = dxp, bx = ch, cx = xt = xp,
	; dh bit 0 = fPrevSpace, dh bit 1 = fHeightPending,
	; ds:si = qch, es:di = pchFli
	errnz	<offset mpchilbl>
	test	bptr (cs:[bx]),dl	    ;clocks = 6     total = 58
	xchg	ax,bx			    ;clocks = 3     total = 61
	jne	Ltemp022;jmp if not default ;clocks = 3     total = 64

;		default:
;			if (ch == vitr.chList)
;				goto LChComma;
; LDef:
;			if (xt <= xtRight)
;				{
LNoDxtDef:
	; ax = ch, bx = dxp, cx = xt = xp,
	; dh bit 0 = fPrevSpace, dh bit 1 = fHeightPending,
	; ds:si = qch, es:di = pchFli
	cmp	cx,[xtRight]		    ;clocks = 6     total = 70
	jg	FL258			    ;clocks = 3     total = 73

; LPChar:
; /* printing character that does not break the line */
;				fPrevSpace = fFalse;
;				if (!fHeightPending)
;					continue;
LNoDxtPChar:
	; ax = ch, bx = dxp, cx = xt = xp,
	; dh bit 0 = fPrevSpace, dh bit 1 = fHeightPending,
	; ds:si = qch, es:di = pchFli
	and	dh,maskFHeightPending	    ;clocks = 3     total = 76
	je	LNoDxtTopOfLoop 	    ;clocks = 10    total = 86
	jmp	FL28

;**************************************************************************
;*************	SECTION 3: CODE RELOCATED TO OUTSIDE HIGH FREQUENCY LOOPS
;**************************************************************************

Ltemp022:
	jmp	FL218

	; ax = ch, bx = dxp, cx = xt = xp,
	; dh bit 0 = fPrevSpace, dh bit 1 = fHeightPending,
	; ds:si = qch, es:di = pchFli
FL254:
	xchg	ax,bx
	test	bptr (cs:[bx]),03Eh
	xchg	ax,bx
	jne	FL256
	cmp	al,ss:[vitr.chListItr]
	je	FL256
	or	dl,dl
	jns	LNoDxtDef
	jmp	short LDxtDef
FL256:
	mov	dl,002h
	db	03Dh	;turns next "mov dl,immediate" into "cmp ax,immediate"
FL257:
	mov	dl,000h
	db	03Dh	;turns next "mov dl,immediate" into "cmp ax,immediate"
FL258:
	mov	dl,004h
	jmp	FL21


;**************************************************************************
;*************	SECTION 4: THE HIGH-FREQUENCY CODE - fDxt == fTrue
;**************************************************************************

;LTopOfLoop:
;		if (hpch == hpchLim) goto LNewRun;
;		ch = *hpch++;
LDxtTopOfLoop:
	; ax = ch, bx = dxp, cx = xt,
	; dh bit 0 = fPrevSpace, dh bit 1 = fHeightPending,
	; ds:si = qch, es:di = pchFli
	cmp	si,[OFF_hpchLim]	    ;clocks = 6     total = 6
	je	FL257			    ;clocks = 3     total = 9
	lodsb				    ;clocks = 5     total = 14

;LHaveCh:
;		vfli.rgch[ich] = ch;
	; ax = ch, bx = dxp, cx = xt,
	; dh bit 0 = fPrevSpace, dh bit 1 = fHeightPending
	; ds:si = qch, es:di = pchFli
LDxtHaveCh:
	stosb				    ;clocks = 3     total = 17

; /* calculate the dxp and dxt width of ch */
;		dxt = dxp = DxpFromCh(ch, &vfti);
	xchg	ax,bx			    ;clocks = 3     total = 20
	shl	bx,1			    ;clocks = 2     total = 22
	mov	ax,ss:[bx.vfti.rgdxpFti]    ;clocks = 5     total = 27
	add	ax,ss:[vfti.dxpExpandedFti] ;clocks = 7     total = 34

;		if (fDxt)
;			dxt = DxpFromCh(ch, &vftiDxt);
	add	cx,ss:[bx.vftiDxt.rgdxpFti]    ;clocks = 7     total = 41
	add	cx,ss:[vftiDxt.dxpExpandedFti] ;clocks = 7     total = 48
	shr	bx,1			    ;clocks = 2     total = 50

;LHaveDxp:
;		xp += dxp;
;		xt += dxt;
LDxtHaveDxp:
	add	[xpVar],ax		    ;clocks = 7     total = 57

;		vfli.rgdxp[ich++] = dxp;
; /* now we are at the right of the current character */
	; ax = dxp, bx = ch, cx = xt,
	; dh bit 0 = fPrevSpace, dh bit 1 = fHeightPending,
	; ds:si = qch, es:di = pchFli
	xchg	di,[pdxpFli]		    ;clocks = 5     total = 62
	stosw				    ;clocks = 3     total = 65
	xchg	di,[pdxpFli]		    ;clocks = 5     total = 70

;		switch (ch)
;			{
;/* non-default characters are: 0,    9,  11, 12,  13 or 10,  14,  31,	32,
;			       Null,Tab,CRJ,Sect,Eop,	    SectJ,NRH,Space
;
;			       40,	 41,	    45,    92 or 6
;			       LeftParen,RightParen,Hyphen,Formula
;*/
	; ax = dxp, bx = ch, cx = xt,
	; dh bit 0 = fPrevSpace, dh bit 1 = fHeightPending,
	; ds:si = qch, es:di = pchFli
	errnz	<offset mpchilbl>
	test	bptr (cs:[bx]),dl	    ;clocks = 6     total = 76
	xchg	ax,bx			    ;clocks = 3     total = 79
	jne	Ltemp022;jmp if not default ;clocks = 3     total = 82

;		default:
;			if (ch == vitr.chList)
;				goto LChComma;
; LDef:
;			if (xt <= xtRight)
;				{
LDxtDef:
	; ax = ch, bx = dxp, cx = xt,
	; dh bit 0 = fPrevSpace, dh bit 1 = fHeightPending,
	; ds:si = qch, es:di = pchFli
	cmp	cx,[xtRight]		    ;clocks = 6     total = 88
	jg	FL258			    ;clocks = 3     total = 91

; LPChar:
; /* printing character that does not break the line */
;				fPrevSpace = fFalse;
;				if (!fHeightPending)
;					continue;
LDxtPChar:
	; ax = ch, bx = dxp, cx = xt,
	; dh bit 0 = fPrevSpace, dh bit 1 = fHeightPending,
	; ds:si = qch, es:di = pchFli
	and	dh,maskFHeightPending	    ;clocks = 3     total = 94
	je	LDxtTopOfLoop		    ;clocks = 10    total = 104
;	jmp	short FL28  ; fall through

;**************************************************************************
;****************   END OF HIGH-FREQUENCY CODE	***************************
;**************************************************************************

; /* this is the end of the high frequency code. All code below is
;   executed only a few times per line */

; /* tabs come here eventually, but shouldn't change line height; when tabs do
;  come here, ch has been changed to zero (doesn't hurt for real ch=0, which
;  has zero width hence zero height) */
;				if (ch)
;					{
; /* update line height information */
;					fHeightPending = fFalse;
;					if (dypDescentMac < dypDescent)
;						dypDescentMac = dypDescent;
;					if (dypAscentMac < dypAscent)
;						dypAscentMac = dypAscent;
;					if (dytDescentMac < dytDescent)
;						dytDescentMac = dytDescent;
;					if (dytAscentMac < dytAscent)
;						dytAscentMac = dytAscent;
;					}
;				continue;
	; ax = ch, bx = dxp, cx = xt = xp,
	; dl bit 0 = fPrevSpace, dl bit 1 = EHeightPending, dh = fDxt
	; ds:si = qch, es:di = pchFli
FL28:
	; No need to store away ch, we're just looping around anyway
	or	ax,ax
	jz	FL29
	and	dh,NOT maskFHeightPending
	;FLg1 performs
	;dypDescentMac = max(dypDescentMac, dypDescent);
	;dypAscentMac = max(dypAscentMac, dypAscent);
	; and the same for the dytAscent/Descent
	;ax and cx are altered.
	push	cx
	call	FLg1
	pop	cx
FL29:
	xor	ax,ax	    ;must have ah zero when we get back in loop
	or	dl,dl
	js	LDxtTopOfLoop
	jmp	LNoDxtTopOfLoop
;				}
;			else
;				{


; LBreak:
; /* printable character with right edge beyond right margin */
;				vfli.dcpDepend =
;					(hpch - vhpchFetch) + vcpFetch - vfli.cpMac;
LBreak:
	;FLf8 performs dx:ax = vcpFetch + (hpch - vhpchFetch)
	;only ax and dx are altered.
	call	FLf8
	sub	ax,[vfli.LO_cpMacFli]
	mov	[vfli.dcpDependFli],ax

;				if (vfli.chBreak == chNil)
;					{
; /* there is no break opportunity to the left of xt */
; /* break to the right of the character if first ch on line, else to its left */

	cmp	[vfli.chBreakFli],chNil
	jnz	FL32

;					if (ich != 1)
;						{
;						struct CHR *pchrT = &(**vhgrpchr)[bchrPrev];
;						hpch--;
;						ich--;
;						if (pchrT->ich == ich && pchrT->chrm != chrmVanish)
;							vbchrMac = bchrPrev;
;						xp -= dxp;

	cmp	[ichVar],1
	je	FL30
	;FLf6 performs ich--; ax = (xt -= dxt); dx = (xp -= dxp);
	;only ax and dx are altered.
	call	FLf6
	dec	[OFF_hpch]
	;FLg9 performs bx = &(**vhgrpchr)[bchrPrev];
	;only ax, bx, dx are altered.
	call	FLg9
	errnz	<(chrmChr) - 0>
	errnz	<(ichChr) - 1>
	mov	ax,wptr ([bx.chrmChr])
	cmp	al,chrmVanish
	je	FL31
	cmp	ah,bptr ([ichVar])
	jne	FL31
	mov	ax,[bchrPrev]
	mov	[vbchrMac],ax
	jmp	short FL31

;						}
;					else if (ffs.fFormatting)
;						{
FL30:
	cmp	[ffs.fFormattingFfs],fFalse
	je	FL31

;						if ((hpch-vhpchFetch)+vcpFetch
;							+ 1 == CpLimField(doc,
;							ffs.ifldFormat))
	cCall	CpLimField,<[doc],[ffs.ifldFormatFfs]>
	mov	bx,[OFF_hpch]
	sub	bx,wlo [vhpchFetch]
	inc	bx
	xor	cx,cx
	add	bx,wlo [vcpFetch]
	adc	cx,whi [vcpFetch]
	sub	ax,bx
	sbb	dx,cx
	or	ax,dx

;						    goto LPChar;
	jne	FL31
	jmp	LPChar

;						}

FL31:
;					vfli.cpMac = (hpch - vhpchFetch) + vcpFetch;
;					vfli.ichMac = ich;
;					vfli.dcpDepend = 1;
;					vfli.xpRight = xp;
;					bchrBreak = vbchrMac;
;					goto LEnd;
	;FLf8 performs dx:ax = vcpFetch + (hpch - vhpchFetch)
	;only ax and dx are altered.
	call	FLf8
	mov	[vfli.dcpDependFli],1
	;FLg8 performs
	;vfli.cpMac = dx:ax;
	;vfli.xpRight = xp;
	;vfli.ichMac = ich;
	;bchrBreak = vbchrMac;
	;only ax, dx are altered.
	call	FLg8
	jmp	LEnd
;					}
FL32:

; /* otherwise break at the last opportunity. vfli was set up at the break
;    opportunity */
;				if (vfli.chBreak == chNonReqHyphen &&
;						!FVisiCondHyphensFli)

	cmp	[vfli.chBreakFli],chNonReqHyphen
	jne	FL33
	test	bptr ([vfli.grpfvisiFli]),maskfvisiCondHyphensGrpfvisi OR maskfvisiShowAllGrpfvisi
	jnz	FL33


;					{ /* Append the now-required hyphen to end of line */
;					vfli.ichMac++;
;/* WIN: Adjustment to assure vfli.ichSpace3 is a valid guide to
;   the underline termination point. */
;					vfli.ichSpace3 = WinMac( vfli.ichMac,
;								vfli.ichMac-1);
;					vfli.rgdxp[vfli.ichMac-1] = dxp = DxpFromCh(chHyphen, &vfti);
;					vfli.rgch[vfli.ichMac-1] = chHyphen;
;					vfli.xpRight += dxp;
;					xpJust = vfli.xpRight;
;					/* xtJust not needed at LEndJ */
; /* truncate grpchr before the vanished run */
;					bchrBreak = bchrNRH;

	;FLf9 performs ax = DxpFromCh(al,&vfti);
	;only ax and bx are altered.
;PAUSE
	mov	cl,chHyphen
	mov	ax,cx
	call	FLf9
	;FLg85 performs
	;bx = vfli.ichMac;
	;vfli.rgch[vfli.ichMac] = cl;
	;vfli.rgdxp[vfli.ichMac++] = dxp = ax;
	;vfli.xpRight += dxp;
	;only bx is altered.
	call	FLg85
	inc	bx
	mov	[vfli.ichSpace3Fli],bl
	mov	ax,[vfli.xpRightFli]
	mov	[xpJust],ax
	mov	ax,[bchrNRH]
	mov	[bchrBreak],ax

;					}
FL33:

;				if (vfli.chBreak == chTab) vfli.ichSpace3 = ichNil;
;				goto LEndJ;
;				}

	cmp	[vfli.chBreakFli],chTab
	jne	FL34
	mov	[vfli.ichSpace3Fli],ichNil
FL34:
	jmp	LEndJ

; ASSUME we never get here

; /* Non-required hyphen. Code here is motivated by the possibility of
; the NRH being turned into a line-terminating hyphen. Until that happens:
;	nothing stored in rgch, rgdxp.
;	room is assured but not reserved.
;	vanished crt is placed in grpcrt.
;	break opportunity is set up following the assumed hyphen.
; */
;		case chNonReqHyphen: /* == 31 */
LChNonReqHyphen:
	; si = pdxpFli, di = pchFli


;			{
;			if ((int)(xt + DxpFromCh(chHyphen, pftiDxt) - dxt) > xtRight)
;				goto LBreak;
	mov	al,chHyphen
	mov	bx,[pftiDxt]
	;FLg0 performs ax = DxpFromCh(al,bx);
	;only ax and bx are altered.
	call	FLg0
	sub	ax,[dxt]
	add	ax,[xtVar]
	cmp	ax,[xtRight]
	jle	Ltemp004
	jmp	LBreak
Ltemp004:

;			if (FVisiCondHyphensFli)
; /* visible. Treat like a hyphen/non break-hyphen combo.
; See conditional at end of LChrt */
;				{
;				chT = chVisNonReqHyphen;
;				chReal = '-';
;				goto LNonBreakHyphen;
;				}
	test	bptr ([vfli.grpfvisiFli]),maskfvisiCondHyphensGrpfvisi OR maskfvisiShowAllGrpfvisi
	jz	FL35
	mov	cx,'-' + (chVisNonReqHyphen SHL 8)
	jmp	LNonBreakHyphen     ; expects cl = chReal, ch = chT
				    ; si = pdxpFli, di = pchFli
FL35:

; /* undo main loop actions */
;			ich--;
;			xtJust = (xt -= dxt);
;			xpJust = (xp -= dxp);
;			OverhangChr(&xp, bchrPrev,ich);
;			bchrPrev = bchrNRH = vbchrMac;
	;FLf6 performs ich--; ax = (xt -= dxt); dx = (xp -= dxp);
	;only ax and dx are altered.
	call	FLf6
	mov	[xtJust],ax
	mov	[xpJust],dx
	;LN_OverhangChr performs: adjust previous chr for overhang
	;also performs bchrPrev = vbchrMac.
	;ax, bx, cx, dx are altered.
	call	LN_OverhangChr
	mov	ax,[vbchrMac]
	mov	[bchrNRH],ax

;			Debug(vdbs.fShakeHeap ? ShakeHeap() : 0);
ifdef DEBUG
	cmp	vdbs.fShakeHeapDbs,fFalse
	jz	FL36
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,1
	cCall	ShakeHeapSb,<ax>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FL36:
endif

;			if ((vbchrMac += cbCHRV) <= vbchrMax ||
;				FExpandGrpchr(cbCHRV))
	mov	bx,cbCHRV
;	FLf4 performs;
;	vbchrMac+=bx;
;	return ((vbchrMac > vbchrMax) ? FExpandGrpchr(bx) : fTrue);
;	FLf4 returns zero flag set if the routine failed,
;	otherwise returns zero flag reset.
	call	FLf4
	jz	FL37
;				{
;				(pchr = &(**vhgrpchr)[bchrNRH])->ich = ich;
;				pchr->chrm = chrmVanish;
;				((struct CHRV *)pchr)->dcp = 1;
;				}
;			goto LBreakOppR;
;			}
	mov	dx,[bchrNRH]
	mov	al,chrmVanish
	;FLh1 performs bx = &(**vhgrpchr)[dx];
	;[bx.chrmChr] = al; [bx.ichChr] = ich;
	;only ax, bx, dx are altered.
	call	FLh1
	mov	[bx.LO_dcpChrv],1
	mov	[bx.HI_dcpChrv],0
FL37:

; LBreakOppR:
; /* set up break opportunity to the right of the char */
; /* ch is current, pch, xp, ich all incremented */
;			vfli.chBreak = ch;
;			vfli.cpMac = (hpch - vhpchFetch) + vcpFetch;
;			vfli.xpRight = xp;
;			vfli.ichMac = ich;
;			bchrBreak = vbchrMac;
;			dyyAscentBreak = dyyAscentMac;
;			dyyDescentBreak = dyyDescentMac;
;			continue;
LBreakOppR:
	;FLf8 performs dx:ax = vcpFetch + (hpch - vhpchFetch)
	;only ax and dx are altered.
	call	FLf8
	;FLg4 performs
	;vfli.chBreak = chVar
	;vfli.cpMac = dx:ax;
	;vfli.xpRight = xp;
	;vfli.ichMac = ich;
	;bchrBreak = vbchrMac;
	;dyyAscentBreak = dyyAscentMac;
	;dyyDescentBreak = dyyDescentMac;
	;only ax, bx, dx are altered.
	call	FLg4
	jmp	LTopOfLoop

;		case chNonBreakSpace:	/* == 160 */
; /* display as visi space, otherwise as LDef ordinary character */
;			chReal = chSpace;
;			chT = chVisNonBreakSpace;
;			if (!FVisiSpacesFli)
;				chT = chReal;
;			goto LVisi;

LChNonBreakSpace:
	; si = pdxpFli, di = pchFli
	mov	cx,chSpace + (chVisNonBreakSpace SHL 8)
	test	bptr ([vfli.grpfvisiFli]),maskfvisiSpacesGrpfvisi OR maskfvisiShowAllGrpfvisi
	jnz	FL38
	mov	ch,cl
FL38:
	jmp	short LVisi	    ; expects cl = chReal, ch = chT
				    ; si = pdxpFli, di = pchFli

;		case chPubLDblQuote:
;		case chPubRDblQuote:
;		case chPubBullet:
;		case chPubEmDash:
;		case chPubEnDash:
;			ich--;
;			xp -= dxp;
;			xt -= dxt;
;			FormatChPubs( ch, &dxp, &dxt );
;			goto LChrt;
LChPubs:
	;FLf6 performs ich--; ax = (xt -= dxt); dx = (xp -= dxp);
	;only ax and dx are altered.
	call	FLf6
	push	[chVar]
	lea	ax,[dxp]
	push	ax
	lea	ax,[dxt]
	push	ax
	call	FormatChPubs
	jmp	LChrt

;		case chNonBreakHyphen: /* == 30 */
; /* display as hyphen, otherwise as LDef ordinary character */
;			chReal = '-';
;			chT = chNil;
;LNonBreakHyphen:	if (!FVisiCondHyphensFli)
;				chT = chReal;

LChNonBreakHyphen:
	; si = pdxpFli, di = pchFli
	mov	cx,chHyphen + (chNil SHL 8)
LNonBreakHyphen:
	test	bptr ([vfli.grpfvisiFli]),maskfvisiCondHyphensGrpfvisi OR maskfvisiShowAllGrpfvisi
	jnz	LVisi
	mov	ch,cl

; LVisi:		ich--;
;			xp -= dxp;
;			xt -= dxt;
;			if (chT == chNil)
;			    {
;			    vfli.fAdjustForVisi = fTrue;
;			    chT = chSpace;
;			    dxt = dxp = C_DxpFromCh('-',&vfti) << 1;
;			    }
;			else
;			    dxt = dxp = C_DxpFromCh(chT, &vfti);

LVisi:
	;FLf6 performs ich--; ax = (xt -= dxt); dx = (xp -= dxp);
	;only ax and dx are altered.
	call	FLf6
	cmp	ch,chNil
	jnz	FL39
	mov	[vfli.fAdjustForVisiFli],fTrue
	mov	ch,chSpace
	mov	al,'-'
	;FLf9 performs ax = DxpFromCh(al,&vfti);
	;only ax and bx are altered.
	call	FLf9
	shl	ax,1
	jmp	short FL40
FL39:
	mov	al,ch
	;FLf9 performs ax = DxpFromCh(al,&vfti);
	;only ax and bx are altered.
	call	FLf9
FL40:
	mov	[dxt],ax
	mov	[dxp],ax

;			if (fDxt)
;				dxt = DxpFromCh(chReal, &vftiDxt);
;			vfli.rgch[ich] = chT;
;			goto LChrt;
	test	bptr ([fDxt]),080h
	jz	FL41
	mov	bx,dataoffset [vftiDxt]
	mov	al,cl
	;FLg0 performs ax = DxpFromCh(al,bx);
	;only ax and bx are altered.
	call	FLg0
	mov	[dxt],ax
FL41:
ifdef DEBUG
	;Assert (di == &vfli.rgch[ich+1]);
	;Perform this assert with a call so as not to mess up short jumps
	call	FLj2
endif ;DEBUG
	mov	[di-1],ch
	jmp	LChrt

;		case chHyphen: /* == 45 */
;LHyphen:
;			if (xt > xtRight)
;				goto LBreak;
LChHyphen:
	; si = pdxpFli, di = pchFli
	mov	ax,[xtVar]
	cmp	[xtRight],ax
	jge	LTemp006
	jmp	LBreak
Ltemp006:

; LHyphen1:		xpJust = xp;
;			xtJust = xt;
;/* WIN: Adjustment to assure vfli.ichSpace3 is a valid guide to
;   the underline termination point. */
;			vfli.ichSpace3 = WinMac( ich, ich - 1);
;			dypDescentMac = max(dypDescentMac, dypDescent);
;			dypAscentMac = max(dypAscentMac, dypAscent);
;			dytDescentMac = max(dytDescentMac, dytDescent);
;			dytAscentMac = max(dytAscentMac, dytAscent);
;			fPrevSpace = fFalse;
;			goto LBreakOppR;
LHyphen1:
	;FLh5 performs
	;xpJust = xp;
	;xtJust = xt;
	;vfli.ichSpace3 = ich;
	;only ax and dx are altered.
	call	FLh5
	;FLg1 performs
	;dypDescentMac = max(dypDescentMac, dypDescent);
	;dypAscentMac = max(dypAscentMac, dypAscent);
	; and the same for the dytAscent/Descent
	;ax and cx are altered.
	call	FLg1
	and	bptr ([fPrevSpace]),NOT maskFPrevSpace
	jmp	LBreakOppR


; /* Null characters are made visible in visible mode */
;		case 0: /* null */
; /* REVIEW: is this the right choice of pref flag to trigger this? */
;			if (FVisiSpacesFli)
;				{
;				xp -= dxp;
;				xt -= dxt;
;				ich--;
;				ch = 0377; goto LHaveCh;
;				}
;			goto LDef;
LChNull:
	; si = pdxpFli, di = pchFli
	xor	dx,dx
	test	bptr ([vfli.grpfvisiFli]),maskfvisiSpacesGrpfvisi OR maskfvisiShowAllGrpfvisi
	jz	Ltemp008     ; REQUIRES di = pch, si = ich, dx = ch
	mov	al,0FFh
	db	03Dh	;turns next "mov al,immediate" into "cmp ax,immediate"
; relocated code from LChSpace; handle vifmaMac != 0
LChSpace:
	mov	al,chNonBreakSpace
ifdef DEBUG
	;Assert (al != chNonBreakSpace || vifmaMac != 0) with a call
	;so as not to mess up short jumps.
	call	FLj4
endif ;DEBUG
	mov	bptr ([chVar]),al
	;FLf6 performs ich--; ax = (xt -= dxt); dx = (xp -= dxp);
	;only ax and dx are altered.
	call	FLf6
	jmp	LHaveCh

Ltemp008:
	jmp	LDef

	;Assembler note:  This chSpace code has been moved to near the
	;inner loop for speed-size reasons.
; /* space: count and enter in list */
;		case chSpace: /* == 32 */
;			if (vifmaMac != 0)    /* REVIEW: Opus want this? */
;				{
;				xp -= dxp;
;				xt -= dxt;
;				ich--;
;				ch = chNonBreakSpace; goto LHaveCh;
;				}
;			ichM1 = ich - 1;
;			if (!fPrevSpace)
; /* first in a series of spaces */
;				{
;				xpJust = xp - dxp;
;				xtJust = xt - dxt;
;				vfli.ichSpace3 = ichM1;
;				}
;			fPrevSpace = fTrue;
;			 if (FVisiSpacesFli)
;			    vfli.rgch [ichM1] = chVisSpace;
; /* spaces are entered in a list iff jcBoth */
;			if (jc == jcBoth)
;				if (ichM1 == vfli.ichSpace1)
; /* except leading spaces in the line are ignored */
;					vfli.ichSpace1++;
;				else
;					{
;					cchSpace++;
;					vfli.rgch[ichM1] = ichSpace;
;					ichSpace = ichM1;
;					}
	;Assembler note:  LBreakOppR has been moved up in order to
	;remove an unnecessary jump from the code
; LBreakOppR:
; /* set up break opportunity to the right of the char */
; /* ch is current, pch, xp, ich all incremented */
;			vfli.chBreak = ch;
;			vfli.cpMac = (hpch - vhpchFetch) + vcpFetch;
;			vfli.xpRight = xp;
;			vfli.ichMac = ich;
;			bchrBreak = vbchrMac;
;			dyyAscentBreak = dyyAscentMac;
;			dyyDescentBreak = dyyDescentMac;
;			continue;

; /* end of paragraph/line/section */
;		case chReturn:	/* == 13 */
;			/* get here when CR is not at run start */
;			ich--;
;			xp -= dxp;
;			xt -= dxt;
;			cpNext = vcpFetch + hpch - vhpchFetch;
;			dcpVanish = 1;
;			goto LVanish;
LChReturn:
	; si = pdxpFli, di = pchFli
	;FLf6 performs ich--; ax = (xt -= dxt); dx = (xp -= dxp);
	;only ax and dx are altered.
	call	FLf6
	;FLf8 performs dx:ax = vcpFetch + (hpch - vhpchFetch)
	;only ax and dx are altered.
	call	FLf8
	mov	[OFF_cpNext],ax
	mov	[SEG_cpNext],dx
	xor	ax,ax
	mov	[SEG_dcpVanish],ax
	inc	ax
	mov	[OFF_dcpVanish],ax
	jmp	LVanish
;		case chEop: /* == 13 or == 10 (CRLF) */
;			fInhibitJust = fTrue;
;			chT = FVisiParaMarksFli ? chVisEop : chSpace;
;			xp -= dxp;
;			dxp = dxt = C_DxpFromCh( chVisEop, &vfti );
;			goto LEol;

LChEop:
	; si = pdxpFli, di = pchFli
	mov	bptr ([fInhibitJust]),fTrue
	;FLf7 performs dx = (xp -= dxp);
	;only ax and dx are altered.
	call	FLf7
	mov	al,chVisEop
	mov	ch,al
	;FLf9 performs ax = DxpFromCh(al,&vfti);
	;only ax and bx are altered.
	call	FLf9
	test	bptr ([vfli.grpfvisiFli]),maskfvisiParaMarksGrpfvisi OR maskfvisiShowAllGrpfvisi
	jnz	LEol
	mov     ch,chSpace
	jmp	short LEol

;		case chCRJ: /* == 11 */
;			chT = chSpace;
;LChTSet:
LChCRJ:
	; si = pdxpFli, di = pchFli
	mov	ch,chSpace
LChTSet:

;			xp -= dxp;

;/* WIN version compensates for VISI-CRJ bitmap that is wider than a
;   normal char */

;			dxt = dxp = min( DxpFromCh( chSpace, &vfti ) << 2,
;					 dxpChVisEach );
	;FLf7 performs dx = (xp -= dxp);
	;only ax and dx are altered.
	call	FLf7
	mov	al,chSpace
	;FLf9 performs ax = DxpFromCh(al,&vfti);
	;only ax and bx are altered.
	call	FLf9
	shl	ax,1
	shl	ax,1
	mov	bx,dxpChVisEach
	cmp	ax,bx
	jle	FL47
	xchg	ax,bx
FL47:

; LEol:
;			vfli.rgch[ich - 1] = chT;
;			vfli.rgdxp [ich - 1] = dxp;
;			xp += dxp;
;			vfli.dcpDepend = 0;

LEol:
	; si = pdxpFli, di = pchFli, ch = chT, ax = new dxp
	mov	[dxt],ax
	mov	[dxp],ax
	add	[xpVar],ax
	mov	[si-2],ax
	mov	[di-1],ch
	mov	[vfli.dcpDependFli],0

; LEol1:
;			xt -= dxt;
;			vfli.chBreak = ch;
; /* set up justification point */
;			if (!fPrevSpace)
;				{
;				xpJust = xp - dxp;
;				xtJust = xt - dxt;
;/* WIN: Adjustment to assure vfli.ichSpace3 is a valid guide to
;   the underline termination point. */
;				vfli.ichSpace3 = ich - 1 + dichSpace3;
;				}
;			if (jcTab != jcLeft)
;				{ ich--; goto LAdjustTab; }
;			goto LEndBreak;

LEol1:
	; si = pdxpFli, di = pchFli, ax = dxt
	sub	[xtVar],ax
	; si = pdxpFli, di = pchFli
FL475:
	mov	ax,[chVar]
	mov	[vfli.chBreakFli],ax
	test	bptr ([fPrevSpace]),maskFPrevSpace
	jnz	FL48
	;FLh4 performs
	;xpJust = xp - dxp;
	;xtJust = xt - dxt;
	;vfli.ichSpace3 = ich;
	;only ax and dx are altered.
	call	FLh4
	mov	al,bptr ([fStopFormat+1])   ;really dichSpace3
	dec	ax
	add	[vfli.ichSpace3Fli],al
FL48:
	cmp	bptr ([jcTab]),jcLeft
	je	Ltemp010
	dec	[ichVar]
	jmp	LAdjustTab
Ltemp010:
	jmp	LEndBreak


;		case chTable: /* == 7 */
;			{
;			CachePara(doc,vcpFetch);
LChTable:
	; si = pdxpFli, di = pchFli
	lea	bx, [vcpFetch]
	;FLh9 performs CachePara(doc, [bx]); ax,bx,cx,dx are altered.
	call	FLh9

;			if (vpapFetch.fInTable && vcpFetch+(hpch-vhpchFetch)==caPara.cpLim)
;				{
	cmp	[vpapFetch.fInTablePap],fFalse
	je	FL50
	;FLf8 performs dx:ax = vcpFetch + (hpch - vhpchFetch)
	;only ax and dx are altered.
	call	FLf8
	sub	ax,[caPara.LO_cpLimCa]
	sbb	dx,[caPara.HI_cpLimCa]
	or	ax,dx
	jne	FL50

;				fInhibitJust = fTrue;
;				if (!vfli.fLayout)
;					vfli.fSplatBreak = fTrue;
;				xp -= dxp;
;				dxp = DxpFromCh(chVisCellEnd, &vfti);
;				chT = FVisiParaMarksFli ? chVisCellEnd : chSpace;
	mov	bptr ([fInhibitJust]),fTrue
ifdef DEBUG
	;Assert (vfli.fLayout == (vfli.fLayout & maskFSplatBreakFli)) with
	;a call so as not to mess up short jumps.
	call	FLk9
endif ;DEBUG
	mov	al,[vfli.fLayoutFli]
	xor	al,maskFSplatBreakFli
	or	[vfli.fSplatBreakFli],al
	mov	al,chVisCellEnd
	;FLf9 performs ax = DxpFromCh(al,&vfti);
	;only ax and bx are altered.
	call	FLf9
	mov	bx,ax
	xchg	[dxp],bx
	sub	[xpVar],bx
	mov	ch,chVisCellEnd
	test	bptr ([vfli.grpfvisiFli]),maskfvisiParaMarksGrpfvisi OR maskfvisiShowAllGrpfvisi
	jnz	FL49
	mov	ch,chSpace
FL49:

;				goto LEol;
	jmp	LEol	; si = pdxpFli, di = pchFli, ch = chT, ax = new dxp

;				}
;			/* It's not an end of cell mark so treat it as a normal character */
;			goto LDef;
;			}
FL50:
	jmp	LDef


;		case chColumnBreak: /* == 14 */
;			if (ich == 1 || vfli.fPageView || vfli.fLayout)
;				vfli.fSplatColumn = fTrue;
;			/* FALL THROUGH */
LChColumnBreak:
	; si = pdxpFli, di = pchFli
	mov	al,[vfli.fPageViewFli]
	or	al,[vfli.fLayoutFli]
	jne	FL502
	cmp	[ichVar],1
	jnz	LChSect
FL502:
	or	[vfli.fSplatColumnFli],maskFSplatColumnFli

; /* section mark, line is broken in front of the char unless first in line.
; There are justifying and a non-justifying variants.
; */
;		case chSect: /* == 12 */
;			{
LChSect:
	; si = pdxpFli, di = pchFli

;			if (fInTable)
;				goto LEndBreak0;
	cmp	bptr ([fInTable]),fFalse
	jne	FL505

;			if (vfli.fPageView || vfli.fLayout)
	mov	al,[vfli.fPageViewFli]
	or	al,[vfli.fLayoutFli]
	je	FL51
;				{
;/* page view ; return a splat */
;				vfli.fSplatBreak = fTrue;
;				xp -= dxp;
;				dxp = NMultDiv(xaRight, vfli.dxsInch, dxaInch) - xp;
;				chT = chSpace;
;				goto LEol;
;				}
	or	[vfli.fSplatBreakFli],maskFSplatBreakFli
	mov	ax,[dxp]
	sub	[xpVar],ax
;	FLi05 performs NMultDiv(xaRight, vfli.dxsInch, dxaInch)
;	ax, bx, cx, dx are altered.
	call	FLi05
	sub	ax,[xpVar]
	mov	ch,chSpace
	jmp	LEol
FL505:
	jmp	LEndBreak0

;			if (ich == 1)
;				{ /* Beginning of line; return a splat */
;				vfli.fSplatBreak = fTrue;
;				dxp = vfli.rgdxp[0] = xpSplat - (xp - dxp);
;				xp = xpSplat;
;				goto LBreak;
;				}
FL51:
	cmp	[ichVar],1
	jnz	FL52
	or	[vfli.fSplatBreakFli],maskFSplatBreakFli
	mov	ax,xpSplat
	sub	ax,[xpVar]
	add	ax,[dxp]
	mov	[vfli.rgdxpFli],ax
	mov	[dxp],ax
	mov	[xpVar],xpSplat
	jmp	LBreak
FL52:

; /* undo main loop actions */
;			ich--;
;			pch--;
;			xp -= dxp; dxp = 0;
;			vfli.dcpDepend = 1;
;			dichSpace3 = 1;
;			goto LEol1;
;			}
	;FLf6 performs ich--; ax = (xt -= dxt); dx = (xp -= dxp);
	;only ax and dx are altered.
	call	FLf6
	dec	[OFF_hpch]
	xor	ax,ax
	mov	[dxp],ax
	inc	ax
	mov	[vfli.dcpDependFli],ax
	mov	bptr ([fStopFormat+1]),al   ;really dichSpace3
	;Assembler note: LEol1 performs xt -= dxt in the C version,
	;but it saves code to do it here in assembler and jump to
	;LEol1 after it has done xt -= dxt.
	jmp	FL475	; si = pdxpFli, di = pchFli

; /* tab */
;		case chTab: /* == 9 */
LChTab:
	; si = pdxpFli, di = pchFli

; /* undo main loop actions */
;			if (xt > xtRight) goto LBreak;
;			ich--;
;			xt -= dxt;
;			xp -= dxp;
;			if (!fPrevSpace)
;				{
;				xpJust = xp;
;				xtJust = xt;
;				}
	mov	ax,[xtVar]
	cmp	ax,[xtRight]
	jle	Ltemp009
	jmp	LBreak
Ltemp009:
	;FLf6 performs ich--; ax = (xt -= dxt); dx = (xp -= dxp);
	;only ax and dx are altered.
	call	FLf6
	test	bptr ([fPrevSpace]),maskFPrevSpace
	jne	FL53
	mov	[xtJust],ax
	mov	[xpJust],dx
FL53:

; LAdjustTab:
; /* adjust the previous right/ctr/dec tab, if any. This code is actually
; a subroutine written as in-line code so that locals in FormatLine can
; be freely referenced.
; */
; /* remove chain of spaces */
;			for (ichT = ichSpace; ichT != ichNil; ichT = ichT1)
;				{
;				ichT1 = vfli.rgch[ichT];
;				vfli.rgch[ichT] = (fWin && FVisiSpacesFli)
;				       ? chVisSpace : chSpace;
;				}

	; Loop variables:  bx = ichT, al = ichT1
LAdjustTab:
	mov	bx,[ichSpace]
FL54:
	cmp	bl,ichNil
	je	FL56
	mov	al,chVisSpace
	test	bptr ([vfli.grpfvisiFli]),maskfvisiSpacesGrpfvisi OR maskfvisiShowAllGrpfvisi
	jnz	FL55
	mov	al,chSpace
FL55:
	xchg	al,[bx+vfli.rgchFli]
	mov	bl,al
	jmp	short FL54
Ltemp023:
	jmp	FL63
FL56:

;			ichSpace = ichNil;
;			vfli.ichSpace1 = ich;
;			if (jcTab != jcLeft)
; /* we have from prev tab: jcTab, xtTabStart, xtTabStop, ichTab */
; /* xt is the alignment point that will be moved to right flush tab */
;				{
;				int dxtT;
;				int xpTabStop = fDxt ?
;				  NMultDiv(xtTabStop, vfti.dxpInch, vftiDxt.dxpInch)
;				  : xtTabStop;
	mov	[ichSpace],ichNil
	mov	ax,[ichVar]
	mov	[vfli.ichSpace1Fli],al
	cmp	bptr ([jcTab]),jcLeft
	je	Ltemp023
	mov	ax,[xtTabStop]
	test	bptr ([fDxt]),080h
	jz	FL57
;	FLi03 performs NMultDiv(ax, vfti.dxpInch, vftiDxt.dxpInch)
;	ax, bx, cx, dx are altered.
	call	FLi03
FL57:
	xchg	ax,di

;				switch (jcTab)
;					{
;				case jcRight:
;					dxtT = xtTabStop - xtJust;
;					dxpT = xpTabStop - xpJust;
;					break;
	; di = xpTabStop
	mov	si,[xtTabStop]
	mov	al,bptr ([jcTab])
	; al = jcTab, si = xtTabStop, di = xpTabStop
	cmp	al,jcRight
	jne	FL58
	sub	si,[xtJust]
	sub	di,[xpJust]
	jmp	short FL60	    ; requires si = dxtT, di = dxpT

;				case jcCenter:
;					dxtT = xtTabStop - xtTabStart -
;						((xtJust - xtTabStart + 1) >> 1);
;					dxpT = xpTabStop - xpTabStart -
;						((xpJust - xpTabStart + 1) >> 1);
;					break;
	; al = jcTab, si = xtTabStop, di = dxpTabStop
FL58:
	errnz	<jcCenter - 1>
	errnz	<jcRight - 2>
	errnz	<jcDecimal - 3>
	errnz	<jcDecTable - 5>
	ja	FL59
	mov	ax,[xtJust]
	sub	ax,[xtTabStart]
	inc	ax
	shr	ax,1
	sub	si,[xtTabStart]
	sub	si,ax
	mov	ax,[xpJust]
	sub	ax,[xpTabStart]
	inc	ax
	shr	ax,1
	sub	di,[xpTabStart]
	sub	di,ax
	jmp	short FL60	    ; requires si = dxtT, di = dxpT

;				case jcDecimal:
;				case jcDecTable:
; /* decimal alignment method defined elsewhere for easy modification */
;					dxtT = -DxtLeftOfTabStop(ichTab, ich, pftiDxt, &dxpT) + xtTabStop - xtTabStart;
;					dxpT = -dxpT + xpTabStop - xpTabStart;
;					break;
	; al = jcTab, si = xtTabStop, di = dxpTabStop
; ASSUME jc == Left,Center,Decimal,or Right
FL59:
	lea	ax,[dxpT]
	cCall	DxtLeftOfTabStop,<[ichTab],[ichVar],[pftiDxt],ax>
	neg	ax
	sub	ax,[xtTabStart]
	add	si,ax
	sub	di,[dxpT]
	sub	di,[xpTabStart]
;	jmp	short FL60	    ; requires si = dxtT, di = dxpT
;					}

;				xt += max(0, dxtT);
;				dxpT = max(0, dxpT);
;				if (jcTab != jcDecTable)
;					vfli.rgdxp[ichTab] = dxpT;
;				else
;					vfli.xpLeft += dxpT;
;				xp += dxpT;
	; si = dxtT, di = dxpT
FL60:	or	si,si
	jle	FL61
	add	[xtVar],si
FL61:
	or	di,di
	jge	FL62
	xor	di,di
FL62:
	add	[xpVar],di
	cmp	bptr ([jcTab]),jcDecTable
	je	FL623
	mov	bx,[ichTab]
	shl	bx,1
	mov	[bx+vfli.rgdxpFli],di
	jmp	short FL63
FL623:
	add	[vfli.xpLeftFli],di

;				}
FL63:

;			if (ch != chTab) {ich++; goto LEndBreak; }
	cmp	bptr ([chVar]),chTab
	je	FL64
	inc	[ichVar]
	jmp	LEndBreak
FL64:
; /* continue ch == chTab processing */

; /* break opportunity in front of the tab, unless first in line */
;			if (ich)
;				{
;				vfli.cpMac = (hpch - vhpchFetch) + vcpFetch - 1;
;				vfli.xpRight = xp;
;				vfli.ichMac = ich;
;				vfli.chBreak = ch;
;				bchrBreak = vbchrMac;
;				dyyAscentBreak = dyyAscentMac;
;				dyyDescentBreak = dyyDescentMac;
;				}
	cmp	[ichVar],0
	jz	FL65
	;FLf8 performs dx:ax = vcpFetch + (hpch - vhpchFetch)
	;only ax and dx are altered.
	call	FLf8
	sub	ax,1
	sbb	dx,0
	;FLg4 performs
	;vfli.chBreak = chVar
	;vfli.cpMac = dx:ax;
	;vfli.xpRight = xp;
	;vfli.ichMac = ich;
	;bchrBreak = vbchrMac;
	;dyyAscentBreak = dyyAscentMac;
	;dyyDescentBreak = dyyDescentMac;
	;only ax, bx, dx are altered.
	call	FLg4
FL65:

; /* Now get info about this tab using xt, result to xtTabStop, jcTab.
; The tab will go to the next tab stop at position > xt.
; */
;			while (itbd < itbdMac &&
;				(xtTabStop = XtFromXa(vpapFetch.rgdxaTab[itbd]))
;				- dxtTabMinT <= xt ||
;				/* bar tab stop */
;				((struct TBD *)vpapFetch.rgtbd + itbd)->jc == jcBar)
;				itbd++;
	mov	si,[itbd]
	mov	di,dxaInch
	; in the loop:	si = itbd, di = dxaInch
	dec	si
FL66:
	inc	si
	cmp	si,[itbdMac]
	jge	FL67
	mov	bx,si
	mov	ax,[bx+si.vpapFetch.rgdxaTabPap]
;	LN_XtFromXa performs XtFromXa(ax)
;	ax, bx, cx, dx are altered.
	call	LN_XtFromXa
	mov	[xtTabStop],ax
	sub	ax,[dxtTabMinT]
	cmp	ax,[xtVar]
	jle	FL66
	mov	al,[si+vpapFetch.rgtbdPap]
	errnz	<maskJcTbd - 007H>
	and	al,maskJcTbd
	cmp	al,jcBar
	je	FL66
FL67:
	mov	[itbd],si

; /* splice in special default tab at left indent point */
;			if (vpapFetch.dxaLeft > xaLeft)
;				{
; /* there is a hanging indent, checked to gain speed in common case */
;				int xtTabDef = XtFromXa(vpapFetch.dxaLeft);
	; di = dxaInch
	mov	ax,[vpapFetch.dxaLeftPap]
	cmp	ax,[xaLeft]
	jle	FL70
;	LN_XtFromXa performs XtFromXa(ax)
;	ax, bx, cx, dx are altered.
	call	LN_XtFromXa
	mov	si,ax

;				if (xt < (xtTabDef - dxtTabMinT) &&
;					(itbd >= itbdMac ||
;					xtTabDef < xtTabStop))
; /* select default tab at hanging indent point */
;					{
;					xtTabStop = xtTabDef;
;					goto LDefTab;
;					}
	; di = dxaInch, si = ax = xtTabDef
	sub	ax,[dxtTabMinT]
	cmp	ax,[xtVar]
	jle	FL69
	mov	ax,[itbd]
	cmp	ax,[itbdMac]
	jge	FL68
	cmp	si,[xtTabStop]
	jge	FL69
FL68:
	mov	[xtTabStop],si
	jmp	short LDefTab
FL69:
;				}
FL70:
;			if (itbd >= itbdMac)
;				{
; /* go to next default tab */
;				struct DOP *pdop = &PdodMother(doc)->dop;
;				int dxtTab = XtFromXa(pdop->dxaTab);
;				if (dxtTab == 0) dxtTab = 1;
;				xtTabStop = ((xt + dxtTabMinT) / dxtTab + 1) * dxtTab;
; LDefTab:			  tlc = tlcNone;
;				jcTab = jcLeft;
;				}
	mov	ax,[itbd]
	cmp	ax,[itbdMac]
	jl	FL72
	cCall	N_PdodMother,<[doc]>
	xchg	ax,si
	mov	ax,[si.dopDod.dxaTabDop]
;	LN_XtFromXa performs XtFromXa(ax)
;	ax, bx, cx, dx are altered.
	call	LN_XtFromXa
	xchg	ax,di
	or	di,di
	jnz	FL71
	inc	di
FL71:
	mov	ax,[xtVar]
	add	ax,[dxtTabMinT]
	xor	dx,dx
	div	di
	inc	ax
	mul	di
	mov	[xtTabStop],ax
LDefTab:
	mov	wptr ([jcTab]),jcLeft + (tlcNone SHL 8)
	jmp	short FL74

;			else
;				{
;				struct TBD tbd;
; /* choose real tab stop itbd */

FL72:
;				if (xtTabStop >= xtRight && abs(vfli.dxa - vsepFetch.dxaColumnWidth) <= 20)
;					{
	mov	ax,[xtTabStop]
	cmp	ax,[xtRight]
	jl	FL73
	mov	ax,[vfli.dxaFli]
	sub	ax,[vsepFetch.dxaColumnWidthSep]
	cwd
	xor	ax,dx
	sub	ax,dx
	cmp	ax,20
	jg	FL73

;/* "breakthrough" tab: set right margin to 20" -- but only in normal lines,
;   meaning not tables and not APO-constrained */
;					Win( xtRight = NMultDiv( vsci.xaRightMax,
;								vftiDxt.dxpInch,
;								czaInch));
;					jc = jcLeft;
;					}
;				tbd = vpapFetch.rgtbd[itbd++];
;				tlc = tbd.tlc;
;				jcTab = tbd.jc;
;	FLi2 performs NMultDiv(ax, dx, czaInch)
;	ax, bx, cx, dx are altered.
	mov	ax,[vsci.xaRightMaxSci]
	mov	dx,[vftiDxt.dxpInchFti]
	call	FLi2
	mov	[xtRight],ax
	mov	bptr ([jcVar]),jcLeft
FL73:
	mov	bx,[itbd]
	mov	al,[bx+vpapFetch.rgtbdPap]
	inc	[itbd]
	mov	ah,al
	and	ax,(maskTlcTbd SHL 8) + maskJcTbd
	mov	cl,ibitTlcTbd
	shr	ah,cl
	errnz	<maskJcTbd - 007H>
	mov	wptr ([jcTab]),ax   ;sets [tlc] also

;				}
FL74:
; /* Do left-justified tabs immediately */
;			if (jcTab == jcLeft)
;				{
;				dxt = xtTabStop - xt;
;				dxp = ((fDxt) ? NMultDiv(xtTabStop, vfti.dxpInch,
;					vftiDxt.dxpInch) : xtTabStop) - xp;
;                               dxp = max(dxp, 0);
;				}
	cmp	bptr ([jcTab]),jcLeft
	jne	FL76
	mov	ax,[xtTabStop]
	mov	cx,ax
	sub	cx,[xtVar]
	mov	[dxt],cx
	test	bptr ([fDxt]),080h
	jz	FL75
;	FLi03 performs NMultDiv(ax, vfti.dxpInch, vftiDxt.dxpInch)
;	ax, bx, cx, dx are altered.
	call	FLi03
FL75:
	sub	ax,[xpVar]
	jg	FL755
        xor     ax,ax
FL755:
	mov	[dxp],ax
	jmp	short FL77
;			/* else dxt, dxp = dxuMissing */
;			else
;				{
; /* save state for AdjustTab */
;				ichTab = ich;
;				dxt = dxtTabMinT; dxp = 0;
;				}
FL76:
	mov	ax,[ichVar]
	mov	[ichTab],ax
	mov	ax,[dxtTabMinT]
	mov	[dxt],ax
	mov	[dxp],0
FL77:

; /* reset justification parameters */
;			/* ichSpace = ichNil as set above */
;			cchSpace = 0;
;			fPrevSpace = fFalse;
; /* create CHR describing the tab */
;			ch = 0;
;			xtTabStart = xt + dxt;
;			xpTabStart = xp;

	xor	ax,ax
	mov	[cchSpace],ax
	and	bptr ([fPrevSpace]),NOT maskFPrevSpace
	mov	bptr ([chVar]),al
	mov	ax,[xtVar]
	add	ax,[dxt]
	mov	[xtTabStart],ax
	mov	ax,[xpVar]
	mov	[xpTabStart],ax

; /* code to create CHRT for tab or visible char in ch, then handle as normal
; character;
; we have: ch, dxp, dxt.
; */
; LChrt:		OverhangChr(&xp, bchrPrev,ich);
;			bchrPrev = vbchrMac;
;			Debug(vdbs.fShakeHeap ? ShakeHeap() : 0);
LChrT:
	;LN_OverhangChr performs: adjust previous chr for overhang
	;also performs bchrPrev = vbchrMac.
	;ax, bx, cx, dx are altered.
	call	LN_OverhangChr
ifdef DEBUG
	cmp	[vdbs.fShakeHeapDbs],0
	jz	FL78
	mov	ax,1
	cCall	ShakeHeapSb,<ax>
FL78:
endif ;DEBUG

;			if ((vbchrMac += cbCHRT) <= vbchrMax ||
;				FExpandGrpchr(cbCHRT))
;				{
;				pchr = &(**vhgrpchr)[bchrPrev];
;				pchr->chrm = chrmTab;
;				pchr->ich = ich;
;				((struct CHRT *)pchr)->tlc = tlc;
;				((struct CHRT *)pchr)->ch = ch;
;				}
	mov	bx,cbCHRT
;	FLf4 performs;
;	vbchrMac+=bx;
;	return ((vbchrMac > vbchrMax) ? FExpandGrpchr(bx) : fTrue);
;	FLf4 returns zero flag set if the routine failed,
;	otherwise returns zero flag reset.
	call	FLf4
	jz	FL79
	mov	al,chrmTab
	;FLh0 performs bx = &(**vhgrpchr)[bchrPrev];
	;[bx.chrmChr] = al; [bx.ichChr] = ich;
	;only ax, bx, dx are altered.
	call	FLh0
	errnz	<(chChrt) - 2>
	errnz	<(tlcChrt) - 3>
	mov	al,bptr ([chVar])
	mov	ah,bptr ([jcTab+1])	;this is [tlc]
	mov	wptr ([bx.chChrt]),ax

; /* now we have a character that either fits or does not fit.
; dxp and dxt are set
; */
;			xp += dxp;
;			xt += dxt;
;			vfli.rgdxp[ich++] = dxp;
;			if (ch != chNonReqHyphen && ch != chPubEmDash 
;				&& ch != chPubEnDash)
;				goto LDef;
;			else
;				goto LHyphen1;

FL79:
	mov	ax,[dxt]
	add	[xtVar],ax
	mov	ax,[dxp]
	add	[xpVar],ax
	mov	bx,[ichVar]
	inc	[ichVar]
	shl	bx,1
	mov	[bx+vfli.rgdxpFli],ax
	mov	al,bptr ([chVar])
	cmp	al,chNonReqHyphen
	je	Ltemp012
	cmp	al,chPubEmDash
	je	Ltemp028
	cmp	al,chPubEnDash
	je	Ltemp028
	jmp	LDef
Ltemp012:
	jmp	LHyphen1
Ltemp028:
	jmp	LChHyphen

;/* following characters are special in Formulas (if cParen is small) */
;		 case '(':
;			 if (!ffs.fFormatting || ffs.flt != fltFormula)
;				 goto LDef;
;			 if (!fmal.fLiteral)
;				 {
;				 if (cParen != 255)
;					 cParen++;
;				 goto LDef;
;				 }
;LLit:			 fmal.fLiteral = fFalse;
;			 goto LDef;
;
;		 case ')':
;			 if (!ffs.fFormatting || ffs.flt != fltFormula)
;				 goto LDef;
;			 if (fmal.fLiteral) goto LLit;
;			 if (cParen != 255)
;				 cParen--;
;			 if (cParen == 0)
;				 goto LFormula;
;			 goto LDef;
;LChComma:	/* case ',': */
;			 if (!ffs.fFormatting || ffs.flt != fltFormula)
;				 goto LDef;
;			 if (cParen != 1 || fmal.fLiteral || (vifmaMac > 0 &&
;					 rgfma[vifmaMac - 1].fmt == fmtList))
;				 goto LLit;
;			 goto LFormula;
;		 case chFormula: /* == 6 */
;			 if (!ffs.fFormatting || ffs.flt != fltFormula)
;				 goto LDef;
;			 if (fmal.fLiteral)
;				 goto LLit;
LChLeftParen:
LChRightParen:
LChComma:
LChFormula:
	; si = pdxpFli, di = pchFli; bx = offset into rglbl
	cmp	[ffs.fFormattingFfs],0
	jz	FL81
	cmp	[ffs.fltFfs],fltFormula
	jnz	FL81
	cmp	[fmal.fLiteralFmal],0
	jnz	LLit
	errnz	<iWLChFormula - 28>
	cmp	bl,30	    ;using iWLChFormula+2 here generates a nop
	je	LFormula
	errnz	<iWLChComma - 30>
	cmp	bl,32	    ;using iWLChFormula+2 here generates a nop
	jne	FL80
	cmp	bptr ([cParen]),1
	jnz	LLit
	mov	si,[vifmaMac]
	dec	si
	jl	LFormula
	mov	ax,cbFmaMin
	mul	si
	xchg	ax,si
	mov	al,bptr ([si.rgfma])
	and	al,maskFmtFma
	errnz	<maskFmtFma - 00Fh>
	cmp	al,fmtList
	jnz	LFormula
LLit:
	mov	[fmal.fLiteralFmal],fFalse
	jmp	short FL81
FL80:
	cmp	bptr ([cParen]),255
	je	FL81
	errnz	<iWLChLeftParen - 24>
	cmp	bl,26	    ;using iWLChLeftParen+2 here generates a nop
	jne	FL82
;				 if (cParen != 255)
;					 cParen++;
;				 goto LDef;
	inc	bptr ([cParen])
FL81:
	jmp	LDef
FL82:
;			 if (cParen != 255)
;				 cParen--;
;			 if (cParen == 0)
;				 goto LFormula;
;			 goto LDef;
	dec	bptr ([cParen])
	jnz	FL81

;LFormula:
;/* undo main loop actions */
;			 ich--;
;			 xt -= dxt;
;			 xp -= dxp;
LFormula:
	;FLf6 performs ich--; ax = (xt -= dxt); dx = (xp -= dxp);
	;only ax and dx are altered.
	call	FLf6

;/* call procedure to do as much of the processing as possible outside of
;the optimized environment.
;*/
;			 fmal.doc = doc;
;			 fmal.cp = (hpch - vhpchFetch) + vcpFetch;
;			 fmal.ich = ich;
;			 fmal.xt = xt;
;			 fmal.xp = xp;
;			 fmal.dypAscent = dyyAscentMac.dyp;
;			 fmal.dytAscent = dyyAscentMac.dyt;
;			 fmal.dypDescent = dyyDescentMac.dyp;
;			 fmal.dytDescent = dyyDescentMac.dyt;
;			 fmal.bchrChp = bchrChp;
;			 fmal.cParen = cParen;
	push	ds
	pop	es
	lea	si,[cParen]
	lea	di,[fmal.cParenFmal]
	lodsw
	stosb
	errnz	<(OFFBP_ichVar) - (OFFBP_cParen) - 2>
	errnz	<(ichFmal) - (cParenFmal) - 1>
	lodsw
	stosb
	errnz	<(OFFBP_dypAscentMac) - (OFFBP_ichVar) - 2>
	errnz	<(dypAscentFmal) - (ichFmal) - 2>
	inc	di
	movsw
	errnz	<(OFFBP_dytAscentMac) - (OFFBP_dypAscentMac) - 2>
	errnz	<(dytAscentFmal) - (dypAscentFmal) - 2>
	movsw
	errnz	<(OFFBP_dypDescentMac) - (OFFBP_dytAscentMac) - 2>
	errnz	<(dypDescentFmal) - (dytAscentFmal) - 2>
	movsw
	errnz	<(OFFBP_dytDescentMac) - (OFFBP_dypDescentMac) - 2>
	errnz	<(dytDescentFmal) - (dypDescentFmal) - 2>
	movsw
	errnz	<(OFFBP_xtVar) - (OFFBP_dytDescentMac) - 2>
	errnz	<(xtFmal) - (dytDescentFmal) - 4>
	inc	di
	inc	di
	movsw
	errnz	<(OFFBP_xpVar) - (OFFBP_xtVar) - 2>
	errnz	<(xpFmal) - (xtFmal) - 2>
	movsw
	;FLf8 performs dx:ax = vcpFetch + (hpch - vhpchFetch)
	;only ax and dx are altered.
	call	FLf8
	errnz	<(cpFmal) - (xpFmal) - 10>
	add	di,8
	stosw
	xchg	ax,dx
	stosw
	errnz	<(docFmal) - (cpFmal) - 4>
	mov	ax,[doc]
	stosw
	mov	ax,[bchrChp]
	mov	[fmal.bchrChpFmal],ax

;			 FormatFormula(&fmal);
;			 if (fmal.fError)
;				 {
;				 Assert (ffs.ifldError == ifldNil);
;				 ffs.ifldError = ffs.ifldFormat;
;				 goto LRestartLine;
;				 }
	lea	ax,[fmal]
	cCall	FormatFormula,<ax>
	cmp	[fmal.fErrorFmal],fFalse
	jz	FL83
ifdef DEBUG
;	Assert (ffs.ifldError == ifldNil) with a call so not to mess
;	up short jumps
	call	FLi7
endif ;DEBUG
	mov	ax,[ffs.ifldFormatFfs]
	mov	[ffs.ifldErrorFfs],ax
	jmp	LRestartLine

;			 docNext = doc;
;			 cpNext = fmal.cp;
;			 /*pch = pchLim;*/
;			 ich = fmal.ich;
;			 xt = fmal.xt;
;			 xp = fmal.xp;
;			 dyyAscentBreak.dyp = dyyAscentMac.dyp = fmal.dypAscent;
;			 dyyDescentBreak.dyp = dyyDescentMac.dyp = fmal.dypDescent;
;			 dyyAscentBreak.dyt = dyyAscentMac.dyt = fmal.dytAscent;
;			 dyyDescentBreak.dyt = dyyDescentMac.dyt = fmal.dytDescent;
;			 fHeightPending = fTrue;
;			 bchrChp = fmal.bchrChp;
;			 cParen = fmal.cParen;
FL83:
	push	ds
	pop	es
	lea	si,[fmal.cParenFmal]
	lea	di,[cParen]
	xor	ax,ax
	lodsb
	stosw
	errnz	<(OFFBP_ichVar) - (OFFBP_cParen) - 2>
	errnz	<(ichFmal) - (cParenFmal) - 1>
	lodsb
	stosw
	errnz	<(OFFBP_dypAscentMac) - (OFFBP_ichVar) - 2>
	errnz	<(dypAscentFmal) - (ichFmal) - 2>
	inc	si
	movsw
	errnz	<(OFFBP_dytAscentMac) - (OFFBP_dypAscentMac) - 2>
	errnz	<(dytAscentFmal) - (dypAscentFmal) - 2>
	movsw
	errnz	<(OFFBP_dypDescentMac) - (OFFBP_dytAscentMac) - 2>
	errnz	<(dypDescentFmal) - (dytAscentFmal) - 2>
	movsw
	errnz	<(OFFBP_dytDescentMac) - (OFFBP_dypDescentMac) - 2>
	errnz	<(dytDescentFmal) - (dypDescentFmal) - 2>
	movsw
	errnz	<(OFFBP_xtVar) - (OFFBP_dytDescentMac) - 2>
	errnz	<(xtFmal) - (dytDescentFmal) - 4>
	inc	si
	inc	si
	movsw
	errnz	<(OFFBP_xpVar) - (OFFBP_xtVar) - 2>
	errnz	<(xpFmal) - (xtFmal) - 2>
	movsw
	errnz	<(OFFBP_cpNext) - (OFFBP_xpVar) - 2>
	errnz	<(cpFmal) - (xpFmal) - 10>
	add	si,8
	movsw
	movsw
	errnz	<(OFFBP_docNext) - (OFFBP_cpNext) - 4>
	errnz	<(docFmal) - (cpFmal) - 4>
	movsw
	mov	ax,[fmal.bchrChpFmal]
	mov	[bchrChp],ax
	;FLg7 performs
	;dypAscentBreak = dypAscentMac;
	;dypDescentBreak = dypDescentMac;
	; and the same for the dytAscent/Descent
	;only ax is altered.
	call	FLg7
	or	bptr ([fPrevSpace]),maskFHeightPending

;			 ffs.fValidEnd = (fmal.cParen == 255);
;			 goto LFetch;
	errnz	<fFalse>
	xor	ax,ax
	cmp	[fmal.cParenFmal],255
	jnz	FL84
	errnz	<fTrue - fFalse - 1>
	inc	ax
FL84:
	mov	[ffs.fValidEndFfs],ax
	jmp	short LFetch

;			} /* switch (ch) */


; /* we come here at the end of each run */
; LNewRun:
;		if (ich >= ichMaxLine)
; /* end of run because of line length limit has been reached. Equivalent to
; breaking through the right margin */
;			{
;			fInhibitJust = fTrue;
;			goto LEndBreak0;
;			}

LNewRun:
	cmp	[ichVar],ichMaxLine
	jl	LFetch
	mov	bptr ([fInhibitJust]),fTrue
	jmp	LEndBreak0

; /* read next run sequentially, unless docNext, cpNext has been set */
; LFetch:
;		if (cpNext >= caPara.cpLim || cpNext >= CpMacDoc (doc)
;			|| fStopFormat)
;			{
;			vfli.cpMac = cpNext;
;			vfli.chBreak = chEop;
;			goto LEndBreakCp;	/* possible vanished eop */
;			}
;		FetchCp(docNext, cpNext, fcm);
;		chpFLDxa = vchpFetch;

; relocated from below
FL845:
	je	LFetch
FL85:
	mov	ax,[OFF_cpNext]
	mov	dx,[SEG_cpNext]
	mov	[vfli.chBreakFli],chEop
	jmp	LEndBreakCp	;expects new vfli.cpMac in dx:ax
LFetch:
	mov	ax,[OFF_cpNext]
	mov	dx,[SEG_cpNext]
	sub	ax,[caPara.LO_cpLimCa]
	sbb	dx,[caPara.HI_cpLimCa]
	jge	FL85
	cCall	CpMacDoc,<[doc]>
	mov	bx,[OFF_cpNext]
	mov	cx,[SEG_cpNext]
	cmp	bx,ax
	push	cx
	sbb	cx,dx
	pop	cx
	jge	FL85
	cmp	bptr ([fStopFormat]),fFalse
	jne	FL85
ifdef DEBUG
	cCall	S_FetchCp,<[docNext],cx,bx,[fcm]>
else ;not DEBUG
	cCall	N_FetchCp,<[docNext],cx,bx,[fcm]>
endif ;DEBUG
	push	ds
	pop	es
	mov	si,dataoffset [vchpFetch]
	lea	di,[chpFLDxa]
	mov	cx,cwCHP
	rep	movsw

;		docNext = docNil;
;		hpch = vhpchFetch;
;		vfli.fRMark |= !chpFLDxa.fSysVanish && (chpFLDxa.fRMark || chpFLDxa.fStrike);

	mov	[docNext],docNil
	mov	ax,wlo ([vhpchFetch])
	mov	[OFF_hpch],ax
	mov	ax,whi ([vhpchFetch])
	mov	[SEG_hpch],ax
	;LN_SetEsSiFetch takes sets es:si according to hpch.
	;ax, bx, cx, si, es are altered.
	call	LN_SetEsSiFetch
	test	[chpFLDxa.fSysVanishChp],maskFSysVanishChp
	jnz	FL86
	errnz <(fRMarkChp) - (fStrikeChp) - 1>
	test	wptr ([chpFLDxa.fStrikeChp]),maskFStrikeChp+(maskFRmarkChp SHL 8)
	jz	FL86
	or	[vfli.fRMarkFli],maskFRMarkFli
FL86:

; #ifdef CRLF
; /* Do this here AND in switch (ch) so that we don't get 0-length chrmCHPs
;    when there's a chReturn at the start of a run */
;		if (*hpch == chReturn)
;			{	/* Make CR's a phony vanished run */
;			cpNext = vcpFetch + 1;
;			dcpVanish = 1;
;			goto LVanish;
;			}
; #endif  /* CRLF */
	; es:si = qch
	lods	bptr es:[si]
	cmp	al,chReturn
	jnz	FL87
	mov	ax,1
	cwd
	mov	[OFF_dcpVanish],ax
	mov	[SEG_dcpVanish],dx
;	FLh98 performs:
;	cpNext = vcpFetch + dx:ax;
;	only ax and dx are altered.
	call	FLh98
	jmp	short LVanish
FL87:

;		if (chpFLDxa.fVanish || chpFLDxa.fFldVanish)
;			{
;			BOOL fRestartPara;
	errnz	<(fFldVanishChp) - (fVanishChp)>
	test	[chpFLDxa.fVanishChp],maskFVanishChp+maskFFldVanishChp
	jnz	Ltemp018
	jmp	FL94
Ltemp018:

;			vfli.fVolatile = fTrue;
;			if (chpFLDxa.fSysVanish || !FSeeHiddenFli)
;				{
	mov	[vfli.fVolatileFli],fTrue
	test	[chpFLDxa.fSysVanishChp],maskFSysVanishChp
	jnz	Ltemp014
	test	bptr ([vfli.grpfvisiFli]),maskfSeeHiddenGrpfvisi OR maskfvisiShowAllGrpfvisi
	jz	Ltemp014
	jmp	FL93
Ltemp014:

;				fPassChSpec = fFalse;
	mov	bptr ([fPassChSpec]),fFalse

;				if (fInTable && *(vhpchFetch + vccpFetch - 1) == chTable)
;					{
	cmp	bptr ([fInTable]),fFalse
	je	FL875
	mov	bx,[vccpFetch]
	;Assembler note: use -2 here because of previous lodsb
	cmp	bptr es:[bx+si-2],chTable
	jne	FL875

;/* We don't allow vanishing cell marks.  There are two possibilities if we
;   encounter a hidden cell mark.  Either it is alone in the run, in which
;   case we just show it, or it is in a run with other hidden text, in which
;   case we hide only the text and back up vccpFetch so we pick up the cell
;   mark alone the next time through.
;*/
;					if (vccpFetch == 1)
;						goto LVanish2;
;					else
;						vccpFetch -= 1;
;					}
	dec	bx
	je	Ltemp026
	mov	[vccpFetch],bx
FL875:

;				cpNext = vcpFetch + vccpFetch;
;				dcpVanish = vccpFetch;
	mov	ax,[vccpFetch]
	cwd
	mov	[OFF_dcpVanish],ax
	mov	[SEG_dcpVanish],dx
;	FLh98 performs:
;	cpNext = vcpFetch + dx:ax;
;	only ax and dx are altered.
	call	FLh98


; LVanish:
;
;/*  At this point, cpNext has been set up to
;    the cpLim of the "run" to be vanished. it may or may not correspond  to
;    a run.  If it does, avoid random access fetch.
;
;    dcpVanish has been set to the number of characters to be vanished.
;*/
;				if (cpNext != vcpFetch + vccpFetch)
;					docNext = doc;
LVanish:
	mov	ax,[vccpFetch]
	cwd
	add	ax,[word ptr vcpFetch]
	adc	dx,[word ptr vcpFetch+2]
	sub	ax,[OFF_cpNext]
	sbb	dx,[SEG_cpNext]
	or	ax,dx
	jz	FL90
	mov	ax,[doc]
	mov	[docNext],ax
FL90:

;				OverhangChr( &xp,bchrPrev, ich);
;				bchrPrev = vbchrMac;
;				Debug(vdbs.fShakeHeap ? ShakeHeap() : 0);
	;LN_OverhangChr performs: adjust previous chr for overhang
	;also performs bchrPrev = vbchrMac.
	;ax, bx, cx, dx are altered.
	call	LN_OverhangChr
ifdef DEBUG
	cmp	[vdbs.fShakeHeapDbs],fFalse
	jz	FL91
	mov	ax,1
	cCall	ShakeHeapSb,<ax>
FL91:
endif ;DEBUG

;				if ((vbchrMac += cbCHRV) > vbchrMax)
;					if (!FExpandGrpchr(cbCHRV))
;						goto LEndBreak0;
;				pchr = &(**vhgrpchr)[bchrPrev];
;				pchr->ich = ich;
;				pchr->chrm = chrmVanish;
;				((struct CHRV *)pchr)->dcp = dcpVanish;
	mov	bx,cbCHRV
;	FLf4 performs;
;	vbchrMac+=bx;
;	return ((vbchrMac > vbchrMax) ? FExpandGrpchr(bx) : fTrue);
;	FLf4 returns zero flag set if the routine failed,
;	otherwise returns zero flag reset.
	call	FLf4
	jnz	Ltemp016
	jmp	LEndBreak0
Ltemp026:
	jmp	LVanish2
Ltemp016:
	mov	al,chrmVanish
	;FLh0 performs bx = &(**vhgrpchr)[bchrPrev];
	;[bx.chrmChr] = al; [bx.ichChr] = ich;
	;only ax, bx, dx are altered.
	call	FLh0
	mov	ax,[OFF_dcpVanish]
	mov	dx,[SEG_dcpVanish]
	mov	[bx.LO_dcpChrv],ax
	mov	[bx.HI_dcpChrv],dx

;				fRestartPara = fFalse;
	xor	si,si

;				/*  if vanished run vanishes the EOP, cache
;				    the new paragraph */
;				if (caPara.cpLim <= cpNext)
;					{
	mov	ax,[OFF_cpNext]
	mov	dx,[SEG_cpNext]
	cmp	ax,[caPara.LO_cpLimCa]
	push	dx
	sbb	dx,[caPara.HI_cpLimCa]
	pop	dx
	jl	FL916

;					/* end line if in outline mode */
;					if (vfli.fOutline)
;						goto LBreakMode;
;					cpCurPara = cpNext;
;					CachePara (doc, cpNext);
	cmp	[vfli.fOutlineFli],fFalse
	jne	FL918
	mov	[OFF_cpCurPara],ax
	mov	[SEG_cpCurPara],dx
	;FLh8 performs CachePara(doc, cpNext); ax,bx,cx,dx are altered.
	call	FLh8

;					/* for non-tables, make sure that apos
;					   do not flow into, and are not flowed
;					   into by, non-apo text */
;					if (!fInTable && (vfli.fLayout || vfli.fPageView))
;						{
	cmp	bptr ([fInTable]),fFalse
	jne	FL914
	mov	al,[vfli.fPageViewFli]
	or	al,[vfli.fLayoutFli]
	je	FL914

;						if (fAbs != FAbsPap(doc, &vpapFetch))
;							{
;							vfli.fParaStopped = fTrue;
;							goto LBreakMode;
;							}
	push	[doc]
	mov	ax,dataoffset [vpapFetch]
	push	ax
	cCall	FAbsPap,<>
	mov	cl,bptr ([fInTable+1])	    ;fAbs
	cmp	al,cl
	jne	FL917

;						if (fAbs)
;							{
	or	cl,cl
	je	FL914

;							int pap[cwPAPBaseScan];
;							CachePara(doc, vfli.cpMin);
	;FLh9 performs CachePara(doc, [bx]); ax,bx,cx,dx are altered.
	mov	bx,dataoffset [vfli.cpMinFli]
	call	FLh9

;							blt(&vpapFetch, &pap, cwPAPBaseScan);
	mov	si,dataoffset [vpapFetch]
	lea	di,[pap]
	push	[doc]	    ;argument for FMatchAbs
	push	si	    ;argument for FMatchAbs
	push	di	    ;argument for FMatchAbs
	errnz	<cbPAPBaseScan AND 1>
	mov	cx,cbPAPBaseScan SHR 1
	push	ds
	pop	es
	rep	movsw

;							CachePara(doc, cpNext);
	;FLh8 performs CachePara(doc, cpNext); ax,bx,cx,dx are altered.
	call	FLh8

;							if (!FMatchAbs(doc, &vpapFetch, pap))
;								{
;								vfli.fParaStopped = fTrue;
;								goto LBreakMode;
;								}
	cCall	FMatchAbs,<>
	xor	si,si	    ;restore fRestartPara == fFalse;
	or	ax,ax
	je	FL917

;							}
;						}
FL914:

;					if (ich == 0)
;						fRestartPara = fTrue;
	cmp	[ichVar],1
	adc	si,si

;					}
FL916:

;				/* we have revealed an unexpected table */
;				if (vpapFetch.fInTable &&
;					fInTable != FInTableVPapFetch(doc,cpNext))
;				    {
;LBreakMode:
;				    vfli.cpMac = cpNext;
;				    vfli.chBreak = chEop;
;				    vfli.dcpDepend = 1;
;				    goto LEndBreakCp;
;				    }
;				else if (fRestartPara)
;				    goto LRestartPara;
;				goto LFetch;
	;FLk2 performs:
	;if (vpapFetch.fInTable &&
	;	 fInTable != FInTableVPapFetch(doc,cpNext))
	;And clears the zero flag if both are true
	;ax, bx, cx, dx are altered.
	call	FLk2
	jnz	FL918
	or	si,si
	je	Ltemp027
	jmp	LRestartPara
FL917:
	or	[vfli.fParaStoppedFli],maskFParaStoppedFli
	or	bp,bp	    ;set zero flag for test at FL845
FL918:
	mov	[vfli.dcpDependFli],1
Ltemp027:
	jmp	FL845

; Relocated code from "limit run not to exceed ichMaxLine"
;			{
;			hpchLim = hpch + ichMaxLine - ich;
;			docNext = doc;
;			cpNext = vcpFetch + ichMaxLine - ich;
;			}
FL92:
;	FLh92 performs:
;	hpchLim = hpch + ax;
;	cpNext = vcpFetch + ax;
;	docNext = doc;
;	only ax and dx are altered.
	mov	ax,ichMaxLine
	sub	ax,[ichVar]
	call	FLh92
	jmp	short FL95
;				}
;			else if (vlm == lmNil)
; LVanish1:
;				chpFLDxa.kul = kulDotted;
FL93:	mov	ax,[vlm]
	or	ax,ax
	jnz	FL94
LVanish1:
	and	[chpFLDxa.kulChp],NOT maskKulChp
	or	[chpFLDxa.kulChp],kulDotted SHL ibitKulChp

;			}
; LVanish2:
LVanish2:
FL94:

; /* limit run not to exceed ichMaxLine */
;		if (ich + vccpFetch > ichMaxLine)
;			{
;			hpchLim = hpch + ichMaxLine - ich;
;			docNext = doc;
;			cpNext = vcpFetch + ichMaxLine - ich;
;			}
;		else
;			hpchLim = hpch + vccpFetch;
	mov	ax,[ichVar]
	add	ax,[vccpFetch]
	cmp	ax,ichMaxLine
	jg	FL92		     ; ich exceeds; jump to relocated code
	mov	ax,[OFF_hpch]
	add	ax,[vccpFetch]
	mov	[OFF_hpchLim],ax
FL95:


; /* also limit run not to exceed para */
;		if ((long) hpchLim >= (long) vhpchFetch +
;						caPara.cpLim - vcpFetch)
;			{
;			hpchLim = vhpchFetch + (int) (caPara.cpLim - vcpFetch);
;			cpNext = caPara.cpLim;
;			}
	mov	bx,[caPara.LO_cpLimCa]
	mov	cx,[caPara.HI_cpLimCa]
	mov	ax,wlo ([vhpchFetch])
	xor	dx,dx
	add	ax,bx
	adc	dx,cx
	sub	ax,[word ptr vcpFetch]
	sbb	dx,[word ptr vcpFetch+2]
	jnz	FL96
	cmp	[OFF_hpchLim],ax
	jb	FL96
	mov	[OFF_hpchLim],ax
	mov	[OFF_cpNext],bx
	mov	[SEG_cpNext],cx
FL96:

;		/*  pre-process fSpec chars */
;		if (chpFLDxa.fSpec)
;			{
;			/* limit all fSpec runs to be one character long */
;			hpchLim = hpch + 1;
;			docNext = doc;
;			cpNext = vcpFetch + 1;
;
	test	[chpFLDxa.fSpecChp],maskFSpecChp
	jnz	Ltemp040
	jmp	FLa9
Ltemp040:
;	FLh92 performs:
;	hpchLim = hpch + ax;
;	cpNext = vcpFetch + ax;
;	docNext = doc;
;	only ax and dx are altered.
	mov	ax,1
	call	FLh92

;			 if (fPassChSpec)
;			     /*  we want to display this field character */
;			     {
;			     chpFLDxa.fBold = fTrue;
;			     fPassChSpec = fFalse;
;			     }
	errnz	<fFalse>
	xor	cx,cx
	xchg	bptr ([fPassChSpec]),cl
	jcxz	FL97
	or	[chpFLDxa.fBoldChp],maskFBoldChp

;			     /* we have revealed an unexpected table */
;			     if (vpapFetch.fInTable &&
;				     fInTable != FInTableVPapFetch(doc,cpNext))
;                            	{
;				fStopFormat = fTrue;
;				vfli.dcpDepend = 1;
;				}
;			     }
	;FLk2 performs:
	;if (vpapFetch.fInTable &&
	;	 fInTable != FInTableVPapFetch(doc,cpNext))
	;And clears the zero flag if both are true
	;ax, bx, cx, dx are altered.
	call	FLk2
	je	FL965
	mov	ax,1
	errnz	<fTrue-1>
	mov	bptr ([fStopFormat]),al
	mov	[vfli.dcpDependFli],ax
FL965:
	jmp	FLa8
FL97:

;			 else if (*hpch < chFieldMax && *hpch >= chFieldMin)
;			    /* Field Character */
;			    /*	*hpch is a special field deliminator which we
;				have not already looked at.
;			    */
;			    {

	;LN_SetEsSiFetch takes sets es:si according to hpch.
	;ax, bx, cx, si, es are altered.
	call	LN_SetEsSiFetch
	lods	bptr es:[si]
	cmp	al,chFieldMax
	jae	Ltemp043
	cmp	al,chFieldMin
	jae	Ltemp041
Ltemp043:
	jmp	FLa6	    ;assumes *hpch in al

;			     CP cpT = vcpFetch;
;			     int ffc;
;			     int dxpT = vfti.dxpOverhang;
;
;			     ch = *hpch;
;
;			     ffc = FfcFormatFieldPdcp (&dcpVanish,
;				      ww, doc, cpT, ch);
Ltemp041:
	mov	bx,[word ptr vcpFetch]
	mov	cx,[word ptr vcpFetch+2]
	mov	[OFF_cpT],bx
	mov	[SEG_cpT],cx
	mov	dx,[vfti.dxpOverhangFti]
	mov	[dxpT],dx

	lea	dx,[OFF_dcpVanish]
	xor	ah,ah
	mov	bptr ([chVar]),al
ifdef DEBUG
	cCall	S_FfcFormatFieldPdcp,<dx,[ww],[doc],cx,bx,ax>
else ;not DEBUG
	cCall	N_FfcFormatFieldPdcp,<dx,[ww],[doc],cx,bx,ax>
endif ;DEBUG
	push	ax	;save ffc

;			    /*	WARNING: above call will call FetchCp.
;				v*Fetch cannot be assumed to be anything
;				reasonable.
;			    */

;			    cpNext = cpT + dcpVanish;  /* cp next char */
;			    docNext = doc; /* vdocFetch may be invalid */
;			    hpchLim = hpch; /* force a fetch */
;PAUSE
	;FUTURE:  Can we remove the push and pop of ax around this
	;code and instead use cx in place of ax?
	mov	ax,[doc]
	mov	[docNext],ax
	mov	ax,[OFF_dcpVanish]
	mov	dx,[SEG_dcpVanish]
	add	ax,[OFF_cpT]
	adc	dx,[SEG_cpT]
	mov	[OFF_cpNext],ax
	mov	[SEG_cpNext],dx
	mov	bx,[OFF_hpch]
	mov	[OFF_hpchLim],bx

;			     /* order of ffc's and use of if for speed in
;				common cases. */
;			     if (ffc == ffcSkip)
;				 /*  Skip over dcpVanish characters */
;				 {
;				 /* restore previous para for LVanish's sake */
;				 CachePara(doc, cpT);
;				 goto LVanish;
;				 }
	pop	ax	;restore ffc
	errnz	<ffcSkip - 0>
	or	ax,ax
	jnz	FL978
	;FLh9 performs CachePara(doc, [bx]); ax,bx,cx,dx are altered.
	lea	bx,[cpT]
	call	FLh9
	jmp	LVanish

;			     /*  FfcFormat... may have cached a different para */
;				     CachePara (doc, cpNext);
FL978:
	;FLh8 performs CachePara(doc, cpNext); ax,bx,cx,dx are altered.
	push	ax	;save ffc
	call	FLh8
	pop	ax	;restore ffc

;			     if (ffc == ffcShow)
;				 /*  Don't skip anything.  Go around and
;				     do this character again, but next
;				     time don't call FfcFormatFieldPdcp.
;				 */
;				 {
;				 fPassChSpec = fTrue;
;				 goto LFetch;
;				 }
	errnz	<ffcShow - 1>
	dec	ax
	jnz	FL98
	mov	bptr ([fPassChSpec]),fTrue
	jmp	LFetch

;			     else if (ffc == ffcDisplay)
;				{
FL98:
	errnz	<ffcDisplay - 2>
	dec	ax
	jz	Ltemp011
	jmp	FLa2
;				int dxpT1;
;				/*  we have a display field.  generate a
;				    chrmDisplayField.
;				*/
;				dxpT1 = vfti.dxpOverhang;
;				vfti.dxpOverhang = dxpT;
;				Win(OverhangChr(&xp, bchrPrev,ich));
;				vfti.dxpOverhang = dxpT1;
;				bchrPrev = vbchrMac;
;				if ((vbchrMac += cbCHRDF) > vbchrMax)
;				    if (!FExpandGrpchr(cbCHRDF))
;					{
;LNoExpand:
;					vfli.dcpDepend = 0;
;					vfli.cpMac = cpT;
;					goto LEndBreakCp;
;					}
Ltemp011:
	push	[vfti.dxpOverhangFti]
	mov	ax,[dxpT]
	mov	[vfti.dxpOverhangFti],ax
	;LN_OverhangChr performs: adjust previous chr for overhang
	;also performs bchrPrev = vbchrMac.
	;ax, bx, cx, dx are altered.
	call	LN_OverhangChr
	pop	ax
	mov	[vfti.dxpOverhangFti],ax
	mov	bx,cbCHRDF
;	FLf4 performs;
;	vbchrMac+=bx;
;	return ((vbchrMac > vbchrMax) ? FExpandGrpchr(bx) : fTrue);
;	FLf4 returns zero flag set if the routine failed,
;	otherwise returns zero flag reset.
	call	FLf4
	jnz	FL99
LNoExpand:
	xor	ax,ax
	jmp	FLa15
FL99:

;				/*  set up CHRDF */
;				pchr = &(**vhgrpchr)[bchrPrev];
;				pchr->chrm = chrmDisplayField;
;				pchr->ich = ich;
;				((struct CHRDF *)pchr)->flt = vfmtss.flt;
;				((struct CHRDF *)pchr)->w = vfmtss.w;
;				((struct CHRDF *)pchr)->w2 = vfmtss.w2;
;				((struct CHRDF *)pchr)->l = vfmtss.l;
;				((struct CHRDF *)pchr)->dxp = vfmtss.dxp;
;				((struct CHRDF *)pchr)->dyp = vfmtss.dyp;
;				((struct CHRDF *)pchr)->dcp = dcpVanish;

	mov	al,chrmDisplayField
	;FLh0 performs bx = &(**vhgrpchr)[bchrPrev];
	;[bx.chrmChr] = al; [bx.ichChr] = ich;
	;only ax, bx, dx are altered.
	call	FLh0
	push	ds
	pop	es
	mov	si,dataoffset [vfmtss.fltFmtss]
	lea	di,[bx.fltChrdf]
	errnz	<(wChrdf) - (fltChrdf) - 2>
	errnz	<(wFmtss) - (fltFmtss) - 2>
	errnz	<(w2Chrdf) - (wChrdf) - 2>
	errnz	<(w2Fmtss) - (wFmtss) - 2>
	errnz	<(lChrdf) - (w2Chrdf) - 2>
	errnz	<(lFmtss) - (w2Fmtss) - 2>
	errnz	<(dxpChrdf) - (lChrdf) - 4>
	errnz	<(dxpFmtss) - (lFmtss) - 4>
	errnz	<(dypChrdf) - (dxpChrdf) - 2>
	errnz	<(dypFmtss) - (dxpFmtss) - 2>
	errnz	<(dcpChrdf) - (dypChrdf) - 2>
	mov	cx,((dcpChrdf) - (fltChrdf)) / 2
	rep	movsw
	lea	si,[dcpVanish]
	movsw
	movsw

;				/*  set up sizing information */
;				dxp = vfmtss.dxp;
;				dyyAscent.dyp = vfmtss.dyp - vfmtss.dyyDescent.dyp;
;				dyyAscent.dyt = vfmtss.dyt - vfmtss.dyyDescent.dyt;
;				dyyDescent = vfmtss.dyyDescent;
;				dxt = vfmtss.dxt;
;				/*  do main loop actions */
;				xp += dxp;
;				xt += dxt;
	
;				/* guarantee subsequent chrmChp */
;				bchrChp = -1;

;				/* a CHRDF has chDisplayField at ich */
;				vfli.rgch [ich] = chDisplayField;
;				vfli.rgdxp [ich++] = dxp;
	mov	[bchrChp],-1

	mov	bx,[ichVar]
	mov	[bx.vfli.rgchFli],chDisplayField
	shl	bx,1
	inc	[ichVar]
	
	lea	si,[vfmtss.dxpFmtss]
	lodsw
	mov	[dxp],ax
	mov	[bx.vfli.rgdxpFli],ax
	add	[xpVar],ax
	errnz	<(dypFmtss) - (dxpFmtss) - 2>
	lodsw
	xchg	ax,bx
	errnz	<(dxtFmtss) - (dypFmtss) - 2>
	lodsw
	mov	[dxt],ax     
	add	[xtVar],ax
	errnz	<(dytFmtss) - (dxtFmtss) - 2>
	lodsw
	xchg	ax,cx
	errnz	<(dypDescentFmtss) - (dytFmtss) - 2>
	lodsw
	mov	[dypDescent],ax
	sub	bx,ax
	mov	[dypAscent],bx
	errnz	<(dytDescentFmtss) - (dypDescentFmtss) - 2>
	lodsw
	mov	[dytDescent],ax
	sub	cx,ax
	mov	[dytAscent],cx

;				/* The following are duplicates of "default"
;				   character processing.  Here due to needed
;				   knowledge of the presence of CHRDF.
;				*/
;
;				/*  check for line overflow */
;				if (xt <= xtRight || ich == 1)
;				    /* we are ok or we broke through but it's
;				       the only thing on line--leave it */
;				    {
	mov	ax,[xtVar]
	cmp	ax,[xtRight]
	jle	FLa0
	cmp	[ichVar],1
	jne	FLa1
FLa0:
;				    /* update line height */
;				    if (dypDescentMac < dypDescent)
;					dypDescentMac = dypDescent;
;				    if (dypAscentMac < dypAscent)
;					dypAscentMac = dypAscent;
	;FLg1 performs
	;dypDescentMac = max(dypDescentMac, dypDescent);
	;dypAscentMac = max(dypAscentMac, dypAscent);
	; and the same for the dytAscent/Descent
	;ax and cx are altered.
	call	FLg1

;				    /* set up break opportunity to right*/
;				    vfli.chBreak = chDisplayField; /* REVIEW */
;				    vfli.cpMac = cpNext;
;				    vfli.xpRight = xp;
;				    vfli.ichMac = ich;
;				    bchrBreak = vbchrMac;
;				    dyyAscentBreak = dyyAscentMac;
;				    dyyDescentBreak = dyyDescentMac;
	mov	bx,chDisplayField
	mov	ax,[OFF_cpNext]
	mov	dx,[SEG_cpNext]
	;FLg5 performs
	;vfli.chBreak = bx
	;vfli.cpMac = dx:ax;
	;vfli.xpRight = xp;
	;vfli.ichMac = ich;
	;bchrBreak = vbchrMac;
	;dyyAscentBreak = dyyAscentMac;
	;dyyDescentBreak = dyyDescentMac;
	;only ax, dx are altered.
	call	FLg5

;				    continue;
	jmp	LTopOfLoop


;				    }

;				/* else went beyond right margin */
;				else  /* break to left of field */
;				    {
;				    /* undo "main loop" action */
;				    xp -= dxp;
;				    xt -= dxt;
;				    ich--;
;				    vfli.dcpDepend = 1;
;				    vbchrMac = bchrPrev;
;				    vfli.cpMac = cpT;

;				    goto LEndBreakCp;
;				    }

;				}  /* ffcDisplay */
FLa1:
	;FLf6 performs ich--; ax = (xt -= dxt); dx = (xp -= dxp);
	;only ax and dx are altered.
	call	FLf6
	mov	ax,[bchrPrev]
	mov	[vbchrMac],ax
	mov	ax,1
FLa15:
	mov	[vfli.dcpDependFli],ax
	mov	ax,[OFF_cpT]
	mov	dx,[SEG_cpT]
	jmp	LEndBreakCp	;expects new vfli.cpMac in dx:ax


;			     else if (ffc == ffcBeginGroup)
;				 {
FLa2:
	errnz	<ffcBeginGroup - 3>
	dec	ax
	jnz	FLa3

; 				OverhangChr(&xp, bchrPrev,ich);
;				 bchrPrev = vbchrMac;
;				 if ((vbchrMac += cbCHRFG) > vbchrMax)
;				     if (!FExpandGrpchr (cbCHRFG))
;					 {
;					 goto LNoExpand;
;					 }
	;LN_OverhangChr performs: adjust previous chr for overhang
	;also performs bchrPrev = vbchrMac.
	;ax, bx, cx, dx are altered.
	call	LN_OverhangChr
	mov	bx,cbCHRFG
;	FLf4 performs;
;	vbchrMac+=bx;
;	return ((vbchrMac > vbchrMax) ? FExpandGrpchr(bx) : fTrue);
;	FLf4 returns zero flag set if the routine failed,
;	otherwise returns zero flag reset.
	call	FLf4
	jnz	Ltemp013
	jmp	LNoExpand

;				 pchr = &(**vhgrpchr)[bchrPrev];
;				 ((struct CHRFG *)pchr)->chrm = chrmFormatGroup;
;				 ((struct CHRFG *)pchr)->ich = ich;
;				 ((struct CHRFG *)pchr)->dbchr = cbCHRFG;
;				 ((struct CHRFG *)pchr)->dcp = -cpT;
;				 ffs.flt = ((struct CHRFG *)pchr)->flt
;					 = vfmtss.flt;
;				 ffs.bchr = bchrPrev;
;				 ffs.xp = xp;
;				 ffs.fValidEnd = fTrue;
;				 goto LVanish;
;				 }
Ltemp013:
	mov	al,chrmFormatGroup
	;FLh0 performs bx = &(**vhgrpchr)[bchrPrev];
	;[bx.chrmChr] = al; [bx.ichChr] = ich;
	;only ax, bx, dx are altered.
	call	FLh0
	mov	[bx.dbchrChrfg],chrmFormatGroup
	xor	ax,ax
	cwd
	sub	ax,[OFF_cpT]
	sbb	dx,[SEG_cpT]
	mov	[bx.LO_dcpChrfg],ax
	mov	[bx.HI_dcpChrfg],dx
	mov	ax,[vfmtss.fltFmtss]
	mov	[bx.fltChrfg],ax
	mov	[ffs.fltFfs],ax
	mov	ax,[bchrPrev]
	mov	[ffs.bchrFfs],ax
	mov	ax,[xpVar]
	mov	[ffs.xpFfs],ax
	mov	[ffs.fValidEndFfs],fTrue
	jmp	LVanish

;
;			     else if (ffc == ffcEndGroup)
;				 {
FLa3:
	errnz	<ffcEndGroup - 4>
	dec	ax
	jnz	FLa4

;				 /*  must guarantee we have room */
;				 if ((vbchrMac + cbCHRV) > vbchrMax)
;					 {
;					 vbchrMac += cbCHRV;
;					 if (!FExpandGrpchr(cbCHRV))
;						 goto LEndBreak0;
;					 vbchrMac -= cbCHRV;
;					 }
	mov	bx,cbCHRV
;	FLf4 performs;
;	vbchrMac+=bx;
;	return ((vbchrMac > vbchrMax) ? FExpandGrpchr(bx) : fTrue);
;	FLf4 returns zero flag set if the routine failed,
;	otherwise returns zero flag reset.
	call	FLf4
	jnz	Ltemp005
	jmp	LEndBreak0
Ltemp005:
	sub	[vbchrMac],cbCHRV

;				 pchr = &(**vhgrpchr)[ffs.bchr];
;				 ((struct CHRFG *)pchr)->dxp = xp - ffs.xp;
;				 ((struct CHRFG *)pchr)->dbchr =
;					 vbchrMac - ffs.bchr + cbCHRV;
;				 ((struct CHRFG *)pchr)->dcp += cpT + 1;
;				 bchrChp = -1; /* guarantee chr to follow */
	mov	bx,[vhgrpchr]
	mov	bx,[bx]
	add	bx,[ffs.bchrFfs]
	mov	ax,[xpVar]
	sub	ax,[ffs.xpFfs]
	mov	[bx.dxpChrfg],ax
	mov	ax,[vbchrMac]
	sub	ax,[ffs.bchrFfs]
	add	ax,cbCHRV
	mov	[bx.dbchrChrfg],ax
	mov	ax,[OFF_cpT]
	mov	dx,[SEG_cpT]
	inc	ax
	adc	dx,0
	add	[bx.LO_dcpChrfg],ax
	adc	[bx.HI_dcpChrfg],dx
	mov	[bchrChp],-1

;
;				 /* set up break opportunity to right*/
;				 vfli.chBreak = chDisplayField; /* REVIEW */
;				 vfli.cpMac = cpNext;
;				 vfli.xpRight = xp;
;				 vfli.ichMac = ich;
;				 bchrBreak = vbchrMac + cbCHRV;
;				 dyyAscentBreak = dyyAscentMac;
;				 dyyDescentBreak = dyyDescentMac;
;
;				 goto LVanish;
;				 }
	mov	[vfli.chBreakFli],chDisplayField
	mov	ax,[OFF_cpNext]
	mov	dx,[SEG_cpNext]
	; new vfli.cpMac in dx:ax
	;FLg6 performs
	;vfli.cpMac = dx:ax;
	;vfli.xpRight = xp;
	;vfli.ichMac = ich;
	;bchrBreak = vbchrMac;
	;dyyAscentBreak = dyyAscentMac;
	;dyyDescentBreak = dyyDescentMac;
	;only ax, dx are altered.
	call	FLg6
	add	[bchrBreak],cbCHRV
	jmp	LVanish

;
;			     else if (ffc == ffcWrap)
;				 goto LEndBreak0;
FLa4:
	errnz	<ffcWrap - 5>
	dec	ax
	jnz	Ltemp015
	jmp	LEndBreak0

;
;			     else if (ffc == ffcRestart)
;				 goto LRestartLine;
;
;#ifdef DEBUG
;			     else
;				 Assert (fFalse);
;#endif /* DEBUG */
;
Ltemp015:
ifdef DEBUG
	errnz	<ffcRestart - 6>
	cmp	ax,1
	jz	FLa5
	mov	ax,midFormatn
	mov	bx, 629
	cCall	AssertProcForNative, <ax, bx>
FLa5:
endif
	jmp	LRestartLine

;			     }	/* Field Character */

;			 /* else non-field fSpec character */
FLa6:
;			else if (*hpch == chTFtn || *hpch == chTFtnCont)
;			     chpFLDxa.fStrike = fTrue;
;
	;assume *hpch in al
	cmp	al,chTFtn
	je	FLa7
	cmp	al,chTFtnCont
	jne	FLa8
FLa7:
	or	[chpFLDxa.fStrikeChp],maskFStrikeChp

FLa8:
;			}  /* fSpec Character */
FLa9:

;		if (vfli.fOutline)
;			 OutlineChp(doc, &chpFLDxa);
	cmp	[vfli.fOutlineFli],fFalse
	jz	FLb0
	lea	ax,[chpFLDxa]
	cCall	OutlineChp,<[doc], ax>
FLb0:

;		/*  Check if we need new chrmChp */
;		if (bchrChp == -1 ||
;/*	if previous font had overhang, always generate chrChp so we can adjust
;	for overhang from previous font, unless it's just a continuation
;	of a char run that will be one TextOut call in DisplayFliCore. */
;			(vfti.dxpOverhang != 0 && bchrChp != bchrPrev) ||
;			FNeChp(&chpFLDxa,
;			&(((struct CHR *)(&(**vhgrpchr)[bchrChp]))->chp)))
;			{
;			int hps;
	mov	bx,[bchrChp]
	cmp	bx,-1
	je	FLb1
	cmp	[vfti.dxpOverhangFti],0
	je	FLb1a
	cmp	bx,[bchrPrev]
	jne	FLb1
;	Perform FNeChp in line because this is the only place it is called.
;	***Begin in line FNeChp
FLb1a:
	lea	si,[chpFLDxa]
	mov	di,[vhgrpchr]
	mov	di,[di]
	lea	di,[di+bx+chpChr]
; {
;       if (((*pchp1++ ^ *pchp2++) & ~maskFs) != 0) return fTrue;
	lodsw
	xchg	bx,ax
	xchg	si,di
	lodsw
	xor	ax,bx
	and	ax,NOT maskFsChp
	jnz	FLb1

;       return FNeRgw(pchp1, pchp2, cwCHP - 1);
	mov	cx,cwCHP-1
	push	ds
	pop	es
	repz	cmpsw
	jnz	FLb1
; }
;	***End in line FNeChp
	jmp	LTopOfLoop

;			OverhangChr(&xp, bchrPrev,ich);
;			bchrPrev = vbchrMac;
;			Debug(vdbs.fShakeHeap ? ShakeHeap() : 0);
;			if ((vbchrMac += cbCHR) > vbchrMax &&
;					!FExpandGrpchr(cbCHR))
;				{
;				goto LEndBreak0;
;				}
;			else
;				{
FLb1:
	;LN_OverhangChr performs: adjust previous chr for overhang
	;also performs bchrPrev = vbchrMac.
	;ax, bx, cx, dx are altered.
	call	LN_OverhangChr
ifdef DEBUG
	cmp	[vdbs.fShakeHeapDbs],fFalse
	jz	FLb2
	mov	ax,1
	cCall	ShakeHeapSb,<ax>
FLb2:
endif
	mov	bx,cbCHR
;	FLf4 performs;
;	vbchrMac+=bx;
;	return ((vbchrMac > vbchrMax) ? FExpandGrpchr(bx) : fTrue);
;	FLf4 returns zero flag set if the routine failed,
;	otherwise returns zero flag reset.
	call	FLf4
	jnz	FLb3
	jmp	LEndBreak0
FLb3:

;				bchrChp = bchrPrev;
	mov	ax,[bchrPrev]
	mov	[bchrChp],ax

;/* transform looks if fRMark is on */
;/* play games with chpFLDxa if fRMark is on */
;				if (chpFLDxa.fRMark)
;					{
	test	[chpFLDxa.fRMarkChp],maskFRMarkChp
	jz	FLc0

;					struct DOD *pdod = PdodMother(doc);
;					int irmProps = pdod->dop.irmProps;
;
;					Assert(irmProps >= irmPropsNone && irmProps <= irmPropsDblUnder);
;					switch (irmProps)
;						{
;					case irmPropsBold:
;						chpFLDxa.fBold = fTrue;
;						break;
;					case irmPropsItalic:
;						chpFLDxa.fItalic = fTrue;
;						break;
;					case irmPropsUnder:
;						chpFLDxa.kul = kulSingle;
;						break;
;					case irmPropsDblUnder:
;						chpFLDxa.kul = kulDouble;
;						break;
;						}
;					}

	cCall	N_PdodMother,<[doc]>
	xchg	ax,si
	mov	bl,[si.dopDod.irmPropsDop]
	errnz	<maskIrmPropsDop - 07Fh>
	and	bx,maskIrmPropsDop
ifdef DEBUG
;	Assert (irmProps >= irmPropsNone && irmProps <= irmPropsDblUnder)
;	with a call so not to mess up short jumps.
	call	FLj6
endif ;DEBUG
	shl	bx,1
	jmp	wptr cs:([bx.FLb4])

FLb4:
	errnz	<irmPropsNone - 0>
	dw	FLc0
	errnz	<irmPropsBold - 1>
	dw	FLb5
	errnz	<irmPropsItalic - 2>
	dw	FLb6
	errnz	<irmPropsUnder - 3>
	dw	FLb7
	errnz	<irmPropsDblUnder - 4>
	dw	FLb8

FLb5:
	errnz	<(fBoldChp) - (fItalicChp)>
	mov	al,maskFBoldChp
	db	03Dh	;turns next "mov al,immediate" into "cmp ax,immediate"
FLb6:
	mov	al,maskFItalicChp
	or	[chpFLDxa.fItalicChp],al
	jmp	short FLc0
FLb7:
	mov	al,kulSingle SHL ibitKulChp
	db	03Dh	;turns next "mov al,immediate" into "cmp ax,immediate"
FLb8:
	mov	al,kulDouble SHL ibitKulChp
	and	[chpFLDxa.kulChp],NOT maskKulChp
	or	[chpFLDxa.kulChp],al

FLc0:

;				pchr = &(**vhgrpchr)[bchrChp];
;				blt(&chpFLDxa, &pchr->chp, cwCHP);
;				pchr->ich = ich;
;				pchr->chrm = chrmChp;
;				}
	mov	dx,[bchrChp]
	mov	al,chrmChp
	;FLh1 performs bx = &(**vhgrpchr)[dx];
	;[bx.chrmChr] = al; [bx.ichChr] = ich;
	;only ax, bx, dx are altered.
	call	FLh1
	lea	di,[bx.chpChr]
	push	ds
	pop	es
	mov	cx,cwCHP
	lea	si,[chpFLDxa]
	rep	movsw

FLc1:
;			   /* note LoadFont done even for special chars
;			      since vfti fields used later, and fcid from
;			      vfti is used in displayflicore.
;			   */
;
;			LoadFont( &chpFLDxa, fTrue );
	lea	ax,[chpFLDxa]
	push	ax
	push	ax
ifdef DEBUG
	cCall	S_LoadFont,<>
else ;not DEBUG
	call	far ptr N_LoadFont
endif ;DEBUG
;
;			if (chpFLDxa.fSpec)
;			    {
;			    vfli.rgch[ich] = *hpch++;
;				FormatChSpec(bchrPrev, ich, &dxp, &dxt,
;					&dyyAscent, &dyyDescent);
;			    if (!fDxt) dxt = dxp;
;			    }
	test	[chpFLDxa.fSpecChp],maskFSpecChp
	jz	FLc2
	;LN_SetEsSiFetch takes sets es:si according to hpch.
	;ax, bx, cx, si, es are altered.
	call	LN_SetEsSiFetch
	lods	bptr es:[si]
	mov	[OFF_hpch],si
	mov	si,[ichVar]
	mov	[si+vfli.rgchFli],al
	lea	ax,[dxp]
	lea	bx,[dxt]
	errnz	<OFFBP_dytAscent - OFFBP_dypAscent - 2>
	lea	cx,[dypAscent]
	errnz	<OFFBP_dytDescent - OFFBP_dypDescent - 2>
	lea	dx,[dypDescent]
	cCall	FormatChSpec,<[bchrPrev], si, ax, bx, cx, dx>
	;Assembler note: dxt is ignored if !fDxt in the assembler version.
	jmp	short FLc3
;			else
;			    {
;			    dyyDescent.dyp = vfti.dypDescent;
;			    dyyDescent.dyt = vftiDxt.dypDescent;
;			    dyyAscent.dyp = vfti.dypXtraAscent;
;			    dyyAscent.dyt = vftiDxt.dypXtraAscent;
;			    }
FLc2:
	errnz	<OFFBP_dytAscent - OFFBP_dypAscent - 2>
	errnz	<OFFBP_dypDescent - OFFBP_dytAscent - 2>
	errnz	<OFFBP_dytDescent - OFFBP_dypDescent - 2>
	push	ds
	pop	es
	lea	di,[dypAscent]
	mov	ax,[vfti.dypXtraAscentFti]
	stosw
	mov	ax,[vftiDxt.dypXtraAscentFti]
	stosw
	mov	ax,[vfti.dypDescentFti]
	stosw
	mov	ax,[vftiDxt.dypDescentFti]
	stosw

FLc3:
;			/* Store away the fcid of the actual font obtained */
;			/* so DisplayFli can request it directly */

;			((struct CHR *) &(**vhgrpchr)[bchrChp])->fcid = vfti.fcid;

	mov	bx,[vhgrpchr]
	mov	bx,[bx]
	add	bx,[bchrChp]
	mov	ax,[vfti.LO_fcidFti]
	mov	dx,[vfti.HI_fcidFti]
	mov	[bx.LO_fcidChr],ax
	mov	[bx.HI_fcidChr],dx

;			if (FVisiSpacesFli && !vfli.fPrint && vfti.fVisiBad)
;				vfli.fAdjustForVisi = fTrue;
;PAUSE
	test	bptr ([vfli.grpfvisiFli]),maskfvisiSpacesGrpfvisi OR maskfvisiShowAllGrpfvisi
	jz	FLc4
	cmp	[vfli.fPrintFli],fFalse
	jne	FLc4
	mov	al,[vfti.fVisiBadFti]
	and	al,maskfVisiBadFti
	or	[vfli.fAdjustForVisiFli],al
FLc4:

; /* see how this run will affect the line height */
;			if ((hps = chpFLDxa.hpsPos) != 0) /* modify font for sub/super */
;			    {
;			    int dypT;
	; hps will be kept in ax, never stored
	xor	ax,ax
	or	al,[chpFLDxa.hpsPosChp]
	jz	FLc5

;			    if (hps < 128)
;				{
;				dyyAscent.dyp += (dypT = NMultDiv( hps,
;					vfti.dypInch, 144 ));
;				/* dyyDescent.dyp = max(0,dyyDescent.dyp - dypT); */
;				dyyAscent.dyt += (dypT = NMultDiv( hps,
;					vftiDxt.dypInch, 144 ));
;				/* dyyDescent.dyt = max(0,dyyDescent.dyt - dypT); */
;				}
;			    else
;				{
;				dyyDescent.dyp += (dypT = NMultDiv((256-hps),
;					vfti.dypInch, 144) );
;				/* dyyAscent.dyp = max( 0, dyyAscent.dyp - dypT ); */
;				dyyDescent.dyt += (dypT = NMultDiv((256-hps),
;					vftiDxt.dypInch, 144) );
;				/* dyyAscent.dyt = max( 0, dyyAscent.dyt - dypT ); */
;				}
	; ax = hps

	;FLk5 performs:
	;if (al < 128)
	;    si += (dypT = NMultDiv(hps, dx, 144));
	;else
	;    di += (dypT = NMultDiv((256-hps), dx, 144));
	;ax, bx, cx, dx, si, di are altered, and cx upon return contains
	;the value passed in ax.
	mov	dx,[vfti.dypInchFti]
	mov	si,[dypAscent]
	mov	di,[dypDescent]
	call	FLk5
	mov	[dypDescent],di
	mov	[dypAscent],si

	xchg	ax,cx
	mov	dx,[vftiDxt.dypInchFti]
	mov	si,[dytAscent]
	mov	di,[dytDescent]
	call	FLk5
	mov	[dytDescent],di
	mov	[dytAscent],si

;			    }
;			fHeightPending = fTrue;
FLc5:
	or	bptr ([fPrevSpace]),maskFHeightPending

;
;			if (chpFLDxa.kul)
;				{
;				dyyDescent.dyp = max(dyyDescent.dyp,
;					chpFLDxa.kul == kulDouble ?
;					  3 : 1);
;				dyyDescent.dyt = max(dyyDescent.dyt,
;					chpFLDxa.kul == kulDouble ?
;					  3 : 1);
;				}

	mov	al,[chpFLDxa.kulChp]
	and	al,maskKulChp
	jz	FLc8
	mov	bx,3
	cmp	al,kulDouble SHL ibitKulChp
	jz	FLc6
	dec	bx
	dec	bx
FLc6:
	cmp	bx,[dypDescent]
	jle	FLc7
	mov	[dypDescent],bx
FLc7:
	cmp	bx,[dytDescent]
	jle	FLc8
	mov	[dytDescent],bx
FLc8:

;			 if (chpFLDxa.fSpec)
;				 {
;				 bchrChp = -1; /* so we force new chp */
;				 ch = 'a';	 /* this is bogus but safe */
;				 goto LHaveDxp;
;				 }
	test	[chpFLDxa.fSpecChp],maskFSpecChp
	jz	FLc9
	mov	[bchrChp],-1
	mov	bptr ([chVar]),'a'
	jmp	LHaveDxp

FLc9:
;		       } /* new chrmChp */
;
;		}	/* end for */
	jmp	LTopOfLoop
Ltemp030:
	jmp	LEndJ

; /* take break opportunity at the left of the char at xp, ich, pch */
; LEndBreak0:
;	vfli.dcpDepend = 0;

LEndBreak0:
	mov [vfli.dcpDependFli],0

; LEndBreak:
;	vfli.cpMac = (hpch - vhpchFetch) + vcpFetch;
LEndBreak:
	;FLf8 performs dx:ax = vcpFetch + (hpch - vhpchFetch)
	;only ax and dx are altered.
	call	FLf8

; LEndBreakCp:
; LEndBreakPara:
;	vfli.xpRight = xp;
;	vfli.ichMac = ich;
;	bchrBreak = vbchrMac;
;	dyyAscentBreak = dyyAscentMac;
;	dyyDescentBreak = dyyDescentMac;

LEndBreakCp:
	; new vfli.cpMac in dx:ax
	;FLg6 performs
	;vfli.cpMac = dx:ax;
	;vfli.xpRight = xp;
	;vfli.ichMac = ich;
	;bchrBreak = vbchrMac;
	;dyyAscentBreak = dyyAscentMac;
	;dyyDescentBreak = dyyDescentMac;
	;only ax, dx are altered.
	call	FLg6

;	pdodT = PdodDoc(vfli.doc);
;	if (!pdodT->fShort && (vfli.fPageView || vfli.fLayout) && !fInTable &&
;		!fAbs && ich < ichMaxLine && vfli.cpMac == caPara.cpLim)
;		{
	mov	bx,[vfli.docFli]
	mov	cx,bx
	;***Begin in line PdodDoc
	shl	bx,1
	mov	bx,[bx.mpdochdod]
	mov	bx,[bx]
	;***End in line PdodDoc
	cmp	[bx.fShortDod],fFalse
	jne	Ltemp030
	mov	al,[vfli.fPageViewFli]
	or	al,[vfli.fLayoutFli]
	je	Ltemp030
	cmp	[fInTable],fFalse	;checks fAbs also
	jne	Ltemp030
	errnz	<ichMaxLine - 255>
	cmp	bptr ([ichVar]),ichMaxLine
	je	Ltemp030
	mov	ax,[vfli.LO_cpMacFli]
	mov	dx,[vfli.HI_cpMacFli]
	cmp	ax,[caPara.LO_cpLimCa]
	jne	Ltemp030
	cmp	dx,[caPara.HI_cpLimCa]
	jne	Ltemp030

;		CP cpPara = caPara.cpFirst;
	mov	bx,[vfli.docFli]
	push	bx			;argument for CachePara
	push	[caPara.HI_cpFirstCa]	;argument for CachePara
	push	[caPara.LO_cpFirstCa]	;argument for CachePara

;		CacheSect(vfli.doc, vfli.cpMin);
	;LN_CacheSect performs
	;if (!FInCa(bx, di:si, &caSect)) CacheSect(bx, di:si);
	;ax, bx, cx, dx are altered.
	mov	di,[vfli.HI_cpMinFli]
	mov	si,[vfli.LO_cpMinFli]
	call	LN_CacheSect

;		if (vfli.cpMac == caSect.cpLim - 1 /* not ccpEop */
;			&& vfli.cpMac != CpMacDocEdit(vfli.doc)
;			&& !FInTableDocCp(vfli.doc, vfli.cpMac))
;			{
	cCall	CpMacDocEdit,<[vfli.docFli]>
	mov	bx,[vfli.LO_cpMacFli]
	mov	cx,[vfli.HI_cpMacFli]
	sub	ax,bx
	sbb	dx,cx
	or	ax,dx
	je	FLc97
	mov	ax,1
	cwd
	add	ax,bx
	adc	dx,cx
	mov	si,[caSect.LO_cpLimCa]
	mov	di,[caSect.HI_cpLimCa]
	sub	ax,si
	sbb	dx,di
	or	ax,dx
	jne	FLc97
	push	[vfli.docFli]
	push	cx
	push	bx
ifdef DEBUG
	cCall	S_FInTableDocCp,<>
else ;not DEBUG
	cCall	N_FInTableDocCp,<>
endif ;DEBUG
	or	ax,ax
	jne	FLc97

;			/* section mark in page view - no space in dr */
;			vfli.rgch[vfli.ichMac] = chSpace;
;			vfli.rgdxp[vfli.ichMac] = dxp = min(9, NMultDiv(xaRight, vfli.dxsInch, dxaInch) - vfli.xpRight);
;			vfli.ichMac++;
;			vfli.xpRight += dxp;
;			vfli.fSplatBreak = fTrue;
;			vfli.fParaAndSplat = fTrue;
;			vfli.cpMac = caSect.cpLim;
;			vfli.chBreak = chSect;
;			}
	mov	[vfli.LO_cpMacFli],si
	mov	[vfli.HI_cpMacFli],di
;	FLi05 performs NMultDiv(xaRight, vfli.dxsInch, dxaInch)
;	ax, bx, cx, dx are altered.
	call	FLi05
	sub	ax,[vfli.xpRightFli]
	cmp	ax,9
	jle	FLc95
	mov	ax,9
FLc95:
	mov	cl,chSpace
	;FLg85 performs
	;bx = vfli.ichMac;
	;vfli.rgch[vfli.ichMac] = cl;
	;vfli.rgdxp[vfli.ichMac++] = dxp = ax;
	;vfli.xpRight += dxp;
	;only bx is altered.
	call	FLg85
	errnz	<(fSplatBreakFli) - (fParaAndSplatFli)>
	or	[vfli.fSplatBreakFli],maskFSplatBreakFli+maskFParaAndSplatFli
	mov	[vfli.chBreakFli],chSect

;		}

;	CachePara(vfli.doc, cpPara); /* need same para for vpapFetch.dya* */
FLc97:
ifdef DEBUG
	cCall S_CachePara,<>
else ;not DEBUG
	cCall N_CachePara,<>
endif ;DEBUG

; LEndJ:
; /* justify (center, right flush) line and return */

;	dxp = max((fDxt ?
;		NMultDiv(xtRight, vfti.dxpInch, vftiDxt.dxpInch)
;		: xtRight) - xpJust, 0);
LEndJ:
	mov	ax,[xtRight]
	test	bptr ([fDxt]),080h
	jz	Fld0
;	FLi03 performs NMultDiv(ax, vfti.dxpInch, vftiDxt.dxpInch)
;	ax, bx, cx, dx are altered.
	call	FLi03
FLd0:
	sub	ax,[xpJust]
	jge	FLd2
	xor	ax,ax
FLd2:

;	switch (jc)
;		{
;	case jcCenter:
;		dxp >>= 1;
;		break;
;	case jcLeft:
;		goto LEndNoJ;
	mov	bl,bptr ([jcVar])
ifdef DEBUG
;	Assert (jc == jcLeft || jc == jcCenter || jc == jcRight
;	     || jc == jcBoth) with a call so not to mess up short jumps.
	call	FLj8
endif ;DEBUG
	errnz	<jcLeft - 0>
	or	bl,bl	    ; test first for left justify, the common case
	je	Ltemp044
	errnz	<jcCenter - 1>
	errnz	<jcRight - 2>
	errnz	<jcBoth - 3>
	cmp	bl,jcRight
	ja	FLd3
	je	Ltemp020
	shr	ax,1
Ltemp020:
	jmp	FLe1

; /* Following comment is preserved verbatim for eternity */
; /* Rounding becomes a non-existant issue due to brilliant re-thinking */
; /* "What a piece of work is man
;	How noble in reason
;	In form and movement,
;	how abject and admirable..."

;		 Bill "Shake" Spear [describing Sand Word] */

;	case jcBoth:
; /* special justification algorithms to be performed outside of FormatLine */
; /* ichSpace2 signals that justification is taking place, plus shows that
; spaces with ich>=ichSpace2 are wider */
;		vfli.ichSpace2 = vfli.ichSpace3;
; /* what are auto hyphenation requirements ? */
;		if (vfli.fSpecialJust)
;			goto LEndNoJ;
FLd3:
	mov	[dxp],ax
	mov	al,[vfli.ichSpace3Fli]
	mov	[vfli.ichSpace2Fli],al
	mov	si,[cchSpace]
	mov	bx,[ichSpace]
	cmp	[vfli.fSpecialJustFli],fFalse
	jz	LDivJust
Ltemp044:
	jmp	short LEndNoJ

FLd4:
;	Code relocated from below for size and speed.
	mov	bl,al
	dec	si

; LDivJust:
;		Mac( vfli.cchSpace = cchSpace );
;		if (cchSpace != 0)
;			{
;			int ddxp, dxpT = dxp / cchSpace;
;			vfli.dxpExtra = dxpT;
;			ddxp = dxpT + 1;
;			cchWide = dxp - dxpT * cchSpace;
	;expect bx = ichSpace, si = cchSpace
LDivJust:
	or	si,si
	jz	LJNoSpace
	mov	ax,[dxp]
	mov	cx,ax
	xor	dx,dx
	div	si
	mov	[vfli.dxpExtraFli],ax
	push	ax
	; ax = dxpT, bx = ichSpace, cx = dxp, si = cchSpace, [sp] = ddxp - 1
	mul	si
	neg	ax
	add	cx,ax
	pop	dx
	inc	dx
	; bx = ichSpace, cx = cchWide, dx = ddxp, si = cchSpace

;			if (fInhibitJust)
;				{ ddxp = 1; cchWide = 0; }
	cmp	bptr ([fInhibitJust]),fFalse
	jz	FLd5
	mov	dx,1
	xor	cx,cx
FLd5:
	; bx = ichSpace, cx = cchWide, dx = ddxp, si = cchSpace
;			for (ichT = ichSpace; ichT != ichNil; ichT = ichSpace)
;				{
	cmp	bx,ichNil
	je	FLd9
	; bx = ichT, cx = cchWide, dx = ddxp, si = cchSpace

; /* for all spaces (at ichT) do: */
;				ichSpace = vfli.rgch[ichT];
;				vfli.rgch[ichT] = FVisiSpacesFli ?
;					chVisSpace : chSpace;
	mov	al,chVisSpace
	test	bptr ([vfli.grpfvisiFli]),maskfvisiSpacesGrpfvisi OR maskfvisiShowAllGrpfvisi
	jnz	FLd6
	mov	al,chSpace
FLd6:
	xchg	[bx+vfli.rgchFli],al
	; al = ichSpace, bx = ichT, cx = cchWide, dx = ddxp, si = cchSpace

; /* ichT1 is next space, rgch is now correct */
;				if (ichT >= vfli.ichSpace3)
;					{
; /* exclude spaces above ichSpace3 from justification. */
;					cchSpace--;
;					goto LDivJust;
	cmp	bl,[vfli.ichSpace3Fli]
	jae	FLd4	   ; values may be > 128 so use unsigned compare

;					}
;				else
;					{
; /* last (first in list) cchWide spaces to be one longer than than rest */
;					if (cchWide-- == 0)
;						{
;						ddxp--;
;						/* ichT is last narrow */
;						vfli.ichSpace2 = ichT + 1;
;						}
	; al = ichSpace, bx = ichT, cx = cchWide, dx = ddxp, si = cchSpace
	jcxz	FLd8
FLd7:
	dec	cx
;					vfli.rgdxp[ichT] += ddxp;
	shl	bx,1
	add	[bx+vfli.rgdxpFli],dx

;					}
;				}	/* end for */
	xor	ah,ah
	xchg	ax,bx
	jmp	short FLd5

FLd8:
	dec	dx
	inc	bx
	mov	[vfli.ichSpace2Fli],bl
	dec	bx
	jmp	short FLd7

;			}
;		else
;			goto LJNoSpace;
FLd9:
;		if (!fInhibitJust)
;			vfli.xpRight += dxp;
;		else
; LJNoSpace:		  vfli.ichSpace2 = ichNil;
;		goto LEndNoJ;

	cmp	bptr ([fInhibitJust]),fFalse
	jnz	FLe0
	mov	ax,[dxp]
	add	[vfli.xpRightFli],ax
	jmp	short LEndNoJ
LJNoSpace:
FLe0:
	mov	[vfli.ichSpace2Fli],ichNil
	jmp	short LEndNoJ

; /*	  case jcRight: dxp = dxp */
;		}
FLe1:

; /* Note: xpLeft is always in points (screen units); xpRight, during printing,
;  is in device units. The justification dxp is in xt's only for printing */
;	vfli.xpLeft += dxp;
;	vfli.xpRight += dxp;
; LEndNoJ:

	add	[vfli.xpRightFli],ax
	add	[vfli.xpLeftFli],ax
LEndNoJ:

; /* set line height and return */
; LEnd:
;/* assure entire format group field was formatted */
;	 if (ffs.fFormatting)
;		 {
;		 struct CHR *pchrT = &(**vhgrpchr)[ffs.bchr];
;		 Assert (ffs.ifldWrap == ifldNil && ffs.ifldError == ifldNil);
;		 if (pchrT->ich != 0)
;		     /* force wrap */
;		     ffs.ifldWrap = ffs.ifldFormat;
;		 else
;		     ffs.ifldError = ffs.ifldFormat;
;		 goto LRestartLine;
;		 }
;	 vpffs = NULL;
LEnd:
	cmp	[ffs.fFormattingFfs],fFalse
	jz	FLe5
ifdef DEBUG
	cmp	[ffs.ifldWrapFfs],ifldNil
	jnz	FLe2
	cmp	[ffs.ifldErrorFfs],ifldNil
	jz	FLe3
FLe2:	mov	ax, midFormatn
	mov	bx, 150
	cCall	AssertProcForNative,<ax,bx>
FLe3:
endif ;DEBUG
	mov	bx,[vhgrpchr]
	mov	bx,[bx]
	add	bx,[ffs.bchrFfs]
	mov	ax,[ffs.ifldFormatFfs]
	cmp	[bx.ichChrfg],0
	jz	FLe4
	mov	[ffs.ifldWrapFfs],ax
	jmp	LRestartLine
FLe4:
	mov	[ffs.ifldErrorFfs],ax
	jmp	LRestartLine
FLe5:
	xor	ax,ax
	mov	[vpffs],ax

; /* caution due to possibility of a line consisting of non-printing chars
; that do not set the line height. */
;	if (dyyAscentBreak.dyp == 0)
;		{
;		if (dyyAscentMac.dyp == 0)
;			{
;			dyyDescentBreak = dyyDescent;
;			dyyAscentBreak = dyyAscent;
;			}
;		else
;			{
;			dyyDescentBreak = dyyDescentMac;
;			dyyAscentBreak = dyyAscentMac;
;			}
;		}
	cmp	[dypAscentBreak],ax
	jne	FLe6
	errnz	<(OFFBP_dytAscentMac) - (OFFBP_dypAscentMac) - 2>
	errnz	<(OFFBP_dypDescentMac) - (OFFBP_dytAscentMac) - 2>
	errnz	<(OFFBP_dytDescentMac) - (OFFBP_dypDescentMac) - 2>
	lea	si,[dypAscentMac]
	cmp	[si],ax
	jne	FLe55
	errnz	<(OFFBP_dytAscent) - (OFFBP_dypAscent) - 2>
	errnz	<(OFFBP_dypDescent) - (OFFBP_dytAscent) - 2>
	errnz	<(OFFBP_dytDescent) - (OFFBP_dypDescent) - 2>
	lea	si,[dypAscent]
FLe55:
	push	ds
	pop	es
	errnz	<(OFFBP_dytAscentBreak) - (OFFBP_dypAscentBreak) - 2>
	errnz	<(OFFBP_dypDescentBreak) - (OFFBP_dytAscentBreak) - 2>
	errnz	<(OFFBP_dytDescentBreak) - (OFFBP_dypDescentBreak) - 2>
	lea	di,[dypAscentBreak]
	movsw
	movsw
	movsw
	movsw
FLe6:


ifdef DEBUG
; %%Function:LLineHt %%Owner:BRADV
PUBLIC LLineHt
LLineHt:
endif

;	dytFont = 0;
;	if (vfli.fPageView || vlm == lmPreview)
;		{
;		dytFont = dyyAscentBreak.dyt + dyyDescentBreak.dyt;
;		vfli.dytLine = NMultDiv( abs(vpapFetch.dyaLine), vftiDxt.dypInch, czaInch );
;		}
;
	xor	di,di
	mov	ax,[vpapFetch.dyaLinePap]   ; tricky abs
	cwd
	xor	ax,dx
	sub	ax,dx
	push	ax			; save abs(vpapFetch.dyaLine)
	cmp	[vfli.fPageViewFli],fFalse
	jnz	FLe63
	cmp	[vlm],lmPreview
	jne	FLe67
FLe63:
;	FLi07 performs NMultDiv(ax, vftiDxt.dypInch, czaInch)
;	ax, bx, cx, dx are altered.
	call	FLi07
	mov	[vfli.dytLineFli],ax
	mov	di,[dytAscentBreak]
	add	di,[dytDescentBreak]	; di = dytFont
FLe67:

;	vfli.dypBefore = dypBefore;
;	vfli.dypFont = dyyAscentBreak.dyp + dyyDescentBreak.dyp;
;/* negative dyaLine means value is maximum line height; else min */
;	vfli.dypLine = DypFromDya(abs(vpapFetch.dyaLine));
;	vfli.dypBase = dyyDescentBreak.dyp;

	; di = dytFont

	mov	ax,[dypBefore]
	mov	[vfli.dypBeforeFli],ax
	mov	ax,[dypDescentBreak]
	mov	[vfli.dypBaseFli],ax
	add	ax,[dypAscentBreak]
	mov	[vfli.dypFontFli],ax
	xchg	ax,si		;save dypFont
	pop	ax			; restore abs(vpapFetch.dyaLine)
;	FLi1 performs DypFromDya(ax)
;	ax, bx, cx, dx are altered.
	call	FLi1
	mov	[vfli.dypLineFli],ax
	
;
;	if (vfli.fOutline)
;		{
;		vfli.dypLine = vfli.dypFont;
;		vfli.dytLine = dytFont;
;		}
;	else if (vpapFetch.dyaLine >= 0)
;		{
;		vfli.dypLine = max(vfli.dypLine, vfli.dypFont);
;		Win( vfli.dytLine = max( vfli.dytLine, dytFont ) );
;		}
;	else
;		{
;		vfli.dypFont = vfli.dypLine;
;		vfli.dypBase = vfli.dypFont / 5;    
;		}

	; di = dytFont, si = dypFont, ax = vfli.dypLine

	cmp	[vfli.fOutlineFli],fFalse
	jne	FLe85
	cmp	[vpapFetch.dyaLinePap],0
	jl	FLe9
	cmp	ax,si
	jge	FLe8
	mov	ax,si
FLe8:
	cmp	[vfli.dytLineFli],di
	jge	FLf0
	db	03Dh	;turns next "mov ax,si" into "cmp ax,immediate"
FLe85:
	mov	ax,si
	mov	[vfli.dytLineFli],di
	jmp	short FLf0

FLe9:
	mov	[vfli.dypFontFli],ax
	push	ax	;save vfli.dypLine
	cwd
	mov	bx,5
	div	bx
	mov	[vfli.dypBaseFli],ax
	pop	ax	;restore vfli.dypLine
;		}
FLf0:
	; ax = vfli.dypLine (not stored)

;        /* special case for fields & tables */
;        if (vfli.ichMac == 0)
;                vfli.dypLine = 0;

        cmp     [vfli.ichMacFli],0
        jne     FLf05
        xor     ax,ax
FLf05:

;	vfli.dypLine += vfli.dypBefore;
;	vfli.dytLine += NMultDiv( dypBefore, vftiDxt.dypInch, vfti.dypInch );
;	vfli.dypAfter = 0;

	add	ax,[vfli.dypBeforeFli]
	mov	[vfli.dypLineFli],ax
;	FLi0 performs NMultDiv(ax, vftiDxt.dypInch, vfti.dypInch)
;	ax, bx, cx, dx are altered.
	mov	ax,[dypBefore]
	call	FLi0
	add	[vfli.dytLineFli],ax
	mov	[vfli.dypAfterFli],0

;	if ((vfli.cpMac == caPara.cpLim || vfli.fParaAndSplat) && !vfli.fOutline)
;		{
;		int dyp;
;		int dyt;
	cmp	[vfli.fOutlineFli],fFalse
	jne	FLf2
	mov	ax,[caPara.LO_cpLimCa]
	mov	dx,[caPara.HI_cpLimCa]
	sub	ax,[vfli.LO_cpMacFli]
	sbb	dx,[vfli.HI_cpMacFli]
	or	ax,dx
	je	FLf07
	test	[vfli.fParaAndSplatFli],maskFParaAndSplatFli
	je	FLf2
FLf07:

;		dyt = NMultDiv( vpapFetch.dyaAfter, vftiDxt.dypInch, czaInch );
;		dyp = DypFromDya(vpapFetch.dyaAfter);
;		
;		if (vfli.fBottomEnable)
;			{
;			vfli.fBottom = fTrue;
;			dyt += NMultDiv( vfli.dypBrcBottom, vftiDxt.dypInch, vfti.dypInch );
;			dyp += vfli.dypBrcBottom;
;			}
;		vfli.dypLine += dyp;
;		vfli.dypBase += dyp;
;		vfli.dypAfter = dyp;
;		vfli.dytLine += dyt;
;		}

	mov	ax,[vpapFetch.dyaAfterPap]
	push	ax
;	FLi07 performs NMultDiv(ax, vftiDxt.dypInch, czaInch)
;	ax, bx, cx, dx are altered.
	call	FLi07
	xchg	ax,di
	pop	ax
;	FLi1 performs DypFromDya(ax)
;	ax, bx, cx, dx are altered.
	call	FLi1
	xchg	ax,si	  	; si = dyp, di = dyt
	test	[vfli.fBottomEnableFli],maskFBottomEnableFli
	jz	FLf1
	or	[vfli.fBottomFli],maskFBottomFli
;	FLi0 performs NMultDiv(ax, vftiDxt.dypInch, vfti.dypInch)
;	ax, bx, cx, dx are altered.
	mov	ax,[vfli.dypBrcBottomFli]
	call	FLi0
	add	di,ax
	add	si,[vfli.dypBrcBottomFli]
FLf1:
	add	[vfli.dypLineFli],si
	add	[vfli.dypBaseFli],si
	mov	[vfli.dypAfterFli],si
	add	[vfli.dytLineFli],di
FLf2:

;	if (vfli.fPageView && fDxt)
;		{
;/* do not round, to avoid pushing screen text to next DR */
;		int dypT = UMultDivNR( vfli.dytLine, vfti.dypInch, vftiDxt.dypInch );
	cmp	[vfli.fPageViewFli],fFalse
	jz	FLf25
	test	bptr ([fDxt]),080h
	jz	FLf25
	;***Begin in line UMultDivNR
	mov	ax,[vfli.dytLineFli]
	mul	[vfti.dypInchFti]
ifdef DEBUG
	cmp	dx,[vftiDxt.dypInchFti]
	jb	FLf23
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFormatn
	mov	bx,1032
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FLf23:
endif ;DEBUG
	div	[vftiDxt.dypInchFti]
	;***End in line UMultDivNR

;		/* if (vfli.dypLine < dypT) */
;			vfli.dypLine = dypT;
;		}		
;	cmp	ax,[vfli.dypLineFli]
;	jle	FLf25
	mov	[vfli.dypLineFli],ax
FLf25:

; /* reset pchr to break */
;	OverhangChr(&vfli.xpRight, bchrPrev,ich);
;	pchr = &(**vhgrpchr)[vbchrMac = bchrBreak];
; /* Now, enter chrmEnd in grpchr. Note: no need to check for sufficient space*/
;	pchr->chrm = chrmEnd;
;	pchr->ich = vfli.ichMac;

	mov	ax,[vfli.xpRightFli]
	mov	[xpVar],ax
	;LN_OverhangChr performs: adjust previous chr for overhang
	;also performs bchrPrev = vbchrMac.
	;ax, bx, cx, dx are altered.
	call	LN_OverhangChr
	mov	ax,[xpVar]
	mov     [vfli.xpRightFli],ax
	mov	dx,[bchrBreak]
	mov	[vbchrMac],dx
	mov	al,chrmEnd
	mov	ah,[vfli.ichMacFli]
	;FLh2 performs bx = &(**vhgrpchr)[dx];
	;[bx.chrmChr] = al; [bx.ichChr] = ah;
	;only ax, bx, dx are altered.
	call	FLh2

;	Scribble(ispFormatLine,' ');
;	Debug(vdbs.fCkFli ? CkVfli() : 0);
;	vrf.fInFormatLine = !vrf.fInFormatLine;
;	return;
ifdef DEBUG
	cmp	[vdbs.grpfScribbleDbs],0
	jz	FLf3
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,ispFormatLine
	mov	bx,020h
	cCall	ScribbleProc,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FLf3:
	; CkVfli call moved to FormatLineVerify
endif ;DEBUG
ifdef PROFILE
	cCall	StopNMeas,<>
endif ;PROFILE
	jmp	FL027
;}
;cEnd

; End of FormatLine

; /* O v e r h a n g  C h r */
; /* Called before entering new chr. If previous chr was a chrmChp with
;    an overhang, adjust width of final char to include the overhang. */
; /* This is to avoid clipping off the overhang when doing blanking on 
;    screens under WINDOWS */
; 
;OverhangChr(pxp, bchrPrev, ich)
;int *pxp;
;{
; struct CHR *pchr;

	;LN_OverhangChr performs: adjust previous chr for overhang
	;also performs bchrPrev = vbchrMac.
	;ax, bx, cx, dx are altered.
LN_OverhangChr:

; if (vbchrMac == 0)
;	 return;
	mov	cx,[vbchrMac]
	jcxz	OC02

; pchr = &(**vhgrpchr)[bchrPrev];
	;FLg9 performs bx = &(**vhgrpchr)[bchrPrev];
	;only ax, bx, dx are altered.
	call	FLg9

; if (!vfli.fPrint && pchr->chrm == chrmChp && ich > 0 && ich > pchr->ich)
;	 {
;	 vfli.rgdxp [ich - 1] += vfti.dxpOverhang;
;	 *pxp += vfti.dxpOverhang;
;	 }
;}
	cmp	[vfli.fPrintFli],fFalse
	jne	OC02
	errnz	<(ichChr) - (chrmChr) - 1>
	mov	ax,wptr [bx.chrmChr]
	cmp	al,chrmChp
	jne	OC02
	mov	bx,[ichVar]
	;Assert (ichVar <= 255)
ifdef DEBUG
	or	bh,bh
	je	OC01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFormatn
	mov	bx,1033
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
OC01:
endif ;DEBUG
	cmp	bl,ah
	jbe	OC02
	shl	bx,1
	mov	ax,[vfti.dxpOverhangFti]
	add	[bx.vfli.rgdxpFli-2],ax
	add	[xpVar],ax
OC02:
	mov	[bchrPrev],cx
	ret


;	FLf4 performs;
;	vbchrMac+=bx;
;	return ((vbchrMac > vbchrMax) ? FExpandGrpchr(bx) : fTrue);
;	FLf4 returns zero flag set if the routine failed,
;	otherwise returns zero flag reset.
FLf4:
	add	[vbchrMac],bx
	mov	ax,fTrue
	mov	cx,vbchrMax
	cmp	[vbchrMac],cx
	jle	FLf5
	cCall	FExpandGrpchr,<bx>
FLf5:
	or	ax,ax
	ret


	;FLf6 performs ich--; ax = (xt -= dxt); dx = (xp -= dxp);
	;only ax and dx are altered.
FLf6:
	dec	[ichVar]
	mov	ax,[xtVar]
	sub	ax,[dxt]
	mov	[xtVar],ax
	;FLf7 performs dx = (xp -= dxp);
	;only ax and dx are altered.
FLf7:
	mov	dx,[xpVar]
	sub	dx,[dxp]
	mov	[xpVar],dx
	ret


	;FLf8 performs dx:ax = vcpFetch + (hpch - vhpchFetch)
	;only ax and dx are altered.
FLf8:
	mov	ax,[OFF_hpch]
	sub	ax,wlo ([vhpchFetch])
	cwd
	add	ax,wlo ([vcpFetch])
	adc	dx,whi ([vcpFetch])
	ret


	;FLf9 performs ax = DxpFromCh(al,&vfti);
	;only ax and bx are altered.
FLf9:
	mov	bx,dataoffset [vfti]
	;FLg0 performs ax = DxpFromCh(al,bx);
	;only ax and bx are altered.
FLg0:
	; *** Begin in line DxpFromCh
	xor	ah,ah
	shl	ax,1
	push	[bx.dxpExpandedFti]
	add	bx,ax
	pop	ax
	add	ax,[bx.rgdxpFti]
	; *** End in line DxpFromCh
	ret


	;FLg1 performs
	;dypDescentMac = max(dypDescentMac, dypDescent);
	;dypAscentMac = max(dypAscentMac, dypAscent);
	; and the same for the dytAscent/Descent
	;ax and cx are altered.
FLg1:
	errnz	<(OFFBP_dytAscentMac) - (OFFBP_dypAscentMac) - 2>
	errnz	<(OFFBP_dypDescentMac) - (OFFBP_dytAscentMac) - 2>
	errnz	<(OFFBP_dytDescentMac) - (OFFBP_dypDescentMac) - 2>
	errnz	<(OFFBP_dytAscent) - (OFFBP_dypAscent) - 2>
	errnz	<(OFFBP_dypDescent) - (OFFBP_dytAscent) - 2>
	errnz	<(OFFBP_dytDescent) - (OFFBP_dypDescent) - 2>
	push	bp
	mov	cx,4
	lea	bp,[dypAscentMac]
FLg2:
	mov	ax,[bp.OFFBP_dypAscent - OFFBP_dypAscentMac]
	cmp	ax,[bp]
	jle	FLg3
	mov	[bp],ax
FLg3:
	inc	bp
	inc	bp
	loop	FLg2
	pop	bp
	ret


	;FLg4 performs
	;vfli.chBreak = chVar
	;vfli.cpMac = dx:ax;
	;vfli.xpRight = xp;
	;vfli.ichMac = ich;
	;bchrBreak = vbchrMac;
	;dyyAscentBreak = dyyAscentMac;
	;dyyDescentBreak = dyyDescentMac;
	;only ax, bx, dx are altered.
FLg4:
	mov	bx,[chVar]
	;FLg5 performs
	;vfli.chBreak = bx
	;vfli.cpMac = dx:ax;
	;vfli.xpRight = xp;
	;vfli.ichMac = ich;
	;bchrBreak = vbchrMac;
	;dyyAscentBreak = dyyAscentMac;
	;dyyDescentBreak = dyyDescentMac;
	;only ax, dx are altered.
FLg5:
	mov	[vfli.chBreakFli],bx
	;FLg6 performs
	;vfli.cpMac = dx:ax;
	;vfli.xpRight = xp;
	;vfli.ichMac = ich;
	;bchrBreak = vbchrMac;
	;dyyAscentBreak = dyyAscentMac;
	;dyyDescentBreak = dyyDescentMac;
	;only ax, dx are altered.
FLg6:
	;FLg8 performs
	;vfli.cpMac = dx:ax;
	;vfli.xpRight = xp;
	;vfli.ichMac = ich;
	;bchrBreak = vbchrMac;
	;only ax, dx are altered.
	call	FLg8
	;FLg7 performs
	;dyyAscentBreak = dyyAscentMac;
	;dyyDescentBreak = dyyDescentMac;
	;only ax is altered.
FLg7:
	errnz	<(OFFBP_dytAscentMac) - (OFFBP_dypAscentMac) - 2>
	errnz	<(OFFBP_dypDescentMac) - (OFFBP_dytAscentMac) - 2>
	errnz	<(OFFBP_dytDescentMac) - (OFFBP_dypDescentMac) - 2>
	errnz	<(OFFBP_dytAscentBreak) - (OFFBP_dypAscentBreak) - 2>
	errnz	<(OFFBP_dypDescentBreak) - (OFFBP_dytAscentBreak) - 2>
	errnz	<(OFFBP_dytDescentBreak) - (OFFBP_dypDescentBreak) - 2>
	push	si
	push	di
	push	ds
	pop	es
	lea	si,[dypAscentMac]
	lea	di,[dypAscentBreak]
	movsw
	movsw
	movsw
	movsw
	pop	di
	pop	si
	ret


	;FLg8 performs
	;vfli.cpMac = dx:ax;
	;vfli.xpRight = xp;
	;vfli.ichMac = ich;
	;bchrBreak = vbchrMac;
	;only ax, dx are altered.
FLg8:
	mov	[vfli.LO_cpMacFli],ax
	mov	[vfli.HI_cpMacFli],dx
	mov	ax,[xpVar]
	mov	[vfli.xpRightFli],ax
	mov	ax,[ichVar]
	mov	[vfli.ichMacFli],al
	mov	ax,[vbchrMac]
	mov	[bchrBreak],ax
	ret


	;FLg85 performs
	;bx = vfli.ichMac;
	;vfli.rgch[vfli.ichMac] = cl;
	;vfli.rgdxp[vfli.ichMac++] = dxp = ax;
	;vfli.xpRight += dxp;
	;only bx is altered.
FLg85:
	xor	bx,bx
	mov	bl,[vfli.ichMacFli]
	inc	[vfli.ichMacFli]
	mov	[bx.vfli.rgchFli],cl
	mov	[dxp],ax
	shl	bx,1
	mov	[bx.vfli.rgdxpFli],ax
	add	[vfli.xpRightFli],ax
	shr	bx,1
	ret


	;FLg9 performs bx = &(**vhgrpchr)[bchrPrev];
	;only ax, bx, dx are altered.
FLg9:
	xor	ax,ax
	;FLh0 performs bx = &(**vhgrpchr)[bchrPrev];
	;[bx.chrmChr] = al; [bx.ichChr] = ich;
	;only ax, bx, dx are altered.
FLh0:
	mov	dx,[bchrPrev]
	;FLh1 performs bx = &(**vhgrpchr)[dx];
	;[bx.chrmChr] = al; [bx.ichChr] = ich;
	;only ax, bx, dx are altered.
FLh1:
	mov	ah,bptr ([ichVar])
	;FLh2 performs bx = &(**vhgrpchr)[dx];
	;[bx.chrmChr] = al; [bx.ichChr] = ah;
	;only ax, bx, dx are altered.
FLh2:
	mov	bx,[vhgrpchr]
	mov	bx,[bx]
	add	bx,dx
	or	al,al
	je	FLh3
	errnz	<(chrmChr) - 0>
	errnz	<(ichChr) - 1>
	mov	[bx],ax
FLh3:
	ret


	;FLh4 performs
	;xpJust = xp - dxp;
	;xtJust = xt - dxt;
	;vfli.ichSpace3 = ich;
	;only ax and dx are altered.
FLh4:
	db	0A8h	;turns next "stc" into "test al,immediate"
			;also clears the carry flag
	;FLh5 performs
	;xpJust = xp;
	;xtJust = xt;
	;vfli.ichSpace3 = ich;
	;only ax and dx are altered.
FLh5:
	stc
	mov	ax,[xpVar]
	mov	dx,[xtVar]
	jc	FLh6
	sub	ax,[dxp]
	sub	dx,[dxt]
FLh6:
	mov	[xpJust],ax
	mov	[xtJust],dx
	mov	ax,[ichVar]
	mov	[vfli.ichSpace3Fli],al
	ret


	;FLh7 performs CachePara(doc, cpCurPara); ax,bx,cx,dx are altered.
FLh7:
	lea	bx,[cpCurPara]
	jmp	short FLh9

	;FLh8 performs CachePara(doc, cpNext); ax,bx,cx,dx are altered.
FLh8:
	lea	bx,[cpNext]

	;FLh9 performs CachePara(doc, [bx]); ax,bx,cx,dx are altered.
FLh9:
	push	[doc]
	push	[bx+2]
	push	[bx]
ifdef DEBUG
	cCall S_CachePara,<>
else ;not DEBUG
	cCall N_CachePara,<>
endif ;DEBUG
	ret


;	FLh92 performs:
;	hpchLim = hpch + ax;
;	cpNext = vcpFetch + ax;
;	docNext = doc;
;	only ax and dx are altered.
FLh92:
	push	ax
	add	ax,[OFF_hpch]
	mov	[OFF_hpchLim],ax
	pop	ax
	mov	dx,[doc]
	mov	[docNext],dx
	cwd
;	FLh98 performs:
;	cpNext = vcpFetch + dx:ax;
;	only ax and dx are altered.
FLh98:
	add	ax,[word ptr vcpFetch]
	adc	dx,[word ptr vcpFetch+2]
	mov	[OFF_cpNext],ax
	mov	[SEG_cpNext],dx
	ret


;	FLi0 performs NMultDiv(ax, vftiDxt.dypInch, vfti.dypInch)
;	ax, bx, cx, dx are altered.
FLi0:
	mov	bx,[vfti.dypInchFti]
	mov	dx,[vftiDxt.dypInchFti]
	jmp	short LN_NMultDiv

;	FLi03 performs NMultDiv(ax, vfti.dxpInch, vftiDxt.dxpInch)
;	ax, bx, cx, dx are altered.
FLi03:
	mov	bx,[vftiDxt.dxpInchFti]
	mov	dx,[vfti.dxpInchFti]
	jmp	short LN_NMultDiv

;	FLi05 performs NMultDiv(xaRight, vfli.dxsInch, dxaInch)
;	ax, bx, cx, dx are altered.
FLi05:
	mov	ax,[xaRight]
	mov	dx,[vfli.dxsInchFli]
	jmp	short FLi3

;	FLi07 performs NMultDiv(ax, vftiDxt.dypInch, czaInch)
;	ax, bx, cx, dx are altered.
FLi07:
	mov	dx,[vftiDxt.dypInchFti]
	jmp	short FLi2

;	FLi1 performs DypFromDya(ax)
;	ax, bx, cx, dx are altered.
;#define DypFromDya(dya) NMultDiv((dya), vfti.dypInch, czaInch)
FLi1:
	mov	dx,[vfti.dypInchFti]

;	FLi2 performs NMultDiv(ax, dx, czaInch)
;	ax, bx, cx, dx are altered.
FLi2:
	mov	bx,czaInch
	jmp	short LN_NMultDiv

LN_XtFromXa:
;	LN_XtFromXa performs XtFromXa(ax)
;	ax, bx, cx, dx are altered.
;#define XtFromXa(xa)		 NMultDiv((xa), vftiDxt.dxpInch, dxaInch)
	mov	dx,[vftiDxt.dxpInchFti]
FLi3:
;	FLi3 performs NMultDiv(ax, dx, dxaInch)
;	ax, bx, cx, dx are altered.
	mov	bx,dxaInch

; %%Function:LN_NMultDiv %%Owner:BRADV
PUBLIC LN_NMultDiv
LN_NMultDiv:
;	LN_NMultDiv performs NMultDiv(ax, dx, bx)
;	ax, bx, cx, dx are altered.
ifdef DEBUG
	or	bx,bx
	jnz	LN_NMD01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFormatn
	mov	bx,1030
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LN_NMD01:
endif ;DEBUG
	imul	dx
	push	bx	;save divisor
	shr	bx,1
	xor	cx,cx
	or	dx,dx
	jns	LN_NMD015
	neg	bx
	dec	cx
LN_NMD015:
	add	ax,bx
	adc	dx,cx
	pop	bx	;restore divisor
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	shr	bx,1
	or	dx,dx
	jns	LN_NMD02
	neg	dx
LN_NMD02:
	or	bx,bx
	js	LN_NMD03
	cmp	dx,bx
	jb	LN_NMD04
LN_NMD03:
	mov	ax,midFormatn
	mov	bx,1031
	cCall	AssertProcForNative,<ax,bx>
LN_NMD04:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG
	idiv	 bx
	ret


ifdef DEBUG
FLi5:
;	Assert (!PdodDoc(doc)->fShort);
	push	ax
	push	bx
	push	cx
	push	dx
	mov	bx,[doc]
	shl	bx,1
	mov	bx,[bx.mpdochdod]
	mov	bx,[bx]
	cmp	[bx.fShortDod],fFalse
	je	FLi6
	mov	ax,midFormatn
	mov	bx,1005
	cCall	AssertProcForNative,<ax,bx>
	mov	[vfli.fOutlineFli],fFalse
FLi6:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG

;			Assert ((uns) ffs.ifldError > ffs.ifldFormat);
ifdef DEBUG
FLi7:
	mov	ax,[ffs.ifldFormatFfs]
	cmp	ax,[ffs.ifldErrorFfs]
	jb	FLi8
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFormatn
	mov	bx,606
	cCall	AssertProcForNative, <ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FLi8:
	ret
endif ;DEBUG

ifdef REVIEW
ifdef DEBUG
;	Assert (cpNext < CpMacDoc (doc));
FLi9:
	cCall	CpMacDoc,<doc>
	sub	ax,[OFF_cpNext]
	sbb	dx,[SEG_cpNext]
	jl	FLj0
	or	ax,dx
	jnz	FLj1
FLj0:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFormatn
	mov	bx,116
	cCall	AssertProcForNative, <ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FLj1:
	ret
endif ; DEBUG
endif ; REVIEW

ifdef DEBUG
FLj2:
	;Assert (di == &vfli.rgch[ich+1]);
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[ichVar]
	add	ax,dataoffset [vfli.rgchFli]
	inc	ax
	cmp	ax,di
	je	FLj3
	mov	ax,midFormatn
	mov	bx,116
	cCall	AssertProcForNative, <ax, bx>
FLj3:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ; DEBUG

ifdef DEBUG
FLj4:
	;Assert (al != chNonBreakSpace || vifmaMac != 0);
	cmp	al,chNonBreakSpace
	jne	FLj5
	cmp	[vifmaMac],0
	jne	FLj5
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFormatn
	mov	bx,1040
	cCall	AssertProcForNative, <ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FLj5:
	ret
endif ; DEBUG

ifdef DEBUG
FLj6:
	;Assert (irmProps >= irmPropsNone && irmProps <= irmPropsDblUnder);
	errnz	<irmPropsNone - 0>
	cmp	bx,irmPropsDblUnder
	jbe	FLj7
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFormatn
	mov	bx,1041
	cCall	AssertProcForNative, <ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FLj7:
	ret
endif ;DEBUG


ifdef DEBUG
;	Assert (jc == jcLeft || jc == jcCenter || jc == jcRight || jc == jcBoth);
FLj8:
	cmp	bl,jcLeft
	je	FLj9
	cmp	bl,jcCenter
	je	FLj9
	cmp	bl,jcRight
	je	FLj9
	cmp	bl,jcBoth
	je	FLj9
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFormatn
	mov	bx,1042
	cCall	AssertProcForNative, <ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FLj9:
	ret
endif ;DEBUG


ifdef DEBUG
;	Assert (ch & 0xFF00 == 0);
FLk0:
	or	ah,ah
	je	FLk1
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFormatn
	mov	bx,1043
	cCall	AssertProcForNative, <ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FLk1:
	ret
endif ;DEBUG


	;FLk2 performs:
	;if (vpapFetch.fInTable &&
	;	 fInTable != FInTableVPapFetch(doc,cpNext))
	;And clears the zero flag if both are true
	;ax, bx, cx, dx are altered.
FLk2:
;#define FInTableVPapFetch(doc, cp)  (vpapFetch.fInTable && FInTableDocCp(doc, cp))
	cmp	[vpapFetch.fInTablePap],fFalse
	je	FLk4
ifdef DEBUG
	cCall	S_FInTableDocCp,<[doc],[SEG_cpNext],[OFF_cpNext]>
else ;not DEBUG
	cCall	N_FInTableDocCp,<[doc],[SEG_cpNext],[OFF_cpNext]>
endif ;DEBUG
	or	al,ah
ifdef DEBUG
	;Assert (al == 0 || fInTable == 0 || al == fInTable);
	or	al,al
	je	FLk3
	cmp	bptr ([fInTable]),fFalse
	je	FLk3
	cmp	bptr ([fInTable]),al
	je	FLk3
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFormatn
	mov	bx,1050
	cCall	AssertProcForNative, <ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FLk3:
endif ; DEBUG
	cmp	al,bptr ([fInTable])
FLk4:
	ret


	;FLk5 performs:
	;if (al < 128)
	;    si += (dypT = NMultDiv(hps, dx, 144));
	;else
	;    di += (dypT = NMultDiv((256-hps), dx, 144));
	;ax, bx, cx, dx, si, di are altered, and cx upon return contains
	;the value passed in ax.
FLk5:
	; ax = hps
	; flags set from or al,al
	or	al,al
	push	ax	;save hps
	jge	FLk6
	neg	al
	cbw
	xchg	si,di
FLk6:
;	LN_NMultDiv performs NMultDiv(ax, dx, bx)
;	ax, bx, cx, dx are altered.
	mov	bx,144
	call	LN_NMultDiv
	pop	cx	;restore hps
	add	si,ax
	or	cl,cl
	jge	FLk8
	xchg	si,di
FLk8:
	ret


ifdef DEBUG
FLk9:
	;Assert (vfli.fLayout == (vfli.fLayout & maskFSplatBreakFli));
	push	ax
	push	bx
	push	cx
	push	dx
	mov	al,[vfli.fLayoutFli]
	and	al,maskFSplatBreakFli
	cmp	al,[vfli.fLayoutFli]
	je	FLl0
	mov	ax,midFormatn
	mov	bx,1044
	cCall	AssertProcForNative, <ax, bx>
FLl0:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ; DEBUG

; /* Given char, ptr to fti, return width of char */

; /* D X P  F R O M  C H */
; native int DxpFromCh( ch, pfti )
; int ch;
; struct FTI *pfti;

; %%Function:DxpFromCh %%Owner:BRADV
PUBLIC	DxpFromCh
DxpFromCh:
; {
	pop	cx
	pop	dx	;far return address in dx:cx
	pop	bx
	pop	ax
	push	dx
	push	cx
	;FLg0 performs ax = DxpFromCh(al,bx);
	;only ax and bx are altered.
	call	FLg0
; }
	db	0CBh	    ;far ret


;-------------------------------------------------------------------------
;	LN_SetEsSiFetch
;-------------------------------------------------------------------------
	;LN_SetEsSiFetch takes sets es:si according to hpch.
	;ax, bx, cx, si, es are altered.
; %%Function:LN_SetEsSiFetch %%Owner:BRADV
cProc	LN_SetEsSiFetch,<NEAR,ATOMIC>,<>

cBegin	nogen
	mov	bx,[SEG_hpch]
	shl	bx,1
	mov	ax,[bx.mpsbps]
	mov	es,ax
	shr	ax,1
	jc	LN_RS01
;	ReloadSb trashes ax, cx, and dx
	push	dx
	cCall	ReloadSb,<>
	pop	dx
LN_RS01:
ifdef DEBUG
	mov	ax,[wFillBlock]
	mov	cx,[wFillBlock]
endif ;DEBUG
	mov	si,[OFF_hpch]
cEnd	nogen
	ret


	;LN_CacheSect performs
	;if (!FInCa(bx, di:si, &caSect)) CacheSect(bx, di:si);
	;ax, bx, cx, dx are altered.
LN_CacheSect:
	mov	ax,dataoffset [caSect]
	push	bx	;save doc
	cCall	FInCa,<bx,di,si,ax>
	pop	bx	;restore doc
	or	ax,ax
	jne	LN_CS01
	cCall	CacheSectProc,<bx,di,si>
LN_CS01:
	ret

ifdef DEBUG ; Done in line if not debug
;-----------------------------------------------------------------------------
; UMultDivNR(w, Numer, Denom) returns (w * Numer) / Denom NOT rounded
; A check is made so that division by zero is not attempted.
; This is for unsigned numbers 
;-----------------------------------------------------------------------------

; %%Function: UMultDivNR %%Owner:BRADV
cProc UMultDivNR, <FAR, PUBLIC,ATOMIC>
parmW  <w, Numer, Denom>
        ;/* **** -+-utility-+- **** */

cBegin UMultDiv
    mov     bx,Denom    ; bx = Denom

    mov     ax,w        
    mov     dx,Numer
    mul     dx          ; dx:ax = w * Numer

; Overflow checks:      (1) Denom == 0
;                       (2) result will be > 0xFFFF
;                       (div instruction generates div-by-0 in this case too)
; Following sequence catches both these conditions by this method:
;   quotient = numer/denom - rem/denom =>  numer = quotient * denom + rem
;   numer is in dx:ax. if quotient is < 10000h (i.e., will fit in 16 bits)
;   then  numer/denom - rem/denom < 10000h and rem/denom is always 0 in integer math.
;   so dx:ax < denom * 10000h. Since multiplying by 10000h is a 16 bit shift,
;   if dx < denom (bx) we are ok.
;    got that? took me a while (bz)

    cmp     dx,bx
    jae     umdOv

    div     bx          ; ax = dx:ax / bx, dx := dx:ax % bx
umdEnd:
cEnd UMultDiv

umdOv:                  ; Overflow, return 0xFFFF
    mov     ax,0FFFFh
    jmp     umdEnd
endif ;DEBUG


ifdef	DEBUG
LN_FreezeHp:
;#define FreezeHp()		 (cHpFreeze++?0:LockHeap(sbDds))
	cmp	[cHpFreeze],0
	jne	LN_FH01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,sbDds
	cCall	LockHeap,<ax>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LN_FH01:
	inc	[cHpFreeze]
	ret
endif ;DEBUG


ifdef	DEBUG
LN_MeltHp:
;#define MeltHp()		 (--cHpFreeze?0:UnlockHeap(sbDds))
	dec	[cHpFreeze]
	jne	LN_MH01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,sbDds
	cCall	UnlockHeap,<ax>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LN_MH01:
	ret
endif ;DEBUG

sEnd	format
	end
