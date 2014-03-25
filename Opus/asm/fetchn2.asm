	include w2.inc
	include noxport.inc
	include consts.inc
	include structs.inc

createSeg	_FETCH,fetch,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midFetchn2	equ 6		; module ID, for native asserts
endif

; EXPORTED LABELS

; EXTERNAL FUNCTIONS

externFP	<IMacPlc,CpPlc,CpMac2Doc>
externFP	<ReloadSb>
externFP	<N_ApplyPrlSgc>
externFP	<StandardPap>
externNP	<LN_LprgcpForPlc>
externNP	<LN_IcpInRgcpAdjusted>
externNP	<LN_DoPrmSgc>
externNP	<LN_HpchGetPn>
externNP	<LN_BFromFcCore>
externNP	<LN_MapStc>

ifdef DEBUG
externFP	<AssertProcForNative>
externFP	<S_ApplyPrlSgc>
externFP	<LockHeap, UnlockHeap>
endif


; EXTERNAL DATA

sBegin	data

externW mpdochdod	; extern struct DOD	  **mpdochdod[];
externW vchpStc 	; extern struct CHP	  vchpStc;
externW mpfnhfcb	; extern struct FCB	  **mpfnhfcb[];
externW caPara		; extern struct CA	  caPara;
externW vpapFetch	; extern struct PAP	  vpapFetch;
externW vpapStc 	; extern struct PAP	  vpapStc;
externW vstcLast	; extern int		  vstcLast;
externW vcchPapxLast	; extern int		  vcchPapxLast;
externW vtapStc 	; extern struct TAP	  vtapStc;

externW vdocPapLast	; extern int		vdocPapLast;
externW vfnPapLast	; extern int		vfnPapLast;
externW vpnPapLast	; extern int		vpnPapLast;
externW vbpapxPapLast	; extern int		vbpapxPapLast;

externW mpsbps		; extern SB		mpsbps[];

ifdef DEBUG
externW 		cHpFreeze
endif

sEnd	data


; CODE SEGMENT _FETCH

sBegin	fetch
	assumes cs,fetch
	assumes ds,dgroup
	assumes ss,dgroup


;-------------------------------------------------------------------------
;	LN_ReloadSb
;-------------------------------------------------------------------------
	;LN_ReloadSb takes an sb in bx and set es to the corresponding value.
	;Only es and bx are altered.
; %%Function: LN_ReloadSb %%Owner:BRADV
PUBLIC LN_ReloadSb
LN_ReloadSb:

	push	ax
	shl	bx,1
	mov	ax,[bx.mpsbps]
	mov	es,ax
	shr	ax,1
	jc	LN_RS01
;	ReloadSb trashes ax, cx, and dx
ifdef PROFILE
	cCall	StartNMeas,<>
endif ;PROFILE
	push	cx
	push	dx
	cCall	ReloadSb,<>
	pop	dx
	pop	cx
ifdef PROFILE
	cCall	StopNMeas,<>
endif ;PROFILE
LN_RS01:
	pop	ax
	ret

;-------------------------------------------------------------------------
;	PdodOrNilFromDoc(doc)
;-------------------------------------------------------------------------
;	PdodOrNilFromDoc takes a doc in bx, and returns hNil in ax and bx
;	if mpdochdod[doc] == hNil.  Otherwise it returns doc in ax
;	and pdod = *mpdochdod[doc] in bx.  The zero flag is set according
;	to the value of bx upon return.  Only ax and bx are altered.

; %%Function:LN_PdodOrNilFromDoc %%Owner:BRADV
PUBLIC LN_PdodOrNilFromDoc
LN_PdodOrNilFromDoc:

	mov	ax,bx
	shl	bx,1
	mov	bx,[mpdochdod.bx]
	or	bx,bx
	mov	bx,[bx]
	jnz	LN_PONFD01
	errnz	<hNil>
	xor	ax,ax
LN_PONFD01:
	ret


;-------------------------------------------------------------------------
;	DocMother(doc)
;-------------------------------------------------------------------------
; %%Function:DocMother %%Owner:BRADV
PUBLIC DocMother
DocMother:
; /* D O C  M O T H E R */
; /* returns mother doc of a doc, doc itself if not short */
; native int DocMother(doc)
; int doc;
; {
;	  struct DOD *pdod, **hdod = mpdochdod[doc];
;
;	  if (hdod == hNil)
;	      return docNil;
;	  pdod = *hdod;
;
;	  return((pdod->fMother) ? doc : pdod->doc);

	mov	bx,sp
	mov	bx,[bx+4]
	call	LN_PdodOrNilFromDoc
	jz	DM01
	cmp	[bx.fMotherDod],fFalse
	jnz	DM01
	mov	ax,[bx.docDod]
DM01:
; }
	db	0CAh, 002h, 000h    ;far ret, pop 2 bytes


;-------------------------------------------------------------------------
;	N_PdodMother(doc)
;-------------------------------------------------------------------------
; %%Function:N_PdodMother %%Owner:BRADV
PUBLIC N_PdodMother
N_PdodMother:
; /* P D O D  M O T H E R */
; /* returns mother dod of a doc, doc's dod if a "mother" doc */
; native struct DOD *PdodMother(doc)
; int doc;
; {
;	  struct DOD *pdod = *mpdochdod[doc];
;	  Assert (mpdochdod[doc] != hNil);
;
;	  if (!pdod->fMother)
;	      {
;	      Assert (mpdochdod[pdod->doc] != hNil);
;	      return *mpdochdod[pdod->doc];
;	      }
;	  else
;	      return pdod;

	mov	bx,sp
	mov	bx,[bx+4]
	call	LN_PdodOrNilFromDoc
ifdef DEBUG
	jnz	PDM01
	inc	bp
	push	bp
	mov	bp,sp	;set up bp chain for call
	push	ax
	push	bx
	mov	ax,midFetchn2
	mov	bx,132
	cCall	AssertProcForNative,<ax,bx>
	pop	bx
	pop	ax
	pop	bp	;restore old bp chain
	dec	bp
PDM01:
endif ;DEBUG
	cmp	[bx.fMotherDod],fFalse
	jnz	PDM03
	mov	bx,[bx.docDod]
	call	LN_PdodOrNilFromDoc
ifdef DEBUG
	jnz PDM02
	inc	bp
	push	bp
	mov	bp,sp	;set up bp chain for call
	push	ax
	push	bx
	mov	ax,midFetchn2
	mov	bx,134
	cCall	AssertProcForNative,<ax,bx>
	pop	bx
	pop	ax
	pop	bp	;restore old bp chain
	dec	bp
PDM02:
endif ;DEBUG
PDM03:
	xchg	ax,bx
; }
	db	0CAh, 002h, 000h    ;far ret, pop 2 bytes


;-------------------------------------------------------------------------
;	DocDotMother(doc)
;-------------------------------------------------------------------------
; %%Function:DocDotMother %%Owner:BRADV
PUBLIC DocDotMother
DocDotMother:
; /* D O C  D O T  M O T H E R */
; /*  returns the docDot of the mother of doc. */
; native int DocDotMother (doc)
; int doc;
; {
; DDM01:
;	struct DOD *pdod, **hdod = mpdochdod[doc];
;
;	if (hdod == hNil) return docNil;
;
;	if ((pdod = *hdod)->fDoc)
;		return pdod->docDot;
;	else if (pdod->fDot)
;		return doc;
;	else
;		{
;		/* return DocDotMother(pdod->doc); */
;		doc = pdod->doc;
;		goto DDM01;
;		}
; }

	mov	bx,sp
	mov	bx,[bx+4]
DDM01:
	call	LN_PdodOrNilFromDoc
	jz	DDM03
	test	[bx.fDocDod],maskFDocDod
	jnz	DDM02
	test	[bx.fDotDod],maskFDotDod
	jnz	DDM03
	mov	bx,[bx.docDod]
	jmp	short DDM01
DDM02:
	mov	ax,[bx.docDotDod]
DDM03:
; }
	db	0CAh, 002h, 000h    ;far ret, pop 2 bytes


;-------------------------------------------------------------------------
;	PnFromPlcbteFc(hplcbte,fc)
;-------------------------------------------------------------------------
; %%Function:N_PnFromPlcbteFc %%Owner:BRADV
cProc	N_PnFromPlcbteFc,<PUBLIC,FAR>,<si,di>
	ParmW	hplcbte
	ParmD	fc
; /* P N  F R O M  P L C B T E	F C */
; /* looks up fc in plcbte. Does simply: PInPlc(plc, Iplc(plc, fc))->pn.
; */
; native PN PnFromPlcbteFc(hplcbte, fc)
; struct PLCBTE **hplcbte;
; FC fc;
; {
;	struct BTE bte;
	localW	bte
cBegin
	mov	bx,[hplcbte]
	mov	ax,[OFF_fc]
	mov	dx,[SEG_fc]
	call	LN_PnFromPlcbteFc
; }
cEnd


;	LN_PnFromPlcbteFc takes hplcbte in bx, fc in dx:ax.
;	The result is returned in ax.  ax, bx, cx, dx, si, di are altered.
; %%Function:LN_PnFromPlcbteFc %%Owner:BRADV
PUBLIC LN_PnFromPlcbteFc
LN_PnFromPlcbteFc:

;	Assert(IMacPlc(hplcbte) > 0);
;	Assert(fc >= CpPlc(hplcbte, 0) &&
;		fc < CpPlc(hplcbte, IMacPlc(hplcbte)));

ifdef DEBUG
	push ax
	push bx
	push cx
	push dx
	push si
	mov si,bx
	push dx     ;save SEG_fc
	push ax     ;save OFF_fc
	cCall IMacPlc,<si>
	or ax,ax
	jg L136
	mov ax,midFetchn2
	mov bx,136
	cCall AssertProcForNative,<ax,bx>
L136:
	xor ax,ax
	cCall CpPlc,<si,ax>
	pop bx	    ;restore OFF_fc
	pop cx	    ;restore SEG_fc
	push cx     ;save SEG_fc
	push bx     ;save OFF_fc
	sub bx,ax
	sbb cx,dx
	jl L138

	cCall IMacPlc,<si>
	cCall CpPlc,<si,ax>
	pop bx	    ;restore OFF_fc
	pop cx	    ;restore SEG_fc
	sub bx,ax
	sbb cx,dx
	jl L137

L138:	mov ax,midFetchn2
	mov bx,137
	cCall AssertProcForNative,<ax,bx>
L137:
	pop si
	pop dx
	pop cx
	pop bx
	pop ax
endif

;	GetPlc(hplcbte, IInPlc(hplcbte, fc), &bte);
;	return (bte.pn);

;	***Begin in line IInPlc
	mov	bx,[bx]
;	LN_LprgcpForPlc takes pplc in register bx,
;	and returns the result in registers es:di.
;	bx is not altered, ax, cx, dx are trashed.
    	push ax
	push dx
	cCall	LN_LprgcpForPlc,<>
	pop dx
	pop ax
;	 return IcpInRgcpAdjusted(lprgcp, pplc, cp);
;	LN_IcpInRgcpAdjusted expects lprgcp in es:di, pplc in bx, cp in dx:ax
;	bx and di are not altered.
	cCall	LN_IcpInRgcpAdjusted,<>
;	***End in line IInPlc

;	***Begin in line GetPlc
	mov si,[bx.iMaxPlc]
	shl si,1
	shl si,1
	shl ax,1
	add si,ax
	add si,di
;	bte now in es:[si]
;	***End in line GetPlc

	mov ax,es:[si]

	ret


;-------------------------------------------------------------------------
;	FInCa(doc,cp,pca)
;-------------------------------------------------------------------------
; %%Function:FInCa %%Owner:BRADV
cProc	FInCa,<PUBLIC,FAR,ATOMIC>,<si,di>
	ParmW	doc
	ParmD	cp
	ParmW	pca
;
; /* F	I N  C A */
; /* returns true iff doc,cp is in the cache ca */
; native int FInCa(doc, cp, pca)
; int doc; CP cp; struct CA *pca;
; {
cBegin
;     /* Mac ran into a native code generation bug here */
;	  return (pca->doc == doc && pca->cpFirst <= cp &&
;		  cp < pca->cpLim);

	mov si,[pca]
	mov di,[doc]
	mov cx,[OFF_cp]
	mov dx,[SEG_cp]
	;LN_FInCa takes a doc in di, a cp in dx:cx and a pca in di and
	;returns the result in bx.  ax, bx, cx and dx are altered.
	call LN_FInCa
	xchg ax,bx

; }
cEnd


; %%Function:LN_FInCa %%Owner:BRADV
PUBLIC LN_FInCa
LN_FInCa:
	errnz <fFalse>
	xor bx,bx
	errnz <(cpFirstCa) - 0>
	lodsw
	cmp cx,ax
	lodsw
	push dx
	sbb dx,ax
	pop dx
	jl LN_FIC01
	errnz <(cpLimCa) - 4>
	lodsw
	sub cx,ax
	lodsw
	sbb dx,ax
	jge LN_FIC01
	errnz <(docCa) - 8>
	lodsw
	cmp ax,di
	jne LN_FIC01
	errnz <fTrue - 1>
	inc bx
LN_FIC01:
	ret


;-------------------------------------------------------------------------
;	CachePara(doc, cp)					GregC
;-------------------------------------------------------------------------
; %%Function:N_CachePara %%Owner:BRADV
cProc	N_CachePara,<PUBLIC,FAR>,<si,di>
	ParmW	doc
	ParmD	cp
; /* C A C H E	P A R A */
; /* returns para props (style + papx in file + prm in piece table) in
;     vpapFetch
; and paragraph boundaries in
;     caPara
; */
; native CachePara(doc, cp)
; int doc;
; CP cp;
; {
;	int ipcdBase, ipcd,  prm;
	localW	<ipcdBase, ipcd, prm>
;	CP cpGuess;
	localD	cpGuess
;	struct DOD *pdod;
	localW	pdod
;	struct PLCPCD **hplcpcd;
	localW hplcpcd
;	struct PCD pcd;

	localW	fn
	localD	fcMin
	localD	fc
	localD	fcMac
	localD	fcFirst
	localW	pfcb
	localD	fcFirstPage
	localD	fcLim
	localD	cpMinVar
	localD	cpLim
	localW	bpapx
	localW	fPaphInval
	localW	sbHpfkp
	localW	nFib
	localV	pheVar,cbPheMin

cBegin
;     if (FInCa(doc, cp, &caPara)) return;
	mov	si,dataOffset [caPara]
	mov	di,[doc]
	mov	dx,[SEG_cp]
	mov	cx,[OFF_cp]
	;LN_FInCa takes a doc in di, a cp in dx:cx and a pca in di and
	;returns the result in bx.  ax, bx, cx and dx are altered.
	call	LN_FInCa
	or	bx,bx
	jz	CP007
CP003:
cEnd

CP007:
ifdef PROFILE
; %%Function:N_CacheParaCore %%Owner:BRADV
PUBLIC N_CacheParaCore
N_CacheParaCore:
	cCall	StartNMeas,<>
endif ;PROFILE

;
;     FreezeHp();
ifdef	DEBUG
	call	LN_FreezeHp
endif ;DEBUG
;     pdod = *mpdochdod[doc];
;	PdodOrNilFromDoc takes a doc in bx, and returns hNil in ax and bx
;	if mpdochdod[doc] == hNil.  Otherwise it returns doc in ax
;	and pdod = *mpdochdod[doc] in bx.  The zero flag is set according
;	to the value of bx upon return.  Only ax and bx are altered.
	mov bx,[doc]
	call	LN_PdodOrNilFromDoc
ifdef DEBUG
	jnz	CP01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFetchn2
	mov	bx,601
	cCall	AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
CP01:					; 601 = doc has null handle
endif ;DEBUG
	mov	[pdod],bx

; bx = pdod

;	Assert(cp >= cp0 && cp < CpMac2Doc(doc));

ifdef	DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	or	dx,dx
	js	CP02
	cCall	CpMac2Doc,<[doc]>
	mov	bx,[OFF_cp]
	mov	cx,[SEG_cp]
	sub	bx,ax
	sbb	cx,dx
	jl	CP03
CP02:
	mov	bx,600			; 600 = cp out of range
	mov	ax,midFetchn2
	cCall	AssertProcForNative,<ax, bx>
CP03:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

;	hplcpcd = pdod->hplcpcd;
;	ipcdBase = IInPlc(hplcpcd, cpGuess = cp);
; bx = pdod
;	***Begin in line IInPlc
	mov	bx,[bx.hplcpcdDod]
	mov	[hplcpcd],bx
	mov	bx,[bx]
;	LN_LprgcpForPlc takes pplc in register bx,
;	and returns the result in registers es:di.
;	bx is not altered, ax, cx, dx are trashed.
	cCall	LN_LprgcpForPlc,<>
;	 return IcpInRgcpAdjusted(lprgcp, pplc, cp);
;}
;	LN_IcpInRgcpAdjusted expects lprgcp in es:di, pplc in bx, cp in dx:ax
;	bx and di are not altered.
	mov	dx,[SEG_cp]
	mov	[SEG_cpGuess],dx
	mov	ax,[OFF_cp]
	mov	[OFF_cpGuess],ax
	cCall	LN_IcpInRgcpAdjusted,<>
;	***End in line IInPlc
	mov	[ipcdBase],ax

;     if (caPara.doc == doc && cp == caPara.cpLim)

	mov	ax,[doc]
	cmp	ax,[caPara.docCa]
	jne	CP04
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	cmp	ax,[caPara.LO_cpLimCa]
	jne	CP04
	cmp	dx,[caPara.HI_cpLimCa]
	jne	CP04
;	  /* looking for the beginning of the paragraph following
;	  the one in the cache */
;	  caPara.cpFirst = cp;
	mov	[caPara.LO_cpFirstCa],ax
	mov	[caPara.HI_cpFirstCa],dx
	jmp	CP19
;     else
;	  {

CP04:
; /* Search backward to find para start.
; We ask each piece in turn: do you have a para end in the interval (cpMin, cpGuess)?
; with cpGuess initially = cp, and = cpMin of the next piece for the others.
; */
;		for (ipcd = ipcdBase; ; --ipcd)
;			{ /* Beware heap movement! */

	mov	cx,[ipcdBase]
;	CP40 puts the address of the ipcd'th pcd of hplcpcd in es:si,
;	the address of the ipcd'th cp of hplcpcd in es:di
;	and *hplcpcd in bx.  ipcd is passed in cx
;	ax, bx, dx, si, di are altered
	call	CP40

CP05:
;			cpMin = CpPlc(hplcpcd, ipcd);
;			GetPlc(hplcpcd, ipcd, &pcd);
;	fn is set at CP09 in the assembler version
;			fn = pcd.fn;
;	CP44 assumes qcp in es:di pplc in bx, ipcd in cx.
;	It returns CpFromPlc in dx:ax.	Only dx:ax are altered.
	call	CP44
;	dx:ax = cpMin, bx = *hplcpcd, cx = ipcd, es:si = qpcd, es:di = qcp

;	      if (!pcd.fNoParaLast)
	test	es:[si.fNoParaLastPcd],maskFNoParaLastPcd
	jz	CP09

;	This loop end code is performed below in the C source
CP06:
; LNoParaEnd1:
; /* Now we know there's no para end from cpMin to cpGuess.
; If original piece, there may be one after cp == cpGuess */
;		if (ipcd != ipcdBase)
	cmp	cx,[ipcdBase]
	je	CP07
;		  {
;		  /* not original piece, save knowledge */
;			Debug(pcd.fNoParaLastValid = fTrue);
;			pcd.fNoParaLast = fTrue;
;			PutPlc(hplcpcd, ipcd, &pcd);
ifdef DEBUG
	or	es:[si.fNoParaLastValidPcd],maskFNoParaLastValidPcd
endif
	or	es:[si.fNoParaLastPcd],maskFNoParaLastPcd
;		  }
CP07:
;	      if (cpMin == cp0)
;		  { /* Beginning of doc is beginning of para */
;		  caPara.cpFirst = cp0;
;		  break;
;		  }
	push	ax
	or	ax,dx
	pop	ax
	jz	CP08

;	      cpGuess = cpMin;
	mov	[OFF_cpGuess],ax
	mov	[SEG_cpGuess],dx

;	      }
	dec	cx
	sub	si,cbPcdMin
	sub	di,4
	jmp	short CP05
;	  }

CP08:
;		  { /* Beginning of doc is beginning of para */
;		  caPara.cpFirst = cp0;
	mov	[caPara.LO_cpFirstCa],ax
	mov	[caPara.HI_cpFirstCa],ax
;		  break;
	jmp	CP19
;		  }
CP09:
;	dx:ax = cpMin, bx = *hplcpcd, cx = ipcd, es:si = qpcd, es:di = qcp
	mov	[ipcd],cx
	mov	[OFF_cpMinVar],ax
	mov	[SEG_cpMinVar],dx

;	CP51 performs
;	fn = (PCD *)(es:si)->fn, fcMin = (PCD *)(es:si)->fc;
;	Upon return cx:bx = fcMin.
;	Only bx, cx are altered.
	call	CP51

;		  { /* Don't check if we know there's no para end */

;	      fcMin = pcd.fc;	    ;Done in CP51
;
;		  fcFirst = fc = fcMin + cpGuess - cpMin;
;	dx:ax = cpMin, cx:bx = fcMin
	sub	ax,[OFF_cpGuess]
	sbb	dx,[SEG_cpGuess]
	sub	bx,ax
	sbb	cx,dx
	mov	[OFF_fc],bx
	mov	[SEG_fc],cx
	mov	[OFF_fcFirst],bx
	mov	[SEG_fcFirst],cx
; /* calculate the fcFirst after the latest para end before fc.
; If there is no Eop at fc' in [fcMin, fc), goto NoParaEnd1. */
;		  if (cpGuess == cpMin)
;		      goto LNoParaEnd1;
	or	ax,dx
	je	Ltemp003

;		  if (fn == fnSpec/*fnInsert*/)
	cmp	[fn],fnSpec
	jne	CP11
;		      {
;		      if ((int)fcMin == fcSpecEop)
	cmp	[OFF_fcMin],fcSpecEop
	jne	CP10

;			{
;			Assert( CpPlc( hplcpcd, ipcd + 1 ) == cpMin + ccpEop );
ifdef DEBUG
;	/* Assert (CpPlc(hplcpcd, ipcd + 1) == cpMin + ccpEop) with a call
;	so as not to mess up short jumps */
	call	CP63
endif ;/* DEBUG */

;#ifdef CRLF
;			if (fc > fcSpecEop + 1)
;#endif
;			goto LParaStart;
	cmp	bx,fcSpecEop+1
	jle	Ltemp003
	jmp	LParaStart

;			}
CP10:
;		      goto LNoParaEnd1;
Ltemp003:
	jmp	LNoParaEnd1
;		      }
;
CP11:
;		  pfcb = *(mpfnhfcb[fn]);
; /* note that the scratch file FCB has the fHasFib flag on as an expedient lie
;    which allows us to recognize that the scratch file has PLCBTEs also. */
;		  if (!pfcb->fHasFib)
;	CP46 tests (pfcb = *mpfnhfcb[fn])->fHasFib.
;	bx is set to pfcb upon return.
;	Only bx is altered.
	call	CP46
	jne	CP15

;		      { /* Unformatted file; scan for an EOL */
;		      PN pn;
;		      FC fcFirstPage;
;
;		      fcFirstPage = ((fc - 1) >> shftSector) << shftSector;
;		      pn = fcFirstPage >> shftSector;
	mov	ax,-1
	cwd
	add	ax,[OFF_fc]
	adc	dx,[SEG_fc]
;	CP47 performs
;	fcFirstPage = ((dx:ax) >> shftSector) << shftSector;
;	dx = fcFirstPage >> shftSector
;	Only ax and dx are altered.
	call	CP47
	mov	ax,[OFF_fc]
	mov	cx,[SEG_fc]
	jmp	short CP13

CP12:
	mov	ax,[OFF_fcFirstPage]
	mov	cx,[SEG_fcFirstPage]
	mov	[OFF_fc],ax
	mov	[SEG_fc],cx

;			  fcFirstPage -= cbSector;
	sub	[OFF_fcFirstPage],cbSector
	sbb	[SEG_fcFirstPage],0


;		      while (fc > fcMin)
CP13:
	mov	bx,[OFF_fcMin]
	sub	bx,ax
	mov	bx,[SEG_fcMin]
	sbb	bx,cx
	jge	Ltemp003

;			  {
;			  char HUGE *hpch, far *qch;
;
;			  hpch = HpchGetPn(fn, pn--) + (int)(fc - fcFirstPage);
;			  qch = LpFromHp(hpch);
;	CP48 performs hpch = HpchGetPn(fn, dx) + (int)(fc - fcFirstPage),
;	qch = LpFromHp(hpch), leaving qch in es:di and
;	qch - (fc - fcFirstPage) in es:si.
;	it also puts [OFF_fcFirstPage] into bx.
;	dx is not altered.
	call	CP48
	dec	dx

;			  if (fcMin >= fcFirstPage)
	mov	ax,[OFF_fcMin]
	mov	cx,[SEG_fcMin]
	cmp	ax,bx
	sbb	cx,[SEG_fcFirstPage]
	jl	CP14
;			      fcFirstPage = fcMin;
	mov	[OFF_fcFirstPage],ax
CP14:
;			  while (fc > fcFirstPage)
	mov	cx,[OFF_fc]
	sub	cx,[OFF_fcFirstPage]
	dec	di	    ;adjust for post decrement
	mov	al,chEol

;			      {
;			      if (*(--qch) == chEol)
	std
	repne	scasb
	cld
	jne	CP12
	inc	di	    ;adjust for post decrement
	inc	di	    ;fc points after chEol
	sub	di,si	    ;compute pch - pchInit;
	add	di,bx	    ;add old OFF_fcFirstPage;
	mov	[OFF_fc],di

;				  {
;				  fcFirst = fc;
	mov	cx,[SEG_fcFirstPage]
	mov	[OFF_fcFirst],di
	mov	[SEG_fcFirst],cx
;					Debug(pcd.fNoParaLastValid = fTrue);
;					PutPlcLast(hplcpcd, ipcd, &pcd);
ifdef DEBUG
;	NOTE: in the assembler version we only have to call PutPlc
;	if we have DEBUG defined
;	Do this DEBUG stuff with a call so as not to mess up short jumps.
	call	CP41
endif ;DEBUG
;				  goto LParaStart;
Ltemp004:
	jmp	short LParaStart
;				  }
;			      fc--;
;			      }
;	The following line is done at CP12 in the assembler version
;			  fcFirstPage -= cbSector;
;			  }
;		      }
;		  else

CP15:
;					{ /* Formatted file; get info from para run */
;					int pn;
;					int ifc, ifcT;
;					FC far *qfc;
;					struct FKP HUGE *hpfkp;
;					struct FKP far *qfkp;
;					FC fcLim, fcFkpLim;
;					int dpn;
;					struct PLCBTE **hplcbte = pfcb->hplcbtePap;
	mov	bx,[bx.hplcbtePapFcb]

; /* this if takes care of the case if fc is beyond the last Eop in the file.
;    the case occurs when writing FKPs to the scratch file and during quicksave. */
;					if (fc >= (fcFkpLim = (FC) CpMacPlc(hplcbte)))
;						{
;/* para bound at lim fc of the plcbte. */
;						if (fcMin < (fcFirst = fcFkpLim))
;							goto LParaStart;
;						goto LNoParaEnd1;
;						}
;	CP42 puts CpMacPlc of the hplc passed in bx into dx:ax
;	bx is not altered
	call	CP42
	mov	cx,[OFF_fc]
	sub	cx,ax
	mov	cx,[SEG_fc]
	sbb	cx,dx
	jl	CP16
	mov	[OFF_fcFirst],ax
	mov	[SEG_fcFirst],dx
	mov	bx,[OFF_fcMin]
	mov	cx,[SEG_fcMin]
	sub	bx,ax
	sbb	cx,dx
	jl	LParaStart
	jmp	short LNoParaEnd1
CP16:
;					Debug(vcFetchPap1++);
;					hpfkp = (struct FKP *) HpchGetPn(fn,
;						pn = PnFromPlcbteFc(hplcbte, fc));
;					qfkp = LpFromHp(hpfkp);
;					ifc = pfcb->ifcPapHint1;
;	CP49 takes hplcbte in bx.  It performs
;	hpfkp = (struct FKP *) HpchGetPn(fn, pn = PnFromPlcbteFc(hplcbte, fc)),
;	qfkp = LpFromHp(hpfkp), and returns qfkp in es:di, pn in dx, pfcb in si.
;	It returns qfkp in es:di, pn in dx, pfcb in si, and sbHpfkp in memory.
;	ax, bx, cx, dx, si, di, es are altered.
	call	CP49
	mov	bx,[si.ifcPapHint1Fcb]

;					if (pn == pfcb->pnPapHint1 &&
;						((ifc > 1 && *(pfc = &pfkp->rgfc[ifcT = ifc - 1]) <= fc) &&
;						fc < *(pfc + 1)) ||
;						((*(pfc = &pfkp->rgfc[ifcT = ifc]) <= fc) && fc < *(pfc + 1)))
	; bx = ifc, dx = pn, es:di = qfkp, si = pfcb
	cmp	dx,[si.pnPapHint1Fcb]
	jne	CP17
;	if (ax == 1) upon entry then CP52 returns
;	   ifc != ifcFkpNil &&
;	   ((ifc > 1 && *(qfc = &qfkp->rgfc[ifcT = ifc - 1]) <= fc) &&
;	   fc < *(qfc + 1)) ||
;	   ((*(qfc = &qfkp->rgfc[ifcT = ifc]) <= fc) && fc < *(qfc + 1)))
;	if (ax == -1) upon entry then CP52 returns
;	   ifc != ifcFkpNil &&
;	   ((ifc < qfkp->crun - 1 && *(qfc = &qfkp->rgfc[ifcT = ifc + 1]) <= fc) &&
;	   fc < *(qfc + 1)) ||
;	   ((*(qfc = &qfkp->rgfc[ifcT = ifc]) <= fc) && fc < *(qfc + 1)))
;	True is indicated by zero flag reset, and false by zero flag set.
;	CP52 assumes bx = ifc, es:di = qfkp upon entry.
;	Upon return bx = ifcT, es:si = qfc, es:di = qfkp.
;	ax, bx, cx, si are altered.
	mov	ax,00001h
	call	CP52
	je	CP17

;						{
;						fcFirst = *qfc;
;#ifdef DEBUG
;						if (ifc == ifcT)
;							vcPap1Same++;
;						else if (ifc - 1 == ifcT)
;							vcPap1Minus1++;
;						else Assert(fFalse);
;#endif /* DEBUG */
;						pfcb->ifcPapHint1 = ifcT;

	; es:si = qfc, bx = ifcT

	push	es
	pop	ds
	push	ss
	pop	es
	lea	di,[fcFirst]
	movsw
	movsw
	push	ss
	pop	ds
	mov	di,[pfcb]
	mov	[di.ifcPapHint1Fcb],bx
	jmp	short CP18
;						}
;					else
;						{
;						BFromFc(hpfkp, fc, &fcFirst, &fcLim/*not used*/, &pfcb->ifcPapHint1);
;						pfcb->pnPapHint1 = pn;
;						}

	; es:di = qfkp, dx = pn;
CP17:
;	CP43 performs
;	ax = BFromFc(es:di, fc, &fcFirst, &fcLim, &ifc);
;	where ifc is returned in cx.
;	ax, bx, cx, si, and di are altered.
	call	CP43
	mov	bx,[pfcb]
	mov	[bx.ifcPapHint1Fcb],cx
	mov	[bx.pnPapHint1Fcb],dx
CP18:

;		      if (fcMin >= fcFirst) goto LNoParaEnd1;
	mov	ax,[OFF_fcMin]
	mov	dx,[SEG_fcMin]
	sub	ax,[OFF_fcFirst]
	sbb	dx,[SEG_fcFirst]
	jge	LNoParaEnd1

; /* Found para start */
; LParaStart:		      caPara.cpFirst = cpMin + (fcFirst - fcMin);
LParaStart:
	mov	ax,[OFF_cpMinVar]
	mov	dx,[SEG_cpMinVar]
	add	ax,[OFF_fcFirst]
	adc	dx,[SEG_fcFirst]
	sub	ax,[OFF_fcMin]
	sbb	dx,[SEG_fcMin]
	mov	[caPara.LO_cpFirstCa],ax
	mov	[caPara.HI_cpFirstCa],dx
;			Debug(pcd.fNoParaLastValid = fTrue);
;			PutPlc(hplcpcd, ipcd, &pcd);
ifdef DEBUG
;	NOTE: in the assembler version we only have to call PutPlc
;	if we have DEBUG defined
;	Do this DEBUG stuff with a call so as not to mess up short jumps.
	call	CP41
endif ;DEBUG
;		      break;
	jmp	short CP19
;		      }
;		  }
; LNoParaEnd1:
LNoParaEnd1:
	mov	cx,[ipcd]
;	CP40 puts the address of the ipcd'th pcd of hplcpcd in es:si,
;	the address of the ipcd'th cp of hplcpcd in es:di
;	and *hplcpcd in bx.  ipcd is passed in cx
;	ax, bx, dx, si, di are altered
	call	CP40
;	CP44 assumes qcp in es:di pplc in bx, ipcd in cx.
;	It returns CpFromPlc in dx:ax.	Only dx:ax are altered.
	call	CP44
	jmp	CP06

;	This loop end code is performed above in the assembler version.
; /* Now we know there's no para end from cpMin to cpGuess.
; If original piece, there may be one after cp == cpGuess */
;		if (ipcd != ipcdBase)
;		  {
;		  /* not original piece, save knowledge */
;			Debug(pcd.fNoParaLastValid = fTrue);
;			pcd.fNoParaLast = fTrue;
;			PutPlc(hplcpcd, ipcd, &pcd);
;		  }
;	      if (cpMin == cp0)
;		  { /* Beginning of doc is beginning of para */
;		  caPara.cpFirst = cp0;
;		  break;
;		  }
;	      cpGuess = cpMin;
;	      }
;	  }

CP19:
;
;     caPara.doc = doc;
	mov	ax,[doc]
	mov	[caPara.docCa],ax
; /* Now go forward to find the cpLim of the para */
;     cpGuess = cp;
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	mov	[OFF_cpGuess],ax
	mov	[SEG_cpGuess],dx

;	for (ipcd = ipcdBase; ; ++ipcd)
	mov	cx,[ipcdBase]
	inc	cx	;use ipcd + 1 in this loop
;	CP40 puts the address of the ipcd'th pcd of hplcpcd in es:si,
;	the address of the ipcd'th cp of hplcpcd in es:di
;	and *hplcpcd in bx.  ipcd is passed in cx
;	ax, bx, dx, si, di are altered
	call	CP40

;		{
;	cpMin is set at CP225 in the assembler version
;		cpMin = CpPlc(hplcpcd, ipcd);
;		cpLim = CpPlc(hplcpcd, ipcd + 1);
;		GetPlc(hplcpcd, ipcd, &pcd);
;	fn is set at CP225 in the assembler version
;		fn = pcd.fn;
CP20:
;	CP44 assumes qcp in es:di pplc in bx, ipcd in cx.
;	It returns CpFromPlc in dx:ax.	Only dx:ax are altered.
	call	CP44

;	dx:ax = cpMin, bx = *hplcpcd, cx = ipcd+1, es:si = qpcd, es:di = qcp
;		if (!pcd.fNoParaLast)
;
	test	es:[si.fNoParaLastPcd-cbPcdMin],maskFNoParaLastPcd
	jz	CP225
CP21:
; LNoParaEnd2:
; /* Now we know there's no para end. */
;	  if (ipcd != ipcdBase)
	dec	cx	;ipcd + 1 ==> ipcd
	cmp	cx,[ipcdBase]
	je	CP22
;	      {
;	      /* not original piece, save knowledge */
;	      Debug(pcd.fNoParaLastValid = fTrue);
;	      pcd.fNoParaLast = fTrue;
;	      PutPlc(hplcpcd, ipcd, &pcd);
ifdef DEBUG
	or	es:[si.fNoParaLastValidPcd-cbPcdMin],maskFNoParaLastValidPcd
endif
	or	es:[si.fNoParaLastPcd-cbPcdMin],maskFNoParaLastPcd
;	      }
CP22:
	inc	cx	;restore ipcd + 1;
;	  cpGuess = cpLim;
	mov	[OFF_cpGuess],ax
	mov	[SEG_cpGuess],dx
;	  }
	inc	cx
	add	si,cbPcdMin
	add	di,4
	jmp	short CP20
;	      { /* Don't check if we know there's no para end */
CP225:
;	dx:ax = cpLim, bx = *hplcpcd, cx = ipcd+1, es:si = qpcd, es:di = qcp
	dec	cx
	mov	[ipcd],cx
	mov	[OFF_cpLim],ax
	mov	[SEG_cpLim],dx
	sub	si,cbPcdMin
	sub	di,4
;	CP44 assumes qcp in es:di pplc in bx, ipcd in cx.
;	It returns CpFromPlc in dx:ax.	Only dx:ax are altered.
	call	CP44
	mov	[OFF_cpMinVar],ax
	mov	[SEG_cpMinVar],dx
	mov	cx,es:[si.prmPcd]
	mov	[prm],cx
;	CP51 performs
;	fn = (PCD *)(es:si)->fn, fcMin = (PCD *)(es:si)->fc;
;	Upon return cx:bx = fcMin.
;	Only bx, cx are altered.
	call	CP51

;	      fcMin = pcd.fc;	    ;Done in CP51

;			if (fn == fnSpec && (int)fcMin == fcSpecEop)
;				{ /* special eop piece */
	cmp	[fn],fnSpec
	jne	CP24
	cmp	bx,fcSpecEop
	jne	CP24
;				caPara.cpLim = cpMin + ccpEop;
	mov	ax,ccpEop
	cwd
	add	ax,[OFF_cpMinVar]
	adc	dx,[SEG_cpMinVar]
	mov	[caPara.LO_cpLimCa],ax
	mov	[caPara.HI_cpLimCa],dx
;				if (doc != vdocPapLast || fn != vfnPapLast ||
;					vbpapxPapLast != cbSector ||
;					vstcLast == stNil)
;					{
;	CP605 tests:
;	if (doc = vdocPapLast && fn == vfnPapLast
;	    && vbpapxPapLast == cbSector)
;	ax is altered.
	call	CP605
	jne	CP227
	cmp	[vstcLast],stcNil
	jne	CP23
CP227:

;					if (doc != vdocPapLast ||
;						vstcLast != stcNormal ||
;						vcchPapxLast > 0)
;						{
;						MapStc(pdod, stcNormal, &vchpStc, &vpapStc);
;						vstcLast = stcNormal;
;						vcchPapxLast = 0;
;						}
;					Debug(else vcStyleUnchanged++);
;	CP495 performs:
;	if (doc != vdocPapLast ||
;		vstcLast != stcNormal ||
;		vcchPapxLast > 0)
;		{
;		vcchPapxLast = 0;
;		vstcLast = stcNormal;
;		MapStc(pdod, stcNormal, &vchpStc, &vpapStc);
;		}
;	Debug(else vcStyleUnchanged++);
;	ax, bx, cx, dx, si, di are altered.
	call	CP495

;					SetWords(&vpapStc.phe, 0, cwPHE);
;	CP508 performs SetWords(&vpapStc.phe, 0, cwPHE);
;	ax, di, and es are altered.
	call	CP508

;					vdocPapLast = doc;
;					vfnPapLast = fn;
;					vbpapxPapLast = cbSector;
;	CP58 performs:
;	vdocPapLast = doc;
;	vfnPapLast = fn;
;	vbPapxPapLast = (carry flag set upon entry ? cbSector : bpapx);
;	if (carry flag clear upon entry) vpnPapLast = dx;
;	ax is altered.
	stc
	call	CP58

;					}

;				Debug(else vcPapStcUnchanged++);
;				Debug(pcd.fNoParaLastValid = fTrue);
;				PutPlcLast(hplcpcd, ipcd, &pcd);

CP23:
ifdef DEBUG
;	NOTE: in the assembler version we only have to call PutPlc
;	if we have DEBUG defined
;	Do this DEBUG stuff with a call so as not to mess up short jumps.
	call	CP41
endif ;DEBUG
;		  break;
	jmp	CP37
;		  }
CP24:
;	      fc = fcMin + cpGuess - cpMin;
;	dx:ax = cpMin, cx:bx = fcMin
	sub	bx,ax
	sbb	cx,dx
	mov	ax,bx
	mov	dx,cx
	add	ax,[OFF_cpGuess]
	adc	dx,[SEG_cpGuess]
	mov	[OFF_fc],ax
	mov	[SEG_fc],dx
;	      fcMac = fcMin + cpLim - cpMin;
	add	bx,[OFF_cpLim]
	adc	cx,[SEG_cpLim]
	mov	[OFF_fcMac],bx
	mov	[SEG_fcMac],cx
; /* Calculate the fcLim after the first para end after or at fc.  If there
; is no para end in [fc, fcMac), goto LNoParaEnd2.  Also get paragraph
; properties in vpapFetch.  Leave normal chp's in vchpStc. */
;	      if (fn == fnInsert)
;		  goto LNoParaEnd2;
	cmp	[fn],fnInsert
	jne	Ltemp005
Ltemp006:
	jmp	LNoParaEnd2
Ltemp005:

;
;	      pfcb = *mpfnhfcb[fn];
; /* note that the scratch file FCB has the fHasFib flag on as an expedient lie
;    which allows us to recognize that the scratch file has PLCBTEs also. */
;	      if (!pfcb2->fHasFib)
;	CP46 tests (pfcb = *mpfnhfcb[fn])->fHasFib.
;	bx is set to pfcb upon return.
;	Only bx is altered.
	call	CP46
	je	Ltemp002
	jmp	CP29
Ltemp002:

;		  { /* Unformatted file; scan for EOL */
;				if (vdocPapLast != doc || vfnPapLast != fn ||
;					vbpapxPapLast != cbSector ||
;					vstcLast == stcNil)
;					{
;	CP605 tests:
;	if (doc = vdocPapLast && fn == vfnPapLast
;	    && vbpapxPapLast == cbSector)
;	ax is altered.
	call	CP605
	jne	CP245
	cmp	[vstcLast],stcNil
	jne	CP25
CP245:

;					if (doc != vdocPapLast ||
;						vstcLast != stcNormal ||
;						 vcchPapxLast > 0)
;						{
;						MapStc(pdod, stcNormal, &vchpStc, &vpapStc);
;						vstcLast = stcNormal;
;						vcchPapxLast = 0;
;						}
;					Debug(else vcStyleUnchanged++);
;	CP495 performs:
;	if (doc != vdocPapLast ||
;		vstcLast != stcNormal ||
;		vcchPapxLast > 0)
;		{
;		vcchPapxLast = 0;
;		vstcLast = stcNormal;
;		MapStc(pdod, stcNormal, &vchpStc, &vpapStc);
;		}
;	Debug(else vcStyleUnchanged++);
;	ax, bx, cx, dx, si, di are altered.
	call	CP495

;					SetWords(&vpapStc.phe, 0, cwPHE);
;	CP508 performs SetWords(&vpapStc.phe, 0, cwPHE);
;	ax, di, and es are altered.
	call	CP508

;					vdocPapLast = doc;
;					vfnPapLast = fn;
;					vbpapxPapLast = cbSector;
;					}
;				Debug(else vcPapStcUnchanged++);
;	CP58 performs:
;	vdocPapLast = doc;
;	vfnPapLast = fn;
;	vbPapxPapLast = (carry flag set upon entry ? cbSector : bpapx);
;	if (carry flag clear upon entry) vpnPapLast = dx;
;	ax is altered.
	stc
	call	CP58
CP25:

;		  fcFirstPage = (fc >> shftSector) << shftSector;
;		  pn = fcFirstPage >> shftSector;
	mov	ax,[OFF_fc]
	mov	dx,[SEG_fc]
;	CP47 performs
;	fcFirstPage = ((dx:ax) >> shftSector) << shftSector;
;	dx = fcFirstPage >> shftSector
;	Only ax and dx are altered.
	call	CP47
	mov	ax,[OFF_fc]
	mov	cx,[SEG_fc]
	jmp	short CP27

CP26:
	mov	ax,[OFF_fcFirstPage]
	mov	cx,[SEG_fcFirstPage]
	mov	[OFF_fc],ax
	mov	[SEG_fc],cx

;		  while (fc < fcMac)
CP27:
	sub	ax,[OFF_fcMac]
	sbb	cx,[SEG_fcMac]
	jge	Ltemp006
;		      {
;		      char far *qch;
;
;		      qch = (char far *) LpFromHp(HpchGetPn(fn, pn++)) + (int)(fc - fcFirstPage);
;	CP48 performs hpch = HpchGetPn(fn, dx) + (int)(fc - fcFirstPage),
;	qch = LpFromHp(hpch), leaving qch in es:di and
;	qch - (fc - fcFirstPage) in es:si.
;	it also puts [OFF_fcFirstPage] into bx.
;	dx is not altered.
	call	CP48
	inc	dx

;		      if ((fcFirstPage += cbSector) > fcMac)
	add	[OFF_fcFirstPage],cbSector
	adc	[SEG_fcFirstPage],0
	mov	ax,[OFF_fcMac]
	mov	cx,[SEG_fcMac]
	cmp	ax,[OFF_fcFirstPage]
	sbb	cx,[SEG_fcFirstPage]
	jge	CP28
;			  fcFirstPage = fcMac;
	mov	[OFF_fcFirstPage],ax
CP28:
;		      while (fc < fcFirstPage)
	mov	cx,[OFF_fcFirstPage]
	sub	cx,[OFF_fc]
	mov	al,chEol

;			  {
;			  fc++;
;			  if (*qch++ == chEol)
	repne	scasb
	jne	CP26
	sub	di,si	    ;compute pch - pchInit
	add	di,bx	    ;add old OFF_fcFirstPage
	adc	[SEG_fc],0
	mov	[OFF_fc],di

;			      {
;			      fcLim = fc;
	mov	ax,[SEG_fc]
	mov	[OFF_fcLim],di
	mov	[SEG_fcLim],ax
;			      Debug(pcd.fNoParaLastValid = fTrue);
;			      PutPlcLast( hplcpcd, ipcd, &pcd  );
ifdef DEBUG
;	NOTE: in the assembler version we only have to call PutPlc
;	if we have DEBUG defined
;	Do this DEBUG stuff with a call so as not to mess up short jumps.
	call	CP41
endif ;DEBUG
;			      goto LParaEnd2;
	jmp	LParaEnd2

;			      }
;			  }
;		      }
;		  }
;	      else
CP29:
; /* Formatted file; get info from para run */
;		  {
;		  int pn;
;		  int ifc, ifcT;
;		  FC far *qfc;
;		  struct FKP HUGE *hpfkp;
;		  struct FKP far *qfkp;
;		  struct FPAP *pfpap;
;		  FC fcFirst;
;		  int bpapx;
;		  int nFib = pfcb->nFib;
;		  int crun;
	mov	ax,[bx.nFibFcb]
	mov	[nFib],ax

;		  hplcbtePap = pfcb->hplcbtePap;
	mov	bx,[bx.hplcbtePapFcb]

;			if (fc >= (FC) CpMacPlc(hplcbtePap))
;	CP42 puts CpMacPlc of the hplc passed in bx into dx:ax
;	bx is not altered
	call	CP42
	mov	cx,[OFF_fc]
	sub	cx,ax
	mov	cx,[SEG_fc]
	sbb	cx,dx

;		      goto LNoParaEnd2;
	jge	Ltemp008

;			Debug(vcFetchPap2++);
;			Debug(pcd.fNoParaLastValid = fTrue);
;			PutPlcLast(hplcpcd, ipcd, &pcd);
ifdef DEBUG
;	NOTE: in the assembler version we only have to call PutPlc
;	if we have DEBUG defined
;	Do this DEBUG stuff with a call so as not to mess up short jumps.
	call	CP41
endif ;DEBUG
;			hpfkp = (struct FKP *)HpchGetPn(fn,
;				pn = PnFromPlcbteFc(hplcbtePap, fc));
;
;			ifc = pfcb->ifcPapHint2;
;	CP49 takes hplcbte in bx.  It performs
;	hpfkp = (struct FKP *) HpchGetPn(fn, pn = PnFromPlcbteFc(hplcbte, fc)),
;	qfkp = LpFromHp(hpfkp), and returns qfkp in es:di, pn in dx, pfcb in si.
;	It returns qfkp in es:di, pn in dx, pfcb in si, and sbHpfkp in memory.
;	ax, bx, cx, dx, si, di, es are altered.
	call	CP49
	mov	bx,[si.ifcPapHint2Fcb]

;			if (pn == pfcb->pnPapHint2 &&
;				((ifc < qfkp->crun && *(qfc = &qfkp->rgfc[ifcT = ifc + 1]) <= fc) &&
;				fc < *(qfc + 1)) ||
;				((*(qfc = &qfkp->rgfc[ifcT = ifc]) <= fc) && fc < *(qfc + 1)))
	; bx = ifc, dx = pn, es:di = qfkp, si = pfcb
	cmp	dx,[si.pnPapHint2Fcb]
	jne	CP30
;	if (ax == 1) upon entry then CP52 returns
;	   ifc != ifcFkpNil &&
;	   ((ifc > 1 && *(qfc = &qfkp->rgfc[ifcT = ifc - 1]) <= fc) &&
;	   fc < *(qfc + 1)) ||
;	   ((*(qfc = &qfkp->rgfc[ifcT = ifc]) <= fc) && fc < *(qfc + 1)))
;	if (ax == -1) upon entry then CP52 returns
;	   ifc != ifcFkpNil &&
;	   ((ifc < qfkp->crun - 1 && *(qfc = &qfkp->rgfc[ifcT = ifc + 1]) <= fc) &&
;	   fc < *(qfc + 1)) ||
;	   ((*(qfc = &qfkp->rgfc[ifcT = ifc]) <= fc) && fc < *(qfc + 1)))
;	True is indicated by zero flag reset, and false by zero flag set.
;	CP52 assumes bx = ifc, es:di = qfkp upon entry.
;	Upon return bx = ifcT, es:si = qfc, es:di = qfkp.
;	ax, bx, cx, si are altered.
	mov	ax,0FFFFh
	call	CP52
	je	CP30

;				{
;				fcFirst = *qfc;
;				fcLim = *(qfc + 1);
;		bpapx = ((char *)&((qfkp->rgfc)[qfkp->crun + 1]))[ifcT];
;				bpapx = bpapx << 1;
;				pfcb->ifcPapHint2 = ifcT;
	; bx = ifcT, si = pfc, es:di = qfkp
	push	es
	pop	ds
	push	ss
	pop	es
	push	di	;save qfkp
	lea	di,[fcFirst]
	movsw
	movsw
	lea	di,[fcLim]
	movsw
	movsw
	mov	di,[pfcb]
	mov	ss:[di.ifcPapHint2Fcb],bx
	pop	si	;restore pfkp
	mov	al,[si.cRunFkp]
	cbw
	inc	ax
	shl	ax,1
	shl	ax,1
	errnz	<(rgfcFkp)>
	add	bx,ax
	xor	ah,ah
	mov	al,[bx+si]
	shl	ax,1
	push	ds
	pop	es
	push	ss
	pop	ds
	mov	[bpapx],ax
	jmp	short CP31
;					}
;				else
;					{
;					bpapx = BFromFc((struct FKP *)pfkp, fc, &fcFirst, &fcLim, &pfcb->ifcPapHint2);
;					pfcb->pnPapHint2 = pn;
CP30:
	; es:di = qfkp, dx = pn;
;	CP43 performs
;	ax = BFromFc(es:di, fc, &fcFirst, &fcLim, &ifc);
;	where ifc is returned in cx.
;	ax, bx, cx, si, and di are altered.
	push	es
	push	di	;save qfkp
	call	CP43	;Note this and subordinate routines consist
			;entirely of near routines that don't allocate
			;so es may be saved and restored here.
	pop	si	;restore qfkp
	pop	es
	mov	bx,[pfcb]
	mov	[bx.ifcPapHint2Fcb],cx
	mov	[bx.pnPapHint2Fcb],dx
	mov	[bpapx],ax

;					}
;
CP31:
	; es:si = pfkp;

;		  if (fcLim <= fcMac)
	mov	ax,[OFF_fcMac]
	sub	ax,[OFF_fcLim]
	mov	ax,[SEG_fcMac]
	sbb	ax,[SEG_fcLim]
	jge	Ltemp007
Ltemp008:
	jmp	LNoParaEnd2
Ltemp007:

;		      {
;		      CHAR HUGE *hpch;
;		      int fPaphInval;
;		      int cch, stc;
;		      if (bpapx != 0/*nil*/)
;			      {
	cmp	[bpapx],0
	je	CP315

;			      hpch = ((CHAR HUGE *)hpfkp) + bpapx;
;			      cch = *hpch++;
	xor	ah,ah
	add	si,[bpapx]
	push	es
	pop	ds
	push	ss
	pop	es
	lodsb
	mov	cx,ax

;			      if (WinMac(nFib >= 25, !fWord3File && nFib >= 9))
;				      cch <<= 1;
	cmp	[nFib],25
	jl	CP313
	shl	cx,1
CP313:

;			      stc = *hpch++;
	lodsb

	;Assembler note: The following two lines have been moved up from below.
;			cbPhe = cbPHE;
;			bltbh(hpch, &phe, cbPHE);
	lea	di,[pheVar]
	errnz	<cbPheMin - 6>
	movsw
	movsw
	movsw		;save paph
	push	ss
	pop	ds

;/* we only need to check for sprmPStcPermute application, if the papx stc is
;   not normal and there is a prm for this piece. */
;			      if (stc != stcNormal && pcd.prm != prmNil)
	cmp	al,stcNormal
	je	CP315
	mov	bx,[prm]
	errnz	<prmNil>
	or	bx,bx
	je	CP315

;				      {
;				      struct PRM *pprm;
;/* sprmPStcPermute can't fit in a non-complex (ie. 2-byte) prm */
;				      if ((pprm = (struct PRM *)&pcd.prm)->fComplex)
;					      {
;					      char *pprl;
;	In the assembler version we get the prm out of the pcd at CP225
	test	bl,maskfComplexPrm
	jz	CP315

;					      struct PRC *pprc = *HprcFromPprmComplex(pprm);
;#define HprcFromPprmComplex(pprm) ((struct PRC **)((pprm)->cfgrPrc<<1))
	errnz <maskcfgrPrcPrm - 0FFFEh>
	and	bl,0FEh
	mov	bx,[bx]

;					      pprl = pprc->grpprl;

;/* if the first sprm is sprmPStcPermute, we interpret it here */
;					      if (*pprl == sprmPStcPermute)
;						      {
	cmp	bptr ([bx.grpprlPrc]),sprmPStcPermute
	jne	CP315

;						      if (stc <= *(pprl + 1))
	cmp	al,bptr ([bx.grpprlPrc+1])
	ja	CP315

;							      stc = *(pprl + 1 + stc);
	add	bx,ax
	mov	al,bptr ([bx.grpprlPrc+1])

;						      }
;					      }
;				      }
CP315:

;			if (vdocPapLast == doc &&
;				vfnPapLast == fn &&
;				vpnPapLast == pn &&
;				vbpapxPapLast == bpapx &&
;				vstcLast != stcNil &&
;				(bpapx == 0 || stc == vstcLast))
;				{
;				Debug(vcPapStcUnchanged++);
;				goto LParaEnd2;
;				}

;	CP60 tests:
;	if (doc = vdocPapLast && fn == vfnPapLast
;	    && vpnPapLast == dx && vbpapxPapLast == bpapx);
;	ax is altered.
	push	ax	;save stc
	call	CP60
	pop	ax	;restore stc
	jne	CP32
	cmp	[vstcLast],stcNil
	je	CP32
	cmp	[bpapx],0
	je	Ltemp020
	cmp	ax,[vstcLast]
	jne	CP32
Ltemp020:
	jmp	LParaEnd2
CP32:
	push	dx	;save pn

;
;		      if (bpapx != 0/*nil*/)
	cmp	[bpapx],0
	jz	CP34
;			{
;			struct PHE phe;
;
;			cbPhe = cbPHE;
;			bltbh(hpch, &phe, cbPHE);
;			if (doc != vdocPapLast ||
;				vstcLast != stc ||
;				vcchPapxLast > 0)
;				{
;				MapStc(pdod, stc, &vchpStc, &vpapStc);
;				SetWords(&vtapStc, 0, cwTAP);
;				vstcLast = stc;
;				}
;			Debug(else vcStyleUnchanged++);
	;Assembler note: bltbh(hpch, &phe, cbPHE) has been moved above.
;	CP50 performs:
;	if (doc != vdocPapLast ||
;		vstcLast != cx ||
;		vcchPapxLast > 0)
;		{
;		vcchPapxLast = 0;
;		vstcLast = cx;
;		SetWords(&vtapStc, 0, cwTAP);
;		MapStc(pdod, cx, &vchpStc, &vpapStc);
;		}
;	Debug(else vcStyleUnchanged++);
;	ax, bx, cx, dx, si, di are altered.
	push	cx	;save cch
	xchg	ax,cx
	push	si	;save loword of qch
	call	CP50
	pop	si	;restore loword of qch

	pop	cx	;restore cch
;			Assert(cch >= cbPhe + 1);
ifdef DEBUG
;	/* Assert (cch >= cbPhe + 1) with a call
;	so as not to mess up short jumps */
	call	CP65
endif ;/* DEBUG */

;			vcchPapxLast = cch - (cbPhe + 1);
	sub	cx,cbPheMin+1
	mov	[vcchPapxLast],cx

;			fPaphInval = vpapStc.phe.fStyleDirty;
	mov	ax,[vpapStc.phePap.fStyleDirtyPhe]
	mov	[fPaphInval],ax

;			vpapStc.phe = phe;
	push	ds
	pop	es
	push	si	;save loword of qch
	lea	si,[pheVar]
	mov	di,dataoffset [vpapStc.phePap]
	errnz	<cbPheMin - 6>
	movsw
	movsw
	movsw
	pop	si	;restore loword of qch

;			if ((cch -= cbPhe + 1) > 0)
;				{
;				ApplyPrlSgc(hpch + cbPhe, cch, &vpapStc, sgcPap);
	or	cx,cx
	jle	CP35
	mov	di,dataOffset [vpapStc]
	errnz	<sgcPap>
	xor	dx,dx
	push	cx  ;save cch
	push	[sbHpfkp]
	push	si
	push	cx
	push	di
	push	dx
ifdef DEBUG
	cCall	S_ApplyPrlSgc,<>
else ;not DEBUG
	cCall	N_ApplyPrlSgc,<>
endif ;DEBUG
	pop	cx  ;restore cch

;				if (vpapStc.fTtp)
;					{
;					ApplyPrlSgc(hpch + cbPhe, cch, &vtapStc, sgcTap, fWord3File);
	cmp	[di.fTtpPap],fFalse
	je	CP35
	mov	ax,dataoffset [vtapStc]
	mov	dx,sgcTap
	push	[sbHpfkp]
	push	si
	push	cx
	push	ax
	push	dx
ifdef DEBUG
	cCall	S_ApplyPrlSgc,<>
else ;not DEBUG
	cCall	N_ApplyPrlSgc,<>
endif ;DEBUG

;					/* blow away non-standard properties of ttp paragraph */
;					StandardPap(&vpapStc);
;					vpapStc.fInTable = vpapStc.fTtp = fTrue;
	cCall	StandardPap,<di>
	errnz	<(fTtpPap) - (fInTablePap) - 1>
	mov	wptr ([di.fInTablePap]),(fTrue SHL 8) + fTrue

;					}
;				}
;			  }
	jmp	short CP35

CP34:
;	[sp] == pn;
;		      else
;			  {
;/* bpapx == 0 is a special state, which is used to encode a set
;   of properties which are an agreement between this clause
;   and CbGenPapxFromPap */
;			  if (doc != vdocPapLast ||
;				   vstcLast != stcNormal ||
;				   vcchPapxLast > 0)
;				   {
;				   MapStc(pdod, stcNormal, &vchpStc, &vpapStc);
;				   vstcLast = stcNormal;
;				   vcchPapxLast = 0;
;				   }
;			   Debug(else vcStyleUnchanged++);
;	CP495 performs:
;	if (doc != vdocPapLast ||
;		vstcLast != stcNormal ||
;		vcchPapxLast > 0)
;		{
;		vcchPapxLast = 0;
;		vstcLast = stcNormal;
;		MapStc(pdod, stcNormal, &vchpStc, &vpapStc);
;		}
;	Debug(else vcStyleUnchanged++);
;	ax, bx, cx, dx, si, di are altered.
	call	CP495


;			  fPaphInval = vpapStc.phe.fStyleDirty;
	mov	ax,[vpapStc.phePap.fStyleDirtyPhe]
	mov	[fPaphInval],ax

;			  vpapStc.phe.fDiffLines = 0;
;			  vpapStc.phe.clMac = 1;
;			  vpapStc.phe.dxaCol = 7980;
;/* constant in following line must match value in CbGenPapxFromPap */
;			  vpapStc.phe.dylLine = WinMac(50, 15);
	errnz	<(fDiffLinesPhe)-0>
	errnz	<(clMacPhe)-1>
	mov	wptr ([vpapStc.phePap.fDiffLinesPhe]),(1 SHL 8)
	mov	[vpapStc.phePap.dxaColPhe],7980
	mov	[vpapStc.phePap.dylLinePhe],50

;			  }
CP35:
;	[sp] == pn;
; /* if the style has been dirtied during the edit session, invalidate the paph*/
;		      if (fPaphInval)
	cmp	[fPaphInval],fFalse
	je	CP36

;			  SetWords(&vpapStc.phe, 0, cwPHE);
;	CP508 performs SetWords(&vpapStc.phe, 0, cwPHE);
;	ax, di, and es are altered.
	call	CP508

CP36:
;	[sp] == pn;
;/* establish the vpapStc cache */
;					vdocPapLast = doc;
;					vfnPapLast = fn;
;					vpnPapLast = pn;
;					vbpapxPapLast = bpapx;
;	CP58 performs:
;	vdocPapLast = doc;
;	vfnPapLast = fn;
;	vbPapxPapLast = (carry flag set upon entry ? cbSector : bpapx);
;	if (carry flag clear upon entry) vpnPapLast = dx;
;	ax is altered.
	pop	dx	;restore pn
	clc
	call	CP58

LParaEnd2:
; LParaEnd2:		      caPara.cpLim = cpMin + (fcLim - fcMin);
	mov	ax,[OFF_fcLim]
	mov	dx,[SEG_fcLim]
	sub	ax,[OFF_fcMin]
	sbb	dx,[SEG_fcMin]
	add	ax,[OFF_cpMinVar]
	adc	dx,[SEG_cpMinVar]
	mov	[caPara.LO_cpLimCa],ax
	mov	[caPara.HI_cpLimCa],dx
;		      break;
	jmp	short CP37
;		      }
;		  }
;	      }
; LNoParaEnd2:
LNoParaEnd2:
	mov	cx,[ipcd]
	inc	cx
;	CP40 puts the address of the ipcd'th pcd of hplcpcd in es:si,
;	the address of the ipcd'th cp of hplcpcd in es:di
;	and *hplcpcd in bx.  ipcd is passed in cx
;	ax, bx, dx, si, di are altered
	call	CP40
;	CP44 assumes qcp in es:di pplc in bx, ipcd in cx.
;	It returns CpFromPlc in dx:ax.	Only dx:ax are altered.
	call	CP44
	jmp	CP21

;	This loop end code is performed above in the assembler version.
; /* Now we know there's no para end. */
;	  if (ipcd != ipcdBase)
;	      {
;	      /* not original piece, save knowledge */
;	      Debug(pcd.fNoParaLastValid = fTrue);
;	      pcd.fNoParaLast = fTrue;
;	      PutPlc(hplcpcd, ipcd, &pcd);
;	      }
;	  cpGuess = cpLim;
;	  }

CP37:
;	vpapFetch = vpapStc;

	mov cx,cwPAP
	push ds
	pop es
	mov di,dataOffset [vpapFetch]
	push di 	;save &vpapFetch
	mov si,dataOffset [vpapStc]
	rep movsw
	pop di		;restore &vpapFetch

;	In the assembler version we get the prm out of the pcd at CP225
;     prm = pcd.prm;
	mov	cx,[prm]

;/* note: if the prm we're applying causes vchpStc to be altered, vstcLast will
;   be set to stcNil on return from DoPrmSgc to ensure that vchpStc is
;   recalculated when we fetch a new paragraph. */
;	if (prm != prmNil && !vpapFetch.fTtp)
	errnz	<prmNil>
	jcxz	CP38
	cmp	[vpapFetch.fTtpPap],fFalse
	jne	CP38

;	  DoPrmSgc(prm, &vpapFetch, sgcPap);
;	LN_DoPrmSgc takes prm in cx,
;	prgbProps in di, sgc in dx.
;	ax, bx, cx, dx, si, di are altered.
	errnz	<sgcPap>
	xor	dx,dx
	cCall	LN_DoPrmSgc,<>

CP38:
ifdef DEBUG
;	Assert (!vpapFetch.brcp && !vpapFetch.brcl);
	errnz	<brcpPap - 00007h>
	errnz	<brclPap - 00008h>
	cmp	wptr ([vpapFetch.brcpPap]),fFalse
	je	CP385
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFetchn2
	mov	bx,1023
	cCall	AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
CP385:
endif ;DEBUG

;     MeltHp();
ifdef	DEBUG
	call	LN_MeltHp
endif ;DEBUG
ifdef PROFILE
	cCall	StopNMeas,<>
endif ;PROFILE
	jmp	CP003
; }
;cEnd


;	CP40 puts the address of the ipcd'th pcd of hplcpcd in es:si,
;	the address of the ipcd'th cp of hplcpcd in es:di
;	and *hplcpcd in bx.  ipcd is passed in cx
;	ax, bx, dx, si, di are altered
CP40:
	mov bx,[hplcpcd]
	mov bx,[bx]
;	LN_LprgcpForPlc takes pplc in register bx,
;	and returns the result in registers es:di.
;	bx is not altered, ax, cx, dx are trashed.
	push cx     ;save ipcd
	cCall	LN_LprgcpForPlc,<>
	pop cx	    ;restore ipcd
	; cx = ipcdFetch, bx = *hplcpcd, es:di = lprgcp
	mov si,[bx.iMaxPlc]
	shl si,1
	shl si,1
	mov ax,cx
	errnz	<cbPcdMin - 8>
	shl ax,1
	shl ax,1
	add si,di
	add di,ax
	shl ax,1
	add si,ax
	ret

ifdef DEBUG
CP41:
	push ax
	push bx
	push cx
	push dx
	push si
	push di
;	CP40 puts the address of the ipcd'th pcd of hplcpcd in es:si,
;	the address of the ipcd'th cp of hplcpcd in es:di
;	and *hplcpcd in bx.  ipcd is passed in cx
;	ax, bx, dx, si, di are altered
	mov cx,[ipcd]
	call CP40
	or	es:[si.fNoParaLastValidPcd],maskFNoParaLastValidPcd
	pop di
	pop si
	pop dx
	pop cx
	pop bx
	pop ax
	ret
endif ;DEBUG


;	CP42 puts CpMacPlc of the hplc passed in bx into dx:ax
;	bx is not altered
CP42:
	push bx
	mov bx,[bx]
;	LN_LprgcpForPlc takes pplc in register bx,
;	and returns the result in registers es:di.
;	bx is not altered, ax, cx, dx are trashed.
	cCall	LN_LprgcpForPlc,<>
	mov ax,[bx.iMacPlcSTR]
	mov cx,ax
	shl ax,1
	shl ax,1
	add di,ax
;	CP44 assumes qcp in es:di pplc in bx, ipcd in cx.
;	It returns CpFromPlc in dx:ax.	Only dx:ax are altered.
	call CP44
	pop bx
	ret


;	CP43 performs
;	ax = BFromFc(es:di, fc, &fcFirst, &fcLim, &ifc);
;	where ifc is returned in cx.
;	ax, bx, cx, si, and di are altered.
CP43:
	push	dx	;save pn
	mov ax,[OFF_fc]
	mov dx,[SEG_fc]
;	LN_BFromFcCore takes qfkp in es:di, fc in dx:ax.
;	The result is returned in ax.  ifc is returned in cx.
;	Upon exit ds == es upon input, es == psDds,
;	and ds:si points to (qfkp->rgfc[ifc])
	call	LN_BFromFcCore
	lea	di,[fcFirst]
	movsw
	movsw
	lea	di,[fcLim]
	movsw
	movsw
	pop	dx	;restore pn
	push	ss
	pop	ds
	ret


;	CP44 assumes qcp in es:di pplc in bx, ipcd in cx.
;	It returns CpFromPlc in dx:ax.	Only dx:ax are altered.
CP44:
	mov	ax,es:[di]
	mov	dx,es:[di+2]
	cmp	cx,[bx.icpAdjustPlc]
	jl	CP45
	add	ax,[bx.LO_dcpAdjustPlc]
	adc	dx,[bx.HI_dcpAdjustPlc]
CP45:
	ret


;	CP46 tests (pfcb = *mpfnhfcb[fn])->fHasFib.
;	bx is set to pfcb upon return.
;	Only bx is altered.
CP46:
	mov	bx,[fn]
	shl	bx,1
	mov	bx,[bx.mpfnhfcb]
	mov	bx,[bx]
	mov	[pfcb],bx
	test	[bx.fHasFibFcb],maskFHasFibFcb
	ret


;	CP47 performs
;	fcFirstPage = ((dx:ax) >> shftSector) << shftSector;
;	dx = fcFirstPage >> shftSector
;	Only ax and dx are altered.
CP47:
	and	ax,NOT (cbSector - 1)
	mov	[OFF_fcFirstPage],ax
	mov	[SEG_fcFirstPage],dx
	errnz	<shftSector - 9>
	shr	dx,1
	rcr	ax,1
	mov	dh,dl
	mov	dl,ah
	ret


;	CP48 performs hpch = HpchGetPn(fn, dx) + (int)(fc - fcFirstPage),
;	qch = LpFromHp(hpch), leaving qch in es:di and
;	qch - (fc - fcFirstPage) in es:si.
;	it also puts [OFF_fcFirstPage] into bx.
;	dx is not altered.
CP48:
;	LN_HpchGetPn takes fn in ax, and pn in dx.
;	The result is returned in bx:ax.  ax, cx, dx, si, di are altered.
	push	dx
	mov	ax,[fn]
	call	LN_HpchGetPn
	;LN_ReloadSb takes an sb in bx and set es to the corresponding value.
	;Only es and bx are altered.
	call	LN_ReloadSb
	mov	si,ax
	add	ax,[OFF_fc]
	sub	ax,[OFF_fcFirstPage]
	xchg	ax,di
	mov	bx,[OFF_fcFirstPage]	;save old OFF_fcFirstPage
	pop	dx
	ret


;	CP49 takes hplcbte in bx.  It performs
;	hpfkp = (struct FKP *) HpchGetPn(fn, pn = PnFromPlcbteFc(hplcbte, fc)),
;	qfkp = LpFromHp(hpfkp), and returns qfkp in es:di, pn in dx, pfcb in si.
;	It returns qfkp in es:di, pn in dx, pfcb in si, and sbHpfkb in memory.
;	ax, bx, cx, dx, si, di, es are altered.
CP49:
;	LN_PnFromPlcbteFc takes hplcbte in bx, fc in dx:ax.
;	The result is returned in ax.  ax, bx, cx, dx, si, di are altered.
	mov	ax,[OFF_fc]
	mov	dx,[SEG_fc]
	call	LN_PnFromPlcbteFc
	xchg	ax,dx
	mov	ax,[fn]
;	LN_HpchGetPn takes fn in ax, and pn in dx.
;	The result is returned in bx:ax.  ax, bx, cx, dx, si, di are altered.
	push	dx	;save pn
	call	LN_HpchGetPn
	;LN_ReloadSb takes an sb in bx and set es to the corresponding value.
	;Only es and bx are altered.
	mov	[sbHpfkp],bx
	call	LN_ReloadSb
	pop	dx	;restore pn
	xchg	ax,di
	mov	si,[pfcb]
	ret


;	CP495 performs:
;	if (doc != vdocPapLast ||
;		vstcLast != stcNormal ||
;		vcchPapxLast > 0)
;		{
;		vcchPapxLast = 0;
;		vstcLast = stcNormal;
;		MapStc(pdod, stcNormal, &vchpStc, &vpapStc);
;		}
;	Debug(else vcStyleUnchanged++);
;	ax, bx, cx, dx, si, di are altered.
CP495:
	errnz	<stcNormal>
	xor	cx,cx
	db	0BBh	    ;turns next "xor bx,bx" into "mov bx,immediate"
;	CP50 performs:
;	if (doc != vdocPapLast ||
;		vstcLast != cx ||
;		vcchPapxLast > 0)
;		{
;		vcchPapxLast = 0;
;		vstcLast = cx;
;		SetWords(&vtapStc, 0, cwTAP);
;		MapStc(pdod, cx, &vchpStc, &vpapStc);
;		}
;	Debug(else vcStyleUnchanged++);
;	ax, bx, cx, dx, si, di are altered.
CP50:
	xor	bx,bx
	mov	ax,[vdocPapLast]
	cmp	ax,[doc]
	jne	CP502
	cmp	[vstcLast],cx
	jne	CP502
	cmp	[vcchPapxLast],0
	jle	CP506
CP502:
	mov	[vstcLast],cx
	xor	ax,ax
	mov	[vcchPapxLast],ax
	or	bx,bx
	jne	CP504
	push	cx	;save stc
	push	ds
	pop	es
	lea	di,[vtapStc]
	mov	cx,cbTapMin/2
	rep	stosw
	pop	cx	;restore stc
CP504:
	mov	bx,[pdod]
	mov	dx,dataOffset [vchpStc]
	mov	di,dataOffset [vpapStc]
;	LN_MapStc takes pdod in bx, stc in cx, pchp in dx, ppap in di.
;	ax, bx, cx, dx, si, di are altered.
	call	LN_MapStc
CP506:
	ret


;	CP508 performs SetWords(&vpapStc.phe, 0, cwPHE);
;	ax, di, and es are altered.
CP508:
	push	ds
	pop	es
	xor	ax,ax
	mov	di,dataoffset [vpapStc.phePap]
	errnz	<cbPheMin - 6>
	stosw
	stosw
	stosw
	ret


;	CP51 performs
;	fn = (PCD *)(es:si)->fn, fcMin = (PCD *)(es:si)->fc;
;	Upon return cx:bx = fcMin.
;	Only bx, cx are altered.
CP51:
	mov	cl,es:[si.fnPcd]
	xor	ch,ch
	mov	[fn],cx
	mov	bx,es:[si.LO_fcPcd]
	mov	cx,es:[si.HI_fcPcd]
	mov	[OFF_fcMin],bx
	mov	[SEG_fcMin],cx
	ret

;	if (ax == 1) upon entry then CP52 returns
;	   ifc != ifcFkpNil &&
;	   ((ifc > 1 && *(qfc = &qfkp->rgfc[ifcT = ifc - 1]) <= fc) &&
;	   fc < *(qfc + 1)) ||
;	   ((*(qfc = &qfkp->rgfc[ifcT = ifc]) <= fc) && fc < *(qfc + 1)))
;	if (ax == -1) upon entry then CP52 returns
;	   ifc != ifcFkpNil &&
;	   ((ifc < qfkp->crun - 1 && *(qfc = &qfkp->rgfc[ifcT = ifc + 1]) <= fc) &&
;	   fc < *(qfc + 1)) ||
;	   ((*(qfc = &qfkp->rgfc[ifcT = ifc]) <= fc) && fc < *(qfc + 1)))
;	True is indicated by zero flag reset, and false by zero flag set.
;	CP52 assumes bx = ifc, es:di = qfkp upon entry.
;	Upon return bx = ifcT, es:si = qfc, es:di = qfkp.
;	ax, bx, cx, si are altered.
CP52:
	push	es
	pop	ds
	push	dx	    ;save pn
	cmp	bl,ifcFkpNil
	je	CP56
	xchg	ax,dx
	mov	si,di
	sub	bx,dx
	push	bx
	shl	bx,1
	shl	bx,1
	errnz	<(rgfcFkp)>
	add	si,bx
	pop	bx
	mov	cx,[SEG_fc]
	mov	ax,[OFF_fc]
	or	dx,dx
	jl	CP53
	or	bx,bx
	jle	CP55
	jmp	short CP54
CP53:
	push	bx
	inc	bx
	cmp	bl,[di.crunFkp]
	pop	bx
	jae	CP55
CP54:
	cmp	ax,[si]
	push	cx
	sbb	cx,[si+2]
	pop	cx
	jl	CP55
	cmp	ax,[si+4]
	push	cx
	sbb	cx,[si+6]
	pop	cx
	jl	CP57
CP55:
	add	bx,dx		;ifc -/+ 1 ==> ifc
	shl	dx,1
	shl	dx,1
	add	si,dx
	cmp	ax,[si]
	push	cx
	sbb	cx,[si+2]
	pop	cx
	jl	CP56
	sub	ax,[si+4]
	sbb	cx,[si+6]
	jl	CP57
CP56:
	xor	dx,dx
CP57:
	or	dx,dx
	pop	dx		;restore pn
	push	ss
	pop	ds
	ret


;	CP58 performs:
;	vdocPapLast = doc;
;	vfnPapLast = fn;
;	vbPapxPapLast = (carry flag set upon entry ? cbSector : bpapx);
;	if (carry flag clear upon entry) vpnPapLast = dx;
;	ax is altered.
CP58:
	mov	ax,cbSector
	jc	CP59
	mov	[vpnPapLast],dx
	mov	ax,[bpapx]
CP59:
	mov	[vbpapxPapLast],ax
	mov	ax,[doc]
	mov	[vdocPapLast],ax
	mov	ax,[fn]
	mov	[vfnPapLast],ax
	ret


;	CP60 tests:
;	if (doc = vdocPapLast && fn == vfnPapLast
;	    && vpnPapLast == dx && vbpapxPapLast == bpapx);
;	ax is altered.
CP60:
	db	0A8h	;turns next "stc" into "test al,immediate"
			;also clears the carry flag
;	CP605 tests:
;	if (doc = vdocPapLast && fn == vfnPapLast
;	    && vbpapxPapLast == cbSector)
CP605:
	stc
	mov	ax,cbSector
	jc	CP61
	cmp	[vpnPapLast],dx
	jne	CP62
	mov	ax,[bpapx]
CP61:
	cmp	[vbpapxPapLast],ax
	jne	CP62
	mov	ax,[doc]
	cmp	[vdocPapLast],ax
	jne	CP62
	mov	ax,[fn]
	cmp	[vfnPapLast],ax
CP62:
	ret


ifdef DEBUG
;	  Assert( CpPlc( hplcpcd, ipcd + 1 ) == cpMin + ccpEop );
CP63:
	push	ax
	push	bx
	push	cx
	push	dx
	push	[hplcpcd]
	mov	ax,[ipcd]
	inc	ax
	push	ax
	cCall	CpPlc,<>
	sub	ax,[OFF_cpMinVar]
	sbb	dx,[SEG_cpMinVar]
	errnz	<ccpEop - 2>
	sub	ax,ccpEop
	or	ax,dx
	jz	CP64
	mov	ax,midFetchn2
	mov	bx,1010
	cCall	AssertProcForNative,<ax, bx>
CP64:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG


ifdef DEBUG
;	  Assert(cch >= cbPhe + 1);
CP65:
	cmp	cx,cbPheMin+1
	jge	CP66
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFetchn2
	mov	bx,1020
	cCall	AssertProcForNative,<ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
CP66:
	ret
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


sEnd	fetch
	end
