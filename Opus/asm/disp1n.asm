        include w2.inc
        include noxport.inc
        include consts.inc
	include structs.inc
	include windows.inc

createSeg	disp1_PCODE,disp1,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midDisp1n	equ 10		; module ID, for native asserts
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
; EXTERNAL FUNCTIONS

externFP	<N_PwwdWw>
externFP	<EndUL>
externFP	<DrawTlc>
externFP	<NMultDiv>
externFP	<N_LoadFcidFull>
ifdef HYBRID
externFP	<MoveToPR>
externFP	<LineToPR>
else ;!HYBRID
externFP	<MoveTo>
externFP	<LineTo>
endif ;HYBRID
externFP	<ShowSpec>
externFP	<DrawDottedLineBox>
externFP	<DisplayField>
externFP	<SetBkMode>
ifdef DEBUG
externFP	<IntersectClipRectFP>
externFP	<SaveDCFP>
externFP	<PatBltFP>
externFP	<RestoreDCFP>
externFP	<SelectObjectFP>
externFP	<ExtTextOutFP>
else ;!DEBUG
externFP	<IntersectClipRect>
ifdef HYBRID
externFP	<SaveDCPR>
externFP	<PatBltPR>
externFP	<RestoreDCPR>
externFP	<SelectObjectPR>
externFP	<ExtTextOutPR>
else ;!HYBRID
externFP	<SaveDC>
externFP	<PatBlt>
externFP	<RestoreDC>
externFP	<SelectObject>
externFP	<ExtTextOut>
endif ;HYBRID
endif ;!DEBUG
externFP	<ExtTextOutKludge>
externFP	<PstFromSttb>
externFP	<DrawPatternLine>
externFP	<PatBltRc>
externFP	<ForeColor>
externFP	<N_PtOrigin>
externFP	<ClipRectFromDr>
externFP	<DrawBorders>
externFP	<DrcpToRcw>
externFP	<CacheSectProc>
externFP	<FInCa>
externFP	<CachePage>
externFP	<FrameDrLine>
externFP	<DxpOutlineSplats>
externFP	<DrawStyNameFromWwDl>
externFP	<N_PdodMother>
externFP	<PutPlc>
externFP	<GetPlc>
externFP	<DrclToRcw>
externFP	<PutCpPlc>
externFP	<N_AddVisiSpaces>
externFP	<MarkSel>
externFP	<DrawChPubs>
externFP	<N_PdrFetch>
externFP	<N_FreePdrf>
externFP	<N_FormatLineDxa>
externFP	<DrawPrvwLine>
externFP	<CpFirstTap1>
externFP	<N_WidthHeightFromBrc>
externFP	<YwFromYl>
externFP	<AdjustOpaqueRect>
externFP	<DrawFormulaLine>
externFP	<FFacingPages>
externFP	<DocMother>
externFP	<PrintUlLine>

ifdef DEBUG
externFP	<AssertProcForNative>
externFP	<S_LoadFcidFull>
externFP	<ScribbleProc>
externFP	<S_DisplayFliCore>
externFP	<S_AddVisiSpaces>
externFP	<S_PdrFetch>
externFP	<S_FreePdrf>
externFP	<S_PtOrigin>
externFP	<S_FormatLineDxa>
externFP	<S_WidthHeightFromBrc>
externFP	<LockHeap, UnlockHeap>
endif ;DEBUG

; EXPORTED LABELS

ifdef DEBUG
PUBLIC  LN_EndULPast
PUBLIC  DFC265
endif ;DEBUG


sBegin	data

; 
; /* E X T E R N A L S */
; 
externW 	vhgrpchr	; extern struct CHR	  **vhgrpchr;
externW         selCur          ; extern struct SEL       selCur;
externW 	vsci		; extern struct SCI	  vsci;
externW 	vfli		; extern struct FLI	  vfli;
externW 	vfti		; extern struct FTI	  vfti;
externW 	vpri		; extern struct PRI	  vpri;
externW 	vfmtss		; extern struct FMTSS	  vfmtss;
externW 	vpdcibDisplayFli; extern struct DCIB	  vpdcibDisplayFli;
externW 	selDotted	; extern struct SEL	  selDotted;
externW 	vhwwdOrigin	; extern struct WWD	  **vhwwdOrigin;
externW 	vpref		; extern struct PREF	  vpref;
externW 	vhwndApp	; extern HWND		  vhwndApp;
externW 	wwCur		; extern int		  wwCur;
externW 	caPage		; extern struct CA	  caPage;
externW 	caSect		; extern struct CA	  caSect;
externW 	vdrdlDisplayFli ; extern struct DRDL	  vdrdlDisplayFli;
externW 	mpdochdod	; extern struct DOD	  **mpdochdod[];
externW 	vfFliMunged	; extern BOOL		  vfFliMunged;
externW 	vlm		; extern int		  vlm;
externW		vfPrvwDisp	; extern int		  vfPrvwDisp;
externW 	caTap		; extern struct CA	  caTap;
externW 	vtapFetch	; extern struct TAP	  vtapFetch;
externW 	vrf		; extern struct RF	  vrf;
externW 	vwWinVersion	; extern int		  vwWinVersion;
externW 	vhsttbFont	; extern struct STTB **	  vhsttbFont;

; #ifdef DEBUG
ifdef DEBUG
externW 	cHpFreeze	; extern int		  cHpFreeze;
externW 	vdbs		; extern struct DBS	  vdbs;
; #endif /* DEBUG */
endif

sEnd    data


; CODE SEGMENT _DISP1

sBegin	disp1
	assumes cs,disp1
        assumes ds,dgroup
        assumes ss,dgroup


;-------------------------------------------------------------------------
;	DisplayFli( ww, hpldr/* or hscc or hwwd */, idr, dl, ypLine )
;-------------------------------------------------------------------------
;/* D I S P L A Y  F L I */
;/* Display formatted line in window pwwd at line dl. ypLine is bottom of
;the line */
;
;NATIVE DisplayFli(ww, hpldr/* or hscc or hwwd */, idr, dl, ypLine)
;struct PLDR	 **hpldr;
;int		 ww, idr, dl, ypLine;
;{
;	 struct PLCEDL	 **hplcedl;
;	 int		 dypToYw;
;	 int		 dxpToXw, xw;
;	 struct DR	 *pdr;
;	 struct PT	 ptOrigin;
;	 struct WWD	 *pwwd, **hwwd;
;	 BOOL		 fInTable;
;	 BOOL		 fFat;
;	 int		 fSoftPageBreak = fFalse;
;	 HDC		 hdc;
;	 BOOL		 fRMark;
;	 GRPFVISI	 grpfvisi;
;	 struct EDL	 edl;
;	 struct RC	 rcw, rcwErase, rcwClip;
;	 struct DRC	 drcp;
;	 int		 irmBar;
;	 struct DRF	 drfFetch;
;	 BOOL		 fFrameLines;

maskfNotSplatBreakLocal     equ 001h
maskfNotPageViewLocal	    equ 002h
maskfFatLocal		    equ 004h
maskfFrameLinesLocal	    equ 008h
maskfInTableLocal	    equ 020h
maskfSoftPageBreakLocal     equ 040h

Ltemp011:
	jmp	DF485

; %%Function:N_DisplayFli %%Owner:BRADV
cProc	N_DisplayFli,<PUBLIC,FAR>,<si,di>
	ParmW	ww
	ParmW	hpldr
	ParmW	idr
	ParmW	dlArg
	ParmW	ypLine

	LocalW	hplcedl
	LocalW	hdc
	LocalW	dxpToXw
	LocalW	dypToYw
	LocalW	hwwd
	LocalW	pdr
	LocalW	fInTable
	LocalW	xwSelBar
	LocalW	grpfvisiVar
	LocalV	drcp,cbDrcMin
	LocalV	edl,cbEdlMin
	LocalV	rcw,cbRcMin
	LocalV	rcwT,cbRcMin
	LocalV	rcwErase,cbRcMin
	LocalV	rcwClip,cbRcMin
	LocalV	uls,cbUlsMin
	LocalV	rcT,cbRcMin
	LocalV	drfFetch,cbDrfMin

cBegin

;	vrf.fInDisplayFli = !vrf.fInDisplayFli;
;	Assert (vrf.fInDisplayFli);
;	if (!vrf.fInDisplayFli)
;		return;
	xor	[vrf.fInDisplayFliRf],maskFInDisplayFliRf
	errnz	<maskFInDisplayFliRf - 080h>
ifdef DEBUG
	js	DF005
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midDisp1n
	mov	bx,1040 		; label # for native assert
	cCall	AssertProcForNative, <ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	jmp	Ltemp011
DF005:
else ;!DEBUG
	jns	Ltemp011
endif ;!DEBUG

;	Assert(hpldr != NULL );
ifdef DEBUG
	cmp	[hpldr],NULL
	jne	DF01
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,midDisp1n
	mov	bx,1050
	cCall	AssertProcForNative,<ax,bx>

	pop	dx
	pop	cx
	pop	bx
	pop	ax
DF01:
endif ;/* DEBUG */

;	FreezeHp();
ifdef	DEBUG
	call	LN_FreezeHp
endif ;DEBUG

;	pdr = PdrFetch(hpldr, idr, &drfFetch);
	lea	ax,[drfFetch]
ifdef DEBUG
	cCall	S_PdrFetch,<[hpldr],[idr],ax>
else ;not DEBUG
	cCall	N_PdrFetch,<[hpldr],[idr],ax>
endif ;DEBUG
	mov	[pdr],ax
	xchg	ax,si

;	hplcedl = pdr->hplcedl;
	mov	bx,[si.hplcedlDr]
	mov	[hplcedl],bx

;	Assert((!vfli.fPrint || ww == wwNil) && (*hplcedl)->dlMax > dl);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	[vfli.fPrintFli],fFalse
	je	DF02
	cmp	[ww],0
	jne	DF025
DF02:
	mov	bx,[bx]
	mov	ax,[dlArg]
	cmp	[bx.dlMaxPlcedl],ax
	jg	DF03
DF025:
	mov	ax,midDisp1n
	mov	bx,1051
	cCall	AssertProcForNative,<ax,bx>
DF03:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;/* Fill up EDL */
;	PutCpPlc(hplcedl, dl, vfli.cpMin);
	cCall	PutCpPlc,<bx, [dlArg], [vfli.HI_cpMinFli], [vfli.LO_cpMinFli]>

;	Assembler note: this code has been moved up from below to reduce code.
;	***Begin moved code
	push	ds
	pop	es
	lea	di,[edl]
;	/* set flags below; wipes out dcpDepend and other flags that
;	   are set later */
;	Assert (!dlkNil);
	errnz	<dlkNil>

;	edl.grpfEdl = 0;
;#ifdef INEFFICIENT
;	/* Native Note: zero this whole byte, give or take fHasBitmap */
;	edl.fDirty = edl.fTableDirty = fFalse;
;	edl.dlk = dlkNil;
;	edl.fNeedsEnhance = fFalse;
;#endif /* INEFFICIENT */

;	fRMark = vfli.fRMark;
;	if ((fInTable = pdr->fInTable) != 0)
;		edl.fRMark = fRMark;
;	edl.dcpDepend = vfli.dcpDepend;

;	/* to keep color metafiles out of scc cache */
;	edl.fColorMFP = vfli.fMetafile && !vsci.fMonochrome;
; PAUSE
	errnz	<(grpfEdl) - 0>
	mov	al,bptr ([vfli.dcpDependFli])
	errnz	<(dcpDependEdl) - 0>
	stosb

	mov	al,[vfli.fMetafileFli]
	cmp	[vsci.fMonochromeSci],fFalse
        je      DF035
        mov     al,0
DF035:
	and	al,maskFMetafileFli
	errnz	<(maskFMetafileFli) - 004h>
	errnz	<(maskFColorMFPEdl) - 040h>
	errnz	<(maskFRMarkEdl) - 010h>
	errnz	<(fColorMFPEdl) - (fRMarkEdl)>
	test	[si.fInTableDr],maskFInTableDr
	je	DF0351
	test	[vfli.fRMarkFli],maskFRMarkFli
	je	DF0351
	errnz	<(maskFRMarkEdl) - 010h>
	inc	ax	;Really cute Tom!
DF0351:
	mov	cl,4
	shl	al,cl
	errnz	<(fColorMFPEdl) - 1>
	errnz	<(fRMarkEdl) - 1>
	stosb

;	edl.hpldr = hNil;
	errnz	<hNil>
	xor	ax,ax
	errnz	<(hpldrEdl) - 2>
	stosw

;	***End moved code

;/* Include margin area, to support xwLimScroll optimization */
;/* Include margin area, to support xwLimScroll optimization */
;        edl.dxp = ((vfli.fRight || vfli.fTop || vfli.fBottom) 
;			?
;			max( vfli.xpMarginRight + dxpLeftRightSpace +  
;				DxFromBrc(vfli.brcRight,fFalse/*fFrameLines*/), 
;			     vfli.xpRight)
;			:
;			vfli.xpRight) - (edl.xpLeft = vfli.xpLeft);
;	edl.dcp = vfli.cpMac - vfli.cpMin;
;	edl.ypTop = ypLine - (edl.dyp = vfli.dypLine);
;#define DxFromBrc(brc, fFrameLines) DxyFromBrc(brc, fFrameLines, fTrue)
;#define DxyFromBrc(brc, fFrameLines, fWidth)	 \
;			 WidthHeightFromBrc(brc, fFrameLines | (fWidth << 1))
	errnz	<(xpLeftEdl) - 00004h>
	errnz	<(ypTopEdl) - 00006h>
	errnz	<(dxpEdl) - 00008h>
	errnz	<(dypEdl) - 0000Ah>
	errnz	<(LO_dcpEdl) - 0000Ch>
	errnz	<(HI_dcpEdl) - 0000Eh>
	mov	ax,[vfli.xpLeftFli]
	stosw
	xchg	ax,bx
	mov	ax,[ypLine]
	sub	ax,[vfli.dypLineFli]
	stosw

	errnz	<(fRightFli)-(fTopFli)>
	errnz	<(fRightFli)-(fBottomFli)>
	mov	ax,[vfli.xpRightFli]
	test	[vfli.fRightFli],maskFBottomFli OR maskFTopFli OR maskFRightFli
	jz	DF0355
	push	bx	;save vfli.xpLeft
	push	ax	;save vfli.xpRight
	push	[vfli.brcRightFli]
	mov	ax,2
	push	ax
ifdef DEBUG
	cCall	S_WidthHeightFromBrc,<>
else ;not DEBUG
	cCall	N_WidthHeightFromBrc,<>
endif ;DEBUG
	add	ax,[vfli.xpMarginRightFli]
	add	ax,[vfti.dxpBorderFti]
	add	ax,[vfti.dxpBorderFti]
	pop	bx	; restore vfli.xpRight
	cmp	ax,bx
	jge 	DF0355A
	xchg	ax,bx
DF0355A:
	pop	bx	;restore vfli.xpLeft
DF0355:
	sub	ax,bx
	stosw
	mov	ax,[vfli.dypLineFli]
	stosw
	mov	ax,[vfli.LO_cpMacFli]
	sub	ax,[vfli.LO_cpMinFli]
	stosw
	mov	ax,[vfli.HI_cpMacFli]
	sbb	ax,[vfli.HI_cpMinFli]
	stosw

;	if (ww == wwNil)
;		{
	mov	bx,[ww]
	errnz	<wwNil>
	or	bx,bx
	jne	DF06

DF04:
;		edl.fDirty = fTrue;
	or	[edl.fDirtyEdl],maskfDirtyEdl

;		PutPlc(hplcedl, dl, &edl);
;	DF49 performs
;	PutPlc(hplcedl, dlArg, &edl).
;	ax, bx, cx, dx are altered.
	call	DF49

DF05:
;		MeltHp();
ifdef	DEBUG
	call	LN_MeltHp
endif ;DEBUG

;		goto LRet0;
	jmp	LRet0

;		}
DF06:

	;Assembler note: intialize ww based variables here, so that we
	;only call PwwdWw once.
	cCall	N_PwwdWw,<bx>
	xchg	ax,bx
	mov	ax,[bx.xwSelBarWwd]
	mov	[xwSelBar],ax
	mov	ax,[bx.grpfvisiWwd]
	mov	[grpfvisiVar],ax

;	PutPlc(hplcedl, dl, &edl);
;	DF49 performs
;	PutPlc(hplcedl, dlArg, &edl).
;	ax, bx, cx, dx are altered.
	call	DF49

;	if (vfli.fPicture)
;	    {
	cmp	[vfli.fPictureFli],fFalse
	je	DF07

;	    vdrdlDisplayFli.hpldr = hpldr;
;	    vdrdlDisplayFli.idr = idr;
;	    vdrdlDisplayFli.dl = dl;
	errnz	<(idrDrdl) - 00000h>
	errnz	<(hpldrDrdl) - 00002h>
	errnz	<(dlDrdl) - 00004h>
	push	ds
	pop	es
	mov	di,dataoffset vdrdlDisplayFli
	mov	ax,[idr]
	stosw
	mov	ax,[hpldr]
	stosw
	mov	ax,[dlArg]
	stosw

;	    }
DF07:

;	Debug(grpfvisi = PwwdWw(ww)->grpfvisi);
;	Assert((int) vfli.grpfvisi == (int) grpfvisi);
ifdef DEBUG
	;Do this DEBUG stuff with a call so as not to mess up short jumps.
	call	DF50
endif ;/* DEBUG */

;	/* fake x coordinates in edl for erase and clip rectangle */
;	/* then transform into w's.  Intersect with rcwDisp */
;	ptOrigin = PtOrigin(hpldr, idr);
ifdef DEBUG
	cCall	S_PtOrigin,<[hpldr], [idr]>
else ;not DEBUG
	cCall	N_PtOrigin,<[hpldr], [idr]>
endif ;DEBUG

;	hwwd = vhwwdOrigin;
;	pwwd = *hwwd;
	mov	bx,[vhwwdOrigin]
	mov	[hwwd],bx
	mov	bx,[bx]

;	dxpToXw = ptOrigin.xl + pwwd->rcePage.xeLeft + pwwd->xwMin;
	add	ax,[bx.rcePageWwd.xeLeftRc]
	add	ax,[bx.xwMinWwd]
	mov	[dxpToXw],ax


;	rcwErase.xwLeft = (drcp.xp = -pdr->dxpOutLeft) + dxpToXw;
	mov	cx,[si.dxpOutLeftDr]
	neg	cx
	mov	[drcp.xpDrc],cx
	add	ax,cx
	errnz	<(xwLeftRc) - 00000h>
	mov	[rcwErase.xwLeftRc],ax

;	if (pwwd->fPageView)
;	    rcwErase.xwRight = rcwErase.xwLeft +
;		(drcp.dxp = pdr->dxl + pdr->dxpOutRight + pdr->dxpOutLeft);
;	else
;	    {
;	    rcwErase.xpRight = pwwd->xwMac;
;	    drcp.dxp = rcwErase.xwRight - rcwErase.xwLeft;
;	    }
	errnz	<(xwRightRc) - (xpRightRc)>
	mov	di,[bx.xwMacWwd]
	test	[bx.fPageViewWwd],maskfPageViewWwd
	je	DF08
	mov	di,[si.dxlDr]
	add	di,[si.dxpOutRightDr]
	sub	di,cx
	add	di,ax
DF08:
	mov	[rcwErase.xpRightRc],di
	sub	di,ax
	mov	[drcp.dxpDrc],di

;	dypToYw = ptOrigin.yl + pwwd->rcePage.yeTop + pwwd->ywMin;
	add	dx,[bx.rcePageWwd.yeTopRc]
	add	dx,[bx.ywMinWwd]
	mov	[dypToYw],dx

;	rcwErase.ywTop = (drcp.yp = edl.drcp.yp) + dypToYw;
	mov	ax,[edl.drcpEdl.ypDrc]
	mov	[drcp.ypDrc],ax
	add	ax,dx
	mov	[rcwErase.ywTopRc],ax

;	rcwErase.ywBottom = rcwErase.ywTop + (drcp.dyp = edl.drcp.dyp);
	mov	dx,[edl.drcpEdl.dypDrc]
	mov	[drcp.dypDrc],dx
	add	ax,dx
	mov	[rcwErase.ywBottomRc],ax

;/* if displaying to screen cache bitmap:
;	use scratchpad memory DC
;	do not validate dl if bitmap cannot fit the line
;*/
;	hdc = pwwd->hdc;
	mov	ax,[bx.hdcWwd]
	mov	[hdc],ax

	lea	ax,[drcp]
	lea	dx,[rcw]

;	ClipRectFromDr(hwwd, hpldr, idr, &drcp, &rcw);
	cCall	ClipRectFromDr,<[hwwd], [hpldr], [idr], ax, dx>

;	rcwClip = rcw;
	push	ds
	pop	es
	push	si	;save pdr
	lea	si,[rcw]
	lea	di,[rcwClip]
	errnz	<cbRcMin - 8>
	movsw
	movsw
	movsw
	movsw
	pop	si	;restore pdr

;	Assert( hdc != NULL );
ifdef DEBUG
	;Assert( hdc != NULL ) with a call so as not to mess up short jumps.
	call	DF56
endif ;/* DEBUG */

;	/* Check for quick return if the line is not visible at all. */
;	if (rcw.ywTop >= rcw.ywBottom || rcw.xwLeft >= rcw.xwRight)
;		{
;		MeltHp();
;		goto LRet;
;		}
	mov	ax,[rcw.ywTopRc]
	cmp	ax,[rcw.ywBottomRc]
	jge	Ltemp020
	mov	ax,[rcw.xwLeftRc]
	cmp	ax,[rcw.xwRightRc]
	jge	Ltemp020

;	Assembler note: fFat is now computed below LBeginLine
;	/* This tells if there is a dr overflow. */
;	Assert(!fInTable || FInCa(pdr->doc, pdr->cpFirst, &caTap));
;	fFat = pdr->doc > 0
;		&& (fInTable ? pdr->fCantGrow : vfli.fPageView)
;		&& ((ypLine == pdr->dyl && pdr->idrFlow == idrNil
;			&& (fInTable ? !vfli.fSplatBreak || vfli.chBreak != chTable
;				: vfli.cpMac < pdr->cpLim))
;		    || ypLine > pdr->dyl);

;	Scribble(ispDisplayFli,'D');
ifdef DEBUG
	call	DF54
endif ;DEBUG

;LBeginLine:
;	Assembler note: this has been moved from the start of DisplayFli
;	fInTable = pdr->fInTable;
;	fSoftPageBreak = fFalse;
	mov	al,[si.fInTableDr]
	mov	ah,[vfli.fSplatBreakFli]
	errnz	<maskfInTableLocal - maskFInTableDr>
	and	ax,maskFInTableDr + (maskFSplatBreakFli SHL 8)
	mov	bptr ([fInTable]),al
	errnz	<maskFSplatBreakFli - 1>
	errnz	<maskFInTableDr AND maskfNotSplatBreakLocal>
	inc	ax
	xor	al,ah
	errnz	<maskfNotSplatBreakLocal - maskFSplatBreakFli>

	cmp	[vfli.fPageViewFli],fFalse
	jne	DF13
	errnz	<maskFInTableDr AND maskfNotPageViewLocal>
	errnz	<maskFSplatBreakFli AND maskfNotPageViewLocal>
	errnz	<maskfNotPageViewLocal - 2>
	inc	ax
	inc	ax
DF13:
	;now have register fInTable, fNotPageView and fNotSplatBreak

;	Assembler note: Computing fFat has been moved from above.
;	/* This tells if there is a dr overflow. */
;	fFat = pdr->doc > 0
;		&& (fInTable ? pdr->fCantGrow : vfli.fPageView)
;		&& ((ypLine == pdr->dyl && pdr->idrFlow == idrNil
;			&& (fInTable ? !vfli.fSplatBreak || vfli.chBreak != chTable
;				: vfli.cpMac < pdr->cpLim))
;		    || ypLine > pdr->dyl);
;PAUSE
	cmp	[si.docDr],0
	jle	DF15
	test	al,maskFInTableDr
	je	DF14
	test	[si.fCantGrowDr],maskfCantGrowDr
	je	DF15
 	jmp	short DF141
DF14:
	cmp	[vfli.fPageViewFli],0
	je	DF15
DF141:
	mov	cx,[ypLine]
	cmp	cx,[si.dylDr]
	jg	DF145
	jne	DF15
	cmp	[si.idrFlowDr],idrNil
	jne	DF15
	test	al,maskFInTableDr
	je	DF143
	test	al,maskfNotSplatBreakLocal
	jne	DF145
	cmp	bptr ([vfli.chBreakFli]),chTable
	jne	DF145
	jmp	short DF15
Ltemp020:
ifdef	DEBUG
	call	LN_MeltHp
endif ;DEBUG
	jmp	LRet
DF143:
	mov	cx,[vfli.LO_cpMacFli]
	sub	cx,[si.LO_cpLimDr]
	mov	cx,[vfli.HI_cpMacFli]
	sbb	cx,[si.HI_cpLimDr]
	jge	DF15
DF145:
	errnz	<maskFInTableDr AND maskfFatLocal>
	errnz	<maskFSplatBreakFli AND maskfFatLocal>
	or	al,maskfFatLocal
DF15:


;	MeltHp();
ifdef	DEBUG
	call	LN_MeltHp
endif ;DEBUG

;	if (!vfli.fPageView)
;		{
	test	al,maskfNotPageViewLocal
	jne	LTemp021
	jmp	DF27
Ltemp021:

;		if (vfli.fSplatBreak && !fInTable)
;			{
	test	al,maskfNotSplatBreakLocal + maskfInTableLocal
	jne	Ltemp022

;			int ipat;
;
;			xw = rcw.xwLeft;
	mov	si,[rcw.xwLeftRc]

;			edl.dxp = rcw.xwRight - dxpToXw - edl.xpLeft;
	mov	bx,[rcw.xwRightRc]
	sub	bx,[dxpToXw]
	sub	bx,[edl.xpLeftEdl]
	mov	[edl.dxpEdl],bx
	db	03Dh	;turns next "jae LSplat2" into "cmp ax,immediate"

;LSplat:
LSplat:
	jae	LSplat2     ;Jump only if we have jumped to here
			    ;and vfli.ichMac > 1.

;/* Must erase the line and sel bar background, since we won't be calling
;   DisplayFliCore. */
;			Assert(vfli.doc != docNil);
ifdef DEBUG
	;Assert(vfli.doc != docNil) with a call so as not to
	;mess up short jumps.
	call	DF65
endif ;/* DEBUG */

;			PatBltRc(hdc, &rcw, vsci.ropErase);
	push	[hdc]
	lea	ax,[rcw]
	push	ax
	push	[vsci.HI_ropEraseSci]
	push	[vsci.LO_ropEraseSci]
	cCall	PatBltRc,<>

;LSplat2:
;			ipat = vfli.fSplatColumn ? ipatHorzLtGray :
;							ipatHorzGray;
LSplat2:
	errnz	<ipatHorzGray - 0>
	errnz	<ipatHorzLtGray - 1>
	xor	di,di
	test	[vfli.fSplatColumnFli],maskFSplatColumnFli
	je	DF16
	inc	di
DF16:

;			CacheSect(vfli.doc, vfli.cpMin);
;#define CacheSect(doc,cp) (FInCa((doc),(cp),(&caSect))?0:CacheSectProc((doc),(cp)))
	stc
DF163:
	push	[vfli.docFli]
	push	[vfli.HI_cpMinFli]
	push	[vfli.LO_cpMinFli]
	jnc	DF167
	mov	ax,dataoffset [caSect]
	push	ax
	cCall	FInCa,<>
	or	ax,ax
	jne	DF17
	jmp	short DF163
Ltemp022:
	jmp	short DF22
DF167:
	cCall	CacheSectProc,<>
DF17:

;			/* Draw the break/section splat. */
;			ywMid = rcwErase.ywBottom - vfli.dypBase;
;			if (ywMid < rcw.ywBottom && ywMid >= rcw.ywTop)
;				DrawPatternLine(hdc, xw, ywMid,	rcw.xwRight - xw, ipat, pltHorz);
	mov	ax,[rcwErase.ywBottomRc]
	sub	ax,[vfli.dypBaseFli]
	;DF575 performs
	;if (ax < rcw.ywBottom && ax >= rcw.ywTop)
	;    DrawPatternLine(hdc, si, ax, rcw.xwRight-si, di, pltHorz);
	;ax, bx, cx, dx are altered.
	call	DF575

;			PutPlc(hplcedl, dl, &edl);
;	DF49 performs
;	PutPlc(hplcedl, dlArg, &edl).
;	ax, bx, cx, dx are altered.
	call	DF49

;			if (vfli.cpMac == caSect.cpLim)
;				{
	mov	ax,[vfli.LO_cpMacFli]
	mov	dx,[vfli.HI_cpMacFli]
	sub	ax,[caSect.LO_cpLimCa]
	sbb	dx,[caSect.HI_cpLimCa]
	or	ax,dx
	jne	DF20

;				ywMid = rcwErase.ywBottom - vfli.dypBase -
;					min((edl.dyp - vfli.dypBase),
;					(vsci.dypBorder << 1));
	mov	cx,[vfli.dypBaseFli]
	mov	ax,[rcwErase.ywBottomRc]
	sub	ax,cx
	mov	dx,[edl.dypEdl]
	sub	dx,cx
	mov	cx,[vsci.dypBorderSci]
	shl	cx,1
	cmp	cx,dx
	jle	DF18
	mov	cx,dx
DF18:
	sub	ax,cx

;				Assert(!vfli.fSplatColumn);
ifdef DEBUG
	;Assert(!vfli.fSplatColumn) with a call so as not to
	;mess up short jumps.
	call	DF63
endif ;/* DEBUG */

;	if (ywMid < rcw.ywBottom && ywMid >= rcw.ywTop)
;		DrawPatternLine(hdc, xw, ywMid, rcw.xwRight - xw, ipat,pltHorz );
	;DF575 performs
	;if (ax < rcw.ywBottom && ax >= rcw.ywTop)
	;    DrawPatternLine(hdc, si, ax, rcw.xwRight-si, di, pltHorz);
	;ax, bx, cx, dx are altered.
	call	DF575

;				}
DF20:

;			/* Paint the outline bullets because LHighlight
;			   is located after we call AddVisiSpaces(). */
;			if (vfli.grpfvisi.w || (vfli.omk != omkNil && vfli.fOutline))
;				{
;				AddVisiSpaces(hdc, dxpToXw, ypLine+dypToYw, &rcwClip );
;				}
	;DF60 performs
	;if (vfli.grpfvisi.w || (vfli.omk != omkNil && vfli.fOutline))
	;	 {
	;	 AddVisiSpaces(hdc, dxpToXw, ypLine+dypToYw, &rcwClip );
	;	 }
	;ax, bx, cx, dx are altered.
	call	DF60

;			goto LHighlight;
	jmp	LHighlight

;			}
;		else
;/* check for soft page breaks */
;			{

DF22:
;			struct DOD *pdod;
;
;			pdod = PdodDoc(vfli.doc);
;			if (!pdod->fShort && pdod->hplcpgd && !(*hwwd)->fOutline)
;				{
	mov	si,[vfli.docFli]
	shl	si,1
	mov	si,[si.mpdochdod]
	mov	bx,[si]
	cmp	[bx.fShortDod],fFalse
	jne	Ltemp008
	cmp	[bx.hplcpgdDod],fFalse
	je	Ltemp008
	mov	bx,[hwwd]
	mov	bx,[bx]
	test	[bx.fOutlineWwd],maskFOutlineWwd
	je	Ltemp007
Ltemp008:
	jmp	DF27
Ltemp007:

	push	ax	;save flags
	push	si	;save hdod
;				if (!FInCa(vfli.doc, vfli.cpMac - 1, &caPage))
;					{
	mov	si,[vfli.LO_cpMacFli]
	mov	di,[vfli.HI_cpMacFli]
	sub	si,1
	sbb	di,0
	mov	ax,dataoffset [caPage]
	cCall	FInCa,<[vfli.docFli], di, si, ax>
	or	ax,ax
	jne	DF23

;					CP cpFli;
;					int wwFli, docFli, dxaFli;
;					wwFli = vfli.ww;
;					docFli = vfli.doc;
;					cpFli = vfli.cpMin;
;					dxaFli = vfli.dxa;
;					CachePage(vfli.doc, vfli.cpMac - 1);
;					if (vfFliMunged && wwFli != wwNil && docFli != docNil)
;						FormatLineDxa(wwFli, docFli, cpFli, dxaFli);
;					}
	mov	bx,[vfli.wwFli]
	push	bx		    ;argument for FormatLineDxa
	mov	ax,[vfli.docFli]
	push	ax		    ;argument for FormatLineDxa
	push	[vfli.HI_cpMinFli]  ;argument for FormatLineDxa
	push	[vfli.LO_cpMinFli]  ;argument for FormatLineDxa
	push	[vfli.dxaFli]	    ;argument for FormatLineDxa
	push	bx	;save wwFli
	push	ax	;save docFli
	cCall	CachePage,<ax, di, si>
	pop	ax	;restore docFli
	pop	bx	;restore wwFli
	mov	cx,[vfFliMunged]
	jcxz	DF225
	errnz	<wwNil>
	or	bx,bx
	je	DF225
	errnz	<docNil>
	or	ax,ax
	je	DF225
ifdef DEBUG
	cCall	S_FormatLineDxa,<>
else ;not DEBUG
	cCall	N_FormatLineDxa,<>
endif ;DEBUG
	jmp	short DF23
DF225:
	add	sp,10		    ;remove FormatLineDxa arguments from stack
DF23:
	pop	bx	;restore hdod
	mov	bx,[bx]
	pop	ax	;restore flags

;/* we are now guaranteed that cpMac-1 is within the cached page */
;				if (!fInTable && vfli.cpMac == caPage.cpLim
;					&& vfli.cpMac < CpMacDocEdit(vfli.doc))
;					{
	test	al,maskfInTableLocal
	jne	DF26
	mov	cx,si
	sub	cx,[caPage.LO_cpLimCa]
	mov	dx,di
	sbb	dx,[caPage.HI_cpLimCa]
	and	dx,cx
	inc	dx
	jne	DF26
	;***Begin in line CpMacDocEdit(vfli.doc)
	sub	si,[bx.LO_cpMacDod]
	sbb	di,[bx.HI_cpMacDod]
	add	si,3*ccpEop+1
	adc	di,0
	;***End in line CpMacDocEdit(vfli.doc)
	jge	DF26

;/* last line of page. Show page break */
;					fSoftPageBreak = fTrue;
	or	al,maskfSoftPageBreakLocal
;					}
DF26:

;				}
;			}
;		}
DF27:

;	fFrameLines = ((vfli.fPageView && FDrawPageDrsWw(ww) && !fInTable)
;		|| (fInTable && FDrawTableDrsWw(ww)));
;#define FDrawPageDrsWw(ww)   (PwwdWw(ww)->grpfvisi.fDrawPageDrs || PwwdWw(ww)->grpfvisi.fvisiShowAll)
	mov	cx,ax
	and	cx,maskfInTableLocal
	test	al,maskfNotPageViewLocal+maskfInTableLocal
	jne	DF271
	errnz	<maskfDrawPageDrsGrpfvisi - 04000h>
	errnz	<maskfvisiShowAllGrpfvisi - 00020h>
	test	[grpfvisiVar],maskfDrawPageDrsGrpfvisi+maskfvisiShowAllGrpfvisi
	jne	DF272
DF271:
	jcxz	DF277
;#define FDrawTableDrsWw(ww)  (PwwdWw(ww)->grpfvisi.fDrawTableDrs || PwwdWw(ww)->grpfvisi.fvisiShowAll)
	errnz	<maskfDrawTableDrsGrpfvisi - 02000h>
	errnz	<maskfvisiShowAllGrpfvisi - 00020h>
	test	[grpfvisiVar],maskfDrawTableDrsGrpfvisi+maskfvisiShowAllGrpfvisi
	je	DF277
DF272:
	or	al,maskfFrameLinesLocal

;	/* don't blow away the frame lines if they're just going to get
;		drawn back again; otherwise they'll flicker */
;	if (fFrameLines)
;		{
	;Assembler note: the test for fFrameLines has already been done

;		if (dl == 0 && pdr->yl == 0)
;			rcw.ywTop = min(rcw.ywBottom, rcw.ywTop + dyFrameLine);
;#define dyFrameLine		 dypBorderFti
;#define dypBorderFti		 vfti.dypBorder
	mov	bx,[pdr]
	mov	cx,[dlArg]
	or	cx,[bx.ylDr]
	jne	DF274
	mov	cx,[rcw.ywTopRc]
	add	cx,[vfti.dypBorderFti]
	cmp	cx,[rcw.ywBottomRc]
	jle	DF273
	mov	cx,[rcw.ywBottomRc]
DF273:
	mov	[rcw.ywTopRc],cx
DF274:

;		if (rcwErase.ywBottom >= YwFromYl(hpldr, (*hpldr)->dyl) &&
;				pdr->fBottomTableFrame && !fFat)
	test	[bx.fBottomTableFrameDr],maskFBottomTableFrameDr
	je	DF277
	test	al,maskfFatLocal
	jne	DF277
	push	ax	;save flags
	mov	bx,[hpldr]
	push	bx
	mov	bx,[bx]
	push	[bx.dylPldr]
	cCall	YwFromYl,<>
	xchg	ax,cx
	pop	ax	;restore flags
	mov	bx,[rcw.ywBottomRc]
	cmp	bx,cx
	jl	DF277

;			rcw.ywBottom = max(rcw.ywTop, rcw.ywBottom - dyFrameLine);
;#define dyFrameLine		 dypBorderFti
;#define dypBorderFti		 vfti.dypBorder
	sub	bx,[vsci.dypBorderSci]
	cmp	bx,[rcw.ywTopRc]
	jge	DF276
	mov	bx,[rcw.ywTopRc]
DF276:
	mov	[rcw.ywBottomRc],bx

;		}
DF277:

;	DisplayFliCore(ww, rcw, dxpToXw, rcwErase.ywBottom);
	push	ax	;save flags
	push	[ww]
	push	[rcw.ypBottomRc]
	push	[rcw.xpRightRc]
	push	[rcw.ypTopRc]
	push	[rcw.xpLeftRc]
	push	[dxpToXw]
	push	[rcwErase.ywBottomRc]
ifdef DEBUG
	cCall	S_DisplayFliCore,<>
else ;not DEBUG
	call	far ptr N_DisplayFliCore
endif ;DEBUG

;	GetPlc(hplcedl, dl, &edl);
	lea	ax,[edl]
	cCall	GetPlc,<[hplcedl], [dlArg], ax>

;	if (fSoftPageBreak)
;		{
;		DrawPatternLine(hdc, rcw.xwLeft,
;				rcwErase.ywBottom - vsci.dypBorder,
;				rcw.xwRight - rcw.xwLeft,
;				ipatHorzLtGray, pltHorz);
;/* last line of page. Show page break */

	pop	ax	;restore flags
	push	ax	;save flags
	test	al,maskfSoftPageBreakLocal
	jz	DF278
	mov	si,[rcw.xwLeftRc]
	mov	ax,[rcwErase.ywBottomRc]
	sub	ax,[vsci.dypBorderSci]
	mov	di,ipatHorzLtGray
	;DF58 performs DrawPatternLine(hdc, si, ax, rcw.xwRight-si, di, pltHorz);
	;ax, bx, cx, dx are altered.
	call	DF58

;		edl.dxp = rcw.xwRight - dxpToXw - edl.xpLeft;
	mov	ax,[rcw.xwRightRc]
	sub	ax,[dxpToXw]
	sub	ax,[edl.xpLeftEdl]
	mov	[edl.dxpEdl],ax

;		PutPlc(hplcedl, dl, &edl);
;		}
;	DF49 performs
;	PutPlc(hplcedl, dlArg, &edl).
;	ax, bx, cx, dx are altered.
	call	DF49

DF278:
	pop	ax	;restore flags
	push	ax	;save flags
	
;	if (!vfli.fPrint && vlm != lmPreview)
;		{
	cmp	[vfli.fPrintFli],fFalse
	jne	DF33
	cmp	[vlm],lmPreview
	je	DF33

;		if (fFrameLines)
	test	al,maskfFrameLinesLocal
	je	DF32

;			FrameDrLine(ww, hpldr, idr, rcwErase.ywTop, rcwErase.ywBottom,
;				fFat, fInTable/*fForceLine*/, fInTable/*fInnerDr*/);
	push	[ww]
	push	[hpldr]
	push	[idr]
	push	[rcwErase.ywTopRc]
	push	[rcwErase.ywBottomRc]
	mov	cx,ax
	and	ax,maskfFatLocal
	push	ax
	and	cx,maskfInTableLocal
	push	cx
	push	cx
	cCall	FrameDrLine,<>
DF32:

;		}
DF33:


;	if (vfli.fOutline)
;		{
	cmp	[vfli.fOutlineFli],fFalse
	je	DF34

;		struct PT	ptT;

;		/* Know: what x position is after trailing space, so ellipses
;			can be drawn ... */
;		ptT.xp = vfli.xpRight + dxpToXw;
;		ptT.yp = ypLine;
;                edl.dxp += DxpOutlineSplats(hdc, dxpToXw, ptT, &rcwErase);
;		PutPlc(hplcedl, dl, &edl);
;PAUSE
	push	[hdc]
	mov	ax,[dxpToXw]
	push	ax
	push	[ypLine]
	add	ax,[vfli.xpRightFli]
	push	ax
	lea	ax,[rcwErase]
	push	ax
	cCall	DxpOutlineSplats,<>
	add	[edl.dxpEdl],ax
;	DF49 performs
;	PutPlc(hplcedl, dlArg, &edl).
;	ax, bx, cx, dx are altered.
	call	DF49


;		}
DF34:

;/* after the line has been painted, retrofit visible spaces, draw selection
;and return bitmaps to their original state */

;	if (vfli.grpfvisi.w || (vfli.omk != omkNil && vfli.fOutline))
;		{
;		AddVisiSpaces(hdc, dxpToXw, ypLine+dypToYw, &rcwClip );
;		}
	;DF60 performs
	;if (vfli.grpfvisi.w || (vfli.omk != omkNil && vfli.fOutline))
	;	 {
	;	 AddVisiSpaces(hdc, dxpToXw, ypLine+dypToYw, &rcwClip );
	;	 }
	;ax, bx, cx, dx are altered.
	call	DF60

	pop	ax	;restore flags

;/* splats in page view */
;	if (vfli.fPageView && vfli.fSplatBreak && !fInTable && vfli.grpfvisi.fvisiShowAll)
;		{
	test	al,maskfNotPageViewLocal+maskfNotSplatBreakLocal+maskfInTableLocal
	jne	DF40
	errnz	<maskfvisiShowAllGrpfvisi - 00020h>
	test	bptr ([vfli.grpfvisiFli]),maskfvisiShowAllGrpfvisi
	je	DF40

;		struct RC rcwT;
;
;		xw = dxpToXw + vfli.xpRight - vfli.rgdxp[vfli.ichMac - 1];
;		rcw.xwLeft = xw;
;		DrclToRcw(hpldr, &pdr->drcl, &rcwT);
	mov	si,[dxpToXw]
	add	si,[vfli.xpRightFli]
	mov	bl,[vfli.ichMacFli]
	xor	bh,bh
	dec	bx
	shl	bx,1
	sub	si,[bx.vfli.rgdxpFli]
	;DF59 performs DrclToRcw(hpldr, &pdr->drcl, &rcwT)
	;ax, bx, cx, dx are altered.
	call	DF59
	mov	cx,si

;		rcw.xwRight = rcwT.xwRight;
	mov	dx,[rcwT.xwRightRc]

;		if (FDrawPageDrsWw(ww))
;			{
;#define FDrawPageDrsWw(ww)   (PwwdWw(ww)->grpfvisi.fDrawPageDrs || PwwdWw(ww)->grpfvisi.fvisiShowAll)
	errnz	<maskfDrawPageDrsGrpfvisi - 04000h>
	errnz	<maskfvisiShowAllGrpfvisi - 00020h>
	test	[grpfvisiVar],maskfDrawPageDrsGrpfvisi+maskfvisiShowAllGrpfvisi
	je	DF38

;/* adjust if FDrawPageDrsWw because PatBltRc would erase the boundary line */
;			rcw.xwLeft += vsci.dxpBorder;
;			rcw.xwRight -= vsci.dxpBorder;
;			if (rcw.ywTop <= rcwT.ywTop)
;				rcw.ywTop += vsci.dypBorder;
;			}
	add	cx,[vsci.dxpBorderSci]
	sub	dx,[vsci.dxpBorderSci]
	mov	ax,[rcw.ywTopRc]
	cmp	ax,[rcwT.ywTopRc]
	jg	DF38
	mov	ax,[vsci.dypBorderSci]
	add	[rcw.ywTopRc],ax
DF38:
	mov	[rcw.xwLeftRc],cx
	mov	[rcw.xwRightRc],dx

;		if (vfli.ichMac > 1)
;			goto LSplat2; /* not the only thing on the line, background already erased */
;		goto LSplat;
	cmp	[vfli.ichMacFli],1
	jmp	LSplat

;		}
DF40:

;/* show highlight */
;LHighlight:
LHighlight:

	mov	si,[ww]

;	if (PwwdWw(ww)->xwSelBar > 0)
;	    {
	cmp	[xwSelBar],fFalse
	jle	DF42

;	    Assert(!PwwdWw(ww)->fPageView);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cCall	N_PwwdWw,<[ww]>
	xchg	ax,bx
	test	[bx.fPageViewWwd],maskfPageViewWwd
	je	DF41
	mov	ax,midDisp1n
	mov	bx,1053
	cCall	AssertProcForNative,<ax,bx>
DF41:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

;	    if (!fInTable)
	cmp	bptr ([fInTable]),fFalse
	jne	DF42

;			DrawStyNameFromWwDl(ww, hpldr, idr, dl);
	cCall	DrawStyNameFromWwDl,<si, [hpldr], [idr], [dlArg]>

;	    }
DF42:


;	MarkSel(ww, hpldr, idr, &selCur, &edl, vfli.cpMin, vfli.cpMac );
;	MarkSel(ww, hpldr, idr, &selDotted, &edl, vfli.cpMin, vfli.cpMac );
	mov	di,dataoffset [selCur]
DF43:
	push	si
	push	[hpldr]
	push	[idr]
	push	di
	lea	ax,[edl]
	push	ax
	push	[vfli.HI_cpMinFli]
	push	[vfli.LO_cpMinFli]
	push	[vfli.HI_cpMacFli]
	push	[vfli.LO_cpMacFli]
	cCall	MarkSel,<>
	cmp	di,dataoffset [selCur]
	mov	di,dataoffset [selDotted]
	je	DF43


;/* iff fRMark text was on line, draw revision bar in selection bar */
;	if (fRMark && ((irmBar = PdodMother(vfli.doc)->dop.irmBar) != irmBarNone))
;		{
;		int xwRevBar;
	test	[vfli.fRMarkFli],maskFRMarkFli
	je	Ltemp002
	cCall	N_PdodMother,<[vfli.docFli]>
	xchg	ax,bx
	mov	al,[bx.dopDod.irmBarDop]
	errnz	<irmBarNone - 0>
	or	al,al
Ltemp002:
	je	DF47

;		if (!(*hwwd)->fPageView)
;		    xwRevBar = (*hwwd)->xwSelBar + dxwSelBarSci / 2;
;#define dxwSelBarSci		 vsci.dxwSelBar
	mov	di,[hwwd]
	mov	di,[di]
	mov	cx,[di.xwSelBarWwd]
	mov	si,[vsci.dxwSelBarSci]
	shr	si,1
	test	[di.fPageViewWwd],maskfPageViewWwd
	je	DF46

;		else
;		    {
;		    DrclToRcw(hpldr, &pdr->drcl, &rcw);
	push	ax	;save irmBar
	;DF59 performs DrclToRcw(hpldr, &pdr->drcl, &rcwT)
	;ax, bx, cx, dx are altered.
	call	DF59
	pop	ax	;restore irmBar

;		    switch (irmBar)
;			{
;			default:
;			    Assert(fFalse);
;			    /* fall through */
ifdef DEBUG
	;This assert is done with a call so as not to mess up short jumps
	call	DF67
endif ;DEBUG

;			case irmBarOutside:
;			    if (vfmtss.pgn & 1 && FFacingPages(DocMother(vfli.doc)))
;                               goto LRight;
;                           /* fall through */
;			case irmBarLeft:
;			    xwRevBar = rcw.xwLeft - (dxwSelBarSci / 2);
;			    break;
;			case irmBarRight:
;  LRight:
;			    xwRevBar = rcw.xwRight + (dxwSelBarSci / 2);
;			    break;
	;Assembler note: use rcwT here for convenience.
	mov	cx,[rcwT.xwRightRc]
	cmp	al,irmBarRight
	je	DF46
	cmp	al,irmBarOutside
	jne	DF455
	test	bptr ([vfmtss.pgnFmtss]),1
	je	DF455
	push	cx  ;save register rcwT.xwRight
	cCall	DocMother,<[vfli.docFli]>
	cCall	FFacingPages,<ax>
	pop	cx  ;restore register rcwT.xwRight
	or	ax,ax
	jne	DF46
DF455:
	mov	cx,[rcwT.xwLeftRc]
	neg	si
DF46:
	add	cx,si

;			}
;		if (!fInTable)
;			DrawRevBar( (*hwwd)->hdc, xwRevBar,
;				ypLine - edl.dyp + dypToYw, edl.dyp, &rcwClip);
; PAUSE
	cmp	bptr ([fInTable]),0
	jne	DF47
; PAUSE
	push	[di.hdcWwd]
	push	cx
	mov	ax,[ypLine]
	sub	ax,[edl.dypEdl]
	add	ax,[dypToYw]
	push	ax
	push	[edl.dypEdl]
	lea	ax,[rcwClip]
	push	ax
	call	far ptr DrawRevBar
;		    }
;		}
DF47:
;LRet:
;	pdr->xwLimScroll = max( pdr->xwLimScroll, 
;				edl.xpLeft + edl.dxp + dxpToXw );
;LRet0:
;	Scribble(ispDisplayFli,' ');
LRet:
	mov	si,[pdr]
	mov	ax,[dxpToXw]
	add	ax,[edl.dxpEdl]
	add	ax,[edl.xpLeftEdl]
	cmp	ax,[si.xwLimScrollDr]
	jl	LRet0
	mov	[si.xwLimScrollDr],ax
LRet0:	
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,ispDisplayFli
	mov	bx,020h
	cmp	[vdbs.grpfScribbleDbs],0
	jz	DF48
	cCall	ScribbleProc,<ax,bx>
DF48:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

;	FreePdrf(&drfFetch);
	lea	ax,[drfFetch]
ifdef DEBUG
	cCall	S_FreePdrf,<ax>
else ;not DEBUG
	cCall	N_FreePdrf,<ax>
endif ;DEBUG

DF485:
	xor	[vrf.fInDisplayFliRf],maskFInDisplayFliRf
;}
cEnd

;End of DisplayFli

;	DF49 performs
;	PutPlc(hplcedl, dlArg, &edl).
;	ax, bx, cx, dx are altered.
DF49:
	lea	ax,[edl]
	cCall	PutPlc,<[hplcedl], [dlArg], ax>
	ret


;	Debug(grpfvisi = PwwdWw(ww)->grpfvisi);
;	Assert((int) vfli.grpfvisi == (int) grpfvisi);
ifdef DEBUG
DF50:
	push	ax
	push	bx
	push	cx
	push	dx
	cCall	N_PwwdWw,<[ww]>
	xchg	ax,bx
	mov	ax,[bx.grpfvisiWwd]
	cmp	ax,[vfli.grpfvisiFli]
	je	DF51
	mov	ax,midDisp1n
	mov	bx,1055
	cCall	AssertProcForNative,<ax,bx>
DF51:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */


;			Assert(edl.dyp >= ((struct SCC *)pwwd)->ywMac);
ifdef DEBUG
DF52:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[bx.ywMacWwd]
	cmp	[edl.dypEdl],ax
	jge	DF53
	mov	ax,midDisp1n
	mov	bx,1056
	cCall	AssertProcForNative,<ax,bx>
DF53:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */


;	Scribble(ispDisplayFli,'D');
ifdef DEBUG
DF54:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,ispDisplayFli
	mov	bx,044h
	cmp	[vdbs.grpfScribbleDbs],0
	jz	DF55
	cCall	ScribbleProc,<ax,bx>
DF55:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG


;	Assert( hdc != NULL );
ifdef DEBUG
DF56:
	cmp	[hdc],NULL
	jne	DF57
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midDisp1n
	mov	bx,1057
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
DF57:
	ret
endif ;/* DEBUG */


	;DF575 performs
	;if (ax < rcw.ywBottom && ax >= rcw.ywTop)
	;    DrawPatternLine(hdc, si, ax, rcw.xwRight-si, di, pltHorz);
	;ax, bx, cx, dx are altered.
DF575:
	cmp	ax,[rcw.ywBottomRc]
	jge	DF585
	cmp	ax,[rcw.ywTopRc]
	jl	DF585
	;DF58 performs DrawPatternLine(hdc, si, ax, rcw.xwRight-si, di, pltHorz);
	;ax, bx, cx, dx are altered.
DF58:
	push	[hdc]
	push	si
	push	ax
	mov	ax,[rcw.xwRightRc]
	sub	ax,si
	push	ax
	push	di
	errnz	<pltHorz - 0>
	xor	ax,ax
	push	ax
	cCall	DrawPatternLine,<>
DF585:
	ret


	;DF59 performs DrclToRcw(hpldr, &pdr->drcl, &rcwT)
	;ax, bx, cx, dx are altered.
DF59:
	mov	bx,[pdr]
	add	bx,(drclDr)
	lea	ax,[rcwT]
	cCall	DrclToRcw,<[hpldr], bx, ax>
	ret


	;DF60 performs
	;if (vfli.grpfvisi.w || (vfli.omk != omkNil && vfli.fOutline))
	;	 {
	;	 AddVisiSpaces(hdc, dxpToXw, ypLine+dypToYw, &rcwClip );
	;	 }
	;ax, bx, cx, dx are altered.
DF60:
	cmp	[vfli.grpfvisiFli],fFalse
	jne	DF61
	errnz	<omkNil>
	test	[vfli.omkFli],maskOmkFli
	je	DF62
	cmp	[vfli.fOutlineFli],fFalse
	je	DF62
DF61:
	push	[hdc]
	push	[dxpToXw]
	mov	ax,[dypToYw]
	add	ax,[ypLine]
	push	ax
	lea	ax,[rcwClip]
	push	ax
ifdef DEBUG
	cCall	S_AddVisiSpaces,<>
else ;not DEBUG
	cCall	N_AddVisiSpaces,<>
endif ;DEBUG
DF62:
	ret


;				Assert(!vfli.fSplatColumn);
ifdef DEBUG
DF63:
	test	[vfli.fSplatColumnFli],maskfSplatColumnFli
	je	DF64
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midDisp1n
	mov	bx,1058
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
DF64:
	ret
endif ;/* DEBUG */


;			Assert(vfli.doc != docNil);
ifdef DEBUG
DF65:
	cmp	[vfli.docFli],docNil
	jne	DF66
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midDisp1n
	mov	bx,1059
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
DF66:
	ret
endif ;DEBUG


ifdef DEBUG
DF67:
	cmp	al,irmBarOutside
	je	DF68
	cmp	al,irmBarLeft
	je	DF68
	cmp	al,irmBarRight
	je	DF68
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midDisp1n
	mov	bx,1054
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
DF68:
	ret
endif ;DEBUG

;-------------------------------------------------------------------------
;	DrawRevBar( hdc, xp, yp, dyp )
;-------------------------------------------------------------------------
;/*
;*  DrawRevBar draws a revision bar at (xw,yw).  The height of the bar is
;*  determined by the size of vfli.dypLine or vfli.dypFont. If not NULL,
;*  drawing is clipped to *prcwClip.
;*/
;DrawRevBar(hdc, xw, yw, dyw, prcwClip)
;HDC hdc;
;int xw,yw,dxw;
;struct RC *prcwClip;
;{
;	 int dxp = vsci.dxpBorder;
;	 HBRUSH hbrOld;

; %%Function:DrawRevBar %%Owner:BRADV
cProc	DrawRevBar,<PUBLIC,FAR>,<si,di>
	ParmW	hdc
	ParmW	xw
	ParmW	yw
	ParmW	dyw
	ParmW	prcwClip

cBegin

;PAUSE
; if (prcwClip != NULL)
; {
	mov	dx,[xw]	; handy register variable
	mov	cx,[dyw] ; another handy register variable
	mov	bx,[prcwClip]
	or	bx,bx
	jz	DRB50
; if (xw < prcwClip->xwLeft || xw + vfti.dxpBorder > prcwClip->xwRight)
;   return;
;PAUSE
	cmp	dx,[bx.xwLeftRc]
	jl	DRBTemp
	mov	ax,[vfti.dxpBorderFti]
	add	ax,dx
	cmp	ax,[bx.xwRightRc]
	jg	DRBTemp
; if (yw < prcwClip->ywTop)
;   {
;PAUSE
	mov	ax,[bx.ywTopRc]
	sub	ax,[yw]
	jle	DRB10
;   dyw -= prcwClip->ywTop - yw;
	sub	[dyw],ax
;   yw = prcwClip->ywTop;
	add	[yw],ax   ; trickey, eh?
;   }
DRB10:
; if ((dyw = min(dyw, prcwClip->ywBottom - yw)) <= 0)
;   return;
	mov	cx,[bx.ywBottomRc]
	sub	cx,[yw]
	cmp	cx,[dyw]
	jle	DRB20
;PAUSE
	mov	cx,[dyw]
DRB20:
	or	cx,cx
	jle	LDRBExit
;PAUSE
; Assert(yw >= prcwClip.ywTop && dyw > 0 && yw + dyw <= prcw.ywBottom);
ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[yw]
	cmp	ax,[bx.ywTopRc]
	jl	DRB30
	cmp	cx,0
	jle	DRB30
	add	ax,cx
	cmp	ax,[bx.ywBottomRc]
	jle	DRB40
DRB30:
	mov	ax,midDisp1n
	mov	bx,1060
	cCall	AssertProcForNative,<ax,bx>
DRB40:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif	;DEBUG
; }
DRB50:

;PAUSE
	mov	di,[hdc]
	push	di
	push	dx
	push	[yw]
	push	[vfti.dxpBorderFti]
	push	cx		;push arguments used in either case
;       if (vlm == lmPreview)
;           DrawPrvwLine(hdc, xw, yw, vfti.dxpBorderFti, dyw, colAuto);

	cmp	[vlm],lmPreview
	jne	DRB60
	mov	ax,1
	push	ax
	cCall	DrawPrvwLine,<>
DRBTemp:
	jmp	short LDRBExit

;	else
;	    {
;	    hbrOld = SelectObject(hdc, vfli.fPrint ? vpri.hbrText:vsci.hbrText);
DRB60:
	push	di
	mov	ax,[vpri.hbrTextPri]
	cmp	[vfli.fPrintFli],0
	jne	DRB70
	mov	ax,[vsci.hbrTextSci]
DRB70:
	push	ax
ifdef DEBUG
	cCall	SelectObjectFP,<>
else ;!DEBUG
ifdef HYBRID
	cCall	SelectObjectPR,<>
else ;!HYBRID
	cCall	SelectObject,<>
endif ;!HYBRID
endif ;!DEBUG
	xchg	ax,si

;	    PatBlt(hdc, xp, yp, vfti.dxpBorderFti, dyp, PATCOPY);
	mov	dx,HI_PATCOPY
	mov	ax,LO_PATCOPY
	push	dx
	push	ax
ifdef DEBUG
	cCall	PatBltFP,<>
else ;!DEBUG
ifdef HYBRID
	cCall	PatBltPR,<>
else ;!HYBRID
	cCall	PatBlt,<>
endif ;!HYBRID
endif ;!DEBUG

;	    SelectObject(hdc, hbrOld);
;	    }
ifdef DEBUG
	cCall	SelectObjectFP,<di, si>
else ;!DEBUG
ifdef HYBRID
	cCall	SelectObjectPR,<di, si>
else ;!HYBRID
	cCall	SelectObject,<di, si>
endif ;!HYBRID
endif ;!DEBUG
LDRBExit:
;}
cEnd

;End of DrawRevBar


;-------------------------------------------------------------------------
;	DisplayFliCore( ww, rcwClip, dxpToXw, ywLine )
;-------------------------------------------------------------------------
;/* D I S P L A Y  F L I  C O R E */
;/* paint line described by vfli onto DC hdc, at coordinates described
;   by *pptPen. There is no need to paint beyond rcwClip.
;
;   Returns fTrue if line contains text with the revision mark property
;*/
;native DisplayFliCore( ww, rcwClip, dxpToXw, ywLine )
;int		 ww;
;struct RC	 rcwClip;
;int		 dxpToXw, ywLine;
;{
;	 int dypPen;
;	 int ich;
;	 int ichNext;
;	 CP dcpVanish;
;	 struct CHR *pchr;
;	 int hpsPosPrev;
;	 int tlc;
;	 HDC hdc = PwwdWw(ww)->hdc;
;	 int eto = ETO_OPAQUE;
;	 BOOL fErased = fFalse;
;	 struct ULS uls;
;	 struct PT ptPen;
;	 struct RC	rcOpaque, rcErase, rcAdjusted;
;	 BOOL	    	fRcAdjusted;
;	 BOOL		fNoSkip = fFalse;
;	 int dxp;
;	 int ilevel;
;	 struct CHP *pchp;
;	 struct PT  ptPenT;  /* A copy of ptPen to be used by DrawBrcl */

maskfPrvwPrintLocal	equ 001h
maskfOverLocal		equ 004h
maskfErasedLocal	equ 008h
maskfUseRcEraseLocal	equ 010h
maskfRcAdjustedLocal	equ 020h
maskfNoSkipLocal	equ 040h

; %%Function:N_DisplayFliCore %%Owner:BRADV
cProc	N_DisplayFliCore,<PUBLIC,FAR>,<si,di>
	ParmW	ww
	errnz	<(xpLeftRc)-0>
	errnz	<(ypTopRc)-2>
	errnz	<(xpRightRc)-4>
	errnz	<(ypBottomRc)-6>
	ParmW	rcwClipYpBottomRc
	ParmW	rcwClipXpRightRc
	ParmW	rcwClipYpTopRc
	ParmW	rcwClipXpLeftRc
	ParmW	dxpToXw
	ParmW	ywLine

	LocalW	eto
	LocalW	hdc
	LocalW	ich
	LocalD	dcpVanish
	LocalW	hpsPosPrev
	LocalW	dypPen
	LocalW	xpVar
	LocalW	ypVar
	LocalW	pchr
	LocalW	cchChop
ifdef DEBUG
	LocalW	fPchpInitialized
endif ;/* DEBUG */
	LocalV	ptPen,cbPtMin
	LocalV	ptPenT,cbPtMin
	LocalV	rcOpaque,cbRcMin
	LocalV	rcErase,cbRcMin
	LocalV	rcT,cbRcMin
	LocalV	uls,cbUlsMin
	LocalV	rcAdjusted,cbRcMin
ifdef SHOWFLD
	LocalV	szFrame,40
endif ;/* SHOWFLD */

cBegin
	mov	si,[ww]
	cCall	N_PwwdWw,<si>
	xchg	ax,bx
	mov	cx,[bx.hdcWwd]
	mov	[hdc],cx

;	ptPenT.xw = ptPen.xw = vfli.xpLeft + dxpToXw;
;	ptPenT.yw = ptPen.yw = ywLine - vfli.dypBase;
	mov	ax,[vfli.xpLeftFli]
	add	ax,[dxpToXw]
	mov	[ptPen.xwPt],ax
	mov	[ptPenT.xwPt],ax
	mov	ax,[ywLine]
	sub	ax,[vfli.dypBaseFli]
	mov	[ptPen.ywPt],ax
	mov	[ptPenT.ywPt],ax

;	Assert( ww != wwNil );
ifdef DEBUG
	cmp	[ww],wwNil
	jne	DFC01
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,midDisp1n
	mov	bx,1001
	cCall	AssertProcForNative,<ax,bx>

	pop	dx
	pop	cx
	pop	bx
	pop	ax
DFC01:
endif ;/* DEBUG */

;	Assert( hdc !=NULL );
ifdef DEBUG
	cmp	[hdc],NULL
	jne	DFC02
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,midDisp1n
	mov	bx,1002
	cCall	AssertProcForNative,<ax,bx>

	pop	dx
	pop	cx
	pop	bx
	pop	ax
DFC02:
endif ;/* DEBUG */

;	 uls.ww = ww;
	mov	[uls.wwUls],si

;	 uls.hdc = hdc;
	mov	[uls.hdcUls],cx

;	 uls.grpfUL = 0;
	xor	cx,cx
	mov	[uls.grpfULUls],cx

ifdef DEBUG
	mov	[fPchpInitialized],fFalse
endif ;/* DEBUG */

;	 ich = 0;
;	 dcpVanish = cp0;
;	 hpsPosPrev = 0;
;	 dypPen = 0;
	mov	[ich],cx
	mov	[OFF_dcpVanish],cx
	mov	[SEG_dcpVanish],cx
	mov	bptr ([hpsPosPrev]),cl
	mov	[dypPen],cx

	push	ds
	pop	es
;	 blt(&rcwClip,&rcOpaque,cwRC);
	errnz	<cbRcMin - 8>
	lea	si,[rcwClipXpLeftRc]
	lea	di,[rcOpaque]
	movsw
	movsw
	movsw
	movsw

;	 blt(&rcwClip,&rcErase,cwRC);
	errnz	<cbRcMin - 8>
	lea	si,[rcwClipXpLeftRc]
	lea	di,[rcErase]
	movsw
	movsw
	movsw
	movsw

;	 pchr = &(**vhgrpchr)[0];
	mov	si,[vhgrpchr]
	mov	si,[si]

;	simultaneously put (vfli.fPrint || vfPrvwDisp) in a register and clear
;	fDacChr, fIichSpaceMac, fErased, fNoSkip.
	mov	ax,[vfPrvwDisp]
	or	al,[vfli.fPrintFli]
ifdef DEBUG
	test	ax,0FFFEh
	je	DFC03
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,midDisp1n
	mov	bx,1003
	cCall	AssertProcForNative,<ax,bx>

	pop	dx
	pop	cx
	pop	bx
	pop	ax
DFC03:
endif ;/* DEBUG */

;	     eto = ETO_CLIPPED;
;	     if (!fPrvwPrint)
;		eto |= ETO_OPAQUE
	errnz	<ETO_CLIPPED - 4>
	mov	dx,ETO_CLIPPED
	test	al,maskfPrvwPrintLocal
	jne	DFC04
	or	dl,ETO_OPAQUE
DFC04:
	mov	eto,dx
	

;/* main loop, goes around for every chr */
;/* pen position must be at the baseline in front of the next character. */

;	At this point we know dac == dacNop and dac is only set to
;	dacNop above this point so we can jump straight to the case
;	dacNop code from here.
	jmp	DFC28

;	The beginning of the for loop has been moved down in the assembler
;	version (at DFC265)
;	 for (;;)
;		 {
;			 {
;			 int chrm;
;/* if is used instead of switch for better space/speed */
;	switch is smaller and probably faster in assembler

DFCJumpTable:
	errnz	<chrmEnd - 02h>
	dw	offset DFC26
	errnz	<chrmTab - 04h>
	dw	offset DFC05
	errnz	<chrmVanish - 06h>
	dw	offset DFC17
	errnz	<chrmFormula - 08h>
	dw	offset DFC22
	dw	0	    ; no chrm of length 0Ah
	errnz	<chrmFormatGroup - 0Ch>
	dw	offset DFC24
	dw	0	    ; no chrm of length 0Eh
	dw	0	    ; no chrm of length 10h
	errnz	<chrmChp - 12h>
	dw	offset DFC12
	errnz	<chrmDisplayField - 14h>
	dw	offset DFC18


Ltemp019:
	db	0C3h	    ;near ret (ret generates far ret opcode here)

;			 if ((chrm = pchr->chrm) == chrmTab)
;				 {
DFC05:

;/* special chrmTab for visible characters: special draw for pub chars, ignore all others. */
;				 bchrCur = (char *) pchr - (char *)*vhgrpchr;
;				 bchpCur = (char *) pchp - (char *)*vhgrpchr;
;				 if (((struct CHRT *)pchr)->ch == 0)
;					 {
	;Assembler note: convert pchr & pchp to and from offsets around
	;calls to DrawTlc and DrawChPubs, not here.
	xor	bx,bx
	mov	bl,[si.chChrt-cbCHRT]
	or	bx,bx
	jnz	DFC103

;					 dxp = vfli.rgdxp[ich];
	mov	bx,[ich]
	shl	bx,1
	mov	cx,[bx.vfli.rgdxpFli]

;					 if ((tlc = ((struct CHRT *)pchr)->tlc) || pchp->kul)
;					     {
	mov	dl,[si.tlcChrt-cbCHRT]
	or	dl,dl
	jne	DFC055
	test	[di.kulChp],maskKulChp
	jz	DFC10

DFC055:

;/* draw tab leader or underlining.  Blank first. */
;					     int xpNew;  struct RC rcT;
;					     int fOver=fFalse;
	and	al,NOT maskfOverLocal

;					     xpNew = ptPen.xp + dxp;
	mov	bx,[ptPen.xpPt]
	add	bx,cx

;					     if (!fErased && !fPrvwPrint)
;						 {
	test	al,maskfErasedLocal+maskfPrvwPrintLocal
	jne	DFC07

;						 blt( &rcErase, &rcT, cwRC );
;						 rcT.xpRight = xpNew;
	or	al,maskfUseRcEraseLocal
;	DFC54 performs:
;	blt(fUseRcErase ? &rcErase : &rcOpaque, &rcT, cwRC );
;	rcT.xpRight = bx;
;	no registers are altered
	call	DFC54

;						 PatBltRc( hdc, &rcT, vsci.ropErase );
;	DFC56 performs:
;	PatBltRc( hdc, fUseRcErase ? &rcErase : &rcT,
;		  vsci.ropErase );
;	no registers are altered
	and	al,NOT maskfUseRcEraseLocal
	call	DFC56

;						 rcOpaque.xpLeft = rcErase.xpLeft
;							 = xpNew;
	mov	[rcErase.xpLeftRc],bx
	mov	[rcOpaque.xpLeftRc],bx

;						 }
DFC07:

;					     if (!tlc)
;						 {
	or	dl,dl
	jne	DFC09

;/* 4th param means round up. Do not round up iff next char is not ul'd */
;						 struct CHR *pchrT =
;						    ((struct CHRT *)pchr) + 1;

;						fOver = !(pchrT->chrm == chrmChp &&
;						    pchrT->ich == ich + 1 &&
;						  pchrT->chp.kul != kulSingle);
	or	al,maskfOverLocal
	cmp	[si.chrmChr],chrmChp
	jne	DFC08
	mov	bl,bptr [ich]
	inc	bx
	cmp	[si.ichChr],bl
	jne	DFC08
	mov	bl,[si.chpChr.kulChp]
	and	bl,maskKulChp
	cmp	bl,kulSingle SHL ibitKulChp
	je	DFC08
	and	al,NOT maskfOverLocal
DFC08:

;						}
DFC09:

;/* note: underlined tabs must be drawn as underlined space "leaders" */
;					     DrawTlc( hdc, ptPen, tlc,
;						      dxp, pchp, fOver, &rcwClip );
	push	ax	;save flags
	push	cx	;save dxp
	push	[hdc]
	errnz	<(ypPt) - (xpPt) - 2>
	push	[ptPen.ypPt]
	push	[ptPen.xpPt]
	xor	dh,dh
	push	dx
	push	cx
	push	di
;	NOTE: we know that ah is zero here because it has no flags.
	and	al,maskfOverLocal
	push	ax
	lea	ax,[rcwClipXpLeftRc]
	push	ax
	;DFC82 converts di and [pchr] from pointers to offsets into
	;*vhgrpchr.  Upon return bx is -pchr.  Only bx is altered.
	call	DFC82
	add	si,bx
	cCall	DrawTlc,<>
	;DFC83 converts di and [pchr] to pointers from offsets into
	;*vhgrpchr.  Upon return bx is pchr.  Only bx is altered.
	call	DFC83
	add	si,bx
	pop	cx	;restore dxp
	pop	ax	;restore flags

;					     }
DFC10:

;					 ptPen.xp += dxp;
;					 ich++;
;					 }
	add	[ptPen.xpPt],cx
	jmp	short DFC107

;				else if ((uns)(ch - chPubMin) < (chPubMax-chPubMin))
;					{
;					DrawChPubs( hdc, &ptPen, ch, &rcOpaque,
;						    &rcErase, fErased, eto);
;					ich++;
;					}
DFC103: 	; bx = pchr->ch
	cmp	bl,chPubMin
	jb	DFC11
	cmp	bl,chPubMax
	jae	DFC11
	push	ax	;save flags
	push	cx	;save dxp
	and	ax,maskfErasedLocal
	push	[hdc]
	lea	dx,[ptPen]
	push	dx
	push	bx		; ch
	lea	dx,[rcOpaque]
	push	dx
	lea	dx,[rcErase]
	push	dx
	push	ax		; fErased
	push	[eto]
	;DFC82 converts di and [pchr] from pointers to offsets into
	;*vhgrpchr.  Upon return bx is -pchr.  Only bx is altered.
	call	DFC82
	add	si,bx
	cCall	DrawChPubs,<>
	;DFC83 converts di and [pchr] to pointers from offsets into
	;*vhgrpchr.  Upon return bx is pchr.  Only bx is altered.
	call	DFC83
	add	si,bx
	pop	cx  	;restore dxp
	pop	ax	;restore flags
DFC107:
	inc	bptr ([ich])
	

DFC11:

;				 (char *) pchr = (char *)*vhgrpchr + bchrCur;
;				 (char *) pchp = (char *)*vhgrpchr + bchpCur;
;				 ((struct CHRT *)pchr)++;
;				 }
	;Assembler note: convert pchr & pchp to and from offsets around
	;calls to DrawTlc and DrawChPubs, not here.
	db	0C3h	    ;near ret (ret generates far ret opcode here)


;			 else if (chrm == chrmChp)
;				 {
DFC12:

;				 int bchrCur = (char *) pchr - (char *)*vhgrpchr;
	mov	bx,[vhgrpchr]
	sub	si,[bx]
	push	si

	push	ax	;save flags

;/* these show if any underline at level 1 or level 2 are set in current looks */
;				 if (uls.grpfUL)
;					 EndULPast(&uls,ptPen.xp,ich,&rcwClip);

;	LN_EndULPast performs:
;	if (uls.grpfUL) EndULPast(&uls,ptPen.xp,ich,&rcwClip);
;	ax, bx, cx and dx are altered
	call	LN_EndULPast

; native note: si = bchrCur, use this to set pchp correctly
;				 pchp = &pchr->chp;
;PAUSE
	lea	di,[si.chpChr-cbCHR]
	mov	bx,[vhgrpchr]
	add	di,[bx]
ifdef DEBUG
	mov	[fPchpInitialized],fTrue
endif ;/* DEBUG */

;				 if (ich < vfli.ichSpace3)
;					 uls.kul = pchr->fcid.kul;
	mov	al,[vfli.ichSpace3Fli]
	cmp	bptr ([ich]),al
	jae	DFC13
	mov	al,[di.fcidChr.kulFcid-(chpChr)]
	and	al,maskKulFcid
	mov	cl,ibitKulFcid
	shr	al,cl
	mov	[uls.kulUls],al
DFC13:

;				 if (hpsPosPrev != pchp->hpsPos)
;					 {
	mov	al,[di.hpsPosChp]
	cmp	al,bptr ([hpsPosPrev])
	je	DFC15

;					 int dypOffset, dypPenMotion;
;					 hpsPosPrev = pchp->hpsPos;
	mov	bptr ([hpsPosPrev]),al

;				     /* 2 * 72 converts half points to inches */
;					 dypOffset = NMultDiv(
;					     (hpsPosPrev >= 128) ?
;					     (hpsPosPrev - 256) : hpsPosPrev,
;					     vfti.dypInch, 2 * 72);
	cbw
	push	ax
	push	[vfti.dypInchFti]
	mov	ax,2*72
	push	ax
	cCall	NMultDiv,<>

;					 dypPenMotion = dypOffset - dypPen;
	sub	ax,[dypPen]

;					 ptPen.yp -= dypPenMotion;
	sub	[ptPen.ypPt],ax

;					 dypPen += dypPenMotion;
	add	[dypPen],ax

;					 }
DFC15:

;			       LoadFcidFull(pchr->fcid);
	errnz	<cbFcid - 4>
	push	[di.fcidChr.HI_lFcid-(chpChr)]
	push	[di.fcidChr.LO_lFcid-(chpChr)]
ifdef DEBUG
	cCall	S_LoadFcidFull,<>
else ;not DEBUG
	cCall	N_LoadFcidFull,<>
endif ;DEBUG

;/* treat begin underline */
;				 if (uls.grpfUL)
;					 {
;					 uls.pt = ptPen;
;					 uls.dypPen = dypPen;
;					 }
	cmp	[uls.grpfULUls],fFalse
	je	DFC16
	push	ds
	pop	es
	lea	si,[ptPen]
	lea	di,[uls.ptUls]
	errnz	<cbPtMin - 4>
	movsw
	movsw
	mov	ax,[dypPen]
	mov	[uls.dypPenUls],ax
DFC16:
	pop	ax	;restore flags

;				 /* reset pointers into heap */
;				 pchr = (char *)*vhgrpchr + bchrCur;
	pop	si
	mov	bx,[vhgrpchr]
	add	si,[bx]

;				 pchp = &pchr->chp;
	lea	di,[si.chpChr - cbCHR]

;				 ForeColor(hdc, pchp->ico);
	push	ax	;save flags
	mov	al,[di.icoChp]
	errnz	<maskIcoChp - 00Fh>
	and	ax,maskIcoChp
	cCall	ForeColor,<[hdc],ax>
	pop	ax	;restore flags


;				 /* advance pchr to next CHR */
;				 pchr = (char *)pchr + sizeof(struct CHR);
;				 }
	db	0C3h	    ;near ret (ret generates far ret opcode here)

;			 else if (chrm == chrmVanish)
;				 {
DFC17:

;/* real cp can be calculated as vfli.cpMin + ich + dcpVanish */
;				 dcpVanish += ((struct CHRV *)pchr)->dcp;
	mov	bx,[si.LO_dcpChrv-cbCHRV]
	add	[OFF_dcpVanish],bx
	mov	bx,[si.HI_dcpChrv-cbCHRV]
	adc	[SEG_dcpVanish],bx

;				 (char *)pchr += cbCHRV;
;				 }
	db	0C3h	    ;near ret (ret generates far ret opcode here)

;			 else if (chrm == chrmDisplayField)
;				 {
DFC18:
;                                if (uls.grpfUL)
;                                        {
;                                        uls.xpLim = ptPen.xp;
;					 EndUL(&uls, &rcwClip);
;                                        }
;	DFC57 performs:
;	if (uls.grpfUL) { bchrCur = ..., bchpCur = ...;
;			uls.xpLim = ptPen.xp; EndUL(&uls, &rcwClip); 
;			pchr = ..., pchp = ...}
;	si = pchr, di = pchp, -> these get properly updated
;	bx, cx and dx are altered
;PAUSE
	call	DFC57


;				 int xpNew = ptPen.xp +
;					     ((struct CHRDF *)pchr)->dxp;
	mov	bx,[ptPen.xpPt]
	add	bx,[si.dxpChrdf-cbCHRDF]

;				 int bchrCur = (char *)pchr - (char *)*vhgrpchr;
;				 int bchpCur = (char *)pchp - (char *)*vhgrpchr;
;				 struct PT ptT;
;				 struct RC rcT;

;	do this stuff later in the assembler version
;				 ptT.xp = ptPen.xp;
;				 ptT.yp = ptPen.yp + vfli.dypBase -
;					 vfli.dypLine + dypPen;

;				 /*  chDisplayField is a placeholder
;				     character in rgch for the CHRDF.  Skip
;				     over it.
;				 */
;				 Assert (vfli.rgch [ich] == chDisplayField);
;				 Assert (vfli.rgdxp [ich] ==
;					 ((struct CHRDF *)pchr)->dxp);
;				 Assert (ich == pchr->ich);
ifdef DEBUG
;	/* Do this debug code with a call so as not to mess up
;	short jumps */
	call	DFC59
endif ;/* DEBUG */

;				 ich++;
	inc	bptr ([ich])

;#ifdef SHOWFLD
;				 CommSzNum ("Displaying CHRDF: ",
;				     (int)vfli.cpMin + dcpVanish + pchr->ich);
;#endif /* SHOWFLD */
ifdef SHOWFLD
;	/* Do this showfld code with a call so as not to mess up
;	short jumps */
	call	DFC63
endif ;/* SHOWFLD */

;				 blt( &rcOpaque, &rcT, cwRC );
;				 rcT.xpRight = xpNew;
	and	al,NOT maskfUseRcEraseLocal
;	DFC54 performs:
;	blt(fUseRcErase ? &rcErase : &rcOpaque, &rcT, cwRC );
;	rcT.xpRight = bx;
;	no registers are altered
	call	DFC54

;				 if (!fErased && !fPrvwPrint)
;				     {
	test	al,maskfErasedLocal+maskfPrvwPrintLocal
	jne	DFC20

;				     PatBltRc( hdc, &rcT, vsci.ropErase );
;	DFC56 performs:
;	PatBltRc( hdc, fUseRcErase ? &rcErase : &rcT,
;		  vsci.ropErase );
;	no registers are altered
	and	al,NOT maskfUseRcEraseLocal
	call	DFC56

;				     rcErase.xpLeft = rcOpaque.xpLeft = xpNew;
	mov	[rcErase.xpLeftRc],bx
	mov	[rcOpaque.xpLeftRc],bx

;				     }
DFC20:

	push	ax	;save flags
	push	bx	;save xpNew
;				 DisplayField (vfli.doc,
;					  vfli.cpMin + dcpVanish + pchr->ich,
;					  bchrCur, ((struct CHRDF *)pchr)->flt,
;					  hdc, ptT, ptPen.yp+dypPen,
;					  ptPen.yp, (ich < vfli.ichSpace3 ? &uls
;					  : NULL), &rcT);
	push	[vfli.docFli]
;	NOTE: We know ah is zero at this point because there are no flags
;	in ah
	mov	al,[si.ichChr-cbCHRDF]
	cwd
	add	ax,[OFF_dcpVanish]
	adc	dx,[SEG_dcpVanish]
	add	ax,[vfli.LO_cpMinFli]
	adc	dx,[vfli.HI_cpMinFli]
	push	dx
	push	ax
	mov	ax,[si.fltChrdf-cbCHRDF]
;	The following two C source lines are performed above in the C version,
;	and performed here in the assembler version for convenience.
;				 int bchrCur = (char *)pchr - (char *)*vhgrpchr;
;				 int bchpCur = (char *)pchp - (char *)*vhgrpchr;
	mov	bx,[vhgrpchr]
	sub	si,[bx]
	sub	di,[bx]
	lea	bx,[si-cbCHRDF]
	push	bx
	push	ax
	push	[hdc]
;	create ptT and push it now
;				 ptT.xp = ptPen.xp;
;				 ptT.yp = ptPen.yp + vfli.dypBase -
;					 vfli.dypLine + dypPen;
	errnz	<(xpPt) - 0>
	errnz	<(ypPt) - 2>
	mov	ax,[ptPen.ypPt]
	mov	dx,ax
	add	ax,[vfli.dypBaseFli]
	sub	ax,[vfli.dypLineFli]
	add	ax,[dypPen]
	push	ax
	push	[ptPen.xpPt]
	mov	ax,dx
	add	ax,[dypPen]
	push	ax
	push	dx
	errnz	<NULL>
	xor	cx,cx
	mov	al,[vfli.ichSpace3Fli]
	cmp	bptr ([ich]),al
	jae	DFC21
	lea	cx,[uls]
DFC21:
	push	cx
	lea	ax,[rcT]
	push	ax
	cCall	DisplayField,<>

;				 pchr = (char *)*vhgrpchr + bchrCur;
;				 pchp = (char *)*vhgrpchr + bchpCur;
	mov	bx,[vhgrpchr]
	add	si,[bx]
	add	di,[bx]

;				 dcpVanish += ((struct CHRDF *)pchr)->dcp -1;
	mov     ax,-1
        cwd
        add	ax,[si.LO_dcpChrdf-cbCHRDF]
        adc	dx,[si.HI_dcpChrdf-cbCHRDF]
        add	[OFF_dcpVanish],ax
        adc	[SEG_dcpVanish],dx

;				 ptPen.xp = xpNew;
	pop	ax	;restore xpNew
	mov	[ptPen.xpPt],ax

	pop	ax	;restore flags

;				 (char *) pchr += cbCHRDF;
;				 }
	db	0C3h	    ;near ret (ret generates far ret opcode here)

;			 else if (chrm == chrmFormula)
;				 {
DFC22:
	push	si	;save pchr
	push	di	;save pchp
	push	ax	;save flags
;                                if (uls.grpfUL)
;                                        {
;                                        uls.xpLim = ptPen.xp;
;					 EndUL(&uls, &rcwClip);
;                                        }
;	DFC57 performs:
;	if (uls.grpfUL) { bchrCur = ..., bchpCur = ...;
;			uls.xpLim = ptPen.xp; EndUL(&uls, &rcwClip); 
;			pchr = ..., pchp = ...}
;	si = pchr, di = pchp, -> these get properly updated
;	bx, cx and dx are altered
;PAUSE
	call	DFC57

	mov	ax,[si.fLineChrf-cbCHRF]

;				 int xpNew, ypNew;
;				 xpNew = ptPen.xp +
;					 ((struct CHRF *) pchr)->dxp;
	mov	di,[si.dxpChrf-cbCHRF]
	add	di,[ptPen.xpPt]

;				 ypNew = ptPen.yp +
;					 ((struct CHRF *) pchr)->dyp;
	mov	si,[si.dypChrf-cbCHRF]
	add	si,[ptPen.ypPt]

ifdef NOTUSED
;				 dxp = ((struct CHRF *) pchr)->dxp;
	mov	bx,[si.dxpChrf-cbCHRF]
	mov	[dxp],bx
endif ;NOTUSED

;				 if (((struct CHRF *) pchr)->fLine)
;					 {
	or	ax,ax
	je	Ltemp001

	push	di	;save xpNew
	push	si	;save ypNew

;					 iLevel = SaveDC(hdc);
	mov	dx,[hdc]
	push	dx	;save hdc
ifdef DEBUG
	cCall	SaveDCFP,<dx>
else ;!DEBUG
ifdef HYBRID
	cCall	SaveDCPR,<dx>
else ;!HYBRID
	cCall	SaveDC,<dx>
endif ;!HYBRID
endif ;!DEBUG
	pop	dx	;restore hdc

;					if (vfPrvwDisp)
;					   {
	mov	cx,[vfPrvwDisp]
	jcxz	DFC222

;					   InflateRect((LPRECT)&rcOpaque,5,5);
;	DFC81 performs
;		InflateRect(si,bx)
;	assumes prc in si, offset in bx, no registers are altered.
	lea	si,[rcOpaque]
	mov	bx,10
	call	DFC81

;					   IntersectClipRect(hdc,
;							     rcOpaque.xpLeft,
;							     rcOpaque.ypTop,
;							     rcOpaque.xpRight,
;							     rcOpaque.ypBottom);
;	DFC69 performs
;	IntersectClipRect(hdc, si.xpLeft, si.ypTop, si.xpRight, si.ypBottom);
;	bx, cx, dx and si are altered.
	push	dx	;save hdc
	push	si	;save &rcOpaque
	call	DFC69
	pop	si	;restore &rcOpaque
	pop	dx	;restore hdc

;					   InflateRect((LPRECT)&rcOpaque,-5,-5);
;	DFC81 performs
;		InflateRect(si,bx)
;	assumes prc in si, offset in bx, no registers are altered.
	mov	bx,-10
	call	DFC81

;					   DrawPrvwLine(hdc, ptPen.xp,
;						ptPen.yp, xpNew - ptPen.xp,
;						ypNew - ptPen.yp, colText);

	pop	si	;restore ypNew
	push	si	;save ypNew
	push	dx	; hdc argument for RestoreDC
	push	ax	; iLevel argument for RestoreDC
	mov	bx,[ptPen.xpPt]
	mov	cx,[ptPen.ypPt]
	push	dx
	push	bx
	push	cx
	sub	di,bx
	push	di
	sub	si,cx
	push	si
	mov 	ax,2
	push	ax
	cCall	DrawPrvwLine,<>

;					   }
	jmp	short DFC228
Ltemp001:
	jmp	short DFC23

;					else
;					   {
DFC222:

;					    Assert( vsci.hpen && ilevel );
ifdef DEBUG
;	/* Assert( vsci.hpen && iLevel) with a call so as not to mess up
;	short jumps */
	call	DFC66
endif ;/* DEBUG */

	push	dx	;hdc argument for RestoreDC
	push	ax	;iLevel argument for RestoreDC

	push	dx	;hdc argument for DrawFormulaLine
	push	di	;xpNew argument for DrawFormulaLine
	push	si	;ypNew argument for DrawFormulaLine
	push	[ptPen.xpPt]   ;xp argument for DrawFormulaLine
	push	[ptPen.ypPt]   ;yp argument for DrawFormulaLine
	mov	al,[vfli.fPrintFli]
ifdef DEBUG
;	/* Assert( vfli.fPrint & 0xFE == 0 ) with a call so as not to mess up
;	short jumps */
	call	DFC85
endif ;/* DEBUG */
	or	al,8+4+2
	push	ax	;grpf argument for DrawFormulaLine

;
;    					IntersectClipRect(hdc, rcOpaque.xpLeft,
;										rcOpaque.ypTop, rcOpaque.xpRight,
;										rcOpaque.ypBottom);
;	DFC69 performs
;	IntersectClipRect(hdc, si.xpLeft, si.ypTop, si.xpRight, si.ypBottom);
;	bx, cx, dx and si are altered.
	lea	si,[rcOpaque]
	call	DFC69

;					    Assert(!(vfli.fPrint & 0xFE));
;					    DrawFormulaLine(hdc, xpNew, ypNew,
;						    ptPen.xp, ptPen.yp,
;						    vfli.fPrint /* fPrint */
;						    + 8 /* fCreate */
;						    + 4 /* fMove */
;						    + 2 /* fDestroy */);
	cCall	DrawFormulaLine,<>

;					   }
DFC228:
;					 RestoreDC (hdc, iLevel);
ifdef DEBUG
	cCall	RestoreDCFP,<>
else ;!DEBUG
ifdef HYBRID
	cCall	RestoreDCPR,<>
else ;!HYBRID
	cCall	RestoreDC,<>
endif ;!HYBRID
endif ;!DEBUG

;					 }
	pop	si	;restore ypNew
	pop	di	;restore xpNew

DFC23:
;				 ptPen.xp = xpNew;
;				 ptPen.yp = ypNew;
;				 fNoSkip = vfli.fPrint;
	mov	[ptPen.xpPt],di
	mov	[ptPen.ypPt],si
	pop	ax	;restore flags
	and	al,NOT maskFNoSkipLocal
	cmp	[vfli.fPrintFli],fFalse
	je	DFC235
	or	al,maskFNoSkipLocal
DFC235:
	pop	di	;restore pchp
	pop	si	;restore pchr

;				 (char *) pchr += chrmFormula;
;				 }
	db	0C3h	    ;near ret (ret generates far ret opcode here)

;			 else if (chrm == chrmFormatGroup)
;				 {
DFC24:

;				 fErased = fTrue;
	or	al,maskfErasedLocal

;				 rcOpaque.xpRight = rcwClip.xpRight;
	mov	bx,[rcwClipXpRightRc]
	mov	[rcOpaque.xpRightRc],bx

;				 if (eto & ETO_OPAQUE)
;				     /*  erase the rest of the line */
;				     {
	errnz	<ETO_OPAQUE - 2>
	test	bptr ([eto]),ETO_OPAQUE
	je	DFC25

;				     PatBltRc (hdc, &rcErase, vsci.ropErase);
;	DFC56 performs:
;	PatBltRc( hdc, fUseRcErase ? &rcErase : &rcT,
;		  vsci.ropErase );
;	no registers are altered
	or	al,maskfUseRcEraseLocal
	call	DFC56

;				     SetBkMode (hdc, TRANSPARENT);
;				     eto &= ~ETO_OPAQUE;
;	DFC71 performs:
;	SetBkMode(hdc, TRANSPARENT);
;	eto &= ~ETO_OPAQUE;
;	bx, cx, and dx are altered
	call	DFC71

;				     }
DFC25:

;				 /* note that rcOpaque and rcErase will no
;				    longer change (rcErase not used) */
;				 (char *) pchr += chrmFormatGroup;
;				 }
	db	0C3h	    ;near ret (ret generates far ret opcode here)

;			 else /*if (chrm == chrmEnd)*/
;				 {
DFC26:

;				 goto LEnd;
	pop	bx	;remove return address to DFC28
	jmp	LEnd

;	This code has been moved from above in the assembler version to
;	reduce jumps
;	 for (;;)
;		 {
DFC265:

;			 {
;			 int chrm;

;/* if is used instead of switch for better space/speed */
;	switch is used in the assembler version because it is smaller
;	and probably faster
	mov	cl,[si.chrmChr]
	xor	ch,ch
	add	si,cx	    ;advance pchr now to save code
ifdef DEBUG
;	Assert we have one of chrmEnd, chrmTab, chrmVanish, chrmVanish
;	chrmFormula, chrmDisplayField, chrmFormatGroup, chrmChp with a
;	call so as not to mess up short jumps.
	call	DFC79
endif ;DEBUG
;	Push offset DFC28 because all chrm routines except chrmEnd ret
;	to there after they complete
	mov	bx,offset DFC28
	push	bx
	mov	bx,cx
	jmp	cs:[bx.(offset DFCJumpTable)-2]  ;no "zero" chrm so subtract 2
DFC28:

;LDacNop:
;/* calculate the next action and the ich it should take place at or perform
;   the action immediately */
;/* see comments at the beginning of the module */
LDacNop:

;		 ichNext = pchr->ich;
	mov	cl,[si.ichChr]

;		 if (ichNext > ich)
;			 {
	mov	bx,[ich]
	cmp	cl,bl
	jbe	DFC265
	mov	[pchr],si   ;save pchr
	push	cx	    ;save ichNext

;			 int *pdxp;
;			 int cch;
;			 int yp = ptPen.yp - vfti.dypAscent;
	mov	si,[ptPen.ypPt]
	sub	si,[vfti.dypAscentFti]
	mov	[ypVar],si

;			 /* if we got here via LDacNop, pchp is unitialized */
ifdef DEBUG
;	/* Assert(fPchpInitialized) with a call so as not to mess up
;	short jumps */
	call	DFC72
endif ;/* DEBUG */

;/* now show the text & update the pen position */
;/* min and max avoid printing chars beyond ichSpace3, like underlined spaces
;   however, for display, may need to display visi spaces, para marks, etc */
;   				
;   			if (fPrvwPrint && ichNext > vfli.ichSpace3)
;				{
;				cch = vfli.ichSpace3;
;				cchChop = ichNext - vfli.ichSpace3;
;				}
;			else
;				{
;				cch = ichNext;
;				cchChop = 0;
;				}
;			cch = max( cch - ich, 0 );

	test	al,maskfPrvwPrintLocal
	jz 	DFC29
	mov	dl,cl
	sub	dl,[vfli.ichSpace3Fli]
	sbb	dh,dh
	jb	DFC29
	mov	cl,[vfli.ichSpace3Fli]
	db	03Dh	;turns next "xor dx,dx" into "cmp ax,immediate"
DFC29:
	xor	dx,dx
	mov	[cchChop],dx
	xor	ch,ch	;cch must be a word for the loop instruction below.
	sub	cx,bx
	jge	DFC295
	xor	cx,cx
DFC295:

;			 pdxp = &vfli.rgdxp [ich];
	mov	si,bx
	lea	si,[bx+si.vfli.rgdxpFli]

;/* Optimizations: Skip stuff that is clipped off anyway */

;/* Yes, but italic characters are a real bummer! */
;	while ((ptPen.xp + *pdxp + vfti.dxpOverhang <= rcwClip.xpLeft) && cch)
;				 {
	mov	dx,[ptPen.xpPt]
	push	ax	;save flags
	push	si	;save pdxpFirst = pdxp;
	jcxz	DFC31
;PAUSE
	mov	bx,[rcwClipXpLeftRc]
	mov	ax,[vfti.dxpOverhangFti]
	sub	bx, ax
DFC30:

;				 cch--;
;				 ptPen.xp += *(pdxp++);
;				 ich++;
;				 }
	lodsw
	add	dx,ax
	cmp	dx,bx
	jg	DFC305
	loop	DFC30
	jmp	short DFC31
DFC305:
	sub	dx,ax
	dec	si
	dec	si
DFC31:
	pop	ax	;restore pdxpFirst
	sub	ax,si
	neg	ax
	shr	ax,1
	add	[ich],ax    ;ich += (pdxp - pdxpFirst) >> 1;
	pop	ax	;restore flags
	mov	[ptPen.xpPt],dx

;			 if (!cch)
;				 continue;
	or	cx,cx
	jnz	Ltemp018
;PAUSE
	pop	cx	    ;remove ichNext
	mov	si,[pchr]   ;restore pchr
	jmp	DFC265
Ltemp018:

;			 if (!fNoSkip && ptPen.xp >= rcwClip.xpRight)
;				 /* Already past clip region; save some work */
;				 goto LEnd;
	test	al,maskFNoSkipLocal
	jne	DFC32 	
	cmp	dx,[rcwClipXpRightRc]
	jl	DFC32
	pop	cx	;restore ichNext
	jmp	LEnd
DFC32:

;			 /* Deal with special chars */
;			 if (pchp->fSpec)
;				 {
	test	[di.fSpecChp],maskFSpecChp
	jne	Ltemp010
	jmp	DFC37
Ltemp010:

;				 int xpNew = ptPen.xp + *pdxp;
;				 struct RC rcT;
;				 int nSaveDC = 0;
;				 int bchrCur;
;				 int bchpCur;
;	Assembler note: we do this chSpec assignment when we use it.
;				 CHAR chSpec = vfli.rgch [ich];


;				if (chSpec == chPic)
;        	                        if (uls.grpfUL)
;	                                        {
;                	                        uls.xpLim = ptPen.xp;
;						EndUL(&uls, &rcwClip);
;	                                        }
	mov	bx,[ich]
	cmp	bptr ([bx.vfli.rgchFli]),chPicture
	push	dx	;save ptPen.xp
	jnz	DFC325
	
;	DFC57 performs:
;	if (uls.grpfUL) { bchrCur = ..., bchpCur = ...;
;			uls.xpLim = ptPen.xp; EndUL(&uls, &rcwClip); 
;			pchr = ..., pchp = ...}
;	si = pchr, di = pchp, -> these get properly updated
;	bx, cx and dx are altered
;PAUSE
	; native note: put pchr in si, so it gets updated
	;  AND save pdxp in [pchr]
	xchg	si,[pchr]
	call	DFC57
	; native note: put things back where they belong...
	xchg	si,[pchr]
DFC325:
	pop	bx	;restore ptPen.xp
	add	bx,[si] ;xpNew = ptPen.xp + *pdxp

;				 Assert(vfli.rgch [ich] != chDisplayField);
ifdef DEBUG
;	/* Assert(vfli.rgch [ich] != chDisplayField) with a call so
;	as not to mess up short jumps */
	call	DFC74
endif ;/* DEBUG */

;				 if (!fErased && !fPrvwPrint)
;				     { /* because ShowSpec is using TextOut,
;				       we have to erase the background */
	test	al,maskfErasedLocal+maskfPrvwPrintLocal
	jne	DFC33

;				    blt( &rcErase, &rcT, cwRC );
;				    rcT.xpRight = min(xpNew, rcwClip.xpRight);
	or	al,maskfUseRcEraseLocal
	push	bx	;save xpNew
	cmp	bx,[rcwClipXpRightRc]
	jl	DFC327
	mov	bx,[rcwClipXpRightRc]
DFC327:
;	DFC54 performs:
;	blt(fUseRcErase ? &rcErase : &rcOpaque, &rcT, cwRC );
;	rcT.xpRight = bx;
;	no registers are altered
	call	DFC54
	pop	bx	;restore xpNew

;				    if (!fPrvwPrint)
;				        PatBltRc( hdc, &rcT, vsci.ropErase );
;	DFC56 performs:
;	PatBltRc( hdc, fUseRcErase ? &rcErase : &rcT,
;		  vsci.ropErase );
;	no registers are altered
	test	al,maskfPrvwPrintLocal
	jne	DFC328
	and	al,NOT maskfUseRcEraseLocal
	call	DFC56
DFC328:

;				    rcErase.xpLeft = rcOpaque.xpLeft = xpNew;
	mov	[rcErase.xpLeftRc],bx
	mov	[rcOpaque.xpLeftRc],bx

;				    }
DFC33:

	push	ax	;save flags
	xchg	ax,cx
;				 vfmtss.cpRef = vfli.cpMin + dcpVanish + ich;
	mov	ax,[ich]
	cwd
	add	ax,[OFF_dcpVanish]
	adc	dx,[SEG_dcpVanish]
        add	ax,[vfli.LO_cpMinFli]
	adc	dx,[vfli.HI_cpMinFli]
	mov	[vfmtss.LO_cpRefFmtss],ax
	mov	[vfmtss.HI_cpRefFmtss],dx

	push	bx	;save xpNew
;				 int nSaveDC = 0;
	xor	ax,ax

;				 if (ww != wwNil)
;				     {
	cmp	[ww],wwNil
	je	DFC34

;				     nSaveDC = SaveDCN(hdc);
ifdef DEBUG
	cCall	SaveDCFP,<[hdc]>
else ;!DEBUG
ifdef HYBRID
	cCall	SaveDCPR,<[hdc]>
else ;!HYBRID
	cCall	SaveDC,<[hdc]>
endif ;!HYBRID
endif ;DEBUG
;					if (vfPrvwDisp)
;						InflateRect((LPRECT)&rcwClip,5);
;					IntersectClipRect(hdc, rcwClip.xpLeft,
;					    rcwClip.ypTop, rcwClip.xpRight,
;					    rcwClip.ypBottom);
;					if (vfPrvwDisp)
;						InflateRect((LPRECT)&rcwClip,-5);
	lea	si,[rcwClipXpLeftRc]
	mov	cx,[vfPrvwDisp]
	jcxz	DFC333
;	DFC81 performs
;		InflateRect(si,bx)
;	assumes prc in si, offset in bx, no registers are altered.
	mov	bx,10
	call	DFC81
DFC333:
;	DFC69 performs
;	IntersectClipRect(hdc, si.xpLeft, si.ypTop, si.xpRight, si.ypBottom);
;	bx, cx, dx and si are altered.
	push	si	;save prc
	push	cx	;save vfPrvwDisp
	call	DFC69
	pop	cx	;restore vfPrvwDisp
	pop	si	;restore prc
	jcxz	DFC337
;	DFC81 performs
;		InflateRect(si,bx)
;	assumes prc in si, offset in bx, no registers are altered.
	mov	bx,-10
	call	DFC81
DFC337:

;				     }
DFC34:
	xchg	ax,si	;save nSaveDC

;				 bchrCur = (char *) pchr - (char *)*vhgrpchr;
;				 bchpCur = (char *) pchp - (char *)*vhgrpchr;
;				 ShowSpec(ww, ich, &ptPen, yp, pchp, &uls);
;				 (char *) pchr = (char *)*vhgrpchr + bchrCur;
;				 (char *) pchp = (char *)*vhgrpchr + bchpCur;
	push	[ww]
	push	[ich]
	lea	ax,[ptPen]
	push	ax
	push	[ypVar]
	push	di
	lea	ax,[uls]
	push	ax
	;DFC82 converts di and [pchr] from pointers to offsets into
	;*vhgrpchr.  Upon return bx is -pchr.  Only bx is altered.
	call	DFC82
	cCall	ShowSpec,<>
	;DFC83 converts di and [pchr] to pointers from offsets into
	;*vhgrpchr.  Upon return bx is pchr.  Only bx is altered.
	call	DFC83

	pop	cx	;restore xpNew
	push	cx	;save xpNew

;				 if (vfli.grpfvisi.fvisiShowAll &&
;				     (chSpec == chFootnote || chSpec == chTFtn || chSpec == chTFtnCont))
;				     {
	errnz	<maskfvisiShowAllGrpfvisi - 00020h>
	test	bptr ([vfli.grpfvisiFli]),maskfvisiShowAllGrpfvisi
	je	DFC35
	mov	bx,[ich]
	mov	al,bptr ([bx.vfli.rgchFli])
	cmp	al,chFootNote
	je	DFC345
	cmp	al,chTFtn
	je	DFC345
	cmp	al,chTFtnCont
	jne	DFC35
DFC345:

;				     struct RC rcBox;
;				     rcBox.xpLeft = ptPen.xp;
;				     rcBox.xpRight = xpNew - 1;
;				     rcBox.ypTop = yp + 1;
;				     rcBox.ypBottom = ptPenT.yp;
;				     /*yp = rcBox.ypTop - vfti.dypAscent */
;				     DrawDottedLineBox(hdc, rcBox);
	push	[hdc]
	errnz	<(xpLeftRc)-0>
	errnz	<(ypTopRc)-2>
	errnz	<(xpRightRc)-4>
	errnz	<(ypBottomRc)-6>
	push	[ptPenT.ypPt]
	dec	cx
	push	cx
	mov	bx,[ypVar]
	inc	bx
	push	bx
	push	[ptPen.xpPt]
	cCall	DrawDottedLineBox,<>

;				     }
DFC35:

;				 if (nSaveDC != 0)
;				     {
	or	si,si
	je	DFC36

;				     RestoreDC(hdc, nSaveDC);
;				     nSaveDC = 0;
	push	[hdc]
	push	si
ifdef DEBUG
	cCall	RestoreDCFP,<>
else ;!DEBUG
ifdef HYBRID
	cCall	RestoreDCPR,<>
else ;!HYBRID
	cCall	RestoreDC,<>
endif ;!HYBRID
endif ;DEBUG

;				     }
DFC36:

;				 ich++;
	inc	[ich]

;				 ptPen.xp = xpNew;
	pop	ax	;restore xpNew
	mov	[ptPen.xpPt],ax

	pop	ax	;restore flags
;				 continue;
Ltemp009:
	jmp	DFC48

;				 }
DFC37:

;	NOTE: we already know at this point that cch > 0.
;			 if (cch > 0)
;				 {

;				 int xp = ptPen.xp;
;                                int cchT = cch + cchChop;

;				 while (cchT--)
;					 ptPen.xp += *pdxp++;
	mov	[xpVar],dx
	push	ax	;save flags
	push	cx	;save cch
	add	cx,[cchChop]
DFC38:
	lodsw
	add	dx,ax
	loop	DFC38
	mov	[ptPen.xpPt],dx
	pop	cx	;restore cch
	pop	ax	;restore flags

;#ifdef DFONT
;				 if (vfDbgRTFlag)
;				     {
;				     HFONT hfontT;
;				     SelectObject (hdc, (hfontT =
;					 SelectObject (hdc, GetStockObject
;					 (SYSTEM_FONT))));
;				     CommSzNum ("Font = ", hfontT);
;				     }
;#endif /* DFONT */
ifdef DFONT
;	Do this DFONT stuff with a call so
;	as not to mess up short jumps */
	call	DFC76
endif ;/* DFONT */

;/* following test prevents inefficient extra call to put out trailing
;   space at end of line */
;                                if (ich < vfli.ichMac-1 || vfli.rgch [ich] != chSpace)
;				    {
	mov	dl,[vfli.ichMacFli]
	dec	dx
	mov	bx,[ich]
	cmp	bl,dl
	jb	DFC39
	cmp	[bx.vfli.rgchFli],chSpace
	je	Ltemp009
DFC39:

;				     int far *lpdxp;
;/* use array-of-widths if:
;	 (1) user-defined character spacing is in effect (pchp->qpsSpace)
;     OR (2) justification is on
;     OR (3) visi spaces and space width != visi space char width
;     OR (4) displaying for preview */

;				     if (pchp->qpsSpace
;					 || vfli.ichSpace2 != ichNil
;					 || vfli.fAdjustForVisi
;                         || vfPrvwDisp)
;					     lpdxp = (int far *)&vfli.rgdxp [ich];
;				     else
;					     lpdxp = (int far *)(DWORD) NULL;
	shl	bx,1
	lea	si,[bx.vfli.rgdxpFli]
	mov	dx,ds
	test	bptr ([di.qpsSpaceChp]),maskQpsSpaceChp
	jne	DFC40
	cmp	[vfli.ichSpace2Fli],ichNil
	jne	DFC40
	cmp	[vfli.fAdjustForVisiFli],fFalse
	jne	DFC40
	cmp	[vfPrvwDisp],fFalse
	jne	DFC40
	errnz	<NULL>
	xor	si,si
	mov	dx,si
DFC40:
	push	di	;save pchp

;				     if (!fErased && !fPrvwPrint)
;					 {
	test	al,maskfErasedLocal+maskfPrvwPrintLocal
	jne	DFC42

;/* blank to the end of rcwClip if this is the last run of visible chars, for
;   efficiency, to avoid an extra PatBlt */
; 					rcErase.xpLeft = 
; 					rcOpaque.xpRight = 
; 					 (ptPen.xp >= vfli.xpRight ||
; 					 ptPen.xp >= rcwClip.xpRight) ? 
;						rcwClip.xpRight : ptPen.xp;

	mov	di,[ptPen.xpPt]
	mov	bx,[rcwClipXpRightRc]
	cmp	di,bx
	jge	DFC41
	cmp	di,[vfli.xpRightFli]
	jge	DFC41
	mov	bx,di
DFC41:
	mov	[rcOpaque.xpRightRc],bx
	mov	[rcErase.xpLeftRc],bx

;					 }
DFC42:
;                                   /* if there are any borders, don't opaque them,
;                                       to avoid flashing; instead we need to wipe
;                                       out any space outside the borders that's
;                                       in the EDL */
;                                   fRcAdjusted = fFalse;
;				    if (!fPrvwPrint &&
;                                       (vfli.fTop || vfli.fLeft || vfli.fBottom || vfli.fRight))
;					{
;                                       fRcAdjusted = fTrue;
;                                       AdjustOpaqueRect(&rcOpaque, &rcAdjusted, hdc, dxpToXw);
;					}

	and	al,NOT maskfRcAdjustedLocal
	test	al,maskfPrvwPrintLocal
	jne	DFC43
	errnz	<(fLeftFli) - (fTopFli)>
	errnz	<(fBottomFli) - (fTopFli)>
	errnz	<(fRIghtFli) - (fTopFli)>
	test	[vfli.fTopFli],maskFTopFli+maskFLeftFli+maskFBottomFli+maskFRightFli
	je	DFC43
	or	al,maskfRcAdjustedLocal
	push	ax	;save flags
	push	cx	;save cch
	push	dx	;save hi lpdxp
	lea	ax,[rcOpaque]
	push	ax
	lea	ax,[rcAdjusted]
	push	ax
	push	[hdc]
	push	[dxpToXw]
	push	[ywLine]
	cCall	AdjustOpaqueRect,<>
	pop	dx	;restore hi lpdxp
	pop	cx	;restore cch
	pop	ax	;restore flags
DFC43:
;	/* FUTURE - The only reason for protecting pchr and pchp from heap
;	   movement here is that the "Cannot print" dialog may be brought
;	   up under ExtTextOut.  If that dialog becomes system modal
;	   and does not move the heap, we can remove this junk.
;	   Brad Verheiden (4-13-89).
;	*/
;				     bchrCur = (char *) pchr - (char *)*vhgrpchr;
;				     bchpCur = (char *) pchp - (char *)*vhgrpchr;
;                                       /* protection around the ExtTextOut calls */
;                                    vrf.fInExternalCall = fTrue;
	or	[vrf.fInExternalCallRf],maskFInExternalCallRf
        pop	di	;restore pchp
	;DFC82 converts di and [pchr] from pointers to offsets into
	;*vhgrpchr.  Upon return bx is -pchr.  Only bx is altered.
	call	DFC82
	push	ax	;save flags

	; native note: push some arguments for ExtTextOut or ExtTextOutKludge
	; or PrintUlLine here, while things are handy in the registers.
;PAUSE
        test	al,maskfRcAdjustedLocal
	stc
DFC46:
	push	[hdc]
	push	[xpVar]
	push	[ypVar]
	push	[eto]
	lea	bx,[rcOpaque]
        je	DFC465
	lea	bx,[rcAdjusted]
DFC465:
	push	ds
	push	bx	; lprc
	jnc	DFC467
	mov	bx,[ich]
	add	bx,dataoffset [vfli.rgchFli]
	push	ds
	push	bx	; lprgch
	push	cx	; cch
	push	dx
	push	si	; lpdxp

; /* Now we have underlining fun for printing */
; if (vfli.fPrint && lpdxp && 
;		 (vfti.fcid.kul || vfti.fcid.fStrike))
;   {
;PAUSE
	cmp	[vfli.fPrintFli],0
	je	DFC47
	mov	bx,si
	or	bx,dx
	je	DFC47
;PAUSE
	errnz	<kulFcid-fStrikeFcid>
	mov	bl,[vfti.fcidFti.kulFcid]
	and	bl,maskKulFcid+maskFStrikeFcid
	je	DFC47
;PAUSE
;   PrintUlLine(hdc, xp, yp,
;     eto, 			/* action bits */
;     fRcAdjusted ? (LPRECT) &rcAdjusted :
;     (LPRECT) &rcOpaque,  	/* opaque rect */
;     lpdxp,
;     cch);
        test	al,maskfRcAdjustedLocal
	jmp	short DFC46 ; pushes hdc through lprc
DFC467:
	push	dx
	push	si	; lpdxp
	push	cx	; cch
	cCall	PrintUlLine,<>
;   }
DFC47:
; if (vwWinVersion < 0x300 && !vfli.fPrint && lpdxp && pchp->fItalic)
;   {
;PAUSE
	cmp	[vwWinVersion],0300h
	jge	LRealExtTextOut
	cmp	[vfli.fPrintFli],0
	jnz	LRealExtTextOut
	mov	ax,si
	or	ax,dx
	jz	LRealExtTextOut
	mov	bx,[vhgrpchr]
	mov	bx,[bx]
	test	[bx.di.fItalicChp],maskFItalicChp
	jz	LRealExtTextOut
;   struct FFN *pffn;
;
;   pffn = PstFromSttb(vhsttbFont, vfti.fcid.ibstFont);
;   if (pffn->fRaster)
;     goto LRealExtTextOut;
;PAUSE
	push	[vhsttbFont]
	xor	ax,ax
	mov	al,[vfti.fcidFti.ibstFontFcid]
	push	ax
	cCall	PstFromSttb,<>
	xchg	ax,bx
	test	[bx.fRasterFfn],maskFRasterFfn
	jnz	LRealExtTextOut
;   ExtTextOutKludge(hdc, xp, yp,
;     eto, 			/* action bits */
;     fRcAdjusted ? (LPRECT)&rcAdjusted :
;     (LPRECT)&rcOpaque,  	/* opaque rect */
;     (LPCH)&vfli.rgch [ich],	/* string */
;     cch,			/* length */
;     lpdxp );		/* lpdx */
;PAUSE
	; native note: arguments pushed above
	cCall	ExtTextOutKludge,<>
	jmp short	DFC475

;   }
; else
;   {
LRealExtTextOut:
;   ExtTextOut(hdc, xp, yp,
;     eto, 			/* action bits */
;     fRcAdjusted ? (LPRECT)&rcAdjusted :
;     (LPRECT)&rcOpaque,  	/* opaque rect */
;     (LPCH)&vfli.rgch [ich],	/* string */
;     cch,			/* length */
;     lpdxp );		/* lpdx */
;   }
; native note: arguments pushed above
ifdef DEBUG
	cCall	ExtTextOutFP,<>
else ;!DEBUG
ifdef HYBRID
	cCall	ExtTextOutPR,<>
else ;!HYBRID
	cCall	ExtTextOut,<>
endif ;!HYBRID
endif ;DEBUG
DFC475:
	pop	ax	;restore flags

; vrf.fInExternalCall = fFalse;
; (char *) pchr = (char *)*vhgrpchr + bchrCur;
; (char *) pchp = (char *)*vhgrpchr + bchpCur;
	and	[vrf.fInExternalCallRf], NOT maskFInExternalCallRf

	;DFC83 converts di and [pchr] to pointers from offsets into
	;*vhgrpchr.  Upon return bx is pchr.  Only bx is altered.
	call	DFC83

;				     if (!fErased && !fPrvwPrint)
	test	al,maskfErasedLocal+maskfPrvwPrintLocal
	jne	DFC48

;					 rcOpaque.xpLeft = ptPen.xp;
	mov	bx,[ptPen.xpPt]
	mov	[rcOpaque.xpLeftRc],bx

;				     }
;				 }
;
;			 ich = ichNext;
;			 }
DFC48:
	pop	cx	    ;restore ichNext
	mov	si,[pchr]   ;restore pchr
	mov	bptr ([ich]),cl

;		 }
	jmp	DFC265

;LEnd:
;	if (uls.grpfUL)
;		EndULPast( &uls, ptPen.xp, ich ,&rcwClip);
LEnd:
	push	ax	;save flags
;	LN_EndULPast performs:
;	if (uls.grpfUL) EndULPast(&uls,ptPen.xp,ich,&rcwClip);
;	ax, bx, cx and dx are altered
	call	LN_EndULPast

;/* erase portion of the line not erased within ExtTextOut blanking box */
;	if (!fPrvwPrint)
;	    SetBkMode( hdc, OPAQUE );	 /* restore output mode */

	pop	ax
	push	ax
	test	al,maskfPrvwPrintLocal
	jne	DFC50
	mov	ax,OPAQUE
	cCall	SetBkMode,<[hdc],ax>
DFC50:
	pop	ax	;restore flags

;	 if (!fPrvwPrint && !fErased && rcErase.xpLeft < rcErase.xpRight)
;	     PatBltRc( hdc, &rcErase, vsci.ropErase );
	test	al,maskfPrvwPrintLocal+maskfErasedLocal
	jne	DFC51
	mov	bx,[rcErase.xpLeftRc]
	cmp	bx,[rcErase.xpRIghtRc]
	jge	DFC51
;	DFC56 performs:
;	PatBltRc( hdc, fUseRcErase ? &rcErase : &rcT,
;		  vsci.ropErase );
;	no registers are altered
	or	al,maskfUseRcEraseLocal
	call	DFC56
DFC51:

;	 ForeColor(hdc, icoAuto);
	errnz	<icoAuto - 0>
	xor	ax,ax
	cCall	ForeColor,<[hdc],ax>

;	/* return if no para border OR 
;	printing and not the pass for drawing borders... 
;	*/
;	if (vfli.grpfBrc == 0)
;		return;
DFC52:
	cmp	[vfli.grpfBrcFli],0
	je	DFC53

;	if (vfli.fPrint)
;		{
	cmp	[vfli.fPrintFli],fFalse
	je	DFC525

;		if (vpri.fDPR)	/* can only use DRAWPATTERNRECT in fText pass */
;			{
;			if (!vpri.fText)
;				return;
;			}
;		else
;			{
;			if (!vpri.fGraphics)
;				return;
;			}
;		}
	mov	al,maskfTextPri
	cmp	[vpri.fDPRPri],fFalse
	jne	DFC523
	mov	al,maskfGraphicsPri
DFC523:
	errnz	<(fTextPri) - (fGraphicsPri)>
	test	[vpri.fTextPri],al
	je	DFC53

DFC525:

;/* handle drawing of borders */
;	 if (
;		 /* Borders */
;		 (vfli.fTop || vfli.fLeft || vfli.fBottom || vfli.fRight)
;			 &&
;		 (ilevel = SaveDC( hdc )))
;	     {
	errnz	<(fLeftFli) - (fTopFli)>
	errnz	<(fBottomFli) - (fTopFli)>
	errnz	<(fRIghtFli) - (fTopFli)>
	test	[vfli.fTopFli],maskFTopFli+maskFLeftFli+maskFBottomFli+maskFRightFli
	je	DFC53
ifdef DEBUG
	cCall	SaveDCFP,<[hdc]>
else ;!DEBUG
ifdef HYBRID
	cCall	SaveDCPR,<[hdc]>
else ;!HYBRID
	cCall	SaveDC,<[hdc]>
endif ;!HYBRID
endif ;DEBUG
	or	ax,ax
	je	DFC53

	push	[hdc]	;hdc argument for RestoreDC
	push	ax	;ilevel argument for RestoreDC

;		if (vfPrvwDisp)
;			InflateRect((LPRECT) rcwClip, 4, 4);
; Assemble Note: coded inline because we're gods

	lea	si, [rcwClipXpLeftRc]
	cmp	[vfPrvwDisp],fFalse
	je	DFC527
	mov	bx,10
;	DFC81 performs
;		InflateRect(si,bx)
;	assumes prc in si, offset in bx, no registers are altered.
	call	DFC81

DFC527:

;	     IntersectClipRect( hdc, rcwClip.xpLeft, rcwClip.ypTop,
;				rcwClip.xpRight, rcwClip.ypBottom );
;	DFC69 performs
;	IntersectClipRect(hdc, si.xpLeft, si.ypTop, si.xpRight, si.ypBottom);
;	bx, cx, dx and si are altered.
	call	DFC69

;	     DrawBorders(dxpToXw, ywLine);
	cCall	DrawBorders,<[dxpToXw],[ywLine]>

;	     RestoreDC( hdc, ilevel );
ifdef DEBUG
	cCall	RestoreDCFP,<>
else ;!DEBUG
ifdef HYBRID
	cCall	RestoreDCPR,<>
else ;!HYBRID
	cCall	RestoreDC,<>
endif ;!HYBRID
endif ;DEBUG

;	     }
DFC53:
;	 return;
;}
cEnd


;	DFC54 performs:
;	blt(fUseRcErase ? &rcErase : &rcOpaque, &rcT, cwRC );
;	rcT.xpRight = bx;
;	no registers are altered
DFC54:
	push	ds
	pop	es
	push	si
	push	di
	lea	si,[rcErase]
	test	al,maskfUseRcEraseLocal
	jne	DFC55
	lea	si,[rcOpaque]
DFC55:
	lea	di,[rcT]
	movsw
	movsw
	movsw
	movsw
	mov	[di.xpRightRc-cbRcMin],bx
	pop	di
	pop	si
	ret


;	DFC56 performs:
;	PatBltRc( hdc, fUseRcErase ? &rcErase : &rcT,
;		  vsci.ropErase );
;	no registers are altered
DFC56:
	push	ax
	push	bx
	push	cx
	push	dx
	push	[hdc]
	lea	bx,[rcT]
	test	al,maskfUseRcEraseLocal
	je	DFC565
	lea	bx,[rcErase]
DFC565:
	push	bx
	mov	bx,[vpdcibDisplayFli]
	push	[vsci.HI_ropEraseSci]
	push	[vsci.LO_ropEraseSci]
	cCall	PatBltRc,<>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret


;	DFC57 performs:
;	if (uls.grpfUL)
;		{
;		bchrCur = (char*)pchr - (char*)*vhgrpchr;
;		bchpCur = (char*)pchp - (char*)*vhgrpchr;
;		uls.xpLim = ptPen.xp;
;		EndUL(&uls, &rcwClip);
;		pchr = (char*)*vhgrpchr + bchrCur;
;		pchp = (char*)*vhgrpchr + bchpCur;
;		}
;	bx, cx and dx are altered
DFC57:
	cmp	[uls.grpfULUls],fFalse
	je	DFC58
;PAUSE
	push	ax

	mov	bx,[vhgrpchr]
	sub	si,[bx]
	sub	di,[bx]

	mov	ax,[ptPen.xpPt]
	mov	[uls.xpLimUls],ax
	lea	ax,[uls]
	lea	bx,[rcwClipXpLeftRc]
	cCall	EndUL,<ax,bx>

	mov	bx,[vhgrpchr]
	add	si,[bx]
	add	di,[bx]

	pop	ax
DFC58:
	ret


;				 Assert (vfli.rgch [ich] == chDisplayField);
;				 Assert (vfli.rgdxp [ich] ==
;					 ((struct CHRDF *)pchr)->dxp);
;				 Assert (ich == pchr->ich);
ifdef DEBUG
DFC59:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	bx,[ich]
	cmp	[bx.vfli.rgchFli],chDisplayField
	je	DFC60
	mov	ax,midDisp1n
	mov	bx,1004
	cCall	AssertProcForNative,<ax,bx>
DFC60:
	mov	bx,[ich]
	shl	bx,1
	mov	ax,[bx.vfli.rgdxpFli]
	cmp	ax,[si.dxpChrdf-cbCHRDF]
	je	DFC61
	mov	ax,midDisp1n
	mov	bx,1005
	cCall	AssertProcForNative,<ax,bx>
DFC61:
	mov	al,bptr ([ich])
	cmp	al,[si.ichChrdf-cbCHRDF]
	je	DFC62
	mov	ax,midDisp1n
	mov	bx,1006
	cCall	AssertProcForNative,<ax,bx>
DFC62:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */


;#ifdef SHOWFLD
;				 CommSzLong ("Displaying CHRDF: ",
;				     vfli.cpMin + dcpVanish + pchr->ich);
;#endif /* SHOWFLD */
ifdef SHOWFLD
DFC63:
	push	ax
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	xor	ax,ax
	mov	al,[si.ichChr]
	cwd
        add	ax,[vfli.LO_cpMinFli]
	adc     dx,[vfli.HI_cpMinFli]
        add	ax,[OFF_dcpVanish]
        adc	dx,[SEG_dcpVanish]
	jmp	short DFC65
DFC64:
	db	'Displaying CHRDF: '
	db	0
DFC65:
	push	ds
	push	cs
	pop	ds
	mov	si,DFC64
	push	ss
	pop	es
	mov	di,[szFrame]
	mov	cx,(DFC65) - (DFC64)
	rep	movsb
	pop	ds
	lea	bx,[szFrame]
	cCall	CommSzLong,<bx, dx, ax>
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* SHOWFLD */


;	Assert( vsci.hpen && iLevel);
ifdef DEBUG
DFC66:
	cmp	[vsci.hpenSci],fFalse
	je	DFC67
	cmp	ax,fFalse
	jne	DFC68
DFC67:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midDisp1n
	mov	bx,1007
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
DFC68:
	ret
endif ;/* DEBUG */


;	DFC69 performs
;	IntersectClipRect(hdc, si.xpLeft, si.ypTop, si.xpRight, si.ypBottom);
;	bx, cx, dx and si are altered.
DFC69:
	push	ax
	push	[hdc]
	errnz	<(xpLeftRc)-0>
	errnz	<(ypTopRc)-2>
	errnz	<(xpRightRc)-4>
	errnz	<(ypBottomRc)-6>
	mov	cx,4
DFC70:
	lodsw
	push	ax	;rc argument for IntersectClipRect
	loop	DFC70
ifdef DEBUG
	cCall	IntersectClipRectFP,<>
else
	cCall	IntersectClipRect,<>
endif ;/* DEBUG */
	pop	ax
	ret


;	DFC71 performs:
;	SetBkMode(hdc, TRANSPARENT);
;	eto &= ~ETO_OPAQUE;
;	bx, cx, and dx are altered
DFC71:
	push	ax	;save flags
	mov	ax,TRANSPARENT
	cCall	SetBkMode,<[hdc],ax>
	errnz	<ETO_OPAQUE - 2>
	and	bptr ([eto]),NOT ETO_OPAQUE
	pop	ax	;restore flags
	ret


;			 Assert(fPchpInitialized);
ifdef DEBUG
DFC72:
	cmp	[fPchpInitialized],fFalse
	jne	DFC73
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midDisp1n
	mov	bx,1008
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
DFC73:
	ret
endif ;/* DEBUG */


;				 Assert(vfli.rgch [ich] != chDisplayField);
ifdef DEBUG
DFC74:
	push	ax
	push	bx
	push	cx
	push	dx
	mov	bx,[ich]
	cmp	[bx.vfli.rgchFli],chDisplayField
	jne	DFC75
	mov	ax,midDisp1n
	mov	bx,1009
	cCall	AssertProcForNative,<ax,bx>
DFC75:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */


;#ifdef DFONT
;				 if (vfDbgRTFlag)
;				     {
;				     HFONT hfontT;
;				     SelectObject (hdc, (hfontT =
;					 SelectObject (hdc, GetStockObject
;					 (SYSTEM_FONT))));
;				     CommSzNum ("Font = ", hfontT);
;				     }
;#endif /* DFONT */
ifdef DFONT
DFC76:
	push	ax
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	mov	ax,SYSTEM_FONT
	cCall	GetStockObject,<ax>
ifdef DEBUG
	cCall	SelectObjectFP,<hdc,ax>
else ;!DEBUG
ifdef HYBRID
	cCall	SelectObjectPR,<hdc,ax>
else ;!HYBRID
	cCall	SelectObject,<hdc,ax>
endif ;!HYBRID
endif ;DEBUG
	push	ax	;save hfontT
ifdef DEBUG
	cCall	SelectObjectFP,<hdc,ax>
else ;!DEBUG
ifdef HYBRID
	cCall	SelectObjectPR,<hdc,ax>
else ;!HYBRID
	cCall	SelectObject,<hdc,ax>
endif ;!HYBRID
endif ;DEBUG
	pop	ax	;restore hfontT
	jmp	short DFC78
DFC77:
	db	'Font = '
	db	0
DFC78:
	push	ds
	push	cs
	pop	ds
	mov	si,DFC77
	push	ss
	pop	es
	mov	di,[szFrame]
	mov	cx,(DFC78) - (DFC77)
	rep	movsb
	pop	ds
	lea	bx,[szFrame]
	cCall	CommSzNum,<bx, ax>
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DFONT */


;	Assert we have one of chrmEnd, chrmTab, chrmVanish, chrmVanish
;	chrmFormula, chrmDisplayField, chrmFormatGroup, chrmChp.
ifdef DEBUG
DFC79:
	cmp	cl,chrmEnd
	je	DFC80
	cmp	cl,chrmTab
	je	DFC80
	cmp	cl,chrmVanish
	je	DFC80
	cmp	cl,chrmFormula
	je	DFC80
	cmp	cl,chrmDisplayField
	je	DFC80
	cmp	cl,chrmFormatGroup
	je	DFC80
	cmp	cl,chrmChp
	je	DFC80
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midDisp1n
	mov	bx,1017
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
DFC80:
	ret
endif ;/* DEBUG */


;	DFC81 performs
;		InflateRect(si,bx)
;	assumes prc in si, offset in bx, no registers are altered.
DFC81:
	add	[si.xpRightRc],bx
	add	[si.ypBottomRc],bx
	sub	[si.xpLeftRc],bx
	sub	[si.ypTopRc],bx
	ret

	;DFC82 converts di and [pchr] from pointers to offsets into
	;*vhgrpchr.  Upon return bx is -pchr.  Only bx is altered.
DFC82:
	db	0A8h	;turns next "stc" into "test al,immediate"
			;also clears the carry flag
	;DFC83 converts di and [pchr] to pointers from offsets into
	;*vhgrpchr.  Upon return bx is pchr.  Only bx is altered.
DFC83:
	stc
	mov	bx,[vhgrpchr]
	mov	bx,[bx]
	jc	DFC84
	neg	bx
DFC84:
	add	di,bx
	add	[pchr],bx
	ret


ifdef DEBUG
;	Assert( vfli.fPrint & 0xFE == 0 );
DFC85:
	test	al,0FEh
	je	DFC86
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midDisp1n
	mov	bx,1030
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
DFC86:
	ret
endif ;/* DEBUG */


;	LN_EndULPast performs:
;	if (uls.grpfUL) EndULPast(&uls,ptPen.xp,ich,&rcwClip);
;	ax, bx, cx and dx are altered
LN_EndULPast:
	cmp	[uls.grpfULUls],fFalse
	je	EULP04

;/* E n d  U L	P a s t */
;/* Terminate underline; limit underline to not past vfli.ichSpace3
;   given current ich and corresponding xp */
;
;NATIVE EndULPast(puls, xpLim, ichLim, prcwClip)
;struct ULS *puls;
;int xpLim, ichLim;
;struct RC *prcwClip;
;{
; while (--ichLim >= (int)vfli.ichSpace3)
;	 xpLim -= vfli.rgdxp [ichLim];
	push	si
	mov	cx,[ich]
	mov	si,cx
	mov	al,[vfli.ichSpace3Fli]
	xor	ah,ah
	mov	dx,[ptPen.xpPt]
	sub	cx,ax
	jle	EULP02
	shl	si,1
	add	si,dataoffset [vfli.rgdxpFli - 2]
	std
EULP01:
	lodsw
	sub	dx,ax
	loop	EULP01
	cld
EULP02:

; if ((puls->xpLim = xpLim) > puls->pt.xp)
;	 EndUL(puls, prcwClip);
;}
	mov	[uls.xpLimUls],dx
	cmp	dx,[uls.ptUls.xpPt]
	jle	EULP03
	lea	ax,[uls]
	lea	bx,[rcwClipXpLeftRc]
	cCall	EndUL,<ax,bx>
EULP03:
	pop	si
EULP04:
	ret


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


sEnd	disp1_PCODE
        end
