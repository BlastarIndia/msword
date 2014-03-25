	include w2.inc
	include noxport.inc
	include consts.inc
	include structs.inc

createSeg	_FETCH,fetch,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midFetchn	equ 1		; module ID, for native asserts
endif

; EXPORTED LABELS

; EXTERNAL FUNCTIONS

externFP	<ReloadSb>
externFP	<HpsAlter>
externFP	<ChUpper>
externNP	<LN_DoPrmSgc>
externNP	<LN_HpchFromFc>
externNP	<LN_HpchGetPn>
externNP	<LN_PnFromPlcbteFc>
externNP	<LN_PdodOrNilFromDoc>
externNP	<LN_BFromFcCore>
externNP	<LN_ReloadSb>

ifdef DEBUG
externFP	<AssertProcForNative>
externFP	<CpMacDoc>
externFP	<LockHeap, UnlockHeap>
endif


; EXTERNAL DATA

sBegin	data

externW mpsbps		; extern SB		mpsbps[];
externW vfEndFetch	; extern BOOL		  vfEndFetch;
externW mpdochdod	; extern struct DOD	  **mpdochdod[];
externW vchpFetch	; extern struct CHP	  vchpFetch;
externW vchpStc 	; extern struct CHP	  vchpStc;
externW vfkpdChp	; extern struct FKPD	  vfkpdChp;
externW vhpchFetch	; extern HUGE		 *vhpchFetch;
externW vdocFetch	; extern int		  vdocFetch;
externD vcpFetch	; extern CP		  vcpFetch;
externW vccpFetch	; extern int		  vccpFetch;
externW rgchInsert	; extern char		  rgchInsert[];
externW ichInsert	; extern int		  ichInsert;
externW mpfnhfcb	; extern struct FCB	  **mpfnhfcb[];
externW caPara		; extern struct CA	  caPara;
externW vibp		; extern int		  vibp;
externW vibpProtect	; extern int		  vibpProtect;
externD fcFetch 	; extern FC		  fcFetch;
externW fcmFetch	; extern int		  fcmFetch;
externD ccpChp		; extern CP		  ccpChp;
externD ccpPcd		; extern CP		  ccpPcd;
externW ccpFile 	; extern int		  ccpFile;
externW ipcdFetch	; extern int		  ipcdFetch;
externW fnFetch 	; extern int		  fnFetch;
externW prmFetch	; extern int		  prmFetch;
externW rgchCaps	; extern char		  rgchCaps[];
externW vrf		; extern struct RF	  vrf;

ifdef DEBUG
externW 		cHpFreeze
externW 		vfCheckPlc
externW 		vcIInPlcCalls
externW 		vcIInPlcHintSuccess
externW 		vcIInPlcHintPlus
externW 		wFillBlock
endif

sEnd	data


; CODE SEGMENT _FETCH

sBegin	fetch
	assumes cs,fetch
	assumes ds,dgroup
	assumes ss,dgroup



;-------------------------------------------------------------------------
;	N_FetchCp( doc, cp, fcm )
;-------------------------------------------------------------------------
; %%Function:N_FetchCp %%Owner:BRADV
cProc	N_FetchCp,<PUBLIC,FAR>,<si,di>
	ParmW	doc
	ParmD	cp
	ParmW	fcm

; native FetchCp(doc, cp, fcm)
; int doc, fcm;
; CP cp;
; {
	localD ccpMin		;	  CP ccpMin;
	localV chpX, cbChpMin	;	  struct CHP chpX;
cBegin

;	vrf.fInFetchCp = !vrf.fInFetchCp;
;	Assert (vrf.fInFetchCp);
;	if (!vrf.fInFetchCp)
;		{
;		vrf.fInFetchCp = !vrf.fInFetchCp;
;		return;
;		}
	xor	[vrf.fInFetchCpRf],maskfInFetchCpRf
	errnz	<maskFInFetchCpRf - 080h>
ifdef DEBUG
	js	FC003
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFetchn
	mov	bx,1040 		; label # for native assert
	cCall	AssertProcForNative, <ax, bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	jmp	Ltemp010
FC003:
else ;!DEBUG
	jns	Ltemp010
endif ;!DEBUG

;	  if (doc == docNil)
;		  goto LSequential;
	; ASSUME docNil == 0
	mov ax,[OFF_cp]
	mov dx,[SEG_cp]
	mov bx,[doc]
	or bx,bx
	jz LSequential

;	  Assert(cp < CpMacDoc(doc));
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cCall	CpMacDoc,<[doc]>
	mov	bx,[OFF_cp]
	sub	bx,ax
	mov	bx,[SEG_cp]
	sbb	bx,dx
	jl	FC005
	mov	ax,midFetchn
	mov	bx,1011 		; label # for native assert
	cCall	AssertProcForNative, <ax, bx>
FC005:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif

;	  if (doc == vdocFetch && cp <= vcpFetch + vccpFetch &&
;		  cp >= vcpFetch && fcm == fcmFetch)
	; bx = doc, dx:ax = cp
	cmp [vdocFetch],bx
	jnz FC06		; requires bx = doc, dx:ax = cp
	sub ax,[word ptr vcpFetch]
	sbb dx,[word ptr vcpFetch + 2]
	jl FC05
	or dx,dx
	jnz FC05		; BRIF cp - vcpFetch > 65535 or < 0
				; (& therefore > vccpFetch)
	cmp [vccpFetch],ax
	jb FC05 		; requires bx = doc
	mov cx,[fcm]
	cmp cx,[fcmFetch]
	jne FC05		; requires bx = doc
;		  {
;		  doc = docNil;
;		  vccpFetch = cp - vcpFetch;
	; dx:ax = cp - vcpFetch
	mov [doc],docNil
	mov [vccpFetch],ax

; /* Sequential call */
; LSequential:
;		  Assert(vcpFetch != cpNil && vdocFetch != docNil);
;
;		  vcpFetch += vccpFetch;  /* Go to where we left off */
;		  fcFetch += vccpFetch;
;		  }
LSequential:
ifdef DEBUG
	cmp [word ptr vcpFetch + 2],-1
	jnz FC01
	cmp [word ptr vcpFetch],-1
	jz FC02
FC01:	cmp [vdocFetch],0
	jnz FC03
FC02:	mov ax, midFetchn
	mov bx, 6			; label # for native assert
	cCall AssertProcForNative, <ax, bx>
FC03:
endif
	mov	ax,[vccpFetch]
	cwd
	add	[word ptr vcpFetch],ax
	adc	[word ptr vcpFetch + 2],dx
	add	[word ptr fcFetch],ax
	adc	[word ptr fcFetch + 2],dx
	jmp	short FC07

; RELOCATED CODE FROM CHECK FOR vcpFetch >= pdod->cpMac for speed
FC04:
	mov	[vfEndFetch],fTrue
Ltemp010:
	jmp	LEndFetchCp

;	  else
;		  {
; /* Random-access call */
;		  vcpFetch = cp;
;		  vdocFetch = doc;
;		  ccpChp = ccpPcd = ccpFile = 0;
;		  fcmFetch = fcm;
FC05:	; bx = doc
	mov ax,[OFF_cp]
	mov dx,[SEG_cp]
FC06:	; bx = doc, dx:ax = cp
	mov [word ptr vcpFetch],ax
	mov [word ptr vcpFetch + 2],dx
	mov [vdocFetch],bx
	xor ax,ax
	mov [word ptr ccpChp],ax
	mov [word ptr ccpChp + 2],ax
	mov [word ptr ccpPcd],ax
	mov [word ptr ccpPcd + 2],ax
	mov [ccpFile],ax
	mov ax,[fcm]
	mov [fcmFetch],ax
;		  }
FC07:
;
;	  pdod = *mpdochdod[vdocFetch];
;	  if (vcpFetch >= CpMacDoc(vdocFetch))
;		  {
;		  vfEndFetch = fTrue;
;		  vrf.fInFetchCp = !vrf.fInFetchCp;
;		  return;
;		  }
;	  vfEndFetch = fFalse;
;	***Do the following code from CpMacDoc in line
;	struct DOD *pdod = PdodDoc(doc);
;	return(pdod->cpMac - 2*ccpEop);
;	***End of CpMacDoc code
	mov bx,[vdocFetch]
;	PdodOrNilFromDoc takes a doc in bx, and returns hNil in ax and bx
;	if mpdochdod[doc] == hNil.  Otherwise it returns doc in ax
;	and pdod = *mpdochdod[doc] in bx.  The zero flag is set according
;	to the value of bx upon return.  Only ax and bx are altered.
	call LN_PdodOrNilFromDoc
	mov ax,2*ccpEop
	cwd
	sub ax,[bx.LO_cpMacDod]
	sbb dx,[bx.HI_cpMacDod]
	add ax,wlo vcpFetch
	adc dx,whi vcpFetch
	jge FC04
	mov [vfEndFetch],fFalse

;
; /* determine if current piece has more text in it or if new piece has to be
; looked at */
;	  if (ccpPcd > vccpFetch)
;		  ccpPcd -= vccpFetch;
	; bx = pdod
	mov ax,[word ptr ccpPcd]
	mov dx,[word ptr ccpPcd + 2]
	sub ax,[vccpFetch]
	sbb dx,0
	jl FC08
	mov cx,ax
	or  cx,dx
	jz FC08
	mov [word ptr ccpPcd], ax
	mov [word ptr ccpPcd + 2], dx
	jmp FC10

;	  else
;		{
;		struct PLCPCD **hplcpcd = pdod->hplcpcd;
;		struct PCD pcd;
;
;		if (doc == docNil)
;			++ipcdFetch; /* Save some work on sequential call */
;		else
;			/* Search for piece and remember index for next time */
;			ipcdFetch = IInPlc(hplcpcd, vcpFetch);

	; bx = pdod
FC08:
	mov bx,[bx.hplcpcdDod]
	mov	bx,[bx]
;	LN_LprgcpForPlc takes pplc in register bx,
;	and returns the result in registers es:di.
;	bx is not altered, ax, cx, dx are trashed.
	cCall	LN_LprgcpForPlc,<>
	mov ax,[ipcdFetch]
	inc ax
	cmp [doc],docNil
	je FC09
;	***Begin in line IInPlc
;	 return IcpInRgcpAdjusted(lprgcp, pplc, cp);
;}
;	LN_IcpInRgcpAdjusted expects lprgcp in es:di, pplc in bx, cp in dx:ax
;	bx and di are not altered.
	mov	ax,wlo vcpFetch
	mov	dx,whi vcpFetch
	cCall	LN_IcpInRgcpAdjusted,<>
;	***End in line IInPlc
FC09:
	mov [ipcdFetch],ax

;		GetPlc(hplcpcd, ipcdFetch, &pcd);
;		ccpPcd = CpPlc(hplcpcd, ipcdFetch + 1) - vcpFetch;
	; ax = ipcdFetch, bx = *hplcpcd, es:di = lprgcp
;	***Begin in line GetPlc
	mov si,ax
	errnz	<cbPcdMin - 8>
	mov cl,3
	shl si,cl
	add si,di
	mov cx,[bx.iMaxPlc]
	shl cx,1
	shl cx,1
	add si,cx
;	***End in line GetPlc
;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	xchg ax,cx
	inc cx
	call LN_CpFromPlcIcp
	sub ax,[word ptr vcpFetch]
	sbb dx,[word ptr vcpFetch + 2]
	mov [word ptr ccpPcd],ax
	mov [word ptr ccpPcd + 2],dx

;		Assert(ccpPcd > cp0);
ifdef DEBUG
	or dx,dx
	jl FC093
	jg FC097
	or ax,ax
	ja FC097
FC093:
	mov ax, midFetchn
	mov bx, 1000			 ; label # for native assert
	cCall AssertProcForNative, <ax, bx>
FC097:
endif ;DEBUG

;/* Invalidate other ccp's since new piece */
;		ccpChp = ccpFile = 0;
	xor ax,ax
	mov [word ptr ccpChp],ax
	mov [word ptr ccpChp + 2],ax
	mov [ccpFile],ax

;		fcFetch = pcd.fc + vcpFetch - CpPlc(hplcpcd, ipcdFetch);
;		fnFetch = pcd.fn;
;		prmFetch = pcd.prm;
;		}
	; ax = ipcdFetch, bx = *hplcpcd, es:si = &pcd, es:di = lprgcp
;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	dec cx
	call LN_CpFromPlcIcp
	not dx
	neg ax
	sbb dx,-1
	add ax,es:[si.LO_fcPcd]
	adc dx,es:[si.HI_fcPcd]
	add ax,[word ptr vcpFetch]
	adc dx,[word ptr vcpFetch + 2]
	mov [word ptr fcFetch],ax
	mov [word ptr fcFetch + 2],dx
	xor ax,ax
	mov al,es:[si.fnPcd]
	mov [fnFetch],ax
	mov ax,es:[si.prmPcd]
	mov [prmFetch],ax
FC10:

;
;	  ccpMin = ccpPcd; /* prepare for calculating next vccpFetch */
	mov ax,[word ptr ccpPcd]
	mov [OFF_ccpMin],ax
	mov ax,[word ptr ccpPcd + 2]
	mov [SEG_ccpMin],ax

;	  if (fcm & fcmChars)
;		  {

	errnz	<fcmChars - 08000h>
	test bptr (fcm+1),fcmChars SHR 8
	jnz Ltemp001
	jmp FC20
Ltemp001:
LFcmChars:

;		  if (ccpFile > vccpFetch)
;			  {
	mov ax,[ccpFile]
	mov bx,[vccpFetch]
	sub ax,bx
	jle FC11
;			  ccpFile -= vccpFetch;
;			  vhpchFetch += vccpFetch;
;			  }
	; ax = ccpFile - vccpFetch
	add wlo (vhpchFetch),bx
	jmp short FC17		; requires ax = ccpFile (not stored yet)

;		  else
; /* get hold of characters */
;			  {
FC11:

;			  if (fnFetch == fnSpec)
;				  {
;				  int ich;
;				  if ((ich = (int)fcFetch) >= cchInsertMax)
	cmp [fnFetch],fnSpec
	jnz FC16
	mov si,[word ptr fcFetch]
	cmp si,cchInsertMax
	jl FC15 	       ; requires si = ich

;					  {
; /* special pieces for out of memory fill and for an end of document Eop */
;					  if (ich == (int)fcSpecEop)
;						  {
;						  extern char rgchEop[];
;
;						  bltbyte( rgchEop,
;							   rgchCaps,
;							   (uns)(ccpFile=ccpEop));
;						  }
; #ifdef CRLF
;					  else if (ich == (int)fcSpecEop + 1)
;						  {
;						  rgchCaps [0] = chEop;
;						  ccpFile = 1;
;						  }
; #endif  /* CRLF */
	; si = ich
	errnz <ccpEop-2>
	mov ax,ccpEop-1
	;Default assume ich == fcSpecEop + 1
	mov dx,(chReturn SHL 8) + chEol
	mov cx,si
	shr cx,1
	jc FC12
	xchg dl,dh
	inc ax
FC12:
	errnz <fcSpecEop AND 1>
	cmp cx,fcSpecEop SHR 1
	jne FC13
	mov [rgchCaps],dx
	jmp short FC14	      ; requires ax = ccpFile (not stored yet)

;					  else
;						  {
;						  SetBytes(rgchCaps, '*', ichCapsMax);
;						  ccpFile = (CP)ichCapsMax;
FC13:
	push ds
	pop es
	mov cx,ichCapsMax
	push cx     ;save ccpFile
	mov di,dataOffset rgchCaps
	mov al,'*'
	rep stosb
	pop ax	    ;restore ccpFile
;						  }
FC14:
	; ax = ccpFile (not stored yet)
;					  vhpchFetch = (char HUGE *)&rgchCaps[0];
;					  }
	; ax = ccpFile (not stored yet)
	mov wlo (vhpchFetch),dataOffset rgchCaps
	jmp short FC155 		; requires ax = ccpFile
;				  else
; /* Special quick and dirty insert mode. */
;					  {
;					  vhpchFetch = (char HUGE *)&rgchInsert[ich];
;					  ccpFile = cchInsertMax - ich;
;					  }
FC15:
	; si = ich
	lea ax,[si+rgchInsert]
	mov wlo (vhpchFetch),ax
	mov ax,cchInsertMax
	sub ax,si
	mov [ccpFile],ax
FC155:
	mov whi (vhpchFetch),sbDds
	jmp short FC17			; requires ax = ccpFile
;				  }
;			  else
;				  {
FC16:
; /* No monkeying with files after this statement, or we may page out */
;				  vhpchFetch = HpchFromFc(fnFetch, fcFetch); /* Read in buffer */
;				  vibpProtect = vibp;
;				  ccpFile = cbSector - (fcFetch & maskSector);
;				  }
	mov ax,[fnFetch]
	mov bx,wptr (fcFetch)
	mov dx,wptr (fcFetch + 2)
;	LN_HpchFromFc takes fn in ax, and fc in dx:bx.
;	The result is returned in dx:ax.
;	ax, bx, cx, dx, si, di are altered.
	call LN_HpchFromFc
	mov wlo (vhpchFetch),ax
	mov whi (vhpchFetch),dx
	mov ax,[vibp]
	mov [vibpProtect],ax
	mov ax,[word ptr fcFetch]
	and ax,maskSector
	neg ax
	add ax,cbSector
;			  }
FC17:
;		  if ((CP)ccpFile < ccpMin) ccpMin = ccpFile;
	; ax = ccpFile (not yet stored)
	mov [ccpFile],ax
	mov dx,[SEG_ccpMin]
	or dx,dx
	jnz FC18
	cmp ax,[OFF_ccpMin]
	jae FC20
FC18:	mov [OFF_ccpMin],ax
	mov [SEG_ccpMin],0
ifdef DEBUG
	cmp ax,cbSector
	jbe FC19
	mov ax, midFetchn
	mov bx, 10			 ; label # for native assert
	cCall AssertProcForNative, <ax, bx>
FC19:
endif
LEndFcmChars:
;		  }
FC20:

;
;	  if (fcm & fcmProps)
	errnz	<fcmProps - 080h>
	test bptr (fcm),fcmProps
	jnz Ltemp003
	jmp FC35
Ltemp003:
LFcmProps:
;		  { /* There must be enough page buffers so that this will not
;			  page out vhpchFetch! */
;		  if (ccpChp > vccpFetch)
;			  ccpChp -= vccpFetch;
	mov ax,[vccpFetch]
	cwd
	sub [word ptr ccpChp],ax
	sbb [word ptr ccpChp + 2],dx
	jl Ltemp004
	mov ax,[word ptr ccpChp]
	or ax,[word ptr ccpChp + 2]
	jz Ltemp004
	jmp FC31

;		  else
;			  {
Ltemp004:
; /* Fill vchpFetch with char props; length of run to ccpChp */
;			  struct FKP HUGE *hpfkp;
;			  struct FCB *pfcb;
;			  struct CHP *pchp;
;
;			  FreezeHp();
;
;			  blt(&vchpStc, &vchpFetch, cwCHP);
ifdef	DEBUG
	call	LN_FreezeHp
endif ;DEBUG
	mov cx,cwCHP
	push ds
	pop es
	mov di,dataOffset vchpFetch
	mov si,dataOffset vchpStc
	rep movsw

;			  if (fnFetch == fnSpec)
;				  {
	cmp [fnFetch],fnSpec
	jnz FC22

;				  if ((int)fcFetch >= cchInsertMax)
;					  {
;					  ccpChp = cpMax;
;					  }
;				  else
	mov [word ptr ccpChp],0ffffh
	mov [word ptr ccpChp + 2],07fffh
	cmp [word ptr fcFetch],cchInsertMax
	jge Ltemp005

;					  {
;					  ApplyChpxTransforms(&vfkpdChp.chp);
; /* ichInsert points to first "vanished" character in the rgchInsert array */
	mov si,dataOffset vfkpdChp+chpFkpd
;	LN_ApplyChpxTransforms expects pchpX in si.
;	ax and bx are altered.
	call LN_ApplyChpxTransforms

;					  if (ichInsert <= (int)fcFetch)
;						  {
;						  /* in the vanished region */
;						  vchpFetch.fVanish = fTrue;
;						  vchpFetch.fSysVanish = fTrue;
;						  ccpChp = cpMax;
;						  }
;					  else
;						  ccpChp = ichInsert - (int)fcFetch;

	mov ax,[ichInsert]
	sub ax,[word ptr fcFetch]
	jg FC21
	; ccpChp is already set to cpMax
	or [vchpFetch.fVanishChp],maskFVanishChp	; ccpChp is already set to cpMax
	or [vchpFetch.fSysVanishChp],maskFSysVanishChp
	jmp FC28
FC21:
	cwd
	mov [word ptr ccpChp],ax
	mov [word ptr ccpChp + 2],dx
;					  }
;				  }
Ltemp005:
	jmp FC28

;			  else if (fnFetch == fnScratch
;				  && fcFetch >= vfkpdChp.fcFirst)
;				  {
FC22:

;				  ApplyChpxTransforms(&vfkpdChp.chp);
;				  ccpChp = cpMax;
;				  }
	cmp [fnFetch],fnScratch
	jne FC23
	mov ax,[word ptr fcFetch]
	mov dx,[word ptr fcFetch + 2]
	sub ax,[vfkpdChp.LO_fcFirstFkpd]
	sbb dx,[vfkpdChp.HI_fcFirstFkpd]
	jl FC23
	mov si,dataOffset vfkpdChp+chpFkpd
;	LN_ApplyChpxTransforms expects pchpX in si.
;	ax and bx are altered.
	call LN_ApplyChpxTransforms
	mov [word ptr ccpChp],0ffffh
	mov [word ptr ccpChp + 2],07fffh
Ltemp006:
	jmp FC28

;			  else
;				  {
FC23:
; /* ensure current para */
;				  Assert(caPara.doc == vdocFetch &&
;					  caPara.cpFirst <= vcpFetch &&
;					  caPara.cpLim > vcpFetch);
ifdef DEBUG
	mov ax,vdocFetch
	cmp ax,[caPara.docCa]
	jne FC24
	mov ax,[word ptr vcpFetch]
	mov dx,[word ptr vcpFetch + 2]
	mov bx,ax
	mov cx,dx
	sub ax,[caPara.LO_cpFirstCa]
	sbb dx,[caPara.HI_cpFirstCa]
	jl FC24
	sub bx,[caPara.LO_cpLimCa]
	sbb cx,[caPara.HI_cpLimCa]
	jl FC25
FC24:	mov ax, midFetchn
	mov bx, 27			 ; label # for native assert
	cCall AssertProcForNative, <ax, bx>
FC25:
endif ;DEBUG
; /* initial estimate of size of CHP run. will be revised if piece not from
;    text file. */
;				  ccpChp = ccpPcd;
;
;				  pfcb = *(mpfnhfcb[fnFetch]);
	mov ax,[word ptr ccpPcd]
	mov [word ptr ccpChp],ax
	mov ax,[word ptr ccpPcd + 2]
	mov [word ptr ccpChp + 2],ax
	mov bx,[fnFetch]
	shl bx,1
	mov bx,[bx+mpfnhfcb]
	mov bx,[bx]

; /* note that the scratch file FCB has the fHasFib flag on as an expedient lie
;    which allows us to recognize that the scratch file has PLCBTEs also. */
;				  if (pfcb->fHasFib)
;					{ /* Copy necessary amt of formatting info over std CHP */
	; bx = pfcb
	test [bx.fHasFibFcb],maskFHasFibFcb
	jz Ltemp006


;					int pn;
;					FC far *qfc;
;					FC fcMin, fcMac;
;					int bchpx;
;					struct CHP chpX;
;					Debug(vcFetchChp++);
;					hpfkp = (struct FKP *) HpchGetPn(fnFetch,
;						pn = PnFromPlcbteFc(pfcb->hplcbteChp, fcFetch));
	; bx = pfcb
	push	bx     ;save pfcb
	mov	bx,[bx.hplcbteChpFcb]
	mov	ax,[word ptr fcFetch]
	mov	dx,[word ptr fcFetch+2]
;	LN_PnFromPlcbteFc takes hplcbte in bx, fc in dx:ax.
;	The result is returned in ax.  ax, bx, cx, dx, si, di are altered.
	call	LN_PnFromPlcbteFc
	xchg	ax,dx
	push	dx     ;save pn
	mov ax,[fnFetch]
;	LN_HpchGetPn takes fn in ax, and pn in dx.
;	The result is returned in bx:ax.  ax, cx, dx, si, di are altered.
	call	LN_HpchGetPn
	;LN_ReloadSb takes an sb in bx and set es to the corresponding value.
	;Only es and bx are altered.
	call	LN_ReloadSb
	xchg	ax,di
	pop	si     ;restore pn
	pop	bx     ;restore pfcb


;					if ((pn == pfcb->pnChpHint && pfcb->ifcChpHint != ifcFkpNil &&
;						*(qfc = &qfkp->rgfc[pfcb->ifcChpHint]) <= fcFetch) &&
;						fcFetch < *(qfc + 1))
;						{
	; bx = pfcb, si = pn, es:di = pfkp
	push bx
	cmp si,[bx.pnChpHintFcb]
	jne FC26
	mov bx,[bx.ifcChpHintFcb]
	cmp bx,ifcFkpNil
	je FC26
	shl bx,1
	shl bx,1
	errnz <(rgfcFkp)>
	add bx,di		; es:bx <== qfc
	mov ax,[word ptr fcFetch]
	mov dx,[word ptr fcFetch + 2]
	sub ax,es:[bx]
	sbb dx,es:[bx+2]
	jl FC26
	mov ax,[word ptr fcFetch]
	mov dx,[word ptr fcFetch + 2]
	sub ax,es:[bx+4]
	sbb dx,es:[bx+6]
	jge FC26


;						fcMac = *(qfc + 1);
;						bchpx = ((char far *)&((qfkp->rgfc)[crun+1]))
;							[pfcb->ifcChpHint];
;						bchpx = bchpx << 1;
;						Debug(vcFetchSameChp++);
	; es:bx = pfc, es:di = pfkp, pfcb is pushed
	pop si
	mov cx,es:[bx+4]
	mov dx,es:[bx+6]
	xor bx,bx
	mov bl,es:[di.crunFkp]
	inc bx
	shl bx,1
	shl bx,1
	errnz <(rgfcFkp)>
	add bx,di
	add bx,[si.ifcChpHintFcb]
	xor ah,ah
	mov al,es:[bx]
	shl ax,1
	xchg ax,si
	jmp short FC27	      ; requires dx:cx=fcMac, es:di = pfkp, si = bchpx
;						}
;					else
;						{
;						/* fcMin not used */
;						bchpx = BFromFc(hpfkp, fcFetch, &fcMin/*nil*/, &fcMac, &pfcb->ifcChpHint);
;						pfcb->pnChpHint = pn;
	; es:di = qfkp, si = pn, pfcb is pushed
FC26:	pop	bx
	mov	[bx.pnChpHintFcb],si
	push	di     ;save hpfkp
	push	bx     ;save pfcb
	mov	ax,wlo (fcFetch)
	mov	dx,whi (fcFetch)
;	LN_BFromFcCore takes qfkp in es:di, fc in dx:ax.
;	The result is returned in ax.  ifc is returned in cx.
;	Upon exit ds == es upon input, es == psDds,
;	and ds:si points to (qfkp->rgfc[ifc])
	call	LN_BFromFcCore
	pop	bx	;restore pfcb
	mov	ss:[bx.ifcChpHintFcb],cx
	mov	cx,[si+4]
	mov	dx,[si+6]
	push	ds
	pop	es
	push	ss
	pop	ds
	xchg	si,ax
	pop	di	;restore hpfkp

;						}


;					  ccpChp = fcMac - fcFetch;
;					  if (bchpx != 0/*nil*/)
;						  {
	; es:di = qfkp, si = bchpx, dx:cx = fcMac
FC27:
	sub cx,[word ptr fcFetch]
	sbb dx,[word ptr fcFetch + 2]
	mov [word ptr ccpChp],cx
	mov [word ptr ccpChp + 2],dx
	or si,si
	jz FC28

;						  char *qchpx = &((char far *)qfkp)[bchpx];
;						  int cch = *qchpx++;
;						  SetWords(&chpX, 0, cwCHP);
;						  bltbx(qchpx,(char far *)&chpX, cch);
;						  /* "apply" transformations expressed in the chpx */
;						  ApplyChpxTransforms(&chpX);
	add si,di
	push es
	pop ds
	push ss
	pop es
	lea di,[chpX]
	push di     ;save &chpX
	lodsb
	cbw
	mov cx,ax
	rep movsb

	mov cx,cbCHP
	sub cx,ax
	xor ax,ax
	rep stosb

	pop si	    ;restore &chpX
	push ss
	pop ds
;	LN_ApplyChpxTransforms expects pchpX in si.
;	ax and bx are altered.
	call LN_ApplyChpxTransforms

;						  }
;					  }
;				  }
FC28:

; /* limit scope of to min of paragraph end or run end */
;			  if (ccpChp > caPara.cpLim - vcpFetch)
;				  ccpChp = caPara.cpLim - vcpFetch;
	mov ax,[caPara.LO_cpLimCa]
	mov dx,[caPara.HI_cpLimCa]
	sub ax,[word ptr vcpFetch]
	sbb dx,[word ptr vcpFetch + 2]
	mov bx,ax
	mov cx,dx
	sub ax,[word ptr ccpChp]
	sbb dx,[word ptr ccpChp + 2]
	jge FC29
	mov [word ptr ccpChp],bx
	mov [word ptr ccpChp + 2],cx
FC29:

;
;			  if (prmFetch != prmNil)
;				  DoPrmSgc(prmFetch, &vchpFetch, sgcChp);
;			  MeltHp();
	mov cx,[prmFetch]
	errnz <prmNil>
	jcxz FC30
;	LN_DoPrmSgc takes prm in cx,
;	prgbProps in di, sgc in dx.
;	ax, bx, cx, dx, si, di are altered.
	mov	dx,sgcChp
	mov	di,dataOffset vchpFetch
	cCall	LN_DoPrmSgc,<>
FC30:
ifdef	DEBUG
	call	LN_MeltHp
endif ;DEBUG

;			  }
FC31:

;
;		  if (ccpChp < ccpMin) ccpMin = ccpChp;
	mov ax,[word ptr ccpChp]
	mov dx,[word ptr ccpChp + 2]
	mov bx,ax
	mov cx,dx
	sub ax,[OFF_ccpMin]
	sbb dx,[SEG_ccpMin]
	jge FC32
	mov [OFF_ccpMin],bx
	mov [SEG_ccpMin],cx
FC32:
ifdef REVIEW
;		 /*  The End-of-Doc EOP is never allowed to be vanished */
;		 if (vchpFetch.fVanish && vcpFetch + ccpMin == cpMac)
;		     {
;		     if (ccpMin <= ccpEop)
;			 {
;			 vchpFetch.fVanish = fFalse;
;			 }
;		     else
;			 {
;			 ccpMin -= ccpEop;
;			 }
;		     }
	test [vchpFetch.fVanishChp],maskFVanishChp
	jz FC34
	mov ax,[word ptr vcpFetch]
	mov dx,[word ptr vcpFetch + 2]
	add ax,[OFF_ccpMin]
	adc dx,[SEG_ccpMin]
	sub ax,[OFF_cpMac]
	sbb dx,[SEG_cpMac]
	or ax,dx
	jnz FC34
	mov ax,ccpEop
	cwd
	sub ax,[OFF_ccpMin]
	sbb dx,[SEG_ccpMin]
	jl FC33
	and [vchpFetch.fVanishChp],NOT maskFVanishChp
	jmp short FC34
FC33:	sub [OFF_ccpMin],ccpEop
	sbb [SEG_ccpMin],0
FC34:
endif ; REVIEW
;
;		  }
;
LEndFcmProps:
FC35:

; /* convert vccpFetch to an integer and prevent larger than 15 bit reach.
; (possible if fcmChar is not set!)
; */
;	  vccpFetch = (ccpMin >= 0x7FFF) ? 0x7FFF : (int)ccpMin;
	mov ax,07fffh		; code is optimized for no jumps
	cmp [SEG_ccpMin],0     ; taken if limiting is not req'd
	jnz FC36
	mov bx,[OFF_ccpMin]
	or bx,bx
	jl FC36
	xchg ax,bx
FC36:
	mov [vccpFetch],ax

;	  Assert(vccpFetch > 0);
;
ifdef DEBUG
	cmp [vccpFetch],0
	jg FC37
	mov ax, midFetchn
	mov bx, 33			 ; label # for native assert
	cCall AssertProcForNative, <ax, bx>
FC37:
endif ;DEBUG
ifdef DEBUG
	test [fcm],fcmChars
	jz FC38
	cmp [vccpFetch],cbSector
	jbe FC38
	mov ax, midFetchn
	mov bx,300			 ; label # for native assert
	cCall AssertProcForNative, <ax, bx>
FC38:
endif ;DEBUG

;	  if ((fcm & fcmParseCaps) != 0)
	errnz	<fcmParseCaps - 1>
	test bptr (fcm),fcmParseCaps
	jz Ltemp007
LFcmParseCaps:

;		  {
;		  char far *qch;
;		  char far *qchFetch;
;		  qch = qchFetch = LpFromHp(vhpchFetch);
	mov	bx,whi ([vhpchFetch])
	;LN_ReloadSb takes an sb in bx and set es to the corresponding value.
	;Only es and bx are altered.
	call	LN_ReloadSb
	mov	si,wlo ([vhpchFetch])

;		  int cch = vccpFetch - 1;

; /* first check for word underline */
;		if (vchpFetch.kul == kulWord)
	mov	cx,[vccpFetch]
	dec	cx
	mov	al,[vchpFetch.kulChp]
	and	al,maskKulChp
	cmp	al,kulWord SHL ibitKulChp
	jnz	FC44

;			{

;			int ch = *qch++;
;			if (ch == chSpace || ch == chTab || ch == chNonBreakSpace)
	; es:si = qch, cx = cch, es = hiword(qch)
	and	[vchpFetch.kulChp],NOT maskKulChp
	push	es
	pop	ds
;	FC57 performs lodsb
;	and returns (al == chSpace || al == chTab || al == chNonBreakSpace).
;	Only al and si are altered.
	call	FC57
	jnz	FC40	    ; requires es:si = qch, cx = cch

;				{
;				while (cch-- != 0 &&
;					(*qch == chSpace ||
;					*qch == chTab || *qch == chNonBreakSpace)) pch++;
;				vchpFetch.kul = 0;
;				}
	; ds:si = qch, cx = cch, es = hiword(qch)
FC39:
	jcxz	FC43
	dec	cx
;	FC57 performs lodsb
;	and returns (al == chSpace || al == chTab || al == chNonBreakSpace).
;	Only al and si are altered.
	call	FC57
	jz	FC39
	jmp	short FC42
;			else
;				{
;				while (cch-- != 0 &&
;					(*pch != chSpace &&
;					*pch != chTab && *pch != chNonBreakSpace)) pch++;
;				vchpFetch.kul = kulSingle;
;				}
	; ds:si = qch, cx = cch, es = hiword(qch)
FC40:
	or	ss:[vchpFetch.kulChp], kulSingle SHL ibitKulChp
FC41:
	jcxz	FC43
	dec	cx
;	FC57 performs lodsb
;	and returns (al == chSpace || al == chTab || al == chNonBreakSpace).
;	Only al and si are altered.
	call	FC57
	jnz	FC41
;				  }
FC42:
	dec	si
FC43:
;			  vccpFetch = qch - qchFetch;
;			  ccpChp = 0;
	; ds:si = qch, es = hiword(qch)
	push	ss
	pop	ds
	sub	si,wlo (vhpchFetch)
	mov	[vccpFetch],si
	xor	ax,ax
	mov	[word ptr ccpChp],ax
	mov	[word ptr ccpChp + 2],ax

;			  }
FC44:

;		  qch = qchFetch;
;		  cch = vccpFetch - 1;
;		  if (vchpFetch.fSmallCaps || vchpFetch.fCaps)
;			  {
	errnz <(fCapsChp)-(fSmallCapsChp)>
	test	[vchpFetch.fSmallCapsChp],maskFSmallCapsChp+maskFCapsChp
	jnz	Ltemp002
Ltemp007:
	jmp	LEndFetchCp
Ltemp002:
	mov	di,wlo (vhpchFetch)
	;Assembler note: Set cch = vccpFetch - 1 below.

; /* run length will be limited to min(ichCapsMax, vccpFetch).
; If small caps: if chars are already caps, further limit length to run of caps
; and we are done. If LC chars need to change, change chp and copy UC versions
; to rgchCaps. If fCaps, just copy UC versions.
; */
;			  if (vchpFetch.fCaps)
;				  {
; LCopyCaps:
	test	[vchpFetch.fCapsChp],maskFCapsChp
	jz	FC48 ; expects cx = cch, es:di = qch
LCopyCaps:

;				  vccpFetch = min(vccpFetch, ichCapsMax);
	cmp	[vccpFetch],ichCapsMax
	jl	FC45
	mov	[vccpFetch],ichCapsMax
FC45:
;				  for (cch = 0, pch = &rgchCaps[0]; cch < vccpFetch; cch++)
;					  *pch++ = ChUpper(*qchFetch++);
	mov	cx,[vccpFetch]
	mov	si,wlo [vhpchFetch]
	mov	di,dataOffset rgchCaps
	push	es
	pop	ds
	push	ss
	pop	es
	push	di
	push	cx
	rep	movsb
	pop	cx
	pop	si
	push	ss
	pop	ds
	jcxz	FC47
FC46:
	push	cx
	lodsb
	xor	ah,ah
	cCall	ChUpper,<ax>
	mov	bptr [si-1],al
	pop	cx
	loop	FC46
FC47:
;				  vhpchFetch = &rgchCaps[0];
;				  ccpFile = 0; /* since pch is changed */
	mov	wlo (vhpchFetch),dataOffset rgchCaps
	mov	whi (vhpchFetch),sbDds
	mov	[ccpFile],0

;				  }


	jmp	short LEndFetchCp
;			  else
;				  {
FC48:
;				  if (!FLower(*qch++))
	;Assembler note: Setting cch to vccpFetch - 1 is done above in
	;in the C source.
	;Set cch to vccpFetch rather than vccpFetch - 1 because the loop
	;instructions we are using perform a pre-decrement rather than
	;a post-decrement on cch.
	mov	cx,[vccpFetch]
	; cx = cch + 1, es:di = qch
	mov	si,di
;	LN_FLower takes ch in al, and returns with carry set if true;
;	Only al is altered
	lods	bptr es:[si]
	call	LN_FLower
	jc	FC52 ; expects cx = cch + 1, ds:si = qch

;					  {
; /* text is already in upper case */
;					  while (cch-- != 0 && !FLower(*hpch))
;						  qch++;
	; cx = cch + 1, es:si = qch
;	LN_FLower takes ch in al, and returns with carry set if true;
;	Only al is altered
FC49:
	dec	cx
	jle	FC51
	lods	bptr es:[si]
	call	LN_FLower
	jnc	FC49
	dec	si

FC51:
;					  vccpFetch = qch - qchFetch;
	; es:si = qch
	sub	si,wlo [vhpchFetch]
	mov	[vccpFetch],si

;					  }
	jmp	short LEndFetchCp
;				  else
;					  {
FC52:
	; cx = cch + 1, es:si = qch
FC53:
; /* text is in lower case, change size and case */
;					  while (cch-- != 0
;						  && (FLower(*hpch) || *hpch == chSpace))
;						  qch++;
	; cx = cch + 1, es:si = qch
	dec	cx
	jle	FC56
	lods	bptr es:[si]
	cmp	al,chSpace
	je	FC53
;	LN_FLower takes ch in al, and returns with carry set if true;
;	Only al is altered
	call	LN_FLower
	jc	FC53
	dec	si
FC56:

;					  vccpFetch = qch - qchFetch;
; /* mapping of point size for small caps: */
;					  vchpFetch.hps = HpsAlter(vchpFetch.hps, -1);
	; es:si = qch
	sub	si,wlo (vhpchFetch)
	mov	[vccpFetch],si
	xor	ax,ax
	mov	al,[vchpFetch.hpsChp]
	push	ax
	cwd
	dec	dx
	push	dx
	call	far ptr HpsAlter
	mov	[vchpFetch.hpsChp],al

;					  ccpChp = 0; /* since chp is changed */
;					  goto LCopyCaps;
	xor	ax,ax
	mov	[word ptr ccpChp],ax
	mov	[word ptr ccpChp + 2],ax
	mov	bx,whi (vhpchFetch)
	;LN_ReloadSb takes an sb in bx and set es to the corresponding value.
	;Only es and bx are altered.
	call	LN_ReloadSb
	jmp	LCopyCaps   ; expects es = hiword(qch)
;					  }
;				  }
;			  }
;		  }

LEndFetchCp:
	xor	[vrf.fInFetchCpRf],maskfInFetchCpRf
; }
;
cEnd


;	FC57 performs lodsb
;	and returns (al == chSpace || al == chTab || al == chNonBreakSpace).
;	Only al and si are altered.
FC57:
	lods	bptr es:[si]
	cmp	al,chSpace
	jz	FC58
	cmp	al,chTab
	jz	FC58
	cmp	al,chNonBreakSpace
FC58:
	ret


; End of FetchCp


;	LN_FLower takes ch in al, and returns with carry set if true;
;	Only al is altered
LN_FLower:
;    return (((uns)(ch - 'a') <= ('z' - 'a')) ||
	sub	al,'a'
	cmp	al,'z'-'a'+1
	jc	LN_FL01
;	     /* foreign */
;	     ((uns)(ch - 0x00DF) <= (0x00F6 - 0x00DF)) ||
	sub	al,0DFh-'a'
	cmp	al,0F6h-0DFh+1
	jc	LN_FL01
;	     ((uns)(ch - 0x00F8) <= (0x00FF - 0x00F8)));
	sub	al,0F8h-0DFh
	cmp	al,0FFh-0F8h+1
LN_FL01:
	ret

;-------------------------------------------------------------------------
;	ApplyChpxTransforms(pchpX)
;-------------------------------------------------------------------------
;	LN_ApplyChpxTransforms expects pchpX in si.
;	ax and bx are altered.
; %%Function:LN_ApplyChpxTransforms %%Owner:BRADV
cProc	LN_ApplyChpxTransforms,<PUBLIC,NEAR,ATOMIC>,<>

; /* A P P L Y	C H P X  T R A N S F O R M S */
; native ApplyChpxTransforms(pchpX)
; struct CHP *pchpX;
; {
cBegin	nogen
;	  *(int *)&vchpFetch ^= *(int *)pchpX;
	mov	bx,[si]
	xor	[vchpFetch],bx

;	  if (*(int *)pchpX & (maskFs | maskFSpec))
;		  {
	errnz	<maskFsChp AND 000FFh>
	errnz	<(fSpecChp) - 1>
	test	bh,(maskFsChp SHR 8) + maskFSpecChp
	jz	LN_ACT08

;		  if (pchpX->fsSpace) vchpFetch.qpsSpace = pchpX->qpsSpace;
	errnz	<(fsSpaceChp) - 1>
	errnz	<maskFsSpaceChp - 080h>
	shl	bh,1
	jnc	LN_ACT01
	errnz	<maskQpsSpaceChp - 03Fh>
	mov	al,[si.qpsSpaceChp]
	and	al,maskQpsSpaceChp
	and	[vchpFetch.qpsSpaceChp],NOT maskQpsSpaceChp
	or	[vchpFetch.qpsSpaceChp],al
LN_ACT01:

;		  if (pchpX->fsPos) vchpFetch.hpsPos = pchpX->hpsPos;
	errnz	<(fsPosChp) - 1>
	errnz	<maskFsPosChp - 040h>
	shl	bh,1
	jnc	LN_ACT02
	mov	al,[si.hpsPosChp]
	mov	[vchpFetch.hpsPosChp],al
LN_ACT02:

;		  if (pchpX->fsKul) vchpFetch.kul = pchpX->kul;
	errnz	<(fsKulChp) - 1>
	errnz	<maskFsKulChp - 020h>
	shl	bh,1
	jnc	LN_ACT03
	and	[vchpFetch.kulChp],NOT maskKulChp
	mov	al,[si.kulChp]
	and	al,maskKulChp
	or	[vchpFetch.kulChp],al
LN_ACT03:

;		  if (pchpX->fsHps) vchpFetch.hps = pchpX->hps;
	errnz	<(fsHpsChp) - 1>
	errnz	<maskFsHpsChp - 010h>
	shl	bh,1
	jnc	LN_ACT04
	mov	al,[si.hpsChp]
	mov	[vchpFetch.hpsChp],al
LN_ACT04:

;		  if (pchpX->fsFtc) vchpFetch.ftc = pchpX->ftc;
	errnz	<(fsFtcChp) - 1>
	errnz	<maskFsFtcChp - 008h>
	shl	bh,1
	jnc	LN_ACT05
	mov	ax,[si.ftcChp]
	mov	[vchpFetch.ftcChp],ax
LN_ACT05:

;		 if (pchpX->fsIco) vchpFetch.ico = pchpX->ico;
	errnz	<(fsIcoChp) - 1>
	errnz	<maskFsIcoChp - 004h>
	shl	bh,1
	jnc	LN_ACT06
	errnz	<maskIcoChp - 00Fh>
	mov	al,[si.icoChp]
	and	al,maskIcoChp
	and	[vchpFetch.icoChp],NOT maskIcoChp
	or	[vchpFetch.icoChp],al
LN_ACT06:

;		 if (pchpX->fSpec) vchpFetch.fcPic = pchpX->fcPic;
	errnz	<(FspecChp) - 1>
	errnz	<maskFSpecChp - 002h>
	shl	bh,1
	jnc	LN_ACT07
	mov	ax,[si.LO_fcPicChp]
	mov	[vchpFetch.LO_fcPicChp],ax
	mov	ax,[si.HI_fcPicChp]
	mov	[vchpFetch.HI_fcPicChp],ax
LN_ACT07:

;
;		  }
; }
LN_ACT08:
cEnd	nogen
	ret

;-------------------------------------------------------------------------
;	LprgcpForPlc( pplc )
;-------------------------------------------------------------------------
;/* L P R G C P  F O R	P L C */
;native CP far *LprgcpForPlc(pplc)
;struct PLC *pplc;
;{
;#ifdef ENABLE
;	if (pplc->fExternal)
;		return ((CP far *)LpFromHp(HpOfHq(pplc->hqplce)));
;	else
;#endif
;		return ((CP far *)pplc->rgcp);

; %%Function:LN_LprgcpForPlc %%Owner:BRADV
cProc	LN_LprgcpForPlc,<PUBLIC,NEAR,ATOMIC>,<>
;	LN_LprgcpForPlc takes pplc in register bx,
;	and returns the result in registers es:di.
;	bx is not altered, ax, cx, dx are trashed.

cBegin	nogen
;	expect pplc in bx, return HpOfHq(hqplce) in es:di, bx is not altered.
include qplc.asm

;}
cEnd	nogen
	ret

; End of LN_LprgcpForPlc


;-------------------------------------------------------------------------
;	DcpAdjust(pplc,icp)
;-------------------------------------------------------------------------
;incorporated in line into CpFromPlcIcp
;CP DcpAdjust(pplc, icp)
;struct PLC *pplc;
;int icp;
;{
; %%Function:DcpAdjust %%Owner:BRADV
;cProc	 DcpAdjust,<PUBLIC,NEAR,ATOMIC>,<>
;	DcpAdjust takes pplc in register bx, and icp in register
;	cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
;
;cBegin
;	 return (icp < pplc->icpAdjust) ? cp0 : pplc->dcpAdjust;
;	 xor	 ax,ax
;	 cwd
;	 cmp	 cx,[bx.icpAdjustPlc]
;	 jl	 DA01
;	 mov	 ax,[bx.LO_dcpAdjustPlc]
;	 mov	 dx,[bx.HI_dcpAdjustPlc]
;DA01:
;}
;cEnd

; End of DcpAdjust


;-------------------------------------------------------------------------
;	CpFromPlcIcp(lprgcp,pplc,icp)
;-------------------------------------------------------------------------
;CP CpFromPlcIcp(pplc, icp)
;struct PLC *pplc;
;int icp;
;{

; %%Function:LN_CpFromPlcIcp %%Owner:BRADV
cProc	LN_CpFromPlcIcp,<PUBLIC,NEAR,ATOMIC>,<>
;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.

cBegin	nogen
;	 return lprgcp[icp] + (icp < pplc->icpAdjust) ? cp0 : pplc->dcpAdjust;
	push	di	;save offset lprgcp
	mov	ax,cx
	shl	ax,1
	shl	ax,1
	add	di,ax
	mov	ax,es:[di]
	mov	dx,es:[di+2]
	cmp	cx,[bx.icpAdjustPlc]
	jl	CFPI01
	add	ax,[bx.LO_dcpAdjustPlc]
	adc	dx,[bx.HI_dcpAdjustPlc]
CFPI01:
	pop	di	;restore offset lprgcp
;}
cEnd	nogen
	ret

; End of LN_CpFromPlcIcp


;-------------------------------------------------------------------------
;	IInPlc(pplc,cp)
;-------------------------------------------------------------------------
; /* I	I N  P L C */
; /* Binary search plc table for cp; return largest i s.t. rgcp[i] <= cp.
; note, search will not work for cp's less than rgcp[0].
; */
; native int IInPlc(hplc, cp)
; struct PLC **hplc;
; CP cp;
; {
;	 struct PLC *pplc = *hplc;
;	 CP far *lprgcp = LprgcpForPlc(pplc);
; %%Function:IInPlc %%Owner:BRADV
cProc	IInPlc,<PUBLIC,FAR>,<si,di>
	ParmW	hplc
	ParmD	cp

ifdef DEBUG
	LocalW	bxSave
endif ;DEBUG

cBegin
	mov	bx,[hplc]
	mov	bx,[bx]
ifdef DEBUG
	mov	[bxSave],bx
endif ;DEBUG
;	LN_LprgcpForPlc takes pplc in register bx,
;	and returns the result in registers es:di.
;	bx is not altered, ax, cx, dx are trashed.
	cCall	LN_LprgcpForPlc,<>
ifdef DEBUG
;    Assert (bx == bxSave);
	cmp	bx,[bxSave]
	je	IIP01
	push	ax
	push	bx
	push	cx
	push	dx
	push	es

	mov	ax,midFetchn
	mov	bx,1001
	cCall	AssertProcForNative,<ax,bx>

	pop	es
	pop	dx
	pop	cx
	pop	bx
	pop	ax
IIP01:
endif ;DEBUG

;#ifdef DEBUG
;	if (WinMac(!pplc->fMult, fTrue)
;		Assert(cp >= lprgcp[0] + DcpAdjust(pplc, 0));
;#endif /* DEBUG */
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	push	di
	push	es

	test	[bx.fMultPlc],maskFMultPlc
	jne	IIP02
	cmp	[vfCheckPlc],fFalse
	je	IIP02
	xor	cx,cx
;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	cCall	LN_CpFromPlcIcp,<>
	mov	bx,[OFF_cp]
	mov	cx,[SEG_cp]
	sub	bx,ax
	sbb	cx,dx
	jge	IIP02

	mov ax,midFetchn
	mov bx,1020		; label number for Assert box
	cCall AssertProcForNative,<ax,bx>

IIP02:
	pop	es
	pop	di
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

;	 return IcpInRgcpAdjusted(lprgcp, pplc, cp);
;}
;	LN_IcpInRgcpAdjusted expects lprgcp in es:di, pplc in bx, cp in dx:ax
;	bx and di are not altered.
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	cCall	LN_IcpInRgcpAdjusted,<>
; }
cEnd

; End of IInPlc


; /* I C P  I N  R G C P  A D J U S T E D */
; /* Binary search plc table for cp; return largest i s.t. rgcp[i] <= cp.
; note, search will not work for cp's less than rgcp[0].
; */
;-------------------------------------------------------------------------
;	IcpInRgcpAdjusted(lprgcp, pplc,cp)
;-------------------------------------------------------------------------
; IcpInRgcpAdjusted(lprgcp, pplc, cp)
; CP far *lprgcp;
; struct PLC *pplc;
; CP cp;
; {
;	 int icp, icpAdjustM1, icpAdjust, icpHint;
;	 CP cpTrans;

; %%Function:LN_IcpInRgcpAdjusted %%Owner:BRADV
cProc	LN_IcpInRgcpAdjusted,<PUBLIC,NEAR,ATOMIC>,<>
;	LN_IcpInRgcpAdjusted expects lprgcp in es:di, pplc in bx, cp in dx:ax
;	bx and di are not altered.

cBegin	 nogen
;	 Debug(vcIInPlcCalls++);
ifdef DEBUG
	inc	[vcIInPlcCalls]
endif ;DEBUG

;	 icpHint = pplc->icpHint;
	mov	cx,[bx.icpHintPlc]

;	 Assert(icpHint <= pplc->iMac);
ifdef DEBUG
	cmp	cx,[bx.iMacPlcSTR]
	jle	IIRA01
	push	ax
	push	bx
	push	cx
	push	dx
	push	es

	mov	ax,midFetchn
	mov	bx,1002
	cCall	AssertProcForNative,<ax,bx>

	pop	es
	pop	dx
	pop	cx
	pop	bx
	pop	ax
IIRA01:
endif ;DEBUG

;	if (pplc->iMac == 0)
;		return 0;
	cmp	[bx.iMacPlcSTR],0
	jnz	IIRA015
	xor	ax,ax
	jmp	IIRA10
IIRA015:

;	 if (lprgcp[icpHint] + DcpAdjust(pplc, icpHint) <= cp &&

;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	push	dx
	push	ax
	cCall	LN_CpFromPlcIcp,<>
	xchg	ax,si
	pop	ax
	cmp	ax,si
	pop	si
	push	si
	sbb	si,dx
	pop	dx
	jl	IIRA03

;		 cp < lprgcp[icpHint + 1] + DcpAdjust(pplc, icpHint + 1))
;		 {
	push	dx
	push	ax
	inc	cx
;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	cCall	LN_CpFromPlcIcp,<>
	dec	cx	    ;restore icpHint
	xchg	ax,si
	pop	ax
	cmp	ax,si
	pop	si
	push	si
	sbb	si,dx
	pop	dx
	jge	IIRA03

IIRA02:
;		 Debug(vcIInPlcHintSuccess++);
ifdef DEBUG
	inc	[vcIInPlcHintSuccess]
endif ;DEBUG

;		 goto LMult;
;		 }
	jmp	short LMult

IIRA03:
;	 cpTrans = cp - pplc->dcpAdjust;
;	 if ((icpAdjust = pplc->icpAdjust) > 0)
;		 {
	mov	cx,[bx.icpAdjustPlc]
	dec	cx
	mov	si,cx
	shl	si,1
	shl	si,1
	push	di	    ;save offset(lprgcp)
	add	di,si
	or	cx,cx
	jl	IIRA045

;		 if (cp <= lprgcp[icpAdjustM1 = icpAdjust - 1])
;			 {
	cmp	es:[di],ax
	mov	si,es:[di+2]
	sbb	si,dx
	jl	IIRA04
	pop	di	    ;restore offset(lprgcp)

;			 icp = IcpInRgcp(lprgcp, icpAdjustM1, cp);
;			 goto LMult;
;			 }
;	 LN_IcpInRgcp expects lprgcp in es:di, icpLim in cx, and cp in ax:dx.
;	 bx and di are not altered.
	cCall	LN_IcpInRgcp,<>
	xchg	ax,cx
	jmp	short LMult

IIRA04:
;		 if (cpTrans < lprgcp[icpAdjust])
;			 {
;			 icp = icpAdjustM1;
;			 goto LMult;
;			 }
	push	dx
	push	ax	;save cp
	sub	ax,[bx.LO_dcpAdjustPlc]
	sbb	dx,[bx.HI_dcpAdjustPlc]     ;compute cpTrans
	sub	ax,es:[di+4]
	sbb	dx,es:[di+6]
	pop	ax
	pop	dx	;restore cp
	jl	IIRA05

;		 }
IIRA045:
	add	di,4	    ;&lprgcp[icpAdjust - 1] ==> &lprgcp[icpAdjust]
	inc	cx	;icpAdjustM1 ==> icpAdjust
	sub	ax,[bx.LO_dcpAdjustPlc]
	sbb	dx,[bx.HI_dcpAdjustPlc]     ;compute cpTrans

;	 icp = IcpInRgcp(&lprgcp[icpAdjust], pplc->iMac - icpAdjust,
;			 cpTrans) + icpAdjust;
;	 LN_IcpInRgcp expects lprgcp in es:di, icpLim in cx, and cp in ax:dx.
;	 bx and di are not altered.
	push	cx	;save icpAdjust
	neg	cx
	add	cx,[bx.iMacPlcSTR]
	cCall	LN_IcpInRgcp,<>
	pop	cx	;restore icpAdjust
	add	cx,ax
IIRA05:
	pop	di	    ;restore offset(lprgcp)

;LMult:
;	 if (pplc->fMult)
;		 {
LMult:
	test	[bx.fMultPlc],maskFMultPlc
	je	IIRA08

;		 for (; icp > 0 && lprgcp[icp - 1] + DcpAdjust(pplc, icp - 1)
;			  == lprgcp[icp] + DcpAdjust(pplc, icp); icp--)
;		 }
;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	cCall	LN_CpFromPlcIcp,<>

IIRA06:
	jcxz	IIRA08
	push	dx
	push	ax
	dec	cx
;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	cCall	LN_CpFromPlcIcp,<>
	pop	si
	cmp	ax,si
	pop	si
	jne	IIRA07
	cmp	dx,si
	je	IIRA06

IIRA07:
	inc	cx

IIRA08:
	xchg	ax,cx

;	 Debug(if (icpHint + 1 == icp) vcIInPlcHintPlus++);
ifdef DEBUG
	push	cx
	mov	cx,[bx.icpHintPlc]
	inc	cx
	cmp	cx,ax
	jne	IIRA09
	inc	[vcIInPlcHintPlus]
IIRA09:
	pop	cx
endif ;DEBUG

;	 return (pplc->icpHint = icp);
	mov	[bx.icpHintPlc],ax

IIRA10:
;}
cEnd	nogen
	ret

; End of LN_IcpInRgcpAdjusted


;-------------------------------------------------------------------------
;	IInPlc2(hplc,cp,iFirst)
;-------------------------------------------------------------------------
;/* I  I N  P L C  2 */
;/* Binary search plc table for cp after iFirst; return index.
;note, search will not work for cp's less than rgcp[iFirst].
;*/
;native int IInPlc2(hplc, cp, iFirst)
;struct PLC **hplc;
;CP cp;
;int iFirst;
;{
;	 int icpAdjustM1, icpAdjust;
;	 CP cpTrans;
;	 struct PLC *pplc = *hplc;
;	 CP far *lprgcp = LprgcpForPlc(pplc);
;	 int iMac = pplc->iMac;

; %%Function:IInPlc2 %%Owner:BRADV
cProc	IInPlc2,<PUBLIC,FAR>,<si,di>
	ParmW	hplc
	ParmD	cp
	ParmW	iFirst

ifdef DEBUG
	LocalW	bxSave
endif ;DEBUG

cBegin
	mov	bx,[hplc]
	mov	bx,[bx]
ifdef DEBUG
	mov	[bxSave],bx
endif ;DEBUG
;	LN_LprgcpForPlc takes pplc in register bx,
;	and returns the result in registers es:di.
;	bx is not altered, ax, cx, dx are trashed.
	cCall	LN_LprgcpForPlc,<>
ifdef DEBUG
;    Assert (bx == bxSave);
	cmp	bx,[bxSave]
	je	IIP201
	push	ax
	push	bx
	push	cx
	push	dx
	push	es

	mov	ax,midFetchn
	mov	bx,1003
	cCall	AssertProcForNative,<ax,bx>

	pop	es
	pop	dx
	pop	cx
	pop	bx
	pop	ax
IIP201:
endif ;DEBUG

ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	push	di
	push	es

;	if (WinMac(!pplc->fMult, fTrue)
;		 Assert(!vfCheckPlc || cp >= lprgcp[iFirst] + DcpAdjust(pplc, iFirst));
	test	[bx.fMultPlc],maskFMultPlc
	jne	IIP2015
	cmp	[vfCheckPlc],fFalse
	je	IIP2015
	mov	cx,[iFirst]
;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	cCall	LN_CpFromPlcIcp,<>
	cmp	[OFF_cp],ax
	mov	cx,[SEG_cp]
	sbb	cx,dx
	jge	IIP2015

	mov ax,midFetchn
	mov bx,1021		; label number for Assert box
	cCall AssertProcForNative,<ax,bx>

IIP2015:
;	Assert(!vfCheckPlc || (iFirst <= (iMac = pplc->iMac)
;		&& cp <= lprgcp[iMac]+DcpAdjust(pplc,iMac)));
	cmp	[vfCheckPlc],fFalse
	je	IIP2025
	mov	cx,[bx.iMacPlcStr]
	cmp	[iFirst],cx
	jg	IIP202

;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	cCall	LN_CpFromPlcIcp,<>
	sub	ax,[OFF_cp]
	sbb	dx,[SEG_cp]
	jge	IIP2025

IIP202:
	mov ax,midFetchn
	mov bx,1030		; label number for Assert box
	cCall AssertProcForNative,<ax,bx>

IIP2025:
	pop	es
	pop	di
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
; 	if (iMac == 0 || cp == lprgcp[iMac] + DcpAdjust(pplc, iMac)) return iMac;
	mov	cx,[bx.iMacPlcSTR]
	or	cx,cx
	je	IIP2027

;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	push	dx
	push	ax
	cCall	LN_CpFromPlcIcp,<>
	xchg	ax,si
	pop	ax
	cmp	ax,si
	mov	si,dx
	pop	dx
	jne	IIP2027
	cmp	dx,si
IIP2027:
	je	IIP205

;	 icpAdjust = pplc->icpAdjust;
	mov	cx,[bx.icpAdjustPlc]

;	 Assert(icpAdjust <= pplc->iMac + 1);
ifdef DEBUG
	call	IIP212
endif ;DEBUG

;	 cpTrans = cp - pplc->dcpAdjust;
;	 if (icpAdjust > 0)
;		 {
	dec	cx
	mov	si,cx
	shl	si,1
	shl	si,1
	push	di	    ;save offset(lprgcp)
	add	di,si
	or	cx,cx
	jl	IIP2035

;		 if (cp <= lprgcp[icpAdjustM1 = icpAdjust - 1])
;			 {
	cmp	es:[di],ax
	mov	si,es:[di+2]
	sbb	si,dx
	jl	IIP203
	pop	di	    ;restore offset(lprgcp)

;			 icpAdjust = IcpInRgcp(&lprgcp[iFirst], icpAdjustM1 - iFirst, cp) + iFirst;
;			 goto LRet;
;			 }
;	 LN_IcpInRgcp expects lprgcp in es:di, icpLim in cx, and cp in ax:dx.
;	 bx and di are not altered.
	mov	si,[iFirst]
	sub	cx,si
	shl	si,1
	shl	si,1
	push	di	    ;save offset(lprgcp)
	add	di,si
	cCall	LN_IcpInRgcp,<>
	pop	di	    ;restore offset(lprgcp)
	add	ax,[iFirst]
	xchg	ax,cx
	jmp	short IIP2055

IIP203:
;		 if (cpTrans < lprgcp[icpAdjust])
;			 {
;			 icpAdjust = icpAdjustM1;
;			 goto LRet;
;			 }
;		 }
	push	dx
	push	ax	;save cp
	sub	ax,[bx.LO_dcpAdjustPlc]
	sbb	dx,[bx.HI_dcpAdjustPlc]     ;compute cpTrans
	sub	ax,es:[di+4]
	sbb	dx,es:[di+6]
	pop	ax
	pop	dx	;restore cp
	jl	IIP205

IIP2035:
	add	di,4	    ;&lprgcp[icpAdjust - 1] ==> &lprgcp[icpAdjust]
	inc	cx	;icpAdjustM1 ==> icpAdjust
	sub	ax,[bx.LO_dcpAdjustPlc]
	sbb	dx,[bx.HI_dcpAdjustPlc]     ;compute cpTrans

;	 if (iFirst > icpAdjust)
;		 icpAdjust = iFirst;
	mov	si,[iFirst]
	sub	si,cx
	jle	IIP204
	add	cx,si
	shl	si,1
	shl	si,1
	add	di,si

IIP204:

;	 icpAdjust = IcpInRgcp(&lprgcp[icpAdjust], iMac - icpAdjust,
;		 cpTrans) + icpAdjust;
;	 LN_IcpInRgcp expects lprgcp in es:di, icpLim in cx, and cp in ax:dx.
;	 bx and di are not altered.
	push	cx	;save icpAdjust
	neg	cx
	add	cx,[bx.iMacPlcSTR]
	cCall	LN_IcpInRgcp,<>
	pop	cx	;restore icpAdjust
	add	cx,ax

;	 if (pplc->fMult)
;		 {

IIP205:
	pop	di	    ;restore offset(lprgcp)

;LRet:
IIP2055:
	test	[bx.fMultPlc],maskFMultPlc
	je	IIP208

;		 for (; icp < iMac && lprgcp[icp] + DcpAdjust(pplc, icp)
;			  == lprgcp[icp + 1] + DcpAdjust(pplc, icp + 1); icp++)
;		 }
;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	cCall	LN_CpFromPlcIcp,<>
IIP206:
	cmp	cx,[bx.iMacPlcStr]
	jge	IIP208
	push	dx
	push	ax
	inc	cx
;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	cCall	LN_CpFromPlcIcp,<>
	pop	si
	cmp	ax,si
	pop	si
	jne	IIP207
	cmp	dx,si
	je	IIP206
IIP207:
	dec	cx
IIP208:

;	 return(icpAdjust);
	xchg	ax,cx

;}
cEnd


;	 Assert(icpAdjust <= iMac + 1);
ifdef DEBUG
IIP212:
	push	ax
	push	bx
	push	cx
	push	dx
	push	es
	mov	dx,[bx.iMacPlcStr]
	inc	dx
	cmp	cx,dx
	jle	IIP213

	mov	ax,midFetchn
	mov	bx,1052
	cCall	AssertProcForNative,<ax,bx>

IIP213:
	pop	es
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;DEBUG

; End of IInPlc2


;-------------------------------------------------------------------------
;	IInPlcCheck(hplc,cp)
;-------------------------------------------------------------------------
;/* I  I N  P L C  C H E C K */
;/* Binary search plc table for cp after iFirst; return index.
;note, search will not work for cp's less than rgcp[iFirst].
;*/
;native int IInPlcCheck(hplc, cp)
;struct PLC **hplc;
;CP cp;
;{
;	 int icpAdjustM1, icpAdjust;
;	 CP cpTrans;
;	 struct PLC *pplc = *hplc;
;	 CP far *lprgcp = LprgcpForPlc(pplc);
;	 int iMac = pplc->iMac;

; %%Function:IInPlcCheck %%Owner:BRADV
cProc	IInPlcCheck,<PUBLIC,FAR>,<si,di>
	ParmW	hplc
	ParmD	cp

ifdef DEBUG
	LocalW	bxSave
endif ;DEBUG

cBegin
	mov	bx,[hplc]
	mov	bx,[bx]
ifdef DEBUG
	mov	[bxSave],bx
endif ;DEBUG
;	LN_LprgcpForPlc takes pplc in register bx,
;	and returns the result in registers es:di.
;	bx is not altered, ax, cx, dx are trashed.
	cCall	LN_LprgcpForPlc,<>
ifdef DEBUG
;    Assert (bx == bxSave);
	cmp	bx,[bxSave]
	je	IIPC01
	push	ax
	push	bx
	push	cx
	push	dx
	push	es

	mov	ax,midFetchn
	mov	bx,1004
	cCall	AssertProcForNative,<ax,bx>

	pop	es
	pop	dx
	pop	cx
	pop	bx
	pop	ax
IIPC01:
endif ;DEBUG

;	 if (cp < lprgcp[0] + DcpAdjust(pplc, 0) || cp >= lprgcp[iMac] + DcpAdjust(pplc, iMac))
;		 return (-1);
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	xor	cx,cx
;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	push	dx
	push	ax
	cCall	LN_CpFromPlcIcp,<>
	xchg	ax,si
	pop	ax
	cmp	ax,si
	pop	si
	push	si
	sbb	si,dx
	pop	dx
	jl	IIPC02
	mov	cx,[bx.iMacPlcSTR]
;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	push	dx
	push	ax
	cCall	LN_CpFromPlcIcp,<>
	xchg	ax,si
	pop	ax
	cmp	ax,si
	pop	si
	push	si
	sbb	si,dx
	pop	dx
	jl	IIPC03
IIPC02:
	mov	ax,-1
	jmp	short IIPC04

IIPC03:
;	 return (IcpInRgcpAdjusted(lprgcp, pplc, cp));
;	LN_IcpInRgcpAdjusted expects lprgcp in es:di, pplc in bx, cp in dx:ax
;	bx and di are not altered.
	cCall	LN_IcpInRgcpAdjusted,<>

IIPC04:
;}
cEnd

; End of IInPlcCheck


;-------------------------------------------------------------------------
;	IInPlcRef(hplc,cpFirst)
;-------------------------------------------------------------------------
;/* I  I N  P L C  R E F  */
;/* returns the smallest i s.t. rgcp[i] >= cp. returns -1 if cp > rgcp[iMac]
;*/
;native int IInPlcRef(hplc, cpFirst)
;struct PLC **hplc;
;CP cpFirst;
;{
;	 struct PLC *pplc = *hplc;
;	 CP far *lprgcp = LprgcpForPlc(pplc);
;	 int i;

; %%Function:IInPlcRef %%Owner:BRADV
cProc	IInPlcRef,<PUBLIC,FAR>,<si,di>
	ParmW	hplc
	ParmD	cpFirst

ifdef DEBUG
	LocalW	bxSave
endif ;DEBUG

cBegin
	mov	bx,[hplc]
	mov	bx,[bx]
ifdef DEBUG
	mov	[bxSave],bx
endif ;DEBUG
;	LN_LprgcpForPlc takes pplc in register bx,
;	and returns the result in registers es:di.
;	bx is not altered, ax, cx, dx are trashed.
	cCall	LN_LprgcpForPlc,<>
ifdef DEBUG
;    Assert (bx == bxSave);
	cmp	bx,[bxSave]
	je	IIPR01
	push	ax
	push	bx
	push	cx
	push	dx
	push	es

	mov	ax,midFetchn
	mov	bx,1005
	cCall	AssertProcForNative,<ax,bx>

	pop	es
	pop	dx
	pop	cx
	pop	bx
	pop	ax
IIPR01:
endif ;DEBUG

;	 if (lprgcp[0] + DcpAdjust(pplc, 0) >= cpFirst) return 0;
	xor	cx,cx
;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	cCall	LN_CpFromPlcIcp,<>
	sub	ax,[OFF_cpFirst]
	sbb	dx,[SEG_cpFirst]
	mov	ax,cx
	jge	IIPR05

;	 if (lprgcp[pplc->iMac] + DcpAdjust(pplc, pplc->iMac) < cpFirst) return -1;
	mov	cx,[bx.iMacPlcSTR]
;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	cCall	LN_CpFromPlcIcp,<>
	sub	ax,[OFF_cpFirst]
	sbb	dx,[SEG_cpFirst]
	mov	ax,-1
	jl	IIPR05

;	 i = IInPlc(hplc, cpFirst);
;	 begin in-line coding of IInPlc ***
;	 Assert(!vfCheckPlc || cp >= lprgcp[0] + DcpAdjust(pplc, 0));
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	push	di
	push	es

	cmp	[vfCheckPlc],fFalse
	je	IIPR02
	xor	cx,cx
;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	cCall	LN_CpFromPlcIcp,<>
	mov	bx,[OFF_cpFirst]
	mov	cx,[SEG_cpFirst]
	sub	bx,ax
	sbb	cx,dx
	jge	IIPR02

	mov ax,midFetchn
	mov bx,1022		; label number for Assert box
	cCall AssertProcForNative,<ax,bx>

IIPR02:
	pop	es
	pop	di
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG

;	 return IcpInRgcpAdjusted(lprgcp, pplc, cp);
;}
;	LN_IcpInRgcpAdjusted expects lprgcp in es:di, pplc in bx, cp in dx:ax
;	bx and di are not altered.
	mov	ax,[OFF_cpFirst]
	mov	dx,[SEG_cpFirst]
	cCall	LN_IcpInRgcpAdjusted,<>
ifdef DEBUG
;    Assert (bx == bxSave);
	cmp	bx,[bxSave]
	je	IIPR03
	push	ax
	push	bx
	push	cx
	push	dx
	push	es

	mov	ax,midFetchn
	mov	bx,1006
	cCall	AssertProcForNative,<ax,bx>

	pop	es
	pop	dx
	pop	cx
	pop	bx
	pop	ax
IIPR03:
endif ;DEBUG
;	 end in-line coding of IInPlc ***

;	 while (lprgcp[i] + DcpAdjust(pplc, i) < cpFirst)
;		 i++;
	xchg	ax,cx
	dec	cx
IIPR04:
	inc	cx
;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	cCall	LN_CpFromPlcIcp,<>
	sub	ax,[OFF_cpFirst]
	sbb	dx,[SEG_cpFirst]
	jl	IIPR04

;	 return i;
	xchg	ax,cx
IIPR05:
;}
cEnd

; End of IInPlcRef


ifdef NOTUSED
;-------------------------------------------------------------------------
;	IInPlcMult(hplc,cp)
;-------------------------------------------------------------------------
;/* I  I N  P L C  M U L T */
;/* Binary search plc table for cp; return largest i s.t. rgcp[i] <= cp.
;note, search will not work for cp's less than rgcp[0].
;This one is specifically for plc's which may have consecutive equal cp's */
;native int IInPlcMult(hplc, cp)
;struct PLC **hplc;
;CP cp;
;{
;	 struct PLC *pplc = *hplc;
;	 CP far *lprgcp = LprgcpForPlc(pplc);
;	 int icp;

; %%Function:IInPlcMult %%Owner:BRADV
cProc	IInPlcMult,<PUBLIC,FAR>,<si,di>
	ParmW	hplc
	ParmD	cp

ifdef DEBUG
	LocalW	bxSave
endif ;DEBUG

cBegin
	mov	bx,[hplc]
	mov	bx,[bx]
ifdef DEBUG
	mov	[bxSave],bx
endif ;DEBUG
;	LN_LprgcpForPlc takes pplc in register bx,
;	and returns the result in registers es:di.
;	bx is not altered, ax, cx, dx are trashed.
	cCall	LN_LprgcpForPlc,<>
ifdef DEBUG
;    Assert (bx == bxSave);
	cmp	bx,[bxSave]
	je	IIPM01
	push	ax
	push	bx
	push	cx
	push	dx
	push	es

	mov	ax,midFetchn
	mov	bx,1007
	cCall	AssertProcForNative,<ax,bx>

	pop	es
	pop	dx
	pop	cx
	pop	bx
	pop	ax
IIPM01:
endif ;DEBUG

;	if (lprgcp[0] + DcpAdjust(pplc, 0) >= cp)
;		return 0;

	xor	cx,cx
;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	cCall	LN_CpFromPlcIcp,<>
	sub	ax,[OFF_cp]
	sbb	dx,[SEG_cp]
	jge	IIPM06

;	 icp = IInPlc(hplc, cp);
;	 begin in-line coding of IInPlc ***

; (BL): Cannot Assert this: may not be true for pgd/pad/phe plc's
; when they are in a bogus state, which they are allowed 
;	 Assert(!vfCheckPlc || cp >= lprgcp[0] + DcpAdjust(pplc, 0));
ifdef BOGUS
	push	ax
	push	bx
	push	cx
	push	dx
	push	di
	push	es

	cmp	[vfCheckPlc],fFalse
	je	IIPM02
	xor	cx,cx
;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	cCall	LN_CpFromPlcIcp,<>
	mov	bx,[OFF_cp]
	mov	cx,[SEG_cp]
	sub	bx,ax
	sbb	cx,dx
	jge	IIPM02

	mov ax,midFetchn
	mov bx,1023		; label number for Assert box
	cCall AssertProcForNative,<ax,bx>

IIPM02:
	pop	es
	pop	di
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;BOGUS

;	 return IcpInRgcpAdjusted(lprgcp, pplc, cp);
;}
;	LN_IcpInRgcpAdjusted expects lprgcp in es:di, pplc in bx, cp in dx:ax
;	bx and di are not altered.
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
	cCall	LN_IcpInRgcpAdjusted,<>
ifdef DEBUG
;    Assert (bx == bxSave);
	cmp	bx,[bxSave]
	je	IIPM03
	push	ax
	push	bx
	push	cx
	push	dx
	push	es

	mov	ax,midFetchn
	mov	bx,1008
	cCall	AssertProcForNative,<ax,bx>

	pop	es
	pop	dx
	pop	cx
	pop	bx
	pop	ax
IIPM03:
endif ;DEBUG
;	 end in-line coding of IInPlc ***

;	 for (; icp > 0 && lprgcp[icp - 1] + DcpAdjust(pplc, icp - 1)== lprgcp[icp] + DcpAdjust(pplc, icp); icp--)
;		 ;
	xchg	ax,cx
;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	cCall	LN_CpFromPlcIcp,<>

IIPM04:
	jcxz	IIPM06
	push	dx
	push	ax
	dec	cx
;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	cCall	LN_CpFromPlcIcp,<>
	pop	si
	cmp	ax,si
	pop	si
	jne	IIPM05
	cmp	dx,si
	je	IIPM04

IIPM05:
	inc	cx

IIPM06:
	xchg	ax,cx
;	 return(icp);
;}
cEnd

; End of IInPlcMult
endif ;NOTUSED


ifdef NOTUSED
;-------------------------------------------------------------------------
;	IInPlc2Mult(hplc,cp,iFirst)
;-------------------------------------------------------------------------
;/* I  I N  P L C  2  M U L T */
;/* Binary search plc table for cp after iFirst; return index.
;note, search will not work for cp's less than rgcp[iFirst].
;This one is specifically for plc's which may have consecutive equal cp's */
;native int IInPlc2Mult(hplc, cp, iFirst)
;struct PLC **hplc;
;CP cp;
;{
;	 int icpAdjust;
;	 struct PLC *pplc = *hplc;
;	 CP far *lprgcp = LprgcpForPlc(pplc);
;	 int icp, iMac = pplc->iMac;

; %%Function:IInPlc2Mult %%Owner:BRADV
cProc	IInPlc2Mult,<PUBLIC,FAR>,<si,di>
;	NOTE: these paramaters must be exactly the same as used by
;	IInPlc2
	ParmW	hplc
	ParmD	cp
	ParmW	iFirst

ifdef DEBUG
	LocalW	bxSave
endif ;DEBUG

cBegin
	mov	bx,[hplc]
	mov	bx,[bx]
ifdef DEBUG
	mov	[bxSave],bx
endif ;DEBUG
;	LN_LprgcpForPlc takes pplc in register bx,
;	and returns the result in registers es:di.
;	bx is not altered, ax, cx, dx are trashed.
	cCall	LN_LprgcpForPlc,<>
ifdef DEBUG
;    Assert (bx == bxSave);
	cmp	bx,[bxSave]
	je	IIP2M01
	push	ax
	push	bx
	push	cx
	push	dx
	push	es

	mov	ax,midFetchn
	mov	bx,1009
	cCall	AssertProcForNative,<ax,bx>

	pop	es
	pop	dx
	pop	cx
	pop	bx
	pop	ax
IIP2M01:
endif ;DEBUG

;	 if (cp <= lprgcp[iFirst] + DcpAdjust(pplc, iFirst))
;		 return iFirst;
	mov	cx,[iFirst]
;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	cCall	LN_CpFromPlcIcp,<>
	sub	ax,[OFF_cp]
	sbb	dx,[SEG_cp]
	jge	IIP2M04

;	 icp = IInPlc2(hplc, cp, iFirst);
	push	di
;	LN_IInPlc2 takes lprgcp in es:di, pplc in register bx,
;	and returns the result in register ax.
;	dx:ax, cx, si and di are altered.
	cCall	LN_IInPlc2,<>
	pop	di

;	 for (; icp < iMac && lprgcp[icp] + DcpAdjust(pplc, icp) == lprgcp[icp + 1] + DcpAdjust(pplc, icp+ 1); icp++)
;		 ;
	xchg	ax,cx
;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	cCall	LN_CpFromPlcIcp,<>

IIP2M02:
	cmp	cx,[bx.iMacPlcSTR]
	jge	IIP2M04

	push	dx
	push	ax
	inc	cx
;	LN_CpFromPlcIcp takes lprgcp in es:di, pplc in register bx,
;	icp in register cx, and returns the result in registers dx:ax.
;	Only dx:ax are altered.
	cCall	LN_CpFromPlcIcp,<>
	pop	si
	cmp	ax,si
	pop	si
	jne	IIP2M03
	cmp	dx,si
	je	IIP2M02

IIP2M03:
	dec	cx

IIP2M04:
	xchg	ax,cx
;	 return(icp);
;}
cEnd

; End of IInPlc2Mult
endif ;NOTUSED


ifdef NOTUSED
;-------------------------------------------------------------------------
;	IInPlcQuick(hplc,cp)
;-------------------------------------------------------------------------
;/* I	I N   P L C   Q U I C K */
;int IInPlcQuick(hplc, cp)
;struct PLC **hplc;
;CP cp;
;{
;	 struct PLC *pplc = *hplc;
;	 CP far *lprgcp = LprgcpForPlc(pplc);
;	 int iMac = pplc->iMac;

; %%Function:IInPlcQuick %%Owner:BRADV
cProc	IInPlcQuick,<PUBLIC,FAR>,<si,di>
	ParmW	hplc
	ParmD	cp

ifdef DEBUG
	LocalW	bxSave
endif ;DEBUG

cBegin
	mov	bx,[hplc]
	mov	bx,[bx]
ifdef DEBUG
	mov	[bxSave],bx
endif ;DEBUG
;	LN_LprgcpForPlc takes pplc in register bx,
;	and returns the result in registers es:di.
;	bx is not altered, ax, cx, dx are trashed.
	cCall	LN_LprgcpForPlc,<>
ifdef DEBUG
;    Assert (bx == bxSave);
	cmp	bx,[bxSave]
	je	IIPQ01
	push	ax
	push	bx
	push	cx
	push	dx
	push	es

	mov	ax,midFetchn
	mov	bx,1010
	cCall	AssertProcForNative,<ax,bx>

	pop	es
	pop	dx
	pop	cx
	pop	bx
	pop	ax
IIPQ01:
endif ;DEBUG

;	 return (iMac == 0 || cp < lprgcp[0] ? -1 : IcpInRgcp(lprgcp, iMac, cp));
	mov	ax,-1
	mov	cx,[bx.iMacPlcSTR]
	jcxz	IIPQ02
	mov	si,[OFF_cp]
	mov	dx,[SEG_cp]
	push	si
	sub	si,es:[di]
	mov	si,dx
	sbb	si,es:[di+2]
	pop	si
	jl	IIPQ02
	xchg	ax,si

;	 LN_IcpInRgcp expects lprgcp in es:di, icpLim in cx, and cp in ax:dx.
;	 bx and di are not altered.
	cCall	LN_IcpInRgcp,<>

IIPQ02:
;}
cEnd

; End of IInPlcQuick
endif ;NOTUSED


;-------------------------------------------------------------------------
;	IcpInRgcp(rgcp,icpLim,cp)
;-------------------------------------------------------------------------
;/* I C P  I N	R G C P */
;/* note, search will not work for cp's less than rgcp[0] */
;native int IcpInRgcp(lprgcp, icpLim, cp)
;CP far *lprgcp;
;CP cp; int icpLim;
;{
;	 int icpMin = 0;

; %%Function:LN_IcpInRgcp %%Owner:BRADV
cProc	LN_IcpInRgcp,<PUBLIC,NEAR,ATOMIC>,<>
;	 LN_IcpInRgcp expects lprgcp in es:di, icpLim in cx, and cp in ax:dx.
;	 bx and di are not altered.

cBegin	nogen
;	 Assert(!vfCheckPlc || cp <= lprgcp[icpLim]);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	push	di
	push	es
	cmp	vfCheckPlc,fFalse
	je	IIR01
	shl	cx,1
	shl	cx,1
	add	di,cx
	mov	bx,es:[di]
	mov	cx,es:[di+2]
	sub	bx,ax
	sbb	cx,dx
	jge	IIR01

	mov ax,midFetchn
	mov bx,1024		; label number for Assert box
	cCall AssertProcForNative,<ax,bx>

IIR01:
	pop	es
	pop	di
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;DEBUG
	push	bx

	xor	si,si

;	 icpLim++;
	inc	cx
	shl	cx,1
	shl	cx,1
	jmp	short IIR04

IIR02:
	mov	si,bx
	mov	bx,cx
IIR03:
	mov	cx,bx
IIR04:

;	 while (icpMin + 1 < icpLim)
;		 {
	mov	bx,si
	add	bx,4
	cmp	bx,cx
	jae	IIR05

;		 int icpGuess = (icpMin + icpLim) >> 1;
	mov	bx,si
	add	bx,cx
	rcr	bx,1
	and	bl,0FCh

;		 if (lprgcp[icpGuess] <= cp)
;			 icpMin = icpGuess;
;		 else
;			 icpLim = icpGuess;
;		 }
	cmp	es:[bx+di+2],dx
	jl	IIR02
	jg	IIR03
	cmp	es:[bx+di],ax
	jbe	IIR02
	jmp	short IIR03

IIR05:
;	 return icpMin;
	xchg	ax,si
	shr	ax,1
	shr	ax,1

	pop	bx
;}
cEnd	nogen
	ret

; End of LN_IcpInRgcp


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
