;               BOTH -- define different names for functions so that
;                       FORMAT.C and FORMATN.ASM can coexist.
;                       Native names are formed by appending "N"

        include w2.inc
        include noxport.inc
        include consts.inc
        include structs.inc

; NOTEXT          equ     1       ;  - don't include TextMetric struc & text drawing modes & stock objs.
NORASTOPS       equ     1       ;  - don't include binary and ternary raster ops.
NOVK            equ     1       ;  - don't include virtual key definitions
NOMB            equ     1       ;  - don't include message box definitions
NOWM            equ     1       ;  - don't include window messages
        include windows.inc

        .list

createSeg       _FORMAT,format,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midFormatn2     equ 5           ; module ID, for native asserts
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

cbszMax equ     30


; EXTERNAL FUNCTIONS

externFP        <GetTextFace>
externFP	<FreeHandlesOfPfce>
externFP        <FReviveTossedPrinterDC>
externFP	<GetCharWidth>
externFP	<GetTextMetrics>
ifdef HYBRID
externFP	<GetStockObjectPR>
externFP	<CreateFontIndirectPR>
externFP	<GetTextExtentPR>
externFP	<GlobalWirePR>
externFP	<GlobalUnlockPR>
else ;!HYBRID
externFP	<GetStockObject>
externFP	<CreateFontIndirect>
externFP	<GetTextExtent>
externFP	<GlobalWire>
externFP	<GlobalUnlock>
endif ;!HYBRID
ifdef DEBUG
externFP        <SelectObjectFP>
else ;!DEBUG
ifdef HYBRID
externFP	<SelectObjectPR>
else ;!HYBRID
externFP        <SelectObject>
endif ;!HYBRID
endif ;!DEBUG
externFP        <NMultDiv,FNeNcSz,CchSz,PpvAllocCb>
externFP	<N_IbstFindSzFfn,IbstAddStToSttb>
externFP	<PstFromSttb,N_PdodDoc,N_PdodMother>
externFP	<GetPhysicalFontHandle>
externFP	<GlobalFlags>
externFP	<FNeRgch>
externNP	<LN_NMultDiv>
externFP	<GenPrvwPlf>
externFP	<HqAllocLcb, LpLockHp, UnlockHp, ReloadSb>
externFP	<OurSetSas>
externFP        <SetErrorMatProc>

ifdef DEBUG
externFP        <AssertProcForNative,ShakeHeapSb>
externFP	<CkFont,FreezeProc>
externFP	<S_ResetFont>
externFP        <ScribbleProc>
externFP        <CommSzNum,CommSzSz>
externFP	<LogGdiHandleProc>
externFP	<S_IbstFindSzFfn>
endif ;DEBUG


; EXTERNAL DATA

sBegin  data
externW vsab            ; extern struct SAB       vsab;
externW vfli            ; extern struct FLI   vfli;
externW vhsttbFont      ; extern struct STTB  **vhsttbFont;
externW vfti            ; extern struct FTI   vfti;
externW vftiDxt         ; extern struct FTI   vftiDxt;
externW vflm            ; extern int vflm;
externW vpri            ; extern struct PRI   vpri;
externW vmerr           ; extern struct MERR  vmerr;
externW vlm             ; extern int          vlm;
externW vfPrvwDisp      ; extern int          vfPrvwDisp;
externW vsci            ; extern struct SCI   vsci;
externW wwMac           ; extern int          wwMac;
externW mpwwhwwd        ; extern struct WWD   **mpwwhwwd[];
externW vpref           ; extern struct PREF  vpref;
externW rgfce           ; extern struct FCE   rgfce[ifceMax];
externW mpsbps		; extern SB	      mpsbps[];
externW vsasCur 	; extern int	      vsasCur;

ifdef DEBUG
externW cHpFreeze
externW vdbs
externW vhgdis		; extern int	      vhgdis;
endif

               
sEnd    data


; CODE SEGMENT _FORMAT

sBegin  format
        assumes cs,format
        assumes ds,dgroup
        assumes ss,dgroup



; /* D X U  E X P A N D */
; /* returns the character expansion number in dxuInch units */
; native int DxuExpand(pchp, dxuInch)
; struct CHP *pchp;
; int dxuInch;
	;LN_DxuExpand takes dxuInch in dx, pchp in di
	;ax, bx, cx, dx are altered.
; %%Function:LN_DxuExpand %%Owner:BRADV
PUBLIC	LN_DxuExpand
LN_DxuExpand:
; {
;	int qps;
;
;       if ((qps = pchp->qpsSpace) == 0 || pchp->fSpec) return 0;
	mov	al,[di.qpsSpaceChp]
	errnz	<maskQpsSpaceChp - 03Fh>
	and	al,maskQpsSpaceChp
	jz	DE04
	test	[di.fSpecChp],maskFSpecChp
	jnz	DE04

; /* qps is encoded to be in the range [-7, 56] */
;       if (qps > 56) qps -= 64;
	cmp	al,56
	jle	DE01
	sub	al,64
DE01:
	cbw

; /* 4 (quarter) * 72 (points per inch) */
;       return NMultDiv(qps, dxuInch, 72 * 4);
;	LN_NMultDiv performs NMultDiv(ax, dx, bx)
;	ax, bx, cx, dx are altered.
	mov	bx,288
	call	LN_NMultDiv
	db	03Dh	;turns next "xor ax,ax" into "cmp ax,immediate"
DE04:
	xor	ax,ax
;}
	ret


;-------------------------------------------------------------------------
;	LoadFont( pchp,fWidthsOnly )
;-------------------------------------------------------------------------
; %%Function:N_Loadfont %%Owner:BRADV
cProc	N_LoadFont,<PUBLIC,FAR>,<si,di>
        ParmW   pchp
	ParmW	fWidthsOnly
; /*  LoadFont( pchp, fWidthsOnly )
;  *
;  *  Description: LoadFont loads the font specified by pchp into appropriate
;  *		 DC's
;  */

; 
; native LoadFont( pchp, fWidthsOnly )
; struct CHP *pchp;
; int fWidthsOnly;
;     {
;     union FCID fcid;
        localV  fcid, cbFcid
;     struct DOD *pdod;
; 
cBegin
;     Assert( vfli.doc != docNil );
ifdef DEBUG
        cmp     [vfli.docFli],docNil
	jne	LFnt01
	push	ax
	push	bx
	push	cx
	push	dx
        mov     ax,midFormatn2
        mov     bx,800
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LFnt01:

;     Assert( pchp != NULL);
	cmp	[pchp],NULL
	jne	LFnt02
	push	ax
	push	bx
	push	cx
	push	dx
        mov     ax,midFormatn2
        mov     bx,801
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LFnt02:

endif ;DEBUG
; 
;     /* Map font requests in short DOD back through parent DOD */
;     pdod = PdodMother(vfli.doc);
        cCall   N_PdodMother,<[vfli.docFli]>
        xchg	si,ax

;     if (vfli.doc == docUndo)
;		pdod = PdodDoc(PdodDoc(docUndo)->doc);
	mov	ax,[vfli.docFli]
	cmp	ax,docUndo
	jne	LFnt025
	cCall	N_PdodDoc,<ax>
	xchg	si,ax
	cCall	N_PdodDoc,<[si.docDod]>
	xchg	si,ax
	jmp	LFnt03


;    /* font table was not actually copied to scrap, so map request back to 
;       data provider doc, just like MapStc does for styles */
;     else if (pdod == PdodDoc(docScrap) && vsab.docStsh != docNil)
;     		pdod = PdodDoc(vsab.docStsh);
LFnt025:
	mov	ax,docScrap
	cCall	N_PdodDoc,<ax>
	cmp	ax,si
	jnz	LFnt03
	mov	cx,[vsab.docStshSab]
	jcxz	LFnt03
	cCall	N_PdodDoc,<cx>
	xchg	ax,si
LFnt03:
;                                               ; si = pdod
;         /* Translate chp --> fcid */
;     fcid.lFcid = 0L;
;     fcid.fItalic = pchp->fItalic;
;     fcid.fBold = pchp->fBold;
;     fcid.fStrike = pchp->fStrike;
;     fcid.kul = pchp->kul;    
;     fcid.hps = pchp->hps;
;/* protect against out-of-range ftcs in case of bogus doc or
;	failed ApplyGrpprlCa during font merge after interdoc paste */
;    fcid.ibstFont = (**pdod->hmpftcibstFont) [min( pchp->ftc, pdod->ftcMac-1)];
        mov     di,[pchp]                       ; di = pchp
	errnz	<(fBoldChp) - 0>
	errnz	<maskFBoldChp - 001h>
	errnz	<(fBoldFcid) - 0>
	errnz	<maskFBoldFcid - 001h>
	errnz	<(fItalicChp) - 0>
	errnz	<maskFItalicChp - 002h>
	errnz	<(fItalicFcid) - 0>
	errnz	<maskFItalicFcid - 002h>
	errnz	<(fStrikeChp) - 0>
	errnz	<maskFStrikeChp - 004h>
	errnz	<(fStrikeFcid) - 0>
	errnz	<maskFStrikeFcid - 004h>
	errnz	<maskKulChp - 070h>
	errnz	<(kulFcid) - 0>
	errnz	<maskKulFcid - 038h>
	errnz	<(hpsFcid) - 1>
	errnz	<(ibstFontFcid) - 2>
	mov	al,[di.fBoldChp]
	mov	ah,[di.kulChp]
	and	ax,maskFItalicFcid+maskFBoldFcid+maskFStrikeFcid+(maskKulChp SHL 8)
	shr	ah,1
	or	al,ah
	mov	ah,[di.hpsChp]
	mov	wptr ([fcid.fBoldFcid]),ax
	mov	ax,[si.ftcMacDod]
	dec	ax
        mov     si,[si.hmpftcibstFontDod]
        mov     si,[si]
	cmp	[di.ftcChp],ax
	ja	Lfnt04
	mov	ax,[di.ftcChp]
LFnt04:
        add     si,ax
        lodsb
	xor	ah,ah
	mov	wptr ([fcid.ibstFontFcid]),ax

;	vfti.dxpExpanded = vftiDxt.dxpExpanded = 0;

	xor	ax,ax
	mov	[vfti.dxpExpandedFti],ax
	mov	[vftiDxt.dxpExpandedFti],ax

;     if (vfli.fFormatAsPrint)
	lea	bx,[fcid]	;default assume vfli.fFormatAsPrint is fFalse
	errnz	<fFalse - 0>
	cmp	[vfli.fFormatAsPrintFli],al
	jz	LFnt08

;             {   /* Two-device case: Displaying; formatting for printer */

;             /* First determine what font we can get for the printer */
;             LoadFcid( fcid, &vftiDxt, fTrue );
        lea     ax,[vftiDxt]
        push    [fcid.HI_lFcid]
        push    [fcid.LO_lFcid]
        push    ax
	push	ax
	push	cs
	call	near ptr N_LoadFcid

;	    if (pchp->qpsSpace != 0)
;	    	vftiDxt.dxpExpanded = DxuExpand(pchp,vftiDxt.dxpInch);
	; di = pchp
	test	[di.qpsSpaceChp],maskQpsSpaceChp
	jz	LFnt06
	mov	dx,[vftiDxt.dxpInchFti]
	;LN_DxuExpand takes dxuInch in dx, pchp in di
	;ax, bx, cx, dx are altered.
	call	LN_DxuExpand
	mov	[vftiDxt.dxpExpandedFti],ax
LFnt06:

; 
;             /* Then request a screen font based on what we got back */
;             LoadFcid( vftiDxt.fcid, &vfti, fWidthsOnly );
	mov	bx,dataoffset [vftiDxt.fcidFti]

;             }
;     else
;             {   /* One-device case: 1. Printing
;                                     2. Displaying; formatting for screen */
;             LoadFcid( fcid, &vfti, fWidthsOnly );
LFnt08:
        lea     ax,[vfti]
	push	[bx.HI_lFcid]
	push	[bx.LO_lFcid]
        push    ax
	push	[fWidthsOnly]
	push	cs
	call	near ptr N_LoadFcid

;             }
	; di = pchp
	test	[di.qpsSpaceChp],maskQpsSpaceChp
	jz	LFnt09
	mov	dx,[vfti.dxpInchFti]
	;LN_DxuExpand takes dxuInch in dx, pchp in di
	;ax, bx, cx, dx are altered.
	call	LN_DxuExpand
	mov	[vfti.dxpExpandedFti],ax
LFnt09:
;     }
cEnd

;-------------------------------------------------------------------------
;	LoadFcidFull( fcid )
;-------------------------------------------------------------------------
;NATIVE LoadFcidFull( fcid )
;union FCID fcid;
;    {
;    LoadFcid( fcid, &vfti, fFalse /* fWidthsOnly */);
;    }
; %%Function:N_LoadFcidFull %%Owner:BRADV
cProc	N_LoadFcidFull,<PUBLIC,FAR>,<si,di>
	ParmD	fcid

cBegin
	mov	dx,[SEG_fcid]
	mov	ax,[OFF_fcid]
	mov	bx,dataoffset [vfti]
	errnz	<fFalse>
	xor	cx,cx
	push	dx
	push	ax
	push	bx
	push	cx
	push	cs
	call	near ptr N_LoadFcid
cEnd

;-------------------------------------------------------------------------
;	LoadFcid( fcid, pfti, fWidthsOnly )
;-------------------------------------------------------------------------
; %%Function:N_LoadFcid
cProc	N_LoadFcid,<PUBLIC,FAR>,<si,di>
        ParmD   fcid
        ParmW   pfti
	ParmW	fWidthsOnly
; /* LoadFcid( fcid, pfti, fWidthsOnly )
;  *
;  * Description:
;  *
;  * Loads a font described by fcid.
;  *
;  * Inputs:
;  *  fcid - a long describing the font
;  *  pfti - pointer to fti structure to fill out
;  *
;  * Outputs:
;  *
;  * *pfti is filled out with a description of the font actually realized for
;  * the request.  Note that it is frequently true that pfti->fcid != fcid passed
;  *
;  * Methods:
;  *
;  *  The last ifceMax fonts requested through LoadFcid are kept in a LRU cache,
;  *  such that the descriptive information, width tables, and mapping from
;  *  requested to actual font are kept around for quick access.
;  *
;  *  The fonts that are cached for the device described in *pfti are
;  *  kept on a chain extending from pfti->pfce.
;  *
;  *  If a font is currently selected into the device, then pfti->hfont
;  *  will be non-null and pfti->fcid will describe the font.
;  *
;  *  The fcid acts as the key field in looking up a font.  FCID is structured
;  *  so that the first word, wProps, contains those fields considered most
;  *  likely to vary.
;  *
;  */
;
; native LoadFcid( fcid, pfti, fWidthsOnly )
; union FCID fcid;
; struct FTI *pfti;
; int fWidthsOnly;
; {
;  extern int vflm;
;  int dyp;
;  struct FCE *pfce;
;  struct FCE *pfceHead;
;  int fFallback;
;  LOGFONT lf;
;  TEXTMETRIC tm;
;  int dxp;
;  HDC hdc;
;         char *pdxp;
;         int dxp;
;         int ch;     /* MUST be int so chDxpMax == 256 will work */
;         union FCID fcidActual;
;         CHAR rgb [cbFfnLast];
;         struct FFN *pffn = (struct FFN *) &rgb [0];
; 
        localW  dyp
        localV  lf, cbLogfontMin
        localV  tm, <SIZE TEXTMETRIC>
        localW  dxp
	localW	hdc
        localW  pdxp
        localW  ch1
        localV  rgb,cbFfnLast
	localW	hfontT
	localW	fFallback
ifdef DEBUG
        localV  szD, cbszMax
endif

cBegin

;  Assert( vflm != flmIdle );
ifdef DEBUG
	;Assert (vflm != flmIdle) with a call so as not
	;to mess up short jumps.
	call	LF40
endif ;DEBUG

; 
;  /* CASE 1: No fonts cached for this device */
; 
;  if ((pfce = pfti->pfce) == NULL)
        mov     di,[pfti]                       ; di = pfti
        mov     si,[di.pfceFti]                 ; si = pfce
	errnz	<NULL>
        or      si,si

;         {
;         goto LNewFce;
	je	Ltemp002

;         }

;  /* CASE 2: There is a font already selected in, and it matches the request */

	mov	ax,[fcid.wPropsFcid]
	mov	dx,[fcid.wExtraFcid]

;  if (pfti->hfont != NULL)
;         {
	cmp	[di.hFontFti],NULL
	je	LF06

;         if (fcid.wProps == pfce->fcidRequest.wProps &&
;             fcid.wExtra == pfce->fcidRequest.wExtra)
;                 {
        cmp     ax,[si.fcidRequestFce.wPropsFcid]
	jne	LF03
	cmp	dx,[si.fcidRequestFce.wExtraFcid]
	jne	LF03

;                 vfli.fGraphics |= pfce->fGraphics;
;                 return;
	test	[si.fGraphicsFce],maskfGraphicsFce
	je	LF02
	or	[vfli.fGraphicsFli],maskFGraphicsFli
LF02:
	jmp	LF37

;                 }
LF03:

; 
;         pfce = pfce->pfceNext;  /* start following search with 2nd font in chain */
        mov     si,[si.pfceNextFce]

;         }
LF04:
	jmp	short LF06

Ltemp002:
	jmp	short LNewFce

	;Assembler note: this line has been moved from the bottom of the
	;while loop
LF05:
;         pfce = pfce->pfceNext;
        mov     si,[si.pfceNextFce]
;  /* CASE 3: Font request matches some cached request for this device */

;  while (pfce != NULL)
;         {

LF06:
	errnz	<NULL>
        or      si,si
	je	LF10

;         if (pfce->fcidRequest.wProps == fcid.wProps &&
;             pfce->fcidRequest.wExtra == fcid.wExtra)
;                 {
        cmp     ax,[si.fcidRequestFce.wPropsFcid]
	jne	LF05
	cmp	dx,[si.fcidRequestFce.wExtraFcid]
	jne	LF05

;                 Assert( pfce->hfont != NULL );
ifdef DEBUG
	;Assert (pfce->hfont != NULL) with a call so as not
	;to mess up short jumps.
	call	LF42
endif ;DEBUG

;		  if (vfPrvwDisp && !pfce->fPrvw || !vfPrvwDisp && pfce->fPrvw)
;			goto LNextFce; /* Assembler Note: LF05 */
	mov	bl,[si.fPrvwFce]
	mov	cx,[vfPrvwDisp]
	jcxz	LF063
	xor	bl,maskfPrvwFce
LF063:
	and	bl,maskfPrvwFce
	jne	LF05

;                 /* Remove the FCE from its present position in its chain */

;                 if (pfce == pfti->pfce)
;                         {
        cmp     si,[di.pfceFti]
	jne	LF07

;                         pfti->pfce = pfce->pfceNext;
	mov	cx,[si.pfceNextFce]
	mov	[di.pfceFti],cx

;                         }
	jmp	short LF08

;                 else
;                         {
LF07:

;                         Assert( pfce->pfcePrev != NULL );
ifdef DEBUG
	;Assert (pfce->pfcePrev != NULL) with a call so as not
	;to mess up short jumps.
	call	LF44
endif ;DEBUG

;                         if ((pfce->pfcePrev->pfceNext = pfce->pfceNext) != 0)
        mov     cx,[si.pfceNextFce]
        mov     bx,[si.pfcePrevFce]
        mov     [bx.pfceNextFce],cx
	jcxz	LF08

;                                 pfce->pfceNext->pfcePrev = pfce->pfcePrev;
	push	bx
        mov     bx,[si.pfceNextFce]
	pop	[bx.pfcePrevFce]

;                         }
LF08:


;                 /* Optimization: don't actually select in the font if we know 
;                   its width and we're just formatting so that's all we need */

;                 if (fWidthsOnly)
	cmp	[fWidthsOnly],fFalse
	jz	LF09

;                         pfti->hfont = NULL;
	mov	[di.hFontFti],NULL
Ltemp003:
        jmp     LLoadFce

LF09:
;		else if (pfce->hfont == hfontSpecNil)
;			goto LValidateFce;
	mov	ax,[si.hfontFce]
	cmp	ax,hfontSpecNil
	je	LValidateFce

;		else if (hfontT = pfce->hfont, 
;			!FSelectFont( pfti, &hfontT, &hdc))
;			{
;/* font could not be selected so system font was selected in its place */

	mov	[hfontT],ax
	push	di	;save pfti
	push	si	;save pfce
	mov	bx,di
	lea	si,[hfontT]
	lea	di,[hdc]
	;LN_FNotSelectFont takes pfti in bx, phfont in si, phdc in di
	;ax, bx, cx, dx, si, di are altered.
	call	LN_FNotSelectFont
	pop	si	;restore pfce
	pop	di	;restore pfti
        or      ax,ax
	je	Ltemp003

;			FreeHandlesOfPfce(pfce);
;			goto LSystemFontErr;
;			}
        cCall   FreeHandlesOfPfce,<si>
	jmp	LSystemFontErr

;                 goto LLoadFce;
;                 }

	;Assembler note: this code has been moved to before the while loop.
;         pfce = pfce->pfceNext;
;         }
LF10:

; 
;  /* CASE 4: No FCE in table for requested font, must build one */
; 
; LNewFce:
LNewFce:
; 
;  /* Get a free FCE slot in which to build new entry */

;  pfce = PfceLruGet();   /* Kicks out LRU font, if no unallocated FCE slots exist */
	push	di	;save pfti
	;LN_PfceLruGet returns its result in si.
	;ax, bx, cx, dx, si, di are altered.
	call	LN_PfceLruGet
	pop	di	;restore pfti

LValidateFce:

;		pfce->fPrvwFce = vfPrvwDisp;
	and	[si.fPrvwFce],NOT maskfPrvwFce
	cmp	[vfPrvwDisp],fFalse
	je	LF105
	or	[si.fPrvwFce],maskfPrvwFce
LF105:

;  fFallback = fFalse;
;  pfce->fPrinter = pfti->fPrinter;
;  pfce->fGraphics = fTrue;	  /* the safer assumption in case of failure */
	mov	bptr ([fFallback]),fFalse
        mov     al,[di.fPrinterFti]
        and     al,maskfPrinterFti
	errnz	<(fGraphicsFce) - (fPrinterFce)>
	or	al,maskfGraphicsFce
	errnz	<maskFPrinterFti - maskFPrinterFce>
        and     [si.fPrinterFce],NOT maskfPrinterFti
        or      [si.fPrinterFce],al

;  pfce->fcidRequest = fcid;
	mov	ax,[fcid.LO_lFcid]
	mov	dx,[fcid.HI_lFcid]
        mov     [si.LO_fcidRequestFce],ax
        mov     [si.HI_fcidRequestFce],dx


;  if (pfti->fTossedPrinterDC)
;     {
        test    [di.fTossedPrinterDCFti],maskfTossedPrinterDCFti
	jz	LF11

;     Assert( pfti->fPrinter );
;     Assert( fWidthsOnly );
ifdef DEBUG
	;Assert (pfti->fPrinter && fWidthsOnly) with a call so as not
	;to mess up short jumps.
	call	LF46
endif ;DEBUG

; /* up till now, we have cheated -- we don't really have a printer DC,
;    but we have been able to manage without it since our cache is filled
;    with font widths.  Now comes the time to pay the piper.
;    Create a printer IC. */

;    if (!FReviveTossedPrinterDC(pfce))
;    	goto LLoadFce;

	cCall	FReviveTossedPrinterDC,<si>
	or	ax,ax
	jnz	LF11
	jmp	LLoadFce
;         }
;     }
LF11:

;  /* Translate FCID request to a logical font */

;  if (fcid.ibstFont == ibstFontNil || (!pfti->fPrinter && vpref.fDraftView))  /* ibstFontNil means use system font */
;     goto LSystemFont;
	cmp	[fcid.ibstFontFcid],ibstFontNil
        je      LSystemFont
        test    [di.fPrinterFti],maskfPrinterFti
	jnz	LF113
        test    [vpref.fDraftViewPref],maskfDraftViewPref
	jnz	LSystemFont
LF113:

;  pfce->fGraphics = FGraphicsFcidToPlf( fcid, &lf, pfce->fPrinter );
	push	si
	push	di
	mov	cl,[si.fPrinterFce]
	and	cx,maskfPrinterFce
	mov	dx,[fcid.HI_lFcid]
	mov	ax,[fcid.LO_lFcid]
	lea	di,[lf]
	;LN_FGraphicsFcidToPlf takes fcid in dx:ax, plf in di, fPrinterFont in cx
	;ax, bx, cx, dx, si, di are altered.
	; returns pffn->fGraphics in maskfGraphicsFfn bit of al
	call	LN_FGraphicsFcidToPlf
	pop	di
	pop	si   	; restore pfce
	errnz	<maskfGraphicsFce-maskfGraphicsFfn>
	and	[si.fGraphicsFce],al

;  /* Call Windows to request the font */

;  if ((pfce->hfont = CreateFontIndirect( (LPLOGFONT)&lf )) == NULL)
;         { 
        lea     ax,[lf]
        push    ss
        push    ax
ifdef HYBRID
	cCall	CreateFontIndirectPR,<>
else ;!HYBRID
	cCall	CreateFontIndirect,<>
endif ;!HYBRID
        mov     [si.hfontFce],ax
	errnz	<NULL>
        or      ax,ax
	jne	LF13


; LSystemFontErr:
LSystemFontErr:
;       SetErrorMat(matFont);
;	fFallback = fTrue;
	call	LN_SetErrorMat
	mov	bptr ([fFallback]),fTrue

; LSystemFont:
LSystemFont:
;       pfce->hfont = GetStockObject( pfti->fPrinter ?
;                                       DEVICEDEFAULT_FONT : SYSTEM_FONT );
        mov     ax,DEVICEDEFAULT_FONT
        test    [di.fPrinterFti],maskfPrinterFti
	jnz	LF12
	errnz	<DEVICEDEFAULT_FONT - SYSTEM_FONT - 1>
	dec	ax

LF12:
        push    ax
ifdef HYBRID
	cCall	GetStockObjectPR,<>
else ;!HYBRID
	cCall	GetStockObject,<>
endif ;!HYBRID
        mov     [si.hfontFce],ax
;         }
LF13:

;LogGdiHandle(pfce->hfont, 10000);
;#define LogGdiHandle(hGdi,wId) (vhgdis ? LogGdiHandleProc(hGdi,wId) : 0)
ifdef DEBUG
	;Do this DEBUG stuff with a call so as not
	;to mess up short jumps.
	call	LF59
endif ;DEBUG

;  /* Now fill out the FCE entry with the attributes of the new font */
;  /* A false return from FSelectFont means we got the system font instead,
;     but we don't care here */
;  FSelectFont( pfti, &pfce->hfont, &hdc ); 
	push	di	;save pfti
	push	si	;save pfce
	mov	bx,di
	add	si,(hfontFce)
	lea	di,[hdc]
	;LN_FNotSelectFont takes pfti in bx, phfont in si, phdc in di
	;ax, bx, cx, dx, si, di are altered.
	call	LN_FNotSelectFont
	pop	si	;restore pfce
	pop	di	;restore pfti

; LNewMetrics:
LNewMetrics:
;  Assert( hdc != NULL );
ifdef DEBUG
	;Assert (hdc != NULL) with a call so as not
	;to mess up short jumps.
	call	LF53
endif ;DEBUG

;  GetTextMetrics( hdc, (LPTEXTMETRIC) &tm );
        push    [hdc]
        lea     ax,[tm]
        push    ds
        push    ax
	cCall	GetTextMetrics,<>

	mov	bx,di	;save pfti
	push	si	;save pfce
	push	ss
	pop	es
	lea	di,[si.dxpOverhangFce]
	lea	si,[tm.tmAscent]

;  pfce->dxpOverhang = tm.tmOverhang;
	mov	ax,[tm.tmOverhang]
	stosw

;  pfce->dypAscent =   tm.tmAscent;
	errnz	<(dypAscentFce) - (dxpOverhangFce) - 2>
	movsw

;  pfce->dypDescent =  tm.tmDescent;
	errnz	<(dypDescentFce) - (dypAscentFce) - 2>
	errnz	<(tmDescent) - (tmAscent) - 2>
	lodsw
	stosw

;  pfce->dypXtraAscent = tm.tmAscent;
;  if (pfti->fPrinter)
;     pfce->dypXtraAscent += tm.tmExternalLeading;
	mov	ax,[tm.tmAscent]
	test	[bx.fPrinterFti],maskfPrinterFti
	jz	LF14
	add	ax,[tm.tmExtLeading]
LF14:
	errnz	<(dypXtraAscentFce) - (dypDescentFce) - 2>
	stosw

	pop	si	;restore pfce
	mov	di,bx	;restore pfti

;  /* Initialize the width table. */

;  Debug( vdbs.fShakeHeap ? ShakeHeap() : 0 );
ifdef DEBUG
	call	LN_ShakeHeap
endif ;DEBUG

;  pfce->fFixedPitch = fTrue;
;  pfce->dxpWidth = tm.tmAveCharWidth;
;  pfce->fVisiBad = fFalse;
        and     [si.fVisiBadFce],NOT maskfVisiBadFce
	or 	[si.fFixedPitchFce],maskfFixedPitchFce
        mov     ax,[tm.tmAveCharWidth]
        mov     [si.dxpWidthFce],ax

;  if ((tm.tmPitchAndFamily & maskFVarPitchTM) && !fFallback)
;	{
        test    [tm.tmPitch],maskFVarPitchTM
	je	Ltemp007
	cmp	bptr ([fFallBack]),fFalse
	jne	Ltemp007

;        int ch;     /* MUST be int so chDxpMax == 256 will work */
;        int *pdxp;
;        int dxp;
LF15:


;        if ((pfce->hqrgdxp = HqAllocLcb( (long)((chDxpMax - chDxpMin ) << 1))) == hqNil)
;/* lose track of logical font & hqrgdxp, but that's tolerable */
;		goto LSystemFontErr;

	mov	ax,(chDxpMax-chDxpMin) SHL 1
	cwd
	cCall	HqAllocLcb, <dx,ax>
	mov	[si.LO_hqrgdxpFce], ax
	mov	[si.HI_hqrgdxpFce], dx
	mov	cx,ax
	or	cx,dx
	jnz	Ltemp005
Ltemp006:
	jmp     LSystemFontErr
Ltemp007:
	jmp	LF21
Ltemp005:

;	lpdxp = LpLockHq( pfce->hqrgdxp );

	; dx:ax = hqrgdxp, si = pfce, di = pfti

	mov	di,ax
	mov	bx,dx
	shl	bx,1
	mov	ax,mpsbps [bx]
	mov	es,ax
	shr	ax,1
	jc	LF152
;	ReloadSb requires:
;		bx = sb * 2
;		ax = mpsbps [sb]/2
;	returns es = physical address of seg
;
;	reload sb trashes ax, cx, and dx
	cCall	ReloadSb,<>
LF152:
	
	mov	di,es:[di]	
	mov	ax,[si.HI_hqrgdxpFce]
	push	ax		; save hp for later unlock
	push	di
	cCall	LpLockHp,<ax,di>
	xchg	ax,di

;/* WINDOWS BUG WORKAROUND (DAVIDBO): GetCharWidth RIPs on any mapping mode
;   other than MM_TEXT
;	if (vfPrvwDisp)
;		OurGetCharWidth(hdc, chDxpMin, chDxpMax - 2, lpdxp);
;/* DRIVER BUG WORKAROUND (BL): Don't ask for n-255 or the EPSON24 and 
;   several other raster printer drivers will crash when you ask for widths */
;	else if (!GetCharWidth( hdc, chDxpMin, chDxpMax - 2, lpdxp ))
;		OurGetCharWidth( hdc, chDxpMin, chDxpMax - 2, lpdxp );
;	OurGetCharWidth( hdc, chDxpMax - 1, chDxpMax - 1, &lpdxp [chDxpMax-1] );

	; si = pfce, dx:di = lpdxp, hp is pushed

	push	di	;save low lpdxp
	push	dx	;save high lpdxp
	errnz	<chDxpMin - 0>
	xor	cx,cx
	mov	ax,chDxpMax-2
	errnz	<fFalse>
	cmp	[vfPrvwDisp],cx
	jne	LF157
	push	[hdc]
	push	cx
	push	ax
	push	dx
	push	di
	cCall	GetCharWidth,<>
	xchg	ax,cx
LF157:
	pop	es	;restore high lpdxp
	errnz	<chDxpMin - 0>
	jcxz	LF158
	mov	cx,chDxpMax-1
	add	di,(chDxpMax-1) SHL 1
LF158:
	;***Begin in line OurGetCharWidth
;HANDNATIVE void OurGetCharWidth( hdc, chFirst, chLast, lpdxp )
;HDC hdc;
;int chFirst, chLast;
;int far *lpdxp;
;{
; Assert( hdc != NULL );
; Assert( chFirst >= 0 && chLast <= 255 );
;
; while (chFirst <= chLast)
;	 {
;	 *lpdxp++ = GetTextExtent( hdc, &chFirst, 1 );
;	 chFirst++;
;	 }
;}
	push	es	;save high lpdxp
	push	cx	;get chFirst into memory
	mov	bx,sp	;address of chFirst
	mov	ax,1
	push	[hdc]
	push	ds
	push	bx
	push	ax
ifdef HYBRID
	cCall	GetTextExtentPR,<>
else ;!HYBRID
	cCall	GetTextExtent,<>
endif ;!HYBRID
	pop	cx	;restore chFirst
	pop	es	;restore high lpdxp
	stosw
	inc	cx
	cmp	cx,chDxpMax
	jb	LF158
	;***End in line OurGetCharWidth
	pop	di	;restore low lpdxp
	
;	if (pfce->dxpOverhang)
;		{
;		lpdxpMin = lpdxp;
;		lpdxpT = lpdxpMin + (chDxpMax - chDxpMin - 1);
;		while (lpdxpT >= lpdxpMin)
;			{
;			*lpdxpT-- -= pfce->dxpOverhang;
;			}
;		}
;	pfce->fFixedPitch = fFalse;
;	pfce->fVisiBad = (lpdxp [chSpace] != lpdxp [chVisSpace]);

	; es:di = lpdxp, hp is pushed, then pfce

	and	[si.fFixedPitchFce],NOT maskfFixedPitchFce
	mov	ax,es:[di+(chSpace SHL 1)]
	cmp	ax,es:[di+(chVisSpace SHL 1)]
	je	LF16
        or      [si.fVisiBadFce],maskfVisiBadFce	; bit was cleared above
LF16:
	mov	ax,[si.dxpOverhangFce]
	or	ax,ax
	jz	LF18
	mov	cx,(chDxpMax - chDxpMin)
LF19:
	sub	es:[di],ax
	inc	di
	inc	di
	loop	LF19
LF18:

;	UnlockHq( pfce->hqrgdxp );
;       }
;#define UnlockHq(hq)	 UnlockHp(HpOfHq(hq))
	
	; si = pfce, hp is pushed

	cCall	UnlockHp,<>
	push	ds
	pop	es
	mov	di,[pfti]
LF21:
	
; 
;  /* Now built fcidActual according to the properties of the font we got */
;  /* We only care about fcidActual for printer fonts */
; 
;  if (pfti->fPrinter)
;         {
        test    [di.fPrinterFti],maskfPrinterFti
	jnz	Ltemp004
	jmp	LF31
Ltemp004:
	push	di	;save pfti

;         union FCID fcidActual;
;         CHAR rgb [cbFfnLast];
;         struct FFN *pffn = (struct FFN *) &rgb [0];
	lea	di,[rgb]
; 
;         fcidActual.lFcid = 0L;
; 
;         /* Store back the real height we got */
; 
;         fcidActual.hps = umin( 0xFF,
;                 (NMultDiv( tm.tmHeight - tm.tmInternalLeading, czaInch,
;                         vfli.dyuInch ) + (czaPoint / 4)) / (czaPoint / 2));
        mov     ax,[tm.tmHeight]
        sub     ax,[tm.tmIntLeading]
	mov	dx,czaInch
	mov	bx,[vfli.dyuInchFli]
;	LN_NMultDiv performs NMultDiv(ax, dx, bx)
;	ax, bx, cx, dx are altered.
	call	LN_NMultDiv
        add     ax,czaPoint / 4
        mov     bx,czaPoint / 2
        cwd
ifdef DEBUG
	push	dx
	or	dx,dx
	jge	LF213
	neg	dx
LF213:
	cmp	dx,czaPoint / 4
	pop	dx
	jb	LF217
	push	ax
	push	bx
	push	cx
	push	dx
        mov     ax,midFormatn2
	mov	bx,1202
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LF217:
endif ;DEBUG
        idiv    bx
	errnz	<(hpsFcid) - 1>
	xchg	al,ah
	or	al,al
	je	LF22
	mov	ah,0FFh
LF22:

	;we now have ah = fcidActual.hps

;/* following line says: if we got back underlining, we can do double underlining too.
;   That's because we print double underlines by printing underlined spaces 
;   just below the first underline. Also let dotted ul through
;   regardless because it is only used on the screen */
;        if (tm.tmUnderlined || fcid.kul == kulDotted)
;                fcidActual.kul = fcid.kul;  
	errnz	<(kulFcid) - 0>
	mov	al,[fcid.kulFcid]
	and	al,maskKulFcid
	cmp	al,kulDotted SHL ibitKulFcid
	je	LF23
	cmp	[tm.tmUnderlined],fFalse
	jne	LF23
	mov	al,0
LF23:

;         if (tm.tmWeight > (FW_NORMAL + FW_BOLD) / 2)
;                 fcidActual.fBold = fTrue;
        cmp     [tm.tmWeight],(FW_NORMAL + FW_BOLD) / 2
	jle	LF24
	errnz	<(fBoldFcid) - 0>
	or	al,maskfBoldFcid
LF24:

;         if (tm.tmItalic)
;                 fcidActual.fItalic = fTrue;
	cmp	[tm.tmItalic],fFalse
	jz	LF25
	errnz	<(fItalicFcid) - 0>
	or	al,maskfItalicFcid
LF25:

;         if (tm.tmStruckOut)
;                 fcidActual.fStrike = fTrue;
	cmp	[tm.tmStruckOut],fFalse
	jz	LF26
	errnz	<(fStrikeFcid) - 0>
	or	al,maskfStrikeFcid
LF26:

;	fcidActual.prq = (tm.tmPitchAndFamily & maskFVarPitchTM) 
;					? VARIABLE_PITCH : FIXED_PITCH;
	mov	cl,VARIABLE_PITCH SHL shiftPrqFcid
        test    [tm.tmPitch],maskFVarPitchTM
	jnz	LF27
	errnz	<(prqFcid) - 0>
	mov	cl,FIXED_PITCH SHL shiftPrqFcid
LF27:
	or	al,cl

	;Assembler note: the low part of pfce->fcidActual = fcidActual
	;is actually done below in the C source.
	mov	[si.LO_fcidActualFce],ax

; /* For printer fonts, we may get back a font with a different name than
;    the one we requested.
;    If this in fact happened, add the new name to the master font table
; */

;         GetTextFace( hdc, LF_FACESIZE, (LPSTR) pffn->szFfn );
	lea	cx,[di.szFfn]
	push	cx	;first argument for FNeNcSz
	lea	ax,[lf.lfFaceNameLogfont]
	push	ax	;second argument for FNeNcSz
        push    [hdc]
        mov     ax,LF_FACESIZE
        push    ax
        push    ds
	push	cx
        call    far ptr GetTextFace

;         if (!FNeNcSz(pffn->szFfn, lf.lfFaceName))
;                 {
;                 /* The face name is the same as what we requested; so, the
;                 font index should be the same. */
;                 fcidActual.ibstFont = pfce->fcidRequest.ibstFont;
;                 }
	cCall	FNeNcSz,<>
        or      ax,ax
	errnz	<(ibstFontFcid) - 2>
        mov     al,[si.fcidRequestFce.ibstFontFcid]
	jz	LF30

;         else
;                 {   /* Font supplied by printer is different from request */
;                 int ibst;
; 
;                 pffn->ffid = tm.tmPitchAndFamily & maskFfFfid;
        mov     al,[tm.tmPitch]
        and     al,maskFfFfid
	mov	[di.ffidFfn],al

;                 pffn->cbFfnM1 = CbFfnFromCchSzFfn( CchSz( pffn->szFfn ) ) - 1;
;                 ChsPffn(pffn) = tm.tmCharSet;
	lea	bx,[di.szFfn]
        push    bx
	cCall	CchSz,<>
	xchg	ax,bx
	add	bl,(szFfn)
	mov	[di.cbFfnM1Ffn],bl
	mov	al,[tm.tmCharSet]
	mov	[di+bx],al

;		  if ((ibst = IbstFindSzFfn( vhsttbFont, pffn )) == iNil)
        push    [vhsttbFont]
	push	di
ifdef DEBUG
	cCall	S_IbstFindSzFfn,<>
else
	cCall	N_IbstFindSzFfn,<>
endif  ;DEBUG
        cmp     ax,iNil                         ; ax = ibst
	jne	LF29

;/* Font supplied by printer is not in table, add it */
;/* note: If ibstNil is returned from IbstAddStToSttb that's fine,
;   it just means we will select the system font whenever we use the
;   result. */
;			 {
;			 Assert ((ibstNil & 0xFF) == ibstFontNil);
;			 ibst = IbstAddStToSttb( vhsttbFont, pffn );
;			 }
	errnz	<(ibstNil AND 0FFh) - ibstFontNil>
        push    [vhsttbFont]
	push	di
	cCall	IbstAddStToSttb,<>

LF29:

;                 fcidActual.ibstFont = ibst;
;                 }
LF30:

;         pfce->fcidActual = fcidActual;
	;Assembler note: the low part of pfce->fcidActual = fcidActual
	;has already been done above.
	errnz	<(ibstFontFcid) - 2>
	xor	ah,ah

;         }
	pop	di	;restore pfti
	jmp	short LF33

LF31:
;  else
;         {   /* Screen font - don't care about fcidActual */
;         if (vpref.fDraftView &&
;             (fcid.fBold || fcid.fItalic || fcid.kul != kulNone || fcid.fStrike)
;                 fcid.kul = kulSingle;
;         pfce->fcidActual = fcid;
	mov	ax,[fcid.LO_lFcid]
	test	[vpref.fDraftViewPref],maskfDraftViewPref
	jz	LF32
	errnz	<(fBoldFcid) - 0>
	errnz	<(fItalicFcid) - 0>
	errnz	<(fStrikeFcid) - 0>
	errnz	<(kulFcid) - 0>
	errnz	<kulNone>
	test	al,maskFBoldFcid OR maskFItalicFcid OR maskKulFcid OR maskFStrikeFcid
	jz	LF32
	and	al,NOT maskKulFcid
	or	al,kulSingle SHL ibitKulFcid
LF32:
        mov     [si.LO_fcidActualFce],ax
	mov	ax,[fcid.HI_lFcid]

;         }
LF33:
	mov	[si.HI_fcidActualFce],ax

; LLoadFce:
LLoadFce:

;  /* Put pfce at the head of the chain extending from pfti */
;  /* When we get here, pfce is not in any chain */

;  pfceHead = pfti->pfce;
;  pfti->pfce = pfce;
	mov	bx,si
	xchg	[di.pfceFti],bx 	; bx = pfceHead

;  pfce->pfcePrev = NULL;
	mov	[si.pfcePrevFce],NULL

;  pfce->pfceNext = pfceHead;
        mov     [si.pfceNextFce],bx

;  if (pfceHead != NULL)
;          pfceHead->pfcePrev = pfce;
	errnz	<NULL>
        or      bx,bx
	jz	LF34
	mov	[bx.pfcePrevFce],si
LF34:

;  /* Load info about fce into *pfti */
; 
;  bltbyte( pfce, pfti, cbFtiFceSame );
	push	si
	push	di
	push	ds
	pop	es
	errnz	<cbFtiFceSame AND 1>
	mov	cx,cbFtiFceSame / 2
	rep	movsw
	pop	di
	pop	si
	; si = pfce, di = pfti, es = ds

;  vfli.fGraphics |= pfce->fGraphics;
	test	[si.fGraphicsFce],maskfGraphicsFce
	je	LF343
	or	[vfli.fGraphicsFli],maskFGraphicsFli
LF343:

; if (pfce->fFixedPitch)
;	SetWords( pfti->rgdxp, pfce->dxpWidth, 256 );
; else
;	bltbh( HpOfHq( pfce->hqrgdxp ), (int huge *)pfti->rgdxp, 256 );
	mov	cx,256
	mov	ax,[si.dxpWidthFce]
	lea	di,[di.rgdxpFti]
	test	[si.fFixedPitchFce],maskfFixedPitchFce
	jnz	LF35
	mov	bx,[si.HI_hqrgdxpFce]
	mov	si,[si.LO_hqrgdxpFce]
	shl	bx,1
	mov	ax,mpsbps[bx]
	mov	es,ax
	shr	ax,1
	jc	LF345
;	ReloadSb requires:
;		bx = sb * 2
;		ax = mpsbps [sb]/2
;	returns es = physical address of seg
;
;	reload sb trashes ax, cx, and dx
	cCall	ReloadSb,<>
LF345:
	push	es
	pop	ds
	mov	si,[si]
	push	ss
	pop	es
	mov	cx,256
	rep	movsw
	push	ss
	pop	ds

	db	03Dh	;turns next "rep stosw" into "cmp ax,immediate"
LF35:	rep	stosw

;  Debug( vdbs.fCkFont ? CkFont() : 0 );
ifdef DEBUG
	cmp	[vdbs.fCkFontDbs],fFalse
	jz	LF36
	cCall	CkFont,<>
LF36:
endif ;DEBUG

LF37:
; }
cEnd


;  Assert( vflm != flmIdle );
ifdef DEBUG
LF40:
        cmp     [vflm],flmIdle
	jne	LF41
	push	ax
	push	bx
	push	cx
	push	dx
        mov     ax,midFormatn2
        mov     bx,900
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LF41:
	ret
endif ;DEBUG

;                 Assert( pfce->hfont != NULL );
ifdef DEBUG
LF42:
	cmp	[si.hFontFce],NULL
	jne	LF43
	push	ax
	push	bx
	push	cx
	push	dx
        mov     ax,midFormatn2
        mov     bx,901
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LF43:
	ret
endif ;DEBUG


;                         Assert( pfce->pfcePrev != NULL );
ifdef DEBUG
LF44:
	cmp	[si.pfcePrevFce],NULL
	jne	LF45
	push	ax
	push	bx
	push	cx
	push	dx
        mov     ax,midFormatn2
        mov     bx,902
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LF45:
	ret
endif ;DEBUG

;     Assert( pfti->fPrinter );
;     Assert( fWidthsOnly );
ifdef DEBUG
LF46:
        test    [di.fPrinterFti],maskfPrinterFti
	jnz	LF47
	push	ax
	push	bx
	push	cx
	push	dx
        mov     ax,midFormatn2
        mov     bx,903
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LF47:
	cmp	[fWidthsOnly],fFalse
	jnz	LF48
	push	ax
	push	bx
	push	cx
	push	dx
        mov     ax,midFormatn2
        mov     bx,904
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LF48:
	ret
endif ;DEBUG


;  Assert( hdc != NULL );
ifdef DEBUG
LF53:
	cmp	[hdc],NULL
	jne	LF54
	push	ax
	push	bx
	push	cx
	push	dx
        mov     ax,midFormatn2
        mov     bx,906
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LF54:
	ret
endif ;DEBUG


ifdef DEBUG
LF56:
	push	ax
	push	bx
	push	cx
	push	dx
	push	di
	mov	di,[pfti]
	mov	ax,DEVICEDEFAULT_FONT
	cmp	[di.fPrinterFti],0
	jnz	LF57
	mov	ax,SYSTEM_FONT
LF57:
ifdef HYBRID
	cCall	GetStockObjectPR,<ax>
else ;!HYBRID
	cCall	GetStockObject,<ax>
endif ;!HYBRID
	cmp	ax,[si.hfontFce]
	jnz	LF58
	mov	ax,midFormatn2
	mov	bx,202
	cCall	AssertProcForNative,<ax,bx>
LF58:
	pop	di
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	jmp	LSystemFontErr
endif ;DEBUG

ifdef DEBUG
;LogGdiHandle(pfce->hfont, 10000);
;#define LogGdiHandle(hGdi,wId) (vhgdis ? LogGdiHandleProc(hGdi,wId) : 0)
LF59:
	cmp	[vhgdis],fFalse
	je	LF60
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,10000
	cCall	LogGdiHandleProc,<[si.hfontFce], ax>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LF60:
	ret
endif ;DEBUG


;-------------------------------------------------------------------------
;	FSelectFont( pfti, phfont, phdc )
;-------------------------------------------------------------------------
; native FSelectFont( pfti, phfont, phdc )
; struct FTI *pfti;
; HFONT *phfont;
; HDC *phdc;
; {
;  HFONT hfontOrig = *phfont;
;  int ww;
; 
;         HFONT hfontSystem = GetStockObject( SYSTEM_FONT );
;             HFONT hfontSav;
;             HANDLE hfontPhy;

	;LN_FNotSelectFont takes pfti in bx, phfont in si, phdc in di
	;ax, bx, cx, dx, si, di are altered.
; %%Function:LN_FNotSelectFont %%Owner:BRADV
PUBLIC LN_FNotSelectFont
LN_FNotSelectFont:
	push	[si]	    ;save hfontOrig

;  Assert( *phfont != NULL && *phfont != hfontSpecNil );
ifdef DEBUG	
	errnz	<NULL>
	errnz	<hfontSpecNil-1>
	cmp	wptr [si],hfontSpecNil
	ja	FSF01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFormatn2
	mov	bx,1062
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FSF01:
endif	; DEBUG

;  Assert( pfti != NULL );
ifdef DEBUG
	errnz	<NULL>
	or	bx,bx
	jne	FSF02
	push	ax
	push	bx
	push	cx
	push	dx
        mov     ax,midFormatn2
        mov     bx,1000
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FSF02:
endif ;DEBUG

;  if (pfti->fPrinter)
;         {       /* Selecting into printer DC */
	test	[bx.fPrinterFti],maskfPrinterFti
	push	bx	;save pfti
;	[sp] = pfti, [sp+2] = hfontOrig, bx = pfti, si = phfont, di = phdc
	jz	FSF04

;	Assert( vpri.hdc != NULL );
;         /* We should not have been called if there is no printer device */
;         Assert( vpri.pfti != NULL );
ifdef DEBUG
	;Assert (vpri.hdc != NULL) and
	;Assert (vpri.pfti != NULL) with a call so as not
	;to mess up short jumps.
	call	FSF17
endif ;DEBUG

;	*phdc = vpri.hdc;

	mov	ax,[vpri.hdcPri]
	mov	[di],ax

;         
;         if (SelectObject( *phdc, *phfont ) == NULL)
;             {
ifdef DEBUG
	cCall	SelectObjectFP,<ax,[si]>
else ;!DEBUG
ifdef HYBRID
	cCall	SelectObjectPR,<ax,[si]>
else ;!HYBRID
	cCall	SelectObject,<ax,[si]>
endif ;!HYBRID
endif ;!DEBUG
	errnz	<NULL>
        or      ax,ax
;	[sp] = pfti, [sp+2] = hfontOrig, si = phfont, di = phdc
	jnz	FSF03

;             /* Selecting a font into the printer DC failed */
;             /* This probably means we are out of global memory */
;             /* Use DEVICEDEFAULT_FONT instead; it is guaranteed to succeed */

;	      SetErrorMat(matFont);
	call	LN_SetErrorMat

;             *phfont = GetStockObject( DEVICEDEFAULT_FONT );
        mov     ax,DEVICEDEFAULT_FONT
ifdef HYBRID
	cCall	GetStockObjectPR,<ax>
else ;!HYBRID
	cCall	GetStockObject,<ax>
endif ;!HYBRID
	mov	[si],ax

;             AssertDo( SelectObject( *phdc, *phfont ) );
	;LN_OurSelectObject takes hdc in cx and h in [si].
	;The result is returned in ax.	ax, bx, cx, dx are altered.
	mov	cx,[di]
	call	LN_OurSelectObject
ifdef DEBUG
	;Assert (SelectObject( *phdc, *phfont ) with a call so as not
	;to mess up short jumps.
	call	FSF21
endif ;DEBUG

;             }
;	[sp] = pfti, [sp+2] = hfontOrig, si = phfont, di = phdc
FSF03:
	jmp	FSF13

;         }
;  else if (vfPrvwDisp)
;	  SelectPrvwFont(phFont, phdc);
FSF04:
	cmp	[vfPrvwDisp],fFalse
;	[sp] = pfti, [sp+2] = hfontOrig
	jz	FSF045
;	[sp] = pfti, [sp+2] = hfontOrig, si = phfont, di = phdc
	;***Begin in-line SelectPrvwFont
;EXPORT SelectPrvwFont(phfont, phdc)
;HFONT *phfont;
;HDC *phdc;
;	{
;	*phdc = PwwdWw(wwLayout)->hdc;
	mov	bx,[mpwwhwwd + (wwLayout SHL 1)]
	mov	bx,[bx]
	mov	cx,[bx.hdcWwd]
	mov	[di],cx

;	Assert(*phdc);
ifdef DEBUG
	;Assert (*phdc) with a call so as not
	;to mess up short jumps.
	call	FSF31
endif ;DEBUG

;	if (OurSelectObject(*phdc, *phfont) == NULL)
;		{
	;LN_OurSelectObject takes hdc in cx and h in [si].
	;The result is returned in ax.	ax, bx, cx, dx are altered.
	call	LN_OurSelectObject
	errnz	<NULL>
	or	ax,ax
	jne	FSF03

;		*phfont = GetStockObject(SYSTEM_FONT);
	mov	ax,SYSTEM_FONT
ifdef HYBRID
	cCall	GetStockObjectPR,<ax>
else ;!HYBRID
	cCall	GetStockObject,<ax>
endif ;!HYBRID
	mov	[si],ax

;		SelectObject(*phdc, *phfont);
ifdef DEBUG
	cCall	SelectObjectFP,<[di], [si]>
else ;!DEBUG
ifdef HYBRID
	cCall	SelectObjectPR,<[di], [si]>
else ;!HYBRID
	cCall	SelectObject,<[di], [si]>
endif ;!HYBRID
endif ;!DEBUG

;		}
;	}
	;***End in-line SelectPrvwFont

;	[sp] = pfti, [sp+2] = hfontOrig, si = phfont, di = phdc
	jmp	short FSF03

;  else
;         {   /* Selecting into screen DCs */
;	[sp] = pfti, [sp+2] = hfontOrig, si = phfont, di = phdc
FSF045:

;         HFONT hfontSystem = GetStockObject( SYSTEM_FONT );
        mov     ax,SYSTEM_FONT
ifdef HYBRID
	cCall	GetStockObjectPR,<ax>
else ;!HYBRID
	cCall	GetStockObject,<ax>
endif ;!HYBRID
	xchg	ax,dx

;	  Assert( vsci.pfti != NULL );	  /* This should always be true */
;					  /* or we should not have been called */
;	  Assert( vsci.hdcScratch );
ifdef DEBUG
	;Assert ( vsci.pfti != NULL ) and
	;Assert ( vsci.hdcScratch ) with a call so as not
	;to mess up short jumps.
	call	FSF24
endif ;DEBUG

;	[sp] = pfti, dx = hfontSystem, si = phfont, di = phdc
;	*phdc = vsci.hdcScratch;
	mov	ax,[vsci.hdcScratchSci]
	mov	[di],ax

;	[sp] = pfti, [sp+2] = hfontOrig,
;	dx = hfontSystem, si = phfont, di = phdc

ifdef DISABLE
;         if (*phfont != hfontSystem)
;             {
	cmp	[si],dx
endif ; DISABLE
	push	dx	;save hfontSystem
	push	di	;save phdc
	;[sp] = phdc, [sp+2] = hfontSystem, [sp+4] = pfti, 
	;[sp+6] = hfontOrig, si = phfont
ifdef DISABLE
	je	FSF06

;             HFONT hfontSav;
;             HANDLE hfontPhy;
;             int f;
;
;             if (!(hfontSav = SelectObject( vsci.hdcScratch, *phfont)))
;                 goto LScreenFail;
	;LN_OurSelectObject takes hdc in cx and h in [si].
	;The result is returned in ax.	ax, bx, cx, dx are altered.
	mov	cx,[vsci.hdcScratchSci]
	call	LN_OurSelectObject
	xchg	ax,cx
	;[sp] = phdc, [sp+2] = hfontSystem, [sp+4] = pfti, 
	;[sp+6] = hfontOrig, si = phfont
	jcxz	LScreenFail

;             hfontPhy = GetPhysicalFontHandle( vsci.hdcScratch );
;             if (!SelectObject( vsci.hdcScratch, hfontSav))
;		  goto LScreenFail;
	xchg	cx,[si] ;create pointer to hfontSav in si
	push	cx	;save *phfont
	mov	ax,[vsci.hdcScratchSci]
	push	ax
	cCall	GetPhysicalFontHandle,<ax>
	xchg	ax,di	    ;save hfontPhy
	;LN_OurSelectObject takes hdc in cx and h in [si].
	;The result is returned in ax.	ax, bx, cx, dx are altered.
	pop	cx
	call	LN_OurSelectObject
	pop	[si]	;restore *phfont (and trash hfontSav)
	xchg	ax,cx
	;[sp] = phdc, [sp+2] = hfontSystem, [sp+4] = pfti, 
	;[sp+6] = hfontOrig, si = phfont, di = hfontPhy
	jcxz	LScreenFail

;             ResetFont(fFalse /*fPrinterFont*/);
	errnz	<fFalse>
        xor     ax,ax
        push    ax
ifdef DEBUG
	cCall	S_ResetFont,<>
else
	push	cs
	call	near ptr N_ResetFont
endif  ;DEBUG

;             if (((f=GlobalFlags(hfontPhy)) & GMEM_LOCKCOUNT) == 0 &&
;                     (f & ~GMEM_LOCKCOUNT) != 0)
;                 {
	cCall	GlobalFlags,<di>
	errnz	<GMEM_LOCKCOUNT - 000FFh>
	or	al,al
	;[sp] = phdc, [sp+2] = hfontSystem, [sp+4] = pfti, 
	;[sp+6] = hfontOrig, si = phfont, di = hfontPhy
	jnz	FSF06
	or	ah,ah
	jz	FSF06

;                 Scribble(ispWireFont, 'W');
ifdef DEBUG
	;Do this debug stuff with a call so as not to mess up short jumps.
	call	FSF27
endif ;DEBUG

;                 GlobalWire(hfontPhy);
;                 GlobalUnlock(hfontPhy); /* because GlobalWire locked it */
	;[sp] = phdc, [sp+2] = hfontSystem, [sp+4] = pfti, 
	;[sp+6] = hfontOrig, si = phfont, di=hfontPhy
ifdef HYBRID
	cCall	GlobalWirePR,<di>
else ;!HYBRID
	cCall	GlobalWire,<di>
endif ;!HYBRID
ifdef HYBRID
	cCall	GlobalUnlockPR,<di>
else ;!HYBRID
	cCall	GlobalUnlock,<di>
endif ;!HYBRID

;                 }
;             }
FSF06:
endif ; DISABLE
	;[sp] = phdc, [sp+2] = hfontSystem, [sp+4] = pfti, 
	;[sp+6] = hfontOrig, si = phfont

;         for ( ;; )
FSF07:
;             {
;             /* Select the font into the scratchpad memory DC and all window DC's*/
; 
;             if (vsci.hdcScratch)
	mov	cx,[vsci.hdcScratchSci]
	;[sp] = phdc, [sp+2] = hfontSystem, [sp+4] = pfti, 
	;[sp+6] = hfontOrig, si = phfont
	jcxz	FSF09

;                 if (!SelectObject( vsci.hdcScratch, *phfont ))
;                     {
	;LN_OurSelectObject takes hdc in cx and h in [si].
	;The result is returned in ax.	ax, bx, cx, dx are altered.
	call	LN_OurSelectObject
        or      ax,ax
	;[sp] = phdc, [sp+2] = hfontSystem, [sp+4] = pfti, 
	;[sp+6] = hfontOrig, si = phfont
	jnz	FSF09

; /* could not select in the font: revert back to the system font */
; LScreenFail:
LScreenFail:
	;[sp] = phdc, [sp+2] = hfontSystem, [sp+4] = pfti, 
	;[sp+6] = hfontOrig, si = phfont

;		      SetErrorMat(matFont);
	call	LN_SetErrorMat

;                     if (*phfont != hfontSystem)
;                         {
	pop	bx	;pop phdc
	pop	ax	;restore hfontSystem
	cmp	[si],ax
	;[sp] = pfti, [sp+2] = hfontOrig, ax = hfontSystem, si = phfont
ifdef DEBUG
	je	FSF08
else ;not DEBUG
	je	FSF13
endif ;DEBUG

;                         *phfont = hfontSystem;
	mov	[si],ax

;                         continue;
	push	ax	;save hfontSystem
	push	bx	;save phdc
	;[sp] = phdc, [sp+2] = hfontSystem, [sp+4] = pfti,
	;[sp+6] = hfontOrig, si = phfont
	jmp	short FSF07

;                         }
; /* this should never happen, according to paulk, but just in case.. */
;                     Assert( fFalse );
ifdef DEBUG
FSF08:
	;Assert ( fFalse ) with a jump so as not
	;to mess up short jumps.
	;[sp] = pfti, [sp+2] = hfontOrig, ax = hfontSystem, si = phfont
	jmp	FSF30
endif ;DEBUG
;                     break;
;                     }
	;[sp] = phdc, [sp+2] = hfontSystem, [sp+4] = pfti, 
	;[sp+6] = hfontOrig, si = phfont
FSF09:

;             for ( ww = wwDocMin; ww < wwMac; ww++ )
;                 {
	mov	di,wwDocMin	      ; di = ww
	;[sp] = phdc, [sp+2] = hfontSystem, [sp+4] = pfti, 
	;[sp+6] = hfontOrig, si = phfont, di = ww
	jmp	short FSF12
FSF10:
	;[sp] = phdc, [sp+2] = hfontSystem, [sp+4] = pfti, 
	;[sp+6] = hfontOrig, si = phfont, di = ww

;                 struct WWD **hwwd = mpwwhwwd [ww];
	mov	bx,di
	mov	bx,[bx+di.mpwwhwwd]	; bx = hwwd

; /* test for hdc == NULL is for wwdtClipboard, whose DC is only 
;    present inside clipboard update messages.  See clipdisp.c */
;                 if (hwwd != hNil && (hdc = (*hwwd)->hdc) != NULL)
;                     {
	errnz	<hNil>
	or	bx,bx
	;[sp] = phdc, [sp+2] = hfontSystem, [sp+4] = pfti, 
	;[sp+6] = hfontOrig, si = phfont, di = ww
	je	FSF11
        mov     bx,[bx]
        mov     cx,[bx.hdcWwd]
	;[sp] = phdc, [sp+2] = hfontSystem, [sp+4] = pfti, 
	;[sp+6] = hfontOrig, si = phfont, di = ww
	jcxz	FSF11

;/* Hack to workaround problem with font mapper 3/18/89: for some unknown 
;   reason, the mapper won't select the Preview and Terminal screen
;   fonts for a window hdc, although it will for a memory DC compatible
;   with it(!).  To workaround this, store away hdc to make sure
;   the font cache gets the metrics that will accurately reflect the screen
;   results. (BL) */
;		    *phdc = hdc;
	pop	bx	;pop phdc
	push	bx	;repush phdc
	mov	[bx],cx

;                     if (!SelectObject( hdc, *phfont ))
	;LN_OurSelectObject takes hdc in cx and h in [si].
	;The result is returned in ax.	ax, bx, cx, dx are altered.
	call	LN_OurSelectObject

;                         goto LScreenFail;
        or      ax,ax
	;[sp] = phdc, [sp+2] = hfontSystem, [sp+4] = pfti, 
	;[sp+6] = hfontOrig, si = phfont, di = ww
        jz      LScreenFail

;                     }
FSF11:
	;[sp] = phdc, [sp+2] = hfontSystem, [sp+4] = pfti, 
	;[sp+6] = hfontOrig, si = phfont, di = ww
	inc	di

;                 }
FSF12:
	cmp	di,[wwMac]
	;[sp] = phdc, [sp+2] = hfontSystem, [sp+4] = pfti, 
	;[sp+6] = hfontOrig, si = phfont, di = ww
	jl	FSF10

;             break;
;             }   /* end for ( ;; ) */
;         }   
	;[sp] = phdc, [sp+2] = hfontSystem, [sp+4] = pfti, 
	;[sp+6] = hfontOrig, si = phfont, di = ww
	pop	ax	;remove phdc from the stack
	pop	ax	;remove hfontSystem from the stack

	;[sp] = pfti, [sp+2] = hfontOrig, ax = hfontSystem, si = phfont
FSF13:
	pop	bx	;restore pfti

;  Scribble(ispWireFont, ' ');
ifdef DEBUG
        test    [vdbs.grpfScribbleDbs], -1
	jz	FSF14
	push	ax
	push	bx
	push	cx
	push	dx
        mov     ax, ispWireFont
        mov     bx, 020h
        cCall   ScribbleProc, <ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FSF14:
endif ;DEBUG

;  pfti->hfont = *phfont;
	;[sp+2] = hfontOrig, ax = hfontSystem, bx = pfti, si = phfont
	mov	dx,[si]
	mov	[bx.hfontFti],dx

;  return *phfont == hfontOrig;
	pop	ax	;restore hfontOrig
	sub	ax,dx

; }
	ret

;	Assert( vpri.hdc != NULL );
;         Assert( vpri.pfti != NULL );
ifdef DEBUG	
FSF17:
	cmp	[vpri.hdcPri],NULL
	jne	FSF18
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFormatn2
	mov	bx,1063
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FSF18:
	cmp	[vpri.pftiPri],NULL
	jne	FSF19
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFormatn2
	mov	bx,1063
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FSF19:
	ret
endif	; DEBUG


;             AssertDo( SelectObject( *phdc, *phfont ) );
ifdef DEBUG
FSF21:
        or      ax,ax
	jnz	FSF22
	push	ax
	push	bx
	push	cx
	push	dx
        mov     ax,midFormatn2
        mov     bx,1003
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FSF22:
	ret
endif ;DEBUG


;         Assert( vsci.pfti != NULL );    /* This should always be true */
;                                         /* or we should not have been called */
;	Assert( vsci.hdcScratch );
ifdef DEBUG
FSF24:
	cmp	[vsci.pftiSci],NULL
	jnz	FSF25
	push	ax
	push	bx
	push	cx
	push	dx
        mov     ax,midFormatn2
        mov     bx,1004
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FSF25:
	cmp	[vsci.hdcScratchSci],fFalse
	jnz	FSF26
	push	ax
	push	bx
	push	cx
	push	dx
        mov     ax,midFormatn2
	mov	bx,1005
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FSF26:
	ret
endif ;DEBUG

;                 Scribble(ispWireFont, 'W');
ifdef DEBUG
FSF27:
        test    [vdbs.grpfScribbleDbs], -1
	jz	FSF28
	push	ax
	push	bx
	push	cx
	push	dx
        mov     ax, ispWireFont
	mov	bx, 057h
	cCall	ScribbleProc, <ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FSF28:
	ret
endif ;DEBUG


ifdef DEBUG
	;Assert ( fFalse ) with a jump so as not
	;to mess up short jumps.
FSF30:
	push	ax
	push	bx
	push	cx
	push	dx
        mov     ax,midFormatn2
        mov     bx,1005
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	;[sp] = pfti, [sp+2] = hfontOrig, ax = hfontSystem, si = phfont
	jmp	FSF13
endif ;DEBUG

;	Assert(*phdc);
ifdef DEBUG
FSF31:
	cmp	wptr [di],0
	jnz	FSF32
	push	ax
	push	bx
	push	cx
	push	dx
        mov     ax,midFormatn2
	mov	bx,1020
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FSF32:
	ret
endif ;DEBUG

LN_SetErrorMat:
        mov     ax,matFont
        cCall   SetErrorMatProc,<ax>
	ret

;-------------------------------------------------------------------------
;	ResetFont(fPrinterFont)
;-------------------------------------------------------------------------
; %%Function:N_ResetFont %%Owner:BRADV
cProc	N_ResetFont,<PUBLIC,FAR>,<si>
        ParmW   fPrinterFont
; /* R E S E T  F O N T */
; 
; native ResetFont(fPrinterFont)
; BOOL fPrinterFont;
;     {
; /* This routine sets to NULL the currently selected printer or screen font,
;    depending on the value of fPrint.  It does not free any fonts, leaving the
;    device's font chain intact.
; 
;    This should be done before freeing fonts to assure that none are selected
;    into DC's.  It may also be used if explicit use of the system font is required.
; 
; */
; 
;     extern BOOL vfPrinterValid;
;     struct FTI *pfti;
;     HFONT hfont;
;     HFONT hdcT;
        localW  hfont
        localW  hdcT

cBegin
;     if ((pfti = (fPrinterFont) ? vpri.pfti : vsci.pfti) == NULL)
;         return; /* device does not exist */
        mov     cx,[fPrinterFont]
        mov     si,[vsci.pftiSci]
	jcxz	RF01
        mov     si,[vpri.pftiPri]
RF01:
	errnz	<NULL>
        or      si,si                   ; si = pfti
	jz	RF05

;     if (fPrinterFont && vpri.pfti->fTossedPrinterDC)
;             {   /* printer DC was tossed away */
;             goto LNoFont;
;             }
	jcxz	RF02
        mov     bx,[vpri.pftiPri]
        test    [bx.fTossedPrinterDCFti],maskfTossedPrinterDCFti
        jnz     LNoFont
RF02:

;     hfont = GetStockObject( fPrinterFont ? DEVICEDEFAULT_FONT : SYSTEM_FONT );
        mov     ax,SYSTEM_FONT
	jcxz	RF03
	errnz	<DEVICEDEFAULT_FONT - SYSTEM_FONT - 1>
	inc	ax
RF03:
ifdef HYBRID
	cCall	GetStockObjectPR,<ax>
else ;!HYBRID
	cCall	GetStockObject,<ax>
endif ;!HYBRID
        mov     [hfont],ax

;     AssertDo(FSelectFont( pfti, &hfont, &hdcT ));
	push	di
        push    si
	mov	bx,si
	lea	si,[hfont]
	lea	di,[hdcT]
	;LN_FNotSelectFont takes pfti in bx, phfont in si, phdc in di
	;ax, bx, cx, dx, si, di are altered.
	call	LN_FNotSelectFont
	pop	si
	pop	di
ifdef DEBUG
	or	ax,ax
	je	RF04
	push	ax
	push	bx
	push	cx
	push	dx
        mov     ax,midFormatn2
	mov	bx,1102
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
RF04:
endif ;DEBUG

; LNoFont:
LNoFont:
;     pfti->hfont = NULL;   /* So we know there's no cached font in pfti->hdc */
	errnz	<NULL>
	xor	ax,ax
	mov	[si.hfontFti],ax

;     pfti->fcid.lFcid = fcidNil;
	errnz	<fcidNil_LO - 0FFFFh>
	errnz	<fcidNil_HI - 0FFFFh>
	dec	ax
	mov	[si.fcidFti.LO_lFcid],ax
	mov	[si.fcidFti.HI_lFcid],ax

;     }
RF05:
cEnd


;-------------------------------------------------------------------------
;	FGraphicsFcidToPlf( fcid, plf, fPrinterFont )
;-------------------------------------------------------------------------
; %%Function:N_FGraphicsFcidToPlf %%Owner:BRADV
cProc	N_FGraphicsFcidToPlf,<PUBLIC,FAR>,<si,di>
        ParmD   fcid
        ParmW   plf
        ParmW   fPrinterFont
; 
; /* F C I D  T O  P L F */
; 
; native FGraphicsFcidToPlf( fcid, plf, fPrinterFont )
; union FCID fcid;
; LOGFONT *plf;
; int fPrinterFont;
; {   /* Translate an FCID into a windows logical font request structure */
;  int ps;
;  struct FFN *pffn;
; 
cBegin
	mov	ax,[OFF_fcid]
	mov	dx,[SEG_fcid]
	mov	di,[plf]
	mov	cx,[fPrinterFont]
	call	LN_FGraphicsFcidToPlf
	and	ax,maskFGraphicsFfn
cEnd

	;LN_FGraphicsFcidToPlf takes fcid in dx:ax, plf in di, fPrinterFont in cx
	;ax, bx, cx, dx, si, di are altered.
	; returns pffn->fGraphics in maskfGraphicsFfn bit of al
LN_FGraphicsFcidToPlf:
;  SetBytes(plf, 0, sizeof(LOGFONT));
	push	ds
	pop	es
; 
;  /* Scale the request into device units */
; 
;  Assert( fcid.hps > 0 );
ifdef DEBUG
	errnz	<(hpsFcid) - 1>
	or	ah,ah
	ja	FTP01
	push	ax
	push	bx
	push	cx
	push	dx
        mov     ax,midFormatn2
        mov     bx,1100
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FTP01:
endif ;DEBUG
;plf->lfHeight = NMultDiv( fcid.hps * (czaPoint / 2),
;							fPrinterFont ? vfli.dyuInch : vfli.dysInch,
;							czaInch );
;/* (BL) *** HACK ***   ***  ACK *** **** GAG ****  *** BARF ***
;		The Windows courier font has interesting non-international pixels
;		in its internal leading area. So, when picking courier
;		screen fonts, use positive lfHeight, meaning choose the font by 
;		CELL height instead of CHARACTER height. 
;		ChrisLa says this fiasco is Aldus' fault. */
;if (fcid.ibstFont != ibstCourier || fPrinterFont)
;	plf->lfHeight = -plf->lfHeight;
	push	ax	;save LO_fcid
	push	cx	;save fPrinterFont
	push	dx	;save HI_fcid
	mov	al,czaPoint / 2
	errnz	<(hpsFcid) - 1>
        mul     ah
	mov	dx,[vfli.dysInchFli]
	jcxz	FTP02
	mov	dx,[vfli.dyuInchFli]
FTP02:
	mov	bx,czaInch
;	LN_NMultDiv performs NMultDiv(ax, dx, bx)
;	ax, bx, cx, dx are altered.
	call	LN_NMultDiv
	pop	dx	;restore HI_fcid
	pop	cx	;restore fPrinterFont
	push	cx
	errnz	<(ibstFontFcid) - 2>
	cmp	dl, ibstCourier
	jne	FTP02Char
	jcxz	FTP02Cell
FTP02Char:
        neg     ax
FTP02Cell:
	errnz	<(lfHeightLogfont) - 0>
	stosw
	xor	ax,ax
	errnz	<(lfWidthLogfont) - (lfHeightLogfont) - 2>
	stosw
	errnz	<(lfEscapementLogfont) - (lfWidthLogfont) - 2>
	stosw
	errnz	<(lfOrientationLogfont) - (lfEscapementLogfont) - 2>
	stosw
; 
; 
;  Assert( fcid.ibstFont < (*vhsttbFont)->ibstMac );
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
        mov     bx,[vhsttbFont]
        mov     bx,[bx]
	errnz	<(ibstFontFcid) - 2>
	xor	dh,dh
	cmp	dx,[bx.ibstMacSttb]
	jl	FTP03
        mov     ax,midFormatn2
        mov     bx,1101
        cCall   AssertProcForNative,<ax, bx>
FTP03:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

;  pffn = PstFromSttb( vhsttbFont, fcid.ibstFont );
        push    [vhsttbFont]
	errnz	<(ibstFontFcid) - 2>
	xor	dh,dh
	push	dx
	cCall	PstFromSttb,<>
	push	ds
	pop	es	;restore es
	xchg	ax,si			; si = pffn
	pop	cx	;restore fPrinterFont
	pop	bx	;restore LO_fcid

; plf->lfPitchAndFamily = (pffn->ffid & maskFfFfid) | fcid.prq;

;PAUSE
	mov	ah,[si.ffidFfn]
	errnz	<(prqFcid)-0>
	mov	al,bl
	and	ax,maskPrqFcid + (maskFfFfid SHL 8)
	errnz	<maskPrqFcid-0C0h>
	rol	al,1
	rol	al,1
	or	al,ah
	mov	[di.lfPitchAndFamilyLogfont - bptr (lfWeightLogfont)],al


;  /* Set Bold, Italic, StrikeOut, Underline */

;  plf->lfWeight = fcid.fBold ? FW_BOLD : FW_NORMAL;
	errnz	<(fBoldFcid) - 0>
	test	bl,maskFBoldFcid
	mov	ax,FW_NORMAL
	jz	FTP08
	mov	ax,FW_BOLD
FTP08:
	errnz	<(lfWeightLogfont) - (lfOrientationLogfont) - 2>
	stosw

;  if (fcid.fItalic)
	xor	ax,ax
	errnz	<(fItalicFcid) - 0>
	test	bl,maskfItalicFcid
	jz	FTP09
;         plf->lfItalic = 1;
	inc	ax
FTP09:

;  if (fPrinterFont && fcid.kul != kulNone)
	jcxz	FTP10
	errnz	<(kulFcid) - 0>
	errnz	<kulNone - 0>
	test	bl,maskKulFcid
	jz	FTP10
;         plf->lfUnderline = 1;
	inc	ah
FTP10:
	errnz	<(lfItalicLogfont) - (lfWeightLogfont) - 2>
	errnz	<(lfUnderlineLogfont) - (lfItalicLogfont) - 1>
	stosw

;  if (fcid.fStrike)
	xor	ax,ax
	errnz	<(fStrikeFcid) - 0>
	test	bl,maskfStrikeFcid
	jz	FTP11

;         plf->lfStrikeOut = 1;
	inc	ax
FTP11:

; /* underlining for the screen is handled by drawing lines */
;  plf->lfCharSet = ChsPffn(pffn);
	xor	bx,bx
	mov	bl,[si.cbFfnM1Ffn]
	mov	ah,[si+bx]
	errnz	<(lfStrikeOutLogfont) - (lfUnderlineLogfont) - 1>
	errnz	<(lfCharSetLogfont) - (lfStrikeOutLogfont) - 1>
	stosw

	errnz	<(lfOutPrecisionLogfont) - (lfCharSetLogfont) - 1>
	errnz	<(lfClipPrecisionLogfont) - (lfOutPrecisionLogfont) - 1>
	errnz	<(lfQualityLogfont) - (lfClipPrecisionLogfont) - 1>
	xor	ax,ax
	stosw
	stosb
	xchg	ax,cx	    ;save fPrinterFont

;  bltbyte( pffn->szFfn, plf->lfFaceName, LF_FACESIZE );
	push	si
	add	si,(szFfn)
	errnz	<(lfPitchAndFamilyLogfont) - (lfQualityLogfont) - 1>
	inc	di
	errnz	<(lfFaceNameLogfont) - (lfPitchAndFamilyLogfont) - 1>
	errnz	<LF_FACESIZE AND 1>
	mov	cx,LF_FACESIZE / 2
	rep	movsw
	pop	si

;  if (vfPrvwDisp && !fPrinterFont)
;    GenPrvwPlf(plf);
	cmp	[vfPrvwDisp],fFalse
	jz	FTP13
	or	ax,ax
        jnz     FTP13
	sub	di,(lfFaceNameLogfont)+LF_FACESIZE
	cCall	GenPrvwPlf,<di>
FTP13:
;	return pffn->fGraphics;
	mov	al,[si.fGraphicsFfn]
	or	al,NOT maskfGraphicsFfn

;  }
	ret


;-------------------------------------------------------------------------
;	PfceLruGet()
;-------------------------------------------------------------------------
	;LN_PfceLruGet returns its result in si.
	;ax, bx, cx, dx, si, di are altered.
; %%Function:LN_PfceLruGet %%Owner:BRADV
PUBLIC LN_PfceLruGet
LN_PfceLruGet:
; /* P F C E  L R U  G E T */
; 
; native struct FCE * (PfceLruGet())
; /* tosses out the LRU cache entry's information */
; {
;  struct FCE *pfce, *pfceEndChain;
;  struct FTI *pfti;
; 
;  Debug( pfceEndChain = NULL );
ifdef DEBUG
	xor	bx,bx				; bx = pfceEndChain
endif
; 
;  /* motivation for algorithm is to free the LRU font if there is only */
;  /* one device's fonts in the cache; and to alternate randomly between */
;  /* the LRU fonts for each device if there are 2 */
; 
;  /* NOTE: This algorithm could repeatedly select the same slot if there */
;  /*       was a length-1 chain and a length-ifcMax-1 chain, but this case */
;  /*       is not interesting here */
; 
;  /* Search for an appropriate FCE slot to free: */
;  /*     1. If we find a slot that is free, use that */
;  /*     2. If not, use the last end-of-chain we come to */
; 
;  for ( pfce = rgfce; pfce < &rgfce [ifceMax]; pfce++ )
;         {
	lea	si,[rgfce]			; si = pfce
	mov	cx,ifceMax

PLG01:
;         if (pfce->hfont == NULL)
;                 return pfce;
	cmp	[si.hfontFce],NULL
	je	PLG07

;         else if (pfce->pfceNext == NULL)
	cmp	[si.pfceNextFce],NULL
	jne	PLG02

;                 pfceEndChain = pfce;
	mov	bx,si

;         }
PLG02:
	add	si,cbFceMin
	loop	PLG01

; /* found a currently allocated cache entry at the end of its device's LRU
;    chain.  Remove it from the chain and free its handles */

;  Assert( pfceEndChain != NULL );
ifdef DEBUG
	;Assert( pfceEndChain != NULL ) with a call so as not to mess up
	;short jumps.
	call	PLG08
endif ;DEBUG

;  pfti = pfceEndChain->fPrinter ? vpri.pfti : vsci.pfti;
        mov     di,[vpri.pftiPri]               ; di = pfti
	test	[bx.fPrinterFce],maskfPrinterFce
	jnz	PLG03
        mov     di,[vsci.pftiSci]
PLG03:

;  if (pfceEndChain->pfcePrev == NULL)
;         {
	cmp	[bx.pfcePrevFce],NULL
	jne	PLG04

;         /* Since this font is at the head of the chain, it may be the one */
;         /* currently selected into the device DC(s).  If so, deselect it */

;         ResetFont( pfti->fPrinter );
	;PLG10 performs ResetFont( di->fPrinter );
	;ax, cx, dx are altered.
	call	PLG10

;         pfti->pfce = NULL;
	mov	[di.pfceFti],NULL

;         }
	jmp	short PLG06

PLG04:
;  else
;         {
;         /* also reset font selected into DC if the hfont is null -- we may
;            have left this font selected in because of the fWidthsOnly
;            optimization */
;         if (pfti->hfont == NULL)
	cmp	[di.hfontFti],NULL
	jne	PLG05

;             ResetFont( pfti->fPrinter );
	;PLG10 performs ResetFont( di->fPrinter );
	;ax, cx, dx are altered.
	call	PLG10

PLG05:
;         pfceEndChain->pfcePrev->pfceNext = NULL;
;         pfceEndChain->pfcePrev = NULL;
	errnz	<NULL>
	xor	si,si
	xchg	si,[bx.pfcePrevFce]
	mov	[si.pfceNextFce],NULL

;         }
PLG06:

;  FreeHandlesOfPfce( pfceEndChain );
	mov	si,bx
	push	si
        call    far ptr FreeHandlesOfPfce

;  return pfceEndChain;
; }
PLG07:
	ret


;  Assert( pfceEndChain != NULL );
ifdef DEBUG
PLG08:
	or	bx,bx
	jne	PLG09
	push	ax
	push	bx
	push	cx
	push	dx
        mov     ax,midFormatn2
        mov     bx,1200
        cCall   AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
PLG09:
	ret
endif ;DEBUG


	;PLG10 performs ResetFont( di->fPrinter );
	;ax, cx, dx are altered.
PLG10:
;             ResetFont( pfti->fPrinter );
	push	bx	;save pfceEndChain
        mov     al,[di.fPrinterFti]
        and     ax,maskfPrinterFti
        push    ax
ifdef DEBUG
	cCall	S_ResetFont,<>
else
	push	cs
	call	near ptr N_ResetFont
endif
	pop	bx	;restore pfceEndChain
	ret


ifdef DEBUG
;  Debug( vdbs.fShakeHeap ? ShakeHeap() : 0 );
LN_ShakeHeap:
        cmp     [vdbs.fShakeHeapDbs],0
	jz	LN_SH01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,1
        cCall   ShakeHeapSb,<ax>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LN_SH01:
	ret
endif ;DEBUG


;/* O U R  S E L E C T	O B J E C T */
;/*  performs a SelectObject, reducing the swap area SOME if it fails */
;OurSelectObject(hdc, h)
;HDC hdc;
;HANDLE h;
;{
;    extern int vsasCur;
;    HANDLE hRet = SelectObject(hdc, h);
;
;    if (hRet == NULL && vsasCur <= sasOK)
;	 {
;	 /* reduce swap area to an acceptable level (won't swap too much, but
;	    may have more room for fonts) */
;	 OurSetSas(sasOK);
;	 hRet = SelectObject(hdc, h);
;	 /* increase the swap area back as far as we can */
;	 OurSetSas(sasFull);
;	 }
;
;    return hRet;
;}
	;LN_OurSelectObject takes hdc in cx and h in [si].
	;The result is returned in ax.	ax, bx, cx, dx are altered.
LN_OurSelectObject:
	push	cx	;save hdc
ifdef DEBUG
	cCall	SelectObjectFP,<cx, [si]>
else ;!DEBUG
ifdef HYBRID
	cCall	SelectObjectPR,<cx, [si]>
else ;!HYBRID
	cCall	SelectObject,<cx, [si]>
endif ;!HYBRID
endif ;!DEBUG
	pop	cx	;restore hdc
	or	ax,ax
	jne	OSO01
	cmp	[vsasCur],sasMin
	ja	OSO01
	push	cx	;save hdc
	mov	ax,sasMin
	cCall	OurSetSas,<ax>
	pop	cx	;restore hdc
ifdef DEBUG
	cCall	SelectObjectFP,<cx, [si]>
else ;!DEBUG
ifdef HYBRID
	cCall	SelectObjectPR,<cx, [si]>
else ;!HYBRID
	cCall	SelectObject,<cx, [si]>
endif ;!HYBRID
endif ;!DEBUG
	push	ax	;save hRet
	errnz	<sasFull - 0>
	xor	ax,ax
	cCall	OurSetSas,<ax>
	pop	ax	;restore hRet
OSO01:
	ret


sEnd    format
        end
