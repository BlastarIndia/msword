        include w2.inc
        include noxport.inc
        include consts.inc
        include structs.inc

createSeg       fieldfmt_PCODE,fieldfmt,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midFieldfmn       equ 4           ; module ID, for native asserts
endif ;DEBUG

; EXPORTED LABELS


; EXTERNAL FUNCTIONS

externFP	<ReloadSb>
externFP	<CpPlc>
externFP	<PutPlc>
externFP	<CpMacDoc>
externFP	<FltParseDocCp>
externFP	<EfltFromFlt>
externFP	<FfcFormatDisplay>
externFP	<IInPlcCheck>
externFP	<GetPlc>
externFP	<IMacPlc>
externFP	<N_IfldFromDocCp>
externFP	<N_QcpQfooPlcfoo>

ifdef DEBUG
externFP        <AssertProcForNative,ScribbleProc>
externFP    	<FCheckHandle>
externFP	<S_DcpSkipFieldChPflcd>
externFP	<S_FShowResultPflcdFvc>
externFP	<S_IfldFromDocCp>
externFP	<S_FillIfldFlcd>
endif ;DEBUG

ifdef DFORMULA
externFP	<CommSzRgNum>
endif ;DFORMULA


; EXTERNAL DATA

sBegin  data

externW mpsbps		;extern SB	     mpsbps[];
externW wwCur		;extern int          wwCur;
externW mpdochdod	;extern struct DOD **mpdochdod[];
externW mpwwhwwd	;extern struct WWD **mpwwhwwd[];

ifdef DEBUG
externW vdbs
externW vfCheckPlc
externW vfSeeAllFieldCps
externW wFillBlock
externW vsccAbove
externW vpdrfHead
endif ;DEBUG

sEnd    data


; CODE SEGMENT fieldfmt_PCODE

sBegin  fieldfmt
	assumes cs,fieldfmt
        assumes ds,dgroup
	assume es:nothing
	assume ss:nothing


include	asserth.asm



;/* F O R M A T  F U N C T I O N S */



;-------------------------------------------------------------------------
;	FfcFormatFieldPdcp (pdcp, ww, doc, cp, ch)
;-------------------------------------------------------------------------
;/* F F C  F O R M A T  F I E L D  P D C P */
;/*  This function is called from FormatLine() to format a field.
;    Returns:
;        ffcSkip (skip *pdcp characters, no other processing)
;        ffcShow (keep formatting normally, *pdcp == 0)
;        ffcDisplay (create CHRDF, then skip *pdcp chars)
;        ffcBeginGroup (create CHRFG, skip *pdcp chars)
;        ffcEndGroup (terminate CHRFG, skip *pdcp chars)
;*/
;
;native FfcFormatFieldPdcp (pdcp, ww, doc, cp, ch)
;CP *pdcp;
;int ww, doc;
;CP cp;
;CHAR ch;
;
;{
;
;
;    int flt;
;    int ifld;
;    BOOL ffcReturn = ffcSkip;
;    BOOL fResult;
;    struct PLC **hplcfld = PdodDoc (doc)->hplcfld;
;    struct FLCD flcd;

; %%Function:N_FfcFormatFieldPdcp %%Owner:BRADV
cProc	N_FfcFormatFieldPdcp,<PUBLIC,FAR>,<si,di>
	ParmW	pdcp
	ParmW	ww
	ParmW	doc
	ParmD	cp
	ParmW	chArg

	LocalV	flcd,cbFlcdMin
ifdef DFORMULA
	LocalV	rgw,10
	LocalV	szFrame,80
endif ;/* DFORMULA */

;	flt kept in si
;	ffcReturn kept in ax
;	ifld kept in di
	
cBegin

;    Assert (ch & 0xFF00 == 0);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	test	chArg,0FF00h
	je	FFFP005

	mov	ax,midFieldfmn
	mov	bx,1004
	cCall	AssertProcForNative,<ax,bx>
FFFP005:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;    Assert (doc != docNil && cp < CpMacDoc (doc) && cp >= cp0);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	doc,docNil
	je	FFFP01
	cCall	CpMacDoc,<doc>
	mov	bx,[OFF_cp]
	mov	cx,[SEG_cp]
	sub	bx,ax
	sbb	cx,dx
	jge	FFFP01
	cmp	[SEG_cp],0
	jge	FFFP02
FFFP01:
	mov	ax,midFieldfmn
	mov	bx,1005
	cCall	AssertProcForNative,<ax,bx>
FFFP02:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;#ifdef DEBUG
;    if (vfSeeAllFieldCps)

ifdef DEBUG
	cmp	[vfSeeAllFieldCps],fFalse
	je	FFFP03

;        {
;        *pdcp = 0;

	xor	ax,ax
	mov	bx,[pdcp]
	mov	[bx],ax
	mov	[bx+2],ax

;        return ffcShow;
	mov	ax,ffcShow
	jmp	FFFP23

;        }
;#endif /* DEBUG */
FFFP03:
endif ;/* DEBUG */

;    Scribble (ispFieldFormat1, 'F');
ifdef DEBUG
	cmp	vdbs.grpfScribbleDbs,0
	je	FFFP04
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,ispFieldFormat1
	mov	bx,'F'
	cCall	ScribbleProc,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FFFP04:
endif ;/* DEBUG */
    
;    if ((ifld = IfldFromDocCp (doc, cp, fTrue)) != ifldNil)
;	 FillIfldFlcd (PdodDoc(doc)->hplcfld, ifld, &flcd);
	mov	ax,fTrue
	push	[doc]
	push	[SEG_cp]
	push	[OFF_cp]
	push	ax
ifdef DEBUG
	cCall	S_IfldFromDocCp,<>
else ;not DEBUG
	cCall	N_IfldFromDocCp,<>
endif ;DEBUG
	cmp	ax,ifldNil
	xchg	ax,di
	je	FFFP06

	mov	bx,[doc]
	;Takes doc in bx, result in bx.  Only bx is altered.
	call	LN_PdodDoc
	push	[bx.hplcfldDod]
	lea	bx,[flcd]
	push	di
	push	bx
ifdef DEBUG
	cCall	S_FillIfldFlcd,<>
else ;not DEBUG
	push	cs
	call	near ptr N_FillIfldFlcd
endif ;DEBUG
	mov	ax,flcd.LO_dcpInstFlcd
	mov	dx,flcd.HI_dcpInstFlcd
FFFP06:

;    flt = flcd.flt;
	mov	si,[flcd.fltFlcd]

;    if ((ifld == ifldNil || flcd.fDirty) && (ch == chFieldBegin))

	cmp	di,ifldNil
	je	FFFP07
	cmp	[flcd.fDirtyFlcd],fFalse
	je	FFFP09
FFFP07:
	cmp	bptr ([chArg]),chFieldBegin
	jne	FFFP09

;        /* field is dead or must be parsed, try to parse it */
;        {
;	 flt = FltParseDocCp (doc, cp, ifld, ww==wwCur /* fChgView */, fFalse /* fEnglish */);
	xor	ax,ax
	cwd
	mov	cx,[ww]
	cmp	cx,[wwCur]
	jne	FFFP08
	inc	ax
FFFP08:
	cCall	FltParseDocCp,<[doc], [SEG_cp], [OFF_cp], di, ax, dx>
	xchg	ax,si

;        if ((ifld = IfldFromDocCp (doc, cp, fTrue)) != ifldNil)
	mov	ax,fTrue
	push	doc
	push	[SEG_cp]
	push	[OFF_cp]
	push	ax
ifdef DEBUG
	cCall	S_IfldFromDocCp,<>
else ;not DEBUG
	cCall	N_IfldFromDocCp,<>
endif ;DEBUG
	cmp	ax,ifldNil
	xchg	ax,di
	je	FFFP09

;		/* Note: FltParseDocCp may initialize hplcfld, so we need
;		   to do another PdodDoc(doc) here */
;	     FillIfldFlcd (PdodDoc(doc)->hplcfld, ifld, &flcd);
	mov	bx,[doc]
	;Takes doc in bx, result in bx.  Only bx is altered.
	call	LN_PdodDoc
	push	[bx.hplcfldDod]
	lea	bx,[flcd]
	push	di
	push	bx
ifdef DEBUG
	cCall	S_FillIfldFlcd,<>
else ;not DEBUG
	push	cs
	call	near ptr N_FillIfldFlcd
endif ;DEBUG

;        }
FFFP09:

;    if (ifld == ifldNil)
	cmp	di,ifldNil
	jne	FFFP12

;        {  /* dead field */
;	 /*  handles the case where the parse call caused the field to be
;	     vanished. */
;	 ffcReturn = ffcShow;
;	 *pdcp = cp0;
	xor	ax,ax
	mov	bx,pdcp
	mov	[bx],ax
	mov	[bx+2],ax
	mov	ax,ffcShow

;        goto LReturn;
	jmp	LReturn;

;        }
;    Assert (ifld != ifldNil && flcd.flt >= fltMin && flcd.flt < fltMax);

FFFP12:
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	di,ifldNil
	je	FFFP13
	cmp	flcd.fltFlcd,fltMin
	jl	FFFP13
	cmp	flcd.fltFlcd,fltMax
	jl	FFFP14
FFFP13:
	mov	ax,midFieldfmn
	mov	bx,1006
	cCall	AssertProcForNative,<ax,bx>
FFFP14:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;    fResult = FShowResultPflcdFvc (&flcd, ww /*fvcScreen*/);
	lea	ax,flcd
	push	ax
	push	ww
ifdef DEBUG
	cCall	S_FShowResultPflcdFvc,<>
else ;not DEBUG
	push	cs
	call	near ptr N_FShowResultPflcdFvc
endif ;DEBUG

;    *pdcp = DcpSkipFieldChPflcd (ch, &flcd, fResult, fFalse/*fFetch*/);
	push	ax	;save fResult
	lea	cx,flcd
	errnz	<fFalse>
	xor	bx,bx
	push	chArg
	push	cx
	push	ax
	push	bx
ifdef DEBUG
	cCall	S_DcpSkipFieldChPflcd,<>
else ;DEBUG
	push	cs
	call	near ptr N_DcpSkipFieldChPflcd
endif ;DEBUG
	pop	cx	;recover fResult
	mov	bx,pdcp
	mov	[bx],ax
	mov	[bx+2],dx

;    if (fResult && dnflt [flt].fDisplay)
	errnz	<fFalse>
	jcxz	FFFP15
	cCall	EfltFromFlt,<si>
	errnz	<(fDisplayEflt) - 0>
	test	al,maskfDisplayEflt
	je	FFFP15

;        {
;        /* display type fields may have a CHRDF/CHRFG to build */
;        ffcReturn = FfcFormatDisplay (ch, doc, &flcd, pdcp);
	lea	ax,flcd
	cCall	FfcFormatDisplay,<chArg, doc, ax, pdcp>
	jmp	short FFFP16

FFFP15:
;        }
;    else if (*pdcp == 0)
;	the next line is from the initialization
;	of ffcReturn in the procedure heading.
;    BOOL ffcReturn = ffcSkip;
	errnz	<ffcSkip>
	xor	ax,ax
	mov	bx,pdcp
	mov	cx,[bx+2]
	or	cx,[bx]
	jne	FFFP16

;        ffcReturn = ffcShow;
	mov	ax,ffcShow

FFFP16:
LReturn:
;    Assert (!(ffcReturn == ffcShow ^ *pdcp == 0));

ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	xor	dx,dx
	cmp	ax,ffcShow
	jne	FFFP17
	inc	dx
FFFP17:
	xor	cx,cx
	mov	bx,[pdcp]
	mov	ax,[bx+2]
	or	ax,[bx]
	jne	FFFP18
	inc	cx
FFFP18:
	xor	cx,dx
	je	FFFP19

	mov	ax,midFieldfmn
	mov	bx,1007
	cCall	AssertProcForNative,<ax,bx>
FFFP19:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;    Scribble (ispFieldFormat1, ' ');
ifdef DEBUG
	cmp	vdbs.grpfScribbleDbs,0
	je	FFFP20
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,ispFieldFormat1
	mov	bx,' '
	cCall	ScribbleProc,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FFFP20:
endif ;/* DEBUG */

;#ifdef DFORMULA
;    {
;    int rgw[6];
;    rgw[0]=ifld;
ifdef DFORMULA
	push	ax
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	mov	word ptr [(rgw)+00000h],di

;    rgw[1]=flt;
	mov	word ptr [(rgw)+00002h],si

;    rgw[2]=ch;
	mov	cx,chArg
	mov	word ptr [(rgw)+00004h],cx

;    rgw[3]=ffcReturn;
	mov	word ptr [(rgw)+00006h],ax

;    bltb (pdcp, &rgw[4], sizeof(CP));
	mov	bx,pdcp
	mov	ax,[bx]
	mov	word ptr [(rgw)+00008h],ax
	mov	ax,[bx+2]
	mov	word ptr [(rgw)+0000Ah],ax

;    CommSzRgNum (SzShared("FormatField (ifld,flt,ch,ffc,dcp): "), rgw, 6);
FFFP21:
	db	'FormatField (ifld,flt,ch,ffc,dcp): '
	db	0
FFFP22:
	push	ds
	push	cs
	pop	ds
	mov	si,FFFP21
	push	ss
	pop	es
	mov	di,szFrame
	mov	cx,(FFFP22) - (FFFP21)
	rep	movsb
	pop	ds
	lea	ax,szFrame
	lea	bx,rgw
	mov	cx,6
	cCall	CommSzRgNum,<ax, bx, cx>

	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
;    }
;#endif /* DFORMULA */
endif ;/* DFORMULA */

;    return ffcReturn;

FFFP23:
;}
cEnd

; End of FfcFormatFieldPdcp



;-------------------------------------------------------------------------
;	DcpSkipFieldChPflcd (ch, pflcd, fShowResult, fFetch)
;-------------------------------------------------------------------------
;/* D C P  S K I P  F I E L D  C H  P F L C D */
;/*  Return the number of characters to skip to display field fld given
;    ch in mode fShowResult.  fFetch is true for fetch visible operations.
;*/
;
;native CP DcpSkipFieldChPflcd (ch, pflcd, fShowResult, fFetch)
;CHAR ch;
;struct FLCD *pflcd;
;BOOL fShowResult, fFetch;
;
;{
;
;    int flt = pflcd->flt;

; %%Function:N_DcpSkipFieldChPflcd %%Owner:BRADV
cProc	N_DcpSkipFieldChPflcd,<PUBLIC,FAR>,<si>
	ParmW	chArg
	ParmW	pflcd
	ParmW	fShowResult
	ParmW	fFetch

;	flt kept in si

cBegin
	mov	bx,pflcd
	mov	si,[bx.fltFlcd]

;    Assert (ch & 0xFF00 == 0 && fFetch >= 0);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	test	chArg,0FF00h
	jne	DSFCP01
	cmp	fFetch,fFalse
	jge	DSFCP02

DSFCP01:
	mov	ax,midFieldfmn
	mov	bx,1008
	cCall	AssertProcForNative,<ax,bx>
DSFCP02:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;    switch (ch)
	mov	cl,bptr (chArg)

;        {
;        case chFieldBegin:

	cmp	cl,chFieldBegin
	jne	DSFCP03

;            if (fShowResult)
;	/* do the else clause first in assembler */
;            else /* ! fShowResults */
;                return cp0;
	xor	ax,ax
	cwd
	cmp	fShowResult,fFalse
	je	DSFCP06

;                if (dnflt [flt].fResult && !pflcd->fPrivateResult)
	cCall	EfltFromFlt,<si>
	mov	bx,pflcd
	errnz	<(fResultEflt) - 0>
	test	al,maskfResultEflt
	
;                    return pflcd->dcpInst;
	mov	ax,[bx.LO_dcpInstFlcd]
	mov	dx,[bx.HI_dcpInstFlcd]
	je	DSFCP10
	test	[bx.fPrivateResultFlcd],maskfPrivateResultFlcd
	je	DSFCP06

;                else
;                    return pflcd->dcpInst + pflcd->dcpResult - fFetch;
DSFCP10:
	add	ax,[bx.LO_dcpResultFlcd]
	adc	dx,[bx.HI_dcpResultFlcd]
	sub	ax,fFetch
	sbb	dx,0
	jmp	short DSFCP06

DSFCP03:
;        case chFieldSeparate:
	cmp	cl,chFieldSeparate
	jne	DSFCP05

;            if (fShowResult)
	cmp	fShowResult,fFalse
	je	DSFCP04

;                return (dnflt [flt].fResult && !pflcd->fPrivateResult ?
;                        (CP) 1 : pflcd->dcpResult);
	cCall	EfltFromFlt,<si>
	mov	bx,pflcd
	errnz	<(fResultEflt) - 0>
	test	al,maskfResultEflt
	mov	ax,00001h
	cwd
	je	DSFCP04
	test	[bx.fPrivateResultFlcd],maskfPrivateResultFlcd
	je	DSFCP06

DSFCP04:

;            else
;                return pflcd->dcpResult;
	mov	ax,[bx.LO_dcpResultFlcd]
	mov	dx,[bx.HI_dcpResultFlcd]
	jmp	short DSFCP06

DSFCP05:
;        case chFieldEnd:
ifdef DEBUG
;	/* Assert (ch == chFieldEnd) with a call so as not to mess up
;	short jumps */
	call	DSFCP07
endif ;/* DEBUG */

;            if (fShowResult)
;	/* do the else clause first in assembler */
;            else
;                return cp0;
	xor	ax,ax
	cwd	
	cmp	fShowResult,fFalse
	je	DSFCP06

;                if (fFetch)
;	/* do the else clause first in assembler */
;                else
;                    return (CP) 1;
	inc	ax
	cmp	fFetch,fFalse
	je	DSFCP06

;                    return dnflt[flt].fDisplay ? (CP) 0 : (CP) 1;
	cCall	EfltFromFlt,<si>
	errnz	<(fDisplayEflt) - 0>
	test	al,maskfDisplayEflt
	mov	ax,00000h
	cwd
	jne	DSFCP06
	inc	ax

;#ifdef DEBUG
;	/* look at case chFieldEnd for this code */
;        default:
;            Assert (fFalse);
;            return cp0;
;#endif /* DEBUG */

DSFCP06:

;        }
;
;}
cEnd

; End of DcpSkipFieldChPflcd

ifdef DEBUG
;    Assert (ch == chFieldEnd);
DSFCP07:
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	cl,chFieldEnd
	je	DSFCP08

	mov	ax,midFieldfmn
	mov	bx,1009
	cCall	AssertProcForNative,<ax,bx>
DSFCP08:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */




;-------------------------------------------------------------------------
;	FShowResultPflcdFvc ( pflcd, fvc )
;-------------------------------------------------------------------------
;/* F  S H O W  R E S U L T  P F L C D */
;/*  Determine the Show Results/Instructions mode for a specific field.
;*/
;
;FShowResultPflcdFvc(pflcd, fvc)
;struct FLCD * pflcd;
;int fvc;
;
;{
;    struct WWD *pwwd;

; %%Function:N_FShowResultPflcdFvc %%Owner:BRADV
cProc	N_FShowResultPflcdFvc,<PUBLIC,FAR>,<>
	ParmW	pflcd
	ParmW	fvc

cBegin

ifdef DEBUG
;    Assert (pflcd != NULL && fvc);
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	[pflcd],NULL
	jne	FSRPF01

	mov	ax,midFieldfmn
	mov	bx,1010
	cCall	AssertProcForNative,<ax,bx>
FSRPF01:
	cmp	[fvc],0
	jne	FSRPF015

	mov	ax,midFieldfmn
	mov	bx,1011
	cCall	AssertProcForNative,<ax,bx>
FSRPF015:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;    if (fvc & fvcmWw)
;        {
	mov	cx,[fvc]
	errnz	<fvcmWw - 000FFh>
	or	cl,cl
	jne	FSRPF02

;	do the else clause first in the assembler version.
;	 }
;    else if (fvc & fvcmFields)
;            return fFalse;
	errnz	<fFalse>
	xor	ax,ax
	errnz	<fvcmFields - 00100h>
	test	ch,fvcmFields SHR 8
	jne	FSRPF04

;    else
;            return fTrue;
	errnz	<fTrue - 1>
	inc	ax
	jmp	short FSRPF04

FSRPF02:
;	 Assert (!(fvc & ~fvcmWw));
ifdef DEBUG
;	/* Assert (!(fvc & ~ fvcmWw)) with a call so as not to mess up
;	short jumps */
	call	FSRPF05
endif ;/* DEBUG */

;	     pwwd = PwwdWw(fvc/*ww*/);
;	     if (!pwwd->grpfvisi.fvisiShowAll)
;	/* do the else clause first in assembler */
;            else
;                return fFalse ;
	mov	bx,cx
	;Takes ww in bx, result in bx.	Only bx is altered.
	call	LN_PwwdWw
	mov	dx,[bx.grpfvisiWwd]
	errnz	<fFalse>
	xor	ax,ax
	errnz	<maskfvisiShowAllGrpfvisi AND 0FF00h>
	test	dl,maskfvisiShowAllGrpfvisi
	jne	FSRPF04
	mov	dl,[bx.fPageViewWwd]

;		 {
;		 int f;
;
;		 f = FFromIGrpf (dnflt [pflcd->flt].fltg, pwwd->grpfvisi.grpfShowResults);

	push	dx	;save high grpfvisi and fPageView
	mov	bx,[pflcd]
	cCall	EfltFromFlt,<[bx.fltFlcd]>
	pop	dx	;restore high grpfvisi and fPageView
	errnz	<fltgEflt - 0>
	and	al,maskfltgEflt
	mov	cl,ibitfltgEflt
	shr	al,cl
	xchg	ax,cx
	mov	al,dh
	errnz	<maskgrpfShowResultsGrpfvisi - 01800h>
	add	cl,3
	and	al,maskgrpfShowResultsGrpfvisi SHR 8
	shr	al,cl

;		 if (!pwwd->grpfvisi.fForceField && !pwwd->fPageView)
	test	dx,maskfPageViewWwd + maskfForceFieldGrpfvisi
	jne	FSRPF03

;                    f ^= pflcd->fDiffer;
	errnz	<maskfDifferFlcd - 001h>
	mov	bx,[pflcd]
	xor	al,[bx.fDifferFlcd]

FSRPF03:
;                return f;
	and	ax,00001h

FSRPF04:

;        }
;}
cEnd

; End of FShowResultPflcdFvc

ifdef DEBUG
;	 Assert (!(fvc & ~fvcmWw));
FSRPF05:
	push	ax
	push	bx
	push	cx
	push	dx
	errnz	<fvcmWw - 000FFh>
	or	ch,ch
	je	FSRPF06

	mov	ax,midFieldfmn
	mov	bx,1012
	cCall	AssertProcForNative,<ax,bx>
FSRPF06:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */



;-------------------------------------------------------------------------
;	FillIfldFlcd ( hplcfld, ifld, pflcd )
;-------------------------------------------------------------------------
;/* F I L L  I F L D  F L C D */
;/*  Fill pflcd with information about field ifld.  Must scan plc and find
;    all of the entries which refer to ifld.
;*/
;
;native FillIfldFlcd (hplcfld, ifld, pflcd)
;struct PLC **hplcfld;
;int ifld;
;struct FLCD *pflcd;
;
;{
;	int cChBegin = 0;
;	BOOL fSeparateFound = fFalse;
;	CP cpSeparate;
;	struct FLD fld;

; %%Function:N_FillIfldFlcd %%Owner:BRADV
cProc	N_FillIfldFlcd,<PUBLIC,FAR>,<si,di>
	ParmW	hplcfld
	ParmW	ifld
	ParmW	pflcd

	LocalW	fSeparateFound
	LocalD	cpSeparate
	LocalV	flcd, cbFlcdMin
ifdef DEBUG
	LocalW	iMacPlcArg
endif ;/* DEBUG */

cBegin

	mov	fSeparateFound,fFalse
ifdef DEBUG
;       Debug (iMacPlc = IMacPlc( hplcfld ));
	cCall	IMacPlc,<hplcfld>
	mov	iMacPlcArg,ax
endif ;/* DEBUG */

;	SetBytes (pflcd, 0, cbFLCD);
	mov	di,pflcd
	push	ds
	pop	es
	xor	al,al
	mov	cx,cbFlcdMin
	rep	stosb

;	Assert (ifld != ifldNil);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	ifld,ifldNil
	jne	FIF01

	mov	ax,midFieldfmn
	mov	bx,1013
	cCall	AssertProcForNative,<ax,bx>
FIF01:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;	GetPlc( hplcfld, ifld, &fld );
;	QcpQfooPlcfoo takes hplcfoo in bx, ifoo in si, it returns pplcfoo
;	in bx, cbfoo in cx, qcp in es:di, qfoo in es:si.
;	if DEBUG it returns hpcp in dx:di, hpfoo in dx:si.
;	Changes ax, bx, cx, dx, si, di.
	mov	bx,hplcfld
	mov	si,ifld
ifdef DEBUG
	xor	di,di	;Not O.K. to pass ifldMac
endif ;DEBUG
	cCall	N_QcpQfooPlcfoo,<>
ifdef DEBUG
	;Check that es == mpsbps[dx];
	call	FIF13
endif ;DEBUG

;	pflcd now kept in bx
	mov	bx,pflcd

;	pflcd->ifldChSeparate = ifldNil;
	mov	[bx.ifldChSeparateFlcd],ifldNil

;	pflcd->ifldChBegin = ifld;
;	ifld now kept in dx
	mov	dx,ifld
	mov	[bx.ifldChBeginFlcd],dx
	
;	pflcd->cpFirst = CpPlc( hplcfld, ifld );
;	here we are performing CpPlc( hplcfld, ifld) in a local subroutine.
;	FIF_CpPlc expects pcp in es:di, ifld in dx, and puts
;	the result cp in cx:ax.  Uses hplcfld from the frame
;	of FillIfldFlcd.  Only ax and cx are altered.
	call	FIF_CpPlc
	mov	[bx.LO_cpFirstFlcd],ax
	mov	[bx.HI_cpFirstFlcd],cx

	errnz	<cbFldMin - 2>
	lods	wptr es:[si]

;	Assert (fld.ch == chFieldBegin);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	errnz	<(chFld) - 0>
	and	al,maskChFld
	cmp	al,chFieldBegin
	je	FIF015

	mov	ax,midFieldfmn
	mov	bx,1014
	cCall	AssertProcForNative,<ax,bx>
FIF015:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;	pflcd->fDirty = fld.fDirty;
	push	ax	;save fld
	errnz	<(fDirtyFld) - 0>
	and	ax,maskFDirtyFld
	mov	[bx.fDirtyFlcd],ax
	pop	ax	;restore fld

;	pflcd->flt = fld.flt;
	errnz	<(fltFld) - 1>
	mov	al,ah
	xor	ah,ah
	mov	[bx.fltFlcd],ax

;	cChBegin now kept in cx
	xor	cx,cx

;	for (;;)
;        	{
FIF02:

;		GetPlc( hplcfld, ++ifld, &fld );
	inc	dx
	errnz	<cbFldMin - 2>
	lods	wptr es:[si]
	add	di,4

ifdef DEBUG
;	/* Assert (ifld < IMacPlc(hplcfld)) with a call so as not to mess up
;	short jumps */
	call	FIF07
endif ;/* DEBUG */

;	        switch (fld.ch)
;        		{
	errnz	<(chFld) - 0>

;		case chFieldBegin:
	and	al,maskChFld
	cmp	al,chFieldBegin
	jne	FIF03

;			cChBegin++;
	inc	cx

;                	break;
;	break and continue in loop;
	jmp	short FIF02

FIF03:
;		case chFieldSeparate:
	cmp	al,chFieldSeparate
	jne	FIF04

;                	if (cChBegin == 0)
;                		{
	or	cx,cx
;	bypass if clause, break and continue in loop;
	jne	FIF02

ifdef DEBUG
;	/* Assert (!fSeparateFound) with a call so as not to mess up
;	short jumps */
	call	FIF09
endif ;/* DEBUG */

;                		fSeparateFound = fTrue;
	mov	fSeparateFound,fTrue

;		                pflcd->ifldChSeparate = ifld;
	mov	[bx.ifldChSeparateFlcd],dx

;                		pflcd->bData = fld.bData;
	errnz <(bDataFld) - 1>
	mov	al,ah
	xor	ah,ah
	mov	[bx.bDataFlcd],ax

;		                cpSeparate = CpPlc( hplcfld, ifld );
	push	cx	; save cChBegin
;	here we are performing CpPlc( hplcfld, ifld) in a local subroutine.
;	FIF_CpPlc expects pcp in es:di, ifld in dx, and puts
;	the result cp in cx:ax.  Uses hplcfld from the frame
;	of FillIfldFlcd.  Only ax and cx are altered.
	call	FIF_CpPlc
	mov	OFF_cpSeparate,ax
	mov	SEG_cpSeparate,cx

;                		pflcd->dcpInst = cpSeparate - pflcd->cpFirst +1;
	sub	ax,[bx.LO_cpFirstFlcd]
	sbb	cx,[bx.HI_cpFirstFlcd]
	add	ax,1
	adc	cx,0
	mov	[bx.LO_dcpInstFlcd],ax
	mov	[bx.HI_dcpInstFlcd],cx
	pop	cx	;restore cChBegin

;		                }
;                	break;
;	break and continue in loop;
	jmp	short FIF02

FIF04:
;	        case chFieldEnd:
ifdef DEBUG
;	/* Assert (fld.ch == chFieldEnd) with a call so as not to mess up
;	short jumps */
	call	FIF11
endif ;/* DEBUG */
;			if (cChBegin-- == 0)
;                		{
	dec	cx
;	bypass if clause, break and continue in loop;
	jge	FIF02

;				pflcd->ifldChEnd = ifld;
	mov	[bx.ifldChEndFlcd],dx

;		                pflcd->grpf = fld.grpf;
	errnz <(grpfFld) - 1>
	mov	[bx.grpfFlcd],ah

;	here we are performing CpPlc( hplcfld, ifld) in a local subroutine.
;	FIF_CpPlc expects pcp in es:di, ifld in dx, and puts
;	the result cp in cx:ax.  Uses hplcfld from the frame
;	of FillIfldFlcd.  Only ax and cx are altered.
	call	FIF_CpPlc

;				if (fSeparateFound)
	cmp	fSeparateFound,fFalse
	je	FIF05

;        	                	pflcd->dcpResult = CpPlc( hplcfld, ifld ) - cpSeparate;
	sub	ax,OFF_cpSeparate
	sbb	cx,SEG_cpSeparate
	mov	[bx.LO_dcpResultFlcd],ax
	mov	[bx.HI_dcpResultFlcd],cx
;	bypass else clause and return
	jmp	short FIF06

FIF05:
;	        	        else
;        			        pflcd->dcpInst = CpPlc( hplcfld, ifld ) - pflcd->cpFirst + 1;
	sub	ax,[bx.LO_cpFirstFlcd]
	sbb	cx,[bx.HI_cpFirstFlcd]
	add	ax,1
	adc	cx,0
	mov	[bx.LO_dcpInstFlcd],ax
	mov	[bx.HI_dcpInstFlcd],cx

;	                    	return;
;               			}
;        	        break;

;#ifdef DEBUG
;	        default:
;	/* look at case chFieldEnd for this code */
;        	        Assert (fFalse);
;#endif /* DEBUG */
;		        }
;		}

FIF06:
;}
cEnd


;	FIF_CpPlc expects pcp in es:di, ifld in dx, and puts
;	the result cp in cx:ax.  Uses hplcfld from the frame
;	of FillIfldFlcd.  Only ax and cx are altered.
FIF_CpPlc:
	mov	ax,es:[di]
	mov	cx,es:[di+2]
	push	si	;save qfld;
	mov	si,hplcfld
	mov	si,[si]
	cmp	dx,[si.icpAdjustPlc]
	jl	FIFCP01
	add	ax,[si.LO_dcpAdjustPlc]
	adc	cx,[si.HI_dcpAdjustPlc]
FIFCP01:
	pop	si	;restore qfld;
	ret


; End of FillIfldFlcd

ifdef DEBUG
;    Assert (ifld < IMacPlc(hplcfld));
FIF07:
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	dx,iMacPlcArg
	jl	FIF08

	mov	ax,midFieldfmn
	mov	bx,1015
	cCall	AssertProcForNative,<ax,bx>
FIF08:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

ifdef DEBUG
;    Assert (!fSeparateFound);
FIF09:
	cmp	fSeparateFound,fFalse
	je	FIF10
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,midFieldfmn
	mov	bx,1016
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
FIF10:
	ret
endif ;/* DEBUG */

ifdef DEBUG
;    Assert (fld.ch == chFieldEnd);
FIF11:
	push	ax
	push	bx
	push	cx
	push	dx
	errnz	<(chFld) - 0>
	and	al,maskChFld
	cmp	al,chFieldEnd
	je	FIF12

	mov	ax,midFieldfmn
	mov	bx,1017
	cCall	AssertProcForNative,<ax,bx>
FIF12:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

ifdef DEBUG
FIF13:
	push	ax
	push	bx
	push	cx
	push	dx
	push	es	;save es from QcpQfooPlcfoo
	mov	bx,dx
	shl	bx,1
	mov	ax,mpsbps[bx]
	mov	es,ax
	shr	ax,1
	jc	FIF14
	;Assembler note: There is no way we should have to call ReloadSb here.
;	reload sb trashes ax, cx, and dx
;	cCall	ReloadSb,<>
	mov	ax,midFieldfmn
	mov	bx,1022
	cCall	AssertProcForNative,<ax,bx>
FIF14:
	pop	ax	;restore es from QcpQfooPlcfoo
	mov	bx,es	;compare with es rederived from the SB of QcpQfooPlcfoo
	cmp	ax,bx
	je	FIF15
	mov	ax,midFieldfmn
	mov	bx,1023
	cCall	AssertProcForNative,<ax,bx>
FIF15:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */



;-------------------------------------------------------------------------
;	GetIfldFlcd ( doc, ifld, pflcd )
;-------------------------------------------------------------------------
;/* G E T  I F L D  F L C D */
;/*  Fill pflcd with the composit field information for field ifld in doc.
;*/
;
;native GetIfldFlcd (doc, ifld, pflcd)
;int doc, ifld;
;struct FLCD *pflcd;
;
;{

; %%Function:N_GetIfldFlcd %%Owner:BRADV
cProc	N_GetIfldFlcd,<PUBLIC,FAR>,<>
	ParmW	doc
	ParmW	ifld
	ParmW	pflcd

cBegin

;    struct PLCFLD **hplcfld = PdodDoc (doc)->hplcfld;
	mov	bx,[doc]
	;Takes doc in bx, result in bx.  Only bx is altered.
	call	LN_PdodDoc
	mov	ax,[bx].hplcfldDod

;    if (hplcfld != hNil && ifld >= 0 && ifld < IMacPlc( hplcfld ))
	errnz	<hNil>
	or	ax,ax
	je	GIF01
	cmp	ifld,0
	jl	GIF01
	push	ax
	cCall	IMacPlc,<ax>
	pop	bx
	cmp	ifld,ax
	jge	GIF01

;        FillIfldFlcd (hplcfld, ifld, pflcd);
	push	bx
	push	ifld
	push	pflcd
ifdef DEBUG
	cCall	S_FillIfldFlcd,<>
else ;not DEBUG
	push	cs
	call	near ptr N_FillIfldFlcd
endif ;DEBUG
	jmp	short GIF02

GIF01:
;    else
;        SetBytes (pflcd, 0, cbFLCD);

	push	di
	mov	di,pflcd
	push	ds
	pop	es
	xor	al,al
	mov	cx,cbFlcdMin
	rep	stosb
	pop	di

GIF02:
;}
cEnd

; End of GetIfldFlcd



;-------------------------------------------------------------------------
;	SetFlcdCh ( doc, pflcd, ch )
;-------------------------------------------------------------------------
;/* S E T  F L C D  C H */
;/*  Set data items for the field represented by pflcd in doc.  ch
;    indicates which items to update (chNil == all)
;*/
;
;native SetFlcdCh (doc, pflcd, ch)
;int doc;
;struct FLCD *pflcd;
;CHAR ch;
;
;{
;	struct PLCFLD **hplcfld = PdodDoc (doc)->hplcfld;
;	int ifld;
;	struct FLD fld;

; %%Function:N_SetFlcdCh %%Owner:BRADV
cProc	N_SetFlcdCh,<PUBLIC,FAR>,<si,di>
	ParmW	doc
	ParmW	pflcd
	ParmW	chArg

	LocalV	fldVar, cbFldMin

;	hplcfld is kept in si
;	pflcd is kept in di

cBegin

	mov	bx,[doc]
	;Takes doc in bx, result in bx.  Only bx is altered.
	call	LN_PdodDoc
	mov	si,[bx.hplcfldDod]
	mov	di,[pflcd]

;	Assert( hplcfld != hNil);
ifdef DEBUG
	push	ax
	push	bx
	push	cx
	push	dx
	cmp	si,hNil
	jne	SFC01

	mov	ax,midFieldfmn
	mov	bx,1018
	cCall	AssertProcForNative,<ax,bx>
SFC01:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;	ch lies in ah
;	chCur lies in al
;	ifld lies in bx
;	bCur lies in ch
;	fDirty lies in cl bit 7
	mov	al,chFieldBegin

;		ifld = pflcd->ifldChBegin;
;		fld.flt = pflcd->flt;
;		fld.fDirty = pflcd->fDirty;
	mov	bx,[di.ifldChBeginFlcd]
	mov	ch,bptr ([di.fltFlcd])
	call	SFC025

;		if (pflcd->ifldChSeparate != ifldNil)
	cmp	[di.ifldChSeparateFlcd],ifldNil
	je	SFC02

;	ch lies in ah
;	chCur lies in al
;	ifld lies in bx
;	bCur lies in ch
;	fDirty lies in cl bit 7
	mov	al,chFieldSeparate

;		ifld = pflcd->ifldChSeparate;
;			fld.bData = pflcd->bData;
	mov	bx,[di.ifldChSeparateFlcd]
	mov	ch,bptr ([di.bDataFlcd])
	call	SFC03

SFC02:
;	ch lies in ah
;	chCur lies in al
;	ifld lies in bx
;	bCur lies in ch
;	fDirty lies in cl bit 7
	mov	al,chFieldEnd

;		ifld = pflcd->ifldChEnd;
;		fld.grpf = pflcd->grpf;
	mov	bx,[di.ifldChEndFlcd]
	mov	ch,bptr ([di.grpfFlcd])
	call	SFC03
;}
cEnd

; End of SetFlcdCh
	

SFC025:
	mov	cl,maskFDirtyFld
	cmp	[di.fDirtyFlcd],fFalse
	jne	SFC035
SFC03:
	mov	cl,0
SFC035:
	mov	ah,bptr ([chArg])

;	ch lies in ah
;	chCur lies in al
;	ifld lies in bx
;	bCur lies in ch
;	fDirty lies in cl bit 7
	
;	if (!ch || ch == chCur)
	or	ah,ah
	je	SFC04
	cmp	ah,al
	jne	SFC06
SFC04:

ifdef DEBUG
;		GetPlc( hplcfld, ifld, &fld );
	push	ax
	push	bx
	push	cx
	push	dx
	lea	ax,[fldVar]
	cCall	GetPlc,<si, bx, ax>
	pop	dx
	pop	cx
	pop	bx
	pop	ax

;	Assert (fld.ch == chCur);
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ah,[fldVar.chFld]
	and	ah,maskChFld
	and	al,maskChFld
	cmp	ah,al
	je	SFC05
	mov	ax,midFieldfmn
	mov	bx,1019
	cCall	AssertProcForNative,<ax,bx>
SFC05:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
endif ;/* DEBUG */

;		fld.bCur = bCur;
;		fld.fDirty = pflcd->fDirty;
;		PutPlcLast( hplcfld, ifld, &fld );
	;Assembler note: since we just asserted fld.ch == chCur
	;we will replace fld.ch with chCur.  This means that we are
	;now replacing the entire fld, so we can get rid of the call
	;above to GetPlc, and replace the call to PutPlcLast with
	;a call to PutPlc.
	errnz	<cbFldMin - 2>
	errnz	<(fltFld) - 1>
	errnz	<(bDataFld) - 1>
	errnz	<(grpfFld) - 1>
	errnz	<(fDirtyFld) - 0>
	errnz	<maskFDirtyFld - 080h>
	errnz	<(chFld) - 0>
	errnz	<maskChFld - 07Fh>
	or	cl,al
	mov	wptr ([fldVar.fDirtyFld]),cx
	lea	ax,[fldVar]
	cCall	PutPlc,<si, bx, ax>

SFC06:
	ret


LN_PwwdWw:
	;Takes ww in bx, result in bx.	Only bx is altered.
	shl	bx,1
	mov	bx,[bx.mpwwhwwd]
ifdef DEBUG
	cmp	bx,hNil
	jne	LN_PW101
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFieldfmn
	mov	bx,1020
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LN_PW101:
endif ;/* DEBUG */
	mov	bx,[bx]
	ret


LN_PdodDoc:
	;Takes doc in bx, result in bx.  Only bx is altered.
	shl	bx,1
	mov	bx,[bx.mpdochdod]
ifdef DEBUG
	cmp	bx,hNil
	jne	LN_PD01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFieldfmn
	mov	bx,1021
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LN_PD01:
endif ;/* DEBUG */
	mov	bx,[bx]
	ret


sEnd	fieldfmt_PCODE
	end
