;   FORMULAN.ASM
;   Fetch routines used by Formula




        .xlist
        memS    = 1
        ?WIN    = 1
        ?PLM    = 1
        ?NODATA = 1
        ?TF     = 1
        include w2.inc
        include noxport.inc
        include consts.inc
        include structs.inc
        .list

; CODE IN THIS MODULE IS APPENDED TO THE PCODE SEGMENT FOR FORMULA.C

createSeg       formula_PCODE,formula,byte,public,CODE


; EXPORTED LABELS

; EXTERNAL FUNCTIONS

externFP	<HpsAlter,ApplySprm>
externFP	<AnsiLower,MapStcStandard>
externFP	<IMacPlc,CpPlc,GetPlc,CpMac2Doc,PutPlc>
externFP	<PutPlcLastProc>
externFP	<ReloadSb>
externFP	<IInPlc,IInPlcRef>
externFP	<N_FetchCp,N_PdodDoc>
externNP	<LN_IcpInRgcp,LN_LprgcpForPlc>

ifdef DEBUG
externFP	<AssertProcForNative,ScribbleProc>
externFP	<S_FetchCp>
externFP	<PutPlcLastDebugProc>
endif


; EXTERNAL DATA

sBegin	data

externW vfEndFetch	; extern BOOL		  vfEndFetch;
externW mpdochdod	; extern struct DOD	  **mpdochdod[];
externW vchpFetch	; extern struct CHP	  vchpFetch;
externW vchpStc 	; extern struct CHP	  vchpStc;
externW vhpchFetch	; extern char HUGE	  *vhpchFetch;
externW vccpFetch	; extern int		  vccpFetch;
externW mpfnhfcb	; extern struct FCB	  **mpfnhfcb[];
externW caPara		; extern struct CA	  caPara;
externW caSect		; extern struct CA	  caSect;
externW vsab		; extern struct SAB	  vsab;
externW vpapFetch	; extern struct PAP	  vpapFetch;
externW vpapStc 	; extern struct PAP	  vpapStc;
externW vibp		; extern int		  vibp;
externW vbptbExt	; extern struct BPTB	  vbptbExt;
externW vstcpMapStc	; extern int		  vstcpMapStc;
externW vdocPapLast	; extern int		vdocPapLast;
externW vfnPapLast	; extern int		vfnPapLast;
externW vpnPapLast	; extern int		vpnPapLast;
externW vbpapxPapLast	; extern int		vbpapxPapLast;
externW mpsbps		; extern SB		mpsbps[];
externW vfnPreload	; extern int		vfnPreload;

ifdef DEBUG
externW 		cHpFreeze
externW 		vdbs
externW 		wFillBlock
endif

sEnd	data

sBegin      formula
        assumes cs,formula
        assumes ds,dgroup
        assumes ss,dgroup



;
;-------------------------------------------------------------------------
;	ChFetchNonVanish(doc, pcp)					GregC
;-------------------------------------------------------------------------
; %%Function:ChFetchNonVanish %%Owner:BRADV
cProc	ChFetchNonVanish,<PUBLIC,FAR>,<si,di>
	ParmW	doc
	ParmW	pcp
; /* C H  F E T C H  N O N  V A N I S H
;
; Return one character from document skipping vanished runs.
;   WARNING: NOT field sensitive.
; */
;
; char ChFetchNonVanish(doc, pcp)
; int doc;
; CP *pcp;
; {
cBegin
;	  CP cp;
;
;	  cp = *pcp;
	mov	bx,[pcp]
	mov	si,[bx]
	mov	di,[bx + 2]
;	  for (;;)
ChFetch2:
;		  {
;		  FetchCp(doc, cp, fcmChars | fcmProps);
	mov	ax,fcmChars OR fcmProps
ifdef DEBUG
	cCall	S_FetchCp,<[doc],di,si,ax>
else ;not DEBUG
	cCall	N_FetchCp,<[doc],di,si,ax>
endif ;DEBUG
;		  if (vchpFetch.fVanish /*|| vchpFetch.fFldVanish*/)
;			{
;			 cp += vccpFetch;
;			 if (cp >= caPara.cpLim - ccpEop)
;			 	{
;				 *pcp = caPara.cpLim - ccpEop;
;				 return (0); /* cause error */
;				}
;			 }
;		  else
;			  break;
	test	[vchpFetch.FVanishChp],maskFVanishChp
;	jnz	ChFetch7
;	test	[vchpFetch.FFldVanishChp],maskFFldVanishChp
	jz	ChFetch8
ChFetch7:
	add	si,[vccpFetch]
	adc	di,0
	mov 	ax,[caPara.LO_cpLimCa]
	mov	dx,[caPara.HI_cpLimCa]
	sub	ax,ccpEop
	sbb	dx,0
	cmp	dx,di
	ja	ChFetch2
	jb	ChFetch71
	cmp	ax,si
	ja	ChFetch2
ChFetch71:
	mov	bx,[pcp]
	mov	[bx],ax
	mov	[bx + 2],dx
	xor	ax,ax
	jmp	ChFetch10
;		  }
ChFetch8:
;	  *pcp = cp;
	mov	bx,[pcp]
	mov	[bx],si
	mov	[bx + 2],di
;
;	  return (vfEndFetch ? 0 : *vhpchFetch);
	xor	ax,ax
	cmp	[vfEndFetch],0
	jnz	ChFetch10
	mov	bx,whi (vhpchFetch)
	shl	bx,1
	mov	ax,[bx.mpsbps]
	mov	es,ax
	shr	ax,1
	jc	ChFetch9
;	ReloadSb trashes ax, cx, and dx
	cCall	ReloadSb,<>
ChFetch9:
ifdef DEBUG
	mov	ax,[wFillBlock]
	mov	cx,[wFillBlock]
	mov	dx,[wFillBlock]
endif ;DEBUG
	mov	si,wlo (vhpchFetch)
	xor	ax,ax
;	lodsb
	lods	byte ptr es:[si]

ChFetch10:
; }
cEnd



;
;-------------------------------------------------------------------------
;	CpFirstNonBlank(doc, cp)				GregC
;-------------------------------------------------------------------------
; %%Function:CpFirstNonBlank %%Owner:BRADV
cProc	CpFirstNonBlank,<PUBLIC,FAR>,<si,di>
	ParmW	doc
	ParmD	cp
; /* C P  F I R S T  N O N  B L A N K
;
;     Return the cp of the first non blank (space and tab are considered blank)
;     character after the given cp.
; */
;
; CP CpFirstNonBlank(doc, cp)
; int doc;
; CP cp;
; {
cBegin
;	  int ch;
;
;	  while ((ch = ChFetchNonVanish(doc, &cp)) == ' ' || ch == chTab)
;		  cp++;
	mov	si,[doc]
	lea	di,[cp]
	jmp	short CpFirstNonBlank4

CpFirstNonBlank2:
	add	[OFF_cp],1
	adc	[SEG_cp],0

CpFirstNonBlank4:
	push	si
	push	di
	call	far ptr ChFetchNonVanish
	cmp	al,' '
	je	CpFirstNonBlank2
	cmp	al,chTab
	je	CpFirstNonBlank2
;
;	  return cp;
	mov	ax,[OFF_cp]
	mov	dx,[SEG_cp]
; }
cEnd


sEnd	formula
    	end
